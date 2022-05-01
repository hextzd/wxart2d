/*! \file canvas/src/canglob.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: algos.cpp,v 1.4 2009/09/03 20:09:53 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <math.h>
#include <limits.h>
#include <float.h>

#include <wx/wfstream.h>

#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"
#include "wx/canvas/layerinf.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/cameleon.h"
#include "wx/canvas/drawer.h"
#include "wx/artbase/afmatrix.h"
#include "wx/artbase/drawer2d.h"


IMPLEMENT_CLASS( a2dWalker_SetPendingFlags, a2dWalker_SetSpecificFlagsCanvasObjects )
IMPLEMENT_CLASS( a2dWalker_ResetPendingFlags, a2dWalker_SetSpecificFlagsCanvasObjects )
IMPLEMENT_CLASS( a2dWalker_SetCheck, a2dWalker_SetBoolProperty )

//----------------------------------------------------------------------------
// a2dDumpWalker
//----------------------------------------------------------------------------


a2dDumpWalker::a2dDumpWalker()
{
    Initialize();
}

a2dDumpWalker::~a2dDumpWalker()
{
}

void a2dDumpWalker::Initialize()
{
}

bool a2dDumpWalker::Start( a2dObject* object )
{
    object->Walker( NULL, *this );
    return true;
}

bool a2dDumpWalker::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        //    a2dCanvasObject* parentobj = (a2dCanvasObject*) parent;
        a2dCanvasObject* canobj = ( a2dCanvasObject* ) object;
        if ( canobj->GetChildObjectList() != wxNullCanvasObjectList )
        {
            forEachIn( a2dCanvasObjectList, canobj->GetChildObjectList() )
            {
                //a2dCanvasObject *obj = (*iter);

            }
        }
    }

    if ( event == a2dWalker_a2dPropertyStart )
    {
        if ( wxDynamicCast( object, a2dObject ) )
            wxLogDebug( _T( "%s" ), wxDynamicCast( object, a2dObject )->GetName().c_str() );
    }

    if ( event == a2dWalker_a2dNamedPropertyStart )
    {
        if ( wxDynamicCast( object, a2dNamedProperty ) )
            wxLogDebug( _T( "%s" ), wxDynamicCast( object, a2dNamedProperty )->GetName().c_str() );

    }

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_CallMemberFunc
//----------------------------------------------------------------------------


a2dWalker_CallMemberFunc::a2dWalker_CallMemberFunc()
{
    SetDepthFirst( true );
    Initialize();
}

a2dWalker_CallMemberFunc::~a2dWalker_CallMemberFunc()
{
}

bool a2dWalker_CallMemberFunc::Start( a2dObject* object, a2dCanvasObjectFunc func )
{
    m_func = func;
    object->Walker( NULL, *this );
    return true;
}

bool a2dWalker_CallMemberFunc::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* canobj = ( a2dCanvasObject* ) object;
        ( canobj->*( ( a2dCanvasObjectFunc ) ( m_func ) ) )();
    }

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_CallMemberFuncW
//----------------------------------------------------------------------------

a2dWalker_CallMemberFuncW::a2dWalker_CallMemberFuncW()
{
    Initialize();
}

a2dWalker_CallMemberFuncW::~a2dWalker_CallMemberFuncW()
{
}

bool a2dWalker_CallMemberFuncW::Start( a2dObject* object, a2dCanvasObjectFuncW func )
{
    m_func = func;
    object->Walker( NULL, *this );
    return true;
}

bool a2dWalker_CallMemberFuncW::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    m_currentParent = parent;

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* canobj = ( a2dCanvasObject* ) object;
        ( canobj->*( ( a2dCanvasObjectFuncW ) ( m_func ) ) )( this );
    }

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_SetAvailable
//----------------------------------------------------------------------------

a2dWalker_SetAvailable::a2dWalker_SetAvailable( a2dLayers* layerSetup )
{
    Initialize();
    m_layerSetup = layerSetup;
    m_drawingPart = NULL;
}

a2dWalker_SetAvailable::a2dWalker_SetAvailable( a2dDrawingPart* drawingPart )
{
    Initialize();
    m_layerSetup = NULL;
    m_drawingPart = drawingPart;
}

a2dWalker_SetAvailable::~a2dWalker_SetAvailable()
{
}

bool a2dWalker_SetAvailable::Start( a2dObject* object )
{
    object->Walker( NULL, *this );
    return true;
}

bool a2dWalker_SetAvailable::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    bool foundfirst = false;

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* canobj = ( a2dCanvasObject* ) object;

        if ( 1 )//! wxDynamicCast( canobj, a2dLayers ) && ! wxDynamicCast( canobj, a2dLayerInfo ) )
        {
            if ( m_drawingPart )
                m_drawingPart->GetLayerRenderArray()[ canobj->GetLayer() ].SetAvailable( true );
            if ( m_layerSetup )
                m_layerSetup->GetLayerIndex()[ canobj->GetLayer() ]->SetAvailable( true );
            //  wxLogDebug( "available layer %d", obj->GetLayer() );
        }
    }

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_RemoveProperty
//----------------------------------------------------------------------------

a2dWalker_RemoveProperty::a2dWalker_RemoveProperty( bool all )
{
    m_all = all;
}

a2dWalker_RemoveProperty::a2dWalker_RemoveProperty( a2dPropertyId* id , bool all )
{
    m_propertyList.push_back( id );
    m_all = all;
}

a2dWalker_RemoveProperty::a2dWalker_RemoveProperty( const a2dPropertyIdList& idList, bool all )
{
    m_propertyList = idList;
    m_all = all;
}

a2dWalker_RemoveProperty::~a2dWalker_RemoveProperty()
{
}

void a2dWalker_RemoveProperty::AddPropertyId( a2dPropertyId* id )
{
    m_propertyList.push_back( id );
}


bool a2dWalker_RemoveProperty::Start( a2dObject* object )
{
    object->Walker( NULL, *this );
    return m_result;
}

bool a2dWalker_RemoveProperty::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dPropertyStart )
    {
        SetStopWalking( true );
        if ( wxDynamicCast( object, a2dObject ) )
        {
            a2dObject* propobj = wxDynamicCast( object, a2dObject );
            forEachIn( a2dPropertyIdList, &m_propertyList )
            {
                a2dPropertyId* id = *iter;
                m_result |= propobj->RemoveProperty( id, m_all );
            }
        }
    }
    if ( event == a2dWalker_a2dPropertyEnd )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}


//----------------------------------------------------------------------------
// a2dWalker_RemovePropertyCandoc
//----------------------------------------------------------------------------

a2dWalker_RemovePropertyCandoc::a2dWalker_RemovePropertyCandoc( a2dCanvasObjectFlagsMask mask, bool all )
{
    m_all = all;
    m_mask = mask;
    m_makePending = false;
}

a2dWalker_RemovePropertyCandoc::a2dWalker_RemovePropertyCandoc( a2dPropertyId* id , a2dCanvasObjectFlagsMask mask, bool all )
{
    m_propertyList.push_back( id );
    m_all = all;
    m_mask = mask;
    m_makePending = false;
}

a2dWalker_RemovePropertyCandoc::a2dWalker_RemovePropertyCandoc( const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask, bool all )
{
    m_propertyList = idList;
    m_all = all;
    m_mask = mask;
    m_makePending = false;
}

a2dWalker_RemovePropertyCandoc::~a2dWalker_RemovePropertyCandoc()
{
}

void a2dWalker_RemovePropertyCandoc::AddPropertyId( a2dPropertyId* id )
{
    m_propertyList.push_back( id );
}


bool a2dWalker_RemovePropertyCandoc::Start( a2dCanvasObject* object )
{
    object->Walker( NULL, *this );
    return m_result;
}

bool a2dWalker_RemovePropertyCandoc::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dPropertyStart )
        SetStopWalking( true );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* propobj =  wxDynamicCast( object, a2dCanvasObject );
        if ( propobj && propobj->CheckMask( m_mask ) )
        {
            forEachIn( a2dPropertyIdList, &m_propertyList )
            {
                a2dPropertyId* id = *iter;
                //propobj->RemoveProperty( *id, m_all );
                bool res = propobj->RemoveProperty( id, m_all );
                m_result |= res;
                if ( res && m_makePending )
                    propobj->SetPending( true );
            }
        }
    }
    if ( event == a2dWalker_a2dPropertyEnd )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_FilterCanvasObjects
//----------------------------------------------------------------------------

a2dWalker_FilterCanvasObjects::a2dWalker_FilterCanvasObjects( a2dCanvasObjectFlagsMask mask )
{
    Initialize();

    m_mask = mask;
}

a2dWalker_FilterCanvasObjects::a2dWalker_FilterCanvasObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
{
    Initialize();

    m_propertyList.push_back( ( a2dPropertyId* ) &id );
    m_mask = mask;
}

a2dWalker_FilterCanvasObjects::a2dWalker_FilterCanvasObjects( const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
{
    Initialize();

    m_propertyList = idList;
    m_mask = mask;
    m_result = false;
}

a2dWalker_FilterCanvasObjects::~a2dWalker_FilterCanvasObjects()
{
}

void a2dWalker_FilterCanvasObjects::Initialize()
{
    m_mask = a2dCanvasOFlags::ALL;
    m_skipStartObject = false;
    m_depth = INT_MAX;
    m_bbox = wxNonValidBbox;
    m_intersectionCondition = _IN;
    m_allowClassList = true;
    m_classnameMap.clear();
    m_objectname = wxT( "" );
    m_id = -1;
    m_layervisible = false;
    m_layerselectable = false;
    m_propertyList.clear();
}

void a2dWalker_FilterCanvasObjects::AddPropertyId( const a2dPropertyId* id )
{
    m_propertyList.push_back( ( a2dPropertyId* ) &id );
}


bool a2dWalker_FilterCanvasObjects::Start( a2dCanvasObject* object )
{
    object->Walker( NULL, *this );
    return m_result;
}

void a2dWalker_FilterCanvasObjects::AddClassName( wxString className )
{
    if ( m_classnameMap.find( className ) == m_classnameMap.end() )
        m_classnameMap.insert( className );
}

void a2dWalker_FilterCanvasObjects::RemoveClassName( wxString className )
{
    if ( m_classnameMap.find( className ) != m_classnameMap.end() )
        m_classnameMap.erase( m_classnameMap.find( className ) );
}

bool a2dWalker_FilterCanvasObjects::ObjectOke( a2dCanvasObject* obj )
{
    if ( obj &&
            ( m_objectname.IsEmpty() || m_objectname.Matches( obj->GetName() ) ) &&
            ( m_id == -1 || obj->GetId() == m_id ) &&
            obj->CheckMask( m_mask )
       )
    {
        if ( m_layervisible && m_layerselectable && obj->GetRoot() && obj->GetRoot()->GetLayerSetup() )
        {
            if ( m_layervisible && !obj->GetRoot()->GetLayerSetup()->GetVisible( obj->GetLayer() ) )
                return false;
            if ( m_layerselectable && !obj->GetRoot()->GetLayerSetup()->GetSelectable( obj->GetLayer() ) )
                return false;
        }

        if ( m_bbox.GetValid() && ( m_bbox.Intersect( obj->GetBbox() ) & m_intersectionCondition ) == 0 )
            return false;

        if ( m_propertyList.size() )
        {
            forEachIn( a2dPropertyIdList, &m_propertyList )
            {
                a2dPropertyId* id = *iter;
                if ( !obj->HasProperty( id ) )
                    return false;
            }
        }
        return true;
    }
    return false;
}

bool a2dWalker_FilterCanvasObjects::ClassOfObjectOke( a2dCanvasObject* obj )
{
    if ( ! m_classnameMap.empty() )
    {
        if ( !m_allowClassList )
        {
            // if class is in list, that is a non wanted object
            if ( m_classnameMap.find( obj->GetClassInfo()->GetClassName() ) != m_classnameMap.end() )
                return false;
            else
                return true;
        }
        else
        {
            // if class is NOT in list, that is a non wanted object
            if ( m_classnameMap.find( obj->GetClassInfo()->GetClassName() ) == m_classnameMap.end() )
                return false;
            else
                return true;
        }
    }
    return true;
}

a2dCanvasObjectList* a2dWalker_FilterCanvasObjects::ConvertToPolygons( a2dCanvasObject* canvasobject, bool transform )
{
   a2dCanvasObjectList* ret = canvasobject->GetAsPolygons( true );
   if ( ret != wxNullCanvasObjectList )
        return  ret;
   return NULL;
}

a2dCanvasObjectList* a2dWalker_FilterCanvasObjects::ConvertToVpath( a2dCanvasObject* canvasobject, bool transform )
{
    a2dCanvasObjectList* vpath = wxNullCanvasObjectList;

    if ( wxDynamicCast( canvasobject, a2dRect ) ||
            wxDynamicCast( canvasobject, a2dRectC ) ||
            wxDynamicCast( canvasobject, a2dArrow ) ||
            wxDynamicCast( canvasobject, a2dCircle ) ||
            wxDynamicCast( canvasobject, a2dEllipse ) ||
            wxDynamicCast( canvasobject, a2dEllipticArc ) ||
            wxDynamicCast( canvasobject, a2dArc ) ||
            wxDynamicCast( canvasobject, a2dPolygonL ) ||
            wxDynamicCast( canvasobject, a2dVectorPath ) ||
            wxDynamicCast( canvasobject, a2dText ) ||
            wxDynamicCast( canvasobject, a2dEndsLine ) ||
            wxDynamicCast( canvasobject, a2dSLine ) ||
            wxDynamicCast( canvasobject, a2dPolylineL )
       )
    {
        vpath = canvasobject->GetAsCanvasVpaths( transform );
    }
    return vpath;
}


//----------------------------------------------------------------------------
// a2dWalker_CollectCanvasObjects
//----------------------------------------------------------------------------

a2dWalker_CollectCanvasObjects::a2dWalker_CollectCanvasObjects( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();

    m_mask = mask;
}

a2dWalker_CollectCanvasObjects::a2dWalker_CollectCanvasObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( id, mask )
{
    Initialize();

    m_mask = mask;
}

a2dWalker_CollectCanvasObjects::a2dWalker_CollectCanvasObjects( const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( idList, mask )
{
    Initialize();

    m_propertyList = idList;
    m_mask = mask;
}

a2dWalker_CollectCanvasObjects::~a2dWalker_CollectCanvasObjects()
{
}

void a2dWalker_CollectCanvasObjects::Initialize()
{
    m_found.clear();
}

bool a2dWalker_CollectCanvasObjects::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{

    if ( m_currentDepth > m_depth || event == a2dWalker_a2dPropertyStart )
        SetStopWalking( true );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && ObjectOke( obj ) && ClassOfObjectOke( obj ) )
        {
            m_found.push_back( obj );
            m_result = true;
        }
    }

    if (  m_currentDepth < m_depth  && event == a2dWalker_a2dPropertyEnd )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_CollectCanvasObjectsSet
//----------------------------------------------------------------------------

bool NameSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    a2dCanvasObject* firstc = wxStaticCast( x.Get(), a2dCanvasObject );
    a2dCanvasObject* secondc = wxStaticCast( y.Get(), a2dCanvasObject );

    if ( firstc->GetName() < secondc->GetName() )
        return true;
    return false;
}

bool NameSorterReverse( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    a2dCanvasObject* firstc = wxStaticCast( x.Get(), a2dCanvasObject );
    a2dCanvasObject* secondc = wxStaticCast( y.Get(), a2dCanvasObject );

    if ( firstc->GetName() > secondc->GetName() )
        return true;
    return false;
}

bool OnNrChilds( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    a2dCanvasObject* firstc = wxStaticCast( x.Get(), a2dCanvasObject );
    a2dCanvasObject* secondc = wxStaticCast( y.Get(), a2dCanvasObject );

    if ( firstc->GetChildObjectsCount() > secondc->GetChildObjectsCount() )
        return true;
    else if ( firstc->GetChildObjectsCount() == secondc->GetChildObjectsCount() )
        return NameSorter( x,  y );
    return false;
}

bool OnNrChildsReverse( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    a2dCanvasObject* firstc = wxStaticCast( x.Get(), a2dCanvasObject );
    a2dCanvasObject* secondc = wxStaticCast( y.Get(), a2dCanvasObject );

    if ( firstc->GetChildObjectsCount() < secondc->GetChildObjectsCount() )
        return true;
    else if ( firstc->GetChildObjectsCount() == secondc->GetChildObjectsCount() )
        return NameSorter( x,  y );
    return false;
}

a2dWalker_CollectCanvasObjectsSet::a2dWalker_CollectCanvasObjectsSet( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();

    m_mask = mask;
}

a2dWalker_CollectCanvasObjectsSet::a2dWalker_CollectCanvasObjectsSet( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( id, mask )
{
    Initialize();

    m_mask = mask;
}

a2dWalker_CollectCanvasObjectsSet::a2dWalker_CollectCanvasObjectsSet( const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( idList, mask )
{
    Initialize();

    m_propertyList = idList;
    m_mask = mask;
}

a2dWalker_CollectCanvasObjectsSet::~a2dWalker_CollectCanvasObjectsSet()
{
}

void a2dWalker_CollectCanvasObjectsSet::Initialize()
{
    m_found.clear();
    m_childs = false;
    m_sortOn = onName;
}

bool a2dWalker_CollectCanvasObjectsSet::Start( a2dCanvasObject* object )
{
    switch ( m_sortOn )
    {
        case onName: s_a2dCanvasObjectSorter = &NameSorter; break;
        case onNameReverse: s_a2dCanvasObjectSorter = &NameSorterReverse; break;
        case onNrChilds: s_a2dCanvasObjectSorter = &OnNrChilds; break;
        case onNrChildsReverse: s_a2dCanvasObjectSorter = &OnNrChildsReverse; break;
    }

    object->Walker( NULL, *this );
    return m_result;
}

bool a2dWalker_CollectCanvasObjectsSet::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{

    if ( m_currentDepth > m_depth || event == a2dWalker_a2dPropertyStart )
        SetStopWalking( true );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && ObjectOke( obj ) && ClassOfObjectOke( obj ) &&
                ( !m_childs || ( m_childs && ( obj->GetChildObjectList() != wxNullCanvasObjectList ) ) ) )
        {
            if ( m_found.find( obj ) == m_found.end() )
                m_found.insert( obj );
            m_result = true;
        }
    }

    if (  m_currentDepth < m_depth  && event == a2dWalker_a2dPropertyEnd )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_FindCanvasObject
//----------------------------------------------------------------------------

a2dWalker_FindCanvasObject::a2dWalker_FindCanvasObject( a2dCanvasObject* search )
    : a2dWalkerIOHandler()
{
    Initialize();
    m_search = search;
}


a2dWalker_FindCanvasObject::~a2dWalker_FindCanvasObject()
{
}

void a2dWalker_FindCanvasObject::Initialize()
{
    m_search = 0;
}

bool a2dWalker_FindCanvasObject::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj == m_search )
        {
            SetStopWalking( true );
            m_result = true;
        }
    }

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

bool a2dWalker_FindCanvasObject::Start( a2dCanvasObject* object )
{
    m_result = false;
    object->Walker( NULL, *this );
    return m_result;
}


//----------------------------------------------------------------------------
// a2dWalker_SetSpecificFlagsCanvasObjects
//----------------------------------------------------------------------------

a2dWalker_SetSpecificFlagsCanvasObjects::a2dWalker_SetSpecificFlagsCanvasObjects( a2dCanvasObjectFlagsMask which, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( mask )
{
    m_which = which;
    Initialize();
}

a2dWalker_SetSpecificFlagsCanvasObjects::a2dWalker_SetSpecificFlagsCanvasObjects( a2dCanvasObjectFlagsMask which, const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( id, mask )
{
    m_which = which;
    Initialize();
}

a2dWalker_SetSpecificFlagsCanvasObjects::a2dWalker_SetSpecificFlagsCanvasObjects( a2dCanvasObjectFlagsMask which, const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( idList, mask )
{
    m_which = which;
    Initialize();
}

a2dWalker_SetSpecificFlagsCanvasObjects::~a2dWalker_SetSpecificFlagsCanvasObjects()
{
}

bool a2dWalker_SetSpecificFlagsCanvasObjects::Start( a2dCanvasObject* object, bool setTo )
{
    if ( !object )
        return false;
    m_setOrClear = setTo;
    object->Walker( NULL, *this );
    return m_result;
}

void a2dWalker_SetSpecificFlagsCanvasObjects::Initialize()
{
}

bool a2dWalker_SetSpecificFlagsCanvasObjects::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{

    if ( m_currentDepth > m_depth || event == a2dWalker_a2dPropertyStart )
        SetStopWalking( true );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( !m_skipStartObject || ( m_skipStartObject && m_currentDepth > 1 ) )
            if ( obj && ObjectOke( obj ) && ClassOfObjectOke( obj ) )
            {
                obj->SetSpecificFlags( m_setOrClear, m_which );
                m_result = true;
            }
    }

    if (  m_currentDepth < m_depth  && event == a2dWalker_a2dPropertyEnd )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_SetPropertyToObject
//----------------------------------------------------------------------------

a2dWalker_SetPropertyToObject::a2dWalker_SetPropertyToObject( a2dNamedProperty* prop )
{
    m_property = prop;
    Initialize();
}

a2dWalker_SetPropertyToObject::~a2dWalker_SetPropertyToObject()
{
}

void a2dWalker_SetPropertyToObject::Initialize()
{
}

bool a2dWalker_SetPropertyToObject::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        m_property->SetToObjectClone( obj, a2dPropertyId::set_none );
    }

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_SetPendingFlags
//----------------------------------------------------------------------------

bool a2dWalker_SetPendingFlags::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{

    if ( m_currentDepth > m_depth || event == a2dWalker_a2dPropertyStart )
        SetStopWalking( true );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && obj->CheckMask( m_mask ) )
        {
            obj->SetPending( m_setOrClear );
            obj->SetPendingSelect( m_setOrClear );
            m_result = true;
        }
    }

    if (  m_currentDepth < m_depth  && event == a2dWalker_a2dPropertyEnd )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_ResetPendingFlags
//----------------------------------------------------------------------------

bool a2dWalker_ResetPendingFlags::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{

    if ( m_currentDepth > m_depth || event == a2dWalker_a2dPropertyStart )
        SetStopWalking( true );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && ( obj->GetPending() || obj->GetPendingSelect() ) )
        {
            obj->SetPending( m_setOrClear );
            obj->SetPendingSelect( m_setOrClear );
            m_result = true;
        }
    }

    if (  m_currentDepth < m_depth  && event == a2dWalker_a2dPropertyEnd )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_SetViewDependent
//----------------------------------------------------------------------------

a2dWalker_SetViewDependent::a2dWalker_SetViewDependent( a2dDrawingPart* aView, bool viewdependent, bool viewspecific, bool onlyinternalarea )
{
    m_drawingPart = aView;
    m_viewdependent = viewdependent;
    m_viewspecific = viewspecific;
    m_onlyinternalarea = onlyinternalarea;
}

a2dWalker_SetViewDependent::~a2dWalker_SetViewDependent()
{
}

void a2dWalker_SetViewDependent::Initialize()
{
}

bool a2dWalker_SetViewDependent::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        obj->SetViewDependent( m_drawingPart, m_viewdependent, m_viewspecific, m_onlyinternalarea );
    }

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

bool a2dWalker_SetViewDependent::Start( a2dCanvasObject* object )
{
    object->Walker( NULL, *this );
    return true;
}


//----------------------------------------------------------------------------
// a2dWalker_SetRoot
//----------------------------------------------------------------------------
IMPLEMENT_CLASS( a2dWalker_SetRoot, a2dWalkerIOHandler )

a2dWalker_SetRoot::a2dWalker_SetRoot( a2dDrawing* root )
{
    m_root = root;
    m_skipNotRenderedInDrawing = true;
}

a2dWalker_SetRoot::~a2dWalker_SetRoot()
{
}

bool a2dWalker_SetRoot::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        a2dDrawing* drawingInDrawing =  wxDynamicCast( object, a2dDrawing );
        if ( drawingInDrawing && drawingInDrawing != m_root )
            //here we start a new walker, with the root of the found drawing.
            drawingInDrawing->SetRootRecursive();            
        else     
            obj->SetRoot( m_root, false );
    }

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

bool a2dWalker_SetRoot::Start( a2dCanvasObject* object )
{
    object->Walker( NULL, *this );
    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_MakeTree
//----------------------------------------------------------------------------

a2dWalker_MakeTree::a2dWalker_MakeTree( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();
    m_mask = mask;
}

a2dWalker_MakeTree::a2dWalker_MakeTree( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( id, mask )
{
    Initialize();
    m_mask = mask;
}

a2dWalker_MakeTree::a2dWalker_MakeTree( const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( idList, mask )
{
    Initialize();
}

a2dWalker_MakeTree::~a2dWalker_MakeTree()
{
}

bool a2dWalker_MakeTree::Start( a2dCanvasObject* object )
{
    a2dWalker_SetCheck setp( false );
    setp.Start( object );

    object->Walker( NULL, *this );
    return true;
}

void a2dWalker_MakeTree::Initialize()
{
    m_useCheck = true;
    m_depthFirst = true;
    m_firstLevelChild = false;
}

bool a2dWalker_MakeTree::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( m_currentDepth > m_depth  )
        SetStopWalking( true );
    else
        SetStopWalking( false );

    if ( event == a2dWalker_a2dObjectStart )
    {
        //wxLogDebug(_T("a2dobj start %s"), wxDynamicCast( object, a2dObject )->GetName() );
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && m_currentDepth == 2 && ObjectOke( obj ) && ClassOfObjectOke( obj ) && !obj->GetCheck() )
        {
            m_firstLevelChild = true;
            obj->SetCheck( true );
        }
    }

    if ( event == a2dWalker_a2dCanvasObjectEnd )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        //wxLogDebug( wxT( "classname = %s" ), obj->GetClassInfo()->GetClassName().c_str() );
        //wxLogDebug( wxT( "name = %s" ), obj->GetName().c_str() );

        if ( obj && m_firstLevelChild )
        {
            if ( wxDynamicCast( object, a2dCanvasObjectArrayReference ) )
            {
                a2dCanvasObjectArrayReference* arrayref = wxDynamicCast( object, a2dCanvasObjectArrayReference );
                if ( arrayref->GetCanvasObject() )
                {
                    a2dAffineMatrix offsetXY;
                    int i, j;
                    for ( i = 0  ; i < arrayref->GetRows(); i++ )
                    {
                        for ( j = 0  ; j < arrayref->GetColumns(); j++ )
                        {
                            a2dCanvasObjectPtr cobj = ( a2dCanvasObject* ) arrayref->GetCanvasObject()->Clone( clone_deep );
                            cobj->SetRelease( false );

                            obj->CreateChildObjectList()->push_back( cobj );
                            cobj->Transform( offsetXY );

                            offsetXY.Translate( arrayref->GetHorzSpace(), 0 );
                        }
                        //translate back (one row of columns) and add one row
                        offsetXY.Translate( -arrayref->GetHorzSpace()*arrayref->GetColumns(), arrayref->GetVertSpace() );
                    }
                    arrayref->SetCanvasObject( NULL );
                }
            }
            else if ( wxDynamicCast( object, a2dCanvasObjectReference ) )
            {
                a2dCanvasObjectReference* ref = wxDynamicCast( object, a2dCanvasObjectReference );

                if ( ref->GetCanvasObject() )
                {
                    a2dCanvasObjectPtr cobj = ( a2dCanvasObject* ) ref->GetCanvasObject()->Clone( clone_deep );
                    cobj->SetRelease( false );

                    obj->CreateChildObjectList()->push_back( cobj );
                    ref->SetCanvasObject( NULL );
                }
            }

            // references are now (must be) gone in derived objects.
            // There for the obj has only children at this stage.
            obj->MakeReferencesUnique();
            obj->SetCheck( true );

        }
    }

    if ( event == a2dWalker_a2dObjectEnd )
    {
        //wxLogDebug(_T("a2dobj end %s"), wxDynamicCast( object, a2dObject )->GetName().c_str() );
        if ( m_currentDepth <= 2 )
        {
            m_firstLevelChild = false;
        }
    }

    if (  m_currentDepth <= m_depth   )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_RemoveHierarchy
//----------------------------------------------------------------------------

a2dWalker_RemoveHierarchy::a2dWalker_RemoveHierarchy( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();
    m_mask = mask;
}

a2dWalker_RemoveHierarchy::a2dWalker_RemoveHierarchy( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( id, mask )
{
    Initialize();
    m_mask = mask;
}

a2dWalker_RemoveHierarchy::a2dWalker_RemoveHierarchy( const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( idList, mask )
{
    Initialize();
}

a2dWalker_RemoveHierarchy::~a2dWalker_RemoveHierarchy()
{
}

bool a2dWalker_RemoveHierarchy::Start( a2dCanvasObject* object )
{
    a2dWalker_MakeTree unify( m_mask );
    unify.Start( object );

    a2dWalker_SetCheck setp( false );
    setp.Start( object );
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
    setflags.SetSkipNotRenderedInDrawing( true );
    setflags.Start( object, false );

    object->Walker( NULL, *this );
    return true;
}

void a2dWalker_RemoveHierarchy::Initialize()
{
    m_selected = true;
    m_useCheck = true;
    m_depthFirst = true;
    m_firstLevelChild = false;
}

bool a2dWalker_RemoveHierarchy::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( m_currentDepth > m_depth  )
        SetStopWalking( true );
    else
        SetStopWalking( false );

    if ( event == a2dWalker_a2dObjectStart )
    {
        m_parentList.push_front( wxDynamicCast( object, a2dObject ) );

        //wxLogDebug(_T("a2dobj start %s"), wxDynamicCast( object, a2dObject )->GetName().c_str() );

        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && m_parentList.size() == 2 && ObjectOke( obj ) && ClassOfObjectOke( obj ) && !obj->GetCheck() )
        {
            m_firstLevelChild = true;
            obj->SetCheck( true );
        }
    }

    if ( event == a2dWalker_a2dCanvasObjectEnd )
    {
        a2dCanvasObject* parentobj =  wxDynamicCast( parent, a2dCanvasObject );
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );

        //wxLogDebug( wxT( "classname = %s" ), obj->GetClassInfo()->GetClassName().c_str() );
        //wxLogDebug( wxT( "name = %s" ), obj->GetName().c_str() );

        if ( obj && !obj->GetBin() && parentobj && m_firstLevelChild )
        {
            if ( wxDynamicCast( object, a2dCanvasObjectReference ) )
            {
                a2dCanvasObjectReference* ref = wxDynamicCast( object, a2dCanvasObjectReference );
                wxASSERT_MSG( ref->GetCanvasObject() == NULL, wxT( "No references allowed at this stage" ) );
                if ( ref->GetChildObjectsCount() > 0 )
                    ref->GetChildObjectList()->front()->SetRelease( true );
            }
            if ( wxDynamicCast( object, a2dCanvasObjectArrayReference ) )
            {
                a2dCanvasObjectArrayReference* arrayref = wxDynamicCast( object, a2dCanvasObjectArrayReference );
                wxASSERT_MSG( arrayref->GetCanvasObject() == NULL, wxT( "No references allowed at this stage" ) );
            }

            if ( obj->GetChildObjectList() != wxNullCanvasObjectList )
            {
                // release later on, since there is an iterator more on it in a2dCanvasObject::DoWalker()
                obj->SetRelease( true );

                a2dCanvasObjectList::iterator iter = obj->GetChildObjectList()->begin();
                while( iter != obj->GetChildObjectList()->end() )
                {
                    a2dCanvasObjectList::value_type objchild = *iter;
                    //iter++;
                    iter = obj->GetChildObjectList()->erase( iter );

                    if ( !objchild->GetRelease() )
                    {
                        a2dPin* pin = wxDynamicCast( objchild.Get(), a2dPin ); 
                        if ( pin )
                            pin->Disconnect( NULL ); 

                        //parentobj->CreateChildObjectList()->insert( iterinserthere, obj );
                        parentobj->CreateChildObjectList()->push_back( objchild );
                        //objchild->SetBin( true );
                        objchild->SetParent( parentobj );
                        objchild->Transform( obj->GetTransformMatrix() );
                        objchild->SetSelected( m_selected );
                    }
                }
            }
        }
    }

    if ( event == a2dWalker_a2dObjectEnd )
    {
        //wxLogDebug(_T("a2dobj end %s"), wxDynamicCast( object, a2dObject )->GetName().c_str() );

        m_parentList.pop_front();
        if ( m_parentList.size() < 2 )
        {
            m_firstLevelChild = false;
        }
    }

    if (  m_currentDepth <= m_depth   )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_LayerGroup
//----------------------------------------------------------------------------

a2dWalker_LayerGroup::a2dWalker_LayerGroup( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();
}

a2dWalker_LayerGroup::a2dWalker_LayerGroup( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( id, mask )
{
    Initialize();
}

a2dWalker_LayerGroup::a2dWalker_LayerGroup( const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( idList, mask )
{
    Initialize();
}

a2dWalker_LayerGroup::~a2dWalker_LayerGroup()
{
}

void a2dWalker_LayerGroup::Initialize()
{
    m_target = wxLAYER_DEFAULT;
    m_selectedOnlyA = false;
    m_selectedOnlyB = false;
}

void a2dWalker_LayerGroup::SetTarget( wxUint16 targetlayer )
{
    wxASSERT_MSG( wxMAXLAYER > targetlayer , wxT( " targetlayer > wxMAXLAYER, index to high" ) );
    m_target = targetlayer;
};

bool a2dWalker_LayerGroup::Start( a2dCanvasObject* object )
{
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
    setflags.SetSkipNotRenderedInDrawing( true );
    setflags.Start( object, false );

    object->Walker( NULL, *this );
    return m_result;
}

//----------------------------------------------------------------------------
// a2dWalker_LayerCanvasObjects
//----------------------------------------------------------------------------

const long a2dWalker_LayerCanvasObjects::moveLayers = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::copyLayers = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::deleteLayers = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::ConvertToArcs = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::ConvertToPolygonPolylinesWithArcs = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::ConvertToPolygonPolylinesWithoutArcs = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::ConvertPolygonToArcs = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::ConvertPolylineToArcs = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::ConvertToVPaths = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::ConvertLinesArcs = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::ConvertToPolylines = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::RemoveRedundant = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::areaLayers = wxGenNewId();
const long a2dWalker_LayerCanvasObjects::ConvertPointsAtDistance = wxGenNewId();

a2dWalker_LayerCanvasObjects::a2dWalker_LayerCanvasObjects( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( mask )
{
    Initialize();
    m_mask = mask;
}

a2dWalker_LayerCanvasObjects::a2dWalker_LayerCanvasObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( id, mask )
{
    Initialize();
    m_mask = mask;
}


a2dWalker_LayerCanvasObjects::a2dWalker_LayerCanvasObjects( const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( idList, mask )
{
    Initialize();
    m_mask = mask;
}

a2dWalker_LayerCanvasObjects::~a2dWalker_LayerCanvasObjects()
{
}

void a2dWalker_LayerCanvasObjects::Initialize()
{
    m_operation = moveLayers;
    m_radiusMin = 0;
    m_radiusMax = DBL_MAX;
    m_detectCircle = false;
    m_calculatedArea = 0.0;
    m_distancePoints = 100;
    m_atVertexPoints = false;
    m_AberPolyToArc = a2dCanvasGlobals->GetHabitat()->ACCUR();
    m_lastPointCloseToFirst = true;
}

bool a2dWalker_LayerCanvasObjects::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( m_currentDepth > m_depth  )
        SetStopWalking( true );
    else
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* parentobj =  wxDynamicCast( parent, a2dCanvasObject );
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && parentobj && !obj->GetIgnoreLayer() && !obj->GetBin() && ObjectOke( obj ) && ClassOfObjectOke( obj ) )

        {
            bool hasLayer = m_groupA.InGroup( obj->GetLayer() );
            if ( hasLayer && ( !m_selectedOnlyA || ( m_selectedOnlyA && obj->GetSelected() ) ) )
            {
                if ( m_operation == moveLayers )
                {
                    obj->SetLayer( m_target );
                }
                else if ( m_operation == copyLayers )
                {
                    a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                    parentobj->Prepend( cobj );
                    cobj->SetLayer( m_target );
                    cobj->SetBin( true );
                }
                else if ( m_operation == deleteLayers )
                {
                    parentobj->ReleaseChild( obj );
                }
                else if ( m_operation == ConvertToArcs )
                {
                    if ( 0 != wxDynamicCast( obj, a2dPolylineL ) && obj->GetContourWidth() )
                    {
                        a2dPolylineL* objc = wxDynamicCast( obj, a2dPolylineL );
                        bool returnIsPolygon;
                        a2dPolygonL* contour = new a2dPolygonL( objc->GetAsVertexList( returnIsPolygon ) );
                        contour->SetBin( true );
                        //contour->GetSegments()->ConvertPolylineToArc( m_AberPolyToArc, m_radiusMin, m_radiusMax );
                        parentobj->Append( contour );
                        contour->SetLayer( m_target );
                    }
                }
                else if ( m_operation == ConvertPolygonToArcs )
                {
                    if ( 0 != wxDynamicCast( obj, a2dPolygonL ) )
                    {
                        a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                        cobj->SetBin( true );
                        a2dPolygonL* objc = wxDynamicCast( cobj, a2dPolygonL );
                        objc->GetSegments()->ConvertPolygonToArc( m_AberPolyToArc, m_radiusMin, m_radiusMax );
                        a2dPoint2D middle;
                        double radius;
                        if ( m_detectCircle && objc->GetSegments()->CheckForOneCircle( middle, radius ) )
                        {
                            a2dCircle* cir = new a2dCircle( middle.m_x, middle.m_y, radius );
                            cir->Transform( obj->GetTransformMatrix() );
                            cir->SetBin( true );
                            parentobj->Prepend( cir );
                            cir->SetLayer( m_target );
                            delete cobj;
                        }
                        else
                        {
                            parentobj->Prepend( cobj );
                            cobj->SetLayer( m_target );
                        }
                    }
                }
                else if ( m_operation == ConvertPolylineToArcs )
                {
                    if ( 0 != wxDynamicCast( obj, a2dPolylineL ) && obj->GetContourWidth() )
                    {
                        a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                        a2dPolylineL* objc = wxDynamicCast( cobj, a2dPolylineL );
                        cobj->SetBin( true );
                        parentobj->Prepend( cobj );
                        cobj->SetLayer( m_target );
                        objc->GetSegments()->ConvertPolylineToArc( m_AberPolyToArc, m_radiusMin, m_radiusMax );
                    }
                    else if ( 0 != wxDynamicCast( obj, a2dPolylineL ) )
                    {
                        a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                        cobj->SetBin( true );
                        parentobj->Prepend( cobj );
                        cobj->SetLayer( m_target );
                        a2dPolylineL* objc = wxDynamicCast( cobj, a2dPolylineL );
                        objc->GetSegments()->ConvertPolylineToArc( m_AberPolyToArc, m_radiusMin, m_radiusMax );
                    }
                }
                else if ( m_operation == ConvertToPolygonPolylinesWithArcs )
                {
                    a2dCanvasObjectList* polylist = ConvertToPolygons( obj, true );
                    for( a2dCanvasObjectList::iterator iterp = polylist->begin(); iterp != polylist->end(); ++iterp )
                    {
                        a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                        a2dPolylineL* polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                        if ( poly && poly->GetNumberOfSegments() > 2 )
                        {
                            poly->SetBin( true );
                            parentobj->Prepend( poly );
                            poly->SetLayer( m_target );
                        }
                        else if ( polyline && polyline->GetNumberOfSegments() > 2 )
                        {
                            polyline->SetBin( true );
                            parentobj->Prepend( polyline );
                            polyline->SetLayer( m_target );
                        }
                    }
                    delete polylist;
                }
                else if ( m_operation == ConvertToPolygonPolylinesWithoutArcs )
                {
                    a2dCanvasObjectList* polylist = ConvertToPolygons( obj, true );
                    for( a2dCanvasObjectList::iterator iterp = polylist->begin(); iterp != polylist->end(); ++iterp )
                    {
                        a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                        a2dPolylineL* polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                        if ( poly && poly->GetNumberOfSegments() > 2 )
                        {
                            poly->SetBin( true );
                            poly->GetSegments()->ConvertToLines( m_AberArcToPoly );
                            parentobj->Prepend( poly );
                            poly->SetLayer( m_target );
                        }
                        else if ( polyline && polyline->GetNumberOfSegments() > 2 )
                        {
                            polyline->SetBin( true );
                            polyline->GetSegments()->ConvertToLines( m_AberArcToPoly );
                            parentobj->Prepend( polyline );
                            polyline->SetLayer( m_target );
                        }
                    }
                    delete polylist;
                }
                else if ( m_operation == ConvertToVPaths )
                {
                    a2dCanvasObjectList* vpath = ConvertToVpath( obj, true );
                    for( a2dCanvasObjectList::iterator iterp = vpath->begin(); iterp != vpath->end(); ++iterp )
                    {
                        a2dVectorPath* path = wxDynamicCast( ( *iterp ).Get(), a2dVectorPath );
                        path->SetBin( true );
                        parentobj->Prepend( path );
                        path->SetLayer( m_target );
                    }
                    delete vpath;
                }
                else if ( m_operation == ConvertToPolylines )
                {
                    a2dCanvasObjectList* polylist = obj->GetAsPolylines( true );
                    if ( polylist != wxNullCanvasObjectList )
                    {
                        int i, count = polylist->size();
                        a2dCanvasObjectList::iterator iterp = polylist->begin();
                        for( i = 0 ; i < count ; i++ )
                        {
                            a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                            a2dPolylineL* polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                            if ( poly && poly->GetNumberOfSegments() > 2 )
                            {
                                a2dPolylineL* conv = new a2dPolylineL( poly->GetSegments() );

                                conv->SetBin( true );
                                parentobj->Prepend( conv );
                                conv->SetLayer( m_target );
                                iterp = polylist->erase( iterp );
                            }
                            else if ( polyline && polyline->GetNumberOfSegments() > 2 )
                            {
                                polyline->SetBin( true );
                                parentobj->Prepend( polyline );
                                polyline->SetLayer( m_target );
                                iterp++;
                            }
                        }
                        delete polylist;
                    }
                }
                else if ( m_operation == ConvertLinesArcs )
                {
                    a2dCanvasObjectList* polylist = ConvertToPolygons( obj, true );
                    int i, count = polylist->size();
                    a2dCanvasObjectList::iterator iterp = polylist->begin();
                    for( i = 0 ; i < count ; i++ )
                    {
                        a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                        poly->SetLayer( m_target );
                        poly->SetBin( true );
                        a2dCanvasObjectList* linesAndArcs = poly->GetAsLinesArcs();
                        for( a2dCanvasObjectList::iterator iterl = linesAndArcs->begin(); iterl != linesAndArcs->end(); ++iterl )
                            parentobj->Prepend( *iterl );
                        iterp = polylist->erase( iterp );
                        delete linesAndArcs;
                    }
                    delete polylist;
                }
                else if ( m_operation == RemoveRedundant )
                {
                    if ( 0 != wxDynamicCast( obj, a2dPolylineL ) )
                    {
                        a2dPolylineL* cobj = wxDynamicCast( obj, a2dPolylineL );
                        cobj->GetSegments()->RemoveRedundant( false, m_small );
                        cobj->SetLayer( m_target );
                        cobj->SetBin( true );
                    }
                    else if ( 0 != wxDynamicCast( obj, a2dPolygonL ) )
                    {
                        a2dPolygonL* cobj = wxDynamicCast( obj, a2dPolygonL );
                        cobj->GetSegments()->RemoveRedundant( true, m_small );
                        cobj->SetLayer( m_target );
                        cobj->SetBin( true );
                    }
                }
                else if ( m_operation == ConvertPointsAtDistance )
                {
                    a2dCanvasObjectList* polylist = ConvertToPolygons( obj, true );
                    for( a2dCanvasObjectList::iterator iterp = polylist->begin(); iterp != polylist->end(); ++iterp )
                    {
                        a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                        a2dPolylineL* polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                        if ( poly && poly->GetNumberOfSegments() > 2 )
                        {
                            poly->SetBin( true );
                            poly->GetSegments()->ConvertToLines( m_AberArcToPoly );
                            a2dVertexListPtr out = poly->GetSegments()->ConvertPointsAtDistance( m_distancePoints, true, m_atVertexPoints, m_lastPointCloseToFirst );
                            a2dVertexList::iterator iterp = out->begin();
                            a2dPolygonL* newpoly = new a2dPolygonL( out );
                            newpoly->SetPosXY( poly->GetPosX(), poly->GetPosY() );
                            parentobj->Prepend( newpoly );
                            newpoly->SetLayer( m_target );
                        }
                        else if ( polyline && polyline->GetNumberOfSegments() > 2 )
                        {
                            polyline->SetBin( true );
                            polyline->GetSegments()->ConvertToLines( m_AberArcToPoly );
                            a2dVertexListPtr out = poly->GetSegments()->ConvertPointsAtDistance( m_distancePoints, false, m_atVertexPoints, m_lastPointCloseToFirst );
                            a2dVertexList::iterator iterp = out->begin();
                            a2dPolylineL* newpoly = new a2dPolylineL( out );
                            newpoly->SetPosXY( poly->GetPosX(), poly->GetPosY() );
                            parentobj->Prepend( newpoly );
                            newpoly->SetLayer( m_target );
                        }
                    }
                    delete polylist;
                }
                else if ( m_operation == areaLayers )
                {
                    AddArea( obj );
                }
                m_result = true;
            }
        }
    }

    if (  m_currentDepth <= m_depth   )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

void a2dWalker_LayerCanvasObjects::AddArea( a2dCanvasObject* getAsPolyObject )
{
    a2dCanvasObjectList* vpath = wxNullCanvasObjectList;

    if ( 0 != wxDynamicCast( getAsPolyObject, a2dRect ) )
    {
        a2dRect* obj = wxDynamicCast( getAsPolyObject, a2dRect );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dRectC ) )
    {
        a2dRectC* obj = wxDynamicCast( getAsPolyObject, a2dRectC );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dArrow ) )
    {
        a2dArrow* obj = wxDynamicCast( getAsPolyObject, a2dArrow );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dCircle ) )
    {
        a2dCircle* obj = wxDynamicCast( getAsPolyObject, a2dCircle );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dEllipse ) )
    {
        a2dEllipse* obj = wxDynamicCast( getAsPolyObject, a2dEllipse );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dEllipticArc ) )
    {
        a2dEllipticArc* obj = wxDynamicCast( getAsPolyObject, a2dEllipticArc );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dArc ) )
    {
        a2dArc* obj = wxDynamicCast( getAsPolyObject, a2dArc );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dEndsLine ) && getAsPolyObject->GetContourWidth() )
    {
        a2dEndsLine* obj = wxDynamicCast( getAsPolyObject, a2dEndsLine );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dSLine ) && getAsPolyObject->GetContourWidth() )
    {
        a2dSLine* obj = wxDynamicCast( getAsPolyObject, a2dSLine );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dPolylineL ) && getAsPolyObject->GetContourWidth() )
    {
        a2dPolylineL* obj = wxDynamicCast( getAsPolyObject, a2dPolylineL );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxDynamicCast( getAsPolyObject, a2dPolygonL );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dVectorPath ) )
    {
        a2dVectorPath* obj = wxDynamicCast( getAsPolyObject, a2dVectorPath );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( getAsPolyObject, a2dText ) )
    {
        a2dText* obj = wxDynamicCast( getAsPolyObject, a2dText );
        vpath = obj->GetAsCanvasVpaths();
    }

    // now we have vector path, which we will convert to polygons and polylines.
    // Next only the polygons will be used for boolean operations.
    if ( vpath != wxNullCanvasObjectList )
    {

        a2dCanvasObjectList::iterator iter = vpath->begin();
        while ( iter != vpath->end() )
        {
            a2dCanvasObjectList* polylist = NULL;
            a2dVectorPath* obj = ( a2dVectorPath* ) ( *iter ).Get();
            polylist = obj->GetAsPolygons();

            for( a2dCanvasObjectList::iterator iterp = polylist->begin(); iterp != polylist->end(); ++iterp )
            {
                a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                if ( poly && poly->GetNumberOfSegments() > 2 )
                {
                    m_calculatedArea += poly->GetSegments()->CalcArea();
                }
            }
            delete polylist;

            iter = vpath->erase( iter );
            iter = vpath->begin();
        }
        delete vpath;
    }
}


//----------------------------------------------------------------------------
// a2dWalker_DetectSmall
//----------------------------------------------------------------------------

a2dWalker_DetectSmall::a2dWalker_DetectSmall( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( mask )
{
    Initialize();
}

a2dWalker_DetectSmall::a2dWalker_DetectSmall( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( id, mask )
{
    Initialize();
}

a2dWalker_DetectSmall::a2dWalker_DetectSmall( const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( idList, mask )
{
    Initialize();
}

a2dWalker_DetectSmall::~a2dWalker_DetectSmall()
{
}

void a2dWalker_DetectSmall::Initialize()
{
    m_asString = false;
    /*
    #if wxUSE_STD_IOSTREAM
        m_strstream << _T("Contents of document \n");
    #else
        wxTextOutputStream text_stream( m_strstream );
        text_stream << _T("Contents of document \n");
    #endif
    */
}

