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
#include "viewcon.h"

#include "wx/editor/recurdoc.h"
#include "wx/editor/xmlparsdoc.h"

#include "docv.h"

#include <wx/tokenzr.h>
#include <wx/regex.h>
#include "wx/cmdline.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>



#if wxART2D_USE_LUA
#include "thumbowrap.h"
#include "wx/luawraps/luawrap.h"
#include "wx/luawraps/luabind.h"
#endif

//wxart2d_Binding bor;


IMPLEMENT_DYNAMIC_CLASS( ThumboDocument, a2dCanvasDocument )

ThumboDocument::ThumboDocument( void )
{
    m_documentTypeName = wxT( "ThumboDocument" );
    //?? m_drawing->SetRootObject( new a2dCameleon() );
}

ThumboDocument::~ThumboDocument( void )
{
}

void ThumboDocument::CreateCommandProcessor()
{
    a2dCanvasGlobals->GetHabitat()->SetFill( *a2dNullFILL );
    a2dCanvasGlobals->GetHabitat()->SetStroke( *a2dNullSTROKE );
}

//------------------------------------------------------------------------
// Vdraw its application object
//------------------------------------------------------------------------


IMPLEMENT_APP( MyApp )

BEGIN_EVENT_TABLE( MyApp, wxApp )
#if (wxART2D_USE_LUA == 1)
    EVT_LUA_PRINT       ( wxID_ANY, MyApp::OnLua )
    EVT_LUA_ERROR       ( wxID_ANY, MyApp::OnLua )
    EVT_LUA_DEBUG_HOOK  ( wxID_ANY, MyApp::OnLua )
#endif
END_EVENT_TABLE()

MyApp::MyApp( void )
{
    //_CrtSetBreakAlloc(556499);

    m_quiet = false;
    m_verbose = true;
    m_exit = false;
}

#if (wxART2D_USE_LUA == 1)
void MyApp::OnLua( wxLuaEvent& event )
{
    if ( event.GetEventType() == wxEVT_LUA_PRINT )
    {
        wxMessageBox( event.GetString(), wxT( "thumbo" ) );
    }
    else if ( event.GetEventType() == wxEVT_LUA_ERROR )
    {
        wxMessageBox( event.GetString(), wxT( "thumbo" ) );
    }
}

#include "wxbind/include/wxbinddefs.h"
WXLUA_DECLARE_BIND_STD

#endif //(wxART2D_USE_LUA == 1)


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


