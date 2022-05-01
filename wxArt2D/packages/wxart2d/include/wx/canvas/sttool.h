/*! \file wx/canvas/sttool.h
    \brief stack based tools controller and tools for drawing and editing.

    This is a whole set of tools and controller which work closely together,
    such that they can function well inside one application.
    The controller uses a stack of tools, where the first one is always active.

    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: sttool.h,v 1.48 2009/10/01 19:22:35 titato Exp $
*/

#ifndef __A2DCANVASSTTOOL_H__
#define __A2DCANVASSTTOOL_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/listimpl.cpp>

#include "wx/canvas/canobj.h"
#include "wx/canvas/tools.h"
#include "wx/canvas/restrict.h"

class a2dVectorPath;

WX_DECLARE_LIST_WITH_DECL( a2dBoundingBox, wxZoomList, class A2DCANVASDLLEXP );



//! zoom menu
/*!
  \ingroup  tools
*/
A2DCANVASDLLEXP_DATA( extern const long ) TC_UNDO;
A2DCANVASDLLEXP_DATA( extern const long ) TC_REDO;
A2DCANVASDLLEXP_DATA( extern const long ) TC_ENDTOOL;
A2DCANVASDLLEXP_DATA( extern const long ) TC_ZOOMIN;
A2DCANVASDLLEXP_DATA( extern const long ) TC_ZOOMOUT;
A2DCANVASDLLEXP_DATA( extern const long ) TC_ZOOMBACK;
A2DCANVASDLLEXP_DATA( extern const long ) TC_ZOOMOUT2;
A2DCANVASDLLEXP_DATA( extern const long ) TC_ZOOMIN2;
A2DCANVASDLLEXP_DATA( extern const long ) TC_ZOOMPANXY;
A2DCANVASDLLEXP_DATA( extern const long ) TC_PROPERTY_XY;



enum a2dSTAT_ID
{
	STAT_Xd_Yd,
	STAT_Xw_Yw,
	STAT_ToolString,
	STAT_toolHelp = 3,
	STAT_Feedback = 10,
	STAT_Feedback2
};

/*!
  \ingroup  tools
*/
enum DRAGMODE
{
    // !!!! I think we need two versions of the RECTANGLE and COPY modes, one in which
    // the original is shown in its original position, and one in which the editcopy is
    // drawn twice, once in edit-mode and once in usual mode.

    // All these modes are now implemented in a2dStTool::Render
    // Even the rectangle mode can be done there by drawing a rectangle with the
    // size of the bounding box for every object.

    // Depending one the mode, some m_renderXXXX flags of the tool are set.

    //! draw a rectangle the size of the bounding box of the object draged.
    wxDRAW_RECTANGLE,
    //! seperate the object from the document, and layer, so it will be draged on top of all other objects.
    wxDRAW_ONTOP,
    //! object stay in document and it is redrawn while dragging, and so are all objects above and below it.
    wxDRAW_REDRAW,
    //! a clone is created which is draged in an outline mode, original stays at position until drag is finished.
    wxDRAW_COPY
};

/*!
  \ingroup tools
*/
enum wxTC_BEHAVIOR
{
    wxTC_DefaultBehavior       = 0x00000000, /*!< enable all Default Behavior enabled (Default) */
    wxTC_NoDefaultKeys         = 0x00000001, /*!< skip the onchardown & up eventhandler */
    wxTC_NoContextMenu         = 0x00000002, /*!< skip the onmouse && rightdown event */
    wxTC_NoDefaultMouseActions = 0x00000004, /*!< skip the onmouse && controlkeys {shift,control,alt} event*/
    wxTC_NoAll                 = 0xFFFFFFFF  /*!< Set All posible flags  */
};

class A2DCANVASDLLEXP a2dStToolContr;
#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dStToolContr>;
#endif

class A2DCANVASDLLEXP a2dObjectEditTool;

//!The a2dStTool is used to derive tools from.
/*! Those are controlled by
    a a2dStToolContr class.
    Those type of tools understand zooming and stacked redraw.
    \sa a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dStTool: public a2dBaseTool
{
public:

    //! send when tool was poped
    //! \ingroup events
    static const a2dSignal sig_toolComEventAddObject;
    //! send when tool starts editing object
    //! \ingroup events
    static const a2dSignal sig_toolComEventSetEditObject;

    //!\param controller Pointer to a a2dStToolContr controller.
    a2dStTool( a2dStToolContr* controller );

    //! constructor
    a2dStTool( const a2dStTool& other, CloneOptions options, a2dRefMap* refs );

    //!destructor
    virtual ~a2dStTool();

    //! halts the tool is busy
    virtual bool OnPushNewTool( a2dBaseTool* newtool );

    virtual void SetActive( bool active = true );

    //! starts a new action (e.g drawing something ) in a tool that is already pushed.
    /*!
        Opens a new a2dCommandGroup, which will be closed at the end of an action.
        This is normally in FinishBusyMode(), but might be delayed when other tools are pushed,
        like the edit tool to further edit the drawn figure. This should become part
        of the same command group.
    */
    virtual bool EnterBusyMode();

    virtual void FinishBusyMode( bool closeCommandGroup = true );
    virtual void AbortBusyMode();

    //! add the current canvas object to parent as child
    /*!
          \param objectToAdd this is the object to add to the current parentobject
    */
    void AddCurrent( a2dCanvasObject* objectToAdd );

    //! implement rendering
    /*!
        in general is rendering for tools just to draw/render m_canvasobject when set.
    */
    virtual void Render();

    //! to render the tools its anotation text.
    /*!
        Tools can have a line of text displayed while in action, this is done in pixel size.
    */
    void RenderAnotation();

    //! return the staacked tool controller
    /*! this cast is save, because a2dStTool takes a a2dStToolContr as
    constructor argument
    */
    a2dStToolContr* GetStToolContr() { return ( a2dStToolContr* ) m_controller; }

    //! to start editing tool when wanted
    virtual a2dObjectEditTool* StartEditTool( a2dCanvasObject* objecttoedit );

    //! flag for setting editing tool after drawing a primitive.
    /*!
        \param editatend when true the drawing tool will start a editing tool after drawing a primitive.

        \remark it depends on the implementatian of the tool if this flag is used.
    */
    void SetEditAtEnd( bool editatend ) { m_editatend = editatend; }

    //! see SetEditAtEnd
    bool GetEditAtEnd() { return m_editatend; }

    //! how editing transparency will be handled
    enum a2dOpaqueMode
    {
        a2dOpaqueMode_Off = 0x00000000, /*!< tool will not use apaque edit mode */
        a2dOpaqueMode_Tool = 0x00000001, /*!< tool will apaque edit mode and setting are from the tool*/
        a2dOpaqueMode_Tool_FixedStyle = 0x00000002, /*!< tool will apaque edit mode and setting are from the tool but style is fixed*/
        a2dOpaqueMode_Controller = 0x00000004, /*!< tool will use apaque edit mode as set in Controller*/
    };

    //! when enabling m_useEditOpaque, this is how transparent the editcopy will be made.
    /*!
        \param editOpacity Real opacity is set opacity * m_OpacityFactor/255
    */
    void SetOpacityFactorEditcopy( wxUint8 editOpacity ) { m_editOpacityFactor = editOpacity; }

    //! When set true, instead of using a special style for editcopies, a clone of the
    /*!
        fill and stroke of the original are made, and used for the editcopy, but its transparancy is
        set using m_editOpacityFactor.
        The effect is that when dragging such an object, that in the editcopy mode, a half transparent object is shown.
    */
    void SetUseOpaqueEditcopy( a2dOpaqueMode editOpaque ) { m_useEditOpaque = editOpaque; }

    //! Set class for generating new connection objects between object and pins
    void SetConnectionGenerator( a2dConnectionGenerator* connectionGenerator ) { m_connectionGenerator = connectionGenerator; };

    //! Get class for generating new connection objects between object and pins
    a2dConnectionGenerator* GetConnectionGenerator() const { return m_connectionGenerator; }

    //! object to draw or edit
    a2dCanvasObject* GetOriginal() { return m_original; }

    //! defines the context, relative to which this tools works.
    /*!
        The more advanged usage of tools is in situation where the tools is used inside a2dCanvasObjects.
        So the context a2dIterC is in such a situation relative to that object its position.
    */
    bool SetContext( a2dIterC& ic, a2dCanvasObject* startObject = NULL );

    void SetStateString( const wxString& message = wxEmptyString, size_t field = 0 );

    //! set what snapping features or enabled for the source to snap to.
    //! see a2dSnapToWhat for snapping features.
    void SetSnapSourceFeatures( wxUint32 snapSourceFeatures ) { m_snapSourceFeatures = snapSourceFeatures; }

    //! set one of the snapping features for the source to true or false, leaf others as is
    void SetSnapSourceFeature( a2dRestrictionEngine::a2dSnapToWhat snapSourceFeature, bool value = true )
    {
        m_snapSourceFeatures = value ? m_snapSourceFeatures | snapSourceFeature :
                               m_snapSourceFeatures & ( a2dRestrictionEngine::snapToAll ^ snapSourceFeature );
    }

    //! set what snapping features or enabled for the source to snap to.
    //! see a2dSnapToWhat for snapping features.
    a2dSnapToWhatMask GetSnapSourceFeatures() const { return m_snapSourceFeatures; }

    //! set what snapping features or enabled for the target to snap to.
    //! see a2dSnapToWhat for snapping features.
    void SetSnapTargetFeatures( wxUint32 snapTargetFeatures ) { m_snapTargetFeatures = snapTargetFeatures; }

    //! set one of the snapping features for targets to true or false, leaf others as is
    void SetSnapTargetFeature( a2dRestrictionEngine::a2dSnapToWhat snapTargetFeature, bool value = true )
    {
        m_snapTargetFeatures = value ? m_snapTargetFeatures | snapTargetFeature :
                           m_snapTargetFeatures & ( a2dRestrictionEngine::snapToAll ^ snapTargetFeature );
    }

    //! return the setting of a specific snapping feature
    bool GetSnapTargetFeature( a2dRestrictionEngine::a2dSnapToWhat snapTargetFeature ) const
    {
        return ( m_snapTargetFeatures & snapTargetFeature ) > 0;
    }

    //! set what snapping features or enabled for the target to snap to.
    //! see a2dSnapToWhat for snapping features.
    a2dSnapToWhatMask GetSnapTargetFeatures() const { return m_snapTargetFeatures; }

    a2dCanvasObject* GetCanvasObject() const { return m_canvasobject; } 
    a2dCanvasObject* GetOriginalObject() const { return m_original; } 

