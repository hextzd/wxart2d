/*! \file wx/canvas/objlist.h
    \brief list for a2dCanvasObject

    Action that can be performed on a list of a2dCanvasObject's go into a2dCanvasObjectList.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: objlist.h,v 1.30 2009/09/26 19:01:02 titato Exp $
*/

#ifndef __WXOBJLIST_H__
#define __WXOBJLIST_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/geometry.h"
#include "wx/artbase/afmatrix.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/canprop.h"

//----------------------------------------------------------------------------
// decls
//----------------------------------------------------------------------------

//! define a NON a2dCanvasObjectList
/*!
    Even if a a2dCanvasObject has no children, still its Length can be asked for.
*/
A2DCANVASDLLEXP_DATA( extern a2dCanvasObjectList* ) wxNullCanvasObjectList;

//initalization
#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP std::allocator<class a2dSmrtPtr<class a2dCanvasObject> >;
template class A2DCANVASDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dCanvasObject>, std::allocator<class a2dSmrtPtr<class a2dCanvasObject> > >::_Node >;
template class A2DCANVASDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dCanvasObject>, std::allocator<class a2dSmrtPtr<class a2dCanvasObject> > >::_Nodeptr >;
template class A2DCANVASDLLEXP std::list<class a2dSmrtPtr<class a2dCanvasObject> >::iterator;
template class A2DCANVASDLLEXP std::list<class a2dSmrtPtr<class a2dCanvasObject> >;
template class A2DCANVASDLLEXP a2dlist<a2dSmrtPtr<a2dCanvasObject> >;
template class A2DCANVASDLLEXP a2dSmrtPtrList<a2dCanvasObject>;
#endif

#include <wx/listimpl.cpp>

//! list of a2dCanvasObject's
/*!
    Action that can be performed on a list of a2dCanvasObject's go into a2dCanvasObjectList.

    \ingroup  canvasobject
*/

class a2dCanvasObjectIter;

class A2DCANVASDLLEXP a2dCanvasObjectList: public a2dSmrtPtrList<a2dCanvasObject>
{

public:
    a2dCanvasObjectList();

    ~a2dCanvasObjectList();

    //!this only copies pointer stored in the list.
    /*!
        if you want the object itself copied also, use Clone
    */
    a2dCanvasObjectList& operator=( const a2dCanvasObjectList& other );

    //!Clone everything ( Clones objects also)  in a new created list
    a2dCanvasObjectList* Clone( a2dObject::CloneOptions options ) const;

    //! clone to new list only objects with certain mask
    /*!
        \param mask object must have this mask.
        \param options  way to clone
        \param objectsIndex list with indexes of cloned objects
        \param bbox only find objects within this box.

        \return cloned objects list
    */
    a2dCanvasObjectList* Clone( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, a2dObject::CloneOptions options = a2dObject::clone_deep, a2dlist< long >* objectsIndex = NULL, const a2dBoundingBox& bbox = wxNonValidBbox ) const;

    //! clone to new list only objects with check flag set
    /*!
        \param options  way to clone
        \param objectsIndex list with indexes of cloned objects

        \return cloned objects list
    */
    a2dCanvasObjectList* CloneChecked( a2dObject::CloneOptions options = a2dObject::clone_deep, a2dlist< long >* objectsIndex = NULL ) const;

    //! sort in X and is same also Y
    void SortXY();

    //! sort in X and is same also in reverse Y
    void SortXRevY();

    //! sort in Y and is same also X
    void SortYX();

    //! sort in Y and is same also in reverse X
    void SortYRevX();

    //! all with reference count > 1 are cloned.
    void MakeUnique();

