/*! \file docview/samples/singleframe/singleframe.cpp
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: singleframe.cpp,v 1.3 2008/10/03 15:14:38 titato Exp $
*/

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "singleframe.h"

#include "wx/colordlg.h"
#include <wx/wfstream.h>

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#include "docdialog.h"
#include "doc.h"

IMPLEMENT_APP( MyApp )


BEGIN_EVENT_TABLE( a2dSingleConnector, a2dScrolledWindowViewConnector )
    EVT_POST_CREATE_VIEW( a2dSingleConnector::OnPostCreateView )
    EVT_DISCONNECT_VIEW( a2dSingleConnector::OnDisConnectView )
END_EVENT_TABLE()

a2dSingleConnector::a2dSingleConnector()
    : a2dScrolledWindowViewConnector()
{
}

void a2dSingleConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    if ( !m_display )
        return; //view will not be connected to a window ( but document and view already are available).

    //next to what is done in base class, set the view to the display.
    DrawingView* view = ( DrawingView* ) event.GetView();

    m_display->SetView( view );
    view->Update();

    m_view = view;
}

void a2dSingleConnector::OnDisConnectView(  a2dTemplateEvent& event )
{
    a2dSmrtPtr<a2dView> theViewToDisconnect = ( a2dView* ) event.GetEventObject();
    theViewToDisconnect->Close( true );

    if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->GetBusyExit() )
    {
        a2dView* view = NULL;
        if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().size() > 1 )
        {
            a2dDocumentList::const_reverse_iterator iter = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().rbegin();
            iter++;
            a2dDocument* doc = ( *iter );

            a2dViewList allviews;
            doc->ReportViews( &allviews );

            if ( !allviews.size() )
            {
                a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateView( doc, _T( "Drawing View" ) )->Update();
                doc->ReportViews( &allviews );
            }
            view = allviews.front();

            view->SetDisplayWindow( m_display );
            view->Update();
            m_display->SetView( view );

        }
        else
        {

            view = new DrawingView();
            a2dViewTemplate* viewtempl;
            viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates().front();
            view->SetViewTemplate( viewtempl );

            a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateDocuments( wxT( "dummy" ), a2dREFDOC_NEW );
            a2dDocument* doc = ( a2dDocument* ) a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().back().Get();
            view->SetDocument( doc );

            m_display->SetView( view );
            view->SetDisplayWindow( m_display );
            view->Update();
        }
        view->Activate( true );
    }
    else
        event.Skip();
}

//-----------------------------------------------------------------------------
// MyApp
//-----------------------------------------------------------------------------

