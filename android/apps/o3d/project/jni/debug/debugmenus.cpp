/*=======================================================================*/
/** @file   debugmenus.cpp

            Simple Debug Menu System

            Note: Design goals are to be simple TO USE. As such memory
            usage is not considered since these are only meant to be
            used during debugging

            There's also a lot of BS because they are stored in lists
            not arrays (vectors) so walking the list for certain things
            is not easy. Maybe that was a poor choice.

            Another huge issue, I wanted to share the Menu code
            as a generic list of items displayer. The problem is
            it needed iterators. Usually iterators are typed
            at compile time but I needed runtime polymorphic
            iterators. This is my first time implementing them
            and I had the restriction that I didn't want to allocate
            memory for them because they would end up allocating
            and deallocated with each menu every frame.

            but, since they are polymorphic I can't use actual
            types, I have to use pointers or references.

            That restricted many decisions and may have made
            the code more unnecessarily complicated.

            TODO:

			O add items that just display state
            O change makeliststay / makelistback to same thing with option
            O make metadata example
            O refactor so root menu doesn't call GetTextWidth

            O make state,enable,disable handler
            O make array template

            O handle mouse input
            O handle keyboard input (typing a number, etc...)
            O make scrolling work with accel
            O make repeat accel

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

#include "ggsassert.h"
#include "debugmenus.h"
#include "debugplatform.h"

namespace ggs {
namespace debug {

/*************************** c o n s t a n t s ***************************/

#define MENUSYSTEM_DEBUG_INPUT 0

static const int MENU_BORDER_SIZE   = 4;
static const int MENU_SPACING       = 1;

static const unsigned int MENU_BG_COLOR_RGBA               = 0x010101C0;
static const unsigned int MENU_ACTIVE_TEXT_COLOR_RGBA      = 0x808080FF;
static const unsigned int MENU_HIGHLIGHTED_TEXT_COLOR_RGBA = 0xFFFF00FF;
static const unsigned int MENU_NORMAL_TEXT_COLOR_RGBA      = 0xC0C0C0FF;
static const unsigned int MENU_SEPARATOR_COLOR_RGBA        = 0x808080FF;
static const unsigned int MENU_HIGHLIGHTED_BG_COLOR_RGBA   = 0x008080C0;
static const unsigned int MENU_ACTIVE_BG_COLOR_RGBA        = 0x404040C0;

/******************************* t y p e s *******************************/


/************************** p r o t o t y p e s **************************/


/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/


/**************************** r o u t i n e s ****************************/

/***********************************  ************************************/
/****************************** MenuContext ******************************/
/***********************************  ************************************/

MenuContext::MenuContext(int x, int y)
    : m_drawX(x)
    , m_drawY(y)
    , m_bHighlighted(false)
    , m_bActive(false)
{

}

unsigned int MenuContext::GetTextColor() const
{
    return isActive() ? MENU_ACTIVE_TEXT_COLOR_RGBA : (isHighlighted() ? MENU_HIGHLIGHTED_TEXT_COLOR_RGBA : MENU_NORMAL_TEXT_COLOR_RGBA);
}

void    MenuContext::DrawHighlightedJustifiedText(Justification j, const char* pStr)
{
    switch (j)
    {
    case JUSTIFY_LEFT:
        {
            Platform::DrawString(GetXPos(), GetYPos(), GetTextColor(), pStr);
        }
        break;
    case JUSTIFY_CENTER:
        {
            int textWidth = Platform::GetTextWidth(pStr);
            Platform::DrawString(GetXPos() + (GetAreaWidth() - textWidth) / 2, GetYPos(), GetTextColor(), pStr);
        }
        break;
    case JUSTIFY_RIGHT:
        {
            int textWidth = Platform::GetTextWidth(pStr);
            Platform::DrawString(GetXPos() + GetAreaWidth() - textWidth, GetYPos(), GetTextColor(), pStr);
        }
        break;
    case JUSTIFY_LEFT_LEFT:
        {
            Platform::DrawString(GetXPos(), GetYPos(), GetTextColor(), pStr);
        }
        break;
    case JUSTIFY_LEFT_CENTER:
        {
            int textWidth = Platform::GetTextWidth(pStr);
            Platform::DrawString(GetXPos() + (GetLeftAreaWidth() - textWidth) / 2, GetYPos(), GetTextColor(), pStr);
        }
        break;
    case JUSTIFY_LEFT_RIGHT:
        {
            int textWidth = Platform::GetTextWidth(pStr);
            Platform::DrawString(GetXPos() + GetLeftAreaWidth() - textWidth, GetYPos(), GetTextColor(), pStr);
        }
        break;
    case JUSTIFY_RIGHT_LEFT:
        {
            Platform::DrawString(GetRightXPos(), GetYPos(), GetTextColor(), pStr);
        }
        break;
    case JUSTIFY_RIGHT_CENTER:
        {
            int textWidth = Platform::GetTextWidth(pStr);
            Platform::DrawString(GetRightXPos() + (GetRightAreaWidth() - textWidth) / 2, GetYPos(), GetTextColor(), pStr);
        }
        break;
    case JUSTIFY_RIGHT_RIGHT:
        {
            int textWidth = Platform::GetTextWidth(pStr);
            Platform::DrawString(GetRightXPos() + GetRightAreaWidth() - textWidth, GetYPos(), GetTextColor(), pStr);
        }
        break;
    default:
        GGSassert(false);
    }
}

void    MenuContext::DrawHighlightedJustifiedTextV(Justification j, const char* pFmt, va_list args)
{
    char buf[256];

    _vsnprintf(buf, sizeof(buf), pFmt, args);
    DrawHighlightedJustifiedText(j, buf);

}

void    MenuContext::DrawHighlightedJustifiedTextF(Justification j, const char* pFmt, ...)
{
    va_list ap; /* points to each unnamed arg in turn */
    va_start (ap, pFmt); /* make ap point to 1st unnamed arg */
    DrawHighlightedJustifiedTextV (j, pFmt, ap);
    va_end (ap);    /* clean up when done */
}

void    MenuContext::DrawHighlightedText(const char* pStr)
{
    DrawHighlightedJustifiedText(JUSTIFY_LEFT, pStr);
}

void    MenuContext::DrawHighlightedTextV(const char* pFmt, va_list args)
{
    char buf[256];

    _vsnprintf(buf, sizeof(buf), pFmt, args);
    DrawHighlightedJustifiedText(JUSTIFY_LEFT, buf);
}

void    MenuContext::DrawHighlightedTextF(const char* pFmt, ...)
{
    va_list ap; /* points to each unnamed arg in turn */
    va_start (ap, pFmt); /* make ap point to 1st unnamed arg */
    DrawHighlightedJustifiedTextV (JUSTIFY_LEFT, pFmt, ap);
    va_end (ap);    /* clean up when done */
}

/***********************************  ************************************/
/****************************** MenuElement ******************************/
/***********************************  ************************************/

MenuElement::MenuElement()
    : m_bDelete(false)
    , m_bDeletableWhenLocked(false)
{
}

MenuElement::~MenuElement()
{
    if (!m_bDeletableWhenLocked)
    {
        GGSassert(!MenuSystem::isLocked());
    }
}

bool MenuElement::isDelete()
{
    return m_bDelete;
}

bool MenuElement::Delete()
{
    if (MenuSystem::isLocked())
    {
        MenuSystem::MarkSomethingWasDeleted();
        m_bDelete = true;
        return false;
    }
    else
    {
        delete this;
        return true;
    }
}

void MenuElement::SetDeletableWhenLocked()
{
    m_bDeletableWhenLocked = true;
}

/***********************************  ************************************/
/************************** MenuItemSimpleBase ***************************/
/***********************************  ************************************/

void MenuItemSimpleBase::Draw(MenuContext& ctx) const
{
    ctx.DrawHighlightedText(GetName());
}

int  MenuItemSimpleBase::GetLeftWidth() const
{
    return Platform::GetTextWidth(GetName());
}

int  MenuItemSimpleBase::GetRightWidth() const
{
	return 0;
}

int  MenuItemSimpleBase::GetHeight() const
{
	return Platform::GetFontHeight();
}

/***********************************  ************************************/
/**************************** MenuItemSimple *****************************/
/***********************************  ************************************/

void MenuItemSimple::Init()
{
}

MenuItemSimple::MenuItemSimple(const char* pName)
    : m_name(pName)
{
    Init();
}

MenuItemSimple::MenuItemSimple(const char* pName, std::string::size_type nameLen)
    : m_name(pName, nameLen)
{
    Init();
}

const char* MenuItemSimple::GetName() const
{
    return m_name.c_str();
}

/***********************************  ************************************/
/***************************** MenuListBase ******************************/
/***********************************  ************************************/

