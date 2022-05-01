/*! \file docview/samples/splitwin/splitwin.cpp
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: splitwin.cpp,v 1.7 2009/07/28 19:47:15 titato Exp $
*/

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "splitwin.h"

#include "wx/colordlg.h"
#include <wx/wfstream.h>

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#include "docdialog.h"
#include "doc.h"

IMPLEMENT_APP( MyApp )



//-----------------------------------------------------------------------------
// MyApp
//-----------------------------------------------------------------------------

bool MyApp::OnInit()
{
    a2dDocumentCommandProcessor* docmanager = a2dDocviewGlobals->GetDocviewCommandProcessor();

    typedef a2dSplitterConnector<a2dDocumentViewScrolledWindow> splitcon;
    a2dSmrtPtr< splitcon > splitconnector = new a2dSplitterConnector<a2dDocumentViewScrolledWindow>();

    a2dDocumentTemplate* doctemplatenew;

    doctemplatenew = new a2dDocumentTemplateAuto( _T( "All Files" ) , _T( "*.*" ) , _T( "" ) , _T( "" ) , _T( "DrawingDocument" ) ,
            CLASSINFO( DrawingDocument ), splitconnector, a2dTemplateFlag::DEFAULT );
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
        splitconnector,
        a2dTemplateFlag::DEFAULT, iohandlerFormatIn, iohandlerFormatOut );
    docmanager->AssociateDocTemplate( doctemplatenew );

    a2dViewTemplate* viewtemplatenew;

    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Drawing View" ),
        CLASSINFO( DrawingView ),
        splitconnector,
        a2dTemplateFlag::DEFAULT );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    // Create a template relating drawing documents to statistics views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Statistics View" ),
        CLASSINFO( StatisticsView ),
        splitconnector,
        a2dTemplateFlag::DEFAULT );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    //// Create the main frame window
    m_frame = new MyFrame( splitconnector, NULL, -1, wxT( "Splitwin with docmanager" ), wxPoint( 10, 110 ), wxSize( 500, 400 ), wxDEFAULT_FRAME_STYLE  );
    m_frame->Show( true );

    SetTopWindow( m_frame );

    return true;
}


int MyApp::OnExit()
{
    return 0;
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

#define CANVAS_ABOUT 1



BEGIN_EVENT_TABLE( MyFrame, EditorFrame )
    EVT_MENU_RANGE( wxID_FILE1, wxID_FILE9, MyFrame::OnMRUFile )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_MENU( WXDOCVIEW_ABOUT11, MyFrame::OnAbout )
    EVT_CLOSE_VIEW( MyFrame::OnCloseView )
    EVT_MENU( WXDOCVIEW_CONNECTWINDOW1, MyFrame::SetConnectWindow )
    EVT_MENU( WXDOCVIEW_CONNECTWINDOW2, MyFrame::SetConnectWindow )
END_EVENT_TABLE()


MyFrame::MyFrame( a2dSplitterConnector<a2dDocumentViewScrolledWindow>* connector, wxFrame* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, const long style ) :
    EditorFrame( true, parent, NULL, -1, title, position, size, style  )
{
    wxMenu* windowconnect = new wxMenu;
    windowconnect->Append( WXDOCVIEW_CONNECTWINDOW1, _( "Window1" ), _( "Connect document to window 1" ), true );
    windowconnect->Append( WXDOCVIEW_CONNECTWINDOW2, _( "Window2" ), _( "Connect document to window 2" ), true );
    m_document_menu->Append( WXDOCVIEW_CONNECTWINDOW, _( "&Choose Connect Window" ), windowconnect, _( "connect documents to this window" ) );

    m_help_menu->Append( WXDOCVIEW_ABOUT11, _( "&About" ) );

    m_menu_bar->Check( WXDOCVIEW_CONNECTWINDOW1, true );

    SetStatusText( wxT( "Welcome to Split Frame Managed sample!" ) );

    m_splitter = new MySplitterWindow( this );

    DrawingView* view1 = new DrawingView();
    DrawingView* view2 = new DrawingView();

    //make a new document and add it to the document manager and view.
    DrawingDocument* doc = new DrawingDocument();
    doc->SetFilename( wxFileName( wxT( "dummy.drw" ) ) );
    doc->CreateCommandProcessor();
    view1->SetDocument( doc );
    view2->SetDocument( doc );
    doc->GetCommandProcessor()->Initialize();

    //drawer->SetDocument( document );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->AddCreatedDocument( doc );

    a2dDocumentViewScrolledWindow* outputwindow2;

    m_canvas = new a2dDocumentViewScrolledWindow( view1, m_splitter, -1 , wxPoint( 0, 0 ), size, wxNO_FULL_REPAINT_ON_RESIZE );
    m_canvas->SetCursor( wxCURSOR_PENCIL );

    outputwindow2 = new a2dDocumentViewScrolledWindow( view2, m_splitter, -1, wxPoint( 0, 0 ), size,  wxNO_FULL_REPAINT_ON_RESIZE );
    outputwindow2->SetCursor( wxCURSOR_PENCIL );

    m_splitter->SplitVertically( m_canvas, outputwindow2, 300 );

    //tell the connector what or the windows it needs to connect to the view.
    m_splitconnector = connector;
    m_splitconnector->Init( m_splitter, m_canvas );

    // Give it scrollbars
    m_canvas->SetScrollbars( 20, 20, 50, 50 );
    m_canvas->SetBackgroundColour( *wxWHITE );
    m_canvas->ClearBackground();

    //for the in edtor templates ( see EditFrame )
    m_singleconnector->SetDisplayWindow( m_canvas );

    // Give it scrollbars
    outputwindow2->SetScrollbars( 20, 20, 30, 30 );
    outputwindow2->SetBackgroundColour( *wxGREEN );
    outputwindow2->ClearBackground();

    // Make sure the document manager knows that this is the
    // current view.
    view1->Activate( true );
}

MyFrame::~MyFrame()
{
}

void MyFrame::OnCloseView( a2dCloseViewEvent& event )
{
    DrawingView* viewclosed = ( DrawingView* ) event.GetEventObject();
    wxASSERT_MSG( viewclosed->GetDisplayWindow() == m_splitter->GetWindow1() ||
                  viewclosed->GetDisplayWindow() == m_splitter->GetWindow2(), wxT( "wxDrawingConnector::OnCloseView Not a valid view to close." ) );

    if ( !event.GetVeto() && viewclosed )
    {
        a2dDocumentViewScrolledWindow* display = ( a2dDocumentViewScrolledWindow* ) viewclosed->GetDisplayWindow();
        display->Refresh();
    }
}


void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    bool oke = true;
    if ( event.CanVeto() )
    {

        a2dSmrtPtr<a2dView> view = m_splitter->GetWindow1()->GetView();
        if ( view )
        {
            oke = view->Close( !event.CanVeto() );
        }
        view = m_splitter->GetWindow2()->GetView();
        if ( view )
        {
            a2dObjectPtr keepalive = view.Get();
            oke = oke && view->Close( !event.CanVeto() );
        }

        if ( oke )
        {
            Destroy();
        }
        else
            event.Veto( true );
    }
    else
        Destroy();
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "Split Frame Two Views Demo\nAuthor: Klaas Holwerda" ), wxT( "About Split Frame" ) );
}

