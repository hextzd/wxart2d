/*! \file wx/general/id.h
    \brief Id based property system with its base a2dPropertyId.

    Properties in wxDocview are template based. Each property type is a unique instance of a
    specific class, and that is what its ID is. A derived property class with an ID can be
    used by other classes like a2dObject to store properties in itself. Each property
    with a specific ID can store the specific information where that property ID class was
    designed for.

    \author Michael S�trop
    \date Created 12/12/2003

    Copyright: 2003-2004 (c) Michael S�trop

    Licence: wxWidgets Licence

    RCS-ID: $Id: id.h,v 1.31 2008/10/30 23:18:09 titato Exp $
*/

#ifndef __ID_H__
#define __ID_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/hashmap.h"
#include "wx/filename.h"
#include "wx/tipwin.h"
#include <wx/uri.h>


#include "wx/general/smrtptr.h"

//! This macro is used to set as the name of a class
/*!
    a2dPropertyId knows how to create a a2dNamedProperty from itself, and a a2dPropertyId
    can be found by its name from the hash of all a2dPropertyId's created.
*/
#define CLASSNAME(name) wxT(#name)

//! A class for id's of various kinds
/*! a2dIdBase is used in situations, where traditionally an enumeration type is used.
    The disadvantage of an enumeration is, that it cannot easily be expanded by derived
    classes. If this would be done, all derived classes would share the same id
    range and would have to know each other. a2dIdBase solves this.
    Instead of a enum value, a distinct a2dIdBase object is used for identification.
    The identification is usually done by address. The a2dIdBase objects are generally
    static members of a class of global variables, so that they are accessable by
    all objects requiring the id.
    A a2dIdBase should always be derived of, so that different ID classes cannot be
    inadvertedly intermixed.
    A a2dIdBase cannot easily be serialized, because the address will change. To
    do this, it must be converted to a string form. Derived classes will have ways to
    do this.
    Another way to have unique IDs is to use wxNewId. The advantage of a2dIdBase is,
    that it can be extended to be serialized and that it need not be initialized.

    \ingroup general
*/
class A2DGENERALDLLEXP a2dIdBase
{
public:
    a2dIdBase() {};
    virtual ~a2dIdBase() {};
    virtual wxString GetIdClassName() { return wxT( "a2dIdBase" ); }
};

// some forward declarations required by a2dPropertyId

class A2DGENERALDLLEXP a2dObject;
class A2DGENERALDLLEXP a2dPropertyId;
class A2DGENERALDLLEXP a2dNamedProperty;
class A2DGENERALDLLEXP a2dInt16Property;
class A2DGENERALDLLEXP a2dInt32Property;
class A2DGENERALDLLEXP a2dUint16Property;
class A2DGENERALDLLEXP a2dUint32Property;
class A2DGENERALDLLEXP a2dBoolProperty;
class A2DGENERALDLLEXP a2dFloatProperty;
class A2DGENERALDLLEXP a2dDoubleProperty;
class A2DGENERALDLLEXP a2dVoidPtrProperty;
class A2DGENERALDLLEXP a2dObjectProperty;
class A2DGENERALDLLEXP a2dStringProperty;
class A2DGENERALDLLEXP a2dFileNameProperty;
class A2DGENERALDLLEXP a2dDateTimeProperty;
class A2DGENERALDLLEXP a2dMenuProperty;
class A2DGENERALDLLEXP a2dUriProperty;
class A2DGENERALDLLEXP a2dWindowProperty;
class A2DGENERALDLLEXP a2dTipWindowProperty;
class A2DGENERALDLLEXP a2dColourProperty;
class A2DGENERALDLLEXP a2dArrayStringProperty;
class A2DGENERALDLLEXP a2dProperty;
class A2DGENERALDLLEXP a2dAutoZeroProperty;
class A2DGENERALDLLEXP a2dNamedPropertyList;

//! Declaration of the hash map type to convert prop names to prop id objects
WX_DECLARE_STRING_HASH_MAP( a2dPropertyId*, a2dPropertyIdHashMap );

