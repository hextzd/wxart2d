/*! \file wx/curves/zoomcur.h
    \brief classes for plotting curve and pie data, and editing them.

    Data stored in a derived a2dCanvasObject, can be plotted as a curve of pie chart.
    One can add markers on the curves, and several curves can be plot as a group in one plot.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: zoomcur.h,v 1.4 2006/12/13 21:43:27 titato Exp $
*/

#ifndef __WXZOOMCUR_H__
#define __WXZOOMCUR_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"
#include "wx/curves/meta.h"

//!Interactive Zooming into a a2dCanvasXYDisplayGroup.
/*! Use mouse to drag a rectangle to zoom into curves.

\remark  Left Click and/or drag.
\remark  Right Click for zoom popup.
\remark  Double Left Click for zoom to full bbox of all curves.

  \ingroup  tools
*/
class A2DCURVESDLLEXP a2dCurveZoomTool: public a2dStTool
{
public:

    a2dCurveZoomTool( a2dStToolContr* controller );
    ~a2dCurveZoomTool();
    bool ZoomSave() {return true;};

protected:

    virtual void GenerateAnotation();

    //! character handling
    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual wxString GetCommandGroupName() { return  _( "Curve zoom" ) ; }

public:

    a2dSmrtPtr<a2dCanvasXYDisplayGroupAreas> m_curves;

    DECLARE_EVENT_TABLE()

    DECLARE_CLASS( a2dCurveZoomTool )

    static const a2dPropertyIdCanvasObject PROPID_curves;
};

#endif

