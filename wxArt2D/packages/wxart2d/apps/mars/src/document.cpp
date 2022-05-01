/*! \file apps/mars/object.cpp
    \author Erik van der Wal

    Copyright: 2004 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: document.cpp,v 1.5 2006/12/13 21:43:21 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "document.h"
#include "page.h"

//----------------------------------------------------------------------------
// msDocument
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( msDocument, a2dCanvasDocument )

msDocument::msDocument()
    : a2dCanvasDocument()
{
}

msSymbolPage* msDocument::GetSymbolPage()
{
    msSymbolPage* page;
    a2dCanvasObject* root = GetDrawing()->GetRootObject();

    a2dCanvasObjectList* children = root->GetChildObjectList();
    a2dCanvasObjectList::iterator iter = children->begin();
    while ( iter != children->end() )
    {
        page = wxDynamicCast( ( *iter ).Get(), msSymbolPage );
        if ( page )
            return page;
        iter++;
    }
    // No symbolpage present yet, so create it.
    page = new msSymbolPage();
    root->Append( page );
    return page;
}

msScionPage* msDocument::GetScionPage()
{
    msScionPage* page;
    a2dCanvasObject* root = GetDrawing()->GetRootObject();
    a2dCanvasObjectList* children = root->GetChildObjectList();
    a2dCanvasObjectList::iterator iter = children->begin();
    while ( iter != children->end() )
    {
        page = wxDynamicCast( ( *iter ).Get(), msScionPage );
        if ( page )
            return page;
        iter++;
    }
    // No symbolpage present yet, so create it.
    page = new msScionPage();
    root->Append( page );
    return page;
}

msCircuitPage* msDocument::GetCircuitPage()
{
    msCircuitPage* page;
    a2dCanvasObject* root = GetDrawing()->GetRootObject();
    a2dCanvasObjectList* children = root->GetChildObjectList();
    a2dCanvasObjectList::iterator iter = children->begin();
    while ( iter != children->end() )
    {
        page = wxDynamicCast( ( *iter ).Get(), msCircuitPage );
        if ( page )
            return page;
        iter++;
    }
    // No circuitpage present yet, so create it.
    page = new msCircuitPage();
    root->Append( page );
    return page;
}

msLocallibPage* msDocument::GetLocallibPage()
{
    msLocallibPage* page;
    a2dCanvasObject* root = GetDrawing()->GetRootObject();
    a2dCanvasObjectList* children = root->GetChildObjectList();
    a2dCanvasObjectList::iterator iter = children->begin();
    while ( iter != children->end() )
    {
        page = wxDynamicCast( ( *iter ).Get(), msLocallibPage );
        if ( page )
            return page;
        iter++;
    }
    // No locallib page present yet, so create it.
    page = new msLocallibPage();
    root->Append( page );
    return page;
}

