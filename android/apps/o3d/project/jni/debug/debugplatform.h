/*=======================================================================*/
/** @file   debugplatform.h

            All the platform/engine dependent functions
            required for debug stuff.

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

#ifndef GGS_ENGINE_DEBUG_DEBUGPLATFORM_H
#define GGS_ENGINE_DEBUG_DEBUGPLATFORM_H

namespace ggs {
namespace debug {

/*************************** c o n s t a n t s ***************************/


/******************************* t y p e s *******************************/


/**
 */
class Platform
{
public:
    /**
     * Get the CURRENT state of the button used to SELECT something
     *
     * @return this function should return the CURRENT state
     *      of the button you want the user to use for
     *      SELECTing something. Typicially X or O on a PS3
     *      or A on a XBox/360
     */
    static bool     GetInputButtonSelect();
    /**
     * Get the CURRENT state of the button
     * used to CANCEL something.
     *
     * @return this function should return the CURRENT state
     *         of the button you want the user to use for
     *         CANCELing something. Typicially Triangle on
     *         a PS3 or X on a XBox/360?
     */
    static bool     GetInputButtonCancel();
    /**
     * Get the CURRENT state of the button
     * used to ACCLERATE something.
     *
     * @return
     */
    static bool     GetInputButtonAccel();
    /**
     * Get the CURRENT state of the buttons
     * used to go UP.
     *
     * @return
     */
    static bool     GetInputButtonUp();
    /**
     * Get the CURRENT state of the buttons
     * used to go DOWN.
     *
     * @return
     */
    static bool     GetInputButtonDown();
    /**
     * Get the CURRENT state of the buttons
     * used to go LEFT.
     *
     * @return
     */
    static bool     GetInputButtonLeft();
    /**
     * Get the CURRENT state of the buttons
     * used to go RIGHT.
     *
     * @return
     */
    static bool     GetInputButtonRight();
    /**
     * Get the CURRENT state of the main Analog X axis
     *
     * @return -1.0f = full left,
     *       1.0f = full right
     *
     */
    static float    GetInputAnalogX();
    /**
     * Get the CURRENT state of the main Analog Y axis
     *
     *
     * @return -1.0 = full up,
     *       1.0 = full right
     *
     */
    static float    GetInputAnalogY();
    /**
     * Return the minimum value under which
     * we should assume the analog values
     * are zero.
     *
     * If your functions already account for a
     * dead zone then return 0.0f here
     *
     * @return the dead zone value
     */
    static float    GetAnalogDeadZone();
    /**
     * Get the number of seconds elapsed
     * since the last frame
     *
     * @return
     */
    static float    GetSecondsElapsed();
    /**
     * Compute the width of a string in pixels
     *
     * @param pStr   the string to measure
     *
     * @return The length of the given string in pixels
     */
    static int      GetTextWidth(const char* pStr);
    /**
     * Return the height of the font used for text in pixels
     *
     * @return The height of the the font used for text
     */
    static int      GetFontHeight();
    /**
     * Draw a rectangle in 2d on the screen
     *
     * @param x x position in pixels
     * @param y y position in pixels
     * @param width width in pixels
     * @param height height in pixels
     * @param colorRGBA color in the format 0xRRGGBBAA
     */
    static void     DrawRect(int x, int y, int width, int height, unsigned int colorRGBA);
    /**
     * Draw a string on the screen given 2d pixel coordinates
     *
     * @param x x position in pixels
     * @param y y position in pixels
     * @param colorRGBA color in the format 0xRRGGBBAA
     * @param pText text to draw
     */
    static void     DrawString(int x, int y, unsigned int colorRGBA, const char* pText);
    /**
     * Draw a string at a 3d position
     *
     * Given a 3d position, computes the
     * correspondiing 2d projected screen position
     * and draws the string there.
     *
     * @param x x position in world coords
     * @param y y position in world coords
     * @param z z position in world coords
     * @param colorRGBA color in 0xRRGGBBAA format
     * @param pText text to draw
     */
    static void     DrawString(float x, float y, float z, unsigned int colorRGBA, const char* pText);
    /**
     * Output a message to the debug terminal.
     *
     * On Windows the simplest solution is
     * to call OutputDebugString
     *
     * @param pStr string to be sent/printed.
     */
    static void     TerminalMessage(const char* pStr);
};

/************************** p r o t o t y p e s **************************/


/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/


/**************************** r o u t i n e s ****************************/

} // namespace debug
} // namespace ggs

#endif // GGS_ENGINE_DEBUG_DEBUGPLATFORM_H

