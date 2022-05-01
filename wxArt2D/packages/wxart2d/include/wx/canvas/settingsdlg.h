/////////////////////////////////////////////////////////////////////////////
// Name:        settingsdlg.h
// Purpose:
// Author:      Klaas Holwerda
// Modified by:
// Created:     17/04/2008 14:15:57
// RCS-ID:
// Copyright:   Klaas Holwerda
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef _SETTINGSDLG_H_
#define _SETTINGSDLG_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/notebook.h"
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxNotebook;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_A2DSETTINGS 10009
#define SYMBOL_A2DSETTINGS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_A2DSETTINGS_TITLE _("Settings")
#define SYMBOL_A2DSETTINGS_IDNAME ID_A2DSETTINGS
#define SYMBOL_A2DSETTINGS_SIZE wxSize(400, 300)
#define SYMBOL_A2DSETTINGS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * a2dSettings class declaration
 */

class a2dSettings: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( a2dSettings )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    a2dSettings();
    a2dSettings( a2dHabitat* habitat, wxWindow* parent, wxWindowID id = SYMBOL_A2DSETTINGS_IDNAME, const wxString& caption = SYMBOL_A2DSETTINGS_TITLE, const wxPoint& pos = SYMBOL_A2DSETTINGS_POSITION, const wxSize& size = SYMBOL_A2DSETTINGS_SIZE, long style = SYMBOL_A2DSETTINGS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_A2DSETTINGS_IDNAME, const wxString& caption = SYMBOL_A2DSETTINGS_TITLE, const wxPoint& pos = SYMBOL_A2DSETTINGS_POSITION, const wxSize& size = SYMBOL_A2DSETTINGS_SIZE, long style = SYMBOL_A2DSETTINGS_STYLE );

    /// Destructor
    ~a2dSettings();

    /// Initialises member variables
    void Init();

    /// Show and intialize
    bool Show( bool show = true );

    void UpdateEntries();

    /// Creates the controls and sizers
    void CreateControls();

////@begin a2dSettings event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_A2DSETTINGS
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_LEFT_DOWN event handler for ID_BUTTON1
    void OnLeftDown( wxMouseEvent& event );

    //void OnChangedDocument( a2dCommandEvent& event );

    void OnUnitsSelected( wxCommandEvent& event );

////@end a2dSettings event handler declarations

////@begin a2dSettings member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end a2dSettings member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin a2dSettings member variables
    wxNotebook* m_settingsTabs;
    wxPanel* m_displaySet;
    wxChoice* m_units;
    wxArrayString m_unitsStrings;
    wxTextCtrl* m_displayAberation;
    wxTextCtrl* m_primThreshold;
    wxCheckBox* m_drawRectangle;
    wxTextCtrl* m_selectionDistance;
    wxPanel* m_booleanSet;
    wxTextCtrl* m_snapfactor;
    wxTextCtrl* m_offsetAber;
    wxTextCtrl* m_roundFactor;
    wxTextCtrl* m_offset;
    wxTextCtrl* m_smoothAber;
    wxTextCtrl* m_maxLineMerge;
    wxTextCtrl* m_intersectionGrid;
    wxCheckBox* m_fillRuleWinding;
    wxCheckBox* m_linkHoles;
    wxPanel* m_arcpolySet;
    wxTextCtrl* m_minRadius;
    wxTextCtrl* m_maxRadius;
    wxTextCtrl* m_poly2arcAber;
    wxTextCtrl* m_arc2polyAber;

    wxTextCtrl* m_gridDistX;
    wxTextCtrl* m_gridDistY;

    wxCheckBox* m_drawOutline;

    a2dHabitat* m_habitat;
////@end a2dSettings member variables
};


#define ID_NEWDOCUMENTSIZE 10000
#define SYMBOL_A2DNEWDOCUMENTSIZE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_A2DNEWDOCUMENTSIZE_TITLE _("New Document Size")
#define SYMBOL_A2DNEWDOCUMENTSIZE_IDNAME ID_NEWDOCUMENTSIZE
#define SYMBOL_A2DNEWDOCUMENTSIZE_SIZE wxSize(400, 300)
#define SYMBOL_A2DNEWDOCUMENTSIZE_POSITION wxDefaultPosition

/*!
 * a2dNewDocumentSize class declaration
 */
class a2dNewDocumentSize: public wxDialog
{
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    a2dNewDocumentSize();
    a2dNewDocumentSize( wxWindow* parent, wxWindowID id = SYMBOL_A2DNEWDOCUMENTSIZE_IDNAME, const wxString& caption = SYMBOL_A2DNEWDOCUMENTSIZE_TITLE, const wxPoint& pos = SYMBOL_A2DNEWDOCUMENTSIZE_POSITION, const wxSize& size = SYMBOL_A2DNEWDOCUMENTSIZE_SIZE, long style = SYMBOL_A2DNEWDOCUMENTSIZE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_A2DNEWDOCUMENTSIZE_IDNAME, const wxString& caption = SYMBOL_A2DNEWDOCUMENTSIZE_TITLE, const wxPoint& pos = SYMBOL_A2DNEWDOCUMENTSIZE_POSITION, const wxSize& size = SYMBOL_A2DNEWDOCUMENTSIZE_SIZE, long style = SYMBOL_A2DNEWDOCUMENTSIZE_STYLE );

    /// Destructor
    ~a2dNewDocumentSize();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_X
    void OnXEnter( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_Y
    void OnYEnter( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_UNITS
    void OnUnitsSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_oke
    void OnOkeClick( wxCommandEvent& event );

    double GetSizeX() const;

    double GetSizeY() const;

    wxString GetUnit() const;

    wxTextCtrl* m_x;
    wxTextCtrl* m_y;
    wxChoice* m_units;
    wxArrayString m_unitsStrings;
};


#endif
// _SETTINGSDLG_H_
