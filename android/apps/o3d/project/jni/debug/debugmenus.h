/*=======================================================================*/
/** @file   debugmenus.h

            Simple Debug Menu System

            Note: Design goals are to be simple TO USE. As such memory
            usage and speed were generally not considered since these
            are only meant to be used during debugging

            There's a lot of BS in here. Some decisions led to apparently
            complications later. I hope in the end it's usable. I'll
            refactor later if and when I think of better solutions

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

#ifndef GGS_ENGINE_DEBUG_DEBUGMENUS_H
#define GGS_ENGINE_DEBUG_DEBUGMENUS_H

#include <vector>
#include <string>
#include <stdarg.h>
#include "debugplatform.h"

namespace ggs {
namespace debug {

/*************************** c o n s t a n t s ***************************/


/******************************* t y p e s *******************************/

/***********************************  ************************************/
/****************************** MenuContext ******************************/
/***********************************  ************************************/

class MenuElement;
class MenuContext
{
    /**
     * current draw position
     */
    int     m_drawX;
    /**
     * current draw position
     */
    int     m_drawY;
    /**
     * current area (menu) width
     *
     * the maximum width of all the items in the menu
     */
    int     m_areaWidth;
    /**
     * current area height
     */
    int     m_areaHeight;
    /**
     * a menu is divided into a left half and right half
     * this is the width of the left half
     */
    int     m_leftAreaWidth;
    /**
     * true if current item is highlighted
     */
    bool    m_bHighlighted;
    /**
     * true if current item is active
     */
    bool    m_bActive;
    /**
     * the index of the current item
     */
    unsigned    m_index;

public:
    MenuContext(int x, int y);

    /**
     * size of separation between left and right areas of a menu
     */
    static const int MENU_MIDDLE_SIZE   = 10;

    /**
     * Sets the area of a menu so that subsquent call to DrawHighlightedText will function
     *
     * @param width
     * @param height
     * @param leftWidth
     */
    void    SetAreaInfo(int width, int height, int leftWidth);
    /**
     * Sets the Draw Position for DrawHighlightedText or can be references with GetXPos/GetYPos
     *
     * @param xpos
     * @param ypos
     */
    void    SetDrawPosition(int xpos, int ypos);

    /**
     * Get the current X draw position
     *
     * For individual MenuElements this is the left edge of the Menu
     *
     * @return
     */
    int     GetXPos() const;
    /**
     * Get the Current Y Draw Position
     *
     * For an individual MenuElement this is the top of the element
     *
     * @return
     */
    int     GetYPos() const;

    /**
     * Get the X position of the right area of the current Menu
     *
     * Values are typically drawn in the right area
     *
     * @return
     */
    int     GetRightXPos() const;

    /**
     * Get the Width of the current area (Menu)
     *
     * @return
     */
    int     GetAreaWidth() const;
    /**
     * Get the Height of the current area (Menu)
     *
     * @return
     */
    int     GetAreaHeight() const;

    /**
     * Get the width of the left Area
     *
     * The left area of a menu is typically used for Labels
     *
     * @return
     */
    int     GetLeftAreaWidth() const;
    /**
     * Get the width of the right area of the current Menu
     *
     * Get right area is generally used for values
     *
     * @return
     */
    int     GetRightAreaWidth() const;

    /**
     * the index of the current item
     *
     * This might be useful to some MenuElement's Draw or Process function
     * as they can use them to know the position in their own collections
     */
    unsigned GetIndex() const;
    /**
     * Set the index of the current item
     *
     * The menu system uses this to allow you to use GetIndex
     *
     * @param index
     */
    void SetIndex(unsigned index);
    /**
     * Is the current item highlighted
     *
     * @return true if highlighted
     *         false if not
     */
    bool    isHighlighted() const;
    /**
     * Set or Clear the Hightlighted Flag
     *
     * @param v
     */
    void    SetHighlighted(bool v);
    /**
     * Is the current MenuElement active
     *
     * @return true if active
     *         false if not
     */
    bool    isActive() const;

    /**
     * Set or Clear the active flag
     *
     * @param v
     */
    void    SetActive(bool v);
    /**
     * Get the color to draw Text based on the current state of the context.
     *
     * @return
     */
    unsigned int GetTextColor() const;
    /**
     * Draw text in the correct color based on the current state.
     *
     * Draws at the current position which is usually the left edge
     * of the current MenuElement
     *
     * @param pStr   string to draw
     */
    void    DrawHighlightedText(const char* pStr);
    /**
     * Draw text in the correct color based on the current state.
     *
     * Draws at the current position which is usually the left edge
     * of the current MenuElement
     *
     * @param pFmt   printf like format string
     * @param args   va_args from stdarg.h
     */
    void    DrawHighlightedTextV(const char* pFmt, va_list args);
    /**
     * Draw text in the correct color based on the current state using a printf style
     * of passing arguments.
     *
     * Draws at the current position which is usually the left edge
     * of the current MenuElement
     *
     * @param pFmt   printf format string
     */
    void    DrawHighlightedTextF(const char* pFmt, ...);

    enum Justification
    {
        /**
         * Draw at the left edge of the menu
         */
        JUSTIFY_LEFT,
        /**
         * Draw in the center of the menu
         */
        JUSTIFY_CENTER,
        /**
         * Draw right justified in the menu
         */
        JUSTIFY_RIGHT,
        /**
         * Draw on the left of the left area
         */
        JUSTIFY_LEFT_LEFT,
        /**
         * Center in the left area
         */
        JUSTIFY_LEFT_CENTER,
        /**
         * Draw right justified in the left area
         */
        JUSTIFY_LEFT_RIGHT,
        /**
         * Draw on the left of the right area
         */
        JUSTIFY_RIGHT_LEFT,
        /**
         * Draw centered in the right area
         */
        JUSTIFY_RIGHT_CENTER,
        /**
         * Draw right justified in the right area
         */
        JUSTIFY_RIGHT_RIGHT,
    };

    /**
     * Draw Justified text in the correct color for the current state
     *
     * @param j      desired justification
     * @param pStr   text to draw
     */
    void    DrawHighlightedJustifiedText(Justification j, const char* pStr);
    /**
     * Draw Justified text in the correct color for the current state
     *
     * @param j      desired justification
     * @param pFmt   printf like format string
     * @param args   va_args from stdarg.h
     */
    void    DrawHighlightedJustifiedTextV(Justification j, const char* pFmt, va_list args);
    /**
     * Draw Justified text in the correct color for the current state with printf style args
     *
     * @param j      desired justification
     * @param pFmt   printf like format string
     */
    void    DrawHighlightedJustifiedTextF(Justification j, const char* pFmt, ...);
};


inline int  MenuContext::GetAreaWidth() const
{
    return m_areaWidth;
}

inline int MenuContext::GetAreaHeight() const
{
    return m_areaHeight;
}

inline int  MenuContext::GetLeftAreaWidth() const
{
    return m_leftAreaWidth;
}

inline int  MenuContext::GetRightAreaWidth() const
{
    return m_areaWidth - m_leftAreaWidth - MENU_MIDDLE_SIZE;
}

inline unsigned MenuContext::GetIndex() const
{
    return m_index;
}

inline void MenuContext::SetIndex(unsigned index)
{
    m_index = index;
}

inline void MenuContext::SetAreaInfo(int width, int height, int leftWidth)
{
    m_areaWidth     = width;
    m_areaHeight    = height;
    m_leftAreaWidth = leftWidth;
}

inline void MenuContext::SetDrawPosition(int xpos, int ypos)
{
    m_drawX = xpos;
    m_drawY = ypos;
}

inline int MenuContext::GetXPos() const
{
    return m_drawX;
}

inline int MenuContext::GetYPos() const
{
    return m_drawY;
}

inline int MenuContext::GetRightXPos() const
{
    return m_drawX + m_leftAreaWidth + MENU_MIDDLE_SIZE;
}

inline bool MenuContext::isHighlighted() const
{
    return m_bHighlighted;
}

inline void MenuContext::SetHighlighted(bool v)
{
    m_bHighlighted = v;
}

inline void MenuContext::SetActive(bool v)
{
    m_bActive = v;
}

inline bool MenuContext::isActive() const
{
    return m_bActive;
}

