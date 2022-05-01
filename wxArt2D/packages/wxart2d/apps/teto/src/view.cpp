/////////////////////////////////////////////////////////////////////////////
// Name:        view.cpp
// Purpose:     View classes
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

#include "view.h"
#include "wx/editor/xmlparsdoc.h"

IMPLEMENT_CLASS( wxDrawingConnector, a2dEditorMultiFrameViewConnector )
IMPLEMENT_CLASS( wxTextConnector, a2dFrameViewConnector )

BEGIN_EVENT_TABLE( wxDrawingConnector, a2dEditorMultiFrameViewConnector )
    EVT_POST_CREATE_VIEW( wxDrawingConnector::OnPostCreateView )
END_EVENT_TABLE()

wxDrawingConnector::wxDrawingConnector( wxClassInfo* EditorClassInfo )
    : a2dEditorMultiFrameViewConnector( EditorClassInfo )
{
}

void wxDrawingConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    a2dEditorMultiFrameViewConnector::OnPostCreateView( event );

    a2dCanvasView* drawer = ( a2dCanvasView* ) event.GetView();

    a2dCanvas* canvas = ( a2dCanvas* ) drawer->GetDisplayWindow();

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    canvas->SetYaxis( true );
    canvas->SetMappingUpp( -600, -600, 4, 4 );

    //defines the whole area in world coordinates that can be seen by scrolling
    canvas->SetScrollMaximum( -800, -800, 1600, 1600 );
    //defines the number of world units scrolled when line up or down events in X
    canvas->SetScrollStepX( 200 );
    //defines the number of world units scrolled when line up or down events in Y
    canvas->SetScrollStepY( 100 );
}


/*********************************************************************************/
/*Text view. */
/*********************************************************************************/

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
END_EVENT_TABLE()


void TextView::OnSetDocument( a2dViewEvent& event )
{
    m_top = ( ( a2dCanvasDocument* )GetDocument() )->GetDrawing()->GetRootObject();
}

void TextView::OnCreateView( a2dViewEvent& event )
{
    m_top = ( ( a2dCanvasDocument* )GetDocument() )->GetDrawing()->GetRootObject();
}


