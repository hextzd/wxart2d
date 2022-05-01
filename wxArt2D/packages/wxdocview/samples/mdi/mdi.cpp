/*! \file docview/samples/mdi/mdi.cpp
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: mdi.cpp,v 1.6 2009/09/26 19:01:19 titato Exp $
*/

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "mdi.h"

#if wxCHECK_VERSION(2, 5, 0)
#if !WXWIN_COMPATIBILITY_2_4
#include "wx/numdlg.h"
#endif
#endif

#include "wx/colordlg.h"
#include <wx/wfstream.h>

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#include "doc.h"
#include "docdialog.h"

IMPLEMENT_APP( MyApp )

// ----------------------------------------------------------------------------
// wxMdiFrameDrawingConnector
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( wxMdiFrameDrawingConnector, a2dViewConnector )
    EVT_POST_CREATE_DOCUMENT( wxMdiFrameDrawingConnector::OnPostCreateDocument )
    EVT_POST_CREATE_VIEW( wxMdiFrameDrawingConnector::OnPostCreateView )
END_EVENT_TABLE()

wxMdiFrameDrawingConnector::wxMdiFrameDrawingConnector( wxClassInfo* EditorClassInfo )
    : a2dViewConnector()
{
    m_docframe = ( a2dDocumentMDIParentFrame* ) NULL;
    m_editorClassInfo = EditorClassInfo;
}

wxMdiFrameDrawingConnector::~wxMdiFrameDrawingConnector()
{
}

void wxMdiFrameDrawingConnector::OnPostCreateDocument( a2dTemplateEvent& event )
{
    a2dViewTemplate* viewtempl;
    viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->SelectViewType( event.GetDocument(),
                m_viewTemplates.size() ? m_viewTemplates : a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates() );

    if ( !viewtempl )
    {
        wxLogMessage( wxT( "No view available for this document in wxMdiFrameDrawingConnector" ) );
        return;
    }

    if ( !viewtempl->CreateView( event.GetDocument(), event.GetFlags() ) )
    {
        wxLogMessage( wxT( "wxMdiFrameDrawingConnector no view was created" ) );
        return;
    }
}

void wxMdiFrameDrawingConnector::Init( a2dDocumentMDIParentFrame* frame )
{
    m_docframe = frame;
}

void wxMdiFrameDrawingConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    a2dView* view = event.GetView();

    a2dDocument* doc = event.GetView()->GetDocument();

    if ( view->GetViewTypeName() == wxT( "Text View Frame" ) )
    {
        wxMDITextViewFrame* textFrame = new wxMDITextViewFrame( ( TextView* ) view, GetInitialPosition(),
                view->GetViewTemplate()->GetInitialSize(), m_docframe );
        textFrame->SetTitle( wxT( "TextEditView" ) );
        textFrame->SetView( view );
        view->SetDisplayWindow( textFrame->m_textdisplay );
        textFrame->Enable();
        textFrame->Show( true );
    }
    else if ( view->GetViewTypeName() == wxT( "Statistics View Frame" ) )
    {
        MyFrame* viewFrame = new MyFrame( m_docframe, view,
                                          -1, view->GetViewTypeName(),
                                          GetInitialPosition(),
                                          view->GetViewTemplate()->GetInitialSize(), //WATCH get it from the viewtemplate
                                          GetInitialStyle()
                                        );
        viewFrame->Show( true );

    }
    else if ( view->GetViewTypeName() == wxT( "Drawing View Frame" ) )
    {
        MDIEditorFrame* viewFrame;
        if ( m_editorClassInfo )
        {
            viewFrame = ( MDIEditorFrame* ) m_editorClassInfo->CreateObject();
            viewFrame->Create( m_docframe, ( DrawingView* ) view, -1,
                               wxT( "Single with docmanager" ), GetInitialPosition(), GetInitialSize(), wxDEFAULT_FRAME_STYLE );
        }
        else
            viewFrame = new MDIEditorFrame( m_docframe, ( DrawingView* ) view, -1,
                                            wxT( "Single with docmanager" ), GetInitialPosition(), GetInitialSize(), wxDEFAULT_FRAME_STYLE );

        doc->GetCommandProcessor()->Initialize();

        viewFrame->GetCanvas()->SetBackgroundColour( wxColour( 12, 240, 230 ) );
        view->SetDisplayWindow( viewFrame->GetCanvas() );
        viewFrame->Enable();
        viewFrame->Show( true );
    }


    m_docframe->Refresh();
}


