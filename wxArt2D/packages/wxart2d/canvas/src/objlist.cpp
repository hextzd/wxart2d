/*! \file canvas/src/objlist.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: objlist.cpp,v 1.88 2009/05/20 18:42:10 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/objlist.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"
#include "wx/general/smrtptr.inl"

#include "wx/general/a2dlist.h"
#include "wx/general/a2dlist.inl"

#include <math.h>
#include <algorithm>

//----------------------------------------------------------------------------
// template instanitiations
//----------------------------------------------------------------------------

// MSVC warning 4660 is quite stupid. It says that the template is already instantiated
// by using it, but it is not fully instantiated as required for a library
#ifdef _MSC_VER
#pragma warning(disable: 4660)
#endif

template class a2dSmrtPtrList<a2dCanvasObject>;

#ifdef _MSC_VER
#pragma warning(default: 4660)
#endif

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

a2dCanvasObjectList* wxNullCanvasObjectList;

bool XYSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    a2dCanvasObject* firstc = wxStaticCast( x.Get(), a2dCanvasObject );
    a2dCanvasObject* secondc = wxStaticCast( y.Get(), a2dCanvasObject );

    if ( firstc->GetPosX() < secondc->GetPosX() )
        return true;
    if ( firstc->GetPosX() == secondc->GetPosX() )
        if ( firstc->GetPosY() < secondc->GetPosY() )
            return true;
    return false;
}

bool XRevYSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    a2dCanvasObject* firstc = wxStaticCast( x.Get(), a2dCanvasObject );
    a2dCanvasObject* secondc = wxStaticCast( y.Get(), a2dCanvasObject );

    if ( firstc->GetPosX() < secondc->GetPosX() )
        return true;
    if ( firstc->GetPosX() == secondc->GetPosX() )
        if ( firstc->GetPosY() > secondc->GetPosY() )
            return true;
    return false;
}

bool YXSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    a2dCanvasObject* firstc = wxStaticCast( x.Get(), a2dCanvasObject );
    a2dCanvasObject* secondc = wxStaticCast( y.Get(), a2dCanvasObject );

    if ( firstc->GetPosY() < secondc->GetPosY() )
        return true;
    if ( firstc->GetPosY() == secondc->GetPosY() )
        if ( firstc->GetPosX() < secondc->GetPosX() )
            return true;
    return false;
}

bool YRevXSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    a2dCanvasObject* firstc = wxStaticCast( x.Get(), a2dCanvasObject );
    a2dCanvasObject* secondc = wxStaticCast( y.Get(), a2dCanvasObject );

    if ( firstc->GetPosY() < secondc->GetPosY() )
        return true;
    if ( firstc->GetPosY() == secondc->GetPosY() )
        if ( firstc->GetPosX() > secondc->GetPosX() )
            return true;
    return false;
}

//----------------------------------------------------------------------------
// a2dCanvasObjectList
//----------------------------------------------------------------------------

a2dCanvasObjectList::a2dCanvasObjectList()
{
}

a2dCanvasObjectList::~a2dCanvasObjectList()
{
}

void a2dCanvasObjectList::SortXY()
{
    s_a2dCanvasObjectSorter = &XYSorter;
    sort();
}

void a2dCanvasObjectList::SortXRevY()
{
    s_a2dCanvasObjectSorter = &XRevYSorter;
    sort();
}

void a2dCanvasObjectList::SortYX()
{
    s_a2dCanvasObjectSorter = &YXSorter;
    sort();
}

void a2dCanvasObjectList::SortYRevX()
{
    s_a2dCanvasObjectSorter = &YRevXSorter;
    sort();
}

void a2dCanvasObjectList::MakeUnique()
{
    a2dCanvasObjectList::iterator iter = begin();
    while( iter != end() )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        if ( obj && obj->GetOwnedBy() > 2 )
        {
            *iter = obj->TClone( a2dObject::clone_deep );
        }
        iter++;
    }
}

a2dCanvasObjectList& a2dCanvasObjectList::operator=( const a2dCanvasObjectList& other )
{
    wxASSERT( this != wxNullCanvasObjectList );

    if ( &other == wxNullCanvasObjectList )
        return *this;

    clear();
    for( a2dCanvasObjectList::const_iterator iter = other.begin(); iter != other.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        push_back( obj );
    }
    return *this;
}

a2dCanvasObjectList* a2dCanvasObjectList::Clone( a2dObject::CloneOptions options ) const
{
    if ( this == wxNullCanvasObjectList )
        return wxNullCanvasObjectList;

    a2dCanvasObjectList* a = new a2dCanvasObjectList();

    for( a2dCanvasObjectList::const_iterator iter = begin(); iter != end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj && options & a2dObject::clone_childs )
        {
            a2dCanvasObject* objnew = obj->TClone( options );
            a->push_back( objnew );
        }
        else
            a->push_back( obj );
    }

    return a;
}

a2dCanvasObjectList* a2dCanvasObjectList::Clone( a2dCanvasObjectFlagsMask mask, a2dObject::CloneOptions options, a2dlist< long >* objectsIndex, const a2dBoundingBox& bbox ) const
{
    if ( this == wxNullCanvasObjectList )
        return wxNullCanvasObjectList;

    a2dCanvasObjectList* a = new a2dCanvasObjectList();
    a2dAffineMatrix tworld;

    int index = 0;
    for( a2dCanvasObjectList::const_iterator iter = begin(); iter != end(); ++iter )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        if ( !obj->GetRelease() && obj->CheckMask( mask ) )
        {
            bool pass = true;
            if ( bbox.GetValid() )
            {
                a2dBoundingBox tmp;
                tmp.Expand( obj->GetMappedBbox( tworld ) );
                if ( bbox.Intersect( tmp ) != _IN )
                    pass = false;
            }
            if ( pass )
            {
                if ( options & a2dObject::clone_deep )
                {
                    a2dCanvasObject* objnew = obj->TClone( options );
                    a->push_back( objnew );
                }
                else
                    a->push_back( obj );
                if ( objectsIndex )
                    objectsIndex->push_back( index );
            }

        }
        if ( !obj->GetRelease() )
            index++;
    }

    return a;
}

a2dCanvasObjectList* a2dCanvasObjectList::CloneChecked( a2dObject::CloneOptions options, a2dlist< long >* objectsIndex ) const
{
    if ( this == wxNullCanvasObjectList )
        return wxNullCanvasObjectList;

    a2dCanvasObjectList* a = new a2dCanvasObjectList();

    int index = 0;
    for( a2dCanvasObjectList::const_iterator iter = begin(); iter != end(); ++iter )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        if ( !obj->GetRelease() && obj->GetCheck() )
        {
            if ( options & a2dObject::clone_deep )
            {
                a2dCanvasObject* objnew = obj->TClone( options );
                a->push_back( objnew );
            }
            else
                a->push_back( obj );
            if ( objectsIndex )
                objectsIndex->push_back( index );
        }
        if ( !obj->GetRelease() )
            index++;
    }

    return a;
}

bool a2dCanvasObjectList::ChangeLayer( wxUint16 layer, a2dCanvasObjectFlagsMask mask )
{
    /*
        a2dCanvasObjectIter myiter = Begin();
        while( myiter != End() )
        {
            a2dCanvasObjectList::value_type obj = *myiter;
            myiter++;
        }
    */
    bool did = false;
    for( a2dCanvasObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj && obj->CheckMask( mask ) )
        {
            did = true;
            obj->SetLayer( layer );
        }
    }

    return did;
}

