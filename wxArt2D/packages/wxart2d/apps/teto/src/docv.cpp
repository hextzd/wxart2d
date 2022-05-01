/////////////////////////////////////////////////////////////////////////////
// Name:        docview.cpp
// Purpose:     Document/view demo
// Author:      Klaas Holwerda
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include "wxart2d.h"

#if wxART2D_USE_GDSIO
#include "wx/gdsio/gdsio.h"
#endif //wxART2D_USE_GDSIO
#if wxART2D_USE_KEYIO
#include "wx/keyio/keyio.h"
#endif //wxART2D_USE_KEYIO

#if wxART2D_USE_SVGIO
#include "wx/svgio/parssvg.h"
#endif //wxART2D_USE_SVGIO

#include "docv.h"
#include "view.h"

#include "wx/editor/recurdoc.h"

#include "docv.h"

#include <wx/tokenzr.h>
#include <wx/regex.h>
#include "wx/cmdline.h"
#include <wx/filename.h>

#include "tetowrap.h"


#if wxART2D_USE_LUA
#include "wx/luawraps/luawrap.h"
#include "wx/luawraps/luabind.h"
#endif

//wxart2d_Binding bor;


IMPLEMENT_DYNAMIC_CLASS( TetoDocument, a2dCanvasDocument )

TetoDocument::TetoDocument( void )
{
    m_documentTypeName = wxT( "TetoDocument" );
}

TetoDocument::~TetoDocument( void )
{
}

void TetoDocument::CreateCommandProcessor()
{
    a2dCanvasGlobals->GetHabitat()->SetFill( *a2dNullFILL );
    a2dCanvasGlobals->GetHabitat()->SetStroke( *a2dNullSTROKE );
}

//------------------------------------------------------------------------
// Vdraw its application object
//------------------------------------------------------------------------


IMPLEMENT_APP( MyApp )

BEGIN_EVENT_TABLE( MyApp, wxApp )
    EVT_LUA_PRINT       ( wxID_ANY, MyApp::OnLua )
    EVT_LUA_ERROR       ( wxID_ANY, MyApp::OnLua )
    EVT_LUA_DEBUG_HOOK  ( wxID_ANY, MyApp::OnLua )
END_EVENT_TABLE()

MyApp::MyApp( void )
{
    m_quiet = false;
    m_verbose = true;
    m_exit = false;
}

void MyApp::OnLua( wxLuaEvent& event )
{
    if ( event.GetEventType() == wxEVT_LUA_PRINT )
    {
        wxMessageBox( event.GetString(), wxT( "teto" ) );
    }
    else if ( event.GetEventType() == wxEVT_LUA_ERROR )
    {
        wxMessageBox( event.GetString(), wxT( "teto" ) );
    }
}



#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

#ifdef _DEBUG
// needed for debugging. Delete it if it hurts
#define _CRTDBG_ALLOC_MEM_DF        0x01  /* Turn on debug allocation */
#define _CRTDBG_DELAY_FREE_MEM_DF   0x02  /* Don't actually free memory */
#define _CRTDBG_CHECK_ALWAYS_DF     0x04  /* Check heap every alloc/dealloc */
#define _CRTDBG_RESERVED_DF         0x08  /* Reserved - do not use */
#define _CRTDBG_CHECK_CRT_DF        0x10  /* Leak check/diff CRT blocks */
#define _CRTDBG_LEAK_CHECK_DF       0x20  /* Leak check at program exit */
extern "C" {
    _CRTIMP int __cdecl _CrtSetDbgFlag( int );
}
#endif

#include "wxbind/include/wxbinddefs.h"
WXLUA_DECLARE_BIND_STD

