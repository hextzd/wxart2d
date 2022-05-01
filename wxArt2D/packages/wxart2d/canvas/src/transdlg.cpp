/////////////////////////////////////////////////////////////////////////////
// Name:        transform.cpp
// Purpose:
// Author:      Klaas Holwerda
// Modified by:
// Created:     15/04/2008 10:44:52
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

#include "wx/canvas/transdlg.h"
#include "wx/canvas/canmod.h"

#define ID_x 10002
#define ID_y 10003
#define ID_scalex 10006
#define ID_scaley 10004
#define ID_TEXTCTRL3 10013
#define ID_vertical 10010
#define ID_horizontal 10009
#define ID_rotateangle 10014
#define ID_clockwise 10011
#define ID_counterclockwise 10012
#define ID_hide 10015
#define ID_apply 10016

/*!
 * transform type definition
 */

IMPLEMENT_DYNAMIC_CLASS( a2dTransDlg, wxDialog )

/*!
 * transform event table definition
 */
BEGIN_EVENT_TABLE( a2dTransDlg, wxDialog )
    EVT_BUTTON( ID_hide, a2dTransDlg::OnHideClick )
    EVT_BUTTON( ID_apply, a2dTransDlg::OnApplyClick )
END_EVENT_TABLE()


/*!
 * transform constructors
 */
a2dTransDlg::a2dTransDlg()
{
    Init();
}