protected:


    void PrepareForRewire( a2dCanvasObjectList& dragList, bool walkWires = true, bool selected = false, bool stopAtSelectedWire = false, bool CreateExtraWires = true, a2dRefMap* = NULL );

    void DeselectAll();

    //! calculate world coordinates from devide coordinates
    void MouseToToolWorld( int x, int y, double& xWorldLocal, double& yWorldLocal );

    //! area occupied by this object
    wxRect GetAbsoluteArea( a2dCanvasObject* object );

    //! return text extends of string
    void GetTextExtent( const wxString& string, wxCoord* w, wxCoord* h, wxCoord* descent = NULL, wxCoord* externalLeading = NULL );

    //! handler for paint event
    void OnPaint( wxPaintEvent& event );

    //! handler for idle events
    void OnIdle( wxIdleEvent& event );

    //! handler for Undo event
    void OnUndoEvent( a2dCommandProcessorEvent& event );

    //! handler for Do event
    void OnDoEvent( a2dCommandProcessorEvent& event );

    //!called when a tool has changed (fill stroke layer spline )
    void OnComEvent( a2dComEvent& event );

    //! to display a string along with a tool drawing.
    virtual void GenerateAnotation();

    //! after calculating anotation position and string, extend the boundingbox of the tools with it.
    void AddAnotationToPendingUpdateArea();

    //! get the increments used when moving is done with cursor keys
    void GetKeyIncrement( double* xIncr, double* yIncr );

    //!called on key down events
    void OnKeyDown( wxKeyEvent& event );
    //!called on key up events
    void OnKeyUp( wxKeyEvent& event );

    //!called on key events
    void OnChar( wxKeyEvent& event );

    //!called on mouse events
    void OnMouseEvent( wxMouseEvent& event );

    //!Adjust the rendering options to the needs of this tool
    virtual void AdjustRenderOptions();

    //!Create the editcopy and other tool objects (e.g. decorations)
    /*! m_original must be set before this function is called */
    virtual bool CreateToolObjects();

    //!Cleanup the editcopy other tool objects (e.g. decorations)
    virtual void CleanupToolObjects();

    a2dCanvasObject* FindTaggedObject(); 

    //! set to the a2dSnapToWhat features enabled by the engine for the source object
    a2dSnapToWhatMask m_snapSourceFeatures;

    //! set to the a2dSnapToWhat target features enabled by the engine and/or snap source object
    a2dSnapToWhatMask m_snapTargetFeatures;

    //! anotation string, which is in general modified by the tools to display position etc.
    wxString m_anotation;

    //! x position of anotation text
    int m_xanotation;

    //! y position of anotation text
    int m_yanotation;

    int m_prevxanotation;
    int m_prevyanotation;

    //! x of mouse in device
    int m_x;
    //! y of mouse in device
    int m_y;

    //! previous x of mouse in device
    int m_xprev;

    //! previous y of mouse in device
    int m_yprev;

    //! x world coordinates old or new value of mouse
    double m_xwprev;

    //! y world coordinates old or new value of mouse
    double m_ywprev;

    //! x of mouse in device at start drag
    int m_dragstartx;
    //! y of mouse in device at start drag
    int m_dragstarty;

    //! used to indicate that a first drag event has arrived.
    bool m_dragStarted;

    //! This is the object currently edited
    /*! This is usually a clone of m_original, if m_original exists.
        This object is directly (without commands) edited by the tool.
        The m_original is in this case edited indirectly via commands.
    */
    a2dCanvasObjectPtr m_canvasobject;

    //! This is the original object selected for editing
    /*! If there is an original and an editcopy, this is the original.
        Tools creating objects do not always have an original and an editcopy.
        This object only manipulated via commands
    */
    a2dCanvasObjectPtr m_original;

    //!controller for canvas
    a2dStToolContr* m_stcontroller;

    //! Use this connection generator for wires and pins
    a2dConnectionGenerator* m_connectionGenerator;

    //! if yes, the original object is rendered in place
    bool m_renderOriginal;
    //! if yes, the editcopy is rendered in place
    /*! this is not yet supported */
    bool m_renderEditcopy;
    //! if yes, the editcopy is rendered on top in usual style
    bool m_renderEditcopyOnTop;
    //! if yes, the editcopy is rendered on top in edit style
    bool m_renderEditcopyEdit;
    //! if yes, the editcopy is rendered on top as bounding box rectangle
    bool m_renderEditcopyRectangle;

    //! when drawing something editing must follow, using the a2dRecursiveEditTool
    bool m_editatend;

    //! when using cloned style for editcopy, use this opacity.
    wxUint8 m_editOpacityFactor;

    //! when true editcopies are using a half transparent cloned style.
    a2dOpaqueMode m_useEditOpaque;

    friend class a2dStToolEvtHandler;
    friend class a2dStToolFixedToolStyleEvtHandler;

public:

    DECLARE_CLASS( a2dStTool )
    DECLARE_EVENT_TABLE()
};

//! base for tools that draw and are stack based.
class A2DCANVASDLLEXP a2dStDrawTool: public a2dStTool
{
public:
    //!\param controller Pointer to a a2dStToolContr controller.
    a2dStDrawTool( a2dStToolContr* controller );

    //!destructor
    virtual ~a2dStDrawTool();

    //! template adepts to style change from outside or not
    void SetAllowModifyTemplate( bool allowModifyTemplate ) { m_AllowModifyTemplate = allowModifyTemplate; }

    //! template adepts to style change from outside or not
    bool GetAllowModifyTemplate() { return m_AllowModifyTemplate; }


protected:

    void StartEditingOrFinish();
    bool EnterBusyMode();
    virtual void FinishBusyMode( bool closeCommandGroup = true );
    void AbortBusyMode();

    //!called on key down events
    void OnKeyDown( wxKeyEvent& event );
    //!called on key up events
    void OnKeyUp( wxKeyEvent& event );

    //!called on key events
    void OnChar( wxKeyEvent& event );

    void OnComEvent( a2dComEvent& event );

    void OnCanUndoEvent( a2dCommandProcessorEvent& event );
    void OnCanRedoEvent( a2dCommandProcessorEvent& event );

    //!Create the editcopy and other tool objects (e.g. decorations)
    /*! m_original must be set before this function is called */
    virtual bool CreateToolObjects();

    void AdjustRenderOptions();

    bool m_AllowModifyTemplate;

public:

    DECLARE_CLASS( a2dStDrawTool )
    DECLARE_EVENT_TABLE()
};

