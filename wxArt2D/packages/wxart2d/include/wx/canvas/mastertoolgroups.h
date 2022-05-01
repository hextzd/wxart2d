/*! \file wx/canvas/mastertoolgroups.h
    \brief master tool using tags to groups object

	This contains code related to the mastertool using tags to group objects.

    \author Klaas Holwerda
    \date Created 22/04/2012

    Copyright: 2012-2012 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: mastertoolgroups.h,v 1.13 2009/04/23 19:35:23 titato Exp $
*/


#ifndef __A2DCANVASMASTERTOOLGROUPS_H__
#define __A2DCANVASMASTERTOOLGROUPS_H__


#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canobj.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/sttool.h"
#include "wx/canvas/mastertool.h"
#include "wx/canvas/styledialg.h"

#define a2dDoubleClickWait 180

class A2DCANVASDLLEXP a2dStyleDialog;

//!Master tool for objects graphics slecting and dragging 
/*!This tool controls other tools such that simple object graphics
can be edited without manually changing tools.

- Left Down + Drag + Shift Down => select objects in rectangle add to already selected
- Left Down + Drag + Cntrl Down => select objects in rectangle deselect already selected first
- Left Down + Drag => move selected objects
- Left Down + Drag => move object and select it
- Left Down + Drag + Ctrl Down + Shift Down => copy selected objects 
- Right Down => Drag canvas view
- Right Down + Up => Show Popup

( wire mode )

- First LeftDown click on non wire object selects it.
- Shift + LeftDown to select a wire.
- Shift + LeftDown to select more.
- Second click on Selected object does NOT deselect
- Wire is draggable in segment and vertex and vertex insert when selected.
- Wire starts on pin of non wire object.
- Wire starts on wire and pin of wire object.
- LeftDown + drag on wire pin, makes it draggable.
- Dragging starting at non hit, or wire, starts a rectangle selection. 
- Double Click => Edit Selected of hit

( select mode )

- First LeftDown click on object selects it.
- Shift + LeftDown to select more.
- Second click on Selected object does de-select
- wire is draggable in segment and vertex and vertex insert, without selecting needed.
- Wire starts on pin of non wire object.
- Wire starts on wire and pin object when Ctrl is down.
- LeftDown + drag on wire pin, makes it draggable.
- Dragging starting at non hit, or wire, starts a rectangle selection. 
- Double Click => Edit Selected of hit

\ingroup  tools
*/
class A2DCANVASDLLEXP a2dMasterTagGroups : public a2dMasterDrawBase
{
public:

    a2dMasterTagGroups ( a2dStToolContr* controller );
    ~a2dMasterTagGroups ();

    //! set wire first mode on or off
    void SetWireMode( bool drawWire ) { m_drawWire = drawWire; }

    //! what is the current mode.
    bool GetWireMode() const { return m_drawWire; }

protected:

    void AbortBusyMode();

    void DoStopTool( bool abort );

    void OnChar( wxKeyEvent& event );

    bool SetWireCursor( a2dWirePolylineL* wire, const a2dHitEvent& hitinfo );
    void SetToolMode( bool ctrl, bool shift );

    void OnMouseEvent( wxMouseEvent& event );

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void Render();

protected:

    bool m_drawWire;

    DECLARE_CLASS( a2dMasterTagGroups  )
    DECLARE_EVENT_TABLE()

};

//!Master tool for objects graphics selecting and dragging 
/*!This tool controls other tools such that simple object graphics
can be edited without manually changing tools.

Main idea is that drawing wires is easy. Moving a wire pin, needs ctrl key.
Selecting a wire needs Shift key.

- Left Down + Drag + Shift Down => select objects in rectangle add to already selected
- Left Down + Drag + Cntrl Down => select objects in rectangle deselect already selected first
- Left Down + Drag => move selected objects
- Left Down + Drag => move object and select it
- Left Down + Drag + Ctrl Down + Shift Down => copy selected objects 
- Right Down => Drag canvas view
- Right Down + Up => Show Popup

- First LeftDown click on object selects it.
- Shift + LeftDown to select more objects
- Shift + LeftDown to select a wire
- Second click on Selected object does not deselect
- Selected wire is draggable in segment and vertex and vertex insert.
- Wire starts on pin of non wire object.
- Wire starts on wire and pin object
- Ctrl + LeftDown on wire pin, makes it draggable.
- Dragging starting at non hit, starts a rectangle selection. 
- Double Click => Edit Selected of hit

\ingroup  tools
*/
class A2DCANVASDLLEXP a2dMasterTagGroups2 : public a2dMasterDrawBase
{
public:

