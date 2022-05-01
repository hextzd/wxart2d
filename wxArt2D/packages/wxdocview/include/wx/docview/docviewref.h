/*! \file wx/docview/docviewref.h
    \brief Docview classes for document view, window and frame.

    All classes for a document view framework are placed in here. Only the controlling
    a2dDocumentCommandProcessor is in doccom.h.

    One or more documents are used to stor the application data,
    and views do display the data in the documents. ViewConnectors are used to couple views
    to the windows in the application GUI. Either a connector generates new frames and windows
    to achieve this or it used the existing frames or windows.

    The view connectors is the key factor for having a separate document view frame work besides
    wxWidgets its own. wxDocview needed views which could live seperately from the GUI itself,
    and even the application itself. Special designed views like the a2dCanvasView is designed
    for displaying a2dCanvasDocument, and is placed in the library to be used where needed.
    The view connectors made this possible.

    \author Julian Smart
    \author Klaas Holwerda
    \date Created 01/02/97

    Copyright: (c)

    Licence: wxWidgets licence

    Modified by: Klaas Holwerda

    RCS-ID: $Id: docviewref.h,v 1.67 2009/09/29 20:06:47 titato Exp $
*/

#ifndef _WX_DOCREFH__
#define _WX_DOCREFH__

/*!
    \defgroup docview Objects in docview frame work.

    Classes part of the docview framework.
*/

/*!
    \defgroup docviewevents Events used in docview frame work.

    \ingroup docview events
*/


#include "wx/defs.h"
#include "wx/list.h"
#include "wx/string.h"
#include "wx/frame.h"
#include "wx/filename.h"
#include <wx/splitter.h>
#include "wx/notebook.h"

#include "sigslot.h"
using namespace sigslot;
using namespace std;


#if wxUSE_PRINTING_ARCHITECTURE
#include "wx/print.h"
#endif

#include <wx/listimpl.cpp>

#include "wx/general/genmod.h"

#if wxART2D_USE_CVGIO
#include "wx/xmlparse/genxmlpars.h"
#endif //wxART2D_USE_CVGIO

#if (defined(__WXMSW__) )
#ifdef A2DDOCVIEWMAKINGDLL
#define A2DDOCVIEWDLLEXP WXEXPORT
#define A2DDOCVIEWDLLEXP_DATA(type) WXEXPORT type
#define A2DDOCVIEWDLLEXP_CTORFN
#elif defined(WXDOCVIEW_USINGDLL)
#define A2DDOCVIEWDLLEXP WXIMPORT
#define A2DDOCVIEWDLLEXP_DATA(type) WXIMPORT type
#define A2DDOCVIEWDLLEXP_CTORFN
#else // not making nor using DLL
#define A2DDOCVIEWDLLEXP
#define A2DDOCVIEWDLLEXP_DATA(type) type
#define A2DDOCVIEWDLLEXP_CTORFN
#endif
#else // (defined(__WXMSW__) )not making nor using DLL
#define A2DDOCVIEWDLLEXP
#define A2DDOCVIEWDLLEXP_DATA(type) type
#define A2DDOCVIEWDLLEXP_CTORFN
#endif //(defined(__WXMSW__) )

class A2DDOCVIEWDLLEXP a2dDocument;
class A2DDOCVIEWDLLEXP a2dView;
class A2DDOCVIEWDLLEXP a2dDocumentTemplate;
class A2DDOCVIEWDLLEXP a2dViewTemplate;


#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dDocumentTemplate>;
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dViewTemplate>;
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dView>;
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dDocument>;
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dIOHandler>;
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dIOHandlerStrIn>;
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dIOHandlerStrOut>;

template class A2DDOCVIEWDLLEXP std::allocator<class a2dSmrtPtr<class a2dDocument> >;
template class A2DDOCVIEWDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dDocument>, std::allocator<class a2dSmrtPtr<class a2dDocument> > >::_Node >;
template class A2DDOCVIEWDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dDocument>, std::allocator<class a2dSmrtPtr<class a2dDocument> > >::_Nodeptr >;
template class A2DDOCVIEWDLLEXP std::list<class a2dSmrtPtr<class a2dDocument> >;
template class A2DDOCVIEWDLLEXP a2dlist<class a2dSmrtPtr<class a2dDocument> >;
template class A2DDOCVIEWDLLEXP a2dSmrtPtrList<a2dDocument>;

template class A2DDOCVIEWDLLEXP std::allocator<class a2dSmrtPtr<class a2dView> >;
template class A2DDOCVIEWDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dView>, std::allocator<class a2dSmrtPtr<class a2dView> > >::_Node >;
template class A2DDOCVIEWDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dView>, std::allocator<class a2dSmrtPtr<class a2dView> > >::_Nodeptr >;
template class A2DDOCVIEWDLLEXP std::list<class a2dSmrtPtr<class a2dView> >;
template class A2DDOCVIEWDLLEXP a2dlist<class a2dSmrtPtr<class a2dView> >;
template class A2DDOCVIEWDLLEXP a2dSmrtPtrList<a2dView>;

template class A2DDOCVIEWDLLEXP std::allocator<class a2dSmrtPtr<class a2dDocumentTemplate> >;
template class A2DDOCVIEWDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dDocumentTemplate>, std::allocator<class a2dSmrtPtr<class a2dDocumentTemplate> > >::_Node >;
template class A2DDOCVIEWDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dDocumentTemplate>, std::allocator<class a2dSmrtPtr<class a2dDocumentTemplate> > >::_Nodeptr >;
template class A2DDOCVIEWDLLEXP std::list<class a2dSmrtPtr<class a2dDocumentTemplate> >;
template class A2DDOCVIEWDLLEXP a2dlist<class a2dSmrtPtr<class a2dDocumentTemplate> >;
template class A2DDOCVIEWDLLEXP a2dSmrtPtrList<a2dDocumentTemplate>;

template class A2DDOCVIEWDLLEXP std::allocator<class a2dSmrtPtr<class a2dViewTemplate> >;
template class A2DDOCVIEWDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dViewTemplate>, std::allocator<class a2dSmrtPtr<class a2dViewTemplate> > >::_Node >;
template class A2DDOCVIEWDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dViewTemplate>, std::allocator<class a2dSmrtPtr<class a2dViewTemplate> > >::_Nodeptr >;
template class A2DDOCVIEWDLLEXP std::list<class a2dSmrtPtr<class a2dViewTemplate> >;
template class A2DDOCVIEWDLLEXP a2dlist<class a2dSmrtPtr<class a2dViewTemplate> >;
template class A2DDOCVIEWDLLEXP a2dSmrtPtrList<a2dViewTemplate>;
#endif

//! document smart pointer
typedef a2dSmrtPtr<a2dDocument> a2dDocumentPtr;
//! view smart pointer
typedef a2dSmrtPtr<a2dView> a2dViewPtr;
//! a2dViewTemplate smart pointer
typedef a2dSmrtPtr<a2dViewTemplate> a2dViewTemplatePtr;
//! a2dDocumentTemplate smart pointer
typedef a2dSmrtPtr<a2dDocumentTemplate> a2dDocumentTemplatePtr;

//! list of documents
typedef a2dSmrtPtrList<a2dDocument> a2dDocumentList;
//! list of views
typedef a2dSmrtPtrList<a2dView> a2dViewList;
//! list of document templates
typedef a2dSmrtPtrList<a2dDocumentTemplate> a2dDocumentTemplateList;
//! list of view templates
typedef a2dSmrtPtrList<a2dViewTemplate> a2dViewTemplateList;

//! iterator for list of ...
typedef a2dDocumentList::iterator a2dDocumentListIter;

//! iterator for list of ...
typedef a2dViewList::iterator a2dViewListIter;

//! iterator for list of ...
typedef a2dDocumentTemplateList::iterator a2dDocumentTemplateListIter;

//! iterator for list of ...
typedef a2dViewTemplateList::iterator a2dViewTemplateListIter;

#if wxUSE_STD_IOSTREAM
#include "wx/ioswrap.h"
#if wxUSE_IOSTREAMH
#include <fstream.h>
#else
#include <fstream>
#endif
#else
#include "wx/wfstream.h"
#endif

//! Document manager flags
enum a2dDocumentFlag
{
    a2dREFDOC_NON      = 0x00000000, /*!< No flag */
    a2dREFDOC_NEW      = 0x00000001, /*!< create a New empty document */
    a2dREFDOC_SILENT   = 0x00000002, /*!< For adding a document by reading from file given a path as input */
    a2dREFDOC_INIT     = 0x00000004, /*!< to initialize new documents when added to the framework   */
    a2dREFDOC_LIBRARY  = 0x00000008, /*!< used as library document   */
    a2dREFDOC_ALL      = 0xFFFFFFFF, /*!< For Xor ing and all set  */
    a2dDEFAULT_DOCUMENT_FLAGS = a2dREFDOC_NON /*!< */
};


//! mask flags for a2dDocument::UpdateAllViews()
enum a2dViewUpdateFlags
{
    a2dVIEW_UPDATE_ALL     = 0x0004, /*!< total area displayed by drawer is added as pending to arealist
                                        (removes all other pending areas) */
};

//! mask of flags for a2dDocumentFlag
/*!
    \ingroup docview
*/
typedef unsigned int a2dDocumentFlagMask ;

//! mask of flags for a2dTemplateFlag
/*!
    \ingroup docview
*/
typedef unsigned int a2dTemplateFlagMask;

//! Template flags
/*!
    \ingroup docview

    Used to test if a template is visble and for what purpose it is should be.
    Mostly used to select appropriate templates from a list of templates.
    The  a2dDocumentTemplate and the a2dViewTemplate are both using this flags mask.
*/
class a2dTemplateFlag
{
public:
    static const a2dTemplateFlagMask NON;  /*!< no flags set */
    static const a2dTemplateFlagMask VISIBLE;  /*!< is the template visible */
    static const a2dTemplateFlagMask MARK;  /*!< for general use to temporary mark a template*/
    static const a2dTemplateFlagMask LOAD;  /*!< a template for loading files*/
    static const a2dTemplateFlagMask SAVE;  /*!< a template for saving  files*/
    static const a2dTemplateFlagMask IMPORTING;  /*!< a template for importing files*/
    static const a2dTemplateFlagMask EXPORTING;  /*!< a template for exporting files*/
    static const a2dTemplateFlagMask ALL;  /*!< all flags set */
    static const a2dTemplateFlagMask DEFAULT;  /*!< default */
    static const a2dTemplateFlagMask LOAD_IMPORT;  /*!< for load + import */
    static const a2dTemplateFlagMask SAVE_EXPORT;  /*!< for save + export */


    a2dTemplateFlag( a2dTemplateFlagMask newmask = NON );

    //! Check if given mask matches (flags in mask, or at least set)
    bool CheckMask( a2dTemplateFlagMask mask );

    void SetFlags( a2dTemplateFlagMask which );

    a2dTemplateFlagMask GetFlags() const;

    bool GetFlag( a2dTemplateFlagMask which ) const;

    //!template is visible
    bool m_visible: 1;
    //!template is marked
    bool m_mark: 1;
    //!template is for loading documents
    bool m_load: 1;
    //!template is for saving documents
    bool m_save: 1;
    //!template is for importing documents
    bool m_import: 1;
    //!template is for exporting documents
    bool m_export: 1;
};

#include <wx/listimpl.cpp>

/**********************************************
 New events Document and View events.
**********************************************/
BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_SIGNAL, 1 )

//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_SAVE_DOCUMENT, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_SAVEAS_DOCUMENT, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_IMPORT_DOCUMENT, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_EXPORT_DOCUMENT, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_CLOSE_DOCUMENT, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_OPEN_DOCUMENT, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_NEW_DOCUMENT, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_CREATE_DOCUMENT, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_ADD_VIEW, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_REMOVE_VIEW, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_DISCONNECT_ALLVIEWS, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_UPDATE_VIEWS, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_ENABLE_VIEWS, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_REPORT_VIEWS, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_CHANGEDFILENAME_DOCUMENT, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_CHANGEDTITLE_DOCUMENT, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_CHANGEDMODIFY_DOCUMENT, 1 )
//!see a2dDocumentEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_POST_LOAD_DOCUMENT, 1 )

//!see a2dViewEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_ACTIVATE_VIEW, 1 )
//!see a2dViewEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_ACTIVATE_VIEW_SENT_FROM_CHILD, 1 )
//!see a2dViewEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_CLOSE_VIEW, 1 )
//!see a2dViewEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_CREATE_VIEW, 1 )
//!see a2dViewEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_SET_DOCUMENT, 1 )
//!see a2dViewEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_ENABLE_VIEW, 1 )
//!see a2dViewEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_CHANGEDFILENAME_VIEW, 1 )

//! see a2dTemplateEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_PRE_ADD_DOCUMENT, 1 )
//! see a2dTemplateEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_POST_CREATE_DOCUMENT, 1 )
//! see a2dTemplateEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_POST_CREATE_VIEW, 1 )
//!see a2dTemplateEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_DISCONNECT_VIEW, 1 )

END_DECLARE_EVENT_TYPES()

//! a2dView close event
/*!
    Sent when a view needs to be closed.
    The event can be vetod.
    The event travels up to the parent window, when it is sent to a wxWindow and not processed or skipped.
    Therefore if the event is sent to the Display Window (m_display) of a view, the event can eventually
    arrive in a wxFrame class containing the wxWindow.
    At any place in this hierachy of wxWindows , one will take care of detaching the view from the windows.

    When via the a2dDocumentCommandProcessor a view is closed, one is able to also cleanup wxWindow and wxFrames
    which use the particular a2dView, this is via a2dDocument.

    The a2dView sends this event to the display window, and this window should might directly sent it
    back to the a2dView. In case it does, this is not a problem, since the a2dView disables
    itself while sending those events.

    The event Object is a a2dView.
    The type of the event is ::wxEVT_CLOSE_VIEW, and only generated in a2dView::Close().
    The event is first sent to the a2dDocument of the a2dView.
    If not vetod, it is next sent to the m_display of the a2dView.
    And if not vetod, it will be sent to the a2dView itself.
    If not vetod there, the a2dView will be removed from the a2dDocument
    its view list, and released.


    \ingroup docview docviewevents events

*/
class A2DDOCVIEWDLLEXP a2dCloseViewEvent : public a2dEvent
{
    DECLARE_CLASS( a2dCloseViewEvent )

public:

    //! Constructor for view event of type ::wxEVT_CLOSE_VIEW
    a2dCloseViewEvent( int id = 0 )
        : a2dEvent( id, wxEVT_CLOSE_VIEW ),
          m_veto( false ),    // should be false by default
          m_canVeto( true )
    {
        m_isCommandEvent = true; //let it go up to parent object in case of a wxWindow.
    }

    //! constructor
    a2dCloseViewEvent( const a2dCloseViewEvent& event )
        : a2dEvent( event ),
          m_veto( event.m_veto ),
          m_canVeto( event.m_canVeto )
    {
    }

    //! clone the event.
    virtual wxEvent* Clone() const { return new a2dCloseViewEvent( *this ); }

    //! use to veto event when the closing of the view needs to be refused in a handler.
    void Veto( bool veto = true )
    {
        // GetVeto() will return false anyhow...
        wxCHECK_RET( m_canVeto,
                     wxT( "call to Veto() ignored (can't veto this event)" ) );

        m_veto = veto;
    }

    //! If the event can not be vetod this is set to false.
    void SetCanVeto( bool canVeto ) { m_canVeto = canVeto; }

    //! If the event can be vetod this is set
    bool CanVeto() const { return m_canVeto; }

    //! If the event can be vetod this return true
    bool GetVeto() const { return m_canVeto && m_veto; }

protected:

    //! veto flags.
    bool m_veto, m_canVeto;

};


//! a2dView event, to report events in the a2dView class
/*!
    The event Object is a a2dView.

    The event travels up to the parent window, when it is sent to a wxWindow and not processed or skipped.
    Therefore if the event is sent to the Display Window (m_display) of a view, the event can eventually
    arive in a wxFrame class containing the wxWindow.

    If a2dView sends this event to the display window, this window might directly sent it
    back to the a2dView. In case it does, this is not a problem, since the a2dView disables
    itself while sending those events upwards.

    \ingroup docview docviewevents  events

*/
class A2DDOCVIEWDLLEXP a2dViewEvent : public a2dEvent
{
    DECLARE_CLASS( a2dViewEvent )

public:

    //! for event types
    /*!
        - ::wxEVT_SET_DOCUMENT
        Is sent by the a2dView when a wxDocument is set for it.
        Intercepts this to initialize your ( new or existing ) view for that document.

        - ::wxEVT_CREATE_VIEW
        Is sent by the a2dViewTemplate to the a2dView when the view
        was just created. But after the document was set.
        Intercepts this to initialize your new view.
    */
    a2dViewEvent( wxEventType type, a2dDocumentFlagMask flags, int id )
        : a2dEvent( id, type )
    {
        m_bAllow = true;
        m_isCommandEvent = true; //let it go up to parent object in case of a wxWindow.
        m_flags = flags;
        m_activate = false;
        m_enable = false;
    }

    //! for event types
    /*!
        - ::wxEVT_ENABLE_VIEW
        Sent to a2dView and DisplayWindow from a2dView::Enable() or from the a2dDocument.
        The last functions is called from a2dView::SetDisplayWindow( wxWindow* display )
        to disable the old display window, and enable the new display window.

        - ::wxEVT_ACTIVATE_VIEW
        sent from  a2dView::Activate() to the view, Register your wxEvtHandler to recieve it.
        ( intercepted at least by a2dDocumentCommandProcessor to set the view as the active
        view within the a2dDocumentCommandProcessor )
        Only when the currently active a2dView within the docmanager is different
        from this one, it will FIRST sent a second ::wxEVT_ACTIVATE_VIEW to the old active view
        to disable it. This event is ignored in a2dDocumentCommandProcessor itself.
        The ::wxEVT_ACTIVATE_VIEW is sent to the a2dView itself to disable it.
        The ::wxEVT_ACTIVATE_VIEW is sent from the view when the view is set active.

        - ::wxEVT_ACTIVATE_VIEW_SENT_FROM_CHILD
        event sent from view and its display up the parent chain. The main use is to set in a2dDocumentFrame
        the current view, so events from the frame arrive (via a window) at the (new) view when sent down there.

    */
    a2dViewEvent( wxEventType type, bool value, int id )
        : a2dEvent( id, type )
    {
        m_bAllow = true;
        m_isCommandEvent = true; //let it go up to parent object in case of a wxWindow.
        m_flags = a2dREFDOC_NON;
        m_activate = false;
        m_enable = false;
        if ( type == wxEVT_ACTIVATE_VIEW || type == wxEVT_ACTIVATE_VIEW_SENT_FROM_CHILD )
        {
            m_activate = value;
        }
        else if ( type == wxEVT_ENABLE_VIEW )
        {
            m_enable = value;
        }
    }

    //! constructor
    a2dViewEvent( const a2dViewEvent& event )
        : a2dEvent( event )
    {
        m_bAllow = event.m_bAllow;
        m_flags = event.m_flags;
        m_activate = event.m_activate;
        m_enable = event.m_enable;
    }

    //! veto the operation (usually it's allowed by default)
    void Veto() { m_bAllow = false; }

    //! allow the operation if it was disabled by default
    void Allow() { m_bAllow = true; }

    //! for implementation code only, testing result of handler: is the operation allowed?
    bool IsAllowed() const { return m_bAllow; }