void TextView::OnUpdate( a2dView* sender, wxObject* hint )
{
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

    //GetDocument()->SaveObject( mem );

    a2dIOHandlerDocCVGOut CVGwriter;
    CVGwriter.SaveStartAt( mem, ( a2dCanvasDocument* ) GetDocument(), m_top );

    if ( m_display )
    {
        ( ( wxTextCtrl* )m_display )->Clear();
#if wxUSE_STD_IOSTREAM
        wxString str = mem.str();
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
    OnUpdate( this, 0 );
    return true;
}

BEGIN_EVENT_TABLE( wxTextViewFrame, a2dDocumentFrame )
    EVT_MENUSTRINGS( wxTextViewFrame::OnSetMenuStrings )
END_EVENT_TABLE()
wxTextViewFrame::wxTextViewFrame( TextView* view, const wxPoint& pos, const wxSize& size, wxFrame* parent )
    :  a2dDocumentFrame( ( bool ) false, parent, view, -1, wxT( "Child Frame" ), pos, size, wxDEFAULT_FRAME_STYLE )
{
#ifdef __WXMSW__
    SetIcon( wxString( wxT( "chrt_icn" ) ) );
#endif

    //// Make a menubar
    wxMenu* file_menu = new wxMenu;

    file_menu->Append( wxID_EXIT, _( "E&xit" ) );

    m_edit_menu = new wxMenu;
    m_edit_menu->Append( wxID_UNDO, _( "&Undo" ) );
    m_edit_menu->Append( wxID_REDO, _( "&Redo" ) );

    m_view = view;

    wxMenu* drawMenu = new wxMenu;
    drawMenu->Append( MSHOW_TOP, _( "object to show on textview" ), _( "choose top object to display" ) );

    wxMenuBar* menu_bar = new wxMenuBar;

    menu_bar->Append( file_menu, _T( "&File" ) );
    menu_bar->Append( m_edit_menu, _( "&Edit" ) );
    menu_bar->Append( drawMenu, _( "&Screens" ) );

    //// Associate the menu bar with the frame

    SetMenuBar( menu_bar );

    Centre( wxBOTH );

    SetTitle( wxT( "TextView" ) );

    m_textdisplay = new wxTextCtrl( this, -1, wxT( "empty" ), wxDefaultPosition, wxDefaultSize, wxTE_READONLY  | wxTE_MULTILINE );

    m_view->SetDisplayWindow( m_textdisplay );
}

void wxTextViewFrame::SetShowObject()
{
    a2dCanvasObjectsDialog groups( this, m_view->GetCanvasDocument()->GetDrawing(), true, true, ( wxSTAY_ON_TOP | wxRESIZE_BORDER | wxCAPTION ) );
    if ( groups.ShowModal() == wxID_OK )
    {
        m_view->SetShowObject( groups.GetCanvasObject() );
    }
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

/* shows how to external iterate on document
void TextViewWindow::Tostring( int indent, wxString& stringstream, a2dCanvasObject* obj )
{
    SingleTostring( indent, stringstream, obj );

    if ( obj->GetChildObjectList())
    {
        wxInt32 n = obj->GetChildObjectList()->GetCount();
        stringstream << obj->GetClassInfo()->GetClassName() << " " << n << '\n';

        for( a2dCanvasObjectList::iterator iter = obj->GetChildObjectList()->begin(); iter != obj->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject *obj = (*iter);
            SingleTostring( indent, stringstream, obj );
        }
    }
}

void TextViewWindow::SingleTostring( int indent, wxString& stringstream, a2dCanvasObject* obj )
{
    for (int i = 0; i < indent; i++)
        stringstream << wxT("    ");
    stringstream << obj->GetClassInfo()->GetClassName() << " ";
    stringstream << obj->GetPosX() << " " << obj->GetPosY() << " " ;

    if (0 != wxDynamicCast(obj, a2dCanvasObjectReference))
    {
        stringstream << "\n";
        Tostring( ++indent, stringstream, obj->GetChildObjectList()->front() );
        stringstream << "\n";
    }
    else if (0 != wxDynamicCast(obj, a2dPolygonL))
    {
        stringstream << "\n";
    }
    else if (0 != wxDynamicCast(obj, a2dPolylineL))
    {
        stringstream << "\n";
    }
    else if (0 != wxDynamicCast(obj, a2dRect))
    {
        a2dRect *rect = (a2dRect*) obj;

        stringstream << rect->GetWidth() << " " << rect->GetHeight() << "\n";
    }
    else if (0 != wxDynamicCast(obj, a2dCircle))
    {
        a2dCircle *cir = (a2dCircle*) obj;
        stringstream << cir->GetRadius() << "\n";
    }
    else if (0 != wxDynamicCast(obj, a2dEllipse))
    {
        a2dEllipse *ele = (a2dEllipse*) obj;
        stringstream << ele->GetWidth() << " " << ele->GetHeight() << "\n";
    }
    else if (0 != wxDynamicCast(obj, a2dArc))
    {
        a2dArc *arc = (a2dArc*) obj;
        stringstream << arc->GetX1() << " " << arc->GetY1() << " " << arc->GetX2() << " " << arc->GetY2() << "\n";
    }
    else if (0 != wxDynamicCast(obj, a2dEllipticArc))
    {
        a2dEllipticArc *arc = (a2dEllipticArc*) obj;
        stringstream << arc->GetWidth() << " " << arc->GetHeight() << " " << arc->GetStart() << " " << arc->GetEnd() << "\n";
    }
    else if (0 != wxDynamicCast(obj, a2dEndsLine))
    {
        a2dEndsLine *line = (a2dEndsLine*) obj;
        stringstream << line->GetPosX2() << " " << line->GetPosY2() << "\n";
    }
    else if (0 != wxDynamicCast(obj, a2dSLine))
    {
        a2dSLine *line = (a2dSLine*) obj;
        stringstream << line->GetPosX2() << " " << line->GetPosY2() << "\n";
    }
    else if (0 != wxDynamicCast(obj, a2dText))
    {
        a2dText *text = (a2dText*) obj;
        stringstream << text->GetText() << "\n";
    }
    else if (0 != wxDynamicCast(obj, a2dImage))
    {
        stringstream << "\n";
    }
    else
    {
        stringstream << "\n";
        Tostring( ++indent, stringstream, obj );
        stringstream << "\n";
    }
}
*/

