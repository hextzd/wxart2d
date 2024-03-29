/////////////////////////////////////////////////////////////////////////////
// Purpose:     Interface to a console to help debug wxLua
// Author:      Francis Irving
// Created:     16/01/2002
// Copyright:   (c) 2002 Creature Labs. All rights reserved.
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#ifndef WX_LUA_CONSOLE_H
#define WX_LUA_CONSOLE_H

#include "wx/frame.h"
#include "wxlua/wxlua.h"

class WXDLLEXPORT wxSplitterWindow;
class WXDLLEXPORT wxTextCtrl;
class WXDLLEXPORT wxListBox;

class wxLuaConsoleWrapper;

// ----------------------------------------------------------------------------
// wxLuaConsole - define a console class to display print statements
// ----------------------------------------------------------------------------

class wxLuaConsole : public wxFrame
{
public:
    wxLuaConsole( wxLuaConsoleWrapper* consoleWrapper,
                  wxWindow* parent, wxWindowID id,
                  const wxString& title = wxT( "wxLua console" ),
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxSize( 300, 400 ),
                  long style = wxDEFAULT_FRAME_STYLE,
                  const wxString& name = wxT( "wxLuaConsole" ) );

    // Display a message in the console
    void DisplayText( const wxString& msg );
    // Display the stack in a wxListBox, but only if there are any items in it
    void DisplayStack( const wxLuaState& wxlState );
    // An error has occurred, when this window is closed call wxExit to close
    //   the app.
    void SetExitOnError( bool is_error ) { m_exit_on_error = m_exit_on_error || is_error; }

protected:
    void OnCloseWindow( wxCloseEvent& event );

    wxLuaConsoleWrapper* m_wrapper;
    wxSplitterWindow*    m_splitter;
    wxTextCtrl*          m_textCtrl;
    wxListBox*           m_debugListBox;
    bool                 m_exit_on_error;

private:
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// wxLuaConsoleWrapper - A smart pointer like wrapper for the wxLuaConsole
//
// Create one as a member of the wxApp or whereever it will exist longer than
// the wxLuaConsole it wraps and the wxLuaConsole will NULL the pointer to
// it when closed. See wxLuaConsole::OnCloseWindow(wxCloseEvent&) as to why
// we simply can't catch the close event elsewhere.
// ----------------------------------------------------------------------------

class wxLuaConsoleWrapper
{
public:

    wxLuaConsoleWrapper( wxLuaConsole* c = NULL ) : m_luaConsole( c ) {}

    bool Ok() const { return m_luaConsole != NULL; }

    wxLuaConsole* GetConsole(); // this will assert if console is NULL, check with Ok()
    void SetConsole( wxLuaConsole* c ) { m_luaConsole = c; }

protected:
    wxLuaConsole* m_luaConsole;
};


#endif // WX_LUA_CONSOLE_H
