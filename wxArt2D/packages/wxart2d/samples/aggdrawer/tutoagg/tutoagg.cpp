/*! \file aggdrawer/samples/tutoagg/simple.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: tutoagg.cpp,v 1.13 2009/09/26 19:01:19 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "tutoagg.h"

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <wx/canvas/canimage.h>

//----------------------------------------------------------------------------
// MyCircle
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( MyCircle, a2dCircle )
    EVT_CANVASOBJECT_MOUSE_EVENT( MyCircle::OnMouseEvent )
END_EVENT_TABLE()

MyCircle::MyCircle( double x, double y, double radius ):
    a2dCircle( x, y, radius )
{
    /*
        a2dStringProperty* ppp = new a2dStringProperty("tipproperty","tip of mycirle");
        ppp->SetVisible( false );

        a2dVisibleProperty* pppvis = new a2dVisibleProperty( ppp, 0, radius + 12, true,
                          wxFont( 18, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL ),30,0);

        pppvis->SetFill( wxColour(219,215,6), a2dFILL_SOLID );
        AddProperty( ppp );
        Append( pppvis );
    */
}

void MyCircle::OnMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    /*
        wxPoint pos = event.GetPosition();

        if ( event.GetEventType() == wxEVT_ENTER_WINDOW )
        {
             GetProperty( "tipproperty")->SetVisible( true );
             SetPending( true );
        }
        else if ( event.GetEventType() == wxEVT_LEAVE_WINDOW )
        {
             GetProperty( "tipproperty")->SetVisible( false );
             SetPending( true );
        }

        if (event.MiddleDown())
        {
            wxMessageBox(" arrived in MyCircle ","event test",wxOK,NULL,pos.x,pos.y);
            event.Skip();
        }
        else
            event.Skip();
    */
}

//----------------------------------------------------------------------------
// a2dPaintConnector
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dPaintConnector,  a2dViewConnector )
    EVT_POST_CREATE_VIEW( a2dPaintConnector::OnPostCreateView )
    EVT_PRE_ADD_DOCUMENT( a2dPaintConnector::OnPreAddCreateDocument )
    EVT_POST_CREATE_DOCUMENT( a2dPaintConnector::OnPostCreateDocument )
    EVT_DISCONNECT_VIEW( a2dPaintConnector::OnDisConnectView )
END_EVENT_TABLE()

a2dPaintConnector::a2dPaintConnector()
    :  a2dViewConnector()
{
	m_display = NULL;
}

void a2dPaintConnector::OnPostCreateView( a2dTemplateEvent& event )
{
}

void a2dPaintConnector::OnPreAddCreateDocument( a2dTemplateEvent& event )
{
    a2dDocument* newdoc = event.GetDocument();
    a2dCanvasDocument* current = ( a2dCanvasDocument* ) a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentDocument();
    if ( current )
        current->Close( true );
}

void a2dPaintConnector::OnPostCreateDocument( a2dTemplateEvent& event )
{
    //a2dDocumentTemplate* templ = (a2dDocumentTemplate*) event.GetEventObject();
    if ( !m_display )
    {

        a2dViewTemplate* viewtempl;
        viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates().front();

        m_view = ( a2dCanvasView* ) viewtempl->CreateView( event.GetDocument(), event.GetFlags() );
        // if you do not want a template to create a view for you, you can do this
        //m_view = new a2dCanvasView( a2dCanvasGlobals->m_initialDocChildFrameSize );

        m_view->SetPrintTitle( false );
        m_view->SetPrintFilename( false );
        m_view->SetPrintFrame( false );

    }
    event.SetView( m_view );
    m_view->SetDocument( event.GetDocument() );

	m_view->SetDisplayWindow( m_display );

    if ( m_display )
    {
 		if ( m_view->GetDrawingPart()->GetCanvasToolContr() )
			m_view->GetDrawingPart()->GetCanvasToolContr()->StopAllTools();

		if ( m_view->GetDrawingPart()->GetCanvasToolContr() )
			m_view->GetDrawingPart()->GetCanvasToolContr()->ReStart();

		m_view->GetDrawingPart()->SetMappingShowAll();
	}
	m_view->Activate( true );
}

