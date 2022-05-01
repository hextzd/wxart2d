/*! \file canvas/samples/simple/simple.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: simple.cpp,v 1.4 2008/07/30 21:54:04 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "simple.h"

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

// Include image
#include "smile.xpm"

#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#else
#include "wx/artbase/dcdrawer.h"
#endif

#include "wx/canvas/eval.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// MyCircle MyCanvasStToolContr
//----------------------------------------------------------------------------

//! zoom menu
/*!
  \ingroup  tools
*/
enum Myzoompop
{
    TC_PROPERTY_XY2
};

BEGIN_EVENT_TABLE( MyCanvasStToolContr, a2dStToolContr )
    EVT_MENU( TC_PROPERTY_XY2    , MyCanvasStToolContr::MyEditProperties )
END_EVENT_TABLE()


MyCanvasStToolContr::MyCanvasStToolContr( a2dDrawingPart* drawer, wxFrame* where ): a2dStToolContr( drawer, where )
{
    /*  Do this if want something complete new.
        delete m_mousemenu;
        m_mousemenu = new wxMenu( _("ZOOM"),(long)0);
    */
    //Take your pick in what is down here.

    //m_mousemenu->Remove(TC_ENDTOOL);//, "end tool");
    //m_mousemenu->Remove(-1);
    //m_mousemenu->Remove(TC_ZOOMIN); //, "Zoom Window");
    //m_mousemenu->Remove(TC_UNDO);   //, "undo");

    m_mousemenu->SetLabel( TC_ZOOMBACK,  _T( "Undo last Zoom" ) );
    m_mousemenu->SetHelpString( TC_ZOOMBACK,  _T( "Go to previous zooming area" ) );

    m_mousemenu->SetLabel( TC_ZOOMOUT,   _T( "Zoom to fit" ) );
    m_mousemenu->SetHelpString( TC_ZOOMOUT,  _T( "Zooms the canvas, so that all objects are visible" ) );

    m_mousemenu->SetLabel( TC_ZOOMOUT2,  _T( "Zoom out" ) );
    m_mousemenu->SetHelpString( TC_ZOOMOUT2,  _T( "Zooms the canvas, to double in width and height" ) );

    m_mousemenu->SetLabel( TC_ZOOMIN2,  _T( "Zoom in" ) );

    //append extra menu's
    wxMenuItem* rem = m_mousemenu->Remove( TC_PROPERTY_XY ); //, "Edit Properties");
    delete rem;
    m_mousemenu->Append( TC_PROPERTY_XY2, ( _T( "My Edit Properties" ) ) );

}

void MyCanvasStToolContr::MyEditProperties( wxCommandEvent& )
{
    a2dPropertyTool* propedit = new a2dPropertyTool( this );
    PushTool( propedit );
    propedit->SetOneShot();
    propedit->StartEditing( m_mouse_x, m_mouse_y );
}

//----------------------------------------------------------------------------
// MyCircle
//----------------------------------------------------------------------------

a2dPropertyIdString* MyCircle::PROPID_tipproperty = NULL;
a2dPropertyIdString* MyCircle::PROPID_node_stats = NULL;

INITIALIZE_PROPERTIES( MyCircle, a2dCircle )
{
    PROPID_tipproperty = new a2dPropertyIdString( wxT( "tipproperty" ), a2dPropertyId::flag_notify, wxT( "" ) );
    AddPropertyId( PROPID_tipproperty );
    PROPID_node_stats = new a2dPropertyIdString( wxT( "NodeStats" ), a2dPropertyId::flag_notify, wxEmptyString );
    AddPropertyId( PROPID_node_stats );

    return true;
}

BEGIN_EVENT_TABLE( MyCircle, a2dCircle )
    EVT_CANVASOBJECT_ENTER_EVENT( MyCircle::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT( MyCircle::OnLeaveObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( MyCircle::OnCanvasObjectMouseEvent )
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS( MyCircle, a2dCircle )

MyCircle::MyCircle()
{
    a2dStringProperty* strProp = new a2dStringProperty();
    a2dPropertyIdString* propid = ( a2dPropertyIdString* ) strProp->CreateId( this, "MyProperty" );
    propid->SetPropertyToObject( this, "whatever value" );
}

MyCircle::MyCircle( double x, double y, double radius ):
    a2dCircle( x, y, radius )
{

    a2dPropertyIdString* propid4 = ( a2dPropertyIdString* ) HasPropertyId( "MyProperty4" );
    if ( !propid4 )
    {
        propid4 = new a2dPropertyIdString( "MyProperty4", wxT( "" ), a2dPropertyId::flag_userDefined );
        AddPropertyId( propid4 );
    }
    propid4->SetPropertyToObject( this, "monkey" );

    a2dStringProperty* strProp = new a2dStringProperty();
    a2dPropertyIdString* propid = ( a2dPropertyIdString* ) strProp->CreateId( this, "MyProperty2" );
    propid->SetPropertyToObject( this, "whatever value" );

    PROPID_tipproperty->SetPropertyToObject( this, _T( "tip of mycirle" ) );

    a2dVisibleProperty* pppvis1 = new a2dVisibleProperty( this, PROPID_tipproperty, 0, radius + 12, true,
            a2dFont( 30.0, wxFONTFAMILY_SWISS ), 0.0 );
    pppvis1->SetVisible( false );

    pppvis1->SetFill( wxColour( 219, 215, 6 ), a2dFILL_SOLID );
    Append( pppvis1 );

    PROPID_node_stats->SetPropertyToObject( this, _T( "VISIBLE PROP" ) );
    a2dVisibleProperty* pppvis =
        new a2dVisibleProperty( this, PROPID_node_stats, 0, -radius - 12, true,
                                a2dFont( 30.0, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD ) );
    pppvis->ShowName( false );
    pppvis->SetNextLineDirection( true );
    pppvis->SetFill( *a2dTRANSPARENT_FILL );
    pppvis->SetStroke( a2dStroke( wxColour( _( "BLACK" ) ), 0.2 ) );
    pppvis->SetMultiLine( true );
    pppvis->SetAlignment( wxMINX | wxMINY );
    Append( pppvis );
}

MyCircle::MyCircle( const MyCircle& other, CloneOptions options, a2dRefMap* refs )
    : a2dCircle( other, options, refs )
{
    m_radius = other.m_radius;
    m_contourwidth = other.m_contourwidth;
    PROPID_tipproperty->SetPropertyToObject( this, _T( "tip of mycirle" ) );
    PROPID_node_stats->SetPropertyToObject( this, _T( "VISIBLE PROP" ) );
}

a2dObject* MyCircle::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new MyCircle( *this, options, refs );
};

void MyCircle::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    a2dVisibleProperty* visprop = wxDynamicCast( Find( wxT( "" ),  wxT( "a2dVisibleProperty" ) ), a2dVisibleProperty );
    wxASSERT ( visprop );
    visprop->SetVisible( true );
    SetPending( true );
}

void MyCircle::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{

    a2dVisibleProperty* visprop = wxDynamicCast( Find( wxT( "" ),  wxT( "a2dVisibleProperty" ) ), a2dVisibleProperty );
    wxASSERT ( visprop );
    visprop->SetVisible( false );
    SetPending( true );
}

void MyCircle::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    double xw, yw;
    xw = event.GetX();
    yw = event.GetY();

    int x, y;
    x = event.GetMouseEvent().GetX();
    y = event.GetMouseEvent().GetY();

    if ( event.GetMouseEvent().MiddleDown() )
    {
        wxString mess;
        mess.Printf( wxT( " arrived in MyCircle at x/y world %f %f, and x/y pixel %d %d" ), xw, yw, x, y );
        wxMessageBox(  mess, _T( "event test" ), wxOK );
        event.Skip();
    }
    else
        event.Skip();
}

//------------------------------------------------------------------------------
// MyDrawingPart
//------------------------------------------------------------------------------


