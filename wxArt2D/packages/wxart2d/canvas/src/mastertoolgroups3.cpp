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
// a2dMasterTagGroups3 
//----------------------------------------------------------------------------

bool a2dMasterTagGroups3::m_selectMoreAtShift = false;

IMPLEMENT_CLASS( a2dMasterTagGroups3 , a2dMasterDrawBase )

BEGIN_EVENT_TABLE( a2dMasterTagGroups3 , a2dMasterDrawBase )
    EVT_MOUSE_EVENTS( a2dMasterTagGroups3 ::OnMouseEvent )
    EVT_CHAR( a2dMasterTagGroups3 ::OnChar )
    EVT_KEY_DOWN( a2dMasterTagGroups3 ::OnKeyDown )
    EVT_KEY_UP( a2dMasterTagGroups3 ::OnKeyUp )
    EVT_COM_EVENT( a2dMasterTagGroups3::OnComEvent ) 
END_EVENT_TABLE()

a2dMasterTagGroups3 ::a2dMasterTagGroups3 ( a2dStToolContr* controller ): 
    m_AllowDoubleClickModifier( true ),
    m_shiftWasDown( false ),
    m_ctrlWasDown( false ),
    m_altWasDown( false ),
    m_generatePinsNonWire( true ),
    m_generatePinsAlsoNonWireMode( true ),
	a2dMasterDrawBase( controller )
 {
    m_selectedObjects = NULL;
    controller->SetDefaultBehavior( controller->GetDefaultBehavior() | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_Select  );
    m_drawWire = false;
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );
}

a2dMasterTagGroups3 ::~a2dMasterTagGroups3 ()
{
    a2dCanvasGlobals->DisconnectEvent( a2dEVT_COM_EVENT, this );
    if ( m_selectedObjects )
        delete m_selectedObjects;
}

wxString a2dMasterTagGroups3 ::GetToolString() const
{
	wxString str =  GetClassInfo()->GetClassName();
	if ( m_drawWire )
		return str + " Wires";
	if ( m_dlgOrEdit )
		return str + " Dlg";
	return str;
}

void a2dMasterTagGroups3 ::Render()
{
}

void a2dMasterTagGroups3::DoStopTool( bool abort )
{
    a2dMasterDrawBase::DoStopTool( abort );
    m_toolBusy = false;
}

void a2dMasterTagGroups3::AbortBusyMode()
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

void a2dMasterTagGroups3::SetActive( bool active )
{
    a2dStTool::SetActive( active );
    m_pending = true;

    if ( active )
    {
		//depending on keys and mouse position mode is decided.
		//(is also called when other tool was poped from stack).
	    wxPoint mouspos = m_stcontroller->GetMousePosition();
		m_x = mouspos.x;
		m_y = mouspos.y;
		MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );
		SetToolMode( false, false, false );
		SelectedStatus();
    }
}

void a2dMasterTagGroups3::OnComEvent( a2dComEvent& event )
{
    if ( GetEvtHandlerEnabled() )
    {
    	a2dSignal comId = event.GetId();

        if ( event.GetId() == a2dBaseTool::sig_toolPoped )
        {
			if ( GetDrawingPart() ) 
            	GetDrawingPart()->Update( a2dCANVIEW_UPDATE_OLDNEW );

			a2dProperty* prop = ( a2dProperty* ) event.GetProperty();
			a2dDrawWirePolylineLTool* wiretool = wxDynamicCast( prop->GetValue(), a2dDrawWirePolylineLTool );
			if ( wiretool )
			{
				m_endSegmentMode = wiretool->GetEndSegmentMode();
				a2dCanvasGlobals->GetHabitat()->SetEndSegmentMode( m_endSegmentMode );
			}
			a2dDragNewTool* dragnewtool = wxDynamicCast( prop->GetValue(), a2dDragNewTool );
			if ( dragnewtool )
			{
			}
		}

        if ( 
            comId == a2dHabitat::sig_changedFill ||
            comId == a2dHabitat::sig_changedStroke ||
            comId == a2dHabitat::sig_changedLayer
           )
        {
            a2dStTool* tool = wxDynamicCast( this, a2dStTool );
            if ( tool && tool->GetActive() && tool->GetEvtHandlerEnabled() && tool->GetDrawingPart() )
            {
                bool OnSelected = false;

			    if ( !m_dlgOrEditModal )
                {
                    a2dCanvasObjectList objects;
                    m_parentobject->CollectObjects( &objects, wxT( "" ), a2dCanvasOFlags::SELECTED );
                    if ( objects.size() > 0 )
                        OnSelected = true;
                }
			    else
                {
                    if ( /*IsShowm_styleDlg &&*/ m_modehit && m_modehit->GetSelected() )
                    {
                        a2dCanvasObjectList objects;
                        m_parentobject->CollectObjects( &objects, wxT( "" ), a2dCanvasOFlags::SELECTED );
                        if ( objects.size() > 0 )
                            OnSelected = true;
                    }
                }
                if ( !OnSelected )
                {
                    //m_styleDlg->GetFill()
                    //m_styleDlg->GetStroke()

                    a2dCanvasCommandProcessor* docCmdh = tool->GetCanvasCommandProcessor();
                    a2dFill f = GetDrawing()->GetHabitat()->GetFill();
                    if ( comId == a2dHabitat::sig_changedFill && ! tool->GetFill().IsSameAs( f ) )
                    {
                        if ( m_modehit )
                            docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( m_modehit, f ) );
                    }
                    a2dStroke s = GetDrawing()->GetHabitat()->GetStroke();
                    if ( comId == a2dHabitat::sig_changedStroke && ! tool->GetStroke().IsSameAs( s ) )
                    {
                        if ( m_modehit )
                            docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( m_modehit, s ) );
                    }
                }
                else
                {
                    a2dCanvasCommandProcessor* docCmdh = tool->GetCanvasCommandProcessor();
                    a2dFill f = GetDrawing()->GetHabitat()->GetFill();
                    if ( comId == a2dHabitat::sig_changedFill )
                    {
                        docCmdh->Submit( new a2dCommand_SetFillStrokeMask(  tool->GetDrawingPart()->GetShowObject() ) );
                    }
                    a2dStroke s = GetDrawing()->GetHabitat()->GetStroke();
                    if ( comId == a2dHabitat::sig_changedStroke )
                    {
                        docCmdh->Submit( new a2dCommand_SetFillStrokeMask(  tool->GetDrawingPart()->GetShowObject() ) );
                    }
                    wxUint16 layer = GetDrawing()->GetHabitat()->GetLayer();
                    if ( comId == a2dHabitat::sig_changedLayer )
                    {
                        docCmdh->Submit( new a2dCommand_SetLayerMask( tool->GetDrawingPart()->GetShowObject(), layer ) );
                    }
                }
            }
        }


        if ( 
            comId == a2dHabitat::sig_changedTextFill ||
            comId == a2dHabitat::sig_changedTextStroke
           )
        {
            a2dStTool* tool = wxDynamicCast( this, a2dStTool );
            if ( tool && tool->GetActive() && tool->GetEvtHandlerEnabled() && tool->GetDrawingPart() )
            {
                bool OnSelected = false;

			    if ( !m_dlgOrEditModal )
                {
                    a2dCanvasObjectList objects;
                    m_parentobject->CollectObjects( &objects, wxT( "" ), a2dCanvasOFlags::SELECTED );
                    if ( objects.size() > 0 )
                        OnSelected = true;
                }
			    else
                {
                    if ( /*IsShowm_styleDlg &&*/ m_modehit && m_modehit->GetSelected() )
                    {
                        a2dCanvasObjectList objects;
                        m_parentobject->CollectObjects( &objects, wxT( "" ), a2dCanvasOFlags::SELECTED );
                        if ( objects.size() > 0 )
                            OnSelected = true;
                    }
                }
                if ( !OnSelected )
                {
                    //m_styleDlg->GetFill()
                    //m_styleDlg->GetStroke()
                    if ( m_modehit )
                    {
                        a2dCanvasCommandProcessor* docCmdh = tool->GetCanvasCommandProcessor();
                        a2dFill f = GetDrawing()->GetHabitat()->GetTextFill();
                        if ( comId == a2dHabitat::sig_changedTextFill && ! m_modehit->GetFill().IsSameAs( f ) )
                        {
                            docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( m_modehit, f ) );
                        }
                        a2dStroke s = GetDrawing()->GetHabitat()->GetTextStroke();
                        if ( comId == a2dHabitat::sig_changedTextStroke && ! m_modehit->GetStroke().IsSameAs( s ) )
                        {
                            docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( m_modehit, s ) );
                        }
                    }
                }
                else
                {
                    a2dCanvasCommandProcessor* docCmdh = tool->GetCanvasCommandProcessor();
                    a2dFill f = GetDrawing()->GetHabitat()->GetTextFill();
                    if ( comId == a2dHabitat::sig_changedTextFill )
                    {
                        docCmdh->Submit( new a2dCommand_SetFillStrokeMask(  tool->GetDrawingPart()->GetShowObject() ) );
                    }
                    a2dStroke s = GetDrawing()->GetHabitat()->GetTextStroke();
                    if ( comId == a2dHabitat::sig_changedTextStroke )
                    {
                        docCmdh->Submit( new a2dCommand_SetFillStrokeMask(  tool->GetDrawingPart()->GetShowObject() ) );
                    }
                }
            }
        }
        event.Skip();

    }


    event.Skip();
} 

