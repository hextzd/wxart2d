/*! \file artbase/src/drawer2d.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: drawer2d.cpp,v 1.159 2009/10/06 18:40:31 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#if wxART2D_USE_OPENGL && wxUSE_GLCANVAS 

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/artbase/ogldrawer2d.h"
#include "wx/artbase/stylebase.h"
#if defined(__WXMSW__)
#include "wx/artbase/mswfont.h"
#endif
#endif



#ifdef __WXGTK__
#define CALLBACK
#endif

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

IMPLEMENT_CLASS( a2dOglDrawer, a2dDrawer2D )

#ifdef __DARWIN__
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

// Framebuffer object
PFNGLGENFRAMEBUFFERSPROC                     pglGenFramebuffers = 0;                      // FBO name generation procedure
PFNGLDELETEFRAMEBUFFERSPROC                  pglDeleteFramebuffers = 0;                   // FBO deletion procedure
PFNGLBINDFRAMEBUFFERPROC                     pglBindFramebuffer = 0;                      // FBO bind procedure
PFNGLCHECKFRAMEBUFFERSTATUSPROC              pglCheckFramebufferStatus = 0;               // FBO completeness test procedure
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC pglGetFramebufferAttachmentParameteriv = 0;  // return various FBO parameters
PFNGLGENERATEMIPMAPPROC                      pglGenerateMipmap = 0;                       // FBO automatic mipmap generation procedure
PFNGLFRAMEBUFFERTEXTURE2DPROC                pglFramebufferTexture2D = 0;                 // FBO texdture attachement procedure
PFNGLFRAMEBUFFERRENDERBUFFERPROC             pglFramebufferRenderbuffer = 0;              // FBO renderbuffer attachement procedure
// Renderbuffer object
PFNGLGENRENDERBUFFERSPROC                    pglGenRenderbuffers = 0;                     // renderbuffer generation procedure
PFNGLDELETERENDERBUFFERSPROC                 pglDeleteRenderbuffers = 0;                  // renderbuffer deletion procedure
PFNGLBINDRENDERBUFFERPROC                    pglBindRenderbuffer = 0;                     // renderbuffer bind procedure
PFNGLRENDERBUFFERSTORAGEPROC                 pglRenderbufferStorage = 0;                  // renderbuffer memory allocation procedure
PFNGLGETRENDERBUFFERPARAMETERIVPROC          pglGetRenderbufferParameteriv = 0;           // return various renderbuffer parameters
PFNGLISRENDERBUFFERPROC                      pglIsRenderbuffer = 0;                       // determine renderbuffer object type
PFNGLBLITFRAMEBUFFERPROC    pglBlitFramebuffer = 0;                   // copy framebuffer

bool checkFramebufferStatus()
{
    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status)
    {
    case GL_FRAMEBUFFER_COMPLETE:
        std::cout << "Framebuffer complete." << std::endl;
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
        return false;
/*
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
        std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
        return false;
*/
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_UNSUPPORTED:
        std::cout << "[ERROR] Framebuffer incomplete: Unsupported by FBO implementation." << std::endl;
        return false;

    default:
        std::cout << "[ERROR] Framebuffer incomplete: Unknown error." << std::endl;
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
// GLU_TESS CALLBACKS
///////////////////////////////////////////////////////////////////////////////
//http://www.songho.ca/opengl/gl_displaylist.html

void CALLBACK tessBeginCB(GLenum what)
{
    glBegin(what);
}

void CALLBACK tessEndCB()
{
    glEnd();
}

void CALLBACK tessVertexCB(const GLvoid *data)
{
    // cast back to double type
    const GLdouble *ptr = (const GLdouble*)data;
    glVertex3dv(ptr);
}

void CALLBACK tessErrorCB(GLenum errorCode)
{
    const GLubyte *errorStr;
    errorStr = gluErrorString(errorCode);
}

/*******************************************************************
a2dOglDrawer
********************************************************************/
bool fboSupported;
bool fboUsed;
bool fboBlitSupported;
bool fboBlitUsed;

float a2dNormDepth = -(wxINT16_MAX-1);

a2dOglDrawer::a2dOglDrawer( int width, int height, wxGLContext* glRC ): a2dDrawer2D( width, height )
{
    m_initGL = false;
    m_context = glRC;   
}

a2dOglDrawer::a2dOglDrawer( const a2dOglDrawer& other )
    : a2dDrawer2D( other )
{
    m_context = other.m_context;
}

a2dOglDrawer::a2dOglDrawer( const a2dDrawer2D& other )
    : a2dDrawer2D( other )
{
    m_context = NULL;
}

a2dOglDrawer::~a2dOglDrawer()
{
   DeleteShapeIds();
   glDeleteTextures( 1, &m_patternTextureId );
}



bool a2dOglDrawer::InitOglcalls()
{
    if ( !m_initGL )
    {

#ifdef _WIN32

    //((a2dOglCanvas*)m_display)->SetCurrent(*m_context);
    m_context->SetCurrent(*static_cast<const wxGLCanvas *>(m_display));

    // check if FBO is supported by your video card
    if( 1 )
    {
        // get pointers to GL functions
        glGenFramebuffers                     = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
        glDeleteFramebuffers                  = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
        glBindFramebuffer                     = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
        glCheckFramebufferStatus              = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
        glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
        glGenerateMipmap                      = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
        glFramebufferTexture2D                = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
        glFramebufferRenderbuffer             = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
        glGenRenderbuffers                    = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
        glDeleteRenderbuffers                 = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
        glBindRenderbuffer                    = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
        glRenderbufferStorage                 = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
        glGetRenderbufferParameteriv          = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetRenderbufferParameteriv");
        glIsRenderbuffer                      = (PFNGLISRENDERBUFFERPROC)wglGetProcAddress("glIsRenderbuffer");

        // check once again FBO extension
        if(glGenFramebuffers && glDeleteFramebuffers && glBindFramebuffer && glCheckFramebufferStatus &&
           glGetFramebufferAttachmentParameteriv && glGenerateMipmap && glFramebufferTexture2D && glFramebufferRenderbuffer &&
           glGenRenderbuffers && glDeleteRenderbuffers && glBindRenderbuffer && glRenderbufferStorage &&
           glGetRenderbufferParameteriv && glIsRenderbuffer)
        {
            fboSupported = fboUsed = true;
            std::cout << "Video card supports GL_ARB_framebuffer_object." << std::endl;
        }
        else
        {
            fboSupported = fboUsed = false;
            std::cout << "Video card does NOT support GL_ARB_framebuffer_object." << std::endl;
        }
    }

    // check if GL_EXT_framebuffer_blit is supported by your video card
    if( 1 )
    {
        // get pointers to GL functions
        glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)wglGetProcAddress("glBlitFramebuffer");
        if(glBlitFramebuffer)
        {
            fboBlitSupported = fboBlitUsed = true;
            std::cout << "Video card supports GL_EXT_framebuffer_blit." << std::endl;
        }
        else
        {
            fboBlitSupported = fboBlitUsed = false;
            std::cout << "Video card does NOT support GL_EXT_framebuffer_blit." << std::endl;
        }
    }

#else // for linux, do not need to get function pointers, it is up-to-date
    if( 1)//glInfo.isExtensionSupported("GL_ARB_framebuffer_object"))
    {
        fboSupported = fboUsed = true;
        std::cout << "Video card supports GL_ARB_framebuffer_object." << std::endl;
        
        // get pointers to GL functions
        glGenFramebuffers                     = (PFNGLGENFRAMEBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenFramebuffers");
        glDeleteFramebuffers                  = (PFNGLDELETEFRAMEBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glDeleteFramebuffers");
        glBindFramebuffer                     = (PFNGLBINDFRAMEBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindFramebuffer");
        glCheckFramebufferStatus              = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glXGetProcAddress((const GLubyte *)"glCheckFramebufferStatus");
        glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)glXGetProcAddress((const GLubyte *)"glGetFramebufferAttachmentParameteriv");
        glGenerateMipmap                      = (PFNGLGENERATEMIPMAPPROC)glXGetProcAddress((const GLubyte *)"glGenerateMipmap");
        glFramebufferTexture2D                = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glXGetProcAddress((const GLubyte *)"glFramebufferTexture2D");
        glFramebufferRenderbuffer             = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)glXGetProcAddress((const GLubyte *)"glFramebufferRenderbuffer");
        glGenRenderbuffers                    = (PFNGLGENRENDERBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenRenderbuffers");
        glDeleteRenderbuffers                 = (PFNGLDELETERENDERBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glDeleteRenderbuffers");
        glBindRenderbuffer                    = (PFNGLBINDRENDERBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindRenderbuffer");
        glRenderbufferStorage                 = (PFNGLRENDERBUFFERSTORAGEPROC)glXGetProcAddress((const GLubyte *)"glRenderbufferStorage");
        glGetRenderbufferParameteriv          = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)glXGetProcAddress((const GLubyte *)"glGetRenderbufferParameteriv");
        glIsRenderbuffer                      = (PFNGLISRENDERBUFFERPROC)glXGetProcAddress((const GLubyte *)"glIsRenderbuffer");

        // check once again FBO extension
        if(glGenFramebuffers && glDeleteFramebuffers && glBindFramebuffer && glCheckFramebufferStatus &&
           glGetFramebufferAttachmentParameteriv && glGenerateMipmap && glFramebufferTexture2D && glFramebufferRenderbuffer &&
           glGenRenderbuffers && glDeleteRenderbuffers && glBindRenderbuffer && glRenderbufferStorage &&
           glGetRenderbufferParameteriv && glIsRenderbuffer)
        {
            fboSupported = fboUsed = true;
            std::cout << "Video card supports GL_ARB_framebuffer_object." << std::endl;
        }
        else
        {
            fboSupported = fboUsed = false;
            std::cout << "Video card does NOT support GL_ARB_framebuffer_object." << std::endl;
        }
        
    }
    else
    {
        fboSupported = fboUsed = false;
        std::cout << "Video card does NOT support GL_ARB_framebuffer_object." << std::endl;
    }

    if(1)//glInfo.isExtensionSupported("GL_EXT_framebuffer_blit"))
    {
        fboBlitSupported = fboBlitUsed = true;
        std::cout << "Video card supports GL_EXT_framebuffer_blit." << std::endl;
        
        // get pointers to GL functions
        glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBlitFramebuffer");
        if(glBlitFramebuffer)
        {
            fboBlitSupported = fboBlitUsed = true;
            std::cout << "Video card supports GL_EXT_framebuffer_blit." << std::endl;
        }
        else
        {
            fboBlitSupported = fboBlitUsed = false;
            std::cout << "Video card does NOT support GL_EXT_framebuffer_blit." << std::endl;
        }
        
    }
    else
    {
        fboBlitSupported = fboBlitUsed = false;
        std::cout << "Video card does NOT support GL_EXT_framebuffer_blit." << std::endl;
    }


#endif

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glStencilMask(0x00);
        // draw where stencil's value is 0
        //glStencilFunc(GL_EQUAL, 0, 0xFF);
        /* (nothing to draw) */
        // draw only where stencil's value is 1
        glStencilFunc(GL_EQUAL, 1, 0xFF);

        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glClearColor((GLfloat)0.15, (GLfloat)0.15, 0.0, (GLfloat)1.0); // Dark, but not black.
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        m_initGL = true;
    }

    return true;
}

void a2dOglDrawer::DoSetMappingUpp() 
{
    glViewport(0, 0, (GLint) m_mapWidth, (GLint) m_mapHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if ( !m_yaxisDevice )
    {
        glOrtho(0,      // left
                m_mapWidth,  // right
                m_mapHeight, // bottom
                0,      // top
                0,      // zNear
                1       // zFar
                );
    }
    else
    {
        glOrtho(0,      // left
                m_mapWidth,  // right
                0, // bottom
                m_mapHeight,      // top
                0,      // zNear
                1       // zFar
                );
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();   
};


void a2dOglDrawer::BeginDraw()
{
    if ( !m_initGL )
    {
        InitOglcalls();
    }

    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
        //((a2dOglCanvas*)m_display)->SetCurrent(*m_context);
        m_context->SetCurrent(*static_cast<const wxGLCanvas *>(m_display));

        glLoadIdentity();
        //glClearColor( 0.3f, 0.4f, 0.6f, 1.0f );
        //glClear( GL_COLOR_BUFFER_BIT );
        glClear( GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT );

        SetDrawerFill( m_currentfill );
        SetDrawerStroke( m_currentstroke );
        DestroyClippingRegion();
        SetDrawStyle( m_drawstyle );
    }

    m_beginDraw_endDraw++;
}

void a2dOglDrawer::EndDraw()
{
    m_beginDraw_endDraw--;

    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
    }
}

void a2dOglDrawer::BlitBuffer( wxRect rect, const wxPoint& bufferpos )
{
    BeginDraw();
    if ( m_display )
    {
        wxClientDC deviceDC( m_display );
        m_display->PrepareDC( deviceDC );
        BlitBuffer( &deviceDC, rect, bufferpos );
    }
    EndDraw();
}

void a2dOglDrawer::SetClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle )
{
    unsigned int n = points->size();

    m_cpointsDouble.resize( n );

    unsigned int i = 0;
    double x, y;
    forEachIn( a2dVertexList, points )
    {
        a2dPoint2D point = ( *iter )->GetPoint();
        //transform to device
        m_usertodevice.TransformPoint( point.m_x, point.m_y, x, y );
        m_cpointsDouble[i].x = x;
        m_cpointsDouble[i].y = y;
        i++;
    }

    if ( spline )
        n = ConvertSplinedPolygon2( n );

    wxPoint* intpoints = _convertToIntPointCache( n, &m_cpointsDouble[0] );
    m_clip = wxRegion( n, intpoints, fillStyle );

    m_clipboxdev = m_clip.GetBox();
    m_clipboxworld = ToWorld( m_clipboxdev );

    //we don't know which part of the transform set in m_context is to go from world to device.
    //remove all first.
    PushIdentityTransform();

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    //glDisable(GL_DEPTH_TEST);  
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    glStencilOp( GL_REPLACE, GL_KEEP, GL_KEEP ); 
    glStencilFunc(GL_NEVER, 1, 0xFF);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);

    glBegin(GL_POLYGON);

    for ( i = 0; i < n; i++ )
    {
        glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, 0 );
    }
    glEnd();       
  
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    glStencilMask(0x00);
    // draw where stencil's value is 0
    //glStencilFunc(GL_EQUAL, 0, 0xFF);
    /* (nothing to draw) */
    // draw only where stencil's value is 1
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); 
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    PopTransform();
}

