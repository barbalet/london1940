/****************************************************************
 
 fence.c
 
 =============================================================
 
 Copyright 1996-2017 Tom Barbalet. All rights reserved.
 
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

#include "mushroom.h"


void fence_draw_each(noble_fence * fence)
{
    gldraw_wide_line();
    gldraw_orange();
    gldraw_line(&fence->points[0], &fence->points[1]);
    gldraw_thin_line();
}

void fence_init(n_byte2 * seed, n_byte rotate, n_vect2 * location, noble_fence * fences)
{
    n_int count = 0;
    
    n_int px = 0;
    while (px < 2)
    {
        n_int py = 0;
        while (py < 1)
        {
            noble_fence * temp_fence_x = &fences[count++];
            
            n_int fence_wabble_x1 = (math_random(seed) % FENCE_WABBLE_SPACE) - (FENCE_WABBLE_SPACE / 2);
            n_int fence_wabble_y1 = (math_random(seed) % FENCE_WABBLE_SPACE) - (FENCE_WABBLE_SPACE / 2);
            n_int fence_wabble_x2 = (math_random(seed) % FENCE_WABBLE_SPACE) - (FENCE_WABBLE_SPACE / 2);
            n_int fence_wabble_y2 = (math_random(seed) % FENCE_WABBLE_SPACE) - (FENCE_WABBLE_SPACE / 2);
            
            if (rotate)
            {
                temp_fence_x->points[0].x = fence_wabble_x1 + location->x + (px * RESIDENCE_SPACE * 2) + 400;
                temp_fence_x->points[0].y = fence_wabble_y1 + location->y + (py * RESIDENCE_SPACE * 4) - 250;

                
                temp_fence_x->points[1].x = fence_wabble_x2 + location->x + (px * RESIDENCE_SPACE * 2) + 400;
                temp_fence_x->points[1].y = fence_wabble_y2 + location->y + (py * RESIDENCE_SPACE * 4) + 2650;
            }
            else
            {
                temp_fence_x->points[0].x = fence_wabble_x1 + location->x + (py * RESIDENCE_SPACE * 4) - 250;
                temp_fence_x->points[0].y = fence_wabble_y1 + location->y + (px * RESIDENCE_SPACE * 2) + 400;
                
                temp_fence_x->points[1].x = fence_wabble_x2 + location->x + (py * RESIDENCE_SPACE * 4) + 2650;
                temp_fence_x->points[1].y = fence_wabble_y2 + location->y + (px * RESIDENCE_SPACE * 2) + 400;
            }
            
            py++;
        }
        px++;
    }
    px = 0;
    while (px < 4)
    {
        n_int py = 0;
        while (py < 2)
        {
            if (((px+4) & 3) != 3)
            {
                noble_fence * temp_fence_x = &fences[count++];
            
                n_int fence_wabble_x1 = (math_random(seed) % FENCE_WABBLE_SPACE) - (FENCE_WABBLE_SPACE / 2);
                n_int fence_wabble_y1 = (math_random(seed) % FENCE_WABBLE_SPACE) - (FENCE_WABBLE_SPACE / 2);
                n_int fence_wabble_x2 = (math_random(seed) % FENCE_WABBLE_SPACE) - (FENCE_WABBLE_SPACE / 2);
                n_int fence_wabble_y2 = (math_random(seed) % FENCE_WABBLE_SPACE) - (FENCE_WABBLE_SPACE / 2);
                
                if (rotate)
                {
                    temp_fence_x->points[0].x = fence_wabble_x1 + location->x + (py * RESIDENCE_SPACE * 2) - 250;
                    temp_fence_x->points[0].y = fence_wabble_y1 + location->y + (px * RESIDENCE_SPACE * 1) + 400;

                    temp_fence_x->points[1].x = fence_wabble_x2 + location->x + (py * RESIDENCE_SPACE * 2) + 1050;
                    temp_fence_x->points[1].y = fence_wabble_y2 + location->y + (px * RESIDENCE_SPACE * 1) + 400;
                }
                else
                {
                    temp_fence_x->points[0].x = fence_wabble_x1 + location->x + (px * RESIDENCE_SPACE * 1) + 400;
                    temp_fence_x->points[0].y = fence_wabble_y1 + location->y + (py * RESIDENCE_SPACE * 2) - 250;
                    
                    temp_fence_x->points[1].x = fence_wabble_x2 + location->x + (px * RESIDENCE_SPACE * 1) + 400;
                    temp_fence_x->points[1].y = fence_wabble_y2 + location->y + (py * RESIDENCE_SPACE * 2) + 1050;
                }
            }
            py++;
        }
        px++;
    }
}

void fence_draw(noble_fence * fences)
{
    n_int count = 0;
    while (count < 8)
    {
        fence_draw_each(&fences[count++]);
    }
}