bool MyApp::OnInit( void )
{
    wxString artroot = a2dGeneralGlobals->GetWxArt2DVar();
    if ( artroot.IsEmpty() )
        return false;

    wxInitAllImageHandlers();
    a2dMenuIdItem::InitializeBitmaps();

    // Initialize the wxLua bindings we want to use.
    // See notes for WXLUA_DECLARE_BIND_STD above.
    WXLUA_IMPLEMENT_BIND_WXLUA 
    WXLUA_IMPLEMENT_BIND_WXBASE 
    WXLUA_IMPLEMENT_BIND_WXCORE 

    wxLuaBinding_wxart2d_init();
    wxLuaBinding_teto_init();

    //wxString fname = a2dGlobals->GetFontPathList().FindValidPath( "nen.chr" );

    a2dGlobals->GetFontPathList().Add( artroot + wxT( "apps/teto/config" ) );
    a2dGlobals->GetImagePathList().Add( artroot +  wxT( "apps/teto/config" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "art/resources" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "apps/teto/config" ) );
    a2dCanvasGlobals->GetLayersPathList().Add( artroot +  wxT( "apps/teto/config" ) );

#if wxUSE_DEBUG_CONTEXT
    wxDebugContext::SetCheckpoint();
#endif

    a2dGlobals->SetPrimitiveThreshold( 3 );
    if ( !a2dCanvasGlobals->GetHabitat()->LoadLayers( wxT( "tetolayers.cvg" ) ) )
        return false;

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

#if wxART2D_USE_KEYIO
    a2dIOHandlerKeyIn* keyhin = new a2dIOHandlerKeyIn();
    a2dIOHandlerKeyOut* keyhout = new a2dIOHandlerKeyOut();
#endif //wxART2D_USE_KEYIO


    //first the base command processor for maintaining documents and views

    // wxLua specific commands strings translated to calls to a2dCentralCanvasCommandProcessor
    m_cmdProc = new TetoLuaProcCommandProcessor();
    a2dCanvasGlobals->GetHabitat()->SetFill( *a2dNullFILL );
    a2dCanvasGlobals->GetHabitat()->SetStroke( *a2dNullSTROKE );

    a2dDocviewGlobals->SetDocviewCommandProcessor( m_cmdProc );


    m_connector = new wxDrawingConnector( CLASSINFO( a2dCanvasTetoEditorFrame ) );
    m_connector->SetInitialPosition( wxPoint( 50, 40 ) );
    m_connector->SetInitialSize( wxSize( 200, 300 ) );

    //doc tamplates -----------------------

    a2dDocumentTemplate* doctemplatenew;

    doctemplatenew = new a2dDocumentTemplateAuto( _T( "All Files" ) , _T( "*.*" ) , _T( "" ) , _T( "" ) , _T( "TetoDocument" ) ,
            CLASSINFO( TetoDocument ), m_connector, a2dTemplateFlag::DEFAULT );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );

#if wxART2D_USE_GDSIO
    doctemplatenew = new a2dDocumentTemplate( wxT( "GDS-II Drawing" ), wxT( "*.cal" ), wxT( "" ), wxT( "cal" ), wxT( "TetoDocument" ),
            CLASSINFO( TetoDocument ), m_connector, a2dTemplateFlag::DEFAULT, gdshin, gdshout );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_GDSIO

    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate( wxT( "CVG Drawing" ), wxT( "*.cvg" ), wxT( "" ), wxT( "cvg" ), wxT( "TetoDocument" ),
            CLASSINFO( TetoDocument ), m_connector, a2dTemplateFlag::DEFAULT,  cvghin, cvghout );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );

#if wxART2D_USE_SVGIO
    doctemplatenew = new a2dDocumentTemplate( wxT( "SVG Drawing" ), wxT( "*.svg" ), wxT( "" ), wxT( "svg" ), wxT( "TetoDocument" ),
            CLASSINFO( TetoDocument ), m_connector, a2dTemplateFlag::DEFAULT, svghin, svghout );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_SVGIO

