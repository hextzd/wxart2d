/*! \file wx/general/smrtptr.h
    \brief smart pointer class and list.

    Smart pointers are used all over wxDocview, and the a2dObject from which most classes are derived
    (in)directly is for that reason reference counted.

    \author Michael Sögtrop / Klaas Holwerda
    \date Created 10/29/2003

    Copyright: 2003-2004 (c) Michael Sögtrop / Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: smrtptr.h,v 1.21 2008/09/05 19:01:13 titato Exp $
*/

#ifndef __SMRTPTR_H__
#define __SMRTPTR_H__

#include <string.h>
#include <wx/general/a2dlist.h>

// Enable smart pointer / reference counter debugging
//#define SMART_POINTER_DEBUG
#define __A2DONLYFILE__(fname) (strchr(fname,'\\') ? strrchr(fname,'\\')+1 : fname)
#define __A2DFILE__  __A2DONLYFILE__(__FILE__)


#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
// This is used as a hint which object a smart pointer might be a member of
// All objects that contain smart pointer as members should set this in the
// constructor of their base class (In the constructor itself it would be
// to late, because the smart pointer constructor for members is called before
// the constructor body is executed.
// You can of course also set m_owner and m_ownertype directly in the constructor
// after constructing the smart pointer object.
A2DGENERALDLLEXP_DATA ( extern wxObject* ) CurrentSmartPointerOwner;

struct A2DGENERALDLLEXP a2dInitCurrentSmartPointerOwner
{
public:
    a2dInitCurrentSmartPointerOwner( wxObject* object )
    {
        CurrentSmartPointerOwner = object;
    }
};
#endif

//! A trivial base class for a2dSmrtPtr. Non-template class, so that it can
/*! be made a friend class of a2dObject and avoid noncompliance problems with
    friend templates.

    \ingroup general
*/
class A2DGENERALDLLEXP a2dSmrtPtrBase
{
protected:
    a2dSmrtPtrBase() {}
private:
    a2dSmrtPtrBase( const a2dSmrtPtrBase& );             // Noncopyable; do not implement.
    a2dSmrtPtrBase& operator=( const a2dSmrtPtrBase& );  // Noncopyable; do not implement.
protected:
    template<class T> static void OwnRef( T* pRefObject )     { if( pRefObject ) pRefObject->SmrtPtrOwn(); }
    template<class T> static void ReleaseRef( T* pRefObject ) { if( pRefObject ) pRefObject->SmrtPtrRelease(); }
};


//! A pointer class, that automatically calls SmrtPtrOwn/SmrtPtrRelease
/*! a2dSmrtPtr is a template class, that performs automatic reference counting.
  It can be used like a usual C++ pointer.
  It can be used for any class, that has SmrtPtrOwn and SmrtPtrRelease functions, like this:

<code>
  class XXX
    private:

        // how many references to this object do exist
        int m_refcount;

        // Call to have a new owner for this object
        // This function should only be called by a2dSmrtPtr
        //
        //    \remark owning mean that the object calling this member needs to call Release at some time,
        //            in order to actually release/delete the object.
        //
        //    \return The return value is the object itself, which is now owned on time extra.
        //
        //    increment refcount by 1 ( use when adding a reference to this object)
        //
        a2dLineSegment* SmrtPtrOwn() { m_refcount++; return this; }

        // To release the object, it is not longer owned by the calling object.
        // This function should only be called by a2dSmrtPtr
        bool SmrtPtrRelease()
        {
            m_refcount--;
            wxASSERT_MSG( m_refcount >= 0, wxT("XXX object Own/Release not matched (extra Release calls)") );
            if ( m_refcount <= 0 )
            {
                delete this;
                return true;
            }
            return false;
      }

    private:
       friend class a2dSmrtPtrBase;
</code>

    \ingroup general

*/
template<class Clss>
class a2dSmrtPtr : private a2dSmrtPtrBase
{
public:
    typedef Clss TType;

