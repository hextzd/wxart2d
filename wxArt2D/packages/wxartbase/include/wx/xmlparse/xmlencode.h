/*! \file wx/xmlparse/xmlencode.h
    \brief functions for encoding characters in xml
    \author Michael Sögtrop

    Copyright: 2001-2004 (C) Michael Sögtrop

    Licence: wxWidgets licence

    RCS-ID: $Id: xmlencode.h,v 1.7 2008/03/31 19:18:16 titato Exp $
*/

#ifndef _WX_XMLENCODE_H_
#define _WX_XMLENCODE_H_


// Converts an Expat-produced UTF-8 encoded wxString in either an Unicode string
// or in non-Unicode environments into a wxConvLocal encoded string.

#if wxUSE_UNICODE
A2DGENERALDLLEXP wxString XmlDecodeStringToString(
    const char* s,
    size_t len = wxString::npos
);
#else
A2DGENERALDLLEXP wxString XmlDecodeStringToString(
    const char* s,
    size_t len = wxString::npos
);
#endif // wxUSE_UNICODE

#endif