void MyFrame::OnMRUFile( wxCommandEvent& event )
{
    event.Skip();
}


void MyFrame::SetConnectWindow( wxCommandEvent& event )
{
    m_menu_bar->Check( WXDOCVIEW_CONNECTWINDOW1, false );
    m_menu_bar->Check( WXDOCVIEW_CONNECTWINDOW2, false );

    switch ( event.GetId() )
    {
        case WXDOCVIEW_CONNECTWINDOW1:
            m_splitconnector->Init( m_splitter, m_splitter->GetWindow1() );
            m_singleconnector->SetDisplayWindow( m_splitter->GetWindow1() );
            m_menu_bar->Check( WXDOCVIEW_CONNECTWINDOW1, true );
            break;
        case WXDOCVIEW_CONNECTWINDOW2:
            m_splitconnector->Init( m_splitter, m_splitter->GetWindow2() );
            m_singleconnector->SetDisplayWindow( m_splitter->GetWindow2() );
            m_menu_bar->Check( WXDOCVIEW_CONNECTWINDOW2, true );
            break;
        default:
            return;
    }
}

// ----------------------------------------------------------------------------
// MySplitterWindow
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( MySplitterWindow, wxSplitterWindow )
    EVT_SPLITTER_SASH_POS_CHANGED( -1, MySplitterWindow::OnPositionChanged )
    EVT_SPLITTER_SASH_POS_CHANGING( -1, MySplitterWindow::OnPositionChanging )

    EVT_SPLITTER_DCLICK( -1, MySplitterWindow::OnDClick )

    EVT_SPLITTER_UNSPLIT( -1, MySplitterWindow::OnUnsplit )
END_EVENT_TABLE()

MySplitterWindow::MySplitterWindow( wxFrame* parent )
    : wxSplitterWindow( parent, -1,
                        wxDefaultPosition, wxDefaultSize,
                        wxNO_FULL_REPAINT_ON_RESIZE | wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN )
{
    m_frame = parent;
}

void MySplitterWindow::OnPositionChanged( wxSplitterEvent& event )
{
    wxLogStatus( m_frame, _T( "Position has changed, now = %d (or %d)" ),
                 event.GetSashPosition(), GetSashPosition() );

    event.Skip();
}

void MySplitterWindow::OnPositionChanging( wxSplitterEvent& event )
{
    wxLogStatus( m_frame, _T( "Position is changing, now = %d (or %d)" ),
                 event.GetSashPosition(), GetSashPosition() );

    event.Skip();
}

void MySplitterWindow::OnDClick( wxSplitterEvent& event )
{
    m_frame->SetStatusText( _T( "Splitter double clicked" ), 1 );

    event.Skip();
}

void MySplitterWindow::OnUnsplit( wxSplitterEvent& event )
{
    m_frame->SetStatusText( _T( "Splitter unsplit" ), 1 );

    event.Skip();
}




