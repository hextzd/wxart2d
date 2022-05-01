/*! \file general/src/gen.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: gen.cpp,v 1.154 2009/09/26 19:01:07 titato Exp $
*/

#include "wxartbaseprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/tokenzr.h>
#include <wx/utils.h>
#include <wx/time.h> 

#include "wx/general/gen.h"
#include "wx/general/comevt.h"

#if wxART2D_USE_CVGIO
#include "wx/xmlparse/genxmlpars.h"
#endif //wxART2D_USE_CVGIO
#endif

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif


/*
// MSVC warning 4660 is quite stupid. It says that the template is already instantiated
// by using it, but it is not fully instantiated as required for a library
#ifdef _MSC_VER
#pragma warning(disable: 4660)
#endif

// explicit template instantiations
template class a2dPropertyIdTyped<a2dObjectPtr, a2dProperty>;

template class a2dSmrtPtrList<a2dObject>;

#ifdef _MSC_VER
#pragma warning(default: 4660)
#endif
*/

//#include "wx/general/id.inl"
#include "wx/general/smrtptr.inl"

a2dVersNo::a2dVersNo(const wxString& versionString)
{
    long major=0, minor=0, micro=0;
    wxStringTokenizer tokenizer( versionString, ".");
    if ( tokenizer.HasMoreTokens() )
        tokenizer.GetNextToken().ToLong( &major );
    if ( tokenizer.HasMoreTokens() )
        tokenizer.GetNextToken().ToLong( &minor );
    if ( tokenizer.HasMoreTokens() )
        tokenizer.GetNextToken().ToLong( &micro );
    m_major = major;
    m_minor = minor;
    m_micro = micro;
}

bool a2dVersNo::operator==( const a2dVersNo& rhs ) const
{
    return (m_major == rhs.m_major) && (m_minor == rhs.m_minor) && (m_micro == rhs.m_micro);
}

bool a2dVersNo::operator>( const a2dVersNo& rhs ) const
{
    return (m_major > rhs.m_major) ||
           ((m_major == rhs.m_major) && (m_minor > rhs.m_minor)) ||
           ((m_major == rhs.m_major) && (m_minor == rhs.m_minor) && (m_micro > rhs.m_micro));
}

bool a2dVersNo::operator>=( const a2dVersNo& rhs ) const
{
    return (m_major > rhs.m_major) ||
           ((m_major == rhs.m_major) && (m_minor > rhs.m_minor)) ||
           ((m_major == rhs.m_major) && (m_minor == rhs.m_minor) && (m_micro >= rhs.m_micro));
}


static long wxGenIdCount = 0;

A2DGENERALDLLEXP long wxGenNewId()
{
    return ++ wxGenIdCount;
}

A2DGENERALDLLEXP void wxGenRegisterId( long id )
{
    wxGenIdCount = id;
}

//#define A2D_RANDOM_ID
#ifdef A2D_RANDOM_ID
#include <random>

wxLongLong GetUniqueSerializationId()
{
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()    std::default_random_engine generator;
    std::uniform_int_distribution< long long > distribution(0, wxINT64_MAX);
    long long dice_roll = distribution(gen);  // generates number in the range 0..wxINT64_MAX
    return dice_roll;
}
#else
wxLongLong GetUniqueSerializationId()
{
	static wxLongLong timel = wxGetUTCTimeUSec();
	wxLongLong timenow = wxGetUTCTimeUSec();
	if ( timel >= timenow )
		timel++; //go ahead of time, as long as needed.
	else
		timel = timenow;

    return timel;
}
#endif


//!@{      \ingroup errorcodes
const a2dError a2dError_NoError( wxT( "a2dError_NoError" ), wxT( "All Oke" ) );
const a2dError a2dError_CommandError( wxT( "a2dError_CommandError" ), wxT( "" ) );
const a2dError a2dError_NotSpecified( wxT( "a2dError_NotSpecified" ), wxT( "" ) );
const a2dError a2dError_CanceledFileDialog( wxT( "a2dError_CanceledFileDialog" ), wxT( "" ) );
const a2dError a2dError_FileCouldNotOpen( wxT( "a2dError_FileCouldNotOpen" ), wxTRANSLATE( "Sorry, could not open file for saving" ) );
const a2dError a2dError_CouldNotCreateDocument( wxT( "a2dError_CouldNotCreateDocument" ), wxT( "" ) );
const a2dError a2dError_NoDocTemplateRef( wxT( "a2dError_NoDocTemplateRef" ), wxT( "" ) );
const a2dError a2dError_DocumentsOpened( wxT( "a2dError_DocumentsOpened" ), wxTRANSLATE( "No template defined in a2dDocument::OnSaveAsDocument" ) );
const a2dError a2dError_SetEnv( wxT( "a2dError_SetEnv" ), wxT( "" ) );
const a2dError a2dError_GetEnv( wxT( "a2dError_GetEnv" ), wxT( "" ) );
const a2dError a2dError_GetVar( wxT( "a2dError_GetVar" ), wxT( "" ) );
const a2dError a2dError_SaveFile( wxT( "a2dError_SaveFile" ), wxTRANSLATE( "Sorry, could not save document to file" ) );
const a2dError a2dError_LoadFile( wxT( "a2dError_LoadFile" ),  wxTRANSLATE( "Sorry, could not load file into document" ) );
const a2dError a2dError_ExportFile( wxT( "a2dError_ExportFile" ), wxTRANSLATE( "Sorry, could not export document to file" ) );
const a2dError a2dError_ImportFile( wxT( "a2dError_ImportFile" ),  wxTRANSLATE( "bad stream" ) );
const a2dError a2dError_IOHandler( wxT( "a2dError_IOHandler" ), wxTRANSLATE( "suitable template I/O handler for loading not available in document templates" ) );
const a2dError a2dError_SaveObject( wxT( "a2dError_SaveObject" ), wxTRANSLATE( "I/O handler cannot save document" ) );
const a2dError a2dError_LoadObject( wxT( "a2dError_LoadObject" ), wxTRANSLATE( "I/O handler cannot load document" ) );
const a2dError a2dError_FileHistory( wxT( "a2dError_FileHistory" ), wxT( "" ) );
const a2dError a2dError_ImportObject( wxT( "a2dError_ImportObject" ), wxT( "" ) );
const a2dError a2dError_XMLparse( wxT( "a2dError_XMLparse" ), wxT( "" ) );
const a2dError a2dError_FileVersion( wxT( "a2dError_FileVersion" ), wxTRANSLATE( "Version of file does not fit document" ) );
const a2dError a2dError_LoadLayers( wxT( "a2dError_LoadLayers" ), wxTRANSLATE( "Could not load layers" ) );
const a2dError a2dError_property( wxT( "a2dError_property" ), wxTRANSLATE( "problems on property" ) );
const a2dError a2dError_NoView( wxT( "a2dError_NoView" ), wxTRANSLATE( "Not a a2dCanvasView View" ) );
const a2dError a2dError_NoDocument( wxT( "a2dError_NoDocument" ), wxTRANSLATE( "No document set for a2dCanvasView in a2dCentralCanvasCommandProcessor::SetShowObject()" ) );
const a2dError a2dError_NoController( wxT( "a2dError_NoController" ), wxTRANSLATE( "No controller set for current view" ) );
const a2dError a2dError_NoTool( wxT( "a2dError_NoTool" ), wxTRANSLATE( "No tool available" ) );
const a2dError a2dError_LinkRef( wxT( "a2dError_LinkRef" ), wxT( "" ) );
const a2dError a2dError_NoWrapper( wxT( "a2dError_NoWrapper" ), wxT( "" ) );
const a2dError a2dError_LinkPin( wxT( "a2dError_LinkPin" ), wxT( "" ) );
const a2dError a2dError_NoPinClass( wxT( "a2dError_NoPinClass" ), wxT( "" ) );
const a2dError a2dError_CouldNotEvaluatePath( wxT( "a2dError_CouldNotEvaluatePath" ), wxT( "" ) );
const a2dError a2dError_Canceled( wxT( "a2dError_Canceled" ), wxT( "" ) );
const a2dError a2dError_CouldNotLoad( wxT( "a2dError_CouldNotLoad" ), wxTRANSLATE( "I/O handler error during loading this format." ) );
const a2dError a2dError_NoSelection( wxT( "a2dError_NoSelection" ), wxT( "" ) );
const a2dError a2dError_ToManyOpen( wxT( "a2dError_ToManyOpen" ), wxT( "" ) );
const a2dError a2dError_canvasObjectRelease( wxT( "a2dError_canvasObjectRelease" ), wxTRANSLATE( "could not release a2dCanvasObject in a2dCommand_ReleaseObject::Do" ) );
//!@}

a2dErrorHash& a2dError::GetHashMap()
{
    static a2dMemoryCriticalSectionHelper helper;
    static a2dErrorHash ms_Name2Id;
    return ms_Name2Id;
}


#if (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))
// by using it, but it is not fully instantiated as required for a library
#ifdef _MSC_VER
#pragma warning(disable: 4660)
#endif

// Explicit template instantiations for DLL

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DGENERALDLLEXP a2dSmrtPtr<a2dObject>;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<a2dSmrtPtr<a2dObject>, a2dProperty>;
template class A2DGENERALDLLEXP a2dAutoZeroPtr<class a2dObject>;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<a2dAutoZeroPtr<a2dObject>, a2dAutoZeroProperty>;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<a2dPathList,     a2dPathListProperty>   ;
//  template class A2DGENERALDLLEXP a2dSmrtPtr<a2dIOHandler>;
template class A2DGENERALDLLEXP a2dSmrtPtrList<a2dObject>;
template class A2DGENERALDLLEXP a2dlist<class a2dSmrtPtr<class a2dObject> >;
#endif

#ifdef _MSC_VER
#pragma warning(default: 4660)
#endif

#endif // (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))

IMPLEMENT_ABSTRACT_CLASS( a2dObject, wxObject )
IMPLEMENT_ABSTRACT_CLASS( a2dNamedProperty, a2dObject )
IMPLEMENT_DYNAMIC_CLASS( a2dProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dAutoZeroProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dBoolProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dStringProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dVoidPtrProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dInt16Property, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dUint16Property, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dInt32Property, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dUint32Property, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dDoubleProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dFloatProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dArrayStringProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dFileNameProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dMenuProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dUriProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dWindowProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dTipWindowProperty, a2dWindowProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dDateTimeProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dPathListProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dColourProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dObjectProperty, a2dNamedProperty )

WX_DEFINE_LIST( a2dResolveIdList );

//! initialize timestamp
long a2dEvent::ms_timestamp = 0;

//----------------------------------------------------------------------------
// a2dResolveIdInfo
//----------------------------------------------------------------------------

bool a2dResolveIdInfo::Assign( a2dObject* ptr )
{
    switch ( m_mode )
    {
        case mode_a2dObjectSmrtPtr:
            if( !ptr )
                return false;
            *m_storedHere.m_a2dObjectSmrtPtr = wxStaticCast( ptr, a2dObject );
            return true;

        case mode_a2dObjectAutoZeroPtr:
            if( !ptr )
                return false;
            *m_storedHere.m_a2dObjectAutoZeroPtr = wxStaticCast( ptr, a2dObject );
            return true;

        case mode_link:
            return m_storedHere.m_link->LinkReference( ptr );

        case mode_inform:
            m_storedHere.m_inform->ResolveInform();
            return true;

        default:
            wxASSERT( 0 );
            return false;
    }
}

//----------------------------------------------------------------------------
// a2dObject
//----------------------------------------------------------------------------

//------------------------------------------------------
//----------------- event handling ---------------------
//------------------------------------------------------

#if wxCHECK_VERSION(3,1,0)


bool a2dObject::ProcessEventLocally( wxEvent& event )
{
    // Try the hooks which should be called before our own handlers and this
    // handler itself first. Notice that we should not call ProcessEvent() on
    // this one as we're already called from it, which explains why we do it
    // here and not in DoTryChain()
    return TryBeforeAndHere( event ) || DoTryChain( event );
}

bool a2dObject::DoTryChain( wxEvent& event )
{
    for ( wxEvtHandler* h = GetNextHandler(); h; h = h->GetNextHandler() )
    {
        wxEventProcessInHandlerOnly processInHandlerOnly( event, h );
        if ( h->ProcessEvent( event ) )
        {
            event.Skip( false );

            return true;
        }

        if ( !event.ShouldProcessOnlyIn( h ) )
        {
            event.Skip();

            return true;
        }
    }

    return false;
}


bool a2dObject::TryHereOnly( wxEvent& event )
{
    // If the event handler is disabled it doesn't process any events
    if ( !GetEvtHandlerEnabled() )
        return false;

    // Handle per-instance dynamic event tables first
    if ( m_dynamicEvents && SearchDynamicEventTable( event ) )
        return true;

    // Then static per-class event tables
    if ( GetEventHashTable().HandleEvent( event, this ) )
        return true;

    // We don't have a handler for this event.
    return false;
}

bool a2dObject::ProcessEvent( wxEvent& event )
{
    // The very first thing we do is to allow the application to hook into
    // event processing in order to globally pre-process all events.
    //
    // Note that we should only do it if we're the first event handler called
    // to avoid calling FilterEvent() multiple times as the event goes through
    // the event handler chain and possibly upwards the window hierarchy.
    if ( !event.WasProcessed() )
    {
        if ( wxTheApp )
        {
            int rc = wxTheApp->FilterEvent( event );
            if ( rc != -1 )
            {
                wxASSERT_MSG( rc == 1 || rc == 0,
                              "unexpected wxApp::FilterEvent return value" );

                return rc != 0;
            }
            //else: proceed normally
        }
    }

    // Short circuit the event processing logic if we're requested to process
    // this event in this handler only, see DoTryChain() for more details.
    if ( event.ShouldProcessOnlyIn( this ) )
        return TryBeforeAndHere( event );


    // Try to process the event in this handler itself.
    if ( ProcessEventLocally( event ) )
    {
        // It is possible that DoTryChain() called from ProcessEventLocally()
        // returned true but the event was not really processed: this happens
        // if a custom handler ignores the request to process the event in this
        // handler only and in this case we should skip the post processing
        // done in TryAfter() but still return the correct value ourselves to
        // indicate whether we did or did not find a handler for this event.
        return !event.GetSkipped();
    }

    // If we still didn't find a handler, propagate the event upwards the
    // window chain and/or to the application object.
    if ( TryAfter( event ) )
        return true;


    // No handler found anywhere, bail out.
    return false;
}

bool a2dObject::SearchDynamicEventTable( wxEvent& event )
{
    wxCHECK_MSG( m_dynamicEvents, false,
                 wxT("caller should check that we have dynamic events") );

    bool returnval = false;
    DynamicEvents& dynamicEvents = *m_dynamicEvents;

    // MAKE sure that the receiving handlers, will not be destroyed while receiving events
	for ( size_t n = 0; n != dynamicEvents.size(); n++ )
    {
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* ) dynamicEvents[n];
        if( entry )
        {
            wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
            if ( eventSink && eventSink != this )
            {
                a2dObject* yes = wxDynamicCast( eventSink, a2dObject );
                if ( yes )
                    yes->SmrtPtrOwn();
            }
        }
    }

    m_iteratorCount++;

    bool needToPruneDeleted = false;

    // We can't use Get{First,Next}DynamicEntry() here as they hide the deleted
    // but not yet pruned entries from the caller, but here we do want to know
    // about them, so iterate directly. Remember to do it in the reverse order
    // to honour the order of handlers connection.
    for ( size_t n = dynamicEvents.size(); n; n-- )
    {
        a2dDynamicEventTableEntry* const entry = ( a2dDynamicEventTableEntry* ) dynamicEvents[n - 1];

        if ( !entry )
        {
            // This entry must have been unbound at some time in the past, so
            // skip it now and really remove it from the vector below, once we
            // finish iterating.
            needToPruneDeleted = true;
            continue;
        }

        if ( !entry->m_disconnect )
        {
			if ( event.GetEventType() == entry->m_eventType )
			{
				wxEvtHandler *handler = entry->m_fn->GetEvtHandler();
				if ( !handler )
				   handler = this;
				if ( ProcessEventIfMatchesId(*entry, handler, event) )
				{
					// It's important to skip pruning of the unbound event entries
					// below because this object itself could have been deleted by
					// the event handler making m_dynamicEvents a dangling pointer
					// which can't be accessed any longer in the code below.
					//
					// In practice, it hopefully shouldn't be a problem to wait
					// until we get an event that we don't handle before pruning
					// because this should happen soon enough and even if it
					// doesn't the worst possible outcome is slightly increased
					// memory consumption while not skipping pruning can result in
					// hard to reproduce (because they require the disconnection
					// and deletion happen at the same time which is not always the
					// case) crashes.
                    returnval = true;
                    break;
				}
			}
		}
    }
    m_iteratorCount--;

    // It is now save to destroy event handlers, so release them
	for ( size_t n = 0; n != dynamicEvents.size(); n++ )
    {
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* ) dynamicEvents[n];
        if( entry )
        {
            wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
            if ( eventSink && eventSink != this )
            {
                a2dObject* yes = wxDynamicCast( eventSink, a2dObject );
                if ( yes )
                    yes->SmrtPtrRelease();
            }
        }
    }
	
    if ( needToPruneDeleted )
    {
        size_t nNew = 0;
        for ( size_t n = 0; n != dynamicEvents.size(); n++ )
        {
            if ( dynamicEvents[n] )
                dynamicEvents[nNew++] = dynamicEvents[n];
        }

        wxASSERT( nNew != dynamicEvents.size() );
        dynamicEvents.resize(nNew);
    }

    // it looks this as already been done above, but processing the event,
    // might disconnect more events.
    if ( !m_iteratorCount && m_pendingDisconnects )
        CleanUpDisconnectedDynamic();
    return returnval;
}

void a2dObject::DoBind( int id,
                        int lastId,
                        wxEventType eventType,
                        wxEventFunctor* func,
                        wxObject* userData )
{
    a2dDynamicEventTableEntry* entry =
        new a2dDynamicEventTableEntry( eventType, id, lastId, func, userData );

    if (!m_dynamicEvents)
        m_dynamicEvents = new DynamicEvents;

    // We prefer to push back the entry here and then iterate over the vector
    // in reverse direction in GetNextDynamicEntry() as it's more efficient
    // than inserting the element at the front.
    m_dynamicEvents->push_back(entry);

    // Make sure we get to know when a sink is destroyed
    wxEvtHandler *eventSink = func->GetEvtHandler();
    if ( eventSink && eventSink != this )
    {
        wxEventConnectionRef *evtConnRef = FindRefInTrackerList(eventSink);
        if ( evtConnRef )
            evtConnRef->IncRef( );
        else
            new wxEventConnectionRef(this, eventSink);
    }
}

bool
a2dObject::DoUnbind(int id,
                       int lastId,
                       wxEventType eventType,
                       const wxEventFunctor& func,
                       wxObject *userData)
{
    if (!m_dynamicEvents)
        return false;

    size_t cookie;
    for ( a2dDynamicEventTableEntry* entry = (a2dDynamicEventTableEntry*) GetFirstDynamicEntry(cookie);
          entry;
          entry = (a2dDynamicEventTableEntry*) GetNextDynamicEntry(cookie) )
    {
        if ((entry->m_id == id) &&
            ((entry->m_lastId == lastId) || (lastId == wxID_ANY)) &&
            ((entry->m_eventType == eventType) || (eventType == wxEVT_NULL)) &&
            entry->m_fn->IsMatching(func) &&
            ((entry->m_callbackUserData == userData) || !userData))
        {
            entry->m_disconnect = true;
            m_pendingDisconnects = true;
            return true;
        }
    }
    return false;
}

void a2dObject::ConnectEvent( wxEventType type, wxEvtHandler* eventSink )
{
	//check first, if not already there.
	bool has = false;
	if ( m_dynamicEvents )
	{
		DynamicEvents& dynamicEvents = *m_dynamicEvents;
		for ( size_t n = 0; n != dynamicEvents.size(); n++ )
		{
			wxDynamicEventTableEntry* entry = ( wxDynamicEventTableEntry* ) dynamicEvents[n];
			if ( ( ( entry && entry->m_eventType == type ) || ( type == wxEVT_NULL ) ) &&
					( eventSink == entry->m_fn->GetEvtHandler() || ( eventSink == ( wxEvtHandler* )NULL ) ) )
				has = true;
		}
	}
	if ( ! has )
		Connect( wxID_ANY, wxID_ANY, type, wxObjectEventFunction( &a2dObject::ProcessConnectedEvent ), 0, eventSink );
}

