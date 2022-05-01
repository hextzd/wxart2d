/////////////////////////////////////////////////////////////////////////////
// Purpose:     Standalone wxLua application
// Author:      Francis Irving
// Created:     16/01/2002
// Modifications: J. Winwood. Bug fixes to original wxLua code.
// RCS-ID:      $Id: wxluaart2d.cpp,v 1.3 2009/09/01 21:13:56 titato Exp $
// Copyright:   (c) 2002 Creature Labs. All rights reserved.
// Copyright:   (c) 2001-2002 Lomtick Software. All rights reserved.
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#include "a2dprec.h"

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXGTK__
#include <locale.h>
#endif

#include "wx/cmdline.h"
#include "wx/fs_mem.h"
#include "wx/image.h"

#include "wxlua/wxlua.h"
#include "wxlua/debugger/wxldserv.h"
#include "wxluaart2d.h"
#include "editorart2d.h"

#include "wxluasmall.xpm"

extern "C"
{
#include "lualib.h"
}

#define ID_WXLUA_CONSOLE 100

// Declare the binding initialization functions
// Note : We could also do this "extern bool wxLuaBinding_XXX_init();" and
//        later call "wxLuaBinding_XXX_init();" to initialize it.
//        However we use the macros that include #if conditions to have a
//        better chance of determining what libs are available.
// Note : Make sure you link to the binding libraries.

#include "wxbind/include/wxbinddefs.h"
WXLUA_DECLARE_BIND_ALL;

WXLUA_DECLARE_BIND_WXXML;
WXLUA_DECLARE_BIND_WXXRC;
WXLUA_DECLARE_BIND_WXHTML;
WXLUA_DECLARE_BIND_WXAUI;
WXLUA_DECLARE_BIND_WXSTC;

#if wxART2D_USE_LUA
#include "wx/luawraps/luawrap.h"
#include "wx/luawraps/luabind.h"
#endif


// ---------------------------------------------------------------------------
// wxCmdLineEntryDesc
// ---------------------------------------------------------------------------

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
    {
        wxCMD_LINE_SWITCH, "h", "help", "Help on command line switches.",
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_OPTION_HELP
    },

    {
        wxCMD_LINE_SWITCH, "s", "stdout", "Do NOT print output to command line stdout.",
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL
    },

    {
        wxCMD_LINE_SWITCH, "m", "msgdlg", "Print output to a message dialog.",
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL
    },

    {
        wxCMD_LINE_SWITCH, "c", "console", "Print output to a persistent message console.",
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL
    },

    {
        wxCMD_LINE_SWITCH, "q", "quit", "Do not exit even if there are no top-level windows open.",
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL
    },

    {
        wxCMD_LINE_OPTION, "e", "stat", "Execute a Lua string 'stat'.",
        wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL
    },

    {
        wxCMD_LINE_OPTION, "d", "debuggee", "Run as debuggee -d[host]:[port] (for internal use)",
        wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL
    },

    {
        wxCMD_LINE_OPTION, "o", "open", "Open file(s) to edit.",
        wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE
    },

    {
        wxCMD_LINE_OPTION, "r", "run", "Run a Lua program with command line args.",
        wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE
    },

    {
        wxCMD_LINE_PARAM,  "", "", "Run a Lua program with command line args.",
        wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE
    },

    { wxCMD_LINE_NONE }
};

// Find the index into the argv that matches name starting at arg start_n
// This is necessary since the wxCmdLineParser drops -- and all -X
//  params so we use this to make sure that the Lua program is at 0.
int FindFirstCmdParam( int start_n, const wxString& name,
                       int argc, wxChar** argv )
{
    for ( int arg_n = start_n; arg_n < argc; arg_n++ )
    {
        if ( argv[arg_n] == name )
            return arg_n - 1;
    }

    return start_n; //oops? just return what was given.
}

// ---------------------------------------------------------------------------
// a2dConnector
// ---------------------------------------------------------------------------


