/*! \file canvas/src/sttool.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: sttool.cpp,v 1.389 2009/10/01 19:22:35 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <algorithm>

#include "wx/canvas/canmod.h"
#include "wx/filename.h"

#include "wx/canvas/sttool.h"
#include "wx/canvas/edit.h"

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

#define CDRAWER2D GetDrawingPart()->GetDrawer2D()

//#include "wx/genart/prfl.h"
//#include "wx/genart/prfltmr.h"
//#include "wx/genart/prflenbl.h"

IMPLEMENT_CLASS( a2dStToolContr, a2dToolContr )
IMPLEMENT_CLASS( a2dStTool, a2dBaseTool )
IMPLEMENT_CLASS( a2dZoomTool, a2dStTool )
IMPLEMENT_CLASS( a2dDragTool, a2dStTool )
IMPLEMENT_CLASS( a2dDragOrgTool, a2dStTool )
IMPLEMENT_CLASS( a2dDragNewTool, a2dDragTool )
IMPLEMENT_CLASS( a2dCopyTool, a2dDragTool )
IMPLEMENT_CLASS( a2dRotateTool, a2dStTool )
IMPLEMENT_CLASS( a2dDeleteTool, a2dStTool )
IMPLEMENT_CLASS( a2dSelectTool, a2dStTool )
IMPLEMENT_CLASS( a2dStDrawTool, a2dStTool )
IMPLEMENT_CLASS( a2dImageTool, a2dStDrawTool )
IMPLEMENT_CLASS( a2dDrawRectangleTool, a2dStDrawTool )
IMPLEMENT_CLASS( a2dDrawCircleTool, a2dStDrawTool )
IMPLEMENT_CLASS( a2dDrawLineTool, a2dStDrawTool )
IMPLEMENT_CLASS( a2dDrawEllipseTool, a2dStDrawTool )
IMPLEMENT_CLASS( a2dDrawEllipticArcTool, a2dStDrawTool )
IMPLEMENT_CLASS( a2dDrawArcTool, a2dStDrawTool )
IMPLEMENT_CLASS( a2dDrawPolylineLTool, a2dStDrawTool )
IMPLEMENT_CLASS( a2dDrawWirePolylineLTool, a2dDrawPolylineLTool )
IMPLEMENT_CLASS( a2dDrawPolygonLTool, a2dStDrawTool )
IMPLEMENT_CLASS( a2dDrawVPathTool, a2dStDrawTool )
IMPLEMENT_CLASS( a2dDrawTextTool, a2dStDrawTool )
IMPLEMENT_CLASS( a2dDragMultiTool, a2dStTool )
IMPLEMENT_CLASS( a2dDragMultiNewTool, a2dDragMultiTool )
IMPLEMENT_CLASS( a2dCopyMultiTool, a2dDragMultiTool )
IMPLEMENT_CLASS( a2dPropertyTool, a2dStTool )
IMPLEMENT_CLASS( a2dFollowLink, a2dStTool )
IMPLEMENT_CLASS( a2dMovePinTool, a2dStTool )

const a2dCommandId a2dDrawRectangleTool::COMID_PushTool_DrawRectangle( wxT( "PushTool_DrawRectangle" ) );
const a2dCommandId a2dZoomTool::COMID_PushTool_Zoom( wxT( "PushTool_Zoom" ) );
const a2dCommandId a2dSelectTool::COMID_PushTool_Select( wxT( "PushTool_Select" ) );
const a2dCommandId a2dImageTool::COMID_PushTool_Image( wxT( "PushTool_Image" ) );
const a2dCommandId a2dImageTool::COMID_PushTool_Image_Embedded( wxT( "PushTool_Image_Embedded" ) );
const a2dCommandId a2dDragTool::COMID_PushTool_Drag( wxT( "PushTool_Drag" ) );
const a2dCommandId a2dDragOrgTool::COMID_PushTool_DragOrg( wxT( "PushTool_DragOrg" ) );
const a2dCommandId a2dDragNewTool::COMID_PushTool_DragNew( wxT( "PushTool_DragNew" ) );
const a2dCommandId a2dCopyTool::COMID_PushTool_Copy( wxT( "PushTool_Copy" ) );
const a2dCommandId a2dRotateTool::COMID_PushTool_Rotate( wxT( "PushTool_Rotate" ) );
const a2dCommandId a2dDeleteTool::COMID_PushTool_Delete( wxT( "PushTool_Delete" ) );
const a2dCommandId a2dDrawCircleTool::COMID_PushTool_DrawCircle( wxT( "PushTool_DrawCircle" ) );
const a2dCommandId a2dDrawLineTool::COMID_PushTool_DrawLine( wxT( "PushTool_DrawLine" ) );
const a2dCommandId a2dDrawLineTool::COMID_PushTool_DrawLineScaledArrow( wxT( "PushTool_DrawLineScaledArrow" ) );
const a2dCommandId a2dDrawEllipseTool::COMID_PushTool_DrawEllipse( wxT( "PushTool_DrawEllipse" ) );
const a2dCommandId a2dDrawEllipticArcTool::COMID_PushTool_DrawEllipticArc( wxT( "PushTool_EllipticArc" ) );
const a2dCommandId a2dDrawEllipticArcTool::COMID_PushTool_DrawEllipticArc_Chord( wxT( "PushTool_EllipticArcChord" ) );
const a2dCommandId a2dDrawArcTool::COMID_PushTool_DrawArc( wxT( "PushTool_DrawArc" ) );
const a2dCommandId a2dDrawArcTool::COMID_PushTool_DrawArc_Chord( wxT( "PushTool_DrawArcChord" ) );
const a2dCommandId a2dDragMultiTool::COMID_PushTool_DragMulti( wxT( "PushTool_DragMulti" ) );
const a2dCommandId a2dCopyMultiTool::COMID_PushTool_CopyMulti( wxT( "PushTool_CopyMulti" ) );
const a2dCommandId a2dDrawTextTool::COMID_PushTool_DrawText( wxT( "PushTool_DrawText" ) );
const a2dCommandId a2dPropertyTool::COMID_PushTool_Property( wxT( "PushTool_Property" ) );
const a2dCommandId a2dDrawVPathTool::COMID_PushTool_DrawVPath( wxT( "PushTool_DrawVPath" ) );
const a2dCommandId a2dFollowLink::COMID_PushTool_FollowLink( wxT( "PushTool_FollowLink" ) );

WX_DEFINE_LIST( wxZoomList );

extern const long TC_UNDO = wxNewId();
extern const long TC_REDO = wxNewId();
extern const long TC_ENDTOOL = wxNewId();
extern const long TC_ZOOMIN = wxNewId();
extern const long TC_ZOOMOUT = wxNewId();
extern const long TC_ZOOMBACK = wxNewId();
extern const long TC_ZOOMOUT2 = wxNewId();
extern const long TC_ZOOMIN2 = wxNewId();
extern const long TC_ZOOMPANXY = wxNewId();
extern const long TC_PROPERTY_XY = wxNewId();

#define  DEFAULTOpacityFactor 125 ;

const a2dPropertyIdBool a2dStToolContr::PROPID_zoomfirst( wxT( "zoomfirst" ), a2dPropertyId::flag_none, false );

DEFINE_EVENT_TYPE( a2dStToolContr::sm_showCursor )

BEGIN_EVENT_TABLE( a2dStToolContr, a2dToolContr )
    EVT_COM_EVENT( a2dStToolContr::OnComEvent )
    EVT_MOUSE_EVENTS( a2dStToolContr::OnMouseEvent )
    EVT_CHAR( a2dStToolContr::OnChar )
    EVT_KEY_DOWN( a2dStToolContr::OnKeyDown )
    EVT_KEY_UP( a2dStToolContr::OnKeyUp )
    EVT_MENU( TC_UNDO            , a2dStToolContr::Undo )
    EVT_MENU( TC_REDO            , a2dStToolContr::Redo )
    EVT_MENU( TC_ENDTOOL         , a2dStToolContr::EndTool )
    EVT_MENU( TC_ZOOMIN          , a2dStToolContr::ZoomWindowMenu )
    EVT_MENU( TC_ZOOMBACK        , a2dStToolContr::ZoomUndoMenu )
    EVT_MENU( TC_ZOOMOUT         , a2dStToolContr::ZoomoutMenu )
    EVT_MENU( TC_ZOOMPANXY       , a2dStToolContr::ZoomPanMenu )
    EVT_MENU( TC_ZOOMOUT2        , a2dStToolContr::Zoomout2Menu )
    EVT_MENU( TC_ZOOMIN2         , a2dStToolContr::Zoomin2Menu )
    EVT_MENU( TC_PROPERTY_XY     , a2dStToolContr::EditProperties )
    EVT_MENUSTRINGS( a2dStToolContr::OnSetmenuStrings )
    EVT_IDLE(  a2dStToolContr::OnIdle )

END_EVENT_TABLE()


a2dStToolContr::a2dStToolContr( a2dDrawingPart* drawingPart, wxFrame* where, bool noStatusEvent ): a2dToolContr( drawingPart )
{
    m_selectionStateUndo = false;
    m_selectedAtEnd = false;

    m_defaultEventHandler = new a2dStToolEvtHandler( this );
    m_defaultEventHandlerFixedStyle = new a2dStToolFixedToolStyleEvtHandler( this );

    m_noStatusEvent = noStatusEvent;
    m_format = wxT( "%6.1f" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_statusStrings.push_back( "" );
    m_drawingPart->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW ) );

    m_useEditOpaque = false;
    m_editOpacityFactor = DEFAULTOpacityFactor;
    m_zoomoutCentered = true;
    m_draggingCanvas = false;
    m_draggingCanvasOption = true;
    m_dragstartx = 0;
    m_dragstarty = 0;
    m_mouse_x = m_mouse_y = 0;

    m_dragmode = wxDRAW_COPY;
    m_drawmode = a2dFILLED;
    m_defaultBehavior = wxTC_DefaultBehavior;
    m_where = where;
	m_noSnapkey = 0;

    m_topTool = NULL;
    m_toolForZoom = new a2dZoomTool( this );
    //OR
    //m_topTool = m_toolForZoom;
    //PushTool(m_topTool);

    m_mousemenu = new wxMenu( _( "ZOOM" ), ( long )0 );
    m_mousemenu->Append( TC_ENDTOOL, _( "end tool" ) );
    m_mousemenu->AppendSeparator();
    m_mousemenu->Append( TC_UNDO, _( "undo" ) );
    m_mousemenu->Append( TC_REDO, _( "redo" ) );
    m_mousemenu->AppendSeparator();
    m_mousemenu->Append( TC_ZOOMIN,    _( "Zoom Window" ) );
    m_mousemenu->Append( TC_ZOOMBACK,  _( "Zoom Undo" ) );
    m_mousemenu->Append( TC_ZOOMOUT,   _( "Zoom out" ) );
    m_mousemenu->Append( TC_ZOOMOUT2,  _( "Zoom out 2X" ) );
    m_mousemenu->Append( TC_ZOOMIN2,  _( "Zoom in 2X" ) );
    m_mousemenu->Append( TC_ZOOMPANXY, _( "Pan to XY" ) );
    m_mousemenu->Append( TC_PROPERTY_XY, _( "Edit Properties" ) );
}


a2dStToolContr::~a2dStToolContr()
{
    if( m_mousemenu )
        delete m_mousemenu;

    m_drawingPart = NULL;

    a2dSmrtPtr< a2dBaseTool > tool;
    a2dToolContr::PopTool( tool );
    while ( tool )
    {
        tool = NULL;
        a2dToolContr::PopTool( tool );
    }

    m_zoomstack.DeleteContents( true );
}

void a2dStToolContr::ReStart()
{
    StopAllTools();
    if ( m_topTool )
    {
        m_topTool->ResetContext();
        PushTool( m_topTool );
    }
}

void a2dStToolContr::SetMousePopupMenu( wxMenu* mousemenu )
{
    if ( m_mousemenu )
        delete m_mousemenu;
    m_mousemenu = mousemenu;
}

void a2dStToolContr::Undo( wxCommandEvent& )
{
    a2dDrawing* drawing = m_drawingPart->GetDrawing();
    drawing->GetCommandProcessor()->Undo();
}

void a2dStToolContr::Redo( wxCommandEvent& )
{
    a2dDrawing* drawing = m_drawingPart->GetDrawing();
    drawing->GetCommandProcessor()->Redo();
}

void a2dStToolContr::EndTool( wxCommandEvent& )
{
    a2dSmrtPtr< a2dBaseTool > tool;
    if ( GetFirstTool() && GetFirstTool()->AllowPop() )
        PopTool( tool );
}

void a2dStToolContr::EditProperties( wxCommandEvent& )
{
    a2dPropertyTool* propedit = new a2dPropertyTool( this );
    PushTool( propedit );
    propedit->SetOneShot();
    propedit->StartEditing( m_mouse_x, m_mouse_y );
}

void a2dStToolContr::OnSetmenuStrings( a2dCommandProcessorEvent& event )
{
    if ( m_drawingPart->GetDrawing() && 
         m_drawingPart->GetDrawing()->GetCommandProcessor() &&
         event.GetEventObject() == m_drawingPart->GetDrawing()->GetCommandProcessor() &&
         m_mousemenu
       )
    {
        if ( m_mousemenu->FindItem( TC_UNDO ) )
        {
            m_mousemenu->SetLabel( TC_UNDO, event.GetUndoMenuLabel() );
            m_mousemenu->Enable( TC_UNDO, event.CanUndo() );
        }
        if ( m_mousemenu->FindItem( TC_REDO ) )
        {
            m_mousemenu->SetLabel( TC_REDO, event.GetRedoMenuLabel() );
            m_mousemenu->Enable( TC_REDO, event.CanRedo() );
        }
    }
    event.Skip();
}

void a2dStToolContr::SetZoomFirst( bool zoomfirst )
{
    if ( !GetFirstTool() && zoomfirst )
    {
        m_topTool = m_toolForZoom;
        PushTool( m_topTool );
    }
}

void a2dStToolContr::SetTopTool( a2dBaseTool* toolOnTop )
{
    m_topTool = toolOnTop;
    if ( !GetFirstTool() && m_topTool )
        PushTool( m_topTool );
}

void a2dStToolContr::SetZoomTool( a2dBaseTool* toolForZoom )
{
    wxASSERT_MSG( wxDynamicCast( toolForZoom, a2dStTool ) != 0, wxT( "Only a2dStTool can be pushed into a2dStToolContr, and not NULL" ) );
    m_toolForZoom = toolForZoom;
}

void a2dStToolContr::ZoomWindowMenu( wxCommandEvent& )
{
    ZoomWindow();
}

void a2dStToolContr::ZoomPanMenu( wxCommandEvent& )
{
    ZoomPan();
}

void a2dStToolContr::ZoomUndoMenu( wxCommandEvent& )
{
    ZoomUndo();
}

void a2dStToolContr::ZoomoutMenu( wxCommandEvent& )
{
    Zoomout();
}

void a2dStToolContr::Zoomout2Menu( wxCommandEvent& )
{
    Zoomout2();
}

void a2dStToolContr::Zoomin2Menu( wxCommandEvent& )
{
    Zoomin2();
}

void a2dStToolContr::ZoomWindow()
{
    a2dBaseTool* tool = m_toolForZoom->TClone( clone_deep );
    tool->SetOneShot();
    PushTool( tool );
}

void a2dStToolContr::ZoomPan()
{
    double mouse_worldx, mouse_worldy;
    GetDrawingPart()->MouseToToolWorld( m_mouse_x, m_mouse_y, mouse_worldx, mouse_worldy );
    mouse_worldx -= CDRAWER2D->GetVisibleWidth() / 2;
    mouse_worldy -= CDRAWER2D->GetVisibleHeight() / 2;

    Zoom( mouse_worldx, mouse_worldy, CDRAWER2D->GetUppX(), CDRAWER2D->GetUppY() );
}

void a2dStToolContr::ZoomUndo()
{
    a2dBoundingBox zoom;
    if ( !m_zoomstack.size() )
    {
        Zoomout();
        return;
    }
    wxZoomList::compatibility_iterator node = m_zoomstack.GetFirst();
    zoom = *node->GetData();
    delete node->GetData();
    m_zoomstack.DeleteNode( node );

    CDRAWER2D->SetMappingWidthHeight( zoom.GetMinX(), zoom.GetMinY(), zoom.GetWidth(), zoom.GetHeight() );
}

void a2dStToolContr::Zoomout()
{
    if ( wxDynamicCast( m_drawingPart->GetDisplayWindow(), a2dCanvas ) )
    {
        a2dCanvas* can = wxStaticCast( m_drawingPart->GetDisplayWindow(), a2dCanvas );
        can->SetMappingShowAll( m_zoomoutCentered );
    }
    else if ( wxDynamicCast( m_drawingPart->GetDisplayWindow(), a2dCanvasSim ) )
    {
        a2dCanvasSim* can = wxStaticCast( m_drawingPart->GetDisplayWindow(), a2dCanvasSim );
        can->SetMappingShowAll( m_zoomoutCentered );
    }
#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
    else if ( wxDynamicCast( m_drawingPart->GetDisplayWindow(), a2dOglCanvas ) )
    {
        a2dOglCanvas* can = wxStaticCast( m_drawingPart->GetDisplayWindow(), a2dOglCanvas );
        can->SetMappingShowAll( m_zoomoutCentered );
    }
#endif
    else
        wxASSERT_MSG( 0, wxT( "not implemented for this type of canvas" ) );
    a2dGeneralGlobals->RecordF( this, _T( "zoomout" ) );
}

void a2dStToolContr::Zoomout2()
{
    int dx, dy;
    m_drawingPart->GetDisplayWindow()->GetClientSize( &dx, &dy );
    if ( dy > dx ) dx = dy;

    Zoom( CDRAWER2D->GetVisibleMinX() - CDRAWER2D->GetUppX()*dx / 2,
          CDRAWER2D->GetVisibleMinY() - CDRAWER2D->GetUppY()*dy / 2,
          CDRAWER2D->GetUppX() * 2, CDRAWER2D->GetUppY() * 2 );
}

void a2dStToolContr::Zoomin2()
{
    int dx, dy;
    m_drawingPart->GetDisplayWindow()->GetClientSize( &dx, &dy );
    if ( dy > dx ) dx = dy;

    Zoom( CDRAWER2D->GetVisibleMinX() + CDRAWER2D->GetUppX()*dx / 4,
          CDRAWER2D->GetVisibleMinY() + CDRAWER2D->GetUppY()*dy / 4,
          CDRAWER2D->GetUppX() * 0.5, CDRAWER2D->GetUppY() * 0.5 );
}

void a2dStToolContr::ZoomOut2AtMouse()
{
    int dx, dy;
    m_drawingPart->GetDisplayWindow()->GetClientSize( &dx, &dy );
    if ( dy > dx ) dx = dy;

    double mouse_worldx, mouse_worldy;
    GetDrawingPart()->MouseToToolWorld( m_mouse_x, m_mouse_y, mouse_worldx, mouse_worldy );
    mouse_worldx -= CDRAWER2D->GetUppX() * dx / 2;
    mouse_worldy -= CDRAWER2D->GetUppY() * dy / 2;

    Zoom( mouse_worldx, mouse_worldy, CDRAWER2D->GetUppX() * 2, CDRAWER2D->GetUppY() * 2 );
}

void a2dStToolContr::ZoomIn2AtMouse()
{
    int dx, dy;
    m_drawingPart->GetDisplayWindow()->GetClientSize( &dx, &dy );
    if ( dy > dx ) dx = dy;

    double mouse_worldx, mouse_worldy;
    GetDrawingPart()->MouseToToolWorld( m_mouse_x, m_mouse_y, mouse_worldx, mouse_worldy );
    mouse_worldx -= CDRAWER2D->GetUppX() * dx / 4;
    mouse_worldy -= CDRAWER2D->GetUppY() * dy / 4;

    Zoom( mouse_worldx, mouse_worldy, CDRAWER2D->GetUppX() * 0.5, CDRAWER2D->GetUppY() * 0.5 );
}

void a2dStToolContr::Zoom( double x, double y, double uppx, double uppy )
{
    CDRAWER2D->SetMappingUpp( x, y, uppx, uppy );
    a2dGeneralGlobals->RecordF( this, _T( "zoom %f %f %f %f upp" ),  x, y, uppx, uppy );
}

bool a2dStToolContr::PushTool( a2dBaseTool* tool )
{
	ClearStateStrings();
    wxASSERT_MSG( wxDynamicCast( tool, a2dStTool ) != 0, wxT( "Only a2dStTool can be pushed into a2dStToolContr" ) );

    bool res = a2dToolContr::PushTool( tool );
    SetStateString();
    return res;
}

bool a2dStToolContr::PopTool( a2dSmrtPtr<a2dBaseTool>& tool, bool abort )
{
	ClearStateStrings();
    a2dToolContr::PopTool( tool, abort );

    if ( !tool )
        return false;

    if ( GetFirstTool() ) //more tools stacked
    {
        //recursive pop tools until no more stopped tools.
        if ( GetFirstTool()->GetStopTool() )
            PopTool( tool );

        ActivateTop( true );
    }
    else
    {
        //always available
        if ( m_topTool )
        {
            PushTool( m_topTool );
            ActivateTop( true );
        }
        else
            m_drawingPart->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW ) );
    }
    SetStateString();
    return true;
}

void a2dStToolContr::OnComEvent( a2dComEvent& event )
{
    if ( GetEvtHandlerEnabled() && m_drawingPart )
    {
        if ( event.GetId() == a2dComEvent::sm_changedProperty )
        {
            a2dNamedProperty* property = event.GetProperty();

			/* todo
            if ( property->GetId() == a2dCentralCanvasCommandProcessor::PROPID_Snap )
            {
                SetSnap( wxStaticCast( property, a2dNamedProperty )->GetBool() );
            }
			*/
        }
        if ( event.GetId() == a2dDrawingPart::sig_changedShowObject )
        {
            if ( event.GetEventObject() == GetDrawingPart() )
            {
                // an existing corridor on a view will become invalid when the ShowObject changes.
                // Most tools can handle a change in corridor, for those there is no need to remove them after a change
                // in ShowObject.
                a2dCanvasObject* newtop = wxDynamicCast( event.GetProperty()->GetRefObject(), a2dCanvasObject );
                if ( m_topTool && newtop )
                {
                    a2dCorridor corridor( *GetDrawingPart() );
                    corridor.Push( newtop );
                    m_topTool->SetCorridor( corridor );
                    m_toolForZoom->SetCorridor( corridor );
                    event.Skip(); //base class and tools
                }
            }
            else
                event.Skip();
        }
        else
            event.Skip(); //base class
    }
    else
        event.Skip(); //base class
}


//!called on key events
void a2dStToolContr::OnChar( wxKeyEvent& event )
{

    /*
        switch(event.GetKeyCode())
        {
            case 'Z':
            case 'z':
            {
                a2dZoomTool* zoom = new a2dZoomTool(this);
                PushTool(zoom);
            }
            break;
            default:
                event.Skip();
        }
    */
    event.Skip();
}

void a2dStToolContr::OnKeyDown( wxKeyEvent& event )
{
    //wxLogDebug(wxT("key %d"), event.GetKeyCode());

    if( m_defaultBehavior & wxTC_NoDefaultKeys )
    {
        event.Skip();
        return;
    }

    switch( event.GetKeyCode() )
    {

        case WXK_CONTROL:
        {
            if  ( !GetFirstTool() || ( GetFirstTool() && !GetFirstTool()->GetBusy() ) )
            {
                a2dDragTool* c = wxDynamicCast( GetFirstTool(), a2dDragTool );
                if ( !c )
                {
                    a2dDragTool* drag = new a2dDragTool( this );
                    PushTool( drag );
                    drag->SetOneShot();
                }
            }
        }
        break;

        /* shift keys already used in tools.
                case WXK_SHIFT:
                {
                    if  ( !GetFirstTool() || (GetFirstTool() && !GetFirstTool()->GetBusy()) )
                    {
                        a2dSelectTool *c = wxDynamicCast(GetFirstTool(), a2dSelectTool);
                        if (!c)
                        {
                            a2dSelectTool* select = new a2dSelectTool(this);
                            PushTool(select);
                            select->SetOneShot();
                        }
                    }
                }
                break;
        */
        case WXK_ESCAPE:
        {
            if ( GetFirstTool() && GetFirstTool()->AllowPop() )
            {
                a2dSmrtPtr<a2dBaseTool> poped;
                PopTool( poped );
            }

            SetStateString();

        }
        break;
        case 'Z':
        case 'z':
        {
            if ( event.m_controlDown && event.m_shiftDown )
                m_drawingPart->GetDrawing()->GetCommandProcessor()->Redo();
            else if ( event.m_controlDown )
                m_drawingPart->GetDrawing()->GetCommandProcessor()->Undo();
            else if  ( !GetFirstTool() || ( GetFirstTool() && !GetFirstTool()->GetBusy() ) )
            {
                a2dBaseTool* tool = m_toolForZoom->TClone( clone_deep );
                PushTool( tool );
            }
            else
                event.Skip();
            break;
        }
        case 'Y':
        case 'y':
        {
            if ( event.m_controlDown )
                m_drawingPart->GetDrawing()->GetCommandProcessor()->Redo();
            else
                event.Skip();
            break;
        }
        case 'r':
        case 'R':
        {
            if  ( !GetFirstTool() || ( GetFirstTool() && !GetFirstTool()->GetBusy() ) )
                m_drawingPart->GetDrawing()->GetHabitat()->GetConnectionGenerator()->RotateRouteMethod();
            else
                event.Skip();
            break;
        }   
        case 't':
        case 'T':
        {
            if  ( !GetFirstTool() || ( GetFirstTool() && !GetFirstTool()->GetBusy() ) )
                m_drawingPart->GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetRouteWhenDrag( !m_drawingPart->GetDrawing()->GetHabitat()->GetConnectionGenerator()->GetRouteWhenDrag() );
            else
                event.Skip();
            break;
        }   
        case WXK_NUMPAD_ENTER:
        {
            Zoomout();
            break;
        }
        case WXK_NUMPAD_ADD:
        {
            if ( event.m_controlDown )
                ZoomIn2AtMouse();
            else
                Zoomin2();
            break;
        }
        case WXK_NUMPAD_SUBTRACT:
        {
            if ( event.m_controlDown )
                ZoomOut2AtMouse();
            else
                Zoomout2();
            break;
        }
        default:
		{
		    a2dRestrictionEngine* restrictEngine = m_drawingPart->GetDrawing()->GetHabitat()->GetRestrictionEngine();
			if ( event.GetKeyCode() == m_noSnapkey )
			{
				if ( restrictEngine )
					restrictEngine->SetReleaseSnap( true );
				break;
			}
			else
				event.Skip();
		}
    }
}

void a2dStToolContr::OnKeyUp( wxKeyEvent& event )
{
    if( m_defaultBehavior & wxTC_NoDefaultKeys )
    {
        event.Skip();
        return;
    }

    if  ( GetFirstTool() && !GetFirstTool()->GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_F1:
                break;


            case WXK_CONTROL:
            {
                a2dDragTool* c = wxDynamicCast( GetFirstTool(), a2dDragTool );
                if ( c )
                {
                    a2dSmrtPtr<a2dBaseTool> poped;
                    PopTool( poped );
                }
                break;
            }
            /*
                        case WXK_SHIFT:
                        {
                            a2dSelectTool *c = wxDynamicCast(GetFirstTool(), a2dSelectTool);
                            if (c)
                            {
                                a2dSmrtPtr<a2dBaseTool> poped;
                                PopTool( poped );
                            }
                            break;
                        }
            */
            default:
			{
		        a2dRestrictionEngine* restrictEngine = m_drawingPart->GetDrawing()->GetHabitat()->GetRestrictionEngine();
				if ( event.GetKeyCode() == m_noSnapkey )
				{
					if ( restrictEngine )
					    restrictEngine->SetReleaseSnap( false );
					break;
				}
				else
					event.Skip();
			}
        }
    }
    else
        event.Skip();

}

void a2dStToolContr::ClearStateStrings()
{
	std::vector<wxString>::iterator it;
	it = m_statusStrings.end();
	for ( it= m_statusStrings.begin(); it != m_statusStrings.end(); ++it)
	{
		*it = "";
	}

}

