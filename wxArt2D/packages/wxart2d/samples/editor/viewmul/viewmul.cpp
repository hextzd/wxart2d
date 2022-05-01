/*! \file editor/samples/viewmul/view.cpp
    \brief View classes
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: viewmul.cpp,v 1.13 2009/09/26 19:01:19 titato Exp $
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

#include "viewmul.h"
#include "docv.h"
#include "doc.h"

IMPLEMENT_DYNAMIC_CLASS( a2dMultiDrawingConnector, a2dFrameViewConnector )

BEGIN_EVENT_TABLE( a2dMultiDrawingConnector, a2dFrameViewConnector )
    EVT_POST_CREATE_VIEW( a2dMultiDrawingConnector::OnPostCreateView )
END_EVENT_TABLE()

a2dMultiDrawingConnector::a2dMultiDrawingConnector()
    : a2dFrameViewConnector( )
{
}

a2dMultiDrawingConnector::~a2dMultiDrawingConnector()
{
}


void a2dMultiDrawingConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    a2dView* view = event.GetView();

    a2dDocument* doc = event.GetView()->GetDocument();

    a2dDocumentFrame* viewFrame = new a2dDocumentFrame( false,
            m_docframe, view, -1, view->GetViewTypeName(),
            GetInitialPosition(),
            GetInitialSize(),
            GetInitialStyle()
                                                      );


    //// Make a menubar
    wxMenu* file_menu = new wxMenu;
    wxMenu* edit_menu = ( wxMenu* ) NULL;

    viewFrame->AddCmdMenu( file_menu, CmdMenu_FileNew() );
    viewFrame->AddCmdMenu( file_menu, CmdMenu_FileOpen() );
    viewFrame->AddCmdMenu( file_menu, CmdMenu_FileClose() );
    viewFrame->AddCmdMenu( file_menu, CmdMenu_FileSave() );
    viewFrame->AddCmdMenu( file_menu, CmdMenu_FileSaveAs() );
    viewFrame->AddCmdMenu( file_menu, CmdMenu_Print() );
    file_menu->AppendSeparator();
    viewFrame->AddCmdMenu( file_menu, CmdMenu_PrintView() );
    viewFrame->AddCmdMenu( file_menu, CmdMenu_PreviewView() );
    viewFrame->AddCmdMenu( file_menu, CmdMenu_PrintDocument() );
    viewFrame->AddCmdMenu( file_menu, CmdMenu_PreviewDocument() );
    viewFrame->AddCmdMenu( file_menu, CmdMenu_PrintSetup() );

    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( file_menu );

    wxMenuBar* menu_bar = new wxMenuBar;

    menu_bar->Append( file_menu, _( "&File" ) );
    if ( edit_menu )
        menu_bar->Append( edit_menu, _( "&Edit" ) );

    //// Associate the menu bar with the frame
    viewFrame->SetMenuBar( menu_bar );

    a2dCanvas* canvas = new a2dCanvas( viewFrame, -1, wxPoint( 0, 0 ), GetInitialSize() );
	view->SetDisplayWindow( canvas );
    canvas->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL ) );
    //defines the minimum x and y to show and the number of world units per pixel in x and y
    canvas->SetYaxis( true );
    canvas->SetMappingUpp( -600, -600, 4, 4 );

    //defines the whole area in world coordinates that can be seen by scrolling
    canvas->SetScrollMaximum( -800, -800, 1600, 1600 );
    //defines the number of world units scrolled when line up or down events in X
    canvas->SetScrollStepX( 200 );
    //defines the number of world units scrolled when line up or down events in Y
    canvas->SetScrollStepY( 100 );

    canvas->SetBackgroundColour( wxColour( 0, 240, 230 ) );

    a2dStToolContr* contr  = new a2dStToolContr( canvas->GetDrawingPart(), viewFrame );

    //the next is handy, but as you prefer
    contr->SetZoomFirst( true );

    a2dDrawRectangleTool* draw = new a2dDrawRectangleTool( contr );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( contr ) );
    draw->SetFill( a2dFill( wxColour( 219, 21, 6 ), a2dFILL_HORIZONTAL_HATCH ) );
    draw->SetStroke( a2dStroke( wxColour( 1, 3, 205 ), 30.0 ) );
    draw->SetEditAtEnd( true );

    contr->PushTool( draw );

    viewFrame->CreateStatusBar( 2 );

    canvas->GetDrawing()->GetCommandProcessor()->Initialize();
    viewFrame->Show( true );
}


/*********************************************************************************/
/*Text view. */
/*********************************************************************************/
IMPLEMENT_CLASS( wxTextConnector, a2dFrameViewConnector )

BEGIN_EVENT_TABLE( wxTextConnector, a2dFrameViewConnector )
    EVT_POST_CREATE_VIEW( wxTextConnector::OnPostCreateView )
END_EVENT_TABLE()

wxTextConnector::wxTextConnector()
    : a2dFrameViewConnector()
{
}

void wxTextConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    a2dView* view = event.GetView();

    if ( view->GetViewTypeName() == wxT( "Text View Frame" ) )
    {
        wxTextViewFrame* textFrame = new wxTextViewFrame( ( TextView* ) view, GetInitialPosition(),
                view->GetViewTemplate()->GetInitialSize(), m_docframe );
        textFrame->SetTitle( wxT( "TextEditView" ) );
        textFrame->SetView( view );
        view->SetDisplayWindow( textFrame->m_textdisplay );
        textFrame->Enable();
        textFrame->Show( true );
    }
    else
        wxLogMessage( _( "wxTextConnector not a Text View Frame, no view was created" ) );

}

