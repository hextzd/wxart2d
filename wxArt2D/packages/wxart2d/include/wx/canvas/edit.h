/*! \file wx/canvas/edit.h
    \brief editing tool for a2dCanvasObject's
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: edit.h,v 1.23 2009/06/03 17:38:13 titato Exp $
*/

#ifndef __WXCANEDITOBJ_H__
#define __WXCANEDITOBJ_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canobj.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/sttool.h"

//----------------------------------------------------------------------------
// decls
//----------------------------------------------------------------------------

class a2dCanvas;
class a2dCanvasDocument;
class a2dLayers;

//!This tool is for editing a single object.
/*!
    The object its EditStart member is called, this generates a Clone of the object
    to be edited. And event processing is redirected to the Cloned object to edit it.

    The cloned edit copy, issues commands to the command processor of the document, and this changes
    the original a2dCanvasObject that is being edited.
    This then changes to make it in sync with editing clone.
    Undo is possible because of this, but it is possible to not use commands and choose to directly
    modify the original object.

    After editing is done a double click will call the EditEnd member of the a2dCanvasObject.
    In the end the Cloned editing object is Released.

    If SetSingleClickToEnd() is set, a Left Down mouse event outside the object being edited,
    also stops editing of the current object, and if subediting it even ends this tool.
    This makes it easy to go to sublevel editing and back with just one click.

    The object itself is in control of all editing facilities of that specific object.
    This might mean that other tools will be pushed on the tool stack, for editing of subobjects.

    The EditStart member initializes the a2dCanvasObject for editing, but only if editing is allowed by it.

    The editing Clone will have as children editing handles, which can be dragged in order to modify the object.
    Those handles depend on the object its editing implementation.
    Rendering of the Cloned editing object can be changed on the basis of the m_editingCopy flag if needed.

    The default editing functionality of a2dCanvasObject, is just editing its local matrix.

    The TAB charecter should be used to switch between this mode and the object specific editing mode.

    Adding editing functionality to an object is implemented by intercepting events using a static event table.
    When the m_editingCopy is set, we know it is the cloned edit copy we are dealing with.
    The events in this case should be handled in such a manner that the edit clone and the original object
    will be modified.
    Hits on child objects of the editing clone ( e.g a2dHandle ), may generate new events for
    the editng clone object itself. a2dHandleMouseEvent is such an event. You can intercept this event
    in the parent event table like this EVT_CANVASHANDLE_MOUSE_EVENT( OnHandleEvent ).
    In the OnHandleEvent you can define what change a specific handle will have on the object that is edited.

    \ingroup  tools
*/
class A2DCANVASDLLEXP a2dObjectEditTool: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_ObjectEdit;

    //! when starting editing an object, this event is sent to tool
    static const a2dSignal sig_toolStartEditObject;

    //! constructor
    /*!
        \param controller controller to which the editing tool was pushed.
        \param editmode starting editing mode for objects
        \param SingleClickToEnd stop editing with a single Left Down, else double click.
    */
    a2dObjectEditTool( a2dStToolContr* controller, int editmode = 1, bool SingleClickToEnd = true );

    //! constructor
    /*!
        \param controller controller to which the editing tool was pushed.
        \param ic iteration context to setup the corridor
        \param editmode starting editing mode for objects
        \param SingleClickToEnd stop editing with a single Left Down, else double click.
    */
    a2dObjectEditTool( a2dStToolContr* controller, a2dIterC& ic, int editmode = 1, bool SingleClickToEnd = true );

    //! destuctor
    ~a2dObjectEditTool();

    //! start editing object within the given iteration context its coordinate system
    /*!
        Start editing the object within the coordinate system defined by ic (iterative context).
        Parent of startobject is defined in ic.

        The ic is used to calculate the editworld matrix, meaning relative to what coordinates system
        the editing takes place. This becomes important when editing children of objects directly.

        \remark at the top level (ShowObject of view) the editorWorld is always a2dIDENTITY_MATRIX
                else it depends on the structure of the document.
    */
    bool SetContext( a2dIterC& ic, a2dCanvasObject* startObject );

    //! start editing this object
    void StartToEdit( a2dCanvasObject* startObject );

    //! restart editing (after undo event or when changing editing mode )
    bool ReStartEdit( wxUint16 editmode );

	a2dCanvasObject* GetEditObjectCopy() { return m_canvasobject; }

    //! trigger restart editing (after undo event or when changing editing mode )
    virtual void TriggerReStartEdit( wxUint16 editmode );

    //! save to zoom
    bool ZoomSave() { return true;};

    //! set editing mode to start editing objects
    void SetMode( int mode );

    //! used extarnal by controller to activate or deactivate this tool.
    /*! If deactivated this tool, m_halted is set true. */
    void SetActive( bool active );

    //! a single non hit Left Down click is enough to end editing of an object.
    /*! If set false, a double click is needed. */
    void SetSingleClickToEnd( bool SingleClickToEnd ) { m_SingleClickToEnd = SingleClickToEnd; }

    //! when set true, other views on the document are disabled when editing a object.
    /*!
        This option is default set true, this prevents from showing editing handles in other views while
        editing an object in a view.
    */
    void SetDisableOtherViews( bool disableOtherViews ) {  m_disableOtherViews = disableOtherViews; }

    //! if set, after editing an object is finshed, it is removed from drawing.
    void SetReleaseEnd( bool releaseEnd ) { m_releaseEnd = releaseEnd; }

    void Render();