MenuListBase::MenuState::MenuState()
    : m_highlightedNdx(0)
    , m_topNdx(0)
    , m_bChildActive(false)
{
}

MenuListBase::MenuListBase(const char* pName)
    : MenuItemSimple(pName)
{
}

MenuListBase::MenuListBase(const char* pName, std::string::size_type nameLen)
    : MenuItemSimple(pName, nameLen)
{
}

void MenuListBase::BoundHighlight()
{
    // bound the highlight
    if (m_state.m_highlightedNdx < 0)
    {
        m_state.m_highlightedNdx = GetNumElements();
    }
    else if (m_state.m_highlightedNdx >= GetNumElements())
    {
        m_state.m_highlightedNdx = 0;
    }
}

void MenuListBase::DrawElements(MenuContext& ctx, bool bTooTall, bool bDraw, int& childPosX, int& childPosY)
{
    // draw each element
    int itemXPos = ctx.GetXPos();
    bool bLast = false;
    unsigned index = 0;
    MenuElemIter& it = GetIterator(ITER_MAIN);
    MenuElemIter& nextIt = GetIterator(ITER_EXTRA);
    it.SetToBegin();
    int elemHeight = it.GetElement()->GetHeight();
    while (!it.IsEnd() && !bLast)
    {
        MenuElement* pME = it.GetElement();
        bool oldHighlighted = ctx.isHighlighted();
        bool oldActive      = ctx.isActive();

        ctx.SetHighlighted(m_state.m_highlightedNdx == index);
        bool thisOneActive = (m_state.m_highlightedNdx == index && m_state.m_bChildActive);

        ctx.SetActive(thisOneActive);
        if (thisOneActive)
        {
            CopyIterator(ITER_HIGHLIGHT, ITER_MAIN);
            childPosX = ctx.GetXPos();
            childPosY = ctx.GetYPos();
        }

        if (bDraw && ctx.isHighlighted())
        {
            Platform::DrawRect(ctx.GetXPos(), ctx.GetYPos(), ctx.GetAreaWidth(), elemHeight,
                ctx.isActive() ? MENU_ACTIVE_BG_COLOR_RGBA : MENU_HIGHLIGHTED_BG_COLOR_RGBA);
        }

        // is the next element the last to draw
        int nextHeight = 0;
        CopyIterator(ITER_EXTRA, ITER_MAIN);
        nextIt.Next();
        if (!nextIt.IsEnd())
        {
            nextHeight = nextIt.GetElement()->GetHeight();
            bLast = (elemHeight + ctx.GetYPos() > MenuSystem::GetMenuAreaBottom());
        }

        // if we are too tall and the top element is not displayed draw an up arrow
        if (bTooTall && index != 0 && index == m_state.m_topNdx && index != m_state.m_highlightedNdx)
        {
            if (bDraw)
            {
                ctx.DrawHighlightedText("-^-^-^-");
                ctx.SetDrawPosition(itemXPos, ctx.GetYPos() + Platform::GetFontHeight() + MENU_SPACING);
            }
        }
        // if we are too tall and the bottom elementis not displayed draw and down arrow
        else if (bTooTall && bLast && index != m_state.m_highlightedNdx && index != GetNumElements() - 1)
        {
            if (bDraw)
            {
                ctx.DrawHighlightedText("-V-V-V-");
                ctx.SetDrawPosition(itemXPos, ctx.GetYPos() + Platform::GetFontHeight() + MENU_SPACING);
            }
        }
        // if we are at or past the topNdx
        else if (index >= m_state.m_topNdx)
        {
            if (bDraw)
            {
                ctx.SetIndex(index);
                pME->Draw(ctx);
            }
            ctx.SetDrawPosition(itemXPos, ctx.GetYPos() + elemHeight + MENU_SPACING);
        }

        ctx.SetHighlighted(oldHighlighted);
        ctx.SetActive(oldActive);

        elemHeight = nextHeight;

        it.Next();
        ++index;
    }
}

void MenuListBase::DrawList(MenuContext& ctx)
{
    MenuElemIter& activeIt = GetIterator(ITER_HIGHLIGHT);
    activeIt.SetToEnd();
    unsigned oldCtxIndex = ctx.GetIndex(); // save ctx index
    int originalXPos = ctx.GetXPos();
    int originalYPos = ctx.GetYPos();
    int childPosX    = 0;
    int childPosY    = 0;
    int menuWidth    = 0;
    int menuHeight   = 0;
    int leftWidth    = 0;
    int rightWidth   = 0;
    int itemMaxWidth = 0;
    bool bTooTall    = false;

    if (MenuSystem::IsShowMenus())
    {
        // figure out size of this menu
        int height = 0;
        {
            MenuElemIter& it = GetIterator(ITER_MAIN);
            for (it.SetToBegin(); !it.IsEnd(); it.Next())
            {
                const MenuElement* pME = it.GetElement();

                int subWidth = pME->GetLeftWidth();
                if (subWidth > leftWidth)
                {
                    leftWidth = subWidth;
                }
                subWidth = pME->GetRightWidth();
                if (subWidth > rightWidth)
                {
                    rightWidth = subWidth;
                }

                height += pME->GetHeight() + MENU_SPACING;
            }
        }

        itemMaxWidth = leftWidth + MenuContext::MENU_MIDDLE_SIZE + rightWidth;
        menuWidth    = itemMaxWidth + MENU_BORDER_SIZE * 2;
        menuHeight   = height + MENU_BORDER_SIZE * 2 - MENU_SPACING;

        // figure out where to start drawing
        int xpos = ctx.GetXPos();
        int ypos = ctx.GetYPos();

        // are we off the right side
        if (xpos + menuWidth > MenuSystem::GetMenuAreaRight())
        {
            // right justify
            xpos = MenuSystem::GetMenuAreaRight() - menuWidth;

            // are we off the left side
            if (xpos < MenuSystem::GetMenuAreaLeft())
            {
                // left justify
                xpos = MenuSystem::GetMenuAreaLeft();
            }
        }

        // are we off the bottom
        if (ypos + menuHeight > MenuSystem::GetMenuAreaBottom())
        {
            // bottom justify
            ypos = MenuSystem::GetMenuAreaBottom() - menuHeight;

            // are we off the top
            if (ypos < MenuSystem::GetMenuAreaTop())
            {
                // top justify
                ypos       = MenuSystem::GetMenuAreaTop();
                bTooTall   = true;
                menuHeight = MenuSystem::GetMenuAreaBottom() - MenuSystem::GetMenuAreaTop() + MENU_BORDER_SIZE * 2 - MENU_SPACING;
            }
        }

        // move our parent to the left
        int leftAdjust = xpos - ctx.GetXPos();
        originalXPos  += leftAdjust;
        originalYPos   = ypos;

        // save the position we want to draw
        ctx.SetDrawPosition(xpos, ypos);

        // figure out if top element needs to be adjusted
        if (bTooTall)
        {
            // this part sucks because it's really computation intensive

            // for now, let's assume 3 items will fit above
            unsigned topNdxTarget = m_state.m_highlightedNdx - 3;
            if (topNdxTarget > m_state.m_highlightedNdx) // because it's unsigned
            {
                topNdxTarget = 0;
            }

            // might we have to scroll up?
            if (m_state.m_topNdx > topNdxTarget)
            {
                m_state.m_topNdx = topNdxTarget;
            }
            // do we have to scroll down?
            else if (m_state.m_highlightedNdx > m_state.m_topNdx)
            {
                MenuElemIter& topIt = GetIterator(ITER_EXTRA);
                int menuAreaHeight = MenuSystem::GetMenuAreaBottom() - MenuSystem::GetMenuAreaTop();
                int height = 0;

                // for now assume 3 items will fit below
                unsigned targetBottomNdx = m_state.m_highlightedNdx + 3;

                if (targetBottomNdx >= GetNumElements())
                {
                    targetBottomNdx = GetNumElements() - 1;
                }

                unsigned index = 0;
                {
                    MenuElemIter& it = GetIterator(ITER_MAIN);
                    for (it.SetToBegin(); !it.IsEnd(); it.Next(), ++index)
                    {
                        if (m_state.m_topNdx == index)
                        {
                            CopyIterator(ITER_EXTRA, ITER_MAIN);
                        }

                        if (index >= m_state.m_topNdx)
                        {
                            height += it.GetElement()->GetHeight() + MENU_SPACING;
                        }

                        // we haven't gotten the highlight on screen yet so
                        while (height > menuAreaHeight && !topIt.IsEnd())
                        {
                            ++m_state.m_topNdx;
                            GGSassert(m_state.m_topNdx <= m_state.m_highlightedNdx);
                            height -= topIt.GetElement()->GetHeight() + MENU_SPACING;
                            topIt.Next();
                        }

                        if (index == targetBottomNdx)
                        {
                            break;
                        }
                    }
                }
            }
        }

        // is there a better way to do this? we need to know where we WANT to draw the child
        // which is just to the right of it's parent but we don't know where that is until
        // we try to draw the parent.
        //
        // at the same time we want to move to the left if there is no room for our child
        // so we need to let to child try to draw itself before we draw ourselves
        //
        // so, here we fake drawing ourselves to get the position we want the child
        DrawElements(ctx, bTooTall, false, childPosX, childPosY);
    }
    else if (m_state.m_bChildActive)
    {
        unsigned index = 0;
        {
            MenuElemIter& it = GetIterator(ITER_MAIN);
            for (it.SetToBegin(); !it.IsEnd(); it.Next(), ++index)
            {
                if (m_state.m_highlightedNdx == index)
                {
                    CopyIterator(ITER_HIGHLIGHT, ITER_MAIN);
                }
            }
        }
    }

    if (!activeIt.IsEnd())
    {
        MenuElement* pActiveME = activeIt.GetElement();
        bool oldHighlighted = ctx.isHighlighted();
        bool oldActive      = ctx.isActive();

        ctx.SetHighlighted(true);
        ctx.SetActive(true);

        int requestedXPos = childPosX + menuWidth;
        ctx.SetDrawPosition(requestedXPos, childPosY);
        ctx.SetIndex(m_state.m_highlightedNdx);

        m_state.m_bChildActive = pActiveME->Process(ctx);
        if (!m_state.m_bChildActive)
        {
            MenuSystem::ShowMenus(true);
        }

        // see if the child moved itself left and move us left to match
        int actualXPos = ctx.GetXPos();
        int adjust = actualXPos - requestedXPos;
        originalXPos += adjust;

        ctx.SetHighlighted(oldHighlighted);
        ctx.SetActive(oldActive);
    }

    ctx.SetDrawPosition(originalXPos, originalYPos);

    if (MenuSystem::IsShowMenus())
    {
        // draw the background
        Platform::DrawRect(ctx.GetXPos(), ctx.GetYPos(), menuWidth, menuHeight, MENU_BG_COLOR_RGBA);

        // set the size of the menu so centered or right justified stuff has a context
        ctx.SetAreaInfo(itemMaxWidth, menuHeight, leftWidth);

        // save space for the border
        ctx.SetDrawPosition(ctx.GetXPos() + MENU_BORDER_SIZE, ctx.GetYPos() + MENU_BORDER_SIZE - MENU_SPACING);

        DrawElements(ctx, bTooTall, true, childPosX, childPosY);
    }

    // restore ctx index
    ctx.SetIndex(oldCtxIndex);
    ctx.SetDrawPosition(originalXPos, originalYPos);
}