    //! clone the event
    virtual wxEvent* Clone() const { return new a2dViewEvent( *this ); }

    //! for following event types, get document flags of the event.
    /*!
        - ::wxEVT_SET_DOCUMENT
        - ::wxEVT_CREATE_VIEW
    */
    a2dDocumentFlagMask GetFlags() { return m_flags; }

    //! when ::wxEVT_ACTIVATE_VIEW get activation value of the event
    bool GetActive() { return m_activate; }

    //! when ::wxEVT_ENABLE_VIEW get enable value of the event
    bool GetEnable() { return m_enable; }

private:

    //! to allow or not this event, initiated with true.
    bool m_bAllow;

    //! document flags ( interesting after a ::wxEVT_CREATE_VIEW event )
    a2dDocumentFlagMask m_flags;

    //! activate value ( interesting after a ::wxEVT_ACTIVATE_VIEW event )
    bool m_activate;

    //! activate value ( interesting after a ::wxEVT_ENABLE_VIEW event )
    bool m_enable;
};


//! sent to document before closing
/*!
    The event Object is a a2dDocument.
    The type of the event is ::wxEVT_CLOSE_DOCUMENT.
    This event is sent form bool a2dDocument::Close( bool force ) to the
    a2dDocument itself.

    The default handler is a2dDocument::OnCloseDocument().
    Which, if not vetod, disconnects ( and maybe closes ) the views on the document using a2dDocument::DisconnectAllViews().
    This will generate a ::wxEVT_DISCONNECT_ALLVIEWS event for the a2dDocument, and
    the same event is sent the view itself. Register your wxEvtHandler to recieve it.
    Views should either close or detach themselfs from the document.

    \ingroup docview docviewevents  events

*/
class A2DDOCVIEWDLLEXP a2dCloseDocumentEvent : public wxCloseEvent
{
public:

    //! constructor
    a2dCloseDocumentEvent( int id = 0 )
        : wxCloseEvent( wxEVT_CLOSE_DOCUMENT, id )
    {
    }

    //! constructor
    a2dCloseDocumentEvent( const a2dCloseDocumentEvent& event )
        : wxCloseEvent( event )
    {
    }

    //! clone the event
    virtual wxEvent* Clone() const { return new a2dCloseDocumentEvent( *this ); }

private:
    DECLARE_CLASS( a2dCloseDocumentEvent )

};

//! used to report a2dDocument events
/*!
    The event Object is a a2dDocument.

    a2dDocument does often get this event from the a2dDocumentCommandProcessor.

    \ingroup docview docviewevents  events

*/
class A2DDOCVIEWDLLEXP a2dDocumentEvent : public a2dEvent
{
    DECLARE_CLASS( a2dDocumentEvent )

public:


    //! for event types
    /*!
        - ::wxEVT_SAVE_DOCUMENT
        Sent from bool a2dDocument::Save()

        - ::wxEVT_SAVEAS_DOCUMENT
        Sent from bool a2dDocument::SaveAs()

        - ::wxEVT_CREATE_DOCUMENT
        Sent from a2dDocumentTemplate::CreateDocument() to a2dDocument() just after
        it created a new document.

        - ::wxEVT_NEW_DOCUMENT
        Sent from bool a2dDocumentCommandProcessor::CreateDocument() when creating a new document
        from a a2dDocumentTemplate.

        - ::wxEVT_OPEN_DOCUMENT
        Sent from bool a2dDocumentCommandProcessor::CreateDocument() after creating a new document
        from a a2dDocumentTemplate, which needs to have its contents read from a file.

        - ::wxEVT_DISCONNECT_ALLVIEWS
        Sent from bool a2dDocument::DisconnectAllViews()
        Register your wxEvtHandler to recieve it.

        - ::wxEVT_CHANGEDFILENAME_DOCUMENT
        Sent from document after a change in file.
        Register your wxEvtHandler to recieve it.

        - ::wxEVT_CHANGEDTITLE_DOCUMENT
        Sent from document after a change in title.
        Register your wxEvtHandler to recieve it.

    */
    a2dDocumentEvent( wxEventType type, const wxFileName& filename, a2dDocumentFlagMask flags = a2dREFDOC_NON, int id = 0 )
        : a2dEvent( id, type )
    {
        m_bAllow = true;
        m_error = a2dError_NoError;
        m_filename = filename;
        m_flags = flags;
        m_view = NULL;
        m_doctemplate = NULL;
        m_updateHint = 0;
        m_objectHint = NULL;
        m_reportViews = NULL;
    }


    //! for event types
    /*!
        - ::wxEVT_IMPORT_DOCUMENT
        Sent from a2dDocument::Import( a2dDocumentTemplate* doctemplate )
        Intercept this event to actually implement the import.

        - ::wxEVT_EXPORT_DOCUMENT
        Sent from a2dDocument::Export( a2dDocumentTemplate* doctemplate )
        Intercept this event to actually implement the export.

        The a2dIOHandlers of the choosen template, should/can be used for
        importing/exporting a file.
        This is what the default handlers a2dDocument::OnExportDocument
        and a2dDocument::OnImportDocument do use.

        \param type ::wxEVT_IMPORT_DOCUMENT or ::wxEVT_EXPORT_DOCUMENT
        \param doctemplate template choosen for importing or exporting.
        \param filename if not empty the file name of the document
        \param flags docuemnt flags
        \param id event id

    */
    a2dDocumentEvent( wxEventType type, a2dDocumentTemplate* doctemplate, const wxFileName& filename = wxFileName( wxT( "" ) ), a2dDocumentFlagMask flags = a2dREFDOC_NON, int id = 0 )
        : a2dEvent( id, type )
    {
        m_bAllow = true;
        m_error = a2dError_NoError;
        m_filename = filename;
        m_flags = flags;
        m_view = NULL;
        m_doctemplate = doctemplate;
        m_updateHint = 0;
        m_objectHint = NULL;
        m_reportViews = NULL;
    }

    //! for event types
    /*!
        ::wxEVT_REPORT_VIEWS
        Sent from a2dDocument to a2dView's. a2dView will add itself to the reportlist of the event, but only
        when this document is indeed used by the a2dView.
        Register your wxEvtHandler to recieve it.
    */
    a2dDocumentEvent( wxEventType WXUNUSED( type ), a2dViewList* reportlist )
        : a2dEvent( 0, wxEVT_REPORT_VIEWS )
    {
        m_bAllow = true;
        m_error = a2dError_NoError;
        m_filename = wxFileName( wxT( "" ) );
        m_flags = 0;
        m_view = NULL;
        m_doctemplate = NULL;
        m_updateHint = 0;
        m_objectHint = NULL;
        m_reportViews = reportlist;
    }

    //! for event types
    /*!
        Following event types:

        - ::wxEVT_ADD_VIEW
        Sent to a2dDocument when a view is added to it.

        - ::wxEVT_REMOVE_VIEW
        Sent to a2dDocument when a view is removed from it.

        - ::wxEVT_UPDATE_VIEWS ( view != NULL is not changed )
        Sent from a2dDocument::UpdateAllViews()

        - ::wxEVT_ENABLE_VIEWS ( view != NULL is not changed )
        Sent from a2dDocument::EnableAllViews()
    */
    a2dDocumentEvent( wxEventType type, a2dView* view = NULL, bool enable = true, int id = 0 )
        : a2dEvent( id, type )
    {
        m_bAllow = true;
        m_error = a2dError_NoError;
        m_view = view;
        m_filename = wxFileName( wxT( "" ) );
        m_flags = a2dREFDOC_NON;
        m_doctemplate = NULL;
        m_updateHint = 0;
        m_objectHint = NULL;
        m_reportViews = NULL;
        m_enable = enable;
    }

    //! constructor
    a2dDocumentEvent( const a2dDocumentEvent& event )
        : a2dEvent( event )
    {
        m_bAllow = event.m_bAllow;
        m_error = event.m_error;
        m_filename = event.m_filename;
        m_flags = event.m_flags;
        m_view = event.m_view;
        m_doctemplate = event.m_doctemplate;
        m_updateHint = event.m_updateHint;
        m_objectHint = event.m_objectHint;
        m_reportViews = event.m_reportViews;
        m_enable = event.m_enable;
    }

    //! veto the operation (usually it's allowed by default)
    void Veto() { m_bAllow = false; }

    //! allow the operation if it was disabled by default
    void Allow() { m_bAllow = true; }

    //! for implementation code only: is the operation allowed?
    bool IsAllowed() const { return m_bAllow; }

    //! set filename of file to open, before sending the event.
    void SetFileName( const wxFileName& filename ) { m_filename = filename; }

    //! get filename of file to open
    wxFileName& GetFileName() { return m_filename; }

    virtual wxEvent* Clone() const { return new a2dDocumentEvent( *this ); }

    //! gives the a2dDocumentFlagMask in case of event types concerning a document.
    a2dDocumentFlagMask GetFlags() { return m_flags; }

    //! Gives the a2dView in case of event types concerning a view.
    /*!
        Used with ::wxEVT_ADD_VIEW and ::wxEVT_REMOVE_VIEW
    */
    a2dView* GetView() { return m_view; }

    //! Get the template choosen to import / export.
    /*!
        Used with ::wxEVT_IMPORT_DOCUMENT and ::wxEVT_EXPORT_DOCUMENT.
    */
    a2dDocumentTemplate* GetDocumentTemplate() { return m_doctemplate; }

    void SetUpdateHint( unsigned int updateHint ) { m_updateHint = updateHint; }

    unsigned int GetUpdateHint() { return m_updateHint; }

    void SetUpdateObject( wxObject* objectHint ) { m_objectHint = objectHint; }

    wxObject* GetUpdateObject() { return m_objectHint; }

    a2dViewList* GetReportList() { return m_reportViews; }

    //! when ::wxEVT_ENABLE_VIEWS get enable value of the event
    bool GetEnable() { return m_enable; }

    //! in case of errors when sending an event, this is to know the reason of failure
    a2dError GetError() { return m_error; }

    //! in case of errors when sending an event, this can be set to tell the reason of failure
    void SetError( a2dError error ) { m_error = error; }

private:

    //! list of reported views
    a2dViewList* m_reportViews;

    //! to allow or not this event, initiated with true.
    bool m_bAllow;

    //! ::wxEVT_ENABLE_VIEWS ( to enable or diable views )
    bool m_enable;

    //! document flags ( interesting after a ::wxEVT_CREATE_VIEW event )
    a2dDocumentFlagMask m_flags;

    //! see GetView()
    a2dView* m_view;

    //! see GetFileName()
    wxFileName m_filename;

    //! see GetDocumentTemplate()
    a2dDocumentTemplate* m_doctemplate;

    //! return code in case of errors loading documents etc.
    a2dError m_error;

    unsigned int m_updateHint;

    wxObject* m_objectHint;
};


//! event sent to a2dViewConnector
/*!

    The ::wxEVT_PRE_ADD_DOCUMENT is sent to a viewconnector, after a new document was created
    from a a2dDocumentTemplate, but before it is added to the a2dDocumentCommandProcessor.
    And also just before ::wxEVT_POST_CREATE_DOCUMENT is sent to the same connector.
    If wxEVT_PRE_ADD_DOCUMENT allows you to merge or copy a new created document with an already existing
    document. In that case the document will be closed in the event handler, and after return
    the ::wxEVT_POST_CREATE_DOCUMENT will not be sent.
    The ::wxEVT_PRE_ADD_DOCUMENT gets as input the newly created document, and if merged return in that
    the document used for merge, or NULL if document is just closed.

    The two events are generated via this template class, when a2dDocumentCommandProcessor creates
    new documents.

    The default handler a2dViewConnector::OnPostCreateDocument(), will select
    a a2dViewTemplate via the a2dDocumentCommandProcessor, and create the first view on the document
    using that template.

    The ::wxEVT_POST_CREATE_DOCUMENT is sent to a viewconnector, after a new document was created
    from a a2dDocumentTemplate.
    The default handler a2dViewConnector::OnPostCreateDocument(), will select
    a a2dViewTemplate via the a2dDocumentCommandProcessor, and create the first view on the document
    using that template.

    The ::wxEVT_POST_CREATE_VIEW is sent to the viewconnector, after a new view was created from a a2dViewTemplate
    The default handler a2dViewConnector::OnPostCreateView() will tell you that you need
    to intercept this event in a derived a2dViewConnector.
    The a2dViewConnector does not know how to connect new views into frames or windows etc.
    A derived a2dViewConnector should know!

    The ::wxEVT_DISCONNECT_VIEW
    Is sent from a2dViews via a2dViewTemplate to a2dViewConnector, in order to disconnect a view.

    \ingroup docview docviewevents  events

*/
class A2DDOCVIEWDLLEXP a2dTemplateEvent : public a2dEvent
{
    DECLARE_CLASS( a2dTemplateEvent )

public:

    //! constructor
    a2dTemplateEvent( a2dDocument* doc, wxEventType type, a2dDocumentFlagMask flags = a2dREFDOC_NON, int id = 0 )
        : a2dEvent( id, type )
    {
        m_doc = doc;
        m_flags = flags;
        m_mask = a2dTemplateFlag::DEFAULT;
    }

    //! constructor
    a2dTemplateEvent( wxEventType type, a2dView* view, a2dDocumentFlagMask flags = a2dREFDOC_NON, int id = 0 )
        : a2dEvent( id, type )
    {
        m_view = view;
        m_flags = flags;
    }

    //! constructor
    a2dTemplateEvent( const a2dTemplateEvent& event )
        : a2dEvent( event )
    {
        m_doc = event.m_doc;
        m_flags = event.m_flags;
        m_mask = event.m_mask;
    }

    //! the document created.
    a2dDocument* GetDocument() { return m_doc; }

    void SetDocument( a2dDocument* doc ) { m_doc = doc; }

    //! the document flags as set by a2dDocumentTemplate::CreateDocument()
    a2dDocumentFlagMask GetFlags() { return m_flags; }

    a2dTemplateFlagMask GetTemplateMaskFlags() { return m_mask; }

    //! clone the event
    virtual wxEvent* Clone() const { return new a2dTemplateEvent( *this ); }

    //! Get the view created.
    a2dView* GetView() { return m_view; }

    //! Set the view created
    void SetView( a2dView* view ) { m_view = view; }

private:

    //! see GetDocument()
    a2dDocument* m_doc;

    //! see GetFlags()
    a2dDocumentFlagMask m_flags;

    a2dTemplateFlagMask m_mask;

    //! see GetView()
    a2dView*  m_view;
};

/*! \addtogroup eventhandlers
*  @{
*/


typedef void ( wxEvtHandler::*a2dDocumentCloseEventFunction )( a2dCloseDocumentEvent& );
typedef void ( wxEvtHandler::*a2dDocumentEventFunction )( a2dDocumentEvent& );
typedef void ( wxEvtHandler::*a2dViewCloseEventFunction )( a2dCloseViewEvent& );
typedef void ( wxEvtHandler::*a2dViewEventFunction )( a2dViewEvent& );
typedef void ( wxEvtHandler::*a2dTemplateEventFunction )( a2dTemplateEvent& );
//! \endcond

#define a2dDocumentCloseEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(a2dDocumentCloseEventFunction, &func)
#define a2dDocumentEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(a2dDocumentEventFunction, &func)
#define a2dViewCloseEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(a2dViewCloseEventFunction, &func)
#define a2dViewEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(a2dViewEventFunction, &func)
#define a2dTemplateEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(a2dTemplateEventFunction, &func)

//! event for document before all view are closed.
#define EVT_DISCONNECT_ALLVIEWS(func)  wx__DECLARE_EVT0( wxEVT_DISCONNECT_ALLVIEWS,  a2dDocumentEventHandler( func ))
//! event for document before a document will be closed ( can veto)
#define EVT_CLOSE_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_CLOSE_DOCUMENT,  a2dDocumentCloseEventHandler( func ))
//! event for document when a document has been opened
#define EVT_OPEN_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_OPEN_DOCUMENT,  a2dDocumentEventHandler( func ))
//! event for document when a document new document was created
#define EVT_NEW_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_NEW_DOCUMENT,  a2dDocumentEventHandler( func ))
//! event sent to document from template when a document has been created
#define EVT_CREATE_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_CREATE_DOCUMENT,  a2dDocumentEventHandler( func ))
//! event sent to document when a document needs to be saved
#define EVT_SAVE_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_SAVE_DOCUMENT,  a2dDocumentEventHandler( func ))
//! event sent to document when a document needs to be saved in a different file
#define EVT_SAVEAS_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_SAVEAS_DOCUMENT,  a2dDocumentEventHandler( func ))
//! event sent to document when a document is modified and the user may save or not
#define EVT_SAVE_MODIFIED_DIALOG_DOCUMENT(func) wx__DECLARE_EVT0( wxEVT_SAVE_MODIFIED_DIALOG_DOCUMENT, a2dDocumentEventHandler( func ))
//! event for document when a document will be exported
#define EVT_EXPORT_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_EXPORT_DOCUMENT,  a2dDocumentEventHandler( func ))
//! event for document when a document will be imported
#define EVT_IMPORT_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_IMPORT_DOCUMENT,  a2dDocumentEventHandler( func ))
//! event from a document when its views need to be updated
#define EVT_UPDATE_VIEWS(func)  wx__DECLARE_EVT0( wxEVT_UPDATE_VIEWS,  a2dDocumentEventHandler( func ))
//! event from a document when its views need to be enabled or disabled
#define EVT_ENABLE_VIEWS(func)  wx__DECLARE_EVT0( wxEVT_ENABLE_VIEWS,  a2dDocumentEventHandler( func ))
//! event from a document when its views need to reported
#define EVT_REPORT_VIEWS(func)  wx__DECLARE_EVT0( wxEVT_REPORT_VIEWS,  a2dDocumentEventHandler( func ))
//! event sent to view when the filename of a document has changed.
#define EVT_CHANGEDFILENAME_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_CHANGEDFILENAME_DOCUMENT, a2dDocumentEventHandler( func ))
//! event sent to view when the title of a document has changed.
#define EVT_CHANGEDTITLE_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_CHANGEDTITLE_DOCUMENT,  a2dDocumentEventHandler( func ))
//! event send from a document when modified status changed.
#define EVT_CHANGEDMODIFY_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_CHANGEDMODIFY_DOCUMENT,  a2dDocumentEventHandler( func ))
//! event send from a document just after a document was loaded from a file
#define EVT_POST_LOAD_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_POST_LOAD_DOCUMENT,  a2dDocumentEventHandler( func ))
//! event sent to document and view when a view needs to be closed ( can veto)
#define EVT_CLOSE_VIEW(func)  wx__DECLARE_EVT0( wxEVT_CLOSE_VIEW, a2dViewCloseEventHandler( func))
//! event sent to view from template when a view has been created
#define EVT_CREATE_VIEW(func)  wx__DECLARE_EVT0( wxEVT_CREATE_VIEW,  a2dViewEventHandler( func ))
//! event sent to view when a new document is ser for it.
#define EVT_SET_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_SET_DOCUMENT,  a2dViewEventHandler( func ))
//! event sent to view and document manager
#define EVT_ACTIVATE_VIEW(func)  wx__DECLARE_EVT0( wxEVT_ACTIVATE_VIEW,  a2dViewEventHandler( func ))
//! event sent from view and its display up the parent chain.
#define EVT_ACTIVATE_VIEW_SENT_FROM_CHILD(func) wx__DECLARE_EVT0( wxEVT_ACTIVATE_VIEW_SENT_FROM_CHILD, a2dViewEventHandler( func ))
//! event sent to connector to disable or enable for a certain view
#define EVT_ENABLE_VIEW(func)  wx__DECLARE_EVT0( wxEVT_ENABLE_VIEW,  a2dViewEventHandler( func ))
//! event sent to document when a view has been added to the document
#define EVT_ADD_VIEW(func)  wx__DECLARE_EVT0( wxEVT_ADD_VIEW,  a2dDocumentEventHandler( func ))
//! event sent to document when a view will be removed from the document
#define EVT_REMOVE_VIEW(func)  wx__DECLARE_EVT0( wxEVT_REMOVE_VIEW,  a2dDocumentEventHandler( func ))
//! sent from a doc template after it created a document
#define EVT_PRE_ADD_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_PRE_ADD_DOCUMENT,  a2dTemplateEventHandler( func ))
//! sent from a doc template after it created a document
#define EVT_POST_CREATE_DOCUMENT(func)  wx__DECLARE_EVT0( wxEVT_POST_CREATE_DOCUMENT,  a2dTemplateEventHandler( func ))
//! sent from a view template after it created a view
#define EVT_POST_CREATE_VIEW(func)  wx__DECLARE_EVT0( wxEVT_POST_CREATE_VIEW,  a2dTemplateEventHandler( func ))
//! event sent to connector to disconnect a certain view from its document
#define EVT_DISCONNECT_VIEW(func)  wx__DECLARE_EVT0( wxEVT_DISCONNECT_VIEW,  a2dTemplateEventHandler( func ))
//! event sent from view upwards to display, when the filename of a document has changed.
#define EVT_CHANGEDFILENAME_VIEW(func)  wx__DECLARE_EVT0( wxEVT_CHANGEDFILENAME_VIEW, a2dViewEventHandler( func ))
//! event sent from view upwards to display, when the title of a document has changed.
#define EVT_CHANGEDTITLE_VIEW(func)  wx__DECLARE_EVT0( wxEVT_CHANGEDTITLE_VIEW,  a2dViewEventHandler( func ))

