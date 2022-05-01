/*! \file wx/canvas/drawing.h
    \brief Contains a2dDrawing Class to hold a drawing.

    a2dDrawing is the class where a hierarchy of a2dCanvasObject's is stored.
    Updating pending/changed objects and drawing a2dDrawing and a2dDrawingPart is all from this class.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: drawing.h,v 1.31 2009/07/10 19:23:13 titato Exp $
*/

#ifndef __WXDRAWING_H__
#define __WXDRAWING_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include <wx/numdlg.h>
#include "a2dprivate.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/vpath.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/polygon.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/recur.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/canprop.h"
#include "wx/canvas/sttool.h"

class a2dCanvasCommandProcessor;
class a2dIOHandlerCVGIn;
class a2dIOHandlerCVGOut;
class a2dDrawingPart;
class a2dCameleonEvent;

//----------------------------------------------------------------------------
// decls
//----------------------------------------------------------------------------
DECLARE_MENU_ITEMID( CmdMenu_Drawing_FileClose )
DECLARE_MENU_ITEMID( CmdMenu_Drawing_FileOpen )
DECLARE_MENU_ITEMID( CmdMenu_Drawing_FileNew )
DECLARE_MENU_ITEMID( CmdMenu_Drawing_FileSave )
DECLARE_MENU_ITEMID( CmdMenu_Drawing_FileSaveAs )

DECLARE_MENU_ITEMID( CmdMenu_ChangeCanvasObjectStyle )
DECLARE_MENU_ITEMID( CmdMenu_ChangeCentralStyle )
DECLARE_MENU_ITEMID( CmdMenu_SetLayer )
DECLARE_MENU_ITEMID( CmdMenu_SetTarget )
DECLARE_MENU_ITEMID( CmdMenu_SetDrawWireFrame )
DECLARE_MENU_ITEMID( CmdMenu_DrawGridLines )
DECLARE_MENU_ITEMID( CmdMenu_DrawGridAtFront )
DECLARE_MENU_ITEMID( CmdMenu_SetYAxis )
DECLARE_MENU_ITEMID( CmdMenu_Option_SplinePoly )
DECLARE_MENU_ITEMID( CmdMenu_Option_CursorCrosshair )
DECLARE_MENU_ITEMID( CmdMenu_Option_RescaleAtReSize )
DECLARE_MENU_ITEMID( CmdMenu_Option_ReverseLayers )

DECLARE_MENU_ITEMID( CmdMenu_RouteMethod_StraightEndSegment )
DECLARE_MENU_ITEMID( CmdMenu_RouteMethod_StraightEndSegmentUnlessPins )
DECLARE_MENU_ITEMID( CmdMenu_RouteMethod_StraightSegment )
DECLARE_MENU_ITEMID( CmdMenu_RouteMethod_StraightSegmentUnlessPins )
DECLARE_MENU_ITEMID( CmdMenu_RouteMethod_GridRouting )
DECLARE_MENU_ITEMID( CmdMenu_RouteMethod_ManhattanSegments )
DECLARE_MENU_ITEMID( CmdMenu_RouteMethod_ManhattanEndSegments )
DECLARE_MENU_ITEMID( CmdMenu_RouteMethod_ManhattanEndSegmentsStay )
DECLARE_MENU_ITEMID( CmdMenu_RouteMethod_ManhattanEndSegmentsConvertAndStay )

DECLARE_MENU_ITEMID( CmdMenu_RotateObject90Left )
DECLARE_MENU_ITEMID( CmdMenu_RotateObject90Right )

DECLARE_MENU_ITEMID( CmdMenu_Refresh )
DECLARE_MENU_ITEMID( CmdMenu_ZoomOut )
DECLARE_MENU_ITEMID( CmdMenu_ZoomOut2 )
DECLARE_MENU_ITEMID( CmdMenu_ZoomIn2 )
DECLARE_MENU_ITEMID( CmdMenu_ZoomOut2CondSelected )
DECLARE_MENU_ITEMID( CmdMenu_ZoomIn2CondSelected )
DECLARE_MENU_ITEMID( CmdMenu_EmptyShownObject )
DECLARE_MENU_ITEMID( CmdMenu_DrawGrid )
DECLARE_MENU_ITEMID( CmdMenu_SetSnap )
DECLARE_MENU_ITEMID( CmdMenu_SetSnapFeatures )
DECLARE_MENU_ITEMID( CmdMenu_SetSnapFeature )
DECLARE_MENU_ITEMID( CmdMenu_SetStroke )
DECLARE_MENU_ITEMID( CmdMenu_SetFill )
DECLARE_MENU_ITEMID( CmdMenu_SetStrokeColour )
DECLARE_MENU_ITEMID( CmdMenu_SetFillColour )
DECLARE_MENU_ITEMID( CmdMenu_SetContourWidth )
DECLARE_MENU_ITEMID( CmdMenu_SetPathType )
DECLARE_MENU_ITEMID( CmdMenu_SetNormalizeFactor )

DECLARE_MENU_ITEMID( CmdMenu_ShowDiagram )
DECLARE_MENU_ITEMID( CmdMenu_ShowSymbol )
DECLARE_MENU_ITEMID( CmdMenu_ShowGui )


DECLARE_MENU_ITEMID( CmdMenu_PushInto )
DECLARE_MENU_ITEMID( CmdMenu_PopOut )
DECLARE_MENU_ITEMID( CmdMenu_NewGroup )
DECLARE_MENU_ITEMID( CmdMenu_NewPin )
DECLARE_MENU_ITEMID( CmdMenu_SaveLayersDrawing )
DECLARE_MENU_ITEMID( CmdMenu_SaveLayersDrawingHome )
DECLARE_MENU_ITEMID( CmdMenu_SaveLayersDrawingLastDir )
DECLARE_MENU_ITEMID( CmdMenu_SaveLayersDrawingCurrentDir )
DECLARE_MENU_ITEMID( CmdMenu_LoadLayersDrawing )
DECLARE_MENU_ITEMID( CmdMenu_LoadLayersDrawingHome )
DECLARE_MENU_ITEMID( CmdMenu_LoadLayersDrawingLastDir )
DECLARE_MENU_ITEMID( CmdMenu_LoadLayersDrawingCurrentDir )
DECLARE_MENU_ITEMID( CmdMenu_AddLayerIfMissing )

DECLARE_MENU_ITEMID( CmdMenu_SetCursor )
DECLARE_MENU_ITEMID( CmdMenu_SetRelativeStart )
DECLARE_MENU_ITEMID( CmdMenu_AddMeasurement )
DECLARE_MENU_ITEMID( CmdMenu_ClearMeasurements )

DECLARE_MENU_ITEMID( CmdMenu_Selected_SelectAll )
DECLARE_MENU_ITEMID( CmdMenu_Selected_DeSelectAll )
DECLARE_MENU_ITEMID( CmdMenu_Selected_SelectAll_NoUndo )
DECLARE_MENU_ITEMID( CmdMenu_Selected_DeSelectAll_NoUndo )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Delete )
DECLARE_MENU_ITEMID( CmdMenu_Selected_GroupNamed )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Group )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ImageToRectangles )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ToTop )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ToBack )
DECLARE_MENU_ITEMID( CmdMenu_Selected_SetStyle )
DECLARE_MENU_ITEMID( CmdMenu_Selected_SetExtStyle )
DECLARE_MENU_ITEMID( CmdMenu_Selected_SetTextChanges )
DECLARE_MENU_ITEMID( CmdMenu_Selected_SetTextChanges2 )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ConvertToPolygonPolylinesWithArcs )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ConvertToPolygonPolylinesWithoutArcs )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ConvertToPolylines )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ConvertLinesArcs )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ConvertToVPaths )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ConvertPolygonToArcs )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ConvertPolylineToArcs )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ConvertPolygonToSurface )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ConvertSurfaceToPolygon )
DECLARE_MENU_ITEMID( CmdMenu_Selected_ConvertPathToPolygon )
DECLARE_MENU_ITEMID( CmdMenu_Selected_UnGroup )
DECLARE_MENU_ITEMID( CmdMenu_Selected_UnGroupDeep )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Merge )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Offset )
DECLARE_MENU_ITEMID( CmdMenu_Selected_CreateRing )
DECLARE_MENU_ITEMID( CmdMenu_Selected_CameleonDiagram )
DECLARE_MENU_ITEMID( CmdMenu_Selected_CameleonSymbol )
DECLARE_MENU_ITEMID( CmdMenu_Selected_CameleonSymbolDiagram )
DECLARE_MENU_ITEMID( CmdMenu_Selected_FlattenCameleon )
DECLARE_MENU_ITEMID( CmdMenu_Selected_CloneCameleonFromInst )
DECLARE_MENU_ITEMID( CmdMenu_Selected_MoveXY )
DECLARE_MENU_ITEMID( CmdMenu_Selected_MoveLayer )
DECLARE_MENU_ITEMID( CmdMenu_Selected_CopyXY )
DECLARE_MENU_ITEMID( CmdMenu_Selected_CopyLayer )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Transform )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Rotate )

DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MinX )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MaxX )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MinY )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MaxY )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MidX )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MidY )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorVert )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorHorz )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorVertBbox )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorHorzBbox )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_DistributeVert )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_DistributeHorz )
DECLARE_MENU_ITEMID( CmdMenu_Selected_RotateObject90Left )
DECLARE_MENU_ITEMID( CmdMenu_Selected_RotateObject90Right )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MinX_Dest )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MaxX_Dest )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MinY_Dest )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MaxY_Dest )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MidX_Dest )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MidY_Dest )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorVert_Dest )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorHorz_Dest )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorVertBbox_Dest )
DECLARE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorHorzBbox_Dest )


DECLARE_MENU_ITEMID( CmdMenu_GroupAB_Offset )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_Smooth )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_CreateRing )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_Delete )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_Move )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_Copy )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_ToArcs )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_Or )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_And )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_Exor )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_AsubB )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_BsubA )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_ConvertPolygonToSurface )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_ConvertSurfaceToPolygon )
DECLARE_MENU_ITEMID( CmdMenu_GroupAB_ConvertPointsAtDistance )


DECLARE_MENU_ITEMID( CmdMenu_PopTool )


DECLARE_MENU_ITEMID( CmdMenu_PushTool_Zoom_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Select_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Select2_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_RecursiveEdit_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_MultiEdit_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Drag_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DragOriginal_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_RenderImageZoom_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Copy_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Rotate_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Delete_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawText_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Image_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Image_Embedded_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawRectangle_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawCircle_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawLine_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawLineScaledArrow_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawEllipse_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawEllipticArc_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawEllipticArc_Chord_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawArc_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawArc_Chord_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawPolylineL_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawPolylineL_Splined_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawPolygonL_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawPolygonL_Splined_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Property_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_CopyMulti_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Measure_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawWirePolylineL_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawVPath_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Origin_OneShot )

DECLARE_MENU_ITEMID( CmdMenu_PushTool_Zoom )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Select )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Select2 )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_RecursiveEdit )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_MultiEdit )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Drag )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DragOriginal )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_RenderImageZoom )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Copy )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Rotate )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Delete )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawText )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Image )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Image_Embedded )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawRectangle )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawCircle )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawLine )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawLineScaledArrow )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawEllipse )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawEllipticArc )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawEllipticArc_Chord )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawArc )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawArc_Chord )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawPolylineL )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawPolylineL_Splined )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawPolygonL )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawPolygonL_Splined )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Property )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DragMulti )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_CopyMulti )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Measure )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawWirePolylineL )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_DrawVPath )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Port )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Cameleon )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Cameleon_BuildIn )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_CameleonInst )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_CameleonInstDlg )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_CameleonRefDlg )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_CameleonRefDlg_Diagram )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_CameleonRefDlg_Symbol )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_TagGroups )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_TagGroups_WireMode )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_TagGroups_DlgMode )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_TagGroups_WireMode_DlgMode )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_SingleClick )

DECLARE_MENU_ITEMID( CmdMenu_InsertGroupRef )

DECLARE_MENU_ITEMID( CmdMenu_ShowDlgLayersDrawing )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgLayersGlobal )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgLayerOrderDrawing )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgLayerOrderGlobal )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgTools )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgStyle )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgSnap )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgGroups )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgIdentify )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgCoordEntry )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgMeasure )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgStructure )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgStructureModal )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgCameleonModal )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgPathSettings )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgSettings )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgTrans )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgPropEdit )

DECLARE_MENU_ITEMID( CmdMenu_SetFont )
 
DECLARE_MENU_ITEMID( CmdMenu_SetMasterTool_SelectFirst )
DECLARE_MENU_ITEMID( CmdMenu_SetMasterTool_ZoomFirst )
DECLARE_MENU_ITEMID( CmdMenu_SetMasterTool_TagGroups )
DECLARE_MENU_ITEMID( CmdMenu_SetMasterTool_TagGroups_WireMode )
DECLARE_MENU_ITEMID( CmdMenu_SetMasterTool_TagGroups_WireMode_DlgMode )
DECLARE_MENU_ITEMID( CmdMenu_SetMasterTool_TagGroups_DlgMode )
DECLARE_MENU_ITEMID( CmdMenu_SetMasterTool_SingleClick )
DECLARE_MENU_ITEMID( CmdMenu_LineBegin )
DECLARE_MENU_ITEMID( CmdMenu_LineEnd )
DECLARE_MENU_ITEMID( CmdMenu_LineScale )

DECLARE_MENU_ITEMID( CmdMenu_SetLayerDlg )
DECLARE_MENU_ITEMID( CmdMenu_SetTargetDlg )
DECLARE_MENU_ITEMID( CmdMenu_SetLayerDlgModeless )
DECLARE_MENU_ITEMID( CmdMenu_SetBackgroundStyle )

DECLARE_MENU_ITEMID( CmdMenu_PrintDrawing )
DECLARE_MENU_ITEMID( CmdMenu_PreviewDrawing )
DECLARE_MENU_ITEMID( CmdMenu_PrintDrawingPart )
DECLARE_MENU_ITEMID( CmdMenu_PreviewDrawingPart )

