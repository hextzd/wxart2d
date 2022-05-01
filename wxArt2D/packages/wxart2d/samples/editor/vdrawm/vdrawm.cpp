/*! \file editor/samples/vdrawm/docv.cpp
    \brief Document/view demo
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: vdrawm.cpp,v 1.8 2009/09/26 19:01:19 titato Exp $
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

#include "wx/canvas/canmod.h"

#if wxART2D_USE_GDSIO
#include "wx/gdsio/gdsio.h"
#endif //wxART2D_USE_GDSIO

#if wxART2D_USE_SVGIO
#include "wx/svgio/parssvg.h"
#endif //wxART2D_USE_SVGIO

#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#endif //wxART2D_USE_AGGDRAWER

#include "vdrawm.h"
#include "view.h"
#include "wx/canextobj/imageioh.h"
#include "wx/editor/recurdoc.h"

#include "screen.h"

// In single window mode, don't have any child windows; use
// main window.
bool singleWindowMode = false;

IMPLEMENT_APP( MyApp )


//------------------------------------------------------------------------
// Vdraw its application object
//------------------------------------------------------------------------

MyApp::MyApp( void )
{
}

bool MyApp::OnInit( void )
{
    wxString artroot = a2dGeneralGlobals->GetWxArt2DVar();
    if ( artroot.IsEmpty() )
        return false;

    wxInitAllImageHandlers();
    a2dMenuIdItem::InitializeBitmaps();

    a2dCentralCanvasCommandProcessor* docManager = new a2dCentralCanvasCommandProcessor();
    a2dDocviewGlobals->SetDocviewCommandProcessor( docManager );

#if wxUSE_DEBUG_CONTEXT
    wxDebugContext::SetCheckpoint();
#endif

    a2dGlobals->GetFontPathList().Add( artroot + wxT( "samples/editor/common/fonts" ) );
    a2dGlobals->GetImagePathList().Add( artroot +  wxT( "samples/editor/common/images" ) );
    a2dGlobals->GetFontPathList().Add(  artroot +  wxT( "samples/editor/common/fonts" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "art/resources" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "apps/teto/config" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./" ) );
    a2dCanvasGlobals->GetLayersPathList().Add( artroot +  wxT( "art/layers" ) );

    a2dCanvasGlobals->GetHabitat()->LoadLayers( wxT( "layersdefault.cvg" ) );
    //a2dCanvasGlobals->GetHabitat()->LoadLayers( _T("layersdefault2.cvg") );

    a2dGlobals->GetFontPathList().Add( wxT( "./config" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./config" ) );
    a2dGlobals->GetIconPathList().Add( wxT( "./config" ) );
    a2dGlobals->GetFontPathList().Add( wxT( "../config" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "../config" ) );
    a2dGlobals->GetIconPathList().Add( wxT( "../config" ) );


#if wxART2D_USE_CVGIO
    a2dIOHandlerCVGIn* cvghin = new a2dIOHandlerCVGIn();
    a2dIOHandlerCVGOut* cvghout = new a2dIOHandlerCVGOut();
#endif //wxART2D_USE_CVGIO

#if wxART2D_USE_SVGIO
    a2dIOHandlerSVGIn* svghin = new a2dIOHandlerSVGIn();
    a2dIOHandlerSVGOut* svghout = new a2dIOHandlerSVGOut();
#endif //wxART2D_USE_SVGIO
#if wxART2D_USE_GDSIO
    a2dIOHandlerGDSIn* gdshin = new a2dIOHandlerGDSIn();
    a2dIOHandlerGDSOut* gdshout = new a2dIOHandlerGDSOut();
#endif //wxART2D_USE_GDSIO

    m_connector = new wxDrawingConnector( CLASSINFO( a2dEditorFrame ) );
    m_connector->SetInitialPosition( wxPoint( 50, 40 ) );
    m_connector->SetInitialSize( wxSize( 200, 300 ) );

    a2dDocumentTemplate* doctemplatenew;

    doctemplatenew = new a2dDocumentTemplateAuto( _T( "All Files" ) , _T( "*.*" ) , _T( "" ) , _T( "" ) , _T( "a2dCanvasDocument" ) ,
            CLASSINFO( a2dCanvasDocument ), m_connector, a2dTemplateFlag::DEFAULT );
    docManager->AssociateDocTemplate( doctemplatenew );

#if wxART2D_USE_CVGIO
    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate( _T( "CVG Drawing" ) , _T( "*.cvg" ) , _T( "" ) , _T( "cvg" ) , _T( "a2dCanvasDocument" ) ,
            CLASSINFO( a2dCanvasDocument ), m_connector, a2dTemplateFlag::DEFAULT, cvghin, cvghout );
    docManager->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_CVGIO

#if wxART2D_USE_GDSIO
    doctemplatenew = new a2dDocumentTemplate( _T( "GDS-II Drawing" ) , _T( "*.cal" ) , _T( "" ) , _T( "cal" ) , _T( "a2dCanvasDocument" ) ,
            CLASSINFO( a2dCanvasDocument ), m_connector, a2dTemplateFlag::DEFAULT, gdshin, gdshout );
    docManager->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_GDSIO

#if wxART2D_USE_SVGIO
    doctemplatenew = new a2dDocumentTemplate( _T( "SVG Drawing" ) , _T( "*.svg" ) , _T( "" ) , _T( "svg" ) , _T( "a2dCanvasDocument" ) ,
            CLASSINFO( a2dCanvasDocument ), m_connector, a2dTemplateFlag::DEFAULT, svghin, svghout );
    docManager->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_SVGIO

    a2dCanvasDocumentIOHandlerImageIn* imagehin = new a2dCanvasDocumentIOHandlerImageIn();
    a2dCanvasDocumentIOHandlerImageOut* imagehout = new a2dCanvasDocumentIOHandlerImageOut();
    doctemplatenew = new a2dDocumentTemplate( _T( "BMP Files" ), _T( "*.bmp" ), _T( "" ) , _T( "bmp" ) , _T( "a2dCanvasDocument" ) ,
            CLASSINFO( a2dCanvasDocument ), m_connector, a2dTemplateFlag::DEFAULT, imagehin, imagehout );
    docManager->AssociateDocTemplate( doctemplatenew );

    a2dCanvasDocumentIOHandlerImageIn* imagehinpng = new a2dCanvasDocumentIOHandlerImageIn( wxBITMAP_TYPE_PNG );
    a2dCanvasDocumentIOHandlerImageOut* imagehoutpng = new a2dCanvasDocumentIOHandlerImageOut( wxBITMAP_TYPE_PNG );
    imagehinpng->SetScale( -1 );
    imagehinpng->SetImageSize( 1000, 1000 ); 
    imagehinpng->SetImagePosition( 100, 200 );
    doctemplatenew = new a2dDocumentTemplate( _( "PNG" ), _T( "*.png" ), _T( "" ) , _T( "png" ) , "a2dCanvasDocument",
            CLASSINFO( a2dCanvasDocument ), m_connector, a2dTemplateFlag::VISIBLE | a2dTemplateFlag::IMPORTING | a2dTemplateFlag::EXPORTING, imagehinpng, imagehoutpng );
    docManager->AssociateDocTemplate( doctemplatenew );


    a2dViewTemplate* viewtemplatenew;
    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate( _T( "CVG Drawing" ) , _T( "a2dCanvasDocument" ) , _T( "Drawing View Frame" ) ,
                                           CLASSINFO( a2dCanvasView ), m_connector, a2dTemplateFlag::DEFAULT, wxSize( 100, 400 )  );
    m_connector->AssociateViewTemplate( viewtemplatenew );
    docManager->AssociateViewTemplate( viewtemplatenew );

#if wxART2D_USE_AGGDRAWER
    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate( _T( "CVG Drawing" ) , _T( "a2dCanvasDocument" ) , _T( "Drawing Anti Aliased View Frame" ) ,
                                           CLASSINFO( a2dCanvasView ), m_connector, a2dTemplateFlag::DEFAULT, wxSize( 100, 400 )  );
    m_connector->AssociateViewTemplate( viewtemplatenew );
    docManager->AssociateViewTemplate( viewtemplatenew );
#endif //wxART2D_USE_AGGDRAWER

    m_connector2 = new wxTextConnector();
    m_connector2->SetInitialPosition( wxPoint( 20, 210 ) );
    m_connector2->SetInitialSize( wxSize( 300, 200 ) );

    // Create a template relating drawing documents to text views
    viewtemplatenew =  new a2dViewTemplate( _T( "Text" ) , _T( "a2dCanvasDocument" ) , _T( "Text View Frame" ) ,
                                            CLASSINFO( TextView ), m_connector2, a2dTemplateFlag::DEFAULT, wxSize( 200, 300 ) );

    m_connector->AssociateViewTemplate( viewtemplatenew );
    docManager->AssociateViewTemplate( viewtemplatenew );

    //Create the main frame window
    //the parent frame will get event from the child frames.
    VdrawFrame* frame = new VdrawFrame( docManager, ( wxFrame* ) NULL, -1, _T( "DocView Demo" ) , wxPoint( 0, 0 ), wxSize( 500, 400 ), wxDEFAULT_FRAME_STYLE );
    SetTopWindow( frame );
    m_connector->Init( frame, true );

    frame->Show( true );
    return true;
}

int MyApp::OnExit( void )
{

    m_connector->DisassociateViewTemplates();
    m_connector2->DisassociateViewTemplates();


#if wxUSE_DEBUG_CONTEXT
//    wxDebugContext::Dump();
//    wxDebugContext::PrintStatistics();
#endif

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

OverviewCanvas::~OverviewCanvas()
{
    //a2dDocviewGlobals->GetDocviewCommandProcessor()->Unregister( this );
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

//------------------------------------------------------------------------
// This is the top-level parent frame of the application.
//------------------------------------------------------------------------

IMPLEMENT_CLASS( VdrawFrame, a2dDocumentFrame )

BEGIN_EVENT_TABLE( VdrawFrame, a2dDocumentFrame )
    EVT_MENU_RANGE( MSCREEN_FIRST, MSCREEN_LAST, VdrawFrame::FillData )
    EVT_MENU( CANVAS_ABOUT, VdrawFrame::OnAbout )
    EVT_MENU( CANVAS_TOOLSDLG, VdrawFrame::OnShowToolDlg  )
    EVT_MENU( CANVAS_STYLEDLG, VdrawFrame::OnShowStyleDlg  )
    EVT_MOUSE_EVENTS( VdrawFrame::OnMouseEvent )
    EVT_CLOSE( VdrawFrame::OnCloseWindow )
END_EVENT_TABLE()

VdrawFrame::VdrawFrame( a2dDocumentCommandProcessor* manager, wxFrame* frame, wxWindowID id, const wxString& title,
                        const wxPoint& pos, const wxSize& size, const long type ):
    a2dDocumentFrame( true, frame, NULL, id, title, pos, size, type )
{
    m_tooldlg = NULL;

    m_styledlg = NULL;

    //// Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
    SetIcon( wxIcon( _T( "doc_icn" ) ) );
#endif

    //// Make a menubar
    wxMenu* file_menu = new wxMenu;

    AddCmdMenu( file_menu, CmdMenu_FileNew() );
    AddCmdMenu( file_menu, CmdMenu_FileOpen() );
    AddCmdMenu( file_menu, CmdMenu_FileClose() );
    AddCmdMenu( file_menu, CmdMenu_FileSave() );
    AddCmdMenu( file_menu, CmdMenu_FileSaveAs() );

    file_menu->AppendSeparator();
    AddCmdMenu( file_menu, CmdMenu_PrintView() );
    AddCmdMenu( file_menu, CmdMenu_PreviewView() );
    AddCmdMenu( file_menu, CmdMenu_PrintDocument() );
    AddCmdMenu( file_menu, CmdMenu_PreviewDocument() );
    AddCmdMenu( file_menu, CmdMenu_PrintSetup() );

    file_menu->AppendSeparator();
    file_menu->Append( wxID_EXIT, _T( "E&xit" ) );

    // A nice touch: a history of files visited. Use this menu.
    manager->FileHistoryUseMenu( file_menu );

    wxMenu* dlg_menu = new wxMenu;
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgStyle() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgTools() );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( CANVAS_ABOUT, _T( "&About" ) );

    wxMenuBar* menu_bar = new wxMenuBar;

    menu_bar->Append( file_menu, _T( "&File" ) );

    wxMenu* drawMenu = new wxMenu;
    drawMenu->Append( MSCREEN_0, _T( "screen0" ) , _T( "image + ref rotated image+ vector objects on layers" ) );
    drawMenu->Append( MSCREEN_1, _T( "screen1" ) , _T( "all/most objects and a group with refs to it" ) );
    drawMenu->Append( MSCREEN_2, _T( "screen2" ) , _T( "speed test 10000+ rectangles" ) );
    drawMenu->Append( MSCREEN_3, _T( "screen3" ) , _T( "random 10000+ rectangles with brushes" ) );
    drawMenu->Append( MSCREEN_4, _T( "screen4" ) , _T( "rotated text" ) );
    drawMenu->Append( MSCREEN_5, _T( "screen5" ) , _T( "object with associated properties and other objects" ) );
    drawMenu->Append( MSCREEN_6, _T( "screen6" ) , _T( "endobject + other" ) );
    drawMenu->Append( MSCREEN_7, _T( "screen7" ) , _T( "connected graphs" ) );
    drawMenu->Append( MSCREEN_8, _T( "screen8" ) , _T( "objects on layers" ) );
    drawMenu->Append( MSCREEN_9, _T( "screen9" ) , _T( "selected and not filled test" ) );
    drawMenu->Append( MSCREEN_10, _T( "screen10" ) , _T( "whatever" ) );
    drawMenu->Append( MSCREEN_11, _T( "screen11" ) , _T( "wxRootCanvas as bitmap/image" ) );
    drawMenu->Append( MSCREEN_12, _T( "screen12" ) , _T( "3d things and contours and curves" ) );
    drawMenu->Append( MSCREEN_13, _T( "screen13" ) , _T( "Vector paths" ) );
    drawMenu->Append( MSCREEN_14, _T( "screen14" ) , _T( "Drawing threshold" ) );
    drawMenu->Append( MSCREEN_15, _T( "screen15" ) , _T( "Multi reference" ) );
    drawMenu->Append( MSCREEN_ALL, _T( "overview" ) , _T( "all screens" ) );

    menu_bar->Append( drawMenu, _T( "&Screens" ) );

    menu_bar->Append( dlg_menu, _T( "&Dialogs" ) );

    menu_bar->Append( help_menu, _T( "&Help" ) );

    //// Associate the menu bar with the frame
    SetMenuBar( menu_bar );

    Centre( wxBOTH );
    Show( true );

    CreateStatusBar( 2 );

    int width, height;
    GetClientSize( &width, &height );

    m_topcanvas = new OverviewCanvas( this, wxPoint( 0, 0 ), size, 0 );

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
    m_topcanvas->GetDrawing()->CreateCommandProcessor();
	m_topcanvas->GetDrawingPart()->Update();

    editMenu = ( wxMenu* ) NULL;
}

VdrawFrame::~VdrawFrame()
{
    //undo stack can hold Owners to Documents etc.
    m_topcanvas->GetDrawing()->GetCommandProcessor()->ClearCommands();
}

void VdrawFrame::OnCloseWindow( wxCloseEvent& event )
{
    if ( event.CanVeto() )
    {
        if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->Exit( !event.CanVeto() ) )
        {
            Destroy();
        }
        else
            event.Veto( true );
    }
    else
    {
        Destroy();
    }
}

bool VdrawFrame::Destroy()
{
    if ( m_tooldlg )
    {
        m_tooldlg->Destroy();
        m_tooldlg = 0;
    }
    if ( m_styledlg )
    {
        m_styledlg->Destroy();
        m_styledlg = 0;
    }
    return a2dDocumentFrame::Destroy();
}



void VdrawFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( _T( " Vdraw \nAuthor: Klaas Holwerda" ) , _T( "About Vdraw" ) );
}

void VdrawFrame::FillData( wxCommandEvent& event )
{
    a2dSmrtPtr<a2dCanvasDocument> doc = new a2dCanvasDocument();
    doc->SetDocumentTypeName( _T( "a2dCanvasDocument" ) );

    a2dCanvasObject* tmp = m_topgroup;

    switch ( event.GetId() )
    {
        case MSCREEN_0:
        {
            FillData0( doc, &tmp );
            doc->SetTitle( _T( "screen 0" ) );
            break;
        }
        case MSCREEN_1:
        {
            FillData1( doc, &tmp );
            doc->SetTitle( _T( "screen 1" ) );
            break;
        }
        case MSCREEN_2:
        {
            FillData2( doc, &tmp );
            doc->SetTitle( _T( "screen 2" ) );
            break;
        }
        case MSCREEN_3:
        {
            FillData3( doc, &tmp );
            doc->SetTitle( _T( "screen 3" ) );
            break;
        }
        case MSCREEN_4:
        {
            FillData4( doc, &tmp );
            doc->SetTitle( _T( "screen 4" ) );
            break;
        }
        case MSCREEN_5:
        {
            FillData5( doc, &tmp );
            doc->SetTitle( _T( "screen 5" ) );
            break;
        }
        case MSCREEN_6:
        {
            FillData6( doc, &tmp );
            doc->SetTitle( _T( "screen 6" ) );
            break;
        }
        case MSCREEN_7:
        {
            FillData7( doc, &tmp );
            doc->SetTitle( _T( "screen 7" ) );
            break;
        }
        case MSCREEN_8:
        {
            FillData8( doc, &tmp );
            doc->SetTitle( _T( "screen 8" ) );
            break;
        }
        case MSCREEN_9:
        {
            FillData9( doc, &tmp );
            doc->SetTitle( _T( "screen 9" ) );
            break;
        }
        case MSCREEN_10:
        {
            FillData10( doc, &tmp );
            doc->SetTitle( _T( "screen 10" ) );
            break;
        }
        case MSCREEN_11:
        {
            FillData11( doc, &tmp );
            doc->SetTitle( _T( "screen 11" ) );
            break;
        }
        case MSCREEN_12:
        {
            FillData12( doc, &tmp );
            doc->SetTitle( _T( "screen 12" ) );
            break;
        }
        case MSCREEN_13:
        {
            FillData13( doc, &tmp );
            doc->SetTitle( _T( "screen 13" ) );
            break;
        }
        case MSCREEN_14:
        {
            FillData14( doc, &tmp );
            doc->SetTitle( _T( "screen 14" ) );
            break;
        }
        case MSCREEN_15:
        {
            FillData15( doc, &tmp );
            doc->SetTitle( _T( "screen 15" ) );
            break;
        }
        case MSCREEN_ALL:
        {
            FillDataAll( doc, &tmp );
            doc->SetTitle( _T( "screen all" ) );
            break;
        }
        default:
            return;
    }
    m_topgroup = tmp;


    doc->GetDrawing()->SetRootRecursive();

    //a2dDocviewGlobals->GetDocviewCommandProcessor()->AddCreatedDocument( doc, true, true );

    doc->Update( a2dCanvasObject::updatemask_force );
    a2dView* createdview = a2dDocviewGlobals->GetDocviewCommandProcessor()->AddDocumentCreateView( doc );//, _T("Drawing View") );

    if ( 0 != wxDynamicCast( createdview, a2dCanvasView ) )
    {
        a2dCanvasView* drawerview = wxStaticCast( createdview, a2dCanvasView );
#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
        a2dOglCanvas* canvas = wxStaticCast( drawerview->GetDisplayWindow(), a2dOglCanvas );
#else
        a2dCanvas* canvas = wxStaticCast( drawerview->GetDisplayWindow(), a2dCanvas );
#endif //wxUSE_GLCANVAS 
        canvas->GetDrawingPart()->SetShowObject( m_topgroup );
    }
    else if ( 0 != wxDynamicCast( createdview, TextView ) )
    {
        TextView* view = ( TextView* ) createdview;
        view->SetShowObject( m_topgroup );
    }

    createdview->Update();
    createdview->Activate( true );
}

void VdrawFrame::OnMouseEvent( wxMouseEvent& event )
{
}

void VdrawFrame::OnShowStyleDlg( wxCommandEvent& event )
{
    if ( !m_styledlg )
    {
        m_styledlg  = new a2dStyleDialog( a2dCanvasGlobals->GetHabitat(), NULL, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxDIALOG_NO_PARENT | wxMINIMIZE_BOX | wxMAXIMIZE_BOX );
        wxRect r = GetRect();
        m_styledlg->Move( r.x + r.width, r.y );
    }

    if ( m_styledlg->IsShown() )
    {
        GetMenuBar()->Check( CANVAS_STYLEDLG, false );
        m_styledlg->Show( false );
    }
    else
    {
        GetMenuBar()->Check( CANVAS_STYLEDLG, true );
        m_styledlg->Show( true );
    }
}

void VdrawFrame::OnShowToolDlg( wxCommandEvent& event )
{
    if ( !m_tooldlg )
    {
        m_tooldlg  = new ToolDlg( NULL );
        wxSize sz = m_tooldlg->GetSize();
        wxPoint pt = GetPosition();
        m_tooldlg->Move( pt.x - sz.x, pt.y );
    }

    if ( m_tooldlg->IsShown() )
    {
        GetMenuBar()->Check( CANVAS_TOOLSDLG, false );
        m_tooldlg->Show( false );
    }
    else
    {
        GetMenuBar()->Check( CANVAS_TOOLSDLG, true );
        m_tooldlg->Show( true );
    }
}



