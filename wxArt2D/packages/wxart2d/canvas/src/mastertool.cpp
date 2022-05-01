/*! \file editor/src/mastertool.cpp
    \author Michael S�gtrop
    \date Created 02/06/2004

    Copyright: 2004-2004 (c) Michael S�gtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: mastertool.cpp,v 1.109 2009/08/07 20:31:32 titato Exp $
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
#include "wx/canvas/sttool2.h"
#include "wx/canvas/mastertool.h"
#include <wx/general/id.inl>
#include <wx/regex.h>
#include <algorithm>


IMPLEMENT_CLASS( a2dSimpleEditPolygonTool, a2dStTool )

BEGIN_EVENT_TABLE( a2dSimpleEditPolygonTool, a2dStTool )
    EVT_MOUSE_EVENTS( a2dSimpleEditPolygonTool::OnMouseEvent )
    EVT_CHAR( a2dSimpleEditPolygonTool::OnChar )
END_EVENT_TABLE()

a2dSimpleEditPolygonTool::a2dSimpleEditPolygonTool( a2dStToolContr* controller, a2dCanvasObject* hit, int index, int count, Action action )
    :
    a2dStTool( controller )
{
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  );
    m_original = hit;
    m_index = index;
    m_count = count;
    m_action = action;
    m_preserve_RouteWhenDrag = false;

    m_mode = 1;
    GetDrawingPart()->SetMouseEvents( false );
}

a2dSimpleEditPolygonTool::~a2dSimpleEditPolygonTool()
{
    if ( GetDrawingPart() )
        GetDrawingPart()->SetMouseEvents( true );
}

bool a2dSimpleEditPolygonTool::EnterBusyMode()
{
    m_preserve_select = m_original->GetSelected();
    m_preserve_RouteWhenDrag = GetDrawing()->GetHabitat()->GetConnectionGenerator()->GetRouteWhenDrag();
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetRouteWhenDrag( true );

    if( !a2dStTool::EnterBusyMode() )
        return false;

    m_controller->GetDrawingPart()->FindAndSetCorridorPath( m_canvasobject );
    return true;
}

void a2dSimpleEditPolygonTool::FinishBusyMode()
{
    if ( GetDrawingPart() )
    {
        GetDrawingPart()->ClearCorridorPath();
    }

    wxASSERT( GetBusy() );

    // The pin which is edited may reconnect
    if( m_action == action_movevertex )
    {
        /*       
        if( m_index == 0 )
            data.m_allowreconnectbegin = true;
        else if( m_index == m_count - 1 )
            data.m_allowreconnectend = true;
        */
        m_original->SetAlgoSkip( true );
        m_canvasobject->SetAlgoSkip( true );
        double hitDistance = GetHitMargin();

    	a2dPinPtr pin = m_handle->GetPin();
		if ( pin )
		{
            a2dPin* pinother = pin->GetPinClass()->GetConnectionGenerator()->
                        SearchPinForFinishWire( GetDrawingPart()->GetShowObject(), pin, NULL, hitDistance  );

            if ( pinother && ! pin->IsConnectedTo( pinother ) )
            {
                GetDrawing()->GetCanvasCommandProcessor()->Submit( new a2dCommand_ConnectPins( pin, pinother ), true );
            }
		}

        m_original->SetAlgoSkip( false );
        m_canvasobject->SetAlgoSkip( false );
    }

    m_original->SetSelected( m_preserve_select );
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetRouteWhenDrag( m_preserve_RouteWhenDrag );

    a2dStTool::FinishBusyMode();
}

void a2dSimpleEditPolygonTool::AbortBusyMode()
{
    if ( GetDrawingPart() )
        GetDrawingPart()->ClearCorridorPath();

    GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetRouteWhenDrag( m_preserve_RouteWhenDrag );
    a2dStTool::FinishBusyMode();
}

void a2dSimpleEditPolygonTool::DoStopTool( bool abort )
{
    if ( GetDrawingPart() )
        GetDrawingPart()->ClearCorridorPath();

    a2dStTool::DoStopTool( abort );
}

void a2dSimpleEditPolygonTool::OnChar( wxKeyEvent& event )
{
    event.Skip();
}

void a2dSimpleEditPolygonTool::OnMouseEvent( wxMouseEvent& event )
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

    if ( ( event.LeftDown() || event.Dragging() ) && !GetBusy() )
    {
        if ( !EnterBusyMode() )
            return; //not editable object

        //we generate some help handles, which get events sent to them via the event corridor to m_canvasobject.

        // m_original->StartEdit() did add m_canvasobject as an editcopy, so eventually those temporary handles will
        // be deleted when m_canvasobject is released.
        a2dPolylineL* poly = wxDynamicCast( m_canvasobject.Get(), a2dPolylineL );
        a2dPolylineL* original = wxDynamicCast( poly->GetOriginal(), a2dPolylineL );
        a2dVertexList::iterator iter = poly->GetSegments()->begin();
        a2dVertexList::iterator iterorg = original->GetSegments()->begin();

        int index = poly->FindSegmentIndex( a2dPoint2D( xw, yw ), GetHitMargin() );
        if ( index != -1 )
        {
            iter = poly->GetSegmentAtIndex( index );
            iterorg = original->GetSegmentAtIndex( index );

            switch( m_action )
            {
                case action_movevertex:
                    m_handle = new a2dPolyHandleL( poly, iter, iterorg, poly->GetSegments(), original->GetSegments(), wxT( "__index__" ) );
                    poly->Append( m_handle );
                    break;

                case action_movesegment:
                    m_handle = new a2dPolyHandleL( poly, iter, iterorg, poly->GetSegments(), original->GetSegments(), xw, yw, wxT( "__segment__" ) );
                    poly->Append( m_handle );
                    break;

                case action_insertvertex:
                    if ( event.AltDown() )
                        m_handle = new a2dPolyHandleL( poly, iter, iterorg, poly->GetSegments(), original->GetSegments(), xw, yw,  wxT( "__segment__" ) );
                    else
                        m_handle = new a2dPolyHandleL( poly, iter, iterorg, poly->GetSegments(), original->GetSegments(), xw, yw,  wxT( "__insert__" ) );
                    poly->Append( m_handle );
                    break;

                default:
                    wxASSERT( 0 );
            }
            m_handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
            m_handle->SetLayer( poly->GetLayer() );
            m_handle->SetPreRenderAsChild( false );
            poly->SetVisiblechilds( true );
            poly->SetChildrenOnSameLayer( true );
            poly->Update( a2dCanvasObject::updatemask_force );
            poly->SetPending( true );


			a2dPolygonL* polyg = wxDynamicCast( m_canvasobject.Get(), a2dPolygonL );
			polyg->SetHandleToIndex( m_handle, m_index );

			bool isHit;
			GetDrawingPart()->ClearCorridorPath();
			GetDrawingPart()->ProcessCanvasObjectEvent( event, isHit, xw, yw, 5 );

			GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( false );

        }
    }

    /*
            pin.FindConnectablePin( tool->GetDrawingPart()->GetShowObject(), 1.0, false );
        if( event.Dragging() && m_action == action_movevertex )
        {
            a2dWirePolyline *wire =
            a2dPin *FindEdnPin();
            a2dPin tmp(
                m_canvasobject, wxT("end"),
                m_thePinClassMap ? *m_thePinClassMap->m_wireEnd : a2dPinClass::Standard,
                m_xwprev, m_ywprev
            );
            a2dIterC ic( GetDrawingPart() );
            GetDrawingPart()->GetShowObject()->EditFeedback( ic, &a2dPin::sm_feedbackGeneratePin, &tmp, 2, 0, m_xw, m_yw );
        }
    */

    else if( ( event.LeftDown() || event.LeftUp() || event.Dragging() ) && GetBusy() && m_handle )
    {
        a2dPolygonL* poly = wxDynamicCast( m_canvasobject.Get(), a2dPolygonL );
        poly->SetHandleToIndex( m_handle, m_index );

        bool isHit;
        if ( event.LeftDown() )
            GetDrawingPart()->ClearCorridorPath();
        GetDrawingPart()->ProcessCanvasObjectEvent( event, isHit, xw, yw, 5 );

        GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( false );
    }

    if( event.LeftUp() && GetBusy() )
    {
        FinishBusyMode();
    }
}

bool a2dSimpleEditPolygonTool::CreateToolObjects()
{
	a2dRefMap refs;

    // Clone the original object
    m_canvasobject = m_original->StartEdit( this, m_mode, wxEDITSTYLE_COPY | wxEDITSTYLE_NOHANDLES, &refs );
    if ( !m_canvasobject )
        return false; //not editable object

    //drag is comming so first create wires where there are non.
    a2dCanvasObjectList dragList;
    dragList.push_back( m_original );

    PrepareForRewire( dragList, true, false, false, true, &refs );

    // only now we call this, earlier not possible, because clones of connectedwires not setup yet.
	refs.LinkReferences();

    // Set the visibility of the original dragged object and the original connected wires
    AdjustRenderOptions();
    m_original->SetVisible( m_renderOriginal );

    m_pending = true;

    return true;
}

void a2dSimpleEditPolygonTool::CleanupToolObjects()
{
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );
    a2dStTool::CleanupToolObjects();
}

//----------------------------------------------------------------------------
// a2dMasterDrawBase
//----------------------------------------------------------------------------

