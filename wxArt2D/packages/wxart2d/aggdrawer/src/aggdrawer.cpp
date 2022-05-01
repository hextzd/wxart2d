/*! \file aggdrawer/src/aggdrawer.cpp
    \brief a2dAggDrawerRgba - Drawer using the antigrain library
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: aggdrawer.cpp,v 1.107 2009/10/01 19:22:34 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/wfstream.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/strconv.h"
#include <wx/tokenzr.h>

#include <stdio.h>
#include <string.h>
#include <math.h>


#if wxART2D_USE_AGGDRAWER

//AGG_RGB24
//#include "pixel_formats.h"

/*
//! the pixel format used for MSW a2dAggDrawer
#define pix_format agg::pix_format_rgb24
//! the pixel format used for MSW a2dAggDrawer
typedef agg::pixfmt_rgb24 pixfmt;
//! the pixel format used for MSW a2dAggDrawer
typedef agg::pixfmt_rgb24_pre pixfmt_pre;

//! the pixel format used for MSW a2dAggDrawer
#define pixfmt_gamma agg::pixfmt_rgb24_gamma
//! the pixel format used for MSW a2dAggDrawer
typedef agg::order_rgb component_order;
*/

#include "agg_basics.h"

#include "wx/artbase/stylebase.h"
#include "wx/aggdrawer/aggdrawer.h"

#else
#error This source file should not be compiled !
#endif


a2dAggDrawerBase::a2dAggDrawerBase( int width, int height )
    :
    m_rasterizer(),
    m_path(),
    m_fillGradient(),
    m_lineGradient(),
    m_fillGradientMatrix(),
    m_lineGradientMatrix(),
    m_fillGradientD1( 0.0 ),
    m_lineGradientD1( 0.0 ),
    m_fillGradientD2( 100.0 ),
    m_lineGradientD2( 100.0 ),
    m_fillGradientInterpolator( m_fillGradientMatrix ),
    m_lineGradientInterpolator( m_lineGradientMatrix ),
    m_linearGradientFunction(),
    m_radialGradientFunction(),
    a2dDrawer2D( width, height )
{
    m_ownsBuffer = true;
    m_preStroke = true;
}

a2dAggDrawerBase::a2dAggDrawerBase( const wxSize& size )
    :
    m_rasterizer(),
    m_path(),
    m_fillGradient(),
    m_lineGradient(),
    m_fillGradientMatrix(),
    m_lineGradientMatrix(),
    m_fillGradientD1( 0.0 ),
    m_lineGradientD1( 0.0 ),
    m_fillGradientD2( 100.0 ),
    m_lineGradientD2( 100.0 ),
    m_fillGradientInterpolator( m_fillGradientMatrix ),
    m_lineGradientInterpolator( m_lineGradientMatrix ),
    m_linearGradientFunction(),
    m_radialGradientFunction(),
    a2dDrawer2D( size )
{
    m_ownsBuffer = true;
    m_preStroke = true;
}

a2dAggDrawerBase::a2dAggDrawerBase( const wxBitmap& bitmap )
    :
    m_rasterizer(),
    m_path(),
    m_fillGradient(),
    m_lineGradient(),
    m_fillGradientMatrix(),
    m_lineGradientMatrix(),
    m_fillGradientD1( 0.0 ),
    m_lineGradientD1( 0.0 ),
    m_fillGradientD2( 100.0 ),
    m_lineGradientD2( 100.0 ),
    m_fillGradientInterpolator( m_fillGradientMatrix ),
    m_lineGradientInterpolator( m_lineGradientMatrix ),
    m_linearGradientFunction(),
    m_radialGradientFunction(),
    a2dDrawer2D( bitmap.GetWidth(), bitmap.GetHeight() )
{
    m_ownsBuffer = true;
    m_preStroke = true;
}

a2dAggDrawerBase::a2dAggDrawerBase( const a2dAggDrawerBase& other )
    :
    m_rasterizer(),
    m_path(),
    m_fillGradient(),
    m_lineGradient(),
    m_fillGradientMatrix(),
    m_lineGradientMatrix(),
    m_fillGradientD1( 0.0 ),
    m_lineGradientD1( 0.0 ),
    m_fillGradientD2( 100.0 ),
    m_lineGradientD2( 100.0 ),
    m_fillGradientInterpolator( m_fillGradientMatrix ),
    m_lineGradientInterpolator( m_lineGradientMatrix ),
    m_linearGradientFunction(),
    m_radialGradientFunction(),
    a2dDrawer2D( other )
{
    m_ownsBuffer = false;
    m_preStroke = other.m_preStroke;
}

void a2dAggDrawerBase::ResetStyle()
{
    SetDrawerFill( *a2dBLACK_FILL ); //set to a sure state
    SetDrawerStroke( *a2dBLACK_STROKE );
    SetDrawStyle( m_drawstyle );
}

agg::trans_affine a2dAggDrawerBase::_get_agg_user_to_device_transform() const
{
    const a2dAffineMatrix& rmtx = GetUserToDeviceTransform();
    return agg::trans_affine(
               rmtx( 0, 0 ), rmtx( 0, 1 ),
               rmtx( 1, 0 ), rmtx( 1, 1 ),
               rmtx( 2, 0 ), rmtx( 2, 1 ) );
}

void a2dAggDrawerBase::BlitBuffer( wxRect rect, const wxPoint& bufferpos )
{
    BeginDraw();
    BlitBuffer( m_deviceDC, rect, bufferpos );
    EndDraw();
}

void a2dAggDrawerBase::BeginDraw()
{
    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_display && m_beginDraw_endDraw == 0 )
    {
        m_deviceDC = new wxClientDC( m_display );
        m_display->PrepareDC( *m_deviceDC );
        DestroyClippingRegion();
        SetDrawStyle( m_drawstyle );
        SetActiveStroke( m_currentstroke );
        SetActiveFill( m_currentfill );
        m_OpacityFactor = 255;
    }
    m_beginDraw_endDraw++;
}


void a2dAggDrawerBase::EndDraw()
{
    m_beginDraw_endDraw--;

    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_display &&  m_beginDraw_endDraw == 0 )
    {
        delete m_deviceDC;
        m_deviceDC = NULL;
    }

}

void a2dAggDrawerBase::DoSetDrawStyle( a2dDrawStyle drawstyle )
{
    m_drawstyle = drawstyle;

    switch( drawstyle )
    {
        case a2dWIREFRAME_INVERT:
            //SetActiveStroke( *a2dBLACK_STROKE );
            //SetActiveFill( *a2dTRANSPARENT_FILL );
            m_blendMode = agg::comp_op_invert;
            break;

        case a2dWIREFRAME:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
            m_blendMode = agg::end_of_comp_op_e;
            break;

        case a2dWIREFRAME_ZERO_WIDTH:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
            m_blendMode = agg::end_of_comp_op_e;
            break;

        case a2dWIREFRAME_INVERT_ZERO_WIDTH:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
            m_blendMode = agg::comp_op_invert;
            break;

        case a2dFILLED:
            SetActiveStroke( m_currentstroke );
            SetActiveFill( m_currentfill );
            m_blendMode = agg::end_of_comp_op_e;
            break;

        case a2dFIX_STYLE:
            //preserve this
            m_fixStrokeRestore = m_currentstroke;
            m_fixFillRestore = m_currentfill;
            m_fixDrawstyle = m_drawstyle;
            SetActiveStroke( m_currentstroke );
            SetActiveFill( m_currentfill );

            m_blendMode = agg::end_of_comp_op_e;
            break;

        case a2dFIX_STYLE_INVERT:
            // don't adjust style
            m_blendMode = agg::comp_op_invert;
            break;

        default:
            wxASSERT( 0 );
    }
}

void a2dAggDrawerBase::DoSetActiveStroke()
{
    if ( m_drawstyle == a2dWIREFRAME_INVERT_ZERO_WIDTH )
    {
        m_strokewidth = 0;
        m_strokewidthDev = 1;
    }
    else if ( m_activestroke.GetType() == a2dSTROKE_ONE_COLOUR )
    {
        switch( m_activestroke.GetStyle() )
        {
            case a2dSTROKE_SOLID:
            case a2dSTROKE_TRANSPARENT:
            case a2dSTROKE_DOT:
            case a2dSTROKE_DOT_DASH:
            case a2dSTROKE_LONG_DASH:
            case a2dSTROKE_SHORT_DASH:
                m_style = m_activestroke.GetStyle();
                break;
                /*
                  // These aren't supported yet
                case a2dSTROKE_USER_DASH:
                break;
                case a2dSTROKE_BDIAGONAL_HATCH:
                break;
                case a2dSTROKE_CROSSDIAG_HATCH
                break;
                case a2dSTROKE_FDIAGONAL_HATCH:
                break;
                case a2dSTROKE_CROSS_HATCH:
                break;
                case a2dSTROKE_HORIZONTAL_HATCH:
                break;
                case a2dSTROKE_VERTICAL_HATCH:
                break;
                case a2dSTROKE_STIPPLE:
                break;
                case a2dSTROKE_STIPPLE_MASK_OPAQUE:
                break;
                */
            default: m_style=a2dSTROKE_SOLID;
        }

        m_strokewidth = m_activestroke.GetWidth();
        if ( m_drawstyle == a2dWIREFRAME_ZERO_WIDTH )
        {
            a2dAffineMatrix invert = m_worldtodevice;
            invert.Invert();
            m_strokewidthDev = 1;
            m_strokewidth = invert.TransformDistance( m_strokewidthDev );
            m_join = agg::round_join;
            m_cap = agg::round_cap;
        }
        else
        {
            switch ( m_activestroke.GetJoin() )
            {
                case wxJOIN_MITER: m_join = agg::miter_join;
                    break;
                case wxJOIN_ROUND: m_join = agg::round_join;
                    break;
                case wxJOIN_BEVEL: m_join = agg::bevel_join;
                    break;
                default: m_join = agg::round_join;
            }
            switch ( m_activestroke.GetCap() )
            {
                case wxCAP_BUTT: m_cap = agg::butt_cap;
                    break;
                case wxCAP_ROUND: m_cap = agg::round_cap;
                    break;
                case wxCAP_PROJECTING: m_cap = agg::square_cap;
                    break;
                default: m_cap = agg::round_cap;
            }

            if ( m_activestroke.GetPixelStroke() )
            {
                a2dAffineMatrix invert = m_worldtodevice;
                invert.Invert();
                m_strokewidthDev = m_activestroke.GetWidth();
                m_strokewidthDev = !m_strokewidthDev ? 1 : m_strokewidthDev;
                m_strokewidth = invert.TransformDistance( m_strokewidthDev );
            }
            else
            {
                m_strokewidth = m_activestroke.GetWidth();
                m_strokewidthDev = m_worldtodevice.TransformDistance( m_strokewidth );
                if ( !m_strokewidthDev )
                {
                    m_strokewidthDev = 1;
                    a2dAffineMatrix invert = m_worldtodevice;
                    invert.Invert();
                    m_strokewidth = invert.TransformDistance( m_strokewidthDev );
                }
            }
        }
    }
}


