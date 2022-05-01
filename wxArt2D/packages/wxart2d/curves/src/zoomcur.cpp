/*! \file curves/src/zoomcur.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: zoomcur.cpp,v 1.20 2008/08/19 23:17:11 titato Exp $
*/

#pragma warning(disable:4786)

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/wfstream.h>
#include <math.h>

#include "wx/canvas/eval.h"
#include "wx/curves/meta.h"

#include "wx/canvas/canobj.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/canvas.h"

#if wxART2D_USE_EDITOR
#include "wx/canvas/edit.h"
#endif //wxART2D_USE_EDITOR

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

IMPLEMENT_CLASS( a2dCurveZoomTool, a2dStTool )


//! used to set internal boundaries of canvas plot objects
/*!
*/
class a2dCommand_SetInternalBoundaries: public a2dCommand
{

public:

    static const a2dCommandId COMID_SetInternalBoundaries;

    a2dCommand_SetInternalBoundaries( a2dCanvasXYDisplayGroupAreas* object, a2dBboxHash* irectHash );
//          double x1, double y1, double x2, double y2 );
    ~a2dCommand_SetInternalBoundaries( void );

    bool Do( void );
    bool Undo( void );

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

protected:

    a2dSmrtPtr<a2dBboxHash> m_irectHash;
//    double m_x1;
//    double m_y1;
//    double m_x2;
//    double m_y2;

    a2dSmrtPtr<a2dCanvasXYDisplayGroupAreas> m_canvasobject;


};

const a2dCommandId a2dCommand_SetInternalBoundaries::COMID_SetInternalBoundaries( wxT( "SetInternalBoundaries" ) );

/*
*   a2dCommand_SetInternalBoundaries
*/

a2dCommand_SetInternalBoundaries::a2dCommand_SetInternalBoundaries(
    a2dCanvasXYDisplayGroupAreas* object, a2dBboxHash* irectHash ) :
//      double x1, double y1, double x2, double y2):
    a2dCommand( true, COMID_SetInternalBoundaries ), m_irectHash( irectHash )
{
    m_canvasobject = object;
//    m_x1 = x1;
//    m_y1 = y1;
//    m_x2 = x2;
//    m_y2 = y2;
}

a2dCommand_SetInternalBoundaries::~a2dCommand_SetInternalBoundaries( void )
{
}

bool a2dCommand_SetInternalBoundaries::Do( void )
{
    a2dBboxHash* oldIRectHash = new a2dBboxHash;
    const a2dCurvesAreaList& areaList = m_canvasobject->GetCurvesAreaList();
    size_t i;
    for( i = 0; i < areaList.GetCount(); i++ )
    {
        a2dCurvesArea* area = areaList.Item( i );
        const a2dBoundingBox& aRect = area->GetInternalBoundaries();
        ( *oldIRectHash )[area->GetName()] = aRect;
    }

    for( i = 0; i < areaList.GetCount(); i++ )
    {
        a2dCurvesArea* area = areaList.Item( i );
        const a2dBoundingBox& aRect = ( *m_irectHash )[area->GetName()];
        area->SetInternalBoundaries( m_canvasobject->GetPlotAreaRect(), aRect );
    }
    m_canvasobject->SetPending( true );
    m_canvasobject->GetAxisX()->SetPending( true );
    if( m_canvasobject->GetCursor() )
        m_canvasobject->GetCursor()->SetPending( true );

    m_irectHash = oldIRectHash;

    m_canvasobject->Update( a2dCanvasObject::updatemask_normal );

    m_canvasobject->GetRoot()->Modify( true );
    m_canvasobject->GetRoot()->UpdateAllViews( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_VIEWDEPENDENT | a2dCANVIEW_UPDATE_VIEWDEPENDENT_RIGHTNOW );

    return true;
}

