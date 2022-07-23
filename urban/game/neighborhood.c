/****************************************************************
 
 neighborhood.c
 
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

#include "../game/city.h"
#include "mushroom.h"

static simulated_twoblock twoblock[TWO_BLOCK_NUM];
static simulated_park     park[PARK_NUM];
static simulated_fence    fences[FENCE_NUM];

simulated_twoblock * neighborhoood_twoblock(n_int * count)
{
    *count = TWO_BLOCK_NUM;
    return twoblock;
}

simulated_park * neighborhoood_park(n_int * count)
{
    *count = PARK_NUM;
    return park;
}

simulated_fence * neighborhoood_fence(n_int * count)
{
    *count = FENCE_NUM;
    return fences;
}

void neighborhood_object(void)
{
    n_object * return_object = 0L;
    n_array * created_array = 0L;
    n_int loop = 0;
    while (loop < TWO_BLOCK_NUM)
    {
        array_add_empty( &created_array, array_object(game_object_twoblock(&twoblock[loop])));
        loop++;
    }
    return_object = object_array(0L,"twoblocks", created_array);

    created_array = 0L;
    loop = 0;
    
    while (loop < PARK_NUM)
    {
        array_add_empty( &created_array, array_object(game_object_park(&park[loop])));
        loop++;
    }
    object_array(return_object,"parks", created_array);

    created_array = 0L;
    loop = 0;
    
    while (loop < FENCE_NUM)
    {
        array_add_empty( &created_array, array_object(game_object_fence(&fences[loop])));
        loop++;
    }
    object_array(return_object,"fences", created_array);
    
    n_file   *output_file = unknown_json( return_object, OBJECT_OBJECT );
    if ( output_file )
    {
        // /Users/SOMEONE/Documents/neighborhood.json
        io_disk_write( output_file, "neighborhood.json" );
        io_file_free( &output_file );
    }
    unknown_free( (void **) &return_object, OBJECT_OBJECT );
}

void neighborhood_init(n_byte2 * seed)
{
    n_vect2 location = { 500 , 500};
    n_int   park_count = 0;
    n_int   twoblock_count = 0;
    n_int   py = 0 - TWO_BLOCK_EDGE_HALF;
    while (py < TWO_BLOCK_EDGE_HALF)
    {
        n_int px = 0 - TWO_BLOCK_EDGE_HALF;
        while (px < TWO_BLOCK_EDGE_HALF)
        {
            n_vect2 additional;
            n_byte2 park_probabilty = math_random(seed) & 255;
            
            additional.x = location.x + (px * NEIGHBORHOOD_UNIT_SPACE);
            additional.y = location.y + (py * NEIGHBORHOOD_UNIT_SPACE);
            
            if (twoblock_count == (TWO_BLOCK_NUM))
            {
                park_probabilty = 255;
            }
            if (park_count == PARK_NUM)
            {
                park_probabilty = 0;
            }
            
            if (park_probabilty > PARK_PROBABILITY)
            {
                park_init(seed, &additional, &park[park_count++]);
            }
            else
            {
                twoblock_init(seed, &additional, &twoblock[twoblock_count++]);
            }
            px++;
        }
        py++;
    }

    fences[0].points[0].x = CITY_BOTTOM_LEFT_X;
    fences[0].points[0].y = CITY_BOTTOM_LEFT_Y;
    fences[0].points[1].x = CITY_TOP_RIGHT_X;
    fences[0].points[1].y = CITY_BOTTOM_LEFT_Y;

    fences[1].points[0].x = CITY_TOP_RIGHT_X;
    fences[1].points[0].y = CITY_BOTTOM_LEFT_Y;
    fences[1].points[1].x = CITY_TOP_RIGHT_X;
    fences[1].points[1].y = CITY_TOP_RIGHT_Y;
    
    fences[2].points[0].x = CITY_TOP_RIGHT_X;
    fences[2].points[0].y = CITY_TOP_RIGHT_Y;
    fences[2].points[1].x = CITY_BOTTOM_LEFT_X;
    fences[2].points[1].y = CITY_TOP_RIGHT_Y;

    fences[3].points[0].x = CITY_BOTTOM_LEFT_X;
    fences[3].points[0].y = CITY_TOP_RIGHT_Y;
    fences[3].points[1].x = CITY_BOTTOM_LEFT_X;
    fences[3].points[1].y = CITY_BOTTOM_LEFT_Y;
}
