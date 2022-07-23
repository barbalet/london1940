/****************************************************************
 
 draw.c
 
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

#include "mapgui.h"

/*#define FILE_LOCAL*/

#ifndef FILE_LOCAL
static png_t png_data;
#endif

static unsigned char * buffer = 0L;
unsigned char * screen_buffer = 0L;

static n_int resolution[2];
static n_int map_width = -1;

extern unsigned char * read_png_file(char * filename, png_t * ptr);

#define SCREEN_WIDTH              (1024)
#define SCREEN_HEIGHT             (768)

static double max_zoom = 1E+00;
static double zoom = 1E+00;

static n_int point_x = 0;
static n_int point_y = 0;

void draw_point_check(void)
{
    if (point_x < 0)
    {
        point_x = 0;
    }
    
    if (point_y < 0)
    {
        point_y = 0;
    }
    
    if (point_x > (resolution[0] - SCREEN_WIDTH))
    {
        point_x = (resolution[0] - SCREEN_WIDTH);
    }
    
    if (point_y > (resolution[1] - SCREEN_HEIGHT))
    {
        point_y = (resolution[1] - SCREEN_HEIGHT);
    }
}

void draw_key_received(n_int value)
{
    if (value == 's')
    {
        printf("Save!\n");
        if (filehandling_save() == -1)
        {
        }
    }
    
    /*left arrow*/
    if (value == 28)
    {
        point_x  -=8;
    }
    /*right arrow*/
    if (value == 29)
    {
        point_x  +=8;
    }
    
    
    /*up arrow*/
    if (value == 30)
    {
        point_y  -=8;
    }
    /*down arrow*/
    if (value == 31)
    {
        point_y  +=8;
    }
    
    draw_point_check();
    draw_screen_do_update();
}

void draw_dp(n_int delta_x, n_int delta_y)
{
    point_x -= delta_x;
    point_y -= delta_y;

    draw_point_check();

    draw_screen_do_update();
}

void draw_mouse(n_int dx, n_int dy)
{

}

void draw_mouse_up(void)
{

}

unsigned char * draw_screen_buffer(void)
{
    return screen_buffer;
}

void zoom_set(n_int *resolution)
{
    double zoomx = resolution[0] / SCREEN_WIDTH;
    double zoomy = resolution[1] / SCREEN_HEIGHT;

    max_zoom = zoomx;
    
    if (zoomx > zoomy)
    {
        max_zoom = zoomy;
    }
    
    zoom = 1;
}

void zoom_interaction(double zoom_interaction)
{
//    zoom = zoom * (zoom_interaction + 1);
//
//    printf("zoom_interaction %f\n", zoom_interaction);
//
//    if (zoom > max_zoom)
//    {
//        zoom = max_zoom;
//    }
//    if (zoom < 1)
//    {
//        zoom = 1;
//    }
//    draw_screen_do_update();
    
    zoom = 1;
}


void zoom_plus(void)
{
    zoom = zoom * 1.1;
    if (zoom > max_zoom)
    {
        zoom = max_zoom;
    }
}

void zoom_minus(void)
{
    zoom = zoom * 0.9;
    if (zoom < 1)
    {
        zoom = 1;
    }
}

double zoom_get(void)
{
    return zoom;
}

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @%s, %ld\n", error_text, location, line_number );
    return -1;
}

extern int write_png_file(char* filename, int width, int height, unsigned char *buffer);

n_int dimen_x = 0, dimen_y = 0;

void draw_dimensions(n_int x, n_int y)
{
    dimen_x = x;
    dimen_y = y;
}

void draw_pixel(n_int x, n_int y)
{

    if ((x >= 0) && (x < dimen_x) && (y >= 0) && (y < dimen_y))
    {
        n_int location = (x + (y * dimen_x))*3;
        n_byte *old_color = oldcolor_get();
        
        if (buffer)
        {
            buffer[location] = old_color[0];
            buffer[location+1] = old_color[1];
            buffer[location+2] = old_color[2];
        }
    }

}


n_byte draw_trickypixel( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    draw_pixel(px, py);
    return 0;
}


