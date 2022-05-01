/*! \file wx/genart/imagergba.h
    \brief Contains image with RGBA per pixel

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: imagergba.h,v 1.2 2009/09/02 20:01:23 titato Exp $
*/

#ifndef __WXIMAGERGBA_H__
#define __WXIMAGERGBA_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "wx/image.h"
#include "wx/prntbase.h"

class a2dImageRGBA
{
public:
    a2dImageRGBA( int width = 0, int height = 0 );

    a2dImageRGBA( const wxImage& image, unsigned char alpha = 255 );

    ~a2dImageRGBA();

    a2dImageRGBA& operator=( const a2dImageRGBA& other );

    wxImage GetImage() const;

    a2dImageRGBA* GetSubImage( const wxRect& rect ) const;

    wxBitmap CreateBitmap();

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    void SetAlpha( unsigned char alpha );

    void SetAlpha( unsigned char* alphadata );

    unsigned char* GetData() const { return m_glimage; }

    bool GetOrder() { return m_rgbOrder; }

    void SetOrder( bool orderRgba ) { m_rgbOrder = orderRgba; }

private:

    unsigned char* m_glimage;

    int m_height;
    int m_width;

    bool m_rgbOrder;
};

#endif /* __WXIMAGERGBA_H__ */

