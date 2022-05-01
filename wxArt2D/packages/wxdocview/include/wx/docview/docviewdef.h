/*! \file wx/docview/docviewdef.h
    \author Francesco Montorsi
    \date 12/6/2005

    Copyright: 2005 (c) Francesco Montorsi

    License: wxWidgets License

    RCS-ID: $Id: docviewdef.h,v 1.1 2005/06/12 20:55:31 frm Exp $
*/

#ifndef __DOCVIEWDEF__
#define __DOCVIEWDEF__

#if (defined(__WXMSW__) )
#ifdef A2DDOCVIEWMAKINGDLL
#define A2DDOCVIEWDLLEXP WXEXPORT
#define A2DDOCVIEWDLLEXP_DATA(type) WXEXPORT type
#define A2DDOCVIEWDLLEXP_CTORFN
#elif defined(WXUSINGDLL)
#define A2DDOCVIEWDLLEXP WXIMPORT
#define A2DDOCVIEWDLLEXP_DATA(type) WXIMPORT type
#define A2DDOCVIEWDLLEXP_CTORFN
#else //not making nor using DLL
#define A2DDOCVIEWDLLEXP
#define A2DDOCVIEWDLLEXP_DATA(type) type
#define A2DDOCVIEWDLLEXP_CTORFN
#endif //
#else // (defined(__WXMSW__) )not making nor using DLL
#define A2DDOCVIEWDLLEXP
#define A2DDOCVIEWDLLEXP_DATA(type) type
#define A2DDOCVIEWDLLEXP_CTORFN
#endif //(defined(__WXMSW__) )

#endif // __DOCVIEWDEF__
