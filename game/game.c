/****************************************************************
 
 game.c
 
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

void game_init(n_byte2 * seed)
{
    house_init(seed);
    road_init();
    fence_init();
}

static void game_grid(void)
{
    n_int loop = -4000;
    n_vect2 horizontal_start = {-4000, -4000}, horizontal_end = {-4000, 4000};
    n_vect2 vertical_start = {-4000, -4000}, vertical_end = {4000, -4000};
    gldraw_black();
    gldraw_thin_line();
    while (loop < 4000)
    {
        horizontal_start.x = loop;
        horizontal_end.x = loop;
        vertical_start.y = loop;
        vertical_end.y = loop;
        gldraw_line(&horizontal_start, &horizontal_end);
        gldraw_line(&vertical_start, &vertical_end);
        
        loop += 100;
    }
}

void game_draw_scene(n_int dim_x, n_int dim_y)
{
    gldraw_background_green();
    if (gldraw_scene_done())
    {        
        gldraw_start_display_list();
        house_draw();
        road_draw();
        
        fence_draw();
        game_grid();
        gldraw_end_display_list();
    }
    else
    {
        n_vect2 center;
        center.x = 0 - (dim_x >> 1);
        center.y = 0 - (dim_y >> 1);
        
        gldraw_delta_move(&center, boy_location(), boy_facing(), boy_zooming());
    }
    gldraw_display_list();
}