void a2dAggDrawerBase::DoSetActiveFill()
{
    if ( m_activefill.GetType() == a2dFILL_ONE_COLOUR )
    {
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_TRANSPARENT:
                break;
            case a2dFILL_SOLID:
                break;
            case a2dFILL_BDIAGONAL_HATCH:
                m_pattern = wxImage( BDIAGONAL_HATCH_XPM );
                break;
            case a2dFILL_CROSSDIAG_HATCH:
                m_pattern = wxImage( CROSSDIAG_HATCH_XPM );
                break;
            case a2dFILL_FDIAGONAL_HATCH:
                m_pattern = wxImage( FDIAGONAL_HATCH_XPM );
                break;
            case a2dFILL_CROSS_HATCH:
                m_pattern = wxImage( CROSS_HATCH_XPM );
                break;
            case a2dFILL_HORIZONTAL_HATCH:
                m_pattern = wxImage( HORIZONTAL_HATCH_XPM );
                break;
            case a2dFILL_VERTICAL_HATCH:
                m_pattern = wxImage( VERTICAL_HATCH_XPM );
                break;
            default: ;
        }
        if ( m_activefill.GetStyle() >= a2dFIRST_HATCH && m_activefill.GetStyle() <= a2dLAST_HATCH )
        {
            m_pattern.Replace( 0, 0, 0,
                               m_colour1redFill,
                               m_colour1greenFill,
                               m_colour1blueFill );

            m_pattern_rbuf.attach( m_pattern.GetData(), m_pattern.GetWidth(), m_pattern.GetHeight(), m_pattern.GetWidth() * 3 );
        }
    }
    else if ( m_activefill.GetType() == a2dFILL_HATCH_TWO_COLOUR )
    {
        bool nohatch = false;
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_TWOCOL_BDIAGONAL_HATCH:
                m_pattern = wxImage( BDIAGONAL_HATCH_XPM );
                break;
            case a2dFILL_TWOCOL_CROSSDIAG_HATCH:
                m_pattern = wxImage( CROSSDIAG_HATCH_XPM );
                break;
            case a2dFILL_TWOCOL_FDIAGONAL_HATCH:
                m_pattern = wxImage( FDIAGONAL_HATCH_XPM );
                break;
            case a2dFILL_TWOCOL_CROSS_HATCH:
                m_pattern = wxImage( CROSS_HATCH_XPM );
                break;
            case a2dFILL_TWOCOL_HORIZONTAL_HATCH:
                m_pattern = wxImage( HORIZONTAL_HATCH_XPM );
                break;
            case a2dFILL_TWOCOL_VERTICAL_HATCH:
                m_pattern = wxImage( VERTICAL_HATCH_XPM );
                break;
            default:
                nohatch = true;
                break;
        }
        m_pattern.Replace( 255, 255, 255,
                           m_colour2redFill,
                           m_colour2greenFill,
                           m_colour2blueFill );

        m_pattern.Replace( 0, 0, 0,
                           m_colour1redFill,
                           m_colour1greenFill,
                           m_colour1blueFill );

        m_pattern_rbuf.attach( m_pattern.GetData(), m_pattern.GetWidth(), m_pattern.GetHeight(), m_pattern.GetWidth() * 3 );
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP )
    {
        wxBrush dcbrush;
        //wxBrushStyle style;
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_STIPPLE:
            case a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT:
            case a2dFILL_STIPPLE_MASK_OPAQUE:
            {
                static const int MASK_RED = 1;
                static const int MASK_GREEN = 2;
                static const int MASK_BLUE = 3;

                m_pattern = m_activefill.GetStipple().ConvertToImage();
                m_pattern.Replace( MASK_RED, MASK_GREEN, MASK_BLUE,
                                   m_colour1redFill,
                                   m_colour1greenFill,
                                   m_colour1blueFill );

                m_pattern_rbuf.attach( m_pattern.GetData(), m_pattern.GetWidth(), m_pattern.GetHeight(), m_pattern.GetWidth() * 3 );

                //test to see what effect it has
                //PixFormat pixf(m_pattern_rbuf);
                //agg::renderer_base<pixfmt> rb(pixf);
                //rb.clear(agg::rgba(0.4, 0.5, 0.1, 1.0)); // Pattern background color
                //rb.clear(agg::rgba8(0,250,80));
            }
            break;
            default:
                break;
        }
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR  )
    {
        double profile = 1.0;
        int i;
        double x1 = m_activefill.GetStart().m_x;
        double y1 = m_activefill.GetStart().m_y;
        double x2 = m_activefill.GetStop().m_x;
        double y2 = m_activefill.GetStop().m_y;
        color_type c1(  m_colour1redFill, m_colour1greenFill, m_colour1blueFill, m_FillOpacityCol1 );
        color_type c2(  m_colour2redFill, m_colour2greenFill, m_colour2blueFill, m_FillOpacityCol2 );
        int startGradient = 128 - int( profile * 128.0 );
        int endGradient   = 128 + int( profile * 128.0 );
        if ( endGradient <= startGradient ) endGradient = startGradient + 1;
        double k = 1.0 / double( endGradient - startGradient );
        for ( i = 0; i < startGradient; i++ )
        {
            m_fillGradient[i] = c1;
        }
        for ( ; i < endGradient; i++ )
        {
            m_fillGradient[i] = c1.gradient( c2, double( i - startGradient ) * k );
        }
        for ( ; i < 256; i++ )
        {
            m_fillGradient[i] = c2;
        }
        double angle = atan2( y2 - y1, x2 - x1 );
        m_fillGradientMatrix.reset();
        m_fillGradientMatrix *= agg::trans_affine_rotation( angle );
        m_fillGradientMatrix *= agg::trans_affine_translation( x1, y1 );
        m_fillGradientMatrix *= _get_agg_user_to_device_transform();
        m_fillGradientMatrix.invert();
        m_fillGradientD1 = 0;
        m_fillGradientD2 = sqrt( ( x2 - x1 ) * ( x2 - x1 ) + ( y2 - y1 ) * ( y2 - y1 ) );
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        double profile = 1.0;
        int i;
        double xf = m_activefill.GetFocal().m_x;
        double yf = m_activefill.GetFocal().m_y;
        double x = m_activefill.GetCenter().m_x;
        double y = m_activefill.GetCenter().m_y;
        double r = m_activefill.GetRadius();

        color_type c1(  m_colour1redFill, m_colour1greenFill, m_colour1blueFill, m_FillOpacityCol1 );
        color_type c2(  m_colour2redFill, m_colour2greenFill, m_colour2blueFill, m_FillOpacityCol2 );
        int startGradient = 128 - int( profile * 127.0 );
        int endGradient   = 128 + int( profile * 127.0 );
        if ( endGradient <= startGradient ) endGradient = startGradient + 1;
        double k = 1.0 / double( endGradient - startGradient );
        for ( i = 0; i < startGradient; i++ )
        {
            m_fillGradient[i] = c1;
        }
        for ( ; i < endGradient; i++ )
        {
            m_fillGradient[i] = c1.gradient( c2, double( i - startGradient ) * k );
        }
        for ( ; i < 256; i++ )
        {
            m_fillGradient[i] = c2;
        }
        m_fillGradientD2 = m_usertodevice.TransformDistance( r );
        m_usertodevice.TransformPoint( x, y, x, y );
        m_fillGradientMatrix.reset();
        m_fillGradientMatrix *= agg::trans_affine_translation( x, y );
        m_fillGradientMatrix.invert();
        m_fillGradientD1 = 0;
    }
}

void a2dAggDrawerBase::SetClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle )
{
    unsigned int n = points->size();
    m_cpointsDouble.resize( n );

    unsigned int i = 0;
    double x, y;
    forEachIn( a2dVertexList, points )
    {
        a2dPoint2D point = ( *iter )->GetPoint();
        //transform to device
        GetUserToDeviceTransform().TransformPoint( point.m_x, point.m_y, x, y );
        m_cpointsDouble[i].x = x;
        m_cpointsDouble[i].y = y;
        i++;
    }

    if ( spline )
        n = ConvertSplinedPolygon2( n );

    wxPoint* intCPoints = new wxPoint[n];

    for ( i = 0; i < n; i++ )
    {
        intCPoints[i].x = Round( m_cpointsDouble[i].x );
        intCPoints[i].y = Round( m_cpointsDouble[i].y );
    }

    m_clip = wxRegion( n, intCPoints, fillStyle );

    delete[] intCPoints;
}

void a2dAggDrawerBase::ExtendAndPushClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle, a2dBooleanClip clipoperation )
{

    wxRegion* push = new wxRegion( m_clip );

    m_clipregionlist.Insert( push );

    ExtendClippingRegion( points, spline, fillStyle, clipoperation );
}

void a2dAggDrawerBase::ExtendClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle, a2dBooleanClip clipoperation )
{
    wxRegion totaladd;
    bool first = false;

    unsigned int n = points->size();
    m_cpointsDouble.resize( n );

    unsigned int i = 0;
    double x, y;
    forEachIn( a2dVertexList, points )
    {
        a2dPoint2D point = ( *iter )->GetPoint();
        //transform to device
        GetUserToDeviceTransform().TransformPoint( point.m_x, point.m_y, x, y );
        m_cpointsDouble[i].x = x;
        m_cpointsDouble[i].y = y;
        i++;
    }

    if ( spline )
        n = ConvertSplinedPolygon2( n );

    wxPoint* intCPoints = new wxPoint[n];

    for ( i = 0; i < n; i++ )
    {
        intCPoints[i].x = Round( m_cpointsDouble[i].x );
        intCPoints[i].y = Round( m_cpointsDouble[i].y );
    }

    wxRegion add = wxRegion( n, intCPoints, fillStyle );

    delete[] intCPoints;

    if ( !first )
    {
        totaladd = add;
        first = true;
    }
    else
        totaladd.Union( add );

    if ( !m_clip.Empty() )
    {
        bool result;
        switch ( clipoperation )
        {
            case a2dCLIP_AND:
                result = m_clip.Intersect( totaladd );
                break ;

            case a2dCLIP_OR:
                result = m_clip.Union( totaladd );
                break ;

            case a2dCLIP_XOR:
                result = m_clip.Xor( totaladd );
                break ;

            case a2dCLIP_DIFF:
                result = m_clip.Subtract( totaladd );
                break ;

            case a2dCLIP_COPY:
            default:
                m_clip = totaladd;
                result = true;
                break ;

        }
        if ( result )
        {
            //DestroyClippingRegion();
            //SetClippingRegion( m_clip );
        }
    }
    else
    {
        m_clip = totaladd;
        //DestroyClippingRegion();
        //SetClippingRegion( m_clip );
    }

    m_clipboxdev = m_clip.GetBox();
    m_clipboxworld = ToWorld( m_clipboxdev );
}

