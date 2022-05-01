/*! \file wx/canvas/algos.h
    \brief a2dWalker based algorithms

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: algos.h,v 1.2 2009/09/03 20:09:53 titato Exp $
*/

#ifndef __WXALGOS_H__
#define __WXALGOS_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <map>
#include <set>

using namespace std;

#include "wx/canvas/canglob.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/xmlpars.h"

//! Debug handler for output the hierarchy found in a a2dDocument
/*!

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dDumpWalker: public a2dWalkerIOHandler
{
public:

    a2dDumpWalker();

    ~a2dDumpWalker();

    void Initialize();

    //! Start traversing at object, returns true.
    bool Start( a2dObject* object );

    //! called from within a2dCanvasDocument
    bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );
};

//! perform a function on each a2dCanvasObject in the document
/*!

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_CallMemberFunc: public a2dWalkerIOHandler
{
public:

    a2dWalker_CallMemberFunc();

    ~a2dWalker_CallMemberFunc();

    typedef void ( a2dCanvasObject::*a2dCanvasObjectFunc ) ();

    //! object to start the algorithm
    /*!
        all object nested down here will have the function called.
    */
    bool Start( a2dObject* object, a2dCanvasObjectFunc func );

    //! called from within a2dCanvasDocument
    bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );


    a2dCanvasObjectFunc m_func;
};

//! perform a function on each a2dCanvasObject in the document
/*!

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_CallMemberFuncW: public a2dWalkerIOHandler
{
public:

    a2dWalker_CallMemberFuncW();

    ~a2dWalker_CallMemberFuncW();

    typedef void ( a2dCanvasObject::*a2dCanvasObjectFuncW ) ( a2dWalkerIOHandler* );

    //! object to start the algorithm
    /*!
        all object nested down here will have the function called.
        return true
    */
    bool Start( a2dObject* object, a2dCanvasObjectFuncW func );

    //! called from within a2dCanvasDocument
    bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );


    a2dCanvasObjectFuncW m_func;
};

//! set layers available in a2dDrawingPart as found in document
/*!

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_SetAvailable: public a2dWalkerIOHandler
{
public:

    a2dWalker_SetAvailable( a2dLayers* layerSetup );

    a2dWalker_SetAvailable( a2dDrawingPart* drawingPart );

    ~a2dWalker_SetAvailable();

    //! object to start the algorithm
    /*!
        all object nested down here will be check on layers available
        returns true
    */
    bool Start( a2dObject* object );

    //! called from within a2dCanvasDocument
    bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

protected:

    //! visible on this drawingPart
    a2dDrawingPart* m_drawingPart;

    //! availability set for this layer setup
    a2dLayers* m_layerSetup;
};

