/*! \file genart/src/imagergba.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: imagergba.cpp,v 1.2 2009/09/02 20:01:23 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include <wx/wfstream.h>
#include "wx/genart/imagergba.h"
#endif

//----------------------------------------------------------------------------
// a2dImageRGBA
//----------------------------------------------------------------------------

#if defined(__WXPALMOS__)
#elif defined(__WXMSW__)
#include "wx/msw/dib.h"
#elif defined(__WXMOTIF__)
#elif defined(__WXGTK20__)
#include <gtk/gtk.h>
#elif defined(__WXGTK__)
#include <gtk/gtk.h>
#elif defined(__WXX11__)
#elif defined(__WXMGL__)
#elif defined(__WXDFB__)
#elif defined(__WXMAC__)
#elif defined(__WXCOCOA__)
#elif defined(__WXPM__)
#endif

a2dImageRGBA::a2dImageRGBA( int width, int height )
{
    m_height = height;
    m_width = width;
    m_glimage = ( unsigned char* ) malloc( width * height * 4 );
    memset( m_glimage, 0, width * height * 4 );
#if defined(__WXMSW__)
    m_rgbOrder = false;
#else
    m_rgbOrder = true;
#endif

}

a2dImageRGBA::a2dImageRGBA( const wxImage& image, unsigned char alpha )
{
/*
#if defined(__WXMSW__)
    m_rgbOrder = false;
#else
    m_rgbOrder = true;
#endif
*/
    m_rgbOrder = true;

    m_height = image.GetHeight();
    m_width = image.GetWidth();
    m_glimage = ( unsigned char* ) malloc( image.GetWidth() * image.GetHeight() * 4 );
    int i, j;

    unsigned char* data =  ( unsigned char* ) m_glimage;
    for ( i = m_height - 1; i >= 0 ; i-- )
        //for ( i=0; i < m_height ; i++ )
    {
        for ( j = 0; j < m_width ; j++ )
        {
            if ( m_rgbOrder )
            {
                *data++ = image.GetRed( j, i );
                *data++ = image.GetGreen( j, i );
                *data++ = image.GetBlue( j, i );
                if ( image.HasAlpha() )
                    * data++ = ( image.GetAlpha( j, i ) * alpha ) / 255;
                else if ( image.HasMask() )
                {
                    if ( image.IsTransparent( j, i ) )
                        * data++ = ( unsigned char ) 0;
                    else
                        * data++ = alpha;
                }
                else
                    *data++ = ( unsigned char ) alpha;
            }
            else
            {
                *data++ = image.GetBlue( j, i );
                *data++ = image.GetGreen( j, i );
                *data++ = image.GetRed( j, i );
                if ( image.HasAlpha() )
                    *data++ = ( image.GetAlpha( j, i ) * alpha ) / 255;
                else if ( image.HasMask() )
                {
                    if ( image.IsTransparent( j, i ) )
                        * data++ = ( unsigned char ) 0;
                    else
                        * data++ = alpha;
                }
                else
                    *data++ = ( unsigned char ) alpha;
            }

        }
    }
}

a2dImageRGBA::~a2dImageRGBA()
{
    m_height = 0;
    m_width = 0;
    free( m_glimage );
    m_glimage = NULL;

}

a2dImageRGBA& a2dImageRGBA::operator=( const a2dImageRGBA& other )
{
    free( m_glimage );
    m_glimage = ( unsigned char* )malloc( other.m_width * other.m_height * 4 );
    memcpy( m_glimage, other.m_glimage, other.m_width * other.m_height * 4 );
    m_height = other.m_height;
    m_width = other.m_width;
    m_rgbOrder = other.m_rgbOrder;
    return *this;
}

void a2dImageRGBA::SetAlpha( unsigned char alpha )
{
    int j;
    unsigned char* data;
    data =  ( unsigned char* ) m_glimage;
    data = data + 3;
    for ( j = 0; j < m_height * m_width; j++ )
    {
        *data = alpha;
        data = data + 4;
    }
}

void a2dImageRGBA::SetAlpha( unsigned char* alphadata )
{
    int j;
    unsigned char* data;
    data =  ( unsigned char* ) m_glimage;
    data = data + 3;
    for ( j = 0; j < m_height * m_width; j++ )
    {
        *data = *alphadata;
        data = data + 4;
        alphadata++;
    }
}

wxImage a2dImageRGBA::GetImage() const
{
    wxImage image = wxImage( m_width, m_height );
    image.SetAlpha();
    unsigned char* imageData = image.GetData();
    int i, j;

    unsigned char* data =  ( unsigned char* ) m_glimage;
    for ( i = m_height - 1; i >= 0 ; i-- )
        //for ( i=0; i < m_height ; i++ )
    {
        for ( j = 0; j < m_width ; j++ )
        {
            if ( m_rgbOrder )
            {
                *imageData++ = *data++;
                *imageData++ = *data++;
                *imageData++ = *data++;
                image.SetAlpha( j, i, *data++ );
            }
            else
            {
                *imageData++ = data[2];
                *imageData++ = data[1];
                *imageData++ = data[0];
                image.SetAlpha( j, i, data[3] );
                data += 4;
            }
        }
    }
    return image;
}

