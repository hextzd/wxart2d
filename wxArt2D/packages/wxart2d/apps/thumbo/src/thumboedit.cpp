//////////////////////////////////////////////////////////////////////////////
// Name:        canedit.cpp
// Author:      Klaas Holwerda
// Created:     XX/XX/XX
// Copyright:   2000 (c) Klaas Holwerda
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/colordlg.h"

#if wxCHECK_VERSION(2, 5, 0)
#if !WXWIN_COMPATIBILITY_2_4
#include "wx/numdlg.h"
#endif
#endif

#include <wx/wfstream.h>
#include <wx/fontdlg.h>

#if wxUSE_XRC
#include <wx/xrc/xmlres.h>
#include <wx/docview/xh_a2dmenudoc.h>
#endif
 

#include "thumboedit.h"
#include "docv.h"
#include "viewcon.h"

#include "wx/canvas/booloper.h"

#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#include "wx/aggdrawer/graphicagg.h"
#endif //wxART2D_USE_AGGDRAWER

//int __cdecl _CrtSetDbgFlag( int a ) {
//    return 0;
//}
//
//void *_imp___CrtSetDbgFlag = _CrtSetDbgFlag;

/********************************************************************
* a2dCanvasThumboEditorFrame
*********************************************************************/

IMPLEMENT_DYNAMIC_CLASS( a2dCanvasThumboEditorFrame, whichEditorFrame )

BEGIN_EVENT_TABLE( a2dCanvasThumboEditorFrame, whichEditorFrame )
    EVT_MENU( wxID_EXIT, a2dCanvasThumboEditorFrame::Quit )
    EVT_MENU( wxID_ABOUT, a2dCanvasThumboEditorFrame::OnAbout )
    EVT_POST_LOAD_DOCUMENT( a2dCanvasThumboEditorFrame::OnPostLoadDocument )
    EVT_THEME_EVENT( a2dCanvasThumboEditorFrame::OnTheme )
    EVT_INIT_EVENT( a2dCanvasThumboEditorFrame::OnInit )
    EVT_ACTIVATE( a2dCanvasThumboEditorFrame::OnActivate )
    EVT_CLOSE( a2dCanvasThumboEditorFrame::OnCloseWindow )
    EVT_CHANGEDFILENAME_VIEW( a2dCanvasThumboEditorFrame::OnChangeFilename )
    EVT_COM_EVENT( a2dCanvasThumboEditorFrame::OnComEvent )
END_EVENT_TABLE()

//! canvas window to display the view upon to be defined by user
#define DOC m_view->GetCanvasDocument()

a2dCanvasThumboEditorFrame::a2dCanvasThumboEditorFrame( bool isParent, wxDrawingConnector* connector,
        wxFrame* parent, const wxPoint& pos, const wxSize& size, long style )
    : whichEditorFrame()
{
    m_initialized = false;
    m_connector = connector;

    // create a canvas in Create, the first arg. is true.
    Create( isParent, parent, pos, size, style );

    m_notebook = new ThumboNotebook( this );

    a2dViewCanvas* canvas = new a2dViewCanvas( NULL, m_notebook, -1, wxPoint( 0, 0 ),  size, wxScrolledWindowStyle | wxNO_FULL_REPAINT_ON_RESIZE );
    canvas->SetCursor( wxCURSOR_PENCIL );
    //following true on start up is not so good ;-), do it later
    //when everything is on the screen (problem is size events generated while intializing a window)
    canvas->SetScaleOnResize( false );

    //defines the whole area in world coordinates that can be seen by scrolling
    canvas->SetScrollMaximum( 0, 0, 1000, 1000 );
    //defines the number of world units scrolled when line up or down events in X
    canvas->SetScrollStepX( 100 );
    //defines the number of world units scrolled when line up or down events in Y
    canvas->SetScrollStepY( 100 );
    //or this way
    //m_canvas->SetScrollbars(20,20,50,50,0,0);

    //do not go outide the scroll maximum
    //m_canvas->FixScrollMaximum(true);

    m_drawingPart = canvas->GetDrawingPart();

    m_notebook->AddPage( canvas, wxT( "window1" ), true );

    Init();
}

