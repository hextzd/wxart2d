/*! \file wx/artbase/dcdrawer.h

    \brief Contains graphical drawing context specific classes.
    a2dDcDrawer and derived classes are used for drawing primitives.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: dcdrawer.h,v 1.22 2009/10/06 18:40:31 titato Exp $
*/

#ifndef __WXDCDRAWER_H__
#define __WXDCDRAWER_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

//#include "wx/image.h"
//#include "wx/prntbase.h"
//#include <wx/module.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>

#include "wx/artbase/drawer2d.h"



//!This class implements drawing functions for wxDC based drawing.
/*!
This class implements drawing functions for wxDC based drawing.
See a2dDrawer2D class for documentation.
You must set the DC to use to draw to the device.
In case of a buffer you must select the bitmap into the DC yourself.
\sa class: a2dCanvas
\sa class: a2dRenderImage

    \ingroup drawer
*/
class A2DARTBASEDLLEXP a2dDcDrawer : public a2dDrawer2D
{
public:

    //! Drawer having a buffer of w pixel wide and h pixels heigh
    a2dDcDrawer( int width = 0, int height = 0 );

    //! Drawer having a buffer of w pixel wide and h pixels height given by size
    a2dDcDrawer( const wxSize& size );

    //!set buffer size to w pixel wide and h pixels heigh
    /*!
        \remark
        SetMappingDeviceRect() is NOT reset.
    */
    void SetBufferSize( int w, int h );

    wxBitmap GetBuffer() const { return wxBitmap( 0, 0 ); }

    //!get part of the buffer given a rect
    wxBitmap GetSubBitmap( wxRect sub_rect ) const;

    //!destructor
    virtual ~a2dDcDrawer();

    //!copy constructor
    a2dDcDrawer( const a2dDcDrawer& other );

    a2dDcDrawer( const a2dDrawer2D& other );

    virtual void BlitBuffer( wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 )  );

    virtual void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 )  );

    void ResetStyle();

    //!start to draw on this context (used to initialize a specific drawer)
    virtual void BeginDraw();

    //!end drawing on this context (used to reset a specific drawer)
    virtual void EndDraw();

    //!get the DC that is used for rendering
    virtual wxDC* GetRenderDC() const { return m_renderDC; }

    //!set the DC that is used for rendering
    /*!Used for setting an external DC ( e.g for printing )
       You must set an external before calling BeginDraw()
       At the end of the printing action, you must call  SetRenderDC( NULL ) to
       Switch back to normal operation, which means the creation of a wxClientDC is handled internal.
    */
    void  SetRenderDC( wxDC* dc );

    wxDC* GetDeviceDC() const { return m_deviceDC; }

    void SetClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    virtual void ExtendClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    virtual void ExtendAndPushClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    void PopClippingRegion();

    void SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height );
    void SetClippingRegion( double minx, double miny, double maxx, double maxy );
    void DestroyClippingRegion();

    //! Draw a wxImage in world coordinates.
    /*! Currently this implementation does not handle rotated transformations properly.
    */
    void DrawImage( const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize = false );

    //more speedy then base, by directly converting to device coordinates.
    void DrawVpath( const a2dVpath* path );

    void DrawPolyPolygon( a2dListOfa2dVertexList polylist, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    // Documented in base class.
    void DrawCircle( double xc, double yc, double radius );

    void DrawEllipse( double xc, double yc, double width, double height );

    void DrawLine( double x1, double y1, double x2, double y2 );

    //! draw single point
    void DrawPoint(  double xc,  double yc );

    virtual void DeviceDrawAnnotation( const wxString& text, wxCoord x, wxCoord y, const wxFont& font );

protected:

    virtual void DoSetDrawStyle( a2dDrawStyle drawstyle );

    void DoSetActiveStroke();

    void DoSetActiveFill();

    //internal drawing of polygons
    virtual void DeviceDrawPolygon( unsigned int n, bool spline , wxPolygonFillMode fillStyle );

    //internal drawing of polylines
    virtual void DeviceDrawLines( unsigned int n, bool spline );

    void DeviceDrawLine( double x1, double y1, double x2, double y2 );

    void DeviceDrawHorizontalLine( int x1, int y1, int x2, bool use_stroke_color );

    void DeviceDrawVerticalLine( int x1, int y1, int y2, bool use_stroke_color );

    void DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a );

    //!in pixels
    void DeviceDrawBitmap( const wxBitmap& bmp, double x, double y,
                           bool useMask = false );

    void SetLogicalFunction( wxRasterOperationMode function );

    // Documented in base class
    virtual void DrawCharDc( wxChar c );
    // Documented in base class
    virtual void DrawTextDc( const wxString& text, double x, double y );

    // Documented in base class
    virtual void DrawCharFreetype( wxChar c );

    //!The DC to use to draw to the device or buffer ( depends on type of a2dDrawer2D )
    /*!In case of a m_buffer you must select the bitmap into the DC yourself. */
    wxDC*     m_renderDC;

    //! wxDc to draw or blit to the device
    /*! ( either from a blit from the buffer or drawn directly ) */
    wxClientDC*  m_deviceDC;

    //! when dc is set from the outside, this is true.
    bool m_externalDc;