bool MyApp::OnInit( void )
{
    wxString artroot = a2dGeneralGlobals->GetWxArt2DVar();
    if ( artroot.IsEmpty() )
        return false;

    wxString thumboRoot;
    wxGetEnv( _T( "THUMBO_ROOT" ), &thumboRoot );
    if( thumboRoot.IsEmpty() )
    {
        // try executable
		wxStandardPaths& stdPaths = wxStandardPaths::Get();
        wxFileName appFileName( stdPaths.GetExecutablePath() );
        //appFileName.MakeAbsolute();//wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_TILDE);
        thumboRoot = appFileName.GetVolume() + appFileName.GetVolumeSeparator() + appFileName.GetPath(wxPATH_GET_SEPARATOR, wxPATH_UNIX);
        if ( !::wxFileExists( thumboRoot + wxT( "doc/licence.txt" ) ) )
            thumboRoot.Empty();
        if( thumboRoot.IsEmpty() )
        {
            // where is my source code, while developing take it from there.
            thumboRoot = artroot + wxT( "apps" ) + wxFileName::GetPathSeparator(wxPATH_UNIX) + wxT( "thumbo" ) + wxFileName::GetPathSeparator(wxPATH_UNIX);
        }
        if ( !::wxFileExists( thumboRoot + wxT( "doc/licence.txt" ) ) )
        {
            wxString mes = _T( "THUMBO_ROOT WRONG:" ) + thumboRoot;
            wxMessageBox(  mes, _T( "environment error" ), wxOK );
            return false;
        }
        wxSetEnv( _T( "THUMBO_ROOT" ), thumboRoot );
        wxGetEnv( _T( "THUMBO_ROOT" ), &thumboRoot );
    }

    if( thumboRoot.IsEmpty() )
    {
#ifndef _GUNIX
        wxMessageBox( _T( "THUMBO_ROOT variable not set\nPlease set the THUMBO_ROOT environment string to the correct dir.\n\nExample : THUMBO_ROOT=c:\\tagra\n\n" ), _T( "environment error" ), wxOK );
#else
        wxMessageBox( _T( "THUMBO_ROOT variable not set\nPlease set the THUMBO_ROOT environment string to the correct dir.\n\nExample : THUMBO_ROOT=/user/home/thumbo; export THUMBO_ROOT \n\n" ), _T( "environment error" ), wxOK );
#endif
        return false;
    }

    // variable WXART2D_ART used internal to find cursors and such
    a2dDocviewGlobals->GetVariablesHash().SetVariableString( wxT( "APPLICATION_DATA" ), thumboRoot + wxT( "data" ) );

    wxInitAllImageHandlers();
    a2dMenuIdItem::InitializeBitmaps();

#if (wxART2D_USE_LUA == 1)
    // Initialize the wxLua bindings we want to use.
    // See notes for WXLUA_DECLARE_BIND_STD above.
    //WXLUA_IMPLEMENT_BIND_STD
    WXLUA_IMPLEMENT_BIND_WXLUA 
    WXLUA_IMPLEMENT_BIND_WXBASE 
    WXLUA_IMPLEMENT_BIND_WXCORE 
    

    wxLuaBinding_wxart2d_init();
    wxLuaBinding_thumbo_init();
#endif //(wxART2D_USE_LUA == 1)

    //wxString fname = a2dGlobals->GetFontPathList().FindValidPath( "nen.chr" );

    a2dGlobals->GetFontPathList().Add( artroot + wxT( "apps/thumbo/config" ) );
    a2dGlobals->GetImagePathList().Add( artroot +  wxT( "apps/thumbo/config" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "art/resources" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "apps/thumbo/config" ) );
    a2dCanvasGlobals->GetLayersPathList().Add( artroot +  wxT( "apps/thumbo/config" ) );
    a2dCanvasGlobals->GetLayersPathList().Add( artroot +  wxT( "art/layers" ) );

#if wxUSE_DEBUG_CONTEXT
    wxDebugContext::SetCheckpoint();
#endif

    a2dGlobals->SetPrimitiveThreshold( 3 );
    if ( !a2dCanvasGlobals->GetHabitat()->LoadLayers( wxT( "thumbolayers2.cvg" ) ) )
        return false;
    //a2dCanvasGlobals->GetHabitat()->SetLayerSetup( NULL );


    a2dCanvasGlobals->GetHabitat()->SetSelectStroke( a2dStroke( wxColour( 255, 55, 0 ), 2, a2dSTROKE_SOLID ) );
    a2dCanvasGlobals->GetHabitat()->SetSelectFill( *a2dTRANSPARENT_FILL );
    a2dCanvasGlobals->GetHabitat()->SetSelectDrawStyle( RenderWIREFRAME_SELECT );

    delete a2dDEFAULT_CANVASFONT;
    a2dDEFAULT_CANVASFONT = new a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 5 );
    a2dCanvasGlobals->GetHabitat()->SetFont( *a2dDEFAULT_CANVASFONT ); 

    a2dIOHandlerDocCVGIn* cvghin = new a2dIOHandlerDocCVGIn();
    a2dIOHandlerDocCVGOut* cvghout = new a2dIOHandlerDocCVGOut();

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
#endif //wxART2D_USE_GDSIO


    //first the base command processor for maintaining documents and views

    // wxLua specific commands strings translated to calls to a2dCentralCanvasCommandProcessor
#if wxART2D_USE_LUA
    m_cmdProc = new ThumboLuaProcCommandProcessor();
#else
    m_cmdProc = new a2dCentralCanvasCommandProcessor();
#endif

    //m_cmdProc->SetFill( *a2dNullFILL );
    //m_cmdProc->SetStroke( *a2dNullSTROKE );

    a2dDocviewGlobals->SetDocviewCommandProcessor( m_cmdProc );


    a2dRestrictionEngine* restrict = new a2dRestrictionEngine();


	restrict->SetSnapThresHold( 3 );

    restrict->SetSnapTargetFeatures( a2dRestrictionEngine::snapToGridPos | a2dRestrictionEngine::snapToGridPosForced
                                 | a2dRestrictionEngine::snapToObjectPos | a2dRestrictionEngine::snapToPinsUnconnected
                                 //|  a2dRestrictionEngine::snapToPointAngleMod 
                                 | a2dRestrictionEngine::snapToPointAngleModForced 
                                 | a2dRestrictionEngine::snapToObjectVertexes );

    restrict->SetSnapSourceFeatures( a2dRestrictionEngine::snapToBoundingBox | a2dRestrictionEngine::snapToObjectPos | 
									 a2dRestrictionEngine::snapToPinsUnconnected  | a2dRestrictionEngine::snapToObjectVertexes );

    /* for ortho line drawing
        restrict->SetSnapTargetFeatures( a2dRestrictionEngine::snapToGridPos
            |a2dRestrictionEngine::snapToObjectPos|a2dRestrictionEngine::snapToPinsUnconnected
            |a2dRestrictionEngine::snapToObjectVertexes
            |a2dRestrictionEngine::snapToPointAngleMod );
    */
    restrict->SetSnap( true );
    restrict->SetSnapGrid( 10, 10 );
    restrict->SetRotationAngle( 15 );
    a2dCanvasGlobals->GetHabitat()->SetRestrictionEngine( restrict );

    a2dRouteData::SetRaster( 20.0 );
    a2dPin::SetWorldBased( false );
	//a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->SetGeneratePins( false );
    a2dCanvasGlobals->GetHabitat()->SetRouteOneLine( false );

    a2dPort::Set( 10, 20, 30, 20 );
    m_connector = new wxDrawingConnector();
    m_connector->SetInitialPosition( wxPoint( 50, 40 ) );
    m_connector->SetInitialSize( wxSize( 200, 300 ) );

    //doc tamplates -----------------------

    a2dDocumentTemplate* doctemplatenew;

    doctemplatenew = new a2dDocumentTemplateAuto( _T( "All Files" ) , _T( "*.*" ) , _T( "" ) , _T( "" ) , _T( "ThumboDocument" ) ,
            CLASSINFO( ThumboDocument ), m_connector, a2dTemplateFlag::DEFAULT );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );

#if wxART2D_USE_GDSIO
    doctemplatenew = new a2dDocumentTemplate( wxT( "GDS-II Drawing" ), wxT( "*.cal" ), wxT( "" ), wxT( "cal" ), wxT( "ThumboDocument" ),
            CLASSINFO( ThumboDocument ), m_connector, a2dTemplateFlag::DEFAULT, gdshin, gdshout );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_GDSIO

    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate( wxT( "CVG Drawing" ), wxT( "*.cvg" ), wxT( "" ), wxT( "cvg" ), wxT( "ThumboDocument" ),
            CLASSINFO( ThumboDocument ), m_connector, a2dTemplateFlag::DEFAULT,  cvghin, cvghout );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );

#if wxART2D_USE_SVGIO
    doctemplatenew = new a2dDocumentTemplate( wxT( "SVG Drawing" ), wxT( "*.svg" ), wxT( "" ), wxT( "svg" ), wxT( "ThumboDocument" ),
            CLASSINFO( ThumboDocument ), m_connector, a2dTemplateFlag::DEFAULT, svghin, svghout );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_SVGIO

#if wxART2D_USE_KEYIO
    doctemplatenew = new a2dDocumentTemplate( wxT( "Key Drawing" ), wxT( "*.key" ), wxT( "" ), wxT( "key" ), wxT( "ThumboDocument" ),
            CLASSINFO( ThumboDocument ), m_connector, a2dTemplateFlag::DEFAULT, keyhin, keyhout );
    m_cmdProc->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_KEYIO

    //View templates ------------------------

    a2dViewTemplate* viewtemplatenew;
    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate( wxT( "CVG Drawing" ), wxT( "ThumboDocument" ), wxT( "Drawing View Frame" ),
                                           CLASSINFO( a2dCanvasView ), m_connector, a2dTemplateFlag::DEFAULT, wxSize( 100, 400 )  );
    m_connector->AssociateViewTemplate( viewtemplatenew );
    m_cmdProc->AssociateViewTemplate( viewtemplatenew );

    viewtemplatenew = new a2dViewTemplate( wxT( "CVG Drawing" ), wxT( "ThumboDocument" ), wxT( "Drawing View NoteBook Page" ),
                                           CLASSINFO( a2dCanvasView ), m_connector, a2dTemplateFlag::DEFAULT, wxSize( 100, 400 )  );
    m_connector->AssociateViewTemplate( viewtemplatenew );
    m_cmdProc->AssociateViewTemplate( viewtemplatenew );

    m_connector2 = new wxTextConnector();
    m_connector2->SetInitialPosition( wxPoint( 20, 210 ) );
    m_connector2->SetInitialSize( wxSize( 300, 200 ) );

    // Create a template relating drawing documents to text views
    viewtemplatenew =  new a2dViewTemplate( wxT( "Text" ), wxT( "ThumboDocument" ), wxT( "Text View Frame" ),
                                            CLASSINFO( TextView ), m_connector2, a2dTemplateFlag::DEFAULT, wxSize( 200, 300 ) );

    m_connector2->AssociateViewTemplate( viewtemplatenew );
    m_cmdProc->AssociateViewTemplate( viewtemplatenew );

    a2dSmrtPtr<ThumboDocument> document = new ThumboDocument();
    document->GetDrawing()->SetDrawingId( a2dDrawingId_cameleonrefs() );

    FillDocument5( document );
