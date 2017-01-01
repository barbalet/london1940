/****************************************************************
 
 economy.c
 
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

static n_byte  map[MAP_AREA];
static n_byte  flat[MAP_AREA];
static n_byte  economy[MAP_AREA];
static n_byte  sum[MAP_AREA];
static n_byte  road[MAP_AREA];

typedef n_byte2 (economy_scan)(n_byte * scan1, n_byte * scan2, n_int py0, n_int py1, n_int pym, n_int px);

n_byte2 generate_road(n_byte * scan1, n_byte * scan2, n_int py0, n_int py1, n_int pym, n_int px)
{
    n_int px0 = (px + (MAP_DIMENSION - 1)) & (MAP_DIMENSION - 1);
    n_int px1 = (px + 1) & (MAP_DIMENSION - 1);
    n_int dx = (n_int)scan1[px1 | pym] - (n_int)scan1[px0 | pym];
    n_int dy = (n_int)scan1[px | py1]  - (n_int)scan1[px | py0];
    n_uint div = (dx * dx) + (dy * dy);
    return math_root(div);
}

n_byte2 generate_flat(n_byte * scan1, n_byte * scan2, n_int py0, n_int py1, n_int pym, n_int px)
{
    n_int px0 = (px + (MAP_DIMENSION - 1)) & (MAP_DIMENSION - 1);
    n_int px1 = (px + 1) & (MAP_DIMENSION - 1);
    n_int dx = (n_int)scan1[px1 | pym] - (n_int)scan1[px0 | pym];
    n_int dy = (n_int)scan1[px | py1]  - (n_int)scan1[px | py0];
    n_uint div = (dx * dx * dy * dy);
    return 0xffff - math_root(div);
}

n_byte2 generate_add(n_byte * scan1, n_byte * scan2, n_int py0, n_int py1, n_int pym, n_int px)
{
    n_int   location = px | pym;
    return (n_byte2)scan1[location] + (n_byte2)scan2[location];
}

static n_int economy_scan_calculation(n_byte * input1, n_byte * input2, n_byte * output, economy_scan calculation)
{
    n_int   max = 0;
    n_int   min = 0xffff;
    n_byte2 *scratchpad = 0L;
    n_int   py = 0;
    n_int   delta;
    
    scratchpad = io_new(sizeof(n_byte2)* MAP_AREA);
    
    if (scratchpad == 0L)
    {
        return SHOW_ERROR("scratchpad to create calculation could not be allocated");
    }
    
    io_erase((n_byte *)scratchpad, sizeof(n_byte2)* MAP_AREA);
    
    while (py < MAP_DIMENSION)
    {
        n_int py0 = ((py + (MAP_DIMENSION - 1)) & (MAP_DIMENSION - 1)) << MAP_BITS;
        n_int py1 = ((py + 1) & (MAP_DIMENSION - 1)) << MAP_BITS;
        n_int pym = py << MAP_BITS;
        n_int px = 0;
        while (px < MAP_DIMENSION)
        {
            n_byte2 div = (calculation)(input1, input2, py0, py1, pym, px);
            if (div > max)
            {
                max = div;
            }
            if (div < min)
            {
                min = div;
            }
            scratchpad[px | pym] = div;
            px++;
        }
        py++;
    }
    
    delta = max - min;
    
    if (delta)
    {
        n_int loop = 0;
        while (loop < MAP_AREA)
        {
            output[loop] = ((scratchpad[loop] - min) * 255) / delta;
            loop++;
        }
    }
    io_free((void**)&scratchpad);
    return 0;
}


/* Roads are defined as either linking or radial from economic centers mapped onto the topology of the landscape */

n_int ecomony_init(n_byte2 * seeds)
{
    n_byte * actual = io_new(MAP_AREA);

    if (actual == 0L)
    {
        return SHOW_ERROR("Memory init economy failed to allocate");
    }
    
    land_creation(map, actual, seeds, 0L);
    land_creation(economy, actual, &seeds[2], 0L);
    
    io_free((void **)&actual);
    
    economy_scan_calculation(map, 0L, flat, generate_flat);
    economy_scan_calculation(flat, economy, sum, generate_add);
    economy_scan_calculation(sum, 0L, road, generate_road);
    
    return 0;
}

static n_int offset_map_x(n_vect2 * point)
{
    n_int  p0 = map[point->x + (point->y * MAP_DIMENSION)];
    n_int  p1 = map[((point->x + 1) & (MAP_DIMENSION - 1)) + (point->y * MAP_DIMENSION)];

    return p0 - p1;
}

static n_int offset_map_y(n_vect2 * point)
{
    n_int  p0 = map[point->x + (point->y * MAP_DIMENSION)];
    n_int  p1 = map[point->x + (((point->x + 1) & (MAP_DIMENSION - 1)) * MAP_DIMENSION)];
    return p0 - p1;
}

void offset_map(n_vect2 * offset, n_vect2 * point)
{
    offset->x = offset_map_x(point) * 5;
    offset->y = offset_map_y(point) * 5;
}



