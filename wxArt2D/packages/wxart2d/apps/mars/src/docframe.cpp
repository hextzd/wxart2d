/*! \file modules/editor/samples/wires/wires.cpp
   \author Erik van der Wal
   \brief First attempts for a wire demo, sorry q&d code at the moment.
    EW: code gets moreand more messy, definitely needs cleanup/documentation
   Copyright: 2004 (C) Erik van der Wal

   Licence: wxWidgets license

   RCS-ID: $Id: docframe.cpp,v 1.20 2009/09/26 19:01:02 titato Exp $
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
#include <wx/dialog.h>

#include <wx/editor/candocproc.h>
//#include <aggdrawer/aggdrawer.h>
#include <wx/canvas/layerinf.h>

// Include private headers
#include "docframe.h"
#include "elements.h"
#include "document.h"
#include "part.h"
#include "mastertool.h"

// Notebook stuff to be moved to wxArt2D library?

// ----------------------------------------------------------------------------
// wxDocNotebook
// ----------------------------------------------------------------------------

#include <wx/notebook.h>

//! Notebook widget, specially prepared to show several (views on) documents.
/*
    The notebook, has several tab pages. Each tab page can contain a view on a document.
    Do not set the window to the a2dDocumentFrame. Instead set the window to this notebook.
    This widget then takes care of setting the window for the a2dDocumentFrame whenever the user
    changes to another notebook tab.
*/

class wxDocNotebook : public wxNotebook
{
public:
    //! Constructor
    wxDocNotebook( wxWindow* parent );

protected:
    //! Event handler to set present document window
    void OnChangedPage( wxNotebookEvent& event );

    DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE( wxDocNotebook, wxNotebook )
    EVT_NOTEBOOK_PAGE_CHANGED( -1, wxDocNotebook::OnChangedPage )
END_EVENT_TABLE()


wxDocNotebook::wxDocNotebook( wxWindow* parent )
    : wxNotebook( parent, -1, wxDefaultPosition, wxDefaultSize )
{
}

void wxDocNotebook::OnChangedPage( wxNotebookEvent& event )
{
    a2dDocumentViewWindow* win = ( ( a2dDocumentViewWindow* )GetPage( event.GetSelection() ) );
    a2dView* view = win->GetView();
    if ( view )
        view->Activate( true );
    event.Skip();
}


//------------------------------------------------------------------------------
// msDocFrame
//------------------------------------------------------------------------------
extern const long ID_QUIT = wxNewId();
extern const long ID_TOOLBAR = wxNewId();
extern const long ID_LISTBOX = wxNewId();

extern const long ID_MENUFIRST = wxNewId();
extern const long ID_MASTERTOOL = wxNewId();
extern const long ID_EDITTOOL = wxNewId();
extern const long ID_SELECTTOOL = wxNewId();
extern const long ID_DELETETOOL = wxNewId();
extern const long ID_COPYTOOL = wxNewId();
extern const long ID_WIRETOOL = wxNewId();
extern const long ID_ZOOMOUT2X = wxNewId();
extern const long ID_ZOOMIN2X = wxNewId();
extern const long ID_ZOOMWIN = wxNewId();
extern const long ID_ZOOMALL = wxNewId();
extern const long ID_TEXTEDIT = wxNewId();
extern const long ID_POLYTOOL = wxNewId();
extern const long ID_DUMP = wxNewId();
extern const long ID_ABOUTMARS = wxNewId();
extern const long ID_SPICE = wxNewId();
extern const long ID_FREEDA = wxNewId();
extern const long ID_INSERTELEMENT = wxNewId();
extern const long ID_MENULAST = wxNewId();
extern const long wires_ABOUT = wxNewId();

BEGIN_EVENT_TABLE( msDocFrame, a2dDocumentFrame )
    EVT_MENU( ID_QUIT, msDocFrame::OnQuit )
    EVT_MENU_RANGE( ID_MENUFIRST, ID_MENULAST, msDocFrame::OnEdittool )
    EVT_CLOSE( msDocFrame::OnCloseWindow )
    EVT_MENUSTRINGS( msDocFrame::OnSetMenuStrings )
    EVT_MENU( wxID_UNDO, msDocFrame::OnUndo )
    EVT_MENU( wxID_REDO, msDocFrame::OnRedo )