bool a2dObject::DisconnectEvent( wxEventType eventType, wxEvtHandler* eventSink )
{
    if ( !m_dynamicEvents )
        return false;

    int id = wxID_ANY;
    int lastId = wxID_ANY; 
    const wxEventFunctor& func = wxMakeEventFunctor( eventType, wxObjectEventFunction( &a2dObject::ProcessConnectedEvent ), eventSink );
    wxObject* userData = 0;
	bool ret = false;

	DynamicEvents& dynamicEvents = *m_dynamicEvents;
	for ( size_t n = 0; n != dynamicEvents.size(); n++ )
	{
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* ) dynamicEvents[n];

        if ( ( entry && entry->m_id == id ) &&
                ( ( entry->m_lastId == lastId ) || ( lastId == wxID_ANY ) ) &&
                ( ( entry->m_eventType == eventType ) || ( eventType == wxEVT_NULL ) ) &&
                entry->m_fn->IsMatching( func ) &&
                ( ( entry->m_callbackUserData == userData ) || !userData ) )
        {
            entry->m_disconnect = true;
	        m_pendingDisconnects = true;
            ret = true;
        }
    }

    if ( !m_iteratorCount )
        CleanUpDisconnectedDynamic();
    return ret;
}

bool a2dObject::DisconnectEventAll( wxEvtHandler* eventSink )
{
    if ( !m_dynamicEvents )
        return false;

    bool did = false;
	DynamicEvents& dynamicEvents = *m_dynamicEvents;
    for ( size_t n = 0; n != dynamicEvents.size(); n++ )
    {
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* ) dynamicEvents[n];
        if ( entry && entry->m_fn->GetEvtHandler() == eventSink )
        {
            entry->m_disconnect = true;
            m_pendingDisconnects = true;
            did = true;
        }
    }

    if ( !m_iteratorCount )
        CleanUpDisconnectedDynamic();

    return did;
}

void a2dObject::ProcessConnectedEvent( wxEvent& event )
{
    if ( a2dGeneralGlobals->GetLogConnectedEvents() )
        wxLogDebug( wxT( "connect event: id: %d, type: %d on classname = %s" ), event.GetId(), event.GetEventType(), GetClassInfo()->GetClassName() );
    if ( ProcessEvent( event ) )
    {

        //even if the event was handled, and was not skipped,
        //we want it to skip it here, so the next dynamic connected event
        //in the calling wxEvtHandler::ProcessEventIfMatches, will be called too.
        //The signal is typically distributed to all connected classes.
        event.Skip( true );
        if ( a2dGeneralGlobals->GetLogConnectedEvents() )
            wxLogDebug( wxT( "    event processed: id: %d, type: %d on classname = %s" ), event.GetId(), event.GetEventType(), GetClassInfo()->GetClassName() );
        return;
    }
    if ( a2dGeneralGlobals->GetLogConnectedEvents() )
        wxLogDebug( wxT( "    event not processed: id: %d, type: %d on classname = %s" ), event.GetId(), event.GetEventType(), GetClassInfo()->GetClassName() );
    event.Skip( true );
}

void a2dObject::CleanUpDisconnectedDynamic()
{
	DynamicEvents& dynamicEvents = *m_dynamicEvents;
    size_t nNew = 0;
	for ( size_t n = 0; n != dynamicEvents.size(); n++ )
	{
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* ) dynamicEvents[n];
        if ( !entry )
        {}
        else if ( !entry->m_disconnect )
            dynamicEvents[nNew++] = dynamicEvents[n];
		else
		{
            // Remove connection from tracker node (wxEventConnectionRef)
            wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
            if ( eventSink && eventSink != this )
            {
                wxEventConnectionRef* evtConnRef = FindRefInTrackerList( eventSink );
                if ( evtConnRef )
                    evtConnRef->DecRef();
            }
            if ( entry->m_callbackUserData )
                delete entry->m_callbackUserData;
            delete entry;
        }
    }
    if ( nNew != dynamicEvents.size() )
        dynamicEvents.resize(nNew);
    m_pendingDisconnects = false;
}

//------------------------------------------------------
//----------------- older versions event handling ------
//------------------------------------------------------
#else //wxCHECK_VERSION(3,1,0)

#ifdef WXVALIDEVENTENTRY 

bool a2dObject::SearchDynamicEventTable( wxEvent& event )
{
    wxCHECK_MSG( m_dynamicEvents, false,
                 wxT( "caller should check that we have dynamic events" ) );

    wxList::compatibility_iterator node = m_dynamicEvents->GetFirst();
    while ( node )
    {
        wxDynamicEventTableEntry* entry = ( wxDynamicEventTableEntry* )node->GetData();
        wxList::compatibility_iterator nodeerase = node;
        node = node->GetNext();
        if ( entry->m_disconnect )
        {
            // Remove connection from tracker node (wxEventConnectionRef)
#if wxCHECK_VERSION(2,9,0)
            wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
            if ( eventSink && eventSink != this )
            {
                wxEventConnectionRef* evtConnRef = FindRefInTrackerList( eventSink );
                if ( evtConnRef )
                    evtConnRef->DecRef();
            }
#endif
            if ( entry->m_callbackUserData )
                delete entry->m_callbackUserData;
            m_dynamicEvents->Erase( nodeerase );
            delete entry;
        }
        else
            entry->m_inIteration = true;
    }

    // MAKE sure that the receiving handlers, will not be destroyed while receiving events
    node = m_dynamicEvents->GetFirst();
    while ( node )
    {
        wxDynamicEventTableEntry* entry = ( wxDynamicEventTableEntry* )node->GetData();
        wxList::compatibility_iterator nodeerase = node;
        node = node->GetNext();
#if wxCHECK_VERSION(2,9,0)
        wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
#else
        wxEvtHandler* eventSink = entry->m_eventSink;
#endif
        if ( eventSink && eventSink != this )
        {
            a2dObject* yes = wxDynamicCast( eventSink, a2dObject );
            if ( yes )
                yes->SmrtPtrOwn();
        }
    }

    node = m_dynamicEvents->GetFirst();
    while ( node )
    {
#if WXWIN_COMPATIBILITY_EVENT_TYPES
        wxEventTableEntry* entry = ( wxEventTableEntry* )node->GetData();
#else // !WXWIN_COMPATIBILITY_EVENT_TYPES
        wxDynamicEventTableEntry* entry = ( wxDynamicEventTableEntry* )node->GetData();
#endif // WXWIN_COMPATIBILITY_EVENT_TYPES/!WXWIN_COMPATIBILITY_EVENT_TYPES

        // get next node before (maybe) calling the event handler as it could
        // call Disconnect() invalidating the current node
        wxList::compatibility_iterator nodeerase = node;
        node = node->GetNext();

        if ( !entry->m_disconnect )
        {
#if !wxCHECK_VERSION(2,9,0)
            if ( ( event.GetEventType() == entry->m_eventType ) && ( entry->m_fn != 0 ) )
            {
                wxEvtHandler* handler =
#if !WXWIN_COMPATIBILITY_EVENT_TYPES
                    entry->m_eventSink ? entry->m_eventSink
                    :
#endif
                    this;

                if ( ProcessEventIfMatches( *entry, handler, event ) )
                {
                    return true;
                }
            }
#else
            if ( event.GetEventType() == entry->m_eventType )
            {
                wxEvtHandler* handler = entry->m_fn->GetEvtHandler();
                if ( !handler )
                    handler = this;
                if ( ProcessEventIfMatchesId( *entry, handler, event ) )
                    return true;
            }
#endif
        }
    }

    // It is now save to destroy event handlers.
    node = m_dynamicEvents->GetFirst();
    while ( node )
    {
        wxDynamicEventTableEntry* entry = ( wxDynamicEventTableEntry* )node->GetData();
        wxList::compatibility_iterator nodeerase = node;
        node = node->GetNext();

#if wxCHECK_VERSION(2,9,0)
        wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
#else
        wxEvtHandler* eventSink = entry->m_eventSink;
#endif
        if ( eventSink && eventSink != this )
        {
            a2dObject* yes = wxDynamicCast( eventSink, a2dObject );
            if ( yes )
                yes->SmrtPtrRelease();
        }
    }


    // it looks this as already been done above, but processing the event,
    // might disconnect more events.
    node = m_dynamicEvents->GetFirst();
    while ( node )
    {
        wxDynamicEventTableEntry* entry = ( wxDynamicEventTableEntry* )node->GetData();
        wxList::compatibility_iterator nodeerase = node;
        node = node->GetNext();
        if ( entry->m_disconnect )
        {
#if wxCHECK_VERSION(2,9,0)
            // Remove connection from tracker node (wxEventConnectionRef)
            wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
            if ( eventSink && eventSink != this )
            {
                wxEventConnectionRef* evtConnRef = FindRefInTrackerList( eventSink );
                if ( evtConnRef )
                    evtConnRef->DecRef();
            }
#endif
            if ( entry->m_callbackUserData )
                delete entry->m_callbackUserData;
            m_dynamicEvents->Erase( nodeerase );
            delete entry;
        }
        else
            entry->m_inIteration = false;
    }

    return false;
}

#else //WXVALIDEVENTENTRY 

#if !wxCHECK_VERSION(2,9,0)

bool a2dObject::ProcessEvent( wxEvent& event )
{
    // allow the application to hook into event processing
    if ( wxTheApp )
    {
        int rc = wxTheApp->FilterEvent( event );
        if ( rc != -1 )
        {
            wxASSERT_MSG( rc == 1 || rc == 0,
                          _T( "unexpected wxApp::FilterEvent return value" ) );

            return rc != 0;
        }
        //else: proceed normally
    }

    // An event handler can be enabled or disabled
    if ( GetEvtHandlerEnabled() )
    {
        // if we have a validator, it has higher priority than our own event
        // table
        if ( TryValidator( event ) )
            return true;

        // Handle per-instance dynamic event tables first
        if ( m_dynamicEvents && SearchDynamicEventTable( event ) )
            return true;

        // Then static per-class event tables
        if ( GetEventHashTable().HandleEvent( event, this ) )
            return true;
    }

    // Try going down the event handler chain
    if ( GetNextHandler() )
    {
        if ( GetNextHandler()->ProcessEvent( event ) )
            return true;
    }

    // Finally propagate the event upwards the window chain and/or to the
    // application object as necessary
    return TryParent( event );
}
#else

bool a2dObject::ProcessEventLocally( wxEvent& event )
{
    // Try the hooks which should be called before our own handlers and this
    // handler itself first. Notice that we should not call ProcessEvent() on
    // this one as we're already called from it, which explains why we do it
    // here and not in DoTryChain()
    return TryBeforeAndHere( event ) || DoTryChain( event );
}

bool a2dObject::DoTryChain( wxEvent& event )
{
    for ( wxEvtHandler* h = GetNextHandler(); h; h = h->GetNextHandler() )
    {
        wxEventProcessInHandlerOnly processInHandlerOnly( event, h );
        if ( h->ProcessEvent( event ) )
        {
            event.Skip( false );

            return true;
        }

        if ( !event.ShouldProcessOnlyIn( h ) )
        {
            event.Skip();

            return true;
        }
    }

    return false;
}


bool a2dObject::TryHereOnly( wxEvent& event )
{
    // If the event handler is disabled it doesn't process any events
    if ( !GetEvtHandlerEnabled() )
        return false;

    // Handle per-instance dynamic event tables first
    if ( m_dynamicEvents && SearchDynamicEventTable( event ) )
        return true;

    // Then static per-class event tables
    if ( GetEventHashTable().HandleEvent( event, this ) )
        return true;

    // We don't have a handler for this event.
    return false;
}

bool a2dObject::ProcessEvent( wxEvent& event )
{
    // The very first thing we do is to allow the application to hook into
    // event processing in order to globally pre-process all events.
    //
    // Note that we should only do it if we're the first event handler called
    // to avoid calling FilterEvent() multiple times as the event goes through
    // the event handler chain and possibly upwards the window hierarchy.
    if ( !event.WasProcessed() )
    {
        if ( wxTheApp )
        {
            int rc = wxTheApp->FilterEvent( event );
            if ( rc != -1 )
            {
                wxASSERT_MSG( rc == 1 || rc == 0,
                              "unexpected wxApp::FilterEvent return value" );

                return rc != 0;
            }
            //else: proceed normally
        }
    }

    // Short circuit the event processing logic if we're requested to process
    // this event in this handler only, see DoTryChain() for more details.
    if ( event.ShouldProcessOnlyIn( this ) )
        return TryBeforeAndHere( event );


    // Try to process the event in this handler itself.
    if ( ProcessEventLocally( event ) )
    {
        // It is possible that DoTryChain() called from ProcessEventLocally()
        // returned true but the event was not really processed: this happens
        // if a custom handler ignores the request to process the event in this
        // handler only and in this case we should skip the post processing
        // done in TryAfter() but still return the correct value ourselves to
        // indicate whether we did or did not find a handler for this event.
        return !event.GetSkipped();
    }

    // If we still didn't find a handler, propagate the event upwards the
    // window chain and/or to the application object.
    if ( TryAfter( event ) )
        return true;


    // No handler found anywhere, bail out.
    return false;
}
#endif

bool a2dObject::SearchDynamicEventTable( wxEvent& event )
{
    wxCHECK_MSG( m_dynamicEvents, false,
                 wxT( "caller should check that we have dynamic events" ) );

    bool returnval = false;
    wxList::compatibility_iterator node = m_dynamicEvents->GetFirst();
/*
    if ( !m_iteratorCount && m_pendingDisconnects )
    {
        while ( node )
        {
            a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* )node->GetData();
            wxList::compatibility_iterator nodeerase = node;
            node = node->GetNext();
            if ( entry->m_disconnect )
            {
                // Remove connection from tracker node (wxEventConnectionRef)
    #if wxCHECK_VERSION(2,9,0)
                wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
                if ( eventSink && eventSink != this )
                {
                    wxEventConnectionRef* evtConnRef = FindRefInTrackerList( eventSink );
                    if ( evtConnRef )
                        evtConnRef->DecRef();
                }
    #endif
                if ( entry->m_callbackUserData )
                    delete entry->m_callbackUserData;
                m_dynamicEvents->Erase( nodeerase );
                delete entry;
            }
        }
        m_pendingDisconnects = false;
    }
*/
    // MAKE sure that the receiving handlers, will not be destroyed while receiving events
    node = m_dynamicEvents->GetFirst();
    while ( node )
    {
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* )node->GetData();
        wxList::compatibility_iterator nodeerase = node;
        node = node->GetNext();
#if wxCHECK_VERSION(2,9,0)
        wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
#else
        wxEvtHandler* eventSink = entry->m_eventSink;
#endif
        if ( eventSink && eventSink != this )
        {
            a2dObject* yes = wxDynamicCast( eventSink, a2dObject );
            if ( yes )
                yes->SmrtPtrOwn();
        }
    }

    m_iteratorCount++;
    node = m_dynamicEvents->GetFirst();
    while ( node )
    {
#if WXWIN_COMPATIBILITY_EVENT_TYPES
        wxEventTableEntry* entry = ( wxEventTableEntry* )node->GetData();
#else // !WXWIN_COMPATIBILITY_EVENT_TYPES
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* )node->GetData();
#endif // WXWIN_COMPATIBILITY_EVENT_TYPES/!WXWIN_COMPATIBILITY_EVENT_TYPES

        // get next node before (maybe) calling the event handler as it could
        // call Disconnect() invalidating the current node
        wxList::compatibility_iterator nodeerase = node;
        node = node->GetNext();

        if ( !entry->m_disconnect )
        {
#if !wxCHECK_VERSION(2,9,0)
            if ( ( event.GetEventType() == entry->m_eventType ) && ( entry->m_fn != 0 ) )
            {
                wxEvtHandler* handler =
#if !WXWIN_COMPATIBILITY_EVENT_TYPES
                    entry->m_eventSink ? entry->m_eventSink
                    :
#endif
                    this;

                if ( ProcessEventIfMatches( *entry, handler, event ) )
                {
                    returnval = true;
                    break;
                }
            }
#else
            if ( event.GetEventType() == entry->m_eventType )
            {
                wxEvtHandler* handler = entry->m_fn->GetEvtHandler();
                if ( !handler )
                    handler = this;
                if ( ProcessEventIfMatchesId( *entry, handler, event ) )
                {
                    returnval = true;
                    break;
                }
            }
#endif
        }
    }
    m_iteratorCount--;

    // It is now save to destroy event handlers.
    node = m_dynamicEvents->GetFirst();
    while ( node )
    {
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* )node->GetData();
        wxList::compatibility_iterator nodeerase = node;
        node = node->GetNext();

#if wxCHECK_VERSION(2,9,0)
        wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
#else
        wxEvtHandler* eventSink = entry->m_eventSink;
#endif
        if ( eventSink && eventSink != this )
        {
            a2dObject* yes = wxDynamicCast( eventSink, a2dObject );
            if ( yes )
                yes->SmrtPtrRelease();
        }
    }


    // it looks this as already been done above, but processing the event,
    // might disconnect more events.
    if ( !m_iteratorCount && m_pendingDisconnects )
    {
        node = m_dynamicEvents->GetFirst();
        while ( node )
        {
            a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* )node->GetData();
            wxList::compatibility_iterator nodeerase = node;
            node = node->GetNext();
            if ( entry->m_disconnect )
            {
    #if wxCHECK_VERSION(2,9,0)
                // Remove connection from tracker node (wxEventConnectionRef)
                wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
                if ( eventSink && eventSink != this )
                {
                    wxEventConnectionRef* evtConnRef = FindRefInTrackerList( eventSink );
                    if ( evtConnRef )
                        evtConnRef->DecRef();
                }
    #endif
                if ( entry->m_callbackUserData )
                    delete entry->m_callbackUserData;
                m_dynamicEvents->Erase( nodeerase );
                delete entry;
            }
        }
        m_pendingDisconnects = false;
    }
    return returnval;
}


#if !wxCHECK_VERSION(2,9,0)

void a2dObject::Connect( int id, int lastId,
                         int eventType,
                         wxObjectEventFunction func,
                         wxObject* userData,
                         wxEvtHandler* eventSink )
{
#if WXWIN_COMPATIBILITY_EVENT_TYPES
    wxEventTableEntry* entry = new wxEventTableEntry;
    entry->m_eventType = eventType;
    entry->m_id = id;
    entry->m_lastId = lastId;
    entry->m_fn = func;
    entry->m_callbackUserData = userData;
#else // !WXWIN_COMPATIBILITY_EVENT_TYPES
    a2dDynamicEventTableEntry* entry =
        new a2dDynamicEventTableEntry( eventType, id, lastId, func, userData, eventSink );
#endif // WXWIN_COMPATIBILITY_EVENT_TYPES/!WXWIN_COMPATIBILITY_EVENT_TYPES

    if ( !m_dynamicEvents )
        m_dynamicEvents = new wxList;

    // Insert at the front of the list so most recent additions are found first
    m_dynamicEvents->Insert( ( wxObject* ) entry );
}

bool a2dObject::Disconnect( int id, int lastId, wxEventType eventType,
                            wxObjectEventFunction func,
                            wxObject* userData,
                            wxEvtHandler* eventSink )
{
    if ( !m_dynamicEvents )
        return false;

    wxList::compatibility_iterator node = m_dynamicEvents->GetFirst();
    while ( node )
    {
#if WXWIN_COMPATIBILITY_EVENT_TYPES
        wxEventTableEntry* entry = ( wxEventTableEntry* )node->GetData();
#else // !WXWIN_COMPATIBILITY_EVENT_TYPES
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* )node->GetData();
#endif // WXWIN_COMPATIBILITY_EVENT_TYPES/!WXWIN_COMPATIBILITY_EVENT_TYPES

        wxList::compatibility_iterator nodeerase = node;
        node = node->GetNext();
        if ( ( entry->m_id == id ) &&
                ( ( entry->m_lastId == lastId ) || ( lastId == wxID_ANY ) ) &&
                ( ( entry->m_eventType == eventType ) || ( eventType == wxEVT_NULL ) ) &&
                ( ( entry->m_fn == func ) || ( func == ( wxObjectEventFunction )NULL ) ) &&
                ( ( entry->m_eventSink == eventSink ) || ( eventSink == ( wxEvtHandler* )NULL ) ) &&
                ( ( entry->m_callbackUserData == userData ) || ( userData == ( wxObject* )NULL ) ) )
        {
            entry->m_disconnect = true;

            if ( !m_iteratorCount )
            {
#if wxCHECK_VERSION(2,9,0)
                // Remove connection from tracker node (wxEventConnectionRef)
                wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
                if ( eventSink && eventSink != this )
                {
                    wxEventConnectionRef* evtConnRef = FindRefInTrackerList( eventSink );
                    if ( evtConnRef )
                        evtConnRef->DecRef();
                }
#endif
                if ( entry->m_callbackUserData )
                    delete entry->m_callbackUserData;
                m_dynamicEvents->Erase( nodeerase );
                delete entry;
            }
            else
                m_pendingDisconnects = true;

            return true;
        }
    }
    return false;
}

