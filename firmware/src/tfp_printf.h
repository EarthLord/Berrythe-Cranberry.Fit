
/*
File: printf.h

MIT License

Copyright (C) 2004,2008  Kustaa Nyholm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

------------------------------------------------------------------------------------------------

This library is realy just two files: 'printf.h' and 'printf.c'.

They provide a simple and small (+100 loc) printf functionality to
be used in embedded systems.

I've found them so usefull in debugging that I do not bother with a
debugger at all.

They are distributed in source form, so to use them, just compile them
into your project.

The formats supported by this implementation are: 'd' 'u' 'c' 's' 'x' 'X'.

Zero padding and field width (limited to 255) are also supported.

The memory foot print of course depends on the target cpu, compiler and
compiler options, but a rough guestimate (based on a HC08 target) is about
600 - 1100 bytes for code and some twenty bytes of static data.  Note
that this printf is not re-entrant.

Note that the code expects that int size is 16 bits, and that char is
8 bits.

To use the printf you need to supply your own character output function,
something like :

void putchar (char c)
	{
	while (!SERIAL_PORT_EMPTY) ;
	SERIAL_PORT_TX_REGISTER = c;
	}


The printf function is actually a macro that translates to 'tfp_printf'.
This makes it possible to use it along with 'stdio.h' printf's in a single
source file. You just need to undef the names before you include the 'stdio.h'.
Note that these are not function-like macros, so if you have variables
or struct members with these names, things will explode in your face.
Without variadic macros this is the best we can do. If it is a  problem
just give up the macros and use the functions directly or rename them.

For further details see source code.

regs Kusti, 26.2.2008
*/


#ifndef __TFP_PRINTF__

#define __TFP_PRINTF__


#include <stdarg.h>


void tfp_printf(char *fmt, ...);

#define printf tfp_printf


#endif