void a2dAggDrawerBase::PopClippingRegion()
{
    if ( !m_clipregionlist.GetCount() )
        return;

    m_clip = *m_clipregionlist.GetFirst()->GetData();
    delete m_clipregionlist.GetFirst()->GetData();
    m_clipregionlist.DeleteNode( m_clipregionlist.GetFirst() );
    //DestroyClippingRegion();
    //SetClippingRegion( m_clip );

}

void a2dAggDrawerBase::SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height )
{
    m_clip = wxRegion( minx, miny, width, height );

    m_clipboxdev = wxRect( minx, miny, width, height );
    RenderSetClip();
    // why not the next, does not work??
    //m_rasterizer.clip_box( minx, miny, width, height );
    m_clipboxworld = ToWorld( m_clipboxdev );
}

void a2dAggDrawerBase::SetClippingRegion( double minx, double miny, double maxx, double maxy )
{
    int iminx = WorldToDeviceX( minx );
    int iminy = WorldToDeviceY( miny );
    int imaxx = WorldToDeviceX( maxx );
    int imaxy = WorldToDeviceY( maxy );
    if ( m_yaxis )
    {
        m_clip = wxRegion( iminx, imaxy, imaxx - iminx, iminy - imaxy );
        m_clipboxdev = wxRect( iminx, imaxy, imaxx - iminx, iminy - imaxy );
    }
    else
    {
        m_clip = wxRegion( iminx, iminy, imaxx - iminx, imaxy - iminy );
        m_clipboxdev = wxRect( iminx, iminy, imaxx - iminx, imaxy - iminy );
    }
    RenderSetClip();
    // why not the next, does not work??
    //m_rasterizer.clip_box( m_clipboxdev.x, m_clipboxdev.y, m_clipboxdev.x + m_clipboxdev.width, m_clipboxdev.y + m_clipboxdev.height);

    m_clipboxworld = ToWorld( m_clipboxdev );
}

void a2dAggDrawerBase::DestroyClippingRegion()
{
    m_clip.Clear();

    m_clipboxdev = wxRect( 0, 0, m_width, m_height );
    m_clipboxworld = ToWorld( m_clipboxdev );
    RenderSetClip();
}