//!  @} eventhandlers

class A2DDOCVIEWDLLEXP a2dIOHandlerXmlSerIn;
class A2DDOCVIEWDLLEXP a2dIOHandlerXmlSerOut;

//! The document class can be used to model an application's file-based data.
/*! It is part of the document/view framework,
    and cooperates with the a2dView, a2dDocumentTemplate, a2dViewTemplate and a2dDocumentCommandProcessor classes.

    a2dDocument can also be used in a setting with just a2dView's and/or templates, so without
    a a2dDocumentCommandProcessor controlling most tasks. Most events coming from the document meant
    for the a2dView (e.g. Update events) will be received, since it is always registered to the document.
    For others register your wxEvtHandler to recieve events.
    Therefore if you just want to display the fixed data in the a2dDocument, a a2dDocumentCommandProcessor is not really
    needed.
    In the general case with several view and documents, it is best to use a a2dDocumentCommandProcessor.


    Events types which can be/are intercepted:

        - ::wxEVT_CLOSE_VIEW sent when a view wants to be closed ( may veto )
        - ::wxEVT_DISCONNECT_ALLVIEWS sent to document and document manager within DisconnectAllViews().
        - ::wxEVT_CLOSE_DOCUMENT sent to document just after calling Close()
        - ::wxEVT_ADD_VIEW sent to document just after a view was added to the document
        - ::wxEVT_REMOVE_VIEW sent to document just before a view is removed from the document
        - ::wxEVT_OPEN_DOCUMENT sent by docmanager when a document was created and opened from a file.
            Default handler loads the file.
        - ::wxEVT_NEW_DOCUMENT sent by docmanager when a document was created as new.
            See default handler.
        - ::wxEVT_CREATE_DOCUMENT sent after creation of document from a template.
        - ::wxEVT_SAVE_DOCUMENT sent to save document ( e.g from Save() )
        - ::wxEVT_SAVEAS_DOCUMENT sent to save document under a different name ( e.g from SaveAs() )
        - ::wxEVT_EXPORT_DOCUMENT sent to export a document ( e.g from Export())
        - ::wxEVT_IMPORT_DOCUMENT sent to export a document ( e.g from Import())
        - ::wxEVT_POST_LOAD_DOCUMENT sent just after a document was loaded from a file in LoadObject()
        - ::wxEVT_CHANGEDMODIFY_DOCUMENT sent just after a document its modify flag has changed in Modify()
        - ::wxEVT_REPORT_VIEWS sent to the event views, to assemble all a2dView using the document.
        - ::wxEVT_UPDATE_VIEWS sent when all view need to update them selfs ( e.g. after a change in the document )
            \see UpdateAllViews()
        - ::wxEVT_ENABLE_VIEWS sent from EnableAllViews(), to disable or enable all views on the document.
        - ::wxEVT_CHANGEDFILENAME_DOCUMENT sent from SetFilename(), when the document its fileinfo has changed.
        - ::wxEVT_CHANGEDTITLE_DOCUMENT sent from SetTitle(), when the document its title has changed.

    \ingroup docview
*/
class A2DDOCVIEWDLLEXP a2dDocument : public a2dObject
{
    DECLARE_ABSTRACT_CLASS( a2dDocument )

    DECLARE_EVENT_TABLE()

public:

    //! Constructor.
    /*!
        The document may have a parent, so a hierachically structure of
        documents is possible.

        \remark
        Current implementation doesn't support a nice access to parent/child
        docs.

        \param parent The document's parent
    */
    a2dDocument( a2dDocument* parent = ( a2dDocument* ) NULL );

    //! destructor
    ~a2dDocument();

    //! constructor with other document
    a2dDocument( const a2dDocument& other );

    // accessors

    //!Sets the filename for this document. Usually called by the framework.
    /*!
        If notifyViews is true, a a2dDocumentEvent of type ::wxEVT_CHANGEDFILENAME_DOCUMENT.
        Register your wxEvtHandler to recieve it.
    */
    void SetFilename( const wxFileName& filename, bool notifyViews = false );

    //! Get the file name in use for this document.
    wxFileName GetFilename() const { return m_documentFile; }

    //!Sets the title for this document.
    /*!
        The document title is used for an associated frame (if any), and is usually
        constructed by the framework from the filename.
        The title may be changed by the programmer or set by an IOHandler

        \param title        The title of the document.
        \param notifyViews  If <code>true</code> a a2dDocumentEvent of type ::wxEVT_CHANGEDTITLE_DOCUMENT
                            Register your wxEvtHandler to recieve it.
    */
    void SetTitle( const wxString& title, bool notifyViews = false );

    //! Returns the title for this document.
    wxString GetTitle() const { return m_documentTitle; }

    //!Sets the document type name for this document. See the comment for documentTypeName.
    void SetDocumentTypeName( const wxString& name ) { m_documentTypeName = name; };

    //!Gets the document type name of this document. See the comment for documentTypeName.
    wxString GetDocumentTypeName() const { return m_documentTypeName; }

    //! Returns if the document was saved after a modification.
    bool GetDocumentSaved() const { return m_savedYet; }

    //! Alias for a2dDocument::GetDocumentSaved
    bool IsDocumentSaved() const { return GetDocumentSaved(); }

    //! Sets the document saved flag.
    /*!
        The document saved flag is normally set by the framework,
    */
    void SetDocumentSaved( bool saved = true ) { m_savedYet = saved; }

    //! returns true when document is closed or busy closing.
    /*!
        Closing a document in general includes closing its views too.
        When busy closing a document, this function here can be used to prevent recursive calls elsewhere.
        You can savely call Close() anyway, but if other actions depends on the document being closed or not,
        you can check it with this here. A typical example is a closing a document and also releasing it from
        some higher order class.
        \see  Close() for more.
    */
    bool IsClosed() { return m_isClosed; }

    //!Closes the document if not vetod.
    /*!Closing a document means that the application does not want the
       the document in memory anymore in order to view or modify it.
       This function prepares the document for deletion.

       Sends a ::wxEVT_CLOSE_DOCUMENT event.
       The default handler is OnCloseDocument( a2dCloseDocumentEvent& event ).

       Default calls SaveModifiedDialog(), to give the user a change to save his changes.

       This does not normally delete the document object, which only happens if the reference
       count reaches zero, indicating no more Owners do exist.

        \see OnCloseDocument
        \see SaveModifiedDialog
    */
    bool Close( bool force );


    //!Saves the document by sending a a2dDocumentEvent ::wxEVT_SAVE_DOCUMENT event.
    /*!
        The method first tests if the document is modified and not saved yet.
        If it is modified and not saved, the event is sent.

        \see OnSaveDocument

        \return true if the document could be saved
    */
    bool Save();

    //! Tries to save the document by sending a a2dDocumentEvent ::wxEVT_SAVEAS_DOCUMENT event.
    /*!
        The default handler asks for a file, to save the document into.

        \see OnSaveAsDocument

        \return true if the document could be saved
    */
    bool SaveAs( const wxFileName& fileTosaveTo = wxFileName( wxT( "" ) ), a2dDocumentFlagMask flags = a2dREFDOC_NON );


    //! Tries to save the document by sending a a2dDocumentEvent ::wxEVT_EXPORT_DOCUMENT event.
    /*!
        \see OnExportDocument

        \return true if the document could be saved
    */
    bool Export( a2dDocumentTemplate* doctemplate, const wxFileName& fileTosaveTo = wxFileName( wxT( "" ) ), a2dDocumentFlagMask flags = a2dREFDOC_NON );

    //! Tries to read a file into the document by sending a a2dDocumentEvent ::wxEVT_IMPORT_DOCUMENT event.
    /*!
        \see OnImportDocument

        \return true if the document could be read
    */
    bool Import( a2dDocumentTemplate* doctemplate, const wxFileName& fileToImport = wxFileName( wxT( "" ) ), a2dDocumentFlagMask flags = a2dREFDOC_NON );

    //! Normally should undo all changes by rereading the file again.
    /*!
        \todo
        make a default implementation which does do it
        ( unless already saved the changes sofar )
    */
    virtual bool Revert();

    //! handling loading of the document from a stream
    /*! The given handler is used to load the document from the stream.

        Override this function to implement your own version.
        LoadObject is called by the framework automatically when
        the document contents needs to be loaded.

        \note only one of these forms exists, depending on how wxWindows was configured.
    */
    virtual a2dDocumentInputStream& LoadObject( a2dDocumentInputStream& stream, a2dIOHandlerStrIn* handler );

    //! handling saving and exporting of the document to a stream
    /*! The a2dIOHandler given is used to save the document to the stream.

        Override this function to implement your own version.
        SaveObject is called by the framework automatically when
        the document contents need to be saved.

        \note only one of these forms exists, depending on how wxWindows was configured.
    */
    virtual a2dDocumentOutputStream& SaveObject( a2dDocumentOutputStream& stream, a2dIOHandlerStrOut* handler );

    //! can be set by a2dIOHandler's by e.g LoadObject().
    /*!
        If parsing a document from the stream is incorrect due to formating errors, the handler or LoadObject itself
        can set this error code, which result in veto-ing the event.
    */
    void SetLastError( a2dError lastError ) { m_lastError = lastError; }

    //! is set by a2dIOHandler's and is tested by LoadObject().
    /*!
        If parsing a document from the stream is incorrect due to formating errors, the handler or LoadObject itself
        can set this error code.
    */
    a2dError GetLastError() { return m_lastError; }

#if wxART2D_USE_CVGIO
    //! used by CVG parser
    /*!
        \remark do not use directly
    */
    void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser );

    //! called from a2dIOHandlerXmlSerOut after a SaveObject.
    /*!
        \param parent parent object from which Save was called
        \param out CVG io handler
        \param towrite object to write later
    */
    void Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite );

    //! called from a2dIOHandlerXmlSerOut after a SaveObject.
    /*!
        \param out CVG io handler
        \param start start wrting from this object
    */
    void Save( a2dIOHandlerXmlSerOut& out, wxObject* start = NULL );
#endif //wxART2D_USE_CVGIO

    //! dialog when the user needs to confirm saving a document
    /*!
        If the document has been modified, prompts the user to ask if the
        changes should be saved.
        If the user replies Yes, the Save function is called and return is wxYES.
        If No, the document is marked as unmodified and return is wxNO
        If Cancel return is wxCANCEL.
        If document was not modified, also wxYES is returned.
    */
    virtual int SaveModifiedDialog( bool canCancel = true );

    //! create a command processor and set it for the document.
    /*!
        If available the existing commandprocessor will be deleted.
        A new CommandProcessor will be created and set for the document.

        Override if you need a more application specific commandhandler.
    */
    virtual void CreateCommandProcessor();

    //!Returns a pointer to the command processor associated with this document
    a2dCommandProcessor* GetCommandProcessor() const { return m_commandProcessor; }

    //! Sets the command processor to be used for this document.
    /*!
        Use in case you need one a2dCommandProcessor to be used for several
        a2dDocument's,
        Make sure you set it to NULL before deleting the document itself,
        else the a2dDocument destructor will delete it ( leaving the others
        with dangling pointers ).
    */
    void SetCommandProcessor( a2dCommandProcessor* proc ) { m_commandProcessor = proc; }

    //! called when there is a need to clear the contents of the document.
    /*!
        Override this function to define the way to do this.
    */
    virtual void DeleteContents();

    //! Might be used to draw the contents of a document to a a2dView using a wxDc as the drawing context.
    /*!
        You need to override this method in derived class to do something usefull.

        \return default only returns true
    */
    virtual bool Draw( wxDC& context );

    //! Has the document been modified
    bool IsModified() const { return m_documentModified != 0; }

    //! set the docuent as modified or not
    /*!
        If the modified status changes an ::wxEVT_CHANGEDMODIFY_DOCUMENT event will
        be send.

        \param mod  <code>true</code> to set document modified, else <code>false</code>
    */
    void Modify( bool mod );

    //! Ask to save when document is modified and last view is closed?
    /*!
        When there are still owners of the document, and the last view is closed
        it can either be saved or delayed until the application closes.
        Setting AskSaveWhenOwned delayes the saving of the data until later.
    */
    void AskSaveWhenOwned( bool askSaveWhenOwned ) { m_askSaveWhenOwned = askSaveWhenOwned; }


    //! sent Update events to all a2dView which display or use this document.
    /*!
        This function is typicaly called when the document its contents has changed,
        this in general will result in the a2dView displaying this new information.
        Understand that this only informs the views to update themselfs, how and what the view needs
        to update can be indicated by the hints, or in more complex situations the views will
        read the document contents to see what did change, and only update what changed in its view.

        \param sender which view did calls this function ( Non = NULL ).
        \param hint update will differ on the hint given and the implementation
        \param objecthint more complex way to deliver hints

    */
    virtual void UpdateAllViews( a2dView* sender = ( a2dView* ) NULL, unsigned int hint = a2dVIEW_UPDATE_ALL, wxObject* objecthint = ( wxObject* ) NULL );


    //! sends a ::wxEVT_REPORT_VIEWS to all views, via the a2dDocumentCommandProcessor.
    /*!
        The a2dView's using this document will report themself by adding
        itself to the allviews list.
    */
    void ReportViews( a2dViewList* allviews );

    //! calls Enable( true/false ) for each view.
    /*!
        Enables or disables  all views. If sender is non-NULL, does NOT call it for this view.
        The disabling a a2dView means that it will not process any events, until enabled again.
    */
    void EnableAllViews( bool enable, a2dView* sender = ( a2dView* ) NULL );

    //! Disconnect a2dView's using this document.
    /*!
        It depends on the application if a disconnected view will be closed or not.
        In case the views will be resused for another document after disconnecting them,
        obviously they should not be closed. Typically thsi will be the case in a one frame application,
        where the frame has one or more fixed views, which will be used for the currently open document.

        Before starting to disconnect the views, a a2dDocumentEvent type ::wxEVT_DISCONNECT_ALLVIEWS is sent to
        this a2dDocument, which can be intercepted to do some general cleanup on the
        document.

        In case the a2dView::Close() is called on disconnection, .
        this generates a a2dCloseViewEvent (type  ::wxEVT_CLOSE_VIEW ) sent to the
        document and the view. The event can not be vetod for this action.
    */
    void DisConnectAllViews();

    //! Gets a2dDocumentTemplate pointer which was used to create the a2dView
    a2dDocumentTemplate* GetDocumentTemplate() const { return m_documentTemplate; }

    //! Set a2dDocumentTemplate pointer
    void SetDocumentTemplate( a2dDocumentTemplate* temp ) { m_documentTemplate = temp; }

    //! Get title, or filename if no title, else [unnamed]
    wxString GetPrintableName() const;

    //! Returns a window that can be used as a parent for document-related dialogs. Override if necessary.
    virtual wxWindow* GetAssociatedWindow() const;

    //! Sets a time of last modification
    /*!
        \param datatime new modification time
    */
    void SetModificationTime( const wxDateTime& datatime ) { m_modificationtime = datatime; }

    //! Returns the time of last modification when the document was saved
    /*!
        \return time of last modification
    */
    wxDateTime& GetModificationTime() { return m_modificationtime; }

