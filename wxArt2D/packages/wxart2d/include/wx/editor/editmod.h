/*! \file wx/editor/editmod.h
    \brief all headers in the editor module
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: editmod.h,v 1.13 2008/06/22 23:09:38 titato Exp $
*/

#ifndef __WXEDITT_H__
#define __WXEDITT_H__

#if wxART2D_USE_CANEXTOBJ
#include "wx/canextobj/canextmod.h"
#endif //wxART2D_USE_CANEXTOBJ

#include "wx/canvas/canmod.h"

#include "wx/editor/canedit.h"
#include "wx/editor/orderdlg.h"
#include "wx/editor/orderdlg.h"
#include "wx/editor/candocproc.h"
#include "wx/editor/strucdlgdoc.h"
#include "wx/editor/canpropedit.h"
#include "wx/editor/edsttool.h"

#ifdef A2DEDITORMAKINGDLL
#define A2DEDITORDLLEXP WXEXPORT
#define A2DEDITORDLLEXP_DATA(type) WXEXPORT type
#define A2DEDITORDLLEXP_CTORFN
#elif defined(WXART2D_USINGDLL)
#define A2DEDITORDLLEXP WXIMPORT
#define A2DEDITORDLLEXP_DATA(type) WXIMPORT type
#define A2DEDITORDLLEXP_CTORFN
#else // not making nor using DLL
#define A2DEDITORDLLEXP
#define A2DEDITORDLLEXP_DATA(type) type
#define A2DEDITORDLLEXP_CTORFN
#endif

#endif

