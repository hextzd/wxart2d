/*! \file general/src/comevt.cpp
    \brief Document/view classes
    \author Klaas Holwerda
    \date Created 05/07/2003

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: comevt.cpp,v 1.113 2009/08/23 19:49:38 titato Exp $
*/

#include "wxartbaseprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(_DEBUG)
//    #define A2D_COMMAND_DEBUG
#endif

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#ifndef WX_PRECOMP
#include "wx/string.h"
#include "wx/dc.h"
#include "wx/list.h"
#endif

#include "wx/general/comevt.h"

#include "wx/general/smrtptr.inl"
#include <wx/general/id.inl>

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

// ----------------------------------------------------------------------------
// wxWindows macros
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dCommandProcessor, a2dObject )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_SetProperty, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_SetVariable, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_SetEnvironmentVariable, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_GetVariable, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_GetEnvVariable, a2dCommand )

DEFINE_EVENT_TYPE( wxEVT_DO )
DEFINE_EVENT_TYPE( wxEVT_UNDO )
DEFINE_EVENT_TYPE( wxEVT_REDO )
DEFINE_EVENT_TYPE( wxEVT_CANUNDO )
DEFINE_EVENT_TYPE( wxEVT_CANREDO )
DEFINE_EVENT_TYPE( wxEVT_MENUSTRINGS )
DEFINE_EVENT_TYPE( wxEVT_BEGINBUSY )
DEFINE_EVENT_TYPE( wxEVT_ENDBUSY )
DEFINE_EVENT_TYPE( a2dEVT_PROPOBJECT_EDITPROPERTIES_EVENT )
DEFINE_EVENT_TYPE( wxEVT_RECORD )

// ----------------------------------------------------------------------------
// a2dCommandId
// ----------------------------------------------------------------------------

a2dCommandId::a2dCommandId( const wxString& commandName )
{
    m_name = commandName;

    // check if name is unique
    for( int indx = 1; GetHashMap().find( commandName ) != GetHashMap().end(); indx++ )
    {
        wxASSERT_MSG( 0, _( "The command id name '" ) + commandName + _( "' already exists" ) );
    }

    GetHashMap()[commandName] = this;
}

const a2dCommandId& a2dCommandId::GetCommandByName( const wxString& commandName )
{
    a2dHashMapCommandIds::iterator iterCommand = GetHashMap().find( commandName );
    return iterCommand != GetHashMap().end() ? *iterCommand->second : a2dCommand::sm_noCommandId;
}

a2dHashMapCommandIds& a2dCommandId::GetHashMap()
{
    static a2dMemoryCriticalSectionHelper helper;
    static a2dHashMapCommandIds namehash;
    return namehash;
}

// ----------------------------------------------------------------------------
// base docview command
// ----------------------------------------------------------------------------

const a2dCommandId a2dCommand::sm_noCommandId = a2dCommandId( wxT( "NoCommandId" ) );
const a2dCommandId a2dCommand::sm_noCommandTypeId = a2dCommandId( wxT( "NoCommandTypeId" ) );
const a2dCommandId a2dCommand::sm_groupCommandId = a2dCommandId( wxT( "GroupCommandId" ) );

a2dCommand::a2dCommand( bool canUndo,
                        const a2dCommandId& commandId,
                        const a2dCommandId& commandTypeId,
                        const wxString& menuString )
    : a2dObject(),
    m_modifies( true ),
    m_sendBusyEvent( true )
{
    m_cmp = 0;
    m_canUndo = canUndo;
    m_commandId = &commandId;
    m_commandTypeId = &commandTypeId;
    m_menuString = menuString;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    extern wxObject* CurrentSmartPointerOwner;
    CurrentSmartPointerOwner = this;
#endif
}

a2dCommand::a2dCommand( const a2dCommand& other )
    :   a2dObject( other, a2dObject::clone_deep, NULL )
{
    m_commandId = other.m_commandId;
    m_commandTypeId = other.m_commandTypeId;
    m_menuString = other.m_menuString;
    m_canUndo = other.m_canUndo;
    m_cmp = other.m_cmp;
    m_modifies = other.m_modifies;
    m_sendBusyEvent = other.m_sendBusyEvent;
}

a2dObject* a2dCommand::DoClone( CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    a2dGeneralGlobals->ReportErrorF( a2dError_CommandError, wxT( "%s" ), _( "clone not implemented for this command" ) );
    return NULL;
}

wxString a2dCommand::GetName() const
{
    if ( m_menuString.IsEmpty() )
        return m_commandId->GetName();
    return m_menuString;
}

a2dCommand::~a2dCommand()
{
}

a2dCommand* a2dCommand::CloneAndBind( a2dObject* WXUNUSED( object ) )
{
    wxASSERT_MSG( 0 , _( "a2dCommand::CloneAndBind not overloaded in derived class" ) );
    return 0;
}

bool a2dCommand::Redo()
{
    return Do();
}

bool a2dCommand::ClearAfterCurrentCommand( a2dCommand* current )
{
    return current == this;
}

a2dCommand* a2dCommand::FindPrevious( a2dCommand* WXUNUSED( current ) ) const
{
    return 0;
}

a2dCommand* a2dCommand::FindNext( a2dCommand* WXUNUSED( current ) ) const
{
    return 0;
}

bool a2dCommand::Remove( a2dCommand* WXUNUSED( command ) )
{
    return false;
}

bool a2dCommand::ContainsCommand( a2dCommand* command )
{
    return command == this;
}

void a2dCommand::DistributeEvent( wxEventType eventType )
{
    if ( eventType == wxEVT_DO )
    {
        a2dCommandProcessorEvent event( wxEVT_DO, this );
        event.SetEventObject( GetCommandProcessor() );
        GetCommandProcessor()->ProcessEvent( event );
    } 
    else if ( eventType == wxEVT_UNDO )
    {
        a2dCommandProcessorEvent event( wxEVT_UNDO, this );
        event.SetEventObject( GetCommandProcessor() );
        GetCommandProcessor()->ProcessEvent( event );
    }
    else if ( eventType == wxEVT_REDO )
    {
        a2dCommandProcessorEvent event( wxEVT_REDO, this );
        event.SetEventObject( GetCommandProcessor() );
        GetCommandProcessor()->ProcessEvent( event );
    }
    else if ( eventType == wxEVT_CANUNDO )
    {
        a2dCommandProcessorEvent event( eventType, this );
        event.SetEventObject( GetCommandProcessor() );
        GetCommandProcessor()->ProcessEvent( event );
    }
    else if ( eventType == wxEVT_CANREDO )
    {
        a2dCommandProcessorEvent event( eventType, this );
        event.SetEventObject( GetCommandProcessor() );
        GetCommandProcessor()->ProcessEvent( event );
    }
}

