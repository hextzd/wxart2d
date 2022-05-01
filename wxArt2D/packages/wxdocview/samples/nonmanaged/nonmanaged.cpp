/*! \file docview/samples/nonmanaged/nonmanaged.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: nonmanaged.cpp,v 1.4 2009/09/26 19:01:19 titato Exp $
*/

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if wxCHECK_VERSION(2, 5, 0)
#if !WXWIN_COMPATIBILITY_2_4
#include "wx/numdlg.h"
#endif
#endif

// Include private headers
#include "nonmanaged.h"

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#include "doc.h"

#if wxUSE_PRINTING_ARCHITECTURE
#include "wx/prntbase.h"
#include "wx/printdlg.h"
#endif

#include "wx/colordlg.h"

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE( MyFrame, a2dDocumentFrame )
    EVT_MENU( ID_QUIT, MyFrame::OnQuit )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_CLOSE_VIEW( MyFrame::OnCloseView )
    EVT_MENU( WXDOCVIEW_ABOUT, MyFrame::OnAbout )
    EVT_MENU( wxID_UNDO, MyFrame::OnUndo )
    EVT_MENU( wxID_REDO, MyFrame::OnRedo )
    EVT_MENU( WXDOCVIEW_DRAWINGCOLOUR, MyFrame::SelectDrawingColour )
    EVT_MENU( WXDOCVIEW_DRAWINGWIDTH, MyFrame::SetDrawingWidth )
    EVT_MENUSTRINGS( MyFrame::OnSetmenuStrings )
    EVT_UPDATE_UI( wxID_UNDO, MyFrame::OnUpdateUndo )
    EVT_UPDATE_UI( wxID_REDO, MyFrame::OnUpdateRedo )
END_EVENT_TABLE()

MyFrame::MyFrame( wxFrame* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    a2dDocumentFrame( true, parent, NULL, -1, title, position, size, style )
{
    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( wxT( "Welcome to wxNonManaged!" ) );

    SetIcon( wxICON( mondrian ) );

    m_view = new DrawingView();

    m_outputwindow = new a2dDocumentViewScrolledWindow( m_view, this, -1, wxPoint( 0, 0 ), size, 0 );
    m_outputwindow->SetCursor( wxCURSOR_PENCIL );

    // Give it scrollbars
    m_outputwindow->SetScrollbars( 20, 20, 50, 50 );
    m_outputwindow->SetBackgroundColour( *wxWHITE );
    m_outputwindow->ClearBackground();

    DrawingDocument* doc = new DrawingDocument();
    doc->CreateCommandProcessor();
    m_view->SetDocument( doc );

    doc->GetCommandProcessor()->Initialize();
    doc->GetCommandProcessor()->ConnectEvent( wxEVT_MENUSTRINGS, this );
    doc->ConnectEvent( wxEVT_DO, this );
    doc->ConnectEvent( wxEVT_UNDO, this );
}

MyFrame::~MyFrame()
{
}

void MyFrame::CreateMyMenuBar()
{

    wxMenuBar* menu_bar = new wxMenuBar();

    wxMenu* file_menu = new wxMenu;

    AddCmdMenu( file_menu, CmdMenu_PrintView() );
    AddCmdMenu( file_menu, CmdMenu_PreviewView() );
    AddCmdMenu( file_menu, CmdMenu_PrintDocument() );
    AddCmdMenu( file_menu, CmdMenu_PreviewDocument() );
    AddCmdMenu( file_menu, CmdMenu_PrintSetup() );

    file_menu->Append( ID_QUIT, _( "Quit..." ), _( "Quit program" ) );
    file_menu->AppendSeparator();

    m_edit_menu = new wxMenu;
    m_edit_menu->Append( wxID_UNDO, _( "&Undo" ) );
    m_edit_menu->Append( wxID_REDO, _( "&Redo" ) );
    m_edit_menu->AppendSeparator();
    m_edit_menu->Append( WXDOCVIEW_CUT, _( "&Cut last segment" ) );

    wxMenu* drawing_menu = new wxMenu;
    drawing_menu->Append( WXDOCVIEW_DRAWINGCOLOUR, _( "&Drawing Colour" ), _( "set drawing colour for current view" ) );
    drawing_menu->Append( WXDOCVIEW_DRAWINGWIDTH, _( "&Drawing Width" ), _( "set drawing width for current view" ) );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( WXDOCVIEW_ABOUT, _( "&About" ) );

    menu_bar->Append( file_menu, _( "&File" ) );
    menu_bar->Append( m_edit_menu, _( "&Edit" ) );
    menu_bar->Append( drawing_menu, _( "&Drawing" ) );
    menu_bar->Append( help_menu, _( "&Help" ) );

    SetMenuBar( menu_bar );

}

void MyFrame::OnQuit( wxCommandEvent& event )
{
    //i don't want to save no matter what
    //BUT you might want to save it.

    // own the doc else view will delete it to soon
    m_view->GetDocument()->Modify( false );
    Close( true );
}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    //i don't want to save no matter what
    //BUT you might want to save it.
    a2dSmrtPtr<a2dDocument> doc = m_view->GetDocument();
    m_view->GetDocument()->Modify( false );
    m_view->GetDocument()->Close( true );

    Destroy();
}

