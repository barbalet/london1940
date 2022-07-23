#include "map2json.h"
#include "toolkit.h"

n_int color_pixel_count(n_byte img[], n_c_int width, n_c_int height,
                        n_byte r, n_byte g, n_byte b)
{
    n_int n, pixels = 0;

    for (n = width*height*3 - 3; n >= 0; n -= 3) {
        if (r != img[n]) continue;
        if (g != img[n+1]) continue;
        if (b != img[n+2]) continue;
        pixels++;
    }
    return pixels;
}

void color_pixel_to_binary(n_byte img[], n_c_int width, n_c_int height,
                           n_byte r, n_byte g, n_byte b,
                           n_byte thresholded[], n_byte clear)
{
    n_int n, n2 = width*height - 1;

    if (clear != 0) memset(thresholded, BACKGROUND, width*height);

    for (n = width*height*3 - 3; n >= 0; n -= 3, n2--) {
        if (r != img[n]) continue;
        if (g != img[n+1]) continue;
        if (b != img[n+2]) continue;
        thresholded[n2] = 0;
    }
}

void detect_color(n_byte img[],
                  n_c_int width, n_c_int height, n_c_int bitsperpixel,
                  n_c_int primary_channel,
                  n_c_int red_low, n_c_int red_high,
                  n_c_int green_low, n_c_int green_high,
                  n_c_int blue_low, n_c_int blue_high,
                  n_c_int averaging_radius)
{
    n_c_int ch, x, y, xx, yy, n, suppress;
    n_c_int bytesperpixel = bitsperpixel/8;
    n_c_int av[3];
    n_c_int averaged_pixels;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            /* calculate average color */
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] = 0;
            }
            averaged_pixels = 0;
            for (yy = y - averaging_radius; yy <= y + averaging_radius; yy++) {
                if (yy < 0) continue;
                if (yy >= height) break;
                for (xx = x - averaging_radius; xx <= x + averaging_radius; xx++) {
                    if (xx < 0) continue;
                    if (xx >= width) break;
                    n = (yy*width + xx)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n++) {
                        av[ch] += (int)img[n];
                    }
                    averaged_pixels++;
                }
            }
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] /= averaged_pixels;
            }
            if (((av[0] < blue_low) || (av[0] > blue_high)) ||
                    ((av[1] < green_low) || (av[1] > green_high)) ||
                    ((av[2] < red_low) || (av[2] > red_high))) {
                n = (y*width + x)*bytesperpixel;
                for (ch = 0; ch < bytesperpixel; ch++, n++) {
                    img[n] = BACKGROUND;
                }
                continue;
            }

            suppress = 0;
            for (ch = 0; ch < bytesperpixel; ch++) {
                if (av[ch] < av[primary_channel]) {
                    suppress++;
                }
            }
            if (suppress != 2) {
                n = (y*width + x)*bytesperpixel;
                for (ch = 0; ch < bytesperpixel; ch++, n++) {
                    img[n] = BACKGROUND;
                }
            }

        }
    }
}


void detect_green(n_byte img[],
                  n_c_int width, n_c_int height, n_c_int bitsperpixel,
                  n_c_int threshold,
                  n_c_int averaging_radius)
{
    n_c_int ch, x, y, xx, yy, n, suppress;
    n_c_int bytesperpixel = bitsperpixel/8;
    n_c_int av[3];
    n_c_int averaged_pixels;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            /* calculate average color */
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] = 0;
            }
            averaged_pixels = 0;
            for (yy = y - averaging_radius; yy <= y + averaging_radius; yy++) {
                if (yy < 0) continue;
                if (yy >= height) break;
                for (xx = x - averaging_radius; xx <= x + averaging_radius; xx++) {
                    if (xx < 0) continue;
                    if (xx >= width) break;
                    n = (yy*width + xx)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n++) {
                        av[ch] += (int)img[n];
                    }
                    averaged_pixels++;
                }
            }
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] /= averaged_pixels;
            }
            if (av[1] - av[2] < threshold) {
                n = (y*width + x)*bytesperpixel;
                for (ch = 0; ch < bytesperpixel; ch++, n++) {
                    img[n] = BACKGROUND;
                }
                continue;
            }

            suppress = 0;
            for (ch = 0; ch < bytesperpixel; ch++) {
                if (av[ch] < av[1]) {
                    suppress++;
                }
            }
            if (suppress != 2) {
                n = (y*width + x)*bytesperpixel;
                for (ch = 0; ch < bytesperpixel; ch++, n++) {
                    img[n] = BACKGROUND;
                }
            }

        }
    }
}

