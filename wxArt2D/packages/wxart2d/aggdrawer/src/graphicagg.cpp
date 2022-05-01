/*! \file aggdrawer/src/aggdrawer.cpp
    \brief a2dAggContext - Drawer using the antigrain library
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: graphicagg.cpp,v 1.11 2009/10/06 18:40:31 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#if wxART2D_USE_AGGDRAWER

#if wxART2D_USE_GRAPHICS_CONTEXT

#ifndef WX_PRECOMP
#include "wx/image.h"
#include "wx/window.h"
#include "wx/dc.h"
#include "wx/utils.h"
#include "wx/dialog.h"
#include "wx/app.h"
#include "wx/bitmap.h"
#include "wx/dcmemory.h"
#include "wx/log.h"
#include "wx/icon.h"
#include "wx/dcprint.h"
#include "wx/module.h"
#endif

#include "wx/aggdrawer/graphicagg.h"
#include "wx/artbase/stylebase.h"

#include "wx/dc.h"

#include "wx/graphics.h"
#include "wx/rawbmp.h"

#if wxART2D_USE_FREETYPE
#include <ft2build.h>
#ifdef _MSC_VER
// Do not use the define include for MSVC, because of a bug with precompiled
// headers. See (freetype)/doc/INSTALL.ANY for (some) more information
#include <freetype.h>
#include <ftoutln.h>
#else // _MSC_VER
// This is the way it should be included.
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#endif // _MSC_VER
#endif

#include <vector>

using namespace std;

//-----------------------------------------------------------------------------
// constants
//-----------------------------------------------------------------------------

const double RAD2DEG = 180.0 / M_PI;

//-----------------------------------------------------------------------------
// Local functions
//-----------------------------------------------------------------------------

static inline double dmin( double a, double b )
{
    return a < b ? a : b;
}
static inline double dmax( double a, double b )
{
    return a > b ? a : b;
}

static inline double DegToRad( double deg )
{
    return ( deg * M_PI ) / 180.0;
}
static inline double RadToDeg( double deg )
{
    return ( deg * 180.0 ) / M_PI;
}


//-----------------------------------------------------------------------------
// wxGraphicsPath implementation
//-----------------------------------------------------------------------------

// TODO remove this dependency (gdiplus needs the macros)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


//-----------------------------------------------------------------------------
// a2dAggContext implementation
//-----------------------------------------------------------------------------
template< class PathT >
void a2dAggContext::_ras_add_stroked_path_xform( PathT& path, const
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
                stroke.width( m_strokewidthDev );
            else
                stroke.width( m_strokewidth );
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


void a2dAggContext::_stipple_render_scanlines( agg::rasterizer_scanline_aa<>& ras,
        agg::scanline_u8& sl,
        RendererBaseA& renb,
        agg::rendering_buffer pat,
        double alpha )
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

    PixFormat  img_pixf( pat );
    img_source_type img_src( img_pixf );
    span_gen_type sg( img_src, offset_x, offset_y );

    sg.alpha( span_gen_type::value_type( alpha ) );

    span_alloc_type sa;
    renderer_type rp( renb, sa, sg );

    agg::render_scanlines( ras, sl, rp );
}

a2dAggContext::a2dAggContext( wxGraphicsRenderer* renderer, const wxImage& drawable )
    :
    m_rendering_buffer(),
    m_pixFormat( m_rendering_buffer ),
    m_pixFormatComp( m_rendering_buffer ),
    m_renBase( m_pixFormat ),
    m_renBaseComp( m_pixFormatComp ),
    m_renderer( m_renBase ),
    m_renSolidComp( m_renBaseComp ),
    m_rasterizer(),
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
    a2dContext( renderer, drawable.GetWidth(), drawable.GetHeight() )
{
    m_buffer = new a2dImageRGBA( m_width, m_height );
    m_pdata = m_buffer->GetData();
    m_rendering_buffer.attach( m_pdata, m_width, m_height, m_width * 4 );
    ResetClip();
    m_preStroke = true;
}

a2dAggContext::a2dAggContext( wxGraphicsRenderer* renderer, a2dImageRGBA* drawable )
    :
    m_rendering_buffer(),
    m_pixFormat( m_rendering_buffer ),
    m_pixFormatComp( m_rendering_buffer ),
    m_renBase( m_pixFormat ),
    m_renBaseComp( m_pixFormatComp ),
    m_renderer( m_renBase ),
    m_renSolidComp( m_renBaseComp ),
    m_rasterizer(),
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
    a2dContext( renderer, drawable->GetWidth(), drawable->GetHeight() )
{
    m_buffer = drawable;
    m_pdata = m_buffer->GetData();
    m_rendering_buffer.attach( m_pdata, m_width, m_height, m_width * 4 );
    ResetClip();
    m_preStroke = true;
}

a2dAggContext::~a2dAggContext()
{
}

void a2dAggContext::Clip( const wxRegion& region )
{
    // Create a path with all the rectangles in the region
    wxGraphicsPath path = GetRenderer()->CreatePath();
    wxRegionIterator ri( region );
    while ( ri )
    {
        path.AddRectangle( ri.GetX(), ri.GetY(), ri.GetW(), ri.GetH() );
        ri++;
    }
}

void a2dAggContext::Clip( wxDouble x, wxDouble y, wxDouble w, wxDouble h )
{
    //need to calculate  clip in device coordinates from world coordinates
    // not even rectangular in the end, so assume a polygon as result.
    wxPoint trpoints[4];
    double dx, dy;

    m_usertodevice.TransformPoint( x, y, dx, dy );
    trpoints[0] = wxPoint( dx, dy );
    m_usertodevice.TransformPoint( x, y + h, dx, dy );
    trpoints[1] = wxPoint( dx, dy );
    m_usertodevice.TransformPoint( x + w, y + h, dx, dy );
    trpoints[2] = wxPoint( dx, dy );
    m_usertodevice.TransformPoint( x + w, y, dx, dy );
    trpoints[3] = wxPoint( dx, dy );

    double xmin = trpoints[0].x;
    double ymin = trpoints[0].y;
    double xmax = trpoints[0].x;
    double ymax = trpoints[0].y;
    for ( int i = 1; i < 4; i ++ )
    {
        xmin = wxMin( xmin, trpoints[i].x );
        xmax = wxMax( xmax, trpoints[i].x );
        ymin = wxMin( ymin, trpoints[i].y );
        ymax = wxMax( ymax, trpoints[i].y );
    }
    wxRect cliprect = wxRect( xmin, ymin, xmax - xmin, ymax - ymin );
    m_clipboxdev = cliprect;

    m_renBase.clip_box( xmin, ymin, xmax, ymax );
    m_renBaseComp.clip_box( xmin, ymin, xmax, ymax );
    m_rasterizer.clip_box( 0, 0, m_width, m_height );
    // why not the next, does not work??
    //m_rasterizer.clip_box( minx, miny, xmax, ymax );
}

void a2dAggContext::ResetClip()
{
    m_clipboxdev = wxRect( 0, 0, m_width, m_height );
    m_renBase.clip_box( 0, 0, m_width, m_height );
    m_renBaseComp.clip_box( 0, 0, m_width, m_height );
    m_rasterizer.clip_box( 0, 0, m_width, m_height );
}

void a2dAggContext::Render( bool fillColor )
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

void a2dAggContext::RenderGradient( bool radial )
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

void a2dAggContext::RenderBitmapFill()
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

void a2dAggContext::Rotate( wxDouble angle )
{
    m_usertodevice.Rotate( angle ) ;
}

void a2dAggContext::Translate( wxDouble dx , wxDouble dy )
{
    m_usertodevice.Translate( dx, dy ) ;
}

void a2dAggContext::Scale( wxDouble xScale , wxDouble yScale )
{
    m_usertodevice.Scale( xScale, yScale, 0, 0 ) ;
}

// concatenates this transform with the current transform of this context
void a2dAggContext::ConcatTransform( const wxGraphicsMatrix& matrix )
{
    a2dAffineMatrix* m = ( a2dAffineMatrix* ) matrix.GetNativeMatrix();
    m_usertodevice *= *m;
}

// sets the transform of this context
void a2dAggContext::SetTransform( const wxGraphicsMatrix& matrix )
{
    a2dAffineMatrix* m = ( a2dAffineMatrix* ) matrix.GetNativeMatrix();
    m_usertodevice = *m;
}

// gets the matrix of this context
wxGraphicsMatrix a2dAggContext::GetTransform() const
{
    wxGraphicsMatrix matrix = CreateMatrix();
    a2dAffineMatrix* m = ( a2dAffineMatrix* ) matrix.GetNativeMatrix();
    *m = m_usertodevice;
    return matrix;
}

#if wxCHECK_VERSION(2,9,0)
bool a2dAggContext::SetCompositionMode( wxCompositionMode op )
{
    if ( m_composition == op )
        return true;

    m_composition = op;

    switch ( op )
    {
        case wxCOMPOSITION_CLEAR:
            m_blendMode = agg::comp_op_clear;
            break;
        case wxCOMPOSITION_SOURCE:
            m_blendMode = agg::comp_op_src;
            break;
        case wxCOMPOSITION_OVER:
            m_blendMode = agg::comp_op_src_over;
            break;
        case wxCOMPOSITION_IN:
            m_blendMode = agg::comp_op_src_in;
            break;
        case wxCOMPOSITION_OUT:
            m_blendMode = agg::comp_op_src_out;
            break;
        case wxCOMPOSITION_ATOP:
            m_blendMode = agg::comp_op_src_atop;
            break;
        case wxCOMPOSITION_DEST:
            m_blendMode = agg::comp_op_dst;
            break;
        case wxCOMPOSITION_DEST_OVER:
            m_blendMode = agg::comp_op_dst_over;
            break;
        case wxCOMPOSITION_DEST_IN:
            m_blendMode = agg::comp_op_dst_in;
            break;
        case wxCOMPOSITION_DEST_OUT:
            m_blendMode = agg::comp_op_dst_out;
            break;
        case wxCOMPOSITION_DEST_ATOP:
            m_blendMode = agg::comp_op_dst_atop;
            break;
        case wxCOMPOSITION_XOR:
            m_blendMode = agg::comp_op_xor;
            break;
        case wxCOMPOSITION_ADD:
            m_blendMode = agg::comp_op_plus;
            break;
        default:
            m_blendMode = agg::end_of_comp_op_e;
    }
    return true;
}
#endif

void a2dAggContext::SetDrawStyle( a2dDrawStyle drawstyle )
{
    m_drawstyle = drawstyle;

    switch( drawstyle )
    {
        case a2dWIREFRAME_INVERT:
            m_activestroke = *a2dBLACK_STROKE;
            m_activefill = *a2dTRANSPARENT_FILL;
            m_blendMode = agg::comp_op_invert;
            break;

        case a2dWIREFRAME:
            m_activestroke = *a2dBLACK_STROKE;
            m_activefill = *a2dTRANSPARENT_FILL;
            m_blendMode = agg::end_of_comp_op_e;
            break;

        case a2dWIREFRAME_ZERO_WIDTH:
            m_activestroke = *a2dBLACK_STROKE;
            m_activefill = *a2dTRANSPARENT_FILL;
            m_blendMode = agg::end_of_comp_op_e;
            break;

        case a2dWIREFRAME_INVERT_ZERO_WIDTH:
            m_activestroke = *a2dBLACK_STROKE;
            m_activefill = *a2dTRANSPARENT_FILL;
            m_blendMode = agg::comp_op_invert;
            break;

        case a2dFILLED:
            m_blendMode = agg::end_of_comp_op_e;
            break;

        default:
            wxASSERT( 0 );
    }
    m_pixFormatComp.comp_op( m_blendMode );
}

bool a2dAggContext::SetLogicalFunction( int function )
{
    switch( function )
    {
        case wxINVERT:
            m_activestroke = *a2dBLACK_STROKE;
            m_activefill = *a2dTRANSPARENT_FILL;
            m_blendMode = agg::comp_op_invert;
            m_drawstyle = a2dWIREFRAME_INVERT;
            break;
        default:
            m_blendMode = agg::end_of_comp_op_e;
            m_drawstyle = a2dFILLED;
    }

    m_pixFormatComp.comp_op( m_blendMode );
    return true;
}

void a2dAggContext::DoSetActiveStroke()
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
            a2dAffineMatrix invert = m_usertodevice;
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
                a2dAffineMatrix invert = m_usertodevice;
                invert.Invert();
                m_strokewidthDev = m_activestroke.GetWidth();
                m_strokewidthDev = !m_strokewidthDev ? 1 : m_strokewidthDev;
                m_strokewidth = invert.TransformDistance( m_strokewidthDev );
            }
            else
            {
                m_strokewidth = m_activestroke.GetWidth();
                m_strokewidthDev = m_usertodevice.TransformDistance( m_strokewidth );
                if ( !m_strokewidthDev )
                {
                    m_strokewidthDev = 1;
                    a2dAffineMatrix invert = m_usertodevice;
                    invert.Invert();
                    m_strokewidth = invert.TransformDistance( m_strokewidthDev );
                }
            }
        }
    }
}


void a2dAggContext::DoSetActiveFill()
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

#if wxCHECK_VERSION(2,9,0)
void a2dAggContext::DrawBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h )
#else
void a2dAggContext::DrawGraphicsBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h )
#endif
{
    a2dBitmapData* data = static_cast<a2dBitmapData*>( bmp.GetRefData() );
    wxImage image = data->GetImage();

    int imagew = image.GetWidth();
    int imageh = image.GetHeight();

    agg::trans_affine mtxi;
    mtxi *= agg::trans_affine_scaling( w / imagew, h / imageh );
    mtxi *= agg::trans_affine_translation( x - w / 2, y - h / 2 );
    // pictures always upwards
    //if ( m_yaxis )
    //    mtxi *= agg::trans_affine_reflection_unit( 1, 0 );

    agg::trans_affine mtx = _get_agg_user_to_device_transform();
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

    typedef agg::renderer_scanline_aa<RendererBaseA, span_alloc_type, span_conv> renderer_type_alpha;

    typedef agg::renderer_scanline_bin<RendererBaseA, span_alloc_type, span_gen_type> renderer_type_normal;

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
    agg::rounded_rect er( x - w / 2, y - h / 2 , x + w / 2, y + h / 2, 0 );
    er.normalize_radius();
    agg::conv_transform<agg::rounded_rect> tr( er, mtx );
    m_rasterizer.reset();
    m_rasterizer.add_path( tr );

    //NOT m_FillOpacityCol1 since for images fill does not influence the picture.
    if ( m_OpacityFactor != 255 )
    {
        span_conv_const_alpha_rgba8 color_alpha( m_OpacityFactor );
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

void a2dAggContext::GetTextExtent( const wxString& str, wxDouble* width, wxDouble* height,
                                   wxDouble* descent, wxDouble* externalLeading ) const
{
}

void a2dAggContext::GetPartialTextExtents( const wxString& text, wxArrayDouble& widths ) const
{
}

void a2dAggContext::DrawCharDc( wxChar c )
{
#if wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
    if ( m_a2dfont.GetType() == a2dFONT_WXDC && m_a2dfont.GetFreetypeFont().Ok() )
    {
        a2dFont oldfont = m_a2dfont;
        m_a2dfont = m_a2dfont.GetFreetypeFont();
        DrawCharFreetype( c );
        m_a2dfont = oldfont;
    }
    else
        a2dContext::DrawCharDc( c );
#else // wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
    a2dContext::DrawCharDc( c );
#endif // wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)

}

void a2dAggContext::DrawTextDc( const wxString& text, double x, double y )
{
    // Get font size in device units
    unsigned int fontsize;
    double dx = m_usertodevice.GetValue( 1, 0 );
    double dy = m_usertodevice.GetValue( 1, 1 );
    fontsize = ( unsigned int ) fabs( m_a2dfont.GetSize() * sqrt( dx * dx + dy * dy ) );
    if ( fontsize < 1 )
        fontsize = 1;
    m_a2dfont.GetFont().SetPointSize( fontsize );

    //m_renderDC->SetFont( m_a2dfont.GetFont() );
    //m_renderDC->SetBackgroundMode( wxTRANSPARENT );
    //m_renderDC->SetTextForeground( m_stroke1 );

    DrawTextGeneric( text, x, y, ( void ( a2dContext::* )( wxChar ) ) & a2dAggContext::DrawCharDc );
}

void a2dAggContext::DrawCharStroke( wxChar c )
{
    agg::path_storage path;
    double size = m_a2dfont.GetSize();
    a2dVertexList** ptr = m_a2dfont.GetGlyphStroke( c );
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
    agg::trans_affine mtx = _get_agg_user_to_device_transform();
    agg::conv_transform< agg::path_storage > tr( path, mtx );
    agg::conv_stroke< agg::conv_transform< agg::path_storage > > stroke( tr );

    agg::rgba8 color( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
    stroke.line_join( m_join );
    stroke.line_cap( m_cap );
    stroke.width( m_a2dfont.GetStrokeWidth() );
    m_rasterizer.reset();
    m_rasterizer.add_path( stroke );
    m_renderer.color( color );
    m_renSolidComp.color( color );
    if( m_blendMode == agg::end_of_comp_op_e )
        agg::render_scanlines( m_rasterizer, m_sl, m_renderer );
    else
        agg::render_scanlines( m_rasterizer, m_sl, m_renSolidComp );
}

#if wxART2D_USE_FREETYPE
extern FT_Library g_freetypeLibrary;

//! used in freetype rendering
typedef struct
{
    wxColour colour;
    int xmin, ymax;
    int stride;
    unsigned char* buf;
}
a2dSpanData;

//! used in freetype rendering
static void a2dSpanFuncGray( int y, int count, FT_Span* spans, a2dSpanData* user )
{
    unsigned int alpha, invalpha, len;
    // although colours are chars, using ints is faster in multiplications.
    unsigned int r, g, b, rpm, gpm, bpm;
    r = user->colour.Red();
    g = user->colour.Green();
    b = user->colour.Blue();

    unsigned char* buf, *buf2;
    buf = user->buf + ( user->ymax - y ) * user->stride - user->xmin * 3;
    do
    {
        buf2 = buf + spans->x * 3;
        len = spans->len;
        alpha = spans->coverage;
        if ( alpha == 255 )
        {
            do
            {
                // do not try to replace by *buf2++ = r, etc.
                // it turns out that this code actually is faster (for x86).
#ifdef __WXGTK__
                buf2[0] = r;
                buf2[1] = g;
                buf2[2] = b;
#else
                buf2[0] = b;
                buf2[1] = g;
                buf2[2] = r;
#endif
                buf2 += 3;
            }
            while( --len );
        }
        else
        {
            rpm = r * alpha;
            gpm = g * alpha;
            bpm = b * alpha;
            invalpha = 255 - alpha;
            do
            {
                // do not try to replace by *buf2++ = ..., etc.
                // it turns out that this code actually is faster (for x86).
#ifdef __WXGTK__
                buf2[0] = ( buf2[0] * invalpha + rpm ) / 255;
                buf2[1] = ( buf2[1] * invalpha + gpm ) / 255;
                buf2[2] = ( buf2[2] * invalpha + bpm ) / 255;
#else
                buf2[0] = ( buf2[0] * invalpha + bpm ) / 255;
                buf2[1] = ( buf2[1] * invalpha + gpm ) / 255;
                buf2[2] = ( buf2[2] * invalpha + rpm ) / 255;
#endif
                buf2 += 3;
            }
            while( --len );
        }
        spans++;
    }
    while ( --count );
}
#endif

//! \bug EW: DrawCharFreetype crashes when zoomed in extremely far.
void a2dAggContext::DrawCharFreetype( wxChar c )
{
    double x, y;
    x = y = 0.0;

#if wxART2D_USE_FREETYPE
    y += m_a2dfont.GetDescent();

    agg::path_storage path;
    double scale = m_a2dfont.GetSize() / ( 64 * m_a2dfont.GetDeviceHeight() );

    FT_Glyph glyph = m_a2dfont.GetGlyphFreetype( c )->m_glyph;
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
    affine = m_usertodevice * affine;

    agg::trans_affine mtx( affine.GetValue( 0, 0 ), affine.GetValue( 0, 1 ),
                           affine.GetValue( 1, 0 ), affine.GetValue( 1, 1 ),
                           affine.GetValue( 2, 0 ), affine.GetValue( 2, 1 ) );
    agg::conv_transform< agg::path_storage > tr( path, mtx );
    agg::conv_curve< agg::conv_transform< agg::path_storage > > stroke( tr );

    agg::rgba8 color( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
    m_rasterizer.reset();
    m_rasterizer.add_path( stroke );
    m_renderer.color( color );
    m_renSolidComp.color( color );
    if( m_blendMode == agg::end_of_comp_op_e )
        agg::render_scanlines( m_rasterizer, m_sl, m_renderer );
    else
        agg::render_scanlines( m_rasterizer, m_sl, m_renSolidComp );

#else // wxART2D_USE_FREETYPE
    a2dContext::DrawCharFreetype( c );
#endif // wxART2D_USE_FREETYPE
}

void a2dAggContext::DrawRoundedRectangle( wxDouble x, wxDouble y, wxDouble width, wxDouble height, wxDouble radius )
{
    if ( width == 0 || height == 0 )
        return;

    if ( m_pen.IsNull() && m_brush.IsNull() )
        return;

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

void a2dAggContext::DrawEllipse( wxDouble x, wxDouble y, wxDouble w, wxDouble h )
{
    if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR || m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        a2dContext::DrawEllipse( x, y, w, h );
        return;
    }

    if ( m_pen.IsNull() && m_brush.IsNull() )
        return;

    double dphi;
    unsigned int segments = 20;

    //circular approximation of radius.
    double radiusDev = m_usertodevice.TransformDistance( wxMax( w, h ) );
    Aberration( m_displayaberration, wxPI * 2, radiusDev , dphi, segments );

    agg::trans_affine mtx = _get_agg_user_to_device_transform();

    agg::rgba8 color( m_colour1redFill, m_colour1greenFill, m_colour1blueFill, m_FillOpacityCol1 );
    agg::ellipse ellipse( x + w / 2.0, y + h / 2.0, w / 2.0, h / 2.0, segments );

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

void a2dAggContext::DrawPolygon( const a2dVertexList* list, wxPolygonFillMode fillStyle )
{
    if ( m_pen.IsNull() && m_brush.IsNull() )
        return;

    int segments = ToAggPath( list, false );

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

void a2dAggContext::DrawLines( const a2dVertexList* list )
{
    if ( m_pen.IsNull() )
        return;

    int segments = ToAggPath( list, false );

    if ( segments == 0 )
        return;

    agg::trans_affine mtx = _get_agg_user_to_device_transform();
    m_rasterizer.reset();
    _ras_add_stroked_path_xform( m_path, mtx );
    Render( false );
}

int a2dAggContext::ToAggPath( a2dVertexArray* points, bool transform )
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

int a2dAggContext::ToAggPath( const a2dVertexList* list, bool transform )
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

/*******************************************************************
a2dGcAggDrawer
********************************************************************/
IMPLEMENT_DYNAMIC_CLASS( a2dGcAggDrawer, a2dGcBaseDrawer )