#if wxART2D_USE_KEYIO
    doctemplatenew = new a2dDocumentTemplate( wxT( "Key Drawing" ), wxT( "*.key" ), wxT( "" ), wxT( "key" ), wxT( "TetoDocument" ),
            CLASSINFO( TetoDocument ), m_connector, a2dTemplateFlag::DEFAULT, keyhin, keyhout );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_KEYIO

    //View templates ------------------------

    a2dViewTemplate* viewtemplatenew;
    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate( wxT( "CVG Drawing" ), wxT( "TetoDocument" ), wxT( "Drawing View Frame" ),
                                           CLASSINFO( a2dCanvasView ), m_connector, a2dTemplateFlag::DEFAULT, wxSize( 100, 400 )  );
    m_connector->AssociateViewTemplate( viewtemplatenew );
    m_cmdProc->AssociateViewTemplate( viewtemplatenew );

    m_connector2 = new wxTextConnector();
    m_connector2->SetInitialPosition( wxPoint( 20, 210 ) );
    m_connector2->SetInitialSize( wxSize( 300, 200 ) );

    // Create a template relating drawing documents to text views
    viewtemplatenew =  new a2dViewTemplate( wxT( "Text" ), wxT( "TetoDocument" ), wxT( "Text View Frame" ),
                                            CLASSINFO( TextView ), m_connector2, a2dTemplateFlag::DEFAULT, wxSize( 200, 300 ) );

    m_connector->AssociateViewTemplate( viewtemplatenew );
    m_cmdProc->AssociateViewTemplate( viewtemplatenew );

    //Create the main frame window
    //the parent frame will get event from the child frames.
    VdrawFrame* frame = new VdrawFrame( ( wxFrame* ) NULL, -1, wxT( "DocView Demo" ), wxPoint( 0, 0 ), wxSize( 500, 400 ), wxDEFAULT_FRAME_STYLE );
    SetTopWindow( frame );
    m_connector->Init( frame, true );

    ParseCmdLine( argc, argv );

    if ( !m_quiet )
        frame->Show( TRUE );

    return true;
}

int MyApp::OnRun()
{
    if ( !m_inputFile.IsEmpty() )
    {
        wxFileName infile( m_inputDir, m_inputFile );
        m_cmdProc->FileOpen( infile.GetFullPath() );
    }

    //m_processFile = _T("../process/2files.lua");

    if ( !m_processFile.IsEmpty() && ::wxFileExists( m_processFile ) )
    {
        m_cmdProc->ExecuteFile( m_processFile );
    }

    //if command Exit does exit the application, the topwindow is closed, and deleted in idle time,
    //leading to stopping the event loop.

    //start the event loop ( e.g. for logging to show up. ), Even is top windoe is not visible, or closed already.
    int ret = wxAppBase::OnRun();

    if ( m_exit )
    {
    }
    return ret;
}

int MyApp::OnExit( void )
{
    m_connector->DisassociateViewTemplates();
    m_connector2->DisassociateViewTemplates();

#if wxUSE_DEBUG_CONTEXT
//    wxDebugContext::Dump();
//    wxDebugContext::PrintStatistics();
#endif

    return 0;
}

int MyApp::ParseCmdLine( int argc,  wxChar** argv )
{
    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        {
            wxCMD_LINE_SWITCH, "h", "help", "show this help message",
            wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP
        },
        { wxCMD_LINE_SWITCH, "v", "verbose" , "be verbose" },
        { wxCMD_LINE_SWITCH, "q", "quiet",  "be quiet" },

        { wxCMD_LINE_OPTION, "i", "input", "input dir" },
        { wxCMD_LINE_OPTION, "e", "execute", "execute wxLua code in file" },

        {
            wxCMD_LINE_PARAM,  NULL, NULL, "input file",
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL
        },

        { wxCMD_LINE_NONE }
    };

    wxCmdLineParser parser( cmdLineDesc, argc, argv );

    switch ( parser.Parse() )
    {
        case -1:
            wxLogMessage( wxT( "Help was given, terminating." ) );
            return -1;
            break;

        case 0:
        {
            parser.Found( wxT( "i" ), &m_inputDir );
            parser.Found( wxT( "e" ), &m_processFile );

            if ( parser.Found( wxT( "q" ) ) ) m_quiet = true;
            if ( parser.Found( wxT( "v" ) ) ) m_verbose = true;

            if ( parser.GetParamCount() > 0 )
                m_inputFile = parser.GetParam( 0 );

            return 0;
            break;
        }
        default:
            wxLogMessage( wxT( "Syntax error detected, aborting." ) );
            return -1;
            break;
    }
}

//------------------------------------------------------------------------
// Overview canvas in toplevel frame for showing all open drawings "symbolic"
//------------------------------------------------------------------------

