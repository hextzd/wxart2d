/////////////////////////////////////////////////////////////////////////////
// Name:        snap.cpp
// Purpose:
// Author:
// Modified by:
// Created:     11/24/06 11:51:50
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "wx/canvas/snap.h"
#include "wx/canvas/canglob.h"

////@begin control identifiers
#define ID_SNAPALL 10000
#define ID_VisibleOnly 10014
#define ID_POS_X 10001
#define ID_POS_Y 10003
#define ID_POS_PIN 10004
#define ID_POS_PIN_UNCONN 10013
#define ID_BBOXVERTEXES 10021
#define ID_VERTEXES 10005
#define ID_POS_X_OBJ 10006
#define ID_POS_Y_OBJ 10007
#define ID_POS_POINT_X 10008
#define ID_POS_POINT_Y 10010
#define ID_POS_POINT_XY 10009
#define ID_SnapGrid_X 10002
#define ID_SnapGrid_Y 10012
#define ID_SnapOrigin_X 10015
#define ID_SnapOrigin_Y 10017
#define ID_RotSnapAngle 10018
#define ID_SnapThreshold 10019
#define ID_snapPointX 10022
#define ID_snapPointY 10023
////@end control identifiers

////@begin XPM images
////@end XPM images

/*!
 * a2dSnapSettings type definition
 */

IMPLEMENT_DYNAMIC_CLASS( a2dSnapSettings, wxDialog )

/*!
 * a2dSnapSettings event table definition
 */