void a2dMasterTagGroups3::OnChar( wxKeyEvent& event )
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

void a2dMasterTagGroups3::OnKeyDown( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    switch( event.GetKeyCode() )
    {
        case 'w':
        case 'W':
        {
            if ( !event.HasAnyModifiers() )
            {
                m_drawWire = !m_drawWire;
                SetToolMode( event.m_controlDown, event.m_shiftDown, event.AltDown() );
            }
            else
                event.Skip();
            break;
        }
        case WXK_CONTROL:
        case WXK_SHIFT:
        case WXK_ALT:
        {
            SetToolMode( event.m_controlDown, event.m_shiftDown, event.AltDown() );

            if ( event.m_shiftDown && ( m_mode == mode_select || m_mode == mode_copymulti ) )
            {   //maybe more cases like this, OnKeyDown has effect on restrict engine, in a2dStTool::OnKeyUp
            }
            else
                event.Skip();
            break;
        }
        default:
            event.Skip();
    }
}

void a2dMasterTagGroups3::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    switch( event.GetKeyCode() )
    {
        case 'w':
        case 'W':
        {
            if ( !event.HasAnyModifiers() )
                SetToolMode( event.m_controlDown, event.m_shiftDown, event.AltDown() );
            else
                event.Skip();
            break;
        }
        case WXK_CONTROL:
        case WXK_SHIFT:
        case WXK_ALT:
        {
            SetToolMode( event.m_controlDown, event.m_shiftDown, event.AltDown() );

            if ( event.m_shiftDown && ( m_mode == mode_select || m_mode == mode_copymulti ) )
            {   //maybe more cases like this, OnKeyDown has effect on restrict engine, in a2dStTool::OnKeyDown
            }
            else
                event.Skip();
            break;
        }
        default:
            event.Skip();
    }
}

bool a2dMasterTagGroups3::SetWireCursor( a2dWirePolylineL* wire, const a2dHitEvent& hitinfo, bool ctrl, bool shift, bool alt )
{
    a2dHit how2;
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
        if ( !alt )
        {
            switch( how2.m_stroke2 )
            {
                case a2dHit::stroke2_vertex:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireVertex ) );
                    SetStateString( _("drag to move vertex on the wire"), STAT_toolHelp );
                    m_mode = mode_editwire_vertex;
                    break;
                case a2dHit::stroke2_edgehor:
                case a2dHit::stroke2_edgevert:
                case a2dHit::stroke2_edgeother:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_WireSegmentInsert ) );
                    SetStateString( _("drag to insert vertex on the wire"), STAT_toolHelp );
                    m_mode = mode_editwire_segmentinsert;
                    break;
                default:
                    m_modehit = NULL;
                    m_modehitinfo = a2dHit();
            }
        }
        else
        {
            switch( how2.m_stroke2 )
            {
                case a2dHit::stroke2_vertex:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireDeleteVertex ) );
                    SetStateString( _("click to remove vertex from wire"), STAT_toolHelp );
                    m_mode = mode_editwire_vertex;
                    break;
                case a2dHit::stroke2_edgehor:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentHorizontal ) );
                    SetStateString( _("drag to move segment of the wire"), STAT_toolHelp );
                    m_mode = mode_editwire_segmenthorizontal;
                    break;
                case a2dHit::stroke2_edgevert:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentVertical ) );
                    SetStateString( _("drag to move segment of the wire"), STAT_toolHelp );
                    m_mode = mode_editwire_segmentvertical;
                    break;
                case a2dHit::stroke2_edgeother:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegment ) );
                    SetStateString( _("drag to move segment, Alt key to insert a vertex on wire"), STAT_toolHelp );
                    m_mode = mode_editwire_segment;
                    break;
                default:
                    m_modehit = NULL;
                    m_modehitinfo = a2dHit();
            }
        }
    }  
    return true;
}