#else

void a2dObject::DoBind( int id,
                        int lastId,
                        wxEventType eventType,
                        wxEventFunctor* func,
                        wxObject* userData )
{
    a2dDynamicEventTableEntry* entry =
        new a2dDynamicEventTableEntry( eventType, id, lastId, func, userData );

    if ( !m_dynamicEvents )
        m_dynamicEvents = new wxList;

    // Insert at the front of the list so most recent additions are found first
    m_dynamicEvents->Insert( ( wxObject* ) entry );

    // Make sure we get to know when a sink is destroyed
    wxEvtHandler* eventSink = func->GetEvtHandler();
    if ( eventSink && eventSink != this )
    {
        wxEventConnectionRef* evtConnRef = FindRefInTrackerList( eventSink );
        if ( evtConnRef )
            evtConnRef->IncRef( );
        else
            new wxEventConnectionRef( this, eventSink );
    }
}

bool
a2dObject::DoUnbind( int id,
                     int lastId,
                     wxEventType eventType,
                     const wxEventFunctor& func,
                     wxObject* userData )
{
    if ( !m_dynamicEvents )
        return false;

    wxList::compatibility_iterator node = m_dynamicEvents->GetFirst();
    while ( node )
    {
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* )node->GetData();

        if ( ( entry->m_id == id ) &&
                ( ( entry->m_lastId == lastId ) || ( lastId == wxID_ANY ) ) &&
                ( ( entry->m_eventType == eventType ) || ( eventType == wxEVT_NULL ) ) &&
                entry->m_fn->IsMatching( func ) &&
                ( ( entry->m_callbackUserData == userData ) || !userData ) )
        {
            entry->m_disconnect = true;
            m_pendingDisconnects = true;
            return true;
        }
        node = node->GetNext();
    }
    return false;
}

#endif

#endif //WXVALIDEVENTENTRY 


void a2dObject::ProcessConnectedEvent( wxEvent& event )
{
    if ( a2dGeneralGlobals->GetLogConnectedEvents() )
        wxLogDebug( wxT( "connect event: id: %d, type: %d on classname = %s" ), event.GetId(), event.GetEventType(), GetClassInfo()->GetClassName() );
    if ( ProcessEvent( event ) )
    {

        //even if the event was handled, and was not skipped,
        //we want it to skip it here, so the next dynamic connected event
        //in the calling wxEvtHandler::ProcessEventIfMatches, will be called too.
        //The signal is typically distributed to all connected classes.
        event.Skip( true );
        if ( a2dGeneralGlobals->GetLogConnectedEvents() )
            wxLogDebug( wxT( "    event processed: id: %d, type: %d on classname = %s" ), event.GetId(), event.GetEventType(), GetClassInfo()->GetClassName() );
        return;
    }
    if ( a2dGeneralGlobals->GetLogConnectedEvents() )
        wxLogDebug( wxT( "    event not processed: id: %d, type: %d on classname = %s" ), event.GetId(), event.GetEventType(), GetClassInfo()->GetClassName() );
    event.Skip( true );
}

void a2dObject::ConnectEvent( wxEventType type, wxEvtHandler* eventSink )
{
    //check first, if not already there.
    bool has = false;
    if ( m_dynamicEvents )
    {
        wxList::compatibility_iterator node = m_dynamicEvents->GetFirst();
        while ( node )
        {
            wxDynamicEventTableEntry* entry = ( wxDynamicEventTableEntry* )node->GetData();
            if ( ( ( entry->m_eventType == type ) || ( type == wxEVT_NULL ) ) &&
#if !wxCHECK_VERSION(2,9,0)
                    ( ( entry->m_eventSink == eventSink ) || ( eventSink == ( wxEvtHandler* )NULL ) ) )
#else
                    ( eventSink == entry->m_fn->GetEvtHandler() || ( eventSink == ( wxEvtHandler* )NULL ) ) )
#endif
                has = true;
            node = node->GetNext();
        }
    }
    if ( ! has )
        Connect( wxID_ANY, wxID_ANY, type, wxObjectEventFunction( &a2dObject::ProcessConnectedEvent ), 0, eventSink );
}


#if !wxCHECK_VERSION(2,9,0)
bool a2dObject::DisconnectEvent( wxEventType eventType, wxEvtHandler* eventSink )
{
    if ( !m_dynamicEvents )
        return false;

    int id = wxID_ANY;
    int lastId = wxID_ANY; 
    wxObjectEventFunction func = wxObjectEventFunction( &a2dObject::ProcessConnectedEvent );
    wxObject* userData = 0;

    wxList::compatibility_iterator node = m_dynamicEvents->GetFirst();
    while ( node )
    {
#ifdef WXVALIDEVENTENTRY 

#if WXWIN_COMPATIBILITY_EVENT_TYPES
        wxEventTableEntry* entry = ( wxEventTableEntry* )node->GetData();
#else // !WXWIN_COMPATIBILITY_EVENT_TYPES
        wxDynamicEventTableEntry* entry = ( wxDynamicEventTableEntry* )node->GetData();
#endif // WXWIN_COMPATIBILITY_EVENT_TYPES/!WXWIN_COMPATIBILITY_EVENT_TYPES

#else //WXVALIDEVENTENTRY 

#if WXWIN_COMPATIBILITY_EVENT_TYPES
        wxEventTableEntry* entry = ( wxEventTableEntry* )node->GetData();
#else // !WXWIN_COMPATIBILITY_EVENT_TYPES
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* )node->GetData();
#endif // WXWIN_COMPATIBILITY_EVENT_TYPES/!WXWIN_COMPATIBILITY_EVENT_TYPES

#endif //WXVALIDEVENTENTRY 

        wxList::compatibility_iterator nodeerase = node;
        node = node->GetNext();
        if ( ( entry->m_id == id ) &&
                ( ( entry->m_lastId == lastId ) || ( lastId == wxID_ANY ) ) &&
                ( ( entry->m_eventType == eventType ) || ( eventType == wxEVT_NULL ) ) &&
                ( ( entry->m_fn == func ) || ( func == ( wxObjectEventFunction )NULL ) ) &&
                ( ( entry->m_eventSink == eventSink ) || ( eventSink == ( wxEvtHandler* )NULL ) ) &&
                ( ( entry->m_callbackUserData == userData ) || ( userData == ( wxObject* )NULL ) ) )
        {
            entry->m_disconnect = true;

            if ( !m_iteratorCount )
            {
#if wxCHECK_VERSION(2,9,0)
                // Remove connection from tracker node (wxEventConnectionRef)
                wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
                if ( eventSink && eventSink != this )
                {
                    wxEventConnectionRef* evtConnRef = FindRefInTrackerList( eventSink );
                    if ( evtConnRef )
                        evtConnRef->DecRef();
                }
#endif
                if ( entry->m_callbackUserData )
                    delete entry->m_callbackUserData;
                m_dynamicEvents->Erase( nodeerase );
                delete entry;
            }
            else
                m_pendingDisconnects = true;

            return true;
        }
    }
    return false;
}

#else //!wxCHECK_VERSION(2,9,0)
bool a2dObject::DisconnectEvent( wxEventType eventType, wxEvtHandler* eventSink )
{
    if ( !m_dynamicEvents )
        return false;

    int id = wxID_ANY;
    int lastId = wxID_ANY; 
    const wxEventFunctor& func = wxMakeEventFunctor( eventType, wxObjectEventFunction( &a2dObject::ProcessConnectedEvent ), eventSink );
    wxObject* userData = 0;

    wxList::compatibility_iterator node = m_dynamicEvents->GetFirst(), node_nxt;
    while ( node )
    {
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* )node->GetData();
        node_nxt = node->GetNext();

        if ( ( entry->m_id == id ) &&
                ( ( entry->m_lastId == lastId ) || ( lastId == wxID_ANY ) ) &&
                ( ( entry->m_eventType == eventType ) || ( eventType == wxEVT_NULL ) ) &&
                entry->m_fn->IsMatching( func ) &&
                ( ( entry->m_callbackUserData == userData ) || !userData ) )
        {
            entry->m_disconnect = true;
            if ( !m_iteratorCount )
            {
#if wxCHECK_VERSION(2,9,0)
                // Remove connection from tracker node (wxEventConnectionRef)
                wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
                if ( eventSink && eventSink != this )
                {
                    wxEventConnectionRef* evtConnRef = FindRefInTrackerList( eventSink );
                    if ( evtConnRef )
                        evtConnRef->DecRef();
                }
#endif
                if ( entry->m_callbackUserData )
                    delete entry->m_callbackUserData;
                m_dynamicEvents->Erase( node );
                delete entry;
            }
            else
                m_pendingDisconnects = true;
            return true;
        }
        node = node_nxt;
    }
    return false;
}
#endif //!wxCHECK_VERSION(2,9,0)

bool a2dObject::DisconnectEventAll( wxEvtHandler* eventSink )
{
    if ( !m_dynamicEvents )
        return false;

#ifdef WXVALIDEVENTENTRY 

    bool did = false;
#if !wxCHECK_VERSION(2,9,0)
    wxList::compatibility_iterator node = m_dynamicEvents->GetFirst();
    while ( node )
    {
#if WXWIN_COMPATIBILITY_EVENT_TYPES
        wxEventTableEntry* entry = ( wxEventTableEntry* )node->GetData();
#else // !WXWIN_COMPATIBILITY_EVENT_TYPES
        wxDynamicEventTableEntry* entry = ( wxDynamicEventTableEntry* )node->GetData();
#endif // WXWIN_COMPATIBILITY_EVENT_TYPES/!WXWIN_COMPATIBILITY_EVENT_TYPES

        if ( ( entry->m_eventSink == eventSink ) || ( eventSink == ( wxEvtHandler* )NULL ) )
        {
            entry->m_disconnect = true;
            did = true;
        }
        node = node->GetNext();
    }
#else
    wxList::compatibility_iterator node = m_dynamicEvents->GetFirst(), node_nxt;
    while ( node )
    {
        wxDynamicEventTableEntry* entry = ( wxDynamicEventTableEntry* )node->GetData();
        node_nxt = node->GetNext();

        if ( entry->m_fn->GetEvtHandler() == eventSink )
        {
            entry->m_disconnect = true;
            did = true;
        }
        node = node_nxt;
    }
#endif

#else //WXVALIDEVENTENTRY 

    bool did = false;
#if !wxCHECK_VERSION(2,9,0)
    wxList::compatibility_iterator node = m_dynamicEvents->GetFirst();
    while ( node )
    {
#if WXWIN_COMPATIBILITY_EVENT_TYPES
        wxEventTableEntry* entry = ( wxEventTableEntry* )node->GetData();
#else // !WXWIN_COMPATIBILITY_EVENT_TYPES
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* )node->GetData();
#endif // WXWIN_COMPATIBILITY_EVENT_TYPES/!WXWIN_COMPATIBILITY_EVENT_TYPES

        wxList::compatibility_iterator nodeerase = node;
        node = node->GetNext();
        if ( ( entry->m_eventSink == eventSink ) || ( eventSink == ( wxEvtHandler* )NULL ) )
        {
            entry->m_disconnect = true;
            if ( !m_iteratorCount )
            {
#if wxCHECK_VERSION(2,9,0)
                // Remove connection from tracker node (wxEventConnectionRef)
                wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
                if ( eventSink && eventSink != this )
                {
                    wxEventConnectionRef* evtConnRef = FindRefInTrackerList( eventSink );
                    if ( evtConnRef )
                        evtConnRef->DecRef();
                }
#endif
                if ( entry->m_callbackUserData )
                    delete entry->m_callbackUserData;
                m_dynamicEvents->Erase( nodeerase );
                delete entry;
            }
            else
                m_pendingDisconnects = true;

            did = true;
        }
    }
#else
    wxList::compatibility_iterator node = m_dynamicEvents->GetFirst(), node_nxt;
    while ( node )
    {
        a2dDynamicEventTableEntry* entry = ( a2dDynamicEventTableEntry* )node->GetData();
        wxList::compatibility_iterator nodeerase = node;
        node_nxt = node->GetNext();

        if ( entry->m_fn->GetEvtHandler() == eventSink )
        {
            entry->m_disconnect = true;
            if ( !m_iteratorCount )
            {
#if wxCHECK_VERSION(2,9,0)
                // Remove connection from tracker node (wxEventConnectionRef)
                wxEvtHandler* eventSink = entry->m_fn->GetEvtHandler();
                if ( eventSink && eventSink != this )
                {
                    wxEventConnectionRef* evtConnRef = FindRefInTrackerList( eventSink );
                    if ( evtConnRef )
                        evtConnRef->DecRef();
                }
#endif
                if ( entry->m_callbackUserData )
                    delete entry->m_callbackUserData;
                m_dynamicEvents->Erase( node );
                delete entry;
            }
            else
                m_pendingDisconnects = true;
            did = true;
        }
        node = node_nxt;
    }
#endif

#endif //WXVALIDEVENTENTRY 

    return did;
}

#endif //wxCHECK_VERSION(3,1,0)


#ifdef _DEBUG
// Here are two globals that can be used as registers in the debugger
a2dObject* _dbro1 = 0;
a2dObject* _dbro2 = 0;
#endif

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
A2DGENERALDLLEXP_DATA ( wxObject* ) CurrentSmartPointerOwner = 0;
#endif

a2dPropertyIdBool* a2dObject::PROPID_Check = NULL;

INITIALIZE_PROPERTIES_BASE( a2dObject )
{
    PROPID_Check = new a2dPropertyIdBool( wxT( "Check" ),
                                          a2dPropertyId::flag_temporary | a2dPropertyId::flag_norender, false,
                                          a2dPropertyIdBool::Get( &a2dObject::GetCheck ),
                                          a2dPropertyIdBool::Set( &a2dObject::SetCheck ) );
    AddPropertyId( PROPID_Check );
    A2D_PROPID_D_F( a2dPropertyIdUri, URI, wxURI(), a2dPropertyId::flag_isEditable );
/*
    PROPID_Name = new a2dPropertyIdString(  wxT( "Name" ), 
                                          a2dPropertyId::flag_isEditable | a2dPropertyId::flag_norender, wxT(""),
                                          a2dPropertyIdString::Get( &a2dObject::GetName ),
                                          a2dPropertyIdString::Set( &a2dObject::SetName ) );
    AddPropertyId( PROPID_Name );

*/
    A2D_PROPID_D_F( a2dPropertyIdString, Name, wxT( "" ), a2dPropertyId::flag_isEditable | a2dPropertyId::flag_norender );
    A2D_PROPID_D( a2dPropertyIdVoidPtr, autozeroptrlist, 0 );

    return true;
}

a2dPropertyIdString* a2dObject::PROPID_Name = NULL;
a2dPropertyIdUri* a2dObject::PROPID_URI = NULL;
a2dPropertyIdVoidPtr* a2dObject::PROPID_autozeroptrlist = NULL;

const a2dSignal a2dObject::sm_Edit_properties = wxNewId();

a2dObject::a2dObject()
{
    m_refcount = 0;
    m_autozeroptrlist = 0;
    m_check = false;
    m_release = false;
    m_recursion_active = false;
    m_iteratorCount = 0;
    m_pendingDisconnects = false;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_ownerlist = 0;
    CurrentSmartPointerOwner = this;
#endif
    m_id = GetUniqueSerializationId();
}

a2dObject* a2dObject::Clone( CloneOptions options, a2dRefMap* refs ) const
{
	if ( refs ) //&& m_refcount > 1 ) //other do point to this object
    {
        wxLongLong id = m_id;
        wxASSERT( id != 0 );
        wxString resolveKey;
        resolveKey << id;
    
	    a2dHashMapIntToObject::iterator obj = refs->GetObjectHashMap().find( resolveKey );

        if( obj == refs->GetObjectHashMap().end() )
        {
			a2dObject* clone = DoClone( options, refs );
			refs->GetObjectHashMap()[resolveKey] = clone; //const_cast<a2dObject*>(this);
			return clone;
        }

        //the object referenced is already cloned return it.
        return  wxDynamicCast( obj->second, a2dObject );
	}
	return DoClone( options, refs ); //new a2dObject( *this, options );
}

a2dObject::a2dObject( const a2dObject& other, CloneOptions options, a2dRefMap* refs )
// private wxEvtHandler( other )
{
    m_id = GetUniqueSerializationId();
    m_refcount = 0;
    m_autozeroptrlist = 0;
    m_check = false;
    m_release = false;
    m_recursion_active = false;
    m_iteratorCount = 0;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_ownerlist = 0;
    CurrentSmartPointerOwner = this;
#endif

    a2dNamedPropertyList::const_iterator iter;
    for( iter = other.m_propertylist.begin(); iter != other.m_propertylist.end(); ++iter )
    {
        const a2dNamedProperty* prop = *iter;
        //properties it self are not refcounted, but its data may be, therefore options are also needed there.
        m_propertylist.push_back( prop->Clone( options, refs ) );
    }

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in property->Clone()
    CurrentSmartPointerOwner = this;
#endif
}


a2dObject::~a2dObject()
{
    wxASSERT_MSG(  m_refcount == 0 || m_refcount == refcount_nondynamic, wxT( "deleting a2dObject while referenced" ) );
    m_autozeroptrlist->TargetDestructing();
}

void a2dObject:: operator = ( const a2dObject& WXUNUSED( other ) )
{
    // m_refcount is not touched
    // m_autozerolist is not touched
    // m_ownerlist is not touched
    // m_recusionactive is not touched
    // m_check is not touched
}

//void a2dObject::operator delete(void* recordptr)
//{
//  ::delete recordptr;
//}

#if wxUSE_STD_IOSTREAM && (defined(__WXDEBUG__) || wxUSE_DEBUG_CONTEXT)
void a2dObject::Dump( wxSTD ostream& str )
{
    str << m_id;
}
#endif

void a2dObject::CheckReferences()
{
    if ( m_refcount <= 0 )
        delete this;
}

bool a2dObject::SmrtPtrRelease()
{
    m_refcount--;
    wxASSERT_MSG( m_refcount >= 0, wxT( "a2dObject Own/Release not matched (extra Release calls)" ) );
    if ( m_refcount <= 0 )
    {
        delete this;
        return true;
    }
    return false;
}

a2dObject* a2dObject::SetNonDynamic()
{
    wxASSERT_MSG( m_refcount == 0, wxT( "Setting a referenced a2dObject to non-dynamic" ) );
    m_refcount += refcount_nondynamic;
    return this;
}

void a2dObject::SetName( const wxString& name )
{
    PROPID_Name->SetPropertyToObject( this, name );
}

wxString a2dObject::GetName() const
{
    //first search if a reserved property exists
    const a2dNamedProperty* p = FindProperty( PROPID_Name );
    if ( p )
        return p->StringValueRepresentation();
    return wxString::Format( wxT( "%lld" ), m_id );
}

#if wxART2D_USE_CVGIO

void a2dObject::SaveAsDocument( a2dIOHandlerXmlSerOut& out )
{
    out.WriteStartDocument( wxT( "1.0" ), wxT( "UTF-8" ) , true );

    out.WriteStartElementAttributes( wxT( "cvg" ) );
    out.WriteEndAttributes();

    a2dObjectList towrite;
    towrite.push_back( this );
    a2dObjectList::iterator iter = towrite.begin();
    while ( towrite.size() )
    {
        a2dObject* obj = *iter;
        obj->Save( this, out, &towrite );
        towrite.erase( iter );
        iter = towrite.begin();
    }

    out.WriteEndElement();
    out.WriteEndDocument();
}

