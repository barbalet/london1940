/****************************************************************

 gui.h

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

#ifndef _GUI_H
#define _GUI_H

#include "../../apesdk/toolkit/shared.h"
#include "../../apesdk/script/script.h"
#include "../../apesdk/entity/entity.h"
#include "../../apesdk/universe/universe.h"

#include "../game/mushroom.h"

#undef MULTITOUCH_CONTROLS

#define	TERRAINWINDOW(alpha)   (alpha)
#define	VIEWWINDOW(alpha)      (alpha + TERRAIN_WINDOW_AREA)

#define IS_WINDOW_KIND(x,y)			(((x)>>(y))&1)

enum colour_type
{
    COLOUR_BLACK     =   (0),
    COLOUR_GREY      =   (252),
    COLOUR_YELLOW    =   (253),
    COLOUR_RED_DARK  =   (254),
    COLOUR_RED       =   (255)
};

#define	spot_colour(alpha,spx,spy,col)	alpha[((spx)|((spy)<<8))]=(col)

/*	Graphics Metrics */

/*	Icon Offset */
#define	ICONOFFSET					27


void draw_tree(simulated_tree * tree);

void draw_each_fence(simulated_fence * fence);

void draw_fence(simulated_fence * fences);

void draw_debug_number_point(n_vect2 * point, n_int number);

void draw_debug_number_fill(n_vect2 * points, n_int number);

void draw_each_path(simulated_path * path);

void draw_path(simulated_path_group * group);

void draw_park(simulated_park * park);

void draw_house(simulated_building * buildings);

void draw_twoblock(simulated_twoblock * twoblock);

void draw_neighborhood(void);

void draw_city(void);

void draw_game_color(n_byte2 * fit);

n_int draw_game_scene(n_int dim_x, n_int dim_y);

void draw_render(n_byte * buffer, n_int dim_x, n_int dim_y);

void draw_init(void);

void draw_close(void);

#endif /* _GUI_H */

