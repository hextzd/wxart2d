/*! \file wx/artbase/mswfont.h
    \author Philip Patrick, Hans Dietrich

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: mswfont.h,v 1.3 2008/04/11 13:39:51 titato Exp $
*/

// XFont.h  Version 1.1
//
// Copyright (C) 2003 Hans Dietrich
//
// This software is released into the public domain.
// You are free to use it in any way you like.
//
// This software is provided "as is" with no expressed
// or implied warranty.  I accept no liability for any
// damage or loss of business that this software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XFONT_H
#define XFONT_H

//! MSW specific font properties
typedef struct _tagFONT_PROPERTIES
{
    TCHAR csName[1024];
    TCHAR csCopyright[1024];
    TCHAR csTrademark[1024];
    TCHAR csFamily[1024];
}
FONT_PROPERTIES, *LPFONT_PROPERTIES;


//! find a font that is close to a wxDC font.
BOOL GetFontFile( LPCTSTR lpszFontName,
                  LPTSTR lpszDisplayName,
                  int nDisplayNameSize,
                  LPTSTR lpszFontFile,
                  int nFontFileSize );

//! get font name from the file
BOOL GetFontProperties( LPCTSTR lpszFilePath,
                        LPFONT_PROPERTIES lpFontProps );

#endif //XFONT_H