/*
    static double i = 200;

    a2dCanvasDocumentReference2* tr = new a2dCanvasDocumentReference2( 120, i, wxFileName( wxT( "polygons.cvg" ) ) );
    a2dRect* rect = new a2dRect( 0, 0, 100, 200 );
    rect->SetStroke( wxColour( 229, 5, 64 ), 0 );
    rect->SetFill( *a2dTRANSPARENT_FILL );
    root->Append( tr );
    tr->Append( rect );

    
    a2dCanvasDocumentReferenceAsText* trt = new a2dCanvasDocumentReferenceAsText( wxFileName( wxT( "arcs.svg" ) ), 120, -600 );
    a2dRect* rectt = new a2dRect( 0, 0, 100, 200 );
    rectt->SetStroke( wxColour( 29, 235, 64 ), 0 );
    rectt->SetFill( *a2dTRANSPARENT_FILL );
    trt->Append( rectt );
    root->Append( trt );
    

    delete a2dDEFAULT_CANVASFONT;
    a2dDEFAULT_CANVASFONT = new a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Bold.ttf" ) ), 500 );
    a2dCanvasGlobals->GetHabitat()->SetFont( *a2dDEFAULT_CANVASFONT );


    a2dText* tt = new a2dText( wxT( "Hello rotated text in a2dCanvas World" ), -500, 750,
                               a2dFont( 80.0, wxFONTFAMILY_SWISS ), 15.0 );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( *wxGREEN );
    root->Append( tt );
*/
    a2dDocviewGlobals->GetDocviewCommandProcessor()->AddCreatedDocument( document, true, true, a2dREFDOC_NEW );
    a2dCanvasThumboEditorFrame* frame = m_connector->m_lastMadeEditorFrame;//new a2dCanvasThumboEditorFrame( true, drawer2d, NULL, wxPoint(0, 0), wxSize(500, 400), wxDEFAULT_FRAME_STYLE);
    SetTopWindow( frame );
    // In this Thumbo application there is no main parent frame.
    // All frames will be eqaul and have no child frames.
    // Exit of application is based on last frame closed.
    // See void a2dCanvasThumboEditorFrame::OnCloseWindow( wxCloseEvent& event )
    m_connector->Init( NULL, true );

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
        a2dDocumentPtr doc = NULL;
        a2dError result = m_cmdProc->FileOpen( doc, infile );
        if ( !doc )
        {
            if ( result == a2dError_Canceled )
            {
            }
        }
    }

    //m_processFile = _T("../process/2files.lua");

#if (wxART2D_USE_LUA == 1)
    if ( !m_processFile.IsEmpty() && ::wxFileExists( m_processFile ) )
    {
        m_cmdProc->ExecuteFile( m_processFile );
    }
#endif

    //if command Exit does exit the application, the topwindow is closed, and deleted in idle time,
    //leading to stopping the event loop.

    //start the event loop ( e.g. for logging to show up. ), Even is top windoe is not visible, or closed already.
    int ret = wxAppBase::OnRun();

    if ( m_exit )
    {
    }
    return ret;
}