IMPLEMENT_CLASS( a2dMasterDrawBase, a2dStTool )

BEGIN_EVENT_TABLE( a2dMasterDrawBase, a2dStTool )
    EVT_MOUSE_EVENTS( a2dMasterDrawBase::OnMouseEvent )
    EVT_CHAR( a2dMasterDrawBase::OnChar )
    EVT_KEY_DOWN( a2dMasterDrawBase::OnKeyDown )
    EVT_KEY_UP( a2dMasterDrawBase::OnKeyUp )
END_EVENT_TABLE()

a2dMasterDrawBase::a2dMasterDrawBase( a2dStToolContr* controller ):
    m_dlgOrEdit( false ),
    m_dlgOrEditModal( false ),
    m_escapeToStopFirst( true ),
    m_allowWireDraw( true ),
    a2dStTool( controller )
{
    m_generatedPinX = 0;
    m_generatedPinY = 0;
	m_styleDlgSimple = false;
    m_hadDoubleClick = false;
    m_dragStarted = false;
    m_toolBusy = false;
    m_select_undo = false;
	m_lateconnect = true;
	m_wiringMode = a2d_StartGenerateSearchFinish;

    m_spaceDown = false;
    m_vertexSegmentEdit = false;
    m_movePin = false;
    m_allowMultiEdit = true;
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_Select  );
    m_mode = mode_none;
    m_canvasobject = 0;
    m_modehit = 0;
    m_endSegmentMode = a2dCanvasGlobals->GetHabitat()->GetEndSegmentMode();

    controller->SetDefaultBehavior( controller->GetDefaultBehavior() | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );

    m_eventHandler = new a2dStToolFixedToolStyleEvtHandler( controller );

    m_selectStroke = a2dCanvasGlobals->GetHabitat()->GetSelectStroke();
    m_selectFill = a2dCanvasGlobals->GetHabitat()->GetSelectFill();
    if ( GetDrawingPart()->GetDrawer2D()->HasAlpha() )
    {
        SetFill( a2dFill( wxColour( 66, 159, 235, 150 ) ) );
        SetStroke( a2dStroke( wxColour( 66, 159, 235, 165 ), 1, a2dSTROKE_LONG_DASH ) );
        //m_selectFill = a2dFill( wxColour( 233, 15, 23, 50 ) );
        //m_selectStroke = a2dStroke( wxColour( 255, 59, 25, 255 ), 1, a2dSTROKE_LONG_DASH );
    }
    else
    {
        SetFill( *a2dTRANSPARENT_FILL );
        SetStroke( a2dStroke( *wxBLACK, 2, a2dSTROKE_LONG_DASH ) );
        m_selectFill = *a2dTRANSPARENT_FILL;
        //m_selectStroke = a2dStroke( *wxRED, 2, a2dSTROKE_LONG_DASH );
    }
}

a2dMasterDrawBase::~a2dMasterDrawBase()
{
}

void a2dMasterDrawBase::SetLastSelected( a2dCanvasObject* lastSelect, bool onOff )
{
    a2dCanvasObjectList* objects = GetDrawingPart()->GetShowObject()->GetChildObjectList();
    objects->SetSpecificFlags( true, a2dCanvasOFlags::PENDING, "", a2dCanvasOFlags::SELECTED2 );
 	objects->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED2 );
    if ( lastSelect )
    {
		lastSelect->SetSelected2( onOff );
    }
    GetDrawingPart()->GetShowObject()->GetRoot()->SetUpdatesPending( true );
}

void a2dMasterDrawBase::InitMouseEvent(wxMouseEvent& eventnew,
                                 int x, int y,
                                 wxMouseEvent& event )
{
    eventnew.m_x = x;
    eventnew.m_y = y;

    eventnew.m_shiftDown = event.m_shiftDown;
    eventnew.m_controlDown = event.m_controlDown;
    eventnew.m_leftDown = event.m_leftDown;
    eventnew.m_middleDown = event.m_middleDown;
    eventnew.m_rightDown = event.m_rightDown;
    eventnew.m_altDown = event.m_altDown;
    eventnew.SetEventObject(this);
    eventnew.SetId( event.GetId());

}

bool a2dMasterDrawBase::RotateObject90LeftRight( bool right )
{
    a2dCanvasObject* selected =  m_parentobject->GetChildObjectList()->Find( "", "", a2dCanvasOFlags::SELECTED );
    if ( selected )
    {
        a2dAffineMatrix mat = selected->GetTransform();
        if ( right )
            GetCanvasCommandProcessor()->Submit( new a2dCommand_RotateMask( GetDrawingPart()->GetShowObject(), 90, true ) );
        else 
            GetCanvasCommandProcessor()->Submit( new a2dCommand_RotateMask( GetDrawingPart()->GetShowObject(), -90, true ) );
        return true;
    }

    return false;
}

void a2dMasterDrawBase::MouseDump( wxMouseEvent& event, wxString strinfo )
{
    int x = event.GetX();
    int y = event.GetY();
    double xwprev, ywprev;
    GetDrawingPart()->MouseToToolWorld( x, y, xwprev, ywprev );

    if ( event.LeftDown() )
        strinfo << "LeftDown";
    if ( event.LeftUp() )
        strinfo << "LeftUp";
    if ( event.LeftDClick() )
        strinfo << "LeftDoubleClick";
    if ( event.RightDClick() )
        strinfo << "RightDoubleClick";
    if ( event.Dragging() )
        strinfo << "Dragging";
    if ( event.Moving() )
        strinfo << "Moving";

    if ( event.ShiftDown() )
        strinfo << " s ";
    if ( event.ControlDown() )
        strinfo << " c ";
    if ( event.AltDown() )
        strinfo << " a ";

    wxLogDebug( strinfo );
  
}

void a2dMasterDrawBase::PushZoomTool()
{
    a2dSmrtPtr< a2dStTool > tool;
    tool = new a2dZoomTool( GetStToolContr() );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetFill( m_fill );
    tool->SetStroke( m_stroke );
    m_controller->PushTool( tool );
}

void a2dMasterDrawBase::SetEndSegmentMode( a2dNextSeg mode )
{
	m_endSegmentMode = mode;
}

void a2dMasterDrawBase::PushDrawWireTool( a2dCanvasObject* WXUNUSED( hit ) )
{
    a2dSmrtPtr< a2dDrawWirePolylineLTool > tool = new a2dDrawWirePolylineLTool( GetStToolContr() );
    tool->SetEndSegmentMode( m_endSegmentMode );
	tool->SetWiringMode( m_wiringMode );
    tool->SetOneShot();
    m_controller->PushTool( tool );
}

void a2dMasterDrawBase::PushDragTool( a2dCanvasObject* hit )
{
    SelectHitObject( hit );
    a2dSmrtPtr< a2dDragTool > tool;
    tool = new a2dDragTool( GetStToolContr(), NULL, m_xwprev, m_ywprev );

    // object without pins normal snap behaviour is in place, else only snap pins to target features.
    if ( hit->HasPins() )
        tool->SetSnapSourceFeatures( a2dRestrictionEngine::snapToPins );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
	tool->SetLateConnect( m_lateconnect );
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
    tool->SetDeleteOnOutsideDrop( true );
    m_controller->PushTool( tool );

    tool->StartDragging( m_x, m_y, hit );
}

void a2dMasterDrawBase::PushDragMultiTool( a2dCanvasObject* hit, bool onlyKeys )
{
    a2dSmrtPtr< a2dStTool > tool;
    tool = new a2dDragMultiTool( GetStToolContr() );

    // object without pins normal snap behaviour is in place, else only snap pins to target features.
    if ( hit->HasPins() )
        tool->SetSnapSourceFeatures( a2dRestrictionEngine::snapToPins );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
    m_controller->PushTool( tool );
    a2dDragMultiTool* mdt = wxDynamicCast( tool.Get(), a2dDragMultiTool );
    if ( mdt )
	{
        mdt->SetOnlyKeys( onlyKeys );
		mdt->SetLateConnect( m_lateconnect );
        mdt->StartDragging( m_x, m_y, hit );
	}
}

void a2dMasterDrawBase::PushCopyTool( a2dCanvasObject* hit )
{
    SelectHitObject( hit );
    a2dSmrtPtr< a2dCopyTool > tool;
    tool = new a2dCopyTool( GetStToolContr() );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
    tool->SetDeleteOnOutsideDrop( true );
    m_controller->PushTool( tool );
}

void a2dMasterDrawBase::PushCopyMultiTool( a2dCanvasObject* hit )
{
    a2dSmrtPtr< a2dStTool > tool;
    tool = new a2dCopyMultiTool( GetStToolContr() );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Copy ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Copy ) );
    m_controller->PushTool( tool );
    a2dCopyMultiTool* mdt = wxDynamicCast( tool.Get(), a2dCopyMultiTool );
    if ( mdt )
        mdt->StartDragging( m_x, m_y, hit );
}

void a2dMasterDrawBase::PushSelectTool()
{
    a2dSmrtPtr< a2dStTool > tool;
    tool = new a2dSelectTool( GetStToolContr() );
    ( ( a2dSelectTool* )tool.Get() )->SetShiftIsAdd();

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetFill( m_selectFill );
    tool->SetStroke( m_selectStroke );

#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
    tool->SetUseOpaqueEditcopy( a2dOpaqueMode_Tool );
#endif
    m_controller->PushTool( tool );
}