void MyDrawingPart::PaintBackground( int x, int y, int width, int height )
{
    m_drawer2D->SetDrawerStroke( *a2dTRANSPARENT_STROKE );
    m_drawer2D->SetDrawerFill( m_backgroundfill );
    m_drawer2D->SetLayer( wxMAXLAYER );

    m_drawer2D->PushIdentityTransform();
    m_drawer2D->DrawRoundedRectangle( x, y, width, height, 0 );

    m_drawer2D->SetDrawerFill(  a2dFill( wxColour( 25, 25, 250 ) ) );

    int w, h;
    w = 100; h = 200;
    m_drawer2D->DrawEllipse( w/2, h/2, w, h );


    m_drawer2D->PopTransform();

    m_drawer2D->SetDrawerFill( *a2dTRANSPARENT_FILL );
    m_drawer2D->SetDrawerStroke( *a2dTRANSPARENT_STROKE );
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

const long ID_CLEAR = wxNewId();
const long ID_OPEN = wxNewId();
const long ID_SAVE = wxNewId();
const long ID_DELETETOOL = wxNewId();
const long SET_SELECTTOOL = wxNewId();
const long SET_POLYTOOL = wxNewId();
const long SET_RECTTOOL = wxNewId();
const long SET_DRAGTOOL = wxNewId();
const long SET_DRAGNEWTOOL = wxNewId();
const long SET_EDITTOOL = wxNewId();
const long SET_MASTERTOOL = wxNewId();
const long END_TOOL = wxNewId();
const long OPTION_SWITCHYAXIS = wxNewId();
const long DOCCANVAS_ABOUT = wxNewId();
const long ShapeClipboard_Copy = wxNewId();
const long ShapeClipboard_Paste = wxNewId();
const long ShapeClipboard_Drag = wxNewId();

enum
{
    MDSCREEN_T  = wxID_HIGHEST + 4000,
    MDSCREEN_FIRST,
    MDSCREEN_E = MDSCREEN_FIRST,
    MDSCREEN_0,
    MDSCREEN_1,
    MDSCREEN_2,
    MDSCREEN_3,
    MDSCREEN_4,
    MDSCREEN_5,
    MDSCREEN_6,
    MDSCREEN_7,
    MDSCREEN_LAST = MDSCREEN_7
};

enum
{
    DrawerOption_First = wxID_HIGHEST + 2000,
    Switch_Drawer_aggRgba = DrawerOption_First,
    Switch_Drawer_agg,
    Switch_Drawer_agggc,
    Switch_Drawer_dc,
    Switch_Drawer_gdiplus,
    Switch_Drawer_dcgc,
    Switch_Drawer_gdigc,
    DrawerOption_Last = Switch_Drawer_gdigc,
    DRAWER_CHANGE
};

// WDR: event table for MyFrame

BEGIN_EVENT_TABLE( MyFrame, wxFrame )
    EVT_MENU( ID_OPEN, MyFrame::OnLoad )
    EVT_MENU( ID_SAVE, MyFrame::OnSave )
    EVT_MENU( ID_CLEAR, MyFrame::OnClear )
    EVT_MENU( wxID_EXIT, MyFrame::OnQuit )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_MENU( OPTION_SWITCHYAXIS, MyFrame::SetSwitchY )
    EVT_MENU( SET_DRAGTOOL, MyFrame::SetDragTool )
    EVT_MENU( SET_DRAGNEWTOOL, MyFrame::SetDragNewTool )
    EVT_MENU( SET_RECTTOOL, MyFrame::SetRectangleTool )
    EVT_MENU( SET_POLYTOOL, MyFrame::SetPolygonTool )
    EVT_MENU( SET_SELECTTOOL, MyFrame::SetSelectTool )
    EVT_MENU( SET_EDITTOOL, MyFrame::SetEditTool )
    EVT_MENU( SET_MASTERTOOL, MyFrame::SetMasterTool )
    EVT_MENU( END_TOOL, MyFrame::EndTool )
    EVT_MENU( wxID_UNDO, MyFrame::OnUndo )
    EVT_MENU( wxID_REDO, MyFrame::OnRedo )
    //EVT_TIMER( -1, MyFrame::OnTimer )
    EVT_TIMER( -1, MyFrame::OnTimer2 )   
    EVT_MENU_RANGE( DrawerOption_First, DrawerOption_Last, MyFrame::OnDrawer )
    EVT_MENU_RANGE( MDSCREEN_FIRST, MDSCREEN_LAST, MyFrame::FillDoc )
    EVT_MENU( ShapeClipboard_Copy,  MyFrame::OnCopyShape)
    EVT_MENU( ShapeClipboard_Paste, MyFrame::OnPasteShape)
    EVT_MENU( ShapeClipboard_Drag, MyFrame::OnDragSimulate)
    //EVT_UPDATE_UI( a2dShapeClipboard_Paste, MyFrame::OnUpdatePasteShape)
    //EVT_UPDATE_UI( a2dShapeClipboard_Copy, MyFrame::OnUpdateCopyShape)
END_EVENT_TABLE()

MyFrame::MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    wxFrame( parent, id, title, position, size, style )
{
    CreateMyMenuBar();
    CreateStatusBar( 1 );
    SetStatusText( _T( "Welcome to a2dCanvas sample!" ) );

    SetIcon( wxICON( mondrian ) );
    a2dCanvasGlobals->GetHabitat()->SetReverseOrder( true );

#if with_GLCANVASdraw
    #if wxCHECK_VERSION(3,1,0)
        wxGLAttributes dispAttrs;
        dispAttrs.PlatformDefaults().MinRGBA(8, 8, 8, 8).DoubleBuffer().Depth(32).EndList();
        dispAttrs.SetNeedsARB(false);
        m_canvas = new a2dOglCanvas( this, dispAttrs, -1, wxDefaultPosition, wxDefaultSize, wxScrolledWindowStyle );
    #else
        int attribList[2];
        attribList[1] = WX_GL_DOUBLEBUFFER;
        m_canvas = new a2dOglCanvas( this, NULL, -1, wxDefaultPosition, wxDefaultSize );//, wxScrolledWindowStyle );
    #endif
#else
    //default 1000,1000 mapping
    m_canvas = new a2dCanvas( this, -1, wxDefaultPosition, wxDefaultSize, wxScrolledWindowStyle, new a2dMemDcDrawer() );

    MyDrawingPart* part = new MyDrawingPart( size.GetWidth(),size.GetHeight() );
    part->SetShowObject( m_canvas->GetDrawing()->GetRootObject() );
    part->SetDisplayWindow( m_canvas );
    m_canvas->SetDrawingPart( part );

#endif

    //! for some tools a tool controller
    m_contr = new MyCanvasStToolContr( m_canvas->GetDrawingPart(), this );
    //the next is handy, but as you prefer
    m_contr->SetZoomFirst( true );
    m_contr->SetOpacityFactorEditcopy( 125 );
    m_contr->SetUseOpaqueEditcopy( true );


    //NEXT 7 lines or optional, and depend on what you need.

    //if you do not want the origin shown
    m_canvas->SetShowOrigin( true );
    //m_canvas->GetDrawer2D()->SetYaxisDevice( true );
    m_canvas->GetDrawer2D()->SetYaxisDevice( false );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->SetMappingUpp( -50, -40, 1, 1 );

    m_canvas->SetYaxis( true );
    m_canvas->SetYaxis( false );
    //defines the whole area in world coordinates that can be seen by scrolling
    m_canvas->SetScrollMaximum( -50, -40, 800, 600 );
    // no change in scrollmaximum when zooming out further
    //m_canvas->FixScrollMaximum(true);
    //m_canvas->ClipToScrollMaximum(false);
    //defines the number of world units scrolled when line up or down events in X
    m_canvas->SetScrollStepX( 50 );
    //defines the number of world units scrolled when line up or down events in Y
    m_canvas->SetScrollStepY( 50 );
    //do not go outide the scroll maximum
    //m_canvas->FixScrollMaximum(true);  //what to do when resizing above ScrollMaximum is still ??
    // don't wnat scroll bars do this
    //m_canvas->SetScrollBarsVisible(false);

    m_canvas->SetBackgroundFill( a2dFill( wxColour( 205, 255, 250 ) ) );

    //Start filling document now.

    // One object group is the root in every canvas.
	a2dCanvasObject* root = m_canvas->GetDrawing()->GetRootObject();

    m_timer = NULL;
    m_button = NULL;
    m_smile1 = m_smile2 = NULL;

    FillDoc1( root );
    //FillDoc6( root );
    //FillDoc5( root );
    
    m_timer2 = new wxTimer( this );
    m_timer2->Start( 500, true );    
}

MyFrame::~MyFrame()
{
    delete m_timer;
    delete m_timer2;
}

void MyFrame::CreateMyMenuBar()
{
    wxMenu* file_menu = new wxMenu;
    file_menu->Append( ID_OPEN, _( "&Open" ), _( "Open file" ) );
    file_menu->Append( ID_SAVE, _( "&Save" ), _( "Save to file" ) );

    file_menu->AppendSeparator();
    file_menu->Append( ID_CLEAR, _( "&Clear" ), _( "Clear drawing" ) );
    file_menu->AppendSeparator();
    file_menu->Append( wxID_EXIT, _( "E&xit" ) );   

	AddCmdMenu( file_menu, CmdMenu_PreviewDrawing() );
	AddCmdMenu( file_menu, CmdMenu_PrintDrawing() );
	AddCmdMenu( file_menu, CmdMenu_PreviewDrawingPart() );
	AddCmdMenu( file_menu, CmdMenu_PrintDrawingPart() );

    m_edit_menu = new wxMenu;
    m_edit_menu->Append( wxID_UNDO, _( "&Undo" ) );
    m_edit_menu->Append( wxID_REDO, _( "&Redo" ) );
    
    //standard menu for tools also available
    AddCmdMenu( m_edit_menu, CmdMenu_PushTool_Delete() );

    m_edit_menu->Append( SET_MASTERTOOL, wxT( "set&mastertool" ) );
    m_edit_menu->Append( SET_POLYTOOL, wxT( "&setpolygontool" ) );
    m_edit_menu->Append( SET_RECTTOOL, wxT( "set&rectangletool" ) );
    m_edit_menu->SetHelpString( SET_RECTTOOL, wxT( "push the rectangle tool on the tool stack" ) );
    m_edit_menu->Append( SET_EDITTOOL, wxT( "set&edittool" ) );
    m_edit_menu->Append( SET_DRAGTOOL, wxT( "drag" ) );
    m_edit_menu->Append( SET_DRAGNEWTOOL, wxT( "drag&new" ) );

    //like much easier, but less flexibility
    AddCmdMenu( m_edit_menu, CmdMenu_PushTool_Select() );
    AddCmdMenu( m_edit_menu, CmdMenu_PushTool_RecursiveEdit() );
    AddCmdMenu( m_edit_menu, CmdMenu_PushTool_MultiEdit() );
    AddCmdMenu( m_edit_menu, CmdMenu_PushTool_Drag() );
    AddCmdMenu( m_edit_menu, CmdMenu_PushTool_Copy() );
    AddCmdMenu( m_edit_menu, CmdMenu_PushTool_Rotate() );
    AddCmdMenu( m_edit_menu, CmdMenu_PushTool_Delete() );
    m_edit_menu->Append( END_TOOL, wxT( "&endtool" ) );

    wxMenu* selected = new wxMenu;
    selected->Append( SET_SELECTTOOL, wxT( "&setselecttool" ) );
    AddCmdMenu( selected, CmdMenu_Selected_SelectAll() );
    AddCmdMenu( selected, CmdMenu_Selected_DeSelectAll() );
    AddCmdMenu( selected, CmdMenu_Selected_Delete() );

    m_optionMenu = new wxMenu;
    m_optionMenu->Append( OPTION_SWITCHYAXIS, _( "Yaxis Positive" ), _( "switch Y axis (positive or negative" ), true );

    wxMenu* drawEng = new wxMenu;
    drawEng->Append( Switch_Drawer_aggRgba, _T( "Switch Drawer &Agg AntiAlias Rgba" ) );
    drawEng->Append( Switch_Drawer_agg, _T( "Switch Drawer &Agg AntiAlias" ) );
    drawEng->Append( Switch_Drawer_agggc, _T( "Switch Drawer Graphics &Context &Agg Gc" ) );

    drawEng->Append( Switch_Drawer_dc, _T( "Switch Drawer &wxDC" ) );
    drawEng->Append( Switch_Drawer_gdiplus, _T( "Switch Drawer &GdiPlus" ) );
    drawEng->Append( Switch_Drawer_gdigc, _T( "Switch Drawer Graphics &Context GDI" ) );
    drawEng->Append( Switch_Drawer_dcgc, _T( "Switch Drawer Graphics Context &wxDC" ) );

    wxMenu* drawMenu = new wxMenu;
    drawMenu->Append( MDSCREEN_0, wxT( "doc0" ), _( "whatever" ) );
    drawMenu->Append( MDSCREEN_1, wxT( "doc1" ), _( "whatever" ) );
    drawMenu->Append( MDSCREEN_2, wxT( "doc2" ), _( "whatever" ) );
    drawMenu->Append( MDSCREEN_3, wxT( "doc3" ), _( "whatever" ) );
    drawMenu->Append( MDSCREEN_4, wxT( "doc4" ), _( "whatever" ) );
    drawMenu->Append( MDSCREEN_5, wxT( "doc5" ), _( "whatever" ) );
    drawMenu->Append( MDSCREEN_6, wxT( "doc6" ), _( "whatever" ) );
    drawMenu->Append( MDSCREEN_7, wxT( "doc7" ), _( "whatever" ) );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( DOCCANVAS_ABOUT, _( "&About" ) );


    wxMenu *menuClipboard = new wxMenu;
    menuClipboard->Append( ShapeClipboard_Copy, wxT("&Copy\tCtrl-C"));
    menuClipboard->Append( ShapeClipboard_Paste, wxT("&Paste\tCtrl-V"));
    menuClipboard->Append( ShapeClipboard_Drag, wxT("&Drag"));

    m_menuBar = new wxMenuBar;
    m_menuBar->Append( file_menu, _( "&File" ) );
    m_menuBar->Append( m_edit_menu, _( "&Edit" ) );
    m_menuBar->Append( selected, _( "&Selected" ) );
    m_menuBar->Append( m_optionMenu, _( "&Options" ) );
    m_menuBar->Append( drawEng, _T( "&Drawer Type" ) );
    m_menuBar->Append( drawMenu, _( "&DemoScreens" ) );
    m_menuBar->Append( menuClipboard, wxT("&Clipboard"));
    m_menuBar->Append( help_menu, _( "&Help" ) );
    //// Associate the menu bar with the frame
    SetMenuBar( m_menuBar );
}

void MyFrame::OnQuit( wxCommandEvent& event )
{
    m_contr = 0; //release pointer from MyFrame, m_canvas is doing the last release of it.

    Close( true );
}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    m_contr = 0; //release pointer from MyFrame, m_canvas is doing the last release of it.

    wxTheClipboard->Clear();
    Destroy();
}

