/////////////////////////////////////////////////////////////////////////////
// Name:        settingsdlg.cpp
// Purpose:
// Author:      Klaas Holwerda
// Modified by:
// Created:     17/04/2008 14:15:57
// RCS-ID:
// Copyright:   Klaas Holwerda
// Licence:
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes
#include "wx/canvas/canmod.h"
#include "wx/canvas/settingsdlg.h"

////@begin XPM images
////@end XPM images

#define ID_UNITS 10005
#define ID_settings 10010
#define ID_displaySettings 10013
#define ID_display 10017
#define ID_TEXTCTRL1 10019
#define ID_CHECKBOX1 10020
#define ID_TEXTCTRL8 10028
#define ID_CHECKBOX4 10038
#define ID_PANEL2 10023
#define ID_TEXTCTRL2 10021
#define ID_TEXTCTRL3 10022
#define ID_TEXTCTRL4 10024
#define ID_TEXTCTRL9 10029
#define ID_TEXTCTRL5 10025
#define ID_TEXTCTRL6 10026
#define ID_TEXTCTRL7 10027
#define ID_CHECKBOX2 10030
#define ID_CHECKBOX3 10031
#define ID_PANEL1 10032
#define ID_TEXTCTRL10 10033
#define ID_TEXTCTRL12 10035
#define ID_TEXTCTRL11 10034
#define ID_TEXTCTRL13 10036
#define ID_BUTTON1 10037
#define ID_SnapGrid_X 10038
#define ID_SnapGrid_Y 10038

/*!
 * a2dSettings type definition
 */

IMPLEMENT_DYNAMIC_CLASS( a2dSettings, wxDialog )


/*!
 * a2dSettings event table definition
 */

BEGIN_EVENT_TABLE( a2dSettings, wxDialog )
    EVT_CLOSE( a2dSettings::OnCloseWindow )
    //EVT_CHANGED_DOCUMENT( a2dSettings::OnChangedDocument )
    EVT_CHOICE( ID_UNITS, a2dSettings::OnUnitsSelected )
END_EVENT_TABLE()


/*!
 * a2dSettings constructors
*/
a2dSettings::a2dSettings()
{
/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );
*/
    Init();
}

a2dSettings::a2dSettings( a2dHabitat* habitat, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_habitat = habitat;
/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );
*/
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*!
 * a2dSettings creator
 */

bool a2dSettings::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin a2dSettings creation
    SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if ( GetSizer() )
    {
        GetSizer()->SetSizeHints( this );
    }
    Centre();
////@end a2dSettings creation

    return true;
}

bool a2dSettings::Show( bool show )
{
    if ( show )
    {
        UpdateEntries();
    }
    return wxDialog::Show( show );
}

/*!
 * a2dSettings destructor
 */

a2dSettings::~a2dSettings()
{
/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );
*/
}


/*!
 * Member initialisation
 */

void a2dSettings::Init()
{
    m_settingsTabs = NULL;
    m_displaySet = NULL;
    m_displayAberation = NULL;
    m_primThreshold = NULL;
    m_drawRectangle = NULL;
    m_selectionDistance = NULL;
    m_booleanSet = NULL;
    m_snapfactor = NULL;
    m_offsetAber = NULL;
    m_roundFactor = NULL;
    m_offset = NULL;
    m_smoothAber = NULL;
    m_maxLineMerge = NULL;
    m_intersectionGrid = NULL;
    m_fillRuleWinding = NULL;
    m_linkHoles = NULL;
    m_arcpolySet = NULL;
    m_minRadius = NULL;
    m_maxRadius = NULL;
    m_poly2arcAber = NULL;
    m_arc2polyAber = NULL;

}

