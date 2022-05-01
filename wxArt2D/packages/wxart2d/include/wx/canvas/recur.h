/*! \file wx/canvas/recur.h
    \brief special a2dCanvasObject which are used to create hierarchy different from
    the standard a2dCanvasObject child list.

    Hierarchy is normally created by adding child a2dCanvasObject's to the root object of the document,
    and next to those a2dCanvasObject's again add child objects.
    But the object here create different types of hierarchy.

    a2dCanvasObjectReference refrences another a2dCanvasObject which might be stored as a child of
    another one already. This way you can display the same object at several places.

    a2dCanvasObjectArrayReference is simular, but here the referenced object is displayed in
    a grid of n by M objects.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: recur.h,v 1.17 2009/09/26 20:40:32 titato Exp $
*/

#ifndef __WXRECUR_H__
#define __WXRECUR_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canprim.h"
#include "wx/canvas/canimage.h"
#include "wx/canvas/cantext.h"

#define __USE_EVENTS  1

#define newline wxString("\n")

//!a2dCanvasObjectReference is a reference to any a2dCanvasObject derived class.
/*!
It does not Clone the referenced object.
It has a matrix to reposition/rotate/scale the object it references.
The position/matrix of the referenced Object is accumulated with the one here.

An object Referenced will be rendered if it has the right layer and the reference
itself is visible. The Object its own visible flag is ignored.

The selected flag of the Object referenced is overruled by the Reference.

    \ingroup canvasobject

*/
class A2DCANVASDLLEXP a2dCanvasObjectReference: public a2dCanvasObject
{
public:

    a2dCanvasObjectReference();

    a2dCanvasObjectReference( double x, double y, a2dCanvasObject* obj );

    ~a2dCanvasObjectReference();

    a2dCanvasObjectReference( const a2dCanvasObjectReference& other, CloneOptions options, a2dRefMap* refs );

    //!\return the object that is referenced
    a2dCanvasObject* GetCanvasObject();

    //! set the object that is referenced
    void SetCanvasObject( a2dCanvasObject* object );

    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    //! when resolving this refrence via a a2dIOHandler, this is used.
    virtual bool LinkReference( a2dObject* other );

    void MakeReferencesUnique();

    DECLARE_DYNAMIC_CLASS( a2dCanvasObjectReference )

    bool IsRecursive() { return true; }

    void RemoveHierarchy();

    virtual a2dCanvasObject* PushInto( a2dCanvasObject* parent );

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    virtual void DoAddPending( a2dIterC& ic );

#if wxART2D_USE_CVGIO
    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );

    void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    a2dCanvasObjectPtr m_object;

    static const a2dPropertyIdCanvasObject PROPID_refobject;

private:

    virtual void DoWalkerWithContext( a2dIterC& ic, wxObject* parent, a2dWalkerIOHandlerWithContext& handler );

    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCanvasObjectReference( const a2dCanvasObjectReference& other );

};

//!a2dCanvasObjectArrayReference is an array of a reference to any a2dCanvasObject derived class.
/*!
    Places the refernced a2dCanvasObject in an array of x elemnts wide and y elements heigh.

    It does not Clone the referenced object.

    The a2dCanvasObject::m_lworld can be used to reposition/rotate/scale the array.

    The position/matrix of the referenced Object is accumulated with the poistion in the array.

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dCanvasObjectArrayReference: public a2dCanvasObjectReference
{
public:

    a2dCanvasObjectArrayReference();

    a2dCanvasObjectArrayReference( double x, double y, int rows, int columns, double horzSpace, double vertSpace, a2dCanvasObject* obj );

    ~a2dCanvasObjectArrayReference();

    a2dCanvasObjectArrayReference( const a2dCanvasObjectArrayReference& other, CloneOptions options, a2dRefMap* refs );

    void SetHorzSpace( double horzSpace ) { m_horzSpace = horzSpace; SetPending( true ); }

    double GetHorzSpace() { return m_horzSpace; }

    void SetVertSpace( double vertSpace ) { m_vertSpace = vertSpace; SetPending( true ); }

    double GetVertSpace() { return m_vertSpace; }

    int GetRows() { return m_rows; }

    void SetRows( int rows ) { m_rows = rows; SetPending( true ); }

    int GetColumns() { return m_columns; }

    void SetColumns( int columns ) { m_columns = columns; SetPending( true ); }

    //!return total width of array
    double  GetWidth() { return m_columns * m_horzSpace; }

    //!return total height of array
    double  GetHeight() { return m_rows * m_vertSpace; }

    void RemoveHierarchy();

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );

    void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

private:

    //! number of columns
    int m_columns;

    //! number of rows
    int m_rows;

    //! space between columns
    double m_horzSpace;

    //! space between rows
    double m_vertSpace;

    DECLARE_DYNAMIC_CLASS( a2dCanvasObjectArrayReference )

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCanvasObjectArrayReference( const a2dCanvasObjectArrayReference& other );
};

//!a2dLibraryReference is a symbol object, it holds a reference
/*!
to a a2dCanvasObject. But renders itself as a symbolic image, which is either a small presentation
of its children or an independent a2dCanvasObject
This object can be used to implement libraries overviews.

    \ingroup canvasobject

*/
class A2DCANVASDLLEXP a2dLibraryReference: public a2dImage
{
public:
    a2dLibraryReference();