extern const a2dPrintWhat a2dPRINT_PrintDrawing;      //!< print a2dDrawing
extern const a2dPrintWhat a2dPRINT_PreviewDrawing;    //!< preview print a2dDrawing
extern const a2dPrintWhat a2dPRINT_PrintDrawingPart;      //!< print a2dDrawing Part
extern const a2dPrintWhat a2dPRINT_PreviewDrawingPart;    //!< preview print a2dDrawing Part


// ----------------------------------------------------------------------------
// wxDoubleNumberEntryDialog
// ----------------------------------------------------------------------------

//! wxDoubleNumberEntryDialog: a dialog with spin control, [ok] and [cancel] buttons
class A2DCANVASDLLEXP wxDoubleNumberEntryDialog : public wxDialog
{
public:
    wxDoubleNumberEntryDialog( wxWindow* parent,
                               const wxString& message,
                               const wxString& prompt,
                               const wxString& caption,
                               double value, double min, double max,
                               const wxPoint& pos = wxDefaultPosition );

    //! get the double
    double GetValue() const { return m_value; }

    //! implementation only
    void OnOK( wxCommandEvent& event );
    //! implementation only
    void OnCancel( wxCommandEvent& event );

protected:

    //! the text control top get the double entry
    wxTextCtrl* m_textctrl;
    //! teh value of the double
    double m_value;
    //! min required
    double m_min;
    //! max required
    double m_max;

private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS( wxDoubleNumberEntryDialog )
    DECLARE_NO_COPY_CLASS( wxDoubleNumberEntryDialog )
};

// ----------------------------------------------------------------------------
// function to get a number from user
// ----------------------------------------------------------------------------

//! get Double from user
double A2DCANVASDLLEXP
wxGetDoubleNumberFromUser( const wxString& message,
                           const wxString& prompt,
                           const wxString& caption,
                           double value = 0,
                           double min = 0,
                           double max = 100,
                           wxWindow* parent = ( wxWindow* )NULL,
                           const wxPoint& pos = wxDefaultPosition );

class A2DCANVASDLLEXP a2dDrawingEvent;

//! Contain one drawing as hierarchical tree of a2dCanvasObject's
/*!
A a2dDrawing contains a drawing and related data, such as units.
A drawing start at the m_rootObject, and its children is what is seen as the top level of the drawing.
The m_rootObject and its children are a2dCanvasObject's. But a a2dCanvasObject itslef can have children,
therefore the drawing can be hierarchy, where children are drawn releative to their parent canvasobject. 

The drawing can be displayed on a a2dDrawingPart. And any level in the drawing hierarchy can set as the
start object to a2dDrawingPart. More then one a2dDrawingPart can be used at the same time, each displaying 
different parts and/or different levels of the a2dDrawing.
Changes in the canvasobjects within the a2dDrawing are reported to the top level, and from there reported 
to the a2dDrawingPart's set on the drawing. The a2dDrawingPart is used on its turn by a a2dCanvas to display
part of the drawing in a window. The same way a2dDrawingPart can be used to print or to draw into bitmap's.

The drawing in a2dDrawing can be on layers, the m_layersetup is used to store the colours and othere features
of the layers in use. Each canvas object rendered, refers to the layer setup by an id. Layers are rendered 
from top to bottom, as defined by the layer order.
For each layer to be rendered on a a2dDrawingPart's, the drawing is traversed, and only objects with that layer
id are rendered.

A a2dDrawing does have its own a2dCommandProcessor, changes to the drawing are (can be) recorded using 
commands submitted to the command processor. The changes can be Undone by reverse applying the commands.

When used in combination with the wxArt2d document view framework, one a2dCanvasDocument can contain 
many a2dDrawing objects. One can even place drawings inside other drawings.
The classes a2dCameleonInst and a2dCameleon together with related classes, are used to do this.
  
*/
class a2dDrawing : public a2dObject
{
    DECLARE_EVENT_TABLE()

public:

    //! Constructor.
    a2dDrawing( const wxString& name = wxT(""), a2dHabitat* habitat = NULL );

    //! Copy constructor
    a2dDrawing( const a2dDrawing& other, CloneOptions options, a2dRefMap* refs );

    virtual void AssignContent( const a2dDrawing& other );

    ~a2dDrawing();

    a2dHabitat* GetHabitat() const { return m_habitat; }

    void SetHabitat( a2dHabitat* habitat ) { m_habitat = habitat; }

    inline a2dDrawing* TClone( CloneOptions options, a2dRefMap* refs = NULL ) { return ( a2dDrawing* ) Clone( options, refs ); }

    //! set parent object of the drawing ( in case there is always only one )
    void SetParent( a2dObject* parent ) { m_parent = parent; }

    //! get parent object of the drawing ( in case there is always only one )
    //! return pointer of the current parent object
    //!\return: pointer to the current object that is used to Get to for instance a2dCameleon.
    a2dObject* GetParent() const { return m_parent; }

    void SetAvailable();

    //!set a2dDrawing to its nested a2dCanvasObject's recursive for whole tree
    void SetRootRecursive();

    //!set the layersettings for the canvas.
    /*!
        A default Layer Setting is created in the constructor a2dCanvasDocument::a2dCanvasDocument

        An example how to set another layer setup
        \code
        // Clone the current layer setup
        a2dLayers* docLayers = doc->GetLayerSetup()->Clone( bool deep = true );

        // Create a new layer with an index above the predefined layers
        a2dLayerInfo* lr = new a2dLayerInfo(wxLAYER_USER_FIRST, "MyLayer");

        // Set layer available
        lr->Set_Available(true);

        // Add layer to to the a2dLayers
        docLayers->Append(lr);

        doc->SetLayerSetup(docLayers);
        \endcode

        \remark
        You don't have to call Set_UpdateAvailableLayers it will be done automatically

        \param layersetup the new layersetup
    */
    void SetLayerSetup( a2dLayers* layersetup );

    //!Get the layersettings for the canvas.
    /*!
        A default Layer Setting is taken from a2dCanvasGlobal.
        \sa a2dCanvasGlobal for central layers settings, used for initializing a a2dCanvasDocument
    */
    a2dLayers* GetLayerSetup() { return m_layersetup; }

	/*! Set if layers will be save to CVG  or not.
		If layers are shared with other drawing or not, savcing of layers in drawing can de disabled here.
	*/
	void SetSaveLayers( bool saveLayers ) { m_saveLayers = saveLayers; }

	static void SetSaveLayersInit( bool saveLayers ) { m_saveLayersInit = saveLayers; }

	void SetInitLayersGlobal( bool initLayersGlobal ) { m_initLayersGlobal = initLayersGlobal; }

	static void SetInitLayersGlobalInit( bool initLayersGlobal ) { m_initLayersGlobalInit = initLayersGlobal; }
	

    //! get the root object, which holds the objects in the document
    inline a2dCanvasObject* GetRootObject() const { return m_rootObject; }

    //! set new root object ( may not be NULL )
    void SetRootObject( a2dCanvasObject* newRoot );

    //! does root object have children.
    bool IsEmpty() const { return m_rootObject->GetChildObjectsCount() == 0; }

    //! return root object, child list, if not yet avaialble, it will be create on the fly.
    a2dCanvasObjectList* GetChildObjectList() const { return m_rootObject->CreateChildObjectList(); }

    void Append( a2dCanvasObject* obj );
    void Prepend( a2dCanvasObject* obj );

    //! set size for view in userunits, when still empty
    void SetInitialSize( double sizeX, double sizeY )
    { m_initialSizeX = sizeX; m_initialSizeY = sizeY; }

    //! get size for view in userunits, when still empty
    double GetInitialSizeX() { return m_initialSizeX; }

    //! get size for view in userunits, when still empty
    double GetInitialSizeY() { return m_initialSizeY; }

    //! create a command processor and set it for the document.
    /*!
        If available the existing commandprocessor will be deleted.
        A new CommandProcessor will be created and set for the document.

        Override if you need a more application specific commandhandler.
    */
    virtual void CreateCommandProcessor();

    //!Returns a pointer to the command processor associated with this document
    a2dCommandProcessor* GetCommandProcessor() const { return m_commandProcessor; }

    //! get a pointer to the command processor
    a2dCanvasCommandProcessor* GetCanvasCommandProcessor();

    //! Sets the command processor to be used for this document.
    /*!
        Use in case you need one a2dCommandProcessor to be used for several
        a2dDocument's,
        Make sure you set it to NULL before deleting the document itself,
        else the a2dDocument destructor will delete it ( leaving the others
        with dangling pointers ).
    */
    void SetCommandProcessor( a2dCommandProcessor* proc ) { m_commandProcessor = proc; }

    //!set all bit flags in object that or true in mask to true or false
    /*!
        set specific flags to true or false, also recursive for children objects

        \remark the object is not setpending when soemthing changed ( actually the pending flag can be set here also )

        \param setOrClear if true sets the flag to true else to false
        \param which set only those flags in object to true or false
        \param classname if not empty, object must have this classname
        \param whichobjects set flags only for object having this flags set already
    */
    void SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which, const wxString& classname = wxT( "" ), a2dCanvasObjectFlagsMask whichobjects = a2dCanvasOFlags::ALL );

    //! Tells the document that an object has changed.
    /*!
        If an object has changed and and needs an update, this method
        informes the document about this.
        The update of pending objects will be proceed in Idle time.
    */
    void SetUpdatesPending( bool onoff );

    //! call DependencyPending on root object and recursive on childs.
    /*!
        This call is repeated until no more a2dCanvasObject were set pending.
        \sa a2dCanvasObject::DependencyPending
    */
    void DependencyPending();

    //!returns true if some objects have changed recently within this document
    bool GetUpdatesPending();

    //! Switches ignorance of pending objects on and off.
    /*!
        If set to <code>true</code> pending objects won't be updated.
        This method is used by several tools (i.e. a2dDragTool) to avoid updates
        while the tool is working.

        \see GetUpdatesPending()

        \param onoff <code>true</code> to ignore pending objects, else <code>false</code>
    */
    void SetIgnorePendingObjects( bool onoff );

    //!get setting of ignore pending objects /sa GetUpdatesPending()
    bool GetIgnorePendingObjects() { return m_ignorePendingObjects; }

    //! also walks m_layersetup and m_rootobject.
    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    //bool Update( a2dCanvasObject::UpdateMode mode );

    void UpdateAllViews( unsigned int hint, wxObject* objecthint = NULL );

    //!adds current and future boundingbox of the objects with pending flag set, to the pendinglist of all a2dDrawingPart's
    /*!
        which are using this document.
        The areas will only be updated from the a2dDrawingPart's, which are using this document in idle time.
        It first check's for pending object's stored in this a2dCanvasDocument.
        If true, it searches those object starting from the displayed showgroup in each drawer, and adds
        the boundingbox of the current state the object is in, to the update area list of the those a2dDrawingPart's
        If the boundingbox of an object is not available, it will be calcualted at that moment.
        In Next step the new boundingboxes are calculated for the pending objects and those are also
        added to the pending arae list's of all a2dDrawingPart's.
        This functions adds areas of all places where this object is rendered also when referenced.
        In the end, the pending flag in all objects is reset.

        AddPendingUpdatesOldNew() makes sure all objects
        that where pending are processed.

        In idle time a2dDrawingPart's will only redraw the areas that are damaged because of changes to canvas objects.



        \return if there were pendingobjects it return true
    */
    bool AddPendingUpdatesOldNew();

    //! Has the document been modified
    bool IsModified() const { return m_documentModified != 0; }

    //! set the drawing as modified or not
    /*!
        If the modified status changes an ::wxEVT_CHANGEDMODIFY_DRAWING event will
        be send.

        \param increment to decrement or increment document modified counter.
    */
    void Modify( bool increment );

    //!this is the number that defines the number of decimal places in the fraction
    /*!when writing numbers to a file.
       When reading a file, this number is set to the accuracy information available in the file.
       1/GetUnitsAccuracy() == 100 means 2 decimal places will be written, or the data just
       read is accurate to this number of decimal places.

       \remark GetUnitsAccuracy() can also be > 1
    */
    double GetUnitsAccuracy() const { return m_units_accuracy; }

    //!this is the number that defines the number of decimal places in the fraction
    /*!when writing numbers to a file.
       When reading a file, this number is set to the accuracy information available in the file.
       1/GetAccuracyUserUnits() == 100 means 2 decimal places will be written, or the data just
       read is accurate to this number of decimal places.
       \remark SetUnitsAccuracy() can also be > 1
    */
    void   SetUnitsAccuracy( double accuracy ) { m_units_accuracy = accuracy; }

    //!this is the number that defines the physical dimension in meters / inch/ etc.
    /*! numbers used for storing length or position or always stored as doubles.
        The real dimension is: (anynumber) * GetUnitsScale() * GetUnits()
    */
    double GetUnitsScale() const { return m_units_scale; }


    //!this is the number that defines the physical dimension in meters / inch/ etc.
    /*! numbers used for storing length or position or always stored as doubles.
        The real dimension is: (anynumber) * GetUnitsScale() * GetUnits()
    */
    void   SetUnitsScale( double scale ) { m_units_scale = scale; }

    //! Normalize objects ( defined in coordinates -1 to 1 ranges ) will be multiplied by this factor.
    /*!
        Libraries containing object which are defined in normalized coordinates,
        need to be multiplied by a certain factor before they can be added to a document.
        This factor defines by how much that should be done.
        As an example a libary containing arrow objects for placing at the end of a2dEndsLine
        objects, will use this factor.
    */
    double GetNormalizeScale() const { return m_normalize_scale; }

    //! see GetNormalizeScale()
    void SetNormalizeScale( double scale ) { m_normalize_scale = scale; }

    //!this string defines the unit e.g. meters / inch/ etc.
    /*! numbers used for storing length or position or always stored as doubles.
        The real dimension is: (anynumber) * GetUnitsScale() * GetUnits()

        \remark the value may also be "trees" "pixels" or "people"  etc.

        \remark conversion to other units is not just simply changing this string.
        You must Change the SetUnitsScale(double) or really scale the data itself.
    */
    wxString GetUnits() const { return m_units; }

    //!this string defines the unit e.g. meters / inch/ etc.
    /*! numbers used for storing length or position or always stored as doubles.
        The real dimension is: (anynumber) * GetUnitsScale() * GetUnits()

        \remark the value may also be "trees" "pixels" or "people" etc.

        \remark conversion to other units is not just simply changing this string.
        You must Change the SetUnitsScale(double) or really scale the data itself.
    */
    void SetUnits( const wxString& unitString ) { m_units = unitString; }

    //! Get measuring unit (e.g. Meters Inch Microns etc.)
    /*!
        Can be used to measure distances in a different unit than m_units.
    */
    wxString GetUnitsMeasure() { return m_units_measure; }

    //! Set measuring unit (e.g. Meters Inch Microns etc.)
    /*!
        Can be used to measure distances in a different unit than m_units.
    */
    void SetUnitsMeasure( const wxString& unitMeasure ) { m_units_measure = unitMeasure; }

    //! Use habitat to get the AberArcToPoly, and scale it to drawing coordinates.
    double GetAberArcToPolyScaled() const { return double( GetHabitat()->GetAberArcToPoly() ) / GetUnitsScale(); }

    //! set special id to differentiate drawings
    a2dDrawingId GetDrawingId() { return m_drawingId; }

    //! get special id to differentiate drawings
    void SetDrawingId( a2dDrawingId id ) { m_drawingId = id; }
    
    //! if true, editing drawing is allowed
    void SetMayEdit( bool mayEdit ) { m_mayEdit = mayEdit; }

    //! if true, editing drawing is allowed
    bool GetMayEdit() const { return m_mayEdit; }

    //!Sets the title for this drawing.
    /*!
        \param title  The title of the drawing.
    */
    void SetTitle( const wxString& title );

    //! Returns the title for this document.
    wxString GetTitle() const { return m_title; }

    //! Sets a description of the drawing
    /*!
        A description may describe the content of this drawing or contain keywords etc.
        The default value is "".

        \param desc description of the drawing
    */
    void SetDescription( const wxString& desc ) { m_description = desc; }

    //! Returns the description of the document
    /*!
        \see SetDescription

        \return the description
    */
    wxString GetDescription() const { return m_description; }

    //! load document from a file with the given filename and given parser type
    bool LoadFromFile( const wxString& filename, a2dIOHandlerStrIn* handler = new a2dIOHandlerCVGIn() );

    //! save document to a file with the given name and given output writer type
    bool SaveToFile( const wxString& filename,  a2dIOHandlerStrOut* handler = new a2dIOHandlerCVGOut() ) const;
    
