
// *****************************************************************
// wx/editor\canedit.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dCanvasView
// ---------------------------------------------------------------------------

class a2dCanvasView : public a2dView
{

    a2dCanvasView( int width = 1000, int height = 1000 );
    a2dCanvasView( const wxSize& size );
    a2dCanvasView( const a2dCanvasView& other );

    virtual void SetDisplayWindow( wxWindow* display );

	a2dDrawingPart* GetDrawingPart() const;

    a2dCanvasDocument* GetCanvasDocument() const;

    void Update( unsigned int how, wxObject* hintObject = NULL );

    void SetPrintTitle( bool val ) ;

    void SetPrintFilename( bool val ) ;
    void SetPrintScaleLimit( double val ) ;
    void SetPrintFrame( bool val ); 
    void SetPrintFitToPage( bool val ); 
};

// ---------------------------------------------------------------------------
// a2dEditorMultiFrameViewConnector
// ---------------------------------------------------------------------------

#include "wx/editor/canedit.h"
class a2dEditorMultiFrameViewConnector : public a2dFrameViewConnector
{
    a2dEditorMultiFrameViewConnector( wxClassInfo *EditorClassInfo = NULL )
    void OnPostCreateDocument( a2dTemplateEvent& event )
    void OnPostCreateView( a2dTemplateEvent& event )
    %member wxClassInfo*      m_editorClassInfo
};

// ---------------------------------------------------------------------------
// a2dEditorFrameEvent
// ---------------------------------------------------------------------------

#include "wx/editor/canedit.h"
class %delete a2dEditorFrameEvent : public wxEvent
{
    %wxEventType a2dEVT_THEME_EVENT             
    %wxEventType a2dEVT_INIT_EVENT             

    a2dEditorFrameEvent( a2dEditorFrame* editorFrame, wxEventType type, int id = 0 )
    a2dEditorFrame* GetEditorFrame() const
    wxEvent* Clone(void) const
};

// ---------------------------------------------------------------------------
// a2dEditorFrame
// ---------------------------------------------------------------------------

#include "wx/editor/canedit.h"
class a2dEditorFrame : public a2dDocumentFrame
{
    a2dEditorFrame()
    a2dEditorFrame( bool isParent, wxFrame* parent,const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE)
    bool Create( bool isParent, wxFrame* parent,const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE)
    virtual void Init()
    virtual void Theme( const wxString& themeName = "default" )
    virtual void CreateStatusBarThemed( const wxString& themeName = "default" )
    wxMenu* GetEditMenu()
    void Update()
    void ConnectDocument( a2dCanvasDocument* doc, a2dCanvasView* drawer = NULL  )
    a2dStToolContr* GetToolController()
    void SetToolDragingMode( wxCommandEvent& event )
    void SetToolDrawingMode( wxCommandEvent& event )
    void OnUndoEvent( a2dCommandProcessorEvent& event )
    void OnActivate(wxActivateEvent& event)
    void OnSetmenuStrings( a2dCommandProcessorEvent& event )
    void OnCloseWindow(wxCloseEvent& event)
    void OnComEvent( a2dComEvent& event )
    void OnUndo(wxCommandEvent& event)
    void OnRedo(wxCommandEvent& event)
    void Quit(wxCommandEvent& event)
    void OnAbout( wxCommandEvent &event )
    void ShowLibs(wxCommandEvent &event)
    //void PlaceFromLibByRef()
    //void PlaceFromLibByCopy()
    void OnMenu(wxCommandEvent &event)
    void CreateThemeTest()
    void CreateThemeDefault()
};

// *****************************************************************
// wx/editor\canpropedit.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dPropertyEditorDlg
// ---------------------------------------------------------------------------

#include "wx/editor/canpropedit.h"
class a2dPropertyEditorDlg : public wxDialog
{
    a2dPropertyEditorDlg( wxFrame* parent, a2dNamedPropertyList* propertylist )
    void CmOk( wxCommandEvent& )
    void CmCancel( wxCommandEvent& )
    void OnCloseWindow(wxCloseEvent& event)
};

// *****************************************************************
// wx/editor\doccancom.h
// *****************************************************************

// *****************************************************************
// wx/canvas\edit.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dRecursiveEditTool
// ---------------------------------------------------------------------------

#include "wx/canvas/edit.h"
class a2dRecursiveEditTool : public a2dObjectEditTool
{
    %member static const a2dCommandId COMID_PushTool_RecursiveEdit
    a2dRecursiveEditTool( a2dStToolContr* controller, int editmode = 1, bool SingleClickToEnd = true )
    a2dRecursiveEditTool( a2dStToolContr* controller, a2dIterC& ic, int editmode = 1, bool SingleClickToEnd = true )
};

// ---------------------------------------------------------------------------
// a2dObjectEditTool
// ---------------------------------------------------------------------------

#include "wx/canvas/edit.h"
class a2dObjectEditTool : public a2dStTool
{
    %member static const a2dCommandId COMID_PushTool_ObjectEdit
    %member const a2dSignal sig_toolStartEditObject
    a2dObjectEditTool( a2dStToolContr* controller, int editmode = 1, bool SingleClickToEnd = true )
    a2dObjectEditTool( a2dStToolContr* controller, a2dIterC& ic, int editmode = 1, bool SingleClickToEnd = true )
    bool SetContext( a2dIterC& ic, a2dCanvasObject* startObject )
    void StartToEdit( a2dCanvasObject* startObject )
    bool ZoomSave()
    void SetMode( int mode )
    void SetActive(bool active)
    void SetSingleClickToEnd( bool SingleClickToEnd )
    void SetDisableOtherViews( bool disableOtherViews )
    void Render()
};

// ---------------------------------------------------------------------------
// a2dMultiEditTool
// ---------------------------------------------------------------------------