/***********************************  ************************************/
/****************************** MenuElement ******************************/
/***********************************  ************************************/

/**
 * base class for all things menu related
 *
 */
class MenuElement
{
    /**
     * true if this item needs to be deleted.
     *
     * The reason we need this is so a child element can delete its parents.
     * In that case we can not delete them immediately but must do it later
     */
    bool    m_bDelete;
    bool    m_bDeletableWhenLocked;
public:
    MenuElement();
    virtual ~MenuElement();

    virtual const char* GetName() const = 0;
    /**
     * used to help us know if we can add child MenuElements to this MenuElement.
     *
     * Although I believe this flag is required it's possible it could be a good
     * idea to add the AddMenuElement etc methods to MenuElement. I didn't feel
     * like I wanted to clutter MenuElement so I cast to a Menu
     *
     * @return true if we can add child MenuElements to this MenuElement
     */
    virtual bool isMenu() const { return false; }
    /**
     *
     * @return
     */
    virtual bool isSelectable() const { return true; }

    /**
     * Process this item
     *
     * @param ctx    current menu context
     *
     * @return Return true if next frame we still want to process input, false if we are finished and should pass reponsibility back to our parent
     */
    virtual bool Process(MenuContext& ctx) { return false; }
    /**
     * Draw this MenuElement
     *
     * This is the draw that gets called as part of
     * drawing this MenuElement as one line of a Menu
     *
     * @param ctx
     */
    virtual void Draw(MenuContext& ctx) const = 0;

    /**
     * get width of the left area
     *
     * @return
     */
    virtual int  GetLeftWidth() const = 0;
    /**
     * get the width of the right area
     *
     * as many menus do not use the right area the default is 0
     *
     * @return
     */
    virtual int  GetRightWidth() const { return 0; }
    /**
     * get the height if this Menu Element
     *
     * @return
     */
    virtual int  GetHeight() const = 0;
    /**
     * True if marked for deletion
     *
     * @return
     */
    bool    isDelete();
    /**
     * Delete this element
     *
     * The reason we need this is so a child element can delete its parents.
     * In that case we can not delete them immediately but must do it later.
     *
     * @return true if actually deleted
     *         false if just marked for deletion
     *
     *         if a Menu is attempting to delete child MenuElement the if this method
     *         returns true it is safe to remove the child from the Menu's collection.
     *         If it returns false though it should keep it on the collection
     *         and later use ScanForDeletions to delete MenuElements that return
     *         true for isDelete()
     */
    bool    Delete();
    void    SetDeletableWhenLocked();
};

/***********************************  ************************************/
/***************************** MenuSeparator *****************************/
/***********************************  ************************************/

/**
 * a non selectable MenuElement that draws a line across the menu
 */
class MenuSeparator : public MenuElement
{
    std::string m_name; // so you can find it later
public:
    MenuSeparator(const char* pName);

    virtual const char * GetName() const;
    virtual bool isSelectable() const { return false; }

    virtual void Draw(MenuContext &ctx) const;
    virtual int GetLeftWidth() const;
    virtual int GetHeight() const;
};

/***********************************  ************************************/
/************************** MenuItemSimpleBase ***************************/
/***********************************  ************************************/

/**
 * A base class for a single line of text menu item
 *
 * It handles returning sizes for left and right areas
 */
class MenuItemSimpleBase : public MenuElement
{
//    int m_leftWidth;
//    int m_rightWidth;
//    int m_height;

public:
    /**
     * return the text we want for this menu item
     *
     * Note: If you want to update the text, do it in Process so that it is consistant during GetHeight and Draw
     *
     * @return
     */
    virtual const char* GetName() const = 0;

    virtual void Draw(MenuContext& ctx) const;

    virtual int  GetLeftWidth() const;
    virtual int  GetRightWidth() const;
    virtual int  GetHeight() const;
};

/***********************************  ************************************/
/**************************** MenuItemSimple *****************************/
/***********************************  ************************************/

/**
 * A basic Menu item that draws one line of text
 */
class MenuItemSimple : public MenuItemSimpleBase
{
    std::string m_name;

    void Init();
public:
    MenuItemSimple(const char* pName);
    MenuItemSimple(const char* pName, std::string::size_type nameLen);

    virtual const char* GetName() const;
};

/***********************************  ************************************/
/******************************* GSHandler *******************************/
/***********************************  ************************************/

/**
 * Getter Setter Handler.
 *
 * This is a template for generating an ABSTRACT base class that provides
 * a getter and setter for a particular type. Because its methods are virtual
 * it can provide a base interface for different ways of getting/setting
 * the specified type (like directy, by callback, my method, etc..)
 */
template <typename T>
class GSHandler
{
public:
	virtual ~GSHandler() { }
    virtual T GetValue() const = 0;
    virtual void  SetValue(T v) = 0;
};

/***********************************  ************************************/
/***************************** ToggleHandler *****************************/
/***********************************  ************************************/

/**
 * an ABSTRACT base class for a bool toggle system.
 *
 * Some bools are set using the method of querying the state of the bool
 * and then calling a toggle function if it is not the desired state.
 */
class ToggleHandler
{
public:
	virtual ~ToggleHandler() { }
    virtual void Toggle() = 0;
    virtual bool GetStatus() = 0;
};

/***********************************  ************************************/
/***************************** MenuListBase ******************************/
/***********************************  ************************************/

/**
 * Displays a list of MenuElements vertically and handles the case
 * where there are too many MenuElements to fit on the screen.
 *
 * After all the MenuElements have been drawn then if there is an
 * active child calls its Process function. (set by SetChildActive
 * and SetHighlightedIndex)
 */
class MenuListBase : public MenuItemSimple
{
public:

    /**
     * This is a struct to make it easy to save and restore
     *
     * This allows you to kind of use the menu system to display hierarchical data
     *
     * See debugmenus_examples.cpp
     */
    struct MenuState
    {
        MenuState();
        /**
         * element that is currently highlighted
         */
        unsigned        m_highlightedNdx;

        /**
         * element that is at the top of the displayed part
         */
        unsigned        m_topNdx;

        /**
         * true if highlighted element that is currently active
         */
        bool            m_bChildActive;
    };
private:
    MenuState   m_state;

protected:
    /**
     * Make sure m_hightlightNdx is not outside the the valid range of MenuElements
     */
    void            BoundHighlight();
    /**
     * Draw all the MenuElements.
     *
     * Makes sure the highlighted MenuElement is on the screen while at the same time
     * handling the case where there are too many elements to fit on the screen
     *
     * Calls Process if the highlighted item is active (SetChildActive)
     *
     * @param ctx
     */
    void            DrawList(MenuContext& ctx);
    void            DrawElements(MenuContext& ctx, bool bTooTall, bool bDraw, int& childXPos, int& childYPos);
    /**
     * Used to skip forward over un-selectable MenuElements.
     * Call after incrementing the highlighted index
     */
    void            SkipForward();
    /**
     * Used to skip backward over un-selectable MenuElements.
     * Call after incrementing the highlighted indexused to s
     */
    void            SkipBackward();

    /**
     * Get the current highlighted index
     *
     * @return
     */
    int             GetHighlightedIndex() { return m_state.m_highlightedNdx; }
    /**
     * Set the current highlighted index
     *
     * @param ndx
     */
    void            SetHighlightedIndex(unsigned ndx) { m_state.m_highlightedNdx = ndx; }
    /**
     * check the status of the current MenuElement
     * IsChildActive means it's Process function is being called
     *
     * @return true if active
     */
    bool            IsChildActive() { return m_state.m_bChildActive; }
    /**
     * Set the highlighted indexed MenuElement as active
     *
     * (i.e., have it's Process function called)
     *
     * @param bActive
     */
    void            SetChildActive(bool bActive) { m_state.m_bChildActive = bActive; }

public:

