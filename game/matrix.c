/****************************************************************
 
 matrix.c
 
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

#include "mushroom.h"
#include "city.h"

static n_int number_windows;
static matrix_plane recorded_windows[6000];

static n_int number_doors;
static matrix_plane recorded_doors[10000];

static n_int number_walls;
static matrix_plane recorded_walls[50000];

static n_int number_fences;
static matrix_plane recorded_fences[1000];

void matrix_init(void)
{
    number_windows = 0;
    number_doors = 0;
    number_fences = 0;
}

void matrix_add_door(n_vect2 * start, n_vect2 * end)
{
    recorded_doors[number_doors].start.x = start->x;
    recorded_doors[number_doors].start.y = start->y;
    recorded_doors[number_doors].end.x = end->x;
    recorded_doors[number_doors].end.y = end->y;
    number_doors ++;
}

void matrix_add_fence(n_vect2 * start, n_vect2 * end)
{
    recorded_fences[number_fences].start.x = start->x;
    recorded_fences[number_fences].start.y = start->y;
    recorded_fences[number_fences].end.x = end->x;
    recorded_fences[number_fences].end.y = end->y;
    number_fences ++;
}

void matrix_add_window(n_vect2 * start, n_vect2 * end)
{
    recorded_windows[number_windows].start.x = start->x;
    recorded_windows[number_windows].start.y = start->y;
    recorded_windows[number_windows].end.x = end->x;
    recorded_windows[number_windows].end.y = end->y;
    number_windows ++;
}

void matrix_add_wall(n_vect2 * start, n_vect2 * end)
{
    recorded_walls[number_walls].start.x = start->x;
    recorded_walls[number_walls].start.y = start->y;
    recorded_walls[number_walls].end.x = end->x;
    recorded_walls[number_walls].end.y = end->y;
    number_walls ++;
}

n_byte matrix_visually_blocked(n_vect2 * origin, n_vect2 * delta)
{
    n_int loop = 0;
    while (loop < number_fences)
    {
        if (math_do_intersect(origin, delta, &recorded_fences[loop].start, &recorded_fences[loop].end))
        {
            return 1;
        }
        loop++;
    }
    loop = 0;
    while (loop < number_walls)
    {
        if (math_do_intersect(origin, delta, &recorded_walls[loop].start, &recorded_walls[loop].end))
        {
            return 1;
        }
        loop++;
    }
    return 0;
}

void matrix_account(void)
{
    printf("number windows %ld\n", number_windows);
    printf("number doors %ld\n",   number_doors);
    printf("number walls %ld\n",   number_walls);
    printf("number fences %ld\n",  number_fences);
}
