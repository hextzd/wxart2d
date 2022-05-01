/*! \file wx/editor/editor/mastertool.h
    \author Michael Sögtrop
    \date Created 02/06/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: mastertool.h,v 1.9 2007/06/18 19:53:53 titato Exp $
*/

// This file contains master tools. This are tools that customize GUI behaviour
// by intercepting events and then deligating work to other tools

#ifndef __MARS_MASTERTOOL_H__
#define __MARS_MASTERTOOL_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/sttool.h"
#include "wx/canvas/edit.h"

//!Master tool for objects+wires graphics
/*!This tool controls other tools such that simple objects+wires graphics
can be edited without manually changing tools.

- When dragging in an empty area, the select tool (with drag box) shall be used

- When clicking on a pin, or an object that can create a dynamic pin at the mouse position the draw wire tool shall be used

- When dragging an existing object, the drag tool shall be used. If multiple object are selected, the drag-multi tool shall be used

- When dragging a wire edge, call a simple wire editing tool, that moves the edge (without first selecting it in edit mode and without showing handles)

- When dragging a vertical/horizontal wire segment, call a simple tool that moves the segment

- When dragging a diagonal wire segment, insert a new point in the segment

\ingroup  tools
*/
class msMasterTool: public a2dStTool
{
public:

    msMasterTool( a2dStToolContr* controller );
    ~msMasterTool();

protected:
    virtual void PushSelectTool();
    virtual void PushDrawWireTool( a2dCanvasObject* hit );
    virtual void PushDragTool( a2dCanvasObject* hit );
    virtual void PushDragMultiTool( a2dCanvasObject* hit );
    virtual void PushCopyTool( a2dCanvasObject* hit );
    virtual void PushCopyMultiTool( a2dCanvasObject* hit );
    virtual void PushEditWireVertexTool( a2dCanvasObject* hit, int vertex );
    virtual void PushEditWireSegmentHorizontalTool( a2dCanvasObject* hit, int segment );
    virtual void PushEditWireSegmentVerticalTool( a2dCanvasObject* hit, int segment );
    virtual void PushEditWireSegmentInsertTool( a2dCanvasObject* hit, int segment );

    virtual wxCursor* GetHintCursorDrag();
    virtual wxCursor* GetHintCursorCopy();
    virtual wxCursor* GetHintCursorDrawWire();
    virtual wxCursor* GetHintCursorEditWireVertex();
    virtual wxCursor* GetHintCursorEditWireSegmentHorizontal();
    virtual wxCursor* GetHintCursorEditWireSegmentVertical();
    virtual wxCursor* GetHintCursorEditWireSegmentInsert();

protected:
    virtual bool ZoomSave();
    virtual void DoStopTool( bool abort );

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    void SelectHitObject( a2dCanvasObject* hit );

public:
    DECLARE_CLASS( msMasterTool )
    DECLARE_EVENT_TABLE()

protected:
    // the editing mode derived from the object under the cursor
    enum Mode
    {
        mode_none,      // no mode
        mode_select,    // select via drag box mode
        mode_drag,      // drag object mode
        mode_dragmulti, // drag selected (multiple) objects mode
        mode_copy,      // drag-copy object mode
        mode_copymulti, // drag-copy selected (multiple) objects mode
        mode_drawwire,  // draw wire mode
        mode_editwire_segmenthorizontal, // wire editing mode: horizontal segment shift
        mode_editwire_segmentvertical,   // wire editing mode: horizontal segment shift
        mode_editwire_segmentinsert,     // wire editing mode: horizontal segment shift
        mode_editwire_vertex,            // wire editing mode: horizontal segment shift
        mode_movelabel,
    } m_mode;
    // the object, that was responsible for setting m_mode to what it is
    a2dCanvasObject* m_modehit;
    // the hit information for m_modehit
    a2dHit m_modehitinfo;
};

class msCurveZoomTool: public a2dStTool
{
public:

    msCurveZoomTool( a2dStToolContr* controller );
    ~msCurveZoomTool();
    virtual bool ZoomSave() { return true; };
protected:
    void OnMouseEvent( wxMouseEvent& event );
public:
    a2dSmrtPtr<a2dCanvasObject> m_object;
    DECLARE_EVENT_TABLE()
    DECLARE_CLASS( msCurveZoomTool )
};





#endif // __MARS_MASTERTOOL_H__


