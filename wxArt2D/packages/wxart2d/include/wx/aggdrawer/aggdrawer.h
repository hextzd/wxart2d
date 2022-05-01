/*! \file wx/aggdrawer/aggdrawer.h

    \brief the antigrain based drawing context derived from a2dDrawer2D

    This drawing context does anti aliased and alpha drawing. And uses a wxImage as a buffer.
    A strong feature is the ability to draw (rotated) wxImage's

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: aggdrawer.h,v 1.43 2009/10/06 18:40:31 titato Exp $
*/

#ifndef __WXAGGDRAWER_H__
#define __WXAGGDRAWER_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// where we keep all our configuration symbol (wxART2D_USE_*)
#include "a2dprivate.h"

#if wxART2D_USE_AGGDRAWER

#include "wx/artbase/drawer2d.h"
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_path_storage.h"
#include "agg_conv_stroke.h"
#include "wx/image.h"
#include "wx/geometry.h"

#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgba.h"
#include "agg_image_accessors.h"
#include "agg_span_allocator.h"
#include "agg_conv_dash.h"
#include "agg_pixfmt_rgba.h"
#include "agg_span_image_filter_rgba.h"
//#include "agg_affine_matrix.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_outline_aa.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_conv_transform.h"
#include "agg_bspline.h"
#include "agg_gsv_text.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"
#include "agg_renderer_mclip.h"

#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_conv_curve.h"
#include "agg_conv_contour.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_conv_marker.h"
#include "agg_arrowhead.h"
#include "agg_vcgen_markers_term.h"
#include "agg_rendering_buffer.h"
#include "agg_arc.h"
#include "agg_ellipse.h"
#include "agg_rounded_rect.h"
#include "agg_path_storage.h"
#include "agg_span_pattern_rgb.h"
#include "agg_trans_affine.h"
#include "agg_span_image_filter_rgb.h"
#include "agg_span_gradient.h"
#include "agg_span_interpolator_linear.h"
#include "agg_span_converter.h"

#ifdef A2DAGGDRAWERMAKINGDLL
#define A2DAGGDRAWERDLLEXP WXEXPORT
#define A2DAGGDRAWERDLLEXP_DATA(type) WXEXPORT type
#define A2DAGGDRAWERDLLEXP_CTORFN
#elif defined(WXART2D_USINGDLL)
#define A2DAGGDRAWERDLLEXP WXIMPORT
#define A2DAGGDRAWERDLLEXP_DATA(type) WXIMPORT type
#define A2DAGGDRAWERDLLEXP_CTORFN
#else // not making nor using DLL
#define A2DAGGDRAWERDLLEXP
#define A2DAGGDRAWERDLLEXP_DATA(type) type
#define A2DAGGDRAWERDLLEXP_CTORFN
#endif

#if defined(WXART2D_USINGDLL)
template struct A2DAGGDRAWERDLLEXP agg::rect_base<int>;

template class A2DAGGDRAWERDLLEXP agg::pod_array<agg::int8u*>;
template class A2DAGGDRAWERDLLEXP agg::row_ptr_cache<agg::int8u>;

template class A2DAGGDRAWERDLLEXP agg::rasterizer_sl_clip<agg::ras_conv_int>;
template class A2DAGGDRAWERDLLEXP agg::pod_vector<agg::rasterizer_cells_aa<agg::cell_aa>::cell_type*>;
template class A2DAGGDRAWERDLLEXP agg::pod_vector<agg::rasterizer_cells_aa<agg::cell_aa>::sorted_y>;
template class A2DAGGDRAWERDLLEXP agg::rasterizer_cells_aa<agg::cell_aa>;
template class A2DAGGDRAWERDLLEXP agg::rasterizer_scanline_aa<>;

template class A2DAGGDRAWERDLLEXP agg::row_accessor<agg::int8u>;

class A2DAGGDRAWERDLLEXP agg::scanline_u8;
#endif

