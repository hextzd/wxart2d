/*! \file docview/samples/common/view.cpp
    \brief View classes
    \author Julian Smart
    \date Created 04/01/98

    Copyright: (c) Julian Smart and Markus Holzem

    Licence: wxWidgets license

    RCS-ID: $Id: view.cpp,v 1.12 2009/09/26 19:01:19 titato Exp $
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

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include "doc.h"
#include "view.h"
#include "docdialog.h"

#include "wx/colordlg.h"
#include <wx/wfstream.h>

IMPLEMENT_DYNAMIC_CLASS( DrawingView, a2dView )

// For drawing lines on a view
float xpos = -1;
float ypos = -1;

BEGIN_EVENT_TABLE( DrawingView, a2dView )
    EVT_MOUSE_EVENTS( DrawingView::OnMouseEvent )
    EVT_UPDATE_VIEWS( DrawingView::OnUpdate )
    EVT_SET_FOCUS( DrawingView::OnUpdate2 )
    EVT_KILL_FOCUS( DrawingView::OnUpdate2 )
END_EVENT_TABLE()


bool DrawingView::ProcessEvent( wxEvent& event )
{
    if ( event.GetEventType() == wxEVT_KILL_FOCUS )
        return a2dView::ProcessEvent( event );
    if ( event.GetEventType() == wxEVT_SET_FOCUS )
        return a2dView::ProcessEvent( event );
    return a2dView::ProcessEvent( event );
}

DrawingView::DrawingView()
{
    m_drawingcolor = *wxBLACK;
    m_penWidth = 0;
    m_scaler = 1.4;
};

// Sneakily gets used for default print/preview
// as well as drawing on the screen.
void DrawingView::OnDraw( wxDC* dc )
{
    if ( !m_viewDocument )
        return;

    dc->SetFont( *wxNORMAL_FONT );
    dc->SetUserScale( m_scaler, m_scaler );

    wxList::compatibility_iterator node = ( ( DrawingDocument* )GetDocument() )->GetDoodleSegments().GetFirst();
    while ( node )
    {
        DoodleSegment* seg = ( DoodleSegment* )node->GetData();
        seg->Draw( dc );
        node = node->GetNext();
    }
}

void DrawingView::OnUpdate( a2dDocumentEvent& event )
{
    if ( m_viewDocument != event.GetEventObject() )
        return;
    if ( m_display )
        m_display->Refresh();
}

void DrawingView::OnUpdate2( wxFocusEvent& event )
{
    if ( m_viewDocument != event.GetEventObject() )
        return;
    if ( m_display )
        m_display->Refresh();
}

// This implements a tiny doodling program. Drag the mouse using
// the left button.
void DrawingView::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_display )
        return;

    static DoodleSegment* currentSegment = ( DoodleSegment* ) NULL;

    wxClientDC dc( m_display );
    m_display->PrepareDC( dc );

    wxPen pen = wxPen( m_drawingcolor, m_penWidth, wxPENSTYLE_SOLID );
    dc.SetPen( pen );

    wxPoint pt( event.GetLogicalPosition( dc ) );

    if ( currentSegment && event.LeftUp() )
    {
        if ( currentSegment->m_lines.size() == 0 )
        {
            delete currentSegment;
            currentSegment = ( DoodleSegment* ) NULL;
        }
        else
        {
            // We've got a valid segment on mouse left up, so store it.
            DrawingDocument* doc = ( DrawingDocument* ) GetDocument();

            doc->GetCommandProcessor()->Submit( new DrawingCommandAddSegment( doc, currentSegment ) );
            doc->UpdateAllViews( this, 0 );
            GetDocument()->Modify( true );
            currentSegment = ( DoodleSegment* ) NULL;
        }
    }

    if ( xpos > -1 && ypos > -1 && event.Dragging() )
    {
        if ( !currentSegment )
            currentSegment = new DoodleSegment( m_drawingcolor, m_penWidth );

        DoodleLine* newLine = new DoodleLine;
        newLine->x1 = ( long )xpos / m_scaler;
        newLine->y1 = ( long )ypos / m_scaler;
        newLine->x2 = pt.x / m_scaler;
        newLine->y2 = pt.y / m_scaler;
        currentSegment->m_lines.Append( newLine );

        dc.DrawLine( ( long )xpos, ( long )ypos, pt.x, pt.y );
    }
    xpos = pt.x;
    ypos = pt.y;
}

IMPLEMENT_DYNAMIC_CLASS( TextView, a2dView )

BEGIN_EVENT_TABLE( TextView, a2dView )
    EVT_IDLE( TextView::OnIdle )
    EVT_UPDATE_VIEWS( TextView::OnUpdate )
END_EVENT_TABLE()

void TextView::OnUpdate( a2dDocumentEvent& event )
{
    if ( m_viewDocument != event.GetEventObject() )
        return;

    //write directly the data into the display window which is a text control.
    //This will automatically cause a refresh of the window.
    //This view does not need a local storage,

    a2dDocumentStringOutputStream mem;

#if wxUSE_STD_IOSTREAM
    mem << _T( "Contents of document \n" );
#else
    wxTextOutputStream text_stream( mem );
    text_stream << _T( "Contents of document \n" );
#endif

    GetDocument()->SaveObject( mem, NULL );

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

void TextView::OnIdle( wxIdleEvent& event )
{
}

BEGIN_EVENT_TABLE( wxTextViewFrame, a2dDocumentFrame )
    EVT_MENUSTRINGS( wxTextViewFrame::OnSetMenuStrings )
END_EVENT_TABLE()

wxTextViewFrame::wxTextViewFrame( TextView* view, const wxPoint& pos, const wxSize& size, wxFrame* parent )
    :  a2dDocumentFrame( ( bool ) false, parent, view, -1, _T( "Child Frame" ), pos, size, wxDEFAULT_FRAME_STYLE )
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

    m_edit_menu = new wxMenu;
    m_edit_menu->Append( wxID_UNDO, wxT( "&Undo" ) );
    m_edit_menu->Append( wxID_REDO, wxT( "&Redo" ) );

    m_view = view;
    wxMenuBar* menu_bar = new wxMenuBar;

    menu_bar->Append( file_menu, wxT( "&File" ) );
    menu_bar->Append( m_edit_menu, wxT( "&Edit" ) );

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



IMPLEMENT_DYNAMIC_CLASS( StatisticsView, a2dView )

BEGIN_EVENT_TABLE( StatisticsView, a2dView )
    EVT_UPDATE_VIEWS( StatisticsView::OnUpdate )
END_EVENT_TABLE()

void StatisticsView::OnUpdate( a2dDocumentEvent& event )
{
    if ( m_viewDocument != event.GetEventObject() )
        return;

    //update the internal data of the view

    m_objectcount = ( ( DrawingDocument* )GetDocument() )->GetDoodleSegments().size();

    m_usedcolours.clear();

    wxList::compatibility_iterator node = ( ( DrawingDocument* )GetDocument() )->GetDoodleSegments().GetFirst();
    while ( node )
    {
        DoodleSegment* segment = ( DoodleSegment* )node->GetData();

        int old = m_usedcolours[ ColourToHex( segment->m_colour ) ];
        m_usedcolours[ ColourToHex( segment->m_colour ) ] = old + 1;

        node = node->GetNext();
    }

    if ( m_display )
        m_display->Refresh();
}

void StatisticsView::OnDraw( wxDC* dc )
{
    dc->SetFont( wxFont( 20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL ) );
    dc->SetPen( *wxBLACK_PEN );

    if ( m_display )
    {
        dc->DrawText( _T( "Colours used in document" ) , 0, 0 );
        int y = 45;
        // iterate over all the elements in the class
        ColourHash::iterator it;
        for( it = m_usedcolours.begin(); it != m_usedcolours.end(); ++it )
        {
            wxString key = it->first;
            wxString text;
            text << wxT( "colour:" ) << key << wxT( " is used in:  " ) << it->second << wxT( " objects" );

            if ( key != _T( "000000" ) )
                dc->SetBackgroundMode( wxSOLID );
            else
                dc->SetBackgroundMode( wxTRANSPARENT );

            dc->SetTextBackground( HexToColour( key ) );
            dc->DrawText( text , 20, y );
            y += 35;
        }
    }
}

IMPLEMENT_DYNAMIC_CLASS( a2dMultiFrameDrawingConnector, a2dFrameViewConnector )

BEGIN_EVENT_TABLE( a2dMultiFrameDrawingConnector, a2dFrameViewConnector )
    EVT_POST_CREATE_DOCUMENT( a2dMultiFrameDrawingConnector::OnPostCreateDocument )
    EVT_POST_CREATE_VIEW( a2dMultiFrameDrawingConnector::OnPostCreateView )
END_EVENT_TABLE()

a2dMultiFrameDrawingConnector::a2dMultiFrameDrawingConnector( wxClassInfo* EditorClassInfo )
    : a2dFrameViewConnector()
{
    m_editorClassInfo = EditorClassInfo;
}

a2dMultiFrameDrawingConnector::~a2dMultiFrameDrawingConnector()
{
}

void a2dMultiFrameDrawingConnector::OnPostCreateDocument( a2dTemplateEvent& event )
{
    a2dViewTemplate* viewtempl;
    viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->SelectViewType( event.GetDocument(),
                m_viewTemplates.size() ? m_viewTemplates : a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates() );

    /*
        a2dViewTemplateList::compatibility_iterator nodev = m_documentManager->GetViewTemplates().GetFirst();
        while (nodev)
        {
            a2dViewTemplate *temp = nodev->GetData();
            if ( temp->IsVisible() && temp->GetDocumentTypeName() == doc->GetDocumentTypeName() && !temp->GetViewTypeName().empty() )
            {
                //this is a valid view BUT i can have only those that produce a frame here.
                temp->SetMarked( NULL != wxDynamicCast( temp->GetViewConnector(), a2dMultiFrameDrawingConnector ) );
            }
            nodev = nodev->GetNext();
        }

        a2dViewTemplate* viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->SelectViewType( event.GetDocument(), false,
                                                    (wxDocumentTemplateFlag) (a2dTemplateFlag::VISIBLE | a2dREFTEMPLATE_MARK) );

        nodev = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates().GetFirst();
        while (nodev)
        {
            nodev->GetData()->SetMarked( false );
            nodev = nodev->GetNext();
        }
    */
    if ( !viewtempl )
    {
        wxLogMessage( _T( "No view available for this document in a2dMultiFrameDrawingConnector" ) );
        return;
    }

    if ( !viewtempl->CreateView( event.GetDocument(), event.GetFlags() ) )
    {
        wxLogMessage( _T( "a2dMultiFrameDrawingConnector no view was created" ) );
        return;
    }
}