void a2dMasterDrawBase::PushEditWireVertexTool( a2dCanvasObject* hit, int vertex )
{
    a2dSmrtPtr< a2dPolygonL > poly = wxStaticCast( hit, a2dPolygonL );
    SelectHitObject( hit );
    a2dSimpleEditPolygonTool* tool = new a2dSimpleEditPolygonTool( GetStToolContr(), hit, vertex, poly->GetSegments()->size(), a2dSimpleEditPolygonTool::action_movevertex );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditVertex ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditVertex ) );
    m_controller->PushTool( tool );
}

void a2dMasterDrawBase::PushEditSegmentTool( a2dCanvasObject* hit, int segment )
{
    a2dSmrtPtr< a2dPolygonL > poly = wxStaticCast( hit, a2dPolygonL );
    SelectHitObject( hit );
    a2dSimpleEditPolygonTool* tool = new a2dSimpleEditPolygonTool( GetStToolContr(), hit, segment, poly->GetSegments()->size(), a2dSimpleEditPolygonTool::action_movesegment );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegment ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegment ) );
    m_controller->PushTool( tool );
}

void a2dMasterDrawBase::PushEditWireSegmentHorizontalTool( a2dCanvasObject* hit, int segment )
{
    a2dSmrtPtr<a2dPolygonL> poly = wxStaticCast( hit, a2dPolygonL );
    SelectHitObject( hit );
    a2dSmrtPtr<a2dSimpleEditPolygonTool> tool = new a2dSimpleEditPolygonTool( GetStToolContr(), hit, segment, poly->GetSegments()->size(), a2dSimpleEditPolygonTool::action_movesegment );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentHorizontal ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentHorizontal ) );
    m_controller->PushTool( tool );
}

void a2dMasterDrawBase::PushEditWireSegmentVerticalTool( a2dCanvasObject* hit, int segment )
{
    a2dSmrtPtr<a2dPolygonL> poly = wxStaticCast( hit, a2dPolygonL );
    SelectHitObject( hit );
    a2dSmrtPtr<a2dSimpleEditPolygonTool> tool = new a2dSimpleEditPolygonTool( GetStToolContr(), hit, segment, poly->GetSegments()->size(), a2dSimpleEditPolygonTool::action_movesegment );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentVertical ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentVertical ) );
    m_controller->PushTool( tool );
}

void a2dMasterDrawBase::PushEditWireSegmentInsertTool( a2dCanvasObject* hit, int segment )
{
    a2dSmrtPtr<a2dPolygonL> poly = wxStaticCast( hit, a2dPolygonL );
    SelectHitObject( hit );
    a2dSmrtPtr<a2dSimpleEditPolygonTool> tool = new a2dSimpleEditPolygonTool( GetStToolContr(), hit, segment, poly->GetSegments()->size(), a2dSimpleEditPolygonTool::action_insertvertex );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_WireSegmentInsert ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_WireSegmentInsert ) );
    m_controller->PushTool( tool );
}

void a2dMasterDrawBase::PushEditTool( a2dCanvasObject* hit )
{
    SelectHitObject( hit );

    a2dSmrtPtr< a2dObjectEditTool > tool;
    tool = new a2dObjectEditTool( m_stcontroller );

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

void a2dMasterDrawBase::PushDlgEditTool( a2dCanvasObject* hit )
{
    SelectHitObject( hit );

    a2dSmrtPtr< a2dObjectEditTool > tool;
    tool = new a2dObjectEditTool( m_stcontroller );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    if ( hit )
    {

    }
    //tool->SetEvtHandler( m_eventHandler  );
    
    m_stcontroller->PushTool( tool );
    tool->StartToEdit( hit );
}


void a2dMasterDrawBase::PushMultiEditTool( a2dCanvasObject* hit )
{
    a2dSmrtPtr< a2dMultiEditTool > tool = new a2dMultiEditTool( m_stcontroller );

    tool->SetShowAnotation( m_anotate );
    tool->StartEditingSelected();
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditSegment ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditSegment ) );
    m_stcontroller->PushTool( tool );
}

void a2dMasterDrawBase::PushMovePinTool( a2dCanvasObject* hit )
{
    a2dPin* hitpin = wxStaticCast( hit, a2dPin );
    a2dSmrtPtr< a2dMovePinTool > tool = new a2dMovePinTool( m_stcontroller, hitpin, m_xwprev, m_ywprev );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
    m_controller->PushTool( tool );
}

void a2dMasterDrawBase::PushRewirePinTool( a2dCanvasObject* hit )
{
    a2dPin* hitpin = wxStaticCast( hit, a2dPin );
    a2dPin* wirePin = NULL;
    if ( hitpin->GetParent()->IsConnect() )
        wirePin = hitpin;
    else
    {
        wirePin = hitpin->FindWirePin( a2dCanvasOFlags::SELECTED );
        if ( !wirePin )
        wirePin = hitpin->FindWirePin();
    }

    a2dSmrtPtr< a2dMovePinTool > tool = new a2dMovePinTool( m_stcontroller, wirePin, m_xwprev, m_ywprev, true );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
    m_controller->PushTool( tool );
}

void a2dMasterDrawBase::EditDlgOrHandles( a2dCanvasObject* hit, bool modifier, bool noHandleEditForWire )
{
	if ( hit )
	{
		if ( m_dlgOrEdit && ! hit->GetFixedStyle() && !modifier )
		{
            /*
        	ClassXXX* xxx = wxDynamicCast( hit, ClassXXX );
            if ( xxx )
		    {
			    hit->SetSelected( m_modehitLastSelectState );
			    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
			    m_mode = mode_edit;
			    PushEditTool( hit );
		    }
            */
        	a2dPort* port = wxDynamicCast( hit, a2dPort );
        	a2dCameleonInst* caminst = wxDynamicCast( hit, a2dCameleonInst );
            if ( caminst || port )
		    {
			    hit->SetSelected( m_modehitLastSelectState );
			    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
			    m_mode = mode_edit;
			    PushEditTool( hit );
		    }
            else if ( !m_dlgOrEditModal )
				GetDrawing()->GetCanvasCommandProcessor()->ShowDlgStyle( true );
			else if ( m_styleDlgSimple )
			{
				bool hitSel = hit->GetSelected();
				hit->SetSelected( true );

                a2dExtFill eFi;
                a2dExtStroke eSt;
				a2dTextChanges eFo;
                wxUint32 nrst = 0;
                wxUint32 nrfi = 0;
				wxUint32 nrfo = 0;
				bool showStyleDlg = false;
				bool showFontDlg = false;
				bool withFill = false;

				a2dCanvasObjectList* objects = GetDrawingPart()->GetShowObject()->GetChildObjectList();
				a2dCanvasObjectList textobjects, primitiveobjects;

				//filter objects
				forEachIn( a2dCanvasObjectList, objects )
				{
					a2dCanvasObject* obj = *iter;
    				a2dText* text = wxDynamicCast( obj, a2dText );

					if ( text )
						textobjects.push_back( obj );
					else
						primitiveobjects.push_back( obj );
				}

				if( wxDynamicCast( hit, a2dText ) )		// Text
				{
					showFontDlg = true;
					objects = &textobjects;
				}
				else		// Primitive
				{
					showStyleDlg = true;
					objects = &primitiveobjects;
				}

				//optional can be resolved with SetFixedStyle() on certain ta wires (TaPin.cpp)
				//we do not want Tawire for Taco's to change in style, other ta wires are okay.
				// a2dWirePolylineL* wire = wxDynamicCast( hit, a2dWirePolylineL );
				//if ( wire && ( wire->GetStartPinClass() == a2dPin::XXXPinClass ) )
				//	showStyleDlg = false;

                nrst = a2dSetExtStroke( eSt, objects, a2dCanvasOFlags::SELECTED, GetDrawing()->GetLayerSetup() );
                nrfi = a2dSetExtFill( eFi, objects, a2dCanvasOFlags::SELECTED, GetDrawing()->GetLayerSetup() );
				nrfo = a2dSetTextChanges( eFo, objects, a2dCanvasOFlags::SELECTED, GetDrawing()->GetLayerSetup() );

                if ( nrst + nrfi == 0 )
                {
                    //eFi.Set( hit->GetFill() );  
                    //eSt.Set( hit->GetStroke() ); 
					//withFill = true; // we want to be able to choose a fill
					showStyleDlg = false;
                }
				else
					withFill = 0 != nrfi;

				if( showFontDlg )
				{
                    GetDrawingPart()->PushCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_WAIT ) );
					a2dTextPropDlgExt dlg( GetDrawing()->GetHabitat(), NULL, false, withFill, eFo.GetFontInfoList(), false );
					dlg.SetUnitsScale( GetDrawing()->GetUnitsScale() );
					dlg.SetExtFill(eFi);
					dlg.SetExtStroke(eSt);
					dlg.SetExtFont(eFo);
					//dlg.SetCustomColors( XXX.GetCustomColors());
					if ( wxID_OK == dlg.ShowModal() )
					{
						eSt = dlg.GetExtStroke(); // get edited ExtStroke from dialog
						eFi = dlg.GetExtFill(); // get edited ExtFill from dialog
						eFo = dlg.GetExtFont(); // get edited ExtFont from dialog

						if ( !hit->GetSelected() )
						{
							hit->SetFill( eFi.Get( hit->GetFill() ) );
							hit->SetStroke( eSt.Get( hit->GetStroke() ) );
							//hit->SetFont( eFo.Get( hit->GetFont() ) );	//todo
						}
						else
						{
							GetDrawing()->GetCanvasCommandProcessor()->Submit( new a2dCommand_SetFillStrokeExtMask( GetDrawingPart()->GetShowObject(), eSt, eFi, a2dCanvasOFlags::BIN2 ) );
							GetDrawing()->GetCanvasCommandProcessor()->Submit( new a2dCommand_SetTextChangesMask( GetDrawingPart()->GetShowObject(), eFo, eFo.GetAlignment(), a2dCanvasOFlags::BIN2 ) );
						}
					}
					// XXX.SetCustomColors(dlg.GetCustomColors());
                    GetDrawingPart()->PopCursor();
				}
				else if ( showStyleDlg )
				{
					a2dDialogStyle dlg( NULL, false, withFill, false );
					dlg.SetUnitsScale( GetDrawing()->GetUnitsScale() );
					dlg.SetExtFill(eFi);
					dlg.SetExtStroke(eSt);
					//dlg.SetCustomColors( XXX.GetCustomColors());

					if ( wxID_OK == dlg.ShowModal() )
					{
						eSt = dlg.GetExtStroke(); // get edited ExtStroke from dialog
						eFi = dlg.GetExtFill(); // get edited ExtFill from dialog

						if ( !hit->GetSelected() )
						{
							hit->SetFill( eFi.Get( hit->GetFill() ) );
							hit->SetStroke( eSt.Get( hit->GetStroke() ) );
						}
						else
							GetDrawing()->GetCanvasCommandProcessor()->Submit( new a2dCommand_SetFillStrokeExtMask( GetDrawingPart()->GetShowObject(), eSt, eFi, a2dCanvasOFlags::BIN2 ) );
					}
					//XXX.SetCustomColors(dlg.GetCustomColors());
				}
				hit->SetSelected( hitSel );
                objects->SetSpecificFlags( false, a2dCanvasOFlags::BIN2 );
			}
			else
			{
				a2dStyleDialog styleDlg( GetDrawing()->GetHabitat(), NULL, wxDEFAULT_DIALOG_STYLE | wxDIALOG_NO_PARENT | wxMINIMIZE_BOX | wxMAXIMIZE_BOX, true );
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
			if ( (wire && wire->GetSelected() ) || (wire && !noHandleEditForWire ) || !wire )
			{
				hit->SetSelected( m_modehitLastSelectState );
				GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
				m_mode = mode_edit;
				PushEditTool( hit );
			}
		}
	}
}

