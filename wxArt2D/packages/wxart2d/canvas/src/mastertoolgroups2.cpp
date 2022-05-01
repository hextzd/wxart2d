/*! \file editor/src/mastertoolgroups.cpp
    \author Klaas Holwerda
    \date Created 22/04/2012

    Copyright: 2012-2012 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: mastertoolgroups.cpp,v 1.13 2009/04/23 19:35:23 titato Exp $
*/

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
#include "wx/canvas/sttool2.h"
#include "wx/canvas/mastertoolgroups.h"

//----------------------------------------------------------------------------
// a2dMasterTagGroups2 
//----------------------------------------------------------------------------

IMPLEMENT_CLASS( a2dMasterTagGroups2 , a2dMasterDrawBase )

BEGIN_EVENT_TABLE( a2dMasterTagGroups2 , a2dMasterDrawBase )
    EVT_MOUSE_EVENTS( a2dMasterTagGroups2 ::OnMouseEvent )
    EVT_CHAR( a2dMasterTagGroups2 ::OnChar )
    EVT_KEY_DOWN( a2dMasterTagGroups2 ::OnKeyDown )
    EVT_KEY_UP( a2dMasterTagGroups2 ::OnKeyUp )
END_EVENT_TABLE()

a2dMasterTagGroups2 ::a2dMasterTagGroups2 ( a2dStToolContr* controller ): a2dMasterDrawBase( controller )
{
    controller->SetDefaultBehavior( controller->GetDefaultBehavior() | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_Select  );
}

a2dMasterTagGroups2 ::~a2dMasterTagGroups2 ()
{
}

void a2dMasterTagGroups2 ::Render()
{
}

void a2dMasterTagGroups2::DoStopTool( bool abort )
{
    a2dMasterDrawBase::DoStopTool( abort );
    m_toolBusy = false;
}

void a2dMasterTagGroups2::AbortBusyMode()
{
    a2dCanvasObject* selected =  m_parentobject->GetChildObjectList()->Find( "", "", a2dCanvasOFlags::SELECTED );

    if ( selected )
    {
       DeselectAll();
    }
    if( GetBusy() && m_dragStarted )
    {
        switch( m_mode )
        {
            case mode_zoom:
            case mode_select:
            case mode_cntrlselect:
            {
                break;
            }
            default:
                break;
        }
    }

    a2dStTool::AbortBusyMode();
}

void a2dMasterTagGroups2::OnChar( wxKeyEvent& event )
{
    //wxLogDebug(wxT("key %d"), event.GetKeyCode());
    a2dBaseTool* first = m_stcontroller->GetFirstTool();

    a2dCanvasObject* selected =  m_parentobject->GetChildObjectList()->Find( "", "", a2dCanvasOFlags::SELECTED );

    if ( selected )
    {
        event.Skip();
    }
    else if( GetBusy() && m_dragStarted )
    {
        switch( m_mode )
        {
            case mode_zoom:
            case mode_select:
            case mode_cntrlselect:
            {
                break;
            }
            default:
                event.Skip();
                break;
        }
    }
    else
        event.Skip();
}

void a2dMasterTagGroups2 ::OnKeyDown( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }
    event.Skip();
}

void a2dMasterTagGroups2 ::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }
    event.Skip();
}