void a2dMultiFrameDrawingConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    a2dView* view = event.GetView();

    a2dDocument* doc = event.GetView()->GetDocument();

    if ( view->GetViewTypeName() == _T( "Text View Frame" ) )
    {
        wxTextViewFrame* textFrame = new wxTextViewFrame( ( TextView* ) view, GetInitialPosition(),
                view->GetViewTemplate()->GetInitialSize(), m_docframe );
        textFrame->SetTitle( _T( "TextEditView" ) );
        textFrame->SetView( view );
        view->SetDisplayWindow( textFrame->m_textdisplay );
        textFrame->Enable();
        textFrame->Show( true );
    }
    else if ( view->GetViewTypeName() == _T( "Statistics View Frame" ) )
    {

        a2dDocumentFrame* viewFrame = new a2dDocumentFrame( false,
                m_docframe, view, -1, view->GetViewTypeName(),
                GetInitialPosition(),
                view->GetViewTemplate()->GetInitialSize(), //WATCH get it from the viewtemplate
                GetInitialStyle()
                                                          );
        viewFrame->SetBackgroundColour( wxColour( 212, 240, 230 ) );
        viewFrame->Show( true );
    }
    else if ( view->GetViewTypeName() == wxT( "Drawing View Frame" ) )
    {
        EditorFrame* viewFrame;
        if ( m_editorClassInfo )
        {
            viewFrame = ( EditorFrame* ) m_editorClassInfo->CreateObject();
            viewFrame->Create( false, m_docframe, ( DrawingView* ) view, -1,
                               _T( "Single with docmanager" ), GetInitialPosition(), GetInitialSize(), wxDEFAULT_FRAME_STYLE );
        }
        else
            viewFrame = new EditorFrame( false, m_docframe, ( DrawingView* ) view, -1,
                                         wxT( "Single with docmanager" ), GetInitialPosition(), GetInitialSize(), wxDEFAULT_FRAME_STYLE );
        viewFrame->Show( true );

        doc->GetCommandProcessor()->Initialize();

        viewFrame->GetCanvas()->SetBackgroundColour( wxColour( 12, 240, 230 ) );
    }


    m_docframe->Refresh();
}


