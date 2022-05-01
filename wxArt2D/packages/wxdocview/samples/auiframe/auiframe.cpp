/*! \file docview/samples/aui/auiframe.cpp
    \author Klaas Holwerda

    Copyright: (C) 1998,  Malcolm Nealon Klaas Holwerda ( based on wxWidgets sample by  Benjamin I. Williams )

    Licence: wxWidgets Licence

    RCS-ID: $Id: auiframe.cpp,v 1.7 2009/07/28 19:47:14 titato Exp $
*/

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "auiframe.h"

#include "wx/colordlg.h"
#include <wx/wfstream.h>

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#include "docdialog.h"
#include "doc.h"

IMPLEMENT_APP( MyApp )

//-----------------------------------------------------------------------------
// MyApp
//-----------------------------------------------------------------------------

bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    a2dDocumentCommandProcessor* docmanager = a2dDocviewGlobals->GetDocviewCommandProcessor();

    wxNotebookConnector<MyNotebook, a2dDocumentViewScrolledWindow>* notebookconnector =
        new wxNotebookConnector<MyNotebook, a2dDocumentViewScrolledWindow>();

    a2dDocumentTemplate* doctemplatenew;

    doctemplatenew = new a2dDocumentTemplateAuto(
        _T( "All Files" ),
        _T( "*.*" ),
        _T( "" ),
        _T( "" ),
        _T( "DrawingDocument" ),
        CLASSINFO( DrawingDocument ),
        notebookconnector,
        a2dTemplateFlag::DEFAULT );
    docmanager->AssociateDocTemplate( doctemplatenew );


    DoodleDocumentIoHandlerIn* iohandlerFormatIn = new DoodleDocumentIoHandlerIn();
    DoodleDocumentIoHandlerOut* iohandlerFormatOut = new DoodleDocumentIoHandlerOut();

    // Create a template relating drawing documents to their files
    doctemplatenew = new a2dDocumentTemplate(
        wxT( "DRW Drawing" ),
        wxT( "*.drw" ),
        wxT( "" ),
        wxT( "drw" ),
        wxT( "DrawingDocument" ),
        CLASSINFO( DrawingDocument ),
        notebookconnector,
        a2dTemplateFlag::DEFAULT,
        iohandlerFormatIn,
        iohandlerFormatOut );
    docmanager->AssociateDocTemplate( doctemplatenew );

    a2dViewTemplate* viewtemplatenew;

    // Create a template relating drawing documents to their views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Drawing View" ),
        CLASSINFO( DrawingView ),
        notebookconnector,
        a2dTemplateFlag::DEFAULT );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    // Create a template relating drawing documents to statistics views
    viewtemplatenew = new a2dViewTemplate(
        wxT( "DrawingDocument View" ),
        wxT( "DrawingDocument" ),
        wxT( "Statistics View" ),
        CLASSINFO( StatisticsView ),
        notebookconnector,
        a2dTemplateFlag::DEFAULT );
    docmanager->AssociateViewTemplate( viewtemplatenew );

    //// Create the main frame window
    m_frame = new MyFrame(
        notebookconnector,
        NULL,
        -1,
        wxT( "Tabframe with docmanager" ),
        wxPoint( 10, 110 ),
        wxSize( 500, 400 ),
        wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE );

    SetTopWindow( m_frame );
    m_frame->Show();

    return true;
}

int MyApp::OnExit()
{
    return 0;
}


//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------
extern const long WXDOCVIEW_ABOUT11 = wxNewId();
extern const long WXDOCVIEW_ABOUT12 = wxNewId();
extern const long WXDOCVIEW_CONNECTWINDOW = wxNewId();
extern const long WXDOCVIEW_CONNECTWINDOW1 = wxNewId();
extern const long WXDOCVIEW_CONNECTWINDOW2 = wxNewId();

BEGIN_EVENT_TABLE( MyFrame, EditorFrame )
    EVT_ERASE_BACKGROUND( MyFrame::OnEraseBackground )
    EVT_SIZE( MyFrame::OnSize )
    EVT_MENU( MyFrame::ID_CreateTree, MyFrame::OnCreateTree )
    EVT_MENU( MyFrame::ID_CreateGrid, MyFrame::OnCreateGrid )
    EVT_MENU( MyFrame::ID_CreateText, MyFrame::OnCreateText )
    EVT_MENU( MyFrame::ID_CreateHTML, MyFrame::OnCreateHTML )
    EVT_MENU( MyFrame::ID_CreateSizeReport, MyFrame::OnCreateSizeReport )
    EVT_MENU( MyFrame::ID_CreateNotebook, MyFrame::OnCreateNotebook )
    EVT_MENU( MyFrame::ID_CreatePerspective, MyFrame::OnCreatePerspective )
    EVT_MENU( MyFrame::ID_CopyPerspectiveCode, MyFrame::OnCopyPerspectiveCode )
    EVT_MENU( ID_AllowFloating, MyFrame::OnManagerFlag )
    EVT_MENU( ID_TransparentHint, MyFrame::OnManagerFlag )
    EVT_MENU( ID_VenetianBlindsHint, MyFrame::OnManagerFlag )
    EVT_MENU( ID_RectangleHint, MyFrame::OnManagerFlag )
    EVT_MENU( ID_NoHint, MyFrame::OnManagerFlag )
    EVT_MENU( ID_HintFade, MyFrame::OnManagerFlag )
    EVT_MENU( ID_NoVenetianFade, MyFrame::OnManagerFlag )
    EVT_MENU( ID_TransparentDrag, MyFrame::OnManagerFlag )
    EVT_MENU( ID_AllowActivePane, MyFrame::OnManagerFlag )
    EVT_MENU( ID_NotebookTabFixedWidth, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NotebookNoCloseButton, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NotebookCloseButton, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NotebookCloseButtonAll, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NotebookCloseButtonActive, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NotebookAllowTabMove, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NotebookAllowTabExternalMove, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NotebookAllowTabSplit, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NotebookScrollButtons, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NotebookWindowList, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NotebookArtGloss, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NotebookArtSimple, MyFrame::OnNotebookFlag )
    EVT_MENU( ID_NoGradient, MyFrame::OnGradient )
    EVT_MENU( ID_VerticalGradient, MyFrame::OnGradient )
    EVT_MENU( ID_HorizontalGradient, MyFrame::OnGradient )
    EVT_MENU( ID_Settings, MyFrame::OnSettings )
    EVT_MENU( ID_GridContent, MyFrame::OnChangeContentPane )
    EVT_MENU( ID_TreeContent, MyFrame::OnChangeContentPane )
    EVT_MENU( ID_TextContent, MyFrame::OnChangeContentPane )
    EVT_MENU( ID_SizeReportContent, MyFrame::OnChangeContentPane )
    EVT_MENU( ID_HTMLContent, MyFrame::OnChangeContentPane )
    EVT_MENU( ID_NotebookContent, MyFrame::OnChangeContentPane )
    EVT_MENU( wxID_EXIT, MyFrame::OnExit )
    EVT_MENU( WXDOCVIEW_ABOUT12, MyFrame::OnAbout )
    EVT_UPDATE_UI( ID_NotebookTabFixedWidth, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NotebookNoCloseButton, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NotebookCloseButton, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NotebookCloseButtonAll, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NotebookCloseButtonActive, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NotebookAllowTabMove, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NotebookAllowTabExternalMove, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NotebookAllowTabSplit, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NotebookScrollButtons, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NotebookWindowList, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_AllowFloating, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_TransparentHint, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_VenetianBlindsHint, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_RectangleHint, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NoHint, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_HintFade, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NoVenetianFade, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_TransparentDrag, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_NoGradient, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_VerticalGradient, MyFrame::OnUpdateUI )
    EVT_UPDATE_UI( ID_HorizontalGradient, MyFrame::OnUpdateUI )
    EVT_MENU_RANGE( MyFrame::ID_FirstPerspective, MyFrame::ID_FirstPerspective + 1000,
                    MyFrame::OnRestorePerspective )
    EVT_AUI_PANE_CLOSE( MyFrame::OnPaneClose )
    EVT_AUINOTEBOOK_ALLOW_DND( wxID_ANY, MyFrame::OnAllowNotebookDnD )
    EVT_AUINOTEBOOK_PAGE_CLOSE( wxID_ANY, MyFrame::OnNotebookPageClose )
    EVT_MENU_RANGE( wxID_FILE1, wxID_FILE9, MyFrame::OnMRUFile )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_MENU( WXDOCVIEW_ABOUT11, MyFrame::OnAUIAbout )
    EVT_CLOSE_VIEW( MyFrame::OnCloseView )
