/*! \file canvas/src/canglob.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canglob.cpp,v 1.205 2009/09/03 20:09:53 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <math.h>
#include <limits.h>
#include <float.h>
#include "wx/canvas/canglob.h"
#include "wx/canvas/layerinf.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/drawing.h"
#include "wx/artbase/afmatrix.h"
#include "wx/artbase/drawer2d.h"
#include "wx/canvas/cameleon.h"
#include "wx/canvas/cansymboledit.h"
#include "wx/canvas/canvas.h"
#include "wx/tokenzr.h"

#include <wx/wfstream.h>

#include "wx/canvas/canprop.h"
#include "wx/canvas/wire.h"

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

//! default a2dPin width in pixels
#define PINWIDTH  9
//! default a2dHandle width in pixels
#define HANDLEWIDTH 7

IMPLEMENT_CLASS( a2dCanvasGlobal, a2dGlobal )
IMPLEMENT_CLASS( a2dCanvasObjectEvent, wxEvent )
IMPLEMENT_CLASS( a2dHandleMouseEvent, wxEvent )
IMPLEMENT_CLASS( a2dCanvasObjectMouseEvent, wxEvent )

DEFINE_EVENT_TYPE( wxEVT_CANVASOBJECT_MOUSE_EVENT )
DEFINE_EVENT_TYPE( wxEVT_CANVASOBJECT_ENTER_EVENT )
DEFINE_EVENT_TYPE( wxEVT_CANVASOBJECT_LEAVE_EVENT )
DEFINE_EVENT_TYPE( wxEVT_CANVASHANDLE_MOUSE_EVENT )
DEFINE_EVENT_TYPE( wxEVT_CANVASHANDLE_MOUSE_EVENT_ENTER )
DEFINE_EVENT_TYPE( wxEVT_CANVASHANDLE_MOUSE_EVENT_LEAVE )
DEFINE_EVENT_TYPE( wxEVT_CANVASOBJECT_RESIZE_EVENT )
DEFINE_EVENT_TYPE( wxEVT_CANVASOBJECT_POPUPMENU_EVENT )

//--------------------------------------------------------------------
// a2dLayerGroup
//--------------------------------------------------------------------
// wxColour <-> wxString utilities, used by wxConfig
wxString wxToString( const a2dLayerGroup&  group )
{
    wxString str;
    for( a2dLayerGroup::const_iterator iter = group.begin(); iter != group.end(); ++iter )
    {
        wxUint16 layer = *iter;
        str << layer << ' ';
    }
    return str;
}

bool wxFromString( const wxString& str, a2dLayerGroup*  group )
{
    if ( str.empty() )
    {
        *group = a2dLayerGroup();
        return true;
    }

    wxStringTokenizer tkz( str );
    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        long val;
        token.ToLong( &val );  
        group->push_back( val );
    }
    return true;
}

bool a2dLayerGroup::InGroup( wxUint16 layer )
{
    bool hasLayer = false;
    forEachIn( a2dLayerGroup, this )
    {
        wxUint16 layerOfGroup = *iter;
        // if one of the layers in the group is wxLAYER_ALL, that means all available layers in document
        if ( wxLAYER_ALL == layerOfGroup )
        {
            hasLayer = true;
            break;
        }
        if ( layer == layerOfGroup )
            hasLayer = true;
    }
    return hasLayer;
}

//----------------------------------------------------------------------------
// a2dCanvasObjectEvent
//----------------------------------------------------------------------------

a2dCanvasObjectEvent::a2dCanvasObjectEvent( a2dIterC* ic, wxEventType type, a2dCanvasObject* object, double x, double y, int id )
    : wxEvent( id, type )
{
    SetEventObject( ( wxObject* ) object );
    m_x = x;
    m_y = y;
    m_ic = ic;
}

a2dCanvasObjectEvent::a2dCanvasObjectEvent( a2dCanvasObject* object, const a2dBoundingBox& box, int id )
    : wxEvent( id, wxEVT_CANVASOBJECT_RESIZE_EVENT )
{
    SetEventObject( ( wxObject* ) object );
    m_box = box;
    m_ic = NULL;
}

a2dCanvasObjectEvent::a2dCanvasObjectEvent( const a2dCanvasObjectEvent& other )
    : wxEvent( other )
{
    m_x = other.m_x;
    m_y = other.m_y;
    m_ic = other.m_ic;
    m_box = other.m_box;
}

//----------------------------------------------------------------------------
// a2dCanvasObjectMouseEvent
//----------------------------------------------------------------------------

a2dCanvasObjectMouseEvent::a2dCanvasObjectMouseEvent( a2dIterC* ic, a2dCanvasObject* object, a2dHit& how, double x, double y, const wxMouseEvent& event )
    : a2dCanvasObjectEvent( ic, wxEVT_CANVASOBJECT_MOUSE_EVENT, object, x, y )
{
    m_mouseevent = event;
    m_how = how;
}

a2dCanvasObjectMouseEvent::a2dCanvasObjectMouseEvent( a2dIterC* ic, a2dCanvasObject* object, wxEventType type, double x, double y, const wxMouseEvent& event )
    : a2dCanvasObjectEvent( ic, type, object, x, y )
{
    m_mouseevent = event;
}


a2dCanvasObjectMouseEvent::a2dCanvasObjectMouseEvent( const a2dCanvasObjectMouseEvent& other )
    : a2dCanvasObjectEvent( other )
{
    m_mouseevent = other.m_mouseevent;
    m_how = other.m_how;
}

//----------------------------------------------------------------------------
// a2dHandleMouseEvent
//----------------------------------------------------------------------------

a2dHandleMouseEvent::a2dHandleMouseEvent( a2dIterC* ic, a2dHandle* handle, double x, double y, const wxMouseEvent& event, wxEventType type )
    : a2dCanvasObjectEvent( ic, type, ( a2dCanvasObject* ) handle, x, y )
{
    m_mouseevent = event;
}

a2dHandleMouseEvent::a2dHandleMouseEvent( const a2dHandleMouseEvent& other )
    : a2dCanvasObjectEvent( other )
{
    m_mouseevent = other.m_mouseevent;
}

//----------------------------------------------------------------------------
// a2dCanvasGlobal
//----------------------------------------------------------------------------

A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_NONE = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_ARROW = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_RIGHT_ARROW = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_BULLSEYE = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_CHAR = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_CROSS = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HAND = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_IBEAM = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_LEFT_BUTTON = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_MAGNIFIER = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_MIDDLE_BUTTON = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_NO_ENTRY = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_PAINT_BRUSH = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_PENCIL = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_POINT_LEFT = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_POINT_RIGHT = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_QUESTION_ARROW = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_RIGHT_BUTTON = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SIZENESW = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SIZENS = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SIZENWSE = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SIZEWE = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SIZING = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SPRAYCAN = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_WAIT = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_WATCH = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_BLANK = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_ARROWWAIT = wxGenNewId();

A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Select = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Select2 = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Zoom = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Drag = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Move = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Copy = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_DrawWire = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWire = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWireVertex = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWireDeleteVertex = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWireSegment = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWireSegmentHorizontal = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditWireSegmentVertical = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_WireSegmentInsert = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_Edit = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditVertex = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditDeleteVertex = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditSegment = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditSegmentHorizontal = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_EditSegmentVertical = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_SegmentInsert = wxGenNewId();

A2DCANVASDLLEXP extern const a2dImageId a2dBitmap_EditRectangleFill = wxGenNewId();

A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HandleLeftDown = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HandleDrag = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HandleMove = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HandleEnter = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_HandleLeave = wxGenNewId();

A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_CanvasObjectHit = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_CanvasObjectEnter = wxGenNewId();
A2DCANVASDLLEXP extern const a2dCursorId a2dCURSOR_CanvasObjectLeave = wxGenNewId();


a2dArtProvider::a2dArtProvider()
{
}

a2dArtProvider::~a2dArtProvider()
{
}

wxCursor LoadCursorFile( const wxString& filename )
{
    wxImage im;
    if ( im.LoadFile( filename ) )
    {
        im.LoadFile( filename );
        im.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 16 );
        im.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16 );
        return wxCursor( im );
    }
    else
    {
        wxString error;
        error.Printf( _T( "could not load cursor from file: %s" ), filename.c_str() );
        wxMessageBox( error, _T( "environment error" ), wxOK );
        return wxCURSOR_ARROW;
    }
}

wxCursor& a2dArtProvider::GetCursor( a2dCursorId id )
{
    if ( m_cursorsLoaded.find( id ) != m_cursorsLoaded.end() )
        return m_cursorsLoaded[id];

    static wxCursor cursor;

    if ( id == a2dCURSOR_NONE )
        cursor = wxCursor( wxCURSOR_NONE );
    if ( id == a2dCURSOR_ARROW )
        cursor = wxCursor( wxCURSOR_ARROW );
    if ( id == a2dCURSOR_RIGHT_ARROW )
        cursor = wxCursor( wxCURSOR_RIGHT_ARROW );
    if ( id == a2dCURSOR_BULLSEYE )
        cursor = wxCursor( wxCURSOR_BULLSEYE );
    if ( id == a2dCURSOR_CHAR )
        cursor = wxCursor( wxCURSOR_CHAR );
    if ( id == a2dCURSOR_CROSS )
        cursor = wxCursor( wxCURSOR_CROSS );
    if ( id == a2dCURSOR_HAND )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_IBEAM )
        cursor = wxCursor( wxCURSOR_IBEAM );
    if ( id == a2dCURSOR_LEFT_BUTTON )
        cursor = wxCursor( wxCURSOR_LEFT_BUTTON );
    if ( id == a2dCURSOR_MAGNIFIER )
        cursor = wxCursor( wxCURSOR_MAGNIFIER );
    if ( id == a2dCURSOR_MIDDLE_BUTTON )
        cursor = wxCursor( wxCURSOR_MIDDLE_BUTTON );
    if ( id == a2dCURSOR_NO_ENTRY )
        cursor = wxCursor( wxCURSOR_NO_ENTRY );
    if ( id == a2dCURSOR_PAINT_BRUSH )
        cursor = wxCursor( wxCURSOR_PAINT_BRUSH );
    if ( id == a2dCURSOR_PENCIL )
        cursor = wxCursor( wxCURSOR_PENCIL );
    if ( id == a2dCURSOR_POINT_LEFT )
        cursor = wxCursor( wxCURSOR_POINT_LEFT );
    if ( id == a2dCURSOR_POINT_RIGHT )
        cursor = wxCursor( wxCURSOR_POINT_RIGHT );
    if ( id == a2dCURSOR_QUESTION_ARROW )
        cursor = wxCursor( wxCURSOR_QUESTION_ARROW );
    if ( id == a2dCURSOR_RIGHT_BUTTON )
        cursor = wxCursor( wxCURSOR_RIGHT_BUTTON );
    if ( id == a2dCURSOR_SIZENESW )
        cursor = wxCursor( wxCURSOR_SIZENESW );
    if ( id == a2dCURSOR_SIZENS )
        cursor = wxCursor( wxCURSOR_SIZENS );
    if ( id == a2dCURSOR_SIZENWSE )
        cursor = wxCursor( wxCURSOR_SIZENWSE );
    if ( id == a2dCURSOR_SIZEWE )
        cursor = wxCursor( wxCURSOR_SIZEWE );
    if ( id == a2dCURSOR_SIZING )
        cursor = wxCursor( wxCURSOR_SIZING );
    if ( id == a2dCURSOR_SPRAYCAN )
        cursor = wxCursor( wxCURSOR_SPRAYCAN );
    if ( id == a2dCURSOR_WAIT )
        cursor = wxCursor( wxCURSOR_WAIT );
    if ( id == a2dCURSOR_WATCH )
        cursor = wxCursor( wxCURSOR_WATCH );
    if ( id == a2dCURSOR_BLANK )
        cursor = wxCursor( wxCURSOR_BLANK );
    if ( id == a2dCURSOR_ARROWWAIT )
        cursor = wxCursor( wxCURSOR_ARROWWAIT );

    if ( id == a2dCURSOR_Select )
        cursor = wxCursor( wxCURSOR_CROSS );
    if ( id == a2dCURSOR_Select2 )
        cursor = wxCursor( wxCURSOR_CROSS );
    if ( id == a2dCURSOR_Zoom )
        cursor = wxCursor( wxCURSOR_MAGNIFIER );
    if ( id == a2dCURSOR_Drag )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_Move )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_Copy )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_DrawWire )
        cursor = wxCursor( wxCURSOR_PENCIL );
    if ( id == a2dCURSOR_EditWire )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_EditWireVertex )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_EditWireDeleteVertex )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_EditWireSegment )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_EditWireSegmentHorizontal )
        cursor = wxCursor( wxCURSOR_SIZENS );
    if ( id == a2dCURSOR_EditWireSegmentVertical )
        cursor = wxCursor( wxCURSOR_SIZEWE );
    if ( id == a2dCURSOR_WireSegmentInsert )
        cursor = wxCursor( wxCURSOR_CROSS );
    if ( id == a2dCURSOR_Edit )
        cursor = wxCursor( wxCURSOR_SIZEWE );
    if ( id == a2dCURSOR_EditVertex )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_EditSegment )
        cursor = wxCursor( wxCURSOR_SIZENS );
    if ( id == a2dCURSOR_EditSegmentHorizontal )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_EditSegmentVertical )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_SegmentInsert )
        cursor = wxCursor( wxCURSOR_HAND );

    wxString artroot = a2dGeneralGlobals->GetWxArt2DArtVar();
    if ( artroot.IsEmpty() )
        return cursor;
    else
        artroot += wxT( "cursors/" );

    if ( id == a2dCURSOR_Select )
        cursor = wxCursor( wxCURSOR_CROSS );
    if ( id == a2dCURSOR_Select2 )
        cursor = wxCursor( wxCURSOR_CROSS );
    if ( id == a2dCURSOR_Zoom )
        cursor = wxCursor( wxCURSOR_MAGNIFIER );
    if ( id == a2dCURSOR_Drag )
        cursor = LoadCursorFile( artroot + wxT( "drag_cur.cur" ) );
    if ( id == a2dCURSOR_Move )
        cursor = LoadCursorFile( artroot + wxT( "move_cur.cur" ) );
    if ( id == a2dCURSOR_Copy )
        cursor = LoadCursorFile( artroot + wxT( "copy_cur.cur" ) );
    if ( id == a2dCURSOR_DrawWire )
        cursor = LoadCursorFile( artroot + wxT( "start_wire.cur" ) );
    if ( id == a2dCURSOR_EditWire )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_EditWireDeleteVertex )
        cursor = LoadCursorFile( artroot + wxT( "delete_vertex.cur" ) );
    if ( id == a2dCURSOR_EditWireVertex )
        cursor = LoadCursorFile( artroot + wxT( "drag_vertex.cur" ) );
    if ( id == a2dCURSOR_EditWireSegment )
        cursor = LoadCursorFile( artroot + wxT( "drag_seg.cur" ) );
    if ( id == a2dCURSOR_EditWireSegmentHorizontal )
        cursor = LoadCursorFile( artroot + wxT( "drag_seg_horz.cur" ) );
    if ( id == a2dCURSOR_EditWireSegmentVertical )
        cursor = LoadCursorFile( artroot + wxT( "drag_seg_vert.cur" ) );
    if ( id == a2dCURSOR_WireSegmentInsert )
        cursor = LoadCursorFile( artroot + wxT( "insert_seg.cur" ) );
    if ( id == a2dCURSOR_Edit )
        cursor = LoadCursorFile( artroot + wxT( "edit_cur.cur" ) );
    if ( id == a2dCURSOR_EditDeleteVertex )
        cursor = LoadCursorFile( artroot + wxT( "delete_vertex.cur" ) );
    if ( id == a2dCURSOR_EditVertex )
        cursor = LoadCursorFile( artroot + wxT( "edit_cur.cur" ) );
    if ( id == a2dCURSOR_EditSegment )
        cursor = wxCursor( wxCURSOR_SIZENS );
    if ( id == a2dCURSOR_EditSegmentHorizontal )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_EditSegmentVertical )
        cursor = wxCursor( wxCURSOR_HAND );
    if ( id == a2dCURSOR_SegmentInsert )
        cursor = wxCursor( wxCURSOR_HAND );

    if ( id == a2dCURSOR_HandleLeftDown )
        cursor = wxCursor( wxCURSOR_CROSS );
    if ( id == a2dCURSOR_HandleDrag )
        cursor = wxCursor( wxCURSOR_CROSS );
    if ( id == a2dCURSOR_HandleMove )
        cursor = wxCursor( wxCURSOR_CROSS );

    if ( id == a2dCURSOR_HandleEnter )
        cursor = wxCursor( wxCURSOR_CROSS );
    if ( id == a2dCURSOR_HandleLeave )
        cursor = wxCursor( wxCURSOR_ARROW );
        //cursor = LoadCursorFile( artroot + wxT( "drag_vertex.cur" ) );

    if ( id == a2dCURSOR_CanvasObjectHit )
        cursor = wxCursor( wxCURSOR_HAND );
        //cursor = LoadCursorFile( artroot + wxT( "drag_vertex.cur" ) );
    if ( id == a2dCURSOR_CanvasObjectEnter )
        cursor = LoadCursorFile( artroot + wxT( "edit_cur.cur" ) );
    if ( id == a2dCURSOR_CanvasObjectLeave )
        cursor = wxCursor( wxCURSOR_ARROW );
        //cursor = LoadCursorFile( artroot + wxT( "edit_cur.cur" ) );

    /* but rotation in wxWidgets is not around a point, to bad.
        if ( id == a2dCURSOR_Move )
        {
            static char down_bits[] = { 255, 255, 255, 255, 31,
              255, 255, 255, 31, 255, 255, 255, 31, 255, 255, 255,
              31, 255, 255, 255, 31, 255, 255, 255, 31, 255, 255,
              255, 31, 255, 255, 255, 31, 255, 255, 255, 25, 243,
              255, 255, 19, 249, 255, 255, 7, 252, 255, 255, 15, 254,
              255, 255, 31, 255, 255, 255, 191, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255 };

            static char down_mask[] = { 240, 1, 0, 0, 240, 1,
              0, 0, 240, 1, 0, 0, 240, 1, 0, 0, 240, 1, 0, 0, 240, 1,
              0, 0, 240, 1, 0, 0, 240, 1, 0, 0, 255, 31, 0, 0, 255,
              31, 0, 0, 254, 15, 0, 0, 252, 7, 0, 0, 248, 3, 0, 0,
              240, 1, 0, 0, 224, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0 };

            wxBitmap down_bitmap(down_bits, 32, 32);
            wxBitmap down_mask_bitmap(down_mask, 32, 32);

            down_bitmap.SetMask(new wxMask(down_mask_bitmap));
            wxImage down_image = down_bitmap.ConvertToImage();
            down_image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
            down_image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
            //down_image =  down_image.Rotate( 45, wxPoint(0,0));
            cursor = wxCursor(down_image);
        }
    */
    m_cursorsLoaded[id] = cursor;
    return cursor;
}

