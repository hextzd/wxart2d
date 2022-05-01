/*! \file wx/canvas/mastertool.h
    \brief master tools are tools on top of other tools.

    Often it is easier to use a tool which only task is to push other tools onto the tool stack.
    That way a tool can be designed for one task, while the mastertool does define when which
    tools becomes active.

    \author Michael Sögtrop
    \date Created 02/06/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: mastertool.h,v 1.13 2009/04/23 19:35:23 titato Exp $
*/

// This file contains master tools. This are tools that customize GUI behaviour
// by intercepting events and then deligating work to other tools

#ifndef __A2DCANVASMASTERTOOL_H__
#define __A2DCANVASMASTERTOOL_H__


#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canobj.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/sttool.h"
#include "wx/canvas/edit.h"

//!Simple (handle-less) wire editing tool intended as sub-tool for master tools
/*!This tool supports the following actions:

- Dragging a wire edge

- Dragging a vertical/horizontal wire segment: move the segment

- Dragging a diagonal wire segment: insert a new point in the segment

- Dragging a vertex onto another vertex: delete vertex

\ingroup  tools
*/
class A2DCANVASDLLEXP a2dSimpleEditPolygonTool: public a2dStTool
{
public:
    enum Action
    {
        action_movevertex,
        action_movesegment,
        action_insertvertex
    };

public:
    a2dSimpleEditPolygonTool( a2dStToolContr* controller, a2dCanvasObject* hit, int index, int count, Action action );
    ~a2dSimpleEditPolygonTool();

protected:
    virtual bool EnterBusyMode();
    virtual void FinishBusyMode();
    virtual void AbortBusyMode();
    virtual void DoStopTool( bool abort );
    virtual bool ZoomSave() { return true;};
    virtual wxString GetCommandGroupName() { return _( "Edit polygon" ); }

    void OnChar( wxKeyEvent& event );
    void OnMouseEvent( wxMouseEvent& event );

    virtual bool CreateToolObjects();
    virtual void CleanupToolObjects();

public:
    DECLARE_CLASS( a2dSimpleEditPolygonTool )
    DECLARE_EVENT_TABLE()

protected:
    //! Index of vertex / segment, that is edited
    int m_index;

    //! Number of vertices
    int m_count;

    //! what to edit (vertex, segment, ...)
    Action m_action;

    //! this is used to simulate a handle attached to the object,
    //! by adding
    a2dSmrtPtr< a2dPolyHandleL > m_handle;

    //! preserve the a2dCANVAS_SELECT flag after editing.
    /*! selection is cleared in a2dCanvasObject::StartEdit */
    bool m_preserve_select;

    bool m_preserve_RouteWhenDrag;

    //! list of connected wire objects
    a2dCanvasObjectList m_connectedwirecopies;
};



//!Draw Master tool for object graphics
/*!This tool controls other tools such that simple objects graphics
can be edited without manually changing tools. It does this by pushing othere tools on the tool stack when the situation ask for.

- When dragging in an empty area, the select tool (with drag box) shall be used

- When dragging an existing object, the drag tool shall be used. If multiple object are selected, the drag-multi tool shall be used


\ingroup  tools
*/

//!Master tool for sub drawing child objects in a2dCanvasObject
/*!

\ingroup  tools
*/
class A2DCANVASDLLEXP a2dSubDrawMasterTool: public a2dStTool
{

public:

    a2dSubDrawMasterTool( a2dStToolContr* controller = NULL );

    ~a2dSubDrawMasterTool();

    void AppendTool( a2dBaseTool* tool, wxMenuItem* menuItem, wxChar key );

    void OnPostPushTool();

    void PushToolFromMouseMenu(  wxCommandEvent& event );

protected:

    virtual bool ZoomSave();
    virtual void DoStopTool( bool abort );

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    a2dSmrtPtr< a2dBaseTool > m_curTool;


public:
    DECLARE_DYNAMIC_CLASS( a2dSubDrawMasterTool )
    DECLARE_EVENT_TABLE()

protected:
};



