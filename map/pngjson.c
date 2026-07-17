#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "lodepng.h"

typedef struct {
    const char *name;
    unsigned left;
    unsigned top;
    unsigned right;
    unsigned bottom;
} sheet_crop;

typedef struct {
    FILE *fp;
    unsigned char carry[3];
    unsigned carry_len;
} base64_writer;

typedef struct {
    const sheet_crop *crop;
    char source_path[PATH_MAX];
    char json_path[PATH_MAX];
    char png_path[PATH_MAX];
    char review_path[PATH_MAX];
    unsigned width;
    unsigned height;
    double mean_delta;
    unsigned max_delta;
    unsigned long long json_bytes;
    unsigned long long png_bytes;
} sheet_result;

static const sheet_crop SHEET_CROPS[] = {
    {"160", 564, 961, 10482, 12102},
    {"161", 491, 996, 10404, 12138},
    {"170", 654, 956, 10572, 12096},
    {"171", 542, 938, 10462, 12106},
};

static const char BASE64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s --sources DIR --output-dir DIR [--sheet 160] [--review-width PX]\n",
            program);
}

static const sheet_crop *find_sheet(const char *name)
{
    size_t i;
    for (i = 0; i < sizeof(SHEET_CROPS) / sizeof(SHEET_CROPS[0]); i++) {
        if (strcmp(SHEET_CROPS[i].name, name) == 0) {
            return &SHEET_CROPS[i];
        }
    }
    return NULL;
}

static int path_join(char *out, size_t out_size, const char *left, const char *right)
{
    int written;
    size_t left_len = strlen(left);
    const char *separator = (left_len > 0 && left[left_len - 1] == '/') ? "" : "/";
    written = snprintf(out, out_size, "%s%s%s", left, separator, right);
    return written > 0 && (size_t)written < out_size;
}

static int make_output_dir(const char *path)
{
    if (mkdir(path, 0755) == 0) {
        return 1;
    }
    if (errno == EEXIST) {
        return 1;
    }
    fprintf(stderr, "pngjson: unable to create %s: %s\n", path, strerror(errno));
    return 0;
}

static unsigned long long file_size(const char *path)
{
    struct stat info;
    if (stat(path, &info) != 0) {
        return 0;
    }
    return (unsigned long long)info.st_size;
}

static void b64_emit(base64_writer *writer, const unsigned char *triple, unsigned len)
{
    unsigned a = triple[0];
    unsigned b = len > 1 ? triple[1] : 0;
    unsigned c = len > 2 ? triple[2] : 0;
    fputc(BASE64[a >> 2], writer->fp);
    fputc(BASE64[((a & 3) << 4) | (b >> 4)], writer->fp);
    fputc(len > 1 ? BASE64[((b & 15) << 2) | (c >> 6)] : '=', writer->fp);
    fputc(len > 2 ? BASE64[c & 63] : '=', writer->fp);
}

static void b64_write(base64_writer *writer, const unsigned char *data, size_t len)
{
    size_t index = 0;
    if (writer->carry_len > 0) {
        while (writer->carry_len < 3 && index < len) {
            writer->carry[writer->carry_len++] = data[index++];
        }
        if (writer->carry_len == 3) {
            b64_emit(writer, writer->carry, 3);
            writer->carry_len = 0;
        }
    }
    while (index + 3 <= len) {
        b64_emit(writer, &data[index], 3);
        index += 3;
    }
    while (index < len) {
        writer->carry[writer->carry_len++] = data[index++];
    }
}

static void b64_close(base64_writer *writer)
{
    if (writer->carry_len > 0) {
        b64_emit(writer, writer->carry, writer->carry_len);
        writer->carry_len = 0;
    }
}

static int b64_value(int ch)
{
    if (ch >= 'A' && ch <= 'Z') return ch - 'A';
    if (ch >= 'a' && ch <= 'z') return ch - 'a' + 26;
    if (ch >= '0' && ch <= '9') return ch - '0' + 52;
    if (ch == '+') return 62;
    if (ch == '/') return 63;
    if (ch == '=') return -2;
    return -1;
}