a2dBoundingBox a2dCanvasObjectList::GetBBox( a2dCanvasObjectFlagsMask mask )
{
    a2dBoundingBox bbox;

    forEachIn( a2dCanvasObjectList, this )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj->GetRelease() || !obj->CheckMask( mask ) )
            continue;
		bbox.Expand( obj->GetBbox() );
	}
    return bbox;
}

int a2dCanvasObjectList::Release( a2dCanvasObjectFlagsMask mask, const wxString& classname, const a2dPropertyId* id, const wxString& name, bool now )
{
    int did = 0;
    a2dCanvasObjectList::iterator iter = begin();
    while( iter != end() )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        if ( obj &&
                ( classname.IsEmpty() || obj->GetClassInfo()->GetClassName() == classname ) &&
                ( !id || obj->HasProperty( id ) ) &&
                ( name.IsEmpty() || obj->GetName() == name ) &&
                ( obj->CheckMask( mask ) )
           )
        {
            did++;
            if ( now )
            {
                iter = erase( iter );
            }
            else
            {
                obj->SetRelease( true );
                obj->SetPending( true );
                iter++;
            }
        }
        else
            iter++;
    }

    return did;
}

int a2dCanvasObjectList::Release( a2dCanvasObject* object, bool backwards, bool all, bool now, const a2dPropertyId* id )
{
    int did = 0;
    if ( backwards )
    {
        a2dCanvasObjectList::reverse_iterator iter = rbegin();
        while( iter != rend() )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && object == obj && ( !id || obj->HasProperty( id ) ) )
            {
                if ( now )
                {
                    iterator it( iter.base() ) ;
                    -- it ;
                    erase( it ) ;
                    iter++;
                    //erase((++iter).base());
                }
                else
                {
                    obj->SetRelease( true );
                    obj->SetPending( true );
                    iter++;
                }
                did++;

                if ( !all )
                    break;
            }
            else
                iter++;
        }
    }
    else
    {
        a2dCanvasObjectList::iterator iter = begin();
        while( iter != end() )
        {
            a2dCanvasObject* obj = *iter;

            if ( obj && object == obj && ( !id || obj->HasProperty( id ) ) )
            {
                if ( now )
                {
                    iter = erase( iter );
                }
                else
                {
                    obj->SetRelease( true );
                    obj->SetPending( true );
                    iter++;
                }
                did++;

                if ( !all )
                    break;

            }
            else
                iter++;
        }
    }

    return did;
}

