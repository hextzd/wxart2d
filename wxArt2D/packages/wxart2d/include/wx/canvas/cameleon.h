/*! \file wx/canvas/cameleon.h
    \brief special a2dCanvasObject to make a multi view hierachy.

    Hierarchy is normally created by adding child a2dCanvasObject's to the root object of the a2dDrawing,
    and those a2dCanvasObject's again can have child objects.
    A different way is to create a hierarchy of a2dDrawing's.
    The objects here have multiple appearances, where each appearance can be rendered differently.
    Some use a a2dDrawing to store and render information.
    The object defined here are all derived from a2dCanvasObject, and therefore can be placed in
    a a2dDrawing themselfs.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: recur.h,v 1.17 2009/09/26 20:40:32 titato Exp $
*/

#ifndef __WXCAMELEON_H__
#define __WXCAMELEON_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/candefs.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/recur.h"

class A2DCANVASDLLEXP a2dCameleon;
class A2DCANVASDLLEXP a2dDiagram;
class A2DCANVASDLLEXP a2dSymbol;
class A2DCANVASDLLEXP a2dParameters;
class A2DCANVASDLLEXP a2dCameleonInst;
class A2DCANVASDLLEXP a2dCameleonSymbolicRef;

#include <map>
typedef std::map< a2dPropertyIdPtr, a2dNamedPropertyPtr > a2dParameterMap;


//! Port to be used in a diagram for connecting to symbol
/*!
    This port object is used to create hierarchical diagrams, where a parent diagram can contains a sub diagram.
    This subdiagram is shown as a a2dSymbol or as a the a2dDiagram itself. The a2dCameleonInst instance in the
    parent diagram, defines a reference to an a2dAppearance in an a2dCameleon, which can be an a2dSymbol or a2dDiagram.
    a2dPort is used within a2dSymbol or a2dDiagram, to define the connecting pins at the parent level.
    The a2dPort is translated into an a2dParPin within each placed a2dCameleonInst instance. 
*/
class a2dPort : public a2dCanvasObject
{
public:

    //! Constructor.
    a2dPort();

    //! Constructor.
    /*!
		\param parent of port
        \param x x coordinates of pin on port
        \param y y coordinates of pin on port
        \param pinclass pinclass for the only pin
        \param parPinClass for a2dParpin used when creating cameleon instance from diagram
    */
    a2dPort( a2dCanvasObject* parent, double x, double y, const wxString& name = wxEmptyString, a2dPinClass* pinclass = a2dPinClass::Standard, a2dPinClass* parPinclass = a2dPinClass::Standard );

    //! Copy constructor
    a2dPort( const a2dPort& other, CloneOptions options, a2dRefMap* refs );

    ~a2dPort();

    //! get absolute angle of the pin ( after applying the parent its matrix and it own matrix )
    double GetAngle() const { return m_lworld.GetRotation(); }

    //! a2dPort normally has one a2dPin as child, return it here.
    a2dPin* GetPin() const;

    wxString GetName() { return m_name; }

    void SetName( const wxString& name ) { m_name = name; }

    //! set parent object of the pin
    virtual void SetParent( a2dCanvasObject* parent );

    //! get parent object of the pin
    a2dCanvasObject* GetParent() const { return m_parent; }

    static void Set( double l1, double l2, double l3, double b );

    // to temporarily disable rendering ( like when rendering from a2dCameleonInst )
    static void SetDoRender( bool doRender ) { m_doRender = doRender; }

    //! set pinclass to be used for a2dParPin to generate when making instances from a2dCameleons using this port
    void SetParPinClass( a2dPinClass* parPinClass ) { m_parPinClass = parPinClass; }

    //! get pinclass to be used for a2dParPin to generate when making instances from a2dCameleons using this port
    a2dPinClass* GetParPinClass() const { return m_parPinClass; }

    DECLARE_PROPERTIES()

protected:

	bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    wxString m_name;

    //! length from top of arrow to outside points in X
    static double m_l1;

    //! length from top of arrow to inside point in X
    static double m_l2;

    //! length from top of arrow to inside point in X
    static double m_l3;

    //! base of arrow
    static double m_b;

    static bool m_doRender;

    a2dPinClass* m_parPinClass;

	a2dCanvasObject* m_parent;

public:
    static a2dPropertyIdCanvasObject* PROPID_wasConnectedTo;

private:
    DECLARE_DYNAMIC_CLASS( a2dPort )
};