protected:

    //!Clone document
    a2dObject* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    //! default handler for ::wxEVT_CREATE_DOCUMENT, Called after creation of document from a template.
    /*! Called by framework if the documemt is created automatically by the default document
        manager (via a document template).
        This gives the document a chance to initialise.

        This Default handler does Nothing Yet.
    */
    void OnCreateDocument( a2dDocumentEvent& event );

    //! Called by wxWindows when a file needs to be saved
    /*!
        Default handler for a2dDocumentEvent of type ::wxEVT_SAVE_DOCUMENT.

        Save in the file filename of the document,
        or call SaveAs if the filename is not set.

        Constructs an output file stream for the given filename (which must not be empty),
        and calls SaveObject. If SaveObject returns true, the document is
        set to unmodified; otherwise, an error message box is displayed.

        event is set Allowed if the file could be opened and the document was saved.

        \remark the document its a2dDocumentTemplate is used to find the right a2dIOHandlerStrOut via
        a2dDocumentTemplate::GetDocumentIOHandlerForSave().

    */
    void OnSaveDocument( a2dDocumentEvent& event );


    //! default handler for a2dDocumentEvent of type ::wxEVT_SAVEAS_DOCUMENT
    /*!
        Prompts the user for a file to save to.

        Notifies the views that the filename has changes by sending ::wxEVT_CHANGEDFILENAME_DOCUMENT.
        Register your wxEvtHandler to receive it.

        \remark the document its a2dDocumentTemplate is used to find the right a2dIOHandlerStrOut via
        a2dDocumentTemplate::GetDocumentIOHandlerForSave().

    */
    void OnSaveAsDocument( a2dDocumentEvent& event );

    //! load a document from a file into this document
    /*!
        Default handler for a2dDocumentEvent of type ::wxEVT_OPEN_DOCUMENT.

        In the default handler SaveModifiedDialog() is called first, and if the document is modified
        the user is asked to save the changes. Cancel will not open the file into the document.

        Constructs an input file stream for the given filename (which must not be empty),
        and calls LoadObject. If LoadObject returns true, the document is set to unmodified; otherwise,
        an error message box is displayed. The document's views are notified that the filename has changed,
        to give windows an opportunity to update their titles.
        All of the document's views are then updated.

        \remark m_documentTemplate must be set for the default handler to work.

        \note only one of these forms exists, depending on how wxWindows was configured.
    */
    void OnOpenDocument( a2dDocumentEvent& event );

    //! called after creating a new document from a template , in order to initialize it.
    /*!
        called by:
        a2dDocumentCommandProcessor::CreateDocument()
        a2dDocumentCommandProcessor::AddCreatedDocument()

        Default handler for a2dDocumentEvent of type ::wxEVT_NEW_DOCUMENT.

        The default makes a default title for the document, and notifies the views that the filename
        (in fact, the title) has changed.

        The user can intercept the event to fill new documents if empty,
        but do Skip() the event for the base implementation to be called.

        Understand that views might already by open on the document.
    */
    void OnNewDocument( a2dDocumentEvent& event );

    //! called when document is closed
    /*!
       Default handler for a2dDocumentEvent of type ::wxEVT_CLOSE_DOCUMENT.

       The default implementation, calls SaveModifiedDialog(), and if not canceled,
       disconnects and/or closes all views using DisconnectAllViews().
       This leads to sending ::wxEVT_DISCONNECT_ALLVIEWS to the document and ::wxEVT_CLOSE_VIEW
       to each view. This enables the program to delete or detach views etc.
       Next empties the document by calling DeleteContents setting modified flag to false.

       Override this to supply additional behaviour
       when the document is closed with Close.

       e.g. In a single document and view approach, you very likely want
       to call DeleteContents and set the modified flag to false.

       Emptying a document while views are using it, is dangerous.
       Views may refer to or use objects in the document.
       Therefore the default:
            - closes all views ( holding and display parts of the document, else maybe dangling pointers )
            - clears the undo stack ( which may hold references to objects within the document )
            - empty the document

       \remark Remember a view also Owns a reference to a document, so certainly all views need to be gone
       to really get a document deleted.
    */
    void OnCloseDocument( a2dCloseDocumentEvent& event );


    //! Intercept to do cleanup/veto closing of a view
    /*!
        The handler function for EVT_CLOSE_VIEW, it is called when the program tries
        to close a View.

        The frame or window containing the view sent a EVT_CLOSE_WINDOW, this
        results in a EVT_CLOSE_VIEW being sent to the document and to the view.
        OR The a2dDocumentCommandProcessor is closing all documents plus views.

        The a2dCloseViewEvent is generated via the a2dView::Close function, so that
        the application can also invoke the handler programmatically.

        \sa a2dCloseViewEvent for the way the event is propegated to the several classes in
        the docview framework.
        If OnCloseView does not veto the close event, the a2dView will futher
        propegate the event.

        The Default implementation:
        Checks if the view is the last reference for the document; if so, asks user
        to confirm save data.
        If AskSaveWhenOwned() == true it will always ask to save the data.
    */
    void OnCloseView( a2dCloseViewEvent& event );

    //! what to do when a view using this document will stop using it.
    /*! When a view is switching its document to another document, or when a view is closed,
        this event is sent to the document just before a2dView releases the document.
        The default implementation asks to save the document if it was modified, but only if m_askSaveWhenOwned
        is true or m_refcount has reached 1, which is when only the view itself Own the document.
        In an application where this document is also controlled by a a2dDocumentCommandProcessor, the refcount
        would by two.
        When an application has a fixed set of views (per Frame) used by any document that is shown,
        it of course makes no sence to automatically close a document when the refcount reaches 1.
        In a standard "multi child frame+view + document", one can automatically close the document when
        a refcount of < 2 is reached. Which means when a document its last view is closed, and the document
        is only owned by the controlling a2dDocumentCommandProcessor.
        To make this happen intercept the ::wxEVT_REMOVE_VIEW, and implement whatever you need.
        In this case you should call a2dDocviewGlobals->GetDocviewCommandProcessor()->FileClose() in order
        to Close the document, which leads to the a2dDocumentCommandProcessor releasing the document.
        The very last release and therefore deletion will be done in a2dView after returning from the
        ::wxEVT_REMOVE_VIEW event.
    */
    void OnRemoveView( a2dDocumentEvent& event );

    //! what to do when a view starts using this document
    void OnAddView( a2dDocumentEvent& event );

    //! The default handler for ::wxEVT_EXPORT_DOCUMENT.
    /*!
        This method asks for a file, to export/save the document into,
        using the handler of the event its a2dDocumentTemplate.
        If a a2dDocumentTemplate was not given as part of the event, the document template
        of this document is used to search for possible templates to be used for exporting
        this document. The selection is based on GetDocumentTemplate()->GetDocumentTypeName().
        Next a file dialog is presented to select a file to export.

        If a filename is part of the event, this is used as initial file to export to in the save dialog.
        If a2dDocumentFlagMask contains a2dREFDOC_SILENT, no dialog will be presented.
    */
    void OnExportDocument( a2dDocumentEvent& event );

    //! The default handler for ::wxEVT_IMPORT_DOCUMENT.
    /*!
        This method asks for a file, to read the document from,
        using the handler of the event its a2dDocumentTemplate.
        If a a2dDocumentTemplate was not given as part of the event, the document template
        of this document is used to search for possible templates to be used for importing
        into this document. The selection is based on GetDocumentTemplate()->GetDocumentTypeName().
        Next a file dialog is presented to select a file to import.

        If as part of the event a path+path was already provided, that will be used.
        If a2dREFDOC_SILENT was set as flag in the event, no dialogs will appear, when a file+path is available.
    */
    void OnImportDocument( a2dDocumentEvent& event );

    //!Set when Owners become zero and document will be deleted. May also be overruled by user.
    bool m_askSaveWhenOwned;

    //! Filename associated with this document ("" if none).
    wxFileName m_documentFile;

    //! last modification time when this document was stored.
    wxDateTime m_modificationtime;

    //!Document title.
    /*!
        The document title is used for an associated
        frame (if any), and is usually constructed by the framework from the filename.
    */
    wxString m_documentTitle;

    //!The document type name given to the a2dDocumentTemplate constructor,
    /*!is copied to this variable when the document is created.

       In a2dDocumentCommandProcessor::AddCreatedDocument() first set this, to search an appropriate template.

       It is used later on to select Views from the a2dViewTemplate's.
       a2dViewTemplate uses the same documentTypeName as a2dDocumentTemplate's.

       The same a2dDocument derived class instances, can have a different m_documentTypeName.

       If several view templates are created that use the same document type,
       this variable is used in a2dDocumentCommandProcessor::CreateView to collate a list of
       alternative view types that can be used on this kind of document.

    */
    wxString m_documentTypeName;

    //!A pointer to the template from which this document was created
    a2dDocumentTemplatePtr m_documentTemplate;

    //! modified flag
    long m_documentModified;

    //! parant document if set.
    a2dDocument* m_documentParent;

    //!A pointer to the command processor associated with this document.
    a2dSmrtPtr<a2dCommandProcessor> m_commandProcessor;

    //! saved flag
    bool m_savedYet;

    //! set true when document is closed or when busy closing document.
    bool m_isClosed;

    //! This identifies the document in event processing
    /*! Not sure, if this is really used for something */
    wxUint32 m_id;

    //! error code used when LoadObject is returning.
    a2dError m_lastError;

private:

    virtual a2dObject* SmrtPtrOwn();
    //!To release the object, it is not longer owned by the calling object.
    /*! the object itself will be deleted when not Owned (the reference count becomes zero)

      \remark AskSaveWhenOwned was false, and this document is not Owned by anyone,
              the user will now be asked to save the document (when it was modified).
    */
    virtual bool SmrtPtrRelease();
    friend class a2dSmrtPtrBase;
};

class A2DDOCVIEWDLLEXP a2dViewConnector;
class A2DDOCVIEWDLLEXP a2dViewTemplate;

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dViewConnector>;
#endif


extern const a2dPrintWhat a2dPRINT_Print; //!< general print
extern const a2dPrintWhat a2dPRINT_Preview;           //!<  general print preview
extern const a2dPrintWhat a2dPRINT_PrintView;         //!< print a2dView
extern const a2dPrintWhat a2dPRINT_PreviewView;       //!< preview print a2dView
extern const a2dPrintWhat a2dPRINT_PrintDocument;     //!< print a2dDocument
extern const a2dPrintWhat a2dPRINT_PreviewDocument;   //!< preview print a2dDocument
extern const a2dPrintWhat a2dPRINT_PrintSetup;


//! Holds a view on a a2dDocument.
/*!
    A view is able to connect your documents to the windows in an application.
    It knows its document or documents, and it know where (which windows) the data from the document is displayed.
    This makes it possible to close or disconnect all views on a document, and indirectly its windows can be closed.
    For instance; closing the application, will close all its documents, and via the views, 
    will close all windows/frames on the documents. But also closing just one frame, can delete one document via the
    frame its view. The view is like a two way bridge between the document and the GUI.

    A a2dView is indirectly used for displaying a (part of) a2dDocument its data.
    It may display this data in any form it wants to.
    Several a2dView's can exist on a a2dDocument, and they can be of different types.
    A a2dView organizes to display data stored in a2dDocument's.
    A document can be set for a view, and this will be used by the view
    to check if ::wxEVT_UPDATE_VIEWS events coming from the documents, are meant
    for the view or not.
    If a view wants to display the data of several documents, this can be arranged
    in a derived class.

    The views on a document, work independent of eachother.
    When using a a2dDocumentCommandProcessor class, this will know the currently active view.
    In general this is the view which has the focus. Meaning its display window
    has the focus.

    The a2dDocument class uses events to notify changes.
    The a2dView's connect themselfs to a a2dDocument to recieve specific events. 
    Any wxEvtHandler derived class may be connected to the a2dDocument. 
    The document does not know all this in advance, it just sends events to itself.
    The dynamically connected handlers, will receive those events too.

    Since a2dViews connects to specific events in a a2dDocument, 
    one could think that the same can be done with wxWindow's etc. directly.
    But using a2dView, makes it easy to centralize the actions needed, independent from 
    the application its windows and dialogs. A dedicated view, used to display data from
    a dedicated document, can be used for a wxFrame or a wxNotebook page at the same time.
    The view does not need to know about your application in terms of windows. 
    Because of this, its functionality can be made part of a library, to be used by any application.

    The view may display the data from the document, directly to the device or in a
    delayed fashion. In which case it stores the data to display first locally,
    and as soon as possible this data is updated to the device ( e.g. a window. )
    If the device is a window, the m_display holds a pointer to it.

    Views display the data in a document, and therefore if the document data changes,
    the view might change because of that. The a2dDocument::UpdateAllViews() is
    sending ::wxEVT_UPDATE_VIEWS to the a2dDocument, and this sents
    the event to all connected a2dView's. a2dView::OnUpdate() will handle the event.
    It checks if it wants to process the event coming from that particular document,
    and if so updates its own data from that document.
    To update all views at every small change made to the document, is often not desired.
    Checking a "pending flag" inside the document when the program becomes idle is a better
    way to update the views.

    Often the view is a rectangular drawing area. The view can use a wxWindow derived class
    to display the data for a document. But this is not a must, a view may be stored anywhere.
    A view might also display parts of a document(s) via wxDialogs, or it might store it in a bitmap.
    How and what is displayed using a view depends on the derived class.

    The way to update what is displayed by the view needs to be defined in the handler
    for ::wxEVT_UPDATE_VIEWS event. The default OnUpdate, just calls refresh on the view its display window.
    Here the display window will redisplay the specific data from the document in the way it wants.
    Changes in the document need to result in calling OnUpdate for each view.
    OnUpdate may decide to directly redrawn on the device, or only store the view data locally.
    In the last case a flag is set to indicate that the view has changed.
    This flag is checked in idle time, resulting in an update of the device eventually.
    This is the principle behind calling m_display->Refresh() after
    changing a document or a view its display data.
    This will result in a wxPaintEvent, which will redraw the view data.

    When the display device is a window, the wxPaintEvent event handled in the window, may
    request a redraw of certain areas within the view. The view may do this by
    actually redrawing directly from the document, or in case of local stored data,
    return/redraw only this locally stored data directly on the window.
    In some cases the data displayed on the window, is actually stored locally within
    the window, and the window handles paint events itself, without asking the view to redisplay
    the damaged areas ( e.g. wxTextCtrl. etc. ). Still the window will need to check
    if the view or document did change, to keep its contents up to date.
    The member a2dView::OnDraw( wxDC* ) can be used by the display window, to have
    the view redraw itself on the window. But of course if not using wxDC based views,
    any other method can be used.
    The important thing to understand, is that the Updating of a a2dView can happen
    at time X, while the display of this updated view can happen at time Y.
    Therefore several updates on a view might take place before they are actually
    redisplayed.

    For printing the view in a2dView::OnPrint(wxDC *dc), you may use
    Ondraw to print the contents of a view to a wxDC.
    For that you should define a way to draw the view its data on a wxDC.

    Events which are sent to a2dView:

    a2dView is by default Registered to a2dDocument which is set to it. And will recieve all events from send there.

    In general all wxWindow events of m_display or the wxFrame containing the a2dView
        - ::wxEVT_SET_FOCUS ::wxEVT_KILL_FOCUS etc.

    View specific events.
        - a2dViewEvent -> type = ::wxEVT_CLOSE_VIEW
        - a2dViewEvent -> type = ::wxEVT_ACTIVATE_VIEW sent view itself from Activate()
        - a2dViewEvent -> type = ::wxEVT_ACTIVATE_VIEW_SENT_FROM_CHILD sent from view itself to m_display from Activate()
        - a2dViewEvent -> type = ::wxEVT_CREATE_VIEW sent to view from a2dViewTemplate::CreateView()
            Called by framework if created automatically by the default document
            manager class: gives view a chance to initialise
            e.g. void OnCreateView( a2dViewEvent* event );
        - a2dViewEvent -> type = ::wxEVT_SET_DOCUMENT sent to view itself when calling SetDocument()
        - a2dDocumentEvent -> type = ::wxEVT_CHANGEDFILENAME_DOCUMENT Recieved from the a2dDocument.
        - a2dDocumentEvent -> type = ::wxEVT_CLOSE_VIEWS
        - a2dViewEvent -> type = ::wxEVT_ENABLE_VIEW sent to m_display and up, when a view is enabled or disabled.
        - ::wxEVT_REMOVE_VIEW sent to document when a view is not using the document anymore. \see SetDocument()
        - ::wxEVT_ADD_VIEW sent to document when a view start using a document  \see SetDocument()

        - ::wxEVT_CHANGEDFILENAME_VIEW See  OnChangeFilename()
        - ::wxEVT_CHANGEDTITLE_VIEW See  OnChangeTitle()

    \ingroup docview
*/
class A2DDOCVIEWDLLEXP a2dView: public a2dObject
{
    DECLARE_ABSTRACT_CLASS( a2dView )

    DECLARE_EVENT_TABLE()

public:
    //  a2dView(a2dDocument *doc = (a2dDocument *) NULL);

    //! constructor
    a2dView();

    //! destructor
    ~a2dView();

    //! construct with other view
    a2dView( const a2dView& other );

    //! A closed flag is set when a view is closed.
    /*! This flag is set just before the actual closing will take place.
        If the close is vetod, it will be reset at the end of the closing action,
        else it will stay true.
        The flag is a handy test for associated windows, to see if their
        view is already closed or not.
    */
    bool IsClosed() { return m_isClosed; }

    //! sets closed flag to true, and calls Release() view.
    /*!
        Normally Close() should be used, but in cases where a view is used temporarely and without
        a display window ( e.g using a view for rendering to bitmap ), this
        can be used to properly close a view without sending extra events.
    */
    void SetClosed();

    //! returns true if view was enabled/disabled, false if nothing done
    /*!
        Sets the view enabled or disabled, which means that no events will be processed,
        except ::wxEVT_ENABLE_VIEW and ::wxEVT_ENABLE_VIEWS.

        The ::wxEVT_ENABLE_VIEW event is sent to the view and the display window (if available).
        After that the new state is set.

        If the view is already in the requested state, nothing is done.
    */
    bool Enable( bool enable );

    //! return true if the view is enabled. See Enable()
    bool GetEnabled() { return m_viewEnabled; }

    //!Set the display window.
    /*!
        The display window can be used to display the view on a wxWindow.
        This is \b not a must, since a view might also be displayed into a bitmap etc.
        But in that case this needs to be arranged in a derived class.
        The default is using a display wxWindow.

        The window may be set in a derived view class.
        But often it is better to set it from a specialized a2dViewConnector.
        If needed in combination with a specialized a2dViewTemplate.
        This makes it easier to make general view classes, which can be used
        in several types of applications or display windows.

        \remark this function has only effect if the view its eventhandler is enabled.
        The reason is that some events like a2dCloseViewEvent are set up to the m_display window,
        during that time the event handler is disabled to prevent looping. But also the m_display
        should stay constant during that time.
    */
    virtual void SetDisplayWindow( wxWindow* display );

    //!Get the display window.
    wxWindow* GetDisplayWindow() { return m_display; }

    //! Get the id of the view (used for identification in event processing
    wxUint32 GetId() { return m_id; }

    //! get the document of the view
    /*! \return NULL if no document is set. */
    a2dDocument* GetDocument() const { return m_viewDocument; }

    //! Set the document for the view and add it to the document its viewlist.
    /*!
         Normally called by the framework.

         If the view already has a document set, the view is first removed from
         that document, sending a ::wxEVT_REMOVE_VIEW event to the old document,
         Can be received from the a2dDocument after Registering this class.

         See a2dDocumentCommandProcessor::OnRemoveView() where this event is used to reset the last active view.

         Next the new document will be set for the view, and added to the viewlist
         of the new document, sending a ::wxEVT_ADD_VIEW event to the new document.

         At last a a2dViewEvent event of type ::wxEVT_SET_DOCUMENT is sent to the a2dView.
         So if extra administrative tasks for the view are needed when setting a view to a document,
         intercept that event to do it.
    */
    void SetDocument( a2dDocument* doc );

    //! get type name of the view.
    /*!
        This name is used to couple document types to view types.
        See a2dViewTemplate.
    */
    wxString GetViewTypeName() const { return m_viewTypeName; }

    //! get the a2dViewTemplate with which this view was created
    /*!
        \see a2dViewTemplate.
    */
    a2dViewTemplate* GetViewTemplate() const { return m_viewTemplate; }

    //! to set the view template from which this view was created.
    void SetViewTemplate( a2dViewTemplate* viewtemplate ) { m_viewTemplate = viewtemplate; }

    //! set type name of the view.
    /*!
        This name is used to couple document types to view types.
        See a2dViewTemplate.
    */
    void SetViewTypeName( const wxString& name ) { m_viewTypeName = name; }

    //! force a2dDocumentEvent ::wxEVT_UPDATE_VIEWS event
    virtual void Update( unsigned int hint = 0, wxObject* hintObject = NULL );