    // EVT_LISTBOX( ID_LISTBOX, msDocFrame::OnListBox )
    EVT_MENU_RANGE( wxID_FILE1, wxID_FILE9, msDocFrame::OnMRUFile )
END_EVENT_TABLE()

bool msDocFrame::InitToolbar( wxToolBar* toolBar )
{
    toolBar->SetToolBitmapSize( wxSize( 20, 20 ) );
    toolBar->AddTool( wxID_HELP, wxT( "Zoom Win" ), wxBitmap( wxImage( wxT( "bitmaps/Magnify.gif" ) ) ), wxT( "Zoom Window" ) );
    toolBar->AddTool( ID_ZOOMIN2X, wxT( "Zoom In" ), wxBitmap( wxImage( wxT( "bitmaps/MagnifyPlus.gif" ) ) ), wxT( "Zoom in 2x" ) );
    toolBar->AddTool( ID_ZOOMOUT2X, wxT( "Zoom Out" ), wxBitmap( wxImage( wxT( "bitmaps/MagnifyMinus.gif" ) ) ), wxT( "Zoom out 2x" ) );
    toolBar->AddTool( ID_ZOOMALL, wxT( "Zoom All" ), wxBitmap( wxImage( wxT( "bitmaps/PageSetup.gif" ) ) ), wxT( "Zoom All" ) );
    toolBar->AddSeparator();
    toolBar->AddTool( ID_COPYTOOL, wxT( "Copy" ), wxBitmap( wxImage( wxT( "bitmaps/Copy.gif" ) ) ), wxT( "Copy" ) );
    toolBar->AddTool( ID_DELETETOOL, wxT( "Cut" ), wxBitmap( wxImage( wxT( "bitmaps/Cut.gif" ) ) ), wxT( "Cut" ) );
    toolBar->AddTool( wxID_HELP, wxT( "Paste" ), wxBitmap( wxImage( wxT( "bitmaps/Paste.gif" ) ) ), wxT( "Paste" ) );
    toolBar->AddSeparator();
    toolBar->AddTool( ID_SELECTTOOL, wxT( "Select Tool" ), wxBitmap( wxImage( wxT( "bitmaps/Select.gif" ) ) ), wxT( "Select Tool" ) );
    toolBar->AddTool( ID_WIRETOOL, wxT( "Wire Tool" ), wxBitmap( wxImage( wxT( "bitmaps/Draw.gif" ) ) ), wxT( "Wire Tool" ) );
    toolBar->AddSeparator();
    toolBar->AddTool( ID_TEXTEDIT, wxT( "Text Tool" ), wxBitmap( wxImage( wxT( "bitmaps/Bold.gif" ) ) ), wxT( "Text Tool" ) );
    toolBar->Realize();


    return true;
}

msDocFrame::msDocFrame( a2dDocumentTemplate* docTemplate, wxFrame* parent, wxWindowID id, const wxString& title,
                        const wxPoint& position, const wxSize& size, long style ) :
    a2dDocumentFrame( false, parent, NULL, id, title, position, size, style )
{
    CreateMyMenuBar();

    CreateStatusBar( 2 );
    SetStatusText( _T( "Welcome to MARS!" ) );

    SetIcon( wxICON( mondrian ) );

    // Create the toolbar
    CreateToolBar( wxNO_BORDER | wxHORIZONTAL | wxTB_FLAT, ID_TOOLBAR );
    GetToolBar()->SetMargins( 2, 2 );
    InitToolbar( GetToolBar() );

//    wxSplitterWindow* splitwin = new wxSplitterWindow( this, -1, wxDefaultPosition, wxDefaultSize, wxSP_NOBORDER );
//    wxSplitterWindow* splitwin2 = new wxSplitterWindow( splitwin, -1, wxDefaultPosition, wxDefaultSize, wxSP_NOBORDER );

    /*    wxListBox* tree = new wxListBox( splitwin2, ID_LISTBOX );
        tree->Append( wxT("Ground") );
        tree->Append( wxT("Input") );
        tree->Append( wxT("Output") );
        tree->Append( wxT("Resistor") );
        tree->Append( wxT("Capacitor") );
        tree->Append( wxT("Rectangle") );
        tree->Append( wxT("Circle") );
        tree->Append( wxT("Ellipse") );
        tree->Append( wxT("CanvasText") );
        tree->Append( wxT("Text") );
    */
    a2dCanvasGlobals->GetHabitat()->SetReverseOrder( true );
    /*
        m_preview = new a2dCanvas( splitwin2, -1, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL | wxSUNKEN_BORDER | wxALWAYS_SHOW_SB );
        m_preview->SetGrid( true );
        m_preview->SetGridX( 4.0 );
        m_preview->SetGridY( 4.0 );
        m_preview->SetGridLines( true );
        m_preview->SetGridStroke( a2dStroke( wxColour( 0xf0, 0xf0, 0xf0), 1 ) );
        m_preview->SetYaxis( true );

        a2dLayers* docLayers = m_preview->GetCanvasDocument()->GetLayerSetup();
        docLayers->Set_Stroke( 0, a2dStroke(*wxBLACK, 0.5) );
        docLayers->Set_Fill( 0, a2dFill( wxColour(0xff, 0xff, 0xc0) ) );

    */
    //! canvas window to display the view upon to be defined by user
    m_notebook = new wxDocNotebook( this );

//    splitwin2->SplitHorizontally( m_preview, tree, 150);
//splitwin->SplitVertically( splitwin2, m_notebook, 150);

}

msDocFrame::~msDocFrame()
{
}

void msDocFrame::CreateMyMenuBar()
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
    file_menu->Append( ID_QUIT, _T( "Quit..." ), _T( "Quit program" ) );