void a2dObject::Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite )
{
    //only write once, if already written,
    if ( m_check == true )
    {
        //save this object as a reference to another by only writing one id attribute.
        out.WriteStartElementAttributes( wxT( "o" ) );
        out.WriteAttribute( wxT( "classname" ), GetClassInfo()->GetClassName() );
        out.WriteAttribute( wxT( "refid" ), m_id );
        out.WriteEndAttributes();
        out.WriteEndElement();
        return;
    }

    if( IsTemporary_DontSave() )
        return;

    out.WriteStartElementAttributes( wxT( "o" ) );
    out.WriteAttribute( wxT( "classname" ), GetClassInfo()->GetClassName() );
    if( m_refcount > 1 || AlwaysWriteSerializationId() )
        out.WriteAttribute( wxT( "id" ), m_id );
    //the default name is same as address of object, there is no need to save that.
    wxString idstr;
    idstr << m_id;

    if ( GetName() != idstr )
    {
        const a2dNamedProperty* p = FindProperty( PROPID_Name );
        if ( !p )
            out.WriteAttributeEnt( wxT( "name" ), GetName() );
    }

    DoSave( parent, out, a2dXmlSer_attrib, towrite );

    out.WriteEndAttributes();

    DoSave( parent, out, a2dXmlSer_Content, towrite );

    out.WriteEndElement();
    m_check = true;
}

void a2dObject::LoadFromDocument( a2dIOHandlerXmlSerIn& parser )
{
    parser.Next();
    parser.Require( START_TAG, wxT( "cvg" ) );
    parser.Next();

    Load( this, parser );

    parser.Require( END_TAG, wxT( "cvg" ) );
}

void a2dObject::Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser )
{
    parser.Require( START_TAG, wxT( "o" ) );
    parser.SetLastObjectLoadedId( wxT( "" ) );

    if ( parser.HasAttribute( wxT( "refid" ) ) )
    {
        // If the object has a refid, preserve refid in
        wxLongLong refId = parser.GetAttributeValueLongLong( wxT( "refid" ) );

        if ( refId != 0 )
        {
            wxString resolveKey;
            resolveKey << refId;
            parser.SetLastObjectLoadedId( resolveKey );
        }
        m_check = true;

        /*      If this is needed again for some reason, that's wrong, only a reference id should be enough.
                So think twice:
                <o classname="a2dArc" refid="4660970"> </o>

                DoLoad( parent, parser, a2dXmlSer_attrib );
                parser.Next();

                DoLoad( parent, parser, a2dXmlSer_Content );
        */
        parser.Next();
        parser.Require( END_TAG, wxT( "o" ) );
        parser.Next();

        return;
    }

    if ( parser.HasAttribute( wxT( "id" ) ) )
    {
        // If the object has an id, insert the object into the object hash map
        wxLongLong id = parser.GetAttributeValueLongLong( wxT( "id" ) );
        wxASSERT( id != 0 );
		m_id = id;
        wxString resolveKey;
        resolveKey << id;
        parser.GetObjectHashMap()[resolveKey] = this;
    }

    DoLoad( parent, parser, a2dXmlSer_attrib );
    parser.Next();

    DoLoad( parent, parser, a2dXmlSer_Content );

    parser.Require( END_TAG, wxT( "o" ) );
    parser.Next();
}
#endif //wxART2D_USE_CVGIO

bool a2dObject::AlwaysWriteSerializationId() const
{
    return false;
}

bool a2dObject::LinkReference( a2dObject* WXUNUSED( other ) )
{
    // This should only happen, if the class has put a a2dResolveIdInfo in the list
    wxASSERT( 0 );
    return false;
}

void a2dObject::ResolveInform(  )
{
    // This should only happen, if the class has put a a2dResolveIdInfo in the list
    wxASSERT( 0 );
}

void a2dObject::Walker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    if ( handler.GetUseCheck() && GetCheck() )
        return;

    handler.IncCurrentDepth();

    handler.WalkTask( parent, this, a2dWalker_a2dObjectStart );

    DoWalker( parent, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dObjectEnd );
    handler.DecCurrentDepth();
}

void a2dObject::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dPropertyStart );

    if ( !handler.GetStopWalking() )
    {
        m_propertylist.Walker( this, handler );
    }
    handler.WalkTask( parent, this, a2dWalker_a2dPropertyEnd );
}

bool a2dObject::EditProperties( const a2dPropertyId* id, bool withUndo )
{
    a2dNamedPropertyList allprops;

    CollectProperties2( &allprops, id, a2dPropertyId::flag_none );

    bool res = false;

    a2dPropertyEditEvent event( this, &allprops );
    event.SetEventObject( this );

    // first try the object itself, this makes object specific property editing possible.
    ProcessEvent( event );

    if ( event.GetEdited() && !allprops.empty() )
    {
        res = true;
        a2dNamedPropertyList::iterator iter;
        for( iter = allprops.begin(); iter != allprops.end(); ++iter )
        {
            a2dNamedProperty* prop = *iter;
            prop->SetToObject( this );
        }
    }

    return res;
}

void a2dObject::TakeOverProperties( a2dObject* from, a2dPropertyIdList* listOfIds )
{
    if ( !listOfIds )
        return;

    forEachIn( a2dPropertyIdList, listOfIds )
    {
        a2dPropertyId* id = *iter;
        a2dNamedProperty* property = from->GetProperty( id );
        if ( property )
            property->SetToObject( this );
    }
}

bool a2dObject::AddProperty( a2dNamedProperty* property )
{
    m_propertylist.push_back( property );
    return true;
}

a2dNamedProperty* a2dObject::FindProperty( const a2dPropertyId* id, a2dPropertyId::Flags flags )
{
    a2dNamedPropertyList::iterator iter;
    for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
    {
        a2dNamedProperty* prop = *iter;
        if (  id->CheckCollect( prop->GetId(), flags ) )
            return prop;
    }

    return NULL;
}

const a2dNamedProperty* a2dObject::FindProperty( const a2dPropertyId* id, a2dPropertyId::Flags flags ) const
{
    a2dNamedPropertyList::const_iterator iter;
    for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
    {
        const a2dNamedProperty* prop = *iter;
        if (  id->CheckCollect( prop->GetId(), flags ) )
            return prop;
    }

    return NULL;
}

bool a2dObject::RemoveProperty( const a2dPropertyId* id, bool all )
{
    if( m_propertylist.Remove( id, all ) )
    {
        OnPropertyChanged( id );
        return true;
    }
    else
        return false;
}

void a2dObject::RemoveTemporaryProperties( )
{
    m_propertylist.RemoveTemporary();
}


a2dNamedProperty* a2dObject::GetProperty( const a2dPropertyId* propertyId, a2dPropertyId::Flags flags ) const
{
    static a2dNamedPropertyPtr returnproperty = NULL;

    // now assemble properties with this id, which is normally just one.
    a2dNamedPropertyList props;
    CollectProperties2( &props, propertyId, flags );
    // normally just one found with this specific Id.
    if( props.begin() != props.end() )
        returnproperty = *props.begin();
    else
        returnproperty = NULL;
    return returnproperty;
}

void a2dObject::SetProperty( a2dNamedProperty* propertyHolder, a2dPropertyId::SetFlags setflags )
{
    wxASSERT( !m_recursion_active );

    propertyHolder->GetId()->SetNamedPropertyToObject( this, propertyHolder );
    OnPropertyChanged( propertyHolder->GetId() );
}

void a2dObject::SetProperty( const a2dNamedProperty& propertyHolder, a2dPropertyId::SetFlags setflags )
{
    wxASSERT( !m_recursion_active );

    propertyHolder.GetId()->SetNamedPropertyToObject( this, const_cast<a2dNamedProperty*>( &propertyHolder ) );
    OnPropertyChanged( propertyHolder.GetId() );
}

bool a2dObject::HasProperty( const a2dPropertyId* id, const wxString& stringvalue ) const
{
    if ( ! HasPropertyId( id ) )
        return false;

    bool ret = false;
    a2dNamedPropertyPtr prop = GetProperty( id );
    if ( prop && ! stringvalue.IsEmpty() )
        ret = prop->StringValueRepresentation() == stringvalue;
    else
        ret = prop.Get() != NULL;
    return ret;
}

void a2dObject::OnPropertyChanged( const a2dPropertyId* id )
{
    if ( id->CheckFlags( a2dPropertyId::flag_notify ) )
    {
        a2dComEvent event( this, id, a2dComEvent::sm_changedProperty );
        event.SetEventObject( this );
        this->ProcessEvent( event );
    }
}

void a2dObject::SetURI( const wxURI& url )
{
    PROPID_URI->SetPropertyToObject( this, url );
}

wxURI a2dObject::GetURI() const
{
    return PROPID_URI->GetPropertyValue( this );
}

#if wxART2D_USE_CVGIO

void a2dObject::DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        if ( !m_propertylist.empty() )
        {
			bool onlyTemp = true;
            a2dNamedPropertyList::iterator iter;
            for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
            {
                a2dNamedProperty* prop = *iter;
                if( !prop->GetId()->IsTemporary() )
					onlyTemp = false;
            }

			if ( !onlyTemp )
			{
				out.WriteNewLine();
				out.WriteStartElement( wxT( "properties" ) );

				
				a2dNamedPropertyList::iterator iter;
				for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
				{
					a2dNamedProperty* prop = *iter;
					// Don't store temporary properties
					if( !prop->GetId()->IsTemporary() )
					{
                        if ( prop->NeedsSave() )
						    prop->Save( this, out, towrite );
					}
				}

				out.WriteEndElement();
				out.WriteNewLine();
			}		
        }
    }
}

void a2dObject::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        if ( parser.GetTagName() == wxT( "properties" ) )
        {
            parser.Next();
            while( parser.GetTagName() == wxT( "o" ) )
            {
                wxString classname = parser.GetAttributeValue( wxT( "classname" ) );
                wxObject* obj = parser.CreateObject( classname );
                a2dNamedPropertyPtr property = wxDynamicCast( obj, a2dNamedProperty );
                if ( !property )
                {
                    a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "could not create a2dNamedProperty %s, will be skipped line %d" ),
                                                     classname.c_str(), parser.GetCurrentLineNumber() );
                    parser.SkipSubTree();
                    parser.Require( END_TAG, wxT( "o" ) );
                    parser.Next();
                }
                else
                {
                    property->Load( this, parser );
                    property->SetToObject( this );
                }
            }

            parser.Require( END_TAG, wxT( "properties" ) );
            parser.Next();
        }
    }
}

#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dNamedProperty
//----------------------------------------------------------------------------
#ifdef _DEBUG
static int _debug_cnt;
#endif

#ifdef CLASS_MEM_MANAGEMENT
a2dMemManager a2dNamedProperty::sm_memManager( wxT( "a2dNamedProperty memory manager" ) );
#endif //CLASS_MEM_MANAGEMENT

a2dNamedProperty::a2dNamedProperty()
    : m_refcount( 0 )
{
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_ownerlist = NULL;
#endif

    m_id = 0;

#ifdef _DEBUG
    _debug_id = _debug_cnt++;
    //      wxLogDebug( wxT("%p=%d"), this, _debug_id );
#endif

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    CurrentSmartPointerOwner = this;
#endif
}

a2dNamedProperty::a2dNamedProperty( const a2dPropertyId* id )
    : m_refcount( 0 )
{
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_ownerlist = NULL;
#endif
    m_id = id;

#ifdef _DEBUG
    _debug_id = _debug_cnt++;
    //      wxLogDebug( wxT("%p=%d"), this, _debug_id );
#endif

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    CurrentSmartPointerOwner = this;
#endif
}

a2dNamedProperty::a2dNamedProperty( const a2dNamedProperty& other )
    :   wxObject( other ), m_refcount( 0 )
{
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_ownerlist = NULL;
#endif
    m_id = other.m_id;

#ifdef _DEBUG
    _debug_id = _debug_cnt++;
    //      wxLogDebug( wxT("%p=%d"), this, _debug_id );
#endif

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    CurrentSmartPointerOwner = this;
#endif
}

a2dNamedProperty::~a2dNamedProperty()
{
}

const a2dPropertyId* a2dNamedProperty::CreateId( a2dObject* parent, const wxString& name )
{
    a2dPropertyId* propid = parent->HasPropertyId( name );
    if ( !propid )
    {
        wxString error;
        error.Printf( wxT( "Property with name: %s has no internal a2dPropertyId, binding this name to an internal property" ), name.c_str() );
        a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, error );
        //wxLogWarning(wxT("CVG : property type: %s is not implemented"), classname.c_str() );
    }
    return propid;
}

a2dNamedProperty* a2dNamedProperty::Clone( a2dObject::CloneOptions options, a2dRefMap* refs ) const 
{ 
	/*
	if ( m_refcount > 1 )
    {
        int id = 1;
        wxASSERT( id != 0 );
        wxString resolveKey;
        resolveKey << id;
        parser.GetObjectHashMap()[resolveKey] = this;
    }
	*/

	return DoClone( options, refs ); 
}

#if wxART2D_USE_CVGIO
void a2dNamedProperty::Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite )
{
    if( m_id->IsTemporary() )
        return;

    out.WriteStartElementAttributes( wxT( "o" ) );
    out.WriteAttribute( wxT( "classname" ), GetClassInfo()->GetClassName() );
    out.WriteAttribute( wxT( "name" ), m_id->GetName() );

    DoSave( parent, out, a2dXmlSer_attrib, towrite );

    out.WriteEndAttributes();

    DoSave( parent, out, a2dXmlSer_Content, towrite );

    out.WriteEndElement();
}


void a2dNamedProperty::Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser )
{
    parser.Require( START_TAG, wxT( "o" ) );

    wxString classname;
    classname = parser.RequireAttributeValue( wxT( "classname" ) );

    wxString name;
    name = parser.RequireAttributeValue( wxT( "name" ) );
    a2dObject* propparent = wxStaticCast( parent, a2dObject );

    if ( propparent )
    {
        const a2dPropertyId* id = CreateId( propparent, name );
        if ( !id )
        {
            parser.SetLastError( a2dGeneralGlobals->GetLastError() );
            //wxLogWarning(wxT("CVG : property type: %s is not implemented"), classname.c_str() );
        }
    }

    DoLoad( parent, parser, a2dXmlSer_attrib );

    parser.Next();

    DoLoad( parent, parser, a2dXmlSer_Content );

    parser.Require( END_TAG, wxT( "o" ) );
    parser.Next();
}
#endif //wxART2D_USE_CVGIO

#if wxART2D_USE_CVGIO
void a2dNamedProperty::DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) )
{
}

void a2dNamedProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString name  = parser.RequireAttributeValue( wxT( "name" ) );
        m_id = wxStaticCast( parent, a2dObject )->HasPropertyId( name );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO


void a2dNamedProperty::Walker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dNamedPropertyStart );
    handler.IncCurrentDepth();

    DoWalker( parent, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dNamedPropertyEnd );
    handler.DecCurrentDepth();
}

void a2dNamedProperty::DoWalker( wxObject* parent, a2dWalkerIOHandler& WXUNUSED( handler ) )
{
}

wxString a2dNamedProperty::GetString() const
{
    wxASSERT_MSG( 0, wxT( "Not derived from a2dStringProperty" ) );
    return wxT( "" );
}

double a2dNamedProperty::GetDouble() const
{
    wxASSERT_MSG( 0, wxT( "Not derived from a2dDoubleProperty" ) );
    return 0.0;
}

float a2dNamedProperty::GetFloat() const
{
    wxASSERT_MSG( 0, wxT( "Not derived from a2dFloatProperty" ) );
    return 0.0;
}

bool a2dNamedProperty::GetBool() const
{
    wxASSERT_MSG( 0, wxT( "Not derived from a2dBoolProperty" ) );
    return false;
}

wxInt16 a2dNamedProperty::GetInt16() const
{
    wxASSERT_MSG( 0, wxT( "Not derived from a2dInt16Property" ) );
    return 0;
}

wxUint16 a2dNamedProperty::GetUint16() const
{
    wxASSERT_MSG( 0, wxT( "Not derived from a2dUint16Property" ) );
    return 0;
}

wxInt32 a2dNamedProperty::GetInt32() const
{
    wxASSERT_MSG( 0, wxT( "Not derived from a2dInt32Property" ) );
    return 0;
}

wxUint32 a2dNamedProperty::GetUint32() const
{
    wxASSERT_MSG( 0, wxT( "Not derived from a2dUint32Property" ) );
    return 0;
}

void* a2dNamedProperty::GetVoidPtr() const
{
    wxASSERT_MSG( 0, wxT( "Not derived from a2dVoidPtrProperty" ) );
    return 0;
}

a2dObject* a2dNamedProperty::GetRefObject() const
{
    wxASSERT_MSG( 0, wxT( "Not derived from a2dProperty" ) );
    return 0;
}

a2dObject* a2dNamedProperty::GetRefObjectNA() const
{
    return 0;
}

double a2dNamedProperty::ParseDouble( unsigned int& position )
{
    const wxChar* buffer = StringValueRepresentation().c_str();

    const wxChar* start = buffer + position;
    wxChar* end;
    double val = wxStrtod( start, &end );
    position = end - buffer;

    if ( start == end )
        a2dGeneralGlobals->ReportErrorF( a2dError_NotSpecified, _( "Not a number while parsing string %s" ), start );

    return val;
}

void a2dNamedProperty::SkipCommaSpaces( unsigned int& position )
{
    char current;
    for( ;; )
    {
        current = StringValueRepresentation()[position];
        switch ( current )
        {
            default:
                return;
            case wxT( ',' ): case 0x20: case 0x09: case 0x0D: case 0x0A:
                position++;
                break;
        }
    }
}

void a2dNamedProperty::SkipSpaces( unsigned int& position )
{
    char current;
    for( ;; )
    {
        current = StringValueRepresentation()[position];
        switch ( current )
        {
            default:
                return;
            case 0x20: case 0x09: case 0x0D: case 0x0A:
                position++;
                break;
        }
    }
}

//----------------------------------------------------------------------------
// a2dNamedPropertyList
//----------------------------------------------------------------------------

a2dNamedPropertyList::a2dNamedPropertyList( const a2dNamedPropertyList& other )
{
    // Copy the list of property object

    a2dNamedPropertyList::const_iterator iter;
    for( iter = other.begin(); iter != other.end(); ++iter )
    {
        a2dNamedProperty* obj = *iter;
        push_back( obj->Clone( a2dObject::clone_flat ) );

    }
}

a2dNamedPropertyList::~a2dNamedPropertyList()
{
}

bool a2dNamedPropertyList::Remove( const a2dPropertyId* id, bool all )
{
    bool did = false;

    a2dNamedPropertyList::iterator iter = begin();
    while( iter != end() )
    {
        a2dNamedProperty* prop = *iter;
        if ( prop->GetId() == id )
        {
            iter = erase( iter );
            if ( !all )
            {
                return true;
            }
            else
                did = true;
        }
        else
            iter++;
    }
    return did;
}

void a2dNamedPropertyList::RemoveTemporary()
{
    a2dNamedPropertyList::iterator iter = begin();
    while( iter != end() )
    {
        a2dNamedProperty* prop = *iter;
        if ( prop->GetId()->IsTemporary() )
        {
            iter = erase( iter );
        }
        else
            iter++;
    }
}

void a2dNamedPropertyList::Walker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, parent, a2dWalker_a2dNamedPropertyListStart );

    a2dNamedPropertyList::iterator iter = begin();
    iter = begin();
    while( iter != end() )
    {
        a2dNamedProperty* prop = *iter;
        prop->Walker( parent, handler );
        iter++;
    }

    handler.WalkTask( parent, parent, a2dWalker_a2dNamedPropertyListEnd );
}

//----------------------------------------------------------------------------
// a2dBoolProperty
//----------------------------------------------------------------------------

a2dBoolProperty::a2dBoolProperty(): a2dNamedProperty()
{
    m_value = true;
}

a2dBoolProperty::a2dBoolProperty( const a2dPropertyIdBool* id, bool value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dBoolProperty::~a2dBoolProperty()
{
}

a2dBoolProperty::a2dBoolProperty( const a2dBoolProperty& other, a2dObject::CloneOptions WXUNUSED( options ) )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dBoolProperty::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dBoolProperty( *this, options );
};

void a2dBoolProperty::Assign( const a2dNamedProperty& other )
{
    a2dBoolProperty* propcast = wxStaticCast( &other, a2dBoolProperty );
    m_value = propcast->m_value;
}

a2dBoolProperty* a2dBoolProperty::CreatePropertyFromString( const a2dPropertyIdBool* id, const wxString& value )
{
    // Remove leading/trailing spaces and convert to lowercase.
    wxString lower = value.Lower().Trim( true ).Trim( false );
    // Notice that _("yes") is different from wxT("yes"),since it allows for i18n, so it could read as wxT("ja").
    bool val = ( lower == wxT( "true" ) ) || ( lower == wxT( "yes" ) ) || ( lower == wxT( "y" ) ) || ( lower == wxT( "1" ) ) || ( lower == _( "yes" ) );

    return new a2dBoolProperty( id, val );
}