//!The a2dStToolContr is a Tool Controller specialized for working with a2dDrawingPart.
/*!
It maintains tools that allows zooming while drawing is in progress.
It has a stack of tools, some tools remain on the stack, while other or pushed and poped from it.
A tool to show the cursor is a tool that is always on the stack and active, a tool to draw something
is only on the stack as long the user wants. Zooming while drawing something is done by temporarily pushing a zoomtool to the stack.
When the zoom as done, the tools is removed again.
You can develop new tools by deriving new tools from a2dStTool.
\sa a2dToolContr
\sa a2dStTool
\sa a2dDrawingPart
\sa a2dCanvas

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dStToolContr: public a2dToolContr
{
public:

    //! Constructor
    /*!
        \param drawingPart a2dDrawingPart where the controller takes events from
        \param where wxFrame to display statusbar text in.
        \param noStatusEvent if true use, m_where to set statusbar text in first field.
        Else a a2dComEvent sm_showCursor is send to the m_where.

    */
    a2dStToolContr( a2dDrawingPart* drawingPart, wxFrame* where, bool noStatusEvent = true );

    //!destructor
    ~a2dStToolContr();

    //! sets the m_toptool (if set ) as first tool.
    virtual void ReStart();

    //! to get the current mouse menu of the controller
    /*!
        /return NULL if no menu is set, else the menu.
    */
    wxMenu* GetMousePopupMenu() { return m_mousemenu; }

    //! to set the current mouse menu of the controller
    /*!
         NULL sets the menu to non.
    */
    void SetMousePopupMenu( wxMenu* mousemenu );

    //!specialize to keep first tool on the stack active
    /*!
        \param tool The tools to push, also checks if it is a a2dStTool derived tools, else an assert.
    */
    bool PushTool( a2dBaseTool* tool );

    //!specialize to keep m_topTool tool active
    /*!
        It the new first tool was already stopped for some reason, this tool will also be poped,
        this continues recursive until a tool is found which does not want to be stopped.
    */
    virtual bool PopTool( a2dSmrtPtr<a2dBaseTool>& poped, bool force = true );

    //! if true a zoomout is centered to the view.
    void SetZoomoutCentered( bool center ) { m_zoomoutCentered = center;}

    //! see SetZoomoutCentered()
    bool GetZoomoutCentered() { return m_zoomoutCentered; }

    //!first tool on the tool stack is the zoom tool.
    /*!
        This is the same as SetTopTool( new a2dZoomTool( MyController ) )
    */
    void SetZoomFirst( bool zoomfirst );

    //! when all tools are poped, this tool will be pushed.
    /*!
        The effect is that this tool always stays on top of the stack.
        This call also makes this the top tool right now.

        \param toolOnTop pointer to a tool you want to habve on top, NULL if non.
    */
    void SetTopTool( a2dBaseTool* toolOnTop );

    //! for zooming purposes this tool wil be used
    /*!
        This only sets for future use.

        \param toolForZoom pointer to a tool you want to use for zooming
    */
    void SetZoomTool( a2dBaseTool* toolForZoom );

    //!drag mode used in drag tool (if used)
    /*!
        Choose one of the three different drag methods see DRAGMODE
    */
    void SetDragMode( DRAGMODE mode ) { m_dragmode = mode; }

    //!Returns drag mode
    /*! \see SetDragMode */
    DRAGMODE GetDragMode() { return m_dragmode; }

    void SetDrawMode( a2dDrawStyle drawstyle ) { m_drawmode = drawstyle; }

    a2dDrawStyle GetDrawMode() { return m_drawmode; }

    //!Control the default behavior
    /*!   \see wxTC_BEHAVIOR  */
    void SetDefaultBehavior( unsigned int behavior ) { m_defaultBehavior = behavior; }

    unsigned int GetDefaultBehavior() { return m_defaultBehavior; }

    //! redirect to command processor of document
    void Undo( wxCommandEvent& );

    //! redirect to command processor of document
    void Redo( wxCommandEvent& );

    //!End the current active tool if not busy
    void EndTool( wxCommandEvent& );

    //! default implementation for property editing
    void EditProperties( wxCommandEvent& );

    //! how to set mouse menu undo/redo
    void OnSetmenuStrings( a2dCommandProcessorEvent& event );

    //!start one shot zoomtool from popup menu (used while another tool is active)
    void ZoomWindow();
    //!zoompan from popup menu (used while another tool is active)
    void ZoomPan();
    //!go to previous zoom area (used while another tool is active)
    void ZoomUndo();
    //!show all on canvas
    void Zoomout();
    //!zoom out two times the current visible area
    void Zoomout2();
    //!zoom in two half the current visible area
    void Zoomin2();

    void Zoom( double x, double y, double uppx, double uppy );

    //!zoom out two times the current visible area at mouse position
    void ZoomOut2AtMouse();

    //!zoom in two half the current visible area at mouse position
    void ZoomIn2AtMouse();

    //!Get zoom stack list, that is use to store zooming areas
    wxZoomList& GetZoomList() {return m_zoomstack;};

    //! set the format for double to be this
    void SetFormat( const wxString& format ) { m_format = format; }

	wxString GetFormat() { return m_format; }

	//! set all state string to empty.
	void ClearStateStrings();

    //! function called at mouse events, the default displays cursor in statusbar.
    /*! Override in a derived class to change behaviour. */
    virtual void SetStateString( const wxString& message = wxEmptyString, size_t field = 3 );

    //! return mouse position in device coordinates
    wxPoint GetMousePosition() { return wxPoint( m_mouse_x, m_mouse_y ); }

    //! what frame has the statusbar
    wxFrame* GetFrame() { return m_where; }

    //! Get the array containing statusbar strings
    std::vector<wxString>& GetStatusStrings() { return m_statusStrings; }

    //! start editing this object
    virtual bool StartEditingObject( a2dCanvasObject* objectToEdit );

    //! start editing this object
    virtual bool StartEditingObject( a2dCanvasObject* objectToEdit, a2dIterC& ic );

    //! when edit tool is firsttool, restart it.
    bool TriggerReStartEdit( wxUint16 editmode );

    //! rotate object of first tool on the stack, when appropriate.
    bool RotateObject90LeftRight( bool Right );

    //! when enabling m_useEditOpaque, this is how transparent the editcopy will be made.
    /*!
        \param editOpacity Real opacity is set opacity * m_OpacityFactor/255
    */
    void SetOpacityFactorEditcopy( wxUint8 editOpacity ) { m_editOpacityFactor = editOpacity; }

    //! see SetOpacityFactorEditcopy()
    wxUint8 GetOpacityFactorEditcopy() { return m_editOpacityFactor; }

    //! When set true, instead of using a special style for editcopies, a clone of the
    /*!
        fill and stroke of the original are made, and used for the editcopy, but its transparancy is
        set using m_editOpacityFactor.
        The effect is that when dragging such an object, that in the editcopy mode, a half transparent object is shown.
    */
    void SetUseOpaqueEditcopy( bool editOpaque ) { m_useEditOpaque = editOpaque; }

    //! see SetUseOpaqueEditcopy()
    bool GetUseOpaqueEditcopy() { return m_useEditOpaque; }

    void SetDraggingCanvasOption( bool draggingCanvasOption ) { m_draggingCanvasOption = draggingCanvasOption; }

    bool GetDraggingCanvasOption() { return m_draggingCanvasOption; }

	//! set key to disable snapping in tools
	void SetNoSnapKey( int keyCode ) { m_noSnapkey = keyCode; }
	
	//! what is the key to disable snapping in tools
	int GetNoSnapKey() { return m_noSnapkey; }

    //! set if selection state of tools object is set during undo.
    bool GetSelectionStateUndo() const { return m_selectionStateUndo; }

    void SetSelectionStateUndo( bool selectionStateUndo ) { m_selectionStateUndo = selectionStateUndo; }

    //! set tool object as selected at end of action
    bool GetSelectAtEnd() const { return m_selectedAtEnd; }

    void SetSelectAtEnd( bool selectedAtEnd ) { m_selectedAtEnd = selectedAtEnd; }

protected:

    //!start one shot zoomtool from popup menu (used while another tool is active)
    void ZoomWindowMenu( wxCommandEvent& );
    //!zoompan from popup menu (used while another tool is active)
    void ZoomPanMenu( wxCommandEvent& );
    //!go to previous zoom area (used while another tool is active)
    void ZoomUndoMenu( wxCommandEvent& );
    //!show all on canvas
    void ZoomoutMenu( wxCommandEvent& );
    //!zoom out two times the current visible area
    void Zoomout2Menu( wxCommandEvent& );
    //!zoom in two half the current visible area
    void Zoomin2Menu( wxCommandEvent& );

    //! handler for this event
    void OnComEvent( a2dComEvent& event );

    //!called on mouse events
    void OnMouseEvent( wxMouseEvent& event );
    //!called on key events
    void OnChar( wxKeyEvent& event );
    //!called on keydown events
    void OnKeyDown( wxKeyEvent& event );
    //!called on keyup events
    void OnKeyUp( wxKeyEvent& event );

    //! if set no event is send to m_where, instead a statusbar text is set directly
    bool m_noStatusEvent;

    //! statusbar frame
    wxFrame* m_where;

    std::vector<wxString> m_statusStrings;

    wxString m_format;

    //! zoom stack containing previous zooms
    wxZoomList m_zoomstack;

    //!popup menu
    wxMenu* m_mousemenu;

    //! mouse position
    int m_mouse_x;

    //! mouse position
    int m_mouse_y;

    //! previous x of mouse in device
    int m_mouse_xprev;

    //! previous y of mouse in device
    int m_mouse_yprev;

    //! used to indicate that dragging canvas is in action.
    bool m_draggingCanvas;

    //! can canvas be dragged using RightMouse.
    bool m_draggingCanvasOption;

    //! x of mouse in device at start drag
    int m_dragstartx;
    //! y of mouse in device at start drag
    int m_dragstarty;

    //! when true zoom out and center to middle of the view
    bool m_zoomoutCentered;

    //! manner to drag
    DRAGMODE m_dragmode;

    //! manner to draw
    a2dDrawStyle m_drawmode;

    //! control the default behavior
    unsigned int m_defaultBehavior;

    //! tool always on top of the stack, or non if NULL
    a2dSmrtPtr< a2dBaseTool > m_topTool;

    //! tool used for zooming
    a2dSmrtPtr< a2dBaseTool > m_toolForZoom;

    //! when using cloned style for editcopy, use this opacity.
    wxUint8 m_editOpacityFactor;

    //! when true editcopies are using a half transparent cloned style.
    bool m_useEditOpaque;

	int m_noSnapkey;

    bool m_selectionStateUndo;

    bool m_selectedAtEnd;