void a2dSettings::UpdateEntries()
{
    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawingPart )
        return;

    wxString unitstr = drawingPart->GetDrawing()->GetUnits();
    double fromMeters;
    a2dDoMu::GetMultiplierFromString( unitstr, fromMeters );
    fromMeters = 1 / fromMeters;
    a2dDoMu marge = m_habitat->GetBooleanEngineMarge();
    m_snapfactor->SetValue( marge.GetValueString() );

    wxString number;

    double d = m_habitat->GetDisplayAberration();
    number.Printf( wxT( "%lg" ), d );
    m_displayAberation->SetValue( number );

    number.Printf( wxT( "%lg" ), m_habitat->GetGridX() );
    m_gridDistX->SetValue( number );
    number.Printf( wxT( "%lg" ), m_habitat->GetGridY() );
    m_gridDistY->SetValue( number );
    number.Printf( wxT( "%d" ), a2dGlobals->GetPrimitiveThreshold() );
    m_primThreshold->SetValue( number );
    number.Printf( wxT( "%d" ), m_habitat->GetHitMarginDevice() );
    m_selectionDistance->SetValue( number );

    m_drawRectangle->SetValue( a2dGlobals->GetThresholdDrawRectangle() );

    m_smoothAber->SetValue( m_habitat->GetBooleanEngineSmoothAber().GetValueString() );
    m_offsetAber->SetValue( m_habitat->GetBooleanEngineCorrectionAber().GetValueString() );
    m_offset->SetValue( m_habitat->GetBooleanEngineCorrectionFactor().GetValueString() );
    m_maxLineMerge->SetValue( m_habitat->GetBooleanEngineMaxlinemerge().GetValueString() );
    m_poly2arcAber->SetValue( m_habitat->GetAberPolyToArc().GetValueString() );
    m_arc2polyAber->SetValue( m_habitat->GetAberArcToPoly().GetValueString() );
    m_minRadius->SetValue( m_habitat->GetRadiusMin().GetValueString() );
    m_maxRadius->SetValue( m_habitat->GetRadiusMax().GetValueString() );

    m_fillRuleWinding->SetValue( m_habitat->GetBooleanEngineWindingRule() );
    m_linkHoles->SetValue( m_habitat->GetBooleanEngineLinkHoles() );
    number.Printf( wxT( "%lg" ), m_habitat->GetBooleanEngineRoundfactor() );
    m_roundFactor->SetValue( number );
    long i = m_habitat->GetBooleanEngineGrid();
    number.Printf( wxT( "%ld" ), i );
    m_intersectionGrid->SetValue( number );

    m_units->SetStringSelection( unitstr );
}

/*!
 * Control creation for a2dSettings
 */

