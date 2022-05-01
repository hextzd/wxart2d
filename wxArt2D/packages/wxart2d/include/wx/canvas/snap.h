/////////////////////////////////////////////////////////////////////////////
// Name:        snap.h
// Purpose:
// Author:
// Modified by:
// Created:     11/21/06 16:24:43
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef _SNAP_H_
#define _SNAP_H_

/*!
 * Control identifiers
 */


/*!
 * Compatibility
 */
#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

#include "wx/general/generaldef.h"
#include "wx/canvas/eval.h"
#include "wx/canvas/canglob.h"

#define ID_SNAP 10011
#define SYMBOL_A2DSNAPSETTINGS_TITLE _("Dialog")
#define SYMBOL_A2DSNAPSETTINGS_POSITION wxDefaultPosition
#define SYMBOL_A2DSNAPSETTINGS_IDNAME ID_SNAP
#define SYMBOL_A2DSNAPSETTINGS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_A2DSNAPSETTINGS_SIZE wxSize(400, 300)

class A2DCANVASDLLEXP a2dRestrictionEngine;

/*!
 * a2dSnapSettings class declaration
 */
class a2dSnapSettings: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( a2dSnapSettings )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    a2dSnapSettings( );
    a2dSnapSettings( a2dHabitat* habitat, wxWindow* parent, wxWindowID id = SYMBOL_A2DSNAPSETTINGS_IDNAME, const wxString& caption = SYMBOL_A2DSNAPSETTINGS_TITLE, const wxPoint& pos = SYMBOL_A2DSNAPSETTINGS_POSITION, const wxSize& size = SYMBOL_A2DSNAPSETTINGS_SIZE, long style = SYMBOL_A2DSNAPSETTINGS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_A2DSNAPSETTINGS_IDNAME, const wxString& caption = SYMBOL_A2DSNAPSETTINGS_TITLE, const wxPoint& pos = SYMBOL_A2DSNAPSETTINGS_POSITION, const wxSize& size = SYMBOL_A2DSNAPSETTINGS_SIZE, long style = SYMBOL_A2DSNAPSETTINGS_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin a2dSnapSettings event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_SNAP
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_IDLE event handler for ID_SNAP
    void OnIdle( wxIdleEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_SNAPALL
    void OnSnapallClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_SNAPALL
    void OnSnapallUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_VisibleOnly
    void OnVisibleonlyClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_VisibleOnly
    void OnVisibleonlyUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_X
    void OnPosXClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_POS_X
    void OnPosXUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_Y
    void OnPosYClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_POS_Y
    void OnPosYUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_PIN
    void OnPosPinClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_POS_PIN
    void OnPosPinUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_PIN_UNCONN
    void OnPosPinUnconnClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_POS_PIN_UNCONN
    void OnPosPinUnconnUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_BBOXVERTEXES
    void OnBboxvertexesClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_BBOXVERTEXES
    void OnBboxvertexesUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_VERTEXES
    void OnVertexesClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_VERTEXES
    void OnVertexesUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_X_OBJ
    void OnPosXObjClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_POS_X_OBJ
    void OnPosXObjUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_Y_OBJ
    void OnPosYObjClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_POS_Y_OBJ
    void OnPosYObjUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_POINT_X
    void OnPosPointXClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_POS_POINT_X
    void OnPosPointXUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_POINT_Y
    void OnPosPointYClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_POS_POINT_Y
    void OnPosPointYUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_POINT_XY
    void OnPosPointXyClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_POS_POINT_XY
    void OnPosPointXyUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_SnapGrid_X
    void OnSnapgridXEnter( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_SnapGrid_X
    void OnSnapgridXUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_SnapGrid_Y
    void OnSnapgridYEnter( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_SnapGrid_Y
    void OnSnapgridYUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_SnapOrigin_X
    void OnSnaporiginXEnter( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_SnapOrigin_X
    void OnSnaporiginXUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_SnapOrigin_Y
    void OnSnaporiginYEnter( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_SnapOrigin_Y
    void OnSnaporiginYUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_RotSnapAngle
    void OnRotsnapangleEnter( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RotSnapAngle
    void OnRotsnapangleUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_SnapThreshold
    void OnSnapthresholdEnter( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_SnapThreshold
    void OnSnapthresholdUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_snapPointX
    void OnSnappointxEnter( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_snapPointX
    void OnSnappointxUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_snapPointY
    void OnSnappointyEnter( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_snapPointY
    void OnSnappointyUpdate( wxUpdateUIEvent& event );

////@end a2dSnapSettings event handler declarations

////@begin a2dSnapSettings member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end a2dSnapSettings member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin a2dSnapSettings member variables
    wxCheckBox* m_snapEnabled;
    wxCheckBox* m_visibleOnly;
    wxCheckBox* m_gridX;
    wxCheckBox* m_gridY;
    wxCheckBox* m_pins;
    wxCheckBox* m_pinsConn;
    wxCheckBox* m_bboxvertexes;
    wxCheckBox* m_vertexes;
    wxCheckBox* m_objPosX;
    wxCheckBox* m_objPosY;
    wxCheckBox* m_pointPosX;
    wxCheckBox* m_pointPosY;
    wxCheckBox* m_pointPosXY;
    wxTextCtrl* m_gridDistX;
    wxTextCtrl* m_gridDistY;
    wxTextCtrl* m_orgX;
    wxTextCtrl* m_orgY;
    wxTextCtrl* m_rotation;
    wxTextCtrl* m_threshold;
    wxTextCtrl* m_snapPointX;
    wxTextCtrl* m_snapPointY;
    a2dRestrictionEngine* m_restrict;

    a2dHabitat* m_habitat;
////@end a2dSnapSettings member variables
};

#endif
// _SNAP_H_
