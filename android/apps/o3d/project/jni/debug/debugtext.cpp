/*=======================================================================*/
/** @file   debug\debugtext.cpp

            on screen text for debuggin

            for printing to a terminal or your debugger see
            debugprint.h

    @author Greggman

    --- LICENSE ----------- (New BSD) ---

    Copyright (c) 2007, Gregg Tavares (Greggman)

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    Neither the name of the Gregg Tavares nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************** i n c l u d e s ****************************/

#include "debugplatform.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "debugtext.h"

namespace ggs {
namespace debug {

/*************************** c o n s t a n t s ***************************/


/******************************* t y p e s *******************************/


/************************** p r o t o t y p e s **************************/


/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/


/**************************** r o u t i n e s ****************************/

void drawText (int x, int y, unsigned int colorRGBA, const char* pText)
{
	Platform::DrawString(x, y, colorRGBA, pText);
}

void drawTextV (int x, int y, unsigned int colorRGBA, const char* pFmt, va_list args)
{
    char buf[256];

    vsnprintf(buf, sizeof(buf), pFmt, args);
    drawText(x, y, colorRGBA, buf);
}

void drawTextF (int x, int y, unsigned int colorRGBA, const char* pFmt, ...)
{
    va_list ap; /* points to each unnamed arg in turn */
    va_start (ap, pFmt); /* make ap point to 1st unnamed arg */
    drawTextV (x, y, colorRGBA, pFmt, ap);
    va_end (ap);    /* clean up when done */
}

int getTextWidth(const char* pStr)
{
    return strlen(pStr) * 10; // platform dependent
}

int getTextHeight()
{
    return 12;  // height of 1 line of text, platform dependent
}

} // namespace debug
} // namespace ggs

