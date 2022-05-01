/*! \file wx/canvas/tools.h
    \brief base classes for tools and controller on top of the tools.

    a2dToolContr gets event from a2dDrawingPart before it handles the event itself.
    a2dToolContr has a stack of tools (a2dToolList) that contains one or more tools (a2dBaseTool).
    The tool first in the list is the active tool, which gets the events first.
    Tools are in general for modifying drawings interactive.

    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: tools.h,v 1.28 2009/06/03 17:38:12 titato Exp $
*/

#ifndef __A2DCANVASTOOLS_H__
#define __A2DCANVASTOOLS_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/event.h"

#include "wx/canvas/candefs.h"
#include "wx/general/smrtptr.h"
#include "wx/general/smrtptr.inl"
#include "wx/canvas/objlist.h"

//----------------------------------------------------------------------------
// decls
//----------------------------------------------------------------------------
//initalization

class A2DCANVASDLLEXP a2dBaseTool;

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr< a2dBaseTool >;
template class A2DCANVASDLLEXP std::allocator<class a2dSmrtPtr<class a2dBaseTool> >;
template class A2DCANVASDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dBaseTool>, std::allocator<class a2dSmrtPtr<class a2dBaseTool> > >::_Node >;
template class A2DCANVASDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dBaseTool>, std::allocator<class a2dSmrtPtr<class a2dBaseTool> > >::_Nodeptr >;
template class A2DCANVASDLLEXP std::list<class a2dSmrtPtr<class a2dBaseTool> >::iterator;
template class A2DCANVASDLLEXP std::list<class a2dSmrtPtr<class a2dBaseTool> >;
template class A2DCANVASDLLEXP a2dlist<class a2dSmrtPtr<class a2dBaseTool> >;
template class A2DCANVASDLLEXP a2dSmrtPtrList<a2dBaseTool>;
#endif

// it is used in editor/mastertool.h
typedef a2dSmrtPtr<a2dBaseTool> a2dBaseToolPtr;

//! list for a2dBaseTool's
/*!
    used as list of tools in a2dToolContr.
*/
class A2DCANVASDLLEXP a2dToolList: public a2dSmrtPtrList<a2dBaseTool>
{
public:
    a2dToolList();

    ~a2dToolList();


    //!Clone everything ( Clones objects also)  in a new created list
    a2dToolList* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

};