typedef agg::order_rgba wxImageComponentOrder;

//! color format in a2dAggDrawer
typedef agg::rgba8 color_type;
typedef agg::pixfmt_rgb24 PixFormat;
typedef agg::blender_rgba<color_type, wxImageComponentOrder>             Blender;
//! base rendering Agg
typedef agg::renderer_base<PixFormat> RendererBase;
//! solid rendering on top of base rendering Agg
typedef agg::renderer_scanline_aa_solid<RendererBase> RendererSolidAa;
// or this one for non aliased, but still alpha, 4 times faster
typedef agg::renderer_scanline_bin_solid<RendererBase> RendererSolidBin;

typedef agg::renderer_outline_aa<RendererBase> RendererOutlineAa;
typedef agg::rasterizer_outline_aa<RendererOutlineAa> RasterOutLineAa;

#include "agg_pixfmt_rgba.h"

//! color format in a2dAggDrawer
#if defined(__WXMSW__)
typedef agg::pixfmt_bgra32 PixFormatA;
typedef agg::order_bgra ComponentOrder; // Platform dependent!
#else
typedef agg::pixfmt_rgba32 PixFormatA;
typedef agg::order_rgba ComponentOrder; // Platform dependent!
#endif
typedef agg::comp_op_adaptor_rgba<color_type, ComponentOrder>     BlenderCompA;
typedef agg::pixfmt_custom_blend_rgba<BlenderCompA, agg::rendering_buffer> PixFormatComp;
//! base rendering Agg
typedef agg::renderer_base<PixFormatA> RendererBaseA;
typedef agg::renderer_base<PixFormatComp>    RendererBaseComp;
//! solid rendering on top of base rendering Agg
typedef agg::renderer_scanline_aa_solid<RendererBaseA> RendererSolidAaA;
typedef agg::renderer_scanline_aa_solid<RendererBaseComp> RendererSolidComp;
// or this one for non aliased, but still alpha, 4 times faster
typedef agg::renderer_scanline_bin_solid<RendererBase> RendererSolidBin;

typedef agg::renderer_outline_aa<RendererBaseA> RendererOutlineAaA;
typedef agg::rasterizer_outline_aa<RendererOutlineAaA> RasterOutLineAaA;

typedef agg::pod_auto_array<color_type, 256> GradientArray;

typedef agg::span_gradient<color_type, agg::span_interpolator_linear<>, agg::gradient_x,      GradientArray> LinearGradientSpan;
typedef agg::span_gradient<color_type, agg::span_interpolator_linear<>, agg::gradient_circle, GradientArray> RadialGradientSpan;

typedef agg::span_allocator<agg::rgba8> span_allocator_type;

typedef agg::renderer_scanline_aa < RendererBase,
        span_allocator_type,
        LinearGradientSpan > RendererLinearGradient;
typedef agg::renderer_scanline_aa < RendererBase,
        span_allocator_type,
        RadialGradientSpan > RendererRadialGradient;

typedef agg::renderer_scanline_aa < RendererBaseA,
        span_allocator_type,
        LinearGradientSpan > RendererLinearGradientA;
typedef agg::renderer_scanline_aa < RendererBaseA,
        span_allocator_type,
        RadialGradientSpan > RendererRadialGradientA;

typedef agg::renderer_scanline_aa < RendererBaseComp,
        span_allocator_type,
        LinearGradientSpan > RendererLinearGradientComp;
typedef agg::renderer_scanline_aa < RendererBaseComp,
        span_allocator_type,
        RadialGradientSpan > RendererRadialGradientComp;

typedef agg::rasterizer_scanline_aa<> RasterizerType;
typedef RendererSolidAa RendererType;
// Non Aliased drawing, not that good, but you can try it.
//typedef RendererSolidBin RendererType;


