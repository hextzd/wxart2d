/*! \file wx/canvas/connectgen.h
    \brief Classes for generating connection between pins in canvas objects

    \author Klaas Holwerda

    Copyright: 2000-2011 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: connectgen.h,v 1.85 2009/07/10 19:23:13 titato Exp $
*/

#ifndef __WXCONNECTGEN_H__
#define __WXCONNECTGEN_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/general/genmod.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/route.h"

class A2DCANVASDLLEXP a2dPinClass;
class A2DCANVASDLLEXP a2dCanvasObject;
class A2DCANVASDLLEXP a2dPin;
class A2DCANVASDLLEXP a2dCanvasObjectList;
class A2DCANVASDLLEXP a2dHabitat;
class A2DCANVASDLLEXP a2dPort;

//! flags for searching a connecting a2dpinClass, for the connecting task at hand.
/*!
    The format is:

    a2d_PinClassWanted_ForPinClassGiven_DirectionFlags

    ObjectPinClass is the object to which we want to connect
    ToConnectPinClass is pinclass from where we want to connect to an object

*/
enum a2dConnectTask
{
    a2d_FeedBackWireStartPin, //!< Return pinclass and pin 
    a2d_StartWire,      //!< find wire/connect pinclass, given start pinclass of start pin
    a2d_StartWire_BasedOnClassStartPin, 
    a2d_StartWire_BasedOnWireClassRequired, 
    a2d_StartWire_BasedOnObjectClassRequired, 

    a2d_GeneratePinsForStartWire, //!< generate pins, given Connect/wire pinclass
    a2d_GeneratePinsForFinishWire, //!< generate pins, given Connect/wire pinclass

    a2d_SearchPinForFinishWire, //!< find normal object pinclass, given Connect/wire pinclass
    a2d_FinishWire,     //!< find normal object pinclass, given Connect/wire pinclass

    a2d_PinToPin, //! connect pins 

    a2d_GeneratePinsForPinClass, //! generate pins on object given a a2dPinClass
};

