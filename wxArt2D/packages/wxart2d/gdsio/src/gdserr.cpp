/*! \file gdsio/src/gdserr.cpp
    \brief Holds an error message
    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: gdserr.cpp,v 1.8 2006/12/13 21:43:31 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/gdsio/gdserr.h"

GDS_Error::GDS_Error( const wxString& message, const wxString& header )
{
    m_message = message;
    m_header = header;
}

GDS_Error::GDS_Error( const GDS_Error& other )
{
    m_message = other.m_message;
    m_header = other.m_header;
}

GDS_Error::~GDS_Error()
{
}




