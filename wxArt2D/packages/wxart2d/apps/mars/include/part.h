/*! \file apps/mars/part.h
    \author Erik van der Wal

    Copyright: 2005 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: part.h,v 1.8 2006/12/13 21:43:21 titato Exp $
*/

#ifndef __PARTH__
#define __PARTH__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "object.h"
#include "document.h"

//----------------------------------------------------------------------------
// msPart
//----------------------------------------------------------------------------

//! Manipulated object of a symbolpage, stored in the locallib.
/*!
    A part is some kind of preprocessed symbol page.
    It will create a2dPin objects, as indicated by the msConnectorPins
    It will create msProperties when text object reference variables.
    It will create a2dCanvasVisibleProperties for text object referencing variables.
*/
class msPart : public msObject
{
public:
    //! Constructor.
    msPart();

    //! Copy constructor
    msPart( const msPart& other, CloneOptions options, a2dRefMap* refs );

    //! Constructor.
    msPart( msDocument* doc );

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    msObject* m_fixed;
    msObject* m_pinsandprops;
private:
    DECLARE_DYNAMIC_CLASS( msPart )
};

//----------------------------------------------------------------------------
// msComponent
//----------------------------------------------------------------------------

//! Circuitpage object
/*!
    A msComponent object is made from a msPart, residing in the locallib page.
    From the mspart, it will clone the pins and properties. For the actual graphics it will
    reference the part in the locallib.
*/
class msComponent : public msObject
{
public:
    //! Constructor.
    msComponent();

    //! Copy constructor
    msComponent( const msComponent& other, CloneOptions options, a2dRefMap* refs );

    //! Constructor.
    msComponent( msPart* part );

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS( msComponent )
};

//----------------------------------------------------------------------------
// msConnectorPin
//----------------------------------------------------------------------------

//! A graphical representation of a pin
/*!
    A connectorpin is a class for drawing pins. Do not confuse class this with a a2dPin.
    A connectorpin consists of at least two text objects, called label and number. Graphical objects
    can also be added, such as a line for a normal pin, but also more complex pin symbols,
    like a hysteresis symbol for digital inputs.
    The actual pin position will allways be (0, 0).

    Pins are drawn with a small red square around the origin in a symbol page, for better
    recogniton of the pin.
    \todo pin number/ label visibility
*/
class msConnectorPin : public msObject
{
public:
    //! Constructor.
    msConnectorPin();

    //! Constructor.
    msConnectorPin( double x, double y );

    //! Constructor.
    msConnectorPin( const wxString& number, double x = 0.0, double y = 0.0 );

    //! Constructor.
    msConnectorPin( const wxString& number, const wxString& label, double x = 0.0, double y = 0.0 );

    //! Copy constructor
    msConnectorPin( const msConnectorPin& other, CloneOptions options, a2dRefMap* refs );

    void Create( const wxString& number, const wxString& label );

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    void SetLabel( const wxString& label );
    void SetNumber( const wxString& number );
    wxString GetNumber();
    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

protected:
    a2dText* m_label;
    a2dText* m_number;

private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS( msConnectorPin )
};

#endif // __PARTH__