void MyFrame::OnClear( wxCommandEvent& event )
{
    m_smile1 = m_smile2 = NULL;
    if ( m_button )
    {
        m_button->Destroy();
        m_button = NULL;
    }
    a2dDrawing* root = m_canvas->GetDrawing();
    root->GetRootObject()->ReleaseChildObjects();
}

void MyFrame::OnLoad( wxCommandEvent& event )
{
    m_smile1 = m_smile2 = NULL;

    a2dIOHandlerCVGIn* cvghin = new a2dIOHandlerCVGIn();
    a2dDrawing* root = m_canvas->GetDrawing();

    if ( wxFileExists( _( "myfile.cvg" ) ) )
    {
        root->GetRootObject()->ReleaseChildObjects();
        root->LoadFromFile( _( "myfile.cvg" ), cvghin );
        m_smile1 = NULL;
        m_smile2 = NULL;

    }
    else
        wxMessageBox( _( "first use save menu to create myfile.cvg" ) );

    m_canvas->SetMappingShowAll( true );
}

void MyFrame::OnSave( wxCommandEvent& event )
{
    a2dIOHandlerCVGOut* cvghout = new a2dIOHandlerCVGOut();
    a2dDrawing* root = m_canvas->GetDrawing();
    root->SaveToFile( _( "myfile.cvg" ), cvghout );
}

void MyFrame::OnTimer2( wxTimerEvent& event )
{
    if ( !IsShown() )
        return;
    if ( !m_canvas->IsShown() )
        return;
    
#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
    if ( wxDynamicCast( m_canvas->GetDrawingPart()->GetDisplayWindow(), a2dOglCanvas ) )
        m_canvas->Thaw();
#endif
    m_canvas->SetSizeOgl();
    m_canvas->Update();
    m_canvas->Refresh();
}

void MyFrame::OnTimer( wxTimerEvent& event )
{
    if( !m_canvas || !m_canvas->GetDrawing() )
    {
        return;
    }

    if ( m_smile1  && m_smile2 )
    {
        m_smile1->Translate( 1, 0 );
        m_smile2->Translate( 1, 0 );

        wxWakeUpIdle();
    }

    forEachIn( a2dCanvasObjectList, m_canvas->GetDrawing()->GetRootObject()->GetChildObjectList() )
    {
        a2dCanvasObject* obj = *iter;
        MyCircle* elem = wxDynamicCast( obj, MyCircle );
        if( !elem )
        {
            continue;
        }

        wxString val = wxString::Format( wxT( "random value: %d" ), rand() );
        MyCircle::PROPID_node_stats->SetPropertyToObject( elem, val );
    }
}

void MyFrame::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
    if ( m_canvas->GetDrawing()->GetCommandProcessor() )
        m_canvas->GetDrawing()->GetCommandProcessor()->Undo();
}

void MyFrame::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
    if ( m_canvas->GetDrawing()->GetCommandProcessor() )
        m_canvas->GetDrawing()->GetCommandProcessor()->Redo();
}

void MyFrame::SetSwitchY( wxCommandEvent& WXUNUSED( event ) )
{
    m_canvas->SetYaxis( !m_canvas->GetYaxis() );
    GetMenuBar()->Check( OPTION_SWITCHYAXIS, m_canvas->GetYaxis() );
    m_contr->Zoomout();
}

void MyFrame::SetSelectTool( wxCommandEvent& event )
{
    a2dSelectTool* draw = new a2dSelectTool( m_contr );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    draw->SetShowAnotation( true );
    m_contr->PushTool( draw );
}

void MyFrame::SetMasterTool( wxCommandEvent& event )
{
    a2dMasterTagGroups3* tool = new a2dMasterTagGroups3( m_contr );
    tool->SetWireMode( true );
    tool->SetDlgOrEdit( true );
	tool->SetStyleDlgSimple( true );
    tool->SetDlgOrEditModal( true );
    m_contr->StopAllTools();
    m_contr->SetTopTool( tool );
}

