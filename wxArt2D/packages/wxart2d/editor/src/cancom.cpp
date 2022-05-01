/*! \file canvas/src/cancom.cpp
    \brief see wx/canvas/cancom.h

    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: cancom.cpp,v 1.236 2009/10/01 19:22:35 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

#include <float.h>

#include "wx/canvas/drawer.h"
#include "wx/canvas/polygon.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"
#include "wx/canvas/canorderdlg.h"
#include "wx/editor/cancom.h"
#include "wx/editor/edsttool.h"
#include "wx/editor/editmod.h"

#include <wx/numdlg.h>


//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

//! used application broad to get the central command processor, when using Canvas module
//! Essential this a2dDocviewGlobals->GetDocviewCommandProcessor()
a2dCentralCanvasCommandProcessor* a2dGetCmdh()
{
    return wxDynamicCast( a2dDocviewGlobals->GetDocviewCommandProcessor(), a2dCentralCanvasCommandProcessor );
}

//! get drawing context of current a2dDrawingPart
a2dDrawer2D* a2dGetDrawer2D()
{
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawer )
        return NULL;
    return drawer->GetDrawer2D();
}

IMPLEMENT_DYNAMIC_CLASS( a2dCommand_GdsIoSaveTextAsPolygon, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_KeyIoSaveTextAsPolygon, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_ViewAsImage, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_ViewAsSvg, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_GdsIoSaveFromView, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_KeyIoSaveFromView, a2dCommand )

//!@{ \ingroup commandid

const a2dCommandId a2dCommand_GdsIoSaveTextAsPolygon::Id( wxT( "GdsIoSaveTextAsPolygon" ) );
const a2dCommandId a2dCommand_KeyIoSaveTextAsPolygon::Id( wxT( "KeyIoSaveTextAsPolygon" ) );

const a2dCommandId a2dCommand_ViewAsImage::Id( wxT( "ViewAsImage" ) );
const a2dCommandId a2dCommand_ViewAsSvg::Id( wxT( "ViewAsSvg" ) );

const a2dCommandId a2dCommand_GdsIoSaveFromView::Id( wxT( "GdsIoSaveFromView" ) );
const a2dCommandId a2dCommand_KeyIoSaveFromView::Id( wxT( "KeyIoSaveFromView" ) );

const a2dCommandId a2dCommand_ShowDlg::Id( wxT( "ShowDlg" ) );

//!@}



//!@{ \ingroup menus
//----------------------------------------------------------------------------
// globals a2dMenuIdItem's
//----------------------------------------------------------------------------

DEFINE_MENU_ITEMID_KIND( CmdMenu_GdsIoSaveTextAsPolygon, wxTRANSLATE("Gdsii Save text as Polgyon"), wxTRANSLATE("When possible/implemented, save text as polygon in GdsII format") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_KeyIoSaveTextAsPolygon, wxTRANSLATE("Key Save text as Polgyon"), wxTRANSLATE("When possible/implemented, save text as polygon in KEY format") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_SetGdsIoSaveFromView, wxTRANSLATE("Gdsii Save from View down"), wxTRANSLATE("Structure visible and all down will be saved") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_SetKeyIoSaveFromView, wxTRANSLATE("Key Save from View down"), wxTRANSLATE("Structure visible and all down will be saved") , wxITEM_CHECK )

DEFINE_MENU_ITEMID( CmdMenu_SaveLayers, wxTRANSLATE("SaveLayers"), wxTRANSLATE("Save layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_SaveLayersHome, wxTRANSLATE("SaveLayers Home"), wxTRANSLATE("Save layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_SaveLayersLastDir, wxTRANSLATE("SaveLayers LastDir"), wxTRANSLATE("Save layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_SaveLayersCurrentDir, wxTRANSLATE("SaveLayers Current Dir"), wxTRANSLATE("Save layers to a layer file") )

DEFINE_MENU_ITEMID( CmdMenu_LoadLayers, wxTRANSLATE("LoadLayers"), wxTRANSLATE("Load layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_LoadLayersHome, wxTRANSLATE("LoadLayers Home"), wxTRANSLATE("Load layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_LoadLayersLastDir, wxTRANSLATE("LoadLayers LastDir"), wxTRANSLATE("Load layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_LoadLayersCurrentDir, wxTRANSLATE("LoadLayers Current Dir"), wxTRANSLATE("Load layers to a layer file") )

DEFINE_MENU_ITEMID_KIND( CmdMenu_GdsIoSaveFromView, wxTRANSLATE("Gdsii Save from View down"), wxTRANSLATE("Structure visible and all down will be saved") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_KeyIoSaveFromView, wxTRANSLATE("Key Save from View down"), wxTRANSLATE("Structure visible and all down will be saved") , wxITEM_CHECK )
DEFINE_MENU_ITEMID( CmdMenu_SetLayersProperty, wxTRANSLATE("Set layer property"), wxTRANSLATE("Set property like colour on a layer") )

DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgLayers, wxTRANSLATE("&Init Layers Document"), wxTRANSLATE("layer dialog initialized for editing layers document") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgLayersDocument, wxTRANSLATE("&Init Layers Document"), wxTRANSLATE("layer dialog initialized for editing layers document") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgLayerOrderDocument, wxTRANSLATE("&Order Layers Document"), wxTRANSLATE("order layer dialog initialized for layers document") , wxITEM_CHECK )

DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgStructureDocument, wxTRANSLATE("&Structure Dialog"), wxTRANSLATE("shows structure dialog") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgStructureDocumentModal, wxTRANSLATE("&Structure Dialog"), wxTRANSLATE("shows structure dialog modal") , wxITEM_CHECK )

DEFINE_MENU_ITEMID( CmdMenu_ViewAsImage, wxTRANSLATE("n"), wxTRANSLATE("n") )
DEFINE_MENU_ITEMID( CmdMenu_ViewAsImage_Png, wxTRANSLATE("PNG bitmap"), wxTRANSLATE("Save as PNG bitmap file") )
DEFINE_MENU_ITEMID( CmdMenu_ViewAsImage_Bmp, wxTRANSLATE("BMP bitmap"), wxTRANSLATE("Save as BMP bitmap file") )
DEFINE_MENU_ITEMID( CmdMenu_DocumentAsImage_Png, wxTRANSLATE("Document PNG bitmap"), wxTRANSLATE("Save as PNG bitmap file") )
DEFINE_MENU_ITEMID( CmdMenu_DocumentAsImage_Bmp, wxTRANSLATE("Document BMP bitmap"), wxTRANSLATE("Save as BMP bitmap file") )
DEFINE_MENU_ITEMID( CmdMenu_ViewAsSvg, wxTRANSLATE("SVG file"), wxTRANSLATE("Save view as SVG file") )

DEFINE_MENU_ITEMID( CmdMenu_PushTool_Link, wxTRANSLATE("Insert Link"), wxTRANSLATE("insert a2dText in dragged rectangle having a link to a file") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Link_OneShot, wxTRANSLATE("Insert Link"), wxTRANSLATE("insert a2dText in dragged rectangle having a link to a file") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_FollowLinkDocDrawCam, wxTRANSLATE("Follow Link"), wxTRANSLATE("follow link to a file or drawing or a2dCameleon") )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Master3Link, wxTRANSLATE("Edit Master tool and link"), wxTRANSLATE("Edit Master tool and link"), wxITEM_CHECK )

DEFINE_MENU_ITEMID( CmdMenu_CreateViewSelected, wxTRANSLATE("Create a new view from selected"), wxTRANSLATE("Create a new view from selected") )

//!@}


/*
*   a2dCentralCanvasCommandProcessor
*/

/*
a2dPropertyIdBool* a2dCentralCanvasCommandProcessor::PROPID_PopBeforePush = NULL;
a2dPropertyIdBool* a2dCentralCanvasCommandProcessor::PROPID_OneShotTools = NULL;
a2dPropertyIdBool* a2dCentralCanvasCommandProcessor::PROPID_EditAtEndTools = NULL;
a2dPropertyIdBool* a2dCentralCanvasCommandProcessor::PROPID_Spline = NULL;
a2dPropertyIdUint16* a2dCentralCanvasCommandProcessor::PROPID_Layer = NULL;
a2dPropertyIdFill* a2dCentralCanvasCommandProcessor::PROPID_Fill = NULL;
a2dPropertyIdStroke* a2dCentralCanvasCommandProcessor::PROPID_Stroke = NULL;
a2dPropertyIdFont* a2dCentralCanvasCommandProcessor::PROPID_Font = NULL;
a2dPropertyIdCanvasObject* a2dCentralCanvasCommandProcessor::PROPID_Begin = NULL;
a2dPropertyIdCanvasObject* a2dCentralCanvasCommandProcessor::PROPID_End = NULL;
a2dPropertyIdDouble* a2dCentralCanvasCommandProcessor::PROPID_EndScaleX = NULL;
a2dPropertyIdDouble* a2dCentralCanvasCommandProcessor::PROPID_EndScaleY = NULL;
a2dPropertyIdBool* a2dCentralCanvasCommandProcessor::PROPID_Snap = NULL;
*/