    a2dMasterTagGroups2 ( a2dStToolContr* controller );
    ~a2dMasterTagGroups2 ();

protected:

    void AbortBusyMode();

    void DoStopTool( bool abort );

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void Render();

public:
    DECLARE_CLASS( a2dMasterTagGroups2  )
    DECLARE_EVENT_TABLE()

};

//!Master tool for objects graphics slecting and dragging 
/*!This tool controls other tools such that simple object graphics
can be edited without manually changing tools.

- Left Down + Drag + Shift Down => select objects in rectangle add to already selected
- Left Down + Drag + Cntrl Down => select objects in rectangle deselect already selected first
- Left Down + Drag => move selected objects
- Left Down + Drag => move object and select it
- Left Down + Drag + Ctrl Down + Shift Down => copy selected objects 
- Right Down => Drag canvas view
- Right Down + Up => Show Popup

( wire mode )

- First LeftDown click on non wire object selects it.
- Shift + LeftDown to select a wire.
- Shift + LeftDown to select more.
- Second click on Selected object does NOT deselect
- Wire is draggable in segment and vertex and vertex insert when selected.
- Wire starts on pin of non wire object.
- Wire starts on wire and pin of wire object.
- LeftDown + drag on wire pin, makes it draggable.
- Dragging starting at non hit, or wire, starts a rectangle selection. 
- Double Click => Edit Selected of hit

( select mode )

- First LeftDown click on object selects it.
- Shift + LeftDown to select more.
- Second click on Selected object does de-select
- wire is draggable in segment and vertex and vertex insert, without selecting needed.
- Wire starts on pin of non wire object.
- Wire starts on wire and pin object when Ctrl is down.
- LeftDown + drag on wire pin, makes it draggable.
- Dragging starting at non hit, or wire, starts a rectangle selection. 
- Double Click => Edit Selected of hit

\ingroup  tools
*/
class A2DCANVASDLLEXP a2dMasterTagGroups3 : public a2dMasterDrawBase
{
public:

    a2dMasterTagGroups3 ( a2dStToolContr* controller );
    ~a2dMasterTagGroups3 ();

	virtual wxString GetToolString() const;

    virtual void SetActive( bool active = true );

    static void SetSelectMoreAtShift( bool selectMoreAtShift ) { m_selectMoreAtShift = selectMoreAtShift; }
   
    //! set wire first mode on or off
    void SetWireMode( bool drawWire ) { m_drawWire = drawWire; }

    //! what is the current mode.
    bool GetWireMode() const { return m_drawWire; }

    //! set if double click can be used to switch editingmode using shift key
    void SetAllowDoubleClickModifier( bool AllowDoubleClickModifier ) { m_AllowDoubleClickModifier = AllowDoubleClickModifier; }

    //! get if double click can be used to switch editingmode using shift key
    bool GetAllowDoubleClickModifier() const { return m_AllowDoubleClickModifier; }

    //! set generate pins on non wire objects on or off
    void SetGeneratePinsNonWire( bool generatePinsNonWire, bool generatePinsAlsoNonWireMode ) 
    { 
        m_generatePinsNonWire = generatePinsNonWire;
        m_generatePinsAlsoNonWireMode = generatePinsAlsoNonWireMode;
    }

    bool GetGeneratePinsNonWire() { return m_generatePinsNonWire; }
    bool GetGeneratePinsAlsoNonWireMode() { return m_generatePinsAlsoNonWireMode; }

protected:

    void AbortBusyMode();

    void DoStopTool( bool abort );

    void OnComEvent( a2dComEvent& event );

    void OnChar( wxKeyEvent& event );

    bool SetWireCursor( a2dWirePolylineL* wire, const a2dHitEvent& hitinfo, bool ctrl, bool shift, bool alt );
    virtual void SetToolMode( bool ctrl, bool shift, bool alt );

    void OnMouseEvent( wxMouseEvent& event );

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void Render();

    static bool m_selectMoreAtShift;

    bool m_drawWire;

    bool m_shiftWasDown;
    bool m_ctrlWasDown;
    bool m_altWasDown;

    bool m_AllowDoubleClickModifier;

    a2dCanvasObjectList* m_selectedObjects;

    bool m_generatePinsNonWire;
    bool m_generatePinsAlsoNonWireMode;

    DECLARE_CLASS( a2dMasterTagGroups3  )
    DECLARE_EVENT_TABLE()

};

class A2DCANVASDLLEXP a2dMasterSingleClick : public a2dMasterDrawBase
{
public:

