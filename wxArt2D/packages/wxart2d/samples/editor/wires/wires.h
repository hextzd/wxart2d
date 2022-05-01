/*! \file modules/editor/samples/wires/wires.h
    \author Erik van der Wal

    Copyright: 2004 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: wires.h,v 1.3 2008/11/06 19:51:03 titato Exp $
*/

#ifndef __WIRESH__
#define __WIRESH__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/canvas/canmod.h"
#include "wx/timer.h"
#include "wx/toolbar.h"
#include <wx/editor/editmod.h>

//! extend a2dWindowViewConnector  to set initial mapping in world coordinates for the view.
/*! In principle it sets the initial values for the a2dView.
    Next to that it swicthes the toolcontroller to this view
*/
class a2dConnector: public a2dViewConnector
{
    DECLARE_EVENT_TABLE()

public:

    a2dConnector();

    void SetDisplayWindow( a2dCanvas* display );

	a2dCanvas* m_canvas;

    void OnPostCreateView( a2dTemplateEvent& event );
    void OnPostCreateDocument( a2dTemplateEvent& event );
    void OnDisConnectView(  a2dTemplateEvent& event );
};

class a2dPreviewCanvas: public a2dCanvas
{
    DECLARE_EVENT_TABLE()

    DECLARE_DYNAMIC_CLASS( a2dPreviewCanvas )

public:

    a2dPreviewCanvas( wxWindow* parent = NULL, wxWindowID id = -1,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxScrolledWindowStyle );

    ~a2dPreviewCanvas();

    void OnMouseEvent( wxMouseEvent& event );
    void OnComEvent( a2dComEvent& event );
    void OnWheel( wxMouseEvent& event );

    a2dCanvasObjectPtr m_newObject;
};

class WiresDrawingPart: public a2dDrawingPartTiled
{
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS( WiresDrawingPart );

public:
    WiresDrawingPart( int width = 1000, int height = 1000 );

    WiresDrawingPart( int width, int height, a2dDrawer2D* drawer2D );

    WiresDrawingPart( a2dDrawer2D* drawer );

    WiresDrawingPart( const wxSize& size );

    WiresDrawingPart( const a2dCanvasView& src );

    WiresDrawingPart( const WiresDrawingPart& other );

	void SetDisplayWindow( wxWindow* display );

protected:

	void OnEnter( wxMouseEvent& event );
	void OnLeave( wxMouseEvent& event );

};

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

enum
{
    MDSCREEN_T  = wxID_HIGHEST + 2000,
    MDSCREEN_FIRST,
    MDSCREEN_E = MDSCREEN_FIRST,
    MDSCREEN_0,
    MDSCREEN_1,
    MDSCREEN_2,
    MDSCREEN_3,
    MDSCREEN_4,
    MDSCREEN_5,
    MDSCREEN_6,
    MDSCREEN_LAST = MDSCREEN_6,

    ID_QUIT,
    ID_WIREMODE,
    ID_TOOLBAR,
    ID_LISTBOX,

    ID_MENUFIRST,
    ID_MASTERTOOL,
    ID_EDITTOOL,
    ID_SELECTTOOL,
    ID_DELETETOOL,
    ID_COPYTOOL,
    ID_WIRETOOL,
    ID_ZOOMOUT2X,
    ID_ZOOMIN2X,
    ID_ZOOMWIN,
    ID_ZOOMALL,
    ID_TEXTEDIT,
    ID_POLYTOOL,
    ID_DUMP,
    ID_ABOUTWIRES,
    ID_SPICE,
    ID_FREEDA,
    ID_MENULAST,
    ID_DRAWER_CHANGE,
    lua_script,
    ID_testthis,
    MasterToolOption_0, 
    MasterToolOption_1, 
    MasterToolOption_2, 
    MasterToolOption_3, 
    MasterToolOption_4, 
    MasterToolOption_5, 
    wires_ABOUT
};

// WDR: class declarations

//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

class MyFrame: public a2dDocumentFrame
{
public:

    typedef void ( a2dEditorFrame::*wxObjectEventFunctionM )( wxCommandEvent& );

public:
    // constructors and destructors
    MyFrame( wxFrame* parent, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_FRAME_STYLE );
    ~MyFrame();

    void ConnectDocument( a2dCanvasDocument* doc, a2dCanvasView* drawer = NULL );

    bool InitToolbar( wxToolBar* toolBar );
    void AddTool( wxToolBar* toolBar, int id, wxString label, wxString filename, wxString help );
    void AddFunctionToMenu( int id, wxMenu* parentMenu, const wxString& text, const wxString& helpString, wxObjectEventFunctionM func, bool check = false );
    void FillDocument( a2dDrawing* doc );
    void FillDocument2( a2dDrawing* doc );
    void FillDocument3( a2dDrawing* doc );
    void FillDocument4( a2dDrawing* doc );
    void FillDocument5( a2dDrawing* doc );

    a2dCanvas* GetCanvas() { return m_canvas; }

	a2dCanvasObjectPtr m_pushNewObject;

private:

    void OnFileNew( wxCommandEvent& event );
    void OnFileClose( wxCommandEvent& event );

    void OnLineModeSwitch( wxCommandEvent& event );

    void OnSetMenuStrings( a2dCommandProcessorEvent& event );
    void OnUndo( wxCommandEvent& event );
    void OnRedo( wxCommandEvent& event );
    void OnActivateViewSentFromChild( a2dViewEvent& viewevent );

    void OnDrawer( wxCommandEvent& event );

    void OnMasterToolOption( wxCommandEvent& event );

    // WDR: method declarations for MyFrame
    void CreateMyMenuBar();

#if (wxART2D_USE_LUA == 1)
    void OnRunScript( wxCommandEvent& event );
#endif //(wxART2D_USE_LUA == 1)

private:
    // WDR: member variable declarations for MyFrame
    a2dStToolContr* m_contr;
    a2dPreviewCanvas* m_preview;
    a2dCanvas* m_canvas;
    wxMenuBar* m_menu_bar;
    wxMenu* m_editMenu;
    wxMenu* m_toolMenu;
    wxUint16 m_editMode; 

private:
    // WDR: handler declarations for MyFrame
    void OnQuit( wxCommandEvent& event );
    void OnMastertool( wxCommandEvent& event );
    void OnEdittool( wxCommandEvent& event );
    void OnCloseWindow( wxCloseEvent& event );
    void OnTimer( wxTimerEvent& event );
    void OnListBox( wxCommandEvent& event );
    void OnAbout( wxCommandEvent& event );
    void OnMRUFile( wxCommandEvent& event );
    void OnComEvent( a2dComEvent& event );

    void OnTest( wxCommandEvent& event );

    void OnCopyShape(wxCommandEvent& event);
    void OnPasteShape(wxCommandEvent& event);
    void OnUpdatePasteShape( wxUpdateUIEvent& event  );
    void OnUpdateCopyShape( wxUpdateUIEvent& event  );

    void OnDragSimulate(wxCommandEvent& event);


    void FillData( wxCommandEvent& event );

private:
    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
// MyApp
//----------------------------------------------------------------------------

class MyApp: public wxApp
{
public:
    MyApp();

    void OnAssertFailure(const wxChar *file,
                                       int line,
                                       const wxChar *func,
                                       const wxChar *cond,
                                       const wxChar *msg);

    virtual bool OnInit();
    virtual int OnExit();

    MyFrame* m_frame;
};

#endif // __WIRESH__