//! This is the base class for all kinds of property id's for a2dObject
/*! Some facts about a2dPropertyId and its derivatives

    - A a2dPropertyId supplies a unique id, that identifies dynamic properties or
       members and fields of objects.
    - A a2dPropertyId knows the value type of a property and can access
       polymorphic property objects in a typesafe way.
    - A a2dPropertyId may access structured members/properties e.g.
       members/properties of members/properties.

    The a2dPropertyId has a major role in setting a property to an object
    If a property or member is set to an object, a2dPropertyId does:

    - Create a local or dynamic property object of proper type from a value.
    - Sets an individual property/member to an individual object. It can do
      this either directly (for PTM property ids) or ask the target object
      to do it (via DoExchangeMembers or DoAddProperty).
      The last is more flexible, but slower.

    The first task can obviously only be performed by the property id and not
    by the target object, because the target object does not have type specific
    functions.

    The second task could be performed by the target object directly in many
    cases, but there are a lot of exceptions, where a propertyId can do it easier:

    - PTM property ids can do this more efficiently
    - Property Ids want to control if a property is set as member, list property,
       if it is set recursively or not.

    A property or member is set to an object via the following functions

    -  a2dPropertyIdTyped::SetPropertyToObject(obj, value):
       This function creates from the value a local property object and calls
       SetProperty( obj, property ).

    -  a2dObject::SetProperty(): This function sets the
       given property object to an object.
       The property object given may not be inserted into
       an object list directly, because it is owned by somebody else or because it is
       not a dynamic object (i.e. sitting on the stack). So if the function needs
       to be inserted into a property list, it is cloned before. If the property
       is set as a member, this is not required.

    -  a2dObject::DoExchangeMembers
       This function calls the a2dPropertyIdXXX::ExchangeMember function of all properties, for which
       the object has members. The ExchangeMember function of the property id finally sets or
       collects, or enumerates one member variable.

    -  a2dObject::DoAddProperty
       This function finally adds a property object to the property list of a a2dObject.

    \ingroup general property

*/
class A2DGENERALDLLEXP a2dPropertyId
{
public:
    //! Flags for property ids
    enum Flags
    {
        //! no special flags set
        flag_none           = 0x00000000,
        //! the property is temporary and never saved
        flag_temporary      = 0x00000001,
        //! transfer this property via a command processor
        /*! This is most usefull for general properties like styles in a drawing
            applications.
            You should set flag_onlymemberhint as well.
        */
        flag_transfer       = 0x00000002,
        //! set this property only as member, never as property list item
        flag_onlymember     = 0x00000004,
        //! this is a hint for DoIgnoreIfNotMember in derived classes
        /* This is usefull for properties that are sometimes set as non-members,
           e.g. in the command processor, and sometimes not, e.g. in objects
           for which the property doesn't make sense.
           This flag is in now way evaluated by the base property system.
           It is intended to evaluate this flag in derived classes DoIgnoreIfNotMember.
        */
        flag_onlymemberhint = 0x00000008,
        //! Search the list before searching members
        /*! This is an optimization flag for properties that are most frequently found
            in the list (e.g. stroke, fill, ... )
        */
        flag_listfirst      = 0x00000010,
        //! Allow adding a specific property with ID more than once to a list
        /*! If this Id refers to a member, it is still added only once
        */
        flag_multiple       = 0x00000020,
        //! When cloning, and this flag is set, the property is cloned deep, else not.
        /*!
            In cases like editing, temporary properties are added to a document, which are removed later on.
            Those properties are not rendered, and only store a pointer to some other a2dObject.
            In such cases a clone should not be made of the a2dObject pointed to.
            In cases like fill and stroke properties, it is often required.
        */
        flag_clonedeep      = 0x00000040,
        //! Search the list only for this dynamic property
        /*! This is an optimization flag for properties that are only found
            in the list
        */
        flag_listonly       = 0x00000080,

        //! when set this type of property has no influence on rendering
        flag_norender       = 0x00000100,

        //! A property id defined by user
        flag_userDefined    = 0x00000200,

        //! A property id for a property which is editable
        flag_isEditable     = 0x00000400,

        //! If a property with this id has changed its parent/holder will be notified.
        flag_notify         = 0x00000800
    };
    friend Flags operator | ( Flags a, Flags b ) { return ( Flags )( ( int )a | ( int )b ); }
    friend Flags operator & ( Flags a, Flags b ) { return ( Flags )( ( int )a & ( int )b ); }

    //! Flags used for manipulating the way a property is set to a2dObject
    enum SetFlags
    {
        //! no special flags set
        set_none                 = 0x00000000,
        //! set the property also to all childs
        /*! This is NOT evaluated by the property code.
            This serves a s a hint to a2dObject derived classes
        */
        //! set the property only as a member, never as a list item
        set_onlymember           = 0x00000010,

        //! if the property is found as member, set it, otherwise remove it from the list
        set_remove_non_member    = 0x00000020,

        //! remove the property, don't set it as member
        set_remove_property      = 0x00000040
    };
    friend SetFlags operator | ( SetFlags a, SetFlags b ) { return ( SetFlags )( ( int )a | ( int )b ); }

public:
    //! Default constructor
    a2dPropertyId( const wxString& name, Flags flags );

