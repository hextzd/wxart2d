/*! \file wx/canvas/sttoolmes.h
    \brief stack based tools for measuring

    Tools to measure things on the view.

    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: sttoolmes.h,v 1.1 2007/07/07 08:17:53 titato Exp $
*/

#ifndef __A2DMEASURE_H__
#define __A2DMEASURE_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/sttool.h"


////@begin control identifiers
#define SYMBOL_A2DMEASUREDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_A2DMEASUREDLG_TITLE _("Measure")
#define SYMBOL_A2DMEASUREDLG_SIZE wxSize(400, 300)
#define SYMBOL_A2DMEASUREDLG_POSITION wxDefaultPosition
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
 * a2dMeasureDlg class declaration
 */

class a2dMeasureDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( a2dMeasureDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    a2dMeasureDlg( );
    ~a2dMeasureDlg( );
    a2dMeasureDlg( a2dHabitat* habitat, wxWindow* parent, wxWindowID id = -1, const wxString& caption = SYMBOL_A2DMEASUREDLG_TITLE, const wxPoint& pos = SYMBOL_A2DMEASUREDLG_POSITION, const wxSize& size = SYMBOL_A2DMEASUREDLG_SIZE, long style = SYMBOL_A2DMEASUREDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = -1, const wxString& caption = SYMBOL_A2DMEASUREDLG_TITLE, const wxPoint& pos = SYMBOL_A2DMEASUREDLG_POSITION, const wxSize& size = SYMBOL_A2DMEASUREDLG_SIZE, long style = SYMBOL_A2DMEASUREDLG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin a2dMeasureDlg event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_IDLE event handler for ID_DIALOG
    void OnIdle( wxIdleEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_X
    void OnXEnter( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_X
    void OnXUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_Y
    void OnYEnter( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_Y
    void OnYUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX
    void OnListboxSelected( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_LISTBOX
    void OnListboxUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_HIDE
    void OnHideClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR
    void OnClearClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_SUM
    void OnSumUpdate( wxUpdateUIEvent& event );

////@end a2dMeasureDlg event handler declarations

////@begin a2dMeasureDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end a2dMeasureDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void AppendMeasure( double x, double y );

    void OnComEvent( a2dComEvent& event );
    void OnDoEvent( a2dCommandProcessorEvent& event );

////@begin a2dMeasureDlg member variables
    wxTextCtrl* m_x;
    wxTextCtrl* m_y;
    wxListBox* m_lb;
    wxButton* m_hide;
    wxButton* m_clear;
    wxTextCtrl* m_sum;

    a2dVertexList m_vertexes;

    a2dHabitat* m_habitat;

////@end a2dMeasureDlg member variables
};


//!Interactive Selection of an Object.
/*!Either with just one click or draging a rectangle to select many.

\remark  Left Click and/or drag.
\remark  Shift Down to Un select.

\sa  a2dStToolContr

*/
class A2DCANVASDLLEXP a2dMeasureTool: public a2dDrawPolylineLTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_Measure;

    a2dMeasureTool( a2dStToolContr* controller );
    ~a2dMeasureTool();

    bool ZoomSave() {return true;};

protected:

    virtual void GenerateAnotation();

    //! character handling
    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    virtual wxString GetCommandGroupName() { return _( "Measure Tool" ); }

public:

    DECLARE_CLASS( a2dMeasureTool )
    DECLARE_EVENT_TABLE()
};

#endif /* __A2DMEASURE_H__ */



