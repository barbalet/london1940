/****************************************************************
 
 house.c
 
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
#define	NUM	20

static n_int    locx[NUM], locy[NUM];
static n_int	velx[NUM], vely[NUM];
static n_byte2  seed[2] = { 0x4342, 0xed34};

void enemy_init(void)
{
	n_int	lpx = 0;
	while (lpx < NUM){
		locx[lpx] = math_random(seed) & 255;
		locy[lpx] = math_random(seed) & 255;
		velx[lpx] = (math_random(seed) % 31) - 15;
		vely[lpx] = (math_random(seed) % 31) - 15;
		lpx++;
	}
}

void enemy_move(void)
{
	n_int	lpx = 0;
	while(lpx < NUM)
    {
		locx[lpx] += velx[lpx] >> 3;
		locy[lpx] += vely[lpx] >> 3;
				
		velx[lpx] += (math_random(seed) % 5) - 2;
		vely[lpx] += (math_random(seed) % 5) - 2;
		
		if ((locx[lpx] < 12) && (velx[lpx] < 0))
        {
            velx[lpx] = 0 - velx[lpx];
        }
		if ((locy[lpx] < 12) && (vely[lpx] < 0))
        {
           vely[lpx] = 0 - vely[lpx];
        }
        
		if ((locx[lpx] > 244) && (velx[lpx] > 0))
        {
            velx[lpx] = 0 - velx[lpx];
        }
		if ((locy[lpx] > 244) && (vely[lpx] > 0))
        {
            vely[lpx] = 0 - vely[lpx];
        }
        
		draw_line(locx[lpx],locy[lpx], locx[lpx],locy[lpx]);
        
		lpx++;
	}	
}