protected:

    //! To stop this tools from inside or from the outside, sets m_stop = true
    /*! The a2dStToolContr will test this with GetStopTool()  */
    void DoStopTool( bool abort );

    void OnUndoEvent( a2dCommandProcessorEvent& WXUNUSED( event ) );

    void OnRedoEvent( a2dCommandProcessorEvent& event );

    void OnKeyDown( wxKeyEvent& event );

    void OnKeyUp( wxKeyEvent& event );

    void OnChar( wxKeyEvent& event );

    virtual void AdjustRenderOptions();
    virtual bool EnterBusyMode();
    virtual void FinishBusyMode( bool closeCommandGroup = true );
    virtual void AbortBusyMode();

    //! send an event with id sig_toolComEventSetEditObject to this tool.
    /*!
        Use this event to do actions when a specific object was clicked for editing.
    */
    void SetEditObject( a2dCanvasObject* objectToEdit );

    //! redirect event to editng object or to a captured object ( handle or object itself. )
    /*! return if event is processed or not. */
    bool RedirectToEditObject( wxMouseEvent& event );

    //! start editing at m_canvasobject
    virtual bool CreateToolObjects();

    //! stop editing of m_canvasobject if busy editing
    virtual void CleanupToolObjects();

    void OnIdle( wxIdleEvent& event );

    void OnComEvent( a2dComEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    void OnDoEvent( a2dCommandProcessorEvent& event );

    virtual wxString GetCommandGroupName() { return _( "Edit object" ); }

    //! preserve the a2dCANVAS_SELECT flag after editing.
    bool m_preserve_select;

    //! a single non hit Left Down click is enough to end editing of an object.
    bool m_SingleClickToEnd;

    bool m_disableOtherViews;

    bool m_triggerRestart;

    bool m_releaseEnd;

    //! used to detect canceling of editing during the start of it (e.g. canceled a dialog)
    //! Editing with a dialog does not add the edit copy to the drawing, and therefore handled differently.
    bool m_abortInStartEdit;

public:
    DECLARE_CLASS( a2dObjectEditTool )
    DECLARE_EVENT_TABLE()

};


//!This tool does do a hit test on objects to edit, and then starts editing the object.
/*!
    When an object is hit, the object its EditStart member is called, this generates a Clone of the object
    to be edited. And event processing is redirected to the Cloned object to edit it.

    See a2dObjectEditTool for more details on editing.

    \ingroup  tools
*/
class A2DCANVASDLLEXP a2dRecursiveEditTool: public a2dObjectEditTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_RecursiveEdit;

    //! constructor
    /*!
        \param controller controller to which the editing tool was pushed.
        \param editmode starting editing mode for objects
        \param SingleClickToEnd stop editing with a single Left Down, else double click.
    */
    a2dRecursiveEditTool( a2dStToolContr* controller, int editmode = 1, bool SingleClickToEnd = true );

    //! constructor
    /*!
        \param controller controller to which the editing tool was pushed.
        \param ic iteration context to setup the corridor
        \param editmode starting editing mode for objects
        \param SingleClickToEnd stop editing with a single Left Down, else double click.
    */
    a2dRecursiveEditTool( a2dStToolContr* controller, a2dIterC& ic, int editmode = 1, bool SingleClickToEnd = true );

    //! destuctor
    ~a2dRecursiveEditTool();

protected:

    void OnComEvent( a2dComEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual wxString GetCommandGroupName() { return _( "Edit" ); }

public:
    DECLARE_CLASS( a2dRecursiveEditTool )
    DECLARE_EVENT_TABLE()

};

//!This tool does do a hit test on objects to edit, and then starts editing the object.
/*!
When an object is hit, the object its EditStart member is called, and event processing is redirected to the
object to edit.
The EditStart member sets the m_editing flag and initializes the a2dCanvasObject for editing.
This normally means adding handles which can be dragged in order to modify the object.
Rendering can also be chnaged on the basis of the m_editing flag if needed.
The default editing functionality is just editing the a2dCanvasObject its local matrix.
The TAB charecter should be used to switch between this mode and the object specific editing mode.
There are several modes to edit an object, the main mode makes a copy of the object first and editing takes
place on that copy. Updating the original object when needed.
After editing is done a double click will call the EditEnd member of the object.
Adding editing  functionality to an object is implemented by intercepting event using a static event table.
When the m_editing of a a2dCanvasObject is set, the events are handled in such a maner that the object will be modified.

    \ingroup  tools
*/
class A2DCANVASDLLEXP a2dMultiEditTool: public a2dObjectEditTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_MultiEdit;

    a2dMultiEditTool( a2dStToolContr* controller );
    ~a2dMultiEditTool();

    bool ZoomSave() { return true;};

    void OnChar( wxKeyEvent& event );

    //! start editing object
    bool StartEditing( a2dCanvasObject* startobject );

    bool StartEditingSelected();

    void SetActive( bool active );

    bool AddToGroup( a2dCanvasObject* canvasobject );

    bool RemoveFromGroup( a2dCanvasObject* canvasobject, int index = 0 );

protected:

    void DoStopTool( bool abort );

    bool RedirectToEditObject( wxMouseEvent& event );

    bool EditStart();

    void CleanupToolObjects();

    void OnIdle( wxIdleEvent& event );

    void OnComEvent( a2dComEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    void OnUndoEvent( a2dCommandProcessorEvent& event );

    void OnRedoEvent( a2dCommandProcessorEvent& event );

    //! this is intentioanally not a smart pointer (would create a reference loop)
    a2dCanvasDocument* m_doc;

public:
    DECLARE_CLASS( a2dMultiEditTool )
    DECLARE_EVENT_TABLE()

};



#endif /* __WXCANEDITOBJ_H__ */


