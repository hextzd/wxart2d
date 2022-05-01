/*! \file wx/canvas/orderdlg.h
    \brief Definition of class for dialog to specify the layerorder.
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: orderdlg.h,v 1.6 2009/10/01 19:22:35 titato Exp $
*/


//!
//!
/*
 * Definition of class for dialog to specify the layerorder.
*/

#ifndef __CANORDERDLG_H
#define __CANORDERDLG_H

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif



//! GUI listbox with copy feature.
class A2DCANVASDLLEXP a2dDragListBox: public wxListBox

{

public:

    a2dDragListBox( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                    int n, const wxString choices[], long style );

    void OnMouseLeftDown( class wxMouseEvent& );
    void OnMouseLeftUp( class wxMouseEvent& );
    void CmListBox( wxCommandEvent& );

    // Declare used events.
    DECLARE_EVENT_TABLE()

    int m_selection;

};

//! This class is used to show the layerorder dialog, in which the drawing order of the used layers
//! can be set.
class A2DCANVASDLLEXP a2dLayerDlg: public wxDialog
{
public:

    //! constructor.
    a2dLayerDlg( a2dHabitat* m_habitat, wxWindow* parent, a2dLayers* layersetup, bool onOrder, bool modal,
                    const wxString& title, const wxString& name = wxT( "layer_dlg" ) );

    //! destructor.
    ~a2dLayerDlg();

    //! initialize with this layer setup
    void Init( a2dLayers* layersetup );

    wxUint16 GetSelectedLayer() { return m_layerselected; }
    void SetSelectedLayer( wxUint16 layerselected ) { m_layerselected = layerselected; }

protected:

    //! Close window if OK-button is pressed.
    void    CmOk( wxCommandEvent& );

    //! Close window if CANCEL-button is pressed.
    void    CmCancel( wxCommandEvent& );

    //! Make sure the UP- and DOWN-button can't be used if the top or the bottom of the list
    //! has been reached.
    void    CmListBox( wxCommandEvent& );

    //! Close window if EXIT-button is pressed.
    void    OnCloseWindow( wxCloseEvent& event );

    void OnComEvent( a2dComEvent& event );

    wxButton*   m_buttonOK;

    wxButton*   m_buttonCANCEL;

    wxListBox*  m_listbox;

    // pointer to the layer setup of the active view.
    a2dSmrtPtr<a2dLayers> m_layersetup;

    //! set or end selected layer
    wxUint16 m_layerselected;

    //! drawing order in listbox instead of layer id
    bool m_onOrder;

    //! show modal or not
    bool m_modal;

    a2dHabitat* m_habitat;

    // Declare used events.
    DECLARE_EVENT_TABLE()

};


//! This class is used to show the layerorder dialog, in which the drawing order of the used layers
//! can be set.
class A2DCANVASDLLEXP a2dLayerOrderDlg: public wxDialog
{
public:

    //! constructor.
    a2dLayerOrderDlg( a2dHabitat* m_habitat, wxWindow* parent, a2dLayers* layersetup, const wxString& title, long style = wxRESIZE_BORDER|wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT( "layer_order_dlg" ) );

    //! destructor.
    ~a2dLayerOrderDlg();

    //! initialize with this layer setup
    void Init( a2dLayers* layersetup );

    //! store current order to layer setup
    void StoreOrder();

protected:

    //! Close window if OK-button is pressed.
    void    CmOk( wxCommandEvent& );

    //! Close window if CANCEL-button is pressed.
    void    CmCancel( wxCommandEvent& );

    //! Move selected layer to a heigher layersetting if UP-button is pressed.
    void    CmUp( wxCommandEvent& );

    //! Move selected layer to a lower layersetting if the DOWN-button is pressed.
    void    CmDown( wxCommandEvent& );

    //! Show new layerorder if the SHOW-button is pressed.
    void    CmShow( wxCommandEvent& );

    //! Make sure the UP- and DOWN-button can't be used if the top or the bottom of the list
    //! has been reached.
    void    CmListBox( wxCommandEvent& );

    //! Close window if EXIT-button is pressed.
    void    OnCloseWindow( wxCloseEvent& event );

    void OnComEvent( a2dComEvent& event );

    wxButton*   m_buttonOK;

    wxButton*   m_buttonCANCEL;

    a2dDragListBox* m_listbox;

    wxButton*   m_buttonUP;

    wxButton*   m_buttonDOWN;

    wxButton*   m_buttonSHOW;

    // pointer to the layer setup of the active view.
    a2dSmrtPtr<a2dLayers> m_layersetup;

    a2dHabitat* m_habitat;

    // Declare used events.
    DECLARE_EVENT_TABLE()

};

#endif