int a2dCanvasObjectList::Copy( double x, double y, a2dCanvasObjectFlagsMask mask, long target, bool check )
{
    int nr = 0;

    a2dRefMap refs;

    a2dCanvasObjectList::iterator iter = begin();
    while( iter != end() )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        if ( obj && check )
            obj->SetCheck( false );
        if ( obj && obj->CheckMask( mask ) )
        {
            nr++;
            a2dCanvasObject* objn = obj->TClone( a2dObject::clone_deep, &refs );
            push_front( objn );
            objn->Translate( x, y );
            objn->SetPending( true );
            objn->SetSpecificFlags( false, mask );
            if ( check )
                objn->SetCheck( true );
            if ( target != -1 )
                objn->SetLayer( target );
        }
        ++iter;
    }

    refs.LinkReferences();
    return nr;
}

int a2dCanvasObjectList::Move( double x, double y, a2dCanvasObjectFlagsMask mask, long target, bool check )
{
    int nr = 0;

    for( a2dCanvasObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj && check )
            obj->SetCheck( false );
        if ( obj && obj->CheckMask( mask ) )
        {
            nr++;
            obj->Translate( x, y );
            if ( check )
                obj->SetCheck( true );
            if ( target != -1 )
                obj->SetLayer( target );
        }
    }

    return nr;
}

int a2dCanvasObjectList::BringToTop( a2dCanvasObjectFlagsMask mask, bool check )
{
    a2dCanvasObjectList toTop;

    int nr = 0;
    a2dCanvasObjectList::iterator iter = begin();
    while( iter != end() )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        if ( obj && check )
            obj->SetCheck( false );
        if ( obj && obj->CheckMask( mask ) )
        {
            if ( check )
                obj->SetCheck( true );
            toTop.push_back( obj );
            nr++;
            iter = erase( iter );
            obj->SetPending( true );
        }
        else
            iter++;
    }
    iter = toTop.begin();
    while( iter != toTop.end() )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        push_back( obj );
        iter++;
    }
    return nr;
}

int a2dCanvasObjectList::BringToBack( a2dCanvasObjectFlagsMask mask, bool check )
{
    int nr = 0;
    a2dCanvasObjectList::iterator iter = begin();
    while( iter != end() )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        if ( obj && check )
            obj->SetCheck( false );
        if ( obj && obj->CheckMask( mask ) )
        {
            if ( check )
                obj->SetCheck( true );
            nr++;
            iter = erase( iter );
            insert( begin(), obj );
            obj->SetPending( true );
        }
        else
            iter++;
    }

    return nr;
}

bool a2dCanvasObjectList::SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which,
        const wxString& classname,
        a2dCanvasObjectFlagsMask whichobjects,
        const a2dBoundingBox& bbox,
        const a2dAffineMatrix& tworld )
{
    bool did = false;
    for( a2dCanvasObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj &&
                ( classname.IsEmpty() || obj->GetClassInfo()->GetClassName() == classname ) &&
                obj->CheckMask( whichobjects ) )
        {
            if ( bbox.GetValid() )
            {
                a2dBoundingBox tmp;
                tmp.Expand( obj->GetMappedBbox( tworld ) );
                if ( bbox.Intersect( tmp ) == _IN )
                {
                    did = true;
                    obj->SetSpecificFlags( setOrClear, which );
                }
            }
            else
            {
                did = true;
                obj->SetSpecificFlags( setOrClear, which );
            }
        }
    }
    return did;
}

