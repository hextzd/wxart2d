/*! \file wx/general/smrtptr.inl
    \brief see wx/general/smrtptr.h
    \author Michael Sögtrop / Klaas Holwerda
    \date Created 10/29/2003

    Copyright: 2003-2004 (c) Michael Sögtrop / Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: smrtptr.inl,v 1.10 2007/07/30 19:34:11 titato Exp $
*/

#ifndef __SMRTPTR_INL__
#define __SMRTPTR_INL__ 



template<class T>
typename a2dDumbPtrList<T>::itSmart a2dDumbPtrList<T>::Find(T *object) 
{
    typename a2dlist< a2dDumbPtr< T > >::iterator iter;

    for( iter = this->begin(); iter != this->end(); ++iter )
    {
        if( (*iter).Get() == object )
            return iter;
    }
    return iter;
}


template<class T>
typename a2dDumbPtrList<T>::const_itSmart a2dDumbPtrList<T>::Find(T *object) const
{
    typename a2dlist< a2dDumbPtr< T > >::const_iterator iter;

    for( iter = this->begin(); iter != this->end(); ++iter )
    {
        if( (*iter).Get() == object )
            return iter;
    }
    return iter;
}

template<class T>
int a2dDumbPtrList<T>::IndexOf( T *object) const
{
    int i=0;
    typename a2dlist< a2dDumbPtr< T > >::const_iterator iter;
    for( iter = this->begin(); iter != this->end(); ++iter )
    {
        if ( (*iter).Get() == object )
            return i;
        i++;
    }
    return -1;
}


template<class T>
bool a2dDumbPtrList<T>::RemoveObject( T *object )
{
    typename a2dlist< a2dDumbPtr< T > >::iterator iter = Find( object );
    if( iter != this->end() )
    {
        this->erase( iter );          
        return true;
    }
    else
        return false;
}

//-------------------------------------------------------

template<class T>
typename a2dSmrtPtrList<T>::itSmart a2dSmrtPtrList<T>::Find(T *object) 
{
    typename a2dlist< a2dSmrtPtr< T > >::iterator iter;

    for( iter = this->begin(); iter != this->end(); ++iter )
    {
        if( (*iter).Get() == object )
            return iter;
    }
    return iter;
}


template<class T>
typename a2dSmrtPtrList<T>::const_itSmart a2dSmrtPtrList<T>::Find(T *object) const
{
    typename a2dlist< a2dSmrtPtr< T > >::const_iterator iter;

    for( iter = this->begin(); iter != this->end(); ++iter )
    {
        if( (*iter).Get() == object )
            return iter;
    }
    return iter;
}

template<class T>
int a2dSmrtPtrList<T>::IndexOf( T *object) const
{
    int i=0;
    typename a2dlist< a2dSmrtPtr< T > >::const_iterator iter;
    for( iter = this->begin(); iter != this->end(); ++iter )
    {
        if ( (*iter).Get() == object )
            return i;
        i++;
    }
    return -1;
}


template<class T>
bool a2dSmrtPtrList<T>::ReleaseObject( T *object )
{
    typename a2dlist< a2dSmrtPtr< T > >::iterator iter = Find( object );
    if( iter != this->end() )
    {
        this->erase( iter );          
        return true;
    }
    else
        return false;
}


#endif // __SMRTPTR_INL__
