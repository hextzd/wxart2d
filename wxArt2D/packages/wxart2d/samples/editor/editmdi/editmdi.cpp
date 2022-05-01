/*! \file editor/samples/editmdi/editmdi.cpp
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: editmdi.cpp,v 1.10 2009/09/26 19:01:19 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "editmdi.h"

#include "wx/colordlg.h"
#include <wx/wfstream.h>
#include "wx/editor/xmlparsdoc.h"

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

IMPLEMENT_APP( MyApp )

// ----------------------------------------------------------------------------
// MyCanvasDocument
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( MyCanvasDocument, a2dCanvasDocument )

BEGIN_EVENT_TABLE( MyCanvasDocument, a2dCanvasDocument )
    EVT_NEW_DOCUMENT( MyCanvasDocument::OnNewDocument )
    EVT_REMOVE_VIEW( MyCanvasDocument::OnRemoveView )
END_EVENT_TABLE()


MyCanvasDocument::MyCanvasDocument(): a2dCanvasDocument()
{
    //to ask for save when a view is closed.
    m_askSaveWhenOwned = true;
}

MyCanvasDocument::MyCanvasDocument( const MyCanvasDocument& other )
    : a2dCanvasDocument( other )
{
}

MyCanvasDocument* MyCanvasDocument::Clone()
{
    MyCanvasDocument* a = new MyCanvasDocument( *this );
    return a;
};

MyCanvasDocument::~MyCanvasDocument()
{
}

void MyCanvasDocument::OnNewDocument( a2dDocumentEvent& event )
{
    //fill it with something

	a2dCanvasObject* root = m_drawing->GetRootObject();

    a2dRect* r = new a2dRect( -50, -40, 800, 600 );
    r->SetFill( *a2dTRANSPARENT_FILL );
    r->SetStroke( wxColour( 255, 255, 0 ), 2 );
    r->SetHitFlags( a2dCANOBJ_EVENT_STROKE );
    root->Append( r );

    int i;
    for ( i = 10; i < 300; i += 10 )
    {
        a2dCircle* circ = new a2dCircle( i, 70, rand() % 84 );
        circ->SetFill( wxColour( rand() % 254, rand() % 254, rand() % 254 ) );
        root->Append( circ );
    }

    //make a group of a2dCanvasObjects
    a2dCanvasObject* group2 = new a2dCanvasObject( 0, 200 );
    group2->SetName( _T( "group2" ) );
    {
        a2dRect* tr = new a2dRect( 0, -100, 500, 300 );
        tr->SetStroke( wxColour( 9, 1, 64 ), 0 );
        tr->SetFill( wxColour( 100, 7, 5 ), a2dFILL_VERTICAL_HATCH );
        group2->Append( tr );

        a2dSLine* ll = new a2dSLine( 50, 0, 240, 100 );
        ll->SetStroke( wxColour( 252, 0, 2 ), 20.0 );
        group2->Append( ll );

        a2dPin* pin1 = new a2dPin( group2, _T( "pin1" ) , a2dPinClass::Standard, 0, 0, 180 );
        group2->Append( pin1 );

        a2dPin* pin2 = new a2dPin( group2, _T( "pin2" ) , a2dPinClass::Standard, 500, 80, 0 );
        group2->Append( pin2 );

        a2dPin* pin3 = new a2dPin( group2, _T( "pin3" ) , a2dPinClass::Standard, 150, -100, -90 );
        group2->Append( pin3 );
    }
    root->Prepend( group2 );

    a2dCanvasObject* group3 = new a2dCanvasObject( 500, 1100 );
    group3->SetName( _T( "group3" ) );
    group3->Rotate( 30 );
    {
        a2dRect* tr = new a2dRect( 0, -100, 500, 210 );
        tr->SetStroke( wxColour( 9, 1, 64 ), 0 );
        tr->SetFill( wxColour( 100, 237, 5 ), a2dFILL_VERTICAL_HATCH );
        group3->Append( tr );
        a2dPin* pin1 = new a2dPin( group3, _T( "pin1" ) , a2dPinClass::Standard, 0, 50, 20 );
        group3->Append( pin1 );

        a2dPin* pin2 = new a2dPin( group3, _T( "pin2" ) , a2dPinClass::Standard, 500, 80, 0 );
        group3->Append( pin2 );
    }
    root->Prepend( group3 );
    //group3->ConnectWith( m_drawing, group2, _T("pin1"));


    a2dText* tt = new a2dText( _T( "Label text" ) , 0, 0,
                               a2dFont( 20.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ) );
    tt->SetFill( wxColour( 180, 10, 20 ) );
    tt->SetStroke( *wxGREEN );
    tt->SetName( _T( "trial and error" ) );
    root->Append( tt );


    event.Skip();
}

void MyCanvasDocument::OnRemoveView( a2dDocumentEvent& event )
{
    //a2dView* viewtoremove = (a2dView*) event.GetView();

    if ( m_refcount <= 2  ) //refcount will become zero
    {
        if ( SaveModifiedDialog() != wxCANCEL )//default ask user to save or not.
        {
            //if saved or not, that is what the users wants.
            //Still the view will be removed
            //close document via commandprocessor, in order to remove it from there.

            //next not really needed, but if you want to know, you can.
            if ( !IsClosed() ) //busy closing are already closed in some other way, e.g closing application.
                a2dDocviewGlobals->GetDocviewCommandProcessor()->FileClose();
        }
        else
            event.Veto();
    }
}

// ----------------------------------------------------------------------------
// a2dCanMdiFrameDrawingConnector
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dCanMdiFrameDrawingConnector, a2dViewConnector )
    EVT_POST_CREATE_DOCUMENT( a2dCanMdiFrameDrawingConnector::OnPostCreateDocument )
    EVT_POST_CREATE_VIEW( a2dCanMdiFrameDrawingConnector::OnPostCreateView )
END_EVENT_TABLE()

a2dCanMdiFrameDrawingConnector::a2dCanMdiFrameDrawingConnector( wxClassInfo* EditorClassInfo )
    : a2dViewConnector()
{
    m_docframe = ( a2dDocumentMDIParentFrame* ) NULL;
    m_editorClassInfo = EditorClassInfo;
}

a2dCanMdiFrameDrawingConnector::~a2dCanMdiFrameDrawingConnector()
{
}

void a2dCanMdiFrameDrawingConnector::OnPostCreateDocument( a2dTemplateEvent& event )
{
    a2dViewTemplate* viewtempl;
    viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->SelectViewType( event.GetDocument(),
                m_viewTemplates.size() ? m_viewTemplates : a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates() );

    if ( !viewtempl )
    {
        wxLogMessage( _( "No view available for this document in a2dCanMdiFrameDrawingConnector" ) );
        return;
    }

    if ( !viewtempl->CreateView( event.GetDocument(), event.GetFlags() ) )
    {
        wxLogMessage( _( "a2dCanMdiFrameDrawingConnector no view was created" ) );
        return;
    }
}

void a2dCanMdiFrameDrawingConnector::Init( a2dDocumentMDIParentFrame* frame )
{
    m_docframe = frame;
}

void a2dCanMdiFrameDrawingConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    a2dCanvasView* view = wxDynamicCast( event.GetView(), a2dCanvasView );

    a2dCanvasDocument* doc = wxDynamicCast( event.GetView()->GetDocument(), a2dCanvasDocument );

    if ( view->GetViewTypeName() == wxT( "a2dCanvas + view" ) )
    {
        MDIEditorFrame* viewFrame;
        viewFrame = new MDIEditorFrame( m_docframe, -1,
                                        wxT( "Mdi Canvas child" ), GetInitialPosition(), GetInitialSize(), wxDEFAULT_FRAME_STYLE );

        viewFrame->GetCanvas()->GetDrawing()->GetCommandProcessor()->Initialize();

        viewFrame->GetCanvas()->SetBackgroundColour( wxColour( 12, 240, 230 ) );
        viewFrame->GetCanvas()->SetView( view );
		viewFrame->GetCanvas()->GetDrawingPart()->SetShowObject( doc->GetDrawing()->GetRootObject() );
        viewFrame->Enable();
        viewFrame->Show( true );


        ( ( a2dCanvasView* ) view )->GetDrawingPart()->SetMappingShowAll();

    }


    m_docframe->Refresh();
}


//-----------------------------------------------------------------------------
// MyApp
//-----------------------------------------------------------------------------

bool MyApp::OnInit()
{
    wxString artroot = a2dGeneralGlobals->GetWxArt2DVar();
    if ( artroot.IsEmpty() )
        return false;

    wxInitAllImageHandlers();

    //_CrtSetBreakAlloc(1744);

    a2dGlobals->GetFontPathList().Add( artroot + wxT( "samples/editor/common/fonts" ) );
    a2dGlobals->GetImagePathList().Add( artroot +  wxT( "samples/editor/common/images" ) );
    a2dGlobals->GetFontPathList().Add(  artroot +  wxT( "samples/editor/common/fonts" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "art/resources" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "apps/teto/config" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./" ) );
    a2dCanvasGlobals->GetLayersPathList().Add( artroot +  wxT( "art/layers" ) );

    a2dCanvasGlobals->GetHabitat()->LoadLayers( wxT( "layersdefault.cvg" ) );
    //a2dCanvasGlobals->LoadLayers( _T("layersdefault2.cvg") );

    a2dGlobals->GetFontPathList().Add( wxT( "./config" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./config" ) );
    a2dGlobals->GetIconPathList().Add( wxT( "./config" ) );
    a2dGlobals->GetFontPathList().Add( wxT( "../config" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "../config" ) );
    a2dGlobals->GetIconPathList().Add( wxT( "../config" ) );


    // we create our own, to enable a language wrapper.
    //a2DocumentCommandProcessor* docmanager = a2dDocviewGlobals->GetDocviewCommandProcessor();
    a2dGlobals->GetFontPathList().Add( _T( "../common/fonts" ) );
    a2dGlobals->GetImagePathList().Add( _T( "../common/images" ) );

    a2dCentralCanvasCommandProcessor* docmanager = new a2dCentralCanvasCommandProcessor();
    a2dDocviewGlobals->SetDocviewCommandProcessor( docmanager );

    a2dCanvasGlobals->GetHabitat()->GetHandle()->Set( 0, 0, 9, 9 );

    a2dPin* defCanConnectPin = new a2dPin( NULL, wxT( "global" ), a2dPinClass::Standard, 0, 0, 0, 10, 10 );
    defCanConnectPin->SetFill( *a2dTRANSPARENT_FILL );
    defCanConnectPin->SetStroke( *wxGREEN, 2 );
    a2dCanvasGlobals->GetHabitat()->SetPinCanConnect( defCanConnectPin );

    a2dViewTemplate* viewtemplatenew;

    m_mainconnector = new a2dCanMdiFrameDrawingConnector( NULL );
    m_mainconnector->SetInitialPosition( wxPoint( 50, 40 ) );
    m_mainconnector->SetInitialSize( wxSize( 200, 300 ) );

#if wxART2D_USE_CVGIO
	a2dIOHandlerDocCVGIn* cvghin = new a2dIOHandlerDocCVGIn();
    a2dIOHandlerDocCVGOut* cvghout = new a2dIOHandlerDocCVGOut();

    a2dDocumentTemplate* doctemplatenew;
    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate(
        wxT( "CVG Drawing" ),
        wxT( "*.cvg" ),
        wxT( "" ),
        wxT( "cvg" ),
        wxT( "MyCanvasDocument" ),
        CLASSINFO( MyCanvasDocument ),
        m_mainconnector,
        a2dTemplateFlag::DEFAULT,
        cvghin, cvghout
    );

    docmanager->AssociateDocTemplate( doctemplatenew );

    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "CVG Drawing" ),
        wxT( "MyCanvasDocument" ),
        wxT( "a2dCanvas + view" ),
        CLASSINFO( a2dCanvasView ),
        m_mainconnector,
        a2dTemplateFlag::DEFAULT,
        wxSize( 100, 400 )
    );
    m_mainconnector->AssociateViewTemplate( viewtemplatenew );
    docmanager->AssociateViewTemplate( viewtemplatenew );
#endif //wxART2D_USE_CVGIO

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

//------------------------------------------------------------------------------
// MyParentFrame
//------------------------------------------------------------------------------


BEGIN_EVENT_TABLE( MyParentFrame, a2dDocumentMDIParentFrame )
    EVT_MENU( wxID_EXIT, MyParentFrame::OnExit )
    EVT_MENU( EditorFrame_ABOUT, MyParentFrame::OnAbout )
    EVT_CLOSE( MyParentFrame::OnCloseWindow )
    EVT_MENU( SHOW_TOOL_DLG, MyParentFrame::OnShowToolDlg )
    EVT_UPDATE_UI( SHOW_TOOL_DLG, MyParentFrame::OnUpdateShowToolDlg )
END_EVENT_TABLE()


MyParentFrame::MyParentFrame( wxFrame* parent, const wxString& title,
                              const wxPoint& position, const wxSize& size, const long style ) :
    a2dDocumentMDIParentFrame( parent, -1, title, position, size, style )

{
    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( wxT( "Welcome to Multiple Frame Managed Views sample!" ) );

    SetIcon( wxICON( mondrian ) );

    m_tooldlg = NULL;
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

    wxMenu* dlg_menu = new wxMenu;
    dlg_menu->Append( SHOW_TOOL_DLG, _( "&Tools Dialog" ) , _( "tools dialog" ) , true );

    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( m_file_menu );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( EditorFrame_ABOUT, _( "&About Mdi Editor" ) );

    menu_bar->Append( m_file_menu, _( "&File" ) );
    menu_bar->Append( dlg_menu, _( "&Dialogs" ) );
    menu_bar->Append( help_menu, _( "&Help" ) );

    SetMenuBar( menu_bar );

}

MyParentFrame::~MyParentFrame()
{
}

void MyParentFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "MDI Editor Demo\nAuthor: Klaas Holwerda" ), wxT( "About MDI editor" ) );
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

void MyParentFrame::OnShowToolDlg( wxCommandEvent& event )
{
    if ( !m_tooldlg )
    {
        m_tooldlg  = new ToolDlg( this );
    }

    if ( m_tooldlg->IsShown() )
    {
        m_tooldlg->Show( false );
    }
    else
    {
        m_tooldlg->Show( true );
    }
}

void MyParentFrame::OnUpdateShowToolDlg( wxUpdateUIEvent& event )
{
    event.Enable( !m_tooldlg || !m_tooldlg->IsShown() );
}

//------------------------------------------------------------------------------
// MDIEditorFrame
//------------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( MDIEditorFrame, a2dDocumentMDIChildFrame )

BEGIN_EVENT_TABLE( MDIEditorFrame, a2dDocumentMDIChildFrame )
    EVT_CLOSE( MDIEditorFrame::OnCloseWindow )
    EVT_MENU( EditorFrame_ABOUT, MDIEditorFrame::OnAbout )
    EVT_MENU( EditorFrame_ZOOM_OUT, MDIEditorFrame::Zoomout )
    EVT_MENU( EditorFrame_STYLE_DLG, MDIEditorFrame::StyleDlg )
    EVT_MENU( REFRESH, MDIEditorFrame::Refresh )
    EVT_MENU( SET_FIXEDSTYLE, MDIEditorFrame::OnMenu )
    EVT_MENU( SET_POLYTOOL, MDIEditorFrame::SetPolygonTool )
    EVT_MENU( SET_POLYLINETOOL, MDIEditorFrame::SetPolylineTool )
    EVT_MENU( SET_POLYLINEWIRETOOL, MDIEditorFrame::SetPolylineWireTool )
    EVT_MENU( SET_RECTTOOL, MDIEditorFrame::SetRectangleTool )
    EVT_MENU( SET_DRAGTOOL, MDIEditorFrame::SetDragTool )
    EVT_MENU( SET_COPYTOOL, MDIEditorFrame::SetCopyTool )
    EVT_MENU( SET_DELETETOOL, MDIEditorFrame::SetDeleteTool )
    EVT_MENU( SET_EDITTOOL, MDIEditorFrame::SetEditTool )
    EVT_MENU( SET_DRAGNEWTOOL, MDIEditorFrame::SetDragNewTool )
    EVT_MENU( SET_MASTERTOOL, MDIEditorFrame::SetMasterTool )
    EVT_MENU( SET_TEXTTOOL, MDIEditorFrame::SetTextTool )
    EVT_MENU( SET_SELECTTOOL, MDIEditorFrame::SetSelectTool )
    EVT_MENU( END_TOOL, MDIEditorFrame::EndTool )
    EVT_ACTIVATE( MDIEditorFrame::OnActivate )
    EVT_POST_LOAD_DOCUMENT( MDIEditorFrame::OnPostLoadDocument )
    EVT_MENU( wxID_UNDO, MDIEditorFrame::OnUndo )
    EVT_MENU( wxID_REDO, MDIEditorFrame::OnRedo )
    EVT_MENUSTRINGS( MDIEditorFrame::OnSetmenuStrings )
END_EVENT_TABLE()

MDIEditorFrame::MDIEditorFrame( wxMDIParentFrame* parent,
                                wxWindowID id, const wxString& title,
                                const wxPoint& position, const wxSize& size, const long style ) :
    a2dDocumentMDIChildFrame( parent, NULL, -1, title, position, size, style )
{
    Init();
}

void MDIEditorFrame::Init()
{
    m_fixedToolStyle = false;

    CreateMyMenuBar();
    CreateStatusBar( 2 );
    SetStatusText( wxT( "Multi purpose MDIEditorFrame for DrawingView's" ) );

    SetIcon( wxICON( mondrian ) );

    m_canvas = new a2dViewCanvas( NULL, this, -1, wxPoint( 0, 0 ), GetSize() , wxDOUBLE_BORDER | wxNO_FULL_REPAINT_ON_RESIZE );
    m_canvas->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL ) );
    // Give it scrollbars
    m_canvas->ClearBackground();

    //if you do not want the origin shown
    //m_canvas->SetShowOrigin( false );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->SetMappingUpp( -50, -40, 1, 1 );

    m_canvas->SetYaxis( true );
    //defines the whole area in world coordinates that can be seen by scrolling
    m_canvas->SetScrollMaximum( -50, -40, 800, 600 );
    //defines the number of world units scrolled when line up or down events in X
    m_canvas->SetScrollStepX( 50 );
    //defines the number of world units scrolled when line up or down events in Y
    m_canvas->SetScrollStepY( 50 );
    //do not go outide the scroll maximum
    //m_canvas->FixScrollMaximum(true);  //what to do when resizing above ScrollMaximum is still ??

    a2dFill back = a2dFill( wxColour( 5, 235, 235 ), a2dFILL_SOLID );
    m_canvas->SetBackgroundFill( back );
        
    m_canvas->GetDrawing()->ConnectEvent( wxEVT_MENUSTRINGS, this );
    m_canvas->GetDrawing()->ConnectEvent( wxEVT_DO, this );
    m_canvas->GetDrawing()->ConnectEvent( wxEVT_UNDO, this );
    m_canvas->GetDrawing()->ConnectEvent( wxEVT_REDO, this );

    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDragMode( wxDRAW_COPY );
    //m_contr->SetDragMode( wxDRAW_ONTOP );

    //grid for routing wires
    a2dRouteData::SetRaster( 30.0 );

}

void MDIEditorFrame::OnPostLoadDocument( a2dDocumentEvent& event )
{
    if ( m_view->GetDocument()  != event.GetEventObject() )
        return;

    m_canvas->SetMappingShowAll();
}

void MDIEditorFrame::Refresh( wxCommandEvent& event )
{
    a2dWalker_RemovePropertyCandoc setp( a2dCanvasObject::PROPID_Original );
    setp.Start( m_canvas->GetDrawing()->GetRootObject() );
}

void MDIEditorFrame::Zoomout( wxCommandEvent& WXUNUSED( event ) )
{
    m_canvas->SetMappingShowAll();
}

void MDIEditorFrame::StyleDlg( wxCommandEvent& WXUNUSED( event ) )
{
    a2dStyleDialog* styledialog = new a2dStyleDialog( a2dCanvasGlobals->GetHabitat(), this, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxDIALOG_NO_PARENT | wxMINIMIZE_BOX | wxMAXIMIZE_BOX );
    styledialog->SentStyle();
    styledialog->Show();
}

void MDIEditorFrame::SetPolygonTool( wxCommandEvent& event )
{
    a2dVertexList* pointlist2 = new a2dVertexList();
    a2dLineSegment* point2 = new a2dLineSegment( -400, -100 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( -400, 200 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( 0, 200 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( 0, 100 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( -200, 175 );
    pointlist2->push_back( point2 );
    a2dPolygonL* polygon = new a2dPolygonL( pointlist2, true );

    //style of template polygon will be used in and NOT from the tool
    a2dDrawPolygonLTool* draw = new a2dDrawPolygonLTool( m_contr, polygon );
    draw->SetShowAnotation( true );

    //OR like this without a starting template
    //a2dDrawPolygonLTool* draw = new a2dDrawPolygonLTool( m_contr );

    a2dFill fill = a2dFill( wxColour( 229, 45, 44 ), a2dFILL_HORIZONTAL_HATCH );
    fill.SetAlpha( 230 );
    a2dStroke stroke = a2dStroke( wxColour( 239, 5, 64 ), 20.0, a2dSTROKE_LONG_DASH );
    draw->SetEditAtEnd( true );

    if ( m_fixedToolStyle )
    {
        draw->SetFill( fill );
        draw->SetStroke( stroke );
        draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    }
    else
    {
        a2dCanvasGlobals->GetHabitat()->SetFill( fill );
        a2dCanvasGlobals->GetHabitat()->SetStroke( stroke );
    }

    m_contr->PushTool( draw );
}

void MDIEditorFrame::SetPolylineTool( wxCommandEvent& event )
{
    a2dArrow* arrow = new  a2dArrow( 0, 0, 140, 90, 70 );
    arrow->SetFill( wxColour( 219, 215, 6 ) );
    arrow->SetStroke( wxColour( 1, 3, 205 ), 6.0 );

    a2dVertexList* pointlist2 = new a2dVertexList();
    a2dLineSegment* point2 = new a2dLineSegment( -400, -100 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( -400, 200 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( 0, 200 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( 0, 100 );
    pointlist2->push_back( point2 );
    point2 = new a2dLineSegment( -200, 175 );
    pointlist2->push_back( point2 );

    a2dPolylineL* poly16 = new a2dPolylineL( pointlist2, false );
    poly16->SetStroke( wxColour( 9, 115, 64 ), 14.0 );
    poly16->SetEnd( arrow );
    poly16->SetBegin( arrow );

    //style of template polygon will be used in and NOT from the tool
    a2dDrawPolylineLTool* draw = new a2dDrawPolylineLTool( m_contr , poly16 );
    draw->SetShowAnotation( true );
    //draw->SetLineEnd(arrow);
    //draw->SetLineBegin(arrow);

    //OR like this without a stating template
    //a2dDrawPolylineLTool* draw = new a2dDrawPolylineLTool( m_contr );

    a2dFill fill = a2dFill( wxColour( 129, 245, 64 ), a2dFILL_VERTICAL_HATCH );
    fill.SetAlpha( 230 );
    a2dStroke stroke = a2dStroke( wxColour( 239, 125, 64 ), 2.0, a2dSTROKE_LONG_DASH );

    draw->SetEditAtEnd( true );

    if ( m_fixedToolStyle )
    {
        draw->SetFill( fill );
        draw->SetStroke( stroke );
        draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    }
    else
    {
        a2dCanvasGlobals->GetHabitat()->SetFill( fill );
        a2dCanvasGlobals->GetHabitat()->SetStroke( stroke );
    }

    m_contr->PushTool( draw );
}

void MDIEditorFrame::SetPolylineWireTool( wxCommandEvent& event )
{
    a2dArrow* arrow = new  a2dArrow( 0, 0, 120, 90, 70 );
    arrow->SetFill( wxColour( 29, 215, 6 ) );
    arrow->SetStroke( wxColour( 90, 30, 205 ), 1.0 );

    //style of template polygon will be used in and NOT from the tool
    a2dDrawWirePolylineLTool* draw = new a2dDrawWirePolylineLTool( m_contr );
    draw->SetShowAnotation( false );
    draw->SetLineEnd( arrow );

    a2dFill fill = a2dFill( wxColour( 129, 245, 64 ), a2dFILL_VERTICAL_HATCH );
    fill.SetAlpha( 230 );
    a2dStroke stroke = a2dStroke( wxColour( 239, 125, 64 ), 4.0, a2dSTROKE_SHORT_DASH );

    if ( m_fixedToolStyle )
    {
        draw->SetFill( fill );
        draw->SetStroke( stroke );
        draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    }
    else
    {
        a2dCanvasGlobals->GetHabitat()->SetFill( fill );
        a2dCanvasGlobals->GetHabitat()->SetStroke( stroke );
    }

    //the next of you want to edit the wire after connecting
    //draw->SetEditAtEnd( true );
    m_contr->PushTool( draw );
}

void MDIEditorFrame::SetDragTool( wxCommandEvent& event )
{
    a2dDragTool* tool = new a2dDragTool( m_contr );
    tool->SetShowAnotation( false );
    m_contr->PushTool( tool );
}

void MDIEditorFrame::SetCopyTool( wxCommandEvent& event )
{
    a2dCopyTool* tool = new a2dCopyTool( m_contr );
    tool->SetShowAnotation( false );
    m_contr->PushTool( tool );
}

void MDIEditorFrame::SetDeleteTool( wxCommandEvent& event )
{
    a2dDeleteTool* tool = new a2dDeleteTool( m_contr );
    tool->SetShowAnotation( false );
    m_contr->PushTool( tool );
}

void MDIEditorFrame::SetEditTool( wxCommandEvent& event )
{
    a2dRecursiveEditTool* draw = new a2dRecursiveEditTool( m_contr );
    draw->SetShowAnotation( false );
    a2dFill fill = a2dFill( wxColour( 29, 225, 164 ), a2dFILL_SOLID );
    fill.SetAlpha( 230 );

    draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 39, 235, 64 ), 5.0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );

    //if you want just a one time action of editing one object
    // draw->SetOneShot();

    m_contr->PushTool( draw );
}

void MDIEditorFrame::SetRectangleTool( wxCommandEvent& event )
{
    a2dDrawRectangleTool* draw = new a2dDrawRectangleTool( m_contr );
    draw->SetShowAnotation( true );
    a2dFill fill = a2dFill( wxColour( 229, 25, 164 ), a2dFILL_HORIZONTAL_HATCH );
    fill.SetAlpha( 200 );
    a2dStroke stroke = a2dStroke( wxColour( 29, 235, 64 ), 10.0, a2dSTROKE_LONG_DASH );

    draw->SetEditAtEnd( true );

    if ( m_fixedToolStyle )
    {
        draw->SetFill( fill );
        draw->SetStroke( stroke );
        draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    }
    else
    {
        a2dCanvasGlobals->GetHabitat()->SetFill( fill );
        a2dCanvasGlobals->GetHabitat()->SetStroke( stroke );
    }

    m_contr->PushTool( draw );

}

void MDIEditorFrame::SetDragNewTool( wxCommandEvent& event )
{
    a2dArrow* arrow = new  a2dArrow( 0, 0, 220, 190, 170 );
    arrow->SetFill( wxColour( 29, 215, 6 ) );
    arrow->SetStroke( wxColour( 90, 30, 205 ), 1.0 );

    a2dDragNewTool* draw = new a2dDragNewTool( m_contr, arrow );
    draw->SetShowAnotation( true );
    a2dFill fill = a2dFill( wxColour( 229, 25, 164 ), a2dFILL_HORIZONTAL_HATCH );
    fill.SetAlpha( 200 );
    a2dStroke stroke = a2dStroke( wxColour( 29, 235, 64 ), 10.0, a2dSTROKE_LONG_DASH );

    draw->SetEditAtEnd( true );

    if ( m_fixedToolStyle )
    {
        draw->SetFill( fill );
        draw->SetStroke( stroke );
        draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    }
    else
    {
        a2dCanvasGlobals->GetHabitat()->SetFill( fill );
        a2dCanvasGlobals->GetHabitat()->SetStroke( stroke );
    }

    m_contr->PushTool( draw );
}

void MDIEditorFrame::SetMasterTool( wxCommandEvent& event )
{
    a2dGraphicsMasterTool* draw = new a2dGraphicsMasterTool( m_contr );
    m_contr->PushTool( draw );
}

void MDIEditorFrame::SetTextTool( wxCommandEvent& event )
{
#if wxART2D_USE_EDITOR
    a2dDrawTextTool* draw = new a2dDrawTextTool( m_contr );
    draw->SetShowAnotation( false );
#if wxART2D_USE_FREETYPE
    draw->GetTemplateObject()->SetFont( a2dFont( wxT( "LiberationSerif-Italic.ttf" ), 150.0 ) );
    //draw->GetTemplateObject()->SetFont( a2dFont( wxT("LiberationSerif-Regular.ttf"), 200 ) );
#if defined(__WXMSW__) && !defined(__GNUWIN32__)
    //draw->GetTemplateObject()->SetFont( a2dFont( wxFont( 180,wxNORMAL, wxNORMAL, wxNORMAL ), 200 ));
#endif // defined( __WXMSW__ )
#else
    draw->GetTemplateObject()->SetFont( a2dFont( 200, wxROMAN ) );
#endif

    a2dFill fill = a2dFill( wxColour( 229, 25, 164 ), a2dFILL_HORIZONTAL_HATCH );
    fill.SetAlpha( 200 );
    a2dStroke stroke = a2dStroke( wxColour( 29, 235, 64 ), 10.0, a2dSTROKE_LONG_DASH );

    draw->SetEditAtEnd( true );

    if ( m_fixedToolStyle )
    {
        draw->SetFill( fill );
        draw->SetStroke( stroke );
        draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    }
    else
    {
        a2dCanvasGlobals->GetHabitat()->SetFill( fill );
        a2dCanvasGlobals->GetHabitat()->SetStroke( stroke );
    }

    m_contr->PushTool( draw );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MDIEditorFrame::SetSelectTool( wxCommandEvent& event )
{
    a2dSelectTool* tool = new a2dSelectTool( m_contr );
    tool->SetShiftIsAdd();
    m_contr->PushTool( tool );
}

void MDIEditorFrame::EndTool( wxCommandEvent& event )
{
    a2dSmrtPtr< a2dBaseTool > tool;
    m_contr->PopTool( tool );
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
    m_edit_menu->Append( REFRESH, wxT( "&refresh" ) );
    m_edit_menu->Append( SET_FIXEDSTYLE, _( "&Fix Style" ), _( "&fixed style on/off" ), true );
    m_edit_menu->Append( SET_POLYTOOL, wxT( "&setpolygontool" ) );
    m_edit_menu->Append( SET_POLYLINETOOL, wxT( "&setpolylinetool" ) );
    m_edit_menu->Append( SET_POLYLINEWIRETOOL, wxT( "&setpolylinewiretool" ) );
    m_edit_menu->Append( SET_RECTTOOL, wxT( "&setrectangletool" ) );
    m_edit_menu->Append( SET_DRAGTOOL, wxT( "&setDragTool" ) );
    m_edit_menu->Append( SET_COPYTOOL, wxT( "&setCopyTool" ) );
    m_edit_menu->Append( SET_DELETETOOL, wxT( "&setDeleteTool" ) );
    m_edit_menu->Append( SET_EDITTOOL, wxT( "&setEditTool" ) );
    m_edit_menu->Append( SET_DRAGNEWTOOL, wxT( "&setDragNewTool" ) );
    m_edit_menu->Append( SET_MASTERTOOL, wxT( "&setMasterTool" ) );
    m_edit_menu->Append( SET_TEXTTOOL, _( "&settexttool" ) );
    m_edit_menu->Append( SET_SELECTTOOL, wxT( "&setSelectTool" ) );
    m_edit_menu->Append( END_TOOL, wxT( "&endtool" ) );

    wxMenu* tool_menu = new wxMenu;
    tool_menu->Append( EditorFrame_ZOOM_OUT, _T( "&ZoomOut" ) );

    wxMenu* dlg_menu = new wxMenu;
    dlg_menu->Append( SHOW_TOOL_DLG, _( "&Tools Dialog" ) , _( "tools dialog" ) , true );
    dlg_menu->Append( EditorFrame_STYLE_DLG, _T( "&Style Dialog" ) );

    m_help_menu = new wxMenu;
    m_help_menu->Append( EditorFrame_ABOUT, _( "&About" ) );

    m_menu_bar->Append( m_file_menu, _( "&File" ) );
    m_menu_bar->Append( m_edit_menu, _( "&Edit" ) );
    m_menu_bar->Append( tool_menu, _( "&Tools" ) );
    m_menu_bar->Append( dlg_menu, _( "&Dialogs" ) );
    m_menu_bar->Append( m_help_menu, _( "&Help" ) );

    SetMenuBar( m_menu_bar );

    GetMenuBar()->Check( SET_FIXEDSTYLE, false );

}

MDIEditorFrame::~MDIEditorFrame()
{
    if ( m_view && m_view->GetDocument() )
    {
        m_canvas->GetDrawing()->DisconnectEvent( wxEVT_MENUSTRINGS, this );
        m_canvas->GetDrawing()->DisconnectEvent( wxEVT_DO, this );
        m_canvas->GetDrawing()->DisconnectEvent( wxEVT_UNDO, this );
        m_canvas->GetDrawing()->DisconnectEvent( wxEVT_REDO, this );
    }
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

void MDIEditorFrame::OnMenu( wxCommandEvent& event )
{
    if ( event.GetId() == SET_FIXEDSTYLE )
    {
        m_fixedToolStyle = !m_fixedToolStyle;
        if ( m_fixedToolStyle )
        {
            m_contr->GetFirstTool()->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
        }
        else
        {
            //default it is this one.
            m_contr->GetFirstTool()->SetEvtHandler(  new a2dStToolEvtHandler( m_contr ) );
        }

        //m_contr->ReStart();
    }
}

void MDIEditorFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "Mdi Canvas Demo\nAuthor: Klaas Holwerda" ), wxT( "About Mdi Canvas Frame" ) );
}

void MDIEditorFrame::Activate( bool activate )
{
    if ( m_view )
        m_view->Activate( activate );

}

void MDIEditorFrame::OnActivate( wxActivateEvent& event )
{
    if ( !m_view )
        return;

    m_view->Activate( true );
    a2dDocument* doc = m_view->GetDocument();
}

void MDIEditorFrame::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( doc->GetCommandProcessor() )
        doc->GetCommandProcessor()->Undo();
}

void MDIEditorFrame::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( doc->GetCommandProcessor() )
        doc->GetCommandProcessor()->Redo();
}

void MDIEditorFrame::OnSetmenuStrings( a2dCommandProcessorEvent& event )
{
    if ( m_view && m_view->GetDocument() && event.GetEventObject() == m_view->GetDocument()->GetCommandProcessor() )
    {
        m_edit_menu->SetLabel( wxID_UNDO, event.GetUndoMenuLabel() );
        m_edit_menu->Enable( wxID_UNDO, event.CanUndo() );

        m_edit_menu->SetLabel( wxID_REDO, event.GetRedoMenuLabel() );
        m_edit_menu->Enable( wxID_REDO, event.CanRedo() );
    }
}
