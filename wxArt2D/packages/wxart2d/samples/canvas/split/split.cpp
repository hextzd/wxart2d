/*! \file canvas/samples/split/src/split.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: split.cpp,v 1.4 2009/02/20 21:05:51 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "split.h"

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

// Include image
#include "smile.xpm"

#include <algorithm>

// WDR: class implementations


class MyCanvasWidgetButtonGroup: public a2dWidgetButtonGroup
{
public:

    //! Standard constructor
    MyCanvasWidgetButtonGroup( a2dCanvasObject* parent, double x, double y, double extra = 0 )
        : a2dWidgetButtonGroup( parent, x, y, extra )
    {}


    DECLARE_EVENT_TABLE()

    void OnButton1( wxCommandEvent& event );
    void OnButton2( wxCommandEvent& event );
    void OnButton3( wxCommandEvent& event );
    void OnButton4( wxCommandEvent& event );
};

BEGIN_EVENT_TABLE( MyCanvasWidgetButtonGroup, a2dWidgetButtonGroup )
    EVT_BUTTON( 1, MyCanvasWidgetButtonGroup::OnButton1 )
    EVT_BUTTON( 2, MyCanvasWidgetButtonGroup::OnButton2 )
    EVT_BUTTON( 3, MyCanvasWidgetButtonGroup::OnButton3 )
    EVT_BUTTON( 4, MyCanvasWidgetButtonGroup::OnButton4 )
END_EVENT_TABLE()

void MyCanvasWidgetButtonGroup::OnButton1( wxCommandEvent& event )
{
    ( void )wxMessageBox( wxT( "Button 1 pressed" ), wxT( "Button message" ) );
}


void MyCanvasWidgetButtonGroup::OnButton2( wxCommandEvent& event )
{
    ( void )wxMessageBox( wxT( "Button 2 pressed" ), wxT( "Button message" ) );
}

void MyCanvasWidgetButtonGroup::OnButton3( wxCommandEvent& event )
{
    ( void )wxMessageBox( wxT( "Button 3 pressed" ), wxT( "Button message" ) );
}

void MyCanvasWidgetButtonGroup::OnButton4( wxCommandEvent& event )
{
    ( void )wxMessageBox( wxT( "Button 4 pressed" ), wxT( "Button message" ) );
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

// WDR: event table for MyFrame

BEGIN_EVENT_TABLE( MyFrame, a2dDrawingEditor )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_TIMER( -1, MyFrame::OnTimer )
    EVT_DRAWINGEDITOR_THEME_EVENT( MyFrame::OnTheme )
    EVT_DRAWINGEDITOR_INIT_EVENT( MyFrame::OnInit )  
END_EVENT_TABLE()

MyFrame::MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    a2dDrawingEditor( NULL, parent, id, title, position, size, style )
{
    m_drawing = new a2dDrawing();

    Init();

    a2dCanvasGlobals->SetActiveDrawingPart( m_canvas->GetDrawingPart() );

    m_initialized = true;
    // need to know change in active drawingpart
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );

    SetStatusText( wxT( "Welcome to a2dCanvas sample!" ) );

    SetIcon( wxICON( mondrian ) );

    m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_MENUSTRINGS, this );
    m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_DO, this );
    m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_UNDO, this );
    m_drawingPart->GetDrawing()->GetCanvasCommandProcessor()->SetMenuStrings();

    //NEXT 7 lines or optional, and depend on what you need.

    //if you do not want the origin shown
    //m_canvas->SetShowOrigin( false );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_drawingPart->GetDrawer2D()->SetMappingUpp( -50, -40, 1, 1 );

    m_drawingPart->GetDrawer2D()->SetYaxis( false );
    //defines the whole area in world coordinates that can be seen by scrolling
    m_drawingPart->GetCanvas()->SetScrollMaximum( -50, -40, 800, 600 );
    //defines the number of world units scrolled when line up or down events in X
    m_drawingPart->GetCanvas()->SetScrollStepX( 50 );
    //defines the number of world units scrolled when line up or down events in Y
    m_drawingPart->GetCanvas()->SetScrollStepY( 50 );
    //do not go outide the scroll maximum
    m_drawingPart->GetCanvas()->FixScrollMaximum( true ); //what to do when resizing above ScrollMaximum is still ??


    //Start filling document now.

    // One object group is the root in every canvas.
	a2dCanvasObject* root = m_drawingPart->GetDrawing()->GetRootObject();

    /*
    //lets check the scroll area
    a2dRect* r = new a2dRect( -50, -40, 800, 600 );
    r->SetFill( *a2dTRANSPARENT_FILL );
    r->SetStroke( wxColour( 255, 255, 0 ), 2 );
    root->Append( r );

    // Bunch of rects and images.
    wxBitmap bitmap( smile_xpm );
    wxImage image( bitmap.ConvertToImage() );

    m_smile1 = new a2dImage( image, 0, 70, 32, 32 );
    m_smile1->SetFill( *a2dTRANSPARENT_FILL );
    root->Append( m_smile1 );

    a2dCircle* circ = new a2dCircle( 170, 70, 50 );
    circ->SetFill(  wxColour( 55, 55, 120 ) );
    root->Append( circ );

    int i;
    for ( i = 10; i < 300; i += 10 )
    {
        a2dRect* r = new a2dRect( i, 50, 3, 140 );
        r->SetFill( wxColour( 0, 255, 0 ) );
        root->Append( r );
    }

    MyCanvasWidgetButtonGroup* butgroup = new MyCanvasWidgetButtonGroup( root, 100, 200, 20 );
    butgroup->SetFill(  wxColour( 255, 235, 220 ) );

    root->Append( butgroup );

    a2dRect* rect2 = new a2dRect( 0, 0, 50, 20 );
    rect2->SetFill( wxColour( 120, 255, 120 ) );
    a2dCircle* circ3 = new a2dCircle( 0, 0, 35 );
    circ3->SetFill(  wxColour( 165, 55, 20 ) );
    a2dCanvasObject* groupcontent = new a2dCanvasObject();
	groupcontent->SetRoot( m_drawingPart->GetDrawing() );
    groupcontent->Append( rect2 );
    groupcontent->Append( circ3 );

    a2dWidgetButton* button1 = new a2dWidgetButton( butgroup, 1, -40, 70, 80, 50, a2dWidgetButton::ScaledContent );
    butgroup->Append( button1 );
    button1->SetContentObject( groupcontent );
    button1->SetHighLightStroke( a2dStroke( wxColour( 10, 222, 215 ), 3.0 ) );
    button1->SetHighLightFill( a2dFill( wxColour( 100, 17, 155 ), a2dFILL_HORIZONTAL_HATCH ) );
    button1->SetFill( wxColour( 195, 195, 195 ) );
    button1->SetStroke( wxColour( 195, 0, 0 ) );

    a2dText* tip = button1->SetObjectTip( _T( "make my day punk" ) , 0, 70, 10 );
    tip->SetFill( wxColour( 195, 5, 195 ) );

    a2dWidgetButton* button2 = new a2dWidgetButton( butgroup, 2, 90, 70, 0, 0 ); //39,81 );
    butgroup->Append( button2 );
    button2->SetContentObject( groupcontent );
    button2->SetHighLightStroke( a2dStroke( wxColour( 130, 222, 215 ), 3.0 ) );
    button2->SetHighLightFill( a2dFill( wxColour( 10, 17, 15 ), a2dFILL_VERTICAL_HATCH ) );
    button2->SetFill( wxColour( 165, 165, 165 ) );
    button2->SetStroke( wxColour( 165, 0, 0 ) );
    a2dText* tip2 = button2->SetObjectTip( _T( "its a dog eat dog eat cat too (AcDc)" ) , 0, 70, 10 );
    tip2->SetFill( wxColour( 5, 196, 195 ) );

    a2dCircle* circ4 = new a2dCircle( 0, 0, 35 );
    circ3->SetFill(  wxColour( 0, 0, 128 ) );
    a2dWidgetButton* button3 = new a2dWidgetButton( butgroup, 3, 170, 70, 1, 1 );
    butgroup->Append( button3 );
    button3->Append( circ4 );

    a2dRect* r2 = new a2dRect( 0, 0, 50, 20 );
    r2->SetFill( wxColour( 120, 255, 120 ) );
    a2dWidgetButton* button4 = new a2dWidgetButton( butgroup, 4, 120, 170, 1, 1 );
    butgroup->Append( button4 );
    button4->Append( r2 );

    m_smile2 = new a2dImage( image, 0, 110, 32, 32 );
    m_smile2->SetFill( *a2dTRANSPARENT_FILL );
    root->Append( m_smile2 );


    //Setup a pin class map array to define which pins can connect, and with which wire
    a2dWirePolylineL* wirenew = new a2dWirePolylineL();
    wirenew->SetStroke( a2dStroke( wxColour( 255, 50, 0 ), 2, a2dSTROKE_SHORT_DASH ) );
    wirenew->SetLayer( 2 );

    wirenew->SetStartPinClass( a2dPinClass::Standard );
    wirenew->SetEndPinClass( a2dPinClass::Standard );
    a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->SetConnectObject( wirenew );

    m_circle = new a2dCircle( 580, 160, 55 );
    a2dPin* pincircle = m_circle->AddPin( wxT( "pincircle" ), 0, 0, a2dPin::objectPin, a2dPinClass::Standard );
    pincircle->SetInternal( true );
    root->Prepend( m_circle );

    m_rect = new a2dRect( 320, 260, 135, 135 );
    a2dPin* pinrect = m_rect->AddPin( wxT( "pinrect" ), 0, 0, a2dPin::objectPin, a2dPinClass::Standard );
    pinrect->SetInternal( true );
    root->Prepend( m_rect );

    a2dText* text = new a2dText( wxT( "editable\n text" ), 330, 150, a2dFont( 30.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    text->SetStroke( a2dStroke( *wxRED, 1 ) );
    text->SetFill( a2dFill( wxColour( 0xc0, 0xff, 0xff ) ) );
    root->Prepend( text );
    a2dPin* pintext = text->AddPin( wxT( "pintext" ), 0, 0, a2dPin::objectPin, a2dPinClass::Standard );
    pintext->SetInternal( true );
    a2dPin* pintext2 = text->AddPin( wxT( "pintext" ), 50, 20, a2dPin::objectPin, a2dPinClass::Standard );
    pintext2->SetInternal( true );

    a2dCanvasObject* wire = a2dPinClass::Standard->GetConnectionGenerator()->CreateConnectObject( root, pinrect, pintext, false );
    root->Prepend( wire );

    //wire = a2dPinClass::Standard->GetConnectionGenerator()->CreateConnectObject( root, pincircle, pintext2, false );
    //root->Append( wire );
    a2dCanvasObject* parento = root;

    a2dWirePolylineL* wiren = new a2dWirePolylineL();
    wiren->AddPoint( pincircle->GetAbsX(), pincircle->GetAbsY(), -1 , true );
    wiren->AddPoint( 500, 100, -1 , true );
    wiren->AddPoint( 500, 200, -1 , true );
    wiren->AddPoint( pintext2->GetAbsX(), pintext2->GetAbsY(), -1 , true );

    a2dPin* begin = wiren->AddPin( wxT( "begin" ), pincircle->GetAbsX(), pincircle->GetAbsY(), a2dPin::NON, a2dPinClass::Standard );
    wiren->ConnectPins( parento, pincircle, begin, false );
    a2dPin* end = wiren->AddPin( wxT( "end" ), pintext2->GetAbsX(), pintext2->GetAbsY(), a2dPin::NON, a2dPinClass::Standard );
    wiren->ConnectPins( parento, pintext2, end, false );

    root->Append( wiren );

    m_timer = new wxTimer( this );
    m_timer->Start( 80, false );
    */
}