bool a2dWalker_DetectSmall::Start( a2dCanvasObject* object )
{
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
    setflags.Start( object, false );

    m_CVGwriter.WriteCvgStartDocument( m_strstream );

    m_CVGwriter.WriteStartElementAttributes( wxT( "o" ) );
    m_CVGwriter.WriteAttribute( wxT( "classname" ), wxT( "a2dCanvasObject" ) );
    m_CVGwriter.WriteEndAttributes();

    object->Walker( NULL, *this );

    m_CVGwriter.WriteEndElement();

    m_CVGwriter.WriteCvgEndDocument();

    return m_result;
}

bool a2dWalker_DetectSmall::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( m_currentDepth > m_depth  )
        SetStopWalking( true );
    else
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* parentobj =  wxDynamicCast( parent, a2dCanvasObject );
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && parentobj && !obj->GetIgnoreLayer() && !obj->GetBin() && ObjectOke( obj ) && ClassOfObjectOke( obj ) )
        {
            bool hasLayer = m_groupA.InGroup( obj->GetLayer() );
            if ( hasLayer && ( !m_selectedOnlyA || ( m_selectedOnlyA && obj->GetSelected() ) ) )
            {
                if ( obj->GetBboxWidth() < m_small && obj->GetBboxHeight() < m_small )
                {
                    if ( m_asString )
                    {
#if wxART2D_USE_CVGIO
                        m_CVGwriter.WriteObject( obj );
#endif //wxART2D_USE_CVGIO
                    }
                    else
                    {
                        a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                        cobj->SetBin( true );
                        parentobj->Prepend( cobj );
                        cobj->SetLayer( m_target );
                    }
                    m_result = true;
                }
            }
        }
    }

    if (  m_currentDepth <= m_depth   )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_DetectSmallSegments