//! class to define a set of properties/parameters on derived classes.
class a2dHasParameters: public a2dCanvasObject
{
public:
    //! Constructor.
    a2dHasParameters( double x = 0, double y = 0 );

    //! Copy constructor
    a2dHasParameters( const a2dHasParameters& other, CloneOptions options, a2dRefMap* refs );

    ~a2dHasParameters();

    void AddStringParameter( const wxString& name, const wxString& value );

    void AddIntegerParameter( const wxString& name, wxInt32 value );

    void AddBoolParameter( const wxString& name, bool value );

    void AddDoubleParameter( const wxString& name, double value );

    void TakeParameters( a2dHasParameters* parInst ) const;

    void SetParameter( a2dPropertyId *dynproperty, a2dNamedProperty* namedproperty );

    a2dPropertyId* GetParameterId( const wxString &idName ) const;

    a2dNamedProperty* GetParameter( const wxString &idName ) const;

    a2dNamedProperty* GetParameter( a2dPropertyIdPtr id ) const;

    a2dParameterMap& GetParameters() { return m_parametermap; }

protected:

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    a2dParameterMap m_parametermap;

private:
    DECLARE_DYNAMIC_CLASS( a2dHasParameters )
};

//! a2dParPin points to a2dPort
/*!
    This pin is part of a a2dCameleonInst, and used to connect canvasobject with a2dPin's.
    The a2dParPin is synchronized with a a2dPort on a a2dSymbol or a2dDiagram.
    The a2dParPin refers to an a2dPort, and if the a2dPort is shifted or removed, the a2dCameleonInst is
    updated to reflect the change.
*/
class a2dParPin : public a2dPin
{
public:

    //! Constructor.
    a2dParPin();

    a2dParPin( a2dCameleonInst* parent, a2dPort* portPin, a2dPinClass* parPinClass );

    a2dParPin( a2dPinClass* parPinClass, double w, double h );

    //! Copy constructor
    a2dParPin( const a2dParPin& other, CloneOptions options, a2dRefMap* refs );

    ~a2dParPin();

    void SetPort( a2dPort* portPin ) { m_port = portPin; }

    a2dPort* GetPort() { return m_port; }

    bool LinkReference( a2dObject* other );

protected:

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    a2dSmrtPtr<a2dPort> m_port;

private:

    DECLARE_DYNAMIC_CLASS( a2dParPin )
};

typedef a2dSmrtPtr<a2dParPin> a2dParPinPtr;

//! Base class for adding to the list of appearances in a a2dCameleon
/*!
    An a2dCameleon can have various ways to look at it ( appearance ).
    Derive an appearance from here, to add to the a2dCameleon appearances list.
    An a2dCameleonInst, will point to one of the appearances in the a2dCameleon.
*/
class a2dAppear : public a2dCanvasObject
{
    DECLARE_EVENT_TABLE()

public:
    //! Constructor.
    a2dAppear( a2dCameleon* cameleon, double x = 0, double y = 0, bool offsetDrawing = true );

    //! Copy constructor
    a2dAppear( const a2dAppear& other, CloneOptions options, a2dRefMap* refs );

    ~a2dAppear();

    //! this appearance is for the returned a2dCameleon here.
    a2dCameleon* GetCameleon() { return m_cameleon; }

    //! this appearance is from this a2dCameleon
    void SetCameleon( a2dCameleon* cam ) { m_cameleon = cam; }

    //! next line is higher in Y if true or lower if false
    static void SetNextLineDirection( bool up ) { m_nextLine = up; }
    //! next line is higher in Y if true or lower if false
    static bool GetNextLineDirection() { return m_nextLine; }

    const a2dAffineMatrix& GetOffset() const { return m_offset; }

    void SetOffset( const a2dAffineMatrix& matrix ) { m_offset = matrix; }

    //! react on change from drawing.
    //! Default the event is redirected to m_cameleon.
    void OnChangeDrawings( a2dDrawingEvent& event );

protected:

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

protected:

    //! point to the a2dCameleon of which this appearance is a part.
    a2dCameleon* m_cameleon;

    a2dAffineMatrix m_offset;

    bool m_offsetDrawing;

	static bool m_nextLine;

    DECLARE_CLASS( a2dAppear )
};