//------------------------------------------------------------------------------
// EditorFrame
//------------------------------------------------------------------------------
extern const long EditorFrame_CUT = wxNewId();
extern const long EditorFrame_ABOUT = wxNewId();
extern const long EditorFrame_DRAWINGCOLOUR = wxNewId();
extern const long EditorFrame_DRAWINGWIDTH = wxNewId();
extern const long EditorFrame_DRAWINGSCALE = wxNewId();
extern const long EditorFrame_WINDOWWVIEW = wxNewId();
extern const long EditorFrame_ADDVIEW = wxNewId();
extern const long EditorFrame_CHOOSEVIEW = wxNewId();
extern const long EditorFrame_CONNECTDOC = wxNewId();

IMPLEMENT_DYNAMIC_CLASS( EditorFrame, a2dDocumentFrame )


BEGIN_EVENT_TABLE( EditorFrame, a2dDocumentFrame )
    EVT_MENU( EditorFrame_CUT, EditorFrame::OnCut )
    EVT_MENU( wxID_EXIT, EditorFrame::OnExit )
    EVT_CLOSE( EditorFrame::OnCloseWindow )
    EVT_MENU( EditorFrame_ABOUT, EditorFrame::OnAbout )
    EVT_MENU( EditorFrame_DRAWINGCOLOUR, EditorFrame::SelectDrawingColour )
    EVT_MENU( EditorFrame_DRAWINGWIDTH, EditorFrame::SetDrawingWidth )
    EVT_MENU( EditorFrame_DRAWINGSCALE, EditorFrame::SetDrawingZoom )
    EVT_MENU( EditorFrame_CONNECTDOC, EditorFrame::ChooseDocumentForView )
    EVT_MENU( EditorFrame_WINDOWWVIEW, EditorFrame::CreateInWindowView )
    EVT_MENU( EditorFrame_ADDVIEW, EditorFrame::CreateAddView )
    EVT_MENU( EditorFrame_CHOOSEVIEW, EditorFrame::ChooseViewFromDocument )
    EVT_ACTIVATE( EditorFrame::OnActivate )
    EVT_ACTIVATE_VIEW_SENT_FROM_CHILD( EditorFrame::OnActivateViewSentFromChild )
    EVT_UPDATE_UI( EditorFrame_DRAWINGCOLOUR, EditorFrame::OnSelectDrawingColour )
    EVT_UPDATE_UI( EditorFrame_DRAWINGWIDTH, EditorFrame::OnSetDrawingWidth )
    EVT_UPDATE_UI( EditorFrame_DRAWINGSCALE, EditorFrame::OnSetDrawingZoom )
    EVT_MENU( wxID_UNDO, EditorFrame::OnUndo )
    EVT_MENU( wxID_REDO, EditorFrame::OnRedo )
    EVT_MENUSTRINGS( EditorFrame::OnSetMenuStrings )