//    EVT_AUINOTEBOOK_PAGE_CHANGED( ID_AUINOTEBOOK1, MyFrame::OnAuinotebook1PageChanged )

END_EVENT_TABLE()


MyFrame::MyFrame(
    wxNotebookConnector<MyNotebook, a2dDocumentViewScrolledWindow>* connector,
    wxFrame* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& position,
    const wxSize& size,
    const long style )
    : EditorFrame( true, parent, NULL, -1, title, position, size, style )
{
    m_help_menu->Append( WXDOCVIEW_ABOUT11, _( "&About Aui" ) );

    SetStatusText( wxT( "Welcome to TabFrame Managed sample!" ) );

    m_notebookconnector = connector;
    m_notebook = new MyNotebook( this );
    m_notebookconnector->Init( m_notebook );

    DrawingView* view1 = new DrawingView();

    //make a new document and add it to the document manager and view.
    DrawingDocument* doc = new DrawingDocument();
    doc->SetFilename( wxFileName( wxT( "dummy.drw" ) ) );
    doc->CreateCommandProcessor();
    view1->SetDocument( doc );

    a2dDocviewGlobals->GetDocviewCommandProcessor()->AddCreatedDocument( doc );

    m_canvas = new a2dDocumentViewScrolledWindow( view1, m_notebook, -1 , wxPoint( 0, 0 ), size, wxNO_FULL_REPAINT_ON_RESIZE );
    m_canvas->SetCursor( wxCURSOR_PENCIL );

    m_notebook->AddPage( m_canvas, wxT( "window1" ), true );

    // Give it scrollbars
    m_canvas->SetScrollbars( 20, 20, 50, 50 );
    m_canvas->SetBackgroundColour( *wxWHITE );
    m_canvas->ClearBackground();

    //for the in editor templates ( see EditFrame )
    m_singleconnector->SetDisplayWindow( m_canvas );

    // Make sure the document manager knows that this is the
    // current view.
    view1->Activate( true );
    doc->GetCommandProcessor()->Initialize();

    // tell wxAuiManager to manage this frame
    m_mgr.SetManagedWindow( this );

    // set up default notebook style
    m_notebook_style = wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER;
    m_notebook_theme = 0;

    wxMenuBar* menu_bar = GetMenuBar();

    wxMenu* view_menu = new wxMenu;
    view_menu->Append( ID_CreateText, _( "Create Text Control" ) );
    view_menu->Append( ID_CreateHTML, _( "Create HTML Control" ) );
    view_menu->Append( ID_CreateTree, _( "Create Tree" ) );
    view_menu->Append( ID_CreateGrid, _( "Create Grid" ) );
    view_menu->Append( ID_CreateNotebook, _( "Create Notebook" ) );
    view_menu->Append( ID_CreateSizeReport, _( "Create Size Reporter" ) );
    view_menu->AppendSeparator();
    view_menu->Append( ID_GridContent, _( "Use a Grid for the Content Pane" ) );
    view_menu->Append( ID_TextContent, _( "Use a Text Control for the Content Pane" ) );
    view_menu->Append( ID_HTMLContent, _( "Use an HTML Control for the Content Pane" ) );
    view_menu->Append( ID_TreeContent, _( "Use a Tree Control for the Content Pane" ) );
    view_menu->Append( ID_NotebookContent, _( "Use a wxAuiNotebook control for the Content Pane" ) );
    view_menu->Append( ID_SizeReportContent, _( "Use a Size Reporter for the Content Pane" ) );

    wxMenu* options_menu = new wxMenu;
    options_menu->AppendRadioItem( ID_TransparentHint, _( "Transparent Hint" ) );
    options_menu->AppendRadioItem( ID_VenetianBlindsHint, _( "Venetian Blinds Hint" ) );
    options_menu->AppendRadioItem( ID_RectangleHint, _( "Rectangle Hint" ) );
    options_menu->AppendRadioItem( ID_NoHint, _( "No Hint" ) );
    options_menu->AppendSeparator();
    options_menu->AppendCheckItem( ID_HintFade, _( "Hint Fade-in" ) );
    options_menu->AppendCheckItem( ID_AllowFloating, _( "Allow Floating" ) );
    options_menu->AppendCheckItem( ID_NoVenetianFade, _( "Disable Venetian Blinds Hint Fade-in" ) );
    options_menu->AppendCheckItem( ID_TransparentDrag, _( "Transparent Drag" ) );
    options_menu->AppendCheckItem( ID_AllowActivePane, _( "Allow Active Pane" ) );
    options_menu->AppendSeparator();
    options_menu->AppendRadioItem( ID_NoGradient, _( "No Caption Gradient" ) );
    options_menu->AppendRadioItem( ID_VerticalGradient, _( "Vertical Caption Gradient" ) );
    options_menu->AppendRadioItem( ID_HorizontalGradient, _( "Horizontal Caption Gradient" ) );
    options_menu->AppendSeparator();
    options_menu->Append( ID_Settings, _( "Settings Pane" ) );

    wxMenu* notebook_menu = new wxMenu;
    notebook_menu->AppendRadioItem( ID_NotebookArtGloss, _( "Glossy Theme (Default)" ) );
    notebook_menu->AppendRadioItem( ID_NotebookArtSimple, _( "Simple Theme" ) );
    notebook_menu->AppendSeparator();
    notebook_menu->AppendRadioItem( ID_NotebookNoCloseButton, _( "No Close Button" ) );
    notebook_menu->AppendRadioItem( ID_NotebookCloseButton, _( "Close Button at Right" ) );
    notebook_menu->AppendRadioItem( ID_NotebookCloseButtonAll, _( "Close Button on All Tabs" ) );
    notebook_menu->AppendRadioItem( ID_NotebookCloseButtonActive, _( "Close Button on Active Tab" ) );
    notebook_menu->AppendSeparator();
    notebook_menu->AppendCheckItem( ID_NotebookAllowTabMove, _( "Allow Tab Move" ) );
    notebook_menu->AppendCheckItem( ID_NotebookAllowTabExternalMove, _( "Allow External Tab Move" ) );
    notebook_menu->AppendCheckItem( ID_NotebookAllowTabSplit, _( "Allow Notebook Split" ) );
    notebook_menu->AppendCheckItem( ID_NotebookScrollButtons, _( "Scroll Buttons Visible" ) );
    notebook_menu->AppendCheckItem( ID_NotebookWindowList, _( "Window List Button Visible" ) );
    notebook_menu->AppendCheckItem( ID_NotebookTabFixedWidth, _( "Fixed-width Tabs" ) );

    m_perspectives_menu = new wxMenu;
    m_perspectives_menu->Append( ID_CreatePerspective, _( "Create Perspective" ) );
    m_perspectives_menu->Append( ID_CopyPerspectiveCode, _( "Copy Perspective Data To Clipboard" ) );
    m_perspectives_menu->AppendSeparator();
    m_perspectives_menu->Append( ID_FirstPerspective + 0, _( "Default Startup" ) );
    m_perspectives_menu->Append( ID_FirstPerspective + 1, _( "All Panes" ) );

    m_help_menu->Append( WXDOCVIEW_ABOUT12, _( "About base" ) );

    menu_bar->Append( view_menu, _( "View" ) );
    menu_bar->Append( m_perspectives_menu, _( "Perspectives" ) );
    menu_bar->Append( options_menu, _( "Options" ) );
    menu_bar->Append( notebook_menu, _( "Notebook" ) );

    GetStatusBar()->SetStatusText( _( "Ready" ) );


    // min size for the frame itself isn't completely done.
    // see the end up wxAuiManager::Update() for the test
    // code. For now, just hard code a frame minimum size
    SetMinSize( wxSize( 400, 300 ) );

    // create some toolbars
    wxToolBar* tb1 = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                    wxTB_FLAT | wxTB_NODIVIDER );
    tb1->SetToolBitmapSize( wxSize( 48, 48 ) );
    tb1->AddTool( 101, wxT( "Test" ), wxArtProvider::GetBitmap( wxART_ERROR ) );
    tb1->AddSeparator();
    tb1->AddTool( 102, wxT( "Test" ), wxArtProvider::GetBitmap( wxART_QUESTION ) );
    tb1->AddTool( 103, wxT( "Test" ), wxArtProvider::GetBitmap( wxART_INFORMATION ) );
    tb1->AddTool( 103, wxT( "Test" ), wxArtProvider::GetBitmap( wxART_WARNING ) );
    tb1->AddTool( 103, wxT( "Test" ), wxArtProvider::GetBitmap( wxART_MISSING_IMAGE ) );
    tb1->Realize();


    wxToolBar* tb2 = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                    wxTB_FLAT | wxTB_NODIVIDER );
    tb2->SetToolBitmapSize( wxSize( 16, 16 ) );

    wxBitmap tb2_bmp1 = wxArtProvider::GetBitmap( wxART_QUESTION, wxART_OTHER, wxSize( 16, 16 ) );
    tb2->AddTool( 101, wxT( "Test" ), tb2_bmp1 );
    tb2->AddTool( 101, wxT( "Test" ), tb2_bmp1 );
    tb2->AddTool( 101, wxT( "Test" ), tb2_bmp1 );
    tb2->AddTool( 101, wxT( "Test" ), tb2_bmp1 );
    tb2->AddSeparator();
    tb2->AddTool( 101, wxT( "Test" ), tb2_bmp1 );
    tb2->AddTool( 101, wxT( "Test" ), tb2_bmp1 );
    tb2->AddSeparator();
    tb2->AddTool( 101, wxT( "Test" ), tb2_bmp1 );
    tb2->AddTool( 101, wxT( "Test" ), tb2_bmp1 );
    tb2->AddTool( 101, wxT( "Test" ), tb2_bmp1 );
    tb2->AddTool( 101, wxT( "Test" ), tb2_bmp1 );
    tb2->Realize();


    wxToolBar* tb3 = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                    wxTB_FLAT | wxTB_NODIVIDER );
    tb3->SetToolBitmapSize( wxSize( 16, 16 ) );
    wxBitmap tb3_bmp1 = wxArtProvider::GetBitmap( wxART_FOLDER, wxART_OTHER, wxSize( 16, 16 ) );
    tb3->AddTool( 101, wxT( "Test" ), tb3_bmp1 );
    tb3->AddTool( 101, wxT( "Test" ), tb3_bmp1 );
    tb3->AddTool( 101, wxT( "Test" ), tb3_bmp1 );
    tb3->AddTool( 101, wxT( "Test" ), tb3_bmp1 );
    tb3->AddSeparator();
    tb3->AddTool( 101, wxT( "Test" ), tb3_bmp1 );
    tb3->AddTool( 101, wxT( "Test" ), tb3_bmp1 );
    tb3->Realize();


    wxToolBar* tb4 = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                    wxTB_FLAT | wxTB_NODIVIDER | wxTB_HORZ_TEXT );
    tb4->SetToolBitmapSize( wxSize( 16, 16 ) );
    wxBitmap tb4_bmp1 = wxArtProvider::GetBitmap( wxART_NORMAL_FILE, wxART_OTHER, wxSize( 16, 16 ) );
    tb4->AddTool( 101, wxT( "Item 1" ), tb4_bmp1 );
    tb4->AddTool( 101, wxT( "Item 2" ), tb4_bmp1 );
    tb4->AddTool( 101, wxT( "Item 3" ), tb4_bmp1 );
    tb4->AddTool( 101, wxT( "Item 4" ), tb4_bmp1 );
    tb4->AddSeparator();
    tb4->AddTool( 101, wxT( "Item 5" ), tb4_bmp1 );
    tb4->AddTool( 101, wxT( "Item 6" ), tb4_bmp1 );
    tb4->AddTool( 101, wxT( "Item 7" ), tb4_bmp1 );
    tb4->AddTool( 101, wxT( "Item 8" ), tb4_bmp1 );
    tb4->Realize();

    // create some toolbars
    wxToolBar* tb5 = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                    wxTB_FLAT | wxTB_NODIVIDER | wxTB_VERTICAL );
    tb5->SetToolBitmapSize( wxSize( 48, 48 ) );
    tb5->AddTool( 101, wxT( "Test" ), wxArtProvider::GetBitmap( wxART_ERROR ) );
    tb5->AddSeparator();
    tb5->AddTool( 102, wxT( "Test" ), wxArtProvider::GetBitmap( wxART_QUESTION ) );
    tb5->AddTool( 103, wxT( "Test" ), wxArtProvider::GetBitmap( wxART_INFORMATION ) );
    tb5->AddTool( 103, wxT( "Test" ), wxArtProvider::GetBitmap( wxART_WARNING ) );
    tb5->AddTool( 103, wxT( "Test" ), wxArtProvider::GetBitmap( wxART_MISSING_IMAGE ) );
    tb5->Realize();


