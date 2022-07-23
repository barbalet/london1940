#include "map2json.h"
#include "toolkit.h"

/**
 * @brief Draws a line of the given width and color
 * @param img containing image
 * @param width Width of the image
 * @param height Height of the image
 * @param bitsperpixel Number of bits per pixel
 * @param tx top left of the line
 * @param ty top of the line
 * @param bx bottom right of the line
 * @param by bottom of the line
 * @param line_width width of the line
 * @param r Red
 * @param g Green
 * @param b Blue
 */
void draw_line(n_byte img[],
               n_byte4 width, n_byte4 height,
               n_c_int bitsperpixel,
               n_c_int tx, n_c_int ty, n_c_int bx, n_c_int by,
               n_c_int line_width,
               n_c_int r, n_c_int g, n_c_int b)
{
    n_c_int linewidth = bx - tx;
    n_c_int lineheight = by - ty;
    n_c_int x, y, xx, yy, incr=1;
    n_c_int bytes_per_pixel = bitsperpixel/8;
    n_c_int half_width = line_width/2;
    n_int n;

    if (abs(lineheight) > abs(linewidth)) {
        /* vertical orientation */
        if (by < ty) incr = -1;
        for (y = ty; y != by; y+=incr) {
            if ((y < 0) || (y >= (int)height))
                continue;
            x = tx + ((y - ty) * linewidth / lineheight);
            for (xx = x - half_width; xx < x - half_width + line_width; xx++) {
                if ((xx < 0) || (xx >= (int)width))
                    continue;
                n = (y * width + xx) * bytes_per_pixel;
                if (bytes_per_pixel == 3) {
                    img[n] = b;
                    img[n+1] = g;
                    img[n+2] = r;
                }
                else {
                    img[n] = r;
                }
            }
        }
    }
    else {
        /* horizontal orientation */
        if (bx < tx) incr = -1;
        for (x = tx; x != bx; x+=incr) {
            if ((x < 0) || (x >= (int)width))
                continue;
            y = ty + ((x - tx) * lineheight / linewidth);
            for (yy = y - half_width; yy < y - half_width + line_width; yy++) {
                if ((yy < 0) || (yy >= (int)height))
                    continue;
                n = (yy * width + x) * bytes_per_pixel;
                if (bytes_per_pixel == 3) {
                    img[n] = b;
                    img[n+1] = g;
                    img[n+2] = r;
                }
                else {
                    img[n] = r;
                }
            }
        }
    }
}

void draw_point(n_byte img[],
                n_byte4 width, n_byte4 height,
                n_c_int x, n_c_int y,
                n_c_int point_radius,
                n_c_int r, n_c_int g, n_c_int b)
{
    n_int n, xx, yy, dx, dy;
    n_int radius2 = point_radius * point_radius;

    for (yy = y - point_radius; yy <= y + point_radius; yy++) {
        for (xx = x - point_radius; xx <= x + point_radius; xx++) {
            if ((yy < 0) || (xx < 0) || (yy >= (int)height) || (xx >= (int)width)) {
                continue;
            }
            dx = xx - x;
            dy = yy - y;
            if (dx*dx + dy*dy < radius2) {
                n = (yy*(int)width + xx)*3;
                img[n] = b;
                img[n+1] = g;
                img[n+2] = r;
            }
        }
    }
}