#include "wx/canvas/edit.h"
class a2dMultiEditTool : public a2dObjectEditTool
{
    %member static const a2dCommandId COMID_PushTool_MultiEdit
    a2dMultiEditTool(a2dStToolContr* controller )
    bool ZoomSave()
    void OnChar(wxKeyEvent& event)
    bool StartEditing( a2dCanvasObject* startobject )
    bool StartEditingSelected()
    void SetActive(bool active)
    bool AddToGroup( a2dCanvasObject* canvasobject )
    bool RemoveFromGroup( a2dCanvasObject* canvasobject, int index = 0 )
};

// *****************************************************************
// wx/editor\editmod.h
// *****************************************************************

// *****************************************************************
// wx/editor\execdlg.h
// *****************************************************************
// ---------------------------------------------------------------------------
// CopywxListBox
// ---------------------------------------------------------------------------

#include "wx/canvas/execdlg.h"
class CopywxListBox : public wxListBox
{
    //CopywxListBox(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size,int n,const wxString choices[],long style)
    void OnMouseRightDown( wxMouseEvent& event )
    void CopySel( wxCommandEvent &event )
    void CopyAll( wxCommandEvent &event )
};

// ---------------------------------------------------------------------------
// a2dCoordinateEntry
// ---------------------------------------------------------------------------

#include "wx/canvas/execdlg.h"
class a2dCoordinateEntry : public wxDialog
{
    a2dCoordinateEntry( )
    //a2dCoordinateEntry( wxWindow* parent, wxWindowID id = SYMBOL_A2DCOORDINATEENTRY_IDNAME, bool modal = false, const wxString& caption = SYMBOL_A2DCOORDINATEENTRY_TITLE, const wxPoint& pos = SYMBOL_A2DCOORDINATEENTRY_POSITION, const wxSize& size = SYMBOL_A2DCOORDINATEENTRY_SIZE, long style = SYMBOL_A2DCOORDINATEENTRY_STYLE )
    //bool Create( wxWindow* parent, wxWindowID id = SYMBOL_A2DCOORDINATEENTRY_IDNAME, const wxString& caption = SYMBOL_A2DCOORDINATEENTRY_TITLE, const wxPoint& pos = SYMBOL_A2DCOORDINATEENTRY_POSITION, const wxSize& size = SYMBOL_A2DCOORDINATEENTRY_SIZE, long style = SYMBOL_A2DCOORDINATEENTRY_STYLE )
    void OnEntryXEnter( wxCommandEvent& event )
    void OnEntryYEnter( wxCommandEvent& event )
    void OnEntryCalccoordEnter( wxCommandEvent& event )
    void OnEntryCartpolarSelected( wxCommandEvent& event )
    void OnEntryAbsrelSelected( wxCommandEvent& event )
    void OnEntrySnapClick( wxCommandEvent& event )
    void OnEntryHideClick( wxCommandEvent& event )
    void OnEntryApplyClick( wxCommandEvent& event )
    void OnEntryCancelClick( wxCommandEvent& event )
    wxBitmap GetBitmapResource( const wxString& name )
    wxIcon GetIconResource( const wxString& name )
    static bool ShowToolTips()
    void CalcCoordinate()
};

// ---------------------------------------------------------------------------
// a2dFontSearchPathDialog
// ---------------------------------------------------------------------------

#include "wx/canvas/fontdlg.h"
class a2dFontSearchPathDialog : public wxDialog
{
    a2dFontSearchPathDialog( wxWindow *parent = NULL )
};

// *****************************************************************
// wx/editor\groupdlg.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dGroupDlg
// ---------------------------------------------------------------------------

#include "wx/canvas/groupdlg.h"
class a2dGroupDlg : public wxDialog
{
    a2dGroupDlg( a2dHabitat* habitat, wxFrame* parent, const wxString& title, long style, const wxString& name = "frame" )
    void Init()
    void OnCloseWindow(wxCloseEvent& event)
};

// *****************************************************************
// wx/editor\identifydlg.h
// *****************************************************************

// ---------------------------------------------------------------------------
// IdentifyDialog
// ---------------------------------------------------------------------------

#include "wx/canvas/identifydlg.h"
class IdentifyDialog : public wxDialog
{
    IdentifyDialog( )
    //IdentifyDialog( wxWindow* parent, wxWindowID id = SYMBOL_IDENTIFYDIALOG_IDNAME, const wxString& caption = SYMBOL_IDENTIFYDIALOG_TITLE, const wxPoint& pos = SYMBOL_IDENTIFYDIALOG_POSITION, const wxSize& size = SYMBOL_IDENTIFYDIALOG_SIZE, long style = SYMBOL_IDENTIFYDIALOG_STYLE )
    //bool Create( wxWindow* parent, wxWindowID id = SYMBOL_IDENTIFYDIALOG_IDNAME, const wxString& caption = SYMBOL_IDENTIFYDIALOG_TITLE, const wxPoint& pos = SYMBOL_IDENTIFYDIALOG_POSITION, const wxSize& size = SYMBOL_IDENTIFYDIALOG_SIZE, long style = SYMBOL_IDENTIFYDIALOG_STYLE )
    void CreateControls()
    void Init( a2dCanvasObject* showObject )
};

// *****************************************************************
// wx/editor\layerdlg.h
// *****************************************************************

// ---------------------------------------------------------------------------
// LayerPropertiesDialog
// ---------------------------------------------------------------------------

