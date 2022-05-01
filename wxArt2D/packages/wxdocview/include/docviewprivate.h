/*! \file include/docviewprivate.h
    \author Francesco Montorsi

    Copyright: 2001-2005 (C) Francesco Montorsi

    Licence: wxWidgets Licence

    RCS-ID: $Id: docviewprivate.h,v 1.8 2009/09/01 21:13:56 titato Exp $
*/

#include "wx/wx.h"

// our setup file which contains all wxDOCVIEW_USE_XXXXX variables.
#include "docviewsetup.h"

// under *nix systems it should be better to place these "option coherency" tests
// into the configure script and not into this file which is evaluated at compile-time
// (instead of configure-time); however, we still do the checks here also for *nix systems
// to avoid a duplication of these checks...

// do some configuration checks; the checks are organized in the following way:
//
//  #if (MODULE_TO_CHECK == 1)
//       // check for presence of MODULE_TO_CHECK dependencies
//  #endif
//
// see the table at wxDocview/doc/dochtml/dependencies.html
#if (wxDOCVIEW_USE_DOCVIEW == 1)
#if (wxDOCVIEW_USE_XMLPARSE == 0)
#error Cannot build DOCVIEW module without XMLPARSE module
#endif
#endif
#if (wxDOCVIEW_USE_LUAWRAPS == 1)
#if (wxDOCVIEW_USE_XMLPARSE == 0)
#error Cannot build LUAWRAPS module without XMLPARSE module
#endif
#if (wxDOCVIEW_USE_DOCVIEW == 0)
#error Cannot build LUAWRAPS module without DOCVIEW module
#endif
#endif

// compiler-dependent warning shut down
#if defined(__VISUALC__)
// warning C4100: 'editmode' : unreferenced formal parameter
//#pragma warning (disable : 4100)      // not necessary anymore thanks to Kevin

#pragma warning (disable : 4786) // identifier was truncated to '255' characters in the debug information

#endif