BEGIN_EVENT_TABLE( a2dConnector, a2dViewConnector )
    EVT_POST_CREATE_VIEW( a2dConnector::OnPostCreateView )
    EVT_PRE_ADD_DOCUMENT( a2dConnector::OnPreAddCreateDocument )
    EVT_POST_CREATE_DOCUMENT( a2dConnector::OnPostCreateDocument )
    EVT_DISCONNECT_VIEW( a2dConnector::OnDisConnectView )
END_EVENT_TABLE()

a2dConnector::a2dConnector()
    : a2dViewConnector()
{
	m_canvas = NULL;
}

void a2dConnector::OnPostCreateView( a2dTemplateEvent& event )
{
	//all fine.
}

void a2dConnector::OnPreAddCreateDocument( a2dTemplateEvent& event )
{
    a2dDocument* newdoc = event.GetDocument();
    a2dCanvasDocument* current = ( a2dCanvasDocument* ) a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentDocument();
    if ( current )
        current->Close( true );
}

void a2dConnector::OnPostCreateDocument( a2dTemplateEvent& event )
{
    //a2dDocumentTemplate* templ = (a2dDocumentTemplate*) event.GetEventObject();
    if ( !m_canvas )
        return;

    a2dViewTemplate* viewtempl;
    viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates().front();

    a2dCanvasView* view = ( a2dCanvasView* ) viewtempl->CreateView( event.GetDocument(), event.GetFlags() );
    view->SetPrintTitle( false );
    view->SetPrintFilename( false );
    view->SetPrintFrame( false );

	view->SetDisplayWindow( m_canvas );
    event.SetView( view );

    if ( view )
    {
        if ( view->GetDrawingPart()->GetCanvasToolContr() )
            view->GetDrawingPart()->GetCanvasToolContr()->StopAllTools();

        view->SetDocument( event.GetDocument() );
        if ( view->GetDrawingPart()->GetCanvasToolContr() )
            view->GetDrawingPart()->GetCanvasToolContr()->ReStart();

        view->GetDrawingPart()->SetMappingShowAll();
        view->Activate( true );
        return;
    }
    wxLogMessage( _( "a2dConnector no view was created" ) );
}

void a2dConnector::OnDisConnectView(  a2dTemplateEvent& event )
{
    a2dSmrtPtr<a2dCanvasView> theViewToDisconnect = ( a2dCanvasView* ) event.GetEventObject();

    theViewToDisconnect->Activate( false );
    if ( theViewToDisconnect->GetDocument() )
    {
        a2dDocumentEvent eventremove( wxEVT_REMOVE_VIEW, theViewToDisconnect, true, theViewToDisconnect->GetId() );
        eventremove.SetEventObject( theViewToDisconnect );
        theViewToDisconnect->GetDocument()->ProcessEvent( eventremove );
    }
    if ( m_canvas->GetDrawingPart()->GetCanvasToolContr() )
        m_canvas->GetDrawingPart()->GetCanvasToolContr()->StopAllTools();

    if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->GetBusyExit() )
    {
        if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().size() > 1 )
        {
            a2dDocumentList::const_reverse_iterator iter = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().rbegin();
            iter++;
            theViewToDisconnect->SetDocument( ( *iter ) );
        }
        else
        {
            a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateDocuments( wxT( "dummy" ), a2dREFDOC_NEW );
            a2dCanvasDocument* document = ( a2dCanvasDocument* ) a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().back().Get();
            theViewToDisconnect->SetDocument( document );
        }

        if ( m_canvas->GetDrawingPart()->GetCanvasToolContr() )
            m_canvas->GetDrawingPart()->GetCanvasToolContr()->ReStart();

        m_canvas->GetDrawingPart()->SetMappingShowAll();

        theViewToDisconnect->Activate( true );
    }
    else
        event.Skip();
}

void a2dConnector::SetDisplayWindow( a2dCanvas* display )
{
    m_canvas = display;
}

// ---------------------------------------------------------------------------
// wxLuaStandaloneApp
// ---------------------------------------------------------------------------
IMPLEMENT_APP( wxLuaStandaloneApp )

