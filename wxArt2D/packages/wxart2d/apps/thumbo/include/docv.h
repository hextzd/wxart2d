/////////////////////////////////////////////////////////////////////////////
// Name:        docv.h
// Purpose:     Document/view demo
// Author:      Klaas Holwerda
// Modified by:
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef __DOCVIEWSAMPLEH__
#define __DOCVIEWSAMPLEH__

#include "wx/editor/editmod.h"

#if wxART2D_USE_LUA
#include "wxlua/debug/wxldebug.h"
#include "wxlua/debugger/wxldserv.h"
#include "wxlua/debugger/wxldtarg.h"
#include "wx/luawraps/luawrap.h"
#include "wx/luawraps/luabind.h"
#include "wxlua/wxlstate.h"
#endif

#define a2dTHUMBO_VERSION       "1.0"

class wxDrawingConnector;
class wxTextConnector;
class ThumboLuaProcCommandProcessor;

//!
class ThumboDocument: public a2dCanvasDocument
{
public:

    ThumboDocument( void );

    ~ThumboDocument( void );

    void CreateCommandProcessor();

    DECLARE_DYNAMIC_CLASS( ThumboDocument )

};

// Define a new application
class MyApp: public wxApp
{
    DECLARE_EVENT_TABLE()

public:
    MyApp( void );
    ~MyApp( void );
    bool OnInit( void );
    int OnExit( void );
    int OnRun();

    void FillDocument2( ThumboDocument* doc );
    void FillDocument3( ThumboDocument* doc );
    void FillDocument5( ThumboDocument* doc );

#if (wxART2D_USE_LUA == 1)
    ThumboLuaProcCommandProcessor* GetLuaCommandProc() { return m_cmdProc; }
#endif

    void SetExit() { m_exit = true; }

protected:

#if (wxART2D_USE_LUA == 1)
    void OnLua( wxLuaEvent& event );
#endif

    a2dSmrtPtr<wxDrawingConnector> m_connector;
    a2dSmrtPtr<wxTextConnector> m_connector2;

private:

    int ParseCmdLine( int argc, wxChar** argv );

#if (wxART2D_USE_LUA == 1)
    ThumboLuaProcCommandProcessor* m_cmdProc;
#else
    a2dCentralCanvasCommandProcessor* m_cmdProc;
#endif

    wxString m_inputDir;
    wxString m_inputFile;
    wxString m_processFile;

    bool m_quiet;
    bool m_verbose;
    bool m_exit;
};

//GLOBAL use of wxGetApp()
DECLARE_APP( MyApp )


// ID for the menu commands
enum
{
    MSCREEN_T  = wxID_HIGHEST + 1000,
    CANVAS_ABOUT
};

#if (wxART2D_USE_LUA == 1)

//! specialized ThumboLuaWrapper
class ThumboLuaProcCommandProcessor: public a2dLuaCentralCommandProcessor
{
    DECLARE_CLASS( ThumboLuaProcCommandProcessor )

public:

    ThumboLuaProcCommandProcessor( long flags = a2dDEFAULT_DOCUMENT_FLAGS, bool initialize = true, int maxCommands = -1 );

    ~ThumboLuaProcCommandProcessor();

    wxString GetName() { return wxT( "ThumboLuaWrapper" ); }

    ToolDlg* GetToolDlg() { return m_tooldlgThumbo; }

    bool ShowDlg( const a2dCommandId* comID, bool modal, bool onTop );

    void DeleteDlgs();

    virtual void OnExit();

protected:

    ToolDlg* m_tooldlgThumbo;
};

extern ThumboLuaProcCommandProcessor* a2dGetThumbo();
#endif

#endif