wxBitmap& a2dArtProvider::GetImage( a2dImageId id )
{
    if ( m_imageLoaded.find( id ) != m_imageLoaded.end() )
        return m_imageLoaded[id];

    static wxBitmap bitmap;

    if ( id == a2dBitmap_EditRectangleFill )
    {
        static const char* bits[] =
        {
            "8 8 3 1 0 0",
            "  c #000000",
            "! c #FFFFFF",
            "# c #FFFFFF",
            //"# c none",
            " # # # #",
            "# # # # ",
            " # # # #",
            "# # # # ",
            " # # # #",
            "# # # # ",
            " # # # #",
            "# # # # "
        };
        bitmap = wxBitmap( bits );
        static const char mask_bits[] =
        {
            0x13, 0x00, 0x15, 0x00,
            char(0xFF), char(0xFF), char(0xFF), char(0xFF), 
            char(0xFF), char(0xFF), char(0x00), char(0x80),
            0x00, 0x60
        };
        wxBitmap mask_bitmap( mask_bits, 8, 8, 1 );
        bitmap.SetMask( new wxMask( mask_bitmap ) );
    }


    m_imageLoaded[id] = bitmap;
    return bitmap;
}

//----------------------------------------------------------------------------
// a2dHabitat
//----------------------------------------------------------------------------
const a2dSignal a2dHabitat::sig_changedFill = wxNewId(); 
const a2dSignal a2dHabitat::sig_changedStroke = wxNewId(); 
const a2dSignal a2dHabitat::sig_changedFont = wxNewId(); 
const a2dSignal a2dHabitat::sig_changedLayer = wxNewId(); 
const a2dSignal a2dHabitat::sig_changedTarget = wxNewId(); 
const a2dSignal a2dHabitat::sig_SetPathType = wxNewId(); 
const a2dSignal a2dHabitat::sig_SetContourWidth = wxNewId(); 
const a2dSignal a2dHabitat::sig_changedTextFill = wxNewId(); 
const a2dSignal a2dHabitat::sig_changedTextStroke = wxNewId(); 
const a2dSignal a2dHabitat::sig_changedTextFont = wxNewId(); 