    a2dSmrtPtr()
    {
        m_ptr = NULL;
        PointerListInit();
    }

    a2dSmrtPtr( Clss* objc )
    {
        PointerListInit();
        m_ptr = objc;
        AddToPointerList();
        OwnRef( m_ptr );
    }

    a2dSmrtPtr( const a2dSmrtPtr& objc )
    {
        PointerListInit();
        m_ptr = &*objc;
        AddToPointerList();
        OwnRef( m_ptr );
    }

    ~a2dSmrtPtr()
    {
        // It is required to set m_ptr to 0 in case the destructor of the target
        // object tries to set the pointer in this object to
        Clss* old = m_ptr;
        RmvFromPointerList();
        m_ptr = 0;
        ReleaseRef( old );
    }

    void operator=( const a2dSmrtPtr& objc )
    {
        Clss* ptrold = m_ptr;
        RmvFromPointerList();
        m_ptr = &*objc;
        AddToPointerList();
        OwnRef( m_ptr );
        ReleaseRef( ptrold );
    }

    void operator=( Clss* objc )
    {
        Clss* ptrold = m_ptr;
        RmvFromPointerList();
        m_ptr = objc;
        AddToPointerList();
        OwnRef( m_ptr );
        ReleaseRef( ptrold );
    }

    friend inline bool operator==( const a2dSmrtPtr& a, const a2dSmrtPtr& b )
    {
        return a.m_ptr == b.m_ptr;
    }

    friend inline bool operator!=( const a2dSmrtPtr& a, const a2dSmrtPtr& b )
    {
        return a.m_ptr != b.m_ptr;
    }

    // This is needed, because otherwise the compiler doesnt know if he
    // should use (Clss*)a == b or a==wxSmtrPtr<Clss>(b)
    friend inline bool operator==( const a2dSmrtPtr& a, const Clss* b )
    {
        return a.m_ptr == b;
    }

    friend inline bool operator!=( const a2dSmrtPtr& a, const Clss* b )
    {
        return a.m_ptr != b;
    }

    friend inline bool operator==( const Clss* a, const a2dSmrtPtr& b )
    {
        return a == b.m_ptr;
    }

    friend inline bool operator!=( const Clss* a, const a2dSmrtPtr& b )
    {
        return a != b.m_ptr;
    }

    friend inline bool operator==( const a2dSmrtPtr& a, Clss* b )
    {
        return a.m_ptr == b;
    }

    friend inline bool operator!=( const a2dSmrtPtr& a, Clss* b )
    {
        return a.m_ptr != b;
    }

    friend inline bool operator==( Clss* a, const a2dSmrtPtr& b )
    {
        return a == b.m_ptr;
    }

    friend inline bool operator!=( Clss* a, const a2dSmrtPtr& b )
    {
        return a != b.m_ptr;
    }

    inline operator Clss* () const {  return m_ptr; }

    inline Clss* Get() const
    {
        return m_ptr;
    }

    inline Clss* operator->() const { /* !!!!! a throw on 0 should go here */  return m_ptr; }

    inline Clss& operator*() const { /* !!!!! a throw on 0 should go here */  return *m_ptr; }

private:
    Clss* m_ptr;

public:

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    a2dSmrtPtr( const wxChar* filename, int linenum )
    {
        m_ptr = NULL;
        PointerListInit();
        m_filename = filename;
        m_linenum = linenum;
    }
    a2dSmrtPtr( const wxChar* filename, int linenum, Clss* objc )
    {
        PointerListInit();
        m_filename = filename;
        m_linenum = linenum;
        m_ptr = objc;
        AddToPointerList();
        OwnRef( m_ptr );
    }
    a2dSmrtPtr( const wxChar* filename, int linenum, const a2dSmrtPtr& objc )
    {
        PointerListInit();
        m_filename = filename;
        m_linenum = linenum;
        m_ptr = &*objc;
        AddToPointerList();
        OwnRef( m_ptr );
    }

