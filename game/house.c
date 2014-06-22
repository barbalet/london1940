/****************************************************************  house.c  =============================================================  Copyright 1996-2014 Tom Barbalet. All rights reserved.  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  This software and Noble Ape are a continuing work of Tom Barbalet, begun on 13 June 1996. No apes or cats were harmed in the writing of this software.  ****************************************************************/#include <math.h>#include "mushroom.h"#define	NUM	20static n_int 	house[64];static n_int 	rooms;static n_int	realrooms;static n_int    maxrooms = 0;static n_int    locx[NUM], locy[NUM];static n_int	velx[NUM], vely[NUM];/*static unsigned char	scr[65536];static void house_soundmap(void){	short lpx = 0;	while (lpx < 256)    {		short lpy = 0;		while (lpy < 256)        {			short lpz = 0;			long relvol = 0;			while (lpz < NUM)            {				relvol += 131072 /(1 + ((locx[lpz]-lpx)*(locx[lpz]-lpx)) + ((locy[lpz]-lpy)*(locy[lpz]-lpy)) );				lpz++;			}			relvol/=NUM;			scr[lpx + (lpy<<8)] = 10 + (relvol>>10);			lpy++;		}		lpx++;	}} */void house_init(void){	n_int	lpx = 0;	while (lpx < NUM){		locx[lpx]=(tbrand()%64)+128;		locy[lpx]=(tbrand()%64)+128;		velx[lpx]=(tbrand()%16);		vely[lpx]=(tbrand()%16);		lpx++;	}}void house_move(void){	n_int	lpx = 0;	while(lpx < NUM)    {		locx[lpx]+=(velx[lpx])>>2;		locy[lpx]+=(vely[lpx])>>2;						velx[lpx]+=tbrand()%2;		vely[lpx]+=tbrand()%2;				if ((locx[lpx]<12)&&(velx[lpx]<0))        {            velx[lpx]=0-velx[lpx];        }		if ((locy[lpx]<12)&&(vely[lpx]<0))        {           vely[lpx]=0-vely[lpx];        }        		if ((locx[lpx]>244)&&(velx[lpx]>0))        {            velx[lpx]=0-velx[lpx];        }		if ((locy[lpx]>244)&&(vely[lpx]>0))        {            vely[lpx]=0-vely[lpx];        }        		draw_line(locx[lpx],locy[lpx], locx[lpx],locy[lpx]);        		lpx++;	}	}/*  1=north	2=south 4=east	8=west*/static void	house_drawroom(n_int topx,n_int topy,n_int botx,n_int boty,n_byte win){	realrooms++;		if (topx>botx){		n_int temp=botx;		botx=topx;		topx=temp;	}	if (topy>boty){		n_int temp=boty;		boty=topy;		topy=temp;	}	    draw_line(topx,topy, topx,boty);    draw_line(topx,boty, botx,boty);    draw_line(botx,boty, botx,topy);    draw_line(botx,topy, topx,topy);	    draw_line(topx+4,topy+4, topx+4,boty-4);    draw_line(topx+4,boty-4, botx-4,boty-4);    draw_line(botx-4,boty-4, botx-4,topy+4);    draw_line(botx-4,topy+4, topx+4,topy+4);	if ((win&1)==1)    {			draw_line(topx+2,topy+10, topx+2,boty-10);	}	if ((win&2)==2)    {			draw_line(botx-2,topy+10, botx-2,boty-10);	}	if ((win&4)==4)    {			draw_line(topx+10,boty-2, botx-10,boty-2);	}	if ((win&8)==8)    {			draw_line(topx+10,topy+2, botx-10,topy+2);	}}static n_int house_plusminus(n_int num){	n_int	tmp=0;	while ((tmp > -3) && (tmp < 3))    {		tmp=tbrand()%6;	}	return(tmp);}static void house_sendout(n_int realrooms){	n_int	loop = 0;		while(loop < realrooms)    {		draw_line((loop * 15) + 20, 600, (loop * 15) + 30, 600);		draw_line((loop * 15) + 20, 601, (loop * 15) + 30, 601);		loop++;	}}void house_create(n_int dpx, n_int dpy){	n_int	loop=2;	n_int	pointp=-4, pointm=-4;	n_int	lp, lpend,count;		realrooms=0;		house[0] = lpend = 10 + (tbrand()%6);	lp = 0;	lp += (tbrand()%2)+3;	count = 1;	while (lp<lpend){		house[(count * 3) - 1] = 0;		house[count * 3] = lp;				house[(count * 3) + 1] = house_plusminus(6);		count++;		lp+=(tbrand() % 2) + 3;	}	house[0] ++;	rooms = count;	if (house[4] > 1)    {        house_drawroom(dpx-8, dpy-4, 8+dpx+(house[4]*20), dpy-4+(house[3]*20), 10);//north + east                        pointp=(house[3]*20)-4;    }    else    {        house_drawroom(dpx-8+(house[4]*20), dpy-4, dpx+8, dpy-4+(house[3]*20), 9);//north + west        pointm=(house[3]*20)-4;    }    while(loop<(rooms-1))    {        if (house[(loop*3)+1]>1)        {            house_drawroom(dpx+8, dpy-4+(house[loop*3]*20), 8+dpx+(house[(loop*3)+1]*20), dpy+pointp, 2);//east            pointp=(house[loop*3]*20)-4;        }        else        {            house_drawroom(dpx-8+(house[(loop*3)+1]*20), dpy-4+(house[loop*3]*20), dpx-8, dpy+pointm, 1);//west            pointm=(house[loop*3]*20)-4;        }        loop++;    }			if (house[((rooms-1)*3)+1]>1)    {        house_drawroom(dpx-8,dpy+(house[0]*20)+4,8+dpx+(house[((rooms-1)*3)+1]*20),dpy-4+(house[(rooms-1)*3]*20),4);        house_drawroom(dpx-8, dpy+pointm,dpx-8-(house[((rooms-1)*3)+1]*20),dpy+(house[0]*20)+4,1);        if (rooms!=loop)        {            house_drawroom(dpx+8,dpy-4+(house[(rooms-1)*3]*20),8+dpx+(house[((rooms-1)*3)+1]*20),dpy+pointp,2);        }    }    else    {        house_drawroom(dpx+8,dpy+(house[0]*20)+4,dpx+8+(house[((rooms-1)*3)+1]*20),dpy-4+(house[(rooms-1)*3]*20),4);        house_drawroom(dpx+8, dpy+pointp,dpx+8-(house[((rooms-1)*3)+1]*20),dpy+(house[0]*20)+4,2);        if (rooms!=loop)        {            house_drawroom(dpx-8,dpy-4+(house[(rooms-1)*3]*20),dpx-8+(house[((rooms-1)*3)+1]*20),dpy+pointm,1);        }    }    if(rooms > 2)    {        house_drawroom(dpx+8,dpy-4+(house[3]*20),dpx-8,dpy-4+(house[(rooms-1)*3]*20),0);    }    house_sendout(realrooms);}void house_main(void){	enemy_init();/*	while((moveKey())!=0)*/    {        enemy_cube();        enemy_cycle();	}}