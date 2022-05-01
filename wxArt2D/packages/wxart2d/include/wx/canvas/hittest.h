/*! \file wx/canvas/hittest.h
    \brief general hittest functions
    \author Klaas Holwerda, Michael Sögtrop

    Copyright: 2001-2004 (C) Klaas Holwerda, Michael Sögtrop

    Licence: wxWidgets licence

    RCS-ID: $Id: hittest.h,v 1.4 2006/12/13 21:43:24 titato Exp $
*/

#ifndef _WX_HITTEST__
#define _WX_HITTEST__

#include "wx/geometry.h"

// !Hittest for rectangles
a2dHit HitTestRectangle( double xtest, double ytest, double xmin, double ymin, double xmax, double ymax, double margin );

#endif

