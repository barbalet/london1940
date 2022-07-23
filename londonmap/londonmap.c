/****************************************************************

 londonmap.c

 =============================================================

 Copyright 1996-2022 Tom Barbalet. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/

/*
 
 160.png "NW"
 161.png "NE"
 170.png "SW"
 171.png "SE"
 
 11100 × 13500

 */


#define STANDARD_WIDTH  (11100)
#define STANDARD_HEIGHT (13500)

#define HALF_WIDTH   (STANDARD_WIDTH/2)
#define HALF_HEIGHT  (STANDARD_HEIGHT/2)

#ifdef MAC_MAP_LOCATION



#define FILE_NW "/Users/barbalet/gitlab/london1940/newlondmaps/160.png"
#define FILE_NE "/Users/barbalet/gitlab/london1940/newlondmaps/161.png"
#define FILE_SW "/Users/barbalet/gitlab/london1940/newlondmaps/170.png"
#define FILE_SE "/Users/barbalet/gitlab/london1940/newlondmaps/171.png"

#define FILE_NW_C "/Users/barbalet/output/160c.png"
#define FILE_NE_C "/Users/barbalet/output/161c.png"
#define FILE_SW_C "/Users/barbalet/output/170c.png"
#define FILE_SE_C "/Users/barbalet/output/171c.png"

#define FILE_NW_SMALL "/Users/barbalet/output/160_small.png"
#define FILE_NE_SMALL "/Users/barbalet/output/161_small.png"
#define FILE_SW_SMALL "/Users/barbalet/output/170_small.png"
#define FILE_SE_SMALL "/Users/barbalet/output/171_small.png"

#else

#define FILE_NW "../newlondmaps/160.png"
#define FILE_NE "../newlondmaps/161.png"
#define FILE_SW "../newlondmaps/170.png"
#define FILE_SE "../newlondmaps/171.png"

#define FILE_NW_C "160c.png"
#define FILE_NE_C "161c.png"
#define FILE_SW_C "170c.png"
#define FILE_SE_C "171c.png"

#define FILE_NW_SMALL "160_small.png"
#define FILE_NE_SMALL "161_small.png"
#define FILE_SW_SMALL "170_small.png"
#define FILE_SE_SMALL "171_small.png"

#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "png/pnglite.h"

extern unsigned char * read_png_file(char * filename, png_t * ptr);
extern int write_png_file(char* filename, int width, int height, unsigned char *buffer);

unsigned long png_point(int x, int y, int width, int color);

void london_sin_cos(double * sin_cos, double theta)
{
    sin_cos[0] = sin(theta);
    sin_cos[1] = cos(theta);
}


void london_rotation(double * sin_cos, int *x, int *y)
{
    double dx = (*x * 2.000);
    double dy = (*y * 2.000);
    
    double resultx = (sin_cos[1]*dx) + (sin_cos[0]*dy);
    double resulty = (sin_cos[1]*dy) - (sin_cos[0]*dx);
    
    *x = (int) resultx;
    *y = (int) resulty;
}

void coverage_create(unsigned char * coverage, int low, int high)
{
    int loop = 0;
    while (loop < 256)
    {
        int value = 0;
        if (loop > high)
        {
            value = 255;
        }else if (loop > low)
        {
            value = (255*(loop - low))/(high - low);
        }
        coverage[loop] = value;
        loop++;
    }
}

void coverage_map(  char * file_in,
                    char * file_out,
                    unsigned char * coverage0,
                    unsigned char * coverage1,
                    unsigned char * coverage2)
{
    int loopy = 0;
    png_t local_ptr;
    
    printf("coverage_map %s -> %s\n", file_in, file_out);
    
    unsigned char * old_map = read_png_file(file_in, &local_ptr);
    while (loopy < STANDARD_HEIGHT)
    {
        int loopx = 0;
        while (loopx < STANDARD_WIDTH)
        {
            unsigned long local_point = png_point(loopx, loopy, STANDARD_WIDTH, 0);
            old_map[local_point] = coverage0[old_map[local_point]];
            
            local_point = png_point(loopx, loopy, STANDARD_WIDTH, 1);
            old_map[local_point] = coverage1[old_map[local_point]];

            local_point = png_point(loopx, loopy, STANDARD_WIDTH, 3);
            old_map[local_point] = coverage2[old_map[local_point]];
            loopx++;
        }
        loopy++;
    }
    
    (void)write_png_file(file_out, STANDARD_WIDTH, STANDARD_HEIGHT, old_map);
}

void histogram_clear(unsigned long * local_hist)
{
    unsigned long loop = 0;
    while (loop < (256 * 3))
    {
        local_hist[loop] = 0;
        loop++;
    }
}

 


