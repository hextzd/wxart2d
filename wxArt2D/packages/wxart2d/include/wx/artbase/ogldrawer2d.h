/*! \file wx/artbase/ogldrawer2d.h
    \brief Contains graphical drawing context specific classes.
    a2dOglDrawer2D is used for drawing primitives using OpenGl.

    A drawing context which is a2dDrawer2D derived, can be used to draw in OpenGl

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: drawer2d.h,v 1.53 2016/21/10 18:40:31 titato Exp $
*/

#ifndef __OGLDRAWER2D_H__
#define __OGLDRAWER2D_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/prntbase.h"
#include <wx/module.h>

#include "wx/artbase/drawer2d.h"

#if wxART2D_USE_OPENGL && wxUSE_GLCANVAS 
#include "wx/glcanvas.h"


#include <wx/GL/glext.h>

// function pointers for FBO extension
// Windows needs to get function pointers from ICD OpenGL drivers,
// because opengl32.dll does not support extensions higher than v1.1.

// Framebuffer object
extern PFNGLGENFRAMEBUFFERSPROC                     pglGenFramebuffers;                      // FBO name generation procedure
extern PFNGLDELETEFRAMEBUFFERSPROC                  pglDeleteFramebuffers;                   // FBO deletion procedure
extern PFNGLBINDFRAMEBUFFERPROC                     pglBindFramebuffer;                      // FBO bind procedure
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC              pglCheckFramebufferStatus;               // FBO completeness test procedure
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC pglGetFramebufferAttachmentParameteriv;  // return various FBO parameters
extern PFNGLGENERATEMIPMAPPROC                      pglGenerateMipmap;                       // FBO automatic mipmap generation procedure
extern PFNGLFRAMEBUFFERTEXTURE2DPROC                pglFramebufferTexture2D;                 // FBO texdture attachement procedure
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC             pglFramebufferRenderbuffer;              // FBO renderbuffer attachement procedure
// Renderbuffer object

extern PFNGLGENRENDERBUFFERSPROC                    pglGenRenderbuffers;                     // renderbuffer generation procedure
extern PFNGLDELETERENDERBUFFERSPROC                 pglDeleteRenderbuffers;                  // renderbuffer deletion procedure
extern PFNGLBINDRENDERBUFFERPROC                    pglBindRenderbuffer;                     // renderbuffer bind procedure
extern PFNGLRENDERBUFFERSTORAGEPROC                 pglRenderbufferStorage;                  // renderbuffer memory allocation procedre
extern PFNGLGETRENDERBUFFERPARAMETERIVPROC          pglGetRenderbufferParameteriv;           // return various renderbuffer parameters
extern PFNGLISRENDERBUFFERPROC                      pglIsRenderbuffer;                       // determine renderbuffer object type

#define glGenFramebuffers                        pglGenFramebuffers
#define glDeleteFramebuffers                     pglDeleteFramebuffers
#define glBindFramebuffer                        pglBindFramebuffer
#define glCheckFramebufferStatus                 pglCheckFramebufferStatus
#define glGetFramebufferAttachmentParameteriv    pglGetFramebufferAttachmentParameteriv
#define glGenerateMipmap                         pglGenerateMipmap
#define glFramebufferTexture2D                   pglFramebufferTexture2D
#define glFramebufferRenderbuffer                pglFramebufferRenderbuffer

#define glGenRenderbuffers                       pglGenRenderbuffers
#define glDeleteRenderbuffers                    pglDeleteRenderbuffers
#define glBindRenderbuffer                       pglBindRenderbuffer
#define glRenderbufferStorage                    pglRenderbufferStorage
#define glGetRenderbufferParameteriv             pglGetRenderbufferParameteriv
#define glIsRenderbuffer                         pglIsRenderbuffer

// GL_EXT_framebuffer_blit
extern PFNGLBLITFRAMEBUFFERPROC    pglBlitFramebuffer;                   // copy framebuffer
#define glBlitFramebuffer   pglBlitFramebuffer


///////////////////////////////////////////////////////////////////////////////
// check FBO completeness
///////////////////////////////////////////////////////////////////////////////
extern bool checkFramebufferStatus();

class a2d3dVertex
{
public:

    a2d3dVertex( float x = 0, float y = 0, float z = 0 )
    { m_v[0] = x; m_v[1] = y; m_v[2] = z; }

    GLdouble m_v[3];
};

class a2dOglDrawer : public a2dDrawer2D
{
    DECLARE_CLASS( a2dOglDrawer )

public:

    //! Drawer having a buffer of w pixel wide and h pixels heigh
    a2dOglDrawer( int width = 0, int height = 0, wxGLContext* glRC = NULL );

    //! copy constructor
    a2dOglDrawer( const a2dOglDrawer& other );

    a2dOglDrawer( const a2dDrawer2D& other );

    virtual ~a2dOglDrawer();

    bool HasAlpha() { return true; }

    virtual void BeginDraw();
    virtual void EndDraw();

    //!return buffer as a bitmap
    wxBitmap GetBuffer() const;

    //!set buffer size to w pixel wide and h pixels heigh
    void SetBufferSize( int w, int h );