END_EVENT_TABLE()

EditorFrame::EditorFrame( bool isParent, wxFrame* parent,
                          DrawingView* view,  wxWindowID id, const wxString& title,
                          const wxPoint& position, const wxSize& size, const long style ) :
    a2dDocumentFrame( isParent, parent, view,  -1, title, position, size, style )
{
    if ( view->GetDocument() )
    {
        view->GetDocument()->ConnectEvent( wxEVT_MENUSTRINGS, this );
        view->GetDocument()->ConnectEvent( wxEVT_DO, this );
        view->GetDocument()->ConnectEvent( wxEVT_UNDO, this );
    }
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_NEW_DOCUMENT, this );
    Init( view );
}

bool EditorFrame::Create( bool isParent, wxFrame* parent,
                          DrawingView* view, wxWindowID id, const wxString& title,
                          const wxPoint& position, const wxSize& size, const long style )
{
    bool res = a2dDocumentFrame::Create( isParent, parent, view, -1, title, position, size, style );
    Init( view );
    return res;
}

void EditorFrame::Init( DrawingView* view )
{
    CreateMyMenuBar();
    CreateStatusBar( 2 );
    SetStatusText( _T( "Multi purpose EditorFrame for DrawingView's" ) );

    SetIcon( wxICON( mondrian ) );

    m_singleconnector = new a2dScrolledWindowViewConnector();

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

    m_singleconnector->SetDisplayWindow( m_canvas );

    // Create a template relating drawing documents to their files
    m_editordoctemplatenew = new a2dDocumentTemplate( _T( "DRW Drawing" ), _T( "*.drw" ), _T( "" ), _T( "drw" ),
            _T( "DrawingDocument" ),
            CLASSINFO( DrawingDocument ),
            m_singleconnector,
            a2dTemplateFlag::DEFAULT );
    // Create a template relating drawing documents to their views
    a2dViewTemplate* draw = new a2dViewTemplate( _T( "DrawingDocument View" ), _T( "DrawingDocument" ), _T( "Drawing View In Window" ),
            CLASSINFO( DrawingView ),
            m_singleconnector,
            a2dTemplateFlag::DEFAULT );
    // Create a template relating drawing documents to statistics views
    a2dViewTemplate* stat = new a2dViewTemplate( _T( "DrawingDocument View" ), _T( "DrawingDocument" ), _T( "Statistics View In Window" ),
            CLASSINFO( StatisticsView ),
            m_singleconnector,
            a2dTemplateFlag::DEFAULT );

    m_singleconnector->AssociateViewTemplate( draw );
    m_singleconnector->AssociateViewTemplate( stat );
}