void a2dStToolContr::SetStateString( const wxString& message, size_t field )
{
    m_statusStrings[ field ] = message;

    //first element always show cursor info.
    {
        wxString str;
        str.Printf( _T( "%d : %d" ), m_mouse_x, m_mouse_y );
        m_statusStrings[ STAT_Xd_Yd ] = str;
    }
    {
        double mouse_worldx, mouse_worldy;
        GetDrawingPart()->MouseToToolWorld( m_mouse_x, m_mouse_y, mouse_worldx, mouse_worldy );
        if ( GetDrawingPart()->GetDrawing() )
        {
            mouse_worldx = mouse_worldx * GetDrawingPart()->GetDrawing()->GetUnitsScale();
            mouse_worldy = mouse_worldy * GetDrawingPart()->GetDrawing()->GetUnitsScale();
        }
		wxString form = GetFormat() + " : " + GetFormat();
        wxString str;
        str.Printf( form, mouse_worldx, mouse_worldy );
        m_statusStrings[ STAT_Xw_Yw ] = str;
    }
    {
        wxString str;
        if  ( GetFirstTool() )
		{
            str = GetFirstTool()->GetToolString();
		}
        m_statusStrings[ STAT_ToolString ] = str;
    }
    if ( m_where )
    {
        if ( m_noStatusEvent )
        {
            if ( m_where->GetStatusBar() )
            {
                wxString str;
                str = m_statusStrings[ STAT_Xd_Yd ];
                str = str + _T( " : " ) + m_statusStrings[ STAT_Xw_Yw ];
                str = str + _T( " : " ) + m_statusStrings[ STAT_ToolString ];
                m_where->SetStatusText( str ); 
            }
        }
        else
        {
            a2dComEvent event( this, sm_showCursor );
            if  ( GetFirstTool() )
            {
                if ( ! GetFirstTool()->ProcessEvent( event ) )
                    m_where->GetEventHandler()->ProcessEvent( event );
            }
        }
    }
}

void a2dStToolContr::OnMouseEvent( wxMouseEvent& event )
{
    wxPoint pos = event.GetPosition();

    m_mouse_x = pos.x;
    m_mouse_y = pos.y;

    SetStateString( "", 0 );

    if  ( m_draggingCanvasOption )
    {
        if ( event.LeftDown() && !m_drawingPart->GetDisplayWindow()->HasFocus() )
        {
            m_drawingPart->GetDisplayWindow()->SetFocus();
        }

        if ( event.Dragging() && m_draggingCanvas )
        {
            double dxworld = m_drawingPart->GetDrawer2D()->DeviceToWorldXRel( floor( m_mouse_xprev - m_mouse_x + 0.5 ) );
            double dyworld = m_drawingPart->GetDrawer2D()->DeviceToWorldYRel( floor( m_mouse_yprev - m_mouse_y + 0.5 ) );

    //wxLogDebug( wxT( "cx =%d, cy =%d" ), m_mouse_x, m_mouse_y );
    //wxLogDebug( wxT( "cx =%12.6lf, cy =%12.6lf" ), dxworld, dyworld );

            wxWindow* win = m_drawingPart->GetDisplayWindow();
            if ( wxDynamicCast( win, a2dCanvas ) )
            {
                a2dCanvas* can = wxStaticCast( win, a2dCanvas );
                can->ScrollWindowConstrained( dxworld, dyworld );
            }
            else if ( wxDynamicCast( win, a2dCanvasSim ) )
            {
                a2dCanvasSim* can = wxStaticCast( win, a2dCanvasSim );
            }
#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
            else if ( wxDynamicCast( win, a2dOglCanvas ) )
            {
                a2dOglCanvas* can = wxStaticCast( win, a2dOglCanvas );
                can->ScrollWindowConstrained( dxworld, dyworld );
            }
#endif //wxUSE_GLCANVAS 

            m_mouse_xprev = m_mouse_x;
            m_mouse_yprev = m_mouse_y;
        }
        else if( event.RightDown() && !event.LeftIsDown () && !m_draggingCanvas )
        {
            // to prevent capturing mouse twice when already dragging in lower level stacked tools, we test also on LeftIsDown
            // Is so, this dragging can not start. Understand also that a lower tool can be busy while this canvas drag is done.
            // Only busy and in drag mode (leftisdown), will prevent this drag from happening. 
            m_drawingPart->PushCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Move ) );
            m_draggingCanvas = true;
			GetDrawingPart()->GetDisplayWindow()->CaptureMouse();
            m_dragstartx = m_mouse_x;
            m_dragstarty = m_mouse_y;
            m_mouse_xprev = m_mouse_x;
            m_mouse_yprev = m_mouse_y;
        }
        else if( event.RightUp() && m_draggingCanvas )
        {
		    GetDrawingPart()->GetDisplayWindow()->ReleaseMouse();
            m_draggingCanvas = false;
            if ( m_dragstartx == m_mouse_xprev && m_dragstarty == m_mouse_yprev )
            {
                //try tool first
                if ( ToolsProcessEvent( event ) )
                    return;
                else if ( ! ( m_defaultBehavior & wxTC_NoContextMenu ) && m_mousemenu )
                    m_drawingPart->GetDisplayWindow()->PopupMenu( GetMousePopupMenu(), m_mouse_x, m_mouse_y );
            }
            else
                m_drawingPart->PopCursor();
        }
        else
            //let the baseclass a2dToolContr
            //handle the event, in order to redirect it to the tools
            event.Skip();
    }
    else
    {
        if ( event.LeftDown() && wxWindow::FindFocus() != m_drawingPart->GetDisplayWindow() )
        {
            // normally handler in a2dDrawingPart::OnActive()
            if ( !m_drawingPart->GetDisplayWindow()->HasFocus() )
                m_drawingPart->GetDisplayWindow()->SetFocus();
        }
        if ( event.RightDown() && !GetDrawingPart()->GetDisplayWindow()->GetCapture() )
        {
            //we first try the tools here, since it can have a popup menu too.
            //If not processed, and not processed in here either, then it will once more
            //be processed by the tools. This fact is not a problem.
            if ( ToolsProcessEvent( event ) )
                return;

            if ( ! ( m_defaultBehavior & wxTC_NoContextMenu ) && m_mousemenu )
            {
                m_drawingPart->GetDisplayWindow()->PopupMenu( m_mousemenu, pos.x, pos.y );
                event.Skip( false ); //event is processed.
                return;
            }
        }
        else if  ( GetFirstTool() && !GetFirstTool()->GetBusy()
                   && ! ( m_defaultBehavior & wxTC_NoDefaultMouseActions ) )
        {
            if ( !wxDynamicCast( GetFirstTool(), a2dZoomTool ) && event.m_controlDown && event.m_shiftDown )
            {
                a2dBaseTool* tool = m_toolForZoom->TClone( clone_deep );
                tool->SetOneShot();
                PushTool( tool );
            }
            if ( !wxDynamicCast( GetFirstTool(), a2dDragTool ) && event.m_controlDown )
            {
                a2dDragTool* drag = new a2dDragTool( this );
                PushTool( drag );
                drag->SetOneShot();
            }
            /*
                    else if ( !wxDynamicCast(GetFirstTool(), a2dSelectTool) && event.m_shiftDown)
                    {
                        a2dSelectTool* select= new a2dSelectTool(this);
                        PushTool(select);
                        select->SetOneShot();
                    }
            */
        }
        else
            //let the baseclass a2dToolContr
            //handle the event, in order to redirect it to the tools
            event.Skip();
    }
}

bool a2dStToolContr::StartEditingObject( a2dCanvasObject* objectToEdit )
{
    a2dObjectEditTool* edit = new a2dObjectEditTool( this );
    PushTool( edit );
    edit->StartToEdit( objectToEdit );
    return true;
}

bool a2dStToolContr::StartEditingObject( a2dCanvasObject* objectToEdit, a2dIterC& ic )
{
    a2dObjectEditTool* edit = new a2dObjectEditTool( this, ic );
    PushTool( edit );
    edit->StartToEdit( objectToEdit );
    return true;
}

bool a2dStToolContr::TriggerReStartEdit( wxUint16 editmode )
{
    a2dObjectEditTool* edit = wxDynamicCast( GetFirstTool(), a2dObjectEditTool );
    if ( edit )
    {
        edit->TriggerReStartEdit( editmode );
        return true;
    }
    return false;
}

bool a2dStToolContr::RotateObject90LeftRight( bool right )
{
    if ( GetFirstTool() )
    {
        return GetFirstTool()->RotateObject90LeftRight( right );
    }
    return false;
}


/********************************************************************************
a2dStTool
********************************************************************************/

const a2dSignal a2dStTool::sig_toolComEventAddObject = wxNewId(); 
const a2dSignal a2dStTool::sig_toolComEventSetEditObject = wxNewId(); 

BEGIN_EVENT_TABLE( a2dStTool, a2dBaseTool )
    EVT_KEY_DOWN( a2dStTool::OnKeyDown )
    EVT_KEY_UP( a2dStTool::OnKeyUp )
    EVT_CHAR( a2dStTool::OnChar )
    EVT_MOUSE_EVENTS( a2dStTool::OnMouseEvent )
    EVT_COM_EVENT( a2dStTool::OnComEvent )
    EVT_PAINT( a2dStTool::OnPaint )
    EVT_IDLE( a2dStTool::OnIdle )
    EVT_DO( a2dStTool::OnDoEvent )
    EVT_UNDO( a2dStTool::OnUndoEvent )
END_EVENT_TABLE()

a2dStTool::a2dStTool( a2dStToolContr* controller ): a2dBaseTool( ( a2dToolContr* )controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandler();

    m_useEditOpaque = a2dOpaqueMode_Controller;
    m_editOpacityFactor = DEFAULTOpacityFactor;

    m_editatend = false;

    m_anotate = false;

    m_canvasobject = 0;
    m_original = 0;

    m_stcontroller = controller;
    m_connectionGenerator = GetDrawing()->GetHabitat()->GetConnectionGenerator();
    m_xanotation = m_yanotation = 0;
    m_commandgroup = 0;

    m_dragStarted = false;
    m_xprev = 0;
    m_yprev = 0;
    m_dragstartx = 0;
    m_dragstarty = 0;
    m_xwprev = 0.0;
    m_ywprev = 0.0;

    m_snapSourceFeatures = a2dRestrictionEngine::snapToNone;

    m_snapTargetFeatures = a2dRestrictionEngine::snapToNone;


    // WARNING: This will call the base class version for basic initialization
    // The derived class version is called e.g. at the beginning of rendering.
    // If it is not sifficient, AdjustRenderOptions must be called in the derived
    // class constructor.
    AdjustRenderOptions();
}

a2dStTool::a2dStTool( const a2dStTool& other, CloneOptions options, a2dRefMap* refs )
    : a2dBaseTool( other, options, refs )
{
    m_eventHandler = other.m_eventHandler;

    m_useEditOpaque = other.m_useEditOpaque;
    m_editOpacityFactor = other.m_editOpacityFactor;

    m_editatend = other.m_editatend;

    m_anotate = other.m_anotate;

    m_canvasobject = 0;
    m_original = 0;

    m_stcontroller = other.m_stcontroller;
    m_connectionGenerator = other.m_connectionGenerator;
    m_xanotation = other.m_xanotation;
    m_yanotation = other.m_yanotation;
    m_commandgroup = other.m_commandgroup;
}

a2dStTool::~a2dStTool()
{
}

void a2dStTool::SetStateString( const wxString& message, size_t field )
{
    m_stcontroller->SetStateString(  message, field );
}

bool a2dStTool::SetContext( a2dIterC& ic, a2dCanvasObject* startObject )
{
    m_corridor = a2dCorridor( ic );
    m_parentobject = m_corridor.back();

    m_canvasobject = startObject;
    return true;
}

wxRect a2dStTool::GetAbsoluteArea( a2dCanvasObject* object )
{
    a2dIterC ic( GetDrawingPart() );
    a2dIterCU cu( ic );

    return object->GetAbsoluteArea( ic );
}

void a2dStTool::GetTextExtent( const wxString& string, wxCoord* w, wxCoord* h, wxCoord* descent, wxCoord* externalLeading )
{
    wxMemoryDC dc;
    dc.SetFont( m_annotateFont );
    dc.GetTextExtent( string, w, h, descent, externalLeading );
}

bool a2dStTool::OnPushNewTool( a2dBaseTool* WXUNUSED( newtool ) )
{
    // If tool is busy, it will enter halted state
    if ( GetBusy() && !m_halted )
    {
        m_halted = true;
    }
    m_pending = true;
    return false; //do not pop this tool before pushing new tool.
}

void a2dStTool::AddCurrent( a2dCanvasObject* objectToAdd )
{
    // Set the style of the documents command-processor to the style of this tool
    if ( GetDrawingPart()->GetDrawing() )
    {
        a2dComEvent event( this, objectToAdd, sig_toolComEventAddObject );
        ProcessEvent( event );
    }

    GetCanvasCommandProcessor()->AddCurrent( objectToAdd, true, NULL, m_parentobject );
}

void a2dStTool::SetActive( bool active )
{
    a2dBaseTool::SetActive( active );
    m_pending = true;

    if ( active )
    {
        a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
        if( restrict )
        {
            restrict->SetParentSnapObjects( m_parentobject );
        }

        // Tool is activated

        // Set tool cursor
        if ( GetBusy() && GetDrawingPart()->GetDrawing() )
        {
            GetDrawingPart()->SetCursor( m_toolBusyCursor );
        }
        else
        {
            GetDrawingPart()->SetCursor( m_toolcursor );
        }

        // Is tool reactivated ?
        if ( m_halted )
        {
            m_halted = false;
            // The command group is closed in a2dBaseTool::SetActive, but
            // not reopended, because this does only make sense for stacked tools
            OpenCommandGroup( true );
        }
        m_pending = true;
    }
    else
    {
        // Tool is deactivated

        //the subedit or other subtools should not influence style of the object.
        //When this tool is activated again, the CurrentCanvasObject is set right again.
	    if ( GetDrawingPart() && GetDrawingPart()->GetShowObject() )
			GetCanvasCommandProcessor()->SetCurrentCanvasObject( 0 );

        m_pending = true;
    }
}

bool a2dStTool::EnterBusyMode()
{
    if( !a2dBaseTool::EnterBusyMode() )
        return false;

    if( !CreateToolObjects() )
    {
        a2dBaseTool::AbortBusyMode();
        return false;
    }

    return true;
}

void a2dStTool::FinishBusyMode( bool closeCommandGroup )
{
    if( GetBusy() )
    {
        CleanupToolObjects();

        // Even tools deleting m_original want it to become visible for undo commands
        if( m_original )
        {
            m_original->SetVisible( true );
            m_original->SetPending( true );
            m_original->SetSnapTo( true );
            m_original = 0;
        }
        m_canvasobject = 0;
	    m_stcontroller->ClearStateStrings();

        a2dBaseTool::FinishBusyMode( closeCommandGroup );
    }
}

void a2dStTool::AbortBusyMode()
{
    CleanupToolObjects();

    // Even tools deleting m_original want it to become visible for undo commands
    if( m_original )
    {
        m_original->SetVisible( true );
        m_original->SetPending( true );
        m_original = 0;
    }
    m_canvasobject = 0;
	m_stcontroller->ClearStateStrings();

    a2dBaseTool::AbortBusyMode();
}

void a2dStTool::DeselectAll()
{
	GetCanvasCommandProcessor()->DeselectAll();

    // !!!! should deslect all, but no idea how to do this properly
    // a2dGeneralGlobals->RecordF( this, _T( "deselect %f %f %f %f" ), -1e10, -1e10, 1e10, 1e10 );
}

void a2dStTool::OnMouseEvent( wxMouseEvent& event )
{
    wxPoint pos = event.GetPosition();
    if ( event.RightUp() && m_mousemenu )
    {
        GetDrawingPart()->GetDisplayWindow()->PopupMenu( m_mousemenu, pos.x, pos.y );
    }
    else
        event.Skip();
}

void a2dStTool::AdjustRenderOptions()
{
    switch( m_stcontroller->GetDragMode() )
    {
        case wxDRAW_RECTANGLE:
            m_renderOriginal = false;
            m_renderEditcopy = false;
            m_renderEditcopyOnTop = false;
            m_renderEditcopyEdit = false;
            m_renderEditcopyRectangle = true;
            break;

        case wxDRAW_ONTOP:
            m_renderOriginal = false;
            m_renderEditcopy = false;
            m_renderEditcopyOnTop = true;
            m_renderEditcopyEdit = false;
            m_renderEditcopyRectangle = false;
            break;

        case wxDRAW_REDRAW:
            m_renderOriginal = true;
            m_renderEditcopy = false;
            m_renderEditcopyOnTop = false;
            m_renderEditcopyEdit = false;
            m_renderEditcopyRectangle = false;
            break;

        case wxDRAW_COPY:
            m_renderOriginal = true;
            m_renderEditcopy = false;
            m_renderEditcopyOnTop = false;
            m_renderEditcopyEdit = true;
            m_renderEditcopyRectangle = false;
            break;

        default:
            wxASSERT( 0 );
            m_renderOriginal = true;
            m_renderEditcopy = false;
            m_renderEditcopyOnTop = false;
            m_renderEditcopyEdit = true;
            m_renderEditcopyRectangle = false;
    }
}

bool a2dStTool::CreateToolObjects()
{
    return true;
}

void a2dStTool::CleanupToolObjects()
{

    for( a2dCanvasObjectList::iterator iter = m_parentobject->GetChildObjectList()->begin(); iter != m_parentobject->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObjectList::value_type obj = *iter;

        bool release = false;
        //if ( obj->GetAlgoSkip() )
        {        
            void* tool = a2dCanvasObject::PROPID_ToolObject->GetPropertyValue( obj );
            if( tool == this )
            {
                a2dCanvasObject* original = a2dCanvasObject::PROPID_Original->GetPropertyValue( obj );
                if( original && obj->GetEditingRender() )
                    obj->EndEdit();
                release = true;
            }
            else if ( a2dCanvasObject::PROPID_ToolDecoration->GetPropertyValue( obj ) )
            {
                release = true;
            }
        }

        if( release )
        {
            a2dWalker_RemovePropertyCandoc setp( a2dCanvasObject::PROPID_Original );
            setp.AddPropertyId( a2dCanvasObject::PROPID_Editcopy );
            setp.SetSkipNotRenderedInDrawing( true );
            setp.Start( obj );

            // Creative tools might reuse this editcopy as real object, so clean it up a bit
            obj->RemoveProperty( a2dCanvasObject::PROPID_ToolObject );
            obj->RemoveProperty( a2dCanvasObject::PROPID_ToolDecoration );
            obj->RemoveProperty( a2dCanvasObject::PROPID_ViewSpecific );
            //we do delayed release, to make subediting possible ( meaning editcopy is used
            //to create extra tools, so it may not be deleted yet )
            obj->SetRelease( true );
            obj->SetPending( true );
            //wrong to erase directly;
        }
    }
    RemoveDecorationObject( NULL );

    // Possibly was set invisible in CreateToolObjects
    if( m_original )
    {
        m_original->SetVisible( true );
    }
}

void a2dStTool::GetKeyIncrement( double* xIncr, double* yIncr )
{
    double pixel = GetDrawer2D()->DeviceToWorldXRel( 1 );


    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        if( restrict->GetSnapGridX() )
            *xIncr = restrict->GetSnapGridX() / GetDrawingPart()->GetDrawing()->GetUnitsScale();
        else
            *xIncr = pixel;

        if( restrict->GetSnapGridY() )
            *yIncr = restrict->GetSnapGridY() / GetDrawingPart()->GetDrawing()->GetUnitsScale();
        else
            *yIncr = pixel;
    }
    else
    {
        *xIncr = pixel;
        *yIncr = pixel;
    }

    if ( !GetDrawer2D()->GetYaxis() )
    {
        *yIncr = -*yIncr;
    }
}

void a2dStTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStTool::AddAnotationToPendingUpdateArea();

    m_anotation = _T( "" );
    m_xanotation = m_x;
    m_yanotation = m_y;

    a2dStTool::AddAnotationToPendingUpdateArea();
}

void a2dStTool::AddAnotationToPendingUpdateArea()
{
    if ( m_anotate )
    {
        wxCoord w, h, descent, external;
        GetTextExtent( m_anotation, &w, &h, &descent, &external );

        wxRect text( m_xanotation, m_yanotation, w, h );
        GetDrawingPart()->AddPendingUpdateArea( text.Inflate( 2, 2 ) );
    }
}

void a2dStTool::OnIdle( wxIdleEvent& event )
{
    if ( m_canvasobject && m_pending && GetBusy() )
    {
        m_pending = false;
        GetDrawingPart()->AddPendingUpdateArea( m_canvasobject, 1 );
        m_canvasobject->Update( a2dCanvasObject::updatemask_normal );
        GetDrawingPart()->AddPendingUpdateArea( m_canvasobject, 1 );

        // Add anotation area to the pending areas
        // old and new annotation area are calculated and added as pendingarea.
        GenerateAnotation();
    }
    else
        event.Skip();
}

void a2dStTool::OnPaint( wxPaintEvent& event )
{
    if ( GetBusy() )
    {
        //CYCLIC paint event result because of a2dStTool::OnIdle() regenerating not paint events somehow
        //m_pending = true;
    }
    event.Skip();
}

// This is usefull for special breakpoints in debug mode
#ifdef _DEBUG
bool renderingtool = false;
#endif

void a2dStTool::Render()
{
#ifdef _DEBUG
    renderingtool = true;
#endif

    if ( m_canvasobject && GetBusy() )
    {
        AdjustRenderOptions();

        if ( m_original )
        {
            a2dCanvasObject::SetIgnoreAllSetpending( true );
            m_original->SetVisible( m_renderOriginal );
            a2dCanvasObject::SetIgnoreAllSetpending( false );
        }

        // Render edit objects in On-Top mode
        if( m_renderEditcopyOnTop )
        {
            GetDrawingPart()->RenderTopObject( RenderTOOL_OBJECTS_STYLED, 1 );
        }

        // Render edit objects in Edit mode
        if( m_renderEditcopyEdit )
        {
            if ( m_useEditOpaque == a2dOpaqueMode_Tool )
            {
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( m_editOpacityFactor );
                GetDrawingPart()->RenderTopObject( RenderTOOL_OBJECTS_STYLED, 1 );
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( 255 );
            }
            else if ( m_useEditOpaque == a2dOpaqueMode_Tool_FixedStyle )
            {
                a2dFill fill = *a2dTRANSPARENT_FILL;
                a2dStroke stroke = GetDrawing()->GetHabitat()->GetHandle()->GetStroke();
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( m_editOpacityFactor );
                GetDrawingPart()->SetFixedStyleFill( fill );
                GetDrawingPart()->SetFixedStyleStroke( stroke );
                GetDrawingPart()->RenderTopObject( RenderTOOL_OBJECTS, 1 );
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( 255 );
            }
            else if (  m_useEditOpaque == a2dOpaqueMode_Controller && m_stcontroller->GetUseOpaqueEditcopy() )
            {
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( m_stcontroller->GetOpacityFactorEditcopy() );
                GetDrawingPart()->RenderTopObject( RenderTOOL_OBJECTS_STYLED, 1 );
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( 255 );
            }
            else //if ( m_useEditOpaque == a2dOpaqueMode_Off )
            {
                a2dFill fill = *a2dTRANSPARENT_FILL;
                a2dStroke stroke = GetDrawing()->GetHabitat()->GetHandle()->GetStroke();

                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( 255 );
                GetDrawingPart()->SetFixedStyleFill( fill );
                GetDrawingPart()->SetFixedStyleStroke( stroke );
                GetDrawingPart()->RenderTopObject( RenderTOOL_OBJECTS, 1 );
            }
        }

        // Render edit objects in Edit mode as rectangles
        if( m_renderEditcopyRectangle )
        {
            GetDrawingPart()->SetFixedStyleFill( *a2dTRANSPARENT_FILL );
            GetDrawingPart()->SetFixedStyleStroke( GetDrawing()->GetHabitat()->GetHandle()->GetStroke() );
            GetDrawingPart()->RenderTopObject( RenderRectangleTOOL_OBJECTS, 1 );
        }

        GetDrawingPart()->RenderTopObject( RenderTOOL_DECORATIONS, 1 );

        RenderAnotation();
    }

#ifdef _DEBUG
    renderingtool = false;
#endif
}

void a2dStTool::RenderAnotation()
{
    if ( m_anotate && m_canvasobject && GetBusy() )
    {
        GetDrawer2D()->DeviceDrawAnnotation( m_anotation, m_xanotation, m_yanotation, m_annotateFont );
    }
}

void a2dStTool::OnUndoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}

void a2dStTool::OnDoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}

void a2dStTool::OnComEvent( a2dComEvent& event )
{
    if ( GetEvtHandlerEnabled() )
    {
        if ( event.GetId() == a2dBaseTool::sig_toolPoped )
        {
            // if edit tool ended, we still need to close the command group
            if ( !GetBusy() && m_editatend )
                CloseCommandGroup();
        }
    }

    event.Skip();
}

void a2dStTool::OnKeyDown( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        a2dRestrictionEngine* restrictEngine = GetDrawing()->GetHabitat()->GetRestrictionEngine();
		if ( event.GetKeyCode() == WXK_SHIFT )
        {
            if ( restrictEngine )
                restrictEngine->SetShiftKeyDown( true );
        }
		else
            event.Skip();
    }
    else
    {
        event.Skip();
        /* interfears with DrawTextTool when typing t, it is eaten here.
        switch( event.GetKeyCode() )
        {
            case 'r':
            case 'R':
            {
                GetDrawing()->GetHabitat()->GetConnectionGenerator()->RotateRouteMethod();
                break;
            }   
            case 't':
            case 'T':
            {
                GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetRouteWhenDrag( !GetDrawing()->GetHabitat()->GetConnectionGenerator()->GetRouteWhenDrag() );
                break;
            }   
            default:
                event.Skip();
        }
        */
    }
}

void a2dStTool::OnKeyUp( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        a2dRestrictionEngine* restrictEngine = GetDrawing()->GetHabitat()->GetRestrictionEngine();
		if ( event.GetKeyCode() == WXK_SHIFT )
        {
            if ( restrictEngine )
                restrictEngine->SetShiftKeyDown( false );
        }
		else
            event.Skip();
    }
	else
		event.Skip();
}


void a2dStTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() && m_canvasobject && m_canvasobject->IsDraggable() )
    {
        double dx = m_xwprev - m_canvasobject->GetPosX();
        double dy = m_ywprev - m_canvasobject->GetPosY();

        m_xwprev = m_canvasobject->GetPosX();
        m_ywprev = m_canvasobject->GetPosY();

        double shiftx;
        double shifty;
        GetKeyIncrement( &shiftx, &shifty );

        //now shift with snap distances in X or Y

        switch( event.GetKeyCode() )
        {
            case WXK_UP:
                shiftx = 0;
                break;

            case WXK_DOWN:
                shiftx = 0;
                shifty = -shifty;
                break;

            case WXK_LEFT:
                shiftx = -shiftx;
                shifty = 0;
                break;

            case WXK_RIGHT:
                shifty = 0;
                break;
            default:
            {
                shiftx = 0;
                shifty = 0;
                event.Skip();
            }
        }

        m_xwprev += shiftx;
        m_ywprev += shifty;

        m_canvasobject->Translate( shiftx, shifty );

        m_pending = true;
    }
    else
        event.Skip();
}

void a2dStTool::MouseToToolWorld( int x, int y, double& xWorldLocal, double& yWorldLocal )
{
    GetDrawingPart()->MouseToToolWorld( x, y, xWorldLocal, yWorldLocal );
    a2dAffineMatrix cworld;
    cworld = GetDrawingPart()->GetShowObject()->GetTransform();
    cworld.Invert();
    cworld.TransformPoint( xWorldLocal, yWorldLocal, xWorldLocal, yWorldLocal );
    m_corridor.GetTransform().TransformPoint( xWorldLocal, yWorldLocal, xWorldLocal, yWorldLocal );
}