//! a2dCameleonInst to show one appearance of an a2dCameleon.
/*!
    Holds a pointer to an instance of an a2dCameleon object.
    The a2dCameleon is the template for creating this instance.
    The appearance chosen of the a2dCameleon defines how the instance is shown.
    If the input is a a2dSymbol, the symbol of the a2dCameleon is used etc.
    The pins are of type a2dParPin, which will be automatically created from an a2dSymbol of the a2dCameleon,
    In case of a a2dDiagram refernce, they are based upon the a2dDiagram its a2dPort's.
    The rendering is done using the same a2dSymbol, or when referencing a diagram, rendering is
    as the a2dDiagram.
*/
class a2dCameleonInst : public a2dHasParameters
{
public:

    a2dCameleonInst( double x = 0, double y = 0, a2dSymbol* symbol = NULL );

    //! Constructor.
    a2dCameleonInst( double x , double y , a2dDiagram* diagram );

    a2dCameleonInst( double x , double y , const wxString& appearanceName );

    //! Copy constructor
    a2dCameleonInst( const a2dCameleonInst& other, CloneOptions options, a2dRefMap* refs );

    void Assign( const a2dCameleonInst& src );

    ~a2dCameleonInst();

    void SetAppearanceName( const wxString& appearanceName );

    void SetAppearance( a2dAppear* appearance );

    //! get referenced a2dCameleon
    a2dAppear* GetAppearance() { return m_appearance; }

    //! get referenced a2dCameleon
    a2dCameleon* GetCameleon();
   
    //! find equivalent a2dParPin voor the given a2dPort.
    a2dParPin* HasParPinForPort( a2dPort* pin );

	a2dParPin* HasParPinForPortByName( a2dPort* port );

    //! when resolving this refrence via a a2dIOHandler, this is used.
    virtual bool LinkReference( a2dObject* other );

    void MakeReferencesUnique();

    bool IsRecursive() { return true; }

    virtual a2dCanvasObject* PushInto( a2dCanvasObject* parent );

    //! gives time when an update on the a2dCameleon was processed
    /*!
        Comparing this time with a a2dCameleon its own update time, one can easily keep them up to date.
    */
    wxDateTime& GetCameleonUpdateTime() { return m_CameleonUpdateTime; }

    //! make the update time Now
    void TriggerCameleonUpdateTime() { m_CameleonUpdateTime = wxDateTime::Now(); }

    void DependencyPending( a2dWalkerIOHandler* handler );

    a2dPort* HasPort( a2dDrawing* drawing, a2dParPin* parPin );

    //! synchronize pins here to the a2dPorts in the drawing.
    void SyncPinsTo( a2dDrawing* drawing, const a2dAffineMatrix& matrix, bool byname = false );

    //! bring the contents in a2dSymbol or a2dDiagram to parent.
    void Flatten( a2dCanvasObject* parent );

protected:

    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

#if wxART2D_USE_CVGIO
    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );

    void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    //! points to an appearance in a2dCameleon
    a2dSmrtPtr<a2dAppear> m_appearance;

    a2dSmrtPtr<class a2dCameleon> m_cameleon;

    //! if m_appearance is not set, this is used to establish link
    wxString m_appearanceName;

    //! gives time when a change was made to the object which effects others 
    /*!
    */
    wxDateTime m_CameleonUpdateTime;

private:
    DECLARE_DYNAMIC_CLASS( a2dCameleonInst )
};

//! diagram is an appearance for a2dCameleon
/*!
    An a2dDiagram appearance needs to be added to a a2dCameleon.
    A diagram appearance is in itself a drawing, which defines a diagram in its childobjects.
    It can have a2dPorts, to connect the diagram at a higher level.

    In a typical diagram application, the a2dDiagram drawing would be used to draw a diagram.
    The a2dDiagram in a2dCameleon dfined the diagra, while an a2dSymbol in the same a2dCameleon, defines
    the appearance at a higher level.
    At a higher level a2dSymbol is used to present the a2dCameleon using an a2dCameleonInst instance.
    A higher level diagram can consist of connected a2dSymbol's. Connections made by a2dParPin objects.
    But any other a2dCanvasObject object can be part of the drawing.
    In a hierarchycal diagram, each diagram has in and out a2dPort objects,
    which are synchronized with the a2dSymbol being part of the same a2dCameleon.
*/
class a2dDiagram : public a2dAppear
{
    DECLARE_EVENT_TABLE()

public:

    a2dDiagram();

    //! Constructor.
    a2dDiagram( a2dCameleon* cameleon, double x = 0, double y = 0 );    

    //! Copy constructor
    a2dDiagram( const a2dDiagram& other, CloneOptions options, a2dRefMap* refs );

    ~a2dDiagram();