INITIALIZE_PROPERTIES( a2dCentralCanvasCommandProcessor, a2dDocumentCommandProcessor )
{
/*
    PROPID_Fill = new a2dPropertyIdFill( wxT( "Fill" ),
                                         a2dPropertyId::flag_transfer | a2dPropertyId::flag_listfirst | a2dPropertyId::flag_clonedeep,
                                         *a2dNullFILL,
                                         static_cast < a2dPropertyIdFill::ConstGet >( &a2dCentralCanvasCommandProcessor::MX_GetFill ),
                                         static_cast < a2dPropertyIdFill::ConstSet >( &a2dCentralCanvasCommandProcessor::MX_SetFill ) );
    AddPropertyId( PROPID_Fill );

    PROPID_Stroke = new a2dPropertyIdStroke( wxT( "Stroke" ),
            a2dPropertyId::flag_transfer | a2dPropertyId::flag_listfirst | a2dPropertyId::flag_clonedeep,
            *a2dNullSTROKE,
            static_cast < a2dPropertyIdStroke::ConstGet >( &a2dCentralCanvasCommandProcessor::MX_GetStroke ),
            static_cast < a2dPropertyIdStroke::ConstSet >( &a2dCentralCanvasCommandProcessor::MX_SetStroke ) );
    AddPropertyId( PROPID_Stroke );

    PROPID_Font = new a2dPropertyIdFont( wxT( "Font" ),
                                         a2dPropertyId::flag_transfer | a2dPropertyId::flag_clonedeep,
                                         *a2dNullFONT,
                                         static_cast < a2dPropertyIdFont::ConstGet >( &a2dCentralCanvasCommandProcessor::MX_GetFont ),
                                         static_cast < a2dPropertyIdFont::ConstSet >( &a2dCentralCanvasCommandProcessor::MX_SetFont ) );
    AddPropertyId( PROPID_Font );

    A2D_PROPID_M( a2dPropertyIdCanvasObject, a2dCentralCanvasCommandProcessor, Begin, 0, m_currentBegin );
    A2D_PROPID_M( a2dPropertyIdCanvasObject, a2dCentralCanvasCommandProcessor, End, 0, m_currentEnd );
    A2D_PROPID_M( a2dPropertyIdDouble, a2dCentralCanvasCommandProcessor, EndScaleX, 0, m_currentXscale );
    A2D_PROPID_M( a2dPropertyIdDouble, a2dCentralCanvasCommandProcessor, EndScaleY, 0, m_currentYscale );
    A2D_PROPID_M( a2dPropertyIdBool, a2dCentralCanvasCommandProcessor, PopBeforePush, false, m_popBeforePush );
    A2D_PROPID_M( a2dPropertyIdBool, a2dCentralCanvasCommandProcessor, OneShotTools, false, m_oneShotTools );
    A2D_PROPID_M( a2dPropertyIdBool, a2dCentralCanvasCommandProcessor, EditAtEndTools, false, m_editAtEndTools );
    A2D_PROPID_M( a2dPropertyIdBool, a2dCentralCanvasCommandProcessor, Spline, false, m_spline );
    A2D_PROPID_D( a2dPropertyIdBool, Snap, false );
    A2D_PROPID_M( a2dPropertyIdUint16, a2dCentralCanvasCommandProcessor, Layer, false, m_layer );
*/
    return true;
}

IMPLEMENT_CLASS( a2dCentralCanvasCommandProcessor, a2dDocumentCommandProcessor )

BEGIN_EVENT_TABLE( a2dCentralCanvasCommandProcessor, a2dDocumentCommandProcessor )
    EVT_BEGINBUSY( a2dCentralCanvasCommandProcessor::OnBusyBegin )
    EVT_ENDBUSY( a2dCentralCanvasCommandProcessor::OnBusyEnd )
    EVT_CHANGED_DOCUMENT( a2dCentralCanvasCommandProcessor::OnChangedDocument )

    EVT_MENU( CmdMenu_ChangeCentralStyle().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_GdsIoSaveTextAsPolygon().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_KeyIoSaveTextAsPolygon().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetGdsIoSaveFromView().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetKeyIoSaveFromView().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_SaveLayers().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SaveLayersHome().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SaveLayersLastDir().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SaveLayersCurrentDir().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_LoadLayers().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_LoadLayersHome().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_LoadLayersLastDir().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_LoadLayersCurrentDir().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_GdsIoSaveFromView().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_KeyIoSaveFromView().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetLayersProperty().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )

    EVT_UPDATE_UI( CmdMenu_GdsIoSaveTextAsPolygon().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_KeyIoSaveTextAsPolygon().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetGdsIoSaveFromView().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetKeyIoSaveFromView().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )

    EVT_UPDATE_UI( wxID_UNDO, a2dCentralCanvasCommandProcessor::OnUpdateUndo )
    EVT_UPDATE_UI( wxID_REDO, a2dCentralCanvasCommandProcessor::OnUpdateRedo )

    EVT_MENU( CmdMenu_PushTool_RenderImageZoom().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_ShowDlgLayers().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgLayersDocument().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgLayersGlobal().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgLayerOrderDocument().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgLayerOrderGlobal().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    //EVT_MENU( CmdMenu_ShowDlgPropEdit().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_ShowDlgStructureDocument().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgStructureDocumentModal().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgStructureDocument().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgStructureDocumentModal().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )

    EVT_MENU( CmdMenu_SetFont().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )

    EVT_UPDATE_UI( CmdMenu_ShowDlgLayers().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgLayersDocument().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgLayersGlobal().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgLayerOrderDocument().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgLayerOrderGlobal().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    //EVT_UPDATE_UI( CmdMenu_ShowDlgPropEdit().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )

    EVT_UPDATE_UI( CmdMenu_SetLayerDlgModeless().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )

    EVT_MENU( CmdMenu_ViewAsImage().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ViewAsImage_Png().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ViewAsImage_Bmp().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_DocumentAsImage_Png().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_DocumentAsImage_Bmp().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ViewAsSvg().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_PushTool_Link_OneShot().GetId() , a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PushTool_Link().GetId() , a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PushTool_FollowLinkDocDrawCam().GetId() , a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PushTool_Master3Link().GetId() , a2dCentralCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_CreateViewSelected().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_FileImport().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_FileExport().GetId(), a2dCentralCanvasCommandProcessor::OnMenu )

    EVT_UPDATE_UI( CmdMenu_ViewAsImage_Png().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ViewAsImage_Bmp().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_DocumentAsImage_Png().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_DocumentAsImage_Bmp().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ViewAsSvg().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_FileExport().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_FileImport().GetId(), a2dCentralCanvasCommandProcessor::OnUpdateMenu )

END_EVENT_TABLE()

a2dCentralCanvasCommandProcessor::a2dCentralCanvasCommandProcessor( long flags , bool initialize, int maxCommands )
    : a2dDocumentCommandProcessor( flags, initialize, maxCommands )
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    , m_initCurrentSmartPointerOwner( this )
#endif
{
    m_layersdlg = NULL;
    m_layerOrderDlg = NULL;
    m_structDlgDoc = NULL;
}

a2dCentralCanvasCommandProcessor::a2dCentralCanvasCommandProcessor( a2dDocumentCommandProcessor* other )
    : a2dDocumentCommandProcessor( other )
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    , m_initCurrentSmartPointerOwner( this )
#endif
{
    m_layersdlg = NULL;
    m_layerOrderDlg = NULL;
    m_structDlgDoc = NULL;
}

a2dCentralCanvasCommandProcessor::~a2dCentralCanvasCommandProcessor()
{
    DeleteDlgs();
}

void a2dCentralCanvasCommandProcessor::UpdateAllViews()
{
    a2dDocumentList::iterator iter = m_docs.begin();
    while( iter != m_docs.end() )
    {
        a2dDECLARE_LOCAL_ITEM( a2dDocumentList::value_type, doc, *iter );
        iter++;
        doc->UpdateAllViews( NULL, a2dCANVIEW_UPDATE_ALL );
    }
}

bool a2dCentralCanvasCommandProcessor::ProcessEvent( wxEvent& event )
{
	if ( !a2dDocumentCommandProcessor::ProcessEvent( event ) )
    {
        if ( event.GetEventType() == wxEVT_COMMAND_MENU_SELECTED )
        {          
            a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
            if ( !drawingPart || !drawingPart->GetDrawing() || !drawingPart->GetDrawing()->GetCommandProcessor() )
                return true;
            return drawingPart->GetDrawing()->GetCommandProcessor()->ProcessEvent( event );           
        }
        else if ( event.GetEventType() == wxEVT_UPDATE_UI
           )
        {          
            a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
            if ( !drawingPart || !drawingPart->GetDrawing() || !drawingPart->GetDrawing()->GetCommandProcessor() )
            {
                a2dCanvasCommandProcessor::StaticProcessEvent(  (wxUpdateUIEvent&) event );
                return true;
            }
            return drawingPart->GetDrawing()->GetCommandProcessor()->ProcessEvent( event );           
        }
        return false;
    }
    return true;
}

void a2dCentralCanvasCommandProcessor::OnUpdateUndo( wxUpdateUIEvent& event )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if  ( doc )
    {
        // assume the current active commandprocessor is based on the active drawingpart its a2dDrawing.
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
        {
            event.Skip();
            return;
        }
        a2dDrawing* drawing = drawer->GetDrawing();
        event.Enable( ( drawing->GetCommandProcessor() && drawing->GetCommandProcessor()->CanUndo() ));
    }
    else
        event.Skip();
}

void a2dCentralCanvasCommandProcessor::OnUpdateRedo( wxUpdateUIEvent& event )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if  ( doc )
    {
        // assume the current active commandprocessor is based on the active drawingpart its a2dDrawing.
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
        {
            event.Skip();
            return;
        }

        a2dDrawing* drawing = drawer->GetDrawing();
        event.Enable( ( drawing->GetCommandProcessor() && drawing->GetCommandProcessor()->CanRedo() ));
    }
    else
        event.Skip();
}

void a2dCentralCanvasCommandProcessor::OnPushToolUpdateMenu( wxUpdateUIEvent& event )
{
    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawingPart )
        return;        
    if ( !drawingPart->GetDrawing() )
        return;

    // a2dCanvasCommandProcessor will decide
    event.Skip();
}

