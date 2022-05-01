/*! \file apps/mars/object.h
    \author Erik van der Wal

    Copyright: 2004 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: document.h,v 1.5 2006/12/13 21:43:21 titato Exp $
*/

#ifndef __DOCUMENTH__
#define __DOCUMENTH__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/editor/candoc.h"

// Forward declarations
class msSymbolPage;
class msCircuitPage;
class msScionPage;
class msLocallibPage;
class msPage;

//#include<list>
//typedef std::list< msPage* > msPageList;
// Next line does not work (yet)
typedef a2dlist< msPage* > msPageList;

//! Page class for storage component
/*!
    A component has several pages.
    - Symbol page contains drawing on how to present this compoent at higher levels
    - Circuit page contains schematic on how to simulate this component
    - Scion page contains properties of the component
    - Locallib ??

    These pages are directly located under the root object.
*/
class msDocument : public a2dCanvasDocument
{
public:
    //! Constructor.
    msDocument();

    msSymbolPage* GetSymbolPage();
    msCircuitPage* GetCircuitPage();
    msScionPage* GetScionPage();
    msLocallibPage* GetLocallibPage();

private:
    DECLARE_DYNAMIC_CLASS( msDocument )
};

#endif // __DOCUMENTH__