void a2dCanvasThumboEditorFrame::OnCloseWindow( wxCloseEvent& event )
{
    bool exit = false;
    int j = 0;
    for ( wxWindowList::const_iterator i = wxTopLevelWindows.begin(),
            end = wxTopLevelWindows.end();
            i != end;
            ++i )
    {
        wxTopLevelWindow* const win = wx_static_cast( wxTopLevelWindow*, *i );
        if ( win->IsShown() )
        {
            a2dCanvasThumboEditorFrame* const ewin = wx_static_cast( a2dCanvasThumboEditorFrame*, win );
            if ( ewin )
                j++;
        }
    }
    if ( j == 1 )
        exit = true;


    if ( event.CanVeto() )
    {
        if ( !exit )
            //if ( !GetIsParentFrame() )
        {
            if ( !m_view || m_view->Close( !event.CanVeto() ) )
            {
                Destroy();
                m_view = NULL;
            }
            else
                event.Veto( true );
        }
        else
        {
            if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->Exit( !event.CanVeto() ) )
            {
                event.Veto( true );
                return;
            }
            //if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear( !event.CanVeto() ) )
            //    event.Veto( true );
        }
    }
    else
    {
        if ( GetIsParentFrame() )
        {
            a2dDocviewGlobals->GetDocviewCommandProcessor()->Exit( true );
        }
        else
            Destroy();
    }
}

void a2dCanvasThumboEditorFrame::OnActivate( wxActivateEvent& event )
{
    if ( !m_view )
        return;
    if ( event.GetActive() )
    {
        m_connector->m_currentActiveEditorFrame = this;
    }
}

void a2dCanvasThumboEditorFrame::OnTheme( a2dEditorFrameEvent& themeEvent )
{
    //Theme();
    CreateThemeXRC();

#if (wxART2D_USE_LUA == 1)
    a2dLuaEditorFrame::OnTheme( themeEvent );
#else
    //a2dEditorFrame::OnTheme( themeEvent );
#endif //(wxART2D_USE_LUA == 1)

    themeEvent.Skip( false );
}

void a2dCanvasThumboEditorFrame::CreateThemeXRC()
{
    CreateStatusBarThemed( wxT( "default" ) );

    m_menuBar = new wxMenuBar;
    SetMenuBar( m_menuBar );

#if wxUSE_XRC
    if( wxXmlResource::Get() )
    {
        wxXmlResource::Get()->InitAllHandlers();
        wxXmlResource::Get()->AddHandler( new a2dMenuBarXmlHandlerDoc );
        wxXmlResource::Get()->AddHandler( new a2dMenuXmlHandlerDoc );
        wxXmlResource::Get()->AddHandler( new a2dToolBarXmlHandlerDoc );

        wxString art2d = a2dGeneralGlobals->GetWxArt2DVar( true );
        art2d += wxT( "art/resources/" );

//      wxString aResName = wxString::Format(wxT("../resource/%s.xrc"),GetAppName().c_str());
        wxString aResName = wxT( "a2dEditorFrame.xrc" );
        if( !wxXmlResource::Get()->Load( art2d + aResName ) )
            wxMessageBox( wxString::Format( _( "Error load XRC-file '%s'.\nWorking directory is '%s'" ), aResName.c_str(), wxGetCwd().c_str() ) );
    }

    wxXmlResource::Get()->LoadObject( this, wxT( "frameMenu" ), wxT( "a2dMenuBar" ) );
    wxXmlResource::Get()->LoadObject( this, wxT( "toolbar" ), wxT( "a2dToolBar" ) );
#endif

    wxMenu *menuClipboard = new wxMenu;
    menuClipboard->Append( a2dShapeClipboard_Copy, wxT("&Copy\tCtrl-C"));
    menuClipboard->Append( a2dShapeClipboard_Paste, wxT("&Paste\tCtrl-V"));
    menuClipboard->Append( a2dShapeClipboard_Drag, wxT("&Drag"));

    m_menuBar->Append( menuClipboard, wxT("&Clipboard"));
}


void a2dCanvasThumboEditorFrame::OnInit( a2dEditorFrameEvent& initEvent )
{
    initEvent.Skip();
}

a2dCanvasThumboEditorFrame::~a2dCanvasThumboEditorFrame()
{
}

void a2dCanvasThumboEditorFrame::Quit( wxCommandEvent& event )
{
    Close( true );
    //event.Skip();
}

void a2dCanvasThumboEditorFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "a2dCanvasThumboEditorFrame\n Klaas Holwerda 2009" ),
                          wxT( "About a2dCanvasThumboEditorFrame" ), wxICON_INFORMATION | wxOK );
}

void a2dCanvasThumboEditorFrame::OnPostLoadDocument( a2dDocumentEvent& event )
{
    if ( m_view->GetDocument()  != event.GetEventObject() )
        return;

    m_drawingPart->SetMappingShowAll();
}

void a2dCanvasThumboEditorFrame::OnChangeFilename( a2dViewEvent& event )
{
    a2dDocument* doc = ( ( a2dView* )event.GetEventObject() )->GetDocument();

    if ( doc )
    {
        wxString title = doc->GetFilename().GetFullName();

        SetTitle( title );
        m_notebook->SetPageText( m_notebook->GetSelection(), title );
    }
}

