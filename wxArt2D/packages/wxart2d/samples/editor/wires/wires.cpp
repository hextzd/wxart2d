/*! \file modules/editor/samples/wires/wires.cpp
   \author Erik van der Wal
   \brief First attempts for a wire demo, sorry q&d code at the moment.
    EW: code gets moreand more messy, definitely needs cleanup/documentation
   Copyright: 2004 (C) Erik van der Wal

   Licence: wxWidgets license

   RCS-ID: $Id: wires.cpp,v 1.15 2009/09/26 19:01:19 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

// Include image
#include "smile.xpm"

#include <wx/treectrl.h>
#include "wx/confbase.h"

#include <wx/canvas/mastertool.h>
#include <wx/canvas/mastertoolgroups.h>
#include <wx/editor/candocproc.h>
#include <wx/canvas/canmod.h>
#include <wx/editor/xmlparsdoc.h>

#if wxART2D_USE_AGGDRAWER
#include <wx/aggdrawer/aggdrawer.h>
#endif //wxART2D_USE_AGGDRAWER


#if (wxART2D_USE_LUA == 1)
#include "wxlua/debug/wxldebug.h"
#include "wxlua/debugger/wxldserv.h"
#include "wxlua/debugger/wxldtarg.h"
#include "wx/luawraps/luawrap.h"
#include "wx/luawraps/luabind.h"
#include "wxlua/wxlstate.h"
#include "wx/luawraps/luawrap.h"
#include "wx/luawraps/luabind.h"
#include "wxbind/include/wxbinddefs.h"
WXLUA_DECLARE_BIND_STD
#endif //(wxART2D_USE_LUA == 1)

// Include private headers
#include "wires.h"
#include "elements.h"

BEGIN_EVENT_TABLE( a2dConnector, a2dViewConnector )
    EVT_POST_CREATE_VIEW( a2dConnector::OnPostCreateView )
    EVT_POST_CREATE_DOCUMENT( a2dConnector::OnPostCreateDocument )
    EVT_DISCONNECT_VIEW( a2dConnector::OnDisConnectView )
END_EVENT_TABLE()

a2dConnector::a2dConnector()
    : a2dViewConnector()
{
	m_canvas = NULL;
}

void a2dConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    a2dCanvasView* view = ( a2dCanvasView* ) event.GetView();

    if ( m_canvas )
    {
		view->SetDisplayWindow( m_canvas->GetDrawingPart()->GetDisplayWindow() );
        view->SetPrintTitle( false );
        view->SetPrintFilename( false );
        view->SetPrintFrame( false );
		m_canvas->SetDrawing( view->GetCanvasDocument()->GetDrawing() );

		if ( m_canvas->GetDrawingPart()->GetCanvasToolContr() )
			m_canvas->GetDrawingPart()->GetCanvasToolContr()->StopAllTools();
		if ( m_canvas->GetDrawingPart()->GetCanvasToolContr() )
			m_canvas->GetDrawingPart()->GetCanvasToolContr()->ReStart();

		m_canvas->GetDrawingPart()->SetShowObject( view->GetCanvasDocument()->GetDrawing()->GetRootObject() );
		view->GetCanvasDocument()->Update( a2dCanvasObject::updatemask_force_and_count );
        m_canvas->GetDrawingPart()->SetMappingShowAll();

	}
}

void a2dConnector::OnPostCreateDocument( a2dTemplateEvent& event )
{
    //a2dDocumentTemplate* templ = (a2dDocumentTemplate*) event.GetEventObject();
    if ( !m_canvas )
        return;

    a2dViewTemplate* viewtempl;
    viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates().front();

    a2dCanvasView* view = ( a2dCanvasView* ) viewtempl->CreateView( event.GetDocument(), event.GetFlags() );
    view->SetPrintTitle( false );
    view->SetPrintFilename( false );
    view->SetPrintFrame( false );

    //a2dCanvasView* view = new a2dCanvasView();
	view->SetDisplayWindow( m_canvas );
    event.SetView( view );

    if ( view )
    {
        view->SetDocument( event.GetDocument() );
        view->Activate( true );
        return;
    }
    wxLogMessage( _( "a2dConnector no view was created" ) );
}

void a2dConnector::OnDisConnectView(  a2dTemplateEvent& event )
{
    a2dSmrtPtr<a2dCanvasView> theViewToDisconnect = ( a2dCanvasView* ) event.GetEventObject();

    theViewToDisconnect->Activate( false );
    if ( theViewToDisconnect->GetDocument() )
    {
        a2dDocumentEvent eventremove( wxEVT_REMOVE_VIEW, theViewToDisconnect, true, theViewToDisconnect->GetId() );
        eventremove.SetEventObject( theViewToDisconnect );
        theViewToDisconnect->GetDocument()->ProcessEvent( eventremove );
    }
    if ( m_canvas->GetDrawingPart()->GetCanvasToolContr() )
        m_canvas->GetDrawingPart()->GetCanvasToolContr()->StopAllTools();

    if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->GetBusyExit() )
    {
        if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().size() > 1 )
        {
            a2dDocumentList::const_reverse_iterator iter = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().rbegin();
            iter++;
            theViewToDisconnect->SetDocument( ( *iter ) );
        }
        else
        {
            a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateDocuments( wxT( "dummy" ), a2dREFDOC_NEW );
            a2dCanvasDocument* document = ( a2dCanvasDocument* ) a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().back().Get();
            theViewToDisconnect->SetDocument( document );
        }

        if ( m_canvas->GetDrawingPart()->GetCanvasToolContr() )
            m_canvas->GetDrawingPart()->GetCanvasToolContr()->ReStart();

		theViewToDisconnect->GetCanvasDocument()->Update( a2dCanvasObject::updatemask_force_and_count );
		m_canvas->GetDrawingPart()->SetShowObject( theViewToDisconnect->GetCanvasDocument()->GetDrawing()->GetRootObject() );
        m_canvas->GetDrawingPart()->SetMappingShowAll();
        theViewToDisconnect->Activate( true );
    }
    else
        event.Skip();
}


void a2dConnector::SetDisplayWindow( a2dCanvas* display )
{
    m_canvas = display;
}

//----------------------------------------------------------------------------
// WiresDrawingPart
//----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS( WiresDrawingPart, a2dDrawingPartTiled )

BEGIN_EVENT_TABLE( WiresDrawingPart, a2dDrawingPartTiled )
    EVT_COM_EVENT( WiresDrawingPart::OnComEvent )
    EVT_UPDATE_DRAWING( WiresDrawingPart::OnUpdate )
	EVT_ENTER_WINDOW(  WiresDrawingPart::OnEnter )
	EVT_LEAVE_WINDOW( WiresDrawingPart::OnLeave )
END_EVENT_TABLE()

WiresDrawingPart::WiresDrawingPart( const wxSize& size )
: a2dDrawingPartTiled( size )
{
    m_documentDrawStyle = RenderLAYERED_All | RenderWIREFRAME_SELECT2 | a2dCanvasGlobals->GetHabitat()->GetSelectDrawStyle();
}

WiresDrawingPart::WiresDrawingPart( int width, int height )
: a2dDrawingPartTiled( width, height )
{
    m_documentDrawStyle = RenderLAYERED_All | RenderWIREFRAME_SELECT2 | a2dCanvasGlobals->GetHabitat()->GetSelectDrawStyle();
}

WiresDrawingPart::WiresDrawingPart( int width, int height, a2dDrawer2D* drawer )
: a2dDrawingPartTiled( width, height, drawer )
{
    m_documentDrawStyle = RenderLAYERED_All | RenderWIREFRAME_SELECT2 | a2dCanvasGlobals->GetHabitat()->GetSelectDrawStyle();
}

WiresDrawingPart::WiresDrawingPart( a2dDrawer2D* drawer )
: a2dDrawingPartTiled( drawer )
{
    m_documentDrawStyle = RenderLAYERED_All | RenderWIREFRAME_SELECT2 | a2dCanvasGlobals->GetHabitat()->GetSelectDrawStyle();
}

WiresDrawingPart::WiresDrawingPart( const WiresDrawingPart& other )
: a2dDrawingPartTiled( other )
{
}

void WiresDrawingPart::SetDisplayWindow( wxWindow* display )
{
	m_drawingDisplay = display;
    //if ( m_drawingDisplay )
    //    m_drawingDisplay->SetDropTarget( new TaDnDCanvasObjectDropTarget( this )); // never used, but prevented file D&D from outside

    if ( m_drawer2D )
        m_drawer2D->SetDisplay( display );
}

void WiresDrawingPart::OnEnter( wxMouseEvent& event )
{
	a2dCanvasObjectPtr newobject = NULL;

	a2dStToolContr* stcontroller = wxStaticCast( m_toolcontroller.Get(), a2dStToolContr );
	if ( stcontroller && stcontroller->GetFrame() )
	{
		MyFrame* f =  wxDynamicCast( stcontroller->GetFrame(), MyFrame );
		if ( f && f->m_pushNewObject )
		{
			newobject = f->m_pushNewObject;
			//wxLogDebug( "now push" );

			//if previous push is still in action, pop that one first.
			if ( wxDynamicCast( stcontroller->GetFirstTool(), a2dDragNewTool ) )
			{ 
				a2dSmrtPtr< a2dBaseTool > tool;
				stcontroller->PopTool( tool );
			}
			if ( wxDynamicCast( stcontroller->GetFirstTool(), a2dDragMultiNewTool ) )
			{ 
				a2dSmrtPtr< a2dBaseTool > tool;
				stcontroller->PopTool( tool );
			}

			f->m_pushNewObject = 0;
		}
	}

	event.Skip();

	if ( !newobject )
		return;

	// simple add object to debug it
	//a2dCanvasObject* newclone = wxStaticCast( newobject->Clone(a2dObject::clone_deep), a2dCanvasObject );
	//GetDrawing()->GetRootObject()->Append( newclone );
    //return;

	//a2dCanvasObject* newclone = wxStaticCast( newobject->Clone(a2dObject::clone_deep), a2dCanvasObject );
	//GetShowObject()->Append( newclone );
    //return;

    GetDisplayWindow()->SetFocus();

    bool dragcenter = true;
    newobject->SetParent( GetShowObject() );
    newobject->SetRoot( GetDrawing() );
    a2dDragNewTool* dragnew = new a2dDragNewTool( stcontroller, newobject );
    // Taco object have pins, and maybe others, but if not normal snap behaviour is in place, else only snap pins to target features.
    if ( newobject->HasPins() )
        dragnew->SetSnapSourceFeatures( a2dRestrictionEngine::snapToPins );

    //optional
    dragnew->SetOneShot();
    dragnew->SetBoxCenterDrag( dragcenter );

    dragnew->SetStroke( a2dStroke(*wxRED, 1) );
    dragnew->SetFill( a2dFill( wxColour(0xc0, 0xff, 0xff) ) );
    stcontroller->PushTool( dragnew );

    event.Skip();
}

void WiresDrawingPart::OnLeave( wxMouseEvent& event )
{
    //if ( GetDisplayWindow() )
    //GetDisplayWindow()->SetFocus();

    event.Skip();
}


IMPLEMENT_DYNAMIC_CLASS( a2dPreviewCanvas, a2dCanvas )

BEGIN_EVENT_TABLE( a2dPreviewCanvas, a2dCanvas )
    EVT_COM_EVENT( a2dPreviewCanvas::OnComEvent )
    EVT_MOUSEWHEEL( a2dPreviewCanvas::OnWheel ) 
    EVT_MOUSE_EVENTS( a2dPreviewCanvas::OnMouseEvent )
END_EVENT_TABLE()


a2dPreviewCanvas::a2dPreviewCanvas( wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ):
    a2dCanvas( parent, id, pos, size, style )
{
    m_newObject = NULL;
}

a2dPreviewCanvas::~a2dPreviewCanvas()
{
}

void a2dPreviewCanvas::OnComEvent( a2dComEvent& event )
{
}

void a2dPreviewCanvas::OnWheel( wxMouseEvent& event )
{
}

void a2dPreviewCanvas::OnMouseEvent( wxMouseEvent& event )
{
    if ( event.LeftDown() )
    {
        a2dCanvasObject* root = m_drawing->GetRootObject();
        a2dCanvasObjectPtr newObject = m_newObject;

        if ( m_newObject )
        {
			MyFrame* f =  wxStaticCast( wxTheApp->GetTopWindow(), MyFrame );
            a2dRefMap refs;
            newObject = wxStaticCast( m_newObject->Clone(a2dObject::clone_deep, &refs ), a2dCanvasObject );
           
			f->m_pushNewObject = newObject;
        }
    }
}


class MyCanvasControl: public a2dControl
{
public:
    MyCanvasControl( double x, double y, double w, double h, wxTextCtrl* but );

    void OnMouseEvent( a2dCanvasObjectMouseEvent& event );

    wxString m_text;
    wxString m_answer;

    DECLARE_EVENT_TABLE()

};

BEGIN_EVENT_TABLE( MyCanvasControl, a2dControl )
    EVT_CANVASOBJECT_MOUSE_EVENT( MyCanvasControl::OnMouseEvent )
//    EVT_CHAR(MyButton::OnChar)
//    EVT_KEY_DOWN(MyButton::OnKeyDown)
//

END_EVENT_TABLE()

MyCanvasControl::MyCanvasControl( double x, double y, double w, double h, wxTextCtrl* but ):
    a2dControl( x, y, w, h, but )
{

    m_text = _T( "are you happy?:" );
    m_answer = _T( "of course" );
}

void MyCanvasControl::OnMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    if ( event.GetMouseEvent().LeftDown() )
    {
        wxString text = wxGetTextFromUser( m_text, m_answer );
        if ( text == _T( "no" ) )
        {
            m_text = _T( "i hate you!:" );
            m_answer = _T( "yes" );
        }
    }
    event.Skip();
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------
const long SCRIPT_lua_script = wxNewId();
const long ShapeDnD_Copy = wxNewId();
const long ShapeDnD_Paste = wxNewId();
const long ShapeDnD_Drag = wxNewId();


BEGIN_EVENT_TABLE( MyFrame, a2dDocumentFrame )
    EVT_COM_EVENT( MyFrame::OnComEvent ) 
    EVT_MENU( ID_WIREMODE, MyFrame::OnLineModeSwitch )
    EVT_MENU( ID_QUIT, MyFrame::OnQuit )
    EVT_MENU( ID_MASTERTOOL, MyFrame::OnMastertool )
    EVT_MENU_RANGE( ID_MENUFIRST, ID_MENULAST, MyFrame::OnEdittool )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_LISTBOX( ID_LISTBOX, MyFrame::OnListBox )
    EVT_MENU( wires_ABOUT, MyFrame::OnAbout )
    EVT_MENU_RANGE( wxID_FILE1, wxID_FILE9, MyFrame::OnMRUFile )
    EVT_MENU( wxID_NEW, MyFrame::OnFileNew )
    EVT_MENU( wxID_CLOSE, MyFrame::OnFileClose )
    EVT_MENUSTRINGS( MyFrame::OnSetMenuStrings )
    EVT_MENU( wxID_UNDO, MyFrame::OnUndo )
    EVT_MENU( wxID_REDO, MyFrame::OnRedo )
    EVT_ACTIVATE_VIEW_SENT_FROM_CHILD( MyFrame::OnActivateViewSentFromChild )
    EVT_MENU( ID_testthis, MyFrame::OnTest )

#if wxART2D_USE_AGGDRAWER
    EVT_MENU( ID_DRAWER_CHANGE, MyFrame::OnDrawer )
#endif //wxART2D_USE_AGGDRAWER

    EVT_MENU( MasterToolOption_0, MyFrame::OnMasterToolOption ) 
    EVT_MENU( MasterToolOption_1, MyFrame::OnMasterToolOption ) 
    EVT_MENU( MasterToolOption_2, MyFrame::OnMasterToolOption ) 
    EVT_MENU( MasterToolOption_3, MyFrame::OnMasterToolOption ) 
    EVT_MENU( MasterToolOption_4, MyFrame::OnMasterToolOption ) 
    EVT_MENU( MasterToolOption_5, MyFrame::OnMasterToolOption ) 

    EVT_MENU_RANGE( MDSCREEN_FIRST, MDSCREEN_LAST, MyFrame::FillData )

    EVT_MENU( ShapeDnD_Copy,  MyFrame::OnCopyShape)
    EVT_MENU( ShapeDnD_Paste, MyFrame::OnPasteShape)
    EVT_MENU( ShapeDnD_Drag, MyFrame::OnDragSimulate)
    EVT_UPDATE_UI( ShapeDnD_Copy, MyFrame::OnUpdatePasteShape)
    EVT_UPDATE_UI( ShapeDnD_Paste, MyFrame::OnUpdateCopyShape)

END_EVENT_TABLE()

bool MyFrame::InitToolbar( wxToolBar* toolBar )
{
    a2dMenuIdItem::InitializeBitmaps();

    toolBar->SetToolBitmapSize( wxSize( 20, 20 ) );
    wxString bitmapfile = a2dGlobals->GetImagePathList().FindValidPath( wxT( "bitmaps/Magnify.gif" ) );
    toolBar->AddTool( wxID_HELP, wxT( "Zoom Win" ), wxBitmap( wxImage( bitmapfile ) ), wxT( "Zoom Window" ) );
    bitmapfile = a2dGlobals->GetImagePathList().FindValidPath( wxT( "bitmaps/MagnifyPlus.gif" ) );
    toolBar->AddTool( ID_ZOOMIN2X, wxT( "Zoom In" ), wxBitmap( wxImage( bitmapfile ) ), wxT( "Zoom in 2x" ) );
    bitmapfile = a2dGlobals->GetImagePathList().FindValidPath( wxT( "bitmaps/MagnifyMinus.gif" ) );
    toolBar->AddTool( ID_ZOOMOUT2X, wxT( "Zoom Out" ), wxBitmap( wxImage( bitmapfile ) ), wxT( "Zoom out 2x" ) );
    bitmapfile = a2dGlobals->GetImagePathList().FindValidPath( wxT( "bitmaps/PageSetup.gif" ) );
    toolBar->AddTool( ID_ZOOMALL, wxT( "Zoom All" ), wxBitmap( wxImage( bitmapfile ) ), wxT( "Zoom All" ) );
    toolBar->AddSeparator();
    bitmapfile = a2dGlobals->GetImagePathList().FindValidPath( wxT( "bitmaps/Copy.gif" ) );
    toolBar->AddTool( ID_COPYTOOL, wxT( "Copy" ), wxBitmap( wxImage( bitmapfile ) ), wxT( "Copy" ) );
    bitmapfile = a2dGlobals->GetImagePathList().FindValidPath( wxT( "bitmaps/Cut.gif" ) );
    toolBar->AddTool( ID_DELETETOOL, wxT( "Cut" ), wxBitmap( wxImage( bitmapfile ) ), wxT( "Cut" ) );
    bitmapfile = a2dGlobals->GetImagePathList().FindValidPath( wxT( "bitmaps/Paste.gif" ) );
    toolBar->AddTool( wxID_HELP, wxT( "Paste" ), wxBitmap( wxImage( bitmapfile ) ), wxT( "Paste" ) );
    toolBar->AddSeparator();
    bitmapfile = a2dGlobals->GetImagePathList().FindValidPath( wxT( "bitmaps/Select.gif" ) );
    toolBar->AddTool( ID_SELECTTOOL, wxT( "Select Tool" ), wxBitmap( wxImage( bitmapfile ) ), wxT( "Select Tool" ) );
    bitmapfile = a2dGlobals->GetImagePathList().FindValidPath( wxT( "bitmaps/Draw.gif" ) );
    toolBar->AddTool( ID_WIRETOOL, wxT( "Wire Tool" ), wxBitmap( wxImage( bitmapfile ) ), wxT( "Wire Tool" ) );
    toolBar->AddSeparator();
    bitmapfile = a2dGlobals->GetImagePathList().FindValidPath( wxT( "bitmaps/Bold.gif" ) );
    toolBar->AddTool( ID_TEXTEDIT, wxT( "Text Tool" ), wxBitmap( wxImage( bitmapfile ) ), wxT( "Text Tool" ) );

    AddCmdToToolbar(CmdMenu_Selected_CameleonSymbol() );          
    AddCmdToToolbar(CmdMenu_Selected_CameleonDiagram() );
    AddCmdToToolbar(CmdMenu_PushInto() );
    AddCmdToToolbar(CmdMenu_PopOut() );
    AddCmdToToolbar(CmdMenu_PushTool_Cameleon() );
    AddCmdToToolbar(CmdMenu_PushTool_Port() );
    AddCmdToToolbar(CmdMenu_PushTool_Origin_OneShot() );
    AddCmdToToolbar(CmdMenu_PushTool_CameleonInst() );
    AddCmdToToolbar(CmdMenu_PushTool_CameleonInstDlg() );             
    AddCmdToToolbar( CmdMenu_PushTool_Zoom() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawPolygonL() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawPolygonL_Splined() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawPolylineL() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawPolylineL_Splined() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawRectangle() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawCircle() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawEllipse() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawEllipticArc() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawEllipticArc_Chord() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawArc() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawArc_Chord() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawText() );
    AddCmdToToolbar( CmdMenu_PushTool_Image() );
    AddCmdToToolbar( CmdMenu_PushTool_Link() );
    AddCmdToToolbar( CmdMenu_PushTool_Rotate() );
    AddCmdToToolbar( CmdMenu_PushTool_Drag() );
    AddCmdToToolbar( CmdMenu_PushTool_Copy() );
    AddCmdToToolbar( CmdMenu_PushTool_Delete() );
    AddCmdToToolbar( CmdMenu_PushTool_RecursiveEdit() );
    AddCmdToToolbar( CmdMenu_PushTool_Select() );
    AddCmdToToolbar( CmdMenu_PushTool_Select2() );
    AddCmdToToolbar( CmdMenu_Selected_SelectAll() );
    AddCmdToToolbar( CmdMenu_Selected_DeSelectAll() );
    AddCmdToToolbar( CmdMenu_Selected_Delete() );
    AddCmdToToolbar( CmdMenu_PushTool_DragMulti() );
    AddCmdToToolbar( CmdMenu_PushTool_CopyMulti() );

    toolBar->Realize();

    return true;
}

MyFrame::MyFrame( wxFrame* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    a2dDocumentFrame( true, parent, NULL, id, title, position, size, style )
{
    CreateMyMenuBar();

    CreateStatusBar( 6 );
    int widths[] = { 80, 120, -1, -1 , -1, -1 };
    SetStatusWidths( 6, widths );
    SetStatusText( _T( "Welcome to Wires sample!" ) );

    SetIcon( wxICON( mondrian ) );

    // Create the toolbar
    CreateToolBar( wxNO_BORDER | wxHORIZONTAL | wxTB_FLAT, ID_TOOLBAR );

    GetToolBar()->SetMargins( 2, 2 );

    InitToolbar( GetToolBar() );

    wxSplitterWindow* splitwin = new wxSplitterWindow( this, -1, wxDefaultPosition, wxDefaultSize, wxSP_NOBORDER );
    wxSplitterWindow* splitwin2 = new wxSplitterWindow( splitwin, -1, wxDefaultPosition, wxDefaultSize, wxSP_NOBORDER );

    wxListBox* tree = new wxListBox( splitwin2, ID_LISTBOX );
    tree->Append( wxT( "Ground" ) );
    tree->Append( wxT( "Input" ) );
    tree->Append( wxT( "Output" ) );
    tree->Append( wxT( "Resistor" ) );
    tree->Append( wxT( "Capacitor" ) );
    tree->Append( wxT( "Rectangle" ) );
    tree->Append( wxT( "Circle" ) );
    tree->Append( wxT( "Ellipse" ) );
    tree->Append( wxT( "CanvasText" ) );
    tree->Append( wxT( "Text" ) );
    tree->Append( wxT( "DirectedFlow" ) );
    tree->Append( wxT( "2Flow" ) );
    tree->Append( wxT( "Plain Port" ) );
    tree->Append( wxT( "Schematic Port" ) );
    //tree->Append( wxT( "Plain Pin" ) );
    tree->Append( wxT( "Virtual Wire" ) );
    tree->Append( wxT( "Text Reference" ) );
    tree->Append( wxT( "New Structure" ) );
    tree->Append( wxT( "Reference" ) );
    a2dCanvasGlobals->GetHabitat()->SetReverseOrder( true );

    m_preview = new a2dPreviewCanvas( splitwin2, -1, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL | wxSUNKEN_BORDER | wxALWAYS_SHOW_SB );
    m_preview->SetGrid( true );
    m_preview->SetGridX( 4.0 );
    m_preview->SetGridY( 4.0 );
    m_preview->SetGridLines( true );
    m_preview->SetGridStroke( a2dStroke( wxColour( 0xf0, 0xf0, 0xf0 ), 1 ) );
    m_preview->SetYaxis( false );//true );

    a2dLayers* docLayers = m_preview->GetDrawing()->GetLayerSetup();
    docLayers->SetStroke( 0, a2dStroke( *wxBLACK, 0.5 ) );
    docLayers->SetFill( 0, a2dFill( wxColour( 0xff, 0xff, 0xc0 ) ) );



	a2dDrawing* drawing = new a2dDrawing();
    WiresDrawingPart* part = new WiresDrawingPart( this->GetSize().GetWidth(), this->GetSize().GetHeight() );
    a2dMemDcDrawer* drawer2D = new a2dMemDcDrawer( this->GetSize().GetWidth(), this->GetSize().GetHeight() );
    part->SetDrawer2D( drawer2D );
	part->SetShowObject( drawing->GetRootObject() );

    m_canvas = new a2dCanvas( part, splitwin, -1, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL | wxSUNKEN_BORDER | wxALWAYS_SHOW_SB );
    m_canvas->GetDrawingPart()->GetDrawer2D()->SetSmallTextThreshold( 20 );

    splitwin->SplitVertically( splitwin2, m_canvas, 150 );
    splitwin2->SplitHorizontally( m_preview, tree, 150 );

    //NEXT 7 lines or optional, and depend on what you need.

    //if you do not want the origin shown
    //m_canvas->SetShowOrigin( false );

    a2dRestrictionEngine* restrict = new a2dRestrictionEngine();

    restrict->SetSnapTargetFeatures( a2dRestrictionEngine::snapToGridPos | a2dRestrictionEngine::snapToGridPosForced
                                 | a2dRestrictionEngine::snapToObjectPos | a2dRestrictionEngine::snapToPinsUnconnected
                                 | a2dRestrictionEngine::snapToObjectVertexes );

    restrict->SetSnapTargetFeatures( a2dRestrictionEngine::snapToGridPos | a2dRestrictionEngine::snapToGridPosForced |a2dRestrictionEngine::snapToPinsUnconnected );

    restrict->SetSnapSourceFeatures( a2dRestrictionEngine::snapToPinsUnconnected );


    /* for ortho line drawing
        restrict->SetSnapToFeatures( a2dRestrictionEngine::snapToGridPos
            |a2dRestrictionEngine::snapToObjectPos|a2dRestrictionEngine::snapToPinsUnconnected
            |a2dRestrictionEngine::snapToObjectVertexes
            |a2dRestrictionEngine::snapToPointAngleMod );
    */
    restrict->SetSnapGrid( 1, 1 );
    restrict->SetRotationAngle( 45 );
    a2dCanvasGlobals->GetHabitat()->SetRestrictionEngine( restrict );
    m_canvas->SetGrid( true );

    m_canvas->SetGridX( 2.0 );
    m_canvas->SetGridY( 2.0 );

    a2dRouteData::SetRaster( 2.0 );

    m_canvas->SetGridLines( true );
    //m_canvas->SetGridAtFront( true );
    m_canvas->SetGridStroke( a2dStroke( wxColour( 0xf0, 0xf0, 0xf0 ), 1 ) );
    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->SetMappingUpp( 80, 10, .25, .25 );

    m_canvas->SetYaxis( true );
    //defines the whole area in world coordinates that can be seen by scrolling
    //    m_canvas->SetScrollMaximum(-50,-40,800,600);
    //defines the number of world units scrolled when line up or down events in X
    m_canvas->SetScrollStepX( 50 );
    //defines the number of world units scrolled when line up or down events in Y
    m_canvas->SetScrollStepY( 50 );
    //do not go outide the scroll maximum
    //    m_canvas->FixScrollMaximum(true);  //what to do when resizing above ScrollMaximum is still ??

    // One object group is the root in every canvas.
	a2dCanvasObject* root = m_canvas->GetDrawing()->GetRootObject();

    m_canvas->SetShowObject( root );

    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this, false );
    //always first tool is zoom tool
    //m_contr->SetZoomFirst( true );
    m_contr->SetSnap( true );
    //m_contr->SetMousePopupMenu(0);
    //m_contr->SetDefaultBehavior(wxTC_NoAll);

    //a2dPin::SetWorldBased( true );
    //a2dPin::SetWorldBased( false );

    double pinwidth = 14; //pixels

    if ( a2dPin::GetWorldBased() )
    {
        a2dHandle::SetWorldBased( true );
        a2dCanvasGlobals->GetHabitat()->SetPinSize( 2 );
        a2dCanvasGlobals->GetHabitat()->SetHandleSize( 2 );
        pinwidth = 2; 
    }

    Pin::InitializeExtraClasses();

    //Wire Object flow

    a2dConnectionGeneratorObjectWire* ConGenObjectWire = new a2dConnectionGeneratorObjectWire();
	//ConGenObjectWire->SetRouteMethod( a2dConnectionGenerator::ManhattanEndSegments );

    //Setup a wire/pin to define which pins can connect, and with which wire
    a2dWirePolylineL* wire = new a2dWirePolylineL();
    wire->SetStroke( a2dStroke( wxColour( 25, 50, 230 ), 2, a2dSTROKE_SHORT_DASH ) );
    //!todo oops putting this, somehow prevents connecting wires to this wire.
    // this is a bug.
    // wire->SetLayer( 2 );
    wire->SetStartPinClass( a2dConnectionGeneratorObjectWire::Wire );
    wire->SetEndPinClass( a2dConnectionGeneratorObjectWire::Wire );
    ConGenObjectWire->SetConnectObject( wire );
    a2dConnectionGeneratorObjectWire::Wire->SetAngleLine( false );
    a2dConnectionGeneratorObjectWire::Object->SetAngleLine( false );

    //DIRECTIONAL WIRES ON DIRECTIONAL PINS

    a2dConnectionGeneratorDirectedFlow* dircon = new a2dConnectionGeneratorDirectedFlow();
    //Setup a wire/pin to define which  directional  pins can connect, and with which wire
    a2dWirePolylineL* wiredirect = new a2dWirePolylineL();
    a2dStroke wirestroke = a2dStroke( wxColour( 55, 250, 250 ), 2, a2dSTROKE_LONG_DASH );
    wiredirect->SetStroke( wirestroke );
    wiredirect->SetStartPinClass( a2dConnectionGeneratorDirectedFlow::WireInput );
    wiredirect->SetEndPinClass( a2dConnectionGeneratorDirectedFlow::WireOutput );
    a2dArrow* arrow2 = new  a2dArrow( 0, 0, 4, 0, 2 );
    arrow2->SetStroke( wirestroke );
    wiredirect->SetEnd( arrow2 );
    //wiredirect->SetBegin(arrow2);
    //wiredirect->SetGeneratePins( false );
    a2dConnectionGeneratorDirectedFlow::WireInput->SetAngleLine( false );
    a2dConnectionGeneratorDirectedFlow::WireOutput->SetAngleLine( false );
    a2dConnectionGeneratorDirectedFlow::ObjectInput->SetAngleLine( false );
    a2dConnectionGeneratorDirectedFlow::ObjectOutput->SetAngleLine( false );
    a2dConnectionGeneratorDirectedFlow::ObjectInput->SetConnectionGenerator( dircon );
    a2dConnectionGeneratorDirectedFlow::ObjectOutput->SetConnectionGenerator( dircon );
    a2dConnectionGeneratorDirectedFlow::WireInput->SetConnectionGenerator( dircon );
    a2dConnectionGeneratorDirectedFlow::WireOutput->SetConnectionGenerator( dircon );
    dircon->SetConnectObject( wiredirect );

    //ELEMENT WIRES AND OBJECTS
    a2dConnectionGenerator* elecon = new ElementConnectionGenerator();
    //define the pin which will be used for generating dynamic connection pins
    a2dPin* defPin = new Pin( NULL, wxT( "global" ), Pin::ElementObject, 0, 0, 0, pinwidth, pinwidth );
    defPin->SetFill( wxColour( 5, 219, 225 ) );
    defPin->SetStroke( wxColour( 5, 219, 225 ), 0 );
    a2dPin* defPin3 = new Pin( NULL, wxT( "global" ), Pin::ElementObject, 0, 0, 0, pinwidth, pinwidth );
    defPin3->SetFill( wxColour( 5, 19, 225 ) );
    defPin3->SetStroke( wxColour( 75, 19, 225 ), 0 );
    a2dPin* defPin2 = new Pin( NULL, wxT( "global" ), Pin::ElementObject, 0, 0, 0, pinwidth, pinwidth );
    defPin2->SetFill( wxColour( 5, 219, 25 ) );
    defPin2->SetStroke( wxColour( 5, 219, 225 ), 0 );
    //Setup a pin class map array to define which pins can connect, and with which wire
    a2dWirePolylineL* wireele = new a2dWirePolylineL();
    wireele->SetStroke( a2dStroke( wxColour( 5, 250, 0 ), 0.5, a2dSTROKE_SOLID ) );
    wireele->SetStartPinClass( Pin::ElementWire );
    wireele->SetEndPinClass( Pin::ElementWire );
    elecon->SetConnectObject( wireele );
    //wireele->SetGeneratePins( false );
    // define the template pins for new or rending features for pins of this class.
    Pin::ElementObject->SetPin( defPin ) ;
    Pin::ElementObject->SetPinCanConnect( defPin3 );
    Pin ::ElementObject->SetPinCannotConnect( defPin2 );
    Pin::ElementWire->SetPin( defPin ) ;
    Pin::ElementWire->SetPinCanConnect( defPin3 );
    Pin::ElementWire->SetPinCannotConnect( defPin2 );
    Pin::ElementObject->SetConnectionGenerator( elecon );
    Pin::ElementWire->SetConnectionGenerator( elecon );
    Pin::ElementObject->AddConnect( Pin::ElementWire );
    Pin::ElementObject->AddConnect( Pin::ElementObject );
    Pin::ElementWire->AddConnect( Pin::ElementObject );
    Pin::ElementWire->AddConnect( Pin::ElementWire );

    m_editMode = 3;

    if ( m_editMode == 0 )
    {
        a2dGraphicsMasterTool* tool = new a2dGraphicsMasterTool( m_contr );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
    }
    else if ( m_editMode == 1 )
    {
        a2dMasterDrawZoomFirst* tool = new a2dMasterDrawZoomFirst( m_contr );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
    }
    else if ( m_editMode == 2 )
    {
        a2dMasterDrawSelectFirst* tool = new a2dMasterDrawSelectFirst( m_contr );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
    } 
    else if ( m_editMode == 3 )
    {
        a2dMasterTagGroups3* tool = new a2dMasterTagGroups3( m_contr );
        a2dCanvasGlobals->GetHabitat()->SetHighLightStroke( a2dStroke( wxColour( 10, 222, 215 ), a2dSTROKE_SOLID ) );
        a2dCanvasGlobals->GetHabitat()->SetHighLightFill( *a2dTRANSPARENT_FILL );
		tool->SetDlgOrEditModal( true );
		tool->SetStyleDlgSimple( true );
		tool->SetDlgOrEdit( true );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
    } 
    m_menu_bar->Check( MasterToolOption_0, m_editMode == 0 );
    m_menu_bar->Check( MasterToolOption_1, m_editMode == 1 );
    m_menu_bar->Check( MasterToolOption_2, m_editMode == 2 );
    m_menu_bar->Check( MasterToolOption_3, m_editMode == 3 );

    a2dConnectionGeneratorTwoFlow* flowgen = new a2dConnectionGeneratorTwoFlow();

	//a2dCanvasGlobals->GetConnectionGenerator()->SetRouteMethod( a2dConnectionGenerator::ManhattanEndSegments );

	FillDocument( m_canvas->GetDrawing() );
    //FillDocument3( m_canvas->GetDrawing() );

}

