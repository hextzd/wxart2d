/*! \file wx/general/comevt.h
    \brief command processor and intializing and event handling classes specific for wxDocview.

    wxDocview has its own event handling classes which are almost equal to the wxWidgets ones.
    But had to be duplicated to be able to have a a2dObject at the top the class tree.
    So things like an a2dCanvasObject can be serialized, even if it can handle events.

    a2dGeneralGlobal is the class for storing basic stuff like error messages and application wide
    variables.

    a2dCommandProcessor is the basic class on which command processing is based.

    \author Klaas Holwerda
    \date Created 05/07/03

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: comevt.h,v 1.76 2009/08/23 19:49:38 titato Exp $
*/

#ifndef _COMEVTH__
#define _COMEVTH__

#include "wx/list.h"
#include "wx/event.h"
#include "wx/cmdproc.h"
#include <wx/module.h>

#if wxCHECK_VERSION(2,9,0)
#include "wx/chartype.h"
#include "wx/strvararg.h"
#endif

#include "wx/general/a2dlist.h"
#include "wx/general/gen.h"

// ============================================================================
// event handler and related classes
// ============================================================================

//! wxEvtHandler macro
#define EVT_BUTTON_ANY(func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_BUTTON_CLICKED, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast < wxCommandEventFunction > ( & func ), (wxObject *) NULL ),

//! wxEvthandler macro
#define EVT_CHOICE_ANY(func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_CHOICE_SELECTED, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast < wxCommandEventFunction > ( & func ), (wxObject *) NULL ),

//!  @} eventhandlers

#if defined(WXDOCVIEW_USINGDLL)
template class A2DGENERALDLLEXP a2dSmrtPtr<a2dObject>;
#endif



class A2DGENERALDLLEXP a2dCommandProcessor;
#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DGENERALDLLEXP a2dSmrtPtr<a2dCommandProcessor>;
#endif

class A2DGENERALDLLEXP a2dCommandId;

class A2DGENERALDLLEXP a2dHashMapCommandIds;
class A2DGENERALDLLEXP a2dHashMapCommandIds_wxImplementation_HashTable;
class A2DGENERALDLLEXP a2dHashMapCommandIds_wxImplementation_KeyEx;

//! This hash table is used for a2dCommandId with name
WX_DECLARE_HASH_MAP_WITH_DECL( wxString, a2dCommandId*, wxStringHash, wxStringEqual, a2dHashMapCommandIds, class A2DGENERALDLLEXP );
//#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
//template class A2DGENERALDLLEXP a2dHashMap<wxString,a2dCommandId*>::wxhash;
//template class A2DGENERALDLLEXP a2dHashMap<wxString,a2dCommandId*>;
//#endif

//! Each a2dCommand is given a command id at construction.
/*!
    The command id used to test for a specific commandId when intercepting command events.

<code>
    EVT_REDO( a2dObjectDerivedClass::OnComEvent )

    void a2dObjectDerivedClass::OnComEvent( a2dComEvent& event )
    {
        // now you can test what the command was.
        event.GetCommand()->GetCommandId()
    }

    EVT_REDO( wxEvtHandlerDerivedClass::OnComEvent )

    void wxEvtHandlerDerivedClass::OnComEvent( a2dComEvent& event )
    {
        // now you can test what the command was.
        event.GetCommand()->GetCommandId()
    }
</code>

    \ingroup commands commandid
*/
class A2DGENERALDLLEXP a2dCommandId
{
public:

    //! constructor
    a2dCommandId( const wxString& commandName );

    //! destructor
    virtual ~a2dCommandId() {};

    //! get name of class
    virtual wxString GetIdClassName() { return wxT( "a2dCommandId" ); }

    //! get name
    wxString GetName() const { return m_name; }

    //! search Id given the name of the command
    static const a2dCommandId& GetCommandByName( const wxString& commandName );

    //! return hash of commands
    static a2dHashMapCommandIds& GetHashMap();

private:

    //! name of command
    wxString m_name;
};

//! a base command for the a2dCommandProcessor
/*!
    When a2dCommandProcessor gets this command submitted, it will set itself
    to the command. The command will be added to the undo stack, and if it can be undone, the information
    to make this happen is part of the this command stored on the undo stack.
    So normally for each command one derives from a2dCommand, in order to have the derived command
    class store the right information for undoing an action, and define how to Do() and Undo() this action.

    To makes things a bit more flexible, this class is derived from a2dObject, therefore it can have a2dProperty's.
    This can be used to  add extra information on a command.

    \ingroup commands
*/
class A2DGENERALDLLEXP a2dCommand: public a2dObject
{

public:

    //! property for command id
    static const a2dCommandId sm_noCommandId;
    //! property for type of command id
    static const a2dCommandId sm_noCommandTypeId;
    //! property for group command id
    static const a2dCommandId sm_groupCommandId;

    //! constructor
    /*!
            \param canUndo can this command be undone
            \param commandId Id of the command
            \param commandTypeId Type Id of the command belong to a group of commands
            \param menuString command is associated with a menu string
    */
    a2dCommand( bool canUndo  = false,
                const a2dCommandId& commandId = sm_noCommandId,
                const a2dCommandId& commandTypeId = sm_noCommandTypeId,
                const wxString& menuString = wxEmptyString
              );

    //! destructor
    ~a2dCommand( void );

    inline a2dCommand* TClone( CloneOptions options = clone_deep, a2dRefMap* refs = NULL ) { return ( a2dCommand* ) Clone( options, refs ); }

    //! return command name
    //! if m_menuName is set this will be used for menu Undo/Redo labels, else the m_commandId its name is used.
    virtual wxString GetName() const;

    //! a specific id for this command.
    /*!
        A command can have an id set to it, to easily identify it.
        This can be done by m_commandName comparison, but id's are saver.
    */
    const a2dCommandId* GetCommandId() { return m_commandId; }

    //! used to identify groups of simular commands
    const a2dCommandId* GetCommandTypeId() { return m_commandTypeId; }

    //! Copy and instantiate the command
    /*! This is usefull for commands that are used as command templates in widgets.
        The widget owns a command template, that is not bound to an object. This
        function clones the command object and binds it to the given object.

        \\param object for which the command is to be generated.
    */
    virtual a2dCommand* CloneAndBind( a2dObject* object );

    //! Override this to perform a command
    virtual bool Do() = 0;

    //! Override this to undo a command
    virtual bool Undo() = 0;

    //! Override this to perform a dependency action before command is done.
    virtual bool PreDo() { return true; }

    //! Override this to perform a dependency action after command is done.
    virtual bool PostDo() { return true; }

    virtual bool CanUndo() const { return m_canUndo; }

    //! Override this to redo a command
    //! Default is the same as do
    virtual bool Redo();

    //! If this command changes the object such that it needs to flagged as modified (e.g. document )
    //! Some command like selecting objects on the canvas, do not need to be saved to the document.
    //! and in such a situation the command action can be undone, but the document is not flagged modified.
    bool Modifies() { return m_modifies; }