// given the current highlightNdx, if it's on a non selectable item go forward until
// we find a selectable item wrapping at the end
void MenuListBase::SkipForward()
{
    for (int ii = 0; ii < 2; ++ii)
    {
        unsigned index = 0;
        {
            MenuElemIter& it = GetIterator(ITER_MAIN);
            for (it.SetToBegin(); !it.IsEnd(); it.Next(), ++index)
            {
                if (index == m_state.m_highlightedNdx)
                {
                    if (!it.GetElement()->isSelectable())
                    {
                        ++m_state.m_highlightedNdx;
                    }
                }
            }
        }

        // wrap it
        if (m_state.m_highlightedNdx >= GetNumElements())
        {
            m_state.m_highlightedNdx = 0;
        }
    }
}

// given the current highlightNdx, if it's on a non selectable item go backard until
// we find a selectable item wrapping the beginning
// note: the reason we use a normal iterator instead of a reverse_iterator is because
// implementing a MenuEnumIter for a particular class would become a pain in the ass
// if we needed multiple kinds of iterators
void MenuListBase::SkipBackward()
{
    if (GetNumElements() > 0)
    {
        for (int ii = 0; ii < 2; ++ii)
        {
            unsigned index = GetNumElements();
            // note: so we don't need a reverse iter
            MenuElemIter& it = GetIterator(ITER_MAIN);
            it.SetToEnd();
            do
            {
                --index;
                it.Prev();

                if (index == m_state.m_highlightedNdx)
                {
                    if (!it.GetElement()->isSelectable())
                    {
                        --m_state.m_highlightedNdx;
                    }
                }
            }
            while(!it.IsBegin());

            // wrap it
            if (m_state.m_highlightedNdx >= GetNumElements())  // because it's unsigned
            {
                m_state.m_highlightedNdx = GetNumElements() - 1;
            }
        }
    }
}

void MenuListBase::SaveState(MenuState& state) const
{
    state = m_state;
}

void MenuListBase::RestoreState(const MenuState& state)
{
    m_state = state;
}

/***********************************  ************************************/
/***************************** MenuListStay ******************************/
/***********************************  ************************************/

MenuListStay::MenuListStay(const char* pName)
    : MenuListBase(pName)
{
}

MenuListStay::MenuListStay(const char* pName,  std::string::size_type nameLen)
    : MenuListBase(pName, nameLen)
{
}

bool MenuListStay::Process(MenuContext& ctx)
{
    // return if we have no children
    if (GetNumElements() == 0)
    {
        return false;
    }

    BoundHighlight();

    bool bChildWasActive = IsChildActive();
    DrawList(ctx);

    if (!bChildWasActive) // we called the child this frame don't do this
    {
        if (MenuSystem::GetButtonWasJustPressed(MenuSystem::BUTTON_CANCEL))
        {
            return false;
        }
        else if (MenuSystem::GetButtonWasJustPressed(MenuSystem::BUTTON_SELECT))
        {
            SetChildActive(true);
        }
        else
        {
            int dy = MenuSystem::GetDigitalAxisRepeat(MenuSystem::AXIS_Y);

            if (dy)
            {
                unsigned highlightedNdx = GetHighlightedIndex();

                if (dy < 0)
                {
                    if (highlightedNdx == 0 && dy < 0)
                    {
                        highlightedNdx = GetNumElements() - 1;
                    }
                    else
                    {
                        highlightedNdx += dy;
                    }
                    SetHighlightedIndex(highlightedNdx);
                    SkipBackward();
                }
                else // if (dy > 0)
                {
                    highlightedNdx += dy;
                    if (highlightedNdx >= GetNumElements())
                    {
                        highlightedNdx = 0;
                    }
                    SetHighlightedIndex(highlightedNdx);
                    SkipForward();
                }
            }
        }
    }

    return true;
}

/***********************************  ************************************/
/***************************** MenuListBack ******************************/
/***********************************  ************************************/

MenuListBack::MenuListBack(const char* pName)
    : MenuListBase(pName)
{
}

MenuListBack::MenuListBack(const char* pName,  std::string::size_type nameLen)
    : MenuListBase(pName, nameLen)
{
}

bool MenuListBack::Process(MenuContext& ctx)
{
    // return if we have no children
    if (GetNumElements() == 0)
    {
        return false;
    }

    BoundHighlight();

    bool bChildWasActive = IsChildActive();
    DrawList(ctx);

    if (!bChildWasActive) // we called the child this frame don't do this
    {
        if (MenuSystem::GetButtonWasJustPressed(MenuSystem::BUTTON_CANCEL))
        {
            return false;
        }
        else if (MenuSystem::GetButtonWasJustPressed(MenuSystem::BUTTON_SELECT))
        {
            SetChildActive(true);
        }
        else
        {
            int dy = MenuSystem::GetDigitalAxisRepeat(MenuSystem::AXIS_Y);

            if (dy)
            {
                unsigned highlightedNdx = GetHighlightedIndex();

                if (dy < 0)
                {
                    if (highlightedNdx == 0 && dy < 0)
                    {
                        highlightedNdx = GetNumElements() - 1;
                    }
                    else
                    {
                        highlightedNdx += dy;
                    }
                    SetHighlightedIndex(highlightedNdx);
                    SkipBackward();
                }
                else // if (dy > 0)
                {
                    highlightedNdx += dy;
                    if (highlightedNdx >= GetNumElements())
                    {
                        highlightedNdx = 0;
                    }
                    SetHighlightedIndex(highlightedNdx);
                    SkipForward();
                }

                SetHighlightedIndex(highlightedNdx);
            }
        }
    }
    else // it's was active
    {
        // if it's not still then go BACK
        if (!IsChildActive())
        {
            return false;
        }
    }

    return true;
}

/***********************************  ************************************/
/******************************* MenuBase ********************************/
/***********************************  ************************************/

MenuBase::MenuBase(const char* pName)
    : MenuListStay(pName)
{
}

