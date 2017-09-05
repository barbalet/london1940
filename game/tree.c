/****************************************************************
 
 tree.c
 
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

static  noble_tree trees[256];

void tree_create(noble_tree * tree, n_byte2 * seed, n_vect2 * center)
{
    n_int loop = 0;
    n_int wandering = (math_random(seed) % 13) - 6;
    
    while (loop < POINTS_PER_TREE)
    {
        wandering += (math_random(seed) % 13) - 6;
        tree->points[loop] = 60-wandering;
        loop++;
    }
    tree->center.x = center->x;
    tree->center.y = center->y;
    tree->radius = (math_random(seed) & 7) + 4;
}


static void tree_draw_each(noble_tree * tree)
{
    n_vect2 quad[4];
    n_vect2 unit[2] = {1, 1};
    n_int   loop = 0;
    gldraw_lightgreen();
    
    while (loop < 16)
    {
        vect2_copy(&quad[0], &tree->center);
        
        vect2_direction(&quad[1], loop * 16, 600);
        vect2_multiplier(&quad[1], &quad[1], (n_vect2 *)&unit, tree->points[loop&15] * tree->radius, 360);
        
        loop++;
        vect2_direction(&quad[2], loop * 16, 600);
        vect2_multiplier(&quad[2], &quad[2], (n_vect2 *)&unit, tree->points[loop&15] * tree->radius, 360);

        loop++;
        vect2_direction(&quad[3], loop * 16, 600);
        vect2_multiplier(&quad[3], &quad[3], (n_vect2 *)&unit, tree->points[loop&15] * tree->radius, 360);

        vect2_subtract(&quad[1], &quad[0], &quad[1]);
        vect2_subtract(&quad[2], &quad[0], &quad[2]);
        vect2_subtract(&quad[3], &quad[0], &quad[3]);
        
        gldraw_quads(quad, 1);
    }
}


void tree_draw(void)
{
    n_int count = 0;
    while (count < 256)
    {
        tree_draw_each(&trees[count++]);
    }
}

static n_int tree_offset(n_byte2 * seed)
{
    return (math_random(seed) % 61) - 30;
}

void tree_init(n_byte2 * seed)
{
    n_int count = 0;
    n_int px = -9;
    while (px < 7)
    {
        n_int py = -9;
        while (py < 7)
        {
            n_vect2 local_center;
            noble_tree * tree = &trees[count++];
            vect2_populate(&local_center,
                           (px * TREE_SPACE)+(TREE_SPACE/2)+tree_offset(seed),
                           (py * TREE_SPACE)+(TREE_SPACE/2)+tree_offset(seed));
            tree_create(tree, seed, &local_center);
            py++;
        }
        px++;
    }
}