//    m_auinotebook = new wxAuiNotebook( this, ID_AUINOTEBOOK1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE );

    // add a bunch of panes
    m_mgr.AddPane( CreateSizeReportCtrl(), wxAuiPaneInfo().
                   Name( wxT( "test1" ) ).Caption( wxT( "Pane Caption" ) ).
                   Top() );

    m_mgr.AddPane( CreateSizeReportCtrl(), wxAuiPaneInfo().
                   Name( wxT( "test2" ) ).Caption( wxT( "Client Size Reporter" ) ).
                   Bottom().Position( 1 ).
                   CloseButton( true ).MaximizeButton( true ) );

    m_mgr.AddPane( CreateSizeReportCtrl(), wxAuiPaneInfo().
                   Name( wxT( "test3" ) ).Caption( wxT( "Client Size Reporter" ) ).
                   Bottom().
                   CloseButton( true ).MaximizeButton( true ) );

    m_mgr.AddPane( CreateSizeReportCtrl(), wxAuiPaneInfo().
                   Name( wxT( "test4" ) ).Caption( wxT( "Pane Caption" ) ).
                   Left() );

    m_mgr.AddPane( CreateSizeReportCtrl(), wxAuiPaneInfo().
                   Name( wxT( "test5" ) ).Caption( wxT( "No Close Button" ) ).
                   Right().CloseButton( false ) );

    m_mgr.AddPane( CreateSizeReportCtrl(), wxAuiPaneInfo().
                   Name( wxT( "test6" ) ).Caption( wxT( "Client Size Reporter" ) ).
                   Right().Row( 1 ).
                   CloseButton( true ).MaximizeButton( true ) );

    m_mgr.AddPane( CreateSizeReportCtrl(), wxAuiPaneInfo().
                   Name( wxT( "test7" ) ).Caption( wxT( "Client Size Reporter" ) ).
                   Left().Layer( 1 ).
                   CloseButton( true ).MaximizeButton( true ) );

    m_mgr.AddPane( CreateTreeCtrl(), wxAuiPaneInfo().
                   Name( wxT( "test8" ) ).Caption( wxT( "Tree Pane" ) ).
                   Left().Layer( 1 ).Position( 1 ).
                   CloseButton( true ).MaximizeButton( true ) );

    m_mgr.AddPane( CreateSizeReportCtrl(), wxAuiPaneInfo().
                   Name( wxT( "test9" ) ).Caption( wxT( "Min Size 200x100" ) ).
                   BestSize( wxSize( 200, 100 ) ).MinSize( wxSize( 200, 100 ) ).
                   Bottom().Layer( 1 ).
                   CloseButton( true ).MaximizeButton( true ) );

    wxWindow* wnd10 = CreateTextCtrl( wxT( "This pane will prompt the user before hiding." ) );
    m_mgr.AddPane( wnd10, wxAuiPaneInfo().
                   Name( wxT( "test10" ) ).Caption( wxT( "Text Pane with Hide Prompt" ) ).
                   Bottom().Layer( 1 ).Position( 1 ) );

    m_mgr.AddPane( CreateSizeReportCtrl(), wxAuiPaneInfo().
                   Name( wxT( "test11" ) ).Caption( wxT( "Fixed Pane" ) ).
                   Bottom().Layer( 1 ).Position( 2 ).Fixed() );


    m_mgr.AddPane( new SettingsPanel( this, this ), wxAuiPaneInfo().
                   Name( wxT( "settings" ) ).Caption( wxT( "Dock Manager Settings" ) ).
                   Dockable( false ).Float().Hide() );

    // create some center panes

    m_mgr.AddPane( CreateGrid(), wxAuiPaneInfo().Name( wxT( "grid_content" ) ).
                   CenterPane().Hide() );

    m_mgr.AddPane( CreateTreeCtrl(), wxAuiPaneInfo().Name( wxT( "tree_content" ) ).
                   CenterPane().Hide() );

    m_mgr.AddPane( CreateSizeReportCtrl(), wxAuiPaneInfo().Name( wxT( "sizereport_content" ) ).
                   CenterPane().Hide() );

    m_mgr.AddPane( CreateTextCtrl(), wxAuiPaneInfo().Name( wxT( "text_content" ) ).
                   CenterPane().Hide() );

    m_mgr.AddPane( CreateHTMLCtrl(), wxAuiPaneInfo().Name( wxT( "html_content" ) ).
                   CenterPane().Hide() );

    m_mgr.AddPane( m_notebook, wxAuiPaneInfo().Name( wxT( "notebook_content" ) ).
                   CenterPane().PaneBorder( false ) );

    // add the toolbars to the manager
    m_mgr.AddPane( tb1, wxAuiPaneInfo().
                   Name( wxT( "tb1" ) ).Caption( wxT( "Big Toolbar" ) ).
                   ToolbarPane().Top().
                   LeftDockable( false ).RightDockable( false ) );

    m_mgr.AddPane( tb2, wxAuiPaneInfo().
                   Name( wxT( "tb2" ) ).Caption( wxT( "Toolbar 2" ) ).
                   ToolbarPane().Top().Row( 1 ).
                   LeftDockable( false ).RightDockable( false ) );

    m_mgr.AddPane( tb3, wxAuiPaneInfo().
                   Name( wxT( "tb3" ) ).Caption( wxT( "Toolbar 3" ) ).
                   ToolbarPane().Top().Row( 1 ).Position( 1 ).
                   LeftDockable( false ).RightDockable( false ) );

    m_mgr.AddPane( tb4, wxAuiPaneInfo().
                   Name( wxT( "tb4" ) ).Caption( wxT( "Sample Bookmark Toolbar" ) ).
                   ToolbarPane().Top().Row( 2 ).
                   LeftDockable( false ).RightDockable( false ) );

    m_mgr.AddPane( tb5, wxAuiPaneInfo().
                   Name( wxT( "tb5" ) ).Caption( wxT( "Sample Vertical Toolbar" ) ).
                   ToolbarPane().Left().
                   GripperTop().
                   TopDockable( false ).BottomDockable( false ) );

    m_mgr.AddPane( new wxButton( this, wxID_ANY, _( "Test Button" ) ),
                   wxAuiPaneInfo().Name( wxT( "tb6" ) ).
                   ToolbarPane().Top().Row( 2 ).Position( 1 ).
                   LeftDockable( false ).RightDockable( false ) );

    // make some default perspectives

    wxString perspective_all = m_mgr.SavePerspective();

    int i, count;
    wxAuiPaneInfoArray& all_panes = m_mgr.GetAllPanes();
    for ( i = 0, count = all_panes.GetCount(); i < count; ++i )
        if ( !all_panes.Item( i ).IsToolbar() )
            all_panes.Item( i ).Hide();
    m_mgr.GetPane( wxT( "tb1" ) ).Hide();
    m_mgr.GetPane( wxT( "tb6" ) ).Hide();
    m_mgr.GetPane( wxT( "test8" ) ).Show().Left().Layer( 0 ).Row( 0 ).Position( 0 );
    m_mgr.GetPane( wxT( "test10" ) ).Show().Bottom().Layer( 0 ).Row( 0 ).Position( 0 );
    m_mgr.GetPane( wxT( "notebook_content" ) ).Show();
    wxString perspective_default = m_mgr.SavePerspective();

    m_perspectives.Add( perspective_default );
    m_perspectives.Add( perspective_all );

    // "commit" all changes made to wxAuiManager
    m_mgr.Update();
}

