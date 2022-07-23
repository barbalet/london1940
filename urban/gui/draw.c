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

#include "../../apesdk/render/glrender.h"
#include "../../apesdk/entity/entity.h"
#include "../game/mushroom.h"
#include "../game/city.h"


#define TREE_COLOR     GLR_LIGHT_GREEN
#define ENTITY_COLOR   GLR_RED
#define FENCE_COLOR    GLR_ORANGE
#define ROAD_COLOR     GLR_BLACK
#define DEBUG_COLOR    GLR_RED
#define WINDOW_COLOR   GLR_LIGHT_GREY
#define DOOR_COLOR     GLR_LIGHT_GREY
#define ROOM_COLOR     GLR_GREY
#define WALL_COLOR     GLR_DARK_GREY

/* cream 239, 215, 184 - background */
/* red 176, 24, 8 - road */
/* light green 192, 224, 79 - tree */
/* light brown 104, 96, 64 - fence color */
/* black 24, 8, 7 - wall */
/* orange 224, 87, 14 - debug color*/
/* light grey 193, 183, 183 - windows / door  */
/* dark grey 16, 24, 24 - wall color */
/* dark grey 168, 151, 128 - room color */

void draw_tree(simulated_tree * tree)
{
    n_vect2 quad[4];
    n_vect2 unit[2] = {1, 1};
    n_int   loop = 0;
    
    while (loop < POINTS_PER_TREE)
    {
        vect2_copy(&quad[0], &tree->center);
        
        vect2_direction(&quad[1], loop * (256/POINTS_PER_TREE), 600);
        vect2_multiplier(&quad[1], &quad[1], (n_vect2 *)&unit, tree->points[loop&(POINTS_PER_TREE-1)] * tree->radius, 360);
        
        loop++;
        vect2_direction(&quad[2], loop * (256/POINTS_PER_TREE), 600);
        vect2_multiplier(&quad[2], &quad[2], (n_vect2 *)&unit, tree->points[loop&(POINTS_PER_TREE-1)] * tree->radius, 360);
        
        loop++;
        vect2_direction(&quad[3], loop * (256/POINTS_PER_TREE), 600);
        vect2_multiplier(&quad[3], &quad[3], (n_vect2 *)&unit, tree->points[loop&(POINTS_PER_TREE-1)] * tree->radius, 360);
        
        vect2_subtract(&quad[1], &quad[0], &quad[1]);
        vect2_subtract(&quad[2], &quad[0], &quad[2]);
        vect2_subtract(&quad[3], &quad[0], &quad[3]);

        glrender_color(TREE_COLOR);
        
        glrender_quads(quad, 1);
        
        glrender_line(&quad[1], &quad[2]);
        glrender_line(&quad[2], &quad[3]);
        
    }
}

void draw_each_fence(simulated_fence * fence)
{
    glrender_wide_line();
    glrender_color(FENCE_COLOR);
    glrender_line(&fence->points[0], &fence->points[1]);
    
    matrix_add_fence(&fence->points[0], &fence->points[1]);
    
    glrender_thin_line();
}

void draw_fence(simulated_fence * fences)
{
    n_int count = 0;
    while (count < 8)
    {
        draw_each_fence(&fences[count++]);
    }
}

void draw_debug_number_point(n_vect2 * point, n_int number)
{
    n_string_block number_string = "1";
    number_string[0] += number - 1;
    glrender_color(DEBUG_COLOR);
    glrender_string(number_string, point->x, point->y);
}

void draw_debug_number_fill(n_vect2 * points, n_int number)
{
    n_vect2  average = {0, 0};
    
    vect2_add(&average, &points[0], &points[1]);
    vect2_add(&average, &average, &points[2]);
    vect2_add(&average, &average, &points[3]);
    
    average.x = average.x / 4;
    average.y = average.y / 4;

    draw_debug_number_point(&average, number);
}

void draw_each_path(simulated_path * path)
{
    glrender_color(ROAD_COLOR);

    glrender_quads(&path->points[0], 1);
    glrender_wide_line();
    glrender_line(&path->points[0], &path->points[1]);
    glrender_line(&path->points[2], &path->points[1]);
    glrender_line(&path->points[2], &path->points[3]);
    glrender_line(&path->points[0], &path->points[3]);
    glrender_thin_line();
#ifdef DEBUG_ROAD_NUMBER
    draw_debug_number_point(&path->points[0], 0);
    draw_debug_number_point(&path->points[1], 1);
    draw_debug_number_point(&path->points[2], 2);
    draw_debug_number_point(&path->points[3], 3);
#endif
}

