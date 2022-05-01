/*! \file wx/curves/meta.h
    \brief classes for plotting curve and pie data, and editing them.

    Data stored in a derived a2dCanvasObject, can be plotted as a curve of pie chart.
    One can add markers on the curves, and several curves can be plot as a group in one plot.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: meta.h,v 1.31 2008/09/05 19:01:12 titato Exp $
*/

#ifndef __WXMETA_H__
#define __WXMETA_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canglob.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/polygon.h"
#include "wx/canvas/eval.h"

#if wxART2D_USE_EDITOR
#include "wx/canvas/edit.h"
#endif //wxART2D_USE_EDITOR

#ifdef A2DCURVESMAKINGDLL
#define A2DCURVESDLLEXP WXEXPORT
#define A2DCURVESDLLEXP_DATA(type) WXEXPORT type
#define A2DCURVESDLLEXP_CTORFN
#elif defined(WXART2D_USINGDLL)
#define A2DCURVESDLLEXP WXIMPORT
#define A2DCURVESDLLEXP_DATA(type) WXIMPORT type
#define A2DCURVESDLLEXP_CTORFN
#else // not making nor using DLL
#define A2DCURVESDLLEXP
#define A2DCURVESDLLEXP_DATA(type) type
#define A2DCURVESDLLEXP_CTORFN
#endif

class A2DCURVESDLLEXP a2dCurveAxis;
class A2DCURVESDLLEXP a2dBaseMarker;
class A2DCURVESDLLEXP a2dMarker;
class A2DCURVESDLLEXP a2dMarkerShow;
class A2DCURVESDLLEXP a2dCurvesArea;
class A2DCURVESDLLEXP a2dCanvasXYDisplayGroupAreas;
class A2DCURVESDLLEXP a2dCursor;

#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dMarkerShow>;
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dCursor>;
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dCanvasXYDisplayGroupAreas>;

#endif

#include "wx/curves/curve.h"
#include "wx/curves/axis.h"
#include "wx/curves/curvegroup.h"
#include "wx/curves/marker.h"
#include "wx/curves/zoomcur.h"

#endif