BEGIN_EVENT_TABLE( OverviewCanvas, a2dCanvas )
    EVT_MOUSE_EVENTS( OverviewCanvas::OnMouseEvent )
    EVT_ADD_DOCUMENT( OverviewCanvas::OnAddDocument )
    EVT_REMOVE_DOCUMENT( OverviewCanvas::OnRemoveDocument )
END_EVENT_TABLE()

// Define a constructor for my canvas
OverviewCanvas::OverviewCanvas( wxFrame* frame, const wxPoint& pos, const wxSize& size, const long style ):
    a2dCanvas( frame, -1, pos, size, style )
{
    m_contr = new a2dStToolContr( GetDrawingPart(), frame );
    m_contr->SetZoomFirst( true );
	a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent(wxEVT_ADD_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_REMOVE_DOCUMENT, this );
}

OverviewCanvas::~OverviewCanvas()
{
    //a2dDocviewGlobals->GetDocviewCommandProcessor()->Unregister( this );
}

//Open clicked root in a MyCanvas
void OverviewCanvas::OnMouseEvent( wxMouseEvent& event )
{
    //Raise();
    if ( !GetDrawingPart() || !GetDrawingPart()->GetMouseEvents() )
    {
        event.Skip();
        return;
    }

    if ( event.LeftDown() )
    {
        int x = event.GetX();
        int y = event.GetY();
        //to world coordinates to do hit test in world coordinates
        double xw = GetDrawer2D()->DeviceToWorldX( x );
        double yw = GetDrawer2D()->DeviceToWorldY( y );

        a2dIterC ic( GetDrawingPart());
        a2dIterCU cu( ic );
        a2dHitEvent hitevent = a2dHitEvent( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
        a2dCanvasObject* obj = GetShowObject()->IsHitWorld( ic, hitevent );

        if ( obj && ( 0 != wxDynamicCast( obj, a2dCanvasDocumentReference ) ) )
        {
            a2dCanvasDocument* doc = wxDynamicCast( obj, a2dCanvasDocumentReference )->GetDocumentReference();
            a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateView( doc );
        }
    }
}

void OverviewCanvas::OnAddDocument( a2dCommandEvent& event )
{
    a2dCanvasDocument* doc = ( a2dCanvasDocument* ) event.GetDocument();

    static double i = 200;

    a2dCanvasDocumentReference* tr = new a2dCanvasDocumentReference( 120, i, doc );
    tr->SetStroke( wxColour( 229, 5, 64 ), 0 );
    tr->SetFill( *a2dTRANSPARENT_FILL );
    GetDrawingPart()->GetDrawing()->GetRootObject()->Append( tr );
    GetDrawingPart()->Update( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_ALL ); //the above object gets drawn and included in bbox
    i = i + 200;

    m_contr->Zoomout();
}

void OverviewCanvas::OnRemoveDocument( a2dCommandEvent& event )
{
    a2dCanvasDocument* doc = ( a2dCanvasDocument* ) event.GetDocument();
    forEachIn( a2dCanvasObjectList, GetDrawingPart()->GetDrawing()->GetRootObject()->GetChildObjectList() )
    {
        a2dCanvasObject* obj = *iter;

        if ( 0 != wxDynamicCast( obj, a2dCanvasDocumentReference ) )
        {
            a2dCanvasDocumentReference* openfile = wxDynamicCast( obj, a2dCanvasDocumentReference );
            if ( openfile->GetDocumentReference() == doc )
            {
                GetDrawingPart()->GetDrawing()->GetRootObject()->ReleaseChild( obj );
                break;
            }
        }
    }
    GetDrawingPart()->Update( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_ALL );
} 

//------------------------------------------------------------------------
// Special command processor, for teto lua commands
//------------------------------------------------------------------------
IMPLEMENT_CLASS( TetoLuaProcCommandProcessor, a2dLuaCentralCommandProcessor )

TetoLuaProcCommandProcessor* a2dGetGedi() { return ( ( MyApp* )&wxGetApp() )->GetLuaCommandProc(); }

TetoLuaProcCommandProcessor::TetoLuaProcCommandProcessor( long flags , bool initialize, int maxCommands )
    : a2dLuaCentralCommandProcessor( flags, initialize, maxCommands )
{
    m_tooldlgteto = NULL;

    //RunBuffer(plotto_lua, plotto_lua_len-1, wxT("plotto.lua"));
}

TetoLuaProcCommandProcessor::~TetoLuaProcCommandProcessor()
{
    if ( m_tooldlgteto )
        m_tooldlgteto->Destroy();

    m_tooldlgteto = NULL;

}

void TetoLuaProcCommandProcessor::DeleteDlgs()
{
    if ( m_tooldlgteto )
        delete m_tooldlgteto;

    m_tooldlgteto = NULL;
}

bool TetoLuaProcCommandProcessor::ShowDlg( const a2dCommandId* comID, bool modal, bool onTop )
{
    if ( comID == &a2dCanvasCommandProcessor::COMID_ShowDlgTools )
    {
        if ( !m_tooldlgteto )
            m_tooldlgteto = new ToolDlg( NULL );

        if ( m_tooldlgteto->IsShown() )
        {
            m_tooldlgteto->Show( false );
        }
        else
        {
            m_tooldlgteto->Show( true );
        }
        return true;
    }
    else
    {
        if ( a2dLuaCentralCommandProcessor::ShowDlg( comID, modal, onTop ) )
            return true;
    }

    return false;
}

void TetoLuaProcCommandProcessor::OnExit()
{
    wxConfigBase* pConfig = wxConfigBase::Get();
    a2dGetCmdh()->FileHistorySave( *pConfig );

    DeleteDlgs();

    a2dLuaCentralCommandProcessor::OnExit();

    wxFrame* pf = ( wxFrame* ) wxGetApp().GetTopWindow();
    pf->Close( true );
    wxGetApp().SetExit();
}


//------------------------------------------------------------------------
// This is the top-level parent frame of the application.
//------------------------------------------------------------------------

IMPLEMENT_CLASS( VdrawFrame, a2dDocumentFrame )

BEGIN_EVENT_TABLE( VdrawFrame, a2dDocumentFrame )
    EVT_MENU( MSCREEN_T, VdrawFrame::FillData )
    EVT_MENU( CANVAS_ABOUT, VdrawFrame::OnAbout )
    EVT_MOUSE_EVENTS( VdrawFrame::OnMouseEvent )
    EVT_CLOSE( VdrawFrame::OnCloseWindow )
END_EVENT_TABLE()

VdrawFrame::VdrawFrame( wxFrame* frame, wxWindowID id, const wxString& title,
                        const wxPoint& pos, const wxSize& size, const long type ):
    a2dDocumentFrame( true, frame, NULL, id, title, pos, size, type )
{
    m_tetoproc = ( TetoLuaProcCommandProcessor* ) a2dLuaWP;
    a2dCanvasGlobals->GetHabitat()->SetFill( *a2dNullFILL );
    a2dCanvasGlobals->GetHabitat()->SetStroke( *a2dNullSTROKE );

    //// Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
    SetIcon( wxIcon( wxString( wxT( "doc_icn" ) ) ) );
#endif

    //// Make a menubar
    wxMenu* file_menu = new wxMenu;

    AddCmdMenu( file_menu, CmdMenu_FileNew() );
    AddCmdMenu( file_menu, CmdMenu_FileOpen() );
    file_menu->Append( wxID_EXIT, _( "E&xit" ) );

    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( file_menu );

    wxMenu* dlg_menu = new wxMenu;

    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgPathSettings() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgSettings() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgTrans() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgTools() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgStyle() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgSnap() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgStructure() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgIdentify() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgMeasure() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgCoordEntry() );

    //AddCmdMenu( dlg_menu, CmdMenu_InsertGroupRef() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgLayersDocument() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgGroups() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgPropEdit() );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( CANVAS_ABOUT, _( "&About" ) );

    wxMenuBar* menu_bar = new wxMenuBar;

    menu_bar->Append( file_menu, _( "&File" ) );

    wxMenu* drawMenu = new wxMenu;
    drawMenu->Append( MSCREEN_T, wxT( "test screen" ), wxT( "whatever" ) );

    menu_bar->Append( drawMenu, _( "&Screens" ) );

    menu_bar->Append( dlg_menu, _( "&Dialogs" ) );

    menu_bar->Append( help_menu, _( "&Help" ) );

    //// Associate the menu bar with the frame
    SetMenuBar( menu_bar );

    Centre( wxBOTH );
    Show( true );

    CreateStatusBar( 2 );

    int width, height;
    GetClientSize( &width, &height );

    m_topcanvas = new OverviewCanvas( this, wxPoint( 0, 0 ), size, 0 );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_topcanvas->SetMappingUpp( 0, 0, 4, 4 );
    m_topcanvas->SetYaxis( true );
    //defines the whole area in world coordinates that can be seen by scrolling
    m_topcanvas->SetScrollMaximum( 0, 0, 1600, 1600 );
    //defines the number of world units scrolled when line up or down events in X
    m_topcanvas->SetScrollStepX( 200 );
    //defines the number of world units scrolled when line up or down events in Y
    m_topcanvas->SetScrollStepY( 100 );

    a2dFill fill = a2dFill( wxColour( 2, 255, 255 ) );
    m_topcanvas->SetBackgroundFill( fill );

    a2dRect* tr = new a2dRect( 0, 0, 100, 100 );
    tr->SetStroke( wxColour( 229, 5, 64 ), 0 );
    tr->SetFill( wxColour( 0, 117, 245 ) );
    m_topcanvas->GetDrawing()->GetRootObject()->Append( tr );

	m_topcanvas->GetDrawingPart()->Update();

    editMenu = ( wxMenu* ) NULL;
}