    //! If this command changes the object such that it needs to flagged as modified (e.g. document )
    //! Some command like selecting objects on the canvas, do not need to be saved to the document.
    //! and in such a situation the command action can be undone, but the document is not flagged modified.
	void SetModifies( bool modifies ) { m_modifies = modifies; }

    void SetSendBusyEvent( bool sendBusyEvent ) { m_sendBusyEvent = sendBusyEvent; }

    bool GetSendBusyEvent() const { return m_sendBusyEvent; }

    //! set when submitting this command via a2dCommandProcessor
    void SetCommandProcessor( a2dCommandProcessor* cmp ) { m_cmp = cmp; }

    a2dCommandProcessor* GetCommandProcessor() { return m_cmp; }

    //! Hierarchically clear all commands after current command
    /*!
        When storing commands, normally when no undo or redo is done yet, the command will be stored
        at the end of the undo stack. But if several Undo have bin done, the currentcommand is not pointing
        to the end of the stack. When at that moment a new command is submitted, this will be added after
        the current command, BUT first all commands after the current command will be cleared.
        As a result on can Redo all Undo's sofar as long as no new command is submitted yet.
    */
    virtual bool ClearAfterCurrentCommand( a2dCommand* current );

    //! This is like a wxDynamicCast, but much faster
    virtual class a2dCommandGroup* IsCommandGroup() { return 0; }

    //! Find the previous command of the given command
    virtual a2dCommand* FindPrevious( a2dCommand* current ) const;

    //! Find the next command of the given command
    virtual a2dCommand* FindNext( a2dCommand* current ) const;

    //! Remove the given command
    virtual bool Remove( a2dCommand* command );

    //! return true if this command/group are nested group contains the given command
    virtual bool ContainsCommand( a2dCommand* command );

    virtual void DistributeEvent( wxEventType eventType );

protected:


    //!Clone this object and return a pointer to the new object
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    void DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) )
    {
        wxLogMessage( _( "Not implemented" ) );
    }

    void DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) )
    {
        wxLogMessage( _( "Not implemented" ) );
    }

#endif //wxART2D_USE_CVGIO

    //! can this command be undone
    bool m_canUndo;

    bool m_modifies;

    bool m_sendBusyEvent;

    //! can be used to identify the command
    const a2dCommandId* m_commandId;

    //! if set can be used to identify groups of commands
    const a2dCommandId* m_commandTypeId;

    //! allows commands to get to the command processor that submitted the command.
    a2dCommandProcessor* m_cmp;

    //! if set this will be used for menu Undo/Redo labels, else the m_commandId its name is used.
    wxString m_menuString;

    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCommand( const a2dCommand& other );
};

typedef a2dSmrtPtr<a2dCommand> a2dCommandPtr;

//! For exceptions thrown from commands
/*!
    \ingroup commands commandid
*/
class a2dCommandException
{

public:
    //! Constructor.
    a2dCommandException( const wxString& exMessage ): m_message( exMessage ) {}

    //! Copy constructor.
    a2dCommandException( const a2dCommandException& other )
    {  m_message       = other.m_message; }

    a2dCommandException& operator=( const a2dCommandException& other )
    {
        if ( &other != this )
        {
            m_message = other.m_message;
        }

        return *this;
    }

    virtual ~a2dCommandException() {}

    wxString getMessage() const  { return m_message; }
    void setMessage( const wxString& exMessage )  { m_message = exMessage; }

protected:

    wxString m_message;
};

//! a list of commands used by the command processor or command groups

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DGENERALDLLEXP a2dSmrtPtr<a2dCommand>;
template class A2DGENERALDLLEXP std::allocator<class a2dSmrtPtr<class a2dCommand> >;
template class A2DGENERALDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dCommand>, std::allocator<class a2dSmrtPtr<class a2dCommand> > >::_Node >;
template class A2DGENERALDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dCommand>, std::allocator<class a2dSmrtPtr<class a2dCommand> > >::_Nodeptr >;
template class A2DGENERALDLLEXP std::list<class a2dSmrtPtr<class a2dCommand> >;
template class A2DGENERALDLLEXP a2dlist<class a2dSmrtPtr<class a2dCommand> >;
template class A2DGENERALDLLEXP a2dSmrtPtrList<class a2dCommand >;
#endif

typedef class A2DGENERALDLLEXP a2dSmrtPtrList< a2dCommand > a2dCommandList;

//! a group of commands, used to group commands together for undo/redo
/*!
    A group can be recorded distributed across the application, you start a command group, and you end it.
    See CommandGroupBegin() CommandGroupEnd().
    All command submitted in between become part of the group. This is very handy when commands lead to
    other commands indirectly. Like when moving objects connected with wires: moving an object will create
    a move command, but the wires will be re-routed, generating extra move commands for all moved wires.
    Combining those in a group, will Undo this as a whole, which is what a user does expect.
    Same for a polygon which is drawn vertex by vertex, while bussy doing this, an Undo action should remove
    the last added vertex. But when finishing a polygon, and drawing several more, the Undo should remove the
    polygon as a whole in one Undo action. This behaviour is all handled within a2dCommandProcessor(), it
    knows which group is currently active, and commands in nested groups are undone/redone at once, while single commands in the active
    group are undone one by one. Undo-ing/redo-ing a group, means it will undo/redo all seperate nested commands, but to the users it is
    as if it is one action.

    Another application of groups is simple to assemble a group of commands, which can be submitted as a new command.
    A group gets a unique a2dCommandId at construction.
    The group of commands is submitted in one row in Do().

    \ingroup commands
*/
class A2DGENERALDLLEXP a2dCommandGroup: public a2dCommand
{
public:

    //! constructor
    /*!
            \param name name of this command
            \param parent of this group
    */
    a2dCommandGroup( const wxString& name, a2dCommandGroup* parent = NULL );

    a2dCommandGroup( 
		        a2dCommandGroup* parent,
                const a2dCommandId& commandId = sm_noCommandId,
                const a2dCommandId& commandTypeId = sm_noCommandTypeId,
                const wxString& menuString = wxEmptyString
              );

    //! destructor
    ~a2dCommandGroup( void );

    inline a2dCommand* TClone( CloneOptions options = clone_deep, a2dRefMap* refs = NULL  ) { return ( a2dCommand* ) Clone( options, refs ); }

    //! return command name
    virtual wxString GetGroupName() const { return m_groupName; }

    //! erase commands with a certain id, starting  at fromcommand
    bool ClearCommandsById( const a2dCommandId& commandId, a2dCommand* fromcommand = NULL );

    //! Hierarchically clear all commands after current
    virtual bool ClearAfterCurrentCommand( a2dCommand* current );

    //! This is like a wxDynamicCast, but much faster
    virtual class a2dCommandGroup* IsCommandGroup() { return this; }

    //! Find the previous command of the given command
    virtual a2dCommand* FindPrevious( a2dCommand* current ) const;

    //! Find the next command of the given command
    virtual a2dCommand* FindNext( a2dCommand* current ) const;

    //! Remove the given command in this group or a subgroup
    virtual bool Remove( a2dCommand* command );

    //! add a command to the group
    void Add( a2dCommand* command );

