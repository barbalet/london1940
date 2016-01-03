/****************************************************************
 
 road.c
 
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

void road_draw_marking(noble_road * road)
{
    
    n_vect2 middle_line[2];
    n_vect2 partial_line;
    n_int   loop = 0;
    n_vect2 delta_stepper;
    
    vect2_center(&middle_line[0], &road->points[0], &road->points[1]);
    vect2_center(&middle_line[1], &road->points[2], &road->points[3]);
    
    vect2_divide(&partial_line, &middle_line[0], &middle_line[1], 200);
    vect2_copy(&delta_stepper, &middle_line[0]);
    
    glLineWidth(100);
    
    glColor3f(0.5, 0.5, 0.5);
    
    while (loop < 200)
    {
        if (loop & 1)
        {
            vect2_d(&delta_stepper, &partial_line, 1, 1);
        }
        else
        {
            glBegin(GL_LINES);
            house_vertex(&delta_stepper);
            vect2_d(&delta_stepper, &partial_line, 1, 1);
            house_vertex(&delta_stepper);
            glEnd();
        }
        loop++;
    }
    glLineWidth(1);
}

void road_draw(noble_road * road)
{
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
    house_vertex(&road->points[0]);
    house_vertex(&road->points[1]);
    house_vertex(&road->points[2]);
    house_vertex(&road->points[3]);
    glEnd();
}