    a2dLibraryReference( double x, double y, a2dCanvasObject* obj,  const wxString& name, double totalheight = 100,
                         a2dCanvasObject* symbol = 0,
                         const a2dFont& font = *a2dDEFAULT_CANVASFONT,
                         int imagew = 100, int imageh = 100 );

    ~a2dLibraryReference();

    a2dLibraryReference( const a2dLibraryReference& other, CloneOptions options, a2dRefMap* refs );

    //!\return the object that is referenced
    a2dCanvasObject* GetCanvasObject();

    //! set the object that is referenced
    void SetCanvasObject( a2dCanvasObject* object );

    //!\return the object that is used as icon
    a2dCanvasObject* GetSymbol();

    //! set the object that is used as icon
    void SetSymbol( a2dCanvasObject* symbol );

    DECLARE_DYNAMIC_CLASS( a2dLibraryReference )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dCanvasObjectPtr m_symbol;

    wxString m_description;

    a2dSmrtPtr<a2dText> m_text;

    //! object to which a reference is hold.
    a2dCanvasObjectPtr m_object;

    double m_height;
    double m_width;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dLibraryReference( const a2dLibraryReference& other );
};


//!a2dExternalReference is a text like object, but next to that it holds a reference
/*!
to a a2dCanvasObject. The referenced object is not a child, and is not part the boundingbox.
This object can be used to implement libraries and overviews.

  \ingroup canvasobject

*/
class A2DCANVASDLLEXP a2dExternalReference: public a2dText
{
public:
    a2dExternalReference();

    a2dExternalReference( double x, double y, a2dCanvasObject* obj = 0, const wxString& text = wxT( "" ),
                          double angle = 0,  const a2dFont& font = *a2dDEFAULT_CANVASFONT );

    ~a2dExternalReference();

    a2dExternalReference( const a2dExternalReference& other, CloneOptions options, a2dRefMap* refs );

    //!\return the object that is referenced
    a2dCanvasObject* GetCanvasObject();

    //! set the object that is referenced
    void SetCanvasObject( a2dCanvasObject* object );

    DECLARE_DYNAMIC_CLASS( a2dExternalReference )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    //! object to which a reference is held.
    a2dCanvasObjectPtr m_object;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dExternalReference( const a2dExternalReference& other );
};

//!a2dNameReference is a text like object, but next to that it holds a reference
/*!
to an a2dCanvasObject.
This object can be used to implement libraries and overviews.

    \ingroup canvasobject

*/
class A2DCANVASDLLEXP a2dNameReference: public a2dText
{
public:
    a2dNameReference();

    a2dNameReference( double x, double y, a2dCanvasObject* obj = 0, const wxString& text = wxT( "" ),
                      double angle = 0,  const a2dFont& font = *a2dDEFAULT_CANVASFONT );

    ~a2dNameReference();

    a2dNameReference( const a2dNameReference& other, CloneOptions options, a2dRefMap* refs );

    //!\return the object that is referenced
    a2dCanvasObject* GetCanvasObject();

    //! set the object that is referenced
    void SetCanvasObject( a2dCanvasObject* object );

    virtual a2dCanvasObject* PushInto( a2dCanvasObject* parent );
    
    DECLARE_DYNAMIC_CLASS( a2dNameReference )

    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    //! when resolving this refrence via a a2dIOHandler, this is used.
    virtual bool LinkReference( a2dObject* other );

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual void DoAddPending( a2dIterC& ic );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

#if wxART2D_USE_CVGIO
    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );

    void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    //! object to which a reference is held.
    a2dCanvasObjectPtr m_objectRef;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dNameReference( const a2dNameReference& other );
};


//! a2dSymbolicReference does not render referenced object.
/*!
    Internal it has a reference to a second a2dCanvasObject.

    \ingroup canvasobject

*/
class A2DCANVASDLLEXP a2dSymbolicReference: public a2dCanvasObject
{
public:
    a2dSymbolicReference();

    a2dSymbolicReference( double x, double y, a2dCanvasObject* obj );

    ~a2dSymbolicReference();

    a2dSymbolicReference( const a2dSymbolicReference& other, CloneOptions options, a2dRefMap* refs );

    //!\return the object that is referenced
    a2dCanvasObject* GetCanvasObject();

    //! set the object that is referenced
    void SetCanvasObject( a2dCanvasObject* object );

    DECLARE_DYNAMIC_CLASS( a2dSymbolicReference )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dCanvasObjectPtr m_object;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dSymbolicReference( const a2dSymbolicReference& other );
};

#endif