#if wxART2D_USE_CVGIO
    virtual void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser );
    virtual void Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite, a2dCanvasObject* start );
    virtual void Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    bool LinkReference( a2dObject* other );

    //! layer info changed id sent around when m_layersetup is changed.
    static const a2dSignal sig_layersetupChanged;

    //! when an object is removed from a layer,
    static const a2dSignal sig_changedLayer;

    a2dCanvasObject* PushInto( a2dCanvasObject* parent );

    //! gives time when a change was made to the drawing
    /*!
        Comparing this time in a different object its own update time, one can easily keep them up to date.

        e.g. set in SetUpdatesPending()
    */
    wxDateTime GetInternalChangedTime() const { return m_changedTime; }

    //! make the changed time Now
    void TriggerChangedTime() { m_changedTime = wxDateTime::Now(); }

    //! line up in rows all a2dCameleon in the root.
    void LineUpCameleons( int rowsize = 6 );

	bool ProcessEvent( wxEvent& event );

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    //! track modification of document
    void OnDoEvent( a2dCommandProcessorEvent& event );

    //! track modification of document
    void OnRedoEvent( a2dCommandProcessorEvent& event );

    //! track modification of document
    void OnUndoEvent( a2dCommandProcessorEvent& event );

    void OnChangeDrawings( a2dDrawingEvent& event );

    void OnAddCameleon( a2dCameleonEvent& event );

    void DoAddPending( a2dIterC& ic );

    //! Called by ProcessEvent(wxEvent& event) of document
    /*!
        Event processing is called by a2dDrawingPart.
        In a a2dDocumentCommandProcessor controller application wxDocChildFrame and wxDocParentFrame
        redirect events to wxView and a2dDocumentCommandProcessor.
        In a non a2dDocumentCommandProcessor the a2dCanvas is calling ProcessEvent(wxEvent& event) of document.
        Onidle updates all drawers if pending updates are available
    */
    void OnIdle( wxIdleEvent& event );

    void OnComEvent( a2dComEvent& event );

#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    a2dDrawingId m_drawingId;

    //!setup for layers in this document
    /*!
        \remark will contain if a layer is available/visible for rendering
        \remark object without style will take the layer style to draw itself.
    */
    a2dSmrtPtr<a2dLayers>  m_layersetup;

    //!A pointer to the command processor associated with this document.
    a2dSmrtPtr<a2dCommandProcessor> m_commandProcessor;

	a2dCanvasObjectPtr m_rootObject;

	a2dCanvasObjectPtr m_saveStartObject;

    //! in case of a single parent, this can be set.
    a2dObject* m_parent;

    //!set by object in this a2dCanvasDocument, to tell they have changed and therefore it needs to be rerendered
    bool m_updatespending;

    //!if set ignore all setting for pendingobjects
    bool m_ignorePendingObjects;

    long m_documentModified;

    //! for new document views what will be the size in userunits in X
    double m_initialSizeX;

    //! for new document views what will be the size in userunits in Y
    double m_initialSizeY;

    //! how many decimal places in fraction are valid
    double m_units_accuracy;

    //! scaling factor (how many "m_units_measure" is one unit)
    double m_units_scale;

    //! normalization factor to scale normalized objects.
    double m_normalize_scale;

    //! unit (e.g. Meters Inch Microns etc.)
    wxString m_units;

    //! measuring unit (e.g. Meters Inch Microns etc.)
    wxString m_units_measure;

    //! gives time when a change was made to the drawing
    /*!
    */
    wxDateTime m_changedTime;

    wxDateTime m_lastCheck;

    //! if true, editing drawing is allowed
    bool m_mayEdit;

	//! save layers or not
	bool m_saveLayers;

	static bool m_saveLayersInit;

	bool m_initLayersGlobal;

	static bool m_initLayersGlobalInit;

    wxString m_title;

    wxString m_description;

    a2dHabitat* m_habitat;

private:

    virtual a2dObject* SmrtPtrOwn();
    //!To release the object, it is not longer owned by the calling object.
    /*! the object itself will be deleted when not Owned (the reference count becomes zero)

      \remark AskSaveWhenOwned was false, and this document is not Owned by anyone,
              the user will now be asked to save the document (when it was modified).
    */
    virtual bool SmrtPtrRelease();
    friend class a2dSmrtPtrBase;

    DECLARE_DYNAMIC_CLASS( a2dDrawing )
};

typedef a2dSmrtPtr<a2dDrawing> a2dDrawingPtr;

BEGIN_DECLARE_EVENT_TYPES()
	//!see a2dDrawingEvent \ingroup eventid
	DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, wxEVT_UPDATE_DRAWING, 1 )
	DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, wxEVT_CHANGEDMODIFY_DRAWING, 1 )
    //DECLARE_EXPORTED_EVTIDFUNC( A2DCANVASDLLEXP, wxEVT_CHANGEDMODIFY_DRAWING )
END_DECLARE_EVENT_TYPES()

//! used to report a2dDrawing events
/*!
    The event Object is a a2dDrawing.

    \ingroup events

*/
class A2DCANVASDLLEXP a2dDrawingEvent : public a2dEvent
{
public:
 
    //! for event types
    /*!
        ::wxEVT_REPORT_VIEWS
        Sent from a2dDocument to a2dView's. a2dView will add itself to the reportlist of the event, but only
        when this document is indeed used by the a2dView.
        Register your wxEvtHandler to recieve it.
    */
    a2dDrawingEvent( unsigned int updateHint, a2dDrawing* drawing )
        : a2dEvent( 0, wxEVT_UPDATE_DRAWING )
    {
        m_updateHint = updateHint;
        SetEventObject( drawing );
    }

    a2dDrawingEvent( int theId, wxEventType commandType, a2dDrawing* drawing )
        : a2dEvent( theId, commandType )
    {
        m_updateHint = 0;
        SetEventObject( drawing );
    }

    //! constructor
    a2dDrawingEvent( const a2dDrawingEvent& event )
        : a2dEvent( event )
    {
        m_updateHint = event.m_updateHint;
    }
 
    virtual wxEvent* Clone() const { return new a2dDrawingEvent( *this ); }

    unsigned int GetUpdateHint() { return m_updateHint; }

    a2dDrawing* GetDrawing() { return (a2dDrawing*) GetEventObject(); }

    int GetModified() { return m_modified; }

    void SetModified( int modifications ) { m_modified = modifications; }

 private:

    int m_modified;

    unsigned int m_updateHint;
};


typedef void ( wxEvtHandler::*a2dDrawingEventFunction )( a2dDrawingEvent& );
#define a2dDrawingEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(a2dDrawingEventFunction, &func)

//! event from a drawing when updated
#define EVT_UPDATE_DRAWING(func)  wx__DECLARE_EVT0( wxEVT_UPDATE_DRAWING,  a2dDrawingEventHandler( func ))

//! event from a drawing when changed by commands
#define EVT_CHANGEDMODIFY_DRAWING(func)  wx__DECLARE_EVT0( wxEVT_CHANGEDMODIFY_DRAWING,  a2dDrawingEventHandler( func ))
//#define EVT_CHANGEDMODIFY_DRAWING( func ) wx__DECLARE_EVT0( GETID_wxEVT_CHANGEDMODIFY_DRAWING(),  a2dDrawingEventHandler( func ))

class A2DCANVASDLLEXP ToolDlg;

class A2DCANVASDLLEXP a2dStyleDialog;

class A2DCANVASDLLEXP LayerPropertiesDialog;

class A2DCANVASDLLEXP a2dLayerOrderDlg;

class A2DCANVASDLLEXP a2dLayerDlg;

class A2DCANVASDLLEXP IdentifyDialog;

class A2DCANVASDLLEXP ExecDlg;

class A2DCANVASDLLEXP a2dCentralCanvasCommandProcessor;

class A2DCANVASDLLEXP a2dPathSettings;

class A2DCANVASDLLEXP a2dSnapSettings;

class A2DCANVASDLLEXP a2dSettings;

class A2DCANVASDLLEXP a2dTransDlg;

class A2DCANVASDLLEXP a2dPropertyEditorDlg;

class A2DCANVASDLLEXP a2dCanvasObjectsDialog;

class A2DCANVASDLLEXP a2dCoordinateEntry;

class A2DCANVASDLLEXP a2dMeasureDlg;


#define DECLARE_CMH_HABITAT() \
 \
    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); } \
    a2dHabitat* GetDrawHabitat() \
    { \
        assert( GetCanvasCmp() ); \
        assert( GetCanvasCmp()->m_drawing ); \
        return GetCanvasCmp()->m_drawing->GetHabitat(); \
    } \



//! a command processor specially designed to work with a a2dCanvasDocument
/*!
    This command processor allows you to add object to the document and
    takes care of undo redo.

    \ingroup tools commands
*/
class A2DCANVASDLLEXP a2dCanvasCommandProcessor : public a2dCommandProcessor
{
    DECLARE_EVENT_TABLE()
    DECLARE_CLASS( a2dCanvasCommandProcessor )

public:

    static const a2dCommandId COMID_InsertGroupRef;
    static const a2dCommandId COMID_ShowDlgLayers;
    static const a2dCommandId COMID_ShowDlgLayersDocument;
    static const a2dCommandId COMID_ShowDlgLayersGlobal;
    static const a2dCommandId COMID_ShowDlgLayerOrderDocument;
    static const a2dCommandId COMID_ShowDlgLayerOrderGlobal;
    static const a2dCommandId COMID_ShowDlgTools;
    static const a2dCommandId COMID_ShowDlgStyle;
    static const a2dCommandId COMID_ShowDlgSnap;
    static const a2dCommandId COMID_ShowDlgGroups;
    static const a2dCommandId COMID_ShowDlgIdentify;
    static const a2dCommandId COMID_ShowDlgCoordEntry;
    static const a2dCommandId COMID_ShowDlgMeasure;
    static const a2dCommandId COMID_ShowDlgStructure;
    static const a2dCommandId COMID_ShowDlgPathSettings;
    static const a2dCommandId COMID_ShowDlgSettings;
    static const a2dCommandId COMID_ShowDlgTrans;
    static const a2dCommandId COMID_ShowDlgPropEdit;
    static const a2dCommandId COMID_PopTool; 

    a2dCanvasCommandProcessor( a2dDrawing* drawing, int maxCommands = -1 );
    ~a2dCanvasCommandProcessor();

    static bool StaticProcessEvent( wxUpdateUIEvent& event );

    void OnUpdateMenu( wxUpdateUIEvent& event );

    static void DefaultOnUpdateMenu( wxUpdateUIEvent& event );

    void OnPushToolUpdateMenu( wxUpdateUIEvent& event );

    //! do this at command start
    void OnBusyBegin( a2dCommandProcessorEvent& WXUNUSED( event ) );

    //! do this at command end
    void OnBusyEnd( a2dCommandProcessorEvent& WXUNUSED( event ) );

    //! All menu's a2dMenuIdItem used can be intercepted here
    void OnMenu( wxCommandEvent& event );

    //! All menu's a2dMenuIdItem used can be intercepted here
	void OnPushToolMenu( wxCommandEvent& event );

