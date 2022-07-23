#include "map2json.h"
#include "toolkit.h"

n_c_int skeleton_to_points(n_byte img[], n_c_int width, n_c_int height,
                           n_c_int bitsperpixel, n_c_int point_spacing,
                           n_c_int line_points[], n_c_int max_points,
                           n_c_int sample_radius)
{
    n_c_int x, y, xx, yy, n, av_x, av_y, hits;
    n_c_int no_of_line_points = 0;
    n_c_int offset = point_spacing/2;

    for (y = 0; y < height; y+=point_spacing) {
        for (x = 0; x < width; x+=point_spacing) {
            hits = 0;
            av_x = 0;
            av_y = 0;
            for (yy = y + offset - sample_radius; yy <= y + offset + sample_radius; yy++) {
                if (yy < 0) continue;
                if (yy >= height) break;
                for (xx = x + offset - sample_radius; xx <= x + offset + sample_radius; xx++) {
                    if (xx < 0) continue;
                    if (xx >= width) break;
                    n = (yy*width + xx)*3;
                    if (img[n] == BACKGROUND) continue;
                    av_x += xx;
                    av_y += yy;
                    hits++;
                }
            }
            if (hits == 0) continue;
            line_points[no_of_line_points*2] = av_x / hits;
            line_points[no_of_line_points*2+1] = av_y / hits;
            no_of_line_points++;
            if (no_of_line_points >= max_points) {
                return no_of_line_points;
            }
        }
    }
    return no_of_line_points;
}

static void skeletonize_stage1(n_byte img[], n_c_int width, n_c_int height,
                               n_byte result[], n_c_int bitsperpixel,
                               n_c_int max_line_width)
{
    n_c_int x, y, state, n, n2, ch;
    n_c_int start_pos = 0, end_pos, mid_pos;
    n_c_int bytesperpixel = bitsperpixel/8;

    memset(result, BACKGROUND, width*height*bytesperpixel*sizeof(n_byte));

    /* top down */
    for (x = 1; x < width - 1; x++) {
        state = 0;
        for (y = 0; y < height; y++) {
            n = y*width + x;
            if (state == 0) {
                if ((img[n] == 0) && (img[n-1] == 0) && (img[n+1] == 0)) {
                    state = 1;
                    start_pos = y;
                }
            }
            else {
                if ((img[n] != 0) && (img[n-1] != 0) && (img[n+1] != 0)) {
                    state = 0;
                    end_pos = y;
                    mid_pos = start_pos + ((end_pos - start_pos)/2);
                    n2 = (mid_pos*width + x)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n2++) {
                        result[n2] = 0;
                    }
                }
                else {
                    if (y - start_pos > max_line_width) {
                        state = 0;
                    }
                }
            }
        }
    }

    /* bottom up */
    for (x = width-2; x >= 1; x--) {
        state = 0;
        for (y = height-1; y >= 0; y--) {
            n = y*width + x;
            if (state == 0) {
                if ((img[n] == 0) && (img[n-1] == 0) && (img[n+1] == 0)) {
                    state = 1;
                    start_pos = y;
                }
            }
            else {
                if ((img[n] != 0) && (img[n-1] != 0) && (img[n+1] != 0)) {
                    state = 0;
                    end_pos = y;
                    mid_pos = start_pos + ((end_pos - start_pos)/2);
                    n2 = (mid_pos*width + x)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n2++) {
                        result[n2] = 0;
                    }
                }
                else {
                    if (start_pos - y > max_line_width) {
                        state = 0;
                    }
                }
            }
        }
    }

    /* right to left */
    for (y = 1; y < height-2; y++) {
        state = 0;
        for (x = width - 1; x >= 0; x--) {
            n = y*width + x;
            if (state == 0) {
                if ((img[n] == 0) && (img[n-width] == 0) && (img[n+width] == 0)) {
                    state = 1;
                    start_pos = x;
                }
            }
            else {
                if ((img[n] != 0) && (img[n-width] != 0) && (img[n+width] != 0)) {
                    state = 0;
                    end_pos = x;
                    mid_pos = start_pos + ((end_pos - start_pos)/2);
                    n2 = (y*width + mid_pos)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n2++) {
                        result[n2] = 0;
                    }
                }
                else {
                    if (start_pos - x > max_line_width) {
                        state = 0;
                    }
                }
            }
        }
    }

    /* left to right */
    for (y = 1; y < height-2; y++) {
        state = 0;
        for (x = 0; x < width; x++) {
            n = y*width + x;
            if (state == 0) {
                if ((img[n] == 0) && (img[n-width] == 0) && (img[n+width] == 0)) {
                    state = 1;
                    start_pos = x;
                }
            }
            else {
                if ((img[n] != 0) && (img[n-width] != 0) && (img[n+width] != 0)) {
                    state = 0;
                    end_pos = x;
                    mid_pos = start_pos + ((end_pos - start_pos)/2);
                    n2 = (y*width + mid_pos)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n2++) {
                        result[n2] = 0;
                    }
                }
                else {
                    if (x - start_pos > max_line_width) {
                        state = 0;
                    }
                }
            }
        }
    }
}