a2dHabitat::a2dHabitat()
{
	Init();
}

a2dHabitat::~a2dHabitat()
{}

void a2dHabitat::Init( a2dLayers* layers )
{
    m_default_acc = 1e-6;

    m_propertytext = ( a2dText* ) NULL;

    if ( ! layers )
    {
        m_layersetup = new a2dLayers();

        // make sure  wxLAYER_DEFAULT is available
        m_layersetup->AddDefaultLayers();

        for ( wxUint16 i = 1; i < 20; i++ )
        {
            wxString buf;
            buf.Printf( wxT( "layer %d" ), i );
            a2dLayerInfo* n = new a2dLayerInfo( i, buf );
            m_layersetup->Append( n );
            n->SetOrder( i );
            n->SetInMapping( i );
            n->SetOutMapping( i );
        }
        m_layersetup->SetPending( true );
    }
    else
    {
        m_layersetup = wxStaticCastNull( layers->Clone( clone_deep ), a2dLayers );
        m_layersetup->SetPending( true );
    }

    m_SelectStroke = a2dStroke( wxColour( 255, 255, 0 ), 2, a2dSTROKE_SOLID );
    m_SelectFill = *a2dTRANSPARENT_FILL;

    m_Select2Stroke = a2dStroke( wxColour( 255, 255, 0 ), 2, a2dSTROKE_SOLID );
    m_Select2Fill = *a2dTRANSPARENT_FILL;

    m_DefaultStroke = a2dStroke( wxColour( 0, 0, 0 ), 0, a2dSTROKE_SOLID );
    m_DefaultFill = *a2dTRANSPARENT_FILL;

    m_HighLightStroke = m_SelectStroke;
    m_HighLightFill = m_SelectFill;

    m_SelectDrawStyle = RenderWIREFRAME_SELECT_INVERT;
    m_documentDrawStyle = RenderLAYERED  | RenderWIREFRAME_SELECT2;

    m_hitmargin_device = 2;
    m_hitmargin_world = 2;
    m_copymindistance = 5;
    // This is related to accuracy of numbers written to text files. 
    m_coordinateepsilon = 1e-3;
    m_reverse_order = false;
    m_oneLine = false;
    m_connectionGenerator = new a2dConnectionGenerator();
//  a2dGeneralGlobals->SetDocviewCommandProcessor(new a2dCentralCanvasCommandProcessor);

    m_defaultUnitsScale = 1;
    m_units_accuracy = 0.001;
    m_units_measure = "non";
    m_units = "non";
    m_normalize_scale = 1;

    m_layer = wxLAYER_DEFAULT;
    m_target = wxLAYER_DEFAULT;
    m_selectedOnlyA = false;
    m_selectedOnlyB = false;
    m_clearTarget = false;
    m_CORRECTIONFACTOR = 500.0e-6;  // correct the polygons by this number (just something)
    m_CORRECTIONABER   = 1.0e-6;    // the accuracy for the rounded shapes used in correction
    m_ROUNDFACTOR      = 1.5;    // when will we round the correction shape to a circle
    m_SMOOTHABER       = 0.001e-6;   // accuracy when smoothing a polygon
    m_MAXLINEMERGE     = 1000.0e-6; // leave as is, segments of this length in smoothen
    m_MARGE = 0.1e-6;
    m_GRID = 100;
    m_DGRID = 100;
    m_AberPolyToArc = 1;
    m_AberArcToPoly = 1;
    m_distancePoints = 1;
    m_AberPolyToArc = m_AberPolyToArc / m_defaultUnitsScale;
    m_AberArcToPoly = m_AberArcToPoly / m_defaultUnitsScale;
    m_radiusMin = m_AberPolyToArc / m_defaultUnitsScale;
    m_radiusMax = m_AberArcToPoly / m_defaultUnitsScale;
    m_distancePoints = m_distancePoints / m_defaultUnitsScale;
    m_atVertexPoints = false;
    m_small = 1 / m_defaultUnitsScale;

    m_gridX = 100;
    m_gridY = 100;

    m_RelativeStartX = 0;
    m_RelativeStartY = 0;

    m_WINDINGRULE = true;
    m_SmoothAber = 0.1e-6;
    m_LinkHoles = false;
    m_currentContourWidth =  0.0e-6;
    m_pathtype = a2dPATH_END_SQAURE;

    m_font = *a2dDEFAULT_CANVASFONT;
    m_textTemplateObject = new a2dText( _T( "" ), 0,0, m_font, 0, false );
    m_textOwnStyle = true;

    m_spline = false;
    m_lastX = m_lastY = 0;

    m_layer = wxLAYER_DEFAULT;
    m_target = wxLAYER_DEFAULT;

    m_currentfill = a2dFill( wxColour( 0, 149, 25 ), a2dFILL_HORIZONTAL_HATCH );
    m_currentstroke = a2dStroke( wxColour( 0, 255, 0 ), 0, a2dSTROKE_SOLID );

    m_currentEnd = 0;
    m_currentBegin = 0;
    m_currentXscale = m_currentYscale = 1;

    m_spline = false;

    m_lastX = m_lastY = 0;
    m_keySaveTextAsPolygon = false;
    m_gdsiiSaveTextAsPolygon = false;
    m_keySaveFromView = false;
    m_gdsiiSaveFromView = false;

    m_symbolLayer = wxLAYER_DEFAULT;

    m_defPinSize = PINWIDTH;
    m_defHandleSize = HANDLEWIDTH;
    m_objectGridSize = 1;
    m_portDistance = 3;
    m_symbolSize = 30;

    m_drawingTemplate = new a2dDrawing( "", this );
    m_drawingTemplateSymbol = m_drawingTemplate;
    m_drawingTemplateDiagram = m_drawingTemplate;
    m_drawingTemplateBuildIn = m_drawingTemplate;
    m_drawingTemplateGui = m_drawingTemplate;
}

#if wxUSE_CONFIG
void a2dHabitat::Load( wxConfigBase& config )
{
    long valLayer;
    a2dLayerGroup group;
    wxString value;
    bool boolval;
    double valdouble;
    if ( config.Read( "Habitat/DefaultUnitsScale", &valdouble ) )
        SetDefaultUnitsScale( valdouble );

    if ( config.Read( "Habitat/UnitsAccuracy", &valdouble ) )
        SetUnitsAccuracy( valdouble );

    if ( config.Read( "Habitat/Units", &value ) )
        SetUnits( value );

    config.Read( "Habitat/GroupA", &group );
    SetGroupA( group );
    group.clear();
    config.Read( "Habitat/GroupB", &group );
    SetGroupB( group );

    if ( config.Read( "Habitat/Target", &valLayer ) )
        SetTarget( valLayer );

    if ( config.Read( "Habitat/SelectedOnlyA", &boolval ) )
        SetSelectedOnlyA( boolval );
    if ( config.Read( "Habitat/SelectedOnlyB", &boolval ) )
        SetSelectedOnlyB( boolval );

    config.Read( "Habitat/AberPolyToArc", &value );
    if ( !value.empty() )
        SetAberPolyToArc( value );
    config.Read( "Habitat/AberArcToPoly", &value );
    if ( !value.empty() )
        SetAberArcToPoly( value );
    config.Read( "Habitat/RadiusMin", &value );
    if ( !value.empty() )
        SetRadiusMin( value );
    config.Read( "Habitat/RadiusMax", &value );
    if ( !value.empty() )
        SetRadiusMax( value );
    config.Read( "Habitat/Small", &value );
    if ( !value.empty() )
        SetSmall( value );
    config.Read( "Habitat/BooleanEngineMarge", &value );
    if ( !value.empty() )
        SetBooleanEngineMarge( value );
    long lvalue;
    if ( config.Read( "Habitat/BooleanEngineGrid", &lvalue ) )
        SetBooleanEngineGrid( lvalue );
    if ( config.Read( "Habitat/BooleanEngineDGrid", &lvalue ) )
        SetBooleanEngineDGrid( lvalue );

    config.Read( "Habitat/BooleanEngineCorrectionAber", &value );
    if ( !value.empty() )
        SetBooleanEngineCorrectionAber( value );
    config.Read( "Habitat/BooleanEngineCorrectionFactor", &value );
    if ( !value.empty() )
        SetBooleanEngineCorrectionFactor( value );
    config.Read( "Habitat/BooleanEngineMaxlinemerge", &value );
    if ( !value.empty() )
        SetBooleanEngineMaxlinemerge( value );
    if ( config.Read( "Habitat/BooleanEngineWindingRule", &boolval ) )
        SetBooleanEngineWindingRule( boolval );
    config.Read( "Habitat/BooleanEngineRoundfactor", &lvalue );
    if ( lvalue )
        SetBooleanEngineRoundfactor( lvalue );
    config.Read( "Habitat/BooleanEngineSmoothAber", &value );
    if ( !value.empty() )
        SetBooleanEngineSmoothAber( value );
    if ( config.Read( "Habitat/BooleanEngineLinkHoles", &boolval ) )
        SetBooleanEngineLinkHoles( boolval );

    //config.Read( "Trial", &value );
}

