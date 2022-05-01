/*! \file docview/samples/tabframe/tabframe.cpp
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: tabframe.cpp,v 1.14 2009/09/04 18:31:31 titato Exp $
*/

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "tabframe.h"

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

    a2dGeneralGlobals->SetLogConnectedEvents( true );

    a2dDocumentCommandProcessor* docmanager = a2dDocviewGlobals->GetDocviewCommandProcessor();

    // the next is shared by the templates, they will relase it in the end.
    m_notebookconnector = new MyNoteBookConnector();

    a2dDocumentTemplate* doctemplatenew;

    // AUTO searches for ioHandler.
    doctemplatenew = new a2dDocumentTemplateAuto( _T( "All Files" ) , _T( "*.*" ) , _T( "" ) , _T( "" ) , _T( "DrawingDocument" ) ,
            CLASSINFO( DrawingDocument ), m_notebookconnector, a2dTemplateFlag::DEFAULT );
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
        m_notebookconnector,
        a2dTemplateFlag::DEFAULT, iohandlerFormatIn, iohandlerFormatOut );
    docmanager->AssociateDocTemplate( doctemplatenew );

    a2dViewTemplate* viewtemplatenew;

    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Drawing View" ),
        CLASSINFO( DrawingView ),
        m_notebookconnector,
        a2dTemplateFlag::DEFAULT );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    // Create a template relating drawing documents to statistics views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Statistics View" ),
        CLASSINFO( StatisticsView ),
        m_notebookconnector,
        a2dTemplateFlag::DEFAULT );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    DrawingView* view1 = new DrawingView();
    //make a new document and add it to the document manager and view.
    DrawingDocument* doc = new DrawingDocument();
    doc->SetFilename( wxFileName( wxT( "dummy.drw" ) ) );
    doc->CreateCommandProcessor();
    view1->SetDocument( doc );
    doc->GetCommandProcessor()->Initialize();

    a2dDocviewGlobals->GetDocviewCommandProcessor()->AddCreatedDocument( doc );


    //// Create the main frame window
    m_frame = new MyFrame( true, m_notebookconnector, view1,  NULL, -1, wxT( "Tabframe with docmanager" ),
                           wxPoint( 10, 110 ), wxSize( 500, 400 ), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE );
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
    EVT_ACTIVATE( MyFrame::OnActivate )
END_EVENT_TABLE()


MyFrame::MyFrame( bool parentFrame, MyNoteBookConnector* connector, DrawingView* view,
                  wxFrame* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, const long style ) :
    EditorFrame( parentFrame, parent, NULL, -1, title, position, size, style  )
{
    m_help_menu->Append( WXDOCVIEW_ABOUT11, _( "&About" ) );

    SetStatusText( wxT( "Welcome to TabFrame Managed sample!" ) );

    m_notebookconnector = connector;
    m_notebook = new MyNotebook( this );
    m_notebookconnector->Init( m_notebook );

    m_canvas = new a2dDocumentViewScrolledWindow( view, m_notebook, -1 , wxPoint( 0, 0 ), size, wxNO_FULL_REPAINT_ON_RESIZE );
    m_canvas->SetCursor( wxCURSOR_PENCIL );

    m_notebook->AddPage( m_canvas, wxT( "window1" ), true );

    // Give it scrollbars
    m_canvas->SetScrollbars( 20, 20, 50, 50 );
    m_canvas->SetBackgroundColour( *wxWHITE );
    m_canvas->ClearBackground();

    //for the in editor templates ( see EditFrame )
    m_singleconnector->SetDisplayWindow( m_canvas );

    // Make sure the document manager knows that this is the
    // current view.
    view->Activate( true );
}

MyFrame::~MyFrame()
{
}

void MyFrame::OnActivate( wxActivateEvent& event )
{
    // in case of more then one top frame make the notebook connector (of which there is only one
    // connected to the document and view templates), have this active frame its m_notebook as the
    // notebook to connect new views to it.
    m_notebookconnector->Init( m_notebook );

    event.Skip(); //skip to base OnActivate Handler which sets proper focus to child window(s)
}


void MyFrame::OnCloseView( a2dCloseViewEvent& event )
{
    DrawingView* viewclosed = ( DrawingView* ) event.GetEventObject();

    if ( !event.GetVeto() && viewclosed )
    {
        a2dDocumentViewScrolledWindow* display = ( a2dDocumentViewScrolledWindow* ) viewclosed->GetDisplayWindow();

        unsigned int i;
        for ( i = 0 ; i < m_notebook->GetPageCount(); i++ )
        {
            if ( m_notebook->GetPage( i ) == display )
            {
                if ( display == m_canvas )
                    m_canvas = NULL;
                m_notebook->DeletePage( i );
                break;
            }
        }
    }

    event.Skip();
}