BEGIN_EVENT_TABLE( wxLuaStandaloneApp, wxApp )
    EVT_LUA_PRINT       ( wxID_ANY, wxLuaStandaloneApp::OnLua )
    EVT_LUA_ERROR       ( wxID_ANY, wxLuaStandaloneApp::OnLua )
    EVT_LUA_DEBUG_HOOK  ( wxID_ANY, wxLuaStandaloneApp::OnLua )
END_EVENT_TABLE()

bool wxLuaStandaloneApp::OnInit()
{
    m_programName       = argv[0];  // The name of this program
    m_print_stdout      = true;
    m_print_msgdlg      = false;
    m_want_console      = false;
    m_wxlDebugTarget    = NULL;
    m_mem_bitmap_added  = false;

    int  arg_count = 0;     // counter for args used during parsing
    bool run_ok    = false; // has the Lua program executed ok
    bool dont_quit = false; // user specified -q switch to not quit

#if defined(__WXMSW__) && wxCHECK_VERSION(2, 3, 3)
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD( 1, 1 );
    WSAStartup( wVersionRequested, &wsaData );
#endif // defined(__WXMSW__) && wxCHECK_VERSION(2, 3, 3)

    wxInitAllImageHandlers();
    wxFileSystem::AddHandler( new wxMemoryFSHandler );
    wxMemoryFSHandler::AddFile( wxT( "wxLua" ), wxBitmap( wxLuaSmall_xpm ), wxBITMAP_TYPE_XPM );
    m_mem_bitmap_added = true;

#ifdef __WXGTK__
    // this call is very important since otherwise scripts using the decimal
    // point '.' could not work with those locales which use a different symbol
    // (e.g. the comma) for the decimal point...
    // It doesn't work to put os.setlocale('c', 'numeric') in the Lua file that
    // you want to use decimal points in. That's because the file has been lexed
    // and compiler before the locale has changed, so the lexer - the part that
    // recognises numbers - will use the old locale.
    setlocale( LC_NUMERIC, "C" );
#endif


    // Initialize the wxLua bindings we want to use.
    // See notes for WXLUA_DECLARE_BIND_STD above.
    WXLUA_IMPLEMENT_BIND_WXLUA 
    WXLUA_IMPLEMENT_BIND_WXBASE 
    WXLUA_IMPLEMENT_BIND_WXCORE 

    wxLuaBinding_wxart2d_init();

    // When this function returns wxApp:MainLoop() will be called by wxWidgets
    // and so we want the Lua code wx.wxGetApp:MainLoop() to not
    // prematurely start it.
    wxLuaState::sm_wxAppMainLoop_will_run = true;


    m_cmdProc = new a2dLuaCentralCommandProcessor();
    a2dDocviewGlobals->SetDocviewCommandProcessor( m_cmdProc );

    a2dCanvasGlobals->GetHabitat()->SetFill( *a2dNullFILL );
    a2dCanvasGlobals->GetHabitat()->SetStroke( *a2dNullSTROKE );
    //m_cmdProc->SetEditAtEndTools( false );
    a2dCanvasGlobals->SetPopBeforePush( false );
    a2dCanvasGlobals->SetOneShotTools( false );
    wxLuaState& m_wxlState = m_cmdProc->GetLuaState();

    if ( !m_wxlState.Ok() )
        return false;


    wxCmdLineParser parser( cmdLineDesc, argc, argv );
    parser.SetLogo( wxString::Format( wxT( "Welcome to %s\n" )
                                      wxT( "The editor is opened if no parameters are given.\n" )
                                      wxT( "Note: specify the parameters in the order shown and -d, -o, -r are exclusive.\n" )
                                      wxT( "To stop this program from processing command line args use '--'.\n" ),
                                      wxLUA_VERSION_STRING ) );

    TemplatesInit();


    switch ( parser.Parse() )
    {
        case -1 :
        {
            // Help should be given by the wxCmdLineParser, exit program
            return false;
        }
        case 0:
        {
            // Don't quit the program even if no top level windows open
            if ( parser.Found( wxT( "q" ) ) )
            {
                arg_count++; // remove -q arg
                dont_quit = true;
            }

            if ( parser.Found( wxT( "s" ) ) )
            {
                arg_count++; // remove -s arg
                m_print_stdout = false;
            }

            if ( parser.Found( wxT( "m" ) ) )
            {
                arg_count++; // remove -m arg
                m_print_msgdlg = true;
            }

            // Check if we are to display the console
            if ( parser.Found( wxT( "c" ) ) )
            {
                arg_count++; // remove -c arg
                m_want_console = true;
                // Note: The wxLuaConsoleWrapper::m_luaConsole will be nulled when it closes in
                //       wxLuaConsole::OnCloseWindow.
                m_luaConsoleWrapper.SetConsole( new wxLuaConsole( &m_luaConsoleWrapper, NULL, ID_WXLUA_CONSOLE ) );
                m_luaConsoleWrapper.GetConsole()->Show( true );
            }

            // Check if we are to run some Lua code stat
            wxString luaCode;
            if ( parser.Found( wxT( "e" ), &luaCode ) )
            {
                arg_count++; // remove -e arg
                int rc = m_wxlState.RunString( luaCode, wxT( "=(command line)" ) );
                run_ok = ( rc == 0 );
                if ( rc != 0 )
                {
                    DisplayMessage( wxlua_LUA_ERR_msg( rc ), true, m_wxlState );
                    break;
                }
            }

            // Set up for debugging
            wxString debugString;
            if ( parser.Found( wxT( "d" ), &debugString ) )
            {
                // Note: wxLuaDebuggerServer::StartClient() runs
                //       wxExecute(m_programName -d[host]:[port], ...)

                wxString serverName( debugString.BeforeFirst( wxT( ':' ) ) );

                if ( serverName.IsEmpty() )
                {
                    DisplayMessage( _( "The wxLua debugger server host name is empty : wxLua -d[host]:[port]\n" ), true );
                }
                else
                {
                    long portNumber = 0;
                    if ( debugString.AfterFirst( wxT( ':' ) ).ToLong( &portNumber ) )
                    {
                        m_wxlDebugTarget = new wxLuaDebugTarget( m_wxlState, serverName, ( int )portNumber );
                        if ( m_wxlDebugTarget != NULL )
                        {
                            bool ok = m_wxlDebugTarget->Run();
                            //wxSafeShowMessage(wxT("Target"), wxString::Format(wxT("PID %ld TIME %s ok %d"), (long)wxGetProcessId(), wxT(__TIME__), int(ok)));
                            return ok;
                        }
                        else
                            DisplayMessage( _( "The wxLua debug target cannot start.\n" ), true );
                    }
                    else
                        DisplayMessage( _( "The wxLua debugger server port is not a number : wxLua -d[host]:[port]\n" ), true );
                }

                return ( m_luaConsoleWrapper.Ok() ); // will exit app when console is closed
            }


            // See if a file has been specified to open or just start editor
            // if there are no parameters.
            wxString openFileName;
            bool open_file = parser.Found( wxT( "o" ), &openFileName );
            if ( open_file ) arg_count++; // remove -o arg


            //int aap = m_wxlState.RunFile( "c:/data/art2d/wxArt2D/apps/wxluaart2d/src/editor.wx.lua" );
            //int aap = m_wxlState.RunFile( "c:/data/art2d/wxArt2D/apps/wxluaart2d/src/minimal.wx.lua" );
            //int aap = m_wxlState.RunFile( "c:/data/art2d/wxArt2D/apps/wxluaart2d/src/geditor.wx.lua" );

            if ( open_file || ( parser.GetParamCount() == 0 ) )
            {
                if ( openFileName.IsEmpty() && ( parser.GetParamCount() > 0u ) )
                    openFileName = parser.GetParam( 0 );
                if ( !openFileName.IsEmpty() )
                    arg_count = FindFirstCmdParam( arg_count, openFileName, argc, argv );

                wxlua_pushargs( m_wxlState.GetLuaState(), argv, argc, arg_count );

                int rc = m_wxlState.RunBuffer( ( const char* ) editor_wx_lua, editor_wx_lua_len, m_programName );
                run_ok = ( rc == 0 );
                if ( rc != 0 )
                {
                    DisplayMessage( wxlua_LUA_ERR_msg( rc ), true, m_wxlState );
                }
                break;
            }

            // at least one parameter was specified, it must be a program to run.

            // See if we are to run the specified file with command-line arguments.
            // This is the default action if r is not specified.
            //if (parser.Found(wxT("r"), &value))
            {
                wxString runFileName;
                if ( !parser.Found( wxT( "r" ), &runFileName ) && ( parser.GetParamCount() > 0u ) )
                    runFileName = parser.GetParam( 0 );
                if ( !runFileName.IsEmpty() )
                    arg_count = FindFirstCmdParam( arg_count, runFileName, argc, argv );

                arg_count++; // skip the program we're running
                wxlua_pushargs( m_wxlState.GetLuaState(), argv, argc, arg_count );

                int rc = m_wxlState.RunFile( runFileName );
                run_ok = ( rc == 0 );
                if ( rc != 0 )
                {
                    DisplayMessage( wxlua_LUA_ERR_msg( rc ), true, m_wxlState );
                }

                break;
            }

            break;
        }
        default:
        {
            wxLogMessage( wxT( "Unknown command line option, aborting." ) );
            return false;
        }
    }

    // check to see if there is a toplevel window open for the user to close, else exit
    if ( run_ok && !dont_quit )
    {
        if ( GetTopWindow() == NULL )
            run_ok = false;
    }

    return m_luaConsoleWrapper.Ok() || run_ok;
}

