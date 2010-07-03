/*=======================================================================*/
/** @file   debugmenus_examples.cpp

            tests / examples for debugmenus

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

#include <string>
#include <list>
#include "debugmenus.h"
#include "debugconsole.h"

/*************************** c o n s t a n t s ***************************/


/******************************* t y p e s *******************************/


/************************** p r o t o t y p e s **************************/


/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/


/**************************** r o u t i n e s ****************************/


/*************************** some global bools ***************************/

bool g_test01;
bool g_test02;
bool g_test03;
bool g_test04;
bool g_test05;
bool g_test06;
bool g_test07;
bool g_test08;
bool g_test09;

/*************************** some global ints ****************************/

int  g_test10;
int  g_test11;

/************************** some global floats ***************************/

float g_test12;
float g_test13;
float g_test14;

/******************* just to make the examples shorter *******************/

using namespace ggs::debug;

/****************** global bool C style getter/setters *******************/

bool getBoolValueFunc()
{
    return g_test09;
}

void setBoolValueFunc(bool v)
{
    g_test09 = v;
}

/*********************** A simple void C callback ************************/

void TestCStyleSimple()
{
    ggs::debug::Console::printf("simple\n");
}

/**************** a simple void c callback with userdata *****************/

void TestCStyleWithData(void* pUserData)
{
    ggs::debug::Console::printf("got called wth (%s)\n", (char*)pUserData);
}

/************** a simple bool c callback that shows control **************/
/*
 * Turns off the menus while it's active and prints
 * something on the screen until the select button is pressed
 *
 */

bool TestCStyleWithControl(void* pUserData)
{
    static int count;
    char buf[256];

    MenuSystem::ShowMenus(false);
    sprintf (buf, "%s : %d", (char*)pUserData, count++);
    Platform::DrawString(100, 100, 0xFFFFFFFF, buf);
    return MenuSystem::GetButtonCurrentlyPressed(MenuSystem::BUTTON_SELECT);
}

/****** A callback showing children can safely delete their parents ******/

void DeleteDebugMenu()
{
    MenuSystem::GetRoot().DeleteMenuElement("debug");
}

/***********************************  ************************************/
/* A class with getters and setters to show member function based stuff **/

class SomeClassWithGettersAndSetters
{
    float m_floatValue;
    bool m_boolValue;
    int m_intValue;
public:
    SomeClassWithGettersAndSetters()
        : m_floatValue(123.0f)
        , m_boolValue(false)
        , m_intValue(456)
    {
    }

    float   getFloatValue() { return m_floatValue; }
    void    setFloatValue(float value) { m_floatValue = value; }
    bool    getBoolValue() { return m_boolValue; }
    void    setBoolValue(bool value) { m_boolValue = value; }
    int     getIntValue() { return m_intValue; }
    void    setIntValue(int value) { m_intValue = value; }
};

// an instance of our class with getters/setters

static SomeClassWithGettersAndSetters g_testClass;

/**************************** An enum example ****************************/

class ClassWithEnum
{
public:
    enum Animal
    {
        Dog,
        Cat,
        Bird,
    };

    Animal   m_petType;

    ClassWithEnum(Animal petType)
        : m_petType(petType)
    { }
};

// in instance of our enum type

ClassWithEnum g_enum01(ClassWithEnum::Cat);

// define a label-value pair

typedef EnumList<ClassWithEnum::Animal> EnumLabelTable;

// create a table of values to labels

EnumLabelTable s_enum01table[] =
{
    { ClassWithEnum::Dog, "Dog", },
    { ClassWithEnum::Cat, "Cat", },
    { ClassWithEnum::Bird, "Bird", },
};

/**********  a hard coded example of creating our own menu item **********/
/*
 * This item uses edits an enum by cycling through the values
 * if left or right is pressed while holding the select button
 */
class MenuItemEnumListCustomExample : public MenuItemValueBase
{
    int m_index;

    bool Process(MenuContext& ctx)
    {
        if (MenuSystem::GetButtonCurrentlyPressed(MenuSystem::BUTTON_SELECT))
        {
            m_index += MenuSystem::GetDigitalAxisRepeat(MenuSystem::AXIS_X);
            if (m_index < 0) m_index = 0;
            if (m_index >= GGS_DEBUGMENU_NUM_ARRAY_ELEMENTS(s_enum01table)) m_index = GGS_DEBUGMENU_NUM_ARRAY_ELEMENTS(s_enum01table) - 1;
            g_enum01.m_petType = s_enum01table[m_index].m_enumValue;
            return true;
        }
        return false;
    }