//----------------------------------------------------------------------------

a2dWalker_DetectSmallSegments::a2dWalker_DetectSmallSegments( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( mask )
{
    Initialize();
}

a2dWalker_DetectSmallSegments::a2dWalker_DetectSmallSegments( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( id, mask )
{
    Initialize();
}

a2dWalker_DetectSmallSegments::a2dWalker_DetectSmallSegments( const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( idList, mask )
{
    Initialize();
}

a2dWalker_DetectSmallSegments::~a2dWalker_DetectSmallSegments()
{
}

void a2dWalker_DetectSmallSegments::Initialize()
{
    m_asString = false;
    /*
    #if wxUSE_STD_IOSTREAM
        m_strstream << _T("Contents of document \n");
    #else
        wxTextOutputStream text_stream( m_strstream );
        text_stream << _T("Contents of document \n");
    #endif
    */
}

bool a2dWalker_DetectSmallSegments::Start( a2dCanvasObject* object )
{
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
    setflags.Start( object, false );

    m_CVGwriter.WriteCvgStartDocument( m_strstream );

    m_CVGwriter.WriteStartElementAttributes( wxT( "o" ) );
    m_CVGwriter.WriteAttribute( wxT( "classname" ), wxT( "a2dCanvasObject" ) );
    m_CVGwriter.WriteEndAttributes();

    object->Walker( NULL, *this );

    m_CVGwriter.WriteEndElement();

    m_CVGwriter.WriteCvgEndDocument();

    return m_result;
}

bool a2dWalker_DetectSmallSegments::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( m_currentDepth > m_depth  )
        SetStopWalking( true );
    else
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* parentobj =  wxDynamicCast( parent, a2dCanvasObject );
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && parentobj && !obj->GetIgnoreLayer() && !obj->GetBin() && ObjectOke( obj ) && ClassOfObjectOke( obj ) )
        {
            bool hasLayer = m_groupA.InGroup( obj->GetLayer() );
            if ( hasLayer && ( !m_selectedOnlyA || ( m_selectedOnlyA && obj->GetSelected() ) ) )
            {
                a2dVertexListPtr redun;
                if ( 0 != wxDynamicCast( obj, a2dPolylineL ) )
                {
                    a2dPolylineL* cobj = wxDynamicCast( obj, a2dPolylineL );
                    redun = cobj->GetSegments()->GetRedundant( false, m_small );
                    if ( redun )
                    {
                        m_result = true;
                        a2dSmrtPtr<a2dPolylineL> p = new a2dPolylineL( redun, false );
                        p->SetBin( true );
                        if ( m_asString )
                        {
#if wxART2D_USE_CVGIO
                            m_CVGwriter.WriteObject( p );
#endif //wxART2D_USE_CVGIO
                        }
                        else
                            parentobj->Append( p );
                        p->SetLayer( m_target );
                    }
                }
                else if ( 0 != wxDynamicCast( obj, a2dPolygonL ) )
                {
                    a2dPolygonL* cobj = wxDynamicCast( obj, a2dPolygonL );
                    redun = cobj->GetSegments()->GetRedundant( true, m_small );
                    if ( redun )
                    {
                        m_result = true;
                        a2dSmrtPtr<a2dPolygonL> p = new a2dPolygonL( redun, false );
                        p->SetBin( true );
                        if ( m_asString )
                        {
#if wxART2D_USE_CVGIO
                            m_CVGwriter.WriteObject( p );
#endif //wxART2D_USE_CVGIO
                        }
                        else
                            parentobj->Append( p );
                        p->SetLayer( m_target );
                    }
                }
            }
        }
    }

    if (  m_currentDepth <= m_depth   )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalker_FindAndSetCorridorPath
//----------------------------------------------------------------------------

a2dWalker_FindAndSetCorridorPath::a2dWalker_FindAndSetCorridorPath( a2dCanvasObject* search )
    : a2dWalker_FindCanvasObject( search )
{
}


a2dWalker_FindAndSetCorridorPath::~a2dWalker_FindAndSetCorridorPath()
{
}

bool a2dWalker_FindAndSetCorridorPath::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dPropertyStart )
        SetStopWalking( true );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( !m_result )
            obj->SetIsOnCorridorPath( true );
        if ( obj == m_search )
        {
            m_result = true;
            SetStopWalking( true );
            return true;
        }
    }

    if ( event == a2dWalker_a2dCanvasObjectEnd )
    {
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( !m_result )
        {
            obj->SetIsOnCorridorPath( false );
            return true;
        }
    }
    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    if (  !m_result  && event == a2dWalker_a2dPropertyEnd )
        SetStopWalking( false );


    return true;
}