    m_edit_menu = new wxMenu;
    m_edit_menu->Append( wxID_UNDO, _( "&Undo" ) );
    m_edit_menu->Append( wxID_REDO, _( "&Redo" ) );

    wxMenu* tool_menu = new wxMenu;
    tool_menu->Append( ID_MASTERTOOL, _T( "Mastertool" ), _T( "Mastertool" ) );
    tool_menu->Append( ID_EDITTOOL, _T( "Edittool" ), _T( "Edittool" ) );
    tool_menu->Append( ID_SELECTTOOL, _T( "Selecttool" ), _T( "Selecttool" ) );
    tool_menu->Append( ID_DELETETOOL, _T( "Deletetool" ), _T( "Deletetool" ) );
    tool_menu->Append( ID_COPYTOOL, _T( "Copytool" ), _T( "Copytool" ) );
    tool_menu->Append( ID_WIRETOOL, _T( "Wiretool" ), _T( "Wiretool" ) );
    tool_menu->Append( ID_DUMP, _T( "Dump doc" ), _T( "Dump" ) );
    tool_menu->Append( ID_POLYTOOL, _T( "PolyLine Tool" ), _T( "Text Tool" ) );

    wxMenu* insert_menu = new wxMenu;
    insert_menu->Append( ID_INSERTELEMENT, _T( "Insert Element" ), _T( "Insert Element" ) );

    wxMenu* netlist_menu = new wxMenu;
    netlist_menu->Append( ID_SPICE, _T( "Spice Netlist" ), _T( "Spice Netlist" ) );
    netlist_menu->Append( ID_FREEDA, _T( "fREEDA Netlist" ), _T( "fREEDA Netlist" ) );