void a2dCentralCanvasCommandProcessor::OnUpdateMenu( wxUpdateUIEvent& event )
{
    if ( event.GetId() == CmdMenu_InsertGroupRef().GetId() )
    {
    }
    else if ( event.GetId() == CmdMenu_GdsIoSaveTextAsPolygon().GetId() )
        event.Check( a2dCanvasGlobals->GetHabitat()->m_gdsiiSaveTextAsPolygon );
    else if ( event.GetId() == CmdMenu_KeyIoSaveTextAsPolygon().GetId() )
        event.Check( a2dCanvasGlobals->GetHabitat()->m_keySaveTextAsPolygon );
    else if ( event.GetId() == CmdMenu_SetGdsIoSaveFromView().GetId() )
        event.Check( a2dCanvasGlobals->GetHabitat()->m_gdsiiSaveFromView );
    else if ( event.GetId() == CmdMenu_SetKeyIoSaveFromView().GetId() )
        event.Check( a2dCanvasGlobals->GetHabitat()->m_keySaveFromView );
    else if ( event.GetId() == CmdMenu_ShowDlgStructureDocumentModal().GetId() )
    {
        event.Check( IsShownStructureDlg() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgStructure().GetId() )
    {
    }
    else if ( 
              event.GetId() == CmdMenu_FileExport().GetId() ||
              event.GetId() == CmdMenu_FileImport().GetId() ||

              event.GetId() == CmdMenu_ViewAsImage_Png().GetId() ||
              event.GetId() == CmdMenu_ViewAsImage_Bmp().GetId() ||
              event.GetId() == CmdMenu_DocumentAsImage_Png().GetId() ||
#if wxART2D_USE_SVGIO
		      event.GetId() == CmdMenu_ViewAsSvg().GetId() ||
#endif // wxART2D_USE_SVGIO
              event.GetId() == CmdMenu_DocumentAsImage_Bmp().GetId()
              )
    {
		event.Enable( false );
        a2dCanvasDocument* doc = CheckCanvasDocument();
        if  ( !doc )
            return;
        a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawingPart )
            return;
        if ( !drawingPart->GetDrawing() )
            return;
        event.Enable( true ); 
    }
    else 
    {
        event.Skip();
    }
}


