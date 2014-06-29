/****************************************************************
 
 economy.c
 
 =============================================================
 
 Copyright 1996-2014 Tom Barbalet. All rights reserved.
 
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

static n_byte      map[HI_RES_MAP_AREA];
static n_byte      economy[HI_RES_MAP_AREA];
static n_byte      road[HI_RES_MAP_AREA];

/* Roads are defined as either linking or radial from economic centers mapped onto the topology of the landscape */
static void  economy_road(n_byte * local_map, n_byte * local_economy, n_byte * local_road)
{
    
}

n_int ecomony_init(n_byte2 * seeds)
{
    n_byte * actual = io_new(MAP_AREA * 2);

    if (actual == 0L)
    {
        return SHOW_ERROR("Memory init economy failed to allocate");
    }
    
    land_init(seeds, actual, map, &actual[MAP_AREA], 0);
    land_init(seeds, actual, economy, &actual[MAP_AREA], 0);
    
    io_free((void **)&actual);
    
    economy_road(map, economy, road);
    
    return 0;
}