void a2dMasterTagGroups3::SetToolMode( bool ctrl, bool shift, bool alt )
{
    a2dCanvasObject* hit = m_modehit;    
    if ( !m_dragStarted )
    {
        //GetDrawingPart()->Update( a2dCANVIEW_UPDATE_PENDING );
        a2dIterC ic( GetDrawingPart() );
        ic.SetLayer( wxLAYER_ALL );
        ic.SetObjectFilter( new a2dCanvasObjectFilterLayerMask( wxLAYER_ALL, a2dCanvasOFlags::VISIBLE | a2dCanvasOFlags::SELECTABLE ) );
        m_hitinfo = a2dHitEvent( m_xwprev, m_ywprev, false, a2dCANOBJHITOPTION_NONE, true );
        m_hitinfo.m_option = a2dCANOBJHITOPTION_LAYERS | a2dCANOBJHITOPTION_NOROOT; // | a2dCANOBJHITOPTION_ALL;
        //at the top level the group its matrix is to be ignored.
        //Since it is normally NOT ignored within a2dCanvasObject, force an inverse.
        m_hitinfo.m_xyRelToChildren = true;

        m_mode = mode_none;
        m_modehit = 0;
        m_modehitinfo = a2dHit();
        a2dCanvasObject* top = GetDrawingPart()->GetShowObject();
        hit = top->IsHitWorld( ic, m_hitinfo );
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( top );
    }
    a2dPin* pin = NULL;
    if ( hit )
    {
        pin = wxDynamicCast( hit, a2dPin );
        if ( pin && pin->GetParent() == GetDrawingPart()->GetShowObject() )
            pin = NULL;
    }

    if ( !pin && !ctrl && shift )
    {
        if ( hit )
        {
            m_modehit = hit;
            SetStateString( _("click to select, double click to edit"), STAT_toolHelp );
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
        }
        else
        {
            m_modehit = NULL;
            SetStateString( _("click on object or drag rectangle to select extra objects"), STAT_toolHelp );
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW ) );
        }
        m_mode = mode_select;
    }      
    /*
    else if ( !pin && ctrl && shift )
    {
        if ( hit )
        {
            m_modehit = hit;
            SetStateString( _("click to select"), STAT_toolHelp );
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
        }
        else
        {
            m_modehit = NULL;
            SetStateString( _("move to object and click to select"), STAT_toolHelp );
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW ) );
        }
        m_mode = mode_cntrlselect;
    }      
    */
    else
    {
        if ( hit )
        {
            // Check if there is pin
            // This has highest priority
            a2dHit how2;

            a2dWirePolylineL* wire = wxDynamicCast( hit, a2dWirePolylineL );

            bool drawWireAtempt = false;

            if ( wire && wire->GetSelected() )
            {
                if ( m_drawWire )
                { 
                    if ( ctrl )
                        drawWireAtempt = true;
                    else
                        drawWireAtempt = false;
                }
                else
                { 
                    if ( ctrl )
                        drawWireAtempt = true;
                    else
                        drawWireAtempt = false;
                }
            }
            else
                drawWireAtempt = ( m_drawWire && !ctrl ) || ( !m_drawWire && ctrl );      

            if ( m_allowWireDraw && wire )
            {
                if ( drawWireAtempt ) 
                {
                    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
                    if ( restrict && 
                         ( m_hitinfo.m_how.m_stroke2 == a2dHit::stroke2_edgehor || 
                           m_hitinfo.m_how.m_stroke2 == a2dHit::stroke2_edgevert ||
                           m_hitinfo.m_how.m_stroke2 == a2dHit::stroke2_vertex
                         ) 
                       )
                    {
                        restrict->RestrictPoint( m_xwprev, m_ywprev );
                        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
                    }
                    // We do generate pins, but in case of restricted point, we might not succeed since the hit is not anymore on the wire.
                    GetDrawing()->GetHabitat()->GetConnectionGenerator()->
        	            GeneratePinsToConnect( GetDrawingPart(), wire, a2dPinClass::Any, a2d_GeneratePinsForStartWire, m_xwprev, m_ywprev );
                }
                else
                {
                    wire->RemovePins( true, true );
                }

                //we did hit a wire, still a pin at this location, can be on a wire or any other object, depending on drawing order.
                pin = GetDrawing()->GetHabitat()->GetConnectionGenerator()->
			                SearchPinForStartWire( GetDrawingPart()->GetShowObject(), m_xwprev, m_ywprev, a2dPinClass::Any, GetHitMargin() );

                if( pin )
                {
                    GetDrawingPart()->ToolWorldToMouse( m_xwprev, m_ywprev, m_generatedPinX, m_generatedPinY );
                    // if in draw wire mode, we must be able to start a wire on a pin
                    // Else we just drag pins, if not connected to a normal object ( so a wire to wire pin ).
                    if ( m_dragStarted && !pin->IsTemporaryPin() )
                    {
                        if ( !drawWireAtempt &&
                              ( pin->GetParent()->IsConnect() && !pin->FindNonWirePin() )
                           )
                        {
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                            SetStateString( _("drag the pin and reroute wires"), STAT_toolHelp );
                            m_mode = mode_move_pin;
                            m_modehit = pin;
                        }
                        if ( !drawWireAtempt &&
                              ( ( pin->GetParent()->IsConnect() && pin->FindNonWirePin() ) || !pin->GetParent()->IsConnect() )
                           )
                        {
                            if ( pin->GetConnectedPinsNr() <= 1 )
                            {
                                // wire drawing mode
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                                    SetStateString( _("drag to rewire"), STAT_toolHelp );
                                m_mode = mode_rewire_pin;
                                if ( !pin->GetParent()->IsConnect() )
                                    m_modehit = pin->FindWirePin();
                            }
                            else
                            {
                                // wire drawing mode
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                                SetStateString( _("draw a wire from pin"), STAT_toolHelp );
                                m_mode = mode_drawwire;
                                m_modehit = pin;
                            }
                        }
                    }
                    // If we know we are on an object pin, or at least a wire pin on that location, we can 
                    // start a wire without need of the drawWireAtempt.
                    // So we can start multiple wires on a normal pin.
                    else if ( !pin->IsTemporaryPin() && 
                              ( ( pin->GetParent()->IsConnect() && pin->FindNonWirePin() ) || !pin->GetParent()->IsConnect() )
                            )    
                    {
                        if ( pin->GetConnectedPinsNr() <= 1 )
                        {
                            // wire drawing mode
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                                SetStateString( _("draw a wire from pin, or drag to rewire"), STAT_toolHelp );
                            m_mode = mode_drawwire;
                        }
                        else
                        {
                            // wire drawing mode
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                            SetStateString( _("draw a wire from pin"), STAT_toolHelp );
                            m_mode = mode_drawwire;
                        }
                        m_modehit = pin;
                    }
                    else if (!m_dragStarted && !drawWireAtempt ) 
                    {
                        // wire drawing mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                        SetStateString( _("Drag the pin or start a wire with a click"), STAT_toolHelp );
                        m_mode = mode_drawwire;
                        m_modehit = pin;
                    }
                    else if ( drawWireAtempt || !m_dragStarted ) 
                    {
                        // wire drawing mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                        SetStateString( _("start a wire with a click"), STAT_toolHelp );
                        m_mode = mode_drawwire;
                        m_modehit = pin;
                    }
                }
                else
                {
                    if ( ! drawWireAtempt ) 
                    {
                        // a wire is not draggabble, but it might be editable, in that case we can drag segments and vertexes.
                        if( hit->GetEditable() )
                        {
                            SetWireCursor( wire, m_hitinfo, ctrl, shift, alt );
/* We do not really want to drag wire do we, anyway connect object are !GetDraggable()?
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
*/
                        }
                        else 
                        {
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                            SetStateString( _("click to select object"), STAT_toolHelp );
                            m_mode = mode_select;
                            m_modehit = hit;
                        }
                    }
                }

            }
            else //not a wire or not m_allowWireDraw
            {
                if ( m_allowWireDraw && !pin && !shift )
                {
                    //While moving event, there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
                    // m_connectionGenerator.
                    if ( m_generatePinsNonWire && ( m_generatePinsAlsoNonWireMode || drawWireAtempt ) ) 
                    {
                        GetDrawing()->GetHabitat()->GetConnectionGenerator()->
        	                GeneratePinsToConnect( GetDrawingPart(), hit, a2dPinClass::Any, a2d_GeneratePinsForStartWire, m_xwprev, m_ywprev );
                    }

                    pin = GetDrawing()->GetHabitat()->GetConnectionGenerator()->
				                SearchPinForStartWire( GetDrawingPart()->GetShowObject(), m_xwprev, m_ywprev, a2dPinClass::Any, GetHitMargin() );

                    if ( pin )
                    {
                        a2dWirePolylineL* wire = wxDynamicCast( pin->GetParent(), a2dWirePolylineL );
                        if ( wire && wire->GetSelected() )
                            pin = NULL;
                    }
                }

                // if not editing a wire, if pin start a wire draw
                if( m_allowWireDraw && pin )
                {
    				GetDrawingPart()->ToolWorldToMouse( m_xwprev, m_ywprev, m_generatedPinX, m_generatedPinY );
                    if ( m_dragStarted && !pin->IsTemporaryPin() )
                    {
                        if ( !drawWireAtempt && 
                              ( pin->GetParent()->IsConnect() && !pin->FindNonWirePin() ) )
                        {
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                            m_mode = mode_move_pin;
                            SetStateString( _("drag the pin and reroute wires"), STAT_toolHelp );
                            m_modehit = pin;
                        }
                        else if ( pin->GetParent()->IsConnect() && !pin->IsConnectedTo() )
                        {
                            // rewire pin mode
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                            SetStateString( _("Rewire this pin"), STAT_toolHelp );
                            m_mode = mode_rewire_pin;
                            m_modehit = pin;
                        }
                        else if ( pin->GetParent()->IsConnect() && !pin->FindNonWirePin() )
                        {
                            // wire drawing mode
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                            SetStateString( _("draw a wire from this pin"), STAT_toolHelp );
                            m_mode = mode_drawwire;
                            m_modehit = pin;
                        }
                        else if ( !pin->GetParent()->IsConnect()  && pin->IsConnectedTo() )
                        {
                            // rewire pin mode
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                            SetStateString( _("Rewire this pin"), STAT_toolHelp );
                            m_mode = mode_rewire_pin;
                            m_modehit = pin;
                        }
						else
						{
                            //we did hit a pin on the object, but do not use it when dragging it.
                            //Instead we do drag the object to which the pin belongs.
                            m_mode = mode_drag;
                            m_modehit = pin->GetParent();
                            SetStateString( _("drag object to new position"), STAT_toolHelp );
						}
                    }
                    else if ( !m_dragStarted )//&& !pin->IsTemporaryPin() ) we can have generated pins on NON wire objects also.
                    {
                        // wire drawing mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                        SetStateString( _("draw a wire from this pin"), STAT_toolHelp );
                        m_mode = mode_drawwire;
                        m_modehit = pin;
                    }
                }
                else 
                {
                    if( hit->GetDraggable() )
                    {
                        if ( hit->GetSelected() )
                        {
                            m_modehit = hit;
                            if ( ctrl && shift )
                            {
                                m_mode = mode_copymulti;
                                SetStateString( _("drag to copy selected objects"), STAT_toolHelp );
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Copy ) );
                            }
                            else //no shift no control                   
                            {
                                SetStateString( _("drag to move selected objects, double click to edit"), STAT_toolHelp );
                                m_mode = mode_dragmulti;
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                            }
                        }
                        else
                        {
                            if ( !m_dragStarted )
                            {
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                                SetStateString( _("click to select object, double click to edit"), STAT_toolHelp );
                                m_mode = mode_select;
                                m_modehit = hit;
                            }
                            else
                            {
                                m_mode = mode_drag;
                                m_modehit = hit;
                                SetStateString( _("drag to move object"), STAT_toolHelp );
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                            }
                        }
                    }
                    else 
                    {
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                        SetStateString( _("can not drag this object, double click to edit"), STAT_toolHelp );
                        m_mode = mode_select;
                        m_modehit = hit;
                    }
                }
            }                
        }
        else
        {
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW ) );
            SetStateString( _("click on object, drag rectangle to select objects"), STAT_toolHelp );
            m_mode = mode_select;
            m_modehit = hit;
        }
    }
}

