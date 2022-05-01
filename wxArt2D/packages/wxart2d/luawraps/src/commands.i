#include "wx/artbase/artmod.h"
#include "wx/docview.h"
#include "wx/docview/docviewref.h"
#include "wx/general/gen.h"
#include "wx/docview/doccom.h"
#include "wx/canvas/canmod.h"
#include "wx/editor/editmod.h"

// ---------------------------------------------------------------------------
// a2dMenuIdItem
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
#include "wx/editor/cancom.h"
#include "wx/editor/candocproc.h"

    %function a2dMenuIdItem& CmdMenu_Exit();
    %function a2dMenuIdItem& CmdMenu_FileClose();
    %function a2dMenuIdItem& CmdMenu_FileOpen();
    %function a2dMenuIdItem& CmdMenu_FileNew();
    %function a2dMenuIdItem& CmdMenu_FileSave();
    %function a2dMenuIdItem& CmdMenu_FileSaveAs();
    %function a2dMenuIdItem& CmdMenu_FileImport();
    %function a2dMenuIdItem& CmdMenu_FileExport();
    %function a2dMenuIdItem& CmdMenu_FileRevert();
    %function a2dMenuIdItem& CmdMenu_Print();
    %function a2dMenuIdItem& CmdMenu_Preview();
    %function a2dMenuIdItem& CmdMenu_PrintView();
    %function a2dMenuIdItem& CmdMenu_PreviewView();
    %function a2dMenuIdItem& CmdMenu_PrintDocument();
    %function a2dMenuIdItem& CmdMenu_PreviewDocument();
    %function a2dMenuIdItem& CmdMenu_PrintSetup();
    %function a2dMenuIdItem& CmdMenu_EmptyDocument();
    %function a2dMenuIdItem& CmdMenu_EmptyShownObject();

	%function a2dMenuIdItem& CmdMenu_ChangeCanvasObjectStyle();
    %function a2dMenuIdItem& CmdMenu_ChangeCentralStyle();
    %function a2dMenuIdItem& CmdMenu_SetLayer();
    %function a2dMenuIdItem& CmdMenu_SetTarget();
    %function a2dMenuIdItem& CmdMenu_SetDrawWireFrame(); 
    %function a2dMenuIdItem& CmdMenu_DrawGridLines(); 
    %function a2dMenuIdItem& CmdMenu_DrawGridAtFront();
    %function a2dMenuIdItem& CmdMenu_SetYAxis();
    %function a2dMenuIdItem& CmdMenu_Option_SplinePoly();
    %function a2dMenuIdItem& CmdMenu_Option_CursorCrosshair();
    %function a2dMenuIdItem& CmdMenu_Option_RescaleAtReSize();
    %function a2dMenuIdItem& CmdMenu_Option_ReverseLayers();

    
    
    
    %function a2dMenuIdItem& CmdMenu_Refresh();
    %function a2dMenuIdItem& CmdMenu_ZoomOut();
    %function a2dMenuIdItem& CmdMenu_ZoomOut2();
    %function a2dMenuIdItem& CmdMenu_ZoomIn2();
    %function a2dMenuIdItem& CmdMenu_EmptyShownObject();
    %function a2dMenuIdItem& CmdMenu_DrawGrid();
    %function a2dMenuIdItem& CmdMenu_SetSnap();
    %function a2dMenuIdItem& CmdMenu_SetSnapFeatures();
    %function a2dMenuIdItem& CmdMenu_SetSnapFeature();
    %function a2dMenuIdItem& CmdMenu_SetStroke();
    %function a2dMenuIdItem& CmdMenu_SetFill();
    %function a2dMenuIdItem& CmdMenu_SetStrokeColour();
    %function a2dMenuIdItem& CmdMenu_SetFillColour();
    %function a2dMenuIdItem& CmdMenu_SetContourWidth();
    %function a2dMenuIdItem& CmdMenu_SetPathType();
    %function a2dMenuIdItem& CmdMenu_SetNormalizeFactor();

    %function a2dMenuIdItem& CmdMenu_ShowDiagram();
    %function a2dMenuIdItem& CmdMenu_ShowSymbol();
    %function a2dMenuIdItem& CmdMenu_ShowGui();

    
    %function a2dMenuIdItem& CmdMenu_PushInto();
    %function a2dMenuIdItem& CmdMenu_PopOut();
    %function a2dMenuIdItem& CmdMenu_NewGroup();
    %function a2dMenuIdItem& CmdMenu_NewPin();
    %function a2dMenuIdItem& CmdMenu_SaveLayersDrawing();
    %function a2dMenuIdItem& CmdMenu_SaveLayersDrawingHome();
    %function a2dMenuIdItem& CmdMenu_SaveLayersDrawingLastDir();
    %function a2dMenuIdItem& CmdMenu_SaveLayersDrawingCurrentDir();
    %function a2dMenuIdItem& CmdMenu_LoadLayersDrawing();
    %function a2dMenuIdItem& CmdMenu_LoadLayersDrawingHome();
    %function a2dMenuIdItem& CmdMenu_LoadLayersDrawingLastDir();
    %function a2dMenuIdItem& CmdMenu_LoadLayersDrawingCurrentDir();
    %function a2dMenuIdItem& CmdMenu_AddLayerIfMissing();

    %function a2dMenuIdItem& CmdMenu_SetCursor();
    %function a2dMenuIdItem& CmdMenu_SetRelativeStart();
    %function a2dMenuIdItem& CmdMenu_AddMeasurement();
    %function a2dMenuIdItem& CmdMenu_ClearMeasurements();

    
    %function a2dMenuIdItem& CmdMenu_Selected_SelectAll();
    %function a2dMenuIdItem& CmdMenu_Selected_DeSelectAll();
    %function a2dMenuIdItem& CmdMenu_Selected_Delete();
    %function a2dMenuIdItem& CmdMenu_Selected_Group();
    %function a2dMenuIdItem& CmdMenu_Selected_GroupNamed();
    %function a2dMenuIdItem& CmdMenu_Selected_ImageToRectangles();
    %function a2dMenuIdItem& CmdMenu_Selected_ToTop();
    %function a2dMenuIdItem& CmdMenu_Selected_ToBack();
    %function a2dMenuIdItem& CmdMenu_Selected_SetStyle();
    %function a2dMenuIdItem& CmdMenu_Selected_SetExtStyle();
    %function a2dMenuIdItem& CmdMenu_Selected_ConvertToPolygonPolylinesWithArcs();
    %function a2dMenuIdItem& CmdMenu_Selected_ConvertToPolygonPolylinesWithoutArcs();
    %function a2dMenuIdItem& CmdMenu_Selected_ConvertToPolylines();
    %function a2dMenuIdItem& CmdMenu_Selected_ConvertLinesArcs();
    %function a2dMenuIdItem& CmdMenu_Selected_ConvertToVPaths();
    %function a2dMenuIdItem& CmdMenu_Selected_ConvertPolygonToArcs();
    %function a2dMenuIdItem& CmdMenu_Selected_ConvertPolylineToArcs();
    %function a2dMenuIdItem& CmdMenu_Selected_ConvertPolygonToSurface();
    %function a2dMenuIdItem& CmdMenu_Selected_ConvertSurfaceToPolygon();
    %function a2dMenuIdItem& CmdMenu_Selected_ConvertPathToPolygon();
    %function a2dMenuIdItem& CmdMenu_Selected_UnGroup();
    %function a2dMenuIdItem& CmdMenu_Selected_UnGroupDeep();
    %function a2dMenuIdItem& CmdMenu_Selected_Merge();
    %function a2dMenuIdItem& CmdMenu_Selected_Offset();
    %function a2dMenuIdItem& CmdMenu_Selected_CreateRing();
    %function a2dMenuIdItem& CmdMenu_Selected_CameleonDiagram();
    %function a2dMenuIdItem& CmdMenu_Selected_CameleonSymbol();
    %function a2dMenuIdItem& CmdMenu_Selected_FlattenCameleon();
    %function a2dMenuIdItem& CmdMenu_Selected_CloneCameleonFromInst();

    %function a2dMenuIdItem& CmdMenu_GroupAB_Offset();
    %function a2dMenuIdItem& CmdMenu_GroupAB_Smooth();
    %function a2dMenuIdItem& CmdMenu_GroupAB_CreateRing();
    %function a2dMenuIdItem& CmdMenu_GroupAB_Delete();
    %function a2dMenuIdItem& CmdMenu_GroupAB_Move();
    %function a2dMenuIdItem& CmdMenu_GroupAB_Copy();
    %function a2dMenuIdItem& CmdMenu_GroupAB_ToArcs();
    %function a2dMenuIdItem& CmdMenu_GroupAB_Or();
    %function a2dMenuIdItem& CmdMenu_GroupAB_And();
    %function a2dMenuIdItem& CmdMenu_GroupAB_Exor();
    %function a2dMenuIdItem& CmdMenu_GroupAB_AsubB();
    %function a2dMenuIdItem& CmdMenu_GroupAB_BsubA();
    %function a2dMenuIdItem& CmdMenu_GroupAB_ConvertPolygonToSurface();
    %function a2dMenuIdItem& CmdMenu_GroupAB_ConvertSurfaceToPolygon();

    %function a2dMenuIdItem& CmdMenu_PopTool();
    %function a2dMenuIdItem& CmdMenu_PushTool_Zoom();
    %function a2dMenuIdItem& CmdMenu_PushTool_Select();
    %function a2dMenuIdItem& CmdMenu_PushTool_Select2();
    %function a2dMenuIdItem& CmdMenu_PushTool_RecursiveEdit();
    %function a2dMenuIdItem& CmdMenu_PushTool_MultiEdit();
    %function a2dMenuIdItem& CmdMenu_PushTool_Drag();
    %function a2dMenuIdItem& CmdMenu_PushTool_DragOriginal();
    %function a2dMenuIdItem& CmdMenu_PushTool_RenderImageZoom();
    %function a2dMenuIdItem& CmdMenu_PushTool_Copy();
    %function a2dMenuIdItem& CmdMenu_PushTool_Rotate();
    %function a2dMenuIdItem& CmdMenu_PushTool_Delete();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawText();
    %function a2dMenuIdItem& CmdMenu_PushTool_Image();
    %function a2dMenuIdItem& CmdMenu_PushTool_Image_Embedded();
    %function a2dMenuIdItem& CmdMenu_PushTool_Link();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawRectangle();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawCircle();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawLine();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawLineScaledArrow();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawEllipse();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawEllipticArc();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawEllipticArc_Chord();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawArc();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawArc_Chord();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawPolylineL();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawPolylineL_Splined();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawPolygonL();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawPolygonL_Splined();
    %function a2dMenuIdItem& CmdMenu_PushTool_Property();
    %function a2dMenuIdItem& CmdMenu_PushTool_DragMulti();
    %function a2dMenuIdItem& CmdMenu_PushTool_CopyMulti();
    %function a2dMenuIdItem& CmdMenu_PushTool_Measure();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawWirePolylineL();
    %function a2dMenuIdItem& CmdMenu_PushTool_DrawVPath();
    %function a2dMenuIdItem& CmdMenu_PushTool_Port();
    %function a2dMenuIdItem& CmdMenu_PushTool_Cameleon();
    %function a2dMenuIdItem& CmdMenu_PushTool_Cameleon_BuildIn();
    %function a2dMenuIdItem& CmdMenu_PushTool_CameleonInst();
    %function a2dMenuIdItem& CmdMenu_PushTool_CameleonInstDlg();
    %function a2dMenuIdItem& CmdMenu_PushTool_CameleonRefDlg();
    %function a2dMenuIdItem& CmdMenu_PushTool_CameleonRefDlg_Diagram();
    %function a2dMenuIdItem& CmdMenu_PushTool_CameleonRefDlg_Symbol();

    %function a2dMenuIdItem& CmdMenu_InsertGroupRef();

    %function a2dMenuIdItem& CmdMenu_ShowDlgLayersDrawing();
    %function a2dMenuIdItem& CmdMenu_ShowDlgLayersGlobal();
    %function a2dMenuIdItem& CmdMenu_ShowDlgLayerOrderDrawing();
    %function a2dMenuIdItem& CmdMenu_ShowDlgLayerOrderGlobal();
    %function a2dMenuIdItem& CmdMenu_ShowDlgTools();
    %function a2dMenuIdItem& CmdMenu_ShowDlgStyle();
    %function a2dMenuIdItem& CmdMenu_ShowDlgSnap();
    %function a2dMenuIdItem& CmdMenu_ShowDlgGroups();
    %function a2dMenuIdItem& CmdMenu_ShowDlgIdentify();
    %function a2dMenuIdItem& CmdMenu_ShowDlgCoordEntry();
    %function a2dMenuIdItem& CmdMenu_ShowDlgMeasure();
    %function a2dMenuIdItem& CmdMenu_ShowDlgStructure();
    %function a2dMenuIdItem& CmdMenu_ShowDlgStructureModal();
    %function a2dMenuIdItem& CmdMenu_ShowDlgCameleonModal();
    %function a2dMenuIdItem& CmdMenu_ShowDlgPathSettings();
    %function a2dMenuIdItem& CmdMenu_ShowDlgSettings();
    %function a2dMenuIdItem& CmdMenu_ShowDlgTrans();
    %function a2dMenuIdItem& CmdMenu_ShowDlgPropEdit();

    %function a2dMenuIdItem& CmdMenu_SetFont();
 
    %function a2dMenuIdItem& CmdMenu_SetMasterTool_SelectFirst();
    %function a2dMenuIdItem& CmdMenu_SetMasterTool_ZoomFirst();
    %function a2dMenuIdItem& CmdMenu_SetMasterTool_TagGroups();
    %function a2dMenuIdItem& CmdMenu_SetMasterTool_TagGroups_WireMode();

    %function a2dMenuIdItem& CmdMenu_LineBegin();
    %function a2dMenuIdItem& CmdMenu_LineEnd();
    %function a2dMenuIdItem& CmdMenu_LineScale();

    %function a2dMenuIdItem& CmdMenu_SetLayerDlg();
    %function a2dMenuIdItem& CmdMenu_SetTargetDlg();
    %function a2dMenuIdItem& CmdMenu_SetLayerDlgModeless();
    %function a2dMenuIdItem& CmdMenu_SetBackgroundStyle();