    //! return true if this command/group are nested group contains the given command
    virtual bool ContainsCommand( a2dCommand* command );

    //! return the parent group of this group
    a2dCommandGroup* GetParentGroup() { return m_parentGroup; }

    //! set the parent group of this group
    void SetParentGroup( a2dCommandGroup* parent ) { m_parentGroup = parent; }

public:
    // Base class overloads
    virtual bool Do();
    virtual bool Undo();
    virtual bool Redo();

protected:

    //!Clone this object and return a pointer to the new object
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    //! this is the parent group of this group
    a2dCommandGroup* m_parentGroup;

    //! name of group of commands
    wxString m_groupName;

	bool m_ownId;

    //! If true, this command group is not yet closed and may still receive commands
    bool m_active;
    //! the list of subcommands
    a2dCommandList m_subcommands;
    //! a2DocumentCommandProcessor may access the list directly
    friend class a2dCommandProcessor;
};

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DGENERALDLLEXP a2dAutoZeroPtr< a2dObject >;
#endif

//! used to change a property on objects
/*!
    A a2dObject property is set on the a2dObject, and the first property
    on the a2dObject with the same name is used for undo.
    Properties with the starting with __M_ are wrapped properties, and or often used to set member variables
    of classes.
    Special name properties starting with just __ are used to set predefined properties.
    Both these type of properties can be set by this command.
    If the property was not yet set for the object, in Undo the new property is just removed, else
    the old value is restored.

    \ingroup commands
*/
class A2DGENERALDLLEXP a2dCommand_SetProperty: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_SetProperty )

public:

    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    //! for dynamic creation
    a2dCommand_SetProperty();

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

       \param object the canvas object to set the property on.
       \param property the property to set
    */
    a2dCommand_SetProperty( a2dObject* object, a2dNamedProperty* property );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the a2dPropertyId of the property to set.
        \param value the value to set the property to.
    */
    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdRefObject* id, a2dObject* value );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the a2dPropertyId of the property to set.
        \param value the value to set the property to.
    */
    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdObject* id, const wxObject& value );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the a2dPropertyId of the property to set.
        \param value the value to set the property to.
    */
    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdString* id, const wxString& value );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the a2dPropertyId of the property to set.
        \param value the value to set the property to.
    */
    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdBool* id, bool value );

    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdBool* id, const wxString& value );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the a2dPropertyId of the property to set.
        \param value the value to set the property to.
    */
    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdInt16* id, wxInt16 value );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the a2dPropertyId of the property to set.
        \param value the value to set the property to.
    */
    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdUint16* id, wxUint16 value );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the a2dPropertyId of the property to set.
        \param value the value to set the property to.
    */
    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdInt32* id, wxInt32 value );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the a2dPropertyId of the property to set.
        \param value the value to set the property to.
    */
    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdUint32* id, wxUint32 value );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the a2dPropertyId of the property to set.
        \param value the value to set the property to.
    */
    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdDouble* id, double value );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the a2dPropertyId of the property to set.
        \param colour the value to set the property to.
    */
    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyIdColour* id, const wxColour& colour );

    ~a2dCommand_SetProperty( void );

    //! copy constructor
    a2dCommand_SetProperty( const a2dCommand_SetProperty& other );


    //! property that is set packed as a2dNamedProperty.
    a2dNamedProperty* GetProperty() { return m_property; }

    //! property that is set packed as a2dNamedProperty.
    const a2dPropertyId* GetPropertyId() { return m_id; }

    a2dObject* GetObject() { return m_propRefObject.Get(); }

    bool Do();
    bool Undo();

protected:
    //! this is for derived classes and derived properties
    a2dCommand_SetProperty( a2dObject* object, const a2dPropertyId* id );

    //! all property references will be set NULL when this object, having the property, will be deleted.
    a2dAutoZeroPtr< a2dObject > m_propRefObject;

    //! property set to the object.
    a2dNamedPropertyPtr m_property;

    //! id of the property that is set.
    const a2dPropertyId* m_id;
};


/**********************************************
 New events Document and View events.
**********************************************/
BEGIN_DECLARE_EVENT_TYPES()
//! see a2dCommandProcessorEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DGENERALDLLEXP, wxEVT_BEGINBUSY, 1 )
//! see a2dCommandProcessorEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DGENERALDLLEXP, wxEVT_ENDBUSY, 1 )
//! see a2dCommandProcessorEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DGENERALDLLEXP, wxEVT_DO, 1 )
//! see a2dCommandProcessorEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DGENERALDLLEXP, wxEVT_UNDO, 1 )
//! see a2dCommandProcessorEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DGENERALDLLEXP, wxEVT_REDO, 1 )
//! see a2dCommandProcessorEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DGENERALDLLEXP, wxEVT_CANUNDO, 1 )
//! see a2dCommandProcessorEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DGENERALDLLEXP, wxEVT_CANREDO, 1 )
//! see a2dCommandProcessorEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DGENERALDLLEXP, wxEVT_MENUSTRINGS, 1 )
//! see a2dPropertyEditEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DGENERALDLLEXP, a2dEVT_PROPOBJECT_EDITPROPERTIES_EVENT, 1 )
DECLARE_EXPORTED_EVENT_TYPE( A2DGENERALDLLEXP, wxEVT_RECORD, 1 ) 
END_DECLARE_EVENT_TYPES()


//! sent from a2dObject::EditProperties().
/*!
    This event is sent, you intercept it and do the editing of the supplied properties there.

    Currently the following Id's:

    - ::a2dEVT_PROPOBJECT_EDITPROPERTIES_EVENT sent when a2dObject::EditProperties() requires editing of properties.

    \ingroup events
*/
class A2DGENERALDLLEXP a2dPropertyEditEvent: public a2dEvent
{
public:

    //! constructor
    a2dPropertyEditEvent( a2dObject* object, a2dNamedPropertyList* properties = NULL );

    //! constructor
    a2dPropertyEditEvent( const a2dPropertyEditEvent& other );

    //! clone the event
    virtual wxEvent* Clone() const { return new a2dPropertyEditEvent( *this ); }

    a2dNamedPropertyList* GetPropertyList() { return m_properties; }

    //! when properties where edited, this must become true
    bool GetEdited() { return m_edited; }

    //! set to signal that properties where edited.
    void SetEdited( bool edited ) { m_edited = edited; }

private:

    a2dNamedPropertyList* m_properties;

    bool m_edited;
};

//! event function for a2dPropertyEditEvent
typedef void ( wxEvtHandler::*a2dPropertyEditEventFunctionEvt )( a2dPropertyEditEvent& );

/*! \addtogroup eventhandlers
*  @{
*/

//! static event table macro for NON a2dCanvasObject when editing properties for wxEvtHandler
#define EVT_PROPOBJECT_EDITPROPERTIES_EVENT(func) DECLARE_EVENT_TABLE_ENTRY( a2dEVT_PROPOBJECT_EDITPROPERTIES_EVENT, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast < a2dPropertyEditEventFunctionEvt > ( & func ), (wxObject *) NULL ),

//!  @} eventhandlers

