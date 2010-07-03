/*=======================================================================*/
/** @file   debugconsole.cpp

            runtime console system.

            lets you do a "printf" to the screen with a few options
            This is to allow you to quickly bring up info at
            runtime and put it in your face vs a the messages in
            a terminal or debugger window that are too easy to ignore

    @author Gregg A. Tavares

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

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "debugconsole.h"
#include "debugplatform.h"

namespace ggs {
namespace debug {

/*************************** c o n s t a n t s ***************************/

#define DEBUG_CONSOLE_COLOR_NORMAL  0xFFFFFFFF
#define DEBUG_CONSOLE_COLOR_ERROR   0xFF0000FF

#define DEBUG_CONSOLE_SHOW_TIME     5.0f   // seconds
#define DEBUG_CONSOLE_LINE_TIME     0.05f  // seconds

/******************************* t y p e s *******************************/

struct ConsoleLine
{
    unsigned int m_color1RGBA;
    unsigned int m_color2RGBA;
    char*        m_pBuf;
};

static ConsoleLine* s_pLines;
static char*        s_pBuffer;

static int      s_currentLineNdx;   // current line
static int      s_currentCharPos;   // current pos to write text in
static int      s_lastLineNdx;  	// last line that should be displayed
static float    s_showTimer;    	// count down until we stop displaying
static float    s_flashTimer;   	// flashing text timer
static float    s_lastTimer;    	// line timer
static bool     s_bShowAll;     	// draw everything currently in buffer
static int      s_charsAcross = 100;
static int      s_linesDown   = 40;
static int      s_oldCharsAcross;
static int      s_oldLinesDown;
static int      s_drawX = 20;
static int      s_drawY = 20;

/************************** p r o t o t y p e s **************************/


/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/


/**************************** r o u t i n e s ****************************/

void Console::ShowAll(bool bShowAll)
{
	s_bShowAll = bShowAll;
}

void Console::Cleanup()
{
    if (s_pBuffer)
    {
        delete [] s_pBuffer;
        s_pBuffer = NULL;
    }
    if (s_pLines)
    {
        delete [] s_pLines;
        s_pLines = NULL;
    }
}

static void SetupBuffer()
{
    if (!s_pLines || s_oldCharsAcross != s_charsAcross || s_oldLinesDown != s_linesDown)
    {
        Console::Cleanup();
    }

    s_pLines  = new ConsoleLine [s_linesDown];
    s_pBuffer = new char [s_linesDown * (s_charsAcross + 1)];

    for (int ii = 0; ii < s_linesDown; ++ii)
    {
        s_pLines[ii].m_pBuf = s_pBuffer + ii * (s_charsAcross + 1);
        s_pLines[ii].m_pBuf[0] = '\0';
    }

    s_oldCharsAcross = s_charsAcross;
    s_oldLinesDown   = s_linesDown;

    s_currentLineNdx = 0;
    s_currentCharPos = 0;
    s_lastLineNdx    = 0;
    s_showTimer      = 0.0f;
    s_flashTimer     = 0.0f;
    s_lastTimer      = 0.0f;
}

// this is here because it was created first so in the interest of not breaking
// the interface it's still here. I separated them into SetSize and SetDrawPosition
// because that way SetDrawPosition lets you move the console around in realtime
// which can be useful for adjusting it's position
void Console::Setup(int drawX, int drawY, int charsAcross, int linesDown)
{
	SetDrawPosition(drawX, drawY);
	SetSize(charsAcross, linesDown);
}

void Console::SetDrawPosition(int drawX, int drawY)
{
    s_drawX       = drawX;
    s_drawY       = drawY;
}

void Console::SetSize(int charsAcross, int linesDown)
{
    s_charsAcross = charsAcross;
    s_linesDown   = linesDown;

    SetupBuffer();
}

void Console::printf(const char* pFmt, ...)
{
    va_list ap; /* points to each unnamed arg in turn */
    va_start (ap, pFmt); /* make ap point to 1st unnamed arg */
    vprintf (DEBUG_CONSOLE_COLOR_NORMAL, DEBUG_CONSOLE_COLOR_NORMAL, pFmt, ap);
    va_end (ap);    /* clean up when done */
}

void Console::vprintf(const char* pFmt, va_list args)
{
    vprintf(DEBUG_CONSOLE_COLOR_NORMAL, DEBUG_CONSOLE_COLOR_NORMAL, pFmt, args);
}

void Console::errmsgf(const char* pFmt, ...)
{
    va_list ap; /* points to each unnamed arg in turn */
    va_start (ap, pFmt); /* make ap point to 1st unnamed arg */
    vprintf (DEBUG_CONSOLE_COLOR_ERROR, DEBUG_CONSOLE_COLOR_NORMAL, pFmt, ap);
    va_end (ap);    /* clean up when done */
}

void Console::verrmsgf(const char* pFmt, va_list args)
{
    vprintf(DEBUG_CONSOLE_COLOR_ERROR, DEBUG_CONSOLE_COLOR_NORMAL, pFmt, args);
}

void Console::printf(unsigned int colorRGBA, const char* pFmt, ...)
{
    va_list ap; /* points to each unnamed arg in turn */
    va_start (ap, pFmt); /* make ap point to 1st unnamed arg */
    vprintf (colorRGBA, colorRGBA, pFmt, ap);
    va_end (ap);    /* clean up when done */
}

void Console::vprintf(unsigned int colorRGBA, const char* pFmt, va_list args)
{
    vprintf(colorRGBA, colorRGBA, pFmt, args);
}

void Console::printf(unsigned int color1RGBA, unsigned int color2RGBA, const char* pFmt, ...)
{
    va_list ap; /* points to each unnamed arg in turn */
    va_start (ap, pFmt); /* make ap point to 1st unnamed arg */
    vprintf (color1RGBA, color2RGBA, pFmt, ap);
    va_end (ap);    /* clean up when done */
}