//! when a new wire or other connection object needs to be created,
/*! this class or a derived one, will deliver the right connection object.
    In other situations it will tell which objects are connectable on two pins, and the wire needed for that.

    The idea is that objects do not only decide themselfs if they can connect to other objects and which.
    Instead this job is centralized to this class. Because of that, an object does not need to know himself
    to which objects it can connect, and what wire to use for that. The a2dConnectionGenerator knows which objects
    can connect to others. Actually in the default situation it is not the canvas object which decides, what can
    be connected to it. Instead the pinclass of a pin on the canvas object, is the input for the a2dConnectionGenerator.
    So one asks the a2dConnectionGenerator if a pinclass of a pin on a certain object, can connect to something else.

    This class here can restrict what is the default way of allowing connections, or implement its own rules.
    When you want to limit the possible connection you can switch to a derived a2dConnectionGenerator.
    An a2dConnectionGenerator is shared by a set of a2dPinClass Objects, and this is how one finds the
    a2dConnectionGenerator for a connection task at hand.

    As an example of a derived a2dConnectionGenerator:
    if you want to limit a wire tool to only draw lines from objects which have pins
    of a specific pin class, overriding GeneratePossibleConnections() can do the job.

    a2dPinClass itself has information on which other a2dPinClass it may connect to, and two
    pin classes can only connect if they both agree.
    a2dPinClass even knows which type of wire/connect object is required to connect the pin to a pin on a wire.
    So when starting a new wire, at a certain pin, the object and wire its a2dPinClass must be able
    to connect to one another. A wiretool is only able to connect two pins of objects if
    the pinclasses in its connect lists are compatible, meaning the one pin contains the other as connectable.
    If the pin is oke, the found PinClassMap defines what type of wire needs to be created now.
    This information is stored in the a2dPinClass, as a template wire/connect object.
    The tool therefore is able to generate a2dCanvasObject connections ( e.g. a2dWirePolylineL wires )
    of different types, and which type is created depends on the pin that is hit when starting a wire.

    In the above one canvas object was normal and the connecting canvas object was a wire.
    Another situation is when two normal canvas objects
    are dragged appart and a wire needs to be created in between. Again pinclasses of the two a2dPin's that will be
    disconnected, will be searched in its connect lists, for a wire pincclass, which can connect to both pins.
    When a wire pinclass is found which can connect to both pins, the type of wire will be created via that same
    pinclass its template object for a wire.  This is achieved CreateConnectObject().

    To understand the principle, imagine 3 a2dCanvasObject's with pins which have Pin classes.
        - one with a2dPinClass A
        - the second with a2dPinClass B
        - the third with a2dPinClass C
    Now when A connects to B we want a wire of type X, if A connects to C, we want wire Y,
    at last when C connects to B we want wire Z.
    On top of this the direction of the connection can be defined, A to C can be different from C to A.
    The information on which pin can connect to which other pin, is stored in the a2dPinClass of the a2dPin.
    Here one can find which other a2dPinClass object can connect to this one, and if it is for a wire, a template object
    to create that wire.

    If more B objects are connected to an A object, via multiple wires, the connection of wires to wires must be defined too
    To conclude we are not only defining the relations between A-B-C, but also the wire begin and end relations towards eachother.
    In our case here, we create wires which are extensions of the pin where they are connected to, meaning the end pin of the wire
    is the same as the pinclass to which the begin pin is connected.
    All this to connect one object pin with pinclass to other objects at a pin with a certain pin class.
    If we define 3 pinclasses for pins on the object A,B,C and three extra pinclasses WA WB WC for wire between those objects,
    we get the following connection tables.

    For the pinclasses which can be connected to each other:

    - (A-PinClass) -> (B-PinClass) (C-PinClass) (WB-PinClass) (WC-PinClass)
    - (B-PinClass) -> (A-PinClass) (C-PinClass) (WA-PinClass) (WC-PinClass)
    - (C-PinClass) -> (A-PinClass) (B-PinClass) (WA-PinClass) (WB-PinClass)

    We get for wires which start at A or C:

    - (A-PinClass) -> (WB-PinClass) Wire X (WA-PinClass) -> (B-PinClass)
    - (A-PinClass) -> (WC-PinClass) Wire Y (WA-PinClass) -> (C-PinClass)
    - (C-PinClass) -> (WB-PinClass) Wire Z (WC-PinClass) -> (B-PinClass)

    And if bidirectional wires which start at B or C:

    - (B-PinClass) -> (WA-PinClass) Wire X (WB-PinClass) -> (A-PinClass)
    - (C-PinClass) -> (WA-PinClass) Wire Y (WC-PinClass) -> (A-PinClass)
    - (B-PinClass) -> (WC-PinClass) Wire Z (WB-PinClass) -> (C-PinClass)

    Interesting in this table is that one can not start a wire on an object, whithout knowing its type, since always two are possible.
    So a wire tool needs to define for which pinclass a wire is needed, before asking this a2dConnectionGenerator to tell if
    the object is oke with that. Another option is to start a wire on a starting pin, and only when finishing the wire decide what
    type of wire is really needed for the begin and end pin of the wire.
    It is best to give wires other pinclasses then normal objects, since that makes it easier to know if one can start a wire
    on a certain pin, and what type of wire it needs to be.

    As an example, drawing a wire using a tool.
    If the user clicks on an object type pin, the corresponding pin
    in the newly created wire will be a Non object type pin.

    At last in the above, there were three objects, but in fact they are of no interest, since the pins its pinclasses define all.
    This way it is possible to define several "flows" in a group of objects. E.g. You can define in and output pinclasses for each
    flow you require. Multiple flow pins can be added to the same object, connecting a set of flow pins by one type of wire.
    You end up with several flows within a group of objects ( control flow - data flow ).

	Where need a2dConnectionGenerator takes settings from the currently active a2dHabitat: a2dCanvasGlobals->GetHabitat()
	So if not set already by the currently active view/drawingpart, set it first ( a2dDrawingPart::SetShowObject() ).
*/
class A2DCANVASDLLEXP a2dConnectionGenerator : public a2dObject
{


public:

    enum RouteMethod
    { 
        StraightSegment, //!< Remove original and replace with one straight segment, dynamic pins move with segment 
        StraightEndSegment, //!< Reuse last straight segment at begin or end of line, dynamic pins move with segment 
        GridRouting, //!< Reroute using Lee routing
        StraightEndSegmentUnlessPins, //!< Same as StraightEndSegment, unless there are dynamic pins on segment 
        StraightSegmentUnlessPins, //!< Same as StraightSegment, unless there are dynamic pins on segment 
        ManhattanSegments, //!< Remove original and replace with manhattan lines 
        ManhattanEndSegments, //!< Add manhattan lines to end.
        ManhattanEndSegmentsStay,  //!< Keep manhattan lines at end intact.
        ManhattanEndSegmentsConvertAndStay //!< Convert straight segments to manhattan lines and keep manhattan lines at end intact.
    };
	
