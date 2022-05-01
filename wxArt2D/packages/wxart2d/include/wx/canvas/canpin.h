/*! \file wx/canvas/canpin.h
    \brief pins and handles

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canprim.h,v 1.37 2009/07/17 16:03:34 titato Exp $
*/

#ifndef __WXCANPIN_H__
#define __WXCANPIN_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/candefs.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/connectgen.h"

//! a2dHandle is used inside editing versions of a certain objects.
/*! In that case instances of this object are used to draw dragging Handles.
\remark width and height are defined in pixels.

\ingroup canvasobject
 */
class A2DCANVASDLLEXP a2dHandle: public a2dCanvasObject
{

    DECLARE_EVENT_TABLE()

public:

    //! constructor
    a2dHandle();

    //! construct a handle
    /*!
        \param parent parent of handle
        \param xc center x
        \param yc center y
        \param name name of handle
        \param w  width of handle rectangle
        \param h  height of handle rectangle
        \param angle angle of rotation of handle rectangle
        \param radius for a rounded handle rectangle
    */
    a2dHandle( a2dCanvasObject* parent, double xc, double yc, const wxString& name = wxT( "" ), double w = 0, double h = 0, double angle = 0 , double radius = 0 );
    a2dHandle( const a2dHandle& other, CloneOptions options, a2dRefMap* refs );
    ~a2dHandle();

    //!get the name given to the handle.
    wxString GetName() const { return m_name; }

    //! set the name of the handle (must be unique)
    void SetName( const wxString& name ) { m_name = name; }

    //! set all parameters
    /*!
        \param xc center x
        \param yc center y
        \param w  width of handle rectangle
        \param h  height of handle rectangle
        \param angle angle of rotation of handle rectangle
        \param radius for a rounded handle rectangle
    */
    void Set( double xc, double yc, double w, double h, double angle = 0 , double radius = 0 );

    //! set basic parameters
    /*!
        \param xc center x
        \param yc center y
        \param name empty means leave name as is
    */
    void Set2( double xc, double yc, const wxString& name = wxT( "" ) );

    //! set parent object of the pin
    void SetParent( a2dCanvasObject* parent );

    //! get parent object of the pin
    a2dCanvasObject* GetParent() const { return m_parent; }

    //!return width
    double GetWidth() const { return m_width; }

    //!return height
    double GetHeight() const { return m_height; }

    //!return radius
    double GetRadius() const { return m_radius; }

    void SetMode( int mode )
    {
        m_mode = mode; SetPending( true );
    }

    int GetMode() const { return m_mode; }

    static void SetWorldBased( bool worldBased ) { m_worldBased = worldBased; }

    static bool GetWorldBased() { return m_worldBased; }

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO
    virtual bool IsTemporary_DontSave() const;

    DECLARE_DYNAMIC_CLASS( a2dHandle )

protected:

    //! clone it
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void OnEnterObject( a2dCanvasObjectMouseEvent& event );

    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //! name of the pin
    wxString m_name;

    //!width in pixels
    double m_width;

    //!height in pixels
    double m_height;

    //!radius of rectangle that will be rendered
    double m_radius;

    //! modifies rendering
    int m_mode;

    //!what is the a2dCanvasObject that i am part of
    //* This is intentionally not a smart pointer to remove a reference loop */
    a2dCanvasObject* m_parent;

    static bool m_worldBased;

public:
    static a2dPropertyIdInt32* PROPID_Width;
    static a2dPropertyIdInt32* PROPID_Height;
    static a2dPropertyIdInt32* PROPID_Radius;

    DECLARE_PROPERTIES()

public:
    const static long sm_HandleNoHit;
    const static long sm_HandleHit;

    //static const a2dPropertyIdLineSegment PROPID_linesegment;


private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dHandle( const a2dHandle& other );
};

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dHandle>;
#endif

//! smart pointer for a2dHandle
typedef a2dSmrtPtr<a2dHandle> a2dHandlePtr;


class A2DCANVASDLLEXP a2dPinClass;
#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DCANVASDLLEXP std::allocator<a2dPinClass*>;
template class A2DCANVASDLLEXP std::allocator< std::_List_nod<a2dPinClass*, std::allocator<a2dPinClass* > >::_Node >;
template class A2DCANVASDLLEXP std::allocator< std::_List_ptr<a2dPinClass*, std::allocator<a2dPinClass* > >::_Nodeptr >;
template class A2DCANVASDLLEXP std::list< a2dPinClass* >;
#endif

