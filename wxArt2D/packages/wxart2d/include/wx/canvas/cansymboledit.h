/*! \file wx/canvas/cansymboledit.h
    \brief editor for symbol drawing

	a2dSymbol being edited as drawing.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: cansymboledit.h,v 1.17 2009/09/26 20:40:32 titato Exp $
*/

#ifndef __WXSYMBOLCAMELEON_H__
#define __WXSYMBOLCAMELEON_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/candefs.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/recur.h"
#include "wx/canvas/cameleon.h"

#include <wx/aui/framemanager.h>
#include "wx/aui/auibar.h"


class A2DCANVASDLLEXP a2dDrawingEditor;

//! used to theme a a2dEditorFrame
/*!
    The event Object is a a2dEditorFrame, and it can be intercepted to
    overrule the default theming ( menu's menubar etc. )

    \ingroup docview docviewevents  events
*/
class A2DCANVASDLLEXP a2dDrawingEditorFrameEvent: public wxEvent
{
public:
    a2dDrawingEditorFrameEvent( a2dDrawingEditor* editorFrame, wxEventType type, int id = 0 );

    a2dDrawingEditor* GetEditorFrame() const;

    wxEvent* Clone( void ) const;
};

BEGIN_DECLARE_EVENT_TYPES()
//! see a2dEditorFrameEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, a2dEVT_DRAWINGEDITOR_THEME_EVENT, 1 )
DECLARE_EXPORTED_EVENT_TYPE( A2DCANVASDLLEXP, a2dEVT_DRAWINGEDITOR_INIT_EVENT, 1 )
END_DECLARE_EVENT_TYPES()

typedef void ( wxEvtHandler::*a2dDrawingEditorFrameEventFunction ) ( a2dDrawingEditorFrameEvent& );

#define a2dDrawingEditorFrameEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(a2dDrawingEditorFrameEventFunction, &func)

/*! \addtogroup eventhandlers
*  @{
*/

//! static wxEvtHandler for theme event, send from a2dEditorFrame
#define EVT_DRAWINGEDITOR_THEME_EVENT(func) wx__DECLARE_EVT0( a2dEVT_DRAWINGEDITOR_THEME_EVENT, a2dDrawingEditorFrameEventHandler( func ))
#define EVT_DRAWINGEDITOR_INIT_EVENT(func) wx__DECLARE_EVT0( a2dEVT_DRAWINGEDITOR_INIT_EVENT, a2dDrawingEditorFrameEventHandler( func ))
//!  @} eventhandlers




/*!
 * a2dDrawingEditor class declaration
 */

const long SYMBOL_a2dDrawingEditorModal_IDNAME = wxNewId();
#define SYMBOL_a2dDrawingEditorModal_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_a2dDrawingEditorModal_TITLE _("Symbol editor")
#define SYMBOL_a2dDrawingEditorModal_SIZE wxSize(400, 300)
#define SYMBOL_a2dDrawingEditorModal_POSITION wxDefaultPosition

class a2dDrawingEditor: public wxFrame
{    
    wxDECLARE_EVENT_TABLE();

public:
    /// Constructors
    a2dDrawingEditor( a2dDrawing* drawing, wxWindow* parent, wxWindowID id = SYMBOL_a2dDrawingEditorModal_IDNAME, 
        const wxString& caption = SYMBOL_a2dDrawingEditorModal_TITLE, 
        const wxPoint& pos = SYMBOL_a2dDrawingEditorModal_POSITION, 
        const wxSize& size = SYMBOL_a2dDrawingEditorModal_SIZE, long style = SYMBOL_a2dDrawingEditorModal_STYLE );

    /// Destructor
    ~a2dDrawingEditor();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
	void CreateToolbars();

    void CreateThemeXRC();
    void CreateThemeDefault( bool modal = false );
 
	void OnCutShape(wxCommandEvent& event);
    void OnCopyShape(wxCommandEvent& event);
    void OnPasteShape(wxCommandEvent& event);

    void OnUpdatePasteShape( wxUpdateUIEvent& event  );
    void OnUpdateCopyShape( wxUpdateUIEvent& event  );

// other

    /// Returns the AUI manager object
    wxAuiManager& GetAuiManager() { return m_auiManager; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    //wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    //! return edit menu to set Undo and Redo commands
    wxMenu* GetEditMenu() { return m_editMenu; }

    wxAuiManager m_auiManager;
    wxMenu* m_submenuSettings;
    wxMenu* m_submenuImport;
    wxMenu* m_submenuExport;
    a2dDrawingId m_LastMenuDrawingId;

    wxStatusBar* m_StatusBar;

    a2dDrawingPart* m_drawingPart;

    //to detect change of drawing
    a2dDrawingPtr m_drawing;

	a2dCanvas* m_canvas;

    void ConnectCmdId( const a2dMenuIdItem& cmdId );

    //! add a menu to the parent menu, and connect it to the eventhandler of the frame
    /*!
    */
    void AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId );

    //! same as for a2dMenuIdItem, Id in wxMenuItem must be a valid id from one a2dMenuIdItem
    //! used in XRC files
    void AddCmdMenu( wxMenu* parentMenu, wxMenuItem* item );

    //! remove a command menu from the parent menu.
    /*!
        See AddCmdMenu()
    */
    void RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId );

    //! called for dynamic added menu's via AddCommandToMenu()
    //! command will be submitted to a2dDocviewGlobals->GetDocviewCommandProcessor()
    void OnExecuteCommand( wxCommandEvent& event );

    //! used by AddCmdMenu() to dynamically connect menu to function.
    //! The function just Skipps the event, so it will be handled by a command processor in the end.
    void OnCmdMenuId( wxCommandEvent& event );

    void OnTheme( a2dDrawingEditorFrameEvent& themeEvent );

    void OnInit( a2dDrawingEditorFrameEvent& initEvent );

