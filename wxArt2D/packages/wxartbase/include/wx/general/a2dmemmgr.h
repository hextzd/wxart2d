/*! \file wx/general/a2dmemmgr.h
    \brief memory management which can optionally be used for certain intensively used classes.
    \author Leo kadisoff
    \author Klaas Holwerda
    \date Created 05/25/2005

    Copyright: 2003-2005 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: a2dmemmgr.h,v 1.7 2007/07/30 19:34:11 titato Exp $
*/

#ifndef __A2DMEMMGR_H__
#define __A2DMEMMGR_H__

#include <wx/general/smrtptr.h>

//! Simple Memory manager for some objects which often create and destroy to replace OS-system calls.
class A2DGENERALDLLEXP a2dMemManager
{
private:
    //! a list of freed memory blocks
    struct a2dFreeList
    {
        //! a next list of freed blocks another blockSize
        a2dFreeList* m_next;
        //! a top of current list of freed memory blocks by this blockSize
        void*        m_topOfList;
        //! a current blocksize for list with top m_topOfList
        size_t       m_blockSize;

        //! a count of freed blocks in this list
        int          m_count;

        //! constructor
        a2dFreeList( a2dFreeList* next, size_t blockSize )
            : m_next( next ),
              m_topOfList( NULL ),
              m_blockSize( blockSize ),
              m_count( 0 )
        {
        }
    };
    //! pointer to a first list of freed blocks
    a2dFreeList* m_allLists;
    //! count of lists of freed blocks
    int          m_count;
    //! a name of memory manager for debugging
    const wxChar*  m_mgrname;

//  wxCriticalSection m_guard;

public:
    //! contructor of memory manager
    a2dMemManager( const wxChar* mgrname )  :  m_allLists( NULL ), m_count( 0 ), m_mgrname( mgrname )
    {
    }
    //! destructor of memory manager
    /*!
        Destroing all lists of freed memory blocks which were allocated in Allocate() by OS-system allocation calls
    */
    ~a2dMemManager();

    //! function for allocating memory block by size bytes
    /*!
        Returns pointer to freed block if exists in lists or calls OS-system function for allocating memory block
    */
    void* Allocate( size_t bytes );
    //! function for adding memory block by size bytes to list of freed memory blocks
    /*
       This function doesn't free to OS-system memory block by pointer 'space'.
       It adds memory block by pointer 'space' to internal lists.
       It is speed up.
    */
    void Deallocate( void* space, size_t bytes );
};

#ifdef __WXDEBUG__
#if wxUSE_STACKWALKER
extern A2DGENERALDLLEXP wxString a2dGetStackTrace( size_t skip = 1, bool withArgs = false, int maxCalls = 30, int maxLines = 20 );
#endif
#endif
#endif // __A2DMEMMGR_H__