void a2dMasterTagGroups3::OnMouseEvent( wxMouseEvent& event )
{
    //MouseDump( event, "OnM " );
	//wxLogDebug( "%d %d %d %d ", m_x, m_y, m_dragstartx, m_dragstarty );

    m_shiftWasDown = event.ShiftDown();
    m_ctrlWasDown = event.ControlDown();
    m_altWasDown = event.AltDown();

    if ( !m_active )
    {
        event.Skip();
        return;
    }
    m_x = event.GetX();
    m_y = event.GetY();

    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );
    //wxLogDebug( wxT( "   mx =%d, my =%d,  wx =%12.6lf, wy =%12.6lf" ), m_x, m_y, m_xwprev, m_ywprev );

    SelectedStatus();

    if ( event.LeftDClick() &&
         !m_toolBusy //&& 
         //no need !( event.ControlDown() || event.ShiftDown() ) 
       )
    {
        m_hadDoubleClick = true;
        a2dPin* pin = wxDynamicCast( m_modehit, a2dPin );
        // if in draw wire mode a double click is on a pin, but edit style should be on wire.
        if( pin )
            m_modehit = pin->GetParent();

		// If selection did take place at LeftDown, undo that one here, as if it did not happen,
		// since it should not have happened at Double Click.
        if ( !m_shiftWasDown && !m_modehitLastSelectState )
            DeselectAll();
        else if ( m_modehit && m_modehitLastSelectState )
        {
            if ( m_selectedObjects )
            {
                forEachIn( a2dCanvasObjectList, m_selectedObjects )
                {
                    a2dCanvasObject* obj = *iter;
                    obj->SetSelected( true ); 
                }
                delete m_selectedObjects;
                m_selectedObjects = NULL;
            }
            m_modehit->SetSelected( m_modehitLastSelectState );
			SetLastSelected( m_modehit, m_modehitLastSelectState );
        }

        bool moreSelectedObjects = m_parentobject->GetChildObjectList()->CollectObjects( NULL, "", a2dCanvasOFlags::SELECTED ) > 1;
        bool singleedit = false;
        if ( m_AllowDoubleClickModifier && m_shiftWasDown && !moreSelectedObjects )
            singleedit = true;

        EditDlgOrHandles( m_modehit, singleedit, m_drawWire );
    }
    else if ( event.Dragging() && m_toolBusy && !m_dragStarted )
    {
        //we assume dragging starts for real if 5 pixels shift.
        if ( abs( m_x - m_dragstartx ) >= 5 || abs( m_y - m_dragstarty ) >= 5 )
        {
            m_dragStarted = true;
            // If selection did take place at LeftDown, undo that here, as if it did not happen,
            // since it should not have happened at drag.
            if ( m_modehit )
            {
                if ( m_selectedObjects )
                {
                    forEachIn( a2dCanvasObjectList, m_selectedObjects )
                    {
                        a2dCanvasObject* obj = *iter;
                        obj->SetSelected( true ); 
                    }
                    delete m_selectedObjects;
                    m_selectedObjects = NULL;
                }
                m_modehit->SetSelected( m_modehitLastSelectState );
				SetLastSelected( m_modehit, m_modehitLastSelectState );
            }
            SetToolMode( m_ctrlWasDown, m_shiftWasDown, m_altWasDown );
            m_toolBusy = false;
            switch( m_mode )
            {
				case mode_none:
				{
                    m_dragStarted = false;
                    m_toolBusy = false;
                    break;
				}
				case mode_drawwire:
				{
                    m_dragStarted = false;
                    m_toolBusy = false;
                    break;
				}
                case mode_cntrlselect:
                    DeselectAll();
                    m_toolBusy = true;
                    event.Skip();
                    break;
                case mode_select:
                    break;
                case mode_dragmulti:
                {
                    m_dragStarted = false;
                    m_x = m_dragstartx;
                    m_y = m_dragstarty;
                    PushDragMultiTool( m_modehit );
                    //event.Skip();
                    break;
                }
                case mode_copymulti:
                {
                    m_dragStarted = false;
                    m_x = m_dragstartx;
                    m_y = m_dragstarty;
                    PushCopyMultiTool( m_modehit );
                    //event.Skip();
                    break;
                }
                case mode_drag:
                {
                    m_dragStarted = false;
                    m_x = m_dragstartx;
                    m_y = m_dragstarty;
                    PushDragTool( m_modehit );
                    //event.Skip();
                    break;
                }
                case mode_copy:
                {
                    m_dragStarted = false;
                    m_x = m_dragstartx;
                    m_y = m_dragstarty;
                    PushCopyTool( m_modehit );
                    //event.Skip();
                    break;
                }
                case mode_move_pin:
                {
                    m_dragStarted = false;
                    PushMovePinTool( m_modehit );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
                case mode_rewire_pin:
                {
                    m_dragStarted = false;
                    PushRewirePinTool( m_modehit );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
				case mode_editwire_vertex:
                {
                    m_dragStarted = false;
					PushEditWireVertexTool( m_modehit, m_modehitinfo.m_index );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
				case mode_editwire_segmenthorizontal:
                {
                    m_dragStarted = false;
					PushEditWireSegmentHorizontalTool( m_modehit, m_modehitinfo.m_index );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
				case mode_editwire_segmentvertical:
                {
                    m_dragStarted = false;
					PushEditWireSegmentVerticalTool( m_modehit, m_modehitinfo.m_index );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
				case mode_editwire_segmentinsert:
                {
                    m_dragStarted = false;
					PushEditWireSegmentInsertTool( m_modehit, m_modehitinfo.m_index );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
				case mode_editwire_segment:
                {
                    m_dragStarted = false;
					PushEditSegmentTool( m_modehit, m_modehitinfo.m_index );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
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
        SetToolMode( m_ctrlWasDown, m_shiftWasDown, m_altWasDown );
        event.Skip();
    }
    else if( event.LeftDown() && !m_toolBusy )
    {
        if ( m_selectedObjects )
        {
            delete m_selectedObjects;
            m_selectedObjects = NULL;
        }

        //preserve selecte state for double click or drag if it happens
        m_modehitLastSelectState = false;
        if ( m_modehit )
            m_modehitLastSelectState = m_modehit->GetSelected();
		
        SetStateString( "" );
        m_dragStarted = false;
        m_xprev = m_x;
        m_yprev = m_y;
        m_dragstartx = m_x;
        m_dragstarty = m_y;

        if( !m_modehit )
		{
            StopDlgEditTool( m_modehit );
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
            m_mode = mode_select;
            event.Skip();
            m_toolBusy = false;
			return;
	    }
        else
        {
            StopDlgEditTool( m_modehit );
        }

        switch( m_mode )
        {
            case mode_select:
            case mode_dragmulti:
            case mode_copymulti:
            case mode_editwire_segmenthorizontal:
            case mode_editwire_segmentvertical:
            case mode_editwire_segmentinsert:
            case mode_editwire_segment:
                m_toolBusy = true; 

                if ( m_shiftWasDown )
                {
                    if ( m_modehit && m_modehit->GetSelectable() )
                    {
                        if ( m_modehit->IsSelected() )
						{
                            m_modehit->SetSelected( false );
							if ( m_modehit->GetSelected2() )
							{
								SetLastSelected( m_modehit, false );
								SetLastSelected( GetTopLeftSelected(), true );
							}
						}
                        else
						{
                            m_modehit->SetSelected( true );
							SetLastSelected( m_modehit, true );
						}
                    }
                }
                else
                {
                    if ( m_selectMoreAtShift || !m_modehit )
                    {
                        m_selectedObjects = m_parentobject->GetChildObjectList()->Clone( a2dCanvasOFlags::SELECTED, a2dObject::clone_flat );
                        DeselectAll(); 
                    }
                    if ( m_modehit && m_modehit->GetSelectable() )
                    {
                        if ( m_drawWire )
						{
                            m_modehit->SetSelected( true );
							SetLastSelected( m_modehit, true );
						}
                        else //is conflicting with starting a wire
						{
							m_modehit->SetSelected( !m_modehitLastSelectState );
							SetLastSelected( m_modehit, !m_modehitLastSelectState );
						}
                    }
                }

                break;
            case mode_move_pin:
                m_toolBusy = true;
                break;
            case mode_rewire_pin:
                m_toolBusy = true;
                break;
            case mode_drawwire:
                m_toolBusy = true;
                break;
			case mode_editwire_vertex:
            {
				if ( m_altWasDown )
				{
					PushEditWireVertexTool( m_modehit, m_modehitinfo.m_index );
					event.Skip();
				}
				else
				{
	                m_toolBusy = true;
				}
                break;
            }
			case mode_dlgedit:
            {
    			PushDlgEditTool( m_modehit );
	            break;
            }
            default:
                // not know until a drag started or not.
                m_mode = mode_none;
                m_toolBusy = true;
                event.Skip();
        }
    }
    else if( event.LeftUp() && m_toolBusy )
    {
        m_toolBusy = false;
        if( m_dragStarted )
        {   
            m_dragStarted = false; //end of drag
            m_mode = mode_none;
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
        }
        else
        {
            //m_dragStarted is only true if drag was at least 5 pixels.
            if ( m_mode == mode_drawwire )
            {
	            m_toolBusy = false;

                //When pin was generated on a wire at mouse moving), use it here.
                a2dPin* pin = wxDynamicCast( m_modehit, a2dPin );
                // if in draw wire mode, we must be able to start a wire on a pin
                if( pin )
                {
                    // wire drawing mode
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                    m_mode = mode_drawwire;
                    m_modehit = pin;
                    PushDrawWireTool( m_modehit );

                    //create mouse events as if a wire was started using mouse in wire tool.
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    mouseevent.m_x = m_generatedPinX;
                    mouseevent.m_y = m_generatedPinY;
                    mouseevent.m_controlDown = true;
                    mouseevent.m_leftDown = true;
                    mouseevent.SetEventObject(this);
                    mouseevent.SetId( GetDrawingPart()->GetDisplayWindow()->GetId() );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );

                    wxMouseEvent mouseevent2( wxEVT_LEFT_UP );
                    mouseevent2.m_x = m_generatedPinX;
                    mouseevent2.m_y = m_generatedPinY;
                    mouseevent2.m_controlDown = true;
                    mouseevent2.m_leftDown = true;
                    mouseevent2.SetEventObject(this);
                    mouseevent2.SetId( GetDrawingPart()->GetDisplayWindow()->GetId() );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent2 );
                }
            }
        }
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




IMPLEMENT_CLASS( a2dMasterSingleClick , a2dMasterDrawBase )

BEGIN_EVENT_TABLE( a2dMasterSingleClick , a2dMasterDrawBase )
    EVT_MOUSE_EVENTS( a2dMasterSingleClick ::OnMouseEvent )
    EVT_CHAR( a2dMasterSingleClick ::OnChar )
    EVT_KEY_DOWN( a2dMasterSingleClick ::OnKeyDown )
    EVT_KEY_UP( a2dMasterSingleClick ::OnKeyUp )
    EVT_COM_EVENT( a2dMasterSingleClick::OnComEvent ) 
END_EVENT_TABLE()

a2dMasterSingleClick ::a2dMasterSingleClick ( a2dStToolContr* controller ): a2dMasterDrawBase( controller )
{
    controller->SetDefaultBehavior( controller->GetDefaultBehavior() | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_Select  );
    m_drawWire = false;
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );
}

a2dMasterSingleClick ::~a2dMasterSingleClick ()
{
    a2dCanvasGlobals->DisconnectEvent( a2dEVT_COM_EVENT, this );
}

wxString a2dMasterSingleClick ::GetToolString() const
{
	wxString str =  GetClassInfo()->GetClassName();
	if ( m_drawWire )
		return str + " Wires";
	if ( m_dlgOrEdit )
		return str + " Dlg";
	return str;
}

void a2dMasterSingleClick ::Render()
{
}

void a2dMasterSingleClick::DoStopTool( bool abort )
{
    a2dMasterDrawBase::DoStopTool( abort );
    m_toolBusy = false;
}

void a2dMasterSingleClick::AbortBusyMode()
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

void a2dMasterSingleClick::SetActive( bool active )
{
    a2dStTool::SetActive( active );
    m_pending = true;

    if ( active )
    {
        m_mode = mode_select;
        m_modehit = NULL;
    }
}

void a2dMasterSingleClick::PushEditTool( a2dCanvasObject* hit )
{
    a2dSmrtPtr< a2dObjectEditTool > tool = new a2dObjectEditTool( m_stcontroller );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditSegment ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditSegment ) );

    if ( hit )
    {

    }
    //tool->SetEvtHandler( m_eventHandler  );
    
    m_stcontroller->PushTool( tool );
    tool->StartToEdit( hit );
}

void a2dMasterSingleClick::OnComEvent( a2dComEvent& event )
{
    if ( GetEvtHandlerEnabled() )
    {
    	a2dSignal comId = event.GetId();

        if ( event.GetId() == a2dBaseTool::sig_toolPoped )
        {
        }

        a2dStTool* tool = wxDynamicCast( m_stcontroller->GetFirstTool(), a2dStTool );
        if ( tool && tool->GetActive() && tool->GetEvtHandlerEnabled() && tool->GetDrawingPart() )
        {
            bool OnSelected = false;

            if ( /*IsShownm_styleDlg &&*/ m_modehit )
            {
                a2dCanvasObjectList objects;
                m_parentobject->CollectObjects( &objects, wxT( "" ), a2dCanvasOFlags::SELECTED );
                if ( objects.size() > 0 )
                    OnSelected = true;
            }
            if ( !OnSelected )
            {
                a2dCanvasCommandProcessor* docCmdh = tool->GetCanvasCommandProcessor();
                a2dFill f = GetDrawing()->GetHabitat()->GetFill();
                if ( comId == a2dHabitat::sig_changedFill && ! tool->GetFill().IsSameAs( f ) )
                {
                    if ( m_modehit )
                        docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( m_modehit, f ) );
                }
                a2dStroke s = GetDrawing()->GetHabitat()->GetStroke();
                if ( comId == a2dHabitat::sig_changedStroke && ! tool->GetStroke().IsSameAs( s ) )
                {
                    if ( m_modehit )
                        docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( m_modehit, s ) );
                }
            }
            else
            {
            if ( 
                comId == a2dHabitat::sig_changedFill ||
                comId == a2dHabitat::sig_changedStroke ||
                comId == a2dHabitat::sig_changedLayer
               )
            {
                a2dCanvasCommandProcessor* docCmdh = tool->GetCanvasCommandProcessor();
                a2dFill f = GetDrawing()->GetHabitat()->GetFill();
                if ( comId == a2dHabitat::sig_changedFill )
                {
                    docCmdh->Submit( new a2dCommand_SetFillStrokeMask(  tool->GetDrawingPart()->GetShowObject() ) );
                }
                a2dStroke s = GetDrawing()->GetHabitat()->GetStroke();
                if ( comId == a2dHabitat::sig_changedStroke )
                {
                    docCmdh->Submit( new a2dCommand_SetFillStrokeMask(  tool->GetDrawingPart()->GetShowObject() ) );
                }
                wxUint16 layer = GetDrawing()->GetHabitat()->GetLayer();
                if ( comId == a2dHabitat::sig_changedLayer )
                {
                    docCmdh->Submit( new a2dCommand_SetLayerMask( tool->GetDrawingPart()->GetShowObject(), layer ) );
                }
            }
            }
        }
    }


    event.Skip();
} 

void a2dMasterSingleClick::OnChar( wxKeyEvent& event )
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

void a2dMasterSingleClick ::OnKeyDown( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    switch( event.GetKeyCode() )
    {
        case 'w':
        case 'W':
        {
            m_drawWire = !m_drawWire;
            SetToolMode( event.m_controlDown, event.m_shiftDown, event.AltDown() );
            break;
        }
        default:
            if ( event.m_controlDown || event.m_shiftDown || event.AltDown() )
                SetToolMode( event.m_controlDown, event.m_shiftDown, event.AltDown() );
            event.Skip();
    }
}

void a2dMasterSingleClick ::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    switch( event.GetKeyCode() )
    {
        case 'w':
        case 'W':
        {
            SetToolMode( event.m_controlDown, event.m_shiftDown, event.AltDown() );
            break;
        }
        default:
            if ( !event.m_controlDown || !event.m_shiftDown || !event.AltDown() )
                SetToolMode( event.m_controlDown, event.m_shiftDown, event.AltDown() );
            event.Skip();
    }
}

bool a2dMasterSingleClick::SetWireCursor( a2dWirePolylineL* wire, const a2dHitEvent& hitinfo, bool ctrl, bool shift, bool alt )
{
    a2dHit how2;
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
        if ( !alt )
        {
            switch( how2.m_stroke2 )
            {
                case a2dHit::stroke2_vertex:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireVertex ) );
                    SetStateString( _("drag to move vertex on the wire"), STAT_toolHelp );
                    m_mode = mode_editwire_vertex;
                    break;
                case a2dHit::stroke2_edgehor:
                case a2dHit::stroke2_edgevert:
                case a2dHit::stroke2_edgeother:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_WireSegmentInsert ) );
                    SetStateString( _("drag to insert vertex on the wire"), STAT_toolHelp );
                    m_mode = mode_editwire_segmentinsert;
                    break;
                default:
                    m_modehit = NULL;
                    m_modehitinfo = a2dHit();
            }
        }
        else
        {
            switch( how2.m_stroke2 )
            {
                case a2dHit::stroke2_vertex:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireDeleteVertex ) );
                    SetStateString( _("click to remove vertex from wire"), STAT_toolHelp );
                    m_mode = mode_editwire_vertex;
                    break;
                case a2dHit::stroke2_edgehor:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentHorizontal ) );
                    SetStateString( _("drag to move segment of the wire"), STAT_toolHelp );
                    m_mode = mode_editwire_segmenthorizontal;
                    break;
                case a2dHit::stroke2_edgevert:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentVertical ) );
                    SetStateString( _("drag to move segment of the wire"), STAT_toolHelp );
                    m_mode = mode_editwire_segmentvertical;
                    break;
                case a2dHit::stroke2_edgeother:
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegment ) );
                    SetStateString( _("drag to move segment, Alt key to insert a vertex on wire"), STAT_toolHelp );
                    m_mode = mode_editwire_segment;
                    break;
                default:
                    m_modehit = NULL;
                    m_modehitinfo = a2dHit();
            }
        }
    }  
    return true;
}