void a2dOglDrawer::ExtendAndPushClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle, a2dBooleanClip clipoperation )
{
    wxRegion* push = new wxRegion( m_clip );

    m_clipregionlist.Insert( push );

    ExtendClippingRegion( points, spline, fillStyle, clipoperation );
}

void a2dOglDrawer::ExtendClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode WXUNUSED( fillStyle ), a2dBooleanClip clipoperation )
{
    wxRegion totaladd;
    bool first = false;

    a2dBoundingBox devbox;
    bool smallPoly = false;
    unsigned int segments = ToDeviceLines( points, devbox, smallPoly );

    if ( segments == 0 )
        return;

    if ( spline )
        segments = ConvertSplinedPolygon2( segments );

    wxPoint* int_cpts = _convertToIntPointCache( segments, &m_cpointsDouble[0] );
    wxRegion add = wxRegion( segments, int_cpts, wxWINDING_RULE );
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
            m_clipboxdev = m_clip.GetBox();
            m_clipboxworld = ToWorld( m_clipboxdev );
            DestroyClippingRegion();
        }
    }
    else
    {
        m_clip = totaladd;
        m_clipboxdev = m_clip.GetBox();
        m_clipboxworld = ToWorld( m_clipboxdev );
        DestroyClippingRegion();
    }

    SetClippingRegion( m_clipboxworld.GetMinX(), m_clipboxworld.GetMinY(), m_clipboxworld.GetMaxX(), m_clipboxworld.GetMaxY() );
}

void a2dOglDrawer::PopClippingRegion()
{
    if ( !m_clipregionlist.GetCount() )
        return;

    m_clip = *m_clipregionlist.GetFirst()->GetData();
    delete m_clipregionlist.GetFirst()->GetData();
    m_clipregionlist.DeleteNode( m_clipregionlist.GetFirst() );

    m_clipboxdev = m_clip.GetBox();
    m_clipboxworld = ToWorld( m_clipboxdev );

    SetClippingRegion( m_clipboxworld.GetMinX(), m_clipboxworld.GetMinY(), m_clipboxworld.GetMaxX(), m_clipboxworld.GetMaxY() );
}

void a2dOglDrawer::SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height )
{
    m_clip = wxRegion( minx, miny, width, height );
    m_clipboxdev = wxRect( minx, miny, width, height );
    m_clipboxworld = ToWorld( m_clipboxdev );

    //we don't know which part of the transform set in m_context is to go from world to device.
    //remove all first.
    PushIdentityTransform();

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    //glDisable(GL_DEPTH_TEST);  
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    glStencilOp( GL_REPLACE, GL_KEEP, GL_KEEP ); 
    glStencilFunc(GL_NEVER, 1, 0xFF);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
   
    glBegin(GL_QUADS);            
        glVertex2f( minx, miny );    
        glVertex2f( minx + width, miny );    
        glVertex2f( minx + width, miny + height );
        glVertex2f( minx, miny + height );
    glEnd();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    glStencilMask(0x00);
    // draw where stencil's value is 0
    //glStencilFunc(GL_EQUAL, 0, 0xFF);
    /* (nothing to draw) */
    // draw only where stencil's value is 1
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); 
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    PopTransform();
}

void a2dOglDrawer::SetClippingRegion( double minx, double miny, double maxx, double maxy )
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


    //we don't know which part of the transform set in m_context is to go from world to device.
    //remove all first.
    PushIdentityTransform();

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    //glDisable(GL_DEPTH_TEST);  
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    glStencilOp( GL_REPLACE, GL_KEEP, GL_KEEP ); 
    glStencilFunc(GL_NEVER, 1, 0xFF);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);

    glBegin(GL_QUADS);            
        glVertex2f( iminx, iminy );    
        glVertex2f( imaxx, iminy );    
        glVertex2f( imaxx, imaxy );
        glVertex2f( iminx, imaxy );
    glEnd();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    glStencilMask(0x00);
    // draw where stencil's value is 0
    //glStencilFunc(GL_EQUAL, 0, 0xFF);
    /* (nothing to draw) */
    // draw only where stencil's value is 1
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); 
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    PopTransform();
}

void a2dOglDrawer::DestroyClippingRegion()
{
    m_clip.Clear();
    m_clipboxdev = wxRect( 0, 0, m_width, m_height );
    m_clipboxworld = ToWorld( m_clipboxdev );

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    //glDisable(GL_DEPTH_TEST);  
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    glStencilOp( GL_REPLACE, GL_KEEP, GL_KEEP ); 
    glStencilFunc(GL_NEVER, 1, 0xFF);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);

    
    PushIdentityTransform();
    glBegin(GL_QUADS);            
        glVertex2f( 0.0f, 0.0f );    
        glVertex2f( m_width, 0.0f );    
        glVertex2f( m_width, m_height );
        glVertex2f( 0.0f, m_height);
    glEnd();
    PopTransform();
    

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    glStencilMask(0x00);
    // draw where stencil's value is 0
    //glStencilFunc(GL_EQUAL, 0, 0xFF);
    /* (nothing to draw) */
    // draw only where stencil's value is 1
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); 
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

void a2dOglDrawer::ResetStyle()
{
    a2dDrawer2D::ResetStyle();
    SetDrawerFill( *a2dBLACK_FILL ); //set to a sure state
    SetDrawerStroke( *a2dBLACK_STROKE );
    SetDrawStyle( m_drawstyle );
}

void a2dOglDrawer::DoSetActiveFont( const a2dFont& font )
{
    // Get font size in device units
    unsigned int fontsize;
    fontsize = m_currentfont.GetSize();
    if ( fontsize < 1 )
        fontsize = 1;
    switch ( m_currentfont.GetType() )
    {
        case a2dFONT_WXDC:
        {
            m_currentfont.GetFont().SetPointSize( fontsize );
            //wxGraphicsFont f = m_render->CreateFont( m_currentfont.GetFont() );
            //m_context->SetFont( f );
            break;
        }
        default:
            wxFont wxfont = *wxNORMAL_FONT;
            wxfont.SetPointSize( fontsize );
            //wxGraphicsFont f = m_render->CreateFont( wxfont );
            //m_context->SetFont( f );
    }
}

void a2dOglDrawer::DoSetActiveStroke()
{
    glLineWidth (1.0);
    glEnable (GL_LINE_STIPPLE);   
    m_strokewidth = 0;
    m_strokewidthDev = 1;

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
                glDisable (GL_LINE_STIPPLE);   
                break;
            case a2dSTROKE_TRANSPARENT:
            case a2dSTROKE_DOT:
                glEnable (GL_LINE_STIPPLE);   
                glLineStipple (1, 0x0101);  /*  dotted  */
                break;
            case a2dSTROKE_DOT_DASH:
                glEnable (GL_LINE_STIPPLE);   
                glLineStipple (1, 0x1C47);  /*  dash/dot/dash  */
                break;
            case a2dSTROKE_LONG_DASH:
                glEnable (GL_LINE_STIPPLE);   
                glLineStipple (1, 0x00FF);  /*  dashed  */
                break;
            case a2dSTROKE_SHORT_DASH:
                glEnable (GL_LINE_STIPPLE);   
                glLineStipple (1, 0x0F0F);  /*  dashed  */
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
        }

        m_strokewidth = m_activestroke.GetWidth();
        if ( m_drawstyle == a2dWIREFRAME_ZERO_WIDTH )
        {
            a2dAffineMatrix invert = m_worldtodevice;
            invert.Invert();
            m_strokewidthDev = 1;
            m_strokewidth = invert.TransformDistance( m_strokewidthDev );
        }
        else
        {
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

    glLineWidth( m_strokewidthDev );

    glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );
}

unsigned char setBit( unsigned char ch, int i ) 
{
  unsigned mask = 1 << i ; 
  return mask | ch;
}

void a2dOglDrawer::DoSetActiveFill()
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
            GLubyte *bitmapData=m_pattern.GetData();
            int imageWidth = m_pattern.GetWidth();
            int imageHeight = m_pattern.GetHeight();
            wxASSERT_MSG( imageHeight == 32, "pattern only 32 * 32 allowed" );
            wxASSERT_MSG( imageWidth == 32, "pattern only 32 * 32 allowed" );
            int rev_val=imageHeight-1;

            memset( m_stipple, 0 , 32*32 );
            // we take only the red to decide stipple is 1 or 0
            for(int y=0; y<32; y++)
            {
                    for(int x=0; x<32; x++)
                    {       
                            int bytexy = (x+y*32)/8;
                            if ( bitmapData[( x+(rev_val-y)*32)*3] == 0 )
                                m_stipple[ bytexy ] = setBit( m_stipple[ bytexy ] , x % 8 );
                    }
            }           
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
        GLubyte *bitmapData=m_pattern.GetData();
        int imageWidth = m_pattern.GetWidth();
        int imageHeight = m_pattern.GetHeight();
        wxASSERT_MSG( imageHeight == 32, "pattern only 32 * 32 allowed" );
        wxASSERT_MSG( imageWidth == 32, "pattern only 32 * 32 allowed" );
        int rev_val=imageHeight-1;

        memset( m_stipple, 0 , 32*32 );
        memset( m_stippleInverse, 0 , 32*32 );
        // we take only the red to decide stipple is 1 or 0
        for(int y=0; y<32; y++)
        {
                for(int x=0; x<32; x++)
                {       
                        int bytexy = (x+y*32)/8;
                        if ( bitmapData[( x+(rev_val-y)*32)*3] == 0 )
                            m_stipple[ bytexy ] = setBit( m_stipple[ bytexy ] , x % 8 );
                        else
                            m_stippleInverse[ bytexy ] = setBit( m_stippleInverse[ bytexy ] , x % 8 );
                }
        }

    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP )
    {
        wxBrush dcbrush;
        //wxBrushStyle style;
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_STIPPLE:
            {
                static const int MASK_RED = 1;
                static const int MASK_GREEN = 2;
                static const int MASK_BLUE = 3;

                m_pattern = m_activefill.GetStipple().ConvertToImage();
                m_pattern.Replace( MASK_RED, MASK_GREEN, MASK_BLUE,
                                   m_colour1redFill,
                                   m_colour1greenFill,
                                   m_colour1blueFill );
                glEnable(GL_TEXTURE_2D);

                glGenTextures(1, &m_patternTextureId);
                glBindTexture(GL_TEXTURE_2D, m_patternTextureId);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glGenerateMipmap(GL_TEXTURE_2D);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);        
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                int width = m_pattern.GetWidth();
                int height = m_pattern.GetHeight();

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pattern.GetData() );
  
                glDisable(GL_TEXTURE_2D);   
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);        
                break;
            }
            case a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT:
            {
                static const int MASK_RED = 1;
                static const int MASK_GREEN = 2;
                static const int MASK_BLUE = 3;

                m_pattern = m_activefill.GetStipple().ConvertToImage();
                m_pattern.Replace( MASK_RED, MASK_GREEN, MASK_BLUE,
                                   0,
                                   0,
                                   0 );

                GLubyte *bitmapData=m_pattern.GetData();
                int imageWidth = m_pattern.GetWidth();
                int imageHeight = m_pattern.GetHeight();
                wxASSERT_MSG( imageHeight == 32, "pattern only 32 * 32 allowed" );
                wxASSERT_MSG( imageWidth == 32, "pattern only 32 * 32 allowed" );
                int rev_val=imageHeight-1;

                memset( m_stipple, 0 , 32*32 );
                memset( m_stippleInverse, 0 , 32*32 );
                // we take only the red to decide stipple is 1 or 0
                for(int y=0; y<32; y++)
                {
                        for(int x=0; x<32; x++)
                        {       
                                int bytexy = (x+y*32)/8;
                                if ( bitmapData[( x+(rev_val-y)*32)*3] == 0 )
                                    m_stipple[ bytexy ] = setBit( m_stipple[ bytexy ] , x % 8 );
                                else
                                    m_stippleInverse[ bytexy ] = setBit( m_stippleInverse[ bytexy ] , x % 8 );
                                
                        }
                }

                break;
            }
            case a2dFILL_STIPPLE_MASK_OPAQUE:
            {
                static const int MASK_RED = 1;
                static const int MASK_GREEN = 2;
                static const int MASK_BLUE = 3;

                m_pattern = m_activefill.GetStipple().ConvertToImage();
                m_pattern.Replace( MASK_RED, MASK_GREEN, MASK_BLUE,
                                   0,
                                   0,
                                   0 );

                GLubyte *bitmapData=m_pattern.GetData();
                int imageWidth = m_pattern.GetWidth();
                int imageHeight = m_pattern.GetHeight();
                wxASSERT_MSG( imageHeight == 32, "pattern only 32 * 32 allowed" );
                wxASSERT_MSG( imageWidth == 32, "pattern only 32 * 32 allowed" );
                int rev_val=imageHeight-1;

                memset( m_stipple, 0 , 32*32 );
                memset( m_stippleInverse, 0 , 32*32 );
                // we take only the red to decide stipple is 1 or 0
                for(int y=0; y<32; y++)
                {
                        for(int x=0; x<32; x++)
                        {       
                                int bytexy = (x+y*32)/8;
                                if ( bitmapData[( x+(rev_val-y)*32)*3] == 0 )
                                    m_stipple[ bytexy ] = setBit( m_stipple[ bytexy ] , x % 8 );
                                else
                                    m_stippleInverse[ bytexy ] = setBit( m_stippleInverse[ bytexy ] , x % 8 );
                        }
                }
            }
            break;
            default:
                break;
        }
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR  )
    {
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
    }

    glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, m_FillOpacityCol1 / 255.0 );
}

void a2dOglDrawer::DoSetDrawStyle( a2dDrawStyle drawstyle )
{
    if ( m_drawstyle == a2dFIX_STYLE && m_drawstyle == a2dFIX_STYLE_INVERT )
        return;

    m_drawstyle = drawstyle;

    switch( drawstyle )
    {
        case a2dWIREFRAME_INVERT:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
            //m_context->SetLogicalFunction( wxINVERT );
            break;

        case a2dWIREFRAME:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
            //m_context->SetLogicalFunction( wxCOPY );
            break;

        case a2dWIREFRAME_ZERO_WIDTH:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
            //m_context->SetLogicalFunction( wxCOPY );
            break;

        case a2dWIREFRAME_INVERT_ZERO_WIDTH:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
            //m_context->SetLogicalFunction( wxINVERT );
            break;

        case a2dFILLED:
            SetActiveStroke( m_currentstroke );
            SetActiveFill( m_currentfill );
            //m_context->SetLogicalFunction( wxCOPY );
            break;

        case a2dFIX_STYLE:
            //preserve this
            m_fixStrokeRestore = m_currentstroke;
            m_fixFillRestore = m_currentfill;
            m_fixDrawstyle = m_drawstyle;
            SetActiveStroke( m_currentstroke );
            SetActiveFill( m_currentfill );
            //m_context->SetLogicalFunction( wxCOPY );
            break;

        case a2dFIX_STYLE_INVERT:
            // don't adjust style
            break;

        default:
            wxASSERT( 0 );
    }
}