a2dImageRGBA* a2dImageRGBA::GetSubImage( const wxRect& rect ) const
{
    const int subwidth = rect.GetWidth();
    const int subheight = rect.GetHeight();

    a2dImageRGBA* image = new a2dImageRGBA( subwidth, subheight );

    wxCHECK_MSG( ( rect.GetLeft() >= 0 ) && ( rect.GetTop() >= 0 ) &&
                 ( rect.GetRight() <= GetWidth() ) && ( rect.GetBottom() <= GetHeight() ),
                 image, wxT( "invalid subimage size" ) );

    const unsigned char* src_data = GetData();
    unsigned char* subdata = image->GetData();

    wxCHECK_MSG( subdata, image,  wxT( "unable to create image" ) );

    const int width = GetWidth();
    const int pixsoff = rect.GetLeft() + width * rect.GetTop();

    src_data += 4 * pixsoff;

    for ( long j = 0; j < subheight; ++j )
    {
        memcpy( subdata, src_data, 4 * subwidth );
        subdata += 4 * subwidth;
        src_data += 4 * width;
    }

    return image;
}

wxBitmap a2dImageRGBA::CreateBitmap()
{
#if defined(__WXMSW__)

    bool m_hasAlpha = true;//false;
    wxImage image = GetImage();
    const int bpp = m_hasAlpha ? 32 : 24;
    return wxBitmap( image, bpp );
/*
    // if we have alpha channel, we need to create a 32bpp RGBA DIB, otherwise
    // a 24bpp RGB is sufficient
    bool m_hasAlpha = true;//false;
    const int bpp = m_hasAlpha ? 32 : 24;

    wxDIB dib( m_width, m_height, bpp );

    // DIBs are stored in bottom to top order (see also the comment above in
    // Create()) so we need to copy bits line by line and starting from the end
    const int srcBytesPerLine = m_width * 4;
    const int dstBytesPerLine = dib.GetLineSize( m_width, bpp );
    const unsigned char* src = GetData() + ( ( m_height - 1 ) * srcBytesPerLine );
    unsigned char* dstLineStart = dib.GetData();
    for ( int y = 0; y < m_height; y++ )
    {
        // copy one DIB line
        unsigned char* dst = dstLineStart;
        if ( !m_rgbOrder )
        {
            if ( m_hasAlpha )
            {
                for ( int x = 0; x < m_width; x++ )
                {
                    // RGB order is reversed, and we need to premultiply
                    // all channels by alpha value for use with ::AlphaBlend.
                    const unsigned char a = src[3];
                    *dst++ = ( unsigned char )( ( src[0] * a + 127 ) / 255 );
                    *dst++ = ( unsigned char )( ( src[1] * a + 127 ) / 255 );
                    *dst++ = ( unsigned char )( ( src[2] * a + 127 ) / 255 );
                    *dst++ = ( unsigned char )( src[3] );
                    src += 4;
                }
            }
            else // no alpha channel
            {
                for ( int x = 0; x < m_width; x++ )
                {
                    // RGB order is reversed.
                    *dst++ = src[0];
                    *dst++ = src[1];
                    *dst++ = src[2];
                    src += 4;
                }
            }
        }
        else
        {
            if ( m_hasAlpha )
            {
                for ( int x = 0; x < m_width; x++ )
                {
                    // RGB order is reversed, and we need to premultiply
                    // all channels by alpha value for use with ::AlphaBlend.
                    const unsigned char a = src[3];
                    *dst++ = ( unsigned char )( ( src[2] * a + 127 ) / 255 );
                    *dst++ = ( unsigned char )( ( src[1] * a + 127 ) / 255 );
                    *dst++ = ( unsigned char )( ( src[0] * a + 127 ) / 255 );
                    *dst++ = ( unsigned char )( src[3] );
                    src += 4;
                }
            }
            else // no alpha channel
            {
                for ( int x = 0; x < m_width; x++ )
                {
                    // RGB order is reversed.
                    *dst++ = src[2];
                    *dst++ = src[1];
                    *dst++ = src[0];
                    src += 4;
                }
            }
        }

        // pass to the previous line in the image
        src -= 2 * srcBytesPerLine;

        // and to the next one in the DIB
        dstLineStart += dstBytesPerLine;
    }

    wxBitmap bitm;
    bitm.CopyFromDIB( dib );
    bool alpha = bitm.HasAlpha();
    return bitm;
*/
#else

    wxBitmap bitm( m_width, m_height, 32 );

    GdkPixbuf* pixbuf = bitm.GetPixbuf();
    if ( !pixbuf )
        return bitm;

    // Copy the data:
    const unsigned char* src = GetData();
    unsigned char* dst = gdk_pixbuf_get_pixels( pixbuf );

    int rowpad = gdk_pixbuf_get_rowstride( pixbuf ) - 4 * m_width;

    if ( m_rgbOrder )
    {
        memcpy( dst, src, m_width * m_height * 4 );
    }
    else
    {
        for ( int y = 0; y < m_height; y++ )
        {
            for ( int x = 0; x < m_width; x++ )
            {
                // RGB order is reversed, and we need to premultiply
                // all channels by alpha value for use with ::AlphaBlend.
                const unsigned char a = src[3];
                *dst++ = ( unsigned char )( ( src[0] * a + 127 ) / 255 );
                *dst++ = ( unsigned char )( ( src[1] * a + 127 ) / 255 );
                *dst++ = ( unsigned char )( ( src[2] * a + 127 ) / 255 );
                *dst++ = ( unsigned char )( src[3] );
                src += 4;
            }
        }
    }

    return bitm;
#endif

}