void draw_line(n_vect2 *in, n_vect2 *out)
{
    n_join draw_draw;
    draw_draw.pixel_draw = draw_trickypixel;
    math_line_vect(in, out, &draw_draw);
}

 void draw_identify(n_object * object,
                    n_array **polygons,
                    n_array **points,
                    n_array **links,
                    n_array **widths,
                    n_array ** lines,
                    n_array ** pointdirections)
{
     n_string str_polygons = obj_contains( object, "polygons", OBJECT_ARRAY );
     n_string str_points = obj_contains( object, "points", OBJECT_ARRAY );
     n_string str_links = obj_contains( object, "links", OBJECT_ARRAY );
     n_string str_widths = obj_contains( object, "widths", OBJECT_ARRAY );
     n_string str_lines = obj_contains( object, "lines", OBJECT_ARRAY );
     n_string str_pointdirections = obj_contains( object, "pointdirections", OBJECT_ARRAY );

     if (str_pointdirections)
     {
         printf(" podi ");
         *pointdirections = obj_get_array(str_pointdirections);
     }
     
     if (str_polygons)
     {
         printf(" poly ");
         *polygons = obj_get_array(str_polygons);
     }
     if (str_points)
     {
         printf(" poin ");
         *points = obj_get_array(str_points);
     }
     if (str_links)
     {
         printf(" link ");
         *links = obj_get_array(str_links);
     }
     if (str_widths)
     {
         printf(" widt");
         *widths = obj_get_array(str_widths);
     }
     if (str_lines)
     {
         printf(" line ");
         *lines = obj_get_array(str_lines);
     }

     printf("\n");
 }

void draw_splodge(n_vect2 * splodge, n_int filled)
{
    n_int square_ends = -1;
    
    while (square_ends < 2)
    {
        draw_pixel(splodge->x + 2, splodge->y + square_ends);
        draw_pixel(splodge->x - 2, splodge->y + square_ends);
        draw_pixel(splodge->x + square_ends, splodge->y + 2);
        draw_pixel(splodge->x + square_ends, splodge->y - 2);
        
        if (filled)
        {
            draw_pixel(splodge->x + square_ends, splodge->y + 2);
            draw_pixel(splodge->x + square_ends, splodge->y + 1);
            draw_pixel(splodge->x + square_ends, splodge->y);
            draw_pixel(splodge->x + square_ends, splodge->y - 1);
            draw_pixel(splodge->x + square_ends, splodge->y - 2);
        }
        square_ends++;
    }
}

