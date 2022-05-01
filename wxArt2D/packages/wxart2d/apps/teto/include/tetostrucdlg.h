/////////////////////////////////////////////////////////////////////////////
// Name:        strucdlg.h
// Author:      Klaas Holwerda
// Created:     XX/XX/XX
// Copyright:   2000 (c) Klaas Holwerda
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////


#ifndef __TETOSTRUCDLG_H__
#define __TETOSTRUCDLG_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"

//!
//!Class TetoObjectsDialog.
//!This class is used to show a dialog containing objects.
class TetoObjectsDialog: public wxDialog
{
public:

    //! constructor.
    TetoObjectsDialog( wxFrame* parent, a2dCanvasDocument* root, a2dCanvasObjectList* total, long style = ( wxCAPTION ) );

    //! destructor.
    ~TetoObjectsDialog();

    //! Pointer to object selected.
    a2dCanvasObject*  GetObject();

    //! Close window if OK-button is pressed.
    void CmOk( wxCommandEvent& );

    //! Close window if CANCEL-button is pressed.
    void CmCancel( wxCommandEvent& );

    //! Close window if EXIT \-button is pressed.
    void OnCloseWindow( wxCloseEvent& event );

protected:

    wxPanel* m_panel;

    wxStaticBox* m_gb;

    wxStaticText*   m_mess1;

    wxStaticText* m_mess2;

    wxListBox* m_listbox;

    wxButton* m_button1;

    wxButton* m_button2;

    wxFrame*  m_parent;

    //! Pointer to object list
    a2dCanvasObjectList*    m_structurelist;

    //! Pointer to the choosen object.
    a2dCanvasObject*    m_object;

    //! Declare used events.
    DECLARE_EVENT_TABLE()
};

#endif

