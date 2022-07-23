/****************************************************************
 
 filehandling.c
 
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

static n_object * file_info = 0L;
static n_string   main_name = 0L;

void filehandling_set_object(n_object * value, n_string main_name_str)
{
    file_info = value;
    main_name = io_string_copy(main_name_str);
}

n_object * filehandling_object(void)
{
    return file_info;
}

void filehandling_main_name(n_string name_dot, n_string main_name)
{
    n_byte value = 0;
    n_int loop = 0;
    n_int end_here = 0;
    do{
        value = name_dot[loop];
        
        if ((value == '.') || (value == '('))
        {
            end_here = 1;
        }
        
        if (end_here == 0)
        {
            main_name[loop] = value;
        }
        loop++;

    }while ((end_here == 0) && (loop < STRING_BLOCK_SIZE) );
}

void filehandling_string_out(n_string_block new_string, n_string actual, n_string add_value)
{
    n_int length = io_length(actual, STRING_BLOCK_SIZE);
    n_int loop = 0, loop2 = 0;
    
    while (loop < length)
    {
        new_string[loop] = actual[loop];
        loop++;
    }
    if (add_value)
    {
        do{
            new_string[loop] = add_value[loop2];
            loop++;
            loop2++;
        }while (add_value[loop2]);
    }
    new_string[loop] = 0;
}

#define SIZE (21)

/* (220514_085751).json */
n_int filehandling_dt_string(n_string string)
{
    time_t curtime = time (NULL);
    struct tm *loctime = localtime (&curtime);
    size_t len = strftime(string, SIZE, "(%y%m%d_%H%M%S).json",loctime);

    if (len == 0 && string[0] != '\0')
    {
        return SHOW_ERROR("Date/Time stamp could not be created!");
    }
    return 0;
}


void * filehandling_gather( n_file * in_file, n_object_type * type_of )
{
    if ( in_file )
    {
        void *returned_blob = unknown_file_to_tree( in_file, type_of );
                
        io_file_free( &in_file );
        if ( returned_blob )
        {
            return returned_blob;
        }
        else
        {
            printf( "no returned object\n" );
        }
    }
    else
    {
        printf( "reading from disk failed\n" );
    }
    return 0L;
}

void filehandling_json_out(void * returned_blob, n_object_type type_of, n_string file_out)
{
    if ( returned_blob )
    {
        n_file * output_file = unknown_json( returned_blob, type_of );
        
        if ( output_file )
        {
            (void)io_disk_write( output_file, file_out );
            io_file_free( &output_file );
        }
        else
        {
            printf( "no returned output file\n" );
            unknown_free( &returned_blob, type_of );
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf( "no returned object\n" );
        exit(EXIT_FAILURE);
    }
}


n_file * filehandling_file( n_string file_in )
{
    n_file   *in_file = io_file_new();
    n_int    file_error = io_disk_read( in_file, file_in );
    
    if (file_error ==  -1)
    {
        io_file_free(&in_file);
        return 0L;
    }
    
    return in_file;
}

n_int filehandling_save(void)
{
    if (main_name == 0L)
    {
        return SHOW_ERROR("No file name ready");
    }
    
    if (file_info == 0L)
    {
        return SHOW_ERROR("No JSON information");
    }
    n_string_block file_name_out = {0};

    {
        n_string_block file_ending_out = {0};

        if (filehandling_dt_string(file_ending_out) == -1)
        {
            return -1;
        }
        
        filehandling_string_out(file_name_out, main_name, file_ending_out);
    }
    
    filehandling_json_out((void *)file_info, OBJECT_OBJECT, file_name_out);
    
    return 0;
}