void detect_orange(n_byte img[],
                   n_c_int width, n_c_int height, n_c_int bitsperpixel,
                   n_c_int threshold_red,
                   n_c_int threshold_green,
                   n_c_int averaging_radius,
                   n_c_int min_red, n_c_int max_red)
{
    n_c_int ch, x, y, xx, yy, n, suppress;
    n_c_int bytesperpixel = bitsperpixel/8;
    n_c_int av[3];
    n_c_int averaged_pixels;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            /* calculate average color */
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] = 0;
            }
            averaged_pixels = 0;
            for (yy = y - averaging_radius; yy <= y + averaging_radius; yy++) {
                if (yy < 0) continue;
                if (yy >= height) break;
                for (xx = x - averaging_radius; xx <= x + averaging_radius; xx++) {
                    if (xx < 0) continue;
                    if (xx >= width) break;
                    n = (yy*width + xx)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n++) {
                        av[ch] += (int)img[n];
                    }
                    averaged_pixels++;
                }
            }
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] /= averaged_pixels;
            }
            /* red should be greater than green. Green should be greater than blue */
            if ((av[0] > max_red) || (av[0] < min_red) ||
                    (av[0] - av[1] < threshold_red) ||
                    (av[1] - av[2] < threshold_green)) {
                n = (y*width + x)*bytesperpixel;
                for (ch = 0; ch < bytesperpixel; ch++, n++) {
                    img[n] = BACKGROUND;
                }
                continue;
            }

            suppress = 0;
            for (ch = 0; ch < bytesperpixel; ch++) {
                if (av[ch] < av[0]) {
                    suppress++;
                }
            }
            if (suppress != 2) {
                n = (y*width + x)*bytesperpixel;
                for (ch = 0; ch < bytesperpixel; ch++, n++) {
                    img[n] = BACKGROUND;
                }
            }

        }
    }
}

void detect_blue(n_byte img[],
                 n_c_int width, n_c_int height, n_c_int bitsperpixel,
                 n_c_int threshold_red,
                 n_c_int threshold_green,
                 n_c_int averaging_radius,
                 n_c_int min_blue, n_c_int max_blue)
{
    n_c_int ch, x, y, xx, yy, n, suppress;
    n_c_int bytesperpixel = bitsperpixel/8;
    n_c_int av[3];
    n_c_int averaged_pixels;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            /* calculate average color */
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] = 0;
            }
            averaged_pixels = 0;
            for (yy = y - averaging_radius; yy <= y + averaging_radius; yy++) {
                if (yy < 0) continue;
                if (yy >= height) break;
                for (xx = x - averaging_radius; xx <= x + averaging_radius; xx++) {
                    if (xx < 0) continue;
                    if (xx >= width) break;
                    n = (yy*width + xx)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n++) {
                        av[ch] += (int)img[n];
                    }
                    averaged_pixels++;
                }
            }
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] /= averaged_pixels;
            }
            /* blue should be greater than green and red */
            if ((av[2] > max_blue) || (av[2] < min_blue) ||
                    (av[2] - av[1] < threshold_green) ||
                    (av[1] - av[0] < threshold_red)) {
                n = (y*width + x)*bytesperpixel;
                for (ch = 0; ch < bytesperpixel; ch++, n++) {
                    img[n] = BACKGROUND;
                }
                continue;
            }

            suppress = 0;
            for (ch = 0; ch < bytesperpixel; ch++) {
                if (av[ch] < av[2]) {
                    suppress++;
                }
            }
            if (suppress != 2) {
                n = (y*width + x)*bytesperpixel;
                for (ch = 0; ch < bytesperpixel; ch++, n++) {
                    img[n] = BACKGROUND;
                }
            }

        }
    }
}

