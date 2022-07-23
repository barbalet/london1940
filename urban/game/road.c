/****************************************************************
 
 road.c
 
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

#define FOOTPATH_OFFSET 300
#define FOOTPATH_WIDTH  200 // 48 inches versus 144 inches

void path_set(n_vect2 * points, n_int px, n_int py)
{
    points[0].x = px;
    points[0].y = py;
    points[1].x = px;
    points[1].y = py;
    points[2].x = px;
    points[2].y = py;
    points[3].x = px;
    points[3].y = py;
}

void path_init(n_vect2 * location, simulated_path_group * group, n_int rotation, n_int ringroad)
{
    n_int count = 0;
    n_int px = 0;
    n_int py = 0;
    
    px = 0;
    while (px < 2)
    {
        simulated_path * temp_path = &group->paths[count++];
        
        path_set(temp_path->points,
                     location->x + 0 - FOOTPATH_OFFSET - FOOTPATH_WIDTH,
                     location->y + (px * RESIDENCE_SPACE * 4) - FOOTPATH_OFFSET - FOOTPATH_WIDTH);
        
        temp_path->points[1].x += (RESIDENCE_SPACE * 4);
        temp_path->points[2].x += (RESIDENCE_SPACE * 4);
        
        temp_path->points[2].y += FOOTPATH_WIDTH;
        temp_path->points[3].y += FOOTPATH_WIDTH;
        
        px++;
    }
    px = 0;
    while (py < 2)
    {
        simulated_path * temp_path = &group->paths[count++];

        path_set(temp_path->points,
                     location->x + (py * RESIDENCE_SPACE * 4) - FOOTPATH_OFFSET - FOOTPATH_WIDTH,
                     location->y + 0 - FOOTPATH_OFFSET - FOOTPATH_WIDTH);
    
        temp_path->points[1].y += (RESIDENCE_SPACE * 4);
        temp_path->points[2].y += (RESIDENCE_SPACE * 4);
        
        if (py == 1)
        {
            temp_path->points[1].y += FOOTPATH_WIDTH;
            temp_path->points[2].y += FOOTPATH_WIDTH;
        }
        
        temp_path->points[2].x += FOOTPATH_WIDTH;
        temp_path->points[3].x += FOOTPATH_WIDTH;
    
        py++;
    }
    
    if (ringroad == 0)
    {
        simulated_path * temp_path = &group->paths[count++];

        if (rotation)
        {
            path_set(temp_path->points,
                     location->x + (RESIDENCE_SPACE * 2) - FOOTPATH_OFFSET - FOOTPATH_WIDTH,
                     location->y + 0 - FOOTPATH_OFFSET - FOOTPATH_WIDTH);
            
            temp_path->points[1].y += (RESIDENCE_SPACE * 4);
            temp_path->points[2].y += (RESIDENCE_SPACE * 4);
            
            temp_path->points[2].x += FOOTPATH_WIDTH;
            temp_path->points[3].x += FOOTPATH_WIDTH;

        }
        else
        {
            path_set(temp_path->points,
                     location->x + 0 - FOOTPATH_OFFSET - FOOTPATH_WIDTH ,
                     location->y + (RESIDENCE_SPACE * 2) - FOOTPATH_OFFSET - FOOTPATH_WIDTH);
            
            temp_path->points[1].x += (RESIDENCE_SPACE * 4);
            temp_path->points[2].x += (RESIDENCE_SPACE * 4);
            
            temp_path->points[2].y += FOOTPATH_WIDTH;
            temp_path->points[3].y += FOOTPATH_WIDTH;
        }
    }
    group->number = count;
}
