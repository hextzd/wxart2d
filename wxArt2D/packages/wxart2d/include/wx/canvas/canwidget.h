/*! \file wx/canvas/canwidget.h

    \brief widget like objects like buttons and sliders go here.

    \author Michael Sögtrop
    \date Created 11/19/2003

    Copyright: 2003-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: canwidget.h,v 1.20 2008/10/21 21:48:30 titato Exp $
*/

#ifndef __WXCANSELECT_H__
#define __WXCANSELECT_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/candefs.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/rectangle.h"

//!a2dWidgetButton is a a2dCanvasObject based button in a canvas
/*!
a2dWidgetButton implements a a2dCanvasObject based button.
This is usefull for selecting objects from a a2dCanvas that is used
as a palette of objects, styles or tools.
If the object is selected, it is drawn with the styles set by SetSelectedStroke
and SetSelectedFill. Otherwise it is drawn by the usual style for this object.
The default selected style is a one pixel black line and white fill.
The default non-selected style is transparent stroke and fill.

a2dWidgetButton does not need a tool. It processes Mouse events.
Events are first processed by the class itself, and if not handled, redirected to m_parent.
You can also connect dynamically to the wxEvtHandler which is a base class.

\ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dWidgetButton: public a2dWindowMM
{

public:

    enum WidgetMode
    {
        Boundingbox,     /*!< The width and height is the boundingbox of the m_content OR-ed with the button and children.*/
        BoundingboxSize, /*!< Like Boundingbox, but objects with negative extend are shifted positive.*/
        ScaledContent,   /*!< The width and height button, m_content are scaled to fit in.*/
        ScaledContentKeepAspect,   /*!< like ScaledContent, but keep aspect ratio.*/
    };

    //! Standard constructor
    a2dWidgetButton( a2dCanvasObject* parent, int buttonId, double x, double y, double w, double h, WidgetMode mode = Boundingbox );

    a2dWidgetButton( const a2dWidgetButton& other, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dWidgetButton();

    //! set parent object, this is where events are sent if not handled in this.
    virtual void SetParent( a2dCanvasObject* parent ) { m_parent = parent; }

    //! get parent object as event handler.
    virtual wxEvtHandler* GetParentEvtHandler() { return m_parent; }

    //! Set the content object.
    /*!
        child objects can be added to a button, but they will always be drawn unscaled, independent
        of WidgetMode. Therefore one can not fit the children inside of the boundingbox of the button.
        It is important to be able to split normalized content from child objects, for example when
        object tips that are added to the button. Such an object tip should never be scaled, when the
        button itself is resized. So the prefered manner of setting the drawing for a button is using
        SetContentObject(). Children should be used for decorating the button itself, not for
        fitting/scaling the inside drawing of the button.
    */
    void SetContentObject( a2dCanvasObject* content ) { m_content = content; }

    //! Get the content object, \see SetContentObject()
    a2dCanvasObject* GetContentObject() { return m_content; }

    //! Set the border width around the content
    void SetContentBorder( double contentBorder ) { m_contentBorder = contentBorder; }

    //! Get the border width around the content
    double GetContentBorder() { return m_contentBorder; }

    //! Set the selected stroke style for the button
    void SetSelectedStroke( const a2dStroke& selectedStroke ) { m_selectedStroke = selectedStroke; }
    //! Set the selected fill style for the button
    void SetSelectedFill( const a2dFill& selectedFill ) { m_selectedFill = selectedFill; }

    //! Set the selected stroke style for the button
    void SetHighLightStroke( const a2dStroke& highLightStroke ) { m_highLightStroke = highLightStroke; }

    //! Set the selected fill style for the button
    void SetHighLightFill( const a2dFill& highLightFill ) { m_highLightFill = highLightFill; }

    //! id used for button event.
    int GetButtonId() { return m_buttonId; }

// overrides from base class a2dCanvasObject
protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    virtual void DrawHighLighted( a2dIterC& ic );

    virtual void Render( a2dIterC& ic, OVERLAP clipparent );
    virtual bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );
    virtual a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    virtual void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    // event handlers
    DECLARE_CLASS( a2dWidgetButton )
    DECLARE_EVENT_TABLE()

    //! handler for EVT_CANVASOBJECT_ENTER_EVENT
    void OnEnterObject( a2dCanvasObjectMouseEvent& event );

    //! handler for EVT_CANVASOBJECT_LEAVE_EVENT
    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    //! handler for EVT_CANVASOBJECT_MOUSE_EVENT
    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

