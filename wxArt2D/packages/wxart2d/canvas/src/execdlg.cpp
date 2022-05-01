/*! \file canvas/src/execdlg.cpp
    \brief Document/view classes
    \author Klaas Holwerda
    \date Created 05/07/2003

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: execdlg.cpp,v 1.26 2009/09/26 19:01:06 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/clipbrd.h>

#include "wx/canvas/execdlg.h"

const int    CP_SEL = wxID_HIGHEST + 8801 ;
const int    CP_ALL = wxID_HIGHEST + 8802 ;

BEGIN_EVENT_TABLE( CopywxListBox, wxListBox )
    EVT_RIGHT_DOWN( CopywxListBox::OnMouseRightDown )
    EVT_MENU( CP_SEL, CopywxListBox::CopySel )
    EVT_MENU( CP_ALL, CopywxListBox::CopyAll )
END_EVENT_TABLE()

CopywxListBox::CopywxListBox( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                              int n, const wxString choices[], long style )
    : wxListBox   ( parent, id, pos, size, n, choices, style )
{
    // make a popupmenu for the right mousebutton
    m_mousemenu = new wxMenu( _T( "Copy" ) , ( long )0 );
    m_mousemenu->Append( CP_SEL, _T( "selected" ) );
    m_mousemenu->Append( CP_ALL, _T( "all" ) );
}

void CopywxListBox::CopySel( class wxCommandEvent& WXUNUSED( event ) )
{
    if ( wxTheClipboard->Open() )
    {
        wxString data;
        data = data + GetStringSelection() + _T( "\n" );
        wxTheClipboard->SetData( new wxTextDataObject( data ) );
        wxTheClipboard->Close();
    }
}

void CopywxListBox::CopyAll( class wxCommandEvent& WXUNUSED( event ) )
{
    if ( wxTheClipboard->Open() )
    {
        wxString data;
        int i;
        for ( i = 0; i < GetCount(); i++ )
        {
            data = data + GetString( i ) + _T( "\n" );
        }
        wxTheClipboard->SetData( new wxTextDataObject( data ) );
        wxTheClipboard->Close();
    }
}

void CopywxListBox::OnMouseRightDown( class wxMouseEvent& WXUNUSED( event ) )
{
    PopupMenu( m_mousemenu, 0, 0 );
}



/*!
 * a2dCoordinateEntry type definition
 */

IMPLEMENT_DYNAMIC_CLASS( a2dCoordinateEntry, wxDialog )

/*!
 * a2dCoordinateEntry event table definition
 */

BEGIN_EVENT_TABLE( a2dCoordinateEntry, wxDialog )

    EVT_TEXT( IDentry_X,  a2dCoordinateEntry::OnEntryXEnter )
    EVT_TEXT( IDentry_Y,  a2dCoordinateEntry::OnEntryYEnter )

    EVT_TEXT_ENTER( IDentry_X, a2dCoordinateEntry::OnEntryXEnter )

    EVT_TEXT_ENTER( IDentry_Y, a2dCoordinateEntry::OnEntryYEnter )

    EVT_TEXT_ENTER( IDentry_CalcCoord, a2dCoordinateEntry::OnEntryCalccoordEnter )

    EVT_RADIOBOX( IDentry_CartPolar, a2dCoordinateEntry::OnEntryCartpolarSelected )

    EVT_RADIOBOX( IDentry_AbsRel, a2dCoordinateEntry::OnEntryAbsrelSelected )

    EVT_CHECKBOX( IDentry_Snap, a2dCoordinateEntry::OnEntrySnapClick )

    EVT_BUTTON( IDentry_Hide, a2dCoordinateEntry::OnEntryHideClick )

    EVT_BUTTON( IDentry_Apply, a2dCoordinateEntry::OnEntryApplyClick )

    EVT_BUTTON( IDentry_Cancel, a2dCoordinateEntry::OnEntryCancelClick )

END_EVENT_TABLE()

/*!
 * a2dCoordinateEntry constructors
 */

a2dCoordinateEntry::a2dCoordinateEntry( )
{
}

a2dCoordinateEntry::a2dCoordinateEntry( wxWindow* parent, wxWindowID id, bool modal, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_modal = modal;
    Create( parent, id, caption, pos, size, style );
}