void detect_blue_green(n_byte img[],
                       n_c_int width, n_c_int height, n_c_int bitsperpixel,
                       n_c_int threshold_red,
                       n_c_int threshold_blue_green_diff,
                       n_c_int averaging_radius,
                       n_c_int min_blue, n_c_int max_blue,
                       n_byte result[])
{
    n_c_int ch, x, y, xx, yy, n;
    n_c_int bytesperpixel = bitsperpixel/8;
    n_c_int av[3];
    n_c_int averaged_pixels;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            /* calculate average color */
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] = 0;
            }
            averaged_pixels = 0;
            for (yy = y - averaging_radius; yy <= y + averaging_radius; yy++) {
                if (yy < 0) continue;
                if (yy >= height) break;
                for (xx = x - averaging_radius; xx <= x + averaging_radius; xx++) {
                    if (xx < 0) continue;
                    if (xx >= width) break;
                    n = (yy*width + xx)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n++) {
                        av[ch] += (int)img[n];
                    }
                    averaged_pixels++;
                }
            }
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] /= averaged_pixels;
            }
            /* blue should be similar to green and both
            should be greater than red */
            if ((av[2] < max_blue) && (av[2] > min_blue) &&
                    (abs(av[2] - av[1]) < threshold_blue_green_diff) &&
                    (av[2] - av[0] > threshold_red)) {
                n = y*width + x;
                result[n] = 0;
            }
        }
    }
}

void detect_red(n_byte img[],
                n_c_int width, n_c_int height, n_c_int bitsperpixel,
                n_c_int threshold_red,
                n_c_int threshold_green,
                n_c_int averaging_radius,
                n_c_int min_red, n_c_int max_red,
                n_c_int max_green)
{
    n_c_int ch, x, y, xx, yy, n, suppress;
    n_c_int bytesperpixel = bitsperpixel/8;
    n_c_int av[3];
    n_c_int averaged_pixels;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            /* calculate average color */
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] = 0;
            }
            averaged_pixels = 0;
            for (yy = y - averaging_radius; yy <= y + averaging_radius; yy++) {
                if (yy < 0) continue;
                if (yy >= height) break;
                for (xx = x - averaging_radius; xx <= x + averaging_radius; xx++) {
                    if (xx < 0) continue;
                    if (xx >= width) break;
                    n = (yy*width + xx)*bytesperpixel;
                    for (ch = 0; ch < bytesperpixel; ch++, n++) {
                        av[ch] += (int)img[n];
                    }
                    averaged_pixels++;
                }
            }
            for (ch = 0; ch < bytesperpixel; ch++) {
                av[ch] /= averaged_pixels;
            }
            /* red should be greater than green. Green should be similar to blue */
            if ((av[0] > max_red) || (av[0] < min_red) ||
                    (av[1] > max_green) ||
                    (av[2] > max_green) ||
                    (av[0] - av[1] < threshold_red) ||
                    (av[0] - av[2] < threshold_red) ||
                    (abs(av[1] - av[2]) > threshold_green)) {
                n = (y*width + x)*bytesperpixel;
                for (ch = 0; ch < bytesperpixel; ch++, n++) {
                    img[n] = BACKGROUND;
                }
                continue;
            }

            suppress = 0;
            for (ch = 0; ch < bytesperpixel; ch++) {
                if (av[ch] < av[0]) {
                    suppress++;
                }
            }
            if (suppress != 2) {
                n = (y*width + x)*bytesperpixel;
                for (ch = 0; ch < bytesperpixel; ch++, n++) {
                    img[n] = BACKGROUND;
                }
            }

        }
    }
}