MyFrame::~MyFrame()
{
    //delete m_timer;
}

void MyFrame::OnInit( a2dDrawingEditorFrameEvent& initEvent )
{
#ifdef __WXMSW__
    SetIcon( wxString( wxT( "chrt_icn" ) ) );
#endif

    m_editMenu = NULL;

    if ( m_drawingPart )
    {
        a2dFill backgr = a2dFill( wxColour( 255, 255, 255 ) );
        m_drawingPart->SetBackgroundFill( backgr );

        m_drawingPart->SetGridStroke( a2dStroke( wxColour( 239, 5, 64 ), 0, a2dSTROKE_DOT ) );
        //to show grid as lines
        //m_drawingPart1->SetGridLines(true);

        m_drawingPart->SetGridSize( 1 );
        m_drawingPart->SetGridX( 100 );
        m_drawingPart->SetGridY( 100 );

        //show it?
        m_drawingPart->SetGrid( false );
        m_drawingPart->SetGridAtFront( true );
    }
    m_initialized = true;
}

void MyFrame::OnTheme( a2dDrawingEditorFrameEvent& themeEvent )
{

    if ( 1 )
    {
        CreateControls();
        CreateThemeDefault();

        //CreateMenubar( false );
    }
    else
        CreateThemeXRC();

    //Theme();
/*
    wxMenuBar* bar =  GetMenuBar();
	wxMenu* menuObject = new wxMenu;
	bar->Append( menuObject, _("Object") );	

	//------------------
	wxMenu* submenuArrange = new wxMenu;
	AddCmdMenu( submenuArrange, CmdMenu_Selected_Group() );
	AddCmdMenu( submenuArrange, CmdMenu_Selected_UnGroup() );
	submenuArrange->AppendSeparator();
	AddCmdMenu( submenuArrange, CmdMenu_Selected_ToTop() );
	AddCmdMenu( submenuArrange, CmdMenu_Selected_ToBack() );
	//---
	menuObject->AppendSubMenu( submenuArrange, _("Arrange") );	
	//----------------
	wxMenu* submenuAlign = new wxMenu;
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MinX_Dest() );
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MaxX_Dest() );
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MinY_Dest() );
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MaxY_Dest() );
	submenuAlign->AppendSeparator();
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MidX_Dest() );
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MidY_Dest() );
	submenuAlign->AppendSeparator();
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_DistributeVert() );
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_DistributeHorz() );
	//---
	menuObject->AppendSubMenu( submenuAlign, _("Align") );
	menuObject->AppendSeparator();
	AddCmdMenu( menuObject, CmdMenu_Selected_Align_MirrorVertBbox() );
	AddCmdMenu( menuObject, CmdMenu_Selected_Align_MirrorHorzBbox() );
	AddCmdMenu( menuObject, CmdMenu_Selected_RotateObject90Left() );
	AddCmdMenu( menuObject, CmdMenu_Selected_RotateObject90Right() );

    menuObject->AppendSeparator();
	AddCmdMenu( menuObject, CmdMenu_Selected_SetExtStyle() );
	AddCmdMenu( menuObject, CmdMenu_Selected_SetTextChanges() );

    wxMenu* drawMenu = new wxMenu;
    drawMenu->Append( MDSCREEN_0, wxT( "many images" ), _( "image + ref rotated image+ vector objects on layers" ) );
    drawMenu->Append( MDSCREEN_1, wxT( "multi refs" ), _( "all/most objects and a group with refs to it" ) );
    drawMenu->Append( MDSCREEN_2, wxT( "layered objects" ), _( "layers in hierarchy" ) );
    drawMenu->Append( MDSCREEN_3, wxT( "Window and widgets" ), _( "windows and widgets" ) );
    drawMenu->Append( MDSCREEN_4, wxT( "whatever" ), _( "layers in hierarchy" ) );
    drawMenu->Append( MDSCREEN_5, wxT( "top struct translated" ), _( "top struct translated tests" ) );
    drawMenu->Append( MDSCREEN_6, wxT( "enterleave" ), _( "enter leave events" ) );
    drawMenu->Append( MDSCREEN_10, wxT( "properties" ), _( "properties" ) );

    wxMenu* drawEng = new wxMenu;
    drawEng->Append( Switch_Drawer_aggRgba, _T( "Switch Drawer &Agg AntiAlias Rgba" ) );
    drawEng->Append( Switch_Drawer_agg, _T( "Switch Drawer &Agg AntiAlias" ) );
    drawEng->Append( Switch_Drawer_agggc, _T( "Switch Drawer Graphics &Context &Agg Gc" ) );

    drawEng->Append( Switch_Drawer_dc, _T( "Switch Drawer &wxDC" ) );
    drawEng->Append( Switch_Drawer_gdiplus, _T( "Switch Drawer &GdiPlus" ) );
    drawEng->Append( Switch_Drawer_gdigc, _T( "Switch Drawer Graphics &Context GDI" ) );
    drawEng->Append( Switch_Drawer_dcgc, _T( "Switch Drawer Graphics Context &wxDC" ) );

    bar->Append( drawMenu, _( "&DemoScreens" ) );
    bar->Append( drawEng, _T( "&Drawer Type" ) );


    wxMenu* menumasterTool = new wxMenu;
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_SelectFirst() );
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_ZoomFirst() );
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_TagGroups() );
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_TagGroups_WireMode() );
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_TagGroups_DlgMode() );
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_TagGroups_WireMode_DlgMode() );

    bar->Append( menumasterTool, _T( "&Choose MasterTool" ) );

    wxMenu* fileMenu = m_menubar->GetMenu( m_menubar->FindMenu( wxT( "File" ) ) );
    //AddCmdMenu( file_menu, CmdMenu_FileNew() );
    //fileMenu->Insert( 0, wxID_NEW, _("&New..."), _("Create new file") );
    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( fileMenu );
*/
    a2dStToolContr* contr = wxStaticCast( m_drawingPart->GetCanvasToolContr(), a2dStToolContr );

    contr->GetFirstTool()->SetShowAnotation( false );
    contr->SetZoomFirst( false );
}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    Destroy();
}