void a2dPaintConnector::OnDisConnectView(  a2dTemplateEvent& event )
{
    a2dSmrtPtr<a2dCanvasView> theViewToDisconnect = ( a2dCanvasView* ) event.GetEventObject();

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
        // not exiting, we do reuse the view, and set it do a document already open.
        // If non open we create a new one.
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

BEGIN_EVENT_TABLE( MyFrame, a2dDocumentFrame )
    EVT_MENU( wxID_NEW, MyFrame::OnFileNew )
    EVT_MENU( wxID_EXIT, MyFrame::OnExit )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_MENU( REFRESH, MyFrame::Refresh )
    EVT_MENU( SET_POLYTOOL, MyFrame::SetPolygonTool )
    EVT_MENU( SET_EDITTOOL, MyFrame::SetEditTool )
    EVT_MENU( SET_RECTTOOL, MyFrame::SetRectangleTool )
    EVT_MENU( SET_ELLIPTOOL, MyFrame::SetEllipseTool )
    EVT_MENU( SET_IMAGETOOL, MyFrame::SetImageTool )
    EVT_MENU( SET_TEXTTOOL, MyFrame::SetTextTool )
    EVT_MENU( SET_DELTOOL, MyFrame::SetDeleteTool )
    EVT_MENU( END_TOOL, MyFrame::EndTool )
    EVT_MENU( DOCCANVAS_ABOUT, MyFrame::OnAbout )
END_EVENT_TABLE()

MyFrame::MyFrame(  a2dCanvasView* canvasview, wxFrame* parent, wxWindowID id, const wxString& title,
                   const wxPoint& position, const wxSize& size, long style ) :
    a2dDocumentFrame( true, parent, canvasview, -1, title, position, size, style )
{
    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( wxT( "Welcome to a2dCanvas sample!" ) );

    SetIcon( wxICON( mondrian ) );

    int wx, wy;
    wx = 400;
    wy = 400;

    m_view = canvasview;


    m_canvas = new a2dCanvas( this,  -1 , wxPoint( 0, 0 ), wxSize( wx, wy ), wxHSCROLL | wxVSCROLL  );
	m_view->SetDisplayWindow( m_canvas );

#if wxART2D_USE_AGGDRAWER
        a2dDrawer2D* drawer2d = new a2dAggDrawer( a2dCanvasGlobals->m_initialDocChildFrameSize );
#else //wxART2D_USE_AGGDRAWER
        a2dDrawer2D* drawer2d = new a2dMemDcDrawer( a2dCanvasGlobals->m_initialDocChildFrameSize );
#endif //wxART2D_USE_AGGDRAWER
        //OR
        //a2dDrawer2D* drawer2d = new a2dAggDrawer( a2dCanvasGlobals->m_initialDocChildFrameSize );
    m_canvas->GetDrawingPart()->SetDrawer2D( drawer2d );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->SetMappingUpp( -200, -200, 3, 3 );
    m_canvas->SetYaxis( true );
    m_canvas->FixScrollMaximum( false );
    //defines the whole area in world coordinates that can be seen by scrolling
    m_canvas->SetScrollMaximum( 0, 0, 1600, 1600 );
    //defines the number of world units scrolled when line up or down events in X
    m_canvas->SetScrollStepX( 100 );
    //defines the number of world units scrolled when line up or down events in Y
    m_canvas->SetScrollStepY( 100 );

    a2dFill back = a2dFill( wxColour( 235, 235, 235 ), a2dFILL_SOLID );
    m_canvas->SetBackgroundFill( back );

    a2dCanvasObjectPtr root = m_canvas->GetDrawing()->GetRootObject();

    a2dPolygonL* poly = new a2dPolygonL();
    poly->AddPoint( -30, -20 );
    poly->AddPoint( 100, 0 );
    poly->AddPoint( 100, 100 );
    poly->AddPoint( 50, 150 );
    poly->AddPoint( 0, 100 );
    poly->SetFill( wxColour( 100, 17, 255, 50 ), a2dFILL_CROSSDIAG_HATCH );
    poly->SetStroke( wxColour( 9, 115, 64 ), 4.0 );
    root->Prepend( poly );
    poly->Rotate( 180 );

    a2dPolygonL* poly2 = new a2dPolygonL();
    poly2->AddPoint( 300, 100 );
    poly2->AddPoint( 350, 150 );
    poly2->AddPoint( 400, 100 );
    poly2->AddPoint( 400, 0 );
    poly2->AddPoint( 270, -20 );
    poly2->SetFill( wxColour( 100, 17, 255, 50 ), a2dFILL_CROSSDIAG_HATCH );
    poly2->SetStroke( wxColour( 9, 115, 64 ), 4.0 );
    root->Prepend( poly2 );


    a2dRect* r = new a2dRect( 100, 100, 300, 700 );
    r->SetContourWidth( 30 );
    r->SetFill( wxColour( 100, 17, 255, 50 ), a2dFILL_CROSSDIAG_HATCH );
    r->SetStroke( wxColour( 9, 115, 64 ), 4.0 );
    root->Append( r );

    a2dPolylineL* polyl = new a2dPolylineL();
    polyl->AddPoint( -130, -20 );
    polyl->AddPoint( 200, 0 );
    polyl->AddPoint( 200, 100 );
    polyl->AddPoint( 250, 150 );
    polyl->AddPoint( 100, 100 );
    a2dStroke str = a2dStroke( wxColour( 9, 115, 164 ), 34.0 );
    polyl->SetStroke( str );
    str.SetJoin( wxJOIN_MITER );
    str.SetCap( wxCAP_PROJECTING );
    root->Prepend( polyl );

    a2dSLine* ll = new a2dSLine( -50, 0, 400, 70 );
    ll->SetStroke( wxColour( 252, 0, 252 ), 0 );
    root->Append( ll );

    a2dSLine* ll2 = new a2dSLine( -150, 100, 400, 270 );
    ll2->SetStroke( wxColour( 2, 230, 252 ), 1.0 );
    root->Append( ll2 );

    int i;
    for( i = 0; i < 10; i++ )
    {
        a2dRectC* r = new a2dRectC( rand() % 1000 - 400, rand() % 1000 - 400, 30 + rand() % 100, 20 + rand() % 300 );
        root->Append( r );
    }

    for( i = 0; i < 20; i++ )
    {
        a2dRect* r = new a2dRect( rand() % 1000 - 400, rand() % 1000 - 400, 30 + rand() % 100, 20 + rand() % 300 );
        r->SetFill( wxColour( rand() % 254, rand() % 254, rand() % 254, 50 ) );
        r->SetStroke( wxColour( rand() % 254, rand() % 254, rand() % 254, 170 ), double( rand() % 10 ) );
        r->SetLayer( 5 );
        root->Append( r );
    }

    MyCircle* cir2 = new  MyCircle( 230, 50, 160 );
    cir2->SetFill( wxColour( 255, 215, 6, 100 ), a2dFILL_HORIZONTAL_HATCH );
    cir2->SetStroke( wxColour( 198, 3, 105 ), 30.0 );
    root->Prepend( cir2 );
    wxBitmap gs_bmp36;
    wxString path = a2dGlobals->GetImagePathList().FindValidPath( _T( "arc.bmp" ) );
    gs_bmp36.LoadFile( path, wxBITMAP_TYPE_BMP );
    wxASSERT_MSG( gs_bmp36.Ok(), _T( " invalid bitmap" ) );

    a2dFill  aa = a2dFill( gs_bmp36 );
    aa.SetColour( wxColour( 250, 78, 216, 210 ) );
    aa.SetColour2( *wxRED );
    aa.SetStyle( a2dFILL_STIPPLE );

    MyCircle* cir3 = new  MyCircle( 530, 50, 260 );
    cir3->SetFill( aa );
    cir3->SetStroke( wxColour( 198, 3, 105 ), 30.0 );
    root->Prepend( cir3 );

    wxBitmap gs_bmp;
    path = a2dGlobals->GetImagePathList().FindValidPath( _T( "smaller.png" ) );
    gs_bmp.LoadFile( path, wxBITMAP_TYPE_PNG );
    wxASSERT_MSG( gs_bmp.Ok(), _T( " invalid bitmap" ) );

    a2dFill  aba = a2dFill( gs_bmp );
    aba.SetColour( wxColour( 250, 78, 216 ) );
    aba.SetColour2( *wxRED );
    aba.SetStyle( a2dFILL_STIPPLE );
    aba.SetAlpha( 190 );

    MyCircle* cir4 = new  MyCircle( 730, 50, 260 );
    cir4->SetFill( aba );
    cir4->SetStroke( wxColour( 198, 3, 105 ), 30.0 );
    cir4->SetObjectTip(  wxT( "make my day punk" ), 0, 50, 40, 0,  a2dFont( 80.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ) );
    root->Prepend( cir4 );

    wxImage rgba;
    path = a2dGlobals->GetImagePathList().FindValidPath( _T( "transp.png" ) );
    rgba.LoadFile( path );
    wxASSERT_MSG( rgba.Ok(), _T( " invalid image" ) );
    a2dRgbaImage* im = new a2dRgbaImage(  200, 100, rgba, 255 );
    root->Append( im );


    a2dVectorPath* path6 = new a2dVectorPath();
    path6->SetStroke( wxColour( 29, 235, 245 ), 22.0 );
    path6->SetFill( wxColour( 130, 21, 72 ) );
    path6->SetFill( wxColour( 130, 21, 72 ), a2dFILL_VERTICAL_HATCH );

    path6->Add( new a2dVpathSegment( 100, 150, a2dPATHSEG_MOVETO ) );
    path6->Add( new a2dVpathSegment( 100, 550, a2dPATHSEG_LINETO ) );
    path6->Add( new a2dVpathSegment( 500, 550, a2dPATHSEG_LINETO ) );
    path6->Add( new a2dVpathSegment( 500, 150, a2dPATHSEG_LINETO ) );
    path6->Add( new a2dVpathSegment( 100, 150, a2dPATHSEG_LINETO ) );
    path6->Add( new a2dVpathSegment( 150, 200, a2dPATHSEG_LINETO_NOSTROKE ) );
    path6->Add( new a2dVpathSegment( 450, 200, a2dPATHSEG_LINETO ) );
    path6->Add( new a2dVpathSegment( 450, 500, a2dPATHSEG_LINETO ) );
    path6->Add( new a2dVpathSegment( 150, 500, a2dPATHSEG_LINETO ) );
    path6->Add( new a2dVpathSegment( 150, 200, a2dPATHSEG_LINETO ) );
    path6->Add( new a2dVpathSegment( 100, 150, a2dPATHSEG_LINETO_NOSTROKE, a2dPATHSEG_END_CLOSED ) );

    root->Append( path6 );

    a2dText* tt = new a2dText( wxT( "Hello from wxWorldCanvas use\n Ctrl LeftDown to drag \n popup to zoom" ), -50, -70,
                               a2dFont( 80.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL ) );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( wxColour( 0, 155, 120, 100 ) );
    root->Append( tt );

    a2dFont lorem( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Regular.ttf" ) ), 93.0 );
    //a2dFont lorem( a2dFont( 3.0, wxFONTFAMILY_SWISS, wxNORMAL ) );
    //a2dFont lorem( a2dGlobals->GetFontPathList().FindValidPath( wxT("/LiberationSerif-Regular.ttf") ),3.0 );

    a2dFont lorem2( a2dGlobals->GetFontPathList().FindValidPath( wxT( "/LiberationSans-Regular.ttf" ) ), 93.0 );
    a2dText* text3 = new a2dText( "And how about this", -30, -70, lorem2 );
    text3->SetFill( wxColour( 0, 30, 200, 100 ) );
    text3->SetBackGround();
    text3->SetStroke( wxColour( 0, 15, 12, 100 ) );
    root->Append( text3 );


    a2dText* tt2 = new a2dText( wxT( "Can have background too" ), -20, -40,
                                a2dFont( 40.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL ) );
    tt2->SetFill( wxColour( 0, 10, 200, 100 ) );
    tt2->SetBackGround();
    tt2->SetStroke( wxColour( 0, 15, 12, 100 ) );
    root->Append( tt2 );

    //m_canvas->SetShowObject( root );

    //next  line not strictly needed (depends on how you fill the a2dDrawing)
    //root->SetDrawingRootRecursive();

#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
    m_contr->SetUseOpaqueEditcopy( true );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
#endif //wxART2D_USE_EDITOR
}


MyFrame::~MyFrame()
{
}

void MyFrame::CreateMyMenuBar()
{
    wxMenu* file_menu = new wxMenu;

    wxMenuBar* menu_bar = new wxMenuBar();

    // if you use this here, it will create extra document, leaving old documents in memory
    //AddCmdMenu( file_menu, CmdMenu_FileNew );

    file_menu->Insert( 0, wxID_NEW, _( "&New..." ), _( "Create new file" ) );

    AddCmdMenu( file_menu, CmdMenu_FileOpen() );
    AddCmdMenu( file_menu, CmdMenu_FileClose() );
    AddCmdMenu( file_menu, CmdMenu_FileSave() );
    AddCmdMenu( file_menu, CmdMenu_FileSaveAs() );
    AddCmdMenu( file_menu, CmdMenu_Print() );
    file_menu->AppendSeparator();
    AddCmdMenu( file_menu, CmdMenu_PrintDrawing() );
    AddCmdMenu( file_menu, CmdMenu_PreviewDrawing() );

    file_menu->AppendSeparator();
    file_menu->Append( wxID_EXIT, _( "E&xit" ) );

    m_edit_menu = new wxMenu;
    m_edit_menu->Append( wxID_UNDO, _( "&Undo" ) );
    m_edit_menu->Append( wxID_REDO, _( "&Redo" ) );
    m_edit_menu->Append( REFRESH, _( "&refresh" ) ) ;
    m_edit_menu->Append( SET_POLYTOOL, _( "set &polygon tool" ) );
    m_edit_menu->Append( SET_RECTTOOL, _( "set &rectangle tool" ) );
    m_edit_menu->Append( SET_ELLIPTOOL, _( "set ellipses tool" ) );
    m_edit_menu->Append( SET_IMAGETOOL, _( "set &image tool" ) );
    m_edit_menu->Append( SET_TEXTTOOL, _( "set &text tool" ) );
    m_edit_menu->Append( SET_EDITTOOL, _( "set &edit tool" ) );
    m_edit_menu->Append( SET_DELTOOL, _( "set &delete tool" ) );
    m_edit_menu->Append( END_TOOL, _( "&endtool" ) );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( DOCCANVAS_ABOUT, _( "&About" ) );

    menu_bar->Append( file_menu, _( "&File" ) );

    menu_bar->Append( m_edit_menu, _( "&Edit" ) );

    menu_bar->Append( help_menu, _( "&Help" ) );

    SetMenuBar( menu_bar );
}

void MyFrame::Refresh( wxCommandEvent& event )
{
    m_canvas->Refresh();
}

void MyFrame::OnFileNew( wxCommandEvent& event )
{
    //closing a file automatically in a2dPaintConnector creates a new document, unless exiting
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileClose();
    event.Skip();
}

void MyFrame::OnExit( wxCommandEvent& event )
{
    //a2dDocument* doc = m_view->GetDocument();
    //next will exit without question.
    //doc->Modify(false);
    Close( true );
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

void MyFrame::SetPolygonTool( wxCommandEvent& event )
{
#if wxART2D_USE_EDITOR
    a2dDrawPolygonLTool* draw = new a2dDrawPolygonLTool( m_contr );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    draw->SetShowAnotation( false );


    wxBitmap gs_bmp36;
    wxString path = a2dGlobals->GetImagePathList().FindValidPath( _T( "arc.bmp" ) );
    gs_bmp36.LoadFile( path, wxBITMAP_TYPE_BMP );
    wxASSERT_MSG( gs_bmp36.Ok(), _T( " invalid bitmap" ) );

    a2dFill  aa = a2dFill( gs_bmp36 );
    aa.SetColour( wxColour( 250, 78, 216 ) );
    aa.SetColour2( *wxRED );
    aa.SetStyle( a2dFILL_STIPPLE );
    aa.SetAlpha( 50 );
    draw->SetFill( aa );

    //a2dFill fill = a2dFill(wxColour(29,245,164,200 ),a2dFILL_HORIZONTAL_HATCH );

    //draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 239, 5, 64 ), 20.0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );
    draw->SetEditAtEnd( true );

    m_contr->PushTool( draw );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::SetEditTool( wxCommandEvent& event )
{
#if wxART2D_USE_EDITOR
    a2dRecursiveEditTool* draw = new a2dRecursiveEditTool( m_contr );
    draw->SetShowAnotation( false );
    a2dFill fill = a2dFill( wxColour( 29, 225, 164, 230 ), a2dFILL_SOLID );

    draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 39, 235, 64 ), 5.0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );

    m_contr->PushTool( draw );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::SetImageTool( wxCommandEvent& event )
{
#if wxART2D_USE_EDITOR
    a2dImageTool* draw = new a2dImageTool( m_contr );
    draw->SetShowAnotation( false );

    //a2dFill fill = a2dFill(wxColour(229,25,164,200 ),a2dFILL_HORIZONTAL_HATCH );
    //draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 29, 235, 64 ), 10.0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );
    draw->SetEditAtEnd( true );

    m_contr->PushTool( draw );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::SetTextTool( wxCommandEvent& event )
{
#if wxART2D_USE_EDITOR
    a2dDrawTextTool* draw = new a2dDrawTextTool( m_contr );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    draw->SetShowAnotation( false );
    draw->GetTemplateObject()->SetFont( a2dFont( wxT( "LiberationSerif-Italic.ttf" ), 150.0 ) );
    //draw->GetTemplateObject()->SetFont( a2dFont( wxT("LiberationSerif-Regular.ttf"), 200 ) );

#if defined(__WXMSW__) && !defined(__GNUWIN32__)
    draw->GetTemplateObject()->SetFont( a2dFont( wxFont( 18, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL ), 200 ) );
#endif // defined( __WXMSW__ )

    //a2dFill fill = a2dFill(wxColour(229,25,164,200 ),a2dFILL_HORIZONTAL_HATCH );
    //draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 29, 235, 64 ), 10.0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );
    draw->SetEditAtEnd( true );

    m_contr->PushTool( draw );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::SetRectangleTool( wxCommandEvent& event )
{
#if wxART2D_USE_EDITOR
    a2dDrawRectangleTool* draw = new a2dDrawRectangleTool( m_contr );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    draw->SetShowAnotation( false );

    wxBitmap gs_bmp36;
    wxString path = a2dGlobals->GetImagePathList().FindValidPath( _T( "smaller.png" ) );
    gs_bmp36.LoadFile( path, wxBITMAP_TYPE_PNG );
    wxASSERT_MSG( gs_bmp36.Ok(), _T( " invalid bitmap" ) );
    a2dFill  aa = a2dFill( gs_bmp36 );
    aa.SetColour( wxColour( 250, 78, 216 ) );
    aa.SetColour2( *wxRED );
    aa.SetStyle( a2dFILL_STIPPLE );
    aa.SetAlpha( 150 );
    draw->SetFill( aa );

    //a2dFill fill = a2dFill(wxColour(229,25,164,200 ),a2dFILL_HORIZONTAL_HATCH );
    //draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 29, 235, 64 ), 10.0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );
    draw->SetEditAtEnd( true );

    m_contr->PushTool( draw );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::SetEllipseTool( wxCommandEvent& event )
{
#if wxART2D_USE_EDITOR
    a2dDrawEllipseTool* draw = new a2dDrawEllipseTool( m_contr );
    draw->SetEvtHandler( new a2dStToolFixedToolStyleEvtHandler( m_contr ) );
    draw->SetShowAnotation( false );


    wxBitmap gs_bmp36;
    wxString path = a2dGlobals->GetImagePathList().FindValidPath( _T( "arc.bmp" ) );
    gs_bmp36.LoadFile( path, wxBITMAP_TYPE_BMP );
    wxASSERT_MSG( gs_bmp36.Ok(), _T( " invalid bitmap" ) );

    a2dFill  aa = a2dFill( gs_bmp36 );
    aa.SetColour( wxColour( 250, 78, 216 ) );
    aa.SetColour2( *wxRED );
    aa.SetStyle( a2dFILL_STIPPLE );
    aa.SetAlpha( 50 );
    draw->SetFill( aa );

    //a2dFill fill = a2dFill(wxColour(229,25,14,200 ),a2dFILL_HORIZONTAL_HATCH );
    //draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 29, 25, 164 ), 30.0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );
    draw->SetEditAtEnd( true );

    m_contr->PushTool( draw );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::SetDeleteTool( wxCommandEvent& event )
{
#if wxART2D_USE_EDITOR
    a2dDeleteTool* del = new a2dDeleteTool( m_contr );

    m_contr->PushTool( del );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::EndTool( wxCommandEvent& event )
{
#if wxART2D_USE_EDITOR
    a2dSmrtPtr< a2dBaseTool > tool;
    m_contr->PopTool( tool );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "a2dCanvas ArtLib Drawer Demo\nAuthor: Klaas Holwerda" ), wxT( "About ArtLib Drawer" ) );
}

//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------


IMPLEMENT_APP( MyApp )

MyApp::MyApp()
{
}

bool MyApp::OnInit()
{
    a2dGlobals->GetFontPathList().Add( wxT( "." ) );
    a2dGlobals->GetImagePathList().Add( wxT( "." ) );
    a2dGlobals->GetFontPathList().Add( wxT( ".." ) );
    a2dGlobals->GetImagePathList().Add( wxT( ".." ) );

    wxInitAllImageHandlers();

    a2dCentralCanvasCommandProcessor* docmanager = new a2dCentralCanvasCommandProcessor();
    a2dDocviewGlobals->SetDocviewCommandProcessor( docmanager );

    a2dPaintConnector* m_singleconnector = new a2dPaintConnector();

    a2dIOHandlerCVGIn* cvghin = new a2dIOHandlerCVGIn();
    a2dIOHandlerCVGOut* cvghout = new a2dIOHandlerCVGOut();

    a2dDocumentTemplate* doctemplatenew;
    doctemplatenew = new a2dDocumentTemplate( wxT( "CVG Drawing" ), wxT( "*.cvg" ), wxT( "" ), wxT( "cvg" ), wxT( "a2dCanvasDocument" ),
            CLASSINFO( a2dCanvasDocument ), m_singleconnector, a2dTemplateFlag::DEFAULT, cvghin, cvghout );
    docmanager->AssociateDocTemplate( doctemplatenew );

    a2dViewTemplate* viewtemplatenew;
    viewtemplatenew = new a2dViewTemplate( wxT( "CVG Drawing" ), wxT( "a2dCanvasDocument" ), wxT( "Drawing View" ),
                                           CLASSINFO( a2dCanvasView ), m_singleconnector, a2dTemplateFlag::DEFAULT, a2dCanvasGlobals->m_initialDocChildFrameSize );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    docmanager->CreateDocuments( wxT( "dummy" ), a2dREFDOC_NEW );
    a2dCanvasDocument* document = ( a2dCanvasDocument* ) docmanager->GetDocuments().back().Get();

    m_frame = new MyFrame( m_singleconnector->m_view, NULL, -1, wxT( "SuperApp" ), wxPoint( 20, 30 ), wxSize( 300, 240 ) );

    a2dCanvas* canvas = m_frame->GetCanvas();
    m_frame->Show( true );
    m_singleconnector->SetDisplayWindow( canvas );
    canvas->SetMappingShowAll();

    a2dDocviewGlobals->GetVariablesHash().SetVariableString( wxT( "wxart2dlayers" ), wxT( "maskproclayers.cvg" ) );
    a2dDocviewGlobals->GetVariablesHash().SetVariableString( wxT( "layerFileSavePath" ), wxT( "C:/tmp" ) );

    return true;
}

int MyApp::OnExit()
{
    return 0;
}