void a2dHabitat::Save( wxConfigBase& config )
{
    /*
    a2dDocumentStringOutputStream mem;
  //a2dCNumericLocaleSetter locSetter;


    mem.str("");

    a2dIOHandlerCVGOut CVGwriter;
    CVGwriter.Save( mem, m_currentfill );
    wxSetlocale( LC_NUMERIC, oldLocale );
    mem.seekp(0);
    wxString strw = mem.str();
    config.Write( "Trial", strw );
    */
    config.Write( "Habitat/DefaultUnitsScale", GetDefaultUnitsScale() );
    config.Write( "Habitat/UnitsAccuracy", GetUnitsAccuracy() );
    config.Write( "Habitat/Units", GetUnits() );

    config.Write( "Habitat/GroupA", GetGroupA() );
    config.Write( "Habitat/GroupB", GetGroupB() );
    config.Write( "Habitat/Target", GetTarget() );
    config.Write( "Habitat/SelectedOnlyA", GetSelectedOnlyA() );
    config.Write( "Habitat/SelectedOnlyB", GetSelectedOnlyB() );
    config.Write( "Habitat/AberPolyToArc", GetAberPolyToArc().GetValueString() );
    config.Write( "Habitat/AberArcToPoly", GetAberArcToPoly().GetValueString() );
    config.Write( "Habitat/RadiusMin", GetRadiusMin().GetValueString() );
    config.Write( "Habitat/RadiusMax", GetRadiusMax().GetValueString() );
    config.Write( "Habitat/Small", GetSmall().GetValueString() );
    config.Write( "Habitat/BooleanEngineMarge", GetBooleanEngineMarge().GetValueString() );
    config.Write( "Habitat/BooleanEngineGrid", GetBooleanEngineGrid() );
    config.Write( "Habitat/BooleanEngineDGrid", GetBooleanEngineDGrid() );
    config.Write( "Habitat/BooleanEngineCorrectionAber", GetBooleanEngineCorrectionAber().GetValueString() );
    config.Write( "Habitat/BooleanEngineCorrectionFactor", GetBooleanEngineCorrectionFactor().GetValueString() );
    config.Write( "Habitat/BooleanEngineMaxlinemerge", GetBooleanEngineMaxlinemerge().GetValueString() );
    config.Write( "Habitat/BooleanEngineWindingRule", GetBooleanEngineWindingRule() );
    config.Write( "Habitat/BooleanEngineRoundfactor", GetBooleanEngineRoundfactor() );
    config.Write( "Habitat/BooleanEngineSmoothAber", GetBooleanEngineSmoothAber().GetValueString() );
    config.Write( "Habitat/BooleanEngineLinkHoles", GetBooleanEngineLinkHoles() );
}
#endif // wxUSE_CONFIG

bool a2dHabitat::LoadLayers( const wxString& filename )
{
    wxString foundfile = filename;

    if ( m_layersetup )
        m_layersetup->ReleaseChildObjects();
    else
    {
        m_layersetup = new a2dLayers();
    }

    foundfile = a2dCanvasGlobals->GetLayersPathList().FindValidPath( foundfile );
    if ( foundfile.IsEmpty() )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_NotSpecified, _( "Path to layers settings file %s not found in path %s" ), filename.c_str(), a2dCanvasGlobals->GetLayersPathList().GetAsString().c_str() );
        return false;
    }

	bool ret = m_layersetup->LoadLayers( foundfile );
    return ret;
}

void a2dHabitat::SetPin( a2dPin* newpin )
{
    m_defPin = newpin;
}

a2dPin* a2dHabitat::GetPin()
{
    if ( !m_defPin )
    {
        m_defPin = new a2dPin( NULL, wxT( "global" ), a2dPinClass::Standard, 0, 0, 0, m_defPinSize, m_defPinSize );
        m_defPin->SetFill( *wxGREEN );
        m_defPin->SetStroke( wxColour( 66, 159, 235 ), 1 );
    }
    return m_defPin;
}

void a2dHabitat::SetPinCannotConnect( a2dPin* newpin )
{
    m_defCannotConnectPin = newpin;
}

a2dPin* a2dHabitat::GetPinCannotConnect()
{
    if ( !m_defCannotConnectPin )
    {
        m_defCannotConnectPin = new a2dPin( NULL, wxT( "global" ), a2dPinClass::Standard, 0, 0, 0, m_defPinSize / 2 + 1, m_defPinSize / 2 + 1 );
        m_defCannotConnectPin->SetFill( wxColour( 255, 128, 128 ) );
        m_defCannotConnectPin->SetStroke( *wxRED, 1 );
    }
    return m_defCannotConnectPin;
}

void a2dHabitat::SetPinCanConnect( a2dPin* newpin )
{
    m_defCanConnectPin = newpin;
}

a2dPin* a2dHabitat::GetPinCanConnect()
{
    if ( !m_defCanConnectPin )
    {
        m_defCanConnectPin = new a2dPin( NULL, wxT( "global" ), a2dPinClass::Standard, 0, 0, 0, 2 * m_defPinSize + 1, 2 * m_defPinSize + 1 );
        m_defCanConnectPin->SetFill( wxColour( 128, 255, 128 ) );
        m_defCanConnectPin->SetStroke( *wxGREEN, 1 );
    }
    return m_defCanConnectPin;
}

void a2dHabitat::SetHandle( a2dHandle* newhandle )
{
    m_defHandle = newhandle;
}

a2dHandle* a2dHabitat::GetHandle()
{

    if ( !m_defHandle )
    {
        m_defHandle = new a2dHandle( NULL, 0, 0, wxT( "global" ), m_defHandleSize, m_defHandleSize );
        m_defHandle->SetFill( *wxCYAN );
        m_defHandle->SetStroke( wxColour( 66, 159, 235 ), 1 );
    }
    return m_defHandle;
}

a2dText* a2dHabitat::GetPropertyText()
{
    if ( !m_propertytext )
    {
        m_propertytext = new a2dText( wxT( "dummy" ), 0, 0, a2dFont( 80.0, wxFONTFAMILY_SWISS ) );
        m_propertytext->SetFill( *wxCYAN );
        m_propertytext->SetStroke( wxColour( 66, 159, 235 ), 0.0 );
    }
    return m_propertytext;
}

double a2dHabitat::ACCUR()
{
    return m_default_acc / 10000;
}

void a2dHabitat::SetAberPolyToArc( const a2dDoMu& aber )
{
    m_AberPolyToArc = aber;
    a2dGlobals->SetAberPolyToArc( double(m_AberPolyToArc ) );/// doc->GetUnitsScale() );
}

void a2dHabitat::SetAberArcToPoly( const a2dDoMu& aber )
{
    m_AberArcToPoly = aber;
    a2dGlobals->SetAberArcToPoly( double( m_AberArcToPoly ) );/// doc->GetUnitsScale() );
}

void a2dHabitat::SetSmall( const a2dDoMu& smallest )
{
    m_small = smallest;
}

bool a2dHabitat::GdsIoSaveTextAsPolygon( bool onOff )
{
    m_gdsiiSaveTextAsPolygon = onOff;
    return true;
}

bool a2dHabitat::KeyIoSaveTextAsPolygon( bool onOff )
{
    m_keySaveTextAsPolygon = onOff;
    return true;
}

bool a2dHabitat::GetGdsIoSaveTextAsPolygon() const
{
    return m_gdsiiSaveTextAsPolygon;
}

bool a2dHabitat::GetKeyIoSaveTextAsPolygon() const
{
    return m_keySaveTextAsPolygon;
}

a2dStroke& a2dHabitat::GetHighLightStroke()
{
    return m_HighLightStroke;
}

a2dFill& a2dHabitat::GetHighLightFill()
{
    return m_HighLightFill;
}

void a2dHabitat::SetHighLightStroke( const a2dStroke& stroke )
{
    m_HighLightStroke = stroke;
}

void a2dHabitat::SetHighLightFill( const a2dFill& fill )
{
    m_HighLightFill = fill;
}

a2dStroke& a2dHabitat::GetSelectStroke()
{
    return m_SelectStroke;
}

a2dFill& a2dHabitat::GetSelectFill()
{
    return m_SelectFill;
}

void a2dHabitat::SetSelectStroke( const a2dStroke& stroke )
{
    m_SelectStroke = stroke;
}

void a2dHabitat::SetSelectFill( const a2dFill& fill )
{
    m_SelectFill = fill;
}

a2dStroke& a2dHabitat::GetSelect2Stroke()
{
    return m_Select2Stroke;
}

a2dFill& a2dHabitat::GetSelect2Fill()
{
    return m_Select2Fill;
}

void a2dHabitat::SetSelect2Stroke( const a2dStroke& stroke )
{
    m_Select2Stroke = stroke;
}

void a2dHabitat::SetSelect2Fill( const a2dFill& fill )
{
    m_Select2Fill = fill;
}

a2dStroke& a2dHabitat::GetDefaultStroke()
{
    return m_DefaultStroke;
}

a2dFill& a2dHabitat::GetDefaultFill()
{
    return m_DefaultFill;
}

void a2dHabitat::SetDefaultStroke( const a2dStroke& stroke )
{
    m_DefaultStroke = stroke;
}

void a2dHabitat::SetDefaultFill( const a2dFill& fill )
{
    m_DefaultFill = fill;
}

a2dRestrictionEngine* a2dHabitat::GetRestrictionEngine()
{
    return m_restrictionEngine;
}

void a2dHabitat::SetDisplayAberration( double aber ) 
{ 
    a2dGlobals->SetDisplayAberration( aber ); 
}

double a2dHabitat::GetDisplayAberration() const 
{ 
    return a2dGlobals->GetDisplayAberration(); 
}

void a2dHabitat::SetStroke( const a2dStroke& stroke )
{
    m_currentstroke = stroke;
    a2dComEvent changed( this, sig_changedStroke );
    a2dCanvasGlobals->ProcessEvent( changed );
}

void a2dHabitat::SetStroke( const unsigned char red, const unsigned char green, const unsigned char blue )
{
    a2dStroke col = GetStroke();
    col.SetColour( wxColour( red, green, blue ) );
    m_currentstroke = col;
    a2dComEvent changed( this, sig_changedStroke );
    a2dCanvasGlobals->ProcessEvent( changed );
}

void a2dHabitat::SetFill( const a2dFill& fill )
{
    m_currentfill = fill;
    a2dComEvent changed( this, sig_changedFill );
    a2dCanvasGlobals->ProcessEvent( changed );
}

void a2dHabitat::SetFill( const unsigned char red, const unsigned char green, const unsigned char blue )
{
    a2dFill col = GetFill();
    col.SetColour( wxColour( red, green, blue ) );
    m_currentfill = col;
    a2dComEvent changed( this, sig_changedFill );
    a2dCanvasGlobals->ProcessEvent( changed );
}