    bool Print( wxUint16 printWhat );
    bool Preview( wxUint16 printWhat );

    //! ask for a file using a file selector.
    /*!
        If your path contains internal variables, they will be expanded.
    */
    wxString AskFile( const wxString& message, const wxString& default_path = "",
                      const wxString& default_filename = "", const wxString& default_extension = "",
                      const wxString& wildcard = "*.*", int flags = 0,
                      int x = -1, int y = -1 );

    //! set undo storage or not
    void SetUndo( bool withUndo ) { m_withUndo = withUndo; }

    //! get undo storage setting
    bool GetUndo() { return m_withUndo; }

    //* next to the base class submit, it sets the document and commandprocessor for
    /*!
        a2dDocumentCommand
    */
    virtual bool Submit( a2dCommand* command, bool storeIt = true );

    //! Undo one command or command group
    virtual bool Undo();
    //! Redo one command or command group
    virtual bool Redo();

    //! submit a command template with multiple objects
    virtual bool SubmitMultiple( a2dCommand* command, a2dCanvasObjectList* objects, bool storeIt = true );

#if wxUSE_PRINTING_ARCHITECTURE
    wxPageSetupDialogData* GetPageSetupData( void ) const { return m_pageSetupData; }
    void SetPageSetupData( const wxPageSetupDialogData& pageSetupData ) { *m_pageSetupData = pageSetupData; }
    void SetPageSetupData( wxPageSetupDialogData* pageSetupData );
#endif

    //! full update of drawing
    void Refresh();

	void DeselectAll();

    a2dDrawingPart* GetActiveDrawingPart() { return a2dCanvasGlobals->GetActiveDrawingPart(); }

    bool IsShownToolDlg();

    bool IsShowna2dStyleDialog();

    bool IsShownLayerPropertiesDialog();

    bool IsShownLayerOrderDialog();

    bool IsShownIdentifyDialog();

    bool IsShowna2dSnapSettings();

    bool IsShowna2dCoordinateEntry();

    bool IsShowna2dMeasureDlg();

    bool IsShowna2dPathSettings();

    bool IsShowna2dSettings();

    bool IsShowna2dTransDlg();

    //bool IsShownPropEditDlg();

    bool IsShownStructureDlg();

    //! show style dialog or not, return true of changed from previous state
    bool ShowDlgStyle( bool onOff );

    bool SaveLayers( const wxFileName& fileName );

    bool LoadLayers( const wxFileName& fileName );

    virtual bool PushToolDragNew( a2dCanvasObject* newObject, const a2dMenuIdItem& initiatingMenuId, bool dragCenter = true, bool oneshot = false, bool lateconnect = true );

    virtual bool PushTool( const a2dCommandId& whichTool, bool shiftadd = false, bool oneshot = false );

    a2dBaseTool* PopTool();

    //!set top object available in the a2dCanvasDocument to be used for adding other objects
    /*!
    \param obj: pointer to object to set as new parent a2dCanvasObject

    \remark the document will be searched to check if the object does exist,
    if not found, the rootobject of the document will be set as parent.

    \remark if obj is set to NULL, the rootobject of the document will be set.

    \return if the object is found and set return is true
    */
    bool SetParentObject( a2dCanvasObject* obj = 0 );

    //!return pointer of then current parent object
    //!\return: pointer to the current object that is used for adding other objects to.
    a2dCanvasObject* GetParentObject() { return m_parent; }

    //! get the current canvas object to add to parent as child
    a2dCanvasObject* GetCurrentCanvasObject() { return m_currentobject; }

    //! set the current canvas object, which gets the change in style etc
    /*!
        The current object is the object that gets the settings made on the commandprocessor
        and that do influence a a2dCanvasObject.
        Normally this is already set when a new object is added via commands.
        You must use NULL to reset the current object to not in use.

        When setting the current object, normally the object its properties are checked and the commandprocessor
        its internal chached properties or updated to that.

        \param currentcanvasobject the object to set current.
        \param takeOverProperties when true the internal chached properties are updated from the object.
    */
    void SetCurrentCanvasObject( a2dCanvasObject* currentcanvasobject, bool takeOverProperties = true );

    //! add the current canvas object to parent as child
    /*! First the current style is to the currentobject, and then it is added to the document.

          \param objectToAdd this is the object to add to the current parentobject
          \param withUndo if true a a2dCommand_AddObject will be used to have undo information
          \param setStyles the object added will get these styles that are currently set in the command processor
          \param parentObject when not NULL, this will become the current parent object.
    */
    a2dCanvasObject* AddCurrent( a2dCanvasObject* objectToAdd, bool withUndo = false, a2dPropertyIdList* setStyles = NULL, a2dCanvasObject* parentObject = NULL );

    //! add specific object
    a2dCanvasObject* Add_a2dCanvasObject( double x = 0, double y = 0 );

    //! add specific object
    a2dCanvasObjectReference* Add_a2dCanvasObjectReference( double x, double y, a2dCanvasObject* obj );

    //a2dNameReference* Add_a2dNameReference( double x, double y, a2dCanvasObject* obj, const wxString &text, double size, double angle );

    //! add specific object
    a2dOrigin* Add_a2dOrigin( double w, double h );

    //! add specific object
    a2dHandle* Add_a2dHandle( double xc, double yc, int w, int h, double angle = 0 , int radius = 0 );

    //! add specific object
    a2dRectC* Add_a2dRectC( double xc, double yc, double w, double h, double angle = 0 , double radius = 0 );

    //! add specific object
    a2dArrow* Add_a2dArrow( double xt, double yt, double l1, double l2, double b, bool spline = false );

    //! add specific object
    a2dRect* Add_a2dRect( double x, double y, double w, double h , double radius = 0 );

    //! add specific object
    a2dCircle* Add_a2dCircle( double x, double y, double radius );

    //! add specific object
    a2dEllipse* Add_a2dEllipse( double xc, double yc, double width, double height );

    //! add specific object
    a2dEllipticArc* Add_a2dEllipticArc( double xc, double yc, double width, double height, double start, double end );

    //! add specific object
    a2dArc* Add_a2dArc( double xc, double yc, double radius, double start, double end );

    //! add specific object
    a2dSLine* Add_a2dSLine( double x1, double y1, double x2, double y2 );

    //! add specific object
    a2dEndsLine* Add_a2dEndsLine( double x1, double y1, double x2, double y2 );

    //! add specific object
    a2dImage* Add_a2dImage( const wxImage& image, double xc, double yc, double w, double h );

    //! add specific object
    a2dImage* Add_a2dImage( const wxString& imagefile, wxBitmapType type, double xc, double yc, double w, double h );

    //! add specific object
    a2dText* Add_a2dText( const wxString& text, double x, double y, double angle, const a2dFont& font );

    //! add specific object
    a2dPolygonL* Add_a2dPolygonL( a2dVertexList* points, bool spline = false );

    //! add specific object
    a2dPolylineL* Add_a2dPolylineL( a2dVertexList* points, bool spline = false );

    //! add a point to the current a2dCanvasObject
    bool Add_Point( double x, double y );

    //! add a point to the current a2dCanvasObject
    bool Move_Point( int index , double x, double y );

	void SetZoomFactor( double zoomfactor ) { m_zoomfactor = zoomfactor; }
	double GetZoomFactor() { return m_zoomfactor; }

	//! Zoom to this rectangle, or Upp at x1, y1.
	bool Zoom( double x1, double y1, double x2, double y2, bool upp );

	//! Zoom to a real size or a factor of it.
	/*! Center of window is the fix point unless the slected flag is set, and there are selected objects, in which case it
		is the centre of the boundingbox of the selected objects.

		\param scaleFromRealSize Factor from the real size the zoom will be.
		\param selected if set also look at selcted objects.
	*/
	bool ZoomRealSize( double scaleFromRealSize, bool selected );

    bool ZoomOut();

    //! select this area
    bool Select( double x1, double y1, double x2, double y2 );

    //! de select this area
    bool DeSelect( double x1, double y1, double x2, double y2 );

    void DeleteDlgs();

    void OnEditProperties( a2dPropertyEditEvent& event );

    //! convert canvasobject to a list of polygons.
    //! But only for simple primitive objects.
    //! first a2dCanvasObject::GetAsCanvasVpaths() is used to convert to vector paths,
    //! and next the vector path is translated into polygons.
    a2dCanvasObjectList* ConvertToPolygons( a2dCanvasObject* canvasobject, bool transform );

    //! convert canvasobject to a list of Vpath.
    //! But only for simple primitive objects.
    //! first a2dCanvasObject::GetAsCanvasVpaths() is used to convert to vector paths,
    a2dCanvasObjectList* ConvertToVpath( a2dCanvasObject* canvasobject, bool transform );

    void ClearMeasurements();
    void AddMeasurement( double x, double y );
    void SetRelativeStart( double x, double y );

    static const a2dSignal sig_ClearMeasurements;
    static const a2dSignal sig_AddMeasurement;
    static const a2dSignal sig_SetRelativeStart;

    //! sets current style object to current canvas object
    /*!
        For the property id's in the list, copy the current settings in the command processor
        to the current object.

        \param setStyles lit of property id which will be set if available in the command processor.
    */
    //void SetStyle(  a2dPropertyIdList* setStyles );

     //! current parent object to which new objects will be added as childs
    //! this is also the ShowObject of the Active a2dDrawingPart when apropriate.
    a2dCanvasObject* m_parent;

    //! last added object
    a2dCanvasObject* m_currentobject;

    //! add in front of parent childs or add end
    bool m_AppendOrPrepend;

	a2dDrawing* m_drawing;

    //! if set, for commands which can undo, will be submitted like that.
    bool m_withUndo;

    double m_meas_x, m_meas_y;

	double m_zoomfactor;

    bool InsertGroupRef();

    static ToolDlg* m_tooldlg;

    static a2dStyleDialog* m_styledlg;

    static LayerPropertiesDialog* m_layersdlg;

    static a2dLayerOrderDlg* m_layerOrderDlg;

    static IdentifyDialog* m_identifyDlg;

    static a2dSnapSettings* m_snapSettings;

    static a2dCoordinateEntry* m_coordEntry;

    static a2dMeasureDlg* m_measure;

    static a2dPathSettings* m_pathsettings;

    static a2dSettings* m_settings;

    static a2dTransDlg* m_transset;

    static a2dPropertyEditorDlg* m_propEdit;

    static a2dCanvasObjectsDialog* m_structDlg;

    static a2dLayerDlg* m_layerChooseDlg;

#if wxUSE_PRINTING_ARCHITECTURE
    //! this is the global printer page setup data for printer
    wxPageSetupDialogData*    m_pageSetupData;
#endif

};

//! used to add object to a a2dCanvasDocument in the current parent
/*!

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_AddObject: public a2dCommand
{
 
public:
    //! used to add object to a a2dCanvasDocument in the current parent
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_AddObject() { m_canvasobject = NULL; m_parent = NULL; }

    a2dCommand_AddObject( a2dCanvasObject* parent, a2dCanvasObject* object );

    ~a2dCommand_AddObject( void );

    bool Do();
    bool Undo();
    bool Redo();

    DECLARE_CMH_HABITAT()

    a2dCanvasObject* GetCanvasObject() { return m_canvasobject; }
    a2dCanvasObject* GetParent() const { return m_parent; }

protected:
    a2dCanvasObjectPtr m_canvasobject;
    a2dCanvasObjectPtr m_parent;
};

//! used to release object from a a2dCanvasDocument in the current parent
/*!

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ReleaseObject: public a2dCommand
{

public:

    //! used to release object from a a2dCanvasDocument in the current parent
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_ReleaseObject() { m_canvasobject = NULL; m_parent = NULL; m_index = 0; m_now = true; }

    a2dCommand_ReleaseObject( a2dCanvasObject* parent,  a2dCanvasObject* object, bool now = true );

    ~a2dCommand_ReleaseObject( void );

    bool Do();
    bool Undo();
    bool PreDo();


    a2dCanvasObject* GetCanvasObject() { return m_canvasobject; }
    a2dCanvasObject* GetParent() { return m_parent; }

    DECLARE_CMH_HABITAT()

protected:

    a2dCanvasObjectPtr m_canvasobject;
    a2dCanvasObjectPtr m_parent;
    int m_index;
    bool m_now;
};

//! move object within a a2dDrawing
/*!

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_MoveObject: public a2dCommand
{

public:

    //! used to release object from a a2dCanvasDocument in the current parent
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_MoveObject( a2dCanvasObject* parentFrom = NULL, a2dCanvasObject* parentTo = NULL,  a2dCanvasObject* object = NULL );

    ~a2dCommand_MoveObject( void );

    bool Do();
    bool Undo();

    a2dCanvasObject* GetCanvasObject() { return m_canvasobject; }
    a2dCanvasObject* GetParentFrom() { return m_parentFrom; }
    a2dCanvasObject* GetParentTo() { return m_parentTo; }

    DECLARE_CMH_HABITAT()

protected:

    a2dCanvasObjectPtr m_canvasobject;
    a2dCanvasObjectPtr m_parentFrom;
    a2dCanvasObjectPtr m_parentTo;
    int m_index;
};

//! used to replace an object in a a2dCanvasDocument in the current parent
/*!

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ReplaceObject: public a2dCommand
{

public:
    //! used to replace an object in a a2dCanvasDocument in the current parent
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_ReplaceObject() { m_canvasobject = NULL;  m_canvasobject2 = NULL; m_parent = NULL; }

    a2dCommand_ReplaceObject( a2dCanvasObject* parent, a2dCanvasObject* oldobject, a2dCanvasObject* newobject );

    ~a2dCommand_ReplaceObject( void );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

protected:

    a2dCanvasObjectPtr m_canvasobject;
    a2dCanvasObjectPtr m_canvasobject2;
    a2dCanvasObjectPtr m_parent;
};

//! for changing boolean values inside a rectangle of the current parent object.
/*!
*/
class A2DCANVASDLLEXP a2dCommand_SetFlags: public a2dCommand
{
public:

    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_SetFlags() { m_parentObject = NULL; }