    //! Close a view and remove the view from its document.
    /*!
        IMPORTANT there are two ways/paths through which a view can be closed.
         -# by closing the frame containing the view(s).
         -# by the docmanager closing a document and this one closing the views on the document.

        The first one will destroy the frames, and it must Close the view(s) it contains.
        The second one must make sure that the frames containing the views will be destroyed.
        This is why ::wxEVT_CLOSE_VIEW travels up in the hiearchy of windows, starting at m_display window.

        Close() generates a ::wxEVT_CLOSE_VIEW event, which is first redirected
        to the a2dDocument, and if not vetod there, to the m_display wxWindow.
        if not vetod there, the event is sent to a2dView itself.

        If not vetod in the ::wxEVT_CLOSE_VIEW handler of the view,
        the View Will be removed and released from the document.
        The view will be set Inactive.
        The m_display wxWindow will be disabled, to prevent it from sending more events.

        When the ::wxEVT_CLOSE_VIEW is sent to the m_display window, the
        a2dView itself is disabled for handling events, untill the event is
        (not)processed by the display and return.
        So event coming back to the view will not be handled during that time.
        The ::wxEVT_CLOSE_VIEW is sent to m_display, but the event is of the type
        that travels up to the parent window if not handled.
        So if not handled in the display window, it will go to the parent window.
        Eventually it arives in the wxFrame holding the windows.
        The idea is thay you do intercept ::wxEVT_CLOSE_VIEW event somewhere in the hierarchy
        of windows, and make sure the a2dView is correctly detached from the windows
        where it is used.
        In case the event is not vetod by the display
        window, it is a good habbit that it sets the display window
        of this/its view to NULL, this will prevent events coming from this view
        to the display window for sure.

        This is the clue to having view independent frames and windows.
        A specialized frame or window, can use different types of connectors to deal
        with different types of attached views.

        Always be aware of dangling pointers from the display window, which normally will/should
        have a pointer to the view. Also other classes having somehow a pointer to this view
        (which will be closed and released soon), should take care of this by intercepting the
        ::wxEVT_CLOSE_VIEW event. Therefore the display window, in such cases should
        redirect the ::wxEVT_CLOSE_VIEW event to such classes.
        Default the event will be redirected to the parent window, if not handled or skipped.

        When not vetod Close() \b the DisplayWindow m_display is set to NULL first.
        The reason for disabling the display window is to prevent it from
        sending more events to this view, which will be deleted soon.

        In any case the ::wxEVT_CLOSE_VIEW handler should check whether the view is being deleted forcibly,
        using a2dCloseViewEvent::CanVeto(). If so it needs to perform the nessecary cleanup.
        Else it may veto.

        Example:

        \code

        //the next class interecepts the ::wxEVT_CLOSE_VIEW
        void wxSomeClass::OnCloseView( a2dCloseViewEvent& event )
        {
            //test if the frame or windows where not already deleted
            //
            if ( event.GetForce() && GetViewFrame() )
            {
                event.GetView()->SetDisplayWindow( NULL );
                m_display->Disable();
                m_display->Destroy();
                m_display = NULL;
            }
        }
        \endcode

        Closing a view recursive, while its busy closing itself is not a problem.
        The owning objects may recursively call this function without problem, for that
        the m_isClosed flag is set at the beginning which prevents closing twice.
        the view object itself will be released only once in the end.
        The ::wxEVT_CLOSE_VIEW will be issued only once.

        If the view has a document set, the view is first removed from
        that document, sending a ::wxEVT_REMOVE_VIEW event to the old document.
        Can be received from the a2dDocument after Registering this class.
        See a2dDocumentCommandProcessor::OnRemoveView() where this event is used to reset the last active view.

        \remark a2dView is reference counted and will only be really deleted when it reaches zero

        \remark In wxWindows the handler function for ::wxEVT_CLOSE_WINDOW event is called when the user has tried
        to close a frame using the window manager (X) or system menu (Windows).
        The ::CLOSE_WINDOW event should normally be intercepted, and ask via
        calling a2dView::Close if it is oke to close the view. In a2dDocumentFrame this arranged.

        \remark You may call also a2dView::Close() directly, it will generate the same events.

        \return true is view was properly closed and not vetod else false

    */
    virtual bool Close( bool force = false );

    //! Extend event processing to search the view's event table
    /*! Some events are handled within the view as if the view is a window within a window.
        Therefore the wxWindow containing the view should redirect all its events to
        its view(s), when the view does not handle the event it will go to the window itself.

        Events handled how:
            - ::wxEVT_ENABLE_VIEW & ::wxEVT_ENABLE_VIEWS always processed, sent to GetEventHandler()
            - ::wxEVT_ACTIVATE sent from a2dDocumentFrame and derived classes, calls Activate()
            - ::wxEVT_SET_FOCUS sent to GetEventHandler() and calls Activate(true)
            - ::wxEVT_KILL_FOCUS sent to GetEventHandler() and calls Activate(false)
            - ::wxEVT_CLOSE_WINDOW  sent from wxWindow containing the a2dView.
            calls Close( !closeevent.CanVeto() )
            - ::wxEVT_IDLE sent from wxWindow containing the a2dView, sent to GetEventHandler()
            and to the a2dDocument.
    */
    virtual bool ProcessEvent( wxEvent& event );

    //! is called via ProcessEvent() when the view becomes active
    /*!
        Generates a ::wxEVT_ACTIVATE_VIEW with active == activate.
        When recieved by a2dDocumentCommandProcessor::OnActivateView() this way,
        the last will deactivate its current active view, using this same function.

        The event ::wxEVT_ACTIVATE_VIEW_SENT_FROM_CHILD event is sent to the m_display window or frame.
        This is the event to catch in order to set the view in the containing frame/windows.
        For example a2dDocumentFrame uses this event to set the view to which it will sent window close events.

        Can also be called directly to notify the view that it should go (in)active.

        Normally Activate() sends the a2dViewEvent of type ::wxEVT_ACTIVATE_VIEW_SENT_FROM_CHILD to m_display.

        \sa a2dViewEvent.
    */
    void Activate( bool activate );

    //! return if the view is active/
    /*!
        a view is in general active when its m_display window has the focus or is selected.
    */
    bool GetActive() { return m_active; }

    //! needs to be implemented to print a view, also so used for simple redrawing of views.
    /*!
        Normal updating the data of a view, is via OnUpdate, called from the changed a2dDocument.
        Once the view its data is up to data, m_display->Refresh() results in a paint event.
        This in a2dDocumentViewScrolledWindow and a2dDocumentViewWindow directly leads to OnDraw(),
        redrawing the data of the view on the display window.
        In a2dDocumentViewScrolledWindow and a2dDocumentViewWindow the wxPaintEvent is intercepted
        and by default redirected via its OnDraw() to the view Ondraw() function here.
        If it is a clever implementation, the above refresh should only refresh the areas
        in the view which did change.
        In m_display->GetUpdateRegion() is where you should get those regions, while the paint
        event is being handled.

        A more complicated situation is as follows.
        Paint events are generated when a wxWindow becomes visible,
        or when you draw something to it. The regions needing an update, or reported by the
        Paint event.
        The view does not need to do a complete re-drawn, only the damaged regions.
        In such a case you should Handle the paint event Your self in the m_display
        window, and redirect it to the a2dView.
        In the a2dView you can iterate over the damaged regions, and only redraw
        the view its data in those regions.
    */
    virtual void OnDraw( wxDC* dc ) = 0;

#if wxUSE_PRINTING_ARCHITECTURE

    //! called from the a2dDocumentCommandProcessor to create a wxPrintout are derived class
    /*! for printing a view
        If the printing framework is enabled in the library, this function returns a
        wxPrintout object for the purposes of printing.
        It should create a new object every time it is called;
        the framework will delete objects it creates.
        By default, this function returns an instance of wxDocPrintout,
        which prints and previews one page by calling wxView::OnDraw.

        Override to return an instance of a class other than wxDocPrintout.

        \param typeOfPrint When called from a2dDocumentCommandProcessor, the a2dPrintWhat that lead to this call.
    */
    virtual wxPrintout* OnCreatePrintout( a2dPrintWhat typeOfPrint, const wxPageSetupDialogData& pageSetupData );
#endif

    //! called from a2dDocumentPrintout when printing a view.
    /*!
        Default calls OnDraw(wxDC *dc)
    */
    virtual void OnPrint( wxDC* dc, wxObject* info );

protected:

    //! default handler for a2dDocumentEvent ::wxEVT_UPDATE_VIEWS
    /*!
        Updates the view data ( e.g the data that the view displays has changed in the document).
        The view may hold/display data which should change when the document(s) changes.
        Here this data can be updated, the internal data of the view can be updated.
        When the view will be redisplayed depends on the derived class.
        One can set a dirty flag after changing the view its data, and do the redisplay
        in idle time, where this flag is checked. But often there is no need
        to do this, since the view can directly update its Display window, or
        tell it that it needs to update itself.
        Therefore the default here simply calls m_display->Refresh(), which leads to a
        paint event on the display window, and that might/should call a2dView::Ondraw().
    */
    void OnUpdate( a2dDocumentEvent& event );

    //! default handler for a2dDocumentEvent of type ::wxEVT_ENABLE_VIEWS
    /*!
        This enable or disables the view by calling Enable( event.GetEnable() ).
        But only if the a2dDocument from which this document was sent, is the
        one the a2dView is using.
        If the view from which the event originated is specified in the event,
        that view will not process the event, all others will.
    */
    void OnEnableViews( a2dDocumentEvent& event );

    //! handler for the ::wxEVT_REPORT_VIEWS
    /*!
        The handlers adds this a2dView to the reportlist of the event, if
        it depends on the document sending the event. The default simply
        checks that the view its document is the one sending the event, and if so
        adds it. In case your view displays info of several documents, you should
        intercept this event to implement this behaviour.
    */
    void OnReport( a2dDocumentEvent& event );

    //! default handler for EVT_ENABLE_VIEW event.
    void OnEnable( a2dViewEvent& event );

    //! default handler for file changed event sent from a2dDocument with type wxEVT_CHANGEDFILENAME_DOCUMENT
    /*!
        Creates new event of type wxEVT_CHANGEDFILENAME_VIEW, which also knows the view now.
        Sends the event to the DisplayWindow, there or in its parent hiearchy it can be intercepted.
    */
    void OnChangeFilename( a2dDocumentEvent& event );

    //! default handler for file changed event sent from a2dDocument with type wxEVT_CHANGEDTITLE_DOCUMENT
    /*!
        Creates new event of type wxEVT_CHANGEDTITLE_VIEW, which also knows the view now.
        Sends the event to the DisplayWindow, there or in its parent hiearchy it can be intercepted.
    */
    void OnChangeTitle( a2dDocumentEvent& event );

    //! ::wxEVT_CLOSE_VIEW event handler to do cleanup/veto Close of the view.
    /*!
        The default handler does nothing yet.

        The Handler for ::wxEVT_CLOSE_VIEW, resulting from view close via
        a2dView:Close() ( either via a2dDocument or a wxFrame containing the view)

        One may intercept ::wxEVT_CLOSE_VIEW event in a derived class
        to Clean up and Destroy windows used for displaying the view.
        But it is often better to do this in a wxWindow derived class which contains the view.
        Therefore the ::wxEVT_CLOSE_VIEW is first sent to the m_display wxWindow of the a2dView.

        The best is to use a a2dViewConnector to create frames and windows to hold a view,
        and a2dDocumentFrame derived class to handle Closing of a2dView's.
        It will recieve the ::wxEVT_CLOSE_VIEW via the wxWindow
        containing the a2dView which is about to be closed.
        This makes it easy to use the same view class in other applications.

        On closing of a wxFrame, you may decide to Close the window by intercepting
        EVT_CLOSE, and from there call a2dView::Close.
        It depends on the application how the views are closed.

        In any case Windows should be Destroyed, not really deleted.
        Destroy only flags for deletion, this means that they will really
        be deleted in idle time by wxWindows.
    */
    void OnCloseView( a2dCloseViewEvent& event );

    //! handler for ::wxEVT_DISCONNECT_ALLVIEWS
    /*!
        The ::wxEVT_DISCONNECT_ALLVIEWS  is generated by the a2dDocument when it disconnects all views.
        This can be when closing the document, or just to attach all views to another document.
    */
    void OnDisConnectView( a2dDocumentEvent& event );

    //! called on (de)activating (wxWindow) containing the view.
    //void OnActivate(wxActivateEvent& event);

    //! when a view gets the focus ( when only one view per window
    /*! that view gets the focus if the window gets the focus )
        Else the window getting the focus should decide which view gets the focus.
    */
    void OnSetFocus( wxFocusEvent& event );

    //! when a view goes out of focus ( when only one view per window
    /*! that view goes out of focus  if the window goes out of focus )
        Else the window getting the focus should decide which view goes out of focus.
    */
    void OnKillFocus( wxFocusEvent& event );

    //! close flag, to indicate that the view is being closed are is closed.
    bool m_isClosed;

    //! the document to which this view belongs.
    a2dSmrtPtr<a2dDocument> m_viewDocument;

    //! set via the a2dViewTemplate on generation of this view.
    wxString m_viewTypeName;

    //! view was created from this view template or if not used NULL
    a2dViewTemplatePtr  m_viewTemplate;

    //! true if this view is activated ( its display window has the focus and recieves mouse events )
    bool m_active;

    //! Is the view enabled?
    /*!
        true if this view is enabled and will process all events.
        else only ::wxEVT_ENABLE_VIEW and ::wxEVT_ENABLE_VIEWS will be processed.
    */
    bool m_viewEnabled;

    //! In case of using a window for displaying the view, this holds the window.
    wxWindow* m_display;

    //! This identifies the view in event processing
    wxUint32 m_id;

    //! this is used to keep the view alive if no other references are there.
    //! This untill the view is closed by calling Close() or SetClosed()
    a2dSmrtPtr<a2dView> m_keepalive;

private:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }

    virtual a2dObject* SmrtPtrOwn();
    virtual bool SmrtPtrRelease();
    friend class a2dSmrtPtrBase;
};


//! The a2dDocumentTemplate class is used to model the relationship between a document class and files
/*!
    Next to that, a a2dViewConnector class pointer
    is used to generate new views for new documents, or whatever the a2dViewConnector finds
    appropriate to do when a new document is created by the a2dDocumentCommandProcessor via a a2dDocumentTemplate.

    Normally new views are created by the a2dViewConnector class via a a2dViewTemplate.
    When and where this new view will be displayed, is up to the a2dViewConnector.

    After CreateDocument created a new document,
    it sends a ::wxEVT_POST_CREATE_DOCUMENT event to the a2dViewConnector class m_connector.
    The a2dViewConnector derived class may intercept the event to
    created new a2dView's for the new a2dDocument.
    This is what the default handler ::wxEVT_POST_CREATE_DOCUMENT
    in a2dViewConnector::OnPostCreateDocument() does do.
    It first uses the a2dDocumentCommandProcessor to display the available a2dViewTemplate's
    for the new created document type.
    The user chooses one, and that a2dViewTemplate will be used
    to create the first view on the document with a2dViewTemplate::CreateView().
    But in other situations, one might prefer to hook the new document into an existing view.
    The existing view its current document is disconnected from the view, and the new one
    is connected to the view.
    a2dViewTemplate::CreateView() is what generates the ::wxEVT_POST_CREATE_VIEW which is also
    redirected to a a2dViewConnector. This event may or may no be handled by a connector.
    It depends on the application, how a connector wants to connect new views into the application.

    \remark It is possible that newly created documents do not need a view.
    Therefore the connector class pointer may be NULL.

    \sa a2dDocumentTemplateEvent
    \sa a2dViewTemplateEvent

    \ingroup docview
*/
class A2DDOCVIEWDLLEXP a2dDocumentTemplate: public a2dObject
{

    DECLARE_CLASS( a2dDocumentTemplate )

public:

    //! Associate document and view types. They're for identifying what view is associated with what template/document type
    /*!
        Constructor. Create instances dynamically near the start of your
        application after creating a a2dDocumentCommandProcessor instance, and before doing
        any document or view operations.

        \param descr is a short description of what the template is for. This string
        will be displayed in the file filter list of Windows file selectors.

        \param filter is an appropriate file filter such as *.txt OR "*.cal;*.gds" for more the one.

        \param dir is the default directory to use for file selectors.

        \param ext is the default file extension (such as txt).

        \param docTypeName is a name that should be unique for a given type of document,
        used for gathering a list of views relevant to a particular document.

        \param docClassInfo is a pointer to the run-time document class information as returned
        by the CLASSINFO macro, e.g. CLASSINFO(MyDocumentClass). If this is not supplied,
        you will need to derive a new a2dDocumentTemplate class and override the CreateDocument
        member to return a new document instance on demand.

        \param connector connector to use to attach this type of template to GUI.

        \param flags is a bit list of the following:

            - a2dTemplateFlag::VISIBLE The template may be displayed to the user in dialogs.
            - wxREFTEMPLATE_INVISIBLE The template may not be displayed to the user in dialogs.
            - a2dTemplateFlag::DEFAULT Defined as a2dTemplateFlag::VISIBLE.

        \param handlerIn IO handler needed to read this type of files into the document.
        \param handlerOut IO handler needed to write this type of files from the document.

    */
    a2dDocumentTemplate(
        const wxString& descr,
        const wxString& filter,
        const wxString& dir,
        const wxString& ext,
        const wxString& docTypeName,
        wxClassInfo* docClassInfo = ( wxClassInfo* ) NULL,
        a2dViewConnector* connector = ( a2dViewConnector* ) NULL,
        long flags = a2dTemplateFlag::DEFAULT,
        a2dIOHandlerStrIn* handlerIn = NULL,
        a2dIOHandlerStrOut* handlerOut = NULL );

    //! Destructor.
    ~a2dDocumentTemplate();

    //! Creates a new document.
    /*!
        Creates a new instance of the associated document class.
        If you have not supplied a wxClassInfo parameter to the template constructor,
        you will need to override this function to return an appropriate document instance.

        After creating a new document two event are generated:

            - a2dDocumentEvent of type ::wxEVT_CREATE_DOCUMENT sent to document
            - a2dDocumentTemplateEvent of type ::wxEVT_POST_CREATE_DOCUMENT sent to a2dViewConnector.
    */
    virtual a2dDocument* CreateDocument( const wxFileName& path, a2dDocumentFlagMask flags = a2dREFDOC_NON );

    //! sends a wxEVT_PRE_ADD_DOCUMENT event to m_connector if available.
    /*!
        This event is sent to the m_connector a2dViewConnector of the template, and the sender/eventObject is this template.
        It is sent just before a new created document is added to the framework.
        The a2dViewConnector can react to this action by closings existing documents and disconnect frames and views.
        At this stage the  a2dDocumentCommandProcessor() its current document has not changed yet.

        \param newDoc the new document which was created
        \param flags  some flags telling for what purpose the new document was created.  e.g a2dREFDOC_NEW
    */
    a2dDocument* SentPreAddCreatedDocumentEvent( a2dDocument* newDoc, a2dDocumentFlagMask flags );

    //! sends a wxEVT_POST_CREATE_DOCUMENT called in general from a2dDocumentCommandProcessor() when a new document was created.
    /*!
        This event is sent to the m_connector a2dViewConnector of the template, and the sender/eventObject is this template.
        The a2dViewConnector can react in several ways to this action:
        - create one or more  new a2dView's ( if wanted a a2dViewTemplate can help to choose one )
        - use one or more existing a2dView's
        - create no a2dView at all
        - merge or copy the contents of the new a2dDocument, and directly close this new document, of course no new views generated then.
        - next to creating one or more a2dViews or re-using one or more a2dViews, also the a2dDocumentFrame's and/or
        a2dDocumentViewWindow's can be created or reused to connect the views into.

        The default implementation see a2dViewConnector::OnPostCreateDocument().

        In a derived a2dViewConnector you can implement your own handler, to treat the new document in ways more
        appropriate to your application.

        \param newDoc the new document which was created
        \param flags  some flags telling for what purpose the new document was created.  e.g a2dREFDOC_NEW

    */
    bool SentPostCreateDocumentEvent( a2dDocument* newDoc, a2dDocumentFlagMask flags );