//! Remove from a2dObject in a hierarchy, certain properties.
/*!
    Property Id's stored in a list, are searched for in the hierarchy of a document.
    a2dObject::Walker() etc. is used to walk through the hierarchy.
    Every a2dObject passed, is inspected for containing one of the the properties of the list.
    The found properties will be removed.

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_RemoveProperty: public a2dWalkerIOHandler
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_RemoveProperty( bool all = true );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_RemoveProperty( a2dPropertyId* id , bool all = true );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_RemoveProperty( const a2dPropertyIdList& idList , bool all = true );

    ~a2dWalker_RemoveProperty();

    //! add a a2dPropertyId to the list
    void AddPropertyId( a2dPropertyId* id );

    //! start removing properties from the object given, and down.
    //! returns true if some property was removed.
    bool Start( a2dObject* object );

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! to remove all or only one property on each object found.
    bool m_all;

    //! properties to search for
    a2dPropertyIdList m_propertyList;
};

//! Remove from property in a hierarchy of a canvas document.
/*!
    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_RemovePropertyCandoc: public a2dWalkerIOHandler
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_RemovePropertyCandoc( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, bool all = true );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_RemovePropertyCandoc( a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, bool all = true );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_RemovePropertyCandoc( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, bool all = true );

    ~a2dWalker_RemovePropertyCandoc();

    //! add a a2dPropertyId to the list
    void AddPropertyId( a2dPropertyId* id );

    //! start removing properties from the object given, and down.
    //! returns true if some property was removed.
    bool Start( a2dCanvasObject* object );

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! if set when a property  is removed make the canvas object pending.
    void SetMakePending( bool makePending ) { m_makePending = makePending; }

    //! to remove all or only one property on each object found.
    bool m_all;

    //! properties to search for
    a2dPropertyIdList m_propertyList;

    //! mask required in object
    a2dCanvasObjectFlagsMask m_mask;

    //! how deep to recurse into document
    int m_depth;

    //! set object pending when property is removed.
    bool m_makePending;
};

//! document walking to select a2dCanvasObject by filtering against several constraints.
/*!
    This is often used as base class for other algorithms.
    Its task is to select object that adhere to the filter properties set for this class.

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_FilterCanvasObjects: public a2dWalkerIOHandler
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_FilterCanvasObjects( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_FilterCanvasObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_FilterCanvasObjects( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    ~a2dWalker_FilterCanvasObjects();

    //! add a a2dPropertyId to the list
    void AddPropertyId( const a2dPropertyId* id );

    //! used from constructors
    void Initialize();

    //! start iteration here
    //! returns m_result, which value depends on derived class, and defaults to false.
    bool Start( a2dCanvasObject* object );

    //! add to classname to set of classes to be filtered.
    void AddClassName( wxString className );

    //! remove this classname from set.
    void RemoveClassName( wxString className );

    //! when set, object filtered on classname are written to target ( file or layer )
    //! Else non filtered on classname objects are written to the target.
    void SetAllowClassList( bool allow ) { m_allowClassList = allow; }

    //! when set, object filtered on classname are written to target ( file or layer )
    //! Else non filtered on classname objects are written to the target.
    bool GetAllowClassList() { return m_allowClassList; }

    //! if not empty, objects must have this name
    void SetObjectName( const wxString& objectname = wxT( "" ) ) { m_objectname = objectname; }

    //! if not 0, objects must have this UniqueSerializationId
    void SetUniqueSerializationId( wxInt64 id ) { m_id = id; }

    //! if valid, objects must fit in this boundingbox
    void SetBoundingBox( const a2dBoundingBox& bbox ) { m_bbox = bbox; }

    //! if boundingbox is set, the object is testen against this condition, default _IN
    void SetBoundingBoxCondition( wxUint16 intersectionCondition ) { m_intersectionCondition = intersectionCondition; }

    //! how far we go into nested a2dObject's
    void SetRecursionDepth( int depth = INT_MAX ) { m_depth = depth; }

    //! if set, layer which the object is on must be visible
    void SetCheckLayerVisible( bool check ) { m_layervisible = check; }

    //! if set, layer which the object is on must be selectable
    void SetCheckLayerSelectable( bool check ) { m_layerselectable = check; }

    //! if set, the start object will be skipped
    void SetSkipStart( bool skip ) { m_skipStartObject = skip; }

    //! if not empty, object must have the properties in this list
    a2dPropertyIdList m_propertyList;

    //! object must fit this mask.
    a2dCanvasObjectFlagsMask m_mask;

    //! list of classnames to be filtered
    set<wxString> m_classnameMap;

    //! name for object is not empty
    wxString m_objectname;

    //! wanted id
    wxInt64 m_id;

    //! wanted depth
    int m_depth;

    //! object must be in this box if it the box is valid
    a2dBoundingBox m_bbox;

    //! test m_bbox against teh object in question for this condition
    wxUint16 m_intersectionCondition;

    //!if set, the layer must be visible
    bool m_layervisible;

    //!if set, the layer must be selectable
    bool m_layerselectable;

    //! the start object itself is not checked
    bool m_skipStartObject;

    //! to negate list of classnames to be filtered
    bool m_allowClassList;

    //! convert canvasobject to a list of polygons.
    //! But only for simple primitive objects.
    //! first a2dCanvasObject::GetAsCanvasVpaths() is used to convert to vector paths,
    //! and next the vector path is translated into polygons.
    a2dCanvasObjectList* ConvertToPolygons( a2dCanvasObject* canvasobject, bool transform );

    //! convert canvasobject to a list of Vpath.
    //! But only for simple primitive objects.
    //! first a2dCanvasObject::GetAsCanvasVpaths() is used to convert to vector paths,
    a2dCanvasObjectList* ConvertToVpath( a2dCanvasObject* canvasobject, bool transform );

protected:

    //! return true if object fits the filter
    bool ObjectOke( a2dCanvasObject* obj );

    //! return true of object fits the classname filter
    bool ClassOfObjectOke( a2dCanvasObject* obj );

};

//! collect a2dCanvasObject's in a hierarchy of a a2dCanvasDocument
/*!
    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_CollectCanvasObjects: public a2dWalker_FilterCanvasObjects
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_CollectCanvasObjects( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_CollectCanvasObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_CollectCanvasObjects( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    ~a2dWalker_CollectCanvasObjects();

    void Initialize();

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! objects found
    a2dCanvasObjectList m_found;
};

struct a2dCanvasObjectPtr_compare {
    bool operator() (const a2dCanvasObjectPtr& lhs, const a2dCanvasObjectPtr& rhs) const{
        return lhs.Get() < rhs.Get();
    }
};

typedef set< a2dCanvasObjectPtr,  a2dCanvasObjectPtr_compare > a2dCanvasObjectsSet;

//! used to canvas object on name
extern bool NameSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y );
//! used to canvas object on name in reverse order
extern bool NameSorterReverse( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y );
//! used to canvas object on number of childs order and if same on name
extern bool OnNrChilds( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y );
//! used to canvas object on number of childs reverse order and if same on name
extern bool OnNrChildsReverse( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y );

//! collect a2dCanvasObject's in a hierarchy of a a2dCanvasDocument
/*!
    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_CollectCanvasObjectsSet: public a2dWalker_FilterCanvasObjects
{
public:

    enum a2dSortOn
    {
        //! name of canvasobject
        onName,
        //! name of canvasobject reverse
        onNameReverse,
        //! Number of childs
        onNrChilds,
        //! Number of childs reverse
        onNrChildsReverse
    };

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_CollectCanvasObjectsSet( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_CollectCanvasObjectsSet( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_CollectCanvasObjectsSet( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    ~a2dWalker_CollectCanvasObjectsSet();

    void Initialize();

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! if set, objects must have childs
    void SetWithChilds( bool check ) { m_childs = check; }

    void SortOn( a2dSortOn which ) { m_sortOn = which; }

    bool Start( a2dCanvasObject* object );

    //! objects found
    a2dCanvasObjectsSet m_found;

private:

    bool m_childs;

    a2dSortOn m_sortOn;

};

//! find a2dCanvasObjects in a hierarchy of a a2dCanvasDocument
/*!
    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_FindCanvasObject: public a2dWalkerIOHandler
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_FindCanvasObject( a2dCanvasObject* toSearch );

    ~a2dWalker_FindCanvasObject();

    bool Start( a2dCanvasObject* object );

    void Initialize();

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! object found
    a2dCanvasObjectPtr m_search;
};

//! set a2dCanvasObjects flags in a hierarchy of a a2dCanvasDocument
/*!
    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_SetSpecificFlagsCanvasObjects: public a2dWalker_FilterCanvasObjects
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_SetSpecificFlagsCanvasObjects( a2dCanvasObjectFlagsMask which, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_SetSpecificFlagsCanvasObjects( a2dCanvasObjectFlagsMask which, const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_SetSpecificFlagsCanvasObjects( a2dCanvasObjectFlagsMask which, const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    ~a2dWalker_SetSpecificFlagsCanvasObjects();

    void Initialize();

    //! start removing properties from the object given, and down.
    bool Start( a2dCanvasObject* object, bool setTo );

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! mask for which flags to set
    a2dCanvasObjectFlagsMask m_which;

    //! clear or set the flags
    bool m_setOrClear;
};

//! set pending flag in a drawing
class A2DCANVASDLLEXP a2dWalker_SetPendingFlags: public a2dWalker_SetSpecificFlagsCanvasObjects
{
    DECLARE_CLASS( a2dWalker_SetPendingFlags )

public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_SetPendingFlags( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL ):
        a2dWalker_SetSpecificFlagsCanvasObjects( a2dCanvasOFlags::ALL, mask )
    {
        m_skipNotRenderedInDrawing = true;
    }

    bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );
};

//! set pending flag in a drawing
class A2DCANVASDLLEXP a2dWalker_ResetPendingFlags: public a2dWalker_SetSpecificFlagsCanvasObjects
{
    DECLARE_CLASS( a2dWalker_ResetPendingFlags )

public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_ResetPendingFlags():
        a2dWalker_SetSpecificFlagsCanvasObjects( a2dCanvasOFlags::ALL )
    {
        m_skipNotRenderedInDrawing = false;
    }

    bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );
};

//! set a2dCanvasObjects property in a hierarchy of a a2dCanvasDocument
/*!
    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_SetPropertyToObject: public a2dWalkerIOHandler
{
public:

    //! constructor with a2dPropertyId to set
    a2dWalker_SetPropertyToObject( a2dNamedProperty* prop );

    ~a2dWalker_SetPropertyToObject();

    void Initialize();

    //! called from within a2dDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! id of property to set
    a2dNamedProperty* m_property;

};

//----------------------------------------------------------------------------
// a2dWalker_SetProperty
//----------------------------------------------------------------------------

//! set a2dObjects property in a hierarchy of a a2dDocument
/*!
    \ingroup docalgo
*/
template < class basetype, class a2dpropertyidtyped, class proptype = a2dObject >
class a2dWalker_SetProperty: public a2dWalkerIOHandler
{
public:

