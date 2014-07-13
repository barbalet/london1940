/****************************************************************
 
 NobleMushroomView.m
 
 =============================================================
 
 Copyright 1996-2014 Tom Barbalet. All rights reserved.
 
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

#import "NobleMushroomView.h"

@implementation NobleMushroomView

- (void) awakeFromNib
{
    [self startEverything];
}

static n_int counter = 0;

- (void) drawRect:(NSRect)rect
{
    
    NSSize size = [[self window] frame].size;

    glViewport(0, 0, size.width, size.height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, size.width, 0, size.height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    if (counter == 120)
    {
        [[self openGLContext] makeCurrentContext];
        
        if (shared_draw(fIdentification) == 0L) return;
        
        counter = 0;
        
        [[self openGLContext] flushBuffer];
    }

    counter++;
    
}

@end