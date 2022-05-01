/*! \file wx/general/gen.h
    \brief fundamental classes used by all other modules.

    In here the reference counted a2dObject and a derived object which can have properties
    attached to it. Save and Load for serialisation is part of this object.
    The basic type properties classes are here.
    The a2dComEvent for communicating events in between classes using an event.
    At last base classes for the serialisation of documents to files or just algorithms performed on documents.
    For all in and output of a document a2dIOHandler is the base.

    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: gen.h,v 1.81 2009/07/15 21:23:43 titato Exp $
*/

#ifndef __WXDOCPROP_H__
#define __WXDOCPROP_H__

// shared builds stuff
#include "wx/general/generaldef.h"

//#ifndef WX_PRECOMP
#include "wx/filename.h"
#include "wx/colour.h"

// it is defined in a2dopt.h for precompiled header
#include <wx/mstream.h>

#include "wx/general/smrtptr.h"
#include "wx/general/a2dmemmgr.h"
#include "wx/general/id.h"
//#endif


#if defined(__VISUALC__) && (_MSC_VER <= 1300)
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_NO_DEPRECATE 1

#undef wxSTD
#define wxSTD std::
#endif // VC++ >= 7

#if wxUSE_STD_IOSTREAM
#include <wx/ioswrap.h>
#include <fstream>
#include <sstream>
#include <wx/txtstrm.h>

/*
typedef enum
{
  wxEOL_NATIVE,
  wxEOL_UNIX,
  wxEOL_MAC,
  wxEOL_DOS
} wxEOL;
*/

#else
#include "wx/wfstream.h"
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#endif

//! Define wxDynamicCast so that it will give a compiler error for unrelated types
#undef wxDynamicCast
#if wxCHECK_VERSION(2, 5, 0)
#define wxDynamicCast(obj, className) \
    ((className *) wxCheckDynamicCast(const_cast<className*>(static_cast<const className*>(obj)), &className::ms_classInfo))