void a2dOglDrawer::DrawImage( const wxImage& imagein, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    // this has influence on the texture
    glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, Opacity * m_OpacityFactor / 255.0 );

    a2dAffineMatrix affine;
    if ( GetYaxis() )
    {
        affine.Translate( 0.0, -( y ) );
        affine.Mirror( true, false );
        affine.Translate( 0.0, y );
        PushTransform( affine );
    }

    /* Not sure what to do with a2dFill setting
    if ( IsStrokeOnly() )
    {
    }
    else
    */
    {  
        glEnable(GL_TEXTURE_2D);

        GLuint tex;
        //glColor4f( 0,0,0, m_FillOpacityCol1 );
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if ( imagein.HasAlpha() )
        {
            // note: must make a local copy before passing the data to OpenGL, as GetData() returns RGB 
            // and we want the Alpha channel if it's present. Additionally OpenGL seems to interpret the 
            // data upside-down so we need to compensate for that.
            GLubyte *bitmapData=imagein.GetData();
            GLubyte *alphaData=imagein.GetAlpha();
 
            int bytesPerPixel = imagein.HasAlpha() ?  4 : 3;
 
            int imageWidth = imagein.GetWidth();
            int imageHeight = imagein.GetHeight();
            int imageSize = imageWidth * imageHeight * bytesPerPixel;
            GLubyte *imageData=new GLubyte[imageSize];
 
            int rev_val=(imageHeight)-1;
 
            for(int y=0; y<(imageHeight); y++)
            {
                    for(int x=0; x<(imageWidth); x++)
                    {
                            imageData[(x+y*(imageWidth))*bytesPerPixel+0]=
                                    bitmapData[( x+(rev_val-y)*(imageWidth))*3];
 
                            imageData[(x+y*(imageWidth))*bytesPerPixel+1]=
                                    bitmapData[( x+(rev_val-y)*(imageWidth))*3 + 1];
 
                            imageData[(x+y*(imageWidth))*bytesPerPixel+2]=
                                    bitmapData[( x+(rev_val-y)*(imageWidth))*3 + 2];
 
                            if(bytesPerPixel==4) imageData[(x+y*(imageWidth))*bytesPerPixel+3]=
                                    alphaData[ x+(rev_val-y)*(imageWidth) ];
                    }//next
            }//next
 
            glTexImage2D(GL_TEXTURE_2D,
                            0,
                            GL_RGBA,
                            imageWidth,
                            imageHeight,
                            0,
                            imagein.HasAlpha() ?  GL_RGBA : GL_RGB,
                            GL_UNSIGNED_BYTE,
                            imageData);
 
            delete [] imageData;
        }
        else
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imagein.GetWidth(), imagein.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, imagein.GetData() );
        }
        double wx, wy;
        glBegin(GL_QUADS);
            m_usertodevice.TransformPoint( x - width / 2.0, y - height / 2.0, wx, wy );
            glTexCoord2f(0.0f, 0.0f); glVertex3f( wx, wy, m_layer / a2dNormDepth );
            m_usertodevice.TransformPoint( x + width / 2.0, y - height / 2.0, wx, wy );
            glTexCoord2f(1.0f, 0.0f); glVertex3f( wx, wy, m_layer / a2dNormDepth );
            m_usertodevice.TransformPoint( x + width / 2.0, y + height / 2.0, wx, wy );
            glTexCoord2f(1.0f, 1.0f); glVertex3f( wx, wy, m_layer / a2dNormDepth );
            m_usertodevice.TransformPoint( x - width / 2.0, y + height / 2.0, wx, wy );
            glTexCoord2f(0.0f, 1.0f); glVertex3f( wx, wy, m_layer / a2dNormDepth );
        glEnd();
    
        glDisable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);        

        glDeleteTextures( 1, &tex );
    }

    if (  IsStroked()  )
    {
        glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );

        double wx, wy;
        glBegin(GL_LINE_LOOP);
            m_usertodevice.TransformPoint( x - width / 2.0, y - height / 2.0, wx, wy );
            glVertex3f( wx, wy, m_layer / a2dNormDepth );
            m_usertodevice.TransformPoint( x + width / 2.0, y - height / 2.0, wx, wy );
            glVertex3f( wx, wy, m_layer / a2dNormDepth );
            m_usertodevice.TransformPoint( x + width / 2.0, y + height / 2.0, wx, wy );
            glVertex3f( wx, wy, m_layer / a2dNormDepth );
            m_usertodevice.TransformPoint( x - width / 2.0, y + height / 2.0, wx, wy );
            glVertex3f( wx, wy, m_layer / a2dNormDepth );
        glEnd();
    }

    if ( GetYaxis() )
        PopTransform();
}

void a2dOglDrawer::DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    wxImage wximage = image.GetImage();
    //wxGraphicsBitmap bitmap = m_render->CreateBitmap( wximage );
   // m_context->DrawGraphicsBitmap( bitmap, x - width / 2.0, y - height / 2.0, width, height );
}

void a2dOglDrawer::DrawShape( const a2dListId& Id ) const
{
    if ( m_disableDrawing )
        return;

    if ( Id.m_fill != 0 || Id.m_stroke != 0  )
    {
        glPushMatrix();

        float mm[] = { m_usertodevice.GetValue(0,0), m_usertodevice.GetValue(1,0), 0.0f, 0.0f,
                        m_usertodevice.GetValue(0,1), m_usertodevice.GetValue(1,1), 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        m_usertodevice.GetValue(2,0),  m_usertodevice.GetValue(2,1), 0.0f, 1.0f };                  
                   
        glLoadMatrixf( mm );

        if ( Id.m_fill )//!IsStrokeOnly() )
        {
            glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, m_FillOpacityCol1 / 255.0 );
            //first fill
            glCallList( Id.m_fill );
        }  
        if ( Id.m_stroke )//IsStroked()  )
        {
            //second stroke
            glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );
            glCallList( Id.m_stroke );
        }
        glLoadIdentity();
        glPopMatrix();
        return;
    }
}

void a2dOglDrawer::DeleteShapeIds()
{    
    for ( a2dShapeIdVector::iterator shapeIdsit= m_shapeIds.begin(); shapeIdsit!= m_shapeIds.end(); ++shapeIdsit)
    {
        DeleteShapeIds( *shapeIdsit );
    }
    m_shapeIds.clear();
}

void a2dOglDrawer::DeleteShapeIds( const a2dListId& Ids )
{
    if ( Ids.m_start )
    {
        glDeleteLists( Ids.m_start, Ids.m_offset );
    }
}

void a2dOglDrawer::DrawLines( const a2dVertexList* list, bool spline )
{
    if ( m_disableDrawing )
        return;

    glPushMatrix();

    float mm[] = { m_usertodevice.GetValue(0,0), m_usertodevice.GetValue(1,0), 0.0f, 0.0f,
                    m_usertodevice.GetValue(0,1), m_usertodevice.GetValue(1,1), 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    m_usertodevice.GetValue(2,0),  m_usertodevice.GetValue(2,1), 0.0f, 1.0f };                  
    glLoadMatrixf( mm );
    glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );

    // create one display list
    GLuint stroke = glGenLists(1);
    m_shapeId.m_stroke = stroke;
    m_shapeId.m_start = stroke;
    m_shapeId.m_offset = 1;
    glNewList( stroke, GL_COMPILE);

    //a2dDrawer2D::DrawLines( list, spline );  

    DrawPolyOgl( list, GL_LINE_STRIP );

    glEndList();
    glCallList( stroke );

    glLoadIdentity();
    glPopMatrix();
    if ( !m_preserveCache )
    {
        DeleteShapeIds( m_shapeId );  
        m_shapeId = a2dListId();
    }
    //else
    //    m_shapeIds.push_back( m_shapeId );
}

void a2dOglDrawer::DrawLines( a2dVertexArray* points, bool spline )
{
    m_shapeId = a2dListId();
    if ( m_disableDrawing )
        return;

    //a2dDrawer2D::DrawLines( points, spline );    

    glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );
    DrawPolyOgl( points, GL_LINE_STRIP );
}

void a2dOglDrawer::DrawPolygon( a2dVertexArray* points, bool spline, wxPolygonFillMode fillStyle )
{
    m_shapeId = a2dListId();
    if ( m_disableDrawing )
        return;

    glPushMatrix();

    float mm[] = { m_usertodevice.GetValue(0,0), m_usertodevice.GetValue(1,0), 0.0f, 0.0f,
                    m_usertodevice.GetValue(0,1), m_usertodevice.GetValue(1,1), 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    m_usertodevice.GetValue(2,0),  m_usertodevice.GetValue(2,1), 0.0f, 1.0f };                  
    glLoadMatrixf( mm );

    // create one display list
    GLuint polygon = glGenLists(1);
    m_shapeId.m_offset = 1;
    m_shapeId.m_start = polygon;
    glNewList( polygon, GL_COMPILE);
    DrawPolyOgl( points, GL_POLYGON );
    glEndList();

    glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, m_FillOpacityCol1 / 255.0 );

    GLuint stroke = 0;
    GLuint fill = glGenLists(1);
    m_shapeId.m_fill = fill;
    m_shapeId.m_offset++;
    glNewList( fill, GL_COMPILE);

    //int segments;
    //if ( spline )
    //{
    //    a2dVertexArray* splinedlist = points->ConvertSplinedPolyline( m_splineaberration );
    //    segments = ToAggPath( splinedlist, false );
    //    delete splinedlist;
    //}
    //else
    //    segments = ToAggPath( array, false );

    //if ( segments == 0 )
    //    return;

    if ( IsStrokeOnly() )
    {
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
    {
        a2dDrawer2D::DrawPolygon( points, spline, fillStyle );
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        a2dDrawer2D::DrawPolygon( points, spline, fillStyle );
    }
    else if ( m_activefill.GetStyle() >= a2dFIRST_HATCH && m_activefill.GetStyle() <= a2dLAST_HATCH )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetStyle() >= a2dFIRST_TWOCOL_HATCH && m_activefill.GetStyle() <= a2dLAST_TWOCOL_HATCH )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glPolygonStipple( m_stippleInverse );
        glColor4f( m_colour2redFill / 255.0, m_colour2greenFill / 255.0, m_colour2blueFill / 255.0, m_FillOpacityCol2 / 255.0 );
        glCallList( polygon );
        glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, m_FillOpacityCol1 / 255.0 );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP && m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP && m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glPolygonStipple( m_stippleInverse );
        glColor4f( m_colour2redFill / 255.0, m_colour2greenFill / 255.0, m_colour2blueFill / 255.0, m_FillOpacityCol2 / 255.0 );
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else //normal solid case
    {
        glCallList( polygon );
    }
    glEndList();
    glCallList( fill );

    if (  IsStroked()  )
    {
        glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );
        stroke = glGenLists(1);
        m_shapeId.m_stroke = stroke;
        m_shapeId.m_offset++;
        glNewList( stroke, GL_COMPILE);

        DrawPolyOgl( points, GL_LINE_LOOP );
        glEndList();
        glCallList( stroke );
    }

    glLoadIdentity();
    glPopMatrix();

    if ( !m_preserveCache )
    {
        DeleteShapeIds( m_shapeId );  
        m_shapeId = a2dListId();
    }
    //else
    //    m_shapeIds.push_back( m_shapeId );
}

void a2dOglDrawer::DrawPolygon( const a2dVertexList* list, bool spline, wxPolygonFillMode fillStyle )
{
    m_shapeId = a2dListId();
    if ( m_disableDrawing )
        return;

    glPushMatrix();

    float mm[] = { m_usertodevice.GetValue(0,0), m_usertodevice.GetValue(1,0), 0.0f, 0.0f,
                    m_usertodevice.GetValue(0,1), m_usertodevice.GetValue(1,1), 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    m_usertodevice.GetValue(2,0),  m_usertodevice.GetValue(2,1), 0.0f, 1.0f };                  
    glLoadMatrixf( mm );

    // create one display list
    GLuint polygon = glGenLists(1);
    m_shapeId.m_start = polygon;
    m_shapeId.m_offset = 1;
    glNewList( polygon, GL_COMPILE);
    DrawPolyOgl( list, GL_POLYGON );
    glEndList();

    glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, m_FillOpacityCol1 / 255.0 );

    GLuint stroke = 0;
    GLuint fill = glGenLists(1);
    m_shapeId.m_fill = fill;
    m_shapeId.m_offset++;
    glNewList( fill, GL_COMPILE);

    //a2dDrawer2D::DrawPolygon( list, spline, fillStyle );    

    if ( IsStrokeOnly() )
    {
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
    {
        a2dDrawer2D::DrawPolygon( list, spline, fillStyle );
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        a2dDrawer2D::DrawPolygon( list, spline, fillStyle );
    }
    else if ( m_activefill.GetStyle() >= a2dFIRST_HATCH && m_activefill.GetStyle() <= a2dLAST_HATCH )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetStyle() >= a2dFIRST_TWOCOL_HATCH && m_activefill.GetStyle() <= a2dLAST_TWOCOL_HATCH )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glPolygonStipple( m_stippleInverse );
        glColor4f( m_colour2redFill / 255.0, m_colour2greenFill / 255.0, m_colour2blueFill / 255.0, m_FillOpacityCol2 / 255.0 );
        glCallList( polygon );
        glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, m_FillOpacityCol1 / 255.0 );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP && m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP && m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glPolygonStipple( m_stippleInverse );
        glColor4f( m_colour2redFill / 255.0, m_colour2greenFill / 255.0, m_colour2blueFill / 255.0, m_FillOpacityCol2 / 255.0 );
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else //normal solid case
    {
        glCallList( polygon );
    }
    glEndList();
    glCallList( fill );
    
    if (  IsStroked()  )
    {
        glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );
        stroke = glGenLists(1);
        m_shapeId.m_offset++;
        m_shapeId.m_stroke = stroke;
        glNewList( stroke, GL_COMPILE);
        DrawPolyOgl( list, GL_LINE_LOOP );
        glEndList();
        glCallList( stroke );
    }

    glLoadIdentity();
    glPopMatrix();

    if ( !m_preserveCache )
    {
        DeleteShapeIds( m_shapeId );  
        m_shapeId = a2dListId();
    }
    //else
    //    m_shapeIds.push_back( m_shapeId );
}