//! Event sent to a2dCommandProcessor
/*!
     - ::wxEVT_DO is sent from a2CommandProcessor or any other commandprocessor when a command is issued.
     - ::wxEVT_UNDO is sent from a2CommandProcessor or any other commandprocessor when a command is issued.

     - ::wxEVT_MENUSTRINGS is sent from a2dCommandProcessor when menu string need to be updated after a command.

    Use a2dObject::ConnectEvent to connect your own wxEvtHandler to the a2CommandProcessor
    to receive them.

    \ingroup commands events
*/
class A2DGENERALDLLEXP a2dCommandProcessorEvent : public a2dEvent
{

public:

    //! constructor
    /*!
        type should be ::wxEVT_DO ::wxEVT_UNDO

    */
    a2dCommandProcessorEvent( wxEventType type, a2dCommand* cmd )
        : a2dEvent( 0, type )
    {
        m_cmd = cmd;
        m_undoLabel = wxT( "" );
        m_redoLabel = wxT( "" );
        m_canUndo = false;
        m_canRedo = false;
        m_cursorIsSet = false;
        m_veto = false;
    }

    //! constructor
    /*!
        type ::wxEVT_MENUSTRINGS

        For ::wxEVT_MENUSTRINGS, you can use the following function, to get the info to set the menu string.
            -wxCommandProcessor::GetUndoMenuLabel()
            -wxCommandProcessor::GetRedoMenuLabel()
            -wxCommandProcessor::CanUndo()
            -wxCommandProcessor::CanRedo()
    */
    a2dCommandProcessorEvent( a2dCommand* cmd,
                              const wxString& undoLabel, bool canUndo,
                              const wxString& redoLabel, bool canRedo
                            )
        : a2dEvent( 0, wxEVT_MENUSTRINGS )
    {
        m_cmd = cmd;
        m_undoLabel = undoLabel;
        m_redoLabel = redoLabel;
        m_canUndo = canUndo;
        m_canRedo = canRedo;
        m_cursorIsSet = m_veto = false;
    }

    //! constructor
    a2dCommandProcessorEvent( const a2dCommandProcessorEvent& event )
        : a2dEvent( event )
    {
        m_cmd = event.m_cmd;
        m_undoLabel = event.m_undoLabel;
        m_redoLabel = event.m_redoLabel;
        m_canUndo = event.m_canUndo;
        m_canRedo = event.m_canRedo;
        m_cursorIsSet = event.m_cursorIsSet;
        m_veto = event.m_veto;
    }

    //! clone the event.
    virtual wxEvent* Clone() const { return new a2dCommandProcessorEvent( *this ); }

    //! the command ( if there was one ) that did it.
    a2dCommand* GetCommand() { return m_cmd; }

    //! by default this is a2dCommandProcessor::GetUndoMenuLabel()
    wxString& GetUndoMenuLabel() { return m_undoLabel; }
    //! by default this is a2dCommandProcessor::GetRedoMenuLabel()
    wxString& GetRedoMenuLabel() { return m_redoLabel; }
    //! by default this is a2dCommandProcessor::CanUndo()
    bool CanUndo() { return m_canUndo; }
    //! by default this is a2dCommandProcessor::CanRedo()
    bool CanRedo() { return m_canRedo; }

    //! when set, indicates that an earlier handler of this event did set the cursor
    void SetCursorSet( bool set ) { m_cursorIsSet = set; }
    //! check if an earlier handler of this event did set the cursor
    bool IsCursorSet() const { return m_cursorIsSet; }

    //! use to veto event for canundo canredo.
    void Veto( bool veto = true )
    {
        m_veto = veto;
    }

    //! If the even must be vetod this is set to true.
    void SetVeto( bool canVeto ) { m_veto = canVeto; }

    //! If the event can be vetod this return true
    bool GetVeto() const { return m_veto; }

protected:

    //! veto flag.
    bool m_veto;

    //! see GetCommand()
    a2dCommand* m_cmd;

    //! used for wxEVT_MENUSTRINGS event
    wxString m_undoLabel;

    //! used for wxEVT_MENUSTRINGS event
    bool m_canUndo;

    //! used for wxEVT_MENUSTRINGS event
    wxString m_redoLabel;

    //! used for wxEVT_MENUSTRINGS event
    bool m_canRedo;

    bool m_cursorIsSet;
};

/*! \addtogroup eventhandlers
*  @{
*/

typedef void ( wxEvtHandler::*wxCommandProcessorEventFunction )( a2dCommandProcessorEvent& );

//! event sent from a2DocumentCommandProcessor when a command is initially done
#define EVT_DO(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_DO, wxID_ANY, wxID_ANY, (wxObjectEventFunction)  static_cast< wxCommandProcessorEventFunction > (& func), (wxObject *) NULL ),
//! event sent from a2DocumentCommandProcessor when a command is undone
#define EVT_UNDO(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_UNDO, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast< wxCommandProcessorEventFunction > (& func), (wxObject *) NULL ),
//! event sent from a2DocumentCommandProcessor when a command is redone
#define EVT_REDO(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_REDO, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast< wxCommandProcessorEventFunction > (& func), (wxObject *) NULL ),

//! event sent from a2DocumentCommandProcessor when a command is undone
#define EVT_CANUNDO(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_CANUNDO, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast< wxCommandProcessorEventFunction > (& func), (wxObject *) NULL ),
//! event sent from a2DocumentCommandProcessor when a command is redone
#define EVT_CANREDO(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_CANREDO, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast< wxCommandProcessorEventFunction > (& func), (wxObject *) NULL ),

//! event sent to a2dCommandProcessorEvents to adjust menu strings (e.g. for current undo/redo command)
#define EVT_MENUSTRINGS(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_MENUSTRINGS, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast< wxCommandProcessorEventFunction > (& func), (wxObject *) NULL ),
//! event sent to a2dCommandProcessorEvents to set modified flag of a document or drawing flag.
#define EVT_MODIFIES(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_MODIFIES, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast< wxCommandProcessorEventFunction > (& func), (wxObject *) NULL ),

//! event sent from a2DocumentCommandProcessor when a command submit/execute is starting
#define EVT_BEGINBUSY(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_BEGINBUSY, wxID_ANY, wxID_ANY, (wxObjectEventFunction)  static_cast< wxCommandProcessorEventFunction > (& func), (wxObject *) NULL ),
//! event sent from a2DocumentCommandProcessor when a command submit/execute is ending
#define EVT_ENDBUSY(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_ENDBUSY, wxID_ANY, wxID_ANY, (wxObjectEventFunction)  static_cast< wxCommandProcessorEventFunction > (& func), (wxObject *) NULL ),

//!  @} eventhandlers

// ----------------------------------------------------------------------------
// a2dCommandProcessor
// ----------------------------------------------------------------------------

//! base command processor
/*!
    a2dCommand commands submitted to this class, are stored on a stack.
    Because commands can be nested, the command processor actually has not a stack, but a tree of nested
    commands. The tree is implemented by nesting a2dDocviewCommand's.

    One can have a command stack for the whole application, and sub command stacks for documents.
    So undo can be called seperate for all documents, or even parts of documents.

    See a2dCommand for how to organize undo information.

    \ingroup commands events
*/
class A2DGENERALDLLEXP a2dCommandProcessor : public a2dObject
{
public:

    //!Constructor.
    /*!
        \param maxCommands maximum of commands on the command stack, default -1 is unlimited.
    */
    a2dCommandProcessor( int maxCommands = -1 );

    //! destructor
    virtual ~a2dCommandProcessor();

    //! next to the base class submit, it sets a2DocumentCommandProcessor for a2dCommand
    /*
        Pass a command to the processor. The processor calls Do(); if
        successful, is appended to the command history unless storeIt is false.

        Internal sends a a2dDocumentCommandEvent with id ::wxEVT_DO
    */
    virtual bool Submit( a2dCommand* command, bool storeIt = true );

    //! just store the command without executing it
    virtual void Store( a2dCommand* command );

    //! Undo one command or command group
    virtual bool Undo();
    //! Redo one command or command group
    virtual bool Redo();
    //! Are there commands to undo and can they be undone ?
    virtual bool CanUndo() const;
    //! Are there commands to redo and can they be redone ?
    virtual bool CanRedo() const;

    //! Initialises the current command and menu strings.
    virtual void Initialize();

    //! en/disable busy event sending
    void PushSendBusyEvent( bool sendBusyEvent );

    bool PopSendBusyEvent();

    //! get busy event sending flag value
    bool GetSendBusyEvent() const { return m_sendBusyEvent; }

    //! sends a a2dCommandProcessorEvent event( wxEVT_BEGINBUSY, command )
    //! or a2dCommandProcessorEvent event( wxEVT_ENDBUSY, command )
    //! to this class. This is done before and after executing/submitting a command.
    //! You can intercept this event to set e.g  a busy cursor.
    //! - EVT_BEGINBUSY( a2dCentralCanvasCommandProcessor::OnBusyBegin )
    //! - EVT_ENDBUSY( a2dCentralCanvasCommandProcessor::OnBusyEnd )
    void SendBusyEvent( bool start, a2dCommand* command = NULL );

    //! does sent an event to update menu strings after a command
    /*
        Use a2dObject::ConnectEvent to connect your own wxEvtHandler to the a2CommandProcessor
        to receive them.

        Sets the Undo/Redo menu strings somewhere the application intercepts this event.
        Sends ::wxEVT_MENUSTRINGS the application should react by setting menu strings.

        In event tables use like:
        <code>
            BEGIN_EVENT_TABLE( classX, baseclassX )
                EVT_MENUSTRINGS( classX::OnSetMenuStrings )
            END_EVENT_TABLE()
        </code>

        Assuming you have in classX this:

        <code>
            m_edit_menu = new wxMenu;
            m_edit_menu->Append(wxID_UNDO, _("&Undo"));
            m_edit_menu->Append(wxID_REDO, _("&Redo"));
        </code>

        In OnSetMenuStrings do this:

        <code>
            void classX::OnSetMenuStrings( a2dCommandProcessorEvent& event )
            {
                if ( event.GetEventObject() == m_view->GetDocument()->GetCommandProcessor() )
                {
                    m_edit_menu->SetLabel(wxID_UNDO, event.GetUndoMenuLabel());
                    m_edit_menu->Enable(wxID_UNDO, event.CanUndo());

                    m_edit_menu->SetLabel(wxID_REDO, event.GetRedoMenuLabel());
                    m_edit_menu->Enable(wxID_REDO, event.CanRedo());
                }
            }
        </code>

        \remark overide this function in derived class, if you need other labels etc.
    */
    virtual void SetMenuStrings();

    //! Gets the current Undo menu label.
    wxString GetUndoMenuLabel() const;

    //! Gets the current Undo menu label.
    wxString GetRedoMenuLabel() const;

    //! command list access
    a2dCommand* GetCurrentCommand() const
    {
        return m_currentCommand;
    }

    //! get the limit on number of commands to store
    int GetMaxCommands() const { return m_maxNoCommands; }

    //! return  top/startgroup
    a2dCommandGroup* GetRootGroup() const { return m_rootGroup; }

    //! return  current group
    a2dCommandGroup* GetCurrentGroup() const { return m_currentGroup; }

    //! set current group
    /*! 
        When a derived a2dCommandGroup, is Submitted, its Do(0 member will normally gather the sub commands for the group.
        In this case, the group itself is already Submitted, and will be stored, only make sure using this function
        that the subcommands that are submitted, end up in this group.
    */
    void SetCurrentGroup( a2dCommandGroup* group );

    //! remove all commands stored
    virtual void ClearCommands();

    //! By default, the accelerators are "\tCtrl+Z" and "\tCtrl+Y"
    const wxString& GetUndoAccelerator() const { return m_undoAccelerator; }
    //! By default, the accelerators are "\tCtrl+Z" and "\tCtrl+Y"
    const wxString& GetRedoAccelerator() const { return m_redoAccelerator; }

    //! By default, the accelerators are "\tCtrl+Z" and "\tCtrl+Y"
    void SetUndoAccelerator( const wxString& accel ) { m_undoAccelerator = accel; }
    //! By default, the accelerators are "\tCtrl+Z" and "\tCtrl+Y"
    void SetRedoAccelerator( const wxString& accel ) { m_redoAccelerator = accel; }

    //! set a named property to the given object
    /*! \remark This function should only be used if the id comes as string from the user or a file.
                The id must be in the id list and the type is derived from the found id.
        \param propRefObject object to set the property to.
        \param name the name of the property to add.
        \param value the value to set the property to.
        \param withUndo if true the setting of this property can be undone.
    */
    virtual bool SetOrAddPropertyToObject( a2dObject* propRefObject, const wxString& name, const wxString& value = wxT( "" ), bool withUndo = true );

    //! set a named property to the given object
    /*!
        \param propRefObject object to set the property to.
        \param property the property (id and value) to add.
        \param withUndo if true the setting of this property can be undone.
    */
    virtual bool SetOrAddPropertyToObject( a2dObject* propRefObject, a2dNamedProperty* property, bool withUndo = true );

    //! Start a new command group
    /*! The return value is given to CommandGroupEnd. */
    a2dCommandGroup* CommandGroupBegin( const wxString& name );

    //! Start a new command group
    void CommandGroupBegin( a2dCommandGroup* group );

    //! End a command group
    /* \param group the return value of the matching CommandGroupBegin */
    void CommandGroupEnd( a2dCommandGroup* group );

    //! when releasing this, release first all command, since they may hold commands referencing this
    //! command processor, so it would never be deleted.
    bool SmrtPtrRelease();

protected:

    //! find the currently active command group
    a2dCommandGroup* FindActiveGroup();

    //! Set the current command to the last command in the active group
    void SetCurrentToLastActive();

    // for further flexibility, command processor doesn't call a2dCommand::Do()
    // and Undo() directly but uses these functions which can be overridden in
    // the derived class

    //! Called before doing the actual command
    virtual bool DoPreCommand( a2dCommand& cmd );

    //! Called after doing the actual command
    virtual bool DoPostCommand( a2dCommand& cmd );