void MyFrame::OnTimer( wxTimerEvent& event )
{
    a2dDrawing* root = m_canvas->GetDrawing();
	a2dCanvasObject* parent = root->GetRootObject();

    a2dCanvasObjectList dragList;
    dragList.push_back( m_circle );
    dragList.push_back( m_rect );

    a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->CreateWiresOnPins( parent, dragList, true, false );

    //find all wires going to this object.
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
    setflags.Start( parent, false );
    // find all wires which do go to the objects, and are to be rerouted.
    a2dCanvasObjectList connectedwires;

    a2dCanvasObjectList::iterator iter;
    for( iter = dragList.begin(); iter != dragList.end(); ++iter )
    {
        a2dCanvasObject* original = *iter;
        original->SetBin( true );
        original->FindConnectedWires( connectedwires, NULL, true, false, false );
    }

    iter = connectedwires.begin();
    while( iter != connectedwires.end() )
    {
        a2dCanvasObjectList::value_type wire = *iter;
        if ( std::find( dragList.begin(), dragList.end(), wire ) !=  dragList.end() )
            iter = connectedwires.erase( iter );
        else
        {
            wire->SetBin( false );
            wire->EliminateMatrix();
            iter++;
        }
    }

    // All non selected wires found connected to m_objects found!
    a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->AddRerouteWires( &connectedwires, parent );
    a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->OptimizeRerouteWires();
    a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->PrepareForRerouteWires();
    setflags.Start( parent, false );

    m_smile1->Translate( 1, 0 );
    m_smile2->Translate( 1, 0 );
    m_rect->Translate( 1, 0 );
    m_circle->Translate( 1, 1 );

    a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->RerouteWires( false );
    a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );
    wxWakeUpIdle();
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
    wxString artroot = a2dGeneralGlobals->GetWxArt2DVar();
    if ( artroot.IsEmpty() )
        return false;

    a2dGlobals->GetIconPathList().Add( artroot + wxT( "art/resources" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "art/layers" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./" ) );
    a2dCanvasGlobals->GetLayersPathList().Add( artroot +  wxT( "art/layers" ) );

    wxInitAllImageHandlers();
    a2dMenuIdItem::InitializeBitmaps();

    m_frame = new MyFrame( NULL, -1, wxT( "SuperApp" ), wxPoint( 20, 30 ), wxSize( 800, 800 ) );
    m_frame->Show( true );

    return true;
}

int MyApp::OnExit()
{
    return 0;
}


