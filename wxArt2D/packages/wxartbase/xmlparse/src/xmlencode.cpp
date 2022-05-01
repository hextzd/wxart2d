/*! \file xmlparse/src/xmlencode.cpp
    \brief functions for encoding characters in xml
    \author Michael Sögtrop / Klaas Holwerda

    Copyright: 2001-2004 (C) Michael Sögtrop / Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: xmlencode.cpp,v 1.17 2009/07/15 21:23:43 titato Exp $
*/

#include "wxartbaseprec.h"
#include "wx/general/smrtptr.h"

// Note: I did put this into a new file, because i wanted to add handling for
// textual character entities like &auml;
// This will include lengthy tables.

// Encode all Non-ASCII and reserved (<>&") chars in &#d; format
// If in Non-unicode mode, this first converts the string to Unicode using wxConvLocal


// Converts an Expat-produced UTF-8 encoded wxString in either an Unicode string
// or in non-Unicode environments into a wxConvLocal encoded string.

#if wxUSE_UNICODE
A2DGENERALDLLEXP wxString XmlDecodeStringToString(
    const char* s,
    size_t len
)
{
    return wxString( s, wxConvUTF8, len );
}
#else
A2DGENERALDLLEXP wxString XmlDecodeStringToString(
    const char* s,
    size_t len
)
{
    const wxWCharBuffer wbuf(
        wxConvUTF8.cMB2WC( s, len == wxString::npos ? wxNO_LEN : len, NULL ) );

    return wxString( wbuf, wxConvLocal );
}
#endif