/* algorithm candidates, which or not in use currentl

    //! Mark multiple references.
    /! Search for objects that are multiple referenced from within the document.
        All such object get there m_bin2 flag set.
    /
virtual bool MarkMultiReferences();


bool a2dCanvasObject::MarkMultiReferences()
{
    bool found = true;

    if ( GetBin() )
    {
        m_flags.m_bin2 = true;
        found = true;
    }
    m_flags.m_bin = true; //passed here

    if ( canobj->GetChildObjectList() == wxNullCanvasObjectList )
        return found;

    forEachIn( a2dCanvasObjectList, canobj->GetChildObjectList() )
    {
        a2dCanvasObject *obj = *iter;
        found = obj->MarkMultiReferences() && found;
    }
    return found;
}








    //!can be used by the user to implement a function that affects all a2dCanvas  derived objects
    /!
      The base implementation calls this function on its childs, so in a derived object always
      call the base class to keep recursing deeper.

       \param function id of the user function to be called
       \param data for the called function (type depends on id)
       \param depth recursion depth (0=this object, 1=this object+childs, ...)
       \param flags objects with this flag set ignore the depth limit
    /
    virtual bool UserBaseFunctionEx( int function, void *data, int depth, a2dCanvasObjectFlagsMask flags );



bool a2dCanvasObject::UserBaseFunctionEx( int function, void *data, int depth,  a2dCanvasObjectFlagsMask flags )
{
    if( depth<=0 && !CheckMask( flags ) )
        return true;

    bool rslt = true;
    if( GetChildObjectsCount() )
    {
        forEachIn( a2dCanvasObjectList, canobj->GetChildObjectList() )
        {
            a2dCanvasObject *obj = *iter;
            rslt &= obj->UserBaseFunctionEx( function, data, depth-1, flags );
        }
    }
    return rslt;
}




    //! to refresh controls after a rendering to a device did take place
    /!
        This brings the wxWindows controls back to the front.
    /
    void RefreshControls();


    void a2dCanvasObject::RefreshControls()
    {
        if ( canobj->GetChildObjectList() == wxNullCanvasObjectList )
            return;

        forEachIn( a2dCanvasObjectList, canobj->GetChildObjectList() )
        {
            a2dCanvasObject *obj = *iter;
            obj->RefreshControls();
        }
    }
*/


