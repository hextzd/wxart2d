/*! \file cansymboledit.cpp
    \brief editor for symbol drawing

	a2dSymbol being edited as drawing.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: cansymboledit.cpp,v 1.17 2009/09/26 20:40:32 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/docview/doccom.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/cansymboledit.h"
#include "wx/canvas/canvas.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/mastertoolgroups.h"
#include "wx/evtloop.h"

#if wxUSE_XRC
#include <wx/xrc/xmlres.h>
#include <wx/canvas/xh_a2dmenu.h>
#endif

/********************************************************************
* a2dDrawingEditor
*********************************************************************/

a2dDrawingEditorFrameEvent::a2dDrawingEditorFrameEvent( a2dDrawingEditor* editorFrame, wxEventType type, int id )
    : wxEvent( id, type )
{
    SetEventObject( editorFrame );
}

a2dDrawingEditor* a2dDrawingEditorFrameEvent::GetEditorFrame() const
{
    return wxDynamicCast( GetEventObject(), a2dDrawingEditor );
}

wxEvent* a2dDrawingEditorFrameEvent::Clone( void ) const
{
    return new a2dDrawingEditorFrameEvent( GetEditorFrame(), m_eventType, m_id );
}

DEFINE_EVENT_TYPE( a2dEVT_DRAWINGEDITOR_THEME_EVENT )
DEFINE_EVENT_TYPE( a2dEVT_DRAWINGEDITOR_INIT_EVENT )

BEGIN_EVENT_TABLE( a2dDrawingEditor, wxFrame )

    EVT_ACTIVATE( a2dDrawingEditor::OnActivate )
    EVT_CLOSE( a2dDrawingEditor::OnCloseWindow )

    EVT_MENU( wxID_UNDO, a2dDrawingEditor::OnUndo )
    EVT_MENU( wxID_REDO, a2dDrawingEditor::OnRedo )

    EVT_MENU( wxID_CUT,  a2dDrawingEditor::OnCutShape)
    EVT_MENU( wxID_COPY,  a2dDrawingEditor::OnCopyShape)
    EVT_MENU( wxID_PASTE, a2dDrawingEditor::OnPasteShape)

    EVT_MENUSTRINGS( a2dDrawingEditor::OnSetmenuStrings )

    EVT_UPDATE_UI( wxID_UNDO, a2dDrawingEditor::OnUpdateUndo )
    EVT_UPDATE_UI( wxID_REDO, a2dDrawingEditor::OnUpdateRedo )

    EVT_DO( a2dDrawingEditor::OnDoEvent )
    EVT_UNDO( a2dDrawingEditor::OnUndoEvent )

    EVT_COM_EVENT( a2dDrawingEditor::OnComEvent )

    EVT_UPDATE_UI( wxID_PASTE, a2dDrawingEditor::OnUpdatePasteShape)
    EVT_UPDATE_UI( wxID_CUT, a2dDrawingEditor::OnUpdateCopyShape)
    EVT_UPDATE_UI( wxID_COPY, a2dDrawingEditor::OnUpdateCopyShape)

	EVT_MENU_HIGHLIGHT_ALL( a2dDrawingEditor::OnMenuHighLight )

    EVT_DRAWINGEDITOR_THEME_EVENT( a2dDrawingEditor::OnTheme )
    EVT_DRAWINGEDITOR_INIT_EVENT( a2dDrawingEditor::OnInit )  
    EVT_MENU( wxID_EXIT, a2dDrawingEditor::OnQuit )

END_EVENT_TABLE()

/*!
 * a2dDrawingEditor constructors
 */

a2dDrawingEditor::a2dDrawingEditor( a2dDrawing* drawing, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : wxFrame()
{
    m_drawing = drawing;
    m_drawingPart = NULL;
    m_initialized = false;

    // create a canvas in Create, the first arg. is true.
    bool res = wxFrame::Create( NULL, wxID_ANY, wxT("wxArt2d"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE );

    if ( drawing )
    {
        Init();

        a2dCanvasGlobals->SetActiveDrawingPart( m_canvas->GetDrawingPart() );

        m_initialized = true;
        // need to know change in active drawingpart
        a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );

        m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_MENUSTRINGS, this );
        m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_DO, this );
        m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_UNDO, this );
        m_drawingPart->GetDrawing()->GetCanvasCommandProcessor()->SetMenuStrings();
    }
}

/*!
 * a2dDrawingEditor destructor
 */

a2dDrawingEditor::~a2dDrawingEditor()
{
    m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_MENUSTRINGS, this );
    m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_DO, this );
    m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_UNDO, this );
    m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_REDO, this );

    GetAuiManager().UnInit();
}


/*!
 * Member initialisation
 */

void a2dDrawingEditor::Init()
{
    m_LastMenuDrawingId = a2dDrawingId_normal();
    m_StatusBar = NULL;

    m_initialized = false;

    wxPathList pathList;
    pathList.Add( wxT( "../common/icons" ) );

    a2dDrawingEditorFrameEvent tevent( this, a2dEVT_DRAWINGEDITOR_INIT_EVENT );
    ProcessEvent( tevent );

    a2dDrawingEditorFrameEvent ievent( this, a2dEVT_DRAWINGEDITOR_THEME_EVENT );
    ProcessEvent( ievent );

}

void a2dDrawingEditor::OnInit( a2dDrawingEditorFrameEvent& initEvent )
{
#ifdef __WXMSW__
    SetIcon( wxString( wxT( "chrt_icn" ) ) );
#endif

    m_editMenu = NULL;

    if ( m_drawingPart )
    {
        a2dFill backgr = a2dFill( wxColour( 255, 255, 255 ) );
        m_drawingPart->SetBackgroundFill( backgr );

        m_drawingPart->SetGridStroke( a2dStroke( wxColour( 239, 5, 64 ), 0, a2dSTROKE_DOT ) );
        //to show grid as lines
        //m_drawingPart1->SetGridLines(true);

        m_drawingPart->SetGridSize( 1 );
        m_drawingPart->SetGridX( 100 );
        m_drawingPart->SetGridY( 100 );

        //show it?
        m_drawingPart->SetGrid( false );
        m_drawingPart->SetGridAtFront( true );
    }
    m_initialized = true;
}

void a2dDrawingEditor::OnTheme( a2dDrawingEditorFrameEvent& themeEvent )
{
    a2dDrawingEditor* ef = themeEvent.GetEditorFrame();
    //ef->Theme();

    CreateControls();
    CreateThemeDefault();

    wxUint16 lay = m_drawingPart->GetDrawing()->GetHabitat()->GetLayer();
    SetStatusText( m_drawingPart->GetDrawing()->GetLayerSetup()->GetName( lay ), 1 );
}

void a2dDrawingEditor::OnQuit( wxCommandEvent& event )
{
    Close( true );
}


/*!
 *Control creation for a2dDrawingEditor
 */