    //! sends a a2dCommandProcessorEvent with id ::wxEVT_DO
    virtual bool DoCommand( a2dCommand& cmd );

    //! sends a a2dCommandProcessorEvent with id ::wxEVT_UNDO
    virtual bool UndoCommand( a2dCommand& cmd );

    //! sends a a2dCommandProcessorEvent with id ::wxEVT_REDO
    virtual bool RedoCommand( a2dCommand& cmd );

    //! maximum number of commands to store
    wxUint32      m_maxNoCommands;
    //! this is the root of the command group tree
    a2dSmrtPtr< a2dCommandGroup > m_rootGroup;

    //! this is the parent group of the current command ( which may be a group itself )
    a2dSmrtPtr< a2dCommandGroup > m_currentGroup;

    //! this is the tree-trace to the currently active command
    a2dCommandPtr m_currentCommand;

    //! associated undo accelerator attached to menu
    wxString      m_undoAccelerator;
    //! associated redo accelerator
    wxString      m_redoAccelerator;

    bool m_sendBusyEvent;

    std::vector<bool> m_busyStack;

private:

    //!Clone this object and return a pointer to the new object
	virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    DECLARE_DYNAMIC_CLASS( a2dCommandProcessor )
};

//! help class which makes it possible to store a smart pointer as a wxObject*
/*!
    If a class X has a member wxObject* m_obj, this pointer can normally not be used for storing a
    a2dSmrtPtr which is a template class not derived from wxObject.
    This class here is  wxObject derived, and the smart pointer is maintained within.
    On deletion of this class it will delete the smart pointer, releasing the object pointed to.

    \ingroup general
*/
template<class T>
class a2dSmrtPtrWrap: public wxObject
{
public:

    a2dSmrtPtrWrap( T* caller )
    {
        m_smrtP = caller;
    }

    ~a2dSmrtPtrWrap()
    {
    }

    a2dSmrtPtr<T> m_smrtP;
};

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DGENERALDLLEXP a2dSmrtPtrWrap< a2dCommand >;
#endif

//! used to connect a command to a wxObject
//! The wxObject derived class, holds a smart pointer to a command. The wxObject can be stored as m_callbackUserData in events.
typedef a2dSmrtPtrWrap< a2dCommand >  a2dCommandStore;

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DGENERALDLLEXP std::allocator< wxEventType >;
template class A2DGENERALDLLEXP std::allocator< std::_List_nod< wxEventType, std::allocator< wxEventType > >::_Node >;
template class A2DGENERALDLLEXP std::allocator< std::_List_ptr< wxEventType, std::allocator< wxEventType > >::_Nodeptr >;
template class A2DGENERALDLLEXP std::list< wxEventType >;
template class A2DGENERALDLLEXP a2dlist< wxEventType >;
#endif

//! One Global instance of this class exists, in order to get to global needed objects.
/*!
    Error from within the docview frame work and derived applications/libraries,
    use this class to report errors and warning too.
    They are maintained in a list, and in each a2dError, you will find the error code, and the
    message that went with it.

    Default error messages will be reported to wxLogError, if m_directlog is false, they will only be stored.

    From this class several other are derived, and they will share the same static information.
    Like the errors reported, and the user variables etc.

    \ingroup global general
*/
class A2DGENERALDLLEXP a2dGeneralGlobal : public a2dObject
{

    DECLARE_CLASS( a2dGeneralGlobal )

public:

    a2dGeneralGlobal();

    virtual ~a2dGeneralGlobal();

    //! reset the error report to empty.
    /*!
        All erros in the error list are cleared.
    */
    virtual void ResetErrors();

    //! concatenate to the the error report the given error.
    virtual void ReportError( const a2dError& error, const wxString& errorstr = wxEmptyString );

    //! concatenate to the the error report the given error.
#if !wxCHECK_VERSION(2,9,0)
    virtual void ReportErrorF( const a2dError& error, const wxChar* Format, ... );
#else
    WX_DEFINE_VARARG_FUNC_VOID( ReportErrorF, 2, ( const a2dError&, const wxFormatString& ), DoPrintfWchar, DoPrintfUtf8 )
#endif
    //! concatenate to the error report the given warning.
    virtual void ReportWarning( const a2dError& error, const wxString& errorstr );

    //! concatenate to the error report the given warning.
#if !wxCHECK_VERSION(2,9,0)
    virtual void ReportWarningF( const a2dError& error, const wxChar* Format, ... );
#else
    WX_DEFINE_VARARG_FUNC_VOID( ReportWarningF, 2, ( const a2dError&, const wxFormatString& ), DoPrintfWcharWarn, DoPrintfUtf8Warn )
#endif

    //! get the errors found sofar.
    static a2dErrorVector& GetErrors() { return m_errors; }

    //! concatenate all errors found into one string.
    wxString GetErrorsAsString();

    //! all stored errors and warning are sent to log target using wxLogError()
    /*!
        Specific use is when m_directlog is false, but still errors or assembled,
        calling this function will sent to the logging target anyway.
        One can ignore all errors with ResetErrors().
    */
    void SendToLogTarget();

    //! code of the last warning or error
    a2dErrorWarningCode GetLastErrorCode() const;

    a2dError GetLastError() const;

    virtual void RecordF( wxObject* sender, const wxChar* Format, ... );

    virtual void RecordF( const wxChar* Format, ... );

    //! Set logging to wxLog target on or off
    void SetDoLog( bool onOff ) { m_directlog = onOff; }

    //! Is logging to wxLog target on or off?
    bool GetDoLog() { return m_directlog; }

    //! Add an error to the list of ignored errors
    void IgnoreError( unsigned int id );

    //! aliases list for setting internal variables
    a2dVariablesHash& GetVariablesHash() { return m_variableList; }

    //! get a variable from the internal aliases list
    wxString* GetVariableString( const wxString& variablename );

    //! Path(s) for configuration file(s) in an application
    a2dPathList& GetConfigPathList() { return m_configpath; }

    //! Get a reference to the central list of  dynamic created property id's.
    a2dPropertyIdList& GetPropertyIdList() { return m_dynamicIdList; }

    wxString GetWxArt2DVar( bool withSep = true ) const;

    wxString GetWxArt2DArtVar( bool withSep = true, bool silent = false ) const;

    void SetLogConnectedEvents( bool logConnectedEvents ) { m_logConnectedEvents = logConnectedEvents; }

    bool GetLogConnectedEvents() { return m_logConnectedEvents; }

protected:

#if wxCHECK_VERSION(2,9,0)
#if !wxUSE_UTF8_LOCALE_ONLY
    virtual void DoPrintfWchar( const a2dError&, const wxChar* format, ... );
    virtual void DoPrintfWcharWarn( const a2dError&, const wxChar* format, ... );
#endif
#if wxUSE_UNICODE_UTF8
    virtual void DoPrintfUtf8( const a2dError&, const char* format, ... );
    virtual void DoPrintfUtf8Warn( const a2dError&, const char* format, ... );
#endif
#endif

    //!  Path(s) for configuration file(s) in an application
    static a2dPathList m_configpath;