void a2dCentralCanvasCommandProcessor::OnMenu( wxCommandEvent& event )
{
    bool onTop = true;
    long style = wxDEFAULT_DIALOG_STYLE;
    if ( !onTop )
        style |= wxSTAY_ON_TOP;

    bool found = false;

    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return;
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return; 
    if ( !drawer->GetDrawingPart()->GetDrawing() )
        return;

    a2dDrawing* drawing = drawer->GetDrawingPart()->GetDrawing();

    a2dCanvasObject* top = drawer->GetDrawingPart()->GetShowObject();

    if ( event.GetId() == CmdMenu_ChangeCentralStyle().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_SetLayer().GetId() )
    {
        found = true;
        long w = wxGetNumberFromUser( _( "Give layer number:" ), _( "Layer:" ), _( "layer number" ), a2dCanvasGlobals->GetHabitat()->m_layer, 0, wxMAXLAYER );

        if (  w != -1 )
        {
            if ( w >= 0 && w < ( long ) wxMAXLAYER )
            {
                a2dCanvasGlobals->GetHabitat()->SetLayer( w, true );
            }
            else
                ( void )wxMessageBox( _( "Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        }
    }
    else if ( event.GetId() == CmdMenu_SetTarget().GetId() )
    {
        found = true;
        long w = wxGetNumberFromUser( _( "Give layer number:" ), _( "Layer:" ), _( "Set Target Layer" ), a2dCanvasGlobals->GetHabitat()->m_target, 0, wxMAXLAYER );

        if (  w != -1 )
        {
            if ( w >= 0 && w < ( long ) wxMAXLAYER )
            {
                a2dCanvasGlobals->GetHabitat()->SetTarget( w );
            }
            else
                ( void )wxMessageBox( _( "Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        }
    }
    else if ( event.GetId() == CmdMenu_SetNormalizeFactor().GetId() )
    {
        found = true;
        if ( !CheckCanvasDocument() )
            return;

        double norm = wxGetDoubleNumberFromUser( _( "Give normalize factor:" ), _( "Normalize" ), _( "Normalize factor" ), 1, 0, 10000 );

        if (  norm != DBL_MAX )
        {
            if ( norm >= 0 && norm < 10000 )
            {
                CheckCanvasDocument()->SetNormalizeScale( norm );
                a2dCanvasGlobals->GetHabitat()->SetTarget( norm );
            }
            else
                ( void )wxMessageBox( _( "Normlaize factor must be > 0" ), _( "Normalize" ), wxICON_INFORMATION | wxOK );
        }
    }
    else if ( event.GetId() == CmdMenu_GdsIoSaveTextAsPolygon().GetId() )
    {
        found = true;
        a2dCanvasGlobals->GetHabitat()->GdsIoSaveTextAsPolygon( !a2dCanvasGlobals->GetHabitat()->m_gdsiiSaveTextAsPolygon );
        const_forEachIn( a2dDocumentTemplateList, &GetDocTemplates() )
        {
            a2dDocumentTemplateList::value_type temp = *iter;
            if ( temp->IsVisible() && wxT( "GDS-II" ) == temp->GetDescription() )
            {
    #if wxART2D_USE_GDSIO
                wxStaticCast( temp->GetDocumentIOHandlerStrOut(), a2dIOHandlerGDSOut )->SetSaveTextAsVpath( a2dCanvasGlobals->GetHabitat()->GetGdsIoSaveTextAsPolygon() );
    #endif //wxART2D_USE_GDSIO
            }
        }
    }
    else if ( event.GetId() == CmdMenu_KeyIoSaveTextAsPolygon().GetId() )
    {
        found = true;
        a2dCanvasGlobals->GetHabitat()->KeyIoSaveTextAsPolygon( !a2dCanvasGlobals->GetHabitat()->m_keySaveTextAsPolygon );
        const_forEachIn( a2dDocumentTemplateList, &GetDocTemplates() )
        {
            a2dDocumentTemplateList::value_type temp = *iter;
            if ( temp->IsVisible() && wxT( "KEY" ) == temp->GetDescription() )
            {
    #if wxART2D_USE_KEYIO
                wxStaticCast( temp->GetDocumentIOHandlerStrOut(), a2dIOHandlerKeyOut )->SetSaveTextAsVpath( a2dCanvasGlobals->GetHabitat()->GetKeyIoSaveTextAsPolygon() );
    #endif //wxART2D_USE_KEYIO
            }
        }
    }
    else if ( event.GetId() == CmdMenu_SetGdsIoSaveFromView().GetId() )
    {
        found = true;
        a2dCanvasGlobals->GetHabitat()->m_gdsiiSaveFromView = !a2dCanvasGlobals->GetHabitat()->m_gdsiiSaveFromView;
        a2dCommand_GdsIoSaveFromView* command = new a2dCommand_GdsIoSaveFromView( a2dCanvasGlobals->GetHabitat()->m_gdsiiSaveFromView );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_SetKeyIoSaveFromView().GetId() )
    {
        found = true;
        a2dCanvasGlobals->GetHabitat()->m_keySaveFromView = !a2dCanvasGlobals->GetHabitat()->m_keySaveFromView;
        a2dCommand_KeyIoSaveFromView* command = new a2dCommand_KeyIoSaveFromView( a2dCanvasGlobals->GetHabitat()->m_keySaveFromView );
        Submit( command );
    }

    if ( found )
    {}
    else if ( event.GetId() == CmdMenu_SaveLayers().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of output Layer file" ),
                                 _T( "@{layerFileApplication}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
        SaveLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_SaveLayersLastDir().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of output Layer file" ),
                                 _T( "@{lastLayerFileSavePath}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
        SaveLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_SaveLayersHome().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of output Layer file" ),
                                 _T( "@{layerFileSavePath}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
        SaveLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_SaveLayersCurrentDir().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of output Layer file" ),
                                 _T( "./" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
        SaveLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_LoadLayers().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of input Layer file" ),
                                 _T( "@{layerFileApplication}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_OPEN | wxFD_FILE_MUST_EXIST );
        LoadLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_LoadLayersHome().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of input Layer file" ),
                                 _T( "@{layerFileSavePath}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_OPEN | wxFD_FILE_MUST_EXIST );
        LoadLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_LoadLayersLastDir().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of input Layer file" ),
                                 _T( "@{lastLayerFileSavePath}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_OPEN | wxFD_FILE_MUST_EXIST );
        LoadLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_LoadLayersCurrentDir().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of input Layer file" ),
                                 _T( "./" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_OPEN | wxFD_FILE_MUST_EXIST );
        LoadLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_SetCursor().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_GdsIoSaveFromView().GetId() )
    {
        found = true;
        a2dCanvasGlobals->GetHabitat()->m_gdsiiSaveFromView = !a2dCanvasGlobals->GetHabitat()->m_gdsiiSaveFromView;
        a2dCommand_GdsIoSaveFromView* command = new a2dCommand_GdsIoSaveFromView( a2dCanvasGlobals->GetHabitat()->m_gdsiiSaveFromView );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_KeyIoSaveFromView().GetId() )
    {
        found = true;
        a2dCanvasGlobals->GetHabitat()->m_keySaveFromView = !a2dCanvasGlobals->GetHabitat()->m_keySaveFromView;
        a2dCommand_KeyIoSaveFromView* command = new a2dCommand_KeyIoSaveFromView( a2dCanvasGlobals->GetHabitat()->m_keySaveFromView );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_SetLayersProperty().GetId() )
    {
        found = true;
    }

    if ( found )
    {
    }
    else if ( event.GetId() == CmdMenu_ShowDlgLayers().GetId() ||
			  event.GetId() == CmdMenu_ShowDlgLayersDrawing().GetId() ||
              event.GetId() == CmdMenu_ShowDlgLayersDocument().GetId() ||
              event.GetId() == CmdMenu_ShowDlgLayersGlobal().GetId()
            )
    {
        found = true;
        a2dCanvasDocument* doc = ( a2dCanvasDocument* ) GetCurrentDocument();

        if ( !m_layersdlg )
        {
            m_layersdlg = new LayerPropertiesDialog( NULL, a2dCanvasGlobals->GetHabitat()->GetLayerSetup(), false, _( "Layer of Document" ),  style | wxRESIZE_BORDER );
        }

        if ( !m_layersdlg->IsShown() )
        {
            m_layersdlg->Show( true );
            if ( event.GetId() == CmdMenu_ShowDlgLayersDocument().GetId() )
            {
                a2dCanvasDocument* doc = CheckCanvasDocument();
                if ( !doc )
                    return;

                m_layersdlg->Init( doc->GetLayerSetup() );
            }
            else if ( event.GetId() == CmdMenu_ShowDlgLayersDrawing().GetId() || event.GetId() == CmdMenu_ShowDlgLayers().GetId() )
            {
                a2dCanvasDocument* doc = CheckCanvasDocument();
                if ( !doc )
                    return;

                m_layersdlg->Init( doc->GetDrawing()->GetLayerSetup() );
            }
            else
                m_layersdlg->Init( a2dCanvasGlobals->GetHabitat()->GetLayerSetup() );
        }
        else
        {
            m_layersdlg->Show( false );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgLayerOrderDocument().GetId() ||
              event.GetId() == CmdMenu_ShowDlgLayerOrderGlobal().GetId() )
    {
        found = true;
        a2dCanvasDocument* doc = ( a2dCanvasDocument* ) GetCurrentDocument();

        if ( !m_layerOrderDlg )
        {
            m_layerOrderDlg = new a2dLayerOrderDialog( NULL, NULL, _( "Layer of Document" ),  style | wxRESIZE_BORDER );
        }

        if ( !m_layerOrderDlg->IsShown() )
        {
            if ( event.GetId() == CmdMenu_ShowDlgLayerOrderDocument().GetId() )
            {
                a2dCanvasDocument* doc = CheckCanvasDocument();
                if ( !doc )
                    return;

                m_layerOrderDlg->Init( doc->GetLayerSetup() );
            }
            else
                m_layerOrderDlg->Init( a2dCanvasGlobals->GetHabitat()->GetLayerSetup() );
            m_layerOrderDlg->Show( true );
        }
        else
        {
            m_layerOrderDlg->Show( false );
        }
    }
    else if ( event.GetId() == CmdMenu_ViewAsImage_Png().GetId() )
    {
        found = true;

        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;

        a2dCanvasDocument* doc = CheckCanvasDocument();
        if ( !doc )
            return;

        wxString file = AskFile( _( "Give Name of output PNG file" ),
                                 _T( "" ),
                                 _T( "" ),
                                 _T( "*.png" ),
                                 _T( "*.png" ),
                                 wxFD_SAVE );

        SaveViewAsImage( doc, file, wxBITMAP_TYPE_PNG, drawer->GetShowObject() );
    }
    else if ( event.GetId() == CmdMenu_ViewAsImage_Bmp().GetId() )
    {
        found = true;

        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;

        a2dCanvasDocument* doc = CheckCanvasDocument();
        if ( !doc )
            return;

        wxString file = AskFile( _( "Give Name of output BMP file" ),
                                 _T( "" ),
                                 _T( "" ),
                                 _T( "*.bmp" ),
                                 _T( "*.bmp" ),
                                 wxFD_SAVE );

        SaveViewAsImage( doc, file, wxBITMAP_TYPE_BMP, drawer->GetShowObject() );
    }
    else if ( event.GetId() == CmdMenu_DocumentAsImage_Png().GetId() )
    {
        found = true;

        a2dCanvasDocument* doc = CheckCanvasDocument();
        if ( !doc )
            return;

        wxString file = AskFile( _( "Give Name of output PNG file" ),
                                 _T( "" ),
                                 _T( "" ),
                                 _T( "*.png" ),
                                 _T( "*.png" ),
                                 wxFD_SAVE );

        SaveViewAsImage( doc, file, wxBITMAP_TYPE_PNG, doc->GetDrawing()->GetRootObject() );
    }
    else if ( event.GetId() == CmdMenu_DocumentAsImage_Bmp().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of output BMP file" ),
                                 _T( "" ),
                                 _T( "" ),
                                 _T( "*.bmp" ),
                                 _T( "*.bmp" ),
                                 wxFD_SAVE );

        a2dCanvasDocument* doc = CheckCanvasDocument();
        if ( !doc )
            return;

        SaveViewAsImage( doc, file, wxBITMAP_TYPE_BMP, doc->GetDrawing()->GetRootObject() );
    }
#if wxART2D_USE_SVGIO
    else if ( event.GetId() == CmdMenu_ViewAsSvg().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of output SVG file" ),
                                 _T( "" ),
                                 _T( "" ),
                                 _T( "*.svg" ),
                                 _T( "*.svg" ),
                                 wxFD_SAVE );

        a2dCommand_ViewAsSvg* command2 = new a2dCommand_ViewAsSvg( a2dCommand_ViewAsSvg::Args().
                file(  wxFileName( file ) ) );
        Submit( command2 );
    }
#endif //wxART2D_USE_SVGIO
    else if ( event.GetId() == CmdMenu_LineBegin().GetId() )
    {
        found = true;
        a2dCanvasObjectList total;

        if ( !m_librarypoints )
        {
            wxMessageBox( _( "point library not loaded" ), _( "Points" ), wxICON_INFORMATION | wxOK );
            return;
        }
        m_librarypoints->GetDrawing()->GetRootObject()->CollectObjects( &total, _T( "a2dLibraryReference" ), a2dCanvasOFlags::ALL );

        a2dCanvasObjectsDialog objects( NULL, &total, true, ( wxSTAY_ON_TOP | wxRESIZE_BORDER | wxCAPTION ) );
        if ( objects.ShowModal() == wxID_OK )
        {
            a2dCanvasObject* begin = ( ( a2dLibraryReference* )objects.GetCanvasObject() )->GetCanvasObject();
            begin->SetPosXY( 0, 0 );
			a2dCanvasGlobals->GetHabitat()->SetLineBegin( begin );
        }
    }
    else if ( event.GetId() == CmdMenu_LineEnd().GetId() )
    {
        found = true;
        a2dCanvasObjectList total;

        if ( !m_librarypoints )
        {
            wxMessageBox( _( "point library not loaded" ), _( "Points" ), wxICON_INFORMATION | wxOK );
            return;
        }
        m_librarypoints->GetDrawing()->GetRootObject()->CollectObjects( &total, _T( "a2dLibraryReference" ), a2dCanvasOFlags::ALL );

        a2dCanvasObjectsDialog objects( NULL, &total, true, ( wxSTAY_ON_TOP | wxRESIZE_BORDER | wxCAPTION ) );
        if ( objects.ShowModal() == wxID_OK )
        {
            a2dCanvasObject* end = ( ( a2dLibraryReference* )objects.GetCanvasObject() )->GetCanvasObject();
            end->SetPosXY( 0, 0 );
		    a2dCanvasGlobals->GetHabitat()->SetLineEnd( end );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgStructureDocument().GetId() )
    {
        found = true;
        a2dCanvasView* drawer = CheckDrawingView();
        if ( !drawer )
            return;
        a2dCanvasDocument* doc = CheckCanvasDocument();
        if ( !doc )
            return;
        if ( !m_structDlgDoc )
        {
            m_structDlgDoc = new a2dCanvasObjectsDocDialog( NULL, doc, true, false, ( wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxRESIZE_BORDER ) );
        }
        if ( m_structDlgDoc->IsShown() )
        {
            m_structDlgDoc->Show( false );
        }
        else
        {
            m_structDlgDoc->Show( true );
            m_structDlgDoc->Init( doc );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgStructureDocumentModal().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;

        a2dCanvasObjectsDocDialog groups( NULL, doc, true, true, ( wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxRESIZE_BORDER ) );
        if ( groups.ShowModal() == wxID_OK )
        {
            a2dCommand_SetShowObject* command = new a2dCommand_SetShowObject( drawer, a2dCommand_SetShowObject::Args().canvasobject( groups.GetCanvasObject() ) );
            a2dGetCmdh()->Submit( command );
        }
    }
    else if ( event.GetId() == CmdMenu_PushTool_Link().GetId() )
    {
        found = true;
        a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !part )
            return;
        a2dStToolContr* contr = ( a2dStToolContr* ) part->GetCanvasToolContr();

        a2dLinkTool* draw = new a2dLinkTool( contr );
        contr->PushTool( draw );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Link_OneShot().GetId() )
    {
        found = true;
        a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !part )
            return;
        a2dStToolContr* contr = ( a2dStToolContr* ) part->GetCanvasToolContr();

        a2dLinkTool* draw = new a2dLinkTool( contr );
        draw->SetOneShot();
        contr->PushTool( draw );
    }
    else if ( event.GetId() == CmdMenu_PushTool_FollowLinkDocDrawCam().GetId() )
    {
        found = true;
        a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !part )
            return;
        a2dStToolContr* contr = ( a2dStToolContr* ) part->GetCanvasToolContr();

        a2dFollowLinkDocDrawCam* draw = new a2dFollowLinkDocDrawCam( contr );
        contr->PushTool( draw );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Master3Link().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;

        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( contr )
        {
			a2dMasterTagGroups3Link* tool = new a2dMasterTagGroups3Link( contr );
			//tool->SetWireMode( true );
			tool->SetDlgOrEdit( true );
			tool->SetDlgOrEditModal( true );
            tool->SetAllowDoubleClickModifier( true );
			contr->PushTool( tool );
		}
    }
    else if ( event.GetId() == CmdMenu_CreateViewSelected().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        if ( ! drawer->GetShowObject() )
            return;

        a2dCanvasDocument* doc = CheckCanvasDocument();
        if ( !doc )
            return;

        a2dCanvasObject* pushIn = drawer->GetShowObject()->Find( wxT( "" ),  wxT( "" ), a2dCanvasOFlags::SELECTED );

        if ( pushIn )
            pushIn = pushIn->PushInto( drawer->GetShowObject() );

        if ( pushIn )
		{
			pushIn->SetSelected( false );
            pushIn->GetChildObjectList()->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED );
            
			a2dDocumentTemplate* doctemp = doc->GetDocumentTemplate();
		    a2dViewConnector* viewcont = doctemp->GetViewConnector();
			a2dView* newview = viewcont->CreateView( doc, "", a2dREFDOC_NON );

			a2dCanvasView* canview = wxDynamicCast( newview, a2dCanvasView );
			canview->GetDrawingPart()->SetShowObject( pushIn );
        }
        else
            ( void )wxMessageBox( _( "unable to view selected" ), _( "create view" ), wxICON_INFORMATION | wxOK );

	}
    else if ( event.GetId() == CmdMenu_FileImport().GetId() )
    {
        found = true;
        a2dCanvasView* drawer = CheckDrawingView();
        if ( !drawer )
            return;
        a2dDrawingPart* drawingpart = drawer->GetDrawingPart();
        if ( !drawingpart )
            return;

        forEachIn( a2dDocumentTemplateList, &m_docTemplates )
        {
            a2dDocumentTemplateList::value_type temp = *iter;
            a2dCanvasDocumentIOHandlerImageIn* handler = wxDynamicCast( temp->GetDocumentIOHandlerStrIn(), a2dCanvasDocumentIOHandlerImageIn );
            if ( handler )
            {
                handler->SetDrawingPart( drawingpart );
            }
        }

        FileImport();
    }
    else if ( event.GetId() == CmdMenu_FileExport().GetId() )
    {
        found = true;

        a2dCanvasView* drawer = CheckDrawingView();
        if ( !drawer )
            return;
        a2dDrawingPart* drawingpart = drawer->GetDrawingPart();
        if ( !drawingpart )
            return;

        forEachIn( a2dDocumentTemplateList, &m_docTemplates )
        {
            a2dDocumentTemplateList::value_type temp = *iter;
            a2dCanvasDocumentIOHandlerImageOut* handler = wxDynamicCast( temp->GetDocumentIOHandlerStrOut(), a2dCanvasDocumentIOHandlerImageOut );
            if ( handler )
            {
                handler->SetShowObject( drawingpart->GetShowObject() );
                handler->SetBorder( 5, true );
            }
        }

        FileExport();
    }

    if ( !found )
        event.Skip();
    else
        a2dDocviewGlobals->SendToLogTarget();
}

bool a2dCentralCanvasCommandProcessor::SubmitToDrawing( a2dCommand* command, bool storeIt )
{
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawer )
        return false;
    if ( !drawer->GetDrawing() )
        return false;

    return drawer->GetDrawing()->GetCommandProcessor()->Submit( command, storeIt );
}

void a2dCentralCanvasCommandProcessor::Refresh()
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return;
    doc->GetDrawing()->GetRootObject()->Update( a2dCanvasObject::updatemask_force );
    a2dCanvasView* drawer = CheckDrawingView();
    if ( !drawer )
        return;

    drawer->Update( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_ALL );
}

bool a2dCentralCanvasCommandProcessor::SaveViewAsImage( a2dCanvasDocument* doc, const wxString& file, wxBitmapType type, a2dCanvasObject* from )
{
    a2dPathList path;
    path.Add( wxT( "." ) );
    wxString fileexp = file;
    if ( !path.ExpandPath( fileexp ) )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_CouldNotEvaluatePath, _( "Could not expand %s resulted in %s" ), fileexp.c_str(), fileexp.c_str() );
        return false;
    }
    if ( fileexp.IsEmpty() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_NotSpecified, _( "file name %s not in %s" ), fileexp.c_str(), path.GetAsString().c_str() );
        return false;
    }

    a2dCanvasDocumentIOHandlerImageOut imagehout( type );
    imagehout.SetBorder( 10 );
    imagehout.SetShowObject( from );

    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawer )
        return false;

    imagehout.SetYaxis( drawer->GetDrawer2D()->GetYaxis() );

    if ( true )
        imagehout.SetViewRectangle( drawer->GetDrawer2D()->GetVisibleBbox() );

#if wxUSE_STD_IOSTREAM
    a2dDocumentFileOutputStream store( fileexp.mb_str(), wxSTD ios_base::out | wxSTD ios_base::binary );
    if ( store.fail() || store.bad() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for saving" ), fileexp.c_str() );
        return false;
    }
#else
    wxFileOutputStream storeUnbuf( fileName );
    if ( storeUnbuf.GetLastError() != wxSTREAM_NO_ERROR )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for saving" ), fileexp.c_str() );
        return false;
    }
    wxBufferedOutputStream store( storeUnbuf );
#endif

    if ( ! doc->SaveObject( store, &imagehout ) )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_SaveFile, _( "Sorry, could not save document to file %s" ), file.c_str() );
        return false;
    }
    return true;
}

void a2dCentralCanvasCommandProcessor::FillLibraryPoints()
{
#if wxART2D_USE_CANEXTOBJ
    //root group always at 0,0
    m_librarypoints = new a2dCanvasDocument();

    {
        a2dCircle* cire = new  a2dCircle( 0, 0, 1 );
        cire->SetFill( wxColour( 219, 215, 6 ), a2dFILL_HORIZONTAL_HATCH );
        cire->SetStroke( wxColour( 1, 3, 205 ), 3.0 );
        cire->SetSpecificFlags( true, a2dCanvasOFlags::normalized );

        a2dLibraryReference* libobject = new a2dLibraryReference( 0, 0, cire, _T( "circle" ), 100 );
        libobject->SetStroke( wxColour( 229, 5, 64 ), 0 );
        m_librarypoints->GetDrawing()->GetRootObject()->Append( libobject );
    }

    {
        a2dArrow* arrow = new  a2dArrow( 0, 0, 1, 0.5, 0.5 );
        arrow->SetFill( wxColour( 219, 215, 6 ) );
        arrow->SetStroke( wxColour( 1, 3, 205 ), 1 );
        arrow->SetSpecificFlags( true, a2dCanvasOFlags::normalized );

        a2dLibraryReference* libobject = new a2dLibraryReference( 0, -200, arrow, _T( "arrow" ), 100 );
        libobject->SetStroke( wxColour( 229, 5, 64 ), 0 );
        m_librarypoints->GetDrawing()->GetRootObject()->Append( libobject );
    }

    {
        a2dArrow* arrow2 = new  a2dArrow( 0, 0, 2, 1, 2 );
        arrow2->SetFill( wxColour( 219, 2, 6 ) );
        arrow2->SetStroke( wxColour( 255, 3, 205 ), 1.0 );
        arrow2->SetSpecificFlags( true, a2dCanvasOFlags::normalized );

        a2dLibraryReference* libobject = new a2dLibraryReference( 0, -400, arrow2, _T( "arrow2" ), 100 );
        libobject->SetStroke( wxColour( 229, 5, 64 ), 0 );
        m_librarypoints->GetDrawing()->GetRootObject()->Append( libobject );
    }

    {
        a2dArrow* arrow3 = new  a2dArrow( 0, 0, 2, 2, 2 );
        arrow3->SetFill( wxColour( 2, 233, 6 ) );
        arrow3->SetStroke( wxColour( 255, 3, 25 ), 1 );
        arrow3->SetSpecificFlags( true, a2dCanvasOFlags::normalized );

        a2dLibraryReference* libobject = new a2dLibraryReference( 0, -400, arrow3, _T( "arrow3" ), 100 );
        libobject->SetStroke( wxColour( 229, 5, 64 ), 0 );
        m_librarypoints->GetDrawing()->GetRootObject()->Append( libobject );
    }

    {
        a2dArrow* arrow3 = new  a2dArrow( 0, 0, 2, 0, 2 );
        arrow3->SetFill( wxColour( 2, 233, 6 ) );
        arrow3->SetStroke( wxColour( 255, 3, 25 ), 1 );
        arrow3->SetSpecificFlags( true, a2dCanvasOFlags::normalized );

        a2dLibraryReference* libobject = new a2dLibraryReference( 0, -400, arrow3, _T( "arrow4" ), 100 );
        libobject->SetStroke( wxColour( 229, 5, 64 ), 0 );
        m_librarypoints->GetDrawing()->GetRootObject()->Append( libobject );
    }
    /*
        {
            wxBitmap bitmap( smile_xpm );
            a2dImage* i=new a2dImage( bitmap.ConvertToImage(), 0,0,1,1 );
            i->SetSpecificFlags( true, a2dCanvasOFlags::normalized );

            a2dLibraryReference* libobject = new a2dLibraryReference( 200, 0, i, _T("smilly"), 100);
            libobject->SetStroke(wxColour(229,5,64 ),0);
            m_librarypoints->Append( libobject );
        }
    */
    {
        a2dRect* rec3 = new a2dRect( -1, -1, 2, 2 );
        rec3->SetFill( wxColour( 0, 120, 240 ) );
        rec3->SetStroke( wxColour( 252, 54, 252 ), 3.0 );
        rec3->SetSpecificFlags( true, a2dCanvasOFlags::normalized );

        a2dLibraryReference* libobject = new a2dLibraryReference( 200, -200, rec3, _T( "rectangle" ), 100 );
        libobject->SetStroke( wxColour( 229, 5, 64 ), 0 );
        m_librarypoints->GetDrawing()->GetRootObject()->Append( libobject );
    }

#endif
}

void a2dCentralCanvasCommandProcessor::OnBusyBegin( a2dCommandProcessorEvent& WXUNUSED( event ) )
{
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawer )
        return;
    drawer->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_WAIT ) );
}