MyFrame::~MyFrame()
{
    m_mgr.UnInit();
}

wxAuiDockArt* MyFrame::GetDockArt()
{
    return m_mgr.GetArtProvider();
}

void MyFrame::DoUpdate()
{
    m_mgr.Update();
}

void MyFrame::OnEraseBackground( wxEraseEvent& event )
{
    event.Skip();
}

void MyFrame::OnSize( wxSizeEvent& event )
{
    event.Skip();
}

void MyFrame::OnSettings( wxCommandEvent& WXUNUSED( event ) )
{
    // show the settings pane, and float it
    wxAuiPaneInfo& floating_pane = m_mgr.GetPane( wxT( "settings" ) ).Float().Show();

    if ( floating_pane.floating_pos == wxDefaultPosition )
        floating_pane.FloatingPosition( GetStartPosition() );

    m_mgr.Update();
}

void MyFrame::OnGradient( wxCommandEvent& event )
{
    int gradient = 0;

    switch ( event.GetId() )
    {
        case ID_NoGradient:         gradient = wxAUI_GRADIENT_NONE; break;
        case ID_VerticalGradient:   gradient = wxAUI_GRADIENT_VERTICAL; break;
        case ID_HorizontalGradient: gradient = wxAUI_GRADIENT_HORIZONTAL; break;
    }

    m_mgr.GetArtProvider()->SetMetric( wxAUI_DOCKART_GRADIENT_TYPE, gradient );
    m_mgr.Update();
}

