/*! \file editor/samples/viewmul/docv.cpp
    \brief Document/view demo
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: docv.cpp,v 1.5 2009/09/26 19:01:19 titato Exp $
*/

/*
* Purpose:  Document/view architecture demo for wxWindows class library
*/

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

#include "docv.h"
#include "doc.h"
#include "viewmul.h"

#include <wx/memory.h>
#include <wx/editor/recurdoc.h>


// In single window mode, don't have any child windows; use
// main window.
bool singleWindowMode = false;


IMPLEMENT_APP( MyApp )

MyApp::MyApp( void )
{
}

bool MyApp::OnInit( void )
{

    wxInitAllImageHandlers();

    //wxDebugContext::SetCheckpoint();

    //// Create a document manager
    a2dCentralCanvasCommandProcessor* docmanager = new a2dCentralCanvasCommandProcessor();
    a2dDocviewGlobals->SetDocviewCommandProcessor( docmanager );

    m_connector = new a2dMultiDrawingConnector();
    m_connector->SetInitialPosition( wxPoint( 50, 40 ) );
    m_connector->SetInitialSize( wxSize( 200, 300 ) );

    a2dDocumentTemplate* doctemplatenew;
    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate( wxT( "recs Drawing" ), wxT( "*.recs" ), wxT( "" ), wxT( "recs" ), wxT( "DrawingDocument" ),
            CLASSINFO( DrawingDocument ), m_connector, a2dTemplateFlag::DEFAULT );
    docmanager->AssociateDocTemplate( doctemplatenew );


    a2dViewTemplate* viewtemplatenew;
    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate( wxT( "recs Drawing" ), wxT( "DrawingDocument" ), wxT( "Drawing View Frame" ),
                                           CLASSINFO( a2dCanvasView ), m_connector, a2dTemplateFlag::DEFAULT );
    m_connector->AssociateViewTemplate( viewtemplatenew );
    docmanager->AssociateViewTemplate( viewtemplatenew );


    m_connector2 = new wxTextConnector();
    m_connector2->SetInitialPosition( wxPoint( 20, 210 ) );
    m_connector2->SetInitialSize( wxSize( 300, 200 ) );

    // Create a template relating drawing documents to text views
    viewtemplatenew =  new a2dViewTemplate( wxT( "Text" ), wxT( "DrawingDocument" ), wxT( "Text View Frame" ),
                                            CLASSINFO( TextView ), m_connector2, a2dTemplateFlag::DEFAULT, wxSize( 200, 300 ) );
    m_connector->AssociateViewTemplate( viewtemplatenew );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    //// Create the main frame window
    MyFrame* frame = new MyFrame( ( wxFrame* ) NULL, -1, wxT( "DocView Demo" ), wxPoint( 0, 0 ), wxSize( 500, 400 ), wxDEFAULT_FRAME_STYLE );

    SetTopWindow( frame );
    m_connector->Init( frame, true );

    return true;
}





int MyApp::OnExit( void )
{

    m_connector->DisassociateViewTemplates();
    m_connector2->DisassociateViewTemplates();

    /*
       _CrtMemState checkPt1;

       // Send all reports to STDOUT, since this example is
       // a console app

       SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF );

       _CrtCheckMemory( );

       _CrtSetDumpClient( MyDumpClientHook );
    #ifdef _DEBUG
       _CrtMemCheckpoint( &checkPt1 );

       _CrtMemDumpAllObjectsSince( &checkPt1 );
       _CrtMemCheckpoint( &checkPt1 );
       _CrtMemDumpStatistics( &checkPt1 );
       _CrtCheckMemory( );

       _CrtMemDumpAllObjectsSince( NULL );
    #endif
       //_CrtMemState aap;
       //_CrtMemCheckpoint( &aap );
       //_CrtMemDumpAllObjectsSince( &aap );
        _CrtDumpMemoryLeaks();

        //_CrtMemDumpAllObjectsSince( NULL );

        //_CrtMemState aap;
        //_CrtMemDumpStatistics( &aap );
        //_CrtDumpMemoryLeaks();
    */
    //  wxDebugContext::Dump();
    return 0;
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
	m_contr = new a2dStToolContr( this->GetDrawingPart(), frame );
    m_contr->SetZoomFirst( true );    
	a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent(wxEVT_ADD_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_REMOVE_DOCUMENT, this );
}