//----------------------------------------------------------------------------
// a2dWalker_AllowedObjects
//----------------------------------------------------------------------------

a2dWalker_AllowedObjects::a2dWalker_AllowedObjects( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( mask )
{
    Initialize();
}

a2dWalker_AllowedObjects::a2dWalker_AllowedObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( id, mask )
{
    Initialize();
}

a2dWalker_AllowedObjects::a2dWalker_AllowedObjects( const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( idList, mask )
{
    Initialize();
}

a2dWalker_AllowedObjects::~a2dWalker_AllowedObjects()
{
}

void a2dWalker_AllowedObjects::Initialize()
{
    m_asString = false;

    /*
    #if wxUSE_STD_IOSTREAM
        m_strstream << _T("Contents of document \n");
    #else
        wxTextOutputStream text_stream( m_strstream );
        text_stream << _T("Contents of document \n");
    #endif
    */
}

bool a2dWalker_AllowedObjects::Start( a2dCanvasObject* object )
{
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
    setflags.Start( object, false );

    m_CVGwriter.WriteCvgStartDocument( m_strstream );

    m_CVGwriter.WriteStartElementAttributes( wxT( "o" ) );
    m_CVGwriter.WriteAttribute( wxT( "classname" ), wxT( "a2dCanvasObject" ) );
    m_CVGwriter.WriteEndAttributes();

    object->Walker( NULL, *this );

    m_CVGwriter.WriteEndElement();

    m_CVGwriter.WriteCvgEndDocument();

    return m_result;
}

bool a2dWalker_AllowedObjects::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( m_currentDepth > m_depth  )
        SetStopWalking( true );
    else
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* parentobj =  wxDynamicCast( parent, a2dCanvasObject );
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && parentobj && !obj->GetIgnoreLayer() && !obj->GetBin() && ObjectOke( obj ) )
        {
            bool hasLayer = m_groupA.InGroup( obj->GetLayer() );
            if ( hasLayer && ( !m_selectedOnlyA || ( m_selectedOnlyA && obj->GetSelected() ) ) )
            {
                // see also m_allowClassList
                if ( ClassOfObjectOke( obj ) )
                {
                    m_result = true;
                    if ( m_store )
                    {
                        if ( m_asString )
                        {
#if wxART2D_USE_CVGIO
                            m_CVGwriter.WriteObject( obj );
#endif //wxART2D_USE_CVGIO
                        }
                        else
                        {
                            a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                            cobj->SetBin( true );
                            parentobj->Prepend( cobj );
                            cobj->SetLayer( m_target );
                        }
                    }
                }
            }
        }
    }

    if (  m_currentDepth <= m_depth   )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