static int read_header(FILE *fp, char *buffer, size_t buffer_size)
{
    size_t bytes;
    rewind(fp);
    bytes = fread(buffer, 1, buffer_size - 1, fp);
    buffer[bytes] = 0;
    return bytes > 0;
}

static int parse_resolution(const char *header, unsigned *width, unsigned *height)
{
    const char *start = strstr(header, "\"resolution\":[");
    if (start == NULL) {
        return 0;
    }
    return sscanf(start, "\"resolution\":[%u,%u]", width, height) == 2;
}

static long find_data_offset(FILE *fp)
{
    const char *needle = "\"data\":\"";
    const size_t needle_len = strlen(needle);
    size_t matched = 0;
    int ch;
    rewind(fp);
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == needle[matched]) {
            matched++;
            if (matched == needle_len) {
                return ftell(fp);
            }
        } else {
            matched = (ch == needle[0]) ? 1 : 0;
        }
    }
    return -1;
}

static int decode_base64_data(FILE *fp, long data_offset, unsigned char *out, size_t expected)
{
    int block[4];
    unsigned count = 0;
    size_t written = 0;
    int ch;
    if (fseek(fp, data_offset, SEEK_SET) != 0) {
        return 0;
    }
    while ((ch = fgetc(fp)) != EOF) {
        int value;
        if (ch == '"') {
            break;
        }
        value = b64_value(ch);
        if (value < -1) {
            block[count++] = value;
        } else if (value >= 0) {
            block[count++] = value;
        } else {
            fprintf(stderr, "pngjson: invalid base64 character %d\n", ch);
            return 0;
        }
        if (count == 4) {
            unsigned triple;
            if (block[0] < 0 || block[1] < 0) {
                return 0;
            }
            triple = ((unsigned)block[0] << 18) | ((unsigned)block[1] << 12);
            if (block[2] >= 0) {
                triple |= (unsigned)block[2] << 6;
            }
            if (block[3] >= 0) {
                triple |= (unsigned)block[3];
            }
            if (written >= expected) return 0;
            out[written++] = (unsigned char)((triple >> 16) & 255);
            if (block[2] >= 0) {
                if (written >= expected) return 0;
                out[written++] = (unsigned char)((triple >> 8) & 255);
            }
            if (block[3] >= 0) {
                if (written >= expected) return 0;
                out[written++] = (unsigned char)(triple & 255);
            }
            count = 0;
        }
    }
    return count == 0 && written == expected;
}

static int write_json_from_source(const char *source_path,
                                  const sheet_crop *crop,
                                  const char *json_path)
{
    unsigned char *image = NULL;
    unsigned width = 0;
    unsigned height = 0;
    unsigned error;
    unsigned y;
    FILE *fp;
    base64_writer writer;
    unsigned crop_width = crop->right - crop->left;
    unsigned crop_height = crop->bottom - crop->top;

    error = lodepng_decode24_file(&image, &width, &height, source_path);
    if (error != 0) {
        fprintf(stderr, "pngjson: decode %s failed: %s\n", source_path, lodepng_error_text(error));
        return 0;
    }
    if (crop->right > width || crop->bottom > height || crop->left >= crop->right || crop->top >= crop->bottom) {
        fprintf(stderr, "pngjson: sheet %s crop is outside %s\n", crop->name, source_path);
        free(image);
        return 0;
    }
    fp = fopen(json_path, "wb");
    if (fp == NULL) {
        fprintf(stderr, "pngjson: unable to write %s: %s\n", json_path, strerror(errno));
        free(image);
        return 0;
    }
    fprintf(fp,
            "{\"format\":\"london1940-rgb-map-json\","
            "\"version\":1,"
            "\"sheet\":\"%s\","
            "\"source\":{\"path\":\"%s\",\"crop\":[%u,%u,%u,%u]},"
            "\"resolution\":[%u,%u],"
            "\"encoding\":{\"codec\":\"rgb24-base64-v1\",\"byteOrder\":\"RGB\",\"data\":\"",
            crop->name,
            source_path,
            crop->left,
            crop->top,
            crop->right,
            crop->bottom,
            crop_width,
            crop_height);
    writer.fp = fp;
    writer.carry_len = 0;
    for (y = crop->top; y < crop->bottom; y++) {
        const unsigned char *row = image + (((size_t)y * width + crop->left) * 3);
        b64_write(&writer, row, (size_t)crop_width * 3);
    }
    b64_close(&writer);
    fprintf(fp, "\"}}\n");
    if (fclose(fp) != 0) {
        fprintf(stderr, "pngjson: unable to close %s: %s\n", json_path, strerror(errno));
        free(image);
        return 0;
    }
    free(image);
    return 1;
}

