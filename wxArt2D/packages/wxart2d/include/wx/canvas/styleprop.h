/*! \file wx/canvas/styleprop.h
    \brief style properties that are specific to the canvas modules.
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: styleprop.h,v 1.15 2008/09/05 19:01:10 titato Exp $
*/

#ifndef __STYLEPROP_H__
#define __STYLEPROP_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/geometry.h"
#include "wx/artbase/afmatrix.h"
#include "wx/canvas/xmlpars.h"
#include "wx/canvas/canobj.h"
#include "wx/artbase/liner.h"
#include "wx/artbase/bbox.h"
#include "wx/artbase/stylebase.h"

class A2DARTBASEDLLEXP a2dDrawer2D;
class A2DCANVASDLLEXP a2dCanvasObject;
class A2DCANVASDLLEXP a2dIterC;

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dCanvasObject>;
#endif
typedef a2dSmrtPtr<a2dCanvasObject> a2dCanvasObjectPtr;

class A2DGENERALDLLEXP a2dFillProperty;
typedef a2dPropertyIdTyped< a2dFill, a2dFillProperty>  a2dPropertyIdFill;


//! property to hold a a2dFill type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dFillProperty: public a2dNamedProperty
{
public:

    a2dFillProperty();

    a2dFillProperty( const a2dPropertyIdFill* id, const a2dFill& value );

    a2dFillProperty( const a2dFillProperty* ori );

    a2dFillProperty( const a2dFillProperty& other );

    virtual a2dNamedProperty* Clone( a2dObject::CloneOptions options ) const;

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dFillProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dFillProperty* CreatePropertyFromString( const a2dPropertyIdFill* id, const wxString& value );

    void SetValue( const a2dFill& value );

    const a2dFill& GetValue() const { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

    bool NeedsSave();

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dFillProperty )

protected:

    a2dFill m_value;

private:
    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const { return new a2dFillProperty( *this ); }

};

class A2DGENERALDLLEXP a2dStrokeProperty;
typedef a2dPropertyIdTyped< a2dStroke, a2dStrokeProperty>  a2dPropertyIdStroke;


//! property to hold a a2dStroke type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dStrokeProperty: public a2dNamedProperty
{
public:

    a2dStrokeProperty();

    a2dStrokeProperty( const a2dPropertyIdStroke* id, const a2dStroke& value );

    a2dStrokeProperty( const a2dStrokeProperty* ori );

    a2dStrokeProperty( const a2dStrokeProperty& other );

    virtual a2dNamedProperty* Clone( a2dObject::CloneOptions options ) const;

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dStrokeProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dStrokeProperty* CreatePropertyFromString( const a2dPropertyIdStroke* id, const wxString& value );

    void SetValue( const a2dStroke& value );

    const a2dStroke& GetValue() const { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

    bool NeedsSave();

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dStrokeProperty )

protected:

    a2dStroke m_value;

private:
    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const { return new a2dStrokeProperty( *this ); }
};

class A2DGENERALDLLEXP a2dFontProperty;
typedef a2dPropertyIdTyped< a2dFont, a2dFontProperty>  a2dPropertyIdFont;


//! property to hold a a2dFont type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dFontProperty: public a2dNamedProperty
{
public:

    a2dFontProperty();

    a2dFontProperty( const a2dPropertyIdFont* id, const a2dFont& value );

    a2dFontProperty( const a2dFontProperty* ori );

    a2dFontProperty( const a2dFontProperty& other );

    virtual a2dNamedProperty* Clone( a2dObject::CloneOptions options ) const;

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dFontProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dFontProperty* CreatePropertyFromString( const a2dPropertyIdFont* id, const wxString& value );

    void SetValue( const a2dFont& value );

    const a2dFont& GetValue() const { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dFontProperty )

protected:

    a2dFont m_value;

private:
    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const { return new a2dFontProperty( *this ); }

};

//! To draw an object along the a2dCanvasObject its stroke.
/*!
    Any a2dCanvasObject can be used to draw along the outline
    of the a2dCanvasObject that is drawn.
    The distance between the object on the outline can be set.

    \ingroup property style
*/
/*
class A2DCANVASDLLEXP a2dCanvasObjectStroke : public a2dOneColourStroke
{
public:

    //!constructor
    a2dCanvasObjectStroke();

    //!constructor
    a2dCanvasObjectStroke( const a2dCanvasObjectStroke& stroke );

    //!give object for stroke
    a2dCanvasObjectStroke( a2dCanvasObject* object );

    //!width defined in world coordinates.
    a2dCanvasObjectStroke( a2dCanvasObject* object, const wxColour& col, float width = 0, a2dStrokeStyle style = a2dSTROKE_SOLID);

    //!width gets defined in pixels.
    a2dCanvasObjectStroke( a2dCanvasObject* object, const wxPen& stroke );

    //!width gets defined in pixels.
    a2dCanvasObjectStroke(  a2dCanvasObject* object, const wxColour& col, int width, a2dStrokeStyle style = a2dSTROKE_SOLID);

    //!Clone this object and return a pointer to the new object
    virtual a2dObject* Clone() const;

    //! destuctor
    ~a2dCanvasObjectStroke();

    float GetExtend() const;

    void Render( a2dIterC& ic, a2dDrawer2D* drawer, const a2dBoundingBox& clipobj, int n, wxPoint points[] );

    void SetDistance( double distance ) { m_distance = distance; }

    DECLARE_DYNAMIC_CLASS(a2dCanvasObjectStroke)

protected:

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    a2dCanvasObjectPtr m_obj;

    double m_distance;

};
*/
#endif /* __STYLEPROP_H__ */

