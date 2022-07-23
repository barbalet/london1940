#include "map2json.h"
#include "toolkit.h"

/* convert an image from mono to color */
void mono_to_color(n_byte * img, n_c_int width, n_c_int height,
                   n_c_int bitsperpixel,
                   n_byte * color)
{
    n_c_int i, ch;
    n_c_int bytesperpixel = bitsperpixel/8;

    for (i = (width*height) - 1; i >= 0; i--)
        for (ch = 0; ch < bytesperpixel; ch++)
            color[i*bytesperpixel + ch] = img[i];
}

/* convert an image from color to mono */
void color_to_mono(n_byte * img, n_c_int width, n_c_int height,
                   n_c_int bitsperpixel,
                   n_byte * mono)
{
    n_c_int i, ch, v;
    n_c_int bytesperpixel = bitsperpixel/8;

    for (i = (width*height) - 1; i >= 0; i--) {
        v = 0;
        for (ch = 0; ch < bytesperpixel; ch++) {
            v += img[i*bytesperpixel + ch];
        }
        mono[i] = v/bytesperpixel;
    }
}

void color_to_binary(n_byte * img, n_c_int width, n_c_int height,
                     n_c_int bitsperpixel, n_c_int threshold,
                     n_byte * mono)
{
    n_c_int i, ch, v;
    n_c_int bytesperpixel = bitsperpixel/8;

    for (i = (width*height) - 1; i >= 0; i--) {
        v = 0;
        for (ch = 0; ch < bytesperpixel; ch++) {
            v += img[i*bytesperpixel + ch];
        }
        if (v/bytesperpixel > threshold)
            mono[i] = BACKGROUND;
        else
            mono[i] = 0;
    }
}
