/*! \file artbase/samples/directdraw/directdraw.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: directdraw.cpp,v 1.6 2009/07/24 16:35:20 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "directdraw.h"
#include "wx/artbase/artmod.h"

// Include image
#include "smile.xpm"

#if with_GLCANVAS 

#ifdef __DARWIN__
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif


BEGIN_EVENT_TABLE( wxMyCanvas, wxGLCanvas )
    EVT_PAINT( wxMyCanvas::OnPaint )
    EVT_ERASE_BACKGROUND( wxMyCanvas::OnEraseBackground )
    EVT_SIZE( wxMyCanvas::OnSize )
END_EVENT_TABLE()

wxMyCanvas::wxMyCanvas( wxWindow* parent,  const int *attribList, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ):
    wxGLCanvas(parent, id, attribList, pos, size, style | wxFULL_REPAINT_ON_RESIZE| wxWANTS_CHARS)
{
    m_ready = false;
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

    // Explicitly create a new rendering context instance for this canvas.
    m_glRC = new wxGLContext(this);

#ifdef _WIN32
    SetCurrent(*m_glRC);
#endif

    m_drawer2d = new a2dOglDrawer( dvx, dvy, m_glRC );

    //blit to this
    m_drawer2d->SetDisplay( this );
}

#if wxCHECK_VERSION(3,1,0)
wxMyCanvas::wxMyCanvas( wxWindow* parent, const wxGLAttributes& dispAttrs, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ):
    wxGLCanvas(parent, dispAttrs, id, pos, size, style | wxFULL_REPAINT_ON_RESIZE| wxWANTS_CHARS)
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


    // Explicitly create a new rendering context instance for this canvas.
    m_glRC = new wxGLContext(this);

    m_drawer2d = new a2dOglDrawer( dvx, dvy, m_glRC );
    //blit to this
    m_drawer2d->SetDisplay( this );
}
#endif

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
    int w, h;
    GetClientSize(&w, &h);

    // Transformations
    SetCurrent(*m_glRC);

    m_drawer2d->BeginDraw();
    m_drawer2d->SetClippingRegionDev( 100, 100, w-200, h-200 );
 
    //m_drawer2d->DestroyClippingRegion();

    //glDisable(GL_STENCIL_TEST);

    glClearColor( 0.8f, 0.8f, 0.1f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 240, 157, 155 ) ) );
    m_drawer2d->PushIdentityTransform();
    m_drawer2d->DrawRoundedRectangle( 100, 100, w-200, h-200, 0 );
    m_drawer2d->PopTransform();

    m_drawer2d->SetDrawerFill( *a2dTRANSPARENT_FILL );
    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 210, 22, 215 ), 3.0 ) );
  
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 200, 17, 5 ) ) );
    //m_drawer2d->DrawRoundedRectangle( 100, 50, 200, 300, 50 );
    double xl = GetDrawer2D()->GetVisibleMinX();
    double xr = GetDrawer2D()->GetVisibleMaxX();
    double yb = GetDrawer2D()->GetVisibleMinY();
    double yt = GetDrawer2D()->GetVisibleMaxY();

    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 140, 157, 155 ) ) );
    //m_drawer2d->DrawRoundedRectangle( xl+10, yb+10, m_drawer2d->GetVisibleWidth()-30, m_drawer2d->GetVisibleHeight()-30, 0 );


    m_drawer2d->SetDrawerStroke( *a2dTRANSPARENT_STROKE );
    m_drawer2d->SetDrawerFill( m_backgroundfill );

    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );
    m_drawer2d->PushIdentityTransform();
    //m_drawer2d->DrawRoundedRectangle( 10, 10, m_drawer2d->GetWidth()-150, m_drawer2d->GetHeight()-150, 0 );
    m_drawer2d->PopTransform();


    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 10, 222, 215 ), 30.0 ) );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 100, 17, 155, 33 ), a2dFILL_HORIZONTAL_HATCH ) );

    m_drawer2d->DrawCircle( 200, 300 , 150 );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 200, 170, 5, 22 ) ) );
    m_drawer2d->DrawCircle( 100, 300 , 125 );

    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 200, 17, 5, 22 ) ) );
    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 210, 22, 215 ), 3.0 ) );
    m_drawer2d->GetDrawerFill().SetAlpha( 50 );

    a2dAffineMatrix mat( 100, 250, 1, 2, 60 );
    m_drawer2d->SetTransform( mat );

    m_drawer2d->DrawRoundedRectangle( 100, 50, 200, 300, 50 );

    m_drawer2d->SetDrawerFill( a2dFill( *wxBLUE, *wxRED, 0, 0, 0, 0, 0 ) );
    m_drawer2d->DrawRoundedRectangle( 300, 50, 200, 300, 50 );

    m_drawer2d->SetDrawerFill( a2dFill( *wxBLUE, *wxRED, 0, 0, 0, 0 ) );
    m_drawer2d->DrawRoundedRectangle( 500, 50, 200, 300, 50 );

    mat.Translate( 10, 30 );
    mat.SkewX( -40 );
    mat.Rotate( 30 );
    mat.Scale( 1.2, 0.7, 20, 50 );
    m_drawer2d->SetTransform( mat );

    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 10, 17, 255,30 ) ) );
    m_drawer2d->DrawEllipse( 20, 50, 200, 50 );

    m_drawer2d->DrawLine( 120, 50, 200, 450 );

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

    m_drawer2d->DrawPolygon( pointlist22, true );

    delete pointlist22;

    m_drawer2d->PushIdentityTransform();
    m_drawer2d->SetDrawerFill( *a2dTRANSPARENT_FILL );
    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 210, 22, 215 ), 3.0 ) );

    a2dFont fnt = a2dFont( wxT( "LiberationSerif-Italic.ttf" ), 15.0 );
    int i = 120;
    h = 13;

    wxString txt;// = "UÉíáouéUÉíáouéUÉíáouéUÉíáouéUÉíáouéUÉíáouéUÉíáouéUÉíáouéUÉíáoué";
    txt = "Freetype is very nice, still complicated to get it right";
    txt = wxT("\u00C9");
    txt = wxT("\u00C9\u0170\u00ED\u00E1\u0151\u0171\u00E9");
    txt = wxT("jy\u0170\u00C9\u00C9\u00C9\u00ED\u00E1\u0151\u0171\u00E9pjy");

    fnt.SetSize( h );
    m_drawer2d->SetFont( fnt );
    m_drawer2d->DrawText( txt, 40, i );
    
    while( i < m_drawer2d->GetHeight()-25 )
    {
        fnt.SetSize( h );
        m_drawer2d->SetFont( fnt );
        m_drawer2d->DrawText( txt, 40, i );
        h = h + 1;
        i = i + h;
    }

    m_drawer2d->PopTransform();
    m_drawer2d->EndDraw();
}

void wxMyCanvas::OnPaint( wxPaintEvent& event )
{
    if (!m_ready)
        return;

    int clientw, clienth;
    GetClientSize( &clientw, &clienth );

    wxPaintDC dc( this );

    //ininiate the wxDc's etc. in a2dCanvasView.
    m_drawer2d->BeginDraw();

    Update();

    //copy the main framebuffer to FBO
    a2dOglDrawer* ogldrawer = wxDynamicCast( m_drawer2d, a2dOglDrawer );
    /*
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  
    glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_BACK );  
    glBlitFramebuffer(0, 0, m_drawer2d->GetWidth(), m_drawer2d->GetHeight(), 0, 0,  m_drawer2d->GetWidth(), m_drawer2d->GetHeight(),  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    */
    GLenum error = glGetError();    

    SwapBuffers();

    //Refresh();

    //get the regions to update and add to the list
    //of areas that need to be blitted
    //wxRegionIterator it( GetUpdateRegion() );
    //while ( it )
    //{
    //    int x = it.GetX();
    //    int y = it.GetY();

    //    int w = it.GetWidth();
    //    int h = it.GetHeight();

    //    int xx;
    //    int yy;
    //    CalcUnscrolledPosition( x, y, &xx, &yy );

    //    m_drawer2d->BlitBuffer( xx, yy, w, h );

    //    it++;
    //}
    m_drawer2d->EndDraw();
}