    //! constructor
    a2dConnectionGenerator();
    //! destructor
    ~a2dConnectionGenerator();

    //! non interactive way to directly reroute wires connected to some objects.
    void ReRouteNow( a2dCanvasObject* parent, a2dCanvasObjectList& objects );

    //! create connection object based on two pins which need to be connected.
    /*!
        Called from the default a2dCanvasObject::CreateConnectObject(), in order to easily change the behaviour
        of standard objects concerning possible connections.
        The returned object is a (connection) object (e.g. a2dWirePolylineL ), with correct pins at the
        position of pinThis and pinOther. The pins are connected already.
        When undo is true, the right commands are sent to the document its command processor.
        In general this means those commands are part of a group of commands in a a2dCommandGroup,
        which internal resulted in a connection being created. For example as a result of dragging an object.
    */
    virtual a2dCanvasObject* CreateConnectObject( a2dCanvasObject* parent, a2dPin* pinThis, a2dPin* pinOther, bool undo = false ) const;

    //! create connection object based on two pin classes, which (may) need to be connected.
    /*!
        The default just returns m_tmplObject.
    */
    virtual a2dCanvasObject* GetConnectTemplate( const a2dCanvasObject* object, a2dPinClass* thisPinClass, const a2dCanvasObject* other, a2dPinClass* otherPinClass ) const;

    //! set begin state of pins, before a tools starts asking feedback or after tool is finsihed
    virtual void SetPinsToBeginState( a2dCanvasObject* root, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::VISIBLE );   

    //! set end state of pins after tool is finsihed
    virtual void SetPinsToEndState( a2dCanvasObject* root, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::VISIBLE );   

    //! generate temporary pins to which objects can connect
    /*!
        When drawing wires object, other object are asked to display pin position,
        to which the wire may connect.  See a2dCanvasObject::GeneratePinsPossibleConnections(). This process is called by the tools,
        and is called editing Feedback.  The pins created are only temporary, and will be removed at the end of a tool its busy cycle
        or when needed.

        The default implementation uses pinClass->GetConnectionGenerator()
        to ask this object to generate a pin in a2dCanvasObject::GeneratePins(). The reason for this, is that there may be more involved
        to allow a pin to connect. Like the pin class.

        \param object The object on which to create temporary pins
        \param pinClass The pinclass to which the generated pins must be able to connect, if NULL any pinclass
        \param task for what purpose is the connection needed
        \param x only connect at this position
        \param y only connect at this position

    */
    virtual bool GeneratePossibleConnections( a2dCanvasObject* object, a2dPinClass* pinClass, a2dConnectTask task, double x, double y, double margin ) const;

