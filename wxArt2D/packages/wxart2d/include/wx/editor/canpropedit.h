/*! \file wx/editor/canpropedit.h
    \brief for edting properties in a a2dNamedPropertyList

    The event for editing properties is intercepted, and the property list
    recieved that way can be edited.

    \author Klaas Holwerda
    \date Created 11/19/2003

    Copyright: 2003-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canpropedit.h,v 1.5 2006/12/13 21:43:29 titato Exp $
*/

#ifndef __WXCANPROPEDIT_H__
#define __WXCANPROPEDIT_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/general/gen.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/sttool.h"



#include "wx/grid.h"

/*!
 * Control identifiers
 */
#define ID_DIALOG 10000
#define SYMBOL_A2DEDITPROPERTIES_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_A2DEDITPROPERTIES_TITLE _("Edit Properties")
#define SYMBOL_A2DEDITPROPERTIES_IDNAME ID_DIALOG
#define SYMBOL_A2DEDITPROPERTIES_SIZE wxSize(400, 300)
#define SYMBOL_A2DEDITPROPERTIES_POSITION wxDefaultPosition
#define ID_GRID 10001
#define ID_ADD 10002
#define ID_CUT 10003
#define ID_PASTE 10004
#define ID_CANCEL 10006
#define ID_OKE 10005

enum PropertyType
{
    pt_string,
    pt_integer,
    pt_real,
    pt_bool
};

class PropGridData
{
public:
    PropGridData()
    {
        m_name = wxT( "" );
        m_type = pt_string;
        m_val_str = wxT( "" );
        m_val_integer = 0;
        m_val_real = 0.0;
        m_val_boolean = true;
        m_prop = NULL;
        m_remove = false;
    }

    wxString m_name;
    PropertyType m_type;
    wxString m_val_str;
    int m_val_integer;
    float m_val_real;
    bool m_val_boolean;
    a2dNamedPropertyPtr m_prop;
    bool m_remove;
};

typedef std::vector<PropGridData> PropGridDataVec;

/*!
 * a2dEditProperties class declaration
 */
class a2dEditProperties: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( a2dEditProperties )
    DECLARE_EVENT_TABLE()

    friend class PropertyTable;

public:
    /// Constructors
    a2dEditProperties( );
    a2dEditProperties( wxWindow* parent, a2dObject* propobject, a2dNamedPropertyList* propertylist, wxWindowID id = SYMBOL_A2DEDITPROPERTIES_IDNAME, const wxString& caption = SYMBOL_A2DEDITPROPERTIES_TITLE, const wxPoint& pos = SYMBOL_A2DEDITPROPERTIES_POSITION, const wxSize& size = SYMBOL_A2DEDITPROPERTIES_SIZE, long style = SYMBOL_A2DEDITPROPERTIES_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_A2DEDITPROPERTIES_IDNAME, const wxString& caption = SYMBOL_A2DEDITPROPERTIES_TITLE, const wxPoint& pos = SYMBOL_A2DEDITPROPERTIES_POSITION, const wxSize& size = SYMBOL_A2DEDITPROPERTIES_SIZE, long style = SYMBOL_A2DEDITPROPERTIES_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// Should we show tooltips?
    static bool ShowToolTips();

private:

    /// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_SIZE event handler for ID_DIALOG
    void OnSize( wxSizeEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ADD
    void OnAddClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CUT
    void OnCutClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PASTE
    void OnPasteClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_OKE
    void OnOkeClick( wxCommandEvent& event );

    //! properties defined on this object will be edited
    a2dObject* m_propobject;

    //! all or subset of properties on object m_propobject
    a2dNamedPropertyList* m_propertylist;

    PropGridDataVec m_propdata;

    wxGrid* m_grid;
    wxButton* m_add;
    wxButton* m_cut;
    wxButton* m_cancel;
    wxButton* m_oke;
};

//! edit properties of a2dCanvasObject's
/*!
*/
class A2DEDITORDLLEXP a2dPropertyEditorDlg: public wxDialog
{
public:

    //! constructor.
    a2dPropertyEditorDlg( wxFrame* parent, a2dNamedPropertyList* propertylist );

    //! destructor.
    ~a2dPropertyEditorDlg();

    //! Close window if OK-button is pressed.
    void CmOk( wxCommandEvent& );

    //! Close window if CANCEL-button is pressed.
    void CmCancel( wxCommandEvent& );

    //! Close window if EXIT \-button is pressed.
    void OnCloseWindow( wxCloseEvent& event );

protected:

    wxListBox* m_listbox;

    wxButton* m_button1;

    wxButton* m_button2;

    a2dNamedPropertyList* m_propertylist;

    //! Declare used events.
    DECLARE_EVENT_TABLE()
};


#endif