bool a2dCommand_SetInternalBoundaries::Undo( void )
{
    a2dBboxHash* oldIRectHash = new a2dBboxHash;
    const a2dCurvesAreaList& areaList = m_canvasobject->GetCurvesAreaList();
    size_t i;
    for( i = 0; i < areaList.GetCount(); i++ )
    {
        a2dCurvesArea* area = areaList.Item( i );
        const a2dBoundingBox& aRect = area->GetInternalBoundaries();
        ( *oldIRectHash )[area->GetName()] = aRect;
    }

    for( i = 0; i < areaList.GetCount(); i++ )
    {
        a2dCurvesArea* area = areaList.Item( i );
        const a2dBoundingBox& aRect = ( *m_irectHash )[area->GetName()];
        area->SetInternalBoundaries( m_canvasobject->GetPlotAreaRect(), aRect );
    }
    m_canvasobject->SetPending( true );
    m_canvasobject->GetAxisX()->SetPending( true );
    if( m_canvasobject->GetCursor() )
        m_canvasobject->GetCursor()->SetPending( true );

    m_irectHash = oldIRectHash;

    m_canvasobject->Update( a2dCanvasObject::updatemask_normal );

    m_canvasobject->GetRoot()->Modify( true );
    m_canvasobject->GetRoot()->UpdateAllViews( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_VIEWDEPENDENT | a2dCANVIEW_UPDATE_VIEWDEPENDENT_RIGHTNOW );

    return true;
}


//----------------------------------------------------------------------------
// a2dCurveZoomTool
//----------------------------------------------------------------------------

#if wxART2D_USE_EDITOR

#define CONTRDRAWER m_controller->GetDrawingPart()
#define CONTRDRAWER2D m_controller->GetDrawingPart()->GetDrawer2D()

BEGIN_EVENT_TABLE( a2dCurveZoomTool, a2dStTool )
    EVT_CHAR( a2dCurveZoomTool::OnChar )
    EVT_MOUSE_EVENTS( a2dCurveZoomTool::OnMouseEvent )
END_EVENT_TABLE()

a2dCurveZoomTool::a2dCurveZoomTool( a2dStToolContr* controller ): a2dStTool( controller )
{
    m_anotate = true;//false;
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_MAGNIFIER );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );

    //stroke and fill should not change as in base class for m_canvasobject

    //i like transparent fill and specific stroke
    SetFill( *a2dTRANSPARENT_FILL );
    SetStroke( a2dStroke( *wxBLACK, 1, a2dSTROKE_LONG_DASH ) );
}

a2dCurveZoomTool::~a2dCurveZoomTool()
{
}

void a2dCurveZoomTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStTool::AddAnotationToPendingUpdateArea();

    a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
    m_anotation.Printf( wxT( "w %6.3f h %6.3f" ) , rec->GetWidth(), rec->GetHeight() );
    m_xanotation = m_x;
    m_yanotation = m_y;

    wxDC* dc = GetDrawer2D()->GetRenderDC();
    if ( dc )
    {
        wxCoord w, h;
        dc->SetFont( m_annotateFont );
        dc->GetTextExtent( m_anotation, &w, &h );
        m_xanotation = m_x + ( int ) h;
        m_yanotation = m_y + ( int ) h;
        a2dStTool::AddAnotationToPendingUpdateArea();
    }
}