    //! a2dPort objects result in pins for the instance
    /*!
        The most left up port is used as reference.
        So when filling the a2dCameleonInst, that port is used to create a a2dParPin at 0.0,
        and all other a2dParPin's are placed relative to that.
        If no port is available, 0,0 is used.
    */
    void TakePortsTo( a2dCameleonInst* parInst );

    //! a2dPort objects in diagram are used to create an equivalent symbol
    void CreateSymbol( a2dDrawing* symbolDrawing, double x = 0, double y = 0 );

    //! find the port in the diagram that is most left up.
    /*! 
        This port will be used to modify origin to this port its point diagram drawing.
    */
    a2dPort* FindLeftUp() const;

    //* return a2dOrigin object if found else NULL
    a2dOrigin* FindOrigin() const;

    //! origin of drawing is shifted to the pos given.
    void TranslateTo( double dx, double dy );

    //! search a2dPort in this a2dDiagram with same name as input a2dPort (coming from e.g a a2dSymbol)
    a2dPort* FindPort( a2dPort* symPin ) const;

    //! search a2dPort in a2dDiagram with same name in a2dParPin (coming from a a2dCameleonInst)
    a2dPort* FindPort( a2dParPin* parPin ) const;

    //! find a2dPort with given name.
    a2dPort* FindPortByName( const wxString& parPinName ) const;

    a2dDrawing* GetDrawing() const { return m_diagramDrawing; }

    a2dCanvasObject* PushInto( a2dCanvasObject* parent );

    void AppendToDrawing( a2dCanvasObject* obj ) { m_diagramDrawing->GetRootObject()->Append( obj ); }   

    virtual void DependencyPending( a2dWalkerIOHandler* handler );

    a2dBoundingBox GetUnTransformedBboxNoPorts() const;

    //! take visible parameters instances into a pameterized instance, to make them unique per instance.
    void TakeVisibleParameters( a2dCameleonInst* parInst );

    void CalculateOffset();

protected:

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    void OnUpdateFromDrawing( a2dDrawingEvent& event );

#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    a2dSmrtPtr<a2dDrawing> m_diagramDrawing;

private:
    DECLARE_DYNAMIC_CLASS( a2dDiagram )
};

typedef a2dDiagram a2dLayout;

//! Symbolic appearance for a2dCameleon
/*!
    Symbolic appearance to be added to a a2dCameleon.
    A symbolic appearance is in itself a drawing, defined in its childobjects.
    It can have a2dPorts, which will become pins in a a2dCameleonInst.
    In a typical diagram application, this a2dSymbol drawing would be used to present the a2dCameleon 
    using a a2dCameleonInst instance in a drawing.
*/
class a2dSymbol : public a2dDiagram
{
public:

    //! Constructor.
    a2dSymbol();

    ~a2dSymbol();

    //! Constructor.
    a2dSymbol( a2dCameleon* cameleon, double x = 0, double y = 0 );

    //! Copy constructor
    a2dSymbol( const a2dSymbol& other, CloneOptions options, a2dRefMap* refs );

protected:

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DoRender( a2dIterC& ic, OVERLAP clipparent );


private:
    DECLARE_DYNAMIC_CLASS( a2dSymbol )
};

//! 
/*!
*/
class a2dBuildIn : public a2dAppear
{
    DECLARE_EVENT_TABLE()

public:

    a2dBuildIn();

    //! Constructor.
    a2dBuildIn( a2dCameleon* cameleon, double x = 0, double y = 0, a2dCanvasObject* buildIn = NULL );

    //! Copy constructor
    a2dBuildIn( const a2dBuildIn& other, CloneOptions options, a2dRefMap* refs );

    ~a2dBuildIn();

    a2dCanvasObject* GetBuildIn() const;

    void SetBuildIn( a2dCanvasObject* buildIn );

    a2dCanvasObject* PushInto( a2dCanvasObject* parent );

protected:

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    void OnUpdateFromDrawing( a2dDrawingEvent& event );

#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    a2dSmrtPtr<a2dDrawing> m_drawing;

private:
    DECLARE_DYNAMIC_CLASS( a2dBuildIn )
};


//! parameters appearance for a2dCameleon
/*!
    parameters appearance to be added to a a2dCameleon.
    It can edit and show the parameters as stored in a a2dCameleon.
*/
class a2dParameters : public a2dAppear
{
public:
    //! Constructor.
    a2dParameters( a2dCameleon* cameleon = NULL, double x = 0, double y = 0 );

