/*! \file wx/gdsio/gdserr.h
    \brief Holds an error message
    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: gdserr.h,v 1.3 2006/12/15 21:00:03 titato Exp $
*/

#ifndef GDS_ERROR_H
#define GDS_ERROR_H

#include "wx/string.h"

#include "wx/canvas/candefs.h"

//! GDSII format input and output
/*!
    \ingroup fileio gdsfileio
*/
class A2DCANVASDLLEXP GDS_Error
{
public:
    GDS_Error( const wxString& message, const wxString& header = wxT( "" ) );
    GDS_Error( const GDS_Error& other );
    ~GDS_Error();

    wxString& GetErrorMessage() { return m_message; }
    wxString& GetHeaderMessage() { return m_header; }

protected:

    wxString m_message;
    wxString m_header;
};

#endif