void MyFrame::SetPolygonTool( wxCommandEvent& event )
{
    a2dDrawPolygonLTool* draw = new a2dDrawPolygonLTool( m_contr );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    draw->SetShowAnotation( true );

    a2dFill fill = a2dFill( wxColour( 29, 245, 164 ), a2dFILL_HORIZONTAL_HATCH );
    fill.SetAlpha( 130 );

    draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 239, 5, 64 ), 5.0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );
    draw->SetEditAtEnd( true );

    m_contr->PushTool( draw );
}

void MyFrame::SetRectangleTool( wxCommandEvent& event )
{
	a2dRect* r = new a2dRect( 0, 0, 0, 0 );
    r->SetStroke( wxColour( 239, 15, 64 ), 2.0, a2dSTROKE_LONG_DASH );
    r->SetFill( a2dFill( wxColour( 29, 25, 164, 100 ), a2dFILL_HORIZONTAL_HATCH ) );
    //r->SetContourWidth( 20 );

    // Uncommment these to disable certain editing handles.
    //a2dCanvasObject::PROPID_Allowrotation->SetPropertyToObject( r, false );
    //a2dCanvasObject::PROPID_Allowskew->SetPropertyToObject( r, false );
    //2dRect::PROPID_AllowRounding->SetPropertyToObject( r, false );

    a2dDrawRectangleTool* draw = new a2dDrawRectangleTool( m_contr, r );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    draw->SetShowAnotation( false );
    draw->SetEditAtEnd( true );

    // can still overrule style of tool
    //a2dFill fill = a2dFill( wxColour( 29, 245, 164 ), a2dFILL_HORIZONTAL_HATCH );
    //draw->SetFill( fill );

    //a2dStroke stroke = a2dStroke( wxColour( 239, 5, 64 ), 20.0, a2dSTROKE_LONG_DASH );
    //draw->SetStroke( stroke );

    m_contr->PushTool( draw );
}

void MyFrame::SetEditTool( wxCommandEvent& event )
{
    a2dRecursiveEditTool* draw = new a2dRecursiveEditTool( m_contr );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    draw->SetShowAnotation( false );

    a2dFill fill = a2dFill( wxColour( 29, 245, 164 ), a2dFILL_HORIZONTAL_HATCH );

    draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 239, 5, 64 ), 20.0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );

    m_contr->PushTool( draw );
}

void MyFrame::SetDragTool( wxCommandEvent& event )
{
    a2dDragTool* drag = new a2dDragTool( m_contr );
    m_contr->PushTool( drag );
}

void MyFrame::SetDragNewTool( wxCommandEvent& event )
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
    draw->SetOneShot();

    draw->SetFill( fill );
    draw->SetStroke( stroke );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );

    m_contr->PushTool( draw );
}

void MyFrame::EndTool( wxCommandEvent& event )
{
    a2dSmrtPtr< a2dBaseTool > tool;
    m_contr->PopTool( tool );
}