MyFrame::~MyFrame()
{
}

void MyFrame::CreateMyMenuBar()
{

    wxMenu* file_menu = new wxMenu;
    AddCmdMenu( file_menu, CmdMenu_FileNew() );
    AddCmdMenu( file_menu, CmdMenu_FileOpen() );
    AddCmdMenu( file_menu, CmdMenu_FileClose() );
    AddCmdMenu( file_menu, CmdMenu_FileSave() );
    AddCmdMenu( file_menu, CmdMenu_FileSaveAs() );
    AddCmdMenu( file_menu, CmdMenu_Print() );
    file_menu->AppendSeparator();
    AddCmdMenu( file_menu, CmdMenu_PrintView() );
    AddCmdMenu( file_menu, CmdMenu_PreviewView() );
    AddCmdMenu( file_menu, CmdMenu_PrintDocument() );
    AddCmdMenu( file_menu, CmdMenu_PreviewDocument() );
    AddCmdMenu( file_menu, CmdMenu_PrintSetup() );

    file_menu->AppendSeparator();

    wxMenu* file_import = new wxMenu;
    file_menu->Append( wxNewId(), wxT( "import file" ), file_import, _( "Import a file" ) );
    AddCmdMenu( file_import, CmdMenu_FileImport() );

    wxMenu* file_export_as = new wxMenu;
    file_menu->Append( wxNewId(), wxT( "Export As" ), file_export_as, _( "Export in other format" ) );
    AddCmdMenu( file_export_as, CmdMenu_FileExport() );

    file_export_as->AppendSeparator();
    AddCmdMenu( file_export_as, CmdMenu_ViewAsImage_Png() );
    AddCmdMenu( file_export_as, CmdMenu_ViewAsImage_Bmp() );
    AddCmdMenu( file_export_as, CmdMenu_DocumentAsImage_Png() );
    AddCmdMenu( file_export_as, CmdMenu_DocumentAsImage_Bmp() );
#if wxART2D_USE_SVGIO
    AddCmdMenu( file_export_as, CmdMenu_ViewAsSvg() );
#endif //wxART2D_USE_SVGIO
    file_menu->AppendSeparator();

    file_menu->Append( ID_QUIT, _T( "Quit..." ), _T( "Quit program" ) );
    //AddCmdMenu( file_menu, CmdMenu_Exit() );

    m_editMenu = new wxMenu;
    m_editMenu->Append( wxID_UNDO, _( "&Undo" ) );
    m_editMenu->Append( wxID_REDO, _( "&Redo" ) );

    wxMenu* option_menu = new wxMenu;
    option_menu->Append( ID_WIREMODE, _T( "SetWireOneLine" ), _T( "Wire or manhattan lines" ), true );
#if wxART2D_USE_AGGDRAWER
    option_menu->Append( ID_DRAWER_CHANGE, wxT( "change drawer" ), _( "switch to aggdrawer or back to dcdrawer" ) );
#endif //wxART2D_USE_AGGDRAWER
    wxMenu* menumasterTool = new wxMenu;
    menumasterTool->Append( MasterToolOption_0, _( "Edit defailt master tool" ), _( "set the default mastertool" ), true );
    menumasterTool->Append( MasterToolOption_1, _( "Edit Mode zoom  first" ), _( "set the main tool in mode zoom first" ), true );
    menumasterTool->Append( MasterToolOption_2, _( "Edit Mode select first" ), _( "set the main tool in mode select first" ), true );
    menumasterTool->Append( MasterToolOption_3, _( "Edit Mode groups tags" ), _( "set the main tool using tags to group" ), true );
    menumasterTool->Append( MasterToolOption_4, _( "Edit Mode groups tags2" ), _( "set the main tool using tags to group" ), true );
    menumasterTool->Append( MasterToolOption_5, _( "Edit Mode groups tags3" ), _( "set the main tool using tags to group" ), true );
    option_menu->Append( wxNewId(), _T( "&Choose MasterTool" ), menumasterTool, _T( "&Choose MasterTool mode" ) ); 
    AddCmdMenu( option_menu, CmdMenu_SetYAxis() );

    wxMenu* netlist_menu = new wxMenu;
    netlist_menu->Append( ID_SPICE, _T( "Spice Netlist" ), _T( "Spice Netlist" ) );
    netlist_menu->Append( ID_FREEDA, _T( "fREEDA Netlist" ), _T( "fREEDA Netlist" ) );

    wxMenu* dlg_menu = new wxMenu;

#if (wxART2D_USE_LUA == 1)
    AddFunctionToMenu( SCRIPT_lua_script, dlg_menu, _T( "Run Script" ), _T( "Run Script" ), ( wxObjectEventFunctionM ) &MyFrame::OnRunScript , true );
#endif //(wxART2D_USE_LUA == 1)

    dlg_menu->Append( ID_testthis, wxT( "test this" ), _( "some testing function" ) );

    // the "About" item should be in the help menu
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append( wires_ABOUT, _T( "&About...\tF1" ), _T( "Show about dialog" ) );

    m_toolMenu = new wxMenu;
    m_toolMenu->Append( ID_MASTERTOOL, _T( "Mastertool" ), _T( "Mastertool" ) );
    m_toolMenu->Append( ID_EDITTOOL, _T( "Edittool" ), _T( "Edittool" ) );
    m_toolMenu->Append( ID_SELECTTOOL, _T( "Selecttool" ), _T( "Selecttool" ) );
    m_toolMenu->Append( ID_DELETETOOL, _T( "Deletetool" ), _T( "Deletetool" ) );
    m_toolMenu->Append( ID_COPYTOOL, _T( "Copytool" ), _T( "Copytool" ) );
    m_toolMenu->Append( ID_WIRETOOL, _T( "Wiretool" ), _T( "Wiretool" ) );
    m_toolMenu->Append( ID_DUMP, _T( "Dump doc" ), _T( "Dump" ) );
    m_toolMenu->Append( ID_ABOUTWIRES, _T( "About" ), _T( "About" ) );
    m_toolMenu->Append( ID_POLYTOOL, _T( "PolyLine Tool" ), _T( "Draw Polyline Tool" ) );


    AddCmdMenu( m_toolMenu, CmdMenu_SetMasterTool_TagGroups() );
    AddCmdMenu( m_toolMenu, CmdMenu_SetMasterTool_TagGroups_WireMode() );
    AddCmdMenu( m_toolMenu, CmdMenu_SetMasterTool_SingleClick() );

    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Zoom() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Select() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Select2() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_RecursiveEdit() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_MultiEdit() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Drag() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Copy() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Rotate() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Delete() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawText() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Image() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Link() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawRectangle() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawCircle() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawLine() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawLineScaledArrow() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawEllipse() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawEllipticArc() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawEllipticArc_Chord() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawArc() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawArc_Chord() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawPolylineL() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawPolygonL() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Property() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DragMulti() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_CopyMulti() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DragMulti() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_CopyMulti() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Measure() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawWirePolylineL() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DrawVPath() );

    m_toolMenu->AppendSeparator();
    AddCmdMenu( m_toolMenu, CmdMenu_LineBegin() );
    AddCmdMenu( m_toolMenu, CmdMenu_LineEnd() );
    AddCmdMenu( m_toolMenu, CmdMenu_LineScale() );
    AddCmdMenu( m_toolMenu, CmdMenu_SetNormalizeFactor() );

    //toolMenu->Append(MTOOL_PlaceFromLibByRef, _("add ref to object from lib"), _(""));
    //toolMenu->Append(MTOOL_PlaceFromLibByCopy, _("add object from lib"), _(""));


    m_toolMenu->AppendSeparator();
    AddCmdMenu( m_toolMenu, CmdMenu_LineBegin() );
    AddCmdMenu( m_toolMenu, CmdMenu_LineEnd() );
    AddCmdMenu( m_toolMenu, CmdMenu_LineScale() );
    AddCmdMenu( m_toolMenu, CmdMenu_SetNormalizeFactor() );

    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_DragOriginal() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Zoom() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_RenderImageZoom() );
    AddCmdMenu( m_toolMenu, CmdMenu_PushTool_Property() );

    wxMenu* drawMenu = new wxMenu;
    AddCmdMenu( drawMenu, CmdMenu_ShowDlgStructureDocument() );
    AddFunctionToMenu( MSHOW_LIBPOINTS, drawMenu, _( "Point Library" ), _( "a library used for end and begin points of lines" ), &a2dEditorFrame::OnMenu );

    wxMenu* refMenu = new wxMenu;

    AddCmdMenu( refMenu, CmdMenu_Refresh() );
    AddCmdMenu( refMenu, CmdMenu_PushInto() );
    AddCmdMenu( refMenu, CmdMenu_PopOut() );
    AddCmdMenu( refMenu, CmdMenu_NewGroup() );
    AddCmdMenu( refMenu, CmdMenu_NewPin() );

    wxMenu* performMenu = new wxMenu;
    wxMenu* selected = new wxMenu;

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
    AddCmdMenu( selected, CmdMenu_Selected_Group() );
    AddCmdMenu( selected, CmdMenu_Selected_RotateObject90Right() );
    AddCmdMenu( selected, CmdMenu_Selected_RotateObject90Left() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MinX() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MaxX() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MinY() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MaxY() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MidX() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MidY() );
    AddCmdMenu( selected, CmdMenu_Selected_ImageToRectangles() );
    AddCmdMenu( selected, CmdMenu_Selected_ToTop() );
    AddCmdMenu( selected, CmdMenu_Selected_ToBack() );
    AddCmdMenu( selected, CmdMenu_Selected_SetStyle() );
    AddCmdMenu( selected, CmdMenu_Selected_SetExtStyle() );
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
    AddCmdMenu( selected, CmdMenu_Selected_CameleonDiagram() );
    AddCmdMenu( selected, CmdMenu_Selected_FlattenCameleon() );
    AddCmdMenu( selected, CmdMenu_Selected_CloneCameleonFromInst() );

    wxMenu* ungroupObjects  = new wxMenu;
    refMenu->Append( wxNewId(), wxT( "ungroup objects" ), ungroupObjects, _( "un-group selected objects" ) );
    AddCmdMenu( ungroupObjects, CmdMenu_Selected_UnGroup() );
    AddCmdMenu( ungroupObjects, CmdMenu_Selected_UnGroupDeep() );