MenuBase::MenuBase(const char* pName, std::string::size_type nameLen)
    : MenuListStay(pName, nameLen)
{
}

/***********************************  ************************************/
/***************************** MenuSeparator *****************************/
/***********************************  ************************************/

MenuSeparator::MenuSeparator(const char* pName)
    : m_name(pName)
{
}

const char * MenuSeparator::GetName() const
{
    return m_name.c_str();
}

int MenuSeparator::GetLeftWidth() const
{
    return 10; // doesn't really matter. We will draw as wide as the menu
}

int MenuSeparator::GetHeight() const
{
    return MENU_SPACING * 2 + 1;
}

void MenuSeparator::Draw(MenuContext& ctx) const
{
    Platform::DrawRect(ctx.GetXPos(), ctx.GetYPos() + MENU_SPACING, ctx.GetAreaWidth(), 1, MENU_SEPARATOR_COLOR_RGBA);
}

/***********************************  ************************************/
/*************************** MenuItemValueBase ***************************/
/***********************************  ************************************/

MenuItemValueBase::MenuItemValueBase(const char* pName)
    : MenuItemSimple(pName)
{
}

void MenuItemValueBase::Draw(MenuContext& ctx) const
{
    ctx.DrawHighlightedText(GetName());
    DrawValue(ctx);
}

/***********************************  ************************************/
/*************************** MenuItemBoolBase ****************************/
/***********************************  ************************************/

class MenuItemBoolBase : public MenuItemValueBase
{
    // yes, these should probably copy the contents but
    // the normal case would probably be static strings so
    // it doesn't seem worth it.
    const char* m_pOnStr;
    const char* m_pOffStr;

public:
    MenuItemBoolBase(const char* pText, const char* pOnStr = "on", const char* pOffStr = "off");

    virtual bool  GetValue() const = 0;
    virtual void  SetValue(bool value) = 0;

	virtual int   GetRightWidth() const;

    virtual bool Process(MenuContext& ctx);
    virtual void DrawValue(MenuContext& ctx) const;
};


MenuItemBoolBase::MenuItemBoolBase(const char* pText, const char* pOnStr, const char* pOffStr)
    : MenuItemValueBase(pText)
    , m_pOnStr(pOnStr)
    , m_pOffStr(pOffStr)
{
}

int MenuItemBoolBase::GetRightWidth() const
{
    int onWidth  = Platform::GetTextWidth(m_pOnStr);
    int offWidth = Platform::GetTextWidth(m_pOffStr);

    int maxValueWidth = onWidth > offWidth ? onWidth : offWidth;
    return maxValueWidth;
}

bool MenuItemBoolBase::Process(MenuContext&)
{
    SetValue(!GetValue());

    return false;   // we are done so pop back to the previous menu
}

void MenuItemBoolBase::DrawValue(MenuContext& ctx) const
{
    ctx.DrawHighlightedJustifiedText(MenuContext::JUSTIFY_RIGHT_LEFT, GetValue() ? m_pOnStr : m_pOffStr);
}

/***********************************  ************************************/
/***************************** MenuItemBool ******************************/
/***********************************  ************************************/

class MenuItemBool : public MenuItemBoolBase
{
    bool&       m_boolVar;

    virtual bool  GetValue() const { return m_boolVar; }
    virtual void  SetValue(bool value) { m_boolVar = value; }

public:
    MenuItemBool(const char* pText, bool& boolVar, const char* pOnStr = "on", const char* pOffStr = "off");
};


MenuItemBool::MenuItemBool(const char* pText, bool& boolVar, const char* pOnStr, const char* pOffStr)
    : MenuItemBoolBase(pText, pOnStr, pOffStr)
    , m_boolVar(boolVar)
{
}

/***********************************  ************************************/
/**************************** MenuItemBoolGS *****************************/
/***********************************  ************************************/

class MenuItemBoolGS : public MenuItemBoolBase
{
    GSHandler<bool>* m_pHndlr;

    virtual bool  GetValue() const { return m_pHndlr->GetValue(); }
    virtual void  SetValue(bool value) { m_pHndlr->SetValue(value); }
public:
    MenuItemBoolGS(const char* pText, GSHandler<bool>* pHndlr, const char* pOnStr = "on", const char* pOffStr = "off");
    ~MenuItemBoolGS();
};

MenuItemBoolGS::MenuItemBoolGS(const char* pText, GSHandler<bool>* pHndlr, const char* pOnStr, const char* pOffStr)
    : MenuItemBoolBase(pText, pOnStr, pOffStr)
    , m_pHndlr(pHndlr)
{
}

MenuItemBoolGS::~MenuItemBoolGS()
{
    delete m_pHndlr;
    m_pHndlr = NULL;
}

/***********************************  ************************************/
/********************** MenuItemBoolEnableDisable ************************/
/***********************************  ************************************/

class MenuItemBoolEnableDisable : public MenuItemBoolBase
{
    Menu::CStyleCallbackVoidNoData  m_enableFunc;
    Menu::CStyleCallbackVoidNoData  m_disableFunc;
    Menu::CStyleCallbackBoolNoData  m_statusFunc;

    virtual bool  GetValue() const;
    virtual void  SetValue(bool value);

public:
    MenuItemBoolEnableDisable(const char* pText, Menu::CStyleCallbackVoidNoData enableFunc, Menu::CStyleCallbackVoidNoData disableFunc, Menu::CStyleCallbackBoolNoData statusFunc, const char* pOnStr = "on", const char* pOffStr = "off");
};


MenuItemBoolEnableDisable::MenuItemBoolEnableDisable(const char* pText, Menu::CStyleCallbackVoidNoData enableFunc, Menu::CStyleCallbackVoidNoData disableFunc, Menu::CStyleCallbackBoolNoData statusFunc, const char* pOnStr, const char* pOffStr)
    : MenuItemBoolBase(pText, pOnStr, pOffStr)
    , m_enableFunc(enableFunc)
    , m_disableFunc(disableFunc)
    , m_statusFunc(statusFunc)
{
}

bool  MenuItemBoolEnableDisable::GetValue() const
{
    return m_statusFunc();
}

void  MenuItemBoolEnableDisable::SetValue(bool value)
{
    if (value)
    {
        m_enableFunc();
    }
    else
    {
        m_disableFunc();
    }
}

/***********************************  ************************************/
/************************** MenuItemBoolToggle ***************************/
/***********************************  ************************************/

class MenuItemBoolToggle : public MenuItemBoolBase
{
    Menu::CStyleCallbackVoidNoData  m_toggleFunc;
    Menu::CStyleCallbackBoolNoData  m_toggleFuncThatReturnsStatus;
    Menu::CStyleCallbackBoolNoData  m_statusFunc;

    virtual bool  GetValue() const;
    virtual void  SetValue(bool value);

public:
    MenuItemBoolToggle(const char* pText, Menu::CStyleCallbackVoidNoData toggleFunc, Menu::CStyleCallbackBoolNoData statusFunc, const char* pOnStr = "on", const char* pOffStr = "off");
    MenuItemBoolToggle(const char* pText, Menu::CStyleCallbackBoolNoData toggleFunc, Menu::CStyleCallbackBoolNoData statusFunc, const char* pOnStr = "on", const char* pOffStr = "off");
};


MenuItemBoolToggle::MenuItemBoolToggle(const char* pText, Menu::CStyleCallbackVoidNoData toggleFunc, Menu::CStyleCallbackBoolNoData statusFunc, const char* pOnStr, const char* pOffStr)
    : MenuItemBoolBase(pText, pOnStr, pOffStr)
    , m_toggleFunc(toggleFunc)
    , m_toggleFuncThatReturnsStatus(NULL)
    , m_statusFunc(statusFunc)
{
}

MenuItemBoolToggle::MenuItemBoolToggle(const char* pText, Menu::CStyleCallbackBoolNoData toggleThatReturnsStatusFunc, Menu::CStyleCallbackBoolNoData statusFunc, const char* pOnStr, const char* pOffStr)
    : MenuItemBoolBase(pText, pOnStr, pOffStr)
    , m_toggleFunc(NULL)
    , m_toggleFuncThatReturnsStatus(toggleThatReturnsStatusFunc)
    , m_statusFunc(statusFunc)
{
}

bool  MenuItemBoolToggle::GetValue() const
{
    return m_statusFunc();
}

void  MenuItemBoolToggle::SetValue(bool value)
{
    if (value != m_statusFunc())
    {
        if (m_toggleFunc)
        {
            m_toggleFunc();
        }
        else
        {
            m_toggleFuncThatReturnsStatus();
        }
    }
}

/***********************************  ************************************/
/************************* MenuItemBoolToggleGS **************************/
/***********************************  ************************************/

