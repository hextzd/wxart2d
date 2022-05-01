/*! \file editor/samples/vdrawm/view.cpp
    \brief View classes
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: view.cpp,v 1.6 2009/09/26 19:01:19 titato Exp $
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

#include "view.h"


#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#endif //wxART2D_USE_AGGDRAWER

#if wxCHECK_VERSION(2, 5, 3)
#include "wx/sstream.h"
#endif

#include "wx/docview/doccom.h"

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

    a2dCanvasView* drawerview = ( a2dCanvasView* ) event.GetView();
#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
    a2dOglCanvas* canvas = wxStaticCast( drawerview->GetDisplayWindow(), a2dOglCanvas );
#else
    a2dCanvas* canvas = wxStaticCast( drawerview->GetDisplayWindow(), a2dCanvas );
#endif //wxUSE_GLCANVAS 

#if wxART2D_USE_AGGDRAWER
    if ( drawerview->GetViewTypeName() == wxT( "Drawing Anti Aliased View Frame" ) )
        canvas->GetDrawingPart()->SetDrawer2D( 
		    new a2dAggDrawer( 
				canvas->GetDrawingPart()->GetDrawer2D()->GetWidth(), 
				canvas->GetDrawingPart()->GetDrawer2D()->GetHeight() ) );

// The default a2dDrawer2D (a2dMemDcDrawer) is already created in a2dCanvasView its constructor,
// so the next is not needed currently.
    /*
        else
        {
            drawer->SetDrawer2D( new a2dMemDcDrawer( drawer->GetDrawer2D()->GetWidth(), drawer->GetDrawer2D()->GetHeight() ) );
        }
    */
#endif // wxART2D_USE_AGGDRAWER

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
    else
        wxLogMessage( _T( "wxTextConnector not a Text View Frame, no view was created" ) );

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

void TextView::OnCreateView( a2dViewEvent& )
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

#if wxART2D_USE_CVGIO
    a2dIOHandlerDocCVGOut CVGwriter;
    CVGwriter.SaveStartAt( mem, ( a2dCanvasDocument* ) GetDocument(), m_top );
#endif //wxART2D_USE_CVGIO

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
    OnUpdate( this, 0 );
    return true;
}

BEGIN_EVENT_TABLE( wxTextViewFrame, a2dDocumentFrame )
    EVT_MENUSTRINGS( wxTextViewFrame::OnSetMenuStrings )
END_EVENT_TABLE()

wxTextViewFrame::wxTextViewFrame( TextView* view, const wxPoint& pos, const wxSize& size, wxFrame* parent )
    :  a2dDocumentFrame( ( bool ) false, parent, view, -1, _T( "Child Frame" ), pos, size, wxDEFAULT_FRAME_STYLE )
{
#ifdef __WXMSW__
    SetIcon( wxString( _T( "chrt_icn" ) ) );
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
    m_edit_menu->Append( wxID_UNDO, _T( "&Undo" ) );
    m_edit_menu->Append( wxID_REDO, _T( "&Redo" ) );

    m_view = view;


    wxMenu* drawMenu = new wxMenu;
    drawMenu->Append( MSHOW_TOP, _T( "object to show on textview" ), _T( "choose top object to display" ) );

    wxMenuBar* menu_bar = new wxMenuBar;

    menu_bar->Append( file_menu, _T( "&File" ) );
    menu_bar->Append( m_edit_menu, _T( "&Edit" ) );
    menu_bar->Append( drawMenu, _T( "&Screens" ) );

    //// Associate the menu bar with the frame

    SetMenuBar( menu_bar );

    Centre( wxBOTH );

    SetTitle( _T( "TextView" ) );

    m_textdisplay = new wxTextCtrl( this, -1, _T( "empty" ), wxDefaultPosition, wxDefaultSize, wxTE_READONLY  | wxTE_MULTILINE );

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

void wxTextViewFrame::SetShowObject()
{
    a2dCanvasObjectsDialog groups( this, m_view->GetCanvasDocument()->GetDrawing(), true, false, ( wxSTAY_ON_TOP | wxRESIZE_BORDER | wxCAPTION ) );
    if ( groups.ShowModal() == wxID_OK )
    {
        m_view->SetShowObject( groups.GetCanvasObject() );
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

        a2dCanvasObjectList::compatibility_iterator node = obj->GetChildObjectList()->GetFirst();
        while (node)
        {
            a2dCanvasObject *obj = node->GetData();

            SingleTostring( indent, stringstream, obj );


            node = node->GetNext();
        }
    }
}

void TextViewWindow::SingleTostring( int indent, wxString& stringstream, a2dCanvasObject* obj )
{
    for (int i = 0; i < indent; i++)
        stringstream << wxT("    ") );
    stringstream << obj->GetClassInfo()->GetClassName() << " ";
    stringstream << obj->GetPosX() << " " << obj->GetPosY() << " " ;

    if (0 != wxDynamicCast(obj, a2dCanvasObjectReference))
    {
        stringstream << "\n";
        Tostring( ++indent, stringstream, obj->GetChildObjectList()->GetFirst()->GetData() );
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

