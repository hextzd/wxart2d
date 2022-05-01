/*! \file editor/samples/tuto4/simple.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: tuto4.cpp,v 1.6 2009/02/20 21:05:52 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "tuto4.h"

#if wxART2D_USE_CURVES
#include "wx/curves/meta.h"
#endif //wxART2D_USE_CURVES

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

BEGIN_EVENT_TABLE( PlotCanvas, a2dCanvas )
    EVT_SIZE( PlotCanvas::OnSize )
    EVT_IDLE( PlotCanvas::OnIdle )
END_EVENT_TABLE()

// Define a constructor for my canvas
PlotCanvas::PlotCanvas(  wxFrame* frame, const wxPoint& pos, const wxSize& size, const long style ):
    a2dCanvas( frame, -1, pos, size, style )
{
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( this->GetDrawingPart(), frame );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );

    SetYaxis( true );

    m_startup = true;
}

void PlotCanvas::OnSize( wxSizeEvent& event )
{
    a2dCanvas::OnSize( event );

    if ( !m_wantScroll ) //zoom all is not having scrollbars here
        SetMappingShowAll();

    //For always seeing everything
    //m_contr->Zoomout();

    //For having a fixed Lower Left Corner.
    //SetMappingUpp(-150,-1000,GetUppX(),GetUppY());

    //For having the same Lower Left Corner
    //SetMappingUpp( GetVisibleMinX() ,GetVisibleMinY(),GetUppX(),GetUppY());

    m_drawingPart->Update( a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );
}

void PlotCanvas::OnIdle( wxIdleEvent& event )
{
    if ( m_startup )
    {
        m_startup = false;
        SetMappingShowAll();
    }
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE( MyFrame, wxFrame )
    EVT_MENU( ID_QUIT, MyFrame::OnQuit )
    EVT_CLOSE( MyFrame::OnCloseWindow )
END_EVENT_TABLE()

MyFrame::MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    wxFrame( parent, id, title, position, size, style )
{
    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText(  _T( "Welcome to a2dCanvas sample!" ) );

    SetIcon( wxICON( mondrian ) );

    m_canvas = new PlotCanvas( this, wxPoint( 0, 0 ), size, wxHSCROLL | wxVSCROLL );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->SetMappingUpp( -150, -1000, 3, 3 );

    //defines the whole area in world coordinates that can be seen by scrolling
    m_canvas->SetScrollMaximum( 0, 0, 1600, 1600 );
    //defines the number of world units scrolled when line up or down events in X
    m_canvas->SetScrollStepX( 100 );
    //defines the number of world units scrolled when line up or down events in Y
    m_canvas->SetScrollStepY( 100 );

    //m_canvas->SetScrollBarsVisible( false );

    a2dCanvasObject* root = m_canvas->GetDrawing()->GetRootObject();

    //The next rectangle is used to easily zoomout to the whole drawing,
    //including a small border.
    a2dRect* maxZoomRec = new a2dRect( -150, -1000, 1050, 1350 );
    maxZoomRec->SetFill( *a2dTRANSPARENT_FILL );
    maxZoomRec->SetStroke( *a2dBLACK_STROKE );
    root->Append( maxZoomRec );

#if wxART2D_USE_CURVES

    a2dCanvasXYDisplayGroup* curvegroup = new a2dCanvasXYDisplayGroup( 500, -600 );
    curvegroup->SetStroke( wxColour( 9, 227, 5 ), 2.0 );
    curvegroup->SetBoundaries( -100, -200, 350, 250 );
    curvegroup->SetInternalBoundaries( -100, -200, 350, 250 );
    curvegroup->GetAxisX()->SetTicTextHeight( 10 );
    curvegroup->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 9, 227, 5 ), 1.0 ) );
    curvegroup->GetAxisX()->SetTicTextStroke( a2dStroke( wxColour( 229, 12, 23 ), 1.0 ) );
    curvegroup->GetAxisX()->SetStroke( a2dStroke( *wxRED_PEN ) );
    curvegroup->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    curvegroup->SetFill( *a2dTRANSPARENT_FILL );
    curvegroup->SetFill( *wxLIGHT_GREY, a2dFILL_SOLID );
    //next slow but works of course
    //curvegroup->SetFill( new a2dRadialGradientFill( wxColour(100,207,255), *wxBLACK ) );
    root->Append( curvegroup );

    a2dFunctionCurve* curve = new  a2dFunctionCurve(  _T( "y=140*(sin(x/10)/(x/10))" ) );
    curve->SetStroke( wxColour( 0, 27, 225 ), 0 );
    curve->SetBoundaries( -100, -100, 250, 400 );
    curve->SetXstep( 2 );
    curvegroup->AddCurve( curve );

    a2dFunctionCurve* curve2 = new  a2dFunctionCurve(  _T( "y=140*cos(x/20) + 80*cos(x*2/20) + 40*cos(x*4/20)" ) );
    curve2->SetStroke( wxColour( 227, 2, 25 ), 1 );
    curve2->SetBoundaries( -100, -100, 350, 400 );
    curve2->SetXstep( 2 );
    curvegroup->AddCurve( curve2 );

    a2dCircle* bb = new a2dCircle( 100, 140, 15 );
    bb->SetFill( wxColour( 0, 0, 5 ) );
    bb->SetStroke( wxColour( 0, 217, 225 ), 2.0 );
    curvegroup->Prepend( bb );

    a2dText* tt = new a2dText(  _T( "Two curves \n On the same plot" ), -50, -240,
                                a2dFont( 20.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( *wxBLACK );
    curvegroup->Append( tt );

    a2dText* ttv = new a2dText(  _T( "Vertical axis" ), -170, 0,
                                 a2dFont( 20.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ), 90.0 );
    ttv->SetFill( wxColour( 0, 10, 200 ) );
    ttv->SetStroke( wxColour( 0, 117, 125 ), 0 );
    curvegroup->Append( ttv );

    a2dVertexArray* pointlist = new a2dVertexArray();
    a2dLineSegment* point = new a2dLineSegment( -200, -50 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -175, 50 );
    pointlist->push_back( point );
    point = new a2dLineSegment( -50, 100 );
    pointlist->push_back( point );
    point = new a2dLineSegment( 0, 30 );
    pointlist->push_back( point );
    point = new a2dLineSegment( 20, 88 );
    pointlist->push_back( point );
    point = new a2dLineSegment( 100, 300 );
    pointlist->push_back( point );
    point = new a2dLineSegment( 200, 50 );
    pointlist->push_back( point );

    a2dVertexCurve* vcurve = new a2dVertexCurve( pointlist );
    vcurve->SetBoundaries( -100, -100, 200, 400 );
    curvegroup->AddCurve( vcurve );

    a2dMarker* mark1 = new a2dMarker(  _T( "%3.0f=%3.0f" ), curve, 20 );
    mark1->SetFill( wxColour( 0, 230, 235 ) );
    mark1->SetStroke( wxColour( 227, 2, 25 ), 0 );
    mark1->Set( 20, 10, 10 );
    curvegroup->AddMarker( mark1 );

    a2dMarker* mark2 = new a2dMarker(  _T( "%3.0f=%3.0f" ), curve2, 210, 20 );
    curvegroup->AddMarker( mark2 );

    a2dMarker* mark3 = new a2dMarker(  _T( "%3.0f=%3.0f" ), vcurve, 120, 40 );
    curvegroup->AddMarker( mark3 );


    //mouse test marker
    a2dFunctionCurve* curve4 = new  a2dFunctionCurve(  _T( "y=50*(sin(x/10)/(x/10))" ) );
    curve4->SetStroke( wxColour( 0, 27, 225 ), 0 );
    curve4->SetBoundaries( -100, -100, 250, 400 );
    curve4->SetXstep( 1 );
    curvegroup->AddCurve( curve4 );

    a2dMarker* mark4 = new a2dMarker(  _T( " x%3.0f->y%3.0f" ), curve4, 20, 20 );
    curvegroup->AddMarker( mark4 );
#else
    a2dText* cttt = new a2dText(  _T( "curve module is needed\n" ), -210, 270,
                                  a2dFont( 50.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    cttt->SetFill( wxColour( 0, 10, 200 ) );
    cttt->SetStroke( wxColour( 0, 255, 0 ) );
    root->Append( cttt );
#endif //wxART2D_USE_CURVES

    a2dText* ttt = new a2dText(  _T( "Hello from wxWorldCanvas use\n Ctrl LeftDown to drag \n 'z' key or popup to zoom" ), -100, 270,
                                 a2dFont( 50.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    ttt->SetFill( wxColour( 0, 10, 200 ) );
    ttt->SetStroke( wxColour( 0, 255, 0 ) );

    root->Append( ttt );

    //next three lines not strictly needed (depends on how you fill the a2dCanvasDocument)
    //root->SetCanvasDocumentRecursive();
    m_canvas->Update();
}


MyFrame::~MyFrame()
{
    m_canvas->GetDrawing()->Modify( false );
}

void MyFrame::CreateMyMenuBar()
{
    wxMenu* file_menu = new wxMenu;
    file_menu->Append( ID_QUIT, _( "Quit..." ), _( "Quit program" ) );

    wxMenuBar* menu_bar = new wxMenuBar();
    menu_bar->Append( file_menu, _( "File" ) );

    SetMenuBar( menu_bar );
}

void MyFrame::OnQuit( wxCommandEvent& event )
{
    Close( true );
}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    Destroy();
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

    m_frame = new MyFrame( NULL, -1, _T( "SuperApp" ), wxPoint( 20, 30 ), wxSize( 700, 540 ) );
    m_frame->Show( true );

    return true;
}

int MyApp::OnExit()
{
    return 0;
}