    //! Returns the default file extension for the document data, as passed to the document template constructor.
    wxString GetDefaultExtension() const { return m_defaultExt; }

    //! Returns the text description of this template, as passed to the document template constructor
    wxString GetDescription() const { return m_description; }

    //! Returns the default directory, as passed to the document template constructor.
    wxString GetDirectory() const { return m_directory; }

    //! Returns the file filter, as passed to the document template constructor.
    wxString GetFileFilter() const { return m_fileFilter; }

    //! Returns the flags, as passed to the document template constructor.
    const a2dTemplateFlag& GetFlags() const { return m_flags; }

    //! Sets the internal document template flags (see the constructor description for more details).
    void SetFlags( a2dTemplateFlagMask flags ) { m_flags.SetFlags( flags ); }

    //! Sets the internal document template flags (see the constructor description for more details).
    void SetFlags( a2dTemplateFlag flags ) { m_flags = flags; }

    //! does template mask match
    bool CheckMask( a2dTemplateFlagMask mask ) { return m_flags.CheckMask( mask ); }

    //! Returns the document type name, as passed to the document template constructor.
    wxString GetDocumentTypeName() const { return m_docTypeName; }

    //! Sets the file filter.
    void SetFileFilter( const wxString& filter ) { m_fileFilter = filter; }

    //! Sets the default directory.
    void SetDirectory( const wxString& dir ) { m_directory = dir; }

    //! Sets the template description.
    void SetDescription( const wxString& descr ) { m_description = descr; }

    //! Sets the default file extension.
    void SetDefaultExtension( const wxString& ext ) { m_defaultExt = ext; }

    //! Returns true if the document template can be shown in user dialogs, false otherwise.
    void SetVisible( bool value ) { m_flags.SetFlags( a2dTemplateFlag::VISIBLE ); }

    //! Returns true if the document template is marked , false otherwise.
    void SetMarked( bool value ) { m_flags.SetFlags( a2dTemplateFlag::MARK ); }

    //! Returns true if the document template can be shown in user dialogs, false otherwise.
    bool IsVisible() const { return m_flags.GetFlag( a2dTemplateFlag::VISIBLE ); }

    //! Returns true if the document template is marked , false otherwise.
    bool IsMarked() const { return m_flags.GetFlag( a2dTemplateFlag::MARK ); }

    //! to test if a file that needs to be loaded, is suitable for this template.
    /*!
        The default implementation is testing the file extension of the template against the
        file extension in the path.
        When no extension is given in the file and a m_ioHandlerStrIn is set, the a2dIOHandler::CanLoad()
        is used to test the file contents for that io handler.
        At last the  m_fileFilter is search for the path extension, if found return true.
    */
    virtual bool FileMatchesTemplate( const wxString& path );

    //! Get Input handler used to store the document using this template
    virtual a2dIOHandlerStrIn* GetDocumentIOHandlerStrIn() { return m_ioHandlerStrIn; }

    //! Get output handler used to save the document using this template
    virtual a2dIOHandlerStrOut* GetDocumentIOHandlerStrOut() { return m_ioHandlerStrOut; }

    //! if there is an io handler, and it can load the document of this template, return it
    virtual a2dIOHandlerStrIn* GetDocumentIOHandlerForLoad( a2dDocumentInputStream& stream, a2dDocument* document );

    //! if there is an io handler, and it can save the document of this template, return it
    /*!
        \param filename when not empty, it is/can be used to choose io handler also based on the extension of the filename.
        \param document the document that needs to be saved.
    */
    virtual a2dIOHandlerStrOut* GetDocumentIOHandlerForSave( const wxString& filename, a2dDocument* document );

    //! Set Inputhandler used to load the document using this template
    virtual void SetDocumentIOHandlerStrIn( a2dIOHandlerStrIn* handler ) { m_ioHandlerStrIn = handler; }

    //! Set output handler used to save the document using this template
    virtual void SetDocumentIOHandlerStrOut( a2dIOHandlerStrOut* handler ) { m_ioHandlerStrOut = handler; }

    //! return the a2dViewConnector that is used for sending the ::wxEVT_POST_CREATE_DOCUMENT too.
    a2dViewConnector* GetViewConnector() { return m_connector; }

#if wxUSE_PRINTING_ARCHITECTURE
    wxPageSetupDialogData* GetPageSetupData( void ) const { return m_pageSetupData; }
    void SetPageSetupData( const wxPageSetupDialogData& pageSetupData ) { *m_pageSetupData = pageSetupData; }
    void SetPageSetupData( wxPageSetupDialogData* pageSetupData );
#endif

    wxClassInfo* GetDocumentClassInfo() const { return m_docClassInfo; }

    //! if set this will be used instead of m_docClassInfo to initiate new documents
    a2dDocument* GetDocumentCreate() { return m_documentCreate; }

    //! if set this will be used instead of m_docClassInfo to initiate new documents
    /*!
        Id set a clone of this document will be used to create new documents through this
        a2dDocumentTemplate. This is handy when a document has a lot of initial settings which
        can now be set once here. So for each document type/purpose you can have unique settings.

        \remark m_docClassInfo still needs ot be set.
    */
    void SetDocumentCreate( a2dDocument* documentCreate ) { m_documentCreate = documentCreate; }

protected:

    a2dObject* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    //! The flags passed to the constructor.
    a2dTemplateFlag   m_flags;

    //! The file filter (such as *.txt) to be used in file selector dialogs.
    wxString          m_fileFilter;

    //! The default directory for files of this type.
    wxString          m_directory;

    //! A short description of this template
    wxString          m_description;

    //! The default extension for files of this type
    wxString          m_defaultExt;

    //! The named type of the document associated with this template.
    wxString          m_docTypeName;

    //! Run-time class information that allows document instances to be constructed dynamically.
    wxClassInfo*      m_docClassInfo;

    //! if set this will be used instead of m_docClassInfo to initiate new documents
    a2dDocumentPtr m_documentCreate;

    //! used for reading the document from streams.
    a2dSmrtPtr<a2dIOHandlerStrIn> m_ioHandlerStrIn;

    //! used for writing the document to streams.
    a2dSmrtPtr<a2dIOHandlerStrOut> m_ioHandlerStrOut;

    //! class used to connect new views with Frames and windows.
    a2dSmrtPtr<a2dViewConnector> m_connector;

#if wxUSE_PRINTING_ARCHITECTURE
    //! this is the printer page setup data for all documents with this template
    wxPageSetupDialogData*    m_pageSetupData;
#endif

private:

    virtual a2dObject* SmrtPtrOwn();
    virtual bool SmrtPtrRelease();
    friend class a2dSmrtPtrBase;
};


//! Automatic template, which searches a suitable handler for the document.
/*!
    a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocTemplates() is used
    to find a template which fits the document type of the template.

    An extra test is performed in GetDocumentIOHandlerForLoad() GetDocumentIOHandlerForSave(),
    to see if that template can be used for saving or loading the document.

    This template is handy in use when you what an All Files template.
    The document with this template, will automatically choose the right handler for files you choose.
    Extension is not important.

*/
class A2DDOCVIEWDLLEXP a2dDocumentTemplateAuto: public a2dDocumentTemplate
{

    DECLARE_CLASS( a2dDocumentTemplateAuto )

public:

    a2dDocumentTemplateAuto(
        const wxString& descr,
        const wxString& filter,
        const wxString& dir,
        const wxString& ext,
        const wxString& docTypeName,
        wxClassInfo* docClassInfo = ( wxClassInfo* ) NULL,
        a2dViewConnector* connector = ( a2dViewConnector* ) NULL,
        long flags = a2dTemplateFlag::DEFAULT );

    //! Destructor.
    ~a2dDocumentTemplateAuto();

    //! if there is an io handler, and it can load the document of this template, return it
    virtual a2dIOHandlerStrIn* GetDocumentIOHandlerForLoad( a2dDocumentInputStream& stream, a2dDocument* document );

    //! if there is an io handler, and it can save the document of this template, return it
    /*!
        \param filename when not empty, it is/can be used to choose io handler also based on the extension of the filename.
        \param document the document that needs to be saved.
    */
    virtual a2dIOHandlerStrOut* GetDocumentIOHandlerForSave( const wxString& filename, a2dDocument* document );

};

//! The a2dViewTemplate class is used to model the relationship between a  document class and a view class.
/*!
    Next to that, a a2dViewConnector class pointer
    is used to connect new views and documents into a wxFrame and/or wxWindow.
    The a2dDocumentCommandProcessor can be used to display a dialog with a list of available
    a2dViewTemplate's for a certain document type.
    Using the choosen a2dViewTemplate, this template will then be used to create
    a view on the document. This happens in CreateView().

    When, how and where this new view will be displayed, is up to the a2dViewConnector.

    CreateView() sents two events:
      - ::wxEVT_CREATE_VIEW event sent to the new view.
      - ::wxEVT_POST_CREATE_VIEW event sent to the a2dViewConnector class m_connector.

    The a2dViewConnector derived class often intercepts the event to connect
    the created a2dView into (newly created or existing) Frames or Windows.

    It is possible that newly created view do not need to be plugged into any Window or Frame.
    Therefore the connector class pointer may be NULL.

    \see a2dDocumentTemplate

    \see a2dViewConnector

    \ingroup docview
*/
class A2DDOCVIEWDLLEXP a2dViewTemplate: public a2dObject
{

    DECLARE_CLASS( a2dViewTemplate )

public:

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    //! Associate document and view types. They're for identifying what view is associated with what template/document type
    /*!
        Constructor. Create instances dynamically near the start of your
        application after creating a a2dDocumentCommandProcessor instance, and before doing
        any document or view operations.

        \param descr is a short description of what the template is for. This string
        will be displayed in the file filter list of Windows file selectors.

        \param docTypeName is a name that should be unique for a given type of document,
        used for gathering a list of views relevant to a particular document.

        \param viewTypeName is a name that should be unique for a given view.

        \param viewClassInfo is a pointer to the run-time view class information as
        returned by the CLASSINFO macro, e.g. CLASSINFO(MyViewClass).
        If this is not supplied, you will need to derive a new a2dDocumentTemplate
        class and override the CreateView member to return a new view instance on demand.

        \param connector is a pointer to a a2dViewConnector class, which will be used in
        the default handler for the ::wxEVT_POST_CREATE_VIEW event, OnPostCreateView to redirect
        the event to the a2dViewConnector class. If NULL the new view is not plugged into anything.

        \param flags is a bit list of the following:

            - a2dTemplateFlag::VISIBLE The template may be displayed to the user in dialogs.
            - wxREFTEMPLATE_INVISIBLE The template may not be displayed to the user in dialogs.
            - a2dTemplateFlag::DEFAULT Defined as a2dTemplateFlag::VISIBLE.

        \param size size sets initial size for views generated from this template

    */
    a2dViewTemplate(
        const wxString& descr,
        const wxString& docTypeName,
        const wxString& viewTypeName,
        wxClassInfo* viewClassInfo = ( wxClassInfo* ) NULL,
        a2dViewConnector* connector = ( a2dViewConnector* ) NULL,
        long flags = a2dTemplateFlag::DEFAULT,
        const wxSize& size = wxSize( 0, 0 ) );

    //! Destructor.
    ~a2dViewTemplate();

    //! Creates a new view for the given document.
    /*!
        Creates a new instance of the associated view class. If you have not supplied a
        wxClassInfo parameter to the template constructor, you will
        need to override this function to return an appropriate view instance.

        After creating a new view using wxClassInfo two events are generated:

            - a2dViewEvent of type ::wxEVT_CREATE_VIEW
            - a2dViewTemplateEvent of type ::wxEVT_POST_CREATE_VIEW
    */
    virtual a2dView* CreateView( a2dDocument* doc, a2dDocumentFlagMask flags = a2dREFDOC_NON );

    //! Returns the text description of this template, as passed to the document template constructor
    wxString GetDescription() const { return m_description; }

    //! Sets the template description.
    void SetDescription( const wxString& descr ) { m_description = descr; };

    //! Returns the view type name, as passed to the document template constructor.
    wxString GetViewTypeName() const { return m_viewTypeName; }

    //! Returns the document type name, as passed to the document template constructor.
    wxString GetDocumentTypeName() const { return m_docTypeName; }

    //! Returns the flags, as passed to the document template constructor.
    const a2dTemplateFlag& GetFlags() const { return m_flags; }

    //! Sets the internal document template flags (see the constructor description for more details).
    void SetFlags( a2dTemplateFlagMask flags ) { m_flags.SetFlags( flags ); }

    //! Sets the internal document template flags (see the constructor description for more details).
    void SetFlags( a2dTemplateFlag flags ) { m_flags = flags; }

    //! does template mask match
    bool CheckMask( a2dTemplateFlagMask mask ) { return m_flags.CheckMask( mask ); }

    //! Returns true if the document template can be shown in user dialogs, false otherwise.
    void SetVisible( bool value ) { m_flags.SetFlags( a2dTemplateFlag::VISIBLE ); }

    //! Returns true if the document template is marked , false otherwise.
    void SetMarked( bool value ) { m_flags.SetFlags( a2dTemplateFlag::MARK ); }

    //! Returns true if the document template can be shown in user dialogs, false otherwise.
    bool IsVisible() const { return m_flags.GetFlag( a2dTemplateFlag::VISIBLE ); }

    //! Returns true if the document template is marked , false otherwise.
    bool IsMarked() const { return m_flags.GetFlag( a2dTemplateFlag::MARK ); }

    a2dViewConnector* GetViewConnector() { return m_connector; }

    //! set size to use for new created views
    void SetInitialSize( const wxSize& size ) { m_initialSize = size; }

    //! Get size used for new created views
    wxSize& GetInitialSize() { return m_initialSize; }

#if wxUSE_PRINTING_ARCHITECTURE
    wxPageSetupDialogData* GetPageSetupData( void ) const { return m_pageSetupData; }
    void SetPageSetupData( const wxPageSetupDialogData& pageSetupData ) { *m_pageSetupData = pageSetupData; }
    void SetPageSetupData( wxPageSetupDialogData* pageSetupData );
#endif

protected:

    a2dObject* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    //! size for new view
    wxSize  m_initialSize;

    //! The flags passed to the constructor.
    a2dTemplateFlag   m_flags;

    //! A short description of this template
    wxString          m_description;

    //! The named type of the document associated with this template.
    wxString          m_docTypeName;

    //! The named type of the view associated with this template.
    wxString          m_viewTypeName;

    //! For dynamic creation of appropriate instances.
    wxClassInfo*      m_viewClassInfo;

    //! class used to connect new views with Frames and windows.
    a2dSmrtPtr<a2dViewConnector> m_connector;

#if wxUSE_PRINTING_ARCHITECTURE
    //! this is the printer page setup data for all views with this template
    wxPageSetupDialogData*    m_pageSetupData;
#endif

private:

    virtual a2dObject* SmrtPtrOwn();
    virtual bool SmrtPtrRelease();
    friend class a2dSmrtPtrBase;
};

//! This class is used to connect newly created a2dView's to a wxFrame and/or wxWindow
/*!
    a2dDocumentTemplate generates new documents and a2dViewTemplate generate new views
    on a document. The new documents and views use the a2dViewConnector to link
    themselfs to the windows and/or frames of the application.
    The a2dViewConnector is an abstract class, so a new connector class needs
    to be derived. This derived class has complete freedom on how to organize new views
    into the application. It may decide to use existing frame or windows, or generate
    new frames and/or windows. In the end it must set one wxWindow pointer
    in the a2dView, which will be used by the view to display itself.
    The a2dViewConnector may also decide to use existing views for connecting the new document to.
    In that case no new views will be created.

    The approach to have a a2dViewConnector decide how to connect views to frames/windows,
    is good for making a2dView independent classes. In such clases, the views can be switched
    with other views.
    Think here of a complete graphical editor wxFrame derived class,
    which edits the view which has the focus.
    It has all the menu's and tools to do it, but does not know yet if there is one or many views
    inside itself. It only knows how to edit one a2dView which is part of the editor frame.
    Different types of views can be switched into the editor frame,
    where each view type can define a different manner of rendering a drawing.
    One might use the standard wxDC based API for drawing in one view type,
    and another using an antialiasing rendering engine. The editor does not need to know what
    view is used internal.

    The a2dViewConnector class is created before setting up a2dDocumentTemplate, and given as a pointer to
    a2dDocumentTemplate and a2dViewTemplate.

    a2dDocumentTemplate sends a ::wxEVT_POST_CREATE_DOCUMENT event after it created a new a2dDocument.
    This event should be catched by the a2dViewConnector.
    The default handler calls CreateView on a choosen a2dViewTemplate from a list,
    leading to a ::wxEVT_POST_CREATE_VIEW being sent to this or another a2dViewConnector class.
    The connector receives this event, and the new view can be plugged into/attached to a window etc.

    WxViewTemplateRef sends a ::wxEVT_POST_CREATE_VIEW event after it created a new a2dView.
    This event may be catched by the a2dViewConnector.
    One can handle this event in a derived a2dViewConnector class, and connect the new a2dView
    somehow into the application its wxFrame's or wxWindow's.
    The normal action should be to plug the new view into a wxFrame, and/or create a wxWindow
    for the a2dView to use as DisplayWindow.
    The wxFrame and DisplayWindow may already exist or will be created each time.
    This depends on the application and what a2dViewConnector is used.

    The new a2dView in the end will have a pointer set to its display window.
    It is up to the type of a2dViewConnector ( which the application uses ), to decide what to do
    with new a2dView's and/or new a2dDocument's.

    A different approach to connect new documents into the application, is to not intercept
    ::wxEVT_POST_CREATE_VIEW. The connector will either directly use newly created views or already
    existing views to connect the new document to. This should all be arranged in the handler
    for ::wxEVT_POST_CREATE_DOCUMENT. In a one wxFrame application with one or more subwindows containing
    a view, this is the better way to connect new documents into the application.

    The a2dView uses its DisplayWindow to redirect events ( e.g. ::wxEVT_CLOSE_VIEW ) to
    the window(s) and frame containing the view.
    A special a2dDocumentFrame can be used to interecept such events, and handle them correctly.
    But you can decide to use your own frame class.

    Events sent to this class:

        - ::wxEVT_POST_CREATE_DOCUMENT from template just after creation of a document
        - ::wxEVT_POST_CREATE_VIEW from template just after creation of a view

    \ingroup docview
*/
class A2DDOCVIEWDLLEXP a2dViewConnector: public a2dObject
{

    DECLARE_ABSTRACT_CLASS( a2dViewConnector )
    DECLARE_EVENT_TABLE()

public:

    //! Constructor
    a2dViewConnector();

    //! destructor
    virtual ~a2dViewConnector() = 0;

    a2dViewConnector( const a2dViewConnector& other );

    //! set position to use for new created views or frames or windows
    void SetInitialPosition( const wxPoint& position ) { m_initialPos = position; }

    //! set size to use for new created views or frames or windows
    void SetInitialSize( const wxSize& size ) { m_initialSize = size; }

    //! set style to use for new created frames or windows
    void SetInitialStyle( long style ) { m_initialStyle = style; }