    a2dMasterSingleClick ( a2dStToolContr* controller );
    ~a2dMasterSingleClick ();

	virtual wxString GetToolString() const;

    virtual void SetActive( bool active = true );

    //! set wire first mode on or off
    void SetWireMode( bool drawWire ) { m_drawWire = drawWire; }

    //! what is the current mode.
    bool GetWireMode() const { return m_drawWire; }

    //! set dlg at Double Left Down if dlgOrEdit == true
    void SetDlgOrEdit( bool dlgOrEdit ) { m_dlgOrEdit = dlgOrEdit; }

    //! set dlg at Double Left Down if dlgOrEdit == true
    bool GetDlgOrEdit() const { return m_dlgOrEdit; }

    //! set style dlg as modal
    void SetDlgOrEditModal( bool dlgOrEditModal ) { m_dlgOrEditModal = dlgOrEditModal; }

    //! get style dlg as modal setting
    bool GetDlgOrEditModal() const { return m_dlgOrEditModal; }
    
protected:

    void PushEditTool( a2dCanvasObject* hit );

    void AbortBusyMode();

    void DoStopTool( bool abort );

    void OnComEvent( a2dComEvent& event );

    void OnChar( wxKeyEvent& event );

    bool SetWireCursor( a2dWirePolylineL* wire, const a2dHitEvent& hitinfo, bool ctrl, bool shift, bool alt );
    void SetToolMode( bool ctrl, bool shift, bool alt );

    void OnMouseEvent( wxMouseEvent& event );

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void Render();

    void OnTimer(wxTimerEvent& event);

	wxTimer m_timer;
    bool m_drawWireIfNoDoubleClick;
	bool m_selectionIfNoDoubleClick;

    bool m_shiftWasDown;
    bool m_ctrlWasDown;
    bool m_altWasDown;

    bool m_drawWire;

    DECLARE_CLASS( a2dMasterSingleClick  )
    DECLARE_EVENT_TABLE()

};

//!Master tool for objects graphics slecting and dragging 
/*!This tool controls other tools such that simple object graphics
can be edited without manually changing tools.

- Left Down + Drag + Shift Down => select objects in rectangle add to already selected
- Left Down + Drag + Cntrl Down => select objects in rectangle deselect already selected first
- Left Down + Drag => move selected objects
- Left Down + Drag => move object and select it
- Left Down + Drag + Ctrl Down + Shift Down => copy selected objects 
- Right Down => Drag canvas view
- Right Down + Up => Show Popup

( wire mode )

- First LeftDown click on non wire object selects it.
- Shift + LeftDown to select a wire.
- Shift + LeftDown to select more.
- Second click on Selected object does NOT deselect
- Wire is draggable in segment and vertex and vertex insert when selected.
- Wire starts on pin of non wire object.
- Wire starts on wire and pin of wire object.
- LeftDown + drag on wire pin, makes it draggable.
- Dragging starting at non hit, or wire, starts a rectangle selection. 
- Double Click => Edit Selected of hit

( select mode )

- First LeftDown click on object selects it.
- Shift + LeftDown to select more.
- Second click on Selected object does de-select
- wire is draggable in segment and vertex and vertex insert, without selecting needed.
- Wire starts on pin of non wire object.
- Wire starts on wire and pin object when Ctrl is down.
- LeftDown + drag on wire pin, makes it draggable.
- Dragging starting at non hit, or wire, starts a rectangle selection. 
- Double Click => Edit Selected of hit

\ingroup  tools
*/
class A2DCANVASDLLEXP a2dMasterTagGroups4 : public a2dMasterDrawBase
{
public:

    a2dMasterTagGroups4 ( a2dStToolContr* controller );
    ~a2dMasterTagGroups4 ();

	virtual wxString GetToolString() const;

    virtual void SetActive( bool active = true );

    static void SetSelectMoreAtShift( bool selectMoreAtShift ) { m_selectMoreAtShift = selectMoreAtShift; }

    //! set wire first mode on or off
    void SetWireMode( bool drawWire ) { m_drawWire = drawWire; }

    //! what is the current mode.
    bool GetWireMode() const { return m_drawWire; }

    //! set dlg at Double Left Down if dlgOrEdit == true
    void SetDlgOrEdit( bool dlgOrEdit ) { m_dlgOrEdit = dlgOrEdit; }

    //! set dlg at Double Left Down if dlgOrEdit == true
    bool GetDlgOrEdit() const { return m_dlgOrEdit; }

    //! set style dlg as modal
    void SetDlgOrEditModal( bool dlgOrEditModal ) { m_dlgOrEditModal = dlgOrEditModal; }