//----------------------------------------------------------------------------
// a2dCommandGroup
//----------------------------------------------------------------------------

a2dCommandGroup::a2dCommandGroup( const wxString& name, a2dCommandGroup* parent )
    : a2dCommand( true, a2dCommand::sm_groupCommandId )
{
    m_modifies = false; // assume a group on it self does nothing, only its sub commands
    wxString idName;
    idName.Printf( wxT( "%s_%ld" ), name.c_str(), wxGenNewId() );

    m_commandId = new a2dCommandId( idName );
	m_ownId = true;

    m_groupName = name;
    m_parentGroup = parent;
    m_active = true;
}

a2dCommandGroup::a2dCommandGroup( 
				a2dCommandGroup* parent,
                const a2dCommandId& commandId,
                const a2dCommandId& commandTypeId,
                const wxString& menuString
              )
    : a2dCommand( true, commandId, commandTypeId, menuString )
{
    m_modifies = false; // assume a group on it self does nothing, only its sub commands
	m_ownId = false;

    m_groupName = m_commandId->GetName();
    m_parentGroup = parent;
    m_active = true;
}

a2dCommandGroup::~a2dCommandGroup( void )
{
    m_subcommands.clear();
	if ( m_ownId )
		delete m_commandId;
}

a2dObject* a2dCommandGroup::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    a2dCommandGroup* clgr = new a2dCommandGroup( m_groupName, m_parentGroup );

    a2dCommandList::const_iterator iter = m_subcommands.begin();
    while( iter != m_subcommands.end() )
    {
        a2dCommand* obj = *iter;
        a2dCommand* clone = ( a2dCommand* ) obj->Clone( options, refs );
        clgr->m_subcommands.push_back( clone );
        iter++;
    }
    return clgr;
}

bool a2dCommandGroup::ClearAfterCurrentCommand( a2dCommand* current )
{
    if( current == this )
        return true;

    int i = 0;
    a2dCommandList::reverse_iterator iter = m_subcommands.rbegin();
    while( iter != m_subcommands.rend() )
    {
        a2dCommand* obj = *iter;
        if( obj->ClearAfterCurrentCommand( current ) )
        {
            // Everything after this command must be removed.
            a2dCommandList::reverse_iterator iterr = m_subcommands.rbegin();
            while( i > 0 && iterr != m_subcommands.rend() )
            {
                m_subcommands.rerase( iterr ) ;
                i--;
            }
            return true;
        }
        iter++;
        i++;
    }
    return false;
}

bool a2dCommandGroup::ClearCommandsById( const a2dCommandId& commandId, a2dCommand* fromcommand )
{
    bool start = false;
    bool found = false;
    a2dCommandList::iterator iter = m_subcommands.begin();
    while( iter != m_subcommands.end() )
    {
        a2dCommand* command = *iter;
        if( ( fromcommand == NULL || fromcommand == command || start ) )
        {
            start = true;
            if ( command->GetCommandId() == &commandId )
            {
                iter = m_subcommands.erase( iter );
                found = true;
            }
            else
                iter++;
        }
        else
            iter++;
    }
    return found;
}

a2dCommand* a2dCommandGroup::FindPrevious( a2dCommand* current ) const
{

    a2dCommandList::const_reverse_iterator iter = m_subcommands.rbegin();
    while( iter != m_subcommands.rend() )
    {
        a2dCommand* obj = *iter;
        // Check if this command/group contains the current command
        if( obj->ContainsCommand( current ) )
        {
            // Check if this command/group also has a previous command
            a2dCommand* previous = obj->FindPrevious( current );

            // If yes, this is what we want
            if( previous )
                return previous;

            // Else find a command previous from our list
            iter++;
            if( iter != m_subcommands.rend() )
                return *iter;

            return 0;
        }
        iter++;
    }

    return 0;
}

a2dCommand* a2dCommandGroup::FindNext( a2dCommand* current ) const
{
    a2dCommandList::const_reverse_iterator iter = m_subcommands.rbegin();
    while( iter != m_subcommands.rend() )
    {
        a2dCommand* obj = *iter;
        // Check if this command/group contains the current command
        if( obj->ContainsCommand( current ) )
        {
            // Check if this command/group also has a next command
            a2dCommand* next = obj->FindNext( current );

            // If yes, this is what we want
            if( next )
                return next;

            // Else find a command next from our list
            if( iter != m_subcommands.rbegin() )
                iter--;
            else
                return 0;

            if( iter != m_subcommands.rend() )
                return *iter;

            return 0;
        }
        iter++;
    }

    return 0;
}

bool a2dCommandGroup::Remove( a2dCommand* command )
{
    a2dCommandList::reverse_iterator iter = m_subcommands.rbegin();
    while( iter != m_subcommands.rend() )
    {
        a2dCommand* obj = *iter;
        if ( command == obj )
        {
            iter = m_subcommands.rerase( iter );
            return true;
        }
        //check recursive/nested groups
        if ( obj->Remove( command ) )
        {
            return true;
        }
        iter++;
    }
    return false;
}

void a2dCommandGroup::Add( a2dCommand* command )
{
    m_subcommands.push_back( command );
}

bool a2dCommandGroup::ContainsCommand( a2dCommand* command )
{
    if( command == this )
        return true;

    a2dCommandList::reverse_iterator iter = m_subcommands.rbegin();
    while( iter != m_subcommands.rend() )
    {
        a2dCommand* obj = *iter;
        if ( obj->ContainsCommand( command ) )
            return true;
        iter++;
    }
    return false;
}

bool a2dCommandGroup::Do()
{
	/*
    m_cmp->SetCurrentGroup( this );
    m_active = true;
    // Redo is forward
    a2dCommandList::iterator iter = m_subcommands.begin();
    while( iter != m_subcommands.end() )
    {
        a2dCommand* obj = *iter;
        obj->SetCommandProcessor( m_cmp );
        if ( !obj->Do() )
            return false;
        else
            obj->DistributeEvent( wxEVT_DO );

        iter++;
    }
    m_cmp->CommandGroupEnd( this );
	*/
    return true;
}