    //! Get position used for new created views or frames or windows
    wxPoint& GetInitialPosition() { return m_initialPos; }

    //! GGet size used for new created views or frames or windows
    wxSize& GetInitialSize() { return m_initialSize; }

    //! Get style used for new created a2dDocumentFrame
    long GetInitialStyle() { return m_initialStyle; }

    a2dViewTemplateList& GetViewTemplates() { return m_viewTemplates; }

    //! add a reference to a a2dViewTemplate to the wxViewTemplateReflist
    /*!
        When Associating a a2dViewTemplate, you are responsible for Disassociating that ViewTemplateRef.
        Not doing this will result in a memory leak, since instances of this class will not be deleted.
        The reason is that the ViewTemplateRef itself also Owns the connector, and therefore the reference count
        will never reach zero, if still associated ViewTemplateRef templates to exist.
    */
    void AssociateViewTemplate( a2dViewTemplate* temp );

    //! remove a reference to a a2dViewTemplate from the wxViewTemplateReflist
    void DisassociateViewTemplate( a2dViewTemplate* temp );

    //! remove All references to a a2dViewTemplate from the wxViewTemplateReflist
    void DisassociateViewTemplates();

    //! create a new view from a list of a2dViewTemplate's
    /*!
        Creates a a2dDocumentTemplateEvent of type ::wxEVT_POST_CREATE_DOCUMENT to be processed by this connector.
        This is handy when one already created/has a document, and wants to create a view for it.
        In general document are created via a2dDocumentTemplate.

    */
    virtual a2dView* CreateView( a2dDocument* doc, const wxString& viewTypeName = wxT( "" ),
                                 a2dDocumentFlagMask flags = a2dREFDOC_NON ,
                                 a2dTemplateFlagMask mask = a2dTemplateFlag::DEFAULT );

    //! handler for ::wxEVT_DISCONNECT_VIEW
    /*!
        The ::wxEVT_DISCONNECT_ALLVIEWS  is generated by the a2dDocument when it wants to disconnects all views.
        This can be when closing the document, or just to attach all views to another document.
        a2dView does intercept this event, and sends a ::wxEVT_DISCONNECT_VIEW via its a2dViewTemplate member to the
        a2dViewConnector of that specific template.
        In short the a2dViewTemplate and a2dViewConnetor which were used to generate the a2dView and connect the view
        into the GUI of the application, is also used to disconnect it later.

        Here the view connector is responsible for disconnecting view from documents, and if wanted to destroy
        the a2dView and/or the a2dDocumentFrame, a2dDocumentViewWindow, a2dDocumentViewScrolledWindow or whatever GUI
        interface was used to contain the view.
    */
    void OnDisConnectView(  a2dTemplateEvent& event );

    //! when view is disconnected is called,
    //! the document is only disconnected from the document and kept alive
    void SetOnlyDisconnect( bool OnlyDisconnect ) { m_OnlyDisconnect = OnlyDisconnect; }

    //! When true, a2dView::OnDisConnectView() only disconnects view from document, and does not close it.
    bool GetOnlyDisconnect() { return m_OnlyDisconnect; }

private:

    //! Default Handler Creates the first a2dView on the a2dDocument
    /*!
        Calls a2dDocumentCommandProcessor::SelectViewType() to choose a proper template for the new document.
        That uses the m_viewTemplates or the a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates()
        to display a list of view Templates for the given document.
        After that, the choosen view template is used to create a new a2dView via the a2dViewTemplate::CreateView() function.
        a2dViewTemplate::CreateView() uses its connector ( in general the same is the connector which created the document )
        to sent a wxEVT_POST_CREATE_VIEW to the connector. Eventually the connector will/can create/reuse windows and frames
        for the new views.
    */
    void OnPostCreateDocument( a2dTemplateEvent& event );

    //! Gives an error, telling the user to handle this event in a derived connector.
    //! default handler for ::wxEVT_POST_CREATE_VIEW
    void OnPostCreateView( a2dTemplateEvent& event );

    void OnCloseView( a2dCloseViewEvent& event );

protected:

    a2dObject* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
    {
        wxFAIL_MSG( wxT( "cannot clone a2dIOHandler" ) );
        return NULL;
    }

    //! position for new frames
    wxPoint m_initialPos;

    //! size for new frames
    wxSize  m_initialSize;

    //! style for new frames
    long m_initialStyle;

    //! templates for views
    a2dViewTemplateList m_viewTemplates;

    //! see SetOnlyDisconnect()
    bool m_OnlyDisconnect;

};

//! uses a wxFrame to hold new views or to create child wxFrame's to hold new views.
/*!

    If createChildframe == true
    Creates for every new view reported by ::wxEVT_POST_CREATE_VIEW a new a2dDocumentFrame as child of GetDocFrame()
    else
    Switches every new view reported by ::wxEVT_POST_CREATE_VIEW into an existing a2dDocumentFrame GetDocFrame()
    The old a2dView for that a2dDocumentFrame stays on the wxDocument.

    GetFrame() / m_docframe can be the Top Frame in a window application or Any other Child Frame.
*/
class A2DDOCVIEWDLLEXP a2dFrameViewConnector: public a2dViewConnector
{

    DECLARE_ABSTRACT_CLASS( a2dFrameViewConnector )

public:

    //! Constructor
    a2dFrameViewConnector();

    //! destructor
    virtual ~a2dFrameViewConnector() {};

    a2dFrameViewConnector( const a2dFrameViewConnector& other );

    //! The pointer m_docframe will be initialized.
    /*! An application can have only one Parent a2dDocumentFrame in the application, which is the Frame which
        other Child a2dDocumentFrame(s) have as Parent. The Parent Frame is in general created in
        bool wxApp::OnInit(void).

        Depending on the derived connector the a2dDocumentFrame may be used in a parent fashion
        or in a child fashion.
        The derived a2dFrameViewConnector may use the parent frame to generate new child frames for it.
        And in another derived a2dFrameViewConnector, the same frame may be used to switch views.

        The docframe can display a document using a a2dView.

        \param docframe The parent frame of the whole application OR a child frame

        \param createChildframe new views will get its own frame as child of m_docframe
    */
    void Init( wxFrame* docframe, bool createChildframe = true );

    //! get the frame
    wxFrame* GetFrame() { return m_docframe; }

    //! set the frame
    /*!
        The wxFrame can be the parent/top frame in the application or a Child Frame
        of this application. See a2dDocumentFrame how to create a parent or child frame.
    */
    void SetFrame( wxFrame* docframe ) { m_docframe = docframe; }

    void SetCreateChild( bool createChildframe ) { m_createChildframe = createChildframe; }

    bool GetCreateChild() { return m_createChildframe; }

private:

    void OnPostCreateView( a2dTemplateEvent& event );

protected:

    //! the frame which receives events and often redirect those to the document manager.
    wxFrame* m_docframe;

    bool m_createChildframe;
};

class A2DDOCVIEWDLLEXP a2dDocumentViewScrolledWindow;
class A2DDOCVIEWDLLEXP a2dDocumentViewWindow;

// ----------------------------------------------------------------------------
// a2dWindowConnector
// ----------------------------------------------------------------------------

//! a connector which holds one pointer to a wxWindow derived class being the template parameter.
/*!
    This connector connects new views to a wxWindow derived class which must have a Setview() member.

    ::wxEVT_POST_CREATE_VIEW  is handled by a2dWindowConnector<Twindow>::OnPostCreateView(),
    and just sets the view to the m_display window.
*/
template<class Twindow>
class a2dWindowConnector : public a2dViewConnector
{
    DECLARE_EVENT_TABLE()

public:
    typedef Twindow TType;

    //! Constructor
    a2dWindowConnector();

    ~a2dWindowConnector() {};

    //! window used by view to display itself.
    Twindow* GetDisplayWindow() { return m_display; }

    //! get window used by view to display itself.
    void SetDisplayWindow( Twindow* window ) { m_display = window; }

private:

    //! default handler for ::wxEVT_POST_CREATE_VIEW
    void OnPostCreateView( a2dTemplateEvent& event );

protected:

    Twindow* m_display;
};

BEGIN_EVENT_TABLE_TEMPLATE1( a2dWindowConnector, a2dViewConnector, Twindow )
EVT_POST_CREATE_VIEW( a2dWindowConnector<Twindow>::OnPostCreateView )
END_EVENT_TABLE()

template<class Twindow>
a2dWindowConnector<Twindow>::a2dWindowConnector(): a2dViewConnector()
{
    m_display = NULL;
}

template<class Twindow>
void a2dWindowConnector<Twindow>::OnPostCreateView( a2dTemplateEvent& event )
{
    if ( !m_display )
        return; //view will not be connected to a window ( but document and view already are available).

    //next to what is done in base class, set the view to the display.
    a2dView* view = event.GetView();

    m_display->SetView( view );
    view->Update();
}

//! lua wrap
typedef a2dWindowConnector<a2dDocumentViewScrolledWindow> a2dWindowConnectorFora2dDocumentViewScrolledWindow;

//! extend a2dViewConnector to have a a2dDocumentViewScrolledWindow for each view.
/*

*/
class A2DDOCVIEWDLLEXP a2dScrolledWindowViewConnector: public a2dWindowConnector<a2dDocumentViewScrolledWindow>
{
    DECLARE_DYNAMIC_CLASS( a2dScrolledWindowViewConnector )
};

//! lua wrap
typedef a2dWindowConnector<a2dDocumentViewWindow> a2dWindowConnectorFora2dDocumentViewWindow;

//! extend a2dViewConnector to have a a2dDocumentViewScrolledWindow for each view.
/*

*/
class A2DDOCVIEWDLLEXP a2dWindowViewConnector: public a2dWindowConnector<a2dDocumentViewWindow>
{
    DECLARE_DYNAMIC_CLASS( a2dWindowViewConnector )
};

// ----------------------------------------------------------------------------
// a2dSplitterConnector
// ----------------------------------------------------------------------------

//! extend wxSameFrameViewConnector to connect view into a splitter window
/*!
    The two window of the splitter must be of type a2dDocumentViewScrolledWindow* or
    a2dDocumentViewWindow or one with equal event handling as those.
*/
template<class Twindow>
class A2DDOCVIEWDLLEXP a2dSplitterConnector: public a2dWindowConnector<Twindow>
{

public:

    //! Constructor
    a2dSplitterConnector();

    ~a2dSplitterConnector();

    void Init( wxSplitterWindow* splitter, Twindow* connecto );

    wxSplitterWindow* GetSplitter() { return m_splitter; }

protected:

    wxSplitterWindow* m_splitter;
};

template<class Twindow>
a2dSplitterConnector<Twindow>::a2dSplitterConnector(): a2dWindowConnector<Twindow>()
{
    m_splitter = ( wxSplitterWindow* ) NULL;
}

template<class Twindow>
a2dSplitterConnector<Twindow>::~a2dSplitterConnector()
{
}

template<class Twindow>
void a2dSplitterConnector<Twindow>::Init( wxSplitterWindow* splitter, Twindow* connecto )
{
    m_splitter = splitter;
    wxASSERT_MSG( connecto == m_splitter->GetWindow1() || connecto == m_splitter->GetWindow2(), wxT( "wxDrawingConnector::OnCloseView Not a valid window in splitter" ) );
    this->SetDisplayWindow( connecto );
}

//----------------------------------------------------------------------------
// wxNotebookConnector
//----------------------------------------------------------------------------

//! extend a2dWindowConnector to connect views into a wxNotebook window
/*!
    Each new a2dView connects as a new page
*/
template < class NoteBook, class Twindow, class Tview = a2dView >
class wxNotebookConnector: public a2dWindowConnector<Twindow>
{
    //DECLARE_DYNAMIC_CLASS( wxNotebookConnector )
    //IMPLEMENT_DYNAMIC_CLASS( wxNotebookConnector, a2dScrolledWindowViewConnector )

    DECLARE_EVENT_TABLE()

public:

    //! Constructor
    /*
        \param documentManager maintaining a pool of a2dDocumentTemplate's, a2dViewTemplate's
               a2dDocument's and their a2dView's
    */
    wxNotebookConnector( );

    //! destructor
    ~wxNotebookConnector();

    //! return pointer to notebook type used
    wxNotebook* GetNotebook() { return m_notebook; }

    //! initialize with/to this notebook
    void Init( NoteBook* notebook );

protected:

    //! what to do after document creation
    void OnPostCreateDocument( a2dTemplateEvent& event );

    //! what to do after view creation
    void OnPostCreateView( a2dTemplateEvent& event );

    //! the notebook to hook views into.
    NoteBook* m_notebook;
};