#if wxART2D_USE_KBOOL
    wxMenu* groups = new wxMenu;
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

    wxMenu* savelayersLoc  = new wxMenu;
    menuLayers->Append( wxNewId(), wxT( "save layerfile" ), savelayersLoc, _( "save layer file to chosen location" ) );
    {
        AddCmdMenu( savelayersLoc, CmdMenu_SaveLayers() );
        AddCmdMenu( savelayersLoc, CmdMenu_SaveLayersHome() );
        AddCmdMenu( savelayersLoc, CmdMenu_SaveLayersLastDir() );
        AddCmdMenu( savelayersLoc, CmdMenu_SaveLayersCurrentDir() );
    }
    wxMenu* loadlayersLoc  = new wxMenu;
    menuLayers->Append( wxNewId(), wxT( "load layerfile" ), loadlayersLoc, _( "load layer file from chosen location" ) );
    {
        AddCmdMenu( loadlayersLoc, CmdMenu_LoadLayers() );
        AddCmdMenu( loadlayersLoc, CmdMenu_LoadLayersHome() );
        AddCmdMenu( loadlayersLoc, CmdMenu_LoadLayersLastDir() );
        AddCmdMenu( loadlayersLoc, CmdMenu_LoadLayersCurrentDir() );
    }
    AddCmdMenu( menuLayers, CmdMenu_SetLayerDlg() );
    AddCmdMenu( menuLayers, CmdMenu_SetLayerDlgModeless() );
    AddCmdMenu( menuLayers, CmdMenu_SetTargetDlg() );

    AddFunctionToMenu( Layer_view, menuLayers, _( "Show layer view" ), _( "layers of this document" ), &a2dEditorFrame::OnMenu, true );

    wxMenu* layerDlgInit  = new wxMenu;
    menuLayers->Append( wxNewId(), wxT( "LayerDlg" ), layerDlgInit, _( "Show LayerDlg" ) );
    AddCmdMenu( layerDlgInit, CmdMenu_ShowDlgLayersDrawing() );
    AddCmdMenu( layerDlgInit, CmdMenu_ShowDlgLayersDocument() );
    AddCmdMenu( layerDlgInit, CmdMenu_ShowDlgLayersGlobal() );

    wxMenu* orderDlgInit  = new wxMenu;
    menuLayers->Append( wxNewId(), wxT( "OrderDlg" ), orderDlgInit, _( "Show OrderDlg" ) );
    AddCmdMenu( orderDlgInit, CmdMenu_ShowDlgLayerOrderDocument() );
    AddCmdMenu( orderDlgInit, CmdMenu_ShowDlgLayerOrderGlobal() );

    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgPathSettings() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgSettings() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgTrans() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgTools() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgStyle() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgSnap() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgStructureDocument() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgIdentify() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgMeasure() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgCoordEntry() );

    //AddCmdMenu( dlg_menu, CmdMenu_InsertGroupRef() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgLayersDocument() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgGroups() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgPropEdit() );

    wxMenu *menuClipboard = new wxMenu;
    menuClipboard->Append( ShapeDnD_Copy, wxT("&Copy\tCtrl-C"));
    menuClipboard->Append( ShapeDnD_Paste, wxT("&Paste\tCtrl-V"));
    menuClipboard->Append( ShapeDnD_Drag, wxT("&Drag"));

    wxMenu* docsMenu = new wxMenu;
    docsMenu->Append( MDSCREEN_0, wxT( "0" ), _( "0" ) );
    docsMenu->Append( MDSCREEN_1, wxT( "1" ), _( "1" ) );
    docsMenu->Append( MDSCREEN_2, wxT( "2" ), _( "2" ) );
    docsMenu->Append( MDSCREEN_3, wxT( "3" ), _( "3" ) );
    docsMenu->Append( MDSCREEN_4, wxT( "4" ), _( "4" ) );

    m_menu_bar = new wxMenuBar();
    m_menu_bar->Append( file_menu, _T( "File" ) );
    m_menu_bar->Append( m_editMenu, _( "&Edit" ) );
    m_menu_bar->Append( option_menu, _T( "Options" ) );
    m_menu_bar->Append( m_toolMenu, _T( "Tools" ) );
    m_menu_bar->Append( netlist_menu, _T( "Netlist" ) );

    m_menu_bar->Append( refMenu, _( "&Action" ) );
    m_menu_bar->Append( performMenu, _( "&Perform Operation" ) );
    m_menu_bar->Append( menuSettings, _( "Settings" ) );
    m_menu_bar->Append( menuLayers, _( "Layers" ) );
    m_menu_bar->Append( dlg_menu, _( "&Dialogs" ) );
    m_menu_bar->Append( drawMenu, _( "&Structs&lib" ) );
    m_menu_bar->Append( menuClipboard, wxT("&Clipboard"));
    m_menu_bar->Append( docsMenu, _( "&DemoScreens" ) );
    m_menu_bar->Append( menuHelp, _T( "Help" ) );

    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( file_menu );

    SetMenuBar( m_menu_bar );
}