void a2dOglDrawer::DrawCircle( double xc, double yc, double radius )
{
    m_shapeId = a2dListId();

    if ( m_disableDrawing )
        return;

    double dphi;
    unsigned int segments;

    if ( radius == 0 )
    {
        DrawPoint( xc, yc );
        return;
    }
/*   
    //circular approximation of radius.
    double radiusDev = m_usertodevice.TransformDistance( radius );
    Aberration( m_displayaberration, wxPI * 2, radiusDev , dphi, segments );

    // limit the number of segments on a circle
    //    if the circle is small only a few segments need to be drawn.
    //    ie for a circle of 4x4 pixels, roughly 8 segments are needed
    //    The number of segments could be tuned with a fiddle factor (now 1.0).
    //    n = (width_pixels + height_pixels) * fiddle_factor;
    //
    double m00 = m_usertodevice.GetValue( 0, 0 ) * radius;
    double m01 = m_usertodevice.GetValue( 0, 1 ) * radius;
    double m10 = m_usertodevice.GetValue( 1, 0 ) * radius;
    double m11 = m_usertodevice.GetValue( 1, 1 ) * radius;
    double n = fabs( m00 + m10 ) + fabs( m01 + m11 ); // * fiddle_factor
    if ( segments > n )
    {
        segments = ( int ) ceil( n );
        dphi = 2.0 * wxPI / double(segments);
    }
*/
    segments = 36;
    dphi = 2.0 * wxPI / double(segments);

	float c = cosf(dphi);//precalculate the sine and cosine
	float s = sinf(dphi);
	float t;

	float x = radius;//we start at angle = 0 
	float y = 0;

    double dx,dy;

    glPushMatrix();

    float mm[] = { m_usertodevice.GetValue(0,0), m_usertodevice.GetValue(1,0), 0.0f, 0.0f,
                    m_usertodevice.GetValue(0,1), m_usertodevice.GetValue(1,1), 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    m_usertodevice.GetValue(2,0),  m_usertodevice.GetValue(2,1), 0.0f, 1.0f };                  
    glLoadMatrixf( mm );

    // create one display list
    GLuint polygon = glGenLists(1);
    m_shapeId.m_start = polygon;
    m_shapeId.m_offset = 1;
    glNewList( polygon, GL_COMPILE);
	    glBegin(GL_POLYGON); 
	    for(int ii = 0; ii < segments; ii++) 
	    { 
            //m_usertodevice.TransformPoint( x + xc, y + yc, dx, dy );
            dx = x + xc;
            dy = y + yc;
		    glVertex3f(dx, dy, m_layer / a2dNormDepth );//output vertex 
        
		    //apply the rotation matrix
		    t = x;
		    x = c * x - s * y;
		    y = s * t + c * y;
	    } 
	    glEnd(); 
    glEndList();

    glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, m_FillOpacityCol1 / 255.0 );

    GLuint stroke = 0;
    GLuint fill = glGenLists(1);
    m_shapeId.m_fill = fill; 
    m_shapeId.m_offset++;
    glNewList( fill, GL_COMPILE);

    if ( IsStrokeOnly() )
    {
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
    {
        DrawEllipse( xc, yc, 2.0 * radius, 2.0 * radius );
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        DrawEllipse( xc, yc, 2.0 * radius, 2.0 * radius );
    }
    else if ( m_activefill.GetStyle() >= a2dFIRST_HATCH && m_activefill.GetStyle() <= a2dLAST_HATCH )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetStyle() >= a2dFIRST_TWOCOL_HATCH && m_activefill.GetStyle() <= a2dLAST_TWOCOL_HATCH )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glPolygonStipple( m_stippleInverse );
        glColor4f( m_colour2redFill / 255.0, m_colour2greenFill / 255.0, m_colour2blueFill / 255.0, m_FillOpacityCol2 / 255.0 );
        glCallList( polygon );
        glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, m_FillOpacityCol1 / 255.0 );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP && m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP && m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glPolygonStipple( m_stippleInverse );
        glColor4f( m_colour2redFill / 255.0, m_colour2greenFill / 255.0, m_colour2blueFill / 255.0, m_FillOpacityCol2 / 255.0 );
	    glBegin(GL_POLYGON); 
        glCallList( polygon );
        glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, m_FillOpacityCol1 / 255.0 );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else //normal solid case
    {
        glCallList( polygon );
    }
    glEndList();
    glCallList( fill );

    if (  IsStroked()  )
    {
        glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );
        stroke = glGenLists(1);
        m_shapeId.m_stroke = stroke; 
        m_shapeId.m_offset++;
        glNewList( stroke, GL_COMPILE);

	    glBegin(GL_LINE_LOOP); 
	    for(int ii = 0; ii < segments; ii++) 
	    { 
            //m_usertodevice.TransformPoint( x + xc, y + yc, dx, dy );
            dx = x + xc;
            dy = y + yc;
		    glVertex3f(dx, dy, m_layer / a2dNormDepth );//output vertex 
        
		    //apply the rotation matrix
		    t = x;
		    x = c * x - s * y;
		    y = s * t + c * y;
	    } 
	    glEnd(); 

        glEndList();
        glCallList( stroke );
    }

    glLoadIdentity();
    glPopMatrix();

    if ( !m_preserveCache )
    {
        DeleteShapeIds( m_shapeId );  
        m_shapeId = a2dListId();
    }
    //else
    //    m_shapeIds.push_back( m_shapeId );
}

void a2dOglDrawer::DrawEllipse( double xc, double yc, double width, double height )
{
    if ( m_disableDrawing )
        return;

    a2dDrawer2D::DrawEllipse( xc, yc, width, height );
}

void a2dOglDrawer::DrawLine( double x1, double y1, double x2, double y2 )
{
    if ( m_disableDrawing )
        return;

    glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );

    glPushMatrix();

    float mm[] = { m_usertodevice.GetValue(0,0), m_usertodevice.GetValue(1,0), 0.0f, 0.0f,
                    m_usertodevice.GetValue(0,1), m_usertodevice.GetValue(1,1), 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    m_usertodevice.GetValue(2,0),  m_usertodevice.GetValue(2,1), 0.0f, 1.0f };                  
    glLoadMatrixf( mm );

    //ouble dx, dy;
    //m_usertodevice.TransformPoint( x1, y1, dx, dy );
    //glVertex3f( dx, dy, m_layer / a2dNormDepth );
    //m_usertodevice.TransformPoint( x2, y2, dx, dy );
    //glVertex3f( dx, dy, m_layer / a2dNormDepth );

    glBegin(GL_LINE_STRIP);
    glVertex3f( x1, y1, m_layer / a2dNormDepth );
    glVertex3f( x2, y2, m_layer / a2dNormDepth );
    glEnd();

    glLoadIdentity();
    glPopMatrix();


    //a2dDrawer2D::DrawLine( x1, y1, x2, y2 );
}


void a2dOglDrawer::DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize )
{
    m_shapeId = a2dListId();

    if ( m_disableDrawing )
        return;

    if ( height < 0 )
    {
        y += height;
        height = -height;
    }
    if ( width < 0 )
    {
        x += width;
        width = -width;
    }

    if ( radius || pixelsize )
    {
        a2dDrawer2D::DrawRoundedRectangle( x, y, width, height, radius, pixelsize );
        return;
    }

    glPushMatrix();

    float mm[] = { m_usertodevice.GetValue(0,0), m_usertodevice.GetValue(1,0), 0.0f, 0.0f,
                    m_usertodevice.GetValue(0,1), m_usertodevice.GetValue(1,1), 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    m_usertodevice.GetValue(2,0),  m_usertodevice.GetValue(2,1), 0.0f, 1.0f };                  
    glLoadMatrixf( mm );

    double x1, y1, x2, y2, x3, y3, x4, y4;
    //m_usertodevice.TransformPoint( x, y, x1, y1 );
    //m_usertodevice.TransformPoint( x + width, y, x2, y2 );
    //m_usertodevice.TransformPoint( x + width, y + height, x3, y3 );
    //m_usertodevice.TransformPoint( x, y + height, x4, y4 );
    x1 = x;
    y1 = y;
    x2 = x + width;
    y2 = y;
    x3 = x + width;
    y3 = y + height;
    x4 = x;
    y4 = y + height;

    GLuint stroke = 0;
    GLuint polygon = glGenLists(1);
    m_shapeId.m_start = polygon;
    m_shapeId.m_offset = 1;
    glNewList( polygon, GL_COMPILE);
        glBegin(GL_POLYGON);
        glVertex3f( x1, y1, m_layer / a2dNormDepth );
        glVertex3f( x2, y2, m_layer / a2dNormDepth );
        glVertex3f( x3, y3, m_layer / a2dNormDepth );
        glVertex3f( x4, y4, m_layer / a2dNormDepth );
        glEnd();
    glEndList();

    glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, m_FillOpacityCol1 / 255.0 );

    GLuint fill = glGenLists(1);
    glNewList( fill, GL_COMPILE);
    m_shapeId.m_fill = fill; 
    m_shapeId.m_offset++;

    if ( IsStrokeOnly() )
    {
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
    {
        a2dDrawer2D::DrawRoundedRectangle( x, y, width, height, radius, pixelsize );
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        a2dDrawer2D::DrawRoundedRectangle( x, y, width, height, radius, pixelsize );
    }
    else if ( m_activefill.GetStyle() >= a2dFIRST_HATCH && m_activefill.GetStyle() <= a2dLAST_HATCH )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetStyle() >= a2dFIRST_TWOCOL_HATCH && m_activefill.GetStyle() <= a2dLAST_TWOCOL_HATCH )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glPolygonStipple( m_stippleInverse );
        glColor4f( m_colour2redFill / 255.0, m_colour2greenFill / 255.0, m_colour2blueFill / 255.0, m_FillOpacityCol2 / 255.0 );
        glCallList( polygon );
        glColor4f( m_colour1redFill / 255.0, m_colour1greenFill / 255.0, m_colour1blueFill / 255.0, m_FillOpacityCol1 / 255.0 );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP && m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP && m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glPolygonStipple( m_stippleInverse );
        glColor4f( m_colour2redFill / 255.0, m_colour2greenFill / 255.0, m_colour2blueFill / 255.0, m_FillOpacityCol2 / 255.0 );
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP )
    {
        glPolygonStipple( m_stipple );
        glEnable(GL_POLYGON_STIPPLE);
        glPointSize(1.0);
        glCallList( polygon );
        glDisable(GL_POLYGON_STIPPLE);
    }
    else //normal solid case
    {
        glCallList( polygon );
    }
    glEndList();
    glCallList( fill );

    if (  IsStroked()  )
    {
        glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );
        stroke = glGenLists(1);
        m_shapeId.m_stroke = stroke; 
        m_shapeId.m_offset++;
        glNewList( stroke, GL_COMPILE);

	    glBegin(GL_LINE_LOOP); 
        glVertex3f( x1, y1, m_layer / a2dNormDepth );
        glVertex3f( x2, y2, m_layer / a2dNormDepth );
        glVertex3f( x3, y3, m_layer / a2dNormDepth );
        glVertex3f( x4, y4, m_layer / a2dNormDepth );
        glEnd();

        glEndList();
        glCallList( stroke );
    }

    glLoadIdentity();
    glPopMatrix();

    if ( !m_preserveCache )
    {
        DeleteShapeIds( m_shapeId );  
        m_shapeId = a2dListId();
    }
    //else
    //    m_shapeIds.push_back( m_shapeId );
}

void a2dOglDrawer::DrawPoint( double xc, double yc )
{
    if ( m_disableDrawing )
        return;

    double xt, yt;
    m_usertodevice.TransformPoint( xc, yc, xt, yt );

    glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );

    //glHint(GL_POINT_SMOOTH, GL_NICEST);
    //glEnable(GL_POINT_SMOOTH);
    glPointSize(1.0f);
    glBegin(GL_POINTS);
        glVertex3f(xt, yt, m_layer / a2dNormDepth );
    glEnd();

}

void a2dOglDrawer::DrawTextDc( const wxString& text, double x, double y )
{

#if 0
    a2dFont oldfont = m_currentfont;
    m_currentfont = m_currentfont.GetFreetypeFont();
    DrawTextFreetype( text, x, y );
    m_currentfont = oldfont;
#else
    double dx = m_usertodevice.GetValue( 1, 0 );
    double dy = m_usertodevice.GetValue( 1, 1 );

    // Get font size in device units
    unsigned int fontsize;
    fontsize = ( unsigned int ) fabs( m_currentfont.GetSize() * sqrt( dx * dx + dy * dy ) );
    if ( fontsize < 1 )
        fontsize = 1;
    //m_currentfont.GetFont().SetPointSize( fontsize / 1.3 );
    m_currentfont.GetFont().SetPointSize( fontsize );

    DrawTextGeneric( text, x, y, ( void ( a2dDrawer2D::* )( wxChar ) ) & a2dDrawer2D::DrawCharDcCb );
#endif
}


#if wxART2D_USE_FREETYPE
extern FT_Library g_freetypeLibrary;

//! used in freetype rendering
typedef struct
{
    wxColour colour;
    int xmin, ymax, ymin;
    int stride;
    int height;
    int descent;
    unsigned char* buf;
    int sizebuf;
    bool modeUp; 
}
a2dSpanData;

//! used in freetype rendering
static void a2dSpanFuncGray( int y, int count, FT_Span* spans, a2dSpanData* user )
{
    unsigned int alpha, len;
    // although colours are chars, using ints is faster in multiplications.
    unsigned int r, g, b;
    r = user->colour.Red();
    g = user->colour.Green();
    b = user->colour.Blue();

    unsigned char* buf, *buf2;

    int scanLine;

    //mirror in Y or not
    if ( user->modeUp )
        scanLine = ( user->height - user->descent - y ) * user->stride + user->xmin * 4;
    else
        scanLine = ( user->ymax - y ) * user->stride - user->xmin * 4;

    buf = user->buf + scanLine;
    do
    {
        buf2 = buf + spans->x * 4;

        int pos;
        pos = scanLine + spans->x * 4 + spans->len;
        if ( pos > user->sizebuf || pos < 0 )
            return;

        len = spans->len;
        alpha = spans->coverage;
        while( len )
        {
            // do not try to replace by *buf2++ = r, etc.
            // it turns out that this code actually is faster (for x86).
            buf2[0] = r; 
            buf2[1] = g;
            buf2[2] = b;
            buf2[3] = alpha;
            buf2 += 4;
            len--;
        }
        spans++;
    }
    while ( --count );
}
#endif

#define BBOX_GLYPH_EXTEND 1

//#undef __WXMSW__

#if wxART2D_USE_FREETYPE