void skeletonize_simple(n_byte img[], n_c_int width, n_c_int height,
                        n_byte result[], n_c_int bitsperpixel,
                        n_c_int max_line_width)
{
    n_c_int x, y, state, n, n2, ch;
    n_c_int start_pos = 0, end_pos, mid_pos;
    n_c_int bytesperpixel = bitsperpixel/8;

    memset(result, BACKGROUND, width*height*bytesperpixel*sizeof(n_byte));

    /* top down */
    for (x = 0; x < width; x++) {
        state = 0;
        for (y = 0; y < height-4; y++) {
            n = y*width + x;
            if (state == 0) {
                if (img[n] == 0) {
                    state = 1;
                    start_pos = y;
                }
            }
            else {
                if ((img[n] != 0) && (img[n+width] != 0) &&
                        (img[n+(width*2)] != 0) && (img[n+(width*3)] != 0)) {
                    state = 0;
                    end_pos = y;
                    mid_pos = start_pos + ((end_pos - start_pos)/2);
                    n2 = (mid_pos*width + x)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n2++) {
                        result[n2] = 0;
                    }
                }
                else {
                    if (y - start_pos > max_line_width) {
                        state = 0;
                    }
                }
            }
        }
    }

    /* bottom up */
    for (x = width-1; x >= 0; x--) {
        state = 0;
        for (y = height-1; y >= 4; y--) {
            n = y*width + x;
            if (state == 0) {
                if (img[n] == 0) {
                    state = 1;
                    start_pos = y;
                }
            }
            else {
                if ((img[n] != 0) && (img[n-width] != 0) &&
                        (img[n-(width*2)] != 0) && (img[n-(width*2)] != 0)) {
                    state = 0;
                    end_pos = y;
                    mid_pos = start_pos + ((end_pos - start_pos)/2);
                    n2 = (mid_pos*width + x)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n2++) {
                        result[n2] = 0;
                    }
                }
                else {
                    if (start_pos - y > max_line_width) {
                        state = 0;
                    }
                }
            }
        }
    }

    /* right to left */
    for (y = 0; y < height-1; y++) {
        state = 0;
        for (x = width - 1; x >= 4; x--) {
            n = y*width + x;
            if (state == 0) {
                if (img[n] == 0) {
                    state = 1;
                    start_pos = x;
                }
            }
            else {
                if ((img[n] != 0) && (img[n-1] != 0) &&
                        (img[n-2] != 0) && (img[n-3] != 0)) {
                    state = 0;
                    end_pos = x;
                    mid_pos = start_pos + ((end_pos - start_pos)/2);
                    n2 = (y*width + mid_pos)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n2++) {
                        result[n2] = 0;
                    }
                }
                else {
                    if (start_pos - x > max_line_width) {
                        state = 0;
                    }
                }
            }
        }
    }

    /* left to right */
    for (y = 0; y < height-1; y++) {
        state = 0;
        for (x = 0; x < width-4; x++) {
            n = y*width + x;
            if (state == 0) {
                if (img[n] == 0) {
                    state = 1;
                    start_pos = x;
                }
            }
            else {
                if ((img[n] != 0) && (img[n+1] != 0) &&
                        (img[n+2] != 0) && (img[n+3] != 0)) {
                    state = 0;
                    end_pos = x;
                    mid_pos = start_pos + ((end_pos - start_pos)/2);
                    n2 = (y*width + mid_pos)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n2++) {
                        result[n2] = 0;
                    }
                }
                else {
                    if (x - start_pos > max_line_width) {
                        state = 0;
                    }
                }
            }
        }
    }
}