void EditorFrame::CreateMyMenuBar()
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

    if ( GetIsParentFrame() )
        m_file_menu->Append( wxID_EXIT, _T( "E&xit" ) );
    else
        m_file_menu->Append( wxID_EXIT, _T( "Close &Frame" ) );

    if ( GetIsParentFrame() )
        // A nice touch: a history of files visited. Use this menu.
        a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( m_file_menu );

    m_edit_menu = new wxMenu;
    m_edit_menu->Append( wxID_UNDO, _T( "&Undo" ) );
    m_edit_menu->Append( wxID_REDO, _T( "&Redo" ) );
    m_edit_menu->AppendSeparator();
    m_edit_menu->Append( EditorFrame_CUT, _T( "&Cut last segment" ) );

    m_drawing_menu = new wxMenu;
    m_drawing_menu->Append( EditorFrame_DRAWINGCOLOUR, _T( "&Drawing Colour" ), _T( "set drawing colour for current view" ) );
    m_drawing_menu->Append( EditorFrame_DRAWINGWIDTH, _T( "&Drawing Width" ), _T( "set drawing width for current view" ) );
    m_drawing_menu->Append( EditorFrame_DRAWINGSCALE, _T( "&Drawing Zoom" ), _T( "set drawing zoom factor" ) );

    m_view_menu = new wxMenu;
    m_view_menu->Append( EditorFrame_WINDOWWVIEW, _T( "&Add Editor view in window" ), _T( "create a choosen view specific to this frame using current view its document and window" ) );
    m_view_menu->Append( EditorFrame_ADDVIEW, _T( "&Add Connector view" ), _T( "create an extra choosen view via docmanager using current view its document" ) );
    m_view_menu->Append( EditorFrame_CHOOSEVIEW, _T( "&Choose View" ), _T( "choose view from the document" ) );

    m_document_menu = new wxMenu;
    m_document_menu->Append( EditorFrame_CONNECTDOC, _T( "&Choose Document" ), _T( "set document to a view" ) );

    m_help_menu = new wxMenu;
    m_help_menu->Append( EditorFrame_ABOUT, _T( "&About EditorFrame" ) );

    m_menu_bar->Append( m_file_menu, _T( "&File" ) );
    m_menu_bar->Append( m_edit_menu, _T( "&Edit" ) );
    m_menu_bar->Append( m_drawing_menu, _T( "&Drawing" ) );
    m_menu_bar->Append( m_view_menu, _T( "&Extra view" ) );
    m_menu_bar->Append( m_document_menu, _T( "&Document" ) );
    m_menu_bar->Append( m_help_menu, _T( "&Help" ) );

    SetMenuBar( m_menu_bar );

}