void DrawCharFreetypeHorizontalOgl( wxColour col, const a2dGlyph* a2dglyph, unsigned char* buf, int bufwidth, int bufheight, wxChar c, int dx, int deviceDescent, FT_BBox bbox )
{
    // Use next to test on screen a2dDcDrawer vpath drawing for printing.
    //a2dDcDrawer::DrawCharFreetype( c );
    //return;

    FT_Glyph image = a2dglyph->m_glyph;

    // If size == 0, nothing has to be drawn.
    if ( bbox.xMax - bbox.xMin <= 0 || bbox.yMax - bbox.yMin <= 0 )
    {
        return;
    }

    // Render glyph to an image
    if ( image->format == FT_GLYPH_FORMAT_OUTLINE )
    {
        a2dSpanData spandata;
        spandata.colour = col;
        spandata.xmin = dx;
        spandata.height = bufheight;
        spandata.descent = deviceDescent;
        spandata.ymin = bbox.yMin;
        spandata.ymax = bbox.yMax;
        spandata.stride = bufwidth * 4;
        spandata.buf = buf;
        spandata.sizebuf = bufheight * bufwidth * 4;
        spandata.modeUp = true;
        wxASSERT( spandata.buf != NULL );

        FT_Raster_Params params;
        params.target = NULL;
        params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT | FT_RASTER_FLAG_CLIP;
        params.gray_spans = ( FT_SpanFunc ) & a2dSpanFuncGray;
        params.black_spans = NULL;
        params.bit_test = ( FT_Raster_BitTest_Func ) NULL;
        params.bit_set = ( FT_Raster_BitSet_Func ) NULL;
        params.user = &spandata;
        params.clip_box = bbox;

        FT_Outline& outline = ( ( FT_OutlineGlyph ) image )->outline;
        if ( FT_Outline_Render( g_freetypeLibrary, &outline, &params ) == 0 )
        {
        }
    }
    else if ( image->format == FT_GLYPH_FORMAT_BITMAP )
    {
        wxFAIL_MSG( _( "Non vector bitmap fonts are not supported" ) );
    }
    else
        wxFAIL_MSG( _( "Non vector fonts are not supported" ) );

}
#endif // wxART2D_USE_FREETYPE

void a2dOglDrawer::DrawTextFreetype( const wxString& text, double x, double y )
{
    glColor4f( m_colour1redStroke / 255.0, m_colour1greenStroke / 255.0, m_colour1blueStroke / 255.0, m_StrokeOpacityCol1 / 255.0 );

    // if text font is normalized, or we have on even scaled and/or rotated text, we go for
    // no device draw of text

    if ( m_forceNormalizedFont || ! m_usertodevice.IsTranslateScaleIso() )
    {
        DrawTextGeneric( text, x, y, ( void ( a2dDrawer2D::* )( wxChar ) ) & a2dDrawer2D::DrawCharFreetypeCb );
        return;
    }


#if wxART2D_USE_FREETYPE

    // Draw text at device height using hinting and all, so small size text looks fine.
    const double lineh = m_currentfont.GetLineHeight();
    bool textwasvisible = false;

    m_usertodevice = m_worldtodevice * m_usertoworld;

    double hdx, hdy;
    int yBboxMinY;
    m_usertodevice.TransformPoint( 0.0, 0.0, hdx, hdy );
    yBboxMinY = Round( hdy );

    // x,y on lower-left point of bbox.
    // Add offset, to put anchor at text start
    double worldDescent = m_currentfont.GetDescent();
    m_usertodevice *= a2dAffineMatrix(  0.0, worldDescent );

    // scale = m_currentfont.GetSize() / deviceh;
    const a2dGlyph* a2dglyphprev = NULL;

    wxColour stroke = m_currentstroke.GetColour();//wxColour( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
    
    int dx, dy;
    m_usertodevice.TransformPoint( 0.0, 0.0, hdx, hdy );
    dx = Round( hdx ); dy = Round( hdy ); 

    // compensate 0 pixel at low y, and high y 1 pixel, 1 extra here.
    wxUint16 deviceh = ( wxUint16 ) ceil( m_usertodevice.TransformDistance( lineh ) ) + 1;
    wxUint16 deviceDescent = ( wxUint16 ) ceil( m_usertodevice.TransformDistance( worldDescent ) ) + 1; // one lower

    int dxstartrel = 0;
    int dxrel = 0;

    // first how long in x is the image we need.
#if wxUSE_UNICODE
    const wxStringCharType *cp(text.wx_str());
#else
    const wxChar* cp(text.c_str());
#endif // wxUSE_UNICODE
    wxChar c = 0;
    size_t n = text.Length();
    for ( size_t i = 0; i < n; i++ )
    {
        c = *cp++;
        const a2dGlyph* a2dglyph = m_currentfont.GetGlyphFreetype( c );
        // The glyph is now grid fitted and hinted.
        if( !a2dglyph )
            return;

        if ( i > 0 )
        {
            FT_Face  face = a2dglyph->m_face;
            FT_Vector  kern;
            FT_Get_Kerning( a2dglyph->m_face, a2dglyphprev->m_index, a2dglyph->m_index, FT_KERNING_DEFAULT, &kern );
            dxrel += ( int ) ( kern.x / 64.0 );

            if ( a2dglyphprev->m_rsb - a2dglyph->m_lsb >= 32 )
                dxrel -= 1;
            else if (  a2dglyphprev->m_rsb - a2dglyph->m_lsb < -32 )
                dxrel += 1;
        }
        else
        {            
            FT_Glyph glyphimage = a2dglyph->m_glyph;
            FT_BBox  bbox;
            FT_Glyph_Get_CBox( glyphimage, FT_GLYPH_BBOX_PIXELS, &bbox );
            dxstartrel = bbox.xMin;
        }
        a2dglyphprev = a2dglyph;
        FT_Glyph glyphimage = a2dglyph->m_glyph;

        FT_BBox  bbox;
        FT_Glyph_Get_CBox( glyphimage, FT_GLYPH_BBOX_PIXELS, &bbox );

        int advance = ( int ) ( glyphimage->advance.x / 64.0 );
        dxrel += advance;
    }
    int devicew = dxrel - dxstartrel;
        
    wxRect stringBbox( dx, yBboxMinY - deviceh, devicew, deviceh );
    if ( stringBbox.GetWidth() == 0 || stringBbox.GetHeight() == 0 )
        return;

    int ppx = dx;
    int ppy = yBboxMinY - deviceh;
    int w = devicew;
    int h = deviceh;

    // Obtain background image
    GLuint textureId; // ID of texture
	//Allocate memory for the texture data.
	GLubyte* imagedata = new GLubyte[ 4 * w * h ];
    memset( imagedata, 0, 4 * w * h);

    dxrel = -dxstartrel;
#if wxUSE_UNICODE
    cp = text.wx_str();
#else
    cp = text.c_str();
#endif // wxUSE_UNICODE
    for ( size_t i = 0; i < n; i++ )
    {
        c = *cp++;
        const a2dGlyph* a2dglyph = m_currentfont.GetGlyphFreetype( c );
        // The glyph is now grid fitted and hinted.
        if( !a2dglyph )
            return;

        if ( i > 0 )
        {
            FT_Face  face = a2dglyph->m_face;
            FT_Vector  kern;
            FT_Get_Kerning( a2dglyph->m_face, a2dglyphprev->m_index, a2dglyph->m_index, FT_KERNING_DEFAULT, &kern );
            dxrel += ( int ) ( kern.x / 64.0 );

            if ( a2dglyphprev->m_rsb - a2dglyph->m_lsb >= 32 )
                dxrel -= 1;
            else if (  a2dglyphprev->m_rsb - a2dglyph->m_lsb < -32 )
                dxrel += 1;

        }
        a2dglyphprev = a2dglyph;

        FT_Glyph glyphimage = a2dglyph->m_glyph;

        FT_BBox  bbox;
        FT_Glyph_Get_CBox( glyphimage, FT_GLYPH_BBOX_PIXELS, &bbox );
        int px = dx + dxrel + bbox.xMin;
        int py = dy - bbox.yMax;
        int wbox = bbox.xMax - bbox.xMin;
        int hbox = bbox.yMax - bbox.yMin;

        int advance = ( int ) ( glyphimage->advance.x / 64.0 );

        if ( wbox || hbox )
        {
            wxRect bboxglyph( px, py, wbox, hbox );
            //bboxglyph.Inflate( 1 ); //if not, some redrawpixels missed.

            // check character bounding box in device coordinates
            // To optimize take take text height and advance width to check if characters needs to be drawn.
            // If not, the rest of the string is outside the clipping rectangle for sure.
            // With just the glyph boundingbox this is not possible, and each character needs to be checked separately
            wxRect bboxd( px, yBboxMinY - deviceh, advance, deviceh );

            // if character is not outside clipping box, draw it.
            if ( m_clipboxdev.Intersects( bboxd ) )
            {
                if ( m_clipboxdev.Intersects( bboxglyph ) )
                {
                    DrawCharFreetypeHorizontalOgl( stroke, a2dglyph, imagedata, devicew, deviceh, c, dxrel, deviceDescent, bbox );
                }
                textwasvisible = true;
            }
            else if ( textwasvisible )
            {
                //wxLogDebug("NO bboxd x=%d, y=%d w=%d h=%d", bboxd.GetLeft(), bboxd.GetTop(), bboxd.GetWidth(), bboxd.GetHeight() );
                // If characters of a string had been visible, and this character is not
                // visible, then so will all succeeding. i.o.w. we can stop drawing.
                break;
            }
        }
        dxrel += advance;
    }

    if ( textwasvisible )
    {
        PushIdentityTransform();
	    //Now we just setup some texture parameters.
        glEnable(GL_TEXTURE_2D);
        //glColor4f( 1,0,0,1 );
        glGenTextures(1, &textureId);
        glBindTexture( GL_TEXTURE_2D, textureId);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, imagedata );
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	    glBegin(GL_QUADS);
	    glTexCoord2f(0.0,0.0); glVertex3f(ppx,ppy, m_layer / a2dNormDepth );
	    glTexCoord2f(1.0,0.0); glVertex3f(ppx+w,ppy, m_layer / a2dNormDepth );
	    glTexCoord2f(1.0,1.0); glVertex3f(ppx+w,ppy+h, m_layer / a2dNormDepth );
	    glTexCoord2f(0.0,1.0); glVertex3f(ppx,ppy+h, m_layer / a2dNormDepth );
	    glEnd();

        glDisable(GL_TEXTURE_2D);
        glDeleteTextures( 1, &textureId );
        PopTransform();
   }
   delete [] imagedata;

#else // wxART2D_USE_FREETYPE
    DrawTextGeneric( text, x, y, ( void ( a2dDrawer2D::* )( wxChar ) ) & a2dDrawer2D::DrawCharFreetypeCb );
#endif //wxART2D_USE_FREETYPE

}

/*
void a2dOglDrawer::DrawCharFreetype( wxChar c )
{
    double x, y;
    x = y = 0.0;

    if ( m_disableDrawing )
        return;

    // scale font to size
    // mirror text, depending on y-axis orientation
    const double h = m_currentfont.GetLineHeight();

#if wxART2D_USE_FREETYPE
    a2dAffineMatrix affine;
    if ( !GetYaxis() )
    {
        //affine.Translate( 0.0, -h );
        //affine.Mirror( true, false );
    }
    // position text, works but only translation done.
    //affine.Scale( scale );
    //affine.Translate( x, y );

    a2dVpath vpath;
    m_currentfont.GetVpath( c, vpath, affine );

    // save context
    PushTransform( affine );

    // the resulting vector path contains contours and holes.
    // Like character 'O' contains on contour and one hole.
    // Contours are clockwise, while holes or counter clockwise.
    // Using PolyPolygon, we can draw such polygons correctly.
    //DrawVpath( &vpath );

    //Text is drawn filled with the current pen.
    //Brush is used earlier to draw a rectangular background.

    a2dListOfa2dVertexList onlyPolygons;
    vpath.ConvertToPolygon( onlyPolygons, false );
    DrawPolyPolygon( onlyPolygons );

    // restore context
    PopTransform();
#else // wxART2D_USE_FREETYPE
    a2dDrawer2D::DrawCharFreetype( c );
#endif // wxART2D_USE_FREETYPE
}
*/

void a2dOglDrawer::DrawCharFreetype( wxChar c )
{
#if wxART2D_USE_FREETYPE
    // Add offset, to put anchor on lower-left point of bbox.
    double wdx, wdy;
    m_usertodevice.TransformPoint( 0.0, m_currentfont.GetDescent(), wdx, wdy );
    int dx = Round( wdx );
    int dy = Round( wdy );
    //int dx = ( wdx );
    //int dy = ( wdy );

    FT_Matrix trans_matrix;
    FT_Vector vec;
    if ( 1 ) // ! m_usertodevice.IsTranslateScaleIso() )
    {
        // Generate affine, to scale character from normalized to real size.
        a2dAffineMatrix glyphPos;
        glyphPos.Scale( m_currentfont.GetSize() / m_currentfont.GetDeviceHeight() );
        a2dAffineMatrix glyphToDevice = m_usertodevice * glyphPos;

        // Transform glyph and scale it to device coordinates. Also correct for sub-pixel drawing.
        trans_matrix.xx = ( FT_Fixed ) ( glyphToDevice.GetValue( 0, 0 ) * 0x10000 );
        trans_matrix.xy = ( FT_Fixed ) ( glyphToDevice.GetValue( 1, 0 ) * 0x10000 );
        trans_matrix.yx = ( FT_Fixed ) ( -glyphToDevice.GetValue( 0, 1 ) * 0x10000 );
        trans_matrix.yy = ( FT_Fixed ) ( -glyphToDevice.GetValue( 1, 1 ) * 0x10000 );
        vec.x = ( int ) ( ( wdx - dx ) * 64.0 );
        vec.y = ( int ) ( ( wdy - dy ) * -64.0 );
    }
    else
    {
        trans_matrix.xx = ( FT_Fixed )( 0x10000L );
        trans_matrix.xy = ( FT_Fixed )( 0 );
        trans_matrix.yx = ( FT_Fixed )( 0 );
        trans_matrix.yy = ( FT_Fixed )( 0x10000L );
        vec.x = 0;
        vec.y = 0;
    }

    wxColour stroke = m_currentstroke.GetColour();//wxColour( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
    DrawCharFreetype( stroke, c, dx, dy, trans_matrix, vec );
#else // wxART2D_USE_FREETYPE
    DrawCharUnknown( c );
#endif // wxART2D_USE_FREETYPE

}


#if wxART2D_USE_FREETYPE

