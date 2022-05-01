/////////////////////////////////////////////////////////////////////////////
// Name:        a2dTransDlg.h
// Purpose:
// Author:      Klaas Holwerda
// Modified by:
// Created:     15/04/2008 10:44:52
// RCS-ID:
// Copyright:   Klaas Holwerda
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "wx/artbase/afmatrix.h"

#define ID_TRANSFORM 10008
#define SYMBOL_TRANSFORM_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_TRANSFORM_TITLE _("transform")
#define SYMBOL_TRANSFORM_IDNAME ID_TRANSFORM
#define SYMBOL_TRANSFORM_SIZE wxSize(400, 300)
#define SYMBOL_TRANSFORM_POSITION wxDefaultPosition


/*!
 * transform class declaration
 */

class a2dTransDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( a2dTransDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    a2dTransDlg();
    a2dTransDlg( wxWindow* parent, bool modal = false, wxWindowID id = SYMBOL_TRANSFORM_IDNAME, const wxString& caption = SYMBOL_TRANSFORM_TITLE, const wxPoint& pos = SYMBOL_TRANSFORM_POSITION, const wxSize& size = SYMBOL_TRANSFORM_SIZE, long style = SYMBOL_TRANSFORM_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_TRANSFORM_IDNAME, const wxString& caption = SYMBOL_TRANSFORM_TITLE, const wxPoint& pos = SYMBOL_TRANSFORM_POSITION, const wxSize& size = SYMBOL_TRANSFORM_SIZE, long style = SYMBOL_TRANSFORM_STYLE );

    /// Destructor
    ~a2dTransDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_hide
    void OnHideClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_apply
    void OnApplyClick( wxCommandEvent& event );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxTextCtrl* m_x;
    wxTextCtrl* m_y;
    wxTextCtrl* m_scalex;
    wxTextCtrl* m_scaley;
    wxCheckBox* m_vertical;
    wxCheckBox* m_horizontal;
    wxTextCtrl* m_rotateangle;
    wxRadioButton* m_clockwise;
    wxRadioButton* m_counterclockwise;
    wxButton* m_hide;
    wxButton* m_apply;

    a2dAffineMatrix m_lworld;

    bool m_modal;
};

#endif
// _TRANSFORM_H_