void a2dDrawingEditor::CreateControls()
{
	GetAuiManager().SetManagedWindow(this);

    a2dStToolContr* contr = NULL;
    a2dCanvas* canvas = NULL;

    m_drawingPart = new a2dDrawingPartTiled( GetSize().GetWidth(), GetSize().GetHeight() );
    m_drawingPart->SetShowOrigin( false );
    //m_drawingPart->ConnectEvent( a2dEVT_COM_EVENT, this );

    m_canvas = new a2dCanvas( m_drawingPart, this, -1, wxDefaultPosition, GetSize(), wxNO_FULL_REPAINT_ON_RESIZE | wxWANTS_CHARS |
        wxHSCROLL | wxVSCROLL | wxSUNKEN_BORDER | wxALWAYS_SHOW_SB);
	GetAuiManager().AddPane( m_canvas, wxAuiPaneInfo()
		.Name("Canvas").Centre().Row(1).Position(1).
		CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(true).PaneBorder(false));

    m_canvas->GetDrawer2D()->StartRefreshDisplayDisable();
    m_canvas->GetDrawer2D()->SetSplineAberration( 1000 );

    m_canvas->SetDrawing( m_drawing );
    m_drawingPart->SetShowObject( m_drawing->GetRootObject() );

    a2dDrawing* drawing = wxStaticCast( m_drawingPart->GetDrawing(), a2dDrawing );

    m_canvas->GetDrawer2D()->SetSmallTextThreshold( 10000 );
    m_canvas->GetDrawer2D()->SetSmallTextThreshold( 50 );
    a2dFont::SetLoadFlags( a2dFont::a2d_LOAD_DEFAULT | a2dFont::a2d_LOAD_FORCE_AUTOHINT ); 

    //following true on start up is not so good ;-), do it later
    //when everything is on the screen (problem is size events generated while intializing a window)
    m_canvas->SetScaleOnResize( false );
    m_drawingPart->SetZoomOutBorder( 25 );

    m_canvas->SetScrollbars( 20, 20, 50, 50 );
    // sets the initial display area
    m_canvas->SetMappingUpp( -1000, -1000, 50, 50 );

    //do not go outide the scroll maximum, need to be set after the above, and bore the next.
    m_canvas->FixScrollMaximum(true);
    //defines the whole area in world coordinates that can be seen by scrolling
    m_canvas->SetScrollMaximum( -1000, -1000, 20000, 20000 );

    //defines the number of world units scrolled when line up or down events in X
    m_canvas->SetScrollStepX( 10 );
    //defines the number of world units scrolled when line up or down events in Y
    m_canvas->SetScrollStepY( 10 );

    a2dFill backgr = a2dFill( wxColour( 255, 255, 255 ) );
    m_canvas->SetBackgroundFill( backgr );
    m_canvas->SetGridStroke( a2dStroke( wxColour( 129, 129, 129 ), 0, a2dSTROKE_DOT ) );
    //to show grid as lines
    //m_canvas->SetGridLines(true);

    m_canvas->SetGridSize( 0 );
    m_canvas->SetGridX( 100 );
    m_canvas->SetGridY( 100 );
    m_canvas->SetYaxis( true );

    //show it?
    m_canvas->SetGrid( true );
    m_canvas->SetGridAtFront( false );
    m_drawingPart->SetGridThreshold(5); // performance  when zooming

    m_canvas->SetCursor( wxCURSOR_PENCIL );

    m_canvas->SetBackgroundColour( wxColour( 0, 235, 235 ) );
    m_canvas->ClearBackground();

    contr  = new a2dStToolContr( m_drawingPart, this, false );

    a2dStToolFixedToolStyleEvtHandler* evthf = new a2dStToolFixedToolStyleEvtHandler( contr );
    contr->SetDefaultToolEvtHandlerFixedStyle( evthf );
    contr->SetZoomFirst( true );
    contr->SetSnap( true );

    contr->SetFormat( wxT( "%6.1f" ) );

    contr->GetFirstTool()->SetShowAnotation( false );
    //contr->SetOpacityFactorEditcopy( 125 );
    //contr->SetUseOpaqueEditcopy( true );

    contr->SetZoomFirst( false );

    contr->SetSelectionStateUndo( true );
    contr->SetSelectAtEnd( true );


#ifdef TA_DEVELOPMENT
    contr->SetDefaultBehavior( contr->GetDefaultBehavior() | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
#else
    contr->SetDefaultBehavior( contr->GetDefaultBehavior() | wxTC_NoContextMenu | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
#endif


    a2dMasterTagGroups3* tool = new  a2dMasterTagGroups3( contr );
    tool->SetEscapeToStopFirst( true );
    //tool->SetWireMode( true );
    tool->SetDlgOrEdit( true );
    tool->SetDlgOrEditModal( true );
    tool->SetSelectMoreAtShift( true );
    tool->SetAllowMultiEdit( false );
	tool->SetLateConnect( false );
    tool->SetEndSegmentMode( a2dCanvasGlobals->GetHabitat()->GetEndSegmentMode() );
    contr->StopAllTools();
    contr->SetTopTool( tool );

    m_canvas->GetDrawer2D()->EndRefreshDisplayDisable();

    drawing->GetCanvasCommandProcessor()->SetMenuStrings();

	GetAuiManager().Update();
}

void a2dDrawingEditor::AddCmdMenu( wxMenu* parentMenu, wxMenuItem* item )
{
    Connect( item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDrawingEditor::OnCmdMenuId ) );
    parentMenu->Append( item );
}

void a2dDrawingEditor::AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
{
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDrawingEditor::OnCmdMenuId ) );
    parentMenu->Append( cmdId.GetId(), wxGetTranslation( cmdId.GetText() ), wxGetTranslation( cmdId.GetHelp() ), cmdId.IsCheckable() );
}

void a2dDrawingEditor::OnCmdMenuId( wxCommandEvent& event )
{
    event.Skip(); // HANDLED IN m_drawing command processor.
}

void a2dDrawingEditor::ConnectCmdId( const a2dMenuIdItem& cmdId )
{
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDrawingEditor::OnCmdMenuId ) );
}

void a2dDrawingEditor::RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
{
    Disconnect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDrawingEditor::OnCmdMenuId ) );
    parentMenu->Delete( cmdId.GetId() );
}

