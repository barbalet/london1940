/****************************************************************
 
 mapgui.h
 
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "png/pnglite.h"

#include "../apesdk/toolkit/toolkit.h"

#define   STRING_TERRAIN     "terrain"
#define   STRING_URBAN       "urban"
#define   STRING_RESOLUTION  "resolution"

typedef enum{
    GLR_RED,
    GLR_PINK,
    
    GLR_ORANGE,
    GLR_APRICOT,
    
    GLR_YELLOW,
    GLR_BEIGE,
    
    GLR_GREEN,
    GLR_MINT,
    
    GLR_PURPLE,
    GLR_LAVENDER,
    
    GLR_GREY,
    GLR_WHITE
} GLR_COLOR;


static n_string color_string[12] = {
    "GLR_RED",
    "GLR_PINK",
    
    "GLR_ORANGE",
    "GLR_APRICOT",
    
    "GLR_YELLOW",
    "GLR_BEIGE",
    
    "GLR_GREEN",
    "GLR_MINT",
    
    "GLR_PURPLE",
    "GLR_LAVENDER",
    
    "GLR_GREY",
    "GLR_WHITE"
};

static n_byte old_color[12 * 3] = {
    230,235,75, /* red */
    250,190,212, /* pink */
    
    245, 230, 48, /* orange */
    255, 215, 180, /* apricot */

    255, 225, 25, /* yellow */
    255, 250, 200, /* beige */
    
    60, 180, 75, /* green */
    170,255,195, /* mint */
    
    145, 30, 180, /* purple */
    220,190, 225, /* lavender */
    
    128, 128, 128, /* grey */
    255, 255, 255 /* white */
};

n_byte* oldcolor_get(void);

void oldcolor_set(const GLR_COLOR color);

void filehandling_string_out(n_string_block new_string, n_string actual, n_string add_value);

void filehandling_main_name(n_string name_dot, n_string main_name);

n_int filehandling_dt_string(n_string string);

void filehandling_json_out(void * returned_blob, n_object_type type_of, n_string file_out);

void * filehandling_gather( n_file * in_file, n_object_type * type_of );

n_file * filehandling_file( n_string file_in );

void filehandling_set_object(n_object * value, n_string main_name_str);

n_int filehandling_save(void);

void draw_dimensions(n_int x, n_int y);

void draw_return_object(n_array * terrain_array, n_string terrain);

unsigned char * read_png_file(char * filename, png_t * ptr);

int write_png_file(char* filename, int width, int height, unsigned char *buffer);

int mainish( int argc, const char *argv[] );

unsigned char * draw_screen_buffer(void);

void draw_cleanup(void);

void zoom_set(n_int *resolution);

void zoom_interaction(double zoom_interaction);

double zoom_get(void);

void draw_screen_do_update(void);

void main_capture(const char *arg);

void draw_mouse(n_int dx, n_int dy);

void draw_mouse_up(void);

void draw_dp(n_int delta_x, n_int delta_y);

void draw_key_received(n_int value);

n_int dt_string(n_string string);
