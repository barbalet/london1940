/****************************************************************
 
 mushroom.h
 
 =============================================================
 
 Copyright 1996-2015 Tom Barbalet. All rights reserved.
 
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
    noble_room   room[MAX_ROOMS];
    n_int        house[GENETICS_COUNT];
    n_int        roomcount;
}noble_building;


typedef struct{
    n_vect2 location;
    n_vect2 location_delta;
    n_int   facing;
    n_int   facing_delta;
}noble_agent;

extern void draw_line(n_int x1, n_int y1, n_int x2, n_int y2);

void enemy_init(void);
void enemy_move(void);
noble_building * house_create(n_byte2 * seed);
void house_transform(noble_building * building, n_vect2 * center, n_int direction);
void house_draw(noble_building * building);

n_int ecomony_init(n_byte2 * seeds);
void  economy_draw(n_int px, n_int py);

void house_draw_scene(void);



void boy_init(void);

n_vect2 * boy_location(void);
n_vect2 * boy_location_delta(void);

n_int boy_facing(void);
n_int boy_turn_delta(void);
void boy_turn(n_int delta);

void boy_move(n_int forwards);

void boy_cycle(void);