void a2dDrawingEditor::CreateThemeDefault( bool modal )
{
    CreateToolbars();

	m_StatusBar = new wxStatusBar( this, wxNewId(), wxST_SIZEGRIP|wxNO_BORDER );
	m_StatusBar->SetFieldsCount( 7 );
	SetStatusBar(m_StatusBar);
	int widths[] = { 80, 100, 200, 300, -1, -1, -1 };
	SetStatusWidths( 7, widths );

    m_menubar = new wxMenuBar;
    SetMenuBar( m_menubar );

    //Centre(wxBOTH);

    //SETUP all menu dynamic, so can be removed too.
    m_menuFile = new wxMenu;

    AddCmdMenu( m_menuFile, CmdMenu_Drawing_FileOpen() );
    AddCmdMenu( m_menuFile, CmdMenu_Drawing_FileClose() );
    AddCmdMenu( m_menuFile, CmdMenu_Drawing_FileSave() );
    AddCmdMenu( m_menuFile, CmdMenu_Drawing_FileSaveAs() );
    //AddCmdMenu( m_menuFile, CmdMenu_Drawing_Print() );
/*
    m_menuFile->AppendSeparator();
	AddCmdMenu( m_menuFile, CmdMenu_PrintSetup() );
*/
	AddCmdMenu( m_menuFile, CmdMenu_PreviewDrawing() );
	AddCmdMenu( m_menuFile, CmdMenu_PrintDrawing() );
/*
    AddCmdMenu( m_menuFile, CmdMenu_PrintView() );
    AddCmdMenu( m_menuFile, CmdMenu_PreviewView() );
*/
    m_menuFile->AppendSeparator();

    wxMenu* file_import = new wxMenu;
    m_menuFile->Append( wxNewId(), wxT( "import file" ), file_import, _( "Import a file" ) );
    AddCmdMenu( file_import, CmdMenu_FileImport() );

    wxMenu* file_export_as = new wxMenu;
    m_menuFile->Append( wxNewId(), wxT( "Export As" ), file_export_as, _( "Export in other format" ) );
    AddCmdMenu( file_export_as, CmdMenu_FileExport() );

    file_export_as->AppendSeparator();
    m_menuFile->AppendSeparator();

    if ( !modal )
    {
	    m_menuFile->AppendSeparator();
        m_menuFile->Append( wxID_EXIT, _("exit"), _("Exit") );
        //AddCmdMenu( m_menuFile, CmdMenu_Exit() );
    }

	//===============================
	// Menu <Edit>
	//--------------
	m_editMenu = new wxMenu;

	m_editMenu->Append( wxID_UNDO, _("Undo\tCtrl+Z"), _("Undo") );
	m_editMenu->Append( wxID_REDO, _("Redo\tCtrl+Y"), _("Rdeo") );
	m_editMenu->AppendSeparator();
	m_editMenu->Append( wxID_CUT, _("Cut\tCtrl+X"), _("Cut") );
	m_editMenu->Append( wxID_COPY, _("Copy\tCtrl+C"), _("Copy") );
	m_editMenu->Append( wxID_PASTE, _("Paste\tCtrl+V"), _("Paste") );
	AddCmdMenu( m_editMenu, CmdMenu_Selected_Delete() );
	m_editMenu->AppendSeparator();
	AddCmdMenu( m_editMenu, CmdMenu_Selected_SelectAll_NoUndo() );
	AddCmdMenu( m_editMenu, CmdMenu_Selected_DeSelectAll_NoUndo() );

    //Connect( MSHOW_FIRST, MSHOW_LAST, wxEVT_COMMAND_MENU_SELECTED, ( wxObjectEventFunction ) &a2dDrawingEditor::ShowLibs );

    wxMenu* optionMenu = new wxMenu;
    /*
    AddFunctionToMenu( TOOLOPTION_ONTOP, optionMenu, _( "Drag &OnTop" ), _( "Drag on top of other objects" ), &a2dDrawingEditorFrame::SetToolDragingMode, true );
    AddFunctionToMenu( TOOLOPTION_RECTANGLE, optionMenu, _( "Drag &Rectangle" ), _( "Drag using a rectangle" ), &a2dDrawingEditorFrame::SetToolDragingMode, true );
    AddFunctionToMenu( TOOLOPTION_REDRAW, optionMenu, _( "Drag Re &Draw" ), _( "Drag and redraw all objects all the time" ), &a2dDrawingEditorFrame::SetToolDragingMode, true );
    AddFunctionToMenu( TOOLOPTION_COPY, optionMenu, _( "Drag &Copy" ), _( "Drag copy of original untill finished drag" ), &a2dDrawingEditorFrame::SetToolDragingMode, true );
    AddFunctionToMenu( TOOLOPTION_FILLED, optionMenu, _( "Drag/Draw Filled" ), _( "Drag/Draw filled objects" ), &a2dDrawingEditorFrame::SetToolDragingMode, true );
    AddFunctionToMenu( TOOLOPTION_WIRE, optionMenu, _( "Drag/Draw OutLine" ), _( "Drag/Draw an wireframe/outline of object" ), &a2dDrawingEditorFrame::SetToolDrawingMode, true );
    AddFunctionToMenu( TOOLOPTION_INVERT, optionMenu, _( "Drag/Draw Invert" ), _( "Drag/Draw an inverted wireframe of object" ), &a2dDrawingEditorFrame::SetToolDrawingMode, true );
    AddFunctionToMenu( TOOLOPTION_WIRE_ZERO_WIDTH, optionMenu, _( "Drag/Draw OutLine zero" ), _( "Drag/Draw zero width wireframe/outline of object" ), &a2dDrawingEditorFrame::SetToolDrawingMode, true );
    AddFunctionToMenu( TOOLOPTION_INVERT_ZERO_WIDTH, optionMenu, _( "Drag/Draw Invert zero" ), _( "Drag/Draw a zero width inverted wireframe of object" ), &a2dDrawingEditorFrame::SetToolDrawingMode, true );
    optionMenu->AppendSeparator();
    */
    AddCmdMenu( optionMenu, CmdMenu_Option_SplinePoly() );
    AddCmdMenu( optionMenu, CmdMenu_Option_CursorCrosshair() );
    AddCmdMenu( optionMenu, CmdMenu_Option_RescaleAtReSize() );
    AddCmdMenu( optionMenu, CmdMenu_Option_ReverseLayers() );
    AddCmdMenu( optionMenu, CmdMenu_SetYAxis() );


    wxMenu* cameleonMenu = new wxMenu;
    AddCmdMenu( cameleonMenu,CmdMenu_ShowDlgCameleonModal() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_ShowDiagram() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_ShowSymbol() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_ShowGui() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_Selected_CameleonSymbol() );             
    AddCmdMenu( cameleonMenu,CmdMenu_Selected_CameleonDiagram() );             
    AddCmdMenu( cameleonMenu,CmdMenu_Selected_CloneCameleonFromInst() );             
    AddCmdMenu( cameleonMenu,CmdMenu_Selected_FlattenCameleon() );             
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_CameleonInst() );             
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_CameleonInstDlg() );             
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_Cameleon() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_Cameleon_BuildIn() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_Port() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_CameleonRefDlg() );         
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_CameleonRefDlg_Symbol() );         
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_CameleonRefDlg_Diagram() );         

    wxMenu* toolMenu = new wxMenu;

    AddCmdMenu( toolMenu, CmdMenu_PushTool_Select() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_RecursiveEdit() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_MultiEdit() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Drag() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Copy() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Rotate() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Delete() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawText() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Image() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawRectangle() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawCircle() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawLine() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawLineScaledArrow() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawEllipse() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawEllipticArc() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawEllipticArc_Chord() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawArc() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawArc_Chord() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawPolylineL() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawPolygonL() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Property() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DragMulti() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_CopyMulti() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Measure() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawWirePolylineL() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawVPath() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Port() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Cameleon() );

    toolMenu->AppendSeparator();
    AddCmdMenu( toolMenu, CmdMenu_RotateObject90Left() );
    AddCmdMenu( toolMenu, CmdMenu_RotateObject90Right() );
    AddCmdMenu( toolMenu, CmdMenu_LineBegin() );
    AddCmdMenu( toolMenu, CmdMenu_LineEnd() );
    AddCmdMenu( toolMenu, CmdMenu_LineScale() );
    AddCmdMenu( toolMenu, CmdMenu_SetNormalizeFactor() );

    //toolMenu->Append(MTOOL_PlaceFromLibByRef, _("add ref to object from lib"), _(""));
    //toolMenu->Append(MTOOL_PlaceFromLibByCopy, _("add object from lib"), _(""));


    wxMenu* drawMenu = new wxMenu;
    AddCmdMenu( drawMenu, CmdMenu_ShowDlgStructure() );
    //AddFunctionToMenu( MSHOW_LIBPOINTS, drawMenu, _( "Point Library" ), _( "a library used for end and begin points of lines" ), &a2dDrawingEditorFrame::OnMenu );

    wxMenu* refMenu = new wxMenu;

    AddCmdMenu( refMenu, CmdMenu_Refresh() );
    AddCmdMenu( refMenu, CmdMenu_PushInto() );
    AddCmdMenu( refMenu, CmdMenu_NewGroup() );
    AddCmdMenu( refMenu, CmdMenu_NewPin() );

    wxMenu* performMenu = new wxMenu;
    wxMenu* selected = new wxMenu;
    wxMenu* groups = new wxMenu;

    /////////////////////////////////////////
    // selected like operation
    /////////////////////////////////////////
    performMenu->Append( wxNewId(), wxT( "selected objects" ), selected, _( "operation on selected objects" ) );

    AddCmdMenu( selected, CmdMenu_Selected_SelectAll() );
    AddCmdMenu( selected, CmdMenu_Selected_DeSelectAll() );
    AddCmdMenu( selected, CmdMenu_Selected_Delete() );
    AddCmdMenu( selected, CmdMenu_Selected_MoveXY() );
    AddCmdMenu( selected, CmdMenu_Selected_MoveLayer() );
    AddCmdMenu( selected, CmdMenu_Selected_CopyXY() );
    AddCmdMenu( selected, CmdMenu_Selected_CopyLayer() );
    AddCmdMenu( selected, CmdMenu_Selected_Transform() );
    AddCmdMenu( selected, CmdMenu_Selected_Rotate() );
    AddCmdMenu( selected, CmdMenu_Selected_RotateObject90Right() );
    AddCmdMenu( selected, CmdMenu_Selected_RotateObject90Left() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MinX() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MaxX() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MinY() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MaxY() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MidX() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MidY() );
    AddCmdMenu( selected, CmdMenu_Selected_Group() );
    AddCmdMenu( selected, CmdMenu_Selected_ImageToRectangles() );
    AddCmdMenu( selected, CmdMenu_Selected_ToTop() );
    AddCmdMenu( selected, CmdMenu_Selected_ToBack() );
    AddCmdMenu( selected, CmdMenu_Selected_SetStyle() );
    AddCmdMenu( selected, CmdMenu_Selected_SetExtStyle() );
	AddCmdMenu( selected, CmdMenu_Selected_SetTextChanges() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertToPolygonPolylinesWithArcs() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertToPolygonPolylinesWithoutArcs() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertToPolylines() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertLinesArcs() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertPolygonToArcs() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertPolylineToArcs() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertToVPaths() );
    AddCmdMenu( selected, CmdMenu_Selected_UnGroup() );
    AddCmdMenu( selected, CmdMenu_Selected_UnGroupDeep() );
    AddCmdMenu( selected, CmdMenu_Selected_Merge() );
    AddCmdMenu( selected, CmdMenu_Selected_Offset() );
    AddCmdMenu( selected, CmdMenu_Selected_CreateRing() );
	AddCmdMenu( selected, CmdMenu_Selected_Align_MinX_Dest() );
	AddCmdMenu( selected, CmdMenu_Selected_Align_MaxX_Dest() );
	AddCmdMenu( selected, CmdMenu_Selected_Align_MinY_Dest() );
	AddCmdMenu( selected, CmdMenu_Selected_Align_MaxY_Dest() );
	AddCmdMenu( selected, CmdMenu_Selected_Align_MidX_Dest() );
	AddCmdMenu( selected, CmdMenu_Selected_Align_MidY_Dest() );
	AddCmdMenu( selected, CmdMenu_Selected_Align_DistributeVert() );
	AddCmdMenu( selected, CmdMenu_Selected_Align_DistributeHorz() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MirrorVertBbox() );
	AddCmdMenu( selected, CmdMenu_Selected_Align_MirrorHorzBbox() );


#if wxART2D_USE_KBOOL
    /////////////////////////////////////////
    // group like operation
    /////////////////////////////////////////
    performMenu->Append( wxNewId(), wxT( "layer groups" ), groups, _( "operation on objects in layer groups" ) );
    AddCmdMenu( groups, CmdMenu_ShowDlgGroups() );
    AddCmdMenu( groups, CmdMenu_GroupAB_Offset() );
    AddCmdMenu( groups, CmdMenu_GroupAB_Smooth() );
    AddCmdMenu( groups, CmdMenu_GroupAB_CreateRing() );
    AddCmdMenu( groups, CmdMenu_GroupAB_Delete() );
    AddCmdMenu( groups, CmdMenu_GroupAB_Move() );
    AddCmdMenu( groups, CmdMenu_GroupAB_Copy() );
    AddCmdMenu( groups, CmdMenu_GroupAB_ToArcs() );

    wxMenu* booleanObjects  = new wxMenu;
    groups->Append( wxNewId(), wxT( "boolean objects" ), booleanObjects, _( "boolean group A-B objects" ) );

    //boolean operation based on group settings
    AddCmdMenu( booleanObjects, CmdMenu_GroupAB_Or() );
    AddCmdMenu( booleanObjects, CmdMenu_GroupAB_And() );
    AddCmdMenu( booleanObjects, CmdMenu_GroupAB_Exor() );
    AddCmdMenu( booleanObjects, CmdMenu_GroupAB_AsubB() );
    AddCmdMenu( booleanObjects, CmdMenu_GroupAB_BsubA() );

