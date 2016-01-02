/****************************************************************
 
 house.c
 
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

#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>
#import <OpenGL/OpenGL.h>

#include "mushroom.h"



extern n_int   terrain_x;
extern n_int   terrain_y;

void house_transform(noble_building * building, n_vect2 * center, n_int direction)
{
    n_int   loop_room = 0;
    n_vect2 * min_max = vect2_min_max_init();
    n_vect2 direction_vector;
    n_vect2 points_center;

    if (min_max == 0L)
    {
        return;
    }
    while (loop_room < building->roomcount)
    {
        noble_room * room = &building->room[loop_room++];
        vect2_min_max(room->points, POINTS_PER_ROOM_STRUCTURE, min_max);
    }
    
    vect2_center(&points_center, &min_max[1], &min_max[0]);
    vect2_direction(&direction_vector, direction, 1);
    
    io_free((void **)&min_max);
    
    loop_room = 0;
    while (loop_room < building->roomcount)
    {
        n_int   loop = 0;
        noble_room * room = &building->room[loop_room++];
        while (loop < POINTS_PER_ROOM)
        {
            vect2_subtract(&room->points[loop], &room->points[loop], &points_center);
            vect2_rotation(&room->points[loop], &direction_vector);
            vect2_add(&room->points[loop], &room->points[loop], center);
            loop++;
        }
    }
}

void house_vertex(n_vect2 * point)
{
    glVertex2i((GLint)point->x, (GLint)point->y);
}


static void house_draw_fence(noble_fence * fence)
{
    glLineWidth(100);

    glColor3f(0.5, 0.2, 0.0);
    glBegin(GL_LINES);
    
    house_vertex(&fence->points[0]);
    house_vertex(&fence->points[1]);

    glEnd();
    glLineWidth(1);

}


static void house_draw_road(noble_road * road)
{
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
    house_vertex(&road->points[0]);
    house_vertex(&road->points[1]);
    house_vertex(&road->points[2]);
    house_vertex(&road->points[3]);
    glEnd();
}

static void house_drawroom(noble_room * room)
{
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_QUADS);
    house_vertex(&room->points[4]);
    house_vertex(&room->points[5]);
    house_vertex(&room->points[6]);
    house_vertex(&room->points[7]);
    glEnd();
    
    glColor3f(0.7, 0.7, 0.7);
    
    glBegin(GL_LINE_LOOP);
    house_vertex(&room->points[0]);
    house_vertex(&room->points[1]);
    house_vertex(&room->points[2]);
    house_vertex(&room->points[3]);
    glEnd();

    glColor3f(0.9, 0.9, 0.9);
    
    if ((room->window&1) == 1)
    {
        glBegin(GL_LINES);
        house_vertex(&room->points[8]);
        house_vertex(&room->points[9]);
        glEnd();

	}
    if ((room->window&2) == 2)
    {
        glBegin(GL_LINES);
        house_vertex(&room->points[10]);
        house_vertex(&room->points[11]);
        glEnd();

	}
    if ((room->window&4) == 4)
    {
        glBegin(GL_LINES);
        house_vertex(&room->points[12]);
        house_vertex(&room->points[13]);
        glEnd();

	}
    if ((room->window&8) == 8)
    {
        glBegin(GL_LINES);
        house_vertex(&room->points[14]);
        house_vertex(&room->points[15]);
        glEnd();
	}
}

void house_draw(noble_building * building)
{
    n_int   loop_room = 0;
    while (loop_room < building->roomcount)
    {
        house_drawroom(&building->room[loop_room]);
        loop_room++;
    }
}

/* 1=north, 2=south, 4=east, 8=west */
static void house_construct(noble_room * room, n_int topx, n_int topy, n_int botx, n_int boty, n_byte window)
{
    room->window = window;
    
    if (topx > botx)
    {
		n_int temp = botx;
		botx = topx;
		topx = temp;
	}
	if (topy > boty)
    {
		n_int temp = boty;
		boty = topy;
		topy = temp;
	}
    
    vect2_populate(&room->points[0], topx, topy);
    vect2_populate(&room->points[1], topx, boty);
    vect2_populate(&room->points[2], botx, boty);
    vect2_populate(&room->points[3], botx, topy);
    vect2_populate(&room->points[4], topx+6, topy+6);
    vect2_populate(&room->points[5], topx+6, boty-6);
    vect2_populate(&room->points[6], botx-6, boty-6);
    vect2_populate(&room->points[7], botx-6, topy+6);
    
    if ((room->window&1) == 1)
    {
        vect2_populate(&room->points[8], topx+3, topy+10);
        vect2_populate(&room->points[9], topx+3, boty-10);
    }
    if ((room->window&2) == 2)
    {
        vect2_populate(&room->points[10], botx-3, topy+10);
        vect2_populate(&room->points[11], botx-3, boty-10);
    }
    if ((room->window&4) == 4)
    {
        vect2_populate(&room->points[12], topx+10, boty-3);
        vect2_populate(&room->points[13], botx-10, boty-3);
    }
    if ((room->window&8) == 8)
    {
        vect2_populate(&room->points[14], topx+3, topy+10);
        vect2_populate(&room->points[15], topx+3, boty-10);
    }
}

