#include "map2json.h"
#include "toolkit.h"

unsigned char * read_png_file(char * filename,
                              n_byte4 * width,
                              n_byte4 * height,
                              n_byte4 * bitsperpixel)
{
    unsigned error;
    unsigned char* image;
    unsigned w, h;
    error = lodepng_decode24_file(&image, &w, &h, filename);
    if (error) printf("read_png_file: error %u: %s\n", error, lodepng_error_text(error));

    *width = w;
    *height = h;
    *bitsperpixel=24;
    return image;
}

n_c_int write_png_file(char* filename,
                       n_byte4 width, n_byte4 height,
                       n_byte4 bitsperpixel,
                       unsigned char *buffer)
{
    unsigned error=1;
    n_byte4 i;
    unsigned char * image = buffer;

    if (bitsperpixel == 32) {
        error = lodepng_encode32_file(filename, image, width, height);
    }
    if (bitsperpixel == 24) {
        error = lodepng_encode24_file(filename, image, width, height);
    }
    if (bitsperpixel == 8) {
        image = (unsigned char*)malloc(width*height*3*sizeof(unsigned char));
        if (image) {
            for (i = (width*height) - 1; i >= 0; i--) {
                image[i*3] = buffer[i];
                image[i*3+1] = buffer[i];
                image[i*3+2] = buffer[i];
            }
            error = lodepng_encode24_file(filename, image, width, height);
            free(image);
        }
    }

    if (error) {
        printf("write_png_file: error %u: %s\n", error, lodepng_error_text(error));
        return -1;
    }
    return 0;
}