void a2dHabitat::SetFont( const a2dFont& font )
{
    m_font = font;
    a2dComEvent changed( this, sig_changedFont );
    a2dCanvasGlobals->ProcessEvent( changed );
}


void a2dHabitat::SetTextStroke( const a2dStroke& stroke )
{
    if ( m_textOwnStyle )
    {
        m_textTemplateObject->SetStroke( stroke );
        a2dComEvent changed( this, sig_changedTextStroke );
        a2dCanvasGlobals->ProcessEvent( changed );
    }
    else
    {
        m_textTemplateObject->SetStroke( stroke );
        SetStroke( stroke );
    }
}

void a2dHabitat::SetTextFill( const a2dFill& fill )
{
    if ( m_textOwnStyle )
    {
        m_textTemplateObject->SetFill( fill );
        a2dComEvent changed( this, sig_changedTextFill );
        a2dCanvasGlobals->ProcessEvent( changed );
    }
    else
    {
        m_textTemplateObject->SetFill( fill );
        SetFill( fill );
    }
}

void a2dHabitat::SetTextFont( const a2dFont& font )
{
    if ( m_textOwnStyle )
    {
        m_textTemplateObject->SetFont( font );
        a2dComEvent changed( this, m_textOwnStyle ? sig_changedTextFont : sig_changedTextFont );
        a2dCanvasGlobals->ProcessEvent( changed );
    }
    else
    {
        m_textTemplateObject->SetFont( font );
        SetFont( font );
    }
}


a2dStroke a2dHabitat::GetTextStroke() const 
{ 
    if ( m_textOwnStyle )
        return m_textTemplateObject->GetStroke(); 
    return GetStroke();
}

a2dFill a2dHabitat::GetTextFill() const 
{ 
    if ( m_textOwnStyle )
        return m_textTemplateObject->GetFill(); 
    return GetFill();
}

a2dFont a2dHabitat::GetTextFont() const 
{ 
    if ( m_textOwnStyle )
        return m_textTemplateObject->GetFont(); 
    return GetFont();
}

void a2dHabitat::SetPathType( a2dPATH_END_TYPE pathtype )
{ 
    m_pathtype = pathtype; 
    a2dComEvent changed( this, sig_SetPathType );
    a2dCanvasGlobals->ProcessEvent( changed );
}

void a2dHabitat::SetContourWidth( const a2dDoMu& currentContourWidth )
{
    m_currentContourWidth = currentContourWidth;
    a2dComEvent changed( this, sig_SetContourWidth );
    a2dCanvasGlobals->ProcessEvent( changed );
}

void a2dHabitat::SetLayerSetup( a2dLayers* layersetup )
{
    m_layersetup = layersetup;
};

void a2dHabitat::SetLayer( wxUint16 layer, bool setStyleOfLayer )
{
    m_layer = layer;
    if ( setStyleOfLayer )
    {
        m_currentfill = *a2dNullFILL;
        m_currentstroke = *a2dNullSTROKE;
    }
    a2dComEvent changed( this, sig_changedLayer );
    a2dCanvasGlobals->ProcessEvent( changed );
}

void a2dHabitat::SetTarget( wxUint16 layer )
{
    m_target = layer;
    a2dComEvent changed( this, sig_changedTarget );
    a2dCanvasGlobals->ProcessEvent( changed );
}

void a2dHabitat::SetGridX( double gridX )
{
    m_gridX = gridX;
}

void a2dHabitat::SetGridY( double gridY )
{
    m_gridY = gridY;
}

void a2dHabitat::SetDrawingTemplate( a2dDrawing* drawingTemplate) { m_drawingTemplate = drawingTemplate; }

a2dDrawing* a2dHabitat::GetDrawingTemplate() const { return m_drawingTemplate; }

void a2dHabitat::SetSymbolDrawingTemplate( a2dDrawing* drawingTemplate) { m_drawingTemplateSymbol = drawingTemplate; }

a2dDrawing* a2dHabitat::GetSymbolDrawingTemplate() const { return m_drawingTemplateSymbol; }

void a2dHabitat::SetBuildInDrawingTemplate( a2dDrawing* drawingTemplate) { m_drawingTemplateBuildIn = drawingTemplate; }

a2dDrawing* a2dHabitat::GetBuildInDrawingTemplate() const { return m_drawingTemplateBuildIn; }

void a2dHabitat::SetDiagramDrawingTemplate( a2dDrawing* drawingTemplate) { m_drawingTemplateDiagram = drawingTemplate; }

a2dDrawing* a2dHabitat::GetDiagramDrawingTemplate() const { return m_drawingTemplateDiagram; }

void a2dHabitat::SetGuiDrawingTemplate( a2dDrawing* drawingTemplate) { m_drawingTemplateGui = drawingTemplate; }

a2dDrawing* a2dHabitat::GetGuiDrawingTemplate() const { return m_drawingTemplateGui; }

bool a2dHabitat::EditCanvasObject( a2dCanvasObject* original, a2dCanvasObject* copy )
{
    //int answer = wxGetNumberFromUser( wxT( "Give me answer to the Ultimate Question of Life, the Universe, and Everything:" ), wxT( "Nr:" ), wxT( "Question" ), 42, 0, 50000 );
	//return true;

	a2dCameleonInst* caminst = wxDynamicCast( copy, a2dCameleonInst );
	if ( caminst )
	{
		a2dCameleon* cam = caminst->GetCameleon();
		a2dSymbol* symbol = cam->GetSymbol();
		a2dDiagram* diagram = cam->GetDiagram();
        wxFrame* frame = wxStaticCast( wxTheApp->GetTopWindow(), wxFrame );
		if ( symbol ) 
        {
    		a2dDrawingPtr preserve = wxDynamicCast( symbol->GetDrawing(), a2dDrawing );
			wxTheApp->GetTopWindow()->SetEvtHandlerEnabled( false );
			a2dDrawingPart* partold = a2dCanvasGlobals->GetActiveDrawingPart();
            a2dToolContr* toolcontr = partold->GetCanvasToolContr();
            toolcontr->SetEvtHandlerEnabled( false );
            a2dBaseTool* firstTool = toolcontr->GetFirstTool();
            firstTool->SetEvtHandlerEnabled( false );
            a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );

            bool ret = false;
            {
			    a2dDrawingEditorModal edit( preserve, wxTheApp->GetTopWindow()  );
		        if ( edit.ShowModal() == wxID_OK )
        		    ret = true;
            }
    	    wxTheApp->GetTopWindow()->SetEvtHandlerEnabled( true );
            toolcontr->SetEvtHandlerEnabled( true );
            firstTool->SetEvtHandlerEnabled( true );
			a2dCanvasGlobals->SetActiveDrawingPart( partold );
    		return ret;
        }
        else if (diagram )
        {
			a2dCameleonInst* caminst = wxDynamicCast( original, a2dCameleonInst );
			a2dCameleon* cam = caminst->GetCameleon();
			a2dDiagram* diagram = cam->GetDiagram();
		    a2dDrawingEditor* edit =  new a2dDrawingEditor( diagram->GetDrawing(), wxTheApp->GetTopWindow() );
		    edit->Show();
		    edit->GetAuiManager().Update();
		    edit->m_drawingPart->SetMappingShowAllCanvas();

            return false;
        }
    }	
	return false;

}

//----------------------------------------------------------------------------
// a2dDrawingId
//----------------------------------------------------------------------------
a2dDrawingIdMap& a2dDrawingId::GetHashMap()
{
    static a2dMemoryCriticalSectionHelper helper;
    static a2dDrawingIdMap ms_Name2Id;
    return ms_Name2Id;
}

a2dDrawingId::a2dDrawingId()
{
	*this = a2dDrawingId_noId();
}

a2dDrawingId::a2dDrawingId( const wxString& drawingName )
{
	static bool f = true ;
    m_id = wxNewId();
    m_name = drawingName;
    if ( GetHashMap().find( m_name ) == GetHashMap().end() )
    {
        GetHashMap()[ m_name ] = this;
    }
    else
        wxFAIL_MSG( wxT( "duplicate id" ) );
}

const a2dDrawingId& a2dDrawingId::GetItemByName( const wxString& a2dDrawingIdName )
{
    a2dDrawingIdMap::iterator iterCommand = GetHashMap().find( a2dDrawingIdName );
    return iterCommand != GetHashMap().end() ? *iterCommand->second : a2dDrawingId_noId();
}

DEFINE_DRAWING_ID( a2dDrawingId_noId )
DEFINE_DRAWING_ID( a2dDrawingId_normal )
DEFINE_DRAWING_ID( a2dDrawingId_cameleonrefs )
DEFINE_DRAWING_ID( a2dDrawingId_physical )
DEFINE_DRAWING_ID( a2dDrawingId_programming )
DEFINE_DRAWING_ID( a2dDrawingId_appearances )

//----------------------------------------------------------------------------
// a2dCanvasGlobal
//----------------------------------------------------------------------------

const a2dPropertyIdPathList a2dCanvasGlobal::PROPID_layerPathList( wxT( "layerPathList" ), a2dPropertyId::flag_none, a2dPathList() );
const a2dSignal a2dCanvasGlobal::sig_changedActiveDrawing = wxNewId(); 

a2dPathList a2dCanvasGlobal::m_layersPath = a2dPathList();

a2dCanvasGlobal::a2dCanvasGlobal()

{
    m_artProvider = new a2dArtProvider();

    m_layersPath.Add( wxT( "." ) );
    m_layersPath.Add( wxT( ".." ) );
    m_layersPath.Add( wxT( "./config" ) );

    m_initialDocChildFramePos = wxPoint( 20, 20 );
    m_initialDocChildFrameSize = wxSize( 600, 300 );
    m_initialDocChildFrameStyle = wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE;
    //m_initialDocChildFrameStyle = wxDEFAULT_FRAME_STYLE | wxFRAME_NO_TASKBAR ;

    m_initialWorldCanvasStyle = wxHSCROLL | wxVSCROLL;

    m_popBeforePush = true;
    m_oneShotTools = false;
    m_editAtEndTools = true;

    m_activeDrawingPart = NULL;

}

void a2dCanvasGlobal::SetArtProvider( a2dArtProvider* artProvider ) 
{ 
    delete m_artProvider;
    m_artProvider = artProvider; 
}


