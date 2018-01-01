/****************************************************************
 
 agent.c
 
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

static noble_agent mushroom_boy;

void agent_init(void)
{
    mushroom_boy.location.x = 0;
    mushroom_boy.location.y = 0;
    mushroom_boy.facing = 0;
    mushroom_boy.zooming = 0;
}

n_vect2 * agent_location(void)
{
    return &mushroom_boy.location;
}


n_int agent_facing(void)
{
    return mushroom_boy.facing;
}

void agent_turn(n_int delta)
{
    mushroom_boy.facing_delta += delta;
}

void agent_zoom(n_int zoom)
{
    n_int total_zoom = mushroom_boy.zooming + zoom;
    if ((total_zoom > -99) && (total_zoom < 40))
    {
        mushroom_boy.zooming = total_zoom;
    }
}

n_int agent_zooming(void)
{
    return mushroom_boy.zooming;
}

void agent_move(n_int forwards)
{
    n_vect2 direction;
    n_vect2 local_location;
    n_vect2 * copy_location = agent_location();
    n_int   translated_facing;
    vect2_copy(&local_location, copy_location);
    
    mushroom_boy.facing = (mushroom_boy.facing + mushroom_boy.facing_delta + 256) & 255;
    
    translated_facing = (128+64+256 - agent_facing())&255;
    
    vect2_direction(&direction, translated_facing, 1);
    vect2_d(copy_location, &direction, forwards, (26880/20));    
    vect2_subtract(&mushroom_boy.location_delta, copy_location, &local_location);
}

void agent_cycle(void)
{
    mushroom_boy.facing_delta = 0;
    mushroom_boy.location_delta.x = 0;
    mushroom_boy.location_delta.y = 0;
}