a2dCanvasObject* a2dCanvasObjectList::Find( a2dCanvasObject* obj ) const
{
    a2dCanvasObject* cobj;

    for( const_iterator iter = begin(); iter != end(); ++iter )
    {
        cobj = *iter;
        if ( cobj && obj == cobj )
            return cobj;
    }

    return ( a2dCanvasObject* ) NULL;
}

a2dCanvasObject* a2dCanvasObjectList::Find( const wxString& objectname, const wxString& classname, a2dCanvasObjectFlagsMask mask, const a2dPropertyId* propid, const wxString& valueAsString, wxUint32 id ) const
{
    a2dCanvasObject* cobj;

    for( const_iterator iter = begin(); iter != end(); ++iter )
    {
        cobj = *iter;

        if ( cobj &&
                ( objectname.IsEmpty() || objectname.Matches( cobj->GetName() ) ) &&
                ( classname.IsEmpty() || cobj->GetClassInfo()->GetClassName() == classname ) &&
                ( id == 0 || cobj->GetId() == ( int )id ) &&
                ( !propid || cobj->HasProperty( propid, valueAsString ) ) &&
                ( cobj->CheckMask( mask ) )
           )
            return cobj;
    }

    return ( a2dCanvasObject* ) NULL;
}

bool a2dCanvasObjectList::SwitchObjectNamed( const wxString& objectname, a2dCanvasObject* newobject )
{
    a2dCanvasObject* cobj;

    for( a2dCanvasObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        cobj = *iter;

        if ( cobj && cobj->GetName() == objectname )
        {
            newobject->SetRoot( cobj->GetRoot(), false );
            *iter =  newobject;
            return true;
        }
    }

    return false;
}

bool a2dCanvasObjectList::SetDrawerStyle( const a2dFill& fill, const a2dStroke& stroke, a2dCanvasObjectFlagsMask mask )
{
    bool did = false;

    for( a2dCanvasObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj && obj->CheckMask( mask ) )
        {
            did = true;
            obj->SetFill( fill );
            obj->SetStroke( stroke );
        }
    }

    return did;
}

void a2dCanvasObjectList::Transform( const a2dAffineMatrix& tworld , const wxString& type, a2dCanvasObjectFlagsMask mask, const a2dPropertyId* id )
{
    a2dCanvasObject* cobj;

    for( a2dCanvasObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        cobj = *iter;

        if ( cobj &&
                ( type.IsEmpty() || cobj->GetClassInfo()->GetClassName() == type ) &&
                ( !id || cobj->HasProperty( id ) ) &&
                ( cobj->CheckMask( mask ) )
           )
        {
            cobj->Transform( tworld );
        }
    }
}

void a2dCanvasObjectList::SetTransform( const a2dAffineMatrix& tworld , const wxString& type, a2dCanvasObjectFlagsMask mask, const a2dPropertyId* id )
{
    a2dCanvasObject* cobj;

    for( a2dCanvasObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        cobj = *iter;

        if ( cobj &&
                ( type.IsEmpty() || cobj->GetClassInfo()->GetClassName() == type ) &&
                ( !id || cobj->HasProperty( id ) ) &&
                ( cobj->CheckMask( mask ) )
           )
        {
            cobj->SetTransformMatrix( tworld );
        }
    }
}

int a2dCanvasObjectList::CollectObjects( a2dCanvasObjectList* total, const wxString& type, a2dCanvasObjectFlagsMask mask,
        const a2dPropertyId* id,
        const a2dBoundingBox& bbox
                                       )
{
    a2dCanvasObject* cobj;
    int count = 0;

    for( a2dCanvasObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        cobj = *iter;

        if ( cobj &&
                !bbox.GetValid() || bbox.Intersect( cobj->GetBbox() ) == _IN )
        {
            if ( ( type.IsEmpty() || cobj->GetClassInfo()->GetClassName() == type ) &&
                    ( !id || cobj->HasProperty( id ) ) &&
                    ( cobj->CheckMask( mask ) )
               )
            {
                if ( total && total != wxNullCanvasObjectList )
                    total->push_back( cobj );
                count++;
            }
        }
    }

    return count;
}

