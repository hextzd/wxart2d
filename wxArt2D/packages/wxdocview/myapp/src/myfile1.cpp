/*! \file myfile1.cpp
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: myapp.cpp,v 1.5 2009/09/26 19:01:19 titato Exp $
*/

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "myfile1.h"

#include "wx/colordlg.h"
#include <wx/wfstream.h>

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#include "doc.h"

IMPLEMENT_APP( MyApp )


BEGIN_EVENT_TABLE( wxOverViewWindow, wxScrolledWindow )
    EVT_PAINT( wxOverViewWindow::OnPaint )
END_EVENT_TABLE()

wxOverViewWindow::wxOverViewWindow( a2dMultiViewDocManager* docmanager, wxWindow* parent, wxWindowID id , const wxPoint& pos , const wxSize& size )
    : wxScrolledWindow( parent, id, pos, size )
{
    m_docmanager = docmanager;
    // Give it scrollbars
    SetVirtualSize( 1000, 3000 );
    SetScrollRate( 100, 100 );
    SetBackgroundColour( *wxWHITE );
}

wxOverViewWindow::~wxOverViewWindow()
{
}

void wxOverViewWindow::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );
    DrawDocumentsAndViews();
}

void wxOverViewWindow::DrawDocumentsAndViews()
{
    wxFont font = wxFont( 15, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );

    wxClientDC dc( this );
    dc.SetFont( font );
    dc.SetBackgroundMode( wxSOLID );
    PrepareDC( dc );

    int y = 10;
    int x = 10;

    const_forEachIn( a2dDocumentList, &m_docmanager->GetDocuments() )
    {
        a2dDocument* doc = *iter;

        dc.DrawText( wxT( "Document=> " ) + doc->GetFilename().GetFullName(), x, y );
        y += 25;

        x += 100;

        a2dViewList allviews;
        doc->ReportViews( &allviews );

        const_forEachIn( a2dViewList, &allviews )
        {
            a2dView* view = *iter;

            dc.DrawText( wxT( "View=> " ) + view->GetViewTypeName(), x, y );
            y += 25;
        }
        x -= 100;
    }
    dc.SetFont( wxNullFont );
}

//-----------------------------------------------------------------------------
// MyApp
//-----------------------------------------------------------------------------