    /**
     * you might question why this is not an STL iterator
     * that's because STL iterators are compile time iterators
     * this is a runtime iterator
     *
     * I'm not sure this was even close the best way to do this.
     *
     * The reason I have a runtime iterator is I wanted to be able to reuse
     * MenuListBase to display various kinds of lists. The reason is MenuListBase
     * handles non uniform heights as well as handling lists that are too tall
     * for the display. On top of that it handles setting the drawing position
     * for sub elements (right of their parent item) but includes checking that
     * position would be off the screen and adjust appropriately.
     *
     * Because of all that it seemed like a good idea to try to reuse it.
     *
     * Unfortunately this first attempt at a runtime iterator implementation
     * has made many things overly complicated IMO. If it's too complicated for
     * you to use with your own lists just draw your own list in your own
     * Process function. That will work in most cases.
     *
     * Note that because the iterators are virtual at runtime we cannot do things
     * like iter = collection.begin() because iter is virtual.
     *
     * We could do pIter = Create() with a virtual create function but that would
     * mean allocating and deallocating memory many times every frame. Something
     * I wanted to avoid for game development.
     *
     * So, I ended up with a model pIter = GetIter(ITER_ID) where a class
     * is required to provide 3 iterators (MAIN, EXTRA, HIGHLIGHTED,)
     * and provide a function to allow me to copy one to the other.
     *
     * Another reason I can't do iter = collection.begin() is because I didn't
     * want to restrict it to working only with collections that have a begin()
     * function that returns a virtual iterator. So, iterators need to know
     * about their collections so they can handle that stuff virtually. That
     * why there's the need for SetToBegin and SetToEnd etc.
     */
    class MenuElemIter
    {
    public:
		virtual ~MenuElemIter() {}
        /**
         * Sets the iterator to the first element
         */
        virtual void SetToBegin() = 0;
        /**
         * Sets the iterator to one PAST the last element
         */
        virtual void SetToEnd() = 0;
        /**
         * advances to the next element
         */
        virtual void Next() = 0;
        /**
         * goes back to the previous element
         */
        virtual void Prev() = 0;
        /**
         * checks if this is one PAST the last element
         *
         * @return true of this is the END (one past the last element)
         */
        virtual bool IsEnd() = 0;
        /**
         * checks if this is the first element
         *
         * @return true if this is the first element
         */
        virtual bool IsBegin() = 0;
        /**
         * gets a pointer to the element
         *
         * @return
         */
        virtual MenuElement* GetElement() = 0;
    };

    enum eIter
    {
        ITER_MAIN,
        ITER_EXTRA,
        ITER_HIGHLIGHT,

        NUM_ITERS,
    };

    /**
     * override and return the number of elements for your collection
     *
     * @return number of elements in the collection
     */
    virtual unsigned GetNumElements() const = 0;
    /**
     * Get's a pointer to an iterator. 3 iterators are expected
     *
     * @param iterId ID of iterator to get
     *
     * @return the requested iterator
     */
    virtual MenuElemIter& GetIterator(eIter iterId) = 0;
    /**
     * copies one iterator to another so they point to the same element.
     *
     * @param dstId  id of destination iterator
     * @param srcId  id of source iterator
     */
    virtual void CopyIterator(eIter dstId, eIter srcId) = 0;

    /**
     * allows you to use the menu system to display your own hierarchical data
     *
     * see debugmenus_examples.cpp
     *
     * @param state
     */
    void SaveState(MenuState& state) const;
    /**
     * allows you to use the menu system to display your own hierarchical data
     *
     * see debugmenus_examples.cpp
     *
     * @param state
     */
    void RestoreState(const MenuState& state);

public:
    MenuListBase(const char* pName);
    MenuListBase(const char* pName, std::string::size_type nameLen);

};

/***********************************  ************************************/
/************************** STLMenuElemIterBase **************************/
/***********************************  ************************************/

/**
 * A runtime virtual iterator for any STL type of collection
 *
 * You need to override GetElement to provide your own MenuElement
 */
template <typename container_type>
class STLMenuElemIterBase : public MenuListBase::MenuElemIter
{
public:
    /**
     */
    typedef typename container_type::iterator   iterator;
    typedef typename container_type::value_type value_type;

    /**
     * a pointer to your STL collection
     */
protected:
    container_type*                     m_pContainer;
    iterator                            m_it;

    virtual void SetToBegin()
    {
        m_it = m_pContainer->begin();
    }
    virtual void SetToEnd()
    {
        m_it = m_pContainer->end();
    }
    virtual void Next()
    {
        ++m_it;
    }
    virtual void Prev()
    {
        --m_it;
    }
    virtual bool IsEnd()
    {
        return m_it == m_pContainer->end();
    }
    virtual bool IsBegin()
    {
        return m_it == m_pContainer->begin();
    }
    virtual MenuElement* GetElement() = 0;
public:
    STLMenuElemIterBase& operator=(const STLMenuElemIterBase& rhs)
    {
        m_it = rhs.m_it;
        return *this;
    }
    STLMenuElemIterBase()
        : m_pContainer(NULL)
    {
    }
    /**
     * Since we are using an array and we can't init arrays in C++
     * we need this function to do it for us.
     *
     * @param pContainer
     */
    void Init(container_type* pContainer)
    {
        m_pContainer = pContainer;
    }
};

/***********************************  ************************************/
/**************************** STLMenuElemIter ****************************/
/***********************************  ************************************/

/**
 * A runtime virtual iterator for any STL type of collection of MenuElements
 *
 */
template <typename container_type>
class STLMenuElemIter : public STLMenuElemIterBase<container_type>
{
    /**
     */
//    typedef typename container_type::iterator   iterator;
//    typedef typename container_type::value_type value_type;

    virtual MenuElement* GetElement()
    {
        return (*this->m_it);
    }
public:
};

/***********************************  ************************************/
/***************************** MenuListStay ******************************/
/***********************************  ************************************/

/**
 * This is a MenuList that when a child returns false the MenuList still
 * appears. (normal menu behavior)
 */
class MenuListStay : public MenuListBase
{
public:
    MenuListStay(const char* pMenu);
    MenuListStay(const char* pMenu, std::string::size_type nameLen);

    virtual bool    Process(MenuContext& ctx);
};

/***********************************  ************************************/
/***************************** MenuListBack ******************************/
/***********************************  ************************************/

/**
 * This is a MenuList that when an active child returns false from
 * its Process() function the list also exits. This is useful for
 * picking a value from a menu (think a menu of enums).
 *
 * There's a MenuElement showing the value
 * Selecting it starts a Menu with all the values.
 * Each value is itself a MenuElement. When we pick one of these
 * bottom menu elements we want to pop back 2 levels to the top element
 * that is showing the value.
 */
class MenuListBack : public MenuListBase
{
public:
    MenuListBack(const char* pMenu);
    MenuListBack(const char* pMenu, std::string::size_type nameLen);

    virtual bool    Process(MenuContext& ctx);
};

/***********************************  ************************************/
/******************************* MenuBase ********************************/
/***********************************  ************************************/

/**
 * A MenuBase an ABSTRACT base class that it is expected you should
 * be able to add and remove child MenuElements too
 */
class MenuBase : public MenuListStay
{
public:
    MenuBase(const char* pName);
    MenuBase(const char* pName, std::string::size_type nameLen);

    static const char MENU_SEPARATOR = '|';

    /**
     * Is this a MenuBase? Yes!
     *
     * @return returns true for MenuBase so that other functions can know if they can go
     *         deeper when searching for Elements etc...
     */
    virtual bool    isMenu() const { return true; }
    /**
     * remove all child MenuElements
     */
    virtual void    Clear() = 0;

    /**
     * add a child MenuElement
     *
     * @param pElement the MenuElement to add
     */
    virtual void    AddElement(MenuElement* pElement) = 0;
    /**
     * Add a child MenuElement to some child of this MenuElement
     *
     * @param pMenuPath path to child
     * @param pElement
     */
    virtual void    AddElementToMenu(const char* pMenuPath, MenuElement* pElement) = 0;

    /**
     * Find a child or child's child etc.. of this MenuElement
     *
     * @param pMenuPath Path of MenuElement to find
     *
     * @return pointer to found MenuElement
     *         NULL if element not found
     */
    virtual MenuElement*    FindMenuElement(const char* pMenuPath) = 0;
    /**
     * Delete a child MenuElement (or child of child etc..)
     *
     * @param pMenuPath path to child
     */
    virtual void            DeleteMenuElement(const char* pMenuPath) = 0;
protected:
    virtual void       ScanForDeletions() = 0;
};