protected:

    bool ProcessEvent( wxEvent& event );

    void AddCmdToToolbar( wxAuiToolBar* toolbar, const a2dMenuIdItem& cmdId );

    //! to hold undo redo menu's which are reinitialized at activation of view
	wxMenu* m_editMenu;

    wxMenu* m_menuFile;

    a2dCommandProcessor* m_UndoRedoCmd;

    void OnUndoEvent( a2dCommandProcessorEvent& event );

    void OnDoEvent( a2dCommandProcessorEvent& event );

    //!intercepted to initialize redo and undo menu's
    void OnActivate( wxActivateEvent& event );

    //! handler for event to set undo redo menustrings
    void OnSetmenuStrings( a2dCommandProcessorEvent& event );

    void OnUpdateUndo( wxUpdateUIEvent& event );

    void OnUpdateRedo( wxUpdateUIEvent& event );

    //! called on Close window
    void OnCloseWindow( wxCloseEvent& event );

    void OnComEvent( a2dComEvent& event );

    void OnUndo( wxCommandEvent& event );
    void OnRedo( wxCommandEvent& event );

	void OnMenuHighLight( wxMenuEvent& event );

    void OnQuit( wxCommandEvent& event );

    bool m_fixedToolStyle; 

    bool m_initialized;

    wxMenuBar* m_menubar;

};

class a2dDrawingEditorModal: public a2dDrawingEditor
{    
    wxDECLARE_EVENT_TABLE();

public:
    a2dDrawingEditorModal( a2dDrawing* drawing, wxWindow* parent, wxWindowID id = SYMBOL_a2dDrawingEditorModal_IDNAME, const wxString& caption = SYMBOL_a2dDrawingEditorModal_TITLE, const wxPoint& pos = SYMBOL_a2dDrawingEditorModal_POSITION, const wxSize& size = SYMBOL_a2dDrawingEditorModal_SIZE, long style = SYMBOL_a2dDrawingEditorModal_STYLE );

    // Destructor
    ~a2dDrawingEditorModal();

    // Initialises member variables
    void Init();
	bool Show(bool show);
    bool IsModal() const;
    int ShowModal();

    void EndModal(int retCode);
    void SetReturnCode(int retCode);
    int GetReturnCode() const;


protected:

    void CreateControls();

    void CreateMenubar();

    void CreateToolbars();

    //! called on Close window
    void OnCloseWindow( wxCloseEvent& event );

    void Oke( wxCommandEvent& event );
    void Cancel( wxCommandEvent& event );

    
    // while we are showing a modal window we disable the other windows using
    // this object
    wxWindowDisabler *m_windowDisabler;

    // modal window runs its own event loop
    wxEventLoop *m_eventLoop;

    // is modal right now?
    bool m_isShowingModal;

    //The return code of a modal window
    int m_returnCode;                                    
};


#define ID_DIALOG 10000
#define SYMBOL_a2dDrawingEditor_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_a2dDrawingEditor_TITLE _("Symbol editor")
#define SYMBOL_a2dDrawingEditor_IDNAME ID_DIALOG
#define SYMBOL_a2dDrawingEditor_SIZE wxSize(400, 300)
#define SYMBOL_a2dDrawingEditor_POSITION wxDefaultPosition
#define IDSE_PANEL 10004
#define IDSE_CANVAS 10001
#define IDSE_BUTTON 10002
#define IDSE_BUTTON1 10003

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * a2dDrawingEditor class declaration
 */

class a2dDrawingEditorDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( a2dDrawingEditorDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    a2dDrawingEditorDlg( );
    a2dDrawingEditorDlg( a2dDrawing* drawing, wxWindow* parent, wxWindowID id = SYMBOL_a2dDrawingEditor_IDNAME, const wxString& caption = SYMBOL_a2dDrawingEditor_TITLE, const wxPoint& pos = SYMBOL_a2dDrawingEditor_POSITION, const wxSize& size = SYMBOL_a2dDrawingEditor_SIZE, long style = SYMBOL_a2dDrawingEditor_STYLE );

	~a2dDrawingEditorDlg();

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_a2dDrawingEditor_IDNAME, const wxString& caption = SYMBOL_a2dDrawingEditor_TITLE, const wxPoint& pos = SYMBOL_a2dDrawingEditor_POSITION, const wxSize& size = SYMBOL_a2dDrawingEditor_SIZE, long style = SYMBOL_a2dDrawingEditor_STYLE );

    /// Creates the controls and sizers
    void CreateControls();
	void CreateToolbars();

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for IDSE_BUTTON
    void OnClickOke( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for IDSE_BUTTON1
    void OnClickCancel( wxCommandEvent& event );

	wxBitmap GetBitmapResource( const wxString& name );

    wxAuiManager& GetAuiManager() { return m_auiManager; }

    wxAuiManager m_auiManager;

    wxButton* m_oke;
    wxButton* m_cancel;

    wxStatusBar* m_StatusBar;

    a2dDrawingPart* m_drawingPart;

    //to detect change of drawing
    a2dDrawingPtr m_drawing;

    wxPanel* m_panel;
	a2dCanvas* m_canvas;

    void OnCmdMenuId( wxCommandEvent& event );

protected:

    void AddCmdToToolbar( wxAuiToolBar* toolbar, const a2dMenuIdItem& cmdId );
};



#endif