    //! logging to wxLog target on or off
    static bool m_directlog;

    //! list of all possible errors
    static a2dErrorVector m_errors;

    //! list of error id's to be ignored.
    static wxArrayInt m_ignoredErrorIds;

    //! aliaslist containing internal variables
    static a2dVariablesHash m_variableList;

    static a2dPropertyIdList m_dynamicIdList;

    bool m_logConnectedEvents;

private:

    //!Clone this object and return a pointer to the new object
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

};

//! initializes the general module
/*!
    The global variable a2dGeneralGlobals is set here.

    \ingroup global
    \ingroup general
*/
class A2DGENERALDLLEXP a2dGeneralModule : public wxModule
{
public:

    a2dGeneralModule() {}

    virtual bool OnInit();
    virtual void OnExit();

private:

    DECLARE_DYNAMIC_CLASS( a2dGeneralModule )
};

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DGENERALDLLEXP a2dSmrtPtr<a2dGeneralGlobal>;
#endif

//! only instance of a2dGeneralGlobal, to get to settings for dynamic variables and errors.
A2DGENERALDLLEXP_DATA ( extern a2dSmrtPtr<a2dGeneralGlobal> ) a2dGeneralGlobals;
//A2DGENERALDLLEXP_DATA (extern a2dGeneralGlobal* ) a2dGeneralGlobals;






#define A2D_ARGUMENT_SETTER( TYPE, ARGUMENT_NAME ) \
    Args & \
    ARGUMENT_NAME( const TYPE& arg) { \
        this->argValue.ARGUMENT_NAME = arg; \
        this->argSet.ARGUMENT_NAME = true; \
        return *this; \
    }

//! Set a string variable inside wxDocview
/*!
    \ingroup commands
*/
class A2DGENERALDLLEXP a2dCommand_SetVariable: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_SetVariable )

public:

    //! Set a string variable inside wxDocview
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    class A2DGENERALDLLEXP Args
    {
    public:
        Args()
        {
            argSet.varname = false;
            argSet.varvalue = false;
        }

        ~Args()
        {
        }

        A2D_ARGUMENT_SETTER( wxString, varname )
        A2D_ARGUMENT_SETTER( wxString, varvalue )

        struct argValuet
        {
            wxString varname; wxString varvalue;
        } argValue;

        struct argSett
        {
            bool varname, varvalue;
        } argSet;
    };

    a2dCommand_SetVariable( const Args& args = Args() )
    {
        m_args.argValue.varname = args.argSet.varname ? args.argValue.varname : wxString( wxEmptyString ); //error throw
        m_args.argValue.varvalue = args.argSet.varvalue ? args.argValue.varvalue : wxString( wxEmptyString );
    }

private:

    Args m_args;

    virtual bool Do()
    {
        a2dGeneralGlobals->GetVariablesHash().SetVariableString( m_args.argValue.varname, m_args.argValue.varvalue );
        return true;
    }
    virtual bool Undo()
    {
        return false;
    }
};

//! Set a environment variable
/*!
    \ingroup commands
*/
class A2DGENERALDLLEXP a2dCommand_SetEnvironmentVariable: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_SetEnvironmentVariable )

public:

    //! Set a string environment variable
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    class A2DGENERALDLLEXP Args
    {
    public:
        Args()
        {
            argSet.varname = false;
            argSet.varvalue = false;
        }

        ~Args()
        {
        }

        A2D_ARGUMENT_SETTER( wxString, varname )
        A2D_ARGUMENT_SETTER( wxString, varvalue )

        struct argValuet
        {
            wxString varname; wxString varvalue;
        } argValue;

        struct argSett
        {
            bool varname, varvalue;
        } argSet;
    };

    a2dCommand_SetEnvironmentVariable( const Args& args  = Args() )
    {
        m_args.argValue.varname = args.argSet.varname ? args.argValue.varname : wxString( wxEmptyString ); //error throw
        m_args.argValue.varvalue = args.argSet.varvalue ? args.argValue.varvalue : wxString( wxEmptyString );
    }

private:

    Args m_args;

    virtual bool Do()
    {
        if ( wxSetEnv( m_args.argValue.varname, m_args.argValue.varvalue ) )
            return true;

#if wxCHECK_VERSION(2,9,0)
        a2dGeneralGlobals->ReportErrorF( a2dError_SetEnv, _( " SetEnv() Error: Could not set Environment variable %s = %s\n" ),
                                         m_args.argValue.varname, m_args.argValue.varvalue );
#else
        a2dGeneralGlobals->ReportErrorF( a2dError_SetEnv, _( " SetEnv() Error: Could not set Environment variable %s = %s\n" ),
                                         m_args.argValue.varname.c_str(), m_args.argValue.varvalue.c_str() );
#endif
        return true;
    }
    virtual bool Undo()
    {
        return false;
    }
};

//! Get internal variable
/*!
    \ingroup commands
*/
class A2DGENERALDLLEXP a2dCommand_GetVariable: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_GetVariable )

public:

    //! Set a string environment variable
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    class A2DGENERALDLLEXP Args
    {
    public:
        Args()
        {
            argSet.varname = false;
            argSet.varvalue = false;
        }

        ~Args()
        {
        }


        A2D_ARGUMENT_SETTER( wxString, varname )
        A2D_ARGUMENT_SETTER( wxString, varvalue )

        struct argValuet
        {
            wxString varname; wxString varvalue;
        } argValue;

        struct argSett
        {
            bool varname, varvalue;
        } argSet;
    };

    a2dCommand_GetVariable( const Args& args  = Args() )
    {
        m_args.argValue.varname = args.argSet.varname ? args.argValue.varname : wxString( wxEmptyString );
        m_args.argValue.varvalue = args.argSet.varvalue ? args.argValue.varvalue : wxString( wxEmptyString );
    }

private:

    Args m_args;

    virtual bool Do()
    {
        const a2dNamedProperty*  propfound = a2dGeneralGlobals->GetVariablesHash().GetVariable( m_args.argValue.varname );
        if( !propfound )
        {
            a2dGeneralGlobals->ReportError( a2dError_GetVar, _( "wrong variable name, variable does not exist" ) );
            return false;
        }

        m_args.argSet.varvalue = true;
        m_args.argValue.varvalue = propfound->StringValueRepresentation();
        return true;
    }
    virtual bool Undo()
    {
        return false;
    }
};

//! Get internal variable
/*!
    \ingroup commands
*/
class A2DGENERALDLLEXP a2dCommand_GetEnvVariable: public a2dCommand
{

    DECLARE_DYNAMIC_CLASS( a2dCommand_GetEnvVariable )

public:

    //! Set a string environment variable
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    class A2DGENERALDLLEXP Args
    {
    public:
        Args()
        {
            argSet.varname = false;
            argSet.varvalue = false;
        }

        ~Args()
        {
        }


        A2D_ARGUMENT_SETTER( wxString, varname )
        A2D_ARGUMENT_SETTER( wxString, varvalue )

        struct argValuet
        {
            wxString varname; wxString varvalue;
        } argValue;

        struct argSett
        {
            bool varname, varvalue;
        } argSet;
    };

