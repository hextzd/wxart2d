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

#define a2dTETO_VERSION "1.0"

class wxDrawingConnector;
class wxTextConnector;
class TetoLuaProcCommandProcessor;

// Define a new application
class MyApp: public wxApp
{
    DECLARE_EVENT_TABLE()

public:
    MyApp( void );
    bool OnInit( void );
    int OnExit( void );
    int OnRun();


    TetoLuaProcCommandProcessor* GetLuaCommandProc() { return m_cmdProc; }

    void SetExit() { m_exit = true; }

protected:
    void OnLua( wxLuaEvent& event );

    a2dSmrtPtr<wxDrawingConnector> m_connector;
    a2dSmrtPtr<wxTextConnector> m_connector2;

private:

    int ParseCmdLine( int argc, wxChar** argv );

    TetoLuaProcCommandProcessor* m_cmdProc;

    wxString m_inputDir;
    wxString m_inputFile;
    wxString m_processFile;

    bool m_quiet;
    bool m_verbose;
    bool m_exit;
};

//GLOBAL use of wxGetApp()
DECLARE_APP( MyApp )

//!
class TetoDocument: public a2dCanvasDocument
{
public:

    TetoDocument( void );

    ~TetoDocument( void );

    void CreateCommandProcessor();

    DECLARE_DYNAMIC_CLASS( TetoDocument )


};

//! Overview canvas  in toplevel frame for showing all open drawings "symbolic"
class OverviewCanvas: public a2dCanvas
{
public:

    OverviewCanvas(  wxFrame* frame,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxScrolledWindowStyle );

    ~OverviewCanvas();

    //! Event Add a document to the manager's list
    /*
    */
    void OnAddDocument( a2dCommandEvent& event );

    //! Event remove a document from the manager's list
    /*
    */
    void OnRemoveDocument( a2dCommandEvent& event );

    //! when clicked on an Open file in the overview, open an extra View of it.
    void OnMouseEvent( wxMouseEvent& event );

    //! single controller with a zoom.
    a2dSmrtPtr<a2dStToolContr> m_contr;

    DECLARE_EVENT_TABLE()
};


// ID for the menu commands
enum
{
    MSCREEN_T  = wxID_HIGHEST + 1000,
    CANVAS_ABOUT
};

//! Top Parent Frame
class VdrawFrame: public a2dDocumentFrame
{
    DECLARE_CLASS( VdrawFrame )

public:

    VdrawFrame( wxFrame* frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size,
                const long type );

    ~VdrawFrame();

    OverviewCanvas* GetOverViewCanvas() { return m_topcanvas; }

    void OnCloseWindow( wxCloseEvent& event );

    void FillData( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    a2dCanvasObject* m_topgroup;

    a2dSmrtPtr<TetoDocument> m_document;

    wxMenu* editMenu;

    OverviewCanvas* m_topcanvas;

    TetoLuaProcCommandProcessor* m_tetoproc;

protected:

    bool ToolDlg();


    DECLARE_EVENT_TABLE()
};


//! specialized TetoLuaWrapper
class TetoLuaProcCommandProcessor: public a2dLuaCentralCommandProcessor
{
    DECLARE_CLASS( TetoLuaWrapper )

public:

    TetoLuaProcCommandProcessor( long flags = a2dDEFAULT_DOCUMENT_FLAGS, bool initialize = true, int maxCommands = -1 );

    ~TetoLuaProcCommandProcessor();

    wxString GetName() { return wxT( "TetoLuaWrapper" ); }

    ToolDlg* GetToolDlg() { return m_tooldlgteto; }

    bool ShowDlg( const a2dCommandId* comID, bool modal, bool onTop );

    void DeleteDlgs();

    virtual void OnExit();

protected:

    ToolDlg* m_tooldlgteto;
};

extern TetoLuaProcCommandProcessor* a2dGetGedi();

#endif