/*
    m_setup = new a2dLayers();

    for (int i = 0; i < 10; i++)
    {
        char buf[40];
        sprintf(buf,"layer %d",i);
        a2dLayerInfo* n=new a2dLayerInfo(i,buf);
        m_setup->Add(n);
        m_setup->Set_Available(i);
        n->Set_Order(i);
        n->Set_InMapping(i);
        n->Set_OutMapping(i);
    }

    char buf[40];
    sprintf(buf,"layer default %d ",wxLAYER_DEFAULT);
    m_setup->Set_Name(wxLAYER_DEFAULT,buf);
    m_setup->Set_Stroke(wxLAYER_DEFAULT, a2dStroke(wxColour(250,0,240),1.0,a2dSTROKE_SOLID));
    m_setup->Set_Fill(wxLAYER_DEFAULT, a2dFill(*wxBLACK,a2dFILL_SOLID));
    //m_setup->Set_Visible(wxLAYER_DEFAULT,false);
    m_setup->Set_Order(wxLAYER_DEFAULT,-1);

    m_setup->Set_Name(wxLAYER_SELECTED,"layer Select");
    //2 not 2.0 to get a pixel pen
    m_setup->Set_Stroke(wxLAYER_SELECTED, a2dStroke(wxColour("YELLOW"),2,a2dSTROKE_SOLID));
    m_setup->Set_Fill(wxLAYER_SELECTED, a2dFill(wxColour("YELLOW"),a2dFILL_CROSSDIAG_HATCH));
    m_setup->Set_Fill(wxLAYER_SELECTED, *a2dTRANSPARENT_FILL);
    m_setup->Set_Order(wxLAYER_SELECTED,-4);

    m_setup->Set_Stroke(5, a2dStroke(wxColour(9,115,64 ),20,a2dSTROKE_SOLID));
    m_setup->Set_Fill(5, a2dFill(wxColour(10,217,2),a2dFILL_CROSSDIAG_HATCH ));

    m_setup->Set_Stroke(6, a2dStroke(wxColour(9,115,64 ),0,a2dSTROKE_SOLID));
    m_setup->Set_Fill(6, a2dFill(wxColour(100,17,255),a2dFILL_CROSSDIAG_HATCH ));

    m_setup->Set_Stroke(7, a2dStroke(wxColour(9,115,64 ),10.0,a2dSTROKE_DOT_DASH));
    m_setup->Set_Fill(7, a2dFill(wxColour(0,120,240),a2dFILL_SOLID ));

    m_setup->Set_Stroke(8, a2dStroke(wxColour(109,115,64 ),40.0,a2dSTROKE_DOT_DASH));
    m_setup->Set_Fill(8, a2dFill(wxColour(200,120,240),a2dFILL_SOLID ));

    m_setup->Set_Stroke(9, a2dStroke(wxColour(255,0,255),40.0,a2dSTROKE_SOLID));
    m_setup->Set_Fill(9, a2dFill(wxColour(0,255,0),a2dFILL_SOLID ));

    m_doc->SetLayerSetup(m_setup);
*/

a2dCanvasGlobal::~a2dCanvasGlobal()
{
    delete m_artProvider;
}

void a2dCanvasGlobal::SetActiveDrawingPart( a2dDrawingPart* part, bool force )
{ 
	bool changed = m_activeDrawingPart != part; 
    m_activeDrawingPart = part; 
	if ( changed || force )
	{
        //wxLogDebug( "again %p", part );
		a2dComEvent changedActiveDrawing( this, sig_changedActiveDrawing );
		ProcessEvent( changedActiveDrawing );
	}
}


//the one and only
a2dCanvasGlobal* a2dCanvasGlobals;

//----------------------------------------------------------------------------
// a2dCanvasModule
//----------------------------------------------------------------------------

a2dFont* a2dCanvasModule::m_FontMedBold;
a2dFont* a2dCanvasModule::m_FontMed;
a2dFont* a2dCanvasModule::m_FontSmall;
a2dFont* a2dCanvasModule::m_FontMini;

IMPLEMENT_DYNAMIC_CLASS( a2dCanvasModule, wxModule )

a2dCanvasModule::a2dCanvasModule()
{
    AddDependency( CLASSINFO( a2dFreetypeModule ) );
    AddDependency( CLASSINFO( a2dArtBaseModule ) );
    AddDependency( CLASSINFO( a2dGeneralModule ) );
}

bool a2dCanvasModule::OnInit()
{   
#if wxUSE_PRINTING_ARCHITECTURE
    m_wxThePrintPaperDatabase = NULL;
    if( wxThePrintPaperDatabase == NULL )
    {
        wxThePrintPaperDatabase = new wxPrintPaperDatabase;
        wxThePrintPaperDatabase->CreateDatabase();
        m_wxThePrintPaperDatabase = wxThePrintPaperDatabase;
    }
#endif

    wxPrintFactory::SetPrintFactory( new a2dPrintFactory() );

    a2dPinClass::InitializeStockPinClasses();

    wxNullCanvasObjectList = new a2dCanvasObjectList();

    a2dCanvasGlobals = new a2dCanvasGlobal();
    a2dCanvasGlobals->SetHabitat( new a2dHabitat() );

    a2dPinClass::Any->SetConnectionGenerator( a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator() );
    a2dPinClass::Standard->SetConnectionGenerator( a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator() );

    wxNullLayerInfo = new a2dLayerInfo( 0, wxT( "not defined" ) );

    a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->SetAnyPinClass( a2dPinClass::Standard );

    m_FontMedBold = new a2dFont( 1, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    m_FontMed = new a2dFont( 1, wxFONTFAMILY_SWISS);
    m_FontSmall = new a2dFont( 0.8, wxFONTFAMILY_SWISS);
    m_FontMini = new a2dFont( 0.5, wxFONTFAMILY_SWISS);

#if wxART2D_USE_FREETYPE

    //Arial TrueTypeFonts fits Agg better and is default available on windows but prefer Linux or internal wxart2d font.
    wxString fontpath = a2dGlobals->GetFontPathList().FindValidPath("LiberationSans-Bold.ttf");
    if ( !fontpath.IsEmpty() )
    {
        delete m_FontMedBold;
        m_FontMedBold = 
            new a2dFont( fontpath, 1);
    }
    else
    {
        fontpath = a2dGlobals->GetFontPathList().FindValidPath("arialbd.ttf");
        if ( !fontpath.IsEmpty() )
        {
            delete m_FontMedBold;
            m_FontMedBold = 
                new a2dFont( fontpath, 1);
        }
    }

    fontpath = a2dGlobals->GetFontPathList().FindValidPath("LiberationSans-Regular.ttf");
    if ( !fontpath.IsEmpty() )
    {
        delete m_FontMed;
        delete m_FontSmall;
        delete m_FontMini;
        m_FontMed = 
            new a2dFont( fontpath, 1 );
        m_FontMini = 
            new a2dFont( fontpath, 0.5 );
        m_FontSmall = 
            new a2dFont( fontpath, 0.8 );
    }
    else
    {
        fontpath = a2dGlobals->GetFontPathList().FindValidPath("arial.ttf");
        if ( !fontpath.IsEmpty() )
        {
            delete m_FontMed;
            delete m_FontSmall;
            delete m_FontMini;
            m_FontMedBold = 
                new a2dFont( fontpath, 1);
            m_FontMed = 
                new a2dFont( fontpath, 1 );
            m_FontMini = 
                new a2dFont( fontpath, 0.5 );
            m_FontSmall = 
                new a2dFont( fontpath, 0.8 );
        }
    }
#endif
    return true;
}

void a2dCanvasModule::OnExit()
{
#if wxUSE_PRINTING_ARCHITECTURE
    if( m_wxThePrintPaperDatabase && m_wxThePrintPaperDatabase != wxThePrintPaperDatabase )
        delete m_wxThePrintPaperDatabase;
    m_wxThePrintPaperDatabase = NULL;
#endif

    // order in OnInit() or in wxWidgets in wxModule::CleanUpModules() muset be reverse order for OnExit()
    // First exit for a highest level module and restore a low level command processor.
    a2dPinClass::DeleteStockPinClasses();

    delete wxNullCanvasObjectList;

    delete a2dCanvasGlobals;

//This is in artglob.cpp  a2dGlobals = NULL;
    a2dCanvasGlobals = NULL;

    delete m_FontMedBold;
    delete m_FontMed;
    delete m_FontSmall;
    delete m_FontMini;
}

const a2dCanvasObjectFlagsMask a2dCanvasOFlags::NON                = 0x00000000; /*!< No flags */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::SELECTED           = 0x00000001; /*!< object is selected */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::SELECTABLE         = 0x00000002; /*!< can select object */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::HasSelectedObjectsBelow = 0x00000004; /*!< selected objects detected below */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::SubEdit            = 0x00000008; /*!< allow subedit of children within this object */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::SubEditAsChild     = 0x00000010; /*!< allow subedit on this object when child of other object */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::VISIBLE            = 0x00000020; /*!< is the object visible (overruled by parent object in some cases during rendering */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::DRAGGABLE          = 0x00000040; /*!< can be dragged */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::SHOWSHADOW         = 0x00000080; /*!< show shadow object if true and shadow property is available */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::FILLED             = 0x00000100; /*!< use fill to fill if set */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::A                  = 0x00000200; /*!< group A flag (e.g boolean operations) */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::B                  = 0x00000400; /*!< group B flag (e.g boolean operations) */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::generatePins       = 0x00000800; /*!< generate Pins when asked for  */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::BIN                = 0x00001000; /*!< general flag  ( used for temporarely marking object ) */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::BIN2               = 0x00002000; /*!< general flag  ( used for temporarely marking object ) */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::PENDING            = 0x00004000; /*!< set when a2dCanvasObject needs an update (redraw) */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::SNAP               = 0x00008000; /*!< snapping of editable object or when moving */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::PUSHIN             = 0x00010000; /*!< push into allowed */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::PRERENDERASCHILD   = 0x00020000; /*!< as child this object will be rendered before the parent object itself when true (default) */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::VISIBLECHILDS      = 0x00040000; /*!< child objects are visible or not */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::EDITABLE           = 0x00080000; /*!< object can be edited */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::ChildrenOnSameLayer = 0x00100000; /*!< child objects are rendered when the object is rendered itself.
                                                   The effect is that the children will appear on the same layer as the object. */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::DoConnect          = 0x00200000; /*!< in case of pins on the object is it currely allowed for other object to connect to it? */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::IsOnCorridorPath   = 0x00400000; /*!< this object is on the corridor path to a captured object */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::HasPins            = 0x00800000; /*!< true if this object does have a2dPin's as children */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::Editing            = 0x01000000; /*!< true if the object is currently being edited */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::EditingCopy        = 0x02000000; /*!< true if the object needs is the editcopy of an object that is edited. */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::IsProperty         = 0x04000000; /*!< True if object is a property show object */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::MouseInObject      = 0x08000000; /*!< true is mouse is currently within object */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::HighLight          = 0x10000000; /*!< true is object is highlighted */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::AlgoSkip           = 0x20000000; /*!< skip object in walker algorithms */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::ignoreSetpending   = 0x40000000; /*!< ignore Setpending calls on a2dCanvasObject */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::HasToolObjectsBelow  = wxULL(0x080000000); /*!< special tool object detected below */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::ChildOnlyTranslate   = wxULL(0x100000000); /*!< do not tranfrom only translate. */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::ignoreLayer          = wxULL(0x200000000); /*!< ignore Layer id, just render always*/
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::SNAP_TO              = wxULL(0x400000000); /*!< snapping of objects to this*/
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::normalized           = wxULL(0x800000000); /*!< normalized object*/
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::NoRenderCanvasObject = wxULL(0x1000000000); /*!< no render if no children*/
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::FixedStyle           = wxULL(0x2000000000); /*!< style on object fixed*/
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::SELECTED2            = wxULL(0x4000000000); /*!< object is selected2 */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::SELECTPENDING        = wxULL(0x8000000000); /*!< object is pending for change in select state */
const a2dCanvasObjectFlagsMask a2dCanvasOFlags::ALL                  = wxULL(0xFFFFFFFFFFFFFFFF);  /*!< To set all flags at once */

a2dCanvasOFlags::a2dCanvasOFlags( a2dCanvasObjectFlagsMask which )
{
    SetFlags( which );
}

void a2dCanvasOFlags::SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which )
{
    if ( which == a2dCanvasOFlags::ALL )
    {
        m_selected = setOrClear;
		m_selected2 = setOrClear;
        m_selectable = setOrClear;
        m_selectPending = setOrClear;
        m_HasSelectedObjectsBelow = setOrClear;
        m_subEdit = setOrClear;
        m_subEditAsChild = setOrClear;
        m_visible = setOrClear;
        m_draggable = setOrClear;
        m_showshadow = setOrClear;
        m_filled = setOrClear;
        m_a = setOrClear;
        m_b = setOrClear;
        m_generatePins = setOrClear;
        m_bin = setOrClear;
        m_bin2 = setOrClear;
        m_pending = setOrClear;
        m_snap = setOrClear;
        m_snap_to = setOrClear;
        m_pushin = setOrClear;
        m_prerenderaschild = setOrClear;
        m_visiblechilds = setOrClear;
        m_editable = setOrClear;
        m_childrenOnSameLayer = setOrClear;
        m_doConnect = setOrClear;
        m_isOnCorridorPath = setOrClear;
        m_hasPins = setOrClear;
        m_editing = setOrClear;
        m_IsProperty = setOrClear;
        m_MouseInObject =
        m_HighLight = setOrClear;
        m_AlgoSkip = setOrClear;
        m_editingCopy = setOrClear;
        m_ignoreSetpending = setOrClear;
        m_template = setOrClear;
        m_external = setOrClear;
        m_resizeToChilds = setOrClear;
        m_normalized = setOrClear;
    }
    else if( which == a2dCanvasOFlags::PENDING )
        m_pending = setOrClear;
    else
    {
        if ( 0 < ( which & a2dCanvasOFlags::SELECTABLE ) ) m_selectable = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::SELECTED ) ) 
            m_selected = setOrClear;
		if ( 0 < ( which & a2dCanvasOFlags::SELECTED2 ) ) m_selected2 = setOrClear;
		if ( 0 < ( which & a2dCanvasOFlags::SELECTPENDING ) ) m_selectPending = setOrClear;        
        if ( 0 < ( which & a2dCanvasOFlags::HasSelectedObjectsBelow ) )  m_HasSelectedObjectsBelow = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::SubEdit ) )   m_subEdit = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::SubEditAsChild ) ) m_subEditAsChild = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::VISIBLE ) )  m_visible = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::DRAGGABLE ) ) m_draggable = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::SHOWSHADOW ) )  m_showshadow = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::FILLED ) )   m_filled = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::A ) )        m_a = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::B ) )        m_b = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::generatePins ) ) m_generatePins = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::BIN ) )      m_bin = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::BIN2 ) )     m_bin2 = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::PENDING ) )  m_pending = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::SNAP ) )     m_snap = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::SNAP_TO ) )  m_snap_to = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::PUSHIN ) )   m_pushin = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::PRERENDERASCHILD ) ) m_prerenderaschild = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::VISIBLECHILDS ) ) m_visiblechilds = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::EDITABLE ) ) m_editable = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::FixedStyle ) ) m_fixedStyle = setOrClear;        
        if ( 0 < ( which & a2dCanvasOFlags::ChildrenOnSameLayer ) ) m_childrenOnSameLayer = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::DoConnect ) ) m_doConnect = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::IsOnCorridorPath ) ) m_isOnCorridorPath = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::HasPins ) )   m_hasPins = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::Editing ) )   m_editing = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::EditingCopy ) )   m_editingCopy = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::IsProperty ) ) m_IsProperty = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::MouseInObject ) ) m_MouseInObject = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::HighLight ) ) m_HighLight = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::AlgoSkip ) ) m_AlgoSkip = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::ignoreSetpending ) )   m_ignoreSetpending = setOrClear;
        if ( 0 < ( which & a2dCanvasOFlags::normalized ) )   m_normalized = setOrClear;
    }
}