bool a2dCommandGroup::Redo()
{
	m_cmp->SetCurrentGroup( this );
    m_active = true;
    // Redo is forward
    a2dCommandList::iterator iter = m_subcommands.begin();
    while( iter != m_subcommands.end() )
    {
        a2dCommand* obj = *iter;
        obj->SetCommandProcessor( m_cmp );
        if ( !obj->Redo() )
            return false;
        else
            obj->DistributeEvent( wxEVT_REDO );

        iter++;
    }
	m_cmp->CommandGroupEnd( this );
    return true;
}

bool a2dCommandGroup::Undo()
{
	//NO group set here 

    // Undo is backward
    a2dCommandList::reverse_iterator iter = m_subcommands.rbegin();
    while( iter != m_subcommands.rend() )
    {
        a2dCommand* obj = *iter;
        if ( !obj->Undo() )
            return false;
        else
            obj->DistributeEvent( wxEVT_UNDO );
        iter++;
    }
    return true;
}


//----------------------------------------------------------------------------
// a2dPropertyEditEvent
//----------------------------------------------------------------------------
DEFINE_EVENT_TYPE( wxEVT_PROPOBJECT_EDITPROPERTIES_EVENT )

a2dPropertyEditEvent::a2dPropertyEditEvent( a2dObject* object, a2dNamedPropertyList* properties )
    : a2dEvent( wxID_ANY, a2dEVT_PROPOBJECT_EDITPROPERTIES_EVENT )
{
    SetEventObject( object );
    m_properties = properties;
    m_edited = false;
}

a2dPropertyEditEvent::a2dPropertyEditEvent( const a2dPropertyEditEvent& other )
    : a2dEvent( other )
{
    m_properties = other.m_properties;
    m_edited = other.m_edited;
}

//----------------------------------------------------------------------------
// a2dCommand_SetProperty
//----------------------------------------------------------------------------

const a2dCommandId a2dCommand_SetProperty::Id = a2dCommandId( wxT( "SetProperty" ) );

a2dCommand_SetProperty::a2dCommand_SetProperty():
    a2dCommand( true, Id )
{
    m_propRefObject = NULL;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyId* id ):
    a2dCommand( true, Id )
{
    m_propRefObject = object;
    m_id = id;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, a2dNamedProperty* property ):
    a2dCommand( true, Id )
{
    m_property = property;
    m_propRefObject = object;
    m_id = property->GetId();
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdRefObject* id, a2dObject* value ):
    a2dCommand( true, Id, sm_noCommandTypeId, id->GetName() )
{
    a2dProperty* prop = new a2dProperty( id, value );

    m_property = prop;
    m_propRefObject = object;
    m_id = id;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdObject* id, const wxObject& value ):
    a2dCommand( true, Id, sm_noCommandTypeId, id->GetName() )
{
    a2dObjectProperty* prop = new a2dObjectProperty( id, value );

    m_property = prop;
    m_propRefObject = object;
    m_id = id;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdString* id, const wxString& value ):
    a2dCommand( true, Id, sm_noCommandTypeId, id->GetName() )
{
    a2dStringProperty* prop = new a2dStringProperty( id, value );

    m_property = prop;
    m_propRefObject = object;
    m_id = id;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdBool* id, bool value ):
    a2dCommand( true, Id, sm_noCommandTypeId, id->GetName() )
{
    a2dBoolProperty* prop = new a2dBoolProperty( id, value );

    m_property = prop;
    m_propRefObject = object;
    m_id = id;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdBool* id, const wxString& value ):
    a2dCommand( true, Id, sm_noCommandTypeId, id->GetName() )
{
    a2dNamedProperty* prop = id->CreatePropertyFromString( value );

    m_property = prop;
    m_propRefObject = object;
    m_id = id;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdInt16* id, wxInt16 value ):
    a2dCommand( true, Id, sm_noCommandTypeId, id->GetName() )
{
    a2dInt16Property* prop = new a2dInt16Property( id, value );

    m_property = prop;
    m_propRefObject = object;
    m_id = id;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdUint16* id, wxUint16 value ):
    a2dCommand( true, Id, sm_noCommandTypeId, id->GetName() )
{
    a2dUint16Property* prop = new a2dUint16Property( id, value );

    m_property = prop;
    m_propRefObject = object;
    m_id = id;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdInt32* id, wxInt32 value ):
    a2dCommand( true, Id, sm_noCommandTypeId, id->GetName() )
{
    a2dInt32Property* prop = new a2dInt32Property( id, value );

    m_property = prop;
    m_propRefObject = object;
    m_id = id;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdUint32* id, wxUint32 value ):
    a2dCommand( true, Id, sm_noCommandTypeId, id->GetName() )
{
    a2dUint32Property* prop = new a2dUint32Property( id, value );

    m_property = prop;
    m_propRefObject = object;
    m_id = id;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdDouble* id, double value ):
    a2dCommand( true, Id, sm_noCommandTypeId, id->GetName() )
{
    a2dDoubleProperty* prop = new a2dDoubleProperty( id, value );

    m_property = prop;
    m_propRefObject = object;
    m_id = id;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdColour* id, const wxColour& colour ):
    a2dCommand( true, Id, sm_noCommandTypeId, id->GetName() )
{
    a2dColourProperty* prop = new a2dColourProperty( id, colour );

    m_property = prop;
    m_propRefObject = object;
    m_id = id;
}


a2dCommand_SetProperty::~a2dCommand_SetProperty( void )
{
    m_propRefObject = NULL;
}

a2dCommand_SetProperty::a2dCommand_SetProperty( const a2dCommand_SetProperty& other )
    :   a2dCommand ( other )
{
    m_propRefObject = other.m_propRefObject;
    m_property = other.m_property->Clone( a2dObject::clone_flat );
    m_id = other.m_id;
}

bool a2dCommand_SetProperty::Do( void )
{
    if ( m_propRefObject )
    {
        a2dNamedPropertyPtr propertyold = m_propRefObject->GetProperty( m_id );
        m_propRefObject->SetProperty( m_property );
        m_property = propertyold;
    }

    return true;
}

bool a2dCommand_SetProperty::Undo( void )
{
    if ( m_propRefObject )
    {
        a2dNamedPropertyPtr propertyold = m_propRefObject->GetProperty( m_id );

        if ( !m_property )
        {
            //the property did not exist before, so we need to remove the one that was added.
            m_propRefObject->RemoveProperty( m_id, true );
        }
        else
        {
            m_propRefObject->SetProperty( m_property );
        }

        m_property = propertyold;
    }

    return true;
}