class MenuItemBoolToggleGS : public MenuItemBoolBase
{
    ToggleHandler*  m_pHndlr;

    virtual bool  GetValue() const;
    virtual void  SetValue(bool value);

public:
    MenuItemBoolToggleGS(const char* pText, ToggleHandler* pHndlr, const char* pOnStr = "on", const char* pOffStr = "off");
    ~MenuItemBoolToggleGS();
};

MenuItemBoolToggleGS::MenuItemBoolToggleGS(const char* pText, ToggleHandler* pHndlr, const char* pOnStr, const char* pOffStr)
    : MenuItemBoolBase(pText, pOnStr, pOffStr)
    , m_pHndlr(pHndlr)
{
}

MenuItemBoolToggleGS::~MenuItemBoolToggleGS()
{
    delete m_pHndlr;
    m_pHndlr = NULL;
}

bool  MenuItemBoolToggleGS::GetValue() const
{
    return m_pHndlr->GetStatus();
}

void  MenuItemBoolToggleGS::SetValue(bool value)
{
    if (value != GetValue())
    {
        m_pHndlr->Toggle();
    }
}

/***********************************  ************************************/
/************************** MenuItemStateSetter **************************/
/***********************************  ************************************/

class MenuItemStateSetter : public MenuItemBoolBase
{
	Menu::CStyleCallbackVoidBool	m_setStateFunc;
	bool	m_state;

	virtual bool  GetValue() const;
	virtual void  SetValue(bool value);

public:
	MenuItemStateSetter(const char* pText, Menu::CStyleCallbackVoidBool setStateFunc, bool bInitialState, const char* pOnStr = "on", const char* pOffStr = "off");
};

MenuItemStateSetter::MenuItemStateSetter(const char* pText, Menu::CStyleCallbackVoidBool setStateFunc, bool bInitialState, const char* pOnStr, const char* pOffStr)
	: MenuItemBoolBase(pText, pOnStr, pOffStr)
	, m_setStateFunc(setStateFunc)
	, m_state(bInitialState)
{
}

bool  MenuItemStateSetter::GetValue() const
{
	return m_state;
}

void  MenuItemStateSetter::SetValue(bool value)
{
	m_state = value;
	m_setStateFunc(value);
}

/***********************************  ************************************/
/*************************** MenuItemIntBase *****************************/
/***********************************  ************************************/

class MenuItemIntBase : public MenuItemValueBase
{
    int         m_minValue;
    int         m_maxValue;

public:
    MenuItemIntBase(const char* pText, int minValue = -32767, int maxValue = 32767);

    virtual int   GetValue() const = 0;
    virtual void  SetValue(int value) = 0;

	virtual int   GetRightWidth() const;

    virtual bool Process(MenuContext& ctx);
    virtual void DrawValue(MenuContext& ctx) const;
};

MenuItemIntBase::MenuItemIntBase(const char* pText, int minValue, int maxValue)
    : MenuItemValueBase(pText)
    , m_minValue(minValue)
    , m_maxValue(maxValue)
{
}

int MenuItemIntBase::GetRightWidth() const
{
	// yes, this is computed every time. so what? This is not critical code
	return Platform::GetTextWidth("12345678");
}

bool MenuItemIntBase::Process(MenuContext&)
{
    if (MenuSystem::GetButtonCurrentlyPressed(MenuSystem::BUTTON_SELECT))
    {
        int dx = MenuSystem::GetDigitalAxisRepeat(MenuSystem::AXIS_X);

        int value = GetValue();

        value += dx;
        if (value < m_minValue)
        {
            value = m_minValue;
        }
        else if (value > m_maxValue)
        {
            value = m_maxValue;
        }

        SetValue(value);

        // still editing so stay here
        return true;
    }
    else
    {
        // finished so pop back up
        return false;
    }
}

void MenuItemIntBase::DrawValue(MenuContext& ctx) const
{
    ctx.DrawHighlightedJustifiedTextF(MenuContext::JUSTIFY_RIGHT, "%d", GetValue());
}

/***********************************  ************************************/
/****************************** MenuItemInt ******************************/
/***********************************  ************************************/

class MenuItemInt : public MenuItemIntBase
{
    int&        m_intVar;

    virtual int   GetValue() const { return m_intVar; }
    virtual void  SetValue(int value) { m_intVar = value; }

public:
    MenuItemInt(const char* pText, int& intVar, int minValue = -32767, int maxValue = 32767);
};

MenuItemInt::MenuItemInt(const char* pText, int& intVar, int minValue, int maxValue)
    : MenuItemIntBase(pText, minValue, maxValue)
    , m_intVar(intVar)
{
}

/***********************************  ************************************/
/***************************** MenuItemIntGS *****************************/
/***********************************  ************************************/

class MenuItemIntGS : public MenuItemIntBase
{
    GSHandler<int>* m_pHndlr;

    virtual int   GetValue() const { return m_pHndlr->GetValue(); }
    virtual void  SetValue(int value) { m_pHndlr->SetValue(value); }
public:
    MenuItemIntGS(const char* pText, GSHandler<int>* pHndlr, int minValue = -32767, int maxValue = 32767);
    ~MenuItemIntGS();
};

MenuItemIntGS::MenuItemIntGS(const char* pText, GSHandler<int>* pHndlr, int minValue, int maxValue)
    : MenuItemIntBase(pText, minValue, maxValue)
    , m_pHndlr(pHndlr)
{
}

MenuItemIntGS::~MenuItemIntGS()
{
    delete m_pHndlr;
    m_pHndlr = NULL;
}

/***********************************  ************************************/
/************************** MenuItemFloatBase ****************************/
/***********************************  ************************************/

class MenuItemFloatBase : public MenuItemValueBase
{
    float         m_minValue;
    float         m_maxValue;
    float         m_incValue;
    float         m_accelMult;
    const char*   m_pFmt;
public:
    MenuItemFloatBase(const char* pText, float minValue = 0.0f, float maxValue = 1.0f, float incValue = 0.1f, float accelMult = 10.0f, const char* pFmt = "%7.3f");

    virtual float GetValue() const = 0;
    virtual void  SetValue(float value) = 0;

	virtual int   GetRightWidth() const;

    virtual bool Process(MenuContext& ctx);
    virtual void DrawValue(MenuContext& ctx) const;
};

MenuItemFloatBase::MenuItemFloatBase(const char* pText, float minValue, float maxValue, float incValue, float accelMult, const char* pFmt)
    : MenuItemValueBase(pText)
    , m_minValue(minValue)
    , m_maxValue(maxValue)
    , m_incValue(incValue)
    , m_accelMult(accelMult)
    , m_pFmt(pFmt)
{
}

int MenuItemFloatBase::GetRightWidth() const
{
	// just a guess
	unsigned numChars = (unsigned)atoi(m_pFmt + 1) + 1;

	static char ref[] = "01234567890123456789";

	// just assume we shouldn't go more than 20 chars
	if (numChars > sizeof(ref) - 1)
	{
		numChars = sizeof(ref) - 1;
	}

	int floatMaxWidth = Platform::GetTextWidth(ref + sizeof(ref) - 1 - numChars);

	return floatMaxWidth;
}

bool MenuItemFloatBase::Process(MenuContext&)
{
    if (MenuSystem::GetButtonCurrentlyPressed(MenuSystem::BUTTON_SELECT))
    {
        float adjust = 0.0f;

        {
            int digitalAdjust = MenuSystem::GetDigitalAxisRepeat(MenuSystem::AXIS_X);
            if (digitalAdjust != 0)
            {
                adjust = (float)digitalAdjust;
            }
            else
            {
                adjust = MenuSystem::GetAnalogAxis(MenuSystem::AXIS_X);
            }
        }

        float value = GetValue();
        value += adjust * m_incValue * (MenuSystem::GetButtonCurrentlyPressed(MenuSystem::BUTTON_ACCEL) ? m_accelMult : 1.0f);
        if (value < m_minValue)
        {
            value = m_minValue;
        }
        else if (value > m_maxValue)
        {
            value = m_maxValue;
        }

        SetValue(value);

        // still editing so stay here
        return true;
    }
    else
    {
        // finished so pop back up
        return false;
    }
}

void MenuItemFloatBase::DrawValue(MenuContext& ctx) const
{
    ctx.DrawHighlightedJustifiedTextF(MenuContext::JUSTIFY_RIGHT, m_pFmt, GetValue());
}

/***********************************  ************************************/
/***************************** MenuItemFloat *****************************/
/***********************************  ************************************/

class MenuItemFloat : public MenuItemFloatBase
{
    float&        m_floatVar;