#include "wx/canvas/layerdlg.h"
class LayerPropertiesDialog : public wxDialog
{
    LayerPropertiesDialog( wxFrame* parent, a2dLayers* layersetup, bool activeDrawingPart = false, const wxString& title = "LayerProperties",long style = wxDEFAULT_DIALOG_STYLE  ,const wxString& name = "Layer_Properties_Dialog" )
    void FillControls()
    void Init( a2dLayers* layersetup )
    void CmHide(wxCommandEvent &)
    void CmSort(wxCommandEvent &)
    void CmCancel(wxCommandEvent &)
    void CmRestore(wxCommandEvent &)
    void CmSaveToFile(wxCommandEvent& event)
    void CmLoadFromFile( wxCommandEvent& event )
    void CmVisible(wxCommandEvent &)
    void CmRead(wxCommandEvent &)
    void CmOutLine(wxCommandEvent &)
    void CmSelect(wxCommandEvent &)
};

// *****************************************************************
// wx/canvas/mastertool.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dMasterDrawBase
// ---------------------------------------------------------------------------

#include "wx/canvas/mastertool.h"
class a2dMasterDrawBase : public a2dStTool
{
    a2dMasterDrawBase(a2dStToolContr* controller)
    void SetAllowMultiEdit( bool onOff )
    bool GetAllowMultiEdit()
    void SetSelectFill( const a2dFill& fill)
    const a2dFill& GetSelectFill() const 
    void SetSelectStroke( const a2dStroke& stroke)
    const a2dStroke& GetSelectStroke() const
	void SetWiringMode( a2dWiringMode wiringMode )
	
};

// ---------------------------------------------------------------------------
// a2dMasterDrawZoomFirst
// ---------------------------------------------------------------------------

#include "wx/canvas/mastertool.h"
class a2dMasterDrawZoomFirst : public a2dMasterDrawBase
{
    a2dMasterDrawZoomFirst(a2dStToolContr* controller)
};

// ---------------------------------------------------------------------------
// a2dMasterDrawSelectFirst
// ---------------------------------------------------------------------------

#include "wx/canvas/mastertool.h"
class a2dMasterDrawSelectFirst : public a2dMasterDrawBase
{
    a2dMasterDrawSelectFirst(a2dStToolContr* controller)
};

// ---------------------------------------------------------------------------
// a2dMasterTagGroups3
// ---------------------------------------------------------------------------

#include "wx/canvas/mastertool.h"
class a2dMasterTagGroups3 : public a2dMasterDrawBase
{
    a2dMasterTagGroups3(a2dStToolContr* controller)
};

// ---------------------------------------------------------------------------
// a2dGraphicsMasterTool
// ---------------------------------------------------------------------------

#include "wx/canvas/mastertool.h"
class a2dGraphicsMasterTool : public a2dStTool
{
    a2dGraphicsMasterTool(a2dStToolContr* controller)
};

// *****************************************************************
// wx/editor\orderdlg.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dLayerOrderDialog
// ---------------------------------------------------------------------------

#include "wx/editor/orderdlg.h"
class a2dLayerOrderDialog : public wxDialog
{
    a2dLayerOrderDialog( wxWindow* parent, a2dLayers* layersetup, const wxString& title, long style = 0, const wxString& name = "layer_order_dlg" )
    void Init( a2dLayers* layersetup )
    void StoreOrder()
};

// *****************************************************************
// wx/editor\pathsettings.h
// *****************************************************************

#define ID_a2dPathSettings 10000

// ---------------------------------------------------------------------------
// a2dPathSettings
// ---------------------------------------------------------------------------