    //! constructor with no a2dPropertyIdTyped<basetype, proptype> added yet. For that use AddPropertyId()
    a2dWalker_SetProperty( basetype valueToSet, bool all = true );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_SetProperty( const a2dpropertyidtyped* id, basetype valueToSet , bool all = true );

    //! destructor
    ~a2dWalker_SetProperty();

    //! add a a2dPropertyId to the list
    void AddPropertyId( const a2dpropertyidtyped* id );

    //! start removing properties from the object given, and down.
    bool Start( a2dObject* object );

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! to remove all or only one property on each object found.
    bool m_all;

    //! value to set
    basetype m_valueToSet;

    //! properties to search for
    a2dPropertyIdList m_propertyList;
};

typedef a2dWalker_SetProperty< bool, a2dPropertyIdBool > a2dWalker_SetBoolProperty;

typedef a2dWalker_SetProperty< wxInt16, a2dPropertyIdInt16> a2dWalker_SetInt16Property;
typedef a2dWalker_SetProperty< wxUint16, a2dPropertyIdUint16> a2dWalker_SetUint16Property;
typedef a2dWalker_SetProperty< wxInt32, a2dPropertyIdInt32> a2dWalker_SetInt32Property;
typedef a2dWalker_SetProperty< wxUint32, a2dPropertyIdUint32> a2dWalker_SetUint32Property;