//----------------------------------------------------------------------------
// a2dWalkerIOHandlerWithContext
//----------------------------------------------------------------------------

a2dWalkerIOHandlerWithContext::a2dWalkerIOHandlerWithContext( a2dIterC& ic )
    : a2dWalkerIOHandler()
{
    m_context = &ic;
    Initialize();
}

a2dWalkerIOHandlerWithContext::~a2dWalkerIOHandlerWithContext()
{
}

void a2dWalkerIOHandlerWithContext::Initialize()
{
}

//! Start traversing at object, returns true.
bool a2dWalkerIOHandlerWithContext::Start( a2dCanvasObject* object )
{
    a2dWalker_SetCheck setp( false );
    setp.SetSkipNotRenderedInDrawing( true );
    setp.Start( object );

    SetDepthFirst( false );
    m_result = true;
    m_context->SetClipStatus( _ON );
    m_context->SetRenderChildDerived( false );
    object->WalkerWithContext( *m_context, NULL, *this );
    return m_result;
}

//----------------------------------------------------------------------------
// a2dWalker_RenderLayers
//----------------------------------------------------------------------------

a2dWalker_RenderLayers::a2dWalker_RenderLayers( a2dIterC& ic )
    : a2dWalkerIOHandlerWithContext( ic )
{

}

a2dWalker_RenderLayers::~a2dWalker_RenderLayers()
{

}

bool a2dWalker_RenderLayers::WalkTask( a2dIterC& ic, wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* parentobj = ( a2dCanvasObject* ) parent;
        a2dCanvasObject* canobj = ( a2dCanvasObject* ) object;
        OVERLAP clipparent = ic.GetParentClipStatus();
        OVERLAP childclip = _IN;
        // if parent is checked it means, it is _ON or _IN
        childclip = canobj->GetClipStatus( ic, clipparent );
        ic.SetClipStatus( childclip );
        //if ( childclip != _OUT ) //if a child is _OUT, no need to render it.
        {
            Render( ic, canobj, childclip );
        }
    }
    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false;

    return true;
}

void a2dWalker_RenderLayers::Render( a2dIterC& ic, a2dCanvasObject* canobj, OVERLAP clipparent )
{
    wxUint16 layer = canobj->GetLayer();
    // Check if the object is not about to be deleted
    if ( canobj->GetRelease( ) )
        return;

    if ( !ic.FilterObject( canobj ) )
    {
        ic.EndFilterObject( canobj );
        return;
    }

    //Now we will either render the object or children or both on the given layer.
    //OR in case layer == wxLAYER_ALL, everything will be rendered at once.
    //Style of objects will be according to their own style properties of using the object
    //m_layer id.


    //get bbox at the absolute position
    //first map without pen width which is not included in the boundingbox

    // if 0 in width or height, nothing will be rendered in principle.
    // But the stroke width is not included in bbox, so it might still be visible.
    if ( !canobj->GetFlag( a2dCanvasOFlags::NoRenderCanvasObject ) && canobj->GetBbox().GetWidth() && canobj->GetBbox().GetHeight() )
    {
        a2dBoundingBox absarea = canobj->GetBbox();
        if ( !ic.GetTransform().IsIdentity() )
            absarea.MapBbox( ic.GetTransform() );

        double size = wxMax( absarea.GetWidth(), absarea.GetHeight() );
        if ( ic.GetDrawer2D()->GetPrimitiveThreshold() != 0
                && size < ic.GetDrawer2D()->DeviceToWorldXRel( ic.GetDrawer2D()->GetPrimitiveThreshold() )  )
        {
            if ( ic.GetDrawer2D()->GetThresholdDrawRectangle() )
            {
                wxRect absareadev = canobj->GetAbsoluteArea( ic, 0 );
                ic.GetDrawer2D()->DrawRoundedRectangle( canobj->GetBbox().GetMinX(), canobj->GetBbox().GetMinY(),
                                                        canobj->GetBbox().GetWidth(), canobj->GetBbox().GetHeight(), 0 );

                if ( canobj->GetHighLight() )
                    canobj->DrawHighLighted( ic );
            }
            ic.EndFilterObject( canobj );
            return;
        }
    }

    // a2dIterCU scope ends for DrawHighLighted( ic )
    {
        //here the Drawer gets a new relative transform
        //Every call for drawing something on it, will use it from now on.
        // we go one level deeper in transform to the child level
        a2dIterCU cu( ic, canobj );

        //if ( m_flags.m_HighLight )
        //    DrawHighLighted( ic );


        //if the propertylist NOT contains style properties,
        //we need to set the style according to the layer setting.
        bool fillset = false;
        bool strokeset = false;
        bool viewSpecific = false;

        a2dLayers* layers = canobj->GetRoot()->GetLayerSetup();
        //if style not set by the properties ,do it now using the layers.
        if ( layers )
        {
            if ( !fillset  )
                ic.GetDrawer2D()->SetDrawerFill( layers->GetFill( layer ) );
            if ( !strokeset )
                ic.GetDrawer2D()->SetDrawerStroke( layers->GetStroke( layer ) );
        }
        else
        {
            if ( !fillset  )
                ic.GetDrawer2D()->SetDrawerFill( *a2dBLACK_FILL );
            if ( !strokeset )
                ic.GetDrawer2D()->SetDrawerStroke( *a2dBLACK_STROKE );
        }

        //if the object has sub objects (apart from the childobjects which are handled here),
        //those subobjects must be rendered by iterating on layer when needed/wanted, simular to child objects.
        //We do not iterate here, since that is only needed if indeed there or subobjects.
        //This will be know in a "wxDerivedCanvasObject DoRender".
        //SO parent objects that call this function here, MUST:
        // 1-  clip object against area to redraw.
        // 2-  iterate on layers when needed.
        if ( ic.GetLayer() == layer || ic.GetLayer() == wxLAYER_ALL || canobj->GetIgnoreLayer() )
        {

            // DoRender() should not change the matrix of the drawing context, or at least restore it.
            // To prevent taking time for this.
            //a2dAffineMatrix tworld = ic.GetTransform();

            canobj->CallDoRender( ic, clipparent );
        }
    }

    if ( canobj->GetHighLight() )
        canobj->DrawHighLighted( ic );

    ic.EndFilterObject( canobj );
}


//----------------------------------------------------------------------------
// a2dWalker_FindPinsToConnect
//----------------------------------------------------------------------------

a2dWalker_FindPinsToConnect::a2dWalker_FindPinsToConnect( a2dCanvasObjectFlagsMask mask, a2dPinClass* pinclassToConnectTo,
        a2dConnectTask connectTask, double xpin, double ypin, double margin )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();

    m_mask = mask;
    m_pinclassToConnectTo = pinclassToConnectTo;
    m_xpin = xpin;
    m_ypin = ypin;
    m_margin = margin;
    m_connectTask = connectTask;
    m_stopAtFirst = false;
}

a2dWalker_FindPinsToConnect::~a2dWalker_FindPinsToConnect()
{
}

void a2dWalker_FindPinsToConnect::Initialize()
{
    m_found.clear();
}

bool a2dWalker_FindPinsToConnect::Start( a2dCanvasObject* object )
{
    if( object->GetChildObjectsCount() )
    {
        object->GetChildObjectList()->SetSpecificFlags( false, a2dCanvasOFlags::BIN );

        a2dCanvasObjectList::iterator iter;

        for( iter = object->GetChildObjectList()->begin(); iter != object->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && !obj->GetAlgoSkip() && obj->DoConnect() && ObjectOke( obj ) && ClassOfObjectOke( obj ) && obj->HasPins() )
            {
                a2dCanvasObjectList::iterator iterpins; //iterate for pins
                for( iterpins = obj->GetChildObjectList()->begin(); iterpins != obj->GetChildObjectList()->end(); ++iterpins )
                {
                    a2dCanvasObject* child = *iterpins;
                    a2dPin* pin = wxDynamicCast( child, a2dPin );
                    //if parent object has the right mask, the pin may be different (e.g. not visible), we take this into account.
                    //This is to be able to disable pins.
                    if ( pin && !pin->GetRelease() && pin->CheckMask( m_mask ) )
                    {
                        a2dPinClass* pinclass = pin->GetPinClass();
                        //check if m_pinclassToConnectTo combines with this pin, if so set connectable.
                        if ( 1 )
                        {
                            double dx = fabs( pin->GetAbsX() - m_xpin );
                            double dy = fabs( pin->GetAbsY() - m_ypin );
                            if ( pin->GetMode() != a2dPin::sm_PinUnConnected )
                                pin->SetMode( a2dPin::sm_PinUnConnected );
                            if ( dx < m_margin  && dy < m_margin  //&&
                                    //pinclass->CanConnectTo( m_pinclassToConnectTo ) &&
                                    //m_pinclassToConnectTo->CanConnectTo( pinclass )
                               )
                            {
                                //we have to pins of different objects on top of eachother, and they maybe can connect
								if ( m_connectTask == a2d_StartWire )
								{
									// wire start on a pin with pinclass, ask generator for pinclass of wire.
                                    if ( m_returnPinclass = pinclass->GetPinClassForTask( m_connectTask, obj, m_pinclassToConnectTo, pin ) )
                                    {
                                        m_found.push_back( pin );
	                                    pin->SetMode( a2dPin::sm_PinCanConnectToPinClass );
                                    }
									else
										pin->SetMode( a2dPin::sm_PinCannotConnect );
								}
                                else if ( m_connectTask == a2d_StartWire_BasedOnClassStartPin )
								{
                                    if ( m_returnPinclass = pinclass->GetPinClassForTask( m_connectTask, obj ) )
                                        m_found.push_back( pin );
                                }
                                else if ( m_connectTask == a2d_StartWire_BasedOnWireClassRequired )
								{
                                    if ( m_returnPinclass = pinclass->GetPinClassForTask( m_connectTask, obj ) )
                                        m_found.push_back( pin );
                                }
                                else if ( m_connectTask == a2d_StartWire_BasedOnObjectClassRequired )
								{
                                    if ( m_returnPinclass = pinclass->GetPinClassForTask( m_connectTask, obj ) )
                                        m_found.push_back( pin );
                                }
                                else if ( m_connectTask == a2d_SearchPinForFinishWire )
								{
                                    if ( m_pinclassToConnectTo == m_pinclassToConnectTo->GetPinClassForTask( m_connectTask, obj, pinclass ) )
                                    {
                                        m_found.push_back( pin );
	                                    pin->SetMode( a2dPin::sm_PinCanConnectToPinClass );
                                    }
									else
										pin->SetMode( a2dPin::sm_PinCannotConnect );
								}
/*
                                else if ( m_connectTask == a2d_GeneratePinsForPinClass &&
                                          pinclass == pinclass->GetConnectionGenerator()->GetPinClassForTask( m_pinclassToConnectTo, m_connectTask, obj )
                                        )
                                {
                                    m_found.push_back( pin );
                                    pin->SetMode( a2dPin::sm_PinCanConnectToPinClass );
                                }
                                else if ( m_connectTask == a2d_GeneratePinsForPinClass &&
                                          m_pinclassToConnectTo == pinclass->GetConnectionGenerator()->GetPinClassForTask( pinclass, m_connectTask, obj )
                                        )
                                {
                                    m_found.push_back( pin );
                                    pin->SetMode( a2dPin::sm_PinCanConnectToPinClass );
                                }
*/
                                else
                                    pin->SetMode( a2dPin::sm_PinCannotConnect );

                                if ( m_found.size() )
                                {
                                    m_result = true;
                                    if ( m_stopAtFirst )
                                        return m_result;
                                }
                            }
                        }
                        else
                        {
                            if ( pin->GetMode() != a2dPin::sm_PinConnected )
                                pin->SetMode( a2dPin::sm_PinConnected );
                        }
                    }
                }
            }
        }
    }

    return m_result;
}

