/*! \file wx/canvas/groupdlg.h
    \brief Definition of class for dialog to set groups for boolean operations.
    \author Klaas Holwerda
    \date Created 05/07/03

    Copyright: (c)

    Licence: wxWidgets licence

    RCS-ID: $Id: groupdlg.h,v 1.3 2008/10/30 23:18:09 titato Exp $
*/

#ifndef _WX_a2dGroupDlgH__
#define _WX_a2dGroupDlgH__

#include "wx/canvas/canmod.h"

//!
/*!
    This class is used to show an execution dialog, in which commands from history can be given to the commandline
    to be executed by pressing the EXECUTE-button.
*/
class A2DCANVASDLLEXP a2dGroupDlg: public wxDialog
{
public:

    // Constructor.
    a2dGroupDlg( a2dHabitat* habitat, wxFrame* parent, const wxString& title = wxT( "group settings" ),
                 long style = ( wxDEFAULT_DIALOG_STYLE ), const wxString& name = _T( "frame" ) );

    // Destructor.
    ~a2dGroupDlg();

    void Init();

    // Close window if EXIT-button is pressed.
    void OnCloseWindow( wxCloseEvent& event );

protected:

    // add item to group a.
    void CmAdd_A( wxCommandEvent& );

    // add item to group b.
    void CmAdd_B( wxCommandEvent& );

    // add item to destination.
    void CmAdd_D( wxCommandEvent& );

    // remove item from a.
    void CmRemove_A( wxCommandEvent& );

    // remove item from b.
    void CmRemove_B( wxCommandEvent& );

    // remove item from destination.
    void CmRemove_D( wxCommandEvent& );

    // remove all items from a.
    void CmClear_A( wxCommandEvent& );

    // remove all items from b.
    void CmClear_B( wxCommandEvent& );

    // Close window if OK-button is pressed.
    void CmOk( wxCommandEvent& );

    // Close window if CANCEL-button is pressed.
    void CmCancel( wxCommandEvent& );

    // wxWindows class, the main listbox control.
    wxListCtrl* m_listbox_main;

    // wxWindows class.
    wxCheckBox* m_onlyselect_A;

    // wxWindows class.
    wxCheckBox* m_onlyselect_B;

    // wxWindows class, the a listbox control.
    wxListCtrl* m_listbox_a;

    // wxWindows class, the b listbox control.
    wxListCtrl* m_listbox_b;

    wxTextCtrl* m_destination;

    int m_target;

    // wxWindows class.
    wxCheckBox* m_clearTarget;

    a2dSmrtPtr<a2dLayers> m_layersetup;

    a2dHabitat* m_habitat;

    // Declare used events.
    DECLARE_EVENT_TABLE()

};

#endif