static n_int house_plusminus(n_int num, n_byte2 * seed)
{
	n_int	tmp=0;
	while ((tmp > -3) && (tmp < 3))
    {
		tmp = (math_random(seed) % 11) - 5;
	}
	return (tmp);
}

static n_int house_genetics(n_int * house, n_byte2 * seed)
{
	n_int	count = 1;
	n_int   lpend = house[0] = 5 + (math_random(seed) % 11); /* 5 to 15 */
	n_int   lp = (math_random(seed) % 3) + 2;
	while (lp < lpend)
    {
		house[(count * 3) - 1] = 0;
		house[count * 3] = lp;
		house[(count * 3) + 1] = house_plusminus(6, seed);
		count++;
        lp += (math_random(seed) % 3) + 2;
	}
    house[0] ++;
	return count;
}

noble_building * house_create(n_byte2 * seed)
{
    n_int	         pointp = -4, pointm = -4;
    n_int	         loop = 2;
    noble_building * building = io_new(sizeof(noble_building));
    n_int          * house;
    noble_room     * room;
    n_int            abstract_rooms;
    n_int            roomcount = 0;
    
    if (building == 0L)
    {
        return 0L;
    }
    
    abstract_rooms = house_genetics(building->house, seed);
    house = building->house;
    room  = building->room;
    
	if (house[4] > 1)
    {
        house_construct(&room[roomcount++], -8, -4, 8+(house[4]*20), -4+(house[3]*20), 10);//north + east
        pointp = (house[3] * 20) - 4;
    }
    else
    {
        house_construct(&room[roomcount++], -8 + (house[4]*20), -4, 8, -4+(house[3]*20), 9);//north + west
        pointm = (house[3] * 20) - 4;
    }
    
    while(loop < (abstract_rooms - 1))
    {
        if (house[(loop * 3) + 1] > 1)
        {
            house_construct(&room[roomcount++], 8, -4+(house[loop*3]*20), 8+(house[(loop*3)+1]*20), pointp, 2);//east
            pointp = (house[loop * 3] * 20) - 4;
        }
        else
        {
            house_construct(&room[roomcount++], -8 + (house[(loop*3)+1]*20), -4+(house[loop*3]*20), -8, pointm, 1);//west
            pointm = (house[loop * 3] * 20) - 4;
        }
        loop++;
    }
		
	if (house[((abstract_rooms - 1) * 3) + 1] > 1)
    {
        house_construct(&room[roomcount++], -8, (house[0]*20)+4, 8+(house[((abstract_rooms-1)*3)+1]*20), -4+(house[(abstract_rooms-1)*3]*20), 4);
        house_construct(&room[roomcount++], -8, pointm, -8-(house[((abstract_rooms-1)*3)+1]*20), (house[0]*20)+4, 1);
        if (abstract_rooms != loop)
        {
            house_construct(&room[roomcount++], 8, -4+(house[(abstract_rooms-1)*3]*20),8+(house[((abstract_rooms-1)*3)+1]*20), pointp, 2);
        }
    }
    else
    {
        house_construct(&room[roomcount++], 8, (house[0]*20)+4, 8+(house[((abstract_rooms-1)*3)+1]*20), -4+(house[(abstract_rooms-1)*3]*20), 4);
        house_construct(&room[roomcount++], 8, pointp, 8-(house[((abstract_rooms-1)*3)+1]*20), (house[0]*20)+4, 2);
        if (abstract_rooms != loop)
        {
            house_construct(&room[roomcount++], -8, -4+(house[(abstract_rooms-1)*3]*20), -8+(house[((abstract_rooms-1)*3)+1]*20),pointm, 1);
        }
    }
    
    if(abstract_rooms > 2)
    {
        house_construct(&room[roomcount++], 8, -4+(house[3]*20), -8, -4+(house[(abstract_rooms-1)*3]*20), 0);
    }
    building->roomcount = roomcount;
    return building;
}

