/*! \file editor/samples/vdraws/vdraw.cpp
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: vdraws.cpp,v 1.42 2009/09/26 19:01:19 titato Exp $
*/ 

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/colordlg.h"
#include <wx/wfstream.h>

#include "wx/canvas/canmod.h"
#include "wx/editor/editmod.h"
#include "wx/editor/xmlparsdoc.h"

#if wxART2D_USE_GDSIO
#include "wx/gdsio/gdsio.h"
#endif //wxART2D_USE_GDSIO

#if wxART2D_USE_SVGIO
#include "wx/svgio/parssvg.h"
#endif //wxART2D_USE_SVGIO
#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#include "wx/aggdrawer/graphicagg.h"
#endif //wxART2D_USE_AGGDRAWER
#if wxART2D_USE_GDIPLUSDRAWER
#include "wx/gdiplusdrawer/gdiplusdrawer.h"
#endif //wxART2D_USE_GDIPLUSDRAWER

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#include "screen.h"
#include "vdraws.h"

IMPLEMENT_APP( MyApp )

BEGIN_EVENT_TABLE( a2dConnector, a2dViewConnector )
    EVT_POST_CREATE_VIEW( a2dConnector::OnPostCreateView )
    EVT_PRE_ADD_DOCUMENT( a2dConnector::OnPreAddCreatedDocument )
    EVT_POST_CREATE_DOCUMENT( a2dConnector::OnPostCreateDocument )
    EVT_DISCONNECT_VIEW( a2dConnector::OnDisConnectView )
END_EVENT_TABLE()

a2dConnector::a2dConnector()
    : a2dViewConnector()
{
	m_frame = NULL;
}

a2dConnector::~a2dConnector()
{
}

void a2dConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    a2dView* view = event.GetView();

    m_view = ( a2dCanvasView* ) view;

    if ( m_frame )
    {
        a2dDrawer2D* drawer2d = new a2dMemDcDrawer( a2dCanvasGlobals->m_initialDocChildFrameSize );
        //OR
        //a2dDrawer2D* drawer2d = new a2dGDIPlusDrawer( a2dCanvasGlobals->m_initialDocChildFrameSize );
        //a2dDrawer2D* drawer2d = new a2dAggDrawer( a2dCanvasGlobals->m_initialDocChildFrameSize );
        m_frame->GetDrawingPart()->SetDrawer2D( drawer2d );
		m_view->SetDisplayWindow( m_frame->GetDrawingPart()->GetDisplayWindow() );
        m_view->SetPrintTitle( false );
        m_view->SetPrintFilename( false );
        m_view->SetPrintFrame( false );

		if ( m_frame->GetDrawingPart()->GetCanvasToolContr() )
			m_frame->GetDrawingPart()->GetCanvasToolContr()->StopAllTools();
		if ( m_frame->GetDrawingPart()->GetCanvasToolContr() )
			m_frame->GetDrawingPart()->GetCanvasToolContr()->ReStart();

		m_frame->GetDrawingPart()->SetMappingShowAll();

	}

}

void a2dConnector::OnPreAddCreatedDocument( a2dTemplateEvent& event )
{
    m_newdoc = event.GetDocument();

    if ( !( event.GetFlags() & a2dREFDOC_SILENT ) &&
            ( event.GetFlags() & a2dREFDOC_INIT ) &&
            ( event.GetFlags() & a2dREFDOC_NEW ) )
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
    if ( !m_frame )
    {

        a2dViewTemplate* viewtempl;
        viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates().front();

        m_view = ( a2dCanvasView* ) viewtempl->CreateView( event.GetDocument(), event.GetFlags() );
        m_view->SetPrintTitle( false );
        m_view->SetPrintFilename( false );
        m_view->SetPrintFrame( false );

        event.SetView( m_view );
        m_view->Activate( true );
    }
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
#if with_GLCANVASdraw
    a2dOglCanvas* canvas = wxStaticCast( theViewToDisconnect->GetDisplayWindow(), a2dOglCanvas );
#else
    a2dCanvas* canvas = wxStaticCast( theViewToDisconnect->GetDisplayWindow(), a2dCanvas );
#endif //wxUSE_GLCANVAS 
    if ( canvas->GetDrawingPart()->GetCanvasToolContr() )
        canvas->GetDrawingPart()->GetCanvasToolContr()->StopAllTools();
	canvas->GetDrawingPart()->SetShowObject( NULL );

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

 		theViewToDisconnect->GetCanvasDocument()->Update( a2dCanvasObject::updatemask_force_and_count );
		canvas->GetDrawingPart()->SetShowObject( theViewToDisconnect->GetCanvasDocument()->GetDrawing()->GetRootObject() );
        if ( canvas->GetDrawingPart()->GetCanvasToolContr() )
            canvas->GetDrawingPart()->GetCanvasToolContr()->ReStart();
        canvas->GetDrawingPart()->SetMappingShowAll();
        theViewToDisconnect->Activate( true );
    }
    else
    {
        m_newdoc = NULL;
        // exiting, do the default, disconnecting the view.
        event.Skip();
    }
}

