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

#ifndef __WXDOCRECUR_H__
#define __WXDOCRECUR_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/editor/candoc.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/canimage.h"
#include "wx/canvas/cantext.h"

#define __USE_EVENTS  1

#define newline wxString("\n")

//! to display a2dCanvasObject references
/*!
    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dCanvasDocumentReference: public a2dLibraryReference
{
public:


    a2dCanvasDocumentReference( double x = 0, double y = 0, a2dCanvasDocument* doc = 0, double totalheight = 100,
                                const a2dFont& font = *a2dDEFAULT_CANVASFONT,
                                int imagew = 100, int imageh = 100, a2dCanvasObject* symbol = 0 );

    a2dCanvasDocumentReference( double x, double y, double totalheight, const wxFileName& filename,
                                const a2dFont& font = *a2dDEFAULT_CANVASFONT,
                                int imagew = 100, int imageh = 100, a2dCanvasObject* symbol = 0 );

    a2dCanvasDocumentReference( const a2dCanvasDocumentReference& other, CloneOptions options, a2dRefMap* refs );

    ~a2dCanvasDocumentReference();

    void SetDocumentReference( a2dCanvasDocument* doc ) { m_doc = doc; }

    a2dCanvasDocument* GetDocumentReference() const { return m_doc; }

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) );

    a2dSmrtPtr<a2dCanvasDocument> m_doc;

    DECLARE_DYNAMIC_CLASS( a2dCanvasDocumentReference )
};

//! to display a2dCanvasObject references
/*!
    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dCanvasDocumentReference2: public a2dCanvasObject
{
public:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS( a2dCanvasDocumentReference2 )

    a2dCanvasDocumentReference2( double x = 0, double y = 0, a2dCanvasDocument* doc = 0 );

    a2dCanvasDocumentReference2( double x, double y, const wxFileName& filename );

    a2dCanvasDocumentReference2( const a2dCanvasDocumentReference2& other, CloneOptions options, a2dRefMap* refs );

    ~a2dCanvasDocumentReference2();

    void SetDocumentReference( a2dCanvasDocument* doc ) { m_doc = doc; }

    a2dCanvasDocument* GetDocumentReference() const { return m_doc; }

    wxFileName& GetFileName() { return m_fileName; }

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dSmrtPtr<a2dCanvasDocument> m_doc;

    wxFileName m_fileName;
};


//! to display a2dCanvasObject references
/*!
    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dCanvasDocumentReferenceAsText: public a2dText
{
public:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS( a2dCanvasDocumentReference2 )

    a2dCanvasDocumentReferenceAsText( a2dCanvasDocument* doc = 0, double x = 0, double y = 0, const a2dFont& font = *a2dDEFAULT_CANVASFONT, double angle = 0.0,
             bool up = false, int alignment = a2dDEFAULT_ALIGNMENT );

    a2dCanvasDocumentReferenceAsText( const wxFileName& filename, double x, double y, const a2dFont& font = *a2dDEFAULT_CANVASFONT, double angle = 0.0,
             bool up = false, int alignment = a2dDEFAULT_ALIGNMENT );

    a2dCanvasDocumentReferenceAsText( const a2dCanvasDocumentReferenceAsText& other, CloneOptions options, a2dRefMap* refs );

    ~a2dCanvasDocumentReferenceAsText();

    void SetDocumentReference( a2dCanvasDocument* doc ) { m_doc = doc; }

    a2dCanvasDocument* GetDocumentReference() const { return m_doc; }

    wxFileName& GetFileName() { return m_fileName; }

    void SetDisplayShort( bool val ) { m_displayShort = val; }
    void SetDisplayExt( bool val ) { m_displayExt = val; }
    bool GetDisplayShort() { return m_displayShort; }
    bool GetDisplayExt() { return m_displayExt; }

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void DoEndEdit();

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dSmrtPtr<a2dCanvasDocument> m_doc;

    wxFileName m_fileName;

    bool m_displayShort;
    bool m_displayExt;

};

#endif

