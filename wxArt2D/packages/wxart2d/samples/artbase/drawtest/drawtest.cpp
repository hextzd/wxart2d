/*! \file artbase/samples/directdraw/directdraw.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: drawtest.cpp,v 1.4 2009/07/24 16:35:20 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "drawtest.h"
#include "wx/artbase/stylebase.h"

#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#else
#include "wx/artbase/dcdrawer.h"
#endif

// Include image
#include "smile.xpm"

// WDR: class implementations

BEGIN_EVENT_TABLE( wxMyCanvas, wxScrolledWindow )
    EVT_PAINT( wxMyCanvas::OnPaint )
    EVT_ERASE_BACKGROUND( wxMyCanvas::OnEraseBackground )
    EVT_SIZE( wxMyCanvas::OnSize )
END_EVENT_TABLE()

wxMyCanvas::wxMyCanvas( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ):
    wxScrolledWindow( parent, id, pos, size, style )
{
    m_backgroundfill = *a2dWHITE_FILL;

    int dvx = size.GetWidth();
    int dvy = size.GetHeight();

    if ( size == wxDefaultSize )
    {
        dvx = 1000;
        dvy = 1000;
    }
    else
        GetClientSize( &dvx, &dvy ); //this will be visible width and height in  world coordinates

#if wxART2D_USE_AGGDRAWER
    m_drawer2d = new a2dAggDrawer( dvx, dvy );
    //m_drawer2d = new a2dMemDcDrawer( dvx, dvy );
    //m_drawer2d = new a2dGDIPlusDrawer( dvx, dvy );
#else
    m_drawer2d = new a2dMemDcDrawer( dvx, dvy );
#endif

    //blit to this
    m_drawer2d->SetDisplay( this );
}

wxMyCanvas::~wxMyCanvas()
{
    delete m_drawer2d;
}

void wxMyCanvas::Clear()
{
    SetBackgroundColour( m_backgroundfill.GetColour() );

    wxWindow::ClearBackground();
}

void wxMyCanvas::OnEraseBackground( wxEraseEvent& event )
{
}

void wxMyCanvas::SetBackgroundFill( const a2dFill& backgroundfill )
{
    m_backgroundfill = backgroundfill;
}

bool wxMyCanvas::GetYaxis() const
{
    return m_drawer2d->GetYaxis();
}

void wxMyCanvas::SetYaxis( bool up )
{
    m_drawer2d->SetYaxis( up );
}

void wxMyCanvas::Update()
{
    //ininiate the wxDc's etc. in a2dCanvasView.
    m_drawer2d->BeginDraw();
    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );

    m_drawer2d->SetDrawerStroke( *a2dTRANSPARENT_STROKE );
    m_drawer2d->SetDrawerFill( m_backgroundfill );
    m_drawer2d->PushIdentityTransform();
    m_drawer2d->DrawRoundedRectangle( 0, 0, m_drawer2d->GetWidth(), m_drawer2d->GetHeight(), 0 );
    m_drawer2d->PopTransform();

    m_drawer2d->SetDrawerStroke( a2dStroke( *wxRED, 4.0 ) );
    m_drawer2d->SetDrawerFill( a2dFill( *wxBLUE ) );
    m_drawer2d->SetDrawerStroke( a2dStroke(  wxColour(4,23,233, 175), 4.0 ) );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour(124,233,123, 125) ) );

    m_drawer2d->DrawCircle( 0.0, 0.0, 25.0 );

    a2dAffineMatrix transform;
    transform.Rotate( 45.0, 0.0, 0.0 );
    transform.Translate( 100.0, 0.0 );
    m_drawer2d->SetTransform( transform );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour(33,33,238, 125) ) );
    m_drawer2d->SetDrawerStroke( a2dStroke(  wxColour(234,23,3, 175), 2.0 ) );
    m_drawer2d->DrawEllipse( 0.0, 0.0, 170.0, 180.0 );
    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );

    m_drawer2d->SetDrawerStroke( a2dStroke(  wxColour(4,253,33, 175), 10.0 ) );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour(254,23,12, 125) ) );
    m_drawer2d->DrawArc( 200.0 + 20.0, 0.0 + 20.0, 200.0 + 20.0, 0.0 - 20.0, 200.0, 0.0, false );
    m_drawer2d->DrawArc( 300.0 + 20.0, 0.0 + 20.0, 300.0 + 20.0, 0.0 - 20.0, 300.0, 0.0, true );

    transform.Identity();
    transform.Rotate( 45.0, 0.0, 0.0 );
    transform.Translate( 400.0, 0.0 );
    m_drawer2d->SetTransform( transform );
    m_drawer2d->DrawEllipticArc( 0.0, 0.0, 30.0, 60.0, 45.0, -45.0, false );
    transform.Translate( 100.0, 0.0 );
    m_drawer2d->SetTransform( transform );
    m_drawer2d->DrawEllipticArc( 0.0, 0.0, 30.0, 60.0, 45.0, -45.0, true );
    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );

    m_drawer2d->DrawRoundedRectangle( 0.0 - 25.0, 100.0 - 25.0, 50.0, 50.0, 0.0 );
    m_drawer2d->DrawRoundedRectangle( 100.0 - 25.0, 100.0 - 25.0, 50.0, 50.0, 10.0 );

    transform.Identity();
    transform.Rotate( 45.0, 0.0, 0.0 );
    transform.Translate( 200.0, 100.0 );
    m_drawer2d->SetTransform( transform );
    m_drawer2d->DrawCenterRoundedRectangle( 0.0, 0.0, 45.0, 45.0, 10.0 );
    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );

    {
        a2dVertexList points;
        for ( int i = 0; i < 5; i++ )
            points.push_back( new a2dLineSegment( 30 * sin( 0.8 * wxPI * i ), -30 * cos( 0.8 * wxPI * i ) ) );
        transform.Identity();
        transform.Translate( 300.0, 100.0 );
        m_drawer2d->SetTransform( transform );
        m_drawer2d->DrawPolygon( &points, false, wxODDEVEN_RULE );
        transform.Translate( 100.0, 0.0 );
        m_drawer2d->SetTransform( transform );
        m_drawer2d->DrawPolygon( &points, false, wxWINDING_RULE );
        m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );
    }

    {
        a2dVertexList points;
        for ( int i = 0; i < 5; i++ )
            points.push_back( new a2dLineSegment( 50 * sin( 0.8 * wxPI * i ), -50 * cos( 0.8 * wxPI * i ) ) );
        transform.Identity();
        transform.Translate( 500.0, 100.0 );
        m_drawer2d->SetTransform( transform );
        m_drawer2d->DrawPolygon( &points, true, wxODDEVEN_RULE );
        transform.Translate( -500.0, 100.0 );
        m_drawer2d->SetTransform( transform );
        m_drawer2d->DrawPolygon( &points, true, wxWINDING_RULE );
        m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );
    }

    {
        a2dVertexList points;
        for ( int i = 0; i < 6; i++ )
            points.push_back( new a2dLineSegment( 30 * sin( 0.8 * wxPI * i ), -30 * cos( 0.8 * wxPI * i ) ) );
        transform.Identity();
        transform.Translate( 100.0, 200.0 );
        m_drawer2d->SetTransform( transform );
        m_drawer2d->DrawLines( &points, false );
        m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );
    }

    {
        a2dVertexList points;
        for ( int i = 0; i < 6; i++ )
            points.push_back( new a2dLineSegment( 50 * sin( 0.8 * wxPI * i ), -50 * cos( 0.8 * wxPI * i ) ) );
        transform.Identity();
        transform.Translate( 200.0, 200.0 );
        m_drawer2d->SetTransform( transform );
        m_drawer2d->DrawLines( &points, true );
        m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );
    }

    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 10, 222, 215 ), 30.0 ) );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 100, 17, 155 ), a2dFILL_HORIZONTAL_HATCH ) );

//    m_drawer2d->DrawCircle( 200, 300 , 150 );

    m_drawer2d->SetDrawerFill( a2dFill( *wxBLUE, *wxRED, 0, 0, 0, 0, 0 ) );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 200, 17, 5 ) ) );
    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 210, 22, 215 ), 3.0 ) );

    a2dAffineMatrix mat( 100, 250, 1, 2, 60 );
    m_drawer2d->SetTransform( mat );

//    m_drawer2d->DrawRoundedRectangle( 100, 50, 200, 300, 50 );

    mat.Translate( 10, 30 );
    mat.SkewX( -40 );
    mat.Rotate( 30 );
    mat.Scale( 1.2, 0.7, 20, 50 );
    m_drawer2d->SetTransform( mat );

    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 10, 17, 255 ) ) );
//    m_drawer2d->DrawEllipse( 20, 50, 200, 50 );

    //  m_drawer2d->DrawLine( 120, 50, 200, 450 );

    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );

    //a vector path
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 120, 67, 255 ) ) );
    a2dVpath path = a2dVpath();

    path.Add( new a2dVpathSegment( 0, 0, a2dPATHSEG_MOVETO ) );
    path.Add( new a2dVpathSegment( 100, 0, a2dPATHSEG_LINETO ) );
    path.Add( new a2dVpathSegment( 200, 200, a2dPATHSEG_LINETO ) );
    path.Add( new a2dVpathSegment( 0, 100, a2dPATHSEG_LINETO ) );
    path.Add( new a2dVpathSegment( 50, 50, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED ) );

    path.Add( new a2dVpathSegment( 80, 80, a2dPATHSEG_MOVETO ) );
    path.Add( new a2dVpathSegment( 180, 80, a2dPATHSEG_LINETO ) );
    path.Add( new a2dVpathSegment( 280, 280, a2dPATHSEG_LINETO ) );
    path.Add( new a2dVpathSegment( 80, 180, a2dPATHSEG_LINETO ) );
    path.Add( new a2dVpathSegment( 130, 130, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED ) );

    //m_drawer2d->DrawVpath( &path );

    //a splined polygon

    a2dAffineMatrix mat2( 400, 250, 1, 1, 20 );
    m_drawer2d->SetTransform( mat2 );

    a2dVertexList* pointlist22 = new a2dVertexList();
    a2dLineSegment* point22 = new a2dLineSegment( -400, 500 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -500, 400 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -100, 500 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -200, 100 );
    pointlist22->push_back( point22 );
    point22 = new a2dLineSegment( -300, 375 );
    pointlist22->push_back( point22 );

    m_drawer2d->DrawPolygon( pointlist22, true );

    delete pointlist22;

    m_drawer2d->EndDraw();
}

void wxMyCanvas::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );

    //ininiate the wxDc's etc. in a2dCanvasView.
    m_drawer2d->BeginDraw();

    //get the regions to update and add to the list
    //of areas that need to be blitted
    wxRegionIterator it( GetUpdateRegion() );
    while ( it )
    {
        int x = it.GetX();
        int y = it.GetY();

        int w = it.GetWidth();
        int h = it.GetHeight();

        int xx;
        int yy;
        CalcUnscrolledPosition( x, y, &xx, &yy );

        m_drawer2d->BlitBuffer( xx, yy, w, h );

        it++;
    }
    m_drawer2d->EndDraw();
}

void wxMyCanvas::OnSize( wxSizeEvent& event )
{
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

// WDR: event table for MyFrame

BEGIN_EVENT_TABLE( MyFrame, wxFrame )
    EVT_MENU( ID_QUIT, MyFrame::OnQuit )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_TIMER( -1, MyFrame::OnTimer )
    EVT_MENU_RANGE( MenuOption_First, MenuOption_Last, MyFrame::OnOption )
END_EVENT_TABLE()

MyFrame::MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    wxFrame( parent, id, title, position, size, style )
{
    m_xScale = 2.0;
    m_yScale = 2.0;
    m_xOrigin = -100;
    m_yOrigin = -200;
    m_yAxisReversed = false;

    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( _T( "Welcome to a2dCanvas sample!" ) );

    m_canvas = new wxMyCanvas( this );
    m_canvas->SetVirtualSize( size );
    m_canvas->SetScrollRate( 50, 50 );

    //Start filling document now.

    // Bunch of rects and images.
    wxBitmap bitmap( smile_xpm );
    wxImage image( bitmap.ConvertToImage() );


    a2dFill backgr = a2dFill( wxColour( 0, 245, 245 ) );
    m_canvas->SetBackgroundFill( backgr );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->GetDrawer2D()->SetMappingUpp( m_xOrigin, m_yOrigin, m_xScale, m_yScale );
    m_canvas->GetDrawer2D()->SetYaxis( m_yAxisReversed );

    //draw on it
    m_canvas->Update();
}

MyFrame::~MyFrame()
{
}

void MyFrame::CreateMyMenuBar()
{
    wxMenu* file_menu = new wxMenu;
    file_menu->Append( ID_QUIT, _T( "Quit..." ), _T( "Quit program" ) );

    wxMenu* menuMap = new wxMenu;
    menuMap->Append( Scale_StretchHoriz, _T( "Stretch &horizontally\tCtrl-H" ) );
    menuMap->Append( Scale_ShrinkHoriz, _T( "Shrin&k horizontally\tCtrl-G" ) );
    menuMap->Append( Scale_StretchVertic, _T( "Stretch &vertically\tCtrl-V" ) );
    menuMap->Append( Scale_ShrinkVertic, _T( "&Shrink vertically\tCtrl-W" ) );
    menuMap->AppendSeparator();
    menuMap->Append( Map_Restore, _T( "&Restore to normal\tCtrl-0" ) );

    wxMenu* menuAxis = new wxMenu;
    menuAxis->Append( Axis_Vertic, _T( "Mirror vertically\tCtrl-N" ), _T( "" ), true );

    wxMenu* menuLogical = new wxMenu;
    menuLogical->Append( Origin_MoveDown, _T( "Move &down\tCtrl-D" ) );
    menuLogical->Append( Origin_MoveUp, _T( "Move &up\tCtrl-U" ) );
    menuLogical->Append( Origin_MoveLeft, _T( "Move &right\tCtrl-L" ) );
    menuLogical->Append( Origin_MoveRight, _T( "Move &left\tCtrl-R" ) );
    menuLogical->AppendSeparator();

    // now append the freshly created menu to the menu bar...
    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append( file_menu, _T( "File" ) );
    menuBar->Append( menuMap, _T( "&Mapping" ) );
    menuBar->Append( menuAxis, _T( "&Axis" ) );
    menuBar->Append( menuLogical, _T( "&Origin" ) );
    SetMenuBar( menuBar );
}

void MyFrame::OnQuit( wxCommandEvent& event )
{
    Close( true );
}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    Destroy();
}

void MyFrame::OnTimer( wxTimerEvent& event )
{
    wxWakeUpIdle();
}

void MyFrame::OnOption( wxCommandEvent& event )
{
    switch ( event.GetId() )
    {
        case Origin_MoveDown:
            m_yOrigin += 10;
            break;
        case Origin_MoveUp:
            m_yOrigin -= 10;
            break;
        case Origin_MoveLeft:
            m_xOrigin += 10;
            break;
        case Origin_MoveRight:
            m_xOrigin -= 10;
            break;

        case Scale_StretchHoriz:
            m_xScale *= 1.10;
            break;
        case Scale_ShrinkHoriz:
            m_xScale /= 1.10;
            break;
        case Scale_StretchVertic:
            m_yScale *= 1.10;
            break;
        case Scale_ShrinkVertic:
            m_yScale /= 1.10;
            break;
        case Map_Restore:
            m_xScale =
                m_yScale = 1.0;
            m_xOrigin =
                m_yOrigin = 0;
            break;

        case Axis_Vertic:
            m_yAxisReversed = !m_yAxisReversed;
            break;

        default:
            // skip Refresh()
            return;
    }

    m_canvas->GetDrawer2D()->SetMappingUpp( m_xOrigin, m_yOrigin, m_xScale, m_yScale );
    m_canvas->GetDrawer2D()->SetYaxis( m_yAxisReversed );

    m_canvas->Update();
    m_canvas->Refresh();
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
    m_frame = new MyFrame( NULL, -1, _T( "Draw Test" ), wxPoint( 20, 30 ), wxSize( 600, 440 ) );
    m_frame->Show( true );

    return true;
}

int MyApp::OnExit()
{
    return 0;
}