void MyFrame::OnManagerFlag( wxCommandEvent& event )
{
    unsigned int flag = 0;

#if !defined(__WXMSW__) && !defined(__WXMAC__) && !defined(__WXGTK__)
    if ( event.GetId() == ID_TransparentDrag ||
            event.GetId() == ID_TransparentHint ||
            event.GetId() == ID_HintFade )
    {
        wxMessageBox( wxT( "This option is presently only available on wxGTK, wxMSW and wxMac" ) );
        return;
    }
#endif

    int id = event.GetId();

    if ( id == ID_TransparentHint ||
            id == ID_VenetianBlindsHint ||
            id == ID_RectangleHint ||
            id == ID_NoHint )
    {
        unsigned int flags = m_mgr.GetFlags();
        flags &= ~wxAUI_MGR_TRANSPARENT_HINT;
        flags &= ~wxAUI_MGR_VENETIAN_BLINDS_HINT;
        flags &= ~wxAUI_MGR_RECTANGLE_HINT;
        m_mgr.SetFlags( flags );
    }

    switch ( id )
    {
        case ID_AllowFloating: flag = wxAUI_MGR_ALLOW_FLOATING; break;
        case ID_TransparentDrag: flag = wxAUI_MGR_TRANSPARENT_DRAG; break;
        case ID_HintFade: flag = wxAUI_MGR_HINT_FADE; break;
        case ID_NoVenetianFade: flag = wxAUI_MGR_NO_VENETIAN_BLINDS_FADE; break;
        case ID_AllowActivePane: flag = wxAUI_MGR_ALLOW_ACTIVE_PANE; break;
        case ID_TransparentHint: flag = wxAUI_MGR_TRANSPARENT_HINT; break;
        case ID_VenetianBlindsHint: flag = wxAUI_MGR_VENETIAN_BLINDS_HINT; break;
        case ID_RectangleHint: flag = wxAUI_MGR_RECTANGLE_HINT; break;
    }

    if ( flag )
    {
        m_mgr.SetFlags( m_mgr.GetFlags() ^ flag );
    }

    m_mgr.Update();
}


void MyFrame::OnNotebookFlag( wxCommandEvent& event )
{
    int id = event.GetId();

    if ( id == ID_NotebookNoCloseButton ||
            id == ID_NotebookCloseButton ||
            id == ID_NotebookCloseButtonAll ||
            id == ID_NotebookCloseButtonActive )
    {
        m_notebook_style &= ~( wxAUI_NB_CLOSE_BUTTON |
                               wxAUI_NB_CLOSE_ON_ACTIVE_TAB |
                               wxAUI_NB_CLOSE_ON_ALL_TABS );

        switch ( id )
        {
            case ID_NotebookNoCloseButton: break;
            case ID_NotebookCloseButton: m_notebook_style |= wxAUI_NB_CLOSE_BUTTON; break;
            case ID_NotebookCloseButtonAll: m_notebook_style |= wxAUI_NB_CLOSE_ON_ALL_TABS; break;
            case ID_NotebookCloseButtonActive: m_notebook_style |= wxAUI_NB_CLOSE_ON_ACTIVE_TAB; break;
        }
    }

    if ( id == ID_NotebookAllowTabMove )
    {
        m_notebook_style ^= wxAUI_NB_TAB_MOVE;
    }
    if ( id == ID_NotebookAllowTabExternalMove )
    {
        m_notebook_style ^= wxAUI_NB_TAB_EXTERNAL_MOVE;
    }
    else if ( id == ID_NotebookAllowTabSplit )
    {
        m_notebook_style ^= wxAUI_NB_TAB_SPLIT;
    }
    else if ( id == ID_NotebookWindowList )
    {
        m_notebook_style ^= wxAUI_NB_WINDOWLIST_BUTTON;
    }
    else if ( id == ID_NotebookScrollButtons )
    {
        m_notebook_style ^= wxAUI_NB_SCROLL_BUTTONS;
    }
    else if ( id == ID_NotebookTabFixedWidth )
    {
        m_notebook_style ^= wxAUI_NB_TAB_FIXED_WIDTH;
    }


    size_t i, count;
    wxAuiPaneInfoArray& all_panes = m_mgr.GetAllPanes();
    for ( i = 0, count = all_panes.GetCount(); i < count; ++i )
    {
        wxAuiPaneInfo& pane = all_panes.Item( i );
        if ( pane.window->IsKindOf( CLASSINFO( wxAuiNotebook ) ) )
        {
            wxAuiNotebook* nb = ( wxAuiNotebook* )pane.window;

            if ( id == ID_NotebookArtGloss )
            {
                nb->SetArtProvider( new wxAuiDefaultTabArt );
                m_notebook_theme = 0;
            }
            else if ( id == ID_NotebookArtSimple )
            {
                nb->SetArtProvider( new wxAuiSimpleTabArt );
                m_notebook_theme = 1;
            }


            nb->SetWindowStyleFlag( m_notebook_style );
            nb->Refresh();
        }
    }


}