#endif //wxART2D_USE_KBOOL

    AddCmdMenu( performMenu, CmdMenu_EmptyDocument() );
    AddCmdMenu( performMenu, CmdMenu_EmptyShownObject() );

    wxMenu* menuSettings = new wxMenu;

    AddCmdMenu( menuSettings, CmdMenu_ShowDlgStyle() );
    AddCmdMenu( menuSettings, CmdMenu_SetFont() );
    AddCmdMenu( menuSettings, CmdMenu_SetBackgroundStyle() );
    AddCmdMenu( menuSettings, CmdMenu_DrawGridLines() );
    AddCmdMenu( menuSettings, CmdMenu_DrawGridAtFront() );
    AddCmdMenu( menuSettings, CmdMenu_DrawGrid() );
    AddCmdMenu( menuSettings, CmdMenu_SetSnap() );
    AddCmdMenu( menuSettings, CmdMenu_SetDrawWireFrame() );

    wxMenu* menuLayers = new wxMenu;

    AddCmdMenu( menuLayers, CmdMenu_SetLayerDlg() );
    AddCmdMenu( menuLayers, CmdMenu_SetLayerDlgModeless() );
    AddCmdMenu( menuLayers, CmdMenu_SetTargetDlg() );

    //AddFunctionToMenu( Layer_view, menuLayers, _( "Show layer view" ), _( "layers of this document" ), &a2dDrawingEditorFrame::OnMenu, true );

    wxMenu* layerDlgInit  = new wxMenu;
    menuLayers->Append( wxNewId(), wxT( "LayerDlg" ), layerDlgInit, _( "Show LayerDlg" ) );
    AddCmdMenu( layerDlgInit, CmdMenu_ShowDlgLayersDrawing() );
    AddCmdMenu( layerDlgInit, CmdMenu_ShowDlgLayersGlobal() );

    wxMenu* orderDlgInit  = new wxMenu;
    menuLayers->Append( wxNewId(), wxT( "OrderDlg" ), orderDlgInit, _( "Show OrderDlg" ) );
    AddCmdMenu( orderDlgInit, CmdMenu_ShowDlgLayerOrderGlobal() );

    wxMenu* dlg_menu = new wxMenu;

    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgPathSettings() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgSettings() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgTrans() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgTools() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgStyle() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgSnap() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgStructure() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgIdentify() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgMeasure() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgCoordEntry() );

    //AddCmdMenu( dlg_menu, CmdMenu_InsertGroupRef() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgGroups() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgPropEdit() );

#if defined(_DEBUG)
    wxMenu* drawTest = new wxMenu;
    //drawTest->Append( MSCREEN_T, wxT( "test screen" ), wxT( "whatever" ) );
#endif

    wxMenu* help_menu = new wxMenu;
    //AddFunctionToMenu( wxID_ABOUT, help_menu, _( "&About" ), _( "&About" ), &a2dDrawingEditorFrame::OnAbout );

    m_menubar->Append( m_menuFile, _( "&File" ) );
    m_menubar->Append( m_editMenu, _( "&Edit" ) );
    m_menubar->Append( optionMenu, _( "&Options" ) );
    m_menubar->Append( cameleonMenu,  _( "&Cameleons" ) );
    m_menubar->Append( toolMenu, _( "&Tools" ) );
    m_menubar->Append( drawMenu, _( "&Show" ) );
    m_menubar->Append( refMenu, _( "&Action" ) );
    m_menubar->Append( performMenu, _( "&Perform Operation" ) );
    m_menubar->Append( menuSettings, _( "Settings" ) );
    m_menubar->Append( menuLayers, _( "Layers" ) );
    m_menubar->Append( dlg_menu, _( "&Dialogs" ) );
#if defined(_DEBUG)
    m_menubar->Append( drawTest, _( "&Test" ) );
#endif
    m_menubar->Append( help_menu, _( "&Help" ) );

    //FillLibraryPoints();

}

void a2dDrawingEditor::CreateThemeXRC()
{
	m_StatusBar = new wxStatusBar( this, wxNewId(), wxST_SIZEGRIP|wxNO_BORDER );
	m_StatusBar->SetFieldsCount( 7 );
	SetStatusBar(m_StatusBar);
	int widths[] = { 80, 100, 200, 300, -1, -1, -1 };
	SetStatusWidths( 7, widths );

    m_menubar = new wxMenuBar;
    SetMenuBar( m_menubar );

#if wxUSE_XRC
    if( wxXmlResource::Get() )
    {
        wxXmlResource::Get()->InitAllHandlers();
        wxXmlResource::Get()->AddHandler( new a2dMenuBarXmlHandler );
        wxXmlResource::Get()->AddHandler( new a2dMenuXmlHandler );
        wxXmlResource::Get()->AddHandler( new a2dToolBarXmlHandler );

        wxString art2d = a2dGeneralGlobals->GetWxArt2DVar( true );
        art2d += wxT( "art/resources/" );

//      wxString aResName = wxString::Format(wxT("../resource/%s.xrc"),GetAppName().c_str());
        wxString aResName = wxT( "a2dEditorFrame.xrc" );
        if( !wxXmlResource::Get()->Load( art2d + aResName ) )
            wxMessageBox( wxString::Format( _( "Error load XRC-file '%s'.\nWorking directory is '%s'" ), aResName.c_str(), wxGetCwd().c_str() ) );
    }

    wxXmlResource::Get()->LoadObject( this, wxT( "frameMenu" ), wxT( "a2dMenuBar" ) );
    wxXmlResource::Get()->LoadObject( this, wxT( "toolbar" ), wxT( "a2dToolBar" ) );

    m_editMenu = m_menubar->GetMenu( m_menubar->FindMenu( wxT("Edit") ) );
#endif
}

void a2dDrawingEditor::CreateToolbars()
{
	//===============================
	// Toolbar <Cameleons>
	//--------------------
	wxAuiToolBar* tbCameleon = new wxAuiToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_GRIPPER );
	tbCameleon->SetToolBitmapSize(wxSize(20, 20));

	AddCmdToToolbar( tbCameleon, CmdMenu_PushTool_Port() );
	AddCmdToToolbar( tbCameleon, CmdMenu_PushTool_Origin_OneShot() );

	tbCameleon->Realize();
	GetAuiManager().AddPane(tbCameleon, wxAuiPaneInfo()
		.ToolbarPane().Name("tbCameleon").Top().Layer(10).LeftDockable(false).RightDockable(false).
		CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(false).PaneBorder(false));
	
	//===============================
	// Toolbar <File>
	//--------------------
	wxAuiToolBar* tbFile = new wxAuiToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_GRIPPER );
	tbFile->SetToolBitmapSize(wxSize(20, 20));

	tbFile->AddTool( wxID_COPY, wxT("Copy"), GetBitmapResource("CmdMenu_EditCopy"), _("Copy") );
	tbFile->AddTool( wxID_PASTE, wxT("Paste"), GetBitmapResource("CmdMenu_EditPaste"), _("Paste") );
	tbFile->AddSeparator();
	tbFile->AddTool( wxID_UNDO, wxT("Undo"), GetBitmapResource("CmdMenu_EditUndo"), _("Undo") );
	tbFile->AddTool( wxID_REDO, wxT("Redo"), GetBitmapResource("CmdMenu_EditRedo"), _("Redo") );

	tbFile->Realize();
	GetAuiManager().AddPane(tbFile, wxAuiPaneInfo()
		.ToolbarPane().Name("tbFile").Top().LeftDockable(false).RightDockable(false).
		CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(false).PaneBorder(true));
	
	
	//===============================
	// Toolbar <View>
	//--------------------
	wxAuiToolBar* tbView = new wxAuiToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_GRIPPER );
	tbView->SetToolBitmapSize(wxSize(20, 20));

	AddCmdToToolbar( tbView, CmdMenu_PushTool_Zoom_OneShot() );
	AddCmdToToolbar( tbView, CmdMenu_ZoomIn2CondSelected() );
	AddCmdToToolbar( tbView, CmdMenu_ZoomOut2CondSelected() );
	AddCmdToToolbar( tbView, CmdMenu_ZoomOut() );

	tbView->Realize();
	GetAuiManager().AddPane(tbView, wxAuiPaneInfo()
		.ToolbarPane().Name("tbView").Top().Layer(9).LeftDockable(false).RightDockable(false).
		CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(false).PaneBorder(true));


	//===============================
	// Toolbar <Object>
	//--------------------
	wxAuiToolBar* tbObject = new wxAuiToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_GRIPPER );
	tbObject->SetToolBitmapSize(wxSize(20, 20));

	AddCmdToToolbar( tbObject, CmdMenu_Selected_Group() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_UnGroup() );
	tbObject->AddSeparator();
	AddCmdToToolbar( tbObject, CmdMenu_Selected_ToTop() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_ToBack() );
	tbObject->AddSeparator();
	AddCmdToToolbar( tbObject, CmdMenu_Selected_RotateObject90Left() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_RotateObject90Right() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MirrorVertBbox() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MirrorHorzBbox() );
	tbObject->AddSeparator();
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MinX_Dest() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MaxX_Dest() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MinY_Dest() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MaxY_Dest() );
	tbObject->AddSeparator();
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MidX_Dest() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MidY_Dest() );
	tbObject->AddSeparator();
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_DistributeVert() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_DistributeHorz() );

	tbObject->Realize();
	GetAuiManager().AddPane(tbObject, wxAuiPaneInfo()
		.ToolbarPane().Name("tbObject").Top().Layer(9).LeftDockable(false).RightDockable(false).
		CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(false).PaneBorder(true));


	//===============================
	// Toolbar <Tools>
	//--------------------
	wxAuiToolBar* tbTools = new wxAuiToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_VERTICAL|wxAUI_TB_GRIPPER );
	tbTools->SetToolBitmapSize(wxSize(20, 20));

	AddCmdToToolbar( tbTools, CmdMenu_PushTool_RecursiveEdit() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawText() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawPolygonL() );
	//AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawPolygonL_Splined() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawPolylineL() );
	//AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawPolylineL_Splined() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawRectangle() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawCircle() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawEllipse() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawEllipticArc() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawEllipticArc_Chord() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawArc() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawArc_Chord() );

	tbFile->SetToolBitmapSize(wxSize(20, 20));
	tbTools->Realize();
	GetAuiManager().AddPane(tbTools, wxAuiPaneInfo()
		.ToolbarPane().Name("tbTools").Layer(9).Right().TopDockable(false).BottomDockable(false)
		.CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(false).PaneBorder(false).Dockable(false));