void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    // in case of parent frame close all child frames, via closing document -> views -> frames.
    if ( GetIsParentFrame() )
    {
        if ( event.CanVeto() )
        {
            if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear( !event.CanVeto() ) )
            {
                // see down here for Destroy();
            }
            else
                event.Veto( true );
        }
        else
        {
            a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear( true );
            // see down here for Destroy();
        }
    }
    else
        event.Skip();

    // normal childs frame of the parent frame, remove all notebook pages and close views.
    // same for parent frma itself
    bool oke = true;
    if ( event.CanVeto() )
    {
        unsigned int i;
        for ( i = 0 ; i < m_notebook->GetPageCount(); i++ )
        {
            oke = oke && ( ( a2dDocumentViewScrolledWindow* ) m_notebook->GetPage( m_notebook->GetSelection() ) )->GetView()->Close( !event.CanVeto() );

            if ( !oke )
                break;
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
    ( void )wxMessageBox( wxT( "NoteBook Frame Two Views Demo\nAuthor: Klaas Holwerda" ), wxT( "About NoteBook Frame" ) );
}

void MyFrame::OnMRUFile( wxCommandEvent& event )
{
    event.Skip();
}


// ----------------------------------------------------------------------------
// MyNotebook
// ----------------------------------------------------------------------------
#ifdef AUI_NOTEBOOK
const int wxAuiBaseTabCtrlId = 5380;
BEGIN_EVENT_TABLE( MyNotebook, NoteBookType )

    EVT_COMMAND_RANGE( wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId + 500,
                       wxEVT_COMMAND_AUINOTEBOOK_BEGIN_DRAG,
                       MyNotebook::OnTabBeginDrag )
    EVT_COMMAND_RANGE( wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId + 500,
                       wxEVT_COMMAND_AUINOTEBOOK_END_DRAG,
                       MyNotebook::OnTabEndDrag )
    EVT_COMMAND_RANGE( wxAuiBaseTabCtrlId, wxAuiBaseTabCtrlId + 500,
                       wxEVT_COMMAND_AUINOTEBOOK_DRAG_MOTION,
                       MyNotebook::OnTabDragMotion )
#ifdef AUI_NOTEBOOK
    EVT_AUINOTEBOOK_PAGE_CHANGED( -1, MyNotebook::OnChangedPage )
#else
    EVT_NOTEBOOK_PAGE_CHANGED( -1, MyNotebook::OnChangedPage )
#endif
END_EVENT_TABLE()
#else
BEGIN_EVENT_TABLE( MyNotebook, NoteBookType )
    EVT_NOTEBOOK_PAGE_CHANGED( -1, MyNotebook::OnChangedPage )
END_EVENT_TABLE()
#endif

MyNotebook::MyNotebook( a2dDocumentFrame* parent )
    : NoteBookType( parent, -1, wxDefaultPosition, wxDefaultSize )
{
    m_dragToFrame = true;
    m_dragViewFrame = NULL;
}

#ifdef AUI_NOTEBOOK
void MyNotebook::OnChangedPage( wxAuiNotebookEvent& evt )
#else
void MyNotebook::OnChangedPage( wxNotebookEvent& evt )
#endif
{
    a2dView* view = ( ( a2dDocumentViewScrolledWindow* )GetPage( evt.GetSelection() ) )->GetView();

    if ( view )
        view->Activate( true );

    a2dDocumentFrame* parent = wxStaticCast( m_parent, a2dDocumentFrame );

    evt.Skip();
}

#ifdef AUI_NOTEBOOK
void MyNotebook::OnTabBeginDrag( wxCommandEvent& event )
{
    m_dragViewFrame = NULL;
    event.Skip();
}


bool MyNotebook::CaptionHit()
{
    wxPoint screen_pt = ::wxGetMousePosition();
    wxPoint client_pt = ScreenToClient( screen_pt );

    wxWindow* hit_wnd = ::wxFindWindowAtPoint( screen_pt );
    if ( hit_wnd )
    {
        wxAuiTabCtrl* tabc = GetTabCtrlFromPoint( client_pt );
        if ( tabc == hit_wnd )
            return true;
    }
    return false;
}

void MyNotebook::OnTabDragMotion( wxCommandEvent&  evt )
{
    if ( CaptionHit() )
        wxLogDebug( wxT( "in caption" ) );

    wxPoint screen_pt = ::wxGetMousePosition();
    wxPoint client_pt = ScreenToClient( screen_pt );


    if ( m_dragToFrame )
    {
        if ( !m_dragViewFrame )
        {
            wxHitTest hit;// = this->HitTest( client_pt );
            if ( hit == wxHT_WINDOW_OUTSIDE )
            {
                //wxLogDebug( "out" );
                a2dDocumentViewScrolledWindow* pagewindow = ( ( a2dDocumentViewScrolledWindow* )GetPage( evt.GetSelection() ) );
                a2dView* viewpage = pagewindow->GetView();
                DrawingView* viewNew = new DrawingView();
                viewNew->SetDocument( viewpage->GetDocument() );

                m_dragViewFrame = new MyFrame( false, wxGetApp().m_notebookconnector, ( DrawingView* ) viewNew,  NULL, -1, wxT( "Tabframe with docmanager" ),
                                               wxPoint( screen_pt.x - 5, screen_pt.y - 5 ), wxSize( 500, 400 ), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE );

                //m_dragViewFrame = new EditorFrame( false, (a2dDocumentFrame*)GetParent(), (DrawingView*) viewNew, -1,
                //         wxT("Frame with docmanager"), wxPoint( screen_pt.x -5, screen_pt.y -5 ), wxSize(500,500), wxDEFAULT_FRAME_STYLE );
                m_dragViewFrame->SetTransparent( 155 );
                m_dragViewFrame->Show();
                m_dragViewFrame->Raise();
                m_dragViewFrame->SetFocus();
            }
            else
            {
                evt.Skip();
                //wxLogDebug( "in" );
            }
        }
        else
        {
            m_dragViewFrame->SetPosition( wxPoint( screen_pt.x - 5, screen_pt.y - 5 ) );
        }
    }
    else
        evt.Skip();
}

void MyNotebook::OnTabEndDrag( wxCommandEvent&  evt )
{
    if ( m_dragToFrame )
    {
        if ( m_dragViewFrame )
        {
            m_dragViewFrame->SetTransparent( 255 );
            m_dragViewFrame = NULL;
            m_mgr.HideHint();
            DeletePage( evt.GetSelection() );
        }
        else
            evt.Skip();
    }
    else
        evt.Skip();
}
#endif
