/*! \file wx/editor/strucdlgdoc.h
    \brief for choosing a a2dCanvasObject from a list.
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: strucdlg.h,v 1.8 2009/10/05 20:03:11 titato Exp $
*/


#ifndef __STRUCDLGDOC_H__
#define __STRUCDLGDOC_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"
#include "wx/docview/doccom.h"
#include "wx/canvas/sttool.h"
#include "wx/editor/candoc.h"

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


//!Class a2dCanvasObjectsDocDialog.
//!This class is used to show a dialog containing objects.
class A2DEDITORDLLEXP a2dCanvasObjectsDocDialog: public wxDialog
{
    DECLARE_EVENT_TABLE()

public:

    a2dCanvasObjectsDocDialog( wxWindow* parent,
                            a2dCanvasDocument* document, bool structOnly = true, bool modal = false, long style = SYMBOL_A2DCANVASOBJECTSDIALOG_STYLE,
                            wxWindowID id = SYMBOL_A2DCANVASOBJECTSDIALOG_IDNAME, const wxString& caption = SYMBOL_A2DCANVASOBJECTSDIALOG_TITLE, const wxPoint& pos = SYMBOL_A2DCANVASOBJECTSDIALOG_POSITION, const wxSize& size = SYMBOL_A2DCANVASOBJECTSDIALOG_SIZE );

    //! constructor with a specific list of objects
    a2dCanvasObjectsDocDialog( wxWindow* parent, a2dCanvasObjectList* total, bool modal = false, long style = SYMBOL_A2DCANVASOBJECTSDIALOG_STYLE,
                            wxWindowID id = SYMBOL_A2DCANVASOBJECTSDIALOG_IDNAME, const wxString& caption = SYMBOL_A2DCANVASOBJECTSDIALOG_TITLE, const wxPoint& pos = SYMBOL_A2DCANVASOBJECTSDIALOG_POSITION, const wxSize& size = SYMBOL_A2DCANVASOBJECTSDIALOG_SIZE );

    ~a2dCanvasObjectsDocDialog();

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_A2DCANVASOBJECTSDIALOG_IDNAME, const wxString& caption = SYMBOL_A2DCANVASOBJECTSDIALOG_TITLE, const wxPoint& pos = SYMBOL_A2DCANVASOBJECTSDIALOG_POSITION, const wxSize& size = SYMBOL_A2DCANVASOBJECTSDIALOG_SIZE, long style = SYMBOL_A2DCANVASOBJECTSDIALOG_STYLE );

    //! Pointer to object selected.
    a2dCanvasObject*  GetCanvasObject();

    //! take over object to show from objects
    void Init( a2dCanvasObjectList* objects );

    void Init( a2dCanvasDocument* document );

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
    void OnChangedDocument( a2dCommandEvent& event );

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

    a2dCanvasDocument* m_document;

};



#endif

