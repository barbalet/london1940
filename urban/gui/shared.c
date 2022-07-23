/****************************************************************

 shared.c

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

//#include "../mapout/map/png.h"
#include "../../apesdk/shared.h"
#include "../../apesdk/render/graph.h"
#include "../game/mushroom.h"
#include "../game/city.h"

#include <stdio.h>

static n_byte  key_identification = 0;
static n_byte2 key_value = 0;
static n_byte  key_down = 0;

static n_byte * outputBuffer = 0L;
static n_byte * outputBufferOld = 0L;
static n_int    outputBufferMax = -1;

extern n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number);

void shared_color_8_bit_to_48_bit(n_byte2 * fit)
{
    draw_game_color(fit);
}

void shared_dimensions(n_int * dimensions)
{
    dimensions[0] = 1;
    dimensions[1] = 800;
    dimensions[2] = 600;
    dimensions[3] = 0;
}

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s, %s line: %ld\n", error_text, location, line_number);
	return -1;
}

shared_cycle_state shared_cycle(n_uint ticks, n_int fIdentification)
{
    return SHARED_CYCLE_OK;
}

n_int shared_init(n_int view, n_uint random)
{
    n_byte2   seed[4];

    draw_init();
    
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
    
    neighborhood_init(seed);
    city_init(seed);
    /*ecomony_init(seed);*/
    agent_init();
    
    return 0;
}

void shared_close(void)
{
    if (outputBufferOld)
    {
        memory_free((void**)&outputBufferOld);
    }
    
    if (outputBuffer)
    {
        memory_free((void**)&outputBuffer);
    }
    
    draw_close();
}

n_int shared_menu(n_int menuValue)
{
    return 0;
}

void shared_rotate(n_double num, n_int wwind)
{
    n_int integer_rotation_256 = (n_int)((num * 256) / 360);
    agent_turn(integer_rotation_256);
}


void shared_delta(n_double delta_x, n_double delta_y, n_int wwind)
{
    if (delta_y > 0)
    {
        agent_move(-1);
    }
    if (delta_y < 0)
    {
        agent_move(1);
    }
}

void shared_zoom(n_double num, n_int wwind)
{
    n_int integer_zoom = (n_int)(num * 100);
    agent_zoom(integer_zoom);
}

void shared_keyReceived(n_int value, n_int fIdentification)
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

void shared_mouseReceived(n_double valX, n_double valY, n_int localIdentification)
{
}

void shared_mouseUp(void)
{
}

void shared_about(void)
{
    
}

#ifdef _WIN32

static n_byte offscreen[800 * 600];

n_byte * shared_legacy_draw(n_byte fIdentification, n_int dim_x, n_int dim_y)
{
    shared_draw(0, 800, 600, 0);
    return offscreen;
}

#endif

static n_byte * shared_output_buffer(n_int width, n_int height)
{
    if (outputBufferOld)
    {
        memory_free((void **)&outputBufferOld);
    }
    
    if ((outputBuffer == 0L) || ((width * height * 4) > outputBufferMax))
    {
        outputBufferMax = width * height * 4;
        outputBufferOld = outputBuffer;
        outputBuffer = memory_new(outputBufferMax);
    }
    return outputBuffer;
}

n_byte * shared_draw(n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed)
{
    n_byte * outputBuffer = shared_output_buffer(dim_x, dim_y);
    n_int print_screen = 0;
    n_int save_neighborhood = 0;
    n_int turn_delta = 0;
    n_int move_delta = 0;
    n_int zoomed_delta = 0;
    if((key_down == 1) && (key_identification == fIdentification))
    {
        n_int mod_key = key_value & 2047;
        n_int shift_key = key_value >> 11;
        
        if ((mod_key == 'p') || (mod_key == 'P'))
        {
            printf("print screen\n");
            print_screen = 1;
        }
        if ((mod_key == 's') || (mod_key == 'S'))
        {
            printf("save neighborhood\n");
            save_neighborhood = 1;
        }
        
        if ((mod_key > 27) && (mod_key < 32))
        {
            switch (mod_key)
            {
                case 28:
                    turn_delta --;
                    break;
                case 29:
                    turn_delta ++;
                    break;
                case 30:
                    if (shift_key)
                    {
                        zoomed_delta++;
                    }
                    else
                    {
                        move_delta ++;
                    }
                    break;
                default:
                    if (shift_key)
                    {
                        zoomed_delta--;
                    }
                    else
                    {
                        move_delta --;
                    }
                    break;
            }
        }
    }

    agent_turn(turn_delta);
    agent_zoom(zoomed_delta);
    agent_move(move_delta);
    agent_cycle();
    city_cycle();
    if (draw_game_scene(dim_x, dim_y))
    {
        draw_render(outputBuffer, dim_x, dim_y);
    }
    if (print_screen)
    {
//        n_byte * threebytes = convert_4_to_3(outputBuffer, (dim_x * dim_y));
//        write_png_file("mushroom_output.png", (int)dim_x, (int)dim_y, threebytes);
//        memory_free((void**)&threebytes);
    }
    
    if (save_neighborhood)
    {
        neighborhood_object();
    }
    return outputBuffer;
}

n_int shared_new(n_uint seed)
{    
    return 0;
}

n_int shared_new_agents(n_uint seed)
{
    return 0;
}

n_byte shared_openFileName(n_constant_string cStringFileName, n_int isScript)
{
//    (void) element_document_disk_read(cStringFileName);
    return 0;
}

void shared_saveFileName(n_constant_string cStringFileName)
{
    
}

void shared_script_debug_handle(n_constant_string cStringFileName)
{
    
}

n_uint shared_max_fps(void)
{
    return 60;
}