//----------------------------------------------------------------------------
// a2dWalker_FindPinsToConnectToPin
//----------------------------------------------------------------------------

a2dWalker_FindPinsToConnectToPin::a2dWalker_FindPinsToConnectToPin( a2dCanvasObjectFlagsMask mask, a2dPin* pinToConnectTo,
        a2dConnectTask connectTask, double margin )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();

    m_mask = mask;
    m_pinToConnectTo = pinToConnectTo;
    m_margin = margin;
    m_connectTask = connectTask;
    m_stopAtFirst = false;
}

a2dWalker_FindPinsToConnectToPin::~a2dWalker_FindPinsToConnectToPin()
{
}

void a2dWalker_FindPinsToConnectToPin::Initialize()
{
    m_found.clear();
}

bool a2dWalker_FindPinsToConnectToPin::Start( a2dCanvasObject* object )
{
    if( object->GetChildObjectsCount() )
    {
        object->GetChildObjectList()->SetSpecificFlags( false, a2dCanvasOFlags::BIN );

        a2dCanvasObjectList::iterator iter;

        for( iter = object->GetChildObjectList()->begin(); iter != object->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && !obj->GetAlgoSkip() && obj->HasPins() && obj->DoConnect() && ObjectOke( obj ) && ClassOfObjectOke( obj ) )
            {
                a2dCanvasObjectList::iterator iterpins; //iterate for pins
                for( iterpins = obj->GetChildObjectList()->begin(); iterpins != obj->GetChildObjectList()->end(); ++iterpins )
                {
                    a2dCanvasObject* child = *iterpins;
                    a2dPin* pin = wxDynamicCast( child, a2dPin );
                    if ( pin && !pin->GetRelease() && pin->CheckMask( m_mask ) )
                    {
                        a2dPinClass* pinclass = pin->GetPinClass();
                        //check if m_pinclassToConnectTo combines with this pin, if so set connectable.
                        if ( 1 )
                        {
                            double dx = fabs( pin->GetAbsX() - m_pinToConnectTo->GetAbsX() );
                            double dy = fabs( pin->GetAbsY() - m_pinToConnectTo->GetAbsY() );
                            if ( pin->GetMode() != a2dPin::sm_PinUnConnected )
                                pin->SetMode( a2dPin::sm_PinUnConnected );
                            if ( dx < m_margin  && dy < m_margin  //&&
                                    //pinclass->CanConnectTo( m_pinclassToConnectTo ) &&
                                    //m_pinclassToConnectTo->CanConnectTo( pinclass )
                               )
                            {
                                //we have to pins of different objects on top of eachother, and they maybe can connect
								if ( m_connectTask == a2d_PinToPin )
								{
                                    if ( m_pinToConnectTo->GetPinClass() == pinclass->GetPinClassForTask( m_connectTask, obj, m_pinToConnectTo->GetPinClass(), m_pinToConnectTo ) )
                                    {
                                        m_found.push_back( pin );
	                                    pin->SetMode( a2dPin::sm_PinCanConnectToPinClass );
                                    }
									else
										pin->SetMode( a2dPin::sm_PinCannotConnect );
								}
								else if ( m_connectTask == a2d_FinishWire )
								{
                                    if ( m_pinToConnectTo->GetPinClass() == pinclass->GetPinClassForTask( m_connectTask, obj, m_pinToConnectTo->GetPinClass(), m_pinToConnectTo ) )
                                    {
                                        m_found.push_back( pin );
	                                    pin->SetMode( a2dPin::sm_PinCanConnectToPinClass );
                                    }
									else
										pin->SetMode( a2dPin::sm_PinCannotConnect );
								}
                                if ( m_found.size() )
                                {
                                    m_result = true;
                                    if ( m_stopAtFirst )
                                        return m_result;
                                }                            }
                        }
                        else
                        {
                            if ( pin->GetMode() != a2dPin::sm_PinConnected )
                                pin->SetMode( a2dPin::sm_PinConnected );
                        }
                    }
                }
                m_result = true;
            }
        }
    }

    return m_result;
}

//----------------------------------------------------------------------------
// a2dWalker_GeneratePinsToConnect
//----------------------------------------------------------------------------

a2dWalker_GeneratePinsToConnect::a2dWalker_GeneratePinsToConnect( a2dCanvasObjectFlagsMask mask, a2dPinClass* pinclassToConnectTo,
        a2dConnectTask connectTask, double xpin, double ypin )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();

    m_mask = mask;
    m_pinclassToConnectTo = pinclassToConnectTo;
    m_xpin = xpin;
    m_ypin = ypin;
    m_connectTask = connectTask;
}

a2dWalker_GeneratePinsToConnect::~a2dWalker_GeneratePinsToConnect()
{
}

void a2dWalker_GeneratePinsToConnect::Initialize()
{
}

bool a2dWalker_GeneratePinsToConnect::Start( a2dIterC& ic, a2dCanvasObject* object, bool ItsChildren )
{
    m_ic = &ic;
    double margin = m_ic->GetHitMarginWorld();

    if ( ItsChildren )
    {
        a2dIterCU cu( ic, object );
        if( object->GetChildObjectsCount() )
        {
            a2dCanvasObjectList::iterator iter;

            //the next will create dynamic pins when possible/wanted for required pinclass.
            //Those pins are specific to a x,y location.
            for( iter = object->GetChildObjectList()->begin(); iter != object->GetChildObjectList()->end(); ++iter )
            {
                a2dCanvasObject* obj = *iter;
                
                if ( obj && !obj->GetAlgoSkip() && obj->GetGeneratePins() && 
                     obj->DoConnect() && ObjectOke( obj ) && ClassOfObjectOke( obj ) )
                {
                    obj->RemovePins( true, true );
                    // First test if canvas object is a hit, so TEMPORARY pins will only be created when hit on object.
                    a2dHitEvent hitevent = a2dHitEvent( m_xpin, m_ypin );
                    if( obj->IsHitWorld( *m_ic, hitevent ) )
                        obj->GeneratePinsPossibleConnections( m_pinclassToConnectTo, m_connectTask, m_xpin, m_ypin, margin );

                    m_result = true;
                }
            }

        }
    }
    else
    {
        a2dCanvasObject* obj = object;
        if ( obj && !obj->GetAlgoSkip() && obj->GetGeneratePins() && 
             obj->DoConnect() && ObjectOke( obj ) && ClassOfObjectOke( obj ) )
        {
            obj->RemovePins( true, true );
            // First test if canvas object is a hit, so TEMPORARY pins will only be created when hit on object.
            a2dHitEvent hitevent = a2dHitEvent( m_xpin, m_ypin );
            if( obj->IsHitWorld( *m_ic, hitevent ) )
            {
                obj->GeneratePinsPossibleConnections( m_pinclassToConnectTo, m_connectTask, m_xpin, m_ypin, margin );
            }
            m_result = true;
        }
    }
    return m_result;
}

//----------------------------------------------------------------------------
// a2dWalker_SetPinsToEndState
//----------------------------------------------------------------------------

a2dWalker_SetPinsToEndState::a2dWalker_SetPinsToEndState( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();

    m_mask = mask;
}

a2dWalker_SetPinsToEndState::~a2dWalker_SetPinsToEndState()
{
}

void a2dWalker_SetPinsToEndState::Initialize()
{
}

bool a2dWalker_SetPinsToEndState::Start( a2dCanvasObject* object )
{
    if( object->GetChildObjectsCount() )
    {
        a2dCanvasObjectList::iterator iter;

        for( iter = object->GetChildObjectList()->begin(); iter != object->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && !obj->GetAlgoSkip() && obj->HasPins() && obj->DoConnect() && ObjectOke( obj ) && ClassOfObjectOke( obj ) )
            {
                obj->RemovePins( true, true );

                a2dCanvasObjectList::iterator iterpins; //iterate for pins
                for( iterpins = obj->GetChildObjectList()->begin(); iterpins != obj->GetChildObjectList()->end(); ++iterpins )
                {
                    a2dCanvasObject* child = *iterpins;
                    a2dPin* pin = wxDynamicCast( child, a2dPin );
                    if ( pin && !pin->GetRelease() )
                    {
                        a2dPinClass* pinclass = pin->GetPinClass();
                        //check if m_pinclassToConnectTo combines with this pin, if so set connectable.
                        if ( ! pin->IsConnectedTo() )
                        {
                            if ( pin->GetMode() != a2dPin::sm_PinUnConnected )
                                pin->SetMode( a2dPin::sm_PinUnConnected );
                        }
                        else
                        {
                            if ( pin->GetMode() != a2dPin::sm_PinConnected )
                                pin->SetMode( a2dPin::sm_PinConnected );
                        }
                    }
                }

                m_result = true;
            }
        }

    }

    return m_result;
}

//----------------------------------------------------------------------------
// a2dWalker_SetPinsToBeginState
//----------------------------------------------------------------------------

a2dWalker_SetPinsToBeginState::a2dWalker_SetPinsToBeginState( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();

    m_mask = mask;
}

a2dWalker_SetPinsToBeginState::~a2dWalker_SetPinsToBeginState()
{
}

void a2dWalker_SetPinsToBeginState::Initialize()
{
}

bool a2dWalker_SetPinsToBeginState::Start( a2dCanvasObject* object )
{
    if( object->GetChildObjectsCount() )
    {
        a2dCanvasObjectList::iterator iter;

        for( iter = object->GetChildObjectList()->begin(); iter != object->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && !obj->GetAlgoSkip() && obj->HasPins() && obj->DoConnect() && ObjectOke( obj ) && ClassOfObjectOke( obj ) 
               )
            {
                obj->RemovePins( true, true );

                a2dCanvasObjectList::iterator iterpins; //iterate for pins
                for( iterpins = obj->GetChildObjectList()->begin(); iterpins != obj->GetChildObjectList()->end(); ++iterpins )
                {
                    a2dCanvasObject* child = *iterpins;
                    a2dPin* pin = wxDynamicCast( child, a2dPin );
                    if ( pin && !pin->GetRelease() )
                    {
                        a2dPinClass* pinclass = pin->GetPinClass();
                        //check if m_pinclassToConnectTo combines with this pin, if so set connectable.
                        if ( ! pin->IsConnectedTo() )
                        {
                            if ( pin->GetMode() != a2dPin::sm_PinUnConnected )
                                pin->SetMode( a2dPin::sm_PinUnConnected );
                        }
                        else
                        {
                            if ( pin->GetMode() != a2dPin::sm_PinConnected )
                                pin->SetMode( a2dPin::sm_PinConnected );
                        }
                    }
                }

                m_result = true;
            }
        }

    }

    return m_result;
}



//----------------------------------------------------------------------------
// a2dWalker_GeneratePinsToConnectObject
//----------------------------------------------------------------------------

a2dWalker_GeneratePinsToConnectObject::a2dWalker_GeneratePinsToConnectObject( a2dCanvasObjectFlagsMask mask, a2dCanvasObject* connectObject,
        a2dConnectTask connectTask )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();

    m_mask = mask;
    m_connectTask = connectTask;
    m_connectObject = connectObject;
}

a2dWalker_GeneratePinsToConnectObject::~a2dWalker_GeneratePinsToConnectObject()
{
}