#else
#define wxDynamicCast(obj, className) \
    ((className *) wxCheckDynamicCast(const_cast<className*>(static_cast<const className*>(obj)), &className::sm_class##className))
#endif

//! The wxWindows 2.4.2 wxStaticCast is buggy. It evaluates its argument twice

#undef wxStaticCast

#ifdef __WXDEBUG__

inline void* wxCheckCastFixed( void* ptr )
{
    wxASSERT_MSG( ptr, _T( "wxStaticCast() used incorrectly" ) );
    return ptr;
}
#define wxStaticCast(obj, className) \
    ((className *)wxCheckCastFixed(wxDynamicCast(obj, className)))

#define wxStaticCastConst(obj, className) \
    ((const className *)wxCheckCastFixed(wxDynamicCast(obj, className)))

//! wxStaticCast asserts, if the input is null, this version not.
inline wxObject* wxCheckCastNull( wxObject* obj, wxClassInfo* clssInfo )
{
    if( !obj ) return 0;
    wxASSERT_MSG( wxCheckDynamicCast( obj, clssInfo ), _T( "wxStaticCast() used incorrectly" ) );
    return obj;
}

#if wxCHECK_VERSION(2, 5, 0)
#define wxStaticCastNull(obj, className) \
    ((className *)wxCheckCastNull(const_cast<wxObject*>(static_cast<const wxObject*>(obj)), &className::ms_classInfo))
#else
#define wxStaticCastNull(obj, className) \
    ((className *)wxCheckCastNull(const_cast<wxObject*>(static_cast<const wxObject*>(obj)), &className::sm_class##className))
#endif

// In 2.4.2, wxStaticCast cannot be used in templates
#if wxCHECK_VERSION(2, 5, 0)
#define wxStaticCastTmpl(obj, className) wxStaticCast(obj, className)
#define wxStaticCastNullTmpl(obj, className) wxStaticCastNull(obj, className)
#else
#define wxStaticCastTmpl(obj, className) ((className *)(wxObject*)(obj))
#define wxStaticCastNullTmpl(obj, className) ((className *)(wxObject*)(obj))
#endif

#else  // !__WXDEBUG__

#define wxStaticCast(obj, className) ((className *)(wxObject*)(obj))
#define wxStaticCastConst(obj, className) ((const className *)(wxObject*)(obj))
#define wxStaticCastNull(obj, className) ((className *)(wxObject*)(obj))
#define wxStaticCastTmpl(obj, className) ((className *)(wxObject*)(obj))
#define wxStaticCastNullTmpl(obj, className) ((className *)(wxObject*)(obj))

#endif  // __WXDEBUG__

class a2dVersNo
{
public:
    a2dVersNo(int major = 0, int minor = 0, int micro = 0)
        : m_major(major), m_minor(minor), m_micro(micro) {}
    a2dVersNo(const wxString& versionString);

    bool operator==( const a2dVersNo& rhs ) const;
    bool operator!=( const a2dVersNo& rhs ) const { return !(*this == rhs); }
    bool operator>( const a2dVersNo& rhs ) const;
    bool operator>=( const a2dVersNo& rhs ) const;
    bool operator<( const a2dVersNo& rhs ) const { return !(*this >= rhs); }
    bool operator<=( const a2dVersNo& rhs ) const { return !(*this > rhs); }

    int GetMajor() const { return m_major; }
    int GetMinor() const { return m_minor; }
    int GetMicro() const { return m_micro; }

private:
    int m_major,
        m_minor,
        m_micro;
};

class a2dVersionInfo
{
public:

    a2dVersionInfo(const wxString& name,
                  int major,
                  int minor,
                  int micro,
                  const wxString& description = wxEmptyString,
                  const wxString& copyright = wxEmptyString)
    :
        m_name( name ),
        m_vers( major, minor, micro ),
        m_description( description ),
        m_copyright( copyright )
    {}

    a2dVersionInfo(const wxString& versionString, 
                  const wxString& name = wxEmptyString,
                  const wxString& description = wxEmptyString,
                  const wxString& copyright = wxEmptyString)
    :
        m_name( name ),
        m_vers( versionString ),
        m_description( description ),
        m_copyright( copyright )
    {}

    // Default copy ctor, assignment operator and dtor are ok.


    const wxString& GetName() const { return m_name; }

    const a2dVersNo& GetVersion() const { return m_vers; }
    int GetVersionMajor() const { return m_vers.GetMajor(); }
    int GetVersionMinor() const { return m_vers.GetMinor(); }
    int GetVersionMicro() const { return m_vers.GetMicro(); }

    wxString ToString() const
    {
        return HasDescription() ? GetDescription() : GetVersionString();
    }

    wxString GetVersionString() const
    {
        wxString str;
        str << m_name << ' ' << m_vers.GetMajor() << '.' << m_vers.GetMinor();
        if ( m_vers.GetMicro() )
            str << '.' << m_vers.GetMicro();

        return str;
    }

    bool HasDescription() const { return !m_description.empty(); }
    const wxString& GetDescription() const { return m_description; }

    bool HasCopyright() const { return !m_copyright.empty(); }
    const wxString& GetCopyright() const { return m_copyright; }

private:
    wxString m_name,
             m_description,
             m_copyright;

    a2dVersNo m_vers;
};


//! This function is like wxNewId, but it has its own ID set
/*! wxNewId is used for menu command ids and frequently the same number space is used
    for menu ids and control/window ids. As window/control ids are limited to 15 bit
    in MSW, they should not be used for other stuff (like canvas object IDs)
*/
A2DGENERALDLLEXP long wxGenNewId();

//! return a unique id (based on time)
extern wxLongLong GetUniqueSerializationId();


//! see wxGenNewId(), use this to resrve a specific id
/*!
    An id of a specific value is allocated.
    new wxGenNewId() generated id's after this will be incremented from this id.
*/
A2DGENERALDLLEXP void wxGenRegisterId( long id );

class A2DGENERALDLLEXP a2dObjectList;
class A2DGENERALDLLEXP a2dResolveIdList;
class A2DGENERALDLLEXP a2dIOHandler;
class A2DGENERALDLLEXP a2dRefMap;

#if wxART2D_USE_CVGIO

class a2dIOHandlerXmlSerIn;
class a2dIOHandlerXmlSerOut;

/* XML a2dObject serialiser flags*/
enum a2dXmlSer_flag
{
    a2dXmlSer_attrib,
    a2dXmlSer_Content
};

#endif //wxART2D_USE_CVGIO

class A2DGENERALDLLEXP a2dWalkerIOHandler;

//! error codes generated in docview framework.
typedef int a2dErrorWarningCode;


#include "wx/xrc/xmlres.h"

#include <map>
class A2DGENERALDLLEXP a2dError;
typedef std::map< wxString, a2dError* > a2dErrorHash;
#include <vector>
typedef std::vector< a2dError > a2dErrorVector;

// ============================================================================
// event handler and related classes
// ============================================================================

typedef int a2dSignal;

class A2DGENERALDLLEXP a2dEvent : public wxEvent
{

public:

    a2dEvent( int id = 0, wxEventType eventType = wxEVT_NULL )
        : wxEvent( id, eventType )
    {
        SetIntTimeStamp();
    }

    void SetIntTimeStamp()
    {
        if ( ms_timestamp == LONG_MAX )
            ms_timestamp = 0;
        SetTimestamp( ++ms_timestamp );
    }

    static long ms_timestamp;
};


//----------------------------------------------------------------------------
// a2dComEvent
//----------------------------------------------------------------------------

/*
#define DECLARE_EXPORTED_EVENT_TYPE(expdecl, name, value) \
    extern expdecl const wxEventType name;

#define DEFINE_EVENT_TYPE(name) const wxEventType name = wxNewEventType();
*/

BEGIN_DECLARE_EVENT_TYPES()
//! see a2dComEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DGENERALDLLEXP, a2dEVT_COM_EVENT, 1 )
END_DECLARE_EVENT_TYPES()

//! special event for communicating between a2dObject's and GUI objects and Command processors.
/*!
    Used for communicating and distributing events which change e.g. a2dObject's

    The information  that is sent around is stored in a a2dNamedProperty.

    To intercept this event ( when sent to an object ), use the next event table entries:

    - EVT_COM_EVENT in case of base wxEvtHandler

    event id's for Change events used for specifying which type of information is communicated
    via a a2dComEvent event.

    The idea is to add static members like this to a class you wnat to sent an event from.
    Like in a2dCanvasObject we have in the header file:

    \code
    const static a2dComEvent::wxEventType sm_changedLayer;
    \endcode

    and in the cpp file do initilize the static member:

    \code
    const a2dComEvent::wxEventType a2dCanvasDocument::sm_changedLayer;
    \endcode

    and in the cpp file do sent an event from member function:

    \code
    a2dComEvent changedlayer( this, PROPID_layer, layer, &sm_changedLayer );
    ProcessEvent( changedlayer );
    \endcode


    \code
    BEGIN_EVENT_TABLE( anyclass, wxWindow )
          EVT_COM_EVENT( anyclass::OnComEvent )
    END_EVENT_TABLE()

    void anyclass::OnComEvent( a2dComEvent& event )
    {
        if ( event.GetId() == &sm_changedLayer )
        {
            DoIt();
        }
    }
    \endcode

    \see a2dGeneralGlobals

    \see a2dComEvent

    \ingroup canvasobject eventmanager property

    \ingroup eventmanager
    \ingroup events
*/
class A2DGENERALDLLEXP a2dComEvent: public wxEvent
{
    DECLARE_DYNAMIC_CLASS( a2dComEvent )

public:

    //! sent from a2dObject::OnPropertyChanged().
    //! id for changed properties
    //! a property on a wxProprefObject has accured.
    //! \ingroup events
    static const a2dSignal sm_changedProperty;
    //! id for changed a2dObject
    //! \ingroup events
    static const a2dSignal sm_changedRefObject;
    //! non info.
    //! \ingroup events
    static const a2dSignal sm_non;

    //! for sending just an event id, m_property is not set
    a2dComEvent( wxObject* sender, a2dSignal id );

    //! sent a property id ( in order to get property on sender object )
    /*!
        \param sender the object which did sent this event.
        \param id id of the property in the sender object
        \param eventid eventid is to further specify the nature of the event.
    */
    a2dComEvent( a2dObject* sender, const a2dPropertyId* id, a2dSignal = sm_changedProperty  );

    //! sent a property
    /*!
        \param sender the object which did sent this event.
        \param property the property to sent
        \param id eventid is to further specify the nature of the event.
        \param ownProp when true the property will be cloned and deleted here
    */
    a2dComEvent( wxObject* sender, a2dNamedProperty* property, a2dSignal id = sm_changedProperty, bool ownProp = false );

    //! sent a property by reference
    /*!
        \param sender the object which did sent this event.
        \param property the property to sent
        \param eventid eventid is to further specify the nature of the event.
    */
    a2dComEvent( wxObject* sender, const a2dNamedProperty& property, a2dSignal eventid = sm_changedProperty );

    //! sent a property generated from a a2dObject.
    /*!
        Creates a wxRefObjectPtrProperty internal.

        \param sender the object which did sent this event.
        \param refObject the a2dObject
        \param eventid eventid is to further specify the nature of the event.
    */
    a2dComEvent( wxObject* sender, a2dObject* refObject, a2dSignal id = sm_changedRefObject );

    //! sent a property id ( in order to get property on sender object )
    /*!
        \param sender the object which did sent this event.
        \param id id of the property in the sender object
        \param eventid eventid is to further specify the nature of the event.
    */
    // a2dComEvent( wxObject* sender, const a2dPropertyId& id, a2dSignalid = sm_changedProperty );

    //! sent a property generated from a a2dObject.
    a2dComEvent( wxObject* sender, const a2dPropertyIdRefObject* propId, a2dObject* refObject, a2dSignal id = sm_changedProperty );

    //! sent a property generated from a bool.
    a2dComEvent( wxObject* sender, const a2dPropertyIdBool* propId, bool propertyValue, a2dSignal id = sm_changedProperty );

    //! sent a property generated from a wxUint16.
    a2dComEvent( wxObject* sender, const a2dPropertyIdUint16* propId, wxUint16 propertyValue, a2dSignal id = sm_changedProperty );

    //! sent a property generated from a wxInt32.
    a2dComEvent( wxObject* sender, const a2dPropertyIdInt32* propId, wxInt32 propertyValue, a2dSignal id = sm_changedProperty );

    //! sent a property generated from a double.
    a2dComEvent( wxObject* sender, const a2dPropertyIdDouble* propId, double propertyValue, a2dSignal id = sm_changedProperty );

    //! sent a property generated from a wxString.
    a2dComEvent( wxObject* sender, const a2dPropertyIdString* propId, const wxString&  propertyValue, a2dSignal id = sm_changedProperty );

    //! sent a property generated from a bool.
    a2dComEvent( wxObject* sender, bool propertyValue, a2dSignal id );

    //! sent a property generated from a wxIint16.
    a2dComEvent( wxObject* sender, wxInt16 propertyValue, a2dSignal id );

    //! sent a property generated from a wxUint16.
    a2dComEvent( wxObject* sender, wxUint16 propertyValue, a2dSignal id );

    //! sent a property generated from a wxInt32.
    a2dComEvent( wxObject* sender, wxInt32 propertyValue, a2dSignal id );

    //! sent a property generated from a wxUint32.
    a2dComEvent( wxObject* sender, wxUint32 propertyValue, a2dSignal id );

    //! sent a property generated from a double.
    a2dComEvent( wxObject* sender, double propertyValue, a2dSignal id );

    //! sent a property generated from a wxString.
    a2dComEvent( wxObject* sender, const wxString&  propertyValue, a2dSignal id );

    a2dComEvent();

    ~a2dComEvent();

    a2dComEvent( const a2dComEvent& other );

    wxEvent* Clone( void ) const;

    a2dNamedProperty* GetProperty() { return m_property; }

    const a2dPropertyId* GetPropertyId() const;

    //! after proecssin the event, on return an id can be set to communicate back to sender.
    a2dSignal GetEventComIdReturn() { return m_comReturn; }

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //! this is a list of all smart pointers pointing to this object
    a2dSmrtPtr<a2dComEvent> *m_ownerlist;
    //! this is needed inside the smart pointer template code
    typedef a2dSmrtPtr<a2dComEvent> TOwnerListClass;
    //! Make a Dump of the objects owner list to the Debug console
    /*! This function is usually called from the Debuggers watch window */
    void DumpOwners();
#endif

private:


    //! Call to have a new owner for this object
    /*! This function should only be called by a2dSmrtPtr

        \remark owning mean that the object calling this member needs to call Release at some time,
                in order to actually release/delete the object.

        \return The return value is the object itself, which is now owned on time extra.

        increment refcount by 1 ( use when adding a reference to this object)
    */
    a2dComEvent* SmrtPtrOwn() { m_refcount++; return this; }

    //!To release the object, it is not longer owned by the calling object.
    /*! This function should only be called by a2dSmrtPtr
    */
    bool SmrtPtrRelease()
    {
        m_refcount--;
        wxASSERT_MSG( m_refcount >= 0, wxT( "a2dComEvent Own/Release not matched (extra Release calls)" ) );
        if ( m_refcount <= 0 )
        {
            delete this;
            return true;
        }
        return false;
    }

    friend class a2dSmrtPtrBase;

    //!how many references to this object do exist
    int m_refcount;

    //! information of the event is in form of a property.
    a2dNamedProperty* m_property;

    //! id of return option
    a2dSignal m_comReturn;

    //! when true, will delete the property itself.
    bool m_ownProp;
};

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DGENERALDLLEXP a2dSmrtPtr<class a2dComEvent>;
template class A2DGENERALDLLEXP std::allocator<class a2dSmrtPtr<class a2dComEvent> >;
template class A2DGENERALDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dComEvent>, std::allocator<class a2dSmrtPtr<class a2dComEvent> > >::_Node >;
template class A2DGENERALDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dComEvent>, std::allocator<class a2dSmrtPtr<class a2dComEvent> > >::_Nodeptr >;
template class A2DGENERALDLLEXP std::list<class a2dSmrtPtr<class a2dComEvent> >;
template class A2DGENERALDLLEXP a2dlist<class a2dSmrtPtr<class a2dComEvent> >;
template class A2DGENERALDLLEXP a2dSmrtPtrList<class a2dComEvent>;
#endif

typedef void ( wxEvtHandler::*a2dComEventFunction ) ( a2dComEvent& );

#define a2dComEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(a2dComEventFunction, &func)

/*! \addtogroup eventhandlers
*  @{
*/

//! static wxEvtHandler for communication event
#define EVT_COM_EVENT(func) wx__DECLARE_EVT0( a2dEVT_COM_EVENT, a2dComEventHandler( func ))

//! static wxEvtHandler for communication event with specific id.
#define EVT_COM_EVENT_ID(id, func) \
    wx__DECLARE_EVT1( a2dEVT_COM_EVENT, id, a2dComEventHandler( func ))


//!  @} eventhandlers


extern const a2dError a2dError_NoError; /*!< No Error */
extern const a2dError a2dError_CommandError; /*!< command syntax */
extern const a2dError a2dError_NotSpecified; /*!< Not Specified Error */
extern const a2dError a2dError_CanceledFileDialog; /*!< canceled file dialog */
extern const a2dError a2dError_FileCouldNotOpen; /*!< file  could not be opened */
extern const a2dError a2dError_CouldNotCreateDocument; /*!< a document could not be created from the template */
extern const a2dError a2dError_NoDocTemplateRef; /*!< a document template was not found */
extern const a2dError a2dError_DocumentsOpened; /*!< To many documents open */
extern const a2dError a2dError_SetEnv; /*!< SetEnv failed */
extern const a2dError a2dError_GetEnv; /*!< GetEnv failed */
extern const a2dError a2dError_GetVar; /*!< GetVariable failed wrong name*/
extern const a2dError a2dError_SaveFile; /*!< SaveFile in a2dDocument*/
extern const a2dError a2dError_LoadFile; /*!< LoadFile in a2dDocument*/
extern const a2dError a2dError_ExportFile; /*!< ExportFile in a2dDocument*/
extern const a2dError a2dError_ImportFile; /*!< ImportFile in a2dDocument*/
extern const a2dError a2dError_IOHandler; /*!< a2dIOHandler In or Out problem*/
extern const a2dError a2dError_SaveObject; /*!< SaveObject in a2dDocument*/
extern const a2dError a2dError_LoadObject; /*!< LoadObject in a2dDocument*/
extern const a2dError a2dError_FileHistory; /*!< File history*/
extern const a2dError a2dError_ImportObject; /*!< ImportObject in a2dDocument*/
extern const a2dError a2dError_XMLparse; /*!< XML parsing error*/
extern const a2dError a2dError_FileVersion; /*!< version of file not for this version of application*/
extern const a2dError a2dError_LoadLayers; /*!< Layer Load problems */
extern const a2dError a2dError_property; /*!< property problems */
extern const a2dError a2dError_NoView; /*!< current view not set */
extern const a2dError a2dError_NoDocument; /*!< current document not set */
extern const a2dError a2dError_NoController; /*!< no controller set */
extern const a2dError a2dError_NoTool; /*!< no tool available */
extern const a2dError a2dError_LinkRef; /*!< Linking references problem */
extern const a2dError a2dError_NoWrapper; /*!< No wrapper with that name */
extern const a2dError a2dError_LinkPin; /*!< Linking pins problem */
extern const a2dError a2dError_NoPinClass; /*!< a2dPinClass of a2dPin not found */
extern const a2dError a2dError_CouldNotEvaluatePath; /*!< path containing aliases/variables could not be evaluated correctly */
extern const a2dError a2dError_Canceled; /*!< file dialog was canceled */
extern const a2dError a2dError_CouldNotLoad; /*!<  Could not load */
extern const a2dError a2dError_NoSelection; /*!<  no selection made */
extern const a2dError a2dError_ToManyOpen; /*!< to many open documents */
extern const a2dError a2dError_canvasObjectRelease; /*!< could not release canvas objects */


//! holds one error report.
/*!
    Error within the docview framework are reported to the one and only a2dDocviewGlobal instance.
    There they are store in a list of this class.
    Each a2dError holds a message and an errorcode. The error code can be used to check
    the eror independent of language.

    \ingroup general
*/
class A2DGENERALDLLEXP a2dError: public wxObject
{
public:

    //! default constructor
    a2dError()
        : m_code( a2dError_NoError.GetErrorCode() )
    {
    }

    //! constructor
    a2dError( const wxString& errorIdName, const wxString& error = wxEmptyString, bool warning = false )
    {
        if ( errorIdName == wxEmptyString )
            m_code = wxXmlResource::GetXRCID( wxT( "a2dError_NoError" ) );
        else
            m_code = wxXmlResource::GetXRCID( errorIdName );
        m_error = error;
        m_warning = warning;
    }

    a2dError( const a2dError& error )
    {
        m_code = error.m_code;
        m_error = error.m_error;
        m_warning = error.m_warning;
    }

    bool operator==( const a2dError& a )
    {
        return m_code == a.m_code;
    }

    bool operator!=( const a2dError& a )
    {
        return m_code != a.m_code;
    }

    friend inline bool operator==( const a2dError& a, const a2dError& b )
    {
        return a.m_code == b.m_code;
    }

    friend inline bool operator!=( const a2dError& a, const a2dError& b )
    {
        return a.m_code != b.m_code;
    }

    //! name of the name of this error id
    wxString GetIdName() const { return m_errorIdName; }

    //! get error string
    inline wxString GetErrorMessage() const { return wxGetTranslation( m_error ); }
    //! get error code
    inline a2dErrorWarningCode GetErrorCode() const { return m_code; }

    //! set error message
    inline void SetErrorMessage( const wxString& error ) { m_error = error; }
    //! set error code
    inline void SetErrorCode( a2dErrorWarningCode code ) { m_code = code; }

    static a2dErrorHash& GetHashMap();

private:

    wxString m_errorIdName;

    //! error string
    wxString m_error;

    //! error string
    wxString m_format;

    //! error code
    a2dErrorWarningCode m_code;

    //! is it a warning only
    bool m_warning;
};


class A2DGENERALDLLEXP a2dObject;
class A2DGENERALDLLEXP a2dHashMapIntToObject;
class A2DGENERALDLLEXP a2dHashMapIntToObject_wxImplementation_HashTable;
class A2DGENERALDLLEXP a2dHashMapIntToObject_wxImplementation_KeyEx;

class A2DGENERALDLLEXP a2dHashMapCreateObject;
class A2DGENERALDLLEXP a2dHashMapCreateObject_wxImplementation_HashTable;
class A2DGENERALDLLEXP a2dHashMapCreateObject_wxImplementation_KeyEx;

class A2DGENERALDLLEXP a2dNamedProperty;
class A2DGENERALDLLEXP a2dNamedPropertyList;

//! This hash table is used for serialization of multiple referenced object
WX_DECLARE_HASH_MAP_WITH_DECL( wxString, class a2dObject*, wxStringHash, wxStringEqual, a2dHashMapIntToObject, class A2DGENERALDLLEXP );

//! This hash table is used for creating object using a symbolic name
WX_DECLARE_HASH_MAP_WITH_DECL( wxString, wxString, wxStringHash, wxStringEqual, a2dHashMapCreateObject, class A2DGENERALDLLEXP );

//! class used in a2dResolveIdList to resolve references to other objects while readin in a file.
/*!
    a2dObject can be referenced multiple times, in certain file formats this can be maintained.
    When reading back such files, the original references need to be restored.
    When reading a reference from a file, the object that it references might not have bin read yet.
    Therefore if not found they are first stored, and when all is read those are resolved.
    The a2dResolveIdList is used to store onsolved references, while all objects which can be referenced to,
    are stored in a2dHashMapIntToObject so they can be quickly found.

    \ingroup fileio

*/
class A2DGENERALDLLEXP a2dResolveIdInfo : public wxObject
{
public:
    a2dResolveIdInfo( const wxString& idStr, a2dSmrtPtr<a2dObject>* storedHere )
    {
        m_mode = mode_a2dObjectSmrtPtr;
        m_idStr = idStr;
        m_storedHere.m_a2dObjectSmrtPtr = storedHere;
    }

    a2dResolveIdInfo( const wxString& idStr, a2dAutoZeroPtr<a2dObject>* storedHere )
    {
        m_mode = mode_a2dObjectAutoZeroPtr;
        m_idStr = idStr;
        m_storedHere.m_a2dObjectAutoZeroPtr = storedHere;
    }

    a2dResolveIdInfo( const wxString& idStr, a2dObject* link )
    {
        m_mode = mode_link;
        m_idStr = idStr;
        m_storedHere.m_link = link;
    }

    a2dResolveIdInfo( a2dObject* inform )
    {
        m_mode = mode_inform;
        m_storedHere.m_inform = inform;
    }

private:
    bool Assign( a2dObject* ptr );

    enum Mode
    {
        mode_a2dObjectSmrtPtr,
        mode_a2dObjectAutoZeroPtr,
        mode_link,
        mode_inform
    };

    Mode m_mode;

    wxString m_idStr;

    union UPtr
    {
        a2dSmrtPtr<a2dObject> *m_a2dObjectSmrtPtr;
        a2dAutoZeroPtr<a2dObject> *m_a2dObjectAutoZeroPtr;
        a2dObject* m_link;
        a2dObject* m_inform;
    } m_storedHere;

    friend class a2dResolveIdList;
    friend class a2dRefMap;
};

WX_DECLARE_LIST_WITH_DECL( a2dResolveIdInfo, a2dResolveIdList, class A2DGENERALDLLEXP );

#include <map>
typedef a2dSmrtPtr<a2dPropertyId> a2dPropertyIdPtr;
typedef std::map< wxString, a2dPropertyIdPtr > a2dDynamicIdMap;

//! list of a2dNamedProperty objects
/*!
    \remark this is only used for lists handled e.g. as paramaters outside of
    a2dObject. a2dObject handles the lists on its own.

    \ingroup property
*/
class A2DGENERALDLLEXP a2dNamedPropertyList : public a2dSmrtPtrList<a2dNamedProperty>
{
public:
    //! Default constructor
    a2dNamedPropertyList()
    {
    }
    //! Copy constructor (clones all properties and copies the list)
    a2dNamedPropertyList( const a2dNamedPropertyList& other );
    //! Destructor
    ~a2dNamedPropertyList();
    //! Remove first/all properties with given id from the list
    /*! return true if the property was found */
    bool Remove( const a2dPropertyId* id, bool all = true );
    //! Remove all temporrary properties from the list
    void RemoveTemporary();

    //! walk a list of properties
    void Walker( wxObject* parent, a2dWalkerIOHandler& handler );


#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    wxObject* m_owner;
#endif

};

//! check if class has the given id as a valid id for this object
/*!
    The id hashlist for the class and base class etc. is checked for the given id.
*/
#define DECLARE_PROPERTIES() \
    static bool InitializePropertyIds();\
    static a2dDynamicIdMap& GetPropertyIdMap();\
    static bool AddPropertyId( a2dPropertyId *dynproperty );\
    virtual bool HasPropertyId( const a2dPropertyId* id ) const;\
    virtual a2dPropertyId* HasPropertyId( const wxString& name );\
    virtual const a2dNamedProperty* FindProperty( const wxString &idName ) const;\
    virtual void CollectProperties2( a2dNamedPropertyList *total, const a2dPropertyId *id = NULL, a2dPropertyId::Flags flags = a2dPropertyId::flag_none ) const;\
    private:\
    static a2dDynamicIdMap sm_dymPropIds;

#define INITIALIZE_PROPERTIES_BASE( classname ) \
    a2dDynamicIdMap& classname::GetPropertyIdMap() \
    {\
        return sm_dymPropIds;\
    }\
    bool classname::AddPropertyId( a2dPropertyId *dynproperty )\
    {\
        if ( sm_dymPropIds.find( dynproperty->GetName() ) == sm_dymPropIds.end() )\
            sm_dymPropIds[ dynproperty->GetName() ]= dynproperty;\
        else\
            wxASSERT_MSG( 0, _("The property id name '") + dynproperty->GetName() + _("' already exists in #classname") );\
        return true;\
    }\
    bool classname::HasPropertyId( const a2dPropertyId* id ) const\
    {\
        a2dDynamicIdMap::iterator iter = sm_dymPropIds.find( id->GetName() );\
        if ( iter != sm_dymPropIds.end() )\
            return true;\
        return false;\
    }\
    a2dPropertyId* classname::HasPropertyId( const wxString& name )\
    {\
        a2dDynamicIdMap::iterator iter = sm_dymPropIds.find( name );\
        if ( iter != sm_dymPropIds.end() )\
            return sm_dymPropIds[ name ];\
        return NULL;\
    }\
    const a2dNamedProperty *classname::FindProperty( const wxString &idName ) const\
    {\
        a2dDynamicIdMap::iterator iter = sm_dymPropIds.find( idName );\
        if ( iter != sm_dymPropIds.end() )\
        {\
            a2dPropertyIdPtr propId = ( *iter ).second;\
            return GetProperty( propId );\
        }\
        return 0;\
    }\
    void classname::CollectProperties2( a2dNamedPropertyList *total, const a2dPropertyId *id, a2dPropertyId::Flags flags ) const\
    {\
        for ( a2dDynamicIdMap::iterator i = sm_dymPropIds.begin(); i != sm_dymPropIds.end(); i++ )\
        {\
            a2dPropertyIdPtr p = (*i).second;\
            if( p->CheckCollect( id, flags ) )\
            {\
                a2dNamedProperty* dprop = p->GetPropertyAsNamedProperty( this );\
                if ( dprop )\
                    total->push_back( dprop );\
            }\
        }\
    }\
    a2dDynamicIdMap classname::sm_dymPropIds; \
    static bool initProp##classname = classname::InitializePropertyIds(); \
    bool classname::InitializePropertyIds()


#define INITIALIZE_PROPERTIES( classname, baseclassname ) \
    a2dDynamicIdMap& classname::GetPropertyIdMap() \
    {\
        return sm_dymPropIds;\
    }\
    bool classname::AddPropertyId( a2dPropertyId *dynproperty )\
    {\
        if ( sm_dymPropIds.find( dynproperty->GetName() ) == sm_dymPropIds.end() )\
            sm_dymPropIds[ dynproperty->GetName() ]= dynproperty;\
        else\
            wxASSERT_MSG( 0, _("The property id name '") + dynproperty->GetName() + _("' already exists in #classname") );\
        return true;\
    }\
    bool classname::HasPropertyId( const a2dPropertyId* id ) const\
    {\
        a2dDynamicIdMap::iterator iter = sm_dymPropIds.find( id->GetName() );\
        if ( iter != sm_dymPropIds.end() )\
            return true;\
        return baseclassname::HasPropertyId( id );\
    }\
    a2dPropertyId* classname::HasPropertyId( const wxString& name )\
    {\
        a2dDynamicIdMap::iterator iter = sm_dymPropIds.find( name );\
        if ( iter != sm_dymPropIds.end() )\
            return sm_dymPropIds[ name ];\
        return baseclassname::HasPropertyId( name );\
    }\
    const a2dNamedProperty *classname::FindProperty( const wxString &idName ) const\
    {\
        a2dDynamicIdMap::iterator iter = sm_dymPropIds.find( idName );\
        if ( iter != sm_dymPropIds.end() )\
        {\
            a2dPropertyIdPtr propId = ( *iter ).second;\
            return GetProperty( propId );\
        }\
        return baseclassname::FindProperty( idName );\
    }\
    void classname::CollectProperties2( a2dNamedPropertyList *total, const a2dPropertyId *id, a2dPropertyId::Flags flags ) const\
    {\
        for ( a2dDynamicIdMap::iterator i = sm_dymPropIds.begin(); i != sm_dymPropIds.end(); i++ )\
        {\
            a2dPropertyIdPtr p = (*i).second;\
            if( p->CheckCollect( id, flags ) )\
            {\
                a2dNamedProperty* dprop = p->GetPropertyAsNamedProperty( this );\
                if ( dprop )\
                    total->push_back( dprop );\
            }\
        }\
        return baseclassname::CollectProperties2( total, id, flags );\
    }\
    a2dDynamicIdMap classname::sm_dymPropIds; \
    static bool initProp##classname = classname::InitializePropertyIds(); \
    bool classname::InitializePropertyIds()

#ifdef WXVALIDEVENTENTRY 
#else
#if !wxCHECK_VERSION(2,9,0)

// an entry used in dynamic event table managed by wxEvtHandler::Connect()
struct WXDLLIMPEXP_BASE a2dDynamicEventTableEntry : public wxDynamicEventTableEntry
{
    a2dDynamicEventTableEntry( int evType, int winid, int idLast,
                               wxObjectEventFunction fn, wxObject* data, wxEvtHandler* eventSink )
        : wxDynamicEventTableEntry( evType, winid, idLast, fn, data, eventSink ),
          m_disconnect( false )
    { }

    bool m_disconnect:1;

    DECLARE_NO_COPY_CLASS( a2dDynamicEventTableEntry )
};
#else
// an entry used in dynamic event table managed by wxEvtHandler::Connect()
struct WXDLLIMPEXP_BASE a2dDynamicEventTableEntry : public wxDynamicEventTableEntry
{
    a2dDynamicEventTableEntry( int evType, int winid, int idLast,
                               wxEventFunctor* fn, wxObject* data )
        : wxDynamicEventTableEntry( evType, winid, idLast, fn, data ),
          m_disconnect( false )
    { }

    ~a2dDynamicEventTableEntry()
    { }

    bool m_disconnect:1;

    DECLARE_NO_COPY_CLASS( a2dDynamicEventTableEntry )
};
#endif
#endif

//! Ref Counted base object.
/*!
    Used for counting the number of objects that own this (derived) object.

    This object is the base object for most classes which used a2dSmrtPtr to keep track of them.
    An instance of a2dObject is held by a a2dObjectPtr, and many of those can point to the same a2dObject.
    When the a2dObjectPtr goes out of scope, automatically the reference count will be decremented.
    When the last a2dObjectPtr holding on to the a2dObject goes is destructed, the a2dObject will be deleted
    too.

    Smart pointers are ideal for holding events, since you can hold on to it until you think it is oke to
    release them.
    In wxDocview smart pointers are used for a2dCanvasObject's, that makes it possible to have a hierarchy of
    drawable objects, a multiple references a2dCanvasObject is drawn at a different locations, because its
    parent is responsible for its position.


    a2dObject has a a2dNamedProperty List containing dynamic properties.
    All objects derived from this can have dynamic properties added to them.
    For that there is a property list, to which those properties are added.
    Functions to manipulate the properties are all concentrated in this class.

    A property is a2dNamedProperty class, which are unique via a a2dPropertyId identification class.

    Assume that a derived class a2dCanvasObject has:

    <code>
        //declare of static member in the class
        static const a2dPropertyIdBoundingBox PROPID_BoundingBox;

        //initialize static member in cpp file
        const a2dPropertyIdBoundingBox a2dCanvasObject::PROPID_BoundingBox( CLASSNAME( a2dCanvasObject ), wxT("statebox"),  a2dPropertyId::flag_temporary|a2dPropertyId::flag_multiple, a2dBoundingBox() );

    </code>
        We can set it like this:
    <code>
        a2dBoundingBox updatearea(0,0,100,100);
        a2dCanvasObject::PROPID_BoundingBox->SetPropertyToObject( a_canvasobject, updatearea );
        a2dBoundingBox updatearea2(0,0,100,100);
        a2dCanvasObject::PROPID_BoundingBox->SetPropertyToObject( a_canvasobject, updatearea2 );
    </code>

    see a2dPropertyId a2dPropertyIdTyped for more.

    \ingroup property

    \remark Little is needed to make it carbage collected.
    \remark Specilized memeory allocation is possible e.g like in a2dFill with a2dMemManager
    \remark And debugging on id, helps a lot.

    \ingroup general
*/
class A2DGENERALDLLEXP a2dObject : public wxEvtHandler
{

    DECLARE_ABSTRACT_CLASS( a2dObject )

public:

#if wxCHECK_VERSION(3,1,0)

    virtual bool ProcessEvent( wxEvent& event );

    bool SearchDynamicEventTable( wxEvent& event );

    bool TryBeforeAndHere( wxEvent& event )
    {
        return TryBefore( event ) || TryHereOnly( event );
    }

    bool DoTryChain( wxEvent& event );

    bool ProcessEventLocally( wxEvent& event );

    bool TryHereOnly( wxEvent& event );

    void Connect( int winid,
                  int lastId,
                  wxEventType eventType,
                  wxObjectEventFunction func,
                  wxObject* userData = NULL,
                  wxEvtHandler* eventSink = NULL )
    {
        DoBind( winid, lastId, eventType,
                wxNewEventFunctor( eventType, func, eventSink ),
                userData );
    }

    bool Disconnect( int winid,
                     int lastId,
                     wxEventType eventType,
                     wxObjectEventFunction func = NULL,
                     wxObject* userData = NULL,
                     wxEvtHandler* eventSink = NULL )
    {
        return DoUnbind( winid, lastId, eventType,
                         wxMakeEventFunctor( eventType, func, eventSink ),
                         userData );
    }

    void CleanUpDisconnectedDynamic();

private:
    void DoBind( int winid,
                 int lastId,
                 wxEventType eventType,
                 wxEventFunctor* func,
                 wxObject* userData = NULL );

    bool DoUnbind( int winid,
                   int lastId,
                   wxEventType eventType,
                   const wxEventFunctor& func,
                   wxObject* userData = NULL );
public:

    //! Used to connect an event coming from classA to classB as a dynamic event.
    //! The dynamic can be handled by the static event table of classB.
    //! The event is handled by calling ProcessEvent(), but will always be skipped.
    void ProcessConnectedEvent( wxEvent& event );

    //! Connect an event from coming classA to classB as a dynamic event.
    //! The event is handled by calling ProcessEvent(), but will always be skipped.
    //! \param eventSink ClassB in which to receive the event.
    void ConnectEvent( wxEventType type, wxEvtHandler* eventSink );

    //! Disconnect an event from classA to classB as a dynamic event.
    //! The event is handled by calling ProcessEvent(), but will always be skipped.
    //! \param eventSink ClassB in to which connection was made.
    bool DisconnectEvent( wxEventType type, wxEvtHandler* eventSink );

    //! Remove all dynamic events in classA, going to classB (eventSink)
    bool DisconnectEventAll( wxEvtHandler* eventSink );

#else //wxCHECK_VERSION(3,1,0)

#ifdef WXVALIDEVENTENTRY

    bool SearchDynamicEventTable( wxEvent& event );

#else //WXVALIDEVENTENTRY 
    virtual bool ProcessEvent( wxEvent& event );

    bool SearchDynamicEventTable( wxEvent& event );

#if !wxCHECK_VERSION(2,9,0)

    // Dynamic association of a member function handler with the event handler,
    // winid and event type
    void Connect( int winid,
                  int lastId,
                  int eventType,
                  wxObjectEventFunction func,
                  wxObject* userData = ( wxObject* ) NULL,
                  wxEvtHandler* eventSink = ( wxEvtHandler* ) NULL );
    bool Disconnect( int winid,
                     int lastId,
                     wxEventType eventType,
                     wxObjectEventFunction func = NULL,
                     wxObject* userData = ( wxObject* ) NULL,
                     wxEvtHandler* eventSink = ( wxEvtHandler* ) NULL );

#else //!wxCHECK_VERSION(2,9,0)

    bool TryBeforeAndHere( wxEvent& event )
    {
        return TryBefore( event ) || TryHereOnly( event );
    }

    bool DoTryChain( wxEvent& event );

    bool ProcessEventLocally( wxEvent& event );

    bool TryHereOnly( wxEvent& event );

    void Connect( int winid,
                  int lastId,
                  wxEventType eventType,
                  wxObjectEventFunction func,
                  wxObject* userData = NULL,
                  wxEvtHandler* eventSink = NULL )
    {
        DoBind( winid, lastId, eventType,
                wxNewEventFunctor( eventType, func, eventSink ),
                userData );
    }

    bool Disconnect( int winid,
                     int lastId,
                     wxEventType eventType,
                     wxObjectEventFunction func = NULL,
                     wxObject* userData = NULL,
                     wxEvtHandler* eventSink = NULL )
    {
        return DoUnbind( winid, lastId, eventType,
                         wxMakeEventFunctor( eventType, func, eventSink ),
                         userData );
    }

private:
    void DoBind( int winid,
                 int lastId,
                 wxEventType eventType,
                 wxEventFunctor* func,
                 wxObject* userData = NULL );

    bool DoUnbind( int winid,
                   int lastId,
                   wxEventType eventType,
                   const wxEventFunctor& func,
                   wxObject* userData = NULL );
#endif //!wxCHECK_VERSION(2,9,0)
#endif //WXVALIDEVENTENTRY 

public:

    //! Used to connect an event coming from classA to classB as a dynamic event.
    //! The dynamic can be handled by the static event table of classB.
    //! The event is handled by calling ProcessEvent(), but will always be skipped.
    void ProcessConnectedEvent( wxEvent& event );

    //! Connect an event from coming classA to classB as a dynamic event.
    //! The event is handled by calling ProcessEvent(), but will always be skipped.
    //! \param eventSink ClassB in which to receive the event.
    void ConnectEvent( wxEventType type, wxEvtHandler* eventSink );

    //! Disconnect an event from classA to classB as a dynamic event.
    //! The event is handled by calling ProcessEvent(), but will always be skipped.
    //! \param eventSink ClassB in to which connection was made.
    bool DisconnectEvent( wxEventType type, wxEvtHandler* eventSink );

    //! Remove all dynamic events in classA, going to classB (eventSink)
    bool DisconnectEventAll( wxEvtHandler* eventSink );

#endif //wxCHECK_VERSION(3,1,0)

    //!constructor
    a2dObject();

    //!destructor
    virtual ~a2dObject();

    //!assignment operator
    void operator = ( const a2dObject& other );

    //! like it to be protected, but this does not work with wxList macros
    //void operator delete(void* recordptr);

    //! used together with wxObject debug possibility, sent a string containing m_id to the output stream.
#if wxUSE_STD_IOSTREAM && (defined(__WXDEBUG__) || wxUSE_DEBUG_CONTEXT)
    virtual void Dump( wxSTD ostream& str );
#endif

    //!get the reference count, how many owners/references are made to this object.
    /*!
    normally the object should only be deleted is the refcount is 0, which means that is not
    longer owned by any other object.
    Use Release() For that.
    */
    inline int GetOwnedBy() { return m_refcount; }

    //! Call to mark this object as non-dynamic (static/local/auto)
    /*!
        Used to have an a2dObject as a non pointer/smart pointer.

        \return The return value is the object itself.

        \remark You need NOT call Release, but you must destruct the object (usually automatic)
    */
    virtual a2dObject* SetNonDynamic();

    //!Check zero reference count and delete if count is 0
    /*! This is usefull if a reference counted object was never owned.
        Then the reference counter is always 0 and Release will never be called.
        As not all pointers (e.g. temporary, paramaters) call Own, this may happen.
    */
    void CheckReferences();

    //! options for cloning
    /*!
        a2dObject is reference counted, and therefore each object can have more then one owner.
        For this reason there are many ways to clone a tree of nested a2dObject's.
        The options here are meant to stear this clone process for nested childs and/or properties.
        Although a a2dObject by itself does not have properties nor child objects, the idea is that
        in derived objects there will be such a structure, a2dObject and a2dCanvasObject to start with.
    */
    enum CloneOptions
    {
        //! if set, clone members (e.g. line end styles), otherwise ref-copy them
        clone_members       = 0x00000001,
        //! if set, clone properties (e.g. fill style), otherwise ref-copy them
        clone_properties    = 0x00000002,
        //! if set, clone childs, otherwise ref-copy them
        clone_childs        = 0x00000004,
        //! cloned version will be named the same
        clone_sameName      = 0x00000008,
        //! if set, set in the clone the PROPID_Original property to the copied object
        /*! clone_setoriginal is done recursively, if childs are cloned */
        clone_setoriginal   = 0x00000010,
        //! if set, set in the clone the PROPID_editcopy property to the original object
        /*! clone_seteditcopy is NOT done recursively, even if childs are cloned */
        clone_seteditcopy   = 0x00000020,
        //! Prevent cloning a reference at a deeper level
        clone_noReference   = 0x00000040,
        //! Prevent cloning a a2dCameleon reference at a deeper level
        clone_noCameleonRef = 0x00000080,
        //! Hint to clone enough of the object to do proper dragging in graphics
        clone_toDrag        = 0x00000100,
        //! m_root of a2dCanvasObject will not be set to original, but to NULL.
        clone_resetRoot     = 0x00000200,

        clone_flat          = 0x00000000,
        clone_deep          = clone_members | clone_properties | clone_childs,
    };
    friend inline CloneOptions operator|( CloneOptions a, CloneOptions b )
    {
        return ( CloneOptions )( ( int )a | ( int ) b );
    }

    //! create an exact copy of this property
    a2dObject* Clone( CloneOptions options, a2dRefMap* refs = NULL ) const;

    a2dObject( const a2dObject& other, CloneOptions options, a2dRefMap* refs );

    //!Returns the name of this object, if no name is given the internal id will be returned
    /*!
        Gives the name of the object in string form,
        or when the a2dStringProperty PROPID_Name is set, its value will be returned

        \remark
        Don't rely on the internal id, because it's a temporary value.
        If you want to save this value, it's more secure to set a name because the
        name can be persistent instead of the internal id.

        \see SetName
        \see a2dObject::GetId
     */
    virtual wxString GetName() const;

    //! Creates the a2dStringProperty PROPID_Name
    /*! If set, the name will be returned for GetName() from now on.
        \param name name to be set for the object
    */
    virtual void SetName( const wxString& name );

    //!Check if this is a temporary object, which should not be saved
    virtual bool IsTemporary_DontSave() const { return false; }

#if wxART2D_USE_CVGIO

    //! Save this object and all below as an XML document
    /*!
        <code>
        //check flag is used for writing once an object which is referenced many times
        a2dWalker_SetCheck setp( false );
        setp.Start( m_rootObject );
        setp.Start( m_layersetup );
        a2dIOHandlerXmlSerOut out;
        SaveAsDocument( out );
        <code/>

    */
    void SaveAsDocument( a2dIOHandlerXmlSerOut& out );

    //!write all needed to an XML type of file called the CVG format
    /*!
        \param parent parent object from where this was called.
        \param out XML io handler
        \param towrite nested object to write later.
    */
    virtual void Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite );

    //! Load this object and all below from an XML document
    /*!
        <code>
        a2dWalker_SetBoolProperty setp( PROPID_Check, false );
        setp.Start( this );
        a2dIOHandlerXmlSerIn parser;
        SaveAsDocument( parser );
        <code/>

    */
    void LoadFromDocument( a2dIOHandlerXmlSerIn& parser );

    //!load object from CVG file
    /*!
        \param parent parent object from where this was called.
        \param parser basic pull parser for XML content
    */
    virtual void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser );

#endif //wxART2D_USE_CVGIO

    //! return a unique id for this object
    /*! The id is derived from time
    */
    wxLongLong GetId() const { return m_id; }

    //! set id for this object
    /*! 
        Normally set with GetUniqueSerializationId(), to make it unique.
    */
    void SetId( wxLongLong id ) { m_id = id; }

    //! If true, always write an id
    /*! Usually an id is only written if the object is multiple referenced and
        refcount is >= 2. Some objects, namely a2dPin objects, are multiple
        referenced even if refcount = 1.
    */
    virtual bool AlwaysWriteSerializationId() const;

    //! link a reference in the object to the given value
    /*! This is intended for e.g. paired links as in a2dPin, where the usual a2dResolveIdInfo mechanism cannot be used. */
    virtual bool LinkReference( a2dObject* other );

    //! inform an object that reference resolution was done
    /*! This is intended for objects that need to do something with loaded objects */
    virtual void ResolveInform(  );

    //! This is used to recursively walk through an object tree
    void Walker( wxObject* parent, a2dWalkerIOHandler& handler );

    //!general flag use at will.
    /*!
        \remark
        This flag should only be used for temporarly purposes.
        This object uses this flag too and you might run into problems if you use this flag.
        It's a good practice to set this flag if you need it and reset this flag to <code>false</code>
        if you don't need it anymore. Another possibility might be to add a new property to this object
        if you want to be on the secure side.

        \param check temporarely status information
    */
    inline void SetCheck( bool check ) { m_check = check; }

    //!general flag use at will.
    inline bool GetCheck() const {return m_check;}

    //!set release flag
    /*!If this flag is set, the object will be released on the next update */
    void SetRelease( bool value ) { m_release = value; }

    //!get release flag
    /*!If this flag is set, the object will be released on the next update */
    bool GetRelease( ) const { return m_release; }

public:

    //! return the list head pointer for autozero pointers to this object
    /*! This is a void pointer, because there might be auto zero pointers
        of various different types (for derived classes) pointing to this object,
        so a strictly typed pointer isn't possible anyway
    */
    a2dAutoZeroPtr<a2dObject> ** GetAutoZeroPtrList() { return &m_autozeroptrlist; }
    //! this is needed inside the smart pointer template code
    typedef a2dAutoZeroPtr<a2dObject> TAutoZeroPtrListClass;

private:

    //! Call to have a new owner for this object
    /*! This function should only be called by a2dSmrtPtr

        \remark owning mean that the object calling this member needs to call Release at some time,
                in order to actually release/delete the object.

        \return The return value is the object itself, which is now owned on time extra.

        increment refcount by 1 ( use when adding a reference to this object)
    */
    virtual a2dObject* SmrtPtrOwn()
    {
        m_refcount++; return this;
    }

    //!To release the object, it is not longer owned by the calling object.
    /*! This function should only be called by a2dSmrtPtr
    */
    virtual bool SmrtPtrRelease();

    //! wxProperty is a friend and allowed to use these functions
    friend class a2dPropertyId;

protected:

    //! used to decide if a property shall be ignored, if it is not a member
    /*! The base class definition ignores all ids, that have the "flag_onlymember" set
        Derived classes might want to check the flag_onlymemberhint or specific ids. */
    virtual bool DoIgnoreIfNotMember( const a2dPropertyId* id ) const { return id->IsOnlyMember(); }

	virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const = 0;

    //********************** PROPERTIES *********************/
    /*! \name Properties
        a a2dObject has a list of general named properties
    */
    //\{
public:


    //! get property on this object
    /*!
        If the propertyId has member or member function pointers, that will be used to get the value.
        Else the property list will be searched for the right propertyId.
        In any case the value of the property is wrapped into a new a2dNamedProperty, which is returned.
        You must delete the returned object yourself.
    */
    a2dNamedProperty* GetProperty( const a2dPropertyId* propertyId, a2dPropertyId::Flags flags = a2dPropertyId::flag_none ) const;

    //! Set the property to the this object.
    /*!
        The a2dNamedProperty to set to the object as indicated by its a2dPropertyId, is used as as,
        or the value is taken from it and assigned to something in the object and the property is deleted itself in the end.

        - The property given will be \b owned by the object or deleted, so you must give a new or cloned property
        - Derived class versions of this function might redirect to a different object (wxHierarchicalId)
        - This is the recommended function for setting a property object in/to a target object
        - Typed derived classes supply a SetPropertyToObject function that takes a value. That is the prefered method.
    */
    void SetProperty( a2dNamedProperty* propertyHolder, a2dPropertyId::SetFlags flags = a2dPropertyId::set_none );

    //! Set the property to the this object and if enabled, to all childs and object members
    /*!
        The a2dNamedProperty to set to the object as indicated by this a2dPropertyId is cloned and like that added to the object,
        or the value is taken from it and assigned to something in the object.

        - The property given will <b> NOT be owned </b> by the object or deleted. You can give a temporary object. If required, the property will be cloned.
        - Derived class versions of this function might redirect to a different object (wxHierarchicalId)
        - This is the recommended function for setting a property object in/to a target object
        - Typed derived classes supply a SetPropertyToObject function that takes a value. This is the prefered method.
    */
    void SetProperty( const a2dNamedProperty& propertyHolder, a2dPropertyId::SetFlags flags = a2dPropertyId::set_none );

#ifdef DOXYGEN
    //! Find a property with given the name of the a2dPropertyId.
    /*!\
        Searches the a2dPropertyId with this name on the object, if found, it is returned as
        a a2dNamedProperty containing the value.
    */
    const a2dNamedProperty* classname::FindProperty( const wxString& idName ) const;
#endif // DOXYGEN

    //! Find a dynamic property with given id in the property list.
    /*!
        Searches the Properties stored in the dynamic property list of a a2dObject, not the
        member properties.
    */
    virtual a2dNamedProperty* FindProperty( const a2dPropertyId* id, a2dPropertyId::Flags flags = a2dPropertyId::flag_none );

    //! Find a dynamic property with given id in the property list.
    /*!
        Searches the Properties stored in the dynamic property list of a a2dObject, not the
        member properties.
    */
    virtual const a2dNamedProperty* FindProperty( const a2dPropertyId* id, a2dPropertyId::Flags flags = a2dPropertyId::flag_none ) const;

    //! Check if the object has a property with given id and string representation.
    /*! first the a check is done if the given id is valid for this object.
        If yes, the property value is retrieved and the stringvalue checked.
        If the property id dynamic, and not set, false is returned.
        If stringvalue is an empty string, only availability of the property is checked.
    */
    virtual bool HasProperty( const a2dPropertyId* id, const wxString& stringvalue = wxEmptyString ) const;

    //! This function is called after a property changed
    /*! This is overloaded to set e.g. a pending flag
    */
    virtual void OnPropertyChanged( const a2dPropertyId* id );

    //! edit properties of the object
    /*!
        \param id If property id is set only matching properties are selected
        \param withUndo   If true, the changes can be undone later.
    */
    virtual bool EditProperties( const a2dPropertyId* id, bool withUndo = true );

    //! Properties in the id list are taken from the given object, en set to this.
    /*!
        If some id in the list is not found in this or the other object, it will be skipped.
    */
    void TakeOverProperties( a2dObject* from, a2dPropertyIdList* listOfIds );

    //!quick way to get to PROPID_URI property
    /*!
    \return if no property PROPID_URI is available the return string is empty
    else it is the string containing the complete URL.
    */
    wxURI GetURI() const;

    //! quickly set a property PROPID_URI
    /*!
        This function stores an url string within a property called PROPID_URI
        When writing to SVG (scalable vector graphics), it is translated to the <a> tag.
        This tag is for linking to other svg content or HTML etc.
        See SVG specification, for a complete overview.
        \param url a string containing a complete url
        ex: "xlink:href="http://www.w3.org""
        ex: "href="http://www.w3.org"
        ex: "mydrawing.svg#myid"
    */
    void SetURI( const wxURI& url );

    //! Get the Property List.
    /*! Returns a reference to the propertylist
    */
    const a2dNamedPropertyList& GetPropertyList() const { return m_propertylist; }

    //! test if there are dynamic properties in the m_propertylist
    bool HasDynamicProperties() const
    {
        return !m_propertylist.empty();
    }

    //! This function is called by a2dPropertyId to remove a property from the list
    /*! The base class implementation does nothing */
    virtual bool RemoveProperty( const a2dPropertyId* WXUNUSED( id ), bool all = true );

    //! remove all temporary properties
    void RemoveTemporaryProperties( );

    //! This function is called by a2dPropertyId to add a property to the list
    /*! 
        Best used via the a2dPropertyId.
    */
    virtual bool AddProperty( a2dNamedProperty* property );

    //\}
    //**************** END PROPERTIES ***************/

protected:

    //! iterate over this object and its children
    /*!
        This function allows you to extend the functionality of all a2dObject classes
        in a a2dDocument, without adding extra members to these objects.

        Default functions are called on the a2dWalkerIOHandler, which redirect the
        calls to other functions based on this object its classname.
        On can register classes to a2dWalkerIOHandler or derived classes.
        This way for each unique object in the document there can be a function
        in a2dWalkerIOHandler.

        \return false if some object did not have a function attached via a2dWalkerIOHandler.

        See a2dWalkerIOHandler for more.
    */
    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

#if wxART2D_USE_CVGIO
    //! Save settings.
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite );
    //! Load settings.
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    //!properties
    a2dNamedPropertyList m_propertylist;

    //!how many references to this object do exist
    int m_refcount;

    //! used for linking multiple referenced objects when serializing
    bool m_check: 1;

    //! when set object is treated as being deleted, and wil be deleted in idle time.
    bool m_release: 1;

    //! this is used as a recursion stopper
    bool m_recursion_active: 1;

    bool m_pendingDisconnects: 1;

    unsigned int m_iteratorCount;

    //!this is a list of all a2dAutoZeroPtr object pointing to this object
    a2dAutoZeroPtr<a2dObject> *m_autozeroptrlist;

    wxLongLong m_id;

    //!special refcount value for non-dynamic objects
    enum { refcount_nondynamic = 0x76543210 };

public:

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //! this is a list of all smart pointers pointing to this object
    a2dSmrtPtr<a2dObject> *m_ownerlist;
    //! this is needed inside the smart pointer template code
    typedef a2dSmrtPtr<a2dObject> TOwnerListClass;
    //! Make a Dump of the objects owner list to the Debug console
    /*! This function is usually called from the Debuggers watch window */
    void DumpOwners();
#endif


    //! used for linking multiple referenced objects when serializing
    static a2dPropertyIdBool* PROPID_Check;
    //! attach an URL to the object
    static a2dPropertyIdUri* PROPID_URI;
    //! Name property, to return name of this object
    static a2dPropertyIdString* PROPID_Name;
    static a2dPropertyIdVoidPtr* PROPID_autozeroptrlist;
    //! edit properties event, see EditProperties()
    static const a2dSignal sm_Edit_properties;

    DECLARE_PROPERTIES();

private:
    friend class a2dSmrtPtrBase;

    //!this is a not implemented copy constructor that avoids automatic creation of one
	a2dObject( const a2dObject& other );

};

//! smart pointer to a2dObject
typedef a2dSmrtPtr<a2dObject> a2dObjectPtr;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
//! we must check this->m_refcount != 0 before keepalive = this
//! keepalive used in ProcessEvent, maybe in destuctor a2dObject where m_refcount already 0,
//! so destructor is called second time when is destroing keepalive
#define a2dREFOBJECTPTR_KEEPALIVE  CurrentSmartPointerOwner = this;\
    a2dObjectPtr keepalive(__FILE__, __LINE__,(this->m_refcount != 0 ? this : NULL ))
#else
//! we must check this->m_refcount != 0 before keepalive = this
//! keepalive used in ProcessEvent, maybe in destuctor a2dObject where m_refcount already 0,
//! so destructor is called second time when is destroing keepalive
#define a2dREFOBJECTPTR_KEEPALIVE  a2dObjectPtr keepalive = (this->m_refcount != 0 ? this : NULL )
#endif

//! is a a2dAutoZeroPtr<a2dObject>
typedef a2dAutoZeroPtr<a2dObject> a2dObjectAutoZeroPtr;

//! simple ref counted class to maintain a wxObject* as if it is a smart pointer
/*!
    If you want to store wxObject pointers in a STL template list ( like a2dlist ), that becomes a problem.
    STL stores object by value, and does not delete the object pointed to when removing a pointer from the list.
    The trick is to store a2dSmrtPtr< wxObject> in the list, which will automatically delete wxObject when a smart pointer.
    But a a2dSmrtPtr< wxObject> can not be created since wxObject has no refcounting as needed for a2dSmrtPtr.
    Now this class wraps the wxObject into a class which does have the refcounting, so one stores a2dSmrtPtr< a2dRefCount >
    in the STL list instead, and all will be oke.

    \ingroup general
*/
class a2dRefCount
{

public:

    //! constructor
    a2dRefCount( wxObject* theObjectToWrap )
    {
        m_refcount = 0;
        m_object = theObjectToWrap;
    }

    //! destructor
    virtual ~a2dRefCount()
    {
        wxASSERT_MSG(  m_refcount == 0, wxT( "deleting a2dRefCount while referenced" ) );
        delete m_object;
    }

    wxObject* GetObj() { return m_object; }

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //! this is a list of all smart pointers pointing to this object
    a2dSmrtPtr<a2dRefCount> *m_ownerlist;
    //! this is needed inside the smart pointer template code
    typedef a2dSmrtPtr<a2dRefCount> TOwnerListClass;
    //! Make a Dump of the objects owner list to the Debug console
    /*! This function is usually called from the Debuggers watch window */
    void DumpOwners();
#endif

private:

    //! Call to have a new owner for this object
    /*! This function should only be called by wxlSmrtPtr

        \remark owning mean that the object calling this member needs to call Release at some time,
                in order to actually release/delete the object.

        \return The return value is the object itself, which is now owned on time extra.

        increment refcount by 1 ( use when adding a reference to this object)
    */
    virtual a2dRefCount* SmrtPtrOwn()
    {
        m_refcount++; return this;
    }

    //!To release the object, it is not longer owned by the calling object.
    /*! This function should only be called by wxlSmrtPtr
    */
    virtual bool SmrtPtrRelease()
    {
        m_refcount--;
        wxASSERT_MSG( m_refcount >= 0, wxT( "wxRefCount Own/Release not matched (extra Release calls)" ) );
        if ( m_refcount <= 0 )
        {
            delete this;
            return true;
        }
        return false;
    }

    friend class a2dSmrtPtrBase;

protected:

    //!how many references to this object do exist
    int m_refcount;

    //! the wxObject that is maintained using Smart pointer within here.
    wxObject* m_object;
};


//! list for a2dRefCount Smart pointers
/*!
    Makes it possible to store wxObject* in a STL lists by using smart pointers.
    Deletion of the wxObject will be automaticaly done via a2dRefCount class.

    \ingroup general
*/
class a2dRefCountList: public a2dSmrtPtrList< a2dRefCount >
{

public:

    //! constructor
    a2dRefCountList() {}

    //! destructor
    ~a2dRefCountList() {}

    //! convenient to push wxObject, instead of  a2dRefCount( theObjectToWrap )
    void push_backobj( wxObject* obj )
    {
        push_back( new a2dRefCount( obj ) );
    }

    //! convenient to push wxObject, instead of  a2dRefCount( theObjectToWrap )
    void push_frontobj( wxObject* obj )
    {
        push_front( new a2dRefCount( obj ) );
    }

};

//! smart pointer for a2dRefCount
typedef a2dSmrtPtr< a2dRefCount > a2dRefCountPtr;

//! (In) Visible property that can be added to Docview Objects.
/*!

    In general derived objects of this class are added as properties to a a2dObject.
    You can store/wrap almost any information you like in a property. The purpose is mainly to allow users
    of the library to attach extra information to an object, without the need to extend the object with C++ members.
    The properties are dynamic, and space needed for them is freed when the property is removed. This keeps classes small
    since all extra information needed for editing an object are whatever, can be stored via the property system,
    as soon as the action is finished all extra data can be removed again.

    There are many ways to use the properties. But the important thing is to understand that a2dNamedProperties instances are
    made unique in type of info and purpose by its a2dPropertyId. A specific derived a2dNamedProperty, containing some data,
    does have a a2dPropertyId set to it on creation. This id is like a unique application broad name for this a2dNamedProperty.
    The type of id can only be used in conbination with that type of a2dNamedProperty.
    Normally you add  specific a2dPropertyId instances to a a2dObject as static members, which defines which properties
    can be set to the object.

    class a2dNamedProperty_X has a one to relation to a class a2dPropertyId_Y.
    There can be many instances of a2dNamedProperty_X all with instantiated with different instances of that a2dPropertyId_Y.

    Properties are chained via its next member.

    see a2dObject

    \ingroup property

*/
class A2DGENERALDLLEXP a2dNamedProperty : public wxObject
{

#ifdef CLASS_MEM_MANAGEMENT

    //! memory manager for speed up to replace system calls allocation and deallocation
    static a2dMemManager sm_memManager;

public:

    //! overloaded operator new for this class and it all derived classes
    void* operator new( size_t bytes )
    {
        return sm_memManager.Allocate( bytes );
    }

    //! overloaded operator delete for this class and it all derived classes
    /*!
    This function doesn't free to OS-system memory block by pointer 'space'.
    It adds memory block by pointer 'space' to internal lists.
    It is for speed up.
    */
    void operator delete( void* space, size_t bytes )
    {
        sm_memManager.Deallocate( space, bytes );
    }

#endif //CLASS_MEM_MANAGEMENT

public:
    //! Default constructor
    a2dNamedProperty();

    //! Constructor giving an id
    a2dNamedProperty( const a2dPropertyId* id );

    //! Copy constructor
    a2dNamedProperty( const a2dNamedProperty& other );

    //! Virtual copy constructor
    a2dNamedProperty* Clone( a2dObject::CloneOptions options, a2dRefMap* refs = NULL ) const;

    //! Virtual assignment operator
    virtual void Assign( const a2dNamedProperty& other ) = 0;

    //! Destructor
    virtual ~a2dNamedProperty();

    //!general flag use at will.
    /*!
        \remark
        This flag should only be used for temporarly purposes.
        This object uses this flag too and you might run into problems if you use this flag.
        It's a good practice to set this flag if you need it and reset this flag to <code>false</code>
        if you don't need it anymore. Another possibility might be to add a new property to this object
        if you want to be on the secure side.

        \param check temporarely status information
    */
    inline void SetCheck( bool check ) { m_check = check; }

    //!general flag use at will.
    inline bool GetCheck() const {return m_check;}

    virtual bool NeedsSave() { return true; }

    //! Set this property to an object
    /*! The property "this" will be owned by the object */
    inline void SetToObject ( a2dObject* obj, a2dPropertyId::SetFlags setflags = a2dPropertyId::set_none ) { obj->SetProperty( this, setflags ); }

    //! Set this property to an object and clone it if neccessary
    /*! The property "this" will NOT be owned by the object. It will be cloned if neccessary */
    inline void SetToObjectClone( a2dObject* obj, a2dPropertyId::SetFlags setflags = a2dPropertyId::set_none ) const { obj->SetProperty( *this, setflags ); }

    //! Get the a2dPropertyId object identifying this property
    const a2dPropertyId* GetId() const { wxASSERT_MSG( m_id, wxT( "id not set in property" ) ); return m_id; }

    //! creates an a2dPropertyId for this property, if it is not available.
    virtual const a2dPropertyId* CreateId( a2dObject* parent, const wxString& name );

    //! Get the name of the a2dPropertyId object
    wxString GetName() const { return m_id->GetName(); }

    //! when a2dStringProperty, return its value else assert
    virtual wxString GetString() const;

    //! when a2dDoubleProperty, return its value else assert
    virtual double GetDouble() const;

    //! when a2dFloatProperty, return its value else assert
    virtual float GetFloat() const;

    //! when a2dBoolProperty, return its value else assert
    virtual bool GetBool() const;

    //! when a2dInt16Property, return its value else assert
    virtual wxInt16 GetInt16() const;

    //! when a2dUint16Property, return its value else assert
    virtual wxUint16 GetUint16() const;

    //! when a2dInt32Property, return its value else assert
    virtual wxInt32 GetInt32() const;

    //! when a2dUint32Property, return its value else assert
    virtual wxUint32 GetUint32() const;

    //! when a2dVoidPtrProperty, return its value else assert
    virtual void* GetVoidPtr() const;

    //! when a2dProperty, return its value else assert
    virtual a2dObject* GetRefObject() const;

    //! when a2dProperty, return its value else return NULL
    virtual a2dObject* GetRefObjectNA() const;

    //! Convert the property to a string, used to serialize class.
    //! Override to define it for your properties.
    virtual wxString StringRepresentation() const { return m_id->GetName(); }

    //! Convert the property value to a string, used to serialize class.
    //! Override to define it for your properties.
    virtual wxString StringValueRepresentation()  const { wxASSERT( 0 ); return wxT( "" ); }

    //! some derived properties can be visible or not and override this
    virtual bool GetVisible() const { return true; }
    //! some derived properties can be renderable or not and override this
    /*!
        some properties have a reference to a a2dCanvasObject, but only to store it,
        not to render it as part of the object.

        a2dPropertyId of a property normally decides if this specific property is to be rendered or not.
    */
    virtual bool GetCanRender() const { return false; }

    //! parse a double from the value string
    double ParseDouble( unsigned int& position );
    //! parse comma, whitespace and skip it form teh value string
    void SkipCommaSpaces( unsigned int& position );
    //! parse whitespace and skip it form teh value string
    void SkipSpaces( unsigned int& position );

    //! This is used to recursively walk through an object tree
    /*!
    */
    void Walker( wxObject* parent, a2dWalkerIOHandler& handler );
    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

#if wxART2D_USE_CVGIO
    virtual void Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite );
    virtual void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_CLASS( a2dNamedProperty )

protected:

	virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const = 0;

    //! The property id object identifying this property
    const a2dPropertyId* m_id;
    // the propeerty ids also have special access right sto properties
    friend class a2dPropertyId;

#ifdef _DEBUG
    int _debug_id;
#endif

public:
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    a2dNamedPropertyList* m_ownerlist;
#endif

private:

    //! used for linking multiple referenced objects when serializing
    bool m_check: 1;

    //!how many references to this object do exist
    int m_refcount;

    //! Call to have a new owner for this object
    /*! This function should only be called by a2dSmrtPtr

        \remark owning mean that the object calling this member needs to call Release at some time,
                in order to actually release/delete the object.

        \return The return value is the object itself, which is now owned on time extra.

        increment refcount by 1 ( use when adding a reference to this object)
    */
    a2dNamedProperty* SmrtPtrOwn() { m_refcount++; return this; }

    //!To release the object, it is not longer owned by the calling object.
    /*! This function should only be called by a2dSmrtPtr
    */
    bool SmrtPtrRelease()
    {
        m_refcount--;
        wxASSERT_MSG( m_refcount >= 0, wxT( "a2dNamedProperty Own/Release not matched (extra Release calls)" ) );
        if ( m_refcount <= 0 )
        {
            delete this;
            return true;
        }
        return false;
    }

private:
    friend class a2dSmrtPtrBase;
};