static unsigned char *read_json_rgb(const char *json_path, unsigned *width, unsigned *height)
{
    char header[65536];
    FILE *fp = fopen(json_path, "rb");
    long data_offset;
    size_t expected;
    unsigned char *rgb;
    if (fp == NULL) {
        fprintf(stderr, "pngjson: unable to read %s: %s\n", json_path, strerror(errno));
        return NULL;
    }
    if (!read_header(fp, header, sizeof(header)) || !parse_resolution(header, width, height)) {
        fprintf(stderr, "pngjson: unable to parse resolution in %s\n", json_path);
        fclose(fp);
        return NULL;
    }
    data_offset = find_data_offset(fp);
    if (data_offset < 0) {
        fprintf(stderr, "pngjson: unable to find image data in %s\n", json_path);
        fclose(fp);
        return NULL;
    }
    expected = (size_t)(*width) * (*height) * 3;
    rgb = (unsigned char *)malloc(expected);
    if (rgb == NULL) {
        fprintf(stderr, "pngjson: unable to allocate %zu RGB bytes\n", expected);
        fclose(fp);
        return NULL;
    }
    if (!decode_base64_data(fp, data_offset, rgb, expected)) {
        fprintf(stderr, "pngjson: base64 decode failed for %s\n", json_path);
        free(rgb);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    return rgb;
}

static int write_png_from_json(const char *json_path, const char *png_path,
                               unsigned char **rgb_out,
                               unsigned *width_out,
                               unsigned *height_out)
{
    unsigned width = 0;
    unsigned height = 0;
    unsigned char *rgb = read_json_rgb(json_path, &width, &height);
    unsigned error;
    if (rgb == NULL) {
        return 0;
    }
    error = lodepng_encode24_file(png_path, rgb, width, height);
    if (error != 0) {
        fprintf(stderr, "pngjson: encode %s failed: %s\n", png_path, lodepng_error_text(error));
        free(rgb);
        return 0;
    }
    if (rgb_out != NULL) {
        *rgb_out = rgb;
        *width_out = width;
        *height_out = height;
    } else {
        free(rgb);
    }
    return 1;
}

static int compare_source_crop(const char *source_path,
                               const sheet_crop *crop,
                               const unsigned char *generated,
                               unsigned generated_width,
                               unsigned generated_height,
                               double *mean_delta,
                               unsigned *max_delta)
{
    unsigned char *source = NULL;
    unsigned width = 0;
    unsigned height = 0;
    unsigned error;
    unsigned y;
    unsigned long long total = 0;
    unsigned max_seen = 0;
    unsigned crop_width = crop->right - crop->left;
    unsigned crop_height = crop->bottom - crop->top;
    error = lodepng_decode24_file(&source, &width, &height, source_path);
    if (error != 0) {
        fprintf(stderr, "pngjson: decode %s failed during compare: %s\n",
                source_path, lodepng_error_text(error));
        return 0;
    }
    if (generated_width != crop_width || generated_height != crop_height) {
        fprintf(stderr, "pngjson: generated resolution mismatch for sheet %s\n", crop->name);
        free(source);
        return 0;
    }
    for (y = 0; y < crop_height; y++) {
        unsigned x;
        const unsigned char *source_row = source + (((size_t)(crop->top + y) * width + crop->left) * 3);
        const unsigned char *generated_row = generated + ((size_t)y * generated_width * 3);
        for (x = 0; x < crop_width * 3; x++) {
            unsigned left = source_row[x];
            unsigned right = generated_row[x];
            unsigned delta = left > right ? left - right : right - left;
            total += delta;
            if (delta > max_seen) {
                max_seen = delta;
            }
        }
    }
    *mean_delta = (double)total / ((double)crop_width * (double)crop_height * 3.0);
    *max_delta = max_seen;
    free(source);
    return 1;
}

static int write_review_png(const char *review_path,
                            const unsigned char *rgb,
                            unsigned width,
                            unsigned height,
                            unsigned review_width)
{
    unsigned review_height;
    unsigned char *review;
    unsigned y;
    unsigned error;
    if (review_width == 0 || width <= review_width) {
        error = lodepng_encode24_file(review_path, rgb, width, height);
        if (error != 0) {
            fprintf(stderr, "pngjson: encode %s failed: %s\n", review_path, lodepng_error_text(error));
            return 0;
        }
        return 1;
    }
    review_height = (unsigned)floor(((double)height * (double)review_width / (double)width) + 0.5);
    if (review_height == 0) {
        review_height = 1;
    }
    review = (unsigned char *)malloc((size_t)review_width * review_height * 3);
    if (review == NULL) {
        fprintf(stderr, "pngjson: unable to allocate review image\n");
        return 0;
    }
    for (y = 0; y < review_height; y++) {
        unsigned x;
        unsigned source_y = (unsigned)(((unsigned long long)y * height) / review_height);
        for (x = 0; x < review_width; x++) {
            unsigned source_x = (unsigned)(((unsigned long long)x * width) / review_width);
            const unsigned char *source_pixel = rgb + (((size_t)source_y * width + source_x) * 3);
            unsigned char *dest_pixel = review + (((size_t)y * review_width + x) * 3);
            dest_pixel[0] = source_pixel[0];
            dest_pixel[1] = source_pixel[1];
            dest_pixel[2] = source_pixel[2];
        }
    }
    error = lodepng_encode24_file(review_path, review, review_width, review_height);
    free(review);
    if (error != 0) {
        fprintf(stderr, "pngjson: encode %s failed: %s\n", review_path, lodepng_error_text(error));
        return 0;
    }
    return 1;
}

static int roundtrip_sheet(const char *sources_dir,
                           const char *output_dir,
                           const sheet_crop *crop,
                           unsigned review_width,
                           sheet_result *result)
{
    unsigned char *rgb = NULL;
    unsigned width = 0;
    unsigned height = 0;
    char source_name[32];
    char json_name[64];
    char png_name[64];
    char review_name[64];
    snprintf(source_name, sizeof(source_name), "%s.png", crop->name);
    snprintf(json_name, sizeof(json_name), "%s.map.json", crop->name);
    snprintf(png_name, sizeof(png_name), "%s.map-from-json.png", crop->name);
    snprintf(review_name, sizeof(review_name), "%s.map-from-json.review.png", crop->name);
    memset(result, 0, sizeof(*result));
    result->crop = crop;
    if (!path_join(result->source_path, sizeof(result->source_path), sources_dir, source_name) ||
        !path_join(result->json_path, sizeof(result->json_path), output_dir, json_name) ||
        !path_join(result->png_path, sizeof(result->png_path), output_dir, png_name) ||
        !path_join(result->review_path, sizeof(result->review_path), output_dir, review_name)) {
        fprintf(stderr, "pngjson: output path is too long for sheet %s\n", crop->name);
        return 0;
    }
    printf("round-tripping sheet %s\n", crop->name);
    fflush(stdout);
    if (!write_json_from_source(result->source_path, crop, result->json_path)) {
        return 0;
    }
    if (!write_png_from_json(result->json_path, result->png_path, &rgb, &width, &height)) {
        return 0;
    }
    if (!compare_source_crop(result->source_path, crop, rgb, width, height,
                             &result->mean_delta, &result->max_delta)) {
        free(rgb);
        return 0;
    }
    if (!write_review_png(result->review_path, rgb, width, height, review_width)) {
        free(rgb);
        return 0;
    }
    free(rgb);
    result->width = width;
    result->height = height;
    result->json_bytes = file_size(result->json_path);
    result->png_bytes = file_size(result->png_path);
    printf("sheet %s: %ux%u meanDelta %.6f maxDelta %u\n",
           crop->name, result->width, result->height,
           result->mean_delta, result->max_delta);
    fflush(stdout);
    return 1;
}

static int write_manifest(const char *output_dir, const sheet_result *results, size_t count)
{
    char manifest_path[PATH_MAX];
    FILE *fp;
    size_t i;
    if (!path_join(manifest_path, sizeof(manifest_path), output_dir, "manifest.json")) {
        fprintf(stderr, "pngjson: manifest path is too long\n");
        return 0;
    }
    fp = fopen(manifest_path, "wb");
    if (fp == NULL) {
        fprintf(stderr, "pngjson: unable to write %s: %s\n", manifest_path, strerror(errno));
        return 0;
    }
    fprintf(fp,
            "{\n"
            "  \"format\": \"london1940-c-png-json-png-roundtrip\",\n"
            "  \"version\": 1,\n"
            "  \"sourceRule\": \"Original source PNG map interior only; no stitching; no gzip JSON.\",\n"
            "  \"sheets\": [\n");
    for (i = 0; i < count; i++) {
        const sheet_result *result = &results[i];
        fprintf(fp,
                "    {\n"
                "      \"sheet\": \"%s\",\n"
                "      \"source\": \"%s\",\n"
                "      \"crop\": [%u, %u, %u, %u],\n"
                "      \"resolution\": [%u, %u],\n"
                "      \"json\": \"%s\",\n"
                "      \"jsonBytes\": %llu,\n"
                "      \"pngFromJson\": \"%s\",\n"
                "      \"pngBytes\": %llu,\n"
                "      \"reviewPng\": \"%s\",\n"
                "      \"meanAbsoluteChannelDelta\": %.6f,\n"
                "      \"maxChannelDelta\": %u\n"
                "    }%s\n",
                result->crop->name,
                result->source_path,
                result->crop->left,
                result->crop->top,
                result->crop->right,
                result->crop->bottom,
                result->width,
                result->height,
                result->json_path,
                result->json_bytes,
                result->png_path,
                result->png_bytes,
                result->review_path,
                result->mean_delta,
                result->max_delta,
                i + 1 == count ? "" : ",");
    }
    fprintf(fp, "  ]\n}\n");
    if (fclose(fp) != 0) {
        fprintf(stderr, "pngjson: unable to close manifest: %s\n", strerror(errno));
        return 0;
    }
    printf("manifest %s\n", manifest_path);
    return 1;
}

int main(int argc, char **argv)
{
    const char *sources_dir = NULL;
    const char *output_dir = NULL;
    const char *only_sheet = NULL;
    unsigned review_width = 2048;
    sheet_result results[sizeof(SHEET_CROPS) / sizeof(SHEET_CROPS[0])];
    size_t result_count = 0;
    size_t i;

    for (i = 1; i < (size_t)argc; i++) {
        if (strcmp(argv[i], "--sources") == 0 && i + 1 < (size_t)argc) {
            sources_dir = argv[++i];
        } else if (strcmp(argv[i], "--output-dir") == 0 && i + 1 < (size_t)argc) {
            output_dir = argv[++i];
        } else if (strcmp(argv[i], "--sheet") == 0 && i + 1 < (size_t)argc) {
            only_sheet = argv[++i];
        } else if (strcmp(argv[i], "--review-width") == 0 && i + 1 < (size_t)argc) {
            review_width = (unsigned)strtoul(argv[++i], NULL, 10);
        } else {
            usage(argv[0]);
            return 1;
        }
    }
    if (sources_dir == NULL || output_dir == NULL) {
        usage(argv[0]);
        return 1;
    }
    if (only_sheet != NULL && find_sheet(only_sheet) == NULL) {
        fprintf(stderr, "pngjson: unknown sheet %s\n", only_sheet);
        return 1;
    }
    if (!make_output_dir(output_dir)) {
        return 1;
    }
    for (i = 0; i < sizeof(SHEET_CROPS) / sizeof(SHEET_CROPS[0]); i++) {
        if (only_sheet != NULL && strcmp(only_sheet, SHEET_CROPS[i].name) != 0) {
            continue;
        }
        if (!roundtrip_sheet(sources_dir, output_dir, &SHEET_CROPS[i], review_width,
                             &results[result_count])) {
            return 1;
        }
        result_count++;
    }
    if (!write_manifest(output_dir, results, result_count)) {
        return 1;
    }
    return 0;
}