void a2dWalker_GeneratePinsToConnectObject::Initialize()
{
}

bool a2dWalker_GeneratePinsToConnectObject::Start( a2dIterC& ic, a2dCanvasObject* top )
{
    m_ic = &ic;
    a2dIterCU cu( ic, top );

    if( !top->GetChildObjectsCount() )
        return false;

    a2dCanvasObjectList::iterator iter;

    bool directChild = false;
    for( iter = top->GetChildObjectList()->begin(); iter != top->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj )
        {
            if ( m_connectObject == obj )
                directChild = true;
            //obj->RemovePins( true, true );
        }
    }
    if ( !directChild )
        return false;

    if( m_connectObject->GetChildObjectsCount() &&
            m_connectObject->GetRoot() &&
            m_connectObject->HasPins() &&
            !m_connectObject->IsConnect() )
    {
        //the next part is to create dynamic pins, which are not a specific x,y but
        //which are on the same pins as m_connectObject. In other words: if another pin'ned objects
        //can deliver a pin at this objects its pins, set bin flag true.

        // are we on an object that is not itself wire like (IsConnect() ), and there are pins,
        // find if those pins can connect to others in object that have pins at the same location.

        // It depends on DoConnect() if the pins can be connected later on in ReWireConnected.

        a2dCanvasObjectList::iterator iter;

        //set all pins on m_connectObject object to bin flag false,
        //this to detect the pins which are able to connect.
        for( iter = m_connectObject->GetChildObjectList()->begin(); iter != m_connectObject->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pinc = wxDynamicCast( obj, a2dPin );
            if ( pinc && !pinc->GetRelease() )
            {
                pinc->SetBin( false );
                if ( ! pinc->IsConnectedTo() )
                    pinc->SetMode( a2dPin::sm_PinUnConnected );
                else
                    pinc->SetMode( a2dPin::sm_PinConnected );
            }
        }

        m_bbox = m_connectObject->GetBbox();
        m_intersectionCondition = _ON;
        
        //flash if hit on pin of other object after modification
        a2dCanvasObjectList allpinobjects;
        for( iter = top->GetChildObjectList()->begin(); iter != top->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( m_bbox.GetValid() && ( m_bbox.Intersect( obj->GetBbox() ) & m_intersectionCondition ) != 0 )
            {
                if ( obj->HasPins() || obj->GetGeneratePins() )
                    allpinobjects.push_back( obj );
            }    
        }
		//wxLogDebug( "How Many did intersect: %d",allpinobjects.size() );

        // ONLY object with pins or where pins will be generated upon in allpinobjects
        for( iter = allpinobjects.begin(); iter != allpinobjects.end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            // if the other object is not a wire like objects, and allows connection,
            // we should generate feedback to the pins, by changing its render state.
            if (
                obj && obj != m_connectObject && !obj->IsConnect() && obj->DoConnect() &&
                !obj->GetAlgoSkip() && ObjectOke( obj ) && ClassOfObjectOke( obj ) 
            )
            {
                bool autocreatepins = obj->GetGeneratePins();
                if ( m_connectObject->GetEditingRender( ) )
                {
                    //m_connectObject object is in editing mode, and is a copy of original.
                    //We don't want to flash connection with that one.
                    a2dCanvasObject* original = a2dCanvasObject::PROPID_Original->GetPropertyValue( m_connectObject );
                    if ( original != obj )
                    {
                        //test connection of this object with the other is possible.
                        //All pins which can connect will have the bin flag set
                        m_connectObject->CanConnectWith( ic, obj, autocreatepins );
                    }
                }
                else
                {
                    //test connection of this object with the other is possible.
                    //All pins which can connect will have the bin flag set
                    m_connectObject->CanConnectWith( ic, obj, autocreatepins );
                }
            }
        }

        //all pins on m_connectObject object which are able to connect do have the bin flag set
        for( iter = m_connectObject->GetChildObjectList()->begin(); iter != m_connectObject->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pinc = wxDynamicCast( obj, a2dPin );
            if ( pinc && !pinc->GetRelease() && pinc->GetBin() )
            {
                if( !pinc->IsConnectedTo() )
                {
                    pinc->SetMode( a2dPin::sm_PinCanConnect );
                }
                pinc->SetBin( false );
            }
        }
    }

    return m_result;
}


//----------------------------------------------------------------------------
// a2dWalker_ConnectToPinsObject
//----------------------------------------------------------------------------

a2dWalker_ConnectToPinsObject::a2dWalker_ConnectToPinsObject( a2dCanvasObjectFlagsMask mask, a2dCanvasObject* connectObject,
        a2dConnectTask connectTask, double margin )
    : a2dWalker_FilterCanvasObjects( mask )
{
    Initialize();

    m_mask = mask;
    m_connectTask = connectTask;
    m_connectObject = connectObject;
    m_margin = margin;
}

a2dWalker_ConnectToPinsObject::~a2dWalker_ConnectToPinsObject()
{
}

void a2dWalker_ConnectToPinsObject::Initialize()
{
}

bool a2dWalker_ConnectToPinsObject::Start( a2dCanvasObject* top, bool alsoWires )
{
    if( !top->GetChildObjectsCount() )
        return false;

    a2dCanvasObjectList::iterator iter;

    bool directChild = false;
    for( iter = top->GetChildObjectList()->begin(); iter != top->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj )
        {
            if ( m_connectObject == obj )
                directChild = true;
            //obj->RemovePins( true, true );
        }
    }
    if ( !directChild )
        return false;

    if( m_connectObject->GetChildObjectsCount() &&
            m_connectObject->GetRoot() &&
            m_connectObject->HasPins() &&
            ( alsoWires || !m_connectObject->IsConnect() ) )
    {
        a2dCanvasObjectList::iterator iter; //iterate for pins
        for( iter = m_connectObject->GetChildObjectList()->begin(); iter != m_connectObject->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* child = *iter;
            a2dPin* pin = wxDynamicCast( child, a2dPin );
            if ( pin && !pin->GetRelease() && pin->CheckMask( m_mask ) )
            {
                ConnectAtPin( top, pin );
            }
        }
    }

    return m_result;
}

bool a2dWalker_ConnectToPinsObject::ConnectAtPin( a2dCanvasObject* top, a2dPin* pintoconnect )
{
    a2dCanvasObjectList::iterator iter;

    for( iter = top->GetChildObjectList()->begin(); iter != top->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj && !obj->GetAlgoSkip() && obj->DoConnect() && ObjectOke( obj ) && ClassOfObjectOke( obj ) )
        {
            a2dCanvasObjectList::iterator iterpins; //iterate for pins
            for( iterpins = obj->GetChildObjectList()->begin(); iterpins != obj->GetChildObjectList()->end(); ++iterpins )
            {
                a2dCanvasObject* child = *iterpins;
                a2dPin* pin = wxDynamicCast( child, a2dPin );
                if ( pin && !pin->GetRelease() && pin->CheckMask( m_mask ) &&
                     pin != pintoconnect //in case of pins on same object as pin, only allow connecting to other pins
                   )
                {
                    a2dPinClass* pinclass = pin->GetPinClass();
                    if ( ! pin->IsConnectedTo( pintoconnect ) )
                    {
                        if ( pin->IsSameLocation( pintoconnect, m_margin ) &&
                             pin->MayConnectTo( pintoconnect )
                           )
                        {
                            pin->SetMode( a2dPin::sm_PinConnected );
                            // check if can connect connect the pins
                            // pins did snap, but might not be at the exact same position
                            // therefore create with wire.
                            // multidrag tool does not create wires for pin to pin connections, it needs a wire
                            // m_connectObject->ConnectPinsCreateConnect( top, pintoconnect, pin, true );
                            // therefore use this until solved, since it always creates a wire.

                            if ( pin->IsSameLocation( pintoconnect ) )
                                m_connectObject->ConnectPins( top, pintoconnect, pin, true );                            
                            else
                                m_connectObject->CreateConnectObject( top, pintoconnect, pin, true );

                        }
                    }
                }
            }
            m_result = true;
        }
    }
    return m_result;
}

//----------------------------------------------------------------------------
// a2dWalker_ConnectEvent
//----------------------------------------------------------------------------

a2dWalker_ConnectEvent::a2dWalker_ConnectEvent( a2dObject* reportTo, wxEventType eventType, bool connect )
    : a2dWalkerIOHandler()
{
    Initialize();
    m_reportTo = reportTo;
    m_eventType = eventType;
    m_connect = connect;
    m_classnameMap.clear();
}

a2dWalker_ConnectEvent::~a2dWalker_ConnectEvent()
{
}

void a2dWalker_ConnectEvent::AddClassName( wxString className )
{
    if ( m_classnameMap.find( className ) == m_classnameMap.end() )
        m_classnameMap.insert( className );
}

void a2dWalker_ConnectEvent::RemoveClassName( wxString className )
{
    if ( m_classnameMap.find( className ) != m_classnameMap.end() )
        m_classnameMap.erase( m_classnameMap.find( className ) );
}

void a2dWalker_ConnectEvent::AddClassInfo( wxClassInfo* classInfo )
{
    if ( m_classInfoMap.find( classInfo ) == m_classInfoMap.end() )
        m_classInfoMap.insert( classInfo );
}

void a2dWalker_ConnectEvent::RemoveClassInfo( wxClassInfo* classInfo )
{
    if ( m_classInfoMap.find( classInfo ) == m_classInfoMap.end() )
        m_classInfoMap.erase( m_classInfoMap.find( classInfo ) );
}

bool a2dWalker_ConnectEvent::ClassOfObjectOke( a2dObject* obj )
{
    if ( ! m_classnameMap.empty() )
    {
        // if class is NOT in list, that is a non wanted object
        if ( m_classnameMap.find( obj->GetClassInfo()->GetClassName() ) != m_classnameMap.end() )
            return true;
    }
    if ( ! m_classInfoMap.empty() )
    {
        set<wxClassInfo*>::iterator iter = m_classInfoMap.begin();
        while( iter != m_classInfoMap.end() )
        {
            wxClassInfo* info = *iter;
            if ( obj->GetClassInfo()->IsKindOf( info ) )
                return true;
            iter++;
        }
    }
    return false;
}

void a2dWalker_ConnectEvent::Initialize()
{
    m_reportTo = 0;
    m_connect = true;
}

bool a2dWalker_ConnectEvent::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dObject* a2dobj = wxDynamicCast( object, a2dObject );
        if ( ClassOfObjectOke( a2dobj ) )
        {
            if ( m_connect )
                a2dobj->ConnectEvent( m_eventType, m_reportTo );
            else
                a2dobj->DisconnectEvent( m_eventType, m_reportTo );
        }
    }

    if ( event == a2dWalker_a2dDrawingStart )
    {
        a2dObject* a2dobj = wxDynamicCast( object, a2dObject );
        if ( ClassOfObjectOke( a2dobj ) )
        {
            if ( m_connect )
                a2dobj->ConnectEvent( m_eventType, m_reportTo );
            else
                a2dobj->DisconnectEvent( m_eventType, m_reportTo );
        }
    }

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

void a2dWalker_ConnectEvent::Start( a2dObject* object )
{
    object->Walker( NULL, *this );
}

//----------------------------------------------------------------------------
// a2dWalker_FindCameleonInst
//----------------------------------------------------------------------------

a2dWalker_FindCameleonInst::a2dWalker_FindCameleonInst( a2dCameleon* cameleon )
    : a2dWalkerIOHandler()
{
    Initialize();
    m_cameleon = cameleon;
}

a2dWalker_FindCameleonInst::~a2dWalker_FindCameleonInst()
{
}

void a2dWalker_FindCameleonInst::Initialize()
{
    m_found = false;
}

bool a2dWalker_FindCameleonInst::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCameleonInst* camInst = wxDynamicCast( object, a2dCameleonInst );
        if ( camInst && camInst->GetCameleon() == m_cameleon )
            m_found = true;
    }

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

void a2dWalker_FindCameleonInst::Start( a2dObject* object )
{
    object->Walker( NULL, *this );
}