void Console::vprintf(unsigned int color1RGBA, unsigned int color2RGBA, const char* pFmt, va_list args)
{
    char buf[256];

    vsnprintf(buf, sizeof(buf), pFmt, args);

    puts(color1RGBA, color2RGBA, buf);
}

static void incLineNdx(int& ndx)
{
    ++ndx;
    if (ndx >= s_linesDown)
    {
        ndx = 0;
    }
}

static void decLineNdx(int& ndx)
{
    --ndx;
    if (ndx < 0)
    {
        ndx = s_linesDown - 1;
    }
}

void Console::puts(const char* pStr)
{
	puts (0xFFFFFFFF, 0xFFFFFFFF, pStr);
}

void Console::puts(unsigned int colorRGBA, const char* pStr)
{
	puts(colorRGBA, colorRGBA, pStr);
}

void Console::puts(unsigned int color1RGBA, unsigned int color2RGBA, const char* pStr)
{
	Platform::TerminalMessage(pStr);

    if (!s_pLines || s_oldCharsAcross != s_charsAcross || s_oldLinesDown != s_linesDown)
    {
        SetupBuffer();
    }

    while (*pStr)
    {
        ConsoleLine* pCL = &s_pLines[s_currentLineNdx];

        //if (s_currentCharPos == 0)
        // use the last color specifed because it's most likely
        // an error if a message of another color did not have an end-of-line
        // so, don't punish the correct message by not displaying it in
        // the color it requested.
        {
            pCL->m_color1RGBA = color1RGBA;
            pCL->m_color2RGBA = color2RGBA;
        }

        char* pDst = &pCL->m_pBuf[s_currentCharPos];
        char* pMax = &pCL->m_pBuf[s_charsAcross];
        char* pEnd = pDst;

        while (*pStr && *pStr != '\n')
        {
            char c = *pStr++;

            if (c >= 32) // check the most common case first
            {
                *pDst++ = c;

                if (pDst >= pMax)
                {
                    break;
                }
            }
            else if (c == '\t')
            {
                char* pStart = &pCL->m_pBuf[0];

                // insert at least 1 space
                *pDst++ = ' ';
                if (pDst >= pMax) { break; }

                while ((pDst - pStart) & 0x7)
                {
                    *pDst++ = ' ';
                    if (pDst >= pMax) { break; }
                }

                if (pDst >= pMax) { break; }
            }
            else if (c == '\r')
            {
				// this is not quite the same as a real \r
				// to make it the same we would need to NOT set the '\0' at the end of the line
				// and instead clear the line buffer to '\0' and then just let these routines
				// print into the buffer. I don't feel like making that change
				// and besides it's arguably more useful this way, you don't have to print a \n
				// after a bunch of \r just to make sure you don't get a line with extra chars

                // remember the furthest out we got
                if (pDst > pEnd)
                {
                    pEnd = pDst;
                }
                pDst = &pCL->m_pBuf[0];
            }
        }

        // if we got out past the current position end it ther
        if (pEnd > pDst)
        {
            *pEnd = '\0';
        }
        else // end at the current position
        {
            *pDst = '\0';
        }

        s_currentCharPos = pDst - &pCL->m_pBuf[0];

        bool bNextLine = false;

        if (*pStr == '\n')
        {
            bNextLine = true;
            ++pStr;
        }
        else if (s_currentCharPos >= s_charsAcross)
        {
            bNextLine = true;
        }

        if (bNextLine)
        {
            s_currentCharPos = 0;
            incLineNdx(s_currentLineNdx);
            s_pLines[s_currentLineNdx].m_pBuf[0] = '\0';

            // advance the last line if we've caught up
            if (s_currentLineNdx == s_lastLineNdx)
            {
                incLineNdx(s_lastLineNdx);
            }
        }
    }

    s_showTimer = DEBUG_CONSOLE_SHOW_TIME;
}

void Console::Draw()
{
    if (s_pLines && (s_showTimer > 0.0f || s_bShowAll || s_lastLineNdx != s_currentLineNdx))
    {
        int lastLineNdx = s_bShowAll ? s_currentLineNdx : s_lastLineNdx;
        int lineNdx = s_currentLineNdx;

        // skip the newest line if it's empty
//        if (lineNdx != lastLineNdx)
        {
            if (!s_pLines[lineNdx].m_pBuf[0])
            {
                decLineNdx(lineNdx);
            }
        }

        s_flashTimer += Platform::GetSecondsElapsed();
        s_flashTimer  = fmodf(s_flashTimer, 1.0f);

        s_showTimer -= Platform::GetSecondsElapsed();

        bool bFlash = s_flashTimer < 0.5f;
		int fontHeight = Platform::GetFontHeight();
		int ypos = s_drawY + s_linesDown * fontHeight; // FIX
        for(;;)
        {
            ConsoleLine* pCL = &s_pLines[lineNdx];

            Platform::DrawString(
                s_drawX,
                ypos,
                (bFlash ? pCL->m_color1RGBA : pCL->m_color2RGBA),
                pCL->m_pBuf);

            ypos -= fontHeight;
            if (lineNdx == lastLineNdx)
            {
                break;
            }
            decLineNdx(lineNdx);
        }
    }

    if (s_lastLineNdx != s_currentLineNdx)
    {
        if (s_showTimer <= 0.0f)
        {
            s_lastTimer -= Platform::GetSecondsElapsed();
            if (s_lastTimer <= 0.0f)
            {
                s_lastTimer = DEBUG_CONSOLE_LINE_TIME;
                incLineNdx(s_lastLineNdx);
            }
        }
    }
}

} // namespace debug
} // namespace ggs