IMPLEMENT_DYNAMIC_CLASS( TextView, a2dView )

BEGIN_EVENT_TABLE( TextView, a2dView )
    EVT_CREATE_VIEW( TextView::OnCreateView )
    EVT_SET_DOCUMENT( TextView::OnSetDocument )
    EVT_UPDATE_VIEWS( TextView::OnUpdate )
END_EVENT_TABLE()


void TextView::OnSetDocument( a2dViewEvent& event )
{
    m_top = ( ( a2dCanvasDocument* )GetDocument() )->GetDrawing()->GetRootObject();
}

void TextView::OnCreateView( a2dViewEvent& event )
{
    m_top = ( ( a2dCanvasDocument* )GetDocument() )->GetDrawing()->GetRootObject();
}


void TextView::OnUpdate( a2dDocumentEvent& event )
{
    if ( m_viewDocument != event.GetEventObject() )
        return;


#if __WXGTK__
    wxSTD ostringstream mem;
#else
    a2dDocumentStringOutputStream mem;
#endif

#if wxUSE_STD_IOSTREAM
    mem << _T( "Contents of document \n" );
#else
    wxTextOutputStream text_stream( mem );
    text_stream << _T( "Contents of document \n" );
#endif

    //GetDocument()->SaveObject( mem );

    a2dIOHandlerDocCVGOut CVGwriter;
    CVGwriter.SaveStartAt( mem, ( a2dCanvasDocument* ) GetDocument(), m_top );

    if ( m_display )
    {
        ( ( wxTextCtrl* )m_display )->Clear();
#if wxUSE_STD_IOSTREAM
        wxString str = wxString( mem.str().c_str(), wxConvUTF8 );
        ( ( wxTextCtrl* )m_display )->WriteText( str );
#else
        ( ( wxTextCtrl* )m_display )->WriteText( mem.GetString() );
#endif
        ( ( wxTextCtrl* )m_display )->SetInsertionPoint( 0 );
    }
}

bool TextView::SetShowObject( a2dCanvasObject* obj )
{
    m_top = obj;
    m_top->SetPending( true );
    return true;
}

BEGIN_EVENT_TABLE( wxTextViewFrame, a2dDocumentFrame )
    EVT_MENUSTRINGS( wxTextViewFrame::OnSetMenuStrings )
    EVT_MENU( MSHOW_TOP, wxTextViewFrame::OnShowObject )
END_EVENT_TABLE()

wxTextViewFrame::wxTextViewFrame( TextView* view, const wxPoint& pos, const wxSize& size, wxFrame* parent )
    :  a2dDocumentFrame( ( bool ) false, parent, view, -1, wxT( "Child Frame" ), pos, size, wxDEFAULT_FRAME_STYLE )
{
#ifdef __WXMSW__
    SetIcon( wxString( wxT( "chrt_icn" ) ) );
#endif

    //// Make a menubar
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

    m_edit_menu = new wxMenu;
    m_edit_menu->Append( wxID_UNDO, _( "&Undo" ) );
    m_edit_menu->Append( wxID_REDO, _( "&Redo" ) );

    m_view = view;


    wxMenu* drawMenu = new wxMenu;
    drawMenu->Append( MSHOW_TOP, _( "object to show on textview" ), _( "choose top object to display" ) );

    wxMenuBar* menu_bar = new wxMenuBar;

    menu_bar->Append( file_menu, _( "&File" ) );
    menu_bar->Append( m_edit_menu, _( "&Edit" ) );
    menu_bar->Append( drawMenu, _( "&Screens" ) );

    //// Associate the menu bar with the frame

    SetMenuBar( menu_bar );

    Centre( wxBOTH );

    SetTitle( wxT( "TextView" ) );

    m_textdisplay = new wxTextCtrl( this, -1, wxT( "empty" ), wxDefaultPosition, wxDefaultSize, wxTE_READONLY  | wxTE_MULTILINE );

    m_view->SetDisplayWindow( m_textdisplay );
}

void wxTextViewFrame::OnSetMenuStrings( a2dCommandProcessorEvent& event )
{
    if ( event.GetEventObject() == m_view->GetDocument()->GetCommandProcessor() )
    {
        m_edit_menu->SetLabel( wxID_UNDO, event.GetUndoMenuLabel() );
        m_edit_menu->Enable( wxID_UNDO, event.CanUndo() );

        m_edit_menu->SetLabel( wxID_REDO, event.GetRedoMenuLabel() );
        m_edit_menu->Enable( wxID_REDO, event.CanRedo() );
    }
}

void wxTextViewFrame::OnShowObject( wxCommandEvent& event )
{
    a2dCanvasObjectsDialog groups( this, m_view->GetCanvasDocument()->GetDrawing(), true, true, ( wxSTAY_ON_TOP | wxRESIZE_BORDER | wxCAPTION ) );
    if ( groups.ShowModal() == wxID_OK )
    {
        m_view->SetShowObject( groups.GetCanvasObject() );
    }
}