bool a2dMasterDrawBase::ZoomSave()
{
    return true;
}

void a2dMasterDrawBase::DoStopTool( bool abort )
{
    a2dStTool::DoStopTool( abort );
}

void a2dMasterDrawBase::OnChar( wxKeyEvent& event )
{
    switch( event.GetKeyCode() )
    {
        case WXK_DELETE:
        {
            wxASSERT( m_parentobject == GetDrawingPart()->GetShowObject() );

            GetCanvasCommandProcessor()->Submit( new a2dCommand_DeleteMask( m_parentobject ), true );

            GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( GetDrawingPart()->GetShowObject() );

            break;
        }
        case WXK_SPACE:
        {
            m_stcontroller->Zoomout();
            break;
        }
        default:
            event.Skip();
    }


    a2dCanvasObject* selected =  m_parentobject->GetChildObjectList()->Find( "", "", a2dCanvasOFlags::SELECTED );

    if ( selected )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_ESCAPE:
            {
                DeselectAll();
                break;
            }
            default:
                event.Skip();
            }
    }
    else
        event.Skip();

}


void a2dMasterDrawBase::OnKeyDown( wxKeyEvent& event )
{
    //wxLogDebug(wxT("key %d"), event.GetKeyCode());
    a2dBaseToolPtr first = m_stcontroller->GetFirstTool();

    switch( event.GetKeyCode() )
    {
        case WXK_ESCAPE:
        {
            if ( m_escapeToStopFirst && first && first->AllowPop() )
            {
                first->StopTool( true );
                return;
            }
        }
        break;
        default:;
    }

    if ( !m_active )
    {
        event.Skip();
        return;
    }

    //wxLogDebug(wxT("key %d"), event.GetKeyCode());

    switch( event.GetKeyCode() )
    {
        case WXK_CONTROL:
        {
            if  (  !first || ( first && !first->GetBusy() ) )
            {
                if ( !wxDynamicCast( first.Get(), a2dDragTool ) )
                {
                    //a2dDragTool* drag = new a2dDragTool(m_stcontroller);
                    //m_stcontroller->PushTool(drag);
                    //drag->SetOneShot();
                }
            }
            event.Skip();
        }
        break;
        case 'p':
        case 'P':
        {
            if ( !event.HasAnyModifiers() )
                m_vertexSegmentEdit = true;
            else
                event.Skip();
            break;
        }
        case 'o':
        case 'O':
        {
            if ( !event.HasAnyModifiers() )
                m_movePin = true;
            else
                event.Skip();
            break;
        }
        case 'Z':
        case 'z':
        {
            if ( !event.HasAnyModifiers() && ( !first || ( first && !first->GetBusy() ) ))
                PushZoomTool();
            else
                event.Skip();

            break;
        }
        case 'r':
        case 'R':
        {
            if ( !event.HasAnyModifiers() )
                GetDrawing()->GetHabitat()->GetConnectionGenerator()->RotateRouteMethod();
            else
                event.Skip();
            break;
        }   
        case 't':
        case 'T':
        {
            if ( !event.HasAnyModifiers() )
                GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetRouteWhenDrag( !GetDrawing()->GetHabitat()->GetConnectionGenerator()->GetRouteWhenDrag() );
            else
                event.Skip();
            break;
        }   
        case WXK_NUMPAD_ENTER:
        {
            if ( !event.HasAnyModifiers() )
                m_stcontroller->Zoomout();
            else
                event.Skip();
            break;
        }
        case WXK_NUMPAD_ADD:
        {
            if ( !event.HasAnyModifiers() )
            {
                if ( event.m_controlDown )
                    m_stcontroller->ZoomIn2AtMouse();
                else
                    m_stcontroller->Zoomin2();
            }
            else
                event.Skip();
            break;
        }
        case WXK_NUMPAD_SUBTRACT:
        {
            if ( !event.HasAnyModifiers() )
            {
                if ( event.m_controlDown )
                    m_stcontroller->ZoomOut2AtMouse();
                else
                    m_stcontroller->Zoomout2();
            }
            else
                event.Skip();
            break;
        }
        case WXK_SPACE:
        {
            m_spaceDown = true;
            m_mode = mode_zoom;
            m_modehit = NULL;
            event.Skip();
            break;
        }
        case WXK_UP:
        case WXK_DOWN:
        case WXK_LEFT:
        case WXK_RIGHT:
        {
            a2dCanvasObject* selected =  m_parentobject->GetChildObjectList()->Find( "", "", a2dCanvasOFlags::SELECTED );
            if ( selected )
            {
				int oldthres = 0;
			    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
				if( restrict )
				{
					oldthres = restrict->GetSnapThresHold();
					restrict->SetSnapThresHold( 0 );
				}
                PushDragMultiTool( selected, true );
                m_stcontroller->GetFirstTool()->ProcessEvent( event );
				if( restrict )
					restrict->SetSnapThresHold( oldthres );
            }
            else
                event.Skip();
            break;
        }
        default:
            event.Skip();
    }
}

void a2dMasterDrawBase::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    switch( event.GetKeyCode() )
    {
        case 'p':
        case 'P':
        {
            m_vertexSegmentEdit = false;
            break;
        }
        case 'o':
        case 'O':
        {
            m_movePin = false;
            break;
        }
        default:
            event.Skip();
    }
}


void a2dMasterDrawBase::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }
/*
    m_x = event.GetX();
    m_y = event.GetY();

    GetDrawingPart()->MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );
*/
    if( event.LeftDown() )//&& !GetBusy() )
    {
        // decide tool depending on mode
        switch( m_mode )
        {
            case mode_none:
                event.Skip();
                break;
            case mode_zoom:
                PushZoomTool();
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
            case mode_copy:
                PushCopyTool( m_modehit );
                event.Skip();
                break;
            case mode_dragmulti:
                PushDragMultiTool( m_modehit );
                event.Skip();
                break;
            case mode_copymulti:
                PushCopyMultiTool( m_modehit );
                event.Skip();
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
                m_mode = mode_none;
                event.Skip();
        }
    }
    else
    {
        event.Skip();
    }

}

void a2dMasterDrawBase::SelectHitObject( a2dCanvasObject* hit )
{
    if ( m_select_undo )
    {
		OpenCommandGroupNamed( _( "Select" ) );
		DeselectAll();
		GetCanvasCommandProcessor()->Submit(
			new a2dCommand_SetFlag( hit, a2dCanvasOFlags::SELECTED, true ) );
		CloseCommandGroup();
	}
    else
    {
        DeselectAll(); //delselect all needs last (selected2) to set also.
        hit->SetSelected( true );
        hit->SetSelected2( true );
    }
}