private:
    //! clipping region.
    wxRegion m_clip;

    DECLARE_DYNAMIC_CLASS( a2dDcDrawer )
};


//!This class implements drawing functions for wxDC based drawing.
/*!
It uses a bitmap buffer to do double buffering.
See a2dDrawer2D class for more documentation.
To start drawing you must call BeginDraw at when stopping drawing you must call EndDraw.
This will takes care of setting and releasing the wxMemoryDC used internal, and keeps your code
"a2dDrawer2D type" independent.
\sa class: a2dCanvas
\sa class: a2dRenderImage

    \ingroup drawer
*/
class A2DARTBASEDLLEXP a2dMemDcDrawer : public a2dDcDrawer
{
public:

    //!constructor ( just some default )
    a2dMemDcDrawer( int width = 0, int height = 0 );

    //! Drawer having a buffer of w pixel wide and h pixels height given by size
    a2dMemDcDrawer( const wxSize& size );

    //!start to draw on this context (used to initialize a specific drawer)
    virtual void BeginDraw();

    //!end drawing on this context (used to reset a specific drawer)
    virtual void EndDraw();

    //!return buffer as a bitmap
    wxBitmap GetBuffer() const {return m_buffer;}

    //! change the size of the buffer
    void SetBufferSize( int w, int h );

    //!return a bitmap representing a rectangular part of the buffer
    wxBitmap GetSubBitmap( wxRect sub_rect ) const;

    //!destructor
    virtual ~a2dMemDcDrawer();

    //!copy constructor
    a2dMemDcDrawer( const a2dMemDcDrawer& other );

    //! Draw a wxImage in world coordinates.
    /*! Use AggDrawer when available.
    */
    void DrawImage( const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void CopyIntoBuffer( const wxBitmap& bitm );

    virtual void BlitBuffer( wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 )  );

    virtual void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 )  );

    virtual void ShiftBuffer( int dxy, bool yshift );

protected:

    virtual void DrawTextFreetype( const wxString& text, double x, double y );

#if wxART2D_USE_FREETYPE
    //! draw one character into screen buffer, at the position (x,y). 
    void DrawCharFreetype( wxColour col, wxChar c, int x, int y, FT_Matrix trans_matrix, FT_Vector vec );
#endif
    virtual void DrawCharFreetype( wxChar c );

    virtual void DrawPolygon( const a2dVertexList* list, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    virtual void DeviceDrawPolygon( unsigned int n, bool spline , wxPolygonFillMode fillStyle );

    //!the buffer that is used for rendering
    wxBitmap  m_buffer;

    //!Created at BeginDraw, and destoyed at EndDraw, used to actually draw
    wxMemoryDC* m_memdc;

    // internally used in methods. Create/ delete of these is very expensive
    mutable wxMemoryDC m_mdc1, m_mdc2;
#if defined(__WXMSW__)
    HDC m_hdc1, m_hdc2;
#endif

private:

    DECLARE_DYNAMIC_CLASS( a2dMemDcDrawer )
};

#if wxART2D_USE_GRAPHICS_CONTEXT

//!  wxGraphicsContext based drawing context derived from a2dDrawer2D
/*!
    Use wxGraphicsContext from wxWidgets to draw.

    \ingroup drawer
*/
class a2dGcDrawer : public a2dGcBaseDrawer
{
    DECLARE_DYNAMIC_CLASS( a2dGcDrawer )

public:

    //! Drawer having a buffer of w pixel wide and h pixels heigh
    a2dGcDrawer( int width = 0, int height = 0 );

    //! Drawer having a buffer of w pixel wide and h pixels height given by size
    a2dGcDrawer( const wxSize& size );

    //! bitmap is converted to image on which you can draw.
    //! Use GetBuffer() to get a bitmap back.
    a2dGcDrawer( const wxBitmap& bitmap );

    //!return buffer as a bitmap
    wxBitmap GetBuffer() const;

    //! copy constructor
    a2dGcDrawer( const a2dGcDrawer& other );

    a2dGcDrawer( const a2dDrawer2D& other );

    //!set buffer size to w pixel wide and h pixels heigh
    void SetBufferSize( int w, int h );

    //!get part of the buffer given a rect
    wxBitmap GetSubBitmap( wxRect sub_rect ) const;

    virtual ~a2dGcDrawer();

    //!start to draw on this context (used to initialize a specific drawer)
    virtual void BeginDraw();

    //!end drawing on this context (used to reset a specific drawer)
    virtual void EndDraw();

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
    wxBitmap  m_buffer;

    //!Created at BeginDraw, and destoyed at EndDraw, used to actually draw
    wxMemoryDC m_memdc;

    //! when dc is set from the outside, this is true.
    bool m_externalDc;
};

#endif  // wxART2D_USE_GRAPHICS_CONTEXT


#endif /* __WXDCDRAWER_H__ */