/***********************************  ************************************/
/********************************* Menu **********************************/
/***********************************  ************************************/

/**
 * A Menu is a concrete MenuBase that allows adding and removing of child
 * MenuElements. It has many methods to assist with adding and removing
 * MenuElements simply.
 */
class Menu : public MenuBase
{
public:
    Menu(const char* pName);
    Menu(const char* pName, std::string::size_type nameLen);
    ~Menu();

    /**
     * delete all child elements.
     */
    virtual void    Clear();

    virtual void    AddElement(MenuElement* pElement);
    virtual void    AddElementToMenu(const char* pMenuPath, MenuElement* pElement);

    virtual MenuElement*    FindMenuElement(const char* pMenuPath);
    virtual void            DeleteMenuElement(const char* pMenuPath);

    /**
     * Search for a Menu by path. If the path does not exist create Menus along the
     * way until it does
     *
     * @param pMenuPath path to desired Menu
     *
     * @return pointer to the Menu or NULL if it could not be found or created.
     *
     *         The only reason this function would return NULL is if something that is NOT
     *         a Menu exists between this Menu and the desired Menu.
     *
     *         In other words, if you have
     *
     *         "Menu|Sub Menu|Value" where Value is a MenuItemBool and
     *         you ask for "Menu|Sub Menu|Value|NewItem" well, you can not put
     *         children under the "Value" since it is not a Menu
     */
    Menu*           FindOrAddMenu(const char* pMenuPath);
    /**
     * Adds the specified MenuElement to a child somewhere below this Menu where
     * the last Element specified in the MenuPath is ignored.
     *
     * In other words, if you the Path is "Menu|SubMenu|Value" it will add the
     * MenuElement under SubMenu (ignoring the "|Value" part)
     *
     * Passing in just "Value" would end up adding to this Menu in ignoring the
     * last part "Value" means all it sees is ""
     *
     * @param pItemPath Path to Item
     * @param pElement  Element you want added
     */
    void            AddElementToMenuWithItemPath(const char* pItemPath, MenuElement* pElement);

    /**
     * Adds a no selectable separator to a menu
     *
     * @param pMenuPath Path to place you want the separated added
     */
    void            AddSeparator(const char* pMenuPath);

    /**
     * Adds a MenuElement that can edit a bool variable.
     *
     * Defaults to displaying on/off for the value but you can set it something else
     * (true/false), (yes/no), (enable/disable), (0,1), (start/stop), (pause/unpause)
     *
     * @param pMenuPath path where you want the element
     * @param boolVar   variable you want the menu to manage
     * @param pOnStr    what to display for a true status. defaults to "on"
     * @param pOffStr   what to display for a false status. defaults to "off"
     */
    void            AddMenuItem(const char* pMenuPath, bool& boolVar, const char* pOnStr = "on", const char* pOffStr = "off");
    /**
     * Adds a MenuElement that can edit a bool variable using getters and setters.
     *
     * Defaults to displaying on/off for the value but you can set it something else
     * (true/false), (yes/no), (enable/disable), (0,1), (start/stop), (pause/unpause)
     *
     * @param pMenuPath path where you want the element
     * @param pHndlr    a GSHandler (Getter Setter Handler) to handle your bool
     *
     *                  Use TypeHandler to generate one.
     * @param pOnStr    what to display for a true status. defaults to "on"
     * @param pOffStr   what to display for a false status. defaults to "off"
     *
     * @example In other words, if you have a class that uses a getter and setter to set a bool
     * you can use this as follows
     *
     * <PRE>
     * AddMenuItem("path|myvar",
     *        new TypeHandler<MyClass, bool>(
     *           instanceOfMyClass,
     *           &MyClass::getBoolValue,
     *           &MyClass::setBoolValue));
     * </PRE>
     *
     * Or if you have global or static functions you can use it like this
     *
     * <PRE>
     * AddMenuItem("path|myvar",
     *        new TypeHandlerCFunc<bool>(
     *           &getBoolValue,
     *           &setBoolValue));
     * </PRE>
     */
    void            AddMenuItem(const char* pMenuPath, GSHandler<bool>* pHndlr, const char* pOnStr = "on", const char* pOffStr = "off");
    /**
     * Adds a MenuElement that can edit an int variable.
     *
     * Optionally you can set the min and max values.
     *
     * @param pMenuPath path where you want the element
     * @param intVar    int variable to edit
     * @param minValue  min value, defaults to -32767 (for no good reason)
     * @param maxValue  max value, defauts to 32767 (for no good reason)
     */
    void            AddMenuItem(const char* pMenuPath, int& intVar, int minValue = -32767, int maxValue = 32767);
    /**
     * Adds a MenuElement that can edit an int variable using getters and setters.
     *
     * @param pMenuPath path where you want the element
     * @param pHndlr    the GSHandler (Getter Setter Handler) to handle your int
     * @param minValue  min value, defaults to -32767 (for no good reason)
     * @param maxValue  max value, defaults to 32767 (for no good reason)
     *
     * @example In other words, if you have a class that uses a getter and setter to set an int
     * you can use this as follows
     *
     * <PRE>
     * AddMenuItem("path|myvar",
     *        new TypeHandler<MyClass, int>(
     *           instanceOfMyClass,
     *           &MyClass::getIntValue,
     *           &MyClass::setIntValue));
     * </PRE>
     *
     * Or if you have global or static functions you can use it like this
     *
     * <PRE>
     * AddMenuItem("path|myvar",
     *        new TypeHandlerCFunc<int>(
     *           &getIntValue,
     *           &setIntValue));
     * </PRE>
     */
    void            AddMenuItem(const char* pMenuPath, GSHandler<int>* pHndlr, int minValue = -32767, int maxValue = 32767);
    /**
     * Adds a MenuElement that can edit a float variable.
     *
     * @param pMenuPath path where you want the element
     * @param floatVar  float variable to edit
     * @param minValue  minimum value, default 0.0f
     * @param maxValue  maximum value, default 1.0f
     * @param incValue  amount to adjust by with each tick or adjustment by the user, default 0.1f
     * @param accelMult amount to multiple increment by if accel button is held down
     * @param pFmt      format, printf style, to draw float. default %7.3f (7 characters, 3 digits after the period)
     */
    void            AddMenuItem(const char* pMenuPath, float& floatVar, float minValue = 0.0f, float maxValue = 1.0f, float incValue = 0.1f, float accelMult = 10.0f, const char* pFmt = "%7.3f");
    /**
     * Adds a MenuElement that can edit a float variable using getters and setters
     *
     * @param pMenuPath path where you want the element
     * @param pHndlr    the GSHandler (Getter Setter Handler) to handle your float
     * @param minValue  minimum value, default 0.0f
     * @param maxValue  maximum value, default 1.0f
     * @param incValue  amount to adjust by with each tick or adjustment by the user, default 0.1f
     * @param accelMult amount to multiple increment by if accel button is held down
     * @param pFmt      format, printf style, to draw float. default %7.3f (7 characters, 3 digits after the period)
     *
     * @example In other words, if you have a class that uses a getter and setter to set a float
     * you can use this as follows
     *
     * <PRE>
     * AddMenuItem("path|myvar",
     *        new TypeHandler<MyClass, float>(
     *           instanceOfMyClass,
     *           &MyClass::getFloatValue,
     *           &MyClass::setFloatValue));
     * </PRE>
     *
     * Or if you have global or static functions you can use it like this
     *
     * <PRE>
     * AddMenuItem("path|myvar",
     *        new TypeHandlerCFunc<float>(
     *           &getFloatValue,
     *           &setFloatValue));
     * </PRE>
     */
    void            AddMenuItem(const char* pMenuPath, GSHandler<float>* pHndlr, float minValue = 0.0f, float maxValue = 1.0f, float incValue = 0.1f, float accelMult = 10.0f, const char* pFmt = "%7.3f");

    // why 3?
    // the first one takes a pointer to a void func.
    // this is for existing function you might have
    // the second is when you need data passed to your callback
    // the third is if you are writing something new. You get data AND you can
    // choose to pass back true if you want to continue to be called

