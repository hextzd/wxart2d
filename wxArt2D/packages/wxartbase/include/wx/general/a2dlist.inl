/*! \file wx/general/a2dlist.inl
    \brief basic list class based on STL containers.
    \author Klaas Holwerda
    \author Kevin J Bluck
    \date 1/11/2005

    Copyright: 2005 (c) Kevin J Bluck

    License: wxWidgets License

    RCS-ID: $Id: a2dlist.inl,v 1.9 2008/06/22 23:09:40 titato Exp $
*/

#ifndef __A2DLIST_INL__
#define __A2DLIST_INL__

template<class T>
a2dlist<T>::a2dlist( const a2dlist<T> &other )
{
    typename a2dlist<T>::const_iterator  iter;
    for( iter = other.begin(); iter != other.end(); iter++ )
	{
        this->push_back( (*iter) );
    }
}

template<class T>
a2dlist<T>::~a2dlist()
{
}

#endif // __A2DLIST_INL__
