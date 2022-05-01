/////////////////////////////////////////////////////////////////////////////
// Name:        a2dPathSettings.h
// Purpose:
// Author:      Klaas Holwerda
// Modified by:
// Created:     10/04/2008 17:02:58
// RCS-ID:
// Copyright:   Klaas Holwerda
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef _a2dPathSettings_H_
#define _a2dPathSettings_H_

#include "wx/artbase/artmod.h"
#include "wx/canvas/canglob.h"

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_a2dPathSettings 10000
#define SYMBOL_a2dPathSettings_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_a2dPathSettings_TITLE _("Path Settings")
#define SYMBOL_a2dPathSettings_IDNAME ID_a2dPathSettings
#define SYMBOL_a2dPathSettings_SIZE wxSize(400, 300)
#define SYMBOL_a2dPathSettings_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * a2dPathSettings class declaration
 */

class a2dPathSettings: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( a2dPathSettings )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    a2dPathSettings();
    a2dPathSettings( a2dHabitat* habitat, wxWindow* parent, wxWindowID id = SYMBOL_a2dPathSettings_IDNAME, const wxString& caption = SYMBOL_a2dPathSettings_TITLE, const wxPoint& pos = SYMBOL_a2dPathSettings_POSITION, const wxSize& size = SYMBOL_a2dPathSettings_SIZE, long style = SYMBOL_a2dPathSettings_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_a2dPathSettings_IDNAME, const wxString& caption = SYMBOL_a2dPathSettings_TITLE, const wxPoint& pos = SYMBOL_a2dPathSettings_POSITION, const wxSize& size = SYMBOL_a2dPathSettings_SIZE, long style = SYMBOL_a2dPathSettings_STYLE );

    /// Destructor
    ~a2dPathSettings();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Should we show tooltips?
    static bool ShowToolTips();

private:

////@begin a2dPathSettings event handler declarations

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_WIDTH
    void OnWidthEnter( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_UNITS
    void OnUnitsSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_Rounded
    void OnRoundedSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_Rectangle
    void OnRectangleSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RectangleExt
    void OnRectangleExtSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_HIDE
    void OnHideClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_APPLY
    void OnApplyClick( wxCommandEvent& event );

    void OnComEvent( a2dComEvent& event );

    wxTextCtrl* m_width;
    wxChoice* m_units;
    wxRadioButton* m_rounded;
    wxRadioButton* m_rectangular;
    wxRadioButton* m_extRectangular;
    wxButton* m_hide;
    wxButton* m_apply;

    a2dPATH_END_TYPE m_pathtype;
    a2dDoMu m_contourWidth;
    wxArrayString m_unitsStrings;

    a2dHabitat* m_habitat;
};

#endif
// _a2dPathSettings_H_