/*
    AddCmdToToolbar(  toolbar, CmdMenu_PushInto() );
    AddCmdToToolbar(  toolbar, CmdMenu_PopOut() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Zoom() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Cameleon() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Port() );
    AddCmdToToolbar(  toolbar, CmdMenu_Selected_CameleonSymbol() );          
    AddCmdToToolbar(  toolbar, CmdMenu_Selected_CameleonDiagram() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Cameleon() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Origin_OneShot() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_CameleonInst() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_CameleonInstDlg() );             

    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawPolygonL() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawPolygonL_Splined() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawPolylineL() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawPolylineL_Splined() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawRectangle() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawCircle() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawEllipse() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawEllipticArc() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawEllipticArc_Chord() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawArc() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawArc_Chord() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DrawText() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Image() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Link() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Rotate() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Drag() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Copy() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Delete() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_RecursiveEdit() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Select() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_Select2() );
    AddCmdToToolbar(  toolbar, CmdMenu_Selected_SelectAll() );
    AddCmdToToolbar(  toolbar, CmdMenu_Selected_DeSelectAll() );
    AddCmdToToolbar(  toolbar, CmdMenu_Selected_Delete() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_DragMulti() );
    AddCmdToToolbar(  toolbar, CmdMenu_PushTool_CopyMulti() );
*/
    GetAuiManager().Update();
}


/*!
 * Should we show tooltips?
 */

bool a2dDrawingEditor::ShowToolTips()
{
    return true;
}


/*!
 * Get bitmap resources
 */

wxBitmap a2dDrawingEditor::GetBitmapResource( const wxString& name )
{
	wxString art2d = a2dGeneralGlobals->GetWxArt2DArtVar();
	art2d += wxT( "resources/" );

	wxBitmap bmp;
	if( wxFileExists( art2d + name + wxT( ".ico" ) ) )
	{
		if( bmp.LoadFile( art2d + name + wxT( ".ico" ), wxBITMAP_TYPE_ICO ) )
			return bmp;
	}
	else if( wxFileExists( art2d + name + wxT( ".bmp" ) ) )
	{
		if( bmp.LoadFile( art2d + name + wxT( ".bmp" ), wxBITMAP_TYPE_BMP ) )
			return bmp;
	}
	else if( wxFileExists( art2d + name + wxT( ".png" ) ) )
	{
		if( bmp.LoadFile( art2d + name + wxT( ".png" ), wxBITMAP_TYPE_PNG ) )
			return bmp;
	}

	wxFAIL_MSG( wxString::Format("Bitmap resource \"%s\" could not be loaded.", name) );
	return wxNullBitmap;
}

bool a2dDrawingEditor::ProcessEvent( wxEvent& event )
{
    bool ret = false;
    {
		if (event.GetEventType() == wxEVT_MENU)
        {
			if ( !wxEvtHandler::ProcessEvent( event ) )
			{
				if ( m_drawing && m_drawing->GetCommandProcessor() )
				{
					ret = m_drawing->GetCommandProcessor()->ProcessEvent( event );
				}
				else
					ret = false;
			}
			else
				ret = true;
        }
        else
        {
			// EVENTS needed for view directly, should be handled above.
			// events down here go only to the parent window or commandprocessor

			//In case the event is a commandevent, it will automatically be redirected to the parent window by
			// wxEvtHandler::ProcessEvent
			//There is no need to handle that case in here.
			if ( !wxEvtHandler::ProcessEvent( event ) )
			{
				if ( m_drawing && m_drawing->GetCommandProcessor() )
				{
					ret = m_drawing->GetCommandProcessor()->ProcessEvent( event );
				}
				else
					ret = false;
			}
			else
				ret = true;
		}
    }
    return ret;
}

void a2dDrawingEditor::AddCmdToToolbar( wxAuiToolBar* toolbar, const a2dMenuIdItem& cmdId )
{
    if ( ! toolbar )
        return;
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDrawingEditor::OnCmdMenuId ) );

    wxASSERT_MSG( a2dMenuIdItem::GetInitialized(), _T( "call a2dMenuIdItem::InitializeBitmaps() before using AddCmdToToolbar" ) );

    wxString error = _T( "No Bitmap for a2dToolCmd found for:" ) + cmdId.GetIdName();
    wxASSERT_MSG( cmdId.GetBitmap().Ok(), error );
	toolbar->AddTool( cmdId.GetId(), wxGetTranslation( cmdId.GetLabel() ), cmdId.GetBitmap( true ), wxGetTranslation( cmdId.GetHelp() ), cmdId.GetKind() );
}

void a2dDrawingEditor::OnCloseWindow( wxCloseEvent& event )
{
	if( event.CanVeto() && m_canvas->GetDrawing()->IsModified() )
	{
		int answer = wxMessageBox( _("Store result?"), wxT("wxArt2D" ), wxICON_QUESTION | wxYES_NO | wxCANCEL);
		if( answer == wxYES )
		{
		}
		else if( answer == wxCANCEL )
		{
			event.Veto();
			return;
		}
	}

    m_drawingPart->GetCanvasToolContr()->StopAllTools();
    Destroy();
}

void a2dDrawingEditor::OnActivate( wxActivateEvent& event )
{
    if ( !m_initialized )
    {
        event.Skip();
        return;
    }
    event.Skip(); //skip to base OnActivate Handler which sets proper focus to child window(s)
}

void a2dDrawingEditor::OnSetmenuStrings( a2dCommandProcessorEvent& event )
{
    if ( !m_drawingPart || !m_drawingPart->GetDrawing() )
        return;

    m_UndoRedoCmd = wxStaticCast( event.GetEventObject(), a2dCommandProcessor );

    if ( m_editMenu && m_drawingPart->GetDrawing() && 
        event.GetEventObject() == m_drawingPart->GetDrawing()->GetCommandProcessor() )
    {
        m_editMenu->SetLabel( wxID_UNDO, event.GetUndoMenuLabel() );
        m_editMenu->SetLabel( wxID_REDO, event.GetRedoMenuLabel() );
        m_editMenu->Enable( wxID_UNDO, event.CanUndo() );
        m_editMenu->Enable( wxID_REDO, event.CanRedo() );
    }
}

void a2dDrawingEditor::OnMenuHighLight( wxMenuEvent& event )
{
	int menuId = event.GetMenuId();
    wxString helpString;
    if ( menuId != wxID_SEPARATOR && menuId != wxID_NONE )
    {
        const wxMenuItem * const item = FindItemInMenuBar(menuId);
        if ( item && !item->IsSeparator() )
            helpString = item->GetHelp();

	    SetStatusText( helpString, 3 );
    }

}

void a2dDrawingEditor::OnUpdateUndo( wxUpdateUIEvent& event )
{
    if ( !m_drawingPart || !m_drawingPart->GetDrawing() )
    { 
        event.Enable( false );
        return;
    }
    
    // assume the current active commandprocessor is based on the active drawingpart its a2dDrawing.
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawer )
    {
        //event.Skip();
        return;
    }
    a2dDrawing* drawing = drawer->GetDrawing();
    if ( m_UndoRedoCmd == drawing->GetCommandProcessor()  )
	    event.Enable( drawing->GetCommandProcessor()->CanUndo() );
    else
	    event.Enable( false );
}

void a2dDrawingEditor::OnUpdateRedo( wxUpdateUIEvent& event )
{
    // assume the current active commandprocessor is based on the active drawingpart its a2dDrawing.
    a2dDrawingPart* drawer = m_drawingPart; 
    if ( !drawer )
    {
        //event.Skip();
        return;
    }

    a2dDrawing* drawing = drawer->GetDrawing();
    if ( m_UndoRedoCmd == drawing->GetCommandProcessor()  )
	    event.Enable( drawing->GetCommandProcessor()->CanRedo() );
    else
	    event.Enable( false );
}

void a2dDrawingEditor::OnUndoEvent( a2dCommandProcessorEvent& WXUNUSED( event ) )
{

}