    //!get part of the buffer given a rect
    wxBitmap GetSubBitmap( wxRect sub_rect ) const;

    virtual void BlitBuffer( wxRect rect, const wxPoint& bufferpos );

    virtual void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 )  );

    virtual void ShiftBuffer( int dxy, bool yshift );

    void ResetStyle();

    void SetClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    virtual void ExtendClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    virtual void ExtendAndPushClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND );

    void PopClippingRegion();

    void SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height );

    void SetClippingRegion( double minx, double miny, double maxx, double maxy );

    void DestroyClippingRegion();

    virtual void DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize = false );

    virtual void DrawCircle( double x, double y, double radius );

    void DrawPoint(  double xc,  double yc );

    virtual void DrawEllipse( double x, double y, double width, double height );

    virtual void DrawLines( const a2dVertexList* list, bool spline );

    virtual void DrawPolygon( a2dVertexArray* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    virtual void DrawPolygon( const a2dVertexList* list, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    virtual void DrawLines( a2dVertexArray* points, bool spline = false );

    void DrawLine( double x1, double y1, double x2, double y2 );

    void DrawImage( const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity = 255 );

    void DrawTextFreetype( const wxString& text, double x, double y );

    void DrawShape( const a2dListId& Ids ) const;

    virtual void DeleteShapeIds();
    virtual void DeleteShapeIds( const a2dListId& Ids );

protected:

    virtual bool InitOglcalls();

    virtual void DoSetMappingUpp();

    void DrawPolyOgl( a2dVertexArray* points, GLenum mode );

    void DrawPolyOgl( const a2dVertexList* list, GLenum mode );

    //! draw an internal  polygon in device coordinates
    virtual void DeviceDrawPolygon( unsigned int n, bool spline , wxPolygonFillMode fillStyle );

    //! draw an internal polyline in device coordinates
    virtual void DeviceDrawLines( unsigned int n, bool spline );

    //!draw in pixels
    virtual void DeviceDrawLine( double x1, double y1, double x2, double y2 );

    //! Draw a pixel-width, unstroked horizontal line in device (pixel) coordinates
    /*! This method is useful for implementing low level fill routines.

    \param x1  x start of line
    \param x2  x end of line
    \param y1  y of line
    \param use_stroke_color If true, then use the current stroke color
                            If false, then use the current fill color.
    */
    virtual void DeviceDrawHorizontalLine( int x1, int y1, int x2, bool use_stroke_color );

    //! Draw a pixel-width, unstroked vertical line in device (pixel) coordinates
    /*! This method is useful for implementing low level fill routines.

    \param x1  x of line
    \param y1  y start of line
    \param y2  y end of line
    \param use_stroke_color If true, then use the current stroke color
                            If false, then use the current fill color.
    */
    virtual void DeviceDrawVerticalLine( int x1, int y1, int y2, bool use_stroke_color );

    //!draw a single, unstroked pixel in device coordinates with the given color
    virtual void DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255 );

    void DrawCharFreetype( wxColour col, wxChar c, int dx, int dy, FT_Matrix trans_matrix, FT_Vector vec );

    void DrawCharFreetype( wxChar c );

    void DrawTextDc( const wxString& text, double x, double y );

    void DrawTextStroke( const wxString& text, double x, double y );

    void DrawCharStroke( wxChar c );

    virtual void DoSetDrawStyle( a2dDrawStyle drawstyle );

    void DoSetActiveStroke();

    void DoSetActiveFill();

    void DoSetActiveFont( const a2dFont& font );

    wxRegion m_clip;

    wxGLContext* m_context;

    std::vector< a2d3dVertex > m_tesselation;

    wxImage m_pattern;
    GLubyte m_stipple[ 32 * 32];
    GLubyte m_stippleInverse[ 32 * 32];

    double m_strokewidth;
    double m_strokewidthDev;

    bool m_initGL;

    GLuint m_patternTextureId;
};


class a2dOglDrawerBuf : public a2dOglDrawer
{
    DECLARE_CLASS( a2dOglDrawerBuf )

public:

    //! Drawer having a buffer of w pixel wide and h pixels heigh
    a2dOglDrawerBuf( int width = 0, int height = 0, wxGLContext* glRC = NULL );

    //! copy constructor
    a2dOglDrawerBuf( const a2dOglDrawerBuf& other );

    a2dOglDrawerBuf( const a2dDrawer2D& other );

    virtual ~a2dOglDrawerBuf();

    void SetBufferSize( int w, int h );

    virtual void BeginDraw();
    virtual void EndDraw();

    virtual void BlitBuffer( wxRect rect, const wxPoint& bufferpos );

    virtual void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos = wxPoint( 0, 0 )  );

    virtual void ShiftBuffer( int dxy, bool yshift );

protected:

    virtual bool InitOglcalls();

    GLuint m_fboId;
    GLuint m_textureId; // ID of texture
    GLuint m_depthId;       // ID of Renderbuffer object

};

#endif //wxART2D_USE_OPENGL

#endif /* __OGLDRAWER2D_H__ */
