/****************************************************************
 
 neighborhood.c
 
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

noble_twoblock twoblock[36];

void neighborhood_init(n_byte2 * seed, n_vect2 * location)
{
    n_int   count = 0;
    n_int   py = -3;
    while (py < 3)
    {
        n_int px = -3;
        while (px < 3)
        {
            n_vect2 additional;
            additional.x = location->x + (px * 3400);
            additional.y = location->y + (py * 3400);
            twoblock_init(seed, &additional, &twoblock[count++]);
            px++;
        }
        py++;
    }
    
    
    
}

void neighborhood_draw(void)
{
    n_int   count = 0;
    while (count < 36)
    {
        twoblock_draw(&twoblock[count++]);
    }
}
