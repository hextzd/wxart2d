/*! \file modules/editor/samples/wires/elements.h
    \author Erik van der Wal

    Copyright: 2004 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: elements.h,v 1.1 2007/01/17 21:35:23 titato Exp $
*/

#ifndef __ELEMENTSH__
#define __ELEMENTSH__

#include <wx/canvas/canmod.h>

// basic element class, to catch edit events and redirect them to text childs
class Element : public a2dCanvasObject
{
public:

    Element();

    Element( const Element& other, CloneOptions options, a2dRefMap* refs );

    //! clone this object
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

protected:
#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

public:

    wxString GetNetlistLine( wxString simulator );

    //! Get name of net connected to pin
    /*
        The name of the net is determined by a label connected to the net.
        If no label is connected, it will determine a net name, based on the
        components refdes and pin name. Eg. for a component Q1 with pin e
        the netname will become netQ1_e.
    */
    static wxString GetNetName( a2dPin* pin );

private:
    //! Private function to determine all pins connected to a net.
    static void GetNetPins(  a2dCanvasObjectList* pins, a2dPin* pin );

public:

    static a2dPropertyIdString* PROPID_refdes;
    static a2dPropertyIdString* PROPID_spice;
    static a2dPropertyIdString* PROPID_freeda;

    DECLARE_PROPERTIES()

private:
    DECLARE_DYNAMIC_CLASS( Element )
    DECLARE_EVENT_TABLE()
};

//! Label is used to add a name to a net.
/*! A label has a dedicated property PROPID_label, which can be used, to give a
    name to a certain net. Eg. the netname "0" is often used to indicate ground.
    This label can also be used in a a2dVisibleProperty, to allow the user
    to edit the label.
*/

class Label : public Element
{
public:
    //! Constructor
    Label();

    //! Constructor for a label with a predefined name.
    Label( const wxString& label );

    //! Copy constructor
    Label( const Label& other, CloneOptions options, a2dRefMap* refs );

    // Documented in base class.
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

protected:
#if wxART2D_USE_CVGIO
    // Documented in base class.
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class.
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

public:

    //! Property which contains the label name.
    static a2dPropertyIdString* PROPID_label;

    DECLARE_PROPERTIES()

private:
    DECLARE_DYNAMIC_CLASS( Label )
};

class Resistor : public Element
{
public:

    Resistor();

    Resistor( const Resistor& other, CloneOptions options, a2dRefMap* refs );

    Resistor(  const wxString& refdes ,  const wxString& val );

    //! clone this object
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    static a2dPropertyIdBool* PROPID_r;

    DECLARE_PROPERTIES()

protected:
#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

private:

    DECLARE_DYNAMIC_CLASS( Resistor )
};

class Capacitor : public Element
{
public:

    Capacitor();

    Capacitor( const Capacitor& other, CloneOptions options, a2dRefMap* refs );

    Capacitor(  const wxString& refdes,  const wxString& val );

    //! clone this object
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    static a2dPropertyIdString* PROPID_c;

    DECLARE_PROPERTIES()

protected:
#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

private:
    DECLARE_DYNAMIC_CLASS( Capacitor )
};

class Ground : public Label
{
public:

    Ground();

    Ground( const Ground& other, CloneOptions options, a2dRefMap* refs );

    Ground( const wxString& refdes );

    //! clone this object
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

protected:
#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( Ground )
};

class Input : public Label
{
public:

    Input();

    Input( const Input& other, CloneOptions options, a2dRefMap* refs );

    Input( const wxString& refdes );

    //! clone this object
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

protected:
#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( Input )
};


class Output : public Label
{
public:

    Output();

    Output( const Output& other, CloneOptions options, a2dRefMap* refs );

    Output( const wxString& refdes );

    //! clone this object
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

protected:
#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( Output )
};

//! pin specific for Resistor and Capacitor and Gnd objects.
class Pin: public a2dPin
{
    DECLARE_DYNAMIC_CLASS( Pin )

public:
    //! constructor
    Pin();

    //! constructor
    Pin( a2dCanvasObject* parent, const wxString& name, a2dPinClass* pinclass,  double xc, double yc, double angle = 0 , int w = 0, int h = 0,  int radius = 0 );

    //! copy constructor
    Pin( const a2dPin& other, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~Pin();

    //! clone this object
    a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    //! Pins of this class can only connect to ElementWire pins and ElementObject pins
    static a2dPinClass* ElementObject;
    //! Pins of this class can only connect to ElementObject pins
    static a2dPinClass* ElementWire;

    //! called to initialize static stockobjects
    static void InitializeExtraClasses();

protected:
    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    Pin( const a2dPin& other );
};


class Text: public a2dText
{
    DECLARE_DYNAMIC_CLASS( Text )
public:

    Text();

    Text( const Text& other, CloneOptions options, a2dRefMap* refs );

    Text( const wxString& text );

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

};

class ElementConnectionGenerator : public a2dConnectionGenerator
{

public:

    ElementConnectionGenerator();
    ~ElementConnectionGenerator();

    virtual a2dPinClass* GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj = NULL, a2dPinClass* pinClassTo = NULL, a2dPin* pinFrom = NULL ) const;

};


#endif // __ELEMENTSH__