void MyFrame::OnDrawer( wxCommandEvent& event )
{
    int width = m_canvas->GetDrawingPart()->GetDrawer2D()->GetWidth();
    int height = m_canvas->GetDrawingPart()->GetDrawer2D()->GetHeight();

    switch ( event.GetId() )
    {
#if wxART2D_USE_AGGDRAWER
        case Switch_Drawer_aggRgba:
        {
            m_canvas->GetDrawingPart()->SetDrawer2D( new a2dAggDrawerRgba( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
        case Switch_Drawer_agg:
        {
            m_canvas->GetDrawingPart()->SetDrawer2D( new a2dAggDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#if wxART2D_USE_GRAPHICS_CONTEXT
        case Switch_Drawer_agggc:
        {
            m_canvas->GetDrawingPart()->SetDrawer2D( new a2dGcAggDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#endif // wxART2D_USE_GRAPHICS_CONTEXT

#endif //wxART2D_USE_AGGDRAWER
        case Switch_Drawer_dc:
            m_canvas->GetDrawingPart()->SetDrawer2D( new a2dMemDcDrawer( width, height ) );
            m_contr->SetUseOpaqueEditcopy( false );
            break;

#if wxART2D_USE_GDIPLUSDRAWER
        case Switch_Drawer_gdiplus:
        {
            m_canvas->GetDrawingPart()->SetDrawer2D( new a2dGDIPlusDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#endif //wxART2D_USE_GDIPLUSDRAWER

#if wxART2D_USE_GRAPHICS_CONTEXT
        case Switch_Drawer_dcgc:
        {
            m_canvas->GetDrawingPart()->SetDrawer2D( new a2dGcDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
        case Switch_Drawer_gdigc:
        {
            m_canvas->GetDrawingPart()->SetDrawer2D( new a2dNativeGcDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#endif // wxART2D_USE_GRAPHICS_CONTEXT
        default:
            // skip Refresh()
            return;
    }

    m_canvas->GetDrawingPart()->GetDrawer2D()->SetYaxis( true );
    m_canvas->SetMappingShowAll();
}

void MyFrame::FillDoc( wxCommandEvent& event )
{
    m_smile1 = m_smile2 = NULL;
    if ( m_button )
        m_button->Destroy();
    a2dCanvasObject* root = m_canvas->GetDrawing()->GetRootObject();
    root->ReleaseChildObjects();

    switch ( event.GetId() )
    {
        case MDSCREEN_0:
        {
            FillDoc0( root );
            //doc->SetTitle( wxT( "screen 0" ) );
            break;
        }
        case MDSCREEN_1:
        {
            FillDoc1( root );
            //doc->SetTitle( wxT( "screen 1" ) );
            break;
        }
        case MDSCREEN_2:
        {
            FillDoc2( root );
           //doc->SetTitle( wxT( "screen 2" ) );
            break;
        }
        case MDSCREEN_3:
        {
            FillDoc3( root );
            //doc->SetTitle( wxT( "screen 3" ) );
            break;
        }
        case MDSCREEN_4:
        {
            FillDoc4( root );
            //doc->SetTitle( wxT( "screen 4" ) );
            break;
        }
        case MDSCREEN_5:
        {
            FillDoc5( root );
            //doc->SetTitle( wxT( "screen 5" ) );
            break;
        }
        case MDSCREEN_6:
        {
            FillDoc6( root );
            //doc->SetTitle( wxT( "screen 6" ) );
            break;
        }
        case MDSCREEN_7:
        {
            FillDoc7( root );
            //doc->SetTitle( wxT( "screen 6" ) );
            break;
        }
        default:
            return;
    }

    m_contr->Zoomout();
}

void MyFrame::FillDoc0( a2dCanvasObject* root )
{
	a2dRect* r = new a2dRect( 0, 0, 80, 40 );
    r->SetStroke( wxColour( 9, 215, 64 ), 10.0 );
    r->SetFill( wxColour( 220, 217, 200 ) );
    root->Append( r );

    a2dText* tt = new a2dText( _T( "Hello from a2dCanvas use\n Ctrl LeftDown to drag \n 'z' key or popup to zoom" ), -50, -70,
                               a2dFont( 50.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( wxColour( 0, 255, 0 ) );

    tt->SetObjectTip( _T( "make my day punk" ), 0, 50, 40 );

    root->Append( tt );

    MyCircle* cir2 = new  MyCircle( 430, 250, 160 );

    cir2->SetFill( wxColour( 250, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cir2->SetStroke( wxColour( 198, 3, 105 ), 30.0 );
    root->Prepend( cir2 );

    m_contr->SetTopTool( new a2dDragTool( m_contr ) );

    m_timer = new wxTimer( this );
    m_timer->Start( 1000, wxTIMER_CONTINUOUS ); // every second
}

void MyFrame::FillDoc5( a2dCanvasObject* root )
{
    //lets check the scroll area
    a2dRect* r = new a2dRect( -50, -40, 800, 600 );
    r->SetFill( *a2dTRANSPARENT_FILL );
    r->SetStroke( wxColour( 255, 255, 0 ), 2 );
    root->Append( r );

    // Bunch of rects and images.
    wxBitmap bitmap( smile_xpm );
    wxImage image( bitmap.ConvertToImage() );

    //show a polygon
    a2dPolygonL* prr = new a2dPolygonL();
    prr->AddPoint( 400, 0 );
    prr->AddPoint( 400, 200 );
    prr->AddPoint( 700, 200 );
    prr->AddPoint( 700, 100 );
    root->Append( prr );
    prr->SetFill(  *a2dTRANSPARENT_FILL );
    prr->SetStroke(  wxColour( 55, 250, 120 ) );

    //a clipper polygon
    a2dPolygonLClipper* rr = new a2dPolygonLClipper();
    rr->AddPoint( 400, 0 );
    rr->AddPoint( 400, 200 );
    rr->AddPoint( 700, 200 );
    rr->AddPoint( 700, 100 );
    root->Append( rr );

    a2dCircle* circclipped = new a2dCircle( 450, 100, 100 );
    circclipped->SetFill(  wxColour( 55, 250, 120 ) );

    rr->Append( circclipped );

    a2dRect* rc = new a2dRect( 460, 50, 137, 140 );
    rc->SetFill( wxColour( 230, 255, 0 ) );
    rr->Append( rc );

    m_smile1 = new a2dImage( image, 0, 70, 32, 32 );
    m_smile1->SetFill( *a2dTRANSPARENT_FILL );
    rr->Append( m_smile1 );

    a2dCircle* circ = new a2dCircle( 170, 70, 50 );
    circ->SetFill(  wxColour( 55, 55, 120 ) );
    root->Append( circ );

    a2dCanvasObject* line = new a2dSLine( -100, 0, 100, 100 );
    a2dAffineMatrix matrix = line->GetTransformMatrix();
    line->Scale( 1.4, 1.6 );
    line->Scale( 1.8, 0.4 );
    line->Scale( 12, 0.9 );
    line->SetTransformMatrix( matrix );
    root->Append( line );


    int i;
    for ( i = 10; i < 300; i += 10 )
    {
        a2dRect* r = new a2dRect( i, 50, 3, 140 );
        r->SetFill( wxColour( 0, 255, 0 ) );
        root->Append( r );
    }

    m_smile2 = new a2dImage( image, 0, 110, 32, 32 );
    m_smile2->SetFill( *a2dTRANSPARENT_FILL );
    root->Append( m_smile2 );

    for ( i = 15; i < 300; i += 10 )
    {
        a2dRect* r = new a2dRect( i, 50, 3, 140 );
        r->SetFill(  wxColour( 0, 255, 0 ) );
        root->Append( r );
    }

    a2dText* tt = new a2dText( _T( "Hello from a2dCanvas its World" ), 10, 20,
                               a2dFont( 10.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ), 15.0 );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( wxColour( 0, 255, 0 ) );

    root->Prepend( tt );

    //a2dDEFAULT_CANVASFONT = a2dFont( 200.0, wxDECORATIVE, wxFONTSTYLE_ITALIC );
    a2dFill fillshared = a2dFill( wxColour( 100, 17, 155 ) );

    a2dText* p = new a2dText( wxT( "Hello, world with shared font" ), 0, 100, *a2dDEFAULT_CANVASFONT  );
    //double dHeight = p->GetTextHeight();
    p->SetFill( fillshared );
    p->SetStroke( wxColour( 198, 3, 105 ), 0.0 );
    root->Append( p );

    a2dText* pp = new a2dText( wxT( "Hello, world with shared font" ), 0, 300, *a2dDEFAULT_CANVASFONT );
    pp->SetFill( fillshared );
    //this will change the text height of all text objects using/sharing this same a2dFontDc.
    pp->SetTextHeight( 50 );
    root->Append( pp );
    //dHeight = p->GetTextHeight();

    //a2dCanvasObjectList* childs = root->GetRootObject()->GetChildObjectList();
    //a2dCanvasObjectList copy;
    //std::copy( childs->begin(), childs->end(), copy.begin() );

    m_button = new wxButton( m_canvas, -1,  _T( "Show a button" ), wxPoint( 100, 100 ), wxSize( 100, 40 ) );

    m_timer = new wxTimer( this );
    //m_timer->Start( 1000, wxTIMER_CONTINUOUS ); // every second
    m_timer->Start( 80, false );
}

void MyFrame::FillDoc3( a2dCanvasObject* root )
{
	wxFont font( _( "0;-12;0;0;0;400;0;0;0;1;0;0;0;0;Segoe UI" ) );

    double y = 10;
    double sizef = 10;
    a2dText* tx = new a2dText( wxString::Format( _( "AÄpNormal %0.1f dempeatlstfl" ), sizef ), 3, y,
                               //a2dFont( font, sizef ),
                               a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Regular.ttf" ) ), sizef ),
                               //a2dFont( size, wxFONTFAMILY_SWISS ),
                               0, true );
    tx->SetAlignment( wxMINX | wxMINY );
    //tx->SetAlignment( wxMINX | wxMAXY);
    //tx->SetAlignment( wxMINX );
    //tx->SetAlignment( wxMAXX );
    //tx->SetAlignment( wxBASELINE );
    //tx->SetAlignment( wxBASELINE_CONTRA );
    tx->SetFill( *a2dTRANSPARENT_FILL );
    tx->SetBackGround();
    //tx->SetFill( *a2dTRANSPARENT_FILL );
    tx->SetFill( a2dFill( wxColour( 123, 120, 120 ) ) );
    root->Append( tx );
    y += sizef + 1.0;

    a2dText* tx2 = new a2dText( wxString::Format( _( "AÄpNormal %0.1f dempeatlstfl" ), sizef ), 3, y,
                               //a2dFont( font, sizef ),
                               a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Regular.ttf" ) ), sizef ),
                               //a2dFont( size, wxFONTFAMILY_SWISS ),
                               0, true );
    tx2->SetAlignment( wxMINX | wxMINY );
    //tx->SetAlignment( wxMINX | wxMAXY);
    //tx->SetAlignment( wxMINX );
    //tx->SetAlignment( wxMAXX );
    //tx->SetAlignment( wxBASELINE );
    //tx->SetAlignment( wxBASELINE_CONTRA );
    tx2->SetFill( *a2dTRANSPARENT_FILL );
    tx2->Rotate( -30 );
    root->Append( tx2 );
    y += sizef + 1.0;



    a2dDEFAULT_CANVASFONT = new a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT("LiberationSerif-Regular.ttf") ), 150.0, wxFONTENCODING_DEFAULT);    
    int maxText = 10;        // crashes if you set maxText to 510.
    for (int i = 0; i < maxText; ++i)
    {
		if ( i == 506 )
			i = 506;
		wxString textstr;
		textstr << wxT( "test text " ) << i;
        a2dText* text = new a2dText( textstr , 2*i, 2*i);
        text->SetTextHeight( i );    // This cause errors!
        root->Append( text );
    }

}

void MyFrame::FillDoc2( a2dCanvasObject* root )
{
	wxFont font( _( "0;-12;0;0;0;400;0;0;0;1;0;0;0;0;Segoe UI" ) );

    double y = 150;
    double sizef = 10;
    for ( double sizef = 1.5; sizef <= 8.1; sizef += ( sizef <= 3.45 ) ? 0.1 : 0.5 )
    {
        a2dText* tx = new a2dText( wxString::Format( _( "Normal %0.1f dempeatlstflÄ" ), sizef ), 150, y,
                                   //a2dFont( font, sizef ),
                                   a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Regular.ttf" ) ), sizef ),
                                   //a2dFont( size, wxFONTFAMILY_SWISS ),
                                   0, true );
        tx->SetAlignment( wxMINX | wxMINY );
        //tx->SetAlignment( wxMINX | wxMAXY);
        //tx->SetAlignment( wxMINX );
        //tx->SetAlignment( wxMAXX );
        //tx->SetAlignment( wxBASELINE );
        //tx->SetAlignment( wxBASELINE_CONTRA );
        tx->SetBackGround();
        tx->SetStroke( wxColour( 254, 15, 64 ), 1 );
        tx->SetFill( *a2dTRANSPARENT_FILL );
        tx->SetFill( a2dFill( wxColour( 223, 220, 220 ) ) );
        root->Append( tx );
        y += sizef + 3.0;
    }
}

void MyFrame::FillDoc4( a2dCanvasObject* root )
{
	a2dRect* r = new a2dRect( 0, 0, 80, 40 );
    r->SetStroke( wxColour( 9, 215, 64 ), 10.0 );
    r->SetFill( wxColour( 220, 217, 200 ) );
    root->Append( r );

    a2dText* tt = new a2dText( _T( "Hello from a2dCanvas use\n Ctrl LeftDown to drag \n 'z' key or popup to zoom" ), -50, -70,
                               a2dFont( 50.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( wxColour( 0, 255, 0 ) );

    tt->SetObjectTip( _T( "make my day punk" ), 0, 50, 40 );

    root->Append( tt );

    MyCircle* cir2 = new  MyCircle( 430, 250, 160 );

    cir2->SetFill( wxColour( 250, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
    cir2->SetStroke( wxColour( 198, 3, 105 ), 30.0 );
    root->Prepend( cir2 );

    m_contr->SetTopTool( new a2dDragTool( m_contr ) );

    m_timer = new wxTimer( this );
    m_timer->Start( 1000, wxTIMER_CONTINUOUS ); // every second
}

void MyFrame::FillDoc6( a2dCanvasObject* root )
{
    a2dVertexList* pointlistX = new a2dVertexList();
    a2dLineSegment* pointX = new a2dLineSegment( 0, 0 );
    pointlistX->push_back( pointX );
    pointX = new a2dLineSegment( 200, 0 );
    pointlistX->push_back( pointX );
    pointX = new a2dLineSegment( 200, 100 );
    pointlistX->push_back( pointX );

    a2dPolygonL* polyX = new a2dPolygonL( pointlistX );
    polyX->SetFill( wxColour( 255, 255, 2, 255 ) );
    root->Append( polyX );
  
    a2dCircle* c1  = new a2dCircle( 200, 300 , 150 );
    c1->SetStroke( a2dStroke( wxColour( 10, 222, 215 ), 30.0, a2dSTROKE_DOT_DASH) );
    c1->SetFill( a2dFill( wxColour( 100, 17, 155, 33 ), a2dFILL_HORIZONTAL_HATCH ) );
    c1->SetFill( wxColour( 255, 0, 0, 255 ), wxColour( 0, 0, 255, 50 ), a2dFILL_TWOCOL_HORIZONTAL_HATCH );
    root->Append( c1 );
    

    a2dCircle* c2  = new a2dCircle( 300, 300 , 125 );
    c2->SetFill( a2dFill( wxColour( 200, 170, 5, 22 ) ) );
    c2->SetLayer(1);
    root->Append( c2 );

    a2dCircle* c3  = new a2dCircle( 400, 350 , 100 );
    c3->SetFill( a2dFill( wxColour( 200, 70, 5 ) ) );
    c3->SetLayer(2);
    root->Append( c3 );

    a2dCircle* c4  = new a2dCircle( 450, 375 , 75 );
    //c4->SetFill( a2dFill( wxColour( 5, 200, 170, 152 ) ) );
    c4->SetFill( a2dFill( wxColour( 5, 200, 170 ) ) );
    c4->SetLayer(3);
    root->Append( c4 );

    a2dCircle* c5  = new a2dCircle( 550, 375 , 75 );
    c5->SetFill( a2dFill( wxColour( 125, 200, 170, 152 ) ) );
    //c5->SetFill( a2dFill( wxColour( 125, 200, 170 ) ) );
    c5->SetLayer(3);
    root->Append( c5 );

    a2dRect* r1 = new a2dRect( 100, 0, 50, 250 );
    r1->SetFill( *a2dTRANSPARENT_FILL );
    //r1->SetFill( wxColour( 82, 80, 2 ) );
    r1->SetStroke( wxColour( 2, 25, 75 ) );
    root->Append( r1 );

    a2dRect* r2 = new a2dRect( 25, 100, 150, 350 );
    r2->SetFill( wxColour( 29, 100, 92, 30 ) );
    r2->SetStroke( wxColour( 2, 55, 255 ) );
    root->Append( r2 );
  
    a2dVertexList* pointlist22 = new a2dVertexList();
    a2dLineSegment* point22 = new a2dLineSegment( -400, 500 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -500, 400 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -700, 300 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( 0, 100 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -300, 375 );
    pointlist22->push_back( point22 );

    a2dPolygonL* poly = new a2dPolygonL( pointlist22 );
    poly->SetFill( wxColour( 255, 255, 2, 255 ), wxColour( 0, 0, 255, 30 ), a2dFILL_TWOCOL_CROSSDIAG_HATCH );
    root->Append( poly );

    a2dSLine* xaxis = new a2dSLine( -110, 0, 210, 0 );
    xaxis->SetStroke( wxColour( 252, 0, 2 ) );
    root->Append( xaxis );

    //all you read next is a way to generate some graphics for a curve
    //but the a2dCanvas Library contains objects that do the same.
    //So this is only to learn.

    a2dSLine* yaxis = new a2dSLine( 0, -50, 0, 160 );
    yaxis->SetStroke( wxColour( 252, 0, 2 ), 1, a2dSTROKE_SHORT_DASH );
    root->Append( yaxis );
    double x;

    wxString line;
    double  result;
    int   a;
    a2dEval toeval;
    toeval.ClearAllVars();
    double xprev = 0;
    double yprev = 0;

    bool second = false;
    for ( x = -100; x < 200; x = x + 2 )
    {
        double y;
        if ( x != 0 )
        {
            toeval.SetValue(  _T( "x" ), x );
            line.Printf( _T( "y=100*(sin(x/20)/(x/20))" ) );
            toeval.SetEvalString( line );
            toeval.Evaluate( &result, &a );
            toeval.GetValue(  _T( "y" ), &y );
        }
        else
            y = 100; //sin(x)/x goes to one at zero

        a2dSLine* l = new a2dSLine( x, 0, x, y );
        l->SetStroke( wxColour( 2, 200, 252 ) );
        root->Append( l );

        if ( second )
        {
            a2dSLine* l2 = new a2dSLine( xprev, yprev, x, y );
            l2->SetStroke( wxColour( 0, 2, 252 ), 0.5 );
            root->Append( l2 );
        }
        xprev = x;
        yprev = y;
        second = true;
    }

    second = false;
    for ( x = -100; x < 200; x = x + 2 )
    {
        double y;
        if ( x != 0 )
        {
            toeval.SetValue(  _T( "x" ), x );
            line.Printf( _T( "y=140*(sin(x/10)/(x/10))" ) );
            toeval.SetEvalString( line );
            toeval.Evaluate( &result, &a );
            toeval.GetValue(  _T( "y" ), &y );
        }
        else
            y = 140; //sin(x)/x goes to one at zero

        a2dSLine* l = new a2dSLine( x, 0, x, y );
        l->SetStroke( wxColour( 252, 0, 252 ) );
        root->Append( l );

        if ( second )
        {
            a2dSLine* l2 = new a2dSLine( xprev, yprev, x, y );
            l2->SetStroke( wxColour( 0, 252, 252 ), 0.5 );
            root->Append( l2 );
        }
        xprev = x;
        yprev = y;
        second = true;
    }

    //add tics
    for ( x = -100; x < 200; x = x + 20 )
    {
        wxString tic;
        tic.Printf(  _T( "x=%3.0f" ), x );
        a2dText* tt = new a2dText( tic, x, -5,
                                   a2dFont( 2.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
        tt->SetFill( *a2dTRANSPARENT_FILL );
        tt->SetStroke( wxColour( 2, 2, 2 ) );
        root->Append( tt );

        a2dSLine* l = new a2dSLine( x, -1.5, x, 1.5 );
        l->SetStroke( wxColour( 2, 0, 2 ), 1.0 );

        root->Append( l );
    }

    double y;
    for ( y = -40; y < 160; y = y + 20 )
    {
        wxString tic;
        tic.Printf(  _T( "y=%3.0f" ), y );
        a2dText* tt = new a2dText( tic, 2, y,
                                   a2dFont( 2.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
        tt->SetFill( wxColour( 2, 0, 2 ) );
        tt->SetStroke( wxColour( 2, 255, 255 ) );
        root->Append( tt );

        a2dSLine* l = new a2dSLine( -1.5, y, 1.5, y );
        l->SetStroke( wxColour( 2, 0, 2 ), 2 );

        root->Append( l );
    }

    a2dText* tt = new a2dText(  _T( "Hello from wxWorldCanvas use\n Ctrl LeftDown to drag \n popup to zoom" ), -100, 270,
                                a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( wxColour( 0, 255, 0 ) );

    root->Append( tt );

    a2dRect* r3 = new a2dRect( -200, -300, 1500, 250 );
    r3->SetFill( *a2dTRANSPARENT_FILL );
    r3->SetFill( wxColour( 82, 80, 2, 100 ) );
    r3->SetStroke( wxColour( 250, 225, 75 ) );
    root->Append( r3 );

    wxString path = a2dGlobals->GetImagePathList().FindValidPath( "imagenPablo.bmp" ); //image2_transp.png

    a2dImage* im = new a2dImage( path, wxBITMAP_TYPE_BMP, 850, 450, 350, 550 );
 
    im->SetStroke( wxColour( 29, 255, 244 ), 15.0 );
    r3->SetFill( wxColour( 82, 80, 2, 255 ) );
    im->SetLayer( wxLAYER_DEFAULT );
    root->Append( im );

    path = a2dGlobals->GetImagePathList().FindValidPath( "smile.xpm" );
    a2dImage* im2 = new a2dImage( path, wxBITMAP_TYPE_XPM, 1340, 0, 400, 500 );
 
    im2->SetStroke( wxColour( 29, 255, 244 ), 15.0 );
    im2->SetFill( a2dFill( wxColour( 225, 20, 17, 52 ) ) );
    im2->SetLayer( wxLAYER_DEFAULT );
    root->Append( im2 );

    path = a2dGlobals->GetImagePathList().FindValidPath( "image2_transp.png" );
    a2dImage* im3 = new a2dImage( path, wxBITMAP_TYPE_PNG, 340, 900, 400, 600 );
 
    im3->SetStroke( wxColour( 29, 25, 244 ), 15.0 );
    im3->SetFill( a2dFill( wxColour( 0, 0, 0, 255 ) ) );
    //im3->SetFill( *a2dTRANSPARENT_FILL );
    im3->SetLayer( wxLAYER_DEFAULT );
    root->Append( im3 );

    path = a2dGlobals->GetImagePathList().FindValidPath( "image1.png" );
    a2dImage* im4 = new a2dImage( path, wxBITMAP_TYPE_PNG, 540, 800, 400, 600 );
 
    im4->SetStroke( wxColour( 255,0, 0), 1 );
    im4->SetFill( a2dFill( wxColour( 125, 200, 170, 100 ) ) );
    im4->SetLayer( wxLAYER_DEFAULT );
    root->Append( im4 );

    a2dSLine* yaxisw = new a2dSLine( 0, -500, 300, 560 );
    yaxisw->SetStroke( wxColour( 252, 120, 2, 120), 5, a2dSTROKE_DOT_DASH );
    root->Append( yaxisw );

    a2dVertexList* pointlist = new a2dVertexList();
    a2dLineSegment* point = new a2dLineSegment(-500,0);
    pointlist->push_back(point);
    point = new a2dLineSegment(-300,100);
    pointlist->push_back(point);
    point = new a2dLineSegment(-100,100);
    pointlist->push_back(point);
    point = new a2dLineSegment(-100,-100);
    pointlist->push_back(point);
    point = new a2dLineSegment(-200,-350);
    pointlist->push_back(point);

    a2dPolygonL* poly15= new a2dPolygonL(pointlist);

    wxBitmap gs_bmp36_mono;
    wxString pathp = a2dGlobals->GetImagePathList().FindValidPath( _T("pat36.bmp") );
    gs_bmp36_mono.LoadFile(pathp, wxBITMAP_TYPE_BMP);
    wxMask* mask36 = new wxMask(gs_bmp36_mono, *wxBLACK);
    // associate a monochrome mask with this bitmap
    gs_bmp36_mono.SetMask(mask36);

    a2dFill aa = a2dFill(gs_bmp36_mono);
    aa.SetColour(wxColour(0,8,216 ));
    aa.SetColour2(*wxRED);
    aa.SetStyle(a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT);
    //aa.SetStyle(a2dFILL_STIPPLE_MASK_OPAQUE);
    poly15->SetFill(aa);
    poly15->SetStroke(*wxRED,4.0);
    root->Append( poly15 );

}

void MyFrame::FillDoc1( a2dCanvasObject* root )
{
    /*
//    a2dText* tt=new a2dText( wxT("Normal dempel Hello from wxWorldCanvas use\n Ctrl LeftDown to drag \n popup to zoom"), -10, 20,
//                                            a2dFont( 80.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ));
    a2dText* tt = new a2dText( wxT( "abcampwonormalAWAVWo\nNormal dempel Hello from wxWorldCanvas use\n Ctrl LeftDown to drag \n popup to zoom" ), -10, 20,
                               a2dFont( 80.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL ) );

    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( wxColour( 230, 255, 0 ) );
    root->Append( tt );
    */
    a2dRect* r = new a2dRect( -100, -200, 30, 14 );
    r->SetStroke( wxColour( 129, 25, 64 ), 10.0 );
    r->SetFill( wxColour( 120, 217, 200 ), a2dFILL_CROSS_HATCH );
    root->Append( r );

    r = new a2dRect( 0, 0, 80, 40 );
    r->SetStroke( wxColour( 9, 215, 64 ), 10.0 );
    r->SetFill( wxColour( 220, 217, 200 ) );
    root->Append( r );

    r = new a2dRect( -200, 1800, 300, 140 );
    r->SetStroke( wxColour( 255, 5, 25 ), 30.0 );
    r->SetFill( wxColour( 244, 117, 5 ), a2dFILL_HORIZONTAL_HATCH );
    root->Append( r );

    r = new a2dRect( 1000, 2200, 300, 140 );
    r->SetStroke( wxColour( 255, 5, 25 ), 30.0 );
    r->SetFill( wxColour( 244, 117, 5 ), a2dFILL_HORIZONTAL_HATCH );
    root->Append( r );

#if wxART2D_USE_FREETYPE
    a2dRect* rt = new a2dRect( 0, 780, 190, -90 );
    rt->SetStroke( wxColour( 229, 25, 64 ), 1 );
    rt->SetFill( wxColour( 20, 217, 200 ), a2dFILL_TRANSPARENT );
    root->Append( rt );

    //a2dText* text2 = new a2dText( wxT("abca"), 0, 780, a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT("/LiberationSans-Bold.ttf") ), 900.0 ) );
    //a2dText* text2 = new a2dText( wxT("abcampwonormal"), 0, 780, a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT("/LiberationSans-Bold.ttf") ), 80.0 ) );
    a2dText* text2 = new a2dText( wxT( "abcampwonormalAWAVWo\nNormal dempel Hello from wxWorldCanvas use\n Ctrl LeftDown to drag \n popup to zoom" ), -10, 440, a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Regular.ttf" ) ), 80.0 ) );
    text2->SetNextLineDirection( true );
    text2->SetStroke( a2dStroke( *wxRED, 1 ) );
    text2->SetFill( a2dFill( wxColour( 0xc0, 0xff, 0xff ) ) );
    root->Prepend( text2 );

#endif //wxART2D_USE_FREETYPE

    a2dText* text = new a2dText( wxT( "editable normal mpawot\n text" ), 130, -50, a2dFont( 13.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL ) );
    text->SetStroke( a2dStroke( *wxRED, 1 ) );
    text->SetFill( a2dFill( wxColour( 0xc0, 0xff, 0xff ) ) );
    root->Prepend( text );

    wxString default_text(
        "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Cras sit amet\n"
        "dui.  Nam sapien. Fusce vestibulum ornare metus. Maecenas ligula orci,\n"
        "consequat vitae, dictum nec, lacinia non, elit. Aliquam iaculis\n"
        "molestie neque. Maecenas suscipit felis ut pede convallis malesuada.\n"
        "Aliquam erat volutpat. Nunc pulvinar condimentum nunc. Donec ac sem vel\n"
        "leo bibendum aliquam. Pellentesque habitant morbi tristique senectus et\n"
        "netus et malesuada fames ac turpis egestas.\n\n"

        "Sed commodo. Nulla ut libero sit amet justo varius blandit. Mauris vitae\n"
        "nulla eget lorem pretium ornare. Proin vulputate erat porta risus.\n"
        "Vestibulum malesuada, odio at vehicula lobortis, nisi metus hendrerit\n"
        "est, vitae feugiat quam massa a ligula. Aenean in tellus. Praesent\n"
        "convallis. Nullam vel lacus.  Aliquam congue erat non urna mollis\n"
        "faucibus. Morbi vitae mauris faucibus quam condimentum ornare. Quisque\n"
        "sit amet augue. Morbi ullamcorper mattis enim. Aliquam erat volutpat.\n"
        "Morbi nec felis non enim pulvinar lobortis.  Ut libero. Nullam id orci\n"
        "quis nisl dapibus rutrum. Suspendisse consequat vulputate leo. Aenean\n"
        "non orci non tellus iaculis vestibulum. Sed neque.\n\n" );


#if wxART2D_USE_FREETYPE
    a2dFont lorem( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Regular.ttf" ) ), 3.0 );
    //a2dFont lorem( a2dFont( 3.0, wxFONTFAMILY_SWISS, wxNORMAL ) );
    //a2dFont lorem( a2dGlobals->GetFontPathList().FindValidPath( wxT("/LiberationSerif-Regular.ttf") ),3.0 );
    a2dText* text3 = new a2dText( default_text, -300, -470, lorem );
    text3->SetStroke( a2dStroke( *wxBLACK, 1 ) );
    text3->SetFill( a2dFill( wxColour( 0xc0, 0xff, 0xff ) ) );
    text3->SetNextLineDirection( !m_canvas->GetYaxis() );
    root->Prepend( text3 );
#endif //wxART2D_USE_FREETYPE


    wxFont font( _( "0;-12;0;0;0;400;0;0;0;1;0;0;0;0;Segoe UI" ) );

    double y = 0;
    double sizef = 10;
    for ( sizef = 1.5; sizef <= 8.1; sizef += ( sizef <= 3.45 ) ? 0.1 : 0.5 )
    {
        a2dText* tx = new a2dText( wxString::Format( _( "Normal %0.1f dempeatlstfl" ), sizef ), 0, y,
                                   //a2dFont( font, sizef ),
                                   a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Regular.ttf" ) ), sizef ),
                                   //a2dFont( size, wxFONTFAMILY_SWISS ),
                                   0, true );
        tx->SetAlignment( wxMINX | wxMINY );
        //tx->SetAlignment( wxMINX | wxMAXY);
        //tx->SetAlignment( wxMINX );
        //tx->SetAlignment( wxMAXX );
        //tx->SetAlignment( wxBASELINE );
        //tx->SetAlignment( wxBASELINE_CONTRA );
        //tx->SetBackGround();
        tx->SetStroke( a2dStroke( *wxBLACK, 1 ) );
        tx->SetFill( a2dFill( wxColour( 223, 0, 220 ) ) );
        root->Append( tx );

        y += sizef + 1.0;
    }
}

void MyFrame::FillDoc7( a2dCanvasObject* root )
{
//    randomize();
    int i;
    for( i = 0; i < 1000; i++ )
    {
        a2dRectC* r = new a2dRectC( rand() % 1000 - 400, rand() % 1000 - 400, 30 + rand() % 100, 20 + rand() % 300 );
        r->SetLayer( 5 );
        root->Append( r );
    }

    for( i = 0; i < 200; i++ )
    {
        a2dRect* r = new a2dRect( rand() % 1000 - 400, rand() % 1000 - 400, 30 + rand() % 100, 20 + rand() % 300 );
        r->SetFill( wxColour( rand() % 254, rand() % 254, rand() % 254, rand() % 250 ) );
        r->SetStroke( wxColour( rand() % 254, rand() % 254, rand() % 254 ), double( rand() % 10 ) );
        r->SetLayer( 1 );
        root->Append( r );
    }

    for( i = 0; i < 100; i++ )
    {
        a2dRect* r = new a2dRect( rand() % 1000 - 400, rand() % 1000 - 400, 30 + rand() % 100, 20 + rand() % 300 );
        r->SetFill( wxColour( rand() % 254, rand() % 254, rand() % 254, rand() % 250 ) );
        r->SetStroke( wxColour( rand() % 254, rand() % 254, rand() % 254 ), double( rand() % 10 ) );
        r->SetLayer( 0 );
        root->Append( r );
    }

}

void MyFrame::OnDragSimulate(wxCommandEvent& event)
{
    a2dDragTool* drag = new a2dDragTool( m_contr );
    drag->SetDropAndDrop( true );
    m_contr->PushTool( drag );

}

void MyFrame::OnCopyShape(wxCommandEvent& WXUNUSED(event))
{
    a2dDrawingPart* part = m_canvas->GetDrawingPart();
    a2dDnDCanvasObjectDataCVG* copied = new a2dDnDCanvasObjectDataCVG( part, wxDataObject::Get );
    copied->SetExportWidthHeightImage( 10000 );
    //a2dDnDCameleonData* copied = new a2dDnDCameleonData();

    if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
    {
        wxClipboardLocker clipLocker;
        if ( !clipLocker )
        {
            wxLogError(wxT("Can't open the clipboard"));

            return;
        }

        wxTheClipboard->AddData( copied );
    }
}

void MyFrame::OnPasteShape(wxCommandEvent& WXUNUSED(event))
{
    wxClipboardLocker clipLocker;
    if ( !clipLocker )
    {
        wxLogError(wxT("Can't open the clipboard"));
        return;
    }

    a2dDrawingPart* part = m_canvas->GetDrawingPart();

    if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
    {
        a2dDnDCanvasObjectDataCVG shapeDataObject( part, wxDataObject::Set );
        //a2dDnDCameleonData shapeDataObject(NULL);
		shapeDataObject.SetExportWidthHeightImage( 10000 );
        shapeDataObject.SetImportWidthHeightImage( 10000 );
        if ( wxTheClipboard->GetData(shapeDataObject) )
        {
            a2dDrawingPtr drawing = shapeDataObject.GetDrawing();
            a2dCanvasObjectPtr shape = drawing->GetRootObject();

			// for all selected objects (to copy paste ) set pending flags ON. 
			a2dWalker_SetPendingFlags setflags1( a2dCanvasOFlags::SELECTED );
			setflags1.Start( part->GetShowObject(), true );
			// now reset selection flags in current drawing
            a2dWalker_SetSpecificFlagsCanvasObjects setflags2( a2dCanvasOFlags::SELECTED | a2dCanvasOFlags::SELECTED2 );
            setflags2.Start( part->GetShowObject(), false );
            shape->Update( a2dCanvasObject::updatemask_force );
            shape->SetRoot( part->GetDrawing() );
            a2dBoundingBox box = shape->GetBbox();
            a2dPoint2D p = box.GetCentre();
            shape->Translate( -p.m_x, -p.m_y );
            shape->EliminateMatrix();
            
            // to test
            //root->Append( shape );

            if ( shape->GetChildObjectList()->size() )
            {
                a2dDragMultiNewTool* dragnew = new a2dDragMultiNewTool( (a2dStToolContr *) (part->GetCanvasToolContr()), shape->GetChildObjectList() );
                dragnew->SetOneShot();
                dragnew->SetStroke( a2dStroke( *wxRED, 1 ) );
                dragnew->SetFill( a2dFill( wxColour( 0xc0, 0xff, 0xff ) ) );
                ((a2dStToolContr *) (part->GetCanvasToolContr()))->PushTool( dragnew );
            }            
            else
            {
                wxLogStatus(wxT("No shape on the clipboard"));
            }
        }
        else
        {
            wxLogStatus(wxT("No shape on the clipboard"));
        }
    }
}

void MyFrame::OnUpdatePasteShape( wxUpdateUIEvent& event  )
{
    a2dDrawingPart* part = m_canvas->GetDrawingPart();
    a2dDnDCanvasObjectDataCVG shapeDataObject( part, wxDataObject::Set );
    wxDataFormat formats[10];
    shapeDataObject.GetAllFormats( formats );
    int nr = shapeDataObject.GetFormatCount();

    bool ret = false;
        
    for ( int i = 0 ; i < nr ; i++ )
    {
        ret |= wxTheClipboard->IsSupported( formats[i] );
    }

    event.Enable( ret );
}

void MyFrame::OnUpdateCopyShape( wxUpdateUIEvent& event  )
{
    bool ret = false;
    a2dDrawingPart* part = m_canvas->GetDrawingPart();
    if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
    {
        a2dCanvasObjectList* objects = part->GetShowObject()->GetChildObjectList();
        forEachIn( a2dCanvasObjectList, objects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj->GetRelease() || !obj->IsVisible()  || !obj->GetSelected() )
                continue;
            ret = true;
            break;
        }
    }
    event.Enable( ret );
}

void MyFrame::AddCmdToToolbar( const a2dMenuIdItem& cmdId )
{
    if ( ! GetToolBar() )
        return;
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MyFrame::OnCmdMenuId ) );

    wxASSERT_MSG( a2dMenuIdItem::GetInitialized(), _T( "call a2dMenuIdItem::InitializeBitmaps() before using AddCmdToToolbar" ) );

    wxString error = _T( "No Bitmap for a2dToolCmd found for:" ) + cmdId.GetIdName();
    wxASSERT_MSG( cmdId.GetBitmap().Ok(), error );
    GetToolBar()->AddTool( cmdId.GetId(), cmdId.GetLabel(), cmdId.GetBitmap(), cmdId.GetHelp() );
}

void MyFrame::ConnectCmdId( const a2dMenuIdItem& cmdId )
{
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MyFrame::OnCmdMenuId ) );
}

void MyFrame::AddCmdMenu( wxMenu* parentMenu, wxMenuItem* item )
{
    Connect( item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MyFrame::OnCmdMenuId ) );
    parentMenu->Append( item );
}

void MyFrame::AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
{
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MyFrame::OnCmdMenuId ) );
    parentMenu->Append( cmdId.GetId(), cmdId.GetLabel(), cmdId.GetHelp(), cmdId.IsCheckable() );
}

void MyFrame::RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
{
    Disconnect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MyFrame::OnCmdMenuId ) );
    parentMenu->Delete( cmdId.GetId() );
}

void MyFrame::OnCmdMenuId( wxCommandEvent& event )
{
    if ( m_canvas->GetDrawing()->GetCommandProcessor() )
    {
        m_canvas->GetDrawing()->GetCommandProcessor()->ProcessEvent( event );
    }
    else
        event.Skip();
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
    wxInitAllImageHandlers();

    wxString artroot = a2dGeneralGlobals->GetWxArt2DVar();
    if ( artroot.IsEmpty() )
        return false;

    a2dGlobals->GetFontPathList().Add( artroot + wxT( "samples/editor/common/fonts" ) );
    a2dGlobals->GetFontPathList().Add( wxT( "/usr/share/fonts/truetype/msttcorefonts" ) );
    a2dGlobals->GetImagePathList().Add( artroot +  wxT( "samples/editor/common/images" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "art/resources" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./" ) );
    a2dCanvasGlobals->GetLayersPathList().Add( artroot +  wxT( "art/layers" ) );

    delete a2dDEFAULT_CANVASFONT;
    a2dDEFAULT_CANVASFONT = new a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Bold.ttf" ) ), 15.0 );
    a2dCanvasGlobals->GetHabitat()->SetFont( *a2dDEFAULT_CANVASFONT ); 

    m_frame = new MyFrame( NULL, -1, _T( "SuperApp" ), wxPoint( 20, 30 ), wxSize( 600, 440 ) );
    m_frame->Show( true );


    m_frame->m_contr->Zoomout();

    return true;
}

int MyApp::OnExit()
{
    return 0;
}