/*
noop_conv_transform: this is a small helper class that can stand in for an
agg::conv_transform.  It introduces little overhead (and possibly none after
compiler optimizations), but allows us to consolidate the code for processing
paths into strokes in one place.
*/
template<class VertexSource> class noop_conv_transform
{
public:
    noop_conv_transform( VertexSource& source, int ) : m_source( source ) {}

    void rewind( unsigned path_id )
    {
        m_source.rewind( path_id );
    }

    unsigned vertex( double* x, double* y )
    {
        return m_source.vertex( x, y );
    }

    void transformer( void* ) {}

private:
    noop_conv_transform( const noop_conv_transform<VertexSource>& );
    const noop_conv_transform<VertexSource>&
    operator = ( const noop_conv_transform<VertexSource>& );
    VertexSource&      m_source;
};

class A2DAGGDRAWERDLLEXP a2dAggDrawerBase : public a2dDrawer2D
{

public:

    a2dAggDrawerBase( int width = 0, int height = 0 );

    //! Drawer having a buffer of w pixel wide and h pixels height given by size
    a2dAggDrawerBase( const wxSize& size );

    //! bitmap is converted to image on which you can draw.
    //! Use GetBuffer() to get a bitmap back.
    a2dAggDrawerBase( const wxBitmap& bitmap );

    //! copy constructor
    a2dAggDrawerBase( const a2dAggDrawerBase& other );

    bool HasAlpha() { return true; }

    void SetPreStroke( bool preStroke ) { m_preStroke = preStroke; }

    bool GetPreStroke() { return m_preStroke; }

    void ResetStyle();

    void SetClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    virtual void ExtendClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    virtual void ExtendAndPushClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    void PopClippingRegion();