void a2dMasterSingleClick::SetToolMode( bool ctrl, bool shift, bool alt )
{
    a2dCanvasObject* hit = m_modehit;    
    if ( !m_dragStarted )
    {
        //GetDrawingPart()->Update( a2dCANVIEW_UPDATE_PENDING );
        a2dIterC ic( GetDrawingPart() );
        ic.SetLayer( wxLAYER_ALL );
        ic.SetObjectFilter( new a2dCanvasObjectFilterLayerMask( wxLAYER_ALL, a2dCanvasOFlags::VISIBLE | a2dCanvasOFlags::SELECTABLE ) );
        m_hitinfo = a2dHitEvent( m_xwprev, m_ywprev, false, a2dCANOBJHITOPTION_NONE, true );
        m_hitinfo.m_option = a2dCANOBJHITOPTION_LAYERS;
        //at the top level the group its matrix is to be ignored.
        //Since it is normally NOT ignored within a2dCanvasObject, force an inverse.
        m_hitinfo.m_xyRelToChildren = true;

        m_mode = mode_none;
        m_modehit = 0;
        m_modehitinfo = a2dHit();
        a2dCanvasObject* top = GetDrawingPart()->GetShowObject();
        hit = top->IsHitWorld( ic, m_hitinfo );
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( top );
    }
    a2dPin* pin = NULL;
    if ( hit )
    {
        pin = wxDynamicCast( hit, a2dPin );
        if ( pin )
            pin = wxDynamicCast( hit, a2dPin );
    }

    if ( !pin && !ctrl && shift )
    {
        if ( hit )
        {
            m_modehit = hit;
            SetStateString( _("click to select"), STAT_toolHelp );
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
        }
        else
        {
            m_modehit = NULL;
            SetStateString( _("move to object and click to select"), STAT_toolHelp );
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW ) );
        }
        m_mode = mode_select;
    }      
    else
    {
        if ( hit )
        {
            // Check if there is pin
            // This has highest priority
            a2dHit how2;

            a2dWirePolylineL* wire = wxDynamicCast( hit, a2dWirePolylineL );

            bool drawWireAtempt = false;

            if ( wire && wire->GetSelected() )
            {
                if ( m_drawWire )
                { 
                    if ( ctrl )
                        drawWireAtempt = true;
                    else
                        drawWireAtempt = false;
                }
                else
                { 
                    if ( ctrl )
                        drawWireAtempt = true;
                    else
                        drawWireAtempt = false;
                }
            }
            else
                drawWireAtempt = ( m_drawWire && !ctrl ) || ( !m_drawWire && ctrl );      

            if ( wire && wire->GetSelected() && drawWireAtempt ) 
            {
                m_modehit = wire;
                m_mode = mode_edit;
                //SetWireCursor( wire, m_hitinfo, ctrl, shift, alt );
            }
            else if ( wire )
            {
                if ( drawWireAtempt ) 
                {
                    GetDrawing()->GetHabitat()->GetConnectionGenerator()->
        	            GeneratePinsToConnect( GetDrawingPart(), wire, a2dPinClass::Any, a2d_GeneratePinsForStartWire, m_xwprev, m_ywprev );
                }
                else
                    wire->RemovePins( true, true );


                //we did hit a wire, still a pin at this location, can be of a wire or anay other object, depending on drawing order.
                pin = GetDrawing()->GetHabitat()->GetConnectionGenerator()->
			                SearchPinForStartWire( GetDrawingPart()->GetShowObject(), m_xwprev, m_ywprev, a2dPinClass::Any, GetHitMargin() );




                if( pin )
                {
                    // if in draw wire mode, we must be able to start a wire on a pin
                    // Else we just drag pins, if not connected to a normal object ( so a wire to wire pin ).
                    if ( m_dragStarted && !pin->IsTemporaryPin() )
                    {
                        if ( !drawWireAtempt && ( pin->GetParent()->IsConnect() && !pin->FindNonWirePin() ) )
                        {
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                            SetStateString( _("drag the pin and reroute wires"), STAT_toolHelp );
                            m_mode = mode_move_pin;
                            m_modehit = pin;
                        }
                        if ( ! drawWireAtempt &&
                              ( ( pin->GetParent()->IsConnect() && pin->FindNonWirePin() ) || !pin->GetParent()->IsConnect() )
                           )
                        {
                            if ( pin->GetConnectedPinsNr() <= 1 )
                            {
                                // wire drawing mode
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                                    SetStateString( _("drag to rewire"), STAT_toolHelp );
                                m_mode = mode_rewire_pin;
                                if ( !pin->GetParent()->IsConnect() )
                                    m_modehit = pin->FindWirePin();
                            }
                            else
                            {
                                // wire drawing mode
                                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                                SetStateString( _("draw a wire from pin"), STAT_toolHelp );
                                m_mode = mode_drawwire;
                                m_modehit = pin;
                            }
                        }
                    }
                    // If we know we are on an object pin, or at least a wire pin on that location, we can 
                    // start a wire without extra Ctrl 
                    // So we can start multiple wires on a normal pin.
                    else if ( !pin->IsTemporaryPin() && 
                              ( ( pin->GetParent()->IsConnect() && pin->FindNonWirePin() ) || !pin->GetParent()->IsConnect() )
                            )    
                    {
                        if ( pin->GetConnectedPinsNr() <= 1 )
                        {
                            // wire drawing mode
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                                SetStateString( _("draw a wire from pin, or drag to rewire"), STAT_toolHelp );
                            m_mode = mode_drawwire;
                        }
                        else
                        {
                            // wire drawing mode
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                            SetStateString( _("draw a wire from pin"), STAT_toolHelp );
                            m_mode = mode_drawwire;
                        }
                        m_modehit = pin;
                    }
                    else if (!m_dragStarted && !drawWireAtempt ) 
                    {
                        // wire drawing mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                        SetStateString( _("Drag the pin or start a wire with a click"), STAT_toolHelp );
                        m_mode = mode_drawwire;
                        m_modehit = pin;
                    }
                    else if ( drawWireAtempt || !m_dragStarted ) 
                    {
                        // wire drawing mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                        SetStateString( _("start a wire with a click"), STAT_toolHelp );
                        m_mode = mode_drawwire;
                        m_modehit = pin;
                    }
                }
                else
                {
                    if ( drawWireAtempt ) 
                    {
                        // wire drawing mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                        SetStateString( _("draw a wire"), STAT_toolHelp );
                        m_mode = mode_drawwire;
                        m_modehit = pin;
                    }
                    else
                    {
                        if ( !shift )
                        {
	                        SetWireCursor( wire, m_hitinfo, ctrl, shift, alt );
                        }
                        else
                        {
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                            SetStateString( _("click to select or deselect"), STAT_toolHelp );
                            m_mode = mode_select;
                            m_modehit = hit;
                        }
                    }
                }

            }
            else //not a wire
            {
                if ( !pin && !shift )
                {
                    //While moving event, there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
                    // m_connectionGenerator.
                    if ( drawWireAtempt ) 
                    {
                        GetDrawing()->GetHabitat()->GetConnectionGenerator()->
        	                GeneratePinsToConnect( GetDrawingPart(), hit, a2dPinClass::Any, a2d_GeneratePinsForStartWire, m_xwprev, m_ywprev );
                    }

                    pin = GetDrawing()->GetHabitat()->GetConnectionGenerator()->
				                SearchPinForStartWire( GetDrawingPart()->GetShowObject(), m_xwprev, m_ywprev, a2dPinClass::Any, GetHitMargin() );

                    if ( pin )
                    {
                        a2dWirePolylineL* wire = wxDynamicCast( pin->GetParent(), a2dWirePolylineL );
                        if ( wire && wire->GetSelected() )
                            pin = NULL;
                    }
                }

                // if not editing a wire, if pin start a wire draw
                if( pin )
                {
                    if ( m_dragStarted && !pin->IsTemporaryPin() )
                    {
                        if ( !drawWireAtempt && 
                              ( pin->GetParent()->IsConnect() && !pin->FindNonWirePin() ) )
                        {
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                            m_mode = mode_move_pin;
                            SetStateString( _("drag the pin and reroute wires"), STAT_toolHelp );
                            m_modehit = pin;
                        }
                        else if ( pin->GetParent()->IsConnect() && !pin->IsConnectedTo() )
                        {
                            // rewire pin mode
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                            SetStateString( _("Rewire this pin"), STAT_toolHelp );
                            m_mode = mode_rewire_pin;
                            m_modehit = pin;
                        }
                        else if ( pin->GetParent()->IsConnect() && !pin->FindNonWirePin() )
                        {
                            // wire drawing mode
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                            SetStateString( _("draw a wire from this pin"), STAT_toolHelp );
                            m_mode = mode_drawwire;
                            m_modehit = pin;
                        }
                        else if ( !pin->GetParent()->IsConnect()  && pin->IsConnectedTo() )
                        {
                            // rewire pin mode
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                            SetStateString( _("Rewire this pin"), STAT_toolHelp );
                            m_mode = mode_rewire_pin;
                            m_modehit = pin;
                        }
						else
						{
                            m_mode = mode_drawwire;
                            m_modehit = pin;
						}
                    }
                    else if ( !m_dragStarted )//&& !pin->IsTemporaryPin() ) we can have generated pins on NON wire objects also.
                    {
                        // wire drawing mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                        SetStateString( _("draw a wire from this pin"), STAT_toolHelp );
                        m_mode = mode_drawwire;
                        m_modehit = pin;
                    }
                }
                else 
                {
                    if ( !shift ) 
                    {
						if( hit->GetDraggable() )
						{
							if ( hit->GetSelected() )
							{
								m_modehit = hit;
								if ( ctrl && shift )
								{
									m_mode = mode_copymulti;
									SetStateString( _("drag to copy selected objects"), STAT_toolHelp );
									GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Copy ) );
								}
								else //no shift no control                   
								{
									SetStateString( _("drag to move selected objects"), STAT_toolHelp );
									m_mode = mode_dragmulti;
									GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
								}
							}
							else
							{
								if ( !m_dragStarted )
								{
									GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
									SetStateString( _("click to select object, double click to edit"), STAT_toolHelp );
									m_mode = mode_edit;
									m_modehit = hit;
								}
								else
								{
									m_mode = mode_drag;
									m_modehit = hit;
									SetStateString( _("drag to move object"), STAT_toolHelp );
									GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
								}
							}
						}
                    }
                    else
                    {
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select2 ) );
                        SetStateString( _("click to (de)select, double click for dlg"), STAT_toolHelp );
                        m_mode = mode_select;
                        m_modehit = hit;
                    }
                }
            }                
        }
        else
        {
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW ) );
            SetStateString( _("click on object, to edit or drag"), STAT_toolHelp );
            m_mode = mode_select;
            m_modehit = hit;
        }
    }
}

