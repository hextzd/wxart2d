/*! \file editor/src/fontdlg.cpp
    \author Erik van der Wal

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: fontdlg.cpp,v 1.19 2008/08/02 13:46:07 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <string.h>
#include <stdlib.h>

#include "wx/canvas/fontdlg.h"
#include "wx/canvas/canglob.h"
#include "wx/artbase/drawer2d.h"
#include "wx/canvas/canvas.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/polygon.h"

#include <wx/tglbtn.h>
#include <wx/valnum.h>

// XPM images:
//------------

#include "../../art/resources/AlignTR.xpm"           // AlignTR_xpm
#include "../../art/resources/AlignTRPressed.xpm"    // AlignTRPressed_xpm
#include "../../art/resources/AlignT.xpm"            // AlignT_xpm
#include "../../art/resources/AlignTPressed.xpm"     // AlignTPressed_xpm
#include "../../art/resources/AlignTL.xpm"           // AlignTL_xpm
#include "../../art/resources/AlignTLPressed.xpm"    // AlignTLPressed_xpm
#include "../../art/resources/AlignR.xpm"            // AlignR_xpm
#include "../../art/resources/AlignRPressed.xpm"     // AlignRPressed_xpm
#include "../../art/resources/AlignC.xpm"            // AlignC_xpm
#include "../../art/resources/AlignCPressed.xpm"     // AlignCPressed_xpm
#include "../../art/resources/AlignL.xpm"            // AlignL_xpm
#include "../../art/resources/AlignLPressed.xpm"     // AlignLPressed_xpm
#include "../../art/resources/AlignBR.xpm"           // AlignBR_xpm
#include "../../art/resources/AlignBRPressed.xpm"    // AlignBRPressed_xpm
#include "../../art/resources/AlignB.xpm"            // AlignB_xpm
#include "../../art/resources/AlignBPressed.xpm"     // AlignBPressed_xpm
#include "../../art/resources/AlignBL.xpm"           // AlignBL_xpm
#include "../../art/resources/AlignBLPressed.xpm"    // AlignBLPressed_xpm


//===========================================================================
// a2dTextPropDlgExt
//===========================================================================

enum
{
	ID_STROKECOLOR_BOX = wxID_HIGHEST + 1,
	ID_FILL_BOX,
	wxID_FONTTYPE_LIST,
    wxID_FONTNAME_LIST,
    wxID_FONTSTYLE_LIST,
    wxID_FONTSIZE_EDIT,
    wxID_FONTSIZE_LIST,
    wxID_FONT_SEARCHPATH,
    wxID_FONTTYPE_OK,
    wxID_ALIGN_TL, wxID_ALIGN_T, wxID_ALIGN_TR,
    wxID_ALIGN_L,  wxID_ALIGN_C, wxID_ALIGN_R,
    wxID_ALIGN_BL, wxID_ALIGN_B, wxID_ALIGN_BR,
    wxID_CHECK_ALIGN,
	wxID_CHECK_FRAME,
    wxID_CHECK_BACKGROUND
};

#define a2dSHOW_ALL_SETTINGS
#define MAXSIZE 300

//--------------------------------------------------------
//	Event Table:
//--------------------------------------------------------
BEGIN_EVENT_TABLE( a2dTextPropDlgExt, wxDialog )
	//Stroke + Fill
	EVT_INIT_DIALOG(a2dTextPropDlgExt::OnInitDialog)
	EVT_BUTTON(wxID_OK, a2dTextPropDlgExt::OnOk)
	EVT_COMBOBOX(ID_STROKECOLOR_BOX, a2dTextPropDlgExt::OnColourChange)
	EVT_COMBOBOX(ID_FILL_BOX, a2dTextPropDlgExt::OnColourChange)
	//Font
	EVT_LISTBOX( wxID_FONTTYPE_LIST, a2dTextPropDlgExt::OnChangeFontType )
    EVT_LISTBOX( wxID_FONTNAME_LIST, a2dTextPropDlgExt::OnChangeFontName )
    EVT_LISTBOX( wxID_FONTSTYLE_LIST, a2dTextPropDlgExt::OnChangeFontStyle )
    EVT_LISTBOX( wxID_FONTSIZE_LIST, a2dTextPropDlgExt::OnChangeFontSizeList )
    EVT_TEXT( wxID_FONTSIZE_EDIT, a2dTextPropDlgExt::OnChangeFontSize )
    EVT_BUTTON( wxID_FONT_SEARCHPATH, a2dTextPropDlgExt::OnSearchPath )
    EVT_TOGGLEBUTTON( wxID_ALIGN_TL, a2dTextPropDlgExt::OnAlignment )
    EVT_TOGGLEBUTTON( wxID_ALIGN_T, a2dTextPropDlgExt::OnAlignment )
    EVT_TOGGLEBUTTON( wxID_ALIGN_TR, a2dTextPropDlgExt::OnAlignment )
    EVT_TOGGLEBUTTON( wxID_ALIGN_L, a2dTextPropDlgExt::OnAlignment )
    EVT_TOGGLEBUTTON( wxID_ALIGN_C, a2dTextPropDlgExt::OnAlignment )
    EVT_TOGGLEBUTTON( wxID_ALIGN_R, a2dTextPropDlgExt::OnAlignment )
    EVT_TOGGLEBUTTON( wxID_ALIGN_BL, a2dTextPropDlgExt::OnAlignment )
    EVT_TOGGLEBUTTON( wxID_ALIGN_B, a2dTextPropDlgExt::OnAlignment )
    EVT_TOGGLEBUTTON( wxID_ALIGN_BR, a2dTextPropDlgExt::OnAlignment )
    EVT_CHECKBOX( wxID_CHECK_ALIGN, a2dTextPropDlgExt::OnAlignment )
	EVT_CHECKBOX( wxID_CHECK_FRAME, a2dTextPropDlgExt::OnTextflags )
    EVT_CHECKBOX( wxID_CHECK_BACKGROUND, a2dTextPropDlgExt::OnTextflags )
    EVT_SIZE( a2dTextPropDlgExt::OnSize )
END_EVENT_TABLE()
//--------------------------------------------------------

a2dTextPropDlgExt::a2dTextPropDlgExt( a2dHabitat* habitat, wxWindow *parent, bool bGlobal, bool bFill, a2dFontInfoList *fontlist, bool bAllowLayerStyle)
	: wxDialog( parent, -1, (bGlobal) ? _("Global Font") : _("Font"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE ), m_bFill(bFill), m_UnitsScale(1), m_CustomColors(), m_ExtStroke(), m_ExtFill(), m_ExtFont()
{
    m_habitat = habitat;
	m_fonts = fontlist;

	//map all possible styles to make them translatable
	m_styles[ wxT( "Medium" ) ] =  _( "Medium" );
	m_styles[ wxT( "Medium Italic" ) ] =  _( "Medium Italic" );
	m_styles[ wxT( "Regular" ) ] = _( "Regular" );
	m_styles[ wxT( "Italic" ) ] = _( "Italic" );
	m_styles[ wxT( "Bold" ) ] =  _( "Bold" );
	m_styles[ wxT( "Bold Oblique" ) ] =  _( "Bold Oblique" );
	m_styles[ wxT( "Bold Italic" ) ] = _( "Bold Italic" );
	m_styles[ wxT( "Light" ) ] = _( "Light" );
	m_styles[ wxT( "Light Italic" ) ] = _( "Light Italic" );
	m_styles[ wxT( "Black" ) ] = _( "Black" );
	m_styles[ wxT( "Black Italic" ) ] = _( "Black Italic" );
	m_styles[ wxT( "Narrow" ) ] = _( "Narrow" );
	m_styles[ wxT( "Roman" ) ] =  _( "Roman" );
	m_styles[ wxT( "Condensed" ) ] = _( "Condensed" );
	m_styles[ wxT( "Condensed Italic" ) ] = _( "Condensed Italic" );
	m_styles[ wxT( "Condensed Bold" ) ] = _( "Condensed Bold" );
	m_styles[ wxT( "Condensed Oblique" ) ] = _( "Condensed Oblique" );
	m_styles[ wxT( "Condensed Bold Italic" ) ] = _( "Condensed Bold Italic" );
	m_styles[ wxT( "Condensed Bold Oblique" ) ] = _( "Condensed Bold Oblique" );
	m_styles[ wxT( "Poster Compressed" ) ] = _( "Poster Compressed" );
	m_styles[ wxT( "ExtraLight" ) ] = _( "ExtraLight" );
	m_styles[ wxT( "Oblique" ) ] = _( "Oblique" );
	m_styles[ wxT( "Book" ) ] = _( "Book" );
	m_styles[ wxT( "Demi" ) ] =  _( "Demi" );
	m_styles[ wxT( "Demi Oblique" ) ] =  _( "Demi Oblique" );
	m_styles[ wxT( "Demibold" ) ] =  _( "Demibold" );
	m_styles[ wxT( "Demibold Italic" ) ] =  _( "Demibold Italic" );
	m_styles[ wxT( "Demibold Roman" ) ] =  _( "Demibold Roman" );
	m_styles[ wxT( "Semilight" ) ] = _( "Semilight" );
	m_styles[ wxT( "Semibold" ) ] = _( "Semibold" );
	m_styles[ wxT( "Pixel Pen" ) ] = _( "Pixel Pen" );
	m_styles[ wxT( "Normalized Pen" ) ] = _( "Normalized Pen" );
	m_styles[ wxT( " 2% of height" ) ] = _( " 2% of height" );
	m_styles[ wxT( " 5% of height" ) ] = _( " 5% of height" );
	m_styles[ wxT( " 7% of height" ) ] = _( " 7% of height" );
	m_styles[ wxT( "10% of height" ) ] = _( "10% of height" );
	m_styles[ wxT( "15% of height" ) ] = _( "15% of height" );
	m_styles[ wxT( "20% of height" ) ] = _( "20% of height" );

	CreateControls(bAllowLayerStyle);
}

void a2dTextPropDlgExt::CreateControls(bool bAllowLayerStyle)
{
	m_pTopSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(m_pTopSizer);

	//----------------------------
	// font
	//----------------------------

    m_viewscale = 1.0;

    wxBoxSizer* propsizer = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer* sizesizer = new wxBoxSizer( wxVERTICAL );

	//Sizer to get correct width of type+border+name
	wxBoxSizer* getwidthsizer_l;
	getwidthsizer_l = new wxBoxSizer( wxHORIZONTAL );

	//Type
    wxBoxSizer* typesizer = new wxBoxSizer( wxVERTICAL );
	wxStaticText* typetext = new wxStaticText( this, -1, _( "Type:" ) );
    typesizer->Add( typetext , 0, wxLEFT | wxRIGHT | wxTOP, 5 );
    wxArrayString types;
    types.Add( _( "* (All types)" ) );
    for ( size_t i = 0; i < m_fonts->GetCount(); i++ )
    {
        wxString type = m_fonts->Item( i )->GetData()->GetType();
        size_t j;
        for ( j = 0; j < types.GetCount(); j++ )
        {
            if ( types.Item( j ) == type )
                break;
        }
        if ( j == types.GetCount() )
            types.Add( type );
    }
    m_typelist = new wxListBox( this, wxID_FONTTYPE_LIST, wxDefaultPosition, wxDefaultSize, types, wxLB_SORT | wxLB_ALWAYS_SB );
    typesizer->Add( m_typelist, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5 );
	getwidthsizer_l->Add( typesizer, 0, wxALL | wxEXPAND, 5 );
#ifndef a2dSHOW_ALL_SETTINGS
	typetext->Hide();		// not changable by user
	m_typelist->Hide();		// not changable by user
#endif // a2dSHOW_ALL_SETTINGS

	//Font
    wxBoxSizer* fontsizer = new wxBoxSizer( wxVERTICAL );
    fontsizer->Add( new wxStaticText( this, -1, _( "Font:" ) ), 0, wxLEFT | wxRIGHT | wxTOP, 5 );
    wxArrayString fonts = GetFontList( a2dDEFAULT_FONTTYPE );
	m_fontlist = new wxListBox( this, wxID_FONTNAME_LIST, wxDefaultPosition, wxDefaultSize, fonts, wxLB_SORT | wxLB_ALWAYS_SB  );
	m_fontlist->SetMinSize( m_fontlist->GetSize() );
    fontsizer->Add( m_fontlist, 1, wxLEFT | wxRIGHT | wxBOTTOM| wxEXPAND , 5 );
	getwidthsizer_l->Add( fontsizer, 0, wxALL | wxEXPAND, 5 );

	propsizer->Add( getwidthsizer_l, 0, wxEXPAND, 0 );

	//Sizer to get correct width of style+border+textsize
	wxBoxSizer* getwidthsizer_r;
	getwidthsizer_r = new wxBoxSizer( wxHORIZONTAL );

	//Style
    wxBoxSizer* stylesizer = new wxBoxSizer( wxVERTICAL );
    stylesizer->Add( new wxStaticText( this, -1, _( "Style:" ) ), 0, wxLEFT | wxRIGHT | wxTOP, 5 );
	wxString longeststyle = GetLongestStyle();
	wxArrayString styles;
	styles.Add( longeststyle );
    m_stylelist = new wxListBox( this, wxID_FONTSTYLE_LIST, wxDefaultPosition, wxSize( -1,-1 ), styles, wxLB_SORT | wxLB_ALWAYS_SB );
	m_stylelist->SetMinSize( m_stylelist->GetSize() );
    stylesizer->Add( m_stylelist, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5 );
    getwidthsizer_r->Add( stylesizer, 0, wxALL | wxEXPAND, 5 );

	//Size
	wxFloatingPointValidator<float> val( 1, NULL, wxNUM_VAL_DEFAULT );
    val.SetRange( 1, MAXSIZE );

    sizesizer->Add( new wxStaticText( this, -1, _( "Size:" ) ), 0, wxLEFT | wxRIGHT | wxTOP, 5 );
    wxArrayString sizes;
	for(float i=5 ; i<=12 ; i++)
		sizes.Add( wxString::Format("%.0f", i) );
	for(float i=14 ; i<=28 ; i=i+2)
		sizes.Add( wxString::Format("%.0f", i) );
	sizes.Add( wxString::Format("%.0f", 36.0) );
	sizes.Add( wxString::Format("%.0f", 48.0) );
	sizes.Add( wxString::Format("%.0f", 72.0) );
    m_sizelist = new wxListBox( this, wxID_FONTSIZE_LIST, wxDefaultPosition, wxDefaultSize, sizes, wxLB_ALWAYS_SB );
    m_sizeedit = new wxTextCtrl( this, wxID_FONTSIZE_EDIT, wxT( "" ), wxDefaultPosition, wxDefaultSize, 0, val );
	m_sizeedit->SetMinSize( wxSize(70, -1) );
    sizesizer->Add( m_sizeedit, 0, wxLEFT | wxRIGHT | wxEXPAND, 5 );
    sizesizer->Add( m_sizelist, 1, wxLEFT | wxRIGHT | wxBOTTOM| wxEXPAND , 5 );
    getwidthsizer_r->Add( sizesizer, 0, wxALL | wxEXPAND, 5 );

	propsizer->Add( getwidthsizer_r, 0, wxEXPAND, 0 );

    m_pTopSizer->Add( propsizer, 1, wxALL | wxEXPAND, 5 );
	getwidthsizer_r->Layout();		//to get correct size of listctrls
	getwidthsizer_l->Layout();		//to get correct size of listctrls
	
    wxBoxSizer* bottomsizer = new wxBoxSizer( wxHORIZONTAL );

	//----------------------------
	// Alignment + Sample
	//----------------------------
	wxStaticBoxSizer* pStaticBoxSizerAlignment = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _( "Alignment:" ) ), wxVERTICAL );
    wxGridSizer* alignsizer2 = new wxGridSizer( 3 );
	m_pAlignTL_Bt = new wxBitmapToggleButton(this, wxID_ALIGN_TL, wxBitmap(AlignTL_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	alignsizer2->Add( m_pAlignTL_Bt, 0, 0, 0 );
	m_pAlignT_Bt = new wxBitmapToggleButton(this, wxID_ALIGN_T, wxBitmap(AlignT_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	alignsizer2->Add( m_pAlignT_Bt, 0, 0, 0 );
	m_pAlignTR_Bt = new wxBitmapToggleButton(this, wxID_ALIGN_TR, wxBitmap(AlignTR_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	alignsizer2->Add( m_pAlignTR_Bt, 0, 0, 0 );
	m_pAlignL_Bt = new wxBitmapToggleButton(this, wxID_ALIGN_L, wxBitmap(AlignL_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	alignsizer2->Add( m_pAlignL_Bt, 0, 0, 0 );
	m_pAlignC_Bt = new wxBitmapToggleButton(this, wxID_ALIGN_C, wxBitmap(AlignC_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	alignsizer2->Add( m_pAlignC_Bt, 0, 0, 0 );
	m_pAlignR_Bt = new wxBitmapToggleButton(this, wxID_ALIGN_R, wxBitmap(AlignR_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	alignsizer2->Add( m_pAlignR_Bt, 0, 0, 0 );
	m_pAlignBL_Bt = new wxBitmapToggleButton(this, wxID_ALIGN_BL, wxBitmap(AlignBL_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	alignsizer2->Add( m_pAlignBL_Bt, 0, 0, 0 );
	m_pAlignB_Bt = new wxBitmapToggleButton(this, wxID_ALIGN_B, wxBitmap(AlignB_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	alignsizer2->Add( m_pAlignB_Bt, 0, 0, 0 );
	m_pAlignBR_Bt = new wxBitmapToggleButton(this, wxID_ALIGN_BR, wxBitmap(AlignBR_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	alignsizer2->Add( m_pAlignBR_Bt, 0, 0, 0 );

	//Align on bbox
    pStaticBoxSizerAlignment->Add( alignsizer2, 0, wxALL | wxEXPAND, 5 );
    m_aligncheck = new wxCheckBox( this, wxID_CHECK_ALIGN, _( "Align on bbox" ) );
    pStaticBoxSizerAlignment->Add( m_aligncheck, 0, 0 );
	bottomsizer->Add( pStaticBoxSizerAlignment, 0, wxALL | wxEXPAND, 5 );
#ifndef a2dSHOW_ALL_SETTINGS
	m_aligncheck->Hide();		// not changable by user
#endif // a2dSHOW_ALL_SETTINGS


	//Sample
	wxStaticBoxSizer* pStaticBoxSizerPreview = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _( "Sample:" ) ), wxVERTICAL );
    m_canvas = new a2dCanvas( this, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER );
    m_canvas->SetYaxis( true );
    m_canvas->SetScaleOnResize( false );
    a2dCanvasObject* root = m_canvas->GetDrawing()->GetRootObject();
    m_canvastext = new a2dText( _T( "AaBbCc12" ) );
    m_canvastext->SetPosXY( 0.0, 0.0 );
    m_canvastext->SetFill( *a2dTRANSPARENT_FILL );
    m_canvastext->SetStroke( *wxBLACK );
    root->Append( m_canvastext );
    m_canvasline1 = new a2dSLine( 0.0, 0.0, 1.0, 0.0 );
    m_canvasline1->SetStroke( wxColour( 0xc0, 0xc0, 0xc0 ), 1 );
    root->Append( m_canvasline1 );
    m_canvasline2 = new a2dSLine( 0.0, 0.0, 0.0, 1.0 );
    m_canvasline2->SetStroke( wxColour( 0xc0, 0xc0, 0xc0 ), 1 );
    root->Append( m_canvasline2 );
    pStaticBoxSizerPreview->Add( m_canvas, 1, wxALL | wxEXPAND, 5 );
	bottomsizer->Add(pStaticBoxSizerPreview, 1, wxALL | wxEXPAND, 5 );

    m_pTopSizer->Add( bottomsizer, 0, wxALL | wxEXPAND, 5 );

	
	//----------------------------
	// colours:
	//----------------------------
	wxBoxSizer* coloursizer;
	coloursizer = new wxBoxSizer( wxHORIZONTAL );

	int spacerwidth=10;
	
	//Textcolour
	wxBoxSizer* textcoloursizer;
	textcoloursizer = new wxBoxSizer( wxVERTICAL );
	wxStaticText* pStrokeColorTxt = new wxStaticText( this, wxID_ANY, _( "Text colour:" ), wxDefaultPosition, wxDefaultSize, 0 );
	textcoloursizer->Add( pStrokeColorTxt, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	m_pStrokeColorBox = new a2dColorComboBox(this, ID_STROKECOLOR_BOX, false, &m_CustomColors);
	m_pStrokeColorBox->SetMinSize( wxSize( getwidthsizer_l->GetMinSize().GetWidth()-spacerwidth*2, -1) );
	textcoloursizer->Add( m_pStrokeColorBox, 0, wxALL, 5 );

	//Frame around text
	m_textframe = new wxCheckBox( this, wxID_CHECK_FRAME, _( "Frame around text" ) );
    textcoloursizer->Add( m_textframe, 0, wxALL, 5 );
#ifndef a2dSHOW_ALL_SETTINGS
	m_textframe->Hide();		// not changable by user
#endif // a2dSHOW_ALL_SETTINGS
	
	coloursizer->Add( textcoloursizer, 0, 0, 0 );

	coloursizer->Add( spacerwidth, 0, 0, 0, 5 );
	
	//Backgroundcolour
	wxBoxSizer* backgroundcoloursizer;
	backgroundcoloursizer = new wxBoxSizer( wxVERTICAL );
	wxStaticText* pFillColorTxt = new wxStaticText( this, wxID_ANY, _( "Background colour:" ), wxDefaultPosition, wxDefaultSize, 0 );
	backgroundcoloursizer->Add( pFillColorTxt, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	m_pFillBox = new a2dFillComboBox(this, ID_FILL_BOX, bAllowLayerStyle, &m_CustomColors);
	m_pFillBox->SetMinSize( wxSize( getwidthsizer_r->GetMinSize().GetWidth()-spacerwidth*2, -1) );
	if(!m_bFill) m_pFillBox->Disable();
	backgroundcoloursizer->Add( m_pFillBox, 0, wxALL, 5 );

	//Fill text background
    m_textbackground = new wxCheckBox( this, wxID_CHECK_BACKGROUND, _( "Fill text background" ) );
    backgroundcoloursizer->Add( m_textbackground, 0, wxALL, 5 );
#ifndef a2dSHOW_ALL_SETTINGS
	m_textbackground->Hide();	// not changable by user
#endif // a2dSHOW_ALL_SETTINGS
	
	coloursizer->Add( backgroundcoloursizer, 0, 0, 0 );

	m_pTopSizer->Add( coloursizer, 0, wxALL, 10 );

	wxBoxSizer* buttonsizer = new wxBoxSizer( wxHORIZONTAL );
#ifdef a2dSHOW_ALL_SETTINGS
	buttonsizer->Add( new wxButton( this, wxID_FONT_SEARCHPATH, _( "Search Path" ) ), 0, wxALL, 5 );			// not changable by user
#endif // a2dSHOW_ALL_SETTINGS
	buttonsizer->Add( new wxButton( this, wxID_OK, _( "OK" ) ), 0, wxALL, 5 );
    buttonsizer->Add( new wxButton( this, wxID_CANCEL, _( "Cancel" ) ), 0, wxALL, 5 );
	m_pTopSizer->Add( buttonsizer, 0, wxALL|wxALIGN_RIGHT, 5 );
}

void a2dTextPropDlgExt::OnInitDialog(wxInitDialogEvent& event)
{
	// -- set default stroke properties --
	a2dStroke defaultStroke(*wxBLACK, 0.3, a2dSTROKE_SOLID);
	defaultStroke.SetJoin(wxJOIN_MITER);
	defaultStroke.SetCap(wxCAP_BUTT);
	m_ExtStroke.SetDefault(defaultStroke);
	//------------------------------------

	// data to dialog
	UpdateStrokeData(false);

	if(m_bFill && m_ExtFill.IsValid())
		m_pFillBox->SetFill(m_ExtFill.Get());

	// data to dialog
	UpdateFontData();

	GetSizer()->SetSizeHints(this);
	
	Centre();
}

void a2dTextPropDlgExt::UpdateStrokeData(bool bGetFromDlg)
{
	if(bGetFromDlg)
	{
		if(m_pStrokeColorBox->IsColorSelected())
			m_ExtStroke.SetColor(m_pStrokeColorBox->GetColor());
	}
	else
	{
		if(m_ExtStroke.HasEditableStyle())
		{
			m_pStrokeColorBox->Enable();
			if(m_ExtStroke.IsColorValid())
				m_pStrokeColorBox->SetColor(m_ExtStroke.GetColor());
		}
		else
		{
			m_pStrokeColorBox->Disable();
			m_pStrokeColorBox->SetSelection(-1);
		}
	}
}

void a2dTextPropDlgExt::UpdateFontData()
{
	//update size
	if(m_ExtFont.IsSizeValid())
	{
		wxString sizetext;
		double size = ( ( m_ExtFont.GetFont().GetFontInfo().GetSize() * m_UnitsScale ) / 25.4 ) * 72;		//convert from world size (1/100 mm) to font size (1/72 inch)
		int remainder = int( ( size+0.05 ) *10 ) % 10;	// calc remainder to find out if necessary to show decimal places
		if( remainder != 0 )
			sizetext.Printf( wxT( "%.1f" ), size  );
		else
			sizetext.Printf( wxT( "%.0f" ), size  );

		m_sizeedit->SetValue( sizetext );
		int itemnr = m_sizelist->FindString( sizetext );
		m_sizelist->SetSelection( itemnr );
		if( itemnr != -1 )
			m_sizelist->EnsureVisible( itemnr );
	}

	//update type
	if(m_ExtFont.IsTypeValid())
	{
		if ( m_typelist->GetCount() )
		{
			int idx = 0;
			idx = m_typelist->FindString( m_ExtFont.GetFont().GetFontInfo().GetType() );
			if ( idx == wxNOT_FOUND )
				idx = 0;
			m_typelist->SetSelection( idx );
		}
	}
	else
	{
#ifndef a2dSHOW_ALL_SETTINGS // if user cannot change type, then set all to default type
		int idx = 0;
		idx = m_typelist->FindString( a2dDEFAULT_FONTTYPE );
		if( idx == -1 )
			wxFAIL;
		else
		{
			m_typelist->SetSelection( idx );
			m_ExtFont.SetType( a2dDEFAULT_FONTTYPE );
		}
#endif
	}

	if(m_ExtFont.IsAlignmentValid())
	{
		//update alignment
		SetAlignment( m_ExtFont.GetAlignment() );

		//update align on bbox checkbox
		m_aligncheck->SetValue( ( m_ExtFont.GetAlignment() & wxBASELINE ) > 0  );
	}

	if(m_ExtFont.IsTextflagsValid())
	{
		//update textframe checkbox
		m_textframe->SetValue( ( m_ExtFont.GetTextFlags() & a2dText::a2dCANOBJTEXT_FRAME ) > 0 );

		//update background checkbox
		m_textbackground->SetValue( ( m_ExtFont.GetTextFlags() & a2dText::a2dCANOBJTEXT_BACKGROUND ) > 0 );
	}

	if(m_ExtFont.IsNameValid())
	{
		//update font
		wxString oldname = m_fontlist->GetStringSelection();
		
		if ( m_fontlist->GetCount() )
		{
			int idx;
			idx = m_fontlist->FindString( m_ExtFont.GetFont().GetFontInfo().GetName() );
			if( idx == -1 )
				idx = m_fontlist->FindString( oldname );
			if ( idx == wxNOT_FOUND )
				idx = 0;
			m_fontlist->SetSelection( idx );
			if( idx != -1 )
				m_fontlist->EnsureVisible( idx );
		}
	}

	//update the rest
	wxCommandEvent dummy;
	if( m_ExtFont.GetFont().GetFontInfo().GetType() != a2dDEFAULT_FONTTYPE )		//when type different to initialized-defaulttype, refill box
		OnChangeFontType( dummy );
	else
		OnChangeFontName( dummy );
}

void a2dTextPropDlgExt::OnOk(wxCommandEvent& event)
{
	if(m_FontFound == false)
	{
		// ----------if no style selected, get only name and size-------------
		wxString type = m_typelist->GetStringSelection();
		wxString name = m_fontlist->GetStringSelection();
		wxString style;
		wxStringToStringHashMap::const_iterator it;
		for( it = m_styles.begin(); it != m_styles.end(); ++it )
		{
			if( it->second == m_stylelist->GetStringSelection() )
			{
				style = it->first;
				break;
			}
		}
		wxArrayString fonts;
		for ( size_t i = 0; i < m_fonts->GetCount(); i++ )
		{
			wxString fonttype = m_fonts->Item( i )->GetData()->GetType();
			wxString fontname = m_fonts->Item( i )->GetData()->GetName();
			wxString fontstyle = m_fonts->Item( i )->GetData()->GetStyle();
			if ( ( fonttype == type || type == _( "* (All types)" ) )		// Special case: when style is invalid and font is selected 
					&& fontname == name )
			{
				double size;
				m_sizeedit->GetValue().ToDouble( &size );
				a2dFontInfo fontinfo = *m_fonts->Item( i )->GetData();
			
				if( size < 1 || size > MAXSIZE )		// size out of range?
					size = ( ( m_ExtFont.GetFont().GetFontInfo().GetSize() * m_UnitsScale ) / 25.4 ) * 72;		//convert from world size (1/100 mm) to font size (1/72 inch)

				int round = float ( ( ( ( ( size ) / 72 ) * 25.4 ) / m_UnitsScale ) + 0.5 );	//convert from font size (1/72 inch) to world size (1/100 mm)
				fontinfo.SetSize( round );
				m_ExtFont.SetFont( a2dFont::CreateFont( fontinfo ) );
				m_ExtFont.SetStyleValidity( false );
			}
		}
		// -------------------------------------------------------------------
	}

	UpdateStrokeData(true);

	if(m_bFill && m_pFillBox->IsFillSelected())
		m_ExtFill.Set(m_pFillBox->GetFill());

	EndModal(wxID_OK);
}

void a2dTextPropDlgExt::OnChangeFontType( wxCommandEvent& event )
{
    wxString type = m_typelist->GetStringSelection();
    wxString oldname = m_fontlist->GetStringSelection();

	while ( m_fontlist->GetCount() )
        m_fontlist->Delete( 0 );

    m_fontlist->Append( GetFontList( type ) );

	if ( m_fontlist->GetCount() )
	{
		if( m_ExtFont.IsNameValid() )
		{
			int idx;
			idx = m_fontlist->FindString( m_ExtFont.GetFont().GetFontInfo().GetName() );
			if( idx == -1 )
				idx = m_fontlist->FindString( oldname );
			if ( idx == wxNOT_FOUND )
				idx = 0;
			m_fontlist->SetSelection( idx );
			if( idx != -1 )
				m_fontlist->EnsureVisible( idx );
		}
	}
	
    OnChangeFontName( event );
}

void a2dTextPropDlgExt::OnChangeFontName( wxCommandEvent& event )
{
    wxString type = m_typelist->GetStringSelection();
    wxString name = m_fontlist->GetStringSelection();
	wxString oldstyle;
	wxStringToStringHashMap::const_iterator it;
	for( it = m_styles.begin(); it != m_styles.end(); ++it )
	{
		if( it->second == m_stylelist->GetStringSelection() )
		{
			oldstyle = it->first;
			break;
		}
	}

    while ( m_stylelist->GetCount() )
        m_stylelist->Delete( 0 );

	if( name != "" )		//only when font selected
	{
		m_ExtFont.SetName(name);
		wxArrayString fonts;
		for ( size_t i = 0; i < m_fonts->GetCount(); i++ )
		{
			wxString fontname = m_fonts->Item( i )->GetData()->GetName();
			wxString fonttype = m_fonts->Item( i )->GetData()->GetType();
			wxString fontstyle = m_fonts->Item( i )->GetData()->GetStyle();

			if ( ( fonttype == type || type == _( "* (All types)" ) )
					&& fontname == name )
			{
				size_t j;
				wxStringToStringHashMap::const_iterator it = m_styles.find( fontstyle );
				if( it != m_styles.end())
				{
					for ( j = 0; j < fonts.GetCount(); j++ )
					{
						if ( fonts.Item( j ) == it->second )
							break;
					}
					if ( j == fonts.GetCount() )
					{
						fonts.Add( it->second );
					}
				}
				else
					wxFAIL_MSG( wxString::Format("Fontstyle \"%s\" is unknown.", fontstyle) );
			}
		}

		m_stylelist->Append( fonts );
	}
	else		//when no font selected -> only insert common styles
	{
		wxArrayString mapped_styles, common_styles = m_ExtFont.GetCommonStyle();

		for( int i=0; i < common_styles.GetCount(); i++ )
		{
			wxStringToStringHashMap::const_iterator it = m_styles.find( common_styles[i] );
			if( it != m_styles.end())
			{
				mapped_styles.Add( it->second );
			}
		}
		m_stylelist->Append( mapped_styles );
	}

	
	if ( m_stylelist->GetCount() )
    {
		if( m_ExtFont.IsStyleValid() )
		{
			int idx;

			wxStringToStringHashMap::const_iterator it = m_styles.find( m_ExtFont.GetFont().GetFontInfo().GetStyle() );
			if( it != m_styles.end())
				idx = m_stylelist->FindString( it->second );
			else
			{
				wxFAIL_MSG( wxString::Format("Fontstyle \"%s\" is unknown.", m_ExtFont.GetFont().GetFontInfo().GetStyle()) );
				idx = -1;
			}

			if( idx != wxNOT_FOUND )
			{
				m_stylelist->SetSelection( idx );
				m_ExtFont.SetStyle( m_ExtFont.GetFont().GetFontInfo().GetStyle() );
			}
		}
    }
	OnChangeFont();
}

void a2dTextPropDlgExt::OnChangeFontStyle( wxCommandEvent& event )
{
	wxStringToStringHashMap::const_iterator it;
	for( it = m_styles.begin(); it != m_styles.end(); ++it )
	{
		if( it->second == m_stylelist->GetStringSelection() )
		{
			if( !m_ExtFont.IsNameValid() )
			{
				//Extra must be set empty, in case of different font names
				//Otherwise creating the a2dFont object fails and style is not taken over
				a2dFontInfo fontinfo=m_ExtFont.GetFont().GetFontInfo();
				fontinfo.SetStyle( it->first );
				fontinfo.SetExtra( wxT("") );
				m_ExtFont.SetFont( a2dFont::CreateFont( fontinfo ) );
			}

			m_ExtFont.SetStyle( it->first );
			break;
		}
	}

    OnChangeFont();
}

void a2dTextPropDlgExt::OnChangeFontSize( wxCommandEvent& event )
{
    m_sizelist->SetSelection( wxNOT_FOUND );
    OnChangeFont();
}

void a2dTextPropDlgExt::OnChangeFontSizeList( wxCommandEvent& event )
{
    int idx = m_sizelist->GetSelection();
    m_sizeedit->SetValue( m_sizelist->GetStringSelection() );
    m_sizelist->SetSelection( idx );

	double size;
	m_sizelist->GetStringSelection().ToDouble( &size );
	int round = float ( ( ( ( ( size ) / 72 ) * 25.4 ) / m_UnitsScale ) + 0.5 );	//convert from font size (1/72 inch) to world size (1/100 mm)
	m_ExtFont.SetSize( round );
    OnChangeFont();
}

void a2dTextPropDlgExt::OnChangeFont()
{
    wxString type = m_typelist->GetStringSelection();
    wxString name = m_fontlist->GetStringSelection();
    wxString style;
	wxStringToStringHashMap::const_iterator it;
	for( it = m_styles.begin(); it != m_styles.end(); ++it )
	{
		if( it->second == m_stylelist->GetStringSelection() )
		{
			style = it->first;
			break;
		}
	}
	
	wxArrayString fonts;
	m_FontFound = false;

	bool use_temp_font = false;
	a2dFontInfo valid_font, temp_font;
	for ( size_t i = 0; i < m_fonts->GetCount(); i++ )
	{
		wxString fonttype = m_fonts->Item( i )->GetData()->GetType();
		wxString fontname = m_fonts->Item( i )->GetData()->GetName();
		wxString fontstyle = m_fonts->Item( i )->GetData()->GetStyle();
		if ( ( fonttype == type || type == _( "* (All types)" ) )
				&& fontname == name && fontstyle == style )
		{
			valid_font = *m_fonts->Item( i )->GetData();
			m_FontFound = true;
			break;
		}

		if ( ( fonttype == type || type == _( "* (All types)" ) )
				&& fontname == name && style == wxT("") )
		{
			if ( !use_temp_font || fontstyle == wxT("Regular"))
			{
				use_temp_font = true;
				temp_font = *m_fonts->Item( i )->GetData();
			}
		}
	}

	//-------------------Prepare sample------------------------

	double size = 1.0;
	if( ! m_sizeedit->GetValue().ToDouble( &size ) // size invalid or
	    || size < 1 || size > MAXSIZE )	           // out of range?
	{
		size = ( ( m_ExtFont.GetFont().GetFontInfo().GetSize() * m_UnitsScale ) / 25.4 ) * 72;		//convert from  world size (1/100 mm) to font size (1/72 inch)
	}

	// Prepare font for sample 
	if( m_FontFound )
	{
		int round = float ( ( ( ( ( size ) / 72 ) * 25.4 ) / m_UnitsScale ) + 0.5 );		//convert from font size (1/72 inch) to world size (1/100 mm)
		valid_font.SetSize( round );
		m_ExtFont.SetFont( a2dFont::CreateFont( valid_font ) );

		m_canvastext->SetFont( m_ExtFont.GetFont() );
	}
	else
	{
		if( use_temp_font )
		{
			int round = float ( ( ( ( ( size ) / 72 ) * 25.4 ) / m_UnitsScale ) + 0.5 );		//convert from font size (1/72 inch) to world size (1/100 mm)
			temp_font.SetSize( round );
			m_canvastext->SetFont( a2dFont::CreateFont( temp_font ) );
		}
		else
			m_canvastext->SetFont( m_habitat->GetTextTemplateObject()->GetFont() );
	}

	m_canvastext->SetPending( true );

	// Prepare alignment and textflags for sample
	if( m_ExtFont.IsAlignmentValid() )
	{
		m_canvastext->SetAlignment( m_ExtFont.GetAlignment() );
		m_canvastext->SetTextFlags( m_ExtFont.GetTextFlags() );
	}
	else
	{
		m_canvastext->SetAlignment( wxMAXY | wxMINX | wxBASELINE_CONTRA );
		m_canvastext->SetTextFlags( a2dText::a2dCANOBJTEXT_BACKGROUND );
	}

	// Prepare stroke for sample
	if( m_pStrokeColorBox->IsColorSelected() )		
		m_canvastext->SetStroke( m_pStrokeColorBox->GetColor() );
	else	// if mixed and not the same colour, set wxBLACK as default
		m_canvastext->SetStroke( wxColour( *wxBLACK ) );

	a2dBoundingBox bbox = m_canvastext->GetCalculatedBoundingBox( 1 );
	RepaintSample();
	//------------------------------------------------------------

}

void a2dTextPropDlgExt::OnSearchPath( wxCommandEvent& event )
{
    a2dFontSearchPathDialog dlg;
    if ( dlg.ShowModal() == wxID_OK )
    {
        m_fonts->Clear();
        a2dFont::GetInfoList( *m_fonts );

        wxString oldtype = m_typelist->GetStringSelection();

        while ( m_typelist->GetCount() )
            m_typelist->Delete( 0 );
        wxArrayString types;
        types.Add( _( "* (All types)" ) );
        for ( size_t i = 0; i < m_fonts->GetCount(); i++ )
        {
            wxString type = m_fonts->Item( i )->GetData()->GetType();
            size_t j;
            for ( j = 0; j < types.GetCount(); j++ )
            {
                if ( types.Item( j ) == type )
                    break;
            }
            if ( j == types.GetCount() )
                types.Add( type );
        }
        m_typelist->Append( types );

        if ( m_typelist->GetCount() )
        {
            int idx = m_typelist->FindString( oldtype );
            if ( idx == wxNOT_FOUND )
                idx = 0;
            m_typelist->SetSelection( idx );
        }
        OnChangeFontType( event );
    }
}

void a2dTextPropDlgExt::OnAlignment( wxCommandEvent& event )
{
    switch( event.GetId() )
    {
		case wxID_ALIGN_TL: m_ExtFont.SetAlignment( wxMAXY | wxMINX ); break;
        case wxID_ALIGN_T: m_ExtFont.SetAlignment( wxMAXY ); break;
        case wxID_ALIGN_TR: m_ExtFont.SetAlignment( wxMAXY | wxMAXX ); break;
        case wxID_ALIGN_L: m_ExtFont.SetAlignment( wxMINX ); break;
        case wxID_ALIGN_C: m_ExtFont.SetAlignment( wxMIDX | wxMIDY ); break;
        case wxID_ALIGN_R: m_ExtFont.SetAlignment( wxMAXX ); break;
        case wxID_ALIGN_BL: m_ExtFont.SetAlignment( wxMINY | wxMINX ); break;
        case wxID_ALIGN_B: m_ExtFont.SetAlignment( wxMINY ); break;
        case wxID_ALIGN_BR: m_ExtFont.SetAlignment( wxMINY | wxMAXX ); break;
        default: ;
    }

	if ( m_aligncheck->IsChecked() )
	{
		m_ExtFont.SetAlignment( m_ExtFont.GetAlignment() | wxBASELINE );
		m_ExtFont.SetAlignment( m_ExtFont.GetAlignment() & ~wxBASELINE_CONTRA );
	}
    else
	{
        m_ExtFont.SetAlignment( m_ExtFont.GetAlignment() | wxBASELINE_CONTRA );
		m_ExtFont.SetAlignment( m_ExtFont.GetAlignment() & ~wxBASELINE );
	}

	SetAlignment( m_ExtFont.GetAlignment() );

    OnChangeFont();
}

void a2dTextPropDlgExt::OnTextflags( wxCommandEvent& event )
{
    if ( m_textframe->IsChecked() )
		m_ExtFont.SetTextFlags( m_ExtFont.GetTextFlags() | a2dText::a2dCANOBJTEXT_FRAME );
    else
        m_ExtFont.SetTextFlags( m_ExtFont.GetTextFlags() & ~a2dText::a2dCANOBJTEXT_FRAME );

    if ( m_textbackground->IsChecked() )
        m_ExtFont.SetTextFlags( m_ExtFont.GetTextFlags() | a2dText::a2dCANOBJTEXT_BACKGROUND );
    else
        m_ExtFont.SetTextFlags( m_ExtFont.GetTextFlags() & ~a2dText::a2dCANOBJTEXT_BACKGROUND );

    OnChangeFont();
}

void a2dTextPropDlgExt::OnSize( wxSizeEvent& event )
{
    event.Skip();
	RepaintSample();
}

void a2dTextPropDlgExt::OnColourChange( wxCommandEvent& event )
{
	OnChangeFont();
}

// Returns a list of fonts for the given type
wxArrayString a2dTextPropDlgExt::GetFontList(wxString type)
{
	wxArrayString fonts;
    for ( size_t i = 0; i < m_fonts->GetCount(); i++ )
    {
        wxString fonttype = m_fonts->Item( i )->GetData()->GetType();
        if ( fonttype == type || type == _( "* (All types)" ) )
        {
            wxString fontname = m_fonts->Item( i )->GetData()->GetName();
            size_t j;
            for ( j = 0; j < fonts.GetCount(); j++ )
            {
                if ( fonts.Item( j ) == fontname )
                    break;
            }
            if ( j == fonts.GetCount() )
                fonts.Add( fontname );
        }
    }
	return fonts;
}

// Returns the longest style-string of all fonts (needed to initialize the size of style-listbox)
wxString a2dTextPropDlgExt::GetLongestStyle()
{
	wxString longeststyle = "";
	for ( size_t i = 0; i < m_fonts->GetCount(); i++ )
	{
		wxString fontname = m_fonts->Item( i )->GetData()->GetName();
		wxString fonttype = m_fonts->Item( i )->GetData()->GetType();
		wxString fontstyle = m_fonts->Item( i )->GetData()->GetStyle();

		wxStringToStringHashMap::const_iterator it = m_styles.find( fontstyle );
		if( it != m_styles.end())
		{
			if( it->second.Length() > longeststyle.Length() )		//find out longest style-string to initialize listbox with correct width
				longeststyle = it->second;
		}
	}
	return longeststyle;
}

void a2dTextPropDlgExt::RepaintSample()
{
	a2dBoundingBox bbox = m_canvastext->GetCalculatedBoundingBox( 1 );

	m_canvasline1->SetPosXY12( -2.0 * bbox.GetWidth(), 0.0, 2.0 * bbox.GetWidth(), 0.0 );
    m_canvasline2->SetPosXY12( 0.0, -2.0 * bbox.GetWidth(), 0.0, 2.0 * bbox.GetWidth() );

    // To have some better visual feedback of alignment, move the text around a little.
    a2dBoundingBox bboxnew = bbox;
    bboxnew.Enlarge( bbox.GetHeight() / 6.0 );
	if ( m_ExtFont.GetAlignment() & wxMINX )
        bboxnew.Translate( -bbox.GetHeight() / 8.0, 0.0 );
    if ( m_ExtFont.GetAlignment() & wxMAXX )
        bboxnew.Translate( bbox.GetHeight() / 8.0, 0.0 );

    if ( m_ExtFont.GetAlignment() & wxMINY )
        bboxnew.Translate( 0.0, -bbox.GetHeight() / 8.0 );
    if ( m_ExtFont.GetAlignment() & wxMAXY )
        bboxnew.Translate( 0.0, bbox.GetHeight() / 8.0 );

	if( m_pFillBox->GetFill().IsNoFill() )		// if mixed and not the same colour, set a2dTRANSPARENT_FILL as default
		m_canvastext->SetFill( *a2dTRANSPARENT_FILL );
	else
	{
		if ( m_ExtFont.GetAlignment() & wxBASELINE )
			m_canvastext->SetFill( m_pFillBox->GetFill() );
		if ( m_ExtFont.GetAlignment() & wxBASELINE_CONTRA )
			m_canvastext->SetFill( m_pFillBox->GetFill() );
		else
			m_canvastext->SetFill( *a2dTRANSPARENT_FILL );
	}

    m_canvas->SetMappingWidthHeight ( bboxnew.GetMinX(), bboxnew.GetMinY(),
                                      bboxnew.GetWidth(), bboxnew.GetHeight(), false );
    m_canvas->Refresh();
}

// Sets the correct alignment button
void a2dTextPropDlgExt::SetAlignment(int align)
{
	switch(align & 15)
	{
		case wxMAXY | wxMINX:
				m_pAlignTL_Bt->SetBitmap(wxBitmap(AlignTLPressed_xpm));
				m_pAlignTL_Bt->SetValue(true);
				m_pAlignT_Bt->SetBitmap(wxBitmap(AlignT_xpm));
				m_pAlignT_Bt->SetValue(false);
				m_pAlignTR_Bt->SetBitmap(wxBitmap(AlignTR_xpm));
				m_pAlignTR_Bt->SetValue(false);
				m_pAlignL_Bt->SetBitmap(wxBitmap(AlignL_xpm));
				m_pAlignL_Bt->SetValue(false);
				m_pAlignC_Bt->SetBitmap(wxBitmap(AlignC_xpm));
				m_pAlignC_Bt->SetValue(false);
				m_pAlignR_Bt->SetBitmap(wxBitmap(AlignR_xpm));
				m_pAlignR_Bt->SetValue(false);
				m_pAlignBL_Bt->SetBitmap(wxBitmap(AlignBL_xpm));
				m_pAlignBL_Bt->SetValue(false);
				m_pAlignB_Bt->SetBitmap(wxBitmap(AlignB_xpm));
				m_pAlignB_Bt->SetValue(false);
				m_pAlignBR_Bt->SetBitmap(wxBitmap(AlignBR_xpm));
				m_pAlignBR_Bt->SetValue(false);
				break;
		case wxMAXY:
				m_pAlignTL_Bt->SetBitmap(wxBitmap(AlignTL_xpm));
				m_pAlignTL_Bt->SetValue(false);
				m_pAlignT_Bt->SetBitmap(wxBitmap(AlignTPressed_xpm));
				m_pAlignT_Bt->SetValue(true);
				m_pAlignTR_Bt->SetBitmap(wxBitmap(AlignTR_xpm));
				m_pAlignTR_Bt->SetValue(false);
				m_pAlignL_Bt->SetBitmap(wxBitmap(AlignL_xpm));
				m_pAlignL_Bt->SetValue(false);
				m_pAlignC_Bt->SetBitmap(wxBitmap(AlignC_xpm));
				m_pAlignC_Bt->SetValue(false);
				m_pAlignR_Bt->SetBitmap(wxBitmap(AlignR_xpm));
				m_pAlignR_Bt->SetValue(false);
				m_pAlignBL_Bt->SetBitmap(wxBitmap(AlignBL_xpm));
				m_pAlignBL_Bt->SetValue(false);
				m_pAlignB_Bt->SetBitmap(wxBitmap(AlignB_xpm));
				m_pAlignB_Bt->SetValue(false);
				m_pAlignBR_Bt->SetBitmap(wxBitmap(AlignBR_xpm));
				m_pAlignBR_Bt->SetValue(false);
				break;
		case wxMAXY | wxMAXX:
				m_pAlignTL_Bt->SetBitmap(wxBitmap(AlignTL_xpm));
				m_pAlignTL_Bt->SetValue(false);
				m_pAlignT_Bt->SetBitmap(wxBitmap(AlignT_xpm));
				m_pAlignT_Bt->SetValue(false);
				m_pAlignTR_Bt->SetBitmap(wxBitmap(AlignTRPressed_xpm));
				m_pAlignTR_Bt->SetValue(true);
				m_pAlignL_Bt->SetBitmap(wxBitmap(AlignL_xpm));
				m_pAlignL_Bt->SetValue(false);
				m_pAlignC_Bt->SetBitmap(wxBitmap(AlignC_xpm));
				m_pAlignC_Bt->SetValue(false);
				m_pAlignR_Bt->SetBitmap(wxBitmap(AlignR_xpm));
				m_pAlignR_Bt->SetValue(false);
				m_pAlignBL_Bt->SetBitmap(wxBitmap(AlignBL_xpm));
				m_pAlignBL_Bt->SetValue(false);
				m_pAlignB_Bt->SetBitmap(wxBitmap(AlignB_xpm));
				m_pAlignB_Bt->SetValue(false);
				m_pAlignBR_Bt->SetBitmap(wxBitmap(AlignBR_xpm));
				m_pAlignBR_Bt->SetValue(false);
				break;
		case wxMINX:
				m_pAlignTL_Bt->SetBitmap(wxBitmap(AlignTL_xpm));
				m_pAlignTL_Bt->SetValue(false);
				m_pAlignT_Bt->SetBitmap(wxBitmap(AlignT_xpm));
				m_pAlignT_Bt->SetValue(false);
				m_pAlignTR_Bt->SetBitmap(wxBitmap(AlignTR_xpm));
				m_pAlignTR_Bt->SetValue(false);
				m_pAlignL_Bt->SetBitmap(wxBitmap(AlignLPressed_xpm));
				m_pAlignL_Bt->SetValue(true);
				m_pAlignC_Bt->SetBitmap(wxBitmap(AlignC_xpm));
				m_pAlignC_Bt->SetValue(false);
				m_pAlignR_Bt->SetBitmap(wxBitmap(AlignR_xpm));
				m_pAlignR_Bt->SetValue(false);
				m_pAlignBL_Bt->SetBitmap(wxBitmap(AlignBL_xpm));
				m_pAlignBL_Bt->SetValue(false);
				m_pAlignB_Bt->SetBitmap(wxBitmap(AlignB_xpm));
				m_pAlignB_Bt->SetValue(false);
				m_pAlignBR_Bt->SetBitmap(wxBitmap(AlignBR_xpm));
				m_pAlignBR_Bt->SetValue(false);
				break;
		case wxMIDX | wxMIDY:
				m_pAlignTL_Bt->SetBitmap(wxBitmap(AlignTL_xpm));
				m_pAlignTL_Bt->SetValue(false);
				m_pAlignT_Bt->SetBitmap(wxBitmap(AlignT_xpm));
				m_pAlignT_Bt->SetValue(false);
				m_pAlignTR_Bt->SetBitmap(wxBitmap(AlignTR_xpm));
				m_pAlignTR_Bt->SetValue(false);
				m_pAlignL_Bt->SetBitmap(wxBitmap(AlignL_xpm));
				m_pAlignL_Bt->SetValue(false);
				m_pAlignC_Bt->SetBitmap(wxBitmap(AlignCPressed_xpm));
				m_pAlignC_Bt->SetValue(true);
				m_pAlignR_Bt->SetBitmap(wxBitmap(AlignR_xpm));
				m_pAlignR_Bt->SetValue(false);
				m_pAlignBL_Bt->SetBitmap(wxBitmap(AlignBL_xpm));
				m_pAlignBL_Bt->SetValue(false);
				m_pAlignB_Bt->SetBitmap(wxBitmap(AlignB_xpm));
				m_pAlignB_Bt->SetValue(false);
				m_pAlignBR_Bt->SetBitmap(wxBitmap(AlignBR_xpm));
				m_pAlignBR_Bt->SetValue(false);
				break;
		case wxMAXX:
				m_pAlignTL_Bt->SetBitmap(wxBitmap(AlignTL_xpm));
				m_pAlignTL_Bt->SetValue(false);
				m_pAlignT_Bt->SetBitmap(wxBitmap(AlignT_xpm));
				m_pAlignT_Bt->SetValue(false);
				m_pAlignTR_Bt->SetBitmap(wxBitmap(AlignTR_xpm));
				m_pAlignTR_Bt->SetValue(false);
				m_pAlignL_Bt->SetBitmap(wxBitmap(AlignL_xpm));
				m_pAlignL_Bt->SetValue(false);
				m_pAlignC_Bt->SetBitmap(wxBitmap(AlignC_xpm));
				m_pAlignC_Bt->SetValue(false);
				m_pAlignR_Bt->SetBitmap(wxBitmap(AlignRPressed_xpm));
				m_pAlignR_Bt->SetValue(true);
				m_pAlignBL_Bt->SetBitmap(wxBitmap(AlignBL_xpm));
				m_pAlignBL_Bt->SetValue(false);
				m_pAlignB_Bt->SetBitmap(wxBitmap(AlignB_xpm));
				m_pAlignB_Bt->SetValue(false);
				m_pAlignBR_Bt->SetBitmap(wxBitmap(AlignBR_xpm));
				m_pAlignBR_Bt->SetValue(false);
				break;
		case wxMINY | wxMINX:
				m_pAlignTL_Bt->SetBitmap(wxBitmap(AlignTL_xpm));
				m_pAlignTL_Bt->SetValue(false);
				m_pAlignT_Bt->SetBitmap(wxBitmap(AlignT_xpm));
				m_pAlignT_Bt->SetValue(false);
				m_pAlignTR_Bt->SetBitmap(wxBitmap(AlignTR_xpm));
				m_pAlignTR_Bt->SetValue(false);
				m_pAlignL_Bt->SetBitmap(wxBitmap(AlignL_xpm));
				m_pAlignL_Bt->SetValue(false);
				m_pAlignC_Bt->SetBitmap(wxBitmap(AlignC_xpm));
				m_pAlignC_Bt->SetValue(false);
				m_pAlignR_Bt->SetBitmap(wxBitmap(AlignR_xpm));
				m_pAlignR_Bt->SetValue(false);
				m_pAlignBL_Bt->SetBitmap(wxBitmap(AlignBLPressed_xpm));
				m_pAlignBL_Bt->SetValue(true);
				m_pAlignB_Bt->SetBitmap(wxBitmap(AlignB_xpm));
				m_pAlignB_Bt->SetValue(false);
				m_pAlignBR_Bt->SetBitmap(wxBitmap(AlignBR_xpm));
				m_pAlignBR_Bt->SetValue(false);
				break;
		case wxMINY:
				m_pAlignTL_Bt->SetBitmap(wxBitmap(AlignTL_xpm));
				m_pAlignTL_Bt->SetValue(false);
				m_pAlignT_Bt->SetBitmap(wxBitmap(AlignT_xpm));
				m_pAlignT_Bt->SetValue(false);
				m_pAlignTR_Bt->SetBitmap(wxBitmap(AlignTR_xpm));
				m_pAlignTR_Bt->SetValue(false);
				m_pAlignL_Bt->SetBitmap(wxBitmap(AlignL_xpm));
				m_pAlignL_Bt->SetValue(false);
				m_pAlignC_Bt->SetBitmap(wxBitmap(AlignC_xpm));
				m_pAlignC_Bt->SetValue(false);
				m_pAlignR_Bt->SetBitmap(wxBitmap(AlignR_xpm));
				m_pAlignR_Bt->SetValue(false);
				m_pAlignBL_Bt->SetBitmap(wxBitmap(AlignBL_xpm));
				m_pAlignBL_Bt->SetValue(false);
				m_pAlignB_Bt->SetBitmap(wxBitmap(AlignBPressed_xpm));
				m_pAlignB_Bt->SetValue(true);
				m_pAlignBR_Bt->SetBitmap(wxBitmap(AlignBR_xpm));
				m_pAlignBR_Bt->SetValue(false);
				break;
		case wxMINY | wxMAXX:
				m_pAlignTL_Bt->SetBitmap(wxBitmap(AlignTL_xpm));
				m_pAlignTL_Bt->SetValue(false);
				m_pAlignT_Bt->SetBitmap(wxBitmap(AlignT_xpm));
				m_pAlignT_Bt->SetValue(false);
				m_pAlignTR_Bt->SetBitmap(wxBitmap(AlignTR_xpm));
				m_pAlignTR_Bt->SetValue(false);
				m_pAlignL_Bt->SetBitmap(wxBitmap(AlignL_xpm));
				m_pAlignL_Bt->SetValue(false);
				m_pAlignC_Bt->SetBitmap(wxBitmap(AlignC_xpm));
				m_pAlignC_Bt->SetValue(false);
				m_pAlignR_Bt->SetBitmap(wxBitmap(AlignR_xpm));
				m_pAlignR_Bt->SetValue(false);
				m_pAlignBL_Bt->SetBitmap(wxBitmap(AlignBL_xpm));
				m_pAlignBL_Bt->SetValue(false);
				m_pAlignB_Bt->SetBitmap(wxBitmap(AlignB_xpm));
				m_pAlignB_Bt->SetValue(false);
				m_pAlignBR_Bt->SetBitmap(wxBitmap(AlignBRPressed_xpm));
				m_pAlignBR_Bt->SetValue(true);
				break;
		default:
				m_pAlignTL_Bt->SetBitmap(wxBitmap(AlignTL_xpm));
				m_pAlignTL_Bt->SetValue(false);
				m_pAlignT_Bt->SetBitmap(wxBitmap(AlignT_xpm));
				m_pAlignT_Bt->SetValue(false);
				m_pAlignTR_Bt->SetBitmap(wxBitmap(AlignTR_xpm));
				m_pAlignTR_Bt->SetValue(false);
				m_pAlignL_Bt->SetBitmap(wxBitmap(AlignL_xpm));
				m_pAlignL_Bt->SetValue(false);
				m_pAlignC_Bt->SetBitmap(wxBitmap(AlignC_xpm));
				m_pAlignC_Bt->SetValue(false);
				m_pAlignR_Bt->SetBitmap(wxBitmap(AlignR_xpm));
				m_pAlignR_Bt->SetValue(false);
				m_pAlignBL_Bt->SetBitmap(wxBitmap(AlignBL_xpm));
				m_pAlignBL_Bt->SetValue(false);
				m_pAlignB_Bt->SetBitmap(wxBitmap(AlignB_xpm));
				m_pAlignB_Bt->SetValue(false);
				m_pAlignBR_Bt->SetBitmap(wxBitmap(AlignBR_xpm));
				m_pAlignBR_Bt->SetValue(false);
				break;
	};
}


wxUint32 a2dSetTextChanges( a2dTextChanges& returned, a2dCanvasObjectList* objects, a2dCanvasObjectFlagsMask mask, a2dLayers* layersetup )
{
    wxUint32 count = 0;

    forEachIn( a2dCanvasObjectList, objects )
    {
        a2dCanvasObject* obj = *iter;
        a2dText* text = wxDynamicCast( obj, a2dText );

        if ( obj->GetFixedStyle() || obj->GetRelease() || !obj->CheckMask( mask ) || !text )
            continue;

        obj->SetBin2( true );

        a2dFont font = text->GetFont();

        if ( count == 0 )
            returned.Set( font, text->GetTextFlags(), text->GetAlignment(), text->GetWrongLoad() );
        else
            returned.Mix( font, text->GetTextFlags(), text->GetAlignment() );

        // if the font is not available, invalidate name and style
        // so that nothing will be selected in the dialog
        if ( text->GetWrongLoad() )
        {
            returned.SetNameValidity( false );
            returned.SetStyleValidity( false );
        }

        count++;
    }

    return count;
}