    void SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height );

    void SetClippingRegion( double minx, double miny, double maxx, double maxy );

    void DestroyClippingRegion();

    void BlitBuffer( wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 )  );

    void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 ) ) = 0;

    virtual void DrawCircle( double x, double y, double radius );

    void DrawPoint(  double xc,  double yc );

    //!start to draw on this context (used to initialize a specific drawer)
    virtual void BeginDraw();

    //!end drawing on this context (used to reset a specific drawer)
    virtual void EndDraw();

    wxDC* GetDeviceDC() const { return m_deviceDC; }

    virtual void DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize = false );

    virtual void DrawEllipse( double x, double y, double width, double height );

    //!Draw polygon in world coordinates using pointarray
    virtual void DrawPolygon( a2dVertexArray* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    //!Draw polygon in world coordinates using pointlist
    virtual void DrawPolygon( const a2dVertexList* list, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    //! draw a list of polygons ( contour clockwise becomes hole )
    virtual void DrawPolyPolygon( a2dListOfa2dVertexList polylist, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    //!Draw polyline in world coordinates using pointarray
    virtual void DrawLines( a2dVertexArray* points, bool spline = false );

    //!Draw polyline in world coordinates using pointlist
    virtual void DrawLines( const a2dVertexList* list, bool spline = false );

protected:

    // Documented in base class
    virtual void DrawCharDc( wxChar c );
    // Documented in base class
    virtual void DrawCharStroke( wxChar c );
    // Documented in base class
    virtual void DrawCharFreetype( wxChar c );

    void ShiftBufferInternal( int dxy, bool yshift, int bytesPerPixel );

    template <class PathT>
    void _ras_add_stroked_path_xform( PathT& path, const agg::trans_affine& mtx );

    virtual void Render( bool fillColor ) = 0;
    virtual void RenderGradient( bool radial ) = 0;
    virtual void RenderBitmapFill() = 0;
    virtual void RenderSetClip() = 0;

    agg::trans_affine _get_agg_user_to_device_transform() const;

    void DoSetDrawStyle( a2dDrawStyle drawstyle );

    void DoSetActiveStroke();

    void DoSetActiveFill();

    int ToAggPath( a2dVertexArray* points, bool transform );
    int ToAggPath( const a2dVertexList* list, bool transform );

    bool m_preStroke;

    wxRegion m_clip;

    wxClientDC*  m_deviceDC;

    bool m_ownsBuffer;

    RasterizerType    m_rasterizer;
    agg::comp_op_e    m_blendMode;

    double m_strokewidth;
    double m_strokewidthDev;
    a2dStrokeStyle m_style;

    agg::line_join_e m_join;

    agg::line_cap_e m_cap;

    wxImage m_pattern;
    agg::rendering_buffer m_pattern_rbuf;

    agg::scanline_u8 m_sl;

    agg::path_storage m_path;

    double                          m_fillGradientD1;
    double                          m_lineGradientD1;
    double                          m_fillGradientD2;
    double                          m_lineGradientD2;
    GradientArray                   m_fillGradient;
    GradientArray                   m_lineGradient;
    agg::trans_affine               m_fillGradientMatrix;
    agg::trans_affine               m_lineGradientMatrix;
    agg::span_interpolator_linear<> m_fillGradientInterpolator;
    agg::span_interpolator_linear<> m_lineGradientInterpolator;

    agg::gradient_x                 m_linearGradientFunction;
    agg::gradient_circle            m_radialGradientFunction;

    unsigned char* m_pdata;
};

//! the antigrain based drawing context derived from a2dDrawer2D
/*!
    This drawing context does anti aliased and alpha drawing. And uses a wxImage as a buffer.
    A strong feature is the ability to draw (rotated) wxImage's

    \ingroup drawer
*/
class A2DAGGDRAWERDLLEXP a2dAggDrawer : public a2dAggDrawerBase
{
    DECLARE_DYNAMIC_CLASS( a2dAggDrawer )

public:

    //! Drawer having a buffer of w pixel wide and h pixels heigh
    a2dAggDrawer( int width = 0, int height = 0 );

    //! Drawer having a buffer of w pixel wide and h pixels height given by size
    a2dAggDrawer( const wxSize& size );

    //! bitmap is converted to image on which you can draw.
    //! Use GetBuffer() to get a bitmap back.
    a2dAggDrawer( const wxBitmap& bitmap );

    //! copy constructor
    a2dAggDrawer( const a2dAggDrawer& other );

    //!get buffer as bitmap pointer
    wxBitmap GetBuffer() const;

    //! return wxImage which is used as buffer here
    wxImage& GetImageBuffer() { return m_buffer; }

    //!set buffer size to w pixel wide and h pixels heigh
    void SetBufferSize( int w, int h );

    //!get part of the buffer given a rect
    wxBitmap GetSubBitmap( wxRect sub_rect ) const;

    void CopyIntoBuffer( const wxBitmap& bitm );

    virtual ~a2dAggDrawer();

    void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 ) );

    void ShiftBuffer( int dxy, bool yshift );

    void DrawImage( const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

protected:

    void DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a );

    void Render( bool fillColor );
    virtual void RenderGradient( bool radial );
    virtual void RenderBitmapFill();
    virtual void RenderSetClip();

    wxImage m_buffer;

    bool m_needbufferupdate;

private:

    PixFormat        m_pixFormat;
    RendererBase     m_renBase;
    RendererType     m_renderer;

    agg::rendering_buffer m_rendering_buffer;

};

//! the antigrain based drawing context derived from a2dDrawer2D
/*!
    This drawing context does anti aliased and alpha drawing. And uses a a2dImageRGBA as a buffer.
    A strong feature is the ability to draw (rotated) wxImage's

    \ingroup drawer
*/
class A2DAGGDRAWERDLLEXP a2dAggDrawerRgba : public a2dAggDrawerBase
{
    DECLARE_DYNAMIC_CLASS( a2dAggDrawerRgba )

public:

    //! Drawer having a buffer of w pixel wide and h pixels heigh
    a2dAggDrawerRgba( int width = 0, int height = 0 );