void a2dGcAggDrawer::InitContext()
{
    m_clip.Clear();
    m_buffer = a2dImageRGBA( m_width, m_height );
    m_pdata = m_buffer.GetData();
    m_render = new a2dRenderer();
    m_context = new a2dAggContext( m_render, &m_buffer );
}

a2dGcAggDrawer::a2dGcAggDrawer( const wxSize& size ): a2dGcBaseDrawer( size.GetWidth(), size.GetHeight() )
{
    InitContext();
}

a2dGcAggDrawer::a2dGcAggDrawer( int width, int height ): a2dGcBaseDrawer( width, height )
{
    InitContext();
}

a2dGcAggDrawer::a2dGcAggDrawer( const a2dGcAggDrawer& other )
    : a2dGcBaseDrawer( other )
{
    InitContext();
}

a2dGcAggDrawer::a2dGcAggDrawer( const a2dDrawer2D& other )
    : a2dGcBaseDrawer( other )
{
    InitContext();
}

a2dGcAggDrawer::~a2dGcAggDrawer()
{
    delete m_context;
    delete m_render;
}

void a2dGcAggDrawer::SetYaxis( bool up )
{
    a2dDrawer2D::SetYaxis( up );
    //static_cast<a2dAggContext*>(m_context)->SetYaxis( up );
}

