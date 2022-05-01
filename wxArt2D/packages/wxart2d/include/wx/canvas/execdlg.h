/*! \file wx/editor/execdlg.h
    \brief Definition of class for dialog to run/create a processfile, in which commands can be executed seperately.
    \author Probably Klaas Holwerda
    \date Created 05/07/03

    Copyright: (c)

    Licence: wxWidgets licence

    RCS-ID: $Id: execdlg.h,v 1.10 2009/09/26 19:01:05 titato Exp $
*/

#ifndef _WX_EXECDLGH__
#define _WX_EXECDLGH__

#include "wx/canvas/canmod.h"

//! GUI listbox with copy feature.
class A2DEDITORDLLEXP CopywxListBox: public wxListBox

{

public:

    CopywxListBox( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                   int n, const wxString choices[], long style );

    void OnMouseRightDown( class wxMouseEvent& );
    void CopySel( class wxCommandEvent& event );
    void CopyAll( class wxCommandEvent& event );

    wxMenu* m_mousemenu; // the floating menu

    // Declare used events.
    DECLARE_EVENT_TABLE()

};


////@begin control identifiers
#define IDEntry_DIALOG 10004
#define SYMBOL_A2DCOORDINATEENTRY_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_A2DCOORDINATEENTRY_TITLE _("SetCursor")
#define SYMBOL_A2DCOORDINATEENTRY_IDNAME IDEntry_DIALOG
#define SYMBOL_A2DCOORDINATEENTRY_SIZE wxSize(400, 300)
#define SYMBOL_A2DCOORDINATEENTRY_POSITION wxDefaultPosition
#define IDentry_X 10000
#define IDentry_Y 10001
#define IDentry_CalcCoord 10002
#define IDentry_CartPolar 10003
#define IDentry_AbsRel 10005
#define IDentry_Snap 10006
#define IDentry_Hide 10007
#define IDentry_Apply 10008
#define IDentry_Cancel 10009
////@end control identifiers

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
 * a2dCoordinateEntry class declaration
 */

class a2dCoordinateEntry: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( a2dCoordinateEntry )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    a2dCoordinateEntry( );
    a2dCoordinateEntry( wxWindow* parent, wxWindowID id = SYMBOL_A2DCOORDINATEENTRY_IDNAME, bool modal = false, const wxString& caption = SYMBOL_A2DCOORDINATEENTRY_TITLE, const wxPoint& pos = SYMBOL_A2DCOORDINATEENTRY_POSITION, const wxSize& size = SYMBOL_A2DCOORDINATEENTRY_SIZE, long style = SYMBOL_A2DCOORDINATEENTRY_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_A2DCOORDINATEENTRY_IDNAME, const wxString& caption = SYMBOL_A2DCOORDINATEENTRY_TITLE, const wxPoint& pos = SYMBOL_A2DCOORDINATEENTRY_POSITION, const wxSize& size = SYMBOL_A2DCOORDINATEENTRY_SIZE, long style = SYMBOL_A2DCOORDINATEENTRY_STYLE );

    /// Creates the controls and sizers
    void CreateControls( long style );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for IDentry_X
    void OnEntryXEnter( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for IDentry_Y
    void OnEntryYEnter( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for IDentry_CalcCoord
    void OnEntryCalccoordEnter( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for IDentry_CartPolar
    void OnEntryCartpolarSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for IDentry_AbsRel
    void OnEntryAbsrelSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for IDentry_Snap
    void OnEntrySnapClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for IDentry_Hide
    void OnEntryHideClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for IDentry_Apply
    void OnEntryApplyClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for IDentry_Cancel
    void OnEntryCancelClick( wxCommandEvent& event );

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// based on the current setting calculate the cursor position
    void CalcCoordinate();

    wxTextCtrl* m_x;
    wxTextCtrl* m_y;
    wxTextCtrl* m_calcCoord;
    wxRadioBox* m_cartPolar;
    wxRadioBox* m_absRel;
    wxCheckBox* m_snap;
    wxButton* m_hide;
    wxButton* m_apply;
    wxButton* m_cancel;

    wxStaticText* m_xText;
    wxStaticText* m_yText;

    double m_xcalc;
    double m_ycalc;

    bool m_modal;
};



#endif