    // the "About" item should be in the help menu
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append( ID_ABOUTMARS, _T( "&About...\tF1" ), _T( "Show about dialog" ) );

    wxMenuBar* menu_bar = new wxMenuBar();
    menu_bar->Append( file_menu, _T( "File" ) );
    menu_bar->Append( m_edit_menu, _( "&Edit" ) );
    menu_bar->Append( tool_menu, _T( "Tools" ) );
    menu_bar->Append( insert_menu, _T( "Insert" ) );
    menu_bar->Append( netlist_menu, _T( "Netlist" ) );
    menu_bar->Append( menuHelp, _T( "Help" ) );

    // A nice touch: a history of files visited. Use this menu.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu( file_menu );

    SetMenuBar( menu_bar );
}

void msDocFrame::OnMRUFile( wxCommandEvent& event )
{
    event.Skip();
    /*
    int n = event.GetId() - wxID_FILE1;  // the index in MRU list
    wxString filename(a2dDocviewGlobals->GetDocviewCommandProcessor()->GetHistoryFile(n));
    if ( !filename.IsEmpty() )
    {
        a2dDocument *doc = NULL;

        forEachIn(a2dDocumentList, a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments())
        {
            doc = (*iter).Get();

            if ( doc->GetFilename().GetFullPath() == filename )
                break;
        }

        if ( doc )
            ConnectDocument( (a2dCanvasDocument*) doc );
    }
    */
}

#if 0
void msDocFrame::ConnectDocument( a2dCanvasDocument* doc, a2dCanvasView* drawer )
{
    /*
    if ( !doc )
        return;


    a2dViewList allviews;
    doc->ReportViews( &allviews );

    if ( !drawer || wxNOT_FOUND == allviews.IndexOf( drawer ) )
    {
        if ( CANVAS->GetCanvasView() )  //document change means: this view will now belong to the new document
            CANVAS->GetCanvasView()->SetDocument( doc );
    }
    else
    {
        CANVAS->SetDrawer( drawer );
    }

    a2dView* view = m_window->GetView();

    if ( view)
    {
        view->SetDisplayWindow( NULL );
    }

    view->SetDisplayWindow( m_window );
    view->Update();
    m_window->SetView( view );

    if ( view->GetDocument() )
    {
        //maybe something to change the title
    }
    view->Activate( true );
    */
}
#endif

void msDocFrame::OnFileNew( wxCommandEvent& event )
{
    // first close old document
    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileClose();
    event.Skip();
}

void msDocFrame::OnFileClose( wxCommandEvent& event )
{
    event.Skip();
    return;
    // first create a NEW document
    // This calls void a2dConnector::OnPostCreateDocument( a2dTemplateEvent& event )
    a2dDocument* documentOld = GetView()->GetDocument();
    a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateDocuments( wxT( "dummy" ), a2dREFDOC_NEW );
    a2dCanvasDocument* document = ( a2dCanvasDocument* ) a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocuments().back().Get();

    //now close the old, which has the view removed already.
    a2dDocviewGlobals->GetDocviewCommandProcessor()->SetCurrentDocument( documentOld );
    event.Skip();
}

void msDocFrame::OnSetMenuStrings( a2dCommandProcessorEvent& event )
{
    if ( m_view && m_view->GetDocument() && event.GetEventObject() == m_view->GetDocument()->GetCommandProcessor() )
    {
        m_edit_menu->SetLabel( wxID_UNDO, event.GetUndoMenuLabel() );
        m_edit_menu->Enable( wxID_UNDO, event.CanUndo() );

        m_edit_menu->SetLabel( wxID_REDO, event.GetRedoMenuLabel() );
        m_edit_menu->Enable( wxID_REDO, event.CanRedo() );
    }
}

void msDocFrame::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( doc->GetCommandProcessor() )
        doc->GetCommandProcessor()->Undo();
}

