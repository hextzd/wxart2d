/*! \file wx/editor/sttoolext.h
    \brief tool special for zooming image objects
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: sttoolext.h,v 1.9 2006/12/13 21:43:29 titato Exp $
*/

#ifndef __A2DCANVASSTTOOLEXT_H__
#define __A2DCANVASSTTOOLEXT_H__

#include "a2dprivate.h"

#if wxART2D_USE_CANEXTOBJ
#include "wx/canextobj/canextmod.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/sttool.h"

//!Interactive Zooming into a a2dRenderImage canvasobject.
/*!draging a rectangle to zoom into area.

\remark  Left Click and/or drag.
\remark  Right Click for zoom popup.
\remark  Double Left Click for zoom out.

\sa  a2dStToolContr
*/
class A2DEDITORDLLEXP a2dRenderImageZoomTool: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_RenderImageZoom;

    a2dRenderImageZoomTool( a2dStToolContr* controller );
    ~a2dRenderImageZoomTool();
    bool ZoomSave() {return true;};

protected:

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    //!mouse position used for zooming
    int       m_zoom_x1;
    //!mouse position used for zooming
    int       m_zoom_y1;
    //!mouse position used for zooming
    int       m_zoom_x2;
    //!mouse position used for zooming
    int       m_zoom_y2;

    a2dRenderImage* m_renderimage;

public:
    DECLARE_CLASS( a2dRenderImageZoomTool )
    DECLARE_EVENT_TABLE()

};


#endif //wxART2D_USE_CANEXTOBJ

#endif /* __A2DCANVASSTTOOL_H__ */


