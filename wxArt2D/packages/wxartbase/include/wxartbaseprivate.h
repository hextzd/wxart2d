/*! \file include/wxartbaseprivate.h
    \author Francesco Montorsi

    Copyright: 2001-2005 (C) Francesco Montorsi

    Licence: wxWidgets Licence

    RCS-ID: $Id: wxartbaseprivate.h,v 1.8 2009/09/01 21:13:56 titato Exp $
*/

#include "wx/wx.h"

// our setup file which contains all wxART2D_USE_XXXXX variables.
#include "wxartbasesetup.h"

// compiler-dependent warning shut down
#if defined(__VISUALC__)
// warning C4100: 'editmode' : unreferenced formal parameter
//#pragma warning (disable : 4100)      // not necessary anymore thanks to Kevin

#pragma warning (disable : 4786) // identifier was truncated to '255' characters in the debug information

#endif