    void DrawValue(MenuContext& ctx) const
    {
        ctx.DrawHighlightedJustifiedText(MenuContext::JUSTIFY_RIGHT_LEFT, s_enum01table[m_index].m_enumLabel);
    }

public:
    MenuItemEnumListCustomExample(const char* pName)
        : MenuItemValueBase(pName)
        , m_index(0)
    {
    }

};

/********* An example of using an STL collection as MenuElements *********/

// first declare our collection

typedef std::list<std::string*> StrList;

// now an instance of that collection

static StrList s_strList;

// create the type that will handle our collection as a menu

typedef STLMenu<StrList> STLStrListMenu;

// now provide the 6 requred functions
//
// each of these functions receives a "value" argument
// the type of "value" will be a reference to whatever the type
// of our STL collection's elements are. In this example we have
// a list of string pointers
//
// since we know they are references to string pointers we can
// either declare our arguments as references to string pointers
// OR we can use the alias "value_type" which is defined for us.
// Both examples are shown

// draw our collection element as a MenuElement
void STLStrListMenu::iterator::Draw(MenuContext& ctx, std::string*& value) const
{
    ctx.DrawHighlightedText(value->c_str());
}
// return the height of our element
int STLStrListMenu::iterator::GetHeight(value_type& value) const
{
    return Platform::GetFontHeight();
}
// get its left width
int STLStrListMenu::iterator::GetLeftWidth(value_type& value) const
{
    return Platform::GetTextWidth(value->c_str());
}
// get its right width. We aren't using right in this example
int STLStrListMenu::iterator::GetRightWidth(value_type& value) const
{
    return 0;
}
// there's no need to provide a name. It's only used for
// searching via FindOrAddMenu etc.
const char * STLStrListMenu::iterator::GetName(value_type& value) const
{
    return "-na-";
}
// called when our element is selected
bool STLStrListMenu::iterator::Process(MenuContext &ctx, value_type& value)
{
    ggs::debug::Console::printf("selected : (%s)\n", value->c_str());
    return false;
}

/********************** A STL hierarchical example ***********************/
/*
 * The problem with this is we end up wanting two behaviors where
 * only one is defined.
 *
 * Normally we'd like process called on any item selected
 * but if that item has children then we need to show
 * the children.
 *
 * we could solve this by making showing the child menu
 * respond to a different button than selecting the current element
 * but that would make these kinds of menus different than all other
 * menus.
 *
 * I'm not sure what the best solution is.
 */

class SomeHierarchyClass
{
public:
    typedef std::list<SomeHierarchyClass*> SomeHierarchyClassList;
    SomeHierarchyClassList m_children;
    std::string m_name;
public:
    // NOTE: this part is intrusive.
    // since we will not actually allocate Menus
    // to represent each collection we need to save
    // the state of each Menu that will be created
    // on the stack as the hierarchy is processed/drawn

    MenuListBase::MenuState m_menuState; // needed to save the menu state

    // now back to our regular class

    SomeHierarchyClass(const char* pName)
        : m_name(pName)
    {
    }
    const SomeHierarchyClassList& GetChildren()
    {
        return m_children;
    }
    const char* GetName()
    {
        return m_name.c_str();
    }
    void AddChild(SomeHierarchyClass* pChild)
    {
        m_children.push_back(pChild);
    }
};

// some global root for our hierarchy

SomeHierarchyClass g_rootHierarchy("**root**");

// we create a type to handle our hierarchy

typedef STLMenu<SomeHierarchyClass::SomeHierarchyClassList> STLHierarchyMenu;

// supply the six required functions

