/*! \file general/src/id.cpp
    \author Michael Sögtrop
    \date Created 12/12/2003

    Copyright: 2003-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: id.cpp,v 1.47 2008/09/05 19:01:13 titato Exp $
*/

#include "wxartbaseprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <wx/filename.h>
#include "wx/general/smrtptr.h"

#include "wx/general/id.h"

#include "wx/general/gen.h"
#include "wx/general/smrtptr.inl"
//#include "wx/general/id.inl"

// explicit tamplate instantiations

#if (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))

// MSVC warning 4660 is quite stupid. It says that the template is already instantiated
// by using it, but it is not fully instantiated as required for a library
#pragma warning(disable: 4660)

#if defined(WXDOCVIEW_USINGDLL)
template class A2DGENERALDLLEXP a2dSmrtPtr<a2dObject>;
template class A2DGENERALDLLEXP a2dAutoZeroPtr<a2dObject>;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxInt16,        a2dInt16Property>      ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxInt32,        a2dInt32Property>      ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxUint16,       a2dUint16Property>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxUint32,       a2dUint32Property>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<bool,           a2dBoolProperty>       ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<double,         a2dDoubleProperty>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<void*,         a2dVoidPtrProperty>    ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxString,       a2dStringProperty>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxFileName,     a2dFileNameProperty>   ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxColour,       a2dColourProperty>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxArrayString,  a2dArrayStringProperty>;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<a2dSmrtPtr<a2dObject>, a2dProperty>;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<a2dAutoZeroPtr<a2dObject>, a2dAutoZeroProperty>;

template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxInt16,        a2dInt16Property>      ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxInt32,        a2dInt32Property>      ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxUint16,       a2dUint16Property>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxUint32,       a2dUint32Property>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<bool,           a2dBoolProperty>       ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<double,         a2dDoubleProperty>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<void*,         a2dVoidPtrProperty>    ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxString,       a2dStringProperty>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxFileName,     a2dFileNameProperty>   ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxColour,       a2dColourProperty>     ;
template class A2DGENERALDLLEXP a2dPropertyIdTyped<wxArrayString,  a2dArrayStringProperty>;
#endif

#pragma warning(default: 4660)

#endif // (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))

#include "wx/general/gen.h"


a2dPropertyId::a2dPropertyId( const wxString& name, Flags flag )
{
    // here a memory leak detection on static property id's is a good place
    //_CrtSetBreakAlloc(348057);

    m_refcount = 0;
    m_name = name;
    m_flags = flag;
}

a2dPropertyId::a2dPropertyId( )
{
    m_refcount = 0;
}

void a2dPropertyId::RemovePropertyFromObject( a2dObject* obj ) const
{
    obj->RemoveProperty( this, true );
}

void a2dPropertyId::SetNamedPropertyToObject( a2dObject* obj, a2dNamedProperty* value, SetFlags setflags ) const
{
}