bool MyApp::OnInit()
{
    //// Create a document manager
    a2dMultiViewDocManager* docmanager = new a2dMultiViewDocManager();
    a2dDocviewGlobals->SetDocviewCommandProcessor( docmanager );

    a2dViewTemplate* viewtemplatenew;

    m_mainconnector = new a2dMultiFrameDrawingConnector( CLASSINFO( MyFrame ) );
    m_mainconnector->SetInitialPosition( wxPoint( 50, 40 ) );
    m_mainconnector->SetInitialSize( wxSize( 200, 300 ) );

    a2dDocumentTemplate* doctemplatenew;

    doctemplatenew = new a2dDocumentTemplateAuto( _T( "All Files" ) , _T( "*.*" ) , _T( "" ) , _T( "" ) , _T( "DrawingDocument" ) ,
            CLASSINFO( DrawingDocument ), m_mainconnector, a2dTemplateFlag::DEFAULT );
    docmanager->AssociateDocTemplate( doctemplatenew );

    DoodleDocumentIoHandlerIn* iohandlerFormatIn = new DoodleDocumentIoHandlerIn();
    DoodleDocumentIoHandlerOut* iohandlerFormatOut = new DoodleDocumentIoHandlerOut();

    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate(
        wxT( "DRW Drawing" ),
        wxT( "*.drw" ),
        wxT( "" ),
        wxT( "drw" ),
        wxT( "DrawingDocument" ),
        CLASSINFO( DrawingDocument ),
        m_mainconnector,
        a2dTemplateFlag::DEFAULT, iohandlerFormatIn, iohandlerFormatOut );
    docmanager->AssociateDocTemplate( doctemplatenew );


    DoodleDocumentIoHandlerDxrIn* iohandlerFormatDxrIn = new DoodleDocumentIoHandlerDxrIn();
    DoodleDocumentIoHandlerDxrOut* iohandlerFormatDxrOut = new DoodleDocumentIoHandlerDxrOut();

    // Create a template relating drawing documents to their files
    a2dDocumentTemplate* doct2 =  new a2dDocumentTemplate(
        wxT( "DRW2 Drawing" ),
        wxT( "*.drx" ),
        wxT( "" ),
        wxT( "drx" ),
        wxT( "DrawingDocument" ),
        CLASSINFO( DrawingDocument ),
        m_mainconnector,
        a2dTemplateFlag::DEFAULT,  iohandlerFormatDxrIn, iohandlerFormatDxrOut );
    docmanager->AssociateDocTemplate( doct2 );

    // Create a template relating drawing documents to drawing views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Drawing View Frame" ),
        CLASSINFO( DrawingView ),
        m_mainconnector,
        a2dTemplateFlag::DEFAULT, wxSize( 100, 400 ) );
    m_mainconnector->AssociateViewTemplate( viewtemplatenew );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    // Create a template relating drawing documents to text views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Text View Frame" ),
        CLASSINFO( TextView ),
        m_mainconnector,
        a2dTemplateFlag::DEFAULT, wxSize( 200, 300 ) );
    m_mainconnector->AssociateViewTemplate( viewtemplatenew );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    // Create a template relating drawing documents to statistics views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Statistics View Frame" ),
        CLASSINFO( StatisticsView ),
        m_mainconnector,
        a2dTemplateFlag::DEFAULT, wxSize( 200, 300 ) );
    m_mainconnector->AssociateViewTemplate( viewtemplatenew );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    //Create the main frame window
    //the parent frame will get event from the child frames.
    m_frame = new MyParentFrame( docmanager, NULL, -1, wxT( "MultiFrame with docmanager" ), wxPoint( 10, 110 ), wxSize( 500, 400 ), wxNO_FULL_REPAINT_ON_RESIZE | wxDEFAULT_FRAME_STYLE  );
    m_mainconnector->Init( m_frame, true );
    m_frame->Show( true );

    SetTopWindow( m_frame );

    docmanager->SetOverViewWindow( m_frame->m_outputwindow );

    return true;
}


int MyApp::OnExit()
{
    m_mainconnector->DisassociateViewTemplates();
    return 0;
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( MyFrame, EditorFrame )

BEGIN_EVENT_TABLE( MyFrame, EditorFrame )
//    EVT_MENU(WXDOCVIEW_ABOUT, MyFrame::OnAbout)
END_EVENT_TABLE()

MyFrame::MyFrame( wxFrame* parent,
                  DrawingView* view,
                  wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, const long style ) :

    EditorFrame( false, parent, NULL, -1, title, position, size, style  )

{
    SetStatusText( wxT( "Welcome to Multi Frame Managed views sample!" ) );

    //m_outputwindow = new a2dDocumentViewWindow(view, this, -1, wxPoint(0, 0), size, 0);
    m_canvas = new a2dDocumentViewScrolledWindow( view, this, -1, wxPoint( 0, 0 ), size, wxNO_FULL_REPAINT_ON_RESIZE );
    m_canvas->SetCursor( wxCURSOR_PENCIL );

    // Give it scrollbars
    m_canvas->SetScrollbars( 20, 20, 50, 50 );
    m_canvas->SetBackgroundColour( *wxWHITE );
    m_canvas->ClearBackground();
    //can set it now
    m_singleconnector->SetDisplayWindow( m_canvas );

}

bool MyFrame::Create( bool isParent,
                      wxFrame* parent,
                      DrawingView* view,
                      wxWindowID id, const wxString& title,
                      const wxPoint& position, const wxSize& size, const long style )
{
    //creation must be before any making of sub windows.
    bool res = EditorFrame::Create( isParent, parent, NULL, -1, title, position, size );

    m_canvas = new a2dDocumentViewScrolledWindow( view, this, -1, wxPoint( 0, 0 ), size, wxNO_FULL_REPAINT_ON_RESIZE );
    m_canvas->SetCursor( wxCURSOR_PENCIL );

    // Give it scrollbars
    m_canvas->SetScrollbars( 20, 20, 50, 50 );
    m_canvas->SetBackgroundColour( *wxWHITE );
    m_canvas->ClearBackground();

    //can set it now
    m_singleconnector->SetDisplayWindow( m_canvas );

    SetStatusText( wxT( "Welcome to Multi Frame Managed views sample!" ) );

    return res;
}

MyFrame::~MyFrame()
{
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "Multi Frame Demo\nAuthor: Klaas Holwerda" ), wxT( "About Multi Frame" ) );
}