public:
    static const a2dPropertyIdBool PROPID_zoomfirst;
    //! send when tool was poped
    //! \ingroup events
    static const wxEventType sm_showCursor;

    DECLARE_CLASS( a2dStToolContr )
    DECLARE_EVENT_TABLE()
};

//!Interactive drawing of a Rectangle.
/*!
\remark  Left Click and drag.
\remark  Cursor keys to shift object.
\remark  Or Spacebar to end rectangle.

\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDrawRectangleTool: public a2dStDrawTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawRectangle;

    //! constructor
    /*!
        \param controller to which controller will it was pushed.
        \param templateObject to clone to use as start for new rectangles if not NULL
    */
    a2dDrawRectangleTool( a2dStToolContr* controller, a2dRect* templateObject = NULL );
    ~a2dDrawRectangleTool();
    bool ZoomSave() { return true; }

    void SetRadius( double radius )         { m_radius = radius; }

protected:

    virtual void GenerateAnotation();

    void OnMouseEvent( wxMouseEvent& event );

    virtual wxString GetCommandGroupName() { return _( "Draw Rectangle" ); }

    double       m_radius;

    //! rectangle object to clone to use as start
    a2dSmrtPtr<a2dRect> m_templateObject;

public:
    DECLARE_CLASS( a2dDrawRectangleTool )
    DECLARE_EVENT_TABLE()

};

//!Interactive Zooming.
/*! Use mouse to drag a rectangle to zoom into area.

\remark  Left Click and/or drag.
\remark  Right Click for zoom popup.
\remark  Double Left Click for zoom out.

\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dZoomTool: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_Zoom;

    //! constructor
    a2dZoomTool( a2dStToolContr* controller );

    //! constructor
    a2dZoomTool( const a2dZoomTool& other, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dZoomTool();

    bool ZoomSave() {return true;};

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    virtual void GenerateAnotation();

    //! character handling
    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual wxString GetCommandGroupName() { return _( "Zoom objects" ); }

public:

    DECLARE_CLASS( a2dZoomTool )
    DECLARE_EVENT_TABLE()

};

//!Interactive Selection of an Object.
/*!Either with just one click or draging a rectangle to select many.

\remark  Left Click and/or drag.
\remark  Shift Down to Un select.

\sa  a2dStToolContr

*/
class A2DCANVASDLLEXP a2dSelectTool: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_Select;

    a2dSelectTool( a2dStToolContr* controller );
    ~a2dSelectTool();
    bool ZoomSave() { return true;};
    void SetShiftIsAdd( bool shift_is_add = true ) { m_shift_is_add = shift_is_add; }

protected:
    void DeselectAll();

    virtual void GenerateAnotation();

    //!called on key down events
    void OnKeyDown( wxKeyEvent& event );
    //!called on key up events
    void OnKeyUp( wxKeyEvent& event );

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual bool EnterBusyMode();

    virtual bool CreateToolObjects();

    wxString GetCommandGroupName() { return  _( "Select object(s)" ) ; }

public:

    DECLARE_CLASS( a2dSelectTool )
    DECLARE_EVENT_TABLE()

protected:
    //! false: shift key will unselect, true: shift will add to selection
    bool m_shift_is_add;

    bool m_storeUndo;
};

//!Interactive Insertion of an a2dImage Object.
/*!Either with just one click or draging a rectangle to fit the image in.

\remark  Left Click and/or drag.

\sa  a2dStToolContr


  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dImageTool: public a2dStDrawTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_Image;
    static const a2dCommandId COMID_PushTool_Image_Embedded;

    a2dImageTool( a2dStToolContr* controller );

    ~a2dImageTool();

    bool ZoomSave() { return true;};

    //! Sets if the pattern (a rectangle) will be drawn on top of this image
    /*!
        If set to <code>true</code> the pattern (a rect) will be drawn on top
        of the image, else the pattern will be drawn behind the image.

        \remark By default the rectangle / pattern will be drawn behind the image

        \param  drawPatternOnTop Set <code>true</code> to draw on top of the image, else <code>false</code>
    */
    void SetDrawPatternOnTop( bool drawPatternOnTop ) { m_drawPatternOnTop = drawPatternOnTop; }

    //! Returns if the pattern will be drawn on top of the image
    /*!
        \see SetDrawPatternOnTop

        \return <code>true</code> if pattern will be drawn on top of this image, else <code>false</code> (default)
    */
    bool GetDrawPatternOnTop() { return m_drawPatternOnTop; }

    //! if set, image will be embedded in document, else a link to the original file
    /*!
    */
    void SetEmbed( bool imbed ) { m_imageEmbedded = imbed; }

    //! if set, image will be embedded in document, else a link to the original file
    /*!
    */
    bool GetEmbed() const { return m_imageEmbedded; }

protected:

    virtual void GenerateAnotation();

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    bool m_drawPatternOnTop;

    bool m_imageEmbedded;

public:

    DECLARE_CLASS( a2dImageTool )
    DECLARE_EVENT_TABLE()

};

//!Interactive drag an object.
/*!Uses the dragmode set for the controller.

\remark  Left Click and drag.
\remark  Key left right up down to move while draging active (Left Down)

\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDragTool: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_Drag;

    //! constructor
    a2dDragTool( a2dStToolContr* controller, a2dCanvasObject* dragobject = NULL, double xw = 0, double yw = 0 );
    //! destructor
    ~a2dDragTool();

    bool StartDragging( int x, int y, a2dCanvasObject* original );

    bool ZoomSave() { return !GetBusy(); }

    //!If set to true, the dragged object is deleted if dropped outside of the window
    void SetDeleteOnOutsideDrop( bool val ) { m_deleteonoutsidedrop = val; }

    void SetDropAndDrop( bool val ) { m_dropAndDrop = val; }

	//! defines if an object will try to connect at the end of a drag
	void SetLateConnect( bool lateconnect ) { m_lateconnect = lateconnect; }

    //!redirect all mouse events for the canvas to this object
    void CaptureMouse();
    //!release the mouse capture for this object
    void ReleaseMouse();

    void Render();

    //!This is overriden by copy tools to have a minimum distance between original and copy
    virtual void AdjustShift( double* x, double* y );

protected:

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual bool EnterBusyMode();
    virtual void FinishBusyMode( bool closeCommandGroup = true );
    virtual void AbortBusyMode();

    //!called when dragging is in progress
    virtual void DragAbsolute( double x, double y );

    virtual wxString GetCommandGroupName() { return  _( "Move object" ) ; }
    virtual bool CreateToolObjects();
    virtual void CleanupToolObjects();

    //! old transform
    a2dAffineMatrix m_transform;

    //! allow connecting to other obejcts
    bool m_wasMayConnect;

    //! connect when released
    bool m_lateconnect;

    //! is mouse captured during drag
    bool m_mousecaptured;

    //! if true, the dragged object is deleted, if it is dropped outside of the window
    bool m_deleteonoutsidedrop;

    // DnD if implemented.
    bool m_dropAndDrop;

    //! hold start of drag
    double m_startObjectx;
    //! hold start of drag
    double m_startObjecty;
    //! hold start of drag
    double m_startMousex;
    //! hold start of drag
    double m_startMousey;

public:
    DECLARE_CLASS( a2dDragTool )
    DECLARE_EVENT_TABLE()

};

class A2DCANVASDLLEXP a2dDragOrgTool: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DragOrg;

    //! constructor
    a2dDragOrgTool( a2dStToolContr* controller );
    //! destructor
    ~a2dDragOrgTool();

    bool StartDragging( int x, int y, a2dCanvasObject* original );

    bool ZoomSave() { return !GetBusy(); }

    //!If set to true, the dragged object is deleted if dropped outside of the window
    void SetDeleteOnOutsideDrop( bool val ) { m_deleteonoutsidedrop = val; }

    //!redirect all mouse events for the canvas to this object
    void CaptureMouse();
    //!release the mouse capture for this object
    void ReleaseMouse();

    void Render();

    //!This is overriden by copy tools to have a minimum distance between original and copy
    virtual void AdjustShift( double* x, double* y );

protected:

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual bool EnterBusyMode();
    virtual void FinishBusyMode( bool closeCommandGroup = true );
    virtual void AbortBusyMode();

    //!called when dragging is in progress
    virtual void DragAbsolute( double x, double y );

    virtual wxString GetCommandGroupName() { return  _( "Move object" ) ; }
    virtual bool CreateToolObjects();
    virtual void CleanupToolObjects();

    //! old transform
    a2dAffineMatrix m_transform;

    //! allow connecting to othere object when placed
    bool m_wasMayConnect;

    //! connect only when released
    bool m_lateconnect;

    //! is mosue captured when dragging
    bool m_mousecaptured;

    //! if true, the dragged object is deleted, if it is dropped outside of the window
    bool m_deleteonoutsidedrop;

    //! hold start of drag
    double m_startObjectx;
    //! hold start of drag
    double m_startObjecty;
    //! hold start of drag
    double m_startMousex;
    //! hold start of drag
    double m_startMousey;

public:
    DECLARE_CLASS( a2dDragOrgTool )
    DECLARE_EVENT_TABLE()

};

DECLARE_MENU_ITEMID( CmdMenu_NoDragMenu )

//!Interactive drag a new object into a drawing.
/*!This is like a2dDragTool but it goes automatically into drag mode
   if it receives a mouse event.

\remark  Left Click to drop.
\remark  Key left right up down to move while draging active (Left Down)

\sa  a2dDragTool

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDragNewTool: public a2dDragTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DragNew;

    /*!
        \param controller controller to place tool in
        \param newObject the canvas object that is drag & droped
    */
    a2dDragNewTool( a2dStToolContr* controller, a2dCanvasObject* newObject, const a2dMenuIdItem& initiatingMenuId = CmdMenu_NoDragMenu() );
    ~a2dDragNewTool();

    void SetBoxCenterDrag( bool boxCenterDrag ) { m_boxCenterDrag = boxCenterDrag; }
    bool GetBoxCenterDrag() { return m_boxCenterDrag; }

    a2dCanvasObject* GetTemplateObject() const { return m_newObject; }

    bool ZoomSave() { return true; }

    bool RotateObject90LeftRight( bool right );

    void OnMouseEvent( wxMouseEvent& event );

    virtual void AdjustRenderOptions();