typedef a2dWalker_SetProperty< double, a2dPropertyIdDouble> a2dWalker_SetDoubleProperty;
typedef a2dWalker_SetProperty< float, a2dPropertyIdFloat> a2dWalker_SetFloatProperty;
typedef a2dWalker_SetProperty< wxString, a2dPropertyIdString> a2dWalker_SetStringProperty;

template< class basetype, class a2dpropertyidtyped, class proptype >
a2dWalker_SetProperty< basetype, a2dpropertyidtyped, proptype >::a2dWalker_SetProperty( basetype valueToSet, bool all )
{
    m_valueToSet = valueToSet;
    m_all = all;
}

template< class basetype, class a2dpropertyidtyped, class proptype >
a2dWalker_SetProperty< basetype, a2dpropertyidtyped, proptype >::a2dWalker_SetProperty( const a2dpropertyidtyped* id, basetype valueToSet, bool all )
{
    m_valueToSet = valueToSet;
    m_propertyList.push_back( ( a2dPropertyId* ) id );
    m_all = all;
}

template< class basetype, class a2dpropertyidtyped, class proptype >
a2dWalker_SetProperty< basetype, a2dpropertyidtyped, proptype >::~a2dWalker_SetProperty()
{
}

template< class basetype, class a2dpropertyidtyped, class proptype >
void a2dWalker_SetProperty< basetype, a2dpropertyidtyped, proptype >::AddPropertyId( const a2dpropertyidtyped* id )
{
    m_propertyList.push_back( ( a2dPropertyId* ) id );
}


template< class basetype, class a2dpropertyidtyped, class proptype >
bool a2dWalker_SetProperty< basetype, a2dpropertyidtyped, proptype >::Start( a2dObject* object )
{
    if ( !object )
        return false;
    object->Walker( NULL, *this );
    return true;
}

template< class basetype, class a2dpropertyidtyped, class proptype >
bool a2dWalker_SetProperty< basetype, a2dpropertyidtyped, proptype >::WalkTask( wxObject* WXUNUSED( parent ), wxObject* object, a2dWalkEvent event )
{
    if ( event == a2dWalker_a2dPropertyStart )
    {
        SetStopWalking( true );
        if ( wxDynamicCast( object, proptype ) )
        {
            a2dObject* propobj = wxDynamicCast( object, proptype );
            forEachIn( a2dPropertyIdList, &m_propertyList )
            {
                a2dpropertyidtyped* id = ( a2dpropertyidtyped* ) (*iter).Get();
                id->SetPropertyToObject( propobj, m_valueToSet );
            }
        }
    }
    if ( event == a2dWalker_a2dPropertyEnd )
        SetStopWalking( false );

    return true;
}

//! set check on a2dObject flag false or true
class A2DCANVASDLLEXP a2dWalker_SetCheck: public a2dWalker_SetBoolProperty
{
    DECLARE_CLASS( a2dWalker_SetCheck )

public:

    //! set check on a2dObject flag false or true
    a2dWalker_SetCheck( bool value ):
        a2dWalker_SetBoolProperty( PROPID_Check, value )
    {
    }
};