a2dCanvasObject* a2dMasterDrawBase::GetTopLeftSelected()
{
    double xmin, ymax;
    xmin = DBL_MAX;
    ymax = DBL_MIN;
    a2dCanvasObject* topLeft = 0;

    a2dBoundingBox bbox;
    a2dCanvasObjectList* objects = m_parentobject->GetChildObjectList();
    forEachIn( a2dCanvasObjectList, objects )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj->GetRelease() || !obj->IsVisible() || !obj->GetSelected() )
            continue;

        if ( !topLeft ) 
        {
            topLeft = obj;
            xmin = obj->GetBboxMinX();
            ymax = obj->GetBboxMaxY();
        }
        else
        {
            if ( obj->GetBboxMinX() < xmin )
            {
                topLeft = obj;
                xmin = obj->GetBboxMinX();
				ymax = obj->GetBboxMaxY();
            }
            else if ( obj->GetBboxMinX() == xmin )
            {
                if ( obj->GetBboxMaxY() < ymax )
                {
                    topLeft = obj;
                    ymax = obj->GetBboxMaxY();
                }
            }
        }
	}
    return topLeft;
}

void a2dMasterDrawBase::SelectedStatus()
{
    a2dBoundingBox bbox = m_parentobject->GetChildObjectList()->GetBBox( a2dCanvasOFlags::VISIBLE | a2dCanvasOFlags::SELECTED );

	if ( bbox.GetValid() )
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
}

//----------------------------------------------------------------------------
// a2dMasterDrawSelectFirst
//----------------------------------------------------------------------------

IMPLEMENT_CLASS( a2dMasterDrawSelectFirst, a2dMasterDrawBase )

BEGIN_EVENT_TABLE( a2dMasterDrawSelectFirst, a2dMasterDrawBase )
    EVT_MOUSE_EVENTS( a2dMasterDrawSelectFirst::OnMouseEvent )
    EVT_CHAR( a2dMasterDrawSelectFirst::OnChar )
    EVT_KEY_DOWN( a2dMasterDrawSelectFirst::OnKeyDown )
    EVT_KEY_UP( a2dMasterDrawSelectFirst::OnKeyUp )
END_EVENT_TABLE()

a2dMasterDrawSelectFirst::a2dMasterDrawSelectFirst( a2dStToolContr* controller ): a2dMasterDrawBase( controller )
{
    controller->SetDefaultBehavior( controller->GetDefaultBehavior() | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
}

a2dMasterDrawSelectFirst::~a2dMasterDrawSelectFirst()
{
}

void a2dMasterDrawSelectFirst::OnChar( wxKeyEvent& event )
{
    switch( event.GetKeyCode() )
    {
        case WXK_SPACE:
        {
            break;
        }
        default:
            event.Skip();
    }
}

void a2dMasterDrawSelectFirst::OnKeyDown( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    //wxLogDebug(wxT("key %d"), event.GetKeyCode());
    a2dBaseTool* first = m_stcontroller->GetFirstTool();

    switch( event.GetKeyCode() )
    {
        case WXK_SPACE:
        {
            m_spaceDown = true;
            m_mode = mode_zoom;
            m_modehit = NULL;
            break;
        }
        case WXK_CONTROL:
        {
            m_mode = mode_copy;
            m_modehit = NULL;
            break;
        }
        case WXK_SHIFT:
        {
            //DecideMode( m_modehit, true, false );
            break;
        }
        default:
            event.Skip();
    }
}

void a2dMasterDrawSelectFirst::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    switch( event.GetKeyCode() )
    {
        case WXK_SPACE:
        {
            m_spaceDown = false;
            m_mode = mode_zoom;
            m_modehit = NULL;
            break;
        }
        case WXK_SHIFT:
        {
            //DecideMode( m_modehit, false, false );
            break;
        }
        default:
            event.Skip();
    }
}


void a2dMasterDrawSelectFirst::DecideMode( a2dCanvasObject* hit, const a2dHitEvent& hitinfo, bool shift, bool control )
{
    // show cursor different depending on the object hit, and set mode for tool to push in case of leftdown.
    m_mode = mode_select;
    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select  ) );
    m_modehit = 0;
    m_modehitinfo = a2dHit();

    if ( m_spaceDown )
    {
        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Zoom  ) );
        m_mode = mode_zoom;
    }
    else if( hit )
    {
        m_mode = mode_none;
        m_modehit = hit;
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( GetDrawingPart()->GetShowObject() );
        // Check if there is an unconnected pin
        // This has highest priority
        a2dPin* pin = NULL;
        a2dWirePolylineL* wire = 0;
        a2dHit how2;

        if ( m_vertexSegmentEdit )
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
                        assert( 0 );
                        m_modehit = 0;
                        m_modehitinfo = a2dHit();
                }
            }        
        }

        // if not editing a wire, search for pins
        if ( m_mode == mode_none )
        {
            if ( !shift && !control )
            {
                //there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
                // m_connectionGenerator.
                GetDrawing()->GetHabitat()->GetConnectionGenerator()->
        	        GeneratePinsToConnect( GetDrawingPart(), GetDrawingPart()->GetShowObject(), a2dPinClass::Any, a2d_GeneratePinsForStartWire, m_xwprev, m_ywprev );

                pin = GetDrawing()->GetHabitat()->GetConnectionGenerator()->
			                SearchPinForStartWire( GetDrawingPart()->GetShowObject(), m_xwprev, m_ywprev, a2dPinClass::Any, GetHitMargin() );
            }

            if( pin )
            {
                if ( m_movePin )
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
                if ( !shift && hit->GetDraggable() && hit->GetSelected() )
                {
                    if ( control )
                    {
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Copy ) );
                        // copy selected mode
                        m_mode = mode_copymulti;
                    }
                    else
                    {
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                        m_mode = mode_dragmulti;
                    }
                }
                else if ( shift && hit->GetDraggable() )
                {
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                    m_mode = mode_select;
                }
                else
                    m_mode = mode_select;

            }

        }                
    }
    else
        GetDrawingPart()->SetCursor( m_toolcursor );
}

void a2dMasterDrawSelectFirst::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }
    m_x = event.GetX();
    m_y = event.GetY();

    GetDrawingPart()->MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    GetDrawingPart()->Update( a2dCANVIEW_UPDATE_PENDING );
    a2dIterC ic( GetDrawingPart() );
    ic.SetLayer( wxLAYER_ALL );
    ic.SetObjectFilter( new a2dCanvasObjectFilterLayerMask( wxLAYER_ALL, a2dCanvasOFlags::VISIBLE | a2dCanvasOFlags::SELECTABLE ) );
    a2dHitEvent hitinfo( m_xwprev, m_ywprev, false, a2dCANOBJHITOPTION_NONE, true );
    hitinfo.m_option = a2dCANOBJHITOPTION_LAYERS;
    if ( event.ShiftDown() && event.ControlDown() )
        hitinfo.m_option |= a2dCANOBJHITOPTION_NOTSELECTED;
    //at the top level the group its matrix is to be ignored.
    //Since it is normally NOT ignored within a2dCanvasObject, force an inverse.
    hitinfo.m_xyRelToChildren = true;
    a2dCanvasObject* top = GetDrawingPart()->GetShowObject();
    a2dCanvasObject* hit = top->IsHitWorld( ic, hitinfo );

    // if left double click and hit on a selected object, go and edit it
    if ( hit && event.LeftDClick() && !GetBusy() )
    {
        int i = 0;
        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  ) );
        m_modehit = hit;
        a2dCanvasObjectList* objects = m_parentobject->GetChildObjectList();
        forEachIn( a2dCanvasObjectList, objects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj->GetSelected() )
                i++;
        }

        if ( i <= 1 )
        {
            m_mode = mode_edit;
            PushEditTool( m_modehit );
        }
        else
        {
            m_mode = mode_multiedit;
            PushMultiEditTool( m_modehit );
        }
    }
    else if ( event.Moving() && !GetBusy() )
    {
        DecideMode( hit, hitinfo, event.ShiftDown(), event.ControlDown() );
        event.Skip();
    }
    else if( event.LeftDown() && !GetBusy() )
    {
        // decide tool depending on mode
        switch( m_mode )
        {
            case mode_none:
                event.Skip();
                break;
            case mode_dragmulti:
                PushDragMultiTool( NULL );//m_modehit );
                event.Skip();
                break;
            case mode_copymulti:
                PushCopyMultiTool( NULL );// m_modehit );
                event.Skip();
                break;
                event.Skip();
                break;
            case mode_zoomdrag:
                break;
            case mode_zoom:
            case mode_select:
                event.Skip();
                break;
            case mode_drag:
            case mode_copy:
                break;
            case mode_move_pin:
            case mode_drawwire:
            case mode_editwire_vertex:
            case mode_editwire_segmenthorizontal:
            case mode_editwire_segmentvertical:
            case mode_editwire_segmentinsert:
            case mode_editwire_segment:
                event.Skip();
                break;

            default:
                m_mode = mode_none;
                event.Skip();
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

//----------------------------------------------------------------------------
// a2dMasterDrawZoomFirst
//----------------------------------------------------------------------------

IMPLEMENT_CLASS( a2dMasterDrawZoomFirst, a2dMasterDrawBase )

BEGIN_EVENT_TABLE( a2dMasterDrawZoomFirst, a2dMasterDrawBase )
    EVT_MOUSE_EVENTS( a2dMasterDrawZoomFirst::OnMouseEvent )
    EVT_CHAR( a2dMasterDrawZoomFirst::OnChar )
    EVT_KEY_DOWN( a2dMasterDrawZoomFirst::OnKeyDown )
    EVT_KEY_UP( a2dMasterDrawZoomFirst::OnKeyUp )
END_EVENT_TABLE()

a2dMasterDrawZoomFirst::a2dMasterDrawZoomFirst( a2dStToolContr* controller ): a2dMasterDrawBase( controller )
{
    controller->SetDefaultBehavior( controller->GetDefaultBehavior() | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_Zoom  );
}

a2dMasterDrawZoomFirst::~a2dMasterDrawZoomFirst()
{
}

void a2dMasterDrawZoomFirst::Render()
{
}

void a2dMasterDrawZoomFirst::DoStopTool( bool abort )
{
    a2dMasterDrawBase::DoStopTool( abort );
    m_toolBusy = false;
}

void a2dMasterDrawZoomFirst::AbortBusyMode()
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
                a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
                rec->Update( a2dCanvasObject::updatemask_force );
                GetDrawingPart()->AddPendingUpdateArea( rec, rec->GetBbox() );
                RemoveDecorationObject( rec );
                break;
            }
            default:
                break;
        }
    }

    a2dStTool::AbortBusyMode();
}

