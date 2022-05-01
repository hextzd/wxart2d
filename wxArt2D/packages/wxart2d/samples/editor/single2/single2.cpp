/*! \file editor/samples/single2/simple.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: single2.cpp,v 1.12 2009/09/26 19:01:19 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "single2.h"
#include "wx/artbase/artmod.h"

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#include "wx/aggdrawer/graphicagg.h"
#endif //wxART2D_USE_AGGDRAWER
#if wxART2D_USE_GDIPLUSDRAWER
#include "wx/gdiplusdrawer/gdiplusdrawer.h"
#endif //wxART2D_USE_GDIPLUSDRAWER 

extern const long OPTION_SWITCHYAXIS = wxNewId();
extern const long DOCCANVAS_ABOUT = wxNewId();
extern const long SET_POLYTOOL = wxNewId();
extern const long SET_RECTTOOL = wxNewId();
extern const long SET_DRAGNEWTOOL = wxNewId();
extern const long SET_EDITTOOL = wxNewId();
extern const long END_TOOL = wxNewId();

//----------------------------------------------------------------------------
//   a2dConnector
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dConnector, a2dViewConnector )
    EVT_POST_CREATE_VIEW( a2dConnector::OnPostCreateView )
    EVT_PRE_ADD_DOCUMENT( a2dConnector::OnPreAddCreateDocument )
    EVT_POST_CREATE_DOCUMENT( a2dConnector::OnPostCreateDocument )
    EVT_DISCONNECT_VIEW( a2dConnector::OnDisConnectView )
END_EVENT_TABLE()

a2dConnector::a2dConnector()
    : a2dViewConnector()
{
    m_display = NULL;
}

void a2dConnector::OnPostCreateView( a2dTemplateEvent& event )
{
}

void a2dConnector::OnPreAddCreateDocument( a2dTemplateEvent& event )
{
    m_newdoc = event.GetDocument();

    if ( !( event.GetFlags() & a2dREFDOC_SILENT ) &&
            ( event.GetFlags() & a2dREFDOC_INIT ) &&
            ( event.GetFlags() & a2dREFDOC_NEW )  && m_display )
    {
        a2dNewDocumentSize* newUnits = new a2dNewDocumentSize( NULL );
        if ( m_newdoc && newUnits->ShowModal() == wxID_OK )
        {
            a2dSmrtPtr<a2dCanvasDocument> docc = wxStaticCastNull( event.GetDocument(), a2dCanvasDocument );
            docc->GetDrawing()->SetInitialSize( newUnits->GetSizeX(), newUnits->GetSizeY() );
            docc->SetUnits( newUnits->GetUnit() );
            double scale;
            if ( a2dDoMu::GetMultiplierFromString( newUnits->GetUnit(), scale ) )
                docc->SetUnitsScale( scale );
        }
        delete newUnits;
    }

    a2dCanvasDocument* current = ( a2dCanvasDocument* ) a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentDocument();
    if ( current && !current->IsClosed() )
    {
        a2dDocviewGlobals->GetDocviewCommandProcessor()->FileClose();
    }
}

void a2dConnector::OnPostCreateDocument( a2dTemplateEvent& event )
{
    //a2dDocumentTemplate* templ = (a2dDocumentTemplate*) event.GetEventObject();
    a2dViewTemplate* viewtempl;
    viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates().front();

    m_view = ( a2dCanvasView* ) viewtempl->CreateView( event.GetDocument(), event.GetFlags() );
    m_view->SetPrintTitle( false );
    m_view->SetPrintFilename( false );
    m_view->SetPrintFrame( false );
    event.SetView( m_view );

	if ( m_display )
    {
        m_view->SetDisplayWindow( m_display );
        a2dCanvas* canvas = ( a2dCanvas* ) m_display;
        if ( canvas->GetDrawingPart()->GetCanvasToolContr() )
            canvas->GetDrawingPart()->GetCanvasToolContr()->StopAllTools();

        m_view->SetDocument( event.GetDocument() );
        if ( canvas->GetDrawingPart()->GetCanvasToolContr() )
            canvas->GetDrawingPart()->GetCanvasToolContr()->ReStart();

        canvas->GetDrawingPart()->SetMappingShowAll();
    }    
    m_view->Activate( true );
}

void a2dConnector::OnDisConnectView(  a2dTemplateEvent& event )
{
    a2dSmrtPtr<a2dCanvasView> theViewToDisconnect = ( a2dCanvasView* ) event.GetEventObject();

    //prevent recursive calls
    if ( theViewToDisconnect->IsClosed() )
        return; //was already closed

    theViewToDisconnect->Activate( false );
    if ( theViewToDisconnect->GetDocument() )
    {
        a2dDocumentEvent eventremove( wxEVT_REMOVE_VIEW, theViewToDisconnect, true, theViewToDisconnect->GetId() );
        eventremove.SetEventObject( theViewToDisconnect );
        theViewToDisconnect->GetDocument()->ProcessEvent( eventremove );
    }
    if ( theViewToDisconnect->GetDrawingPart()->GetCanvasToolContr() )
        theViewToDisconnect->GetDrawingPart()->GetCanvasToolContr()->StopAllTools();

    if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->GetBusyExit() )
    {
        // Not existing, we do reuse the view, and set it to a document already open.
        // If non open we create a new one.
        if ( m_newdoc )
        {
            theViewToDisconnect->SetDocument( m_newdoc );
            m_newdoc = NULL;
            //a2dDocumentList::const_reverse_iterator iter = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().rbegin();
            //iter++;
            //theViewToDisconnect->SetDocument( (*iter) );
        }
        else if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().size() > 1 ) //other documents open?
        {
            wxMessageBox( wxT( "I expect there is always <= one document open, this is weird!" ) );
            int nr = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().size();
            a2dDocumentList::const_reverse_iterator iter = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().rbegin();
            iter++;
            theViewToDisconnect->SetDocument( ( *iter ) );
        }
        else if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().size() == 1 )
        {
            a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateDocuments( wxT( "dummy" ), a2dREFDOC_NEW | a2dREFDOC_SILENT );
            a2dCanvasDocument* document = ( a2dCanvasDocument* ) a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().back().Get();
            theViewToDisconnect->SetDocument( document );
            m_newdoc = NULL;
        }
        else // no more document, we create one
        {
            a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateDocuments( wxT( "dummy" ), a2dREFDOC_NEW | a2dREFDOC_SILENT );
            a2dCanvasDocument* document = ( a2dCanvasDocument* ) a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().back().Get();
            theViewToDisconnect->SetDocument( document );
            m_newdoc = NULL;
        }

        if ( theViewToDisconnect->GetDrawingPart()->GetCanvasToolContr() )
            theViewToDisconnect->GetDrawingPart()->GetCanvasToolContr()->ReStart();
        theViewToDisconnect->GetDrawingPart()->SetMappingShowAll();
        theViewToDisconnect->Activate( true );
    }
    else
        // exiting, do the default, disconnecting the view.
        event.Skip();
}


//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

IMPLEMENT_CLASS( MyFrame, a2dDocumentFrame )

BEGIN_EVENT_TABLE( MyFrame, a2dDocumentFrame )
    EVT_MENU( wxID_EXIT, MyFrame::OnExit )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_MENU( DOCCANVAS_ABOUT, MyFrame::OnAbout )
    EVT_MENU( SET_POLYTOOL, MyFrame::SetPolygonTool )
    EVT_MENU( SET_RECTTOOL, MyFrame::SetRectangleTool )
    EVT_MENU( SET_DRAGNEWTOOL, MyFrame::SetDragNewTool )
    EVT_MENU( SET_EDITTOOL, MyFrame::SetEditTool )
    EVT_MENU( END_TOOL, MyFrame::EndTool )
    EVT_MENU( wxID_UNDO, MyFrame::OnUndo )
    EVT_MENU( wxID_REDO, MyFrame::OnRedo )
END_EVENT_TABLE()


MyFrame::MyFrame( a2dConnector* connect, wxFrame* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, const long style ) :
    a2dDocumentFrame( true, parent, NULL, -1, title, position, size, style )
{
    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( wxT( "Welcome to a2dCanvas sample!" ) );

    SetIcon( wxICON( mondrian ) );
#ifdef SIMPLE
    m_canvas = new a2dCanvasSim( this, -1 , wxPoint( 0, 0 ), size, wxNO_FULL_REPAINT_ON_RESIZE );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    //! set virtual size in pixels, this independent of the mapping below
    m_canvas->SetVirtualSize( 1000, 800 );
    m_canvas->SetScrollRate( 50, 50 );
    m_canvas->SetYaxis( true );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->SetMappingUpp( -200, -300, 4, 4 );
#else
	#if wxART2D_USE_AGGDRAWER
        a2dDrawer2D* drawer2d = new a2dAggDrawer( connect->GetInitialSize() );
	#else
        a2dDrawer2D* drawer2d = new a2dMemDcDrawer( connect->GetInitialSize() );
	#endif //wxART2D_USE_AGGDRAWER
    //OR
    //a2dDrawer2D* drawer2d = new a2dAggDrawer( connect->GetInitialSize() );
    a2dFont::SetLoadFlags( a2dFont::a2d_LOAD_DEFAULT | a2dFont::a2d_LOAD_FORCE_AUTOHINT );

    m_canvas = new a2dCanvas( this , -1, wxPoint( 0, 0 ), size, wxHSCROLL | wxVSCROLL );
    
    m_canvas->GetDrawingPart()->SetDrawer2D( drawer2d );
    drawer2d->SetSmallTextThreshold( 100 );
    drawer2d->SetSmallTextThreshold( 2000 );
    
    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->SetMappingUpp( -200, -300, 4, 4 );
    //defines the whole area in world coordinates that can be seen by scrolling
    m_canvas->SetScrollMaximum( -100, -200, 1600, 1600 );
    //defines the number of world units scrolled when line up or down events in X
    m_canvas->SetScrollStepX( 100 );
    //defines the number of world units scrolled when line up or down events in Y
    m_canvas->SetScrollStepY( 100 ); 
    //do not go outide the scroll maximum
    m_canvas->FixScrollMaximum( true ); //what to do when resizing above ScrollMaximum is still ??
    m_canvas->SetYaxis( true );

#endif
    m_canvas->SetShowOrigin( false );

	connect->SetDisplayWindow( m_canvas );
    //make a new document and add it to the document manager and this will create a a2dCanvasView.
    a2dCanvasDocument* document = new a2dCanvasDocument();
	document->SetDrawing( m_canvas->GetDrawing() );
    document->GetDrawing()->CreateCommandProcessor();
    a2dDocviewGlobals->GetDocviewCommandProcessor()->AddCreatedDocument( document, false, true );

    //tell the connector what are the window and the view.
    connect->m_view->SetPrintTitle( false );
    connect->m_view->SetPrintFilename( false );
    connect->m_view->SetPrintFrame( false );

    //! for some tools a tool controller
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //the next is handy, but as you prefer
    m_contr->SetZoomFirst( true );

    bool ydir = false;

    m_canvas->SetYaxis( ydir );
    m_canvas->SetBackgroundFill( a2dFill( wxColour( 0, 200, 200 ) ) );
    m_menuBar->Check( OPTION_SWITCHYAXIS, m_canvas->GetYaxis() );
    m_canvas->SetGrid( true );

    // Make sure the document manager knows that this is the
    // current view.
    m_view->Activate( true );

	a2dRect* r = new a2dRect( 0, 0, 80, 40 );
    r->SetStroke( wxColour( 9, 215, 64 ), 10.0 );
    r->SetFill( wxColour( 220, 217, 200 ) );
    a2dCanvasObject::PROPID_Allowrotation->SetPropertyToObject( r, false );
    a2dCanvasObject::PROPID_Allowskew->SetPropertyToObject( r, false );
    a2dRect::PROPID_AllowRounding->SetPropertyToObject( r, false );

    document->GetDrawing()->GetRootObject()->Append( r );

    m_canvas->Update();
}

MyFrame::~MyFrame()
{
}

void MyFrame::CreateMyMenuBar()
{
    //// Make a menubar
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

    m_edit_menu = new wxMenu;
    m_edit_menu->Append( wxID_UNDO, _( "&Undo" ) );
    m_edit_menu->Append( wxID_REDO, _( "&Redo" ) );
    m_edit_menu->Append( SET_POLYTOOL, wxT( "&setpolygontool" ) );
    m_edit_menu->Append( SET_RECTTOOL, wxT( "set&rectangletool" ) );
    m_edit_menu->Append( SET_EDITTOOL, wxT( "set&edittool" ) );
    m_edit_menu->Append( SET_DRAGNEWTOOL, wxT( "drag&new" ) );
    m_edit_menu->Append( END_TOOL, wxT( "&endtool" ) );

    m_optionMenu = new wxMenu;
    AddFunctionToMenu( OPTION_SWITCHYAXIS, m_optionMenu, _( "Yaxis Positive" ), _( "switch Y axis (positive or negative" ), &MyFrame::SetSwitchY, true );
    AddCmdMenu( m_optionMenu, CmdMenu_Refresh() );

    file_menu->AppendSeparator();
    file_menu->Append( wxID_EXIT, _( "E&xit" ) );

    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( file_menu );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( DOCCANVAS_ABOUT, _( "&About" ) );

    m_menuBar = new wxMenuBar;
    m_menuBar->Append( file_menu, _( "&File" ) );
    m_menuBar->Append( m_edit_menu, _( "&Edit" ) );
    m_menuBar->Append( m_optionMenu, _( "&Options" ) );
    m_menuBar->Append( help_menu, _( "&Help" ) );
    //// Associate the menu bar with the frame
    SetMenuBar( m_menuBar );
}

void MyFrame::AddFunctionToMenu( int id, wxMenu* parentMenu, const wxString& text, const wxString& helpString, wxObjectEventFunctionM func, bool check )
{
    Connect( id, wxEVT_COMMAND_MENU_SELECTED, ( wxObjectEventFunction ) wxStaticCastEvent( wxCommandEventFunction, func ) );
    parentMenu->Append( id, text, helpString, check );
}

void MyFrame::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
    if ( m_canvas->GetDrawing()->GetCommandProcessor() )
        m_canvas->GetDrawing()->GetCommandProcessor()->Undo();
}

void MyFrame::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
    if ( m_canvas->GetDrawing()->GetCommandProcessor() )
        m_canvas->GetDrawing()->GetCommandProcessor()->Redo();
}

void MyFrame::SetSwitchY( wxCommandEvent& WXUNUSED( event ) )
{
    m_canvas->SetYaxis( !m_canvas->GetYaxis() );
    GetMenuBar()->Check( OPTION_SWITCHYAXIS, m_canvas->GetYaxis() );
    m_contr->Zoomout();
}

void MyFrame::SetPolygonTool( wxCommandEvent& event )
{
    a2dDrawPolygonLTool* draw = new a2dDrawPolygonLTool( m_contr );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );

    a2dFill fill = a2dFill( wxColour( 29, 245, 164 ), a2dFILL_HORIZONTAL_HATCH );

    draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 239, 5, 64 ), 20.0, a2dSTROKE_LONG_DASH );
	stroke.SetJoin( wxJOIN_BEVEL );
    //wxJOIN_MITER,
    //wxJOIN_ROUND
	stroke.SetCap( wxCAP_PROJECTING );
	//wxCAP_ROUND
    //wxCAP_PROJECTING
    //wxCAP_BUTT


    draw->SetStroke( stroke );

    m_contr->PushTool( draw );
}

void MyFrame::SetRectangleTool( wxCommandEvent& event )
{
	a2dRect* r = new a2dRect( 0, 0, 80, 100 );
    r->SetStroke( wxColour( 239, 215, 64 ), 2.0, a2dSTROKE_LONG_DASH );
    r->SetFill( a2dFill( wxColour( 29, 25, 164 ), a2dFILL_HORIZONTAL_HATCH ) );
    r->SetContourWidth( 20 );
    a2dCanvasObject::PROPID_Allowrotation->SetPropertyToObject( r, false );
    a2dCanvasObject::PROPID_Allowskew->SetPropertyToObject( r, false );
    a2dRect::PROPID_AllowRounding->SetPropertyToObject( r, false );

    a2dDrawRectangleTool* draw = new a2dDrawRectangleTool( m_contr, r );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    draw->SetEditAtEnd( true );

    // can still overrule style of tool
    //a2dFill fill = a2dFill( wxColour( 29, 245, 164 ), a2dFILL_HORIZONTAL_HATCH );
    //draw->SetFill( fill );

    //a2dStroke stroke = a2dStroke( wxColour( 239, 5, 64 ), 20.0, a2dSTROKE_LONG_DASH );
    //draw->SetStroke( stroke );

    m_contr->PushTool( draw );
}

void MyFrame::SetEditTool( wxCommandEvent& event )
{
    a2dRecursiveEditTool* draw = new a2dRecursiveEditTool( m_contr );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );

    a2dFill fill = a2dFill( wxColour( 29, 245, 164 ), a2dFILL_HORIZONTAL_HATCH );

    draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 239, 5, 64 ), 20.0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );

    m_contr->PushTool( draw );
}

void MyFrame::SetDragNewTool( wxCommandEvent& event )
{
    a2dArrow* arrow = new  a2dArrow( 0, 0, 220, 190, 170 );
    arrow->SetFill( wxColour( 29, 215, 6 ) );
    arrow->SetStroke( wxColour( 90, 30, 205 ), 1.0 );

    a2dDragNewTool* draw = new a2dDragNewTool( m_contr, arrow );
    draw->SetShowAnotation( true );
    a2dFill fill = a2dFill( wxColour( 229, 25, 164 ), a2dFILL_HORIZONTAL_HATCH );
    fill.SetAlpha( 200 );
    a2dStroke stroke = a2dStroke( wxColour( 29, 235, 64 ), 10.0, a2dSTROKE_LONG_DASH );

    draw->SetEditAtEnd( true );
    draw->SetOneShot();

    draw->SetFill( fill );
    draw->SetStroke( stroke );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );

    m_contr->PushTool( draw );
}

void MyFrame::EndTool( wxCommandEvent& event )
{
    a2dSmrtPtr< a2dBaseTool > tool;
    m_contr->PopTool( tool );
}


void MyFrame::OnExit( wxCommandEvent& event )
{
    //i don't want to save no matter what
    //BUT you might want to save it.
    m_canvas->GetDrawing()->Modify( false );
    Close( true );
}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    // this is a top level window, so the real deletion will take place in idle time.
    Destroy();
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "a2dCanvas Single View Demo\nAuthor: Klaas Holwerda" ), wxT( "About Single View" ) );
}

//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------

IMPLEMENT_APP( MyApp )

MyApp::MyApp()
{
        //_CrtSetBreakAlloc(16428);
}

bool MyApp::OnInit()
{
    wxInitAllImageHandlers();

    wxString artroot = a2dGeneralGlobals->GetWxArt2DVar();
    if ( artroot.IsEmpty() )
        return false;

    a2dGlobals->GetFontPathList().Add( artroot + wxT( "samples/editor/common/fonts" ) );
    a2dGlobals->GetFontPathList().Add( wxT( "/usr/share/fonts/truetype/msttcorefonts" ) );
    a2dGlobals->GetImagePathList().Add( artroot +  wxT( "samples/editor/common/images" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "art/resources" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./" ) );
    a2dCanvasGlobals->GetLayersPathList().Add( artroot +  wxT( "art/layers" ) );

    a2dCentralCanvasCommandProcessor* docmanager = new a2dCentralCanvasCommandProcessor();
    a2dDocviewGlobals->SetDocviewCommandProcessor( docmanager );

#ifdef SIMPLE
    a2dWindowConnector<a2dDocumentViewScrolledWindow>* singleconnector = new a2dWindowConnector<a2dDocumentViewScrolledWindow>();
    //a2dScrolledWindowViewConnector* singleconnector = new a2dScrolledWindowViewConnector();
#else
    a2dConnector* singleconnector = new a2dConnector();
#endif

    singleconnector->SetInitialPosition( wxPoint( 50, 40 ) );
    singleconnector->SetInitialSize( wxSize( 200, 300 ) );

    a2dDocumentTemplate* doctemplatenew;
    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate(
        wxT( "CVG Drawing" ),
        wxT( "*.cvg" ),
        wxT( "" ),
        wxT( "cvg" ),
        wxT( "a2dCanvasDocument" ),
        CLASSINFO( a2dCanvasDocument ),
        singleconnector,
        a2dTemplateFlag::DEFAULT
    );

    docmanager->AssociateDocTemplate( doctemplatenew );

    a2dViewTemplate* viewtemplatenew;
    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "CVG Drawing" ),
        wxT( "a2dCanvasDocument" ),
        wxT( "Drawing View" ),
        CLASSINFO( a2dCanvasView ),
        singleconnector,
        a2dTemplateFlag::DEFAULT
    );

    docmanager->AssociateViewTemplate( viewtemplatenew );

    //// Create the main frame window
    m_frame = new MyFrame( singleconnector, NULL, -1, wxT( "Single with docmanager" ), wxPoint( 10, 110 ), wxSize( 500, 400 ), wxDEFAULT_FRAME_STYLE  );
    m_frame->SetDestroyOnCloseView( false );

    m_frame->Show( true );
    SetTopWindow( m_frame );

    return true;
}

int MyApp::OnExit()
{
    return 0;
}