void a2dMasterSingleClick ::OnMouseEvent( wxMouseEvent& event )
{
    MouseDump( event, "OnM " );

    if ( !m_active )
    {
        event.Skip();
        return;
    }
    m_x = event.GetX();
    m_y = event.GetY();

    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dBoundingBox bbox;
    a2dCanvasObjectList* objects = m_parentobject->GetChildObjectList();
    forEachIn( a2dCanvasObjectList, objects )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj->GetRelease() || !obj->IsVisible() || !obj->GetSelected() )
            continue;
        bbox.Expand( obj->GetBbox() );
	}

	if ( 0 )//|| bbox.GetValid() )
	{
		double unitScale = GetDrawing()->GetUnitsScale();
		wxString state, form;
		form = m_stcontroller->GetFormat() + " " + m_stcontroller->GetFormat();
		state.Printf( form, bbox.GetMinX()*unitScale, bbox.GetMinY()*unitScale );
		SetStateString( state, 10 );
		form = _T("width = ") + m_stcontroller->GetFormat() + _T(" height = ") + m_stcontroller->GetFormat();
		state.Printf( form, bbox.GetWidth()*unitScale, bbox.GetHeight()*unitScale );
		SetStateString( state, 11 );
	}

    if ( event.LeftDClick() &&
         !m_toolBusy //&& 
         //noneed !( event.ControlDown() || event.ShiftDown() ) 
       )
    {
        m_hadDoubleClick = true;
        a2dCanvasObject* hit = m_modehit; //reuse from the LeftDown event.
        if ( hit )
        {
            if ( m_dlgOrEdit && ! hit->GetFixedStyle() && !event.ShiftDown() || 
                 !m_dlgOrEdit && ! hit->GetFixedStyle() && event.ShiftDown()
               )
            {
                if ( !m_dlgOrEditModal )
                    GetDrawing()->GetCanvasCommandProcessor()->ShowDlgStyle( true );
                else
                {
				    a2dStyleDialog styleDlg( GetDrawing()->GetHabitat(), NULL, wxDEFAULT_DIALOG_STYLE | wxDIALOG_NO_PARENT | wxMINIMIZE_BOX | wxMAXIMIZE_BOX, true );
				    styleDlg.SetFill( hit->GetFill() );
				    styleDlg.SetStroke( hit->GetStroke() );
				    if ( styleDlg.ShowModal() == wxID_OK )
				    {
				        hit->SetFill( styleDlg.GetFill() );
				        hit->SetStroke( styleDlg.GetStroke() );
				    }
                }
            }
            else
            {
                a2dWirePolylineL* wire = wxDynamicCast( hit, a2dWirePolylineL );
                m_modehit = hit;

                if ( (wire && wire->GetSelected() ) || (wire && !m_drawWire ) || !wire )
                {
                    hit->SetSelected( m_modehitLastSelectState );
					SetLastSelected( hit, m_modehitLastSelectState );
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                    m_mode = mode_edit;
                    PushEditTool( m_modehit );
                }
            }
        }
    }
    else if ( event.Dragging() && m_toolBusy && !m_dragStarted )
    {
        //we assume dragging starts for real if 5 pixels shift.
        if ( abs( m_x - m_dragstartx ) >= 5 || abs( m_y - m_dragstarty ) >= 5 )
        {
            m_dragStarted = true;
            SetToolMode( event.ControlDown(), event.ShiftDown(), event.AltDown() );
            m_toolBusy = false;
            switch( m_mode )
            {
				case mode_none:
				{
                    m_dragStarted = false;
                    m_toolBusy = false;
                    break;
				}
                case mode_dragmulti:
                {
                    m_dragStarted = false;
                    PushDragMultiTool( m_modehit );
                    //event.Skip();
                    break;
                }
                case mode_copymulti:
                {
                    m_dragStarted = false;
                    PushCopyMultiTool( m_modehit );
                    //event.Skip();
                    break;
                }
				case mode_drawwire:
				{
                    m_dragStarted = false;
                    m_toolBusy = false;
                    break;
				}
                case mode_drag:
                {
                    m_dragStarted = false;
                    PushDragTool( m_modehit );
                    //event.Skip();
                    break;
                }
                case mode_move_pin:
                {
                    m_dragStarted = false;
                    PushMovePinTool( m_modehit );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
                case mode_rewire_pin:
                {
                    m_dragStarted = false;
                    PushRewirePinTool( m_modehit );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
				case mode_editwire_vertex:
                {
                    m_dragStarted = false;
					PushEditWireVertexTool( m_modehit, m_modehitinfo.m_index );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
				case mode_editwire_segmenthorizontal:
                {
                    m_dragStarted = false;
					PushEditWireSegmentHorizontalTool( m_modehit, m_modehitinfo.m_index );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
				case mode_editwire_segmentvertical:
                {
                    m_dragStarted = false;
					PushEditWireSegmentVerticalTool( m_modehit, m_modehitinfo.m_index );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
				case mode_editwire_segmentinsert:
                {
                    m_dragStarted = false;
					PushEditWireSegmentInsertTool( m_modehit, m_modehitinfo.m_index );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
                    break;
                }
				case mode_editwire_segment:
                {
                    m_dragStarted = false;
					PushEditSegmentTool( m_modehit, m_modehitinfo.m_index );
                    wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                    InitMouseEvent( mouseevent, m_dragstartx, m_dragstarty, event );
                    m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );
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
        SetToolMode( event.ControlDown(), event.ShiftDown(), event.AltDown() );
        event.Skip();
    }
    else if( event.LeftDown() && !m_toolBusy )
    {
        SetStateString( "" );
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
            case mode_edit:
                m_toolBusy = true;
                break;
            case mode_select:
                m_toolBusy = true;
                break;
            case mode_move_pin:
                m_toolBusy = true;
                break;
            case mode_rewire_pin:
                m_toolBusy = true;
                break;
            case mode_drawwire:
                m_toolBusy = true;
                break;
            case mode_editwire_vertex:
            {
				if ( event.AltDown() )
				{
					PushEditWireVertexTool( m_modehit, m_modehitinfo.m_index );
					event.Skip();
				}
				else
	                m_toolBusy = true;
                break;
            }
            case mode_editwire_segmenthorizontal:
            case mode_editwire_segmentvertical:
            case mode_editwire_segmentinsert:
            case mode_editwire_segment:
                m_toolBusy = true;
                break;
            default:
                // not know until a drag started or not.
                m_mode = mode_none;
                m_toolBusy = true;
                event.Skip();
        }
    }
    else if( event.LeftUp() && m_toolBusy && m_dragStarted )
    {
        m_toolBusy = false;
        m_dragStarted = false; //end of drag
        m_mode = mode_none;
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

        if ( m_mode == mode_select || ( m_mode == mode_drawwire && event.ShiftDown() ) )
        {
			if ( !m_modehit )
                DeselectAll();
            if ( m_modehit && m_modehit->GetSelectable() )
            {
                m_modehit->SetSelected( !m_modehitLastSelectState );
				SetLastSelected( m_modehit, !m_modehitLastSelectState );
			}
		}
        else if ( m_mode == mode_drawwire ) //&&  event.ControlDown() )
        {
	        m_toolBusy = false;
            //When pin was generated on a wire at mouse moving), use it here.
            a2dPin* pin = wxDynamicCast( m_modehit, a2dPin );
            // if in draw wire mode, we must be able to start a wire on a pin
            if( pin )
            {
                // wire drawing mode
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                m_mode = mode_drawwire;
                m_modehit = pin;
                PushDrawWireTool( m_modehit );

                wxMouseEvent mouseevent( wxEVT_LEFT_DOWN );
                mouseevent.m_x = m_dragstartx;
                mouseevent.m_y = m_dragstarty;
                mouseevent.m_controlDown = true;
                mouseevent.m_leftDown = true;
                mouseevent.SetEventObject(this);
                mouseevent.SetId( GetDrawingPart()->GetDisplayWindow()->GetId() );
                m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent );

                wxMouseEvent mouseevent2( wxEVT_LEFT_UP );
                mouseevent2.m_x = m_dragstartx;
                mouseevent2.m_y = m_dragstarty;
                mouseevent2.m_controlDown = true;
                mouseevent2.m_leftDown = true;
                mouseevent2.SetEventObject(this);
                mouseevent2.SetId( GetDrawingPart()->GetDisplayWindow()->GetId() );
                m_stcontroller->GetFirstTool()->ProcessEvent( mouseevent2 );
            }
        }
        else if ( !m_dragStarted && m_mode == mode_edit )
		{
	        m_toolBusy = false;
            a2dCanvasObject* hit = m_modehit;
            a2dPin* pin = wxDynamicCast( m_modehit, a2dPin );

			if ( hit && !pin )
			{
				GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Edit ) );
				PushEditTool( m_modehit );
			}
			/*
            if ( m_dlgOrEdit && ! hit->GetFixedStyle() && !event.ShiftDown() || 
                    !m_dlgOrEdit && ! hit->GetFixedStyle() && event.ShiftDown()
                )
            {
                if ( !m_dlgOrEditModal )
                    GetDrawing()->GetCanvasCommandProcessor()->ShowDlgStyle( true );
                else
                {
				    a2dStyleDialog styleDlg( NULL, wxDEFAULT_DIALOG_STYLE | wxDIALOG_NO_PARENT | wxMINIMIZE_BOX | wxMAXIMIZE_BOX, true );
				    styleDlg.SetFill( hit->GetFill() );
				    styleDlg.SetStroke( hit->GetStroke() );
				    if ( styleDlg.ShowModal() == wxID_OK )
				    {
                        if ( !hit->GetSelected() )
                        {
					        hit->SetFill( styleDlg.GetFill() );
					        hit->SetStroke( styleDlg.GetStroke() );
                        }
                        else
						    GetDrawing()->GetCanvasCommandProcessor()->Submit( new a2dCommand_SetFillStrokeMask( GetDrawingPart()->GetShowObject() ) );
				    }
                }
            }
            else
            {
                a2dWirePolylineL* wire = wxDynamicCast( hit, a2dWirePolylineL );
                m_modehit = hit;

                if ( (wire && wire->GetSelected() ) || (wire && !m_drawWire ) || !wire )
                {
                    hit->SetSelected( m_modehitLastSelectState );
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                    m_mode = mode_edit;
                    PushEditTool( m_modehit );
                }
            }
			*/
		}

        SetToolMode( event.ControlDown(), event.ShiftDown(), event.AltDown() );
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