void a2dMasterTagGroups2 ::OnMouseEvent( wxMouseEvent& event )
{
    //MouseDump( event, "OnM " );

    if ( !m_active )
    {
        event.Skip();
        return;
    }
    m_x = event.GetX();
    m_y = event.GetY();

    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    //GetDrawingPart()->Update( a2dCANVIEW_UPDATE_PENDING );
    a2dIterC ic( GetDrawingPart() );
    ic.SetLayer( wxLAYER_ALL );
    ic.SetObjectFilter( new a2dCanvasObjectFilterLayerMask( wxLAYER_ALL, a2dCanvasOFlags::VISIBLE | a2dCanvasOFlags::SELECTABLE ) );
    a2dHitEvent hitinfo( m_xwprev, m_ywprev, false, a2dCANOBJHITOPTION_NONE, true );
    hitinfo.m_option = a2dCANOBJHITOPTION_LAYERS;
    //at the top level the group its matrix is to be ignored.
    //Since it is normally NOT ignored within a2dCanvasObject, force an inverse.
    hitinfo.m_xyRelToChildren = true;

    if ( event.LeftDClick() && !m_toolBusy )
    {
        a2dCanvasObject* hit = m_modehit; //reuse from the LeftDown event.
        if ( hit )
        {
            a2dWirePolylineL* wire = wxDynamicCast( hit, a2dWirePolylineL );
            m_modehit = hit;

            if ( (wire && wire->GetSelected()) || !wire )
            {
                hit->SetSelected( m_modehitLastSelectState );
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                m_mode = mode_edit;
                PushEditTool( m_modehit );
            }
        }
    }
    else if ( event.Dragging() && m_toolBusy && !m_dragStarted )
    {
        //we assume dragging starts for real if 5 pixels shift.
        if ( abs( m_x - m_dragstartx ) >= 5 || abs( m_y - m_dragstarty ) >= 5 )
        {
            a2dCanvasObject* hit = m_modehit; //use last hit at LeftDown
            if ( event.ControlDown() && !event.ShiftDown() )
            {
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                m_mode = mode_cntrlselect;
                m_dragStarted = true;
            }
            else if ( !event.ControlDown() && event.ShiftDown() )
            {
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                m_mode = mode_select;
                m_dragStarted = true;
            }
            else 
            {
                //both control and shift or non 
                if( hit && hit->GetDraggable() )
                {
                    if ( hit->GetSelected() )
                    {
                        m_dragStarted = true;

                        if ( event.ControlDown() && event.ShiftDown() )
                        {
                            m_mode = mode_copymulti;
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Copy ) );
                        }
                        else //no shift no control                   
                        {
                            m_mode = mode_dragmulti;
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                        }
                    }
                    else               
                    {
                        m_mode = mode_drag;
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                    }
                }
                else 
                {
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                    m_mode = mode_select;
                    m_modehit = hit;//NULL;
                }
            }

            m_toolBusy = false;
            switch( m_mode )
            {
                case mode_cntrlselect:
                    DeselectAll();
                    m_toolBusy = true;
                    event.Skip();
                    break;
                case mode_select:
                    break;
                case mode_dragmulti:
                {
                    PushDragMultiTool( m_modehit );
                    //event.Skip();
                    break;
                }
                case mode_copymulti:
                {
                    PushCopyMultiTool( m_modehit );
                    //event.Skip();
                    break;
                }
                case mode_drag:
                {
                    PushDragTool( m_modehit );
                    //event.Skip();
                    break;
                }
                case mode_copy:
                {
                    PushCopyTool( m_modehit );
                    //event.Skip();
                    break;
                }
                default:
                    m_toolBusy = true;
                    break;
            }
        }
    }
    else if ( event.Dragging() && m_toolBusy && m_dragStarted )
    {
    }
    else if ( event.Moving() && !m_toolBusy )
    {
        m_mode = mode_none;
        m_modehit = 0;
        m_modehitinfo = a2dHit();
        a2dCanvasObject* top = GetDrawingPart()->GetShowObject();
        a2dCanvasObject* hit = top->IsHitWorld( ic, hitinfo );
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( top );

        if ( !event.ControlDown() && event.ShiftDown() )
        {
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
            if ( hit )
                m_modehit = hit;
            m_mode = mode_select;
        }      
        else
        {
            if ( hit )
            {
                // Check if there is an unconnected pin
                // This has highest priority
                a2dPin* pin = NULL;
                a2dHit how2;

                a2dWirePolylineL* wire = wxDynamicCast( hit, a2dWirePolylineL );

                if ( wire && wire->GetSelected() || m_vertexSegmentEdit )
                {
                    int i;
                    float minDist = FLT_MAX;
                    for( i = 0; i < hitinfo.m_extended.size(); i++ )
                    {
                        a2dWirePolylineL* wire2 = wxDynamicCast( hitinfo.m_extended[i].GetObject(), a2dWirePolylineL );
                        if( wire2 && hitinfo.m_extended[i].GetHitType().m_distance < minDist && hitinfo.m_extended[i].GetHitType().IsStrokeHit() )
                        //if( wire2 && hitinfo.m_extended[i].GetHitType().m_distance < minDist && hitinfo.m_extended[i].GetHitType().IsDirectStrokeHit() )
                        {
                            wire = wire2;
                            how2 = hitinfo.m_extended[i].GetHitType();
                            minDist = how2.m_distance;
                        }
                    }

                    if ( wire )
                    {
                        // wire editing mode
                        // check, where the wire was hit:
                        m_modehit = wire;
                        m_modehitinfo = how2;
                        switch( how2.m_stroke2 )
                        {
                            case a2dHit::stroke2_vertex:
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireVertex ) );
                                m_mode = mode_editwire_vertex;
                                break;
                            case a2dHit::stroke2_edgehor:
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentHorizontal ) );
                                m_mode = mode_editwire_segmenthorizontal;
                                break;
                            case a2dHit::stroke2_edgevert:
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentVertical ) );
                                m_mode = mode_editwire_segmentvertical;
                                break;
                            case a2dHit::stroke2_edgeother:
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_WireSegmentInsert ) );
                                m_mode = mode_editwire_segmentinsert;
                                break;
                            default:
                                m_modehit = NULL;
                                m_modehitinfo = a2dHit();
                        }
                    }        
                }
                else if ( wire )
                {
                    // m_connectionGenerator.
                    GetDrawing()->GetHabitat()->GetConnectionGenerator()->
	        	        GeneratePinsToConnect( GetDrawingPart(), wire, a2dPinClass::Any, a2d_GeneratePinsForStartWire, m_xwprev, m_ywprev );

                    pin = GetDrawing()->GetHabitat()->GetConnectionGenerator()->
				                SearchPinForStartWire( GetDrawingPart()->GetShowObject(), m_xwprev, m_ywprev, a2dPinClass::Any, GetHitMargin() );

                    if( event.ControlDown() && pin && !pin->IsTemporaryPin() )
                    {
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                        m_mode = mode_move_pin;
                        m_modehit = pin;
                    }
                    else
                    {
                        // wire drawing mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                        m_mode = mode_drawwire;
                        m_modehit = pin;
                    }

                }
                else
                {
                    if ( !event.ShiftDown() && !event.ControlDown() )
                    {
                        //While moving event, there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
                        // m_connectionGenerator.
                        GetDrawing()->GetHabitat()->GetConnectionGenerator()->
		        	        GeneratePinsToConnect( GetDrawingPart(), hit, a2dPinClass::Any, a2d_GeneratePinsForStartWire, m_xwprev, m_ywprev );

                        pin = GetDrawing()->GetHabitat()->GetConnectionGenerator()->
					                SearchPinForStartWire( GetDrawingPart()->GetShowObject(), m_xwprev, m_ywprev, a2dPinClass::Any, GetHitMargin() );

                        if ( pin )
                        {
                            a2dWirePolylineL* wire = wxDynamicCast( pin->GetParent(), a2dWirePolylineL );
                            if ( wire && wire->GetSelected() || m_vertexSegmentEdit )
                                pin = NULL;
                        }
                    }

                    // if not editing a wire, search for pins first, in order to start a wire draw, if not a pin hit ....
                    if( pin )
                    {
                        // wire drawing mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                        m_mode = mode_drawwire;
                        m_modehit = pin;
                    }
                    else 
                    {
                        if( hit->GetDraggable() )
                        {
                            if ( hit->GetSelected() )
                            {
                                m_modehit = hit;
                                if ( event.ControlDown() && event.ShiftDown() )
                                {
                                    m_mode = mode_copymulti;
                                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Copy ) );
                                }
                                else //no shift no control                   
                                {
                                    m_mode = mode_dragmulti;
                                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                                }
                            }
                            else
                            {
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                                m_mode = mode_select;
                                m_modehit = hit;
                            }
                        }
                        else 
                        {
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                            m_mode = mode_select;
                            m_modehit = hit;
                        }
                    }
                }                
            }
            else
            {
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                m_mode = mode_select;
                m_modehit = hit;
            }
        }
        event.Skip();
    }
    else if( event.LeftDown() && !m_toolBusy )
    {
        m_dragStarted = false;
        m_xprev = m_x;
        m_yprev = m_y;
        m_dragstartx = m_x;
        m_dragstarty = m_y;

        if( !m_modehit )
		{
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
            m_mode = mode_select;
            event.Skip();
            m_toolBusy = false;
			return;
	    }

        switch( m_mode )
        {
            case mode_cntrlselect:
                DeselectAll();
                m_toolBusy = true;
                break;
            case mode_select:
                m_toolBusy = true;
                break;
            case mode_move_pin:
                PushMovePinTool( m_modehit );
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
            case mode_editwire_segment:
                PushEditSegmentTool( m_modehit, m_modehitinfo.m_index );
                event.Skip();
                break;
            default:
                // not know untill a drag started or not.
                m_mode = mode_none;
                m_toolBusy = true;
                event.Skip();
        }
    }
    else if( event.LeftUp() && m_toolBusy && m_dragStarted )
    {
        m_toolBusy = false;
        m_dragStarted = false; //end of drag
        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
    }
    else if( event.LeftUp() && m_toolBusy )
    {
        m_toolBusy = false;
        m_dragStarted = false; //never started but still

        //reserve for double click if it happens
        m_modehitLastSelectState = false;
        if ( m_modehit )
            m_modehitLastSelectState = m_modehit->GetSelected();

        if ( !event.ControlDown() && !event.ShiftDown() )
        {
            DeselectAll();
            if ( m_modehit && m_modehit->GetSelectable() )
                m_modehit->SetSelected( true );
        }
        else if ( event.ShiftDown() )
        {
            if ( m_modehit && m_modehit->GetSelectable() )
            {
                if ( m_modehit->IsSelected() )
                    m_modehit->SetSelected( false );
                else
                    m_modehit->SetSelected( true );
            }
        }
        else
        {
            if ( !m_modehit )
                DeselectAll();
            else
            {
                if ( m_modehit->GetSelected() )
                {
                    if ( m_mode == mode_none && m_modehit->GetSelectable() )
                        m_modehit->SetSelected( ! m_modehit->GetSelected() );
                }
                else
                {
                    if ( m_mode == mode_none && m_modehit->GetSelectable() )
                    {
                        a2dCanvasObject* selected =  m_parentobject->GetChildObjectList()->Find( "", "", a2dCanvasOFlags::SELECTED );
                        if ( selected )
                        {
                            DeselectAll();
                            m_modehit->SetSelected( true );
                        }
                        else
                        {
                            m_modehit->SetSelected( true );
                        }
                    }
                }
            }
        }
        if ( m_modehit && m_modehit->GetSelected() )
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
        else
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
    }
    else if( event.RightDown() && !GetBusy() )
    {
        PushZoomTool();
        event.Skip();
    }
    else
    {
        event.Skip();
    }
}

