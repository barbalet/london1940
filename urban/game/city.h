/****************************************************************
 
 city.h
 
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


#include "../game/mushroom.h"
#include "../../apesdk/entity/entity.h"


#define NEIGHBORHOOD_UNIT_SPACE (3400)
#define TWO_BLOCK_EDGE_HALF     (2) // 4 (up to 8)
#define TWO_BLOCK_EDGE          (TWO_BLOCK_EDGE_HALF * 2)

#define MAX_NUMBER_APES  (256) // (TWO_BLOCK_EDGE * TWO_BLOCK_EDGE * 4)

#define CITY_TOP_RIGHT_X ((NEIGHBORHOOD_UNIT_SPACE * TWO_BLOCK_EDGE_HALF) + 5 + (2*ROAD_WIDTH))
#define CITY_TOP_RIGHT_Y ((NEIGHBORHOOD_UNIT_SPACE * TWO_BLOCK_EDGE_HALF) + 5 + (2*ROAD_WIDTH))

#define CITY_BOTTOM_LEFT_X (0 - CITY_TOP_RIGHT_X )
#define CITY_BOTTOM_LEFT_Y (0 - CITY_TOP_RIGHT_Y )

#define PARK_NUM (TWO_BLOCK_EDGE_HALF)
#define TWO_BLOCK_NUM ((TWO_BLOCK_EDGE * TWO_BLOCK_EDGE) - PARK_NUM)
#define FENCE_NUM (4)

#define PARK_PROBABILITY  ((255 * TWO_BLOCK_NUM) / (PARK_NUM + TWO_BLOCK_NUM))

void city_init(n_byte2 * seed);
void city_cycle(void);

void city_translate(n_vect2 * pnt);

simulated_being * city_beings(void);
n_uint city_beings_number(void);

simulated_twoblock * neighborhoood_twoblock(n_int * count);
simulated_park * neighborhoood_park(n_int * count);
simulated_fence * neighborhoood_fence(n_int * count);