//-----------------------------------------------------------------------------
// MyApp
//-----------------------------------------------------------------------------

bool MyApp::OnInit()
{
    //_CrtSetBreakAlloc(16428);

    //a2dCanvasGlobals->SetMaxLayer( 100 );


    wxString artroot = a2dGeneralGlobals->GetWxArt2DVar();
    if ( artroot.IsEmpty() )
        return false;

    a2dCanvasGlobals->m_initialDocChildFrameSize = wxSize( 800, 800 );

    wxInitAllImageHandlers();
    a2dMenuIdItem::InitializeBitmaps();

#if wxUSE_LIBPNG
    //wxImage::AddHandler( new wxPNGHandler );
#endif
    //SetUseBestVisual(true);
    a2dGlobals->GetFontPathList().Add( artroot + wxT( "samples/editor/common/fonts" ) );
    a2dGlobals->GetImagePathList().Add( artroot +  wxT( "samples/editor/common/images" ) );
    a2dGlobals->GetFontPathList().Add(  artroot +  wxT( "samples/editor/common/fonts" ) );
    a2dGlobals->GetIconPathList().Add(  artroot +  wxT( "samples/editor/common/fonts" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "art/resources" ) );
    a2dGlobals->GetIconPathList().Add( artroot + wxT( "apps/teto/config" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "./" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "/home/klaas/pixi" ) );
    a2dGlobals->GetImagePathList().Add( wxT( "C:/tmp/pixi/" ) );
    a2dCanvasGlobals->GetLayersPathList().Add( artroot +  wxT( "art/layers" ) );

    a2dCentralCanvasCommandProcessor* docmanager = new a2dCentralCanvasCommandProcessor();
    a2dDocviewGlobals->SetDocviewCommandProcessor( docmanager );
    //a2dCanvasGlobals->SetEditAtEndTools( false );
    a2dCanvasGlobals->SetPopBeforePush( false );
    a2dCanvasGlobals->SetOneShotTools( false );

    m_singleconnector = new a2dConnector();

    a2dIOHandlerDocCVGIn* cvghin = new a2dIOHandlerDocCVGIn();
    a2dIOHandlerDocCVGOut* cvghout = new a2dIOHandlerDocCVGOut();

#if wxART2D_USE_SVGIO
    a2dIOHandlerSVGIn* svghin = new a2dIOHandlerSVGIn();
    a2dIOHandlerSVGOut* svghout = new a2dIOHandlerSVGOut();
#endif //wxART2D_USE_SVGIO
#if wxART2D_USE_GDSIO
    a2dIOHandlerGDSIn* gdshin = new a2dIOHandlerGDSIn();
    a2dIOHandlerGDSOut* gdshout = new a2dIOHandlerGDSOut();
#endif //wxART2D_USE_GDSIO
#if wxART2D_USE_KEYIO
    a2dIOHandlerKeyIn* keyhin = new a2dIOHandlerKeyIn();
    a2dIOHandlerKeyOut* keyhout = new a2dIOHandlerKeyOut();
#endif //wxART2D_USE_KEYIO

    a2dCanvasGlobals->GetHabitat()->LoadLayers( wxT( "layersdefault.cvg" ) );

    a2dDocumentTemplatePtr doctemplatenew;

    doctemplatenew = new a2dDocumentTemplateAuto( _T( "All Files" ) , _T( "*.*" ) , _T( "" ) , _T( "" ) , _T( "a2dCanvasDocument" ) ,
            CLASSINFO( a2dCanvasDocument ), m_singleconnector, a2dTemplateFlag::DEFAULT );
    docmanager->AssociateDocTemplate( doctemplatenew );

    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate( wxT( "CVG Drawing" ), wxT( "*.cvg" ), wxT( "" ), wxT( "cvg" ), wxT( "a2dCanvasDocument" ),
            CLASSINFO( a2dCanvasDocument ), m_singleconnector, a2dTemplateFlag::DEFAULT, cvghin, cvghout );
    docmanager->AssociateDocTemplate( doctemplatenew );

#if wxART2D_USE_GDSIO
    a2dDocumentTemplate* gdsii_doctemplate = new a2dDocumentTemplate( wxT( "GDS-II Drawing" ), wxT( "*.cal" ), wxT( "" ), wxT( "cal" ), wxT( "a2dCanvasDocument" ),
            CLASSINFO( a2dCanvasDocument ), m_singleconnector, a2dTemplateFlag::DEFAULT, gdshin, gdshout );
    docmanager->AssociateDocTemplate( gdsii_doctemplate );
#endif //wxART2D_USE_GDSIO

#if wxART2D_USE_SVGIO
    doctemplatenew = new a2dDocumentTemplate( wxT( "SVG Drawing" ), wxT( "*.svg" ), wxT( "" ), wxT( "svg" ), wxT( "a2dCanvasDocument" ),
            CLASSINFO( a2dCanvasDocument ), m_singleconnector, a2dTemplateFlag::DEFAULT, svghin, svghout );
    docmanager->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_SVGIO

#if wxART2D_USE_KEYIO
    doctemplatenew = new a2dDocumentTemplate( wxT( "Key Drawing" ), wxT( "*.key" ), wxT( "" ), wxT( "key" ), wxT( "a2dCanvasDocument" ),
            CLASSINFO( a2dCanvasDocument ), m_singleconnector, a2dTemplateFlag::DEFAULT, keyhin, keyhout );
    docmanager->AssociateDocTemplate( doctemplatenew );
#endif //wxART2D_USE_KEYIO

    a2dViewTemplate* viewtemplatenew;
    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate( wxT( "CVG Drawing" ), wxT( "a2dCanvasDocument" ), wxT( "Drawing View" ),
                                           CLASSINFO( a2dCanvasView ), m_singleconnector, a2dTemplateFlag::DEFAULT, a2dCanvasGlobals->m_initialDocChildFrameSize );
    docmanager->AssociateViewTemplate( viewtemplatenew );


	// create the list of fonts (stored in a2dTextChanges) and filter them ------------------------
	const int TNumFonts = 16;
	const wxString TFonts[TNumFonts] = {  wxT("Liberation Sans"), wxT("Liberation Serif"), wxT("Liberation Mono"), wxT("DejaVu Sans"), wxT("DejaVu Sans Mono"), wxT("DejaVu Serif") };
	wxArrayString filterlist(TNumFonts, TFonts);
	
	//a2dTextChanges::InitialiseFontList(filterlist, a2dDEFAULT_FONTTYPE);		// create the list of available fonts
	a2dTextChanges::InitialiseFontList();		// create the list of available fonts

    MyFrame* myFrame = NULL;
    a2dCanvasDocument* document = NULL;

//choose one of these, 4 is most elegant
    int method = 4;
    if ( method == 1 )
    {
        // 1. document manager creates a new document ( uses a document template )
        // 2. The new created document is connected via connector m_singleconnector
        // 3. get the last created document
        docmanager->CreateDocuments( wxT( "dummy" ), a2dREFDOC_NEW );
        document = ( a2dCanvasDocument* ) docmanager->GetDocuments().back().Get();
        myFrame = new MyFrame( ( wxFrame* ) NULL,
                               a2dCanvasGlobals->m_initialDocChildFramePos,
                               a2dCanvasGlobals->m_initialDocChildFrameSize,
                               a2dCanvasGlobals->m_initialDocChildFrameStyle
                             );
    }
    else if ( method == 2 )
    {
        // 1. make a document
        // 2. this did already set its type for the document template to be found
        // 3. set a document template to it
        // 4. and let the m_singleconnector create a view
        // 5. add the document to the document manager its document list.
        document = new a2dCanvasDocument();
        document->SetDocumentTemplate( doctemplatenew );
        a2dCanvasView* view = ( a2dCanvasView* ) m_singleconnector->CreateView( document );
        docmanager->AddDocument( document );
        myFrame = new MyFrame( ( wxFrame* ) NULL,
                               a2dCanvasGlobals->m_initialDocChildFramePos,
                               a2dCanvasGlobals->m_initialDocChildFrameSize,
                               a2dCanvasGlobals->m_initialDocChildFrameStyle
                             );
    }
    else if ( method == 3 )
    {
        // 1. make a document
        // 2. this did already set its type for the document template to be found
        // 3. add the created document to the document manager
        // 4. and let the document manager create a view based on a selected/given template
        document = new a2dCanvasDocument();
        a2dCanvasView* view = ( a2dCanvasView* ) docmanager->AddDocumentCreateView( document );
        myFrame = new MyFrame( ( wxFrame* ) NULL,
                               a2dCanvasGlobals->m_initialDocChildFramePos,
                               a2dCanvasGlobals->m_initialDocChildFrameSize,
                               a2dCanvasGlobals->m_initialDocChildFrameStyle
                             );
    }
    else if ( method == 4 )
    {
        // 1. make a document
        // 2. this did already set its type for the document template to be found
        // 3. add the created document to the document manager, and send events
        // 4. the m_singleconnector will create the first view.
        document = new a2dCanvasDocument();
        //document->CreateCommandProcessor();
        a2dDocviewGlobals->GetDocviewCommandProcessor()->AddCreatedDocument( document, true, true, a2dREFDOC_NEW );
        myFrame = new MyFrame( ( wxFrame* ) NULL,
                               a2dCanvasGlobals->m_initialDocChildFramePos,
                               a2dCanvasGlobals->m_initialDocChildFrameSize,
                               a2dCanvasGlobals->m_initialDocChildFrameStyle
                             );
    }

	m_singleconnector->m_frame = myFrame;

    a2dCanvasGlobals->GetHabitat()->SetReverseOrder( true );

#if with_GLCANVASdraw
    a2dOglCanvas* canvas = wxStaticCast( myFrame->GetDrawingPart()->GetDisplayWindow(), a2dOglCanvas );
    canvas->SetDrawing( document->GetDrawing() );
    canvas->Show( true );

#else
    a2dCanvas* canvas = wxStaticCast( myFrame->GetDrawingPart()->GetDisplayWindow(), a2dCanvas );
    canvas->SetDrawing( document->GetDrawing() );
#endif //wxUSE_GLCANVAS 
 	m_singleconnector->m_view->SetDisplayWindow( canvas );

	myFrame->SetDestroyOnCloseView( false );
    myFrame->SetTitle( wxT( "vdraws" ) ); //a2dArt2D_VERSION );
    SetTopWindow( myFrame );

    canvas->GetDrawingPart()->SetZoomOutBorder( 30 );

    a2dCanvasObject* tmp;
    //fill document with data
    FillData16( document, &tmp );
    myFrame->m_topobject = tmp;

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    canvas->SetYaxis( true );
    canvas->SetMappingUpp( -600, -600, 4, 4 );
   
    //defines the whole area in world coordinates that can be seen by scrolling
    canvas->SetScrollMaximum( -800, -800, 1600, 1600 );
    //defines the number of world units scrolled when line up or down events in X
    canvas->SetScrollStepX( 200 );
    //defines the number of world units scrolled when line up or down events in Y
    canvas->SetScrollStepY( 100 );

	canvas->SetShowObject( myFrame->m_topobject );
	
    myFrame->Show( true );
    canvas->Show( true );

//#define OVERV
#ifdef OVERV

    m_over = new wxFrame( ( wxFrame* ) NULL, -1, wxT( "over" ),
                          wxPoint( 0, 0 ),
                          wxSize( 100, 100 ),
                          a2dCanvasGlobals->m_initialDocChildFrameStyle
                        );
    a2dZoomedView* overv = new a2dZoomedView( m_over, -1, wxDefaultPosition, wxDefaultSize, 0, m_singleconnector->m_view );
    overv->SetBackgroundColour( *wxWHITE );


    m_over->Show( true );
#endif

    return true;
}