const a2dPropertyId* a2dBoolProperty::CreateId( a2dObject* parent, const wxString& name )
{
    a2dPropertyIdBool* propid = ( a2dPropertyIdBool* ) parent->HasPropertyId( name );
    if ( !propid )
    {
        propid = new a2dPropertyIdBool( name, false, a2dPropertyId::flag_userDefined );
        parent->AddPropertyId( propid );
    }
    return propid;
}

#if wxART2D_USE_CVGIO
void a2dBoolProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "value" ), m_value );
    }
    else
    {
    }
}

void a2dBoolProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_value  = parser.GetAttributeValueBool( wxT( "value" ), true );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dBoolProperty::SetValue( bool value )
{
    m_value = value;
}

wxString a2dBoolProperty::StringRepresentation() const
{
    return m_value == true ? GetName() + wxT( "true" ) : GetName() + wxT( "false" );
}

wxString a2dBoolProperty::StringValueRepresentation() const
{
    return m_value == true ? wxT( "true" ) : wxT( "false" );
}

//----------------------------------------------------------------------------
// a2dStringProperty
//----------------------------------------------------------------------------

a2dStringProperty::a2dStringProperty(): a2dNamedProperty()
{
    m_value = wxT( "" );
}

a2dStringProperty::a2dStringProperty( const a2dPropertyIdString* id, const wxString& value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dStringProperty::~a2dStringProperty()
{
}

a2dStringProperty::a2dStringProperty( const a2dStringProperty& other, a2dObject::CloneOptions WXUNUSED( options ) )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dStringProperty::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dStringProperty( *this, options );
};

void a2dStringProperty::Assign( const a2dNamedProperty& other )
{
    a2dStringProperty* propcast = wxStaticCast( &other, a2dStringProperty );
    m_value = propcast->m_value;
}

a2dStringProperty* a2dStringProperty::CreatePropertyFromString( const a2dPropertyIdString* id, const wxString& value )
{
    return new a2dStringProperty( id, value );
}

#if wxART2D_USE_CVGIO
void a2dStringProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttributeEnt( "value", m_value );
    }
    else
    {
    }
}

void a2dStringProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_value  = parser.GetAttributeValue( wxT( "value" ), wxT( "" ) );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dStringProperty::SetValue( const wxString& value )
{
    m_value = value;
}

const a2dPropertyId* a2dStringProperty::CreateId( a2dObject* parent, const wxString& name )
{
    a2dPropertyIdString* propid = ( a2dPropertyIdString* ) parent->HasPropertyId( name );
    if ( !propid )
    {
        propid = new a2dPropertyIdString( name, wxT( "" ), a2dPropertyId::flag_userDefined );
        parent->AddPropertyId( propid );
    }
    return propid;
}

//----------------------------------------------------------------------------
// a2dObjectProperty
//----------------------------------------------------------------------------

a2dObjectProperty::a2dObjectProperty(): a2dNamedProperty()
{
}

a2dObjectProperty::a2dObjectProperty( const a2dPropertyIdObject* id, const wxObject& value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dObjectProperty::~a2dObjectProperty()
{
}

a2dObjectProperty::a2dObjectProperty( const a2dObjectProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dObjectProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dObjectProperty( *this );
};

void a2dObjectProperty::Assign( const a2dNamedProperty& other )
{
    a2dObjectProperty* propcast = wxStaticCast( &other, a2dObjectProperty );
    m_value = propcast->m_value;
}

void a2dObjectProperty::SetValue( const wxObject& value )
{
    m_value = value;
}

//----------------------------------------------------------------------------
// a2dVoidPtrProperty
//----------------------------------------------------------------------------

a2dVoidPtrProperty::a2dVoidPtrProperty(): a2dNamedProperty()
{
    m_value = NULL;
}

a2dVoidPtrProperty::a2dVoidPtrProperty( const a2dPropertyIdVoidPtr* id, void* value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dVoidPtrProperty::~a2dVoidPtrProperty()
{
}

a2dVoidPtrProperty::a2dVoidPtrProperty( const a2dVoidPtrProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dVoidPtrProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dVoidPtrProperty( *this );
};

void a2dVoidPtrProperty::Assign( const a2dNamedProperty& other )
{
    a2dVoidPtrProperty* propcast = wxStaticCast( &other, a2dVoidPtrProperty );
    m_value = propcast->m_value;
}

a2dVoidPtrProperty* a2dVoidPtrProperty::CreatePropertyFromString( const a2dPropertyIdVoidPtr* WXUNUSED( id ), const wxString& WXUNUSED( value ) )
{
    return 0;
}

#if wxART2D_USE_CVGIO
void a2dVoidPtrProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "value" ), wxT( "VOID" ) );
    }
    else
    {
    }
}

void a2dVoidPtrProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_value  = NULL;
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dVoidPtrProperty::SetValue( void* value )
{
    m_value = value;
}

wxString a2dVoidPtrProperty::StringRepresentation() const
{
    return m_value == 0 ?  GetName() + wxT( "NULL" ) : GetName() + wxT( "VOID" );
}

wxString a2dVoidPtrProperty::StringValueRepresentation() const
{
    return m_value == 0 ? wxT( "NULL" ) : wxT( "VOID" );
}

//----------------------------------------------------------------------------
// a2dProperty
//----------------------------------------------------------------------------

a2dProperty::a2dProperty(): a2dNamedProperty()
{
    m_value = NULL;
}

a2dProperty::a2dProperty( const a2dPropertyIdRefObject* id, a2dObject* value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dProperty::~a2dProperty()
{
}

a2dProperty::a2dProperty( const a2dProperty& other, a2dObject::CloneOptions options )
    : a2dNamedProperty( other )
{
    if ( options & a2dObject::clone_properties && m_id->IsCloneDeep() )
    {
        if ( other.m_value )
            m_value = other.m_value->Clone( options );
    }
    else
        m_value = other.m_value;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in Clone ( )
    CurrentSmartPointerOwner = this;
#endif
}

a2dNamedProperty* a2dProperty::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dProperty( *this, options );
};

void a2dProperty::Assign( const a2dNamedProperty& other )
{
    a2dProperty* propcast = wxStaticCast( &other, a2dProperty );
    m_value = propcast->m_value;
}

a2dProperty* a2dProperty::CreatePropertyFromString( const a2dPropertyIdRefObject* WXUNUSED( id ), const wxString& WXUNUSED( value ) )
{
    return 0;
}

#if wxART2D_USE_CVGIO
void a2dProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        if ( m_value )
            m_value->Save( parent, out, towrite  );
    }
}

void a2dProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_value  = NULL;
    }
    else
    {
        if ( parser.GetTagName() == wxT( "o" )  && parser.GetEventType() != END_TAG )
        {
            parser.Require( START_TAG, wxT( "o" ) );

            wxString classname = parser.GetAttributeValue( wxT( "classname" ) );
            m_value = wxDynamicCast( parser.CreateObject( classname ), a2dObject );
            if ( !m_value )
            {
                a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "could not create a2dObject %s, will be skipped line %d" ),
                                                 classname.c_str(), parser.GetCurrentLineNumber() );
                parser.SetLastError( a2dGeneralGlobals->GetLastError() );

                parser.SkipSubTree();
                parser.Require( END_TAG, wxT( "o" ) );
                parser.Next();
            }
            else
            {
                m_value->Load( this, parser );
                parser.ResolveOrAdd( &m_value );
            }

        }
    }
}
#endif //wxART2D_USE_CVGIO

void a2dProperty::SetValue( a2dObject* value )
{
    m_value = value;
}

wxString a2dProperty::StringRepresentation() const
{
    return m_value ?  GetName() + wxT( "NULL" ) : GetName() + wxT( "RefObject" );
}

wxString a2dProperty::StringValueRepresentation() const
{
    return m_value ? wxT( "NULL" ) : wxT( "RefObject" );
}

//----------------------------------------------------------------------------
// a2dAutoZeroProperty
//----------------------------------------------------------------------------

a2dAutoZeroProperty::a2dAutoZeroProperty(): a2dNamedProperty()
{
    m_value = NULL;
}

a2dAutoZeroProperty::a2dAutoZeroProperty( const a2dPropertyIdRefObjectAutoZero* id, a2dObject* value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dAutoZeroProperty::~a2dAutoZeroProperty()
{
}

a2dAutoZeroProperty::a2dAutoZeroProperty( const a2dAutoZeroProperty& other, a2dObject::CloneOptions options )
    : a2dNamedProperty( other )
{
    if ( options & a2dObject::clone_properties && m_id->IsCloneDeep() )
    {
        if ( other.m_value )
            m_value = other.m_value->Clone( options );
    }
    else
        m_value = other.m_value;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in Clone ( )
    CurrentSmartPointerOwner = this;
#endif
}

a2dNamedProperty* a2dAutoZeroProperty::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dAutoZeroProperty( *this, options );
};

void a2dAutoZeroProperty::Assign( const a2dNamedProperty& other )
{
    a2dAutoZeroProperty* propcast = wxStaticCast( &other, a2dAutoZeroProperty );
    m_value = propcast->m_value;
}

a2dAutoZeroProperty* a2dAutoZeroProperty::CreatePropertyFromString( const a2dPropertyIdRefObjectAutoZero* WXUNUSED( id ), const wxString& WXUNUSED( value ) )
{
    return 0;
}

#if wxART2D_USE_CVGIO
void a2dAutoZeroProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        if ( m_value )
            m_value->Save( parent, out, towrite  );
    }
}

void a2dAutoZeroProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_value  = NULL;
    }
    else
    {
        if ( parser.GetTagName() == wxT( "o" )  && parser.GetEventType() != END_TAG )
        {
            parser.Require( START_TAG, wxT( "o" ) );

            wxString classname = parser.GetAttributeValue( wxT( "classname" ) );
            m_value = wxStaticCast( parser.CreateObject( classname ), a2dObject );
            if ( !m_value )
            {
                a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "could not create a2dObject %s, will be skipped line %d" ),
                                                 classname.c_str(), parser.GetCurrentLineNumber() );
                parser.SkipSubTree();
                parser.Require( END_TAG, wxT( "o" ) );
                parser.Next();
            }
            else
            {
                m_value->Load( this, parser );
                parser.ResolveOrAdd( &m_value );
            }

        }
    }
}
#endif //wxART2D_USE_CVGIO

void a2dAutoZeroProperty::SetValue( a2dObject* value )
{
    m_value = value;
}

wxString a2dAutoZeroProperty::StringRepresentation() const
{
    return m_value ?  GetName() + wxT( "NULL" ) : GetName() + wxT( "RefObject" );
}

wxString a2dAutoZeroProperty::StringValueRepresentation() const
{
    return m_value ? wxT( "NULL" ) : wxT( "RefObject" );
}

//----------------------------------------------------------------------------
// a2dInt16Property
//----------------------------------------------------------------------------

a2dInt16Property::a2dInt16Property(): a2dNamedProperty()
{
    m_value = 0;
}

a2dInt16Property::a2dInt16Property( const a2dPropertyIdInt16* id, wxInt16 value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dInt16Property::~a2dInt16Property()
{
}

a2dInt16Property::a2dInt16Property( const a2dInt16Property& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dInt16Property::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dInt16Property( *this );
};

void a2dInt16Property::Assign( const a2dNamedProperty& other )
{
    a2dInt16Property* propcast = wxStaticCast( &other, a2dInt16Property );
    m_value = propcast->m_value;
}

a2dInt16Property* a2dInt16Property::CreatePropertyFromString( const a2dPropertyIdInt16* id, const wxString& value )
{
    long intval;
    value.ToLong( &intval , 10 );
    return new a2dInt16Property( id, intval );
}

#if wxART2D_USE_CVGIO
void a2dInt16Property::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "value" ), m_value );
    }
    else
    {
    }
}

void a2dInt16Property::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_value  = parser. GetAttributeValueInt( wxT( "value" ), 0 );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dInt16Property::SetValue( wxInt16 value )
{
    m_value = value;
}

wxString a2dInt16Property::StringRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%s = %d" ), GetName().c_str(), m_value );
    return  form;
}

wxString a2dInt16Property::StringValueRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%d" ), m_value );
    return form;
}

//----------------------------------------------------------------------------
// a2dUint16Property
//----------------------------------------------------------------------------

a2dUint16Property::a2dUint16Property(): a2dNamedProperty()
{
    m_value = 0;
}

a2dUint16Property::a2dUint16Property( const a2dPropertyIdUint16* id, wxUint16 value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dUint16Property::~a2dUint16Property()
{
}

a2dUint16Property::a2dUint16Property( const a2dUint16Property& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dUint16Property::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dUint16Property( *this );
};

void a2dUint16Property::Assign( const a2dNamedProperty& other )
{
    a2dUint16Property* propcast = wxStaticCast( &other, a2dUint16Property );
    m_value = propcast->m_value;
}

a2dUint16Property* a2dUint16Property::CreatePropertyFromString( const a2dPropertyIdUint16* id, const wxString& value )
{
    unsigned long intval;
    value.ToULong( &intval , 10 );
    return new a2dUint16Property( id, intval );
}

#if wxART2D_USE_CVGIO
void a2dUint16Property::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "value" ), m_value );
    }
    else
    {
    }
}

void a2dUint16Property::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_value  = parser. GetAttributeValueInt( wxT( "value" ), 0 );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dUint16Property::SetValue( wxUint16 value )
{
    m_value = value;
}

wxString a2dUint16Property::StringRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%s = %d" ), GetName().c_str(), m_value );
    return  form;
}

wxString a2dUint16Property::StringValueRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%d" ), m_value );
    return form;
}

//----------------------------------------------------------------------------
// a2dInt32Property
//----------------------------------------------------------------------------

a2dInt32Property::a2dInt32Property(): a2dNamedProperty()
{
    m_value = 0;
}

a2dInt32Property::a2dInt32Property( const a2dPropertyIdInt32* id, wxInt32 value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dInt32Property::~a2dInt32Property()
{
}

a2dInt32Property::a2dInt32Property( const a2dInt32Property& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dInt32Property::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dInt32Property( *this );
};

void a2dInt32Property::Assign( const a2dNamedProperty& other )
{
    a2dInt32Property* propcast = wxStaticCast( &other, a2dInt32Property );
    m_value = propcast->m_value;
}

a2dInt32Property* a2dInt32Property::CreatePropertyFromString( const a2dPropertyIdInt32* id, const wxString& value )
{
    long intval;
    value.ToLong( &intval , 10 );
    return new a2dInt32Property( id, intval );
}

#if wxART2D_USE_CVGIO
void a2dInt32Property::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "value" ), m_value );
    }
    else
    {
    }
}

void a2dInt32Property::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_value  = parser. GetAttributeValueInt( wxT( "value" ), 0 );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dInt32Property::SetValue( wxInt32 value )
{
    m_value = value;
}

wxString a2dInt32Property::StringRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%s = %d" ), GetName().c_str(), m_value );
    return  form;
}

wxString a2dInt32Property::StringValueRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%d" ), m_value );
    return form;
}

const a2dPropertyId* a2dInt32Property::CreateId( a2dObject* parent, const wxString& name )
{
    a2dPropertyIdInt32* propid = ( a2dPropertyIdInt32* ) parent->HasPropertyId( name );
    if ( !propid )
    {
        propid = new a2dPropertyIdInt32( name, 0, a2dPropertyId::flag_userDefined );
        parent->AddPropertyId( propid );
    }
    return propid;
}

//----------------------------------------------------------------------------
// a2dUint32Property
//----------------------------------------------------------------------------

a2dUint32Property::a2dUint32Property(): a2dNamedProperty()
{
    m_value = 0;
}

a2dUint32Property::a2dUint32Property( const a2dPropertyIdUint32* id, wxUint32 value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dUint32Property::~a2dUint32Property()
{
}

a2dUint32Property::a2dUint32Property( const a2dUint32Property& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dUint32Property::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dUint32Property( *this );
};

void a2dUint32Property::Assign( const a2dNamedProperty& other )
{
    a2dUint32Property* propcast = wxStaticCast( &other, a2dUint32Property );
    m_value = propcast->m_value;
}

a2dUint32Property* a2dUint32Property::CreatePropertyFromString( const a2dPropertyIdUint32* id, const wxString& value )
{
    unsigned long intval;
    value.ToULong( &intval , 10 );
    return new a2dUint32Property( id, intval );
}

#if wxART2D_USE_CVGIO
void a2dUint32Property::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "value" ), m_value );
    }
    else
    {
    }
}

void a2dUint32Property::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_value  = parser. GetAttributeValueInt( wxT( "value" ), 0 );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dUint32Property::SetValue( wxUint32 value )
{
    m_value = value;
}

wxString a2dUint32Property::StringRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%s = %d" ), GetName().c_str(), m_value );
    return  form;
}

wxString a2dUint32Property::StringValueRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%d" ), m_value );
    return form;
}

//----------------------------------------------------------------------------
// a2dDoubleProperty
//----------------------------------------------------------------------------

a2dDoubleProperty::a2dDoubleProperty(): a2dNamedProperty()
{
    m_value = 0.0;
}

a2dDoubleProperty::a2dDoubleProperty( const a2dPropertyIdDouble* id, double value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dDoubleProperty::~a2dDoubleProperty()
{
}

a2dDoubleProperty::a2dDoubleProperty( const a2dDoubleProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dDoubleProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dDoubleProperty( *this );
};

const a2dPropertyId* a2dDoubleProperty::CreateId( a2dObject* parent, const wxString& name )
{
    a2dPropertyIdDouble* propid = ( a2dPropertyIdDouble* ) parent->HasPropertyId( name );
    if ( !propid )
    {
        propid = new a2dPropertyIdDouble( name, 0, a2dPropertyId::flag_userDefined );
        parent->AddPropertyId( propid );
    }
    return propid;
}

void a2dDoubleProperty::Assign( const a2dNamedProperty& other )
{
    a2dDoubleProperty* propcast = wxStaticCast( &other, a2dDoubleProperty );
    m_value = propcast->m_value;
}

a2dDoubleProperty* a2dDoubleProperty::CreatePropertyFromString( const a2dPropertyIdDouble* id, const wxString& value )
{
    double doubleval;
    value.ToDouble( &doubleval );
    return new a2dDoubleProperty( id, doubleval );
}

#if wxART2D_USE_CVGIO
void a2dDoubleProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "value" ), m_value );
    }
    else
    {
    }
}

void a2dDoubleProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_value  = parser. GetAttributeValueDouble( wxT( "value" ), 0 );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dDoubleProperty::SetValue( double value )
{
    m_value = value;
}

wxString a2dDoubleProperty::StringRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%s = %s" ), GetName().c_str(), wxString::FromCDouble( m_value ) );
    return form;
}

wxString a2dDoubleProperty::StringValueRepresentation() const
{
    wxString form = wxString::FromCDouble( m_value );
    return form;
}

//----------------------------------------------------------------------------
// a2dFloatProperty
//----------------------------------------------------------------------------

a2dFloatProperty::a2dFloatProperty(): a2dNamedProperty()
{
    m_value = 0.0;
}

a2dFloatProperty::a2dFloatProperty( const a2dPropertyIdFloat* id, float value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dFloatProperty::~a2dFloatProperty()
{
}

a2dFloatProperty::a2dFloatProperty( const a2dFloatProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dFloatProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dFloatProperty( *this );
};

void a2dFloatProperty::Assign( const a2dNamedProperty& other )
{
    a2dFloatProperty* propcast = wxStaticCast( &other, a2dFloatProperty );
    m_value = propcast->m_value;
}

a2dFloatProperty* a2dFloatProperty::CreatePropertyFromString( const a2dPropertyIdFloat* id, const wxString& value )
{
    double doubleval;
    value.ToDouble( &doubleval );
    return new a2dFloatProperty( id, ( float ) doubleval );
}

#if wxART2D_USE_CVGIO
void a2dFloatProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "value" ), m_value );
    }
    else
    {
    }
}

void a2dFloatProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_value  = parser. GetAttributeValueDouble( wxT( "value" ), 0 );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dFloatProperty::SetValue( float value )
{
    m_value = value;
}

wxString a2dFloatProperty::StringRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%s = %6.3f" ), GetName().c_str(), wxString::FromCDouble( m_value ) );
    return form;
}

wxString a2dFloatProperty::StringValueRepresentation() const
{
    wxString form = wxString::FromCDouble( m_value );
    return form;
}

//----------------------------------------------------------------------------
// a2dArrayStringProperty
//----------------------------------------------------------------------------

a2dArrayStringProperty::a2dArrayStringProperty(): a2dNamedProperty()
{
    m_value = wxArrayString();
}

