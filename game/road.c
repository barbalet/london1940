/****************************************************************
 
 road.c
 
 =============================================================
 
 Copyright 1996-2016 Tom Barbalet. All rights reserved.
 
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

static noble_road     roads[9];


void road_init(void)
{
    n_int px = -2;
    n_int count = 0;
    while (px < 3)
    {
        noble_road * temp_road = &roads[count++];
        temp_road->points[0].x = 0 - (RESIDENCE_SPACE * 4) - 500;
        temp_road->points[0].y = (px * RESIDENCE_SPACE * 2) - 500;
        
        temp_road->points[1].x = 0 - (RESIDENCE_SPACE * 4) - 500;
        temp_road->points[1].y = (px * RESIDENCE_SPACE * 2)-300;
        
        temp_road->points[2].x = (RESIDENCE_SPACE * 4) - 300;
        temp_road->points[2].y = (px * RESIDENCE_SPACE * 2)-300;
        
        temp_road->points[3].x = (RESIDENCE_SPACE * 4) - 300;
        temp_road->points[3].y = (px * RESIDENCE_SPACE * 2)-500;
        px++;
    }
    
    px = -1;
    
    while (px < 2)
    {
        noble_road * temp_road = &roads[count++];
        temp_road->points[0].x = (px * RESIDENCE_SPACE * 4) - 500;
        temp_road->points[0].y = 0 - (RESIDENCE_SPACE * 4) - 500;
        
        temp_road->points[1].x = (px * RESIDENCE_SPACE * 4) - 300;
        temp_road->points[1].y = 0 - (RESIDENCE_SPACE * 4) - 500;
        
        temp_road->points[2].x = (px * RESIDENCE_SPACE * 4) - 300;
        temp_road->points[2].y = (RESIDENCE_SPACE * 4) - 300;
        
        temp_road->points[3].x = (px * RESIDENCE_SPACE * 4) - 500;
        temp_road->points[3].y = (RESIDENCE_SPACE * 4) - 300;
        
        px++;
    }
}

void road_draw_marking(noble_road * road)
{
    n_vect2 middle_line[2];
    n_vect2 partial_line;
    n_int   loop = 0;
    n_vect2 delta_stepper;
    
    vect2_center(&middle_line[0], &road->points[0], &road->points[1]);
    vect2_center(&middle_line[1], &road->points[2], &road->points[3]);
    
    vect2_divide(&partial_line, &middle_line[0], &middle_line[1], 256);
    vect2_copy(&delta_stepper, &middle_line[0]);
    
    gldraw_wide_line();
    gldraw_darkgrey();
    
    while (loop < 264)
    {
        if (loop & 1)
        {
            vect2_d(&delta_stepper, &partial_line, 1, 1);
        }
        else
        {
            n_vect2 line_origin;
            
            vect2_copy(&line_origin, &delta_stepper);
            vect2_d(&delta_stepper, &partial_line, 1, 1);

            gldraw_line(&line_origin, &delta_stepper);
        }
        loop++;
    }
    gldraw_thin_line();
}

void road_draw_each(noble_road * road)
{
   gldraw_black();
   gldraw_quads(&road->points[0], 1);
}

void road_draw(void)
{
    n_int px = 0;
    
    while (px < 9)
    {
        road_draw_each(&roads[px]);
        px++;
    }
    
    px = 0;
    
    while (px < 9)
    {
        road_draw_marking(&roads[px]);
        px++;
    }
}