    //! given flags of objects within the rectangle are set to value
    a2dCommand_SetFlags( a2dCanvasObject* parentObject, a2dCanvasObjectFlagsMask which, bool value, bool preserve = false, a2dCanvasObjectFlagsMask whichobjects = a2dCanvasOFlags::ALL, const a2dBoundingBox& bbox = wxNonValidBbox );

    ~a2dCommand_SetFlags( void );

    bool Do();
    bool Undo();
	bool Redo();

    DECLARE_CMH_HABITAT()

protected:

    a2dBoundingBox m_bbox;

    a2dCanvasObjectFlagsMask m_which;
	a2dCanvasObjectFlagsMask m_whichObjects;

	//! objects found with required mask
    a2dCanvasObjectList* m_maskedObjects;
    a2dlist< a2dCanvasObjectFlagsMask > m_objectOldMask;

	//! parent of found objects
    a2dCanvasObjectPtr m_parentObject;
	bool m_preserve;
    bool m_bool;
};

//! for changing boolean values inside canvas objects
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetFlag: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_SetFlag() { m_canvasobject = NULL; }

    //! given flag of object set to value
    a2dCommand_SetFlag( a2dCanvasObject* object, a2dCanvasObjectFlagsMask which, bool value );

    ~a2dCommand_SetFlag( void );

    virtual a2dCommand* CloneAndBind( a2dObject* object );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

protected:

    a2dCanvasObjectPtr m_canvasobject;

    a2dCanvasObjectFlagsMask m_which;

    bool m_bool;
};

//! for changing boolean flag values inside pins
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetPinFlags: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_SetPinFlags() { m_pin = NULL; }

    //! given flags of object set to value
    a2dCommand_SetPinFlags( a2dPin* pin, a2dPin::a2dPinFlagsMask which, bool value );

    ~a2dCommand_SetPinFlags( void );

    virtual a2dCommand* CloneAndBind( a2dObject* object );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

protected:

    a2dPinPtr m_pin;

    a2dPin::a2dPinFlagsMask m_which;

    a2dPin::a2dPinFlagsMask m_oldmask;

    bool m_value;
};

// used as base commands for commands on a group of selected objects.
class A2DCANVASDLLEXP a2dCommandMasked: public a2dCommandGroup
{
public:

    a2dCommandMasked( 				
				a2dCanvasObject* parentObject,
				a2dCommandGroup* parent,
				a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED,
				a2dCanvasObjectFlagsMask targetMask = a2dCanvasOFlags::SELECTED2,
				bool index = false, bool rewire = false,
                const a2dCommandId& commandId = sm_noCommandId,
                const a2dCommandId& commandTypeId = sm_noCommandTypeId,
                const wxString& menuString = wxEmptyString
              );

    //! destructor
    ~a2dCommandMasked( void );

    inline a2dCommand* TClone( CloneOptions options = clone_deep, a2dRefMap* refs = NULL  ) { return ( a2dCommand* ) Clone( options, refs ); }

	bool Do();
	bool PostDo();

	bool Undo();
	bool Redo();

    DECLARE_CMH_HABITAT()

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

	//! the mask required for the objects to search
	a2dCanvasObjectFlagsMask m_which;
	//! target mask for m_target
	a2dCanvasObjectFlagsMask m_targetMask;

	//! produce index of where masked object are found.
	bool m_index;

	//! objects connected with pins rewiring is optional
	bool m_rewire;

	//! objects found with required mask
	a2dCanvasObjectList m_maskedObjects;
	//! indexes of found objects
	a2dlist< long > m_objectsIndex;
	//! target object
	a2dCanvasObjectPtr  m_target;
	//! parent of found objects
	a2dCanvasObject* m_parentObject;

	a2dCanvasObjectList m_copies;
};

//! for changing boolean select flag values inside canvas objects
/*!
    This command takes into account the layer settings of the a2dCanvasDocument,
    if there the layer is set non selectable, nothing will be done.

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetSelectFlag: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_SetSelectFlag() { m_canvasobject = NULL; }

    //! given select flag of object set to value
    a2dCommand_SetSelectFlag( a2dCanvasObject* object, bool value, bool preserve = false );

    ~a2dCommand_SetSelectFlag( void );

    virtual a2dCommand* CloneAndBind( a2dObject* object );

    bool Do();
    bool Undo();
	bool Redo();

    DECLARE_CMH_HABITAT()

protected:

    a2dCanvasObjectPtr m_canvasobject;

    bool m_bool; //value to set select flag to
	bool m_preserve; //do undo redo sets the flag (used as subcommand in groups)
	bool m_oldvalue; //what it was
};

//! for changing boolean select2 flag values inside canvas objects
/*!
    This command takes into account the layer settings of the a2dCanvasDocument,
    if there the layer is set non selectable, nothing will be done.

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetSelect2Flag: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_SetSelect2Flag() { m_canvasobject = NULL; }

    //! given select flag of object set to value
    a2dCommand_SetSelect2Flag( a2dCanvasObject* object, bool value, bool preserve = false );

    ~a2dCommand_SetSelect2Flag( void );

    virtual a2dCommand* CloneAndBind( a2dObject* object );

    bool Do();
    bool Undo();
	bool Redo();

    DECLARE_CMH_HABITAT()

protected:

    a2dCanvasObjectPtr m_canvasobject;

    bool m_bool;
	bool m_preserve; //do undo redo sets the flag (used as subcommand in groups)
	bool m_oldvalue; //what it was
};


#include <wx/listimpl.cpp>

//! used as part of a a2dCommand_TransformMask to store the matrix
//! of a a2dCanvasObject, in order to Undo/Redo the command
class A2DCANVASDLLEXP a2dCommandMatrixData
{

public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommandMatrixData( a2dCanvasObject* WXUNUSED( parent ), a2dCanvasObject* object, const a2dAffineMatrix& transform )
    {
        m_object = ( a2dCanvasObject* ) object;
        m_transform = transform;
    }

    ~a2dCommandMatrixData()
    {
    }

    a2dCanvasObjectPtr m_parent;

    a2dCanvasObjectPtr m_object;
    a2dAffineMatrix m_transform;
};

typedef a2dlist< a2dCommandMatrixData > a2dObjectMatrixList;

//! for changing only the matrix of objects for which a certain mask was set
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_TransformMask: public a2dCommandMasked
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_TransformMask( a2dCanvasObject* parent, const a2dAffineMatrix& newtransform, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED, bool rewire = true );

    ~a2dCommand_TransformMask( void );

    bool Do();
	bool PostDo( void );
    bool Undo();
	bool Redo( void );

    DECLARE_CMH_HABITAT()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const
    {
        return new a2dCommand_TransformMask( m_parentObject, m_transform, m_which );
    }

    a2dObjectMatrixList m_objectsMatrix;

    a2dAffineMatrix m_transform;
};

//! for changing only the matrix of objects for which a certain mask was set
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_RotateMask: public a2dCommandMasked
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_RotateMask( a2dCanvasObject* parent, double angle, bool bboxCenter = false, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED );

    ~a2dCommand_RotateMask( void );

    bool Do();
	bool PostDo();
    bool Undo();
	bool Redo( void );

    DECLARE_CMH_HABITAT()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const
    {
        return new a2dCommand_RotateMask( m_parentObject, m_angle, m_bboxCenter, m_which );
    }

    a2dObjectMatrixList m_objectsMatrix;

    double m_angle;
    bool m_bboxCenter;
};


//! for changing only the alignment of objects for which a certain mask was set
/*!
    All alignment features are on objects which are not connect objects (like wires).
    Wires are reconnected based on the connectionnengine set for the drawing its habitat.

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_AlignMask: public a2dCommandMasked
{
public:

    enum Alignment
    {
        MIDY				= 0x00000000,
        MIDX				= 0x00000001,
        MINX				= 0x00000002,
        MAXX				= 0x00000004,
        MINY				= 0x00000008,
        MAXY				= 0x00000010,
        MirrorVert			= 0x00000020,
        MirrorHorz			= 0x00000040,
        MirrorVertBbox		= 0x00000080,
        MirrorHorzBbox		= 0x00000100,
		DistributeVert		= 0x00000200,
		DistributeHorz		= 0x00000400,
        MIDY_Dest			= 0x00010000,
        MIDX_Dest			= 0x00020000,
        MINX_Dest			= 0x00040000,
        MAXX_Dest			= 0x00080000,
        MINY_Dest			= 0x00100000,
        MAXY_Dest			= 0x00200000,
        MirrorVert_Dest     = 0x00400000,
        MirrorHorz_Dest     = 0x00800000,
        MirrorVertBbox_Dest = 0x01000000,
        MirrorHorzBbox_Dest = 0x02000000,
		DistributeVert_Dest = 0x04000000,
		DistributeHorz_Dest = 0x08000000
	};


    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_AlignMask( a2dCanvasObject* parent, Alignment align, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED, bool doSnap = false );

    ~a2dCommand_AlignMask( void );

    bool Do();
	bool PostDo( void );
    bool Undo();
	bool Redo();

    DECLARE_CMH_HABITAT()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const
    {
        return new a2dCommand_AlignMask( m_parentObject, m_align, m_which );
    }

    a2dObjectMatrixList m_objectsMatrix;

    Alignment m_align;
    bool m_doSnap;
};


//! used to add points to polygon objects
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_AddPoint: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_AddPoint() 
    { 
        m_sendBusyEvent = false;
        m_canvasobject = NULL; 
    }

    a2dCommand_AddPoint( a2dCanvasObject* object, double x, double y, int index, bool afterinversion = true );
    a2dCommand_AddPoint( a2dCanvasObject* object, double x, double y, a2dLineSegmentPtr seg, bool afterinversion = true );
    ~a2dCommand_AddPoint( void );

    bool Do();
    bool Undo();
    bool Redo();

    DECLARE_CMH_HABITAT()

    // x of point
    double m_x;

    // y of point
    double m_y;

    a2dLineSegmentPtr m_seg;
    a2dLineSegmentPtr m_segAdded;

    bool m_afterinversion;

    a2dCanvasObjectPtr m_canvasobject;

};

//! used to move points in polygon objects
/*!
*/
class A2DCANVASDLLEXP a2dCommand_MoveSegment: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_MoveSegment() { m_canvasobject = NULL; }

    a2dCommand_MoveSegment( a2dCanvasObject* object, double x, double y, int index, bool afterinversion = true );
    a2dCommand_MoveSegment( a2dCanvasObject* object, double x, double y, a2dLineSegmentPtr seg, bool afterinversion = true );
    ~a2dCommand_MoveSegment( void );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

    // x of point
    double m_x;

    // y of point
    double m_y;

    a2dLineSegmentPtr m_seg;

    bool m_afterinversion;

    a2dCanvasObjectPtr m_canvasobject;

};

//! used to move points in polygon objects
/*!
*/
class A2DCANVASDLLEXP a2dCommand_MoveMidSegment: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_MoveMidSegment() { m_canvasobject = NULL; }

    a2dCommand_MoveMidSegment( a2dCanvasObject* object, double x, double y, int index, bool afterinversion = true );
    a2dCommand_MoveMidSegment( a2dCanvasObject* object, double x, double y, a2dLineSegmentPtr seg, bool afterinversion = true );
    ~a2dCommand_MoveMidSegment( void );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

    // x of point
    double m_x;

    // y of point
    double m_y;

    bool m_afterinversion;

    a2dLineSegmentPtr m_seg;

    a2dCanvasObjectPtr m_canvasobject;

};

//! used to remove points from polygon objects
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_RemoveSegment: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_RemoveSegment() { m_canvasobject = NULL; }

    a2dCommand_RemoveSegment( a2dCanvasObject* object, int index );
    a2dCommand_RemoveSegment( a2dCanvasObject* object, a2dLineSegmentPtr seg );
    ~a2dCommand_RemoveSegment( void );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

    //! where did we remove it, in order to insert there in case of undo
    unsigned int m_index;

    //! segment removed
    a2dLineSegmentPtr m_seg;

    a2dCanvasObjectPtr m_canvasobject;
};

//! used to set the complete Segment list/array of polygons
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetSegments: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_SetSegments() { m_canvasobject = NULL; }

    a2dCommand_SetSegments( a2dCanvasObject* object, a2dVertexList* segments, bool afterinversion );
    ~a2dCommand_SetSegments( void );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

    // Segment list
    a2dSmrtPtr<a2dVertexList> m_segments;

    a2dCanvasObjectPtr m_canvasobject;
};

//! used to move start or end point in line objects
/*!
    Move begin or end point of a2dSLine or derived.
*/
class A2DCANVASDLLEXP a2dCommand_MovePoint: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_MovePoint() { m_line = NULL; }

    a2dCommand_MovePoint( a2dSLine* object, double x, double y, bool start, bool afterinversion = true );
    ~a2dCommand_MovePoint( void );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

    // x of point
    double m_x;

    // y of point
    double m_y;

    // true of start point is moved
    bool m_start;

    bool m_afterinversion;

    a2dSmrtPtr<a2dSLine> m_line;

};

//! used to change text and caret of canvas text objects in a a2dCanvasDocument
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ChangeText: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_ChangeText() { m_canvasobject = NULL; }

    a2dCommand_ChangeText( a2dText* object, wxString text,
                           int caret, const a2dFont& font, double linespace, unsigned int textflags );
    ~a2dCommand_ChangeText( void );

    bool Do();
    bool Undo();

	a2dText* GetTextObject() { return m_canvasobject; }

    DECLARE_CMH_HABITAT()

protected:

    wxString m_text;
    int m_caret;
    a2dFont m_font;
    double m_linespace;
    unsigned int m_textflags;
    a2dSmrtPtr<a2dText> m_canvasobject;
};

