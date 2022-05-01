/*! \file wx/aggdrawer/aggdrawer.h

    \brief the antigrain based drawing context derived from a2dDrawer2D

    This drawing context does anti aliased and alpha drawing. And uses a wxImage as a buffer.
    A strong feature is the ability to draw (rotated) wxImage's

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: graphicagg.h,v 1.11 2009/10/06 18:40:31 titato Exp $
*/

#ifndef __WXAGGGRAPHICS_H__
#define __WXAGGGRAPHICS_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// where we keep all our configuration symbol (wxART2D_USE_*)
#include "a2dprivate.h"

#if wxART2D_USE_GRAPHICS_CONTEXT
#if wxART2D_USE_AGGDRAWER

#include "wx/artbase/artglob.h"
#include "wx/artbase/stylebase.h"
#include "wx/genart/imagergba.h"
#include "wx/artbase/graphica.h"

#include "wx/aggdrawer/aggdrawer.h"


//!  wxGraphicsContext based drawing context.
/*!
    Use a2dAggContext derived from a2dContext and therefore wxGraphicsContext from wxWidgets to draw.

    a2dContext  uses a2dFill, a2dStroke, a2dVpath for storing the brush,pen and path data.

    \ingroup drawer
*/
class  a2dAggContext : public a2dContext
{
    DECLARE_NO_COPY_CLASS( a2dAggContext )

public:
    a2dAggContext( wxGraphicsRenderer* renderer, const wxImage& drawable );
    a2dAggContext( wxGraphicsRenderer* renderer, a2dImageRGBA* drawable );
    virtual ~a2dAggContext();

    virtual void Clip( const wxRegion& region );

    // clips drawings to the rect
    virtual void Clip( wxDouble x, wxDouble y, wxDouble w, wxDouble h );

    // resets the clipping to original extent
    virtual void ResetClip();

    virtual void Translate( wxDouble dx , wxDouble dy );
    virtual void Scale( wxDouble xScale , wxDouble yScale );
    virtual void Rotate( wxDouble angle );

    // concatenates this transform with the current transform of this context
    virtual void ConcatTransform( const wxGraphicsMatrix& matrix );

    // sets the transform of this context
    virtual void SetTransform( const wxGraphicsMatrix& matrix );

    // gets the matrix of this context
    virtual wxGraphicsMatrix GetTransform() const;

    virtual void SetDrawStyle( a2dDrawStyle drawstyle );

#if wxCHECK_VERSION(2,9,0)
    virtual void DrawBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h );
#else
    virtual void DrawGraphicsBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h );
#endif

    virtual void GetTextExtent( const wxString& str, wxDouble* width, wxDouble* height,
                                wxDouble* descent, wxDouble* externalLeading ) const;
    virtual void GetPartialTextExtents( const wxString& text, wxArrayDouble& widths ) const;

#if wxCHECK_VERSION(2,9,0)
    bool SetCompositionMode( wxCompositionMode op );
#endif

    // sets the current logical function, returns true if it supported
    virtual bool SetLogicalFunction( int function ) ;

    virtual void DrawCharDc( wxChar c );
    virtual void DrawTextDc( const wxString& text, double x, double y );
    virtual void DrawCharStroke( wxChar c );
    virtual void DrawCharFreetype( wxChar c );

    void DrawRoundedRectangle( wxDouble x, wxDouble y, wxDouble w, wxDouble h, wxDouble radius );
    void DrawEllipse( wxDouble x, wxDouble y, wxDouble w, wxDouble h );

    void SetPreStroke( bool preStroke ) { m_preStroke = preStroke; }

    bool GetPreStroke() { return m_preStroke; }