void a2dCentralCanvasCommandProcessor::OnBusyEnd( a2dCommandProcessorEvent& WXUNUSED( event ) )
{
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawer )
        return;
    drawer->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  ) );
}

void a2dCentralCanvasCommandProcessor::OnChangedDocument( a2dCommandEvent& event )
{
    event.Skip();
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return;

    // this has to be set when document changes.
    a2dGlobals->SetAberPolyToArc( double( a2dCanvasGlobals->GetHabitat()->GetAberPolyToArc() ) / doc->GetUnitsScale() );
    a2dGlobals->SetAberArcToPoly( double( a2dCanvasGlobals->GetHabitat()->GetAberArcToPoly() ) / doc->GetUnitsScale() );
}

double a2dCentralCanvasCommandProcessor::GetUnitsScaleOfDocument()
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return 1;

    return doc->GetUnitsScale();
}

bool a2dCentralCanvasCommandProcessor::SaveLayers( const wxFileName& fileName )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;

    a2dPathList path;
    path.Add( wxT( "." ) );
    wxString foundfile = fileName.GetFullPath();
    if ( !path.ExpandPath( foundfile ) )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_CouldNotEvaluatePath, _( "Could not expand %s resulted in %s" ), fileName.GetFullPath().c_str(), foundfile.c_str() );
        return false;
    }
    if ( foundfile.IsEmpty() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_NotSpecified, _( "Filename %s not in %s" ), foundfile.c_str(), path.GetAsString().c_str() );
        return false;
    }

    if ( doc->GetLayerSetup() )
        doc->GetLayerSetup()->SaveLayers( foundfile );
    else
        a2dDocviewGlobals->ReportErrorF( a2dError_NotSpecified, _( "Document does not have layer setup" ) );

    return true;
}