    //! destructor
    virtual ~a2dPropertyId() {};
    //! Get the ids print and serialization name
    virtual wxString GetName() const { return m_name; }
    //! Set the ids print and serialization name
    void SetName( const wxString& name ) { m_name = name; }
    //! Get the main (pre-dot) part of a hierarchical id
    virtual const a2dPropertyId* GetMainID() const { return this; }
    //! Get the sub (post-dot) part of a hierarchical id
    virtual const a2dPropertyId* GetSubID() const { return 0; }
    //! Check if this id is equal to another id
    virtual bool operator == ( const a2dPropertyId& other ) const { return &other == this; }
    //! Create a new property object from a value string
    virtual a2dNamedProperty* CreatePropertyFromString( const wxString& WXUNUSED( value ) ) const { return 0; };

    virtual a2dNamedProperty* GetPropertyAsNamedProperty( const a2dObject* obj ) const { return 0; };

    //! Set the property in obj to value (not cloning value)
    virtual void SetNamedPropertyToObject( a2dObject* obj, a2dNamedProperty* nprop, SetFlags setflags = set_none ) const;

    //! Test if this id identifies a property that needs to be cloned deep.
    bool IsCloneDeep() const { return ( m_flags & flag_clonedeep ) != 0; }
    //! Test if this id identifies a temporary property
    bool IsTemporary() const { return ( m_flags & flag_temporary ) != 0; }
    //! true if this property is transfered via e.g. a command processor
    bool IsTransfer() const { return ( m_flags & flag_transfer ) != 0; }
    //! true if this property is only set as member
    bool IsOnlyMember() const { return ( m_flags & flag_onlymember ) != 0; }
    //! true if this property has the only set as member hint set
    bool IsOnlyMemberHint() const { return ( m_flags & flag_onlymemberhint ) != 0; }
    //! true if this property searches the list first, then members
    bool IsListFirst() const { return  ( m_flags & flag_listfirst ) != 0; }
    //! true if this property searches the list only
    bool IsListOnly() const { return  ( m_flags & flag_listonly ) != 0; }
    //! true if this property is user defined
    bool IsUserDefined() const { return  ( m_flags & flag_userDefined ) != 0; }
    //! true if this property is editable ( can be tested in a property editor ).
    bool IsEditable() const { return  ( m_flags & flag_isEditable ) != 0; }

    //! check a flag mask (all given flags must be set)
    bool CheckFlags( Flags flags ) const { return ( m_flags & flags ) == flags; }
    //! check a property id and flags to see if the property shall be collected
    bool CheckCollect( const a2dPropertyId* id, Flags flags ) const { return ( !id || id == this ) && ( ( m_flags & flags ) == flags ); }

    //! Dynamic properties with this id will be removed from the object
    /*!
        \param obj the object to remove the properties from
        \param setflags must at least be  set_remove_property ( default ), can be extended to be more specific.
    */
    void RemovePropertyFromObject( a2dObject* obj ) const;

protected:

    //! this constructor is only for property ids referencing other property ids
    a2dPropertyId( );

    //! this is called by derived classes to add a list property in an object
    /*! This functions exists because the derived classes are not friend of a2dObject.
        This function gives access to a2dObject::AddProperty.
    */
    inline void DoAddPropertyToObject( a2dObject* obj, a2dNamedProperty* prop ) const;

    //! this is called by derived to check if a property should be added as non-member
    /*! This functions exists because the derived classes are not friend of a2dObject.
        This function gives access to a2dObject::DoIgnoreIfNotMember.
    */
    inline bool DoIgnoreIfNotMember( a2dObject* obj ) const;

    //! this is called by derived classes to change an id in a property
    /*! This functions exists because the derived classes are not friend of a2dNamedProperty.
        This function gives access to a2dNamedProperty.
    */
    static inline void DoSetPropertyId( a2dNamedProperty* prop, a2dPropertyId* id );

protected:

    wxString m_name;

    Flags m_flags;

private:
    //!how many references to this object do exist
    int m_refcount;

    //! Call to have a new owner for this object
    /*! This function should only be called by a2dSmrtPtr

        \remark owning mean that the object calling this member needs to call Release at some time,
                in order to actually release/delete the object.

        \return The return value is the object itself, which is now owned on time extra.

        increment refcount by 1 ( use when adding a reference to this object)
    */
    a2dPropertyId* SmrtPtrOwn() { m_refcount++; return this; }