//! used to change style of canvas objects in a a2dCanvasDocument
/*!
*/
class A2DCANVASDLLEXP a2dCommand_ChangeCanvasObjectStyle: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    //! non info.
    static const a2dCommandId sm_SetFillStyleCommand;
    static const a2dCommandId sm_SetStrokeStyleCommand;
    static const a2dCommandId sm_SetStyleCommand;

    a2dCommand_ChangeCanvasObjectStyle() { m_propObject = NULL; }

    a2dCommand_ChangeCanvasObjectStyle( a2dObject* object, const a2dFill& fill );
    a2dCommand_ChangeCanvasObjectStyle( a2dObject* object, const a2dStroke& stroke );
    a2dCommand_ChangeCanvasObjectStyle( a2dObject* object, const a2dFill& fill, const a2dStroke& stroke );

    ~a2dCommand_ChangeCanvasObjectStyle( void );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

protected:

    a2dSmrtPtr< a2dObject > m_propObject;

    a2dFill m_fill;

    a2dStroke m_stroke;

};

//! used to change a property on objects
/*!
    A a2dObject property is set on the a2dObject, and the first property
    on the a2dObject with the same name is used for undo.
    If the property was not yet set for the object, in Undo the new property is just removed, else
    the old value is restored.
*/
class A2DCANVASDLLEXP a2dCommand_SetCanvasProperty: public a2dCommand_SetProperty
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_SetCanvasProperty() {}

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

       \param object the canvas object to set the property on.
       \param property the property to set
    */
    a2dCommand_SetCanvasProperty( a2dObject* object, a2dNamedProperty* property )
        : a2dCommand_SetProperty( object, property ) {}

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the name of the property to set.
        \param value the value to set the property to.
    */
    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdRefObject* id, a2dObject* value )
        : a2dCommand_SetProperty( object, id, value ) {}

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the name of the property to set.
        \param value the value to set the property to.

    */
    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdString* id, const wxString& value )
        : a2dCommand_SetProperty( object, id, value ) {}

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the name of the property to set.
        \param value the value to set the property to.

    */
    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdBool* id, bool value )
        : a2dCommand_SetProperty( object, id, value ) {}

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the name of the property to set.
        \param value the value to set the property to.

    */
    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdInt16* id, wxInt16 value )
        : a2dCommand_SetProperty( object, id, value ) {}

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the name of the property to set.
        \param value the value to set the property to.

    */
    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdUint16* id, wxUint16 value )
        : a2dCommand_SetProperty( object, id, value ) {}

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the name of the property to set.
        \param value the value to set the property to.

    */
    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdInt32* id, wxInt32 value )
        : a2dCommand_SetProperty( object, id, value ) {}

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the name of the property to set.
        \param value the value to set the property to.

    */
    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdUint32* id, wxUint32 value )
        : a2dCommand_SetProperty( object, id, value ) {}

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param object the canvas object to set the property on.
        \param id the name of the property to set.
        \param value the value to set the property to.

    */
    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdDouble* id, double value )
        : a2dCommand_SetProperty( object, id, value ) {}

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param id the name of the property to set.
        \param value the value to set the property to.
        \param object the canvas object to set the property on.
    */
    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdCanvasObject* id, a2dCanvasObject* value );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param id the name of the property to set.
        \param value the value to set the property to.
        \param object the canvas object to set the property on.
    */
    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdMatrix* id, const a2dAffineMatrix& value );

    //! Set property on object
    /*!
        The property its name is used for undo, and restore the old value if available.

        \param id the name of the property to set.
        \param point the point for the internal a2dPoint2DProperty
        \param index the index for the internal a2dPoint2DProperty
        \param afterinversion the afterinversion for the internal a2dPoint2DProperty
        \param object the canvas object to set the property on.
    */
    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdPoint2D* id, const a2dPoint2D& point, int index = -1, bool afterinversion = true );

    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdFill* id, const a2dFill& fill = *a2dNullFILL );

    a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdStroke* id, const a2dStroke& stroke = *a2dNullSTROKE );

    ~a2dCommand_SetCanvasProperty( void );

    virtual a2dCommand* CloneAndBind( a2dObject* object );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()
};


//! used to connect two pins
/*!

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ConnectPins: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_ConnectPins() {}

    a2dCommand_ConnectPins( a2dPin* pin1, a2dPin* pin2 );

    ~a2dCommand_ConnectPins( void );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

    a2dPinPtr m_pin1;
    a2dPinPtr m_pin2;
};

//! used to disconnect two pins
/*!

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_DisConnectPins: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_DisConnectPins() {}

    a2dCommand_DisConnectPins( a2dPin* pin1, a2dPin* pin2 );

    ~a2dCommand_DisConnectPins( void );

    bool Do();
    bool Undo();

    DECLARE_CMH_HABITAT()

    a2dPinPtr m_pin1;
    a2dPinPtr m_pin2;
};

//===========================================================================================
// commands
//===========================================================================================


//! command on selected objects
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_Select: public a2dCommand
{
public:


    //! Print the current document or view
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    //! defines what to do
    /*!
    */
    enum a2dDoWhat
    {
        SelectAll,      //!< select all objects
        DeSelectAll,    //!< de-select all objects
        SelectRect,     //!< select objects in a rectangle area
        DeSelectRect    //!< de-select objects in a rectangle area
    };

    class A2DCANVASDLLEXP Args
    {
    public:
        Args()
        {
            argSet.what = false;
            argSet.layer = false;
            argSet.x1 = false;
            argSet.y1 = false;
            argSet.x2 = false;
            argSet.y2 = false;
            argValue.layer = 0;
            argValue.x1 = 0;
            argValue.y1 = 0;
            argValue.x2 = 0;
            argValue.y2 = 0;
            argValue.layervisible = true;
            argValue.layerselectable = true;
        }

        A2D_ARGUMENT_SETTER( a2dDoWhat, what )
        A2D_ARGUMENT_SETTER( wxUint16, layer )
        A2D_ARGUMENT_SETTER( double, x1 )
        A2D_ARGUMENT_SETTER( double, y1 )
        A2D_ARGUMENT_SETTER( double, x2 )
        A2D_ARGUMENT_SETTER( double, y2 )
        A2D_ARGUMENT_SETTER( bool, layervisible )
        A2D_ARGUMENT_SETTER( bool, layerselectable )

        struct argValue
        {
            a2dDoWhat what;
            wxUint16 layer;
            double x1;
            double y1;
            double x2;
            double y2;
            bool layervisible;
            bool layerselectable;
        } argValue;

        struct argSet
        {
            bool what;
            bool layer;
            bool x1;
            bool y1;
            bool x2;
            bool y2;
            bool layervisible;
            bool layerselectable;
        } argSet;
    };

    a2dCommand_Select() {}

    a2dCommand_Select( a2dCanvasObject* parent, const Args& args ): a2dCommand( true, Id )
    {
        m_args = args;
        m_parent = parent;
        m_oldobjects = wxNullCanvasObjectList;
    }

    ~a2dCommand_Select()
    {
        if ( m_oldobjects != wxNullCanvasObjectList )
            delete m_oldobjects;
    }

    DECLARE_CMH_HABITAT()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_Select( m_parent, m_args );
    }


private:

    Args m_args;
    a2dCanvasObjectList* m_oldobjects;
    a2dlist< bool > m_oldobjectsSelected;

    a2dCanvasObjectPtr m_parent;

    virtual bool Do();

    virtual bool Undo();

};

//! command on selected objects
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_GroupAB: public a2dCommand
{
public:

    //! Print the current document or view
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    //! defines what to do
    /*!
    */
    enum a2dDoWhat
    {
        DeleteGroupA,      //!<  delete group A objects
        MoveGroupA,        //!<  move   group A objects
        CopyGroupA,        //!<  copy   group A objects
        ConvertToArcs, //!<  convert segments in polygon/polyline objects in group A to Arcs where possible
        ConvertToPolygonPolylinesWithArcs, //!<  convert to simple polygons and polylines
        ConvertToPolygonPolylinesWithoutArcs, //!<  convert to simple polygons and polylines
        ConvertPolygonToArcs,   //!<  convert segments in polygon/polyline with width objects in group A to Arcs where possible
        ConvertPolylineToArcs,   //!<  convert segments in polyline objects in group A to Arcs where possible
        ConvertToVPaths, //!<  convert shapes to vector paths
        ConvertLinesArcs, //!< convert shapes to seperate lines and arcs
        ConvertToPolylines, //!< convert shapes to polylines even if polygons
        DetectSmall, //! < generate a report of small objects
        RemoveRedundant,    //! remove redundant segment
        Boolean_OR, /*!< boolean OR operation */
        Boolean_AND, /*!< boolean AND operation */
        Boolean_EXOR, /*!< boolean EX_OR operation */
        Boolean_A_SUB_B, /*!< boolean Group A - Group B operation */
        Boolean_B_SUB_A, /*!< boolean Group B - Group A operation */
        Boolean_CORRECTION, /*!< polygon correction/offset operation */
        Boolean_SMOOTHEN, /*!< smooth operation */
        Boolean_MAKERING, /*!< create a ring on all polygons */
        Boolean_Polygon2Surface, /*!< convert polygon shapes to surfaces */
        Boolean_Surface2Polygon, /*!< convert surfaces shapes to polygons */
        ConvertPointsAtDistance /*!< convert to polygons with vertex at distance x */
    };

    class A2DCANVASDLLEXP Args
    {
    public:
        Args()
        {
            argSet.what = false;
            argSet.target = false;
            argSet.clearTarget = false;
            argSet.selectedA = false;
            argSet.selectedB = false;
            argSet.fileNameOut = false;
            argSet.detectCircle = false;
            argValue.target = 0;
            argValue.clearTarget = false;
            argValue.selectedA = false;
            argValue.selectedB = false;
            argValue.detectCircle = false;
        }

        A2D_ARGUMENT_SETTER( a2dDoWhat, what )
        A2D_ARGUMENT_SETTER( wxUint16, target )
        A2D_ARGUMENT_SETTER( bool, clearTarget )
        A2D_ARGUMENT_SETTER( bool, selectedA )
        A2D_ARGUMENT_SETTER( bool, selectedB )
        A2D_ARGUMENT_SETTER( wxString, fileNameOut )
        A2D_ARGUMENT_SETTER( bool, detectCircle )

        struct argValue
        {
            a2dDoWhat what;
            wxUint16 target;
            bool clearTarget;
            bool selectedA;
            bool selectedB;
            bool detectCircle;
            wxString fileNameOut;
        } argValue;

        struct argSet
        {
            bool what;
            bool target;
            bool clearTarget;
            bool selectedA;
            bool selectedB;
            bool detectCircle;
            bool fileNameOut;
        } argSet;
    };

    a2dCommand_GroupAB( a2dCanvasObject* parent, const Args& args = Args() ): a2dCommand( false, Id )
    {
        m_args = args;
        m_parent = parent;
    }

    DECLARE_CMH_HABITAT()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_GroupAB( m_parent, m_args );
    }

private:

    virtual bool Do();

    virtual bool Undo();

    Args m_args;
    a2dCanvasObjectPtr m_parent;

};

//! create new group at x,y
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_NewGroup: public a2dCommand
{
public:

    //! Print the current document or view
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    class A2DCANVASDLLEXP Args
    {
    public:
        Args()
        {
            argSet.x = false;
            argSet.y = false;
            argSet.name = false;
            argSet.layer = false;
            argValue.x = 0;
            argValue.y = 0;
            argValue.layer = 0;
        }

        A2D_ARGUMENT_SETTER( wxString, name )
        A2D_ARGUMENT_SETTER( wxUint16, layer )
        A2D_ARGUMENT_SETTER( double, x )
        A2D_ARGUMENT_SETTER( double, y )

        struct argValue
        {
            wxString name;
            wxUint16 layer;
            double x;
            double y;
        } argValue;

        struct argSet
        {
            bool name;
            bool layer;
            bool x;
            bool y;
        } argSet;
    };

    a2dCommand_NewGroup( a2dCanvasObject* parent, const Args& args = Args() ): a2dCommand( false, Id )
    {
        m_args = args;
        m_parent = parent;
    }

    DECLARE_CMH_HABITAT()

    Args m_args;
    a2dCanvasObjectPtr m_parent;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_NewGroup( m_parent, m_args );
    }

private:

    virtual bool Do()
    {
        a2dCanvasObject* group = new a2dCanvasObject();
        group->CreateChildObjectList();

        wxString groupname = m_args.argValue.name;
        if ( groupname.IsEmpty() )
        {
            wxTextEntryDialog getname( NULL, wxT( "Give group name:" ), wxT( "Group name" ) );
            if ( wxID_OK == getname.ShowModal() )
                groupname = getname.GetValue();
            else
                groupname = wxT( "Not Specified" );
        }

        group->SetName( groupname );
        group->SetLayer( m_args.argValue.layer );
        group->CreateChildObjectList();
        group->SetIgnoreLayer( true );
        a2dCanvasObjectReference* groupref = new a2dCanvasObjectReference( m_args.argValue.x, m_args.argValue.y, group );
        m_parent->Prepend( groupref );

        return true;
    }

    virtual bool Undo()
    {
        return false;
    }
};

//! create new group at x,y
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_NewPin: public a2dCommand
{
public:

    //! Print the current document or view
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    class A2DCANVASDLLEXP Args
    {
    public:
        Args()
        {
            argSet.x = false;
            argSet.y = false;
            argSet.name = false;
            argValue.x = 0;
            argValue.y = 0;
        }

        A2D_ARGUMENT_SETTER( wxString, name )
        A2D_ARGUMENT_SETTER( double, x )
        A2D_ARGUMENT_SETTER( double, y )

        struct argValue
        {
            wxString name;
            double x;
            double y;
        } argValue;

        struct argSet
        {
            bool name;
            bool x;
            bool y;
        } argSet;
    };

    a2dCommand_NewPin( a2dCanvasObject* parent, const Args& args = Args() ): a2dCommand( false, Id )
    {
        m_args = args;
        m_parent = parent;
    }

    DECLARE_CMH_HABITAT()

    Args m_args;
    a2dCanvasObjectPtr m_parent;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_NewPin( m_parent, m_args );
    }