//-----------------------------------------------------------------------------
// MyApp
//-----------------------------------------------------------------------------

bool MyApp::OnInit()
{
    a2dDocumentCommandProcessor* docmanager = a2dDocviewGlobals->GetDocviewCommandProcessor();

    a2dViewTemplate* viewtemplatenew;

    m_mainconnector = new wxMdiFrameDrawingConnector( NULL );
    m_mainconnector->SetInitialPosition( wxPoint( 50, 40 ) );
    m_mainconnector->SetInitialSize( wxSize( 200, 300 ) );

    a2dDocumentTemplate* doctemplatenew;

    // AUTO searches for ioHandler.
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
    m_frame = new MyParentFrame( NULL, wxT( "MdiFrame with docmanager" ), wxPoint( 10, 110 ), wxSize( 500, 400 ), wxNO_FULL_REPAINT_ON_RESIZE | wxDEFAULT_FRAME_STYLE  );
    m_mainconnector->Init( m_frame );
    m_frame->Show( true );

    SetTopWindow( m_frame );

    return true;
}


int MyApp::OnExit()
{
    m_mainconnector->DisassociateViewTemplates();
    return 0;
}

wxMDITextViewFrame::wxMDITextViewFrame( TextView* view, const wxPoint& pos, const wxSize& size, wxMDIParentFrame* parent )
    :  a2dDocumentMDIChildFrame( parent, NULL, -1, wxT( "Child Frame" ), pos, size, wxDEFAULT_FRAME_STYLE )
{

#ifdef __WXMSW__
    SetIcon( wxString( wxT( "chrt_icn" ) ) );
#endif

    m_view = view;

    Centre( wxBOTH );

    SetTitle( wxT( "TextView" ) );

    m_textdisplay = new wxTextCtrl( this, -1, wxT( "empty" ), wxDefaultPosition, wxDefaultSize, wxTE_READONLY  | wxTE_MULTILINE );

    m_view->SetDisplayWindow( m_textdisplay );
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE( MyFrame, a2dDocumentMDIChildFrame )
//    EVT_MENU(WXDOCVIEW_ABOUT, MyFrame::OnAbout)
END_EVENT_TABLE()

MyFrame::MyFrame( a2dDocumentMDIParentFrame* parent,
                  a2dView* view,
                  wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, const long style ) :

    a2dDocumentMDIChildFrame( parent, NULL, -1, title, position, size, style  )

{
    m_canvas = new a2dDocumentViewScrolledWindow( view, this, -1, wxPoint( 0, 0 ), size, wxNO_FULL_REPAINT_ON_RESIZE );
    m_canvas->SetCursor( wxCURSOR_PENCIL );

    // Give it scrollbars
    m_canvas->SetScrollbars( 20, 20, 50, 50 );
    m_canvas->SetBackgroundColour( *wxWHITE );
    m_canvas->ClearBackground();
}

MyFrame::~MyFrame()
{
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "MDI Single Frame Demo\nAuthor: Klaas Holwerda" ), wxT( "About MDI Frame" ) );
}


//------------------------------------------------------------------------------
// MyParentFrame
//------------------------------------------------------------------------------


BEGIN_EVENT_TABLE( MyParentFrame, a2dDocumentMDIParentFrame )
    EVT_MENU( wxID_EXIT, MyParentFrame::OnExit )
    EVT_MENU( WXDOCVIEW_ABOUT, MyParentFrame::OnAbout )
    EVT_CLOSE( MyParentFrame::OnCloseWindow )
END_EVENT_TABLE()


MyParentFrame::MyParentFrame( wxFrame* parent, const wxString& title,
                              const wxPoint& position, const wxSize& size, const long style ) :
    a2dDocumentMDIParentFrame( parent, -1, title, position, size, style )

{
    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( wxT( "Welcome to Multiple Frame Managed Views sample!" ) );

    SetIcon( wxICON( mondrian ) );
}