    /**
    * pointer to void function
    */
    typedef void    (*CStyleCallbackVoidNoData)();
    /**
     * Adds a MenuElement that calls a void function when selected
     *
     * @param pMenuPath path where you want the element
     * @param cb        the void function you want called
     */
    void            AddMenuItem(const char* pMenuPath, CStyleCallbackVoidNoData cb);
    /**
     * pointer to a void function that receives a void* as it's only argument
     */
    typedef void    (*CStyleCallbackVoidData)(void* pUserData);
    /**
     * Adds a MenuElement that calls a void function that has single void* userdata argument when selected
     *
     * @param pMenuPath path where you want the element
     * @param cb        void function you want called with the signature
     *
     *                  void func(void* pUserData)
     * @param pUserData userdata to pass to your function when called.
     */
    void            AddMenuItem(const char* pMenuPath, CStyleCallbackVoidData cb, void* pUserData = NULL);
    /**
     * pointer to a function that takes a single void* userdata and returns a bool
     */
    typedef bool    (*CStyleCallbackBoolData)(void* pUserData);
    /**
     * Adds a MenuElement that calls a bool function that has single void* userdata argument when selected
     *
     * Return true from this function to continue to be called next frame
     * Return false when you are finished and want to return control to the parent menu
     *
     * You can use this to continue to process input in your callback, checking joypad buttons
     * or whatever until you are finished.
     *
     * @param pMenuPath path where you want the element
     * @param cb        bool function that has a single void* argument
     * @param pUserData userdata to pass to your function
     */
    void            AddMenuItem(const char* pMenuPath, CStyleCallbackBoolData cb, void* pUserData = NULL);
    /**
     * an ABSTRACT base class for a void callback function object
     *
     * Inherit from this class to use a CPP function object as a response in a menu
     *
     * @see CPPStlyleBoolCallback
     */
    class CPPStyleVoidCallback
    {
    public:
		virtual ~CPPStyleVoidCallback() { }
        virtual void  operator()() = 0;
    };

    /**
     * Adds a MenuElement that calls a void function object when selected
     *
     * @param pMenuPath path where you want the element
     * @param pCB       pointer to a CPPStyleVoidCallback object
     *
     * @see CPPStyleVoidCallback
     */
    void            AddMenuItem(const char* pMenuPath, CPPStyleVoidCallback* pCB);
    /**
     * an ABSTRACT base class for a bool callback function object
     *
     * Inherit from this class to use a CPP function object as a response in a menu
     *
     * Return true from this function to continue to be called next frame
     * Return false when you are finished and want to return control to the parent menu
     *
     * You can use this to continue to process input in your callback, checking joypad buttons
     * or whatever until you are finished.
     *
     * @see CPPStyleVoidCallback
     */
    class CPPStyleBoolCallback
    {
    public:
		virtual ~CPPStyleBoolCallback() { }
        virtual bool operator()() = 0;
    };
    /**
     * Adds a MenuElement that calls a bool function object when selected
     *
     * @param pMenuPath path where you want the element
     * @param pCB       pointer to a CPPSTyleBoolCallback
     *
     * @see CPPStyleBoolCallback
     */
    void            AddMenuItem(const char* pMenuPath, CPPStyleBoolCallback* pCB);

    typedef bool    (*CStyleCallbackBoolNoData)();
    /**
     * Adds a MenuElement that for a ToggleHandler
     *
     * A ToggleHandler is an abstact class that manages a bool through a GetState function and a Toggle function.
     *
     * @param pMenuPath path where you want the element
     * @param pHndlr    pointer to a ToggleHandler
     */
    void            AddMenuItem(const char* pMenuPath, ToggleHandler* pHndlr);

    /**
     * Adds a MenuElement that handles a feature through an enable/disable/status set of functions
     *
     * lets say you have a graphics library with the global functions as follows
     *
     * void Gfx::EnableShadows() // turns on the shadows
     * void Gfx::DisableShadows() // turns off the shadows
     * bool Gfx::IsShadowsEnabled() // returns the state of the shadows
     *
     * You add a MenuElement to manage that feature as follows
     *
     * <PRE>
     * AddMenuItem("Shadows", Gfx::EnableShadows, Gfx::DisableShadows, Gfx::IsShadowsEnabled);
     * </PRE>
     *
     * @param pMenuPath  path where you want the element
     * @param enableFunc function that enables the feature
     * @param disableFunc
     *                   function that disables the feature
     * @param statusFunc function that returns the status of the feature
     * @param pOnStr     what to display for a true status. defaults to "on"
     * @param pOffStr    what to display for a false status. defaults to "off"
     */
    void            AddMenuItem(const char* pMenuPath, CStyleCallbackVoidNoData enableFunc, CStyleCallbackVoidNoData disableFunc, CStyleCallbackBoolNoData statusFunc, const char* pOnStr = "on", const char* pOffStr = "off");
    /**
     *
     * @param pMenuPath  path where you want the element
     *
     * @param toggleFunc
     * @param statusFunc
     * @param pOnStr    what to display for a true status. defaults to "on"
     * @param pOffStr   what to display for a false status. defaults to "off"
     */
    void            AddMenuItem(const char* pMenuPath, CStyleCallbackVoidNoData toggleFunc, CStyleCallbackBoolNoData statusFunc, const char* pOnStr = "on", const char* pOffStr = "off");
    /**
     *
     * @param pMenuPath  path where you want the element
     *
     * @param toggleFuncThatReturnsStatus
     * @param statusFunc
     * @param pOnStr    what to display for a true status. defaults to "on"
     * @param pOffStr   what to display for a false status. defaults to "off"
     */
    void            AddMenuItem(const char* pMenuPath, CStyleCallbackBoolNoData toggleFuncThatReturnsStatus, CStyleCallbackBoolNoData statusFunc, const char* pOnStr = "on", const char* pOffStr = "off");

	/**
     * pointer to function that takes a bool
     */
    typedef void    (*CStyleCallbackVoidBool)(bool);
	/**
	 * Add a MenuElement for a system that has a on/off state
	 * setting function but no way to get the state. (you track the
	 * state yourself)
	 * 
	 * @param pMenuPath path where you want the element
	 * @param setStateFunc
	 *                  function that sets the state
	 * @param bInitialState
	 * @param pOnStr    what to display for a true status.
	 *                  defaults to "on"
	 * @param pOffStr   what to display for a false status.
	 *                  defaults to "off"
	 */
	void            AddMenuItem(const char* pMenuPath, CStyleCallbackVoidBool setStateFunc, bool bInitialState, const char* pOnStr = "on", const char* pOffStr = "off");

private:
    void            Init();

    typedef std::vector<MenuElement*> ElementList;

    ElementList m_menuElements;

    MenuElement*    FindMenuElementOrAddMenu(const char* pMenuPath, const char* pEnd, bool bAdd);
    Menu*           FindOrAddMenu(const char* pMenuPath, const char* pEnd);

    ElementList::iterator FindMenuElementHelper(const char* pMenuPath, const char* pEnd, const char*& pSep);

    static const char* GetNameFromPath(const char* pMenuPath);

    friend class MenuSystem;
    virtual void       ScanForDeletions();

    // collection methods
    typedef STLMenuElemIter<ElementList> MenuMenuElemIter;

    MenuMenuElemIter m_iters[NUM_ITERS];

    virtual unsigned GetNumElements() const { return m_menuElements.size(); }
    virtual MenuElemIter& GetIterator(eIter iterId) { return m_iters[iterId]; }
    /**
     *
     * @param dstId
     * @param srcId
     */
    virtual void CopyIterator(eIter dstId, eIter srcId) { m_iters[dstId] = m_iters[srcId]; }

};

/***********************************  ************************************/
/*************************** MenuItemValueBase ***************************/
/***********************************  ************************************/

/**
 * Abstract Base type for a MenuElement that displays a label on the left and a value on the right
 *
 * override DrawValue to draw the value part
 */
class MenuItemValueBase : public MenuItemSimple
{
    virtual void Draw(MenuContext& ctx) const;

public:
    MenuItemValueBase(const char* pName);

    virtual void DrawValue(MenuContext& ctx) const = 0;
};

/***********************************  ************************************/
/*************************** TypeHandlerCFunc ****************************/
/***********************************  ************************************/

/**
 * template to generate a GSHandler (Getter Setter Handler) class for the specified type to be edited with 2 C style functions
 *
 * @see Menu::AddMenuItem
 */