void draw_path(simulated_path_group * group)
{
    n_int count = 0;
    while (count < group->number)
    {
        draw_each_path(&group->paths[count]);
#ifdef DEBUG_ROAD_NUMBER
        draw_debug_number_fill(group->paths[count].points, count);
#endif
        count++;
    }
}


void draw_park(simulated_park * park)
{
    n_int count = 0;
    
    draw_path(&park->road);
    //draw_path(&park->footpath);
    
    while (count < 16)
    {
        if (tree_populated(&park->trees[count][0]))
        {
            draw_tree(&park->trees[count][0]);
        }
        if (tree_populated(&park->trees[count][1]))
        {
            draw_tree(&park->trees[count][1]);
        }
        if (tree_populated(&park->trees[count][2]))
        {
            draw_tree(&park->trees[count][2]);
        }
        if (tree_populated(&park->trees[count][3]))
        {
            draw_tree(&park->trees[count][3]);
        }
        count++;
    }
}

static void draw_house_room(simulated_room * room, n_int room_number)
{
    glrender_color(WALL_COLOR);
    glrender_quads(&room->points[4], 1);
    glrender_wide_line();

    glrender_line(&room->points[4], &room->points[5]);
    glrender_line(&room->points[5], &room->points[6]);
    glrender_line(&room->points[6], &room->points[7]);
    glrender_line(&room->points[4], &room->points[7]);
    glrender_thin_line();

    glrender_color(ROOM_COLOR);
    glrender_quads(&room->points[0], 0);
    
    matrix_add_wall(&room->points[0], &room->points[1]);
    matrix_add_wall(&room->points[1], &room->points[2]);
    matrix_add_wall(&room->points[2], &room->points[3]);
    matrix_add_wall(&room->points[3], &room->points[0]);
    
    glrender_color(WINDOW_COLOR);

    if (house_window_present(&room->points[8]))
    {
        glrender_line(&room->points[8], &room->points[9]);
    }
    if (house_window_present(&room->points[10]))
    {
        glrender_line(&room->points[10], &room->points[11]);
    }
    if (house_window_present(&room->points[12]))
    {
        glrender_line(&room->points[12], &room->points[13]);
    }
    if (house_window_present(&room->points[14]))
    {
        glrender_line(&room->points[14], &room->points[15]);
    }
#ifdef DEBUG_ROOM_NUMBER
    draw_debug_number_fill(room->points, room_number);
#endif
}

static void draw_house_door(simulated_room * room)
{
    glrender_color(DOOR_COLOR);

    glrender_wide_line();
    
    if (house_door_present(&room->points[16]))
    {
        glrender_quads(&room->points[16], 1);
        
        glrender_line(&room->points[16], &room->points[17]);
        glrender_line(&room->points[17], &room->points[18]);
        glrender_line(&room->points[18], &room->points[19]);
        glrender_line(&room->points[16], &room->points[19]);
    }
    if (house_door_present(&room->points[20]))
    {
        glrender_quads(&room->points[20], 1);
        
        glrender_line(&room->points[20], &room->points[21]);
        glrender_line(&room->points[21], &room->points[22]);
        glrender_line(&room->points[22], &room->points[23]);
        glrender_line(&room->points[20], &room->points[23]);
    }
    
    if (house_door_present(&room->points[24]))
    {
        glrender_quads(&room->points[24], 1);
        
        glrender_line(&room->points[24], &room->points[25]);
        glrender_line(&room->points[25], &room->points[26]);
        glrender_line(&room->points[26], &room->points[27]);
        glrender_line(&room->points[24], &room->points[27]);
    }
    
    if (house_door_present(&room->points[28]))
    {
        glrender_quads(&room->points[28], 1);
        
        glrender_line(&room->points[28], &room->points[29]);
        glrender_line(&room->points[29], &room->points[30]);
        glrender_line(&room->points[30], &room->points[31]);
        glrender_line(&room->points[28], &room->points[31]);
    }
    glrender_thin_line();

}