bool MyApp::OnInit()
{
    //_CrtSetBreakAlloc(3375);

    a2dDocumentCommandProcessor* docmanager = a2dDocviewGlobals->GetDocviewCommandProcessor();

    m_singleconnector = new a2dSingleConnector();
    m_singleconnector->SetInitialPosition( wxPoint( 50, 40 ) );
    m_singleconnector->SetInitialSize( wxSize( 200, 300 ) );

    a2dDocumentTemplate* doctemplatenew;

    doctemplatenew = new a2dDocumentTemplateAuto( _T( "All Files" ) , _T( "*.*" ) , _T( "" ) , _T( "" ) , _T( "DrawingDocument" ) ,
            CLASSINFO( DrawingDocument ), m_singleconnector, a2dTemplateFlag::DEFAULT );
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
        m_singleconnector,
        a2dTemplateFlag::DEFAULT, iohandlerFormatIn, iohandlerFormatOut );
    docmanager->AssociateDocTemplate( doctemplatenew );

    a2dViewTemplate* viewtemplatenew;

    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Drawing View" ),
        CLASSINFO( DrawingView ),
        m_singleconnector,
        a2dTemplateFlag::DEFAULT );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    // Create a template relating drawing documents to statistics views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Statistics View" ),
        CLASSINFO( StatisticsView ),
        m_singleconnector,
        a2dTemplateFlag::DEFAULT );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    m_multiconnector = new a2dMultiFrameDrawingConnector();
    m_multiconnector->SetInitialPosition( wxPoint( 50, 40 ) );
    m_multiconnector->SetInitialSize( wxSize( 200, 300 ) );

    a2dViewTemplate* viewtemplatenew2;

    // Create a template relating drawing documents to drawing views
    viewtemplatenew2 = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Drawing View Frame" ),
        CLASSINFO( DrawingView ),
        m_multiconnector,
        a2dTemplateFlag::DEFAULT, wxSize( 100, 400 ) );
    docmanager->AssociateViewTemplate( viewtemplatenew2 );

    // Create a template relating drawing documents to text views
    viewtemplatenew2 = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Text View Frame" ),
        CLASSINFO( TextView ),
        m_multiconnector,
        a2dTemplateFlag::DEFAULT, wxSize( 200, 300 ) );
    docmanager->AssociateViewTemplate( viewtemplatenew2 );

    // Create a template relating drawing documents to statistics views
    viewtemplatenew2 = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Statistics View Frame" ),
        CLASSINFO( StatisticsView ),
        m_multiconnector,
        a2dTemplateFlag::DEFAULT, wxSize( 200, 300 ) );
    docmanager->AssociateViewTemplate( viewtemplatenew2 );

    //// Create the main frame window
    m_frame = new MyFrame( NULL, -1, wxT( "Single with docmanager" ), wxPoint( 10, 110 ), wxSize( 500, 400 ), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE );
    m_frame->Show( true );
    m_singleconnector->SetDisplayWindow( m_frame->GetCanvas() );

    m_multiconnector->Init( m_frame, true );

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


BEGIN_EVENT_TABLE( MyFrame, EditorFrame )
//    EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, MyFrame::OnMRUFile)
    EVT_MENU( WXDOCVIEW_ABOUT, MyFrame::OnAbout )
    EVT_CLOSE_VIEW( MyFrame::OnCloseView )
END_EVENT_TABLE()


MyFrame::MyFrame( wxFrame* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, const long style ) :
    EditorFrame( true, parent, new DrawingView(),  -1, title, position, size, style  )

{

    //DrawingDocument* document = (DrawingDocument* ) a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateDocument(wxT("dummy"), a2dREFDOC_NEW );

    SetStatusText( wxT( "Welcome to Single Frame Managed sample!" ) );

    DrawingView* view = ( DrawingView* ) m_view.Get();
    a2dViewTemplate* viewtempl;
    viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates().front();
    view->SetViewTemplate( viewtempl );

    //make a new document and add it to the document manager and view.
    DrawingDocument* doc = new DrawingDocument();
    //drawer->SetDocument( document );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->AddCreatedDocument( doc );

    view->SetDocument( doc );
    m_canvas->SetView( view );

    doc->GetCommandProcessor()->Initialize();

    // Make sure the document manager knows that this is the
    // current view.
    view->Activate( true );
    Enable();


}


MyFrame::~MyFrame()
{
}

void MyFrame::OnCloseView( a2dCloseViewEvent& event )
{
    a2dView* view = ( a2dView* ) event.GetEventObject();

    //there is no need to handle the parent frame differently

    view->SetDisplayWindow( NULL );

    m_canvas->Refresh();
    Refresh();
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "Single Frame Demo\nAuthor: Klaas Holwerda" ), wxT( "Single Frame" ) );
}

/*
void MyFrame::OnMRUFile(wxCommandEvent& event)
{
    int n = event.GetId() - wxID_FILE1;  // the index in MRU list
    wxString filename(a2dDocviewGlobals->GetDocviewCommandProcessor()->GetHistoryFile(n));
    if ( !filename.IsEmpty() )
    {
        a2dDocument *doc = NULL;

        wxList::compatibility_iterator node = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().First();
        while (node)
        {
            doc = (a2dDocument *)node->Data();

            if ( doc->GetFilename().GetFullPath() == filename )
                break;
            node = node->Next();
        }

        if ( doc )
            m_connector->ConnectDocument( (DrawingDocument*) doc );
    }
}
*/