a2dArrayStringProperty::a2dArrayStringProperty( const a2dPropertyIdArrayString* id, const wxArrayString& value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dArrayStringProperty::a2dArrayStringProperty( const a2dPropertyIdArrayString* id, const wxString& value )
    : a2dNamedProperty( id )
{
    wxStringTokenizer args( value, wxT( "," ) );
    while ( args.HasMoreTokens() )
    {
        m_value.Add( args.GetNextToken() );
    }
}

a2dArrayStringProperty::~a2dArrayStringProperty()
{
}

a2dArrayStringProperty::a2dArrayStringProperty( const a2dArrayStringProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dArrayStringProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dArrayStringProperty( *this );
};

void a2dArrayStringProperty::Assign( const a2dNamedProperty& other )
{
    a2dArrayStringProperty* propcast = wxStaticCast( &other, a2dArrayStringProperty );
    m_value = propcast->m_value;
}

a2dArrayStringProperty* a2dArrayStringProperty::CreatePropertyFromString( const a2dPropertyIdArrayString* id, const wxString& value )
{
    wxArrayString valuenew;
    wxStringTokenizer args( value );
    while ( args.HasMoreTokens() )
    {
        valuenew.Add( args.GetNextToken() );
    }
    return new a2dArrayStringProperty( id, valuenew );
}

#if wxART2D_USE_CVGIO
void a2dArrayStringProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString attrib;
        size_t j;
        for ( j = 0; j < m_value.GetCount(); j++ )
        {
            attrib += m_value.Item( j );
        }

        out.WriteAttribute( wxT( "value" ), attrib );
    }
    else
    {
    }
}

void a2dArrayStringProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxStringTokenizer args( parser.GetAttributeValue( wxT( "value" ) ), wxT( "," ) );
        while ( args.HasMoreTokens() )
        {
            m_value.Add( args.GetNextToken() );
        }
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dArrayStringProperty::SetValue( const wxArrayString& value )
{
    m_value = value;
}

void a2dArrayStringProperty::Add( const wxString& value )
{
    m_value.Add( value );
}

wxString a2dArrayStringProperty::StringRepresentation() const
{
    wxString attrib;
    size_t j;
    for ( j = 0; j < m_value.GetCount(); j++ )
    {
        attrib += m_value.Item( j );
    }

    wxString form;
    form.Printf( wxT( "%s = %s" ), GetName().c_str(), attrib.c_str() );
    return form;
}

wxString a2dArrayStringProperty::StringValueRepresentation() const
{
    wxString attrib;
    size_t j;
    for ( j = 0; j < m_value.GetCount(); j++ )
    {
        attrib += m_value.Item( j );
    }

    wxString form;
    form.Printf( wxT( "%s" ), attrib.c_str() );
    return form;
}

//----------------------------------------------------------------------------
// a2dFileNameProperty
//----------------------------------------------------------------------------

a2dFileNameProperty::a2dFileNameProperty(): a2dNamedProperty()
{
    m_filenameObject = wxT( "" );
}

a2dFileNameProperty::a2dFileNameProperty( const a2dPropertyIdFileName* id, const wxFileName& filename )
    : a2dNamedProperty( id )
{
    m_filenameObject = filename;
}

a2dFileNameProperty::~a2dFileNameProperty()
{
}

a2dFileNameProperty::a2dFileNameProperty( const a2dFileNameProperty& other )
    : a2dNamedProperty( other )
{
    m_filenameObject = other.m_filenameObject;
}

a2dNamedProperty* a2dFileNameProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dFileNameProperty( *this );
};

void a2dFileNameProperty::Assign( const a2dNamedProperty& other )
{
    a2dFileNameProperty* propcast = wxStaticCast( &other, a2dFileNameProperty );
    m_filenameObject = propcast->m_filenameObject;
}

a2dFileNameProperty* a2dFileNameProperty::CreatePropertyFromString( const a2dPropertyIdFileName* id, const wxString& value )
{
    wxFileName filenameval( value );
    return new a2dFileNameProperty( id, filenameval );
}

#if wxART2D_USE_CVGIO
void a2dFileNameProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        //!todo write complete filename object
        out.WriteAttribute( wxT( "fullfilename" ), m_filenameObject.GetFullPath() );
    }
    else
    {
    }
}

void a2dFileNameProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_filenameObject  = parser.GetAttributeValue( wxT( "fullfilename" ), wxT( "" ) );
    }
    else
    {
    }

    //!todo read complete filename object
}
#endif //wxART2D_USE_CVGIO

void a2dFileNameProperty::SetValue( const wxFileName& filenameObject )
{
    m_filenameObject = filenameObject;
}

void a2dFileNameProperty::SetFileName( const wxFileName& filenameObject )
{
    m_filenameObject = filenameObject;
}

wxString a2dFileNameProperty::StringRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%s = %s" ), GetName().c_str(), m_filenameObject.GetFullPath().c_str() );
    return form;
}

wxString a2dFileNameProperty::StringValueRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%s" ), m_filenameObject.GetFullPath().c_str() );
    return form;
}

//----------------------------------------------------------------------------
// a2dDateTimeProperty
//----------------------------------------------------------------------------

a2dDateTimeProperty::a2dDateTimeProperty(): a2dNamedProperty()
{
    m_datetimeObject = wxDefaultDateTime;
}

a2dDateTimeProperty::a2dDateTimeProperty( const a2dPropertyIdDateTime* id, const wxDateTime& datetime )
    : a2dNamedProperty( id )
{
    m_datetimeObject = datetime;
}

a2dDateTimeProperty::~a2dDateTimeProperty()
{
}

a2dDateTimeProperty::a2dDateTimeProperty( const a2dDateTimeProperty& other )
    : a2dNamedProperty( other )
{
    m_datetimeObject = other.m_datetimeObject;
}

a2dNamedProperty* a2dDateTimeProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dDateTimeProperty( *this );
};

void a2dDateTimeProperty::Assign( const a2dNamedProperty& other )
{
    a2dDateTimeProperty* propcast = wxStaticCast( &other, a2dDateTimeProperty );
    m_datetimeObject = propcast->m_datetimeObject;
}

a2dDateTimeProperty* a2dDateTimeProperty::CreatePropertyFromString( const a2dPropertyIdDateTime* id, const wxString& value )
{
    wxDateTime datetimeval;
    //datetimeval.ParseFormat( value, wxDefaultDateTimeFormat, NULL );
    datetimeval.ParseISOCombined( value );
    return new a2dDateTimeProperty( id, datetimeval );
}

#if wxART2D_USE_CVGIO
void a2dDateTimeProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        //!todo write complete datetime object
        if ( !m_datetimeObject.IsValid() )
            m_datetimeObject = wxDateTime::Now();
        out.WriteAttribute( wxT( "datetime" ), m_datetimeObject.FormatISOCombined()  );
    }
    else
    {
    }
}

void a2dDateTimeProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString dateTime = parser.GetAttributeValue( wxT( "datetime" ), wxT( "" ) );
        bool good = m_datetimeObject.ParseISOCombined( dateTime );
        if ( !good )
            good = m_datetimeObject.ParseFormat( dateTime, wxDefaultDateTimeFormat );
            if ( !good )
                m_datetimeObject = wxDateTime::Now();
    }
    else
    {
    }

    //!todo read complete datetime object
}
#endif //wxART2D_USE_CVGIO

void a2dDateTimeProperty::SetValue( const wxDateTime& datetimeObject )
{
    m_datetimeObject = datetimeObject;
}

void a2dDateTimeProperty::SetDateTime( const wxDateTime& datetimeObject )
{
    m_datetimeObject = datetimeObject;
}

wxString a2dDateTimeProperty::StringRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%s = %s" ), GetName().c_str(), m_datetimeObject.FormatISOCombined().c_str() );
    return form;
}

wxString a2dDateTimeProperty::StringValueRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%s" ), m_datetimeObject.FormatISOCombined().c_str() );
    return form;
}

//----------------------------------------------------------------------------
// a2dMenuProperty
//----------------------------------------------------------------------------

a2dMenuProperty::a2dMenuProperty(): a2dNamedProperty()
{
    m_menu = NULL;
}

a2dMenuProperty::a2dMenuProperty( const a2dPropertyIdMenu* id, wxMenu* menu )
    : a2dNamedProperty( id )
{
    m_menu = menu;
}

a2dMenuProperty::~a2dMenuProperty()
{
}

a2dMenuProperty::a2dMenuProperty( const a2dMenuProperty& other )
    : a2dNamedProperty( other )
{
    m_menu = other.m_menu;
}

a2dNamedProperty* a2dMenuProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dMenuProperty( *this );
};

void a2dMenuProperty::Assign( const a2dNamedProperty& other )
{
    a2dMenuProperty* propcast = wxStaticCast( &other, a2dMenuProperty );
    m_menu = propcast->m_menu;
}

#if wxART2D_USE_CVGIO
void a2dMenuProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dMenuProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }

    //!todo read complete menu object
}
#endif //wxART2D_USE_CVGIO

void a2dMenuProperty::SetValue( wxMenu* menu )
{
    m_menu = menu;
}

a2dMenuProperty* a2dMenuProperty::CreatePropertyFromString( const a2dPropertyIdMenu* id, const wxString& value )
{
    return NULL;
}

//----------------------------------------------------------------------------
// a2dWindowProperty
//----------------------------------------------------------------------------

a2dWindowProperty::a2dWindowProperty(): a2dNamedProperty()
{
    m_window = NULL;
}

a2dWindowProperty::a2dWindowProperty( const a2dPropertyIdWindow* id, wxWindow* window )
    : a2dNamedProperty( id )
{
    m_window = window;
}

a2dWindowProperty::~a2dWindowProperty()
{
    if( m_window )
        m_window->Close();
}

a2dWindowProperty::a2dWindowProperty( const a2dWindowProperty& other )
    : a2dNamedProperty( other )
{
    m_window = other.m_window;
}

a2dNamedProperty* a2dWindowProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dWindowProperty( *this );
};

void a2dWindowProperty::Assign( const a2dNamedProperty& other )
{
    a2dWindowProperty* propcast = wxStaticCast( &other, a2dWindowProperty );
    m_window = propcast->m_window;
}

#if wxART2D_USE_CVGIO
void a2dWindowProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dWindowProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }

    //!todo read complete window object
}
#endif //wxART2D_USE_CVGIO

void a2dWindowProperty::SetValue( wxWindow* window )
{
    m_window = window;
}

a2dWindowProperty* a2dWindowProperty::CreatePropertyFromString( const a2dPropertyIdWindow* id, const wxString& value )
{
    return NULL;
}

//----------------------------------------------------------------------------
// a2dWindowProperty
//----------------------------------------------------------------------------

a2dTipWindowProperty::a2dTipWindowProperty(): a2dWindowProperty()
{
    m_rectStay = wxRect( -20, -20, 40 , 40 );
}

a2dTipWindowProperty::a2dTipWindowProperty( const a2dPropertyIdWindow* id, wxTipWindow* window, const wxString& tipstring, bool useObjRect )
    : a2dWindowProperty( id, window )
{
    m_useObjRect = useObjRect;
    m_tipstring = tipstring;
}

a2dTipWindowProperty::a2dTipWindowProperty( const a2dPropertyIdWindow* id, wxTipWindow* window, const wxString& tipstring, const wxRect& rect )
    : a2dWindowProperty( id, window )
{
    m_rectStay = rect;
    m_useObjRect = false;
    m_tipstring = tipstring;
}

a2dTipWindowProperty::~a2dTipWindowProperty()
{
    if( m_window )
    {
        wxTipWindow* tipwindow = wxDynamicCast( m_window, wxTipWindow );
        if( tipwindow )
            tipwindow->SetTipWindowPtr( NULL );
    }
}

a2dTipWindowProperty::a2dTipWindowProperty( const a2dTipWindowProperty& other )
    : a2dWindowProperty( other )
{
    m_tipstring = other.m_tipstring;
}

a2dNamedProperty* a2dTipWindowProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dTipWindowProperty( *this );
};

void a2dTipWindowProperty::Assign( const a2dNamedProperty& other )
{
    a2dTipWindowProperty* propcast = wxStaticCast( &other, a2dTipWindowProperty );
    m_window = propcast->m_window;
    m_tipstring = propcast->m_tipstring;
}

#if wxART2D_USE_CVGIO
void a2dTipWindowProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dTipWindowProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }

    //!todo read complete window object
}
#endif //wxART2D_USE_CVGIO

void a2dTipWindowProperty::SetString( const wxString& tipstring )
{
    m_tipstring = tipstring;
}

a2dTipWindowProperty* a2dTipWindowProperty::CreatePropertyFromString( const a2dPropertyIdWindow* id, const wxString& value )
{
    return NULL;
}

//----------------------------------------------------------------------------
// a2dPathListProperty
//----------------------------------------------------------------------------

a2dPathListProperty::a2dPathListProperty(): a2dNamedProperty()
{
    m_pathlistObject = a2dPathList();
}

a2dPathListProperty::a2dPathListProperty( const a2dPropertyIdPathList* id, const a2dPathList& pathlist )
    : a2dNamedProperty( id )
{
    m_pathlistObject = pathlist;
}

a2dPathListProperty::~a2dPathListProperty()
{
}

a2dPathListProperty::a2dPathListProperty( const a2dPathListProperty& other )
    : a2dNamedProperty( other )
{
    m_pathlistObject = other.m_pathlistObject;
}

a2dNamedProperty* a2dPathListProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dPathListProperty( *this );
};

void a2dPathListProperty::Assign( const a2dNamedProperty& other )
{
    a2dPathListProperty* propcast = wxStaticCast( &other, a2dPathListProperty );
    m_pathlistObject = propcast->m_pathlistObject;
}

a2dPathListProperty* a2dPathListProperty::CreatePropertyFromString( const a2dPropertyIdPathList* id, const wxString& value )
{
    a2dPathList a = a2dPathList();
    a.Add( value );
    return new a2dPathListProperty( id, a );
}

#if wxART2D_USE_CVGIO
void a2dPathListProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        //!todo write complete filename object
        //out.WriteAttribute( wxT("fullfilename"), m_pathlistObject.GetFullPath() );
    }
    else
    {
    }
}

void a2dPathListProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        //!todo m_pathlistObject  = parser.GetAttributeValue( wxT("fullfilename"), wxT("") );
    }
    else
    {
    }

    //!todo read complete filename object
}
#endif //wxART2D_USE_CVGIO

void a2dPathListProperty::SetValue( const a2dPathList& pathlistObject )
{
    m_pathlistObject = pathlistObject;
}

void a2dPathListProperty::SetFileName( const a2dPathList& pathlistObject )
{
    m_pathlistObject = pathlistObject;
}

wxString a2dPathListProperty::StringRepresentation() const
{
    wxString form;
    //!todo form.Printf( wxT("%s = %s"), GetName().c_str(), m_pathlistObject.GetFullPath().c_str() );
    return form;
}

wxString a2dPathListProperty::StringValueRepresentation() const
{
    wxString form;
    //!todo form.Printf( wxT("%s"), m_pathlistObject.GetFullPath().c_str() );
    return form;
}

//----------------------------------------------------------------------------
// a2dColourProperty
//----------------------------------------------------------------------------

A2DGENERALDLLEXP wxColour ParseSvgColour( wxString value )
{
    wxColour colour;
    value.Trim( false ).Trim();
    if ( value.GetChar( 0 ) == _T( '#' ) )
    {
        colour = HexToColour( value.After( _T( '#' ) ) );
    }
    else if ( value.Mid( 0, 4 ).Lower() == _T( "rgb(" ) ) // Read values like RGB(0,0,0) / rgb(0,0,0)
    {
        long red, green, blue;
        value = value.Mid( 4 );
        value = value.BeforeLast( _T( ')' ) );

        wxStringTokenizer rgb( value, _T( "," ) );
        if ( rgb.CountTokens() != 3 )
            wxLogWarning( _( "Invalid RGB value" ) );
        if ( !rgb.GetNextToken().ToLong( &red ) )
            wxLogWarning( _( "Invalid RGB value" ) );
        if ( !rgb.GetNextToken().ToLong( &green ) )
            wxLogWarning( _( "Invalid RGB value" ) );
        if ( !rgb.GetNextToken().ToLong( &blue ) )
            wxLogWarning( _( "Invalid RGB value" ) );

        colour.Set( red, green, blue );
    }
    else
    {
        // if exactly 6 characters and all are 0..9,a..f, assume hex colour.
        bool ishex = ( value.Length() == 6 );
        for ( int i = 0; ishex && i < 6; i++ )
            ishex = ( wxIsxdigit( value.GetChar( i ) ) != 0 );
        if ( ishex )
            colour = HexToColour( value );
        else
        {
            // if three values separated by either comma, or space assume decimal rgb
            unsigned long red, green, blue;
            bool commavalue = true;
            wxStringTokenizer rgbcomma( value, _T( "," ), wxTOKEN_RET_EMPTY );
            if ( rgbcomma.CountTokens() != 3 )
                commavalue = false;
            if ( !rgbcomma.GetNextToken().ToULong( &red ) )
                commavalue = false;
            if ( !rgbcomma.GetNextToken().ToULong( &green ) )
                commavalue = false;
            if ( !rgbcomma.GetNextToken().ToULong( &blue ) )
                commavalue = false;

            if ( !commavalue )
            {
                commavalue = true;
                wxStringTokenizer rgbspace( value, _T( " " ) );
                if ( rgbspace.CountTokens() != 3 )
                    commavalue = false;
                if ( !rgbspace.GetNextToken().ToULong( &red ) )
                    commavalue = false;
                if ( !rgbspace.GetNextToken().ToULong( &green ) )
                    commavalue = false;
                if ( !rgbspace.GetNextToken().ToULong( &blue ) )
                    commavalue = false;
            }
            if ( commavalue )
                colour.Set( red, green, blue );
            else
            {
                // assume a colourname string.
                colour = wxTheColourDatabase->FindName( value );
            }
        }
    }
    return colour;
}

a2dColourProperty::a2dColourProperty(): a2dNamedProperty()
{
    m_colour = wxT( "" );
}

a2dColourProperty::a2dColourProperty( const a2dPropertyIdColour* id, const wxColour& colour )
    : a2dNamedProperty( id )
{
    m_colour = colour;
}

a2dColourProperty::~a2dColourProperty()
{
}

a2dColourProperty::a2dColourProperty( const a2dColourProperty& other )
    : a2dNamedProperty( other )
{
    m_colour = other.m_colour;
}

a2dNamedProperty* a2dColourProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dColourProperty( *this );
};

void a2dColourProperty::Assign( const a2dNamedProperty& other )
{
    a2dColourProperty* propcast = wxStaticCast( &other, a2dColourProperty );
    m_colour = propcast->m_colour;
}

a2dColourProperty* a2dColourProperty::CreatePropertyFromString( const a2dPropertyIdColour* id, const wxString& value )
{
    return new a2dColourProperty( id, ParseSvgColour( value )  );
}

#if wxART2D_USE_CVGIO
void a2dColourProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        //!todo write complete filename object
        wxString form;
        form.Printf( wxT( "%d %d %d" ),  m_colour.Red(), m_colour.Green(), m_colour.Blue() );
        out.WriteAttribute( wxT( "colour" ), form );
    }
    else
    {
    }
}

void a2dColourProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString colstr  = parser.GetAttributeValue( wxT( "colour" ), wxT( "" ) );
        m_colour = ParseSvgColour( colstr );
    }
    else
    {
    }

    //!todo read complete filename object
}
#endif //wxART2D_USE_CVGIO

void a2dColourProperty::SetValue( const wxColour& colour )
{
    m_colour = colour;
}

void a2dColourProperty::SetColour( const wxColour& colour )
{
    m_colour = colour;
}

wxString a2dColourProperty::StringRepresentation() const
{
    wxString form;
    form.Printf( wxT( "%s = {%d %d %d}" ), GetName().c_str(), m_colour.Red(), m_colour.Green(), m_colour.Blue() );

    return form;
}

wxString a2dColourProperty::StringValueRepresentation() const
{
    wxString form;
    form.Printf( wxT( "{%d %d %d}" ), m_colour.Red(), m_colour.Green(), m_colour.Blue() );
    return form;
}

//----------------------------------------------------------------------------
// a2dUriProperty
//----------------------------------------------------------------------------

a2dUriProperty::a2dUriProperty(): a2dNamedProperty()
{
}

a2dUriProperty::a2dUriProperty( const a2dPropertyIdUri* id, const wxURI& uri )
    : a2dNamedProperty( id )
{
    m_uri = uri;
}

a2dUriProperty::~a2dUriProperty()
{
}

