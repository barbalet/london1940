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

static n_byte  map[MAP_AREA];
static n_byte  economy[MAP_AREA];
static n_byte  road[MAP_AREA];

extern n_byte * draw_screen(void);


/* Roads are defined as either linking or radial from economic centers mapped onto the topology of the landscape */
static void  economy_road(n_byte * local_map, n_byte * local_economy, n_byte * local_road)
{
    n_int max = 0;
    n_int min = 0xffff;
    n_byte2 *scratchpad = 0L;
    n_int   py = 0;
    n_int   delta;
    
    scratchpad = io_new(sizeof(n_byte2)* MAP_AREA);
    
    if (scratchpad == 0L)
    {
        SHOW_ERROR("scratchpad to create economy could not be allocated");
        return;
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
            n_int px0 = (px + (MAP_DIMENSION - 1)) & (MAP_DIMENSION - 1);
            n_int px1 = (px + 1) & (MAP_DIMENSION - 1);
            n_int dx = (n_int)local_economy[px1 | pym] - (n_int)local_economy[px0 | pym] + (n_int)local_map[px1 | pym] - (n_int)local_map[px0 | pym];
            n_int dy = (n_int)local_economy[px | py1] - (n_int)local_economy[px | py0]   + (n_int)local_map[px | py1] -  (n_int)local_map[px | py0];
            n_uint div = (dx * dx) + (dy * dy);
            div = math_root(div);
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
            local_road[loop] = ((scratchpad[loop] - min) * 255) / delta;
            loop++;
        }
    }
    io_free((void**)&scratchpad);
}

n_int ecomony_init(n_byte2 * seeds)
{
    n_byte2  local_random[2];
    
    n_byte * actual = io_new(MAP_AREA);

    if (actual == 0L)
    {
        return SHOW_ERROR("Memory init economy failed to allocate");
    }
    
    local_random[0] = seeds[0];
    local_random[1] = seeds[1];
    
    math_patch(map, actual, &math_random, local_random, MAP_BITS, 0, 7, 1);

    local_random[0] = seeds[2];
    local_random[1] = seeds[3];
    
    math_patch(economy, actual, &math_random, local_random, MAP_BITS, 0, 4, 1);
    
    io_free((void **)&actual);
    
    economy_road(map, economy, road);
    
    return 0;
}

static void economy_micro(n_byte * value)
{
    n_byte * screen = draw_screen();
    n_int loop = 0;
    while (loop < 512)
    {
        io_copy(&value[MAP_DIMENSION * loop], &screen[1024 * loop], 512);
        io_copy(&value[MAP_DIMENSION * loop], &screen[(1024 * loop) + 512], 512);
        loop++;
    }
    while (loop < 768)
    {
        io_copy(&screen[1024 * (loop-512)], &screen[1024 * loop], 1024);
        loop++;
    }
}

static n_int switcher = 0;

void economy_draw(n_int px, n_int py)
{
    if (switcher)
    {
        economy_micro(road);
        switcher = 0;
    }
    else
    {
        economy_micro(economy);
        switcher = 1;
    }
}
