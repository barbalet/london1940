/****************************************************************  house.c  =============================================================  Copyright 1996-2014 Tom Barbalet. All rights reserved.  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  This software and Noble Ape are a continuing work of Tom Barbalet, begun on 13 June 1996. No apes or cats were harmed in the writing of this software.  ****************************************************************/#include "mushroom.h"static n_int 	house[64];static n_int 	rooms;static n_int	realrooms;static n_int    direction = 0;static n_byte2  seed[2] = { 0x4342, 0xed34};/* 1=north	2=south 4=east	8=west*/static void	house_drawroom(n_int edge_x, n_int edge_y, n_int topx, n_int topy, n_int botx, n_int boty, n_byte window){    n_int   loop = 0;    n_vect2 center = {0};    n_vect2 location[16] = {0};        /* set up values */    	realrooms++;	if (topx>botx){		n_int temp = botx;		botx = topx;		topx = temp;	}	if (topy>boty){		n_int temp = boty;		boty = topy;		topy = temp;	}        vect2_populate(&center, edge_x + 100, edge_y + 100);        vect2_populate(&location[0], topx, topy);    vect2_populate(&location[1], topx, boty);    vect2_populate(&location[2], botx, boty);    vect2_populate(&location[3], botx, topy);    vect2_populate(&location[4], topx+4, topy+4);    vect2_populate(&location[5], topx+4, boty-4);    vect2_populate(&location[6], botx-4, boty-4);    vect2_populate(&location[7], botx-4, topy+4);        if ((window&1) == 1)    {        vect2_populate(&location[8], topx+2, topy+10);        vect2_populate(&location[9], topx+2, boty-10);    }    if ((window&2) == 2)    {        vect2_populate(&location[10], botx-2, topy+10);        vect2_populate(&location[11], botx-2, boty-10);    }    if ((window&4) == 4)    {        vect2_populate(&location[12], topx+10, boty-2);        vect2_populate(&location[13], botx-10, boty-2);    }    if ((window&8) == 8)    {        vect2_populate(&location[14], topx+2, topy+10);        vect2_populate(&location[15], topx+2, boty-10);    }        /* perform transform */        while (loop < 16)    {        n_vect2 direction_vector;        vect2_direction(&direction_vector, direction, 1);                vect2_subtract(&location[loop], &location[loop], &center);        vect2_rotation(&location[loop], &direction_vector);                vect2_add(&location[loop], &location[loop], &center);        loop++;    }        /* draw transform */        draw_line(location[0].x, location[0].y, location[1].x, location[1].y);    draw_line(location[1].x, location[1].y, location[2].x, location[2].y);    draw_line(location[2].x, location[2].y, location[3].x, location[3].y);    draw_line(location[3].x, location[3].y, location[0].x, location[0].y);    draw_line(location[4].x, location[4].y, location[5].x, location[5].y);    draw_line(location[5].x, location[5].y, location[6].x, location[6].y);    draw_line(location[6].x, location[6].y, location[7].x, location[7].y);    draw_line(location[7].x, location[7].y, location[4].x, location[4].y);	if ((window&1) == 1)    {        draw_line(location[8].x, location[8].y, location[9].x, location[9].y);	}	if ((window&2) == 2)    {        draw_line(location[10].x, location[10].y, location[11].x, location[11].y);	}	if ((window&4) == 4)    {        draw_line(location[12].x, location[12].y, location[13].x, location[13].y);	}	if ((window&8) == 8)    {        draw_line(location[14].x, location[14].y, location[15].x, location[15].y);	}}static n_int house_plusminus(n_int num){	n_int	tmp=0;	while ((tmp > -3) && (tmp < 3))    {		tmp = (math_random(seed) % 11) - 5;	}	return(tmp);}static void house_sendout(n_int realrooms){	n_int	loop = 0;		while(loop < realrooms)    {		draw_line((loop * 15) + 20, 600, (loop * 15) + 30, 600);		draw_line((loop * 15) + 20, 601, (loop * 15) + 30, 601);		loop++;	}}void house_create(n_int dpx, n_int dpy){	n_int	loop=2;	n_int	pointp=-4, pointm=-4;	n_int	lp, lpend,count;	    direction = math_random(seed) & 255;    	realrooms=0;		house[0] = lpend = 5 + (math_random(seed) % 11); /* 5 to 15 */	lp = 0;    lp += (math_random(seed) % 3) + 2;	count = 1;	while (lp < lpend){		house[(count * 3) - 1] = 0;		house[count * 3] = lp;				house[(count * 3) + 1] = house_plusminus(6);		count++;        lp += (math_random(seed) % 3) + 2;	}	    house[0] ++;	rooms = count;    	if (house[4] > 1)    {        house_drawroom(dpx, dpy, dpx-8, dpy-4, 8+dpx+(house[4]*20), dpy-4+(house[3]*20), 10);//north + east        pointp = (house[3]*20)-4;    }    else    {        house_drawroom(dpx, dpy, dpx-8+(house[4]*20), dpy-4, dpx+8, dpy-4+(house[3]*20), 9);//north + west        pointm = (house[3]*20)-4;    }    while(loop < (rooms - 1))    {        if (house[(loop*3)+1]>1)        {            house_drawroom(dpx, dpy, dpx+8, dpy-4+(house[loop*3]*20), 8+dpx+(house[(loop*3)+1]*20), dpy+pointp, 2);//east            pointp = (house[loop*3]*20)-4;        }        else        {            house_drawroom(dpx, dpy, dpx-8+(house[(loop*3)+1]*20), dpy-4+(house[loop*3]*20), dpx-8, dpy+pointm, 1);//west            pointm = (house[loop*3]*20)-4;        }        loop++;    }			if (house[((rooms-1)*3)+1]>1)    {        house_drawroom(dpx, dpy, dpx-8, dpy+(house[0]*20)+4, 8+dpx+(house[((rooms-1)*3)+1]*20), dpy-4+(house[(rooms-1)*3]*20), 4);        house_drawroom(dpx, dpy, dpx-8, dpy+pointm, dpx-8-(house[((rooms-1)*3)+1]*20), dpy+(house[0]*20)+4, 1);        if (rooms != loop)        {            house_drawroom(dpx, dpy, dpx+8,dpy-4+(house[(rooms-1)*3]*20),8+dpx+(house[((rooms-1)*3)+1]*20),dpy+pointp, 2);        }    }    else    {        house_drawroom(dpx, dpy, dpx+8, dpy+(house[0]*20)+4,dpx+8+(house[((rooms-1)*3)+1]*20), dpy-4+(house[(rooms-1)*3]*20), 4);        house_drawroom(dpx, dpy, dpx+8, dpy+pointp,dpx+8-(house[((rooms-1)*3)+1]*20), dpy+(house[0]*20)+4, 2);        if (rooms != loop)        {            house_drawroom(dpx, dpy, dpx-8,dpy-4+(house[(rooms-1)*3]*20),dpx-8+(house[((rooms-1)*3)+1]*20),dpy+pointm, 1);        }    }    if(rooms > 2)    {        house_drawroom(dpx, dpy, dpx+8,dpy-4+(house[3]*20),dpx-8,dpy-4+(house[(rooms-1)*3]*20), 0);    }    house_sendout(realrooms);}