BEGIN_EVENT_TABLE( a2dSnapSettings, wxDialog )

    EVT_CLOSE( a2dSnapSettings::OnCloseWindow )
    EVT_IDLE( a2dSnapSettings::OnIdle )

    EVT_CHECKBOX( ID_SNAPALL, a2dSnapSettings::OnSnapallClick )
    EVT_UPDATE_UI( ID_SNAPALL, a2dSnapSettings::OnSnapallUpdate )

    EVT_CHECKBOX( ID_VisibleOnly, a2dSnapSettings::OnVisibleonlyClick )
    EVT_UPDATE_UI( ID_VisibleOnly, a2dSnapSettings::OnVisibleonlyUpdate )

    EVT_CHECKBOX( ID_POS_X, a2dSnapSettings::OnPosXClick )
    EVT_UPDATE_UI( ID_POS_X, a2dSnapSettings::OnPosXUpdate )

    EVT_CHECKBOX( ID_POS_Y, a2dSnapSettings::OnPosYClick )
    EVT_UPDATE_UI( ID_POS_Y, a2dSnapSettings::OnPosYUpdate )

    EVT_CHECKBOX( ID_POS_PIN, a2dSnapSettings::OnPosPinClick )
    EVT_UPDATE_UI( ID_POS_PIN, a2dSnapSettings::OnPosPinUpdate )

    EVT_CHECKBOX( ID_POS_PIN_UNCONN, a2dSnapSettings::OnPosPinUnconnClick )
    EVT_UPDATE_UI( ID_POS_PIN_UNCONN, a2dSnapSettings::OnPosPinUnconnUpdate )

    EVT_CHECKBOX( ID_BBOXVERTEXES, a2dSnapSettings::OnBboxvertexesClick )
    EVT_UPDATE_UI( ID_BBOXVERTEXES, a2dSnapSettings::OnBboxvertexesUpdate )

    EVT_CHECKBOX( ID_VERTEXES, a2dSnapSettings::OnVertexesClick )
    EVT_UPDATE_UI( ID_VERTEXES, a2dSnapSettings::OnVertexesUpdate )

    EVT_CHECKBOX( ID_POS_X_OBJ, a2dSnapSettings::OnPosXObjClick )
    EVT_UPDATE_UI( ID_POS_X_OBJ, a2dSnapSettings::OnPosXObjUpdate )

    EVT_CHECKBOX( ID_POS_Y_OBJ, a2dSnapSettings::OnPosYObjClick )
    EVT_UPDATE_UI( ID_POS_Y_OBJ, a2dSnapSettings::OnPosYObjUpdate )

    EVT_CHECKBOX( ID_POS_POINT_X, a2dSnapSettings::OnPosPointXClick )
    EVT_UPDATE_UI( ID_POS_POINT_X, a2dSnapSettings::OnPosPointXUpdate )

    EVT_CHECKBOX( ID_POS_POINT_Y, a2dSnapSettings::OnPosPointYClick )
    EVT_UPDATE_UI( ID_POS_POINT_Y, a2dSnapSettings::OnPosPointYUpdate )

    EVT_CHECKBOX( ID_POS_POINT_XY, a2dSnapSettings::OnPosPointXyClick )
    EVT_UPDATE_UI( ID_POS_POINT_XY, a2dSnapSettings::OnPosPointXyUpdate )

    EVT_TEXT_ENTER( ID_SnapGrid_X, a2dSnapSettings::OnSnapgridXEnter )
    EVT_UPDATE_UI( ID_SnapGrid_X, a2dSnapSettings::OnSnapgridXUpdate )

    EVT_TEXT_ENTER( ID_SnapGrid_Y, a2dSnapSettings::OnSnapgridYEnter )
    EVT_UPDATE_UI( ID_SnapGrid_Y, a2dSnapSettings::OnSnapgridYUpdate )

    EVT_TEXT_ENTER( ID_SnapOrigin_X, a2dSnapSettings::OnSnaporiginXEnter )
    EVT_UPDATE_UI( ID_SnapOrigin_X, a2dSnapSettings::OnSnaporiginXUpdate )

    EVT_TEXT_ENTER( ID_SnapOrigin_Y, a2dSnapSettings::OnSnaporiginYEnter )
    EVT_UPDATE_UI( ID_SnapOrigin_Y, a2dSnapSettings::OnSnaporiginYUpdate )

    EVT_TEXT_ENTER( ID_RotSnapAngle, a2dSnapSettings::OnRotsnapangleEnter )
    EVT_UPDATE_UI( ID_RotSnapAngle, a2dSnapSettings::OnRotsnapangleUpdate )

    EVT_TEXT_ENTER( ID_SnapThreshold, a2dSnapSettings::OnSnapthresholdEnter )
    EVT_UPDATE_UI( ID_SnapThreshold, a2dSnapSettings::OnSnapthresholdUpdate )

    EVT_TEXT_ENTER( ID_snapPointX, a2dSnapSettings::OnSnappointxEnter )
    EVT_UPDATE_UI( ID_snapPointX, a2dSnapSettings::OnSnappointxUpdate )

    EVT_TEXT_ENTER( ID_snapPointY, a2dSnapSettings::OnSnappointyEnter )
    EVT_UPDATE_UI( ID_snapPointY, a2dSnapSettings::OnSnappointyUpdate )

END_EVENT_TABLE()

/*!
 * a2dSnapSettings constructors
 */

a2dSnapSettings::a2dSnapSettings( )
{
}

a2dSnapSettings::a2dSnapSettings( a2dHabitat* habitat, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_habitat = habitat;
    m_restrict = m_habitat->GetRestrictionEngine();
    wxASSERT_MSG( m_restrict, _( "a2dSnapSettings needs a m_habitat->GetRestrictionEngine() set" ) );

    Create( parent, id, caption, pos, size, style );
}

/*!
 * a2dSnapSettings creator
 */

bool a2dSnapSettings::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_snapEnabled = NULL;
    m_visibleOnly = NULL;
    m_gridX = NULL;
    m_gridY = NULL;
    m_pins = NULL;
    m_pinsConn = NULL;
    m_vertexes = NULL;
    m_objPosX = NULL;
    m_objPosY = NULL;
    m_pointPosX = NULL;
    m_pointPosY = NULL;
    m_pointPosXY = NULL;
    m_gridDistX = NULL;
    m_gridDistY = NULL;
    m_orgX = NULL;
    m_orgY = NULL;
    m_rotation = NULL;
    m_threshold = NULL;
    m_snapPointX = NULL;
    m_snapPointY = NULL;

    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
    Centre();

    return TRUE;
}