private:

    virtual bool Do()
    {
        if ( !m_args.argSet.name )
            m_args.argValue.name = wxGetTextFromUser( _( "give name of new pin:" ) );
        long ang = wxGetNumberFromUser( _( "Give pin angle:" ), _( "angle:" ), _( "pin angle" ), 0, -360, 360 );

        a2dPin* pin = new a2dPin( m_parent, m_args.argValue.name, a2dPinClass::Standard, m_args.argValue.x, m_args.argValue.y, ang );
        m_parent->Prepend( pin );
        return true;
    }

    virtual bool Undo()
    {
        return false;
    }
};



//! set layer of objects which fit the mask
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetLayerMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_SetLayerMask( a2dCanvasObject* parent, long set = -1, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_layer = set;
        m_mask = mask;
        m_objects = wxNullCanvasObjectList;
    }

    ~a2dCommand_SetLayerMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    long m_layer;
    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dLayerGroup m_objectOldLayers;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_SetLayerMask( m_parent, m_layer, m_mask );
    }
};

//! delete objects which fit the mask
/*!
	The bin flag is temporarily used to skip connected wire optimization, for objects fitting the mask.
	\see a2dCommand_ReleaseObject::PreDo()

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_DeleteMask: public a2dCommandMasked
{
public:
    static const a2dCommandId Id;

    a2dCommand_DeleteMask( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED );

    ~a2dCommand_DeleteMask()
    {
    }

    bool Do( void );

    bool Undo( void );

    bool Redo( void );

    DECLARE_CMH_HABITAT()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_DeleteMask( m_parentObject, m_which );
    }
};

//! objects which fit the mask to top in parent (rendered last)
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ToTopMask: public a2dCommandMasked
{
public:
    static const a2dCommandId Id;

    a2dCommand_ToTopMask( a2dCanvasObject* parent , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED );

    ~a2dCommand_ToTopMask()
    {
    }

    bool Do( void );

    bool Undo( void );

	bool Redo( void );

    DECLARE_CMH_HABITAT()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_ToTopMask( m_parentObject, m_which );
    }
};

//! objects which fit the mask to bottom in parent (rendered first)
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ToBackMask: public a2dCommandMasked
{
public:
    static const a2dCommandId Id;

    a2dCommand_ToBackMask( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED );

    ~a2dCommand_ToBackMask()
    {
    }

    bool Do( void );

    bool Undo( void );

	bool Redo( void );

    DECLARE_CMH_HABITAT()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_ToBackMask( m_parentObject, m_which );
    }
};


//! objects which fit the mask are copied to target and translated
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_CopyMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_CopyMask( a2dCanvasObject* parent, double x = 0, double y = 0, long target = -1, bool toTarget = false, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_x = x;
        m_y = y;
        m_target = target;
        m_toTarget = toTarget;
        m_objects = wxNullCanvasObjectList;
    }

    ~a2dCommand_CopyMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    double m_x, m_y;
    long m_target;
    bool m_toTarget;
    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_CopyMask( m_parent, m_x, m_y, m_target, m_toTarget, m_mask );
    }
};

//! objects which fit the mask are moved to target and translated
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_MoveMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_MoveMask(  a2dCanvasObject* parent, double x = 0, double y = 0, long target = -1, bool toTarget = false, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_x = x;
        m_y = y;
        m_target = target;
        m_toTarget = toTarget;
        m_objects = wxNullCanvasObjectList;
    }

    ~a2dCommand_MoveMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    double m_x, m_y;
    long m_target;
    bool m_toTarget;
    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dLayerGroup m_objectOldLayers;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_MoveMask( m_parent, m_x, m_y, m_target, m_toTarget, m_mask );
    }
};

//! objects which fit the mask are grouped into a new object
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_GroupMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_GroupMask( a2dCanvasObject* parent, const wxString& name = wxEmptyString, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED ): a2dCommand( true, Id )
    {
        m_name = name;
        m_parent = parent;
        m_mask = mask;
        m_objects = wxNullCanvasObjectList;
    }

    ~a2dCommand_GroupMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    wxString m_name;
    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dCanvasObjectPtr m_groupobject;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_GroupMask( m_parent, m_name, m_mask );
    }
};

//! objects which fit the mask are grouped into a new object
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_UnGroupMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_UnGroupMask( a2dCanvasObject* parent, bool deep = false, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_deep = deep;
        m_objects = wxNullCanvasObjectList;
    }

    ~a2dCommand_UnGroupMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    a2dCanvasObjectFlagsMask m_mask;
    bool m_deep;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_UnGroupMask( m_parent, m_deep, m_mask );
    }
};

//! objects which fit the mask are merge into one new object
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_MergeMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_MergeMask( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED,
                          long target = -1, bool layerFirstObject = false, bool clearorg = true,
                          int depth = 20 ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_takeLayerFirstObject = layerFirstObject;
        m_target = target;
        m_clearorg = clearorg;
        m_depth = depth;
        m_objects = wxNullCanvasObjectList;
        m_mergedObjects = wxNullCanvasObjectList;
    }

    ~a2dCommand_MergeMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
        if ( m_mergedObjects != wxNullCanvasObjectList )
            delete m_mergedObjects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    long m_target;
    bool m_takeLayerFirstObject;
    bool m_clearorg;
    int m_depth;
    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dCanvasObjectList* m_mergedObjects;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_MergeMask( m_parent, m_mask, m_target, m_takeLayerFirstObject, m_clearorg, m_depth );
    }
};

//! objects which fit the mask are offset corrected
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_OffsetMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_OffsetMask( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED,
                           long target = -1, bool layerFirstObject = false, bool clearorg = true,
                           int depth = 20 ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_takeLayerFirstObject = layerFirstObject;
        m_target = target;
        m_clearorg = clearorg;
        m_depth = depth;
        m_objects = wxNullCanvasObjectList;
        m_offsetObjects = wxNullCanvasObjectList;
    }

    ~a2dCommand_OffsetMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
        if ( m_offsetObjects != wxNullCanvasObjectList )
            delete m_offsetObjects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    long m_target;
    bool m_takeLayerFirstObject;
    bool m_clearorg;
    int m_depth;
    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dCanvasObjectList* m_offsetObjects;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_OffsetMask( m_parent, m_mask, m_target, m_takeLayerFirstObject, m_clearorg, m_depth );
    }
};

//! objects which fit the mask are used for creating a ring
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_CreateRingMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_CreateRingMask( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED,
                           long target = -1, bool layerFirstObject = false, bool clearorg = true,
                           int depth = 20 ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_takeLayerFirstObject = layerFirstObject;
        m_target = target;
        m_clearorg = clearorg;
        m_depth = depth;
        m_objects = wxNullCanvasObjectList;
        m_CreateRingObjects = wxNullCanvasObjectList;
    }

    ~a2dCommand_CreateRingMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
        if ( m_CreateRingObjects != wxNullCanvasObjectList )
            delete m_CreateRingObjects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    long m_target;
    bool m_takeLayerFirstObject;
    bool m_clearorg;
    int m_depth;
    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dCanvasObjectList* m_CreateRingObjects;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_CreateRingMask( m_parent, m_mask, m_target, m_takeLayerFirstObject, m_clearorg, m_depth );
    }
};

//! objects which fit the mask are offset corrected
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_PolygonToSurfaceMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_PolygonToSurfaceMask( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED,
                                     long target = -1, bool layerFirstObject = false, bool clearorg = true,
                                     int depth = 20 ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_takeLayerFirstObject = layerFirstObject;
        m_target = target;
        m_clearorg = clearorg;
        m_depth = depth;
        m_objects = wxNullCanvasObjectList;
        m_offsetObjects = wxNullCanvasObjectList;
    }

    ~a2dCommand_PolygonToSurfaceMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
        if ( m_offsetObjects != wxNullCanvasObjectList )
            delete m_offsetObjects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    long m_target;
    bool m_takeLayerFirstObject;
    bool m_clearorg;
    int m_depth;
    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dCanvasObjectList* m_offsetObjects;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_PolygonToSurfaceMask( m_parent, m_mask, m_target, m_takeLayerFirstObject, m_clearorg, m_depth );
    }
};

//! objects which fit the mask are offset corrected
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SurfaceToPolygonMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_SurfaceToPolygonMask( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED,
                                     long target = -1, bool layerFirstObject = false, bool clearorg = true,
                                     int depth = 20 ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_takeLayerFirstObject = layerFirstObject;
        m_target = target;
        m_clearorg = clearorg;
        m_depth = depth;
        m_objects = wxNullCanvasObjectList;
        m_offsetObjects = wxNullCanvasObjectList;
    }

    ~a2dCommand_SurfaceToPolygonMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
        if ( m_offsetObjects != wxNullCanvasObjectList )
            delete m_offsetObjects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    long m_target;
    bool m_takeLayerFirstObject;
    bool m_clearorg;
    int m_depth;
    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_objects;
    a2dCanvasObjectList* m_offsetObjects;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_SurfaceToPolygonMask( m_parent, m_mask, m_target, m_takeLayerFirstObject, m_clearorg, m_depth );
    }
};

//! objects which fit the mask are given an Url property
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_UrlOnMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_UrlOnMask( a2dCanvasObject* parent, wxURI uri = wxURI(), a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_uri = uri;
        m_objects = wxNullCanvasObjectList;
        m_oldobjects = wxNullCanvasObjectList;
    }

    ~a2dCommand_UrlOnMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
        if ( m_oldobjects != wxNullCanvasObjectList )
            delete m_oldobjects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_oldobjects;
    a2dCanvasObjectList* m_objects;
    a2dlist< long > m_objectsIndex;
    wxURI m_uri;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_UrlOnMask( m_parent, m_uri, m_mask );
    }
};

//! objects which fit the mask are given a new fill and stroke style
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetFillStrokeMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_SetFillStrokeMask( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_objects = wxNullCanvasObjectList;
        m_oldobjects = wxNullCanvasObjectList;
    }

    ~a2dCommand_SetFillStrokeMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
        if ( m_oldobjects != wxNullCanvasObjectList )
            delete m_oldobjects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_oldobjects;
    a2dCanvasObjectList* m_objects;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_SetFillStrokeMask( m_parent, m_mask );
    }
};

//! objects which fit the mask are given a new fill and stroke style
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetFillStrokeExtMask: public a2dCommandMasked
{
public:
    static const a2dCommandId Id;

	a2dCommand_SetFillStrokeExtMask( a2dCanvasObject* parent, const a2dExtStroke& extStroke, const a2dExtFill& extFill,  a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED );

    ~a2dCommand_SetFillStrokeExtMask()
    {
    }

    bool Do( void );

    bool Undo( void );

	bool Redo( void );

    DECLARE_CMH_HABITAT()

    a2dlist< a2dStroke > m_strokes;
    a2dlist< a2dFill > m_fills;

	a2dExtStroke m_extStroke;
	a2dExtFill m_extFill;

protected:
    void RestorePreviousState();

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return NULL;//new a2dCommand_SetFillStrokeExtMask( m_parent, m_mask );
    }

    //! objects modified by this command
    a2dCanvasObjectList m_modifiedObjects;
};

//! objects which fit the mask are given a new font and other text properties
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetTextChangesMask: public a2dCommandMasked
{
public:
    static const a2dCommandId Id;

	a2dCommand_SetTextChangesMask( a2dCanvasObject* parent, const a2dTextChanges& extFont, int alignment, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED ); 

    ~a2dCommand_SetTextChangesMask()
    {
    }

    bool Do( void );

    bool Undo( void );

	bool Redo( void );
	
    DECLARE_CMH_HABITAT()

    a2dlist< a2dFont > m_fonts;
    a2dlist< int > m_alignmentslist;
    a2dlist< unsigned int > m_textflagslist;

	a2dTextChanges m_extFont;
    int m_alignment;

protected:
    void RestorePreviousState();

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return NULL;
    }

    //! objects modified by this command
    a2dCanvasObjectList m_modifiedObjects;
};

//! objects which fit the mask are converted to rectangles
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ImagesToRectMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_ImagesToRectMask( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED ): a2dCommand( true, Id )
    {
        m_parent = parent;
        m_mask = mask;
        m_objects = wxNullCanvasObjectList;
        m_oldobjects = wxNullCanvasObjectList;
    }

    ~a2dCommand_ImagesToRectMask()
    {
        if ( m_objects != wxNullCanvasObjectList )
            delete m_objects;
        if ( m_oldobjects != wxNullCanvasObjectList )
            delete m_oldobjects;
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    a2dCanvasObjectFlagsMask m_mask;
    a2dCanvasObjectPtr m_parent;
    a2dCanvasObjectList* m_oldobjects;
    a2dCanvasObjectList* m_objects;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_ImagesToRectMask( m_parent, m_mask );
    }
};

//! set all layers visible or not
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetLayersProperty: public a2dCommand
{
public:
    static const a2dCommandId  Id;

    enum SetWhat { visible, selectable, readable, filling };

    a2dCommand_SetLayersProperty( a2dLayers* layerSetup, bool value = true, SetWhat what = visible ): a2dCommand( true, Id )
    {
        m_layerSetup = layerSetup;
        m_value = value;
        m_setWhat = what;
    }

    ~a2dCommand_SetLayersProperty()
    {
    }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    bool m_value;
    a2dlist< bool > m_oldLayersVisible;

    SetWhat m_setWhat;

    a2dSmrtPtr<a2dLayers> m_layerSetup;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_SetLayersProperty( m_layerSetup, m_value );
    }

};

//! objects which fit the mask are converted to polygons or vector paths
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ConvertMask: public a2dCommand
{
public:
    static const a2dCommandId Id;

    //! defines what to do
    /*!
    */
    enum a2dDoWhat
    {
        ConvertToPolygonPolylinesWithArcs, //!<  convert to simple polygons and polylines
        ConvertToPolygonPolylinesWithoutArcs, //!<  convert to simple polygons and polylines
        ConvertPolygonToArcs,   //!<  convert segments in polygon/polyline with width objects in group A to Arcs where possible
        ConvertPolylineToArcs,   //!<  convert segments in polyline objects in group A to Arcs where possible
        ConvertToVPaths, //!<  convert shapes to vector paths
        ConvertLinesArcs, //!< convert shapes to seperate lines and arcs
        ConvertToPolylines, //!< convert shapes to polylines even if polygons
    };

    class A2DCANVASDLLEXP Args
    {
    public:
        Args()
        {
            argSet.what = false;
            argSet.target = false;
            argSet.clearTarget = false;
            argSet.mask = false;
            argSet.deleteOriginal = false;
            argSet.keepLayer = false;
            argSet.clearTarget = false;
            argSet.detectCircle = false;
            argSet.detectCircle = false;
            argSet.parent = false;
            argValue.what = ConvertToVPaths;
            argValue.target = -1;
            argValue.clearTarget = false;
            argValue.mask = a2dCanvasOFlags::SELECTED;
            argValue.deleteOriginal = true;
            argValue.keepLayer = true;
            argValue.clearTarget = false;
            argValue.detectCircle = false;
            argValue.parent = NULL;
        }

        A2D_ARGUMENT_SETTER( a2dDoWhat, what )
        A2D_ARGUMENT_SETTER( long, target )
        A2D_ARGUMENT_SETTER( a2dCanvasObjectFlagsMask, mask )
        A2D_ARGUMENT_SETTER( bool, deleteOriginal )
        A2D_ARGUMENT_SETTER( bool, keepLayer )
        A2D_ARGUMENT_SETTER( bool, clearTarget )
        A2D_ARGUMENT_SETTER( bool, detectCircle )
        A2D_ARGUMENT_SETTER( a2dCanvasObjectPtr, parent )

        struct argValue
        {
            a2dDoWhat what;
            long target;
            a2dCanvasObjectFlagsMask mask;
            bool deleteOriginal;
            bool keepLayer;
            bool clearTarget;
            bool detectCircle;
            a2dCanvasObjectPtr parent;
        } argValue;

        struct argSet
        {
            bool what, target, mask, deleteOriginal, keepLayer, clearTarget, detectCircle, parent;
        } argSet;
    };

    a2dCommand_ConvertMask( const Args& args = Args() ): a2dCommand( true, Id )
    {
        m_args = args;
        m_newobjects = wxNullCanvasObjectList;
        m_maskobjects = wxNullCanvasObjectList;
    }

    a2dCommand_ConvertMask( a2dCanvasObject* parent, a2dDoWhat what,
                            long target = -1, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::SELECTED,
                            bool deleteOriginal = true, bool keepLayer = true )
        : a2dCommand( true, Id )
    {
        m_args.what( what ).target( target ).mask( mask ).deleteOriginal( deleteOriginal ).keepLayer( keepLayer ).parent( parent );
        m_newobjects = wxNullCanvasObjectList;
        m_maskobjects = wxNullCanvasObjectList;
    }

    ~a2dCommand_ConvertMask()
    {
        if ( m_newobjects != wxNullCanvasObjectList )
            delete m_newobjects;
        if ( m_maskobjects != wxNullCanvasObjectList )
            delete m_maskobjects;
    }

    bool Do( void );

    bool Undo( void );

    bool PreDo();

    DECLARE_CMH_HABITAT()

    Args m_args;

    a2dCanvasObjectList* m_newobjects;
    a2dCanvasObjectList* m_maskobjects;
    a2dlist< long > m_objectsIndex;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_ConvertMask( m_args );
    }
};