int MyApp::OnExit()
{
    m_singleconnector = NULL;
    return 0;
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

#define CANVAS_ABOUT 1



BEGIN_EVENT_TABLE( MyFrame, a2dEditorFrame )
    EVT_MENU_RANGE( MDSCREEN_FIRST, MDSCREEN_LAST, MyFrame::FillData )
    EVT_MENU_RANGE( wxID_FILE1, wxID_FILE9, MyFrame::OnMRUFile )
    EVT_MENU_RANGE( DrawerOption_First, DrawerOption_Last, MyFrame::OnDrawer )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_THEME_EVENT( MyFrame::OnTheme )
    EVT_INIT_EVENT( MyFrame::OnInit )
    EVT_CLOSE_VIEW( MyFrame::OnCloseView )
END_EVENT_TABLE()


MyFrame::MyFrame( wxFrame* parent, const wxPoint& pos, const wxSize& size, long style )

    : a2dEditorFrame( true, parent, pos, size, style )
{

    //a2dCanvas* canvas = new a2dCanvas( this, -1, pos,  size, a2dCanvasGlobals->m_initialWorldCanvasStyle );

#if with_GLCANVASdraw 
    #if wxCHECK_VERSION(3,1,0)
        //wxGLAttributes dispAttrs;
        //dispAttrs.PlatformDefaults().MinRGBA(8, 8, 8, 8).DoubleBuffer().Depth(32).EndList();
        //dispAttrs.Defaults().EndList();
        //dispAttrs.SetNeedsARB(true);



    wxGLAttributes vAttrs;
    // Defaults should be accepted
    vAttrs.PlatformDefaults().Defaults().EndList();
    bool accepted = wxGLCanvas::IsDisplaySupported(vAttrs) ;

    if ( accepted )
    {
#if wxUSE_LOGWINDOW
        //wxLogMessage("The display supports required visual attributes.");
#endif // wxUSE_LOGWINDOW
    }
    else
    {
#if wxUSE_LOGWINDOW
        wxLogMessage("First try with OpenGL default visual attributes failed.");
#endif // wxUSE_LOGWINDOW
        // Try again without sample buffers
        vAttrs.Reset();
        vAttrs.PlatformDefaults().RGBA().DoubleBuffer().Depth(16).EndList();
        accepted = wxGLCanvas::IsDisplaySupported(vAttrs) ;

        if ( !accepted )
        {
            wxMessageBox("Visual attributes for OpenGL are not accepted.\nThe app will exit now.",
                         "Error with OpenGL", wxOK | wxICON_ERROR);
        }
        else
        {
#if wxUSE_LOGWINDOW
            wxLogMessage("Second try with other visual attributes worked.");
#endif // wxUSE_LOGWINDOW
        }
    }


        a2dOglCanvas* canvas = new a2dOglCanvas( this, vAttrs, -1, pos,  size, a2dCanvasGlobals->m_initialWorldCanvasStyle );
    #else
        int attribList[2];
        attribList[1] = WX_GL_DOUBLEBUFFER;
        a2dOglCanvas* canvas = new a2dOglCanvas( this, NULL, -1, pos,  size, a2dCanvasGlobals->m_initialWorldCanvasStyle );
    #endif
#else
    a2dCanvas* canvas = new a2dCanvas( this, -1, pos,  size, a2dCanvasGlobals->m_initialWorldCanvasStyle );
#endif

    canvas->SetVirtualSize( size );

    m_drawingPart = canvas->GetDrawingPart();

    /* or like this for more flexibility
        : a2dEditorFrame()
    {
        Create( true, true, drawer, parent, pos, size, style );
    */
    Init();
    m_drawingPart->GetDrawer2D()->SetDisplayAberration( 1 );

    SetStatusText( wxT( "Welcome to Single canvas editor!" ) );

    SetIcon( wxICON( mondrian ) );
}


MyFrame::~MyFrame()
{
//#define OVERV
#ifdef OVERV
    wxGetApp().m_over->Destroy();
#endif

}

void MyFrame::OnTheme( a2dEditorFrameEvent& themeEvent )
{
    CreateThemeXRC();
    //Theme();

    wxMenuBar* bar =  GetMenuBar();
	wxMenu* menuObject = new wxMenu;
	bar->Append( menuObject, _("Object") );	

	//------------------
	wxMenu* submenuArrange = new wxMenu;
	AddCmdMenu( submenuArrange, CmdMenu_Selected_Group() );
	AddCmdMenu( submenuArrange, CmdMenu_Selected_UnGroup() );
	submenuArrange->AppendSeparator();
	AddCmdMenu( submenuArrange, CmdMenu_Selected_ToTop() );
	AddCmdMenu( submenuArrange, CmdMenu_Selected_ToBack() );
	//---
	menuObject->AppendSubMenu( submenuArrange, _("Arrange") );	
	//----------------
	wxMenu* submenuAlign = new wxMenu;
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MinX_Dest() );
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MaxX_Dest() );
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MinY_Dest() );
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MaxY_Dest() );
	submenuAlign->AppendSeparator();
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MidX_Dest() );
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_MidY_Dest() );
	submenuAlign->AppendSeparator();
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_DistributeVert() );
	AddCmdMenu( submenuAlign, CmdMenu_Selected_Align_DistributeHorz() );
	//---
	menuObject->AppendSubMenu( submenuAlign, _("Align") );
	menuObject->AppendSeparator();
	AddCmdMenu( menuObject, CmdMenu_Selected_Align_MirrorVertBbox() );
	AddCmdMenu( menuObject, CmdMenu_Selected_Align_MirrorHorzBbox() );
	AddCmdMenu( menuObject, CmdMenu_Selected_RotateObject90Left() );
	AddCmdMenu( menuObject, CmdMenu_Selected_RotateObject90Right() );

    menuObject->AppendSeparator();
	AddCmdMenu( menuObject, CmdMenu_Selected_SetExtStyle() );
	AddCmdMenu( menuObject, CmdMenu_Selected_SetTextChanges() );

    wxMenu* drawMenu = new wxMenu;
    drawMenu->Append( MDSCREEN_0, wxT( "many images" ), _( "image + ref rotated image+ vector objects on layers" ) );
    drawMenu->Append( MDSCREEN_1, wxT( "multi refs" ), _( "all/most objects and a group with refs to it" ) );
    drawMenu->Append( MDSCREEN_2, wxT( "layered objects" ), _( "layers in hierarchy" ) );
    drawMenu->Append( MDSCREEN_3, wxT( "many rectangles" ), _( "many rectangles" ) );
    drawMenu->Append( MDSCREEN_4, wxT( "whatever" ), _( "layers in hierarchy" ) );
    drawMenu->Append( MDSCREEN_5, wxT( "top struct translated" ), _( "top struct translated tests" ) );
    drawMenu->Append( MDSCREEN_6, wxT( "enterleave" ), _( "enter leave events" ) );
    drawMenu->Append( MDSCREEN_10, wxT( "properties" ), _( "properties" ) );
    drawMenu->Append( MDSCREEN_16, wxT( "slider" ), _( "slider" ) );
    drawMenu->Append( MDSCREEN_17, wxT( "layered objects" ), _( "layers in hierarchy" ) );
    drawMenu->Append( MDSCREEN_18, wxT( "many images" ), _( "image + ref rotated image+ vector objects on layers" ) );
    drawMenu->Append( MDSCREEN_19, wxT( "multi refs" ), _( "all/most objects and a group with refs to it" ) );
    drawMenu->Append( MDSCREEN_20, wxT( "Window and widgets" ), _( "windows and widgets" ) );
    drawMenu->Append( MDSCREEN_21, wxT( "Window multi buttons" ), _( "screen 21" ) );
    drawMenu->Append( MDSCREEN_22, wxT( "SubEdit" ), _( "screen 22" ) );
    drawMenu->Append( MDSCREEN_23, wxT( "hit nested" ), _( "screen 23" ) );
    drawMenu->Append( MDSCREEN_24, wxT( "Transparent mask" ), _( "screen 24" ) );

    wxMenu* drawEng = new wxMenu;
    drawEng->Append( Switch_Drawer_aggRgba, _T( "Switch Drawer &Agg AntiAlias Rgba" ) );
    drawEng->Append( Switch_Drawer_agg, _T( "Switch Drawer &Agg AntiAlias" ) );
    drawEng->Append( Switch_Drawer_agggc, _T( "Switch Drawer Graphics &Context &Agg Gc" ) );

    drawEng->Append( Switch_Drawer_dc, _T( "Switch Drawer &wxDC" ) );
    drawEng->Append( Switch_Drawer_gdiplus, _T( "Switch Drawer &GdiPlus" ) );
    drawEng->Append( Switch_Drawer_gdigc, _T( "Switch Drawer Graphics &Context GDI" ) );
    drawEng->Append( Switch_Drawer_dcgc, _T( "Switch Drawer Graphics Context &wxDC" ) );

    wxMenu* menumasterTool = new wxMenu;
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_SelectFirst() );
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_ZoomFirst() );
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_TagGroups() );
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_TagGroups_WireMode() );
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_TagGroups_DlgMode() );
    AddCmdMenu( menumasterTool, CmdMenu_SetMasterTool_TagGroups_WireMode_DlgMode() );

    bar->Append( drawMenu, _( "&DemoScreens" ) );
    bar->Append( drawEng, _T( "&Drawer Type" ) );
    bar->Append( menumasterTool, _T( "&Choose MasterTool" ) );

    wxMenu* fileMenu = m_menuBar->GetMenu( m_menuBar->FindMenu( wxT( "File" ) ) );
    //AddCmdMenu( file_menu, CmdMenu_FileNew() );
    //fileMenu->Insert( 0, wxID_NEW, _("&New..."), _("Create new file") );
    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( fileMenu );
    m_contr->GetFirstTool()->SetShowAnotation( false );

