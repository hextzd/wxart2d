/*! \file wx/canvas/layerdlg.h
    \brief layer dialog for changing layer settings.
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: layerdlg.h,v 1.17 2008/10/30 23:18:09 titato Exp $
*/


#ifndef __LAYERDLG_H__
#define __LAYERDLG_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/canvas/strucdlg.h>

#include <wx/colordlg.h>
#include "wx/dataview.h"


//! Dialog for displaying/editing layers and properties of layers.
/*!
    This class is used to show a layerproperties dialog, in which the layernames, layernumber, visibility, select/read,
    fillcolour, border, pattern and mapping can be set.
*/
class A2DCANVASDLLEXP LayerPropertiesDialog : public wxDialog
{
public:

    // Constructor.
    LayerPropertiesDialog( wxFrame* parent, a2dLayers* layersetup, bool activeDrawingPart = false, const wxString& title = wxT( "LayerProperties" ),
                           long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxDIALOG_NO_PARENT ,
                           const wxString& name = _T( "Layer_Properties_Dialog" ) );

    // Destructor.
    ~LayerPropertiesDialog();

    // initialize the controls with the right values
    void FillControls();

    //! get the values from the controls and store them
    void StoreControls();

    // Initialize
    void Init( a2dLayers* layersetup );

    //close/hide window if HIDE-button is pressed.
    void CmHide( wxCommandEvent& );

    //! sort order
    void CmSort( wxCommandEvent& );

    // Close window if CANCEL-button is pressed.
    void CmCancel( wxCommandEvent& );

    // Restore original settings for active view when RESTORE-button is pressed.
    void CmRestore( wxCommandEvent& );

    //! save layers settings in dialog to file
    void CmSaveToFile( wxCommandEvent& WXUNUSED( event ) );

    //! load layers settings in dialog from file
    void CmLoadFromFile( wxCommandEvent& WXUNUSED( event ) );

    void CmAddLayer( wxCommandEvent& WXUNUSED( event ) );

    void CmVisible( wxCommandEvent& );

    // Set read properties.
    void CmRead( wxCommandEvent& );

    // Set outline properties.
    void CmOutLine( wxCommandEvent& );

    // Select layer.
    void CmSelect( wxCommandEvent& );

    // pointer to the layer setup of the active view.
    a2dSmrtPtr<a2dLayers> m_targetLayerSetup;

    // pointer to the cloned layer setup in order to restore when canceling.
    a2dSmrtPtr<a2dLayers> m_backuplayersetup;

    wxDataViewListCtrl* m_lc;

    wxButton*   m_buttonSORT;

    // wxWindows class.
    wxButton*   m_buttonHIDE;

    // wxWindows class.
    wxButton*   m_buttonCANCEL;

    // wxWindows class.
    wxButton*   m_buttonVISIBLE;

    // wxWindows class.
    wxButton*   m_buttonREAD;

    wxButton*   m_buttonOutLine;

    // wxWindows class.
    wxButton*   m_buttonSELECT;

    // wxWindows class.
    wxButton*   m_buttonRESTORE;

    wxButton*   m_buttonSAVETOFILE;

    wxButton*   m_buttonLOADFROMFILE;

    wxButton*   m_buttonAddLayer;

protected:

    a2dLayerInfo* GetTargetLayerInfo( int i );

    void OnCloseWindow( wxCloseEvent& WXUNUSED( event ) );

    void OnComEvent( a2dComEvent& event );

    void OnDoEvent( a2dCommandProcessorEvent& event );

    void Cm_NumberBox( wxEvent& event );

    void Cm_VisibleBox( wxEvent& event );

    void Cm_SelectBox( wxEvent& event );

    void Cm_ReadBox( wxEvent& event );

    void Cm_OutLineBox( wxEvent& event );

    void Cm_LayerName( wxEvent& event );

    void Cm_edit( wxDataViewEvent& event );
    void Cm_changed( wxDataViewEvent& event );

    void Cm_SortColumn( wxDataViewEvent& event );

    void Cm_InMap( wxEvent& event );

    void Cm_OutMap( wxEvent& event );

    // Pointer to parent window.
    wxFrame*    m_parent;
 
    // wxWindows class, to help construct the colour dialog.
    wxColourData* m_cdata;

    // wxWindows class, the colour dialog.
    wxColourDialog* m_cd;

    FillPatterns* m_fillpatterns;

    wxDateTime m_dialogLastUpdate;

    bool m_onorder;

    a2dHabitat* m_habitat;

    bool m_activeDrawingPart;

    // Declare used events.
    DECLARE_EVENT_TABLE()


};

#endif