#include "wx/canvas/pathsettings.h"
class a2dPathSettings : public wxDialog
{
    a2dPathSettings()
    a2dPathSettings( a2dHabitat* habitat, wxWindow* parent, wxWindowID id = -1, const wxString& caption = "a2dPathSettings", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    bool Create( wxWindow* parent, wxWindowID id = -1, const wxString& caption = "a2dPathSettings", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    void Init()
    void CreateControls()
    static bool ShowToolTips()
};

// *****************************************************************
// wx/editor\settingsdlg.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dNewDocumentSize
// ---------------------------------------------------------------------------

#include "wx/canvas/settingsdlg.h"
class a2dNewDocumentSize : public wxDialog
{
    a2dNewDocumentSize()
    a2dNewDocumentSize( wxWindow* parent, wxWindowID id = -1, const wxString& caption = "New Document Size", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    bool Create( wxWindow* parent, wxWindowID id = -1, const wxString& caption = "New Document Size", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    void Init()
    void CreateControls()
    static bool ShowToolTips()
    double GetSizeX() const
    double GetSizeY() const
    wxString GetUnit() const
};

// ---------------------------------------------------------------------------
// a2dSettings
// ---------------------------------------------------------------------------

#include "wx/canvas/settingsdlg.h"
class a2dSettings : public wxDialog
{
    a2dSettings()
    a2dSettings( a2dHabitat* habitat, wxWindow* parent, wxWindowID id = -1, const wxString& caption = "Settings", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    bool Create( wxWindow* parent, wxWindowID id = -1, const wxString& caption = "Settings", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    void Init()
    bool Show( bool show = true )
    void UpdateEntries()
    void CreateControls()
    wxBitmap GetBitmapResource( const wxString& name )
    wxIcon GetIconResource( const wxString& name )
    static bool ShowToolTips()
};

// *****************************************************************
// wx/editor\snap.h
// *****************************************************************

#define wxCLOSE_BOX 0x1000
#define wxFIXED_MINSIZE 0
#define ID_SNAP 10011

// ---------------------------------------------------------------------------
// a2dSnapSettings
// ---------------------------------------------------------------------------

#include "wx/canvas/snap.h"
class a2dSnapSettings : public wxDialog
{
    a2dSnapSettings( )
    a2dSnapSettings( a2dHabitat* habitat, wxWindow* parent, wxWindowID id = SYMBOL_A2DSNAPSETTINGS_IDNAME, const wxString& caption = "SnapSettings", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_A2DSNAPSETTINGS_IDNAME, const wxString& caption = "SnapSettings", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    void CreateControls()
    wxBitmap GetBitmapResource( const wxString& name )
    wxIcon GetIconResource( const wxString& name )
    static bool ShowToolTips()
};

// *****************************************************************
// wx/editor\strucdlg.h
// *****************************************************************

#define NR_PATTERNS 72

// ---------------------------------------------------------------------------
// a2dCanvasObjectsDialog
// ---------------------------------------------------------------------------

#include "wx/canvas/strucdlg.h"
class a2dCanvasObjectsDialog : public wxDialog
{
    a2dCanvasObjectsDialog(wxWindow* parent, a2dDrawing* root, bool modal, bool structOnly, long style )
    a2dCanvasObjectsDialog(wxWindow* parent, a2dCanvasObjectList* total, bool modal, long style)
    
    a2dCanvasObject*  GetCanvasObject()
};

// ---------------------------------------------------------------------------
// LogicalFunction
// ---------------------------------------------------------------------------

#include "wx/canvas/strucdlg.h"
class LogicalFunction : public wxDialog
{
    LogicalFunction(wxFrame* parent)
    int GetLogicalFunction()
};

// ---------------------------------------------------------------------------
// PatternDialog
// ---------------------------------------------------------------------------

#include "wx/canvas/strucdlg.h"
class PatternDialog : public wxDialog
{
    PatternDialog(wxWindow* parent,FillPatterns* fills, const wxString& title,long style =0 , const wxString& name = "main_pattern_dlg" )
};

// ---------------------------------------------------------------------------
// CanvasFillDialog
// ---------------------------------------------------------------------------

#include "wx/canvas/strucdlg.h"
class CanvasFillDialog : public wxDialog
{
    CanvasFillDialog(wxFrame* parent)
    a2dFillStyle GetStyle()
};

// ---------------------------------------------------------------------------
// PenDialog
// ---------------------------------------------------------------------------

#include "wx/canvas/strucdlg.h"
class PenDialog : public wxDialog
{
    PenDialog(wxFrame* parent)
    a2dStrokeStyle GetStyle()
};

// ---------------------------------------------------------------------------
// FillPatterns
// ---------------------------------------------------------------------------

#include "wx/canvas/strucdlg.h"
class FillPatterns
{
    FillPatterns()
    wxBitmap*      GetPattern(short patternnr)
};

// *****************************************************************
// wx/editor\sttool.h
// *****************************************************************

enum wxTC_BEHAVIOR
{
    wxTC_DefaultBehavior,
    wxTC_NoDefaultKeys,
    wxTC_NoContextMenu,
    wxTC_NoDefaultMouseActions,
    wxTC_NoAll
};

enum a2dWiringMode
{
    a2d_BasedOnClassStartPin,
    a2d_BasedOnWireClassRequired,
    a2d_BasedOnObjectClassRequired,
    a2d_BasedOnClassEndPin
};

enum a2dStTool::a2dOpaqueMode
{
    a2dOpaqueMode_Off,
    a2dOpaqueMode_Tool,
    a2dOpaqueMode_Tool_FixedStyle,
    a2dOpaqueMode_Controller
};

enum DRAGMODE
{
    wxDRAW_RECTANGLE,
    wxDRAW_ONTOP,
    wxDRAW_REDRAW,
    wxDRAW_COPY
};

enum a2dStToolEvtHandler::options
{
    a2dTakeToolstyleNon,
    a2dTakeToolstyleToCmdhOnPush,
    a2dTakeToolstyleFromCmdhOnPop,
    a2dTakeToolstyleFromCmdhOnPush,
    a2dTakeToolstyleToNewObject,
    a2d_ALL
};

// ---------------------------------------------------------------------------
// a2dStTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dStTool : public a2dBaseTool
{
    %member const a2dSignal sig_toolComEventAddObject
    %member const a2dSignal sig_toolComEventSetEditObject
    virtual bool OnPushNewTool( a2dBaseTool* newtool )
    virtual void SetActive( bool active = true )
    virtual bool EnterBusyMode()
    virtual void FinishBusyMode( bool closeCommandGroup = true )
    virtual void AbortBusyMode()
    void AddCurrent( a2dCanvasObject* objectToAdd )
    void Render()
    void RenderAnotation()
    a2dStToolContr *GetStToolContr()
    a2dObjectEditTool* StartEditTool( a2dCanvasObject *objecttoedit )
    void SetEditAtEnd( bool editatend )
    bool GetEditAtEnd()
    void SetOpacityFactorEditcopy( wxUint8 editOpacity )
    // void SetUseOpaqueEditcopy( a2dOpaqueMode editOpaque )
    void SetConnectionGenerator( a2dConnectionGenerator *connectionGenerator )
    a2dConnectionGenerator* GetConnectionGenerator() const
    a2dCanvasObject* GetOriginal()
    bool SetContext( a2dIterC& ic, a2dCanvasObject* startObject = NULL )
    void SetStateString( const wxString& message = wxEmptyString, size_t field = 0 )
};

// ---------------------------------------------------------------------------
// a2dDrawArcTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDrawArcTool : public a2dStDrawTool
{
    %member static const a2dCommandId COMID_PushTool_DrawArc
    %member static const a2dCommandId COMID_PushTool_DrawArc_Chord
    a2dDrawArcTool(a2dStToolContr* controller)
    bool ZoomSave()
    void SetMode( int mode )
    void SetMode2( int mode )
    void SetChord( bool chord )
};

// ---------------------------------------------------------------------------
// a2dDrawEllipticArcTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDrawEllipticArcTool : public a2dStDrawTool
{
    %member static const a2dCommandId COMID_PushTool_DrawEllipticArc
    %member static const a2dCommandId COMID_PushTool_DrawEllipticArc_Chord
    a2dDrawEllipticArcTool(a2dStToolContr* controller, a2dEllipticArc* templateObject = NULL )
    bool ZoomSave()
    void SetMode( int mode )
    void SetMode2( int mode )
    void SetChord( bool chord )
};

// ---------------------------------------------------------------------------
// a2dStDrawTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dStDrawTool : public a2dStTool
{
};

// ---------------------------------------------------------------------------
// a2dDeleteTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDeleteTool : public a2dStTool
{
    %member static const a2dCommandId COMID_PushTool_Delete
    a2dDeleteTool(a2dStToolContr* controller, a2dCanvasObjectFlagsMask whichobjects =a2dCanvasOFlags::ALL )
    bool ZoomSave()
    void SetWhichObjectsFlagMask( a2dCanvasObjectFlagsMask whichobjects )
};

// ---------------------------------------------------------------------------
// a2dCopyTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dCopyTool : public a2dDragTool
{
    %member static const a2dCommandId COMID_PushTool_Copy
    a2dCopyTool(a2dStToolContr* controller)
    bool ZoomSave()
};

// ---------------------------------------------------------------------------
// a2dDrawRectangleTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDrawRectangleTool : public a2dStDrawTool
{
    %member static const a2dCommandId COMID_PushTool_DrawRectangle
    a2dDrawRectangleTool(a2dStToolContr* controller)
    bool ZoomSave()
    void SetRadius(double radius)
};

// ---------------------------------------------------------------------------
// a2dImageTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dImageTool : public a2dStDrawTool
{
    %member static const a2dCommandId COMID_PushTool_Image
    a2dImageTool(a2dStToolContr* controller)
    bool ZoomSave()
    void SetDrawPatternOnTop(bool drawPatternOnTop)
    bool GetDrawPatternOnTop()
};

// ---------------------------------------------------------------------------
// a2dDrawPolylineLTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDrawPolylineLTool : public a2dDrawPolygonLTool
{
    %member static const a2dCommandId COMID_PushTool_DrawPolylineL
    %member static const a2dCommandId COMID_PushTool_DrawPolylineL_Splined
    a2dDrawPolylineLTool(a2dStToolContr* controller, a2dPolylineL* templateObject = NULL )
    void SetLineBegin(a2dCanvasObject* begin)
    void SetLineEnd(a2dCanvasObject* end)
    a2dCanvasObject* GetLineBegin()
    a2dCanvasObject* GetLineEnd()
    void SetEndScaleX(double xs)
    void SetEndScaleY(double ys)
    double GetEndScaleX()
    double GetEndScaleY()
    void SetPathType( a2dPATH_END_TYPE pathtype )
    a2dPATH_END_TYPE GetPathType()
};

// ---------------------------------------------------------------------------
// a2dSelectTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dSelectTool : public a2dStTool
{
    %member static const a2dCommandId COMID_PushTool_Select
    a2dSelectTool(a2dStToolContr* controller)
    bool ZoomSave()
    void SetShiftIsAdd()
};

// ---------------------------------------------------------------------------
// a2dDragOrgTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDragOrgTool : public a2dStTool
{
    %member static const a2dCommandId COMID_PushTool_DragOrg
    a2dDragOrgTool(a2dStToolContr* controller)
    bool ZoomSave()
    void SetDeleteOnOutsideDrop( bool val )
    void CaptureMouse()
    void ReleaseMouse()
    void Render()
    virtual void AdjustShift( double *x, double *y )
};

// ---------------------------------------------------------------------------
// a2dDrawLineTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDrawLineTool : public a2dStDrawTool
{
    %member static const a2dCommandId COMID_PushTool_DrawLine
    a2dDrawLineTool(a2dStToolContr* controller)
    bool ZoomSave()
    void SetLineBegin(a2dCanvasObject* begin)
    void SetLineEnd(a2dCanvasObject* end)
    a2dCanvasObject* GetLineBegin()
    a2dCanvasObject* GetLineEnd()
    void SetEndScaleX(double xs)
    void SetEndScaleY(double ys)
    double GetEndScaleX()
    double GetEndScaleY()
    //void SetPathType( a2dPATH_END_TYPE pathtype )
    a2dPATH_END_TYPE GetPathType()
};

// ---------------------------------------------------------------------------
// a2dCopyMultiTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dCopyMultiTool : public a2dDragMultiTool
{
    %member static const a2dCommandId COMID_PushTool_CopyMulti
    a2dCopyMultiTool(a2dStToolContr* controller)
};

// ---------------------------------------------------------------------------
// a2dDrawTextTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDrawTextTool : public a2dStDrawTool
{
    %member static const a2dCommandId COMID_PushTool_DrawText
    a2dDrawTextTool( a2dStToolContr* controller, a2dText* templateObject = NULL );
    
    a2dText* GetTemplateObject() const; 
    void SetTemplateObject( a2dText* templateobject );   
    bool ZoomSave()
};

// ---------------------------------------------------------------------------
// a2dDrawCircleTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDrawCircleTool : public a2dStDrawTool
{
    %member static const a2dCommandId COMID_PushTool_DrawCircle
    a2dDrawCircleTool(a2dStToolContr* controller)
    bool ZoomSave()
    void SetMode( int mode )
};

// ---------------------------------------------------------------------------
// a2dDragNewTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDragNewTool : public a2dDragTool
{
    %member static const a2dCommandId COMID_PushTool_DragNew
    a2dDragNewTool( a2dStToolContr* controller, a2dCanvasObject *newObject)
    bool ZoomSave()
    void OnMouseEvent(wxMouseEvent& event)
    virtual void AdjustRenderOptions()
};

// ---------------------------------------------------------------------------
// a2dZoomTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dZoomTool : public a2dStTool
{
    %member static const a2dCommandId COMID_PushTool_Zoom
    a2dZoomTool(a2dStToolContr* controller)
    a2dZoomTool( const a2dZoomTool& other, a2dObject::CloneOptions options, a2dRefMap* refs )
    bool ZoomSave()
};

// ---------------------------------------------------------------------------
// a2dStToolFixedToolStyleEvtHandler
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dStToolFixedToolStyleEvtHandler : public a2dToolEvtHandler
{
    a2dStToolFixedToolStyleEvtHandler( a2dStToolContr* controller )
    void SetTakeToolstyleToCmdhOnPush( bool set )
    bool GetTakeToolstyleToCmdhOnPush()
};

// ---------------------------------------------------------------------------
// a2dDragTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDragTool : public a2dStTool
{
    %member static const a2dCommandId COMID_PushTool_Drag
    a2dDragTool(a2dStToolContr* controller)
    bool ZoomSave()
    void SetDeleteOnOutsideDrop( bool val )
    void CaptureMouse()
    void ReleaseMouse()
    void Render()
    virtual void AdjustShift( double *x, double *y )
};

// ---------------------------------------------------------------------------
// a2dStToolEvtHandler
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dStToolEvtHandler : public a2dToolEvtHandler
{
    a2dStToolEvtHandler( a2dStToolContr* controller )
    void SetTakeToolstyleToCmdhOptions( wxUint32 set )
    wxUint32 GetTakeToolstyleToCmdhOptions()
};

// ---------------------------------------------------------------------------
// a2dStToolContr
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dStToolContr : public a2dToolContr
{
    a2dStToolContr(a2dDrawingPart* drawingPart,wxFrame* where)
    virtual void ReStart()
    wxMenu* GetMousePopupMenu()
    void SetMousePopupMenu( wxMenu* mousemenu )
    bool PushTool(a2dBaseTool* tool)
    virtual bool PopTool( a2dBaseToolPtr& poped, bool force = true )
    void SetZoomoutCentered(bool center )
    bool GetZoomoutCentered()
    void SetZoomFirst( bool zoomfirst )
    void SetTopTool( a2dBaseTool* toolOnTop )
    void SetZoomTool( a2dBaseTool* toolForZoom )
    void SetDragMode(DRAGMODE mode)
    DRAGMODE GetDragMode()
    void SetDrawMode( a2dDrawStyle drawstyle )
    a2dDrawStyle GetDrawMode()
    void SetDefaultBehavior( unsigned int behavior )
    void Undo( wxCommandEvent& )
    void Redo( wxCommandEvent& )
    void EndTool( wxCommandEvent& )
    void EditProperties( wxCommandEvent& )
    void OnSetmenuStrings( a2dCommandProcessorEvent& event )
    void ZoomWindow()
    void ZoomPan()
    void ZoomUndo()
    void Zoomout()
    void Zoomout2()
    void Zoomin2()
    void Zoom( double x, double y, double uppx, double uppy )
    void ZoomOut2AtMouse()
    void ZoomIn2AtMouse()
    // wxZoomList& GetZoomList()
    void SetStateString( const wxString& message = wxEmptyString, size_t field = 0 );
    wxPoint GetMousePosition()
    wxFrame* GetFrame()
    virtual bool StartEditingObject( a2dCanvasObject* objectToEdit )
    virtual bool StartEditingObject( a2dCanvasObject* objectToEdit, a2dIterC& ic )
    void SetOpacityFactorEditcopy( wxUint8 editOpacity )
    wxUint8 GetOpacityFactorEditcopy()
    void SetUseOpaqueEditcopy( bool editOpaque )
    bool GetUseOpaqueEditcopy()
    %member static const a2dPropertyIdBool PROPID_zoomfirst
    %member static const wxEventType sm_showCursor; 
};

// ---------------------------------------------------------------------------
// a2dDrawWirePolylineLTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDrawWirePolylineLTool : public a2dDrawPolylineLTool
{
    %member static const a2dCommandId COMID_PushTool_DrawWirePolylineL
    a2dDrawWirePolylineLTool(a2dStToolContr* controller )
    //a2dPin *FindPin( a2dCanvasObject *root, double x, double y, bool ForStartPin )
};

// ---------------------------------------------------------------------------
// a2dDragMultiTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDragMultiTool : public a2dStTool
{
    %member static const a2dCommandId COMID_PushTool_DragMulti
    a2dDragMultiTool(a2dStToolContr* controller)
    bool ZoomSave()
};

// ---------------------------------------------------------------------------
// a2dPropertyTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dPropertyTool : public a2dStTool
{
    %member static const a2dCommandId COMID_PushTool_Property
    a2dPropertyTool(a2dStToolContr* controller)
    bool StartEditing( double x, double y )
    bool StartEditing( int x, int y )
    bool ZoomSave()
};

// ---------------------------------------------------------------------------
// a2dDrawEllipseTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDrawEllipseTool : public a2dStDrawTool
{
    %member static const a2dCommandId COMID_PushTool_DrawEllipse
    a2dDrawEllipseTool(a2dStToolContr* controller)
    bool ZoomSave()
    void SetMode( int mode )
};

// ---------------------------------------------------------------------------
// a2dDrawPolygonLTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dDrawPolygonLTool : public a2dStDrawTool
{
    %member static const a2dCommandId COMID_PushTool_DrawPolygonL
    %member static const a2dCommandId COMID_PushTool_DrawPolygonL_Splined
    a2dDrawPolygonLTool(a2dStToolContr* controller, a2dPolygonL* templateObject = NULL )
    bool ZoomSave()
    void SetMode( int mode )
    void SetSpline(bool spline)
    bool GetSpline()
};

// ---------------------------------------------------------------------------
// a2dRotateTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttool.h"
class a2dRotateTool : public a2dStTool
{
    %member static const a2dCommandId COMID_PushTool_Rotate
    a2dRotateTool(a2dStToolContr* controller)
    bool ZoomSave()
    void CaptureMouse()
    void ReleaseMouse()
};

// *****************************************************************
// wx/canextobj/sttoolext.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dRenderImageZoomTool
// ---------------------------------------------------------------------------

#include "wx/canextobj/sttoolext.h"
class a2dRenderImageZoomTool : public a2dStTool
{
    %member static const a2dCommandId COMID_PushTool_RenderImageZoom
    a2dRenderImageZoomTool(a2dStToolContr* controller)
    bool ZoomSave()
};

// *****************************************************************
// wx/editor\sttoolmes.h
// *****************************************************************

#define wxCLOSE_BOX 0x1000
#define wxFIXED_MINSIZE 0

// ---------------------------------------------------------------------------
// a2dMeasureDlg
// ---------------------------------------------------------------------------

#include "wx/canvas/sttoolmes.h"
class a2dMeasureDlg : public wxDialog
{
    a2dMeasureDlg( )
    a2dMeasureDlg( a2dHabitat* habitat, wxWindow* parent, wxWindowID id = -1, const wxString& caption = "Measure", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    bool Create( wxWindow* parent, wxWindowID id = -1, const wxString& caption = "Measure", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    void CreateControls()
    wxBitmap GetBitmapResource( const wxString& name )
    wxIcon GetIconResource( const wxString& name )
    static bool ShowToolTips()
    void AppendMeasure( double x, double y )
};

// ---------------------------------------------------------------------------
// a2dMeasureTool
// ---------------------------------------------------------------------------

#include "wx/canvas/sttoolmes.h"
class a2dMeasureTool : public a2dDrawPolylineLTool
{
    %member static const a2dCommandId COMID_PushTool_Measure
    a2dMeasureTool(a2dStToolContr* controller)
    bool ZoomSave()
};

// *****************************************************************
// wx/editor\styledialg.h
// *****************************************************************


// ---------------------------------------------------------------------------
// a2dStyleDialog
// ---------------------------------------------------------------------------

#include "wx/canvas/styledialg.h"
class a2dStyleDialog : public wxDialog
{
    a2dStyleDialog( a2dHabitat* habitat, wxWindow *parent,long style = wxDEFAULT_DIALOG_STYLE, bool modal = false )
    void SetColor( wxSpinEvent& spinevent )
    void ColorSelect(  wxCommandEvent &event )
    void OnSpinCtrlText(wxCommandEvent& event)
    void OnSliderUpdate( wxScrollEvent &event )
    void OnColourBitmap( wxCommandEvent& event )
    void OnStippleBitmap( wxCommandEvent& event )
    void OnFillStyle( wxCommandEvent &event )
    void OnStrokeStyle( wxCommandEvent &event )
    void OnPixelWidth( wxCommandEvent& event )
    void OnPixelWidthSpin( wxSpinEvent& event )
    void OnPixelStroke( wxCommandEvent& event )
    void OnCloseWindow( wxCloseEvent& event )
    void OnActivate( wxActivateEvent& event)
    void OnIdle( wxIdleEvent& idleEvent )
    void SentStyle( bool fill = true, bool stroke = true )
    a2dFill& GetFill()
    void SetFill( const a2dFill& fill )
    a2dStroke& GetStroke()
    void SetStroke( const a2dStroke& stroke )
};

// *****************************************************************
// wx/canvas/tooldlg.h
// *****************************************************************

%function wxBitmap* GetBitmap( const wxString& name )

// ---------------------------------------------------------------------------
// ToolDlg
// ---------------------------------------------------------------------------

#include "wx/canvas/tooldlg.h"
class ToolDlg : public wxMiniFrame
{
    ToolDlg( wxFrame* parent )
    void AddCmdMenu( wxBitmap& bitmap, const a2dMenuIdItem& cmdId )
    void RemoveCmdMenu( const a2dMenuIdItem& cmdId )
};

// *****************************************************************
// wx/editor\transdlg.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dTransDlg
// ---------------------------------------------------------------------------

#include "wx/canvas/transdlg.h"
class a2dTransDlg : public wxDialog
{
    a2dTransDlg()
    a2dTransDlg( wxWindow* parent, bool modal = false, wxWindowID id = -1, const wxString& caption = "transform", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    bool Create( wxWindow* parent, wxWindowID id = -1, const wxString& caption = "transform", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 )
    void Init()
    void CreateControls()
    void OnHideClick( wxCommandEvent& event )
    void OnApplyClick( wxCommandEvent& event )
    static bool ShowToolTips()
};

// ---------------------------------------------------------------------------
// a2dViewPrintout
// ---------------------------------------------------------------------------

#include "wx/canvas/drawer.h"
class a2dViewPrintout : public a2dDocumentPrintout
{
    a2dViewPrintout( const wxPageSetupDialogData& pageSetupData, a2dCanvasView* drawer,const wxString& title, const wxString& filename, a2dPrintWhat typeOfPrint, bool drawframe, double scalelimit, bool fitToPage )
    bool OnPrintPage(int)
};

// ---------------------------------------------------------------------------
// a2dCentralCanvasCommandProcessor
// ---------------------------------------------------------------------------

#include "wx/editor/cancom.h"
class a2dCentralCanvasCommandProcessor : public a2dDocumentCommandProcessor
{
    a2dCentralCanvasCommandProcessor(long flags = a2dDEFAULT_DOCUMENT_FLAGS,bool initialize = true,int maxCommands = -1 )
    a2dCentralCanvasCommandProcessor( a2dDocumentCommandProcessor* other )
    a2dCanvasDocument* GetCanvasDocument()
	double GetUnitsScaleOfDocument()
    //a2dCanvasObject* AddCurrent( a2dCanvasObject* objectToAdd, bool withUndo = false, a2dPropertyIdList* setStyles = NULL, a2dCanvasObject* parentObject = NULL )
    bool CvgString( const wxString& cvgString )
    bool SetPropertyToObject(const wxString& objectname, const wxString& propertyname, const wxString& value = "" )
    bool SetPropertyToObject( const wxString& objectname, a2dNamedProperty *property )
    bool SetLayerProperty(long layer, const wxString &propertyname, const wxString& value )
    a2dBaseTool* GetActiveTool()
    bool SetShowObject(int index)
    bool Zoom( double x1,double y1, double x2, double y2, bool upp )
    bool Update()
    void SetUndo( bool withUndo )
    bool GetUndo()
    a2dCanvasView* CheckDrawingView() const
    a2dCanvasDocument* CheckCanvasDocument() const
    a2dCanvasCommandProcessor* GetDrawingCmdProcessor()
    
    void FillLibraryPoints()
    a2dCanvasObject* GetLibraryPoints()
   
    virtual void OnExit()
    bool ShowDlg( const a2dCommandId* comID, bool modal = false, bool onTop = true )
    void DeleteDlgs()
    
    void SetBooleanEngineMarge( const a2dDoMu& marge )
    void SetBooleanEngineGrid( long grid )
    void SetBooleanEngineDGrid( double dgrid )
    void SetBooleanEngineCorrectionAber( const a2dDoMu& aber )
    void SetBooleanEngineCorrectionFactor( const a2dDoMu& aber )
    void SetBooleanEngineMaxlinemerge( const a2dDoMu& maxline )
    void SetBooleanEngineWindingRule( bool rule )
    void SetBooleanEngineRoundfactor( double roundfac )
    void SetBooleanEngineSmoothAber( const a2dDoMu& aber )
    void SetBooleanEngineLinkHoles( bool doLinkHoles )
    void SetRadiusMin( const a2dDoMu& radiusMin ) 
    void SetRadiusMax( const a2dDoMu& radiusMax )
    void SetAberPolyToArc( const a2dDoMu& aber )
    void SetAberArcToPoly( const a2dDoMu& aber );
    void SetSmall( const a2dDoMu& smallest );
    bool GdsIoSaveTextAsPolygon( bool onOff ); 
    bool KeyIoSaveTextAsPolygon( bool onOff );    
    bool GdsIISaveFromView( bool onOff );
    bool KeySaveFromView( bool onOff );
	
    const a2dDoMu&  GetRadiusMin() 
    const a2dDoMu& GetRadiusMax() 
    const a2dDoMu& GetAberPolyToArc() 
    const a2dDoMu& GetAberArcToPoly()
    const a2dDoMu& GetSmall() 
    
    void SetGridX( const a2dDoMu& gridX )
    void SetGridY( const a2dDoMu& gridY )
    const a2dDoMu& GetBooleanEngineMarge() const
    long GetBooleanEngineGrid() const
    double GetBooleanEngineDGrid() const
    const a2dDoMu& GetBooleanEngineCorrectionAber() const
    const a2dDoMu& GetBooleanEngineCorrectionFactor() const
    const a2dDoMu& GetBooleanEngineMaxlinemerge() const
    bool GetBooleanEngineWindingRule() const
    double GetBooleanEngineRoundfactor() const
    const a2dDoMu& GetBooleanEngineSmoothAber() const
    bool GetBooleanEngineLinkHoles() const

};

// *****************************************************************
// wx/editor\candoc.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dCanvasDocument
// ---------------------------------------------------------------------------

#include "wx/editor/candoc.h"
class a2dCanvasDocument : public a2dDocument
{
    a2dCanvasDocument()
    a2dCanvasDocument( const a2dCanvasDocument &other )
    a2dDrawing* GetDrawing() const
    void SetDrawing( a2dDrawing* newRoot )
    void SetDescription( const wxString& desc )
    wxString& GetDescription()
    void SetLibraryName( const wxString& libraryname )
    wxString& GetLibraryName()
    void SetVersion( wxString version )
    wxString GetVersion()
    virtual void DeleteContents()
    bool WriteSVG( a2dCanvasView* drawer, const wxString& filename, a2dCanvasObject* top, double Width, double Height, wxString unit)
    virtual a2dDocumentInputStream& LoadObject( a2dDocumentInputStream& stream, a2dIOHandlerStrIn* handler )
    void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser )
    bool Load( const wxString& filename, a2dIOHandlerStrIn* handler )
    bool SaveLayers( const wxString& filename )
    bool LoadLayers( const wxString& filename )
    virtual void UpdateLoadedLayerSetup()
    a2dDocumentOutputStream& SaveObject( a2dDocumentOutputStream& stream, a2dIOHandlerStrOut* handler )
    void Save( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dObjectList* towrite )
    void Save( a2dIOHandlerCVGOut &out, a2dCanvasObject* start = 0 )
    bool Save( const wxString& filename,  a2dIOHandlerStrOut* handler ) const
    void Update( a2dCanvasObject::UpdateMode mode )
    void SetLayerSetup( a2dLayers* layersetup )
    a2dLayers* GetLayerSetup()
    void SetMultiRoot( bool multiRoot = true )
    bool GetMultiRoot()
    a2dCanvasObject* GetStartObject()
    void SetStartObject( a2dCanvasObject* start )
    a2dCanvasObject* GetShowObject()
    void SetShowObject( a2dCanvasObject* show )
    double GetUnitsAccuracy()
    void   SetUnitsAccuracy(double accuracy)
    double GetUnitsScale()
    void   SetUnitsScale(double scale)
    wxString GetUnits()
    void SetUnits( const wxString& unitString )
    void SetModificationTime( const wxDateTime& datatime )
    wxDateTime& GetModificationTime()
    void SetAccessTime( const wxDateTime& datatime )
    wxDateTime& GetAccessTime()
    wxDateTime& GetInternalChangedTime()
};
