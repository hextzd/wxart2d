/*! \file apps/mars/object.cpp
    \author Erik van der Wal

    Copyright: 2004 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: object.cpp,v 1.5 2006/12/13 21:43:21 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "object.h"

static a2dPropertyIdString* CreateStringPropertyId( a2dObject* parent, const wxString& name )
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
// msProperty
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( msProperty, a2dStringProperty )

msProperty::msProperty()
    : a2dStringProperty()
{
}

msProperty::msProperty( const a2dPropertyIdString* id, const wxString& value )
    : a2dStringProperty( id, value )
{
}

msProperty::msProperty( const msProperty& other, a2dObject::CloneOptions options )
    : a2dStringProperty( other, options )
{
}

a2dNamedProperty* msProperty::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new msProperty( *this, options );
};

#if wxART2D_USE_CVGIO
void msProperty::Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser )
{
    parser.Require( START_TAG, wxT( "o" ) );
    wxString name;
    name = parser.RequireAttributeValue( wxT( "name" ) );
    CreateStringPropertyId( ( a2dObject* ) parent, name );
    a2dStringProperty::Load( parent, parser );
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// msObject
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( msObject, a2dCanvasObject )

msObject::msObject()
    : a2dCanvasObject()
{
}

msObject::msObject( const msObject& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
}

a2dObject* msObject::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new msObject( *this, options, refs );
};

#if wxART2D_USE_CVGIO
void msObject::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void msObject::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

msProperty* msObject::SetStringProperty( const wxString& name, const wxString& value )
{
    a2dPropertyIdString* propid = CreateStringPropertyId( this, name );
    propid->SetPropertyToObject( this, value );

    msProperty* prop = wxDynamicCast( GetProperty( propid ), msProperty );
    /*
        if ( !prop )
        {
            prop = new msProperty( id, value );
            prop->SetToObject( this );
        }
        else
            prop->SetValue( value );
    */
    return prop;
}

msProperty* msObject::GetStringProperty( const wxString& name )
{
    const a2dNamedProperty* prop = FindProperty( name );
    msProperty* sprop = wxDynamicCast( prop, msProperty );
    if ( sprop )
        return sprop;
    return NULL;
}

wxString msObject::GetStringPropertyValue( const wxString& name )
{
    a2dPropertyIdString* propid = CreateStringPropertyId( this, name );
    msProperty* prop = wxDynamicCast( GetProperty( propid ), msProperty );
    if ( prop )
        return prop->GetValue();
    else
        return wxEmptyString;
}

void msObject::MergeProperties( msObject* from )
{
    a2dNamedPropertyList::iterator iter;
    for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
    {
        a2dNamedProperty* prop = *iter;
        wxString name = prop->GetName();
        a2dPropertyIdString* propid = CreateStringPropertyId( this, name );
        if ( !GetProperty( propid ) )
            SetStringProperty( name, from->GetStringPropertyValue( name ) );
    }
}