static void draw_each_house(simulated_building * building)
{
    n_int   loop = 0;
    while (loop < building->roomcount)
    {
        draw_house_room(&building->room[loop], loop);
        loop++;
    }
    loop = 0;
    while (loop < building->roomcount)
    {
        draw_house_door(&building->room[loop]);
        loop++;
    }
    loop = 0;
    while (loop < 4)
    {
        if (tree_populated(&building->trees[loop]))
        {
            draw_tree(&building->trees[loop]);
        }
        loop++;
    }
}

void draw_house(simulated_building * buildings)
{
    n_int count = 0;
    while (count < 16)
    {
        draw_each_house(&buildings[count++]);
    }
}

void draw_twoblock(simulated_twoblock * twoblock)
{
    draw_path(&twoblock->road);
    //draw_path(&twoblock->footpath);
    draw_fence((simulated_fence*)&(twoblock->fence));
    draw_house((simulated_building*)&(twoblock->house));
}

void draw_neighborhood(void)
{
    n_int  twoblock_count, park_count, fence_count;
    simulated_twoblock *twoblock = neighborhoood_twoblock(&twoblock_count);
    simulated_park     *park = neighborhoood_park(&park_count);
    simulated_fence    *fences = neighborhoood_fence(&fence_count);

    n_int   count = 0;
    while (count < twoblock_count)
    {
        draw_twoblock(&twoblock[count++]);
    }
    count = 0;
    while (count < park_count)
    {
        draw_park(&park[count++]);
    }
    
    count = 0;
    while (count < fence_count)
    {
        draw_fence(&fences[count++]);
    }
}

void draw_city(void)
{
    simulated_being *beings = city_beings();
    n_uint beings_number = city_beings_number();
    n_uint loop = 0;
    /*n_int  line_count = 0;
    matrix_plane * span_values;*/
    
    glrender_color(ENTITY_COLOR);
    
    glrender_wide_line();
    while (loop < beings_number)
    {
        n_vect2 line_start;
        n_vect2 line_end;
        n_vect2 facing;
        
        line_start.x = line_end.x = being_location_x(&beings[loop]);
        line_start.y = line_end.y = being_location_y(&beings[loop]);
        
        being_facing_vector(&beings[loop], &facing, 50);
        
        city_translate(&line_start);
        city_translate(&line_end);
        
        vect2_add(&facing, &line_start, &facing);
        
        glrender_line(&line_start, &facing);
        
        line_start.x -= 5;
        line_end.x += 5;
        
        glrender_line(&line_start, &line_end);
        
        line_start.x += 5;
        line_end.x -= 5;
        
        line_start.y -= 5;
        line_end.y += 5;
        
        glrender_line(&line_start, &line_end);
        
        loop++;
    }

}

void draw_game_color(n_byte2 * fit)
{
    glrender_color_map(fit);
}

n_int draw_game_scene(n_int dim_x, n_int dim_y)
{
    n_int draw_game_scene_done = 0;
    glrender_background_green();
    if (glrender_scene_done())
    {
        glrender_start_display_list();
        draw_neighborhood();
        /*game_grid();*/
        glrender_end_display_list();
    }
    else
    {
        n_vect2 center;
        center.x = 0 - (dim_x >> 1);
        center.y = 0 - (dim_y >> 1);
        glrender_delta_move(&center, agent_location(), agent_facing(), agent_zooming());
        draw_game_scene_done = 1;
    }
    glrender_start_active_list();
    draw_city();
    glrender_end_active_list();
    matrix_account();
    return draw_game_scene_done;
}

void draw_render(n_byte * buffer, n_int dim_x, n_int dim_y)
{
    glrender_set_size(dim_x, dim_y);
    
    if (buffer)
    {
        glrender_render_display(buffer);
    }
    if (buffer)
    {
        glrender_render_active(buffer);
#ifdef DEBUG_BLOCKING_BOUNDARIES
        glrender_render_lines(buffer, matrix_draw_block());
        glrender_render_lines(buffer, matrix_draw_identifier());
#endif
    }
}

void draw_init(void)
{/* SIMSEALION_COLOR_SET*/
//    n_byte old_color[32] = {
//     0, 239, 215, 184,
//     0, 192, 224, 79,
//     0, 224, 87, 14,
//     0, 104, 96, 64,
//     
//     0, 193, 183, 183,
//     0, 168, 151, 128,
//     0, 16, 24, 24,
//     0, 176, 24, 8};
//    
//    glrender_color_map_replace((n_byte4*)old_color);
    
    glrender_init();
    
    matrix_init();
}

void draw_close(void)
{
    glrender_close();
    matrix_close();
}
