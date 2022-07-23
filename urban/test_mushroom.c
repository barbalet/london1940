/****************************************************************

 test_mushroom.c

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

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

#include <stdio.h>
#include <time.h>

#include "game/mushroom.h"
#include "game/city.h"

extern n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number);

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s, %s line: %ld\n", error_text, location, line_number);
    return -1;
}

void test_init(n_uint random)
{
    n_byte2   seed[4];
    
    printf("random %lu\n", random);
    
    seed[3] = (random >>  0) & 0xffff;
    seed[2] = (random >> 16) & 0xffff;
    seed[1] = (random >> 32) & 0xffff;
    seed[0] = (random >> 48) & 0xffff;
    
    seed[0] ^= seed[2];
    seed[1] ^= seed[3];
    
    math_random(seed);
    math_random(seed);
    math_random(seed);
    math_random(seed);
    math_random(seed);
    
    city_init(seed);
    /*ecomony_init(seed);*/
    agent_init();
}

void test_cycle(void)
{
    agent_cycle();
    city_cycle();
}

int main(int argc, const char * argv[])
{
    n_int loop = 0;
    printf(" --- test mushroom --- start -----------------------------------------------\n");
    
    test_init(0x12738291);

    while (loop < 2000)
    {
        test_cycle();

        loop++;
    }
        
    printf(" --- test mushroom ---  end  -----------------------------------------------\n");
    
    return 1;
}