// ----------------------------------------------------------------------------
// a2dCommandProcessor
// ----------------------------------------------------------------------------

const a2dCommandId a2dCommand_SetVariable::Id( wxT( "SetVariable" ) );
const a2dCommandId a2dCommand_SetEnvironmentVariable::Id( wxT( "SetEnv" ) );
const a2dCommandId a2dCommand_GetVariable::Id( wxT( "GetVariable" ) );

a2dCommandProcessor::a2dCommandProcessor( int maxCommands )
{
    m_rootGroup = new a2dCommandGroup( _( "Root" ), NULL );
    m_currentGroup = m_rootGroup;
    m_maxNoCommands = maxCommands;
    m_currentCommand = NULL;
    m_undoAccelerator = _( "\tCtrl+Z" );
    m_redoAccelerator = _( "\tCtrl+Y" );
    m_sendBusyEvent = true;
}

a2dCommandProcessor::~a2dCommandProcessor()
{
    m_currentGroup = m_rootGroup;
    ClearCommands();
}

a2dObject* a2dCommandProcessor::DoClone( CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    wxFAIL_MSG( wxT( "cannot clone a2dIOHandler" ) );
    return NULL;
}

bool a2dCommandProcessor::SmrtPtrRelease()
{
    m_refcount--;
    wxASSERT_MSG( m_refcount >= 0, wxT( "a2dCommandProcessor Own/Release not matched (extra Release calls)" ) );
    if ( m_refcount <= 0 )
    {
        delete this;
        return true;
    }
    return false;
}

a2dCommandGroup* a2dCommandProcessor::FindActiveGroup()
{
    a2dCommandGroup* group = m_currentGroup;

    for( ;; )
    {
        if( group->m_subcommands.empty() )
            break;

        a2dCommandGroup* next = group->m_subcommands.back()->IsCommandGroup();
        if( !next || !next->m_active )
            break;

        group = next;
    }

    return group;
}

void a2dCommandProcessor::SetCurrentToLastActive()
{
    a2dCommandGroup* group = m_currentGroup;

    for( ;; )
    {
        if( group->m_subcommands.empty() )
            break;

        a2dCommandGroup* next = group->m_subcommands.back()->IsCommandGroup();
        if( !next || !next->m_active || next->m_subcommands.empty() )
            break;

        group = next;
    }

    m_currentCommand = ( group->m_subcommands.empty() ) ? 0 : group->m_subcommands.back();
}

bool a2dCommandProcessor::DoPreCommand( a2dCommand& cmd )
{
#if defined(A2D_COMMAND_DEBUG)
    wxLogDebug( wxT( "Pre Do name = %s 0x%p" ), cmd.GetName().c_str(), &cmd );
#endif
    bool ret = cmd.PreDo();
    return ret;
}

bool a2dCommandProcessor::DoPostCommand( a2dCommand& cmd )
{
#if defined(A2D_COMMAND_DEBUG)
    wxLogDebug( wxT( "Post Do name = %s 0x%p" ), cmd.GetName().c_str(), &cmd );
#endif
    bool ret = cmd.PostDo();
    return ret;
}

bool a2dCommandProcessor::DoCommand( a2dCommand& cmd )
{
#if defined(A2D_COMMAND_DEBUG)
    wxLogDebug( wxT( "Do name = %s 0x%p" ), cmd.GetName().c_str(), &cmd );
#endif
    bool ret = cmd.Do();

    if ( ret )
        cmd.DistributeEvent( wxEVT_DO );
    return ret;
}

bool a2dCommandProcessor::UndoCommand( a2dCommand& cmd )
{
#if defined(A2D_COMMAND_DEBUG)
    wxLogDebug( wxT( "UnDo name = %s 0x%p" ), cmd.GetName().c_str(), &cmd );
#endif

    bool ret = cmd.Undo();

    if ( ret )
        cmd.DistributeEvent( wxEVT_UNDO );
    return ret;
}

bool a2dCommandProcessor::RedoCommand( a2dCommand& cmd )
{
#if defined(A2D_COMMAND_DEBUG)
    wxLogDebug( wxT( "ReDo name = %s 0x%p" ), cmd.GetName().c_str(), &cmd );
#endif

    bool ret = cmd.Redo();

    if ( ret )
        cmd.DistributeEvent( wxEVT_REDO );
    return ret;
}

void a2dCommandProcessor::PushSendBusyEvent( bool sendBusyEvent )
{
    m_busyStack.push_back( m_sendBusyEvent );
    m_sendBusyEvent = sendBusyEvent;
}

bool a2dCommandProcessor::PopSendBusyEvent()
{
    if ( m_busyStack.size() > 0 )
    {
        m_sendBusyEvent = m_busyStack.back();
        m_busyStack.pop_back();
    }
    return m_sendBusyEvent;
}

void a2dCommandProcessor::SendBusyEvent( bool start, a2dCommand* command )
{
    if ( !m_sendBusyEvent || (command && !command->GetSendBusyEvent() ) )
        return;

    if ( start )
    {
        if ( m_busyStack.size() == 0 )
        {
            a2dCommandProcessorEvent event( wxEVT_BEGINBUSY, command );
            event.SetEventObject( this );
            this->ProcessEvent( event );
        }
        m_busyStack.push_back( true );
    }
    else
    {        
        m_busyStack.pop_back();
        if ( m_busyStack.size() == 0 )
        {
            a2dCommandProcessorEvent event( wxEVT_ENDBUSY, command );
            event.SetEventObject( this );
            this->ProcessEvent( event );
        }
    }
}

// Pass a command to the processor. The processor calls Do();
// if successful, is appended to the command history unless
// storeIt is false.
bool a2dCommandProcessor::Submit( a2dCommand* command, bool storeIt )
{
    wxCHECK_MSG( command, false, _( "no command in a2dCommandProcessor::Submit" ) );

    SendBusyEvent( true, command );

    // to properly delete it if needed
    a2dSmrtPtr<a2dCommand> lcommand = command;

    command->SetCommandProcessor( this );
    if ( command->IsCommandGroup() )
    {
        wxStaticCast( command, a2dCommandGroup )->m_parentGroup = m_currentGroup;
    }

    bool oke = false;
    try
    {
        //commands in here will be stored before the actual one. 
        oke = DoPreCommand( *command );

        //first store, and if not right command, remove it again.
        if ( storeIt )
            Store( command );

        oke |= DoCommand( *command );

        //commands in here will be stored after the actual one. 
        oke |= DoPostCommand( *command );
    }
    catch ( const a2dCommandException& e )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_CommandError, wxT( "%s" ),  e.getMessage().c_str() );
        oke = false;
    }

    if ( !oke )
    {
        if ( storeIt )
        {
            m_currentGroup->Remove( command );
            SetCurrentToLastActive();
        }

        SendBusyEvent( false, command );
        // the user code expects the command to be released/deleted
        return false;
    }

    SendBusyEvent( false, command );

    return true;
}