//----------------------------------------------------------------------------
// a2dMasterDrawBase
//----------------------------------------------------------------------------

//! common stuff for several master tools
/*!
    - Pushing tools used form derived master tools.
    - Cursor hints
*/
class A2DCANVASDLLEXP a2dMasterDrawBase: public a2dStTool
{
public:

    //! constructor
    a2dMasterDrawBase( a2dStToolContr* controller );
    //! destructor
    ~a2dMasterDrawBase();

    //! if set true, Double Click on N selected objects, leads to editing them as a group
    void SetAllowMultiEdit( bool onOff ) { m_allowMultiEdit = onOff; }

    //! if true, Double Click on N selected objects, leads to editing them as a group
    bool GetAllowMultiEdit() { return m_allowMultiEdit; }

    //! if set true selecting object will be undoable
    void SetSelectUndo( bool onOff ) { m_select_undo = onOff; }

    //! if true selecting object is undoable
    bool GetSelectUndo() { return m_select_undo; }

    //! if set true pressing escape key, stops current active tool.
    /*! If escape key is handled by tool itself, that has priority.
    So if for instance a rectangle drawing tool is active, and busy drawing a rectangle, pressing escape key
    will and that drawing action, but the drawing tool stays active. If the drawing tool is not busy, the escape key will
    be handled here. And if this flag is set, the drawing tool will be stopped.
    If current first tool on the tool stack is this mastertool, there is no tool to stop.
    */
    void SetEscapeToStopFirst( bool onOff ) { m_escapeToStopFirst = onOff; }

    //! \see SetEscapeToStopFirst()
    bool GetEscapeToStopFirst() { return m_escapeToStopFirst; }

    //!set select fill
    void SetSelectFill( const a2dFill& fill ) { m_selectFill = fill; }

    //!get the select fill
    /*!
        Return the select fill
    */
    const a2dFill& GetSelectFill() const { return m_selectFill; }

    //!set select stroke
    void SetSelectStroke( const a2dStroke& stroke ) { m_selectStroke = stroke; }

    //!get the select stroke
    /*!
        Return the select stroke
    */
    const a2dStroke& GetSelectStroke() const { return m_selectStroke; }

	void SetWiringMode( a2dWiringMode wiringMode ) { m_wiringMode = wiringMode; }

    void MouseDump( wxMouseEvent& event, wxString strinfo = "" );

    bool RotateObject90LeftRight( bool right );

    //! set dlg at Double Left Down if dlgOrEdit == true
    void SetDlgOrEdit( bool dlgOrEdit ) { m_dlgOrEdit = dlgOrEdit; }

    //! set dlg at Double Left Down if dlgOrEdit == true
    bool GetDlgOrEdit() const { return m_dlgOrEdit; }

    //! set style dlg as modal
    void SetDlgOrEditModal( bool dlgOrEditModal ) { m_dlgOrEditModal = dlgOrEditModal; }

    //! get style dlg as modal setting
    bool GetDlgOrEditModal() const { return m_dlgOrEditModal; }

    void SetStyleDlgSimple( bool styleDlgSimple ) { m_styleDlgSimple = styleDlgSimple; }

	//! defines if an object will try to connect at the end of a drag
	void SetLateConnect( bool lateconnect ) { m_lateconnect = lateconnect; }

	void SetLastSelected( a2dCanvasObject* lastSelect, bool onOff );

    //! with this set to 1 or 2 the next point to add to the wire is a two segment piece.
    //! angle can be altered with the TAB key. Set to 0 is a straight line.
	void SetEndSegmentMode( a2dNextSeg mode );

	a2dNextSeg GetEndSegmentMode() { return m_endSegmentMode; }

    //! set if is allowed to draw Wires.
    void SetWireAllowDraw( bool allowWireDraw ) { m_allowWireDraw = allowWireDraw; }

    //! is it allowed to draw Wires.
    bool GetWireAllowDraw() const { return m_allowWireDraw; }

protected:

    a2dWiringMode m_wiringMode;