VdrawFrame::~VdrawFrame()
{
    //undo stack can hold Owners to Documents etc.
    m_document->GetCommandProcessor()->ClearCommands();

    //to prevent (asking for) saving this root
    m_document->Modify( false );
    m_document->Close( true ); //also closses the view but they should be closed by now.
}

void VdrawFrame::OnCloseWindow( wxCloseEvent& event )
{
    if ( event.CanVeto() )
    {
        if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear( !event.CanVeto() ) )
        {
            m_tetoproc->DeleteDlgs();
            Destroy();
        }
        else
            event.Veto( true );
    }
    else
    {
        Destroy();
        a2dDocviewGlobals->GetDocviewCommandProcessor()->Exit( true );
    }
}

void VdrawFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( " Teto \nAuthor: Klaas Holwerda" ), wxT( "About Teto" ) );
}

void VdrawFrame::FillData( wxCommandEvent& event )
{
    a2dSmrtPtr<TetoDocument> doc = new TetoDocument();
    doc->SetDocumentTypeName( wxT( "TetoDocument" ) );
    a2dView* createdview;

    a2dCanvasObject* top = new a2dCanvasObject();
    top->SetName( wxT( "topdrawing" ) );
    doc->GetDrawing()->GetRootObject()->Append( top );

    a2dRect* tr = new a2dRect( -500, 800, 300, 200 );
    tr->SetFill( *a2dNullFILL );
    tr->SetStroke( *a2dNullSTROKE );
    tr->SkewX( -40 );
    tr->Scale( 2, 3.2 );
    tr->SetRotation( 20 );
    top->Append( tr );

    a2dSLine* ll = new a2dSLine( -500, 0, 400, -100 );
    ll->SetStroke( wxColour( 252, 0, 252 ), 0.0 );
    top->Append( ll );

    m_topgroup = top;;


    doc->SetTitle( wxT( "screen all" ) );

    createdview = a2dDocviewGlobals->GetDocviewCommandProcessor()->AddDocumentCreateView( doc );//, "Drawing View" );

    if ( createdview )
    {
        doc->GetDrawing()->SetRootRecursive();

        if ( 0 != wxDynamicCast( createdview, a2dCanvasView ) )
        {
            a2dCanvasView* view = ( a2dCanvasView* ) createdview;
            view->GetDrawingPart()->SetShowObject( m_topgroup );
        }
        else if ( 0 != wxDynamicCast( createdview, TextView ) )
        {
            TextView* view = ( TextView* ) createdview;
            view->SetShowObject( m_topgroup );
        }
        createdview->Activate( true );
        createdview->Update();
    }
    else
        a2dDocviewGlobals->GetDocviewCommandProcessor()->RemoveDocument( doc );
}

void VdrawFrame::OnMouseEvent( wxMouseEvent& event )
{
}
