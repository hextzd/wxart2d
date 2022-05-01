/////////////////////////////////////////////////////////////////////////////
// Name:        a2dPathSettings.cpp
// Purpose:
// Author:      Klaas Holwerda
// Modified by:
// Created:     10/04/2008 17:02:58
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

#include "wx/canvas/pathsettings.h"
#include "wx/canvas/edit.h"
#include "wx/canvas/canmod.h"

#define ID_WIDTH 10002
#define ID_UNITS 10005
#define ID_Rounded 10001
#define ID_Rectangle 10003
#define ID_RectangleExt 10004
#define ID_HIDE 10006
#define ID_APPLY 10007

////@begin XPM images
////@end XPM images


/*!
 * a2dPathSettings type definition
 */

IMPLEMENT_DYNAMIC_CLASS( a2dPathSettings, wxDialog )


/*!
 * a2dPathSettings event table definition
 */

BEGIN_EVENT_TABLE( a2dPathSettings, wxDialog )
    EVT_COM_EVENT( a2dPathSettings::OnComEvent )
    EVT_TEXT_ENTER( ID_WIDTH, a2dPathSettings::OnWidthEnter )
    EVT_CHOICE( ID_UNITS, a2dPathSettings::OnUnitsSelected )
    EVT_RADIOBUTTON( ID_Rounded, a2dPathSettings::OnRoundedSelected )
    EVT_RADIOBUTTON( ID_Rectangle, a2dPathSettings::OnRectangleSelected )
    EVT_RADIOBUTTON( ID_RectangleExt, a2dPathSettings::OnRectangleExtSelected )
    EVT_BUTTON( ID_HIDE, a2dPathSettings::OnHideClick )
    EVT_BUTTON( ID_APPLY, a2dPathSettings::OnApplyClick )
END_EVENT_TABLE()


/*!
 * a2dPathSettings constructors
 */

a2dPathSettings::a2dPathSettings()
{
    Init();
}

a2dPathSettings::a2dPathSettings( a2dHabitat* habitat, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_habitat = habitat;
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*!
 * a2dPathSettings creator
 */

bool a2dPathSettings::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin a2dPathSettings creation
    SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if ( GetSizer() )
    {
        GetSizer()->SetSizeHints( this );
    }
    Centre();
////@end a2dPathSettings creation
    return true;
}


/*!
 * a2dPathSettings destructor
 */

a2dPathSettings::~a2dPathSettings()
{
}


/*!
 * Member initialisation
 */

void a2dPathSettings::Init()
{
    m_width = NULL;
    m_units = NULL;
    m_rounded = NULL;
    m_rectangular = NULL;
    m_extRectangular = NULL;
    m_hide = NULL;
    m_apply = NULL;
    m_pathtype = m_habitat->GetPathType();
    m_contourWidth = m_habitat->GetContourWidth();
}


/*!
 * Control creation for a2dPathSettings
 */