private:

    void Render( bool fillColor );
    virtual void RenderGradient( bool radial );
    virtual void RenderBitmapFill();

    int ToAggPath( a2dVertexArray* points, bool transform );
    int ToAggPath( const a2dVertexList* list, bool transform );

    void DrawPolygon( const a2dVertexList* list, wxPolygonFillMode fillStyle );
    void DrawLines( const a2dVertexList* list );

    agg::trans_affine _get_agg_user_to_device_transform() const
    {
        return agg::trans_affine(
                   m_usertodevice( 0, 0 ), m_usertodevice( 0, 1 ),
                   m_usertodevice( 1, 0 ), m_usertodevice( 1, 1 ),
                   m_usertodevice( 2, 0 ), m_usertodevice( 2, 1 ) );
    }

    //! per drawer implementation
    virtual void DoSetActiveStroke();

    //! per drawer implementation
    virtual void DoSetActiveFill();

    void DeviceDrawBitmap( const wxBitmap& bmp, double x, double y, bool useMask );

    a2dImageRGBA* m_buffer;

    bool m_needbufferupdate;

    wxClientDC*  m_deviceDC;

    PixFormatA       m_pixFormat;
    PixFormatComp    m_pixFormatComp;
    RendererBaseA    m_renBase;
    RendererBaseComp m_renBaseComp;
    RendererSolidComp m_renSolidComp;
    RendererSolidAaA  m_renderer;
    agg::rasterizer_scanline_aa<>  m_rasterizer;
    agg::comp_op_e    m_blendMode;

    template <class PathT>
    void _ras_add_stroked_path_xform( PathT& path, const agg::trans_affine& mtx );

    unsigned char* m_pdata;

    agg::rendering_buffer m_rendering_buffer;

    double m_strokewidth;
    double m_strokewidthDev;

    bool m_preStroke;

    a2dStrokeStyle m_style;

    agg::line_join_e m_join;

    agg::line_cap_e m_cap;

    wxImage m_pattern;
    agg::rendering_buffer m_pattern_rbuf;

    agg::scanline_u8 m_sl;

    void _stipple_render_scanlines(   agg::rasterizer_scanline_aa<>& ras,
                                      agg::scanline_u8& sl,
                                      RendererBaseA& renb,
                                      agg::rendering_buffer pat,
                                      double alpha );

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
};

//!  Drawing context derived from a2dDrawer2D
/*!
    Uses a2dAggContext internal, which is derived from a2dContext and therefore wxGraphicsContext from wxWidgets to draw.

    a2dContext  uses a2dFill, a2dStroke, a2dVpath for storing the brush,pen and path data.

    \ingroup drawer
*/
class a2dGcAggDrawer : public a2dGcBaseDrawer
{
    DECLARE_DYNAMIC_CLASS( a2dGcAggDrawer )

public:

    //! Drawer having a buffer of w pixel wide and h pixels heigh
    a2dGcAggDrawer( int width = 0, int height = 0 );

    //! Drawer having a buffer of w pixel wide and h pixels height given by size
    a2dGcAggDrawer( const wxSize& size );

    //! bitmap is converted to image on which you can draw.
    //! Use GetBuffer() to get a bitmap back.
    a2dGcAggDrawer( const wxBitmap& bitmap );

    bool HasAlpha() { return true; }

    //!return buffer as a bitmap
    wxBitmap GetBuffer() const;

    //! copy constructor
    a2dGcAggDrawer( const a2dGcAggDrawer& other );

    a2dGcAggDrawer( const a2dDrawer2D& other );

    //!set buffer size to w pixel wide and h pixels heigh
    void SetBufferSize( int w, int h );

    //!get part of the buffer given a rect
    wxBitmap GetSubBitmap( wxRect sub_rect ) const;

    void CopyIntoBuffer( const wxBitmap& bitm );

    virtual ~a2dGcAggDrawer();

    virtual void SetYaxis( bool up );

    void InitContext();

    void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 ) );

    void ShiftBuffer( int dxy, bool yshift );

    void DrawPoint(  double xc,  double yc );

    void DrawImage( const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

protected:

    void DoSetActiveStroke();

    void DoSetActiveFill();

    void DoSetActiveFont( const a2dFont& font );

    //!the buffer that is used for rendering
    a2dImageRGBA m_buffer;

    unsigned char* m_pdata;
};

#endif

#endif //wxART2D_USE_GRAPHICS_CONTEXT

#endif