a2dObjectEditTool* a2dStTool::StartEditTool( a2dCanvasObject* objecttoedit )
{
    //TODO maybe better add a seperate function to reset style and current object in  command processor
    GetCanvasCommandProcessor()->SetCurrentCanvasObject( 0 );

    if ( m_editatend )
    {
        a2dStTool::FinishBusyMode( false );

        //make sure all updates have bin processed first
        //GetDrawingPart()->Update( a2dCANVIEW_UPDATE_SYNC_DRAWERS );

        a2dObjectEditTool* edit = new a2dObjectEditTool( m_stcontroller );
        //take over event handler e.g. to have fixed style
        edit->SetEvtHandler( GetEventHandler() );
        edit->SetFill( m_fill );
        edit->SetStroke( m_stroke );
        m_stcontroller->PushTool( edit );
        objecttoedit->SetVisible( true );
        edit->StartToEdit( objecttoedit );
        return edit;
    }
    else
    {
         a2dStTool::FinishBusyMode( true );
        // add update area for redraw of last state of the tool
        // GetDrawingPart()->AddPendingUpdateArea( m_new );
        GetDrawingPart()->Update( a2dCANVIEW_UPDATE_SYNC_DRAWERS );
        return NULL;
    }
}

a2dCanvasObject* a2dStTool::FindTaggedObject()
{
    a2dCanvasObject* ret = NULL;
    for( a2dCanvasObjectList::iterator iter = m_parentobject->GetChildObjectList()->begin(); iter != m_parentobject->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        a2dTagVecProperty* tagsp = a2dCanvasObject::PROPID_Tags->GetPropertyListOnly( obj );
        if ( tagsp )
        {
            a2dTag tag = tagsp->Last();
            ret = obj;
        }
    }
    return ret;
}

void a2dStTool::PrepareForRewire( a2dCanvasObjectList& dragList, bool walkWires, bool selected, bool stopAtSelectedWire, bool CreateExtraWires, a2dRefMap* refs )
{
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetNoEditCopy( false );
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->PrepareForRewire( m_parentobject, dragList, walkWires, selected, stopAtSelectedWire, CreateExtraWires, this, refs );
}


/********************************************************************************
a2dStDrawTool
********************************************************************************/


BEGIN_EVENT_TABLE( a2dStDrawTool, a2dStTool )
    EVT_KEY_DOWN( a2dStDrawTool::OnKeyDown )
    EVT_KEY_UP( a2dStDrawTool::OnKeyUp )
    EVT_CHAR( a2dStDrawTool::OnChar )
    EVT_COM_EVENT( a2dStDrawTool::OnComEvent )
    EVT_CANUNDO( a2dStDrawTool::OnCanUndoEvent )
    EVT_CANREDO( a2dStDrawTool::OnCanRedoEvent )
END_EVENT_TABLE()

a2dStDrawTool::a2dStDrawTool( a2dStToolContr* controller ): a2dStTool( controller )
{
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );

    // the base tool sets layer style using a2dBLACK_STROKE->Clone()->SetNoStroke() etc.
    // This for debug.
    m_stroke = a2dStroke( *wxBLACK, 0 );
    m_fill = a2dFill( *wxRED );
    m_AllowModifyTemplate = true;
}

a2dStDrawTool::~a2dStDrawTool()
{
    a2dCanvasGlobals->DisconnectEvent( a2dEVT_COM_EVENT, this );
}

void a2dStDrawTool::OnComEvent( a2dComEvent& event )
{
    if ( m_AllowModifyTemplate )
    {
        if ( event.GetId() == a2dHabitat::sig_SetContourWidth )
        {
            m_contourwidth = GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale();
            if ( GetBusy() )
            {
                m_canvasobject->SetContourWidth( m_contourwidth );
                m_original->SetContourWidth( m_contourwidth );
            }
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

bool a2dStDrawTool::CreateToolObjects()
{
    a2dStTool::CreateToolObjects();

    // Clone the original drag object
    m_canvasobject = m_original->TClone( clone_members | clone_childs | clone_seteditcopy | clone_setoriginal );
    // we don't want to snap points to the drawing objects itself
    m_original->SetSnapTo( false );

    m_canvasobject->SetSelected( false );

    AddEditobject( m_canvasobject );

    AdjustRenderOptions();
    m_original->SetVisible( m_renderOriginal );

    m_pending = true;
    return true;
}

void a2dStDrawTool::OnCanUndoEvent( a2dCommandProcessorEvent& event )
{
    if ( GetBusy() &&  !m_halted)
    {
        event.Veto( true );
    }
}

void a2dStDrawTool::OnCanRedoEvent( a2dCommandProcessorEvent& event )
{
    if ( GetBusy() &&  !m_halted)
    {
        event.Veto( true );
    }
}

bool a2dStDrawTool::EnterBusyMode()
{
    m_pending = true;
    return a2dStTool::EnterBusyMode();
}

void a2dStDrawTool::StartEditingOrFinish()
{
    if ( m_editatend && GetDrawingPart() )
    {
        a2dREFOBJECTPTR_KEEPALIVE;
        m_pending = true;
        a2dCanvasObjectPtr original = m_original;
        if ( m_stcontroller->GetSelectAtEnd() )
        {
            DeselectAll();
            original->SetSelected( true );
            original->SetSelected2( true );
        }
        if ( m_stcontroller->GetSelectionStateUndo() ) 
        {
	        GetCanvasCommandProcessor()->Submit( new a2dCommandMasked( GetDrawingPart()->GetShowObject(), GetCommandgroup(), 
			    								    a2dCanvasOFlags::SELECTED, a2dCanvasOFlags::SELECTED2 ) );
        }
        StartEditTool( original );
    }
    else
        a2dStTool::FinishBusyMode( true );
}

void a2dStDrawTool::FinishBusyMode( bool closeCommandGroup )
{
    a2dREFOBJECTPTR_KEEPALIVE;
    m_pending = true;
    a2dCanvasObjectPtr original = m_original;
    if( GetDrawingPart() )
    {
        if ( m_stcontroller->GetSelectAtEnd() )
        {
            DeselectAll();
            original->SetSelected( true );
            original->SetSelected2( true );
        }
        if ( m_stcontroller->GetSelectionStateUndo() ) 
        {
	        GetCanvasCommandProcessor()->Submit( new a2dCommandMasked( GetDrawingPart()->GetShowObject(), GetCommandgroup(), 
			    								    a2dCanvasOFlags::SELECTED, a2dCanvasOFlags::SELECTED2 ) );
        }
        a2dStTool::FinishBusyMode( closeCommandGroup );
    }
    else
        a2dStTool::FinishBusyMode( closeCommandGroup );
}

void a2dStDrawTool::AbortBusyMode()
{
    m_pending = true;
    a2dStTool::AbortBusyMode();
}

void a2dStDrawTool::OnKeyDown( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_SPACE:
            {
                FinishBusyMode();
                break;
            }
            case WXK_ESCAPE:
            {
                AbortBusyMode();
                break;
            }
            default:
                event.Skip();
        }
    }
    else
        event.Skip();
}

void a2dStDrawTool::OnKeyUp( wxKeyEvent& event )
{
    event.Skip();
}

void a2dStDrawTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
    }
    else
        event.Skip();
}

void a2dStDrawTool::AdjustRenderOptions()
{
    a2dStTool::AdjustRenderOptions();
}

//----------------------------------------------------------------------------
// a2dDrawRectangleTool
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dDrawRectangleTool, a2dStDrawTool )
    EVT_CHAR( a2dDrawRectangleTool::OnChar )
    EVT_MOUSE_EVENTS( a2dDrawRectangleTool::OnMouseEvent )
END_EVENT_TABLE()

a2dDrawRectangleTool::a2dDrawRectangleTool( a2dStToolContr* controller, a2dRect* templateObject ): a2dStDrawTool( controller )
{
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );
    m_radius = 0;

    m_AllowModifyTemplate = true;
    m_templateObject = templateObject;
    if ( !m_templateObject )
    {
        m_templateObject = new a2dRect();
        m_templateObject->SetContourWidth( GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale() );
        m_contourwidth = GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale();
    }
    else
    {
        m_layer = m_templateObject->GetLayer();
        m_fill = m_templateObject->GetFill();
        m_stroke = m_templateObject->GetStroke();
        m_contourwidth = m_templateObject->GetContourWidth();
    }

}

a2dDrawRectangleTool::~a2dDrawRectangleTool()
{
}

void a2dDrawRectangleTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStTool::AddAnotationToPendingUpdateArea();

    a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
    m_anotation.Printf( _T( "w %6.3f h %6.3f" ), rec->GetWidth(), rec->GetHeight() );

    wxCoord w, h, descent, external;
    GetTextExtent( m_anotation, &w, &h, &descent, &external );

    m_xanotation = m_x + h;
    m_yanotation = m_y + h;

    a2dStDrawTool::AddAnotationToPendingUpdateArea();
}

void a2dDrawRectangleTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }
    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    }

    if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        GetDrawingPart()->PushCursor( m_toolBusyCursor );

        m_original = m_templateObject->TClone( clone_childs );
        m_original->Translate( m_xwprev, m_ywprev );
        m_original->SetContourWidth( m_contourwidth );
        m_original->SetStroke( m_stroke );
        m_original->SetFill( m_fill );
        m_original->SetLayer( m_layer );

        if ( !EnterBusyMode() )
            return;
        AddCurrent( m_original );

        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
	    double unitScale = GetDrawing()->GetUnitsScale();
		wxString state;
		wxString form = m_stcontroller->GetFormat() + " " + m_stcontroller->GetFormat();
		state.Printf( form, rec->GetPosX()*unitScale, rec->GetPosY()*unitScale );
		SetStateString( state, 10 );
		form = _T("width = ") + m_stcontroller->GetFormat() + _T(" height = ") + m_stcontroller->GetFormat();
		state.Printf( form, rec->GetWidth()*unitScale, rec->GetHeight()*unitScale );
		SetStateString( state, 11 );

    }
    else if ( event.LeftDown() && GetBusy() )
    {
        GetDrawingPart()->PopCursor();
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );

		a2dGeneralGlobals->RecordF( this, _T( "rect %f %f %f %f" ), rec->GetPosX(), rec->GetPosY(), rec->GetWidth(), rec->GetHeight() );

        StartEditingOrFinish();
    }
    else  if ( ( event.Dragging() && GetBusy() ) || ( event.Moving() && GetBusy() ) )
    {
        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
        rec->SetWidth( m_xwprev - rec->GetPosX() );
        rec->SetHeight( m_ywprev - rec->GetPosY() );

        a2dRect* recorg = wxStaticCast( m_original.Get(), a2dRect );
        recorg->SetWidth( rec->GetWidth() );
        recorg->SetHeight( rec->GetHeight() );

	    double unitScale = GetDrawing()->GetUnitsScale();
		wxString state;
		wxString form = m_stcontroller->GetFormat() + " " + m_stcontroller->GetFormat();
		state.Printf( form, rec->GetPosX()*unitScale, rec->GetPosY()*unitScale );
		SetStateString( state, 10 );
		form = _T("width = ") + m_stcontroller->GetFormat() + _T(" height = ") + m_stcontroller->GetFormat();
		state.Printf( form, rec->GetWidth()*unitScale, rec->GetHeight()*unitScale );
		SetStateString( state, 11 );

        m_pending = true;
    }
    else
        event.Skip();
}

//----------------------------------------------------------------------------
// a2dZoomTool
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dZoomTool, a2dStTool )
    EVT_CHAR( a2dZoomTool::OnChar )
    EVT_MOUSE_EVENTS( a2dZoomTool::OnMouseEvent )
END_EVENT_TABLE()

a2dZoomTool::a2dZoomTool( a2dStToolContr* controller ): a2dStTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_isEditTool = false;
    m_anotate = true;//false;
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_MAGNIFIER );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );

    if ( GetDrawingPart()->GetDrawer2D()->HasAlpha() )
    {
        SetFill( a2dFill( wxColour( 66, 159, 235, 50 ) ) );
        SetStroke( a2dStroke( wxColour( 66, 159, 235, 165 ), 2, a2dSTROKE_LONG_DASH ) );
    }
    else
    {
        SetFill( *a2dTRANSPARENT_FILL );
        SetStroke( a2dStroke( *wxBLACK, 1, a2dSTROKE_LONG_DASH ) );
    }
}

a2dZoomTool::a2dZoomTool( const a2dZoomTool& other, CloneOptions options, a2dRefMap* refs )
    : a2dStTool( other, options, refs )
{
    m_eventHandler = other.m_eventHandler;

    m_anotate = other.m_anotate;
    m_toolcursor = other.m_toolcursor;
    m_toolBusyCursor = other.m_toolBusyCursor;
    m_isEditTool = other.m_isEditTool;

    m_fill = other.m_fill;
    m_stroke = other.m_stroke;
}

a2dObject* a2dZoomTool::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dZoomTool( *this, options, refs );
}

a2dZoomTool::~a2dZoomTool()
{
}

void a2dZoomTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStTool::AddAnotationToPendingUpdateArea();

    a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
    m_anotation.Printf( _T( "w %6.3f h %6.3f" ), rec->GetWidth(), rec->GetHeight() );
    m_xanotation = m_x;
    m_yanotation = m_y;

    wxCoord w, h, descent, external;
    GetTextExtent( m_anotation, &w, &h, &descent, &external );

    m_xanotation = m_x + h;
    m_yanotation = m_y + h;

    a2dStTool::AddAnotationToPendingUpdateArea();
}

void a2dZoomTool::OnChar( wxKeyEvent& event )
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

void a2dZoomTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();

    if ( event.LeftDClick() && !GetBusy() )
    {
        m_stcontroller->Zoomout();
    }
    else if ( event.LeftDown() && !GetBusy() )
    {
        MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );
        a2dRect* rec = new a2dRect( m_xwprev, m_ywprev, 0, 0, 0 );
        m_canvasobject = rec;
        // objects are rendered via tool, but stored in document or view.
        AddDecorationObject( rec );
        rec->SetFill( m_fill );
        rec->SetStroke( m_stroke );
        rec->Update( a2dCanvasObject::updatemask_force );
        m_pending = true;

        //special case to have the canvas itself recieve at least this one also.
        //because often the zoomtool is active as only tool, but
        //some object on the canvas are meant to react to MD (Href's etc.)
        event.Skip();
        if ( !EnterBusyMode() )
            return;
    }
    else if ( event.LeftUp() /*event.LeftDown()*/ && GetBusy() )
    {
        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
        //Prevent this object from being visible before it is deleted.
        m_canvasobject->SetVisible( false );

        double w = GetDrawer2D()->WorldToDeviceXRel( rec->GetWidth() );
        double h = GetDrawer2D()->WorldToDeviceYRel( rec->GetHeight() );

        //to be able to intercept doubleclick
        //ignore the LeftDown and LeftUp if mouse position is the same
        if ( fabs( w ) < 3 && fabs( h ) < 3 )
        {
            GetDisplayWindow()->Refresh();
            FinishBusyMode();
            event.Skip();
        }
        else
        {
            double x1 = rec->GetBboxMinX();

            double y1 = rec->GetBboxMinY();

            //Get the current window size to put on zoomstack
            a2dBoundingBox* bbox = new a2dBoundingBox( GetDrawer2D()->GetVisibleMinX(),
                    GetDrawer2D()->GetVisibleMinY(),
                    GetDrawer2D()->GetVisibleMaxX(),
                    GetDrawer2D()->GetVisibleMaxY()
                                                     );

            m_stcontroller->GetZoomList().Insert( bbox );

			GetCanvasCommandProcessor()->Zoom( x1, y1, rec->GetBboxWidth(), rec->GetBboxHeight(), false );

            a2dGeneralGlobals->RecordF( this, _T( "zoom %f %f %f %f" ), x1, y1, x1 + rec->GetBboxWidth(), y1 + rec->GetBboxHeight() );

            //generate an OnPaint event in order to redraw
            //other tools that are busy at this moment
            //GetDrawingPart()->Refresh();

            FinishBusyMode();
            RemoveDecorationObject( rec );

            event.Skip();
        }
    }
    else  if ( ( event.Dragging() && GetBusy() ) /*|| (event.Moving() && GetBusy())*/ )
    {
        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );

        MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

        rec->SetWidth( m_xwprev - rec->GetPosX() );
        rec->SetHeight( m_ywprev - rec->GetPosY() );
        rec->SetPending( true );
        m_pending = true;
    }
    else if( event.RightUp() && GetBusy() )
    {
        if ( m_oneshot )
            StopTool();
        event.Skip(); //for popup menu in stcontroller.
    }
    else
        event.Skip();
}

BEGIN_EVENT_TABLE( a2dSelectTool, a2dStTool )
    EVT_CHAR( a2dSelectTool::OnChar )
    EVT_MOUSE_EVENTS( a2dSelectTool::OnMouseEvent )
    EVT_KEY_DOWN( a2dSelectTool::OnKeyDown )
    EVT_KEY_UP(  a2dSelectTool::OnKeyUp )
END_EVENT_TABLE()

a2dSelectTool::a2dSelectTool( a2dStToolContr* controller ): a2dStTool( controller )
{
    m_storeUndo = false;

    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_isEditTool = false;
    m_anotate = true;//false;
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );
    m_shift_is_add = false;

    if ( GetDrawingPart()->GetDrawer2D()->HasAlpha() )
    {
        //SetFill( a2dFill( wxColour( 233, 15, 23, 220 ) ) );
        //SetStroke( a2dStroke( wxColour( 255, 59, 25, 65 ), 1, a2dSTROKE_LONG_DASH ) );
        SetStroke( a2dCanvasGlobals->GetHabitat()->GetSelectStroke() );
        SetFill( a2dCanvasGlobals->GetHabitat()->GetSelectFill() );
    }
    else
    {
        //SetFill( *a2dTRANSPARENT_FILL );
        //SetStroke( a2dStroke( *wxRED, 10, a2dSTROKE_LONG_DASH ) );
        SetStroke( a2dCanvasGlobals->GetHabitat()->GetSelectStroke() );
        SetFill( a2dCanvasGlobals->GetHabitat()->GetSelectFill() );
    }

    m_deselectAtStart = false;
}

a2dSelectTool::~a2dSelectTool()
{
}

void a2dSelectTool::DeselectAll()
{
	a2dStTool::DeselectAll();
    //GetCanvasCommandProcessor()->Submit( new a2dCommand_Select( m_parentobject, a2dCommand_Select::Args().what( a2dCommand_Select::DeSelectAll ) ) );
}

void a2dSelectTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStTool::AddAnotationToPendingUpdateArea();

    a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
    m_anotation.Printf( _T( "w %6.3f h %6.3f" ), rec->GetWidth(), rec->GetHeight() );

    wxCoord w, h, descent, external;
    GetTextExtent( m_anotation, &w, &h, &descent, &external );

    m_xanotation = m_x + h;
    m_yanotation = m_y + h;

    a2dStTool::AddAnotationToPendingUpdateArea();
}

void a2dSelectTool::OnKeyDown( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }


    if ( event.m_shiftDown )
    {   //maybe more cases like this, OnKeyDown has effect on restrict engine, in a2dStTool::OnKeyUp
    }
    else
        event.Skip();
}

void a2dSelectTool::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    if ( event.m_shiftDown )
    {   //maybe more cases like this, OnKeyDown has effect on restrict engine, in a2dStTool::OnKeyDown
    }
    else
        event.Skip();
}

void a2dSelectTool::OnChar( wxKeyEvent& event )
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

void a2dSelectTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    if ( event.Moving() )
    {
        if ( !GetBusy() )
        {
            double xWorld, yWorld;
            GetDrawingPart()->MouseToToolWorld( m_x, m_y, xWorld, yWorld );

            static a2dCanvasObject* oldhitobject = NULL;
            a2dCanvasObject* hitobject = NULL;
            hitobject = GetDrawingPart()->IsHitWorld( xWorld, yWorld );
            if ( hitobject  )
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );
            else
                GetDrawingPart()->SetCursor( m_toolcursor );
        }
    }

    if ( event.LeftDClick() && !GetBusy() )
    {

    }
    else if ( event.LeftDown() && !GetBusy() )
    {
        if ( !EnterBusyMode() )
            return;

        //special case to have the canvas itself recieve at least this one also.
        //because often the zoomtool is active as only tool, but
        //some object on the canvas are meant to react to MD (Href's etc.)
        event.Skip();
    }
    else if ( event.LeftUp() && GetBusy() )
    {
        if ( m_storeUndo )
            OpenCommandGroup( false );

        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
        //Prevent this object from being visible before it is deleted.
        m_canvasobject->SetVisible( false );

        double w = GetDrawer2D()->WorldToDeviceXRel( rec->GetWidth() );
        double h = GetDrawer2D()->WorldToDeviceYRel( rec->GetHeight() );

        if ( fabs( w ) < 3 && fabs( h ) < 3 )
        {
            // The user did click somewhere ( no drag )

            a2dCanvasObject* hitobject = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev );

            if( m_shift_is_add )
            {
                if ( !event.m_shiftDown )
                {
                    //if NOT shift down, deselect all and  set selection to hit object.
                    DeselectAll();

                    if ( hitobject ) //set selection to hit object.
                    {
                        GetCanvasCommandProcessor()->Submit(
                            new a2dCommand_SetSelectFlag( hitobject, true )
                            , m_storeUndo
                        );
                    }
                }
                else
                {
                    //if shift down, add to selection when hit on object, else nothing
                    if ( hitobject ) // deselect all if not on an object
                    {
                        GetCanvasCommandProcessor()->Submit(
                            new a2dCommand_SetSelectFlag( hitobject, !hitobject->GetSelected() )
                            , m_storeUndo
                        );
                    }
                }

                if ( !hitobject )
                {
                    if( !m_oneshot )
                        event.Skip();
                    FinishBusyMode( m_storeUndo );
                    return;
                }
            }
            else
            {
                if ( !hitobject )
                {
                    if( !m_oneshot )
                        event.Skip();
                    FinishBusyMode( m_storeUndo );
                    return;
                }

                if ( event.m_shiftDown || hitobject->GetSelected() )
                {
                    GetCanvasCommandProcessor()->Submit(
                        new a2dCommand_SetSelectFlag( hitobject, false )
                        , m_storeUndo
                    );
                }
                else
                {
                    GetCanvasCommandProcessor()->Submit(
                        new a2dCommand_SetSelectFlag( hitobject, true )
                        , m_storeUndo
                    );
                }
            }
        }
        else
        {
            if( m_shift_is_add )
            {
                //if shift is not down, start a new selection, by first deslecting all,
                //and dragging a new selection rectangle

                if ( !event.m_shiftDown )
                {
                    DeselectAll();
                    GetCanvasCommandProcessor()->Submit(
                        new a2dCommand_Select( m_parentobject, a2dCommand_Select::Args()
                                               .what( a2dCommand_Select::SelectRect )
                                               .x1( rec->GetBbox().GetMinX() )
                                               .y1( rec->GetBbox().GetMinY() )
                                               .x2( rec->GetBbox().GetMaxX() )
                                               .y2( rec->GetBbox().GetMaxY() )
                                             )
                        , m_storeUndo
                    );
                }
                else
                {
                    GetCanvasCommandProcessor()->Submit(
                        new a2dCommand_Select( m_parentobject, a2dCommand_Select::Args()
                                               .what( a2dCommand_Select::SelectRect )
                                               .x1( rec->GetBbox().GetMinX() )
                                               .y1( rec->GetBbox().GetMinY() )
                                               .x2( rec->GetBbox().GetMaxX() )
                                               .y2( rec->GetBbox().GetMaxY() )
                                             )
                        , m_storeUndo
                    );
                }
            }
            else
            {
                if ( event.m_shiftDown )
                {
                    GetCanvasCommandProcessor()->Submit(
                        new a2dCommand_Select( m_parentobject, a2dCommand_Select::Args()
                                               .what( a2dCommand_Select::DeSelectRect )
                                               .x1( rec->GetBbox().GetMinX() )
                                               .y1( rec->GetBbox().GetMinY() )
                                               .x2( rec->GetBbox().GetMaxX() )
                                               .y2( rec->GetBbox().GetMaxY() )
                                             )
                        , m_storeUndo
                    );
                }
                else
                {
                    GetCanvasCommandProcessor()->Submit(
                        new a2dCommand_Select( m_parentobject, a2dCommand_Select::Args()
                                               .what( a2dCommand_Select::SelectRect )
                                               .x1( rec->GetBbox().GetMinX() )
                                               .y1( rec->GetBbox().GetMinY() )
                                               .x2( rec->GetBbox().GetMaxX() )
                                               .y2( rec->GetBbox().GetMaxY() )
                                             )
                        , m_storeUndo
                    );
                }
            }
        }

        m_pending = true;
        a2dREFOBJECTPTR_KEEPALIVE;
        FinishBusyMode( m_storeUndo );
        GetDrawingPart()->Update( a2dCANVIEW_UPDATE_SYNC_DRAWERS );
    }
    else  if ( ( event.Dragging() && GetBusy() ) /*|| (event.Moving() && GetBusy())*/ )
    {
        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
        rec->SetWidth( m_xwprev - rec->GetPosX() );
        rec->SetHeight( m_ywprev - rec->GetPosY() );
        rec->SetPending( true );

        m_pending = true;
    }
    else
        event.Skip();
}

bool a2dSelectTool::EnterBusyMode()
{
    EnterBusyModeNoGroup();

    if( !CreateToolObjects() )
    {
        a2dBaseTool::AbortBusyMode();
        return false;
    }

    return true;
}

bool a2dSelectTool::CreateToolObjects()
{
    // The base class CleanupToolObjects is fine and also does an equivalent of
    // RemoveEditobject, so this is not overloaded
    a2dRect* rec = new a2dRect( m_xwprev, m_ywprev, 0, 0, 0 );
    m_canvasobject = rec;
    rec->SetFill( m_fill );
    rec->SetStroke( m_stroke );
    rec->Update( a2dCanvasObject::updatemask_force );
    rec->SetPending( true );
    AddDecorationObject( rec );
    return true;
}

BEGIN_EVENT_TABLE( a2dImageTool, a2dStDrawTool )
    EVT_CHAR( a2dImageTool::OnChar )
    EVT_MOUSE_EVENTS( a2dImageTool::OnMouseEvent )
END_EVENT_TABLE()

a2dImageTool::a2dImageTool( a2dStToolContr* controller ): a2dStDrawTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_drawPatternOnTop = false;
    m_imageEmbedded = false;
    m_anotate = true;//false;
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );

    //i like transparent fill and specific stroke
    SetFill( *a2dTRANSPARENT_FILL );
    SetStroke( a2dStroke( *wxRED, 1, a2dSTROKE_DOT ) );
}

a2dImageTool::~a2dImageTool()
{
}

void a2dImageTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStDrawTool::AddAnotationToPendingUpdateArea();

    a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
    m_anotation.Printf( _T( "w %6.3f h %6.3f" ), rec->GetWidth(), rec->GetHeight() );

    wxCoord w, h, d, l;
    GetTextExtent( m_anotation, &w, &h, &d, &l );
    m_xanotation = m_x + ( int ) h;
    m_yanotation = m_y + ( int ) h;
    a2dStDrawTool::AddAnotationToPendingUpdateArea();
}

void a2dImageTool::OnChar( wxKeyEvent& event )
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

void a2dImageTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    }

    if ( event.LeftDClick() && !GetBusy() )
    {

    }
    else if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dRect* rec = new a2dRect( m_xwprev, m_ywprev, 0, 0, 0 );
        rec->SetRoot( GetDrawing(), false );
        rec->SetFill( m_fill );
        rec->SetStroke( m_stroke );
        rec->Update( a2dCanvasObject::updatemask_force );

        m_original = rec;
        if ( !EnterBusyMode() )
            return;

        //special case to have the canvas itself recieve at least this one also.
        //because often the zoomtool is active as only tool, but
        //some object on the canvas are meant to react to MD (Href's etc.)
        event.Skip();
    }
    else if ( event.LeftDown() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        CleanupToolObjects();

        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );

        double w = GetDrawer2D()->WorldToDeviceXRel( rec->GetWidth() );
        double h = GetDrawer2D()->WorldToDeviceYRel( rec->GetHeight() );

        static wxString image_dir = _T( "./" );
        static wxString filter = _T( "All files (*.*)| *.*| BMP files (*.bmp)|*.bmp| GIF files (*.gif)|*.gif| JPEG files (*.jpg)|*.jpg;*.JPG| PNG files (*.png)| *.png" );

        wxFileName image_file;

        wxFileDialog dialog( GetDisplayWindow(), _T( "Select Image File" ), image_dir, image_file.GetFullPath(), filter, wxFD_OPEN );

        if ( dialog.ShowModal() != wxID_OK )
            return;

        // save for the next time
        image_dir = dialog.GetDirectory();
        image_file = dialog.GetFilename();

        wxBitmapType type = wxBITMAP_TYPE_ANY;

        if ( image_file.GetExt() == _T( "bmp" ) )
            type = wxBITMAP_TYPE_BMP;
        else if ( image_file.GetExt() == _T( "gif" ) )
            type = wxBITMAP_TYPE_GIF;
        else if ( image_file.GetExt() == _T( "jpg" ) )
            type = wxBITMAP_TYPE_JPEG;
        else if ( image_file.GetExt() == _T( "png" ) )
            type = wxBITMAP_TYPE_PNG;
        else
            type = wxBITMAP_TYPE_ANY;

        wxImage image;

        a2dImage* canvas_image; //canvas image object to create