a2dUriProperty::a2dUriProperty( const a2dUriProperty& other )
    : a2dNamedProperty( other )
{
    m_uri = other.m_uri;
}

a2dNamedProperty* a2dUriProperty::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    return new a2dUriProperty( *this );
};

void a2dUriProperty::Assign( const a2dNamedProperty& other )
{
    a2dUriProperty* propcast = wxStaticCast( &other, a2dUriProperty );
    m_uri = propcast->m_uri;
}

#if wxART2D_USE_CVGIO
void a2dUriProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        //!todo write complete filename object
        wxString uri = m_uri.BuildURI();
        out.WriteAttribute( wxT( "uri" ), uri );
    }
    else
    {
    }
}

void a2dUriProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString uri  = parser.GetAttributeValue( wxT( "uri" ), wxT( "" ) );
        m_uri = wxURI( uri );
    }
    else
    {
    }

    //!todo read complete uri object
}
#endif //wxART2D_USE_CVGIO

void a2dUriProperty::SetValue( const wxURI& uri )
{
    m_uri = uri;
}

a2dUriProperty* a2dUriProperty::CreatePropertyFromString( const a2dPropertyIdUri* id, const wxString& value )
{
    return new a2dUriProperty( id, wxURI( value )  );
}

//----------------------------------------------------------------------------
// a2dObjectList
//----------------------------------------------------------------------------
A2DGENERALDLLEXP_DATA( a2dObjectList* ) wxNullRefObjectList = 0;

//IMPLEMENT_CLASS(a2dObjectList, wxRefObjectBaseList)

a2dObjectList::a2dObjectList()
{
}

a2dObjectList::~a2dObjectList()
{
}

void a2dObjectList::MakeUnique()
{
    for( a2dObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        a2dObject* obj = *iter;
        if ( obj && obj->GetOwnedBy() > 1 )
        {
            *iter = obj->Clone( a2dObject::clone_flat );
        }
    }
}

a2dObjectList* a2dObjectList::Clone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{

    if ( this == wxNullRefObjectList )
        return wxNullRefObjectList;
    else
        return new a2dObjectList( *this );
}

bool a2dObjectList::Release( const wxString& classname, bool all )
{
    bool did = false;

    a2dObjectList::iterator iter = begin();
    while ( iter != end() )
    {
        a2dObject* obj = ( *iter );
        if ( classname.IsEmpty() || obj->GetClassInfo()->GetClassName() == classname )
        {
            did = true;
            iter = erase( iter );
            if ( !all )
                return did;
        }
        else
            iter++;
    }

    return did;
}

a2dObject* a2dObjectList::Find( const wxString& classname, const wxString& name )
{
    for( a2dObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        a2dObject* cobj = *iter;

        if (
            ( classname.IsEmpty() || cobj->GetClassInfo()->GetClassName() == classname ) &&
            ( name.IsEmpty() || cobj->GetName() == name )
        )
        {
            return cobj;
        }
    }

    return ( a2dObject* ) NULL;
}

int a2dObjectList::CollectObjects( a2dObjectList* total, const wxString& propertyNameFilter, const wxString& classname )
{
    int count = 0;
    for( a2dObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        a2dObject* cobj = *iter;

        if ( ( classname.IsEmpty() || cobj->GetClassInfo()->GetClassName() == classname ) &&
                ( propertyNameFilter.IsEmpty() || ( cobj->GetName().Matches( propertyNameFilter ) ) )
           )
        {
            if ( total && total != wxNullRefObjectList )
                total->push_back( cobj );
            count++;
        }
    }

    return count;
}

//----------------------------------------------------------------------------
// a2dVariablesHash
//----------------------------------------------------------------------------

a2dVariablesHash::a2dVariablesHash()
{
}

a2dVariablesHash::a2dVariablesHash( const a2dVariablesHash& other )
    :
    a2dVariablesHashBase( other )
{
    // all objects in here need to be cloned
    iterator i;
    for( i = begin(); i != end(); ++i )
    {
        i->second = i->second->Clone( a2dObject::clone_flat );
    }
}

void a2dVariablesHash::operator = ( const a2dVariablesHash& other )
{
    a2dVariablesHashBase::operator = ( other );
    // all objects in here need to be cloned
    iterator i;
    for( i = begin(); i != end(); ++i )
    {
        i->second = i->second->Clone( a2dObject::clone_flat );
    }
}

a2dVariablesHash::~a2dVariablesHash()
{
    iterator i;
    for( i = begin(); i != end(); ++i )
    {
        delete i->second;
    }
}

bool a2dVariablesHash::SetVariable( const wxString& variableName, a2dNamedProperty* property )
{
    iterator iter = find( variableName );
    if( iter != end() )
    {
        delete iter->second;
    }

    ( *this )[ variableName ] = property;

    return iter != end();
}

bool a2dVariablesHash::SetVariableRefObject( const wxString& variableName, a2dObject* value )
{
    iterator iter = find( variableName );
    if( iter != end() )
    {
        delete iter->second;
    }

    ( *this )[ variableName ] = new a2dProperty( a2dPropertyIdRefObject::GetDummy(), value );

    return iter != end();
}

bool a2dVariablesHash::SetVariableString( const wxString& variableName, const wxString& value )
{
    iterator iter = find( variableName );
    if( iter != end() )
    {
        delete iter->second;
    }

    ( *this )[ variableName ] = new a2dStringProperty( a2dPropertyIdString::GetDummy(), value );

    return iter != end();
}

bool a2dVariablesHash::SetVariableDouble( const wxString& variableName, double value )
{
    iterator iter = find( variableName );
    if( iter != end() )
    {
        delete iter->second;
    }

    ( *this )[ variableName ] = new a2dDoubleProperty( a2dPropertyIdDouble::GetDummy(), value );

    return iter != end();
}

bool a2dVariablesHash::SetVariableInt( const wxString& variableName, int value )
{
    iterator iter = find( variableName );
    if( iter != end() )
    {
        delete iter->second;
    }

    ( *this )[ variableName ] = new a2dInt32Property( a2dPropertyIdInt32::GetDummy(), value );

    return iter != end();
}

const a2dNamedProperty* a2dVariablesHash::GetVariable( const wxString& variableName )
{
    iterator iter = find( variableName );
    if( iter == end() )
        return NULL;// error, alias does not exist

    return iter->second;
}

a2dNamedProperty* a2dVariablesHash::GetVariableClone( const wxString& variableName )
{
    iterator iter = find( variableName );
    if( iter == end() )
        return NULL;// error, alias does not exist

    return iter->second->Clone( a2dObject::clone_flat );
}

a2dObject* a2dVariablesHash::GetVariableRefObject( const wxString& variableName )
{
    iterator iter = find( variableName );
    if( iter == end() )
        return NULL;// error, alias does not exist

    return iter->second->GetRefObjectNA();
}

wxString* a2dVariablesHash::GetVariableString( const wxString& variableName )
{
    iterator iter = find( variableName );
    if( iter == end() )
        return NULL;// error, alias does not exist

    a2dStringProperty* prop = wxDynamicCast( iter->second, a2dStringProperty );
    if( prop )
        return prop->GetValuePtr();
    else
        return 0;
}

//----------------------------------------------------------------------------
// a2dVariablesArray
//----------------------------------------------------------------------------

a2dVariablesArray::a2dVariablesArray()
{
    m_nelements = 0;
    m_melements = 100;
    m_properties = new a2dNamedProperty*[m_melements];

    for( int i = 0; i < 100; i++ )
    {
        m_properties[i] = 0;
    }
}

a2dVariablesArray::~a2dVariablesArray()
{
    for( int i = 0; i < m_nelements; i++ )
    {
        delete m_properties[i];
    }
    delete [] m_properties;
}

void a2dVariablesArray::DeleteAll()
{
    for( int i = 0; i < m_nelements; i++ )
    {
        delete m_properties[i];
        m_properties[i] = 0;
    }
    m_nelements = 0;
}

void a2dVariablesArray::AppendVariable( a2dNamedProperty* value )
{
    SetMinSize( m_nelements + 1 );
    m_properties[ m_nelements++ ] = value;
}

void a2dVariablesArray::AppendVariableRefObject( a2dObject* value )
{
    SetMinSize( m_nelements + 1 );
    m_properties[ m_nelements++ ] = new a2dProperty( a2dPropertyIdRefObject::GetDummy(), value );
}

void a2dVariablesArray::AppendVariableString( const wxString& value )
{
    SetMinSize( m_nelements + 1 );
    m_properties[ m_nelements++ ] = new a2dStringProperty( a2dPropertyIdString::GetDummy(), value );
}

void a2dVariablesArray::AppendVariableDouble( double value )
{
    SetMinSize( m_nelements + 1 );
    m_properties[ m_nelements++ ] = new a2dDoubleProperty( a2dPropertyIdDouble::GetDummy(), value );
}

void a2dVariablesArray::AppendVariableInt( int value )
{
    SetMinSize( m_nelements + 1 );
    m_properties[ m_nelements++ ] = new a2dInt32Property( a2dPropertyIdInt32::GetDummy(), value );
}

const a2dNamedProperty& a2dVariablesArray::GetVariable( int index )
{
    wxASSERT( index < m_nelements && index >= 0 );
    return *m_properties[index];
}

a2dNamedProperty* a2dVariablesArray::GetVariableClone( int index )
{
    wxASSERT( index < m_nelements && index >= 0 );
    return m_properties[index]->Clone( a2dObject::clone_flat );
}

a2dObject* a2dVariablesArray::GetVariableRefObject( int index )
{
    wxASSERT( index < m_nelements && index >= 0 );
    return m_properties[index]->GetRefObject();
}

wxString a2dVariablesArray::GetVariableString( int index )
{
    wxASSERT( index < m_nelements && index >= 0 );
    return m_properties[index]->GetString();
}

wxString a2dVariablesArray::GetVariableAsString( int index )
{
    wxASSERT( index < m_nelements && index >= 0 );
    return m_properties[index]->StringValueRepresentation();
}

void a2dVariablesArray::SetMinSize( int size )
{
    if( size > m_melements )
    {
        int i;
        if( size < m_melements * 2 )
            m_melements *= 2;
        else
            m_melements = size;
        a2dNamedProperty** newdata = new a2dNamedProperty*[m_melements];

        for( i = 0; i < m_nelements; i++ )
        {
            newdata[i] = m_properties[i];
        }
        for( ; i < m_melements; i++ )
        {
            newdata[i] = 0;
        }
        delete [] m_properties;
        m_properties = newdata;
    }
}

// ----------------------------------------------------------------------------
// a2dPathList
// ----------------------------------------------------------------------------
#define PATHLENGTH 255 * 4

// MT-FIXME: get rid of this horror and all code using it
static wxChar wxFileFunctionsBuffer[PATHLENGTH];

a2dPathList::a2dPathList()
{
}

a2dPathList::~a2dPathList()
{
    Clear();
}

wxString a2dPathList::GetAsString()
{
    wxString ret;

#if wxCHECK_VERSION(2,7,0)
    for ( wxPathList::iterator node = begin(); node != end(); node++ )
#else
    for ( wxStringList::compatibility_iterator node = GetFirst(); node; node = node->GetNext() )
#endif
    {
        wxString path = node->GetData();
        ret += wxPATH_SEP + path;
    }
    return ret;
}

wxString a2dPathList::FindValidPath ( const wxString& filename, bool reportError )
{
    wxFileName ffile( filename );
    wxString filefull = ffile.GetFullPath();

    if ( ExpandPath( filefull ) && wxFileExists( filefull ) )
        return wxString( m_b );

    wxFileName fileobj = wxFileName( filefull );

    // relative ./whatdir/file.ext should also be searched for.
    wxString filenamefull = fileobj.IsAbsolute() ? fileobj.GetFullName() : fileobj.GetFullPath();

#if wxCHECK_VERSION(2,7,0)
    for ( wxPathList::iterator node = begin(); node != end(); node++ )
#else
    for ( wxStringList::compatibility_iterator node = GetFirst(); node; node = node->GetNext() )
#endif
    {
        wxFileName filep( node->GetData() );
        wxString path = filep.GetFullPath();
        if ( !path.IsEmpty() )
        {
            wxChar ch = path[wxStrlen( path ) - 1];
            if ( ch != wxFileName::GetPathSeparator() )
                path += wxFileName::GetPathSeparator(wxPATH_UNIX);
        }
        path += filenamefull;

        if ( ExpandPath( path ) && wxFileExists( path ) )
            return wxString( m_b ); // Found!
    }

    if ( reportError )
    {
        wxString mess = GetAsString();
        /*
                for (wxStringList::compatibility_iterator node = GetFirst(); node; node = node->GetNext())
                {
                    wxFileName filep( node->GetData() );
                    wxString path = filep.GetFullPath();
                    if ( !path.IsEmpty() )
                    {
                        wxChar ch = path[wxStrlen(path)-1];
                        if ( ch != wxFileName::GetPathSeparator() )
                            path += wxFileName::GetPathSeparator();
                    }
                    path += filenamefull;

                    ExpandPath( path );
                    mess += path + wxT("\n");
                }
        */
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "valid Path not found for: %s in path list: %s" ), filename.c_str(), mess.c_str() );
        //wxLogError( _T( "valid Path not found for: %s in path list: %s" ), filename.c_str(), mess.c_str() );
        //wxLog::GetActiveTarget()->Flush();

    }
    return wxEmptyString; //Not found
}

wxString a2dPathList::FindAbsoluteValidPath ( const wxString& file )
{
    wxString f = FindValidPath( file );
    if ( f.empty() || wxIsAbsolutePath( f ) )
        return f;

    wxString buf = wxGetCwd();

    if ( !wxEndsWithPathSeparator( buf ) )
    {
        buf += wxFileName::GetPathSeparator();
    }
    buf += f;

    return buf;
}

void a2dPathList::IncC()
{
    if ( m_c == 0 ) //get first character from buffer
    {
        //m_c = m_path.wchar_str();
        m_c = m_path.c_str();
    }
    else
    {
        if ( *m_c != wxT( '\0' ) )
            m_c++;
    }
    a = *m_c;
}

//  <name>          ::= { LETTER | DIGIT | UNDERSCORE | MINUS }*
// Check Name specification
bool a2dPathList::Name()
{
    m_varref.Empty();
    if ( !( wxIsalnum( a ) || a == wxT( '_' ) ||  a == wxT( '-' ) ) )
    {
        m_error_mes = wxT( "wrong name" );
        return false;   // invalid name (begin char)
    }
    m_varref += a;
    IncC();
    while( wxIsalnum( a ) || a == wxT( '_' ) )
    {
        m_varref += a;
        IncC();
    }
    return true;    // name is validated
}

//<path>=  { <aliasref> | <envref> | {CHAR}* }*
bool a2dPathList::ExpandPath( wxString& pathToExpand, wxPathFormat format )
{
    wxString result;
    m_path = pathToExpand;
    m_b.Clear();
    m_b.Alloc( PATHLENGTH );

    bool change = true;
    while ( change )
    {
        change = false;
        m_b.Clear();
        m_varref.Clear();
        m_error_mes.Clear();
        m_c = 0;

        IncC();

        while( a != wxT( '\t' ) && a != wxT( '\0' ) && a != wxT( '\n' ) && a != wxT( ';' ) )
        {
            /*
                    if (a == wxT('\\') ) //take the next character
                    {   IncC();
                        if ( a == wxT('\n') )
                            IncC(); //treat it as if it was a space

                        if ( a == wxT(' ') || a == wxT('\t') || a == wxT('\0') || a == wxT(';') )
                            break; //treat it as word seperator

                        //all other backslash character take them
                        m_b += a;
                        IncC();
                        continue;
                    }
            */
            if ( ( a == wxT( '$' ) ) || ( a == wxT( '%' ) ) || ( a == wxT( '@' ) ) )
            {
                if ( !VarRef( a ) )
                    return false;
                change = true;
                continue;
            }

            //all other charactersdo
            m_b += a;
            IncC();
        }

        m_path = m_b;
    }

    wxFileName ffile( m_b );
    pathToExpand = ffile.GetFullPath( format );

    return true;
}

// Get Alias value
// <aliasref>           ::= ('%') { <name> <blank> | ('{') <name> ('}') }
// Get Environment string
//  <envstring>     ::= ('$') { <name> <blank> | ('{') <name> ('}') }
bool a2dPathList::VarRef( wxChar type )
{
    if( a != type ) // '$' or '%' must be there
    {
        m_error_mes = wxT( "wrong environment $ or % or @ missing" );
        return false;
    }

    IncC();
    if( a == wxT( '{' ) ) // brace could be there
    {
        IncC();
        if( !Name() )
            return false; // error, wrong alias name
        if( a != '}' ) // brace must be there
        {
            m_error_mes = wxT( "wrong environment name, end brace missing" );
            return false; // error, wrong alias, endbrace missing or wrong name
        }
        IncC();
    }
    else
    {
        if( !Name() )
            return false; // error, wrong alias name
    }

    wxString evaluated;
    if ( type == wxT( '$' ) )
    {
        if ( m_varref.Len() == 0 )
            return false;   // A name is needed

        if( !wxGetEnv( m_varref, &evaluated ) )
        {
            m_error_mes = wxT( "wrong environment Variable, environment does not exist" );
            return false;// error, alias does not exist
        }
    }
    else
    {
        a2dVariablesHash aliaslist = a2dGeneralGlobals->GetVariablesHash();
        const a2dNamedProperty* prop = a2dGeneralGlobals->GetVariablesHash().GetVariable( m_varref );
        if( !prop )
        {
            m_error_mes = wxT( "wrong Variable name, Variable does not exist" );
            return false;// error, alias does not exist
        }
        evaluated = prop->StringValueRepresentation();
    }

    m_b += evaluated;
    return true;
}

// ----------------------------------------------------------------------------
// Definition of a2dRefMap
// ----------------------------------------------------------------------------
a2dRefMap::a2dRefMap()
{
    m_refcount = 0;
    m_toResolve.DeleteContents( true );
    m_lastObjectId.Empty();
}

a2dRefMap::~a2dRefMap()
{
    wxASSERT_MSG(  m_refcount == 0, wxT( "deleting a2dDocument while referenced" ) );
    m_objecthashmap.clear();
    m_objectCreate.clear();
    m_toResolve.Clear();
    m_lastObjectId.Empty();
}

a2dObject* a2dRefMap::DoClone( CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    wxFAIL_MSG( wxT( "cannot clone a2dRefMap" ) );
    return NULL;
}

#if wxART2D_USE_CVGIO
void a2dRefMap::DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) )
{
    wxFAIL_MSG( wxT( "cannot Save a2dRefMap" ) );
}

void a2dRefMap::DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) )
{
    wxFAIL_MSG( wxT( "cannot Load a2dRefMap" ) );
}
#endif //wxART2D_USE_CVGIO

void a2dRefMap::Initialize()
{
    m_objecthashmap.clear();
    m_toResolve.Clear();
    m_lastObjectId.Empty();
}

bool a2dRefMap::ResolveOrAdd( a2dSmrtPtr<a2dObject>* storedHere, const wxString& id )
{
    if ( storedHere && ( !id.IsEmpty() || !m_lastObjectId.IsEmpty() ) )
    {
        a2dHashMapIntToObject::iterator obj = m_objecthashmap.find( id.IsEmpty() ? m_lastObjectId : id );

        if( obj == m_objecthashmap.end() )
        {
            a2dResolveIdInfo* a = new a2dResolveIdInfo( id.IsEmpty() ? m_lastObjectId : id, storedHere );
            m_toResolve.Append( a );
            if( id.IsEmpty() )
                m_lastObjectId.Empty();
            return false;
        }

        //the object referenced is already found, link it now.
        *storedHere =  wxDynamicCast( obj->second, a2dObject );
        if( id.IsEmpty() )
            m_lastObjectId.Empty();
        return true;
    }
    return false;
}

bool a2dRefMap::ResolveOrAdd( a2dAutoZeroPtr<a2dObject>* storedHere, const wxString& id )
{
    if ( storedHere && ( !id.IsEmpty() || !m_lastObjectId.IsEmpty() ) )
    {
        a2dHashMapIntToObject::iterator obj = m_objecthashmap.find( id.IsEmpty() ? m_lastObjectId : id );

        if( obj == m_objecthashmap.end() )
        {
            a2dResolveIdInfo* a = new a2dResolveIdInfo( id.IsEmpty() ? m_lastObjectId : id, storedHere );
            m_toResolve.Append( a );
            if( id.IsEmpty() )
                m_lastObjectId.Empty();
            return false;
        }

        //the object referenced is already found, link it now.
        *storedHere =  wxDynamicCast( obj->second, a2dObject );
        if( id.IsEmpty() )
            m_lastObjectId.Empty();
        return true;
    }
    return false;
}