// member variables
protected:

    //! \see SetSelectedStroke
    a2dStroke m_selectedStroke;

    //! \see SetSelectedFill
    a2dFill m_selectedFill;

    //! \see SetSelectedStroke
    a2dStroke m_highLightStroke;

    //! \see SetSelectedFill
    a2dFill m_highLightFill;

    //! event id used when sending events
    int m_buttonId;

    //! type of button rendering of the m_content object.
    WidgetMode m_mode;

    //! here a border between the button and the content can be given
    double m_contentBorder;

    //! the button its graphics, which will be scaled in WidgetMode::ScaleChildren
    a2dCanvasObjectPtr m_content;

    //! events are sent from the button to this object, if not handled by the button itself.
    //* This is intentionally not a smart pointer to remove a reference loop */
    a2dCanvasObject* m_parent;

    //! for scaling/translating the m_content
    a2dAffineMatrix m_contentWorld;
};


//! Holds several a2dWidgetButton and othere a2dCanvasObject's
/*!
    For a2dWidgetButton childs, the EVT_CANVASOBJECT_BUTTON_ANY is intercepted,
    and the wxEVT_COMMAND_CHOICE_SELECTED is sent to that a2dWidgetButton to process it.
*/
class A2DCANVASDLLEXP a2dWidgetButtonGroup: public a2dCanvasObject
{
public:

    //! Standard constructor
    a2dWidgetButtonGroup( a2dCanvasObject* parent, double x, double y, float extra = 0 );
    //! destructor
    ~a2dWidgetButtonGroup();

    virtual wxEvtHandler* GetParentEvtHandler() { return m_parent.Get(); }

    //! If set to true (default), only a single object can be selected
    void SetSingleSelect( bool singleSelect ) { m_singleSelect = singleSelect; }

    //! set parent object of the pin or some other objects that needs a parent
    virtual void SetParent( a2dCanvasObject* parent ) { m_parent = parent; }

    // overrides from base class a2dCanvasObject
protected:

    virtual void DoRender( a2dIterC& ic, OVERLAP clipparent );

    // event handlers
    DECLARE_CLASS( a2dWidgetButtonGroup )
    DECLARE_EVENT_TABLE()

    void OnAnyButton( wxCommandEvent& event );

    // member variables
protected:

    //! \see SetSingleSelect
    bool m_singleSelect;

    a2dCanvasObjectPtr m_parent;
};

//!specialized a2dWidgetButton which processes a given a2dCommand
/*!
    /! rodo what is m_event doing?
*/
class A2DCANVASDLLEXP a2dWidgetButtonCommand: public a2dWidgetButton
{
public:

    //! Standard constructor
    a2dWidgetButtonCommand( a2dCanvasObject* parent, int buttonId, double x, double y, double w, double h, WidgetMode mode = Boundingbox );
    //! destructor
    ~a2dWidgetButtonCommand();

    //! If set this command will be executed when the object is selected
    /*! If the document has a command processor, the command is sent to this
        command processor. Otherwise it is sent to the global a2dDocumentCommandProcessor */
    void SetCanvasCommand( class a2dCommand* command );

    //! If set this event will be triggered when the object is selected
    /*! The event is owned and deleted by the widget.
        If the top level window of the app is a MDI frame, the command is sent
        to the active child windows of the MDI frane.
        Otherwise the command is sent to the top level window itself. */
    void SetEvent( wxEvent* event );

    //! Set a client data object owned by the command buton
    /*! This is used for events that need a wxClientData object.
        The event won't own or delete such an object but as the event is owned
        by the command button, the command button must take care of this also.
        The command button does not use this in any way, it simply deletes
        it, when it deletes the event.
    */
    void SetClientData( wxClientData* clientData ) { m_clientData = clientData; }