    //! This is a double linkes list of all pointers to an object
    /*! This is only used for debugging */
    a2dSmrtPtr<Clss> *m_next;
    a2dSmrtPtr<Clss> *m_prvs;
    wxObject* m_owner;
    const wxChar* m_filename;
    int m_linenum;
    enum
    {
        owner_none,
        owner_wxobject,
        owner_smartpointerlist
    } m_ownertype;

    void PointerListInit()
    {
        // see CurrentSmartPointerOwner comment

        m_next = 0;
        m_prvs = 0;
        m_filename = 0;
        m_linenum = 0;
        // objects tend to be smaller than 1k
        if( ( char* )this - ( char* )CurrentSmartPointerOwner < 1024 )
        {
            m_owner = CurrentSmartPointerOwner;
            m_ownertype = owner_wxobject;
        }
        else
        {
            m_owner = 0;
            m_ownertype = owner_none;
        }
    }

    void AddToPointerList()
    {
        if( !m_ptr ) return;
        m_next = ( a2dSmrtPtr<Clss>* ) m_ptr->m_ownerlist;
        if( m_next )
            m_next->m_prvs = this;
        m_prvs = 0;
        m_ptr->m_ownerlist = ( typename Clss::TOwnerListClass* ) this;
    }

    void RmvFromPointerList()
    {
        if( !m_ptr ) return;
        if( m_prvs )
            m_prvs->m_next = m_next;
        else
            m_ptr->m_ownerlist = ( typename Clss::TOwnerListClass* ) m_next;
        if( m_next )
            m_next->m_prvs = m_prvs;
    }
#else
    void PointerListInit() {}
    void AddToPointerList() {}
    void RmvFromPointerList() {}
#endif
};

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
#define a2dDECLARE_GLOBAL_VAR(type, var)        type var(__FILE__,__LINE__)
#define a2dDECLARE_GLOBAL_ITEM(type, var, item) type var(__FILE__,__LINE__,item)
#define a2dDECLARE_LOCAL_ITEM(type, var, item) CurrentSmartPointerOwner = this;\
    type var(__FILE__,__LINE__,item)
#else
#define a2dDECLARE_GLOBAL_VAR(type, var)        type var
#define a2dDECLARE_GLOBAL_ITEM(type, var, item) type var(item)
#define a2dDECLARE_LOCAL_ITEM(type, var, item) type var(item)
#endif

//! A pointer class, that is automatically set to 0 if the target is deleted
/*! The target keeps a list of all auto zero smart pointers pointing to it.

    For e.g. a2dObject, this list is implemented as a property.

    \ingroup general
*/
template<class Clss>
class a2dAutoZeroPtr
{
public:
    typedef Clss TType;

    a2dAutoZeroPtr()
    {
        m_ptr = NULL;
        PointerListInit();
    }

    a2dAutoZeroPtr( Clss* objc )
    {
        PointerListInit();
        m_ptr = objc;
        AddToPointerList();
    }

    a2dAutoZeroPtr( const a2dAutoZeroPtr& objc )
    {
        PointerListInit();
        m_ptr = &*objc;
        AddToPointerList();
    }

    ~a2dAutoZeroPtr()
    {
        RmvFromPointerList();
    }

    //! This is called from the targets destructor
    /*! It is safe to call this function with a zero this pointer */
    void TargetDestructing()
    {
        a2dAutoZeroPtr<Clss> *next, *crnt;
        for( crnt = this; crnt; crnt = next )
        {
            next = crnt->m_next;
            crnt->m_ptr = 0;
            crnt->m_next = 0;
            crnt->m_prvs = 0;
        }
    }

    void operator=( const a2dAutoZeroPtr& objc )
    {
        RmvFromPointerList();
        m_ptr = &*objc;
        AddToPointerList();
    }