void msDocFrame::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
    a2dDocument* doc = m_view->GetDocument();
    if ( doc->GetCommandProcessor() )
        doc->GetCommandProcessor()->Redo();
}

void msDocFrame::OnQuit( wxCommandEvent& event )
{
    Close( true );
}

// ----------------------------------------------------------------------------
// msAboutDialog
// ----------------------------------------------------------------------------

class msAboutDialog : public wxDialog
{
public:
    msAboutDialog( wxWindow* parent );

private:
    wxButton* m_btnModal,
              *m_btnModeless,
              *m_btnDelete;

};

msAboutDialog::msAboutDialog( wxWindow* parent )
    : wxDialog( parent, -1, wxString( _T( "Modal dialog" ) ) )
{
    wxBoxSizer* sizer1 = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer* sizer2 = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer* sizer3 = new wxBoxSizer( wxVERTICAL );
    sizer1->Add( sizer2, 0, wxALIGN_CENTER | wxALL, 5 );

    wxImage img( wxT( "marslogo128.png" ) );
    img.SetMaskColour( 0, 255, 0 );
    wxBitmap* bmp = new wxBitmap( img );
    wxStaticBitmap* sb = new wxStaticBitmap( this, -1, *bmp, wxDefaultPosition );
    sizer2->Add( sb, 0, wxALIGN_CENTER | wxALL, 5 );
    sizer2->Add( sizer3, 0, wxALIGN_CENTER | wxALL, 5 );

    wxStaticText* biglabel = new wxStaticText( this, -1 , wxT( "MARS" ) );
    biglabel->SetFont( wxFont( 36, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD ) );
    sizer3->Add( biglabel, 0, wxALIGN_CENTER | wxALL, 0 );
    sizer3->Add( new wxStaticText( this, -1 , wxT( "Microwave And Rf Simulator" ) ), 0, wxALIGN_CENTER | wxALL, 0 );


#if wxUSE_UNICODE
    wxString text = wxT( "(c) 2005 Erik van der Wal\n" );
#else
    wxString text = wxT(
                        "(c) 2005 Erik van der Wal\n"
                        "\n"
                        "Using:\n"
                        "- wxArt2D (c) Klaas Holwerda\n"
                        "  www.sourceforge.net/projects/wxart2d\n"
                        "- Icons (c) 1998 Dean S. Jones\n"
                        "  dean@gallant.com www.gallant.com/icons.htm\n"
                        "- Mars logo (c) 2005 Erik van der Wal\n"
                        "  Many thanks to Douwe Buurstra for 3D drawing" );
#endif
    sizer1->Add( new wxStaticText( this, -1 , text ), 0, wxALIGN_CENTER | wxALL, 5 );
    wxButton* btnOk = new wxButton( this, wxID_CANCEL, _T( "&Close" ) );
    sizer1->Add( btnOk, 0, wxALIGN_CENTER | wxALL, 5 );

    SetAutoLayout( TRUE );
    SetSizer( sizer1 );

    sizer1->SetSizeHints( this );
    sizer1->Fit( this );

    btnOk->SetFocus();
    btnOk->SetDefault();
}