/* removes single pixels */
static void skeletonize_denoise(n_byte img[], n_c_int width, n_c_int height,
                                n_c_int bitsperpixel)
{
    n_c_int x,y,xx,yy,n,n2,ch;
    n_byte single_pixel;
    n_c_int bytesperpixel = bitsperpixel/8;

    for (y = 1; y < height-2; y++) {
        for (x = 1; x < width-2; x++) {
            n = (y*width + x)*3;
            if (img[n] == BACKGROUND) continue;
            single_pixel = 1;
            for (yy = y - 1; yy <= y + 1; yy++) {
                for (xx = x - 1; xx <= x + 1; xx++) {
                    if ((xx == x) && (yy == y)) continue;
                    n2 = (yy*width + xx)*3;
                    if (img[n2] != BACKGROUND) {
                        yy = y+2;
                        single_pixel = 0;
                        break;
                    }
                }
            }
            if (single_pixel == 1) {
                for (ch = 0; ch < bytesperpixel; ch++) {
                    img[n+ch] = BACKGROUND;
                }
            }
        }
    }
}

/* difference of gaussians approximation with binarized result */
static void skeletonize_dog(n_byte img[], n_c_int width, n_c_int height,
                            n_byte result[], n_c_int bitsperpixel,
                            n_c_int outer_radius, n_c_int inner_radius)
{
    n_c_int x, y, xx, yy, n, dx, dy, i;
    n_c_int outer_sum, inner_sum;
    n_c_int bytesperpixel = bitsperpixel/8;
    n_c_int inner_pixels = inner_radius * inner_radius;
    n_c_int outer_pixels = (outer_radius * outer_radius) - inner_pixels;
    n_c_int dog, max_dog = 1;

    memset(result, 0, width*height*bytesperpixel*sizeof(n_byte));

    /* nothing but a hound DoG */
    for (y = outer_radius; y < height-outer_radius-1; y++) {
        for (x = outer_radius; x < width-outer_radius-1; x++) {
            n = y*width + x;
            if (img[n] == BACKGROUND) continue;
            outer_sum = 0;
            inner_sum = 0;
            for (yy = y - outer_radius; yy <= y + outer_radius; yy++) {
                dy = abs(yy - y);
                for (xx = x - outer_radius; xx <= x + outer_radius; xx++) {
                    n = yy*width + xx;
                    if (img[n] != BACKGROUND) {
                        dx = abs(xx - x);
                        if ((dx < inner_radius) && (dy < inner_radius)) {
                            inner_sum++;
                        }
                        else {
                            outer_sum++;
                        }
                    }
                }
            }
            dog = ((inner_sum * outer_pixels) / inner_pixels) - outer_sum;
            if (dog < 0) {
                dog = 0;
            }
            if (dog > max_dog) {
                max_dog = dog;
            }
        }
    }

    for (y = outer_radius; y < height-outer_radius-1; y++) {
        for (x = outer_radius; x < width-outer_radius-1; x++) {
            n = y*width + x;
            outer_sum = 0;
            inner_sum = 0;
            for (yy = y - outer_radius; yy <= y + outer_radius; yy++) {
                dy = abs(yy - y);
                for (xx = x - outer_radius; xx <= x + outer_radius; xx++) {
                    n = yy*width + xx;
                    if (img[n] != BACKGROUND) {
                        dx = abs(xx - x);
                        if ((dx < inner_radius) && (dy < inner_radius)) {
                            inner_sum++;
                        }
                        else {
                            outer_sum++;
                        }
                    }
                }
            }
            dog = ((inner_sum * outer_pixels) / inner_pixels) - outer_sum;
            if (dog < 0) {
                dog = 0;
            }
            else {
                dog = dog * 255 / max_dog;
            }

            n = (y*width + x)*3;
            result[n] = dog;
            result[n+1] = dog;
            result[n+2] = dog;
        }
    }

    /* binarize */
    for (i = 0; i < width*height*bytesperpixel; i++) {
        if (result[i] == 0) {
            result[i] = BACKGROUND;
        }
        else {
            result[i] = 0;
        }
    }
}

void skeletonize(n_byte img[], n_c_int width, n_c_int height,
                 n_byte result[], n_c_int bitsperpixel,
                 n_c_int max_line_width)
{
    n_c_int i;
    n_c_int outer_radius = max_line_width/4;
    n_c_int inner_radius = max_line_width/8;

    if (inner_radius < 1) {
        inner_radius = 1;
        outer_radius = 2;
    }

    skeletonize_dog(img, width, height,
                    result, bitsperpixel,
                    outer_radius, inner_radius);

    for (i = 0; i < width*height; i++) {
        img[i] = result[i*3];
    }

    skeletonize_stage1(img, width, height, result, bitsperpixel,
                       max_line_width);

    skeletonize_denoise(result, width, height, bitsperpixel);

    for (i = 0; i < width*height; i++) {
        img[i] = result[i*3];
    }

    skeletonize_simple(img, width, height, result, bitsperpixel,
                       max_line_width);

    skeletonize_denoise(result, width, height, bitsperpixel);
}