    virtual float GetValue() const { return m_floatVar; }
    virtual void  SetValue(float value) { m_floatVar = value; }

public:
    MenuItemFloat(const char* pText, float& floatVar, float minValue = 0.0f, float maxValue = 1.0f, float incValue = 0.1f, float accelMult = 10.0f, const char* pFmt = "%7.3f");
};


MenuItemFloat::MenuItemFloat(const char* pText, float& floatVar, float minValue, float maxValue, float incValue, float accelMult, const char* pFmt)
    : MenuItemFloatBase(pText, minValue, maxValue, incValue, accelMult, pFmt)
    , m_floatVar(floatVar)
{
}

/***********************************  ************************************/
/**************************** MenuItemFloatGS ****************************/
/***********************************  ************************************/

class MenuItemFloatGS : public MenuItemFloatBase
{
    GSHandler<float>* m_pHndlr;

    virtual float GetValue() const { return m_pHndlr->GetValue(); }
    virtual void  SetValue(float value) { m_pHndlr->SetValue(value); }
public:
    MenuItemFloatGS(const char* pText, GSHandler<float>* pHndlr, float minValue = 0.0f, float maxValue = 1.0f, float incValue = 0.1f, float accelMult = 10.0f, const char* pFmt = "%7.3f");
    ~MenuItemFloatGS();
};

MenuItemFloatGS::MenuItemFloatGS(const char* pText, GSHandler<float>* pHndlr, float minValue, float maxValue, float incValue, float accelMult, const char* pFmt)
    : MenuItemFloatBase(pText, minValue, maxValue, incValue, accelMult, pFmt)
    , m_pHndlr(pHndlr)
{
}

MenuItemFloatGS::~MenuItemFloatGS()
{
    delete m_pHndlr;
    m_pHndlr = NULL;
}

/***********************************  ************************************/
/********************** MenuItemCStyleBoolCallback ***********************/
/***********************************  ************************************/

class MenuItemCStyleCallbackBoolData : public MenuItemSimple
{
    Menu::CStyleCallbackBoolData    m_callbackFunc;
    void*                           m_pUserData;

public:
    MenuItemCStyleCallbackBoolData(const char* pText, Menu::CStyleCallbackBoolData callbackFunc, void* pUserData = NULL);

    virtual bool Process(MenuContext& ctx);
};

MenuItemCStyleCallbackBoolData::MenuItemCStyleCallbackBoolData(const char* pText, Menu::CStyleCallbackBoolData callbackFunc, void* pUserData)
    : MenuItemSimple(pText)
    , m_callbackFunc(callbackFunc)
    , m_pUserData(pUserData)
{
}

bool MenuItemCStyleCallbackBoolData::Process(MenuContext&)
{
    return m_callbackFunc(m_pUserData);
}

/***********************************  ************************************/
/********************** MenuItemCStyleBoolCallback ***********************/
/***********************************  ************************************/

class MenuItemCStyleCallbackVoidData : public MenuItemSimple
{
    Menu::CStyleCallbackVoidData    m_callbackFunc;
    void*                           m_pUserData;

public:
    MenuItemCStyleCallbackVoidData(const char* pText, Menu::CStyleCallbackVoidData callbackFunc, void* pUserData = NULL);

    virtual bool Process(MenuContext& ctx);
};

MenuItemCStyleCallbackVoidData::MenuItemCStyleCallbackVoidData(const char* pText, Menu::CStyleCallbackVoidData callbackFunc, void* pUserData)
    : MenuItemSimple(pText)
    , m_callbackFunc(callbackFunc)
    , m_pUserData(pUserData)
{
}

bool MenuItemCStyleCallbackVoidData::Process(MenuContext&)
{
    m_callbackFunc(m_pUserData);
    return false;
}

/***********************************  ************************************/
/******************* MenuItemCStyleCallbackVoidNoData ********************/
/***********************************  ************************************/

class MenuItemCStyleCallbackVoidNoData : public MenuItemSimple
{
    Menu::CStyleCallbackVoidNoData  m_callbackFunc;

public:
    MenuItemCStyleCallbackVoidNoData(const char* pText, Menu::CStyleCallbackVoidNoData callbackFunc);

    virtual bool Process(MenuContext& ctx);
};

MenuItemCStyleCallbackVoidNoData::MenuItemCStyleCallbackVoidNoData(const char* pText, Menu::CStyleCallbackVoidNoData callbackFunc)
    : MenuItemSimple(pText)
    , m_callbackFunc(callbackFunc)
{
}

bool MenuItemCStyleCallbackVoidNoData::Process(MenuContext&)
{
    m_callbackFunc();
    return false;
}

/***********************************  ************************************/
/********************* MenuItemCPPStyleVoidCallback **********************/
/***********************************  ************************************/

class MenuItemCPPStyleVoidCallback : public MenuItemSimple
{
    // this functor will be deleted automatically
    Menu::CPPStyleVoidCallback* m_pCallbackFunctor;
    bool                        m_bAutoDelete;

public:
    MenuItemCPPStyleVoidCallback(const char* pText, Menu::CPPStyleVoidCallback* pCallbackFunctor, bool bAutoDelete = true);
    ~MenuItemCPPStyleVoidCallback();

    virtual bool Process(MenuContext& ctx);
};

MenuItemCPPStyleVoidCallback::MenuItemCPPStyleVoidCallback(const char* pText, Menu::CPPStyleVoidCallback* pCallbackFunctor, bool bAutoDelete)
    : MenuItemSimple(pText)
    , m_pCallbackFunctor(pCallbackFunctor)
    , m_bAutoDelete(bAutoDelete)
{
}

MenuItemCPPStyleVoidCallback::~MenuItemCPPStyleVoidCallback()
{
    if (m_bAutoDelete)
    {
        delete m_pCallbackFunctor;
        m_pCallbackFunctor = NULL;
    }
}

bool MenuItemCPPStyleVoidCallback::Process(MenuContext&)
{
    (*m_pCallbackFunctor)();
    return false;
}

/***********************************  ************************************/
/********************* MenuItemCPPStyleBoolCallback **********************/
/***********************************  ************************************/

class MenuItemCPPStyleBoolCallback : public MenuItemSimple
{
    // this functor will be deleted automatically
    Menu::CPPStyleBoolCallback* m_pCallbackFunctor;
    bool                        m_bAutoDelete;

public:
    MenuItemCPPStyleBoolCallback(const char* pText, Menu::CPPStyleBoolCallback* pCallbackFunctor, bool bAutoDelete = true);
    ~MenuItemCPPStyleBoolCallback();

    virtual bool Process(MenuContext& ctx);
};

MenuItemCPPStyleBoolCallback::MenuItemCPPStyleBoolCallback(const char* pText, Menu::CPPStyleBoolCallback* pCallbackFunctor, bool bAutoDelete)
    : MenuItemSimple(pText)
    , m_pCallbackFunctor(pCallbackFunctor)
    , m_bAutoDelete(bAutoDelete)
{
}

MenuItemCPPStyleBoolCallback::~MenuItemCPPStyleBoolCallback()
{
    if (m_bAutoDelete)
    {
        delete m_pCallbackFunctor;
        m_pCallbackFunctor = NULL;
    }
}

bool MenuItemCPPStyleBoolCallback::Process(MenuContext&)
{
    return (*m_pCallbackFunctor)();
}


/***********************************  ************************************/
/********************************* Menu **********************************/
/***********************************  ************************************/

Menu::Menu(const char* pName)
    : MenuBase(pName)
{
    Init();
}

Menu::Menu(const char* pName, std::string::size_type nameLen)
    : MenuBase(pName, nameLen)
{
    Init();
}

void Menu::Init()
{
    for (int ii = 0; ii < NUM_ITERS; ++ii)
    {
        m_iters[ii].Init(&m_menuElements);
    }
}

Menu::~Menu()
{
    GGSassert (!MenuSystem::isLocked());

	while (!m_menuElements.empty())
	{
		delete m_menuElements.back();
		m_menuElements.pop_back();
	}
}

void Menu::Clear()
{
    for (ElementList::iterator it = m_menuElements.begin(); it != m_menuElements.end();)
    {
        if ((*it)->Delete())
		{
			it = m_menuElements.erase(it);
		}
		else
		{
			++it;
		}
    }
}

const char* Menu::GetNameFromPath(const char* pMenuPath)
{
    const char* pLastSep = strrchr(pMenuPath, MENU_SEPARATOR);

    return pLastSep ? (pLastSep + 1) : pMenuPath;
}

