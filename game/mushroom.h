/****************************************************************
 
 mushroom.h
 
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
#ifdef	_WIN32
#include "../noble/noble.h"
#else
#include "noble.h"
#endif

#define POINTS_PER_ROOM             (16)
#define POINTS_PER_ROOM_STRUCTURE   (8)
#define MAX_ROOMS                   (8)
#define GENETICS_COUNT              (64)

#define POINTS_PER_ROAD             (4)
#define POINTS_PER_FENCE            (2)

#define POINTS_PER_TREE             (32)

#define TREE_SPACE                  (400)
#define RESIDENCE_SPACE             (800)

typedef struct{
    n_vect2  points[POINTS_PER_FENCE];
}noble_fence;

typedef struct{
    n_vect2  points[POINTS_PER_ROAD];
}noble_road;

typedef struct{
    n_vect2  points[POINTS_PER_ROOM];
    n_byte   window;
}noble_room;

typedef struct{
    n_int    points[POINTS_PER_TREE];
    n_int    inner[POINTS_PER_TREE];
    n_int    radius;
    n_vect2  center;
}noble_tree;

typedef struct{
    noble_room   room[MAX_ROOMS];
    n_int        house[GENETICS_COUNT];
    n_int        roomcount;
    n_byte       rotation;
}noble_building;

typedef struct{
    n_vect2 location;
    n_vect2 location_delta;
    n_int   facing;
    n_int   facing_delta;
    n_int   zooming;
}noble_agent;

void game_init(n_byte2 * seed);
void game_draw_scene(n_int dim_x, n_int dim_y);


void house_vertex(n_vect2 * point);

void enemy_init(void);
void enemy_move(void);

n_int ecomony_init(n_byte2 * seeds);
void  economy_draw(n_int px, n_int py);

void road_init(void);
void road_draw(void);

void fence_init(void);
void fence_draw(void);

void house_init(n_byte2 * seed);
void house_draw(void);

void tree_init(n_byte2 * seed);
void tree_draw(void);

void boy_init(void);

n_vect2 * boy_location(void);

n_int boy_facing(void);

void boy_zoom(n_int zoom);
n_int boy_zooming(void);

void boy_turn(n_int delta);
void boy_move(n_int forwards);
void boy_cycle(void);

void offset_map(n_vect2 * offset, n_vect2 * point);