void STLHierarchyMenu::iterator::Draw(MenuContext& ctx, SomeHierarchyClass*& value) const
{
    ctx.DrawHighlightedText(value->GetName());
}
int STLHierarchyMenu::iterator::GetHeight(SomeHierarchyClass*& value) const
{
    return Platform::GetFontHeight();
}
int STLHierarchyMenu::iterator::GetLeftWidth(value_type& value) const
{
    return Platform::GetTextWidth(value->GetName());
}
int STLHierarchyMenu::iterator::GetRightWidth(value_type& value) const
{
    return 0;
}
const char* STLHierarchyMenu::iterator::GetName(value_type& value) const
{
    return "--doesn't matter because we are not searching for these--";
}
bool STLHierarchyMenu::iterator::Process(MenuContext &ctx, value_type& value)
{
    // skip if we have no children to show
    if (!value->GetChildren().empty())
    {
        // create a menu for the children on the stack
        STLHierarchyMenu subMenu("-NA-", const_cast<SomeHierarchyClass::SomeHierarchyClassList&>(value->GetChildren()));

        // mark it as deletable to override our
        // safe delete features since this menu
        // will be deleted when we go out of scope
        subMenu.SetDeletableWhenLocked();

        // restore the previous menu state
        subMenu.RestoreState(value->m_menuState);

        // show the submenu
        bool bResult = subMenu.Process(ctx);

        // save the new state of the submenu
        subMenu.SaveState(value->m_menuState);

        return bResult;
    }

    return false;
}

/************************** Add our test menus ***************************/