void a2dCommandProcessor::Store( a2dCommand* command )
{
    wxCHECK_RET( command, _( "no command in a2dCommandProcessor::Store" ) );

    // If there are too many command in the command list, delete one
    if ( m_currentGroup->m_subcommands.size() == m_maxNoCommands )
        m_currentGroup->m_subcommands.pop_front();

    // If the complete command list in currentgroup was undone, m_currentCommand will be 0
    // In this case, all commands in current group must be cleared.
    // This also happens if no commands have been added yet, but in this case
    // it doesn't hurt to clear the list.
    if( !m_currentCommand )
        ClearCommands();
    else
        m_currentGroup->ClearAfterCurrentCommand( m_currentCommand );

    m_currentCommand = command;
    m_currentGroup->m_subcommands.push_back( command );
    //FindActiveGroup()->m_subcommands.push_back( command );
    SetMenuStrings();
}

bool a2dCommandProcessor::Undo()
{
    if ( m_currentCommand && m_currentCommand->CanUndo() )
    {
        if ( UndoCommand( *m_currentCommand ) )
        {
            m_currentCommand = m_currentGroup->FindPrevious( m_currentCommand );
            SetMenuStrings();
            return true;
        }
    }

    return false;
}

bool a2dCommandProcessor::Redo()
{
    a2dCommand* redoCommand = ( a2dCommand* ) NULL;

    if ( m_currentCommand )
    {
        // is there anything to redo?
        a2dCommand* next = m_currentGroup->FindNext( m_currentCommand );
        if ( next )
        {
            redoCommand = next;
        }
    }
    else // no current command, redo the first one
    {
        // m_currentCommand is 0 is all commands have been undone
        // OR if no command was inserted yet
        if ( !m_currentGroup->m_subcommands.empty() )
            redoCommand = m_currentGroup->m_subcommands.front();
    }

    if ( redoCommand )
    {
        bool success = RedoCommand( *redoCommand );
        if ( success )
        {
            m_currentCommand = redoCommand;
            SetMenuStrings();
            return true;
        }
    }
    return false;
}

bool a2dCommandProcessor::CanUndo() const
{
    a2dCommand* command = GetCurrentCommand();

    if ( command && command->CanUndo() )
    {
        a2dCommandProcessorEvent event( wxEVT_CANUNDO, command );
        a2dCommandProcessor* ncmp = const_cast< a2dCommandProcessor* >(this);
        event.SetEventObject( ncmp );
        bool processed = ncmp->ProcessEvent( event );
        if ( event.GetVeto() )
            return false;
        return true;
    }

    return command && command->CanUndo();
}

bool a2dCommandProcessor::CanRedo() const
{
    if( m_currentCommand )
    {
        a2dCommand* next = m_currentGroup->FindNext( m_currentCommand );
        if ( next && next->CanUndo() )
        {
            a2dCommandProcessorEvent event( wxEVT_CANREDO, next );
            a2dCommandProcessor* ncmp = const_cast< a2dCommandProcessor* >(this);
            event.SetEventObject( ncmp );
            bool processed = ncmp->ProcessEvent( event );
            if ( event.GetVeto() )
                return false;
            return true;
        }
        return false;
    }
    else
    {
		return m_currentGroup->m_subcommands.size() != 0;
    }
}

void a2dCommandProcessor::Initialize()
{
    SetCurrentToLastActive();
    SetMenuStrings();
}

void a2dCommandProcessor::SetMenuStrings()
{
    // sent event wxEVT_MENUSTRINGS
    a2dCommandProcessorEvent event(
        m_currentCommand,
        GetUndoMenuLabel(),
        CanUndo(),
        GetRedoMenuLabel(),
        CanRedo()
    );
    event.SetEventObject( this );
    ProcessEvent( event );
}

// Gets the current Undo menu label.
wxString a2dCommandProcessor::GetUndoMenuLabel() const
{
    wxString buf;
    if ( m_currentCommand )
    {
        wxString commandName( m_currentCommand->GetName() );
        if ( commandName.IsEmpty() ) commandName = _( "Unnamed command" );
        bool canUndo = m_currentCommand->CanUndo();
        if ( canUndo )
            buf = wxString( _( "&Undo" ) ) + wxT( " " ) + commandName + m_undoAccelerator;
        else
            buf = wxString( _( "Can't &Undo" ) ) + wxT( " " ) + commandName + m_undoAccelerator;
    }
    else
    {
        buf = _( "&Undo" ) + m_undoAccelerator;
    }

    return buf;
}

// Gets the current Undo menu label.
wxString a2dCommandProcessor::GetRedoMenuLabel() const
{
    a2dCommand* redoCommand;
    wxString buf;

    if ( m_currentCommand )
    {
        // We can redo, if we're not at the end of the history.
        redoCommand = m_currentGroup->FindNext( m_currentCommand );
    }
    else
    {
        redoCommand = m_currentGroup->m_subcommands.empty() ? 0 : m_currentGroup->m_subcommands.front();
    }
    if ( redoCommand )
    {
        wxString redoCommandName( redoCommand->GetName() );
        if ( redoCommandName.IsEmpty() ) redoCommandName = _( "Unnamed command" );
        buf = wxString( _( "&Redo" ) ) + wxT( " " ) + redoCommandName + m_redoAccelerator;
    }
    else
    {
        buf = _( "&Redo" ) + m_redoAccelerator;
    }
    return buf;
}

void a2dCommandProcessor::ClearCommands()
{
    m_currentGroup->m_subcommands.clear();

    m_currentCommand = 0;
}

bool a2dCommandProcessor::SetOrAddPropertyToObject( a2dObject* propRefObject, const wxString& name, const wxString& value, bool withUndo )
{
    wxASSERT_MSG( propRefObject != 0 , _( "a2dObject not NULL" ) );

    // get the id object from the id hash table
    a2dPropertyId* id = propRefObject->HasPropertyId( name );
    if( !id )
        return false;

    a2dNamedProperty* prop = id->CreatePropertyFromString( value );

    if( !prop )
        return false;

    return Submit( new a2dCommand_SetProperty( propRefObject, prop ), withUndo );
}