void a2dCanvasOFlags::SetFlags( a2dCanvasObjectFlagsMask which )
{
    if ( which == a2dCanvasOFlags::ALL )
    {
        m_selected = true;
		m_selected2 = true;
        m_selectable = true;
        m_selectPending = true;
        m_HasSelectedObjectsBelow = true;
        m_subEdit = true;
        m_subEditAsChild = true;
        m_visible = true;
        m_draggable = true;
        m_showshadow = true;
        m_filled = true;
        m_a = true;
        m_b = true;
        m_generatePins = true;
        m_bin = true;
		m_bin2 = true;
        m_pending = true;
        m_snap = true;
        m_snap_to = true;
        m_pushin = true;
        m_prerenderaschild = true;
        m_visiblechilds = true;
        m_editable = true;
        m_fixedStyle = true;        
        m_childrenOnSameLayer = true;
        m_doConnect = true;
        m_isOnCorridorPath = true;
        m_hasPins = true;
        m_editing = true;
        m_editingCopy = true;
        m_IsProperty = true;
        m_MouseInObject = true;
        m_HighLight = true;
        m_AlgoSkip = true;
        m_ignoreSetpending = true;
        m_template = true;
        m_external = true;
        m_resizeToChilds = true;
        m_normalized = true;
		m_NoRenderCanvasObject = true;
    }
    else
    {
        m_selectable =  0 < ( which & a2dCanvasOFlags::SELECTABLE );
        m_selected   =  0 < ( which & a2dCanvasOFlags::SELECTED );
		m_selected2  =  0 < ( which & a2dCanvasOFlags::SELECTED2 );
        m_selectPending  =  0 < ( which & a2dCanvasOFlags::SELECTPENDING );
        m_HasSelectedObjectsBelow  =  0 < ( which & a2dCanvasOFlags::HasSelectedObjectsBelow );
        m_subEdit   =  0 < ( which & a2dCanvasOFlags::SubEdit );
        m_subEditAsChild =  0 < ( which & a2dCanvasOFlags::SubEditAsChild );
        m_visible    =  0 < ( which & a2dCanvasOFlags::VISIBLE );
        m_draggable  =  0 < ( which & a2dCanvasOFlags::DRAGGABLE ) ;
        m_showshadow =  0 < ( which & a2dCanvasOFlags::SHOWSHADOW );
        m_filled     =  0 < ( which & a2dCanvasOFlags::FILLED );
        m_a          =  0 < ( which & a2dCanvasOFlags::A );
        m_b          =  0 < ( which & a2dCanvasOFlags::B );
        m_generatePins =  0 < ( which & a2dCanvasOFlags::generatePins );
        m_bin        =  0 < ( which & a2dCanvasOFlags::BIN );
        m_bin2       =  0 < ( which & a2dCanvasOFlags::BIN2 );
        m_pending    =  0 < ( which & a2dCanvasOFlags::PENDING );
        m_snap       =  0 < ( which & a2dCanvasOFlags::SNAP );
        m_snap_to    =  0 < ( which & a2dCanvasOFlags::SNAP_TO );
        m_pushin     =  0 < ( which & a2dCanvasOFlags::PUSHIN );
        m_prerenderaschild =  0 < ( which & a2dCanvasOFlags::PRERENDERASCHILD );
        m_visiblechilds   =  0 < ( which & a2dCanvasOFlags::VISIBLECHILDS );
        m_editable   =  0 < ( which & a2dCanvasOFlags::EDITABLE ) ;
        m_fixedStyle =  0 < ( which & a2dCanvasOFlags::FixedStyle ) ;
        m_childrenOnSameLayer =  0 < ( which & a2dCanvasOFlags::ChildrenOnSameLayer );
        m_doConnect  =   0 < ( which & a2dCanvasOFlags::DoConnect );
        m_isOnCorridorPath =   0 < ( which & a2dCanvasOFlags::IsOnCorridorPath );
        m_hasPins    =   0 < ( which & a2dCanvasOFlags::HasPins );
        m_editing    =   0 < ( which & a2dCanvasOFlags::Editing );
        m_editingCopy =   0 < ( which & a2dCanvasOFlags::EditingCopy );
        m_IsProperty =   0 < ( which & a2dCanvasOFlags::IsProperty );
        m_MouseInObject = 0 < ( which & a2dCanvasOFlags::MouseInObject );
        m_HighLight = 0 < ( which & a2dCanvasOFlags::HighLight );
        m_AlgoSkip = 0 < ( which & a2dCanvasOFlags::AlgoSkip );
        m_ignoreSetpending = 0 < ( which & a2dCanvasOFlags::ignoreSetpending );
        m_ChildOnlyTranslate = 0 < ( which & ChildOnlyTranslate );
        m_ignoreLayer = 0 < ( which & ignoreLayer );
        m_normalized = 0 < ( which & a2dCanvasOFlags::normalized );
		m_NoRenderCanvasObject = 0 < ( which & a2dCanvasOFlags::NoRenderCanvasObject );
    }
}