protected:

    void OnComEvent( a2dComEvent& event );

    //! when dragging an object from another window into the tool its window, this window wants to have the focus.
    void OnEnter( wxMouseEvent& WXUNUSED( event ) );

    void OnChar( wxKeyEvent& event );

    virtual wxString GetCommandGroupName() { return  _( "Create new object" ) ; }
    virtual void DoStopTool( bool abort );
    virtual bool EnterBusyMode();
    virtual void FinishBusyMode( bool closeCommandGroup = true );

    a2dCanvasObjectPtr m_newObject;
    //! This is the parent, to which the temporary object was added.
    /*! This must be remembered, because it changes if the shows object changes */
    a2dCanvasObjectPtr m_parentAddedTo;

    //! drag from origin of object or center of bbox
    bool m_boxCenterDrag; 

public:
    DECLARE_CLASS( a2dDragNewTool )
    DECLARE_EVENT_TABLE()
};

//!Interactive copy an object.
/*!
\remark  Left Click and drag.

\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dCopyTool: public a2dDragTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_Copy;

    a2dCopyTool( a2dStToolContr* controller, a2dCanvasObject* copyobject = NULL, double xw = 0, double yw = 0 );
    bool ZoomSave() { return !GetBusy(); };

    void SetDeepCopy( bool deepCopy ) { m_deepCopy = deepCopy ; }

    bool GetDeepCopy() { return m_deepCopy; }

protected:
    void OnMouseEvent( wxMouseEvent& event );

    virtual bool CreateToolObjects();
    bool EnterBusyMode();
    void FinishBusyMode( bool closeCommandGroup = true );

    virtual void AdjustShift( double* x, double* y );

    // minimum distance of the copy from the original
    double m_mindist;
    bool m_deepCopy;

    virtual wxString GetCommandGroupName() { return _( "Copy" ); }

public:
    DECLARE_CLASS( a2dCopyTool )
    DECLARE_EVENT_TABLE()
};

//!Interactive rotate an object.
/*!
\remark  Left Click and drag.

\sa  a2dStToolContr
*/
class A2DCANVASDLLEXP a2dRotateTool: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_Rotate;

    a2dRotateTool( a2dStToolContr* controller );
    ~a2dRotateTool();
    bool ZoomSave() { return !GetBusy(); };


    //!redirect all mouse events for the canvas to this object
    void CaptureMouse();
    //!release the mouse capture for this object
    void ReleaseMouse();

protected:

    bool EnterBusyMode();
    virtual void FinishBusyMode( bool closeCommandGroup = true );
    void AbortBusyMode();

    void OnChar( wxKeyEvent& event );

    void OnIdle( wxIdleEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual wxString GetCommandGroupName() { return  _( "Rotate object" ) ; }

    double m_xr;
    double m_yr;

    bool m_mode;

    //! old transform
    a2dAffineMatrix m_transform;

    double m_ang;

public:
    DECLARE_CLASS( a2dRotateTool )
    DECLARE_EVENT_TABLE()

};


//!Interactive delete an object.
/*!
\remark  Left Click on object to delete.

\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDeleteTool: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_Delete;

    a2dDeleteTool( a2dStToolContr* controller, a2dCanvasObjectFlagsMask whichobjects = a2dCanvasOFlags::ALL );
    bool ZoomSave() { return true;};
    void SetWhichObjectsFlagMask( a2dCanvasObjectFlagsMask whichobjects ) { m_whichobjects = whichobjects; }

protected:

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual wxString GetCommandGroupName() { return  _( "Delete object" ) ; }

public:
    DECLARE_CLASS( a2dDeleteTool )
    DECLARE_EVENT_TABLE()

protected:
    a2dCanvasObjectFlagsMask m_whichobjects;

};


//!Interactive drawing of a Circle.
/*!
\remark  Left Click and drag.
\remark  Cursor keys to shift object.
\remark  Or Spacebar to end line.

\sa  a2dStToolContr
*/
class A2DCANVASDLLEXP a2dDrawCircleTool: public a2dStDrawTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawCircle;

    a2dDrawCircleTool( a2dStToolContr* controller );
    bool ZoomSave() { return true;};

    void SetMode( int mode );

protected:

    void OnIdle( wxIdleEvent& event );

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual void GenerateAnotation();

    double       m_start_x;
    double       m_start_y;
    double       m_radius;

public:
    DECLARE_CLASS( a2dDrawCircleTool )
    DECLARE_EVENT_TABLE()

};

//!Interactive drawing of a EllipticArc.
/*!
\remark  Left Click for first point and second point again.
\remark  Cursor keys to shift object.
\remark  Or Spacebar to end line.

\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDrawLineTool: public a2dStDrawTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawLine;
    static const a2dCommandId COMID_PushTool_DrawLineScaledArrow;

    a2dDrawLineTool( a2dStToolContr* controller, a2dSLine* templateObject = NULL );

    ~a2dDrawLineTool();

    bool ZoomSave() { return true;};

    //!defines line begin object for line and polyline drawing tools
    void SetLineBegin( a2dCanvasObject* begin );

    //!defines line end object for line and polyline drawing tools
    void SetLineEnd( a2dCanvasObject* end );

    //!get current line begin object for line and polyline drawing tools.
    /*!\return Null if not set. */
    a2dCanvasObject* GetLineBegin() { return m_templateObject->GetBegin(); }

    //!get current line end object for line and polyline drawing tools
    /*!return: Null if not set. */
    a2dCanvasObject* GetLineEnd() { return m_templateObject->GetEnd(); }

    //!scaling in X for begin and end objects of lines and polylines
    void SetEndScaleX( double xs );
    //!scaling in Y for begin and end objects of lines and polylines
    void SetEndScaleY( double ys );

    //!get scaling in X for begin and end objects of lines and polylines
    double GetEndScaleX() { return m_templateObject->GetEndScaleX(); }

    //!get scaling in Y for begin and end objects of lines and polylines
    double GetEndScaleY() { return m_templateObject->GetEndScaleY(); }

    //! Set when m_contourwidth != 0 what is the end of the line should be.
    void SetPathType( a2dPATH_END_TYPE pathtype );

    //! get when m_contourwidth != 0 what is the end of the line looks like.
    a2dPATH_END_TYPE GetPathType() { return m_templateObject->GetPathType(); }

    //! set template for line object
    a2dSLine* GetTemplateObject() { return m_templateObject; }

    //! get template for line object
    void SetTemplateObject( a2dSLine* templateobject ) { m_templateObject = templateobject; }

protected:

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    void OnDoEvent( a2dCommandProcessorEvent& event );

    virtual void GenerateAnotation();

    double       m_x1;
    double       m_y1;
    double       m_x2;
    double       m_y2;

    //! object to clone to use as start
    a2dSmrtPtr<a2dSLine> m_templateObject;

public:
    DECLARE_CLASS( a2dDrawLineTool )
    DECLARE_EVENT_TABLE()

};

