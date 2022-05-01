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

#include "viewcon.h"
#include "thumboedit.h"
#include "docv.h"
#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#endif //wxART2D_USE_AGGDRAWER

IMPLEMENT_CLASS( wxDrawingConnector, a2dFrameViewConnector )
IMPLEMENT_CLASS( wxTextConnector, a2dFrameViewConnector )

BEGIN_EVENT_TABLE( wxDrawingConnector, a2dFrameViewConnector )
    EVT_POST_CREATE_DOCUMENT( wxDrawingConnector::OnPostCreateDocument )
    EVT_POST_CREATE_VIEW( wxDrawingConnector::OnPostCreateView )
    EVT_DISCONNECT_VIEW( wxDrawingConnector::OnDisConnectView )
END_EVENT_TABLE()

wxDrawingConnector::wxDrawingConnector()
    : a2dFrameViewConnector()
{
    m_lastMadeEditorFrame = NULL;
}

a2dObject* wxDrawingConnector::SmrtPtrOwn()
{
    m_refcount++; return this;
}

bool wxDrawingConnector::SmrtPtrRelease()
{
    m_refcount--;
    wxASSERT_MSG( m_refcount >= 0, wxT( "a2dObject Own/Release not matched (extra Release calls)" ) );
    if ( m_refcount <= 0 )
    {
        delete this;
        return true;
    }
    return false;
}

void wxDrawingConnector::OnDisConnectView(  a2dTemplateEvent& event )
{
    a2dSmrtPtr<a2dView> theViewToDisconnect = ( a2dView* ) event.GetEventObject();
    a2dCanvasView* drawer = ( a2dCanvasView* ) theViewToDisconnect.Get();

    if ( drawer->GetViewTypeName() == wxT( "Drawing View Frame" ) || a2dDocviewGlobals->GetDocviewCommandProcessor()->GetBusyExit()  )
    {
        theViewToDisconnect->Close( true );
    }
    else if ( drawer->GetViewTypeName() == wxT( "Drawing View NoteBook Page" ) )
    {
        if ( m_currentActiveEditorFrame->GetNoteBook()->IsClosingTab() )
        {
            // keep the view and its display window, only remove it from the document.
            theViewToDisconnect->Activate( false );
            if ( theViewToDisconnect->GetDocument() )
            {
                a2dDocumentEvent eventremove( wxEVT_REMOVE_VIEW, theViewToDisconnect, true, theViewToDisconnect->GetId() );
                eventremove.SetEventObject( theViewToDisconnect );
                theViewToDisconnect->GetDocument()->ProcessEvent( eventremove );
            }
            drawer->GetDrawingPart()->GetCanvasToolContr()->StopAllTools();
            theViewToDisconnect->SetDisplayWindow( 0 );
            theViewToDisconnect->SetDocument( NULL );
        }
        else
        {
            drawer->GetDrawingPart()->GetCanvasToolContr()->StopAllTools();
            theViewToDisconnect->Close( true );
        }
    }
}

void wxDrawingConnector::OnPostCreateDocument( a2dTemplateEvent& event )
{
    //a2dDocumentTemplate* templ = (a2dDocumentTemplate*) event.GetEventObject();
    //a2dDocument *doc = event.GetDocument();

    a2dViewTemplate* viewtempl = NULL;
    if ( !m_lastMadeEditorFrame )
    {
        viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->SelectViewType( event.GetDocument(),
                    a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates(), wxT( "Drawing View NoteBook Page" ) );
    }
    else
    {
        viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->SelectViewType( event.GetDocument(),
                    a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates() );
    }

    if ( !viewtempl )
    {
        wxLogMessage( wxT( "No view available for this document in a2dEditorMultiFrameViewConnector" ) );
        return;
    }

    if ( !viewtempl->CreateView( event.GetDocument(), event.GetFlags() ) )
    {
        wxLogMessage( wxT( "a2dEditorMultiFrameViewConnector no view was created" ) );
        return;
    }
}


void wxDrawingConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    a2dCanvasView* drawer = ( a2dCanvasView* ) event.GetView();

    a2dStToolContr* contr = NULL;
    a2dViewCanvas* canvas = NULL;

    if ( drawer->GetViewTypeName() == wxT( "Drawing View Frame" ) )
    {
        //a2dCanvasDocument* doc = (a2dCanvasDocument*) event.GetView()->GetDocument();

        m_lastMadeEditorFrame = new a2dCanvasThumboEditorFrame( false,
                this,
                m_docframe,
                GetInitialPosition(),
                GetInitialSize(),
                GetInitialStyle()
                                                              );
        m_lastMadeEditorFrame->Enable();
        m_lastMadeEditorFrame->SetTitle( wxT( "Thumbo editor" ) );
        m_lastMadeEditorFrame->Show( true );
        ThumboNotebook* notebook = m_lastMadeEditorFrame->GetNoteBook();
        //canvas = wxDynamicCast( notebook->GetCurrentPage(), a2dViewCanvas);
        canvas = wxDynamicCast( notebook->GetPage(0), a2dViewCanvas);
        canvas->SetView( drawer );
        //canvas->GetDrawingPart()->SetBufferSize( m_initialSize.GetWidth(), m_initialSize.GetHeight() );
        contr = m_lastMadeEditorFrame->GetToolController();
    }
    else if ( drawer->GetViewTypeName() == wxT( "Drawing View NoteBook Page" ) )
    {
        if( !m_lastMadeEditorFrame )
        {
            m_lastMadeEditorFrame = new a2dCanvasThumboEditorFrame( false,
                    this,
                    m_docframe,
                    GetInitialPosition(),
                    GetInitialSize(),
                    GetInitialStyle()
                                                                  );
            m_lastMadeEditorFrame->Enable();
            m_lastMadeEditorFrame->SetTitle( wxT( "Thumbo editor" ) );
            m_lastMadeEditorFrame->Show( true );
            m_currentActiveEditorFrame = m_lastMadeEditorFrame;
            ThumboNotebook* notebook = m_lastMadeEditorFrame->GetNoteBook();
	        canvas = wxDynamicCast( notebook->GetPage( notebook->GetSelection() ), a2dViewCanvas);
            //canvas = wxDynamicCast( notebook->GetCurrentPage(), a2dViewCanvas);
            canvas->SetView( drawer );
            m_currentActiveEditorFrame->SetDrawingPart( canvas->GetDrawingPart() );
            //canvas->GetDrawingPart()->SetBufferSize( m_initialSize.GetWidth(), m_initialSize.GetHeight() );
            contr = m_lastMadeEditorFrame->GetToolController();
        }
        else
        {
            ThumboNotebook* notebook = m_currentActiveEditorFrame->GetNoteBook();
            canvas = new a2dViewCanvas( drawer, notebook, -1, wxDefaultPosition, m_currentActiveEditorFrame->GetSize(), wxScrolledWindowStyle | wxNO_FULL_REPAINT_ON_RESIZE );
            canvas->SetView( drawer );
            notebook->AddPage( canvas, drawer->GetDocument()->GetFilename().GetName(), true );

            contr  = new a2dStToolContr( canvas->GetDrawingPart(), m_currentActiveEditorFrame );
            m_currentActiveEditorFrame->SetDrawingPart( canvas->GetDrawingPart() );
            m_currentActiveEditorFrame->SetToolController( contr );
            contr->SetZoomFirst( true );

            canvas->Update();
        }
    }

    // Give it scrollbars
    canvas->SetScrollbars( 20, 20, 50, 50 );
    canvas->SetBackgroundColour( *wxWHITE );
    canvas->ClearBackground();

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    canvas->SetMappingUpp( -600, -600, 4, 4 );

    //defines the whole area in world coordinates that can be seen by scrolling
    canvas->SetScrollMaximum( -1000, -1000, 2000, 2000 );
    //defines the number of world units scrolled when line up or down events in X
    canvas->SetScrollStepX( 100 );
    //defines the number of world units scrolled when line up or down events in Y
    canvas->SetScrollStepY( 100 );

    int width = canvas->GetDrawingPart()->GetDrawer2D()->GetWidth();
    int height = canvas->GetDrawingPart()->GetDrawer2D()->GetHeight();
    canvas->SetGridStroke( a2dStroke( wxColour( 239, 5, 64 ), 0, a2dSTROKE_DOT ) );
    
#if wxART2D_USE_AGGDRAWER
    canvas->GetDrawingPart()->SetDrawer2D( new a2dAggDrawerRgba( width, height ) );
    //canvas->GetDrawingPart()->SetDrawer2D( new a2dMemDcDrawer( width, height ) );
#else    
    canvas->GetDrawingPart()->SetDrawer2D( new a2dMemDcDrawer( width, height ) );
#endif    
    canvas->GetDrawingPart()->GetDrawer2D()->SetYaxis( true );
    contr->SetOpacityFactorEditcopy( 125 );
    contr->SetUseOpaqueEditcopy( true );
    //to show grid as lines
    //m_canvas1->SetGridLines(true);

    canvas->SetGridSize( 1 );
    canvas->SetGridX( 10 );
    canvas->SetGridY( 10 );

    //show it?
    canvas->SetGrid( false );
    canvas->SetGridAtFront( true );

    contr->GetFirstTool()->SetShowAnotation( false );
    //contr->SetOpacityFactorEditcopy( 125 );
    //contr->SetUseOpaqueEditcopy( true );

    contr->SetZoomFirst( false );

    //a2dMasterDrawZoomFirst* tool = new a2dMasterDrawZoomFirst( contr );
    a2dMasterTagGroups3Link* tool = new a2dMasterTagGroups3Link( contr );
    tool->SetDlgOrEdit(true );
    tool->SetAllowDoubleClickModifier( true );
    tool->SetDlgOrEditModal( false );
    tool->SetWireMode( true );

    //tool->SetFastTools( true );
    contr->StopAllTools();
    contr->SetTopTool( tool );


    ThumboDocument* doc = (ThumboDocument*) (drawer->GetDocument() );

    a2dCameleon* cam = doc->GetTopCameleon();
    //a2dCanvasObject* root = cam->GetAppearance<a2dDiagram>()->GetDrawing()->GetRootObject();
    if ( cam )
	    canvas->GetDrawingPart()->SetShowObject( cam );

	//canvas->GetDrawingPart()->SetShowObject( root );
    if ( doc->GetShowObject() )
	    canvas->GetDrawingPart()->SetShowObject( doc->GetShowObject() );
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

    a2dIOHandlerCVGOut CVGwriter;
    CVGwriter.SaveStartAt( mem, (( a2dCanvasDocument* ) GetDocument())->GetDrawing(), m_top );

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
    a2dWalker_CollectCanvasObjects collector;
    collector.Start( m_view->GetCanvasDocument()->GetDrawing()->GetRootObject() );

    a2dCanvasObjectsDialog groups( this, &collector.m_found, true, ( wxSTAY_ON_TOP | wxRESIZE_BORDER | wxCAPTION  ) );
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