void draw_return_object(n_array * terrain_array, n_string terrain)
{
    n_array * terrain_array_follow = 0L;
    while ( ( terrain_array_follow = obj_array_next( terrain_array, terrain_array_follow ) ) )
    {
        if (terrain_array_follow){
            if(terrain_array_follow->type == OBJECT_OBJECT)
            {
                n_object * object = obj_get_object(terrain_array_follow->data);
                n_array * polygons = 0L;
                n_array * points = 0L;
                n_array * links = 0L;
                n_array * widths = 0L;
                n_array * lines = 0L;
                n_array * pointdirections = 0L;
                
                if (object->primitive.type == OBJECT_STRING)
                {
                    n_string space_removed = object->primitive.data;
                    
                    n_int length = io_length(space_removed,STRING_BLOCK_SIZE);
                    
                    
                    if (io_find(space_removed,0,length,"woodland", 8)>-1)
                    {
                        printf("\nwoodland\n");
                        oldcolor_set(GLR_GREEN);
                    }
                    
                    if (io_find(space_removed,0,length,"rivers", 6)>-1)
                    {
                        printf("\nrivers\n");
                        oldcolor_set(GLR_PURPLE);
                    }
                    
                    if (io_find(space_removed,0,length,"sand", 5)>-1)
                    {
                        printf("\nsand\n");
                        oldcolor_set(GLR_YELLOW);
                    }
                    
                    if (io_find(space_removed,0,length,"sea", 3)>-1)
                    {
                        printf("\nsea\n");
                        oldcolor_set(GLR_PURPLE);
                    }
                    
                    if (io_find(space_removed,0,length,"railway stations", 16)>-1)
                    {
                        printf("\nrailway stations\n");
                        oldcolor_set(GLR_WHITE);
                    }
                    if (io_find(space_removed,0,length,"orchards", 8)>-1)
                    {
                        printf("\norchards\n");
                        oldcolor_set(GLR_GREEN);
                    }
                    if (io_find(space_removed,0,length,"bridges", 7)>-1)
                    {
                        printf("\nbridges\n");
                        oldcolor_set(GLR_GREY);
                    }
                    if (io_find(space_removed,0,length,"junctions", 9)>-1)
                    {
                        printf("\njunctions\n");
                        oldcolor_set(GLR_ORANGE);
                    }
                    
                    if (io_find(space_removed,0,length,"main roads", 10)>-1)
                    {
                        printf("\nmain roads\n");
                        oldcolor_set(GLR_WHITE);
                    }
                    if (io_find(space_removed,0,length,"minor roads", 11)>-1)
                    {
                        printf("\nminor roads\n");
                        oldcolor_set(GLR_GREY);
                    }
                    if (io_find(space_removed,0,length,"buildings", 9)>-1)
                    {
                        printf("\nbuildings\n");
                        oldcolor_set(GLR_MINT);
                    }
                    
                    printf("%s %s %s %ld", terrain, object->name, space_removed, io_length(space_removed, STRING_BLOCK_SIZE));
                }

                draw_identify(object, &polygons, &points, &links, &widths, &lines, &pointdirections);

                if (polygons)
                {
                    n_array * polygons_array_follow = 0L;
                    while ( ( polygons_array_follow = obj_array_next( polygons, polygons_array_follow ) ) )
                    {
                        if (polygons_array_follow){
                            if(polygons_array_follow->type == OBJECT_ARRAY)
                            {
                                memory_list * memory_list = object_list_vect2(obj_get_array(polygons_array_follow->data));
                                
                                if (memory_list == 0L)
                                {
                                    exit(EXIT_FAILURE);
                                }
                                
                                n_vect2 * vect_data = (n_vect2 *)memory_list->data;
                                n_int loop = 1;
                                
                                while (loop < memory_list->count)
                                {
                                    draw_splodge(&vect_data[loop-1], 0);
                                    draw_splodge(&vect_data[loop], 0);
                                    
                                    draw_line(&vect_data[loop - 1], &vect_data[loop]);
                                    loop++;
                                }
                                memory_list_free(& memory_list);
                            }
                        }
                    }
                }

                
                if (points)
                {
                    memory_list * processing_list = 0L;
                    memory_list * memory_list = object_list_vect2(points);
                    
                    
                    if (memory_list == 0L)
                    {
                        exit(EXIT_FAILURE);
                    }
                    
                    n_vect2 * vect_data = (n_vect2 *)memory_list->data;
                    n_int loop = 0;
                                        
                    while (loop < memory_list->count)
                    {
                        draw_pixel(vect_data[loop].x, vect_data[loop].y);
                        draw_splodge(&vect_data[loop], 0);
                        loop++;
                    }

                    oldcolor_set(GLR_WHITE);
                    
                    /* gather lines */
                    processing_list = vect2_point_to_lines(memory_list, 400);
                    vect2_x_entry_bubblesort(processing_list);
                    
                    loop = 0;
                    n_vect2 * processing_data = (n_vect2 *)processing_list->data;
                    /* show lines */

                    while (loop < processing_list->count)
                    {
                        if (processing_data[loop].x != -1)
                        {
                            n_vect2 * vect_in = &vect_data[processing_data[loop].x];
                            n_vect2 * vect_out = &vect_data[processing_data[loop].y];

                            draw_line(vect_in, vect_out);
                        }
                        loop++;
                    }
                    memory_list_free(&processing_list);
                    memory_list_free(&memory_list);
                    //io_file_debug(unknown_json(points, OBJECT_ARRAY));
                    
                    oldcolor_set(GLR_GREY);

                }
            }
        }
    }
}

void draw_screen_update(unsigned char * ds, n_int res0)
{
    n_int px = 0;
    while (px < SCREEN_WIDTH)
    {
        n_int p_x = (px * zoom_get())+ point_x;
        n_int py = 0;
        while (py < SCREEN_HEIGHT)
        {
            n_int screen_value = (px + (py * SCREEN_WIDTH))*4;
            n_int p_y = (py * zoom_get()) + point_y;
            n_int point_value = (p_x + (p_y * res0))*3;
            
            ds[screen_value] = 0;
            ds[screen_value + 1] = buffer[point_value];
            ds[screen_value + 2] = buffer[point_value + 1];
            ds[screen_value + 3] = buffer[point_value + 2];
            py++;
        }
        px++;
    }
}