//!Interactive drawing of a Ellipse.
/*!
\remark  Left Click and drag.
\remark  Cursor keys to shift object.
\remark  Or Spacebar to end line.

\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDrawEllipseTool: public a2dStDrawTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawEllipse;

    a2dDrawEllipseTool( a2dStToolContr* controller );
    bool ZoomSave() { return true;};

    void SetMode( int mode );

protected:

    void OnIdle( wxIdleEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    void OnChar( wxKeyEvent& event );

    virtual void GenerateAnotation();

    double       m_start_x;
    double       m_start_y;


public:
    DECLARE_CLASS( a2dDrawEllipseTool )
    DECLARE_EVENT_TABLE()

};

//!Interactive drawing of a EllipticArc.
/*!
\remark  Left Click and drag/move.
\remark  Left Click again Or Spacebar to end arc drawing.
\remark  Return/Enter to cycle through drawing mode2 for changing start and end angle.
\remark  Cursor keys to shift object.
\remark  Shift drag/move to change start angle.

\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDrawEllipticArcTool: public a2dStDrawTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawEllipticArc;
    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawEllipticArc_Chord;

    a2dDrawEllipticArcTool( a2dStToolContr* controller, a2dEllipticArc* templateObject = NULL );
    bool ZoomSave() { return true;};

    void SetMode( int mode );

    //! change mode of drawing
    void SetMode2( int mode );

    void SetChord( bool chord ) { m_chord = chord; };

    bool GetChord() const { return  m_chord; }

protected:

    void OnIdle( wxIdleEvent& event );

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual void GenerateAnotation();

    //! object to clone to use as start
    a2dSmrtPtr<a2dEllipticArc> m_templateobject;

    //! start arc
    double        m_arcstart;
    //! end arc
    double        m_arcend;

    //! start x
    double       m_start_x;
    //! start y
    double       m_start_y;

    //! draw just a cord object, not filled.
    bool m_chord;

    //! mode to change what is drawn
    int m_mode2;

public:
    DECLARE_CLASS( a2dDrawEllipticArcTool )
    DECLARE_EVENT_TABLE()

};

//!Interactive drawing of a circular arc.
/*!
\remark  Left Click and drag/move.
\remark  Left Click again Or Spacebar to end arc drawing.
\remark  Return/Enter to cycle through drawing mode2 for changing start and end angle.
\remark  Cursor keys to shift object.
\remark  Shift drag/move to change start angle.

\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDrawArcTool: public a2dStDrawTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawArc;
    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawArc_Chord;

    a2dDrawArcTool( a2dStToolContr* controller );
    bool ZoomSave() { return true;};

    void SetMode( int mode );

    //! change mode of drawing
    void SetMode2( int mode );

    //! draw jus a cord object
    void SetChord( bool chord ) { m_chord = chord; };

    bool GetChord() const { return  m_chord; }

protected:

    void OnIdle( wxIdleEvent& event );

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual void GenerateAnotation();

    virtual wxString GetCommandGroupName() { return  _( "Draw Arc" ) ; }

    //! way of drawing
    int m_mode2;


    //! start arc
    double        m_arcstart;
    //! end arc
    double        m_arcend;

    //! start x
    double       m_start_x;
    //! start y
    double       m_start_y;

    //! draw just a cord object, not filled.
    bool m_chord;

public:
    DECLARE_CLASS( a2dDrawArcTool )
    DECLARE_EVENT_TABLE()

};


//!Interactive drawing of a polygon.
/*!
\remark  Left Click to add points.
\remark  Space bar or Double click to end drawing.
\remark  Cursor keys to shift object.
\remark  Cursor keys + control to shift point.
\remark  Right Click to zoom.

\sa  a2dStToolContr

    \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDrawPolygonLTool: public a2dStDrawTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawPolygonL;
    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawPolygonL_Splined;

    //! constructor
    /*!
        \param controller tool controller one which this tool is pushed.
        \param templateObject when not NULL, this object is cloned and used for the new drawn object.
    */
    a2dDrawPolygonLTool( a2dStToolContr* controller, a2dPolygonL* templateObject = NULL );

    ~a2dDrawPolygonLTool();

    //! save to zoom while drawing
    bool ZoomSave() { return true;};

    //! mode of drawing, can be swicthed with Tab key.
    void SetMode( int mode );

    //!certain drawing tools like polygon drawing use this setting
    /*! You might want to reset m_newObjectsGetCommandProcStyle also.
        Otherwise the command processor will overwrite what you set here
    */
    void SetSpline( bool spline );

    //!certain drawing tools like polygon drawing use this setting
    bool GetSpline() {return m_templateObject->GetSpline();}

protected:

    void OnIdle( wxIdleEvent& event );

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

	void OnDoEvent( a2dCommandProcessorEvent& event );
    void OnUndoEvent( a2dCommandProcessorEvent& event );
    void OnRedoEvent( a2dCommandProcessorEvent& event );
    void OnCanUndoEvent( a2dCommandProcessorEvent& event );
    void OnCanRedoEvent( a2dCommandProcessorEvent& event );

    void AddPoint( a2dLineSegment* point );
	void BeginPolygon( double x, double y );
	void AddPoint( double x, double y );


    virtual wxString GetCommandGroupName() { return  _( "Draw polygon" ) ; }

    //! object to clone to use as start
    a2dSmrtPtr<a2dPolygonL> m_templateObject;

    //! previous x
    double m_prev_x;
    //! previous y
    double m_prev_y;

    //! points sofar
    a2dVertexList* m_points;

    //! last point
    a2dLineSegment* m_point;

    //! end to begin wire.
    bool m_reverse;

public:
    DECLARE_CLASS( a2dDrawPolygonLTool )
    DECLARE_EVENT_TABLE()

};

//!Interactive drawing of a polyline.
/*!
\remark  Left Click to add points.
\remark  Space bar or Double click to end drawing.
\remark  Cursor keys to shift object.
\remark  Cursor keys + control to shift point.
\remark  Right Click to zoom.

\sa  a2dStToolContr

    \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDrawPolylineLTool: public a2dDrawPolygonLTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawPolylineL;
    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawPolylineL_Splined;

    //! constructor
    /*!
        \param controller tool controller one which this tool is pushed.
        \param templateObject when not NULL, this object is cloned and used for the new drawn object.
    */
    a2dDrawPolylineLTool( a2dStToolContr* controller, a2dPolylineL* templateObject = NULL );

    ~a2dDrawPolylineLTool();

    //!defines line begin object for line and polyline drawing tools
    /*! You might want to reset m_newObjectsGetCommandProcStyle also.
        Otherwise the command processor will overwrite what you set here
    */
    void SetLineBegin( a2dCanvasObject* begin );

    //!defines line end object for line and polyline drawing tools
    /*! You might want to reset m_newObjectsGetCommandProcStyle also.
        Otherwise the command processor will overwrite what you set here
    */
    void SetLineEnd( a2dCanvasObject* end );

    //!get current line begin object for line and polyline drawing tools.
    /*!\return Null if not set. */
    a2dCanvasObject* GetLineBegin() { return m_templatePline->GetBegin(); }

    //!get current line end object for line and polyline drawing tools
    /*!return: Null if not set. */
    a2dCanvasObject* GetLineEnd() { return m_templatePline->GetEnd(); }

    //!scaling in X for begin and end objects of lines and polylines
    void SetEndScaleX( double xs );
    //!scaling in Y for begin and end objects of lines and polylines
    void SetEndScaleY( double ys );

    //!get scaling in X for begin and end objects of lines and polylines
    double GetEndScaleX() { return m_templatePline->GetEndScaleX(); }

    //!get scaling in Y for begin and end objects of lines and polylines
    double GetEndScaleY() { return m_templatePline->GetEndScaleY(); }

    //! Set when m_contourwidth != 0 what is the end of the line should be.
    void SetPathType( a2dPATH_END_TYPE pathtype );

    //! get when m_contourwidth != 0 what is the end of the line looks like.
    a2dPATH_END_TYPE GetPathType() { return m_templatePline->GetPathType(); }

protected:

    void OnMouseEvent( wxMouseEvent& event );

    void OnDoEvent( a2dCommandProcessorEvent& event );

    void OnComEvent( a2dComEvent& event );

    a2dSmrtPtr<a2dPolylineL> m_templatePline;

public:
    DECLARE_CLASS( a2dDrawPolylineLTool )
    DECLARE_EVENT_TABLE()

};

//! the way a new wire is created
enum a2dWiringMode
{
    //! the pin which is clicked as start pin for the wire, its pinclass is used to
    //! find a connecting pinclass in the wire beginpin. Only usefull if there is only
    //! one type of wire possible for that pinclass.
    a2d_BasedOnClassStartPin,
    //! the tool provides a starting wire pinclass, only pins which can connect to his wirepin its pinclass,
    //! will be clickable. Use this to only draw specific wires from this tool.
    a2d_BasedOnWireClassRequired,
    a2d_BasedOnObjectClassRequired,
    //! the wire will be decided when the end pin for the wire is hit, based on the start pin and endpin.
    //! use this if more then on type of wire can start from a certain pinclass, and it depends on the end pin
    //! what type of wire it will be.
    a2d_BasedOnClassEndPin,
	a2d_StartGenerateSearchFinish
};


