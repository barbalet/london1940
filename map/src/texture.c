#include "map2json.h"
#include "toolkit.h"

n_int detect_texture(n_byte img[], n_c_int width, n_c_int height,
                     n_c_int patch_size, n_c_int texture_threshold,
                     n_byte result [])
{
    n_c_int x, y, patch_texture, patch_pixels, patch_texture_percent;
    n_int n, xx, yy, y_max, x_max, y_max2, x_max2;
    n_int stride = width*3;
    n_c_int small_patch_size = patch_size/4;

    memset(result, 255, width*height*sizeof(n_byte));

    patch_pixels = patch_size * patch_size * 2;

    for (y = 0; y < height; y += small_patch_size) {
        y_max = (n_int)(y + patch_size - 1);
        if (y_max >= (n_int)height-1) y_max = (n_int)height - 2;
        for (x = 0; x < width; x += small_patch_size) {
            patch_texture = 0;
            x_max = (n_int)(x + patch_size - 1);
            if (x_max >= (n_int)width-1) x_max = (n_int)width - 2;
            /* horizontal texture */
            for (yy = y; yy < y_max; yy++) {
                for (xx = x; xx < x_max; xx++) {
                    n = (yy * (n_int)width + xx)*3;
                    if ((img[n] == 255) || (img[n+3] == 255)) {
                        if ((img[n] != 255) || (img[n+3] != 255)) {
                            patch_texture++;
                        }
                    }
                }
            }
            /* vertical texture */
            for (xx = x; xx < x_max; xx++) {
                for (yy = y; yy < y_max; yy++) {
                    n = (yy * (n_int)width + xx)*3;
                    if ((img[n] == 255) || (img[n+stride] == 255)) {
                        if ((img[n] != 255) || (img[n+stride] != 255)) {
                            patch_texture++;
                        }
                    }
                }
            }
            patch_texture_percent = patch_texture * 100 / patch_pixels;
            if (patch_texture_percent > texture_threshold) {
                y_max2 = (n_int)(y + small_patch_size);
                if (y_max2 >= (n_int)height) y_max2 = (n_int)height - 1;
                for (yy = y; yy < y_max2; yy++) {
                    x_max2 = (n_int)(x + small_patch_size);
                    if (x_max2 >= (n_int)width) x_max2 = (n_int)width - 1;
                    for (xx = x; xx < x_max2; xx++) {
                        n = yy * (n_int)width + xx;
                        result[n] = 0;
                    }
                }
            }
        }
    }
    return 0;
}