void a2dMasterDrawZoomFirst::OnChar( wxKeyEvent& event )
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
                a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
                rec->Update( a2dCanvasObject::updatemask_force );
                GetDrawingPart()->AddPendingUpdateArea( rec, rec->GetBbox() );
                FinishBusyMode();
                RemoveDecorationObject( rec );
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

void a2dMasterDrawZoomFirst::OnKeyDown( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    //wxLogDebug(wxT("key %d"), event.GetKeyCode());
    a2dBaseTool* first = m_stcontroller->GetFirstTool();

    switch( event.GetKeyCode() )
    {
        case WXK_SPACE:
        {
            m_spaceDown = true;
            m_mode = mode_zoom;
            m_modehit = NULL;
            break;
        }
        default:
            event.Skip();
    }

}

void a2dMasterDrawZoomFirst::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    switch( event.GetKeyCode() )
    {
        case WXK_SPACE:
        {
            m_spaceDown = false;
            m_mode = mode_zoom;
            m_modehit = NULL;
            break;
        }
        default:
            event.Skip();
    }

}

void a2dMasterDrawZoomFirst::OnMouseEvent( wxMouseEvent& event )
{
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

    if ( event.LeftDClick() && !m_toolBusy && !GetBusy() )
    {
        a2dCanvasObject* hit = m_modehit; //reuse from the LeftDown event.
        if ( hit )
        {
            hit->SetSelected( m_modehitLastSelectState );
            int i = 0;
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
            m_modehit = hit;
            a2dCanvasObjectList* objects = m_parentobject->GetChildObjectList();
            forEachIn( a2dCanvasObjectList, objects )
            {
                a2dCanvasObject* obj = *iter;
                if ( obj->GetSelected() )
                    i++;
            }

            if ( i > 1 && m_allowMultiEdit )
            {
                m_mode = mode_multiedit;
                PushMultiEditTool( m_modehit );
            }
            else
            {
                /*
                if ( event.ControlDown() )
                {
                    wxURI uri = hit->GetURI();
                    wxString link = uri.BuildURI();
                    if ( !link.IsEmpty() )
                    {
                        wxString file = link;
                        wxString scheme = uri.GetScheme();
                        if ( scheme == wxT( "file" ) )  
                        {
			                wxRegEx reVolume( wxT("^\\/[a-fA-F].*$") );
                            file = uri.GetPath();
                            if ( reVolume.Matches( file ) )
                                file = file.Mid( 1 );
                        } 
    
                        if ( !::wxFileExists( file ) )
                        {
                            file = *( a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "APPLICATION_DATA" ) ) ) + wxFileName::GetPathSeparator() + file;
                            if ( !::wxFileExists( file ) )
                            {
                                //a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s" ), file.c_str() );
                                a2dDocumentPtr doc;
                                a2dError res = a2dDocviewGlobals->GetDocviewCommandProcessor()->FileNew( doc );
                                doc->SetTitle( file, true );
                                doc->SetFilename( wxFileName( file ), true );
                                return;
                            }
                        }
                        a2dDocumentPtr doc;
                        a2dDocviewGlobals->GetDocviewCommandProcessor()->FileOpenCheck( doc, file, true );
                    }
                }
                else
                */
                {
                    m_mode = mode_edit;
                    PushEditTool( m_modehit );
                }
            }
        }
    }
    else if ( event.Dragging() && m_toolBusy && !m_dragStarted && !event.AltDown() )
    {
        if ( abs( m_x - m_dragstartx ) >= 5 || abs( m_y - m_dragstarty ) >= 5 )
        {
            a2dCanvasObject* hit = m_modehit; //use last hit at LeftDown
            if ( event.ControlDown() && !event.ShiftDown() )
            {
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                m_mode = mode_cntrlselect;
            }
            else if ( !event.ControlDown() && event.ShiftDown() )
            {
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
                m_mode = mode_select;
            }
            else 
            {
                if( hit && hit->GetDraggable() && hit->GetSelected() )
                {
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
                    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Zoom ) );
                    m_mode = mode_zoom;
                    m_modehit = hit;//NULL;
                }
            }

            switch( m_mode )
            {
                case mode_zoom:
                case mode_select:
                case mode_cntrlselect:
                {
                    m_dragStarted = true;
                    m_parentobject = GetDrawingPart()->GetShowObject();
                    MouseToToolWorld(  m_dragstartx, m_dragstarty, m_xwprev, m_ywprev );

                    a2dRect* rec = new a2dRect(  m_xwprev, m_ywprev , 0, 0, 0 );
                    m_canvasobject = rec;
                    if ( m_mode == mode_select || m_mode == mode_cntrlselect )
                    {
                        rec->SetFill( m_selectFill );
                        rec->SetStroke( m_selectStroke );
                    }
                    else
                    {
                        rec->SetFill( m_fill );
                        rec->SetStroke( m_stroke );
                    }
                    rec->Update( a2dCanvasObject::updatemask_force );
                    AddDecorationObject( rec );
                    m_pending = true;
                    event.Skip();
                    EnterBusyMode();
                    break;
                }
                case mode_dragmulti:
                {
                    m_dragStarted = true;
                    PushDragMultiTool( m_modehit );
                    event.Skip();
                    break;
                }
                case mode_copymulti:
                {
                    m_dragStarted = true;
                    PushCopyMultiTool( m_modehit );
                    event.Skip();
                    break;
                }
                default:
                    break;
            }
        }
    }
    else if ( event.Dragging() && m_toolBusy && m_dragStarted )
    {
        switch( m_mode )
        {
            case mode_zoom:
            case mode_select:
            case mode_cntrlselect:
            {
                a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );

                MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

                rec->SetWidth( m_xwprev - rec->GetPosX() );
                rec->SetHeight( m_ywprev - rec->GetPosY() );
                rec->SetPending( true );
                //m_pending = true;
                break;
            }
            default:
                break;
        }
    }