void msDocFrame::OnEdittool( wxCommandEvent& event )
{
    a2dCanvasView* view = wxStaticCast( m_view.Get(), a2dCanvasView );

    a2dStToolContr* contr = wxDynamicCast( view->GetDrawingPart()->GetCanvasToolContr(), a2dStToolContr );
    if ( !contr )
        return;

    msDocument* doc = wxDynamicCast( view->GetDocument(), msDocument );

    if ( event.GetId() == ID_EDITTOOL )
        contr->PushTool( new a2dRecursiveEditTool( contr ) );
    else if ( event.GetId() == ID_SELECTTOOL )
        contr->PushTool( new a2dSelectTool( contr ) );
    else if ( event.GetId() == ID_DELETETOOL )
        contr->PushTool( new a2dDeleteTool( contr ) );
    else if ( event.GetId() == ID_COPYTOOL )
        contr->PushTool( new a2dCopyTool( contr ) );
    else if ( event.GetId() == ID_WIRETOOL )
    {
        a2dDrawWirePolylineLTool* tool = new a2dDrawWirePolylineLTool( contr );
        tool->SetStroke( a2dStroke( *wxGREEN, 3.0 ) );
        contr->PushTool( tool );
    }
    else if ( event.GetId() == ID_TEXTEDIT )
    {
    }
    else if ( event.GetId() == ID_MASTERTOOL )
        contr->PushTool( new msMasterTool( contr ) );
    else if ( event.GetId() == ID_POLYTOOL )
    {
        a2dDrawPolylineLTool* tool = new a2dDrawPolylineLTool( contr );
        tool->SetStroke( a2dStroke( *wxRED, 1.0 ) );
        contr->PushTool( tool );
    }
    else if ( event.GetId() == ID_ZOOMOUT2X )
        contr->Zoomout2();
    else if ( event.GetId() == ID_ZOOMIN2X )
        contr->Zoomin2();
    else if ( event.GetId() == ID_ZOOMALL )
        contr->Zoomout();
    else if ( event.GetId() == ID_DUMP )
    {
#if DEBUG
        m_view->GetShowObject()->Dump( 4 );
#endif
    }
    else if ( event.GetId() == ID_ABOUTMARS )
    {
        msAboutDialog about( NULL );
        about.ShowModal();
    }
    else if ( event.GetId() == ID_SPICE )
    {
        /*
                   {
                   wxString msg;
                   a2dCanvasDocument* candoc = m_view->GetCanvasDocument();
                   a2dCanvasObject* root = candoc->GetRootObject();
                   a2dCanvasObjectList* children = root->GetChildObjectList();
                   for ( size_t i =0; i < (size_t) children->size(); i++ )
                   {
                       Element* elm = wxDynamicCast( (*children)[i], Element );
                       if ( elm )
                       {
                           wxString line = elm->GetNetlistLine(wxT("SPICE") );
                           if ( !line.IsEmpty() )
                               msg += line + wxT("\n");
                       }
                   }
                   wxMessageBox( msg );
                   }
        */
    }
    else if ( event.GetId() == ID_FREEDA )
    {
        /*           {
                       // just a copy of spice above with one difference
                   wxString msg;
                   a2dCanvasDocument* candoc = m_view->GetCanvasDocument();
                   a2dCanvasObject* root = candoc->GetRootObject();
                   a2dCanvasObjectList* children = root->GetChildObjectList();
                   for ( size_t i =0; i < (size_t) children->size(); i++ )
                   {
                       Element* elm = wxDynamicCast( (*children)[i], Element );
                       if ( elm )
                       {
                           // the difference is in the next line
                           wxString line = elm->GetNetlistLine(wxT("FREEDA") );
                           if ( !line.IsEmpty() )
                               msg += line + wxT("\n");
                       }
                   }
                   wxMessageBox( msg );
                   }
           */
    }
    else if ( event.GetId() == ID_INSERTELEMENT )
    {
        wxString el = wxGetTextFromUser( wxT( "Element name (try: \"R\" or \"C\")" ) );
        msPart* part = doc->GetLocallibPage()->GetPart( el );
        if ( !part )
            wxMessageBox( wxT( "Could not find element" ) );
        else
        {
            //! \todo EW: Memory leak?
            msComponent* obj = new msComponent( part );
            doc->GetCircuitPage()->Append( obj );
            
            a2dDragNewTool* dragnew = new a2dDragNewTool( contr, obj );
            dragnew->SetStroke( a2dStroke(*wxRED, 1) );
            dragnew->SetFill( a2dFill( wxColour(0xc0, 0xff, 0xff) ) );
            contr->PushTool( dragnew );
            
        }
    }

}