void histogram_run(unsigned long * local_hist, unsigned char * buffer, int width, int height)
{
    int loopy = 0;
    while (loopy < height)
    {
        int loopx = 0;
        while (loopx < width)
        {
            local_hist[buffer[png_point(loopx, loopy, width, 0)]]++;
            local_hist[buffer[png_point(loopx, loopy, width, 1)] + 256]++;
            local_hist[buffer[png_point(loopx, loopy, width, 2)] + 512]++;
            loopx++;
        }
        loopy++;
    }
}

void    histogram_top_bottom_highest(unsigned long * local_hist,
            int * top, int * bottom, int * highest_btt, int * highest_ttb)
{
    int local_top = -1;
    int local_bottom = -1;
    int local_highest_btt = -1;
    long local_highest_value_btt = -1;
    int local_highest_ttb = -1;
    long local_highest_value_ttb = -1;
    int loop = 0;
    while (loop < 256)
    {
        long value = local_hist[loop];
        long value_ttb = local_hist[255 - loop];
        if ((value != 0) && (local_bottom == -1))
        {
            local_bottom = loop;
        }
        if ((local_bottom != -1) && (value != 0))
        {
            local_top = loop;
        }
        if (value > local_highest_value_btt)
        {
            local_highest_value_btt = value;
            local_highest_btt = loop;
        }
        if (value_ttb > local_highest_value_ttb)
        {
            local_highest_value_ttb = value_ttb;
            local_highest_ttb = 255 - loop;
        }
        loop++;
    }
    * top = local_top;
    * bottom = local_bottom;
    * highest_btt = local_highest_btt;
    * highest_ttb = local_highest_ttb;
    
}

unsigned long big_point_buffer(int x, int y, int color, unsigned char * big_buffer, double * sin_cos, double * sin_cos2)
{
    unsigned long value = 0;
    unsigned long location;
    int rotated_x = x;
    int rotated_y = y;
    
    london_rotation(sin_cos, &rotated_x, &rotated_y);
    
    location = (rotated_x) + ((rotated_y) * STANDARD_WIDTH);
     
    if ((location < 0) || (location >= (STANDARD_WIDTH*STANDARD_HEIGHT)))
    {
        value += 0;
    }
    else
    {
        location = location * 3;
        value +=  2*big_buffer[location+color];
    }

    rotated_x = x;
    rotated_y = y;
    
    london_rotation(sin_cos2, &rotated_x, &rotated_y);
    
    location = (rotated_x) + ((rotated_y) * STANDARD_WIDTH);
    if ((location < 0) || (location >= (STANDARD_WIDTH*STANDARD_HEIGHT)))
    {
        value += 0;
    }
    else
    {
        location = location * 3;
        value +=  big_buffer[location+color];
    }

    return value / 3;
}

unsigned long png_point(int x, int y, int width, int color)
{
    return ((x + (y * width))*3)+color;
}

void png_scrink(char * file_in, char * file_out, png_t * local_png, unsigned char * buffer,
               int start_x, int start_y, int width, int height, double theta, double theta2)
{
    unsigned char * big_file = read_png_file(file_in, local_png);
    int loopy = 0;
    printf("png_scrink(%s, %s);\n", file_in, file_out);
    
    double sin_cos[2], sin_cos2[2];
    
    london_sin_cos(sin_cos, theta);
    london_sin_cos(sin_cos2, theta2);

    while (loopy < height)
    {
        int loopx = 0;
        while (loopx < width)
        {
            unsigned char p0 = big_point_buffer(loopx + start_x, loopy + start_y, 0, big_file, sin_cos, sin_cos2);
            unsigned char p1 = big_point_buffer(loopx + start_x, loopy + start_y, 1, big_file, sin_cos, sin_cos2);
            unsigned char p2 = big_point_buffer(loopx + start_x, loopy + start_y, 2, big_file, sin_cos, sin_cos2);
            /* remove the grid addition */
            int black_line = 0;

            int divisions = 0;
            while (divisions < 40)
            {
                if (loopx == ((divisions * width)/40))
                {
                    black_line = 1;
                }
                divisions++;
            }
            
            divisions = 0;
            while (divisions < 45)
            {
                if (loopy == ((divisions * height)/45))
                {
                    black_line = 1;
                }
                divisions++;
            }
                
            if (black_line)
            {
                p0 = 255;
                p1 = 0;
                p2 = 0;
            }

            buffer[png_point(loopx, loopy, width, 0)] = p0;
            buffer[png_point(loopx, loopy, width, 1)] = p1;
            buffer[png_point(loopx, loopy, width, 2)] = p2;

            loopx++;
        }
        loopy++;
    }
    (void) write_png_file(file_out, width, height, buffer);
}