    //! return a a2dPinClass which should be used to connect to the input a2dPinClass.
    /*!
        The connection generator searches for a possible connection to the input a2dPinClass in combination
        with the canvasobject obj if needed.

        \param pinClass pin class for which to search a connecting PinClass
        \param task for what purpose is the connection needed
        \param obj object for which pins are checked / needed.

        \return If a a2dPinClass is not found NULL is returned.
    */
    virtual a2dPinClass* GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj = NULL, a2dPinClass* pinClassTo = NULL, a2dPin* pinFrom = NULL ) const;

    //! return a port that fits the to input pins.
    virtual a2dPort* GetPortForPinClasses(  a2dPin* one,  a2dPin* two );

    //! return the pin class for GetPinClassForTask( a2dPinClass::Any  )
    /*!
        Basic object can generate pins by a2dPinclass, but i8n case of a2dPinClass::Any
        it will generate pins of this pinclass.
    */
    a2dPinClass* GetAnyPinClass() const { return m_anypinclass; }

    //! see GetAnyPinClass()
    void SetAnyPinClass( a2dPinClass* pinClass ) { m_anypinclass = pinClass; }

    a2dPinClass* GetReturnPinClass() const { return m_returnPinClass; }

    a2dCanvasObject* GetReturnConnect() const { return m_returnConnect; }

    //! when a wire was created, this return the direction is was created ( first to last pin or visa versa ).
    bool GetLastConnectCreationDirection() const { return m_reverseCreate; }

	virtual bool GeneratePinsToConnect( a2dDrawingPart* part, a2dCanvasObject* root, a2dPinClass* pinClassToConnectTo, a2dConnectTask task, double xpin, double ypin, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::VISIBLE );

	/*!

        \param root search in children fromt this.
        \param pin The pin to which the generated pins must connect
        \param pinClass The pinclass to which the generated pins must be able to connect, if NULL any pinclass
		\param margin how close to pinToConnectTo, must the searched pins be.
        \param mask search only objects with this mask

    */
	virtual a2dPin* SearchPinForFinishWire( a2dCanvasObject* root, a2dPin* pinToConnectTo, a2dPinClass* pinClassToConnectTo, double margin, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::VISIBLE );

	/*!

        \param root search in children fromt this.
        \param xpin The pin x to which the generated pins must connect
        \param ypin The pin y to which the generated pins must connect
        \param pinClass The pinclass to which the generated pins must be able to connect, if NULL any pinclass
		\param margin how close to pinToConnectTo, must the searched pins be.
        \param mask search only objects with this mask

    */
	virtual a2dPin* SearchPinForStartWire( a2dCanvasObject* root, double xpin, double ypin, a2dPinClass* pinClassToConnectTo, double margin, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::VISIBLE );

    //! Generate pins on objects, and test which can connect to the given object.
    /*!
        Pins in the given canvas object are tested for possible connection to pins on other objects.
        Meaning if pins of other objects are at the same position as a pin of the given object,
        a test done, to see if a connection can be made.
        The connection is not made yet, it is only an indication that connection is possible.
        This is done by setting the rendering mode for those pins different.
        The will automatically be re-rendered in idle time.

        Some canvas objects can create pins automatically. If the pins of the given object, hit another objects,
        and it can generate pins automatically, this will be done first. Next a connection test on those pins is made.
        The generated pins are temporary, and if no connection is made, they will be deleted in idle time.

        \ingroup docalgo
    */
	virtual bool GeneratePinsToConnectObject( a2dDrawingPart* part, a2dCanvasObject* root, a2dCanvasObject* connectObject, a2dPinClass* pinClassToConnectTo = NULL, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::VISIBLE );

    virtual bool ConnectToPinsObject( a2dCanvasObject* root, a2dCanvasObject* connectObject, double margin, a2dPinClass* pinClassToConnectTo = NULL, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::VISIBLE );

    //! the template object is used as a template for creating new wires between pins.
    /*! You can use it for other purposes as well if needed.
        This is a template for a new wire.
    */
    void SetConnectObject( a2dCanvasObject* tmplObject );

    //! return connect object.
    a2dCanvasObject* GetConnectObject() const;

    void AddRerouteWires( a2dCanvasObjectList* wires, a2dCanvasObject* parentOfWires );

    void AddRerouteWire( a2dCanvasObject* wire, a2dCanvasObject* parentOfWires );

    void PrepareForRewire( a2dCanvasObject* parent, a2dCanvasObjectList& dragList, bool walkWires = true, bool selected = false, bool stopAtSelectedWire = false, bool CreateExtraWires = true, a2dBaseTool* tool = NULL, a2dRefMap* refs = NULL );

    //! create wires on pins which do not have wires, but directly are connected to other objects.
    /*!
        This prepares the object for dragging/moving, while preserving the connection, since then wires will
        be rerouted when dragging.
    */
    void CreateWiresOnPins( a2dCanvasObject* parent, const a2dCanvasObjectList& objectsToDrag, bool undo, bool onlyNonSelected = false );

    void OptimizeRerouteWires( bool removeZero = false );

    //! Reset clonebrothers
    /*
       All clonebrothers in pins are reset to NULL.
    */
    void ResetCloneBrother();

    void ResetPositionsToOrignals();

    /*! prepare a set of wires for rerouting them.
        
        
        \param tool ointer to the tool it was called from
        \param resetCloneBrothers if true all clonebrothers in pins are reset to NULL.
    */
    void PrepareForRerouteWires( a2dBaseTool* tool = NULL, a2dRefMap* refs = NULL );

    void RerouteWires( bool final = false, bool fromOriginal = true );

    //! route also while dragging object
    void SetRouteWhenDrag( bool routeWhenDrag ) { m_routeWhenDrag = routeWhenDrag; }

    //! route also while dragging object
    bool GetRouteWhenDrag() { return m_routeWhenDrag; }

    //! set the way routing will be done
    void SetRouteMethod( RouteMethod routing ) { m_routeMethod = routing; }

    RouteMethod GetRouteMethod() { return m_routeMethod; }

    wxString GetRouteMethodAsString();

    //! cycle through routing methods
    void RotateRouteMethod();

    //! if set a busy cursor is shown when rewiring
    void SetWithBusyCursor( bool withBusyEvent ) { m_withBusyEvent = withBusyEvent; }

    //! if set a busy cursor is shown when rewiring
    bool GetWithBusyCursor() const { return m_withBusyEvent; }

    //! when optimizing wires, and wire direction (end and begin pins) is not important, this can be set true.
    //! When true, joining wires which are not in same direction, are redirected.
    void SetAllowRedirect( bool allowredirect ) { m_allowredirect = allowredirect; }

    //! see SetAllowRedirect()
    bool GetAllowRedirect() { return m_allowredirect; }

	void SetGeneratePins( bool onOff ) { m_generatePins = onOff; }

	bool GetGeneratePins() const { return m_generatePins; }

    void SetNoEditCopy( bool noEditCopy ) { m_noEditCopy = noEditCopy; }

    bool GetNoEditCopy() { return m_noEditCopy; }

    void SetOffSet( double offset ) { m_offset = offset; }
    double GetOffSet() const { return m_offset; }

