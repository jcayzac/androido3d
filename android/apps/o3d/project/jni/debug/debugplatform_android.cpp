/*=======================================================================*/
/** @file   debugplatform.cpp

            All the platform/engine dependent functions
            required for debug stuff.


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

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <string.h>
#include "debugplatform.h"

#define  LOG_TAG    "gameconsole"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/*************************** c o n s t a n t s ***************************/


/******************************* t y p e s *******************************/


/************************** p r o t o t y p e s **************************/


/***************************** g l o b a l s *****************************/

//extern bool g_keyTable[256];
//extern void DrawText(int x, int y, unsigned int colorRGBA, const char* pText);
//extern int GetTextWidth(const char* pText);
//extern void DrawRect(int x, int y, int width, int height, unsigned int colorRGBA);
//extern float getSecondsElapsed();

/****************************** m a c r o s ******************************/


/**************************** r o u t i n e s ****************************/


namespace ggs {
namespace debug {

bool    Platform::GetInputButtonSelect()
{
    return false;
}

bool    Platform::GetInputButtonCancel()
{
    return false;
}

bool    Platform::GetInputButtonAccel()
{
    return false;
}

bool    Platform::GetInputButtonLeft()
{
    return false;
}

bool    Platform::GetInputButtonRight()
{
    return false;
}

bool    Platform::GetInputButtonUp()
{
    return false;
}

bool    Platform::GetInputButtonDown()
{
    return false;
}

float   Platform::GetInputAnalogX()
{
    return false;
}

float   Platform::GetInputAnalogY()
{
    return false;
}

float   Platform::GetAnalogDeadZone()
{
    return false;
}

float   Platform::GetSecondsElapsed()
{
    return 0.0f;  // getSecondsElapsed();
}

int     Platform::GetTextWidth(const char* pStr)
{
    return strlen(pStr) * 10; // simple assumption works in most cases or for a fixed sized font
}

int     Platform::GetFontHeight(void)
{
    return 12;
}

void        Platform::DrawRect(int x, int y, int width, int height, unsigned int colorRGBA)
{
    // ::DrawRect(x, y, width, height, colorRGBA);
}

void        Platform::DrawString(int x, int y, unsigned int colorRGBA, const char* pText)
{
    // ::DrawText(x, y, colorRGBA, pText);
}

void        Platform::TerminalMessage(const char* pStr)
{
    LOGI("%s\n", pStr);
}

} // namespace debug
} // namespace ggs




