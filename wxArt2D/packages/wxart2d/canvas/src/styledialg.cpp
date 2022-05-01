/*! \file editor/src/styledialg.cpp
    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: styledialg.cpp,v 1.60 2009/09/10 17:04:09 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private header
#include "wx/canvas/styledialg.h"
#include "wx/canvas/edit.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/polygon.h"
#include "wx/artbase/dcdrawer.h"

#include <wx/gbsizer.h>
#include <wx/odcombo.h>
#include <wx/tglbtn.h>

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

BEGIN_EVENT_TABLE( a2dStyleDialog, wxDialog )

    EVT_SPINCTRL( ID_RED, a2dStyleDialog::SetColor )
    EVT_SPINCTRL( ID_GREEN, a2dStyleDialog::SetColor )
    EVT_SPINCTRL( ID_BLUE, a2dStyleDialog::SetColor )
    EVT_SPINCTRL( ID_ALPHA, a2dStyleDialog::SetColor )
    EVT_TEXT    ( ID_RED, a2dStyleDialog::OnSpinCtrlText )
    EVT_TEXT    ( ID_GREEN, a2dStyleDialog::OnSpinCtrlText )
    EVT_TEXT    ( ID_BLUE, a2dStyleDialog::OnSpinCtrlText )
    EVT_TEXT    ( ID_ALPHA, a2dStyleDialog::OnSpinCtrlText )
    EVT_RADIOBOX( ID_COLOURS, a2dStyleDialog::ColorSelect )
    EVT_COMMAND_SCROLL( ID_SLIDER_RED, a2dStyleDialog::OnSliderUpdate )
    EVT_COMMAND_SCROLL( ID_SLIDER_GREEN, a2dStyleDialog::OnSliderUpdate )
    EVT_COMMAND_SCROLL( ID_SLIDER_BLUE, a2dStyleDialog::OnSliderUpdate )
    EVT_COMMAND_SCROLL( ID_SLIDER_ALPHA, a2dStyleDialog::OnSliderUpdate )
    EVT_BUTTON  ( ID_FILLCOLOR, a2dStyleDialog::OnColourBitmap )
    EVT_BUTTON  ( ID_STROKECOLOR, a2dStyleDialog::OnColourBitmap )
    EVT_BUTTON  ( ID_STIPPLEBITMAP, a2dStyleDialog::OnStippleBitmap )
    EVT_CHOICE  ( ID_FILLSTYLE, a2dStyleDialog::OnFillStyle )
    EVT_CHOICE  ( ID_STROKESTYLE, a2dStyleDialog::OnStrokeStyle )
    EVT_TEXT    ( ID_PIXELWIDTH, a2dStyleDialog::OnPixelWidth )
    EVT_SPINCTRL( ID_PIXELWIDTH, a2dStyleDialog::OnPixelWidthSpin )
    EVT_CHECKBOX( ID_PIXELSTROKE, a2dStyleDialog::OnPixelStroke )
    EVT_CLOSE   ( a2dStyleDialog::OnCloseWindow )
    EVT_COM_EVENT( a2dStyleDialog::OnComEvent )
    EVT_DO( a2dStyleDialog::OnDoEvent )
    EVT_UNDO( a2dStyleDialog::OnUndoEvent )
    EVT_SIZE( a2dStyleDialog::OnSize )
    EVT_BUTTON  ( ID_MODEL_BUTTON_OK, a2dStyleDialog::CmOk )
    EVT_IDLE ( a2dStyleDialog::OnIdle )
END_EVENT_TABLE()

a2dStyleDialog::a2dStyleDialog( a2dHabitat* habitat, wxWindow* parent, long style, bool modal, bool setCentralStyle ):
    wxDialog( parent, -1, _T( "style dialog" ), wxDefaultPosition, wxDefaultSize, style, _T( "style dialog" ) )
{
    m_habitat =  habitat;
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );

    m_fillpatterns = new FillPatterns();

    m_fillpatternmono = *m_fillpatterns->GetPattern( 0 );

    // no standard colours
    // set extra part of dialog true
    m_cdata = new wxColourData();
    m_cdata->SetChooseFull( true );
    // make colour dialog
    m_cd = new wxColourDialog( this, m_cdata );

    m_fillstippleb = wxBitmap( 34, 34, -1 );

    // the three colors which can be changed in the dialog sliders
    m_colorFill1 = wxColour( 0, 255, 255 );
    m_colorFill2 = wxColour( 0, 255, 0 );
    m_colorStroke = wxColour( 0, 0, 255 );

    //set the current color to be changeable
    m_color = &m_colorFill1;

    //make for every type of style a template to be modified.
    m_oneColFill   = a2dFill( m_colorFill1, a2dFILL_SOLID );
    m_PattFill     = a2dFill( m_fillpatternmono );
    m_hatchTwoColFill = a2dFill( m_colorFill1, m_colorFill2, a2dFILL_TWOCOL_BDIAGONAL_HATCH );
    m_linGrad      = a2dFill( m_colorFill1, m_colorFill2, a2dFILL_GRADIENT_FILL_XY_LINEAR );
    m_radGrad      = a2dFill( m_colorFill1, m_colorFill2, a2dFILL_GRADIENT_FILL_XY_RADIAL );
    m_dropGrad     = a2dFill( m_colorFill1, m_colorFill2, a2dFILL_GRADIENT_FILL_XY_RADIAL  );
    m_oneColStroke = a2dStroke( m_colorStroke, 1.0, a2dSTROKE_SOLID );
    m_pattStroke   = a2dStroke( m_fillpatternmono );

    //set the actual current/initial templates
    m_fill = m_oneColFill;
    m_stroke = m_oneColStroke;

    wxBoxSizer* item0 = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* item4 = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* item5 = new wxBoxSizer( wxHORIZONTAL );

    wxBoxSizer* item6 = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* item7 = new wxBoxSizer( wxHORIZONTAL );

	SetEvtHandlerEnabled( false );
    m_spinred = new wxSpinCtrl( this, ID_RED, _T( "125" ), wxDefaultPosition, wxSize( 50, -1 ), wxSP_ARROW_KEYS, 0, 255, 122 );
    item7->Add( m_spinred, 0, wxALIGN_CENTRE | wxALL, 2 );

    m_sliderred = new wxSlider( this, ID_SLIDER_RED, 125, 0, 255, wxDefaultPosition, wxSize( 120, -1 ), 0 );

    item7->Add( m_sliderred, 0, wxALIGN_CENTRE | wxALL, 2 );

    item6->Add( item7, 0, wxALIGN_CENTER, 2 );

    wxBoxSizer* item10 = new wxBoxSizer( wxHORIZONTAL );

    m_spingreen = new wxSpinCtrl( this,  ID_GREEN, _T( "125" ), wxDefaultPosition, wxSize( 50, -1 ), wxSP_ARROW_KEYS, 0, 255, 122 );
    item10->Add( m_spingreen, 0, wxALIGN_CENTRE | wxALL, 2 );

    m_slidergreen = new wxSlider( this, ID_SLIDER_GREEN, 125, 0, 255, wxDefaultPosition, wxSize( 120, -1 ), 0 );
    item10->Add( m_slidergreen, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT | wxBOTTOM, 2 );

    item6->Add( item10, 0, wxALIGN_CENTRE, 2 );

    wxBoxSizer* item13 = new wxBoxSizer( wxHORIZONTAL );

    m_spinblue = new wxSpinCtrl( this, ID_BLUE, _T( "125" ), wxDefaultPosition, wxSize( 50, -1 ), wxSP_ARROW_KEYS, 0, 255, 122 );
    item13->Add( m_spinblue, 0, wxALIGN_CENTRE | wxALL, 2 );

    m_sliderblue = new wxSlider( this, ID_SLIDER_BLUE, 125, 0, 255, wxDefaultPosition, wxSize( 120, -1 ), 0 );
    item13->Add( m_sliderblue, 0, wxALIGN_CENTRE | wxALL, 2 );

    item6->Add( item13, 0, wxALIGN_CENTRE, 2 );

    wxBoxSizer* item133 = new wxBoxSizer( wxHORIZONTAL );

    m_spinalpha = new wxSpinCtrl( this, ID_ALPHA, _T( "125" ), wxDefaultPosition, wxSize( 50, -1 ), wxSP_ARROW_KEYS, 0, 255, 122 );
    item133->Add( m_spinalpha, 0, wxALIGN_CENTRE | wxALL, 2 );

    m_slideralpha = new wxSlider( this, ID_SLIDER_ALPHA, 125, 0, 255, wxDefaultPosition, wxSize( 120, -1 ), 0 );
    item133->Add( m_slideralpha, 0, wxALIGN_CENTRE | wxALL, 2 );

    item6->Add( item133, 0, wxALIGN_CENTRE, 2 );

    wxBoxSizer* item63 = new wxBoxSizer( wxHORIZONTAL );
    wxString strs18[] =
    {
        _T( "Stroke" ),
        _T( "Fill 1" ),
        _T( "Fill2" )
    };
    wxRadioBox* item18 = new wxRadioBox( this, ID_COLOURS, _T( "Colours" ), wxDefaultPosition, wxDefaultSize, 3, strs18, 1, wxRA_SPECIFY_ROWS );
    item18->SetSelection( 1 );
    item63->Add( item18, 0, wxALIGN_CENTRE );
    item6->Add( item63, 0, wxALIGN_CENTRE, 2 );

    wxBoxSizer* item32 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText* item33 = new wxStaticText( this, -1, _T( "stipple bitmap" ), wxDefaultPosition, wxSize( 60, -1 ), 0 );
    item33->SetForegroundColour( *wxRED );
    item33->SetBackgroundColour( *wxLIGHT_GREY );
    item32->Add( item33, 0, wxALIGN_CENTRE | wxALL, 2 );
    m_fillstipple = new wxBitmapButton( this, ID_STIPPLEBITMAP, m_fillstippleb, wxDefaultPosition, wxSize( 34, 34 ) );
    item32->Add( m_fillstipple, 0, wxALIGN_CENTRE | wxALL, 2 );
    item6->Add( item32, 0, wxALIGN_LEFT, 2 );

    item5->Add( item6, 1, wxGROW, 2 );

    m_colourfillshow = new wxBitmapButton( this, ID_FILLCOLOR, wxBitmap( 100, 150 ), wxDefaultPosition, wxDefaultSize );
    item5->Add( m_colourfillshow, 0, wxGROW, 1 );

    m_colourstrokeshow = new wxBitmapButton( this, ID_STROKECOLOR, wxBitmap( 10, 150 ), wxDefaultPosition, wxDefaultSize );
    item5->Add( m_colourstrokeshow, 0, wxGROW, 1 );

    item4->Add( item5, 0, wxEXPAND, 2 );

    item0->Add( item4, 0, wxALIGN_CENTER_HORIZONTAL, 2 );


    //fill style line
    wxBoxSizer* item21 = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* item22 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText* item23 = new wxStaticText( this, ID_TEXT, _T( "Fill Style" ), wxDefaultPosition, wxSize( 60, -1 ), wxST_NO_AUTORESIZE );
    item23->SetForegroundColour( *wxRED );
    item23->SetBackgroundColour( *wxLIGHT_GREY );
    item22->Add( item23, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxLEFT, 1 );

    wxString fillstyles[] =
    {
        _T( "Transparent (no fill)" ),
        _T( "Solid" ),
        _T( "Backward diagonal hatch" ),
        _T( "Cross-diagonal hatch" ),
        _T( "Forward diagonal hatch" ),
        _T( "Cross hatch" ),
        _T( "Horizontal hatch" ),
        _T( "Vertical hatch" ),
        _T( "Two Colour Backward diagonal hatch" ),
        _T( "Two Colour Cross-diagonal hatch" ),
        _T( "Two Colour Forward diagonal hatch" ),
        _T( "Two Colour Cross hatch" ),
        _T( "Two Colour Horizontal hatch" ),
        _T( "Two Colour Vertical hatch" ),
        _T( "wxSTIPPLE" ),
        _T( "wxSTIPPLE_MASK_OPAQUE" ),
        _T( "wxSTIPPLE_MASK_OPAQUE_TRANSPARENT" ),
        _T( "wxGRADIENT_FILL_XY_LINEAR" ),
        _T( "wxGRADIENT_FILL_XY_RADIAL" ),
        _T( "wxGRADIENT_FILL_XY_DROP" ),
        _T( "Inherit" ),
        _T( "Layer" )
    };

    m_fillstyles = new wxChoice( this, ID_FILLSTYLE, wxDefaultPosition,  wxSize( 70, -1 ), 22, fillstyles, 0 );
    item22->Add( m_fillstyles, 1, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    item21->Add( item22, 0, wxEXPAND );

    wxBoxSizer* item25 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText* item26 = new wxStaticText( this, ID_TEXT, _T( "stroke width" ), wxDefaultPosition, wxSize( 60, -1 ), wxST_NO_AUTORESIZE );
    item26->SetForegroundColour( *wxRED );
    item26->SetBackgroundColour( *wxLIGHT_GREY );
    item25->Add( item26, 0,  wxALIGN_LEFT | wxLEFT, 1 );

    m_pixelwidth = new wxSpinCtrl( this, ID_PIXELWIDTH, _T( "0" ), wxDefaultPosition, wxSize( 60, -1 ), wxSP_ARROW_KEYS, 0, 1000, 0 );
    item25->Add( m_pixelwidth, 0, wxEXPAND );

    m_pixelstroke = new wxCheckBox( this, ID_PIXELSTROKE, _T( "Pixel stroke" ), wxDefaultPosition, wxDefaultSize, 0 );
    item25->Add( m_pixelstroke, 0, wxALL, 1 );

    item21->Add( item25, 0, wxEXPAND );

    item0->Add( item21, 0, wxEXPAND );

    wxBoxSizer* item29 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText* item30 = new wxStaticText( this, ID_TEXT, _T( "StrokeStyle" ), wxDefaultPosition, wxSize( 60, -1 ), wxST_NO_AUTORESIZE );
    item30->SetForegroundColour( *wxRED );
    item30->SetBackgroundColour( *wxLIGHT_GREY );
    item29->Add( item30, 0, wxALIGN_LEFT | wxLEFT, 1 );

    wxString strokestyles[] =
    {
        _T( "Transparent" ),
        _T( "Solid style" ),
        _T( "Dotted style" ),
        _T( "Long dashed style" ),
        _T( "Short dashed style" ),
        _T( "Dot and dash style" ),
        _T( "User defined dash style" ),
        _T( "Backward diagonal hatch" ),
        _T( "Cross-diagonal hatch" ),
        _T( "Forward diagonal hatch" ),
        _T( "Cross hatch" ),
        _T( "Horizontal hatch" ),
        _T( "Vertical hatch" ),
        _T( "wxSTIPPLE" ),
        _T( "wxSTIPPLE_MASK_OPAQUE" ),

        _T( "wxSTIPPLE_MASK_OPAQUE_TRANSPARENT" ),
        _T( "a2dSTROKE_OBJECTS" ),
        _T( "Inherit" ),
        _T( "Layer" ),
    };

    m_strokestyles = new wxChoice( this, ID_STROKESTYLE, wxDefaultPosition, wxSize( 70, -1 ), 19, strokestyles, 0 );
    item29->Add( m_strokestyles, 1, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    item0->Add( item29, 0, wxEXPAND, 2 );

    if ( modal )
    {
        item0->Add( new wxButton(    this, ID_MODEL_BUTTON_OK, _T( "Ok" ), wxDefaultPosition, wxSize( 55, 20 ) ) );
        m_modal = true;
    }
    else
    {
        m_modal = false;
    }

	SetEvtHandlerEnabled( true );
    SetAutoLayout( true );
    SetSizer( item0 );
    item0->Fit( this );
    item0->SetSizeHints( this );

    m_skipspinevent = false;
    m_doSentCommand = true;

    m_needsUpdate = true;
    m_fillCommand = false;
    m_strokeCommand = false;
    m_setCentralStyle = setCentralStyle;
}


a2dStyleDialog::~a2dStyleDialog()
{
    a2dCanvasGlobals->DisconnectEvent( a2dEVT_COM_EVENT, this );
    delete m_fillpatterns;
    delete m_cdata;
    delete m_cd;
}

void a2dStyleDialog::SetColor( wxSpinEvent& spinevent )
{
    unsigned char m_colorR = m_color->Red();
    unsigned char m_colorG = m_color->Green();
    unsigned char m_colorB = m_color->Blue();
    unsigned char m_colorA = m_color->Alpha();

    switch ( spinevent.GetId() )
    {
        case ID_RED:
        {
            m_color->Set( spinevent.GetPosition(), m_colorG, m_colorB, m_colorA );
        }
        break;
        case ID_GREEN:
        {
            m_color->Set( m_colorR, spinevent.GetPosition(), m_colorB, m_colorA );
        }
        break;
        case ID_BLUE:
        {
            m_color->Set( m_colorR, m_colorG, spinevent.GetPosition(), m_colorA );
        }
        break;
        case ID_ALPHA:
        {
            m_color->Set( m_colorR, m_colorG, m_colorB, spinevent.GetPosition() );
        }
        break;
        default:
            break;
    }

    m_needsUpdate = true;

    if ( m_color == &m_colorStroke )
        SentStyle( false, true );
    else
        SentStyle( true, false );
}

void a2dStyleDialog::OnPixelWidth( wxCommandEvent& WXUNUSED( event ) )
{
    m_stroke.SetWidth( m_pixelwidth->GetValue() );
    SentStyle( false, true );
}

void a2dStyleDialog::OnPixelWidthSpin( wxSpinEvent& WXUNUSED( event ) )
{
    m_stroke.SetWidth( m_pixelwidth->GetValue() );
    SentStyle( false, true );
}

void a2dStyleDialog::OnPixelStroke( wxCommandEvent& WXUNUSED( event ) )
{
    m_stroke.SetPixelStroke( m_pixelstroke->GetValue() );
    SentStyle( false, true );
}


void a2dStyleDialog::OnSpinCtrlText( wxCommandEvent& event )
{
    if ( m_skipspinevent == true )
    {
        m_skipspinevent = false;
        return;
    }

    unsigned char m_colorR = m_color->Red();
    unsigned char m_colorG = m_color->Green();
    unsigned char m_colorB = m_color->Blue();
    unsigned char m_colorA = m_color->Alpha();

    switch ( event.GetId() )
    {
        case ID_RED:
        {
            m_color->Set( m_spinred->GetValue(), m_colorG, m_colorB, m_colorA );
        }
        break;
        case ID_GREEN:
        {
            m_color->Set( m_colorR, m_spingreen->GetValue(), m_colorB, m_colorA );
        }
        break;
        case ID_BLUE:
        {
            m_color->Set( m_colorR, m_colorG, m_spinblue->GetValue(), m_colorA );
        }
        break;
        case ID_ALPHA:
        {
            m_color->Set( m_colorR, m_colorG, m_colorB, m_spinalpha->GetValue() );
        }
        break;
        default:
            break;
    }

    m_needsUpdate = true;

    if ( m_color == &m_colorStroke )
        SentStyle( false, true );
    else
        SentStyle( true, false );
}

void a2dStyleDialog::OnSliderUpdate( wxScrollEvent& event )
{
    unsigned char m_colorR = m_color->Red();
    unsigned char m_colorG = m_color->Green();
    unsigned char m_colorB = m_color->Blue();
    unsigned char m_colorA = m_color->Alpha();

    switch ( event.GetId() )
    {
        case ID_SLIDER_RED:
        {
            m_color->Set( m_sliderred->GetValue(), m_colorG, m_colorB, m_colorA );
        }
        break;
        case ID_SLIDER_GREEN:
        {
            m_color->Set( m_colorR, m_slidergreen->GetValue(), m_colorB, m_colorA );
        }
        break;
        case ID_SLIDER_BLUE:
        {
            m_color->Set( m_colorR, m_colorG, m_sliderblue->GetValue(), m_colorA );
        }
        break;
        case ID_SLIDER_ALPHA:
        {
            m_color->Set( m_colorR, m_colorG, m_colorB, m_slideralpha->GetValue() );
        }
        break;
        default:
            break;
    }

    m_needsUpdate = true;

    if ( event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE )
    {
        if ( m_color == &m_colorStroke )
            SentStyle( false, true );
        else
            SentStyle( true, false );
    }
}


void a2dStyleDialog::ColorSelect( wxCommandEvent& event )
{

    switch ( event.GetInt() )
    {
        case 0:
        {
            m_color = &m_colorStroke;
        }
        break;
        case 1:
        {
            m_color = &m_colorFill1;
        }
        break;
        case 2:
        {
            m_color = &m_colorFill2;
        }
        break;
        default:
            break;
    }

    m_needsUpdate = true;

    if ( m_color == &m_colorStroke )
        SentStyle( false, true );
    else
        SentStyle( true, false );
}


void a2dStyleDialog::OnColourBitmap( wxCommandEvent& event )
{
    if ( m_cd->ShowModal() == wxID_OK )
    {
        *m_cdata = m_cd->GetColourData();
        if ( event.GetId() == ID_STROKECOLOR )
            m_colorStroke = m_cdata->GetColour();
        else if ( event.GetId() == ID_FILLCOLOR )
            *m_color = m_cdata->GetColour();
    }

    m_needsUpdate = true;

    if ( m_color == &m_colorStroke )
        SentStyle( false, true );
    else
        SentStyle( true, false );
}


void a2dStyleDialog::OnStippleBitmap( wxCommandEvent& WXUNUSED( event ) )
{
    PatternDialog pattern( this, m_fillpatterns, _T( "Patterns" ), ( wxDEFAULT_DIALOG_STYLE ) );
    int choosen = pattern.ShowModal();

    m_fillpatternmono = *m_fillpatterns->GetPattern( choosen );

    m_needsUpdate = true;
    SentStyle( true, false );
}

void a2dStyleDialog::OnStrokeStyle( wxCommandEvent& event )
{
    a2dStrokeStyle strokestyle = a2dSTROKE_SOLID;

    switch( event.GetInt() )
    {
        case 0: strokestyle = a2dSTROKE_TRANSPARENT;
            break;
        case 1: strokestyle = a2dSTROKE_SOLID;
            break;
        case 2: strokestyle = a2dSTROKE_DOT;
            break;
        case 3: strokestyle = a2dSTROKE_LONG_DASH;
            break;
        case 4: strokestyle = a2dSTROKE_SHORT_DASH;
            break;
        case 5: strokestyle = a2dSTROKE_DOT_DASH;
            break;
        case 6: strokestyle = a2dSTROKE_USER_DASH;
            break;
        case 7: strokestyle = a2dSTROKE_BDIAGONAL_HATCH;
            break;
        case 8: strokestyle = a2dSTROKE_CROSSDIAG_HATCH;
            break;
        case 9: strokestyle = a2dSTROKE_FDIAGONAL_HATCH;
            break;
        case 10: strokestyle = a2dSTROKE_CROSS_HATCH;
            break;
        case 11: strokestyle = a2dSTROKE_HORIZONTAL_HATCH;
            break;
        case 12: strokestyle = a2dSTROKE_VERTICAL_HATCH;
            break;
        case 13: strokestyle = a2dSTROKE_STIPPLE ;
            break;
        case 14: strokestyle = a2dSTROKE_STIPPLE_MASK_OPAQUE;
            break;
        case 15: strokestyle = a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT;
            break;
        case 16: strokestyle = a2dSTROKE_OBJECTS;
            break;
        case 17: strokestyle = a2dSTROKE_INHERIT;
            break;
        case 18: strokestyle = a2dSTROKE_LAYER;
            break;

        default: strokestyle = a2dSTROKE_SOLID;
    }

    if ( strokestyle != m_stroke.GetStyle() )
    {
        if ( strokestyle == a2dSTROKE_LAYER )
        {
            m_stroke = *a2dNullSTROKE;
        }
        else if ( strokestyle == a2dSTROKE_INHERIT )
        {
            m_stroke = *a2dINHERIT_STROKE;
        }
        else if ( strokestyle == a2dSTROKE_STIPPLE_MASK_OPAQUE )
        {
            m_pattStroke.SetStyle( a2dSTROKE_STIPPLE_MASK_OPAQUE );
            m_stroke = m_pattStroke;
        }
        else if ( strokestyle == a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT )
        {
            m_pattStroke.SetStyle( a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT );
            m_stroke = m_pattStroke;
        }
        else if ( strokestyle == a2dSTROKE_STIPPLE )
        {
            m_pattStroke.SetStyle( a2dSTROKE_STIPPLE );
            m_stroke = m_pattStroke;
        }
        else
            m_stroke.SetStyle( strokestyle );
    }

    m_needsUpdate = true;
    SentStyle( false, true );
}

void a2dStyleDialog::OnFillStyle( wxCommandEvent& event )
{

    a2dFillStyle fillstyle = a2dFILL_SOLID;
    switch( event.GetInt() )
    {
        case 0: fillstyle = a2dFILL_TRANSPARENT;
            break;
        case 1: fillstyle = a2dFILL_SOLID;
            break;
        case 2: fillstyle = a2dFILL_BDIAGONAL_HATCH;
            break;
        case 3: fillstyle = a2dFILL_CROSSDIAG_HATCH;
            break;
        case 4: fillstyle = a2dFILL_FDIAGONAL_HATCH;
            break;
        case 5: fillstyle = a2dFILL_CROSS_HATCH;
            break;
        case 6: fillstyle = a2dFILL_HORIZONTAL_HATCH;
            break;
        case 7: fillstyle = a2dFILL_VERTICAL_HATCH;
            break;
        case 8: fillstyle = a2dFILL_TWOCOL_BDIAGONAL_HATCH;
            break;
        case 9: fillstyle = a2dFILL_TWOCOL_CROSSDIAG_HATCH;
            break;
        case 10: fillstyle = a2dFILL_TWOCOL_FDIAGONAL_HATCH;
            break;
        case 11: fillstyle = a2dFILL_TWOCOL_CROSS_HATCH;
            break;
        case 12: fillstyle = a2dFILL_TWOCOL_HORIZONTAL_HATCH;
            break;
        case 13: fillstyle = a2dFILL_TWOCOL_VERTICAL_HATCH;
            break;
        case 14: fillstyle = a2dFILL_STIPPLE;
            break;
        case 15: fillstyle = a2dFILL_STIPPLE_MASK_OPAQUE;
            break;
        case 16: fillstyle = a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT;
            break;
        case 17: fillstyle = a2dFILL_GRADIENT_FILL_XY_LINEAR;
            break;
        case 18: fillstyle = a2dFILL_GRADIENT_FILL_XY_RADIAL;
            break;
        case 19: fillstyle = a2dFILL_GRADIENT_FILL_XY_DROP;
            break;
        case 20: fillstyle = a2dFILL_INHERIT;
            break;
        case 21: fillstyle = a2dFILL_LAYER;
            break;
        default: fillstyle = a2dFILL_SOLID;
    }

    //if the new style does not fit the current fill type,
    //we release the current one, and replace it with the template fill that does
    //fit the new fill style.
    //Colours for the fill will be set from the old fill when possible.
    if ( fillstyle != m_fill.GetStyle() )
    {
        if ( fillstyle == a2dFILL_LAYER )
        {
            m_fill = *a2dNullFILL;
        }
        else if ( fillstyle == a2dFILL_INHERIT )
        {
            m_fill = *a2dINHERIT_FILL;
        }
        else if ( fillstyle == a2dFILL_STIPPLE_MASK_OPAQUE )
        {
            m_PattFill.SetStyle( a2dFILL_STIPPLE_MASK_OPAQUE );
            m_fill = m_PattFill;
        }
        else if ( fillstyle == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
        {
            m_PattFill.SetStyle( a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT );
            m_fill = m_PattFill;
        }
        else if ( fillstyle == a2dFILL_STIPPLE )
        {
            m_PattFill.SetStyle( a2dFILL_STIPPLE );
            m_fill = m_PattFill;
        }
        else if ( fillstyle == a2dFILL_GRADIENT_FILL_XY_LINEAR )
        {
            m_fill = m_linGrad;
        }
        else if ( fillstyle == a2dFILL_GRADIENT_FILL_XY_RADIAL )
        {
            m_fill = m_radGrad;
        }
        else if ( fillstyle == a2dFILL_GRADIENT_FILL_XY_DROP )
        {
            m_fill = m_dropGrad;
        }
        else if ( fillstyle >= a2dFIRST_TWOCOL_HATCH && fillstyle <= a2dLAST_TWOCOL_HATCH )
        {
            m_hatchTwoColFill.SetStyle( fillstyle );
            m_fill = m_hatchTwoColFill;
        }
        else if ( fillstyle >= a2dFIRST_HATCH && fillstyle <= a2dLAST_HATCH )
        {
            m_oneColFill.SetStyle( fillstyle );
            m_fill = m_oneColFill;
        }
        else
        {
            m_oneColFill.SetStyle( fillstyle );
            m_fill = m_oneColFill;
        }
    }

    m_needsUpdate = true;
    SentStyle( true, false );
}


void a2dStyleDialog::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    if ( m_modal )
        EndModal( wxID_OK );
    else
    {
        SetReturnCode( wxID_OK );
        Show( false );
    }
}

void a2dStyleDialog::OnIdle( wxIdleEvent& WXUNUSED( idleEvent ) )
{
    if ( m_needsUpdate )
    {
        m_needsUpdate = false;

        unsigned char m_colorR = m_color->Red();
        unsigned char m_colorG = m_color->Green();
        unsigned char m_colorB = m_color->Blue();
        unsigned char m_colorA = m_color->Alpha();

        m_sliderred->SetValue( m_colorR );
        m_slidergreen->SetValue( m_colorG );
        m_sliderblue->SetValue( m_colorB );
        m_slideralpha->SetValue( m_colorA );

        //this SetValue generates an event which leads to extra SentStyle calls, not wanted
        //solved by setting skip flag.
        m_skipspinevent = true;
        m_spinred->SetValue( m_colorR );
        m_skipspinevent = true;
        m_spingreen->SetValue( m_colorG );
        m_skipspinevent = true;
        m_spinblue->SetValue( m_colorB );
        m_skipspinevent = true;
        m_spinalpha->SetValue( m_colorA );

        m_pixelwidth->SetValue( ( int ) m_stroke.GetWidth() );
        m_pixelstroke->SetValue( m_stroke.GetPixelStroke() );

        //adjust all colors for all possible styles to the current situation
        m_oneColFill.SetColour( m_colorFill1 );
        m_PattFill.SetColour( m_colorFill1 );
        m_PattFill.SetColour2( m_colorFill2 );
        m_linGrad.SetColour( m_colorFill1 );
        m_linGrad.SetColour2( m_colorFill2 );
        m_radGrad.SetColour( m_colorFill1 );
        m_radGrad.SetColour2( m_colorFill2 );
        m_dropGrad.SetColour( m_colorFill1 );
        m_dropGrad.SetColour2( m_colorFill2 );
        m_oneColStroke.SetColour( m_colorStroke );
        m_pattStroke.SetColour( m_colorStroke );
        m_hatchTwoColFill.SetColour( m_colorFill1 );
        m_hatchTwoColFill.SetColour2( m_colorFill2 );

        m_fillstipple->Enable( false );
        int style;
        int curstyle = m_fill.GetStyle();
        switch( curstyle )
        {
            case a2dFILL_TRANSPARENT: style = 0;
                break;
            case a2dFILL_SOLID: style = 1;
                break;
            case a2dFILL_BDIAGONAL_HATCH: style = 2;
                break;
            case a2dFILL_CROSSDIAG_HATCH: style = 3;
                break;
            case a2dFILL_FDIAGONAL_HATCH: style = 4;
                break;
            case a2dFILL_CROSS_HATCH: style = 5;
                break;
            case a2dFILL_HORIZONTAL_HATCH: style = 6;
                break;
            case a2dFILL_VERTICAL_HATCH: style = 7;
                break;
            case a2dFILL_TWOCOL_BDIAGONAL_HATCH: style = 8;
                break;
            case a2dFILL_TWOCOL_CROSSDIAG_HATCH: style = 9;
                break;
            case a2dFILL_TWOCOL_FDIAGONAL_HATCH: style = 10;
                break;
            case a2dFILL_TWOCOL_CROSS_HATCH: style = 11;
                break;
            case a2dFILL_TWOCOL_HORIZONTAL_HATCH: style = 12;
                break;
            case a2dFILL_TWOCOL_VERTICAL_HATCH: style = 13;
                break;
            case a2dFILL_STIPPLE: style = 14;
                m_fillstipple->Enable( true );
                break;
            case a2dFILL_STIPPLE_MASK_OPAQUE:  style = 15;
                m_fillstipple->Enable( true );
                break;
            case a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT: style = 16;
                m_fillstipple->Enable( true );
                break;
            case a2dFILL_GRADIENT_FILL_XY_LINEAR: style = 17;
                break;
            case a2dFILL_GRADIENT_FILL_XY_RADIAL: style = 18;
                break;
            case a2dFILL_GRADIENT_FILL_XY_DROP: style = 19;
                break;
            case a2dFILL_INHERIT: style = 20;
                break;
            case a2dFILL_LAYER: style = 21;
                break;
            default: style = 0;
        }
        m_fillstyles->SetSelection( style );

        int strokestyle = 0;
        switch( m_stroke.GetStyle() )
        {
            case a2dSTROKE_TRANSPARENT: strokestyle = 0;
                break;
            case a2dSTROKE_SOLID: strokestyle = 1;
                break;
            case a2dSTROKE_DOT: strokestyle = 2;
                break;
            case a2dSTROKE_LONG_DASH: strokestyle = 3;
                break;
            case a2dSTROKE_SHORT_DASH: strokestyle = 4;
                break;
            case a2dSTROKE_DOT_DASH: strokestyle = 5;
                break;
            case a2dSTROKE_USER_DASH:  strokestyle = 6;
                break;
            case a2dSTROKE_BDIAGONAL_HATCH: strokestyle = 7;
                break;
            case a2dSTROKE_CROSSDIAG_HATCH: strokestyle = 8;
                break;
            case a2dSTROKE_FDIAGONAL_HATCH: strokestyle = 9;
                break;
            case a2dSTROKE_CROSS_HATCH: strokestyle = 10;
                break;
            case a2dSTROKE_HORIZONTAL_HATCH: strokestyle = 11;
                break;
            case a2dSTROKE_VERTICAL_HATCH: strokestyle = 12;
                break;
            case a2dSTROKE_STIPPLE: strokestyle = 13;
                break;
            case a2dSTROKE_STIPPLE_MASK_OPAQUE: strokestyle = 14;
                break;
            case a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT: strokestyle = 15;
                break;
            case a2dSTROKE_OBJECTS: style = 16;
                break;
            case a2dSTROKE_INHERIT: style = 17;
                break;
            case a2dSTROKE_LAYER: style = 18;
                break;
            default: strokestyle = 0;
        }

        m_strokestyles->SetSelection( strokestyle );

        // fill the bitmap button again.
        wxMemoryDC dcmemtemp;
        dcmemtemp.SelectObject( m_fillstippleb );
        wxBrush brush;
        brush.SetColour( *wxBLACK );
        dcmemtemp.SetBackground( brush );
        dcmemtemp.Clear();
        dcmemtemp.SetPen( *wxTRANSPARENT_PEN );

        dcmemtemp.SetTextBackground( m_PattFill.GetColour2() );
        dcmemtemp.SetTextForeground( m_PattFill.GetColour() );
        wxBrush brushbitm( m_fillpatternmono );
        dcmemtemp.SetBrush( brushbitm );

        dcmemtemp.DrawRectangle( 0, 0, 32, 32 );
        dcmemtemp.SelectObject( wxNullBitmap );
        m_fillstipple->SetBitmapLabel( m_fillstippleb );
        m_fillstipple->Refresh( true );

        /*
            dcmemtemp.SelectObject( m_fillstippleb );
            dcmemtemp.SetTextBackground( *wxBLACK );
            dcmemtemp.SetTextForeground( m_colorFill1 );
            dcmemtemp.DrawBitmap( m_fillpatternmono, 1, 1 );
               dcmemtemp.SelectObject(wxNullBitmap);
        */
        if ( m_fill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE )
        {
            m_PattFill.SetStipple( m_fillpatternmono );
            m_PattFill.SetStyle( a2dFILL_STIPPLE_MASK_OPAQUE );
            m_fill = m_PattFill;
        }
        else if ( m_fill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
        {
            m_PattFill.SetStipple( m_fillpatternmono );
            m_PattFill.SetStyle( a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT );
            m_fill = m_PattFill;
        }
        else if ( m_fill.GetStyle() == a2dFILL_STIPPLE )
        {
            //if ( m_fillpatternmono.GetDepth() == 1)
            {
                wxBitmap fillpattern = wxBitmap( 32, 32, -1 );
                // adjust the colors of the filling bitmap to the colours of the pattern fill.
                wxMemoryDC dcmemtemp2;
                dcmemtemp2.SelectObject( fillpattern );

                wxBrush brush;
                brush.SetColour( *wxBLACK );
                dcmemtemp2.SetBackground( brush );
                dcmemtemp2.Clear();
                dcmemtemp2.SetPen( *wxTRANSPARENT_PEN );

                dcmemtemp2.SetTextBackground( m_PattFill.GetColour2() );
                dcmemtemp2.SetTextForeground( m_PattFill.GetColour() );
                wxBrush brushbitm( m_fillpatternmono );
                dcmemtemp2.SetBrush( brushbitm );

                dcmemtemp2.DrawRectangle( 0, 0, 32, 32 );
                dcmemtemp2.SelectObject( wxNullBitmap );
                m_PattFill.SetStipple( fillpattern );
            }
            //else
            //    m_PattFill->SetStipple( m_fillpatternmono );
            m_PattFill.SetStyle( a2dFILL_STIPPLE );
            m_fill = m_PattFill;
        }
        else if ( m_fill.GetType() == a2dFILL_HATCH_TWO_COLOUR )
        {
            m_fill = m_hatchTwoColFill;
        }
        else if ( m_fill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
        {
            m_fill = m_linGrad;
        }
        else if ( m_fill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
        {
            m_fill = m_radGrad;
        }
        else if ( m_fill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
        {
            m_fill = m_radGrad;
        }
        {
            //update the bitmap to show fill and stroke properties
            int w = m_colourfillshow->GetBitmapLabel().GetWidth();
            int h = m_colourfillshow->GetBitmapLabel().GetHeight();
            a2dMemDcDrawer* drawer = new a2dMemDcDrawer( w, h );
            drawer->BeginDraw();
            drawer->SetDrawerFill( m_fill );
            drawer->DrawRoundedRectangle( 0, 0, w, h, 0 );
            drawer->EndDraw();
            m_colourfillshow->SetBitmapLabel( drawer->GetBuffer() );
            m_colourfillshow->Refresh( true );
            delete drawer;

        }

        int w = m_colourstrokeshow->GetBitmapLabel().GetWidth();
        int h = m_colourstrokeshow->GetBitmapLabel().GetHeight();
        a2dMemDcDrawer* drawer2 = new a2dMemDcDrawer( w, h );
        drawer2->BeginDraw();
        a2dStroke f = a2dStroke( m_colorStroke, ( int ) w, m_stroke.GetStyle() );
        drawer2->SetDrawerStroke( f );
        drawer2->DrawLine( w / 2, 0, w / 2, h );
        drawer2->EndDraw();
        m_colourstrokeshow->SetBitmapLabel( drawer2->GetBuffer() );
        m_colourstrokeshow->Refresh( true );
        delete drawer2;
    }

    if ( m_fillCommand )
    {
        m_fillCommand = false;
        m_habitat->SetFill( m_fill );
    }
    if ( m_strokeCommand )
    {
        m_strokeCommand = false;
        m_habitat->SetStroke( m_stroke );
    }
}

void a2dStyleDialog::SentStyle( bool fill, bool stroke )
{
    m_fill.SetColour( m_colorFill1 );
    m_fill.SetColour2( m_colorFill2 );
    m_stroke.SetColour( m_colorStroke );

    if ( !m_doSentCommand )
        return;

    if ( !m_setCentralStyle )
        return;

    m_fillCommand = fill;
    m_strokeCommand = stroke;
}

void a2dStyleDialog::OnComEvent( a2dComEvent& event )
{
	a2dSignal comId = event.GetId();

    m_doSentCommand = false;

    if ( event.GetId() == a2dRecursiveEditTool::sig_toolStartEditObject )
    {
        a2dObjectEditTool* tool = wxDynamicCast( event.GetEventObject(), a2dObjectEditTool );
        if ( tool && tool->GetActive() && tool->GetDrawingPart() )
        {
            SetFill( tool->GetOriginal()->GetFill() );
            SetStroke( tool->GetOriginal()->GetStroke() );
        }
    }
	if ( event.GetId() == a2dComEvent::sm_changedProperty )
    {
        a2dNamedProperty* property = event.GetProperty();
        a2dObject* object = property->GetRefObjectNA();

        if ( 0 != wxDynamicCast( property, a2dStrokeProperty ) )
        {
            a2dStroke stroke = ( ( a2dStrokeProperty* )property )->GetValue();
            SetStroke( stroke );
        }
        else if ( 0 != wxDynamicCast( property, a2dFillProperty ) )
        {
            a2dFill fill = ( ( a2dFillProperty* )property )->GetValue();
            SetFill( fill );
        }
        else
            event.Skip();
    }


    if  ( comId == a2dHabitat::sig_changedFill ||
          comId == a2dHabitat::sig_changedStroke ||
          comId == a2dHabitat::sig_changedLayer
        )
    {
        a2dFill f = m_habitat->GetFill();
        if ( ! GetFill().IsSameAs( f ) )
            SetFill( f );
        a2dStroke s = m_habitat->GetStroke();
        if ( ! GetStroke().IsSameAs( s ) )
            SetStroke( s );
    }

	if ( event.GetId() == a2dCanvasGlobal::sig_changedActiveDrawing )
	{
		a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();
		if ( part && part->GetDrawing() )
		{
		    part->GetDrawing()->DisconnectEvent( a2dEVT_COM_EVENT, this );
		    part->GetDrawing()->ConnectEvent( a2dEVT_COM_EVENT, this );
		}
	}

    m_doSentCommand = true;
}

void a2dStyleDialog::OnUndoEvent( a2dCommandProcessorEvent& WXUNUSED( event ) )
{
}

void a2dStyleDialog::OnDoEvent( a2dCommandProcessorEvent& WXUNUSED( event ) )
{
}

// Size the subwindows when the frame is resized
void a2dStyleDialog::OnSize( wxSizeEvent& WXUNUSED( event ) )
{
    Layout();
}

void a2dStyleDialog::CmOk( wxCommandEvent& WXUNUSED( event ) )
{
    if ( m_modal )
        EndModal( wxID_OK );
    else
    {
        SetReturnCode( wxID_OK );
        Show( false );
    }
}

void a2dStyleDialog::SetFill( const a2dFill& fill )
{
    m_fill = fill;
    m_colorFill1 = m_fill.GetColour();
    m_colorFill2 = m_fill.GetColour2();

    m_needsUpdate = true;
}

void a2dStyleDialog::SetStroke( const a2dStroke& stroke )
{
    m_stroke = stroke;
    m_colorStroke = m_stroke.GetColour();

    m_needsUpdate = true;
}




//===========================================================================
// a2dCustomColors
//===========================================================================

a2dCustomColors::a2dCustomColors()
{
	Init();
}

void a2dCustomColors::Init()
{
	for(int i=0; i < wxColourData::NUM_CUSTOM; i++)
	{
		m_Color[i] = *wxWHITE;
	}
}

void a2dCustomColors::Set(const wxColourData &colData)
{
	for(int i=0; i < wxColourData::NUM_CUSTOM; i++)
	{
		m_Color[i] = colData.GetCustomColour(i);
	}
}

void a2dCustomColors::Get(wxColourData *pColData) const
{
	for(int i=0; i < wxColourData::NUM_CUSTOM; i++)
	{
		pColData->SetCustomColour(i, m_Color[i]);
	}
}

void a2dCustomColors::Set(int i, const wxColour &color)
{
	wxASSERT(i < wxColourData::NUM_CUSTOM);
	m_Color[i] = color;
}

wxColour a2dCustomColors::Get(int i) const
{
	wxASSERT(i < wxColourData::NUM_CUSTOM);
	return m_Color[i];
}

bool a2dCustomColors::operator==(const a2dCustomColors& rhs)
{
	for(int i=0; i < wxColourData::NUM_CUSTOM; i++)
	{
		if(m_Color[i] != rhs.m_Color[i]) return false;
	}
	return true;
}

bool a2dCustomColors::operator!=(const a2dCustomColors& rhs)
{
	return !(*this==rhs);
}


// XPM images:
//------------
#include "../../art/resources/JoinMiter.xpm"             // JoinMiter_xpm
#include "../../art/resources/JoinBevel.xpm"             // JoinBevel_xpm
#include "../../art/resources/JoinRound.xpm"             // JoinRound_xpm
#include "../../art/resources/JoinMiterPressed.xpm"      // JoinMiterPressed_xpm
#include "../../art/resources/JoinBevelPressed.xpm"      // JoinBevelPressed_xpm
#include "../../art/resources/JoinRoundPressed.xpm"      // JoinRoundPressed_xpm
#include "../../art/resources/JoinMiterDisabled.xpm"     // JoinMiterDisabled_xpm
#include "../../art/resources/JoinBevelDisabled.xpm"     // JoinBevelDisabled_xpm
#include "../../art/resources/JoinRoundDisabled.xpm"     // JoinRoundDisabled_xpm
#include "../../art/resources/CapButt.xpm"               // CapButt_xpm
#include "../../art/resources/CapProjecting.xpm"         // CapProjecting_xpm
#include "../../art/resources/CapRound.xpm"              // CapRound_xpm
#include "../../art/resources/CapButtPressed.xpm"        // CapButtPressed_xpm
#include "../../art/resources/CapProjectingPressed.xpm"  // CapProjectingPressed_xpm
#include "../../art/resources/CapRoundPressed.xpm"       // CapRoundPressed_xpm
#include "../../art/resources/CapButtDisabled.xpm"       // CapButtDisabled_xpm
#include "../../art/resources/CapProjectingDisabled.xpm" // CapProjectingDisabled_xpm
#include "../../art/resources/CapRoundDisabled.xpm"      // CapRoundDisabled_xpm


//===========================================================================
// ColorComboBox
//===========================================================================
const wxArrayString a2dColorComboBox::TChoices(bool bAllowTransparent)
{
	if ( bAllowTransparent )
	{
		const int TNumChoices = 2;
		const wxString TChoices[TNumChoices] = { _("Transparent"), _("Select colour...") };
		return wxArrayString(TNumChoices, TChoices);
	}
	else
	{
		const int TNumChoices = 1;
		const wxString TChoices[TNumChoices] = { _("Select colour...") };
		return wxArrayString(TNumChoices, TChoices);
	}
}

//--------------------------------------------------------
//	Event Table:
//--------------------------------------------------------
BEGIN_EVENT_TABLE( a2dColorComboBox, wxOwnerDrawnComboBox )
	EVT_COMBOBOX(wxID_ANY, a2dColorComboBox::OnSelected )
END_EVENT_TABLE()
//--------------------------------------------------------

a2dColorComboBox::a2dColorComboBox(wxWindow *parent, wxWindowID id, bool bAllowTransparent, a2dCustomColors *pCustCol, const wxSize &size)
	: wxOwnerDrawnComboBox(parent, id, wxEmptyString, wxDefaultPosition, size, TChoices(bAllowTransparent), wxCB_READONLY), 
	  m_bAllowTransparent(bAllowTransparent), m_pCustomColors(pCustCol), m_Color(*wxBLACK)
{
	SetSelection(-1);
}

void a2dColorComboBox::SetColor(const wxColour &color)
{
	m_Color = color;
	if ( m_bAllowTransparent )
		SetSelection( (m_Color == wxTransparentColour) ? 0 : 1 );
	else
		SetSelection( (m_Color == wxTransparentColour) ? -1 : 0 );
}

void a2dColorComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const
{
	if( !(flags & wxODCB_PAINTING_CONTROL) )
	{
		dc.DrawLabel(GetString(item), rect, wxALIGN_CENTER);
	}
	else
	{
		if ( m_Color == wxTransparentColour )
			dc.DrawLabel(GetString(0), rect, wxALIGN_CENTER);
		else
		{
			dc.SetPen(*wxBLACK_PEN);
			dc.SetBrush(wxBrush(m_Color));
			dc.DrawRectangle(rect.x+3, rect.y+3, rect.width-6, rect.height-6);
		}
	}
}

wxCoord a2dColorComboBox::OnMeasureItem(size_t item) const
{
	return wxOwnerDrawnComboBox::GetClientSize().GetHeight();
}

void a2dColorComboBox::OnSelected(wxCommandEvent& event)
{
	switch ( event.GetSelection() )
	{
		case 0:	if ( m_bAllowTransparent )
					m_Color = wxTransparentColour;
				else
					SelectColor();
				break;

		case 1:	SelectColor();
				break;
	}

	event.Skip();
}

void a2dColorComboBox::SelectColor()
{
	wxColourData cData;
	cData.SetChooseFull(true);
	if ( m_pCustomColors )
		m_pCustomColors->Get(&cData); // set custom colors to dialog
	cData.SetColour(m_Color); // set selected color to dialog
	wxColourDialog cDlg(this, &cData);
	cDlg.Center();
	if ( cDlg.ShowModal() == wxID_OK )
	{
		cData = cDlg.GetColourData();
		if ( m_pCustomColors )
			m_pCustomColors->Set(cData); // get custom colors from dialog
		m_Color = cData.GetColour(); // get selected color from dialog
	}
}


//===========================================================================
// a2dStrokeStyleComboBox
//===========================================================================
const wxArrayString a2dStrokeStyleComboBox::TChoices(bool bAllowLayerStyle)
{
	const int TNumChoices = 5;
	const wxString TChoices[TNumChoices] = { wxT("Solid"), wxT("Dot"), wxT("Long Dash"), wxT("Dot Dash"), _("No Line") };
	wxArrayString choices(TNumChoices, TChoices);

	if ( bAllowLayerStyle )
		choices.Add( _("Layer") );

	return choices;
}

//--------------------------------------------------------
//	Event Table:
//--------------------------------------------------------
BEGIN_EVENT_TABLE( a2dStrokeStyleComboBox, wxOwnerDrawnComboBox )
	EVT_COMBOBOX(wxID_ANY, a2dStrokeStyleComboBox::OnSelected)
END_EVENT_TABLE()
//--------------------------------------------------------

a2dStrokeStyleComboBox::a2dStrokeStyleComboBox(wxWindow *parent, wxWindowID id, bool bAllowLayerStyle, const wxSize &size)
	: wxOwnerDrawnComboBox(parent, id, wxEmptyString, wxDefaultPosition, size, TChoices(bAllowLayerStyle), wxCB_READONLY), 
	  m_bAllowLayerStyle(bAllowLayerStyle), m_Style(a2dSTROKE_SOLID)
{
	SetSelection(-1);
}

void a2dStrokeStyleComboBox::SetStyle(const a2dStrokeStyle &style)
{
	m_Style = style;
	int iSel = -1;
	switch ( m_Style )
	{
		case a2dSTROKE_SOLID:		iSel = 0;
									break;
		case a2dSTROKE_DOT:			iSel = 1;
									break;
		case a2dSTROKE_LONG_DASH:	iSel = 2;
									break;
		case a2dSTROKE_DOT_DASH:	iSel = 3;
									break;
		case a2dSTROKE_TRANSPARENT:	iSel = 4;
									break;
		case a2dSTROKE_LAYER:
		case a2dSTROKE_NULLSTROKE:	if ( m_bAllowLayerStyle )
										iSel = 5;
									break;
	}
	SetSelection( iSel );
}

void a2dStrokeStyleComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const
{
	wxPenStyle penStyle = wxPENSTYLE_INVALID;
	switch ( item )
	{
		case 0:	penStyle = wxPENSTYLE_SOLID;
				break;
		case 1:	penStyle = wxPENSTYLE_DOT;
				break;
		case 2:	penStyle = wxPENSTYLE_LONG_DASH;
				break;
		case 3:	penStyle = wxPENSTYLE_DOT_DASH;
				break;
		case 4:	dc.DrawLabel(GetString(item), rect, wxALIGN_CENTER);
				return;
		case 5:	dc.DrawLabel(GetString(item), rect, wxALIGN_CENTER);
				return;
	}
	wxPen pen(dc.GetTextForeground(), 3, penStyle);
	pen.SetCap(wxCAP_BUTT);
	dc.SetPen(pen);
	dc.DrawLine(rect.x+5, rect.y+rect.height/2, rect.x+rect.width-5, rect.y+rect.height/2);
}

wxCoord a2dStrokeStyleComboBox::OnMeasureItem(size_t item) const
{
	return wxOwnerDrawnComboBox::GetClientSize().GetHeight();
}

void a2dStrokeStyleComboBox::OnSelected(wxCommandEvent& event)
{
	switch ( event.GetSelection() )
	{
		case 0:	m_Style = a2dSTROKE_SOLID;
				break;
		case 1:	m_Style = a2dSTROKE_DOT;
				break;
		case 2:	m_Style = a2dSTROKE_LONG_DASH;
				break;
		case 3:	m_Style = a2dSTROKE_DOT_DASH;
				break;
		case 4:	m_Style = a2dSTROKE_TRANSPARENT;
				break;
		case 5:	m_Style = a2dSTROKE_LAYER;
				break;
	}

	event.Skip();
}


//===========================================================================
// a2dFillComboBox
//===========================================================================
const wxArrayString a2dFillComboBox::TChoices(bool bAllowLayerStyle)
{
	const int TNumChoices = 2;
	const wxString TChoices[TNumChoices] = { _("Transparent"), _("Select colour...") };
	wxArrayString choices(TNumChoices, TChoices);

	if ( bAllowLayerStyle )
		choices.Add( _("Layer") );

	return choices;
}

//--------------------------------------------------------
//	Event Table:
//--------------------------------------------------------
BEGIN_EVENT_TABLE( a2dFillComboBox, wxOwnerDrawnComboBox )
	EVT_COMBOBOX(wxID_ANY, a2dFillComboBox::OnSelected)
END_EVENT_TABLE()
//--------------------------------------------------------

a2dFillComboBox::a2dFillComboBox(wxWindow *parent, wxWindowID id, bool bAllowLayerStyle, a2dCustomColors *pCustCol, const wxSize &size)
	: wxOwnerDrawnComboBox(parent, id, wxEmptyString, wxDefaultPosition, size, TChoices(bAllowLayerStyle), wxCB_READONLY), 
	  m_bAllowLayerStyle(bAllowLayerStyle), m_pCustomColors(pCustCol), m_Fill()
{
	SetSelection(-1);
}

void a2dFillComboBox::SetFill(const a2dFill &fill)
{
	m_Fill = fill;
	int iSel = -1;
	switch ( m_Fill.GetStyle() )
	{
		case a2dFILL_TRANSPARENT:	iSel = 0;
									break;
		case a2dFILL_SOLID:			iSel = 1;
									break;
		case a2dFILL_LAYER:
		case a2dFILL_NULLFILL:		if ( m_bAllowLayerStyle )
										iSel = 2;
									break;
	}
	SetSelection( iSel );
}

void a2dFillComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const
{
	if( !(flags & wxODCB_PAINTING_CONTROL) )
	{
		dc.DrawLabel(GetString(item), rect, wxALIGN_CENTER);
	}
	else
	{
		switch(item)
		{
			case 0:	dc.DrawLabel(GetString(item), rect, wxALIGN_CENTER);
					break;

			case 1:	dc.SetPen(*wxBLACK_PEN);
					dc.SetBrush(wxBrush(m_Fill.GetColour()));
					dc.DrawRectangle(rect.x+3, rect.y+3, rect.width-6, rect.height-6);
					break;

			case 2:	dc.DrawLabel(GetString(item), rect, wxALIGN_CENTER);
					break;
		}
	}
}

wxCoord a2dFillComboBox::OnMeasureItem(size_t item) const
{
	return wxOwnerDrawnComboBox::GetClientSize().GetHeight();
}

void a2dFillComboBox::OnSelected(wxCommandEvent& event)
{
	switch ( event.GetSelection() )
	{
		case 0:	m_Fill = *a2dTRANSPARENT_FILL;
				break;

		case 1:	SelectColor();
				break;

		case 2:	m_Fill = *a2dNullFILL;
				break;
	}

	event.Skip();
}

void a2dFillComboBox::SelectColor()
{
	wxColourData cData;
	cData.SetChooseFull(true);
	if ( m_pCustomColors )
		m_pCustomColors->Get(&cData); // set custom colors to dialog
	cData.SetColour(m_Fill.GetColour()); // set selected color to dialog
	wxColourDialog cDlg(this, &cData);
	cDlg.Center();
	if ( cDlg.ShowModal() == wxID_OK )
	{
		cData = cDlg.GetColourData();
		if ( m_pCustomColors )
			m_pCustomColors->Set(cData); // get custom colors from dialog
		m_Fill.SetStyle(a2dFILL_SOLID);
		m_Fill.SetColour(cData.GetColour()); // get selected color from dialog
	}
}


//===========================================================================
// a2dDialogStyle
//===========================================================================

enum
{
	ID_STROKESTYLE_BOX = wxID_HIGHEST + 1,
	ID_JOINMITER_BT,
	ID_JOINBEVEL_BT,
	ID_JOINROUND_BT,
	ID_STROKECOLOR_BOX,
	ID_CAPBUTT_BT,
	ID_CAPPROJECTING_BT,
	ID_CAPROUND_BT,
	ID_STROKEWIDTH_CTRL,
	ID_FILL_BOX,
};

//--------------------------------------------------------
//	Event Table:
//--------------------------------------------------------
BEGIN_EVENT_TABLE( a2dDialogStyle, wxDialog )
	EVT_INIT_DIALOG(a2dDialogStyle::OnInitDialog)
	EVT_COMBOBOX(ID_STROKESTYLE_BOX, a2dDialogStyle::OnStrokeStyle)
	EVT_TOGGLEBUTTON(ID_JOINMITER_BT, a2dDialogStyle::OnJoin)
	EVT_TOGGLEBUTTON(ID_JOINBEVEL_BT, a2dDialogStyle::OnJoin)
	EVT_TOGGLEBUTTON(ID_JOINROUND_BT, a2dDialogStyle::OnJoin)
	EVT_TOGGLEBUTTON(ID_CAPBUTT_BT, a2dDialogStyle::OnCap)
	EVT_TOGGLEBUTTON(ID_CAPPROJECTING_BT, a2dDialogStyle::OnCap)
	EVT_TOGGLEBUTTON(ID_CAPROUND_BT, a2dDialogStyle::OnCap)
	EVT_BUTTON(wxID_OK, a2dDialogStyle::OnOk)
END_EVENT_TABLE()
//--------------------------------------------------------

a2dDialogStyle::a2dDialogStyle(wxWindow *parent, bool bGlobal, bool bFill, bool bAllowLayerStyle)
	: m_bFill(bFill), m_UnitsScale(1), m_CustomColors(), m_ExtStroke(), m_ExtFill()
{
	wxString DlgTitel = (bGlobal) ? _("Global stroke and fill") : _("Stroke and fill");
	wxDialog::Create(parent, wxID_ANY, DlgTitel, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE);
	CreateControls(bAllowLayerStyle);
	if(GetSizer()) GetSizer()->SetSizeHints(this);
	Centre();
}

void a2dDialogStyle::CreateControls(bool bAllowLayerStyle)
{
	wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(pTopSizer);

	//----------------------------
	// line:
	//----------------------------
	wxStaticBox* pStaticBoxLine = new wxStaticBox(this, wxID_ANY, _("Stroke"));
	wxStaticBoxSizer* pStaticBoxSizerLine = new wxStaticBoxSizer(pStaticBoxLine, wxVERTICAL);
	pTopSizer->Add(pStaticBoxSizerLine, 0, wxGROW|wxALL, 5);

	wxGridBagSizer* pGridBagSizerLine = new wxGridBagSizer(0, 0);
	pGridBagSizerLine->SetEmptyCellSize(wxSize(10, 10));
	pStaticBoxSizerLine->Add(pGridBagSizerLine, 0, wxALIGN_LEFT|wxALL, 5);

	wxStaticText* pStrokeStyleTxt = new wxStaticText(pStaticBoxSizerLine->GetStaticBox(), wxID_STATIC, _("Style:"), wxDefaultPosition, wxDefaultSize, 0);
	pGridBagSizerLine->Add(pStrokeStyleTxt, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	m_pSrokeStyleBox = new a2dStrokeStyleComboBox(pStaticBoxSizerLine->GetStaticBox(), ID_STROKESTYLE_BOX, bAllowLayerStyle);
	pGridBagSizerLine->Add(m_pSrokeStyleBox, wxGBPosition(0, 1), wxGBSpan(1, 1), wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* pStrokeJoinTxt = new wxStaticText(pStaticBoxSizerLine->GetStaticBox(), wxID_STATIC, _("Corners:"), wxDefaultPosition, wxDefaultSize, 0);
	pGridBagSizerLine->Add(pStrokeJoinTxt, wxGBPosition(0, 2), wxGBSpan(1, 1), wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxBoxSizer* pStrokeJoinSizer = new wxBoxSizer(wxHORIZONTAL);
	pGridBagSizerLine->Add(pStrokeJoinSizer, wxGBPosition(0, 3), wxGBSpan(1, 1), wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

	m_pJoinMiterBt = new wxBitmapToggleButton(pStaticBoxSizerLine->GetStaticBox(), ID_JOINMITER_BT, wxBitmap(JoinMiter_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	m_pJoinMiterBt->SetBitmapDisabled(wxBitmap(JoinMiterDisabled_xpm));
	pStrokeJoinSizer->Add(m_pJoinMiterBt, 0, wxALIGN_CENTER_VERTICAL, 5);

	m_pJoinBevelBt = new wxBitmapToggleButton(pStaticBoxSizerLine->GetStaticBox(), ID_JOINBEVEL_BT, wxBitmap(JoinBevel_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	m_pJoinBevelBt->SetBitmapDisabled(wxBitmap(JoinBevelDisabled_xpm));
	pStrokeJoinSizer->Add(m_pJoinBevelBt, 0, wxALIGN_CENTER_VERTICAL, 5);

	m_pJoinRoundBt = new wxBitmapToggleButton(pStaticBoxSizerLine->GetStaticBox(), ID_JOINROUND_BT, wxBitmap(JoinRound_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	m_pJoinRoundBt->SetBitmapDisabled(wxBitmap(JoinRoundDisabled_xpm));
	pStrokeJoinSizer->Add(m_pJoinRoundBt, 0, wxALIGN_CENTER_VERTICAL, 5);

	wxStaticText* pStrokeColorTxt = new wxStaticText(pStaticBoxSizerLine->GetStaticBox(), wxID_STATIC, _("Colour:"), wxDefaultPosition, wxDefaultSize, 0);
	pGridBagSizerLine->Add(pStrokeColorTxt, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	m_pStrokeColorBox = new a2dColorComboBox(pStaticBoxSizerLine->GetStaticBox(), ID_STROKECOLOR_BOX, false, &m_CustomColors);
	pGridBagSizerLine->Add(m_pStrokeColorBox, wxGBPosition(1, 1), wxGBSpan(1, 1), wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* pStrokeCapTxt = new wxStaticText(pStaticBoxSizerLine->GetStaticBox(), wxID_STATIC, _("Caps:"), wxDefaultPosition, wxDefaultSize, 0);
	pGridBagSizerLine->Add(pStrokeCapTxt, wxGBPosition(1, 2), wxGBSpan(1, 1), wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxBoxSizer* pStrokeCapSizer = new wxBoxSizer(wxHORIZONTAL);
	pGridBagSizerLine->Add(pStrokeCapSizer, wxGBPosition(1, 3), wxGBSpan(1, 1), wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

	m_pCapButtBt = new wxBitmapToggleButton(pStaticBoxSizerLine->GetStaticBox(), ID_CAPBUTT_BT, wxBitmap(CapButt_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	m_pCapButtBt->SetBitmapDisabled(wxBitmap(CapButtDisabled_xpm));
	pStrokeCapSizer->Add(m_pCapButtBt, 0, wxALIGN_CENTER_VERTICAL, 5);

	m_pCapProjectingBt = new wxBitmapToggleButton(pStaticBoxSizerLine->GetStaticBox(), ID_CAPPROJECTING_BT, wxBitmap(CapProjecting_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	m_pCapProjectingBt->SetBitmapDisabled(wxBitmap(CapProjectingDisabled_xpm));
	pStrokeCapSizer->Add(m_pCapProjectingBt, 0, wxALIGN_CENTER_VERTICAL, 5);

	m_pCapRoundBt = new wxBitmapToggleButton(pStaticBoxSizerLine->GetStaticBox(), ID_CAPROUND_BT, wxBitmap(CapRound_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	m_pCapRoundBt->SetBitmapDisabled(wxBitmap(CapRoundDisabled_xpm));
	pStrokeCapSizer->Add(m_pCapRoundBt, 0, wxALIGN_CENTER_VERTICAL, 5);

	wxBoxSizer* pStrokeWidthSizer = new wxBoxSizer(wxHORIZONTAL);
	pGridBagSizerLine->Add(pStrokeWidthSizer, wxGBPosition(2, 0), wxGBSpan(1, 2), wxGROW|wxALIGN_CENTER_VERTICAL, 5);

	wxStaticText* pStrokeWidthTxt = new wxStaticText(pStaticBoxSizerLine->GetStaticBox(), wxID_STATIC, _("Stroke width (mm):"), wxDefaultPosition, wxDefaultSize, 0);
	pStrokeWidthSizer->Add(pStrokeWidthTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	m_pStrokeWidthCtrl = new wxSpinCtrlDouble(pStaticBoxSizerLine->GetStaticBox(), ID_STROKEWIDTH_CTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0.1, 5.0, 0.2, 0.1);
	m_pStrokeWidthCtrl->SetDigits(1);
	m_pStrokeWidthCtrl->SetValue(wxT(""));
	pStrokeWidthSizer->Add(m_pStrokeWidthCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	pStaticBoxSizerLine->Layout();

	//----------------------------
	// fill:
	//----------------------------
	wxStaticBox* pStaticBoxFill = new wxStaticBox(this, wxID_ANY, _("Fill"));
	wxStaticBoxSizer* pStaticBoxSizerFill = new wxStaticBoxSizer(pStaticBoxFill, wxVERTICAL);
	pTopSizer->Add(pStaticBoxSizerFill, 0, wxGROW|wxALL, 5);

	wxBoxSizer* pFillColorSizer = new wxBoxSizer(wxHORIZONTAL);
	pFillColorSizer->SetMinSize( pStrokeWidthSizer->GetSize() ); // at least same width as left 'line attributes' column
	pStaticBoxSizerFill->Add(pFillColorSizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxStaticText* pFillColorTxt = new wxStaticText( pStaticBoxSizerFill->GetStaticBox(), wxID_STATIC, _("Colour:"), wxDefaultPosition, wxDefaultSize, 0 );
	pFillColorSizer->Add(pFillColorTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	m_pFillBox = new a2dFillComboBox(pStaticBoxSizerFill->GetStaticBox(), ID_FILL_BOX, bAllowLayerStyle, &m_CustomColors);
	if(!m_bFill) m_pFillBox->Disable();
	pFillColorSizer->Add(m_pFillBox, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	//----------------------------
	// dialog buttons:
	//----------------------------
	wxStdDialogButtonSizer* pStdDlgBtSizer = new wxStdDialogButtonSizer;

	pTopSizer->Add(pStdDlgBtSizer, 0, wxGROW|wxALL, 5);
	wxButton* pOkButton = new wxButton( this, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
	pOkButton->SetDefault();
	pStdDlgBtSizer->AddButton(pOkButton);

	wxButton* pCancelButton = new wxButton( this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	pStdDlgBtSizer->AddButton(pCancelButton);

	pStdDlgBtSizer->Realize();
}

void a2dDialogStyle::OnInitDialog(wxInitDialogEvent& event)
{
	// -- set default stroke properties --
	a2dStroke defaultStroke(*wxBLACK, 0.3, a2dSTROKE_SOLID);
	defaultStroke.SetJoin(wxJOIN_MITER);
	defaultStroke.SetCap(wxCAP_BUTT);
	m_ExtStroke.SetDefault(defaultStroke);
	//------------------------------------

	UpdateStrokeData(false);

	if ( m_bFill && m_ExtFill.IsValid() )
		m_pFillBox->SetFill( m_ExtFill.Get() );
}

void a2dDialogStyle::UpdateStrokeData(bool bGetFromDlg)
{
	if ( bGetFromDlg )
	{
		if ( m_pSrokeStyleBox->IsStyleSelected() )
			m_ExtStroke.SetStyle( m_pSrokeStyleBox->GetStyle() );

		if ( m_pStrokeColorBox->IsColorSelected() )
			m_ExtStroke.SetColor( m_pStrokeColorBox->GetColor() );

#if wxCHECK_VERSION(3, 1, 6)
		if ( !m_pStrokeWidthCtrl->GetTextValue().IsEmpty() )
#elif defined(__WXGTK__)
		// Before wxWidgets-3.1.6 there was no suitable way with wxGTK
		// to check whether a value is selected in the spin control.
		if ( 1 ) //m_pStrokeWidthCtrl->GetValue() )
#else
		if ( !m_pStrokeWidthCtrl->GetText()->GetValue().IsEmpty() )
#endif
			m_ExtStroke.SetWidth( m_pStrokeWidthCtrl->GetValue() / m_UnitsScale );

		if ( m_pJoinMiterBt->GetValue() )
			m_ExtStroke.SetJoin( wxJOIN_MITER );
		else if ( m_pJoinBevelBt->GetValue() )
			m_ExtStroke.SetJoin( wxJOIN_BEVEL );
		else if ( m_pJoinRoundBt->GetValue() )
			m_ExtStroke.SetJoin( wxJOIN_ROUND );

		if ( m_pCapButtBt->GetValue() )
			m_ExtStroke.SetCap( wxCAP_BUTT );
		else if ( m_pCapProjectingBt->GetValue() )
			m_ExtStroke.SetCap( wxCAP_PROJECTING );
		else if ( m_pCapRoundBt->GetValue() )
			m_ExtStroke.SetCap( wxCAP_ROUND );
	}
	else
	{
		if ( m_ExtStroke.IsStyleValid() )
			m_pSrokeStyleBox->SetStyle( m_ExtStroke.GetStyle() );

		if ( m_ExtStroke.HasEditableStyle() )
		{
			m_pStrokeColorBox->Enable();
			if ( m_ExtStroke.IsColorValid() )
				m_pStrokeColorBox->SetColor( m_ExtStroke.GetColor() );

			m_pStrokeWidthCtrl->Enable();
			if ( m_ExtStroke.IsWidthValid() )
				m_pStrokeWidthCtrl->SetValue( m_ExtStroke.GetWidth() * m_UnitsScale );

			m_pJoinMiterBt->Enable();
			m_pJoinBevelBt->Enable();
			m_pJoinRoundBt->Enable();
			if ( m_ExtStroke.IsJoinValid() )
				SetJoin( m_ExtStroke.GetJoin() );

			m_pCapButtBt->Enable();
			m_pCapProjectingBt->Enable();
			m_pCapRoundBt->Enable();
			if ( m_ExtStroke.IsCapValid() )
				SetCap( m_ExtStroke.GetCap() );
		}
		else
		{
			m_pStrokeColorBox->Disable();
			m_pStrokeColorBox->SetSelection(-1);

			m_pStrokeWidthCtrl->Disable();
			m_pStrokeWidthCtrl->SetValue(wxT(""));

			m_pJoinMiterBt->Disable();
			m_pJoinMiterBt->SetValue(false);
			m_pJoinBevelBt->Disable();
			m_pJoinBevelBt->SetValue(false);
			m_pJoinRoundBt->Disable();
			m_pJoinRoundBt->SetValue(false);

			m_pCapButtBt->Disable();
			m_pCapButtBt->SetValue(false);
			m_pCapProjectingBt->Disable();
			m_pCapProjectingBt->SetValue(false);
			m_pCapRoundBt->Disable();
			m_pCapRoundBt->SetValue(false);
		}
	}
}

void a2dDialogStyle::OnStrokeStyle(wxCommandEvent& event)
{
	UpdateStrokeData(true);

	if ( m_ExtStroke.HasEditableStyle() && !m_ExtStroke.IsInitialised() )
	{	// initialise stroke with default properties
		a2dStroke defaultStroke = m_ExtStroke.GetDefault();
		defaultStroke.SetStyle(m_ExtStroke.GetStyle());
		m_ExtStroke.Set(defaultStroke);
	}

	UpdateStrokeData(false);
}

void a2dDialogStyle::OnJoin(wxCommandEvent& event)
{
	switch ( event.GetId() )
	{
		case ID_JOINMITER_BT:	SetJoin( wxJOIN_MITER );
								break;
		case ID_JOINBEVEL_BT:	SetJoin( wxJOIN_BEVEL );
								break;
		case ID_JOINROUND_BT:	SetJoin( wxJOIN_ROUND );
								break;
	};
}

void a2dDialogStyle::SetJoin(wxPenJoin join)
{
	switch ( join )
	{
		case wxJOIN_MITER:
				m_pJoinMiterBt->SetBitmap(wxBitmap(JoinMiterPressed_xpm));
				m_pJoinMiterBt->SetValue(true);
				m_pJoinBevelBt->SetBitmap(wxBitmap(JoinBevel_xpm));
				m_pJoinBevelBt->SetValue(false);
				m_pJoinRoundBt->SetBitmap(wxBitmap(JoinRound_xpm));
				m_pJoinRoundBt->SetValue(false);
				break;

		case wxJOIN_BEVEL:
				m_pJoinMiterBt->SetBitmap(wxBitmap(JoinMiter_xpm));
				m_pJoinMiterBt->SetValue(false);
				m_pJoinBevelBt->SetBitmap(wxBitmap(JoinBevelPressed_xpm));
				m_pJoinBevelBt->SetValue(true);
				m_pJoinRoundBt->SetBitmap(wxBitmap(JoinRound_xpm));
				m_pJoinRoundBt->SetValue(false);
				break;

		case wxJOIN_ROUND:
				m_pJoinMiterBt->SetBitmap(wxBitmap(JoinMiter_xpm));
				m_pJoinMiterBt->SetValue(false);
				m_pJoinBevelBt->SetBitmap(wxBitmap(JoinBevel_xpm));
				m_pJoinBevelBt->SetValue(false);
				m_pJoinRoundBt->SetBitmap(wxBitmap(JoinRoundPressed_xpm));
				m_pJoinRoundBt->SetValue(true);
				break;

		default:
				m_pJoinMiterBt->SetBitmap(wxBitmap(JoinMiter_xpm));
				m_pJoinMiterBt->SetValue(false);
				m_pJoinBevelBt->SetBitmap(wxBitmap(JoinBevel_xpm));
				m_pJoinBevelBt->SetValue(false);
				m_pJoinRoundBt->SetBitmap(wxBitmap(JoinRound_xpm));
				m_pJoinRoundBt->SetValue(false);
				break;
	};
}

void a2dDialogStyle::OnCap(wxCommandEvent& event)
{
	switch ( event.GetId() )
	{
		case ID_CAPBUTT_BT:			SetCap( wxCAP_BUTT );
									break;
		case ID_CAPPROJECTING_BT:	SetCap( wxCAP_PROJECTING );
									break;
		case ID_CAPROUND_BT:		SetCap( wxCAP_ROUND );
									break;
	};
}

void a2dDialogStyle::SetCap(wxPenCap cap)
{
	switch ( cap )
	{
		case wxCAP_BUTT:
				m_pCapButtBt->SetBitmap(wxBitmap(CapButtPressed_xpm));
				m_pCapButtBt->SetValue(true);
				m_pCapProjectingBt->SetBitmap(wxBitmap(CapProjecting_xpm));
				m_pCapProjectingBt->SetValue(false);
				m_pCapRoundBt->SetBitmap(wxBitmap(CapRound_xpm));
				m_pCapRoundBt->SetValue(false);
				break;

		case wxCAP_PROJECTING:
				m_pCapButtBt->SetBitmap(wxBitmap(CapButt_xpm));
				m_pCapButtBt->SetValue(false);
				m_pCapProjectingBt->SetBitmap(wxBitmap(CapProjectingPressed_xpm));
				m_pCapProjectingBt->SetValue(true);
				m_pCapRoundBt->SetBitmap(wxBitmap(CapRound_xpm));
				m_pCapRoundBt->SetValue(false);
				break;

		case wxCAP_ROUND:
				m_pCapButtBt->SetBitmap(wxBitmap(CapButt_xpm));
				m_pCapButtBt->SetValue(false);
				m_pCapProjectingBt->SetBitmap(wxBitmap(CapProjecting_xpm));
				m_pCapProjectingBt->SetValue(false);
				m_pCapRoundBt->SetBitmap(wxBitmap(CapRoundPressed_xpm));
				m_pCapRoundBt->SetValue(true);
				break;

		default:
				m_pCapButtBt->SetBitmap(wxBitmap(CapButt_xpm));
				m_pCapButtBt->SetValue(false);
				m_pCapProjectingBt->SetBitmap(wxBitmap(CapProjecting_xpm));
				m_pCapProjectingBt->SetValue(false);
				m_pCapRoundBt->SetBitmap(wxBitmap(CapRound_xpm));
				m_pCapRoundBt->SetValue(false);
				break;
	};
}

void a2dDialogStyle::OnOk(wxCommandEvent& event)
{
	//------------------------------------------------------------
	wxFocusEvent evt;                         // Hack to update value in case of pressing Enter in the spin control, 
#if defined(__WXMSW__)
	m_pStrokeWidthCtrl->OnTextLostFocus(evt); // because wxWidgets(2.9.4) updates its value only when losing focus.
#endif
	//------------------------------------
	UpdateStrokeData(true);

	if ( m_bFill && m_pFillBox->IsFillSelected() )
		m_ExtFill.Set( m_pFillBox->GetFill() );

	EndModal( wxID_OK );
}


wxUint32 a2dSetExtFill( a2dExtFill& returned, a2dCanvasObjectList* objects, a2dCanvasObjectFlagsMask mask, a2dLayers* layersetup )
{
    wxUint32 count = 0;

    forEachIn( a2dCanvasObjectList, objects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPolylineL* pline = wxDynamicCast( obj, a2dPolylineL );
        a2dSLine* line = wxDynamicCast( obj, a2dSLine );
        a2dEllipticArc* elArc = wxDynamicCast( obj, a2dEllipticArc );
        a2dArc* arc = wxDynamicCast( obj, a2dArc );

        if ( obj->GetFixedStyle() || obj->GetRelease() || !obj->CheckMask( mask ) ||
                pline || line || ( elArc && elArc->GetChord() ) || ( arc && arc->GetChord() )
           )
            continue;

        //if ( obj->GetFill().IsSameAs( *a2dTRANSPARENT_FILL ) )
        //    continue;

        //if ( obj->GetFill().IsSameAs( *a2dNullFILL ) )
        //    continue;

        obj->SetBin2( true );

        a2dFill fill = obj->GetFill();
        if ( (fill == *a2dNullFILL) && layersetup ) //layer like?
            fill = layersetup->GetFill( obj->GetLayer() );

        if ( count == 0 )
            returned.Set( fill );
        else
            returned.Mix( fill );

        count++;
    }

    return count;
}

wxUint32 a2dSetExtStroke( a2dExtStroke& returned, a2dCanvasObjectList* objects, a2dCanvasObjectFlagsMask mask, a2dLayers* layersetup )
{
    wxUint32 count = 0;

    forEachIn( a2dCanvasObjectList, objects )
    {
        a2dCanvasObject* obj = *iter;

        //optional can be resolved with SetFixedStyle() on certain ta wires (TaPin.cpp)
        //we do not want TaWire for Taco's to change in style, other ta wires are okay.
        //a2dWirePolylineL* wire = wxDynamicCast( obj, a2dWirePolylineL );
        //if ( wire )
        //    continue;

        if ( obj->GetFixedStyle() || obj->GetRelease() || !obj->CheckMask( mask ) )
            continue;

        //if ( obj->GetStroke().IsSameAs( *a2dTRANSPARENT_STROKE ) )
        //    continue;

        //if ( obj->GetStroke().IsSameAs( *a2dNullSTROKE ) )
        //    continue;

        obj->SetBin2( true );

        a2dStroke stroke = obj->GetStroke();
        if ( (stroke == *a2dNullSTROKE) && layersetup ) //layer like?
            stroke = layersetup->GetStroke( obj->GetLayer() );

        if ( count == 0 )
            returned.Set( stroke );
        else
            returned.Mix( stroke );

        count++;
    }

    return count;
}

