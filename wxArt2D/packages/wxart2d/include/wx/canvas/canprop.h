/*! \file wx/canvas/canprop.h
    \brief properties specific for a2dCanvasOject
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canprop.h,v 1.20 2008/10/21 21:48:30 titato Exp $
*/

#ifndef __WXCANPROP_H__
#define __WXCANPROP_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/candefs.h"
#include "wx/canvas/layerinf.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/cantext.h"
#include "wx/canvas/polygon.h"

class A2DCANVASDLLEXP a2dDrawingPart;

//!Invisible Style property that can be added to Canvas Objects.
/*!
    Rendering is not done on this object.
    All style like object like a2dStroke and a2dFill
    are added to a2dCanvasObjects with this property.

    Fill Colour being the foreground color if filling with a monochrome bitmap
    and fill style is a2dFILL_STIPPLE_MASK_OPAQUE.
    Or in case of gradient filling the beginning colour for the fill

    You get transparent bitmap filling when bitmap fill is monochrome and fill style is
    a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT.
    The fill colour will be used for non Transparent parts in the fill bitmap,
    while other parts will be transparent.

    colour2 being the background color if filling with a monochrome bitmap
    and fill style is a2dFILL_STIPPLE_MASK_OPAQUE.
    Or in case of gradient filling the ending colour for the fill.

    \remark Fill Second color, used for Text, transparent filling and gradient filling.

    \remark style is inhereted from parent object.

    \remark an object may have style but is free to use it or not.

    \ingroup property style
*/
class A2DCANVASDLLEXP a2dStyleProperty: public a2dNamedProperty
{
public:

    a2dStyleProperty();

    a2dStyleProperty( const a2dPropertyId* id );

    a2dStyleProperty( const a2dStyleProperty& other );

    virtual ~a2dStyleProperty();

    a2dNamedProperty* Clone( a2dObject::CloneOptions options ) const;

    virtual void Assign( const a2dNamedProperty& other );

    //!if IsNoStroke() and IsNoFill() return true
    bool AllNo();

    //!Set a fill for the object which will be used instead of the layer fill
    //! returns a2dNullFILL set if no fill is used in which case the layer fill is used for filling
    inline const a2dFill& GetFill() const { return m_fill; }

    //!Set a stroke  for the object which will be used instead of the layer stroke
    //! returns a2dBLACK_STROKE with IsNoStroke() set, if no stroke is used in which case the layer fill is used for stroking
    inline const a2dStroke& GetStroke() const { return m_stroke; }

    //!Set a fill for the object which will be used instead of the layer fill
    /*!
        The fill is for filling the a2dCanvasObject.
        Use IsNoFill() to remove the fill from a a2dCanvasObject.
        Use a2dTRANSPARENT_FILL to not fill the object.
        \remark m_flags.m_filled flag overrules the fill to fill TRANSPARENT
    */
    void SetFill( const a2dFill& fill );

    //!Set a fill color for the object which will be used instead of the layer fill
    /*!
    \param fillcolor color to fill a2dCanvasObject with
    \param style style for one colour fill
    \remark creates a a2dOneColourFill internal
    \sa a2dOneColourFill
    */
    void SetFill( const wxColour& fillcolor, a2dFillStyle style = a2dFILL_SOLID );

    //!Set a fill color for the object which will be used instead of the layer fill
    /*!
    \param fillcolor color to fill a2dCanvasObject with
    \param fillcolor2 color to fill a2dCanvasObject with
    \param style style for one colour fill
    \remark creates a a2dTwoColourFill internal
    \sa a2dTwoColourFill
    */
    void SetFill(  const wxColour& fillcolor, const wxColour& fillcolor2, a2dFillStyle style = a2dFILL_SOLID );

    //!Set a stroke  for the object which will be used instead of the layer stroke
    void SetStroke(  const wxColour& strokecolor, float width = 0,  a2dStrokeStyle style = a2dSTROKE_SOLID );

    //!Set a stroke  for the object which will be used instead of the layer stroke
    void SetStroke(  const wxColour& strokecolor, int width ,  a2dStrokeStyle style = a2dSTROKE_SOLID );