void MyFrame::OnUpdateUI( wxUpdateUIEvent& event )
{
    unsigned int flags = m_mgr.GetFlags();

    switch ( event.GetId() )
    {
        case ID_NoGradient:
            event.Check( m_mgr.GetArtProvider()->GetMetric( wxAUI_DOCKART_GRADIENT_TYPE ) == wxAUI_GRADIENT_NONE );
            break;
        case ID_VerticalGradient:
            event.Check( m_mgr.GetArtProvider()->GetMetric( wxAUI_DOCKART_GRADIENT_TYPE ) == wxAUI_GRADIENT_VERTICAL );
            break;
        case ID_HorizontalGradient:
            event.Check( m_mgr.GetArtProvider()->GetMetric( wxAUI_DOCKART_GRADIENT_TYPE ) == wxAUI_GRADIENT_HORIZONTAL );
            break;
        case ID_AllowFloating:
            event.Check( ( flags & wxAUI_MGR_ALLOW_FLOATING ) != 0 );
            break;
        case ID_TransparentDrag:
            event.Check( ( flags & wxAUI_MGR_TRANSPARENT_DRAG ) != 0 );
            break;
        case ID_TransparentHint:
            event.Check( ( flags & wxAUI_MGR_TRANSPARENT_HINT ) != 0 );
            break;
        case ID_VenetianBlindsHint:
            event.Check( ( flags & wxAUI_MGR_VENETIAN_BLINDS_HINT ) != 0 );
            break;
        case ID_RectangleHint:
            event.Check( ( flags & wxAUI_MGR_RECTANGLE_HINT ) != 0 );
            break;
        case ID_NoHint:
            event.Check( ( ( wxAUI_MGR_TRANSPARENT_HINT |
                             wxAUI_MGR_VENETIAN_BLINDS_HINT |
                             wxAUI_MGR_RECTANGLE_HINT ) & flags ) == 0 );
            break;
        case ID_HintFade:
            event.Check( ( flags & wxAUI_MGR_HINT_FADE ) != 0 );
            break;
        case ID_NoVenetianFade:
            event.Check( ( flags & wxAUI_MGR_NO_VENETIAN_BLINDS_FADE ) != 0 );
            break;

        case ID_NotebookNoCloseButton:
            event.Check( ( m_notebook_style & ( wxAUI_NB_CLOSE_BUTTON | wxAUI_NB_CLOSE_ON_ALL_TABS | wxAUI_NB_CLOSE_ON_ACTIVE_TAB ) ) != 0 );
            break;
        case ID_NotebookCloseButton:
            event.Check( ( m_notebook_style & wxAUI_NB_CLOSE_BUTTON ) != 0 );
            break;
        case ID_NotebookCloseButtonAll:
            event.Check( ( m_notebook_style & wxAUI_NB_CLOSE_ON_ALL_TABS ) != 0 );
            break;
        case ID_NotebookCloseButtonActive:
            event.Check( ( m_notebook_style & wxAUI_NB_CLOSE_ON_ACTIVE_TAB ) != 0 );
            break;
        case ID_NotebookAllowTabSplit:
            event.Check( ( m_notebook_style & wxAUI_NB_TAB_SPLIT ) != 0 );
            break;
        case ID_NotebookAllowTabMove:
            event.Check( ( m_notebook_style & wxAUI_NB_TAB_MOVE ) != 0 );
            break;
        case ID_NotebookAllowTabExternalMove:
            event.Check( ( m_notebook_style & wxAUI_NB_TAB_EXTERNAL_MOVE ) != 0 );
            break;
        case ID_NotebookScrollButtons:
            event.Check( ( m_notebook_style & wxAUI_NB_SCROLL_BUTTONS ) != 0 );
            break;
        case ID_NotebookWindowList:
            event.Check( ( m_notebook_style & wxAUI_NB_WINDOWLIST_BUTTON ) != 0 );
            break;
        case ID_NotebookTabFixedWidth:
            event.Check( ( m_notebook_style & wxAUI_NB_TAB_FIXED_WIDTH ) != 0 );
            break;
        case ID_NotebookArtGloss:
            event.Check( m_notebook_style == 0 );
            break;
        case ID_NotebookArtSimple:
            event.Check( m_notebook_style == 1 );
            break;

    }
}

void MyFrame::OnPaneClose( wxAuiManagerEvent& evt )
{
    if ( evt.pane->name == wxT( "test10" ) )
    {
        int res = wxMessageBox(
                      wxT( "Are you sure you want to close/hide this pane?" ),
                      wxT( "wxAUI" ),
                      wxYES_NO,
                      this );
        if ( res != wxYES )
            evt.Veto();
    }
}

void MyFrame::OnCreatePerspective( wxCommandEvent& WXUNUSED( event ) )
{
    wxTextEntryDialog dlg( this, wxT( "Enter a name for the new perspective:" ),
                           wxT( "wxAUI Test" ) );

    dlg.SetValue( wxString::Format( wxT( "Perspective %u" ), unsigned( m_perspectives.GetCount() + 1 ) ) );
    if ( dlg.ShowModal() != wxID_OK )
        return;

    if ( m_perspectives.GetCount() == 0 )
    {
        m_perspectives_menu->AppendSeparator();
    }

    m_perspectives_menu->Append( ID_FirstPerspective + m_perspectives.GetCount(), dlg.GetValue() );
    m_perspectives.Add( m_mgr.SavePerspective() );
}

void MyFrame::OnCopyPerspectiveCode( wxCommandEvent& WXUNUSED( evt ) )
{
    wxString s = m_mgr.SavePerspective();

#if wxUSE_CLIPBOARD
    if ( wxTheClipboard->Open() )
    {
        wxTheClipboard->SetData( new wxTextDataObject( s ) );
        wxTheClipboard->Close();
    }
#endif
}

void MyFrame::OnRestorePerspective( wxCommandEvent& evt )
{
    m_mgr.LoadPerspective( m_perspectives.Item( evt.GetId() - ID_FirstPerspective ) );
}

void MyFrame::OnNotebookPageClose( wxAuiNotebookEvent& evt )
{
    wxAuiNotebook* ctrl = ( wxAuiNotebook* )evt.GetEventObject();
    if ( ctrl->GetPage( evt.GetSelection() )->IsKindOf( CLASSINFO( wxHtmlWindow ) ) )
    {
        int res = wxMessageBox( wxT( "Are you sure you want to close/hide this notebook page?" ),
                                wxT( "wxAUI" ),
                                wxYES_NO,
                                this );
        if ( res != wxYES )
            evt.Veto();
    }
}

void MyFrame::OnAllowNotebookDnD( wxAuiNotebookEvent& evt )
{
    // for the purpose of this test application, explicitly
    // allow all noteboko drag and drop events
    evt.Allow();
}

wxPoint MyFrame::GetStartPosition()
{
    static int x = 0;
    x += 20;
    wxPoint pt = ClientToScreen( wxPoint( 0, 0 ) );
    return wxPoint( pt.x + x, pt.y + x );
}

void MyFrame::OnCreateTree( wxCommandEvent& WXUNUSED( event ) )
{
    m_mgr.AddPane( CreateTreeCtrl(), wxAuiPaneInfo().
                   Caption( wxT( "Tree Control" ) ).
                   Float().FloatingPosition( GetStartPosition() ).
                   FloatingSize( wxSize( 150, 300 ) ) );
    m_mgr.Update();
}

void MyFrame::OnCreateGrid( wxCommandEvent& WXUNUSED( event ) )
{
    m_mgr.AddPane( CreateGrid(), wxAuiPaneInfo().
                   Caption( wxT( "Grid" ) ).
                   Float().FloatingPosition( GetStartPosition() ).
                   FloatingSize( wxSize( 300, 200 ) ) );
    m_mgr.Update();
}

void MyFrame::OnCreateHTML( wxCommandEvent& WXUNUSED( event ) )
{
    m_mgr.AddPane( CreateHTMLCtrl(), wxAuiPaneInfo().
                   Caption( wxT( "HTML Control" ) ).
                   Float().FloatingPosition( GetStartPosition() ).
                   FloatingSize( wxSize( 300, 200 ) ) );
    m_mgr.Update();
}

void MyFrame::OnCreateNotebook( wxCommandEvent& WXUNUSED( event ) )
{
    m_mgr.AddPane( CreateNotebook(), wxAuiPaneInfo().
                   Caption( wxT( "Notebook" ) ).
                   Float().FloatingPosition( GetStartPosition() ).
                   //FloatingSize(300,200).
                   CloseButton( true ).MaximizeButton( true ) );
    m_mgr.Update();
}