bool a2dCentralCanvasCommandProcessor::LoadLayers( const wxFileName& fileName )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return false;

    a2dPathList path;
    path.Add( wxT( "." ) );
    wxString foundfile = fileName.GetFullPath();
    if ( !path.ExpandPath( foundfile ) )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_CouldNotEvaluatePath, _( "Could not expand %s resulted in %s" ), fileName.GetFullPath().c_str(), foundfile.c_str() );
        return false;
    }
    if ( foundfile.IsEmpty() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_NotSpecified, _( "Filename %s not in %s" ), foundfile.c_str(), path.GetAsString().c_str() );
        return false;
    }

#if wxUSE_STD_IOSTREAM
    a2dDocumentFileInputStream store( foundfile.mb_str() );
    if ( store.fail() || store.bad() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for import" ), foundfile.c_str() );
        return false;
    }
#else
    wxFileInputStream storeUnbuf( foundfile );
    if ( !storeUnbuf.Ok() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for import" ), foundfile.c_str() );
        return false;
    }
    wxBufferedInputStream store( storeUnbuf );
#endif

#if wxART2D_USE_CVGIO
    //assume it is the default CVG format.
    a2dIOHandlerCVGIn* handler = new a2dIOHandlerCVGIn();
    a2dLayers* layersetup = new a2dLayers();
    handler->LoadLayers( store, layersetup );

    delete handler;
#else
    wxFAIL_MSG( wxT( "wxART2D_USE_CVGIO is needed to load object from a file CVG file" ) );
#endif //wxART2D_USE_CVGIO

    doc->SetLayerSetup( layersetup );
    return true;
}

a2dCanvasObject* a2dCentralCanvasCommandProcessor::AddCurrent( a2dCanvasObject* objectToAdd, bool withUndo, a2dPropertyIdList* setStyles, a2dCanvasObject* parentObject )
{
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawer )
        return NULL;
    if ( !drawer->GetDrawing() )
        return NULL;

    a2dDrawing* drawing = drawer->GetDrawing();
    return drawing->GetCanvasCommandProcessor()->AddCurrent( objectToAdd, withUndo, setStyles, parentObject );
}

/*
void a2dCentralCanvasCommandProcessor::SetStroke( const a2dStroke& stroke )
{
    a2dCentralCanvasCommandProcessor::PROPID_Stroke->SetPropertyToObject( this, stroke );
}

void a2dCentralCanvasCommandProcessor::SetStroke( const unsigned char red, const unsigned char green, const unsigned char blue )
{
    a2dCanvasObject* canobj = GetCurrentCanvasObject();
    a2dStroke col = GetStroke();
    col.SetColour( wxColour( red, green, blue ) );
    Submit( new a2dCommand_SetCanvasProperty( canobj, a2dCanvasObject::PROPID_Stroke, col ), m_withUndo );
}

void a2dCentralCanvasCommandProcessor::SetFill( const a2dFill& fill )
{
    a2dCentralCanvasCommandProcessor::PROPID_Fill->SetPropertyToObject( this, fill );
}

void a2dCentralCanvasCommandProcessor::SetFill( const unsigned char red, const unsigned char green, const unsigned char blue )
{
    a2dCanvasObject* canobj = GetCurrentCanvasObject();
    a2dFill col = GetFill();
    col.SetColour( wxColour( red, green, blue ) );
    Submit( new a2dCommand_SetCanvasProperty( canobj, a2dCanvasObject::PROPID_Fill, col ), m_withUndo );
}

void a2dCentralCanvasCommandProcessor::SetFont( const a2dFont& font )
{
    a2dCentralCanvasCommandProcessor::PROPID_Font->SetPropertyToObject( this, font );

    a2dCanvasObject* canobj = GetCurrentCanvasObject();
    if ( wxDynamicCast( canobj, a2dText ) )
    {
        //! todo set text to object.
//        Submit(new a2dCommand_SetCanvasProperty( canobj, a2dCanvasObject::PROPID_Stroke, GetDrawingCmdProcessor()->GetStroke()->Clone( clone_flat ) ), m_withUndo );
    }
}

void a2dCentralCanvasCommandProcessor::SetContourWidth( const a2dDoMu& currentContourWidth )
{
    m_currentContourWidth = currentContourWidth;
}
*/

/*
void a2dCentralCanvasCommandProcessor::SetStyle( a2dPropertyIdList* setStyles )
{
    if ( !m_currentobject )
        return;

    if ( !setStyles )
        return;

    if( setStyles->Find( &a2dCentralCanvasCommandProcessor::PROPID_Stroke ))
        m_currentobject->SetStroke( m_currentstroke );

    if( setStyles->Find( &a2dCentralCanvasCommandProcessor::PROPID_Fill ))
        m_currentobject->SetFill( m_currentfill );

    if( setStyles->Find( &a2dCentralCanvasCommandProcessor::PROPID_Layer ))
        m_currentobject->SetLayer( m_layer );

    if( setStyles->Find( &a2dCentralCanvasCommandProcessor::PROPID_Spline ))
        a2dCanvasObject::PROPID_spline->SetPropertyToObject( m_currentobject, m_spline );

    if( setStyles->Find( &a2dCentralCanvasCommandProcessor::PROPID_begin ))
        a2dCanvasObject::PROPID_begin->SetPropertyToObject( m_currentobject, m_currentBegin );

    if( setStyles->Find( &a2dCentralCanvasCommandProcessor::PROPID_end ))
        a2dCanvasObject::PROPID_end->SetPropertyToObject( m_currentobject, m_currentEnd );

    if( setStyles->Find( &a2dCentralCanvasCommandProcessor::PROPID_endScaleX ))
        a2dCanvasObject::PROPID_endScaleX->SetPropertyToObject( m_currentobject, m_currentXscale );

    if( setStyles->Find( &a2dCentralCanvasCommandProcessor::PROPID_endScaleY ))
        a2dCanvasObject::PROPID_endScaleY->SetPropertyToObject( m_currentobject, m_currentYscale );

}
*/

bool a2dCentralCanvasCommandProcessor::CvgString( const wxString& cvgString )
{
    if ( !m_currentDocument )
        return false;

    wxString cvgStringtoExecute = cvgString;

#if wxART2D_USE_CVGIO
    if ( cvgString.Contains( _T( "<?xml" ) ) && cvgString.Contains( _T( "<cvg" ) ) && cvgString.Contains( _T( "</cvg>" ) )  )
    {
        //almost certain the user meant to enter a complete valid CVG document
    }
    else
    {
        //only partly a valid CVG document, make it complete now
        cvgStringtoExecute = _T( "<?xml version=\"1.0\" standalone=\"yes\"?> <cvg classname=\"a2dCanvasDocument\"> " ) + cvgString + _T( " </cvg>" );
    }

#if wxUSE_STD_IOSTREAM
#if wxUSE_UNICODE
    a2dDocumentStringInputStream stream( cvgStringtoExecute.mb_str().data(), wxSTD ios_base::in );
#else
    a2dDocumentStringInputStream stream( cvgStringtoExecute.mb_str(), wxSTD ios_base::in );
#endif // wxUSE_UNICODE
#else
    a2dDocumentStringInputStream stream( cvgStringtoExecute );
#endif
    a2dIOHandlerCVGIn CvgString;

    return CvgString.Load( stream, ( a2dDrawing* ) m_currentDocument, GetCanvasDocument()->GetDrawing()->GetRootObject() );

#else
    return false;
#endif //wxART2D_USE_CVGIO
}

//--------------------- start tool commands ------------------

