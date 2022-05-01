/*! \file wx/canvas/strucdlg.h
    \brief for choosing a a2dCanvasObject from a list.
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: strucdlg.h,v 1.8 2009/10/05 20:03:11 titato Exp $
*/


#ifndef __STRUCDLG_H__
#define __STRUCDLG_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/algos.h"
#include "wx/canvas/sttool.h"
class A2DCANVASDLLEXP FillPatterns;
#include "wx/general/genmod.h"

#define ID_DIALOG 10000
#define SYMBOL_A2DCANVASOBJECTSDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_A2DCANVASOBJECTSDIALOG_TITLE _("Choose Canvas Object")
#define SYMBOL_A2DCANVASOBJECTSDIALOG_IDNAME ID_DIALOG
#define SYMBOL_A2DCANVASOBJECTSDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_A2DCANVASOBJECTSDIALOG_POSITION wxDefaultPosition
#define STRUCT_ID_STATIC 10001
#define STRUCT_ID_LISTBOX 10002
#define STRUCT_ID_HIDE 10003
#define STRUCT_ID_APPLY 10004


//!Class a2dCanvasObjectsDialog.
//!This class is used to show a dialog containing objects.
class A2DCANVASDLLEXP a2dCanvasObjectsDialog: public wxDialog
{
    DECLARE_EVENT_TABLE()

public:

    a2dCanvasObjectsDialog( wxWindow* parent,
                            a2dDrawing* drawing, bool structOnly = true, bool modal = false, long style = SYMBOL_A2DCANVASOBJECTSDIALOG_STYLE,
                            wxWindowID id = SYMBOL_A2DCANVASOBJECTSDIALOG_IDNAME, const wxString& caption = SYMBOL_A2DCANVASOBJECTSDIALOG_TITLE, const wxPoint& pos = SYMBOL_A2DCANVASOBJECTSDIALOG_POSITION, const wxSize& size = SYMBOL_A2DCANVASOBJECTSDIALOG_SIZE );

    //! constructor with a specific list of objects
    a2dCanvasObjectsDialog( wxWindow* parent, a2dCanvasObjectList* total, bool modal = false, long style = SYMBOL_A2DCANVASOBJECTSDIALOG_STYLE,
                            wxWindowID id = SYMBOL_A2DCANVASOBJECTSDIALOG_IDNAME, const wxString& caption = SYMBOL_A2DCANVASOBJECTSDIALOG_TITLE, const wxPoint& pos = SYMBOL_A2DCANVASOBJECTSDIALOG_POSITION, const wxSize& size = SYMBOL_A2DCANVASOBJECTSDIALOG_SIZE );

    ~a2dCanvasObjectsDialog();

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_A2DCANVASOBJECTSDIALOG_IDNAME, const wxString& caption = SYMBOL_A2DCANVASOBJECTSDIALOG_TITLE, const wxPoint& pos = SYMBOL_A2DCANVASOBJECTSDIALOG_POSITION, const wxSize& size = SYMBOL_A2DCANVASOBJECTSDIALOG_SIZE, long style = SYMBOL_A2DCANVASOBJECTSDIALOG_STYLE );

    //! Pointer to object selected.
    a2dCanvasObject*  GetCanvasObject();

    //! take over object to show from objects
    void Init( a2dCanvasObjectList* objects );

    void Init( a2dDrawing* drawing );

    void InitCameleons( a2dDrawing* drawing, const wxString& appearanceClassName );

protected:

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for STRUCT_ID_LISTBOX
    void OnStructIdListboxSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for STRUCT_ID_LISTBOX
    void OnStructIdListboxDoubleClicked( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for STRUCT_ID_HIDE
    void OnStructIdHideClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for STRUCT_ID_APPLY
    void OnStructIdApplyClick( wxCommandEvent& event );

    /// Should we show tooltips?
    static bool ShowToolTips() { return true; }

    //! document changed
    //void OnChangedDocument( a2dCommandEvent& event );

    void OnComEvent( a2dComEvent& event );

    void OnDoEvent( a2dCommandProcessorEvent& event );

    void CmApply();

    bool m_modal;

    wxListBox* m_listbox;
    wxButton* m_hide;
    wxButton* m_Apply;

    wxStaticText* m_numberObjects;

    //! Pointer to object list
    a2dCanvasObjectsSet m_structureSet;

    //! Pointer to the choosen object.
    a2dCanvasObject*    m_object;

    a2dDrawing* m_drawing;

};