//! set the a2dCanvasObject's view dependent
/*!
    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_SetViewDependent: public a2dWalkerIOHandler
{
public:

    //! constructor
    /*!
        \param drawingPart view for adding next properties on this object and maybe its children.
        \param viewdependent if true then to add property 'PROPID_viewDependent' to objects
                             that are depend on 'aView' view when it comes to size.
                             Else to remove this property.
        \param viewspecific  if true then to add property 'PROPID_viewSpecific' to objects
                             that are only visible on 'aView' view.
                             Else to remove this property to make visible on all views.
        \param onlyinternalarea Remove scale in matrix for children objects.
    */
    a2dWalker_SetViewDependent( a2dDrawingPart* drawingPart, bool viewdependent, bool viewspecific = false, bool onlyinternalarea = false  );

    //! destructor
    ~a2dWalker_SetViewDependent();

    bool Start( a2dCanvasObject* object );

    void Initialize();

    //! called from within a2dDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! which view to set view dependent
    a2dDrawingPart* m_drawingPart;
    //! on/off for view dependent updating
    bool m_viewdependent;
    //! set only visible for this view
    bool m_viewspecific;
    //! transform without scale for children
    bool m_onlyinternalarea;
};

//! set the a2dCanvasObject's a2dDrawing root recursive
/*!
    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_SetRoot: public a2dWalkerIOHandler
{

    DECLARE_CLASS( a2dWalker_SetRoot )

public:

    //!
    /*!
        \param root set the a2dDrawing object to this root
    */
    a2dWalker_SetRoot( a2dDrawing* root );

    ~a2dWalker_SetRoot();

    //! start setting the root object from here
    bool Start( a2dCanvasObject* object );

    //! called from within a2dDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! document to set
    a2dDrawing* m_root;
};