void MyParentFrame::CreateMyMenuBar()
{

    wxMenuBar* menu_bar = new wxMenuBar();

    m_file_menu = new wxMenu;

    AddCmdMenu( m_file_menu, CmdMenu_FileNew() );
    AddCmdMenu( m_file_menu, CmdMenu_FileOpen() );
    AddCmdMenu( m_file_menu, CmdMenu_FileClose() );
    AddCmdMenu( m_file_menu, CmdMenu_FileSave() );
    AddCmdMenu( m_file_menu, CmdMenu_FileSaveAs() );
    AddCmdMenu( m_file_menu, CmdMenu_Print() );
    m_file_menu->AppendSeparator();
    AddCmdMenu( m_file_menu, CmdMenu_PrintView() );
    AddCmdMenu( m_file_menu, CmdMenu_PreviewView() );
    AddCmdMenu( m_file_menu, CmdMenu_PrintDocument() );
    AddCmdMenu( m_file_menu, CmdMenu_PreviewDocument() );
    AddCmdMenu( m_file_menu, CmdMenu_PrintSetup() );

    m_file_menu->AppendSeparator();
    m_file_menu->Append( wxID_EXIT, _( "E&xit" ) );

    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( m_file_menu );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( WXDOCVIEW_ABOUT, _( "&About" ) );

    menu_bar->Append( m_file_menu, _( "&File" ) );
    menu_bar->Append( help_menu, _( "&Help" ) );

    SetMenuBar( menu_bar );

}

MyParentFrame::~MyParentFrame()
{
}

void MyParentFrame::Refresh( bool eraseBackground, const wxRect* rect )
{
}

void MyParentFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "MDI Frame Demo\nAuthor: Klaas Holwerda" ), wxT( "About MDI editor Frame" ) );
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

//------------------------------------------------------------------------------
// MDIEditorFrame
//------------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( MDIEditorFrame, a2dDocumentMDIChildFrame )


BEGIN_EVENT_TABLE( MDIEditorFrame, a2dDocumentMDIChildFrame )
    EVT_MENU( wxID_EXIT, MDIEditorFrame::OnExit )
    EVT_CLOSE( MDIEditorFrame::OnCloseWindow )
    EVT_MENU( EditorFrame_ABOUT, MDIEditorFrame::OnAbout )
    EVT_MENU( EditorFrame_DRAWINGCOLOUR, MDIEditorFrame::SelectDrawingColour )
    EVT_MENU( EditorFrame_DRAWINGWIDTH, MDIEditorFrame::SetDrawingWidth )
    EVT_MENU( EditorFrame_DRAWINGSCALE, MDIEditorFrame::SetDrawingZoom )
    EVT_MENU( EditorFrame_CONNECTDOC, MDIEditorFrame::ChooseDocumentForView )
    EVT_MENU( EditorFrame_WINDOWWVIEW, MDIEditorFrame::CreateInWindowView )
    EVT_MENU( EditorFrame_ADDVIEW, MDIEditorFrame::CreateAddView )
    EVT_MENU( EditorFrame_CHOOSEVIEW, MDIEditorFrame::ChooseViewFromDocument )
    EVT_ACTIVATE( MDIEditorFrame::OnActivate )
    EVT_UPDATE_UI( EditorFrame_DRAWINGCOLOUR, MDIEditorFrame::OnSelectDrawingColour )
    EVT_UPDATE_UI( EditorFrame_DRAWINGWIDTH, MDIEditorFrame::OnSetDrawingWidth )
    EVT_UPDATE_UI( EditorFrame_DRAWINGSCALE, MDIEditorFrame::OnSetDrawingZoom )
    EVT_MENU( wxID_UNDO, MDIEditorFrame::OnUndo )
    EVT_MENU( wxID_REDO, MDIEditorFrame::OnRedo )
    EVT_MENUSTRINGS( MDIEditorFrame::OnSetmenuStrings )
END_EVENT_TABLE()

MDIEditorFrame::MDIEditorFrame( wxMDIParentFrame* parent,
                                DrawingView* view,  wxWindowID id, const wxString& title,
                                const wxPoint& position, const wxSize& size, const long style ) :
    a2dDocumentMDIChildFrame( parent, view, -1, title, position, size, style )
{
    Init( view );
}