void msDocFrame::OnListBox( wxCommandEvent& event )
{
    a2dCanvasView* view = wxStaticCast( m_view.Get(), a2dCanvasView );
    a2dStToolContr* contr = wxDynamicCast( view->GetDrawingPart()->GetCanvasToolContr(), a2dStToolContr );
    if ( !contr )
        return;

    wxListBox* lb = ( wxListBox* ) event.GetEventObject();
    wxString sel = lb->GetStringSelection();

    if ( sel.IsEmpty() )
        return;

	a2dCanvasObject* doc = m_preview->GetDrawing()->GetRootObject();
    a2dCanvasObject* obj = NULL;
    doc->ReleaseChildObjects();

    if ( wxDynamicCast( contr->GetFirstTool(), a2dDragNewTool ) )
    {
        a2dSmrtPtr< a2dBaseTool > tool;
        contr->PopTool( tool );
    }

    a2dCanvasObject* newobject = NULL;

    if ( sel == wxT( "Ground" ) )
    {
//       newobject = new Ground( );
    }
    if ( sel == wxT( "Input" ) )
    {
//       newobject = new Input( );
    }
    if ( sel == wxT( "Output" ) )
    {
//       newobject = new Output( );
    }
    else if ( sel == wxT( "Resistor" ) )
    {
//        msComponent *rescmp = CreateResistor();
//        Element* elm = new Element( rescmp );
//        delete rescmp;
//        elm->SetProperty( wxT("REFDES"), wxT("R8") );
        //elm->SetProperty( wxT("R"), wxT("10k") );
        //newobject = elm;
    }
    else if ( sel == wxT( "Capacitor" ) )
    {
//       newobject = new Capacitor( wxT("C9"), wxT("100p") );
    }
    else if ( sel == wxT( "Rectangle" ) )
    {
        //newobject = new a2dRect(0,0,10,10 );
    }
    else if ( sel == wxT( "Circle" ) )
    {
        //newobject = new a2dCircle(0,0,10 );
    }
    else if ( sel == wxT( "Ellipse" ) )
    {
        //newobject = new a2dEllipse(0,0,10,20 );
    }
    else if ( sel == wxT( "CanvasText" ) )
    {
        //newobject = new a2dText( wxT("editable text"), 0.0, 0.0, a2dFont( 3.0, wxFONTFAMILY_SWISS, wxITALIC ) );
    }
    else if ( sel == wxT( "Text" ) )
    {
        //newobject = new Text( wxT("whatever") );
    }
    //doc->GetRootObject()->Append( wxStaticCast( newobject->Clone(a2dObject::clone_deep), a2dCanvasObject ));
    doc->Append( newobject );
    a2dDragNewTool* dragnew = new a2dDragNewTool( contr, newobject );
    //optional
    // dragnew->SetOneShot();

    dragnew->SetStroke( a2dStroke( *wxRED, 1 ) );
    dragnew->SetFill( a2dFill( wxColour( 0xc0, 0xff, 0xff ) ) );
    contr->PushTool( dragnew );
    m_preview->SetMappingShowAll();
}

void msDocFrame::OnCloseWindow( wxCloseEvent& event )
{
    event.Skip(); //to base wxDocFrameScrolledWindow
}

void msDocFrame::CreateViews( msDocument* doc )
{
    doc->GetCircuitPage();
    doc->GetSymbolPage();
    doc->GetScionPage();
    doc->GetLocallibPage();

    a2dCanvasObjectList* list = doc->GetDrawing()->GetRootObject()->GetChildObjectList();
    forEachIn( a2dCanvasObjectList, list )
    {
        msPage* page = wxDynamicCast( ( *iter ).Get(), msPage );
        if ( page )
        {
            wxWindow* viewwin = page->CreateView( m_notebook, this, doc );
            if ( viewwin )
            {
                a2dDocumentViewWindow* win = ( a2dDocumentViewWindow* ) viewwin;
                m_notebook->AddPage( viewwin, page->GetName()  );
            }
        }
    }
}
