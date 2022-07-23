/****************************************************************
 
 game_objects.c
 
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

#include "mushroom.h"

static n_string game_object_direction(n_int direction)
{
    switch (direction)
    {
        case 0:
        {
            return "north";
        }
        case 1:
        {
            return "south";
        }
        case 2:
        {
            return "east";
        }
        case 3:
        {
            return "west";
        }
    }
    return "unspecified";
}

/*
typedef struct{
    n_vect2  points[POINTS_PER_FENCE];
}simulated_fence;
*/

n_object * game_object_fence(simulated_fence * fence)
{
    return object_array(0L, "fence", object_vect2_pointer(fence->points, POINTS_PER_FENCE));
}

/*
typedef struct{
    n_vect2  points[POINTS_PER_PATH];
}simulated_path;
*/

static n_object * game_object_path(simulated_path * path)
{
    return object_array(0L, "path", object_vect2_pointer(path->points, POINTS_PER_PATH));
}

/*
typedef struct{
    simulated_path  paths[PATHS_PER_GROUP];
    n_int           number;
}simulated_path_group;
*/

static n_object * game_object_path_group(simulated_path_group * path_group)
{
    n_array  * path_array = 0L;
    if (path_group->number)
    {
        n_int      loop = 0;
        while (loop < path_group->number)
        {
            array_add_empty( &path_array, array_object(game_object_path(&path_group->paths[loop])));
            loop++;
        }
    }
    if (path_array)
    {
        return object_array(0L, "paths", path_array);
    }
    return 0L;
}

/*
typedef struct{
    n_vect2  points[POINTS_PER_ROOM];
    n_byte   window;
    n_byte   door;
}simulated_room;
*/


static n_array * game_object_numbers_array_internal(n_int * numbers, n_uint count)
{
    n_array  * points = 0L;
    n_int      loop = 0;
    while (loop < count)
    {
        array_add_empty( &points, array_number(numbers[loop]) );
        loop++;
    }
    return points;
}

/*
typedef struct{
    n_int    points[POINTS_PER_TREE];
    n_int    radius;
    n_vect2  center;
}simulated_tree;
*/

static n_object * game_object_tree(simulated_tree * tree, n_string location)
{
    n_object * return_object = object_string(0L, "location", location);
    object_number(return_object, "radius", tree->radius);
    object_array(return_object, "center", object_vect2_array(&tree->center));
    object_array(return_object, "values", game_object_numbers_array_internal(tree->points, POINTS_PER_TREE));
    return return_object;
}

static n_array * game_object_four_trees_internal(simulated_tree * trees)
{
    n_array  * created_array = 0L;
    n_int      loop = 0;
    while (loop < 4)
    {
        if (tree_populated(&trees[loop]))
        {
            array_add_empty(& created_array, array_object(game_object_tree(&trees[loop], game_object_direction(loop))));
        }
        loop++;
    }
    return created_array;
}


static n_object * game_object_window(n_vect2 * window, n_string location)
{
    n_object * return_object = object_string(0L, "location", location);
    object_array(return_object, "points", object_vect2_pointer(window, 2));
    return return_object;
}

static n_object * game_object_door(n_vect2 * door, n_string location)
{
    n_object * return_object = object_string(0L, "location", location);
    object_array(return_object, "points", object_vect2_pointer(door, 4));
    return return_object;
}

/*
 
 struct
 {
     n_vect2 inner_walls[4];
     n_vect2 outer_walls[4];
     n_vect2 windows[4][2];
     n_vect2 doors[4][4];
 };
 
 */

static n_object * game_object_room(simulated_room * room)
{
    n_int      loop = 0;
    n_array  * windows_array = 0L;
    n_array  * doors_array = 0L;
    n_object * return_object = object_array(0L, "inner_walls", object_vect2_pointer(&room->points[0], 4));
    object_array(return_object, "outer_walls", object_vect2_pointer(&room->points[4], 4));

    while (loop < 4)
    {
        n_string direction = game_object_direction(loop);
        n_vect2 * window = &room->points[8 + (loop * 2)];
        n_vect2 * door = &room->points[16 + (loop * 4)];
        if (house_window_present(window))
        {
            array_add_empty( &windows_array, array_object(game_object_window(window, direction)));
        }
        if (house_door_present(door))
        {
            array_add_empty( &doors_array, array_object(game_object_door(door, direction)));
        }
        loop++;
    }
    
    if (doors_array)
    {
        object_array(return_object, "doors", doors_array);
    }
    if (windows_array)
    {
        object_array(return_object, "windows", windows_array);
    }

    return return_object;
}


/*
typedef struct{
    simulated_room   room[MAX_ROOMS];
    n_int            house[GENETICS_COUNT];
    n_int            roomcount;
    n_byte           rotation;
    simulated_tree   trees[4];
    n_byte           tree_mod;
}simulated_building;
*/


static n_object * game_object_building(simulated_building * building)
{
    n_int               loop = 0;
    n_array  * created_array = 0L;
    while ( loop < building->roomcount )
    {
        array_add_empty( &created_array, array_object(game_object_room(&building->room[loop])));
        loop++;
    }
    if (created_array)
    {
        return object_array(0L, "rooms", created_array);
    }
    return 0L;
}

/*
typedef struct{
    simulated_path_group   road;
    simulated_tree         trees[16][4];
    n_byte                 tree_mod[16];
}simulated_park;
*/

n_object * game_object_park(simulated_park * park)
{
    n_object * return_object = object_object(0L, "road", game_object_path_group(&park->road));
    n_int      loop = 0;
    n_array  * created_array = 0L;

    while (loop < 16)
    {
        n_array * tree_array = game_object_four_trees_internal(park->trees[loop]);
        if (tree_array)
        {
            array_add_empty(&created_array, array_array(tree_array));
        }
        loop++;
    }
    object_array(return_object, "trees", created_array);
    return return_object;
}

/*
typedef struct{
    simulated_building   house[16];
    simulated_fence      fence[8];
    simulated_path_group footpath;
    simulated_path_group road;
    n_byte               rotation;
}simulated_twoblock;
*/

n_object * game_object_twoblock(simulated_twoblock * twoblock)
{
    n_object * return_object = 0L;
    n_int      loop = 0;
    n_array  * created_array = 0L;
    n_array * trees_array = 0L;
    
    while (loop < 16)
    {
        simulated_tree * four_trees = (simulated_tree *) &(twoblock->house[loop].trees);
        if (four_trees)
        {
            n_array * four_trees_array = game_object_four_trees_internal(four_trees);
            if (four_trees_array)
            {
                array_add_empty( &trees_array, array_array(four_trees_array));
            }
        }
        array_add_empty( &created_array, array_object(game_object_building(&twoblock->house[loop])));
        loop++;
    }
    
    return_object = object_array(0L, "houses", created_array);
    
    if (trees_array)
    {
        object_array(return_object, "trees", trees_array);
    }
    
    created_array = 0L;
    loop = 0;
    
    while (loop < 8)
    {
        array_add_empty( &created_array, array_object(game_object_fence (&twoblock->fence[loop])) );
        loop++;
    }
    object_array(return_object, "fences", created_array);
    
    object_object(return_object, "roads", game_object_path_group(&twoblock->road));

    return return_object;
}
