/****************************************************************

 shared.c

 =============================================================

 Copyright 1996-2021 Tom Barbalet. All rights reserved.

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

#include "../apesdk/shared.h"
#include "mapgui.h"



void shared_color_8_bit_to_48_bit(n_byte2* values)
{
#ifdef _WIN32
	n_byte rgb[3];
	n_int loop = 0;
	n_int loop3 = 0;
	while (loop < 256)
	{
		draw_color(loop, rgb);
		values[loop3++] = rgb[0] << 8;
		values[loop3++] = rgb[1] << 8;
		values[loop3++] = rgb[2] << 8;
		loop++;
	}
#endif
}

void shared_dimensions(n_int * dimensions)
{
    dimensions[0] = 1;   /* number windows */
    dimensions[1] = (256 * 4); /* window dimension x */
    dimensions[2] = (256 * 3); /* window dimension y */
    dimensions[3] = 0;   /* has menus */
}

shared_cycle_state shared_cycle(n_uint ticks, n_int fIdentification)
{
//    if (simulation_started)
//        engine_update();

    return SHARED_CYCLE_OK;
}

n_int shared_init(n_int view, n_uint random)
{
//    if (engine_init(random))
//    {
//        simulation_started = 1;
//    }
    return 0;
}

void shared_close(void)
{
    printf("shared_close\n");
    draw_cleanup();
}

n_int shared_menu(n_int menuValue)
{
    return 0;
}

void shared_delta(n_double delta_x, n_double delta_y, n_int wwind)
{
    draw_dp((n_int)delta_x*2, (n_int)delta_y*2);
}

void shared_zoom(n_double num, n_int wwind)
{
    zoom_interaction(num);
}

void shared_rotate(n_double num, n_int wwind)
{
    
}

n_int key_pressed = -1;

void shared_keyReceived(n_int value, n_int fIdentification)
{
    draw_key_received(value);
}

void shared_keyUp(void)
{
}

void shared_mouseOption(n_byte option)
{
    
}

void shared_mouseReceived(n_double valX, n_double valY, n_int fIdentification)
{
    draw_mouse((n_int)valX, (n_int)valY);
}

void shared_mouseUp(void)
{
    draw_mouse_up();
}

void shared_about(void)
{
    
}

static n_byte * shared_output_buffer(n_int width, n_int height)
{
    printf("shared_output_buffer\n");

    return draw_screen_buffer();
}
n_byte * shared_legacy_draw(n_byte fIdentification, n_int dim_x, n_int dim_y)
{
    printf("shared_legacy_draw\n");

    return shared_output_buffer(dim_x, dim_y);
}

n_byte * shared_draw(n_int fIdentification, n_int dim_x, n_int dim_y, n_byte changed)
{
    return draw_screen_buffer();
}

n_int shared_new(n_uint seed)
{
//    engine_new();
    return 0;
}

n_int shared_new_agents(n_uint seed)
{
//    engine_new();
    return 0;
}

n_byte shared_openFileName(n_constant_string cStringFileName, n_int isScript)
{
    main_capture(cStringFileName);
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


#ifndef	_WIN32

n_int sim_thread_console_quit(void)
{
    return 0;
}

#endif