void draw_check_arg_count(n_int count)
{
    if (count == 1)
    {
        printf("Too many args!\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        if (count > 2)
        {
            printf("Too few args!\n");
            exit(EXIT_FAILURE);
        }
    }
}

void draw_cleanup(void)
{
    if (screen_buffer)
    {
        memory_free((void **)&screen_buffer);

    }
    if (buffer)
    {
        memory_free((void **)&buffer);
    }
}


#ifdef FILE_LOCAL
extern n_char folkestone[129709];
#endif

void draw_init(n_string name_dot)
{
    n_string_block main_name = {0};
    filehandling_main_name(name_dot, main_name);

    n_object_type   type_of = OBJECT_EMPTY;
    
    n_string_block png_filename_in = {0};
    n_string_block json_filename_in = {0};

    void * test_output = 0L;

    printf("main_name %s\n", main_name);
    
    filehandling_string_out(png_filename_in,  main_name, ".png");
    filehandling_string_out(json_filename_in, main_name, ".json");
#ifdef FILE_LOCAL
    test_output = filehandling_gather( io_file_new_from_string(folkestone, 129709), &type_of );
#else
    test_output = filehandling_gather( filehandling_file((n_string)json_filename_in), &type_of );
#endif
    
    if (test_output == 0L)
    {
        printf("Object didn't load!\n");
        exit(EXIT_FAILURE);
    }
    
    if (type_of != OBJECT_OBJECT)
    {
        printf("Wrong Type of Object type %s\n", object_type_string(type_of));
        exit(EXIT_FAILURE);
    }
            
    n_object * returned_object = (n_object *) test_output;
    
    filehandling_set_object(returned_object, main_name);
    
    n_string terrain_string = obj_contains(returned_object, STRING_TERRAIN, OBJECT_ARRAY);
    n_string urban_string = obj_contains(returned_object, STRING_URBAN, OBJECT_ARRAY);
    
    n_int resolution_okay = obj_contains_array_numbers(returned_object, STRING_RESOLUTION, (n_int *)&resolution, 2);
            
    draw_dimensions(resolution[0], resolution[1]);
#ifdef FILE_LOCAL
    buffer = (n_byte *)memory_new(resolution[0] * resolution[1]*4);
#else
    buffer = read_png_file(png_filename_in, &png_data);
#endif
    if (screen_buffer == 0L)
    {
        printf("screen_buffer init\n");
        screen_buffer = (unsigned char *) memory_new(SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    }

    n_array * terrain_array = obj_get_array(terrain_string);
    n_array * urban_array = obj_get_array(urban_string);

    draw_return_object(terrain_array, STRING_TERRAIN);
    draw_return_object(urban_array, STRING_URBAN);
    
    printf("resolution_okay %ld {%ld, %ld}\n", resolution_okay, resolution[0], resolution[1]);

    zoom_set(resolution);
    
    map_width = resolution[0];
}


void main_capture(const char *arg)
{
    draw_init((n_string) arg);

    printf("draw_init %s\n", arg);

    printf("map_width %ld\n", map_width);
    
    if (screen_buffer)
    {
        printf("screen_buffer\n");
    }

    if ((map_width > 0) && screen_buffer)
    {
        draw_screen_update(screen_buffer, map_width);
    }
}

void draw_screen_do_update(void)
{
    if ((map_width > 0) && screen_buffer)
    {
        draw_screen_update(screen_buffer, map_width);
    }
}


int mainish( int argc, const char *argv[] )
{
    n_string_block png_filename_out = {0};
    

#ifdef FILE_LOCAL
    filehandling_string_out(png_filename_out, (n_string) "/Users/barbalet/Documents/output.json", "_mix.png");

    main_capture("/Users/barbalet/Documents/output.json");
#else
    filehandling_string_out(png_filename_out, (n_string) argv[1], "_mix.png");

    draw_check_arg_count(argc);

    main_capture(argv[1]);
#endif
    if (screen_buffer)
    {
        write_png_file(png_filename_out, (int)resolution[0], (int)resolution[1], buffer);
        printf("write_png_file %s\n", png_filename_out);
    }

    draw_cleanup();

    exit(EXIT_SUCCESS);
}