bool MDIEditorFrame::Create( wxMDIParentFrame* parent,
                             DrawingView* view, wxWindowID id, const wxString& title,
                             const wxPoint& position, const wxSize& size, const long style )
{
    bool res = a2dDocumentMDIChildFrame::Create( parent, view, -1, title, position, size, style );
    Init( view );
    return res;
}

void MDIEditorFrame::Init( DrawingView* view )
{
    CreateMyMenuBar();
    CreateStatusBar( 2 );
    SetStatusText( wxT( "Multi purpose MDIEditorFrame for DrawingView's" ) );

    SetIcon( wxICON( mondrian ) );

    if ( view )
    {
        //m_canvas = new a2dDocumentViewWindow(view, this, -1, wxPoint(0, 0), size, 0);
        m_canvas = new a2dDocumentViewScrolledWindow( view, this, -1, wxPoint( 0, 0 ), wxDefaultSize, wxDOUBLE_BORDER | wxNO_FULL_REPAINT_ON_RESIZE );
        m_canvas->SetCursor( wxCURSOR_PENCIL );
        // Give it scrollbars
        m_canvas->SetScrollbars( 20, 20, 50, 50 );
        m_canvas->SetBackgroundColour( *wxGREEN );

        m_canvas->ClearBackground();
    }

    m_singleconnector = new a2dScrolledWindowViewConnector();
    m_singleconnector->SetDisplayWindow( m_canvas );

    // Create a template relating drawing documents to their files
    m_editordoctemplatenew = new a2dDocumentTemplate(
        wxT( "DRW Drawing" ),
        wxT( "*.drw" ),
        wxT( "" ),
        wxT( "drw" ),
        wxT( "DrawingDocument" ),
        CLASSINFO( DrawingDocument ),
        m_singleconnector,
        a2dTemplateFlag::DEFAULT );

    // Create a template relating drawing documents to their views
    a2dViewTemplate* draw = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Drawing View In Window" ),
        CLASSINFO( DrawingView ),
        m_singleconnector,
        a2dTemplateFlag::DEFAULT );

    // Create a template relating drawing documents to statistics views
    a2dViewTemplate* stat = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Statistics View In Window" ),
        CLASSINFO( StatisticsView ),
        m_singleconnector,
        a2dTemplateFlag::DEFAULT );

    m_singleconnector->AssociateViewTemplate( draw );
    m_singleconnector->AssociateViewTemplate( stat );
}

void MDIEditorFrame::CreateMyMenuBar()
{

    m_menu_bar = new wxMenuBar();

    m_file_menu = new wxMenu;

    AddCmdMenu( m_file_menu, CmdMenu_FileNew() );
    AddCmdMenu( m_file_menu, CmdMenu_FileOpen() );
    AddCmdMenu( m_file_menu, CmdMenu_FileClose() );
    AddCmdMenu( m_file_menu, CmdMenu_FileSave() );
    AddCmdMenu( m_file_menu, CmdMenu_FileSaveAs() );
    AddCmdMenu( m_file_menu, CmdMenu_Print() );
    m_file_menu->AppendSeparator();
    AddCmdMenu( m_file_menu, CmdMenu_PrintView() );
    AddCmdMenu( m_file_menu, CmdMenu_PreviewView() );
    AddCmdMenu( m_file_menu, CmdMenu_PrintDocument() );
    AddCmdMenu( m_file_menu, CmdMenu_PreviewDocument() );
    AddCmdMenu( m_file_menu, CmdMenu_PrintSetup() );

    m_file_menu->AppendSeparator();
    m_file_menu->Append( wxID_EXIT, _( "E&xit" ) );

    m_edit_menu = new wxMenu;
    m_edit_menu->Append( wxID_UNDO, _( "&Undo" ) );
    m_edit_menu->Append( wxID_REDO, _( "&Redo" ) );
    m_edit_menu->AppendSeparator();
    m_edit_menu->Append( EditorFrame_CUT, _( "&Cut last segment" ) );

    m_drawing_menu = new wxMenu;
    m_drawing_menu->Append( EditorFrame_DRAWINGCOLOUR, _( "&Drawing Colour" ), _( "set drawing colour for current view" ) );
    m_drawing_menu->Append( EditorFrame_DRAWINGWIDTH, _( "&Drawing Width" ), _( "set drawing width for current view" ) );
    m_drawing_menu->Append( EditorFrame_DRAWINGSCALE, _( "&Drawing Zoom" ), _( "set drawing zoom factor" ) );

    m_view_menu = new wxMenu;
    m_view_menu->Append( EditorFrame_WINDOWWVIEW, _( "&Add Editor view in window" ), _( "create a choosen view specific to this frame using current view its document and window" ) );
    m_view_menu->Append( EditorFrame_ADDVIEW, _( "&Add Connector view" ), _( "create an extra choosen view via docmanager using current view its document" ) );
    m_view_menu->Append( EditorFrame_CHOOSEVIEW, _( "&Choose View" ), _( "choose view from the document" ) );

    m_document_menu = new wxMenu;
    m_document_menu->Append( EditorFrame_CONNECTDOC, _( "&Choose Document" ), _( "set document to a view" ) );

    m_help_menu = new wxMenu;
    m_help_menu->Append( EditorFrame_ABOUT, _( "&About" ) );

    m_menu_bar->Append( m_file_menu, _( "&File" ) );
    m_menu_bar->Append( m_edit_menu, _( "&Edit" ) );
    m_menu_bar->Append( m_drawing_menu, _( "&Drawing" ) );
    m_menu_bar->Append( m_view_menu, _( "&Extra view" ) );
    m_menu_bar->Append( m_document_menu, _( "&Document" ) );
    m_menu_bar->Append( m_help_menu, _( "&Help" ) );

    SetMenuBar( m_menu_bar );

}

