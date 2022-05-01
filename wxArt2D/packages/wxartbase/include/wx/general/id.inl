/*! \file wx/general/id.inl
    \author Michael S�trop
    \date Created 12/12/2003

    Copyright: 2003-2004 (c) Michael S�trop

    Licence: wxWidgets Licence

    RCS-ID: $Id: id.inl,v 1.14 2008/10/30 23:18:09 titato Exp $
*/

#ifndef __ID_INL__
#define __ID_INL__

#include "wx/general/gen.h"

//--------------------------------------------------------------------
// a2dPropertyId
//--------------------------------------------------------------------

bool a2dPropertyId::DoIgnoreIfNotMember( a2dObject *obj ) const
{
    return obj->DoIgnoreIfNotMember( this );
}

void a2dPropertyId::DoSetPropertyId( a2dNamedProperty *prop, a2dPropertyId *id )
{
    prop->m_id = id;
}

//--------------------------------------------------------------------
// a2dPropertyIdProp
//--------------------------------------------------------------------

template<class proptype>
void a2dPropertyIdProp<proptype>::SetPropertyToObject( a2dObject *obj, const proptype &value ) const
{
    obj->SetProperty( value.Clone( a2dObject::clone_flat ) );
}

template<class proptype>
void a2dPropertyIdProp<proptype>::SetPropertyToObject( a2dObject *obj, proptype* property ) const
{
    // and assign it
    if( property->GetId() == this )
    {
        if ( obj->HasPropertyId( this ) )
        {    
            //dynamic property  id->IsOnlyMember();
            if( !IsOnlyMember() )
            {
                // check if the property already exists
                a2dNamedProperty* existprop = 0;

                // If the property can be set more than once, ignore existing one

                if( ! CheckFlags( a2dPropertyId::flag_multiple ) )
                {
                    existprop = obj->FindProperty( this );
                }

                if ( existprop )
                {
                    // does exist, so assign the new value
                    existprop->Assign( *property );
                    obj->OnPropertyChanged( this );
                }
                else
                {
                    // does not exist, so add it
                    // we know that it doesn't exist, so we do not need to remove it
                    // or we want to add it more than once
                    obj->AddProperty( property->Clone( a2dObject::clone_flat ) );
                    obj->OnPropertyChanged( this );
                }
            }
        }
        else
        {
            wxString warn;
            warn.Printf( wxT("object with name %s does not have propertyId with name: %s"), obj->GetName().c_str(), GetName().c_str() );
            wxLogWarning( warn ); 
        }
    }
    else
    {
        wxString warn;
        warn.Printf( wxT("a2dNamedProperty Id with name %s does not fit propertyId to set with name: %s"), property->GetName().c_str(), GetName().c_str() );
        wxLogWarning( warn ); 
    }

}

template<class proptype>
void a2dPropertyIdProp<proptype>::SetNamedPropertyToObject( a2dObject* obj, a2dNamedProperty* nprop, SetFlags setflags ) const
{
    proptype *castprop = wxStaticCastTmpl( nprop, proptype );
    if ( castprop )
        SetPropertyToObject( obj, castprop );
}

template<class proptype>
a2dNamedProperty* a2dPropertyIdProp<proptype>::GetPropertyAsNamedProperty( const a2dObject *obj ) const
{
    return GetPropertyClone( obj );
}

template<class proptype>
proptype *a2dPropertyIdProp<proptype>::GetPropertyClone( const a2dObject *obj ) const
{
    const a2dNamedProperty* prop = obj->FindProperty( this );
    if ( prop )
    {
        proptype *prop2 = wxStaticCastTmpl( prop->Clone( a2dObject::clone_deep ), proptype );
        return prop2;
    }
    return NULL;
}

template<class proptype>
proptype *a2dPropertyIdProp<proptype>::GetProperty( a2dObject *obj ) const
{
    a2dNamedProperty* prop = obj->FindProperty( this );
    if ( prop )
    {
        proptype *prop2 = wxStaticCastTmpl( prop, proptype );
        return prop2;
    }

    return 0;
}

template<class proptype>
const proptype *a2dPropertyIdProp<proptype>::GetProperty( const a2dObject *obj ) const
{
    const a2dNamedProperty* prop = obj->FindProperty( this );
    if ( prop )
    {
        const proptype *prop2 = wxStaticCastTmpl( prop, proptype );
        return prop2;
    }

    return 0;
}


//--------------------------------------------------------------------
// a2dPropertyIdTyped
//--------------------------------------------------------------------