    void operator=( Clss* objc )
    {
        RmvFromPointerList();
        m_ptr = objc;
        AddToPointerList();
    }

    friend inline bool operator==( const a2dAutoZeroPtr& a, const a2dAutoZeroPtr& b )
    {
        return a.m_ptr == b.m_ptr;
    }

    friend inline bool operator!=( const a2dAutoZeroPtr& a, const a2dAutoZeroPtr& b )
    {
        return a.m_ptr != b.m_ptr;
    }

    // This is needed, because otherwise the compiler doesnt know if he
    // should use (Clss*)a == b or a==wxSmtrPtr<Clss>(b)
    friend inline bool operator==( const a2dAutoZeroPtr& a, const Clss* b )
    {
        return a.m_ptr == b;
    }

    friend inline bool operator!=( const a2dAutoZeroPtr& a, const Clss* b )
    {
        return a.m_ptr != b;
    }

    friend inline bool operator==( const Clss* a, const a2dAutoZeroPtr& b )
    {
        return a == b.m_ptr;
    }

    friend inline bool operator!=( const Clss* a, const a2dAutoZeroPtr& b )
    {
        return a != b.m_ptr;
    }

    friend inline bool operator==( const a2dAutoZeroPtr& a, Clss* b )
    {
        return a.m_ptr == b;
    }

    friend inline bool operator!=( const a2dAutoZeroPtr& a, Clss* b )
    {
        return a.m_ptr != b;
    }

    friend inline bool operator==( Clss* a, const a2dAutoZeroPtr& b )
    {
        return a == b.m_ptr;
    }

    friend inline bool operator!=( Clss* a, const a2dAutoZeroPtr& b )
    {
        return a != b.m_ptr;
    }

    inline operator Clss* () const {  return m_ptr; }

    inline Clss* Get() const
    {
        return m_ptr;
    }

    inline Clss* operator->() const { /* !!!!! a throw on 0 should go here */  return m_ptr; }

    inline Clss& operator*() const { /* !!!!! a throw on 0 should go here */  return *m_ptr; }

private:
    //! Initialize the pointer list pointers
    void PointerListInit()
    {
        m_next = 0;
        m_prvs = 0;
        // objects tend to be smaller than 1k
    }
    //! Add this pointer to the pointer list of the target
    void AddToPointerList()
    {
        if( !m_ptr ) return;
        a2dAutoZeroPtr<Clss> **ownerlist = ( a2dAutoZeroPtr<Clss>** ) m_ptr->GetAutoZeroPtrList();
        m_next = *ownerlist;
        if( m_next )
            m_next->m_prvs = this;
        m_prvs = 0;
        *ownerlist = ( a2dAutoZeroPtr<Clss>* ) this;
    }
    //! Remove this pointer from the pointer list of the target
    void RmvFromPointerList()
    {
        if( !m_ptr ) return;
        if( m_prvs )
        {
            m_prvs->m_next = m_next;
        }
        else
        {
            a2dAutoZeroPtr<Clss> **ownerlist = ( a2dAutoZeroPtr<Clss>** ) m_ptr->GetAutoZeroPtrList();
            *ownerlist = ( a2dAutoZeroPtr<Clss>* ) m_next;
        }
        if( m_next )
            m_next->m_prvs = m_prvs;
    }

private:
    Clss* m_ptr;

    //! This is a double linked list of all pointers to an object
    /*! This is only used for debugging */
    a2dAutoZeroPtr<Clss> *m_next;
    a2dAutoZeroPtr<Clss> *m_prvs;
};

//! A pointer class, that does not call SmrtPtrOwn/SmrtPtrRelease, but initializes to 0
/*!
    \ingroup general
*/
template<class Clss>
class a2dDumbPtr
{
public:
    typedef Clss TType;

    a2dDumbPtr()
    {
        m_ptr = NULL;
    }