#define wxBEGIN_EVENT_TABLE_TEMPLATE3(theClass, baseClass, T1, T2, T3) \
    template<typename T1, typename T2, typename T3> \
    const wxEventTable theClass<T1, T2, T3>::sm_eventTable = \
            { &baseClass::sm_eventTable, &theClass<T1, T2, T3>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2, typename T3> \
    const wxEventTable *theClass<T1, T2, T3>::GetEventTable() const \
    { return &theClass<T1, T2, T3>::sm_eventTable; } \
    template<typename T1, typename T2, typename T3> \
    wxEventHashTable theClass<T1, T2, T3>::sm_eventHashTable(theClass<T1, T2, T3>::sm_eventTable); \
    template<typename T1, typename T2, typename T3> \
    wxEventHashTable &theClass<T1, T2, T3>::GetEventHashTable() const \
    { return theClass<T1, T2, T3>::sm_eventHashTable; } \
    template<typename T1, typename T2, typename T3> \
    const wxEventTableEntry theClass<T1, T2, T3>::sm_eventTableEntries[] = { \
             
wxBEGIN_EVENT_TABLE_TEMPLATE3( wxNotebookConnector, a2dWindowConnector<Twindow>, NoteBook, Twindow, Tview )
EVT_POST_CREATE_DOCUMENT( wxNotebookConnector::OnPostCreateDocument )
EVT_POST_CREATE_VIEW( wxNotebookConnector::OnPostCreateView )
END_EVENT_TABLE()

template<class NoteBook, class Twindow, class Tview>
wxNotebookConnector<NoteBook, Twindow, Tview>::wxNotebookConnector(): a2dWindowConnector<Twindow>()
{
    m_notebook = ( NoteBook* ) NULL;
}

template<class NoteBook, class Twindow, class Tview>
wxNotebookConnector<NoteBook, Twindow, Tview>::~wxNotebookConnector()
{
}

template<class NoteBook, class Twindow, class Tview>
void wxNotebookConnector<NoteBook, Twindow, Tview>::Init( NoteBook* notebook )
{
    m_notebook = notebook;
}

template<class NoteBook, class Twindow, class Tview>
void wxNotebookConnector<NoteBook, Twindow, Tview>::OnPostCreateDocument( a2dTemplateEvent& event )
{
    event.GetDocument()->SetTitle( event.GetDocument()->GetFilename().GetFullName() );
    event.Skip(); //to baseclass
}

template<class NoteBook, class Twindow, class Tview>
void wxNotebookConnector<NoteBook, Twindow, Tview>::OnPostCreateView( a2dTemplateEvent& event )
{
    //next to what is done in base class, set the view to the display.
    Tview* view = wxDynamicCast( event.GetView(), Tview );

    Twindow* outputwindow;
    outputwindow = new Twindow( view, m_notebook, -1, wxDefaultPosition, wxDefaultSize, wxNO_FULL_REPAINT_ON_RESIZE );
    m_notebook->AddPage( outputwindow, view->GetDocument()->GetPrintableName(), true );

    outputwindow->SetCursor( wxCURSOR_PENCIL );
    outputwindow->SetScrollbars( 20, 20, 50, 50 );
    outputwindow->SetBackgroundColour( wxColour( 0, 235, 235 ) );
    outputwindow->ClearBackground();

    view->Update();
}

// ----------------------------------------------------------------------------
// Provide simple default printing facilities
// ----------------------------------------------------------------------------

#if wxUSE_PRINTING_ARCHITECTURE
//! print a document
/*!
    \ingroup docview
*/
class A2DDOCVIEWDLLEXP a2dDocumentPrintout : public wxPrintout
{
    DECLARE_DYNAMIC_CLASS( a2dDocumentPrintout )

public:
    a2dDocumentPrintout( a2dView* view = ( a2dView* ) NULL, const wxString& title = wxT( "Printout" ) );
    bool OnPrintPage( int page );
    bool HasPage( int page );
    bool OnBeginDocument( int startPage, int endPage );
    void GetPageInfo( int* minPage, int* maxPage, int* selPageFrom, int* selPageTo );

    virtual a2dView* GetView() { return m_printoutView; }

protected:
    a2dView*       m_printoutView;
};
#endif // wxUSE_PRINTING_ARCHITECTURE


// ----------------------------------------------------------------------------
// Special Frame for views in a frame.
// ----------------------------------------------------------------------------


//!The a2dDocumentFrame class provides a default frame for displaying documents
/*!
    Can be parent or child frame.

    In case of a ::wxEVT_CLOSE_WINDOW event, it is first redirected to the frame, and
    if not vetod askes the view to close.
    In a multi frame application the View should Destroy its windows/frame, including this one.
    The View may Veto the Close.

    The view which belongs to this frame is either set directly when constructing, or
    it is set automatically by the view when a2dView::Activate() is sending the
    ::wxEVT_ACTIVATE_VIEW_SENT_FROM_CHILD event.
    Imagine that this frame would have 4 windows, each having a view on a 3D object. When clicking on one
    of those view, that views, its window will be activated, resulting in its view being activated, and that
    sends the wanted event. The frame will always know which view is active, and e.g. closing one view/window is
    via this chain.

    If this frame is the parent all other child frames will close also,
    else only the child frame.

    \ingroup docview
*/
class A2DDOCVIEWDLLEXP a2dDocumentFrame : public wxFrame
{
    DECLARE_CLASS( a2dDocumentFrame )

public:

    //! Constructor for two step creation, call a2dDocumentFrame::Create later.
    a2dDocumentFrame();

    //! constructor
    /*!
        \param parentFrame set true if this is to be the parent frame in the application.
        \param frame    the parent frame for this frame. If topwindow use  <code>NULL</code>.
        \param view     The view for the document ( can be <code>NULL</code> )
        \param id       The window identifier.
                        It may take a value of -1 to indicate a default value.
        \param title    The caption to be displayed on the frame's title bar.
        \param pos      The window position.
        \param size     The window size.
        \param style    Window style, please look into wxWindows docs for an overview.
        \param name     The name of the window.
    */
    a2dDocumentFrame(
        bool parentFrame,
        wxWindow* frame,
        a2dView* view = NULL,
        wxWindowID id = -1,
        const wxString& title = wxT( "a2dDocumentFrame" ),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long  style = wxDEFAULT_FRAME_STYLE,
        const wxString& name = wxT( "frame" ) );

    //! wxFrame compatible constructor
    // YOU must set m_view and m_isParentFrame correctly.
    a2dDocumentFrame(
        wxWindow* frame,
        wxWindowID id = -1,
        const wxString& title = wxT( "a2dDocumentFrame" ),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long  style = wxDEFAULT_FRAME_STYLE,
        const wxString& name = wxT( "frame" ) );


    //! Constructor for two-step-creation (2nd step)
    /*!
        \param parentFrame set true if this is to be the parent frame in the application.
        \param frame    the parent frame for this frame. If topwindow use  <code>NULL</code>.
        \param view     The view for the document ( can be <code>NULL</code> )
        \param id       The window identifier.
                         It may take a value of -1 to indicate a default value.
        \param title    The caption to be displayed on the frame's title bar.
        \param pos      The window position.
        \param size     The window size.
        \param style    Window style, please look into wxWindows docs for an overview.
        \param name     The name of the window.
    */
    bool Create(
        bool parentFrame,
        wxWindow* frame,
        a2dView* view = NULL,
        wxWindowID id = -1,
        const wxString& title = wxT( "a2dDocumentFrame" ),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long  style = wxDEFAULT_FRAME_STYLE,
        const wxString& name = wxT( "frame" ) );

    //! destructor
    ~a2dDocumentFrame();

    void SetDestroyOnCloseView( bool destroyOnCloseView ) { m_destroyOnCloseView = destroyOnCloseView; }

    //! set a pointer to a a2dView to sent view events to.
    /*!
        Normally set via OnActivateViewSentFromChild()

        The m_view is used to redirect the frame event for closing the window,
        and the activate event for the frame, to the view.
        Only if the m_view->GetDisplayWindow() is the a2dDocumentFrame itself, it will also be used for
        drawing the view ( See OnDraw() )
    */
    void SetView( a2dView* view );

    //! get the pointer to a a2dView to which view events are sent
    a2dView* GetView() { return m_view; }

    //! Modified event handler to also sent event to a2dView or wxDocviewCommonEvents
    /*!
        Events handled how:
        - ::wxEVT_CLOSE_WINDOW if this frame is a parent frame, and the event is not vetod by the frame
        itself, calls a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear(), this will
        eventually close all document and close all child views. And a child view will
        sent a ::wxEVT_CLOSE_VIEW to its containing window, and this will end up in the child frame
        (if there). So closing the parent frame, will lead to closing the child frames.
        If this frame is a childframe, the event is only sent to the view, if not vetod by this frame.
        This will lead to calling a2dView::Close(), and this gives you the chance to clean up
        by intercepting the ::wxEVT_CLOSE_VIEW event which is generated in there.

        - ::wxEVT_ACTIVATE sent to view, and next to the frame itself.
        - all others first to frame itself, if not handled to view, and if not handled to
        a2dDocviewGlobals->GetDocviewCommonEvents()

    */
    bool ProcessEvent( wxEvent& event );

    bool ProcessConnectedEvent( wxEvent& event );
    void ConnectEvent( wxEventType type, wxEvtHandler* evtObject );
    bool DisconnectEvent( wxEventType type, wxEvtHandler* evtObject );

    bool GetIsParentFrame() { return m_isParentFrame; }

    void SetIsParentFrame( bool parentFrame ) { m_isParentFrame = parentFrame; }

    //! Override this function to draw the graphic (or just process EVT_PAINT)
    /*!
        The default handler for EVT_PAINT is OnPaint, and it calls OnDraw().
        OnDraw its default implementation is calling the m_view its OnDraw().

        If m_view its DisplayWindow, is the frame itself, the m_view->OnDraw() is called.
        If not the m_view, is only used for redirecting Close and activate events to the
        view which has the focus.
    */
    virtual void OnDraw( wxDC& dc );

    //! add items to toolbar using a predefined command ID.
    void AddCmdToToolbar( const a2dMenuIdItem& cmdId );
    void ConnectCmdId( const a2dMenuIdItem& cmdId );

    //! add a menu to the parent menu, and connect it to the eventhandler of the frame
    /*!
        Many commands which can be submitted to a commandprocessor in the following manner:
        \code
        commandprocessor->Submit(  new a2dCommand_SomeCommand() )
        \endcode

        If a command does not make sense, but a function on the command handler must be called,
        it can be handled as described below too.

        Submitting a command makes it possible to undo the action.
        To issue such a command from a menu requires a menu Id, which is handled by wxWidgets
        its event system. e.g. a2dDocumentFrame and if not processed, try a2dDocumentCommandProcessor.
        The last in many cases submits a proper command based on the menu id.

        Therefore wxDocview defines many predefined
        menus items in the form of a2dMenuIdItem. They produce a unique Id to be handled in
        the commandprocessor. Like in:
        \code
        void a2dDocumentCommandProcessor::OnMenu( wxCommandEvent& event )
        \endcode
        Several modules in wxDocview extend the set of handled menu Id's in a derived a2dDocumentCommandProcessor.
        The OnMenu intercepts the menu event, and sets up a new a2dCommand to be submitted.
        Understand that a2dDocumentFrame and a2dDocumentMDIParentFrame already redirect non handled events to
        the commandprocessor of the application.
        An a2dMenuIdItem defined menu text and helpstring, therefore all you need to do to add a command to
        your application is AddCmdMenu().
        Many commands have parameters, and a a2dMenuIdItem its Id, can only be coupled to one a2dCommand with
        some predefined set of parameters.
        If you need a different implementation of the same command you can extend in the same manner
        in your derived commandprocessor. But you can always use the standard way of wxWidgets to add a menu,
        and add a handler in the frame. In the handler, you setup the a2dCommand and submit it.
        Exactly the same behaviour, only not predefined.

        \remark a2dMenuIdItem uses Id my name ready to be used by XRC files.
    */
    void AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId );

    //! same as for a2dMenuIdItem, Id in wxMenuItem must be a valid id from one a2dMenuIdItem
    //! used in XRC files
    void AddCmdMenu( wxMenu* parentMenu, wxMenuItem* item );

    //! remove a command menu from the parent menu.
    /*!
        See AddCmdMenu()
    */
    void RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId );

    //! called for dynamic added menu's via AddCommandToMenu()
    //! command will be submitted to a2dDocviewGlobals->GetDocviewCommandProcessor()
    void OnExecuteCommand( wxCommandEvent& event );

    //! used by AddCmdMenu() to dynamically connect menu to function.
    //! The function just Skipps the event, so it will be handled by a command processor in the end.
    void OnCmdMenuId( wxCommandEvent& event );

protected:


    //! default handler for paint events, it calls OnDraw().
    void OnPaint( wxPaintEvent& event );

    //! default for closing a view in this frame
    /*!
        Close the view, if set, and destroy the frame.
        The view is closed even if it is displayed in a nested subwindow.
        Since it Destroys the frame ( if  m_destroyOnCloseView  is set ),
        that will automatically destroy the nested windows too.

        /remark m_view is set to the active view if there are more sub windows with
        views. So just closing the currently active view is not enough, if there are more.
    */
    void OnCloseView( a2dCloseViewEvent& event );


    //! default for closing this frame via close button.
    /*!
        default handler for wxEVT_CLOSE event.
        Give the Frame the right to veto.
        The default tries to close the view.
        That is the right thing to do if the frame has just a single view.

        If there are more views ( even in nested windows ), you need to
        handle that yourself.

        /remark m_view is set to the active view if there are more sub windows with
        views. So just closing the currently active view is not enough, if there are more.
    */
    void OnCloseWindow( wxCloseEvent& event );

    //! called by API and only handled if this is the Parent Frame
    void OnExit( wxCommandEvent& event );

    //! called by API and only handled if this is the Parent Frame
    void OnMRUFile( wxCommandEvent& event );

    //! default handler for file changed event  sent from a2dView
    /*!
        Sends the event to the DisplayWindow, there or in its parent hiearchy it can be intercepted.
    */
    void OnChangeFilename( a2dViewEvent& event );

    //! sets m_view to the current active view, generate by a2dView::Activate()
    void OnActivateViewSentFromChild( a2dViewEvent& viewevent );

    //! used for redirecting close and activate events to the view which has the focus.
    //! See SetView()
    a2dSmrtPtr<a2dView> m_view;

    //! is this the docframe or not.
    bool m_isParentFrame;

    //! when closing the view the frame will be destroyed.
    bool m_destroyOnCloseView;

    DECLARE_EVENT_TABLE()
};



//! specialized wxWindow derived template class to handle a2dView events
/*!
    The following events are sent up from the a2dView  to this window.

    ::wxEVT_CLOSE_VIEW

    Next one can be recieved from the a2dDocument after Registering this class.

    ::wxEVT_CHANGEDFILENAME_DOCUMENT

    When the a2dView sent this events it disables itself until return, to prevent looping.

    ::wxEVT_CLOSE_VIEW should be handled in a derived class, or in some/the parent window of this window.
    The handler must call a2dDocumentViewScrolledWindow::SetView( NULL )
    or set m_view to NULL and release it, this Detaches the view from the window,
    In case the window gets deleted via a Destroy() this will be done automatically.

    These event are of type commandevents, and travel up to the parent of this window if not handled here.

    All other events to the window are sent first to the a2dView and if not handled there,
    can be handled here.
*/
template<class Twindow>
class A2DDOCVIEWDLLEXP a2dViewWindow: public Twindow
{

public:

    //! Constructor for creation with view.
    /*!
        \param view     The view for the document ( can be <code>NULL</code> )
        \param parent   the parent window for this window
        \param id       The window identifier.
                        It may take a value of -1 to indicate a default value.
        \param pos      The window position.
        \param size     The window size.
        \param style    Window style, please look into wxWindows docs for an overview.
        \param name     The name of the window.
    */
    a2dViewWindow( a2dView* view,
                   wxWindow* parent,
                   wxWindowID id = -1,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = wxHSCROLL | wxVSCROLL,
                   const wxString& name = wxT( "a2dViewWindow" ) );

    //! Constructor for creation without view.
    /*!
        \param parent   the parent window for this window
        \param id       The window identifier.
                        It may take a value of -1 to indicate a default value.
        \param pos      The window position.
        \param size     The window size.
        \param style    Window style, please look into wxWindows docs for an overview.
        \param name     The name of the window.
    */
    a2dViewWindow( wxWindow* parent,
                   wxWindowID id = -1,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = wxHSCROLL | wxVSCROLL,
                   const wxString& name = wxT( "a2dViewWindow" ) );

    //! Destructor.
    ~a2dViewWindow();

    //! to switch the view in this window to another view
    /*!
        Default releases the old view and sets it display to NULL.
        Next it sets and owns the new view and sets the view its display to this window.
        Override this if more needs to be done to switch the view.
    */
    virtual void SetView( a2dView* view );

    //! get the current view
    a2dView* GetView() { return m_view; }

    //! Extend event processing to search the view's event table
    /*! Some events are handled within the view as if the view is a window within a window.
        Therefore the wxWindow containing the view should redirect all its events to
        its views, when the view does not handle the event it will go to the window itself.

        View events ::wxEVT_CLOSE_VIEW from the view are
        sent up to this window.

        Next one can be recieved from the a2dDocument after Registering this class.

        ::wxEVT_CHANGEDFILENAME_DOCUMENT

    */
    virtual bool ProcessEvent( wxEvent& event );

    //! Override this function to draw the graphic (or just process EVT_PAINT)
    virtual void OnDraw( wxDC& dc );

    //! resets view to NULL when view was closed and skips the event for the parent window/frame.
    void OnCloseView( a2dCloseViewEvent& event );

    //! calls OnDraw(dc);
    void OnPaint( wxPaintEvent& event );

protected:

    //! the view that uses this window to display itself
    a2dSmrtPtr<a2dView> m_view;

//private:

};

// Define a constructor for my canvas
template<class Twindow>
a2dViewWindow<Twindow>::a2dViewWindow( a2dView* view, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, const long style, const wxString& name ):
    Twindow( parent, id, pos, size, style, name )
{
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_view.m_owner = this;
    m_view.m_ownertype = a2dSmrtPtr<a2dView>::owner_wxobject;
#endif
    m_view = view;
    if ( m_view )
    {
        //Attach the canvas as the window for the view to display its stuff
        m_view->SetDisplayWindow( this );
    }
    Twindow::Enable();
}

// Define a constructor for my canvas
template<class Twindow>
a2dViewWindow<Twindow>::a2dViewWindow( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, const long style, const wxString& name ):
    Twindow( parent, id, pos, size, style, name )
{
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_view.m_owner = this;
    m_view.m_ownertype = a2dSmrtPtr<a2dView>::owner_wxobject;
#endif
    m_view = NULL;
}

template<class Twindow>
a2dViewWindow<Twindow>::~a2dViewWindow()
{
    if ( m_view )
    {
        m_view->SetDisplayWindow( NULL );
    }
    m_view = NULL;
}

template<class Twindow>
void a2dViewWindow<Twindow>::SetView( a2dView* view )
{
    if ( m_view )
    {
        m_view->SetDisplayWindow( NULL );
    }

    m_view = view;

    if ( m_view )
    {
        //Attach the canvas as the window for the view to display its stuff
        m_view->SetDisplayWindow( this );
        m_view->Activate( true );
    }
}

template<class Twindow>
void a2dViewWindow<Twindow>::OnPaint( wxPaintEvent& WXUNUSED( event ) )
{
    wxPaintDC dc( this );
    Twindow::PrepareDC( dc );

    OnDraw( dc );
}

// Define the repainting behaviour
template<class Twindow>
void a2dViewWindow<Twindow>::OnDraw( wxDC& dc )
{
    if ( m_view )
        m_view->OnDraw( & dc );
}

template<class Twindow>
void a2dViewWindow<Twindow>::OnCloseView( a2dCloseViewEvent& event )
{
    wxASSERT_MSG( m_view == event.GetEventObject(), wxT( "view in a2dCloseViewEvent not equal to m_view" ) );

    if ( m_view )
    {
        //we do not set the m_view->SetDisplayWindow( NULL );
        //This enables parent window to still find the display window via the a2dCloseViewEvent.
        //The view itself will set the m_display to NULL.
        m_view = NULL;
    }

    Twindow::Refresh();
    event.Skip(); //to parent window/frame
}

template<class Twindow>
bool a2dViewWindow<Twindow>::ProcessEvent( wxEvent& event )
{
    //some wxViewEvents  go upstream ( document -> view -> window -> frame )
    //These event are of the commandevent type, so they do get processed by the parent window
    //of this window ( when not processed here ).a

    // Extend event processing to search the view's event table
    if ( event.GetEventType() == wxEVT_ACTIVATE_VIEW_SENT_FROM_CHILD )
    {
        return wxEvtHandler::ProcessEvent( event );
    }
    else
    {
        // most events will be processed first by the view
        if (  m_view && !m_view->IsClosed() &&
                m_view->ProcessEvent( event ) )
            return true;

        return wxEvtHandler::ProcessEvent( event );
    }
    return false;
}

//! specialized wxScrolledWindow to handle a2dView events
/*!
    The following events are sent up from the a2dView  to this window.

    ::wxEVT_CLOSE_VIEW

    Next one can be recieved from the a2dDocument after Registering this class.

    ::wxEVT_CHANGEDFILENAME_DOCUMENT

    When the a2dView sent this events it disables itself until return, to prevent looping.

    ::wxEVT_CLOSE_VIEW should be handled in a derived class, or in some/the parent window of this window.
    The handler must call a2dDocumentViewScrolledWindow::SetView( NULL )
    or set m_view to NULL and release it, this Detaches the view from the window,
    In case the window gets deleted via a Destroy() this will be done automatically.

    These event are of type commandevents, and travel up to the parent of this window if not handled here.

    All other events to the window are sent first to the a2dView and if not handled there,
    can be handled here.
*/
class A2DDOCVIEWDLLEXP a2dDocumentViewScrolledWindow: public a2dViewWindow<wxScrolledWindow>
{
    DECLARE_EVENT_TABLE()

public:
    //! Constructor for creation with view.
    /*!
        \param view     The view for the document ( can be <code>NULL</code> )
        \param parent   the parent window for this window
        \param id       The window identifier.
                        It may take a value of -1 to indicate a default value.
        \param pos      The window position.
        \param size     The window size.
        \param style    Window style, please look into wxWindows docs for an overview.
        \param name     The name of the window.
    */
    a2dDocumentViewScrolledWindow( a2dView* view,
                                   wxWindow* parent,
                                   wxWindowID id = -1,
                                   const wxPoint& pos = wxDefaultPosition,
                                   const wxSize& size = wxDefaultSize,
                                   long style = wxHSCROLL | wxVSCROLL,
                                   const wxString& name = wxT( "a2dDocumentViewScrolledWindow" ) );

    //! Constructor for creation without view.
    /*!
        \param parent   the parent window for this window
        \param id       The window identifier.
                        It may take a value of -1 to indicate a default value.
        \param pos      The window position.
        \param size     The window size.
        \param style    Window style, please look into wxWindows docs for an overview.
        \param name     The name of the window.
    */
    a2dDocumentViewScrolledWindow( wxWindow* parent,
                                   wxWindowID id = -1,
                                   const wxPoint& pos = wxDefaultPosition,
                                   const wxSize& size = wxDefaultSize,
                                   long style = wxHSCROLL | wxVSCROLL,
                                   const wxString& name = wxT( "a2dDocumentViewScrolledWindow" ) );

    //! Destructor.
    ~a2dDocumentViewScrolledWindow();

private:

    DECLARE_CLASS( a2dDocumentViewScrolledWindow )
};


//! specialized wxWindow to handle a2dView events
/*!
    The following events are sent up from the a2dView to this window.

    ::wxEVT_CLOSE_VIEW

    Next one can be recieved from the a2dDocument after Registering this class.
    ::wxEVT_CHANGEDFILENAME_DOCUMENT

    When the a2dView sent this events it disables itself until return, to prevent looping.

    ::wxEVT_CLOSE_VIEW should be handled in a derived class, or in some/the parent window of this window.
    The handler must call a2dDocumentViewScrolledWindow::SetView( NULL )
    or set m_view to NULL and release it, this Detaches the view from the window,
    In case the window gets deleted via a Destroy() this will be done automatically.

    These event are of type commandevents, and travel up to the parent of this window if not handled here.

    All other events to the window are sent first to the a2dView and if not handled there,
    can be handled here.
*/
class a2dDocumentViewWindow: public a2dViewWindow<wxWindow>
{
    DECLARE_EVENT_TABLE()

public:
    //! Constructor for creation with view.
    /*!
        \param view     The view for the document ( can be <code>NULL</code> )
        \param parent   the parent window for this window
        \param id       The window identifier.
                        It may take a value of -1 to indicate a default value.
        \param pos      The window position.
        \param size     The window size.
        \param style    Window style, please look into wxWindows docs for an overview.
        \param name     The name of the window.
    */
    a2dDocumentViewWindow( a2dView* view,
                           wxWindow* parent,
                           wxWindowID id = -1,
                           const wxPoint& pos = wxDefaultPosition,
                           const wxSize& size = wxDefaultSize,
                           long style = wxHSCROLL | wxVSCROLL,
                           const wxString& name = wxT( "a2dDocumentViewWindow" ) );


    //! Constructor for creation without view.
    /*!
        \param parent   the parent window for this window
        \param id       The window identifier.
                        It may take a value of -1 to indicate a default value.
        \param pos      The window position.
        \param size     The window size.
        \param style    Window style, please look into wxWindows docs for an overview.
        \param name     The name of the window.
    */
    a2dDocumentViewWindow( wxWindow* parent,
                           wxWindowID id = -1,
                           const wxPoint& pos = wxDefaultPosition,
                           const wxSize& size = wxDefaultSize,
                           long style = wxHSCROLL | wxVSCROLL,
                           const wxString& name = wxT( "a2dDocumentViewWindow" ) );

    //! Destructor.
    ~a2dDocumentViewWindow();

    DECLARE_CLASS( a2dDocumentViewWindow )
};




#endif /* _WX_DOCREFH__ */