template<class basetype, class proptype>
a2dPropertyIdTyped<basetype,proptype>::a2dPropertyIdTyped( const wxString& name, Flags flags, const basetype &defaultvalue )
:    a2dPropertyId( name, flags )
{
    m_defaultvalue = defaultvalue;
    m_get = NULL;
	m_set = NULL;
	m_constset = NULL;
    m_constget = NULL;
	m_ptm = NULL;
}

template<class basetype, class proptype>
a2dPropertyIdTyped<basetype,proptype>::a2dPropertyIdTyped( const wxString& name, basetype defaultvalue, Flags flags )
:    a2dPropertyId( name, flags )
{
    m_defaultvalue = defaultvalue;
    m_get = NULL;
	m_set = NULL;
	m_constset = NULL;
    m_constget = NULL;
	m_ptm = NULL;
}

template<class basetype, class proptype >
a2dPropertyIdTyped<basetype, proptype >::a2dPropertyIdTyped( const wxString& name, Flags flags, const basetype &defaultvalue, Get getFunc, Set setFunc )
:    a2dPropertyId( name, flags )
{
    //m_defaultvalue = defaultvalue;
    m_get = getFunc;
    m_set = setFunc;
    m_constget = NULL;
	m_constset = NULL;
    m_ptm = NULL;
}

template<class basetype, class proptype >
a2dPropertyIdTyped<basetype, proptype >::a2dPropertyIdTyped( const wxString& name, Flags flags, const basetype &defaultvalue, Get getFunc, ConstSet setFunc )
:    a2dPropertyId( name, flags )
{
    //m_defaultvalue = defaultvalue;
    m_get = getFunc;
    m_set = NULL;
	m_constset = setFunc;
    m_constget = NULL;
    m_ptm = NULL;
}

template<class basetype, class proptype >
a2dPropertyIdTyped<basetype, proptype >::a2dPropertyIdTyped( const wxString& name, Flags flags, const basetype &defaultvalue, ConstGet getFunc, ConstSet setFunc )
:    a2dPropertyId( name, flags )
{
    //m_defaultvalue = defaultvalue;
    m_get = NULL;
    m_set = NULL;
    m_constget = getFunc;
	m_constset = setFunc;
    m_ptm = NULL;
}

template<class basetype, class proptype >
a2dPropertyIdTyped<basetype, proptype >::a2dPropertyIdTyped( const wxString& name, Flags flags, const basetype &defaultvalue, basetype wxObject::* ptm )
:    a2dPropertyId( name, flags )
{
    //m_defaultvalue = defaultvalue;
    m_get = NULL;
    m_set = NULL;
    m_constget = NULL;
	m_constset = NULL;
    m_ptm = ptm;
}

template<class basetype, class proptype>
void a2dPropertyIdTyped<basetype,proptype>::SetNamedPropertyToObject( a2dObject* obj, a2dNamedProperty* nprop, SetFlags setflags ) const
{
    proptype *castprop = wxStaticCastTmpl( nprop, proptype );
    if ( castprop )
        SetPropertyToObject( obj, castprop,  a2dPropertyId::set_none );
}

template<class basetype, class proptype>
a2dNamedProperty* a2dPropertyIdTyped<basetype,proptype>::GetPropertyAsNamedProperty( const a2dObject *obj ) const
{
    return GetPropertyClone( obj );
}

template<class basetype, class proptype>
void a2dPropertyIdTyped<basetype,proptype>::SetPropertyToObject( a2dObject *obj, const basetype &value, SetFlags setflags ) const
{
    // create a property object
    proptype prop( this, value );
    SetPropertyToObject( obj, &prop, setflags );
}