void wxMyCanvas::OnSize( wxSizeEvent& event )
{
    if (!m_ready)
        return;

    if ( !IsShown() )
        return;

    SetSizeOgl();
    
    Refresh();
}

void wxMyCanvas::InitGL()
{
    if (0)
    {
        //glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }    
    else 
    {   

    }      

}

void wxMyCanvas::SetSizeOgl()
{
    int clientw, clienth;
    GetClientSize( &clientw, &clienth );
    m_drawer2d->BeginDraw();
    m_drawer2d->SetBufferSize( clientw, clienth );

    m_drawer2d->SetMappingDeviceRect( 0, 0, clientw, clienth, true );
    m_drawer2d->EndDraw();
}

#else

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

    m_drawer2d = new a2dMemDcDrawer( dvx, dvy );
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

    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );
    m_drawer2d->PushIdentityTransform();
    m_drawer2d->DrawRoundedRectangle( 0, 0, m_drawer2d->GetWidth()-150, m_drawer2d->GetHeight()-15, 0 );
    m_drawer2d->PopTransform();

    m_drawer2d->SetDrawerFill( *a2dTRANSPARENT_FILL );
    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 210, 22, 215 ), 3.0 ) );

    a2dFont fnt = a2dFont( wxT( "LiberationSerif-Italic.ttf" ), 15.0 );
    int i = 20;
    double h = 13;

    wxString txt;// = "UÉíáouéUÉíáouéUÉíáouéUÉíáouéUÉíáouéUÉíáouéUÉíáouéUÉíáouéUÉíáoué";
    txt = "Freetype is very nice, still complicated to get it right";
    txt = wxT("\u00C9");
    txt = wxT("\u00C9\u0170\u00ED\u00E1\u0151\u0171\u00E9");
    txt = wxT("jy\u0170\u00C9\u00C9\u00C9\u00ED\u00E1\u0151\u0171\u00E9pjy");

    fnt.SetSize( h );
    m_drawer2d->SetFont( fnt );
    m_drawer2d->DrawText( txt, 40, i );
    
    while( i < m_drawer2d->GetHeight()-25 )
    {
        fnt.SetSize( h );
        m_drawer2d->SetFont( fnt );
        m_drawer2d->DrawText( txt, 40, i );
        h = h + 1;
        i = i + h;
    }
   
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 200, 17, 5 ) ) );
    //m_drawer2d->DrawRoundedRectangle( 100, 50, 200, 300, 50 );

    m_drawer2d->EndDraw();
}