/*
    else if ( event.Moving() && !GetBusy() && event.AltDown() )
    {
        // show cursor different depending on the object hit, and set mode for tool to push in case of leftdown.
        m_mode = mode_none;
        m_modehit = 0;
        m_modehitinfo = a2dHit();
        a2dCanvasObject* top = GetDrawingPart()->GetShowObject();
        a2dAffineMatrix cworld = top->GetTransformMatrix();
        cworld.Invert();
        a2dIterCU cu( ic, cworld );
        a2dCanvasObject* hit = top->IsHitWorld( ic, hitinfo );

        if( hit && hit->GetDraggable() && hit->GetSelected() )
        {
            m_modehit = hit;
            if( event.ControlDown() )
            {
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Copy ) );
                m_mode = mode_copymulti;
            }
            else
            {
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                m_mode = mode_dragmulti;
            }
        }
        event.Skip();
    }
*/
    else if ( event.Moving() && !GetBusy() )
    {
        m_mode = mode_none;
        m_modehit = 0;
        m_modehitinfo = a2dHit();

        if ( event.ControlDown() && !event.ShiftDown() )
        {
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
            a2dCanvasObject* top = GetDrawingPart()->GetShowObject();
            a2dCanvasObject* hit = top->IsHitWorld( ic, hitinfo );
            GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( top );
            if ( hit )
                m_modehit = hit;
            m_mode = mode_cntrlselect;
        }
        else if ( !event.ControlDown() && event.ShiftDown() )
        {
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
            a2dCanvasObject* top = GetDrawingPart()->GetShowObject();
            a2dCanvasObject* hit = top->IsHitWorld( ic, hitinfo );
            GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( top );
            if ( hit )
                m_modehit = hit;
            m_mode = mode_select;
        }      
        else
        {
            a2dCanvasObject* top = GetDrawingPart()->GetShowObject();
            a2dCanvasObject* hit = top->IsHitWorld( ic, hitinfo );
            GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( top );
            if ( hit )
            {
                // Check if there is an unconnected pin
                // This has highest priority
                a2dPin* pin = NULL;
                a2dWirePolylineL* wire = 0;
                a2dHit how2;

                if ( m_vertexSegmentEdit )
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
                                assert( 0 );
                                m_modehit = 0;
                                m_modehitinfo = a2dHit();
                        }
                    }        
                }

                // if not editing a wire, search for pins
                if ( m_mode == mode_none )
                {
                    if ( !event.ShiftDown() && !event.ControlDown() )
                    {
                        //there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
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

                    if( pin )
                    {
                        if ( m_movePin )
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
                        if( hit->GetDraggable() && hit->GetSelected() )
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
                            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Zoom ) );
                            m_mode = mode_zoom;
                            m_modehit = hit;
                        }
                    }
                }                
            }
            else
                GetDrawingPart()->SetCursor( m_toolcursor );
        }
        event.Skip();
    }
    else if( event.LeftDown() && !GetBusy() )
    {
        a2dCanvasObject* hit = m_modehit;

        m_dragStarted = false;
        m_xprev = m_x;
        m_yprev = m_y;
        m_dragstartx = m_x;
        m_dragstarty = m_y;
        m_toolBusy = true;

        if( !hit )
		{
            m_mode = mode_none;
			event.Skip();
			return;
	    }

        switch( m_mode )
        {
            case mode_drag:
            case mode_copy:
            case mode_dragmulti:
            case mode_copymulti:
                m_mode = mode_none;
                event.Skip();
                break;

            case mode_move_pin:
            case mode_drawwire:
            case mode_editwire_vertex:
            case mode_editwire_segmenthorizontal:
            case mode_editwire_segmentvertical:
            case mode_editwire_segmentinsert:
            case mode_editwire_segment:
                event.Skip();
                break;

            default:
                m_mode = mode_none;
                event.Skip();
        }
    }
    else if( event.LeftUp() && GetBusy() && m_dragStarted )
    {
        m_toolBusy = false;
        m_dragStarted = false;
        switch( m_mode )
        {
            case mode_zoom:
            {
                a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );

                double w = GetDrawer2D()->WorldToDeviceXRel( rec->GetWidth() );
                double h = GetDrawer2D()->WorldToDeviceYRel( rec->GetHeight() );

                rec->Update( a2dCanvasObject::updatemask_force );
                double x1 = rec->GetBbox().GetMinX();

                double y1 = rec->GetBbox().GetMinY();

                //Get the current window size to put on zoomstack
                a2dBoundingBox* bbox = new a2dBoundingBox( GetDrawer2D()->GetVisibleMinX(),
                        GetDrawer2D()->GetVisibleMinY(),
                        GetDrawer2D()->GetVisibleMaxX(),
                        GetDrawer2D()->GetVisibleMaxY()
                                                         );

                m_stcontroller->GetZoomList().Insert( bbox );
                GetDrawer2D()->SetMappingWidthHeight( x1, y1, fabs( rec->GetWidth() ), fabs( rec->GetHeight() ) );

                FinishBusyMode();
                RemoveDecorationObject( rec );

                event.Skip();
                break;
            }
            case mode_select:
            {
                SetIgnorePendingObjects( false );

                a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
                rec->Update( a2dCanvasObject::updatemask_force );
                GetDrawingPart()->AddPendingUpdateArea( rec, rec->GetBbox() );

                GetCanvasCommandProcessor()->Submit(
                    new a2dCommand_Select( m_parentobject, a2dCommand_Select::Args()
                                           .what( a2dCommand_Select::SelectRect )
                                           .x1( rec->GetBbox().GetMinX() )
                                           .y1( rec->GetBbox().GetMinY() )
                                           .x2( rec->GetBbox().GetMaxX() )
                                           .y2( rec->GetBbox().GetMaxY() )
                                         )
                );
                FinishBusyMode();
                RemoveDecorationObject( rec );
                event.Skip();
                break;
            }
            case mode_cntrlselect:
            {
                DeselectAll();
                SetIgnorePendingObjects( false );

                a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
                rec->Update( a2dCanvasObject::updatemask_force );
                GetDrawingPart()->AddPendingUpdateArea( rec, rec->GetBbox() );

                GetCanvasCommandProcessor()->Submit(
                    new a2dCommand_Select( m_parentobject, a2dCommand_Select::Args()
                                           .what( a2dCommand_Select::SelectRect )
                                           .x1( rec->GetBbox().GetMinX() )
                                           .y1( rec->GetBbox().GetMinY() )
                                           .x2( rec->GetBbox().GetMaxX() )
                                           .y2( rec->GetBbox().GetMaxY() )
                                         )
                );
                FinishBusyMode();
                RemoveDecorationObject( rec );
                event.Skip();
                break;
            }
            default:
                break;
        }
        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Zoom ) );
    }
    else if( event.LeftUp() && m_toolBusy )
    {
        m_toolBusy = false;
        //reserve for double click if it happens
        m_modehitLastSelectState = false;
        if ( m_modehit )
            m_modehitLastSelectState = m_modehit->GetSelected();


        if ( event.ControlDown() )
        {
            DeselectAll();
            if ( m_modehit && m_modehit->GetSelectable() )
                m_modehit->SetSelected( true );
        }
        else if ( event.ShiftDown() )
        {
            if ( m_modehit && m_modehit->GetSelectable() )
                m_modehit->SetSelected( true );
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
        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Zoom ) );
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

//----------------------------------------------------------------------------
// a2dToolProperty
//----------------------------------------------------------------------------

const long SUBMASTER_PUSHTOOL = wxNewId();
const long SUBMASTER_MENUTOOL_DRAG = wxNewId();
const long SUBMASTER_MENUTOOL_REC = wxNewId();
const long SUBMASTER_MENUTOOL_EDIT = wxNewId();

//----------------------------------------------------------------------------
// a2dSubDrawMasterTool
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dSubDrawMasterTool, a2dStTool )

BEGIN_EVENT_TABLE( a2dSubDrawMasterTool, a2dStTool )
    EVT_MOUSE_EVENTS( a2dSubDrawMasterTool::OnMouseEvent )
    EVT_CHAR( a2dSubDrawMasterTool::OnChar )
END_EVENT_TABLE()

a2dSubDrawMasterTool::a2dSubDrawMasterTool( a2dStToolContr* controller ): a2dStTool( controller )
{
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  );
    m_mode = 0;
    m_canvasobject = 0;

    m_mousemenu = new wxMenu( _( "Tool menu" ), ( long )0 );

    a2dSmrtPtr<a2dDragTool> tool = new a2dDragTool( controller );
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );
    tool->SetDeleteOnOutsideDrop( false );
    AppendTool( tool, new wxMenuItem( m_mousemenu, SUBMASTER_MENUTOOL_DRAG, _( "drag" ), _( "pushes drag tool" ) ), 'd' );

    a2dSmrtPtr<a2dRecursiveEditTool> edit = new a2dRecursiveEditTool( controller );
    edit->SetCorridor( m_corridor );
    AppendTool( edit, new wxMenuItem( m_mousemenu, SUBMASTER_MENUTOOL_EDIT, _( "edit" ), _( "pushes edit tool" ) ), 'e' );

    a2dSmrtPtr<a2dDrawRectangleTool> drawrec = new a2dDrawRectangleTool( controller );
    drawrec->SetEditAtEnd( true );
    AppendTool( drawrec, new wxMenuItem( m_mousemenu, SUBMASTER_MENUTOOL_REC, _( "draw rectangle" ), _( "pushes draw rectangle tool" ) ), 'r' );

    m_curTool = drawrec;
}

a2dSubDrawMasterTool::~a2dSubDrawMasterTool()
{
}

void a2dSubDrawMasterTool::OnPostPushTool()
{
    m_curTool->SetCorridor( m_corridor );
    m_controller->PushTool( m_curTool );
}

void a2dSubDrawMasterTool::AppendTool( a2dBaseTool* tool, wxMenuItem* menuItem, wxChar key )
{
    //SetProperty( new a2dToolProperty( PROPID_toolMenu, tool, menuItem, key ) );
    m_mousemenu->Append( menuItem );
}

bool a2dSubDrawMasterTool::ZoomSave()
{
    return true;
}

void a2dSubDrawMasterTool::DoStopTool( bool abort )
{
    a2dStTool::DoStopTool( abort );
}

void a2dSubDrawMasterTool::OnChar( wxKeyEvent& WXUNUSED( event ) )
{
    //switch( event.GetKeyCode() )
    //{
    //}
}

void a2dSubDrawMasterTool::OnMouseEvent( wxMouseEvent& event )
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

    a2dHitEvent hitinfo( xw, yw, false, a2dCANOBJHITOPTION_LAYERS, true );
    a2dCanvasObject* hit = GetDrawingPart()->IsHitWorld( hitinfo, wxLAYER_ALL );

    if ( event.Moving() && !GetBusy() )
    {
        if( hit )
        {
            a2dHit how2;
        }
    }
    else if( event.LeftDown() && !GetBusy() )
    {
    }
    else
    {
        event.Skip();
    }
}

void a2dSubDrawMasterTool::PushToolFromMouseMenu( wxCommandEvent& event )
{
    /*
        a2dToolProperty *property = wxStaticCast( event.m_callbackUserData, a2dToolProperty );
        if ( property->GetMenuItem()->GetId() == event.GetId() )
        {
            a2dSmrtPtr< a2dBaseTool > tool;
            if ( m_controller->GetFirstTool() != this )
                m_controller->PopTool( tool );

            property->GetToolObject()->SetCorridor( m_corridor );
            m_controller->PushTool( property->GetToolObject() );
        }
    */
}

IMPLEMENT_CLASS( a2dGraphicsMasterTool, a2dMasterDrawBase )