//!The a2dToolContr is the base class for Tool controller classes.
/*!
A tool controller handles redirection of events from the a2dCanvas/a2dDrawingPart Events to the
tools that are maintained by a controller.
The controller is derived from wxEvtHandler, and there for can be (un)plugged into a
a2dDrawingPart window.
It then intercepts all events on the canvas window, and sends them to the active tool.
The controller has a list of tools, only the active tool will react on the events.
More then one tool may be active, in which case skipped events will be handled by
later tools in the tool list. In the end skipped events in all tools
are handled by a2dDrawingPart itself.
\sa a2dBaseTool
\sa a2dDrawingPart
\sa a2dCanvas

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dToolContr: public a2dObject
{
public:

    //!\param view view where the controller takes events from
    a2dToolContr( a2dDrawingPart* drawingPart );

    //! destructor
    virtual ~a2dToolContr();

    //!sets the a2dDrawingPart on which the controller will work.
    /*! \param view the new canvas view to work on, if NULL this a2dToolContr
    will be disabled, and the controller will be popped out of a previous a2dCanvas.
    */
    void SetDrawingPart( a2dDrawingPart* drawingPart );

    //!Get the a2dDrawingPart object that the controller is plugged into
    a2dDrawingPart* GetDrawingPart() { return m_drawingPart; }

    //! all tools currently on the tool stack will be terminated and poped ( forced )
    /*!
        Use this when the document of the view of this controller is changed.
    */
    void StopAllTools( bool abort = true );

    //! Toolcontroller can be re-initialized
    /*!
        After changing a document on a view, and StopAllTools() was called, this function
        can be used to bring the controller back into a wanted state.
        The default does nothing.
    */
    virtual void ReStart();

    //! can be used by a2dCanvas or a2dDrawingPart to disable this class
    /*!
        for events, and also pops all tools from the tool stack.
    */
    void Disable();

    //!get currently used eventhandler (always the first in the list)
    a2dBaseTool* GetFirstTool() const;

    //! return reference to tool list
    const a2dToolList& GetToolList() const { return m_tools; }

    //!process an event for the object, if the event is not handled by
    /*! the class itself through a static event table,
        it will be sent to the first tool via ToolsProcessEvent().
    */
    bool ProcessEvent( wxEvent& event );

    //! push/pop tool:
    /*! add a tool on top of the already existing ones
        Calls first tools start, and if this returned true the tool will be pushed.
        \return true if tool its ToolStart() returned true;
    */
    virtual bool PushTool( a2dBaseTool* handler );

    //!remove first tool on the tool stack
    /*!
        Calls first the tool its StopTool() which prepares the tool for stopping.
        Calling StopTool() within a tool, automatically result in the tool to be poped by the controller,
        using this function. This happens within the event processing chain.

        \param poped the tool which is poped from the stack.
        \param force If AllowPop() return false, the PopTool is not done, unless force is true.

        \remark the tool is removed from the stack, tools are reference counted so
        if you want to preserve it, you can.

        \return true if a tool was available for poping.
    */
    virtual bool PopTool( a2dBaseToolPtr& poped, bool force = true );

    //!append a tool to the list, this tool will recieve an event if the other skipped the event to process. */
    void AppendTool( a2dBaseTool* handler );

    //!enable the tool with the given name
    /*!\param tool: classname of the tool
       \param disableothers: if true other tools in the tool list are disabled
    */
    bool EnableTool( const wxString& tool , bool disableothers );

    //!enable the given tool
    /*!\param tool: pointer to the tool
       \param disableothers: if true other tools in the tool list are disabled
    */
    bool EnableTool( a2dBaseTool* tool , bool disableothers );

    //!disable the tool with the given name
    /*!Disabling a tool means that it will not receive any events.
       \param tool: classname of the tool
    */
    bool DisableTool( const wxString& tool );

    //!disable the tool with the given name
    /*!Disabling a tool means that it will not receive any events.
       \param tool: pointer to the tool
    */
    bool DisableTool( a2dBaseTool* tool );

    //!search for the tools in the tool list with the given name.
    /*!\param tool: classname of the tool */
    a2dBaseTool* SearchTool( const wxString& tool );

    //!(de)activate the first tool in the list.
    /*! (de)activates the first tool on the stack, and deactivates all others
    */
    void ActivateTop( bool active );

    //!Activate the tool with the given name
    /*!\remark  DIS activating a tool means it will skip mouse events
       \remark  it will still receive other events like Onpaint Onchar.
       \param tool: pointer to the tool
       \param disableothers: if true other tools in the tool list are made inactive
    */
    bool Activate( const wxString& tool, bool disableothers );

    //! start editing object using existing corridor to object
    /*!
        Start editing the object within the coordinate system defined by the corridor set by for instance
        a2dIterC::SetCorridorPath() Parent of startobject is defined as last object in corridor.

        The corridor is used to calculate the editworld matrix, meaning relative to what coordinates system
        the editing takes place. This becomes important when editing children of objects directly.

        \remark if a corridor is not set in the a2dCanvasDocument, the ShowObject() of the a2dDrawingPart is used.

        \remark at the top level (ShowObject of view) the editorWorld is always a2dIDENTITY_MATRIX
                else it depends on the structure of the document.
    */
    virtual bool StartEditingObject( a2dCanvasObject* objectToEdit );

    //! start editing object using iteration context to define corridor.
    /*!
        The editcopy created of the objectToEdit is used to capture the corridor to that object.
    */
    virtual bool StartEditingObject( a2dCanvasObject* objectToEdit, a2dIterC& ic );

    //! trigger restart editing (after undo event or when changing editing mode )
    /*!
        An editing tool on a a2dCanvasObject can be restarted in derived controller.
    */
    virtual bool TriggerReStartEdit( wxUint16 editmode );

    //! define corridor for the controller its first tool
    bool SetCorridor( const a2dCorridor& corridor );

    //! render the tool chain
    /*!
        A tool chain gets rendered when a2dDrawingPart is updating areas that are changed in a document.
    */
    virtual void Render();

    //!set snap on or off.
    /*!
        When snapping is on, tools will respect this, and drawing will be on the snap grid.
    */
    void SetSnap( bool doSnap );

    //!get current snap setting ( true or false ).
    inline bool GetSnap() { return m_snap; }

    //! see m_eventHandler
    void SetDefaultToolEvtHandler( a2dObject* handler ) { m_defaultEventHandler = handler; }

    //! see m_eventHandler
    a2dObject* GetDefaultToolEventHandler() { return m_defaultEventHandler; }

    //! see m_defaultEventHandlerFixedStyle
    void SetDefaultToolEvtHandlerFixedStyle( a2dObject* handler ) { m_defaultEventHandlerFixedStyle = handler; }

    //! see m_defaultEventHandlerFixedStyle
    a2dObject* GetDefaultToolEventHandlerFixedStyle() { return m_defaultEventHandlerFixedStyle; }

    virtual void SetStateString( const wxString& message = wxEmptyString, size_t field = 0 );