void MyFrame::OnCreateText( wxCommandEvent& WXUNUSED( event ) )
{
    m_mgr.AddPane( CreateTextCtrl(), wxAuiPaneInfo().
                   Caption( wxT( "Text Control" ) ).
                   Float().FloatingPosition( GetStartPosition() ) );
    m_mgr.Update();
}

void MyFrame::OnCreateSizeReport( wxCommandEvent& WXUNUSED( event ) )
{
    m_mgr.AddPane( CreateSizeReportCtrl(), wxAuiPaneInfo().
                   Caption( wxT( "Client Size Reporter" ) ).
                   Float().FloatingPosition( GetStartPosition() ).
                   CloseButton( true ).MaximizeButton( true ) );
    m_mgr.Update();
}

void MyFrame::OnChangeContentPane( wxCommandEvent& event )
{
    m_mgr.GetPane( wxT( "grid_content" ) ).Show( event.GetId() == ID_GridContent );
    m_mgr.GetPane( wxT( "text_content" ) ).Show( event.GetId() == ID_TextContent );
    m_mgr.GetPane( wxT( "tree_content" ) ).Show( event.GetId() == ID_TreeContent );
    m_mgr.GetPane( wxT( "sizereport_content" ) ).Show( event.GetId() == ID_SizeReportContent );
    m_mgr.GetPane( wxT( "html_content" ) ).Show( event.GetId() == ID_HTMLContent );
    m_mgr.GetPane( wxT( "notebook_content" ) ).Show( event.GetId() == ID_NotebookContent );
    m_mgr.Update();
}

void MyFrame::OnExit( wxCommandEvent& WXUNUSED( event ) )
{
    Close( true );
}

void MyFrame::OnAUIAbout( wxCommandEvent& WXUNUSED( event ) )
{
    wxMessageBox( _( "wxAUI Demo\nAn advanced window management library for wxWidgets\n(c) Copyright 2005-2006, Kirix Corporation" ), _( "About wxAUI Demo" ), wxOK, this );
}

wxTextCtrl* MyFrame::CreateTextCtrl( const wxString& ctrl_text )
{
    static int n = 0;

    wxString text;
    if ( ctrl_text.Length() > 0 )
        text = ctrl_text;
    else
        text.Printf( wxT( "This is text box %d" ), ++n );

    return new wxTextCtrl( this, wxID_ANY, text,
                           wxPoint( 0, 0 ), wxSize( 150, 90 ),
                           wxNO_BORDER | wxTE_MULTILINE );
}


wxGrid* MyFrame::CreateGrid()
{
    wxGrid* grid = new wxGrid( this, wxID_ANY,
                               wxPoint( 0, 0 ),
                               wxSize( 150, 250 ),
                               wxNO_BORDER | wxWANTS_CHARS );
    grid->CreateGrid( 50, 20 );
    return grid;
}

wxTreeCtrl* MyFrame::CreateTreeCtrl()
{
    wxTreeCtrl* tree = new wxTreeCtrl( this, wxID_ANY,
                                       wxPoint( 0, 0 ), wxSize( 160, 250 ),
                                       wxTR_DEFAULT_STYLE | wxNO_BORDER );

    wxImageList* imglist = new wxImageList( 16, 16, true, 2 );
    imglist->Add( wxArtProvider::GetBitmap( wxART_FOLDER, wxART_OTHER, wxSize( 16, 16 ) ) );
    imglist->Add( wxArtProvider::GetBitmap( wxART_NORMAL_FILE, wxART_OTHER, wxSize( 16, 16 ) ) );
    tree->AssignImageList( imglist );

    wxTreeItemId root = tree->AddRoot( wxT( "wxAUI Project" ), 0 );
    wxArrayTreeItemIds items;



    items.Add( tree->AppendItem( root, wxT( "Item 1" ), 0 ) );
    items.Add( tree->AppendItem( root, wxT( "Item 2" ), 0 ) );
    items.Add( tree->AppendItem( root, wxT( "Item 3" ), 0 ) );
    items.Add( tree->AppendItem( root, wxT( "Item 4" ), 0 ) );
    items.Add( tree->AppendItem( root, wxT( "Item 5" ), 0 ) );


    int i, count;
    for ( i = 0, count = items.Count(); i < count; ++i )
    {
        wxTreeItemId id = items.Item( i );
        tree->AppendItem( id, wxT( "Subitem 1" ), 1 );
        tree->AppendItem( id, wxT( "Subitem 2" ), 1 );
        tree->AppendItem( id, wxT( "Subitem 3" ), 1 );
        tree->AppendItem( id, wxT( "Subitem 4" ), 1 );
        tree->AppendItem( id, wxT( "Subitem 5" ), 1 );
    }


    tree->Expand( root );

    return tree;
}

wxSizeReportCtrl* MyFrame::CreateSizeReportCtrl( int width, int height )
{
    wxSizeReportCtrl* ctrl = new wxSizeReportCtrl( this, wxID_ANY,
            wxDefaultPosition,
            wxSize( width, height ), &m_mgr );
    return ctrl;
}

wxHtmlWindow* MyFrame::CreateHTMLCtrl( wxWindow* parent )
{
    if ( !parent )
        parent = this;

    wxHtmlWindow* ctrl = new wxHtmlWindow( parent, wxID_ANY,
                                           wxDefaultPosition,
                                           wxSize( 400, 300 ) );
    ctrl->SetPage( GetIntroText() );
    return ctrl;
}

wxAuiNotebook* MyFrame::CreateNotebook()
{
    // create the notebook off-window to avoid flicker
    wxSize client_size = GetClientSize();

    wxAuiNotebook* ctrl = new wxAuiNotebook( this, wxID_ANY,
            wxPoint( client_size.x, client_size.y ),
            wxSize( 430, 200 ),
            m_notebook_style );

    wxBitmap page_bmp = wxArtProvider::GetBitmap( wxART_NORMAL_FILE, wxART_OTHER, wxSize( 16, 16 ) );

    ctrl->AddPage( CreateHTMLCtrl( ctrl ), wxT( "Welcome to wxAUI" ) , false, page_bmp );

    wxPanel* panel = new wxPanel( ctrl, wxID_ANY );
    wxFlexGridSizer* flex = new wxFlexGridSizer( 2 );
    flex->AddGrowableRow( 0 );
    flex->AddGrowableRow( 3 );
    flex->AddGrowableCol( 1 );
    flex->Add( 5, 5 );   flex->Add( 5, 5 );
    flex->Add( new wxStaticText( panel, -1, wxT( "wxTextCtrl:" ) ), 0, wxALL | wxALIGN_CENTRE, 5 );
    flex->Add( new wxTextCtrl( panel, -1, wxT( "" ), wxDefaultPosition, wxSize( 100, -1 ) ),
               1, wxALL | wxALIGN_CENTRE, 5 );
    flex->Add( new wxStaticText( panel, -1, wxT( "wxSpinCtrl:" ) ), 0, wxALL | wxALIGN_CENTRE, 5 );
    flex->Add( new wxSpinCtrl( panel, -1, wxT( "5" ), wxDefaultPosition, wxSize( 100, -1 ),
                               wxSP_ARROW_KEYS, 5, 50, 5 ), 0, wxALL | wxALIGN_CENTRE, 5 );
    flex->Add( 5, 5 );   flex->Add( 5, 5 );
    panel->SetSizer( flex );
    ctrl->AddPage( panel, wxT( "wxPanel" ), false, page_bmp );


    ctrl->AddPage( new wxTextCtrl( ctrl, wxID_ANY, wxT( "Some text" ),
                                   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER ) , wxT( "wxTextCtrl 1" ), false, page_bmp );

    ctrl->AddPage( new wxTextCtrl( ctrl, wxID_ANY, wxT( "Some more text" ),
                                   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER ) , wxT( "wxTextCtrl 2" ) );

    ctrl->AddPage( new wxTextCtrl( ctrl, wxID_ANY, wxT( "Some more text" ),
                                   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER ) , wxT( "wxTextCtrl 3" ) );

    ctrl->AddPage( new wxTextCtrl( ctrl, wxID_ANY, wxT( "Some more text" ),
                                   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER ) , wxT( "wxTextCtrl 4" ) );

    ctrl->AddPage( new wxTextCtrl( ctrl, wxID_ANY, wxT( "Some more text" ),
                                   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER ) , wxT( "wxTextCtrl 5" ) );

    ctrl->AddPage( new wxTextCtrl( ctrl, wxID_ANY, wxT( "Some more text" ),
                                   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER ) , wxT( "wxTextCtrl 6" ) );

    ctrl->AddPage( new wxTextCtrl( ctrl, wxID_ANY, wxT( "Some more text" ),
                                   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER ) , wxT( "wxTextCtrl 7 (longer title)" ) );

    ctrl->AddPage( new wxTextCtrl( ctrl, wxID_ANY, wxT( "Some more text" ),
                                   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER ) , wxT( "wxTextCtrl 8" ) );

    return ctrl;
}