int a2dCanvasObjectList::TakeOverTo( a2dCanvasObjectList* total, const wxString& type, a2dCanvasObjectFlagsMask mask, const a2dPropertyId* id )
{
    int count = 0;

    a2dCanvasObjectList::iterator iter = begin();
    while( iter != end() )
    {
        a2dCanvasObjectList::value_type cobj = *iter;

        if ( cobj &&
                ( type.IsEmpty() || cobj->GetClassInfo()->GetClassName() == type ) &&
                ( !id || cobj->HasProperty( id ) ) &&
                ( cobj->CheckMask( mask ) )
           )
        {
            if ( total && total != wxNullCanvasObjectList )
            {
                iter = erase( iter );
                total->push_back( cobj );
            }
            else
                iter++;
            count++;
        }
        else
            iter++;
    }

    return count;
}

int a2dCanvasObjectList::TakeOverFrom( a2dCanvasObjectList* total, const wxString& type, a2dCanvasObjectFlagsMask mask, const a2dPropertyId* id )
{
    return total->TakeOverTo( this, type, mask, id );
}

void a2dCanvasObjectList::AssertUnique()
{
#ifdef _DEBUG
    for( a2dCanvasObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        for( a2dCanvasObjectList::iterator iter2 = iter;  ( ++iter2 ) != end();  )
        {
            wxASSERT( *iter != *iter2 );
        }
    }
#endif
}

void a2dCanvasObjectList::Insert( size_t before, a2dCanvasObject* obj, bool ignoreReleased )
{
    a2dCanvasObjectList::iterator iter = begin();

    if ( !ignoreReleased && before > size() )
        push_back( obj );
    else
    {
        size_t i;
        for( i = 0; i < before; i++ )
        {
            a2dCanvasObject* obj = *iter;
            if ( ignoreReleased && obj->GetRelease()  )
                i--;
            iter++;
        }
        insert( iter, obj );
    }
    obj->SetPending( true );
}

bool a2dCanvasObjectList::RemoveDuplicates( const wxString& classname )
{
    bool did = false;
    a2dCanvasObjectList::iterator iter = begin();
    a2dCanvasObjectList::iterator iterc = iter;
    while( iter != end() )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj && ( classname.IsEmpty() || obj->GetClassInfo()->GetClassName() == classname ) )
        {
            iterc = iter;
            iterc ++;
            while( iterc != end() )
            {
                a2dCanvasObject* objc = *iterc;
                if ( objc == obj )
                {
                    *iterc = 0; //dereference of canvasobject
                    iterc = erase( iterc );
                    did = true;
                }
                else
                    iterc++;
            }
        }
        iter++;
    }
    return did;
}

//----------------------------------------------------------------------------
// a2dCorridor
//----------------------------------------------------------------------------
a2dCorridor::a2dCorridor()
{
}

a2dCorridor::~a2dCorridor()
{
}

a2dCorridor::a2dCorridor( const a2dIterC& ic )
{
    if ( ic.m_contextList.size() )
    {
        m_relativetransform = ic.m_contextList.back()->GetTransform();
        m_inverseRelativetransform = ic.m_contextList.back()->GetInverseTransform();
    }
	for( a2dSmrtPtrList< a2dIterPP >::const_iterator iter = ic.m_contextList.begin(); iter != ic.m_contextList.end(); iter++ )
    {
		a2dIterPP* pp = *iter;
		a2dCanvasObject* obj = pp->GetObject();
            push_back( obj );
    }
}

a2dCorridor::a2dCorridor( const a2dDrawingPart& view )
{
    a2dWalker_CollectCanvasObjects findcorridor( a2dCanvasOFlags::IsOnCorridorPath );
    if ( view.GetShowObject() )
    {
        findcorridor.SetSkipNotRenderedInDrawing( true );
        findcorridor.Start( view.GetShowObject() );
        if( findcorridor.m_found.size() )
        {
            this->TakeOverFrom( &findcorridor.m_found );
        }
    }

    a2dCanvasObject* cobj;
    for( a2dCanvasObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        cobj = *iter;
        if ( cobj )
        {
            m_relativetransform *= cobj->GetTransformMatrix();
        }
    }

    m_inverseRelativetransform = m_relativetransform;
    m_inverseRelativetransform.Invert();
}

void a2dCorridor::Push( a2dCanvasObject* object )
{
    push_back( object );

    a2dCanvasObject* cobj;
    for( a2dCanvasObjectList::iterator iter = begin(); iter != end(); ++iter )
    {
        cobj = *iter;
        if ( cobj )
        {
            m_relativetransform *= cobj->GetTransformMatrix();
        }
    }

    m_inverseRelativetransform = m_relativetransform;
    m_inverseRelativetransform.Invert();
}