template <typename T>
class TypeHandlerCFunc : public GSHandler<T>
{
    typedef T     (*Getter)();
    typedef void  (*Setter)(T value);

    Getter  m_getter;
    Setter  m_setter;

public:
    TypeHandlerCFunc(Getter getter, Setter setter)
        : m_getter(getter)
        , m_setter(setter)
    {
    }
    virtual T GetValue() const
    {
        return m_getter();
    }
    virtual void  SetValue(T value)
    {
        m_setter(value);
    }
};

/***********************************  ************************************/
/****************************** TypeHandler ******************************/
/***********************************  ************************************/

/**
 * a template to generate a GSHandler (Getter Setter Handler) for the given type using member functions
 *
 * @see Menu::AddMenuItem
 */
template <typename T, typename T2>
class TypeHandler : public GSHandler<T2>
{
    typedef T2    (T::*Getter)();
    typedef void  (T::*Setter)(T2 value);

    Getter  m_getter;
    Setter  m_setter;
    T&      m_class;

public:
    TypeHandler(T& aclass, Getter getter, Setter setter)
        : m_class(aclass)
        , m_getter(getter)
        , m_setter(setter)
    {
    }

    virtual T2 GetValue() const
    {
        return (m_class.*m_getter)();
    }
    virtual void  SetValue(T2 value)
    {
        (m_class.*m_setter)(value);
    }
};

/***********************************  ************************************/
/************************ MemberFuncVoidCallback *************************/
/***********************************  ************************************/

/**
 * template to generate a member function callback handler
 * for member functions that return void
 */
template <typename T>
class MemberFuncVoidCallback : public Menu::CPPStyleVoidCallback
{
    typedef void (T::*Func)();

    Func    m_func;
    T&      m_class;

public:
    MemberFuncVoidCallback(T& aclass, Func func)
        : m_class(aclass)
        , m_func(func)
    {
    }

    virtual void  operator()()
    {
        (m_class.*m_func)();
    }
};

/***********************************  ************************************/
/************************ MemberFuncToggleHandler ************************/
/***********************************  ************************************/

/**
 * template to generate a member function ToggleHandler
 * that uses member function
 */
template <typename T>
class MemberFuncToggleHandler : public ToggleHandler
{
    typedef void (T::*ToggleFunc)();
    typedef bool (T::*StatusFunc)();

    ToggleFunc  m_toggleFunc;
    StatusFunc  m_statusFunc;
    T&          m_class;

public:
    MemberFuncToggleHandler(T& aclass, ToggleFunc toggleFunc, StatusFunc statusFunc)
        : m_class(aclass)
        , m_toggleFunc(toggleFunc)
        , m_statusFunc(statusFunc)
    {
    }

    virtual void  Toggle()
    {
        (m_class.*m_toggleFunc)();
    }
    virtual bool  GetStatus()
    {
        return (m_class.*m_statusFunc)();
    }
};

/***********************************  ************************************/
/******************************* EnumList ********************************/
/***********************************  ************************************/

/**
 * Macro to return the number of elements in a C array
 */
#define GGS_DEBUGMENU_NUM_ARRAY_ELEMENTS(array) (sizeof(array)/sizeof(array[0]))

/**
 * template to generate a {value, label} type for a given enum
 *
 * @Example :
 *
 * <PRE>
 * enum Animal
 * {
 *     Dog,
 *     Cat,
 *     Bird,
 * };
 *
 * typedef EnumList<Animal> MyEnumLabelTable;
 *
 * MyEnumLabelTable s_enumTable[] =
 * {
 *     { Dog, "Dog", },
 *     { Cat, "Cat", },
 *     { Bird, "Bird", },
 * };
 * </PRE>
 */
template <typename enum_type>
struct EnumList
{
    typedef enum_type  value_type;

    enum_type   m_enumValue;
    const char* m_enumLabel;
};

/***********************************  ************************************/
/************************ ValueLabelArrayElemIter ************************/
/***********************************  ************************************/

/**
 * iterator for value-label pair table
 *
 * @see MenuItemEnumGS
 * @see MenuItemEnum
 */
template <typename elemlabel_type>
class ValueLabelArrayElemIter : public MenuListBase::MenuElemIter
{
    typedef typename elemlabel_type::value_type value_type;

    class LocalMenuElement : public MenuElement
    {
        elemlabel_type*         m_pElemLabel;
        GSHandler<value_type>*  m_pVarGS;
    public:
        LocalMenuElement()
            : m_pVarGS(NULL)
        {
        }
        virtual const char* GetName() const
        {
            return m_pElemLabel->m_enumLabel;
        }
        virtual void Draw(MenuContext& ctx) const
        {
            ctx.DrawHighlightedText(GetName());
        }
        virtual int  GetLeftWidth() const
        {
            return Platform::GetTextWidth(GetName());
        }
        virtual int  GetHeight() const
        {
            return Platform::GetFontHeight();
        }
        // there's something about this that seems like it's overly complicated
        // how do we get info back that this was selected?
        virtual bool Process(MenuContext& ctx)
        {
            m_pVarGS->SetValue(m_pElemLabel->m_enumValue);
            return false;
        }
        void SetElementLabel(elemlabel_type* pElemLabel)
        {
            m_pElemLabel = pElemLabel;
        }
        void SetVarGS(GSHandler<value_type>* pVarGS)
        {
            m_pVarGS = pVarGS;
        }
    };

    elemlabel_type*  m_pBegin;
    elemlabel_type*  m_pEnd;
    elemlabel_type*  m_pElem;

    // marquarade as MenuElement
    LocalMenuElement    m_localElem;

    virtual void SetToBegin()
    {
        m_pElem = m_pBegin;
    }
    virtual void SetToEnd()
    {
        m_pElem = m_pEnd;
    }
    virtual void Next()
    {
        ++m_pElem;
    }
    virtual void Prev()
    {
        --m_pElem;
    }
    virtual bool IsEnd()
    {
        return m_pElem == m_pEnd;
    }
    virtual bool IsBegin()
    {
        return m_pElem == m_pBegin;
    }
    virtual MenuElement* GetElement()
    {
        m_localElem.SetElementLabel(m_pElem);
        return &m_localElem;
    }
public:
    ValueLabelArrayElemIter& operator=(const ValueLabelArrayElemIter& rhs)
    {
        m_pElem = rhs.m_pElem;
        return *this;
    }
    ValueLabelArrayElemIter()
        : m_pBegin(NULL)
        , m_pEnd(NULL)
        , m_pElem(NULL)
    {
    }
    void Init(elemlabel_type* pBegin, unsigned numElements, GSHandler<value_type>* pVarGS)
    {
        m_pBegin = pBegin;
        m_pEnd   = pBegin + numElements;
        m_pElem  = NULL;
        m_localElem.SetVarGS(pVarGS);
    }
};

/***********************************  ************************************/
/**************************** MenuItemEnumGS *****************************/
/***********************************  ************************************/

/**
 * MenuElement that edits an enum using a value-label pair table and
 * a GSHandler(Getter Setter Handler)
 *
 * @Example
 * <PRE>
 * class ClassWithEnum
 * {
 * public:
 *     enum Animal
 *     {
 *         Dog,
 *         Cat,
 *         Bird,
 *     };
 *
 *     Animal getPetType() const { return m_petType; }
 *     void setPetType(Animal petType) { m_petType = petType; }
 *
 * private:
 *     Animal   m_petType;
 *
 * };
 *
 * -- in instance of our enum type
 *
 * ClassWithEnum s_enumInst;
 *
 * -- define a label-value pair
 *
 * typedef EnumList<ClassWithEnum::Animal> MyEnumLabelTable;
 *
 * -- create a table of values to labels
 *
 * MyEnumLabelTable s_enumTable[] =
 * {
 *     { ClassWithEnum::Dog, "Dog", },
 *     { ClassWithEnum::Cat, "Cat", },
 *     { ClassWithEnum::Bird, "Bird", },
 * };
 *
 * AddElementToMenu("Debug",
 *      new MenuItemEnumGS<MyEnumLabelTable>(
 *             "an enum example",
 *             s_enumTable,
 *             GGS_DEBUGMENU_NUM_ARRAY_ELEMENTS(s_enumTable),
 *             TypeHandler<ClassWithEnum::Animal>(
 *                 &ClassWithEnum::getPetType,
 *                 &ClassWithEnum::setPetType
 *                 s_enumInst)));
 *
 * </PRE>
 */