bool a2dCentralCanvasCommandProcessor::SetPropertyToObject( const wxString& objectname, const wxString& propertyname, const wxString& value )
{
    if ( objectname == _T( "a2dBaseTool" ) )
    {
        if ( !m_currentDocument )
            return false;

        a2dBaseTool* tool = GetActiveTool();

        if ( tool )
            return GetDrawingCmdProcessor()->SetOrAddPropertyToObject( tool, propertyname, value, m_withUndo );

        return false;
    }
    else if ( objectname == _T( "a2dToolContr" ) )
    {
        if ( !m_currentDocument )
            return false;

        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( drawer )
        {
            a2dToolContr* contr = drawer->GetCanvasToolContr();
            return GetDrawingCmdProcessor()->SetOrAddPropertyToObject( contr, propertyname, value, m_withUndo );
        }

        return false;
    }
    else if ( objectname == _T( "a2dCanvasView" ) )
    {
        if ( !m_currentDocument )
            return false;

        a2dCanvasView* drawer = CheckDrawingView();

        if ( drawer )
            return SetOrAddPropertyToObject( drawer, propertyname, value, m_withUndo );
        else
            a2dDocviewGlobals->ReportErrorF( a2dError_NoView, _( "No a2dCanvasView View available" ) );

        return false;
    }
    else if ( objectname == _T( "a2dCanvasCommandProcessor" ) )
    {
        if ( !m_currentDocument )
            return false;

        return GetDrawingCmdProcessor()->SetOrAddPropertyToObject( GetDrawingCmdProcessor(), propertyname, value, m_withUndo );
    }
    else if ( objectname == _T( "a2dCentralCanvasCommandProcessor" ) )
    {
        if ( !m_currentDocument )
            return false;

        return SetOrAddPropertyToObject( this, propertyname, value, m_withUndo );
    }
    return false;
}

bool a2dCentralCanvasCommandProcessor::SetPropertyToObject( const wxString& objectname, a2dNamedProperty* property )
{
    //!todo HOW undo menu's work when commands endsup on this commandprocessor its commandstack.

    //for the moment command are submitted to the object which is related to the document.
    //e.g. a2dCanvasView is clearly showing a document.

    if ( objectname == _T( "a2dBaseTool" ) )
    {
        if ( !m_currentDocument )
            return false;

        a2dBaseTool* tool = GetActiveTool();

        if ( tool )
            return SetOrAddPropertyToObject( tool, property, m_withUndo );

        return false;
    }
    else if ( objectname == _T( "a2dToolContr" ) )
    {
        if ( !m_currentDocument )
            return false;

        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( drawer )
        {
            a2dToolContr* contr = drawer->GetCanvasToolContr();
            return SetOrAddPropertyToObject( contr, property, m_withUndo );
        }

        return false;
    }
    else if ( objectname == _T( "a2dCanvasView" ) )
    {
        if ( !m_currentDocument )
            return false;

        a2dCanvasView* drawer = CheckDrawingView();

        if ( drawer )
            return SetOrAddPropertyToObject( drawer, property, m_withUndo );
        else
            a2dDocviewGlobals->ReportErrorF( a2dError_NoView, _( "No a2dCanvasView View available" ) );

        return false;
    }
    else if ( objectname == _T( "a2dCanvasCommandProcessor" ) )
    {
        if ( !m_currentDocument )
            return false;

        return GetDrawingCmdProcessor()->SetOrAddPropertyToObject( GetDrawingCmdProcessor(), property, m_withUndo );
    }
    else if ( objectname == _T( "a2dCentralCanvasCommandProcessor" ) )
    {
        if ( !m_currentDocument )
            return false;

        return SetOrAddPropertyToObject( this, property, m_withUndo );
    }
    return false;
}

bool a2dCentralCanvasCommandProcessor::SetLayerProperty( long layer,
        const wxString& propertyname,
        const wxString& value
                                                       )
{
    if ( !m_currentDocument )
        return false;

    a2dLayers* layers = ( ( a2dCanvasDocument* ) m_currentDocument )->GetLayerSetup();
    if ( !layers )
        return false;

    a2dLayerInfo* layerinfodoc = layers->GetLayerIndex()[ layer ];

    bool res = GetDrawingCmdProcessor()->SetOrAddPropertyToObject( layerinfodoc, propertyname, value, m_withUndo );

    return res;
}

a2dBaseTool* a2dCentralCanvasCommandProcessor::GetActiveTool()
{
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( drawer )
    {
        a2dToolContr* contr = drawer->GetCanvasToolContr();
        return contr->GetFirstTool();
    }
    return NULL;
}

a2dBaseTool* a2dCentralCanvasCommandProcessor::PopTool()
{
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( drawer )
    {
        a2dToolContr* contr = drawer->GetCanvasToolContr();
        if ( contr )
        {
            a2dSmrtPtr< a2dBaseTool > tool;
            contr->PopTool( tool );
            if ( ! tool )
                a2dDocviewGlobals->ReportError( a2dError_NoTool );
            return ( a2dBaseTool* ) tool.Get();
        }
        a2dDocviewGlobals->ReportError( a2dError_NoController );
        return NULL;
    }
    a2dDocviewGlobals->ReportError( a2dError_NoView );
    return NULL;
}

//--------------------- drawer related commands ------------------

bool a2dCentralCanvasCommandProcessor::SetShowObject( int index )
{
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( drawer )
    {
        a2dCanvasObject* top = wxStaticCast( drawer->GetDrawing()->GetRootObject()->Find( _T( "" ), _T( "" ), index ), a2dCanvasObject );
        if ( !top )
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_NotSpecified, _T( "No Object with id %d in a2dCentralCanvasCommandProcessor::SetShowObject()" ), index );
            return false;
        }
        drawer->SetShowObject( top );
        return true;
    }

    a2dDocviewGlobals->ReportError( a2dError_NoView );
    return false;
}

void a2dCentralCanvasCommandProcessor::SetGridX( const a2dDoMu& gridX )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return;

    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    a2dCanvasGlobals->GetHabitat()->m_gridX = gridX;
    if ( drawer )
    {
        drawer->SetGridX( double( a2dCanvasGlobals->GetHabitat()->m_gridX ) / doc->GetUnitsScale() );
    }
}

void a2dCentralCanvasCommandProcessor::SetGridY( const a2dDoMu& gridY )
{
    a2dCanvasDocument* doc = CheckCanvasDocument();
    if ( !doc )
        return;

    a2dCanvasGlobals->GetHabitat()->m_gridY = gridY;
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( drawer )
    {
        drawer->SetGridY( double( a2dCanvasGlobals->GetHabitat()->m_gridY ) / doc->GetUnitsScale() );
    }
}

bool a2dCentralCanvasCommandProcessor::Zoom( double x1, double y1, double x2, double y2, bool upp )
{
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( drawer )
    {
		drawer->GetDrawing()->GetCanvasCommandProcessor()->Zoom( x1, y1, x2, y2, upp );
        return true;
    }

    a2dDocviewGlobals->ReportError( a2dError_NoView );
    return false;
}

bool a2dCentralCanvasCommandProcessor::Update()
{
    a2dCanvasView* drawer = CheckDrawingView();
    if ( drawer )
    {
        drawer->Update( a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );
        return true;
    }

    a2dDocviewGlobals->ReportError( a2dError_NoView );
    return false;
}

void a2dCentralCanvasCommandProcessor::SetCurrentView( a2dView* view )
{
    //deactivate the current view if different from the new one.
    if ( m_currentView )
    {
        if ( m_currentView != view || m_currentView->IsClosed() )
        {
            m_currentView->DisconnectEvent( wxEVT_DO, this );
            m_currentView->DisconnectEvent( wxEVT_UNDO, this );
            m_currentView->DisconnectEvent( wxEVT_REDO, this );
            m_currentView->DisconnectEvent( wxEVT_CANUNDO, this );
            m_currentView->DisconnectEvent( wxEVT_CANREDO, this );
            if ( view )
            {
                view->ConnectEvent( a2dEVT_COM_EVENT, this );
                view->ConnectEvent( wxEVT_DO, this );
                view->ConnectEvent( wxEVT_UNDO, this );
                view->ConnectEvent( wxEVT_REDO, this );
                view->ConnectEvent( wxEVT_CANUNDO, this );
                view->ConnectEvent( wxEVT_CANREDO, this );
            }
        }
        // the base sets the new document of which this view is a part
        a2dDocumentCommandProcessor::SetCurrentView( view );
        
        a2dCanvasDocument* candoc = CheckCanvasDocument();
        if ( candoc )
            a2dCameleon::SetCameleonRoot( candoc->GetDrawing()->GetRootObject() );
		else
			a2dCameleon::SetCameleonRoot( NULL );
    }
    else if ( view )
    {   
        view->ConnectEvent( a2dEVT_COM_EVENT, this );
        view->ConnectEvent( wxEVT_DO, this );
        view->ConnectEvent( wxEVT_UNDO, this );
        view->ConnectEvent( wxEVT_REDO, this );
        view->ConnectEvent( wxEVT_CANUNDO, this );
        view->ConnectEvent( wxEVT_CANREDO, this );
        // the base sets the new document of which this view is a part
        a2dDocumentCommandProcessor::SetCurrentView( view );

        a2dCanvasDocument* candoc = CheckCanvasDocument();
        if ( candoc )
            a2dCameleon::SetCameleonRoot( candoc->GetDrawing()->GetRootObject() );
		else
			a2dCameleon::SetCameleonRoot( NULL );
    }
    //wxLogDebug( "view: %p camroot: %p ", view, a2dCameleon::GetCameleonRoot() );
}

a2dCanvasView* a2dCentralCanvasCommandProcessor::CheckDrawingView() const
{
    if ( !m_currentDocument )
        return NULL;

    if ( !GetCurrentView() )
    {
        return NULL;
    }

    return wxDynamicCast( GetCurrentView(), a2dCanvasView );
}

a2dCanvasDocument* a2dCentralCanvasCommandProcessor::CheckCanvasDocument() const
{
    if ( !m_currentDocument )
        return NULL;

    return wxDynamicCast( m_currentDocument, a2dCanvasDocument );
}

bool a2dCentralCanvasCommandProcessor::IsShownStructureDlg() { return m_structDlgDoc != NULL && m_structDlgDoc->IsShown(); }

