/*! \file editor/samples/viewmul/docv.cpp
    \brief Document/view demo
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: docv.cpp,v 1.18 2009/09/26 19:01:02 titato Exp $
*/

/*
* Purpose:  Document/view architecture demo for wxWindows class library
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include <wx/memory.h>

#include "docv.h"
//#include "doc.h"
#include "view.h"
#include "document.h"
#include "elements.h"
#include "part.h"


#include "wx/editor/recurdoc.h"
#include "wx/editor/xmlparsdoc.h"

// In single window mode, don't have any child windows; use
// main window.
bool singleWindowMode = false;


IMPLEMENT_APP( MyApp )

MyApp::MyApp( void )
{
}

bool MyApp::OnInit( void )
{
    int pinwidth = 14;//7; //pixels

    a2dCentralCanvasCommandProcessor* docmanager = new a2dCentralCanvasCommandProcessor();
    a2dDocviewGlobals->SetDocviewCommandProcessor( docmanager );



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
    Pin::ElementObject->SetPinCannotConnect( defPin2 );
    Pin::ElementWire->SetPin( defPin ) ;
    Pin::ElementWire->SetPinCanConnect( defPin3 );
    Pin::ElementWire->SetPinCannotConnect( defPin2 );
    Pin::ElementObject->SetConnectionGenerator( elecon );
    Pin::ElementWire->SetConnectionGenerator( elecon );
    Pin::ElementObject->AddConnect( Pin::ElementWire );
    Pin::ElementObject->AddConnect( Pin::ElementObject );
    Pin::ElementWire->AddConnect( Pin::ElementObject );
    Pin::ElementWire->AddConnect( Pin::ElementWire );

    // define which connector should be used for connecting to the automatically generated on pin a2dPinClass::Any
    a2dPinClass::Any->SetConnectionGenerator( elecon );

    ::wxInitAllImageHandlers();
    //wxDebugContext::SetCheckpoint();

    m_connector = new a2dMultiDrawingConnector();
    m_connector->SetInitialPosition( wxPoint( 50, 40 ) );
    m_connector->SetInitialSize( wxSize( 200, 300 ) );

#if wxART2D_USE_CVGIO
    a2dIOHandlerDocCVGIn* cvghin = new a2dIOHandlerDocCVGIn();
    a2dIOHandlerDocCVGOut* cvghout = new a2dIOHandlerDocCVGOut();
#endif //wxART2D_USE_CVGIO

    a2dDocumentTemplate* doctemplatenew;
#if wxART2D_USE_CVGIO
    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate( wxT( "Mars Document" ), wxT( "*.mrs" ), wxT( "" ), wxT( "drw" ), wxT( "msDocument" ),
            CLASSINFO( msDocument ), m_connector, a2dTemplateFlag::DEFAULT, cvghin, cvghout );
    docmanager->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_CVGIO

    a2dViewTemplate* viewtemplatenew;
    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate( wxT( "Mars Document" ), wxT( "msDocument" ), wxT( "Drawing View Frame" ),
                                           CLASSINFO( a2dCanvasView ), m_connector, a2dTemplateFlag::DEFAULT );
    m_connector->AssociateViewTemplate( viewtemplatenew );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    //// Create the main frame window
    MyFrame* frame = new MyFrame( ( wxFrame* ) NULL, -1, wxT( "DocView Demo" ), wxPoint( 0, 0 ), wxSize( 500, 400 ), wxDEFAULT_FRAME_STYLE );

    SetTopWindow( frame );
    m_connector->Init( frame, true );

    msDocument* comp = new msDocument;
    {
        msSymbolPage* symbolpage = comp->GetSymbolPage();

        // Create properties
        Resistor::PROPID_r->SetPropertyToObject( symbolpage, wxT( "50" ) );
        Element::PROPID_spice->SetPropertyToObject( symbolpage, wxT( "R_{REFDES}    [1] [2]    {R}" ) );
        Element::PROPID_freeda->SetPropertyToObject( symbolpage, wxT( "r:{REFDES}    [1] [2]    r={R}" ) );

        // Create symbol
        a2dVertexList* array = new a2dVertexList();
        array->push_back( new a2dLineSegment( -8.0, -2.0 ) );
        array->push_back( new a2dLineSegment( -8.0, 2.0 ) );
        array->push_back( new a2dLineSegment( 8.0, 2.0 ) );
        array->push_back( new a2dLineSegment( 8.0, -2.0 ) );
        symbolpage->Append( new a2dPolygonL( array ) );

        a2dText* text;
        text = new a2dText( wxT( "@REFDES" ), 0.0, 3.0, a2dFont( 3.0, wxFONTFAMILY_SWISS ) );
        text->SetAlignment( wxMINY );
        text->SetFill( *a2dTRANSPARENT_FILL );
        text->SetEngineeringText();
        symbolpage->Append( text );

        text = new a2dText( wxT( "@R=" ), 0.0, -3.0, a2dFont( 3.0, wxFONTFAMILY_SWISS ) );
        text->SetAlignment( wxMAXY );
        text->SetFill( *a2dTRANSPARENT_FILL );
        text->SetEngineeringText();
        symbolpage->Append( text );

        msConnectorPin* pin1 = new msConnectorPin( wxT( "1" ), -12.0, 0.0 );
        symbolpage->Append( pin1 );
        msConnectorPin* pin2 = new msConnectorPin( wxT( "2" ) );
        pin2->Mirror( true, true );
        pin2->SetPosXY( 12.0, 0.0 );
        symbolpage->Append( pin2 );
    }
    comp->Save( wxT( "R.mrs" ), cvghout );
    delete comp;

    comp = new msDocument;
    {
        msSymbolPage* symbolpage = comp->GetSymbolPage();

        // Create properties
        Capacitor::PROPID_c->SetPropertyToObject( symbolpage,  wxT( "1 pF" ) );
        Element::PROPID_spice->SetPropertyToObject( symbolpage, wxT( "C_{REFDES}    [1] [2]    {C}" ) );
        Element::PROPID_freeda->SetPropertyToObject( symbolpage, wxT( "c:{REFDES}    [1] [2]    c={C} {int_g=INT_G} {time_d=TIME_D}" ) );

        // Create symbol
        symbolpage->Append( new a2dSLine( -1.0, 0.0, -4.0, 0.0 ) );
        symbolpage->Append( new a2dSLine( 1.0, 0.0, 4.0, 0.0 ) );
        symbolpage->Append( new a2dSLine( -1.0, -3.0, -1.0, 3.0 ) );
        symbolpage->Append( new a2dSLine( 1.0, -3.0, 1.0, 3.0 ) );

        a2dText* text;
        text = new a2dText( wxT( "@REFDES" ), 0.0, 4.0, a2dFont( 3.0, wxFONTFAMILY_SWISS ) );
        text->SetAlignment( wxMINY );
        text->SetFill( *a2dTRANSPARENT_FILL );
        text->SetEngineeringText();
        symbolpage->Append( text );

        text = new a2dText( wxT( "@C=" ), 0.0, -4.0, a2dFont( 3.0, wxFONTFAMILY_SWISS ) );
        text->SetAlignment( wxMAXY );
        text->SetFill( *a2dTRANSPARENT_FILL );
        text->SetEngineeringText();
        symbolpage->Append( text );

        msConnectorPin* pin1 = new msConnectorPin( wxT( "1" ), -8.0, 0.0 );
        symbolpage->Append( pin1 );
        msConnectorPin* pin2 = new msConnectorPin( wxT( "2" ) );
        pin2->Mirror( false, true );
        pin2->SetPosXY( 8.0, 0.0 );
        symbolpage->Append( pin2 );
    }
    comp->Save( wxT( "C.mrs" ), cvghout );
    delete comp;

    return true;
}

int MyApp::OnExit( void )
{

    m_connector->DisassociateViewTemplates();
    return 0;
}

//------------------------------------------------------------------------
// Overview canvas in toplevel frame for showing all open drawings "symbolic"
//------------------------------------------------------------------------

BEGIN_EVENT_TABLE( OverviewCanvas, a2dCanvas )
    EVT_MOUSE_EVENTS( OverviewCanvas::OnMouseEvent )
    EVT_ADD_DOCUMENT( OverviewCanvas::OnAddDocument )
    EVT_REMOVE_DOCUMENT( OverviewCanvas::OnRemoveDocument )
END_EVENT_TABLE()

// Define a constructor for my canvas
OverviewCanvas::OverviewCanvas( wxFrame* frame, const wxPoint& pos, const wxSize& size, const long style ):
    a2dCanvas( frame, -1, pos, size, style )
{
    m_contr = new a2dStToolContr( GetDrawingPart(), frame );
    m_contr->SetZoomFirst( true );
	a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent(wxEVT_ADD_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_REMOVE_DOCUMENT, this );
}

//Open clicked root in a MyCanvas
void OverviewCanvas::OnMouseEvent( wxMouseEvent& event )
{
    Raise();
    if ( !GetDrawing() || !GetDrawingPart()->GetMouseEvents() )
    {
        event.Skip();
        return;
    }

    if ( event.LeftDown() )
    {
        int x = event.GetX();
        int y = event.GetY();

        //to world coordinates to do hit test in world coordinates
        double xw = GetDrawer2D()->DeviceToWorldX( x );
        double yw = GetDrawer2D()->DeviceToWorldY( y );

        a2dIterC ic( GetDrawingPart() );
        a2dIterCU cu( ic );
        a2dHitEvent hitevent = a2dHitEvent( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
        a2dCanvasObject* obj = GetShowObject()->IsHitWorld( ic, hitevent );

        if ( obj && ( 0 != wxDynamicCast( obj, a2dCanvasDocumentReference ) ) )
        {
            //a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateView( ( a2dDocument* ) ( ( a2dCanvasDocumentReference* ) obj )->GetCanvasObject()->GetRoot()->GetCanvasDocument() );
        }
    }
}

OverviewCanvas::~OverviewCanvas()
{
}

void OverviewCanvas::OnAddDocument( a2dCommandEvent& event )
{
    a2dCanvasDocument* doc = ( a2dCanvasDocument* ) event.GetDocument();

    static double i = 200;

    a2dCanvasDocumentReference* tr = new a2dCanvasDocumentReference( 120, i, doc );
    tr->SetStroke( wxColour( 229, 5, 64 ), 0 );
    tr->SetFill( *a2dTRANSPARENT_FILL );
    GetDrawingPart()->GetDrawing()->GetRootObject()->Append( tr );
    GetDrawingPart()->Update( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_ALL ); //the above object gets drawn and included in bbox
    i = i + 200;

    m_contr->Zoomout();
}

void OverviewCanvas::OnRemoveDocument( a2dCommandEvent& event )
{
    a2dCanvasDocument* doc = ( a2dCanvasDocument* ) event.GetDocument();
    forEachIn( a2dCanvasObjectList, GetDrawingPart()->GetDrawing()->GetRootObject()->GetChildObjectList() )
    {
        a2dCanvasObject* obj = *iter;

        if ( 0 != wxDynamicCast( obj, a2dCanvasDocumentReference ) )
        {
            a2dCanvasDocumentReference* openfile = wxDynamicCast( obj, a2dCanvasDocumentReference );
            if ( openfile->GetDocumentReference() == doc )
            {
                GetDrawingPart()->GetDrawing()->GetRootObject()->ReleaseChild( obj );
                break;
            }
        }
    }
    GetDrawingPart()->Update( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_ALL );
}

/*
* This is the top-level window of the application.
*/
#define CANVAS_ABOUT 10000