template <typename enumlabel_type>
class MenuItemEnumGS : public MenuListBack
{
public:
    typedef typename enumlabel_type::value_type value_type;
private:

    // collection methods
    typedef ValueLabelArrayElemIter<enumlabel_type> iterator;

    iterator m_iters[NUM_ITERS];
    unsigned m_numElements;

    virtual unsigned GetNumElements() const { return m_numElements; }
    virtual MenuElemIter& GetIterator(eIter iterId) { return m_iters[iterId]; }
    virtual void CopyIterator(eIter dstId, eIter srcId) { m_iters[dstId] = m_iters[srcId]; }

    enumlabel_type* m_pBegin;
    enumlabel_type* m_pCurrentLabel;

    value_type      m_oldValue;

    GSHandler<value_type>*  m_pVarGS;

    void SetFindLabel()
    {
       enumlabel_type* pEL = m_pBegin;
       for (unsigned ii = 0; ii < m_numElements; ++pEL, ++ii)
       {
          if (pEL->m_enumValue == m_pVarGS->GetValue())
          {
             m_pCurrentLabel = pEL;
             return;
          }
       }
       m_pCurrentLabel = NULL;
    }

    virtual void Draw(MenuContext& ctx) const
    {
        MenuListBack::Draw(ctx);

        if (m_oldValue != m_pVarGS->GetValue())
        {
            const_cast<MenuItemEnumGS*>(this)->SetFindLabel();
			*(const_cast<value_type*>(&m_oldValue)) = m_pVarGS->GetValue();
        }

        ctx.DrawHighlightedJustifiedText(ggs::debug::MenuContext::JUSTIFY_RIGHT_LEFT, m_pCurrentLabel ? m_pCurrentLabel->m_enumLabel : "*INVALID*");
    }

    virtual bool Process(MenuContext& ctx)
    {
        if (!IsChildActive())
        {
            // keep menu insync with value
            if (m_oldValue != m_pVarGS->GetValue())
            {
                SetFindLabel();
                m_oldValue = m_pVarGS->GetValue();
                if (m_pCurrentLabel)
                {
                    SetHighlightedIndex(m_pCurrentLabel - m_pBegin);
                }
            }
        }
        return MenuListBack::Process(ctx);
    }

public:
    MenuItemEnumGS(const char* pName, enumlabel_type* pBegin, unsigned numElements, GSHandler<value_type>* pVarGS)
        : MenuListBack(pName)
        , m_numElements(numElements)
        , m_pBegin(pBegin)
        , m_pCurrentLabel(NULL)
        , m_oldValue((value_type)0)
        , m_pVarGS(pVarGS)
    {
        for (int ii = 0; ii < NUM_ITERS; ++ii)
        {
            m_iters[ii].Init(pBegin, numElements, pVarGS);
        }

        // SetFindLabel(); we can't call this because it's going to call stuff in the dervied class and that class has not been inited
    }
};

/***********************************  ************************************/
/***************************** MenuItemEnum ******************************/
/***********************************  ************************************/

/**
 * MenuElement that edits an enum directly using a value-label pair table
 *
 * @Example
 * <PRE>
 * enum Animal
 * {
 *     Dog,
 *     Cat,
 *     Bird,
 * };
 *
 * -- in instance of our enum type
 *
 * Animal s_petType;
 *
 * -- define a label-value pair
 *
 * typedef EnumList<Animal> MyEnumLabelTable;
 *
 * -- create a table of values to labels
 *
 * MyEnumLabelTable s_enumTable[] =
 * {
 *     { Dog, "Dog", },
 *     { Cat, "Cat", },
 *     { Bird, "Bird", },
 * };
 *
 * AddElementToMenu("Debug",
 *      new MenuItemEnum<MyEnumLabelTable>(
 *             "an enum example",
 *             s_enumTable,
 *             GGS_DEBUGMENU_NUM_ARRAY_ELEMENTS(s_enumTable),
 *             s_petType));
 *
 * </PRE>
 */
template <typename enumlabel_type>
class MenuItemEnum : public MenuItemEnumGS<enumlabel_type>
{
    typedef typename enumlabel_type::value_type value_type;

    class EnumGS : public GSHandler<value_type>
    {
        value_type& m_var;
public:
        EnumGS(value_type& var)
            : m_var(var)
        {
        }
        virtual value_type GetValue() const
        {
            return m_var;
        }
        virtual void SetValue(value_type v)
        {
            m_var = v;
        }
    };

    EnumGS  m_gs;

public:
    MenuItemEnum(const char* pName, enumlabel_type* pBegin, unsigned numElements, value_type& var)
        : MenuItemEnumGS<enumlabel_type>(pName, pBegin, numElements, &m_gs)
        , m_gs(var)
    {
    }
};

/***********************************  ************************************/
/******************************** STLMenu ********************************/
/***********************************  ************************************/

/**
 * An iterator to let your STL elements appear as MenuElements
 *
 * @see STLMenu
 */
template <typename T>
class STLMenuElementImposterIter : public ggs::debug::STLMenuElemIterBase<T>
{
	typedef typename ggs::debug::STLMenuElemIterBase<T>::iterator   iterator;
	typedef typename ggs::debug::STLMenuElemIterBase<T>::value_type value_type;

    class STLMenuElementImposter : public MenuElement
    {
        value_type*                 m_pValue;
        STLMenuElementImposterIter* m_pImp;

    public:
        virtual void Draw(MenuContext &ctx) const
        {
            m_pImp->Draw(ctx, *m_pValue);
        }
        virtual int GetHeight() const
        {
            return m_pImp->GetHeight(*m_pValue);
        }
        virtual int GetLeftWidth() const
        {
            return m_pImp->GetLeftWidth(*m_pValue);
        }
        virtual int GetRightWidth() const
        {
            return m_pImp->GetRightWidth(*m_pValue);
        }
        virtual const char * GetName() const
        {
            return m_pImp->GetName(*m_pValue);
        }
        virtual bool Process(MenuContext &ctx)
        {
            return m_pImp->Process(ctx, *m_pValue);
        }
        void SetValue(value_type* pValue)
        {
            m_pValue = pValue;
        }

        void SetImpIter(STLMenuElementImposterIter* pImp)
        {
            m_pImp = pImp;
        }

        STLMenuElementImposter()
            : m_pValue(NULL)
            , m_pImp(NULL)
        {
        }
    };

    STLMenuElementImposter m_elem;

    MenuElement* GetElement()
    {
        m_elem.SetValue(&(*(this->m_it)));
        return &m_elem;
    }

    // you have to provide these for your type
    virtual void Draw(MenuContext& ctx, value_type& value) const;
    virtual int GetHeight(value_type& value) const;
    virtual int GetLeftWidth(value_type& value) const;
    virtual int GetRightWidth(value_type& value) const;
    virtual const char * GetName(value_type& value) const;
    virtual bool Process(MenuContext &ctx, value_type& value);

public:
    STLMenuElementImposterIter()
    {
        m_elem.SetDeletableWhenLocked();
        m_elem.SetImpIter(this);
    }
};

