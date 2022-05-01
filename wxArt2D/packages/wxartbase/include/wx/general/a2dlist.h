/*! \file wx/general/a2dlist.h
    \brief basic list class based on STL containers.
    \author Klaas Holwerda
    \author Kevin J Bluck
    \date 1/11/2005

    Copyright: 2005 (c) Kevin J Bluck

    License: wxWidgets License

    RCS-ID: $Id: a2dlist.h,v 1.22 2008/08/05 15:52:43 titato Exp $
*/

#ifndef __A2DLIST_H__
#define __A2DLIST_H__

// shared builds stuff
#include "wx/general/generaldef.h"

template<class Clss> class a2dSmrtPtr;

//#include <memory>
//#include <list>

#include <memory>
#include <list>

//! std list compatible list
/*!
    Template list used all over wxDocview. It is a standard template list, with some extra features.
    The (STL) list, stores the object by value.
    Therefore often smartpointers are used to store the object,
    in order to properly delete the object when a node is deleted.

    - See a2dSmrtPtrList< T >
    - See a2dSmrtPtr< T >

    \ingroup general

*/
template<class T>
class a2dlist: public std::list< T, std::allocator<T> >
{
public:

    //! Default constructor
    a2dlist()
    {
    }

    //! Copy constructor
    a2dlist( const a2dlist& other );

    //! Destructor
    ~a2dlist();

    typename a2dlist<T>::const_iterator item( size_t index ) const
    {
        typename a2dlist<T>::const_iterator iter = this->begin();
        for( size_t i = 0; i < index; ++i )
            iter++;
        return iter;
    }

    typename a2dlist<T>::iterator item( size_t index )
    {
        typename a2dlist<T>::iterator iter = this->begin();
        for( size_t i = 0; i < index; ++i )
            iter++;
        return iter;
    }

    typename a2dlist<T>::reverse_iterator rerase( typename std::list< T, std::allocator< T > >::reverse_iterator iterr )
    {
        typename std::list< T, std::allocator< T > >::iterator it( iterr.base() ) ;
        iterr++;
        -- it ;
        this->erase( it ) ;
        return iterr;
    }

    //! get the previous node ( --end() if no previous )
    typename std::list< T, std::allocator< T > >::iterator GetPreviousAround( typename std::list< T, std::allocator< T > >::iterator iter )
    {
        if ( this->size() )
        {
            if ( iter == this->begin() )
                return  --( this->end() );
            return --iter;
        }
        return  this->begin();
    }

    //! get the next node ( begin() if no next )
    typename std::list< T, std::allocator< T > >::iterator GetNextAround( typename std::list< T, std::allocator< T > >::iterator iter )
    {
        if ( this->size() )
        {
            if ( iter == --( this->end() ) )
                return  this->begin();
            return ++iter;
        }
        return  this->begin();
    }
};

//! easy iteration for a2dlist
/*!
    \ingroup general
*/
#define forEachIn( listtype, list ) \
    for( listtype::iterator iter = (list)->begin(); iter != (list)->end(); ++iter )

//! easy const iteration for a2dlist
/*!
    \ingroup general
*/
#define const_forEachIn( listtype, list ) \
    for( listtype::const_iterator iter = (list)->begin(); iter != (list)->end(); ++iter )

#include "a2dlist.inl"

/*
// minimal wrap to be able to use wxWidget or Stl hash implementation.
#if 0
    #ifdef _MSC_VER
        #include <hash_map>
        // In case of compiler problems, please add more checks.
        #if _MSC_VER > 1300
            #define  a2dHashMap stdext::hash_map
        #else
            #define a2dHashMap std::hash_map
        #endif
    #endif // _MSC_VER
    #ifdef __GNUG__
        #include <ext/hash_map>
        #define a2dHashMap __gnu_cxx::hash_map
    #endif // __GNUG__
#else
    template <class Key, class Type> class a2dHashMap
    {
    public:
        class KeyHash
        {
        public:
            unsigned long operator()( const Key& k ) const { return (unsigned long) &k; }
            KeyHash& operator=(const KeyHash&) { return *this; }
        };
        class KeyEqual
        {
        public:
            bool operator()( const Key& a, const Key& b ) const { return ( &a == &b ); }
            KeyEqual& operator=(const KeyEqual&) { return *this; }
        };
        WX_DECLARE_HASH_MAP( Key, Type, KeyHash, KeyEqual, wxhash );
    public:
        Type& operator[](const Key& key) { return m_hash[ key ]; }
        void erase( Key& key ) { m_hash.erase( key ); }
        void clear() { m_hash.clear(); }
    protected:
        wxhash m_hash;
    };
#endif
*/





#endif // __A2DLIST_H__