void ExampleMenusInit()
{
    // -- create a hierarchy

    SomeHierarchyClass* pS2 = new SomeHierarchyClass("Tami");
    pS2->AddChild(new SomeHierarchyClass("Rick"));
    pS2->AddChild(new SomeHierarchyClass("Gene"));
    pS2->AddChild(new SomeHierarchyClass("Dave"));

    SomeHierarchyClass* pS1 = new SomeHierarchyClass("Gregg");
    pS1->AddChild(new SomeHierarchyClass("Terry"));
    pS1->AddChild(new SomeHierarchyClass("Carole"));
    pS1->AddChild(pS2);

    g_rootHierarchy.AddChild(new SomeHierarchyClass("Colin"));
    g_rootHierarchy.AddChild(new SomeHierarchyClass("Eric"));
    g_rootHierarchy.AddChild(new SomeHierarchyClass("Aaron"));
    g_rootHierarchy.AddChild(pS1);
    g_rootHierarchy.AddChild(new SomeHierarchyClass("Alan"));

    // create an STL collection

    s_strList.push_back(new std::string("Gregg"));
    s_strList.push_back(new std::string("Tavares"));
    s_strList.push_back(new std::string("Tami"));
    s_strList.push_back(new std::string("Tavares"));
    s_strList.push_back(new std::string("Rick"));
    s_strList.push_back(new std::string("Hurd"));

    // an example of our own custom MenuElement that
    // turns off the menus while it's active

    class MenuItemDrawMeOnly : public MenuItemSimple
    {
        int m_value;
public:
        MenuItemDrawMeOnly(const char* pStr)
            : MenuItemSimple(pStr)
            , m_value(0)
        {
        }
        virtual bool Process(MenuContext &ctx)
        {
            // turn off the drawing of the menus
            MenuSystem::ShowMenus(false);

            // exit if cancelled
            if (MenuSystem::GetButtonWasJustPressed(MenuSystem::BUTTON_CANCEL))
            {
                return false;
            }

            // print something to show we are working
            for (int ii = 0; ii < 10; ++ii)
            {
                char buf[50];

                sprintf (buf, "%d", m_value++);
                Platform::DrawString(100, 100 + ii * 12, 0xFF0000FF, buf);
            }

            return true;
        }
    };

    // an example of a void CPP function object used as a callback

    class MyCPPCallback : public Menu::CPPStyleBoolCallback
    {
        // yea, I know this should be a copied string probably
        // but in my test case the strings are constant so I'm being lazy
        const char* m_pStr;
public:
        MyCPPCallback(const char* pStr)
            : m_pStr(pStr)
        { }
        virtual bool operator ()()
        {
            ggs::debug::Console::printf("got called (%s)\n", m_pStr);
            return false;
        }
    };

    // ---------------------------------
    // add a bunch of tests the easy way

    MenuSystem::GetRoot().AddMenuItem("Debug|Draw Lines", g_test04);
    MenuSystem::GetRoot().AddMenuItem("Debug|Draw Crap", g_test05);

    // separators don't really need names unless you want to search
    // for them later.

    MenuSystem::GetRoot().AddSeparator("foo1");
    MenuSystem::GetRoot().AddMenuItem("Ship", g_test06);
    MenuSystem::GetRoot().AddSeparator("foo2");
    MenuSystem::GetRoot().AddMenuItem("Debug|Draw Stuff", g_test07);
    MenuSystem::GetRoot().AddMenuItem("Debug|some int value", g_test10);
    MenuSystem::GetRoot().AddMenuItem("Debug|some limited int value", g_test11, -10, 20);
    MenuSystem::GetRoot().AddMenuItem("Debug|some float value", g_test12);
    MenuSystem::GetRoot().AddMenuItem("Debug|some float value fast", g_test13, -100.0f, 100.0f, 1.0f);
    MenuSystem::GetRoot().AddMenuItem("Debug|some float slow fast", g_test14, 0.0f, 1.0f, 0.001f, 100.0f);
    MenuSystem::GetRoot().AddMenuItem("Debug|some c style callback 1", TestCStyleSimple);
    MenuSystem::GetRoot().AddMenuItem("Debug|some c style callback 2", TestCStyleWithData, "two");
    MenuSystem::GetRoot().AddMenuItem("Debug|some c style callback 3", TestCStyleWithData, "three");
    MenuSystem::GetRoot().AddMenuItem("Debug|some c style callback 4", TestCStyleWithControl, "four");
    MenuSystem::GetRoot().AddMenuItem("Debug|some c style callback 5", TestCStyleWithControl, "five");
    MenuSystem::GetRoot().AddMenuItem("Debug|some cpp style callback 1", new MyCPPCallback("three"));
    MenuSystem::GetRoot().AddMenuItem("Debug|some cpp style callback 2", new MyCPPCallback("four"));
    MenuSystem::GetRoot().AddElementToMenu("Debug", new MenuItemDrawMeOnly("some no draw menu"));
    MenuSystem::GetRoot().AddMenuItem("Debug|some GS float",
                            new TypeHandler<SomeClassWithGettersAndSetters, float>(
                                    g_testClass,
                                    &SomeClassWithGettersAndSetters::getFloatValue,
                                    &SomeClassWithGettersAndSetters::setFloatValue));
    MenuSystem::GetRoot().AddMenuItem("Debug|some gs float 2",
                            new TypeHandler<SomeClassWithGettersAndSetters, float>(
                                    g_testClass,
                                    &SomeClassWithGettersAndSetters::getFloatValue,
                                    &SomeClassWithGettersAndSetters::setFloatValue));
    MenuSystem::GetRoot().AddMenuItem("Debug|some gs int",
                            new TypeHandler<SomeClassWithGettersAndSetters, int>(
                                    g_testClass,
                                    &SomeClassWithGettersAndSetters::getIntValue,
                                    &SomeClassWithGettersAndSetters::setIntValue));
    MenuSystem::GetRoot().AddMenuItem("Debug|some memfunc gs bool",
                            new TypeHandler<SomeClassWithGettersAndSetters, bool>(
                                    g_testClass,
                                    &SomeClassWithGettersAndSetters::getBoolValue,
                                    &SomeClassWithGettersAndSetters::setBoolValue));
    MenuSystem::GetRoot().AddMenuItem("Debug|some normal func gs bool",
                            new TypeHandlerCFunc<bool>(
                                    &getBoolValueFunc,
                                    &setBoolValueFunc));
    MenuSystem::GetRoot().AddElementToMenu("Debug", new MenuItemEnumListCustomExample("some custom item"));
    MenuSystem::GetRoot().AddElementToMenu("Debug",
        new MenuItemEnum<EnumLabelTable>("an enum example", s_enum01table, GGS_DEBUGMENU_NUM_ARRAY_ELEMENTS(s_enum01table), g_enum01.m_petType));
    MenuSystem::GetRoot().AddElementToMenu("Debug", new STLStrListMenu("stl example", s_strList));
    MenuSystem::GetRoot().AddElementToMenu("Debug", new STLHierarchyMenu("hierarchy example", const_cast<SomeHierarchyClass::SomeHierarchyClassList&>(g_rootHierarchy.GetChildren())));

    // add a specifc menu and add some items to it

    Menu* pGMenu = MenuSystem::GetRoot().FindOrAddMenu("Graphics");
    pGMenu->AddMenuItem("Draw Background", g_test01);
    pGMenu->AddMenuItem("Draw Objects", g_test02);
    pGMenu->AddMenuItem("Draw Terrain", g_test03);

    MenuSystem::GetRoot().AddMenuItem("debug|delete test|do it!", DeleteDebugMenu);

    // add a menu that's too long

    for (int ii = 0; ii < 80; ++ii)
    {
        char buf[100];
        sprintf (buf, "too long|item %d", ii);
        MenuSystem::GetRoot().AddMenuItem(buf, g_test07);
    }
}