/*
void wxMyCanvas::Update()
{
    //ininiate the wxDc's etc. in a2dCanvasView.
    m_drawer2d->BeginDraw();
    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );

    m_drawer2d->SetDrawerStroke( *a2dTRANSPARENT_STROKE );
    m_drawer2d->SetDrawerFill( m_backgroundfill );

    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );
    m_drawer2d->PushIdentityTransform();
    m_drawer2d->DrawRoundedRectangle( 0, 0, m_drawer2d->GetWidth(), m_drawer2d->GetHeight(), 0 );
    m_drawer2d->PopTransform();


    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 10, 222, 215 ), 30.0 ) );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 100, 17, 155 ), a2dFILL_HORIZONTAL_HATCH ) );

    m_drawer2d->DrawCircle( 200, 300 , 150 );

    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 200, 17, 5 ) ) );
    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 210, 22, 215 ), 3.0 ) );
    m_drawer2d->GetDrawerFill().SetAlpha( 50 );

    a2dAffineMatrix mat( 100, 250, 1, 2, 60 );
    m_drawer2d->SetTransform( mat );

    m_drawer2d->DrawRoundedRectangle( 100, 50, 200, 300, 50 );

    m_drawer2d->SetDrawerFill( a2dFill( *wxBLUE, *wxRED, 0, 0, 0, 0, 0 ) );
    m_drawer2d->DrawRoundedRectangle( 300, 50, 200, 300, 50 );

    m_drawer2d->SetDrawerFill( a2dFill( *wxBLUE, *wxRED, 0, 0, 0, 0 ) );
    m_drawer2d->DrawRoundedRectangle( 500, 50, 200, 300, 50 );

    mat.Translate( 10, 30 );
    mat.SkewX( -40 );
    mat.Rotate( 30 );
    mat.Scale( 1.2, 0.7, 20, 50 );
    m_drawer2d->SetTransform( mat );

    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 10, 17, 255 ) ) );
    m_drawer2d->DrawEllipse( 20, 50, 200, 50 );

    m_drawer2d->DrawLine( 120, 50, 200, 450 );

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

    m_drawer2d->DrawPolygon( pointlist22, true );

    delete pointlist22;

    m_drawer2d->EndDraw();
}
*/
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
    int clientw, clienth;
    GetClientSize( &clientw, &clienth );
    m_drawer2d->BeginDraw();
    m_drawer2d->SetBufferSize( clientw, clienth );

    m_drawer2d->SetMappingDeviceRect( 0, 0, clientw, clienth, true );
    m_drawer2d->EndDraw();
    Update();
}