#define ID_DIALOGCHOOSE 10090
#define SYMBOL_A2DCHOOSE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_A2DCHOOSE_TITLE _("Choose Canvas Object")
#define SYMBOL_A2DCHOOSE_IDNAME ID_DIALOGCHOOSE
#define SYMBOL_A2DCHOOSE_SIZE wxSize(400, 800)
#define SYMBOL_A2DCHOOSE_POSITION wxDefaultPosition
#define CHOOSE_ID_STATIC ID_DIALOGCHOOSE + 1
#define CHOOSE_ID_LISTBOX ID_DIALOGCHOOSE + 2
#define CHOOSE_ID_HIDE ID_DIALOGCHOOSE + 3
#define CHOOSE_ID_APPLY ID_DIALOGCHOOSE + 4

//!Class a2dCanvasObjectsDialog.
//!This class is used to show a dialog containing objects.
class A2DCANVASDLLEXP a2dCanvasObjectsChooseDialog: public wxDialog
{
    DECLARE_EVENT_TABLE()

public:

    a2dCanvasObjectsChooseDialog( wxWindow* parent,
                            a2dDrawing* drawing, bool structOnly = true, long style = SYMBOL_A2DCHOOSE_STYLE,
                            wxWindowID id = SYMBOL_A2DCHOOSE_IDNAME, const wxString& caption = SYMBOL_A2DCHOOSE_TITLE, const wxPoint& pos = SYMBOL_A2DCHOOSE_POSITION, const wxSize& size = SYMBOL_A2DCANVASOBJECTSDIALOG_SIZE );

    a2dCanvasObjectsChooseDialog( wxWindow* parent, a2dCanvasObjectList* total, const wxString& appearancename, long style = SYMBOL_A2DCHOOSE_STYLE,
                            wxWindowID id = SYMBOL_A2DCHOOSE_IDNAME, const wxString& caption = SYMBOL_A2DCHOOSE_TITLE, const wxPoint& pos = SYMBOL_A2DCHOOSE_POSITION, const wxSize& size = SYMBOL_A2DCANVASOBJECTSDIALOG_SIZE );

    //! constructor with a specific list of objects
    a2dCanvasObjectsChooseDialog( wxWindow* parent, a2dCanvasObjectList* total, long style = SYMBOL_A2DCHOOSE_STYLE,
                            wxWindowID id = SYMBOL_A2DCHOOSE_IDNAME, const wxString& caption = SYMBOL_A2DCHOOSE_TITLE, const wxPoint& pos = SYMBOL_A2DCHOOSE_POSITION, const wxSize& size = SYMBOL_A2DCHOOSE_SIZE );

    ~a2dCanvasObjectsChooseDialog();

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_A2DCHOOSE_IDNAME, const wxString& caption = SYMBOL_A2DCHOOSE_TITLE, const wxPoint& pos = SYMBOL_A2DCHOOSE_POSITION, const wxSize& size = SYMBOL_A2DCHOOSE_SIZE, long style = SYMBOL_A2DCHOOSE_STYLE );

    //! Pointer to object selected.
    a2dCanvasObject*  GetCanvasObject();

    //! take over object to show from objects
    void Init( a2dCanvasObjectList* objects );

    void Init( a2dDrawing* drawing );

    void InitCameleons( a2dCanvasObjectList* total, const wxString& appearanceClassName );