//! make the child which fit the mask/filter into a tree structure to a certain level
/*!
    Normally a document structure can have multiple references to the same object.
    Like branches on a tree which touch eachother. This walker makes all branches unique.

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_MakeTree: public a2dWalker_FilterCanvasObjects
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_MakeTree( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_MakeTree( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_MakeTree( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    ~a2dWalker_MakeTree();

    //! start here
    bool Start( a2dCanvasObject* object );

    void Initialize();

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

private:

    //! detection for start of first level childs
    bool m_firstLevelChild;
};

//! remove hierarchy in grouped objects
/*!
    in a2dCanvasObject with children, the children are moved one level.
    Same for a2dCanvasObjectReference and a2dCanvasObjectArrayReference its m_object.

    m_depth > 1 will result in smashing all children in an object also nest.
    m_depth = 1 will result in on level of hierarchy removed.

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_RemoveHierarchy: public a2dWalker_FilterCanvasObjects
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_RemoveHierarchy( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_RemoveHierarchy( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_RemoveHierarchy( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    ~a2dWalker_RemoveHierarchy();

    //! start here
    bool Start( a2dCanvasObject* object );

    void Initialize();

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

private:

    //! set objects selected after removing hierarchy
    bool m_selected;

    //! detection for start of first level childs
    bool m_firstLevelChild;

    //! to hold parents passed
    a2dSmrtPtrList<a2dObject> m_parentList;
};


//! base algorithm class to work on a2dCanvasObjects in groups of layers
/*!
    groupA and groupB are filled with layer numbers.

    a2dCanvasObjects with the flag a2dCanvasOFlags::m_ignoreLayer set, will be ignored here.

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_LayerGroup: public a2dWalker_FilterCanvasObjects
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_LayerGroup( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_LayerGroup( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_LayerGroup( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! destructor
    ~a2dWalker_LayerGroup();

    //! used from constructor
    void Initialize();

    //! start at this object
    bool Start( a2dCanvasObject* object );

    //! objects on layers in group A will be used as first operand
    void SetGroupA( const a2dLayerGroup& groupA ) { m_groupA = groupA; }

    //! objects on layers in group B will be used as first operand
    void SetGroupB( const a2dLayerGroup& groupB ) { m_groupB = groupB; }

    a2dLayerGroup GetGroupA() const { return m_groupA; }

    a2dLayerGroup GetGroupB() const { return m_groupB; }

    //! set the layer where the result will be placed
    void SetTarget( wxUint16 targetlayer );

    wxUint16 GetTarget() const { return m_target; };

    //! set selectedOnlyA in group operations
    void SetSelectedOnlyA( bool selectedOnlyA ) { m_selectedOnlyA = selectedOnlyA; }

    //! get selectedOnlyA in group operations
    bool GetSelectedOnlyA() { return m_selectedOnlyA; }

    //! set selectedOnlyB in group operations
    void SetSelectedOnlyB( bool selectedOnlyB ) { m_selectedOnlyB = selectedOnlyB; }

    //! get selectedOnlyB in group operations
    bool GetSelectedOnlyB() { return m_selectedOnlyB; }

protected:

    //! group A layers operand
    a2dLayerGroup m_groupA;

    //! group B layers operand
    a2dLayerGroup m_groupB;

    //! result into this layer
    wxUint16 m_target;

    //! work only on selected object in group A
    bool m_selectedOnlyA;
    //! work only on selected object in group B
    bool m_selectedOnlyB;

};

//! detection of small objects.
/*! Detect objects, which are smaller than a certain size.
The report is generated as a string containing the CVG output, which can be written to a file.
*/
class A2DCANVASDLLEXP a2dWalker_DetectSmall: public a2dWalker_LayerGroup
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_DetectSmall( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_DetectSmall( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_DetectSmall( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! destructor
    ~a2dWalker_DetectSmall();

    //! used from constructor
    void Initialize();

    //! set what is seen as small.
    void SetSmall( double smallest ) { m_small = smallest; }

    //! set asString
    void SetAsString( bool asString ) { m_asString = asString; }

    //! get asString
    bool GetAsString() { return m_asString; }

    bool Start( a2dCanvasObject* object );

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

#if wxUSE_STD_IOSTREAM
    const wxString& GetString() const
    {
        static wxString aap = wxString( m_strstream.str().c_str(), wxConvUTF8 );
        return aap;
    }
#else
    const wxString GetString() const { return m_strstream.GetString(); }
#endif

protected:

    a2dIOHandlerCVGOut m_CVGwriter;
    a2dDocumentStringOutputStream m_strstream;
    bool m_asString;
    double m_small;
};

//! detection of small objects.
/*! Detect objects, which are smaller than a certain size.
The report is generated as a string containing the CVG output, which can be written to a file.
*/
class A2DCANVASDLLEXP a2dWalker_DetectSmallSegments: public a2dWalker_LayerGroup
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_DetectSmallSegments( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_DetectSmallSegments( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_DetectSmallSegments( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! destructor
    ~a2dWalker_DetectSmallSegments();

    //! used from constructor
    void Initialize();

    //! set what is seen as small.
    void SetSmall( double smallest ) { m_small = smallest; }

    //! set asString
    void SetAsString( bool asString ) { m_asString = asString; }

    //! get asString
    bool GetAsString() { return m_asString; }

    bool Start( a2dCanvasObject* object );

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

#if wxUSE_STD_IOSTREAM
    const wxString& GetString() const
    {
        static wxString aap = wxString( m_strstream.str().c_str(), wxConvUTF8 );
        return aap;
    }
#else
    const wxString GetString() const { return m_strstream.GetString(); }
#endif

protected:

    a2dIOHandlerCVGOut m_CVGwriter;
    a2dDocumentStringOutputStream m_strstream;
    bool m_asString;
    double m_small;
};

//! find the a2dcanvasObject, and set the corridor flag on the path to it.
/*!
    Corridors are used as event tubes in a a2dCanvasDocument, in order to redirect events directly
    to a specific object, but maintaining its position/matrixes correctly as in the document.

     \see a2dIterC::SetCorridorPath
     \see a2dDrawingPart::ClearCorridorPath

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_FindAndSetCorridorPath: public a2dWalker_FindCanvasObject
{
public:

    //! constructor
    /*! \see a2dIterC::SetCorridorPath
        \see a2dDrawingPart::FindAndSetCorridorPath
        This function is not used directly, but via a2dCanvasView::FindAndSetCorridorPath

        \param findObject (child) object to be searched for.
        \return true if findObject was found
    */
    a2dWalker_FindAndSetCorridorPath( a2dCanvasObject* findObject );

    //! destructor
    ~a2dWalker_FindAndSetCorridorPath();

    //! called from within a2dDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );
};

//! filter to string or file
/*!
    First filter is ObjectOke(), and next
    objects with classnames which are part of the m_classnameMap ( or NOT if m_allowClassList is set ),
    are filtered.

*/
class A2DCANVASDLLEXP a2dWalker_AllowedObjects: public a2dWalker_LayerGroup
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_AllowedObjects( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_AllowedObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_AllowedObjects( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! destructor
    ~a2dWalker_AllowedObjects();

    //! used from constructor
    void Initialize();

    //! set asString
    void SetAsString( bool asString ) { m_asString = asString; }

    //! get asString
    bool GetAsString() { return m_asString; }

    //! set if result must be stored
    void SetStore( bool store ) { m_store = store; }

    //! get store
    bool GetStore() { return m_store; }

    bool Start( a2dCanvasObject* object );

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

#if wxUSE_STD_IOSTREAM
    const wxString& GetString() const
    {
        static wxString aap = wxString( m_strstream.str().c_str(), wxConvUTF8 );
        return aap;
    }
#else
    const wxString GetString() const { return m_strstream.GetString(); }
#endif

protected:

    a2dIOHandlerCVGOut m_CVGwriter;
    a2dDocumentStringOutputStream m_strstream;
    bool m_asString;
    bool m_store;
};

//! Walker taking iterative context into account.
/*!

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalkerIOHandlerWithContext: public a2dWalkerIOHandler
{
public:

    //! constructor
    a2dWalkerIOHandlerWithContext( a2dIterC& ic );

    //! destructor
    ~a2dWalkerIOHandlerWithContext();

    //! init
    void Initialize();

    //! Start traversing at object, returns true.
    bool Start( a2dCanvasObject* object );

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( a2dIterC& ic, wxObject* parent, wxObject* object, a2dWalkEvent event ) = 0;

    a2dIterC* m_context;
};


//! set a2dCanvasObjects check flag when in view
/*!
    \ingroup docalgo
*/
class a2dWalker_RenderLayers: public a2dWalkerIOHandlerWithContext
{
public:

    //! constructor
    a2dWalker_RenderLayers( a2dIterC& ic );

    //! destructor
    ~a2dWalker_RenderLayers();

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( a2dIterC& ic, wxObject* parent, wxObject* object, a2dWalkEvent event );

    void Render( a2dIterC& ic, a2dCanvasObject* canobj, OVERLAP clipparent );
};

//! Find pins in view, which can connect to the given pinclass.
/*!

    To provide visual feedback by other objects that might participate in an editing sequence.

    In some situations, e.g. when drawing wires or when draging objects in hierarchies,
    it is helpfull if other objects provide visual feedback, that they e.g. would
    accept the currently edited objects. If e.g. a wire is drawn, all pins that can
    accept the wire can turn green and all other pins can turn red. This class
    is usually used from the show object of a a2dDrawingPart and goes down recursively.

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_FindPinsToConnect: public a2dWalker_FilterCanvasObjects
{
public:

    a2dWalker_FindPinsToConnect( a2dCanvasObjectFlagsMask which, a2dPinClass* pinclassToConnectTo,
                                 a2dConnectTask connectTask, double xpin, double ypin, double margin );

    ~a2dWalker_FindPinsToConnect();

    void Initialize();

    void StopAtFirst( bool stopAtFirst ) { m_stopAtFirst = stopAtFirst; }

    //! called from within a2dCanvasDocument
    bool Start( a2dCanvasObject* object );

    double m_xpin;
    double m_ypin;

    a2dPinClass* m_pinclassToConnectTo;

    a2dPinClass* m_returnPinclass;

    //! pins found
    a2dCanvasObjectList m_found;

protected:

    double m_margin;
    bool m_stopAtFirst;

    a2dConnectTask m_connectTask;
};

class A2DCANVASDLLEXP a2dWalker_FindPinsToConnectToPin: public a2dWalker_FilterCanvasObjects
{
public:

    a2dWalker_FindPinsToConnectToPin( a2dCanvasObjectFlagsMask which, a2dPin* pinToConnectTo,
                                 a2dConnectTask connectTask, double margin );

    ~a2dWalker_FindPinsToConnectToPin();

    void Initialize();

    void StopAtFirst( bool stopAtFirst ) { m_stopAtFirst = stopAtFirst; }

    //! called from within a2dCanvasDocument
    bool Start( a2dCanvasObject* object );

    a2dPin* m_pinToConnectTo;

    //! pins found
    a2dCanvasObjectList m_found;
    double m_margin;

    a2dConnectTask m_connectTask;

    bool m_stopAtFirst;
};

//! Set pins in begin state after wiring up some pins
/*!
    This is used to highlight matching pins when drawing a wire

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_SetPinsToBeginState: public a2dWalker_FilterCanvasObjects
{
public:

    a2dWalker_SetPinsToBeginState( a2dCanvasObjectFlagsMask which );

    ~a2dWalker_SetPinsToBeginState();

    //! Start traversing at object, returns true.
    bool Start( a2dCanvasObject* object );

    void Initialize();
};


//! Set pins in end state after wiring up some pins
/*!
    This is used to delete dynamically create pins and remove highlighting

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_SetPinsToEndState: public a2dWalker_FilterCanvasObjects
{
public:

    a2dWalker_SetPinsToEndState( a2dCanvasObjectFlagsMask which );

    ~a2dWalker_SetPinsToEndState();

    //! Start traversing at object, returns true.
    bool Start( a2dCanvasObject* object );

    void Initialize();
};

//! Generate pins on objects, which can connect to the given pinclass.
/*!
    This is used to dynamically create pins, which can connect to
    the pinclass given. When a wire needs to be connected to another wire or object,
    this results in generating the pin if non there. The connection is not made yet,
    it is only an indication that connection is possible.

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_GeneratePinsToConnect: public a2dWalker_FilterCanvasObjects
{
public:

    a2dWalker_GeneratePinsToConnect( a2dCanvasObjectFlagsMask which, a2dPinClass* pinclassToConnectTo,
                                     a2dConnectTask connectTask, double xpin, double ypin );

    ~a2dWalker_GeneratePinsToConnect();

    void Initialize();

    //! Start sgenerating pins for children of object if ItsChildren is set.
    //! Else only for object.
    bool Start( a2dIterC& ic, a2dCanvasObject* object, bool ItsChildren = true );

    double m_xpin;
    double m_ypin;

    a2dPinClass* m_pinclassToConnectTo;

    a2dIterC* m_ic;

    a2dConnectTask m_connectTask;
};

//! Generate pins on objects, and test which can connect to the given object.
/*!
    Pins in the given canvas object are tested for possible connection to pins on other objects.
    Meaning if pins of other objects are at the same position as a pin of the given object,
    a test done, to see if a connection can be made.
    The connection is not made yet, it is only an indication that connection is possible.
    This is done by setting the rendering mode for those pins different.
    The will automatically be re-rendered in idle time.

    Some canvas objects can create pins automatically. If the pins of the given object, hit another objects,
    and it can generate pins automatically, this will be done first. Next a connection test on those pins is made.
    The generated pins are temporary, and if no connection is made, they will be deleted in idle time.

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_GeneratePinsToConnectObject: public a2dWalker_FilterCanvasObjects
{
public:

    a2dWalker_GeneratePinsToConnectObject( a2dCanvasObjectFlagsMask which, a2dCanvasObject* connectObject,
                                           a2dConnectTask connectTask );

    ~a2dWalker_GeneratePinsToConnectObject();

    void Initialize();

    bool Start( a2dIterC& ic, a2dCanvasObject* object );

    a2dCanvasObject* m_connectObject;
    a2dIterC* m_ic;
    a2dConnectTask m_connectTask;
};


//! Connect objects, which can connect to the given object.
/*!
    Pins in the given canvas object are tested for possible connection to pins on other objects.
    Meaning if pins of other objects are at the same position as a pin of the given object,
    a test done, to see if a connection can be made.
    If a connection is allowed, a wire is created to connect the objects at the those pins.

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_ConnectToPinsObject: public a2dWalker_FilterCanvasObjects
{
public:

    a2dWalker_ConnectToPinsObject( a2dCanvasObjectFlagsMask which, a2dCanvasObject* connectObject,
                                   a2dConnectTask connectTask, double margin );

    ~a2dWalker_ConnectToPinsObject();

    void Initialize();

    bool Start( a2dCanvasObject* top, bool alsoWires = false );

private:

    bool ConnectAtPin( a2dCanvasObject* top, a2dPin* pintoconnect );

    a2dCanvasObject* m_connectObject;
    a2dConnectTask m_connectTask;
    double m_margin;
};


//! find a2dCameleon objects  and connect them to a given a2dObject to report e.g. wxEVT_CHANGEDMODIFY_DRAWING events.
/*!
    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_ConnectEvent: public a2dWalkerIOHandler
{
public:

    //! constructor
    a2dWalker_ConnectEvent( a2dObject* reportTo, wxEventType eventType, bool connect = true );

    ~a2dWalker_ConnectEvent();

    void Start( a2dObject* object );

    void Initialize();

    //! called within context  of a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! add a classname to the set of objects to connect/disconnect
    void AddClassName( wxString className );

    //! removea classname to the set of objects to connect/disconnect
    void RemoveClassName( wxString className );

    //! add a classinfo to the set of objects to connect/disconnect
    void AddClassInfo( wxClassInfo* classInfo );

    //! removea classinfo to the set of objects to connect/disconnect
    void RemoveClassInfo( wxClassInfo* classInfo );

    bool ClassOfObjectOke( a2dObject* obj );

    //! object found
    a2dObjectPtr m_reportTo;

    wxEventType m_eventType;

    //! connect or disconnect event.
    bool m_connect;

    //! list of classnames to be filtered
    set<wxString> m_classnameMap;

    set<wxClassInfo*> m_classInfoMap;
};

class A2DCANVASDLLEXP a2dCameleon;

//! find a2dCameleon objects  and connect them to a given a2dObject to report e.g. wxEVT_CHANGEDMODIFY_DRAWING events.
/*!
    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_FindCameleonInst: public a2dWalkerIOHandler
{
public:

    //! constructor
    a2dWalker_FindCameleonInst( a2dCameleon* find );

    ~a2dWalker_FindCameleonInst();

    void Start( a2dObject* object );

    void Initialize();

    //! called within context  of a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! object found
    a2dObjectPtr m_cameleon;

    bool m_found;
};

#include "wx/canvas/algos2.h"

#endif