EditorFrame::~EditorFrame()
{
    m_singleconnector->DisassociateViewTemplates();
    if ( m_view->GetDocument() )
    {
        m_view->GetDocument()->ConnectEvent( wxEVT_MENUSTRINGS, this );
        m_view->GetDocument()->ConnectEvent( wxEVT_DO, this );
        m_view->GetDocument()->ConnectEvent( wxEVT_UNDO, this );
    }
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_NEW_DOCUMENT, this );
}

void EditorFrame::OnCut( wxCommandEvent& WXUNUSED( event ) )
{
    DrawingDocument* doc = ( DrawingDocument* )m_view->GetDocument();
    doc->GetCommandProcessor()->Submit( new DrawingCommandCutSegment( doc, ( DoodleSegment* ) NULL ) );
}

void EditorFrame::OnSetMenuStrings( a2dCommandProcessorEvent& event )
{
    if ( m_view && m_view->GetDocument() &&
            event.GetEventObject() == m_view->GetDocument()->GetCommandProcessor() )
    {
        m_edit_menu->SetLabel( wxID_UNDO, event.GetUndoMenuLabel() );
        m_edit_menu->Enable( wxID_UNDO, event.CanUndo() );

        m_edit_menu->SetLabel( wxID_REDO, event.GetRedoMenuLabel() );
        m_edit_menu->Enable( wxID_REDO, event.CanRedo() );
    }
}


void EditorFrame::OnCloseWindow( wxCloseEvent& event )
{
    a2dDocument* doc = NULL;
    if ( m_view )
        doc = m_view->GetDocument();

    if ( !event.CanVeto() )
    {
    }

    event.Skip(); //to base wxDocFrameScrolledWindow
}

void EditorFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( _T( "Common Editor Frame class\nAuthor: Klaas Holwerda" ), _T( "About Editor Frame" ) );
}

void EditorFrame::OnExit( wxCommandEvent& event )
{
    if ( !GetIsParentFrame() )
        //GetParent()->Close( true );
        Close( true );
    else
        Close( true );
}

void EditorFrame::OnActivate( wxActivateEvent& event )
{
    if ( !m_view )
        return;
    if ( event.GetActive() )
    {
        m_view->Activate( true );
        m_singleconnector->SetDisplayWindow( ( a2dDocumentViewScrolledWindow* ) m_view->GetDisplayWindow() );
        //a2dDocument* doc = m_view->GetDocument();
    }
}

void EditorFrame::OnActivateViewSentFromChild( a2dViewEvent& viewevent )
{
    if ( viewevent.GetActive() )
    {
        m_view = ( a2dView* ) viewevent.GetEventObject();
        m_singleconnector->SetDisplayWindow( ( a2dDocumentViewScrolledWindow* ) m_view->GetDisplayWindow() );
    }
    viewevent.Skip();
}

void EditorFrame::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( !doc )
        return;
    if ( doc->GetCommandProcessor() )
        doc->GetCommandProcessor()->Undo();
}

void EditorFrame::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( !doc )
        return;
    if ( doc->GetCommandProcessor() )
        doc->GetCommandProcessor()->Redo();
}

void EditorFrame::SelectDrawingColour( wxCommandEvent& event )
{
    wxColour col;
    wxColourData data;
    wxColourDialog dialog( this, &data );

    if ( dialog.ShowModal() == wxID_OK )
    {
        col = dialog.GetColourData().GetColour();
    }

    if ( wxDynamicCast( m_view.Get(), DrawingView ) )
        ( ( DrawingView* ) m_view.Get() )->SetDrawingColour( col );
}

void EditorFrame::SetDrawingWidth( wxCommandEvent& event )
{
    int currentwidth = ( ( DrawingView* ) m_view.Get() )->GetDrawingWidth();

    long w = wxGetNumberFromUser( _T( "Give Pen Width:" ), _T( "pen width" ), _T( "pen width" ), currentwidth, 0, 10000 );

    if ( wxDynamicCast( m_view.Get(), DrawingView ) )
        ( ( DrawingView* )m_view.Get() )->SetDrawingWidth( w );
}