bool a2dCommandProcessor::SetOrAddPropertyToObject( a2dObject* propRefObject, a2dNamedProperty* property, bool withUndo )
{
    wxASSERT_MSG( propRefObject != 0, _( "a2dObject not NULL" ) );

    return Submit( new a2dCommand_SetProperty( propRefObject, property ), withUndo );
}

void a2dCommandProcessor::SetCurrentGroup( a2dCommandGroup* group )
{
    group->m_active = true;
    m_currentGroup = group;
}

a2dCommandGroup* a2dCommandProcessor::CommandGroupBegin( const wxString& name )
{
    a2dCommandGroup* group = new a2dCommandGroup( name, m_currentGroup );
#if defined(A2D_COMMAND_DEBUG)
    wxLogDebug( wxT( "Open Group = %s 0x%p" ), name.c_str(), group );
#endif
    group->SetCommandProcessor( this );
    //store in the group m_currentGroup!!
    Store( group );
    //now change to the new group
    m_currentGroup = group;
    return group;
}

void a2dCommandProcessor::CommandGroupBegin( a2dCommandGroup* group )
{
    group->SetParentGroup( m_currentGroup );
    group->m_active = true;

#if defined(A2D_COMMAND_DEBUG)
    wxLogDebug( wxT( "Open Group = %s 0x%p" ), name.c_str(), group );
#endif
    group->SetCommandProcessor( this );
    //store in the group m_currentGroup!!
    Store( group );
    //now change to the new group
    m_currentGroup = group;
}

void a2dCommandProcessor::CommandGroupEnd( a2dCommandGroup* group )
{
    m_currentGroup = group->m_parentGroup;
    group->m_active = false;
    /*
    a2dCommandGroup* activeGroup = FindActiveGroup();
    wxASSERT( activeGroup == m_currentGroup );
    */
#if defined(A2D_COMMAND_DEBUG)
    wxLogDebug( wxT( "End Group = %s 0x%p" ), group->GetGroupName().c_str(), group );
#endif
    //parent group must be active
    wxASSERT( m_currentGroup->m_active );
    SetCurrentToLastActive();
    SetMenuStrings();
}

// ----------------------------------------------------------------------------
// a2dGeneralModule
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dGeneralModule, wxModule )

//! a global pointer to get to global instance of important classes.
A2DGENERALDLLEXP a2dSmrtPtr<a2dGeneralGlobal> a2dGeneralGlobals = new a2dGeneralGlobal();
//a2dGeneralGlobal* a2dGeneralGlobals = NULL;


bool a2dGeneralModule::OnInit()
{
    a2dGeneralGlobals = new a2dGeneralGlobal();
    return true;
}

void a2dGeneralModule::OnExit()
{
    a2dGeneralGlobals = NULL;
}

// ----------------------------------------------------------------------------
// a2dGeneralGlobal
// ----------------------------------------------------------------------------

a2dPathList a2dGeneralGlobal::m_configpath = a2dPathList();
bool a2dGeneralGlobal::m_directlog = true;
a2dErrorVector a2dGeneralGlobal::m_errors = a2dErrorVector();
wxArrayInt a2dGeneralGlobal::m_ignoredErrorIds = wxArrayInt();
a2dVariablesHash a2dGeneralGlobal::m_variableList = a2dVariablesHash();
a2dPropertyIdList a2dGeneralGlobal::m_dynamicIdList = a2dPropertyIdList();

IMPLEMENT_CLASS( a2dGeneralGlobal, a2dObject )

a2dGeneralGlobal::a2dGeneralGlobal()
{
    m_logConnectedEvents = false;
    m_directlog = true;
}

a2dGeneralGlobal::~a2dGeneralGlobal()
{
    m_errors.clear();
}

a2dObject* a2dGeneralGlobal::DoClone( CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    wxLogMessage( _( "Not implemented" ) );
    return NULL;
}

#if wxART2D_USE_CVGIO
void a2dGeneralGlobal::DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) )
{
    wxLogMessage( _( "Not implemented" ) );
}

void a2dGeneralGlobal::DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) )
{
    wxLogMessage( _( "Not implemented" ) );
}

#endif //wxART2D_USE_CVGIO

void a2dGeneralGlobal::ResetErrors()
{
    m_errors.clear();
    //wxLog* target = wxLog::GetActiveTarget();
    //target->Flush();
}

void a2dGeneralGlobal::IgnoreError( unsigned int id )
{
    if( m_ignoredErrorIds.Index( id ) == wxNOT_FOUND )
    {
        m_ignoredErrorIds.Add( id );
    }
}

void a2dGeneralGlobal::ReportError( const a2dError& error, const wxString& errorstr )
{
    if( m_ignoredErrorIds.Index( error.GetErrorCode() ) != wxNOT_FOUND )
        return;

    a2dError errorc( error );
    if ( !errorstr.IsEmpty() )
        errorc.SetErrorMessage( errorstr );

    m_errors.push_back( errorc  );

    if ( m_directlog )
        wxLogError( errorc.GetErrorMessage() );
}


#if wxCHECK_VERSION(2,9,0)

#if !wxUSE_UTF8_LOCALE_ONLY
void a2dGeneralGlobal::DoPrintfWchar( const a2dError& error, const wxChar* format, ... )
{
    va_list args;
    va_start( args, format );
    wxString out;

    out.PrintfV( format, args );
    va_end( args );

    ReportError( error, out );
}

void a2dGeneralGlobal::DoPrintfWcharWarn( const a2dError& error, const wxChar* format, ... )
{
    va_list args;
    va_start( args, format );
    wxString out;

    out.PrintfV( format, args );
    va_end( args );

    ReportWarning( error, out );
}

#endif // !wxUSE_UTF8_LOCALE_ONLY

#if wxUSE_UNICODE_UTF8
void a2dGeneralGlobal::DoPrintfUtf8( const a2dError& error, const char* format, ... )
{
    va_list args;
    va_start( args, format );
    wxString out;

    out.PrintfV( format, args );
    va_end( args );

    ReportError( error, out );
}

void a2dGeneralGlobal::DoPrintfUtf8Warn( const a2dError& error, const char* format, ... )
{
    va_list args;
    va_start( args, format );
    wxString out;

    out.PrintfV( format, args );
    va_end( args );

    ReportError( error, out );
}

#endif // wxUSE_UNICODE_UTF8