#ifdef with_GLCANVASdraw
    m_contr->SetOpacityFactorEditcopy( 125 );
    m_contr->SetUseOpaqueEditcopy( true );
#endif

    m_contr->SetZoomFirst( false );

}

void MyFrame::OnInit( a2dEditorFrameEvent& initEvent )
{
    if ( m_drawingPart && m_drawingPart->GetDrawing() )
    {
        m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_MENUSTRINGS, this );
        m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_DO, this );
        m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_UNDO, this );
        m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_REDO, this );
    }

    initEvent.Skip();
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

void MyFrame::OnCloseView( a2dCloseViewEvent& event )
{
    event.Skip();
}

void MyFrame::OnMRUFile( wxCommandEvent& event )
{
    event.Skip();

    /*
        int n = event.GetId() - wxID_FILE1;  // the index in MRU list
        wxString filename(a2dDocviewGlobals->GetDocviewCommandProcessor()->GetHistoryFile(n));
        if ( !filename.IsEmpty() )
        {
            a2dDocument *doc = NULL;

            const_forEachIn( a2dDocumentList, &a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments() )
            {
                doc = *iter;
                if ( doc->GetFilename().GetFullPath() == filename )
                    break;
            }

            if ( doc )
                ConnectDocument( (a2dCanvasDocument*) doc );
        }
    */
}