void a2dOglDrawer::DrawCharFreetype( wxColour col, wxChar c, int dx, int dy, FT_Matrix trans_matrix, FT_Vector vec )
{
    // Use next to test on screen a2dDcDrawer vpath drawing for printing.
    //a2dDcDrawer::DrawCharFreetype( c );
    //return;


    // Get the glyph and make a copy of it
    const a2dGlyph* a2dglyph = m_currentfont.GetGlyphFreetype( c );
    // The glyph is now grid fitted and hinted.
    if( !a2dglyph )
        return;
    FT_Glyph image;
    if ( FT_Glyph_Copy( a2dglyph->m_glyph, &image ) != 0 )
        return;

    // we need to round to make sure the bitmap is shifted only an integer pixel quantity
    // translating grid fitted/hinted outline should be N pixels.

    if ( FT_Glyph_Transform( image, &trans_matrix, &vec ) != 0 )
    {
        wxFAIL_MSG( _( "glyph transform wrong" ) );
        FT_Done_Glyph( image );
        return;
    }

    // Get bounding box of the area which should be drawn.
    FT_BBox bbox;
    FT_Glyph_Get_CBox( image, FT_GLYPH_BBOX_PIXELS, &bbox );

    // clip glyph device box against device drawing clipbox
    if ( m_clipboxdev.GetLeft() > dx + bbox.xMin )
        bbox.xMin = m_clipboxdev.GetLeft() - dx;
    if ( m_clipboxdev.GetRight() < dx + bbox.xMax )
        bbox.xMax = m_clipboxdev.GetRight() - dx;

    if ( m_clipboxdev.GetTop() > dy - bbox.yMax )
        bbox.yMax = dy - m_clipboxdev.GetTop();
    if ( m_clipboxdev.GetBottom() < dy - bbox.yMin )
        bbox.yMin = dy - m_clipboxdev.GetBottom();

    // If size == 0, nothing has to be drawn.
    if ( bbox.xMax - bbox.xMin <= 0 || bbox.yMax - bbox.yMin <= 0 )
    {
        FT_Done_Glyph( image );
        return;
    }
    // WHY: The glyph boundingbox is used to position and align text to draw (it is transformed)
    // This can rounded to one pixel more or less, coming from world coordinates.
    // It depends on the redraw area how the overlap of the glyph is rounded.
    // And therefore some small part of the text that should be (re)drawn in the clipping rectangle, is not.
    // This becomes a problem when glyph box is clipped to the whole drawing window, it sometimes a pixel off (even if just drawing rectangles and such)
    // Moving content of screen, it becomes visible.
    // When the update rectangle which fits completely in the window, it is not a problem, because it already has some extra pixels to redraw.
    // SOLUTION:
    // We assume the device clipping box is set to redraw part of the canvas (already made a few pixels more then needed, void a2dDrawingPart::RedrawPendingUpdateAreas() ).
    // The glyph bounding box part, which might overlap the redraw area (device clipping box), should be redrawn.
    // It does not hurt to extend the overlap part a pixel, as it will stay within the device clipping box.
    // We extend the boundingbox of a glyph, to make sure a bit more is drawn where needed.   
    // It seems adding one pixel like this enough, else add other also.
    //bbox.xMin -= BBOX_GLYPH_EXTEND;
    bbox.xMax += BBOX_GLYPH_EXTEND;
    //bbox.yMin -= BBOX_GLYPH_EXTEND;
    bbox.yMax += BBOX_GLYPH_EXTEND;

    //glyph boundingbox is clipped as if placed at right position, but not calculated/placed at device coordinate.
    //De bitmap we will render is (0,0,w,h) same size a glyph bbox

    int px = dx + bbox.xMin;
    int py = dy - bbox.yMax;
    int w = bbox.xMax - bbox.xMin + 1; //yes one more in pixels!
    int h = bbox.yMax - bbox.yMin + 1;

    // If size == 0, nothing has to be drawn.
    if ( bbox.xMax - bbox.xMin <= 0 || bbox.yMax - bbox.yMin <= 0 )
    {
        FT_Done_Glyph( image );
        return;
    }

    PushIdentityTransform();

    /* TO DEBUG
    glColor4f( 0,0,0, 255 );
    glBegin(GL_LINE_LOOP);
	glVertex3f(px,py, m_layer / a2dNormDepth );
	glVertex3f(px+w,py, m_layer / a2dNormDepth );
	glVertex3f(px+w,py+h, m_layer / a2dNormDepth );
	glVertex3f(px,py+h, m_layer / a2dNormDepth );
	glEnd();

    */

    // Obtain background image
    GLuint textureId; // ID of texture
	//Allocate memory for the texture data.
	GLubyte* imagedata = new GLubyte[ 4 * w * h];
    memset( imagedata, 0, 4 * w * h);

	//Now we just setup some texture parameters.
    glEnable(GL_TEXTURE_2D);
    //glColor4f( 1,0,0,1 );
    glGenTextures(1, &textureId);
    glBindTexture( GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render glyph to an image
    if ( image->format == FT_GLYPH_FORMAT_OUTLINE )
    {
        a2dSpanData spandata;
        spandata.colour = col;
        spandata.xmin = bbox.xMin;
        spandata.ymax = bbox.yMax;
        spandata.stride = w * 4;
        spandata.buf = imagedata;
        spandata.modeUp = false;
        spandata.sizebuf = w * h * 4;
        wxASSERT( spandata.buf != NULL );

        FT_Raster_Params params;
        params.target = NULL;
        params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT | FT_RASTER_FLAG_CLIP;
        params.gray_spans = ( FT_SpanFunc ) & a2dSpanFuncGray;
        params.black_spans = NULL;
        params.bit_test = ( FT_Raster_BitTest_Func ) NULL;
        params.bit_set = ( FT_Raster_BitSet_Func ) NULL;
        params.user = &spandata;
        params.clip_box = bbox;

        FT_Outline& outline = ( ( FT_OutlineGlyph ) image )->outline;
        if ( FT_Outline_Render( g_freetypeLibrary, &outline, &params ) == 0 )
        {   
            /* check board
            memset( imagedata, 0, 4 * w * h);
            for (int i = 0 ; i < w; i++ )
            {
                for (int j = 0 ; j < h; j++ )
                {
                    int x = ( j * w + i ) * 4;
                    if ( i % 8 < 4 && j % 8 < 4 )
                    {
                        imagedata[ x ] = 255;
                        imagedata[ x + 1] = 0;
                        imagedata[ x + 2] = 0;
                        imagedata[ x + 3 ] = 255;
                    }
                    else
                    {
                        imagedata[ x ] = 0;
                        imagedata[ x + 1] = 0;
                        imagedata[ x + 2] = 0;
                        imagedata[ x + 3 ] = 0;
                    }
                }
            }
            */


            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h,
		          0, GL_RGBA, GL_UNSIGNED_BYTE, imagedata );

	        glBegin(GL_QUADS);
	        glTexCoord2f(0.0,0.0); glVertex3f(px,py, m_layer / a2dNormDepth );
	        glTexCoord2f(1.0,0.0); glVertex3f(px+w,py, m_layer / a2dNormDepth );
	        glTexCoord2f(1.0,1.0); glVertex3f(px+w,py+h, m_layer / a2dNormDepth );
	        glTexCoord2f(0.0,1.0); glVertex3f(px,py+h, m_layer / a2dNormDepth );
	        glEnd();
        }
    }

    else
        wxFAIL_MSG( _( "Non vector fonts are not supported" ) );

    FT_Done_Glyph( image );

	//With the texture created, we don't need to expanded data anymore
    glDisable(GL_TEXTURE_2D);
    glDeleteTextures( 1, &textureId );
    delete [] imagedata;
    PopTransform();
}

#endif // wxART2D_USE_FREETYPE

void a2dOglDrawer::DrawTextStroke( const wxString& text, double x, double y )
{
    GLboolean on = glIsEnabled( GL_LINE_STIPPLE );
    glDisable (GL_LINE_STIPPLE);   
    double width = m_currentfont.GetStrokeWidth();
    double widthdevice = m_usertodevice.TransformDistance( width );
    glLineWidth( width );

    if ( widthdevice < 1 )//width 
        glLineWidth( 1 );
    else
        glLineWidth( widthdevice );

    DrawTextGeneric( text, x, y, &a2dDrawer2D::DrawCharStrokeCb ); 

    glLineWidth( m_strokewidthDev );

    if ( on )
        glEnable (GL_LINE_STIPPLE);   
}

void a2dOglDrawer::DrawCharStroke( wxChar c )
{
    // scale character to size
    a2dAffineMatrix affine;
    affine.Scale( m_currentfont.GetSize() );
    PushTransform( affine );

    a2dVertexList** ptr = m_currentfont.GetGlyphStroke( c );
    if ( ptr )
    {
        while ( *ptr )
        {
            DrawLines( *ptr, false );
            ptr++;
        }
    }

    // restore context
    PopTransform();
}

wxBitmap a2dOglDrawer::GetBuffer() const
{
    // Read the OpenGL image into a pixel array
    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    void* pixels = malloc(3 * view[2] * view[3]); // must use malloc
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer( GL_BACK_LEFT );
    glReadPixels(0, 0, view[2], view[3], GL_RGB, GL_UNSIGNED_BYTE, pixels);

    /*
    // Put the image into a wxImage
    m_buffer.SetData((unsigned char*) pixels);
    m_buffer = m_buffer.Mirror(false);
    */
    wxImage helpbuf  = wxImage( m_width, m_height );
    return wxBitmap( helpbuf );
}

wxBitmap a2dOglDrawer::GetSubBitmap( wxRect rect ) const
{
    wxImage helpbuf  = wxImage( m_width, m_height );
    return wxBitmap( helpbuf.GetSubImage( rect ) );
}

void a2dOglDrawer::SetBufferSize( int w, int h )
{
    m_width = w;
    m_height = h;
}


void a2dOglDrawerBuf::BlitBuffer( wxRect rect, const wxPoint& bufferpos )
{
    BeginDraw();
    if ( m_display )
    {
        wxClientDC deviceDC( m_display );
        m_display->PrepareDC( deviceDC );
        BlitBuffer( &deviceDC, rect, bufferpos );
    }
    EndDraw();
}

void a2dOglDrawer::BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos )
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

    if ( rect.x + rect.width > m_width )
    {
        rect.width = m_width - rect.x;
    }
    if ( rect.width <= 0 ) return;

    if ( rect.y + rect.height > m_height )
    {
        rect.height = m_height - rect.y;
    }
    if ( rect.height <= 0 ) return;
   
    //copy the main framebuffer to FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_FRONT);  
    glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_BACK);  
    glBlitFramebuffer(0, 0,  m_width, m_height, 0, 0, m_width, m_height,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    GLenum error = glGetError();    

    //glReadBuffer(GL_BACK);
    //glDrawBuffer(GL_FRONT);
    // set up raster pos
    //glCopyPixels( 0, 0, GetWidth(), GetHeight(), GL_COLOR);

    //dc->Blit( rect.x - bufferpos.x, rect.y - bufferpos.y, rect.width, rect.height, &m_memdc, rect.x, rect.y, wxCOPY, false );
}

void a2dOglDrawer::ShiftBuffer( int dxy, bool yshift )
{
    int bw = GetBuffer().GetWidth();
    int bh = GetBuffer().GetHeight();
    if ( yshift )
    {
        if ( dxy > 0 && dxy < bh )
        {
            GLuint t_textureId; // ID of texture
            GLuint t_depthId;       // ID of Renderbuffer object

            glGenRenderbuffers(1, &t_textureId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_textureId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);
            glBindFramebuffer(GL_FRAMEBUFFER, GL_FRONT);
            glGenRenderbuffers(1, &t_depthId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_depthId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height );
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // attach a texture to FBO color attachement point
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, t_textureId);
            // attach a renderbuffer to depth attachment point
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, t_depthId);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_FRONT);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_BACK);  
            glBlitFramebuffer( 0, 0, bw, bh - dxy, 0, dxy, bw, bh - dxy,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            GLenum error = glGetError();    

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_FRONT);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_BACK);  
            glBlitFramebuffer( 0, 0,  m_width, m_height, 0, 0,  m_width, m_height,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            glDeleteRenderbuffers(1, &t_textureId );
            glDeleteRenderbuffers(1, &t_depthId );

        }
        else  if ( dxy < 0 && dxy > -bh )
        {
            GLuint t_textureId; // ID of texture
            GLuint t_depthId;       // ID of Renderbuffer object

            glGenRenderbuffers(1, &t_textureId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_textureId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);

            // create a framebuffer object, you need to delete them when program exits.
            glBindFramebuffer(GL_FRAMEBUFFER, GL_FRONT);
            glGenRenderbuffers(1, &t_depthId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_depthId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height );
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // attach a texture to FBO color attachement point
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, t_textureId);
            // attach a renderbuffer to depth attachment point
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, t_depthId);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_FRONT);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_BACK);  
            glBlitFramebuffer( 0, -dxy, bw, bh + dxy, 0, 0, bw, bh + dxy,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            GLenum error = glGetError();    

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_FRONT);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_BACK);  
            glBlitFramebuffer( 0, 0,  m_width, m_height, 0, 0,  m_width, m_height,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            glDeleteRenderbuffers(1, &t_textureId );
            glDeleteRenderbuffers(1, &t_depthId );
        }
        else
            wxFAIL_MSG( wxT( "you can only shift within height of buffer" ) );

    }
    else
    {
        if ( dxy > 0 && dxy < bw )
        {
            GLuint t_textureId; // ID of texture
            GLuint t_depthId;       // ID of Renderbuffer object
            GLuint t_fboId;

            glGenRenderbuffers(1, &t_textureId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_textureId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);

            // create a framebuffer object, you need to delete them when program exits.
            glGenFramebuffers(1, &t_fboId);
            glBindFramebuffer(GL_FRAMEBUFFER, t_fboId);
            glGenRenderbuffers(1, &t_depthId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_depthId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height );
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // attach a texture to FBO color attachement point
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, t_textureId);
            // attach a renderbuffer to depth attachment point
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, t_depthId);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_FRONT);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_BACK);  
            glBlitFramebuffer( 0, 0, bw - dxy, bh, dxy, 0, bw - dxy, bh,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            GLenum error = glGetError();    

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_FRONT);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_BACK);  
            glBlitFramebuffer( 0, 0,  m_width, m_height, 0, 0,  m_width, m_height,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            glDeleteRenderbuffers(1, &t_textureId );
            glDeleteRenderbuffers(1, &t_depthId );
        }
        else if ( dxy < 0 && dxy > -bw )
        {
            GLuint t_textureId; // ID of texture
            GLuint t_depthId;       // ID of Renderbuffer object

            glGenRenderbuffers(1, &t_textureId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_textureId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);

            // create a framebuffer object, you need to delete them when program exits.
            glBindFramebuffer(GL_FRAMEBUFFER, GL_BACK);
            glGenRenderbuffers(1, &t_depthId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_depthId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height );
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // attach a texture to FBO color attachement point
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, t_textureId);
            // attach a renderbuffer to depth attachment point
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, t_depthId);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_FRONT);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_BACK);  
            glBlitFramebuffer( -dxy, 0, bw + dxy, bh, 0, 0, bw + dxy, bh,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            GLenum error = glGetError();    

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_FRONT);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_BACK);  
            glBlitFramebuffer( 0, 0,  m_width, m_height, 0, 0,  m_width, m_height,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            glDeleteRenderbuffers(1, &t_textureId );
            glDeleteRenderbuffers(1, &t_depthId );
        }
        else
            wxFAIL_MSG( wxT( "you can only shift within width of buffer" ) );
    }
}