#ifdef __WXMSW__
        const wxChar SEP = wxT( '\\' );
#else
        const wxChar SEP = wxT( '/' );
#endif

        wxString image_fullpath = image_dir + SEP + image_file.GetFullPath();
        image.LoadFile( image_fullpath, type );

        double x1 = rec->GetBbox().GetMinX();
        double y1 = rec->GetBbox().GetMinY();
        double image_w = fabs( rec->GetWidth() );
        double image_h = fabs( rec->GetHeight() );

        if ( fabs( w ) < 3 && fabs( h ) < 3 )
        {
            image_w = image.GetWidth();
            image_h = image.GetHeight();

            canvas_image = new a2dImage( image, x1 + image_w / 2 , y1 + image_h / 2, 0, 0 );
        }
        else
        {
            canvas_image = new a2dImage( image, x1 + image_w / 2 , y1 + image_h / 2, image_w, image_h );
        }
        if ( m_imageEmbedded )
            canvas_image->SetFilename( "", type, false );
        else
            canvas_image->SetFilename( image_fullpath, type, false );
        canvas_image->SetRoot( GetDrawing(), false );
        canvas_image->SetFill( *a2dTRANSPARENT_FILL );
        canvas_image->SetStroke( *a2dTRANSPARENT_STROKE );
        canvas_image->SetLayer( m_layer );
        canvas_image->SetImageType( type );
        canvas_image->SetDrawPatternOnTop( m_drawPatternOnTop );

        m_original = canvas_image;
        AddCurrent( canvas_image );

        a2dREFOBJECTPTR_KEEPALIVE;
        FinishBusyMode();
        GetDrawingPart()->Update( a2dCANVIEW_UPDATE_SYNC_DRAWERS );
    }
    else  if ( ( event.Dragging() && GetBusy() ) || ( event.Moving() && GetBusy() ) )
    {
        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
        rec->SetWidth( m_xwprev - rec->GetPosX() );
        rec->SetHeight( m_ywprev - rec->GetPosY() );

        m_pending = true;
    }
    else
        event.Skip();
}


BEGIN_EVENT_TABLE( a2dCopyTool, a2dDragTool )
    EVT_MOUSE_EVENTS( a2dCopyTool::OnMouseEvent )
END_EVENT_TABLE()

a2dCopyTool::a2dCopyTool( a2dStToolContr* controller, a2dCanvasObject* copyobject, double xw, double yw ): a2dDragTool( controller )
{
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_Copy );
    m_mindist = GetDrawing()->GetHabitat()->GetCopyMinDistance();
    m_deepCopy = true;

    if ( copyobject )
    {
        m_original = GetDrawingPart()->IsHitWorld( xw, yw, wxLAYER_ALL, a2dCANOBJHITOPTION_LAYERS );

        if ( !m_original )
        {
            m_original = 0;
            return;
        }
        if ( !m_original->GetDraggable() )
        {
            m_original = 0;
            return;
        }

        if ( !EnterBusyMode() )
            return;

        m_xwprev = xw;
        m_ywprev = yw;
        m_startMousex = xw;
        m_startMousey = yw;

        double dx = xw - m_startMousex;
        double dy = yw - m_startMousey;
        AdjustShift( &dx, &dy );
        DragAbsolute( m_startObjectx + dx, m_startObjecty + dy );
    }
}

bool a2dCopyTool::CreateToolObjects()
{
    // Don't clone connected wires as in a2dDragTool
    m_canvasobject = m_original->TClone( clone_childs | clone_seteditcopy | clone_setoriginal | clone_members | clone_noCameleonRef );
    m_canvasobject->SetSelected( false );

    AddEditobject( m_canvasobject );
    m_pending = true;

    return true;
}

bool a2dCopyTool::EnterBusyMode()
{
    if( !a2dStTool::EnterBusyMode() )
        return false;

    CaptureMouse();

    m_wasMayConnect = m_original->DoConnect();
    m_original->DoConnect( false );
    //the drag copy should not connect while dragging
    m_canvasobject->DoConnect( false );
    m_original->DoConnect( false );

    m_startObjectx = m_canvasobject->GetPosX();
    m_startObjecty = m_canvasobject->GetPosY();

    return true;
}

void a2dCopyTool::FinishBusyMode( bool closeCommandGroup )
{
    //restore connect backup
    m_original->DoConnect( m_wasMayConnect );
    a2dCanvasObjectPtr copy = NULL; 
    if ( m_deepCopy )
        copy = m_original->TClone( clone_deep );
    else
        copy = m_original->TClone( clone_flat | clone_childs | clone_noCameleonRef );

    GetCanvasCommandProcessor()->Submit(
        new a2dCommand_AddObject( m_parentobject, copy )
    );

    GetCanvasCommandProcessor()->Submit(
        new a2dCommand_SetCanvasProperty( copy, a2dCanvasObject::PROPID_TransformMatrix, m_canvasobject->GetTransformMatrix() )
    );

    if ( m_stcontroller->GetSelectAtEnd() )
    {
        DeselectAll();
        m_original->SetSelected( true );
        m_original->SetSelected2( true );
    }
    if ( m_stcontroller->GetSelectionStateUndo() ) 
    {
	    GetCanvasCommandProcessor()->Submit( new a2dCommandMasked( GetDrawingPart()->GetShowObject(), GetCommandgroup(), 
			    								a2dCanvasOFlags::SELECTED, a2dCanvasOFlags::SELECTED2 ) );
    }

    ReleaseMouse();

    a2dIterC ic( GetDrawingPart() );

    if ( m_lateconnect && m_wasMayConnect )
    {
        //functionality to connect object pins to objectpins which now can connect.
        m_original->SetAlgoSkip( true );
        m_canvasobject->SetAlgoSkip( true );
        copy->SetAlgoSkip( true );
        //a2dCanvasObject::PROPID_DisableFeedback->SetPropertyToObject( m_original, true );
        //a2dCanvasObject::PROPID_DisableFeedback->SetPropertyToObject( m_canvasobject, true );
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->
    	    ConnectToPinsObject( GetDrawingPart()->GetShowObject(), copy, GetHitMargin() );

        m_original->SetAlgoSkip( false );
        m_canvasobject->SetAlgoSkip( false );
        copy->SetAlgoSkip( false );
        //m_original->RemoveProperty( a2dCanvasObject::PROPID_DisableFeedback );
        //m_canvasobject->RemoveProperty( a2dCanvasObject::PROPID_DisableFeedback );
    }

    a2dStTool::FinishBusyMode();
}

void a2dCopyTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    int x = event.GetX();
    int y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw, yw;
    MouseToToolWorld( x, y, xw, yw );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
    }

    if ( event.Moving() && !GetBusy() )
    {
        a2dCanvasObject* hit = GetDrawingPart()->IsHitWorld( xw, yw );
        if ( hit && hit->GetDraggable() )
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );
        else
            GetDrawingPart()->SetCursor( m_toolcursor );
    }

    if ( event.LeftDown() )
    {
        m_original = GetDrawingPart()->IsHitWorld( xw, yw, wxLAYER_ALL, a2dCANOBJHITOPTION_LAYERS );

        if ( !m_original )
        {
            m_original = 0;
            return;
        }
        if ( !m_original->GetDraggable() )
        {
            m_original = 0;
            return;
        }

        if ( !EnterBusyMode() )
            return;

        m_xwprev = xw;
        m_ywprev = yw;
        m_startMousex = xw;
        m_startMousey = yw;

        if( restrict )
            restrict->SetRestrictPoint( m_startMousex, m_startMousey );

        double dx = xw - m_startMousex;
        double dy = yw - m_startMousey;
        AdjustShift( &dx, &dy );
        DragAbsolute( m_startObjectx + dx, m_startObjecty + dy );
    }
    else if ( event.LeftUp() && GetBusy() )
    {
        // Needed by FinishBusyMode
        m_xwprev = xw;
        m_ywprev = yw;

        FinishBusyMode();
    }
    else if ( GetBusy() )
    {
        double dx = xw - m_startMousex;
        double dy = yw - m_startMousey;
        AdjustShift( &dx, &dy );
        DragAbsolute( m_startObjectx + dx, m_startObjecty + dy );
        m_xwprev = xw;
        m_ywprev = yw;
    }
    else
    {
        event.Skip();
    }
}

void a2dCopyTool::AdjustShift( double* dx, double* dy )
{
    if( ( *dx ) * ( *dx ) + ( *dy ) * ( *dy ) < m_mindist )
    {
        if( ( *dx ) == 0 && ( *dy ) == 0 )
        {
            ( *dx ) = m_mindist * 1.0;
            ( *dy ) = m_mindist * 0.5;
        }
        else
        {
            double fac = m_mindist / sqrt( ( *dx ) * ( *dx ) + ( *dy ) * ( *dy ) );
            ( *dx ) *= fac;
            ( *dy ) *= fac;
        }
    }
}

BEGIN_EVENT_TABLE( a2dDragTool, a2dStTool )
    EVT_MOUSE_EVENTS( a2dDragTool::OnMouseEvent )
    EVT_CHAR( a2dDragTool::OnChar )
END_EVENT_TABLE()

a2dDragTool::a2dDragTool( a2dStToolContr* controller, a2dCanvasObject* dragobject, double xw, double yw ): a2dStTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag );
    m_mousecaptured = false;
    m_deleteonoutsidedrop = false;
    m_dropAndDrop = false;

    m_anotate = false;
    m_lateconnect = true;

    //i like transparent fill and specific stroke
    SetFill( *a2dTRANSPARENT_FILL );
    SetStroke( a2dStroke( *wxRED, 1, a2dSTROKE_DOT ) );

    if ( dragobject )
    {
        GetDrawingPart()->ToolWorldToMouse( xw, yw, m_x, m_y );
        GetDrawing()->GetHabitat()->SetLastXyEntry( xw, yw );
        a2dCanvasObject* original = GetDrawingPart()->IsHitWorld( xw, yw, wxLAYER_ALL, a2dCANOBJHITOPTION_LAYERS );

        if ( !StartDragging( m_x, m_y, original ) )
            return;
    }

    m_snapSourceFeatures = a2dRestrictionEngine::snapToNone;

    //only modes which are usefull in dragging
    m_snapTargetFeatures = 
                a2dRestrictionEngine::snapToGridPos |
                a2dRestrictionEngine::snapToGridPosForced |
                a2dRestrictionEngine::snapToBoundingBox |
                a2dRestrictionEngine::snapToObjectPos |
                //a2dRestrictionEngine::snapToPinsUnconnected |
                a2dRestrictionEngine::snapToPins |
                a2dRestrictionEngine::snapToObjectVertexes |
                a2dRestrictionEngine::snapToObjectSnapVPath;
}

a2dDragTool::~a2dDragTool()
{
}

void a2dDragTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        double xw = m_xwprev;
        double yw = m_ywprev;

        double shiftx;
        double shifty;
        GetKeyIncrement( &shiftx, &shifty );

        //now shift with snap distances in X or Y
        switch( event.GetKeyCode() )
        {
			case WXK_DELETE:
			{
				// if busy do nothing with this key.
				break;
			}
            case WXK_UP:
                shiftx = 0;
                m_xwprev = xw + shiftx;
                m_ywprev = yw + shifty;
                DragAbsolute( m_xwprev, m_ywprev );

                GetDrawingPart()->Update( a2dCANVIEW_UPDATE_SYNC_DRAWERS );
                GetDisplayWindow()->WarpPointer( GetDrawer2D()->WorldToDeviceX( m_xwprev ), GetDrawer2D()->WorldToDeviceY( m_ywprev ) );
                break;

            case WXK_DOWN:
                shiftx = 0;
                shifty = -shifty;
                m_xwprev = xw + shiftx;
                m_ywprev = yw + shifty;
                DragAbsolute( m_xwprev, m_ywprev );

                GetDrawingPart()->Update( a2dCANVIEW_UPDATE_SYNC_DRAWERS );
                GetDisplayWindow()->WarpPointer( GetDrawer2D()->WorldToDeviceX( m_xwprev ), GetDrawer2D()->WorldToDeviceY( m_ywprev ) );
                break;

            case WXK_LEFT:
                shiftx = -shiftx;
                shifty = 0;
                m_xwprev = xw + shiftx;
                m_ywprev = yw + shifty;
                DragAbsolute( m_xwprev, m_ywprev );

                GetDrawingPart()->Update( a2dCANVIEW_UPDATE_SYNC_DRAWERS );
                GetDisplayWindow()->WarpPointer( GetDrawer2D()->WorldToDeviceX( m_xwprev ), GetDrawer2D()->WorldToDeviceY( m_ywprev ) );
                break;

            case WXK_RIGHT:
                shifty = 0;
                m_xwprev = xw + shiftx;
                m_ywprev = yw + shifty;
                DragAbsolute( m_xwprev, m_ywprev );

                GetDrawingPart()->Update( a2dCANVIEW_UPDATE_SYNC_DRAWERS );
                GetDisplayWindow()->WarpPointer( GetDrawer2D()->WorldToDeviceX( m_xwprev ), GetDrawer2D()->WorldToDeviceY( m_ywprev ) );
                break;

            case '9':
            {
                //if ( event.m_controlDown )
                if ( GetBusy() )
                {
                    a2dAffineMatrix mat = m_canvasobject->GetTransform();
                    mat.Rotate( 90, m_canvasobject->GetPosX(), m_canvasobject->GetPosY() );
                    m_canvasobject->SetTransform( mat );
                }
                break;
            }
            default:
            {
                shiftx = 0;
                shifty = 0;
                event.Skip();
            }
        }
    }
    else
        event.Skip();

}

void a2dDragTool::Render()
{
    if ( m_original && GetBusy() )
    {
        a2dStTool::Render();
    }
}

void a2dDragTool::AdjustShift( double* WXUNUSED( x ), double* WXUNUSED( y ) )
{
}

bool a2dDragTool::StartDragging( int x, int y, a2dCanvasObject* original )
{
    m_x = x;
    m_y = y;
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );

    m_original = original;

    if ( !m_original || !m_original->GetDraggable() )
    {
        m_original = 0;
        return false;
    }

    m_transform = m_original->GetTransformMatrix();

    if ( !EnterBusyMode() )
        return false;

    m_startMousex = m_xwprev;
    m_startMousey = m_ywprev;

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
        restrict->SetRestrictPoint( m_original->GetPosX(), m_original->GetPosY() );
    return true;
}

void a2dDragTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
    }

    if ( event.Moving() && !GetBusy() )
    {
        a2dCanvasObject* hit = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev );
        if ( hit && hit->GetDraggable() )
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );
        else
            GetDrawingPart()->SetCursor( m_toolcursor );
        // this to allow EVT_CANVASOBJECT_LEAVE_EVENT
        event.Skip();
    }

    if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dCanvasObject* original = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev, wxLAYER_ALL, a2dCANOBJHITOPTION_LAYERS );

        if ( m_dropAndDrop && original )
        {
            ReleaseMouse();
            a2dCanvasObject* clone = original->TClone( clone_members | clone_childs );
            a2dDnDCanvasObjectDataCVG shapeData( GetDrawingPart() );
            shapeData.Append( clone );
            wxDropSource source(shapeData, GetDrawingPart()->GetDisplayWindow() );

            const wxChar *pc = NULL;
            switch ( source.DoDragDrop(true) )
            {
                default:
                case wxDragError:
                    wxLogError(wxT("An error occurred during drag and drop operation"));
                    break;

                case wxDragNone:
                    SetStateString( _("No drop happened"), STAT_toolHelp );
                    break;

                case wxDragCopy:
                    pc = wxT("copied");
                    break;

                case wxDragMove:
                    pc = wxT("moved");
                    break;

                case wxDragCancel:
                    SetStateString( _("Drag and drop operation cancelled"), STAT_toolHelp );
                    break;
            }

            if ( pc )
            {
                SetStateString(wxString(wxT("Shape successfully ")) + pc, STAT_toolHelp );
            }
            //else: status text already set

        }
        else if ( !StartDragging( m_x, m_y, original ) )
            return;
    }
    else if ( event.LeftUp() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );

        if (  !m_deleteonoutsidedrop &&
              (  m_xwprev < GetDrawer2D()->GetVisibleMinX() ||
                m_xwprev > GetDrawer2D()->GetVisibleMaxX() ||
                m_ywprev < GetDrawer2D()->GetVisibleMinY() ||
                m_ywprev > GetDrawer2D()->GetVisibleMaxY()
              )  
            )
        {
            AbortBusyMode();
            return;
        }
        else
            FinishBusyMode();
    }
    else if ( event.RightDown() )
    {
        event.Skip();
    }
    else if ( GetBusy() )
    {
        //PRFL_AUTO_TMR( a2dDragTool_MouseMove );

        double xPosObject = m_startObjectx - m_startMousex + m_xwprev;
        double yPosObject = m_startObjecty - m_startMousey   + m_ywprev;
        DragAbsolute( xPosObject, yPosObject );

        event.Skip(); //scroll wheel will work

		double unitScale = GetDrawing()->GetUnitsScale();
		wxString state, form;
		form = m_stcontroller->GetFormat() + " " + m_stcontroller->GetFormat();
		state.Printf( form, m_canvasobject->GetBboxMinX()*unitScale, m_canvasobject->GetBboxMinY()*unitScale );
		SetStateString( state, 10 );
		form = _T("width = ") + m_stcontroller->GetFormat() + _T(" height = ") + m_stcontroller->GetFormat();
		state.Printf( form, m_canvasobject->GetBboxWidth()*unitScale, m_canvasobject->GetBboxHeight()*unitScale );
		SetStateString( state, 11 );
    }
    //!todo skip is needed if object should connect itself, in a2dCanvasObject::Update()
    // Do this to be able to scroll inout using mouse wheel.
    else
        event.Skip();
}


void a2dDragTool::CaptureMouse()
{
    GetDrawingPart()->SetCorridor( m_canvasobject, m_canvasobject );
    GetDrawingPart()->GetDisplayWindow()->CaptureMouse();
    m_mousecaptured = true;
}

void a2dDragTool::ReleaseMouse()
{
    if( m_mousecaptured )
    {
        GetDrawingPart()->SetCorridor( NULL, NULL );
        GetDrawingPart()->GetDisplayWindow()->ReleaseMouse();
    }
    m_mousecaptured = false;
}

bool a2dDragTool::EnterBusyMode()
{
    if( !a2dStTool::EnterBusyMode() )
        return false;

    CaptureMouse();

    //the drag copy should not connect while dragging
    m_canvasobject->DoConnect( false );
    m_wasMayConnect = m_original->DoConnect();
    m_original->DoConnect( false );

    m_startObjectx = m_canvasobject->GetPosX();
    m_startObjecty = m_canvasobject->GetPosY();

    return true;
}

void a2dDragTool::FinishBusyMode( bool closeCommandGroup )
{
    //restore connect backup
    m_original->DoConnect( m_wasMayConnect );
    if(
        ( m_deleteonoutsidedrop ) && 
        (
            m_xwprev < GetDrawer2D()->GetVisibleMinX() ||
            m_xwprev > GetDrawer2D()->GetVisibleMaxX() ||
            m_ywprev < GetDrawer2D()->GetVisibleMinY() ||
            m_ywprev > GetDrawer2D()->GetVisibleMaxY()
        )
    )
    {
        if ( m_deleteonoutsidedrop )
        {
            ReleaseMouse();

            // the object was dropped outside of the window
            // The code below is copied from a2dDeleteTool
            m_original->SetPending( true );
            m_original->SetVisible( false );

            //redraw in buffer what should be there without this object
            GetDrawing()->AddPendingUpdatesOldNew();

            GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );
            //delete object from document (actually move to command for redo)
            GetCanvasCommandProcessor()->Submit(
                new a2dCommand_ReleaseObject( m_parentobject, m_original ) );
        }
    }
    else
    {
        ReleaseMouse();

        //the next has internal the functionality to connect object pins to objectpins which now can connect.
        GetCanvasCommandProcessor()->Submit(
            new a2dCommand_SetCanvasProperty( m_original, a2dCanvasObject::PROPID_TransformMatrix, m_canvasobject->GetTransformMatrix() )
        );
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );
    }

    /* TODO maybe
    The next would be a way to drag object from this level to one deeper.
    But in that case we also want the other way around.
    So we would need a drag in a clipper go to one level higher.
    a2dCanvasObject* canvasobjecttophit = GetDrawingPart()->IsHitWorld(xw,yw);

    if (canvasobjecttophit)
    {
        a2dPolygonLClipper* clipper = wxDynamicCast(canvasobjecttophit,a2dPolygonLClipper);
        if (clipper &&  clipper != m_canvasobject )
        {
             GetDrawing()->Remove(m_canvasobject,false);
             clipper->Append(m_canvasobject);
        }
    }
    */

	if ( m_lateconnect && m_wasMayConnect )
	{
		m_original->SetAlgoSkip( true );
		m_canvasobject->SetAlgoSkip( true );
		GetDrawing()->GetHabitat()->GetConnectionGenerator()->
    		ConnectToPinsObject( GetDrawingPart()->GetShowObject(), m_original, GetHitMargin() );

		m_original->SetAlgoSkip( false );
		m_canvasobject->SetAlgoSkip( false );
	}

    a2dStTool::FinishBusyMode();
}

void a2dDragTool::AbortBusyMode()
{
    m_original->DoConnect( m_wasMayConnect );
    ReleaseMouse();
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );
    a2dStTool::AbortBusyMode();
}

void a2dDragTool::DragAbsolute( double x, double y )
{
    a2dPoint2D oldpos = m_canvasobject->GetPosXY();

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        bool snapold = m_original->GetSnapTo(); //do not snap to original
        m_original->SetSnapTo( false );
        //temporarily restore position according to the mouse position, which is the unrestricted position.
        m_canvasobject->SetPosXY( x, y );

        a2dPoint2D point;
        double dx, dy; //detect restriction distance of any point.

        if ( restrict->GetShiftKeyDown() )
        {
            restrict->RestrictPoint( x, y );
        }
        else
        {
            a2dSnapToWhatMask snapTo = m_snapTargetFeatures;
            a2dSnapToWhatMask was = restrict->GetSnapSourceFeatures();
            if ( m_snapSourceFeatures != a2dRestrictionEngine::snapToNone )
                restrict->SetSnapSourceFeatures( m_snapSourceFeatures );
            if ( snapTo == a2dRestrictionEngine::snapToNone )
                restrict->GetSnapTargetFeatures();

            if ( restrict->RestrictCanvasObjectAtVertexes( m_canvasobject, point, dx, dy, snapTo, true ) )
            {
                //restrict the object drawing to that point
                x += dx;
                y += dy;
                //wxLogDebug( wxT( "   dx =%12.6lf, dy =%12.6lf" ), dx, dy );
            }
            //wxLogDebug( wxT( "cx =%12.6lf, cy =%12.6lf" ), x, y );
            restrict->SetSnapSourceFeatures( was );
        }
        m_original->SetSnapTo( snapold );
    }

    // show the object according to the restricted position.
    m_canvasobject->SetPosXY( x, y );
    if ( m_stcontroller->GetDragMode() == wxDRAW_REDRAW )
    {
        m_original->SetPosXY( x, y );
    }

    if ( oldpos != m_canvasobject->GetPosXY() )
    {
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->GeneratePinsToConnectObject( GetDrawingPart(), m_parentobject, m_canvasobject );

        GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( false );
        m_pending = true;
    }
}

bool a2dDragTool::CreateToolObjects()
{
    //as a result of dragging ad rerouting, sometimes object connected to itself, now (also) correcting it.
	m_original->DisConnectWith( m_original, "", true );

	// Clone the original drag object
	a2dRefMap refs;
    m_canvasobject = m_original->TClone( clone_toDrag | clone_childs | clone_seteditcopy | clone_setoriginal | clone_members | clone_noCameleonRef, &refs );
    m_canvasobject->SetSelected( false );
    m_original->SetSnapTo( false );

    // Add the draged object clone and connected object clones to the parent
    AddEditobject( m_canvasobject );
    // Set the visibility of the original dragged object and the original connected wires
    m_original->SetVisible( m_renderOriginal );

    //drag is comming so first create wires where there are non.
    a2dCanvasObjectList dragList;
    dragList.push_back( m_original );
    PrepareForRewire( dragList, true, false, false, true, &refs );

	refs.LinkReferences();

    m_pending = true;
    return true;
}

void a2dDragTool::CleanupToolObjects()
{
    a2dStTool::CleanupToolObjects();
}

BEGIN_EVENT_TABLE( a2dDragOrgTool, a2dStTool )
    EVT_MOUSE_EVENTS( a2dDragOrgTool::OnMouseEvent )
    EVT_CHAR( a2dDragOrgTool::OnChar )
END_EVENT_TABLE()

a2dDragOrgTool::a2dDragOrgTool( a2dStToolContr* controller ): a2dStTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag );
    m_mousecaptured = false;
    m_deleteonoutsidedrop = false;

    m_anotate = false;
    m_lateconnect = true;

    //i like transparent fill and specific stroke
    SetFill( *a2dTRANSPARENT_FILL );
    SetStroke( a2dStroke( *wxRED, 1, a2dSTROKE_DOT ) );
}

a2dDragOrgTool::~a2dDragOrgTool()
{
}

void a2dDragOrgTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        double xw = m_xwprev;
        double yw = m_ywprev;

        double shiftx;
        double shifty;
        GetKeyIncrement( &shiftx, &shifty );

        //now shift with snap distances in X or Y
        switch( event.GetKeyCode() )
        {
            case WXK_UP:
                shiftx = 0;
                break;

            case WXK_DOWN:
                shiftx = 0;
                shifty = -shifty;
                break;

            case WXK_LEFT:
                shiftx = -shiftx;
                shifty = 0;
                break;

            case WXK_RIGHT:
                shifty = 0;
                break;
            default:
            {
                shiftx = 0;
                shifty = 0;
                event.Skip();
            }
        }

        m_xwprev = xw + shiftx;
        m_ywprev = yw + shifty;
        DragAbsolute( m_xwprev, m_ywprev );
        GetDrawingPart()->Update( a2dCANVIEW_UPDATE_SYNC_DRAWERS );
        GetDisplayWindow()->WarpPointer( GetDrawer2D()->WorldToDeviceX( m_xwprev ), GetDrawer2D()->WorldToDeviceY( m_ywprev ) );
    }
    else
        event.Skip();

}

void a2dDragOrgTool::Render()
{
    if ( m_original && GetBusy() )
    {
        a2dStTool::Render();
    }
}

void a2dDragOrgTool::AdjustShift( double* WXUNUSED( x ), double* WXUNUSED( y ) )
{
}

bool a2dDragOrgTool::StartDragging( int x, int y, a2dCanvasObject* original )
{
    m_x = x;
    m_y = y;
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );

    m_original = original;

    if ( !m_original || !m_original->GetDraggable() )
    {
        m_original = 0;
        return false;
    }

    m_transform = m_original->GetTransformMatrix();

    if ( !EnterBusyMode() )
        return false;

    m_startMousex = m_xwprev;
    m_startMousey = m_ywprev;

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
        restrict->SetRestrictPoint( m_original->GetPosX(), m_original->GetPosY() );
    return true;
}

void a2dDragOrgTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    }

    if ( event.Moving() && !GetBusy() )
    {
        a2dCanvasObject* hit = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev );
        if ( hit && hit->GetDraggable() )
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );
        else
            GetDrawingPart()->SetCursor( m_toolcursor );
    }

    if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dCanvasObject* original = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev, wxLAYER_ALL, a2dCANOBJHITOPTION_LAYERS );

        if ( !StartDragging( m_xwprev, m_ywprev, original ) )
            return;
    }
    else if ( event.LeftUp() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        FinishBusyMode();
    }
    else if ( event.RightDown() )
    {
        event.Skip();
    }
    else if ( GetBusy() )
    {
        DragAbsolute( m_startObjectx + m_xwprev - m_startMousex, m_startObjecty + m_ywprev - m_startMousey );
    }
    //!todo skip is needed if object should connect itself, in a2dCanvasObject::Update()
    //else
    //    event.Skip();
}