void a2dSnapSettings::CreateControls()
{
////@begin a2dSnapSettings content construction

    a2dSnapSettings* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1 );

    m_snapEnabled = new wxCheckBox( itemDialog1, ID_SNAPALL, _( "Enable snapping" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_snapEnabled->SetValue( TRUE );
    m_snapEnabled->SetHelpText( _( "To enable/disable all snapping " ) );
    if ( ShowToolTips() )
        m_snapEnabled->SetToolTip( _( "To enable/disable all snapping" ) );
    itemBoxSizer3->Add( m_snapEnabled, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    m_visibleOnly = new wxCheckBox( itemDialog1, ID_VisibleOnly, _( "Vissible Only" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_visibleOnly->SetValue( FALSE );
    itemBoxSizer3->Add( m_visibleOnly, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer6, 1, wxGROW | wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 1 );

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer6->Add( itemBoxSizer7, 0, wxALIGN_TOP | wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 1 );

    m_gridX = new wxCheckBox( itemDialog1, ID_POS_X, _( "Snap Grid X" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_gridX->SetValue( FALSE );
    m_gridX->SetHelpText( _( "Snap to canvas grid in Y" ) );
    if ( ShowToolTips() )
        m_gridX->SetToolTip( _( "Snap to canvas grid in Y" ) );
    itemBoxSizer7->Add( m_gridX, 0, wxALIGN_LEFT | wxALL, 5 );

    m_gridY = new wxCheckBox( itemDialog1, ID_POS_Y, _( "Snap Grid Y" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_gridY->SetValue( FALSE );
    m_gridY->SetHelpText( _( "Snap to canvas grid in Y" ) );
    if ( ShowToolTips() )
        m_gridY->SetToolTip( _( "Snap to canvas grid in Y" ) );
    itemBoxSizer7->Add( m_gridY, 0, wxALIGN_LEFT | wxALL, 5 );

    m_pins = new wxCheckBox( itemDialog1, ID_POS_PIN, _( "Position Pin" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_pins->SetValue( FALSE );
    m_pins->SetHelpText( _( "Snap to pins in a canvas object" ) );
    if ( ShowToolTips() )
        m_pins->SetToolTip( _( "Snap to pins in a canvas object" ) );
    itemBoxSizer7->Add( m_pins, 0, wxALIGN_LEFT | wxALL, 5 );

    m_pinsConn = new wxCheckBox( itemDialog1, ID_POS_PIN_UNCONN, _( "Position Pin Uncon." ), wxDefaultPosition, wxDefaultSize, 0 );
    m_pinsConn->SetValue( FALSE );
    m_pinsConn->SetHelpText( _( "Snap to unconnected pins in a canvas object" ) );
    if ( ShowToolTips() )
        m_pinsConn->SetToolTip( _( "Snap to unconnected pins in a canvas object" ) );
    itemBoxSizer7->Add( m_pinsConn, 0, wxALIGN_LEFT | wxALL, 5 );

    m_bboxvertexes = new wxCheckBox( itemDialog1, ID_BBOXVERTEXES, _( "Object Bbox" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_bboxvertexes->SetValue( FALSE );
    itemBoxSizer7->Add( m_bboxvertexes, 0, wxALIGN_LEFT | wxALL, 5 );

    m_vertexes = new wxCheckBox( itemDialog1, ID_VERTEXES, _( "Object Vertexes " ), wxDefaultPosition, wxDefaultSize, 0 );
    m_vertexes->SetValue( FALSE );
    m_vertexes->SetHelpText( _( "Snap to the canvas object vector path vertexes" ) );
    if ( ShowToolTips() )
        m_vertexes->SetToolTip( _( "Snap to the canvas object vector path vertexes" ) );
    itemBoxSizer7->Add( m_vertexes, 0, wxALIGN_LEFT | wxALL, 5 );

    m_objPosX = new wxCheckBox( itemDialog1, ID_POS_X_OBJ, _( "Object Pos X" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_objPosX->SetValue( FALSE );
    m_objPosX->SetHelpText( _( "Snap to the canvas object position in X" ) );
    if ( ShowToolTips() )
        m_objPosX->SetToolTip( _( "Snap to the canvas object position in X" ) );
    itemBoxSizer7->Add( m_objPosX, 0, wxALIGN_LEFT | wxALL, 5 );

    m_objPosY = new wxCheckBox( itemDialog1, ID_POS_Y_OBJ, _( "Object Pos Y" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_objPosY->SetValue( FALSE );
    m_objPosY->SetHelpText( _( "Snap to the canvas object position in Y" ) );
    if ( ShowToolTips() )
        m_objPosY->SetToolTip( _( "Snap to the canvas object position in Y" ) );
    itemBoxSizer7->Add( m_objPosY, 0, wxALIGN_LEFT | wxALL, 5 );

    m_pointPosX = new wxCheckBox( itemDialog1, ID_POS_POINT_X, _( "Snap Point X" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_pointPosX->SetValue( FALSE );
    m_pointPosX->SetHelpText( _( "Snap to the snap point  in Y" ) );
    if ( ShowToolTips() )
        m_pointPosX->SetToolTip( _( "Snap to the snap point  in Y" ) );
    itemBoxSizer7->Add( m_pointPosX, 0, wxALIGN_LEFT | wxALL, 5 );

    m_pointPosY = new wxCheckBox( itemDialog1, ID_POS_POINT_Y, _( "Snap Point Y" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_pointPosY->SetValue( FALSE );
    m_pointPosY->SetHelpText( _( "Snap to the snap point  in Y" ) );
    if ( ShowToolTips() )
        m_pointPosY->SetToolTip( _( "Snap to the snap point  in Y" ) );
    itemBoxSizer7->Add( m_pointPosY, 0, wxALIGN_LEFT | wxALL, 5 );

    m_pointPosXY = new wxCheckBox( itemDialog1, ID_POS_POINT_XY, _( "Snap Point X,Y" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_pointPosXY->SetValue( FALSE );
    m_pointPosXY->SetHelpText( _( "Snap to the snap point  in X and Y" ) );
    if ( ShowToolTips() )
        m_pointPosXY->SetToolTip( _( "Snap to the snap point  in X and Y" ) );
    itemBoxSizer7->Add( m_pointPosXY, 0, wxALIGN_LEFT | wxALL, 5 );

    wxBoxSizer* itemBoxSizer19 = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer6->Add( itemBoxSizer19, 2, wxALIGN_TOP | wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 1 );

    wxBoxSizer* itemBoxSizer20 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer19->Add( itemBoxSizer20, 0, wxGROW | wxALL, 1 );

    wxStaticText* itemStaticText21 = new wxStaticText( itemDialog1, wxID_STATIC, _( "Snap Grid" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText21->SetHelpText( _( "Canvas grid distance in X and Y" ) );
    if ( ShowToolTips() )
        itemStaticText21->SetToolTip( _( "Canvas grid distance in X and Y" ) );
    itemBoxSizer20->Add( itemStaticText21, 0, wxALIGN_CENTER_VERTICAL | wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 5 );

    m_gridDistX = new wxTextCtrl( itemDialog1, ID_SnapGrid_X, _T( "" ), wxDefaultPosition, wxSize( 50, -1 ), wxTE_PROCESS_ENTER );
    itemBoxSizer20->Add( m_gridDistX, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_gridDistY = new wxTextCtrl( itemDialog1, ID_SnapGrid_Y, _T( "" ), wxDefaultPosition, wxSize( 50, -1 ), wxTE_PROCESS_ENTER );
    itemBoxSizer20->Add( m_gridDistY, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer19->Add( itemBoxSizer24, 0, wxGROW | wxALL, 1 );

    wxStaticText* itemStaticText25 = new wxStaticText( itemDialog1, wxID_STATIC, _( "Snap Origin" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText25->SetHelpText( _( "Canvas grid origin in X and Y" ) );
    if ( ShowToolTips() )
        itemStaticText25->SetToolTip( _( "Canvas grid origin in X and Y" ) );
    itemBoxSizer24->Add( itemStaticText25, 0, wxALIGN_CENTER_VERTICAL | wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 5 );

    m_orgX = new wxTextCtrl( itemDialog1, ID_SnapOrigin_X, _T( "" ), wxDefaultPosition, wxSize( 50, -1 ), wxTE_PROCESS_ENTER );
    itemBoxSizer24->Add( m_orgX, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_orgY = new wxTextCtrl( itemDialog1, ID_SnapOrigin_Y, _T( "" ), wxDefaultPosition, wxSize( 50, -1 ), wxTE_PROCESS_ENTER );
    itemBoxSizer24->Add( m_orgY, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer19->Add( itemBoxSizer28, 0, wxGROW | wxALL, 1 );

    wxStaticText* itemStaticText29 = new wxStaticText( itemDialog1, wxID_STATIC, _( "Rotation Angle snap" ), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
    itemStaticText29->SetHelpText( _( "steps in which rotation can snap" ) );
    if ( ShowToolTips() )
        itemStaticText29->SetToolTip( _( "steps in which rotation can snap" ) );
    itemBoxSizer28->Add( itemStaticText29, 0, wxALIGN_CENTER_VERTICAL | wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 5 );

    m_rotation = new wxTextCtrl( itemDialog1, ID_RotSnapAngle, _T( "" ), wxDefaultPosition, wxSize( 50, -1 ), wxTE_PROCESS_ENTER );
    itemBoxSizer28->Add( m_rotation, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer31 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer19->Add( itemBoxSizer31, 0, wxGROW | wxALL, 1 );

    wxStaticText* itemStaticText32 = new wxStaticText( itemDialog1, wxID_STATIC, _( "Snap Threshold" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer31->Add( itemStaticText32, 0, wxALIGN_CENTER_VERTICAL | wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 5 );

    m_threshold = new wxTextCtrl( itemDialog1, ID_SnapThreshold, _T( "" ), wxDefaultPosition, wxSize( 50, -1 ), wxTE_PROCESS_ENTER );
    itemBoxSizer31->Add( m_threshold, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer34 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer19->Add( itemBoxSizer34, 0, wxGROW | wxALL, 1 );

    wxStaticText* itemStaticText35 = new wxStaticText( itemDialog1, wxID_STATIC, _( "Snap Point" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer34->Add( itemStaticText35, 0, wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 5 );

    m_snapPointX = new wxTextCtrl( itemDialog1, ID_snapPointX, _T( "" ), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    itemBoxSizer34->Add( m_snapPointX, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_snapPointY = new wxTextCtrl( itemDialog1, ID_snapPointY, _T( "" ), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    itemBoxSizer34->Add( m_snapPointY, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

////@end a2dSnapSettings content construction
}

/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_DIALOG
 */

void a2dSnapSettings::OnCloseWindow( wxCloseEvent& event )
{
    event.Skip();
}

/*!
 * wxEVT_IDLE event handler for ID_DIALOG
 */

void a2dSnapSettings::OnIdle( wxIdleEvent& event )
{
    event.Skip();
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_SNAPALL
 */

void a2dSnapSettings::OnSnapallClick( wxCommandEvent& event )
{
    m_restrict->SetSnap( m_snapEnabled->IsChecked() );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_SNAPALL
 */

void a2dSnapSettings::OnSnapallUpdate( wxUpdateUIEvent& event )
{
    m_snapEnabled->SetValue( m_restrict->GetSnap() );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_VisibleOnly
 */

void a2dSnapSettings::OnVisibleonlyClick( wxCommandEvent& event )
{
    //a2dCommand_SetProperty* command = new a2dCommand_SetProperty( m_restrict, a2dRestrictionEngine::PROPID_SnapOnlyVisbleObjects, m_visibleOnly->IsChecked() );
    //a2dGetCmdh()->Submit( command );
    m_restrict->SetSnapOnlyVisibleObjects( m_visibleOnly->IsChecked() );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_VisibleOnly
 */

void a2dSnapSettings::OnVisibleonlyUpdate( wxUpdateUIEvent& event )
{
    m_visibleOnly->SetValue( m_restrict->GetSnapOnlyVisibleObjects() );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_X
 */

void a2dSnapSettings::OnPosXClick( wxCommandEvent& event )
{
    m_restrict->SetSnapTargetFeature( a2dRestrictionEngine::snapToGridPosX, m_gridX->GetValue() );
    //m_restrict->SetSnapTargetFeatures( featureOn );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_POS_X
 */

void a2dSnapSettings::OnPosXUpdate( wxUpdateUIEvent& event )
{
    m_gridX->SetValue( m_restrict->GetSnapTargetFeature( a2dRestrictionEngine::snapToGridPosX ) );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_Y
 */

void a2dSnapSettings::OnPosYClick( wxCommandEvent& event )
{
    m_restrict->SetSnapTargetFeature( a2dRestrictionEngine::snapToGridPosY, m_gridY->GetValue() );
    //m_restrict->SetSnapTargetFeatures( featureOn );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_POS_Y
 */

void a2dSnapSettings::OnPosYUpdate( wxUpdateUIEvent& event )
{
    m_gridY->SetValue( m_restrict->GetSnapTargetFeature( a2dRestrictionEngine::snapToGridPosY ) );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_PIN
 */

void a2dSnapSettings::OnPosPinClick( wxCommandEvent& event )
{
    m_restrict->SetSnapTargetFeature( a2dRestrictionEngine::snapToPins, m_pins->GetValue() );
    //m_restrict->SetSnapTargetFeatures( featureOn );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_POS_PIN
 */

void a2dSnapSettings::OnPosPinUpdate( wxUpdateUIEvent& event )
{
    m_pins->SetValue( m_restrict->GetSnapTargetFeature( a2dRestrictionEngine::snapToPins ) );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_PIN_UNCONN
 */

void a2dSnapSettings::OnPosPinUnconnClick( wxCommandEvent& event )
{
    m_restrict->SetSnapTargetFeature( a2dRestrictionEngine::snapToPinsUnconnected, m_pinsConn->GetValue() );
    //m_restrict->SetSnapTargetFeatures( featureOn );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_POS_PIN_UNCONN
 */

void a2dSnapSettings::OnPosPinUnconnUpdate( wxUpdateUIEvent& event )
{
    m_pinsConn->SetValue( m_restrict->GetSnapTargetFeature( a2dRestrictionEngine::snapToPinsUnconnected ) );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_VERTEXES
 */

void a2dSnapSettings::OnVertexesClick( wxCommandEvent& event )
{
    m_restrict->SetSnapTargetFeature( a2dRestrictionEngine::snapToObjectVertexes, m_vertexes->GetValue() );
    //m_restrict->SetSnapTargetFeatures( featureOn );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_VERTEXES
 */

void a2dSnapSettings::OnVertexesUpdate( wxUpdateUIEvent& event )
{
    m_vertexes->SetValue( m_restrict->GetSnapTargetFeature( a2dRestrictionEngine::snapToObjectVertexes ) );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_BBOXVERTEXES
 */

void a2dSnapSettings::OnBboxvertexesClick( wxCommandEvent& event )
{
    m_restrict->SetSnapTargetFeature( a2dRestrictionEngine::snapToBoundingBox, m_bboxvertexes->GetValue() );
    //m_restrict->SetSnapTargetFeatures( featureOn );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_BBOXVERTEXES
 */

void a2dSnapSettings::OnBboxvertexesUpdate( wxUpdateUIEvent& event )
{
    m_bboxvertexes->SetValue( m_restrict->GetSnapTargetFeature( a2dRestrictionEngine::snapToBoundingBox ) );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_X_OBJ
 */

void a2dSnapSettings::OnPosXObjClick( wxCommandEvent& event )
{
    m_restrict->SetSnapTargetFeature( a2dRestrictionEngine::snapToObjectPosX, m_objPosX->GetValue() );
    //m_restrict->SetSnapTargetFeatures( featureOn );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_POS_X_OBJ
 */

void a2dSnapSettings::OnPosXObjUpdate( wxUpdateUIEvent& event )
{
    m_objPosX->SetValue( m_restrict->GetSnapTargetFeature( a2dRestrictionEngine::snapToObjectPosX ) );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_Y_OBJ
 */

void a2dSnapSettings::OnPosYObjClick( wxCommandEvent& event )
{
    m_restrict->SetSnapTargetFeature( a2dRestrictionEngine::snapToObjectPosY, m_objPosY->GetValue() );
    //m_restrict->SetSnapTargetFeatures( featureOn );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_POS_Y_OBJ
 */

void a2dSnapSettings::OnPosYObjUpdate( wxUpdateUIEvent& event )
{
    m_objPosY->SetValue( m_restrict->GetSnapTargetFeature( a2dRestrictionEngine::snapToObjectPosY ) );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_POINT_X
 */

void a2dSnapSettings::OnPosPointXClick( wxCommandEvent& event )
{
    m_restrict->SetSnapTargetFeature( a2dRestrictionEngine::snapToPointPosX, m_pointPosX->GetValue() );
    //m_restrict->SetSnapTargetFeatures( featureOn );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_POS_POINT_X
 */

void a2dSnapSettings::OnPosPointXUpdate( wxUpdateUIEvent& event )
{
    m_pointPosX->SetValue( m_restrict->GetSnapTargetFeature( a2dRestrictionEngine::snapToPointPosX ) );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_POINT_Y
 */

void a2dSnapSettings::OnPosPointYClick( wxCommandEvent& event )
{
    m_restrict->SetSnapTargetFeature( a2dRestrictionEngine::snapToPointPosY, m_pointPosY->GetValue() );
    //m_restrict->SetSnapTargetFeatures( featureOn );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_POS_POINT_Y
 */

void a2dSnapSettings::OnPosPointYUpdate( wxUpdateUIEvent& event )
{
    m_pointPosY->SetValue( m_restrict->GetSnapTargetFeature( a2dRestrictionEngine::snapToPointPosY ) );
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_POS_POINT_XY
 */

void a2dSnapSettings::OnPosPointXyClick( wxCommandEvent& event )
{
    m_restrict->SetSnapTargetFeature( a2dRestrictionEngine::snapToPointPosXorYForce, m_pointPosXY->GetValue() );
    //m_restrict->SetSnapTargetFeatures( featureOn );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_POS_POINT_XY
 */

void a2dSnapSettings::OnPosPointXyUpdate( wxUpdateUIEvent& event )
{
    m_pointPosXY->SetValue( m_restrict->GetSnapTargetFeature( a2dRestrictionEngine::snapToPointPosXorYForce ) );
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_SnapGrid_X
 */

void a2dSnapSettings::OnSnapgridXEnter( wxCommandEvent& event )
{
    a2dDoMu dx;
    dx.Eval( m_gridDistX->GetValue() );
    a2dDoMu dy;
    dy.Eval( m_gridDistY->GetValue() );
    m_restrict->SetSnapGrid( dx, dy );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_SnapGrid_X
 */

void a2dSnapSettings::OnSnapgridXUpdate( wxUpdateUIEvent& event )
{
    if ( FindFocus() != m_gridDistX )
        m_gridDistX->SetValue( m_restrict->GetSnapGridX().GetValueString() );
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_SnapGrid_Y
 */

void a2dSnapSettings::OnSnapgridYEnter( wxCommandEvent& event )
{
    a2dDoMu dx;
    dx.Eval( m_gridDistX->GetValue() );
    a2dDoMu dy;
    dy.Eval( m_gridDistY->GetValue() );
    m_restrict->SetSnapGrid( dx, dy );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_SnapGrid_Y
 */

void a2dSnapSettings::OnSnapgridYUpdate( wxUpdateUIEvent& event )
{
    if ( FindFocus() != m_gridDistY )
        m_gridDistY->SetValue( m_restrict->GetSnapGridY().GetValueString() );
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_SnapOrigin_X
 */

void a2dSnapSettings::OnSnaporiginXEnter( wxCommandEvent& event )
{
    double dx, dy;
    m_orgX->GetValue().ToDouble( &dx );
    m_orgY->GetValue().ToDouble( &dy );
    m_restrict->SetSnapOrigin( dx, dy );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_SnapOrigin_X
 */

void a2dSnapSettings::OnSnaporiginXUpdate( wxUpdateUIEvent& event )
{
    if ( FindFocus() != m_orgX )
        m_orgX->SetValue( m_restrict->GetSnapOriginX().GetValueString() );
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_SnapOrigin_Y
 */

void a2dSnapSettings::OnSnaporiginYEnter( wxCommandEvent& event )
{
    double dx, dy;
    m_orgX->GetValue().ToDouble( &dx );
    m_orgY->GetValue().ToDouble( &dy );
    m_restrict->SetSnapOrigin( dx, dy );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_SnapOrigin_Y
 */

void a2dSnapSettings::OnSnaporiginYUpdate( wxUpdateUIEvent& event )
{
    if ( FindFocus() != m_orgY )
        m_orgY->SetValue( m_restrict->GetSnapOriginY().GetValueString() );
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_RotSnapAngle
 */

void a2dSnapSettings::OnRotsnapangleEnter( wxCommandEvent& event )
{
    double ang;
    m_orgY->GetValue().ToDouble( &ang );
    m_restrict->SetRotationAngle( ang );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_RotSnapAngle
 */

void a2dSnapSettings::OnRotsnapangleUpdate( wxUpdateUIEvent& event )
{
    wxString buf;
    buf.Printf( wxT( "%f" ), m_restrict->GetRotationAngle() );
    if ( FindFocus() != m_rotation )
        m_rotation->SetValue( buf );
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_SnapThreshold
 */

void a2dSnapSettings::OnSnapthresholdEnter( wxCommandEvent& event )
{
    long ang;
    m_threshold->GetValue().ToLong( &ang );
    m_restrict->SetSnapThresHold( ang );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_SnapThreshold
 */

void a2dSnapSettings::OnSnapthresholdUpdate( wxUpdateUIEvent& event )
{
    wxString buf;
    buf.Printf( wxT( "%d" ), m_restrict->GetSnapThresHold() );
    if ( FindFocus() != m_threshold )
        m_threshold->SetValue( buf );
}


void a2dSnapSettings::OnSnappointyEnter( wxCommandEvent& event )
{
    double dx, dy;
    m_snapPointX->GetValue().ToDouble( &dx );
    m_snapPointY->GetValue().ToDouble( &dy );
    m_restrict->SetRestrictPoint( dx, dy );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_TEXTCTRL1
 */

void a2dSnapSettings::OnSnappointyUpdate( wxUpdateUIEvent& event )
{
    wxString buf;
    buf.Printf( wxT( "%f" ), m_restrict->GetRestrictPoint().m_y );
    if ( FindFocus() != m_snapPointY )
        m_snapPointY->SetValue( buf );
}


/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_snapPointX
 */

void a2dSnapSettings::OnSnappointxEnter( wxCommandEvent& event )
{
    double dx, dy;
    m_snapPointX->GetValue().ToDouble( &dx );
    m_snapPointY->GetValue().ToDouble( &dy );
    m_restrict->SetRestrictPoint( dx, dy );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_snapPointX
 */

void a2dSnapSettings::OnSnappointxUpdate( wxUpdateUIEvent& event )
{
    wxString buf;
    buf.Printf( wxT( "%f" ), m_restrict->GetRestrictPoint().m_x );
    if ( FindFocus() != m_snapPointX )
        m_snapPointX->SetValue( buf );
}

/*!
 * Should we show tooltips?
 */

bool a2dSnapSettings::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap a2dSnapSettings::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin a2dSnapSettings bitmap retrieval
    return wxNullBitmap;
////@end a2dSnapSettings bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon a2dSnapSettings::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin a2dSnapSettings icon retrieval
    return wxNullIcon;
////@end a2dSnapSettings icon retrieval
}