/*!
 * a2dCoordinateEntry creator
 */

bool a2dCoordinateEntry::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_x = NULL;
    m_y = NULL;
    m_calcCoord = NULL;
    m_cartPolar = NULL;
    m_absRel = NULL;
    m_snap = NULL;
    m_hide = NULL;
    m_apply = NULL;
    m_cancel = NULL;

    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls( style );
    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
    Centre();
    return TRUE;
}

/*!
 * Control creation for a2dCoordinateEntry
 */

void a2dCoordinateEntry::CreateControls( long style )
{
    a2dCoordinateEntry* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer3, 0, wxGROW | wxALL, 1 );

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox( itemDialog1, wxID_ANY, _( "Coordinates" ) );
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer( itemStaticBoxSizer4Static, wxVERTICAL );
    itemBoxSizer3->Add( itemStaticBoxSizer4, 1, wxGROW | wxALL, 0 );

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer( wxHORIZONTAL );
    itemStaticBoxSizer4->Add( itemBoxSizer5, 0, wxGROW | wxALL, 5 );

    m_xText = new wxStaticText( itemDialog1, wxID_STATIC, _( "X" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add( m_xText, 0, wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 5 );

    m_x = new wxTextCtrl( itemDialog1, IDentry_X, _T( "" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add( m_x, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_yText = new wxStaticText( itemDialog1, wxID_STATIC, _( "Y" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add( m_yText, 0, wxALIGN_CENTER_VERTICAL | wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 5 );

    m_y = new wxTextCtrl( itemDialog1, IDentry_Y, _T( "" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add( m_y, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer( wxHORIZONTAL );
    itemStaticBoxSizer4->Add( itemBoxSizer10, 0, wxGROW | wxALL, 5 );

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _( "Calc Coord" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add( itemStaticText11, 0, wxALIGN_CENTER_VERTICAL | wxALL/* wxWidgets 2.9 deprecated version |wxADJUST_MINISIZE*/, 5 );

    m_calcCoord = new wxTextCtrl( itemDialog1, IDentry_CalcCoord, _T( "" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add( m_calcCoord, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer3->Add( itemBoxSizer13, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxString m_cartPolarStrings[] =
    {
        _( "&Cart" ),
        _( "&Polar" )
    };
    m_cartPolar = new wxRadioBox( itemDialog1, IDentry_CartPolar, _( "Cart-Polar" ), wxDefaultPosition, wxDefaultSize, 2, m_cartPolarStrings, 1, wxRA_SPECIFY_ROWS );
    itemBoxSizer13->Add( m_cartPolar, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1 );

    wxString m_absRelStrings[] =
    {
        _( "&Abs" ),
        _( "&Rel" )
    };
    m_absRel = new wxRadioBox( itemDialog1, IDentry_AbsRel, _( "Abs-Rel" ), wxDefaultPosition, wxDefaultSize, 2, m_absRelStrings, 1, wxRA_SPECIFY_ROWS );
    itemBoxSizer13->Add( m_absRel, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1 );

    m_snap = new wxCheckBox( itemDialog1, IDentry_Snap, _( "Use Snapping" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_snap->SetValue( FALSE );
    itemBoxSizer13->Add( m_snap, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer17, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

    if ( m_modal )
    {
        m_apply = new wxButton( itemDialog1, IDentry_Apply, _( "Oke" ), wxDefaultPosition, wxDefaultSize, 0 );
    }
    else
    {
        m_hide = new wxButton( itemDialog1, IDentry_Hide, _( "Hide" ), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer17->Add( m_hide, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0 );

        m_apply = new wxButton( itemDialog1, IDentry_Apply, _( "Apply" ), wxDefaultPosition, wxDefaultSize, 0 );
    }
    itemBoxSizer17->Add( m_apply, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0 );

    m_cancel = new wxButton( itemDialog1, IDentry_Cancel, _( "Cancel" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add( m_cancel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0 );

}

/*!
 * Should we show tooltips?
 */

bool a2dCoordinateEntry::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap a2dCoordinateEntry::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon a2dCoordinateEntry::GetIconResource( const wxString& name )
{
    // Icon retrieval
    return wxNullIcon;
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for IDentry_TEXTCTRL
 */

void a2dCoordinateEntry::OnEntryXEnter( wxCommandEvent& event )
{
    // Before editing this code, remove the block markers.
    CalcCoordinate();
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for IDentry_TEXTCTRL1
 */

void a2dCoordinateEntry::OnEntryYEnter( wxCommandEvent& event )
{
    CalcCoordinate();
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for IDentry_TEXTCTRL2
 */

void a2dCoordinateEntry::OnEntryCalccoordEnter( wxCommandEvent& event )
{
    CalcCoordinate();
}

/*!
 * wxEVT_COMMAND_RADIOBOX_SELECTED event handler for IDentry_RADIOBOX
 */

void a2dCoordinateEntry::OnEntryCartpolarSelected( wxCommandEvent& event )
{
    if ( m_cartPolar->GetSelection() == 0 )
    {
        m_xText->SetLabel( _T( "X" ) );
        m_yText->SetLabel( _T( "Y" ) );
    }
    else
    {
        m_xText->SetLabel( _T( "Ang" ) );
        m_yText->SetLabel( _T( "Radius" ) );
    }
    GetSizer()->Fit( this );
    CalcCoordinate();
}

/*!
 * wxEVT_COMMAND_RADIOBOX_SELECTED event handler for IDentry_RADIOBOX1
 */

void a2dCoordinateEntry::OnEntryAbsrelSelected( wxCommandEvent& event )
{
    CalcCoordinate();
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for IDentry_CHECKBOX
 */

void a2dCoordinateEntry::OnEntrySnapClick( wxCommandEvent& event )
{
    CalcCoordinate();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for IDentry_BUTTON
 */

void a2dCoordinateEntry::OnEntryHideClick( wxCommandEvent& event )
{
    SetReturnCode( wxID_OK );
    Show( false );
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for IDentry_BUTTON1
 */

void a2dCoordinateEntry::OnEntryApplyClick( wxCommandEvent& event )
{
    if ( m_modal )
    {
        CalcCoordinate();
        EndModal( wxID_OK );
    }
    else
    {
        CalcCoordinate();
        bool polar = m_cartPolar->GetSelection() == 1;
        bool relative = m_absRel->GetSelection() == 1;

        a2dSmrtPtr<a2dCommand_SetCursor> command = new a2dCommand_SetCursor(
            a2dCommand_SetCursor::Args().
            x( m_xcalc ).
            y( m_ycalc ).
            relative( relative ).
            polar( polar ).
            snapped( m_snap->GetValue() ) );

        a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawingPart )
            return;

        drawingPart->GetDrawing()->GetCommandProcessor()->Submit( command );
    }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for IDentry_BUTTON2
 */

void a2dCoordinateEntry::OnEntryCancelClick( wxCommandEvent& event )
{
    Show( false );
}

void a2dCoordinateEntry::CalcCoordinate()
{
    m_xcalc = wxAtoi( m_x->GetValue() );
    m_ycalc = wxAtoi( m_y->GetValue() );
    bool polar = m_cartPolar->GetSelection() == 1;
    bool relative = m_absRel->GetSelection() == 1;

    //get current position
    double xold = a2dCanvasGlobals->GetHabitat()->GetLastXEntry();
    double yold = a2dCanvasGlobals->GetHabitat()->GetLastYEntry();

    //get current position
    if ( relative )
    {
        if ( polar )
        {
            double radius = m_ycalc;
            double ang = m_xcalc;

            m_xcalc = xold + radius * cos( wxDegToRad( ang ) );
            m_ycalc = yold + radius * sin( wxDegToRad( ang ) );
        }
        else
        {
            m_xcalc += xold;
            m_ycalc += yold;
        }
    }
    else
    {
        if ( polar )
        {
            double radius = m_ycalc;
            double ang = m_xcalc;

            m_xcalc = radius * cos( wxDegToRad( ang ) );
            m_ycalc = radius * sin( wxDegToRad( ang ) );
        }
    }

    wxString calc;
    calc.Printf( wxT( " x = %lg, y = %lg" ), m_xcalc, m_ycalc );
    m_calcCoord->SetValue( calc );
}