wxString MyFrame::GetIntroText()
{
    const char* text =
        "<html><body>"
        "<h3>Welcome to wxAUI</h3>"
        "<br/><b>Overview</b><br/>"
        "<p>wxAUI is an Advanced User Interface library for the wxWidgets toolkit "
        "that allows developers to create high-quality, cross-platform user "
        "interfaces quickly and easily.</p>"
        "<p><b>Features</b></p>"
        "<p>With wxAUI, developers can create application frameworks with:</p>"
        "<ul>"
        "<li>Native, dockable floating frames</li>"
        "<li>Perspective saving and loading</li>"
        "<li>Native toolbars incorporating real-time, &quot;spring-loaded&quot; dragging</li>"
        "<li>Customizable floating/docking behavior</li>"
        "<li>Completely customizable look-and-feel</li>"
        "<li>Optional transparent window effects (while dragging or docking)</li>"
        "<li>Splittable notebook control</li>"
        "</ul>"
        "<p><b>What's new in 0.9.3?</b></p>"
        "<p>wxAUI 0.9.3, which is now bundled with wxWidgets, adds the following features:"
        "<ul>"
        "<li>New wxAuiNotebook class, a dynamic splittable notebook control</li>"
        "<li>New wxAuiMDI* classes, a tab-based MDI and drop-in replacement for classic MDI</li>"
        "<li>Maximize/Restore buttons implemented</li>"
        "<li>Better hinting with wxGTK</li>"
        "<li>Class rename.  'wxAui' is now the standard class prefix for all wxAUI classes</li>"
        "<li>Lots of bug fixes</li>"
        "</ul>"
        "<p><b>What's new in 0.9.2?</b></p>"
        "<p>The following features/fixes have been added since the last version of wxAUI:</p>"
        "<ul>"
        "<li>Support for wxMac</li>"
        "<li>Updates for wxWidgets 2.6.3</li>"
        "<li>Fix to pass more unused events through</li>"
        "<li>Fix to allow floating windows to receive idle events</li>"
        "<li>Fix for minimizing/maximizing problem with transparent hint pane</li>"
        "<li>Fix to not paint empty hint rectangles</li>"
        "<li>Fix for 64-bit compilation</li>"
        "</ul>"
        "<p><b>What changed in 0.9.1?</b></p>"
        "<p>The following features/fixes were added in wxAUI 0.9.1:</p>"
        "<ul>"
        "<li>Support for MDI frames</li>"
        "<li>Gradient captions option</li>"
        "<li>Active/Inactive panes option</li>"
        "<li>Fix for screen artifacts/paint problems</li>"
        "<li>Fix for hiding/showing floated window problem</li>"
        "<li>Fix for floating pane sizing problem</li>"
        "<li>Fix for drop position problem when dragging around center pane margins</li>"
        "<li>LF-only text file formatting for source code</li>"
        "</ul>"
        "<p>See README.txt for more information.</p>"
        "</body></html>";

    return wxString::FromAscii( text );
}

void MyFrame::OnCloseView( a2dCloseViewEvent& event )
{
    DrawingView* viewclosed = ( DrawingView* ) event.GetEventObject();

    if ( !event.GetVeto() && viewclosed )
    {
        a2dDocumentViewScrolledWindow* display = ( a2dDocumentViewScrolledWindow* ) viewclosed->GetDisplayWindow();

        unsigned int i;
        for ( i = 0 ; i < m_notebook->GetPageCount(); i++ )
        {
            if ( m_notebook->GetPage( i ) == display )
            {
                if ( display == m_canvas )
                    m_canvas = NULL;
                m_notebook->DeletePage( i );
                break;
            }
        }
    }
}


void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    bool oke = true;
    if ( event.CanVeto() )
    {
        unsigned int i;
        for ( i = 0 ; i < m_notebook->GetPageCount(); i++ )
        {
            oke = oke && ( ( a2dDocumentViewScrolledWindow* ) m_notebook->GetPage( m_notebook->GetSelection() ) )->GetView()->Close( !event.CanVeto() );

            if ( !oke )
                break;
        }

        if ( oke )
        {
            Destroy();
        }
        else
            event.Veto( true );
    }
    else
        Destroy();
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "AuiNoteBook Frame sample\nAuthors: Malcolm Nealon Klaas Holwerda\n ( based on wxWidgets sample by  Benjamin I. Williams )" ), wxT( "About Aui Frame" ) );
}

void MyFrame::OnMRUFile( wxCommandEvent& event )
{
    event.Skip();
}


/*!
 * wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED event handler
 */

void MyFrame::OnAuinotebook1PageChanged( wxAuiNotebookEvent& event )
{
    /*
    a2dView* view = ((a2dDocumentViewScrolledWindow*)GetPage( event.GetSelection() ))->GetView();

        if ( view )
            view->Activate( true );

        event.Skip();
    */
}


// ----------------------------------------------------------------------------
// MyNotebook
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( MyNotebook, wxAuiNotebook )
    EVT_AUINOTEBOOK_PAGE_CHANGED( -1, MyNotebook::OnChangedPage )
END_EVENT_TABLE()

MyNotebook::MyNotebook( a2dDocumentFrame* parent )
    : wxAuiNotebook( parent, -1, wxDefaultPosition, wxDefaultSize )
{
}

void MyNotebook::OnChangedPage( wxAuiNotebookEvent& event )
{
    a2dView* view = ( ( a2dDocumentViewScrolledWindow* )GetPage( event.GetSelection() ) )->GetView();

    if ( view )
        view->Activate( true );

    event.Skip();
}