protected:

    //! Event not handled in this class are redirected to the chain of tools.
    /*!
        When there are tools in the tools chain, the first tools receives the event first.

        If you derive a class from this controller class here, you might not want to use the
        base event handling for some type of events, even if you do not handle it
        yourself in your static event callback.
        Normally handling an event in the baseclass is achieved with event.Skip().
        ( e.g in OnMouse or OnChar ).
        But by calling ToolsProcessEvent() function here directly, skipping is not needed, and the event
        will directly go to the first available tool.

        \remark If new tools are pushed while handling the event in the currently available tools,
        and the event itself was skipped ( not handled ), it will be redirect to the pushed tools.
        This is in a recursive manner, so there may be a chain of extra pushed tools.

        \remark If a tool is stopped ( \see GetStopTool() ), it is poped from the tools stack.
        And if the event is not processed yet, it is sent to the next/new first tool.
        More tools can be stopped in a chain of tools as a respons to one and the same event.

        \see ProcessEvent

        \param event the event which will be sent to the tools.

        \return true if the event was processed and event.Skip() was not called.
    */
    bool ToolsProcessEvent( wxEvent& event );

    //! sets the focus to the display window.
    void OnEnter( wxMouseEvent& WXUNUSED( event ) );

    //!window Onpaint is received here before the window Onpaint where this controller is plugged into.
    /*!In general a flag is set to do the job in Onidle time after the window itself is repainted */
    void OnPaint( wxPaintEvent& event );

    void OnIdle( wxIdleEvent& event );

    //!tool list
    a2dToolList m_tools;

    //!a2dDrawingPart where tool is plugged into
    a2dDrawingPart* m_drawingPart;

    //! set when poping a tool is in progress.
    //! This is used to prevent recursive calls from event sent around in the poping process.
    bool m_bussyPoping;

    //!snap is on or off
    bool m_snap;

    //! use to define default handlers in tools.
    /*!
        Several tools use a centralized defaultEventHandler, 
        which is called to process events before handling it in the tool itself.
        Here the default one in those tools can be set.
    */
    a2dSmrtPtr<a2dObject> m_defaultEventHandler;

    //! use to define default handlers in tools.
    /*!
        Several tools use a centralized defaultEventHandler, 
        which is called to process events before handling it in the tool itself.
        Here the default one in those tools can be set, for tools having a fixed style.
    */
    a2dSmrtPtr<a2dObject> m_defaultEventHandlerFixedStyle;

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; };

    DECLARE_EVENT_TABLE()
    DECLARE_CLASS( a2dToolContr )
};

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dToolContr>;
#endif

//-----------------------------------------------------------
// tool modifier on action
//-----------------------------------------------------------

class A2DCANVASDLLEXP a2dToolFunctor;
class A2DCANVASDLLEXP a2dCanvasCommandProcessor;

