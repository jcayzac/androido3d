/*=======================================================================*/
/** @file   debugconsole.h

            runtime console system.

            lets you do a "printf" to the screen with a few options
            This is to allow you to quickly bring up info at
            runtime and put it in your face vs a the messages in
            a terminal or debugger window that are too easy to ignore

			note that it also echos to the terminal / debugger so that
			if you see something brought to your attention and it
			scrolls off the screen or whatever you can check the
			termninal / debugger for a history of messages.

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
#ifndef GGS_ENGINE_DEBUG_DEBUGCONSOLE_H
#define GGS_ENGINE_DEBUG_DEBUGCONSOLE_H
/**************************** i n c l u d e s ****************************/

#include <stdarg.h>

namespace ggs {
namespace debug {

/*************************** c o n s t a n t s ***************************/


/******************************* t y p e s *******************************/

class Console
{
public:
    /// <summary>printf but to the console</summary>
    /// 
    /// <param name="pFmt"></param>
    static void printf(const char* pFmt, ...);

	/// <summary>vprintf but to the console so you can write your own printf
    ///     covers</summary>
    /// 
    /// <param name="pFmt">printf format string</param>
    /// <param name="args">va_list. See stdarg.h</param>
    static void vprintf(const char* pFmt, va_list args);

	/// <summary>put a string to the console</summary>
	/// 
	/// <param name="pStr"></param>
	static void puts(const char* pStr);

    /// <summary>same as printf to the console but with flashing red/white text</summary>
    /// 
    /// <param name="pFmt"></param>
    static void errmsgf(const char* pFmt, ...);

    /// <summary>same as vprintf to the console but with flashing red/white text</summary>
    /// 
	/// <param name="pFmt">printf format string</param>
	/// <param name="args">va_list. See stdarg.h</param>
    static void verrmsgf(const char* pFmt, va_list args);

    /// <summary>printf in a certain color to the console</summary>
    /// 
    /// <param name="colorRGBA"></param>
    /// <param name="pFmt"></param>
    static void printf(unsigned int colorRGBA, const char* pFmt, ...);

	/// <summary>vprintf to the console in a certain color</summary>
    /// 
    /// <param name="colorRGBA"></param>
	/// <param name="pFmt">printf format string</param>
	/// <param name="args">va_list. See stdarg.h</param>
    static void vprintf(unsigned int colorRGBA, const char* pFmt, va_list args);

	/// <summary>put a string to the console in flashing colors</summary>
	/// 
	/// <param name="colorRGBA">first color in format 0xRRGGBBAA</param>
	/// <param name="pStr"></param>
	static void puts(unsigned int colorRGBA, const char* pStr);

    /// <summary>printf in flashing colors.</summary>
    /// 
    /// <param name="color1RGBA">first color in format 0xRRGGBBAA</param>
    /// <param name="color2RGBA">second color in format 0xRRGGBBAA</param>
    /// <param name="pFmt"></param>
    static void printf(unsigned int color1RGBA, unsigned int color2RGBA, const char* pFmt, ...);
    /// <summary>vprintf to console in flashing colors</summary>
    /// 
    /// <param name="color1RGBA">first color in format 0xRRGGBBAA</param>
    /// <param name="color2RGBA">second color in format 0xRRGGBBAA</param>
	/// <param name="pFmt">printf format string</param>
	/// <param name="args">va_list. See stdarg.h</param>
    static void vprintf(unsigned int color1RGBA, unsigned int color2RGBA, const char* pFmt, va_list args);

    /// <summary>put a string to the console in flashing colors</summary>
    /// 
	/// <param name="color1RGBA">first color in format 0xRRGGBBAA</param>
	/// <param name="color2RGBA">second color in format 0xRRGGBBAA</param>
    /// <param name="pStr"></param>
    static void puts(unsigned int color1RGBA, unsigned int color2RGBA, const char* pStr);

    /// <summary>Displays the console. Call once per frame. For most engines
    ///     you would call this in your render loop.
    /// </summary>
    static void Draw();

    /// <summary>Setup the debug console.
    /// 
    ///     depreciated. Use SetDrawPosition() and SetSize()</summary>
    /// 
    /// <param name="x">left position on screen</param>
    /// <param name="y">top position on screen</param>
    /// <param name="charsAcross">number of characters across a line</param>
    /// <param name="linesDown">number of lines of text to draw and keep a history of</param>
    static void Setup(int x, int y, int charsAcross, int linesDown);

    /// <summary>Set the draw position for the console</summary>
    /// 
	/// <param name="x">left position on screen</param>
	/// <param name="y">top position on screen</param>
	static void SetDrawPosition(int x, int y);

    /// <summary>Set the size of the console in terms of characters across (per line) and lines down</summary>
    /// 
	/// <param name="charsAcross">number of characters across a line</param>
	/// <param name="linesDown">number of lines of text to draw and keep a history of</param>
	static void SetSize(int charsAcross, int linesDown);

    /// <summary>Free / Delete all resources used by the console
    /// 
    ///     There is no need to call this but if you want to explicitly specify when the console
    ///     allocates and frees memory then call SetSize as your first call in the console. That
    ///     will make the console allocate it's memory. Later you can call Cleanup when you want
    ///     it to free it's memory.
    /// 
    ///     Be aware that any call before Setup or after Cleanup will make the console
    ///     allocate it's memory.</summary>
    static void Cleanup();

    /// <summary>Display all the lines in the console's buffers
    /// 
    ///     Normally the console will stop displaying any lines a few seconds after they are shown
    ///     This achieves the goal of bringing something to the user's attention while at the same
    ///     time getting out of their way and taking no CPU/GPU hit after a few seconds.
    /// 
    ///     Calling this with true will ignore that and just display all lines until this is
    ///     called with false.</summary>
    /// 
    /// <param name="bAll">true to display all lines
    ///     false to go back to normal behavior</param>
	static void ShowAll(bool bAll);
};

/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/


/************************** p r o t o t y p e s **************************/

} // namespace debug
} // namespace ggs

#endif /* GGS_ENGINE_DEBUG_DEBUGCONSOLE_H */