void MyFrame::OnCloseView( a2dCloseViewEvent& event )
{
    wxASSERT_MSG( m_view || event.GetEventObject(), wxT( "not view of frame closed" ) );

    //there is no need to handle the parent frame differently

    m_view = NULL;
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( _T( " \
Shows how to use a view and its display window in a \
standalone situation, without a central command controller\
 ( which normally keeps track of open documents etc.) . \
 It still can Do and Undo changes to the document of the view, \
 since a document will have its own command stack.\
	\n\nNon Managed View Demo\nAuthor: Klaas Holwerda" ), _T( "About NonManaged" ) );
}

void MyFrame::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( !doc )
        return;
    if ( doc->GetCommandProcessor() )
        doc->GetCommandProcessor()->Undo();
}

void MyFrame::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( !doc )
        return;
    if ( doc->GetCommandProcessor() )
        doc->GetCommandProcessor()->Redo();
}

void MyFrame::OnUpdateUndo( wxUpdateUIEvent& event )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( !doc )
        return;
    event.Enable( ( doc && doc->GetCommandProcessor() && doc->GetCommandProcessor()->CanUndo() ) );
}

void MyFrame::OnUpdateRedo( wxUpdateUIEvent& event )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( !doc )
        return;
    event.Enable( ( doc && doc->GetCommandProcessor() && doc->GetCommandProcessor()->CanRedo() ) );
}

void MyFrame::OnSetmenuStrings( a2dCommandProcessorEvent& event )
{
    if ( event.GetEventObject() == m_view->GetDocument()->GetCommandProcessor() )
    {
        m_edit_menu->SetLabel( wxID_UNDO, event.GetUndoMenuLabel() );
        m_edit_menu->Enable( wxID_UNDO, event.CanUndo() );

        m_edit_menu->SetLabel( wxID_REDO, event.GetRedoMenuLabel() );
        m_edit_menu->Enable( wxID_REDO, event.CanRedo() );
    }
}

void MyFrame::SelectDrawingColour( wxCommandEvent& event )
{
    wxColour col;
    wxColourData data;
    wxColourDialog dialog( this, &data );

    if ( dialog.ShowModal() == wxID_OK )
    {
        col = dialog.GetColourData().GetColour();
    }

    m_view->SetDrawingColour( col );
}

void MyFrame::SetDrawingWidth( wxCommandEvent& event )
{

    int currentwidth = m_view->GetDrawingWidth();

    long w = wxGetNumberFromUser( _( "Give Pen Width:" ), _( "pen width" ), _( "pen width" ), currentwidth, 0, 10000 );

    m_view->SetDrawingWidth( w );
}

//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------


IMPLEMENT_APP( MyApp )

MyApp::MyApp()
{
}

bool MyApp::OnInit()
{
    m_frame = new MyFrame( NULL, -1, wxT( "Nonmanaged App" ), wxPoint( 20, 30 ), wxSize( 300, 240 ) );
    m_frame->Show( true );

    return true;
}

int MyApp::OnExit()
{
    return 0;
}

