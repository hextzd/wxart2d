/*! \file modules/editor/samples/wires/wires.h
    \author Erik van der Wal

    Copyright: 2004 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: docframe.h,v 1.8 2006/12/13 21:43:21 titato Exp $
*/

#ifndef __DOCFRAMEH__
#define __DOCFRAMEH__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/canvas/canmod.h"
#include "wx/timer.h"
#include "wx/toolbar.h"
#include <wx/canvas/sttool.h>

class wxDocNotebook;
class msDocument;

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

extern const long ID_QUIT;
extern const long ID_TOOLBAR;
extern const long ID_LISTBOX;

extern const long ID_MENUFIRST;
extern const long ID_MASTERTOOL;
extern const long ID_EDITTOOL;
extern const long ID_SELECTTOOL;
extern const long ID_DELETETOOL;
extern const long ID_COPYTOOL;
extern const long ID_WIRETOOL;
extern const long ID_ZOOMOUT2X;
extern const long ID_ZOOMIN2X;
extern const long ID_ZOOMWIN;
extern const long ID_ZOOMALL;
extern const long ID_TEXTEDIT;
extern const long ID_POLYTOOL;
extern const long ID_DUMP;
extern const long ID_ABOUTMARS;
extern const long ID_SPICE;
extern const long ID_FREEDA;
extern const long ID_INSERTELEMENT;
extern const long ID_MENULAST;
extern const long wires_ABOUT;

// WDR: class declarations

//----------------------------------------------------------------------------
// msDocFrame
//----------------------------------------------------------------------------

class msDocFrame: public a2dDocumentFrame
{

public:
    // constructors and destructors
    msDocFrame( a2dDocumentTemplate* docTemplate, wxFrame* parent, wxWindowID id, const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE );
    ~msDocFrame();

    void CreateViews( msDocument* doc );

    bool InitToolbar( wxToolBar* toolBar );
    void AddTool( wxToolBar* toolBar, int id, wxString label, wxString filename, wxString help );
    void FillDocument( a2dCanvasDocument* doc );
    void FillDocument2( a2dCanvasDocument* doc );

private:

    void OnFileNew( wxCommandEvent& event );
    void OnFileClose( wxCommandEvent& event );

    // WDR: method declarations for MyFrame
    void CreateMyMenuBar();

private:
    // WDR: member variable declarations for MyFrame
//    a2dStToolContr *m_contr;
    a2dCanvas* m_preview;
    wxDocNotebook* m_notebook;

private:

    void OnSetMenuStrings( a2dCommandProcessorEvent& event );
    void OnUndo( wxCommandEvent& event );
    void OnRedo( wxCommandEvent& event );

    // WDR: handler declarations for MyFrame
    void OnQuit( wxCommandEvent& event );
    void OnEdittool( wxCommandEvent& event );
    void OnCloseWindow( wxCloseEvent& event );
    void OnTimer( wxTimerEvent& event );
    void OnListBox( wxCommandEvent& event );
    void OnAbout( wxCommandEvent& event );
    void OnMRUFile( wxCommandEvent& event );

    wxMenu* m_edit_menu;

private:
    DECLARE_EVENT_TABLE()
};

#endif // __DOCFRAMEH__