void a2dPathSettings::CreateControls()
{
    a2dPathSettings* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer3, 1, wxGROW | wxALL, 2 );

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _( "Width" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText4->SetHelpText( _( "width of polylines and lines" ) );
    if ( a2dPathSettings::ShowToolTips() )
        itemStaticText4->SetToolTip( _( "width of polylines and lines" ) );
    itemBoxSizer3->Add( itemStaticText4, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_width = new wxTextCtrl( itemDialog1, ID_WIDTH, _( "0" ), wxDefaultPosition, wxSize( 100, -1 ), wxTE_PROCESS_ENTER );
    m_width->SetHelpText( _( "Width of polyline" ) );
    if ( a2dPathSettings::ShowToolTips() )
        m_width->SetToolTip( _( "Width of polyline" ) );
    m_width->SetName( _T( "width" ) );
    itemBoxSizer3->Add( m_width, 2, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

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

    m_units = new wxChoice( itemDialog1, ID_UNITS, wxDefaultPosition, wxSize( 40, -1 ), m_unitsStrings, 0 );
    m_units->SetStringSelection( _( "um" ) );
    m_units->SetHelpText( _( "Units in which width is given" ) );
    if ( a2dPathSettings::ShowToolTips() )
        m_units->SetToolTip( _( "Units in which width is given" ) );
    m_units->SetName( _T( "Units" ) );
    itemBoxSizer3->Add( m_units, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    m_rounded = new wxRadioButton( itemDialog1, ID_Rounded, _( "Rounded" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_rounded->SetValue( true );
    m_rounded->SetHelpText( _( "end of lines will be rounded" ) );
    if ( a2dPathSettings::ShowToolTips() )
        m_rounded->SetToolTip( _( "end of lines will be rounded" ) );
    m_rounded->SetName( _T( "rounded" ) );
    itemBoxSizer2->Add( m_rounded, 0, wxALIGN_LEFT | wxALL, 2 );

    m_rectangular = new wxRadioButton( itemDialog1, ID_Rectangle, _( "Rectangular" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_rectangular->SetValue( false );
    m_rectangular->SetHelpText( _( "end of lines will be rectangular" ) );
    if ( a2dPathSettings::ShowToolTips() )
        m_rectangular->SetToolTip( _( "end of lines will be rectangular" ) );
    m_rectangular->SetName( _T( "rectangular" ) );
    itemBoxSizer2->Add( m_rectangular, 0, wxALIGN_LEFT | wxALL, 2 );

    m_extRectangular = new wxRadioButton( itemDialog1, ID_RectangleExt, _( "Extended Rectangular" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_extRectangular->SetValue( false );
    m_extRectangular->SetHelpText( _( "end of lines will be extended rectangular" ) );
    if ( a2dPathSettings::ShowToolTips() )
        m_extRectangular->SetToolTip( _( "end of lines will be extended rectangular" ) );
    m_extRectangular->SetName( _T( "extended" ) );
    itemBoxSizer2->Add( m_extRectangular, 0, wxALIGN_LEFT | wxALL, 2 );

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer10, 1, wxGROW | wxALL, 2 );

    m_hide = new wxButton( itemDialog1, ID_HIDE, _( "Hide" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_hide->SetHelpText( _( "hide and NOT execute commands" ) );
    if ( a2dPathSettings::ShowToolTips() )
        m_hide->SetToolTip( _( "hide and NOT execute commands" ) );
    m_hide->SetName( _T( "hide" ) );
    itemBoxSizer10->Add( m_hide, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    m_apply = new wxButton( itemDialog1, ID_APPLY, _( "Apply" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_apply->SetHelpText( _( "execute commands" ) );
    if ( a2dPathSettings::ShowToolTips() )
        m_apply->SetToolTip( _( "execute commands" ) );
    m_apply->SetName( _T( "apply" ) );
    itemBoxSizer10->Add( m_apply, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

}


/*!
 * Should we show tooltips?
 */

bool a2dPathSettings::ShowToolTips()
{
    return true;
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_WIDTH
 */

void a2dPathSettings::OnWidthEnter( wxCommandEvent& event )
{
    double value;
    m_width->GetValue().ToDouble( &value );
    m_contourWidth = a2dDoMu( value, m_unitsStrings[m_units->GetSelection()] );
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_UNITS
 */
void a2dPathSettings::OnUnitsSelected( wxCommandEvent& event )
{
    event.Skip();
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_Rounded
 */

void a2dPathSettings::OnRoundedSelected( wxCommandEvent& event )
{
    m_pathtype = a2dPATH_END_ROUND;
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_Rectangle
 */

void a2dPathSettings::OnRectangleSelected( wxCommandEvent& event )
{
    m_pathtype = a2dPATH_END_SQAURE;
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RectangleExt
 */

void a2dPathSettings::OnRectangleExtSelected( wxCommandEvent& event )
{
    m_pathtype = a2dPATH_END_SQAURE_EXT;
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_HIDE
 */

void a2dPathSettings::OnHideClick( wxCommandEvent& event )
{
    Show( false );
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_APPLY
 */

void a2dPathSettings::OnApplyClick( wxCommandEvent& event )
{
    double value;
    m_width->GetValue().ToDouble( &value );
    m_contourWidth = a2dDoMu( value, m_unitsStrings[m_units->GetSelection()] );

    m_habitat->SetContourWidth( m_contourWidth );
    m_habitat->SetPathType( m_pathtype );
}

void a2dPathSettings::OnComEvent( a2dComEvent& event )
{
    bool baseContinue = true;
    if ( event.GetId() == a2dRecursiveEditTool::sig_toolStartEditObject )
    {
        a2dObjectEditTool* editTool = wxDynamicCast( event.GetEventObject(), a2dObjectEditTool );
        if ( editTool )
        {
            m_contourWidth = editTool->GetContourWidth();
            m_width->SetValue( m_contourWidth.GetNumberString() ); // / a2dGetCmdh()->GetCanvasDocument()->GetUnitsScale() );
            m_units->SetStringSelection( m_contourWidth.GetMultiplierString() );

            a2dPolylineL* poly = wxDynamicCast( editTool->GetOriginal(), a2dPolylineL );
            if ( poly )
                m_pathtype = poly->GetPathType();
            a2dSLine* line = wxDynamicCast( editTool->GetOriginal(), a2dSLine );
            if ( line )
                m_pathtype = line->GetPathType();

            switch ( m_pathtype )
            {
                case a2dPATH_END_ROUND : m_rounded->SetValue( true ); break;
                case a2dPATH_END_SQAURE : m_rectangular->SetValue( true ); break;
                case a2dPATH_END_SQAURE_EXT : m_extRectangular->SetValue( true ); break;
            }
        }
    }
}