void a2dDragOrgTool::CaptureMouse()
{
    GetDrawingPart()->SetCorridor( m_canvasobject, m_canvasobject );
    GetDrawingPart()->GetDisplayWindow()->CaptureMouse();
    m_mousecaptured = true;
}

void a2dDragOrgTool::ReleaseMouse()
{
    if( m_mousecaptured )
    {
        GetDrawingPart()->SetCorridor( NULL, NULL );
        GetDrawingPart()->GetDisplayWindow()->ReleaseMouse();
    }
    m_mousecaptured = false;
}

bool a2dDragOrgTool::EnterBusyMode()
{
    if( !a2dStTool::EnterBusyMode() )
        return false;

    CaptureMouse();

    //the drag copy should not connect while dragging
    m_canvasobject->DoConnect( false );
    m_wasMayConnect = m_original->DoConnect();
    m_original->DoConnect( false );

    m_startObjectx = m_canvasobject->GetPosX();
    m_startObjecty = m_canvasobject->GetPosY();

    return true;
}

void a2dDragOrgTool::FinishBusyMode( bool closeCommandGroup )
{
    //restore connect backup
    m_original->DoConnect( m_wasMayConnect );

    if(
        m_deleteonoutsidedrop &&
        (
            m_xwprev < GetDrawer2D()->GetVisibleMinX() ||
            m_xwprev > GetDrawer2D()->GetVisibleMaxX() ||
            m_ywprev < GetDrawer2D()->GetVisibleMinY() ||
            m_ywprev > GetDrawer2D()->GetVisibleMaxY()
        )
    )
    {
        // the object was dropped outside of the window
        // The code below is copied from a2dDeleteTool
        m_original->SetPending( true );
        m_original->SetVisible( false );

        //redraw in buffer what should be there without this object
        GetDrawing()->AddPendingUpdatesOldNew();

        //delete object from document (actually move to command for redo)
        GetCanvasCommandProcessor()->Submit(
            new a2dCommand_ReleaseObject( m_parentobject, m_original ) );
    }
    else
    {
        //the next has internal the functionality to connect object pins to objectpins which now can connect.
        GetCanvasCommandProcessor()->Submit(
            new a2dCommand_SetCanvasProperty( m_original, a2dCanvasObject::PROPID_TransformMatrix, m_canvasobject->GetTransformMatrix() )
        );

        GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );
    }

    ReleaseMouse();

    a2dStTool::FinishBusyMode( closeCommandGroup );
}

void a2dDragOrgTool::AbortBusyMode()
{
    ReleaseMouse();

    a2dStTool::AbortBusyMode();
}

void a2dDragOrgTool::DragAbsolute( double x, double y )
{
    a2dPoint2D oldpos = m_canvasobject->GetPosXY();

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        //temporarily restore position according to the mouse position, which is the unrestricted position.
        m_canvasobject->SetPosXY( x, y );

        a2dPoint2D point;
        double dx, dy; //detect restriction distance of any point.
        if ( restrict->RestrictCanvasObjectAtVertexes( m_canvasobject, point, dx, dy ) )
        {
            //restrict the object drawing to that point
            x += dx;
            y += dy;
        }
    }

    // show the object according to the restricted position.
    m_canvasobject->SetPosXY( x, y );
    if ( m_stcontroller->GetDragMode() == wxDRAW_REDRAW )
        m_original->SetPosXY( x, y );

    if( oldpos != m_canvasobject->GetPosXY() )
    {
        a2dIterC ic( GetDrawingPart() );
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->GeneratePinsToConnectObject( GetDrawingPart(), m_parentobject, m_canvasobject );

        GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( false );

        m_pending = true;
    }
}

bool a2dDragOrgTool::CreateToolObjects()
{
    // Clone the original drag object
    m_canvasobject = m_original;
    m_canvasobject->SetSelected( false );
    m_original->SetSnapTo( false );

	a2dRefMap refs;
    //drag is comming so first create wires where there are non.
    a2dCanvasObjectList dragList;
    dragList.push_back( m_original );
    PrepareForRewire( dragList, true, false, false, true, &refs );
	refs.LinkReferences();

    m_pending = true;

    return true;
}


void a2dDragOrgTool::CleanupToolObjects()
{
    a2dStTool::CleanupToolObjects();
}

DEFINE_MENU_ITEMID( CmdMenu_NoDragMenu, wxTRANSLATE("Drag the new object"), wxTRANSLATE("Drag new object in place") )

BEGIN_EVENT_TABLE( a2dDragNewTool, a2dDragTool )
    EVT_ENTER_WINDOW( a2dDragNewTool::OnEnter )
    EVT_MOUSE_EVENTS( a2dDragNewTool::OnMouseEvent )
    EVT_COM_EVENT( a2dDragNewTool::OnComEvent )
    EVT_CHAR( a2dDragNewTool::OnChar )
END_EVENT_TABLE()

a2dDragNewTool::a2dDragNewTool( a2dStToolContr* controller, a2dCanvasObject* newObject, const a2dMenuIdItem& initiatingMenuId )
    : a2dDragTool( controller )
{
    m_initiatingMenuId = &initiatingMenuId;  
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();
    m_newObject = newObject;
    m_boxCenterDrag = false;
}

a2dDragNewTool::~a2dDragNewTool()
{
}

bool a2dDragNewTool::RotateObject90LeftRight( bool right ) 
{ 
    if ( GetBusy() )
    {
        a2dAffineMatrix mat = m_canvasobject->GetTransform();
        if ( right )
			mat.Rotate( 90, m_canvasobject->GetPosX(), m_canvasobject->GetPosY() );
        else 
            mat.Rotate( -90, m_canvasobject->GetPosX(), m_canvasobject->GetPosY() );
        m_canvasobject->SetTransform( mat );
        a2dAffineMatrix matn = m_newObject->GetTransform();
        matn.Rotate( 90, m_newObject->GetPosX(), m_newObject->GetPosY() );
        m_newObject->SetTransform( matn );
    }
    return true; 
}

void a2dDragNewTool::OnComEvent( a2dComEvent& event )
{
    if ( GetBusy() )
    {
        if ( event.GetId() == a2dStTool::sig_toolBeforePush )
        {
            a2dProperty* property = ( a2dProperty* ) event.GetProperty();
            a2dStTool* newtool = wxStaticCast( property->GetRefObject(), a2dStTool );
            if ( GetBusy() )
            {
                if ( !wxDynamicCast( newtool, a2dZoomTool ) )
                {
                    FinishBusyMode();
                }
                else
                {
                    m_halted = true;
                }
            }
            SetPending( true );
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

void a2dDragNewTool::OnEnter( wxMouseEvent& event )
{
    if ( GetDrawingPart()->GetDisplayWindow() && !GetDrawingPart()->GetDisplayWindow()->HasFocus() )
        GetDrawingPart()->GetDisplayWindow()->SetFocus();
    event.Skip();
}

void a2dDragNewTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case '9':
            {
                //if ( event.m_controlDown )
                RotateObject90LeftRight( true );
                break;
            }
            default:
            {
                event.Skip();
            }
        }
    }
    else
        event.Skip();

}

void a2dDragNewTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw, yw;
    MouseToToolWorld( m_x, m_y, xw, yw );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    }

    if( !GetBusy() ) 
    {
        //m_original = m_newObject->TClone( clone_deep );
        //! todo Object having reference sometimes better not deep.
        m_original = m_newObject->TClone( clone_deep  | clone_noCameleonRef | clone_noReference );

		a2dBoundingBox box = m_original->GetBbox();
        if ( m_boxCenterDrag )
            m_original->SetPosXY( xw - box.GetMinX() - box.GetWidth()/2.0 , yw - box.GetMinY() - box.GetHeight()/2.0 );
        else
            m_original->SetPosXY( xw, yw );
        m_original->SetRoot( GetDrawing() );

        // AddCurrent adjusts the objects style and submits an a2dCommand_AddObject
        m_parentAddedTo = m_parentobject;

        m_original->SetParent( m_parentAddedTo );

        if ( !EnterBusyMode() )
            return;

        m_transform = m_original->GetTransformMatrix();

        m_xwprev = xw;
        m_ywprev = yw;
        m_startMousex = xw;
        m_startMousey = yw;
    }
    else if ( event.LeftDown() )
    {
        // don't process this in the base class
    }
    else if ( event.RightDown() )
    {
        event.Skip();
    }
    else if ( event.LeftUp() )
    {
        a2dDragTool::OnMouseEvent( event );
        GetCanvasCommandProcessor()->SetCurrentCanvasObject( 0 );
    }
    else
    {
        // all the rest is as with a usual drag
        a2dDragTool::OnMouseEvent( event );
    }
}

void a2dDragNewTool::AdjustRenderOptions()
{
    // we generally don't want the original to be visible, regardless of the mode
    // this is because the original was cloned from the template object, and is not a
    // true original object
    m_renderOriginal = false;
}

void a2dDragNewTool::DoStopTool( bool WXUNUSED( abort ) )
{
    // a a2dDragNewTool can be stopped, even if it is busy. This is because the
    // tool can be busy with mouse up.
    if( GetBusy() )
    {
        wxASSERT( m_original );
        AbortBusyMode();
    }

    if ( m_commandgroup )
        CloseCommandGroup();
}

bool a2dDragNewTool::EnterBusyMode()
{
    if( !a2dDragTool::EnterBusyMode() )
        return false;

    // we don't want the mouse to be captured, so undo this
    ReleaseMouse();

    // Note: there is no need to overload Finish/AbortBusyMode,
    // because the base class stores if it did capture the mouse

    return true;
}

void a2dDragNewTool::FinishBusyMode( bool closeCommandGroup )
{
    //restore connect backup
    m_original->DoConnect( m_wasMayConnect );

    //the next has internal the functionality to connect object pins to objectpins which now can connect.
    GetCanvasCommandProcessor()->Submit(
        new a2dCommand_SetCanvasProperty( m_original, a2dCanvasObject::PROPID_TransformMatrix, m_canvasobject->GetTransformMatrix() )
    );
    AddCurrent( m_original );

    if ( m_lateconnect )
        m_original->ReWireConnected( m_parentobject, true );

    if ( m_stcontroller->GetSelectAtEnd() )
    {
        DeselectAll();
        m_original->SetSelected( true );
        m_original->SetSelected2( true );
    }
    if ( m_stcontroller->GetSelectionStateUndo() ) 
    {
	    GetCanvasCommandProcessor()->Submit( new a2dCommandMasked( GetDrawingPart()->GetShowObject(), GetCommandgroup(), 
			    								a2dCanvasOFlags::SELECTED, a2dCanvasOFlags::SELECTED2 ) );
    }

    GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );
    //ReleaseMouse();

    a2dStTool::FinishBusyMode();
}


BEGIN_EVENT_TABLE( a2dDeleteTool, a2dStTool )
    EVT_CHAR( a2dDeleteTool::OnChar )
    EVT_MOUSE_EVENTS( a2dDeleteTool::OnMouseEvent )
END_EVENT_TABLE()

a2dDeleteTool::a2dDeleteTool( a2dStToolContr* controller, a2dCanvasObjectFlagsMask whichobjects )
    :
    a2dStTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_BULLSEYE );
    m_canvasobject = 0;
    m_whichobjects = whichobjects;
}

void a2dDeleteTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_SPACE:
            {
                if( m_canvasobject )
                {
                    OpenCommandGroup( false );
                    //delete object from document (actually move to command for redo)
                    GetCanvasCommandProcessor()->Submit(
                        new a2dCommand_ReleaseObject( m_parentobject, m_canvasobject ) );
                    m_canvasobject->SetVisible( true );
                    FinishBusyMode();
                }
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();

}

void a2dDeleteTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    int x = event.GetX();
    int y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw, yw;
    MouseToToolWorld( x, y, xw, yw );

    if ( event.LeftDown() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        m_canvasobject = GetDrawingPart()->IsHitWorld( xw, yw, wxLAYER_ALL, a2dCANOBJHITOPTION_LAYERS );

        if ( !m_canvasobject )
        {
            m_canvasobject = 0;
            event.Skip();
            if ( m_oneshot )
                StopTool();
            return;
        }

        if( !m_canvasobject->CheckMask( m_whichobjects ) )
        {
            if ( m_oneshot )
                StopTool();
            return;
        }

        if ( !EnterBusyMode() )
            return;

        m_canvasobject->SetPending( true );
        m_canvasobject->SetVisible( false );

        //redraw in buffer what should be there without this object
        GetDrawing()->AddPendingUpdatesOldNew();

        //delete object from document (actually move to command for redo)
        GetCanvasCommandProcessor()->Submit(
            new a2dCommand_ReleaseObject( m_parentobject, m_canvasobject ) );

        GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( GetDrawingPart()->GetShowObject() );

        //make visible on undo stack
        m_canvasobject->SetVisible( true );
        FinishBusyMode();
    }
    else
        event.Skip();

}

BEGIN_EVENT_TABLE( a2dDrawCircleTool, a2dStDrawTool )
    EVT_IDLE( a2dDrawCircleTool::OnIdle )
    EVT_CHAR( a2dDrawCircleTool::OnChar )
    EVT_MOUSE_EVENTS( a2dDrawCircleTool::OnMouseEvent )
END_EVENT_TABLE()

a2dDrawCircleTool::a2dDrawCircleTool( a2dStToolContr* controller ): a2dStDrawTool( controller )
{
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );
}

void a2dDrawCircleTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStDrawTool::AddAnotationToPendingUpdateArea();

    //a2dCircle* cir = wxStaticCast( m_canvasobject.Get(), a2dCircle );

    m_anotation.Printf( _T( "radius %6.3f" ), m_radius );

    wxCoord w, h, descent, external;
    GetTextExtent( m_anotation, &w, &h, &descent, &external );

    m_xanotation = m_x + h;
    m_yanotation = m_y + h;

    a2dStDrawTool::AddAnotationToPendingUpdateArea();
}


void a2dDrawCircleTool::OnIdle( wxIdleEvent& event )
{
    if ( m_pending && GetBusy() )
    {
        RemoveAllDecorations();

        a2dCircle* cir = wxStaticCast( m_canvasobject.Get(), a2dCircle );
        a2dSLine* ll = new a2dSLine( cir->GetPosX(), cir->GetPosY(), m_xwprev, m_ywprev );
        ll->SetStroke( *wxRED, 0, a2dSTROKE_DOT );

        a2dCircle* zero = new a2dCircle( m_start_x, m_start_y, GetDrawer2D()->DeviceToWorldXRel( 3 ) );
        zero->SetFill( *wxRED );

        a2dRect* around = new a2dRect( cir->GetBboxMinX(), cir->GetBboxMinY(), cir->GetBboxWidth(), cir->GetBboxHeight() );
        around->SetStroke( *wxRED, 0, a2dSTROKE_LONG_DASH );
        around->SetFill( *a2dTRANSPARENT_FILL );

        AddDecorationObject( around );
        AddDecorationObject( ll );
        AddDecorationObject( zero );

        event.Skip();
    }
    else
        event.Skip();
}

void a2dDrawCircleTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_TAB:
            {
                m_mode++;
                SetMode( m_mode );
            }
            break;

            default:
                event.Skip();
        }
    }
    else
        event.Skip();

}

void a2dDrawCircleTool::SetMode( int mode )
{
    m_mode = mode;
    if ( m_mode > 2 ) m_mode = 0;

    if ( !m_canvasobject )
        return;

    a2dCircle* cir = wxStaticCast( m_canvasobject.Get(), a2dCircle );
    switch ( m_mode )
    {
        case 0:
        {
            double dx, dy;

            if ( fabs( m_xwprev - m_start_x ) > fabs( m_ywprev - m_start_y ) )
            {
                dx = ( m_xwprev - m_start_x ) / 2 ;
                m_radius = fabs( dx );
                if ( m_ywprev > m_start_y )
                    dy = m_radius;
                else
                    dy = -m_radius;
            }
            else
            {
                dy = ( m_ywprev - m_start_y ) / 2 ;
                m_radius = fabs( dy );
                if ( m_xwprev > m_start_x )
                    dx = m_radius;
                else
                    dx = -m_radius;
            }
            cir->SetPosXY( m_start_x + dx , m_start_y + dy );
            break;
        }
        case 1:
        {
            cir->SetPosXY( m_start_x, m_start_y );
            m_radius = sqrt( pow( m_xwprev - cir->GetPosX(), 2 ) + pow( m_ywprev - cir->GetPosY(), 2 ) );
            break;
        }
        case 2:
        {
            cir->SetPosXY( ( m_xwprev + m_start_x ) / 2, ( m_ywprev + m_start_y ) / 2 );
            m_radius = sqrt( pow( m_xwprev - m_start_x, 2 ) + pow( m_ywprev - m_start_y, 2 ) ) / 2;
            break;
        }
        default:
            break;
    }

    cir->SetRadius( m_radius );
    a2dCircle* cirorg = wxStaticCast( m_original.Get(), a2dCircle );
    cirorg->SetPosXY( cir->GetPosX(),  cir->GetPosY() );
    cirorg->SetRadius( m_radius );

	double unitScale = GetDrawing()->GetUnitsScale();
	wxString state;
	wxString form = m_stcontroller->GetFormat() + " " + m_stcontroller->GetFormat();
	state.Printf( form, cir->GetPosX()*unitScale, cir->GetPosY()*unitScale );
	SetStateString( state, 10 );
	form = _T("radius = ") + m_stcontroller->GetFormat();
	state.Printf( form, m_radius*unitScale );
	SetStateString( state, 11 );

    m_pending = true;
}

void a2dDrawCircleTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    }

    if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        m_radius = 0;
        m_original = new a2dCircle( m_xwprev, m_ywprev, m_radius );
        m_original->SetContourWidth( m_contourwidth );
        m_original->SetStroke( m_stroke );
        m_original->SetFill( m_fill );
        m_original->SetLayer( m_layer );

        m_start_x = m_xwprev;
        m_start_y = m_ywprev;

        if ( !EnterBusyMode() )
            return;
        AddCurrent( m_original );
    }
    else if ( event.LeftDown() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dGeneralGlobals->RecordF( this, _T( "circle %f %f %f" ), m_canvasobject->GetPosX(), m_canvasobject->GetPosY(), m_radius );

        a2dCircle* cirorg = wxStaticCast( m_original.Get(), a2dCircle );
        a2dCircle* cir = wxStaticCast( m_canvasobject.Get(), a2dCircle );
        cirorg->SetPosXY( cir->GetPosX(),  cir->GetPosY() );
        cirorg->SetRadius( m_radius );

        StartEditingOrFinish();
    }
    else  if ( ( event.Dragging() && GetBusy() ) || ( event.Moving() && GetBusy() ) )
    {
        SetMode( m_mode );
    }
    else
        event.Skip();
}


BEGIN_EVENT_TABLE( a2dDrawEllipseTool, a2dStDrawTool )
    EVT_IDLE( a2dDrawEllipseTool::OnIdle )
    EVT_CHAR( a2dDrawEllipseTool::OnChar )
    EVT_MOUSE_EVENTS( a2dDrawEllipseTool::OnMouseEvent )
END_EVENT_TABLE()

a2dDrawEllipseTool::a2dDrawEllipseTool( a2dStToolContr* controller ): a2dStDrawTool( controller )
{
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );
}

void a2dDrawEllipseTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStDrawTool::AddAnotationToPendingUpdateArea();

    a2dEllipse* ellip = wxStaticCast( m_canvasobject.Get(), a2dEllipse );

    m_anotation.Printf( _T( "w %6.3f h %6.3f" ), ellip->GetWidth(), ellip->GetHeight() );

    wxCoord w, h, descent, external;
    GetTextExtent( m_anotation, &w, &h, &descent, &external );

    m_xanotation = m_x + h;
    m_yanotation = m_y + h;

    a2dStDrawTool::AddAnotationToPendingUpdateArea();
}

void a2dDrawEllipseTool::OnIdle( wxIdleEvent& event )
{
    if ( m_pending && GetBusy() )
    {
        RemoveAllDecorations();

        a2dEllipse* ellip = wxStaticCast( m_canvasobject.Get(), a2dEllipse );
        a2dSLine* ll = new a2dSLine( ellip->GetPosX(), ellip->GetPosY(), m_xwprev, m_ywprev );
        ll->SetStroke( *wxRED, 0, a2dSTROKE_DOT );

        a2dCircle* zero = new a2dCircle( m_start_x, m_start_y, GetDrawer2D()->DeviceToWorldXRel( 3 ) );
        zero->SetFill( *wxRED );

        a2dRect* around = new a2dRect( ellip->GetBboxMinX(), ellip->GetBboxMinY(), ellip->GetBboxWidth(), ellip->GetBboxHeight() );
        around->SetStroke( *wxRED, 0, a2dSTROKE_LONG_DASH );
        around->SetFill( *a2dTRANSPARENT_FILL );

        AddDecorationObject( around );
        AddDecorationObject( ll );
        AddDecorationObject( zero );

        event.Skip();
    }
    else
        event.Skip();
}

void a2dDrawEllipseTool::SetMode( int mode )
{
    m_mode = mode;
    if ( m_mode > 1 ) m_mode = 0;

    if ( !m_canvasobject )
        return;

    a2dEllipse* ellip = wxStaticCast( m_canvasobject.Get(), a2dEllipse );

    switch ( m_mode )
    {
        case 0:
        {
            ellip->SetPosXY( m_start_x, m_start_y );

            //modify object
            double xs, ys;
            xs = m_xwprev - ellip->GetPosX();
            ys = m_ywprev - ellip->GetPosY();

            ellip->SetWidth( fabs( xs * 2 ) );
            ellip->SetHeight( fabs( ys * 2 ) );

            break;
        }
        case 1:
        {
            ellip->SetPosXY( ( m_xwprev + m_start_x ) / 2, ( m_ywprev + m_start_y ) / 2 );

            //modify object
            double xs, ys;
            xs = m_xwprev - ellip->GetPosX();
            ys = m_ywprev - ellip->GetPosY();

            ellip->SetWidth( fabs( xs * 2 ) );
            ellip->SetHeight( fabs( ys * 2 ) );
            break;
        }
        default:
            break;
    }

	double unitScale = GetDrawing()->GetUnitsScale();
	wxString state;
	wxString form = m_stcontroller->GetFormat() + " " + m_stcontroller->GetFormat();
	state.Printf( form, ellip->GetPosX()*unitScale, ellip->GetPosY()*unitScale );
	SetStateString( state, 10 );
	form = _T("width = ") + m_stcontroller->GetFormat() + _T(" height = ") + m_stcontroller->GetFormat();
	state.Printf( form, ellip->GetWidth()*unitScale, ellip->GetHeight()*unitScale );
	SetStateString( state, 11 );


    m_pending = true;
}

void a2dDrawEllipseTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_TAB:
            {
                m_mode++;
                SetMode( m_mode );
            }
            break;

            default:
                event.Skip();
        }
    }
    else
        event.Skip();

}

void a2dDrawEllipseTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    }
    if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        m_original = new a2dEllipse( m_xwprev, m_ywprev, 0, 0 );
        m_start_x = m_xwprev;
        m_start_y = m_ywprev;
        a2dEllipse* ellip = wxStaticCast( m_original.Get(), a2dEllipse );
        ellip->SetRoot( GetDrawing() );
        if ( !EnterBusyMode() )
            return;
        AddCurrent( m_original );
    }
    else if ( event.LeftDown() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dEllipse* ellip = wxStaticCast( m_canvasobject.Get(), a2dEllipse );
        a2dGeneralGlobals->RecordF( this, _T( "ellipse %f %f %f %f" ), ellip->GetPosX(), ellip->GetPosY(), ellip->GetWidth(), ellip->GetHeight() );

        a2dEllipse* elliporg = wxStaticCast( m_original.Get(), a2dEllipse );
        elliporg->SetPosXY( ellip->GetPosX(), ellip->GetPosY() );
        elliporg->SetWidth( ellip->GetWidth() );
        elliporg->SetHeight( ellip->GetHeight() );

        StartEditingOrFinish();
    }
    else  if ( ( event.Dragging() && GetBusy() ) || ( event.Moving() && GetBusy() ) )
    {
        SetMode( m_mode );
    }
    else
        event.Skip();
}

BEGIN_EVENT_TABLE( a2dDrawEllipticArcTool, a2dStDrawTool )
    EVT_IDLE( a2dDrawEllipticArcTool::OnIdle )
    EVT_CHAR( a2dDrawEllipticArcTool::OnChar )
    EVT_MOUSE_EVENTS( a2dDrawEllipticArcTool::OnMouseEvent )
END_EVENT_TABLE()

a2dDrawEllipticArcTool::a2dDrawEllipticArcTool( a2dStToolContr* controller, a2dEllipticArc* templateobject ): a2dStDrawTool( controller )
{
    m_mode2 = 0;
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );
    m_arcstart = 45;
    m_arcend = 315;
    m_chord = false;

    m_templateobject = templateobject;
}

void a2dDrawEllipticArcTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStDrawTool::AddAnotationToPendingUpdateArea();

    a2dEllipticArc* ellip = wxStaticCast( m_canvasobject.Get(), a2dEllipticArc );

    m_anotation.Printf( _T( "w %6.3f h %6.3f" ), ellip->GetWidth(), ellip->GetHeight() );

    wxCoord w, h, descent, external;
    GetTextExtent( m_anotation, &w, &h, &descent, &external );

    m_xanotation = m_x + h;
    m_yanotation = m_y + h;

    a2dStDrawTool::AddAnotationToPendingUpdateArea();
}

void a2dDrawEllipticArcTool::OnIdle( wxIdleEvent& event )
{
    if ( m_pending && GetBusy() )
    {
        RemoveAllDecorations();

        a2dEllipticArc* ellip = wxStaticCast( m_canvasobject.Get(), a2dEllipticArc );
        a2dSLine* ll = new a2dSLine( ellip->GetPosX(), ellip->GetPosY(), m_xwprev, m_ywprev );
        ll->SetStroke( *wxRED, 0, a2dSTROKE_DOT );

        a2dCircle* zero = new a2dCircle( m_start_x, m_start_y, GetDrawer2D()->DeviceToWorldXRel( 3 ) );
        zero->SetFill( *wxRED );

        a2dRect* around = new a2dRect( ellip->GetBboxMinX(), ellip->GetBboxMinY(), ellip->GetBboxWidth(), ellip->GetBboxHeight() );
        around->SetStroke( *wxRED, 0, a2dSTROKE_LONG_DASH );
        around->SetFill( *a2dTRANSPARENT_FILL );

        AddDecorationObject( ll );
        AddDecorationObject( around );
        AddDecorationObject( zero );

        event.Skip();
    }
    else
        event.Skip();
}

void a2dDrawEllipticArcTool::SetMode( int mode )
{
    m_mode = mode;
    if ( m_mode > 1 ) m_mode = 0;

    if ( !m_canvasobject )
        return;

    a2dEllipticArc* ellip = wxStaticCast( m_canvasobject.Get(), a2dEllipticArc );

    switch ( m_mode )
    {
        case 0:
        {
            ellip->SetPosXY( m_start_x, m_start_y );

            //modify object
            double xs, ys;
            xs = m_xwprev - ellip->GetPosX();
            ys = m_ywprev - ellip->GetPosY();

            ellip->SetWidth( fabs( xs * 2 ) );
            ellip->SetHeight( fabs( ys * 2 ) );
            break;
        }
        case 1:
        {
            ellip->SetPosXY( ( m_xwprev + m_start_x ) / 2, ( m_ywprev + m_start_y ) / 2 );

            //modify object
            double xs, ys;
            xs = m_xwprev - ellip->GetPosX();
            ys = m_ywprev - ellip->GetPosY();

            ellip->SetWidth( fabs( xs * 2 ) );
            ellip->SetHeight( fabs( ys * 2 ) );
            break;
        }
        default:
            break;
    }

    a2dEllipticArc* elliporg = wxStaticCast( m_original.Get(), a2dEllipticArc );
    elliporg->SetWidth( ellip->GetWidth() );
    elliporg->SetHeight( ellip->GetHeight() );
    elliporg->SetPosXY( ellip->GetPosX(), ellip->GetPosY() );

	double unitScale = GetDrawing()->GetUnitsScale();
	wxString state;
	wxString form = m_stcontroller->GetFormat() + " " + m_stcontroller->GetFormat();
	state.Printf( form, ellip->GetPosX()*unitScale, ellip->GetPosY()*unitScale );
	SetStateString( state, 10 );
	form = _T("width = ") + m_stcontroller->GetFormat() + _T(" height = ") + m_stcontroller->GetFormat();
	state.Printf( form, ellip->GetWidth()*unitScale, ellip->GetHeight()*unitScale );
	SetStateString( state, 11 );

    m_pending = true;
}