int a2dAggDrawerBase::ToAggPath( a2dVertexArray* points, bool transform )
{
    unsigned int segments = 0;

    m_path.remove_all();

    unsigned int count = 0;
    double x, y, lastx, lasty;
    int i;
    for ( i = 0; i < points->size(); i++ )
    {
        const a2dLineSegmentPtr seg = points->Item( i );

        if ( !seg->GetArc() )
        {
            if ( transform )
                m_usertodevice.TransformPoint( seg->m_x, seg->m_y, x, y );
            else
            {
                x = seg->m_x;
                y = seg->m_y;
            }

            //to reduce the points count for lines or a polygon on the screen
            if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
            {
                if ( count == 0 )
                    m_path.move_to( x, y );
                else
                    m_path.line_to( x, y );
                lastx = x;
                lasty = y;
                count++;
            }
        }
        else
        {
            const a2dArcSegment* cseg = ( const a2dArcSegment* ) seg.Get();

            double radius, center_x, center_y, beginrad, midrad, endrad, phit;

            if ( cseg->CalcR( *( points->Item( i ? i - 1 : 0 ) ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
            {
                double dphi;
                unsigned int segments = 20;
                double radiusDev = m_usertodevice.TransformDistance( radius );
                Aberration( m_displayaberration, phit, radiusDev , dphi, segments );

                double theta = beginrad;
                unsigned int step;

                double x, y;
                for ( step = 0; step < segments + 1; step++ )
                {
                    if ( transform )
                        m_usertodevice.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                    else
                    {
                        x = center_x + radius * cos ( theta );
                        y = center_y + radius * sin ( theta );
                    }
                    //to reduce the points count for lines or a polygon on the screen
                    if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                    {
                        if ( count == 0 )
                            m_path.move_to( x, y );
                        else
                            m_path.line_to( x, y );

                        lastx = x;
                        lasty = y;
                        count++;
                    }
                    theta = theta + dphi;
                }
            }
            else
            {
                double x, y;
                if ( transform )
                    m_usertodevice.TransformPoint( cseg->m_x, cseg->m_y, x, y );
                else
                {
                    x = cseg->m_x;
                    y = cseg->m_y;
                }

                //to reduce the points count for lines or a polygon on the screen
                if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                {
                    if ( count == 0 )
                        m_path.move_to( x, y );
                    else
                        m_path.line_to( x, y );
                    lastx = x;
                    lasty = y;
                    count++;
                }
            }
        }
    }

    return count;
}

int a2dAggDrawerBase::ToAggPath( const a2dVertexList* list, bool transform )
{
    unsigned int segments = 0;

    if ( list->empty() )
        return 0;

    m_path.remove_all();

    a2dVertexList::const_iterator iterprev = list->end();

    unsigned int count = 0;
    double x, y, lastx, lasty;
    iterprev = list->end();
    if ( iterprev != list->begin() )
        iterprev--;
    a2dVertexList::const_iterator iter = list->begin();
    iter = list->begin();
    while ( iter != list->end() )
    {
        a2dLineSegment* seg = ( *iter );

        if ( !seg->GetArc() )
        {
            if ( transform )
                m_usertodevice.TransformPoint( seg->m_x, seg->m_y, x, y );
            else
            {
                x = seg->m_x;
                y = seg->m_y;
            }

            //to reduce the points count for lines or a polygon on the screen
            if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
            {
                if ( count == 0 )
                    m_path.move_to( x, y );
                else
                    m_path.line_to( x, y );
                lastx = x;
                lasty = y;
                count++;
            }
        }
        else
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;

            double radius, center_x, center_y, beginrad, midrad, endrad, phit;

            if ( cseg->CalcR( *( *iterprev ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
            {
                double dphi;
                unsigned int segments = 20;
                double radiusDev = m_usertodevice.TransformDistance( radius );
                Aberration( m_displayaberration, phit, radiusDev , dphi, segments );

                double theta = beginrad;
                unsigned int step;

                double x, y;
                for ( step = 0; step < segments + 1; step++ )
                {
                    if ( transform )
                        m_usertodevice.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                    else
                    {
                        x = center_x + radius * cos ( theta );
                        y = center_y + radius * sin ( theta );
                    }

                    //to reduce the points count for lines or a polygon on the screen
                    if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                    {
                        if ( count == 0 )
                            m_path.move_to( x, y );
                        else
                            m_path.line_to( x, y );
                        lastx = x;
                        lasty = y;
                        count++;
                    }
                    theta = theta + dphi;
                }
            }
            else
            {
                double x, y;
                if ( transform )
                    m_usertodevice.TransformPoint( cseg->m_x, cseg->m_y, x, y );
                else
                {
                    x = cseg->m_x;
                    y = cseg->m_y;
                }

                //to reduce the points count for lines or a polygon on the screen
                if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                {
                    if ( count == 0 )
                        m_path.move_to( x, y );
                    else
                        m_path.line_to( x, y );
                    lastx = x;
                    lasty = y;
                    count++;
                }
            }
        }
        iterprev = iter++;
    }

    return count;
}

void a2dAggDrawerBase::ShiftBufferInternal( int dxy, bool yshift, int bytesPerPixel )
{
    if ( yshift )
    {
        int pixelwidth = m_width * bytesPerPixel;
        //pixel coordinates so ( 0,0 ) upper left
        if ( dxy > 0 && dxy < m_height )
        {
            unsigned char* highline = m_pdata + ( m_height - dxy ) * pixelwidth ;
            unsigned char* lowline = m_pdata + m_height * pixelwidth ;

            for ( int yp = 0; yp < m_height - dxy; yp++ )
            {
                highline -= pixelwidth;
                lowline -= pixelwidth;
                memcpy( lowline, highline, pixelwidth );
            }
        }
        else  if ( dxy < 0 && dxy > -m_height )
        {
            dxy = -dxy;
            unsigned char* highline = m_pdata;
            unsigned char* lowline = m_pdata + dxy * pixelwidth ;

            for ( int yp = 0; yp < m_height - dxy ; yp++ )
            {
                memcpy( highline, lowline, pixelwidth );
                highline += pixelwidth;
                lowline += pixelwidth;
            }
        }
        else
            wxFAIL_MSG( wxT( "you can only shift within height of buffer" ) );
    }
    else
    {
        int pixelwidth = m_width * bytesPerPixel;
        if ( dxy > 0 && dxy < m_width )
        {
            int subwidth = ( m_width - dxy ) * bytesPerPixel;
            unsigned char* low = m_pdata;
            unsigned char* high = m_pdata + dxy * bytesPerPixel;

            for ( int yp = 0; yp < m_height  ; yp++ )
            {
                memmove( high, low, subwidth );
                high += pixelwidth;
                low += pixelwidth;
            }
        }
        else if ( dxy < 0 && dxy > -m_width )
        {
            dxy = -dxy;
            int subwidth = ( m_width - dxy ) * bytesPerPixel;
            unsigned char* low = m_pdata;
            unsigned char* high = m_pdata + dxy * bytesPerPixel ;

            for ( int yp = 0; yp < m_height  ; yp++ )
            {
                memmove( low, high, subwidth );
                high += pixelwidth;
                low += pixelwidth;
            }
        }
        else
            wxFAIL_MSG( wxT( "you can only shift within width of buffer" ) );
    }
}

template< class PathT >
void a2dAggDrawerBase::_ras_add_stroked_path_xform( PathT& path, const
        agg::trans_affine& mtx )
{
    // This does the job of constructing all stroke rendering pipelines
    // based on the current stroke parameters.
    // Transformation is always part of the pipeline, but sometimes its
    // a no-op transformer (noop_conv_transform).

    if ( m_style == a2dSTROKE_SOLID )
    {
        if ( m_preStroke )
        {
            typedef agg::conv_stroke< PathT > path_stroke_t;
            typedef agg::conv_transform< path_stroke_t > path_stroke_trans_t;

            path_stroke_t stroke( path );
            path_stroke_trans_t trans_path( stroke, mtx );
            stroke.line_join( m_join );
            stroke.line_cap( m_cap );
            if ( m_activestroke.GetPixelStroke() )
                stroke.width( m_strokewidthDev );
            else
                stroke.width( m_strokewidth );
            m_rasterizer.reset();
            m_rasterizer.add_path( trans_path );
        }
        else
        {
            typedef agg::conv_transform<PathT> trans_t;
            typedef agg::conv_stroke< trans_t > path_stroke_t;

            trans_t trans_path( path, mtx );
            path_stroke_t stroke( trans_path );
            //stroke.miter_limit();
            stroke.line_join( m_join );
            stroke.line_cap( m_cap );
            stroke.width( m_strokewidthDev );
            m_rasterizer.reset();
            m_rasterizer.add_path( stroke );
        }
    }
    else
    {
        if ( m_preStroke )
        {
            typedef agg::conv_dash< PathT > path_dash_t;
            typedef agg::conv_stroke< path_dash_t > path_dashstroked_t;
            typedef agg::conv_transform< path_dashstroked_t > path_transdash_t;

            path_dash_t dash_path( path );
            path_dashstroked_t stroke( dash_path );
            path_transdash_t trans_path( stroke, mtx );

            double scale = m_strokewidth;
            if ( scale < 1.0 ) scale = 1.0;
            //scale = 1.0;
            switch( m_style )
            {
                case a2dSTROKE_DOT:
                    dash_path.add_dash( 0.25 * scale, 1.5 * scale );
                    break;
                case a2dSTROKE_DOT_DASH:
                    dash_path.add_dash( 0.25 * scale, 1.5 * scale );
                    dash_path.add_dash( 2.0 * scale, 2.0 * scale );
                    break;
                case a2dSTROKE_LONG_DASH:
                    dash_path.add_dash( 2.0 * scale, 2.0 * scale );
                    break;
                case a2dSTROKE_SHORT_DASH:
                    dash_path.add_dash( 2.0 * scale, 2.0 * scale );
                    break;
                    //case a2dSTROKE_USER_DASH:
                    //    break;
            }
            //dash_path.dash_start(m_dash_offset);
            stroke.line_join( m_join );
            stroke.line_cap( m_cap );
            //stroke.miter_limit();
            if ( m_activestroke.GetPixelStroke() )
                stroke.width( m_strokewidth );
            else
                stroke.width( m_strokewidthDev );
            m_rasterizer.reset();
            m_rasterizer.add_path( trans_path );
        }
        else
        {
            typedef agg::conv_transform<PathT> trans_t;
            typedef agg::conv_dash< trans_t > path_transdash_t;
            typedef agg::conv_stroke< path_transdash_t > path_dashstroked_t;

            trans_t trans_path( path, mtx );
            path_transdash_t dash_path( trans_path );
            path_dashstroked_t stroke( dash_path );

            double scale = m_strokewidthDev;
            if ( scale < 1.0 ) scale = 1.0;
            //scale = 1.0;
            switch( m_style )
            {
                case a2dSTROKE_DOT:
                    dash_path.add_dash( 0.25 * scale, 1.5 * scale );
                    break;
                case a2dSTROKE_DOT_DASH:
                    dash_path.add_dash( 0.25 * scale, 1.5 * scale );
                    dash_path.add_dash( 2.0 * scale, 2.0 * scale );
                    break;
                case a2dSTROKE_LONG_DASH:
                    dash_path.add_dash( 2.0 * scale, 2.0 * scale );
                    break;
                case a2dSTROKE_SHORT_DASH:
                    dash_path.add_dash( 2.0 * scale, 2.0 * scale );
                    break;
                    //case a2dSTROKE_USER_DASH:
                    //    break;
            }
            //dash_path.dash_start(m_dash_offset);
            stroke.line_join( m_join );
            stroke.line_cap( m_cap );
            //stroke.miter_limit();
            stroke.width( m_strokewidthDev );
            m_rasterizer.reset();
            m_rasterizer.add_path( stroke );
        }

    }
}

void a2dAggDrawerBase::DrawCircle( double x, double y, double radius )
{
    if ( m_disableDrawing )
        return;

    DrawEllipse( x, y, radius * 2, radius * 2 );
}

void a2dAggDrawerBase::DrawPoint( double xc, double yc )
{
    if ( m_disableDrawing )
        return;

    m_usertodevice.TransformPoint( xc, yc, xc, yc );
    DeviceDrawPixel( ( int ) xc, ( int ) yc, m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
}

void a2dAggDrawerBase::DrawCharDc( wxChar c )
{
#if wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
    if ( m_currentfont.GetType() == a2dFONT_WXDC && m_currentfont.GetFreetypeFont().Ok() )
    {
        a2dFont oldfont = m_currentfont;
        m_currentfont = m_currentfont.GetFreetypeFont();
        DrawCharFreetype( c );
        m_currentfont = oldfont;
    }
    else
        a2dDrawer2D::DrawCharDc( c );
#else // wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
    a2dDrawer2D::DrawCharDc( c );
#endif // wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
}

void a2dAggDrawerBase::DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize )
{
    if ( m_disableDrawing )
        return;

    if ( width == 0  || height == 0 )
        return;

    if ( pixelsize )
    {
        width = DeviceToWorldXRel( width );
        height = DeviceToWorldYRel( height );
        radius = DeviceToWorldXRel( radius );
        if( m_yaxis )
            height = -height;
    }

    if ( !IsStrokeOnly() )
    {
        agg::trans_affine mtx = _get_agg_user_to_device_transform();

        if ( fabs( radius ) <= 0.00000001 )
        {
            plain_rect r( x, y, x + width, y + height );
            agg::conv_transform<plain_rect> tr( r, mtx );
            m_rasterizer.reset();
            m_rasterizer.add_path( tr );
        }
        else
        {
            agg::rounded_rect r( x, y, x + width, y + height, radius );
            r.normalize_radius();
            agg::conv_transform<agg::rounded_rect> tr( r, mtx );
            m_rasterizer.reset();
            m_rasterizer.add_path( tr );
        }

        if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
        {
            RenderGradient( false );
        }
        else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
        {
            RenderGradient( true );
        }
        else if ( m_activefill.GetType() == a2dFILL_BITMAP ||
                  ( m_activefill.GetStyle() >= a2dFIRST_HATCH && m_activefill.GetStyle() <= a2dLAST_HATCH )  ||
                  ( m_activefill.GetStyle() >= a2dFIRST_TWOCOL_HATCH && m_activefill.GetStyle() <= a2dLAST_TWOCOL_HATCH )  )
        {
            RenderBitmapFill();
        }
        else
        {
            Render( true );
        }
    }

    if (  IsStroked()  )
    {
        if ( fabs( radius ) <= 0.00000001 )
        {
            plain_rect r( x, y, x + width, y + height );
            agg::trans_affine mtx = _get_agg_user_to_device_transform();
            m_rasterizer.reset();
            _ras_add_stroked_path_xform( r, mtx );
        }
        else
        {
            m_rasterizer.reset();
            agg::rounded_rect r( x, y, x + width, y + height, radius );
            r.normalize_radius();
            agg::trans_affine mtx = _get_agg_user_to_device_transform();
            _ras_add_stroked_path_xform( r, mtx );
        }
        Render( false );
    }
}

void a2dAggDrawerBase::DrawEllipse( double xc, double yc, double width, double height )
{
    if ( m_disableDrawing )
        return;

    double dphi;
    unsigned int segments = 20;

    //circular approximation of radius.
    double radiusDev = m_usertodevice.TransformDistance( wxMax( width, height ) );
    Aberration( m_displayaberration, wxPI * 2, radiusDev , dphi, segments );

    agg::trans_affine mtx = _get_agg_user_to_device_transform();

    agg::ellipse ellipse( xc, yc, width / 2.0, height / 2.0, segments );

    agg::conv_transform<agg::ellipse> tr( ellipse, mtx );

    if ( !IsStrokeOnly() )
    {
        m_path.remove_all();
        m_path.concat_path( tr, 0 );

        m_rasterizer.reset();
        m_rasterizer.add_path( m_path );

        if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
        {
            RenderGradient( false );
        }
        else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
        {
            RenderGradient( true );
        }
        else if ( m_activefill.GetType() == a2dFILL_BITMAP ||
                  ( m_activefill.GetStyle() >= a2dFIRST_HATCH && m_activefill.GetStyle() <= a2dLAST_HATCH )  ||
                  ( m_activefill.GetStyle() >= a2dFIRST_TWOCOL_HATCH && m_activefill.GetStyle() <= a2dLAST_TWOCOL_HATCH )  )
        {
            RenderBitmapFill();
        }
        else
            Render( true );
    }
    if (  IsStroked()  )
    {
        m_rasterizer.reset();
        _ras_add_stroked_path_xform( ellipse, mtx );
        Render( false );
    }
}

void a2dAggDrawerBase::DrawPolygon( a2dVertexArray* array, bool spline, wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    int segments;
    if ( spline )
    {
        a2dVertexArray* splinedlist = array->ConvertSplinedPolyline( m_splineaberration );
        segments = ToAggPath( splinedlist, false );
        delete splinedlist;
    }
    else
        segments = ToAggPath( array, false );

    if ( segments == 0 )
        return;

    agg::trans_affine mtx = _get_agg_user_to_device_transform();
    if ( IsStrokeOnly() )
    {
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
    {
        m_path.close_polygon();
        agg::conv_transform<agg::path_storage> tr( m_path, mtx );
        m_rasterizer.reset();
        m_rasterizer.add_path( tr );

        RenderGradient( false );
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        m_path.close_polygon();
        agg::conv_transform<agg::path_storage> tr( m_path, mtx );
        m_rasterizer.reset();
        m_rasterizer.add_path( tr );

        RenderGradient( true );
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP ||
              ( m_activefill.GetStyle() >= a2dFIRST_HATCH && m_activefill.GetStyle() <= a2dLAST_HATCH )  ||
              ( m_activefill.GetStyle() >= a2dFIRST_TWOCOL_HATCH && m_activefill.GetStyle() <= a2dLAST_TWOCOL_HATCH )  )
    {
        m_path.close_polygon();
        agg::conv_transform<agg::path_storage> tr( m_path, mtx );
        m_rasterizer.reset();
        m_rasterizer.add_path( tr );

        RenderBitmapFill();
    }
    else //normal solid case
    {
        m_path.close_polygon();
        agg::conv_transform<agg::path_storage> tr( m_path, mtx );
        m_rasterizer.reset();
        m_rasterizer.add_path( tr );

        Render( true );
    }

    if (  IsStroked()  )
    {
        m_rasterizer.reset();
        m_path.close_polygon();
        _ras_add_stroked_path_xform( m_path, mtx );

        Render( false );
    }
}

void a2dAggDrawerBase::DrawPolygon( const a2dVertexList* list, bool spline, wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    int segments;
    if ( spline )
    {
        a2dVertexList* splinedlist = list->ConvertSplinedPolygon( m_splineaberration );
        segments = ToAggPath( splinedlist, false );
        delete splinedlist;
    }
    else
        segments = ToAggPath( list, false );

    if ( segments == 0 )
        return;

    agg::trans_affine mtx = _get_agg_user_to_device_transform();
    if ( IsStrokeOnly() )
    {
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
    {
        m_path.close_polygon();
        agg::conv_transform<agg::path_storage> tr( m_path, mtx );
        m_rasterizer.reset();
        m_rasterizer.add_path( tr );

        RenderGradient( false );
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        m_path.close_polygon();
        agg::conv_transform<agg::path_storage> tr( m_path, mtx );
        m_rasterizer.reset();
        m_rasterizer.add_path( tr );

        RenderGradient( true );
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP ||
              ( m_activefill.GetStyle() >= a2dFIRST_HATCH && m_activefill.GetStyle() <= a2dLAST_HATCH )  ||
              ( m_activefill.GetStyle() >= a2dFIRST_TWOCOL_HATCH && m_activefill.GetStyle() <= a2dLAST_TWOCOL_HATCH )  )
    {
        m_path.close_polygon();

        agg::conv_transform<agg::path_storage> tr( m_path, mtx );
        m_rasterizer.reset();
        m_rasterizer.add_path( tr );

        RenderBitmapFill();
    }
    else //normal solid case
    {
        m_path.close_polygon();

        agg::conv_transform<agg::path_storage> tr( m_path, mtx );
        m_rasterizer.reset();
        m_rasterizer.add_path( tr );

        Render( true );
    }

    if (  IsStroked()  )
    {
        m_rasterizer.reset();
        m_path.close_polygon();
        _ras_add_stroked_path_xform( m_path, mtx );
        Render( false );
    }
}

void a2dAggDrawerBase::DrawPolyPolygon( a2dListOfa2dVertexList polylist, wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    for( a2dListOfa2dVertexList::iterator iterp = polylist.begin(); iterp != polylist.end(); iterp++ )
    {
        DrawPolygon( *iterp, false, fillStyle );
    }
}

void a2dAggDrawerBase::DrawLines( const a2dVertexList* list, bool spline )
{
    if ( m_disableDrawing )
        return;

    int segments;
    if ( spline )
    {
        a2dVertexList* splinedlist = list->ConvertSplinedPolyline( m_splineaberration );
        segments = ToAggPath( splinedlist, false );
        delete splinedlist;
    }
    else
        segments = ToAggPath( list, false );

    if ( segments == 0 )
        return;

    agg::trans_affine mtx = _get_agg_user_to_device_transform();
    m_rasterizer.reset();
    _ras_add_stroked_path_xform( m_path, mtx );
    Render( false );
}

void a2dAggDrawerBase::DrawLines( a2dVertexArray* array, bool spline )
{
    if ( m_disableDrawing )
        return;

    int segments;
    if ( spline )
    {
        a2dVertexArray* splinedlist = array->ConvertSplinedPolyline( m_splineaberration );
        segments = ToAggPath( splinedlist, false );
        delete splinedlist;
    }
    else
        segments = ToAggPath( array, false );

    if ( segments == 0 )
        return;

    agg::trans_affine mtx = _get_agg_user_to_device_transform();
    m_rasterizer.reset();
    _ras_add_stroked_path_xform( m_path, mtx );
    Render( false );
}

void a2dAggDrawerBase::DrawCharFreetype( wxChar c )
{
    double x, y;
    x = y = 0.0;

    if ( m_disableDrawing )
        return;

#if wxART2D_USE_FREETYPE
    y += m_currentfont.GetDescent();

    agg::path_storage path;
    double scale = m_currentfont.GetSize() / ( 64 * m_currentfont.GetDeviceHeight() );

    FT_Glyph glyph = m_currentfont.GetGlyphFreetype( c )->m_glyph;
    if ( glyph->format != FT_GLYPH_FORMAT_OUTLINE )
        return;
    FT_Outline& outline = ( ( FT_OutlineGlyph ) glyph )->outline;

    FT_Vector   v_last;
    FT_Vector   v_control;
    FT_Vector   v_start;

    FT_Vector*  point;
    FT_Vector*  limit;
    char*       tags;

    int   n;         // index of contour in outline
    int   first;     // index of first point in contour
    char  tag;       // current point's state

    first = 0;

    for( n = 0; n < outline.n_contours; n++ )
    {
        int  last;  // index of last point in contour

        last  = outline.contours[n];
        limit = outline.points + last;

        v_start = outline.points[first];
        v_last  = outline.points[last];

        v_control = v_start;

        point = outline.points + first;
        tags  = outline.tags  + first;
        tag   = FT_CURVE_TAG( tags[0] );

        // A contour cannot start with a cubic control point!
        if( tag == FT_CURVE_TAG_CUBIC ) return;

        // check first point to determine origin
        if( tag == FT_CURVE_TAG_CONIC )
        {
            // first point is conic control.  Yes, this happens.
            if( FT_CURVE_TAG( outline.tags[last] ) == FT_CURVE_TAG_ON )
            {
                // start at last point if it is on the curve
                v_start = v_last;
                limit--;
            }
            else
            {
                // if both first and last points are conic,
                // start at their middle and record its position
                // for closure
                v_start.x = ( v_start.x + v_last.x ) / 2;
                v_start.y = ( v_start.y + v_last.y ) / 2;

                v_last = v_start;
            }
            point--;
            tags--;
        }

        path.move_to( v_start.x, v_start.y );

        while( point < limit )
        {
            point++;
            tags++;

            tag = FT_CURVE_TAG( tags[0] );
            switch( tag )
            {
                case FT_CURVE_TAG_ON:  // emit a single line_to
                {
                    path.line_to( point->x, point->y );
                    continue;
                }

                case FT_CURVE_TAG_CONIC:  // consume conic arcs
                {
                    v_control.x = point->x;
                    v_control.y = point->y;

Do_Conic:
                    if( point < limit )
                    {
                        FT_Vector vec;
                        FT_Vector v_middle;

                        point++;
                        tags++;
                        tag = FT_CURVE_TAG( tags[0] );

                        vec.x = point->x;
                        vec.y = point->y;

                        if( tag == FT_CURVE_TAG_ON )
                        {
                            path.curve3( v_control.x, v_control.y, vec.x, vec.y );
                            continue;
                        }

                        if( tag != FT_CURVE_TAG_CONIC ) return;

                        v_middle.x = ( v_control.x + vec.x ) / 2;
                        v_middle.y = ( v_control.y + vec.y ) / 2;

                        path.curve3( v_control.x, v_control.y, v_middle.x, v_middle.y );

                        v_control = vec;
                        goto Do_Conic;
                    }
                    path.curve3( v_control.x, v_control.y, v_start.x, v_start.y );
                    goto Close;
                }

                default:  // FT_CURVE_TAG_CUBIC
                {
                    FT_Vector vec1, vec2;

                    if( point + 1 > limit || FT_CURVE_TAG( tags[1] ) != FT_CURVE_TAG_CUBIC )
                    {
                        return;
                    }

                    vec1.x = point[0].x;
                    vec1.y = point[0].y;
                    vec2.x = point[1].x;
                    vec2.y = point[1].y;

                    point += 2;
                    tags  += 2;

                    if( point <= limit )
                    {
                        FT_Vector vec;

                        vec.x = point->x;
                        vec.y = point->y;

                        path.curve4( vec1.x, vec1.y, vec2.x, vec2.y, vec.x, vec.y );
                        continue;
                    }

                    path.curve4( vec1.x, vec1.y, vec2.x, vec2.y, v_start.x, v_start.y );
                    goto Close;
                }
            }
        }

        path.close_polygon();

Close:
        first = last + 1;
    }

    // scale font to size
    a2dAffineMatrix affine;
    affine.Scale( scale );
    affine.Translate( x, y );
    affine = GetUserToDeviceTransform() * affine;

    agg::trans_affine mtx( affine.GetValue( 0, 0 ), affine.GetValue( 0, 1 ),
                           affine.GetValue( 1, 0 ), affine.GetValue( 1, 1 ),
                           affine.GetValue( 2, 0 ), affine.GetValue( 2, 1 ) );
    agg::conv_transform< agg::path_storage > tr( path, mtx );
    agg::conv_curve< agg::conv_transform< agg::path_storage > > stroke( tr );

    m_rasterizer.reset();
    m_rasterizer.add_path( stroke );

    Render( false );
#else // wxART2D_USE_FREETYPE
    a2dDrawer2D::DrawCharFreetype( c );
#endif // wxART2D_USE_FREETYPE
}

void a2dAggDrawerBase::DrawCharStroke( wxChar c )
{
    agg::path_storage path;
    double size = m_currentfont.GetSize();
    a2dVertexList** ptr = m_currentfont.GetGlyphStroke( c );
    if ( ptr )
    {
        while ( *ptr )
        {
            a2dVertexList::iterator iter = ( *ptr )->begin();
            if ( ( *ptr )->size() )
            {
                a2dPoint2D point = ( *iter )->GetPoint();
                path.move_to( point.m_x * size, point.m_y * size );
                iter++;
            }
            while ( iter != ( *ptr )->end() )
            {
                a2dPoint2D point = ( *iter )->GetPoint();
                path.line_to( point.m_x * size, point.m_y * size );
                iter++;
            }
            ptr++;
        }
    }

    // apply affine matrix
    agg::trans_affine mtx( GetUserToDeviceTransform().GetValue( 0, 0 ), GetUserToDeviceTransform().GetValue( 0, 1 ),
                           GetUserToDeviceTransform().GetValue( 1, 0 ), GetUserToDeviceTransform().GetValue( 1, 1 ),
                           GetUserToDeviceTransform().GetValue( 2, 0 ), GetUserToDeviceTransform().GetValue( 2, 1 ) );
    agg::conv_transform< agg::path_storage > tr( path, mtx );
    agg::conv_stroke< agg::conv_transform< agg::path_storage > > stroke( tr );

    stroke.line_join( m_join );
    stroke.line_cap( m_cap );
    stroke.width( m_currentfont.GetStrokeWidth() * GetMappingMatrix()( 0, 0 ) );
    m_rasterizer.reset();
    m_rasterizer.add_path( stroke );

    Render( true );
}



/*******************************************************************
a2dAggDrawerRgba
********************************************************************/

IMPLEMENT_DYNAMIC_CLASS( a2dAggDrawerRgba, a2dAggDrawerBase )

void a2dAggDrawerRgba::DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b , unsigned char a )
{
    agg::rgba8 colorfill( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
    m_renBase.blend_pixel( x1, y1, colorfill, 255 );
}

void a2dAggDrawerRgba::Render( bool fillColor )
{
    if ( fillColor )
    {
        agg::rgba8 colorfill( m_colour1redFill, m_colour1greenFill, m_colour1blueFill, m_FillOpacityCol1 );
        m_renderer.color( colorfill );
        m_renSolidComp.color( colorfill );
    }
    else
    {
        agg::rgba8 colorstroke( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
        m_renderer.color( colorstroke );
        m_renSolidComp.color( colorstroke );
    }

    if( m_blendMode == agg::end_of_comp_op_e )
        agg::render_scanlines( m_rasterizer, m_sl, m_renderer );
    else
        agg::render_scanlines( m_rasterizer, m_sl, m_renSolidComp );
}

void a2dAggDrawerRgba::RenderGradient( bool radial )
{
    if ( !radial )
    {
        span_allocator_type aggallocator;
        LinearGradientSpan span( /*gr.m_allocator, */
            m_fillGradientInterpolator,
            m_linearGradientFunction,
            m_fillGradient,
            m_fillGradientD1,
            m_fillGradientD2 );
        RendererLinearGradientA ren( m_renBase, aggallocator, span );
        RendererLinearGradientComp renComp( m_renBaseComp, aggallocator, span );

        if( m_blendMode == agg::end_of_comp_op_e )
            agg::render_scanlines( m_rasterizer, m_sl, ren );
        else
            agg::render_scanlines( m_rasterizer, m_sl, renComp );
    }
    else
    {
        span_allocator_type aggallocator;
        RadialGradientSpan span( /*gr.m_allocator, */
            m_fillGradientInterpolator,
            m_radialGradientFunction,
            m_fillGradient,
            m_fillGradientD1,
            m_fillGradientD2 );
        RendererRadialGradientA ren( m_renBase, aggallocator, span );
        RendererRadialGradientComp renComp( m_renBaseComp, aggallocator, span );

        if( m_blendMode == agg::end_of_comp_op_e )
            agg::render_scanlines( m_rasterizer, m_sl, ren );
        else
            agg::render_scanlines( m_rasterizer, m_sl, renComp );

    }
}

void a2dAggDrawerRgba::RenderBitmapFill()
{
    unsigned offset_x = 0;
    unsigned offset_y = 0;

    //typedef agg::wrap_mode_reflect_auto_pow2 wrap_x_type;
    //typedef agg::wrap_mode_reflect_auto_pow2 wrap_y_type;
    typedef agg::wrap_mode_repeat wrap_x_type;
    typedef agg::wrap_mode_repeat wrap_y_type;

    typedef agg::image_accessor_wrap<PixFormat, wrap_x_type, wrap_y_type> img_source_type;
    typedef agg::span_pattern_a2d<img_source_type> span_gen_type;
    typedef agg::span_allocator<color_type> span_alloc_type;

    typedef agg::renderer_scanline_aa<RendererBaseA, span_alloc_type, span_gen_type> renderer_type;

    PixFormat  img_pixf( m_pattern_rbuf );
    img_source_type img_src( img_pixf );
    span_gen_type sg( img_src, offset_x, offset_y );

    sg.alpha( span_gen_type::value_type( m_FillOpacityCol1 ) );

    span_alloc_type sa;
    renderer_type rp( m_renBase, sa, sg );

    agg::render_scanlines( m_rasterizer, m_sl, rp );
}

void a2dAggDrawerRgba::RenderSetClip()
{
    m_renBase.clip_box( m_clipboxdev.x, m_clipboxdev.y, m_clipboxdev.x + m_clipboxdev.width, m_clipboxdev.y + m_clipboxdev.height );
    m_renBaseComp.clip_box( m_clipboxdev.x, m_clipboxdev.y, m_clipboxdev.x + m_clipboxdev.width, m_clipboxdev.y + m_clipboxdev.height );
    m_rasterizer.clip_box( 0, 0, m_width, m_height );
}

a2dAggDrawerRgba::a2dAggDrawerRgba( const wxSize& size ):
    m_rendering_buffer(),
    m_pixFormat( m_rendering_buffer ),
    m_pixFormatComp( m_rendering_buffer ),
    m_renBase( m_pixFormat ),
    m_renBaseComp( m_pixFormatComp ),
    m_renderer( m_renBase ),
    m_renSolidComp( m_renBaseComp ),

    a2dAggDrawerBase( size )
{
    m_buffer = a2dImageRGBA( m_width, m_height );
    m_ownsBuffer = true;
    m_preStroke = false;//true;

    m_pdata = m_buffer.GetData();
    m_rendering_buffer.attach( m_pdata, m_width, m_height, m_width * 4 );
    DestroyClippingRegion();
}

a2dAggDrawerRgba::a2dAggDrawerRgba( int width, int height ):
    m_rendering_buffer(),
    m_pixFormat( m_rendering_buffer ),
    m_pixFormatComp( m_rendering_buffer ),
    m_renBase( m_pixFormat ),
    m_renBaseComp( m_pixFormatComp ),
    m_renderer( m_renBase ),
    m_renSolidComp( m_renBaseComp ),

    a2dAggDrawerBase( width, height )
{
    if ( width == 0 || height == 0 )
        m_buffer = a2dImageRGBA( 100, 100 );
    else
        m_buffer = a2dImageRGBA( width, height );
    m_ownsBuffer = true;
    m_pdata = m_buffer.GetData();
    m_rendering_buffer.attach( m_pdata, m_width, m_height, m_width * 4 );
    m_pixFormatComp.premultiply();
    DestroyClippingRegion();
    m_preStroke = false;//true;
}


a2dAggDrawerRgba::a2dAggDrawerRgba( const a2dAggDrawerRgba& other ):
    m_rendering_buffer(),
    m_pixFormat( m_rendering_buffer ),
    m_pixFormatComp( m_rendering_buffer ),
    m_renBase( m_pixFormat ),
    m_renBaseComp( m_pixFormatComp ),
    m_renderer( m_renBase ),
    m_renSolidComp( m_renBaseComp ),

    a2dAggDrawerBase( other )
{
    m_ownsBuffer = false;
    m_width = other.m_width;
    m_height  = other.m_height;
    m_buffer = other.m_buffer;
    m_preStroke = other.m_preStroke;
}


a2dAggDrawerRgba::a2dAggDrawerRgba( const wxBitmap& bitmap ):
    m_rendering_buffer(),
    m_pixFormat( m_rendering_buffer ),
    m_pixFormatComp( m_rendering_buffer ),
    m_renBase( m_pixFormat ),
    m_renBaseComp( m_pixFormatComp ),
    m_renderer( m_renBase ),
    m_renSolidComp( m_renBaseComp ),

    a2dAggDrawerBase( bitmap.GetWidth(), bitmap.GetHeight() )
{
    wxImage image = bitmap.ConvertToImage();
    m_buffer = a2dImageRGBA( image );
    m_ownsBuffer = true;
    m_pdata = m_buffer.GetData();
    m_rendering_buffer.attach( m_pdata, m_width, m_height, m_width * 4 );
    DestroyClippingRegion();
    m_preStroke = false;//true;
}


a2dAggDrawerRgba::~a2dAggDrawerRgba()
{
}

wxImage a2dAggDrawerRgba::GetImageBuffer() const
{
    wxImage image = m_buffer.GetImage();
    return image;
}

void a2dAggDrawerRgba::SetBufferSize( int w, int h )
{
    a2dImageRGBA newbuf( w, h );

    int old_pixelwidth = m_width * 4;
    int new_pixelwidth = w * 4;
    int pixelwidth = wxMin( w, m_width ) * 4;
    int minheight =  wxMin( h, m_height );

    //pixel coordinates so ( 0,0 ) upper left
    unsigned char* oldpdata = m_pdata;
    unsigned char* newpdata = newbuf.GetData();

    for ( int yp = 0; yp < minheight; yp++ )
    {
        memcpy( newpdata, oldpdata, pixelwidth );
        oldpdata += old_pixelwidth;
        newpdata += new_pixelwidth;
    }

    m_width = w;
    m_height  = h;
    m_buffer = newbuf;

    m_pdata = m_buffer.GetData();
    m_rendering_buffer.attach( m_pdata, m_width, m_height, m_width * 4 );
    m_pixFormatComp.premultiply();
    DestroyClippingRegion();
}

wxBitmap a2dAggDrawerRgba::GetBuffer() const
{
    return wxBitmap( m_buffer.GetImage() );
}

wxBitmap a2dAggDrawerRgba::GetSubBitmap( wxRect rect ) const
{
    return wxBitmap( m_buffer.GetImage().GetSubImage( rect ) );
}

void a2dAggDrawerRgba::CopyIntoBuffer( const wxBitmap& bitm )
{
    m_buffer = a2dImageRGBA( bitm.ConvertToImage() );
}

void a2dAggDrawerRgba::DoSetDrawStyle( a2dDrawStyle drawstyle )
{
    a2dAggDrawerBase::DoSetDrawStyle( drawstyle );
    m_pixFormatComp.comp_op( m_blendMode );
}

void a2dAggDrawerRgba::DrawImage(  const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;
    int imagew = image.GetWidth();
    int imageh = image.GetHeight();

    agg::trans_affine mtxi;
    mtxi *= agg::trans_affine_scaling( width / imagew, height / imageh );
    mtxi *= agg::trans_affine_translation( x - width / 2, y - height / 2 );
    // pictures always upwards
    //if ( m_yaxis )
    //    mtxi *= agg::trans_affine_reflection_unit( 1, 0 );

    const a2dAffineMatrix& r2d = GetUserToDeviceTransform();
    agg::trans_affine mtx(
        r2d.GetValue( 0, 0 ), r2d.GetValue( 0, 1 ),
        r2d.GetValue( 1, 0 ), r2d.GetValue( 1, 1 ),
        r2d.GetValue( 2, 0 ), r2d.GetValue( 2, 1 )
    );
    mtxi *= mtx;
    mtxi.invert();

    typedef agg::span_allocator<agg::rgba8> span_alloc_type;

    span_alloc_type sa;
    typedef agg::image_accessor_clip<PixFormat> img_source_type;
    typedef agg::span_interpolator_linear<> interpolator_type;
    interpolator_type interpolator( mtxi );

    typedef agg::span_image_filter_rgb_bilinear<img_source_type, interpolator_type> span_gen_type;
    //typedef agg::span_image_filter_rgb_nn<img_source_type, interpolator_type> span_gen_type;

    typedef agg::span_converter<span_gen_type, span_conv_const_alpha_rgba8> span_conv;

    agg::rendering_buffer image_buffer;
    image_buffer.attach( image.GetData(), imagew, imageh, ( m_yaxis ) ? -imagew * 3 : imagew * 3 );
    PixFormat img_pixf( image_buffer );
    img_source_type img_src( img_pixf, agg::rgba( 1, 1, 1, 0 ) );

    span_gen_type sg( img_src, interpolator );

    //now add the path to render, which should be the image bounding polygon/rotate rectangle.
    //But you could even make cuts from the image using ellipse etc.
    /*
        agg::rounded_rect er(0,0,m_width,m_height, 0);
        er.normalize_radius();
        m_rasterizer.add_path(er);

        agg::ellipse ellipse( x, y, width/2, height/2);
        agg::conv_transform<agg::ellipse> tr(ellipse, mtx);
        m_rasterizer.add_path( tr );

        agg::rounded_rect er(x-width/2, y-height/2 ,x+width/2, y+height/2, 0);
        er.normalize_radius();
        agg::conv_transform<agg::rounded_rect> tr(er, mtx);
        m_rasterizer.add_path(tr);
    */
    agg::rounded_rect er( x - width / 2, y - height / 2 , x + width / 2, y + height / 2, 0 );
    er.normalize_radius();
    agg::conv_transform<agg::rounded_rect> tr( er, mtx );
    m_rasterizer.reset();
    m_rasterizer.add_path( tr );

    wxUint8 OpacityFactor = ( m_OpacityFactor * Opacity ) / 255;

    //NOT m_FillOpacityCol1 since for images fill does not influence the picture.
    if ( 1 ) //OpacityFactor != 255 )
    {
        span_conv_const_alpha_rgba8 color_alpha( OpacityFactor );
        span_conv sc( sg, color_alpha );
        typedef agg::renderer_scanline_aa<RendererBaseA, span_alloc_type, span_conv> renderer_type_alpha;
        renderer_type_alpha ri( m_renBase, sa, sc );

        agg::render_scanlines( m_rasterizer, m_sl, ri );
    }
    else
    {
        typedef agg::renderer_scanline_bin<RendererBaseA, span_alloc_type, span_gen_type> renderer_type_normal;
        renderer_type_normal ri( m_renBase, sa, sg );
        agg::render_scanlines( m_rasterizer, m_sl, ri );
    }
}

void a2dAggDrawerRgba::DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    int imagew = image.GetWidth();
    int imageh = image.GetHeight();

    agg::trans_affine mtxi;
    mtxi *= agg::trans_affine_scaling( width / imagew, height / imageh );
    mtxi *= agg::trans_affine_translation( x, y );

    const a2dAffineMatrix& r2d = GetUserToDeviceTransform();
    agg::trans_affine mtx(
        r2d.GetValue( 0, 0 ), r2d.GetValue( 0, 1 ),
        r2d.GetValue( 1, 0 ), r2d.GetValue( 1, 1 ),
        r2d.GetValue( 2, 0 ), r2d.GetValue( 2, 1 )
    );
    mtxi *= mtx;
    mtxi.invert();

    typedef agg::span_allocator<agg::rgba8> span_alloc_type;
    span_alloc_type sa;
    typedef agg::image_accessor_clip<agg::pixfmt_rgba32> img_source_type;
    typedef agg::span_interpolator_linear<> interpolator_type;
    interpolator_type interpolator( mtxi );

    typedef agg::span_image_filter_rgba_nn<img_source_type, interpolator_type> span_gen_type;

    typedef agg::span_converter<span_gen_type, span_conv_const_alpha_rgba8> span_conv;

    agg::rendering_buffer image_buffer;
    image_buffer.attach( image.GetData(), imagew, imageh, imagew * 4 );
    agg::pixfmt_rgba32 img_pixf( image_buffer );
    img_source_type img_src( img_pixf, agg::rgba( 0, 0, 0, 0 ) );

    span_gen_type sg( img_src, interpolator );


    agg::rounded_rect er( x, y , x + width, y + height, 1 );
    er.normalize_radius();
    agg::conv_transform<agg::rounded_rect> tr( er, mtx );
    m_rasterizer.reset();
    m_rasterizer.add_path( tr );

    wxUint8 OpacityFactor = ( m_OpacityFactor * Opacity ) / 255;

    //NOT m_FillOpacityCol1 since for images fill does not influence the picture.
    if ( OpacityFactor != 255 )
    {
        span_conv_const_alpha_rgba8 color_alpha( OpacityFactor );
        span_conv sc( sg, color_alpha );
        typedef agg::renderer_scanline_aa<RendererBaseA, span_alloc_type, span_conv> renderer_type_alpha;
        renderer_type_alpha ri( m_renBase, sa, sc );

        agg::render_scanlines( m_rasterizer, m_sl, ri );
    }
    else
    {
        typedef agg::renderer_scanline_bin<RendererBaseA, span_alloc_type, span_gen_type> renderer_type_normal;
        renderer_type_normal ri( m_renBase, sa, sg );
        agg::render_scanlines( m_rasterizer, m_sl, ri );
    }
}

void a2dAggDrawerRgba::BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos )
{
    // clip to buffer
    if ( rect.x < 0 )
    {
        rect.width += rect.x;
        rect.x = 0;
    }
    if ( rect.width <= 0 ) return;

    if ( rect.y < 0 )
    {
        rect.height += rect.y;
        rect.y = 0;
    }
    if ( rect.height <= 0 ) return;

    if ( rect.x + rect.width >= m_width )
        rect.width = m_width - rect.x;

    if ( rect.width <= 0 ) return;

    if ( rect.y + rect.height >= m_height )
        rect.height = m_height - rect.y;

    if ( rect.height <= 0 ) return;

    int xmax = rect.x + rect.width;
    int ymax = rect.y + rect.height;

    a2dImageRGBA* subImage = m_buffer.GetSubImage( rect );
    wxBitmap subbitmap = subImage->CreateBitmap();
    delete subImage;

    wxMemoryDC mdc;
    mdc.SelectObject( subbitmap );

    dc->Blit( rect.x - bufferpos.x, rect.y - bufferpos.y, xmax - rect.x, ymax - rect.y, &mdc, 0, 0, wxCOPY, false );

    mdc.SelectObject( wxNullBitmap );
}

void a2dAggDrawerRgba::ShiftBuffer( int dxy, bool yshift )
{
    ShiftBufferInternal( dxy, yshift, 4 );
}

/*******************************************************************
a2dAggDrawer
********************************************************************/

void a2dAggDrawer::DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b , unsigned char a )
{
    agg::rgba8 colorfill( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
    m_renBase.blend_pixel( x1, y1, colorfill, 255 );
}

void a2dAggDrawer::Render( bool fillColor )
{
    if ( fillColor )
    {
        agg::rgba8 colorfill( m_colour1redFill, m_colour1greenFill, m_colour1blueFill, m_FillOpacityCol1 );
        m_renderer.color( colorfill );
    }
    else
    {
        agg::rgba8 colorstroke( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
        m_renderer.color( colorstroke );
    }
    agg::render_scanlines( m_rasterizer, m_sl, m_renderer );
}

void a2dAggDrawer::RenderGradient( bool radial )
{
    if ( !radial )
    {
        span_allocator_type aggallocator;
        LinearGradientSpan span( /*gr.m_allocator, */
            m_fillGradientInterpolator,
            m_linearGradientFunction,
            m_fillGradient,
            m_fillGradientD1,
            m_fillGradientD2 );
        RendererLinearGradient ren( m_renBase, aggallocator, span );
        agg::render_scanlines( m_rasterizer, m_sl, ren );
    }
    else
    {
        span_allocator_type aggallocator;
        RadialGradientSpan span( /*gr.m_allocator, */
            m_fillGradientInterpolator,
            m_radialGradientFunction,
            m_fillGradient,
            m_fillGradientD1,
            m_fillGradientD2 );
        RendererRadialGradient ren( m_renBase, aggallocator, span );
        agg::render_scanlines( m_rasterizer, m_sl, ren );
    }
}

void a2dAggDrawer::RenderBitmapFill()
{
    unsigned offset_x = 0;
    unsigned offset_y = 0;

    //typedef agg::wrap_mode_reflect_auto_pow2 wrap_x_type;
    //typedef agg::wrap_mode_reflect_auto_pow2 wrap_y_type;
    typedef agg::wrap_mode_repeat wrap_x_type;
    typedef agg::wrap_mode_repeat wrap_y_type;

    typedef agg::image_accessor_wrap<PixFormat, wrap_x_type, wrap_y_type> img_source_type;
    typedef agg::span_pattern_a2d<img_source_type> span_gen_type;
    typedef agg::span_allocator<color_type> span_alloc_type;

    typedef agg::renderer_scanline_aa<RendererBase, span_alloc_type, span_gen_type> renderer_type;

    PixFormat  img_pixf( m_pattern_rbuf );
    img_source_type img_src( img_pixf );
    span_gen_type sg( img_src, offset_x, offset_y );

    sg.alpha( span_gen_type::value_type( m_FillOpacityCol1 ) );

    span_alloc_type sa;
    renderer_type rp( m_renBase, sa, sg );

    agg::render_scanlines( m_rasterizer, m_sl, rp );
}

void a2dAggDrawer::RenderSetClip()
{
    m_renBase.clip_box( m_clipboxdev.x, m_clipboxdev.y, m_clipboxdev.x + m_clipboxdev.width, m_clipboxdev.y + m_clipboxdev.height );
    m_rasterizer.clip_box( 0, 0, m_width, m_height );
}

IMPLEMENT_DYNAMIC_CLASS( a2dAggDrawer, a2dAggDrawerBase )

a2dAggDrawer::a2dAggDrawer( const wxSize& size ):
    m_rendering_buffer(),
    m_pixFormat( m_rendering_buffer ),
    m_renBase( m_pixFormat ),
    m_renderer( m_renBase ),

    a2dAggDrawerBase( size )
{
    m_buffer = wxImage( m_width, m_height );

    m_pdata = m_buffer.GetData();
    m_rendering_buffer.attach( m_pdata, m_width, m_height, m_width * 3 );
    DestroyClippingRegion();
    m_preStroke = false;//true;
}

a2dAggDrawer::a2dAggDrawer( int width, int height ):
    m_rendering_buffer(),
    m_pixFormat( m_rendering_buffer ),
    m_renBase( m_pixFormat ),
    m_renderer( m_renBase ),

    a2dAggDrawerBase( width, height )
{
    if ( width == 0 || height == 0 )
        m_buffer = wxImage( 100, 100 );
    else
        m_buffer = wxImage( width, height );

    m_pdata = m_buffer.GetData();
    m_rendering_buffer.attach( m_pdata, m_width, m_height, m_width * 3 );
    DestroyClippingRegion();
    m_preStroke = false;//true;
}

a2dAggDrawer::a2dAggDrawer( const a2dAggDrawer& other ):
    m_rendering_buffer(),
    m_pixFormat( m_rendering_buffer ),
    m_renBase( m_pixFormat ),
    m_renderer( m_renBase ),

    a2dAggDrawerBase( other )
{
    m_width = other.m_width;
    m_height  = other.m_height;
    m_buffer = other.m_buffer;
    m_preStroke = other.m_preStroke;
}

a2dAggDrawer::a2dAggDrawer( const wxBitmap& bitmap ):
    m_rendering_buffer(),
    m_pixFormat( m_rendering_buffer ),
    m_renBase( m_pixFormat ),
    m_renderer( m_renBase ),

    a2dAggDrawerBase( bitmap.GetWidth(), bitmap.GetHeight() )
{
    m_buffer = bitmap.ConvertToImage();
    m_pdata = m_buffer.GetData();
    m_rendering_buffer.attach( m_pdata, m_width, m_height, m_width * 3 );
    DestroyClippingRegion();
    m_preStroke = false;//true;
}

a2dAggDrawer::~a2dAggDrawer()
{
}

void a2dAggDrawer::SetBufferSize( int w, int h )
{
    wxImage newbuf = wxImage( w, h );

    int old_pixelwidth = m_width * 3;
    int new_pixelwidth = w * 3;
    int pixelwidth = wxMin( w, m_width ) * 3;
    int minheight =  wxMin( h, m_height );

    //pixel coordinates so ( 0,0 ) upper left
    unsigned char* oldpdata = m_pdata;
    unsigned char* newpdata = newbuf.GetData();

    for ( int yp = 0; yp < minheight; yp++ )
    {
        memcpy( newpdata, oldpdata, pixelwidth );
        oldpdata += old_pixelwidth;
        newpdata += new_pixelwidth;
    }

    m_width = w;
    m_height  = h;
    m_buffer = newbuf;

    m_pdata = m_buffer.GetData();
    m_rendering_buffer.attach( m_pdata, m_width, m_height, m_width * 3 );
    /*
        agg::renderer_rgb24_util  utl( *m_rendering_buffer );

        unsigned char colour1red = m_backgroundfill->GetColour().Red();
        unsigned char colour1green = m_backgroundfill->GetColour().Green();
        unsigned char colour1blue = m_backgroundfill->GetColour().Blue();

        agg::rgba8 color( colour1red, colour1green, colour1blue, m_backgroundfill->m_FillOpacityCol1 );
        utl.clear( color );
    */
}

wxBitmap a2dAggDrawer::GetBuffer() const
{
    return wxBitmap( m_buffer );
}

wxBitmap a2dAggDrawer::GetSubBitmap( wxRect rect ) const
{
    return wxBitmap( m_buffer.GetSubImage( rect ) );
}

void a2dAggDrawer::CopyIntoBuffer( const wxBitmap& bitm )
{
    m_buffer = bitm.ConvertToImage();
    m_pdata = m_buffer.GetData();
    m_rendering_buffer.attach( m_pdata, m_width, m_height, m_width * 3 );
    DestroyClippingRegion();
}

void a2dAggDrawer::DrawImage(  const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;
    int imagew = image.GetWidth();
    int imageh = image.GetHeight();

    agg::trans_affine mtxi;
    mtxi *= agg::trans_affine_scaling( width / imagew, height / imageh );
    mtxi *= agg::trans_affine_translation( x - width / 2, y - height / 2 );
    // pictures always upwards
    //if ( m_yaxis )
    //    mtxi *= agg::trans_affine_reflection_unit( 1, 0 );

    const a2dAffineMatrix& r2d = GetUserToDeviceTransform();
    agg::trans_affine mtx(
        r2d.GetValue( 0, 0 ), r2d.GetValue( 0, 1 ),
        r2d.GetValue( 1, 0 ), r2d.GetValue( 1, 1 ),
        r2d.GetValue( 2, 0 ), r2d.GetValue( 2, 1 )
    );
    mtxi *= mtx;
    mtxi.invert();

    typedef agg::span_allocator<agg::rgba8> span_alloc_type;
    span_alloc_type sa;
    typedef agg::image_accessor_clip<agg::pixfmt_rgba32> img_source_type;
    typedef agg::span_interpolator_linear<> interpolator_type;
    interpolator_type interpolator( mtxi );

    typedef agg::span_image_filter_rgba_nn<img_source_type, interpolator_type> span_gen_type;

    typedef agg::span_converter<span_gen_type, span_conv_const_alpha_rgba8> span_conv;

    typedef agg::renderer_scanline_aa<RendererBase, span_alloc_type, span_conv> renderer_type_alpha;

    typedef agg::renderer_scanline_bin<RendererBase, span_alloc_type, span_gen_type> renderer_type_normal;

    agg::rendering_buffer image_buffer;

    unsigned char* glimage = ( unsigned char* ) malloc( imagew * imageh * 4 );
    int i, j;
    unsigned char* data =  ( unsigned char* ) glimage;
    for ( i = 0; i < imageh; i++ )
    {
        for ( j = 0; j < imagew; j++ )
        {
            *data++ = image.GetRed( j, i );
            *data++ = image.GetGreen( j, i );
            *data++ = image.GetBlue( j, i );
            if ( image.HasAlpha() )
                * data++ = ( image.GetAlpha( j, i ) * Opacity ) / 255;
            else if ( image.HasMask() )
            {
                if ( image.IsTransparent( j, i ) )
                    * data++ = ( unsigned char ) 0;
                else
                    * data++ = Opacity;
            }
            else
                *data++ = ( unsigned char ) Opacity;
        }
    }

    image_buffer.attach( glimage, imagew, imageh, imagew * 4 );
    agg::pixfmt_rgba32 img_pixf( image_buffer );
    img_source_type img_src( img_pixf, agg::rgba( 0, 0, 0, 0 ) );

    span_gen_type sg( img_src, interpolator );

    //now add the path to render, which should be the image bounding polygon/rotate rectangle.
    //But you could even make cuts from the image using ellipse etc.
    /*
        agg::rounded_rect er(0,0,m_width,m_height, 0);
        er.normalize_radius();
        m_rasterizer.add_path(er);

        agg::ellipse ellipse( x, y, width/2, height/2);
        agg::conv_transform<agg::ellipse> tr(ellipse, mtx);
        m_rasterizer.add_path( tr );

        agg::rounded_rect er(x-width/2, y-height/2 ,x+width/2, y+height/2, 0);
        er.normalize_radius();
        agg::conv_transform<agg::rounded_rect> tr(er, mtx);
        m_rasterizer.add_path(tr);
    */
    agg::rounded_rect er( x - width / 2, y - height / 2 , x + width / 2, y + height / 2, 0 );
    er.normalize_radius();
    agg::conv_transform<agg::rounded_rect> tr( er, mtx );
    m_rasterizer.reset();
    m_rasterizer.add_path( tr );

    wxUint8 OpacityFactor = ( m_OpacityFactor * Opacity ) / 255;

    //NOT m_FillOpacityCol1 since for images fill does not influence the picture.
    if ( OpacityFactor != 255 )
    {
        span_conv_const_alpha_rgba8 color_alpha( OpacityFactor );
        //span_conv_array_alpha_rgba8 color_alpha_array( image, OpacityFactor );
        span_conv sc( sg, color_alpha );
        renderer_type_alpha ri( m_renBase, sa, sc );

        agg::render_scanlines( m_rasterizer, m_sl, ri );
    }
    else
    {
        renderer_type_normal ri( m_renBase, sa, sg );
        agg::render_scanlines( m_rasterizer, m_sl, ri );
    }
    delete glimage;
}

void a2dAggDrawer::DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    int imagew = image.GetWidth();
    int imageh = image.GetHeight();

    agg::trans_affine mtxi;
    mtxi *= agg::trans_affine_scaling( width / imagew, height / imageh );
    mtxi *= agg::trans_affine_translation( x, y );

    const a2dAffineMatrix& r2d = GetUserToDeviceTransform();
    agg::trans_affine mtx(
        r2d.GetValue( 0, 0 ), r2d.GetValue( 0, 1 ),
        r2d.GetValue( 1, 0 ), r2d.GetValue( 1, 1 ),
        r2d.GetValue( 2, 0 ), r2d.GetValue( 2, 1 )
    );
    mtxi *= mtx;
    mtxi.invert();

    typedef agg::span_allocator<agg::rgba8> span_alloc_type;
    span_alloc_type sa;
    typedef agg::image_accessor_clip<agg::pixfmt_rgba32> img_source_type;
    typedef agg::span_interpolator_linear<> interpolator_type;
    interpolator_type interpolator( mtxi );

    typedef agg::span_image_filter_rgba_nn<img_source_type, interpolator_type> span_gen_type;

    typedef agg::span_converter<span_gen_type, span_conv_const_alpha_rgba8> span_conv;

    typedef agg::renderer_scanline_aa<RendererBase, span_alloc_type, span_conv> renderer_type_alpha;

    typedef agg::renderer_scanline_bin<RendererBase, span_alloc_type, span_gen_type> renderer_type_normal;

    agg::rendering_buffer image_buffer;
    image_buffer.attach( image.GetData(), imagew, imageh, imagew * 4 );
    agg::pixfmt_rgba32 img_pixf( image_buffer );
    img_source_type img_src( img_pixf, agg::rgba( 0, 0, 0, 0 ) );

    span_gen_type sg( img_src, interpolator );


    agg::rounded_rect er( x, y , x + width, y + height, 1 );
    er.normalize_radius();
    agg::conv_transform<agg::rounded_rect> tr( er, mtx );
    m_rasterizer.reset();
    m_rasterizer.add_path( tr );

    wxUint8 OpacityFactor = ( m_OpacityFactor * Opacity ) / 255;

    //NOT m_FillOpacityCol1 since for images fill does not influence the picture.
    if ( OpacityFactor != 255 )
    {
        span_conv_const_alpha_rgba8 color_alpha( OpacityFactor );
        span_conv sc( sg, color_alpha );
        renderer_type_alpha ri( m_renBase, sa, sc );

        agg::render_scanlines( m_rasterizer, m_sl, ri );
    }
    else
    {
        renderer_type_normal ri( m_renBase, sa, sg );
        agg::render_scanlines( m_rasterizer, m_sl, ri );
    }
}

void a2dAggDrawer::BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos )
{
    // clip to buffer
    if ( rect.x < 0 )
    {
        rect.width += rect.x;
        rect.x = 0;
    }
    if ( rect.width <= 0 ) return;

    if ( rect.y < 0 )
    {
        rect.height += rect.y;
        rect.y = 0;
    }
    if ( rect.height <= 0 ) return;

    if ( rect.x + rect.width >= m_width )
        rect.width = m_width - rect.x;

    if ( rect.width <= 0 ) return;

    if ( rect.y + rect.height >= m_height )
        rect.height = m_height - rect.y;

    if ( rect.height <= 0 ) return;

    int xmax = rect.x + rect.width;
    int ymax = rect.y + rect.height;

    wxBitmap subbitmap = m_buffer.GetSubImage( rect );
    wxMemoryDC mdc;
    mdc.SelectObject( subbitmap );

    dc->Blit( rect.x - bufferpos.x, rect.y - bufferpos.y, xmax - rect.x, ymax - rect.y, &mdc, 0, 0, wxCOPY, false );

    mdc.SelectObject( wxNullBitmap );
}

void a2dAggDrawer::ShiftBuffer( int dxy, bool yshift )
{
    ShiftBufferInternal( dxy, yshift, 3 );
}