void MyFrame::FillData( wxCommandEvent& event )
{
    a2dSmrtPtr<a2dCanvasDocument> doc = new a2dCanvasDocument();
    doc->SetDocumentTypeName( wxT( "a2dCanvasDocument" ) );
    a2dCanvasObject* tmp = m_topobject;
    switch ( event.GetId() )
    {
        case MDSCREEN_0:
        {
            FillData0( doc, &tmp  );
            doc->SetTitle( wxT( "screen 0" ) );
            break;
        }
        case MDSCREEN_1:
        {
            FillData1( doc, &tmp  );
            doc->SetTitle( wxT( "screen 1" ) );
            break;
        }
        case MDSCREEN_2:
        {
            FillData2( doc, &tmp  );
            doc->SetTitle( wxT( "screen 2" ) );
            break;
        }
        case MDSCREEN_3:
        {
            FillData3( doc, &tmp );
            doc->SetTitle( wxT( "screen 3" ) );
            break;
        }
        case MDSCREEN_4:
        {
            FillData4( doc, &tmp  );
            doc->SetTitle( wxT( "screen 4" ) );
            break;
        }
        case MDSCREEN_5:
        {
            FillData5( doc, &tmp  );
            doc->SetTitle( wxT( "screen 5" ) );
            break;
        }
        case MDSCREEN_6:
        {
            FillData6( doc, &tmp  );
            doc->SetTitle( wxT( "screen 6" ) );
            break;
        }
        case MDSCREEN_10:
        {
            FillData10( doc, &tmp );
            doc->SetTitle( wxT( "screen 10" ) );
            break;
        }
        case MDSCREEN_16:
        {
            FillData16( doc, &tmp );
            doc->SetTitle( wxT( "screen 16" ) );
            break;
        }
        case MDSCREEN_17:
        {
            FillData17( doc, &tmp );
            doc->SetTitle( wxT( "screen 17" ) );
            break;
        }
        case MDSCREEN_18:
        {
            FillData18( doc, &tmp );
            doc->SetTitle( wxT( "screen 18" ) );
            break;
        }
        case MDSCREEN_19:
        {
            FillData19( doc, &tmp );
            doc->SetTitle( wxT( "screen 19" ) );
            break;
        }
        case MDSCREEN_20:
        {
            FillData20( doc, &tmp );
            doc->SetTitle( wxT( "screen 20" ) );
            break;
        }
        case MDSCREEN_21:
        {
            FillData21( doc, &tmp );
            doc->SetTitle( wxT( "screen 21" ) );
            break;
        }
        case MDSCREEN_22:
        {
            FillData22( doc, &tmp );
            doc->SetTitle( wxT( "screen 22" ) );
            break;
        }
        case MDSCREEN_23:
        {
            FillData23( doc, &tmp );
            doc->SetTitle( wxT( "screen 23" ) );
            break;
        }
        case MDSCREEN_24:
        {
            FillData24( doc, &tmp );
            doc->SetTitle( wxT( "screen 24" ) );
            break;
        }
        default:
            return;
    }
    m_topobject = tmp;
    a2dDocviewGlobals->GetDocviewCommandProcessor()->AddCreatedDocument( doc, true, true );

    m_drawingPart->SetShowObject( m_topobject );
}

