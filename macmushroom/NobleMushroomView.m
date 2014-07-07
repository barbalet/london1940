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

#if 0

- (void) awakeFromNib
{
/*    
    fIdentification = NUM_VIEW;
    
    if ([[[self window] title] isEqualToString:@"Terrain"])
    {
        fIdentification = NUM_TERRAIN;
    }
    [self startEverything];
 */
}

- (void) drawRect:(NSRect)rect
{
    /*
    n_int         dimensionX = (n_int)rect.size.width;
    n_int         dimensionY = (n_int)rect.size.height;
    n_byte        * index = shared_draw(fIdentification);
    
    if (index == 0L) return;
    
    {
        shared_cycle_state returned_value = shared_cycle((n_uint)CFAbsoluteTimeGetCurrent (), fIdentification, dimensionX, dimensionY);
        if (returned_value == SHARED_CYCLE_DEBUG_OUTPUT)
        {
            [self debugOutput];
        }
        if (returned_value == SHARED_CYCLE_QUIT)
        {
            [self quitProcedure];
        }
    }
    [[self openGLContext] makeCurrentContext];
    
    {
        n_int           ly = 0;
        n_int           loop = 0;
        n_int			loopColors = 0;
        n_byte2         fit[256*3];
        
        shared_color(fit, fIdentification);
        
        while(loopColors < 256)
        {
            colorTable[loopColors][0] = fit[loop++] >> 8;
            colorTable[loopColors][1] = fit[loop++] >> 8;
            colorTable[loopColors][2] = fit[loop++] >> 8;
            loopColors++;
        }
        loop = 0;
        while(ly < dimensionY)
        {
            n_int    lx = 0;
            n_byte * indexLocalX = &index[(dimensionY-ly-1)*dimensionX];
            while(lx < dimensionX)
            {
                unsigned char value = indexLocalX[lx++] ;
                outputBuffer[loop++] = colorTable[value][0];
                outputBuffer[loop++] = colorTable[value][1];
                outputBuffer[loop++] = colorTable[value][2];
            }
            ly++;
        }
    }
    
    glDrawPixels((GLsizei)dimensionX, (GLsizei)dimensionY,GL_RGB,GL_UNSIGNED_BYTE, (const GLvoid *)outputBuffer);
    [[self openGLContext] flushBuffer];
    */
}

#endif

@end