void a2dDrawingEditor::OnDoEvent( a2dCommandProcessorEvent& event )
{
    wxString cmdName = event.GetCommand()->GetName();

    if ( !m_drawingPart || !m_drawingPart->GetDrawing() )
        return;

    if ( m_drawingPart->GetDrawing() && event.GetEventObject() == m_drawingPart->GetDrawing()->GetCommandProcessor() )
    {
    }
}

void a2dDrawingEditor::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
    if ( !m_drawingPart )
        return;

    //if ( m_drawingPart->GetDrawing() && m_drawingPart->GetDrawing()->GetCommandProcessor() )
    //    m_drawingPart->GetDrawing()->GetCommandProcessor()->Undo();

    if ( m_drawingPart->GetDrawing() && m_UndoRedoCmd )
	{
		m_UndoRedoCmd->Undo();
	}
}

void a2dDrawingEditor::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
    if ( !m_drawingPart )
        return;

    //if ( m_drawingPart->GetDrawing() && m_drawingPart->GetDrawing()->GetCommandProcessor() )
    //    m_drawingPart->GetDrawing()->GetCommandProcessor()->Redo();

    if ( m_drawingPart->GetDrawing() && m_UndoRedoCmd )
	{
		m_UndoRedoCmd->Redo();
	}
}

void a2dDrawingEditor::OnComEvent( a2dComEvent& event )
{
    if ( event.GetId() == a2dStToolContr::sm_showCursor && GetStatusBar() )
    {
        a2dStToolContr* contr = wxStaticCast( event.GetEventObject(), a2dStToolContr );
        wxString str;

        //This will show mouse coordinates SetStatusText( contr->GetStatusStrings()[0], 0 );
        SetStatusText( contr->GetStatusStrings()[STAT_Xw_Yw], 0 );
		SetStatusText( contr->GetStatusStrings()[STAT_Feedback], 1 );

		if ( contr->GetStatusStrings()[STAT_Feedback2].IsEmpty() )
	        SetStatusText( contr->GetStatusStrings()[STAT_ToolString], 2 );
		else
			SetStatusText( contr->GetStatusStrings()[STAT_Feedback2], 2 );

	    SetStatusText( contr->GetStatusStrings()[STAT_toolHelp], 3 );

        SetStatusText( a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->GetRouteMethodAsString(), 4 );
        wxUint16 lay = a2dCanvasGlobals->GetHabitat()->GetLayer();
        if (  contr->GetDrawingPart()->GetDrawing() && contr->GetDrawingPart()->GetDrawing()->GetLayerSetup() )
            SetStatusText( contr->GetDrawingPart()->GetDrawing()->GetLayerSetup()->GetName( lay ), 5 );
        if (  contr->GetDrawingPart()->GetDrawing() )
            SetStatusText( contr->GetDrawingPart()->GetDrawing()->GetName(), 6 );
    }
    else if ( event.GetId() == a2dHabitat::sig_changedLayer )
    {
        wxUint16 lay = a2dCanvasGlobals->GetHabitat()->GetLayer();
        if ( m_drawingPart->GetDrawing()->GetLayerSetup() )
            SetStatusText( m_drawingPart->GetDrawing()->GetLayerSetup()->GetName( lay ), 5 );
    }
    else
        event.Skip();
}

void a2dDrawingEditor::OnCutShape(wxCommandEvent& WXUNUSED(event))
{
}

void a2dDrawingEditor::OnCopyShape(wxCommandEvent& WXUNUSED(event))
{
    // only for Regler
    a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !( part && part->GetShowObject() && part->GetShowObject()->GetRoot() ) )
        return;
}

void a2dDrawingEditor::OnPasteShape(wxCommandEvent& WXUNUSED(event))
{
    // only for Regler
    a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !( part && part->GetShowObject() && part->GetShowObject()->GetRoot() ) )
        return;

	a2dDragMultiNewTool* dragnew = wxDynamicCast( part->GetCanvasToolContr()->GetFirstTool(), a2dDragMultiNewTool );
	if ( dragnew && dragnew->GetBusy() )
		return;


}

void a2dDrawingEditor::OnUpdatePasteShape( wxUpdateUIEvent& event  )
{
    a2dDrawingPart* part = m_drawingPart; 
    if ( !part || !part->GetDrawing() || !part->GetDrawing()->GetCommandProcessor() )
    {
        event.Enable( false );
        return;
    }
}

void a2dDrawingEditor::OnUpdateCopyShape( wxUpdateUIEvent& event  )
{
    a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !part || !part->GetDrawing() || !part->GetDrawing()->GetCommandProcessor() )
    {
        event.Enable( false );
        return;
    }

    bool ret = false;
    if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
    {
		if ( wxDynamicCast( part->GetCanvasToolContr(), a2dStToolContr ) )
		{
	        a2dStToolContr* contr = wxDynamicCast( part->GetCanvasToolContr(), a2dStToolContr );
			if ( contr )
			{
    			a2dObjectEditTool* edittool = wxDynamicCast( contr->GetFirstTool(), a2dObjectEditTool );
				if ( edittool )
				{
			        a2dText* text = wxDynamicCast( edittool->GetEditObjectCopy(), a2dText );
					if ( text && !text->GetSelectedText().IsEmpty() )
                        ret = true;
				}
			}
		}

        if ( !ret )
        {
            //something selected?
            a2dCanvasObjectList* objects = part->GetShowObject()->GetChildObjectList();
            forEachIn( a2dCanvasObjectList, objects )
            {
                a2dCanvasObject* obj = *iter;
                if ( obj->GetRelease() || !obj->IsVisible()  || !obj->GetSelected() )
                    continue;
                ret = true;
                break;
            }
        }
    }
    event.Enable( ret );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++
/*!
 * a2dDrawingEditorModal
 */

const long a2dID_EXIT_CANCEL = wxNewId();
const long a2dID_EXIT_OKE = wxNewId();

BEGIN_EVENT_TABLE( a2dDrawingEditorModal, a2dDrawingEditor )

    EVT_MENU( a2dID_EXIT_CANCEL, a2dDrawingEditorModal::Cancel )
    EVT_MENU( a2dID_EXIT_OKE, a2dDrawingEditorModal::Oke )
    EVT_CLOSE( a2dDrawingEditorModal::OnCloseWindow )

END_EVENT_TABLE()


/*!
 * TaDrawingEditor constructors
 */

a2dDrawingEditorModal::a2dDrawingEditorModal( a2dDrawing* drawing, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : a2dDrawingEditor( NULL, parent, id, caption, pos, size, style )
{
    m_drawing = drawing;
    m_drawingPart = NULL;
    m_initialized = false;

    Init();

	CreateMenubar();
	CreateToolbars();
    CreateControls();

    a2dCanvasGlobals->SetActiveDrawingPart( m_canvas->GetDrawingPart() );

    m_initialized = true;
    // need to know change in active drawingpart
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );

    m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_MENUSTRINGS, this );
    m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_DO, this );
    m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_UNDO, this );
    m_drawingPart->GetDrawing()->GetCanvasCommandProcessor()->SetMenuStrings();

    SetExtraStyle(GetExtraStyle() | wxTOPLEVEL_EX_DIALOG);
}

/*!
 * TaDrawingEditor destructor
 */

a2dDrawingEditorModal::~a2dDrawingEditorModal()
{   
	delete m_eventLoop;

    // if the dialog is modal, this will end its event loop
    if ( IsModal() )
        EndModal(wxID_CANCEL);  
}

/*!
 * Member initialisation
 */

void a2dDrawingEditorModal::Init()
{
    a2dDrawingEditor::Init();

	m_returnCode = 0;
    m_windowDisabler = NULL;
    m_eventLoop = NULL;
    m_isShowingModal = false;
}

/*!
 *Control creation for a2dDrawingEditor
 */

void a2dDrawingEditorModal::CreateControls()
{
    a2dDrawingEditor::CreateControls();

}

void a2dDrawingEditorModal::CreateMenubar()
{
    CreateThemeDefault( true );

    m_menuFile->Append( a2dID_EXIT_OKE, _("Store and exit"), _("Exit and store changes") );
	m_menuFile->Append( a2dID_EXIT_CANCEL, _("Cancel"), _("Cancel Editing do not store changes") );
}

void a2dDrawingEditorModal::CreateToolbars()
{
    a2dDrawingEditor::CreateToolbars();

	//===============================
	// Toolbar <Ok Cancel>
	//--------------------
	wxAuiToolBar* tbOKCancel = new wxAuiToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_GRIPPER );

	tbOKCancel->AddTool( a2dID_EXIT_OKE, wxT("OK"), GetBitmapResource("OK"), _("Store and exit") );
	tbOKCancel->AddTool( a2dID_EXIT_CANCEL, wxT("Cancel"), GetBitmapResource("Cancel"), _("Cancel editing") );
}

void a2dDrawingEditorModal::OnCloseWindow( wxCloseEvent& event )
{
	if( event.CanVeto() && m_canvas->GetDrawing()->IsModified() )
	{
		int answer = wxMessageBox( _("Do you want to preserve the changes?"), wxT("wxArt2d" ), wxICON_QUESTION | wxYES_NO | wxCANCEL);
		if( answer == wxYES )
		{
			m_returnCode = wxID_OK;
			//Close( true );
			//return;
		}
		else if( answer == wxCANCEL )
		{
			event.Veto();
			return;
		}
	}

    m_drawingPart->GetCanvasToolContr()->StopAllTools();
    EndModal( m_returnCode );
    //Destroy();
}

