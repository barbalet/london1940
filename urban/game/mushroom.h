/****************************************************************
 
 mushroom.h
 
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
#ifndef _MUSHROOM_H
#define _MUSHROOM_H

#include "../../apesdk/toolkit/toolkit.h"
#include "../../apesdk/sim/sim.h"

#undef DEBUG_BLOCKING_BOUNDARIES

#define DEBUG_ROOM_NUMBER
#define DEBUG_ROAD_NUMBER

enum direction_constant
{
    DC_NONE         = 0,
    DC_NORTH        = 1,
    DC_SOUTH        = 2,
    DC_EAST         = 4,
    DC_WEST         = 8
};

#define POINTS_PER_ROOM             (32)
#define POINTS_PER_ROOM_STRUCTURE   (8)
#define MAX_ROOMS                   (8)
#define GENETICS_COUNT              (64)

#define POINTS_PER_PATH             (4)
#define PATHS_PER_GROUP             (5)

#define POINTS_PER_FOOTPATH         (4)

#define POINTS_PER_ROAD             (4)
#define POINTS_PER_FENCE            (2)

#define POINTS_PER_TREE             (32)

#define TREE_SPACE                  (400)
#define RESIDENCE_SPACE             (800)

#define FENCE_WABBLE_SPACE          (100)

#define FENCE_OFFSET      (400)
#define FENCE_START       (-250)
#define FENCE_END_LONG    ((RESIDENCE_SPACE * 3) + 250)
#define FENCE_END_SHORT   ((RESIDENCE_SPACE * 1) + 250)

#define ROAD_WIDTH              (25)


typedef struct{
    n_vect2  points[POINTS_PER_FENCE];
}simulated_fence;

typedef struct{
    n_vect2  points[POINTS_PER_PATH];
}simulated_path;

typedef struct{
    simulated_path  paths[PATHS_PER_GROUP];
    n_int           number;
}simulated_path_group;

typedef struct{
    n_vect2  points[4];
    n_string location;
}simulated_door;

typedef struct{
    n_vect2  points[2];
    n_string location;
}simulated_window;

typedef union
{
        n_vect2  points[POINTS_PER_ROOM];
}simulated_room;

typedef struct{
    n_int    points[POINTS_PER_TREE];
    n_int    radius;
    n_vect2  center;
}simulated_tree;

typedef struct{
    simulated_room   room[MAX_ROOMS];
    n_int            house[GENETICS_COUNT];
    n_int            roomcount;
    n_byte           rotation;
    simulated_tree   trees[4];
}simulated_building;

typedef struct{
    simulated_path_group   road;
    simulated_tree         trees[16][4];
}simulated_park;

typedef struct{
    simulated_building   house[16];
    simulated_fence      fence[8];
    simulated_path_group footpath;
    simulated_path_group road;
    n_byte               rotation;
}simulated_twoblock;

typedef struct{
    n_vect2 location;
    n_vect2 location_delta;
    n_int   facing;
    n_int   facing_delta;
    n_int   zooming;
}simulated_agent;

typedef struct{
    n_vect2 start;
    n_vect2 end;
#ifdef DEBUG_BLOCKING_BOUNDARIES
    n_byte  color;
    n_byte  thickness;
#endif
}matrix_plane;

void park_init(n_byte2 * seed, n_vect2 * location, simulated_park * parks);
void park_draw(simulated_park * park);

void neighborhood_init(n_byte2 * seed);
void draw_neighborhood(void);

void twoblock_init(n_byte2 * seed, n_vect2 * location, simulated_twoblock * twoblock);
void draw_twoblock(simulated_twoblock * twoblock);

void draw_city(void);

n_int house_window_present(n_vect2 * window);
n_int house_door_present(n_vect2 * door);

n_int draw_game_scene(n_int dim_x, n_int dim_y);
void draw_render(n_byte * buffer, n_int dim_x, n_int dim_y);
void draw_init(void);
void draw_close(void);

void draw_game_color(n_byte2 * fit);

void house_vertex(n_vect2 * point);

n_int ecomony_init(n_byte2 * seeds);
void  economy_draw(n_int px, n_int py);

void path_init(n_vect2 * location, simulated_path_group * group, n_int rotation, n_int ringroad);
void draw_path(simulated_path_group * group);

void fence_init(n_byte2 * seed, n_byte rotate, n_vect2 * location, simulated_fence * fences);
void fence_draw(simulated_fence * fences);

void house_init(n_byte2 * seed, n_vect2 * location, simulated_building * buildings);
void draw_house(simulated_building * buildings);

void tree_init(simulated_tree * trees, n_byte2 * seed, n_vect2 * edge);
n_int tree_populated(simulated_tree * tree);
void draw_tree(simulated_tree * tree);

void agent_init(void);

n_vect2 * agent_location(void);

n_int agent_facing(void);

void agent_zoom(n_int zoom);
n_int agent_zooming(void);

void agent_turn(n_int delta);
void agent_move(n_int forwards);
void agent_cycle(void);

void offset_map(n_vect2 * offset, n_vect2 * point);

void matrix_add_window(n_vect2 * start, n_vect2 * end);
void matrix_add_door(n_vect2 * start, n_vect2 * end);
void matrix_add_wall(n_vect2 * start, n_vect2 * end);
void matrix_add_fence(n_vect2 * start, n_vect2 * end);

n_byte matrix_visually_open(n_vect2 * origin, n_vect2 * end);

void matrix_account(void);

memory_list * matrix_draw_identifier(void);
memory_list * matrix_draw_block(void);

void matrix_draw_identifier_clear(void);

void matrix_init(void);
void matrix_close(void);

n_object * game_object_fence(simulated_fence * fence);
n_object * game_object_park(simulated_park * park);
n_object * game_object_twoblock(simulated_twoblock * twoblock);

void neighborhood_object(void);

#endif /* _MUSHROOM_H */