void a2dOglDrawer::DeviceDrawPolygon( unsigned int n, bool spline , wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    PushIdentityTransform();
    if ( IsStrokeOnly() )
    {
        glBegin(GL_LINE_LOOP);

        int i;
        for ( i = 0; i < n; i++ )
        {
            glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, m_layer / a2dNormDepth );
        }
        glEnd();
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR || m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        FillPolygon( n, &m_cpointsDouble[0] );

        if ( IsStroked() )
        {
            glBegin(GL_LINE_LOOP);

            int i;
            for ( i = 0; i < n; i++ )
            {
                glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, m_layer / a2dNormDepth );
            }
            glEnd();
        }
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP && m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        glBegin(GL_POLYGON);

        int i;
        for ( i = 0; i < n; i++ )
        {
            glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, m_layer / a2dNormDepth );
        }
        glEnd();

        if ( IsStroked() )
        {
            glBegin(GL_LINE_LOOP);

            int i;
            for ( i = 0; i < n; i++ )
            {
                glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, m_layer / a2dNormDepth );
            }
            glEnd();
        }
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP &&
              ( m_activefill.GetStyle() == a2dFILL_STIPPLE ||
                m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE )
            )
    {
        glBegin(GL_POLYGON);

        int i;
        for ( i = 0; i < n; i++ )
        {
            glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, m_layer / a2dNormDepth );
        }
        glEnd();

        if ( IsStroked() )
        {
            glBegin(GL_LINE_LOOP);

            int i;
            for ( i = 0; i < n; i++ )
            {
                glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, m_layer / a2dNormDepth );
            }
            glEnd();
        }
    }
    else if ( m_activefill.GetType() == a2dFILL_HATCH_TWO_COLOUR )
    {
        glBegin(GL_POLYGON);

        int i;
        for ( i = 0; i < n; i++ )
        {
            glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, m_layer / a2dNormDepth );
        }
        glEnd();

        if ( IsStroked() )
        {
            glBegin(GL_LINE_LOOP);

            int i;
            for ( i = 0; i < n; i++ )
            {
                glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, m_layer / a2dNormDepth );
            }
            glEnd();
        }
    }
    else
    {
        glBegin(GL_POLYGON);

        int i;
        for ( i = 0; i < n; i++ )
        {
            glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, m_layer / a2dNormDepth );
        }
        glEnd();

        if ( IsStroked() )
        {
            glBegin(GL_LINE_LOOP);

            int i;
            for ( i = 0; i < n; i++ )
            {
                glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, m_layer / a2dNormDepth );
            }
            glEnd();
        }
    }

    if ( IsStroked() && m_activestroke.GetType() == a2dSTROKE_OBJECT )
    {
    }
    PopTransform();
}

void a2dOglDrawer::DeviceDrawLines( unsigned int n, bool spline )
{
    PushIdentityTransform();
    glBegin(GL_LINE_STRIP);

    int i;
    for ( i = 0; i < n; i++ )
    {
        glVertex3f( m_cpointsDouble[i].x, m_cpointsDouble[i].y, m_layer / a2dNormDepth );
    }
    glEnd();       
    PopTransform();
}

void a2dOglDrawer::DeviceDrawLine( double x1, double y1, double x2, double y2 )
{
    PushIdentityTransform();
    glBegin(GL_LINES);
    glVertex3f( x1, y1, m_layer / a2dNormDepth );
    glVertex3f( x2, y2, m_layer / a2dNormDepth );
    glEnd();       
    PopTransform();
}

void a2dOglDrawer::DeviceDrawHorizontalLine( int x1, int y1, int x2, bool WXUNUSED( use_pen ) )
{
    PushIdentityTransform();
    glBegin(GL_LINES);
    glVertex3f( x1, y1, m_layer / a2dNormDepth );
    glVertex3f( x2, y1, m_layer / a2dNormDepth );
    glEnd();       
    PopTransform();
}

void a2dOglDrawer::DeviceDrawVerticalLine( int x1, int y1, int y2, bool WXUNUSED( use_pen ) )
{
    PushIdentityTransform();
    glBegin(GL_LINES);
    glVertex3f( x1, y1, m_layer / a2dNormDepth );
    glVertex3f( x1, y2, m_layer / a2dNormDepth );
    glEnd();       
    PopTransform();
}

void a2dOglDrawer::DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
    glBegin(GL_POINTS);
    glVertex3f( x1, y1, m_layer / a2dNormDepth );
    glEnd();       
}

void a2dOglDrawer::DrawPolyOgl( a2dVertexArray* points, GLenum mode )
{
    if ( m_disableDrawing )
        return;

    //a2dDrawer2D::DrawPolygon( points, spline, fillStyle );    

    if ( mode == GL_POLYGON )
    {
        m_tesselation.clear();
        unsigned int segments = 0;
        unsigned int count = 0;
        double x, y, lastx, lasty;
        int i;
        for ( i = 0; i < points->size(); i++ )
        {
            const a2dLineSegment* seg = points->Item( i );

            if ( !seg->GetArc() )
            {
                //m_usertoworld.TransformPoint( seg->m_x, seg->m_y, x, y );
                //m_usertodevice.TransformPoint( seg->m_x, seg->m_y, x, y );
                x = seg->m_x;
                y = seg->m_y;

                //to reduce the points count for lines or a polygon on the screen
                if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                {
                    a2d3dVertex vertex( x, y, m_layer / a2dNormDepth );
                    m_tesselation.push_back(  vertex );
                    lastx = x;
                    lasty = y;
                    count++;
                }
            }
            else
            {
                a2dArcSegment* cseg = ( a2dArcSegment* ) seg;

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
                        //m_usertodevice.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                        x = center_x + radius * cos ( theta );
                        y = center_y + radius * sin ( theta );

                        //to reduce the points count for lines or a polygon on the screen
                        if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                        {
                            a2d3dVertex vertex( x, y, m_layer / a2dNormDepth );
                            m_tesselation.push_back( vertex );
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
                    //m_usertodevice.TransformPoint( cseg->m_x, cseg->m_y, x, y );
                    x = cseg->m_x;
                    y = cseg->m_y;

                    //to reduce the points count for lines or a polygon on the screen
                    if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                    {
                        a2d3dVertex vertex( x, y, m_layer / a2dNormDepth );
                        m_tesselation.push_back(  vertex );
                        lastx = x;
                        lasty = y;
                        count++;
                    }
                }
            }

        }

        GLUtesselator *tess = gluNewTess(); // create a tessellator
        if(!tess) return;         // failed to create tessellation object, return 0

        // register callback functions
        gluTessCallback(tess, GLU_TESS_BEGIN, (void (CALLBACK *)())tessBeginCB);
        gluTessCallback(tess, GLU_TESS_END, (void (CALLBACK *)())tessEndCB);
        gluTessCallback(tess, GLU_TESS_ERROR, (void (CALLBACK *)())tessErrorCB);
        gluTessCallback(tess, GLU_TESS_VERTEX, (void (CALLBACK *)())tessVertexCB);


        gluTessBeginPolygon(tess, 0);
        gluTessBeginContour(tess); 

        for ( i = 0; i < m_tesselation.size(); i++ )
        {
            gluTessVertex(tess, m_tesselation[i].m_v, m_tesselation[i].m_v);
        }
        gluTessEndContour(tess);
        gluTessEndPolygon(tess);

        gluDeleteTess(tess);        // delete after tessellation
    }
    else
    {
        glBegin( mode );

        unsigned int segments = 0;
        unsigned int count = 0;
        double x, y, lastx, lasty;
        int i;
        for ( i = 0; i < points->size(); i++ )
        {
            const a2dLineSegment* seg = points->Item( i );

            if ( !seg->GetArc() )
            {
                //m_usertoworld.TransformPoint( seg->m_x, seg->m_y, x, y );
                //m_usertodevice.TransformPoint( seg->m_x, seg->m_y, x, y );
                x = seg->m_x;
                y = seg->m_y;

                //to reduce the points count for lines or a polygon on the screen
                if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                {
                    glVertex3f( x, y, m_layer / a2dNormDepth );
                    lastx = x;
                    lasty = y;
                    count++;
                }
            }
            else
            {
                a2dArcSegment* cseg = ( a2dArcSegment* ) seg;

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
                        //m_usertodevice.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                        x = center_x + radius * cos ( theta );
                        y = center_y + radius * sin ( theta );

                        //to reduce the points count for lines or a polygon on the screen
                        if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                        {
                            glVertex3f( x, y, m_layer / a2dNormDepth );
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
                    //m_usertodevice.TransformPoint( cseg->m_x, cseg->m_y, x, y );
                    x = cseg->m_x;
                    y = cseg->m_y;

                    //to reduce the points count for lines or a polygon on the screen
                    if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                    {
                        glVertex3f( x, y, m_layer / a2dNormDepth );
                        lastx = x;
                        lasty = y;
                        count++;
                    }
                }
            }

        }
        glEnd();       
    }
}

void a2dOglDrawer::DrawPolyOgl( const a2dVertexList* list, GLenum mode )
{
    if ( m_disableDrawing )
        return;

    //a2dDrawer2D::DrawPolygon( list, spline, fillStyle );    

    if ( list->empty() )
        return;

    unsigned int segments = 0;

    if ( mode == GL_POLYGON )
    {
        m_tesselation.clear();
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
                //m_usertoworld.TransformPoint( seg->m_x, seg->m_y, x, y );
                //m_usertodevice.TransformPoint( seg->m_x, seg->m_y, x, y );
                x = seg->m_x;
                y = seg->m_y;
                //to reduce the points count for lines or a polygon on the screen
                if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                {
                    a2d3dVertex vertex( x, y, m_layer / a2dNormDepth );
                    m_tesselation.push_back(  vertex );
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
                        //m_usertodevice.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                        x = center_x + radius * cos ( theta );
                        y = center_y + radius * sin ( theta );

                        //to reduce the points count for lines or a polygon on the screen
                        if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                        {
                            a2d3dVertex vertex( x, y, m_layer / a2dNormDepth );
                            m_tesselation.push_back( vertex );
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
                    //m_usertodevice.TransformPoint( cseg->m_x, cseg->m_y, x, y );
                    x = cseg->m_x;
                    y = cseg->m_y;

                    //to reduce the points count for lines or a polygon on the screen
                    if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                    {
                        a2d3dVertex vertex( x, y, m_layer / a2dNormDepth );
                        m_tesselation.push_back(  vertex );
                        lastx = x;
                        lasty = y;
                        count++;
                    }
                }
            }
            iterprev = iter++;
        }

        GLUtesselator *tess = gluNewTess(); // create a tessellator
        if(!tess) return;         // failed to create tessellation object, return 0

        // register callback functions
        gluTessCallback(tess, GLU_TESS_BEGIN, (void (CALLBACK *)())tessBeginCB);
        gluTessCallback(tess, GLU_TESS_END, (void (CALLBACK *)())tessEndCB);
        gluTessCallback(tess, GLU_TESS_ERROR, (void (CALLBACK *)())tessErrorCB);
        gluTessCallback(tess, GLU_TESS_VERTEX, (void (CALLBACK *)())tessVertexCB);


        gluTessBeginPolygon(tess, 0);
        gluTessBeginContour(tess); 

        unsigned int i;
        for ( i = 0; i < m_tesselation.size(); i++ )
        {
            gluTessVertex(tess, m_tesselation[i].m_v, m_tesselation[i].m_v);
        }
        gluTessEndContour(tess);
        gluTessEndPolygon(tess);

        gluDeleteTess(tess);        // delete after tessellation
    }
    else
    {
        glBegin( mode );

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
                //m_usertoworld.TransformPoint( seg->m_x, seg->m_y, x, y );
                //m_usertodevice.TransformPoint( seg->m_x, seg->m_y, x, y );
                x = seg->m_x;
                y = seg->m_y;

                //to reduce the points count for lines or a polygon on the screen
                if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                {
                    glVertex3f( x, y, m_layer / a2dNormDepth );
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
                        //m_usertodevice.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                        x = center_x + radius * cos ( theta );
                        y = center_y + radius * sin ( theta );

                        //to reduce the points count for lines or a polygon on the screen
                        if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                        {
                            glVertex3f( x, y, m_layer / a2dNormDepth );
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
                    //m_usertodevice.TransformPoint( cseg->m_x, cseg->m_y, x, y );
                    x = seg->m_x;
                    y = seg->m_y;

                    //to reduce the points count for lines or a polygon on the screen
                    if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                    {
                        glVertex3f( x, y, m_layer / a2dNormDepth );
                        lastx = x;
                        lasty = y;
                        count++;
                    }
                }
            }
            iterprev = iter++;
        }
        glEnd(); 
    }
}










IMPLEMENT_CLASS( a2dOglDrawerBuf, a2dOglDrawer )

a2dOglDrawerBuf::a2dOglDrawerBuf( int width, int height, wxGLContext* glRC ): a2dOglDrawer( width, height, glRC )
{
}