void a2dDrawingEditorModal::Oke( wxCommandEvent& event )
{
    m_returnCode = wxID_OK;
    Close( true );
    //event.Skip();
}

void a2dDrawingEditorModal::Cancel( wxCommandEvent& event )
{
    m_returnCode = wxID_CANCEL;
    Close( true );
    //event.Skip();
}

bool a2dDrawingEditorModal::Show(bool show)
{
    if ( !show )
    {
        // if we had disabled other app windows, reenable them back now because
        // if they stay disabled Windows will activate another window (one
        // which is enabled, anyhow) and we will lose activation
        if ( m_windowDisabler )
        {
            delete m_windowDisabler;
            m_windowDisabler = NULL;
        }

        if ( IsModal() )
            EndModal(wxID_CANCEL);
    }

    bool ret = a2dDrawingEditor::Show(show);

    return ret;
}

bool a2dDrawingEditorModal::IsModal() const 
{
   return m_isShowingModal;
}

int a2dDrawingEditorModal::ShowModal() 
{
	GetAuiManager().Update();
	m_drawingPart->SetMappingShowAllCanvas();   

    if ( IsModal() )
    {
       wxFAIL_MSG( wxT("a2dDrawingEditorModal::ShowModal called twice") );
       return GetReturnCode();
    }

    // use the apps top level window as parent if none given unless explicitly
    // forbidden
    if ( !GetParent() )
    {
        wxWindow *parent = wxTheApp->GetTopWindow();
        if ( parent && parent != this )
        {
            m_parent = parent;
        }
    }

    Show(true);

    m_isShowingModal = true;

    wxASSERT_MSG( !m_windowDisabler, _T("disabling windows twice?") );

#if defined(__WXGTK__) || defined(__WXMGL__)
    wxBusyCursorSuspender suspender;
    // FIXME (FIXME_MGL) - make sure busy cursor disappears under MSW too
#endif

    m_windowDisabler = new wxWindowDisabler(this);
    if ( !m_eventLoop )
        m_eventLoop = new wxEventLoop;

    m_eventLoop->Run();

    return GetReturnCode();
}


void a2dDrawingEditorModal::EndModal(int retCode) 
{
    wxASSERT_MSG( m_eventLoop, _T("a2dDrawingEditorModal is not modal") );

    SetReturnCode(retCode);

    if ( !IsModal() )
    {
        wxFAIL_MSG( wxT("a2dDrawingEditorModal::EndModal called twice") );
        return;
    }

    m_isShowingModal = false;

    m_eventLoop->Exit();

    Show(false);
}


void a2dDrawingEditorModal::SetReturnCode(int retCode) 
{
   m_returnCode = retCode;
}


int a2dDrawingEditorModal::GetReturnCode() const 
{
   return m_returnCode;
}

//===================================================

/*!
 * a2dDrawingEditorDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( a2dDrawingEditorDlg, wxDialog )

/*!
 * a2dDrawingEditorDlg event table definition
 */

BEGIN_EVENT_TABLE( a2dDrawingEditorDlg, wxDialog )

////@begin a2dDrawingEditorDlg event table entries
    EVT_BUTTON( IDSE_BUTTON, a2dDrawingEditorDlg::OnClickOke )

    EVT_BUTTON( IDSE_BUTTON1, a2dDrawingEditorDlg::OnClickCancel )

////@end a2dDrawingEditorDlg event table entries

END_EVENT_TABLE()

/*!
 * a2dDrawingEditorDlg constructors
 */

a2dDrawingEditorDlg::a2dDrawingEditorDlg( )
{
    // need to know change in active drawingpart
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );
}

a2dDrawingEditorDlg::a2dDrawingEditorDlg( a2dDrawing* drawing, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
	m_drawing = drawing;

    // need to know change in active drawingpart
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );

    m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_MENUSTRINGS, this );
    m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_DO, this );
    m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_UNDO, this );
    m_drawingPart->GetDrawing()->GetCanvasCommandProcessor()->SetMenuStrings();

    Create(parent, id, caption, pos, size, style);
}

a2dDrawingEditorDlg::~a2dDrawingEditorDlg()
{
    m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_MENUSTRINGS, this );
    m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_DO, this );
    m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_UNDO, this );
    m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_REDO, this );

    GetAuiManager().UnInit();
}

/*!
 * a2dDrawingEditorDlg creator
 */

bool a2dDrawingEditorDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin a2dDrawingEditorDlg member initialisation
    m_canvas = NULL;
    m_oke = NULL;
    m_cancel = NULL;
////@end a2dDrawingEditorDlg member initialisation

////@begin a2dDrawingEditorDlg creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end a2dDrawingEditorDlg creation
    return TRUE;
}

/*!
 * Control creation for a2dDrawingEditorDlg
 */