protected:

    void EndManhattanRoute( a2dPin* pin, a2dLineSegment* seg1, a2dLineSegment* seg2, a2dLineSegment* seg3, a2dVertexList* points, bool reverse );

    a2dCanvasObjectList* m_wires;
    a2dCanvasObjectList* m_wiresEditCopies;

    bool m_routeWhenDrag;
    RouteMethod m_routeMethod;

    //! Template for created object which represents a connection. e.g a2dWirePolylineL.
    mutable a2dCanvasObjectPtr m_tmplObject;

    a2dPinClass* m_returnPinClass;

    a2dCanvasObject* m_returnConnect;

    //! template pinclass
    a2dPinClass* m_anypinclass;

    //! how to create a connection
    mutable bool m_reverseCreate;

    bool m_allowredirect;

    bool m_generatePins;

    double m_offset;

    bool m_withBusyEvent;

private:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const {  return NULL; };

    a2dSmrtPtr< a2dRouteData > m_routedata;
    
    a2dCanvasObject* m_parent;

    void RerouteWire( a2dWirePolylineL* wire, bool& again );
    void PrepareGridRoute();
    void AutoConnect( a2dWirePolylineL* wire, bool allowreconnectbegin, bool allowreconnectend );

    bool m_noEditCopy;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) ) { wxASSERT( 0 ); }
    virtual void DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) ) { wxASSERT( 0 ); }
#endif //wxART2D_USE_CVGIO

};

//! Smart pointer type for a2dConnectionGenerator
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dConnectionGenerator>;
typedef a2dSmrtPtr<a2dConnectionGenerator> a2dConnectionGeneratorPtr;

//! specialized generator for flow on one a2dcanvasObject using wires
/*!
	This a2dConnectionGenerator can be set to a a2dPinClass, and will be called from there.
*/
class A2DCANVASDLLEXP a2dConnectionGeneratorInOut : public a2dConnectionGenerator
{

public:

    //! constructor
    a2dConnectionGeneratorInOut();
    //! destructor
    ~a2dConnectionGeneratorInOut();

    virtual a2dPinClass* GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj = NULL, a2dPinClass* pinClassTo = NULL, a2dPin* pinFrom = NULL ) const;

    //! Pins of this class can only connect to Output pins
    static a2dPinClass* Input;
    //! Pins of this class can only connect to Input pins
    static a2dPinClass* Output; 

protected:

    //! Some stuff needed by the ref counting base class
    virtual a2dObject* Clone( CloneOptions WXUNUSED( options ) ) const { wxASSERT( 0 ); return 0; }

private:

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) ) { wxASSERT( 0 ); }
    virtual void DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) ) { wxASSERT( 0 ); }
#endif //wxART2D_USE_CVGIO

};

//! specialized generator for flow on one a2dcanvasObject using wires
/*!
	This a2dConnectionGenerator can be set to a a2dPinClass, and will be called from there.
*/
class A2DCANVASDLLEXP a2dConnectionGeneratorObjectWire : public a2dConnectionGenerator
{

public:

    //! constructor
    a2dConnectionGeneratorObjectWire();
    //! destructor
    ~a2dConnectionGeneratorObjectWire();

    virtual a2dPinClass* GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj = NULL, a2dPinClass* pinClassTo = NULL, a2dPin* pinFrom = NULL ) const;

    //! Pins of this class can only connect to Wire pins
    static a2dPinClass* Object;
    //! Pins of this class can only connect to Object pins
    static a2dPinClass* Wire;