    /*!
        The stroke is for drawing outlines of the a2dCanvasObject.
        Use NoStroke() to remove the stroke  from a a2dCanvasObject.
        Use a2dTRANSPARENT_STROKE to not fill the object.
    */
    void SetStroke( const a2dStroke& stroke );

    wxString StringRepresentation() const;
    wxString StringValueRepresentation() const;

    DECLARE_DYNAMIC_CLASS( a2dStyleProperty )

protected:

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    //!stroke used for outline of object.
    //! a2dNullSTROKE when no stroke
    a2dStroke    m_stroke;

    //!fill used for filling of object.
    //!If m_filled object is false the object will not be filled.
    //! a2dNullFILL when no fill
    a2dFill      m_fill;

private:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const { return new a2dStyleProperty( *this ); }
};

//! Creates a shadow behind a a2dCanvasObject when added as property.
/*! When this property is added to a a2dCanvasObject, that object is rendered
twice. The first time as a shadow behind the real shape.
Angle and Depth can be set for the Shadow.
The Shadow its style (colour etc.) is the style of the baseclass a2dStyleProperty.

    \ingroup property style
*/
class A2DCANVASDLLEXP a2dShadowStyleProperty: public a2dStyleProperty
{
public:

    a2dShadowStyleProperty();

    a2dShadowStyleProperty( const a2dPropertyIdCanvasShadowStyle* id, double depth, double angle = 30 );

    a2dShadowStyleProperty( const a2dShadowStyleProperty& other );

    virtual ~a2dShadowStyleProperty();

    a2dNamedProperty* Clone( a2dObject::CloneOptions options ) const;

    double GetExtrudeDepth() const { return m_depth; }

    double GetExtrudeAngle() const { return m_angle3d; }

    void SetExtrudeDepth( double depth )  { m_depth = depth; }
    void SetExtrudeAngle( double angle3d )  { m_angle3d = wxDegToRad( angle3d ); }

    DECLARE_DYNAMIC_CLASS( a2dShadowStyleProperty )

protected:

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    double m_depth;
    double m_angle3d;

private:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const { return new a2dShadowStyleProperty( *this ); }

};

//! property to hold a a2dCanvasObject pointer type variable to be associated with a canvasobject
/*!

    \ingroup property

*/
class A2DCANVASDLLEXP a2dCanvasObjectPtrProperty: public a2dNamedProperty
{
public:

    a2dCanvasObjectPtrProperty();

    a2dCanvasObjectPtrProperty( const a2dPropertyIdCanvasObject* id, a2dCanvasObject* object, bool visible = false, bool render = false );

    a2dCanvasObjectPtrProperty( const a2dCanvasObjectPtrProperty& other, a2dObject::CloneOptions options );

    virtual ~a2dCanvasObjectPtrProperty();

    virtual void Assign( const a2dNamedProperty& other );

    //! to set the a2dCanvasObject*
    void SetObject( wxObject* object );

    inline a2dCanvasObject* GetCanvasObject() const { return m_object; }

    //! to get the a2dCanvasObject*
    virtual a2dObject* GetRefObject() const { return m_object; }
    virtual a2dObject* GetRefObjectNA() const { return m_object; }

    DECLARE_DYNAMIC_CLASS( a2dCanvasObjectPtrProperty )

    //! the object is visible or not.
    /*! If a object is visible and it has a a2dObject internal,
        that object will rendered.
        A wxDocviewVisibleProperty uses this flag to make a object visible or not.
    */
    virtual void SetVisible( bool visible ) { m_visible = visible; }
    //! return true if the object is visible.
    virtual bool GetVisible() const { return m_visible; }

    //! if object can be rendered or not.
    /*! some properties have a reference to a a2dCanvasObject, but only to store it,
        not to render it as part of the object.
    */
    void SetCanRender( bool render ) { m_render = render; }
    //! return true if the object can be rendered.
    bool GetCanRender() const { return m_render; }

    //! is set true, the object within this property will be rendered before its parent a2dCanvasObject
    /*!
        \param prerender if true render m_object before its parent a2dCanvasObject, default false.
    */
    void SetPreRender( bool prerender ) { m_prerender = prerender; }

    //! returns the current prerender setting.
    bool GetPreRender() const { return m_prerender; }