void a2dDrawEllipticArcTool::SetMode2( int mode )
{
    m_mode2 = mode;
    if ( m_mode2 > 2 ) m_mode2 = 0;

    if ( !m_canvasobject )
        return;

    m_pending = true;
}

void a2dDrawEllipticArcTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_SPACE:
            {
                a2dEllipticArc* ellip = wxStaticCast( m_canvasobject.Get(), a2dEllipticArc );
                a2dGeneralGlobals->RecordF( this, _T( "ellipticArc %f %f% f %f %f %f" ),
                                            ellip->GetPosX(), ellip->GetPosY(),
                                            ellip->GetWidth(), ellip->GetHeight(),
                                            ellip->GetStart(), ellip->GetEnd()
                                          );
                StartEditingOrFinish();
            }
            break;
            case WXK_TAB:
            {
                m_mode++;
                SetMode( m_mode );
            }
            break;

            case WXK_RETURN:
            {
                m_mode2++;
                SetMode2( m_mode2 );
            }
            break;

            default:
                event.Skip();
        }
    }
    else
        event.Skip();

}


void a2dDrawEllipticArcTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    }

    if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dEllipticArc* ellip = 0;
        if ( m_templateobject )
        {
            m_original = m_templateobject->TClone( clone_childs );
            ellip = wxStaticCast( m_original.Get(), a2dEllipticArc );
            ellip->SetStart( m_arcstart );
            ellip->SetEnd( m_arcend );
            m_original->Translate( m_xwprev, m_ywprev );
            m_original->EliminateMatrix();
        }
        else
        {
            m_original = new a2dEllipticArc( m_xwprev, m_ywprev, 0, 0, m_arcstart, m_arcend );
            ellip = wxStaticCast( m_original.Get(), a2dEllipticArc );
            ellip->SetChord( m_chord );
        }
        if ( !EnterBusyMode() )
            return;
        AddCurrent( m_original );
        m_start_x = m_xwprev;
        m_start_y = m_ywprev;
    }
    else if ( event.LeftDown() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        m_mode2 = 0;

        a2dEllipticArc* ellip = wxStaticCast( m_canvasobject.Get(), a2dEllipticArc );
        a2dGeneralGlobals->RecordF( this, _T( "ellipticArc %f %f %f %f %f %f" ),
                                    ellip->GetPosX(), ellip->GetPosY(),
                                    ellip->GetWidth(), ellip->GetHeight(),
                                    ellip->GetStart(), ellip->GetEnd()
                                  );

        a2dEllipticArc* elliporg = wxStaticCast( m_original.Get(), a2dEllipticArc );
        elliporg->SetWidth( ellip->GetWidth() );
        elliporg->SetHeight( ellip->GetHeight() );
        elliporg->SetPosXY( ellip->GetPosX(), ellip->GetPosY() );

        StartEditingOrFinish();
    }
    else  if ( ( event.Dragging() && GetBusy() ) || ( event.Moving() && GetBusy() ) )
    {
        a2dEllipticArc* ellip = wxStaticCast( m_canvasobject.Get(), a2dEllipticArc );
        //modify object
        double xs, ys;
        xs = m_xwprev - ellip->GetPosX();
        ys = m_ywprev - ellip->GetPosY();

        if ( m_mode2 == 1 )
        {
            m_arcstart =  wxRadToDeg( atan2( ys, xs ) );
            ellip->SetStart( m_arcstart );
        }
        else if (  m_mode2 == 2 )
        {
            m_arcend =  wxRadToDeg( atan2( ys, xs ) );
            ellip->SetEnd( m_arcend );
        }
        else
        {
            SetMode( m_mode );
        }
        m_pending = true;
    }
    else
        event.Skip();
}

BEGIN_EVENT_TABLE( a2dDrawArcTool, a2dStDrawTool )
    EVT_IDLE( a2dDrawArcTool::OnIdle )
    EVT_CHAR( a2dDrawArcTool::OnChar )
    EVT_MOUSE_EVENTS( a2dDrawArcTool::OnMouseEvent )
END_EVENT_TABLE()

a2dDrawArcTool::a2dDrawArcTool( a2dStToolContr* controller ): a2dStDrawTool( controller )
{
    m_mode2 = 0;
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );
    m_arcstart = 45;
    m_arcend   = 315;
    m_chord = false;
}

void a2dDrawArcTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStDrawTool::AddAnotationToPendingUpdateArea();

    a2dArc* arc = wxStaticCast( m_canvasobject.Get(), a2dArc );

    m_anotation.Printf( _T( "radius %6.3f" ), arc->GetRadius() );

    wxCoord w, h, descent, external;
    GetTextExtent( m_anotation, &w, &h, &descent, &external );

    m_xanotation = m_x + h;
    m_yanotation = m_y + h;

    a2dStDrawTool::AddAnotationToPendingUpdateArea();
}

void a2dDrawArcTool::OnIdle( wxIdleEvent& event )
{
    if ( m_pending && GetBusy() )
    {
        RemoveAllDecorations();

        a2dArc* cir = wxStaticCast( m_canvasobject.Get(), a2dArc );
        a2dSLine* ll = new a2dSLine( cir->GetPosX(), cir->GetPosY(), m_xwprev, m_ywprev );
        ll->SetStroke( *wxRED, 0, a2dSTROKE_DOT );

        a2dCircle* zero = new a2dCircle( m_start_x, m_start_y, GetDrawer2D()->DeviceToWorldXRel( 3 ) );
        zero->SetFill( *wxRED );

        a2dRect* around = new a2dRect( cir->GetBboxMinX(), cir->GetBboxMinY(), cir->GetBboxWidth(), cir->GetBboxHeight() );
        around->SetStroke( *wxRED, 0, a2dSTROKE_LONG_DASH );
        around->SetFill( *a2dTRANSPARENT_FILL );

        AddDecorationObject( ll );
        AddDecorationObject( around );
        AddDecorationObject( zero );

        event.Skip();
    }
    else
        event.Skip();
}

void a2dDrawArcTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_SPACE:
            {
                a2dArc* arc = wxStaticCast( m_canvasobject.Get(), a2dArc );
                a2dGeneralGlobals->RecordF( this, _T( "arc %f %f %f %f %f" ),
                                            arc->GetPosX(), arc->GetPosY(),
                                            arc->GetRadius(),
                                            m_arcstart, m_arcend );
                StartEditingOrFinish();
            }
            break;
            case WXK_TAB:
            {
                m_mode++;
                SetMode( m_mode );
            }
            break;

            case WXK_RETURN:
            {
                m_mode2++;
                SetMode2( m_mode2 );
            }
            break;

            default:
                event.Skip();
        }
    }
    else
        event.Skip();

}

void a2dDrawArcTool::SetMode( int mode )
{
    m_mode = mode;
    if ( m_mode > 2 ) m_mode = 0;

    if ( !m_canvasobject )
        return;

    a2dArc* cir = wxStaticCast( m_canvasobject.Get(), a2dArc );
    double radius = 0;
    switch ( m_mode )
    {
        case 0:
        {
            double dx, dy;

            if ( fabs( m_xwprev - m_start_x ) > fabs( m_ywprev - m_start_y ) )
            {
                dx = ( m_xwprev - m_start_x ) / 2 ;
                radius = fabs( dx );
                if ( m_ywprev > m_start_y )
                    dy = radius;
                else
                    dy = -radius;
            }
            else
            {
                dy = ( m_ywprev - m_start_y ) / 2 ;
                radius = fabs( dy );
                if ( m_xwprev > m_start_x )
                    dx = radius;
                else
                    dx = -radius;
            }
            cir->SetPosXY( m_start_x + dx , m_start_y + dy );
            break;
        }
        case 1:
        {
            cir->SetPosXY( m_start_x, m_start_y );
            radius = sqrt( pow( m_xwprev - cir->GetPosX(), 2 ) + pow( m_ywprev - cir->GetPosY(), 2 ) );
            break;
        }
        case 2:
        {
            cir->SetPosXY( ( m_xwprev + m_start_x ) / 2, ( m_ywprev + m_start_y ) / 2 );
            radius = sqrt( pow( m_xwprev - m_start_x, 2 ) + pow( m_ywprev - m_start_y, 2 ) ) / 2;
            break;
        }
        default:
            break;
    }

    cir->SetRadius( radius );
    a2dArc* cirorg = wxStaticCast( m_original.Get(), a2dArc );
    cirorg->SetPosXY( cir->GetPosX(),  cir->GetPosY() );
    cirorg->SetRadius( radius );

	double unitScale = GetDrawing()->GetUnitsScale();
	wxString state;
	wxString form = m_stcontroller->GetFormat() + " " + m_stcontroller->GetFormat();
	state.Printf( form, cir->GetPosX()*unitScale, cir->GetPosY()*unitScale );
	SetStateString( state, 10 );
	form = _T("radius = ") + m_stcontroller->GetFormat();
	state.Printf( form, radius*unitScale );
	SetStateString( state, 11 );

    m_pending = true;
}


void a2dDrawArcTool::SetMode2( int mode )

{
    m_mode2 = mode;
    if ( m_mode2 > 2 ) m_mode2 = 0;

    if ( !m_canvasobject )
        return;

    m_pending = true;
}


void a2dDrawArcTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    }

    if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        double radius = 0.00001; //needs to be something to initialize properly on angle
        m_original = new a2dArc( m_xwprev, m_ywprev, radius, m_arcstart, m_arcend );
        a2dArc* arc = wxStaticCast( m_original.Get(), a2dArc );
        m_start_x = m_xwprev;
        m_start_y = m_ywprev;
        arc->SetChord( m_chord );
        if ( !EnterBusyMode() )
            return;
        AddCurrent( m_original );
    }
    else if ( event.LeftDown() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        m_mode2 = 0;
        a2dArc* arc = wxStaticCast( m_canvasobject.Get(), a2dArc );
        a2dGeneralGlobals->RecordF( this, _T( "arc %f %f %f %f %f" ),
                                    arc->GetPosX(), arc->GetPosY(),
                                    arc->GetRadius(),
                                    m_arcstart, m_arcend );

        a2dArc* arcorg = wxStaticCast( m_original.Get(), a2dArc );
        arcorg->SetPosXY( arc->GetPosX(),  arc->GetPosY() );
        arcorg->SetRadius( arc->GetRadius() );

        StartEditingOrFinish();
    }
    else  if ( ( event.Dragging() && GetBusy() ) || ( event.Moving() && GetBusy() ) )
    {
        a2dArc* arc = wxStaticCast( m_canvasobject.Get(), a2dArc );
        //modify object
        double xs, ys;
        xs = m_xwprev - arc->GetPosX();
        ys = m_ywprev - arc->GetPosY();

        if ( m_mode2 == 1 )
        {
            double radius = arc->GetRadius();

            m_arcstart =  wxRadToDeg( atan2( ys, xs ) );

            //normalize to same radius
            arc->SetX1( cos( atan2( ys, xs ) ) * radius );
            arc->SetY1( sin( atan2( ys, xs ) ) * radius );
        }
        else if (  m_mode2 == 2 )
        {
            //no need to normalize to same radius because it does not change
            arc->SetX2( xs );
            arc->SetY2( ys );
            m_arcend =  wxRadToDeg( atan2( ys, xs ) );
        }
        else
        {
            SetMode( m_mode );
        }
        m_pending = true;
    }
    else
        event.Skip();
}


BEGIN_EVENT_TABLE( a2dDrawLineTool, a2dStDrawTool )
    EVT_CHAR( a2dDrawLineTool::OnChar )
    EVT_MOUSE_EVENTS( a2dDrawLineTool::OnMouseEvent )
    EVT_DO( a2dDrawLineTool::OnDoEvent )
END_EVENT_TABLE()

a2dDrawLineTool::a2dDrawLineTool( a2dStToolContr* controller, a2dSLine* templateObject ): a2dStDrawTool( controller )
{
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );

    m_AllowModifyTemplate = true;
    m_templateObject = templateObject;
    if ( !m_templateObject )
    {
        m_templateObject = new a2dEndsLine();
        m_templateObject->SetEndScaleX( GetDrawing()->GetHabitat()->GetEndScaleX() );
        m_templateObject->SetEndScaleY( GetDrawing()->GetHabitat()->GetEndScaleY() );
        m_templateObject->SetBegin( GetDrawing()->GetHabitat()->GetLineBegin() );
        m_templateObject->SetEnd( GetDrawing()->GetHabitat()->GetLineEnd() );
        m_templateObject->SetPathType( GetDrawing()->GetHabitat()->GetPathType() );
        m_templateObject->SetContourWidth( GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale() );
        m_contourwidth = GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale();
    }
    else
    {
        m_layer = m_templateObject->GetLayer();
        m_fill = m_templateObject->GetFill();
        m_stroke = m_templateObject->GetStroke();
        m_contourwidth = m_templateObject->GetContourWidth();
    }
}

a2dDrawLineTool::~a2dDrawLineTool()
{
}

void a2dDrawLineTool::SetLineBegin( a2dCanvasObject* begin )
{
    m_templateObject->SetBegin( begin );
}

void a2dDrawLineTool::SetLineEnd( a2dCanvasObject* end )

{
    m_templateObject->SetEnd( end );
}

void a2dDrawLineTool::SetEndScaleX( double xs )
{
    m_templateObject->SetEndScaleX( xs );
}


void a2dDrawLineTool::SetEndScaleY( double ys )
{
    m_templateObject->SetEndScaleY( ys );
}

void a2dDrawLineTool::SetPathType( a2dPATH_END_TYPE pathtype )
{
    if ( GetBusy() )
    {
        a2dSLine* line = wxStaticCast( m_original.Get(), a2dSLine );
        line->SetPathType( pathtype );
        line = wxStaticCast( m_canvasobject.Get(), a2dSLine );
        line->SetPathType( pathtype );
    }

    m_templateObject->SetPathType( pathtype );
}


void a2dDrawLineTool::OnDoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}

void a2dDrawLineTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStDrawTool::AddAnotationToPendingUpdateArea();

    a2dSLine* line = wxStaticCast( m_canvasobject.Get(), a2dSLine );

    m_anotation.Printf( _T( "x %6.3f y %6.3f l %6.3f" ), m_x2, m_y2, line->GetLength() );

    wxCoord w, h, d, l;
    GetTextExtent( m_anotation, &w, &h, &d, &l );
    m_xanotation = GetDrawer2D()->WorldToDeviceX( m_x2 ) + ( int ) h;
    m_yanotation = GetDrawer2D()->WorldToDeviceY( m_y2 ) - ( int ) h;

    a2dStDrawTool::AddAnotationToPendingUpdateArea();
}

void a2dDrawLineTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        if ( event.GetKeyCode() == WXK_SPACE )
        {
            a2dGeneralGlobals->RecordF( this, _T( "line %f %f %f %f" ), m_x1, m_y1, m_x2, m_y2 );

            StartEditingOrFinish();
            return;
        }

        double shiftx;
        double shifty;
        GetKeyIncrement( &shiftx, &shifty );

        //now shift with snap distances in X or Y
        switch( event.GetKeyCode() )
        {
            case WXK_UP:
                shiftx = 0;
                break;

            case WXK_DOWN:
                shiftx = 0;
                shifty = -shifty;
                break;

            case WXK_LEFT:
                shiftx = -shiftx;
                shifty = 0;
                break;

            case WXK_RIGHT:
                shifty = 0;
                break;
            default:
            {
                shiftx = 0;
                shifty = 0;
                event.Skip();
            }
        }

        a2dSLine* line = wxStaticCast( m_canvasobject.Get(), a2dSLine );
        if ( event.m_controlDown )
        {
            m_x2 += shiftx;
            m_y2 += shifty;
            line->EliminateMatrix();
            line->SetPosXY12( m_x1, m_y1, m_x2, m_y2 );
        }
        else
        {
            m_x1 += shiftx;
            m_y1 += shifty;
            m_x2 += shiftx;
            m_y2 += shifty;
            line->EliminateMatrix();
            line->SetPosXY12( m_x1, m_y1, m_x2, m_y2 );
        }

        GetDisplayWindow()->WarpPointer( GetDrawer2D()->WorldToDeviceX( m_x2 ), GetDrawer2D()->WorldToDeviceY( m_y2 ) );

        m_pending = true;
    }
    else
        event.Skip();

}

void a2dDrawLineTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    }

    if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        m_x2 = m_x1 =  m_xwprev;
        m_y2 = m_y1 =  m_ywprev;

        m_original = m_templateObject->TClone( clone_childs | clone_members );
        m_original->Translate( m_xwprev, m_ywprev );
        m_original->EliminateMatrix();
        m_original->SetContourWidth( m_contourwidth );
        m_original->SetStroke( m_stroke );
        m_original->SetFill( m_fill );
        m_original->SetLayer( m_layer );

        a2dSLine* line = wxStaticCast( m_original.Get(), a2dSLine );
        if ( line->GetBegin() && line->GetBegin()->GetFlag( a2dCanvasOFlags::normalized ) )
        {
            line->GetBegin()->SetSpecificFlags( false, a2dCanvasOFlags::normalized );
            double norm = GetDrawing()->GetNormalizeScale();
            line->GetBegin()->Scale( norm, norm );
        }
        if ( line->GetEnd() && line->GetEnd()->GetFlag( a2dCanvasOFlags::normalized ) )
        {
            line->GetEnd()->SetSpecificFlags( false, a2dCanvasOFlags::normalized );
            double norm = GetDrawing()->GetNormalizeScale();
            line->GetEnd()->Scale( norm, norm );
        }

        if ( !EnterBusyMode() )
            return;
        AddCurrent( m_original );
    }
    else if ( event.LeftDown() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        //a2dSLine* line = wxStaticCast( m_canvasobject.Get(), a2dSLine );

        a2dGeneralGlobals->RecordF( this, _T( "line %f %f %f %f" ), m_x1, m_y1, m_x2, m_y2 );

        StartEditingOrFinish();
    }
    else  if ( ( event.Dragging() && GetBusy() ) || ( event.Moving() && GetBusy() ) )
    {
        a2dSLine* line = wxStaticCast( m_canvasobject.Get(), a2dSLine );
        a2dSLine* lineorg = wxStaticCast( m_original.Get(), a2dSLine );

        m_x2 =  m_xwprev;
        m_y2 =  m_ywprev;

        line->SetPosXY12( m_x1, m_y1, m_x2, m_y2 );
        lineorg->SetPosXY12( m_x1, m_y1, m_x2, m_y2 );

        m_pending = true;
    }
    else
        event.Skip();

}

BEGIN_EVENT_TABLE( a2dRotateTool, a2dStTool )
    EVT_CHAR( a2dRotateTool::OnChar )
    EVT_MOUSE_EVENTS( a2dRotateTool::OnMouseEvent )
    EVT_IDLE( a2dRotateTool::OnIdle )
    EVT_KEY_DOWN( a2dRotateTool::OnKeyDown )
END_EVENT_TABLE()

a2dRotateTool::a2dRotateTool( a2dStToolContr* controller ): a2dStTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_canvasobject = 0;
    m_mode = false;
}

a2dRotateTool::~a2dRotateTool()
{
}

bool a2dRotateTool::EnterBusyMode()
{
    if( !a2dStTool::EnterBusyMode() )
        return false;
    CaptureMouse();
    return true;
}

void a2dRotateTool::FinishBusyMode( bool closeCommandGroup )
{
    ReleaseMouse();
    a2dStTool::FinishBusyMode();
}

void a2dRotateTool::AbortBusyMode()
{
    ReleaseMouse();
    a2dStTool::AbortBusyMode();
}

void a2dRotateTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() && m_canvasobject )
    {
        a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();

        double dx = m_xwprev - m_xr;
        double dy = m_ywprev - m_yr;
        double ang;
        if ( !dx && !dy )
            ang = 0;
        else
            ang = atan2( dy, dx );

        double shiftx;
        double shifty;
        double xw;
        double yw;

        GetKeyIncrement( &shiftx, &shifty );

        switch( event.GetKeyCode() )
        {
            case WXK_UP:
            {
                yw = m_yr + shifty;
            }
            break;

            case WXK_DOWN:
            {
                yw = m_yr - shifty;
            }
            break;

            case WXK_LEFT:
            {
                xw = m_xr - shiftx;
            }
            break;

            case WXK_RIGHT:
            {
                xw = m_xr + shiftx;
            }
            break;
            case WXK_SPACE:
            {
                m_canvasobject->SetVisible( true );
                GetCanvasCommandProcessor()->Submit(
                    new a2dCommand_SetCanvasProperty( m_canvasobject, a2dCanvasObject::PROPID_TransformMatrix,  m_transform )
                );
                FinishBusyMode();
            }
            break;
            default:
                event.Skip();
        }

        if( restrict )
            restrict->RestrictPoint( xw, yw, a2dRestrictionEngine::snapToPointAngle );

        dx = xw - m_xr;
        dy = yw - m_yr;

        double angn;
        if ( !dx && !dy )
            angn = 0;
        else
            angn = atan2( dy, dx );

        m_ang =  wxRadToDeg( angn );
        angn = angn - ang;
        if ( angn <= 0 ) angn += 2.0 * wxPI;
        if ( ang < 0 )   ang += 2.0 * wxPI;
        angn = angn + ang;
        ang = angn - ang;

        a2dAffineMatrix rotate;
        rotate.Translate( -m_xr, -m_yr );
        rotate.Rotate( wxRadToDeg( -ang ) );
        rotate.Translate( m_xr, m_yr );

        GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );

        m_canvasobject->Transform( rotate );

        m_pending = true;

        m_xwprev = xw;
        m_ywprev = yw;

        m_pending = true;

        //GetDisplayWindow()->WarpPointer( GetDrawer2D()->WorldToDeviceX(m_xr), GetDrawer2D()->WorldToDeviceY(m_yr) );
    }
    else
        event.Skip();


}

void a2dRotateTool::OnIdle( wxIdleEvent& event )
{
    if ( m_pending && GetBusy() )
    {
        RemoveAllDecorations();

        a2dSLine* ll = new a2dSLine( m_xr, m_yr, m_xwprev, m_ywprev );
        ll->SetStroke( *wxRED, 0 );
        ll->SetRoot( GetDrawing() );

        m_anotation.Printf( _T( "angle %6.3f" ), m_ang );
        m_xanotation = GetDrawer2D()->WorldToDeviceX( m_xwprev );
        m_yanotation = GetDrawer2D()->WorldToDeviceY( m_ywprev );

        AddDecorationObject( ll );

        event.Skip();
    }
    else
        event.Skip();
}

void a2dRotateTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    static int px, py;

    m_x = event.GetX();
    m_y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw, yw;
    MouseToToolWorld( m_x, m_y, xw, yw );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
    }

    if ( event.Moving() && !GetBusy() )
    {
        if ( GetDrawingPart()->IsHitWorld( xw, yw ) )
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );
        else
            GetDrawingPart()->SetCursor( m_toolcursor );
    }

    if ( event.LeftDown()  && !GetBusy() )
    {
        if( restrict )
            restrict->RestrictPoint( m_xwprev, m_ywprev );
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );

        m_canvasobject = GetDrawingPart()->IsHitWorld( xw, yw, wxLAYER_ALL, a2dCANOBJHITOPTION_LAYERS );

        if ( !m_canvasobject )
        {
            m_canvasobject = 0;
            return;
        }
        if ( !m_canvasobject->GetDraggable() )
        {
            m_canvasobject = 0;
            return;
        }

        if ( !EnterBusyMode() )
            return;

        m_transform = m_canvasobject->GetTransformMatrix();

        if ( event.m_shiftDown )
            m_mode = true;
        else
            m_mode = false;

        if ( m_mode )
        {
            m_xr = m_canvasobject->GetPosX();
            m_yr = m_canvasobject->GetPosY();
        }
        else
        {
            m_xr = xw;
            m_yr = yw;
        }

        m_xwprev = xw;
        m_ywprev = yw;

        if( restrict )
            restrict->SetRestrictPoint( m_xr, m_yr );
    }
    else if ( event.LeftUp() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );

        a2dAffineMatrix newtransform = m_canvasobject->GetTransformMatrix();
        m_canvasobject->SetTransformMatrix( m_transform );

        GetCanvasCommandProcessor()->Submit(
            new a2dCommand_SetCanvasProperty( m_canvasobject, a2dCanvasObject::PROPID_TransformMatrix, newtransform )
        );

        FinishBusyMode();
    }
    else if ( ( event.Dragging() && GetBusy() ) || ( event.Moving() && GetBusy() ) )
    {
        if ( px != m_x || py != m_y )
        {
            double dx, dy;

            // like this around object position
            a2dAffineMatrix rotate;

            if( restrict )
                restrict->RestrictPoint( xw, yw, a2dRestrictionEngine::snapToPointAngle );

            dx = m_xwprev - m_xr;
            dy = m_ywprev - m_yr;

            double ang;
            if ( !dx && !dy )
                ang = 0;
            else
                ang = atan2( dy, dx );

            dx = xw - m_xr;
            dy = yw - m_yr;
            double angn;
            if ( !dx && !dy )
                angn = 0;
            else
                angn = atan2( dy, dx );

            m_ang =  wxRadToDeg( angn );

            angn = angn - ang;
            if ( angn <= 0 ) angn += 2.0 * wxPI;

            if ( ang < 0 )   ang += 2.0 * wxPI;

            angn = angn + ang;

            ang = angn - ang;

            rotate.Translate( -m_xr, -m_yr );
            rotate.Rotate( wxRadToDeg( -ang ) );
            rotate.Translate( m_xr, m_yr );

            m_canvasobject->Transform( rotate );

            m_pending = true;

            m_xwprev = xw;
            m_ywprev = yw;
            px = m_x;
            py = m_y;
        }
    }
    else
    {
        event.Skip();
    }
}

void a2dRotateTool::CaptureMouse()
{
    GetDrawingPart()->SetCorridor( m_canvasobject, m_canvasobject );
    GetDrawingPart()->GetDisplayWindow()->CaptureMouse();
}

void a2dRotateTool::ReleaseMouse()
{
    GetDrawingPart()->SetCorridor( NULL, NULL );
    GetDrawingPart()->GetDisplayWindow()->ReleaseMouse();
}


BEGIN_EVENT_TABLE( a2dDrawTextTool, a2dStDrawTool )
    EVT_MOUSE_EVENTS( a2dDrawTextTool::OnMouseEvent )
END_EVENT_TABLE()

INITIALIZE_PROPERTIES( a2dDrawTextTool, a2dStDrawTool )
{
    return true;
}

a2dDrawTextTool::a2dDrawTextTool( a2dStToolContr* controller, a2dText* templateObject ): a2dStDrawTool( controller )
{
    m_useEditOpaque = a2dOpaqueMode_Off;
    m_templateObject = templateObject;
    if ( !m_templateObject )
    {
        m_templateObject = new a2dText( _T( "" ), 0,0, GetDrawing()->GetHabitat()->GetTextTemplateObject()->GetFont(), 0, false );
        a2dText* textobject = m_templateObject.Get();
        textobject->SetCaret( 0 );
        textobject->SetCaretShow( false );
        //textobject->SetLineSpacing( ? );
        bool up = false;
        if ( GetDrawingPart() && GetDrawingPart()->GetDrawer2D() )
            up = !GetDrawingPart()->GetDrawer2D()->GetYaxis();
        textobject->SetNextLineDirection( up );
        //textobject->SetDrawFrame( true );
        //textobject->SetBackGround( true );

        textobject->SetFill( m_fill );
        textobject->SetStroke( m_stroke );
    }
    else
    {
        m_fill = m_templateObject->GetFill();
        m_stroke = m_templateObject->GetStroke();
    }

    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_canvasobject = 0;
}

a2dDrawTextTool::~a2dDrawTextTool()
{
}