void MyFrame::OnComEvent( a2dComEvent& event )
{
    if ( event.GetId() == a2dStToolContr::sm_showCursor && GetStatusBar() )
    {
        a2dStToolContr* contr = wxStaticCast( event.GetEventObject(), a2dStToolContr );
        wxString str;
/*
        wxPoint mpos = contr->GetMousePosition();

        double mouse_worldx, mouse_worldy;
        contr->GetDrawingPart()->MouseToToolWorld( mpos.x, mpos.y, mouse_worldx, mouse_worldy );

        wxString unitmeasure = contr->GetDrawingPart()->GetDrawing()->GetUnits();
        double scaleToMeters = contr->GetDrawingPart()->GetDrawing()->GetUnitsScale();

        double multi = 1;
        a2dDoMu::GetMultiplierFromString( unitmeasure, multi );

        mouse_worldx *= scaleToMeters / multi;
        mouse_worldy *= scaleToMeters / multi;

        wxString str;
        str.Printf( _T( "%6.3f,%6.3f" ), mouse_worldx, mouse_worldy );
        str = str + _T( " " ) + unitmeasure;
        SetStatusText( str, 0 );
*/

        SetStatusText( contr->GetStatusStrings()[0], 0 );
        SetStatusText( contr->GetStatusStrings()[1], 1 );
        SetStatusText( contr->GetStatusStrings()[2], 2 );
        SetStatusText( contr->GetStatusStrings()[3], 3 );
        SetStatusText( a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->GetRouteMethodAsString(), 4 );
        wxUint16 lay = a2dCanvasGlobals->GetHabitat()->GetLayer();
        SetStatusText( contr->GetDrawingPart()->GetDrawing()->GetLayerSetup()->GetName( lay ), 5 );
    }
    else if ( event.GetId() == a2dHabitat::sig_changedLayer )
    {
        a2dStToolContr* contr = wxStaticCast( event.GetEventObject(), a2dStToolContr );
        wxUint16 lay = a2dCanvasGlobals->GetHabitat()->GetLayer();
        SetStatusText( contr->GetDrawingPart()->GetDrawing()->GetLayerSetup()->GetName( lay ), 5 );
    }

    else
        event.Skip();
}
 
void MyFrame::OnMRUFile( wxCommandEvent& event )
{
    event.Skip();
}

void MyFrame::ConnectDocument( a2dCanvasDocument* doc, a2dCanvasView* drawer )
{
    if ( !doc )
        return;

//todo
/*
    a2dViewList allviews;
    doc->ReportViews( &allviews );

    if ( !drawer || wxNOT_FOUND == allviews.IndexOf( drawer ) )
    {
        if ( m_view )  //document change means: this view will now belong to the new document
            m_view->SetDocument( doc );
    }
    else
    {
        m_canvas->SetCanvasView( drawer );
    }

    a2dView* view = m_view;

    if ( view )
    {
        view->SetDisplayWindow( NULL );
    }

    view->SetDisplayWindow( m_canvas );
    view->Update();
    m_canvas->SetView( view );

    if ( view->GetDocument() )
    {
        //maybe something to change the title
    }
    view->Activate( true );
*/
}

void MyFrame::OnFileNew( wxCommandEvent& event )
{
    // first close old document
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileClose();
    event.Skip();
}

void MyFrame::OnFileClose( wxCommandEvent& event )
{
    // first create a NEW document
    // This calls void a2dConnector::OnPostCreateDocument( a2dTemplateEvent& event )
    a2dDocument* documentOld = m_view->GetDocument();
    a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateDocuments( wxT( "dummy" ), a2dREFDOC_NEW );
    a2dCanvasDocument* document = ( a2dCanvasDocument* ) a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().back().Get();

    //now close the old, which has the view removed already.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->SetCurrentDocument( documentOld );
    event.Skip();
}

void MyFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
    Close( true );
}

void MyFrame::OnMastertool( wxCommandEvent& WXUNUSED( event ) )
{
    if ( wxDynamicCast( m_contr->GetFirstTool(), a2dDragNewTool ) )
    {
        a2dSmrtPtr< a2dBaseTool > tool;
        m_contr->PopTool( tool );
    }

    m_contr->PushTool( new a2dGraphicsMasterTool( m_contr ) );
}