    a2dCommand_GetEnvVariable( const Args& args = Args() )
    {
        m_args.argValue.varname = args.argSet.varname ? args.argValue.varname : wxString( wxEmptyString );
        if ( !args.argSet.varname || m_args.argValue.varname.IsEmpty() )
            a2dGeneralGlobals->ReportError( a2dError_GetEnv, _( "wrong variable name, variable may not be empty" ) );
    }

private:

    Args m_args;

    virtual bool Do()
    {
        wxString envValue;
        if( !wxGetEnv( m_args.argValue.varname, &envValue ) )
        {
            a2dGeneralGlobals->ReportError( a2dError_GetEnv, _( "environment variable does not exist" ) );
            return false;   // Environment name not found
        }

        m_args.argSet.varvalue = true;
        m_args.argValue.varvalue = envValue;
        return true;
    }
    virtual bool Undo()
    {
        return false;
    }
};


#include <map>
class A2DGENERALDLLEXP a2dMenuIdItem;
typedef std::map< wxString, a2dMenuIdItem* > a2dMenuIdItemMap;

//! store a menu Id generated by XRCID( menuIdString ) plus a menustring and helpstring
/*!
    The class instance can be used to add directly to a menu
*/
class A2DGENERALDLLEXP a2dMenuIdItem : public wxObject
{
public:

    DECLARE_CLASS( a2dMenuIdItem )

    //! constant to defined non valid id.
    static const a2dMenuIdItem sm_noCmdMenuId;

    //! constructor
    a2dMenuIdItem( const wxString& menuIdName = wxEmptyString,
                   const wxString& text = wxEmptyString,
                   const wxString& help = wxEmptyString, wxItemKind kind = wxITEM_NORMAL );
    //! destructor
    virtual ~a2dMenuIdItem() {};

    //! set id
    void SetId( int itemid ) { m_id = itemid; }

    //! get id
    int  GetId() const { return m_id; }

    //! check if same id.
    bool SameId( const a2dMenuIdItem& other ) const { return m_id == other.m_id; }

    // the item's text (or name)
    //
    // NB: the item's text includes the accelerators and mnemonics info (if
    //     any), i.e. it may contain '&' or '_' or "\t..." and thus is
    //     different from the item's label which only contains the text shown
    //     in the menu
    virtual void SetText( const wxString& str );

    //! get label text
    wxString GetLabel() const { return GetLabelText( m_text ); }
    //! get plain text for menu
    const wxString& GetText() const { return m_text; }

    //! get the label from text (implemented in platform-specific code)
    static wxString GetLabelText( const wxString& text );

    //! what kind of menu item we are
    wxItemKind GetKind() const { return m_kind; }
    void SetKind( wxItemKind kind ) { m_kind = kind; }

    //! set if menu is checkable
    virtual void SetCheckable( bool checkable ) { m_kind = checkable ? wxITEM_CHECK : wxITEM_NORMAL; }

    //! get if menu is checkable
    bool IsCheckable() const
    { return m_kind == wxITEM_CHECK || m_kind == wxITEM_RADIO; }

    //! initial state of menu
    virtual void Enable( bool enable = true ) { m_isEnabled = enable; }
    //! initial state of menu
    virtual bool IsEnabled() const { return m_isEnabled; }

    //! set initial check state of menu
    virtual void Check( bool check = true ) { m_isChecked = check; }
    //! get initial check state of menu
    virtual bool IsChecked() const { return m_isChecked; }
    //! toggel initial check state of menu
    void Toggle() { Check( !m_isChecked ); }

    // set help string (displayed in the status bar by default)
    void SetHelp( const wxString& str );
    // get help string (displayed in the status bar by default)
    const wxString& GetHelp() const { return m_help; }

    void SetBitmaps( const wxBitmap& bmpChecked,
                     const wxBitmap& bmpUnchecked = wxNullBitmap );
    void SetBitmap( const wxBitmap& bmp ) { SetBitmaps( bmp ); }
    const wxBitmap& GetBitmap( bool checked = true ) const
    { return checked ? m_bmpChecked : m_bmpUnchecked; }

    void SetDisabledBitmap( const wxBitmap& bmpDisabled )
    { m_bmpDisabled = bmpDisabled; }
    const wxBitmap& GetDisabledBitmap() const
    { return m_bmpDisabled; }

#if wxUSE_ACCEL
    // get our accelerator or NULL (caller must delete the pointer)
    virtual wxAcceleratorEntry* GetAccel() const;

    // set the accel for this item - this may also be done indirectly with
    // SetText()
    virtual void SetAccel( wxAcceleratorEntry* accel );
#endif // wxUSE_ACCEL

    //! name of the menu coupled to unique id
    wxString GetIdName() const { return m_name; }

    //! search Id given the name of the command
    static const a2dMenuIdItem& GetItemByName( const wxString& menuIdName );

    //! stored map of names to get id
    static a2dMenuIdItemMap& GetHashMap();

    //! initialize bitmaps associated with found a2dMenuIdItem's from files
    static void InitializeBitmaps();

    //! Are the bitmaps for button bar etc. initlialized?
    static bool GetInitialized() { return m_bitmapinitialized; }

private:

    int           m_id;             // numeric id of the item >= 0 or wxID_ANY or wxID_SEPARATOR
    wxString      m_text,           // label of the item
                  m_help;           // the help string for the item
    wxItemKind    m_kind;           // separator/normal/check/radio item?
    bool          m_isChecked;      // is checked?
    bool          m_isEnabled;      // is enabled?

    // the bitmaps (may be invalid, then they're not used)
    wxBitmap m_bmpChecked,
             m_bmpUnchecked,
             m_bmpDisabled;

    //! name of menu id
    wxString m_name;

    // were bitmaps already initialized?
    static bool m_bitmapinitialized;
};

#define DECLARE_MENU_ITEMID( menuName ) extern a2dMenuIdItem& menuName() ; 

#define DEFINE_MENU_ITEMID( menuName, menuText, menuHelp ) a2dMenuIdItem& menuName() \
{ \
    static a2dMenuIdItem item( wxT(#menuName), menuText, menuHelp ); \
    return item; \
}

#define DEFINE_MENU_ITEMID_KIND( menuName, menuText, menuHelp, kind ) a2dMenuIdItem& menuName () \
{ \
    static a2dMenuIdItem item( wxT(#menuName), menuText, menuHelp, (kind) ); \
    return item; \
}

#define DECLARE_EXPORTED_EVTIDFUNC( expdecl, eventName ) \
	DECLARE_EXPORTED_EVENT_TYPE( expdecl, eventName, 1 ) \
    extern wxEventType& GETID_##eventName(); 


#define DECLARE_EVTIDFUNC( eventName ) \
	DECLARE_EVENT_TYPE( eventName, 1 ) \
    extern wxEventType& GETID_##eventName(); 

#define DEFINE_EVTIDFUNC( eventName ) \
wxEventType& GETID_##eventName() \
{ \
    static wxEventType type = wxNewEventType(); \
    return type; \
} \
const wxEventType eventName = GETID_##eventName();


#endif // _COMEVTH__