int wxLuaStandaloneApp::OnExit()
{
    // If acting as a debuggee, we're done - disconnect from the debugger.
    if ( m_wxlDebugTarget != NULL )
    {
        m_wxlDebugTarget->Stop();
        delete m_wxlDebugTarget;
        m_wxlDebugTarget = NULL;
    }

    m_want_console = false; // no more messages

    if ( m_luaConsoleWrapper.Ok() && !m_luaConsoleWrapper.GetConsole()->IsBeingDeleted() )
    {
        m_luaConsoleWrapper.GetConsole()->Destroy();
        m_luaConsoleWrapper.SetConsole( NULL );
    }

    if ( m_mem_bitmap_added )
        wxMemoryFSHandler::RemoveFile( wxT( "wxLua" ) );

    wxSafeYield();  // make sure windows get destroyed

    int ret = wxApp::OnExit();

#if defined(__WXMSW__) && wxCHECK_VERSION(2, 3, 3)
    WSACleanup ();
#endif // defined(__WXMSW__) && wxCHECK_VERSION(2, 3, 3)

    return ret;
}

void wxLuaStandaloneApp::DisplayMessage( const wxString& msg, bool is_error,
        const wxLuaState& wxlState )
{
    // If they closed the console, but specified they wanted it
    // on the command-line, recreate it.
    if ( m_want_console && ( !m_luaConsoleWrapper.Ok() ) )
    {
        m_luaConsoleWrapper.SetConsole( new wxLuaConsole( &m_luaConsoleWrapper, NULL, ID_WXLUA_CONSOLE ) );
        m_luaConsoleWrapper.GetConsole()->Show( true );
    }

    if ( !is_error )
    {
        //fprintf(stderr, wx2lua(msg + wxT("\n")));

        if ( m_print_stdout )
            wxPrintf( wxT( "%s\n" ), msg.c_str() );

        if ( m_luaConsoleWrapper.Ok() )
            m_luaConsoleWrapper.GetConsole()->DisplayText( msg );

        if ( m_print_msgdlg )
            wxMessageBox( msg, wxT( "wxLua Print" ) );
    }
    else
    {
        //if (m_print_stdout) // always print errors, FIXME: to stderr or is stdout ok?
        wxPrintf( wxT( "%s\n" ), msg.c_str() );

        if ( m_luaConsoleWrapper.Ok() )
        {
            m_luaConsoleWrapper.GetConsole()->DisplayText( msg );
            m_luaConsoleWrapper.GetConsole()->SetExitOnError( is_error );
            if ( wxlState.Ok() )
                m_luaConsoleWrapper.GetConsole()->DisplayStack( wxlState );
        }

        if ( m_wxlDebugTarget != NULL )
            m_wxlDebugTarget->DisplayError( msg );

        if ( m_print_msgdlg )
            wxMessageBox( msg, wxT( "wxLua Error" ) );
    }
}

