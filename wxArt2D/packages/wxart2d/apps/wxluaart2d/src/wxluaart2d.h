/////////////////////////////////////////////////////////////////////////////
// Purpose:     Standalone wxLua application
// Author:      Francis Irving
// Created:     16/01/2002
// Modified:    J. Winwood. Added debugging support
//              May 2002.
// Copyright:   (c) 2002 Creature Labs. All rights reserved.
// Copyright:   (c) 2002 Lomtick Software. All rights reserved.
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#ifndef WX_LUAAPP_H
#define WX_LUAAPP_H

#include "wx/app.h"
#include "wxlua/debugger/wxldtarg.h"
#include "lconsoleart2d.h"

#include "wx/editor/editmod.h"

#if wxART2D_USE_LUA
#include "wxlua/debug/wxldebug.h"
#include "wxlua/debugger/wxldserv.h"
#include "wxlua/debugger/wxldtarg.h"
#include "wx/luawraps/luawrap.h"
#include "wx/luawraps/luabind.h"
#include "wxlua/wxlstate.h"
#endif

class WXDLLIMPEXP_WXLUA wxLuaEvent;

//! extend a2dWindowViewConnector  to set initial mapping in world coordinates for the view.
/*! In principle it sets the initial values for the a2dView.
    Next to that it swicthes the toolcontroller to this view
*/
class a2dConnector: public a2dViewConnector
{
    DECLARE_EVENT_TABLE()

public:

    a2dConnector();

    void SetDisplayWindow( a2dCanvas* display );

    void OnPostCreateView( a2dTemplateEvent& event );
    void OnPreAddCreateDocument( a2dTemplateEvent& event );
    void OnPostCreateDocument( a2dTemplateEvent& event );
    void OnDisConnectView(  a2dTemplateEvent& event );

	a2dCanvas* m_canvas;
};

// ----------------------------------------------------------------------------
// wxLuaStandaloneApp
// ----------------------------------------------------------------------------

class wxLuaStandaloneApp : public wxApp
{
public:
    // Override the base class virtual functions
    virtual bool OnInit();
    virtual int  OnExit();

    void DisplayMessage( const wxString& msg, bool is_error,
                         const wxLuaState& wxlState = wxNullLuaState );

    void TemplatesInit();

    void OnLua( wxLuaEvent& event );

    wxString            m_programName;
    wxLuaConsoleWrapper m_luaConsoleWrapper;
    bool                m_print_stdout;
    bool                m_print_msgdlg;
    bool                m_want_console;
    bool                m_mem_bitmap_added;
    wxLuaDebugTarget*   m_wxlDebugTarget;

    a2dLuaCentralCommandProcessor* m_cmdProc;

    a2dSmrtPtr<a2dConnector> m_singleconnector;

private:
    DECLARE_EVENT_TABLE();
};

DECLARE_APP( wxLuaStandaloneApp )

#endif // WX_LUAAPP_H