class A2DCANVASDLLEXP a2dParPin;

#define a2dAnyPin NULL

//! a2dPin is used in a2dCanvasObject to add pins to it.
/*!
   This class is meant to be added as child object to a a2dCanvasObject, there it is treated in a special manner
   to connect the parent a2dCanvasObject to other a2dCanvasObject's.
   Using pins one can create graph/diagram structures of connected a2dCanvasObjects.
   Pins can connect one canvas object to another via two connected pins.
   The Pin can be connected to several other Pins.
   Special connection canvas objects like a2dWirePolylineL and a2dWires,
   can be used to create wires between canvas objects.
   The connection objects also have pins, which they use to connect to other a2dCanvasObjects.
   Fro instance a2dWirePolylineL is a wire, which can have many pins along it, which are connected to othere canvas objects.
   Those wire line objects do recalculate the wires when moving a connected a2dCanvasObject.
   This is often called rubberbanding. A wire connection objects are used in dedicated reroute algorithms.
   a2dConnectionGenerator is used to implement those algorithms. It work on sets of wires which are connected to other objects.
   Moving the objects, leads to rewiring the connection objects.

   One can traverse connected a2dCanvasObject's via its pins, since the Pin has a parent pointer
   to the a2dCanvasObject it belongs to. This way one can find groups of connected objects.

   a2dCanvasObject has member functions to manipulate the a2dPin's it has.
   A pin itself is created by its parent, and therefore parent object decides where pins are located,
   and what needs to happen when one wants to connect to one of its pins.
   This is/can be specific for each derived a2dCanvasObject and each of its pins.
   To achieve different behaviour for pins without deriving new pin classes, a pin has a a2dPinClass member.
   The a2dPinClass is used to specialize a pin for a certain connection purpose.
   Think of input and output pins on objects. One can for instance not connect an input pin of one object to
   the input pin of another object.
   All these constraints can be stored in the pin its a2dPinClass.
   So in general there will be no need to derive your own MyCanvasPin, unless you want to e.g. change rendering.

   a2dPinClass is not stored into CVG format, only its name is stored, the list of available
   pinclasses is assumed to be specific to the application.

   A pin does have an angle, this make the relative angle to a connected Pin
   of another a2dCanvasObject well defined. Certain application require object to be connected at the same angles.

   Pins with the visible fag set false, will not be connected in routines where connections are being made. 

   \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dPin: public a2dCanvasObject
{

    DECLARE_EVENT_TABLE()

public:

    typedef wxUint16 a2dPinFlagsMask;

    enum a2dPinFlags
    {
        /*! no flag set */
        NON  = 0x0,

        /*! dynamic pins can walk along a line */
        dynamic = 0x0001,

        /*! temporary pins are created to indicate possible connection, and deleted or cloned when connected */
        temporary = 0x0002,

        /*! set when pin is on an object and not a wire or connect.*/
        objectPin = 0x0004,

        /*! true when pin is internal to object border. */
        internal  = 0x0008,

        /*! temporary and objectPin set */
        temporaryObjectPin = temporary | objectPin,

        /*! For Xor ing and all set */
        ALLSET  = 0xFFFFFFFF
    };

    //! constructor
    a2dPin();

    //! constructor
    a2dPin( a2dCanvasObject* parent, const wxString& name, a2dPinClass* pinclass,  double xc, double yc, double angle = 0 , double w = 0, double h = 0, double radius = 0 );

    //! copy constructor
    a2dPin( const a2dPin& other, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dPin();

    //! flag to render connected pin
    /*!
        \remark set the object its pending flag and inform root about this
        \remark default false
        \param RenderConnected if true connected pin is rendered visible
    */
    inline void SetRenderConnected( bool RenderConnected ) { SetPending( true ); m_RenderConnected = RenderConnected; }

    //! \return true is connected pins are rendered
    inline bool GetRenderConnected() { return m_RenderConnected; }

    virtual void SetPending( bool pending );

    //! set parent object of the pin
    virtual void SetParent( a2dCanvasObject* parent );

    //! get parent object of the pin
    a2dCanvasObject* GetParent() const { return m_parent; }

    //!return width
    double GetWidth() const { return m_width; }

    //!return height
    double GetHeight() const { return m_height; }

    //!return radius
    double GetRadius() const { return m_radius; }

    //! default used to modify rendering
    void SetMode( int mode )
    {
        m_mode = mode; SetPending( true );
    }

    //! get current mode 
    int GetMode() const { return m_mode; }

    //! set if pins its size is in world coordinates, else it is in pixels.
    static void SetWorldBased( bool worldBased ) { m_worldBased = worldBased; }

    //! get if pin sizes are in world coordinates, else it is in pixels.
    static bool GetWorldBased() { return m_worldBased; }

    //!get the name given to the pin.
    wxString GetName() const { return m_name; }

    //! set the name of the pin (must be unique)
    void SetName( const wxString& name ) { m_name = name; }

    //! set postion angle and name of the pin
    /*!
        \param xc x-position
        \param yc y-position
        \param angle the angle
        \param name empty means leave name as is
        \param dynamic when true pin is only a temporary pin.
    */
    void Set( double xc, double yc, double angle = 0 , const wxString& name = wxT( "" ), bool dynamic = false );

    //! get absolute position of the pin ( after applying the parent's matrix and its own matrix )
    a2dPoint2D GetAbsXY() const;

    //! get absolute X position of the pin ( after applying the parent's matrix and its own matrix )
    double GetAbsX() const;

    //! get absolute Y position of the pin ( after applying the parent's matrix and its own matrix )
    double GetAbsY() const;

    //! set the absolute X,Y position of the pin ( after applying the parent's matrix and its own matrix )
    void SetAbsXY( double x, double y );

    //! set the absolute X,Y position of the pin ( after applying the parent's matrix and its own matrix )
    void SetAbsXY( const a2dPoint2D& point );

    //! get absolute angle of the pin ( after applying the parent its matrix and it own matrix )
    double GetAbsAngle() const;

    //! return the pin class of this pin
    a2dPinClass* GetPinClass() const { return m_pinclass; }

    //! Set the pin class
    void SetPinClass( a2dPinClass* pinClass ) { m_pinclass = pinClass; }

    //! Is given pin allowed to connect to this pin
    /*!
        Override this function to make restriction on connecting pins.
        E.g You might make restriction for pins on one layer, not to connect with pins on other layers.

        The default implementation checks if the two pins have a2dPinClass which a2dPinClass::CanConnectTo() to eachother.
    */
    virtual bool MayConnectTo( a2dPin* connectto );

    //! If pin is connected to given pin, it will be disconnected from that other pin.
    //! Next to that the rendering mode will be set to sm_PinUnConnected.
    //! The connection entries are set to NULL in m_connectedPins and pin->m_connectedPins.
    //! They are not erased from the list, in order to not interupt ongoing iterations at a higher level.
    //! Call CleanUpNonConnected() to really erase them, or it will be done at destruction of pin.
    //! \param pin if not NULL, this pin connected is removed or set to NULL.
    //! \param forceErase if true, the pin connections is removed from the connection list else, only set to NULL. 
    bool Disconnect( a2dPin* pin = a2dAnyPin, bool forceErase = false ); 

    //! connect this pin to the given pin
    void ConnectTo( a2dPin* connectto );

    //! Return the pin to which this pin is connected.   
    /*!
        \param pin look for connection to this pin, if not given, first connected pin is returned.
    */
    a2dPin* IsConnectedTo( a2dPin* pin = a2dAnyPin ) const;

    //! find connecting pin which is not a wire.
    //! Return NULL if non found
    //! /param mask the connected object must have given mask.
    a2dPin* FindNonWirePin( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! find connecting pin which is a wire.
    //! Return NULL if non found
    //! /param mask the connected object must have given mask.
    a2dPin* FindWirePin( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! Find a connectable pin.
    /*! This only searches for pins of child objects of the root given */
    a2dPin* FindConnectablePin( a2dCanvasObject* root, double margin, bool autocreate );

    //! Try to connect this pin to another pin at the same location
    void AutoConnect( a2dCanvasObject* root, double margin );

    //! Check if this pin is Dislocated ( pin pos different from connected pins pos )
    //! returns connected pin which is dislocated the most.
    a2dPin* IsDislocated() const;

    //! Check if this pin is at same location
    bool IsSameLocation( a2dPin* other, double margin = 0 ) const;

    //! return true if this pin is a dynamic pin
    /*!
        Pins like this are create automatically by some objects, when they are able to connect.
        The location is dynamic, and therefore they are called dynamic.
        When moving connected objects/wires, the pins may move in position.
    */
    bool IsDynamicPin() const { return m_dynamicPin; }

    //! set when pin is a dynamic pin
    void SetDynamicPin( bool dynamicPin ) { m_dynamicPin = dynamicPin; }

    //! return true if this pin is a temporary pin
    /*!
        Pins like this are created automatically by some objects, when they are able to connect.
        Since for Undo actions, commands are normally used to create extra pins inside tools.
        But a temporary pin is in general for editing feedback, and therefore it only will stay alive when
        the tools attempt to connect a second pin to it.
        At that moment it becomes clear that a command is needed to create and connect the temporary pin using commands.
        If not, the temporary pin will be deleted in the end, as if it did never existed, and no commands on the command stack
        were issued.
    */
    bool IsTemporaryPin() const { return m_temporaryPin; }

    //! set when pin is only a temporary pin, which will be deleted by the application if not connected
    //! to another pin later.
    void SetTemporaryPin( bool temporaryPin ) { m_temporaryPin = temporaryPin; }

    //! set when pin is within the border of its parent
    /*!
        Used when routing wires to a pin which is within the object and not on the border.
        Meaning wires must be able to route within the object that contains the pin.
    */
    void SetInternal( bool internal ) { m_internal = internal; }

    //! see SetInternal()
    bool IsInternal() const { return m_internal; }

    //! set when pin is an object pin
    void SetObjectPin( bool objectPin ) { m_objectPin = objectPin; }

    //! return true if the pin is on object instead of wire/connects
    bool IsObjectPin() const { return m_objectPin; }

    a2dPinFlagsMask GetPinFlags() const;

    void SetPinFlags( a2dPinFlagsMask which, bool value );

    void SetPinFlags( a2dPinFlagsMask which );

    //! Return list fo connected pins.
    a2dlist< a2dDumbPtr<a2dPin> >& GetConnectedPins() { return m_connectedPins; }

    //!Get the number of connected pins
    wxUint32 GetConnectedPinsNr() const;

    //! Items in m_connectedPins which are NULL will be erased.
    //! The NULL items is how Disconnect() does disconnect pins, if erase is not forced.
    void CleanUpNonConnected();

    //! all connections to this pins, will become available in those connected pins too.
    void DuplicateConnectedToOtherPins( bool undo = false );

    //! pins connected to given other pin, will become part of this pin.
    void DuplicateConnectedPins( a2dPin* other, bool undo = false );

    //! pins connected to given other pin, will be removed on this pin.
    void RemoveDuplicateConnectedPins( a2dPin* other );

    // to temporarily disable rendering ( like when rendering from a2dCameleonInst )
    static void SetDoRender( bool doRender ) { m_doRender = doRender; }

    // to temporarily disable rendering ( like when rendering from a2dCameleonInst )
    static bool GetDoRender() { return m_doRender; }

    virtual bool IsTemporary_DontSave() const;
    virtual bool AlwaysWriteSerializationId() const;
    virtual bool LinkReference( a2dObject* other );

    //! find wires startin at this Pin towards a second pin
    /*!
        A depth first search for correct branches. Wire passed towards  second pin, are stored to result.
        Wires on the path to the second pin, do get the bin2 flag set.

        \param pinTo wire path ending at this pin is correct
        \param result wires to pinTo stored here, unless NULL
    */    
    bool FindWiresToPin( a2dPin* pinTo, a2dCanvasObjectList* result = NULL );

    DECLARE_DYNAMIC_CLASS( a2dPin )

    //****************** RENDERING AND STYLE ******************/
    /*! \name Rendering mode, set by connection feedback in e.g. a wire tool
    */
    //\{
    //! Pin is not connected rendering mode
    const static long sm_PinUnConnected;
    //! Pin is connected rendering mode
    const static long sm_PinConnected;
    //! Pin can be connected rendering mode
    const static long sm_PinCanConnect;
    //! Pin can NOT be connected rendering mode
    const static long sm_PinCannotConnect;
    //! Pin can connect to supplied a2dPinClass
    const static long sm_PinCanConnectToPinClass;
    //\}

protected:

    //! \remark the parent of the pin is taken over, but its connection pin only via refs to resolve
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    void OnEnterObject( a2dCanvasObjectMouseEvent& event );

    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    virtual void DrawHighLighted( a2dIterC& ic );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

#ifdef _DEBUG
    virtual void DoDump( int indent, wxString* line );
#endif

    //! name of the handle
    wxString m_name;

    //!width in pixels
    double m_width;

    //!height in pixels
    double m_height;

    //!radius of rectangle that will be rendered
    double m_radius;

    //! modifies rendering
    int m_mode;

    //!what is the a2dCanvasObject that i am part of
    //* This is intentionally not a smart pointer to remove a reference loop */
    a2dCanvasObject* m_parent;

    //! pins are world based or not.
    static bool m_worldBased;

    //! when set disables rending of pin objects
    static bool m_doRender;

    //! The class defines to which other pins this pin can connect
    a2dPinClass* m_pinclass;

    //! set when this pin is a temporary pin for editing feedback
    bool m_dynamicPin: 1;

    //! set when this pin is a temporary pin for editing feedback,
    //! this pin will be replaced by a normal pin or its flag will be reset, to make it so.
    bool m_temporaryPin: 1;

    //! flag to render or not when connected
    bool m_RenderConnected: 1;

    //! true when pin is internal to object border.
    bool m_internal: 1;

    //! is true when pin is on an object instead of a wire/connect
    bool m_objectPin: 1;

    //!to which a2dPin is this pin connected with other canvas objects its pins.
    /*!
        Added connections can be set to NULL or erased.
        When not possible to erase (several ieraters on the list),
        you can use CleanUpNonConnected() to clean the NULL items later on.
    */
    a2dlist< a2dDumbPtr<a2dPin> > m_connectedPins;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dPin( const a2dPin& other );
};

typedef a2dlist< a2dDumbPtr<a2dPin> > a2dPinList;

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
//! smart pointer for a2dPin
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dPin>;
#endif
typedef a2dSmrtPtr<a2dPin> a2dPinPtr;

//! This is a class/type description for a2dPin's
/*! a2dPin's have a pin class which can be used by a2dConnectionGenerator to define
    to which other pin classes ( of other pins ) it can connect and how.
    E.g. One class might be input and one class might be output and it is
    only possible to connect an input to an output, but not two inputs or two
    outputs.
    This class mainly has a name and an array of pin classes it is allowed to connect to.
    How and when a pinclass is connected to another pinclass ( one to one, or with a wire and what type of wire ),
    is decided by the a2dConnectionGenerator.

    As an example:
    You want pin object on certain layers to only connect to pins on some other layers. For that you give a pin
    on a layerA a a2dPinClass called PC_layerA. And for pins on layers B C D you do the same.
    Now to be able to connect a  PC_layerA to PC_layerB PC_layerC and NOT PC_layerD. You add to PC_layerA
    its m_canConnectTo, the pin classes: PC_layerA, PC_layerB, PC_layerC. And to PC_layerB, PC_layerC you add at least PC_layerA.
    You could also define a pin class called PC_layersABC with in its m_canConnectTo he same PC_layersABC,
    next you use for pins one layers A,B,C this class. So all pins on those ayesr can connect with each other.
    As you see what layer a pin is on, is of no importance, it is the pin class which allows pins to connect or not.



    Both the class and the m_canConnectTo array are created statically. If not,
    it is your responsibility to remove the canConnectTo array. The
    m_canConnectTo array is terminated with a zero pointer. The default
    class is a2dPinClass::Standard, which can only connect to itself.
*/

class A2DCANVASDLLEXP a2dPinClass
{
public:

    enum a2dPinClassFlags
    {
        /*! output */
        PC_angle = 0x0001,

        /*! For Xor ing and all set */
        PC_ALLSET  = 0xFFFFFFFF
    };

    //! constructor
    /*!
        \param name name of the pinclass, can be used e.g. for XML saving
    */
    a2dPinClass( const wxString& name );

    //! destructor
    ~a2dPinClass();

    //! add a connection pinclass, to which this pin can connect
    void AddConnect( a2dPinClass* pinClass ) { m_canConnectTo.push_back( pinClass ); }

    //! remove a connection pinclass
    void RemoveConnect( a2dPinClass* pinClass );

    //! called to initialize static stockobjects
    static void InitializeStockPinClasses();

    //! called to delete all pin class objects
    static void DeleteStockPinClasses();

    const wxString& GetName() const { return m_name; }

    //! is there an angle line on the pin
    bool HasAngleLine() const { return ( m_flags & PC_angle ) == PC_angle; }

    //! is there an angle line on the pin
    void SetAngleLine( bool value )
    {
        value ? ( m_flags = m_flags | PC_angle ) :
        m_flags = m_flags & ( PC_ALLSET ^ PC_angle );
    }

    //! get all flags
    wxUint32 GetFlags() { return m_flags; }

    //! searches in the connection list for a given pin class
    /*!
        If the given pin class is found, this means that the pin having that pinclass can connect to
        the pin having this pinclass.

        \param other pin to test connection
        \param flags type of pin needed
    */
    a2dPinClass*  CanConnectTo( a2dPinClass* other = NULL ) const;

    //! return list of connectable pin class objects.
    std::list< a2dPinClass* >& GetConnectList() { return m_canConnectTo; }

    //! return the name of this pinclass.
    static a2dPinClass* GetClassByName( const wxString& name );

    //! set default pin for graph like structure
    void SetPin( a2dPin* newpin );

    //! get default pin for graph like structure
    a2dPin* GetPin();

    //! \see  GetPinCanConnect()
    void SetPinCanConnect( a2dPin* newpin );

    //! represents the pin styles for pins that can connect right now
    /*! This pin is usually bigger than the default pin and green */
    a2dPin* GetPinCanConnect();

    //! \see  GetPinCannotConnect()
    void SetPinCannotConnect( a2dPin* newpin );

    //! represents the pin styles for pins that cannot connect right now
    /*! This pin is usually smaller than the default pin and red */
    a2dPin* GetPinCannotConnect();

    //! \see  GetParPin()
    void SetParPin( a2dParPin* newpin );

    //! Pin to use in a2dCameleonInst when creating a2dParPin from an a2dPort.
    a2dParPin* GetParPin();

    //! Set class for generating new connection objects between object and pins
    void SetConnectionGenerator( a2dConnectionGenerator* connectionGenerator ) { m_connectionGenerator = connectionGenerator; };

    //! Get class for generating new connection objects between object and pins
    a2dConnectionGenerator* GetConnectionGenerator() const { return m_connectionGenerator; }

    a2dPinClass* GetPinClassForTask( a2dConnectTask task, a2dCanvasObject* obj = NULL, a2dPinClass* pinClassTo = NULL, a2dPin* pinFrom = NULL ) const
    { return m_connectionGenerator->GetPinClassForTask( const_cast<a2dPinClass*>( this ), task, obj, pinClassTo, pinFrom ); }

protected:

    //! name of pin class
    wxString m_name;

    //! list of a2dPinClass objects, to which this a2dPinClass object can connect.
    //! Each Pin has a a2dPinClass which defines to which that pin can connect to other pins.
    //! This is approved based on those other Pins having a a2dPinClass in this array here.
    //! Of course another pin which wants to connect, should check the availibility of this pin its pinclass
    //! in its own connection list.
    std::list< a2dPinClass* > m_canConnectTo;

    //! pinclass specific flags
    wxUint32 m_flags;

    a2dPinPtr m_defPin;

    a2dPinPtr m_defCanConnectPin;

    a2dPinPtr m_defCannotConnectPin;

    a2dSmrtPtr<a2dParPin> m_defParPin;

    //! Use this connection generator for wires and pins
    a2dConnectionGeneratorPtr m_connectionGenerator;

public:

    //! a linked lists of all a2dPinClass'es, so that one can get a class by name
    static std::list< a2dPinClass* > m_allPinClasses;

    //! used to ask for a new default pin on an object.
    /*!
        m_connectionGenerator decides what is returned when asking a connection task given this pin class.
    */
    static a2dPinClass* Any;

    //! Pins of this class can only connect to pins of the same class
    static a2dPinClass* Standard;
};

#endif /* __WXCANPIN_H__ */