//------------------------------------------------------------------------------
// MyParentFrame
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE( MyParentFrame, a2dDocumentFrame )
    EVT_MENU( wxID_EXIT, MyParentFrame::OnExit )
    EVT_MENU( WXDOCVIEW_ABOUT, MyParentFrame::OnAbout )
    EVT_CLOSE( MyParentFrame::OnCloseWindow )
END_EVENT_TABLE()


MyParentFrame::MyParentFrame( a2dMultiViewDocManager* manager, wxFrame* parent, wxWindowID id, const wxString& title,
                              const wxPoint& position, const wxSize& size, const long style ) :
    a2dDocumentFrame( true, parent, NULL, -1, title, position, size, style )
{
    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( wxT( "Welcome to Multiple Frame Managed Views sample!" ) );

    SetIcon( wxICON( mondrian ) );

    m_outputwindow = new wxOverViewWindow( manager, this, -1, wxPoint( 0, 0 ), size );
    m_outputwindow->SetBackgroundColour( *wxWHITE );

    // Needed here, because wxOverViewWindow is derived from wxScrolled window, not wxDocViewScrolled Window
    // Should be removed!
#if wxCHECK_VERSION(2, 5, 0)
    m_outputwindow->ClearBackground();
#else
    m_outputwindow->Clear();
#endif

    //to distribute activate view
    manager->SetParentWindow( this );
}

void MyParentFrame::CreateMyMenuBar()
{

    wxMenuBar* menu_bar = new wxMenuBar();

    wxMenu* file_menu = new wxMenu;

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

    file_menu->AppendSeparator();
    AddCmdMenu( file_menu, CmdMenu_Exit() );

    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( file_menu );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( WXDOCVIEW_ABOUT, _( "&About" ) );

    menu_bar->Append( file_menu, _( "&File" ) );
    menu_bar->Append( help_menu, _( "&Help" ) );

    SetMenuBar( menu_bar );

}

MyParentFrame::~MyParentFrame()
{
}

void MyParentFrame::Refresh( bool eraseBackground, const wxRect* rect )
{
    m_outputwindow->Refresh();
}

void MyParentFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "Multi Frame Demo\nAuthor: Klaas Holwerda" ), wxT( "About Multi Frame" ) );
}

void MyParentFrame::OnExit( wxCommandEvent& event )
{
    Close( true );
}

void MyParentFrame::OnCloseWindow( wxCloseEvent& event )
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


//------------------------------------------------------------------------
// Special document manager to show open files on top parent frame
//------------------------------------------------------------------------
IMPLEMENT_CLASS( a2dMultiViewDocManager, a2dDocumentCommandProcessor )

a2dMultiViewDocManager::a2dMultiViewDocManager( long flags, bool initialize ): a2dDocumentCommandProcessor( flags, initialize )
{
    m_overview = NULL;
    m_parentFrameInApp = NULL;
}

a2dMultiViewDocManager::~a2dMultiViewDocManager()
{

}

void a2dMultiViewDocManager::SetOverViewWindow( wxOverViewWindow* overview )
{
    m_overview = overview;
}

void a2dMultiViewDocManager::AddDocument( a2dDocument* doc )
{
    a2dDocumentCommandProcessor::AddDocument( doc );
    m_overview->Refresh();
}

void a2dMultiViewDocManager::RemoveDocument( a2dDocument* doc )
{
    a2dDocumentCommandProcessor::RemoveDocument( doc );
    m_overview->Refresh();
}