    //! Copy constructor
    a2dParameters( const a2dParameters& other, CloneOptions options, a2dRefMap* refs );

protected:

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

private:
    DECLARE_DYNAMIC_CLASS( a2dParameters )
};

//! user interface appearance for a2dCameleon
/*!
    To define a Graphical user interface to set parameters on a a2dCameleon.
*/
class a2dGui : public a2dAppear
{
public:
    //! Constructor.
    a2dGui( a2dCameleon* cameleon = NULL, double x = 0, double y = 0 );

    //! Copy constructor
    a2dGui( const a2dGui& other, CloneOptions options, a2dRefMap* refs );

protected:

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

private:
    DECLARE_DYNAMIC_CLASS( a2dGui )
};

//! object to show several appearance views on what it contains
/*!
    Like a cameleon, this object changes with its environment.
    Its appearance differs depending on the context in which it is used.

    An a2dCameleonInst is used to refernce a a2dCameleon in a higher level drawing.
    The a2dCameleonInst will reference a specific appearance within the a2dCameleon.

    The parameters is a list of properties, defining the input parameters to a a2dCameleon.
    The appearances of an a2dCameleon may depend on its parameters.

    Visible parameters are displayed in an a2dSymbol, and synchronized with the a2dSymbol 
    being part of the same a2dCameleon.

    a2dCameleon itself can also be placed in a drawing, but the rendering will be as defined in DoRender().
    The a2dAppear objects are placed in the a2dDrawing m_appearances.
    Pushing into a a2dCameleon, will show that drawing.

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dCameleon: public a2dHasParameters
{
public:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS( a2dCameleon )

    a2dCameleon( const wxString& name = wxT(""), double x = 0, double y = 0, a2dHabitat* habitat = NULL );

    a2dCameleon( const a2dCameleon& other, CloneOptions options, a2dRefMap* refs );

    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    ~a2dCameleon();

    //!get the name given to the handle.
    wxString GetName() const { return m_name; }

    //! set the name of the handle (must be unique)
    void SetName( const wxString& name ) { m_name = name; }

    a2dHabitat* GetHabitat() const { return m_habitat; }

    void SetHabitat( a2dHabitat* habitat ) { m_habitat = habitat; }

    //! set special id to set for type of drawing this fits
    a2dDrawingId GetDrawingId() { return m_drawingId; }
    
    //! set special id to set for type of drawing this fits
    void SetDrawingId( a2dDrawingId id ) { m_drawingId = id; }

	//! template means this is placed as its contents (flattened) and not as a block with hierarchy.
	bool IsTemplate() const { return m_isTemplate; }

	//! template means this is placed as its contents (flattened) and not as a block with hierarchy.
	void SetTemplate( bool isTemplate ) { m_isTemplate = true; }

	//! this is used as a multireferenced block (e.g. subroutine)
	bool IsMultiRef() const { return m_isMultiRef; }

	//! this is used as a multireferenced block (e.g. subroutine)
	void SetMultiRef( bool multiRef ) { m_isMultiRef = true; }

    a2dDrawing* GetAppearances() const { return m_appearances; }

    void AddAppearance( a2dAppear* appearance );

    a2dCanvasObject* PushInto( a2dCanvasObject* parent );

    OVERLAP GetClipStatusData( a2dAppear* appearance, a2dIterC& ic, OVERLAP clipparent ) const;

    void RenderData( a2dAppear* appearance, a2dIterC& ic, OVERLAP clipparent );

    bool IsHitWorldData( a2dAppear* appearance, a2dIterC& ic, a2dHitEvent& hitEvent );

    a2dBoundingBox GetUnTransformedBboxData( a2dAppear* appearance, a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool UpdateData( a2dAppear* appearance, UpdateMode mode );

    //! Get a specific a2dAppear derived class instance from here.
    template < typename Tappear >
    Tappear* GetAppearance( bool autoCreate = false )
    {
        Tappear* find;
        a2dCanvasObjectList::iterator iter = m_appearances->GetRootObject()->GetChildObjectList()->begin();
        while ( iter != m_appearances->GetRootObject()->GetChildObjectList()->end() )
        {
            find = wxDynamicCast( ( *iter ).Get(), Tappear );
            if ( find )
                return find;
            iter++;
        }

        // No locallib page present yet, so create it.
        if ( autoCreate )
        {
            find = new Tappear( this );
            find->SetPosXyPoint( GetSuitablePointForNewAppearance() );
            m_appearances->GetRootObject()->Append( find );
        }
        return find;
    }

    //! get appearance by its name
    a2dAppear* GetAppearanceByName( const wxString& name ) const;

    //! get appearance by its Classname
    a2dAppear* GetAppearanceByClassName( const wxString& appearranceClassName ) const;

    a2dDiagram* GetDiagram( bool autoCreate = false );
    a2dSymbol* GetSymbol( bool autoCreate = false );
    a2dBuildIn* GetBuildIn( bool autoCreate = false );
    a2dGui* GetGui( bool autoCreate = false );

    //! gives time when a change was made to the cameleon which may effect rendering at a higher level
    /*!
        Comparing this time in a2dCameleonInst its own update time, one can easily keep them up to date.

        e.g. set in SetUpdatesPending()
    */
    wxDateTime& GetInternalChangedTime() { return m_changedInternalAccesstime; }

    //! make the changed time Now
    void TriggerChangedTime() { m_changedInternalAccesstime = wxDateTime::Now(); }

    //! return the root where all a2dCameleon's are stored
    static a2dCanvasObject* GetCameleonRoot() { return ms_centralCameleonRoot; }

    //! set the root where all a2dCameleon's are stored
    static void SetCameleonRoot( a2dCanvasObject* cameleonRoot ) { ms_centralCameleonRoot = cameleonRoot; }

    static a2dCameleon* HasInLocalLibrary( const wxString& name );

    //! add this to ms_centralCameleonRoot at a suitable position
    //! In a document framework setting, it is normally set to m_rootObject of the a2dDrawing in the current a2dCanvasDocument.
    //! All a2dCameleons can be accessed from the a2dCanvasDocument directly.
    void AddToRoot( bool autoPlace = true );

    //! x + dx position of next a2dCameleon;
    static void SetLocationDeltaX( double dx ) { ms_dx = dx; }

    //! y + dy position of next a2dCameleon;
    static void SetLocationDeltaY( double dy ) { ms_dy = dy; }

    //! x + dx position of next a2dCameleon;
    static double GetLocationDeltaX() { return ms_dx; }

    //! y + dy position of next a2dCameleon;
    static double GetLocationDeltaY() { return ms_dy; }

    //! Get a suitable location for a new a2dCameleon();
    static a2dPoint2D GetSuitablePoint();

    a2dPoint2D GetSuitablePointForNewAppearance() const;

    //! next line is higher in Y if true or lower if false
    static void SetNextLineDirection( bool up ) { m_nextLine = up; }
    //! next line is higher in Y if true or lower if false
    static bool GetNextLineDirection() { return m_nextLine; }

    const a2dBoundingBox& GetDataBoundingBox() const { return m_dataBbox; }

    DECLARE_PROPERTIES()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    //! track modification of document
    void OnDoEvent( a2dCommandProcessorEvent& event );

    //! track modification of document
    void OnRedoEvent( a2dCommandProcessorEvent& event );

    //! track modification of document
    void OnUndoEvent( a2dCommandProcessorEvent& event );

    //! called when a drawing in a document did change.
    void OnChangeDrawings( a2dDrawingEvent& event );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dSmrtPtr<a2dDrawing> m_appearances;

    //! gives time when a change was made to the object which effects others e.g. a2dCameleonInst
    /*!
    */
    wxDateTime m_changedInternalAccesstime;

    //! when set, a new a2dCameleon object will be added here also.
    //! In a document framework setting, it is normally set to m_rootObject of the a2dDrawing in the current a2dCanvasDocument.
    //! All a2dCameleons can be accessed from the a2dCanvasDocument directly.
    static a2dCanvasObject* ms_centralCameleonRoot;

    static double ms_y;
    //! dx position for next a2dCameleon;
    static double ms_dx;
    //! dy position for next a2dCameleon;
    static double ms_dy;

    static bool m_nextLine;
    
    a2dHabitat* m_habitat;  

    a2dDrawingId m_drawingId;

	bool m_isTemplate;

	bool m_isMultiRef;

    a2dBoundingBox m_dataBbox;

    //! name
    wxString m_name;
};

