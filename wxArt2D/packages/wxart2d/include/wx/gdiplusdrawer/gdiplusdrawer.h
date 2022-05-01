/////////////////////////////////////////////////////////////////////////////
// Name:        gdiplusdrawer.h
// Author:      Tsolakos Stavros
// Created:     04/30/04
// Copyright:   2004 (c) Tsolakos Stavros
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WXGDIPLUSDRAWER_H__
#define __WXGDIPLUSDRAWER_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// where we keep all our configuration symbol (wxART2D_USE_*)
#include "a2dprivate.h"

#if wxART2D_USE_GDIPLUSDRAWER

#include <gdiplus.h>

#include "wx/artbase/drawer2d.h"
#include "wx/artbase/stylebase.h"
#include "wx/image.h"
#include "wx/geometry.h"

// We do not want to have a Gdiplus:: prefix in front
// of every GDI+ specific symbol.
using namespace Gdiplus;

// Declare a Gdiplus::Region list. It will act as a
// stack that keeps clipping regions.
WX_DECLARE_LIST( Region, RegionList );

class a2dGDIPlusDrawer : public a2dDrawer2D
{
public:
    a2dGDIPlusDrawer( int width = 100, int height = 100 );
    a2dGDIPlusDrawer( const wxSize& size );

    a2dGDIPlusDrawer( const a2dGDIPlusDrawer& other );

    virtual ~a2dGDIPlusDrawer( );

    void Init();

public:

    //!get buffer as bitmap pointer
    wxBitmap GetBuffer() const { return m_buffer; }

    //!set buffer size to w pixel wide and h pixels heigh
    void SetBufferSize( int w, int h );

    //!get part of the buffer given a rect
    wxBitmap GetSubBitmap( wxRect sub_rect ) const;

    void CopyIntoBuffer( const wxBitmap& bitm );

    virtual void SetTransform( const a2dAffineMatrix& userToWorld );
    virtual void PushTransform();
    virtual void PushIdentityTransform();
    virtual void PushTransform( const a2dAffineMatrix& affine );
    virtual void PopTransform( void );

    //!start to draw on this context (used to initialize a specific drawer)
    virtual void BeginDraw();

    //!end drawing on this context (used to reset a specific drawer)
    virtual void EndDraw();

    //!set the DC that is used for rendering
    /*!Used for setting an external DC ( e.g for printing )
       You must set an external before calling BeginDraw()
       At the end of the printing action, you must call  SetRenderDC( NULL ) to
       Switch back to normal operation, which means the creation of a wxClientDC is handled internal.
    */
    void  SetRenderDC( wxDC* dc );

    wxDC* GetDeviceDC() const { return m_deviceDC; }

    void BlitBuffer( wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 ) );

    //! used for blitting to a wxDC.
    virtual void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 ) );

    void ShiftBuffer( int dxy, bool yshift );

    void ResetStyle( );

    void SetClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    virtual void ExtendClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    virtual void ExtendAndPushClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    void PopClippingRegion( );

    void SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height );

    void SetClippingRegion( double minx, double miny, double maxx, double maxy );

    void DestroyClippingRegion( );

    virtual void DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize = false );

    virtual void DrawCircle( double x, double y, double radius );

    //! draw single point
    void DrawPoint(  double xc,  double yc );

    virtual void DrawEllipse( double x, double y, double width, double height );

    void DrawImage( const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void SetTextBackgroundMode( int mode );

protected:

    virtual void DoSetDrawStyle( a2dDrawStyle drawstyle );

    void DoSetActiveStroke();

    void DoSetActiveFill();

    virtual void DeviceDrawPolygon( unsigned int n, bool spline , wxPolygonFillMode fillStyle );

    virtual void DeviceDrawLines( unsigned int n, bool spline );

    void DeviceDrawLine( double x1, double y1, double x2, double y2 );

    void DeviceDrawHorizontalLine( int x1, int y1, int x2, bool use_stroke_color );

    void DeviceDrawVerticalLine( int x1, int y1, int y2, bool use_stroke_color );

    void DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255 );

    virtual void DrawTextDc( const wxString& text, double x, double y );

    // Documented in base class
    virtual void DrawCharStroke( wxChar c );
    // Documented in base class
    virtual void DrawCharFreetype( wxChar c );
    // Documented in base class
    virtual void DrawCharDc( wxChar c );

    Matrix* _get_gdiplus_user_to_device_transform();

    GdiplusStartupInput m_gdiplus_startup_input;
    ULONG_PTR m_gdiplus_token;
    Graphics* m_context;

    //!the buffer that is used for rendering
    wxBitmap  m_buffer;
    //!Created at BeginDraw, and destoyed at EndDraw, used to actually draw
    wxMemoryDC*  m_dc;
    RegionList m_clipping_region_stack;

    Pen* m_current_pen;
    Brush* m_current_brush;
    Image* m_penImage;
    Brush* m_penBrush;
    Image* m_brushImage;
    Matrix* m_matrix;
    GraphicsPath* m_brushPath;

    //! wxDc to draw or blit to the device
    /*! ( either from a blit from the buffer or drawn directly ) */
    wxClientDC*  m_deviceDC;

    //! when dc is set from the outside, this is true.
    bool m_externalDc;

    //! clipping region.
    wxRegion m_clip;

    // Assistant functions that perform several conversions
    // between wxArt2d and GDI+ similar data types.
    GraphicsPath* createGraphicsPath( a2dVertexList* list, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );
};


#endif // wxART2D_USE_GDIPLUS_DRAWER

#endif // __WXGDIPLUSDRAWER_H__