Menu::ElementList::iterator Menu::FindMenuElementHelper(const char* pMenuPath, const char* pEnd, const char*& pSep)
{
    pSep = strchr(pMenuPath, MENU_SEPARATOR);
    if (!pSep)
    {
        pSep = pMenuPath + strlen(pMenuPath);
    }

    size_t len = pSep - pMenuPath;

    ElementList::iterator it = m_menuElements.begin();
    for (; it != m_menuElements.end(); ++it)
    {
        if (!(*it)->isDelete() && !_strnicmp(pMenuPath, (*it)->GetName(), len) && len == strlen((*it)->GetName()))
        {
            break;
        }
    }
    return it;
}

MenuElement* Menu::FindMenuElementOrAddMenu(const char* pMenuPath, const char* pEnd, bool bAdd)
{
    if (!pEnd)
    {
        return this;
    }

    const char* pSep;
    ElementList::iterator it = FindMenuElementHelper(pMenuPath, pEnd, pSep);
    MenuElement* pME = NULL;
    if (it != m_menuElements.end())
    {
        pME = *it;
    }

    // if we didn't find one make a new one
    if (!pME)
    {
        if (bAdd)
        {
            pME = new Menu(pMenuPath, pSep - pMenuPath);
            AddElement(pME);
        }
        else
        {
            return NULL;
        }
    }

    // we have a MenuElement. Is it a menu?
    // yes, is this the last one?
    if (pSep == pEnd)
    {
        // yes, so return it
        return pME;
    }
    else if (pME->isMenu())
    {
        // no so search deeper
        return static_cast<Menu*>(pME)->FindOrAddMenu(pSep + 1, pEnd);
    }

    GGSassert(!bAdd); // if we are adding then the current node is not a menu

    // it's not a menu or we didn't find the element we wanted
    return NULL;
}

Menu* Menu::FindOrAddMenu(const char* pMenuPath, const char* pEnd)
{
    MenuElement* pME = FindMenuElementOrAddMenu(pMenuPath, pEnd, true);
    if (pME && pME->isMenu())
    {
        return static_cast<Menu*>(pME);
    }
    return NULL;
}

void Menu::AddElementToMenuWithItemPath(const char* pItemPath, MenuElement* pElement)
{
    GGSassert(pElement);

    const char* pLastSep = strrchr(pItemPath, MENU_SEPARATOR);
    Menu* pMenu = FindOrAddMenu(pItemPath, pLastSep);
    if (pMenu)
    {
        pMenu->AddElement(pElement);
    }
}

Menu* Menu::FindOrAddMenu(const char* pMenuPath)
{
    return FindOrAddMenu(pMenuPath, pMenuPath + strlen(pMenuPath));
}

void Menu::AddElement(MenuElement* pElement)
{
    GGSassert(pElement);
    m_menuElements.push_back(pElement);
}

void Menu::AddElementToMenu(const char* pMenuPath, MenuElement* pElement)
{
    GGSassert(pElement);

    Menu* pMenu = FindOrAddMenu(pMenuPath);
    if (pMenu)
    {
        pMenu->AddElement(pElement);
    }
}

MenuElement* Menu::FindMenuElement(const char* pMenuPath)
{
    return FindMenuElementOrAddMenu(pMenuPath, pMenuPath + strlen(pMenuPath), false);
}

void Menu::DeleteMenuElement(const char* pMenuPath)
{
    const char* pSep;
    ElementList::iterator it = FindMenuElementHelper(pMenuPath, pMenuPath + strlen(pMenuPath), pSep);
    // we didn't find anything
    if (it == m_menuElements.end())
    {
        MenuSystem::ErrMessage("%s|%s not found for deletion", GetName(), pMenuPath);
    }
    else
    {
        MenuElement* pME = *it;

        if (*pSep) // there's more to go
        {
            if (pME->isMenu())
            {
                static_cast<Menu*>(pME)->DeleteMenuElement(pSep + 1);
            }
            else
            {
                MenuSystem::ErrMessage("%s|%s not found for deletion(2)", GetName(), pMenuPath);
            }
        }
        else // we found it
        {
            // if it's locked (ie, if menus are being processed)
            if ((*it)->Delete())
            {
                m_menuElements.erase(it);
            }
        }
    }
}

void Menu::ScanForDeletions()
{
    for (ElementList::iterator it = m_menuElements.begin(); it != m_menuElements.end();)
    {
        MenuElement* pME = *it;
        if (pME->isDelete())
        {
            it = m_menuElements.erase(it);
            delete pME;
        }
        else
        {
            if (pME->isMenu())
            {
                static_cast<Menu*>(pME)->ScanForDeletions();
            }
            ++it;
        }
    }
}

void Menu::AddSeparator(const char* pMenuPath)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuSeparator(Menu::GetNameFromPath(pMenuPath)));
}

void Menu::AddMenuItem(const char* pMenuPath, bool& boolVar, const char* pOnStr, const char* pOffStr)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemBool(Menu::GetNameFromPath(pMenuPath), boolVar, pOnStr, pOffStr));
}

void Menu::AddMenuItem(const char* pMenuPath, GSHandler<bool>* pHndlr, const char* pOnStr, const char* pOffStr)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemBoolGS(Menu::GetNameFromPath(pMenuPath), pHndlr, pOnStr, pOffStr));
}

void Menu::AddMenuItem(const char* pMenuPath, int& intVar, int minValue, int maxValue)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemInt(Menu::GetNameFromPath(pMenuPath), intVar, minValue, maxValue));
}

void Menu::AddMenuItem(const char* pMenuPath, GSHandler<int>* pHndlr, int minValue, int maxValue)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemIntGS(Menu::GetNameFromPath(pMenuPath), pHndlr, minValue, maxValue));
}

void Menu::AddMenuItem(const char* pMenuPath, float& floatVar, float minValue, float maxValue, float incValue, float accelMult, const char* pFmt)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemFloat(Menu::GetNameFromPath(pMenuPath), floatVar, minValue, maxValue, incValue, accelMult, pFmt));
}

void Menu::AddMenuItem(const char* pMenuPath, GSHandler<float>* pHndlr, float minValue, float maxValue, float incValue, float accelMult, const char* pFmt)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemFloatGS(Menu::GetNameFromPath(pMenuPath), pHndlr, minValue, maxValue, incValue, accelMult, pFmt));
}

void Menu::AddMenuItem(const char* pMenuPath, CStyleCallbackBoolData cb, void* pUserData)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemCStyleCallbackBoolData(Menu::GetNameFromPath(pMenuPath), cb, pUserData));
}

void Menu::AddMenuItem(const char* pMenuPath, CStyleCallbackVoidData cb, void* pUserData)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemCStyleCallbackVoidData(Menu::GetNameFromPath(pMenuPath), cb, pUserData));
}

void Menu::AddMenuItem(const char* pMenuPath, CStyleCallbackVoidNoData cb)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemCStyleCallbackVoidNoData(Menu::GetNameFromPath(pMenuPath), cb));
}

void Menu::AddMenuItem(const char* pMenuPath, CPPStyleVoidCallback* pCB)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemCPPStyleVoidCallback(Menu::GetNameFromPath(pMenuPath), pCB));
}

void Menu::AddMenuItem(const char* pMenuPath, CPPStyleBoolCallback* pCB)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemCPPStyleBoolCallback(Menu::GetNameFromPath(pMenuPath), pCB));
}

void Menu::AddMenuItem(const char* pMenuPath, CStyleCallbackVoidNoData enableFunc, CStyleCallbackVoidNoData disableFunc, CStyleCallbackBoolNoData statusFunc, const char* pOnStr, const char* pOffStr)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemBoolEnableDisable(Menu::GetNameFromPath(pMenuPath), enableFunc, disableFunc, statusFunc, pOnStr, pOffStr));
}

void Menu::AddMenuItem(const char* pMenuPath, CStyleCallbackVoidNoData toggleFunc, CStyleCallbackBoolNoData statusFunc, const char* pOnStr, const char* pOffStr)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemBoolToggle(Menu::GetNameFromPath(pMenuPath), toggleFunc, statusFunc, pOnStr, pOffStr));
}

void Menu::AddMenuItem(const char* pMenuPath, CStyleCallbackBoolNoData toggleFuncThatReturnsStatus, CStyleCallbackBoolNoData statusFunc, const char* pOnStr, const char* pOffStr)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemBoolToggle(Menu::GetNameFromPath(pMenuPath), toggleFuncThatReturnsStatus, statusFunc, pOnStr, pOffStr));
}

void Menu::AddMenuItem(const char* pMenuPath, ToggleHandler* pHndlr)
{
    AddElementToMenuWithItemPath(pMenuPath, new MenuItemBoolToggleGS(Menu::GetNameFromPath(pMenuPath), pHndlr));
}