//!
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetShowObject: public a2dCommand
{
public:

    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    class A2DCANVASDLLEXP Args
    {
    public:
        Args()
        {
            argSet.canvasobject = false;
            argSet.index = false;
            argSet.selected = false;
            argSet.name = false;
            argValue.canvasobject = NULL;
            argValue.index = 0;
            argValue.selected = false;
        }

        A2D_ARGUMENT_SETTER( a2dCanvasObjectPtr, canvasobject )
        A2D_ARGUMENT_SETTER( wxInt32, index )
        A2D_ARGUMENT_SETTER( wxString, name )
        A2D_ARGUMENT_SETTER( bool, selected )

        struct argValue
        {
            a2dCanvasObjectPtr canvasobject;
            wxInt32 index;
            wxString name;
            bool selected;
        } argValue;

        struct argSet
        {
            bool canvasobject;
            bool name;
            bool index;
            bool selected;
        } argSet;
    };

    a2dCommand_SetShowObject( a2dDrawingPart* drawingPart, const Args& args = Args() ): a2dCommand( false, Id )
    {
        m_args = args;
        m_drawingPart = drawingPart;
    }

    virtual bool Do();

    virtual bool Undo();

    DECLARE_CMH_HABITAT()

    Args m_args;
    a2dDrawingPart* m_drawingPart;

    a2dCanvasObjectPtr m_previous;
    a2dCanvasObjectPtr m_showobject;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_SetShowObject( m_drawingPart, m_args );
    }
};

//!
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_PushInto: public a2dCommand
{
public:

    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    class A2DCANVASDLLEXP Args
    {
    public:
        Args()
        {
            argSet.canvasobject = false;
            argSet.name = false;
            argSet.selected = false;
            argValue.selected = false;
            argValue.canvasobject = NULL;
        }

        A2D_ARGUMENT_SETTER( a2dCanvasObjectPtr, canvasobject )
        A2D_ARGUMENT_SETTER( wxString, name )
        A2D_ARGUMENT_SETTER( bool, selected )

        struct argValue
        {
            a2dCanvasObjectPtr canvasobject;
            wxString name;
            bool selected;
        } argValue;

        struct argSet
        {
            bool canvasobject;
            bool name;
            bool selected;
        } argSet;
    };

    a2dCommand_PushInto( a2dDrawingPart* drawingPart, const Args& args = Args() ): a2dCommand( true, Id )
    {
        m_args = args;
        m_drawingPart = drawingPart;
    }

    virtual bool Do();

    virtual bool Undo();

    DECLARE_CMH_HABITAT()

    Args m_args;
    a2dDrawingPart* m_drawingPart;

private:

    a2dCanvasObjectPtr m_previous;
    a2dCanvasObjectPtr m_pushIn;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_PushInto( m_drawingPart, m_args );
    }
};

//! layer group A and B commands
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetLayerGroup: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_SetLayerGroup )
public:

    //!  layer group A and B command
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    //! defines what to do
    /*!
    */
    enum a2dDoWhat
    {
        SetGroupA,      //!< add a layer to a operation group A
        SetGroupB,      //!< add a layer to a operation group B
        AddGroupA,      //!< add a layer to a operation group A
        AddGroupB,      //!< add a layer to a operation group B
        ClearGroupA,      //!< clear all layers in operation group A
        ClearGroupB,      //!< clear all layers in operation group B
    };

    class A2DCANVASDLLEXP Args
    {
    public:
        Args()
        {
            argSet.what = false;
            argSet.layer = false;
            argSet.group = false;
            argValue.layer = 0;
        }

        A2D_ARGUMENT_SETTER( a2dDoWhat, what )
        A2D_ARGUMENT_SETTER( wxUint16, layer )
        A2D_ARGUMENT_SETTER( a2dLayerGroup, group )

        struct argValue
        {
            a2dDoWhat what;
            wxUint16 layer;
            a2dLayerGroup group;
        } argValue;

        struct argSet
        {
            bool what;
            bool layer;
            bool group;
        } argSet;
    };

    a2dCommand_SetLayerGroup( const Args& args  = Args() ): a2dCommand( false, Id )
    {
        m_args = args;
    }

    DECLARE_CMH_HABITAT()

    Args m_args;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_SetLayerGroup( m_args );
    }

    virtual bool Do()
    {
        if ( m_args.argValue.what == SetGroupA )
        {
            GetDrawHabitat()->GetGroupA() = m_args.argValue.group;
        }
        else if ( m_args.argValue.what == SetGroupB )
        {
            GetDrawHabitat()->GetGroupB() = m_args.argValue.group;
        }
        else if ( m_args.argValue.what == AddGroupA )
        {
            GetDrawHabitat()->GetGroupA().push_back( m_args.argValue.layer );
        }
        else if ( m_args.argValue.what == AddGroupB )
        {
            GetDrawHabitat()->GetGroupB().push_back( m_args.argValue.layer );
        }
        else if ( m_args.argValue.what == ClearGroupA )
        {
            GetDrawHabitat()->GetGroupA().clear();
        }
        else if ( m_args.argValue.what == ClearGroupB )
        {
            GetDrawHabitat()->GetGroupB().clear();
        }
        return false;
    }

    virtual bool Undo()
    {
        return false;
    }
};

//! set
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetStroke: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_SetStroke )
public:
    static const a2dCommandId Id;

    a2dCommand_SetStroke( const a2dStroke& stroke = *a2dNullSTROKE ): a2dCommand( true, Id )
    {
        m_value = stroke;
    }

    a2dStroke m_value;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_SetStroke( m_value );
    }

    bool Do( void )
    {
        m_value = GetDrawHabitat()->GetStroke();

        a2dStroke val = GetDrawHabitat()->GetStroke();
        GetDrawHabitat()->SetStroke( m_value );
        m_value = val;
        return true;

    }
    bool Undo( void )
    {
        return Do();
    }

    DECLARE_CMH_HABITAT()

};

//! set
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetFill: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_SetFill )
public:
    static const a2dCommandId Id;

    a2dCommand_SetFill( const a2dFill& fill = *a2dNullFILL ): a2dCommand( true, Id )
    {
        m_value = fill;
    }

    bool Do( void )
    {
        a2dFill val = GetDrawHabitat()->GetFill();
        GetDrawHabitat()->SetFill( m_value );
        m_value = val;
        return true;

    }
    bool Undo( void )
    {
        return Do();
    }

    DECLARE_CMH_HABITAT()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_SetFill( m_value );
    }

    a2dFill m_value;
};

class A2DCANVASDLLEXP a2dCommand_SetStrokeColour: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_SetStrokeColour )
public:
    static const a2dCommandId Id;

    a2dCommand_SetStrokeColour( const wxColour& colour = *wxBLACK ): a2dCommand( true, Id )
    {
        m_value = colour;
    }

    wxColour m_value;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_SetStrokeColour( m_value );
    }

    bool Do( void )
    {
        a2dStroke stroke = GetDrawHabitat()->GetStroke();
        wxColour old = stroke.GetColour();
        stroke.SetColour( m_value );
        m_value = old;
        return true;

    }
    bool Undo( void )
    {
        return Do();
    }

    DECLARE_CMH_HABITAT()
};

class A2DCANVASDLLEXP a2dCommand_SetFillColour: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_SetFillColour )
public:
    static const a2dCommandId Id;

    a2dCommand_SetFillColour( const wxColour& colour = *wxBLACK ): a2dCommand( true, Id )
    {
        m_value = colour;
    }

    wxColour m_value;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_SetFillColour( m_value );
    }

    bool Do( void )
    {
        a2dFill fill = GetDrawHabitat()->GetFill();
        wxColour old = fill.GetColour();
        fill.SetColour( m_value );
        m_value = old;
        return true;

    }
    bool Undo( void )
    {
        return Do();
    }

    DECLARE_CMH_HABITAT()

};

//! set cursor
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_SetCursor: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_SetCursor )

public:
    static const a2dCommandId Id;


    class A2DCANVASDLLEXP Args
    {
    public:
        Args()
        {
            argValue.x = 0;
            argValue.y = 0;
            argValue.polar = false;
            argValue.relative = false;
            argValue.snapped = false;

            argSet.x = false;
            argSet.y = false;
            argSet.polar = false;
            argSet.relative = false;
            argSet.snapped = false;
        }

        A2D_ARGUMENT_SETTER( double, x )
        A2D_ARGUMENT_SETTER( double, y )
        A2D_ARGUMENT_SETTER( bool, polar )
        A2D_ARGUMENT_SETTER( bool, relative )
        A2D_ARGUMENT_SETTER( bool, snapped )

        struct argValue
        {
            double x;
            double y;
            bool polar;
            bool relative;
            bool snapped;
        } argValue;

        struct argSet
        {
            bool x;
            bool y;
            bool polar;
            bool relative;
            bool snapped;
        } argSet;
    };

    a2dCommand_SetCursor( const Args& args = Args() ): a2dCommand( false, Id )
    {
        m_args = args;
    }

    a2dCommand_SetCursor( double x, double y ): a2dCommand( true, Id )
    {
        m_args.x( x );
        m_args.y( y );
    }

	double GetX() const { return m_args.argValue.x; }
	double GetY() const { return m_args.argValue.y; }

    bool Do( void );

    bool Undo( void );

    DECLARE_CMH_HABITAT()

    Args m_args;
    double m_xold, m_yold;

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    {
        return new a2dCommand_SetCursor( m_args );
    }
};

//! used to add objects to a a2dCanvasDocument in the current parent
/*!
   The objects to add can be connected. Connections are preserved in undo  and redo

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_AddObjects: public a2dCommand
{

public:

    //! used to release object from a a2dCanvasDocument in the current parent
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_AddObjects() { m_parent = NULL; }

    a2dCommand_AddObjects( a2dCanvasObject* parent,  a2dCanvasObjectList objects );

    bool Do();
    bool Undo();
    bool Redo();
    
    a2dCanvasObject* GetParent() { return m_parent; }

    a2dCanvasObjectList& GetObjectList() { return m_objects; }

    DECLARE_CMH_HABITAT()

protected:

    a2dCanvasObjectList m_objects;
    a2dCanvasObjectPtr m_parent;
};

#endif /* __WXDRAWING_H__ */