    //!To release the object, it is not longer owned by the calling object.
    /*! This function should only be called by a2dSmrtPtr
    */
    bool SmrtPtrRelease()
    {
        m_refcount--;
        wxASSERT_MSG( m_refcount >= 0, wxT( "a2dPropertyId Own/Release not matched (extra Release calls)" ) );
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

#if defined(WXDOCVIEW_USINGDLL)
template class A2DGENERALDLLEXP std::allocator<a2dPropertyId* >;
template class A2DGENERALDLLEXP std::allocator< std::_List_nod<a2dPropertyId*, std::allocator<a2dPropertyId* > >::_Node >;
template class A2DGENERALDLLEXP std::allocator< std::_List_ptr<a2dPropertyId*, std::allocator<a2dPropertyId* > >::_Nodeptr >;
template class A2DGENERALDLLEXP std::list<a2dPropertyId* >;
template class A2DGENERALDLLEXP a2dlist<a2dPropertyId* >;
template class A2DGENERALDLLEXP a2dPtrList<a2dPropertyId>;

template class A2DGENERALDLLEXP std::allocator<wxUint16 >;
template class A2DGENERALDLLEXP std::allocator< std::_List_nod<wxUint16, std::allocator<wxUint16 > >::_Node >;
template class A2DGENERALDLLEXP std::allocator< std::_List_ptr<wxUint16, std::allocator<wxUint16 > >::_Nodeptr >;
template class A2DGENERALDLLEXP std::list<wxUint16 >;
template class A2DGENERALDLLEXP a2dlist<wxUint16 >;
#endif

//! list to hold a set of property id objects pointers
typedef a2dSmrtPtrList<a2dPropertyId> a2dPropertyIdList;

//! This template class is for property ids meant for properties that do not encapsulate another type.
/*! For some properties, the property itself is the value. These properties use a2dPropertyIdProp as id.
    Other properties encapsulate a value. These properties use wxProperyIdTyped as id.

    For example, a2dShadowStyleProperty is a property which is normally added as a dynamic property
    to an a2dCanvasObject. It directly holds the information/value for shadow in its members.
    So we will use:
    <code>
        typedef a2dPropertyIdProp<class a2dShadowStyleProperty> a2dPropertyIdCanvasShadowStyle;
    </code>

    If however the property is just holding a single object, it is more handy to use a2dPropertyIdTyped
    as template, since this has many function to get/set that object without further effort.
    So for a a2dBoundingBox in a a2dBoudingBoxProperty it is best to use this:
    <code>
        typedef a2dPropertyIdTyped<a2dBoundingBox, a2dBoudingBoxProperty> a2dPropertyIdBoundingBox;
    </code>

    \ingroup general property

*/
template<class proptype>
class a2dPropertyIdProp : public a2dPropertyId
{
public:
    //! Default constructor
    a2dPropertyIdProp( const wxString& name, Flags flags ) : a2dPropertyId( name, flags )
    {
    }

    ~a2dPropertyIdProp() {}

    //! Set the property in obj to value (cloning value)
    void SetPropertyToObject( a2dObject* obj, const proptype& value ) const;
    //! Set the property in obj to value (not cloning value)
    void SetPropertyToObject( a2dObject* obj, proptype* value ) const;

    //! retrieve in form of a a2dNamedProperty the value of the property identified with this id
    /*!
        The property id is defining the way a a2dNamedProperty can be set or retrieved from the a2dObject.
        In this class only via dynamic properties.

        \param obj the object on which the property needs to be retrieved
    */
    proptype* GetPropertyClone( const a2dObject* obj ) const;

    //! retrieve in form of a a2dNamedProperty the property identified with this id
    /*!
        The property id is defining the way a a2dNamedProperty can be set or retrieved from the a2dObject.
        The a2dPropertyIdProp is only used for dynamic properties.

        \param obj the object on which the property needs to be set ot retrieved
    */
    a2dNamedProperty* GetPropertyAsNamedProperty( const a2dObject* obj ) const;

    //! Set the property in obj to value (not cloning value)
    void SetNamedPropertyToObject( a2dObject* obj, a2dNamedProperty* nprop, SetFlags setflags = set_none ) const;

    //! Get the property in obj (not cloned)
    proptype* GetProperty( a2dObject* obj ) const;
    //! Get the property in obj (const, not cloned)
    const proptype* GetProperty( const a2dObject* obj ) const;

protected:
    //! this constructor is only for property ids referencing other property ids
    a2dPropertyIdProp( ) : a2dPropertyId( ) {};

protected:
};

//! This template class is for property ids with a known data type
/*!
    When the property type has only a single value stored, this is the template to use,
    since this has many function to get/set that object without further effort.
    So for a a2dBoundingBox in a a2dBoudingBoxProperty it is best to use this:
    <code>
        typedef a2dPropertyIdTyped<a2dBoundingBox, a2dBoudingBoxProperty> a2dPropertyIdBoundingBox;
    </code>

        Assume that a2dCanvasObject has:

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

    This will give us two dynamic properties (see flag_multiple ) on the a_canvasobject.

    \ingroup general property
*/
template<class basetype, class proptype>
class a2dPropertyIdTyped : public a2dPropertyId
{

public:

    typedef basetype ( wxObject::*Get )() const;
    typedef const basetype& ( wxObject::*ConstGet )() const;
    typedef void ( wxObject::*Set )( basetype );
    typedef void ( wxObject::*ConstSet )( const basetype& value );
    typedef basetype wxObject::* Mptr;

    //! constructor for dynamic properties on objects
    /*!
        This constructor is for purely dynamic properties ( stored a a2dNamedProperty on a a2dObject ).

        \param name the name of the propertyId
        \param flags defines how a property needs to be set and retrieved.
        \param defaultvalue if property is not available, and still asked for, and allowed as dynamic property, this value is returned.
    */
    a2dPropertyIdTyped( const wxString& name, Flags flags, const basetype& defaultvalue );

    a2dPropertyIdTyped( const wxString& name, basetype defaultvalue, Flags flags = flag_listfirst );

    //! constructor for Get and Set functions to get the property
    /*!
        \param name the name of the propertyId
        \param flags defines how a property needs to be set and retrieved.
        \param defaultvalue if property is not available, and still asked for, and allowed as dynamic property, this value is returned.
        \param getFunc  function to get the property to a class member, if NULL property is retrieved in a different manner
        \param setFunc  function to set the property to a class member, if NULL property is set in a different manner
    */
    a2dPropertyIdTyped( const wxString& name, Flags flags, const basetype& defaultvalue,
                        Get getFunc, Set setFunc );

    //! constructor for Get and ConstSet functions to get the property
    /*!
        \param name the name of the propertyId
        \param flags defines how a property needs to be set and retrieved.
        \param defaultvalue if property is not available, and still asked for, and allowed as dynamic property, this value is returned.
        \param getFunc  function to get the property from a class member, if NULL property is retrieved in a different manner
        \param setFunc  function to set the property to a class member, if NULL property is set in a different manner
    */
    a2dPropertyIdTyped( const wxString& name, Flags flags, const basetype& defaultvalue,
                        Get getFunc, ConstSet setFunc );

    a2dPropertyIdTyped( const wxString& name, Flags flags, const basetype& defaultvalue,
                        ConstGet getFunc, ConstSet setFunc );

    //! constructor for a pointer to a member variable to get the property
    /*!
        \param name the name of the propertyId
        \param flags defines how a property needs to be set and retrieved.
        \param defaultvalue if property is not available, and still asked for, and allowed as dynamic property, this value is returned.
        \param ptm  pointer to a class field to get and set the property, if NULL property is retrieved in a different manner
    */
    a2dPropertyIdTyped( const wxString& name, Flags flags, const basetype& defaultvalue,
                        Mptr ptm );

    ~a2dPropertyIdTyped() {}

    //! Set the property in obj to value
    void SetPropertyToObject( a2dObject* obj, const basetype& value, SetFlags setflags = set_none ) const;

    //! Given the object to set the property on, and a a2dNamedProperty to
    /*!
        hold the property value and type, set it as a property to the object.
    */
    void SetPropertyToObject( a2dObject* obj, proptype* property, SetFlags setflags = set_none ) const;

    //! retrieve in form of a a2dNamedProperty the value of the property identified with this id
    /*!
        The property id is defining the way a a2dNamedProperty can be set or retrieved from the a2dObject.
        If the m_ptm is set that will be used, else m_get and m_set, and in the end via dynamic properties.

        \param obj the object on which the property needs to be retrieved
    */
    proptype* GetPropertyClone( const a2dObject* obj ) const;

    a2dNamedProperty* GetPropertyAsNamedProperty( const a2dObject* obj ) const;

    //! Set the property in obj to value (not cloning value)
    void SetNamedPropertyToObject( a2dObject* obj, a2dNamedProperty* nprop, SetFlags setflags = set_none ) const;

    //! Get the property from the list in obj ( no members, not cloned )
    proptype* GetPropertyListOnly( const a2dObject* obj ) const;

    //! Get the property value in obj
    basetype GetPropertyValue( const a2dObject* obj ) const;

    //! Get the property value in obj, if not found return default value given
    basetype GetPropertyValue( const a2dObject* obj, const basetype& defaultvalue ) const;

    //! Get a pointer to the property value in obj
    basetype* GetPropertyValuePtr( a2dObject* obj ) const;

    //! Get the default value for this property
    const basetype& GetDefaultValue() const { return m_defaultvalue; }

    //! Create a new property object from a value string
    a2dNamedProperty* CreatePropertyFromString( const wxString& value ) const;

    //! returns a dummy property id of this type, that can be used in non-id applications
    /*!
        Normally properties that are used internal in the library are declared by a a2dPropertyId
        as a static member of a class.
        Like this:
        <code>
        static const a2dPropertyIdString PROPID_Name;
        </code>
        And as this the property id is initialized, as to say what type of property it is and to which class it belongs.
        <code>
          const a2dPropertyIdString a2dObject::PROPID_Name( wxT("name"), a2dPropertyId::flag_none, wxT("") );
        </code>

        If non of this is required, you can have non id properties.
        As an example you can have user defined properties, which the program will not know about,
        and will not treat as setting something inside a class, or holds in a dynamic fashion some information,
        which the program does use.

        Like the user can add a dynamic property to a a2dObject, dynamic properties are stored in  a2dObject::m_propertylist.
        And using this function here, you can give such a property the dummy id which is always there as a static existing id.
    */
    static a2dPropertyIdTyped<basetype, proptype>* GetDummy();

protected:
    //! this constructor is only for property ids referencing other property ids
    a2dPropertyIdTyped( ) : a2dPropertyId( ) {};

    Get m_get;
    ConstGet m_constget;
    Set m_set;
    ConstSet m_constset;

    basetype a2dObject::* m_ptm;

    //! this is the value returned if e.g. a property is not found
    basetype m_defaultvalue;
};

class A2DGENERALDLLEXP a2dObject;

#if defined(A2DGENERALMAKINGDLL) || defined(WXDOCVIEW_USINGDLL)
template class A2DGENERALDLLEXP a2dSmrtPtr<a2dObject>;
template class A2DGENERALDLLEXP a2dAutoZeroPtr<a2dObject>;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxInt16,        a2dInt16Property>      ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxInt32,        a2dInt32Property>      ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxUint16,       a2dUint16Property>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxUint32,       a2dUint32Property>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<bool,           a2dBoolProperty>       ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<double,         a2dDoubleProperty>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<float,          a2dFloatProperty>      ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<void*,         a2dVoidPtrProperty>    ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxString,       a2dStringProperty>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxFileName,     a2dFileNameProperty>   ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxDateTime,     a2dDateTimeProperty>   ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxMenu*,        a2dMenuProperty>       ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxUri,          a2dUriProperty>       ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxWindow*,      a2dWindowProperty>     ;
//template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxTipWindow*,   a2dTipWindowProperty>  ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxColour,       a2dColourProperty>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxArrayString,  a2dArrayStringProperty>;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<a2dSmrtPtr<a2dObject>, a2dProperty>;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<a2dAutoZeroPtr<a2dObject>, a2dAutoZeroProperty>;
#endif

/*! \addtogroup property
*  @{
*/

//! property of this type
typedef a2dPropertyIdTyped<wxInt16,        a2dInt16Property>       a2dPropertyIdInt16;
//! property of this type
typedef a2dPropertyIdTyped<wxInt32,        a2dInt32Property>       a2dPropertyIdInt32;
//! property of this type
typedef a2dPropertyIdTyped<wxUint16,       a2dUint16Property>      a2dPropertyIdUint16;
//! property of this type
typedef a2dPropertyIdTyped<wxUint32,       a2dUint32Property>      a2dPropertyIdUint32;
//! property of this type
typedef a2dPropertyIdTyped<bool,           a2dBoolProperty>        a2dPropertyIdBool;
//! property of this type
typedef a2dPropertyIdTyped<double,         a2dDoubleProperty>      a2dPropertyIdDouble;
//! property of this type
typedef a2dPropertyIdTyped<float,          a2dFloatProperty>       a2dPropertyIdFloat;
//! property of this type
typedef a2dPropertyIdTyped<void*,         a2dVoidPtrProperty>     a2dPropertyIdVoidPtr;
//! property of this type
typedef a2dPropertyIdTyped<wxObject,       a2dObjectProperty>      a2dPropertyIdObject;
//! property of this type
typedef a2dPropertyIdTyped<wxString,       a2dStringProperty>      a2dPropertyIdString;
//! property of this type
typedef a2dPropertyIdTyped<wxFileName,     a2dFileNameProperty>    a2dPropertyIdFileName;
//! property of this type
typedef a2dPropertyIdTyped<wxDateTime,     a2dDateTimeProperty>    a2dPropertyIdDateTime;
//! property of this type
typedef a2dPropertyIdTyped<wxMenu*,        a2dMenuProperty>        a2dPropertyIdMenu;
//! property of this type
typedef a2dPropertyIdTyped<wxURI,          a2dUriProperty>         a2dPropertyIdUri;
//! property of this type
typedef a2dPropertyIdTyped<wxWindow*,      a2dWindowProperty>      a2dPropertyIdWindow;
//typedef a2dPropertyIdTyped<wxTipWindow*,   a2dWindowProperty>      a2dPropertyIdTipWindow;
typedef a2dPropertyIdTyped<wxColour,       a2dColourProperty>      a2dPropertyIdColour;
//! property of this type
typedef a2dPropertyIdTyped<wxArrayString,  a2dArrayStringProperty> a2dPropertyIdArrayString;
//! property of this type
typedef a2dPropertyIdTyped<a2dSmrtPtr<a2dObject>, a2dProperty>   a2dPropertyIdRefObject;
//! property of this type
typedef a2dPropertyIdTyped<a2dAutoZeroPtr<a2dObject>, a2dAutoZeroProperty>   a2dPropertyIdRefObjectAutoZero;

//!  @} property

/*============================*/

//! to define a get set property more easily
#define A2D_PROPID_GSI_F( type, classname , propname, defaultval, flags ) \
    PROPID_##propname = new type( wxT(#propname), \
                                  flags, defaultval, \
                                  static_cast < type::Get >( &classname::Get##propname), \
                                  static_cast < type::Set >( &classname::Set##propname) ); \
    AddPropertyId( PROPID_##propname );

//! to define a get set property more easily
#define A2D_PROPID_GSI( type, classname , propname, defaultval ) \
    A2D_PROPID_GSI_F( type, classname, propname, defaultval, a2dPropertyId::flag_none )

//! to define a get set property more easily
#define A2D_PROPID_GS_F( type, classname, propname, defaultval, fget, fset, flags ) \
    PROPID_##propname = new type( wxT(#propname), \
                                  flags, defaultval, \
                                  static_cast < type::Get >( &classname::fget), \
                                  static_cast < type::Set >( &classname::fset) ) ; \
    AddPropertyId( PROPID_##propname );

//! to define a get set property more easily
#define A2D_PROPID_GS( type, classname , propname, defaultval, fget, fset ) \
    A2D_PROPID_GS_F( type, classname, propname, defaultval, fget, fset, a2dPropertyId::flag_none )

//! to define a get set property more easily
#define A2D_PROPID_D_F( type, propname, defaultval, flags ) \
    PROPID_##propname = new type( wxT(#propname), \
                                  flags, defaultval ); \
    AddPropertyId( PROPID_##propname );

#define A2D_PROPID_D( type, propname, defaultval ) \
    A2D_PROPID_D_F( type, propname, defaultval, a2dPropertyId::flag_none )

//! to define a get set property more easily
#define A2D_PROPID_M_F( type, classname , propname, defaultval, mptr, flags ) \
    PROPID_##propname = new type( wxT(#propname), \
                                  flags, defaultval, \
                                  static_cast < type::Mptr >( &classname::mptr) ); \
    AddPropertyId( PROPID_##propname );

//! to define a get set property more easily
#define A2D_PROPID_M( type, classname, propname, defaultval, mptr ) \
    A2D_PROPID_M_F( type, classname, propname, defaultval, mptr, a2dPropertyId::flag_none )



//! Like a2dPropertyIdTyped, but always references a certain member Get and Set function in a certain class
//! \ingroup property
template<class basetype, class proptype >
class a2dPropertyIdGetSet : public a2dPropertyIdTyped< basetype, proptype >
{
public:

    //! get method
    typedef basetype ( a2dObject::*Get )() const;
    //! set method
    typedef void ( a2dObject::*Set )( basetype );
    //! constant get method
    typedef void ( a2dObject::*ConstSet )( const basetype& value );
    //! member pointer
    typedef basetype a2dObject::* Mptr;

    //! constructor
    a2dPropertyIdGetSet( const wxString& name, a2dPropertyId::Flags flags, const basetype& defaultvalue,
                         Get getFunc, Set setFunc
                       );

    //! constructor
    a2dPropertyIdGetSet( const wxString& name, a2dPropertyId::Flags flags, const basetype& defaultvalue,
                         Get getFunc, ConstSet setFunc
                       );

    //! Set the property value in obj
    bool SetPropertyToObject( a2dObject* obj, const basetype& value ) const;

    //! Get the property value in obj
    basetype GetPropertyValue( const a2dObject* obj ) const;

    //! Get the property value in obj, if not found return default value given
    basetype GetPropertyValue( const a2dObject* obj, const basetype& defaultvalue ) const;

    //! Get the property in obj (cloned)
    proptype* GetPropertyClone( const a2dObject* obj ) const;

    a2dNamedProperty* GetPropertyAsNamedProperty( const a2dObject* obj ) const;

};

//! get set property id for boolean property
//! \ingroup property
typedef a2dPropertyIdGetSet< bool, a2dBoolProperty > a2dPropertyIdBoolGetSet;

//! Like a2dPropertyIdTyped, but always references a certain member in a certain class
/*!
     \ingroup property
*/
template<class basetype, class proptype>
class a2dMemberDclrTyped : public a2dPropertyIdTyped<basetype, proptype>
{
public:

    a2dMemberDclrTyped ( basetype a2dObject::* ptm ) : a2dPropertyIdTyped<basetype, proptype>( ) { m_ptm = ptm; }
    bool SetPropertyToObject( a2dObject* obj, const basetype& value ) const;
    proptype* GetPropertyClone( const a2dObject* obj ) const;

    //! Get the property value in obj
    basetype GetPropertyValue( const a2dObject* obj ) const;

    //! Get the property value in obj, if not found return default value given
    basetype GetPropertyValue( const a2dObject* obj, const basetype& defaultvalue ) const;

protected:

    //! pointer to member
    basetype a2dObject::* m_ptm;
};

#if defined(WXDOCVIEW_USINGDLL)
template class A2DGENERALDLLEXP a2dMemberDclrTyped<wxInt16,       a2dInt16Property>       ;
template class A2DGENERALDLLEXP a2dMemberDclrTyped<wxInt32,       a2dInt32Property>       ;
template class A2DGENERALDLLEXP a2dMemberDclrTyped<wxUint16,      a2dUint16Property>      ;
template class A2DGENERALDLLEXP a2dMemberDclrTyped<wxUint32,      a2dUint32Property>      ;
template class A2DGENERALDLLEXP a2dMemberDclrTyped<bool,          a2dBoolProperty>        ;
template class A2DGENERALDLLEXP a2dMemberDclrTyped<double,        a2dDoubleProperty>      ;
template class A2DGENERALDLLEXP a2dMemberDclrTyped<void*,        a2dVoidPtrProperty>     ;
template class A2DGENERALDLLEXP a2dMemberDclrTyped<wxString,      a2dStringProperty>      ;
template class A2DGENERALDLLEXP a2dMemberDclrTyped<wxFileName,    a2dFileNameProperty>    ;
template class A2DGENERALDLLEXP a2dMemberDclrTyped<wxColour,      a2dColourProperty>      ;
template class A2DGENERALDLLEXP a2dMemberDclrTyped<wxArrayString, a2dArrayStringProperty> ;
#endif

/*! \addtogroup property
*  @{
*/

//! for member associated properties of this type
typedef a2dMemberDclrTyped<wxInt16,       a2dInt16Property>       wxMemberDclrInt16;
//! for member associated properties of this type
typedef a2dMemberDclrTyped<wxInt32,       a2dInt32Property>       wxMemberDclrInt32;
//! for member associated properties of this type
typedef a2dMemberDclrTyped<wxUint16,      a2dUint16Property>      wxMemberDclrUint16;
//! for member associated properties of this type
typedef a2dMemberDclrTyped<wxUint32,      a2dUint32Property>      wxMemberDclrUint32;
//! for member associated properties of this type
typedef a2dMemberDclrTyped<bool,          a2dBoolProperty>        wxMemberDclrBool;
//! for member associated properties of this type
typedef a2dMemberDclrTyped<double,        a2dDoubleProperty>      wxMemberDclrDouble;
//! for member associated properties of this type
typedef a2dMemberDclrTyped<void*,        a2dVoidPtrProperty>     wxMemberDclrVoidPtr;
//! for member associated properties of this type
typedef a2dMemberDclrTyped<wxString,      a2dStringProperty>      wxMemberDclrString;
//! for member associated properties of this type
typedef a2dMemberDclrTyped<wxFileName,    a2dFileNameProperty>    wxMemberDclrFileName;
//! for member associated properties of this type
typedef a2dMemberDclrTyped<wxColour,      a2dColourProperty>      wxMemberDclrColour;
//! for member associated properties of this type
typedef a2dMemberDclrTyped<wxArrayString, a2dArrayStringProperty> wxMemberDclrArrayString;

//!  @} property

#endif

