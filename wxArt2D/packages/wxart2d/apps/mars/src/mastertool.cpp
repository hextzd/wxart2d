/*! \file editor/src/mastertool.cpp
    \author Michael Sögtrop
    \date Created 02/06/2004

    Copyright: 2004-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: mastertool.cpp,v 1.22 2008/08/19 23:17:05 titato Exp $
*/

// This file contains master tools. This are tools that customize GUI behaviour
// by intercepting events and then deligating work to other tools

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"
//#include "wx/filename.h"

#include <float.h>
#include "wx/canvas/sttool.h"
#include "wx/canvas/mastertool.h"
#include "mastertool.h"


//!
/*!

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dMarsToolEvtHandler: public a2dStToolEvtHandler
{
    DECLARE_EVENT_TABLE()

public:

    //!
    a2dMarsToolEvtHandler();

    void OnComEvent( a2dComEvent& event );
};


IMPLEMENT_CLASS( msMasterTool, a2dStTool )

BEGIN_EVENT_TABLE( msMasterTool, a2dStTool )
    EVT_MOUSE_EVENTS( msMasterTool::OnMouseEvent )
    EVT_CHAR( msMasterTool::OnChar )
END_EVENT_TABLE()

msMasterTool::msMasterTool( a2dStToolContr* controller ): a2dStTool( controller )
{
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  );
    m_mode = mode_none;
    m_canvasobject = 0;
    m_modehit = 0;

    controller->SetDefaultBehavior( wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
}

msMasterTool::~msMasterTool()
{
}

void msMasterTool::PushDrawWireTool( a2dCanvasObject* WXUNUSED( hit ) )
{
    a2dDrawWirePolylineLTool* tool = new a2dDrawWirePolylineLTool( GetStToolContr() );
    tool->SetOneShot();
    m_controller->PushTool( tool );
}

void msMasterTool::PushDragTool( a2dCanvasObject* hit )
{
    SelectHitObject( hit );
    a2dDragTool* tool = new a2dDragTool( GetStToolContr() );
    tool->SetOneShot();
    tool->SetCursorType( *GetHintCursorDrag() );
    tool->SetBusyCursorType( *GetHintCursorDrag() );
    tool->SetDeleteOnOutsideDrop( true );
    m_controller->PushTool( tool );
}

void msMasterTool::PushDragMultiTool( a2dCanvasObject* WXUNUSED( hit ) )
{
    a2dDragMultiTool* tool = new a2dDragMultiTool( GetStToolContr() );
    tool->SetOneShot();
    tool->SetCursorType( *GetHintCursorDrag() );
    tool->SetBusyCursorType( *GetHintCursorDrag() );
    m_controller->PushTool( tool );
}

void msMasterTool::PushCopyTool( a2dCanvasObject* hit )
{
    SelectHitObject( hit );
    a2dCopyTool* tool = new a2dCopyTool( GetStToolContr() );
    tool->SetOneShot();
    tool->SetCursorType( *GetHintCursorDrag() );
    tool->SetBusyCursorType( *GetHintCursorDrag() );
    tool->SetDeleteOnOutsideDrop( true );
    m_controller->PushTool( tool );
}

void msMasterTool::PushCopyMultiTool( a2dCanvasObject* WXUNUSED( hit ) )
{
    a2dCopyMultiTool* tool = new a2dCopyMultiTool( GetStToolContr() );
    tool->SetOneShot();
    tool->SetCursorType( *GetHintCursorDrag() );
    tool->SetBusyCursorType( *GetHintCursorDrag() );
    m_controller->PushTool( tool );
}

void msMasterTool::PushSelectTool()
{
    a2dSelectTool* tool = new a2dSelectTool( GetStToolContr() );
    tool->SetShiftIsAdd();
    tool->SetOneShot();

    m_controller->PushTool( tool );
}

void msMasterTool::PushEditWireVertexTool( a2dCanvasObject* hit, int vertex )
{
    a2dPolygonL* poly = wxStaticCast( hit, a2dPolygonL );
    SelectHitObject( hit );
    a2dSimpleEditPolygonTool* tool = new a2dSimpleEditPolygonTool( GetStToolContr(), hit, vertex, poly->GetSegments()->size(), a2dSimpleEditPolygonTool::action_movevertex );
    tool->SetOneShot();
    tool->SetCursorType( *GetHintCursorEditWireVertex() );
    tool->SetBusyCursorType( *GetHintCursorEditWireVertex() );
    m_controller->PushTool( tool );
}

void msMasterTool::PushEditWireSegmentHorizontalTool( a2dCanvasObject* hit, int segment )
{
    a2dPolygonL* poly = wxStaticCast( hit, a2dPolygonL );
    SelectHitObject( hit );
    a2dSimpleEditPolygonTool* tool = new a2dSimpleEditPolygonTool( GetStToolContr(), hit, segment, poly->GetSegments()->size(), a2dSimpleEditPolygonTool::action_movesegment );
    tool->SetOneShot();
    tool->SetCursorType( *GetHintCursorEditWireSegmentHorizontal() );
    tool->SetBusyCursorType( *GetHintCursorEditWireSegmentHorizontal() );
    m_controller->PushTool( tool );
}

void msMasterTool::PushEditWireSegmentVerticalTool( a2dCanvasObject* hit, int segment )
{
    a2dPolygonL* poly = wxStaticCast( hit, a2dPolygonL );
    SelectHitObject( hit );
    a2dSimpleEditPolygonTool* tool = new a2dSimpleEditPolygonTool( GetStToolContr(), hit, segment, poly->GetSegments()->size(), a2dSimpleEditPolygonTool::action_movesegment );
    tool->SetOneShot();
    tool->SetCursorType( *GetHintCursorEditWireSegmentVertical() );
    tool->SetBusyCursorType( *GetHintCursorEditWireSegmentVertical() );
    m_controller->PushTool( tool );
}

void msMasterTool::PushEditWireSegmentInsertTool( a2dCanvasObject* hit, int segment )
{
    a2dPolygonL* poly = wxStaticCast( hit, a2dPolygonL );
    SelectHitObject( hit );
    a2dSimpleEditPolygonTool* tool = new a2dSimpleEditPolygonTool( GetStToolContr(), hit, segment, poly->GetSegments()->size(), a2dSimpleEditPolygonTool::action_insertvertex );
    tool->SetOneShot();
    tool->SetCursorType( *GetHintCursorEditWireSegmentInsert() );
    tool->SetBusyCursorType( *GetHintCursorEditWireSegmentInsert() );
    m_controller->PushTool( tool );
}

wxCursor* msMasterTool::GetHintCursorDrag()
{
    static wxCursor cursor( wxCURSOR_HAND );
    return &cursor;
}

wxCursor* msMasterTool::GetHintCursorCopy()
{
    static wxCursor cursor( wxCURSOR_HAND );
    return &cursor;
}

wxCursor* msMasterTool::GetHintCursorDrawWire()
{
    static wxCursor cursor( wxCURSOR_PENCIL );
    return &cursor;
}

wxCursor* msMasterTool::GetHintCursorEditWireVertex()
{
    static wxCursor cursor( wxCURSOR_HAND );
    return &cursor;
}

wxCursor* msMasterTool::GetHintCursorEditWireSegmentHorizontal()
{
    static wxCursor cursor( wxCURSOR_SIZENS );
    return &cursor;
}

wxCursor* msMasterTool::GetHintCursorEditWireSegmentVertical()
{
    static wxCursor cursor( wxCURSOR_SIZEWE );
    return &cursor;
}

wxCursor* msMasterTool::GetHintCursorEditWireSegmentInsert()
{
    static wxCursor cursor( wxCURSOR_SIZENESW );
    return &cursor;
}

bool msMasterTool::ZoomSave()
{
    return true;
}

void msMasterTool::DoStopTool( bool abort )
{
    a2dStTool::DoStopTool( abort );
}

void msMasterTool::OnChar( wxKeyEvent& event )
{
    switch( event.GetKeyCode() )
    {
        case WXK_DELETE:
            // delete all selected objects
            OpenCommandGroupNamed( _( "Delete selected objects" ) );
            a2dCanvasObjectList list;
            m_parentobject->CollectObjects( &list, wxT( "" ), a2dCanvasOFlags::SELECTED );

            forEachIn( a2dCanvasObjectList, &list )
            {
                //delete object from document (actually move to command for redo)
                GetCanvasCommandProcessor()->Submit( new a2dCommand_ReleaseObject( m_parentobject, ( *iter ) ) );
            }
            CloseCommandGroup( );
            break;
    }
}

void msMasterTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }
    m_x = event.GetX();
    m_y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw;
    double yw;

    xw = GetDrawer2D()->DeviceToWorldX( m_x );
    yw = GetDrawer2D()->DeviceToWorldY( m_y );

    if ( event.Moving() && !GetBusy() && !event.ShiftDown() && !event.ControlDown() )
    {
        a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( GetDrawingPart()->GetShowObject() );

        //there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
        // m_connectionGenerator.
        double hitDistance = GetHitMargin();

        a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->
        	GeneratePinsToConnect( GetDrawingPart(), GetDrawingPart()->GetShowObject(), a2dPinClass::Any, a2d_GeneratePinsForStartWire, xw, yw );

        a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->
			        SearchPinForStartWire( GetDrawingPart()->GetShowObject(), xw, yw, a2dPinClass::Any, hitDistance );

    }

    a2dHitEvent hitinfo( xw, yw, false, a2dCANOBJHITOPTION_NONE, true );
    a2dCanvasObject* hit = GetDrawingPart()->IsHitWorld( hitinfo, wxLAYER_ALL );

    if ( event.Moving() && !GetBusy() )
    {
        m_mode = mode_none;
        m_modehit = 0;
        m_modehitinfo = a2dHit();

        if( hit )
        {
            a2dHit how2;

            // Check if there is an unconnected pin
            // This has highest priority
            a2dPin* pin = 0;
            {
                int i;
                float minDist = FLT_MAX;
                for( i = 0; i < hitinfo.m_extended.size(); i++ )
                {
                    a2dPin* pin2 = wxDynamicCast( hitinfo.m_extended[i].GetObject(), a2dPin );
                    if(
                        pin2 &&
                        !pin2->IsConnectedTo() &&
                        ( !pin2->GetPinClass() || pin2->IsObjectPin() ) &&
                        hitinfo.m_extended[i].GetHitType().m_distance < minDist
                    )
                    {
                        pin = pin2;
                        how2 = hitinfo.m_extended[i].GetHitType();
                        minDist = how2.m_distance;
                    }
                }
            }

            // Check if there is a wire
            a2dWirePolylineL* wire = 0;
            if( !pin )
            {
                int i;
                float minDist = FLT_MAX;
                for( i = 0; i < hitinfo.m_extended.size(); i++ )
                {
                    a2dWirePolylineL* wire2 = wxDynamicCast( hitinfo.m_extended[i].GetObject(), a2dWirePolylineL );
                    if( wire2 && hitinfo.m_extended[i].GetHitType().m_distance < minDist && hitinfo.m_extended[i].GetHitType().IsDirectStrokeHit() )
                    {
                        wire = wire2;
                        how2 = hitinfo.m_extended[i].GetHitType();
                        minDist = how2.m_distance;
                    }
                }
            }

            // Check if there is a property
            a2dVisibleProperty* visprop = NULL;
            {
                int i;
                float minDist = FLT_MAX;
                for( i = 0; i < hitinfo.m_extended.size(); i++ )
                {
                    a2dVisibleProperty* visprop2 = wxDynamicCast( hitinfo.m_extended[i].GetObject(), a2dVisibleProperty );
                    if( visprop2 && hitinfo.m_extended[i].GetHitType().m_distance < minDist && hitinfo.m_extended[i].GetHitType().IsHit() )
                    {
                        visprop = visprop2;
                        how2 = hitinfo.m_extended[i].GetHitType();
                        minDist = how2.m_distance;
                    }
                }
            }

            if( pin )
            {
                // wire drawing mode
                GetDrawingPart()->SetCursor( *GetHintCursorDrawWire() );
                m_mode = mode_drawwire;
                m_modehit = pin;
            }
            else if ( wire )
            {
                // wire edting mode
                // check, where the wire was hit:
                m_modehit = wire;
                m_modehitinfo = how2;
                switch( how2.m_stroke2 )
                {
                    case a2dHit::stroke2_vertex:
                        GetDrawingPart()->SetCursor( *GetHintCursorEditWireVertex() );
                        m_mode = mode_editwire_vertex;
                        break;
                    case a2dHit::stroke2_edgehor:
                        GetDrawingPart()->SetCursor( *GetHintCursorEditWireSegmentHorizontal() );
                        m_mode = mode_editwire_segmenthorizontal;
                        break;
                    case a2dHit::stroke2_edgevert:
                        GetDrawingPart()->SetCursor( *GetHintCursorEditWireSegmentVertical() );
                        m_mode = mode_editwire_segmentvertical;
                        break;
                    case a2dHit::stroke2_edgeother:
                        GetDrawingPart()->SetCursor( *GetHintCursorEditWireSegmentInsert() );
                        m_mode = mode_editwire_segmentinsert;
                        break;
                    default:
                        assert( 0 );
                        m_modehit = 0;
                        m_modehitinfo = a2dHit();
                }
            }
            else if ( visprop )
            {
//                GetDrawingPart()->SetCursor( *GetHintCursorDrawWire() );
                m_mode = mode_movelabel;
                m_modehit = hit;
            }
            else if ( !event.ShiftDown() && hit->GetDraggable() && hitinfo.m_how.IsInsideHit() )
            {
                if( !event.ControlDown() )
                {
                    if( hit->GetSelected() )
                    {
                        // drag selected mode
                        GetDrawingPart()->SetCursor( *GetHintCursorDrag() );
                        m_mode = mode_dragmulti;
                        m_modehit = hit;
                    }
                    else
                    {
                        // drag mode
                        GetDrawingPart()->SetCursor( *GetHintCursorDrag() );
                        m_mode = mode_drag;
                        m_modehit = hit;
                    }
                }
                else
                {
                    if( hit->GetSelected() )
                    {
                        // copy selected mode
                        GetDrawingPart()->SetCursor( *GetHintCursorCopy() );
                        m_mode = mode_copymulti;
                        m_modehit = hit;
                    }
                    else
                    {
                        // drag mode
                        GetDrawingPart()->SetCursor( *GetHintCursorCopy() );
                        m_mode = mode_copy;
                        m_modehit = hit;
                    }
                }
            }
            else if ( event.ShiftDown() && hitinfo.m_how.IsInsideHit()  )
            {
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  ) );
                m_mode = mode_select;
                m_modehit = hit;
            }
            else
            {
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  ) );
                m_mode = mode_select;
                m_modehit = hit;
            }
        }
        else
        {
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  ) );
            m_mode = mode_select;
            m_modehit = hit;
        }
        event.Skip();
    }
    else if( event.LeftDown() && !GetBusy() )
    {
        switch( m_mode )
        {
            case mode_none:
                event.Skip();
                break;
            case mode_select:
                PushSelectTool();
                event.Skip();
                break;
            case mode_drag:
                PushDragTool( m_modehit );
                event.Skip();
                break;
            case mode_dragmulti:
                PushDragMultiTool( m_modehit );
                event.Skip();
                break;
            case mode_copy:
                PushCopyTool( m_modehit );
                event.Skip();
                break;
            case mode_copymulti:
                PushCopyMultiTool( m_modehit );
                event.Skip();
                break;
            case mode_drawwire:
                PushDrawWireTool( m_modehit );
                event.Skip();
                break;
            case mode_editwire_vertex:
                PushEditWireVertexTool( m_modehit, m_modehitinfo.m_index );
                event.Skip();
                break;
            case mode_editwire_segmenthorizontal:
                PushEditWireSegmentHorizontalTool( m_modehit, m_modehitinfo.m_index );
                event.Skip();
                break;
            case mode_editwire_segmentvertical:
                PushEditWireSegmentVerticalTool( m_modehit, m_modehitinfo.m_index );
                event.Skip();
                break;
            case mode_editwire_segmentinsert:
                PushEditWireSegmentInsertTool( m_modehit, m_modehitinfo.m_index );
                event.Skip();
                break;
            case mode_movelabel:
            {
                a2dObjectEditTool* tool = new a2dObjectEditTool( GetStToolContr() );
                tool->SetCursorType( *GetHintCursorDrag() );
                tool->SetBusyCursorType( *GetHintCursorDrag() );
                tool->SetCorridor( m_corridor );
                m_controller->PushTool( tool );
            }
//            PushEditWireSegmentInsertTool( m_modehit, m_modehitinfo.m_index );
            event.Skip();
            break;
            default:
                event.Skip();
        }
    }
    else
    {
        event.Skip();
    }
}

void msMasterTool::SelectHitObject( a2dCanvasObject* hit )
{
    OpenCommandGroupNamed( _( "Select" ) );

    DeselectAll();

    GetCanvasCommandProcessor()->Submit(
        new a2dCommand_SetFlag( hit, a2dCanvasOFlags::SELECTED, true ) );

    CloseCommandGroup();
}



//----------------------------------------------------------------------------
// a2dCurveZoomTool
//----------------------------------------------------------------------------

#define CONTRDRAWER m_controller->GetDrawingPart()
#define CONTRDRAWER2D m_controller->GetDrawingPart()->GetDrawer2D()

IMPLEMENT_CLASS( msCurveZoomTool, a2dStTool )


BEGIN_EVENT_TABLE( msCurveZoomTool, a2dStTool )
    EVT_MOUSE_EVENTS( msCurveZoomTool::OnMouseEvent )
END_EVENT_TABLE()

msCurveZoomTool::msCurveZoomTool( a2dStToolContr* controller )
    : a2dStTool( controller )
{
    m_anotate = false;
    m_toolcursor = wxCURSOR_MAGNIFIER;
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );

    //stroke and fill should not change as in base class for m_canvasobject

    //i like transparent fill and specific stroke
    SetFill( *a2dTRANSPARENT_FILL );
    SetStroke( a2dStroke( *wxBLACK, 1, a2dSTROKE_LONG_DASH ) );
}

msCurveZoomTool::~msCurveZoomTool()
{
}

void msCurveZoomTool::OnMouseEvent( wxMouseEvent& event )
{
    event.Skip();
}

/*
//-----------------------------------------------------------
// a2dMarsToolEvtHandler
//-----------------------------------------------------------

BEGIN_EVENT_TABLE(a2dMarsToolEvtHandler, a2dMarsToolEvtHandler )
    EVT_COM_EVENT( a2dMarsToolEvtHandler::OnComEvent )
END_EVENT_TABLE()

a2dMarsToolEvtHandler::a2dMarsToolEvtHandler();
{
}

bool a2dMarsToolEvtHandler::ProcessEvent( wxEvent& event )
{
    if ( ! a2dEvtHandler::ProcessEvent( wxEvent& event ) )
        event.GetEventObject()->ProcessEvent( event );
    return true;
}

void a2dMarsToolEvtHandler::OnComEvent( a2dComEvent& event )
{
}

}

*/