BEGIN_EVENT_TABLE( a2dGraphicsMasterTool, a2dMasterDrawBase )
    EVT_MOUSE_EVENTS( a2dGraphicsMasterTool::OnMouseEvent )
    EVT_CHAR( a2dGraphicsMasterTool::OnChar )
    EVT_KEY_DOWN( a2dGraphicsMasterTool::OnKeyDown )
END_EVENT_TABLE()

a2dGraphicsMasterTool::a2dGraphicsMasterTool( a2dStToolContr* controller ): a2dMasterDrawBase( controller )
{
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  );
    m_mode = mode_none;
    m_canvasobject = 0;
    m_modehit = 0;

    controller->SetDefaultBehavior( controller->GetDefaultBehavior() | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
}

a2dGraphicsMasterTool::~a2dGraphicsMasterTool()
{
}

void a2dGraphicsMasterTool::OnMouseEvent( wxMouseEvent& event )
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
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( GetDrawingPart()->GetShowObject() );

        //there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
        // m_connectionGenerator.
        double hitDistance = GetHitMargin();

        //GetDrawing()->GetHabitat()->GetConnectionGenerator()->
		//	        SearchPinForStartWire( GetDrawingPart()->GetShowObject(), a2d_StartWire, xw, yw, a2dPinClass::Any, hitDistance );
    }

	//!todo this can be a specialized hit on pin in connection generator.
	// and if not hit on a correct pin, do a hitworld to see other objects like wires.
	// Or do a connection generator for pin hit, on specific object, instead of root.
    a2dHitEvent hitinfo( xw, yw, false, a2dCANOBJHITOPTION_NONE, true );
    a2dCanvasObject* hit = GetDrawingPart()->IsHitWorld( hitinfo, wxLAYER_ALL );

    /* TO DEBUG
        if( hit )
        {
            wxLogDebug( hit->GetClassInfo()->GetClassName() );
            switch( how.m_hit )
            {
            case a2dHit::hit_none:
                wxLogDebug( "Hit NoHit" );
                break;
            case a2dHit::hit_stroke:
                wxLogDebug( "Hit Stroke" );
                break;
            case a2dHit::hit_fill:
                wxLogDebug( "Hit Fill" );
                break;
            default:
                wxLogDebug( "Hit ?" );
                break;
            }

            switch( how.m_stroke2 )
            {
            case a2dHit::stroke2_none:
                wxLogDebug( "Stroke None" );
                break;
            case a2dHit::stroke2_vertex:
                wxLogDebug( "Stroke Vertex" );
                break;
            case a2dHit::stroke2_edgehor:
                wxLogDebug( "Stroke Horizontal" );
                break;
            case a2dHit::stroke2_edgevert:
                wxLogDebug( "Stroke Vertical" );
                break;
            case a2dHit::stroke2_edgeother:
                wxLogDebug( "Stroke Other" );
                break;
            }
        }
        else
            wxLogDebug( "NoHit" );
    */

    if ( event.Moving() && !GetBusy() )
    {
        m_mode = mode_none;
        m_modehit = 0;
        m_modehitinfo = a2dHit();

        GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( GetDrawingPart()->GetShowObject() );
        if( hit )
        {
            a2dHit how2;

            // Check if there is an unconnected pin
            // This has highest priority

            a2dPin* pin = NULL;
            if ( !event.ShiftDown() && !event.ControlDown() )
            {
                GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( GetDrawingPart()->GetShowObject() );

                //there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
                // m_connectionGenerator.

                GetDrawing()->GetHabitat()->GetConnectionGenerator()->
		        	GeneratePinsToConnect( GetDrawingPart(), hit, a2dPinClass::Any, a2d_GeneratePinsForStartWire, xw, yw );

                pin = GetDrawing()->GetHabitat()->GetConnectionGenerator()->
					        SearchPinForStartWire( GetDrawingPart()->GetShowObject(), xw, yw, a2dPinClass::Any, GetHitMargin() );

                if ( pin )
                {
                    a2dWirePolylineL* wire = wxDynamicCast( pin->GetParent(), a2dWirePolylineL );
                    if ( wire && wire->GetSelected() || m_vertexSegmentEdit )
                        pin = NULL;
                }
            }

            /* INSTEAD OFF?
            a2dPin *pin = 0;
            {
                //First try to find object pins which are not of connection/wire objects
                int i;
                float minDist = FLT_MAX;
                for( i=0; i< hitinfo.m_extended.size(); i++ )
                {
                    a2dPin *pin2 = wxDynamicCast( hitinfo.m_extended[i].GetObject(), a2dPin );
                    if(
                        pin2 &&
                        !pin2->ConnectedTo() &&
                        pin2->GetPinClass() && pin2->GetPinClass()->IsObjectPin() &&
                        hitinfo.m_extended[i].GetHitType().m_distance < minDist
                    )
                    {
                        pin = pin2;
                        how2 = hitinfo.m_extended[i].GetHitType();
                        minDist = how2.m_distance;
                    }
                }
                if ( !pin )
                    //Now try to find object pins which are of connection/wire objects
                    for( i=0; i< hitinfo.m_extended.size(); i++ )
                    {
                        a2dPin *pin2 = wxDynamicCast( hitinfo.m_extended[i].GetObject(), a2dPin );
                        if(
                            pin2 &&
                            !pin2->ConnectedTo() &&
                            pin2->GetPinClass() && !pin2->GetPinClass()->IsObjectPin() ) &&
                            hitinfo.m_extended[i].GetHitType().m_distance < minDist
                        )
                        {
                            pin = pin2;
                            how2 = hitinfo.m_extended[i].GetHitType();
                            minDist = how2.m_distance;
                        }
                    }
            }
            */

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

            if( pin )
            {
                // wire drawing mode
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_DrawWire ) );
                m_mode = mode_drawwire;
                m_modehit = pin;
            }
            else if ( wire && event.ShiftDown() )
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
                        assert( 0 );
                        m_modehit = 0;
                        m_modehitinfo = a2dHit();
                }
            }
            else if ( !event.ShiftDown() && hit->GetDraggable() &&
                      ( hitinfo.m_how.IsInsideHit() ||
                        wxDynamicCast( hit, a2dPolylineL )
                      )
                    )
            {
                if( !event.ControlDown() )
                {
                    if( hit->GetSelected() )
                    {
                        // drag selected mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                        m_mode = mode_dragmulti;
                        m_modehit = hit;
                    }
                    else
                    {
                        // drag mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
                        m_mode = mode_drag;
                        m_modehit = hit;
                    }
                }
                else
                {
                    if( hit->GetSelected() )
                    {
                        // copy selected mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Copy ) );
                        m_mode = mode_copymulti;
                        m_modehit = hit;
                    }
                    else
                    {
                        // drag mode
                        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Copy ) );
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
            case mode_drag:
            case mode_dragmulti:
            case mode_copy:
            case mode_copymulti:
            case mode_drawwire:
            case mode_editwire_vertex:
            case mode_editwire_segmenthorizontal:
            case mode_editwire_segmentvertical:
            case mode_editwire_segmentinsert:
            case mode_editwire_segment:
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

void a2dGraphicsMasterTool::SelectHitObject( a2dCanvasObject* hit )
{
    OpenCommandGroupNamed( _( "Select" ) );

    if ( m_select_undo )
    {
		OpenCommandGroupNamed( _( "Select" ) );
		DeselectAll();
		GetCanvasCommandProcessor()->Submit(
			new a2dCommand_SetFlag( hit, a2dCanvasOFlags::SELECTED, true ) );
		CloseCommandGroup();
	}
    else
    {
        DeselectAll();
        hit->SetSelected( true );
    }

/*
    a2dTagVecProperty* hittagged = a2dCanvasObject::PROPID_Tags->GetPropertyListOnly( hit );
    if ( hittagged )
    {
        if ( !hit->GetSelected() )
        {
            // find other and select
            a2dTag tag = hittagged->Last();
            a2dCanvasObject* ret = NULL;
            for( a2dCanvasObjectList::iterator iter = m_parentobject->GetChildObjectList()->begin(); iter != m_parentobject->GetChildObjectList()->end(); ++iter )
            {
                a2dCanvasObjectList::value_type obj = *iter;
                a2dTagVecProperty* tagsp = a2dCanvasObject::PROPID_Tags->GetPropertyListOnly( obj );
                if ( tagsp )
                {
                    tagsp->PushTag( tag );
                    GetCanvasCommandProcessor()->Submit(
                        new a2dCommand_SetFlag( obj, a2dCanvasOFlags::SELECTED, true ) );
                }
            }
        }
    }
    else
    {
        a2dTag tag;
        a2dCanvasObject* tagged = FindTaggedObject();
        if ( tagged && tagged->GetSelected() )
        {
            // find other and select
            a2dTagVecProperty* tagsp = a2dCanvasObject::PROPID_Tags->GetPropertyListOnly( tagged );
            tag = tagsp->Last();
        }
        else
            tag = a2dNewTag();

        a2dCanvasObject::PROPID_Tags->SetPropertyToObject( hit, a2dTagVec() );   
        a2dTagVecProperty* tagsp = a2dCanvasObject::PROPID_Tags->GetPropertyListOnly( hit );
        tagsp->PushTag( tag );

        GetCanvasCommandProcessor()->Submit(
            new a2dCommand_SetFlag( hit, a2dCanvasOFlags::SELECTED, true ) );

    }
*/
    CloseCommandGroup();
}