    //! Drawer having a buffer of w pixel wide and h pixels height given by size
    a2dAggDrawerRgba( const wxSize& size );

    //! bitmap is converted to image on which you can draw.
    //! Use GetBuffer() to get a bitmap back.
    a2dAggDrawerRgba( const wxBitmap& bitmap );

    //! copy constructor
    a2dAggDrawerRgba( const a2dAggDrawerRgba& other );

    //!get buffer as bitmap pointer
    wxBitmap GetBuffer() const;

    //! return wxImage which is used as buffer here
    wxImage GetImageBuffer() const;

    //!set buffer size to w pixel wide and h pixels heigh
    void SetBufferSize( int w, int h );

    //!get part of the buffer given a rect
    wxBitmap GetSubBitmap( wxRect sub_rect ) const;

    void CopyIntoBuffer( const wxBitmap& bitm );

    virtual ~a2dAggDrawerRgba();

    void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 ) );

    void ShiftBuffer( int dxy, bool yshift );

    void DrawImage( const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

protected:

    void DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a );

    void DoSetDrawStyle( a2dDrawStyle drawstyle );

    void Render( bool fillColor );
    virtual void RenderGradient( bool radial );
    virtual void RenderBitmapFill();
    virtual void RenderSetClip();

    a2dImageRGBA m_buffer;

    bool m_needbufferupdate;

private:

    PixFormatA       m_pixFormat;
    PixFormatComp    m_pixFormatComp;
    RendererBaseA    m_renBase;
    RendererBaseComp m_renBaseComp;
    RendererSolidComp m_renSolidComp;
    RendererSolidAaA  m_renderer;

    agg::rendering_buffer m_rendering_buffer;

};

namespace
{

//! converter of an image to a certain alpha
class span_conv_const_alpha_rgba8
{
public:
    typedef agg::rgba8 color_type;
    typedef agg::int8u alpha_type;

    span_conv_const_alpha_rgba8( alpha_type a ) :
        m_alpha( a ) {}

    void alpha( alpha_type a ) { m_alpha = a; }
    alpha_type alpha() const { return m_alpha; }

    void prepare() {}
    void generate( color_type* colors,  int x, int y, unsigned len ) const
    {
        do
        {
            colors->a = colors->a * m_alpha / 255 ;
            ++colors;
        }
        while( --len );
    }

private:
    alpha_type m_alpha;
};

//! converter of an image to a certain alpha
class span_conv_array_alpha_rgba8
{
public:
    typedef agg::rgba8 color_type;
    typedef agg::int8u alpha_type;

    span_conv_array_alpha_rgba8( const wxImage& image, alpha_type a ) :
        m_alpha( a ), m_image( image ) {}

    void prepare() {}
    void generate( color_type* colors,  int x, int y, unsigned len ) const
    {
        do
        {
            colors->a = m_image.GetAlpha( x++, y ) * m_alpha / 255;
            ++colors;
        }
        while( --len );
    }

private:
    alpha_type m_alpha;
    wxImage m_image;
};


}