//!Interactive drawing of a polyline wire.
/*!
    This tool is used to draw wires in between a2dCanvasObject's which have a2dPin's.
    a2dPin's are used to connect objects with eachother. A wire is a special kind of a2dCanvasObject
    since it is designed to keep other objects connected when moving them around.
    Rerouting algorithms take care of this.

    This tool here first waits for a pin that is clicked. It uses a a2dConnectionGenerator::GetPinClassForTask()
    to get a connection a2dPinClass for the pin which was clicked.
    From that the pin clicked, a suitable wire is found via a2dConnectionGenerator::CreateConnectObject()
    The wire returned has a pin which matches the pin clicked at the start, and an end pin is generated
    by a2dConnectionGenerator itself. The end pin must eventually fits a pin which will be clicked to end the wire.

    If you want this tool to start only on specific pins, for a specific wire type and/or pin class,
    you must set a a2dConnectionGenerator that does only returns that type of pin in its GetPinClassForTask().
    When more start pin classes are allowed, and pins overlap the first one found is returned. That might
    be not be the one you want.

    For usage of those features, think here of graphs of a2dCanvasObjects with more then one flow.

    \remark  Left Click to add points.
    \remark  Space bar or Double click to end drawing.
    \remark  Cursor keys to shift object.
    \remark  Cursor keys + control to shift point.
    \remark  Right Click to zoom.

    \sa  a2dStToolContr

    \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDrawWirePolylineLTool: public a2dDrawPolylineLTool
{

public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawWirePolylineL;

    a2dDrawWirePolylineLTool( a2dStToolContr* controller );
    ~a2dDrawWirePolylineLTool();

	void SetWiringMode( a2dWiringMode wiringMode ) { m_wiringMode = wiringMode; }

    //! if set, end of wire splits connecting wire.
    void SetEndMode( bool splitAtEnd ) { m_splitAtEnd = splitAtEnd; }

    bool GetEndMode() { return m_splitAtEnd; }

    //! with this set to 1 or 2 the next point to add to the wire is a two segment piece.
    //! angle can be altered with the TAB key. Set to 0 is a straight line.
	void SetEndSegmentMode( a2dNextSeg mode );

	a2dNextSeg GetEndSegmentMode() { return m_endSegmentMode; }

public:

    //! Helper function to find a pin in a DIRECT child object of the given root object
    /*! if a pin is found, return the m_connectionGenerator is used to find the pin which can connect
        to the pin found.
    */
    a2dPin* FindPin( a2dCanvasObject* root, double x, double y );

protected:

	void AtNewVertex();

	void OnKeyDown( wxKeyEvent& event );
    void OnChar( wxKeyEvent& event );

    void OnUndoEvent( a2dCommandProcessorEvent& event );
    void OnRedoEvent( a2dCommandProcessorEvent& event );

    void OnCanUndoEvent( a2dCommandProcessorEvent& event );
    void OnCanRedoEvent( a2dCommandProcessorEvent& event );

	void OnIdle( wxIdleEvent& event );

    a2dWiringMode m_wiringMode;

    //! required pin class at start of wire
    a2dPinClass* m_pinClassStartWire;

    //! required pin class at start of wire
    a2dPinClass* m_pinClassEndWire;

    //! if set, end of wire splits connecting wire.
    bool m_splitAtEnd;

    virtual bool EnterBusyMode();
    virtual void FinishBusyMode( bool closeCommandGroup = true );
    virtual void AbortBusyMode();

    void AdjustRenderOptions();
    void OnMouseEvent( wxMouseEvent& event );
    void SetActive( bool active );

    virtual wxString GetCommandGroupName() { return  _( "Draw wire" ) ; }

    int m_startWireX;
    int m_startWireY;
    int m_startWireXdev;
    int m_startWireYdev;
    bool m_hadDoubleClick;

    //! manhattan point
    a2dLineSegment* m_pointmanhattan;

	a2dNextSeg m_endSegmentMode;
	
    bool m_manhattan;
    bool m_like;
	double m_manx, m_many;

public:
    DECLARE_CLASS( a2dDrawWirePolylineLTool )
    DECLARE_EVENT_TABLE()
};

//!Drag Selected canvasObjects
/*! \sa  a2dStToolContr
    \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDragMultiTool: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DragMulti;

    a2dDragMultiTool( a2dStToolContr* controller );
    ~a2dDragMultiTool();
    bool ZoomSave() { return !GetBusy(); };

    bool StartDragging( int x, int y, a2dCanvasObject* original );

	//! defines if an object will try to connect at the end of a drag
	void SetLateConnect( bool lateconnect ) { m_lateconnect = lateconnect; }

	void SetOnlyKeys( bool onlyKeys ) { m_onlyKeys = onlyKeys; }

protected:

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual bool EnterBusyMode();
    virtual void FinishBusyMode( bool closeCommandGroup = true );
    virtual void AbortBusyMode();

    //!redirect all mouse events for the canvas to this object
    void CaptureMouse();
    //!release the mouse capture for this object
    void ReleaseMouse();

    void DragAbsolute( double x, double y );

    //!render in drag style
    virtual void Render();
    //!This is overriden by copy tools to have a minimum distance between original and copy
    virtual void AdjustShift( double* x, double* y );

    virtual wxString GetCommandGroupName() { return  _( "Move multiple objects" ) ; }
    virtual bool CreateToolObjects();
    virtual void CleanupToolObjects();

    //!the list of selected objects
    a2dCanvasObjectList m_objects;

    a2dCanvasObjectList m_copies;

    //! hold start of drag
    double m_startObjectx;
    //! hold start of drag
    double m_startObjecty;
    //! hold start of drag
    double m_startMousexw;
    //! hold start of drag
    double m_startMouseyw;

    //! allow connecting to other obejcts
    bool m_lateconnect;

    bool m_onlyKeys;


public:
    DECLARE_CLASS( a2dDragMultiTool )
    DECLARE_EVENT_TABLE()

};

class A2DCANVASDLLEXP a2dDragMultiNewTool: public a2dDragMultiTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DragNew;

    /*!
        \param controller controller to place tool in
        \param originals the canvas objects that is drag & droped
    */
    a2dDragMultiNewTool( a2dStToolContr* controller, a2dCanvasObjectList* originals );
    ~a2dDragMultiNewTool();

    bool ZoomSave() { return true; }

    void OnMouseEvent( wxMouseEvent& event );

    virtual void AdjustRenderOptions();

    bool StartDragging( int x, int y );

    void SetBoxCenterDrag( bool boxCenterDrag ) { m_boxCenterDrag = boxCenterDrag; }
    bool GetBoxCenterDrag() { return m_boxCenterDrag; }

protected:

    void OnComEvent( a2dComEvent& event );

    //! when dragging an object from another window into the tool its window, this window wants to have the focus.
    void OnEnter( wxMouseEvent& WXUNUSED( event ) );

    virtual wxString GetCommandGroupName() { return  _( "Create new object" ) ; }
    virtual void DoStopTool( bool abort );
    virtual bool EnterBusyMode();
    virtual void AbortBusyMode();
    virtual void FinishBusyMode( bool closeCommandGroup = true );

    a2dCanvasObjectList m_originals;

    //! This is the parent, to which the temporary object was added.
    /*! This must be remembered, because it changes if the shows object changes */
    a2dCanvasObjectPtr m_parentAddedTo;

    //! drag from origin of object or center of bbox
    bool m_boxCenterDrag; 

public:
    DECLARE_CLASS( a2dDragMultiNewTool )
    DECLARE_EVENT_TABLE()
};


//!Drag and Copy Selected canvasObjects
/*!\sa  a2dStToolContr
    \ingroup  tools
*/
class A2DCANVASDLLEXP a2dCopyMultiTool: public a2dDragMultiTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_CopyMulti;

    a2dCopyMultiTool( a2dStToolContr* controller );

    bool StartDragging( int x, int y, a2dCanvasObject* original );

protected:

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    void AdjustShift( double* x, double* y );

    virtual wxString GetCommandGroupName() { return  _( "Copy multiple objects" ) ; }

    // minimum distance of the copy from the original
    double m_mindist;
public:
    DECLARE_CLASS( a2dCopyMultiTool )
    DECLARE_EVENT_TABLE()

};

//!Interactive drawing a text object.
/*!
\remark  Cursor keys to navigate through text
\remark  Home & End (goto begin or end of line)
\remark  control + Home & End (goto begin or end of text object)
\remark  altdown + Cursor keys to shift text object
\remark  controldown + Cursor keys (default canvas behaviour)

\sa  a2dStToolContr

    \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDrawTextTool: public a2dStDrawTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawText;

    //! constructor
    /*!
        \param controller tool controller one which this tool is pushed.
        \param templateObject when not NULL, this object is cloned and used for the new drawn object.
    */
    a2dDrawTextTool( a2dStToolContr* controller, a2dText* templateObject = NULL );
    ~a2dDrawTextTool();

    //! set template for line object
    a2dText* GetTemplateObject() const { return m_templateObject; }

    //! get template for line object
    void SetTemplateObject( a2dText* templateobject ) { m_templateObject = templateobject; }

    bool ZoomSave() { return true;};

    DECLARE_PROPERTIES()

protected:

    void OnMouseEvent( wxMouseEvent& event );

    //! object to clone to use as start
    a2dSmrtPtr<a2dText> m_templateObject;

public:
    DECLARE_CLASS( a2dDrawTextTool )
    DECLARE_EVENT_TABLE()

};


