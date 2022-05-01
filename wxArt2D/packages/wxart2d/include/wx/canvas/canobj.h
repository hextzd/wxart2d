/*! \file wx/canvas/canobj.h
    \brief The base class for all drawable objects in a a2dCanvasDocument.

    Next to a2dCanvasObject, there are classes for filtering object ( a2dCanvasObjectFilter )
    when iterating recursively through a document hierarchy.
    The class a2dIterC is used for holding the iteration context,
    which on its turn holds the drawing context when needed.

    \author Klaas Holwerda &  Robert Roebling

    Copyright: 2000-2004 (c) Robert Roebling

    Licence: wxWidgets Licence

    RCS-ID: $Id: canobj.h,v 1.85 2009/07/10 19:23:13 titato Exp $
*/

#ifndef __WXCANOBJ_H__
#define __WXCANOBJ_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"

#include "wx/general/genmod.h"

#include "wx/geometry.h"
#include "wx/artbase/afmatrix.h"
#include "wx/artbase/bbox.h"
#include "wx/canvas/candefs.h"

typedef a2dSmrtPtr<a2dCanvasObject> a2dCanvasObjectPtr;

#include "wx/canvas/styleprop.h"
#include "wx/canvas/xmlpars.h"
#include "wx/canvas/restrict.h"
#include "wx/canvas/hittest.h"
#include "wx/canvas/connectgen.h"

#include "wx/canvas/route.h"

#include <bitset>
using namespace std;

class a2dWalkerIOHandlerWithContext;

#include "wx/canvas/canobj1.h"
#include "wx/canvas/canobj2.h"



#endif    // WXCANOBJ