/**
 * Use this to display an STL collection as a menu and get called back for each element in your collection
 *
 * To use you must provide 6 functions. These functions let your elements masquarade as MenuElements
 * Note. Each of the gets passed an reference to an element from your collection.
 * So if your collection is list<string> then you'll be passed a string&. If your collection is a
 * list<string*> you'll be passed a string*& etc..  Also, note, you can use the type "value_type"
 * as an alias for your element type.
 *
 * Example:
 * <PRE>
 *     typedef std::list<std::string*> StrList;
 *
 *     static StrList s_strList;
 *
 *     typedef STLMenu<StrList> STLStrListMenu;
 *
 *     void STLStrListMenu::iterator::Draw(MenuContext& ctx, std::string*& value) const
 *     {
 *         ctx.DrawHighlightedText(value->c_str());
 *     }
 *     int STLStrListMenu::iterator::GetHeight(std::string*& value) const
 *     {
 *         return Platform::GetFontHeight();
 *     }
 *     int STLStrListMenu::iterator::GetLeftWidth(std::string*& value) const
 *     {
 *         return Platform::GetTextWidth(value->c_str());
 *     }
 *     int STLStrListMenu::iterator::GetRightWidth(std::string*& value) const
 *     {
 *         return 0;
 *     }
 *     const char * STLStrListMenu::iterator::GetName(std::string*& value) const
 *     {
 *         return value->c_str();
 *     }
 *     bool STLStrListMenu::iterator::Process(MenuContext &ctx, std::string*& value)
 *     {
 *         printf ("selected : (%s)\n", value->c_str());
 *         return false;
 *     }
 * </PRE>
 *
 * Finally, add your menu like this
 *
 * <PRE>
 *    MenuSystem::GetRoot().AddElementToMenu("Debug", new STLStrListMenu("stl example", myStrList));
 * </PRE>
 */
template <typename T>
class STLMenu : public ggs::debug::MenuListStay
{
    typedef typename T::value_type value_type;
    T&  m_collection;

public:
    STLMenu(const char* pName, T& collection)
        : MenuListStay(pName)
        , m_collection(collection)
    {
        for (int ii = 0; ii < NUM_ITERS; ++ii)
        {
            m_iters[ii].Init(&m_collection);
        }
    };

    typedef STLMenuElementImposterIter<T> iterator;
    iterator m_iters[NUM_ITERS];

    virtual unsigned GetNumElements() const { return m_collection.size(); }
    virtual MenuElemIter& GetIterator(eIter iterId) { return m_iters[iterId]; }
    virtual void CopyIterator(eIter dstId, eIter srcId) { m_iters[dstId] = m_iters[srcId]; }
};

/***********************************  ************************************/
/****************************** MenuSystem *******************************/
/***********************************  ************************************/

/**
 * The base of the menu system
 *
 * The menu system works by you calling MenuSystem::DoMenus()
 * in your main loop or render loop. It is safe to always call
 * DoMenus() as it exits quick of the menus are not active.
 *
 * You can make the menus appear and function by calling
 * MenuSystem::ActivateDebugMenus(true).
 *
 * You can check if the menus are active by calling
 * MenuSystem::isDebugMenusActive() and for example
 * skip processing your AI if they are.
 */
class MenuSystem
{
public:

    /**
     * Gets a reference to the root menu
     *
     * @return reference to the root menu
     */
    static Menu& GetRoot() { return s_root; }

    /**
     * Sets the area on the screen in pixels the menu system
     * will use to display the menus.
     *
     * @param x
     * @param y
     * @param width
     * @param height
     */
    static void  SetMenuArea(int x, int y, int width, int height);

    /**
     * get the left edge of the entire menu system area
     *
     * @return
     */
    static int   GetMenuAreaLeft();
    /**
     * get the right edge of the entire menu system area
     *
     * @return
     */
    static int   GetMenuAreaRight();
    /**
     * get the top edge of the entire menu system area
     *
     * @return
     */
    static int   GetMenuAreaTop();
    /**
     * get the bottom edge of the entire menu system area
     *
     * @return
     */
    static int   GetMenuAreaBottom();

    /**
     * Process and display the menus.
     *
     * Call every frame in your update or render loop
     */
    static void  DoMenus();
    /**
     * check if the menus are activeq
     *
     * @return true if menus are active
     */
    static bool  isDebugMenusActive();
    /**
     * turn the menus on or off
     *
     * @param bOn    true = menus on
     */
    static void  ActivateDebugMenus(bool bOn);
    /**
     * for internal use only
     *
     * This is part of the system that lets MenuElements delete their parents.
     * The system is "locked" when it's unsafe to delete a MenuElement in
     * which case they will mark themselves for later removal.
     *
     * @return true if locked
     */
    static bool  isLocked();
    /**
     * Allows you to turn off them menus inside a Process function.
     *
     * So, for example, you have a debug GUI that brings up a list of objects
     * to select and you don't want to put all those objects on a menu.
     *
     * You can create a MenuElement that when it's Process function is first called
     * it calls MenuSystem::ShowMenus(false) at which point the menus will stop
     * displaying. They will automatically be turned on when your Process() returns
     * false to indicate you are finished.
     *
     * @param bShow
     */
    static void  ShowMenus(bool bShow);
    /**
     * for internal use
     *
     * The menu system uses this to skip drawing if false
     *
     * @return
     */
    static bool  IsShowMenus();

    static void  ErrMessageV(const char* pFmt, va_list args);
    static void  ErrMessage(const char* pFmt, ...);

    enum Button
    {
        BUTTON_SELECT,
        BUTTON_CANCEL,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_RIGHT,
        BUTTON_ACCEL,

        BUTTON_LAST,
    };

    enum Axis
    {
        AXIS_X,
        AXIS_Y,
    };

    /**
     * check if a button is currently held down
     *
     * @param btn    button to check
     *
     * @return true of the button is currently held down
     */
    static bool  GetButtonCurrentlyPressed(Button btn);
    /**
     * check if a button was just pressed since the last call to DoMenus()
     *
     * @param btn    button to check
     *
     * @return true if button was just pressed
     */
    static bool  GetButtonWasJustPressed(Button btn);
    /**
     * Check if a button was just released ... since the last call to DoMenus
     *
     * @param btn    button to check
     *
     * @return true if just released
     */
    static bool  GetButtonWasJustReleased(Button btn);
    /**
     * check if button was just pressed or is repeating
     *
     * @param btn    button to check
     *
     * @return returns true if button was just pressed or again
     *         every few moments as long as button is still held
     */
    static bool  GetButtonRepeat(Button btn);

    /**
     * check currently held direction on DPAD
     *
     * @param axis   axis to check
     *
     * @return -1, 0 or 1 depending on direction on dpad
     */
    static int   GetDigitalAxisCurrentlyPressed(Axis axis);
    /**
     * Get direction just pressed on dpad (since last frame)
     *
     * @param axis   axis to check
     *
     * @return -1, 0, or 1 depending on which direction was just pressed
     */
    static int   GetDigitalAxisJustPressed(Axis axis);
    /**
     * Get repeating direction on dpad.
     *
     * @param axis   axis to check
     *
     * @return returns -1, 0 or 1 if dpad was just pressed or every
     *         few moments while still held.
     */
    static int   GetDigitalAxisRepeat(Axis axis);

    /**
     * Get left analog stick direction value
     *
     * @param axis   axis to check
     *
     * @return -1.0 to 1.0
     */
    static float GetAnalogAxis(Axis axis);

private:
    friend class MenuElement;
    /**
     * for internal use only.
     *
     * If a MenuElement is in use it can not be deleted immediately
     * so it is marked for deletion. So the system does not have
     * to scan all the menu items every frame this flag is set
     * as a quick shortcut not to check everything.
     */
    static void     MarkSomethingWasDeleted();

    class RootMenu : public Menu
    {
    public:
        RootMenu()
            : Menu("*root*")
        { }
    };

    static RootMenu s_root;
    static bool     s_bOn;
    static bool     s_bShow;
    static bool     s_bLocked;
    static bool     s_bSomethingWasDeleted;
    static int      s_xPos;
    static int      s_yPos;
    static int      s_menuAreaLeft;
    static int      s_menuAreaTop;
    static int      s_menuAreaRight;
    static int      s_menuAreaBottom;
    static bool     s_bButtonCurrentlyPressed[BUTTON_LAST];
    static bool     s_bButtonLastState[BUTTON_LAST];
    static float    s_buttonRepeatClock[BUTTON_LAST];
};

inline int MenuSystem::GetMenuAreaLeft()
{
    return s_menuAreaLeft;
}

inline int MenuSystem::GetMenuAreaTop()
{
    return s_menuAreaTop;
}

inline int MenuSystem::GetMenuAreaRight()
{
    return s_menuAreaRight;
}

inline int MenuSystem::GetMenuAreaBottom()
{
    return s_menuAreaBottom;
}


/************************** p r o t o t y p e s **************************/


/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/


/**************************** r o u t i n e s ****************************/

} // namespace debug
} // namespace ggs

#endif // GGS_ENGINE_DEBUG_DEBUGMENUS_H

