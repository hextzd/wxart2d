/*! \file editor/src/sttoolext.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: sttoolext.cpp,v 1.28 2008/08/19 23:17:13 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "a2dprivate.h"

#if wxART2D_USE_CANEXTOBJ

#include "wx/canvas/canmod.h"
#include "wx/filename.h"

#include "wx/canvas/sttool.h"
#include "wx/canextobj/sttoolext.h"

IMPLEMENT_CLASS( a2dRenderImageZoomTool, a2dStTool )

const a2dCommandId a2dRenderImageZoomTool::COMID_PushTool_RenderImageZoom( wxT( "PushTool_RenderImageZoom" ) );

BEGIN_EVENT_TABLE( a2dRenderImageZoomTool, a2dStTool )
    EVT_CHAR( a2dRenderImageZoomTool::OnChar )
    EVT_MOUSE_EVENTS( a2dRenderImageZoomTool::OnMouseEvent )
END_EVENT_TABLE()

a2dRenderImageZoomTool::a2dRenderImageZoomTool( a2dStToolContr* controller ): a2dStTool( controller )
{
    GetDrawingPart()->SetMouseEvents( false );
    m_stcontroller = controller;
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_MAGNIFIER );
    m_stroke = a2dStroke( *wxBLACK, 0 , a2dSTROKE_LONG_DASH );
}

a2dRenderImageZoomTool::~a2dRenderImageZoomTool()
{
}

void a2dRenderImageZoomTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_SPACE:
            {
                FinishBusyMode();
                //to be save
                m_canvasobject = 0;
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();

}

void a2dRenderImageZoomTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    if ( GetBusy() )
        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
    else
        GetDrawingPart()->SetCursor( m_toolcursor );

    int x = event.GetX();
    int y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw = GetDrawer2D()->DeviceToWorldX( x );
    double yw = GetDrawer2D()->DeviceToWorldY( y );

    wxPoint pos = event.GetPosition();

    if ( event.LeftDClick() && !GetBusy() )
    {
        a2dCanvasObject* object;
        object = GetDrawingPart()->IsHitWorld( xw, yw );

        if ( !object )
            return;
        if ( 0 == wxDynamicCast( object , a2dRenderImage ) )
        {
            m_renderimage = 0;
            return;
        }
        m_renderimage = ( a2dRenderImage* )object;

        m_renderimage->SetMappingWidthHeight( m_renderimage->GetShowObject()->GetBboxMinX(),
                                              m_renderimage->GetShowObject()->GetBboxMinY(),
                                              m_renderimage->GetShowObject()->GetBboxWidth(),
                                              m_renderimage->GetShowObject()->GetBboxHeight()
                                            );
    }
    else if ( event.LeftDown() )
    {
        a2dCanvasObject* object;
        object = GetDrawingPart()->IsHitWorld( xw, yw );

        if ( !object )
            return;
        if ( 0 == wxDynamicCast( object , a2dRenderImage ) )
        {
            m_renderimage = 0;
            return;
        }

        m_renderimage = ( a2dRenderImage* )object;

        m_zoom_x1 = m_zoom_x2 = x;
        m_zoom_y1 = m_zoom_y2 = y;
        EnterBusyMode();
        event.Skip();
    }
    else if ( event.RightDown() )
    {
        event.Skip();
    }
    else if ( event.LeftUp() && GetBusy() )
    {
        //to be able to intercept doubleclick
        //ignore the LeftDown and LeftUp if mouse position is the same
        if ( abs( m_zoom_x1 - m_zoom_x2 ) < 3 && abs( m_zoom_y1 - m_zoom_y2 ) < 3 )
        {
            FinishBusyMode();
            GetDisplayWindow()->Refresh();
            if ( m_oneshot )
                StopTool();
            event.Skip();
        }
        else
        {
            m_zoom_x2 = pos.x;
            m_zoom_y2 = pos.y;
            //wich way the zoom rectangle is drawn must be irrelevant
            //first determine lefttop and bottomright in device coordinates
            int topx = wxMin( m_zoom_x1, m_zoom_x2 );
            int topy = wxMin( m_zoom_y1, m_zoom_y2 );
            int botx = wxMax( m_zoom_x1, m_zoom_x2 );
            int boty = wxMax( m_zoom_y1, m_zoom_y2 );

            double x1 = GetDrawer2D()->DeviceToWorldX( topx );
            double y1 = GetDrawer2D()->DeviceToWorldY( topy );
            double x2 = GetDrawer2D()->DeviceToWorldX( botx );
            double y2 = GetDrawer2D()->DeviceToWorldY( boty );

            a2dIterC ic( GetDrawingPart() );
            if ( m_renderimage->GetYaxis() )
                m_renderimage->SetMappingAbs( ic, x1, y2, fabs( x1 - x2 ), fabs( y1 - y2 ) );
            else
                m_renderimage->SetMappingAbs( ic, x1, y1, fabs( x1 - x2 ), fabs( y1 - y2 ) );

            FinishBusyMode();
            event.Skip();
        }
    }
    else if ( event.Dragging() && event.m_leftDown && GetBusy() )
    {
        wxClientDC dc( GetDisplayWindow() );
        GetDisplayWindow()->PrepareDC( dc );
        wxPen pen( m_stroke.GetColour(), 0, wxPENSTYLE_SOLID );
        dc.SetPen( pen );
        dc.SetBrush( wxNullBrush );
        dc.SetLogicalFunction( wxINVERT );
        dc.DrawRectangle( m_zoom_x1, m_zoom_y1, m_zoom_x2 - m_zoom_x1, m_zoom_y2 - m_zoom_y1 );
        m_zoom_x2 = pos.x;
        m_zoom_y2 = pos.y;
        dc.DrawRectangle( m_zoom_x1, m_zoom_y1, m_zoom_x2 - m_zoom_x1, m_zoom_y2 - m_zoom_y1 );
        dc.SetBrush( wxNullBrush );
        dc.SetPen( wxNullPen );
    }
    else
        event.Skip();
}

#endif // wxART2D_USE_CANEXTOBJ