bool a2dOglDrawerBuf::InitOglcalls()
{

    if ( !m_initGL )
    {

#ifdef _WIN32

    //((a2dOglCanvas*)m_display)->SetCurrent(*m_context);
    m_context->SetCurrent(*static_cast<const wxGLCanvas *>(m_display));

    // check if FBO is supported by your video card
    if( 1 )
    {
        // get pointers to GL functions
        glGenFramebuffers                     = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
        glDeleteFramebuffers                  = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
        glBindFramebuffer                     = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
        glCheckFramebufferStatus              = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
        glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
        glGenerateMipmap                      = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
        glFramebufferTexture2D                = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
        glFramebufferRenderbuffer             = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
        glGenRenderbuffers                    = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
        glDeleteRenderbuffers                 = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
        glBindRenderbuffer                    = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
        glRenderbufferStorage                 = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
        glGetRenderbufferParameteriv          = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetRenderbufferParameteriv");
        glIsRenderbuffer                      = (PFNGLISRENDERBUFFERPROC)wglGetProcAddress("glIsRenderbuffer");

        // check once again FBO extension
        if(glGenFramebuffers && glDeleteFramebuffers && glBindFramebuffer && glCheckFramebufferStatus &&
           glGetFramebufferAttachmentParameteriv && glGenerateMipmap && glFramebufferTexture2D && glFramebufferRenderbuffer &&
           glGenRenderbuffers && glDeleteRenderbuffers && glBindRenderbuffer && glRenderbufferStorage &&
           glGetRenderbufferParameteriv && glIsRenderbuffer)
        {
            fboSupported = fboUsed = true;
            std::cout << "Video card supports GL_ARB_framebuffer_object." << std::endl;
        }
        else
        {
            fboSupported = fboUsed = false;
            std::cout << "Video card does NOT support GL_ARB_framebuffer_object." << std::endl;
        }
    }

    // check if GL_EXT_framebuffer_blit is supported by your video card
    if( 1 )
    {
        // get pointers to GL functions
        glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)wglGetProcAddress("glBlitFramebuffer");
        if(glBlitFramebuffer)
        {
            fboBlitSupported = fboBlitUsed = true;
            std::cout << "Video card supports GL_EXT_framebuffer_blit." << std::endl;
        }
        else
        {
            fboBlitSupported = fboBlitUsed = false;
            std::cout << "Video card does NOT support GL_EXT_framebuffer_blit." << std::endl;
        }
    }

#else // for linux, do not need to get function pointers, it is up-to-date
    if( 1)//glInfo.isExtensionSupported("GL_ARB_framebuffer_object"))
    {
        fboSupported = fboUsed = true;
        std::cout << "Video card supports GL_ARB_framebuffer_object." << std::endl;
        
        // get pointers to GL functions
        glGenFramebuffers                     = (PFNGLGENFRAMEBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenFramebuffers");
        glDeleteFramebuffers                  = (PFNGLDELETEFRAMEBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glDeleteFramebuffers");
        glBindFramebuffer                     = (PFNGLBINDFRAMEBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindFramebuffer");
        glCheckFramebufferStatus              = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glXGetProcAddress((const GLubyte *)"glCheckFramebufferStatus");
        glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)glXGetProcAddress((const GLubyte *)"glGetFramebufferAttachmentParameteriv");
        glGenerateMipmap                      = (PFNGLGENERATEMIPMAPPROC)glXGetProcAddress((const GLubyte *)"glGenerateMipmap");
        glFramebufferTexture2D                = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glXGetProcAddress((const GLubyte *)"glFramebufferTexture2D");
        glFramebufferRenderbuffer             = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)glXGetProcAddress((const GLubyte *)"glFramebufferRenderbuffer");
        glGenRenderbuffers                    = (PFNGLGENRENDERBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenRenderbuffers");
        glDeleteRenderbuffers                 = (PFNGLDELETERENDERBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glDeleteRenderbuffers");
        glBindRenderbuffer                    = (PFNGLBINDRENDERBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindRenderbuffer");
        glRenderbufferStorage                 = (PFNGLRENDERBUFFERSTORAGEPROC)glXGetProcAddress((const GLubyte *)"glRenderbufferStorage");
        glGetRenderbufferParameteriv          = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)glXGetProcAddress((const GLubyte *)"glGetRenderbufferParameteriv");
        glIsRenderbuffer                      = (PFNGLISRENDERBUFFERPROC)glXGetProcAddress((const GLubyte *)"glIsRenderbuffer");

        // check once again FBO extension
        if(glGenFramebuffers && glDeleteFramebuffers && glBindFramebuffer && glCheckFramebufferStatus &&
           glGetFramebufferAttachmentParameteriv && glGenerateMipmap && glFramebufferTexture2D && glFramebufferRenderbuffer &&
           glGenRenderbuffers && glDeleteRenderbuffers && glBindRenderbuffer && glRenderbufferStorage &&
           glGetRenderbufferParameteriv && glIsRenderbuffer)
        {
            fboSupported = fboUsed = true;
            std::cout << "Video card supports GL_ARB_framebuffer_object." << std::endl;
        }
        else
        {
            fboSupported = fboUsed = false;
            std::cout << "Video card does NOT support GL_ARB_framebuffer_object." << std::endl;
        }
        
    }
    else
    {
        fboSupported = fboUsed = false;
        std::cout << "Video card does NOT support GL_ARB_framebuffer_object." << std::endl;
    }

    if(1)//glInfo.isExtensionSupported("GL_EXT_framebuffer_blit"))
    {
        fboBlitSupported = fboBlitUsed = true;
        std::cout << "Video card supports GL_EXT_framebuffer_blit." << std::endl;
        
        // get pointers to GL functions
        glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBlitFramebuffer");
        if(glBlitFramebuffer)
        {
            fboBlitSupported = fboBlitUsed = true;
            std::cout << "Video card supports GL_EXT_framebuffer_blit." << std::endl;
        }
        else
        {
            fboBlitSupported = fboBlitUsed = false;
            std::cout << "Video card does NOT support GL_EXT_framebuffer_blit." << std::endl;
        }
        
    }
    else
    {
        fboBlitSupported = fboBlitUsed = false;
        std::cout << "Video card does NOT support GL_EXT_framebuffer_blit." << std::endl;
    }


#endif

        glGenRenderbuffers(1, &m_textureId);
        glBindRenderbuffer(GL_RENDERBUFFER, m_textureId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);

        /*
        // create a texture object
        glGenTextures(1, &m_textureId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);  
        */

        // create a framebuffer object, you need to delete them when program exits.
        glGenFramebuffers(1, &m_fboId);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

        // create a renderbuffer object to store depth info
        // NOTE: A depth renderable image should be attached the FBO for depth test.
        // If we don't attach a depth renderable image to the FBO, then
        // the rendering output will be corrupted because of missing depth test.
        // If you also need stencil test for your rendering, then you must
        // attach additional image to the stencil attachement point, too.
        glGenRenderbuffers(1, &m_depthId);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthId);
        //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height );
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height );
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // attach a texture to FBO color attachement point
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_textureId);
        //glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);

        // attach a renderbuffer to depth attachment point
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthId);
        //glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthId);

            wxASSERT_MSG( m_context, "must have OpenGl context");

    /*
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);  
        glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height );
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // attach a texture to FBO color attachement point
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);

        // attach a renderbuffer to depth attachment point
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthId);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        GLenum status;
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
        switch(status)
        {
            case GL_FRAMEBUFFER_COMPLETE:
            cout<<"good";
        }

        glStencilMask(0x00);
        // draw where stencil's value is 0
        //glStencilFunc(GL_EQUAL, 0, 0xFF);
        /* (nothing to draw) */
        // draw only where stencil's value is 1
        glStencilFunc(GL_EQUAL, 1, 0xFF);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glClearColor((GLfloat)0.15, (GLfloat)0.15, 0.0, (GLfloat)1.0); // Dark, but not black.
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        m_initGL = true;
    }

    return true;
}

a2dOglDrawerBuf::a2dOglDrawerBuf( const a2dOglDrawerBuf& other )
    : a2dOglDrawer( other )
{
}

a2dOglDrawerBuf::a2dOglDrawerBuf( const a2dDrawer2D& other )
    : a2dOglDrawer( other )
{
}

a2dOglDrawerBuf::~a2dOglDrawerBuf()
{
   glDeleteRenderbuffers(1, &m_textureId );
   glDeleteRenderbuffers(1, &m_depthId );
}

void a2dOglDrawerBuf::SetBufferSize( int w, int h )
{
    m_width = w;
    m_height  = h;
 
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthId);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height );
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height );
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, m_textureId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, m_width, m_height );
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    // attach a texture to FBO color attachement point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_textureId);

    // attach a renderbuffer to depth attachment point
    //glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthId);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthId);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); 
    //glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void a2dOglDrawerBuf::BeginDraw()
{
    if ( !m_initGL )
    {
        InitOglcalls();
    }

    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
        //((a2dOglCanvas*)m_display)->SetCurrent(*m_context);
        m_context->SetCurrent(*static_cast<const wxGLCanvas *>(m_display));

        glBindFramebuffer(GL_FRAMEBUFFER, m_fboId );

        glLoadIdentity();
        //glClearColor( 0.3f, 0.4f, 0.6f, 1.0f );
        //glClear( GL_COLOR_BUFFER_BIT );
        glClear( GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT );

        SetDrawerFill( m_currentfill );
        SetDrawerStroke( m_currentstroke );
        DestroyClippingRegion();
        SetDrawStyle( m_drawstyle );
    }

    m_beginDraw_endDraw++;
}

void a2dOglDrawerBuf::EndDraw()
{
    m_beginDraw_endDraw--;

    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0 );
    }
}

void a2dOglDrawerBuf::BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos )
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

    if ( rect.x + rect.width > m_width )
    {
        rect.width = m_width - rect.x;
    }
    if ( rect.width <= 0 ) return;

    if ( rect.y + rect.height > m_height )
    {
        rect.height = m_height - rect.y;
    }
    if ( rect.height <= 0 ) return;
   
    //copy the main framebuffer to FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboId);  
    glBlitFramebuffer(0, 0,  m_width, m_height, 0, 0, m_width, m_height,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    GLenum error = glGetError();    

    //glReadBuffer(GL_BACK);
    //glDrawBuffer(GL_FRONT);
    // set up raster pos
    //glCopyPixels( 0, 0, GetWidth(), GetHeight(), GL_COLOR);

    //dc->Blit( rect.x - bufferpos.x, rect.y - bufferpos.y, rect.width, rect.height, &m_memdc, rect.x, rect.y, wxCOPY, false );
}

void a2dOglDrawerBuf::ShiftBuffer( int dxy, bool yshift )
{
    int bw = GetBuffer().GetWidth();
    int bh = GetBuffer().GetHeight();
    if ( yshift )
    {
        if ( dxy > 0 && dxy < bh )
        {
            GLuint t_textureId; // ID of texture
            GLuint t_depthId;       // ID of Renderbuffer object
            GLuint t_fboId;

            glGenRenderbuffers(1, &t_textureId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_textureId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);

            // create a framebuffer object, you need to delete them when program exits.
            glGenFramebuffers(1, &t_fboId);
            glBindFramebuffer(GL_FRAMEBUFFER, t_fboId);
            glGenRenderbuffers(1, &t_depthId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_depthId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height );
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // attach a texture to FBO color attachement point
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, t_textureId);
            // attach a renderbuffer to depth attachment point
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, t_depthId);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t_fboId);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboId);  
            glBlitFramebuffer( 0, 0, bw, bh - dxy, 0, dxy, bw, bh - dxy,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            GLenum error = glGetError();    

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboId);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, t_fboId);  
            glBlitFramebuffer( 0, 0,  m_width, m_height, 0, 0,  m_width, m_height,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            glDeleteRenderbuffers(1, &t_textureId );
            glDeleteRenderbuffers(1, &t_depthId );

        }
        else  if ( dxy < 0 && dxy > -bh )
        {
            GLuint t_textureId; // ID of texture
            GLuint t_depthId;       // ID of Renderbuffer object
            GLuint t_fboId;

            glGenRenderbuffers(1, &t_textureId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_textureId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);

            // create a framebuffer object, you need to delete them when program exits.
            glGenFramebuffers(1, &t_fboId);
            glBindFramebuffer(GL_FRAMEBUFFER, t_fboId);
            glGenRenderbuffers(1, &t_depthId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_depthId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height );
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // attach a texture to FBO color attachement point
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, t_textureId);
            // attach a renderbuffer to depth attachment point
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, t_depthId);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t_fboId);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboId);  
            glBlitFramebuffer( 0, -dxy, bw, bh + dxy, 0, 0, bw, bh + dxy,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            GLenum error = glGetError();    

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboId);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, t_fboId);  
            glBlitFramebuffer( 0, 0,  m_width, m_height, 0, 0,  m_width, m_height,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            glDeleteRenderbuffers(1, &t_textureId );
            glDeleteRenderbuffers(1, &t_depthId );
        }
        else
            wxFAIL_MSG( wxT( "you can only shift within height of buffer" ) );

    }
    else
    {
        if ( dxy > 0 && dxy < bw )
        {
            GLuint t_textureId; // ID of texture
            GLuint t_depthId;       // ID of Renderbuffer object
            GLuint t_fboId;

            glGenRenderbuffers(1, &t_textureId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_textureId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);

            // create a framebuffer object, you need to delete them when program exits.
            glGenFramebuffers(1, &t_fboId);
            glBindFramebuffer(GL_FRAMEBUFFER, t_fboId);
            glGenRenderbuffers(1, &t_depthId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_depthId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height );
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // attach a texture to FBO color attachement point
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, t_textureId);
            // attach a renderbuffer to depth attachment point
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, t_depthId);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t_fboId);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboId);  
            glBlitFramebuffer( 0, 0, bw - dxy, bh, dxy, 0, bw - dxy, bh,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            GLenum error = glGetError();    

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboId);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, t_fboId);  
            glBlitFramebuffer( 0, 0,  m_width, m_height, 0, 0,  m_width, m_height,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            glDeleteRenderbuffers(1, &t_textureId );
            glDeleteRenderbuffers(1, &t_depthId );
        }
        else if ( dxy < 0 && dxy > -bw )
        {
            GLuint t_textureId; // ID of texture
            GLuint t_depthId;       // ID of Renderbuffer object
            GLuint t_fboId;

            glGenRenderbuffers(1, &t_textureId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_textureId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);

            // create a framebuffer object, you need to delete them when program exits.
            glGenFramebuffers(1, &t_fboId);
            glBindFramebuffer(GL_FRAMEBUFFER, t_fboId);
            glGenRenderbuffers(1, &t_depthId);
            glBindRenderbuffer(GL_RENDERBUFFER, t_depthId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height );
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // attach a texture to FBO color attachement point
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, t_textureId);
            // attach a renderbuffer to depth attachment point
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, t_depthId);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t_fboId);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboId);  
            glBlitFramebuffer( -dxy, 0, bw + dxy, bh, 0, 0, bw + dxy, bh,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            GLenum error = glGetError();    

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboId);  
            glBindFramebuffer(GL_READ_FRAMEBUFFER, t_fboId);  
            glBlitFramebuffer( 0, 0,  m_width, m_height, 0, 0,  m_width, m_height,  GL_COLOR_BUFFER_BIT, GL_NEAREST); 

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);       
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            glDeleteRenderbuffers(1, &t_textureId );
            glDeleteRenderbuffers(1, &t_depthId );
        }
        else
            wxFAIL_MSG( wxT( "you can only shift within width of buffer" ) );
    }
}


#endif //wxART2D_USE_OPENGL
