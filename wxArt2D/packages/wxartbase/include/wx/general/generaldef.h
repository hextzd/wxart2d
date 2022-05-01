/*! \file wx/general/generaldef.h
    \brief the settings used by all other include files are stored here.
    \author Francesco Montorsi
    \date 12/6/2005

    Copyright: 2005 (c) Francesco Montorsi

    License: wxWidgets License

    RCS-ID: $Id: generaldef.h,v 1.14 2008/09/08 20:03:18 titato Exp $
*/

#ifndef __GENERALDEF__
#define __GENERALDEF__

#include "wxartbaseprivate.h"

#ifdef A2DGENERALMAKINGDLL
#define A2DGENERALDLLEXP WXEXPORT
#define A2DGENERALDLLEXP_DATA(type) WXEXPORT type
#define A2DGENERALDLLEXP_CTORFN
#elif defined(WXDOCVIEW_USINGDLL)
#define A2DGENERALDLLEXP WXIMPORT
#define A2DGENERALDLLEXP_DATA(type) WXIMPORT type
#define A2DGENERALDLLEXP_CTORFN
#else // not making nor using DLL
#define A2DGENERALDLLEXP
#define A2DGENERALDLLEXP_DATA(type) type
#define A2DGENERALDLLEXP_CTORFN
#endif

#if defined(__VISUALC__) && (_MSC_VER >= 1300)
#define CLASS_MEM_MANAGEMENT 1
#endif // VC++ >= 7

#undef CLASS_MEM_MANAGEMENT

// compiler-dependent warning shut down
#if defined(__VISUALC__)
// warning C4100: 'editmode' : unreferenced formal parameter
//#pragma warning (disable : 4100)      // not necessary anymore thanks to Kevin

#pragma warning (disable : 4786) // identifier was truncated to '255' characters in the debug information
#pragma warning (disable : 4284)
#pragma warning (disable : 4018)
//something with unicode characters in files
#pragma warning (disable : 4819)
#endif

/*!
    \defgroup general base classes used for many others
*/

/*!
    \defgroup property a2dObject holds property objects

    Properties can be added to a2dObject's. There is large range of properties, wrapping types available in wxWidgets
    and wxDocview.

    \ingroup general
*/

/*!
    \defgroup commands commands used in command processor

    The a2dCommandProcessor has commands submitted to it.
*/

/*!
    \defgroup commandid id used for submitting commands to a a2dCommandProcessor

    The a2dCommandProcessor can recieve commands by a2dCommandId, which is unique per command.
    Submitting a command, calls a function inside the command  processor.

    \ingroup commands

*/


/*!
    \defgroup events events in the wxDocview library
*/

/*!
    \defgroup eventid id of events

    \ingroup events
*/

/*!
    \defgroup eventhandlers handlers for event types in static event tables

    \ingroup events
*/

/*!
    \defgroup eventmanager Event Manager

    \ingroup events
*/

/*!
    \defgroup canvasobject a2dCanvasObject related objects
*/

/*!
    \defgroup drawer classes used for drawing with a2dDrawer2D
*/

/*!
    \defgroup tools Objects used for interactive manipulation.
*/

/*!
    \defgroup style objects used for styling a2dCanvasObject's

*/

/*!
    \defgroup global general objects can be reached global
*/

/*!
    \defgroup docalgo Algorithms on hierarchy in a2dDocument

    Algorithms on hierarchy in a2dDocument, using a2dWalkerIOHandler as base.
*/

/*!
    \defgroup fileio File IO

    \ingroup docalgo
*/

/*!
    \defgroup gdsfileio GDSII format File IO

    GDSII is a binary format used in the chip industry

    \ingroup fileio
*/

/*!
    \defgroup cvgfileio CVG File IO

    CVG is an XML format used as the main format for wxDocview.

    \ingroup fileio
*/

/*!
    \defgroup svgfileio SVG File IO

    SVG is an XML format.

    \ingroup fileio
*/

/*!
    \defgroup meta complex canvasobject's
*/

/*!
    \defgroup curve complex canvasobject's for drawing sets of curves
*/

/*!
    \defgroup menus templates to call commands

    a2dmenuItemId templates to call commands
*/

/*!
    \defgroup errorcodes id for error messages

    const a2dError id's for error messages
*/

#endif // __GENERALDEF__