    //! is set true, the object will only be rendered, if the owning object is selected
    /*!
        \param selectedonly if true, the object will only be rendered, if the owning object is selected
    */
    void SetSelectedOnly( bool selectedonly ) { m_selectedonly = selectedonly; }

    //! returns the current only selected setting.
    bool GetSelectedOnly() const { return m_selectedonly; }

    //! Get property value
    a2dCanvasObject* GetValue() const { return m_object; }

    //! Set property value
    void SetValue( a2dCanvasObject* newvalue ) { m_object = newvalue; }

    a2dCanvasObjectPtr* GetValuePtr() { return &m_object; }

    a2dCanvasObjectPtr& GetValueRef() { return m_object; }

    const a2dCanvasObjectPtr* GetValuePtr() const { return &m_object; }

    const a2dCanvasObjectPtr& GetValueRef() const { return m_object; }

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dCanvasObjectPtrProperty* CreatePropertyFromString( const a2dPropertyIdCanvasObject* id, const wxString& value );

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;
    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    //! \see SetPreRender()
    bool m_prerender;
    //! \see SetSelectedOnly()
    bool m_selectedonly;
    //! see SetVisible()
    bool m_visible;
    //! in case of a grahics application, should this property be rendered.
    bool m_render;

    //! when loading a document, this is used to resolve references to objects
    wxUint32 m_resolveid;

    //! object held by this property.
    a2dCanvasObjectPtr m_object;
};

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dPropertyIdProp<class a2dClipPathProperty>;
#endif
typedef a2dPropertyIdProp<class a2dClipPathProperty> a2dPropertyIdCanvasClipPath;

//!Clipping Path property that can be added to a2dCanvasObject's.
/*!Used during rendering to set a clipping path on a a2dCanvasObject.
   Everything outside the area defined by the clipping polygon is not drawn.

   \remark Currently only clipping object of type a2dPolygonL is supported.

   \remark the clipping object itself can also be rendered by adding it to the children of
   the object being clipped. Like any Object it has to be set visible to see it.

    \ingroup property style
*/
class A2DCANVASDLLEXP a2dClipPathProperty: public a2dNamedProperty
{
public:

    a2dClipPathProperty();

    a2dClipPathProperty( const a2dPropertyIdCanvasClipPath* id, a2dPolygonL* clip );

    a2dClipPathProperty( const a2dClipPathProperty& other, a2dObject::CloneOptions options );

    virtual ~a2dClipPathProperty();

    void Assign( const a2dNamedProperty& other );

    //! return a2dPolygonL that is used for clipping
    virtual a2dPolygonL* GetClipObject() const { return m_clip; }

    //! set object to use for clipping
    void SetCanvasObject( a2dPolygonL* clip );

    void PushClip( a2dDrawingPart* drawer, a2dBooleanClip clipoperation = a2dCLIP_AND );
    void PopClip( a2dDrawingPart* drawer );

    //! the object is visible or not.
    /*! If a object is visible and it has a a2dObject internal,
        that object will rendered.
        A wxDocviewVisibleProperty uses this flag to make a object visible or not.
    */
    virtual void SetVisible( bool visible ) { m_visible = visible; }
    //! return true if the object is visible.
    virtual bool GetVisible() const { return m_visible; }

    //! if object can be rendered or not.
    /*! some properties have a reference to a a2dCanvasObject, but only to store it,
        not to render it as part of the object.
    */
    void SetCanRender( bool render ) { m_render = render; }
    //! return true if the object can be rendered.
    bool GetCanRender() const { return m_render; }

    DECLARE_DYNAMIC_CLASS( a2dClipPathProperty )

    virtual a2dObject* GetRefObject() const;
    virtual a2dObject* GetRefObjectNA() const;

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;
    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    void DoRender( a2dIterC& ic, a2dAffineMatrix* cworld, OVERLAP clipparent, int layer , a2dCanvasObjectFlagsMask mask );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dSmrtPtr<a2dPolygonL> m_clip;
    //! see SetVisible()
    bool m_visible;
    //! in case of a grahics application, should this property be rendered.
    bool m_render;
};

