/*! \file wx/canvas/identifydlg.h
    \brief Identification Dialog
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: identifydlg.h,v 1.6 2009/09/03 20:09:53 titato Exp $
*/


//!
//!
/*
 * Definition of class for dialog to specify the layerorder.
*/

#ifndef _IDENTIFYDLG_H_
#define _IDENTIFYDLG_H_

#include "wx/canvas/canobj.h"

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_IDENTIFYDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_IDENTIFYDIALOG_TITLE _("Identify Dialog")
#define SYMBOL_IDENTIFYDIALOG_IDNAME 10000
#define SYMBOL_IDENTIFYDIALOG_SIZE wxSize(500, 100)
#define SYMBOL_IDENTIFYDIALOG_POSITION wxDefaultPosition
#define ID_IDENT_LISTBOX 10001
#define ID_IDENT_HIDE 10002
#define ID_IDENT_UPDATE 10003
////@end control identifiers

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * IdentifyDialog class declaration
 */
class IdentifyDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( IdentifyDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    IdentifyDialog( );
    IdentifyDialog( wxWindow* parent, wxWindowID id = SYMBOL_IDENTIFYDIALOG_IDNAME, const wxString& caption = SYMBOL_IDENTIFYDIALOG_TITLE, const wxPoint& pos = SYMBOL_IDENTIFYDIALOG_POSITION, const wxSize& size = SYMBOL_IDENTIFYDIALOG_SIZE, long style = SYMBOL_IDENTIFYDIALOG_STYLE );

    ~IdentifyDialog( );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_IDENTIFYDIALOG_IDNAME, const wxString& caption = SYMBOL_IDENTIFYDIALOG_TITLE, const wxPoint& pos = SYMBOL_IDENTIFYDIALOG_POSITION, const wxSize& size = SYMBOL_IDENTIFYDIALOG_SIZE, long style = SYMBOL_IDENTIFYDIALOG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    //! selected child objects with this as parent will be shown
    void Init( a2dCanvasObject* showObject );

protected:
    void hide( wxCommandEvent& );
    void OnCloseWindow( wxCloseEvent& event );
    void Update( wxCommandEvent& );
    void OnActivate( wxActivateEvent& windowevent );
    //void OnActivateView( a2dViewEvent& viewevent );
    //void OnChangedDocument( a2dCommandEvent& event );

    void OnSize( wxSizeEvent& WXUNUSED( event ) );
    void OnDoEvent( a2dCommandProcessorEvent& event );
    void OnComEvent( a2dComEvent& event );

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxListBox* m_objectsList;
    wxButton* m_update;

    //! selected child objects with this as parent will be shown
    a2dCanvasObject* m_showObject;
};

#endif // _IDENTIFYDLG_H_