//Open clicked root in a MyCanvas
void OverviewCanvas::OnMouseEvent( wxMouseEvent& event )
{
    Raise();
    if ( !GetDrawing() || !GetDrawingPart()->GetMouseEvents() )
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

        a2dIterC ic( GetDrawingPart() );
        a2dIterCU cu( ic );
        a2dHitEvent hitevent = a2dHitEvent( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
        a2dCanvasObject* obj = GetShowObject()->IsHitWorld( ic, hitevent );

        if ( obj && ( 0 != wxDynamicCast( obj, a2dCanvasDocumentReference ) ) )
        {
            //a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateView( ( a2dDocument* ) ( ( a2dCanvasDocumentReference* ) obj )->GetCanvasObject()->GetRoot()->GetCanvasDocument() );
        }
    }
}

OverviewCanvas::~OverviewCanvas()
{
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

/*
* This is the top-level window of the application.
*/
#define CANVAS_ABOUT 10000

IMPLEMENT_CLASS( MyFrame, a2dDocumentFrame )
BEGIN_EVENT_TABLE( MyFrame, a2dDocumentFrame )
    EVT_MENU( CANVAS_ABOUT, MyFrame::OnAbout )
    EVT_CLOSE( MyFrame::OnCloseWindow )
END_EVENT_TABLE()

MyFrame::MyFrame( wxFrame* frame, wxWindowID id, const wxString& title,
                  const wxPoint& pos, const wxSize& size, const long type ):
    a2dDocumentFrame( true, frame, NULL, id, title, pos, size, type )
{
    //// Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
    SetIcon( wxIcon( wxString( wxT( "doc_icn" ) ) ) );
#endif

    //// Make a menubar
    wxMenu* file_menu = new wxMenu;
    wxMenu* edit_menu = ( wxMenu* ) NULL;

    AddCmdMenu( file_menu, CmdMenu_FileNew() );
    AddCmdMenu( file_menu, CmdMenu_FileOpen() );
    AddCmdMenu( file_menu, CmdMenu_FileClose() );
    AddCmdMenu( file_menu, CmdMenu_FileSave() );
    AddCmdMenu( file_menu, CmdMenu_FileSaveAs() );
    AddCmdMenu( file_menu, CmdMenu_Print() );
    file_menu->AppendSeparator();
    AddCmdMenu( file_menu, CmdMenu_PrintView() );
    AddCmdMenu( file_menu, CmdMenu_PreviewView() );
    AddCmdMenu( file_menu, CmdMenu_PrintDocument() );
    AddCmdMenu( file_menu, CmdMenu_PreviewDocument() );
    AddCmdMenu( file_menu, CmdMenu_PrintSetup() );

    file_menu->Append( wxID_EXIT, _( "E&xit" ) );

    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( file_menu );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( CANVAS_ABOUT, _( "&About" ) );

    wxMenuBar* menu_bar = new wxMenuBar;

    menu_bar->Append( file_menu, _( "&File" ) );
    if ( edit_menu )
        menu_bar->Append( edit_menu, _( "&Edit" ) );
    menu_bar->Append( help_menu, _( "&Help" ) );

    //// Associate the menu bar with the frame
    SetMenuBar( menu_bar );

    Centre( wxBOTH );
    Show( true );

    CreateStatusBar( 2 );

    int width, height;
    GetClientSize( &width, &height );

    m_topcanvas = new OverviewCanvas( this, wxPoint( 0, 0 ), size );

    a2dDrawer2D* drawer2d = new a2dMemDcDrawer( width, height );
    m_topcanvas->GetDrawingPart()->SetDrawer2D( drawer2d );

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

MyFrame::~MyFrame()
{
    //undo stack can hold Owners to Documents etc.
    m_topcanvas->GetDrawing()->GetCommandProcessor()->ClearCommands();
}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    if ( event.CanVeto() )
    {
        if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear( !event.CanVeto() ) )
        {
            Destroy();
        }
        else
            event.Veto( true );
    }
    else
    {
        a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear( true );
        Destroy();
    }
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "a2dCanvas MultiView Demo\nAuthor: Klaas Holwerda" ), wxT( "About DocView" ) );
}