void MyFrame::OnEdittool( wxCommandEvent& event )
{
    switch( event.GetId() )
    {
        case ID_EDITTOOL:
            m_contr->PushTool( new a2dRecursiveEditTool( m_contr ) );
            break;
        case ID_SELECTTOOL:
            m_contr->PushTool( new a2dSelectTool( m_contr ) );
            break;
        case ID_DELETETOOL:
            m_contr->PushTool( new a2dDeleteTool( m_contr ) );
            break;
        case ID_COPYTOOL:
            m_contr->PushTool( new a2dCopyTool( m_contr ) );
            break;
        case ID_WIRETOOL:
        {
            a2dDrawWirePolylineLTool* tool = new a2dDrawWirePolylineLTool( m_contr );
            tool->SetStroke( a2dStroke( *wxGREEN, 3.0 ) );
            m_contr->PushTool( tool );
        }
        break;
        case ID_TEXTEDIT:
        {
            a2dSubDrawMasterTool* tool = new a2dSubDrawMasterTool( m_contr );
            tool->SetStroke( a2dStroke( *wxBLACK, 1.0 ) );
            m_contr->PushTool( tool );
        }
        break;
        case ID_POLYTOOL:
        {
            a2dDrawPolylineLTool* tool = new a2dDrawPolylineLTool( m_contr );
            tool->SetStroke( a2dStroke( *wxRED, 1.0 ) );
            m_contr->PushTool( tool );
        }
        break;
        case ID_ZOOMOUT2X:
            m_contr->Zoomout2();
            break;
        case ID_ZOOMIN2X:
            m_contr->Zoomin2();
            break;
        case ID_ZOOMALL:
            m_contr->Zoomout();
            break;
        case ID_DUMP:
#if DEBUG
            m_canvas->GetShowObject()->Dump( 4 );
#endif
            break;
        case ID_ABOUTWIRES:
            wxMessageBox( wxT( "Wires Demo\n" )
                          wxT( "(c) 2004 Erik van der Wal\n" )
                          wxT( "\n" )
                          wxT( "Using:\n" )
                          wxT( "- wxArt2D (c) Klaas Holwerda\n" )
                          wxT( "  www.sourceforge.net/projects/wxart2d\n" )
                          wxT( "- Icons (c) 1998 Dean S. Jones\n" )
                          wxT( "  dean@gallant.com www.gallant.com/icons.htm" ) );
            break;
        case ID_SPICE:
        {
            wxString msg;
            a2dCanvasObject* root = m_canvas->GetDrawing()->GetRootObject();
            for( a2dCanvasObjectList::iterator iter = root->GetChildObjectList()->begin(); iter != root->GetChildObjectList()->end(); ++iter )
            {
                a2dPin* pin = wxDynamicCast( ( *iter ).Get(), a2dPin );
                Element* elm = wxDynamicCast( ( *iter ).Get(), Element );
                if ( elm )
                {
                    wxString line = elm->GetNetlistLine( wxT( "SPICE" ) );
                    if ( !line.IsEmpty() )
                        msg += line + wxT( "\n" );
                }
            }
            wxMessageBox( msg );
        }
        break;
        case ID_FREEDA:
        {
            // just a copy of spice above with one difference
            wxString msg;
            a2dCanvasObject* root = m_canvas->GetDrawing()->GetRootObject();
            a2dCanvasObjectList* children = root->GetChildObjectList();
            for( a2dCanvasObjectList::iterator iter = root->GetChildObjectList()->begin(); iter != root->GetChildObjectList()->end(); ++iter )
            {
                Element* elm = wxDynamicCast( ( *iter ).Get(), Element );
                if ( elm )
                {
                    // the difference is in the next line
                    wxString line = elm->GetNetlistLine( wxT( "FREEDA" ) );
                    if ( !line.IsEmpty() )
                        msg += line + wxT( "\n" );
                }
            }
            wxMessageBox( msg );
        }
        break;
    }
}