MyApp::~MyApp( void )
{
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
        { wxCMD_LINE_SWITCH, "v", "verbose", "be verbose" },
        { wxCMD_LINE_SWITCH, "q", "quiet",   "be quiet" },

        { wxCMD_LINE_OPTION, "i", "input",   "input dir" },
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

#if (wxART2D_USE_LUA == 1)

//------------------------------------------------------------------------
// Special command processor, for Thumbo lua commands
//------------------------------------------------------------------------
IMPLEMENT_CLASS( ThumboLuaProcCommandProcessor, a2dLuaCentralCommandProcessor )

ThumboLuaProcCommandProcessor* a2dGetThumbo() { return ( ( MyApp* )&wxGetApp() )->GetLuaCommandProc(); }

ThumboLuaProcCommandProcessor::ThumboLuaProcCommandProcessor( long flags , bool initialize, int maxCommands )
    : a2dLuaCentralCommandProcessor( flags, initialize, maxCommands )
{
    m_tooldlgThumbo = NULL;

    //RunBuffer(plotto_lua, plotto_lua_len-1, wxT("plotto.lua"));
}

ThumboLuaProcCommandProcessor::~ThumboLuaProcCommandProcessor()
{
    if ( m_tooldlgThumbo )
        m_tooldlgThumbo->Destroy();

    m_tooldlgThumbo = NULL;

}

void ThumboLuaProcCommandProcessor::DeleteDlgs()
{
    if ( m_tooldlgThumbo )
        delete m_tooldlgThumbo;

    m_tooldlgThumbo = NULL;
}

bool ThumboLuaProcCommandProcessor::ShowDlg( const a2dCommandId* comID, bool modal, bool onTop )
{
    if ( comID == &a2dCanvasCommandProcessor::COMID_ShowDlgTools )
    {
        if ( !m_tooldlgThumbo )
            m_tooldlgThumbo = new ToolDlg( NULL );

        if ( m_tooldlgThumbo->IsShown() )
        {
            m_tooldlgThumbo->Show( false );
        }
        else
        {
            m_tooldlgThumbo->Show( true );
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

void ThumboLuaProcCommandProcessor::OnExit()
{
    wxConfigBase* pConfig = wxConfigBase::Get();
    a2dGetCmdh()->FileHistorySave( *pConfig );

    DeleteDlgs();

    a2dLuaCentralCommandProcessor::OnExit();

    wxFrame* pf = ( wxFrame* ) wxGetApp().GetTopWindow();
    pf->Close( true );
    wxGetApp().SetExit();
}

#endif //(wxART2D_USE_LUA == 1)


void MyApp::FillDocument2( ThumboDocument* document )
{
	a2dCanvasObjectPtr root = document->GetDrawing()->GetRootObject();

    // top cameleon
        a2dCameleon* cam3 = new a2dCameleon(  wxT("top"), 0,0);
        a2dSymbol* sym3 = new a2dSymbol( cam3 );
        sym3->SetName( "top_symbol" );
        cam3->AddAppearance( sym3 );
        sym3->AppendToDrawing( new a2dRect( 0, 0, 21, 39 ) );    
        sym3->AppendToDrawing( new a2dCircle( 0, 0, 21 ) );    

        a2dDiagram* diagram3 = new a2dDiagram( cam3 );
        diagram3->SetName( "top_schematic" );
        cam3->AddAppearance( diagram3 );
        //diagram3->AppendToDrawing( new a2dRect( 0, 0, 21, 39 ) );    

        root->Append( cam3 );
    // end top cameleon

    // overview cameleon
        a2dCameleon* camoverview = new a2dCameleon( wxT("overview"), 0,-20);
        a2dDiagram* diagramoverview = new a2dDiagram( camoverview );
        diagramoverview->SetName( "overview" );
        camoverview->AddAppearance( diagramoverview );
        root->Append( camoverview );

        a2dCameleonSymbolicRef* nameref = new a2dCameleonSymbolicRef( 0,0, sym3 );
        diagramoverview->AppendToDrawing( nameref );

        a2dCameleonSymbolicRef* namerefd = new a2dCameleonSymbolicRef( 0,10, diagram3 );
        diagramoverview->AppendToDrawing( namerefd );
    //end overview cameleon

    //Start filling document now.
    //a2dLayers* docLayers = document->GetDrawing()->GetLayerSetup();
    //docLayers->SetStroke( 0, a2dStroke( *wxBLACK, 0.5 ) );
    //docLayers->SetFill( 0, a2dFill( wxColour( 0xff, 0xff, 0xc0 ) ) );

    // FIRST create a cameleon, with a symbol and a diagram

    a2dCameleon* cam1 = new a2dCameleon( wxT("aap"), 0,-40 );
    cam1->AddDoubleParameter( wxT("firstPar"), 12.345 );
    cam1->AddBoolParameter( wxT("secondPar"), true );
    a2dSymbol* sym1 = new a2dSymbol( cam1 );
    // how does it look like as place holder
    sym1->Append( new a2dRect( 0, -2.5, 20, 5 ) );    
    // define the symbol drawing
    sym1->AppendToDrawing( new a2dRect( 0, -2.5, 10, 15 ) );    
    sym1->AppendToDrawing( new a2dCircle( 5, 2, 3 ) );    
    a2dPort* sympin;
    sym1->AppendToDrawing( sympin = new a2dPort( NULL, 0, 0, wxT("aap_1") ) );
    sympin->GetPin()->SetPinClass( a2dPinClass::Standard );

    sym1->AppendToDrawing( sympin = new a2dPort( NULL, 10, 0, wxT("aap_2") ) );
    sympin->SetRotation( 180 );
    sympin->GetPin()->SetPinClass( a2dPinClass::Standard );
    //sym1->Append( new a2dPort( sym1, wxT("aap_1"), a2dPinClass::Standard, 0, 0 ) );
    //sym1->Append( new a2dPort( sym1, wxT("aap_2"), a2dPinClass::Standard, 100, 0 ) );

#ifdef WITHPARS
    //make some of the parameters visible inside the symbol
    a2dPropertyId* propid = cam1->GetParameterId( wxT("firstPar") );
    if ( propid )
        sym1->AppendToDrawing( new a2dVisibleParameter( cam1, propid, 0.0, -3.0, true, a2dFont( 2.0, wxFONTFAMILY_SWISS, wxITALIC ) ) );
    propid = cam1->GetParameterId( wxT("secondPar") );
    if ( propid )
        sym1->AppendToDrawing( new a2dVisibleParameter( cam1, propid, 0.0, -5.5, true, a2dFont( 2.0, wxFONTFAMILY_SWISS, wxITALIC ) ) );
#endif

    cam1->AddAppearance( sym1 );

    a2dDiagram* diagram1 = new a2dDiagram( cam1 );
    diagram1->SetName( wxT("MyCircuit") );
    diagram1->AppendToDrawing( new a2dRect( 0, -2.5, 10, 15 ) );    
    diagram1->AppendToDrawing( new a2dRect( 15, -7.5, 30, 25 ) );    
    diagram1->AppendToDrawing( new a2dPort( NULL, 0,  0, wxT("aap_1"), a2dPinClass::Standard ) );
    diagram1->AppendToDrawing( new a2dPort( NULL, 0, 45, wxT("aap_2"), a2dPinClass::Standard ) );
    cam1->AddAppearance( diagram1 );

    root->Append( cam1 );

    // SECOND create a cameleon, with a symbol and a diagram

    a2dCameleon* cam2 = new a2dCameleon(  wxT("noot"), 0,-60);
    a2dSymbol* sym2 = new a2dSymbol( cam2 );
    sym2->AppendToDrawing( new a2dRect( 0, -4, 15, 8 ) );
    sym2->AppendToDrawing( sympin = new a2dPort( NULL, 0, 0, wxT("noot_1") ) );
    sympin->GetPin()->SetPinClass( a2dPinClass::Standard );
    sym2->AppendToDrawing( sympin = new a2dPort( NULL, 15, 0, wxT("noot_2") ) );
    sympin->SetRotation( 180 );
    sympin->GetPin()->SetPinClass( a2dPinClass::Standard );
    //sym2->Append( new a2dPort( sym2, wxT("noot_1"), a2dPinClass::Standard, 0, 0 ) );
    //sym2->Append( new a2dPort( sym2, wxT("noot_2"), a2dPinClass::Standard, 150, 0 ) );
    cam2->AddAppearance( sym2 );

    a2dDiagram* diagram2 = new a2dDiagram( cam2 );
    diagram2->SetName( wxT("MyOtherCircuit") );
    diagram2->AppendToDrawing( new a2dRect( 0, -2.5, 20, 25 ) );    
    diagram2->AppendToDrawing( new a2dCircle( 25, -7.5, 25 ) );    
    diagram2->AppendToDrawing( new a2dPort( NULL,  0, 0, wxT("noot_1"), a2dPinClass::Standard ) );
    diagram2->AppendToDrawing( new a2dPort( NULL, 40, 5, wxT("noot_2"), a2dPinClass::Standard ) );
    diagram2->SetName( "diagram2" );
    cam2->AddAppearance( diagram2 );

    root->Append( cam2 );

    root = cam3->GetAppearance<a2dDiagram>()->GetDrawing()->GetRootObject();

    // place the two in a new group.

    a2dCameleonInst* ref1 = new a2dCameleonInst( 2, 5, sym1 ); 
    a2dCameleonInst* ref2 = new a2dCameleonInst( 20, 5, sym2 );
    ref1->SetName( "ref1" );
    ref2->SetName( "ref2" );
    diagram3->AppendToDrawing( ref1 );
    diagram3->AppendToDrawing( ref2 );
    diagram3->SetName( "schematic" );

    a2dCameleonInst* ref3 = new a2dCameleonInst( 52.2, 5, diagram2 );
    ref3->SetName( "ref3" );
    diagram3->AppendToDrawing( ref3 );

    document->SetTopCameleon( camoverview );
    document->SetTopCameleon( cam3 );
    document->GetDrawing()->SetDrawingId( a2dDrawingId_cameleonrefs() );

    cam1->TriggerChangedTime();
    cam2->TriggerChangedTime();
    cam3->TriggerChangedTime();

    document->SetShowObject( diagramoverview->GetDrawing()->GetRootObject() );
    document->SetShowObject( document->GetDrawing()->GetRootObject() );
    document->SetShowObject( diagram3->GetDrawing()->GetRootObject() );
}

void MyApp::FillDocument3( ThumboDocument* document )
{
	a2dCanvasObjectPtr root = document->GetDrawing()->GetRootObject();
/*
    // top cameleon
        a2dCameleon* cam3 = new a2dCameleon(0,0, wxT("top"));
        a2dSymbol* sym3 = new a2dSymbol( cam3 );
        sym3->SetName( "top_symbol" );
        cam3->AddAppearance( sym3 );
        sym3->AppendToDrawing( new a2dRect( 0, 0, 21, 39 ) );    
        sym3->AppendToDrawing( new a2dCircle( 0, 0, 21 ) );    

        a2dDiagram* diagram3 = new a2dDiagram( cam3 );
        diagram3->SetName( "top_schematic" );
        cam3->AddAppearance( diagram3 );
        //diagram3->AppendToDrawing( new a2dRect( 0, 0, 21, 39 ) );    

        root->Append( cam3 );
    // end top cameleon
*/
    //Start filling document now.
    //a2dLayers* docLayers = document->GetDrawing()->GetLayerSetup();
    //docLayers->SetStroke( 0, a2dStroke( *wxBLACK, 0.5 ) );
    //docLayers->SetFill( 0, a2dFill( wxColour( 0xff, 0xff, 0xc0 ) ) );

    // FIRST create a cameleon, with a symbol and a diagram

    a2dCameleon* cam1 = new a2dCameleon( wxT("aap"), 0,-40 );
    cam1->AddDoubleParameter( wxT("firstPar"), 12.345 );
    cam1->AddBoolParameter( wxT("secondPar"), true );
    a2dSymbol* sym1 = new a2dSymbol( cam1 );
    // how does it look like as place holder
    sym1->Append( new a2dRect( 0, -2.5, 20, 5 ) );    
    // define the symbol drawing
    sym1->AppendToDrawing( new a2dRect( 0, -2.5, 10, 15 ) );    
    sym1->AppendToDrawing( new a2dCircle( 5, 2, 3 ) );    
    a2dPort* sympin;
    sym1->AppendToDrawing( sympin = new a2dPort( NULL, 0, 0, wxT("aap_1") ) );
    sympin->GetPin()->SetPinClass( a2dPinClass::Standard );

    sym1->AppendToDrawing( sympin = new a2dPort( NULL, 10, 0, wxT("aap_2") ) );
    sympin->SetRotation( 180 );
    sympin->GetPin()->SetPinClass( a2dPinClass::Standard );
    //sym1->Append( new a2dPort( sym1, wxT("aap_1"), a2dPinClass::Standard, 0, 0 ) );
    //sym1->Append( new a2dPort( sym1, wxT("aap_2"), a2dPinClass::Standard, 100, 0 ) );

#ifdef WITHPARS
    //make some of the parameters visible inside the symbol
    a2dPropertyId* propid = cam1->GetParameterId( wxT("firstPar") );
    if ( propid )
        sym1->AppendToDrawing( new a2dVisibleParameter( cam1, propid, 0.0, -3.0, true, a2dFont( 2.0, wxFONTFAMILY_SWISS, wxITALIC ) ) );
    propid = cam1->GetParameterId( wxT("secondPar") );
    if ( propid )
        sym1->AppendToDrawing( new a2dVisibleParameter( cam1, propid, 0.0, -5.5, true, a2dFont( 2.0, wxFONTFAMILY_SWISS, wxITALIC ) ) );
#endif

    cam1->AddAppearance( sym1 );

    a2dDiagram* diagram1 = new a2dDiagram( cam1 );
    diagram1->SetName( wxT("MyCircuit") );
    diagram1->AppendToDrawing( new a2dRect( 0, -2.5, 10, 15 ) );    
    diagram1->AppendToDrawing( new a2dRect( 15, -7.5, 30, 25 ) );    
    diagram1->AppendToDrawing( new a2dPort( NULL, 0,  0, wxT("aap_1") ) );
    diagram1->AppendToDrawing( new a2dPort( NULL, 0, 45, wxT("aap_2") ) );
    cam1->AddAppearance( diagram1 );

    //root->Append( cam1 );

    // SECOND create a cameleon, with a symbol and a diagram

    a2dCameleon* cam2 = new a2dCameleon( wxT("noot"), 0,-60 );
    a2dSymbol* sym2 = new a2dSymbol( cam2 );
    sym2->AppendToDrawing( new a2dRect( 0, -4, 15, 8 ) );
    sym2->AppendToDrawing( sympin = new a2dPort( NULL, 0, 0,wxT("noot_1") ) );
    sympin->GetPin()->SetPinClass( a2dPinClass::Standard );
    sym2->AppendToDrawing( sympin = new a2dPort( NULL, 15, 0, wxT("noot_2") ) );
    sympin->SetRotation( 180 );
    sympin->GetPin()->SetPinClass( a2dPinClass::Standard );
    //sym2->Append( new a2dPort( sym2, wxT("noot_1"), a2dPinClass::Standard, 0, 0 ) );
    //sym2->Append( new a2dPort( sym2, wxT("noot_2"), a2dPinClass::Standard, 150, 0 ) );
    cam2->AddAppearance( sym2 );

    a2dDiagram* diagram2 = new a2dDiagram( cam2 );
    diagram2->SetName( wxT("MyOtherCircuit") );
    diagram2->AppendToDrawing( new a2dRect( 0, -2.5, 20, 25 ) );    
    diagram2->AppendToDrawing( new a2dCircle( 25, -7.5, 25 ) );    
    diagram2->AppendToDrawing( new a2dPort( NULL,  0, 0, wxT("noot_1"), a2dPinClass::Standard ) );
    diagram2->AppendToDrawing( new a2dPort( NULL, 40, 5, wxT("noot_2"), a2dPinClass::Standard ) );
    diagram2->SetName( "diagram2" );
    cam2->AddAppearance( diagram2 );

    // add the two to the drawing.
    root->Prepend( cam1 );
    root->Prepend( cam2 );

    // place the two in a new group.

    a2dCanvasObject* gr = new a2dCanvasObject(0,0);
    root->Prepend( gr );

    //root->Append( cam2 );

    //root = cam3->GetAppearance<a2dDiagram>()->GetDrawing()->GetRootObject();

    // place the two in a new group.

    a2dCameleonInst* ref1 = new a2dCameleonInst( 2, 5, sym1 ); 
    a2dCameleonInst* ref2 = new a2dCameleonInst( 20, 5, sym2 );
    ref1->SetName( "ref1" );
    ref2->SetName( "ref2" );
    gr->Append( ref1 );
    gr->Append( ref2 );
    gr->SetName( "schematic" );

    a2dCameleonInst* ref3 = new a2dCameleonInst( 52.2, 5, diagram2 );
    ref3->SetName( "ref3" );
    gr->Append( ref3 );

    //document->m_main = cam3;
    //document->GetDrawing()->SetDrawingId( a2dDrawingId_cameleonrefs() );

    //document->m_show = diagram3->GetDrawing()->GetRootObject();
    document->SetShowObject( gr );

    wxSleep(1);
    cam1->TriggerChangedTime();
    cam2->TriggerChangedTime();
    //cam3->TriggerChangedTime();


}


void MyApp::FillDocument5( ThumboDocument* doc )
{
    a2dDrawing* drawing = doc->GetDrawing();
	a2dCanvasObjectPtr root = drawing->GetRootObject();

    a2dLayers* drawLayers = drawing->GetLayerSetup();
    if ( drawLayers )
    {
        drawLayers->SetStroke( 0, a2dStroke( *wxBLACK, 0.1 ) );
        drawLayers->SetFill( 0, a2dFill( wxColour( 0xff, 0xff, 0xc0 ) ) );
    }
    else
    {
        a2dCanvasGlobals->GetHabitat()->SetDefaultStroke( a2dStroke( *wxBLACK, 0.1 ) );
        a2dCanvasGlobals->GetHabitat()->SetDefaultFill( a2dFill( wxColour( 0xff, 0xff, 0xc0 ) ) );
    }

    a2dPinClass* pclass = a2dConnectionGeneratorObjectWire::Object;
    pclass = a2dPinClass::Standard;
    // cameleons go in the top of the document.
    root->ReleaseChildObjects();

    // FIRST create a cameleon, with a symbol

    a2dCameleon::SetCameleonRoot( root );
    a2dCameleon* cam1 = new a2dCameleon( wxT("aap"), 0,0, a2dCanvasGlobals->GetHabitat() );
    cam1->AddToRoot();

    a2dSymbol* sym1 = new a2dSymbol( cam1, 0,0 );
    // define the symbol drawing
    sym1->AppendToDrawing( new a2dRect( 0, -25, 100, 150 ) );    
    a2dPort* sympin;
    sym1->AppendToDrawing( sympin = new a2dPort( NULL, 0, 0, wxT("aap_1") ) );
    sympin->GetPin()->SetPinClass( pclass );

    cam1->AddAppearance( sym1 );


    // THIRD create a cameleon, with a diagram being the top
    a2dCameleon* cam3 = new a2dCameleon( wxT("top"), 0,0, a2dCanvasGlobals->GetHabitat() );
    cam3->AddToRoot();
    a2dDiagram* diagram3 = new a2dDiagram( cam3, 0,0 );
    diagram3->SetName( "top cameleon overview" );
    cam3->AddAppearance( diagram3 );

    // file top diagram
    a2dCameleonInst* ref1 = new a2dCameleonInst( 20, 50, sym1 ); 
    ref1->SetName( "ref1" );
    diagram3->AppendToDrawing( ref1 );

    diagram3->AppendToDrawing( new a2dRect( 60, -35, 200, 50 ) );    
    diagram3->AppendToDrawing( new a2dRect( 160, -85, 200, 50 ) );    

    a2dCameleon* camall = new a2dCameleon( wxT("all"),0,0,a2dCanvasGlobals->GetHabitat() );
    camall->AddToRoot();
    a2dDiagram* diagramall = new a2dDiagram( camall );
    diagramall->SetName( "all cameleons in document" );
    camall->AddAppearance( diagramall );

    diagramall->AppendToDrawing( new a2dCameleonSymbolicRef( 0, 0 , cam1 ) );
    diagramall->AppendToDrawing( new a2dCameleonSymbolicRef( 0, -20 , cam3 ) );

    doc->SetShowObject( diagram3->GetDrawing()->GetRootObject() );
    doc->SetShowObject( root );
    doc->SetTopCameleon( cam3 );

}