void a2dDrawingEditorDlg::CreateControls()
{    
////@begin a2dDrawingEditorDlg content construction
    a2dDrawingEditorDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 2);

    m_panel = new wxPanel( itemDialog1, IDSE_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(m_panel, 1, wxGROW|wxALL, 5);

	GetAuiManager().SetManagedWindow( m_panel );

    a2dStToolContr* contr = NULL;

    m_drawingPart = new a2dDrawingPart( GetSize().GetWidth(), GetSize().GetHeight() );
    m_drawingPart->SetShowOrigin( false );
    //m_drawingPart->ConnectEvent( a2dEVT_COM_EVENT, this );


    m_canvas = new a2dCanvas( m_drawingPart,  m_panel, -1, wxDefaultPosition, GetSize(), wxNO_FULL_REPAINT_ON_RESIZE | wxWANTS_CHARS );
	GetAuiManager().AddPane( m_canvas, wxAuiPaneInfo()
		.Name("Canvas").Centre().Row(1).Position(1).
		CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(true).PaneBorder(false));

    m_canvas->GetDrawer2D()->StartRefreshDisplayDisable();
    m_canvas->GetDrawer2D()->SetSplineAberration( 1000 );

    m_canvas->SetDrawing( m_drawing );
    m_drawingPart->SetShowObject( m_drawing->GetRootObject() );

    a2dDrawing* drawing = wxStaticCast( m_drawingPart->GetDrawing(), a2dDrawing );

    m_canvas->GetDrawer2D()->SetSmallTextThreshold( 10000 );
    m_canvas->GetDrawer2D()->SetSmallTextThreshold( 50 );
    a2dFont::SetLoadFlags( a2dFont::a2d_LOAD_DEFAULT | a2dFont::a2d_LOAD_FORCE_AUTOHINT ); 

    //following true on start up is not so good ;-), do it later
    //when everything is on the screen (problem is size events generated while intializing a window)
    m_canvas->SetScaleOnResize( false );
    m_drawingPart->SetZoomOutBorder( 25 );

    m_canvas->SetScrollbars( 20, 20, 50, 50 );
    // sets the initial display area
    m_canvas->SetMappingUpp( -1000, -1000, 50, 50 );

    //do not go outide the scroll maximum, need to be set after the above, and bore the next.
    m_canvas->FixScrollMaximum(true);
    //defines the whole area in world coordinates that can be seen by scrolling
    m_canvas->SetScrollMaximum( -1000, -1000, 20000, 20000 );

    //defines the number of world units scrolled when line up or down events in X
    m_canvas->SetScrollStepX( 10 );
    //defines the number of world units scrolled when line up or down events in Y
    m_canvas->SetScrollStepY( 10 );

    a2dFill backgr = a2dFill( wxColour( 220, 220, 220 ) );
    m_canvas->SetBackgroundFill( backgr );
    m_canvas->SetGridStroke( a2dStroke( wxColour( 129, 129, 129 ), 0, a2dSTROKE_DOT ) );
    //to show grid as lines
    //m_canvas->SetGridLines(true);

    m_canvas->SetGridSize( 0 );
    m_canvas->SetGridX( 100 );
    m_canvas->SetGridY( 100 );

    //show it?
    m_canvas->SetGrid( true );
    m_canvas->SetGridAtFront( false );
    m_drawingPart->SetGridThreshold(5); // performance  when zooming

    m_canvas->SetCursor( wxCURSOR_PENCIL );

    m_canvas->SetBackgroundColour( wxColour( 0, 235, 235 ) );
    m_canvas->ClearBackground();

    contr  = new a2dStToolContr( m_drawingPart, NULL, false );

    a2dStToolFixedToolStyleEvtHandler* evthf = new a2dStToolFixedToolStyleEvtHandler( contr );
    contr->SetDefaultToolEvtHandlerFixedStyle( evthf );
    contr->SetZoomFirst( true );
    contr->SetSnap( true );

    contr->SetFormat( wxT( "%6.1f" ) );

    contr->GetFirstTool()->SetShowAnotation( false );
    //contr->SetOpacityFactorEditcopy( 125 );
    //contr->SetUseOpaqueEditcopy( true );

    contr->SetZoomFirst( false );

    contr->SetSelectionStateUndo( true );
    contr->SetSelectAtEnd( true );


#ifdef TA_DEVELOPMENT
    contr->SetDefaultBehavior( contr->GetDefaultBehavior() | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
#else
    contr->SetDefaultBehavior( contr->GetDefaultBehavior() | wxTC_NoContextMenu | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
#endif


    a2dMasterTagGroups3* tool = new  a2dMasterTagGroups3( contr );
    tool->SetEscapeToStopFirst( true );
    //tool->SetWireMode( true );
    tool->SetDlgOrEdit( true );
    tool->SetDlgOrEditModal( true );
    tool->SetSelectMoreAtShift( true );
    tool->SetAllowMultiEdit( false );
	tool->SetLateConnect( false );
    tool->SetEndSegmentMode( a2dCanvasGlobals->GetHabitat()->GetEndSegmentMode() );
    contr->StopAllTools();
    contr->SetTopTool( tool );

	CreateToolbars();

    m_canvas->GetDrawer2D()->EndRefreshDisplayDisable();

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_oke = new wxButton( itemDialog1, IDSE_BUTTON, _("Oke"), wxDefaultPosition, wxDefaultSize, 0 );
    m_oke->SetHelpText(_("Editing symbol ready"));
    itemBoxSizer6->Add(m_oke, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_cancel = new wxButton( itemDialog1, IDSE_BUTTON1, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    m_cancel->SetHelpText(_("Edting symbol canceled"));
    itemBoxSizer6->Add(m_cancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for IDSE_BUTTON
 */

void a2dDrawingEditorDlg::OnClickOke( wxCommandEvent& event )
{
    a2dToolContr* contr  = m_drawingPart->GetCanvasToolContr();
	contr->StopAllTools();

	EndModal( wxID_OK );
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for IDSE_BUTTON1
 */

void a2dDrawingEditorDlg::OnClickCancel( wxCommandEvent& event )
{
    a2dToolContr* contr  = m_drawingPart->GetCanvasToolContr();
	contr->StopAllTools();

	EndModal( wxID_CANCEL );
}

wxBitmap a2dDrawingEditorDlg::GetBitmapResource( const wxString& name )
{
	wxString art2d = a2dGeneralGlobals->GetWxArt2DArtVar();
	art2d += wxT( "resources/" );

	wxBitmap bmp;
	if( wxFileExists( art2d + name + wxT( ".ico" ) ) )
	{
		if( bmp.LoadFile( art2d + name + wxT( ".ico" ), wxBITMAP_TYPE_ICO ) )
			return bmp;
	}
	else if( wxFileExists( art2d + name + wxT( ".bmp" ) ) )
	{
		if( bmp.LoadFile( art2d + name + wxT( ".bmp" ), wxBITMAP_TYPE_BMP ) )
			return bmp;
	}
	else if( wxFileExists( art2d + name + wxT( ".png" ) ) )
	{
		if( bmp.LoadFile( art2d + name + wxT( ".png" ), wxBITMAP_TYPE_PNG ) )
			return bmp;
	}

	wxFAIL_MSG( wxString::Format("Bitmap resource \"%s\" could not be loaded.", name) );
	return wxNullBitmap;
}

void a2dDrawingEditorDlg::CreateToolbars()
{
	//===============================
	// Toolbar <Cameleons>
	//--------------------
	wxAuiToolBar* tbCameleon = new wxAuiToolBar( m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_GRIPPER );
	tbCameleon->SetToolBitmapSize(wxSize(20, 20));

	AddCmdToToolbar( tbCameleon, CmdMenu_PushTool_Port() );
	AddCmdToToolbar( tbCameleon, CmdMenu_PushTool_Origin_OneShot() );
	
	tbCameleon->Realize();
	GetAuiManager().AddPane(tbCameleon, wxAuiPaneInfo()
		.ToolbarPane().Name("tbCameleon").Top().Layer(10).LeftDockable(false).RightDockable(false).
		CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(false).PaneBorder(false));
	
	//===============================
	// Toolbar <File>
	//--------------------
	wxAuiToolBar* tbFile = new wxAuiToolBar( m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_GRIPPER );
	tbFile->SetToolBitmapSize(wxSize(20, 20));

	tbFile->AddTool( wxID_COPY, wxT("Copy"), GetBitmapResource("CmdMenu_EditCopy"), _("Copy") );
	tbFile->AddTool( wxID_PASTE, wxT("Paste"), GetBitmapResource("CmdMenu_EditPaste"), _("Paste") );
	tbFile->AddSeparator();
	tbFile->AddTool( wxID_UNDO, wxT("Undo"), GetBitmapResource("CmdMenu_EditUndo"), _("Undo") );
	tbFile->AddTool( wxID_REDO, wxT("Redo"), GetBitmapResource("CmdMenu_EditRedo"), _("Redo") );

	tbFile->Realize();
	GetAuiManager().AddPane(tbFile, wxAuiPaneInfo()
		.ToolbarPane().Name("tbFile").Top().LeftDockable(false).RightDockable(false).
		CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(false).PaneBorder(true));
	
	
	//===============================
	// Toolbar <View>
	//--------------------
	wxAuiToolBar* tbView = new wxAuiToolBar( m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_GRIPPER );
	tbView->SetToolBitmapSize(wxSize(20, 20));

	AddCmdToToolbar( tbView, CmdMenu_PushTool_Zoom_OneShot() );
	AddCmdToToolbar( tbView, CmdMenu_ZoomIn2CondSelected() );
	AddCmdToToolbar( tbView, CmdMenu_ZoomOut2CondSelected() );
	AddCmdToToolbar( tbView, CmdMenu_ZoomOut() );

	tbView->Realize();
	GetAuiManager().AddPane(tbView, wxAuiPaneInfo()
		.ToolbarPane().Name("tbView").Top().Layer(9).LeftDockable(false).RightDockable(false).
		CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(false).PaneBorder(true));


	//===============================
	// Toolbar <Object>
	//--------------------
	wxAuiToolBar* tbObject = new wxAuiToolBar( m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_GRIPPER );
	tbObject->SetToolBitmapSize(wxSize(20, 20));

	AddCmdToToolbar( tbObject, CmdMenu_Selected_Group() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_UnGroup() );
	tbObject->AddSeparator();
	AddCmdToToolbar( tbObject, CmdMenu_Selected_ToTop() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_ToBack() );
	tbObject->AddSeparator();
	AddCmdToToolbar( tbObject, CmdMenu_Selected_RotateObject90Left() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_RotateObject90Right() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MirrorVertBbox() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MirrorHorzBbox() );
	tbObject->AddSeparator();
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MinX_Dest() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MaxX_Dest() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MinY_Dest() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MaxY_Dest() );
	tbObject->AddSeparator();
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MidX_Dest() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_MidY_Dest() );
	tbObject->AddSeparator();
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_DistributeVert() );
	AddCmdToToolbar( tbObject, CmdMenu_Selected_Align_DistributeHorz() );

	tbObject->Realize();
	GetAuiManager().AddPane(tbObject, wxAuiPaneInfo()
		.ToolbarPane().Name("tbObject").Top().Layer(9).LeftDockable(false).RightDockable(false).
		CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(false).PaneBorder(true));


	//===============================
	// Toolbar <Tools>
	//--------------------
	wxAuiToolBar* tbTools = new wxAuiToolBar( m_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_VERTICAL|wxAUI_TB_GRIPPER );
	tbTools->SetToolBitmapSize(wxSize(20, 20));

	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawText() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawPolygonL() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawPolygonL_Splined() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawPolylineL() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawPolylineL_Splined() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawRectangle() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawCircle() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawEllipse() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawEllipticArc() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawEllipticArc_Chord() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawArc() );
	AddCmdToToolbar( tbTools, CmdMenu_PushTool_DrawArc_Chord() );

	tbTools->Realize();
	GetAuiManager().AddPane(tbTools, wxAuiPaneInfo()
		.ToolbarPane().Name("tbTools").Layer(9).Right().TopDockable(false).BottomDockable(false)
		.CaptionVisible(false).CloseButton(false).DestroyOnClose(false).Resizable(false).PaneBorder(false).Dockable(false));
}

void a2dDrawingEditorDlg::AddCmdToToolbar( wxAuiToolBar* toolbar, const a2dMenuIdItem& cmdId )
{
    if ( ! toolbar )
        return;
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDrawingEditorDlg::OnCmdMenuId ) );

    wxASSERT_MSG( a2dMenuIdItem::GetInitialized(), _T( "call a2dMenuIdItem::InitializeBitmaps() before using AddCmdToToolbar" ) );

    wxString error = _T( "No Bitmap for a2dToolCmd found for:" ) + cmdId.GetIdName();
    wxASSERT_MSG( cmdId.GetBitmap().Ok(), error );
	toolbar->AddTool( cmdId.GetId(), wxGetTranslation( cmdId.GetLabel() ), cmdId.GetBitmap( true ), wxGetTranslation( cmdId.GetHelp() ), cmdId.GetKind() );
}

void a2dDrawingEditorDlg::OnCmdMenuId( wxCommandEvent& event )
{
	m_canvas->GetDrawing()->GetCommandProcessor()->ProcessEvent( event );
    event.Skip();
}