    //! get style dlg as modal setting
    bool GetDlgOrEditModal() const { return m_dlgOrEditModal; }


    //! set if editing should start at a slow/delayed LeftUp event. 
    void SetEditIfDelayed( bool startEditIfDelayed ) { m_startEditIfDelayed = startEditIfDelayed; }

    //! get if editing should start at a slow/delayed LeftUp event. 
    bool GetEditIfDelayed() const { return m_startEditIfDelayed; }

protected:

    void EditDlgOrHandles( a2dCanvasObject* hit, bool modifier, bool noHandleEditForWire );

    void AbortBusyMode();

    void DoStopTool( bool abort );

    void OnComEvent( a2dComEvent& event );

    void OnChar( wxKeyEvent& event );

    void OnTimer(wxTimerEvent& event);

    bool SetWireCursor( a2dWirePolylineL* wire, const a2dHitEvent& hitinfo, bool ctrl, bool shift, bool alt );
    void SetToolMode( bool ctrl, bool shift, bool alt );

    void OnMouseEvent( wxMouseEvent& event );

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void Render();

    static bool m_selectMoreAtShift;

    bool m_drawWire;

	wxTimer m_timer;

	bool m_selectionIfNoDoubleClick;
    bool m_drawWireIfNoDoubleClick;

	//detection for delayed Left Up
	bool m_editIfDelayed;

	//if set, start a delayed LeftUp event will start editing.
	bool m_startEditIfDelayed;

    DECLARE_CLASS( a2dMasterTagGroups4  )
    DECLARE_EVENT_TABLE()

};


class A2DCANVASDLLEXP a2dMasterTagGroups33 : public a2dMasterDrawBase
{
public:

    a2dMasterTagGroups33 ( a2dStToolContr* controller );
    ~a2dMasterTagGroups33 ();

	virtual wxString GetToolString() const;

    virtual void SetActive( bool active = true );

    static void SetSelectMoreAtShift( bool selectMoreAtShift ) { m_selectMoreAtShift = selectMoreAtShift; }

    //! set wire first mode on or off
    void SetWireMode( bool drawWire ) { m_drawWire = drawWire; }

    //! what is the current mode.
    bool GetWireMode() const { return m_drawWire; }

    //! set if double click can be used to switch editingmode using shift key
    void SetAllowDoubleClickModifier( bool AllowDoubleClickModifier ) { m_AllowDoubleClickModifier = AllowDoubleClickModifier; }

    //! get if double click can be used to switch editingmode using shift key
    bool GetAllowDoubleClickModifier() const { return m_AllowDoubleClickModifier; }

    //! set Allow Delayed Editing
    void SetAllowDelayedEdit( bool AllowDelayedEdit ) { m_AllowDelayedEdit = AllowDelayedEdit; }

    //! get Allow Delayed Editing
    bool GetAllowDelayedEdit() const { return m_AllowDelayedEdit; }

    //! set if editing should start at a slow/delayed LeftUp event. 
    void SetEditIfDelayed( bool startEditIfDelayed ) { m_startEditIfDelayed = startEditIfDelayed; }

    //! get if editing should start at a slow/delayed LeftUp event. 
    bool GetEditIfDelayed() const { return m_startEditIfDelayed; }

protected:

    void AbortBusyMode();

    void DoStopTool( bool abort );

    void OnComEvent( a2dComEvent& event );

    void OnChar( wxKeyEvent& event );

    void OnTimer(wxTimerEvent& event);

    bool SetWireCursor( a2dWirePolylineL* wire, const a2dHitEvent& hitinfo, bool ctrl, bool shift, bool alt );
    void SetToolMode( bool ctrl, bool shift, bool alt );

    void OnMouseEvent( wxMouseEvent& event );

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void Render();

    static bool m_selectMoreAtShift;

    bool m_drawWire;

	wxTimer m_timer;

	bool m_selectionIfNoDoubleClick;
    bool m_drawWireIfNoDoubleClick;

    bool m_shiftWasDown;
    bool m_ctrlWasDown;
    bool m_altWasDown;

	//detection for delayed Left Up
	bool m_editIfDelayed;

	//if set, start a delayed LeftUp event will start editing.
	bool m_startEditIfDelayed;

    bool m_AllowDoubleClickModifier;
    bool m_AllowDelayedEdit;

    DECLARE_CLASS( a2dMasterTagGroups33  )
    DECLARE_EVENT_TABLE()

};

#endif // __A2DCANVASMASTERTOOL_H__