MDIEditorFrame::~MDIEditorFrame()
{
    m_singleconnector->DisassociateViewTemplates();
}

void MDIEditorFrame::OnCloseWindow( wxCloseEvent& event )
{
    a2dDocument* doc = NULL;
    if ( m_view )
        doc = m_view->GetDocument();

    a2dDocumentMDIChildFrame::OnCloseWindow( event );

    if ( !event.CanVeto() )
    {
    }
}

void MDIEditorFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "Single Frame Demo\nAuthor: Klaas Holwerda" ), wxT( "About Multi Frame" ) );
}

void MDIEditorFrame::OnExit( wxCommandEvent& event )
{
    Close( true );
}

void MDIEditorFrame::OnActivate( wxActivateEvent& event )
{
    if ( !m_view )
        return;

    m_view->Activate( true );
    //a2dDocument* doc = m_view ->GetDocument();
}

void MDIEditorFrame::OnSetmenuStrings( a2dCommandProcessorEvent& event )
{
    if ( event.GetEventObject() == m_view->GetDocument()->GetCommandProcessor() )
    {
        m_edit_menu->SetLabel( wxID_UNDO, event.GetUndoMenuLabel() );
        m_edit_menu->Enable( wxID_UNDO, event.CanUndo() );

        m_edit_menu->SetLabel( wxID_REDO, event.GetRedoMenuLabel() );
        m_edit_menu->Enable( wxID_REDO, event.CanRedo() );
    }
}

void MDIEditorFrame::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( !doc )
        return;
    if ( doc->GetCommandProcessor() )
        doc->GetCommandProcessor()->Undo();
}

void MDIEditorFrame::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( !doc )
        return;
    if ( doc->GetCommandProcessor() )
        doc->GetCommandProcessor()->Redo();
}

void MDIEditorFrame::SelectDrawingColour( wxCommandEvent& event )
{
    wxColour col;
    wxColourData data;
    wxColourDialog dialog( this, &data );

    a2dView* backup = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView();

    if ( dialog.ShowModal() == wxID_OK )
    {
        col = dialog.GetColourData().GetColour();
    }

    if ( wxDynamicCast( backup, DrawingView ) )
        ( ( DrawingView* ) backup )->SetDrawingColour( col );
}

void MDIEditorFrame::SetDrawingWidth( wxCommandEvent& event )
{
    a2dView* backup = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView();

    int currentwidth = ( ( DrawingView* ) backup )->GetDrawingWidth();

    long w = wxGetNumberFromUser( _( "Give Pen Width:" ), wxT( "pen width" ), wxT( "pen width" ), currentwidth, 0, 10000 );

    if ( wxDynamicCast( backup, DrawingView ) )
        ( ( DrawingView* )backup )->SetDrawingWidth( w );
}