    a2dDumbPtr( Clss* objc )
    {
        m_ptr = objc;
    }

    a2dDumbPtr( const a2dDumbPtr& objc )
    {
        m_ptr = &*objc;
    }

    void operator=( const a2dDumbPtr& objc )
    {
        m_ptr = &*objc;
    }

    friend inline bool operator==( const a2dDumbPtr& a, const a2dDumbPtr& b )
    {
        return a.m_ptr == b.m_ptr;
    }

    friend inline bool operator!=( const a2dDumbPtr& a, const a2dDumbPtr& b )
    {
        return a.m_ptr != b.m_ptr;
    }

    // This is needed, because otherwise the compiler doesnt know if he
    // should use (Clss*)a == b or a==wxSmtrPtr<Clss>(b)
    friend inline bool operator==( const a2dDumbPtr& a, const Clss* b )
    {
        return a.m_ptr == b;
    }

    friend inline bool operator!=( const a2dDumbPtr& a, const Clss* b )
    {
        return a.m_ptr != b;
    }

    friend inline bool operator==( const Clss* a, const a2dDumbPtr& b )
    {
        return a == b.m_ptr;
    }

    friend inline bool operator!=( const Clss* a, const a2dDumbPtr& b )
    {
        return a != b.m_ptr;
    }

    friend inline bool operator==( const a2dDumbPtr& a, Clss* b )
    {
        return a.m_ptr == b;
    }

    friend inline bool operator!=( const a2dDumbPtr& a, Clss* b )
    {
        return a.m_ptr != b;
    }

    friend inline bool operator==( Clss* a, const a2dDumbPtr& b )
    {
        return a == b.m_ptr;
    }

    friend inline bool operator!=( Clss* a, const a2dDumbPtr& b )
    {
        return a != b.m_ptr;
    }

    inline operator Clss* () const {  return m_ptr; }

    ~a2dDumbPtr()
    {
    }

    inline Clss* Get() const
    {
        return m_ptr;
    }

    inline Clss* operator->() const { /* !!!!! a throw on 0 should go here */ return m_ptr; }

    inline Clss& operator*() const { /* !!!!! a throw on 0 should go here */ return *m_ptr; }

private:
    Clss* m_ptr;
};



//! A list class for reference counted objects
/*! The list uses a2dSmrtPtr for reference counting,
    and therefore objects which reach refcount zero will be deleted automatically.

    \ingroup general
*/
template<class T>
class a2dDumbPtrList: public a2dlist< a2dDumbPtr< T > >
{

public:

    typedef typename a2dlist< a2dDumbPtr< T > >::iterator itSmart;
    typedef typename a2dlist< a2dDumbPtr< T > >::const_iterator const_itSmart;

    //! Find a specific object
    itSmart Find( T* object );

    //! Find a specific object
    const_itSmart Find( T* object ) const;

    //! Find the index of a specific object
    int IndexOf( T* object ) const;

    //! release a certain object from the list
    bool RemoveObject( T* object );

};


//! A list class for reference counted objects
/*! The list uses a2dSmrtPtr for reference counting,
    and therefore objects which reach refcount zero will be deleted automatically.

    \ingroup general
*/
template<class T>
class a2dSmrtPtrList: public a2dlist< a2dSmrtPtr< T > >
{

public:

    typedef typename a2dlist< a2dSmrtPtr< T > >::iterator itSmart;
    typedef typename a2dlist< a2dSmrtPtr< T > >::const_iterator const_itSmart;

    //! Find a specific object
    itSmart Find( T* object );

    //! Find a specific object
    const_itSmart Find( T* object ) const;

    //! Find the index of a specific object
    int IndexOf( T* object ) const;

    //! release a certain object from the list
    bool ReleaseObject( T* object );

};

#if defined(WXDOCVIEW_USINGDLL)
#include "wx/general/smrtptr.inl"
#endif

#endif // __SMRTPTR_H__
