/*! \file canvas/src/cansim.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: cansim.cpp,v 1.34 2008/08/01 18:32:29 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/cansim.h"
#include "wx/canvas/tools.h"
#include "wx/artbase/dcdrawer.h"

#include <wx/wfstream.h>


//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

#define WIDTHSCROLLBAR 20

//----------------------------------------------------------------------------
// a2dCanvas
//----------------------------------------------------------------------------

//! drawer is actually m_view of base class, only specialized

IMPLEMENT_CLASS( a2dCanvasSim, wxScrolledWindow )

BEGIN_EVENT_TABLE( a2dCanvasSim, wxScrolledWindow )
    EVT_PAINT( a2dCanvasSim::OnPaint )
    EVT_ERASE_BACKGROUND( a2dCanvasSim::OnEraseBackground )
    EVT_SIZE( a2dCanvasSim::OnSize )
END_EVENT_TABLE()


a2dCanvasSim::a2dCanvasSim( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ):
    wxScrolledWindow( parent, id, pos, size, style )
{
    m_border = 0;
    m_delta = 100;

    int dvx = size.GetWidth();
    int dvy = size.GetHeight();

    if ( size == wxDefaultSize )
    {
        dvx = 1000;
        dvy = 1000;
    }
    else
        GetClientSize( &dvx, &dvy ); //this will be visible width and height in  world coordinates

	m_drawing = new a2dDrawing();

    m_drawingPart = new a2dDrawingPart( dvx, dvy );
    m_drawingPart->SetDisplayWindow( this );
    a2dCanvasGlobals->SetActiveDrawingPart( m_drawingPart );
	m_drawingPart->SetShowObject( m_drawing->GetRootObject() );
}

a2dDrawing* a2dCanvasSim::GetDrawing() const
{ 
	if ( m_drawingPart )
		return m_drawingPart->GetDrawing(); 
	return NULL;
}

void a2dCanvasSim::SetDrawingPart( a2dDrawingPart* drawingPart )
{
    int w, h;
    if ( m_drawingPart )
    {
        w = m_drawingPart->GetDrawer2D()->GetWidth();
        h = m_drawingPart->GetDrawer2D()->GetHeight();
    }
    else
    {
        GetClientSize( &w, &h );
    }

	m_drawingPart = drawingPart;

    if ( m_drawingPart )
    {
        m_drawingPart->SetBufferSize( w, h );
        m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
        Enable();
    }
}

void a2dCanvasSim::ClearBackground()
{
	SetBackgroundColour( m_drawingPart->GetBackgroundFill().GetColour() );

    wxWindow::ClearBackground();
}

void a2dCanvasSim::Refresh( bool eraseBackground, const wxRect* rect )
{
    if ( m_drawingPart && !rect )
    {
        m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
    }
    wxWindow::Refresh( eraseBackground, rect );
}

a2dCanvasSim::~a2dCanvasSim()
{
	m_drawingPart = NULL; 
	m_drawing = NULL;
}

a2dCanvasObject* a2dCanvasSim::SetShowObject( const wxString& name )
{
    if ( !m_drawingPart )
        return NULL;
    return m_drawingPart->SetShowObject( name );
}

bool a2dCanvasSim::SetShowObject( a2dCanvasObject* obj )
{
    if ( !m_drawingPart )
        return false;
    return m_drawingPart->SetShowObject( obj );
}

void a2dCanvasSim::SetBackgroundFill( const a2dFill& backgroundfill )
{
    if ( !m_drawingPart )
        return;
    m_drawingPart->SetBackgroundFill( backgroundfill );
    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
}

bool a2dCanvasSim::GetYaxis() const
{
    if ( !m_drawingPart )
        return true;
    return GetDrawer2D()->GetYaxis();
}

void a2dCanvasSim::OnPaint( wxPaintEvent& WXUNUSED( event ) )
{
    wxPaintDC dc( this ); //needed to prevent looping

    // extra wxDC's are created, so we use CalcUnscrolledPosition() instead.
    //DoPrepareDC(dc);

    if ( !m_drawingPart )
    {
        return;
    }

    if ( m_drawingPart->IsFrozen() )
        return;

    //TODOif (!GetDrawer2D()->GetBuffer()->Ok()) return;

    //first redraw/update that were not yet updated pending areas (if available)
    //with the magic flag wxNO_FULL_REPAINT_ON_RESIZE blitting
    //also what is in the blitting list is not usefull
    //because Onpaint ignores blits outside the rectangles
    //that it finds damaged, therefore no blit flag for blitting areas.
    //This is done in idle time.

    //Mind that Onpaint events received from the m_drawingPart is done with that eventhandler of that specific view disabled, else
    // the event would directly loop back to that view.
    //So we enable the event handler here temporarely if needed, in order to get the update event handled.
    bool eventHandlerEnabledState = m_drawingPart->GetEvtHandlerEnabled();
    if ( !eventHandlerEnabledState )
        m_drawingPart->SetEvtHandlerEnabled( true );

    // now sent the update event
    //Onpaint events are always coming trough even if OnIdle events are not, for batch redraw etc. we make sure that all pending
    //areas or first redrawn in the buffer
    m_drawingPart->Update( a2dCANVIEW_UPDATE_AREAS );

    //restore state
    m_drawingPart->SetEvtHandlerEnabled( eventHandlerEnabledState );

    //ininiate the wxDc's etc. in a2dDrawingPart.
    GetDrawer2D()->BeginDraw();

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
        GetDrawer2D()->BlitBuffer( xx, yy, w, h );

        it++;
    }
    GetDrawer2D()->EndDraw();
}

void a2dCanvasSim::SetMouseEvents( bool onoff )
{
    if ( !m_drawingPart )
        return;

    m_drawingPart->SetMouseEvents( onoff );
}

void a2dCanvasSim::OnEraseBackground( wxEraseEvent& WXUNUSED( event ) )
{
}

a2dCanvasObject* a2dCanvasSim::IsHitWorld(
    double x, double y,
    int layer,
    a2dHitOption option
)
{
    if ( !m_drawingPart )
        return NULL;
    return m_drawingPart->IsHitWorld( x, y, layer, option );
}

bool a2dCanvasSim::WriteSVG( const wxString& filename, double Width, double Height, wxString unit )
{
    if ( !m_drawingPart || !GetDrawer2D() )
        return false;

    //to make sure
    return false;//m_drawingPart->GetCanvasDocument()->WriteSVG( m_drawingPart, filename, m_drawingPart->GetShowObject(), Width, Height, unit );
}

void a2dCanvasSim::SetMappingWidthHeight( double vx1, double vy1, double width, double height )
{
    int dxn, dyn;
    GetVirtualSize( &dxn, &dyn );

    if ( dxn == 0 ) dxn = 1000;
    if ( dyn == 0 ) dyn = 1000;

    double xupp = width / dxn;
    double yupp = height / dyn;

    if ( yupp == 0 || xupp == 0 ) //no drawing at all
    {
        yupp = 1; xupp = 1; //some value
    }

    if ( yupp > xupp )
        SetMappingUpp( vx1, vy1, yupp, yupp );
    else
        SetMappingUpp( vx1, vy1, xupp, xupp );
}

// maps the virtual window (Real drawing to the window coordinates
// also used for zooming
void a2dCanvasSim::SetMappingUpp( double vx1, double vy1, double xpp, double ypp )
{
    if ( !m_drawingPart || !GetDrawer2D() )
        return;

    int dxn, dyn;
    GetVirtualSize( &dxn, &dyn );

    if ( dxn == 0 ) dxn = 1000;
    if ( dyn == 0 ) dyn = 1000;

    GetDrawer2D()->SetMappingDeviceRect( 0, 0, dxn, dyn );
    GetDrawer2D()->SetMappingUpp( vx1, vy1, xpp, ypp );
}

void a2dCanvasSim::SetYaxis( bool up )
{
    if ( !m_drawingPart || !GetDrawer2D() )
        return;

    GetDrawer2D()->SetYaxis( up );

    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
}

bool a2dCanvasSim::GetMouseEvents()
{
    if ( !m_drawingPart || !GetDrawer2D() )
        return false;

    return m_drawingPart->GetMouseEvents();
}


void a2dCanvasSim::DoFreeze()
{
    if ( m_drawingPart )
        m_drawingPart->Freeze();
}

void a2dCanvasSim::DoThaw()
{
    if ( m_drawingPart )
        m_drawingPart->Thaw( true );
}

void a2dCanvasSim::OnSize( wxSizeEvent& WXUNUSED( event ) )
{
    if ( !m_drawingPart )
        return;

    if ( !GetDrawer2D() )
        return;

    //TRICKS!
    //The buffer is a little bigger then the clientsize.
    //This is for two reasons.
    //1- A small resize on the window does not result in redrawing
    //everything.
    //1- To be able to ignore onsize events when only scrollbars (dis)appear
    //which also would cause a redraw here

    int oldw = GetDrawer2D()->GetBuffer().GetWidth();
    int oldh = GetDrawer2D()->GetBuffer().GetHeight();

    int w, h;

    GetVirtualSize( &w, &h );

    //the client size may be with are without scrollbars
    //always draw to a buffer that is inclusif scrollbars
    w = w + WIDTHSCROLLBAR + m_delta;
    h = h + WIDTHSCROLLBAR + m_delta;

    if ( abs( oldw - w ) > m_delta || abs( oldh - h ) > m_delta )
    {
        //first redraw/update that were not yet updated pending areas (if available)
        //blit pending updates to the window/screen
        m_drawingPart->Update( a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );

        m_drawingPart->SetBufferSize( w, h );

        //only redraw/repaint what was added, so if smaller
        //do nothing else update only the new areas

        if ( GetDrawer2D()->GetYaxis() )
        {
            if ( oldw < w )
                m_drawingPart->AddPendingUpdateArea( oldw, 0, w - oldw, h );
            if ( oldh < h )
                m_drawingPart->AddPendingUpdateArea( 0, oldh, w, h - oldh );
        }
        else
        {
            if ( oldw < w )
                m_drawingPart->AddPendingUpdateArea( oldw, 0, w - oldw, h );
            if ( oldh < h )
                m_drawingPart->AddPendingUpdateArea( 0, oldh, w, h - oldh );
        }
    }
    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_BLIT );
}

void a2dCanvasSim::SetMappingShowAll( bool centre )
{
    if ( !m_drawingPart || !GetDrawer2D() )
        return;

    if ( !GetShowObject() )
        return;

    m_drawingPart->Update( a2dCANVIEW_UPDATE_OLDNEW );
    a2dBoundingBox untr = GetShowObject()->GetBbox();
    a2dAffineMatrix cworld = GetShowObject()->GetTransformMatrix();
    cworld.Invert();
    untr.MapBbox( cworld );

    double w = untr.GetWidth();
    double h = untr.GetHeight();

    if ( w == 0 )
        w = 1000;
    if ( h == 0 )
        h = 1000;

    double uppx, uppy;
    int clientw, clienth;
    GetVirtualSize( &clientw, &clienth );

    //what is needed in units per pixel to make height of bbox fit.
    if ( clientw - m_border > 0 )
        uppx = w / ( clientw - m_border );
    else
        uppx = w / clientw;

    //what is needed in units per pixel to make width of bbox fit.
    if ( clienth - m_border > 0 )
        uppy = h / ( clienth - m_border );
    else
        uppy = h / clienth;

    // now take the largets, to make sure it will fit either in x or y
    if ( uppy > uppx )
        uppx = uppy;

    GetDrawer2D()->StartRefreshDisplayDisable();
    GetDrawer2D()->SetMappingDeviceRect( 0, 0, clientw, clienth );

    if ( !centre )
    {
        SetMappingWidthHeight( untr.GetMinX(),
                               untr.GetMinY(),
                               w,
                               h
                             );
    }
    else
    {
        double middlexworld = untr.GetMinX() + w / 2.0;
        double middleyworld = untr.GetMinY() + h / 2.0;
        GetDrawer2D()->SetMappingUpp( middlexworld - clientw / 2.0 * uppx, middleyworld - clienth / 2.0 * uppx, uppx, uppx );
    }

    // if scrolling bars were added are removed because of the above, we do it once more.
    int clientwNew, clienthNew;
    GetVirtualSize( &clientwNew, &clienthNew );
    if ( clientw != clientwNew || clienth != clienthNew )
    {
        //what is needed in units per pixel to make height of bbox fit.
        if ( clientw - m_border > 0 )
            uppx = w / ( clientw - m_border );
        else
            uppx = w / clientw;

        //what is needed in units per pixel to make width of bbox fit.
        if ( clienth - m_border > 0 )
            uppy = h / ( clienth - m_border );
        else
            uppy = h / clienth;

        // now take the largets, to make sure it will fit either in x or y
        if ( uppy > uppx )
            uppx = uppy;

        GetDrawer2D()->SetMappingDeviceRect( 0, 0, clientwNew, clienthNew );
        if ( !centre )
        {
            SetMappingWidthHeight( untr.GetMinX(),
                                   untr.GetMinY(),
                                   w,
                                   h
                                 );
        }
        else
        {
            double middlexworld = untr.GetMinX() + w / 2.0;
            double middleyworld = untr.GetMinY() + h / 2.0;
            GetDrawer2D()->SetMappingUpp( middlexworld - clientwNew / 2.0 * uppx, middleyworld - clienthNew / 2.0 * uppx, uppx, uppx );
        }
    }

    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
    GetDrawer2D()->EndRefreshDisplayDisable();
    Refresh();
}