void MDIEditorFrame::SetDrawingZoom( wxCommandEvent& event )
{
    DrawingView* v;
    v = wxDynamicCast( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView(), DrawingView );
    if ( v )
    {
        long z = wxGetNumberFromUser( _( "set zoom factor " ), _( "(negative means < 1):" ), wxT( "DoodleZoom" ),
                                      ( int ) v->GetScaler(), -30, 30, this );
        if ( z < 0 )
            v->SetScaler( 1 / ( double ) - z );
        else
            v->SetScaler( z );
        v->Update();
    }
}

// Handler for UI update command
void MDIEditorFrame::OnSelectDrawingColour( wxUpdateUIEvent& event )
{
    event.Enable( 0 != wxDynamicCast( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView(), DrawingView ) );
}

void MDIEditorFrame::OnSetDrawingWidth( wxUpdateUIEvent& event )
{
    event.Enable( 0 != wxDynamicCast( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView(), DrawingView ) );
}

void MDIEditorFrame::OnSetDrawingZoom( wxUpdateUIEvent& event )
{
    DrawingView* v;

    v =  wxDynamicCast( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView(), DrawingView );
    if ( v )
    {
        event.Enable( true );

        wxString s;
        s.Printf( _( "Zoom factor is %f" ), v->GetScaler() );

        SetStatusText( s, 1 );
    }
    else
        event.Enable( false );
}

void MDIEditorFrame::CreateAddView( wxCommandEvent& event )
{
    if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().size() )
    {
        wxMessageBox( wxT( "No documents, therefore no view to choose" ), wxT( "ChooseViewFromDocument" ), wxOK , this );
        return;
    }
    if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView()->GetDocument() )
    {
        wxMessageBox( wxT( "No document set, therefore no view to choose" ), wxT( "ChooseViewFromDocument" ), wxOK , this );
        return;
    }

    a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateView( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView()->GetDocument() )->Update();
}

void MDIEditorFrame::CreateInWindowView( wxCommandEvent& event )
{
    a2dView* backup = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView();

    if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().size() )
    {
        wxMessageBox( wxT( "No documents, therefore no view to choose" ), wxT( "ChooseViewFromDocument" ), wxOK , this );
        return;
    }
    if ( !backup->GetDocument() )
    {
        wxMessageBox( wxT( "No document set, therefore no view to choose" ), wxT( "ChooseViewFromDocument" ), wxOK , this );
        return;
    }

    backup->Activate( true );
    a2dView* v = m_singleconnector->CreateView( backup->GetDocument(), wxT( "" ) );
    m_canvas->SetView( v );
    v->Update();
//    a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView()->GetDocument()->CreateView( "", a2dTemplateFlag::VISIBLE )->Update();
//    a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView()->GetDocument()->CreateView( "Drawing View" )->Update();
}

void MDIEditorFrame::ChooseDocumentForView( wxCommandEvent& event )
{
    DocumentListDialog documents( this, a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments() );
    if ( documents.ShowModal() == wxID_OK )
    {
        ConnectDocument( ( DrawingDocument* ) documents.GetDocument() );
    }

}

void MDIEditorFrame::ChooseViewFromDocument( wxCommandEvent& event )
{
    if ( m_view  && m_view->GetDocument() )
    {
        a2dViewList allviews;
        m_view->GetDocument()->ReportViews( &allviews );

        ViewListDialog views( this, allviews );
        if ( views.ShowModal() == wxID_OK )
        {
            m_canvas->SetView( views.GetView() );
            m_view->Update();
        }
    }
    else
        wxMessageBox( wxT( "No document set" ), wxT( "ChooseViewFromDocument" ), wxOK , this );

}

void MDIEditorFrame::ConnectDocument( a2dDocument* doc )
{
    if ( !doc )
        return;

    a2dViewList allviews;
    doc->ReportViews( &allviews );

    if ( !allviews.size() )
    {
        a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateView( doc, wxT( "Drawing View" ) )->Update();
        doc->ReportViews( &allviews );
    }

    a2dView* view = m_view ;

    if ( view )
    {
        view->SetDisplayWindow( NULL );
    }

    view = allviews.front();

    view->SetDisplayWindow( m_canvas );
    view->Update();
    m_canvas->SetView( view );

    if ( view->GetDocument() )
    {
    }
    view->Activate( true );
}