typedef a2dSmrtPtr<a2dToolFunctor> a2dToolFunctorPtr;

DECLARE_MENU_ITEMID( CmdMenu_NoMenu )

//!The a2dBaseTool is used to derive tools from that are controlled by
/*! a a2dToolContr derived class.

    The basetool does not implement default behaviour for events in many cases, instead it is possible to set m_eventHandler to take
    care of action which are the same for a set of tools. a2dBaseTool::ProcessEvent() first redirect incoming events to
    m_eventHandler, and if not handled there it goes to the tools its own event handler.

    - wxEventType sig_toolPushed sent from controller just after this tool is pushed,
    the tool is the first on the stack. This event can be used to have a master tool push extra tools.

    - a2dComEvent( GetFirstTool(), poped,  &sig_toolPoped  )  sent from controller  to tool when some
    other tool is poped from the tool stack. This can be used to control style settings for this tool,
    based on the last active tool on the top of the tool stack. E.g When style is changed when editing
    a primitive, the tool that was used to draw this primitive and started the editing tool, normally wants to take over this style.
    This function is called just after a tool was poped from the stack.

    - a2dComEvent( GetFirstTool(), poped,  &sig_toolBeforePoped  )  sent from controller when a new tool is pushed,
    while the current tool is on the stack. This can be used to control halting or stopping of the current tool.
    E.g. When a tool allows Zooming while busy, you halt the tool here, but while the new tool is totataly independent,
    one may terminate the current tool action and close its command group. This function is called just before the new
    tool is indeed pushed on the stack.

  \sa  a2dToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dBaseTool: public a2dObject
{
public:

    //! sent to new first tool when tool was pushed
    static const a2dSignal sig_toolPushed;
    //! sent to new first tool when last first tool was poped
    static const a2dSignal sig_toolPoped;
    //! sent to current firsttool so it can pop itself based on the tool to push.
    //! Allows the current first tool to react on a the pushing of the new tool.
    static const a2dSignal sig_toolBeforePush;
    //! return id after a sig_toolBeforePush, to tell current tool needs to be poped.
    static const a2dSignal sig_toolDoPopBeforePush;
    //! not yet used.
    static const a2dSignal sig_toolComEvent;

    //! events recieved from controller processed here
    /*!
        first m_eventHandler is tried, if not set or not processed, try object itself.
    */
    bool ProcessEvent( wxEvent& event );

    //! to reroute events to this event handler
    void SetEvtHandler( a2dObject* handler ) { m_eventHandler = handler; }

    a2dObject* GetEventHandler()
    {
        if ( m_eventHandler )
            return m_eventHandler;
        return this;
    }

    //!construct a new tool for the given controller.
    a2dBaseTool( a2dToolContr* controller, const a2dMenuIdItem& initiatingMenuId = CmdMenu_NoMenu() );

    //! constructor initializing partly from other tool
    a2dBaseTool( const a2dBaseTool& other, CloneOptions options, a2dRefMap* refs );

    //!destructor
    virtual ~a2dBaseTool();

    inline a2dBaseTool* TClone( CloneOptions options, a2dRefMap* refs = NULL ) { return ( a2dBaseTool* ) Clone( options, refs ); }

	virtual wxString GetToolString() const;

    //!called to initiate while pushing tool to toolcontroller
    /*! It also resets the m_stop flag to false.
    */
    virtual bool StartTool( a2dBaseTool* currenttool );

    //!Only one action of the tool, after that it will ask the controller to stop this tool.
    void SetOneShot() { m_oneshot = true; }

    //!Is Zooming while the tool is busy Save.
    /*!If so, the active tool will redraw itself in the new Mapping state */
    virtual bool ZoomSave() = 0;

    //! tells if a tool can be poped from the stack.
    /*!
        The controller tests if a tool is allowed to be poped from the stack.
        When a tool returns false here, that tool will stay on the stack until controller
        is destructed.
    */
    virtual bool AllowPop() { return true; }

    //! call to stop a tool, internal and external.
    /*! The stop flag within the tool will be set, triggering deletion of the tool
        from the outside by the a2dToolContr. Stopping one tool often means activating
        another which is on the tool stack of the a2dToolContr. Therefore the tools
        are really stopped and removed from the tool stack from the outside.

        After this call the tool must be left in a state that allows the controller to pop the tool.

        Override DoStopTool() function, if there is cleaning up to do before stopping a tool.
        Understand that stopping the tool is something else then (de)activating a tool.
        A tool can be de-activated, but still in action or at least on the toolstack.
        Stopping a tools really removes the tool from the stack by the controller.

        To prevent poping the tool from the outside before it is really stopped,
        m_bussyStopping is incremented, when recursive calling this function.
        GetStopTool() only returns true when all recursive StopTool() calls have returned.

        \remark If a tool can not be stopped right now, the function should return false.

        \remark Stopping a tools will indirectly lead to the a2dToolContr Popping the
        tool from the stack, and that action at least de-activates the tool just stopped.
        So indirectly SetActive( false ) is called on this tool, when it is really stopped.

        \param abort if true the tool is calling AbortBusyMode() instead FinishBusyMode(),
        the effect is that the tool does directly Undo what it was doing, but not yet did finish.
    */
    void StopTool( bool abort = false );

    //!checked by controller to see if the tool needs to be stopped e.g. after a oneshot.
    /*! or when wanted.
        The stop flag is in general set after a tool has received and handled an event
        which should stop the tool. A tool should not stop itself, since the tool will be deleted.
        The StopTool() function is called in such a case to trigger the stopping of the tool.
    */
    bool GetStopTool();

    //!is the tool active?
    /*! A tool is "active", when it receives mouse input. Usually this is only the top tool
        on the tool stack. This is something completely different then "busy".
        A tool is "busy", when it currently focuses on editing a specific object.
        Toolstates change like this:
        1. Created
        2. Pushed on the tool stack -> set active
        3. User clicks on an object to edit -> set busy
        4. User starts a different tool (e.g. zoom) -> busy but not active (halted)
        5. subtool is stopped -> set active again (and busy)
        6. User continues editing the object selected in 3. -> still busy
        7. User finishes editing the object selected in 3. -> no longer busy, but still active
           "One-shot" tools are stopped in this situations, other tools continue.
        8. User clicks on a different object, repeat step 3..7 -> busy again
        9. User stops tool (end tool menu or ESC or 2x double-click ...) -> stopped
        10 Tool gets popped from the tool stack
    */
    bool GetActive() { return m_active; }

    //!set the tool active or inactive.
    /*!
        If the tool needs initializing after it was inactive for a while,
        override this function to re-initialize the tool.
        This function can be used to distribute its settings for style etc. to the documents
        \see a2dCanvasDocument::GetCanvasCommandProcessor().
        The default implementation sets fill, stroke to the commandprocessor,
        but only when m_doSetStyleToCommandProc is true.
        If you need more complicated behaviour just override.
    */
    virtual void SetActive( bool active = true );

    //! Check if the tool is busy editing a distinct object */
    /*! \see GetActive for a discription of various tool states */
    bool GetBusy() { return m_busy; }

    //! Called when the user selects a distinct object for editing */
    /*! \see GetActive for a discription of various tool states
        You should call the base class (this) version at the END of your function
    */
    virtual bool EnterBusyMode();

    virtual bool EnterBusyModeNoGroup();

    //! Called when the user finishes editing a distinct object */
    /*! \see EnterBusyMode, \see GetActive
        You should call the base class (this) version at the END of your function
    */
    virtual void FinishBusyMode( bool closeCommandGroup = true );

    //! Called when the user aborts editing a distinct object */
    /*! \see EnterBusyMode
        This is supposed to undo aeverything the tool did since entering busy mode
    */
    virtual void AbortBusyMode();

    //!set fill if used inside a tool
    void SetFill( const a2dFill& fill );

    //!get the current fill
    /*!
        Return the current fill
    */
    const a2dFill& GetFill() const { return m_fill; }

    //!set stroke if used inside a tool
    void SetStroke( const a2dStroke& stroke );

    //!get the current stroke
    /*!
        Return the current stroke
    */
    const a2dStroke& GetStroke() const { return m_stroke; }

    //! layer set for the object that is drawn using a tool
    void SetLayer( wxUint16 layer );

    //!layer set for new objects.
    inline wxUint16 GetLayer() { return m_layer; }

    //!when true anotation will be shown, if used inside a tool
    /*! tools can show text when in action (e.g radius of the circle that is drawn)
    This is called anotation.
    The flag set by this function can be used to enable or disable the anotation.
    */
    void SetShowAnotation( bool show ) { m_anotate = show; }

    //! font to use for anotation
    void SetAnotationFont( const wxFont& font ) { m_annotateFont = font; }

    //! Sets cursor which the tool should use when started
    /*!
        You may change the predefined cursor which the tool uses when it
        becomes active.

        \remark
        Many tools are using more than one cursor. These cursors indicate
        the status of the tool (busy etc.) and only the busy cursor can be
        changed currently.
        This cursor only indicate that the tool is active.

        \param cursor the tool cursor
    */
    void SetCursorType( const wxCursor& cursor ) { m_toolcursor = cursor; }

    //! Sets cursor which the tool should use when busy
    /*!
        You may change the predefined cursor which the tool uses when it
        becomes busy.

        \param cursor the tool cursor
    */
    void SetBusyCursorType( const wxCursor& cursor ) { m_toolBusyCursor = cursor; }

    //!what cursor is used when tool is started
    wxCursor GetCursorType() { return m_toolcursor; }

    //!what cursor is used when tool is busy
    wxCursor GetBusyCursorType() { return m_toolBusyCursor; }

    //! general integer to set operation modes for a tool (e.g the way it draws)
    /*! tools most check the mode and handle in accordance
        In general Tab should be used to tab/switch/circle between modes
        understood by a tool.
    */
    virtual void SetMode( int mode ) { m_mode = mode; }

    //! can be used to modify the behaviour of the derived tool.
    /*!
        Tools may have several ways of doing the same or simular things.
        This integer can be used to set this mode, it depends on the implementation
        what happens.
    */
    int GetMode() { return m_mode; }

    //! a tool is set pending when it needs to be redrawn.
    void SetPending( bool pending = true ) { m_pending = pending; }

    //! return if the tool is set pending for redraw.
    bool GetPending() { return m_pending; }

    //! render the tool chain
    /*!
        A tool chain gets rendered when a2dDrawingPart is updating areas that are changed in a document.
    */
    virtual void Render() {};

    //! to get the tool controller to which this tool is attached.
    a2dToolContr* GetToolController()
    {
        return m_controller;
    }
    //! Access to the tool controllers drawer
    a2dDrawingPart* GetDrawingPart()
    {
        return m_controller->GetDrawingPart();
    }
    //! Access to the tool controllers drawers drawer2d
    a2dDrawer2D* GetDrawer2D();

    //! Access to the tool controllers drawers canvas
    wxWindow* GetDisplayWindow();

    //!Returns a pointer to the drawing
    a2dDrawing* GetDrawing();

    //!Returns a pointer to the command processor associated with this document
    a2dCanvasCommandProcessor* GetCanvasCommandProcessor();

    //! Get the hitmargin in world coordinates, base on the pin a2dCanvasGlobals->GetPinSize()/2.0
    //! If a2dPin its pinsize is pixelbased, it is translated to world calculated based on the active a2dDrawingPart. 
    double GetHitMargin();

    //!called when starting an editing operation (e.g. on mouse down)
    /*!\param restart add a hint to the command group name, that this tool was restarted */
    virtual void OpenCommandGroup( bool restart );
    //!called when starting an editing operation with another than the default name
    virtual void OpenCommandGroupNamed( const wxString& name );
    //!called when ending an editing operation (e.g. mouse up)
    virtual void CloseCommandGroup();
    //!return the command group name for commands of a derived class
    /*!this defaults to the class name */
    virtual wxString GetCommandGroupName();

    //! return the command group that is open else NULL.
    a2dCommandGroup* GetCommandgroup() { return m_commandgroup; }

    //! to get the current mouse menu of the tool
    /*! see also RightMouseClick()
        /return NULL if no menu is set, else the menu.
    */
    wxMenu* GetMousePopupMenu() { return m_mousemenu; }

    //! to set the current mouse menu of the tool
    /*!  see also RightMouseClick()
         NULL sets the menu to non.
    */
    void SetMousePopupMenu( wxMenu* mousemenu );

    //! parent object relative to which the actions take place.
    /*!
        Tools can do there work on child objects of some parent a2dCanvasObject.
        In such cases the context of the tool is set with SetCorridor(), and the last object in the corridor
        is the m_parentobject.
    */
    a2dCanvasObject* GetParentObject() { return m_parentobject; }

    //! Add an editcopy object to the tool/document
    void AddEditobject( a2dCanvasObject* object );
    //! Remove an editcopy object to the tool/document
    void RemoveEditobject( a2dCanvasObject* object );

    //! Add a decoration object to be rendered by the tool.
    /*! Decoration object are rendered by the tool in the style they have.
    They are used to add extra drawings to the actual tool object to clarify editing and drawing.*/
    void AddDecorationObject( a2dCanvasObject* object );

    //! remove all object that were added as decorations.
    void RemoveAllDecorations();

    //! remove from the list of decoration objects (must be children of m_top)
    void RemoveDecorationObject( a2dCanvasObject* obj );

    //! Switches ignorance of pending objects on and off.
    /*!
        If set to <code>true</code> pending objects won't be updated.
        This method is used by several tools (i.e. a2dDragTool) to avoid updates
        while the tool is working.

        \see GetUpdatesPending()

        \param onoff <code>true</code> to ignore pending objects, else <code>false</code>
    */
    void SetIgnorePendingObjects( bool onoff );

    //!get setting of ignore pending objects /sa GetUpdatesPending()
    bool GetIgnorePendingObjects() { return m_ignorePendingObjects; }

    //! get reference to the corridor list
    a2dCorridor& GetCorridor() { return m_corridor; }

    //! set a corridor from a list of objects
    void SetCorridor( const a2dCorridor& corridor );

    void SetContourWidth( double width );

    //!get the Contour width of the shape
    double GetContourWidth() const { return m_contourwidth; }

    //! context like corridor and parentobject are reset
    void ResetContext();

    //! rotate object of first tool on the stack, when appropriate.
    virtual bool RotateObject90LeftRight( bool right ) { return false; }

    void SetIsEditTool( bool isEditTool ) { m_isEditTool = isEditTool; }

    bool GetIsEditTool() { return m_isEditTool; }

    //! Some tools work on selection of objects, other do deselect all objects first.
    void SetDeselectAtStart( bool deselectAtStart ) { m_deselectAtStart = deselectAtStart; }

    bool GetDeselectAtStart() { return m_deselectAtStart; }


    void SetInitiatingMenuId( const a2dMenuIdItem& initiatingMenuId ) { m_initiatingMenuId = &initiatingMenuId; }

    const a2dMenuIdItem& GetInitiatingMenuId() { return *m_initiatingMenuId; }