void wxLuaStandaloneApp::OnLua( wxLuaEvent& event )
{
    DisplayMessage( event.GetString(), event.GetEventType() == wxEVT_LUA_ERROR,
                    event.GetwxLuaState() );
}

void wxLuaStandaloneApp::TemplatesInit()
{
    a2dCanvasGlobals->m_initialDocChildFrameSize = wxSize( 800, 800 );

    wxInitAllImageHandlers();

    //SetUseBestVisual(true);
    a2dGlobals->GetFontPathList().Add( wxT( "../common/fonts" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "../common/images" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./" ) );

#if wxUSE_LIBPNG
    wxImage::AddHandler( new wxPNGHandler );
#endif

    m_singleconnector = new a2dConnector();

    a2dIOHandlerCVGIn* cvghin = new a2dIOHandlerCVGIn();
    a2dIOHandlerCVGOut* cvghout = new a2dIOHandlerCVGOut();

#if wxART2D_USE_SVGIO
    a2dIOHandlerSVGIn* svghin = new a2dIOHandlerSVGIn();
    a2dIOHandlerSVGOut* svghout = new a2dIOHandlerSVGOut();
#endif //wxART2D_USE_SVGIO
#if wxART2D_USE_GDSIO
    a2dIOHandlerGDSIn* gdshin = new a2dIOHandlerGDSIn();
    a2dIOHandlerGDSOut* gdshout = new a2dIOHandlerGDSOut();
#endif //wxART2D_USE_GDSIO

    a2dDocumentTemplate* doctemplatenew;

    doctemplatenew = new a2dDocumentTemplateAuto( _T( "All Files" ) , _T( "*.*" ) , _T( "" ) , _T( "" ) , _T( "a2dCanvasDocument" ) ,
            CLASSINFO( a2dCanvasDocument ), m_singleconnector, a2dTemplateFlag::DEFAULT );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );

    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate( wxT( "CVG Drawing" ), wxT( "*.cvg" ), wxT( "" ), wxT( "cvg" ), wxT( "a2dCanvasDocument" ),
            CLASSINFO( a2dCanvasDocument ), m_singleconnector, a2dTemplateFlag::DEFAULT, cvghin, cvghout );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );

#if wxART2D_USE_GDSIO
    doctemplatenew = new a2dDocumentTemplate( wxT( "GDS-II Drawing" ), wxT( "*.cal" ), wxT( "" ), wxT( "cal" ), wxT( "a2dCanvasDocument" ),
            CLASSINFO( a2dCanvasDocument ), m_singleconnector, a2dTemplateFlag::DEFAULT, gdshin, gdshout );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_GDSIO

#if wxART2D_USE_SVGIO
    doctemplatenew = new a2dDocumentTemplate( wxT( "SVG Drawing" ), wxT( "*.svg" ), wxT( "" ), wxT( "svg" ), wxT( "a2dCanvasDocument" ),
            CLASSINFO( a2dCanvasDocument ), m_singleconnector, a2dTemplateFlag::DEFAULT, svghin, svghout );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_SVGIO

    a2dViewTemplate* viewtemplatenew;
    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate( wxT( "CVG Drawing" ), wxT( "a2dCanvasDocument" ), wxT( "Drawing View" ),
                                           CLASSINFO( a2dCanvasView ), m_singleconnector, a2dTemplateFlag::DEFAULT, a2dCanvasGlobals->m_initialDocChildFrameSize );
    m_cmdProc->AssociateViewTemplate( viewtemplatenew );

}