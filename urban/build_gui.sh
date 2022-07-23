#!/bin/bash
#	build.sh
#
#	=============================================================
#
#   Copyright 1996-2020 Tom Barbalet. All rights reserved.
#
#   Permission is hereby granted, free of charge, to any person
#   obtaining a copy of this software and associated documentation
#   files (the "Software"), to deal in the Software without
#   restriction, including without limitation the rights to use,
#   copy, modify, merge, publish, distribute, sublicense, and/or
#   sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following
#   conditions:
#
#   The above copyright notice and this permission notice shall be
#	included in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#   OTHER DEALINGS IN THE SOFTWARE.
#
#   This software is a continuing work of Tom Barbalet, begun on
#   13 June 1996. No apes or cats were harmed in the writing of
#   this software.

SOURCEDIR=.

if [ $# -ge 1 -a "$1" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2 
fi

if [ $# -ge 1 -a "$1" == "--coverage" ]
then
COMMANDLINEE="-ftest-coverage -fprofile-arcs"
else
COMMANDLINEE=-DCOMMAND_LINE_EXPLICIT
fi

gcc  ${CFLAGS} ${COMMANDLINEE} -c ./../../../apesdk/toolkit/*.c -lz -lm -lpthread -w
gcc  ${CFLAGS} ${COMMANDLINEE} -c ./../../../apesdk/script/*.c -lz -lm -lpthread -w
gcc  ${CFLAGS} ${COMMANDLINEE} -c ./../../../apesdk/render/*.c -lz -lm -lpthread -w
gcc  ${CFLAGS} ${COMMANDLINEE} -c ./../../../apesdk/sim/*.c -lz -lm -lpthread -w
gcc  ${CFLAGS} ${COMMANDLINEE} -c ./../../../apesdk/entity/*.c -lz -lm -lpthread -w
gcc  ${CFLAGS} ${COMMANDLINEE} -c ./../../../apesdk/universe/*.c -lz -lm -lpthread -w

gcc  ${CFLAGS} ${COMMANDLINEE} -c ./game/*.c -lz -lm -lpthread -w
gcc  ${CFLAGS} ${COMMANDLINEE} -c ./gui/*.c -lz -lm -lpthread -w

gcc ${CFLAGS} ${COMMANDLINEE} -c $SOURCEDIR/test_gui.c -o test_gui.o
if [ $? -ne 0 ]
then
exit 1
fi

gcc ${CFLAGS} ${COMMANDLINEE} -I/usr/include -o test_urban_gui *.o -lz -lm -lpthread
if [ $? -ne 0 ]
then
exit 1
fi

if [ $# -ge 1 -a "$1" == "--test" ]
then
./test_urban_gui
fi

if [ $# -ge 1 -a "$1" == "--coverage" ]
then
./test_urban_gui
gcov -n *.gcda
rm *.gc*
fi

rm test_urban_gui
rm *.o