//!(In)Visible property that can be added to Canvas Objects.
/*!
    This a2dCanvasObject makes it possible to display and edit properties, which are normally
    not visible as part of a a2dCanvasObject its propertylist.
    It has a a2dPropertyId member which points to a unique propertyID in a a2dCanvasObject.
    Normally this object is added as a child to the same a2dCanvasObject which contains the property.

    Rendering is done using a2dText object internal, and the text is taken from the property as
    its GetName() + " = " + StringValueRepresentation();
*/
class A2DCANVASDLLEXP a2dVisibleProperty: public a2dText
{
    DECLARE_EVENT_TABLE()

public:

    //! constructor
    a2dVisibleProperty();

    //! constructor copy
    a2dVisibleProperty( const a2dVisibleProperty& other, CloneOptions options, a2dRefMap* refs );

    //! constructor to set prop at x,y and angle
    a2dVisibleProperty( a2dCanvasObject* parent, const a2dPropertyId* property, double x, double y, double angle = 0 );

    //! constructor to set font and visibility also.
    a2dVisibleProperty( a2dCanvasObject* parent, const a2dPropertyId* property, double x, double y, bool visible = true,
                        const a2dFont& font = *a2dDEFAULT_CANVASFONT, double angle = 0 );

    virtual ~a2dVisibleProperty();

    //! Show both name and value of the property or just the value.
    void ShowName( bool show = false );

    //! to set the parent where the m_propId is searched for.
    void SetParent( a2dCanvasObject* parent );

    //! to get the parent where the m_propId is searched for.
    inline a2dCanvasObject* GetParent( void ) const { return m_parent; }

    //! get the id of the property displayed by this object
    inline const a2dPropertyId* GetPropId() { return m_propId; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( a2dCanvasObjectMouseEvent& event );

    void OnPropertyChanged( a2dComEvent& event );

    DECLARE_DYNAMIC_CLASS( a2dVisibleProperty );

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DoEndEdit();

    virtual bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //! object which contains m_propId
    a2dCanvasObject* m_parent;

    //! the property id for which the value needs to be displayed.
    const a2dPropertyId* m_propId;

    //! Indicates if both name and value of the property are visible, or just the value.
    bool m_showname;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dVisibleProperty( const a2dVisibleProperty& other );
};


//! property to hold a a2dBoundingBox type variable to be associated with a canvasobject
/*!
    \ingroup property
*/
class A2DCANVASDLLEXP a2dBoudingBoxProperty: public a2dNamedProperty
{
public:

    a2dBoudingBoxProperty();

    a2dBoudingBoxProperty( const a2dPropertyIdBoundingBox* id, const a2dBoundingBox& value );

    a2dBoudingBoxProperty( const a2dPropertyIdBoundingBox* id, const wxString& value );

    a2dBoudingBoxProperty( const a2dBoudingBoxProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dBoudingBoxProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dBoudingBoxProperty* CreatePropertyFromString( const a2dPropertyIdBoundingBox* id, const wxString& value );

    void SetValue( const a2dBoundingBox& value );

    a2dBoundingBox& GetValue() { return m_value; }

    a2dBoundingBox* GetValuePtr() { return &m_value; }

    const a2dBoundingBox& GetValue() const { return m_value; }

    const a2dBoundingBox* GetValuePtr() const { return &m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dBoudingBoxProperty )

protected:

    a2dBoundingBox m_value;

private:
    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;
};

//! Generate new tag for grouping
A2DCANVASDLLEXP a2dTag a2dNewTag();

//! property to hold a byte integer type vector to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DCANVASDLLEXP a2dTagVecProperty: public a2dNamedProperty
{
public:

    a2dTagVecProperty();

    a2dTagVecProperty( const a2dPropertyIdTagVec* id, a2dTagVec value );

    a2dTagVecProperty( const a2dTagVecProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dTagVecProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dTagVecProperty* CreatePropertyFromString( const a2dPropertyIdTagVec* id, const wxString& value );

    void SetValue( a2dTagVec value );

    a2dTagVec GetValue() const { return m_value; }

    a2dTagVec* GetValuePtr() { return &m_value; }

    a2dTagVec& GetValueRef() { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

    void PushTag( a2dTag tag );

    a2dTag PopTag();

    a2dTag Last() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dTagVecProperty )

protected:

    a2dTagVec m_value;

private:
    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;
};


#endif