typedef a2dSmrtPtr<a2dCameleon> a2dCameleonPtr;

//!(In)Visible parameters that can be added to Canvas Objects.
/*!
    This a2dCanvasObject makes it possible to display and edit parameters, which are normally
    not visible as part of a a2dCameleon its parameters map.
    It has a a2dPropertyId member which points to a unique parameter in a a2dCameleon.
    Normally this object is added as a child to the a a2dCanvasObject.

    Rendering is done using a2dText object internal, and the text is taken from the parameter as
    its GetName() + " = " + StringValueRepresentation();
*/
class A2DCANVASDLLEXP a2dVisibleParameter: public a2dText
{
    DECLARE_EVENT_TABLE()

public:

    //! constructor
    a2dVisibleParameter();

    //! constructor copy
    a2dVisibleParameter( const a2dVisibleParameter& other, CloneOptions options, a2dRefMap* refs );

    //! constructor to set prop at x,y and angle
    a2dVisibleParameter( a2dHasParameters* parent, a2dPropertyIdPtr property, double x, double y, double angle = 0 );

    //! constructor to set font and visibility also.
    a2dVisibleParameter( a2dHasParameters* parent, a2dPropertyIdPtr property, double x, double y, bool visible = true,
                        const a2dFont& font = *a2dDEFAULT_CANVASFONT, double angle = 0 );