bool a2dRefMap::ResolveOrAddLink( a2dObject* owner, const wxString& id )
{
    if ( owner && ( !id.IsEmpty() || !m_lastObjectId.IsEmpty() ) )
    {
        a2dHashMapIntToObject::iterator obj = m_objecthashmap.find( id.IsEmpty() ? m_lastObjectId : id );

        if( obj == m_objecthashmap.end() )
        {
            a2dResolveIdInfo* a = new a2dResolveIdInfo( id.IsEmpty() ? m_lastObjectId : id, owner );
            m_toResolve.Append( a );
            if( id.IsEmpty() )
                m_lastObjectId.Empty();
            return false;
        }

        //the object referenced is already found, link it now.
        owner->LinkReference( obj->second );
        if( id.IsEmpty() )
            m_lastObjectId.Empty();
        return true;
    }
    return false;
}

void a2dRefMap::ResolveInform( a2dObject* obj )
{
    a2dResolveIdInfo* a = new a2dResolveIdInfo( obj );
    m_toResolve.Append( a );
}

void a2dRefMap::SetLastObjectLoadedId( const wxString& lastid )
{
    wxASSERT_MSG( m_lastObjectId.IsEmpty(), wxT( "last refid not resolved" ) );
    m_lastObjectId = lastid;
}

bool a2dRefMap::RemoveReference( const wxString& idToRemove )
{
    a2dResolveIdList::compatibility_iterator node = m_toResolve.GetFirst();
    while ( node )
    {
        a2dResolveIdList::compatibility_iterator delnode = node;
        node = node->GetNext();
        a2dResolveIdInfo* linkinfo = delnode->GetData();
        wxString toLinkName = linkinfo->m_idStr;
        if( toLinkName == idToRemove )
        {
            m_toResolve.DeleteNode( delnode );
            return true;
        }
    }
    return false;
}

bool a2dRefMap::LinkReferences( bool ignoreNonResolved )
{
    a2dResolveIdList::compatibility_iterator node = m_toResolve.GetFirst();
    while ( node )
    {
        a2dResolveIdList::compatibility_iterator delnode = node;
        node = node->GetNext();
        a2dResolveIdInfo* linkinfo = delnode->GetData();
        wxString toLinkName = linkinfo->m_idStr;
        a2dHashMapIntToObject::iterator obj = m_objecthashmap.find( toLinkName );
        if( obj != m_objecthashmap.end() )
        {
            delnode->GetData()->Assign( obj->second );
            m_toResolve.DeleteNode( delnode );
        }
        else if ( !ignoreNonResolved )
        {
            if ( linkinfo->m_mode == a2dResolveIdInfo::mode_a2dObjectSmrtPtr )
            {
                a2dSmrtPtr<a2dObject>* poin = linkinfo->m_storedHere.m_a2dObjectSmrtPtr;
                if ( *poin )
                    wxLogWarning(  wxT( "%s %s %s" ), _( "Reference not resolved: " ), (*poin)->GetClassInfo()->GetClassName(), toLinkName );
                else
                    wxLogWarning(  wxT( "%s%s" ), _( "Reference not resolved: " ), toLinkName );
            }
            else if ( linkinfo->m_mode == a2dResolveIdInfo::mode_link )
            {
                a2dObject* poin = linkinfo->m_storedHere.m_link;
                wxLogWarning(  wxT( "%s %s %s" ), _( "Reference not resolved: " ), poin->GetClassInfo()->GetClassName(), toLinkName );
            }
            else
                wxLogWarning(  wxT( "%s%s" ), _( "Reference not resolved: " ), toLinkName );
        }
    };

    return true;
}

// ----------------------------------------------------------------------------
// Definition of a2dIOHandler
// ----------------------------------------------------------------------------
a2dIOHandler::a2dIOHandler()
{
    m_refcount = 0;
    m_toResolve.DeleteContents( true );
    m_lastObjectId.Empty();
}

a2dIOHandler::~a2dIOHandler()
{
    wxASSERT_MSG(  m_refcount == 0, wxT( "deleting a2dDocument while referenced" ) );
    m_objecthashmap.clear();
    m_objectCreate.clear();
    m_toResolve.Clear();
    m_lastObjectId.Empty();
}

a2dObject* a2dIOHandler::DoClone( CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    wxFAIL_MSG( wxT( "cannot clone a2dIOHandler" ) );
    return NULL;
}

#if wxART2D_USE_CVGIO
void a2dIOHandler::DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) )
{
    wxFAIL_MSG( wxT( "cannot Save a2dIOHandler" ) );
}

void a2dIOHandler::DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) )
{
    wxFAIL_MSG( wxT( "cannot Load a2dIOHandler" ) );
}
#endif //wxART2D_USE_CVGIO

wxObject* a2dIOHandler::CreateObject( const wxString& symbolicName )
{
    a2dHashMapCreateObject::iterator obj = m_objectCreate.find( symbolicName );

    if( obj == m_objectCreate.end() )
    {
        return wxCreateDynamicObject( symbolicName );
    }
    return wxCreateDynamicObject( obj->second );
}

// ----------------------------------------------------------------------------
// Definition of a2dIOHandlerStrIn
// ----------------------------------------------------------------------------
a2dIOHandlerStrIn::a2dIOHandlerStrIn( wxMBConv& conv ): m_conv( conv )
{
    m_mode = wxEOL_NATIVE;
    if ( m_mode == wxEOL_NATIVE )
    {
#if defined(__WXMSW__) || defined(__WXPM__)
        m_mode = wxEOL_DOS;
#elif defined(__WXMAC__) && !defined(__DARWIN__)
        m_mode = wxEOL_MAC;
#else
        m_mode = wxEOL_UNIX;
#endif
    }
}

a2dIOHandlerStrIn::~a2dIOHandlerStrIn()
{
    wxASSERT_MSG(  m_refcount == 0, wxT( "deleting a2dDocument while referenced" ) );
}

void a2dIOHandlerStrIn::InitializeLoad()
{
    m_objecthashmap.clear();
    m_toResolve.Clear();
}

void a2dIOHandlerStrIn::ResetLoad()
{
    m_objecthashmap.clear();
    m_toResolve.Clear();
}

char a2dIOHandlerStrIn::Peek()
{
#if wxUSE_STD_IOSTREAM
    return m_streami->peek();
#else
    return m_streami->Peek();
#endif
}

char a2dIOHandlerStrIn::GetC()
{
#if wxUSE_STD_IOSTREAM
    return m_streami->get();
#else
    return m_streami->GetC();
#endif
}

bool a2dIOHandlerStrIn::Eof() const
{
#if wxUSE_STD_IOSTREAM
    if ( m_streami->eof() )
#else
    if ( m_streami->Eof() )
#endif
        return true;
    return false;
}

bool a2dIOHandlerStrIn::IsOk() const
{
#if wxUSE_STD_IOSTREAM
    return ( m_streami->fail() || m_streami->bad() );
#else
    return ( m_streami->IsOk() );
#endif
}

a2dIOHandlerStrIn& a2dIOHandlerStrIn::SeekI( wxFileOffset pos )
{
#if wxUSE_STD_IOSTREAM
    m_streami->clear(); // not interested in stream errors here
    m_streami->seekg( pos, wxSTD ios::beg );
#else
    m_streami->SeekI( pos );
#endif
    return *this;
}
/*
size_t a2dIOHandlerStrIn::Read( wxChar* buffer, size_t size)
{
#if wxUSE_STD_IOSTREAM
    m_streami->read( buffer, size);
    size_t len = m_streami->gcount();
#else
    size_t len = m_streami->Read( buffer, size).LastRead();
#endif
    return len;
}
*/

//#if wxUSE_UNICODE
size_t a2dIOHandlerStrIn::Read( char* buffer, size_t size )
{
#if wxUSE_STD_IOSTREAM
    m_streami->read( buffer, size );
    size_t len = m_streami->gcount();
#else
    size_t len = m_streami->Read( buffer, size ).LastRead();
#endif
    return len;
}
//#endif

void a2dIOHandlerStrIn::SetMode( wxEOL mode )
{
    m_mode = mode;
    if ( m_mode == wxEOL_NATIVE )
    {
#if defined(__WXMSW__) || defined(__WXPM__)
        m_mode = wxEOL_DOS;
#elif defined(__WXMAC__) && !defined(__DARWIN__)
        m_mode = wxEOL_MAC;
#else
        m_mode = wxEOL_UNIX;
#endif
    }
}

// ----------------------------------------------------------------------------
// Definition of a2dIOHandlerStrOut
// ----------------------------------------------------------------------------
a2dIOHandlerStrOut::a2dIOHandlerStrOut( wxMBConv& conv ): m_conv( conv )
{
    m_precision = -1;
    m_filename = wxFileName( "" );
    m_mode = wxEOL_NATIVE;
    if ( m_mode == wxEOL_NATIVE )
    {
#if defined(__WXMSW__) || defined(__WXPM__)
        m_mode = wxEOL_DOS;
#elif defined(__WXMAC__) && !defined(__DARWIN__)
        m_mode = wxEOL_MAC;
#else
        m_mode = wxEOL_UNIX;
#endif
    }
}

a2dIOHandlerStrOut::~a2dIOHandlerStrOut()
{
}

void a2dIOHandlerStrOut::InitializeSave()
{
    m_objecthashmap.clear();
    m_toResolve.Clear();
}

void a2dIOHandlerStrOut::ResetSave()
{
    m_objecthashmap.clear();
    m_toResolve.Clear();
}

void a2dIOHandlerStrOut::WriteString( const wxString& string )
{
    size_t len = string.length();

    wxString out;
    out.reserve( len );

    if ( m_mode != wxEOL_UNIX )
    {
        for ( size_t i = 0; i < len; i++ )
        {
            const wxChar c = string[i];
            if ( c == wxT( '\n' ) )
            {
                switch ( m_mode )
                {
                    case wxEOL_DOS:
                        out << _T( "\r\n" );
                        continue;

                    case wxEOL_MAC:
                        out << _T( '\r' );
                        continue;

                    default:
                        wxFAIL_MSG( _T( "unknown EOL mode in a2dIOHandlerStrOut" ) );
                        // fall through

                    case wxEOL_UNIX:
                        // don't treat '\n' specially
                        ;
                }
            }

            out << c;
        }
    }
    else
        out = string;

#if wxUSE_STD_IOSTREAM

    // We must not write the trailing NULL here
	#if wxUSE_UNICODE
		#if !wxUSE_WSTREAM
			wxCharBuffer buffer = m_conv.cWC2MB( out );
			m_streamo->write( ( const char* ) buffer, strlen( ( const char* ) buffer ) );
		#else
			m_streamo->write(  out, len );
		#endif
	#else
		m_streamo->write( out.c_str(), out.length() );
	#endif

#else
    // We must not write the trailing NULL here
	#if wxUSE_UNICODE
		wxCharBuffer buffer = m_conv.cWC2MB( out );
		m_streamo->Write( ( const char* ) buffer, strlen( ( const char* ) buffer ) );
	#else
		//in memory converted to wide char unicode string with wxConvLocal, and convert with m_conv e.g UTF-8
		wxString str2( out.wc_str( wxConvLocal ), m_conv );
		m_streamo->Write( str2.mb_str(), str2.Len() );
	#endif

#endif
}

void a2dIOHandlerStrOut::Write32( wxUint32 i )
{
    wxString str;
    str.Printf( wxT( "%u" ), i );

    WriteString( str );
}

void a2dIOHandlerStrOut::Write16( wxUint16 i )
{
    wxString str;
    str.Printf( wxT( "%u" ), i );

    WriteString( str );
}

void a2dIOHandlerStrOut::Write8( wxUint8 i )
{
    wxString str;
    str.Printf( wxT( "%u" ), i );

    WriteString( str );
}

void a2dIOHandlerStrOut::WriteDouble( double d )
{
    wxString str = wxString::FromCDouble( d, m_precision );

    WriteString( str );
}

bool a2dIOHandlerStrOut::IsOk() const
{
#if wxUSE_STD_IOSTREAM
    return ( m_streamo->fail() || m_streamo->bad() ) == 0;
#else
    return ( m_streamo->IsOk() );
#endif
}

a2dIOHandlerStrOut& a2dIOHandlerStrOut::PutChar( wxChar c )
{
    WriteString( wxString( &c, m_conv, 1 ) );
    return *this;
}

//----------------------------------------------------------------------------
// a2dWalkerIOHandler
//----------------------------------------------------------------------------

IMPLEMENT_CLASS( a2dWalkerIOHandler, a2dIOHandler )

extern a2dWalkEvent a2dWalker_a2dObjectStart = wxNewId();
extern a2dWalkEvent a2dWalker_a2dObjectEnd = wxNewId();
extern a2dWalkEvent a2dWalker_a2dPropertyStart = wxNewId();
extern a2dWalkEvent a2dWalker_a2dPropertyEnd = wxNewId();
extern a2dWalkEvent a2dWalker_a2dNamedPropertyListStart = wxNewId();
extern a2dWalkEvent a2dWalker_a2dNamedPropertyListEnd = wxNewId();
extern a2dWalkEvent a2dWalker_a2dNamedPropertyStart = wxNewId();
extern a2dWalkEvent a2dWalker_a2dNamedPropertyEnd = wxNewId();

extern a2dWalkEvent a2dWalker_a2dCanvasObjectStart = wxNewId();
extern a2dWalkEvent a2dWalker_a2dCanvasObjectProperties = wxNewId();
extern a2dWalkEvent a2dWalker_a2dCanvasObjectPreChild = wxNewId();
extern a2dWalkEvent a2dWalker_a2dCanvasObjectPostChild = wxNewId();
extern a2dWalkEvent a2dWalker_a2dCanvasObjectEnd = wxNewId();
extern a2dWalkEvent a2dWalker_a2dDerivedCanvasObjectStart = wxNewId();
extern a2dWalkEvent a2dWalker_a2dDerivedCanvasObjectEnd = wxNewId();

extern a2dWalkEvent a2dWalker_a2dDrawingStart = wxNewId();
extern a2dWalkEvent a2dWalker_a2dDrawingEnd = wxNewId();

a2dWalkerIOHandler::a2dWalkerIOHandler()
{
    m_useCheck = false;
    m_depthFirst = false;
    m_skipNotRenderedInDrawing = false;
    Initialize();
}

a2dWalkerIOHandler::~a2dWalkerIOHandler()
{
}

void a2dWalkerIOHandler::Initialize()
{
    m_currentDepth = 0;
    m_currentParent = NULL;
    m_stopWalking = false;
    m_result = false;
}

bool a2dWalkerIOHandler::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    a2dObjectIOFn* function = m_register.find( object->GetClassInfo()->GetClassName() )->second;

    if ( function )
    {
        ( *function )( parent, object, event );
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// a2dWalker_Renumber
//----------------------------------------------------------------------------


a2dWalker_Renumber::a2dWalker_Renumber()
{
    SetDepthFirst( true );
    Initialize();
}

a2dWalker_Renumber::~a2dWalker_Renumber()
{
}

bool a2dWalker_Renumber::Start( a2dObject* object )
{
    object->Walker( NULL, *this );
    return true;
}

bool a2dWalker_Renumber::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dObjectStart )
    {
        a2dObject* obj = ( a2dObject* ) object;
        wxLongLong newid = GetUniqueSerializationId();
        obj->SetId( newid );
    }
    return true;
}

//----------------------------------------------------------------------------
// color conversion
//----------------------------------------------------------------------------

// 3-digit hex to wxColour
wxColour HexToColour( const wxString& hex )
{
    if ( hex.Length() == 6 )
    {
        long r, g, b;
        r = g = b = 0;
        hex.Mid( 0, 2 ).ToLong( &r, 16 );
        hex.Mid( 2, 2 ).ToLong( &g, 16 );
        hex.Mid( 4, 2 ).ToLong( &b, 16 );
        return wxColour( r, g, b );
    }
    else if ( hex.Length() == 8 )
    {
        long r, g, b, a;
        r = g = b = a = 0;
        hex.Mid( 0, 2 ).ToLong( &r, 16 );
        hex.Mid( 2, 2 ).ToLong( &g, 16 );
        hex.Mid( 4, 2 ).ToLong( &b, 16 );
        hex.Mid( 6, 2 ).ToLong( &a, 16 );
        return wxColour( r, g, b, a );
    }
    else
        return wxColour( 0, 0, 0 );
}

// RGB to 3-digit hex
wxString ColourToHex( const wxColour& colour )
{
    wxChar buf[9];
    unsigned int red = colour.Red();
    unsigned int green = colour.Green();
    unsigned int blue = colour.Blue();
    unsigned int alpha = colour.Alpha();

    wxDecToHex( red, buf );
    wxDecToHex( green, buf + 2 );
    wxDecToHex( blue, buf + 4 );
    wxDecToHex( alpha, buf + 6 );

    return wxString( buf );
}


//----------------------------------------------------------------------------
// a2dComEvent
//----------------------------------------------------------------------------
const a2dSignal a2dComEvent::sm_changedProperty = wxNewId();
const a2dSignal a2dComEvent::sm_changedRefObject = wxNewId();
const a2dSignal a2dComEvent::sm_non = wxNewId();

IMPLEMENT_DYNAMIC_CLASS( a2dComEvent, wxEvent )

DEFINE_EVENT_TYPE( a2dEVT_COM_EVENT )

a2dComEvent::a2dComEvent( wxObject* sender, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = 0;
    m_ownProp = false;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( a2dObject* sender, const a2dPropertyId* propId, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = sender->FindProperty( propId );
    m_ownProp = false;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, a2dNamedProperty* property, a2dSignal id, bool ownProp )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    if ( ownProp )
        m_property = property->Clone( a2dObject::clone_flat );
    else
        m_property = property;
    m_ownProp = ownProp;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, const a2dNamedProperty& property, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = property.Clone( a2dObject::clone_flat );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, a2dObject* refObject, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dProperty( a2dPropertyIdRefObject::GetDummy(), refObject );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, const a2dPropertyIdRefObject* propId, a2dObject* refObject, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dProperty( propId, refObject );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, const a2dPropertyIdBool* propId, bool propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dBoolProperty( propId, propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, const a2dPropertyIdUint16* propId, wxUint16 propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dUint16Property( propId, propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, const a2dPropertyIdInt32* propId, wxInt32 propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dInt32Property( propId, propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, const a2dPropertyIdDouble* propId, double propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dDoubleProperty( propId, propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, const a2dPropertyIdString* propId, const wxString&  propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dStringProperty( propId, propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, bool propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dBoolProperty( a2dPropertyIdBool::GetDummy(), propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, wxInt16 propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dInt16Property( a2dPropertyIdInt16::GetDummy(), propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, wxUint16 propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dUint16Property( a2dPropertyIdUint16::GetDummy(), propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, wxInt32 propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dInt32Property( a2dPropertyIdInt32::GetDummy(), propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, wxUint32 propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dUint32Property( a2dPropertyIdUint32::GetDummy(), propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, double propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dDoubleProperty( a2dPropertyIdDouble::GetDummy(), propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent( wxObject* sender, const wxString&  propertyValue, a2dSignal id )
    : wxEvent( id, a2dEVT_COM_EVENT )
{
    SetEventObject( sender );
    m_property = new a2dStringProperty( a2dPropertyIdString::GetDummy(), propertyValue );
    m_ownProp = true;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::a2dComEvent()
    : wxEvent( sm_non, a2dEVT_COM_EVENT )
{
    SetEventObject( 0 );
    m_ownProp = false;
    m_comReturn = sm_non;
    m_refcount = 0;
}

a2dComEvent::~a2dComEvent()
{
    wxASSERT_MSG(  m_refcount == 0, wxT( "deleting a2dComEvent while referenced" ) );
    if ( m_ownProp )
        delete m_property;
}

a2dComEvent::a2dComEvent( const a2dComEvent& other )
    : wxEvent( other )
{
    m_property = other.m_property ? other.m_property->Clone( a2dObject::clone_flat ) : 0;
    m_ownProp = true;
    m_comReturn = other.m_comReturn;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in Clone ( )
    CurrentSmartPointerOwner = this;
#endif
    m_refcount = 0;
}

wxEvent* a2dComEvent::Clone( void ) const
{
    return new a2dComEvent( *this );
}

const a2dPropertyId* a2dComEvent::GetPropertyId() const
{
    return m_property ? m_property->GetId() : 0;
}