namespace agg
{

//========================================================================
template<> struct gradient_linear_color<rgba8>
{
    typedef rgba8 color_type;

    gradient_linear_color() {}
    gradient_linear_color( const color_type& c1, const color_type& c2 ) :
        m_c1( c1 ), m_c2( c2 ) {}

    color_type operator [] ( unsigned v ) const
    {
        color_type c;
        c.r = ( int8u )( ( ( ( m_c2.r - m_c1.r ) * int( v ) ) + ( m_c1.r << 8 ) ) >> 8 );
        c.g = ( int8u )( ( ( ( m_c2.g - m_c1.g ) * int( v ) ) + ( m_c1.g << 8 ) ) >> 8 );
        c.b = ( int8u )( ( ( ( m_c2.b - m_c1.b ) * int( v ) ) + ( m_c1.b << 8 ) ) >> 8 );
        c.a = ( int8u )( ( ( ( m_c2.a - m_c1.a ) * int( v ) ) + ( m_c1.a << 8 ) ) >> 8 );
        return c;
    }

    void colors( const color_type& c1, const color_type& c2 )
    {
        m_c1 = c1;
        m_c2 = c2;
    }

    color_type m_c1;
    color_type m_c2;
};

//========================================================span_pattern_a2d
template<class Source> class span_pattern_a2d
{
public:
    typedef Source source_type;
    typedef typename source_type::color_type color_type;
    typedef typename source_type::order_type order_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    //--------------------------------------------------------------------
    span_pattern_a2d() {}
    span_pattern_a2d( source_type& src,
                      unsigned offset_x, unsigned offset_y ) :
        m_src( &src ),
        m_offset_x( offset_x ),
        m_offset_y( offset_y ),
        m_alpha( color_type::base_mask )
    {}

    //--------------------------------------------------------------------
    void   attach( source_type& v )      { m_src = &v; }
    source_type& source()       { return *m_src; }
    const  source_type& source() const { return *m_src; }

    //--------------------------------------------------------------------
    void       offset_x( unsigned v ) { m_offset_x = v; }
    void       offset_y( unsigned v ) { m_offset_y = v; }
    unsigned   offset_x() const { return m_offset_x; }
    unsigned   offset_y() const { return m_offset_y; }
    void       alpha( value_type v ) { m_alpha = v; }
    value_type alpha() const { return m_alpha; }

    //--------------------------------------------------------------------
    void prepare() {}
    void generate( color_type* span, int x, int y, unsigned len )
    {
        x += m_offset_x;
        y += m_offset_y;
        const value_type* p = ( const value_type* )m_src->span( x, y, len );
        do
        {
            span->r = p[order_type::R];
            span->g = p[order_type::G];
            span->b = p[order_type::B];
            if ( span->r == 255 && span->g == 255 && span->b == 255 )
                span->a = 0;
            else
                span->a = m_alpha;
            p = m_src->next_x();
            ++span;
        }
        while( --len );
    }

private:
    source_type* m_src;
    unsigned     m_offset_x;
    unsigned     m_offset_y;
    value_type   m_alpha;

};
}

class plain_rect
{
public:
    plain_rect() {}
    plain_rect( double x1, double y1, double x2, double y2 ) :
        m_x1( x1 ), m_y1( y1 ), m_x2( x2 ), m_y2( y2 )
    {
        if( x1 > x2 ) { m_x1 = x2; m_x2 = x1; }
        if( y1 > y2 ) { m_y1 = y2; m_y2 = y1; }
    }

    void rect( double x1, double y1, double x2, double y2 )
    {
        m_x1 = x1; m_y1 = y1; m_x2 = x2; m_y2 = y2;
        if( x1 > x2 ) { m_x1 = x2; m_x2 = x1; }
        if( y1 > y2 ) { m_y1 = y2; m_y2 = y1; }
    }
    void rewind( unsigned ) { m_status = 0;}
    unsigned vertex( double* x, double* y )
    {
        switch( m_status )
        {
            case 0:
                *x = m_x1; *y = m_y1;
                m_status++;
                return agg::path_cmd_move_to;

            case 1:
                *x = m_x2; *y = m_y1;
                m_status++;
                return agg::path_cmd_line_to;

            case 2:
                *x = m_x2; *y = m_y2;
                m_status++;
                return agg::path_cmd_line_to;

            case 3:
                *x = m_x1; *y = m_y2;
                m_status++;
                return agg::path_cmd_line_to;

            case 4:
                *x = m_x1; *y = m_y1;
                m_status++;
                return agg::path_cmd_line_to;

            case 5:
                m_status++;
                return agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw;
        }
        return agg::path_cmd_stop;
    }
private:
    double m_x1;
    double m_y1;
    double m_x2;
    double m_y2;
    unsigned m_status;
};


#endif

#endif
