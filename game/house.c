/****************************************************************
 
 house.c
 
 =============================================================
 
 Copyright 1996-2017 Tom Barbalet. All rights reserved.
 
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

static void house_transform(noble_building * building, n_vect2 * center, n_int direction)
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

static void house_drawroom(noble_room * room)
{
    gldraw_darkgrey();
    gldraw_quads(&room->points[4], 1);
    
    gldraw_grey();
    gldraw_quads(&room->points[0], 0);

    gldraw_lightgrey();
    
    if ((room->window&1) == 1)
    {
        gldraw_line(&room->points[8], &room->points[9]);
	}
    if ((room->window&2) == 2)
    {
        gldraw_line(&room->points[10], &room->points[11]);
	}
    if ((room->window&4) == 4)
    {
        gldraw_line(&room->points[12], &room->points[13]);
	}
    if ((room->window&8) == 8)
    {
        gldraw_line(&room->points[14], &room->points[15]);
	}
}

static void house_drawdoor(noble_room * room)
{
    gldraw_lightgrey();
    
    if ((room->door&1) == 1)
    {
        gldraw_quads(&room->points[16], 1);
    }
    if ((room->door&2) == 2)
    {
        gldraw_quads(&room->points[20], 1);
    }
    if ((room->door&4) == 4)
    {
        gldraw_quads(&room->points[24], 1);
    }
    if ((room->door&8) == 8)
    {
        gldraw_quads(&room->points[28], 1);
    }
}

static void house_draw_each(noble_building * building)
{
    n_int   loop_room = 0;
    while (loop_room < building->roomcount)
    {
        house_drawroom(&building->room[loop_room]);
        loop_room++;
    }
    loop_room = 0;
    while (loop_room < building->roomcount)
    {
        house_drawdoor(&building->room[loop_room]);
        loop_room++;
    }
    if (building->tree_mod & 1)
    {
        tree_draw(&building->trees[0]);
    }
    if (building->tree_mod & 2)
    {
        tree_draw(&building->trees[1]);
    }
    if (building->tree_mod & 4)
    {
        tree_draw(&building->trees[2]);
    }
    if (building->tree_mod & 8)
    {
        tree_draw(&building->trees[3]);
    }
}

/* 1=north, 2=south, 4=east, 8=west */
static void house_construct(noble_room * room, n_int topx, n_int topy, n_int botx, n_int boty, n_byte window, n_byte door)
{
    n_int midx, midy;
    room->window = window;
    room->door = door;
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
        vect2_populate(&room->points[14], topx+10, topy+3);
        vect2_populate(&room->points[15], botx-10, topy+3);
    }
    
    midx = (topx + botx) / 2;
    midy = (topy + boty) / 2;
    
    if ((room->door&1) == 1)
    {
        vect2_populate(&room->points[16], topx + 3, midy + 7);
        vect2_populate(&room->points[17], topx + 3, midy - 7);
        vect2_populate(&room->points[18], topx - 3, midy - 7);
        vect2_populate(&room->points[19], topx - 3, midy + 7);

    }
    if ((room->door&2) == 2)
    {
        vect2_populate(&room->points[20], botx - 3, midy + 7);
        vect2_populate(&room->points[21], botx - 3, midy - 7);
        vect2_populate(&room->points[22], botx + 3, midy - 7);
        vect2_populate(&room->points[23], botx + 3, midy + 7);
    }
    if ((room->door&4) == 4)
    {
        vect2_populate(&room->points[24], midx + 7, boty - 3);
        vect2_populate(&room->points[25], midx - 7, boty - 3);
        vect2_populate(&room->points[26], midx - 7, boty + 3);
        vect2_populate(&room->points[27], midx + 7, boty + 3);
    }
    if ((room->door&8) == 8)
    {
        vect2_populate(&room->points[28], midx + 7, topy + 3);
        vect2_populate(&room->points[29], midx - 7, topy + 3);
        vect2_populate(&room->points[30], midx - 7, topy - 3);
        vect2_populate(&room->points[31], midx + 7, topy - 3);
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

void house_create(noble_building * building, n_byte2 * seed, n_vect2 * center)
{
    n_int	         pointp = -4, pointm = -4;
    n_int	         loop = 2;
    n_int          * house;
    noble_room     * room;
    n_int            abstract_rooms;
    n_int            roomcount = 0;
    
    abstract_rooms = house_genetics(building->house, seed);
    house = building->house;
    room  = building->room;
    
	if (house[4] > 1)
    {
        house_construct(&room[roomcount++], -8, -4, 8+(house[4]*20), -4+(house[3]*20), 2/*10*/,1);//north + east
        pointp = (house[3] * 20) - 4;
    }
    else
    {
        house_construct(&room[roomcount++], -8 + (house[4]*20), -4, 8, -4+(house[3]*20), 1+8/*9*/, 2);//north + west
        pointm = (house[3] * 20) - 4;
    }
    
    while(loop < (abstract_rooms - 1))
    {
        if (house[(loop * 3) + 1] > 1)
        {
            house_construct(&room[roomcount++], 8, -4+(house[loop*3]*20), 8+(house[(loop*3)+1]*20), pointp, 2, 1);//east
            pointp = (house[loop * 3] * 20) - 4;
        }
        else
        {
            house_construct(&room[roomcount++], -8 + (house[(loop*3)+1]*20), -4+(house[loop*3]*20), -8, pointm, 1, 2);//west
            pointm = (house[loop * 3] * 20) - 4;
        }
        loop++;
    }
		
	if (house[((abstract_rooms - 1) * 3) + 1] > 1)
    {
        house_construct(&room[roomcount++], -8, (house[0]*20)+4, 8+(house[((abstract_rooms-1)*3)+1]*20), -4+(house[(abstract_rooms-1)*3]*20), 4, 8);
        house_construct(&room[roomcount++], -8, pointm, -8-(house[((abstract_rooms-1)*3)+1]*20), (house[0]*20)+4, 1, 2);
        if (abstract_rooms != loop)
        {
            house_construct(&room[roomcount++], 8, -4+(house[(abstract_rooms-1)*3]*20),8+(house[((abstract_rooms-1)*3)+1]*20), pointp, 2, 1);
        }
    }
    else
    {
        house_construct(&room[roomcount++], 8, (house[0]*20)+4, 8+(house[((abstract_rooms-1)*3)+1]*20), -4+(house[(abstract_rooms-1)*3]*20), 4, 8);
        house_construct(&room[roomcount++], 8, pointp, 8-(house[((abstract_rooms-1)*3)+1]*20), (house[0]*20)+4, 2, 1);
        if (abstract_rooms != loop)
        {
            house_construct(&room[roomcount++], -8, -4+(house[(abstract_rooms-1)*3]*20), -8+(house[((abstract_rooms-1)*3)+1]*20),pointm, 1, 2);
        }
    }
    
    if(abstract_rooms > 2)
    {
        house_construct(&room[roomcount++], 8, -4+(house[3]*20), -8, -4+(house[(abstract_rooms-1)*3]*20), 0, 12);
    }
    building->roomcount = roomcount;
    
    house_transform(building, center, math_random(seed) & 255);
}

void house_init(n_byte2 * seed, n_vect2 * location, noble_building * buildings)
{
    n_int count = 0;
    n_int px = 0;
    
    while (px < 4)
    {
        n_int py = 0;
        while (py < 4)
        {
            n_vect2 local_center;
            noble_building * building = &buildings[count++];
            vect2_populate(&local_center, (px * RESIDENCE_SPACE) + location->x, (py * RESIDENCE_SPACE) + location->y);
            house_create(building, seed, &local_center);
            building->tree_mod = math_random(seed) & 15;
            tree_init(building->trees, seed, &local_center);
            py++;
        }
        px++;
    }
}

void house_draw(noble_building * buildings)
{
    n_int count = 0;
    while (count < 16)
    {
        house_draw_each(&buildings[count++]);
    }
}