void a2dCanvasThumboEditorFrame::OnComEvent( a2dComEvent& event )
{
    if ( m_drawingPart && event.GetId() == a2dStToolContr::sm_showCursor && GetStatusBar() )
    {
        a2dStToolContr* contr = wxStaticCast( event.GetEventObject(), a2dStToolContr );

        /*
        wxPoint mpos = contr->GetMousePosition();

        double mouse_worldx, mouse_worldy;
        contr->GetDrawingPart()->MouseToToolWorld( mpos.x, mpos.y, mouse_worldx, mouse_worldy );

        wxString unitmeasure = contr->GetDrawingPart()->GetDrawing()->GetUnits();
        double scaleToMeters = contr->GetDrawingPart()->GetDrawing()->GetUnitsScale();

        double multi = 1;
        a2dDoMu::GetMultiplierFromString( unitmeasure, multi );

        mouse_worldx *= scaleToMeters / multi;
        mouse_worldy *= scaleToMeters / multi;

        wxString str;
        str.Printf( _T( "%d %d -> %6.3f,%6.3f" ), mpos.x, mpos.y, mouse_worldx, mouse_worldy );

        if  ( contr->GetFirstTool() )
            str = str + _T( " " ) + unitmeasure + _T( " :" ) + contr->GetFirstTool()->GetClassInfo()->GetClassName();

        str = str + _T( " :" ) + a2dCanvasGlobals->GetConnectionGenerator()->GetRouteMethodAsString();

        SetStatusText( str );
        wxUint16 lay = a2dCanvasGlobals->GetLayer();
        if ( m_drawingPart )
        {    
            wxUint16 lay = a2dCanvasGlobals->GetLayer();
            SetStatusText( m_drawingPart->GetDrawing()->GetLayerSetup()->GetName( lay ), 1 );
        }
        */
        SetStatusText( contr->GetStatusStrings()[0], 0 );
        SetStatusText( contr->GetStatusStrings()[1], 1 );
        SetStatusText( contr->GetStatusStrings()[2], 2 );
        SetStatusText( contr->GetStatusStrings()[3], 3 );
        SetStatusText( a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->GetRouteMethodAsString(), 4 );
        wxUint16 lay = a2dCanvasGlobals->GetHabitat()->GetLayer();
        wxString layername = "";
        if ( contr->GetDrawingPart()->GetDrawing()->GetLayerSetup() )
            layername = contr->GetDrawingPart()->GetDrawing()->GetLayerSetup()->GetName( lay );
        SetStatusText( layername, 5 );
    }
    else if ( event.GetId() == a2dHabitat::sig_changedLayer )
    {
        a2dStToolContr* contr = wxStaticCast( event.GetEventObject(), a2dStToolContr );
        wxUint16 lay = a2dCanvasGlobals->GetHabitat()->GetLayer();
        wxString layername = "";
        if ( contr->GetDrawingPart()->GetDrawing()->GetLayerSetup() )
            layername = contr->GetDrawingPart()->GetDrawing()->GetLayerSetup()->GetName( lay );
        SetStatusText( layername, 5 );
    }
    else
        event.Skip();
}


// ----------------------------------------------------------------------------
// ThumboNotebook
// ----------------------------------------------------------------------------
const int wxThumboNotebookWindowId = 5380;
BEGIN_EVENT_TABLE( ThumboNotebook, NoteBookType )

    EVT_COMMAND_RANGE( wxThumboNotebookWindowId, wxThumboNotebookWindowId + 500,
                       wxEVT_COMMAND_AUINOTEBOOK_BEGIN_DRAG,
                       ThumboNotebook::OnTabBeginDrag )
    EVT_COMMAND_RANGE( wxThumboNotebookWindowId, wxThumboNotebookWindowId + 500,
                       wxEVT_COMMAND_AUINOTEBOOK_END_DRAG,
                       ThumboNotebook::OnTabEndDrag )
    EVT_COMMAND_RANGE( wxThumboNotebookWindowId, wxThumboNotebookWindowId + 500,
                       wxEVT_COMMAND_AUINOTEBOOK_DRAG_MOTION,
                       ThumboNotebook::OnTabDragMotion )
    EVT_AUINOTEBOOK_PAGE_CHANGING( wxID_ANY, ThumboNotebook::OnChangingPage )
    EVT_AUINOTEBOOK_PAGE_CLOSE( wxID_ANY, ThumboNotebook::OnPageClose )
    //EVT_AUINOTEBOOK_PAGE_CLOSED( wxID_ANY, ThumboNotebook::OnPageClose )
    EVT_AUINOTEBOOK_PAGE_CHANGED( wxID_ANY, ThumboNotebook::OnChangedPage )
    //EVT_NOTEBOOK_PAGE_CLOSED( wxThumboNotebookWindowId, ThumboNotebook::OnPageClose )
    EVT_CLOSE_VIEW( ThumboNotebook::OnCloseView )
	EVT_SET_FOCUS( ThumboNotebook::OnSetFocus )
	EVT_KILL_FOCUS( ThumboNotebook::OnKillFocus )