#endif

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

// WDR: event table for MyFrame

BEGIN_EVENT_TABLE( MyFrame, wxFrame )
    EVT_MENU( ID_QUIT, MyFrame::OnQuit )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_TIMER( -1, MyFrame::OnTimer )
    EVT_MENU_RANGE( MenuOption_First, MenuOption_Last, MyFrame::OnOption )
    EVT_IDLE( MyFrame::OnIdle )
END_EVENT_TABLE()

MyFrame::MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    wxFrame( parent, id, title, position, size, style )
{
    m_xScale = 1.0;
    m_yScale = 1.0;
    m_xOrigin = 0;
    m_yOrigin = 0;
    m_yAxisReversed = false;

    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( _T( "Welcome to a2dCanvas sample!" ) );

#if with_GLCANVAS 
    #if wxCHECK_VERSION(3,1,0)
    /*
        wxGLAttributes vAttrs;
        vAttrs.PlatformDefaults().MinRGBA(8, 8, 8, 8).DoubleBuffer().Depth(32).EndList();
        vAttrs.Defaults().EndList();
        vAttrs.SetNeedsARB(true);
    */


    wxGLAttributes vAttrs;
    //Defaults should be accepted
    vAttrs.PlatformDefaults().Defaults().EndList();
    bool accepted = wxGLCanvas::IsDisplaySupported(vAttrs) ;

    if ( accepted )
    {
#if wxUSE_LOGWINDOW
        wxLogMessage("The display supports required visual attributes.");
#endif // wxUSE_LOGWINDOW
    }
    else
    {
#if wxUSE_LOGWINDOW
        wxLogMessage("First try with OpenGL default visual attributes failed.");
#endif // wxUSE_LOGWINDOW
        // Try again without sample buffers
        vAttrs.Reset();
        vAttrs.PlatformDefaults().RGBA().DoubleBuffer().Depth(32).EndList();
        //vAttrs.SetNeedsARB(true);
        accepted = wxGLCanvas::IsDisplaySupported(vAttrs) ;

        if ( !accepted )
        {
            wxMessageBox("Visual attributes for OpenGL are not accepted.\nThe app will exit now.",
                         "Error with OpenGL", wxOK | wxICON_ERROR);
        }
        else
        {
#if wxUSE_LOGWINDOW
            wxLogMessage("Second try with other visual attributes worked.");
#endif // wxUSE_LOGWINDOW
        }
    }











        m_canvas = new wxMyCanvas( this, vAttrs );
    #else
        int attribList[2];
        attribList[1] = WX_GL_DOUBLEBUFFER;
        m_canvas = new wxMyCanvas( this, NULL, -1, wxDefaultPosition, size );
    #endif
#else
    m_canvas = new wxMyCanvas( this );
#endif
    m_canvas->SetVirtualSize( size );
    //m_canvas->SetScrollRate( 50, 50 );

    //Start filling document now.

    // Bunch of rects and images.
    wxBitmap bitmap( smile_xpm );
    wxImage image( bitmap.ConvertToImage() );


    a2dFill backgr = a2dFill( wxColour( 0, 245, 245 ) );
    m_canvas->SetBackgroundFill( backgr );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->GetDrawer2D()->SetMappingUpp( m_xOrigin, m_yOrigin, m_xScale, m_yScale );
    m_canvas->GetDrawer2D()->SetYaxis( m_yAxisReversed );
    
    m_timer = new wxTimer( this );
    m_timer->Start( 1000, true );
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
    if ( !IsShown() )
        return;
    if ( !m_canvas->IsShown() )
        return;
    
    m_canvas->m_ready = true;
    m_canvas->SetSizeOgl();
    m_canvas->Update();
    m_canvas->Refresh();
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

void MyFrame::OnIdle( wxIdleEvent& event )
{
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

    m_frame = new MyFrame( NULL, -1, _T( "SuperApp" ), wxPoint( 20, 30 ), wxSize( 600, 440 ) );
    m_frame->Show( true );
    //draw on it
    m_frame->m_canvas->Show( true );

    SetTopWindow(m_frame);


    return true;
}

int MyApp::OnExit()
{
    return 0;
}