bool a2dCanvasOFlags::GetFlag( a2dCanvasObjectFlagsMask which ) const
{
    switch ( which )
    {
        case a2dCanvasOFlags::SELECTED:        return m_selected;
		case a2dCanvasOFlags::SELECTED2:       return m_selected2;
        case a2dCanvasOFlags::SELECTABLE:      return m_selectable;
        case a2dCanvasOFlags::SELECTPENDING:   return m_selectPending;
        case a2dCanvasOFlags::HasSelectedObjectsBelow:         return m_HasSelectedObjectsBelow;
        case a2dCanvasOFlags::SubEdit:         return m_subEdit;
        case a2dCanvasOFlags::SubEditAsChild:  return m_subEditAsChild;
        case a2dCanvasOFlags::VISIBLE:         return m_visible;
        case a2dCanvasOFlags::DRAGGABLE:       return m_draggable;
        case a2dCanvasOFlags::SHOWSHADOW:      return m_showshadow;
        case a2dCanvasOFlags::FILLED:          return m_filled;
        case a2dCanvasOFlags::A:               return m_a;
        case a2dCanvasOFlags::B:               return m_b;
        case a2dCanvasOFlags::generatePins:    return m_generatePins;
        case a2dCanvasOFlags::BIN:             return m_bin;
        case a2dCanvasOFlags::BIN2:            return m_bin2;
        case a2dCanvasOFlags::PENDING:         return m_pending;
        case a2dCanvasOFlags::SNAP:            return m_snap;
        case a2dCanvasOFlags::SNAP_TO:         return m_snap_to;
        case a2dCanvasOFlags::PUSHIN:          return m_pushin;
        case a2dCanvasOFlags::PRERENDERASCHILD: return m_prerenderaschild;
        case a2dCanvasOFlags::VISIBLECHILDS:   return m_visiblechilds;
        case a2dCanvasOFlags::EDITABLE:        return m_editable;
        case a2dCanvasOFlags::FixedStyle:      return m_fixedStyle;
        case a2dCanvasOFlags::ChildrenOnSameLayer: return m_childrenOnSameLayer;
        case a2dCanvasOFlags::DoConnect:       return m_doConnect;
        case a2dCanvasOFlags::IsOnCorridorPath: return m_isOnCorridorPath;
        case a2dCanvasOFlags::HasPins:         return m_hasPins;
        case a2dCanvasOFlags::Editing:         return m_editing;
        case a2dCanvasOFlags::EditingCopy:     return m_editingCopy;
        case a2dCanvasOFlags::IsProperty:      return m_IsProperty;
        case a2dCanvasOFlags::MouseInObject:   return m_MouseInObject;
        case a2dCanvasOFlags::HighLight:       return m_HighLight;
        case a2dCanvasOFlags::AlgoSkip:        return m_AlgoSkip;
        case a2dCanvasOFlags::ignoreSetpending: return m_ignoreSetpending;
        case a2dCanvasOFlags::normalized:       return m_normalized;
        case a2dCanvasOFlags::NoRenderCanvasObject: return m_NoRenderCanvasObject;
        default:
            wxFAIL_MSG( wxT( "This Flag not implemented" ) );
            return false;
    }
}

a2dCanvasObjectFlagsMask a2dCanvasOFlags::GetFlags() const
{
    a2dCanvasObjectFlagsMask oflags = a2dCanvasOFlags::NON;

    if ( m_selected )   oflags = oflags | a2dCanvasOFlags::SELECTED;
	if ( m_selected2 )  oflags = oflags | a2dCanvasOFlags::SELECTED2;
    if ( m_selectable ) oflags = oflags | a2dCanvasOFlags::SELECTABLE;
    if ( m_selectPending ) oflags = oflags | a2dCanvasOFlags::SELECTPENDING;
    if ( m_HasSelectedObjectsBelow )  oflags = oflags | a2dCanvasOFlags::HasSelectedObjectsBelow;
    if ( m_subEdit )   oflags = oflags | a2dCanvasOFlags::SubEdit;
    if ( m_subEditAsChild ) oflags = oflags | a2dCanvasOFlags::SubEditAsChild;
    if ( m_visible )    oflags = oflags | a2dCanvasOFlags::VISIBLE;
	if ( m_draggable )  oflags = oflags | a2dCanvasOFlags::DRAGGABLE;
    if ( m_showshadow ) oflags = oflags | a2dCanvasOFlags::SHOWSHADOW;
    if ( m_filled )     oflags = oflags | a2dCanvasOFlags::FILLED;
    if ( m_a )          oflags = oflags | a2dCanvasOFlags::A;
    if ( m_b )          oflags = oflags | a2dCanvasOFlags::B;
    if ( m_generatePins ) oflags = oflags | a2dCanvasOFlags::generatePins;
    if ( m_bin )        oflags = oflags | a2dCanvasOFlags::BIN;
    if ( m_bin2 )       oflags = oflags | a2dCanvasOFlags::BIN2;
    if ( m_pending )    oflags = oflags | a2dCanvasOFlags::PENDING;
    if ( m_snap )       oflags = oflags | a2dCanvasOFlags::SNAP;
    if ( m_snap_to )    oflags = oflags | a2dCanvasOFlags::SNAP_TO;
    if ( m_pushin )     oflags = oflags | a2dCanvasOFlags::PUSHIN;
    if ( m_prerenderaschild ) oflags = oflags | a2dCanvasOFlags::PRERENDERASCHILD;
    if ( m_visiblechilds ) oflags = oflags | a2dCanvasOFlags::VISIBLECHILDS;
    if ( m_editable )   oflags = oflags | a2dCanvasOFlags::EDITABLE;
    if ( m_fixedStyle ) oflags = oflags | a2dCanvasOFlags::FixedStyle;
    if ( m_childrenOnSameLayer ) oflags = oflags | a2dCanvasOFlags::ChildrenOnSameLayer;
    if ( m_doConnect )  oflags = oflags | a2dCanvasOFlags::DoConnect;
    if ( m_isOnCorridorPath )  oflags = oflags | a2dCanvasOFlags::IsOnCorridorPath;
    if ( m_hasPins )    oflags = oflags | a2dCanvasOFlags::HasPins;
    if ( m_editing )    oflags = oflags | a2dCanvasOFlags::Editing;
    if ( m_editingCopy ) oflags = oflags | a2dCanvasOFlags::EditingCopy;
    if ( m_IsProperty ) oflags = oflags | a2dCanvasOFlags::IsProperty;
    if ( m_MouseInObject ) oflags = oflags | a2dCanvasOFlags::MouseInObject;
    if ( m_HighLight ) oflags = oflags | a2dCanvasOFlags::HighLight;
    if ( m_AlgoSkip ) oflags = oflags | a2dCanvasOFlags::AlgoSkip;
    if ( m_ignoreSetpending ) oflags = oflags | a2dCanvasOFlags::ignoreSetpending;
	//if ( m_template ) oflags = oflags | a2dCanvasOFlags::?;
	//if ( m_external ) oflags = oflags | a2dCanvasOFlags::?;
	//if ( m_resizeToChilds ) oflags = oflags | a2dCanvasOFlags::?;
    if ( m_normalized ) oflags = oflags | a2dCanvasOFlags::normalized;
    if ( m_NoRenderCanvasObject ) oflags = oflags | a2dCanvasOFlags::NoRenderCanvasObject;

    return oflags;
}

bool a2dCanvasOFlags::CheckMask( a2dCanvasObjectFlagsMask mask ) const
{
    if ( mask == a2dCanvasOFlags::NON )
        return true;
    if ( mask == a2dCanvasOFlags::ALL )
        return true;

    if ( mask == a2dCanvasOFlags::VISIBLE && m_visible ) return true;

    //check the bits in the mask, is set, the corresponding bit in m_flags must be true.
    if ( 0 < ( mask & a2dCanvasOFlags::VISIBLE )                 && !m_visible ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::SELECTABLE )              && !m_selectable ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::SELECTED )                && !m_selected ) return false;
	if ( 0 < ( mask & a2dCanvasOFlags::SELECTED2 )               && !m_selected2 ) return false;
	if ( 0 < ( mask & a2dCanvasOFlags::SELECTPENDING )            && !m_selectPending ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::HasSelectedObjectsBelow ) && !m_HasSelectedObjectsBelow ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::SubEdit )               && !m_subEdit ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::SubEditAsChild )        && !m_subEditAsChild ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::DRAGGABLE )             && !m_draggable ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::SHOWSHADOW )            && !m_showshadow ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::FILLED )                && !m_filled ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::A )                     && !m_a ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::B )                     && !m_b ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::generatePins )          && !m_generatePins ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::BIN )                   && !m_bin ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::BIN2 )                  && !m_bin2 ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::PENDING )               && !m_pending ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::SNAP )                  && !m_snap ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::SNAP_TO )               && !m_snap_to ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::PUSHIN )                && !m_pushin ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::PRERENDERASCHILD )      && !m_prerenderaschild ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::VISIBLECHILDS )         && !m_visiblechilds ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::EDITABLE )              && !m_editable ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::FixedStyle )            && !m_fixedStyle ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::ChildrenOnSameLayer )   && !m_childrenOnSameLayer ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::DoConnect )             && !m_doConnect ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::IsOnCorridorPath )      && !m_isOnCorridorPath ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::HasPins )               && !m_hasPins ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::Editing )               && !m_editing ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::EditingCopy )           && !m_editingCopy ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::IsProperty )            && !m_IsProperty ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::MouseInObject )         && !m_MouseInObject ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::HighLight )             && !m_HighLight ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::AlgoSkip )              && !m_AlgoSkip ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::ignoreSetpending )      && !m_ignoreSetpending ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::normalized )            && !m_normalized ) return false;
    if ( 0 < ( mask & a2dCanvasOFlags::NoRenderCanvasObject )  && !m_NoRenderCanvasObject ) return false;

    return true;
}


