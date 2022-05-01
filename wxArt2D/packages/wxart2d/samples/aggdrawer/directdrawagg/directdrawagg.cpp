/*! \file aggdrawer/samples/directdrawagg/directdrawagg.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: directdrawagg.cpp,v 1.8 2009/09/10 17:04:09 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "directdrawagg.h"
#include "wx/artbase/artmod.h"
#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#endif //wxART2D_USE_AGGDRAWER
#if wxART2D_USE_GDIPLUSDRAWER
#include "wx/gdiplusdrawer/gdiplusdrawer.h"
#endif //wxART2D_USE_GDIPLUSDRAWER


#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#include "wx/aggdrawer/graphicagg.h"
#endif //wxART2D_USE_AGGDRAWER

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

    int dvx = size.GetWidth();
    int dvy = size.GetHeight();

    if ( size == wxDefaultSize )
    {
        dvx = 1000;
        dvy = 1000;
    }
    else
        GetClientSize( &dvx, &dvy ); //this will be visible width and height in  world coordinates


#if wxART2D_USE_GDIPLUSDRAWER
    m_drawer2d = new a2dGDIPlusDrawer( dvx, dvy );
#else
    m_drawer2d = new a2dMemDcDrawer( dvx, dvy );
#endif //wxART2D_USE_GDIPLUSDRAWER
    //m_drawer2d = new a2dAggDrawerRgba( dvx, dvy );

    //m_drawer2d = new a2dMemDcDrawer( dvx, dvy );

    m_drawer2d = new a2dAggDrawer( dvx, dvy );

    //blit to this
    m_drawer2d->SetDisplay( this );
}

wxMyCanvas::~wxMyCanvas()
{
    delete m_drawer2d;
}

void wxMyCanvas::SetDrawer2D( a2dDrawer2D* drawer2d )
{
    int width = m_drawer2d->GetWidth();
    int height = m_drawer2d->GetHeight();

    if ( m_drawer2d )
        delete m_drawer2d;

    m_drawer2d = drawer2d;
    if ( m_drawer2d )
    {
        m_drawer2d->SetBufferSize( width, height );
        m_drawer2d->SetDisplay( this );
        m_drawer2d->Init();
    }
}

void wxMyCanvas::ClearBackground()
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
    m_drawer2d->SetDrawerFill( *a2dTRANSPARENT_FILL );
    m_drawer2d->SetDrawerStroke( *a2dTRANSPARENT_STROKE );

    m_drawer2d->BeginDraw();

    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );
    m_drawer2d->SetDrawerFill( m_backgroundfill );

    m_drawer2d->PushIdentityTransform();
    m_drawer2d->DrawRoundedRectangle( 0, 0, m_drawer2d->GetWidth(), m_drawer2d->GetHeight(), 0 );
    m_drawer2d->PopTransform();

    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 10, 222, 215 ), 30.0, a2dSTROKE_LONG_DASH ) );
    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 10, 222, 215 ), 30.0, a2dSTROKE_DOT_DASH ) );

    //a2dAffineMatrix mat66( 100, 250, 1, 2, 60 );
    //m_drawer2d->SetTransform( mat66 );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 255, 120, 255, 100 ) ) );
    m_drawer2d->DrawRoundedRectangle( 250 - 400, 350 - 250, 800, 500, 0 );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 100, 17, 155 ), a2dFILL_HORIZONTAL_HATCH ) );
    m_drawer2d->DrawEllipse( 250, 350, 800, 500 );

    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 200, 17, 155 ), wxColour( 0, 17, 155, 100 ), a2dFILL_TWOCOL_HORIZONTAL_HATCH ) );
    m_drawer2d->DrawCircle( 1000, 300 , 250 );

    a2dFill fill = a2dFill( wxColour( 200, 17, 5 ) );
    fill.SetAlpha( 50 );
    m_drawer2d->SetDrawerFill( fill );

    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 210, 22, 215 ), 13.0, a2dSTROKE_DOT ) );

    a2dAffineMatrix mat( 100, 250, 1, 2, 60 );
    m_drawer2d->SetTransform( mat );

    m_drawer2d->DrawRoundedRectangle( 100, 50, 200, 300, 50 );

    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 210, 22, 215 ), 13.0, a2dSTROKE_DOT_DASH ) );
    m_drawer2d->SetDrawerFill( a2dFill( *wxBLUE, *wxRED, 400, 200, 400, 200, 200 ) );
    m_drawer2d->DrawRoundedRectangle( 300, 50, 200, 300, 50 );

    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 210, 22, 215 ), 13.0, a2dSTROKE_SHORT_DASH ) );
    m_drawer2d->SetDrawerFill( a2dFill( *wxBLUE, *wxRED, 600, 50, 600, 350 ) );
    m_drawer2d->DrawRoundedRectangle( 500, 50, 200, 300, 50 );


    mat.Translate( 10, 30 );
    mat.SkewX( -40 );
    mat.Rotate( 30 );
    mat.Scale( 1.2, 0.7, 20, 50 );
    m_drawer2d->SetTransform( mat );

    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 10, 17, 255 ) ) );
    m_drawer2d->DrawEllipse( 20, 50, 200, 50 );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 255, 120, 255, 200 ) ) );
    m_drawer2d->DrawEllipse( 40, 100, 400, 100 );

    m_drawer2d->DrawLine( 120, 50, 200, 450 );

    m_drawer2d->SetOpacityFactor( 255 );
    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );

    //a vector path
    a2dFill fill33 = a2dFill( wxColour( 10, 167, 159 ) );
    fill33.SetAlpha( 100 );
    m_drawer2d->SetDrawerFill( fill33 );
    m_drawer2d->SetDrawerFill( a2dFill( *wxGREEN, *wxRED, 100, 100, 50, 50, 200 ) );
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

    m_drawer2d->DrawVpath( &path );

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

    m_drawer2d->SetDrawerFill( a2dFill( *wxGREEN, *wxBLACK, -100, 60, -100, 60, 500 ) );
    m_drawer2d->DrawPolygon( pointlist22, true );
    m_drawer2d->DrawPolygon( pointlist22, false );

    a2dAffineMatrix mat42( 450, 300, 1, 1, 20 );
    m_drawer2d->SetTransform( mat42 );
    m_drawer2d->DrawLines( pointlist22, true );
    m_drawer2d->DrawLines( pointlist22, false );

    delete pointlist22;

    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );

    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 0, 255, 0 ), 30.0 ) );
    a2dFill fill2 = a2dFill( wxColour( 10, 167, 159 ) );
    fill2.SetAlpha( 100 );
    m_drawer2d->SetDrawerFill( fill2 );
    m_drawer2d->SetOpacityFactor( 100 );
    m_drawer2d->DrawLine( 620, 50, 700, 550 );

    m_drawer2d->SetDrawerStroke( a2dStroke(  wxColour( 255, 0, 0 ), 40.0 ) );
    a2dFill fill3 = a2dFill( wxColour( 210, 17, 59 ) );
    //fill3.SetAlpha( 120 );
    m_drawer2d->SetDrawerFill( fill3 );

    m_drawer2d->SetOpacityFactor( 255 );
    m_drawer2d->DrawLine( 510,  50, 1000, 450 );
    m_drawer2d->SetDrawStyle( a2dFILLED );
    m_drawer2d->DrawLine( 510,  50, 1000, 650 );
    m_drawer2d->SetDrawStyle( a2dWIREFRAME_INVERT );
    m_drawer2d->DrawLine( 510,  50, 1000, 650 );
    m_drawer2d->SetDrawStyle( a2dFILLED );

    m_drawer2d->SetOpacityFactor( 120 );
    wxBitmap gs_bmp36_mono;
    wxString fpath = a2dGlobals->GetImagePathList().FindValidPath( _T( "pat36.bmp" ) );
    wxASSERT_MSG( gs_bmp36_mono.LoadFile( fpath, wxBITMAP_TYPE_BMP ), wxT( "pat36.bmp not found" ) );
    //wxMask* mask36 = new wxMask(gs_bmp36_mono, *wxBLACK);
    // associate a monochrome mask with this bitmap
    //gs_bmp36_mono.SetMask(mask36);

    a2dFill aa = a2dFill( gs_bmp36_mono );
    aa.SetColour( wxColour( 0, 178, 216 ) );
    aa.SetColour2( *wxRED );
    //aa->SetStyle(a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT);
    m_drawer2d->SetDrawerFill( aa );
    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 210, 222, 5, 100 ), 3 ) );
    m_drawer2d->DrawEllipse( 440, 400, 400, 600 );

    m_drawer2d->EndDraw();
}

void wxMyCanvas::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );

    //ininiate the wxDc's etc. in a2dCanvasView.
    m_drawer2d->SetDrawerFill( *a2dTRANSPARENT_FILL );
    m_drawer2d->SetDrawerStroke( *a2dTRANSPARENT_STROKE );
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
    //m_drawer2d->SetBufferSize( event.GetSize().GetWidth(),  event.GetSize().GetHeight() );
    //Update();
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

    wxMenu* drawEng = new wxMenu;
    drawEng->Append( Switch_Drawer_aggRgba, _T( "Switch Drawer &Agg AntiAlias Rgba" ) );
    drawEng->Append( Switch_Drawer_agg, _T( "Switch Drawer &Agg AntiAlias" ) );
    drawEng->Append( Switch_Drawer_agggc, _T( "Switch Drawer Graphics &Context &Agg Gc" ) );

    drawEng->Append( Switch_Drawer_dc, _T( "Switch Drawer &wxDC" ) );
    drawEng->Append( Switch_Drawer_gdiplus, _T( "Switch Drawer &GdiPlus" ) );
    drawEng->Append( Switch_Drawer_gdigc, _T( "Switch Drawer Graphics &Context GDI" ) );
    drawEng->Append( Switch_Drawer_dcgc, _T( "Switch Drawer Graphics Context &wxDC" ) );


    wxMenu* menuLogical = new wxMenu;
    menuLogical->Append( Origin_MoveDown, _T( "Move &down\tCtrl-D" ) );
    menuLogical->Append( Origin_MoveUp, _T( "Move &up\tCtrl-U" ) );
    menuLogical->Append( Origin_MoveLeft, _T( "Move &right\tCtrl-L" ) );
    menuLogical->Append( Origin_MoveRight, _T( "Move &left\tCtrl-R" ) );

    // now append the freshly created menu to the menu bar...
    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append( file_menu, _T( "File" ) );
    menuBar->Append( menuMap, _T( "&Mapping" ) );
    menuBar->Append( menuAxis, _T( "&Axis" ) );
    menuBar->Append( drawEng, _T( "&Drawer Type" ) );
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
    int width = m_canvas->GetDrawer2D()->GetWidth();
    int height = m_canvas->GetDrawer2D()->GetHeight();

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
#if wxART2D_USE_AGGDRAWER
        case Switch_Drawer_aggRgba:
        {
            a2dAggDrawerRgba* drawer = new a2dAggDrawerRgba( width, height );
            drawer->SetPreStroke( true );
            m_canvas->SetDrawer2D( drawer );
        }
        break;
        case Switch_Drawer_agg:
        {
            m_canvas->SetDrawer2D( new a2dAggDrawer( width, height ) );
        }
        break;
#if wxART2D_USE_GRAPHICS_CONTEXT
        case Switch_Drawer_agggc:
        {
            m_canvas->SetDrawer2D( new a2dGcAggDrawer( width, height ) );
        }
        break;
#endif // wxART2D_USE_GRAPHICS_CONTEXT

#endif //wxART2D_USE_AGGDRAWER
        case Switch_Drawer_dc:
            m_canvas->SetDrawer2D( new a2dMemDcDrawer( width, height ) );
            break;

#if wxART2D_USE_GDIPLUSDRAWER
        case Switch_Drawer_gdiplus:
        {
            m_canvas->SetDrawer2D( new a2dGDIPlusDrawer( width, height ) );
        }
        break;
#endif //wxART2D_USE_GDIPLUSDRAWER

#if wxART2D_USE_GRAPHICS_CONTEXT
        case Switch_Drawer_dcgc:
        {
            m_canvas->SetDrawer2D( new a2dGcDrawer( width, height ) );
        }
        break;
        case Switch_Drawer_gdigc:
        {
            m_canvas->SetDrawer2D( new a2dNativeGcDrawer( width, height ) );
        }
        break;
#endif // wxART2D_USE_GRAPHICS_CONTEXT
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

// great way in VC to find reproduceble leaks, using first number (allocation number ) in leak report
    //_CrtSetBreakAlloc(1784);

    //_CrtSetBreakAlloc(1744);


}

bool MyApp::OnInit()
{
    wxInitAllImageHandlers();

    m_frame = new MyFrame( NULL, -1, _T( "SuperApp" ), wxPoint( 20, 30 ), wxSize( 600, 440 ) );
    m_frame->Show( true );

    return true;
}

int MyApp::OnExit()
{
    return 0;
}