void png_histogram(char * file_in, png_t * local_png)
{
    unsigned long histogram[256 * 3];

    unsigned char * big_file = read_png_file(file_in, local_png);

    int top, bottom, highest_btt, highest_ttb;
    
    histogram_clear(histogram);
    histogram_run(histogram, big_file, STANDARD_WIDTH, STANDARD_HEIGHT);
    histogram_top_bottom_highest(histogram, &top, &bottom, &highest_btt, &highest_ttb);

    if (highest_btt == highest_ttb)
    {
        printf("%s, #0, %d - %d high %d\n", file_in, bottom, top, highest_btt);
    }
    else
    {
        printf("not ideal\n");
    }

    histogram_top_bottom_highest(&histogram[256], &top, &bottom, &highest_btt, &highest_ttb);

    if (highest_btt == highest_ttb)
    {
        printf("%s, #1, %d - %d high %d\n", file_in, bottom, top, highest_btt);
    }
    else
    {
        printf("not ideal\n");
    }
    histogram_top_bottom_highest(&histogram[512], &top, &bottom, &highest_btt, &highest_ttb);

    if (highest_btt == highest_ttb)
    {
        printf("%s, #2, %d - %d high %d\n\n", file_in, bottom, top, highest_btt);
    }
    else
    {
        printf("not ideal\n");
    }
}

void coverage(char * name, unsigned char * coverage)
{
    printf("name : %s\n\n", name);
    int loop = 0;
    while (loop < 256)
    {
        printf("%d : %d\n", loop, coverage[loop]);
        loop++;
    }
    printf("\n\n");

}

/*
 
 #A - 16 - 248 high 240
 #B -  8 - 232 high 216
 #C -  0 - 240 high 184
 #D - 16 - 248 high 248
 #E -  8 - 232 high 232
 #F -  0 - 240 high 240

 /Users/barbalet/londonmap/londonmap/londonpng/160.png, #0, 16 - 248 high 240(A)
 /Users/barbalet/londonmap/londonmap/londonpng/160.png, #1, 8 - 232 high 216 (B)
 /Users/barbalet/londonmap/londonmap/londonpng/160.png, #2, 0 - 240 high 184 (C)

 /Users/barbalet/londonmap/londonmap/londonpng/161.png, #0, 16 - 248 high 248(D)
 /Users/barbalet/londonmap/londonmap/londonpng/161.png, #1, 8 - 232 high 232 (E)
 /Users/barbalet/londonmap/londonmap/londonpng/161.png, #2, 0 - 240 high 240 (F)

 /Users/barbalet/londonmap/londonmap/londonpng/170.png, #0, 16 - 248 high 240(A)
 /Users/barbalet/londonmap/londonmap/londonpng/170.png, #1, 8 - 232 high 216 (B)
 /Users/barbalet/londonmap/londonmap/londonpng/170.png, #2, 0 - 240 high 184 (C)

 /Users/barbalet/londonmap/londonmap/londonpng/171.png, #0, 16 - 248 high 248(D)
 /Users/barbalet/londonmap/londonmap/londonpng/171.png, #1, 8 - 232 high 232 (E)
 /Users/barbalet/londonmap/londonmap/londonpng/171.png, #2, 0 - 240 high 240 (F)
 
 */

#define XSPEC1 (80)
#define YSPEC1 (80)

#define XSPEC2 (94)
#define YSPEC2 (196)

#define XSPEC3 (178)
#define YSPEC3 (178)
#define XSPEC4 (118)
#define YSPEC4 (165)

#define XSTART (150)
#define YSTART (300)

#define XEND (5550-300-288)
#define YEND (6750-800-382)

//(void)png_scrink(FILE_NW, FILE_NW_SMALL, &local_png, bitmap, 150+120 + 6 + 2, 300+170, HALF_WIDTH - 300-272 - 9, HALF_HEIGHT - 800 - 352);

int main(int argc, const char * argv[]) {

    
    png_t local_png;

    unsigned char * bitmap = (unsigned char *) malloc(HALF_WIDTH * HALF_HEIGHT * 3);

    png_scrink(FILE_NW, FILE_NW_SMALL, &local_png, bitmap, XSTART+XSPEC1, YSTART+YSPEC1, XEND, YEND, 0.001, 0.002);
    
    // works
    //png_scrink(FILE_NE, FILE_NE_SMALL, &local_png, bitmap, XSTART+XSPEC2, YSTART+YSPEC2, XEND, YEND, 0.00, 0.00);
    
    //png_scrink(FILE_SW, FILE_SW_SMALL, &local_png, bitmap, XSTART+XSPEC3, YSTART+YSPEC3, XEND, YEND, 0.00, 0.01);
    //png_scrink(FILE_SE, FILE_SE_SMALL, &local_png, bitmap, XSTART+XSPEC4, YSTART+YSPEC4, XEND, YEND, 0.00, 0.01);

    free(bitmap);

    return 0;
}