a2dTransDlg::a2dTransDlg( wxWindow* parent, bool modal, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_modal = modal;
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*!
 * transform creator
 */
bool a2dTransDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
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


/*!
 * transform destructor
 */
a2dTransDlg::~a2dTransDlg()
{
}


/*!
 * Member initialisation
 */
void a2dTransDlg::Init()
{
    m_x = NULL;
    m_y = NULL;
    m_scalex = NULL;
    m_scaley = NULL;
    m_vertical = NULL;
    m_horizontal = NULL;
    m_rotateangle = NULL;
    m_clockwise = NULL;
    m_counterclockwise = NULL;
    m_hide = NULL;
    m_apply = NULL;
}

void a2dTransDlg::CreateControls()
{
    a2dTransDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox( itemDialog1, wxID_ANY, _( "Move (x,y)" ) );
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer( itemStaticBoxSizer3Static, wxVERTICAL );
    itemBoxSizer2->Add( itemStaticBoxSizer3, 0, wxALIGN_LEFT | wxALL, 2 );

    m_x = new wxTextCtrl( itemDialog1, ID_x, _( "0" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_x->SetHelpText( _( "Move by x" ) );
    if ( a2dTransDlg::ShowToolTips() )
        m_x->SetToolTip( _( "Move by x" ) );
    m_x->SetName( _T( "x" ) );
    itemStaticBoxSizer3->Add( m_x, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 2 );

    m_y = new wxTextCtrl( itemDialog1, ID_y, _( "0" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_y->SetHelpText( _( "Move by y" ) );
    if ( a2dTransDlg::ShowToolTips() )
        m_y->SetToolTip( _( "Move by y" ) );
    m_y->SetName( _T( "y" ) );
    itemStaticBoxSizer3->Add( m_y, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 2 );

    wxStaticBox* itemStaticBoxSizer6Static = new wxStaticBox( itemDialog1, wxID_ANY, _( "Scale (x,y)" ) );
    wxStaticBoxSizer* itemStaticBoxSizer6 = new wxStaticBoxSizer( itemStaticBoxSizer6Static, wxVERTICAL );
    itemBoxSizer2->Add( itemStaticBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    m_scalex = new wxTextCtrl( itemDialog1, ID_scalex, _( "1" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_scalex->SetHelpText( _( "Scale in x with this value" ) );
    if ( a2dTransDlg::ShowToolTips() )
        m_scalex->SetToolTip( _( "Scale in x with this value" ) );
    m_scalex->SetName( _T( "scalex" ) );
    itemStaticBoxSizer6->Add( m_scalex, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 2 );

    m_scaley = new wxTextCtrl( itemDialog1, ID_scaley, _( "1" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_scaley->SetHelpText( _( "Scale in y with this value" ) );
    if ( a2dTransDlg::ShowToolTips() )
        m_scaley->SetToolTip( _( "Scale in y with this value" ) );
    m_scaley->SetName( _T( "scaley" ) );
    itemStaticBoxSizer6->Add( m_scaley, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 2 );

    wxStaticBox* itemStaticBoxSizer9Static = new wxStaticBox( itemDialog1, wxID_ANY, _( "Mirror" ) );
    wxStaticBoxSizer* itemStaticBoxSizer9 = new wxStaticBoxSizer( itemStaticBoxSizer9Static, wxVERTICAL );
    itemBoxSizer2->Add( itemStaticBoxSizer9, 0, wxALIGN_LEFT | wxALL, 2 );

    m_vertical = new wxCheckBox( itemDialog1, ID_vertical, _( "Vertical" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_vertical->SetValue( false );
    m_vertical->SetHelpText( _( "Mirror on vertical axis" ) );
    if ( a2dTransDlg::ShowToolTips() )
        m_vertical->SetToolTip( _( "Mirror on vertical axis" ) );
    m_vertical->SetName( _T( "vertical" ) );
    itemStaticBoxSizer9->Add( m_vertical, 0, wxALIGN_LEFT | wxALL, 2 );

    m_horizontal = new wxCheckBox( itemDialog1, ID_horizontal, _( "Horizontal" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_horizontal->SetValue( false );
    m_horizontal->SetHelpText( _( "Mirror the object on horizontal" ) );
    if ( a2dTransDlg::ShowToolTips() )
        m_horizontal->SetToolTip( _( "Mirror the object on horizontal" ) );
    m_horizontal->SetName( _T( "horizontal" ) );
    itemStaticBoxSizer9->Add( m_horizontal, 0, wxALIGN_LEFT | wxALL, 2 );

    wxStaticBox* itemStaticBoxSizer12Static = new wxStaticBox( itemDialog1, wxID_ANY, _( "Rotate" ) );
    wxStaticBoxSizer* itemStaticBoxSizer12 = new wxStaticBoxSizer( itemStaticBoxSizer12Static, wxVERTICAL );
    itemBoxSizer2->Add( itemStaticBoxSizer12, 0, wxALIGN_LEFT | wxALL, 2 );

    m_rotateangle = new wxTextCtrl( itemDialog1, ID_rotateangle, _( "0" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_rotateangle->SetHelpText( _( "Rotate object by this angle" ) );
    if ( a2dTransDlg::ShowToolTips() )
        m_rotateangle->SetToolTip( _( "Rotate object by this angle" ) );
    m_rotateangle->SetName( _T( "rotateangle" ) );
    itemStaticBoxSizer12->Add( m_rotateangle, 0, wxALIGN_LEFT | wxALL, 2 );

    m_clockwise = new wxRadioButton( itemDialog1, ID_clockwise, _( "Clockwise" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_clockwise->SetValue( true );
    m_clockwise->SetHelpText( _( "Rotate clockwise" ) );
    if ( a2dTransDlg::ShowToolTips() )
        m_clockwise->SetToolTip( _( "Rotate clockwise" ) );
    m_clockwise->SetName( _T( "clockwise" ) );
    itemStaticBoxSizer12->Add( m_clockwise, 0, wxALIGN_LEFT | wxALL, 2 );

    m_counterclockwise = new wxRadioButton( itemDialog1, ID_counterclockwise, _( "Counter clockwise" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_counterclockwise->SetValue( false );
    m_counterclockwise->SetHelpText( _( "Rotate clockwise" ) );
    if ( a2dTransDlg::ShowToolTips() )
        m_counterclockwise->SetToolTip( _( "Rotate clockwise" ) );
    m_counterclockwise->SetName( _T( "counterclockwise" ) );
    itemStaticBoxSizer12->Add( m_counterclockwise, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer16, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    if ( !m_modal )
    {
        m_hide = new wxButton( itemDialog1, ID_hide, _( "Hide" ), wxDefaultPosition, wxSize( 50, -1 ), 0 );
        m_hide->SetHelpText( _( "Hide this dialog" ) );
        if ( a2dTransDlg::ShowToolTips() )
            m_hide->SetToolTip( _( "Hide this dialog" ) );
        m_hide->SetName( _T( "hide" ) );
        itemBoxSizer16->Add( m_hide, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

        m_apply = new wxButton( itemDialog1, ID_apply, _( "Apply" ), wxDefaultPosition, wxSize( 50, -1 ), 0 );
        m_apply->SetHelpText( _( "Apply transform" ) );
        if ( a2dTransDlg::ShowToolTips() )
            m_apply->SetToolTip( _( "Apply transform" ) );
        m_apply->SetName( _T( "apply" ) );
        itemBoxSizer16->Add( m_apply, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );
    }
    else
    {
        m_hide = new wxButton( itemDialog1, ID_hide, _( "Cancel" ), wxDefaultPosition, wxSize( 50, -1 ), 0 );
        m_hide->SetHelpText( _( "Cancel this action" ) );
        if ( a2dTransDlg::ShowToolTips() )
            m_hide->SetToolTip( _( "Cancel this dialog" ) );
        m_hide->SetName( _T( "hide" ) );

        itemBoxSizer16->Add( m_hide, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );
        m_apply = new wxButton( itemDialog1, ID_apply, _( "Oke" ), wxDefaultPosition, wxSize( 50, -1 ), 0 );
        m_apply->SetHelpText( _( "Apply transform" ) );
        if ( a2dTransDlg::ShowToolTips() )
            m_apply->SetToolTip( _( "Apply transform" ) );
        m_apply->SetName( _T( "apply" ) );
        itemBoxSizer16->Add( m_apply, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );
    }
}

/*!
 * Should we show tooltips?
 */
bool a2dTransDlg::ShowToolTips()
{
    return true;
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_hide
 */
void a2dTransDlg::OnHideClick( wxCommandEvent& event )
{
    if ( m_modal )
    {
        EndModal( wxID_CANCEL );
    }
    else
        Show( false );
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_apply
 */
void a2dTransDlg::OnApplyClick( wxCommandEvent& event )
{
    m_lworld = a2dIDENTITY_MATRIX;
    m_lworld.Mirror( m_vertical->GetValue(), m_horizontal->GetValue() );

    double x = 0;
    m_x->GetValue().ToDouble( &x );
    double y = 0;
    m_y->GetValue().ToDouble( &y );
    m_lworld.Translate( x, y );

    double rotate = 0;
    m_rotateangle->GetValue().ToDouble( &rotate );
    if ( m_clockwise->GetValue() )
        rotate = -rotate;
    m_lworld = m_lworld.Rotate( rotate, 0, 0 );

    double scalex = 1;
    m_scalex->GetValue().ToDouble( &scalex );
    double scaley = 1;
    m_scaley->GetValue().ToDouble( &scaley );
    m_lworld.Scale( scalex, scaley, x, y );

    if ( m_modal )
    {
        EndModal( wxID_OK );
    }
    else
    {
        a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawingPart )
            return;

        a2dCanvasObject* top = drawingPart->GetShowObject();

        a2dCommand_TransformMask* com = new a2dCommand_TransformMask( top, m_lworld );
        drawingPart->GetDrawing()->GetCommandProcessor()->Submit( com );
    }
}