wxBitmap a2dGcAggDrawer::GetBuffer() const
{
    return wxBitmap( m_buffer.GetImage() );
}

void a2dGcAggDrawer::SetBufferSize( int w, int h )
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
    DestroyClippingRegion();
    delete m_context;
    m_context = new a2dAggContext( m_render, &m_buffer );
}

wxBitmap a2dGcAggDrawer::GetSubBitmap( wxRect rect ) const
{
    return wxBitmap( m_buffer.GetImage().GetSubImage( rect ) );
}

void a2dGcAggDrawer::CopyIntoBuffer( const wxBitmap& bitm )
{
    m_buffer = a2dImageRGBA( bitm.ConvertToImage() );
}

void a2dGcAggDrawer::BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos )
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

void a2dGcAggDrawer::ShiftBuffer( int dxy, bool yshift )
{
    if ( yshift )
    {
        int pixelwidth = m_width * 4;
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
        int pixelwidth = m_width * 4;
        if ( dxy > 0 && dxy < m_width )
        {
            int subwidth = ( m_width - dxy ) * 4;
            unsigned char* low = m_pdata;
            unsigned char* high = m_pdata + dxy * 4 ;

            for ( int yp = 0; yp < m_height  ; yp++ )
            {
                memcpy( high, low, subwidth );
                high += pixelwidth;
                low += pixelwidth;
            }
        }
        else if ( dxy < 0 && dxy > -m_width )
        {
            dxy = -dxy;
            int subwidth = ( m_width - dxy ) * 4;
            unsigned char* low = m_pdata;
            unsigned char* high = m_pdata + dxy * 4 ;

            for ( int yp = 0; yp < m_height  ; yp++ )
            {
                memcpy( low, high, subwidth );
                high += pixelwidth;
                low += pixelwidth;
            }
        }
        else
            wxFAIL_MSG( wxT( "you can only shift within width of buffer" ) );
    }
}

