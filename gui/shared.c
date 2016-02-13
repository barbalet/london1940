/****************************************************************

 shared.c

 =============================================================

 Copyright 1996-2016 Tom Barbalet. All rights reserved.

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

#include "shared.h"
#include "mushroom.h"
#include <stdio.h>

static n_byte  key_identification = 0;
static n_byte2 key_value = 0;
static n_byte  key_down = 0;


extern n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number);


n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s, %s line: %ld\n", error_text, location, line_number);
	return -1;
}

shared_cycle_state shared_cycle(n_uint ticks, n_byte fIdentification, n_int dim_x, n_int dim_y)
{
    if((key_down == 1) && (key_identification == fIdentification))
    {
        /* control_key(key_identification, key_value); */
    }
    
    return SHARED_CYCLE_OK;
}

n_int shared_init(n_byte view, n_uint random)
{
    n_byte2   seed[4];
    n_byte2   *seed_random = (n_byte2*)&random;
    
    seed[0] = seed_random[0];
    seed[1] = seed_random[1];
    seed[2] = seed_random[2];
    seed[3] = seed_random[3];

    game_init(seed);
    
    enemy_init();
    ecomony_init(seed);
    
    boy_init();
    
    return 0;
}

void shared_close(void)
{

}

n_int shared_menu(n_int menuValue)
{
    return 0;
}

void shared_rotate(n_double num, n_byte wwind)
{
    
}

void shared_keyReceived(n_byte2 value, n_byte fIdentification)
{
    key_down = 1;
    key_value = value;
    key_identification = fIdentification;
}

void shared_keyUp(void)
{
    key_down = 0;
}

void shared_mouseOption(n_byte option)
{
    
}

void shared_mouseReceived(n_int valX, n_int valY, n_byte fIdentification)
{

}

void shared_mouseUp(void)
{
    
}

void shared_about(n_constant_string value)
{
    
}



void shared_draw(n_byte * outputBuffer, n_byte fIdentification, n_int dim_x, n_int dim_y)
{
    n_int turn_delta = 0;
    n_int move_delta = 0;
    
    if((key_down == 1) && (key_identification == fIdentification))
    {
        if ((key_value > 27) && (key_value < 32))
        {
            switch (key_value)
            {
                case 28:
                    turn_delta --;
                    break;
                case 29:
                    turn_delta ++;
                    break;
                case 30:
                    move_delta ++;
                    break;
                default:
                    move_delta --;
                    break;
            }
        }
    }

    boy_turn(turn_delta);
    boy_move(move_delta);
    boy_cycle();
    /*enemy_move();*/

    game_draw_scene(dim_x, dim_y);
}

n_int shared_new(n_uint seed)
{    
    return 0;
}

n_byte shared_openFileName(n_string cStringFileName, n_byte isScript)
{
    return 0;
}

void shared_saveFileName(n_string cStringFileName)
{
    
}

void shared_script_debug_handle(n_string cStringFileName)
{
    
}

n_uint shared_max_fps(void)
{
    return 60;
}

#ifndef	_WIN32

n_int sim_thread_console_quit(void)
{
    return 0;
}

#endif