void MyFrame::OnListBox( wxCommandEvent& event )
{
    wxListBox* lb = ( wxListBox* ) event.GetEventObject();
    wxString sel = lb->GetStringSelection();

    if ( sel.IsEmpty() )
        return;

	a2dCanvasObject* doc = m_preview->GetDrawing()->GetRootObject();
    a2dCanvasObject* obj = NULL;
    doc->ReleaseChildObjects();

    if ( wxDynamicCast( m_contr->GetFirstTool(), a2dDragNewTool ) )
    {
        a2dSmrtPtr< a2dBaseTool > tool;
        m_contr->PopTool( tool );
    }

    a2dCanvasObjectPtr newobject = NULL;

    if ( sel == wxT( "Ground" ) )
    {
        newobject = new Ground( wxT( "0" ) );
    }
    if ( sel == wxT( "Input" ) )
    {
        newobject = new Input( wxT( "Vin" ) );
    }
    if ( sel == wxT( "Output" ) )
    {
        newobject = new Output( wxT( "Vout" ) );
    }
    else if ( sel == wxT( "Resistor" ) )
    {
        newobject = new Resistor( wxT( "R8" ), wxT( "10k" ) );
    }
    else if ( sel == wxT( "Capacitor" ) )
    {
        newobject = new Capacitor( wxT( "C9" ), wxT( "100p" ) );
    }
    else if ( sel == wxT( "Rectangle" ) )
    {
        newobject = new a2dRect( 0, 0, 10, 10 );
    }
    else if ( sel == wxT( "Circle" ) )
    {
        newobject = new a2dCircle( 0, 0, 10 );
    }
    else if ( sel == wxT( "Ellipse" ) )
    {
        newobject = new a2dEllipse( 0, 0, 10, 20 );
    }
    else if ( sel == wxT( "CanvasText" ) )
    {
        //newobject = new a2dText( wxT("editable text"), 0.0, 0.0, a2dFont( 3.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
        newobject = new a2dText( wxT( "editable text" ), 0.0, 0.0, a2dFont( 3.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
        ( ( a2dText* )newobject.Get() )->SetBackGround( true );
    }
    else if ( sel == wxT( "Text" ) )
    {
        newobject = new a2dRect( 0, 0, 10, 10 );
        Text* tt = new Text( wxT( "whatever" ) );
        newobject->Append( tt );
        tt->SetResizeOnChildBox( true );
        newobject->SetSubEdit( true );
        tt->SetSubEditAsChild( true );
    }
    else if ( sel == wxT( "DirectedFlow" ) )
    {
        newobject = new a2dRect( 0, 0, 20, 20 );
        newobject->SetGeneratePins( false );
        a2dPin* pin = new a2dPin( newobject, wxT( "PinIn" ), a2dConnectionGeneratorDirectedFlow::ObjectInput, 0, 10 );
        pin->SetFill( wxColour( 125, 29, 25 ) );
        newobject->Append( pin );
        pin = new a2dPin( newobject, wxT( "PinOut" ), a2dConnectionGeneratorDirectedFlow::ObjectOutput, 20, 10 );
        pin->SetFill( wxColour( 125, 29, 125 ) );
        newobject->Append( pin );
        pin = new a2dPin( newobject, wxT( "InOut" ), a2dConnectionGeneratorDirectedFlow::ObjectInput, 10, 20 );
        newobject->Append( pin );
        pin = new a2dPin( newobject, wxT( "InOut" ), a2dConnectionGeneratorDirectedFlow::ObjectOutput, 10, 0 );
        newobject->Append( pin );
    }
    else if ( sel == wxT( "2Flow" ) )
    {
        newobject = new a2dRect( 0, 0, 20, 20 );
	    newobject->SetStroke( a2dStroke( *wxGREEN, 0.5 ) );
		newobject->SetFill( a2dFill( wxColour( 233, 24, 155 ) ) );
        newobject->SetGeneratePins( false );
        a2dPin* pin = new a2dPin( newobject, wxT( "PinIn" ), a2dConnectionGeneratorTwoFlow::FlowAInput, 0, 10 );
        pin->SetFill( wxColour( 125, 29, 25 ) );
        newobject->Append( pin );
        pin = new a2dPin( newobject, wxT( "PinOut" ), a2dConnectionGeneratorTwoFlow::FlowAOutput, 20, 10 );
        pin->SetFill( wxColour( 125, 29, 125 ) );
        newobject->Append( pin );
        //pin = new a2dPin( newobject, wxT( "InOut" ), a2dConnectionGeneratorObjectWire::Object, 10, 20 );
        pin = new a2dPin( newobject, wxT( "InOut" ), a2dConnectionGeneratorTwoFlow::FlowBInput, 10, 20 );
        newobject->Append( pin );
        //pin = new a2dPin( newobject, wxT( "InOut" ), a2dConnectionGeneratorObjectWire::Object, 10, 0 );
        pin = new a2dPin( newobject, wxT( "InOut" ), a2dConnectionGeneratorTwoFlow::FlowBOutput, 10, 0 );
        newobject->Append( pin );
    }
    else if ( sel == wxT( "Schematic Port" ) )
    {
        a2dPort* sympin = new a2dPort( NULL, 0, 0, wxT("port_") );
        sympin->GetPin()->SetPinClass( Pin::ElementObject );
        newobject = sympin;
	    newobject->SetStroke( a2dStroke( *wxGREEN, 0 ) );
		newobject->SetFill( a2dFill( wxColour( 0, 24, 255 ) ) );
    }
    else if ( sel == wxT( "Plain Port" ) )
    {
        a2dPort* sympin = new a2dPort( NULL,  0, 0, wxT("port_") );
        //sympin->GetPin()->SetPinClass( a2dConnectionGeneratorObjectWire::Object );
        //sympin->GetPin()->SetPinClass( a2dPinClass::Standard );
        newobject = sympin;
	    newobject->SetStroke( a2dStroke( wxColour( 0, 24, 255 ), 0 ) );
		newobject->SetFill( a2dFill( wxColour( 255, 0, 0 ) ) );
    }
/*
    else if ( sel == wxT( "Plain Pin" ) )
    {
        wxString name = wxGetTextFromUser( _( "give name of new pin:" ) );
        a2dPin* pin = new a2dPin( m_contr->GetDrawingPart()->GetShowObject(), name, a2dPinClass::Standard, 0,0,0 );
        pin->SetPinClass( a2dConnectionGeneratorObjectWire::Object );
        newobject = pin;
	    //newobject->SetStroke( a2dStroke( *wxGREEN, 0.5 ) );
		//newobject->SetFill( a2dFill( wxColour( 233, 24, 155 ) ) );
    }
*/
    else if ( sel == wxT( "Virtual Wire" ) )
    {
        static int nr = 1;
        nr++;
        wxString strNr = wxString::Format("%ld", nr);

        //a2dVirtWire* virtWire = new a2dVirtWire( NULL, wxT("virtWire_") + strNr, a2dConnectionGeneratorObjectWire::Object );
        a2dVirtWire* virtWire = new a2dVirtWire( NULL, wxT("virtWire_") + strNr, a2dPinClass::Standard );
        newobject = virtWire;
	    //newobject->SetStroke( a2dStroke( *wxGREEN, 0.5 ) );
		//newobject->SetFill( a2dFill( wxColour( 233, 24, 155 ) ) );
    }
    else if ( sel == wxT( "Text Reference" ) )
    {
        a2dCanvasObjectsChooseDialog groups( this, m_contr->GetDrawingPart()->GetDrawing(), false);
        if ( groups.ShowModal() == wxID_OK )
        {
            a2dNameReference* nameref = new a2dNameReference( 0, 0, groups.GetCanvasObject(), wxT(""),0, a2dFont( 2.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
            nameref->SetGeneratePins( false );
            newobject = nameref;
        }
    }
    else if ( sel == wxT( "Reference" ) )
    {
        a2dCanvasObjectsChooseDialog groups( this, m_contr->GetDrawingPart()->GetDrawing(), false);
        if ( groups.ShowModal() == wxID_OK )
        {
            a2dCanvasObjectReference* ref = new a2dCanvasObjectReference( 0, 0, groups.GetCanvasObject() );
            ref->SetGeneratePins( false );
            newobject = ref;
        }
    }
    else if ( sel == wxT( "New Structure" ) )
    {
        a2dCanvasObjectPtr refTo = new a2dCanvasObject();
        wxString nameIt = "New Structure";
        nameIt << wxGenNewId();
        refTo->SetName( nameIt );
        a2dText* empty = new a2dText( wxT( "Empty Structure" ), 0.0, 0.0, a2dFont( 3.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
        refTo->Append( empty );
        refTo->SetRoot( m_contr->GetDrawingPart()->GetDrawing() );
        a2dNameReference* nameref = new a2dNameReference( 0, 0, refTo, wxT(""),0, a2dFont( 2.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
        nameref->SetGeneratePins( false );
        newobject = nameref;
    }

    if ( newobject )
    {
		MyFrame* f =  wxStaticCast( wxTheApp->GetTopWindow(), MyFrame );
		f->m_pushNewObject = newobject;
        newobject->SetParent(  m_contr->GetDrawingPart()->GetShowObject() );

		// additional clone for the preview canvas
        a2dRefMap refs;
		m_preview->m_newObject = wxStaticCast( newobject->Clone(a2dObject::clone_deep, &refs ), a2dCanvasObject );
        doc->Append( m_preview->m_newObject );
        //doc->GetRootObject()->Append( newobject );
    }
    m_preview->SetMappingShowAll();
}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    if ( event.CanVeto() )
    {
        if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->Exit( !event.CanVeto() ) )
        {
            event.Veto( true );
            return;
        }
    }
    else
        a2dDocviewGlobals->GetDocviewCommandProcessor()->Exit( true );
    // The above Exit before THE topwindow, do first close other top windows.
    Destroy();
}

void MyFrame::FillDocument( a2dDrawing* drawing )
{
	a2dCanvasObject* root = drawing->GetRootObject();

    root->ReleaseChildObjects();

    //Start filling docume
    a2dLayers* docLayers = drawing->GetLayerSetup();
    docLayers->SetStroke( 0, a2dStroke( *wxBLACK, 0.5 ) );
    docLayers->SetFill( 0, a2dFill( wxColour( 0xff, 0xff, 0xc0 ) ) );

    a2dCanvasObject* vin = new Input( wxT( "Vin" ) );
    vin->SetPosXY( 80.0, 0.0 );
    root->Append( vin );

    a2dCanvasObject* r1 = new Resistor( wxT( "R1" ), wxT( "100k" ) );
    r1->SetPosXY( 100.0, 0.0 );
    root->Append( r1 );

    a2dCanvasObject* c1 = new Capacitor( wxT( "C1" ), wxT( "10p" ) );
    c1->SetPosXY( 120.0, -20.0 );
    c1->SetRotation( 90.0 );
    root->Append( c1 );

    a2dCanvasObject* gnd = new Ground( wxT( "0" ) );
    gnd->SetPosXY( 120.0, -40.0 );
    root->Append( gnd );

    a2dCanvasObject* r2 = new Resistor( wxT( "R2" ), wxT( "100k" ) );
    r2->SetPosXY( 140.0, 0.0 );
    root->Append( r2 );

    a2dCanvasObject* c2 = new Capacitor( wxT( "C2" ), wxT( "10p" ) );
    c2->SetPosXY( 160.0, -20.0 );
    c2->SetRotation( 90.0 );
    root->Append( c2 );

    gnd = new Ground( wxT( "0" ) );
    gnd->SetPosXY( 160.0, -40.0 );
    root->Append( gnd );

    a2dCanvasObject* vout = new Output( wxT( "Vout" ) );
    vout->SetPosXY( 170.0, 0.0 );
    root->Append( vout );

    wxTextCtrl* but = new wxTextCtrl( m_canvas, -1, _T( "apenoot \n schei uit" ) , wxPoint( 0, 0 ), wxSize( 100, 100 ), wxTE_MULTILINE );
    but->Show( false );
    root->Prepend( new MyCanvasControl( 180, -20, 15, 15, but ) );

    a2dText* text = new a2dText( wxT( "editable normal mpawot\n text" ), 130, -50, a2dFont( 3.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL ) );
    text->SetStroke( a2dStroke( *wxRED, 1 ) );
    text->SetFill( a2dFill( wxColour( 0xc0, 0xff, 0xff ) ) );
    root->Prepend( text );

    wxString default_text(
        "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Cras sit amet\n"
        "dui.  Nam sapien. Fusce vestibulum ornare metus. Maecenas ligula orci,\n"
        "consequat vitae, dictum nec, lacinia non, elit. Aliquam iaculis\n"
        "molestie neque. Maecenas suscipit felis ut pede convallis malesuada.\n"
        "Aliquam erat volutpat. Nunc pulvinar condimentum nunc. Donec ac sem vel\n"
        "leo bibendum aliquam. Pellentesque habitant morbi tristique senectus et\n"
        "netus et malesuada fames ac turpis egestas.\n\n"

        "Sed commodo. Nulla ut libero sit amet justo varius blandit. Mauris vitae\n"
        "nulla eget lorem pretium ornare. Proin vulputate erat porta risus.\n"
        "Vestibulum malesuada, odio at vehicula lobortis, nisi metus hendrerit\n"
        "est, vitae feugiat quam massa a ligula. Aenean in tellus. Praesent\n"
        "convallis. Nullam vel lacus.  Aliquam congue erat non urna mollis\n"
        "faucibus. Morbi vitae mauris faucibus quam condimentum ornare. Quisque\n"
        "sit amet augue. Morbi ullamcorper mattis enim. Aliquam erat volutpat.\n"
        "Morbi nec felis non enim pulvinar lobortis.  Ut libero. Nullam id orci\n"
        "quis nisl dapibus rutrum. Suspendisse consequat vulputate leo. Aenean\n"
        "non orci non tellus iaculis vestibulum. Sed neque.\n\n" );


#if wxART2D_USE_FREETYPE
    a2dFont lorem( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Regular.ttf" ) ), 3.0 );
    //a2dFont lorem( a2dGlobals->GetFontPathList().FindValidPath( wxT("/LiberationSerif-Regular.ttf") ),3.0 );
    a2dText* text3 = new a2dText( default_text, 80, -70, lorem );
    text3->SetStroke( a2dStroke( *wxBLACK, 1 ) );
    text3->SetFill( a2dFill( wxColour( 0xc0, 0xff, 0xff ) ) );
    text3->SetNextLineDirection( false );
    root->Prepend( text3 );
#endif //wxART2D_USE_FREETYPE

    m_canvas->SetShowObject( root );

}

//#define WITHPARS 1

void MyFrame::FillDocument2( a2dDrawing* drawing )
{
    a2dPinClass* pclass = a2dConnectionGeneratorObjectWire::Object;
    pclass = a2dPinClass::Standard;
	a2dCanvasObject* root = drawing->GetRootObject();
    root->ReleaseChildObjects();

    //Start filling document now.
    a2dLayers* docLayers = drawing->GetLayerSetup();
    docLayers->SetStroke( 0, a2dStroke( *wxBLACK, 0.5 ) );
    docLayers->SetFill( 0, a2dFill( wxColour( 0xff, 0xff, 0xc0 ) ) );

    // FIRST create a cameleon, with a symbol and a diagram

    a2dCameleon::SetCameleonRoot( root );
    a2dCameleon* cam1 = new a2dCameleon( wxT("aap") );
    cam1->AddToRoot();
    cam1->AddDoubleParameter( wxT("firstPar"), 12.345 );
    cam1->AddBoolParameter( wxT("secondPar"), true );
    a2dSymbol* sym1 = new a2dSymbol( cam1 );
    // how does it look like as place holder
    sym1->Append( new a2dRect( 0, -2.5, 20, 5 ) );    
    // define the symbol drawing
    sym1->AppendToDrawing( new a2dRect( 0, -2.5, 10, 15 ) );    
    sym1->AppendToDrawing( new a2dCircle( 5, 2, 3 ) );    
    a2dPort* sympin;
    sym1->AppendToDrawing( sympin = new a2dPort( NULL, 0, 0, wxT("aap_1") ) );
    sympin->GetPin()->SetPinClass( pclass );

    sym1->AppendToDrawing( sympin = new a2dPort( NULL, 10, 0, wxT("aap_2") ) );
    sympin->SetRotation( 180 );
    sympin->GetPin()->SetPinClass( pclass );
    //sym1->Append( new a2dPort( sym1, wxT("aap_1"), a2dConnectionGeneratorDirectedFlow::ObjectInput, 0, 0 ) );
    //sym1->Append( new a2dPort( sym1, wxT("aap_2"), a2dConnectionGeneratorDirectedFlow::ObjectOutput, 100, 0 ) );

#ifdef WITHPARS
    //make some of the parameters visible inside the symbol
    a2dPropertyId* propid = cam1->GetParameterId( wxT("firstPar") );
    if ( propid )
        sym1->AppendToDrawing( new a2dVisibleParameter( cam1, propid, 0.0, -3.0, true, a2dFont( 2.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) ) );
    propid = cam1->GetParameterId( wxT("secondPar") );
    if ( propid )
        sym1->AppendToDrawing( new a2dVisibleParameter( cam1, propid, 0.0, -5.5, true, a2dFont( 2.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) ) );
#endif

    cam1->AddAppearance( sym1 );

    a2dDiagram* diagram1 = new a2dDiagram( cam1 );
    diagram1->SetName( wxT("MyCircuit") );
    diagram1->AppendToDrawing( new a2dRect( 0, -2.5, 10, 15 ) );    
    diagram1->AppendToDrawing( new a2dRect( 15, -7.5, 30, 25 ) );    
    diagram1->AppendToDrawing( new a2dPort( NULL, 0,0,wxT("aap_1"), pclass ) );
    diagram1->AppendToDrawing( new a2dPort( NULL, 0, 45, wxT("aap_2"), pclass ) );
    cam1->AddAppearance( diagram1 );

    // SECOND create a cameleon, with a symbol and a diagram

    a2dCameleon::SetCameleonRoot( root );
    a2dCameleon* cam2 = new a2dCameleon( wxT("noot") );
    cam2->AddToRoot();
    a2dSymbol* sym2 = new a2dSymbol( cam2 );
    sym2->AppendToDrawing( new a2dRect( 0, -4, 15, 8 ) );
    sym2->AppendToDrawing( sympin = new a2dPort( NULL, 0, 0, wxT("noot_1") ) );
    sympin->GetPin()->SetPinClass( pclass );
    sym2->AppendToDrawing( sympin = new a2dPort( NULL, 15, 0, wxT("noot_2") ) );
    sympin->SetRotation( 180 );
    sympin->GetPin()->SetPinClass( pclass );
    //sym2->Append( new a2dPort( sym2, wxT("noot_1"), a2dConnectionGeneratorDirectedFlow::ObjectInput, 0, 0 ) );
    //sym2->Append( new a2dPort( sym2, wxT("noot_2"), a2dConnectionGeneratorDirectedFlow::ObjectOutput, 150, 0 ) );
    cam2->AddAppearance( sym2 );

    a2dDiagram* diagram2 = new a2dDiagram( cam2 );
    diagram2->SetName( wxT("MyOtherCircuit") );
    diagram2->AppendToDrawing( new a2dRect( 0, -2.5, 20, 25 ) );    
    diagram2->AppendToDrawing( new a2dCircle( 25, -7.5, 25 ) );    
    diagram2->AppendToDrawing( new a2dPort( NULL, 0,0,wxT("noot_1"), pclass ) );
    diagram2->AppendToDrawing( new a2dPort( NULL, 40, 5, wxT("noot_2"), pclass ) );
    diagram2->SetName( "diagram2" );
    cam2->AddAppearance( diagram2 );

    // add the two to the drawing.
    root->Prepend( cam1 );
    root->Prepend( cam2 );

    // place the two in a new group.

    a2dCanvasObject* gr = new a2dCanvasObject(0,0);
    root->Prepend( gr );

    a2dCameleonInst* ref1 = new a2dCameleonInst( 2, 5, sym1 ); 
    a2dCameleonInst* ref2 = new a2dCameleonInst( 20, 5, sym2 );
    ref1->SetName( "ref1" );
    ref2->SetName( "ref2" );
    gr->Prepend( ref1 );
    gr->Prepend( ref2 );
    gr->SetName( "schematic" );

    a2dCameleonInst* ref3 = new a2dCameleonInst( 52.2, 5, diagram2 );
    ref3->SetName( "ref3" );
    gr->Prepend( ref3 );

    //a2dNameReference* nameref = new a2dNameReference( 6.2, -5, diagram1, wxT(""),0, a2dFont( 2.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );

    //gr->Prepend( nameref );

    m_canvas->SetShowObject( gr );
}

void MyFrame::FillDocument3( a2dDrawing* drawing )
{
	a2dCanvasObject* root = drawing->GetRootObject();
    root->ReleaseChildObjects();

    //Start filling document now.
    a2dLayers* docLayers = drawing->GetLayerSetup();
    docLayers->SetStroke( 0, a2dStroke( *wxBLACK, 0.5 ) );
    docLayers->SetFill( 0, a2dFill( wxColour( 0xff, 0xff, 0xc0 ) ) );
/*
    a2dDrawing* subdraw = new a2dDrawing( 100, 200 );
    subdraw->GetRootObject()->Append( new a2dRect( 0, -4, 15, 8 ) );
    subdraw->GetRootObject()->Append( new a2dPin( subdraw, wxT("noot_1"), a2dConnectionGeneratorObjectWire::Object, 0, 0 ) );
    subdraw->GetRootObject()->Append( new a2dPin( subdraw, wxT("noot_2"), a2dConnectionGeneratorObjectWire::Object, 15, 0 ) );
    root->Prepend( subdraw );

    a2dDrawing* subdraw2 = new a2dDrawing( 100, 300 );
    subdraw2->SetDrawAsSymbol( true );
    subdraw2->GetRootObject()->Append( new a2dRect( 0, -4, 15, 8 ) );
    subdraw2->GetRootObject()->Append( new a2dPin( subdraw2, wxT("noot_1"), a2dConnectionGeneratorObjectWire::Object, 0, 0 ) );
    subdraw2->GetRootObject()->Append( new a2dPin( subdraw2, wxT("noot_2"), a2dConnectionGeneratorObjectWire::Object, 15, 0 ) );
    subdraw2->GetRootObject()->Append( new a2dCircle( 0, 0, 10 ) );
    subdraw2->GetRootObject()->Append( new a2dRect( 0, -8, 25, 8 ) );
    root->Prepend( subdraw2 );

    a2dDrawing* subdraw3 = new a2dDrawing( 200, 200 );
    subdraw3->SetDrawAsSymbol( false );
    subdraw3->Append( new a2dRect( 0, -4, 15, 8 ) );
    subdraw3->Append( new a2dPin( subdraw3, wxT("noot_1"), a2dConnectionGeneratorObjectWire::Object, 0, 0 ) );
    subdraw3->Append( new a2dPin( subdraw3, wxT("noot_2"), a2dConnectionGeneratorObjectWire::Object, 15, 0 ) );
    subdraw3->GetRootObject()->Append( new a2dCircle( 40, 0, 10 ) );
    subdraw3->GetRootObject()->Append( new a2dRect( 0, -48, 25, 8 ) );
    root->Prepend( subdraw3 );

    m_canvas->SetShowObject( root );
*/
    //root->Prepend( new a2dRect( 100, -10, 15, 15 ) );
    //root->Prepend( new a2dRect( 100, -40, 15, 15 ) );
}

void MyFrame::FillDocument4( a2dDrawing* drawing )
{
	a2dCanvasObject* root = drawing->GetRootObject();
    root->ReleaseChildObjects();

    //Start filling document now.
    a2dLayers* docLayers = drawing->GetLayerSetup();
    docLayers->SetStroke( 0, a2dStroke( *wxBLACK, 0.5 ) );
    docLayers->SetFill( 0, a2dFill( wxColour( 0xff, 0xff, 0xc0 ) ) );

    // FIRST create a cameleon, with a symbol and a diagram

    // ceate the symbol

    a2dCameleon::SetCameleonRoot( root );

    a2dCameleon* cam1 = new a2dCameleon( wxT("aap") );
    cam1->AddToRoot();
    a2dSymbol* sym1 = new a2dSymbol( cam1 );
    // how does it look like as place holder
    sym1->Append( new a2dRect( 0, -2.5, 20, 5 ) );    
    // define the symbol drawing
    sym1->AppendToDrawing( new a2dRect( 0, -2.5, 10, 15 ) );    
    sym1->AppendToDrawing( new a2dCircle( 5, 2, 3 ) );    
    a2dPort* sympin;
    sym1->AppendToDrawing( sympin = new a2dPort( NULL, 0, 0, wxT("aap_1") ) );
    sympin->GetPin()->SetPinClass( a2dConnectionGeneratorObjectWire::Object );

    sym1->AppendToDrawing( sympin = new a2dPort( NULL, 10, 0, wxT("aap_2") ) );
    sympin->SetRotation( 180 );
    sympin->GetPin()->SetPinClass( a2dConnectionGeneratorObjectWire::Object );
    //sym1->Append( new a2dPort( sym1, wxT("aap_1"), a2dConnectionGeneratorDirectedFlow::ObjectInput, 0, 0 ) );
    //sym1->Append( new a2dPort( sym1, wxT("aap_2"), a2dConnectionGeneratorDirectedFlow::ObjectOutput, 100, 0 ) );

    cam1->AddAppearance( sym1 );

    // ceate the diagram

    a2dDiagram* diagram1 = new a2dDiagram( cam1 );
    diagram1->SetName( wxT("MyCircuit") );
    diagram1->AppendToDrawing( new a2dRect( 0, -2.5, 10, 15 ) );    
    diagram1->AppendToDrawing( new a2dRect( 15, -7.5, 30, 25 ) );    
    diagram1->AppendToDrawing( new a2dPort( NULL, 0,0,wxT("aap_1"), a2dConnectionGeneratorObjectWire::Object ) );
    diagram1->AppendToDrawing( new a2dPort( NULL, 0, 45, wxT("aap_2"), a2dConnectionGeneratorObjectWire::Object ) );
    cam1->AddAppearance( diagram1 );

    // SECOND create a cameleon, with a symbol and a diagram

    a2dCameleon* cam2 = new a2dCameleon( wxT("noot") );
    cam2->AddToRoot();
    a2dSymbol* sym2 = new a2dSymbol( cam2 );
    sym2->AppendToDrawing( new a2dRect( 0, -4, 15, 8 ) );
    sym2->AppendToDrawing( sympin = new a2dPort( NULL, 0, 0, wxT("noot_1") ) );
    sympin->GetPin()->SetPinClass( a2dConnectionGeneratorObjectWire::Object );
    sym2->AppendToDrawing( sympin = new a2dPort( NULL, 15, 0, wxT("noot_2") ) );
    sympin->SetRotation( 180 );
    sympin->GetPin()->SetPinClass( a2dConnectionGeneratorObjectWire::Object );
    //sym2->Append( new a2dPort( sym2, wxT("noot_1"), a2dConnectionGeneratorDirectedFlow::ObjectInput, 0, 0 ) );
    //sym2->Append( new a2dPort( sym2, wxT("noot_2"), a2dConnectionGeneratorDirectedFlow::ObjectOutput, 150, 0 ) );
    cam2->AddAppearance( sym2 );

    a2dDiagram* diagram2 = new a2dDiagram( cam2 );
    diagram2->SetName( wxT("MyOtherCircuit") );
    diagram2->AppendToDrawing( new a2dRect( 0, -2.5, 20, 25 ) );    
    diagram2->AppendToDrawing( new a2dCircle( 25, -7.5, 25 ) );    
    diagram2->AppendToDrawing( new a2dPort( NULL, 0,0,wxT("noot_1"), a2dConnectionGeneratorObjectWire::Object ) );
    diagram2->AppendToDrawing( new a2dPort( NULL, 40, 5, wxT("noot_2"), a2dConnectionGeneratorObjectWire::Object ) );
    cam2->AddAppearance( diagram2 );

    // place the two in a new cameleon called top.

    // THIRD create a cameleon, with a diagram using the above two as sub diagrams

    a2dCameleon* cam3 = new a2dCameleon( wxT("top") );
    cam3->AddToRoot();

    a2dDiagram* diagram3 = new a2dDiagram( cam3 );
    diagram3->SetName( "top cameleon overview" );
    cam3->AddAppearance( diagram3 );

    a2dCameleonInst* ref1 = new a2dCameleonInst( 2, 5, sym1 ); 
    a2dCameleonInst* ref2 = new a2dCameleonInst( 20, 5, sym2 );
    ref1->SetName( "ref1" );
    ref2->SetName( "ref2" );
    diagram3->AppendToDrawing( ref1 );
    diagram3->AppendToDrawing( ref2 );
    diagram3->SetName( "schematic" );

    a2dCameleonInst* ref3 = new a2dCameleonInst( 52.2, 5, diagram2 );
    ref3->SetName( "ref3" );
    diagram3->AppendToDrawing( ref3 );

    //make an overview of all cameleons

    a2dCameleonSymbolicRef* namerefsymbol1 = new a2dCameleonSymbolicRef( 20,0, sym1 );
	root->Append( namerefsymbol1 );
    a2dCameleonSymbolicRef* namerefsymbol2 = new a2dCameleonSymbolicRef( 20,-4, sym2 );
	root->Append( namerefsymbol2 );

    a2dCameleonSymbolicRef* nameref = new a2dCameleonSymbolicRef( 0,0, diagram1 );
	root->Append( nameref );
    a2dCameleonSymbolicRef* nameref2 = new a2dCameleonSymbolicRef( 0,-4, diagram2 );
	root->Append( nameref2 );

    a2dCameleonSymbolicRef* nameref3 = new a2dCameleonSymbolicRef( 0,-10, diagram3 );
	root->Append( nameref3 );
    root->SetName( "top cameleon overview drawing" );

    drawing->SetDrawingId( a2dDrawingId_cameleonrefs() );

    m_canvas->SetShowObject( root );
    //m_canvas->SetShowObject( diagram3->GetDrawing()->GetRootObject() );
}


void MyFrame::FillDocument5( a2dDrawing* drawing )
{
    a2dLayers* drawLayers = drawing->GetLayerSetup();
    drawLayers->SetStroke( 0, a2dStroke( *wxBLACK, 0.1 ) );
    drawLayers->SetFill( 0, a2dFill( wxColour( 0xff, 0xff, 0xc0 ) ) );

    a2dPinClass* pclass = a2dConnectionGeneratorObjectWire::Object;
    pclass = a2dPinClass::Standard;
    // cameleons go in the top of the document.
	a2dCanvasObject* root = drawing->GetRootObject();
    root->ReleaseChildObjects();

    // FIRST create a cameleon, with a symbol

    a2dCameleon::SetCameleonRoot( root );
    a2dCameleon* cam1 = new a2dCameleon( wxT("aap") );
    cam1->AddToRoot();

    a2dSymbol* sym1 = new a2dSymbol( cam1 );
    // define the symbol drawing
    sym1->AppendToDrawing( new a2dRect( 0, -2.5, 10, 15 ) );    
    a2dPort* sympin;
    sym1->AppendToDrawing( sympin = new a2dPort( NULL, 0, 0, wxT("aap_1") ) );
    sympin->GetPin()->SetPinClass( pclass );

    cam1->AddAppearance( sym1 );


    // THIRD create a cameleon, with a diagram being the top
    a2dCameleon* cam3 = new a2dCameleon( wxT("top") );
    cam3->AddToRoot();
    a2dDiagram* diagram3 = new a2dDiagram( cam3 );
    diagram3->SetName( "top cameleon overview" );
    cam3->AddAppearance( diagram3 );

    // file top diagram
    a2dCameleonInst* ref1 = new a2dCameleonInst( 2, 5, sym1 ); 
    ref1->SetName( "ref1" );
    diagram3->AppendToDrawing( ref1 );

    diagram3->AppendToDrawing( new a2dRect( 6, -3.5, 20, 5 ) );    
    diagram3->AppendToDrawing( new a2dRect( 16, -8.5, 20, 5 ) );    

    m_canvas->SetShowObject( diagram3->GetDrawing()->GetRootObject() );
    m_canvas->SetShowObject( root );
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    wxMessageBox( _T( "wires sample\n(c) 2002 Erik van der Wal & Klaas Holwerda\
 Demonstrates use of connected objects, and two types of objects and wires.\
 Schematic objects are connected through one type. Basic primitives get automatic\
 generated pins, and or connected via a second type of wire. " ),
                  _T( "About wires sample" ), wxOK | wxICON_INFORMATION, this );
}

void MyFrame::OnLineModeSwitch( wxCommandEvent& event )
{
    bool line = a2dCanvasGlobals->GetHabitat()->GetRouteOneLine();
    a2dCanvasGlobals->GetHabitat()->SetRouteOneLine( !line );
    m_menu_bar->Check( ID_WIREMODE, !line );
}

void MyFrame::OnActivateViewSentFromChild( a2dViewEvent& viewevent )
{
    a2dView* view = ( a2dView* ) viewevent.GetEventObject();

    if ( view != m_view )
    {
        if ( viewevent.GetActive() )
        {
            a2dCanvasView* canview = wxDynamicCast( view, a2dCanvasView );
            if ( canview && m_canvas->GetDrawing() && m_canvas->GetDrawing()->GetCommandProcessor() )
            {
                m_canvas->GetDrawing()->DisconnectEvent( wxEVT_MENUSTRINGS, this );
                m_canvas->GetDrawing()->DisconnectEvent( wxEVT_DO, this );
                m_canvas->GetDrawing()->DisconnectEvent( wxEVT_UNDO, this );
                m_canvas->GetDrawing()->DisconnectEvent( wxEVT_REDO, this );
            }
            m_view = view;
            canview = wxDynamicCast( m_view.Get(), a2dCanvasView );
            if ( canview && m_canvas->GetDrawing() && m_canvas->GetDrawing()->GetCommandProcessor() )
            {
                m_canvas->GetDrawing()->ConnectEvent( wxEVT_MENUSTRINGS, this );
                m_canvas->GetDrawing()->ConnectEvent( wxEVT_DO, this );
                m_canvas->GetDrawing()->ConnectEvent( wxEVT_UNDO, this );
                m_canvas->GetDrawing()->ConnectEvent( wxEVT_REDO, this );
            }
        }
        else
        {
            a2dCanvasView* canview = wxDynamicCast( m_view.Get(), a2dCanvasView );
            if ( canview && m_canvas->GetDrawing() && m_canvas->GetDrawing()->GetCommandProcessor() )
            {
                m_canvas->GetDrawing()->DisconnectEvent( wxEVT_DO, this );
                m_canvas->GetDrawing()->DisconnectEvent( wxEVT_UNDO, this );
                m_canvas->GetDrawing()->DisconnectEvent( wxEVT_REDO, this );
            }
            m_view = view;
        }
    }
    viewevent.Skip();
}

void MyFrame::OnSetMenuStrings( a2dCommandProcessorEvent& event )
{
    if ( !m_view )
        return;

    a2dCanvasView* canview = wxDynamicCast( m_view.Get(), a2dCanvasView );

    if ( m_editMenu && canview && m_canvas->GetDrawing() && event.GetEventObject() == m_canvas->GetDrawing()->GetCommandProcessor() )
    {
        m_editMenu->SetLabel( wxID_UNDO, event.GetUndoMenuLabel() );
        m_editMenu->Enable( wxID_UNDO, event.CanUndo() );

        m_editMenu->SetLabel( wxID_REDO, event.GetRedoMenuLabel() );
        m_editMenu->Enable( wxID_REDO, event.CanRedo() );
    }
}

void MyFrame::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
    if ( !m_view )
        return;

    a2dCanvasView* canview = wxDynamicCast( m_view.Get(), a2dCanvasView );

    if ( canview && m_canvas->GetDrawing() && m_canvas->GetDrawing()->GetCommandProcessor() )
        m_canvas->GetDrawing()->GetCommandProcessor()->Undo();
}

void MyFrame::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
    if ( !m_view )
        return;

    a2dCanvasView* canview = wxDynamicCast( m_view.Get(), a2dCanvasView );

    if ( canview && m_canvas->GetDrawing() && m_canvas->GetDrawing()->GetCommandProcessor() )
        m_canvas->GetDrawing()->GetCommandProcessor()->Redo();
}

#if wxART2D_USE_AGGDRAWER
void MyFrame::OnDrawer( wxCommandEvent& event )
{
    if ( wxDynamicCast( m_canvas->GetDrawingPart()->GetDrawer2D(), a2dAggDrawer ) )
        m_canvas->GetDrawingPart()->SetDrawer2D( new a2dMemDcDrawer( m_canvas->GetSize() ) );
    else
        m_canvas->GetDrawingPart()->SetDrawer2D( new a2dAggDrawer( m_canvas->GetSize() ) );
    m_canvas->GetDrawingPart()->GetDrawer2D()->SetSmallTextThreshold( 20 );

    m_canvas->GetDrawingPart()->Update( a2dCANVIEW_UPDATE_ALL );

}
#endif //wxART2D_USE_AGGDRAWER

void MyFrame::AddFunctionToMenu( int id, wxMenu* parentMenu, const wxString& text, const wxString& helpString, wxObjectEventFunctionM func, bool check )
{
    Connect( id, wxEVT_COMMAND_MENU_SELECTED, ( wxObjectEventFunction ) wxStaticCastEvent( wxCommandEventFunction, func ) );
    parentMenu->Append( id, text, helpString, check );
}

void MyFrame::OnTest( wxCommandEvent& event )
{
    a2dCanvasObject* root = m_canvas->GetDrawing()->GetRootObject();
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
    setflags.Start( root, false );
    //root->GetChildObjectList()->SetSpecificFlags( false, a2dCanvasOFlags::BIN );
    a2dCanvasObject* R1 = root->GetChildObjectList()->Find( wxT("R1") );
    a2dCanvasObjectList result;
    wxString names;
    if ( R1 )
    {
        R1->FindConnectedPins( result, NULL, true, NULL );
        forEachIn( a2dCanvasObjectList, &result )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pinc = wxDynamicCast( obj, a2dPin );
            if ( pinc )
            {
                wxString pinname = pinc->GetName();
                wxString name = pinc->GetParent()->GetName();
                names += name + " " + pinname + "\n";
            }
        }
    }

    ( void )wxMessageBox( names, _( "connected pins" ), wxICON_INFORMATION | wxOK );
    names.Clear();

    setflags.Start( root, false );
    forEachIn( a2dCanvasObjectList, root->GetChildObjectList() )
    {
        a2dCanvasObject* topobj = *iter;
        a2dCanvasObjectList result;
        if ( topobj->IsConnect() )
            names += "from wire: " + topobj->GetName() + "\n  ";
        else
            names += "from: " + topobj->GetName() + "\n   ";
        // clear bin flags in between unless you want all connection reported only once
        a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
        setflags.Start( root, false );

        topobj->FindConnectedPins( result, NULL, true, NULL );
        forEachIn( a2dCanvasObjectList, &result )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pinc = wxDynamicCast( obj, a2dPin );
            if ( pinc )
            {
                wxString pinname = pinc->GetName();
                wxString name = pinc->GetParent()->GetName();
                names += " object: " + name + " pin: " + pinname;
            }
        }
        names += "\n";
    }

    ( void )wxMessageBox( names, _( "connected pins" ), wxICON_INFORMATION | wxOK );
    names.Clear();


    setflags.Start( root, false );
    forEachIn( a2dCanvasObjectList, root->GetChildObjectList() )
    {
        a2dCanvasObject* topobj = *iter;
        a2dCanvasObjectList result;
        if ( topobj->IsConnect() )
            names += "from wire: " + topobj->GetName() + "\n  ";
        else
            names += "from: " + topobj->GetName() + "\n   ";
        // clear bin flags in between unless you want all connection reported only once
        a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
        setflags.Start( root, false );

        topobj->FindConnectedPins( result, NULL, true, a2dConnectionGeneratorDirectedFlow::ObjectInput, NULL );
        forEachIn( a2dCanvasObjectList, &result )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pinc = wxDynamicCast( obj, a2dPin );
            if ( pinc )
            {
                wxString pinname = pinc->GetName();
                wxString name = pinc->GetParent()->GetName();
                names += " object: " + name + " pin: " + pinname;
            }
        }
        names += "\n";
    }

    ( void )wxMessageBox( names, _( "connected pins" ), wxICON_INFORMATION | wxOK );


}

void MyFrame::OnMasterToolOption( wxCommandEvent& event )
{
    m_editMode = event.GetId() - MasterToolOption_0;
    if ( m_editMode == 0 )
    {
        a2dGraphicsMasterTool* tool = new a2dGraphicsMasterTool( m_contr );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
    }
    else if ( m_editMode == 1 )
    {
        a2dMasterDrawZoomFirst* tool = new a2dMasterDrawZoomFirst( m_contr );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
    }
    else if ( m_editMode == 2 )
    {
        a2dMasterDrawSelectFirst* tool = new a2dMasterDrawSelectFirst( m_contr );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
    } 
    else if ( m_editMode == 3 )
    {
        a2dMasterTagGroups* tool = new a2dMasterTagGroups( m_contr );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
    }
    else if ( m_editMode == 4 )
    {
        a2dMasterTagGroups2* tool = new a2dMasterTagGroups2( m_contr );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
    }
    else if ( m_editMode == 5 )
    {
        a2dMasterTagGroups3* tool = new a2dMasterTagGroups3( m_contr );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
    }
    m_menu_bar->Check( MasterToolOption_0, m_editMode == 0 );
    m_menu_bar->Check( MasterToolOption_1, m_editMode == 1 );
    m_menu_bar->Check( MasterToolOption_2, m_editMode == 2 );
    m_menu_bar->Check( MasterToolOption_3, m_editMode == 3 );
    m_menu_bar->Check( MasterToolOption_4, m_editMode == 4 );
    m_menu_bar->Check( MasterToolOption_5, m_editMode == 5 );
}


void MyFrame::FillData( wxCommandEvent& event )
{
    switch ( event.GetId() )
    {
        case MDSCREEN_0:
        {
            FillDocument(  m_canvas->GetDrawing() );
            break;
        }
        case MDSCREEN_1:
        {
            FillDocument2(  m_canvas->GetDrawing() );
            break;
        }
        case MDSCREEN_2:
        {
            FillDocument3(  m_canvas->GetDrawing() );
            break;
        }
        case MDSCREEN_3:
        {
            FillDocument4(  m_canvas->GetDrawing() );
            break;
        }
        case MDSCREEN_4:
        {
            FillDocument5(  m_canvas->GetDrawing() );
            break;
        }
        default:
            return;
    }
    m_canvas->SetMappingShowAll();
}

void MyFrame::OnDragSimulate(wxCommandEvent& event)
{
    a2dDragTool* drag = new a2dDragTool( m_contr );
    drag->SetDropAndDrop( true );
    m_contr->PushTool( drag );

}

void MyFrame::OnCopyShape(wxCommandEvent& WXUNUSED(event))
{
    a2dDrawingPart* part = m_canvas->GetDrawingPart();
    a2dDnDCanvasObjectDataCVG* copied = new a2dDnDCanvasObjectDataCVG( part, wxDataObject::Get);
    //a2dDnDCameleonData* copied = new a2dDnDCameleonData();

    if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
    {
        a2dCanvasObjectList* objects = part->GetShowObject()->GetChildObjectList();
        forEachIn( a2dCanvasObjectList, objects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj->GetRelease() || !obj->IsVisible()  || !obj->GetSelected() )
                continue;
            copied->Append( obj );
        }
        copied->Reconnect();

        wxClipboardLocker clipLocker;
        if ( !clipLocker )
        {
            wxLogError(wxT("Can't open the clipboard"));

            return;
        }

        wxTheClipboard->AddData( copied );
    }
}

void MyFrame::OnPasteShape(wxCommandEvent& WXUNUSED(event))
{
    wxClipboardLocker clipLocker;
    if ( !clipLocker )
    {
        wxLogError(wxT("Can't open the clipboard"));
        return;
    }

    a2dDrawingPart* part = m_canvas->GetDrawingPart();

    if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
    {
        a2dDnDCanvasObjectDataCVG shapeDataObject( part, wxDataObject::Set);
        //a2dDnDCameleonData shapeDataObject(NULL);
        shapeDataObject.SetImportWidthHeightImage( 10000 );
        if ( wxTheClipboard->GetData(shapeDataObject) )
        {
            a2dDrawingPtr drawing = shapeDataObject.GetDrawing();
            a2dCanvasObjectPtr shape = drawing->GetRootObject();
    	    a2dCanvasObject* root = part->GetDrawing()->GetRootObject();

            a2dWalker_SetSpecificFlagsCanvasObjects setflags2( a2dCanvasOFlags::SELECTED | a2dCanvasOFlags::SELECTED2 );
            setflags2.Start( part->GetShowObject(), false );
            shape->Update( a2dCanvasObject::updatemask_force );
            shape->SetRoot( part->GetDrawing() );
            a2dBoundingBox box = shape->GetBbox();
            a2dPoint2D p = box.GetCentre();
            shape->Translate( -p.m_x, -p.m_y );
            shape->EliminateMatrix();
            
            // to test
            //root->Append( shape );

            if ( shape->GetChildObjectList()->size() )
            {
                a2dDragMultiNewTool* dragnew = new a2dDragMultiNewTool( (a2dStToolContr *) (part->GetCanvasToolContr()), shape->GetChildObjectList() );
                dragnew->SetOneShot();
                dragnew->SetStroke( a2dStroke( *wxRED, 1 ) );
                dragnew->SetFill( a2dFill( wxColour( 0xc0, 0xff, 0xff ) ) );
                (a2dStToolContr *) (part->GetCanvasToolContr())->PushTool( dragnew );
            }            
            else
            {
                wxLogStatus(wxT("No shape on the clipboard"));
            }
        }
        else
        {
            wxLogStatus(wxT("No shape on the clipboard"));
        }
    }
}

void MyFrame::OnUpdatePasteShape( wxUpdateUIEvent& event  )
{
    a2dDrawingPart* part = m_canvas->GetDrawingPart();
    a2dDnDCanvasObjectDataCVG shapeDataObject( part, wxDataObject::Set);
    wxDataFormat formats[10];
    shapeDataObject.GetAllFormats( formats );
    int nr = shapeDataObject.GetFormatCount();

    bool ret = false;
        
    for ( int i = 0 ; i < nr ; i++ )
    {
        ret |= wxTheClipboard->IsSupported( formats[i] );
    }

    event.Enable( ret );
}

void MyFrame::OnUpdateCopyShape( wxUpdateUIEvent& event  )
{
    bool ret = false;
    a2dDrawingPart* part = m_canvas->GetDrawingPart();
    if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
    {
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
    event.Enable( ret );
}

//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------

IMPLEMENT_APP( MyApp )

MyApp::MyApp()
{
    //_CrtSetBreakAlloc(44597);
}

bool MyApp::OnInit()
{
    wxString artroot = a2dGeneralGlobals->GetWxArt2DVar();
    if ( artroot.IsEmpty() )
        return false;

    wxInitAllImageHandlers();

    a2dGlobals->GetFontPathList().Add( artroot + wxT( "samples/editor/common/fonts" ) );
    a2dGlobals->GetFontPathList().Add( wxT( "/usr/share/fonts/truetype/msttcorefonts" ) );
    a2dGlobals->GetImagePathList().Add( artroot +  wxT( "samples/editor/common/images" ) );
    a2dGlobals->GetImagePathList().Add( artroot +  wxT( "samples/editor/wires" ) );
    a2dGlobals->GetFontPathList().Add(  artroot +  wxT( "samples/editor/common/fonts" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./" ) );
    a2dCanvasGlobals->GetLayersPathList().Add( artroot +  wxT( "art/layers" ) );

    a2dCanvasGlobals->GetHabitat()->LoadLayers( wxT( "layersdefault_3lay.cvg" ) );

    a2dGlobals->GetFontPathList().Add( wxT( "./config" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./config" ) );
    a2dGlobals->GetIconPathList().Add( wxT( "./config" ) );
    a2dGlobals->GetFontPathList().Add( wxT( "../config" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "../config" ) );
    a2dGlobals->GetIconPathList().Add( wxT( "../config" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "art/resources" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "art/layers" ) );

#if (wxART2D_USE_LUA == 1)

    // Initialize the wxLua bindings we want to use.
    // See notes for WXLUA_DECLARE_BIND_STD above.
    //WXLUA_IMPLEMENT_BIND_STD
    WXLUA_IMPLEMENT_BIND_WXLUA 
    WXLUA_IMPLEMENT_BIND_WXBASE 
    WXLUA_IMPLEMENT_BIND_WXCORE 

    wxLuaBinding_wxart2d_init();

    a2dLuaCentralCommandProcessor* cmdProc = new a2dLuaCentralCommandProcessor();
#else
    //set a canvas commandprocessor instead of the default, in order to use more commands.
    a2dCentralCanvasCommandProcessor* cmdProc = new a2dCentralCanvasCommandProcessor();
#endif

    a2dDocviewGlobals->SetDocviewCommandProcessor( cmdProc );
    //cmdProc->SetEditAtEndTools( false );
    a2dCanvasGlobals->SetPopBeforePush( true );
    a2dCanvasGlobals->SetOneShotTools( false );

    a2dCanvasGlobals->GetHabitat()->SetSelectStroke( a2dStroke( wxColour( 255, 55, 0 ), 2, a2dSTROKE_SOLID ) );
    a2dCanvasGlobals->GetHabitat()->SetSelectFill( *a2dTRANSPARENT_FILL );
    a2dCanvasGlobals->GetHabitat()->SetSelectDrawStyle( RenderWIREFRAME_SELECT );

    a2dIOHandlerDocCVGIn* cvghin = new a2dIOHandlerDocCVGIn();
    a2dIOHandlerDocCVGOut* cvghout = new a2dIOHandlerDocCVGOut();

#if wxART2D_USE_FREETYPE
    delete a2dDEFAULT_CANVASFONT;
    a2dDEFAULT_CANVASFONT = new a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Bold.ttf" ) ), 5 );
    a2dCanvasGlobals->GetHabitat()->SetFont( *a2dDEFAULT_CANVASFONT ); 
#endif

    //create the list of available fonts
    a2dTextChanges::InitialiseFontList();

    //connector to couple the single and only canvas + view to new documents.
    a2dSmrtPtr<a2dConnector> singleconnector = new a2dConnector();

    a2dDocumentTemplate* doctemplatenew;

    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate( wxT( "CVG Drawing" ), wxT( "*.cvg" ), wxT( "" ), wxT( "cvg" ), wxT( "a2dCanvasDocument" ),
            CLASSINFO( a2dCanvasDocument ), singleconnector, a2dTemplateFlag::DEFAULT, cvghin, cvghout );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->AssociateDocTemplate( doctemplatenew );


    a2dViewTemplate* viewtemplatenew;
    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate( wxT( "CVG Drawing" ), wxT( "a2dCanvasDocument" ), wxT( "Drawing View" ),
                                           CLASSINFO( a2dCanvasView ), singleconnector, a2dTemplateFlag::DEFAULT, a2dCanvasGlobals->m_initialDocChildFrameSize );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->AssociateViewTemplate( viewtemplatenew );

    m_frame = new MyFrame( NULL, -1, _T( "Wires Example" ), wxPoint( 20, 30 ), wxSize( 600, 440 ) );
	m_frame->Show( true );
    singleconnector->SetDisplayWindow( m_frame->GetCanvas() );

    a2dCanvasDocument* document = new a2dCanvasDocument();
	document->SetDrawing( m_frame->GetCanvas()->GetDrawing() );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->AddCreatedDocument( document, true, true, a2dREFDOC_NEW );
	document->SetDocumentTemplate( doctemplatenew );
    document->SetDocumentTypeName( doctemplatenew->GetDocumentTypeName() );
    document->SetTitle( wxT( "wire document" ) );

    ::wxInitAllImageHandlers();

    wxConfigBase* pConfig = wxConfigBase::Get();
    a2dGetCmdh()->FileHistoryLoad( *pConfig );

    return true;
}

int MyApp::OnExit()
{
    wxConfigBase* pConfig = wxConfigBase::Get();
    a2dGetCmdh()->FileHistorySave( *pConfig );
    return 0;
}


void MyApp::OnAssertFailure(const wxChar *file,
                                       int line,
                                       const wxChar *func,
                                       const wxChar *cond,
                                       const wxChar *msg)
{
    wxAppConsoleBase::OnAssertFailure( file, line, func, cond, msg );
}

#if (wxART2D_USE_LUA == 1)

void MyFrame::OnRunScript( wxCommandEvent& event )
{
    if ( !a2dLuaWP )
    {
        ( void )wxMessageBox( _( "a2dLuaCentralCommandProcessor not setup" ), _( "a2dEditorFrame" ), wxICON_INFORMATION | wxOK );
        return;
    }

    wxString fullPath = wxFileSelector( _( "Select a script file" ),
                                        _( "" ),
                                        _( "" ),
                                        _( "lua" ),
                                        _( "*.lua" ) ,
                                        0,
                                        this
                                      );

    if ( !fullPath.IsEmpty() && ::wxFileExists( fullPath ) )
    {
        if ( !a2dLuaWP->ExecuteFile( fullPath ) )
        {
            wxLogWarning( _( "Error in Lua Script" ) );
        }
    }
}

#endif //(wxART2D_USE_LUA == 1)