END_EVENT_TABLE()

ThumboNotebook::ThumboNotebook( a2dDocumentFrame* parent )
    : NoteBookType( parent, -1, wxDefaultPosition, wxDefaultSize )
{
    m_dragToFrame = true;
    m_busyClosingTab = true;

    m_dragViewFrame = NULL;
}

void ThumboNotebook::OnChangingPage( wxAuiNotebookEvent& evt )
{
    if ( evt.GetOldSelection() >= 0 )
    {
        wxWindow* oldwin = GetPage( evt.GetOldSelection() );
        if (  oldwin )
        {
            a2dView* oldview = ( ( a2dViewCanvas* )oldwin )->GetView();
            if ( oldview )
                oldview->Activate( false );
        }
    }
    if ( evt.GetSelection() >= 0 )
    {
        wxWindow* win = GetPage( evt.GetSelection() );
        if ( win )
        {
            a2dView* view = ( ( a2dViewCanvas* )win )->GetView();
            if ( view )
                view->Activate( true );
        }
    }
    evt.Skip();
}

void ThumboNotebook::OnChangedPage( wxAuiNotebookEvent& evt )
{
    if ( evt.GetOldSelection() >= 0 )
    {
        wxWindow* win = GetPage( evt.GetSelection() );
        if ( win )
        {
            a2dView* view = ( ( a2dViewCanvas* )win )->GetView();

            if ( view )
                view->Activate( true );
        }
    }
    evt.Skip();
}
 
void ThumboNotebook::OnPageClose( wxAuiNotebookEvent& evt )
{
    a2dViewCanvas* canvas = wxDynamicCast( GetPage( evt.GetSelection()), a2dViewCanvas);
    a2dCanvasView* view = canvas->GetView();

    m_busyClosingTab = true;

    if ( view )
    {
        view->GetDocument()->Close( true );
    }
    m_busyClosingTab = false;
}

void ThumboNotebook::OnSetFocus( wxFocusEvent& focusevent )
{
    wxLogDebug( "ThumboNotebook focus %p", this );
    wxWindow* win = GetCurrentPage ();
    if ( win )
    {
        //win->SetFocus();
    }

	focusevent.Skip();
}

void ThumboNotebook::OnKillFocus( wxFocusEvent& focusevent )
{
    wxLogDebug( "ThumboNotebook off focus %p", this );
    wxWindow* win = focusevent.GetWindow();
    if ( win )
    {
    }
	focusevent.Skip();
}

void ThumboNotebook::OnTabBeginDrag( wxCommandEvent& event )
{
    m_dragViewFrame = NULL;
    event.Skip();
}

void ThumboNotebook::OnCloseView( a2dCloseViewEvent& event )
{
    a2dView* view = wxDynamicCast( event.GetEventObject(), a2dView );

    if ( view )
    {
		// find page with this view
		int index = GetPageIndex( view->GetDisplayWindow() );
		if ( index != wxNOT_FOUND )
			DeletePage( index ); 
    }

    if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetBusyExit() )
        GetParent()->Destroy();
}

bool ThumboNotebook::CaptionHit()
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

void ThumboNotebook::OnTabDragMotion( wxCommandEvent&  evt )
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
                /*
                                //wxLogDebug( "out" );
                                a2dDocumentViewScrolledWindow* pagewindow = ((a2dDocumentViewScrolledWindow*)GetPage( evt.GetSelection() ));
                                a2dView* viewpage = pagewindow->GetView();
                                a2dCanvasView* viewNew = new a2dCanvasView();
                                viewNew->SetDocument( viewpage->GetDocument() );

                                m_dragViewFrame = new MyFrame( false, wxGetApp().m_notebookconnector, (DrawingView*) viewNew,  NULL, -1, wxT("Tabframe with docmanager"),
                                                       wxPoint( screen_pt.x -5, screen_pt.y -5 ), wxSize(500, 400), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE );

                                //m_dragViewFrame = new EditorFrame( false, (a2dDocumentFrame*)GetParent(), (DrawingView*) viewNew, -1,
                                //         wxT("Frame with docmanager"), wxPoint( screen_pt.x -5, screen_pt.y -5 ), wxSize(500,500), wxDEFAULT_FRAME_STYLE );
                                m_dragViewFrame->SetTransparent(155);
                                m_dragViewFrame->Show();
                                m_dragViewFrame->Raise();
                                m_dragViewFrame->SetFocus();
                */
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

void ThumboNotebook::OnTabEndDrag( wxCommandEvent&  evt )
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
