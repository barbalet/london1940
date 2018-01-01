/****************************************************************
 
 tree.c
 
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

void tree_create(noble_tree * tree, n_byte2 * seed, n_vect2 * center)
{
    n_int loop = 0;
    n_int wandering = (math_random(seed) % 11) - 5;
    
    while (loop < POINTS_PER_TREE)
    {
        wandering += (math_random(seed) % 11) - 5;
        tree->points[loop] = 60-wandering;
        
        loop++;
    }
    
    loop = 0;
    while (loop < POINTS_PER_TREE)
    {
        tree->points[loop] = (tree->points[(loop + POINTS_PER_TREE - 1) & (POINTS_PER_TREE - 1)] +
                              tree->points[loop] + tree->points[(loop + 1) & (POINTS_PER_TREE - 1)])/3;
        loop++;
    }
    
    tree->center.x = center->x;
    tree->center.y = center->y;
    tree->radius = (math_random(seed) % 8) + 5;
}


void tree_draw(noble_tree * tree)
{
    n_vect2 quad[4];
    n_vect2 unit[2] = {1, 1};
    n_int   loop = 0;
    
    while (loop < 32)
    {
        gldraw_green();

        vect2_copy(&quad[0], &tree->center);
        
        vect2_direction(&quad[1], loop * 8, 600);
        vect2_multiplier(&quad[1], &quad[1], (n_vect2 *)&unit, tree->points[loop&31] * tree->radius, 360);
        
        loop++;
        vect2_direction(&quad[2], loop * 8, 600);
        vect2_multiplier(&quad[2], &quad[2], (n_vect2 *)&unit, tree->points[loop&31] * tree->radius, 360);

        loop++;
        vect2_direction(&quad[3], loop * 8, 600);
        vect2_multiplier(&quad[3], &quad[3], (n_vect2 *)&unit, tree->points[loop&31] * tree->radius, 360);

        vect2_subtract(&quad[1], &quad[0], &quad[1]);
        vect2_subtract(&quad[2], &quad[0], &quad[2]);
        vect2_subtract(&quad[3], &quad[0], &quad[3]);
        
        gldraw_quads(quad, 1);
        gldraw_lightgreen();
        gldraw_line(&quad[1], &quad[2]);
        gldraw_line(&quad[2], &quad[3]);        
    }
    
}

static n_int tree_offset(n_byte2 * seed)
{
    return (math_random(seed) % 61) - 30;
}

void tree_init(noble_tree * trees, n_byte2 * seed, n_vect2 * edge)
{
    n_int count = 0;
    n_int px = -1;
    while (px < 1)
    {
        n_int py = -1;
        while (py < 1)
        {
            n_vect2 local_center;
            noble_tree * tree = &trees[count++];
            vect2_populate(&local_center,
                           (px * TREE_SPACE)+(TREE_SPACE/2)+tree_offset(seed),
                           (py * TREE_SPACE)+(TREE_SPACE/2)+tree_offset(seed));
            vect2_delta(&local_center, edge);
            tree_create(tree, seed, &local_center);
            py++;
        }
        px++;
    }
}