//!Interactive edit properties of an object.
/*!
\remark  Left Click on object edit properties of

\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dPropertyTool: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_Property;

    a2dPropertyTool( a2dStToolContr* controller );

    bool StartEditing( double x, double y );

    bool StartEditing( int x, int y );

    bool ZoomSave() { return true;};

protected:

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    //! id of property to set
    a2dPropertyId* m_id;

    //! undoable or not
    bool m_withUndo;

public:
    DECLARE_CLASS( a2dPropertyTool )
    DECLARE_EVENT_TABLE()

};


//! behaviour model of a2dStTool tools
/*!

    Tries to model general behaviour for tools controlled by a2dStToolContr, and tools known in wxArt2D.
    If you want different behaviour you can derive or develop your own a2dToolEvtHandler.

    A a2dToolEvtHandler is/can be set to a a2dBaseTool as an event handler in between the tool its own event handler.
    This is the easiest way to make a whole set of tools behave to a certain model.

    \ingroup  tools
*/
class A2DCANVASDLLEXP a2dStToolEvtHandler: public a2dToolEvtHandler
{
    DECLARE_EVENT_TABLE()

public:

    //! Enum for hit test options
    enum options
    {
        a2dTakeToolstyleNon = 0x0000,

        //! can be used to take over style from the a2dCentralCanvasCommandProcessor
        /*!
            when set fill stroke and other specific properties are set as current status to the
            a2dCentralCanvasCommandProcessor when a drawing tool is pushed.
        */
        a2dTakeToolstyleToCmdhOnPush = 0x0001,

        //! can be used to set style to the a2dCentralCanvasCommandProcessor
        /*!
            when set fill stroke contour and other specific properties are taken from the
            a2dCentralCanvasCommandProcessor when a drawing tool is poped.
        */
        a2dTakeToolstyleFromCmdhOnPop = 0x0002,

        //! can be used to set style to the a2dCentralCanvasCommandProcessor
        /*!
            when set fill stroke contour and other specific properties are taken from the
            a2dCentralCanvasCommandProcessor when a drawing tool is pushed.
        */
        a2dTakeToolstyleFromCmdhOnPush = 0x0004,

        //! new object added or object edited gets tool style
        a2dTakeToolstyleToNewObject = 0x0008,

        //! editing an object, its style becomes the central style.
        a2dTakeToolstyleToCentral = 0x0010,

        //! all
        a2d_ALL    = 0xFFFF
    };

    //! constructor
    a2dStToolEvtHandler( a2dStToolContr* controller );
    //! destructor
    ~a2dStToolEvtHandler();

    void SetTakeToolstyleToCmdhOptions( wxUint32 set ) { m_options = set; }
    wxUint32 GetTakeToolstyleToCmdhOptions() { return m_options; }

    void TakeToCmdhFromEditedObject( a2dObjectEditTool* editTool );

protected:

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void OnDoEvent( a2dCommandProcessorEvent& event );
    void OnUndoEvent( a2dCommandProcessorEvent& event );
    void OnRedoEvent( a2dCommandProcessorEvent& event );
    void OnComEvent( a2dComEvent& event );
    void OnIdle( wxIdleEvent& event );

    //! tools using this controller
    a2dStToolContr* m_stcontroller;

    wxUint32 m_options;
};


//! OR-ing a2dHitOption is allowed

inline a2dStToolEvtHandler::options operator | ( a2dStToolEvtHandler::options a, a2dStToolEvtHandler::options b )
{
    return ( a2dStToolEvtHandler::options ) ( ( int ) a | ( int ) b );
}


//! makes a tool fixed in style.
/*!
    Tools like a2dZoomTool or a2dDrawWirePolylineLTool do in general have a style which is not influenced by
    the interactive style change in the application.
*/
class A2DCANVASDLLEXP a2dStToolFixedToolStyleEvtHandler: public a2dToolEvtHandler
{
    DECLARE_EVENT_TABLE()

public:

    //! constructor
    a2dStToolFixedToolStyleEvtHandler( a2dStToolContr* controller );
    //! destructor
    ~a2dStToolFixedToolStyleEvtHandler();

    //! can be used to take over style from the a2dCentralCanvasCommandProcessor
    /*!
        when set fill stroke contour and other specific properties are set as current status to the
        a2dCentralCanvasCommandProcessor when a drawing tool is pushed.
    */
    void SetTakeToolstyleToCmdhOnPush( bool set ) { m_takeToolstyleToCmdhOnPush = set; }
    //! see SetTakeToolstyleToCmdhOnPush()
    bool GetTakeToolstyleToCmdhOnPush() { return m_takeToolstyleToCmdhOnPush; }

protected:

    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void OnDoEvent( a2dCommandProcessorEvent& event );
    void OnUndoEvent( a2dCommandProcessorEvent& event );
    void OnRedoEvent( a2dCommandProcessorEvent& event );

    void OnComEvent( a2dComEvent& event );

    //! tools using this controller
    a2dStToolContr* m_stcontroller;

    //! can be used to take over style from the a2dCentralCanvasCommandProcessor
    bool m_takeToolstyleToCmdhOnPush;
};









//!Interactive drawing of a polygon.
/*!
\remark  Left Click to add points.
\remark  Space bar or Double click to end drawing.
\remark  Cursor keys to shift object.
\remark  Cursor keys + control to shift point.
\remark  Right Click to zoom.

\sa  a2dStToolContr

    \ingroup  tools
*/
class A2DCANVASDLLEXP a2dDrawVPathTool: public a2dStDrawTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_DrawVPath;

    //! constructor
    /*!
        \param controller tool controller one which this tool is pushed.
        \param templateObject when not NULL, this object is cloned and used for the new drawn object.
    */
    a2dDrawVPathTool( a2dStToolContr* controller, a2dVectorPath* templateObject = NULL );

    ~a2dDrawVPathTool();

    //! save to zoom while drawing
    bool ZoomSave() { return true;};

    //! mode of drawing, can be swicthed with Tab key.
    void SetMode( int mode );

protected:

    void OnIdle( wxIdleEvent& event );

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    void OnUndoEvent( a2dCommandProcessorEvent& event );
    void OnRedoEvent( a2dCommandProcessorEvent& event );

    void AddSegment( a2dVpathSegment* segment );

    virtual wxString GetCommandGroupName() { return  _( "Draw polygon" ) ; }

    //! object to clone to use as start
    a2dSmrtPtr<a2dVectorPath> m_templateObject;
    a2dSmrtPtr<a2dPolylineL> m_controlPoints;

    //! previous x
    double m_prev_x;
    //! previous y
    double m_prev_y;

    //! previous x
    double m_contrmid_x;
    //! previous y
    double m_contrmid_y;

    bool m_controlSet;
    bool m_endSegment;
    bool m_bezier;

    //! points sofar
    a2dVpath* m_vpath;

    //! last point
    a2dVpathSegment* m_segment;
    a2dVpathSegment* m_segmentPrev;

    //! end to begin wire.
    bool m_reverse;

public:
    DECLARE_CLASS( a2dDrawVPathTool )
    DECLARE_EVENT_TABLE()

};

//!Interactive follow link.
/*!
\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dFollowLink: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_FollowLink;

    a2dFollowLink( a2dStToolContr* controller );

    bool StartEditing( double x, double y );

    bool StartEditing( int x, int y );

    bool ZoomSave() { return true;};

protected:

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    //! undoable or not
    bool m_withUndo;

public:
    DECLARE_CLASS( a2dFollowLink )
    DECLARE_EVENT_TABLE()

};

//!Interactive drag a a2dPin on a wire.
/*!

\remark  Left Click and drag on Pin.
\remark  Key left right up down to move while draging active (Left Down)

\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dMovePinTool: public a2dDragTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_MovePin;

    //! constructor
    a2dMovePinTool( a2dStToolContr* controller, a2dPin* dragPin = NULL, double xw = 0, double yw = 0, bool disConnectFirst = false );
    //! destructor
    ~a2dMovePinTool();
    
    bool ZoomSave() { return !GetBusy(); }

    void OnMouseEvent( wxMouseEvent& event );

    virtual void AdjustRenderOptions();

protected:

    void OnComEvent( a2dComEvent& event );

    //! when dragging an object from another window into the tool its window, this window wants to have the focus.
    void OnEnter( wxMouseEvent& WXUNUSED( event ) );

    virtual wxString GetCommandGroupName() { return  _( "Move Pin object" ) ; }
    virtual void DoStopTool( bool abort );
    virtual bool EnterBusyMode();
    virtual void FinishBusyMode( bool closeCommandGroup = true );
    virtual void AbortBusyMode();

    a2dSmrtPtr<a2dPin> m_dragPin;
    //! This is the parent, to which the temporary object was added.
    /*! This must be remembered, because it changes if the shows object changes */
    a2dCanvasObjectPtr m_parentAddedTo;
    //! dummy object that is dragged 
    a2dCanvasObjectPtr m_dragPinObj;
    //! the pin on the dummy object that is dragged 
    a2dSmrtPtr<a2dPin> m_dragPinObjPin;

public:

    DECLARE_CLASS( a2dMovePinTool )
    DECLARE_EVENT_TABLE()
};




#endif /* __A2DCANVASSTTOOL_H__ */