template<class basetype, class proptype>
void a2dPropertyIdTyped<basetype,proptype>::SetPropertyToObject( a2dObject *obj, proptype* property, SetFlags setflags ) const
{
    if( property->GetId() == this )
    {
        if ( obj->HasPropertyId( this ) )
        {    
            if( setflags & a2dPropertyId::set_remove_property )
            {
                obj->RemoveProperty( this, true );
            }

            proptype *castprop = wxStaticCastTmpl( property, proptype );
            if ( m_ptm )
            {
                obj->*m_ptm = castprop->GetValue();
                obj->OnPropertyChanged( castprop->GetId() );
            }
            else if (m_set)
            {
                (obj->*m_set)( castprop->GetValue() );
                obj->OnPropertyChanged( castprop->GetId() );
            }
            else if (m_constset)
            {
                (obj->*m_constset)( castprop->GetValue() );
                obj->OnPropertyChanged( castprop->GetId() );
            }
            //dynamic property  id->IsOnlyMember();
            else if( !IsOnlyMember() && !(setflags & a2dPropertyId::set_onlymember)  )
            {
                // check if the property already exists
                a2dNamedProperty* existprop = 0;

                // If the property can be set more than once, ignore existing one

                if( ! CheckFlags( a2dPropertyId::flag_multiple ) )
                {
                    existprop = obj->FindProperty( this );
                }

                if ( existprop )
                {
                    // does exist, so assign the new value
                    existprop->Assign( *property );
                    obj->OnPropertyChanged( this );
                }
                else
                {
                    // does not exist, so add it
                    // we know that it doesn't exist, so we do not need to remove it
                    // or we want to add it more than once
                    obj->AddProperty( property->Clone( a2dObject::clone_flat ) );
                    obj->OnPropertyChanged( this );
                }
            }
        }
        else
        {
            wxString warn;
            warn.Printf( wxT("object with name %s does not have propertyId with name: %s"), obj->GetName().c_str(), GetName().c_str() );
            wxLogWarning( warn ); 
        }
    }
    else
    {
        wxString warn;
        warn.Printf( wxT("a2dNamedProperty Id with name %s does not fit propertyId to set with name: %s"), property->GetName().c_str(), GetName().c_str() );
        wxLogWarning( warn ); 
    }
}

template<class basetype, class proptype>
proptype *a2dPropertyIdTyped<basetype,proptype>::GetPropertyListOnly( const a2dObject *obj ) const
{
    const a2dNamedProperty *prop = obj->FindProperty( this );
    return wxStaticCastNullTmpl( prop, proptype );
}

template<class basetype, class proptype>
basetype a2dPropertyIdTyped<basetype,proptype>::GetPropertyValue( const a2dObject *obj ) const
{
	//first try the pointer, since they have priority
    if ( m_ptm )
        return obj->*m_ptm;
    if (m_get)
        return (obj->*m_get)();
    if (m_constget)
        return (obj->*m_constget)();

    const a2dNamedProperty* prop = obj->FindProperty( this );
    if ( prop )
    {
        proptype *prop2 = wxStaticCastTmpl( prop, proptype );
        return prop2->GetValue();
    }
    return GetDefaultValue();
}

template<class basetype, class proptype>
basetype a2dPropertyIdTyped<basetype,proptype>::GetPropertyValue( const a2dObject *obj, const basetype &defaultvalue  ) const
{
	//first try the pointer, since they have priority
    if ( m_ptm )
        return obj->*m_ptm;
    if (m_get)
        return (obj->*m_get)();
    if (m_constget)
        return (obj->*m_constget)();

	// now assemble properties with this id, which is normally just one.
    const a2dNamedProperty* prop = obj->FindProperty( this );
    if ( prop )
    {
        proptype *prop2 = wxStaticCastTmpl( prop, proptype );
        return prop2->GetValue();
    }
    return defaultvalue;
}

template<class basetype, class proptype>
basetype *a2dPropertyIdTyped<basetype,proptype>::GetPropertyValuePtr( a2dObject *obj ) const
{
    // Note: this is not yet supported for properties implemented as members
    a2dNamedProperty* prop = obj->FindProperty( this );
    if ( prop )
    {
        proptype *prop2 = wxStaticCastTmpl( prop, proptype );
        return prop2->GetValuePtr();
    }
    else
    {
        return 0;
    }
}

template<class basetype, class proptype>
inline a2dNamedProperty *a2dPropertyIdTyped<basetype,proptype>::CreatePropertyFromString( const wxString &value ) const
{
    return proptype::CreatePropertyFromString( this, value );
}


template<class basetype, class proptype>
a2dPropertyIdTyped<basetype,proptype>* a2dPropertyIdTyped<basetype, proptype>::GetDummy()
{
    static a2dPropertyIdTyped<basetype,proptype> dummy( wxT("__dummy__"), a2dPropertyId::flag_none, basetype() );
    return &dummy;
}

template<class basetype, class proptype>
proptype *a2dPropertyIdTyped<basetype,proptype>::GetPropertyClone( const a2dObject *obj ) const
{
    if ( m_ptm )
	{
	    return new proptype( this, obj->*m_ptm );
	}
    if (m_get)
	{
		basetype val = (obj->*m_get)();
		return new proptype( this, val );
	}
    if (m_get)
	{
		basetype val = (obj->*m_constget)();
		return new proptype( this, val );
	}

    const a2dNamedProperty* prop = obj->FindProperty( this );
    if ( prop )
    {
        proptype *prop2 = wxStaticCastTmpl( prop->Clone( a2dObject::clone_deep ), proptype );
        return prop2;
    }
    return NULL;
}