void a2dCurveZoomTool::OnChar( wxKeyEvent& event )
{
    if ( !GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_RETURN:
            {
                m_stcontroller->Zoomout();
            }
            break;
            case WXK_SUBTRACT:
            {
                m_stcontroller->ZoomUndo();
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();
}

void a2dCurveZoomTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    if ( GetBusy() )
        GetDrawingPart()->SetCursor( m_toolBusyCursor );
    else
        GetDrawingPart()->SetCursor( m_toolcursor );

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    if ( event.LeftDClick() && !GetBusy() )
    {

        a2dCanvasObject* object;
        object = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev );

        if ( !object )
            return;
        if ( 0 == wxDynamicCast( object , a2dCanvasXYDisplayGroupAreas ) )
        {
            m_curves = 0;
            return;
        }
        m_curves = ( a2dCanvasXYDisplayGroupAreas* )object;

        a2dBboxHash* aHash = new a2dBboxHash;
//      m_curves->AppendInternalBoundaries(aHash);
        m_curves->AppendCurvesBoundaries( aHash );

        GetDrawingPart()->GetDrawing()->GetCommandProcessor()->Submit(
            new a2dCommand_SetInternalBoundaries( m_curves, aHash ) );

        m_curves->SetPending( true );
    }
    else if ( event.LeftDown() && !GetBusy() )
    {
        a2dCanvasObject* object;
        object = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev );

        if ( !object )
            return;
        if ( 0 == wxDynamicCast( object , a2dCanvasXYDisplayGroupAreas ) )
        {
            m_curves = 0;
            return;
        }
        m_curves = ( a2dCanvasXYDisplayGroupAreas* )object;

        a2dRect* rec = new a2dRect( m_xwprev, m_ywprev, 0, 0, 0 );
        m_parentobject = GetDrawingPart()->GetShowObject();
        m_canvasobject = rec;
        AddDecorationObject( rec );

        rec->SetRoot( m_parentobject->GetRoot() );
        rec->SetFill( m_fill );
        rec->SetStroke( m_stroke );
        rec->Update( a2dCanvasObject::updatemask_force );
        m_pending = true;
        EnterBusyMode();

        //special case to have the canvas itself recieve at least this one also.
        //because often the zoomtool is active as only tool, but
        //some object on the canvas are meant to react to MD (Href's etc.)
        event.Skip();
    }
    else if ( event.LeftUp() /*event.LeftDown()*/ && GetBusy() )
    {
        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );

        double w = GetDrawer2D()->WorldToDeviceXRel( rec->GetWidth() );
        double h = GetDrawer2D()->WorldToDeviceYRel( rec->GetHeight() );

        a2dAffineMatrix tworld = m_curves->GetTransformMatrix();

        a2dAffineMatrix inverse = tworld;
        inverse.Invert();

        //to be able to intercept doubleclick
        //ignore the LeftDown and LeftUp if mouse position is the same
        if ( fabs( w ) < 3 && fabs( h ) < 3 )
        {
            FinishBusyMode();
            SetPending( true );
            a2dCursor* aCursor = m_curves->GetCursor();
            if( aCursor )
            {
//              m_curves->SetCursorPosition(rec->GetPosX(),rec->GetPosY());
                double xwi;
                double ywi;
                inverse.TransformPoint( rec->GetPosX(), rec->GetPosY(), xwi, ywi );
                a2dSmrtPtr<a2dCurvesArea> area = m_curves->GetCurvesAreaList().GetBaseCurvesArea();
                double xcurve, ycurve;
                area->World2Curve( xwi, ywi, xcurve, ycurve );
                aCursor->SetPosition( xcurve, ycurve, area->GetCurveAreaTransform() );
            }
            event.Skip();
        }
        else
        {
            double xwi;
            double ywi;
            double x2wi;
            double y2wi;
            inverse.TransformPoint( rec->GetBbox().GetMinX(), rec->GetBbox().GetMinY(), xwi, ywi );
            inverse.TransformPoint( rec->GetBbox().GetMaxX(), rec->GetBbox().GetMaxY(), x2wi, y2wi );

            a2dBboxHash* aHash = new a2dBboxHash;

            const a2dCurvesAreaList& areaList = m_curves->GetCurvesAreaList();
            for( size_t i = 0; i < areaList.GetCount(); i++ )
            {
                a2dCurvesArea* area = areaList.Item( i );

                double xcurve, ycurve;
                double xcurve2, ycurve2;
                area->World2Curve( xwi, ywi, xcurve, ycurve );
                area->World2Curve( x2wi, y2wi, xcurve2, ycurve2 );

                a2dBoundingBox aRect( xcurve, ycurve, xcurve2, ycurve2 );
                ( *aHash )[area->GetName()] = aRect;
            }

            GetDrawingPart()->GetDrawing()->GetCommandProcessor()->Submit(
                new a2dCommand_SetInternalBoundaries( m_curves, aHash ) );

            m_curves->SetPending( true );

            FinishBusyMode();
            event.Skip();
        }
    }
    else  if ( ( event.Dragging() && GetBusy() ) /*|| (event.Moving() && GetBusy())*/ )
    {
        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
        rec->SetWidth( m_xwprev - rec->GetPosX() );
        rec->SetHeight( m_ywprev - rec->GetPosY() );

        m_pending = true;
    }
    else
        event.Skip();
}

#endif //wxART2D_USE_EDITOR