//! smart pointerv to a2dNamedProperty
typedef a2dSmrtPtr<a2dNamedProperty> a2dNamedPropertyPtr;

//! property to hold a bool type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dBoolProperty: public a2dNamedProperty
{
public:

    a2dBoolProperty();

    a2dBoolProperty( const a2dPropertyIdBool* id, bool deep = true );

    a2dBoolProperty( const a2dBoolProperty& other, a2dObject::CloneOptions options );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dBoolProperty();

    //! Construct a new property object from a string
    /*! Accepted values for true are:
        - "true"
        - "yes"
        - "y"
        - "1"
        - native yes, eg. "ja" in the Netherlands or Germany. Dependant on local settings (i18n).
        All other values are interpreted as false.
    */
    static a2dBoolProperty* CreatePropertyFromString( const a2dPropertyIdBool* id, const wxString& value );

    void SetValue( bool value );

    bool GetValue() const { return m_value; }

    virtual bool GetBool() const { return m_value; }

    bool* GetValuePtr() { return &m_value; }

    bool& GetValueRef() { return m_value; }

    const bool* GetValuePtr() const { return &m_value; }

    const bool& GetValueRef() const { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

    virtual const a2dPropertyId* CreateId( a2dObject* parent, const wxString& name );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dBoolProperty )

protected:
    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    bool m_value;
};