void a2dGcAggDrawer::DoSetActiveStroke()
{
    if ( 1 )
    {
        // a2dRenderer render class does understand a2dStroke and a2dFill,
        // so we will have a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT available, else lost.
        a2dRenderer* render = ( a2dRenderer* ) m_render;
        a2dContext* context = ( a2dContext* ) m_context;
        wxGraphicsPen p = render->CreateStroke( m_activestroke  );
        m_context->SetPen( p );
    }
    else
    {
        a2dGcBaseDrawer::DoSetActiveStroke();
    }
}

void a2dGcAggDrawer::DoSetActiveFill()
{
    if ( 1 )
    {
        // a2dRenderer render class does understand a2dStroke and a2dFill,
        // so we will have a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT available, else lost.
        a2dRenderer* render = ( a2dRenderer* ) m_render;
        a2dContext* context = ( a2dContext* ) m_context;
        wxGraphicsBrush b = render->CreateFill( m_activefill );
        m_context->SetBrush( b );
    }
    else
    {
        a2dGcBaseDrawer::DoSetActiveFill();
    }
}

void a2dGcAggDrawer::DoSetActiveFont( const a2dFont& font )
{
    a2dContext* context = ( a2dContext* ) m_context;
    a2dRenderer* render = ( a2dRenderer* ) m_render;
    wxGraphicsFont f = render->CreateFont( font );
    context->SetFont( font );
}