protected:

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for CHOOSE_ID_LISTBOX
    void OnStructIdListboxSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for CHOOSE_ID_LISTBOX
    void OnStructIdListboxDoubleClicked( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for CHOOSE_ID_HIDE
    void OnStructIdHideClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for CHOOSE_ID_APPLY
    void OnStructIdApplyClick( wxCommandEvent& event );

    /// Should we show tooltips?
    static bool ShowToolTips() { return true; }

    //! document changed
    //void OnChangedDocument( a2dCommandEvent& event );

    void OnComEvent( a2dComEvent& event );

    void OnDoEvent( a2dCommandProcessorEvent& event );

    void CmApply();

    bool m_modal;

    wxListBox* m_listbox;
    wxButton* m_hide;
    wxButton* m_Apply;

    wxStaticText* m_numberObjects;

    //! Pointer to object list
    a2dCanvasObjectsSet m_structureSet;

    //! Pointer to the choosen object.
    a2dCanvasObject*    m_object;

    a2dDrawing* m_drawing;
};

//! to choose fill
class A2DCANVASDLLEXP CanvasFillDialog: public wxDialog
{
public:

    // constructor.
    CanvasFillDialog( wxFrame* parent );
    a2dFillStyle GetStyle() {return m_style;};
    void CmOk( wxCommandEvent& );
    void CmCancel( wxCommandEvent& );
    void OnCloseWindow( wxCloseEvent& event );

    DECLARE_EVENT_TABLE()

protected:

    wxChoice* m_choice;
    wxButton* m_button1;
    wxButton* m_button2;

    a2dFillStyle m_style;
};

//! pen dialog fro choosing a stroke.
class A2DCANVASDLLEXP PenDialog: public wxDialog
{
public:

    // constructor.
    PenDialog( wxFrame* parent );
    a2dStrokeStyle GetStyle() {return m_style;};
    void CmOk( wxCommandEvent& );
    void CmCancel( wxCommandEvent& );
    void OnCloseWindow( wxCloseEvent& event );

    DECLARE_EVENT_TABLE()

protected:

    wxChoice* m_choice;
    wxButton* m_button1;
    wxButton* m_button2;

    a2dStrokeStyle m_style;
};

//! to choose the way drawing is done ( not yet implemented ).
class A2DCANVASDLLEXP LogicalFunction: public wxDialog
{
public:

    // constructor.
    LogicalFunction( wxFrame* parent );
    int GetLogicalFunction() {return m_function;};
    void CmOk( wxCommandEvent& );
    void CmCancel( wxCommandEvent& );
    void OnCloseWindow( wxCloseEvent& event );

    DECLARE_EVENT_TABLE()

protected:

    wxChoice* m_choice;
    wxButton* m_button1;
    wxButton* m_button2;

    int m_function;
};

#define NR_PATTERNS 72

//! read a predefined set of fill patterns from files
//! and stores them in an array.
class A2DCANVASDLLEXP FillPatterns
{
public:
    FillPatterns();
    ~FillPatterns();

    wxBitmap*      GetPattern( short patternnr );
private:

    //! array for the different fill pitmaps.
    wxBitmap*       m_fillbitmaps[NR_PATTERNS];
};

//! choose a fill pattern
class A2DCANVASDLLEXP PatternDialog: public wxDialog
{

public:

    PatternDialog( wxWindow* parent, FillPatterns* fills, const wxString& title, long style = 0 , const wxString& name = wxT( "main_pattern_dlg" ) );

    ~PatternDialog();

    void OnActivate( wxActivateEvent& event );

protected:

    //! Close window if EXIT-button is pressed.
    void    OnCloseWindow( wxCloseEvent& event );

    //! Get chosen pattern.
    void    Cm_P_BitB( wxEvent& event );

    wxColour* ltowxc( long colour );

    wxPanel* m_panel1;

    wxScrolledWindow* m_scroll;

    //! Pointer to parent, holds the parent window.
    wxWindow* m_parent;

    //layer PATTERN  buttons.
    wxBitmapButton*    m_patternbut[NR_PATTERNS];

    //!the choosen pattern.
    int m_choosen;

    //! Declare used events.
    DECLARE_EVENT_TABLE()

};

#endif