void a2dSettings::CreateControls()
{
////@begin a2dSettings content construction
    a2dSettings* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    m_settingsTabs = new wxNotebook( itemDialog1, ID_settings, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );
    m_settingsTabs->SetHelpText( _( "settings" ) );
    if ( a2dSettings::ShowToolTips() )
        m_settingsTabs->SetToolTip( _( "settings" ) );
    m_settingsTabs->SetName( _T( "Settings" ) );

    m_displaySet = new wxPanel( m_settingsTabs, ID_displaySettings, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxTAB_TRAVERSAL );
    m_displaySet->SetHelpText( _( "displaySettings" ) );
    if ( a2dSettings::ShowToolTips() )
        m_displaySet->SetToolTip( _( "displaySettings" ) );
    m_displaySet->SetName( _T( "displaySettings" ) );
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer( wxVERTICAL );
    m_displaySet->SetSizer( itemBoxSizer5 );

    m_unitsStrings.Add( wxT( "non" ) );
    m_unitsStrings.Add( wxT( "pm" ) );
    m_unitsStrings.Add( wxT( "nm" ) );
    m_unitsStrings.Add( wxT( "um" ) );
    m_unitsStrings.Add( wxT( "mm" ) );
    m_unitsStrings.Add( wxT( "cm" ) );
    m_unitsStrings.Add( wxT( "dm" ) );
    m_unitsStrings.Add( wxT( "m" ) );
    m_unitsStrings.Add( wxT( "mil" ) );
    m_unitsStrings.Add( wxT( "inch" ) );
    m_unitsStrings.Add( wxT( "foot" ) );

    m_units = new wxChoice( m_displaySet, ID_UNITS, wxDefaultPosition, wxSize( 60, -1 ), m_unitsStrings, 0 );
    m_units->SetStringSelection( _( "um" ) );
    m_units->SetHelpText( _( "Units used in document" ) );
    if ( a2dSettings::ShowToolTips() )
        m_units->SetToolTip( _( "Units used in document" ) );
    m_units->SetName( _T( "Units" ) );
    itemBoxSizer5->Add( m_units, 0, wxALIGN_LEFT | wxALL, 5 );

    wxBoxSizer* itemBoxSizer20 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer5->Add( itemBoxSizer20, 0, wxALIGN_LEFT | wxALL, 5 );
    wxStaticText* itemStaticText21 = new wxStaticText( m_displaySet, wxID_STATIC, _( "Grid X-Y" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText21->SetHelpText( _( "Canvas grid distance in X and Y in meters, e.g. 100 um" ) );
    if ( ShowToolTips() )
        itemStaticText21->SetToolTip( _( "Canvas grid distance in X and Y e.g. 100 um" ) );
    itemBoxSizer20->Add( itemStaticText21, 0, wxALIGN_CENTER_VERTICAL | wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 5 );
    m_gridDistX = new wxTextCtrl( m_displaySet, ID_SnapGrid_X, _T( "" ), wxDefaultPosition, wxSize( 60, -1 ), wxTE_PROCESS_ENTER );
    itemBoxSizer20->Add( m_gridDistX, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    m_gridDistY = new wxTextCtrl( m_displaySet, ID_SnapGrid_Y, _T( "" ), wxDefaultPosition, wxSize( 60, -1 ), wxTE_PROCESS_ENTER );
    itemBoxSizer20->Add( m_gridDistY, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer5->Add( itemBoxSizer6, 0, wxALIGN_LEFT | wxALL, 5 );
    m_displayAberation = new wxTextCtrl( m_displaySet, ID_display, _( "-1" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_displayAberation->SetName( _T( "displayAberation" ) );
    itemBoxSizer6->Add( m_displayAberation, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );
    wxStaticText* itemStaticText8 = new wxStaticText( m_displaySet, wxID_STATIC, _( "Display aberation" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add( itemStaticText8, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer5->Add( itemBoxSizer12, 0, wxALIGN_LEFT | wxALL, 2 );
    m_primThreshold = new wxTextCtrl( m_displaySet, ID_TEXTCTRL1, _( "-1" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_primThreshold->SetName( _T( "primThreshold" ) );
    itemBoxSizer12->Add( m_primThreshold, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText14 = new wxStaticText( m_displaySet, wxID_STATIC, _( "Primitive Threshold" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText14->SetHelpText( _( "Primitive Threshold below this number of pixels no display" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText14->SetToolTip( _( "Primitive Threshold below this number of pixels no display" ) );
    itemBoxSizer12->Add( itemStaticText14, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer5->Add( itemBoxSizer15, 0, wxALIGN_LEFT | wxALL, 5 );
    m_drawRectangle = new wxCheckBox( m_displaySet, ID_CHECKBOX1, _( "Draw Rectangle" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_drawRectangle->SetValue( true );
    m_drawRectangle->SetHelpText( _( "Draw Rectangle at threshold of object or primitive" ) );
    if ( a2dSettings::ShowToolTips() )
        m_drawRectangle->SetToolTip( _( "Draw Rectangle at threshold of object or primitive" ) );
    m_drawRectangle->SetName( _T( "drawRectangle" ) );
    itemBoxSizer15->Add( m_drawRectangle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer5->Add( itemBoxSizer17, 0, wxALIGN_LEFT | wxALL, 2 );
    m_selectionDistance = new wxTextCtrl( m_displaySet, ID_TEXTCTRL8, _( "0" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_selectionDistance->SetName( _T( "selectionDistance" ) );
    itemBoxSizer17->Add( m_selectionDistance, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxStaticText* itemStaticText19 = new wxStaticText( m_displaySet, wxID_STATIC, _( "Selection distance" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText19->SetHelpText( _( "Selection will hit if within this amount of pixels" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText19->SetToolTip( _( "Selection will hit if within this amount of pixels" ) );
    itemBoxSizer17->Add( itemStaticText19, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_drawOutline = new wxCheckBox( m_displaySet, ID_CHECKBOX4, _( "Draw Only Outlines" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_drawOutline->SetValue( false );
    m_drawOutline->SetHelpText( _( "Draw filled or outline" ) );
    if ( a2dSettings::ShowToolTips() )
        m_drawOutline->SetToolTip( _( "Draw filled or outline" ) );
    m_drawOutline->SetName( _T( "m_outline" ) );
    itemBoxSizer5->Add( m_drawOutline, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    m_settingsTabs->AddPage( m_displaySet, _( "Display" ) );

    m_booleanSet = new wxPanel( m_settingsTabs, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer( wxVERTICAL );
    m_booleanSet->SetSizer( itemBoxSizer22 );

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer22->Add( itemBoxSizer23, 0, wxALIGN_LEFT | wxALL, 5 );
    m_snapfactor = new wxTextCtrl( m_booleanSet, ID_TEXTCTRL2, _( "-1" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add( m_snapfactor, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText25 = new wxStaticText( m_booleanSet, wxID_STATIC, _( "Snap Factor" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText25->SetHelpText( _( "Point closer to Segments will be snapped to segment" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText25->SetToolTip( _( "Point closer to Segments will be snapped to segment" ) );
    itemBoxSizer23->Add( itemStaticText25, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer22->Add( itemBoxSizer26, 0, wxALIGN_LEFT | wxALL, 5 );
    m_offsetAber = new wxTextCtrl( m_booleanSet, ID_TEXTCTRL3, _( "-1" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add( m_offsetAber, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText28 = new wxStaticText( m_booleanSet, wxID_STATIC, _( "Offset Aberation" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText28->SetHelpText( _( "Correction/Offset arc segments, will deviate this maximal amount" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText28->SetToolTip( _( "Correction/Offset arc segments, will deviate this maximal amount" ) );
    itemBoxSizer26->Add( itemStaticText28, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer29 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer22->Add( itemBoxSizer29, 0, wxALIGN_LEFT | wxALL, 5 );
    m_roundFactor = new wxTextCtrl( m_booleanSet, ID_TEXTCTRL4, _( "1.5" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer29->Add( m_roundFactor, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText31 = new wxStaticText( m_booleanSet, wxID_STATIC, _( "Rounding Factor" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText31->SetHelpText( _( "Offset and path 2 polygon, will use this to define arc or angle at corners" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText31->SetToolTip( _( "Offset and path 2 polygon, will use this to define arc or angle at corners" ) );
    itemBoxSizer29->Add( itemStaticText31, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer32 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer22->Add( itemBoxSizer32, 0, wxALIGN_LEFT | wxALL, 5 );
    m_offset = new wxTextCtrl( m_booleanSet, ID_TEXTCTRL9, _( "0" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer32->Add( m_offset, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText34 = new wxStaticText( m_booleanSet, wxID_STATIC, _( "Offset Amount" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText34->SetHelpText( _( "How far will the offset be from original (neg also possible)" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText34->SetToolTip( _( "How far will the offset be from original (neg also possible)" ) );
    itemBoxSizer32->Add( itemStaticText34, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer35 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer22->Add( itemBoxSizer35, 0, wxALIGN_LEFT | wxALL, 5 );
    m_smoothAber = new wxTextCtrl( m_booleanSet, ID_TEXTCTRL5, _( "0" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer35->Add( m_smoothAber, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText37 = new wxStaticText( m_booleanSet, wxID_STATIC, _( "Smoothing abberation" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText37->SetHelpText( _( "when simplyfying polygons in boolean operation, this is maximal deviation" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText37->SetToolTip( _( "when simplyfying polygons in boolean operation, this is maximal deviation" ) );
    itemBoxSizer35->Add( itemStaticText37, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer38 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer22->Add( itemBoxSizer38, 0, wxALIGN_LEFT | wxALL, 5 );
    m_maxLineMerge = new wxTextCtrl( m_booleanSet, ID_TEXTCTRL6, _( "-1" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer38->Add( m_maxLineMerge, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText40 = new wxStaticText( m_booleanSet, wxID_STATIC, _( "Maximum Line Merge" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText40->SetHelpText( _( "lines to a lenght of this can be combined with other lines" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText40->SetToolTip( _( "lines to a lenght of this can be combined with other lines" ) );
    itemBoxSizer38->Add( itemStaticText40, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer41 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer22->Add( itemBoxSizer41, 0, wxALIGN_LEFT | wxALL, 5 );
    m_intersectionGrid = new wxTextCtrl( m_booleanSet, ID_TEXTCTRL7, _( "100" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer41->Add( m_intersectionGrid, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText43 = new wxStaticText( m_booleanSet, wxID_STATIC, _( "Grid for intersections" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText43->SetHelpText( _( "How accurate will intersections in boolean algorithm be" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText43->SetToolTip( _( "How accurate will intersections in boolean algorithm be" ) );
    itemBoxSizer41->Add( itemStaticText43, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer44 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer22->Add( itemBoxSizer44, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );
    m_fillRuleWinding = new wxCheckBox( m_booleanSet, ID_CHECKBOX2, _( "Winding Rule" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_fillRuleWinding->SetValue( false );
    m_fillRuleWinding->SetHelpText( _( "Winding Rule if set else alternate filling" ) );
    if ( a2dSettings::ShowToolTips() )
        m_fillRuleWinding->SetToolTip( _( "Winding Rule if set else alternate filling" ) );
    itemBoxSizer44->Add( m_fillRuleWinding, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    m_linkHoles = new wxCheckBox( m_booleanSet, ID_CHECKBOX3, _( "Link Holes" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_linkHoles->SetValue( false );
    m_linkHoles->SetHelpText( _( "connect holes in polygons to outer contour, result is polygon.\nOr use vector path as output." ) );
    if ( a2dSettings::ShowToolTips() )
        m_linkHoles->SetToolTip( _( "connect holes in polygons to outer contour, result is polygon.\nOr use vector path as output." ) );
    itemBoxSizer44->Add( m_linkHoles, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    m_settingsTabs->AddPage( m_booleanSet, _( "Boolean Operations" ) );

    m_arcpolySet = new wxPanel( m_settingsTabs, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer48 = new wxBoxSizer( wxVERTICAL );
    m_arcpolySet->SetSizer( itemBoxSizer48 );

    wxBoxSizer* itemBoxSizer49 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer48->Add( itemBoxSizer49, 0, wxALIGN_LEFT | wxALL, 5 );
    m_minRadius = new wxTextCtrl( m_arcpolySet, ID_TEXTCTRL10, _( "0" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer49->Add( m_minRadius, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText51 = new wxStaticText( m_arcpolySet, wxID_STATIC, _( "Minimum Radius" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText51->SetHelpText( _( "found arc segments with a radius above this will be converted from polygon" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText51->SetToolTip( _( "found arc segments with a radius above this will be converted from polygon" ) );
    itemBoxSizer49->Add( itemStaticText51, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer52 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer48->Add( itemBoxSizer52, 0, wxALIGN_LEFT | wxALL, 5 );
    m_maxRadius = new wxTextCtrl( m_arcpolySet, ID_TEXTCTRL12, _T( "" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer52->Add( m_maxRadius, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText54 = new wxStaticText( m_arcpolySet, wxID_STATIC, _( "Maximum Radius" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText54->SetHelpText( _( "found arc segments with a radius below this will be converted from polygon" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText54->SetToolTip( _( "found arc segments with a radius below this will be converted from polygon" ) );
    itemBoxSizer52->Add( itemStaticText54, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer55 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer48->Add( itemBoxSizer55, 0, wxALIGN_LEFT | wxALL, 5 );
    m_poly2arcAber = new wxTextCtrl( m_arcpolySet, ID_TEXTCTRL11, _T( "" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer55->Add( m_poly2arcAber, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText57 = new wxStaticText( m_arcpolySet, wxID_STATIC, _( "Conversion Aberration" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText57->SetHelpText( _( "polygon segments must be this close to a perfect arc" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText57->SetToolTip( _( "polygon segments must be this close to a perfect arc" ) );
    itemBoxSizer55->Add( itemStaticText57, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticLine* itemStaticLine58 = new wxStaticLine( m_arcpolySet, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer48->Add( itemStaticLine58, 0, wxGROW | wxALL, 5 );

    wxBoxSizer* itemBoxSizer59 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer48->Add( itemBoxSizer59, 0, wxALIGN_LEFT | wxALL, 5 );
    m_arc2polyAber = new wxTextCtrl( m_arcpolySet, ID_TEXTCTRL13, _T( "" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer59->Add( m_arc2polyAber, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxStaticText* itemStaticText61 = new wxStaticText( m_arcpolySet, wxID_STATIC, _( "Arc conversion aberration" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText61->SetHelpText( _( "polygon segments will be this close to a perfect arc after conversion" ) );
    if ( a2dSettings::ShowToolTips() )
        itemStaticText61->SetToolTip( _( "polygon segments will be this close to a perfect arc after conversion" ) );
    itemBoxSizer59->Add( itemStaticText61, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    m_settingsTabs->AddPage( m_arcpolySet, _( "arc<=>polygon conversion" ) );

    itemBoxSizer2->Add( m_settingsTabs, 1, wxGROW | wxALL, 5 );

    wxButton* itemButton62 = new wxButton( itemDialog1, ID_BUTTON1, _( "Apply" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton62->SetHelpText( _( "Apply settings in current tab" ) );
    if ( a2dSettings::ShowToolTips() )
        itemButton62->SetToolTip( _( "Apply settings in current tab" ) );
    itemBoxSizer2->Add( itemButton62, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 2 );

    // Connect events and objects
    itemButton62->Connect( ID_BUTTON1, wxEVT_LEFT_DOWN, wxMouseEventHandler( a2dSettings::OnLeftDown ), NULL, this );
////@end a2dSettings content construction
}


/*!
 * Should we show tooltips?
 */

bool a2dSettings::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap a2dSettings::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin a2dSettings bitmap retrieval
    wxUnusedVar( name );
    return wxNullBitmap;
////@end a2dSettings bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon a2dSettings::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin a2dSettings icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
////@end a2dSettings icon retrieval
}

/*
void a2dSettings::OnChangedDocument( a2dCommandEvent& event )
{
    a2dCanvasDocument* doc = ( a2dCanvasDocument* ) event.GetDocument();
    if ( doc )
    {
        UpdateEntries();
    }
}
*/

/*!
 * wxEVT_LEFT_DOWN event handler for ID_BUTTON1
 */

void a2dSettings::OnLeftDown( wxMouseEvent& event )
{
    wxString unitOfMeasure = m_unitsStrings[m_units->GetSelection()];

    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( drawingPart )
    {
        drawingPart->GetDrawing()->SetUnits( unitOfMeasure );
        double val;
        m_gridDistX->GetValue().ToDouble( &val );
        drawingPart->SetGridX( val );
        m_gridDistY->GetValue().ToDouble( &val );
        drawingPart->SetGridY( val );
        m_displayAberation->GetValue().ToDouble( &val );
        m_habitat->SetDisplayAberration( val );
        drawingPart->GetDrawer2D()->SetDisplayAberration( val );       
        if ( m_drawOutline->GetValue() )
            drawingPart->SetDocumentDrawStyle( RenderWIREFRAME_ZERO_WIDTH | RenderWIREFRAME_SELECT | RenderWIREFRAME_SELECT2 );
        else
            drawingPart->SetDocumentDrawStyle( RenderLAYERED | RenderWIREFRAME_SELECT | RenderWIREFRAME_SELECT2 );
        m_primThreshold->GetValue().ToDouble( &val );
        drawingPart->GetDrawer2D()->SetPrimitiveThreshold( val, m_drawRectangle->GetValue() );
        a2dGlobals->SetPrimitiveThreshold( val );

        m_selectionDistance->GetValue().ToDouble( &val );
        drawingPart->SetHitMarginDevice( val );
        m_habitat->SetHitMarginDevice( val );

    }

    m_habitat->SetUnits( unitOfMeasure );

    a2dDoMu snapfactor;
    snapfactor.Eval(  m_snapfactor->GetValue() );
    m_habitat->SetBooleanEngineMarge( snapfactor );

    a2dDoMu smoothAber;
    smoothAber.Eval(  m_smoothAber->GetValue() );
    m_habitat->SetBooleanEngineSmoothAber( smoothAber );

    m_habitat->SetBooleanEngineGrid( wxAtoi( m_intersectionGrid->GetValue() ) );

    a2dDoMu offsetAber;
    offsetAber.Eval(  m_offsetAber->GetValue() );
    m_habitat->SetBooleanEngineCorrectionAber( offsetAber );

    a2dDoMu offset;
    offset.Eval(  m_offset->GetValue() );
    m_habitat->SetBooleanEngineCorrectionFactor( offset );

    a2dDoMu maxLineMerge;
    maxLineMerge.Eval( m_maxLineMerge->GetValue() );
    m_habitat->SetBooleanEngineMaxlinemerge( maxLineMerge );

    a2dDoMu minRadius;
    minRadius.Eval( m_minRadius->GetValue() );
    m_habitat->SetRadiusMin( minRadius );

    a2dDoMu maxRadius;
    maxRadius.Eval( m_maxRadius->GetValue() );
    m_habitat->SetRadiusMax( maxRadius );

    a2dDoMu aberPolyToArc;
    aberPolyToArc.Eval( m_poly2arcAber->GetValue() );
    m_habitat->SetAberPolyToArc( aberPolyToArc );


    a2dDoMu aberArcToPoly;
    aberArcToPoly.Eval( m_arc2polyAber->GetValue() );
    m_habitat->SetAberArcToPoly( aberArcToPoly );

    m_habitat->SetBooleanEngineWindingRule( m_fillRuleWinding->GetValue() );

    m_habitat->SetBooleanEngineLinkHoles( m_linkHoles->GetValue() );
    m_habitat->SetBooleanEngineRoundfactor( wxAtoi( m_roundFactor->GetValue() ) );
    if ( drawingPart )
    {
        if ( m_drawOutline->GetValue() )
            drawingPart->SetDocumentDrawStyle( RenderWIREFRAME_ZERO_WIDTH | RenderWIREFRAME_SELECT | RenderWIREFRAME_SELECT2 );
        else
            drawingPart->SetDocumentDrawStyle( RenderLAYERED | RenderWIREFRAME_SELECT | RenderWIREFRAME_SELECT2 );
    }
}

void a2dSettings::OnUnitsSelected( wxCommandEvent& event )
{
    event.Skip();
}

/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_A2DSETTINGS
 */

void a2dSettings::OnCloseWindow( wxCloseEvent& event )
{
////@begin wxEVT_CLOSE_WINDOW event handler for ID_A2DSETTINGS in a2dSettings.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_CLOSE_WINDOW event handler for ID_A2DSETTINGS in a2dSettings.
}


#define ID_X 10001
#define ID_Y 10002
#define ID_UNITSDOC 10003
#define ID_oke 10004

BEGIN_EVENT_TABLE( a2dNewDocumentSize, wxDialog )
    EVT_TEXT_ENTER( ID_X, a2dNewDocumentSize::OnXEnter )
    EVT_TEXT_ENTER( ID_Y, a2dNewDocumentSize::OnYEnter )
    EVT_CHOICE( ID_UNITS, a2dNewDocumentSize::OnUnitsSelected )
    EVT_BUTTON( ID_oke, a2dNewDocumentSize::OnOkeClick )
END_EVENT_TABLE()

a2dNewDocumentSize::a2dNewDocumentSize()
{
    Init();
}

a2dNewDocumentSize::a2dNewDocumentSize( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}

bool a2dNewDocumentSize::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if ( GetSizer() )
    {
        GetSizer()->SetSizeHints( this );
    }
    Centre();
    return true;
}

a2dNewDocumentSize::~a2dNewDocumentSize()
{
}

void a2dNewDocumentSize::Init()
{
    m_x = NULL;
    m_y = NULL;
    m_units = NULL;
}

void a2dNewDocumentSize::CreateControls()
{
    a2dNewDocumentSize* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer3, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _( "Size in X" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add( itemStaticText4, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_x = new wxTextCtrl( itemDialog1, ID_X, _( "1000" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_x->SetHelpText( _( "Document size in X" ) );
    if ( a2dNewDocumentSize::ShowToolTips() )
        m_x->SetToolTip( _( "Document size in X" ) );
    itemBoxSizer3->Add( m_x, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer6, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _( "Size in Y" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add( itemStaticText7, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_y = new wxTextCtrl( itemDialog1, ID_Y, _( "1000" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_y->SetHelpText( _( "Document size in Y" ) );
    if ( a2dNewDocumentSize::ShowToolTips() )
        m_y->SetToolTip( _( "Document size in Y" ) );
    itemBoxSizer6->Add( m_y, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxStaticText* itemStaticText10 = new wxStaticText( itemDialog1, wxID_STATIC, _( "Units" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add( itemStaticText10, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_unitsStrings.Add( wxT( "non" ) );
    m_unitsStrings.Add( wxT( "pm" ) );
    m_unitsStrings.Add( wxT( "nm" ) );
    m_unitsStrings.Add( wxT( "um" ) );
    m_unitsStrings.Add( wxT( "mm" ) );
    m_unitsStrings.Add( wxT( "cm" ) );
    m_unitsStrings.Add( wxT( "dm" ) );
    m_unitsStrings.Add( wxT( "m" ) );
    m_unitsStrings.Add( wxT( "mil" ) );
    m_unitsStrings.Add( wxT( "inch" ) );
    m_unitsStrings.Add( wxT( "foot" ) );

    m_units = new wxChoice( itemDialog1, ID_UNITSDOC, wxDefaultPosition, wxDefaultSize, m_unitsStrings, 0 );
    m_units->SetStringSelection( _( "um" ) );
    m_units->SetHelpText( _( "Units used for document" ) );
    if ( a2dNewDocumentSize::ShowToolTips() )
        m_units->SetToolTip( _( "Units used for document" ) );
    itemBoxSizer9->Add( m_units, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxButton* itemButton12 = new wxButton( itemDialog1, ID_oke, _( "Oke" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemButton12, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

}

bool a2dNewDocumentSize::ShowToolTips()
{
    return true;
}

void a2dNewDocumentSize::OnXEnter( wxCommandEvent& event )
{
    event.Skip();
}

void a2dNewDocumentSize::OnYEnter( wxCommandEvent& event )
{
    event.Skip();
}

void a2dNewDocumentSize::OnUnitsSelected( wxCommandEvent& event )
{
    event.Skip();
}

void a2dNewDocumentSize::OnOkeClick( wxCommandEvent& event )
{
    wxString unitOfMeasure = m_unitsStrings[m_units->GetSelection()];

    EndModal( wxID_OK );
}

double a2dNewDocumentSize::GetSizeX() const
{
    a2dDoMu sizex;
    sizex.Eval( m_x->GetValue() );
    return sizex.GetNumber();
}

double a2dNewDocumentSize::GetSizeY() const
{
    a2dDoMu sizey;
    sizey.Eval( m_y->GetValue() );
    return sizey.GetNumber();
}

wxString a2dNewDocumentSize::GetUnit() const
{
    return m_unitsStrings[m_units->GetSelection()];
}