#else
/*
void a2dGeneralGlobal::ReportErrorFF( const a2dError& error, ... )
{
    wxString errorformat = error.GetErrorMessage();
    if ( errorformat.IsEmpty() )
        errorformat = wxT("%s");

    va_list ap;

    wxString errorstr;
    va_start( ap, errorformat );

    errorstr.PrintfV( errorformat, ap );
    va_end( ap );

    ReportError( error, errorstr );
}
*/
void a2dGeneralGlobal::ReportErrorF( const a2dError& error, const wxChar* Format, ... )
{
    va_list ap;

    wxString errorstr;
    va_start( ap, Format );

    errorstr.PrintfV( Format, ap );
    va_end( ap );

    ReportError( error, errorstr );
}

void a2dGeneralGlobal::ReportWarningF( const a2dError& error, const wxChar* Format, ... )
{
    va_list ap;

    wxString errorstr;
    va_start( ap, Format );

    errorstr.PrintfV( Format, ap );
    va_end( ap );

    ReportWarning( error, errorstr );
}

#endif //wxCHECK_VERSION(2,9,0)


void a2dGeneralGlobal::ReportWarning( const a2dError& error, const wxString& errorstr )
{
    if( m_ignoredErrorIds.Index( error.GetErrorCode() ) != wxNOT_FOUND )
        return;

    wxString errorsmes = error.GetErrorMessage();
    if ( !errorstr.IsEmpty() )
        errorsmes = errorstr;
    m_errors.push_back( a2dError( error.GetIdName(), errorsmes, true ) );


    if ( m_directlog )
        wxLogWarning( errorsmes );
}

wxString a2dGeneralGlobal::GetErrorsAsString()
{
    wxString errors;
    forEachIn( a2dErrorVector, &m_errors )
    {
        a2dErrorVector::value_type errorRecord = *iter;
        errors += errorRecord.GetErrorMessage() + wxT( ".\n" );
    }
    return errors;
}

void a2dGeneralGlobal::SendToLogTarget()
{
    if ( !m_directlog )
    {
        forEachIn( a2dErrorVector, &m_errors )
        {
            a2dErrorVector::value_type errorRecord = *iter;
            wxLogError( errorRecord.GetErrorMessage() );
        }
    }
    ResetErrors();
}

a2dErrorWarningCode a2dGeneralGlobal::GetLastErrorCode() const
{
    if ( m_errors.size() )
        return m_errors.back().GetErrorCode();
    return a2dError_NoError.GetErrorCode();
}

a2dError a2dGeneralGlobal::GetLastError() const
{
    if ( m_errors.size() )
        return m_errors.back();
    return a2dError_NoError;
}

void a2dGeneralGlobal::RecordF( wxObject* sender, const wxChar* Format, ... )
{
    va_list ap;

    wxString recordstring;
    va_start( ap, Format );

    //Format.Replace( wxT("%f"), wxT("%6.3f") );

    recordstring.PrintfV( Format, ap );
    va_end( ap );

    a2dCommandProcessorEvent event( wxEVT_RECORD, NULL );//recordstring );
    event.SetEventObject( sender );

    ProcessEvent( event );
}

void a2dGeneralGlobal::RecordF( const wxChar* Format, ... )
{
    va_list ap;

    wxString recordstring;
    va_start( ap, Format );

    //Format.Replace( wxT("%f"), wxT("%6.3f") );

    recordstring.PrintfV( Format, ap );
    va_end( ap );

    a2dCommandProcessorEvent event( wxEVT_RECORD, NULL );//recordstring );
    event.SetEventObject( this );

    ProcessEvent( event );
}

wxString* a2dGeneralGlobal::GetVariableString( const wxString& variablename )
{
    wxString*  prop = GetVariablesHash().GetVariableString( variablename );

    if( !prop )
    {
        a2dGeneralGlobals->ReportError( a2dError_GetVar, _( "wrong variable name, variable does not exist" ) );
        return NULL;// error, variable does not exist
    }

    return prop;
}

wxString a2dGeneralGlobal::GetWxArt2DVar( bool withSep ) const
{
    wxString art2Droot;
    wxGetEnv( wxT( "WXART2D" ), &art2Droot );
    wxFileName artdir = wxFileName( art2Droot, wxEmptyString );
    art2Droot = artdir.GetVolume() + artdir.GetVolumeSeparator() + artdir.GetPath(0, wxPATH_UNIX);

    if( art2Droot.IsEmpty() )
    {
#ifndef _GUNIX
        wxMessageBox( _( " WXART2D variable not set\nPlease set the WXART2D environment string to the correct dir.\n\nExample : WXART2D=c:\\wxArt2D\n\n" ), _( "environment error" ), wxOK );
#else
        wxMessageBox( _( " WXART2D variable not set\nPlease set the WXART2D environment string to the correct dir.\n\nExample : WXART2D=/user/home/wxArt2D; export WXART2D \n\n" ), _( "environment error" ), wxOK );
#endif
    }
    else
    {
        if ( withSep )
            return art2Droot + wxFileName::GetPathSeparator(wxPATH_UNIX);
    }
    return art2Droot;
}

wxString a2dGeneralGlobal::GetWxArt2DArtVar( bool withSep, bool silent ) const
{
    wxString* artdir = a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "WXART2D_ART" ) );
    wxString artroot;
    if ( !artdir || artdir->IsEmpty() )
    {
        wxGetEnv( wxT( "WXART2D_ART" ), &artroot );
        if( artroot.IsEmpty() )
        {
            wxGetEnv( wxT( "WXART2D" ), &artroot );
            if( artroot.IsEmpty() )
            {
                if ( !silent )
                {
#ifndef _GUNIX
                    wxMessageBox( _( "WXART2D_ART or WXART2D variable not set\nPlease set the WXART2D environment string to the correct dir.\n\nExample : WXART2D=c:\\wxArt2D\n\n" ), _( "environment error" ), wxOK );
#else
                    wxMessageBox( _( "WXART2D_ART or WXART2D variable not set\nPlease set the WXART2D environment string to the correct dir.\n\nExample : WXART2D=/user/home/wxArt2D; export WXART2D \n\n" ), _( "environment error" ), wxOK );
#endif
                }
                return artroot;
            }
            artroot = artroot + wxFileName::GetPathSeparator(wxPATH_UNIX) + wxT( "art" );
        }
    }
    else
        artroot = *artdir;
    if ( withSep )
        return artroot + wxFileName::GetPathSeparator(wxPATH_UNIX);
    return artroot;
}