//--------------------------------------------------------------------
// a2dPropertyIdGetSet
//--------------------------------------------------------------------

template<class basetype, class proptype >
a2dPropertyIdGetSet<basetype, proptype >::a2dPropertyIdGetSet( const wxString& name, a2dPropertyId::Flags flags, const basetype &defaultvalue, Get getFunc, Set setFunc )
    :  a2dPropertyIdTyped<basetype, proptype>( name, flags, defaultvalue, getFunc, setFunc )
{
}

template<class basetype, class proptype >
a2dPropertyIdGetSet<basetype, proptype >::a2dPropertyIdGetSet( const wxString& name, a2dPropertyId::Flags flags, const basetype &defaultvalue, Get getFunc, ConstSet setFunc )
    :  a2dPropertyIdTyped<basetype, proptype>( name, flags, defaultvalue, getFunc, setFunc )
{
}

template<class basetype, class proptype >
bool a2dPropertyIdGetSet<basetype, proptype >::SetPropertyToObject( a2dObject *obj, const basetype &value ) const
{
    if (a2dPropertyIdTyped<basetype, proptype>::m_set)
        (obj->*a2dPropertyIdTyped<basetype, proptype>::m_set)( value );
    if (a2dPropertyIdTyped<basetype, proptype>::m_constset)
        (obj->*a2dPropertyIdTyped<basetype, proptype>::m_constset)( value );
    return true;
}

template<class basetype, class proptype >
basetype a2dPropertyIdGetSet<basetype, proptype >::GetPropertyValue( const a2dObject *obj ) const
{
    if (a2dPropertyIdTyped<basetype, proptype>::m_get)
        return (obj->*a2dPropertyIdTyped<basetype, proptype>::m_get)();
    if (a2dPropertyIdTyped<basetype, proptype>::m_constget)
        return (obj->*a2dPropertyIdTyped<basetype, proptype>::m_constget)();
    return a2dPropertyIdTyped<basetype, proptype>::GetDefaultValue();
}

template<class basetype, class proptype >
basetype a2dPropertyIdGetSet<basetype, proptype >::GetPropertyValue( const a2dObject *obj, const basetype &defaultvalue  ) const
{
    if (a2dPropertyIdTyped<basetype, proptype>::m_get)
        return (obj->*a2dPropertyIdTyped<basetype, proptype>::m_get)();
    if (a2dPropertyIdTyped<basetype, proptype>::m_constget)
        return (obj->*a2dPropertyIdTyped<basetype, proptype>::m_constget)();
    return defaultvalue;
}

template<class basetype, class proptype >
proptype* a2dPropertyIdGetSet<basetype, proptype >::GetPropertyClone( const a2dObject *obj ) const
{
    basetype val;
    if (a2dPropertyIdTyped<basetype, proptype>::m_get)
        val = (obj->*a2dPropertyIdTyped<basetype, proptype>::m_get)();
    if (a2dPropertyIdTyped<basetype, proptype>::m_constget)
        val = (obj->*a2dPropertyIdTyped<basetype, proptype>::m_constget)();
    return new proptype( *this, val );
}

template<class basetype, class proptype >
a2dNamedProperty* a2dPropertyIdGetSet<basetype, proptype >::GetPropertyAsNamedProperty( const a2dObject *obj ) const
{
    return GetPropertyClone( obj );
}


//--------------------------------------------------------------------
// a2dMemberDclrTyped
//--------------------------------------------------------------------

template<class basetype, class proptype>
bool a2dMemberDclrTyped<basetype,proptype>::SetPropertyToObject( a2dObject *obj, const basetype &value ) const
{
    // dynamically check obj type
    obj->*m_ptm = value;
    return true;
}

template<class basetype, class proptype >
basetype a2dMemberDclrTyped<basetype, proptype>::GetPropertyValue( const a2dObject *obj ) const
{
    if ( m_ptm )
        return obj->*m_ptm;
    return a2dPropertyIdTyped<basetype, proptype>::GetDefaultValue();
}

template<class basetype, class proptype >
basetype a2dMemberDclrTyped<basetype, proptype>::GetPropertyValue( const a2dObject *obj, const basetype &defaultvalue  ) const
{
    if ( m_ptm )
        return obj->*m_ptm;
    return defaultvalue;
}

template<class basetype, class proptype>
proptype *a2dMemberDclrTyped<basetype,proptype>::GetPropertyClone( const a2dObject *obj ) const
{
    return new proptype( *this, obj->*m_ptm );
}


#endif