	virtual void PushZoomTool();
    virtual void PushSelectTool();
    virtual void PushDragTool( a2dCanvasObject* hit );
    virtual void PushDragMultiTool( a2dCanvasObject* hit, bool onlyKeys = false );
    virtual void PushCopyTool( a2dCanvasObject* hit );
    virtual void PushCopyMultiTool( a2dCanvasObject* hit );
    virtual void PushEditTool( a2dCanvasObject* hit );
    virtual void PushDlgEditTool( a2dCanvasObject* hit );
    virtual void StopDlgEditTool( a2dCanvasObject* hit ) {};
    virtual void PushMultiEditTool( a2dCanvasObject* hit );
    virtual void PushEditSegmentTool( a2dCanvasObject* hit, int segment );
    virtual void PushEditWireVertexTool( a2dCanvasObject* hit, int vertex );
    virtual void PushEditWireSegmentHorizontalTool( a2dCanvasObject* hit, int segment );
    virtual void PushEditWireSegmentVerticalTool( a2dCanvasObject* hit, int segment );
    virtual void PushEditWireSegmentInsertTool( a2dCanvasObject* hit, int segment );
    virtual void PushDrawWireTool( a2dCanvasObject* hit );
    virtual void PushMovePinTool( a2dCanvasObject* hit );
    virtual void PushRewirePinTool( a2dCanvasObject* hit );

    virtual void EditDlgOrHandles( a2dCanvasObject* hit, bool modifier, bool noHandleEditForWire );

    void SelectedStatus();

    a2dCanvasObject* GetTopLeftSelected();

protected:

    void SelectHitObject( a2dCanvasObject* hit );
    virtual bool ZoomSave();
    virtual void DoStopTool( bool abort );

    void OnChar( wxKeyEvent& event );

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    //! generate fack mouse events, based on current mouse event.
    //! used to set pushed tools in a certain mode e.g. generate LeftDown event when dragging already started in mastertool.
    void InitMouseEvent(wxMouseEvent& eventnew,
                                 int x, int y,
                                 wxMouseEvent& event );

    bool m_hadDoubleClick;

    bool m_dlgOrEdit;

    bool m_dlgOrEditModal;

	bool m_styleDlgSimple;

	bool m_lateconnect;

    bool m_escapeToStopFirst;

    bool m_allowWireDraw;

    int m_generatedPinX;
    int m_generatedPinY;

	a2dNextSeg m_endSegmentMode;

public:
    DECLARE_CLASS( a2dMasterDrawBase )
    DECLARE_EVENT_TABLE()

protected:

    // the editing mode derived from the object under the cursor
    enum Mode
    {
        mode_none,      // no mode
        mode_edit,      // edit object tool
        mode_multiedit, // edit all selected
        mode_zoom,      // zoom via drag box mode
        mode_select,    // select via drag box mode
        mode_cntrlselect,  // de first and select via drag box mode
        mode_deselect,  // de select via drag box mode
        mode_drag,      // drag object mode
        mode_dragmulti, // drag selected (multiple) objects mode
        mode_copy,      // drag-copy object mode
        mode_copymulti, // drag-copy selected (multiple) objects mode
        mode_zoomdrag,  // move view with mouse
        mode_drawwire,   // drawing a connection wire from a pin
        mode_editwire_segment,   // segment shift
        mode_editwire_segmenthorizontal, // wire editing mode: horizontal segment shift
        mode_editwire_segmentvertical,   // wire editing mode: horizontal segment shift
        mode_editwire_segmentinsert,     // wire editing mode: horizontal segment shift
        mode_editwire_vertex,            // wire editing mode: horizontal segment shift
        mode_move_pin,                   // move a pin
        mode_rewire_pin,                  // disconnect and move a pin and connect at new location if possible.
        mode_dlgedit
    } m_mode;

    // the object, that was responsible for setting m_mode to what it is
    a2dCanvasObject* m_modehit;

    // the hit information for m_modehit
    a2dHit m_modehitinfo;
    a2dHitEvent m_hitinfo;