// ----------------------------------------------------------------------------
// class a2dMenuIdItem
// ----------------------------------------------------------------------------

IMPLEMENT_CLASS( a2dMenuIdItem, wxMenuItem )

const a2dMenuIdItem a2dMenuIdItem::sm_noCmdMenuId = a2dMenuIdItem( wxT( "NoCommandId" ) );
bool a2dMenuIdItem::m_bitmapinitialized = false;

a2dMenuIdItemMap& a2dMenuIdItem::GetHashMap()
{
    static a2dMemoryCriticalSectionHelper helper;
    static a2dMenuIdItemMap ms_Name2Id;
    return ms_Name2Id;
}

a2dMenuIdItem::a2dMenuIdItem(  const wxString& menuIdName, const wxString& text, const wxString& help, wxItemKind kind )
{
    m_isEnabled   = true;
    m_isChecked   = false;
    m_id          = wxXmlResource::GetXRCID( menuIdName );
    m_kind        = kind;
    if ( m_id == wxID_ANY )
        m_id = wxNewId();
    if ( m_id == wxID_SEPARATOR )
        m_kind = wxITEM_SEPARATOR;

    SetText( text );
    SetHelp( help );

    m_name = menuIdName;
    if ( GetHashMap().find( m_name ) == GetHashMap().end() )
    {
        GetHashMap()[ m_name ] = this;
    }
    else
        wxFAIL_MSG( wxT( "duplicate id" ) );
}

void a2dMenuIdItem::InitializeBitmaps()
{
    a2dMenuIdItemMap::iterator iter;
    for( iter = GetHashMap().begin(); iter != GetHashMap().end(); ++iter )
    {
        a2dMenuIdItem& item = *( iter->second );
        //wxLogDebug( "%s", item.m_name );
        //if ( item.m_name == "CmdMenu_Selected_RotateObject90Right" )
        //    wxLogDebug( "%s", item.m_name );

        wxString art2d = a2dGeneralGlobals->GetWxArt2DArtVar();
        art2d += wxT( "resources/" ) ;

        wxImage image;
        if ( wxFileExists( art2d + item.m_name + wxT( ".ico" ) ) )
        {
            if ( image.LoadFile( art2d + item.m_name + wxT( ".ico" ), wxBITMAP_TYPE_ICO ) )
            {
                item.m_bmpChecked = wxBitmap( image );
                wxASSERT_MSG( item.m_bmpChecked.Ok(),
                              _( "Bitmap for a2dMenuIdItem wrong" ) );
            }
            item.m_bmpUnchecked =  item.m_bmpChecked;
        }
        if ( wxFileExists( art2d + item.m_name + wxT( ".bmp" ) ) )
        {
            if ( image.LoadFile( art2d + item.m_name + wxT( ".bmp" ), wxBITMAP_TYPE_BMP ) )
            {
                item.m_bmpChecked = wxBitmap( image );
                wxASSERT_MSG( item.m_bmpChecked.Ok(),
                              _( "Bitmap for a2dMenuIdItem wrong" ) );
            }
            item.m_bmpUnchecked =  item.m_bmpChecked;
        }
        if ( wxFileExists( art2d + item.m_name + wxT( ".png" ) ) )
        {
            if ( image.LoadFile( art2d + item.m_name + wxT( ".png" ), wxBITMAP_TYPE_PNG ) )
            {
                item.m_bmpChecked = wxBitmap( image );
                wxASSERT_MSG( item.m_bmpChecked.Ok(),
                              _( "Bitmap for a2dMenuIdItem wrong" ) );

            }
            item.m_bmpUnchecked =  item.m_bmpChecked;
        }
        if ( wxFileExists( art2d + item.m_name + wxT( "_Un.ico" ) ) )
        {
            if ( image.LoadFile( art2d + item.m_name + wxT( "_Un.ico" ), wxBITMAP_TYPE_ICO ) )
            {
                item.m_bmpUnchecked =  wxBitmap( image );
                wxASSERT_MSG( item.m_bmpUnchecked.Ok(),
                              _( "Bitmap for a2dMenuIdItem wrong" ) );
            }
        }
        if ( wxFileExists( art2d + item.m_name + wxT( "_Un.bmp" ) ) )
        {
            if ( image.LoadFile( art2d + item.m_name + wxT( "_Un.bmp" ), wxBITMAP_TYPE_BMP ) )
            {
                item.m_bmpUnchecked =  wxBitmap( image );
                wxASSERT_MSG( item.m_bmpUnchecked.Ok(),
                              _( "Bitmap for a2dMenuIdItem wrong" ) );
            }
        }
        if ( wxFileExists( art2d + item.m_name + wxT( "_Un.png" ) ) )
        {
            if ( image.LoadFile( art2d + item.m_name + wxT( "_Un.png" ), wxBITMAP_TYPE_PNG ) )
            {
                item.m_bmpUnchecked =  wxBitmap( image );
                wxASSERT_MSG( item.m_bmpUnchecked.Ok(),
                              _( "Bitmap for a2dMenuIdItem wrong" ) );
            }
        }


    }
    m_bitmapinitialized = true;
}

const a2dMenuIdItem& a2dMenuIdItem::GetItemByName( const wxString& menuIdName )
{
    a2dMenuIdItemMap::iterator iterCommand = GetHashMap().find( menuIdName );
    return iterCommand != GetHashMap().end() ? *iterCommand->second : a2dMenuIdItem::sm_noCmdMenuId;
}

#if wxUSE_ACCEL

wxAcceleratorEntry* a2dMenuIdItem::GetAccel() const
{
    return wxAcceleratorEntry::Create( GetText() );
}

void a2dMenuIdItem::SetAccel( wxAcceleratorEntry* accel )
{
    wxString text = m_text.BeforeFirst( wxT( '\t' ) );
    if ( accel )
    {
        text += wxT( '\t' );
        text += accel->ToString();
    }

    SetText( text );
}

#endif // wxUSE_ACCEL

void a2dMenuIdItem::SetText( const wxString& str )
{
    m_text = str;
}

void a2dMenuIdItem::SetHelp( const wxString& str )
{
    m_help = str;
}

wxString a2dMenuIdItem::GetLabelText( const wxString& label )
{
    return wxStripMenuCodes( label );
}

void a2dMenuIdItem::SetBitmaps( const wxBitmap& bmpChecked, const wxBitmap& bmpUnchecked )
{
    m_bmpChecked = bmpChecked;
    m_bmpUnchecked = bmpUnchecked;
}