void a2dDrawTextTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    }

    a2dCanvasObject* hitobject = NULL;
    a2dText* hitTextObject = NULL;
    if ( event.Moving() )
    {
        if ( !GetBusy() )
        {
            GetDrawingPart()->MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );
            hitobject = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev );
            a2dText* hitTextObject = wxDynamicCast( hitobject, a2dText );
            if ( hitTextObject  && !event.m_shiftDown )
            {
                GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Edit ) );
            }
            else
                GetDrawingPart()->SetCursor( m_toolcursor );
        }
        else
            GetDrawingPart()->SetCursor( m_toolBusyCursor );
    }

    if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        GetDrawingPart()->MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );
        hitobject = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev );
        a2dText* hitTextObject = wxDynamicCast( hitobject, a2dText );
        if ( hitTextObject && !event.m_shiftDown )
            m_original = hitobject;
        else
        {
            m_original = m_templateObject->TClone( clone_childs );
            m_original->SetPosXY( m_xwprev, m_ywprev );
        }
        if ( !a2dBaseTool::EnterBusyMode() )
            return;
        bool backupstartedit = m_editatend;
        m_editatend = true;
        if ( ! ( hitTextObject && !event.m_shiftDown ) )
			AddCurrent( m_original );
        a2dObjectEditTool* edittool = StartEditTool( m_original );
        edittool->SetUseOpaqueEditcopy( a2dOpaqueMode_Off );
        //CloseCommandGroup();delayed to pop of edit tool
        if( m_oneshot )
            StopTool();
        m_editatend = backupstartedit;
    }
    else
        event.Skip();
}

//----------------------------------------------------------------------------
// a2dDragMultiTool
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dDragMultiTool, a2dStTool )
    EVT_KEY_DOWN( a2dDragMultiTool::OnKeyDown )
    EVT_KEY_UP( a2dDragMultiTool::OnKeyUp )
    EVT_CHAR( a2dDragMultiTool::OnChar )
    EVT_MOUSE_EVENTS( a2dDragMultiTool::OnMouseEvent )
END_EVENT_TABLE()

a2dDragMultiTool::a2dDragMultiTool( a2dStToolContr* controller ): a2dStTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag );
    m_onlyKeys = false;

    //only modes which are usefull in dragging
    m_snapTargetFeatures =
        a2dRestrictionEngine::snapToGridPos |
        a2dRestrictionEngine::snapToGridPosForced |
        a2dRestrictionEngine::snapToBoundingBox |
        a2dRestrictionEngine::snapToObjectPos |
        //a2dRestrictionEngine::snapToPinsUnconnected |
        a2dRestrictionEngine::snapToPins |
        a2dRestrictionEngine::snapToObjectVertexes |
        a2dRestrictionEngine::snapToObjectSnapVPath;

    m_deselectAtStart = false;
}

a2dDragMultiTool::~a2dDragMultiTool()
{
}

void a2dDragMultiTool::OnKeyDown( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    if ( !GetBusy() )
    {
        a2dCanvasObject* selected =  m_parentobject->GetChildObjectList()->Find( "", "", a2dCanvasOFlags::SELECTED );
        if ( selected )
        {
            m_original = selected;

            if ( !m_original || !m_original->IsSelected() || !m_original->GetDraggable() )
            {
                m_original = 0;
                return;
            }

            GetDrawingPart()->GetShowObject()->CollectObjects( &m_objects, _T( "" ), a2dCanvasOFlags::SELECTED );
            if ( !EnterBusyMode() )
                return;

            double shiftx;
            double shifty;
	        GetKeyIncrement( &shiftx, &shifty );

            //now shift with snap distances in X or Y

            switch( event.GetKeyCode() )
            {
                case WXK_UP:
                    shiftx = 0;
                    break;

                case WXK_DOWN:
                    shiftx = 0;
                    shifty = -shifty;
                    break;

                case WXK_LEFT:
                    shiftx = -shiftx;
                    shifty = 0;
                    break;

                case WXK_RIGHT:
                    shifty = 0;
                    break;
                default:
                {
                    shiftx = 0;
                    shifty = 0;
                    event.Skip();
                }
            }

            if ( shiftx || shifty )
            {
                m_xwprev += shiftx;
                m_ywprev += shifty;

                DragAbsolute( m_xwprev, m_ywprev );
                m_pending = true;
            }
        }
        else
            event.Skip();
    }
    else
    {
            double shiftx;
            double shifty;
	        GetKeyIncrement( &shiftx, &shifty );

            //now shift with snap distances in X or Y

            switch( event.GetKeyCode() )
            {
                case WXK_UP:
                    shiftx = 0;
                    break;

                case WXK_DOWN:
                    shiftx = 0;
                    shifty = -shifty;
                    break;

                case WXK_LEFT:
                    shiftx = -shiftx;
                    shifty = 0;
                    break;

                case WXK_RIGHT:
                    shifty = 0;
                    break;
                default:
                {
                    shiftx = 0;
                    shifty = 0;
                    event.Skip();
                }
            }

            if ( shiftx || shifty )
            {
                m_xwprev += shiftx;
                m_ywprev += shifty;

                double dxm = m_xwprev - m_startMousexw;
                double dym = m_ywprev - m_startMouseyw;
                DragAbsolute( dxm + m_startObjectx, dym + m_startObjecty );

                m_pending = true;
            }
    }
}

void a2dDragMultiTool::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_UP:
            case WXK_DOWN:
            case WXK_LEFT:
            case WXK_RIGHT:
                FinishBusyMode();
                break;
            default:
            {
                event.Skip();
            }
        }
    }
    else
        event.Skip();
  
}

void a2dDragMultiTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
			case WXK_DELETE:
			{
				// if busy do nothing with this key.
				break;
			}
            default:
                event.Skip();

		}
	}
    else 
    {
        switch( event.GetKeyCode() )
        {
			case WXK_SPACE:
            {
                m_stcontroller->Zoomout();
                break;
            }
            default:
                event.Skip();
        }
    }
}

bool a2dDragMultiTool::StartDragging( int x, int y, a2dCanvasObject* original )
{
    m_x = x;
    m_y = y;
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );

    m_original = original;

    if ( !m_original || !m_original->IsSelected() || !m_original->GetDraggable() )
    {
        m_original = 0;
        return false;
    }

    // collect selected objects
    GetDrawingPart()->GetShowObject()->CollectObjects( &m_objects, _T( "" ), a2dCanvasOFlags::SELECTED );
    if ( !EnterBusyMode() )
        return false;

    a2dCanvasCommandProcessor* cmdh = GetCanvasCommandProcessor();
    if ( cmdh )
        cmdh->PushSendBusyEvent( false );

    m_startMousexw = m_xwprev;
    m_startMouseyw = m_ywprev;
    m_startObjectx = m_original->GetPosX();
    m_startObjecty = m_original->GetPosY();

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetRestrictPoint( m_startObjectx, m_startObjecty );
    }
    return true;
}

void a2dDragMultiTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( m_onlyKeys )
        return;

    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    if ( event.Moving() && !GetBusy() )
    {
        a2dCanvasObject* hit = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev );
        if ( hit && hit->GetDraggable() )
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );
        else
            GetDrawingPart()->SetCursor( m_toolcursor );
        // this to allow EVT_CANVASOBJECT_LEAVE_EVENT
        event.Skip();
    }

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
    }

    if ( event.LeftDown() && !GetBusy() )
    {
        a2dCanvasObject* original = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev );
        if ( !StartDragging( m_x, m_y, original ) )
            return;
    }
    else if ( event.LeftUp() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        if (
            m_xwprev < GetDrawer2D()->GetVisibleMinX() ||
            m_xwprev > GetDrawer2D()->GetVisibleMaxX() ||
            m_ywprev < GetDrawer2D()->GetVisibleMinY() ||
            m_ywprev > GetDrawer2D()->GetVisibleMaxY()
        )
            AbortBusyMode();
        else
            FinishBusyMode();

        //GetDrawingPart()->GetDrawing()->UpdateAllViews( NULL, a2dCANVIEW_UPDATE_ALL );
        //if (m_oneshot)
        //    StopTool();
    }
    else if ( GetBusy() )
    {
        if ( !m_canvasobject )
            return; //todo why does this happen
        double dxm = m_xwprev - m_startMousexw;
        double dym = m_ywprev - m_startMouseyw;
        AdjustShift( &dxm, &dym );
        // drag main object
        // calculate the shift resulting from this
        DragAbsolute( dxm + m_startObjectx, dym + m_startObjecty );

        event.Skip(); //scroll wheel will work

		double unitScale = GetDrawing()->GetUnitsScale();
		wxString state, form;
		form = m_stcontroller->GetFormat() + " " + m_stcontroller->GetFormat();
		state.Printf( form, m_canvasobject->GetBboxMinX()*unitScale, m_canvasobject->GetBboxMinY()*unitScale );
		SetStateString( state, 10 );
		form = _T("width = ") + m_stcontroller->GetFormat() + _T(" height = ") + m_stcontroller->GetFormat();
		state.Printf( form, m_canvasobject->GetBboxWidth()*unitScale, m_canvasobject->GetBboxHeight()*unitScale );
		SetStateString( state, 11 );
    }
    else
    {
        event.Skip();
    }
}

bool a2dDragMultiTool::EnterBusyMode()
{
    if( !a2dStTool::EnterBusyMode() )
        return false;
    CaptureMouse();
    return true;
}

void a2dDragMultiTool::FinishBusyMode( bool closeCommandGroup )
{
    ReleaseMouse();

    a2dCanvasCommandProcessor* cmdh = GetCanvasCommandProcessor();
    //if ( cmdh )
    //    cmdh->SendBusyEvent( true );

    a2dPoint2D pos = m_canvasobject->GetPosXY();
    double dx = m_original->GetPosX() - pos.m_x;
    double dy = m_original->GetPosY() - pos.m_y;
    a2dAffineMatrix transform;
    transform.Translate( -dx, -dy );

    GetCanvasCommandProcessor()->Submit( new a2dCommand_TransformMask( GetDrawingPart()->GetShowObject(), transform, a2dCanvasOFlags::SELECTED, false ) );
    /*
    for( a2dCanvasObjectList::iterator iter = m_copies.begin(); iter != m_copies.end(); ++iter )
    {
        a2dCanvasObject* copy = *iter;
        a2dCanvasObject* original = a2dCanvasObject::PROPID_Original->GetPropertyValue( copy );

        GetCanvasCommandProcessor()->Submit(
            new a2dCommand_SetCanvasProperty( original, a2dCanvasObject::PROPID_TransformMatrix, copy->GetTransformMatrix() )
            );
    }
    */

    //eliminate matrix, to correctly optimize wires.
    for( a2dCanvasObjectList::iterator iter = m_objects.begin(); iter != m_objects.end(); ++iter )
    {
        a2dCanvasObject* connect = *iter;
        if ( !connect->GetRelease() && connect->IsConnect() )
        {
            a2dWirePolylineL* wire = wxStaticCast( connect, a2dWirePolylineL );
            if ( wire && ! wire->GetTransformMatrix().IsIdentity() )
                GetCanvasCommandProcessor()->Submit( new a2dCommand_EliminateMatrix( wire ) );
        }
        (*iter)->SetBin2( false );
    }

    GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );

    m_copies.clear();

	if ( m_lateconnect )
	{
		double hitDistance = GetHitMargin();

		for( a2dCanvasObjectList::iterator iter = m_objects.begin(); iter != m_objects.end(); ++iter )
		{
			a2dCanvasObject* obj = *iter;
			if ( !obj->IsConnect() && obj->HasPins() )
			{
				obj->SetAlgoSkip( true );
				GetDrawing()->GetHabitat()->GetConnectionGenerator()->
        			ConnectToPinsObject( GetDrawingPart()->GetShowObject(), obj, hitDistance );
				obj->SetAlgoSkip( false );
			}
		}
	}

    m_objects.clear();
   
    if ( m_stcontroller->GetSelectionStateUndo() ) 
    {
	    GetCanvasCommandProcessor()->Submit( new a2dCommandMasked( GetDrawingPart()->GetShowObject(), GetCommandgroup(), 
											    a2dCanvasOFlags::SELECTED, a2dCanvasOFlags::SELECTED2 ) );
    }
    
    if ( cmdh )
        cmdh->PopSendBusyEvent();

    a2dStTool::FinishBusyMode();
}

void a2dDragMultiTool::AbortBusyMode()
{
    a2dCanvasCommandProcessor* cmdh = GetCanvasCommandProcessor();
    if ( cmdh )
        cmdh->PopSendBusyEvent();

    ReleaseMouse();
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );
    m_copies.clear();
    m_objects.clear();

    a2dStTool::AbortBusyMode();
}

void a2dDragMultiTool::CaptureMouse()
{
    wxASSERT( m_canvasobject );
    GetDrawingPart()->SetCorridor( m_canvasobject, m_canvasobject );
    GetDrawingPart()->GetDisplayWindow()->CaptureMouse();
}

void a2dDragMultiTool::ReleaseMouse()
{
    GetDrawingPart()->SetCorridor( NULL, NULL );
    GetDrawingPart()->GetDisplayWindow()->ReleaseMouse();
}

void a2dDragMultiTool::DragAbsolute( double x, double y )
{
    a2dCanvasCommandProcessor* cmdh = GetCanvasCommandProcessor();
    //if ( cmdh )
    //    cmdh->SendBusyEvent( true );

    a2dPoint2D oldpos = m_canvasobject->GetPosXY();

    a2dBoundingBox bbox;
    for( a2dCanvasObjectList::iterator iter = m_copies.begin(); iter != m_copies.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        bbox.Expand( obj->GetBbox() );
    }

    // show the object according to the restricted position.
    m_canvasobject->SetPosXY( x, y );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        bool snapold = m_original->GetSnapTo(); //do not snap to original
        m_original->SetSnapTo( false );
        //temporarily restore position according to the mouse position, which is the unrestricted position.
        m_canvasobject->SetPosXY( x, y );

        a2dPoint2D point;
        double dx, dy; //detect restriction distance of any point.

        if ( restrict->GetShiftKeyDown() )
        {
            restrict->RestrictPoint( x, y );
        }
        else
        {
            a2dSnapToWhatMask snapTo = m_snapTargetFeatures;
            a2dSnapToWhatMask was = restrict->GetSnapSourceFeatures();
            if ( m_snapSourceFeatures != a2dRestrictionEngine::snapToNone )
                restrict->SetSnapSourceFeatures( m_snapSourceFeatures );
            if ( snapTo == a2dRestrictionEngine::snapToNone )
                restrict->GetSnapTargetFeatures();

            if ( restrict->RestrictCanvasObjectAtVertexes( m_canvasobject, point, dx, dy, snapTo, true ) )
            {
                //restrict the object drawing to that point
                x += dx;
                y += dy;
            }
            restrict->SetSnapSourceFeatures( was );
        }
        m_original->SetSnapTo( snapold );
    }

    // show the object according to the restricted position.
    m_canvasobject->SetPosXY( x, y );

    if( oldpos != m_canvasobject->GetPosXY() )
    {
        double dx = x - oldpos.m_x;
        double dy = y - oldpos.m_y;
        for( a2dCanvasObjectList::iterator iter = m_copies.begin(); iter != m_copies.end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if( obj != m_canvasobject )
                obj->Translate( dx, dy );
        }

	    if ( bbox.GetValid() )
	    {
		    double unitScale = GetDrawing()->GetUnitsScale();
		    wxString state, form;
		    form = m_stcontroller->GetFormat() + " " + m_stcontroller->GetFormat();
		    state.Printf( form, (bbox.GetMinX()+dx)*unitScale, (bbox.GetMinY()+dy)*unitScale );
		    SetStateString( state, 10 );
		    form = _T("width = ") + m_stcontroller->GetFormat() + _T(" height = ") + m_stcontroller->GetFormat();
		    state.Printf( form, bbox.GetWidth()*unitScale, bbox.GetHeight()*unitScale );
		    SetStateString( state, 11 );
	    }

        GetDrawing()->GetHabitat()->GetConnectionGenerator()->
        	GeneratePinsToConnectObject( GetDrawingPart(), GetDrawingPart()->GetShowObject(), m_canvasobject );

        GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( false );

        m_pending = true;
    }
    //cmdh->SendBusyEvent( false );
}

void a2dDragMultiTool::Render()
{
    if ( m_canvasobject && GetBusy() )
    {
        a2dStTool::Render();
    }
}

void a2dDragMultiTool::AdjustShift( double* WXUNUSED( dx ), double* WXUNUSED( dy ) )
{
}

bool a2dDragMultiTool::CreateToolObjects()
{
    m_objects.AssertUnique();
    a2dCanvasObjectList::iterator iter;

    // find wires which do connect selected objects (wire from a selected object to another selected object)
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
    setflags.SetSkipNotRenderedInDrawing( true );
    setflags.Start( m_parentobject, false );
    a2dCanvasObjectList inBetweenWires;
    for( iter = m_objects.begin(); iter != m_objects.end(); ++iter )
    {
        a2dCanvasObject* original = *iter;
		// correct connect to itself.
		original->DisConnectWith( original, "", true );

        if ( !original->IsConnect() )
            original->FindConnectedWires( inBetweenWires, NULL, true, true );
    }

    // if a wire was already selected as part of the dragged object, remove it here.
    iter = inBetweenWires.begin();
    while( iter != inBetweenWires.end() )
    {
        a2dCanvasObjectList::value_type wire = *iter;
        if ( std::find( m_objects.begin(), m_objects.end(), wire ) !=  m_objects.end() )
            iter = inBetweenWires.erase( iter );
        else
        {
            wire->SetSelected( true );
            iter++;
        }
    }
    setflags.Start( m_parentobject, false );

    // All that will be dragged without rerouting is found and selected.
    m_objects.TakeOverFrom( &inBetweenWires );

	a2dRefMap refs;

    // Clone all selected objects (including newly selected wires in between), in order to drag that
    for( iter = m_objects.begin(); iter != m_objects.end(); ++iter )
    {
        a2dCanvasObjectPtr original = *iter;
        //a2dCanvasObject* copy = original->TClone( clone_members | clone_childs | clone_seteditcopy | clone_setoriginal | clone_noCameleonRef );
        a2dCanvasObject* copy = original->TClone( clone_toDrag | clone_childs | clone_seteditcopy | clone_setoriginal | clone_noCameleonRef, &refs );

        original->SetBin( true );
        m_copies.push_back( copy );
        copy->SetSelected( false );
        copy->SetSelected2( false );

        if( original == m_original )
            m_canvasobject = copy;
    }
    // Add the draged object clones to the parent
    for( iter = m_copies.begin(); iter != m_copies.end(); ++iter )
    {
        a2dCanvasObject* copy = *iter;
        AddEditobject( copy );
    }

    PrepareForRewire(  m_objects, true, false, true, true, &refs );

    // Set the visibility of the original objects and the original wires
    m_objects.SetSpecificFlags( m_renderOriginal, a2dCanvasOFlags::VISIBLE );
    //m_objects.SetSpecificFlags( true, a2dCanvasOFlags::VISIBLE );

    // only now we call this, earlier not possible, because clones of connectedwires not setup yet.
	refs.LinkReferences( true );
    m_pending = true;

    return true;
}

/* clonenbrother check
    for( iter = m_objects.begin(); iter != m_objects.end(); ++iter )
    //for( a2dCanvasObjectList::iterator iter = m_parentobject->GetChildObjectList()->begin(); iter != m_parentobject->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        for( a2dCanvasObjectList::iterator iter2 = obj->GetChildObjectList()->begin(); iter2 != obj->GetChildObjectList()->end(); ++iter2 )
        {
            a2dCanvasObject* obj2 = *iter2;
            a2dPin* pin = wxDynamicCast( obj2, a2dPin );
            if( !pin || pin->GetRelease( ) )
                continue;

            if ( pin->GetCloneBrother() )
                continue;
            if ( pin->ConnectedTo() && pin->ConnectedTo()->GetCloneBrother() )
                continue;
        }
    }
*/

void a2dDragMultiTool::CleanupToolObjects()
{
    a2dStTool::CleanupToolObjects();
}

//----------------------------------------------------------------------------
// a2dDragMultiNewTool
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dDragMultiNewTool, a2dDragMultiTool )
    EVT_ENTER_WINDOW( a2dDragMultiNewTool::OnEnter )
    EVT_MOUSE_EVENTS( a2dDragMultiNewTool::OnMouseEvent )
    EVT_COM_EVENT( a2dDragMultiNewTool::OnComEvent )
END_EVENT_TABLE()

a2dDragMultiNewTool::a2dDragMultiNewTool( a2dStToolContr* controller, a2dCanvasObjectList* originals )
    : a2dDragMultiTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();
    m_originals = *originals;
    m_boxCenterDrag = false;
    m_deselectAtStart = true;
}

a2dDragMultiNewTool::~a2dDragMultiNewTool()
{
    m_originals.clear();
    m_objects.clear();
}

void a2dDragMultiNewTool::OnComEvent( a2dComEvent& event )
{
    if ( GetBusy() )
    {
        if ( event.GetId() == a2dStTool::sig_toolBeforePush )
        {
            a2dProperty* property = ( a2dProperty* ) event.GetProperty();
            a2dStTool* newtool = wxStaticCast( property->GetRefObject(), a2dStTool );
            if ( GetBusy() )
            {
                if ( !wxDynamicCast( newtool, a2dZoomTool ) )
                {
                    FinishBusyMode();
                }
                else
                {
                    m_halted = true;
                }
            }
            SetPending( true );
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

void a2dDragMultiNewTool::OnEnter( wxMouseEvent& event )
{
    if ( GetDrawingPart()->GetDisplayWindow() && !GetDrawingPart()->GetDisplayWindow()->HasFocus() )
        GetDrawingPart()->GetDisplayWindow()->SetFocus();
    event.Skip();
}

bool a2dDragMultiNewTool::StartDragging( int x, int y )
{
    m_x = x;
    m_y = y;
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
    a2dRefMap refs;

    a2dBoundingBox box;

    a2dCanvasObjectList::iterator iter = m_originals.begin();
    for( iter =  m_originals.begin(); iter !=  m_originals.end(); ++iter )
    {
        a2dCanvasObjectPtr original = *iter;
        box.Expand( original->GetBbox() );
    }

    // Clone all given objects (including newly selected wires in between), in order to drag that
    iter = m_originals.begin();
    for( iter =  m_originals.begin(); iter !=  m_originals.end(); ++iter )
    {
        a2dCanvasObjectPtr original = *iter;
        //clone deep, because the cloned objects end up in the document, the originals not, they or only a template.
        a2dCanvasObject* copy = original->TClone( clone_deep  | clone_noCameleonRef | clone_noReference | clone_seteditcopy | clone_setoriginal, &refs );
        copy->SetRoot( GetDrawing() );
        copy->SetSelected( true ); //this is used to find connected wires to new/copied objects.
        m_objects.push_back( copy );
        if ( m_boxCenterDrag )
            copy->Translate( m_xwprev - box.GetMinX() - box.GetWidth()/2.0, m_ywprev - box.GetMinY() - box.GetHeight()/2.0 );
        else
            copy->Translate( m_xwprev, m_ywprev );
    }

    // if template is a bunch of connected objects, the clone needs to be reconnected.
    refs.LinkReferences();

    iter = m_originals.begin();
    for( iter =  m_originals.begin(); iter !=  m_originals.end(); ++iter )
    {
        a2dCanvasObjectPtr original = *iter;
        original->RemoveProperty( a2dCanvasObject::PROPID_Editcopy );
    }

    m_original = m_objects.front();

    if ( !m_original )
    {
        m_original = 0;
        return false;
    }

    if ( !EnterBusyMode() )
        return false;

    a2dCanvasCommandProcessor* cmdh = GetCanvasCommandProcessor();
    if ( cmdh )
        cmdh->PushSendBusyEvent( false );

    m_startMousexw = m_xwprev;
    m_startMouseyw = m_ywprev;
    m_startObjectx = m_original->GetPosX();
    m_startObjecty = m_original->GetPosY();

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetRestrictPoint( m_startObjectx, m_startObjecty );
    }
    DragAbsolute( m_startObjectx, m_startObjecty );
    return true;
}

void a2dDragMultiNewTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw, yw;
    MouseToToolWorld( m_x, m_y, xw, yw );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( xw, yw );
    }

    if( !GetBusy() )
    {
        StartDragging( m_x, m_y );

        // AddCurrent adjusts the objects style and submits an a2dCommand_AddObject
        //m_parentAddedTo = m_parentobject;

        //m_original->SetParent( m_parentAddedTo );
    }
    else if ( event.LeftDown() )
    {
        // don't process this in the base class
    }
    else if ( event.RightDown() )
    {
        event.Skip();
    }
    else if ( event.LeftUp() )
    {
        a2dDragMultiTool::OnMouseEvent( event );
        GetCanvasCommandProcessor()->SetCurrentCanvasObject( 0 );
    }
    else
    {
        // all the rest is as with a usual drag
        a2dDragMultiTool::OnMouseEvent( event );
    }
}

void a2dDragMultiNewTool::AdjustRenderOptions()
{
    // we generally don't want the original to be visible, regardless of the mode
    // this is because the original was cloned from the template object, and is not a
    // true original object
    m_renderOriginal = false;
}

void a2dDragMultiNewTool::DoStopTool( bool WXUNUSED( abort ) )
{
    // a a2dDragMultiNewTool can be stopped, even if it is busy. This is because the
    // tool can be busy with mouse up.
    if( GetBusy() )
    {
        wxASSERT( m_original );
        AbortBusyMode();
    }

    if ( m_commandgroup )
        CloseCommandGroup();
}

bool a2dDragMultiNewTool::EnterBusyMode()
{
    if( !a2dDragMultiTool::EnterBusyMode() )
        return false;

    // we don't want the mouse to be captured, so undo this
    ReleaseMouse();
    return true;
}

void a2dDragMultiNewTool::AbortBusyMode()
{
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );
    m_copies.clear();
    m_objects.clear();

    a2dCanvasCommandProcessor* cmdh = GetCanvasCommandProcessor();
    if ( cmdh )
        cmdh->PopSendBusyEvent();

    a2dStTool::AbortBusyMode();
}

void a2dDragMultiNewTool::FinishBusyMode( bool closeCommandGroup )
{
    a2dCanvasCommandProcessor* cmdh = GetCanvasCommandProcessor();
    //if ( cmdh )
    //    cmdh->SendBusyEvent( true );

    // only difference a2dDragMultiTool ReleaseMouse();
    for( a2dCanvasObjectList::iterator iter = m_copies.begin(); iter != m_copies.end(); ++iter )
    {
        a2dCanvasObject* copy = *iter;
        a2dCanvasObject* original = a2dCanvasObject::PROPID_Original->GetPropertyValue( copy );

        GetCanvasCommandProcessor()->Submit(
            new a2dCommand_SetCanvasProperty( original, a2dCanvasObject::PROPID_TransformMatrix, copy->GetTransformMatrix() )
        );
    }

    GetCanvasCommandProcessor()->Submit(
        new a2dCommand_AddObjects( GetDrawingPart()->GetShowObject(), m_objects )
    );
    
    //eliminate matrix, to correctly optimize wires.
    for( a2dCanvasObjectList::iterator iter = m_objects.begin(); iter != m_objects.end(); ++iter )
    {
        a2dCanvasObject* connect = *iter;
        if ( !connect->GetRelease() && connect->IsConnect() )
        {
            a2dWirePolylineL* wire = wxStaticCast( connect, a2dWirePolylineL );
            if ( wire && ! wire->GetTransformMatrix().IsIdentity() )
                GetCanvasCommandProcessor()->Submit( new a2dCommand_EliminateMatrix( wire ) );
        }
        (*iter)->SetBin2( false );
    }

    GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );

    m_copies.clear();

	if ( m_lateconnect )
	{
		double hitDistance = GetHitMargin();

		for( a2dCanvasObjectList::iterator iter = m_objects.begin(); iter != m_objects.end(); ++iter )
		{
			a2dCanvasObject* obj = *iter;
			if ( !obj->IsConnect() )
			{
				obj->SetAlgoSkip( true );
				GetDrawing()->GetHabitat()->GetConnectionGenerator()->
        			ConnectToPinsObject( GetDrawingPart()->GetShowObject(), obj, hitDistance );
				obj->SetAlgoSkip( false );
			}
		}
	}

    if ( m_stcontroller->GetSelectAtEnd() )
    {
        DeselectAll();
        m_original->SetSelected2( true );
		for( a2dCanvasObjectList::iterator iter = m_objects.begin(); iter != m_objects.end(); ++iter )
		{
			a2dCanvasObject* obj = *iter;
        	obj->SetSelected( true );
		}
    }

    m_objects.clear();

    if ( cmdh )
        cmdh->PopSendBusyEvent();
    //cmdh->SendBusyEvent( false );

    a2dStTool::FinishBusyMode();
}