IMPLEMENT_CLASS( MyFrame, a2dDocumentFrame )
BEGIN_EVENT_TABLE( MyFrame, a2dDocumentFrame )
    EVT_MENU( CANVAS_ABOUT, MyFrame::OnAbout )
END_EVENT_TABLE()

MyFrame::MyFrame( wxFrame* frame, wxWindowID id, const wxString& title,
                  const wxPoint& pos, const wxSize& size, const long type ):
    a2dDocumentFrame( true, frame, NULL, id, title, pos, size, type )
{
    //// Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
    SetIcon( wxIcon( wxString( wxT( "doc_icn" ) ) ) );
#endif

    //// Make a menubar
    wxMenu* file_menu = new wxMenu;
    wxMenu* edit_menu = ( wxMenu* ) NULL;

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

    file_menu->Append( wxID_EXIT, _( "E&xit" ) );

    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( file_menu );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( CANVAS_ABOUT, _( "&About" ) );

    wxMenuBar* menu_bar = new wxMenuBar;

    menu_bar->Append( file_menu, _( "&File" ) );
    if ( edit_menu )
        menu_bar->Append( edit_menu, _( "&Edit" ) );
    menu_bar->Append( help_menu, _( "&Help" ) );

    //// Associate the menu bar with the frame
    SetMenuBar( menu_bar );

    Centre( wxBOTH );
    Show( true );

    CreateStatusBar( 2 );

    int width, height;
    GetClientSize( &width, &height );

    m_topcanvas = new OverviewCanvas( this, wxPoint( 0, 0 ), size );

    a2dDrawer2D* drawer2d = new a2dMemDcDrawer( width, height );
    m_topcanvas->GetDrawingPart()->SetDrawer2D( drawer2d );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_topcanvas->SetMappingUpp( 0, 0, 4, 4 );
    m_topcanvas->SetYaxis( true );
    //defines the whole area in world coordinates that can be seen by scrolling
    m_topcanvas->SetScrollMaximum( 0, 0, 1600, 1600 );
    //defines the number of world units scrolled when line up or down events in X
    m_topcanvas->SetScrollStepX( 200 );
    //defines the number of world units scrolled when line up or down events in Y
    m_topcanvas->SetScrollStepY( 100 );

    a2dFill fill = a2dFill( wxColour( 2, 255, 255 ) );
    m_topcanvas->SetBackgroundFill( fill );

    a2dRect* tr = new a2dRect( 0, 0, 100, 100 );
    tr->SetStroke( wxColour( 229, 5, 64 ), 0 );
    tr->SetFill( wxColour( 0, 117, 245 ) );
	m_topcanvas->GetDrawing()->GetRootObject()->Append( tr );

    m_topcanvas->GetDrawingPart()->Update();

    editMenu = ( wxMenu* ) NULL;
}

MyFrame::~MyFrame()
{
    //undo stack can hold Owners to Documents etc.
    m_document->GetCommandProcessor()->ClearCommands();

    //to prevent (asking for) saving this root
    m_document->Modify( false );
    m_document->Close( true ); //also closses the view
}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    if ( event.CanVeto() )
    {
        if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear( !event.CanVeto() ) )
        {
            Destroy();
        }
        else
            event.Veto( true );
    }
    else
    {
        a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear( true );
        Destroy();
    }
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "a2dCanvas MultiView Demo\nAuthor: Klaas Holwerda" ), wxT( "About DocView" ) );
}