//! property to hold a wxString type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dStringProperty: public a2dNamedProperty
{
public:

    a2dStringProperty();

    a2dStringProperty( const a2dPropertyIdString* id, const wxString& value );

    a2dStringProperty( const a2dStringProperty& other, a2dObject::CloneOptions options );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dStringProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dStringProperty* CreatePropertyFromString( const a2dPropertyIdString* id, const wxString& value );

    void SetValue( const wxString& value );

    wxString& GetValue() { return m_value; }
    const wxString& GetValue() const { return m_value; }

    virtual wxString GetString() const { return m_value; }

    wxString* GetValuePtr() { return &m_value; }

    virtual wxString StringRepresentation()  const { return m_id->GetName() + m_value; };

    virtual wxString StringValueRepresentation() const { return m_value; };

    virtual const a2dPropertyId* CreateId( a2dObject* parent, const wxString& name );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dStringProperty )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxString m_value;
};

//! property to hold a void pointer type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dVoidPtrProperty: public a2dNamedProperty
{
public:

    a2dVoidPtrProperty();

    a2dVoidPtrProperty( const a2dPropertyIdVoidPtr* id, void* value );

    a2dVoidPtrProperty( const a2dVoidPtrProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dVoidPtrProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dVoidPtrProperty* CreatePropertyFromString( const a2dPropertyIdVoidPtr* id, const wxString& value );

    void SetValue( void* value );

    void* GetValue() const { return m_value; }

    virtual void* GetVoidPtr() const { return m_value; }

    void** GetValuePtr() { return &m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dVoidPtrProperty )

protected:
    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    void* m_value;
};

//! property to hold a wxObject variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dObjectProperty: public a2dNamedProperty
{
public:

    a2dObjectProperty();

    a2dObjectProperty( const a2dPropertyIdObject* id, const wxObject& value );

    a2dObjectProperty( const a2dObjectProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dObjectProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dObjectProperty* CreatePropertyFromString( const a2dPropertyIdObject* id, const wxString& value );

    void SetValue( const wxObject& value );

    const wxObject& GetValue() const { return m_value; }

    DECLARE_DYNAMIC_CLASS( a2dObjectProperty )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxObject m_value;
};

//! property to hold a a2dObjectPtr smart pointer type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dProperty: public a2dNamedProperty
{
public:

    a2dProperty();

    a2dProperty( const a2dPropertyIdRefObject* id, a2dObject* value );

    a2dProperty( const a2dProperty& other, a2dObject::CloneOptions options );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dProperty* CreatePropertyFromString( const a2dPropertyIdRefObject* id, const wxString& value );

    void SetValue( a2dObject* value );

    virtual a2dObject* GetRefObject() const { return m_value; }
    virtual a2dObject* GetRefObjectNA() const { return m_value; }

    a2dObject* GetValue() const { return m_value; }

    a2dObjectPtr* GetValuePtr() { return &m_value; }

    a2dObjectPtr& GetValueRef() { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dProperty )

protected:

	virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    a2dObjectPtr m_value;
};

//! property to hold a a2dObject pointer type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dAutoZeroProperty: public a2dNamedProperty
{
public:

    a2dAutoZeroProperty();

    a2dAutoZeroProperty( const a2dPropertyIdRefObjectAutoZero* id, a2dObject* value );

    a2dAutoZeroProperty( const a2dAutoZeroProperty& other, a2dObject::CloneOptions options );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dAutoZeroProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dAutoZeroProperty* CreatePropertyFromString( const a2dPropertyIdRefObjectAutoZero* id, const wxString& value );

    void SetValue( a2dObject* value );

    virtual a2dObject* GetRefObject() const { return m_value; }
    virtual a2dObject* GetRefObjectNA() const { return m_value; }

    a2dObject* GetValue() const { return m_value; }

    a2dObjectAutoZeroPtr* GetValuePtr() { return &m_value; }

    a2dObjectAutoZeroPtr& GetValueRef() { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dAutoZeroProperty )

protected:

	virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    a2dObjectAutoZeroPtr m_value;
};

//! property to hold a 2 byte integer type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dInt16Property: public a2dNamedProperty
{
public:

    a2dInt16Property();

    a2dInt16Property( const a2dPropertyIdInt16* id, wxInt16 value );

    a2dInt16Property( const a2dInt16Property& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dInt16Property();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dInt16Property* CreatePropertyFromString( const a2dPropertyIdInt16* id, const wxString& value );

    void SetValue( wxInt16 value );

    wxInt16 GetValue() const { return m_value; }

    virtual wxInt16 GetInt16() const { return m_value; }

    wxInt16* GetValuePtr() { return &m_value; }

    wxInt16& GetValueRef() { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dInt16Property )

protected:

	virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxInt16 m_value;
};

//! property to hold an unsigned 2 byte integer type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dUint16Property: public a2dNamedProperty
{
public:

    a2dUint16Property();

    a2dUint16Property( const a2dPropertyIdUint16* id, wxUint16 value );

    a2dUint16Property( const a2dUint16Property& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dUint16Property();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dUint16Property* CreatePropertyFromString( const a2dPropertyIdUint16* id, const wxString& value );

    void SetValue( wxUint16 value );

    wxUint16 GetValue() const { return m_value; }

    virtual wxUint16 GetUint16() const { return m_value; }

    wxUint16* GetValuePtr() { return &m_value; }

    wxUint16& GetValueRef() { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dUint16Property )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxUint16 m_value;
};

//! property to hold a 2 byte integer type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dInt32Property: public a2dNamedProperty
{
public:

    a2dInt32Property();

    a2dInt32Property( const a2dPropertyIdInt32* id, wxInt32 value );

    a2dInt32Property( const a2dInt32Property& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dInt32Property();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dInt32Property* CreatePropertyFromString( const a2dPropertyIdInt32* id, const wxString& value );

    void SetValue( wxInt32 value );

    wxInt32 GetValue() const { return m_value; }

    virtual wxInt32 GetInt32() const { return m_value; }

    wxInt32* GetValuePtr() { return &m_value; }

    wxInt32& GetValueRef() { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

    virtual const a2dPropertyId* CreateId( a2dObject* parent, const wxString& name );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dInt32Property )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxInt32 m_value;
};

//! property to hold an unsigned 4 byte integer type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dUint32Property: public a2dNamedProperty
{
public:

    a2dUint32Property();

    a2dUint32Property( const a2dPropertyIdUint32* id, wxUint32 value );

    a2dUint32Property( const a2dUint32Property& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dUint32Property();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dUint32Property* CreatePropertyFromString( const a2dPropertyIdUint32* id, const wxString& value );

    void SetValue( wxUint32 value );

    wxUint32 GetValue() const { return m_value; }

    virtual wxUint32 GetUint32() const { return m_value; }

    wxUint32* GetValuePtr() { return &m_value; }

    wxUint32& GetValueRef() { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dUint32Property )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxUint32 m_value;
};

//! property to hold a double type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dDoubleProperty: public a2dNamedProperty
{
public:

    a2dDoubleProperty();

    a2dDoubleProperty( const a2dPropertyIdDouble* id, double value );

    a2dDoubleProperty( const a2dDoubleProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dDoubleProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dDoubleProperty* CreatePropertyFromString( const a2dPropertyIdDouble* id, const wxString& value );

    void SetValue( double value );

    double GetValue() const { return m_value; }

    virtual double GetDouble() const { return m_value; }

    double* GetValuePtr() { return &m_value; }

    double& GetValueRef() { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

    virtual const a2dPropertyId* CreateId( a2dObject* parent, const wxString& name );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dDoubleProperty )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    double m_value;
};

//! property to hold a float type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dFloatProperty: public a2dNamedProperty
{
public:

    a2dFloatProperty();

    a2dFloatProperty( const a2dPropertyIdFloat* id, float value );

    a2dFloatProperty( const a2dFloatProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dFloatProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dFloatProperty* CreatePropertyFromString( const a2dPropertyIdFloat* id, const wxString& value );

    void SetValue( float value );

    float GetValue() const { return m_value; }

    virtual float GetFloat() const { return m_value; }

    virtual double GetDouble() const { return m_value; }

    float* GetValuePtr() { return &m_value; }

    float& GetValueRef() { return m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dFloatProperty )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    float m_value;
};

//! property to hold a wxArrayString type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dArrayStringProperty: public a2dNamedProperty
{
public:

    a2dArrayStringProperty();

    a2dArrayStringProperty( const a2dPropertyIdArrayString* id, const wxArrayString& value );

    a2dArrayStringProperty( const a2dPropertyIdArrayString* id, const wxString& value );

    a2dArrayStringProperty( const a2dArrayStringProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dArrayStringProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dArrayStringProperty* CreatePropertyFromString( const a2dPropertyIdArrayString* id, const wxString& value );

    void Add( const wxString& value );

    void SetValue( const wxArrayString& value );

    wxArrayString& GetValue() { return m_value; }
    const wxArrayString& GetValue() const { return m_value; }

    wxArrayString* GetValuePtr() { return &m_value; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dArrayStringProperty )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxArrayString m_value;
};

//! property to hold a FileName type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dFileNameProperty: public a2dNamedProperty
{
public:

    a2dFileNameProperty();

    a2dFileNameProperty( const a2dPropertyIdFileName* id, const wxFileName& filename );

    a2dFileNameProperty( const a2dFileNameProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dFileNameProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dFileNameProperty* CreatePropertyFromString( const a2dPropertyIdFileName* id, const wxString& value );

    void SetValue( const wxFileName& filenameObject );

    wxFileName GetValue() const { return m_filenameObject; }

    wxFileName* GetValuePtr() { return &m_filenameObject; }

    wxFileName& GetValueRef() { return m_filenameObject; }

    void SetFileName( const wxFileName& filenameObject );

    wxFileName GetFileName() { return m_filenameObject; }

    wxFileName& GetFileNameRef() { return m_filenameObject; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dFileNameProperty )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxFileName m_filenameObject;
};

//! property to hold a wxDateTime type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dDateTimeProperty: public a2dNamedProperty
{
public:

    a2dDateTimeProperty();

    a2dDateTimeProperty( const a2dPropertyIdDateTime* id, const wxDateTime& datetime );

    a2dDateTimeProperty( const a2dDateTimeProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dDateTimeProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dDateTimeProperty* CreatePropertyFromString( const a2dPropertyIdDateTime* id, const wxString& value );

    void SetValue( const wxDateTime& datetimeObject );

    wxDateTime GetValue() const { return m_datetimeObject; }

    wxDateTime* GetValuePtr() { return &m_datetimeObject; }

    wxDateTime& GetValueRef() { return m_datetimeObject; }

    void SetDateTime( const wxDateTime& datetimeObject );

    wxDateTime GetDateTime() { return m_datetimeObject; }

    wxDateTime& GetDateTimeRef() { return m_datetimeObject; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dDateTimeProperty )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxDateTime m_datetimeObject;
};

//! property to hold a Menu type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dMenuProperty: public a2dNamedProperty
{
public:

    a2dMenuProperty();

    a2dMenuProperty( const a2dPropertyIdMenu* id, wxMenu* menu );

    a2dMenuProperty( const a2dMenuProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dMenuProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dMenuProperty* CreatePropertyFromString( const a2dPropertyIdMenu* id, const wxString& value );

    void SetValue( wxMenu* menu );

    wxMenu* GetValue() const { return m_menu; }

    wxMenu** GetValuePtr() { return &m_menu; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dMenuProperty )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxMenu* m_menu;
};

//! property to hold a window type variable (for example wxTipWindow) to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dWindowProperty: public a2dNamedProperty
{
public:

    a2dWindowProperty();

    a2dWindowProperty( const a2dPropertyIdWindow* id, wxWindow* window );

    a2dWindowProperty( const a2dWindowProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dWindowProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dWindowProperty* CreatePropertyFromString( const a2dPropertyIdWindow* id, const wxString& value );

    void SetValue( wxWindow* window );

    wxWindow* GetValue() const { return m_window; }

    wxWindow** GetValuePtr() { return &m_window; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dWindowProperty )

protected:

	virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxWindow* m_window;
};

//! property to hold a tip window type variable to be associated with a a2dObject
/*!
    Value is stired as wxWindow*. GetValue() must be casted to wxTipWindow to get the tip window.

    \ingroup property
*/
class A2DGENERALDLLEXP a2dTipWindowProperty: public a2dWindowProperty
{
public:

    a2dTipWindowProperty();

    //! constructor
    /*!
        \param id id of property e.g. a2dCanvasObject::PROPID_TipWindow
        \param window tip window to show
        \param topstring string to show in tip window
        \param useObjRect if true, use the a2dCanvasObject its boundingbox to keep tip visible
    */
    a2dTipWindowProperty( const a2dPropertyIdWindow* id, wxTipWindow* window, const wxString& tipstring, bool useObjRect = true );

    //! constructor
    /*!
        \param id id of property e.g. a2dCanvasObject::PROPID_TipWindow
        \param window tip window to show
        \param topstring string to show in tip window
        \param rect use rect to keep tip visible
    */
    a2dTipWindowProperty( const a2dPropertyIdWindow* id, wxTipWindow* window, const wxString& tipstring, const wxRect& rect );

    a2dTipWindowProperty( const a2dTipWindowProperty* ori );

    a2dTipWindowProperty( const a2dTipWindowProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dTipWindowProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dTipWindowProperty* CreatePropertyFromString( const a2dPropertyIdWindow* id, const wxString& value );

    void SetString( const wxString& tipstring );

    const wxString& GetString( void )
    {
        return m_tipstring;
    }

    //! get rect which keep tip visible
    const wxRect& GetRect() const { return m_rectStay; }

    //! is the tip based on a2dCanvasObject boundingbox
    bool GetUseObjRect() const { return m_useObjRect; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dTipWindowProperty )

protected:

	virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxString m_tipstring;

    bool m_useObjRect;

    wxRect m_rectStay;
};

//! Path searching
/*!
    Path may contain ${aap} for environment variables and %{aap} for internal variables.
    See VarRef() for exact syntax.

    Internal variables come from a2dGeneralGlobals->GetVariablesHash()

    \ingroup general
*/
class A2DGENERALDLLEXP a2dPathList : public wxPathList
{
public:

    a2dPathList();

    ~a2dPathList();

    //! Find the first full path for which the file exists
    wxString FindValidPath ( const wxString& filename, bool reportError = true );

    //! Find the first full path for which the file exists; ensure it's an
    //! absolute path that gets returned.
    wxString FindAbsoluteValidPath( const wxString& filename );

    //! Get all search paths as one string.
    wxString GetAsString();

    bool ExpandPath( wxString& pathToExpand, wxPathFormat format = wxPATH_NATIVE );

private:

    bool Name();

    void IncC();

    // <aliasref>           ::= ('%') { <name> <blank> | ('{') <name> ('}') }
    // Get Environment string
    //  <envstring>     ::= ('$') { <name> <blank> | ('{') <name> ('}') }
    bool VarRef( wxChar type );

    wxChar a;
    wxString m_path;
    const wxChar* m_c;
    wxString m_b;
    wxString m_error_mes;
    wxString m_varref;
};


class A2DGENERALDLLEXP a2dPathListProperty;
#if defined(WXDOCVIEW_USINGDLL)
template class A2DGENERALDLLEXP a2dPropertyIdTyped<a2dPathList,    a2dPathListProperty>;
#endif
typedef a2dPropertyIdTyped<a2dPathList,    a2dPathListProperty>    a2dPropertyIdPathList;

//! property to hold a FileName type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dPathListProperty: public a2dNamedProperty
{
public:

    a2dPathListProperty();

    a2dPathListProperty( const a2dPropertyIdPathList* id, const a2dPathList& pathlist );

    a2dPathListProperty( const a2dPathListProperty* ori );

    a2dPathListProperty( const a2dPathListProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dPathListProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dPathListProperty* CreatePropertyFromString( const a2dPropertyIdPathList* id, const wxString& value );

    void SetValue( const a2dPathList& pathlistObject );

    a2dPathList GetValue() const { return m_pathlistObject; }

    a2dPathList* GetValuePtr() { return &m_pathlistObject; }

    a2dPathList& GetValueRef() { return m_pathlistObject; }

    void SetFileName( const a2dPathList& pathlistObject );

    a2dPathList GetFileName() { return m_pathlistObject; }

    a2dPathList& GetFileNameRef() { return m_pathlistObject; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dPathListProperty )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    a2dPathList m_pathlistObject;
};


extern A2DGENERALDLLEXP wxColour ParseSvgColour( wxString value );

//! property to hold a FileName type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dColourProperty: public a2dNamedProperty
{
public:

    a2dColourProperty();

    a2dColourProperty( const a2dPropertyIdColour* id, const wxColour& colour );

    a2dColourProperty( const a2dColourProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dColourProperty();

    //! Construct a new property object from a string
    /*! If this is not appropriate, this may return NULL.
        Accepted colours are:
        - hex numbers: "#a1b2c3" or just "a1b2c3".
        - decimal numbers: "rgb(10,20,30)", "10,20,30" or "10 20 30".
        - colour names: "red", "pink", "forest green".
    */
    static a2dColourProperty* CreatePropertyFromString( const a2dPropertyIdColour* id, const wxString& value );

    void SetValue( const wxColour& colour );

    wxColour GetValue() const { return m_colour; }

    wxColour* GetValuePtr() { return &m_colour; }

    wxColour& GetValueRef() { return m_colour; }

    void SetColour( const wxColour& colour );

    wxColour GetColour() { return m_colour; }

    wxColour& GetColourRef() { return m_colour; }

    virtual wxString StringRepresentation() const;

    virtual wxString StringValueRepresentation() const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dColourProperty )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxColour m_colour;
};


//! property to hold a wxURI type variable to be associated with a a2dObject
/*!
    \ingroup property
*/
class A2DGENERALDLLEXP a2dUriProperty: public a2dNamedProperty
{
public:

    a2dUriProperty();

    a2dUriProperty( const a2dPropertyIdUri* id, const wxURI& uri );

    a2dUriProperty( const a2dUriProperty& other );

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dUriProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dUriProperty* CreatePropertyFromString( const a2dPropertyIdUri* id, const wxString& value );

    void SetValue( const wxURI& uri );

    wxURI GetValue() const { return m_uri; }

    wxURI& GetValueRef() { return m_uri; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dUriProperty )

protected:

    virtual a2dNamedProperty* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    wxURI m_uri;
};



//! define a NON a2dObjectList
/*!
    Even if a wxDocviewObject has no properties, still its Length can be asked for.
*/
A2DGENERALDLLEXP_DATA( extern a2dObjectList* ) wxNullRefObjectList;

#include <wx/listimpl.cpp>

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DGENERALDLLEXP a2dSmrtPtr<class a2dObject>;
template class A2DGENERALDLLEXP std::allocator<class a2dSmrtPtr<class a2dObject> >;
template class A2DGENERALDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dObject>, std::allocator<class a2dSmrtPtr<class a2dObject> > >::_Node >;
template class A2DGENERALDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dObject>, std::allocator<class a2dSmrtPtr<class a2dObject> > >::_Nodeptr >;
template class A2DGENERALDLLEXP std::list<class a2dSmrtPtr<class a2dObject> >;
template class A2DGENERALDLLEXP a2dlist<class a2dSmrtPtr<class a2dObject> >;
template class A2DGENERALDLLEXP a2dSmrtPtrList<class a2dObject>;
#endif

//! list of a2dObject's
/*!
    \ingroup general
*/
class A2DGENERALDLLEXP a2dObjectList: public a2dSmrtPtrList<a2dObject>
{
public:
    a2dObjectList();

    ~a2dObjectList();

    //! all with reference count > 1 are cloned.
    void MakeUnique();

    //!this only copies pointer stored in the list, if you want the object itself
    //!copied also, use Clone
    a2dObjectList& operator=( a2dObjectList& other );

    //!Clone everything ( Clones objects also)  in a new created list
    a2dObjectList* Clone( a2dObject::CloneOptions options, a2dRefMap* refs = NULL ) const;

    //!release only objects with the given classname and has property named propertyname and object name
    //! empty string means ignore.
    bool Release( const wxString& classname = wxT( "" ), bool all = false );

    //!Find object within the given property name and classname.
    /*!
        \param classname If classname is empty it collects all objects else only object with this class name.
        \param name If a name is set the object needs to have this name
    */
    a2dObject* Find( const wxString& classname = wxT( "" ),  const wxString& name = wxT( "" ) );

    //!Copy objects fitting the given filter to the total list.
    /*!
    \param total list of object found (may already contain elements found in earlier call)
    \param propertyNameFilter If property name is set the object needs to have a name which matches propertyname.
    \param classname  If type is empty it collects all objects else only object with this class name.

    \return number of objects found
    */
    int CollectObjects( a2dObjectList* total, const wxString& propertyNameFilter = wxT( "" ), const wxString& classname = wxT( "" ) );

    //DECLARE_CLASS(a2dObjectList)

};

#include <wx/hashmap.h>

//! wxString keys plus a2dObject pointers
/*!
    This holds internal variables.
*/
class A2DGENERALDLLEXP a2dVariablesHashBase;
class A2DGENERALDLLEXP a2dVariablesHashBase_wxImplementation_HashTable;
class A2DGENERALDLLEXP a2dVariablesHashBase_wxImplementation_KeyEx;
WX_DECLARE_STRING_HASH_MAP_WITH_DECL( a2dNamedProperty*, a2dVariablesHashBase, class A2DGENERALDLLEXP );


//! Holds internal variables to be used whereever needed.
/*!
    The variables are stored as wxString keys plus a2dNamedProperty in a hash table.

    Understand that a variable can hold any type of property and therefore any type of information can be stored.
    How that information is used in the commands within a2dCommandProcessor depends on the commands you implement.
    e.g. You can have C++ command functions which return a a2dObject* and this you can add a variable to the variable hash.
    Next that variable can be used to extract information using other commands again.

    \ingroup general property

*/
class A2DGENERALDLLEXP a2dVariablesHash : private a2dVariablesHashBase
{

public:

    //! constructor
    a2dVariablesHash();

    //! copy constructor
    a2dVariablesHash( const a2dVariablesHash& other );

    //! assignment operator
    void operator = ( const a2dVariablesHash& other );

    //! destructor
    ~a2dVariablesHash();

    //! set a new or replace an existing variable of arbitrary type
    /*!
        \remark The given property object is owned and deleted by the variable hash.
        \param variableName variable to add or replace
        \param property new property object for the variable ( id is usually wxPropertyIdXXX::GetDummy )

        \return true if variable already did exist.
    */
    bool SetVariable( const wxString& variableName, a2dNamedProperty* property );

    //! set a new or replace an existing a2dObject variable
    /*!
        \param variableName variable to add or replace
        \param value new value of the variable

        \return true if variable already did exist.
    */
    bool SetVariableRefObject( const wxString& variableName, a2dObject* value );

    //! set a new or replace an existing wxString variable
    /*!
        \param variableName variable to add or replace
        \param value new value of the variable

        \return true if variable already did exist.
    */
    bool SetVariableString( const wxString& variableName, const wxString& value );

    //! set a new or replace an existing double variable
    /*!
        \param variableName variable to add or replace
        \param value new value of the variable

        \return true if variable already did exist.
    */
    bool SetVariableDouble( const wxString& variableName, double value );

    //! set a new or replace an existing int variable
    /*!
        \param variableName variable to add or replace
        \param value new value of the variable

        \return true if variable already did exist.
    */
    bool SetVariableInt( const wxString& variableName, int value );

    //! get an existing variable of unknown type (not cloned)
    /*!
        \param variableName variable to search
        \return NULL if variable does not exist, else the property found.
    */
    const a2dNamedProperty* GetVariable( const wxString& variableName );

    //! get an existing variable of unknown type (cloned)
    /*!
        \param variableName variable to search
        \return NULL if variable does not exist, else the property found.
    */
    a2dNamedProperty* GetVariableClone( const wxString& variableName );

    //! get an existing a2dObject variable
    /*!
        \param variableName variable to search
        \return NULL if variable does not exist, else the property found.
    */
    a2dObject* GetVariableRefObject( const wxString& variableName );

    //! get an existing wxString variable
    /*!
        \param variableName variable to search
        \return NULL if variable does not exist, else the property found.
    */
    wxString* GetVariableString( const wxString& variableName );
};

//! Holds arrayed variables, e.g. parameter lists in a command processor
/*!
    The variables are stored as a2dNamedProperty in an integer indexed array.

    Understand that a variable can hold any type of property and therefore any type of information can be stored.
    How that information is used in the commands within a2dCommandProcessor depends on the commands you implement.
    e.g. You can have C++ command functions which return a a2dObject* and this you can add a variable to the variable hash.
    Next that variable can be used to extract information using other commands again.

    \ingroup general property
*/
class A2DGENERALDLLEXP a2dVariablesArray
{

public:

    //! constructor
    a2dVariablesArray();

    //! destructor
    ~a2dVariablesArray();

    //! delete all variables in the array
    void DeleteAll();

    //! add a new variable of arbitrary type
    /*!
        \param property new variable to add ( id is usually wxPropertyIdXXX::GetDummy )
    */
    void AppendVariable( a2dNamedProperty* property );

    //! add a new a2dObject variable
    /*!
        \param value new value of the variable
    */
    void AppendVariableRefObject( a2dObject* value );

    //! add a new wxString variable
    /*!
        \param value new value of the variable
    */
    void AppendVariableString( const wxString& value );

    //! add a new double variable
    /*!
        \param value new value of the variable
    */
    void AppendVariableDouble( double value );

    //! add a new int variable
    /*!
        \param value new value of the variable
    */
    void AppendVariableInt( int value );

    //! get an existing variable of unknown type (not cloned)
    /*!
        \param index of the variable (must be valid!)
        \return the property found.
    */
    const a2dNamedProperty& GetVariable( int index );

    //! get an existing variable of unknown type (cloned)
    /*!
        \param index of the variable (must be valid!)
        \return the property found.
    */
    a2dNamedProperty* GetVariableClone( int index );

    //! get an existing a2dObject variable
    /*!
        \param index of the variable (must be valid!)
        \return the refobject of the property found.
    */
    a2dObject* GetVariableRefObject( int index );

    //! get an existing wxString variable
    /*!
        \param index of the variable (must be valid!)
        \return the string value of the property found.
    */
    wxString GetVariableString( int index );

    //! get an existing variable of any type as String
    /*!
        \param index of the variable (must be valid!)
        \return the string representation of the property found
    */
    wxString GetVariableAsString( int index );

    //! get the number of elements in the array
    int GetCount() { return m_nelements; }

protected:
    //! set the minimum size for the array
    void SetMinSize( int size );
    //! this is the array of properties
    a2dNamedProperty** m_properties;
    //! number of used elements in m_properties
    int m_nelements;
    //! number of elements in m_properties
    int m_melements;
};

// type defs to work both stream types within wxDocview
#if wxUSE_STD_IOSTREAM

#if wxUSE_WSTREAM //!wxCHECK_VERSION(2,9,0)
//! string input stream based on STD
typedef wxSTD wistringstream a2dDocumentStringInputStream;
//! string output stream based on STD
typedef wxSTD wostringstream a2dDocumentStringOutputStream;
//! input stream based on STD
typedef wxSTD wistream a2dDocumentInputStream;
//! output stream based on STD
typedef wxSTD wostream a2dDocumentOutputStream;
//! input stream based on STD
typedef wxSTD wifstream a2dDocumentFileInputStream;
//! output stream based on STD
typedef wxSTD wofstream a2dDocumentFileOutputStream;
#else
//! string input stream based on STD
typedef wxSTD istringstream a2dDocumentStringInputStream;
//! string output stream based on STD
typedef wxSTD ostringstream a2dDocumentStringOutputStream;
//! input stream based on STD
typedef wxSTD istream a2dDocumentInputStream;
//! output stream based on STD
typedef wxSTD ostream a2dDocumentOutputStream;
//! input stream based on STD
typedef wxSTD ifstream a2dDocumentFileInputStream;
//! output stream based on STD
typedef wxSTD ofstream a2dDocumentFileOutputStream;
#endif

#else
//! string input stream based wxStreams
typedef wxStringInputStream a2dDocumentStringInputStream;
//! string output stream based wxStreams
typedef wxStringOutputStream a2dDocumentStringOutputStream;
//! input stream based wxStreams
typedef wxInputStream a2dDocumentInputStream;
//! output stream based wxStreams
typedef wxOutputStream a2dDocumentOutputStream;
#endif

//! class to map references to objects stored in XML, in order to make the connection later on.
class A2DGENERALDLLEXP a2dRefMap : public a2dObject
{

public:

    //! constructor
    a2dRefMap();

    //! destructor
    ~a2dRefMap();

    //!initialize mapping tables
    /*!
    */
    virtual void Initialize();

    //! This is used to find multiple referenced objects by id
    a2dHashMapIntToObject& GetObjectHashMap() { return m_objecthashmap; }

    //! has to map symbolic names to real classnames.
    a2dHashMapCreateObject& GetObjectCreateHashMap() { return m_objectCreate; }

    //! list of not resolved objects
    a2dResolveIdList& GetResolveIdList() { return m_toResolve; }

    //! try to resolve the object that is in storedHere when it is a reference.
    /*! If not succes, add it to m_toResolve.
        \param storedHere pointer to the smart pointer, that is to be set
        \param id object id of the referenced object. if "" SetLastObjectLoadedId is used.
    */
    bool ResolveOrAdd( a2dSmrtPtr<a2dObject>* storedHere, const wxString& id = wxT( "" ) );

    //! try to resolve the object that is in storedHere when it is a reference.
    bool ResolveOrAdd( a2dAutoZeroPtr<a2dObject>* storedHere, const wxString& id = wxT( "" ) );

    //! try to resolve an object referenced by obj using the LinkReference function
    //* If not succes, add it to m_toResolve. */
    bool ResolveOrAddLink( a2dObject* obj, const wxString& id = wxT( "" ) );

    //! call the objects LinkReference function (ptr=0) to inform it that links are done
    void ResolveInform( a2dObject* obj );

    //! set by RefObject when finding a reference id during loading.
    void SetLastObjectLoadedId( const wxString& lastid );

    //! link references to their destination
    /*!
        Override this to define input format specific needs for linking references in a certain format.
        In the default situation the parser uses m_objecthashmap to store object to which references can be made,
        while m_lastObjectId is used to store unresolved references.
        Here those too are resolved.

        In the CVG format one can store multiple refrences to one and the same object.
        The object that is referenced is only written once, for the other references only
        the object with refid or specific id attributes are written to the CVG file.
        When loading a file in CVG, such references are resolved and removed from the document.
        So in the end the old reference is restored. When loading an object, it is created, but its contents
        will be empty, after resolving, this one will be replaced by the real referenced object.
        The reason behind all this, is that the actual object might need to be read yet, when a reference
        is encountered in the CVG file, and therefore the link can not be directly created.
        In other formats references are used in a simular manner, and to resolve them the same mechanism
        is used.

        Some input/ouput formats are organized in a list of grouped objects, the hierarchy is created
        after the whole file is read into memory.
        All non referenced objects are direct childs of the rootobject.
        In the GDSII format these are called top structures.

        \return true if all references were resolved.
    */
    virtual bool LinkReferences( bool ignoreNonResolved = false );

    //! remove given reference if from reference map.
    bool RemoveReference( const wxString& idToRemove );

    //! get last error encountered
    a2dError GetLastError() const { return m_lastError; }

    //! set last error encountered
    void SetLastError( a2dError error ) { m_lastError = error; }

protected:

    //! This is used to find multiple referenced objects by id
    a2dHashMapIntToObject m_objecthashmap;

    //! This is used to find a classname using a symbolic name.
    a2dHashMapCreateObject m_objectCreate;

    //! refernces to object which have not bin resolved yet ( e.g not read yet, but already referenced )
    a2dResolveIdList m_toResolve;

    wxString m_lastObjectId;

    //! set to last error found when parsing
    a2dError m_lastError;

private:

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    virtual a2dObject* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

};


//! a2dIOHandler takes care of loading and/or saving data.
/*!
    Classes to read data from a file into a specific a2dDocument, are derived from this class.
    Classes to write data from a specific a2dDocument to a file, are derived from this class.

    This class provides a way to add parsers and writers to a specific type of document.
    It will not become part of that document class itself, but still fills it with information.
    It acts like a plugin in that sence.
    The a2dDocumentTemplate class is used to connect a a2dDocumentIOhandler to a document.
    It tells for which files ( extensions ), this handler should be used.

    If a a2dDocument has a native data format, which is integrated into the class itself,
    one should make a sort of dummy a2dIOHandler, which internal calls the read/write
    function on the a2dDocument.
    Or you can override a2dDocument::SaveObject() a2dDocument::LoadObject() to make it work
    the way you want.

    \ingroup docview
    \ingroup fileio

*/
class A2DGENERALDLLEXP a2dIOHandler : public a2dRefMap
{

public:

    //! constructor
    a2dIOHandler();

    //! destructor
    ~a2dIOHandler();

    //! Creates an specific object by name.
    /*!
        A hash table has a list of names and its coresponding classname,
        from which the object is created using wxWidgets wxCreateDynamicObject().
        In order to be more flexible in future classname changes,
        the hash of symbolic names is in between.
    */
    virtual wxObject* CreateObject( const wxString& symbolicName );

private:

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    virtual a2dObject* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

    a2dIOHandler( a2dIOHandler& other );

};

//! using a file stream for input, stream the file into a a2dDocument or other object
/*!
    Base class used to model loading data from a stream into an object, which can be a a2dDocument but also any other.
    What can be read into the object, depends on the implementation for which the derived IOHandler was written.

    \ingroup fileio
*/
class A2DGENERALDLLEXP a2dIOHandlerStrIn : public a2dIOHandler
{

public:

    //! constructor
    a2dIOHandlerStrIn( wxMBConv& conv = wxConvUTF8 );

    //! destructor
    ~a2dIOHandlerStrIn();

    //! Inits the handler for reading
    /*!
    */
    virtual void InitializeLoad();

    //! Reset the handler after loading
    virtual void ResetLoad();

    //! Should return true if the handler can read from the stream.
    /*!
        A typical implementation is to read the first part of the file, to see if the
        contents is right for this handler.
        Then it resets the file pointer to the beginning again.

        In general a handler is written with a certain type of document in mind, since that is where the information will be stored.
        The document view framework, uses document templates to get to input handlers.
        The last 2 parameters can be checked if set.
        Use SetDocumentClassInfo() to set the right classinfo, in case of a derived document object,
        which still can be read with the this handler.
        Like:
        \code
        a2dIOHandlerKeyIn* keyhin = new a2dIOHandlerKeyIn();
        keyhin->SetDocumentClassInfo( &MyDocument::ms_classInfo );
        \endcode


        \param stream the open stream which its header can be tested
        \param obj to check and load the data into, needs to be casted to the right one internal.
        \param docClassInfo to check if the inout handler is indeed able to read document of m_docClassInfo.

    */
    virtual bool CanLoad( a2dDocumentInputStream& stream, const wxObject* obj = NULL, wxClassInfo* docClassInfo = NULL ) = 0;

    //! override to read the stream and store (part of) the contents in to a specific a2dDocument or othere object.
    /*!
        Understand that a a2dDocument itself is useless for storing data,
        so it needs to be casted internal.

        \param stream the open stream to load from
        \param obj to load the data into, needs to be casted to the right one internal.
    */
    virtual bool Load( a2dDocumentInputStream& stream , wxObject* obj ) = 0;

    //! class info for the (document) class this handler reads data into.
    wxClassInfo* GetDocumentClassInfo() const { return m_docClassInfo; }

    //! class info for the (document) class this handler reads data into.
    void SetDocumentClassInfo( wxClassInfo* docClassInfo ) { m_docClassInfo = docClassInfo; }

    void SetFileName( const wxFileName& filename ) { m_filename = filename; }

    const wxFileName& GetFileName() const { return m_filename; }

protected:

    // if set, can be used to find path to load/save images etc.
    wxFileName m_filename;

    //! unicode conversion
    wxMBConv& m_conv;

    //! define the line end mode for textual files
    void SetMode( wxEOL mode = wxEOL_NATIVE );
    //! get line end mode for textual files
    wxEOL GetMode() { return m_mode; }

    //! return a character from the stream without removing it, i.e. it will
    //! still be returned by the next call to GetC()
    char Peek();

    //! return one character from the stream, blocking until it appears if
    //! necessary
    char GetC();

    //! set stream at a position
    a2dIOHandlerStrIn& SeekI( wxFileOffset pos );

    // is the stream at EOF?
    bool Eof() const;

    // is the stream OK
    bool IsOk() const;

    // read at most the given number of bytes from the stream
    //size_t Read( wxChar* buffer, size_t size);

//#if wxUSE_UNICODE
    size_t Read( char* buffer, size_t size );
//#endif

    //! file or other string stream containing the format to parse.
    a2dDocumentInputStream* m_streami;

    //! Run-time class information that allows document instances to be constructed dynamically.
    wxClassInfo*      m_docClassInfo;

    //! end of line mode
    wxEOL m_mode;
};

//! using a file stream for output, stream a a2dDocument or other wxObject into a stream.
/*!
    Base class used to model saving data to a stream from an object, which can be a a2dDocument but also any other.
    What can be saved into the stream, depends on the implementation for which the derived IOHandler was written.

    \ingroup fileio

*/
class A2DGENERALDLLEXP a2dIOHandlerStrOut : public a2dIOHandler
{

public:

    //! constructor
    a2dIOHandlerStrOut( wxMBConv& conv = wxConvUTF8 );

    //! destructor
    ~a2dIOHandlerStrOut();

    //! Inits the handler for writing
    virtual void InitializeSave();

    //! Reset the object after saving
    virtual void ResetSave();

    //! Should return true if the handler can write this document to a stream.
    /*!
        Mostly implemented by simply returning true if the handler can indeed do this job.
        Understand that a a2dDocument itself is useless for storing data,
        so it needs to be casted internal to a specific a2dDocument.

        \param obj pointer to object to save, normally a document.
    */
    virtual bool CanSave( const wxObject* obj = NULL ) = 0;

    //! Override to write to the stream and store (part of) of the document contents in the stream.
    /*!
        Understand that a a2dDocument itself is useless for storing data,
        so it needs to be casted internal to a specific a2dDocument.

        \param stream the open stream to which the object needs to be saved
        \param obj object to save into stream, needs to be casted to the right one internal.
    */
    virtual bool Save( a2dDocumentOutputStream& stream, const wxObject* obj ) = 0;

    void SetFileName( const wxFileName& filename ) { m_filename = filename; }

    const wxFileName& GetFileName() const { return m_filename; }

    //! set precision of double/float numbers in output files, when conversion to string is needed.
    //! use at will, for instance matrix calculation often unwise to truncate numbers to much,
    //! while vertixes sometimes must be truncated in certain formats.
    void SetPrecision( int precision = -1 ) { m_precision = precision; }

protected:

    // if set, can be used to find path to load/save images etc.
    wxFileName m_filename;

    //! define the line end mode for textual files
    void SetMode( wxEOL mode = wxEOL_NATIVE );
    //! get line end mode for textual files
    wxEOL GetMode() { return m_mode; }

    //! write a 32 bits integer
    void Write32( wxUint32 i );
    //! write a 16 bits integer
    void Write16( wxUint16 i );
    //! write a 8 bits integer
    void Write8( wxUint8 i );
    //! write a double
    virtual void WriteDouble( double d );
    //! write a string
    virtual void WriteString( const wxString& string );

    //! is the stream Oke to write
    bool IsOk() const;

    //! write on character
    a2dIOHandlerStrOut& PutChar( wxChar c );

    int m_precision;

protected:

    //! unicode conversion
    wxMBConv& m_conv;

    //! file or other string stream containing the format to output to.
    a2dDocumentOutputStream* m_streamo;

    //! end of line mode
    wxEOL m_mode;

private:

    a2dIOHandlerStrOut( a2dIOHandlerStrOut& other );

};

/*! \addtogroup  docalgo
*  @{
*/


/*! a2dDocument object traversing via design visitor pattern */
typedef const long a2dWalkEvent;

//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dObjectStart;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dObjectEnd;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dPropertyStart;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dPropertyEnd;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dNamedPropertyListStart;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dNamedPropertyListEnd;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dNamedPropertyStart;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dNamedPropertyEnd;

//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dCanvasObjectStart;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dCanvasObjectProperties;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dCanvasObjectPreChild;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dCanvasObjectPostChild;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dCanvasObjectEnd;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dDerivedCanvasObjectStart;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dDerivedCanvasObjectEnd;

//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dDrawingStart;
//! id for a2dWalkEvent issued from within a2dWalkerIOHandler
A2DGENERALDLLEXP extern a2dWalkEvent a2dWalker_a2dDrawingEnd;

//!  @}  docalgo


//! a functions to store in a hash table
typedef bool ( *a2dObjectIOFn )( wxObject* parent, wxObject* object, a2dWalkEvent );

class A2DGENERALDLLEXP a2dIOHandlerCoupleHash;
class A2DGENERALDLLEXP a2dIOHandlerCoupleHash_wxImplementation_HashTable;
class A2DGENERALDLLEXP a2dIOHandlerCoupleHash_wxImplementation_KeyEx;

#include <wx/hashmap.h>

WX_DECLARE_STRING_HASH_MAP( a2dObjectIOFn*, a2dIOHandlerCoupleHash );

//! Io handler to iterate through a a2dDocument.
/*!
    It can be used in combination with a file for reading or writing.

    Calling a2dObject::Walker( a2dWalkerIOHandler& handler ),
    will have the a2dObject start calling the function WalkTask()
    at predifined moments. The a2dCanvasObject will also recursively
    go into its children. While a2dObject goes into its properties.
    In a derived class you can define what needs to happen in each WalkTask().

    This makes it possible to write algorithm on a a2dDocument, without
    extending the a2dObject derived classes itself.
    For instance calculating the area occupied by all a2dCanvasObject, can be implemented
    with this class.

    The default WalkTask() function, checks if a function is registrated for the specific class,
    and if so calls that function.

    \ingroup docalgo
    \ingroup fileio
*/
class A2DGENERALDLLEXP a2dWalkerIOHandler : public a2dIOHandler
{

    DECLARE_CLASS( a2dWalkerIOHandler )

public:

    //! constructor
    a2dWalkerIOHandler();

    //! destructor
    ~a2dWalkerIOHandler();

    //! to initialize a walker class, default does nothing.
    virtual void Initialize();

    //! get this when an error occured.
    wxString GetErrorString() { return m_error; }

    //! to further recursion, e.g. in a2dObject::Walker()
    void SetStopWalking( bool stopWalking ) { m_stopWalking = stopWalking; }

    //! check for stop
    bool GetStopWalking() { return m_stopWalking; }

    //! Increment recursion depth
    void IncCurrentDepth() { m_currentDepth++; }

    //! Decrement recursion depth
    void DecCurrentDepth() { m_currentDepth--; }

    //! What is the current recursion depth
    int GetCurrentDepth() { return m_currentDepth; }

    //! set if walker needs to go into recursive objects first.
    void SetDepthFirst( bool depthFirst ) { m_depthFirst = depthFirst; }

    //! What is the current recursion depth
    bool GetDepthFirst() { return m_depthFirst; }

    //! set if walker needs to check objects for a2dObject::m_check set, before walking into it
    void SetUseCheck( bool useCheck ) { m_useCheck = useCheck; }

    //! see SetUseCheck
    bool GetUseCheck() { return m_useCheck; }

    //! to register a function to handle a class
    void Register( const wxString& classname, a2dObjectIOFn* IOfunc ) { m_register[ classname ] = IOfunc; }

    //! to Un-register a function to handle a class
    void Unregister( const wxString& classname )  { m_register.erase( classname ); }

    //! called from within a2dObject's and derived classes
    /*!
        The object which are walked over/into, can announce specific events within that object to
        the a2dWalkerIOHandler at hand. The a2dWalkerIOHandler can react to those events.
    */
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //!if set by walker in WalkTask(), it can be used in child object to get to the parent via the
    //! a2dWalkerIOHandler, which is the argument to a child member function.
    wxObject* GetParent() { return m_currentParent; }

    bool GetSkipNotRenderedInDrawing() const { return m_skipNotRenderedInDrawing; }

    void SetSkipNotRenderedInDrawing( bool skipNotRenderedInDrawing ) { m_skipNotRenderedInDrawing = skipNotRenderedInDrawing; }

    //! depending upon derived class implementation.
    bool GetResult() { return m_result; }

protected:

    //! result of walk, set false at start, use at will.
    bool m_result;

    //! parent object of current object
    wxObject* m_currentParent;

    //! to store errors
    wxString m_error;

    //! quit the iteration
    bool m_stopWalking;

    //! contour to be used by algorithm, to decide on how deep recursing is to be.
    int m_currentDepth;

    //! if true objects with nested objects should be walked first.
    //! it is the object to test this flag and if set, search for nested objects to walk first.
    bool m_depthFirst;

    //! if set a2dObject are only walked if not m_check is set
    bool m_useCheck;

    //! if a drawing contains references to objects in other drawings, which shuld not be included in the walk.
    //! this flag can be used to check it inside that kind of objects to stop walking.
    bool m_skipNotRenderedInDrawing;

    //! coupling function to event and classname
    a2dIOHandlerCoupleHash m_register;
};

//! Renumber Id's of objects
/*!

    \ingroup docalgo
*/
class A2DGENERALDLLEXP a2dWalker_Renumber: public a2dWalkerIOHandler
{
public:

    a2dWalker_Renumber();

    ~a2dWalker_Renumber();

    //! object to start the algorithm
    /*!
        all object nested down here will be renumbered.
        return true
    */
    bool Start( a2dObject* object );

    //! called from within a2dCanvasDocument
    bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );
};


//! 3-digit hex to wxColour
extern wxColour A2DGENERALDLLEXP HexToColour( const wxString& hex );

//! RGB to 3-digit hex
extern wxString A2DGENERALDLLEXP ColourToHex( const wxColour& colour );

/*! This object helps fixing initialization order of static objects and the wxWidgets
internal memory critical sections. If you have  static object that owns dynamic memory,
but doesn't create it in its constructor, just create such an object in the objects
constructor of before the definition of the static object to fix crashes in memory.cpp.

This could also be a function as it doesn't have data, but you might want to use it as
static object, so it is a class.

    \ingroup general
*/
class A2DGENERALDLLEXP a2dMemoryCriticalSectionHelper
{
public:
    a2dMemoryCriticalSectionHelper()
    {
        // Just create some memory, so that the MemoryCriticalSection in GetMemLocker in memory.cpp is initialized
        int* dummy = new int;
        delete dummy;
    }
};

#include <wx/sstream.h>

//#if defined(WXDOCVIEW_USINGDLL)
#include "wx/general/id.inl"
//#endif


//! defines what to print
/*!
*/
typedef wxUint16 a2dPrintWhat;

/*
// Helper class that changes LC_NUMERIC facet of the global locale in its ctor
// to "C" locale and restores it in its dtor later.
class a2dCNumericLocaleSetter
{
public:
    a2dCNumericLocaleSetter()
        : m_oldLocale(wxStrdupA(setlocale(LC_NUMERIC, NULL)))
    {
        if ( !wxSetlocale(LC_NUMERIC, "C") )
        {
            // Setting locale to "C" should really always work.
            wxFAIL_MSG( wxS("Couldn't set LC_NUMERIC to \"C\"") );
        }
    }

    ~a2dCNumericLocaleSetter()
    {
        wxSetlocale(LC_NUMERIC, m_oldLocale);
        free(m_oldLocale);
    }

private:
    char * const m_oldLocale;

    wxDECLARE_NO_COPY_CLASS(a2dCNumericLocaleSetter);
};
*/


#endif