BEGIN_EVENT_TABLE( a2dCopyMultiTool, a2dDragMultiTool )
    EVT_CHAR( a2dCopyMultiTool::OnChar )
    EVT_MOUSE_EVENTS( a2dCopyMultiTool::OnMouseEvent )
END_EVENT_TABLE()

a2dCopyMultiTool::a2dCopyMultiTool( a2dStToolContr* controller ): a2dDragMultiTool( controller )
{
    m_mindist = GetDrawing()->GetHabitat()->GetCopyMinDistance();
    m_deselectAtStart = false;
}

void a2dCopyMultiTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_SPACE:
            {
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();

}

bool a2dCopyMultiTool::StartDragging( int x, int y, a2dCanvasObject* original )
{
    m_x = x;
    m_y = y;
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );

    m_original = original;

    if ( !m_original || !m_original->IsSelected() || !m_original->GetDraggable() )
    {
        m_original = 0;
        return false;
    }

    // OpenCommandGroup must be issued before new objects are added
    // This is also done in EnterBusyMode, but it can be done more than once
    OpenCommandGroup( false );
    a2dRefMap refs;
    // Copy all selected objects
    // Deselect the original selected objects
    // Select the copied objects
    {
        bool originalfound = false;
        a2dCanvasObjectList::iterator iter;
        for( iter = m_parentobject->GetChildObjectList()->begin(); iter != m_parentobject->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if( obj->GetSelected() )
            {
                a2dCanvasObject* objn = obj->TClone( clone_members | clone_childs, &refs );
                // Collect the new object into the active object list
                m_objects.push_back( objn );
                // Deselect the original objects
                if ( m_stcontroller->GetSelectionStateUndo() ) 
                    GetCanvasCommandProcessor()->Submit( new a2dCommand_SetFlag( obj, a2dCanvasOFlags::SELECTED, false ), true );
                else
                    obj->SetSelected( false );
                // m_original must point to the copied object
                if( obj == m_original )
                {
                    m_original = objn;
                    originalfound = true;
                }
            }
        }

        wxASSERT( originalfound );

        for( iter = m_objects.begin(); iter != m_objects.end(); ++iter )
        {
            ( *iter )->SetSelected( true );
            // Add the new object
            a2dCommand_AddObject* command = new a2dCommand_AddObject( m_parentobject, ( *iter ) );
            command->SetSendBusyEvent( false );
            GetCanvasCommandProcessor()->Submit( command, true );
        }

        // Connect pins in copy as in source
        refs.LinkReferences();
    }
    if ( !EnterBusyMode() )
        return false;

    a2dCanvasCommandProcessor* cmdh = GetCanvasCommandProcessor();
    if ( cmdh )
        cmdh->PushSendBusyEvent( false );

    m_startMousexw = m_xwprev;
    m_startMouseyw = m_ywprev;
    m_startObjectx = m_original->GetPosX();
    m_startObjecty = m_original->GetPosY();

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
        restrict->SetRestrictPoint( m_startObjectx, m_startObjecty );

    // Make one shift so that the copy is not over the original
    {
        double dxm = m_xwprev - m_startMousexw;
        double dym = m_ywprev - m_startMouseyw;
        AdjustShift( &dxm, &dym );
        // drag main object
        // calculate the shift resulting from this
        //DragAbsolute( dxm + m_startObjectx, dym + m_startObjecty );
    }
    return true;
}

void a2dCopyMultiTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    int x = event.GetX();
    int y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    MouseToToolWorld( x, y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
    }

    if ( event.LeftDown() && !GetBusy() )
    {
        a2dCanvasObject* original = GetDrawingPart()->IsHitWorld( m_xwprev, m_ywprev );
        if ( !StartDragging( m_x, m_y, original ) )
            return;
    }
    else if ( event.LeftUp() && GetBusy() )
    {
        a2dDragMultiTool::OnMouseEvent( event );
    }
    else if ( GetBusy() )
    {
        a2dDragMultiTool::OnMouseEvent( event );
    }
    else
    {
        event.Skip();
    }
}

void a2dCopyMultiTool::AdjustShift( double* dx, double* dy )
{
    if( ( *dx ) * ( *dx ) + ( *dy ) * ( *dy ) < m_mindist )
    {
        if( ( *dx ) == 0 && ( *dy ) == 0 )
        {
            ( *dx ) = m_mindist * 1.0;
            ( *dy ) = m_mindist * 0.5;
        }
        else
        {
            double fac = m_mindist / sqrt( ( *dx ) * ( *dx ) + ( *dy ) * ( *dy ) );
            ( *dx ) *= fac;
            ( *dy ) *= fac;
        }
    }
}

BEGIN_EVENT_TABLE( a2dPropertyTool, a2dStTool )
    EVT_CHAR( a2dPropertyTool::OnChar )
    EVT_MOUSE_EVENTS( a2dPropertyTool::OnMouseEvent )
END_EVENT_TABLE()

a2dPropertyTool::a2dPropertyTool( a2dStToolContr* controller ): a2dStTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_id = 0;
    m_withUndo = true;

    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_BULLSEYE );
    m_canvasobject = 0;
}

void a2dPropertyTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_SPACE:
            {
                FinishBusyMode();
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();

}

void a2dPropertyTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }
    GetDrawingPart()->SetCursor( m_toolcursor );

    int x = event.GetX();
    int y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw, yw;
    MouseToToolWorld( x, y, xw, yw );

    if ( event.LeftDown() )
    {
        m_canvasobject = GetDrawingPart()->IsHitWorld( xw, yw, wxLAYER_ALL, a2dCANOBJHITOPTION_LAYERS );

        if ( !m_canvasobject )
        {
            m_canvasobject = 0;
            event.Skip();
            return;
        }

        m_canvasobject->ConnectEvent( a2dEVT_PROPOBJECT_EDITPROPERTIES_EVENT, GetCanvasCommandProcessor() );
        a2dIterC ic( GetDrawingPart() );
        OpenCommandGroup( false );
        m_canvasobject->EditProperties( m_id, m_withUndo );
        /*
                a2dPropertyIdInt32* propId =  m_canvasobject->HasPropertyId( wxT("myprop" )  );
                if ( !propId )
                {
                    propId = new a2dPropertyIdInt32( CLASSNAME( a2dCanvasObject ), wxT("myprop"), a2dPropertyId::flag_none, 0 );
                    a2dGeneralGlobals->GetPropertyIdList().push_back( propId );
                }
                propId->SetPropertyToObject( m_canvasobject, 1234 );
        */
        m_canvasobject->DisconnectEvent( a2dEVT_PROPOBJECT_EDITPROPERTIES_EVENT, GetCanvasCommandProcessor() );
        m_canvasobject->SetPending( true );
        CloseCommandGroup();
        m_canvasobject = 0;
        if ( m_oneshot )
            StopTool();
    }
    else
        event.Skip();

}

bool a2dPropertyTool::StartEditing( double x, double y )
{
    int mouse_x = GetDrawer2D()->WorldToDeviceX( x );
    int mouse_y = GetDrawer2D()->WorldToDeviceY( y );

    return StartEditing( mouse_x, mouse_y );
}

bool a2dPropertyTool::StartEditing( int x, int y )
{
    wxMouseEvent event( wxEVT_LEFT_DOWN );
    event.m_x = x;
    event.m_y = y;

    OnMouseEvent( event );

    return true;
}

BEGIN_EVENT_TABLE( a2dFollowLink, a2dStTool )
    EVT_CHAR( a2dFollowLink::OnChar )
    EVT_MOUSE_EVENTS( a2dFollowLink::OnMouseEvent )
END_EVENT_TABLE()

a2dFollowLink::a2dFollowLink( a2dStToolContr* controller ): a2dStTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_withUndo = true;

    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_BULLSEYE );
    m_canvasobject = 0;
}

void a2dFollowLink::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_SPACE:
            {
                FinishBusyMode();
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();

}

void a2dFollowLink::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }
    GetDrawingPart()->SetCursor( m_toolcursor );

    int x = event.GetX();
    int y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw, yw;
    MouseToToolWorld( x, y, xw, yw );

    if ( event.LeftDown() )
    {
        m_canvasobject = GetDrawingPart()->IsHitWorld( xw, yw, wxLAYER_ALL, a2dCANOBJHITOPTION_LAYERS );

        if ( !m_canvasobject )
        {
            m_canvasobject = 0;
            event.Skip();
            return;
        }

        m_canvasobject->SetPending( true );
        CloseCommandGroup();
        m_canvasobject = 0;
        if ( m_oneshot )
            StopTool();
    }
    else
        event.Skip();

}

bool a2dFollowLink::StartEditing( double x, double y )
{
    int mouse_x = GetDrawer2D()->WorldToDeviceX( x );
    int mouse_y = GetDrawer2D()->WorldToDeviceY( y );

    return StartEditing( mouse_x, mouse_y );
}

bool a2dFollowLink::StartEditing( int x, int y )
{
    wxMouseEvent event( wxEVT_LEFT_DOWN );
    event.m_x = x;
    event.m_y = y;

    OnMouseEvent( event );

    return true;
}












BEGIN_EVENT_TABLE( a2dDrawVPathTool, a2dStDrawTool )
    EVT_IDLE( a2dDrawVPathTool::OnIdle )
    EVT_CHAR( a2dDrawVPathTool::OnChar )
    EVT_MOUSE_EVENTS( a2dDrawVPathTool::OnMouseEvent )
    EVT_UNDO( a2dDrawVPathTool::OnUndoEvent )
    EVT_REDO( a2dDrawVPathTool::OnRedoEvent )
END_EVENT_TABLE()

void a2dDrawVPathTool::AddSegment( a2dVpathSegment* segment )
{
    if ( m_reverse )
        m_vpath->Add( segment );
    else
        m_vpath->Add( segment );
}

a2dDrawVPathTool::a2dDrawVPathTool( a2dStToolContr* controller, a2dVectorPath* templateObject ): a2dStDrawTool( controller )
{
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );
    m_contourwidth = GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale();
    m_segment = NULL;

    m_canvasobject = ( a2dCanvasObject* )0;
    m_vpath = ( a2dVpath* ) 0;

    m_reverse = false;
    m_controlSet = false;

    m_templateObject = templateObject;
    m_controlPoints = NULL;
}

a2dDrawVPathTool::~a2dDrawVPathTool()
{
}

void a2dDrawVPathTool::OnIdle( wxIdleEvent& event )
{
    if ( m_pending && GetBusy() )
    {
        RemoveAllDecorations();

        a2dVectorPath* path = wxStaticCast( m_canvasobject.Get(), a2dVectorPath );
        a2dRect* around = NULL;

        switch ( m_mode )
        {
            case 0:
            {
                //a nice to see normal polygon around it and if stroke transparent show it.
                if ( m_segment && m_bezier )
                {
                    m_controlPoints = new a2dPolylineL();
                    m_controlPoints->AddPoint( m_segmentPrev->m_x1, m_segmentPrev->m_y1 );
                    a2dVpathCBCurveSegment* seg =  ( a2dVpathCBCurveSegment* ) m_segment;
                    m_controlPoints->AddPoint( seg->m_x2, seg->m_y2 );
                    m_controlPoints->AddPoint( seg->m_x3, seg->m_y3 );
                    m_controlPoints->AddPoint( m_segment->m_x1, m_segment->m_y1 );
                    m_controlPoints->SetVisible( true );
                    m_controlPoints->SetFill( *a2dTRANSPARENT_FILL );
                    //m_controlPoints->SetStroke( *wxRED, 0, a2dSTROKE_DOT_DASH );
                    m_controlPoints->SetStroke( *wxRED, 0, a2dSTROKE_SOLID );
                    AddDecorationObject( m_controlPoints );
                }
                break;
            }
            case 1:
            {
                around = new a2dRect( path->GetBboxMinX(), path->GetBboxMinY(), path->GetBboxWidth(), path->GetBboxHeight() );
                around->SetStroke( *wxRED, 0, a2dSTROKE_DOT_DASH );
                around->SetFill( *a2dTRANSPARENT_FILL );
                around->SetRoot( GetDrawing(), false );
                AddDecorationObject( around );
                break;
            }
            default:
                break;
        }

        event.Skip();
    }
    else
        event.Skip();
}

void a2dDrawVPathTool::OnUndoEvent( a2dCommandProcessorEvent& WXUNUSED( event ) )
{
    if ( GetBusy() && !m_halted )
    {
        // recreate edit copies so the edit copy becomes updated from the original.

        a2dVectorPath* path = wxStaticCast( m_canvasobject.Get(), a2dVectorPath );
        if ( ! path->IsEmpty() )
        {
            a2dVpathSegment* lastpoint = path->GetSegments()->back();

            CleanupToolObjects();
            GetDrawingPart()->Update( a2dCANVIEW_UPDATE_OLDNEW );
            CreateToolObjects();
            m_vpath = wxStaticCast( m_canvasobject.Get(), a2dVectorPath )->GetSegments();
            m_segment = lastpoint->Clone();
            m_vpath->push_back( m_segment );
        }
    }
}

void a2dDrawVPathTool::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    OnUndoEvent( event );
}

void a2dDrawVPathTool::SetMode( int mode )
{
    m_mode = mode;
    if ( m_mode > 2 ) m_mode = 0;

    if ( !m_canvasobject )
        return;

    m_pending = true;
}

void a2dDrawVPathTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() && m_canvasobject )
    {
        double shiftx;
        double shifty;
        GetKeyIncrement( &shiftx, &shifty );

        switch( event.GetKeyCode() )
        {
            case WXK_TAB:
            {
                m_mode++;
                SetMode( m_mode );
            }
            break;

            case WXK_UP:
                shiftx = 0;
                break;

            case WXK_DOWN:
                shiftx = 0;
                shifty = -shifty;
                break;

            case WXK_LEFT:
                shiftx = -shiftx;
                shifty = 0;
                break;

            case WXK_RIGHT:
                shifty = 0;
                break;

            default:
            {
                shiftx = 0;
                shifty = 0;
                event.Skip();
            }
        }

        if ( event.m_controlDown )
        {
            m_segment->m_x1 += shiftx;
            m_segment->m_y1 += shifty;
        }
        else
        {
            double x = m_canvasobject->GetPosX();
            double y = m_canvasobject->GetPosY();
            m_canvasobject->SetPosXY( x + shiftx, y + shifty, true );
            a2dVectorPath* path = wxStaticCast( m_canvasobject.Get(), a2dVectorPath );
            path->EliminateMatrix();
        }

        GetDisplayWindow()->WarpPointer( GetDrawer2D()->WorldToDeviceX( m_segment->m_x1 ), GetDrawer2D()->WorldToDeviceY( m_segment->m_y1 ) );

        m_pending = true;
    }
    else
        event.Skip();

}

void a2dDrawVPathTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
    {
        restrict->SetShiftKeyDown( event.ShiftDown() );
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    }
    if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        if ( m_templateObject )
        {
            m_original = m_templateObject->TClone( clone_childs );
            m_original->Translate( m_xwprev, m_ywprev );
            m_original->EliminateMatrix();
        }
        else
        {
            m_original = new a2dVectorPath();
            m_original->SetContourWidth( m_contourwidth );
            m_original->SetStroke( m_stroke );
            m_original->SetFill( m_fill );
            m_original->SetLayer( m_layer );
        }

        if ( !EnterBusyMode() )
            return;
        AddCurrent( m_original );
        m_original->SetRoot( GetDrawing() );
        a2dVectorPath* path = wxStaticCast( m_original.Get(), a2dVectorPath );
        /*
                // Append first point to original
                GetCanvasCommandProcessor()->Submit(
                    new a2dCommand_AddPoint( m_original, m_xwprev, m_ywprev, -1 )
                );
        */
        path->Add( new a2dVpathSegment( m_xwprev, m_ywprev, a2dPATHSEG_MOVETO ) );


        // Append first point to editcopy
        a2dVectorPath* pathedit = wxStaticCast( m_canvasobject.Get(), a2dVectorPath );
        m_vpath = pathedit->GetSegments();
        m_segmentPrev = new a2dVpathSegment( m_xwprev, m_ywprev, a2dPATHSEG_MOVETO );
        m_vpath->Add( m_segmentPrev );
        m_segment = new a2dVpathCBCurveSegment( m_xwprev, m_ywprev, m_xwprev, m_ywprev, m_xwprev, m_ywprev  );
        m_vpath->Add( m_segment );

        // Record commands
        a2dGeneralGlobals->RecordF( this, _T( "polygon" ) );
        a2dGeneralGlobals->RecordF( this, _T( "addpoint %f %f" ), m_xwprev, m_ywprev );

        m_prev_x = m_xwprev;
        m_prev_y = m_ywprev;
        m_contrmid_x = m_xwprev;
        m_contrmid_y = m_ywprev;
        m_pending = true;
        m_controlSet = false;
        m_endSegment = false;
        m_bezier = false;
        if( restrict )
            restrict->SetRestrictPoint( m_xwprev, m_ywprev );
    }
    else if ( event.LeftDown() && GetBusy() )
    {
        if ( m_segmentPrev->m_x1 == m_xwprev && m_segmentPrev->m_y1 == m_ywprev )
            m_controlSet = false;
        else if ( !m_bezier && !m_endSegment )
        {
            m_controlSet = true;
            m_contrmid_x = m_xwprev;
            m_contrmid_y = m_ywprev;
        }
    }
    else if ( event.LeftUp() && GetBusy() )
    {
        if ( m_segmentPrev->m_x1 == m_xwprev && m_segmentPrev->m_y1 == m_ywprev )
        {
        }
        else if ( m_bezier && !m_controlSet )
        {
            m_controlSet = true;
            m_contrmid_x = m_xwprev;
            m_contrmid_y = m_ywprev;
            m_endSegment = true;
        }
        else if ( m_bezier && m_controlSet && !m_endSegment )
        {
            m_endSegment = true;
        }
        else if ( !m_bezier || ( m_bezier && m_endSegment ) )
        {
            GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
            int x = GetDrawer2D()->WorldToDeviceX( m_prev_x );
            int y = GetDrawer2D()->WorldToDeviceY( m_prev_y );
            if ( abs( m_x - x ) >= 3 || abs( m_y - y ) >= 3 )
            {
                /*
                            GetCanvasCommandProcessor()->Submit(
                                new a2dCommand_AddPoint( m_original, m_xwprev, m_ywprev, -1 )
                            );

                            a2dGeneralGlobals->RecordF( this, _T("addpoint %f %f "), m_xwprev, m_ywprev );
                */
                a2dVectorPath* path = wxStaticCast( m_original.Get(), a2dVectorPath );
                path->Add( m_segment->Clone() );

                m_segmentPrev = m_segment;

                m_segment = new a2dVpathCBCurveSegment( m_xwprev, m_ywprev, m_xwprev, m_ywprev, m_xwprev, m_ywprev );
                //m_segment = new a2dVpathSegment(m_xwprev,m_ywprev);
                m_vpath->Add( m_segment );
                m_prev_x = m_xwprev;
                m_prev_y = m_ywprev;
                m_controlSet = false;
                m_endSegment = false;
                m_bezier = false;
            }
            m_pending = true;
            if( restrict )
                restrict->SetRestrictPoint( m_xwprev, m_ywprev );
        }
    }
    else if ( event.Moving() && GetBusy() )
    {
        m_segment->m_x1 = m_xwprev;
        m_segment->m_y1 = m_ywprev;
        m_canvasobject->SetPending( true );
        m_original->SetPending( true );
        m_pending = true;
        event.Skip();
    }
    else if ( event.Dragging() && GetBusy() )
    {
        m_bezier = true;
        if ( !m_endSegment )
        {
            double dx = m_xwprev - m_contrmid_x;
            double dy = m_ywprev - m_contrmid_y;
            a2dVpathCBCurveSegment* seg = ( a2dVpathCBCurveSegment* ) m_segment;
            if ( !m_controlSet )
            {
                seg->m_x2 = m_xwprev;
                seg->m_y2 = m_ywprev;
            }
            else
            {
                seg->m_x2 = m_contrmid_x - dx;
                seg->m_y2 = m_contrmid_y - dy;
            }
            seg->m_x3 = m_xwprev;
            seg->m_y3 = m_ywprev;
            seg->m_x1 = m_xwprev + 1;
            seg->m_y1 = m_ywprev + 1;
        }
        else
        {
            m_segment->m_x1 = m_xwprev;
            m_segment->m_y1 = m_ywprev;
        }
        m_canvasobject->SetPending( true );
        m_pending = true;
        event.Skip();
    }
    else if ( event.LeftDClick() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dGeneralGlobals->RecordF( this, _T( "end" ) );
        StartEditingOrFinish();
    }
    else
        event.Skip();
}

BEGIN_EVENT_TABLE( a2dMovePinTool, a2dDragTool )
    EVT_MOUSE_EVENTS( a2dMovePinTool::OnMouseEvent )
    EVT_CHAR( a2dMovePinTool::OnChar )
END_EVENT_TABLE()

a2dMovePinTool::a2dMovePinTool( a2dStToolContr* controller, a2dPin* dragPin, double xw, double yw, bool disConnectFirst )
    : a2dDragTool( controller )
{
    m_deleteonoutsidedrop = false;
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();


    m_snapSourceFeatures = a2dRestrictionEngine::snapToPins;

    //only modes which are usefull in dragging
    m_snapTargetFeatures = 
                a2dRestrictionEngine::snapToGridPos |
                a2dRestrictionEngine::snapToGridPosForced |
                a2dRestrictionEngine::snapToPins ;

    m_dragPin = dragPin;

    SetFill( *a2dTRANSPARENT_FILL );
    SetStroke( a2dStroke( *wxRED, 1, a2dSTROKE_SOLID ) );

    if ( dragPin )
    {
        if ( disConnectFirst && m_dragPin && m_dragPin->FindNonWirePin() )
        {
            OpenCommandGroup( false );
            m_dragPin->GetParent()->DisConnectAt( m_dragPin, true );
        }

        GetDrawingPart()->ToolWorldToMouse( xw, yw, m_x, m_y );

        GetDrawing()->GetHabitat()->SetLastXyEntry( xw, yw );

        m_dragPinObj = new a2dRectC( xw, yw, 4*GetHitMargin(), 4*GetHitMargin() );
        m_dragPinObj->SetAlgoSkip( true );

        m_dragPinObjPin = wxStaticCast( dragPin->Clone( clone_deep ), a2dPin );
        m_dragPinObjPin->SetTemporaryPin( false );
        m_dragPinObjPin->SetPosXY( 0, 0 );
        m_dragPinObjPin->SetInternal( true );
        m_dragPinObjPin->SetParent( m_dragPinObj );
        m_dragPinObj->Append( m_dragPinObjPin );
        m_dragPinObj->HasPins( true ); 
        m_dragPinObj->SetSelected( true );
        m_dragPinObj->SetFill( m_fill );
        m_dragPinObj->SetStroke( m_stroke );
        dragPin->DuplicateConnectedToOtherPins( false );
        dragPin->ConnectTo( m_dragPinObjPin );
        m_dragPinObjPin->DuplicateConnectedPins( dragPin );
        GetDrawingPart()->GetShowObject()->Append( m_dragPinObj );

        if ( !StartDragging( m_x, m_y, m_dragPinObj ) )
            return;
    }
}

a2dMovePinTool::~a2dMovePinTool()
{
}

void a2dMovePinTool::OnEnter( wxMouseEvent& WXUNUSED( event ) )
{
    if ( GetDrawingPart()->GetDisplayWindow() && !GetDrawingPart()->GetDisplayWindow()->HasFocus() )
        GetDrawingPart()->GetDisplayWindow()->SetFocus();
}

void a2dMovePinTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    //a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    //if( restrict )
    //    restrict->RestrictPoint( m_xwprev, m_ywprev );

    if( event.LeftDown() && !GetBusy() )
    {
        m_dragPin = GetDrawing()->GetHabitat()->GetConnectionGenerator()->
				      SearchPinForStartWire( GetDrawingPart()->GetShowObject(), m_xwprev, m_ywprev, a2dPinClass::Any, GetHitMargin() );

        if ( m_dragPin )
        {
            GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );

            m_dragPinObj = new a2dRectC( m_xwprev, m_ywprev, 4*GetHitMargin(), 4*GetHitMargin() );
            m_dragPinObj->SetAlgoSkip( true );

            m_dragPinObjPin = wxStaticCast( m_dragPin->Clone( clone_deep ), a2dPin );
            m_dragPinObjPin->SetTemporaryPin( false );
            m_dragPinObjPin->SetPosXY( 0, 0 );
            m_dragPinObjPin->SetInternal( true );
            m_dragPinObjPin->SetParent( m_dragPinObj );
            m_dragPinObj->Append( m_dragPinObjPin );
            m_dragPinObj->HasPins( true ); 
            m_dragPinObj->SetSelected( true );
            m_dragPin->DuplicateConnectedToOtherPins( false );
            m_dragPin->ConnectTo( m_dragPinObjPin );
            m_dragPinObjPin->DuplicateConnectedPins( m_dragPin );
            GetDrawingPart()->GetShowObject()->Append( m_dragPinObj );

            if ( !StartDragging( m_x, m_y, m_dragPinObj ) )
                return;
        }
    }
    else if ( event.RightDown() )
    {
        event.Skip();
    }
    else if ( event.LeftUp() )
    {
        a2dDragTool::OnMouseEvent( event );
        GetCanvasCommandProcessor()->SetCurrentCanvasObject( 0 );
    }
    else
    {
        // all the rest is as with a usual drag
        a2dDragTool::OnMouseEvent( event );
    }
}

void a2dMovePinTool::AdjustRenderOptions()
{
    // we generally don't want the original to be visible, regardless of the mode
    // this is because the original was cloned from the template object, and is not a
    // true original object
    m_renderOriginal = false;
}

void a2dMovePinTool::DoStopTool( bool WXUNUSED( abort ) )
{
    // a a2dMovePinTool can be stopped, even if it is busy. This is because the
    // tool can be busy with mouse up.
    if( GetBusy() )
    {
        wxASSERT( m_original );
        AbortBusyMode();
    }

    if ( m_commandgroup )
        CloseCommandGroup();
}

bool a2dMovePinTool::EnterBusyMode()
{
    if( !a2dDragTool::EnterBusyMode() )
        return false;

    // we don't want the mouse to be captured, so undo this
    ReleaseMouse();

    // Note: there is no need to overload Finish/AbortBusyMode,
    // because the base class stores if it did capture the mouse

    return true;
}

void a2dMovePinTool::FinishBusyMode( bool closeCommandGroup )
{
    m_dragPinObj->SetRelease( true );

    //restore connect backup
    m_original->DoConnect( m_wasMayConnect );
    ReleaseMouse();

    //the next has internal the functionality to connect object pins to objectpins which now can connect.
    GetCanvasCommandProcessor()->Submit(
        new a2dCommand_SetCanvasProperty( m_original, a2dCanvasObject::PROPID_TransformMatrix, m_canvasobject->GetTransformMatrix() )
    );
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );

    a2dCanvasObjectPtr parentObjDragPin = m_dragPin->GetParent();
    parentObjDragPin->SetAlgoSkip( true );
    m_original->SetAlgoSkip( true );
    m_canvasobject->SetAlgoSkip( true );

    double hitDistance = GetHitMargin()/10.0; //make more accurate
    //double hitDistance = GetDrawing()->GetHabitat()->GetCoordinateEpsilon();//make more accurate
    a2dPin* pinother = m_dragPin->GetPinClass()->GetConnectionGenerator()->
                SearchPinForFinishWire( GetDrawingPart()->GetShowObject(), m_dragPin, NULL, hitDistance  );

    if ( pinother && ! m_dragPin->IsConnectedTo( pinother ) )
    {
        GetCanvasCommandProcessor()->Submit( new a2dCommand_ConnectPins( m_dragPin, pinother ), true );
    }

    parentObjDragPin->SetAlgoSkip( false );
    m_original->SetAlgoSkip( false );
    m_canvasobject->SetAlgoSkip( false );

    //m_dragPin->DuplicateConnectedPins( m_dragPinObjPin, true );
    m_dragPinObj->DisConnectAt( m_dragPinObjPin );

    a2dStTool::FinishBusyMode();    
}

void a2dMovePinTool::AbortBusyMode()
{
    m_dragPinObj->SetRelease( true );

    m_original->DoConnect( m_wasMayConnect );
    ReleaseMouse();
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );
    a2dStTool::AbortBusyMode();
}