protected:

    //! Some stuff needed by the ref counting base class
    virtual a2dObject* Clone( CloneOptions WXUNUSED( options ) ) const { wxASSERT( 0 ); return 0; }

private:

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) ) { wxASSERT( 0 ); }
    virtual void DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) ) { wxASSERT( 0 ); }
#endif //wxART2D_USE_CVGIO

};

//! specialized generator for flow on one a2dcanvasObject using wires
/*!
	This a2dConnectionGenerator can be set to a a2dPinClass, and will be called from there.
*/
class A2DCANVASDLLEXP a2dConnectionGeneratorDirectedFlow : public a2dConnectionGenerator
{


public:

    //! constructor
    a2dConnectionGeneratorDirectedFlow();
    //! destructor
    ~a2dConnectionGeneratorDirectedFlow();

    virtual a2dPinClass* GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj = NULL, a2dPinClass* pinClassTo = NULL, a2dPin* pinFrom = NULL ) const;

    //! Pins of this class can only connect to WireInput pins
    static a2dPinClass* ObjectInput;
    //! Pins of this class can only connect to WireOutput pins
    static a2dPinClass* ObjectOutput;
    //! Pins of this class can only connect to ObjectInput pins
    static a2dPinClass* WireInput;
    //! Pins of this class can only connect to ObjectOutput pins
    static a2dPinClass* WireOutput;

protected:

    //! Some stuff needed by the ref counting base class
    virtual a2dObject* Clone( CloneOptions WXUNUSED( options ) ) const { wxASSERT( 0 ); return 0; }

private:

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) ) { wxASSERT( 0 ); }
    virtual void DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) ) { wxASSERT( 0 ); }
#endif //wxART2D_USE_CVGIO

};

//! specialized generator for two flow on one a2dcanvasObject
/*!
	This a2dConnectionGenerator can be set to a a2dPinClass, and will be called from there.
*/
class A2DCANVASDLLEXP a2dConnectionGeneratorTwoFlow : public a2dConnectionGenerator
{


public:

    //! constructor
    a2dConnectionGeneratorTwoFlow();

    //! destructor
    ~a2dConnectionGeneratorTwoFlow();

    virtual a2dPinClass* GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj = NULL, a2dPinClass* pinClassTo = NULL, a2dPin* pinFrom = NULL ) const;

    virtual a2dCanvasObject* CreateConnectObject( a2dCanvasObject* parent, a2dPin* pinThis, a2dPin* pinOther, bool undo = false ) const;

    //! Pins of this class can only connect to FlowAWireOutput and FlowAOutput pins
    static a2dPinClass* FlowAInput;
    //! Pins of this class can only connect to FlowAWireInput and FlowAInput pins
    static a2dPinClass* FlowAOutput;
    //! Pins of this class can only connect to FlowAWireOutput and FlowAOutput pins
    static a2dPinClass* FlowAWireInput;
    //! Pins of this class can only connect to FlowAInput and FlowAWireInput pins
    static a2dPinClass* FlowAWireOutput;

    //! Pins of this class can only connect to FlowBWireOutput and FlowBOutput pins
    static a2dPinClass* FlowBInput;
    //! Pins of this class can only connect to FlowBWireInput and FlowBInput pins
    static a2dPinClass* FlowBOutput;
    //! Pins of this class can only connect to FlowBWireOutput and FlowBOutput pins
    static a2dPinClass* FlowBWireInput;
    //! Pins of this class can only connect to FlowBInput and FlowBWireInput pins
    static a2dPinClass* FlowBWireOutput;

    //! the template object is used as a template for creating new wires between pins.
    /*! You can use it for other purposes as well if needed.
        This is a template for a new wire in FlowB.
    */
    void SetConnectObjectFlowB( a2dCanvasObject* tmplObject );

    //! return connect object.
    a2dCanvasObject* GetConnectObjectFlowB() const;

protected:

    //! Template for created object which represents a connection. e.g a2dWirePolylineL.
    mutable a2dCanvasObjectPtr m_tmplObjectFlowB;

    //! Some stuff needed by the ref counting base class
    virtual a2dObject* Clone( CloneOptions WXUNUSED( options ) ) const { wxASSERT( 0 ); return 0; }

private:

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) ) { wxASSERT( 0 ); }
    virtual void DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) ) { wxASSERT( 0 ); }
#endif //wxART2D_USE_CVGIO

};

#endif    // WXCANOBJ