    //!set all given bit flags at once recursive for all objects in given boundingbox
    bool SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which, const wxString& classname = wxT( "" ),
                           a2dCanvasObjectFlagsMask whichobjects = a2dCanvasOFlags::ALL, const a2dBoundingBox& bbox = wxNonValidBbox,
                           const a2dAffineMatrix& tworld = a2dIDENTITY_MATRIX );

    //!move only in this group objects with the given mask to the layer given
    bool ChangeLayer( wxUint16 layer, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    a2dBoundingBox GetBBox( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //!move only in this group objects with the given mask to the back of the list drawn last
    /*!
        \param mask object must have this mask.
        \param check  if true, moved objects will have check flag set

        \return number of moved objects
    */
    int BringToTop( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, bool check = false );

    //!move only in this group objects with the given mask to the front of the list drawn first
    /*!
        \param mask object must have this mask.
        \param check  if true, moved objects will have check flag set

        \return number of moved objects
    */
    int BringToBack( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, bool check = false );

    //! release only objects with the given mask and classname and has property named propertyname and object name
    /*!
        \param mask object must have this mask.
        \param classname if set, this classname is required
        \param id is object contains this property, it will be released. If NULL, this parameter is ignored.
        \param name if set, this name is required for the objects
        \param now if true remove all references to object right now, else only set its delete flag for delete in update cycle.
        empty string means ignore.
    */
    int Release( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL,  const wxString& classname = wxT( "" ), const a2dPropertyId* id = NULL, const wxString& name = wxT( "" ), bool now = true );

    //! release a specific object
    /*!
        \param object object to release
        \param backwards start at the end
        \param all if true remove all references to object
        \param now if true remove all references to object right now, else only set its delete flag for delete in update cycle.
        \param id is object contains this property, it will be released. If NULL, this parameter is ignored.

        \return number of released objects
    */
    int Release( a2dCanvasObject* object, bool backwards = false, bool all = true, bool now = true, const a2dPropertyId* id = NULL );

    //!copy only in this group object with the same mask
    /*!
        \param x copy object at distance x
        \param y copy object at distance y
        \param mask object must have this mask.
        \param target if not -1, objects will be copied to this layer
        \param check  if true, copied objects will have check flag set

        \return number of copied objects
    */
    int Copy( double x, double y, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, long target = -1, bool check = false );

    //!move only in this group object with the same mask
    /*!
        \param x move object distance x
        \param y move object distance y
        \param mask object must have this mask.
        \param target if not -1, objects will be copied to this layer
        \param check  if true, moved objects will have check flag set

        \return number of moved objects
    */
    int Move( double x, double y, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, long target = -1, bool check = false );

    //!return the object if it is part of the list
    /*!
        \param obj object to search for
    */
    a2dCanvasObject* Find( a2dCanvasObject* obj ) const;

    //!return the object which fits the filter.
    /*!
        \param objectname object with this name to search for
        \param classname If classname is empty it collects all objects else only object with this class name.
        \param mask object must have this mask.
        \param propid if a property id is given, the object must have a property with this id
        \param valueAsString StringValueRepresentation of the property that is required (if not empty).
        \param id GetId() should be this unless 0
    */
    a2dCanvasObject* Find( const wxString& objectname, const wxString& classname = wxT( "" ),
                           a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL , const a2dPropertyId* propid = NULL, const wxString& valueAsString = wxT( "" ),
                           wxUint32 id = 0 ) const;

    //! If object with the given name is found, it is switched to newobject
    /*!
        The old object will be released.
        The new object will be owned.

        \return true is object was found.
    */
    bool SwitchObjectNamed( const wxString& objectname, a2dCanvasObject* newobject );

    //!Transform objects fitting the given filter.
    /*!
    \param tworld transform matrix to transform the objects
    \param type  If type is empty it collects all objects else only object with this class name.
    \param mask object must have this mask.
    \param id if a property id is given, the object must have a property with this id
    */
    void Transform( const a2dAffineMatrix& tworld , const wxString& type = wxT( "" ),
                    a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL ,
                    const a2dPropertyId* id = NULL );

    //!Transform objects fitting the given filter.
    /*!
    \param tworld transform matrix to be set as the new transform for the objects
    \param type  If type is empty it collects all objects else only object with this class name.
    \param mask object must have this mask.
    \param id if a property id is given, the object must have a property with this id
    */
    void SetTransform( const a2dAffineMatrix& tworld , const wxString& type = wxT( "" ),
                       a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL ,
                       const a2dPropertyId* id = NULL );

    //!Move objects fitting the given filter to the total list.
    /*!
    \param total list of object found (may already contain elements found in earlier call)
    \param type  If type is empty it collects all objects else only object with this class name.
    \param mask object must have this mask.
    \param id the object needs to have this property set if not NULL

    \return number of objects found
    */
    int TakeOverTo( a2dCanvasObjectList* total, const wxString& type = wxT( "" ),
                    a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL ,
                    const a2dPropertyId* id = NULL );

    //!Move objects fitting the given filter from the total list to this list
    /*!
    \param total list of object found (may already contain elements found in earlier call)
    \param type  If type is empty it collects all objects else only object with this class name.
    \param mask object must have this mask.
    \param id the object needs to have this property set if not NULL

    \return number of objects found
    */
    int TakeOverFrom( a2dCanvasObjectList* total, const wxString& type = wxT( "" ),
                      a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL ,
                      const a2dPropertyId* id = NULL );


    //!Copy objects fitting the given filter to the total list.
    /*!
    \param total list of object found (may already contain elements found in earlier call)
    \param type  If type is empty it collects all objects else only object with this class name.
    \param mask object must have this mask.
    \param id if a property id is given, the object must have a property with this id
    \param bbox only find objects within this box.

    \return number of objects found
    */
    int CollectObjects( a2dCanvasObjectList* total, const wxString& type = wxT( "" ),
                        a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL ,
                        const a2dPropertyId* id = NULL,
                        const a2dBoundingBox& bbox = wxNonValidBbox
                      );

    //!set only in this list fill and stroke of objects with the given mask
    bool SetDrawerStyle( const a2dFill& brush, const a2dStroke& stroke, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //DECLARE_CLASS(a2dCanvasObjectList)

    //! Check if all objects are only once in the list
    void AssertUnique();

    //! insert at index, taking into account released objects if needed.
    void Insert( size_t before, a2dCanvasObject* obj, bool ignoreReleased );

    bool RemoveDuplicates( const wxString& classname = "" );

};

typedef a2dCanvasObjectList::iterator a2dCanvasObjectListIter;

#if defined(__WXDEBUG__) && wxUSE_GLOBAL_MEMORY_OPERATORS && wxUSE_DEBUG_NEW_ALWAYS
#define new new(__TFILE__,__LINE__)
#endif

//! corridor as a direct event path to a a2dCanvasObject
/*!
    This class is used to store the state of a corridor path set inside a a2dCanvasDocument.
    All a2dCanvasObject's on the path do have the a2dCanvasOFlags::IsOnCorridorPath flag set.
    A corridor path is normally initiated from inside a a2dCanvasDocument, somewhere in one of its nested a2dCanvasObject's.
    This is done via a2dIterC::SetCorridorPath(), this function also stores the matrixes that the iterative context did
    calculate sofar. This object can therefore be initiated with a a2dIterC.

    The a2dCorridor is used in drawings and editing tools to restore a corridor to a specific object in a document,
    e.g when pushing and popping a tool from the toolstack. See a2dBaseTool::GetCorridor()

    When one tries to construct a corridor based on a2dDrawingPart, the matrixes are calculated based on the matrix
    of the objects in the list.
*/
class A2DCANVASDLLEXP a2dCorridor: public a2dCanvasObjectList
{
public:

    a2dCorridor();

    a2dCorridor( const a2dIterC& context );

    //! initiate corridor from the current corridor path in a document.
    /*!
        The a2dCanvasObject in the document with the m_flags.m_isOnCorridorPath set, are searched
        starting from the view its ShowObject(), and pushed into the corridor list.

        The matrix (and its inverse) at the end of the corridor is calculated from the multiplied
        a2dCanvasObject matrixes in the corridor.
    */
    a2dCorridor( const a2dDrawingPart& drawingPart );

    ~a2dCorridor();

    //! push object onto existing corridor
    void Push( a2dCanvasObject* object );

    //! Get the accumulated transform up to and including m_lworld of the current object
    /*! This converts from relative local coordinates of the current object to world coordinates.
        This matrix transforms all drawing primitives used to draw a a2dCanvasObject from relative
        world coordinates to absolute world coordinates.
    */
    const a2dAffineMatrix& GetTransform() const { return m_relativetransform; }

    //! Inverse of GetTransform()
    const a2dAffineMatrix& GetInverseTransform() const { return m_inverseRelativetransform; }

private:

    //! the accumulated transforms up to and including m_lworld of last object
    a2dAffineMatrix m_relativetransform;

    //! inverse of m_relativetransform
    a2dAffineMatrix m_inverseRelativetransform;
};

class A2DCANVASDLLEXP a2dCanvasObjectIter: public a2dCanvasObjectList::iterator
{
public:

    a2dCanvasObjectIter();

    a2dCanvasObjectIter& operator=( const a2dCanvasObjectIter& other )
    {
        *this = other;
        return *this;
    }

    void operator++()
    {
        a2dCanvasObjectList::iterator::operator++();
    }

    a2dCanvasObject* Item()
    {
        a2dCanvasObject* cur = ( *this )->Get();
        return cur;
    }

};

#endif /* __WXOBJLIST_H__ */
