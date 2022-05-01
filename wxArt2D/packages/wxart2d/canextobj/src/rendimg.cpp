/*! \file canextobj/src/rendimg.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: rendimg.cpp,v 1.36 2009/07/24 16:35:01 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/wfstream.h>
#include "wx/canvas/sttool.h"

#include "wx/canextobj/rendimg.h"

#include "wx/canvas/canobj.h"
#include "wx/canvas/cameleon.h"
#include "wx/editor/candoc.h"
#include "wx/canvas/drawer.h"
#include "wx/artbase/dcdrawer.h"

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// a2dRenderImage
//----------------------------------------------------------------------------

IMPLEMENT_CLASS( a2dRenderImage, a2dCanvasObject )

a2dRenderImage::a2dRenderImage( a2dCanvasObject* top, double xc, double yc, double w, double h )
{
    int iw = 100;
    int ih = 100;
    if ( w > 1 )
        iw = Round( w );
    if ( h > 1 )
        ih = Round( h );

    m_drawingPart = new a2dDrawingPart( iw, ih );
    a2dDrawer2D* drawer2d = new a2dMemDcDrawer( iw, ih );
    m_drawingPart->SetDrawer2D( drawer2d );

    m_drawingPart->SetShowObjectAndRender( top );

    m_scaleonresize = false;

    m_lworld.Translate( xc, yc );

    m_width = w;
    m_height = h;

    m_is_cached = false;
    m_b_is_cached = false;
}

a2dRenderImage::~a2dRenderImage()
{
}

a2dRenderImage::a2dRenderImage( const a2dRenderImage& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_drawingPart = new a2dDrawingPart( *( other.m_drawingPart ) );
    a2dDrawer2D* drawer2d = new a2dMemDcDrawer( *( ( a2dMemDcDrawer* ) other.m_drawingPart->GetDrawer2D()) );
    m_drawingPart->SetDrawer2D( drawer2d );

    m_scaleonresize = other.m_scaleonresize;

    m_width = other.m_width;
    m_height = other.m_height;

    m_is_cached = false;
    m_b_is_cached = false;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //it is because the CurrentSmartPointerOwner can change in new a2dDrawer2D()
    CurrentSmartPointerOwner = this;
#endif
}

a2dObject* a2dRenderImage::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dRenderImage( *this, options, refs );
}

a2dCanvasObject* a2dRenderImage::GetShowObject() const
{
    return m_drawingPart->GetShowObject();
}

a2dCanvasObject* a2dRenderImage::SetShowObject( const wxString& name )
{
    return m_drawingPart->SetShowObject( name );
}

bool a2dRenderImage::SetShowObject( a2dCanvasObject* obj )
{
    m_drawingPart->SetShowObjectAndRender( obj );
    return true;
}

void a2dRenderImage::SetWidth( double width )
{
    m_width = width;
    m_is_cached = false;
    Update( updatemask_force );
    SetPending( true );
}

void a2dRenderImage::SetHeight( double height )
{
    m_height = height;
    Update( updatemask_force );
    m_is_cached = false;
    SetPending( true );
}

bool a2dRenderImage::DoIsHitWorld( a2dIterC& WXUNUSED( ic ), a2dHitEvent& hitEvent )
{
    return true;
}

a2dBoundingBox a2dRenderImage::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    bbox.Expand( -m_width / 2, -m_height / 2 );
    bbox.Expand( +m_width / 2, +m_height / 2 );
    return bbox;
}

bool a2dRenderImage::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        m_drawingPart->GetShowObject()->Update( mode );
        m_bbox.Expand( -m_width / 2, -m_height / 2 );
        m_bbox.Expand( +m_width / 2, +m_height / 2 );
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

void a2dRenderImage::SetMappingWidthHeight( double vx1, double vy1, double width, double height )
{
    m_drawingPart->GetDrawer2D()->SetMappingWidthHeight( vx1, vy1, width, height );
    m_is_cached = false;
    SetPending( true );
}

void a2dRenderImage::SetMappingUpp( double vx1, double vy1, double xpp, double ypp )
{
    m_drawingPart->GetDrawer2D()->SetMappingDeviceRect( 0, 0, m_drawingPart->GetDrawer2D()->GetBuffer().GetWidth(), m_drawingPart->GetDrawer2D()->GetBuffer().GetHeight() );
    m_drawingPart->GetDrawer2D()->SetMappingUpp( vx1, vy1, xpp, ypp );
    m_is_cached = false;
    SetPending( true );
}

void a2dRenderImage::SetMappingAbs( a2dIterC& ic, double vx1, double vy1, double width, double height )
{
    double xs = m_drawingPart->GetDrawer2D()->GetBuffer().GetWidth() / ( double )m_cW;
    double ys = m_drawingPart->GetDrawer2D()->GetBuffer().GetHeight() / ( double )m_cH;

    double x = m_drawingPart->GetDrawer2D()->DeviceToWorldX( ic.GetDrawer2D()->WorldToDeviceXRel( vx1 - GetBboxMinX() ) * xs );
    double y;
    if ( ic.GetDrawer2D()->GetYaxis() )
        y = m_drawingPart->GetDrawer2D()->DeviceToWorldY( ic.GetDrawer2D()->WorldToDeviceYRel( vy1 - GetBboxMaxY() ) * ys );
    else
        y = m_drawingPart->GetDrawer2D()->DeviceToWorldY( ic.GetDrawer2D()->WorldToDeviceYRel( vy1 - GetBboxMinY() ) * ys );
    double w = m_drawingPart->GetDrawer2D()->DeviceToWorldXRel( ic.GetDrawer2D()->WorldToDeviceXRel( width * xs ) );
    double h = fabs( m_drawingPart->GetDrawer2D()->DeviceToWorldYRel( ic.GetDrawer2D()->WorldToDeviceYRel( height * ys ) ) );

    m_drawingPart->GetDrawer2D()->SetMappingWidthHeight( x, y, w, h );

    m_is_cached = false;
    SetPending( true );
}

void a2dRenderImage::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    if ( !m_drawingPart->GetDrawing() ) return;

    int dw, dh;

    dw = ic.GetDrawer2D()->WorldToDeviceXRel( ic.GetTransform().Get_scaleX() * m_width );
    dh = abs( ic.GetDrawer2D()->WorldToDeviceYRel( ic.GetTransform().Get_scaleY() * m_height ) );

    if ( dw == 0 || dh == 0 )
        return;

    double x;
    double y;
    ic.GetTransform().TransformPoint( 0.0, 0.0, x, y );
    x = ic.GetDrawer2D()->WorldToDeviceX( x );
    y = ic.GetDrawer2D()->WorldToDeviceY( y );

    wxImage tmp;

    if ( m_is_cached && m_cImage.Ok() && ( m_cW == dw ) && ( m_cH == dh ) )
    {
        // use cached image
        tmp = m_cImage;
    }
    else
    {
        if ( ( dw == m_drawingPart->GetDrawer2D()->GetBuffer().GetWidth() ) &&
                ( dh == m_drawingPart->GetDrawer2D()->GetBuffer().GetHeight() ) )
        {
            //when world == device coordinates (a2dCanvas) do not need cache
            tmp = m_drawingPart->GetDrawer2D()->GetBuffer().ConvertToImage();
        }
        else
        {
            //TODOif (m_scaleonresize)
            m_drawingPart->GetDrawer2D()->SetMappingDeviceRect(  0, 0, ( int ) m_width, ( int ) m_height );

            m_drawingPart->UpdateArea( 0, 0, m_drawingPart->GetDrawer2D()->GetBuffer().GetWidth(), m_drawingPart->GetDrawer2D()->GetBuffer().GetHeight() );

            m_cImage.Destroy();
            // create cached image
            m_cImage = m_drawingPart->GetDrawer2D()->GetBuffer().ConvertToImage();
            m_cImage = m_cImage.Scale( dw, dh );
            m_is_cached = true;
            m_cW = m_cImage.GetWidth();
            m_cH = m_cImage.GetHeight();
            m_b_is_cached = false;
            tmp = m_cImage;
        }
    }

    wxPoint centr( dw / 2, dh / 2 ); //this doesn't make any difference (BUG?)
    //anyway the rotation center is the middle of the image

    if ( ic.GetDrawer2D()->GetDrawStyle() != a2dWIREFRAME_INVERT ) //invert on image does not work optimal
    {
        ic.GetDrawer2D()->DrawImage( tmp, 0, 0, m_width, m_height, 255 );
    }
    else
    {
        if ( GetStroke() != *a2dTRANSPARENT_STROKE || GetFill() != *a2dTRANSPARENT_FILL )
            ic.GetDrawer2D()->DrawCenterRoundedRectangle( 0.0, 0.0, ( int ) m_width, ( int ) m_height, 0 );
    }
    if ( m_flags.m_selected )
    {
        //layer pens for select layer are set already on higher levels
        ic.GetDrawer2D()->DrawCenterRoundedRectangle( 0.0, 0.0, m_width, m_height, 0 );
    }
    else if ( GetStroke() != *a2dTRANSPARENT_STROKE ||  GetFill() != *a2dTRANSPARENT_FILL )
    {
        //feature of shape, so why not use it.
        ic.GetDrawer2D()->DrawCenterRoundedRectangle( 0.0, 0.0, m_width, m_height, 0 );

    }
}