    virtual ~a2dVisibleParameter();

    //! Show both name and value of the property or just the value.
    void ShowName( bool show = false );

    //! to set the parent where the m_propId is searched for.
    void SetParent( a2dHasParameters* parent );

    //! to get the parent where the m_propId is searched for.
    inline a2dHasParameters* GetParent( void ) const { return m_parent; }

    //! get the id of the property displayed by this object
    inline const a2dPropertyId* GetPropId() { return m_propId; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( a2dCanvasObjectMouseEvent& event );

    void OnPropertyChanged( a2dComEvent& event );

    DECLARE_DYNAMIC_CLASS( a2dVisibleParameter );

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DoEndEdit();

    virtual bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //! object which contains m_propId
    a2dHasParameters* m_parent;

    //! the property id for which the value needs to be displayed.
    a2dPropertyIdPtr m_propId;

    //! Indicates if both name and value of the property are visible, or just the value.
    bool m_showname;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dVisibleParameter( const a2dVisibleParameter& other );
};

//! a2dCameleonSymbolicRef
/*!
*/
class a2dCameleonSymbolicRef : public a2dCanvasObject
{
public:

    a2dCameleonSymbolicRef( double x, double y, a2dCameleon* cameleon );
    
    a2dCameleonSymbolicRef( double x, double y, a2dSymbol* symbol );

    //! Constructor.
    a2dCameleonSymbolicRef( double x , double y , a2dDiagram* diagram );

    a2dCameleonSymbolicRef( double x=0 , double y=0 , const wxString& appearanceName = wxEmptyString );

    //! Copy constructor
    a2dCameleonSymbolicRef( const a2dCameleonSymbolicRef& other, CloneOptions options, a2dRefMap* refs );

    ~a2dCameleonSymbolicRef();

    void SetAppearanceName( const wxString& appearanceName );

    //! get referenced a2dCameleon
    a2dAppear* GetAppearance() { return m_appearance; }

    //! get referenced a2dCameleon
    a2dCameleon* GetCameleon();

    virtual a2dCanvasObject* PushInto( a2dCanvasObject* parent );

    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

	bool LinkReference( a2dObject* linkto );

    //! next line is higher in Y if true or lower if false
    static void SetNextLineDirection( bool up ) { m_nextLine = up; }
    //! next line is higher in Y if true or lower if false
    static bool GetNextLineDirection() { return m_nextLine; }

protected:

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

	bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

#if wxART2D_USE_CVGIO
    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );

    void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    //! points to an appearance in a2dCameleon
    a2dSmrtPtr<a2dAppear> m_appearance;

    a2dSmrtPtr<class a2dCameleon> m_cameleon;

    //! if m_appearance is not set, this is used to establish link
    wxString m_appearanceName;
    wxString m_appearanceClassName;

    static bool m_nextLine;

private:
    DECLARE_DYNAMIC_CLASS( a2dCameleonSymbolicRef )
};


//!===================== commands for cameleons =========================

//! objects which fit the mask are grouped into a new object
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_CreateCameleonMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    enum a2dCameleonTask
    {
        //! Create symbol from masked objects
        CreateSymbol, 
        //! Create diagram from masked objects
        CreateDiagram,
        //! Create template diagram from masked objects
        CreateTemplateDiagram,
        //! Create symbol and diagram from masked objects
        CreateSymbolDiagram
    };

    a2dCommand_CreateCameleonMask( a2dCanvasObject* parent, const wxString& name = wxEmptyString, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED, a2dCameleonTask task = CreateSymbol ): a2dCommand( true, Id )
    {
        m_name = name;
        m_task = task;
        m_parent = parent;
        m_mask = mask;
        m_objects = wxNullCanvasObjectList;
    }

    ~a2dCommand_CreateCameleonMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
    }

    bool Do( void );

    bool Undo( void );

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

    wxString m_name;
    a2dCameleonTask m_task;
    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dSmrtPtr<a2dCameleon> m_groupobject;
    a2dSmrtPtr<a2dCameleonInst> m_camref;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const
    {
        return new a2dCommand_CreateCameleonMask( m_parent, m_name, m_mask );
    }

};