void MyFrame::OnDrawer( wxCommandEvent& event )
{
    int width = m_drawingPart->GetDrawer2D()->GetWidth();
    int height = m_drawingPart->GetDrawer2D()->GetHeight();

    switch ( event.GetId() )
    {
#if wxART2D_USE_AGGDRAWER
        case Switch_Drawer_aggRgba:
        {
            m_drawingPart->SetDrawer2D( new a2dAggDrawerRgba( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
        case Switch_Drawer_agg:
        {
            m_drawingPart->SetDrawer2D( new a2dAggDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#if wxART2D_USE_GRAPHICS_CONTEXT
        case Switch_Drawer_agggc:
        {
            m_drawingPart->SetDrawer2D( new a2dGcAggDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#endif // wxART2D_USE_GRAPHICS_CONTEXT

#endif //wxART2D_USE_AGGDRAWER
        case Switch_Drawer_dc:
            m_drawingPart->SetDrawer2D( new a2dMemDcDrawer( width, height ) );
            m_contr->SetUseOpaqueEditcopy( false );
            break;

#if wxART2D_USE_GDIPLUSDRAWER
        case Switch_Drawer_gdiplus:
        {
            m_drawingPart->SetDrawer2D( new a2dGDIPlusDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#endif //wxART2D_USE_GDIPLUSDRAWER

#if wxART2D_USE_GRAPHICS_CONTEXT
        case Switch_Drawer_dcgc:
        {
            m_drawingPart->SetDrawer2D( new a2dGcDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
        case Switch_Drawer_gdigc:
        {
            m_drawingPart->SetDrawer2D( new a2dNativeGcDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#endif // wxART2D_USE_GRAPHICS_CONTEXT
        default:
            // skip Refresh()
            return;
    }

    m_drawingPart->GetDrawer2D()->SetYaxis( true );
    m_drawingPart->SetMappingShowAll();
}