protected:

    //! command which initiated the tool, used to seperate commands using the tool
    const a2dMenuIdItem* m_initiatingMenuId;

    //! default handler for a2dComEvent event
    /*!
        Event id a2dDrawingPart::sig_changedShowObject makes the tools corridor change to keep the tool active.
        In derived tools one can implement different behaviour if needed.
    */
    void OnComEvent( a2dComEvent& event );

    void OnDoEvent( a2dCommandProcessorEvent& event );
    void OnUndoEvent( a2dCommandProcessorEvent& event );
    void OnRedoEvent( a2dCommandProcessorEvent& event );

    //! called on mouse events
    void    OnMouseEvent( wxMouseEvent& event );
    //! called on key events
    void    OnChar( wxKeyEvent& event );
    //! called on keydown events
    void    OnKeyDown( wxKeyEvent& event );
    //! called on keyup events
    void    OnKeyUp( wxKeyEvent& event );

    //! to do tool specific stuff to stop a tool. Called from StopTool().
    virtual void DoStopTool( bool abort );

    //! cursor to use
    wxCursor m_toolcursor;

    //! cursor to use when the tool is busy doing something.
    wxCursor m_toolBusyCursor;

    //! under control of this toolcontroller, to give me events.
    a2dToolContr* m_controller;

    //! tool is operational
    bool m_active;

    //! if > 0, the tool is in the process of stopping, which means it can not be poped yet by controller.
    wxUint8 m_bussyStopping;

    //! when set called before own event handler
    /*!
        To be able to change the behaviour in a central location for all tools, one can set this event handler.
        The default handler is the tool itself.
    */
    a2dSmrtPtr<a2dObject> m_eventHandler;

    //!if set ignore all setting for pendingobjects
    bool m_ignorePendingObjects;

    //! used in tools that can do nested editing/drawing.
    //! the corridor is a copy of the a2dCanvasObject in the a2dDocument that form the corridor path.
    //! re-activating a tool, this list can be used to re-instantiate the corridor path for the tool.
    a2dCorridor m_corridor;

    //! if tool does change drawing
    bool m_isEditTool;

    //! deselect at start
    bool m_deselectAtStart;

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }

    //!tool is busy with something (can not interrupt/stop it)
    /*! This is private, because it should only be changed by Enter/Finish/AbortBusyMode
    */
    bool m_busy;