    // overrides from base class a2dCanvasObject
protected:

    // event handlers
    DECLARE_CLASS( a2dWidgetButtonCommand )
    DECLARE_EVENT_TABLE()

    void OnAnyChoice( wxCommandEvent& event );

    // member variables
protected:

    //! \see SetCanvasCommand
    class a2dCommand* m_docviewCommand;
    //! \see SetEvent
    wxEvent* m_event;
    //! Client data (usually used for the event)
    wxClientData* m_clientData;
};

//!a2dWidgetButton is a a2dCanvasObject based button in a canvas
/*!
a2dWidgetButton implements a a2dCanvasObject based button.
This is usefull for selecting objects from a a2dCanvas that is used
as a palette of objects, styles or tools.
If the object is selected, it is drawn with the styles set by SetSelectedStroke
and SetSelectedFill. Otherwise it is drawn by the usual style for this object.
The default selected style is a one pixel black line and white fill.
The default non-selected style is transparent stroke and fill.

a2dWidgetButton does not need a tool. It processes Mouse events.
Events are first processed by the class itself, and if not handled, redirected to m_parent.
You can also connect dynamically to the wxEvtHandler which is a base class.

\ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dWidgetSlider: public a2dWindowMM
{

public:

    //! Standard constructor
    a2dWidgetSlider( a2dCanvasObject* parent, int sliderId, double x, double y, double w, double h );

    a2dWidgetSlider( const a2dWidgetSlider& other, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dWidgetSlider();

    void SetPosition( int position ) { m_position = position; }

    int GetPosition() { return m_position; }

    //! set parent object, this is where events are sent if not handled in this.
    virtual void SetParent( a2dCanvasObject* parent ) { m_parent = parent; }

    //! get parent object as event handler.
    virtual wxEvtHandler* GetParentEvtHandler() { return m_parent; }

    //! Set the selected stroke style for the button
    void SetSelectedStroke( const a2dStroke& selectedStroke ) { m_selectedStroke = selectedStroke; }
    //! Set the selected fill style for the button
    void SetSelectedFill( const a2dFill& selectedFill ) { m_selectedFill = selectedFill; }

    //! Set the selected stroke style for the button
    void SetHighLightStroke( const a2dStroke& highLightStroke ) { m_highLightStroke = highLightStroke; }

    //! Set the selected fill style for the button
    void SetHighLightFill( const a2dFill& highLightFill ) { m_highLightFill = highLightFill; }

    //! id used for button event.
    int GetButtonId() { return m_sliderId; }

// overrides from base class a2dCanvasObject
protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    virtual void DrawHighLighted( a2dIterC& ic );

    virtual void Render( a2dIterC& ic, OVERLAP clipparent );
    virtual bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );
    virtual void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    // event handlers
    DECLARE_CLASS( a2dWidgetSlider )
    DECLARE_EVENT_TABLE()

    //! handler for EVT_CANVASOBJECT_ENTER_EVENT
    void OnEnterObject( a2dCanvasObjectMouseEvent& event );

    //! handler for EVT_CANVASOBJECT_LEAVE_EVENT
    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    //! handler for EVT_CANVASOBJECT_MOUSE_EVENT
    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    void OnSliderEvent( wxCommandEvent& event );

// member variables
protected:

    //! \see SetSelectedStroke
    a2dStroke m_selectedStroke;

    //! \see SetSelectedFill
    a2dFill m_selectedFill;

    //! \see SetSelectedStroke
    a2dStroke m_highLightStroke;

    //! \see SetSelectedFill
    a2dFill m_highLightFill;

    //! event id used when sending events
    int m_sliderId;

    int m_position;
    int m_max;
    int m_min;
    double m_handleH;
    double m_handleW;
    double m_slideW;

    //! events are sent from the button to this object, if not handled by the button itself.
    //* This is intentionally not a smart pointer to remove a reference loop */
    a2dCanvasObject* m_parent;
};

#endif