    bool m_spaceDown;
    bool m_vertexSegmentEdit;
    bool m_movePin;

    bool m_toolBusy;
    bool m_modehitLastSelectState;

    bool m_allowMultiEdit;

    bool m_select_undo;

    a2dFill m_selectFill;
    a2dStroke m_selectStroke;
};


//!Master tool for objects graphics slecting and dragging nd zooming
/*!This tool controls other tools such that simple object graphics
can be edited without manually changing tools.

- Left Down + Drag => select objects in rectangle
- Left Down + Drag + Shift Down => select objects in rectangle add to already selected
- Left Down + Drag + Alt Down => move selected objects if allowed else editmode
- Left Down + Drag + Alt & Ctrl Down => copy selected objects if allowed else editmode
- Right Down => Drag canvas view
- Right Down + Up => Show Popup
- SpaceDown => Zoom area with mouse
- Double Click => Edit Selected of hit

\ingroup  tools
*/
class A2DCANVASDLLEXP a2dMasterDrawSelectFirst: public a2dMasterDrawBase
{
public:

    a2dMasterDrawSelectFirst( a2dStToolContr* controller );
    ~a2dMasterDrawSelectFirst();

protected:

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );

    void SelectHitObject( a2dCanvasObject* hit );
    void DecideMode( a2dCanvasObject* hit, const a2dHitEvent& hitinfo, bool shift, bool control );

public:
    DECLARE_CLASS( a2dMasterDrawSelectFirst )
    DECLARE_EVENT_TABLE()
};

//!Master tool for objects graphics slecting and dragging nd zooming
/*!This tool controls other tools such that simple object graphics
can be edited without manually changing tools.

- Left Down + Drag => zoom to rectangle
- Left Down + Drag + Shift Down => select objects in rectangle add to already selected
- Left Down + Drag + Cntrl Down => select objects in rectangle deselect already selected first
- Left Down + Drag + Alt Down => move selected objects if allowed else editmode
- Left Down + Drag + Alt & Ctrl Down => copy selected objects if allowed else editmode
- Right Down => Drag canvas view
- Right Down + Up => Show Popup
- Double Click => Edit Selected of hit

\ingroup  tools
*/
class A2DCANVASDLLEXP a2dMasterDrawZoomFirst: public a2dMasterDrawBase
{
public:

    a2dMasterDrawZoomFirst( a2dStToolContr* controller );
    ~a2dMasterDrawZoomFirst();

protected:

    void DoStopTool( bool abort );

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void Render();

public:

    void AbortBusyMode();

    DECLARE_CLASS( a2dMasterDrawZoomFirst )
    DECLARE_EVENT_TABLE()

};

//!Master tool for objects+wires graphics
/*!This tool controls other tools such that simple objects+wires graphics
can be edited without manually changing tools.

- When dragging in an empty area, the select tool (with drag box) shall be used

- When clicking on a pin, or an object that can create a dynamic pin at the mouse position the draw wire tool shall be used

- When dragging an existing object, the drag tool shall be used. If multiple object are selected, the drag-multi tool shall be used

- When dragging + Shift a wire edge, call a simple wire editing tool, that moves the edge (without first selecting it in edit mode and without showing handles)

- When dragging + Shift a vertical/horizontal wire segment, call a simple tool that moves the segment

- When dragging + Shift a diagonal wire segment, insert a new point in the segment

\ingroup  tools
*/
class A2DCANVASDLLEXP a2dGraphicsMasterTool: public a2dMasterDrawBase
{
public:

    a2dGraphicsMasterTool( a2dStToolContr* controller );
    ~a2dGraphicsMasterTool();

protected:

    void OnMouseEvent( wxMouseEvent& event );

    void SelectHitObject( a2dCanvasObject* hit );

public:
    DECLARE_CLASS( a2dGraphicsMasterTool )
    DECLARE_EVENT_TABLE()

protected:
};



#endif // __A2DCANVASMASTERTOOL_H__