void EditorFrame::SetDrawingZoom( wxCommandEvent& event )
{
    DrawingView* v;
    v = wxDynamicCast( m_view.Get(), DrawingView );
    if ( v )
    {
        long z = wxGetNumberFromUser( _T( "set zoom factor " ), _T( "(negative means < 1):" ), _T( "DoodleZoom" ),
                                      ( int ) v->GetScaler(), -30, 30, this );

        if ( z < 0 )
            v->SetScaler( 1 / ( double ) - z );
        else
            v->SetScaler( z );
        v->Update();
    }
}

// Handler for UI update command
void EditorFrame::OnSelectDrawingColour( wxUpdateUIEvent& event )
{
    event.Enable( 0 != wxDynamicCast( m_view.Get(), DrawingView ) );
}
void EditorFrame::OnSetDrawingWidth( wxUpdateUIEvent& event )
{
    event.Enable( 0 != wxDynamicCast( m_view.Get(), DrawingView ) );
}
void EditorFrame::OnSetDrawingZoom( wxUpdateUIEvent& event )
{
    DrawingView* v;

    v =  wxDynamicCast( m_view.Get(), DrawingView );
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

void EditorFrame::CreateAddView( wxCommandEvent& event )
{
    if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().size() )
    {
        wxMessageBox( _T( "No documents, therefore no view to choose" ), _T( "ChooseViewFromDocument" ), wxOK , this );
        return;
    }
    if ( !m_view->GetDocument() )
    {
        wxMessageBox( _T( "No document set, therefore no view to choose" ), _T( "ChooseViewFromDocument" ), wxOK , this );
        return;
    }

    a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateView( m_view->GetDocument() )->Update();
}

void EditorFrame::CreateInWindowView( wxCommandEvent& event )
{

    if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().size() )
    {
        wxMessageBox( _T( "No documents, therefore no view to choose" ), _T( "CreateInWindowView" ), wxOK , this );
        return;
    }

    a2dDocument* doc;
    if ( m_view )
        doc = m_view->GetDocument();
    else
        doc = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentDocument();

    if ( !doc )
    {
        wxMessageBox( _T( "No document set, therefore no view to choose" ), _T( "CreateInWindowView" ), wxOK , this );
        return;
    }

    a2dView* v = m_singleconnector->CreateView( doc, _T( "" ) );
    m_singleconnector->GetDisplayWindow()->SetView( v );
    v->Update();
//    a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateView( "", a2dTemplateFlag::VISIBLE )->Update();
//    a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateView( "Drawing View" )->Update();
}

void EditorFrame::ChooseDocumentForView( wxCommandEvent& event )
{
    DocumentListDialog documents( this, a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments() );
    if ( documents.ShowModal() == wxID_OK )
    {
        ConnectDocument( ( DrawingDocument* ) documents.GetDocument() );
    }

}

void EditorFrame::ChooseViewFromDocument( wxCommandEvent& event )
{
    if ( m_view && m_view->GetDocument() )
    {
        a2dViewList allviews;
        m_view->GetDocument()->ReportViews( &allviews );

        ViewListDialog views( this, allviews );
        if ( views.ShowModal() == wxID_OK )
        {
            m_singleconnector->GetDisplayWindow()->SetView( views.GetView() );
            m_view->Update();
        }
    }
    else
        wxMessageBox( _T( "No document set" ), _T( "ChooseViewFromDocument" ), wxOK , this );

}

void EditorFrame::ConnectDocument( a2dDocument* doc )
{
    if ( !doc )
        return;

    a2dViewList allviews;
    doc->ReportViews( &allviews );

    if ( !allviews.size() )
    {
        a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateView( doc, _T( "Drawing View" ) )->Update();
        doc->ReportViews( &allviews );
    }

    a2dView* view = m_view;

    if ( view )
    {
        view->SetDisplayWindow( NULL );
    }

    view = allviews.front();

    view->SetDisplayWindow( m_canvas );
    view->Update();
    m_singleconnector->GetDisplayWindow()->SetView( view );

    if ( view->GetDocument() )
    {
    }
}