void a2dGcAggDrawer::DrawPoint( double xc, double yc )
{
    if ( m_disableDrawing )
        return;

    double xt, yt;
    GetUserToDeviceTransform().TransformPoint( xc, yc, xt, yt );
    //DeviceDrawPixel( xt, yt, m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
}

void a2dGcAggDrawer::DrawImage( const wxImage& imagein, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    a2dAffineMatrix affine;
    if ( GetYaxis() )
    {
        affine.Translate( 0.0, -( y ) );
        affine.Mirror( true, false );
        affine.Translate( 0.0, y );
    }
    PushTransform( affine );

    a2dContext* context = ( a2dContext* ) m_context;
    a2dRenderer* render = ( a2dRenderer* ) m_render;
    wxGraphicsBitmap bitmap = render->CreateBitmap( imagein );
#if wxCHECK_VERSION(2,9,0)
    m_context->DrawBitmap( bitmap, x, y, width, height );
#else
    context->DrawGraphicsBitmap( bitmap, x, y, width, height );
#endif

    PopTransform();
}

void a2dGcAggDrawer::DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    a2dContext* context = ( a2dContext* ) m_context;
    a2dRenderer* render = ( a2dRenderer* ) m_render;
    wxImage wximage = image.GetImage();
    wxGraphicsBitmap bitmap = render->CreateBitmap( wximage );