//! objects which fit the mask are grouped into a new object
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_FlattenCameleonMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_FlattenCameleonMask( a2dCanvasObject* parent, bool deep = false, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_deep = deep;
        m_objects = wxNullCanvasObjectList;
    }

    ~a2dCommand_FlattenCameleonMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
    }

    bool Do( void );

    bool Undo( void );

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

    a2dCanvasObjectFlagsMask m_mask;
    bool m_deep;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const
    {
        return new a2dCommand_FlattenCameleonMask( m_parent, m_deep, m_mask );
    }

};

//! objects which fit the mask are grouped into a new object
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_CloneCameleonFromInstMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_CloneCameleonFromInstMask( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_objects = wxNullCanvasObjectList;
        m_newobjects = wxNullCanvasObjectList;
    }

    ~a2dCommand_CloneCameleonFromInstMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
        if ( m_newobjects != wxNullCanvasObjectList )
            delete m_newobjects;
    }

    bool Do( void );

    bool Undo( void );

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dCanvasObjectList* m_newobjects;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const
    {
        return new a2dCommand_CloneCameleonFromInstMask( m_parent, m_mask );
    }
};



BEGIN_DECLARE_EVENT_TYPES()
	//!see a2dCameleonEvent \ingroup eventid
	DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, wxEVT_NEW_CAMELEON, 1 )
END_DECLARE_EVENT_TYPES()

//! used to report a2dCameleon events
/*!
    The event Object is a a2dCameleon.

    \ingroup events

*/
class A2DCANVASDLLEXP a2dCameleonEvent : public a2dEvent
{
public:
 
    //! for event types
    /*!
        ::wxEVT_REPORT_VIEWS
        Sent from a2dDocument to a2dView's. a2dView will add itself to the reportlist of the event, but only
        when this document is indeed used by the a2dView.
        Register your wxEvtHandler to recieve it.
    */
    a2dCameleonEvent( a2dDrawing* drawing, a2dCameleon* cameleon )
        : a2dEvent( 0, wxEVT_NEW_CAMELEON )
    {
        SetEventObject( drawing );
		m_cameleon = cameleon;
    }

    //! constructor
    a2dCameleonEvent( const a2dCameleonEvent& event )
        : a2dEvent( event )
    {
    }
 
    virtual wxEvent* Clone() const { return new a2dCameleonEvent( *this ); }

    a2dCameleon* GetCameleon() { return m_cameleon; }

	a2dDrawing* GetDrawing() { return (a2dDrawing*) GetEventObject(); } ;
private:

	a2dCameleon* m_cameleon;
};

typedef void ( wxEvtHandler::*a2dCameleonEventFunction )( a2dCameleonEvent& );
#define a2dCameleonEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(a2dCameleonEventFunction, &func)

//! event from a document when its views need to be updated
#define EVT_NEW_CAMELEON(func)  wx__DECLARE_EVT0( wxEVT_NEW_CAMELEON,  a2dCameleonEventHandler( func ))


//! specialized command using the Assign method of a2dCommand_CameleonInst to transfer changes in editcopy to original.
class A2DCANVASDLLEXP a2dCommand_CameleonInst: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_CameleonInst() { m_target = NULL; }
    a2dCommand_CameleonInst( a2dCameleonInst* target, a2dCameleonInst* objectWithChanges );
    ~a2dCommand_CameleonInst( void );

    bool Do();
    bool Undo();

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

protected:
    a2dSmrtPtr<a2dCameleonInst> m_target;
    a2dSmrtPtr<a2dCameleonInst> m_objectchanges;
};


//! used to replace a a2dCameleonInst with another
/*!

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ReplaceCameleonInst: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_ReplaceCameleonInst() {}

    a2dCommand_ReplaceCameleonInst( a2dCameleonInst* camold, a2dCameleonInst* camnew );

    ~a2dCommand_ReplaceCameleonInst( void );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

protected:
    a2dSmrtPtr<a2dCameleonInst> m_cam1;
    a2dSmrtPtr<a2dCameleonInst> m_cam2;
};


#endif