static n_int draw_scene_not_done = 0;

void house_draw_scene(n_int dim_x, n_int dim_y)
{
    static n_byte2  seed[2] = {0xf728, 0xe231};
    static GLuint  terrain_display_list = 0;
    
    n_vect2 * location = boy_location();
    n_vect2 offset;
    n_vect2 center;
    center.x = 0 - (dim_x >> 1);
    center.y = 0 - (dim_y >> 1);
    vect2_add(&offset, &center, location);
    
    glClearColor(0, 0.2, 0, 0);

    glClear(GL_COLOR_BUFFER_BIT);

    if (draw_scene_not_done < 2)
    {
        n_int           px = 0;
        
        terrain_display_list = glGenLists(1);
        
        glNewList(terrain_display_list, GL_COMPILE);
        while (px < 8)
        {
            n_int py = 0;
            while (py < 8)
            {
                noble_building * building = house_create(seed);
                
                vect2_populate(&center, (px - 4) * 800, (py - 4) * 800);
                
                if (building)
                {
                    house_transform(building, &center, math_random(seed) & 255);
                    house_draw(building);
                    io_free((void **)&building);
                }
                py++;
            }
            px++;
        }
        
        px = 0;
        
        while (px < 5)
        {
            noble_road temp_road;
            temp_road.points[0].x = -3700;
            temp_road.points[0].y = ((px - 2) * 1600)-500;
            
            temp_road.points[1].x = -3700;
            temp_road.points[1].y = ((px - 2) * 1600)-300;
            
            temp_road.points[2].x = 2900;
            temp_road.points[2].y = ((px - 2) * 1600)-300;
            
            temp_road.points[3].x = 2900;
            temp_road.points[3].y = ((px - 2) * 1600)-500;
            
            house_draw_road(&temp_road);
            px++;
        }
        
        px = 0;
        
        while (px < 4)
        {
            noble_road temp_road;
            temp_road.points[0].y = -3700;
            temp_road.points[0].x = ((px - 2) * 3200)-500;
        
            temp_road.points[1].y = -3700;
            temp_road.points[1].x = ((px - 2) * 3200)-300;
            
            temp_road.points[2].y = 2900;
            temp_road.points[2].x = ((px - 2) * 3200)-300;
            
            temp_road.points[3].y = 2900;
            temp_road.points[3].x = ((px - 2) * 3200)-500;
            
            house_draw_road(&temp_road);
            
            px++;
        }
        
        px = 0;
        while (px < 4)
        {
            n_int py = 0;
            while (py < 4)
            {
                noble_fence temp_fence_x;
                noble_fence temp_fence_y;
                
                temp_fence_x.points[0].x = ((px - 2) * 1600) + 500;
                temp_fence_x.points[0].y = ((py - 2) * 1600) + 1100;
                temp_fence_x.points[1].x = ((px - 2) * 1600) + 500;
                temp_fence_x.points[1].y = ((py - 2) * 1600) - 100;
                
                temp_fence_y.points[0].x = ((px - 2) * 1600) + 1100;
                temp_fence_y.points[0].y = ((py - 2) * 1600) + 500;
                temp_fence_y.points[1].x = ((px - 2) * 1600) - 100;
                temp_fence_y.points[1].y = ((py - 2) * 1600) + 500;

                house_draw_fence(&temp_fence_x);
                house_draw_fence(&temp_fence_y);
                
                py++;
            }
            px++;
        }
        
        glEndList();
        draw_scene_not_done++;
    }
    else
    {
        n_vect2 * location_delta = boy_location_delta();
        glTranslatef(0-offset.x , 0-offset.y, 0);
        glRotatef(1.40625 * boy_turn_delta(), 0, 0, 1);
        glTranslatef(location_delta->x + offset.x, location_delta->y + offset.y, 0);
    }
    glCallList(terrain_display_list);
}