#if wxCHECK_VERSION(2,9,0)
    m_context->DrawBitmap( bitmap, x - width / 2.0, y - height / 2.0, width, height );
#else
    context->DrawGraphicsBitmap( bitmap, x - width / 2.0, y - height / 2.0, width, height );
#endif
}

/* HANDLED IN m_context
void a2dGcAggDrawer::DrawCharDc( wxChar c )
{
    wxGraphicsMatrix m = m_context->CreateMatrix();
    m.Set(
        m_usertodevice( 0, 0), m_usertodevice( 0, 1),
        m_usertodevice( 1, 0), m_usertodevice( 1, 1),
        m_usertodevice( 2, 0), m_usertodevice( 2, 1) );
    m_context->SetTransform( m );

    static_cast<a2dAggContext*>(m_context)->DrawCharDc( c );
}

//! \bug EW: DrawCharFreetype crashes when zoomed in extremely far.
void a2dGcAggDrawer::DrawCharFreetype( wxChar c )
{
    wxGraphicsMatrix m = m_context->CreateMatrix();
    m.Set(
        m_usertodevice( 0, 0), m_usertodevice( 0, 1),
        m_usertodevice( 1, 0), m_usertodevice( 1, 1),
        m_usertodevice( 2, 0), m_usertodevice( 2, 1) );
    m_context->SetTransform( m );
    static_cast<a2dAggContext*>(m_context)->DrawCharFreetype( c );
}
*/


#endif  // wxART2D_USE_GRAPHICS_CONTEXT

#endif //wxART2D_USE_AGGDRAWER 