void Menu::AddMenuItem(const char* pMenuPath, CStyleCallbackVoidBool setStateFunc, bool bInitialState, const char* pOnStr, const char* pOffStr)
{
	AddElementToMenuWithItemPath(pMenuPath, new MenuItemStateSetter(Menu::GetNameFromPath(pMenuPath), setStateFunc, bInitialState, pOnStr, pOffStr));
}

/***********************************  ************************************/
/****************************** MenuSystem *******************************/
/***********************************  ************************************/

MenuSystem::RootMenu MenuSystem::s_root;
bool    MenuSystem::s_bOn = false;
bool    MenuSystem::s_bShow = true;
bool    MenuSystem::s_bLocked = false;
bool    MenuSystem::s_bSomethingWasDeleted = false;
int     MenuSystem::s_menuAreaLeft = 30;
int     MenuSystem::s_menuAreaTop = 30;
int     MenuSystem::s_menuAreaRight = 580;
int     MenuSystem::s_menuAreaBottom = 420;
bool    MenuSystem::s_bButtonCurrentlyPressed[BUTTON_LAST];
bool    MenuSystem::s_bButtonLastState[BUTTON_LAST];
float   MenuSystem::s_buttonRepeatClock[BUTTON_LAST];

void MenuSystem::SetMenuArea(int x, int y, int width, int height)
{
    s_menuAreaLeft   = x;
    s_menuAreaTop    = y;
    s_menuAreaRight  = x + width;
    s_menuAreaBottom = y + height;
}

void MenuSystem::ShowMenus(bool bShow)
{
    s_bShow = bShow;
}

bool MenuSystem::IsShowMenus()
{
    return s_bShow;
}

bool MenuSystem::isLocked()
{
    return s_bLocked;
}

void MenuSystem::MarkSomethingWasDeleted()
{
    s_bSomethingWasDeleted = true;
}

void MenuSystem::DoMenus()
{
    if (isDebugMenusActive())
    {
        s_bLocked = true;

        // update input
        memcpy(s_bButtonLastState, s_bButtonCurrentlyPressed, sizeof(s_bButtonLastState));

        s_bButtonCurrentlyPressed[BUTTON_SELECT] = Platform::GetInputButtonSelect();
        s_bButtonCurrentlyPressed[BUTTON_CANCEL] = Platform::GetInputButtonCancel();
        s_bButtonCurrentlyPressed[BUTTON_UP]     = Platform::GetInputButtonUp();
        s_bButtonCurrentlyPressed[BUTTON_DOWN]   = Platform::GetInputButtonDown();
        s_bButtonCurrentlyPressed[BUTTON_LEFT]   = Platform::GetInputButtonLeft();
        s_bButtonCurrentlyPressed[BUTTON_RIGHT]  = Platform::GetInputButtonRight();
        s_bButtonCurrentlyPressed[BUTTON_ACCEL]  = Platform::GetInputButtonAccel();

        for (int bb = 0; bb < BUTTON_LAST; ++bb)
        {
            if (!s_bButtonCurrentlyPressed[bb])
            {
                s_buttonRepeatClock[bb] = 0.0f;
            }
            else
            {
                s_buttonRepeatClock[bb] += Platform::GetSecondsElapsed();
            }
        }

        #if MENUSYSTEM_DEBUG_INPUT // debug input
        {
            for (int b = BUTTON_SELECT; b <= BUTTON_ACCEL; b = b + 1)
            {
                char buf[256];
                Button btn = (Button)b;

                sprintf (buf, "%s %s %s %s"
                    , MenuSystem::GetButtonCurrentlyPressed(btn) ? "CURRENT" : "-------"
                    , MenuSystem::GetButtonWasJustPressed(btn)   ? "JUSTP"   : "-----"
                    , MenuSystem::GetButtonWasJustReleased(btn)  ? "JUSTR"   : "-----"
                    , MenuSystem::GetButtonRepeat(btn)           ? "REPEAT"  : "------"
                    );
                Platform::DrawString(10,200+btn*10,0xFFFFFFFF,buf);
            }
        }
        #endif

        MenuContext ctx(GetMenuAreaLeft(), GetMenuAreaTop());

        if (!s_root.Process(ctx))
        {
            MenuSystem::ActivateDebugMenus(false);
        }

        s_bLocked = false;

        if (s_bSomethingWasDeleted)
        {
            s_bSomethingWasDeleted = false;
            s_root.ScanForDeletions();
        }
    }
}

bool MenuSystem::GetButtonCurrentlyPressed(Button btn)
{
    return s_bButtonCurrentlyPressed[btn];
}

bool MenuSystem::GetButtonWasJustPressed(Button btn)
{
    return  s_bButtonCurrentlyPressed[btn] &&
           !s_bButtonLastState[btn];
}

bool MenuSystem::GetButtonWasJustReleased(Button btn)
{
    return !s_bButtonCurrentlyPressed[btn] &&
            s_bButtonLastState[btn];
}

bool MenuSystem::GetButtonRepeat(Button btn)
{
    static float REPEAT_RATE = 1.0f / 6.0f; // in seconds

    if (s_buttonRepeatClock[btn] > REPEAT_RATE)
    {
        s_buttonRepeatClock[btn] = 0.0f;
        return true;
    }
    return GetButtonWasJustPressed(btn);
}

int MenuSystem::GetDigitalAxisRepeat(Axis axis)
{
    switch (axis)
    {
    case AXIS_X:
        return
            (MenuSystem::GetButtonRepeat(BUTTON_LEFT  ) ? -1  : 0) +
            (MenuSystem::GetButtonRepeat(BUTTON_RIGHT ) ?  1  : 0) ;
    case AXIS_Y:
        return
            (MenuSystem::GetButtonRepeat(BUTTON_UP    ) ? -1  : 0) +
            (MenuSystem::GetButtonRepeat(BUTTON_DOWN  ) ?  1  : 0) ;
    default:
        GGSassert(false);
        break;
    }
    return 0;
}

int MenuSystem::GetDigitalAxisCurrentlyPressed(Axis axis)
{
    switch (axis)
    {
    case AXIS_X:
        return
            (MenuSystem::GetButtonCurrentlyPressed(BUTTON_LEFT  ) ? -1  : 0) +
            (MenuSystem::GetButtonCurrentlyPressed(BUTTON_RIGHT ) ?  1  : 0) ;
    case AXIS_Y:
        return
            (MenuSystem::GetButtonCurrentlyPressed(BUTTON_UP    ) ? -1  : 0) +
            (MenuSystem::GetButtonCurrentlyPressed(BUTTON_DOWN  ) ?  1  : 0) ;
    default:
        GGSassert(false);
        break;
    }
    return 0;
}

int MenuSystem::GetDigitalAxisJustPressed(Axis axis)
{
    switch (axis)
    {
    case AXIS_X:
        return
            (MenuSystem::GetButtonWasJustPressed(BUTTON_LEFT  ) ? -1  : 0) +
            (MenuSystem::GetButtonWasJustPressed(BUTTON_RIGHT ) ?  1  : 0) ;
    case AXIS_Y:
        return
            (MenuSystem::GetButtonWasJustPressed(BUTTON_UP    ) ? -1  : 0) +
            (MenuSystem::GetButtonWasJustPressed(BUTTON_DOWN  ) ?  1  : 0) ;
    default:
        GGSassert(false);
        break;
    }
    return 0;
}

float MenuSystem::GetAnalogAxis(Axis axis)
{
    float value = 0.0f;
    switch (axis)
    {
    case AXIS_X:
        value = Platform::GetInputAnalogX();
        break;
    case AXIS_Y:
        value = Platform::GetInputAnalogY();
        break;
    default:
        GGSassert(false);
        break;
    }

    float absValue = value > 0.0f ? value : -value;
    if (absValue < Platform::GetAnalogDeadZone())
    {
        return 0.0;
    }

    // scale value inside deadzone
    absValue   -= Platform::GetAnalogDeadZone();
    float range = 1.0f - Platform::GetAnalogDeadZone();
    value       = absValue / range * ((value < 0) ? -1.0f : 1.0f);

    return value;
}

void MenuSystem::ActivateDebugMenus(bool bOn)
{
    s_bOn = bOn;
}

bool MenuSystem::isDebugMenusActive()
{
    return s_bOn;
}

void MenuSystem::ErrMessageV(const char* pFmt, va_list args)
{
    char buf[256];

    _vsnprintf(buf, sizeof(buf), pFmt, args);
    Platform::TerminalMessage(buf);
}

void MenuSystem::ErrMessage(const char* pFmt, ...)
{
    va_list ap; /* points to each unnamed arg in turn */

    va_start (ap,pFmt); /* make ap point to 1st unnamed arg */
    ErrMessageV (pFmt,ap);
    va_end (ap);    /* clean up when done */
}

} // namespace debug
} // namespace ggs