//    %function a2dMenuIdItem& CmdMenu_ViewAsImage();
//    %function a2dMenuIdItem& CmdMenu_ViewAsImage_Png();
//    %function a2dMenuIdItem& CmdMenu_ViewAsImage_Bmp();
//    %function a2dMenuIdItem& CmdMenu_ViewAsSvg();

    %function a2dMenuIdItem& CmdMenu_Selected_MoveXY();
    %function a2dMenuIdItem& CmdMenu_Selected_MoveLayer();
    %function a2dMenuIdItem& CmdMenu_Selected_CopyXY();
    %function a2dMenuIdItem& CmdMenu_Selected_CopyLayer();
    %function a2dMenuIdItem& CmdMenu_Selected_Transform();  
    	
    %function a2dMenuIdItem& CmdMenu_GdsIoSaveTextAsPolygon();
    %function a2dMenuIdItem& CmdMenu_KeyIoSaveTextAsPolygon();
    %function a2dMenuIdItem& CmdMenu_SetGdsIoSaveFromView();
    %function a2dMenuIdItem& CmdMenu_SetKeyIoSaveFromView();

    %function a2dMenuIdItem& CmdMenu_SaveLayers();
    %function a2dMenuIdItem& CmdMenu_SaveLayersHome();
    %function a2dMenuIdItem& CmdMenu_SaveLayersLastDir();
    %function a2dMenuIdItem& CmdMenu_SaveLayersCurrentDir();
    %function a2dMenuIdItem& CmdMenu_LoadLayers();
    %function a2dMenuIdItem& CmdMenu_LoadLayersHome();
    %function a2dMenuIdItem& CmdMenu_LoadLayersLastDir();
    %function a2dMenuIdItem& CmdMenu_LoadLayersCurrentDir();

    %function a2dMenuIdItem& CmdMenu_SetRelativeStart();
    %function a2dMenuIdItem& CmdMenu_AddMeasurement();
    %function a2dMenuIdItem& CmdMenu_ClearMeasurements();

    %function a2dMenuIdItem& CmdMenu_GdsIoSaveFromView();
    %function a2dMenuIdItem& CmdMenu_KeyIoSaveFromView();
    %function a2dMenuIdItem& CmdMenu_SetLayersProperty();

    %function a2dMenuIdItem& CmdMenu_ShowDlgLayers();
    %function a2dMenuIdItem& CmdMenu_ShowDlgStructureDocument();
    %function a2dMenuIdItem& CmdMenu_ShowDlgStructureDocumentModal();
    %function a2dMenuIdItem& CmdMenu_ShowDlgLayersDocument();
    %function a2dMenuIdItem& CmdMenu_ShowDlgLayerOrderDocument();

    %function a2dMenuIdItem& CmdMenu_ViewAsImage();
    %function a2dMenuIdItem& CmdMenu_ViewAsImage_Png();
    %function a2dMenuIdItem& CmdMenu_ViewAsImage_Bmp();
    %function a2dMenuIdItem& CmdMenu_ViewAsSvg();

    %function a2dMenuIdItem& CmdMenu_DocumentAsImage_Png();
    %function a2dMenuIdItem& CmdMenu_DocumentAsImage_Bmp();
        
        
class %delete a2dMenuIdItem
{
        
    %member static const a2dMenuIdItem sm_noCmdMenuId;   
    a2dMenuIdItem( const wxString& menuIdName, const wxString& text, const wxString& help, wxItemKind kind = wxITEM_NORMAL )
    
    void SetId(int itemid);
    int  GetId() const;
    void SetText(const wxString& str);
    wxString GetLabel() const;
    const wxString& GetText() const;
    static wxString GetLabelText(const wxString& text);
    wxItemKind GetKind() const;
    void SetKind(wxItemKind kind);
    virtual void SetCheckable(bool checkable);
    bool IsCheckable() const;
    void Enable(bool enable = true);
    bool IsEnabled() const;
    void Check(bool check = true);
    bool IsChecked() const;
    void Toggle();
    void SetHelp(const wxString& str);
    const wxString& GetHelp() const;
    wxAcceleratorEntry *GetAccel() const;
    void SetAccel(wxAcceleratorEntry *accel);  
    
    wxString GetIdName() const;
	static const a2dMenuIdItem& GetItemByName( const wxString& menuIdName );
};