public:
    //! if a tool is deactivated while m_busy is true, this flag is set
    bool m_halted;

    //! set when tool needs an redraw (after a a2dCanvas Repaint etc.)
    /*! this result in redrawing the tool ( e.g after a change ) */
    bool m_pending;

    //! do it only once
    bool m_oneshot;

    //! stop the tool
    bool m_stop;

    //!fill for new object
    a2dFill m_fill;

    //!stroke for new object
    a2dStroke m_stroke;

    //! if != 0 the polygon is contoured at distance m_contourwidth/2
    double m_contourwidth;

    //!layer for a new object
    wxUint16 m_layer;

    //!used to save the a2dCanvas mouse event setting.
    /*! when tool is destroyed the original mouse event setting is set for a2dCanvas. */
    bool m_canvas_mouseevents_restore;

    //!when true anotation will be shown, if used inside a tool
    bool m_anotate;

    //! font to use for anotation
    wxFont m_annotateFont;

    //! general operation mode setting for a tool.
    int m_mode;

    //!the command group of the command processor
    a2dCommandGroup* m_commandgroup;

    //!popup menu
    wxMenu* m_mousemenu;

    //! ( if needed ) parent a2dCanvasObject relative to which the tool actions take place.
    a2dCanvasObjectPtr m_parentobject;

public:
    static a2dPropertyIdBool* PROPID_Oneshot;
    static a2dPropertyIdBool* PROPID_Stop;
    static a2dPropertyIdFill* PROPID_Fill;
    static a2dPropertyIdStroke* PROPID_Stroke;
    static a2dPropertyIdUint16* PROPID_Layer;

    DECLARE_PROPERTIES()

public:
    DECLARE_EVENT_TABLE()
    DECLARE_CLASS( a2dBaseTool )

};


//-----------------------------------------------------------
// tool event handler
//-----------------------------------------------------------

//! To implement behaviour on a set of tools.
/*!
    A a2dToolEvtHandler is/can be set to a a2dBaseTool as an event handler in between the tool its own event handler.
    This is the easiest way to make a whole set of tools behave to a certain model.

    \ingroup  tools

*/
class A2DCANVASDLLEXP a2dToolEvtHandler: public a2dObject
{
    DECLARE_EVENT_TABLE()

public:

    //!
    a2dToolEvtHandler();

    void OnComEvent( a2dComEvent& event );

    //! event recieved from tools processed here
    bool ProcessEvent( wxEvent& event );

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }
};

#endif //__A2DCANVASTOOLS_H__

