/****************************************************************
 
 park.c
 
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


void park_init(n_byte2 * seed, n_vect2 * location, noble_park * parks)
{
    n_int count = 0;
    n_int px = 0;
    
    road_init(0, 1, location, (noble_road*)parks->road);
    
    while (px < 4)
    {
        n_int py = 0;
        while (py < 4)
        {
            n_vect2 local_center;
            vect2_populate(&local_center, (px * RESIDENCE_SPACE) + location->x, (py * RESIDENCE_SPACE) + location->y);
            parks->tree_mod[count] = math_random(seed) & 15;
            tree_init(parks->trees[count], seed, &local_center);
            
            parks->trees[count][0].radius = (math_random(seed) % 16) + 16;
            parks->trees[count][1].radius = (math_random(seed) % 16) + 16;
            parks->trees[count][2].radius = (math_random(seed) % 16) + 16;
            parks->trees[count][3].radius = (math_random(seed) % 16) + 16;

            count++;
            py++;
        }
        px++;
    }
}

void park_draw(noble_park * park)
{
    n_int count = 0;
    
    road_draw_ring(park->road);
    
    while (count < 16)
    {
        if (park->tree_mod[count] & 1)
        {
            tree_draw(&park->trees[count][0]);
        }
        if (park->tree_mod[count] & 2)
        {
            tree_draw(&park->trees[count][1]);
        }
        if (park->tree_mod[count] & 4)
        {
            tree_draw(&park->trees[count][2]);
        }
        if (park->tree_mod[count] & 8)
        {
            tree_draw(&park->trees[count][3]);
        }
        count++;
    }
}
