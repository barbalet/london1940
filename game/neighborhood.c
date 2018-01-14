/****************************************************************
 
 neighborhood.c
 
 =============================================================
 
 Copyright 1996-2018 Tom Barbalet. All rights reserved.
 
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
 
 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.
 
 ****************************************************************/

#include "city.h"
#include "mushroom.h"

noble_twoblock twoblock[64 - 8];
noble_park     park[8];
noble_fence    fences[4];

void neighborhood_init(n_byte2 * seed, n_vect2 * location)
{
    n_int   park_count = 0;
    n_int   twoblock_count = 0;
    n_int   py = -4;
    while (py < 4)
    {
        n_int px = -4;
        while (px < 4)
        {
            n_vect2 additional;
            n_byte2 park_probabilty = math_random(seed) & 255;
            additional.x = location->x + (px * 3400);
            additional.y = location->y + (py * 3400);
            
            if (twoblock_count == (64-8))
            {
                park_probabilty = 255;
            }
            if (park_count == 8)
            {
                park_probabilty = 0;
            }
            
            
            if (park_probabilty > 207)
            {
                park_init(seed, &additional, &park[park_count++]);
            }
            else
            {
                twoblock_init(seed, &additional, &twoblock[twoblock_count++]);
            }
            px++;
        }
        py++;
    }

    fences[0].points[0].x = CITY_BOTTOM_LEFT_X;
    fences[0].points[0].y = CITY_BOTTOM_LEFT_Y;
    fences[0].points[1].x = CITY_TOP_RIGHT_X;
    fences[0].points[1].y = CITY_BOTTOM_LEFT_Y;

    fences[1].points[0].x = CITY_TOP_RIGHT_X;
    fences[1].points[0].y = CITY_BOTTOM_LEFT_Y;
    fences[1].points[1].x = CITY_TOP_RIGHT_X;
    fences[1].points[1].y = CITY_TOP_RIGHT_Y;
    
    fences[2].points[0].x = CITY_TOP_RIGHT_X;
    fences[2].points[0].y = CITY_TOP_RIGHT_Y;
    fences[2].points[1].x = CITY_BOTTOM_LEFT_X;
    fences[2].points[1].y = CITY_TOP_RIGHT_Y;

    fences[3].points[0].x = CITY_BOTTOM_LEFT_X;
    fences[3].points[0].y = CITY_TOP_RIGHT_Y;
    fences[3].points[1].x = CITY_BOTTOM_LEFT_X;
    fences[3].points[1].y = CITY_BOTTOM_LEFT_Y;
}

void neighborhood_draw(void)
{
    n_int   count = 0;
    while (count < (48+8))
    {
        twoblock_draw(&twoblock[count++]);
    }
    count = 0;
    while (count < 8)
    {
        park_draw(&park[count++]);
    }
    
    fence_draw(&fences[0]);
    fence_draw(&fences[1]);
    fence_draw(&fences[2]);
    fence_draw(&fences[3]);
}