void a2dCentralCanvasCommandProcessor::DeleteDlgs()
{
    if ( m_layersdlg )
        m_layersdlg->Destroy();

    if ( m_layerOrderDlg )
        m_layerOrderDlg->Destroy();

    if ( m_structDlgDoc )
        m_structDlgDoc->Destroy();

    m_layersdlg = NULL;
    m_layerOrderDlg = NULL;
    m_structDlgDoc = NULL;
}

void a2dCentralCanvasCommandProcessor::OnExit()
{
    a2dCanvasGlobals->SetActiveDrawingPart( NULL );
    DeleteDlgs();
}

bool a2dCentralCanvasCommandProcessor::ShowDlg( const a2dCommandId* comID, bool modal, bool onTop )
{
    return false;
}

void a2dCentralCanvasCommandProcessor::SetBooleanEngineMarge( const a2dDoMu& marge )
    { a2dCanvasGlobals->GetHabitat()->SetBooleanEngineMarge( marge ); }
void a2dCentralCanvasCommandProcessor::SetBooleanEngineGrid( long grid )
    { a2dCanvasGlobals->GetHabitat()->SetBooleanEngineGrid( grid ); }
void a2dCentralCanvasCommandProcessor::SetBooleanEngineDGrid( double dgrid )
    { a2dCanvasGlobals->GetHabitat()->SetBooleanEngineDGrid( dgrid ); }
void a2dCentralCanvasCommandProcessor::SetBooleanEngineCorrectionAber( const a2dDoMu& aber )
    { a2dCanvasGlobals->GetHabitat()->SetBooleanEngineCorrectionAber( aber ); }
void a2dCentralCanvasCommandProcessor::SetBooleanEngineCorrectionFactor( const a2dDoMu& aber )
    { a2dCanvasGlobals->GetHabitat()->SetBooleanEngineCorrectionFactor( aber ); }
void a2dCentralCanvasCommandProcessor::SetBooleanEngineMaxlinemerge( const a2dDoMu& maxline )
    { a2dCanvasGlobals->GetHabitat()->SetBooleanEngineMaxlinemerge( maxline ); }
void a2dCentralCanvasCommandProcessor::SetBooleanEngineWindingRule( bool rule )
    { a2dCanvasGlobals->GetHabitat()->SetBooleanEngineWindingRule( rule ); }
void a2dCentralCanvasCommandProcessor::SetBooleanEngineRoundfactor( double roundfac )
    { a2dCanvasGlobals->GetHabitat()->SetBooleanEngineRoundfactor( roundfac ) ; }
void a2dCentralCanvasCommandProcessor::SetBooleanEngineSmoothAber( const a2dDoMu& aber )
    { a2dCanvasGlobals->GetHabitat()->SetBooleanEngineSmoothAber( aber ); }
void a2dCentralCanvasCommandProcessor::SetBooleanEngineLinkHoles( bool doLinkHoles )
    { a2dCanvasGlobals->GetHabitat()->SetBooleanEngineLinkHoles( doLinkHoles ); }
void a2dCentralCanvasCommandProcessor::SetRadiusMin( const a2dDoMu& radiusMin )
    { a2dCanvasGlobals->GetHabitat()->SetRadiusMin( radiusMin ); }
void a2dCentralCanvasCommandProcessor::SetRadiusMax( const a2dDoMu& radiusMax )
    { a2dCanvasGlobals->GetHabitat()->SetRadiusMax( radiusMax ) ; }
void a2dCentralCanvasCommandProcessor::SetAberPolyToArc( const a2dDoMu& aber )
    { a2dCanvasGlobals->GetHabitat()->SetAberPolyToArc( aber ); }
void a2dCentralCanvasCommandProcessor::SetAberArcToPoly( const a2dDoMu& aber )
    { a2dCanvasGlobals->GetHabitat()->SetAberArcToPoly( aber ); }
void a2dCentralCanvasCommandProcessor::SetSmall( const a2dDoMu& smallest )
    { a2dCanvasGlobals->GetHabitat()->SetSmall( smallest ); }

bool a2dCentralCanvasCommandProcessor::GdsIoSaveTextAsPolygon( bool onOff )
{ 
    bool was = a2dCanvasGlobals->GetHabitat()->GdsIoSaveTextAsPolygon( onOff );
    const_forEachIn( a2dDocumentTemplateList, &GetDocTemplates() )
    {
        a2dDocumentTemplateList::value_type temp = *iter;
        if ( temp->IsVisible() && wxT( "GDS-II" ) == temp->GetDescription() )
        {
#if wxART2D_USE_GDSIO
            wxStaticCast( temp->GetDocumentIOHandlerStrOut(), a2dIOHandlerGDSOut )->SetSaveTextAsVpath( onOff );
#endif //wxART2D_USE_GDSIO
        }
    }
    return was; 
}

bool a2dCentralCanvasCommandProcessor::KeyIoSaveTextAsPolygon( bool onOff )
{ 
    bool was = a2dCanvasGlobals->GetHabitat()->KeyIoSaveTextAsPolygon( onOff );
    const_forEachIn( a2dDocumentTemplateList, &GetDocTemplates() )
    {
        a2dDocumentTemplateList::value_type temp = *iter;
        if ( temp->IsVisible() && wxT( "KEY" ) == temp->GetDescription() )
        {
#if wxART2D_USE_KEYIO
            wxStaticCast( temp->GetDocumentIOHandlerStrOut(), a2dIOHandlerKeyOut )->SetSaveTextAsVpath( onOff );
#endif //wxART2D_USE_KEYIO
        }
    }
    return was; 
}

bool a2dCentralCanvasCommandProcessor::GdsIISaveFromView( bool onOff )
{ 
    bool was = a2dCanvasGlobals->GetHabitat()->m_gdsiiSaveFromView;
    const_forEachIn( a2dDocumentTemplateList, &GetDocTemplates() )
    {
        a2dDocumentTemplateList::value_type temp = *iter;
        if ( temp->IsVisible() && wxT( "GDS-II" ) == temp->GetDescription() )
        {
#if wxART2D_USE_GDSIO
            wxStaticCast( temp->GetDocumentIOHandlerStrOut(), a2dIOHandlerGDSOut )->SetFromViewAsTop( onOff );
#endif //wxART2D_USE_GDSIO
        }
    }
    return was; 
}

bool a2dCentralCanvasCommandProcessor::KeySaveFromView( bool onOff )
{ 
    bool was = a2dCanvasGlobals->GetHabitat()->m_keySaveFromView;
    const_forEachIn( a2dDocumentTemplateList, &GetDocTemplates() )
    {
        a2dDocumentTemplateList::value_type temp = *iter;
        if ( temp->IsVisible() && wxT( "KEY" ) == temp->GetDescription() )
        {
#if wxART2D_USE_KEYIO
            wxStaticCast( temp->GetDocumentIOHandlerStrOut(), a2dIOHandlerKeyOut )->SetFromViewAsTop( onOff );
#endif //wxART2D_USE_KEYIO
        }
    }
    return was; 
}


const a2dDoMu&  a2dCentralCanvasCommandProcessor::GetRadiusMin() const
    { return a2dCanvasGlobals->GetHabitat()->GetRadiusMin(); }
const a2dDoMu& a2dCentralCanvasCommandProcessor::GetRadiusMax() const
    { return a2dCanvasGlobals->GetHabitat()->GetRadiusMax(); }
const a2dDoMu& a2dCentralCanvasCommandProcessor::GetAberPolyToArc() const
    { return a2dCanvasGlobals->GetHabitat()->GetAberPolyToArc(); }
const a2dDoMu& a2dCentralCanvasCommandProcessor::GetAberArcToPoly() const
    { return a2dCanvasGlobals->GetHabitat()->GetAberArcToPoly(); }
const a2dDoMu& a2dCentralCanvasCommandProcessor::GetSmall() const
    { return a2dCanvasGlobals->GetHabitat()->GetSmall(); }

const a2dDoMu& a2dCentralCanvasCommandProcessor::GetBooleanEngineMarge() const
    { return a2dCanvasGlobals->GetHabitat()->GetBooleanEngineMarge(); }
long a2dCentralCanvasCommandProcessor::GetBooleanEngineGrid() const
    { return a2dCanvasGlobals->GetHabitat()->GetBooleanEngineGrid(); }
double a2dCentralCanvasCommandProcessor::GetBooleanEngineDGrid() const
    { return a2dCanvasGlobals->GetHabitat()->GetBooleanEngineDGrid(); }
const a2dDoMu& a2dCentralCanvasCommandProcessor::GetBooleanEngineCorrectionAber() const
    { return a2dCanvasGlobals->GetHabitat()->GetBooleanEngineCorrectionAber(); }
const a2dDoMu& a2dCentralCanvasCommandProcessor::GetBooleanEngineCorrectionFactor() const
    { return a2dCanvasGlobals->GetHabitat()->GetBooleanEngineCorrectionFactor(); }
const a2dDoMu& a2dCentralCanvasCommandProcessor::GetBooleanEngineMaxlinemerge() const
    { return a2dCanvasGlobals->GetHabitat()->GetBooleanEngineMaxlinemerge(); }
bool a2dCentralCanvasCommandProcessor::GetBooleanEngineWindingRule() const
    { return a2dCanvasGlobals->GetHabitat()->GetBooleanEngineWindingRule(); }
double a2dCentralCanvasCommandProcessor::GetBooleanEngineRoundfactor() const
    { return a2dCanvasGlobals->GetHabitat()->GetBooleanEngineRoundfactor(); }
const a2dDoMu& a2dCentralCanvasCommandProcessor::GetBooleanEngineSmoothAber() const
    { return a2dCanvasGlobals->GetHabitat()->GetBooleanEngineSmoothAber(); }
bool a2dCentralCanvasCommandProcessor::GetBooleanEngineLinkHoles() const 
    { return a2dCanvasGlobals->GetHabitat()->GetBooleanEngineLinkHoles(); }
