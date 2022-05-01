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

//-----------------------------------------------------------------------------
// a2dFontSearchPathDialog
//-----------------------------------------------------------------------------

enum
{
    wxID_PATHLIST = wxID_HIGHEST + 1,
    wxID_PATHEDIT,
    wxID_PATHBUTTON
};

IMPLEMENT_DYNAMIC_CLASS( a2dFontSearchPathDialog, wxDialog )

BEGIN_EVENT_TABLE( a2dFontSearchPathDialog, wxDialog )
    EVT_BUTTON( wxID_NEW, a2dFontSearchPathDialog::OnAdd )
    EVT_BUTTON( wxID_DELETE, a2dFontSearchPathDialog::OnDelete )
    EVT_BUTTON( wxID_OK, a2dFontSearchPathDialog::OnOK )
    EVT_BUTTON( wxID_PATHBUTTON, a2dFontSearchPathDialog::OnPathButton )
    EVT_LISTBOX( wxID_PATHLIST, a2dFontSearchPathDialog::OnListBox )
    EVT_TEXT( wxID_PATHEDIT, a2dFontSearchPathDialog::OnText )
END_EVENT_TABLE()

a2dFontSearchPathDialog::a2dFontSearchPathDialog( wxWindow* parent )
    : wxDialog( parent, -1, _( "Font Search Path" ), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
    wxBoxSizer* topsizer = new wxBoxSizer( wxVERTICAL );

    wxArrayString paths;
    wxPathList& pathlist = a2dGlobals->GetFontPathList();
#if wxCHECK_VERSION(2,7,0)
    for ( wxPathList::iterator node = pathlist.begin(); node != pathlist.end(); node++ )
#else
    for ( wxPathList::compatibility_iterator node = pathlist.GetFirst(); node; node = node->GetNext() )
#endif
        paths.Add( node->GetData() );
    m_listbox = new wxListBox( this, wxID_PATHLIST, wxDefaultPosition, wxDefaultSize, paths );
    topsizer->Add( m_listbox, 1, wxTOP | wxLEFT | wxRIGHT | wxEXPAND, 10 );

    wxBoxSizer* editsizer = new wxBoxSizer( wxHORIZONTAL );
    m_textctrl = new wxTextCtrl( this, wxID_PATHEDIT, wxT( "" ) );
    editsizer->Add( m_textctrl, 1, wxEXPAND, 0 );
    m_button = new wxButton( this, wxID_PATHBUTTON, wxT( "..." ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    editsizer->Add( m_button, 0, wxEXPAND, 0 );
    topsizer->Add( editsizer, 0, wxBOTTOM | wxLEFT | wxRIGHT | wxEXPAND, 10 );

    wxBoxSizer* buttonsizer = new wxBoxSizer( wxHORIZONTAL );

    wxBoxSizer* adddelsizer = new wxBoxSizer( wxHORIZONTAL );
    adddelsizer->Add( new wxButton( this, wxID_NEW, _( "Add" ) ), 0, wxRIGHT, 10 );
    adddelsizer->Add( new wxButton( this, wxID_DELETE, _( "Delete" ) ), 0, 0, 0 );
    buttonsizer->Add( adddelsizer, 1, wxRIGHT | wxEXPAND, 30 );

    buttonsizer->Add( new wxButton( this, wxID_OK, _( "OK" ) ), 0, wxRIGHT, 10 );
    buttonsizer->Add( new wxButton( this, wxID_CANCEL, _( "Cancel" ) ), 0, 0, 0 );
    topsizer->Add( buttonsizer, 0, wxLEFT | wxBOTTOM | wxRIGHT | wxEXPAND, 10 );

    SetSizer( topsizer );
    topsizer->SetSizeHints( this );
}

a2dFontSearchPathDialog::~a2dFontSearchPathDialog()
{
}

void a2dFontSearchPathDialog::OnAdd( wxCommandEvent& WXUNUSED( event ) )
{
    m_listbox->Append( m_textctrl->GetValue() );
}

void a2dFontSearchPathDialog::OnPathButton( wxCommandEvent& WXUNUSED( event ) )
{
    const wxString& dir = wxDirSelector( _( "Choose a folder" ) );
    if ( !dir.empty() )
        m_textctrl->SetValue( dir );
}

void a2dFontSearchPathDialog::OnDelete( wxCommandEvent& WXUNUSED( event ) )
{
    int idx = m_listbox->GetSelection();
    if ( idx != wxNOT_FOUND )
        m_listbox->Delete( idx );
    m_textctrl->SetValue( wxT( "" ) );
}

void a2dFontSearchPathDialog::OnListBox( wxCommandEvent& WXUNUSED( event ) )
{
    int idx = m_listbox->GetSelection();
    m_textctrl->SetValue( m_listbox->GetStringSelection() );
    m_listbox->SetSelection( idx );
}

void a2dFontSearchPathDialog::OnOK( wxCommandEvent& event )
{
    wxPathList& pathlist = a2dGlobals->GetFontPathList();
    pathlist.Clear();
    for ( unsigned int i = 0; i < m_listbox->GetCount(); i++ )
        pathlist.Add( m_listbox->GetString( i ) );
#if wxCHECK_VERSION(2,7,0)
    AcceptAndClose();
#else
    wxDialog::OnOK( event );
#endif
}

void a2dFontSearchPathDialog::OnText( wxCommandEvent& WXUNUSED( event ) )
{
    m_listbox->SetSelection( wxNOT_FOUND );
}

//-----------------------------------------------------------------------------
// a2dTextPropDlg
//-----------------------------------------------------------------------------

enum
{
    wxID_FONTTYPE_LIST = wxID_HIGHEST + 1,
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

IMPLEMENT_DYNAMIC_CLASS( a2dTextPropDlg, wxDialog )

BEGIN_EVENT_TABLE( a2dTextPropDlg, wxDialog )
    EVT_LISTBOX( wxID_FONTTYPE_LIST, a2dTextPropDlg::OnChangeFontType )
    EVT_LISTBOX( wxID_FONTNAME_LIST, a2dTextPropDlg::OnChangeFontName )
    EVT_LISTBOX( wxID_FONTSTYLE_LIST, a2dTextPropDlg::OnChangeFontStyle )
    EVT_LISTBOX( wxID_FONTSIZE_LIST, a2dTextPropDlg::OnChangeFontSizeList )
    EVT_TEXT( wxID_FONTSIZE_EDIT, a2dTextPropDlg::OnChangeFontSize )
    EVT_BUTTON( wxID_FONT_SEARCHPATH, a2dTextPropDlg::OnSearchPath )
    EVT_BUTTON( wxID_ALIGN_TL, a2dTextPropDlg::OnAlignment )
    EVT_BUTTON( wxID_ALIGN_T, a2dTextPropDlg::OnAlignment )
    EVT_BUTTON( wxID_ALIGN_TR, a2dTextPropDlg::OnAlignment )
    EVT_BUTTON( wxID_ALIGN_L, a2dTextPropDlg::OnAlignment )
    EVT_BUTTON( wxID_ALIGN_C, a2dTextPropDlg::OnAlignment )
    EVT_BUTTON( wxID_ALIGN_R, a2dTextPropDlg::OnAlignment )
    EVT_BUTTON( wxID_ALIGN_BL, a2dTextPropDlg::OnAlignment )
    EVT_BUTTON( wxID_ALIGN_B, a2dTextPropDlg::OnAlignment )
    EVT_BUTTON( wxID_ALIGN_BR, a2dTextPropDlg::OnAlignment )
    EVT_CHECKBOX( wxID_CHECK_ALIGN, a2dTextPropDlg::OnAlignment )
    EVT_CHECKBOX( wxID_CHECK_FRAME, a2dTextPropDlg::OnAlignment )
    EVT_CHECKBOX( wxID_CHECK_BACKGROUND, a2dTextPropDlg::OnAlignment )
    EVT_SIZE( a2dTextPropDlg::OnSize )
END_EVENT_TABLE()

a2dTextPropDlg::a2dTextPropDlg( wxWindow* parent, const a2dFont& currentfont, unsigned int textflags, int alignment )
    : wxDialog( parent, -1, _( "Choose Font" ), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
    m_currentinfo = currentfont.GetFontInfo();
    m_font = *a2dDEFAULT_CANVASFONT;
    m_textflags = textflags;
    m_alignment = alignment;
    m_viewscale = 1.0;
    m_fonts.DeleteContents( true );
    a2dFont::GetInfoList( m_fonts );

    wxBoxSizer* propsizer = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer* panelsizer = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer* sizesizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* typesizer = new wxBoxSizer( wxVERTICAL );
    typesizer->Add( new wxStaticText( this, -1, _( "Type" ) ), 0, 0, 0 );
    wxArrayString types;
    types.Add( _( "* (All types)" ) );
    for ( size_t i = 0; i < m_fonts.GetCount(); i++ )
    {
        wxString type = m_fonts.Item( i )->GetData()->GetType();
        size_t j;
        for ( j = 0; j < types.GetCount(); j++ )
        {
            if ( types.Item( j ) == type )
                break;
        }
        if ( j == types.GetCount() )
            types.Add( type );
    }
    m_typelist = new wxListBox( this, wxID_FONTTYPE_LIST, wxDefaultPosition, wxDefaultSize, types, wxLB_SORT );
    typesizer->Add( m_typelist, 1, wxEXPAND, 0 );
    propsizer->Add( typesizer, 1, wxALL | wxEXPAND, 10 );

    wxBoxSizer* fontsizer = new wxBoxSizer( wxVERTICAL );
    fontsizer->Add( new wxStaticText( this, -1, _( "Font" ) ), 0, 0, 0 );
    wxArrayString fontnames;
    m_fontlist = new wxListBox( this, wxID_FONTNAME_LIST, wxDefaultPosition, wxDefaultSize, fontnames, wxLB_SORT );
    fontsizer->Add( m_fontlist, 1, wxEXPAND, 0 );
    propsizer->Add( fontsizer, 2, wxALL | wxEXPAND, 10 );

    wxBoxSizer* stylesizer = new wxBoxSizer( wxVERTICAL );
    stylesizer->Add( new wxStaticText( this, -1, _( "Style" ) ), 0, wxLEFT | wxRIGHT | wxTOP, 5 );
    wxArrayString styles;
    m_stylelist = new wxListBox( this, wxID_FONTSTYLE_LIST, wxDefaultPosition, wxDefaultSize, styles, wxLB_SORT );
    stylesizer->Add( m_stylelist, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5 );
    propsizer->Add( stylesizer, 1, wxALL | wxEXPAND, 5 );

    sizesizer->Add( new wxStaticText( this, -1, _( "Size" ) ), 0, wxLEFT | wxRIGHT | wxTOP, 5 );
    wxArrayString sizes;
    sizes.Add( wxT( "1.0" ) );
    sizes.Add( wxT( "2.0" ) );
    sizes.Add( wxT( "5.0" ) );
    sizes.Add( wxT( "10.0" ) );
    sizes.Add( wxT( "20.0" ) );
    sizes.Add( wxT( "50.0" ) );
    sizes.Add( wxT( "100.0" ) );
    sizes.Add( wxT( "200.0" ) );
    sizes.Add( wxT( "500.0" ) );
    m_sizelist = new wxListBox( this, wxID_FONTSIZE_LIST, wxDefaultPosition, wxDefaultSize, sizes );
    m_sizeedit = new wxTextCtrl( this, wxID_FONTSIZE_EDIT, wxT( "50.0" ) );
    wxString sizetext;
    sizetext.Printf( wxT( "%f" ), m_currentinfo.GetSize() );
    m_sizeedit->SetValue( sizetext );

    sizesizer->Add( m_sizeedit, 0, wxLEFT | wxRIGHT | wxEXPAND, 5 );
    sizesizer->Add( m_sizelist, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5 );

    propsizer->Add( sizesizer, 1, wxALL | wxEXPAND, 5 );
    panelsizer->Add( propsizer, 2, wxALL | wxEXPAND, 5 );

    wxBoxSizer* bottomsizer = new wxBoxSizer( wxHORIZONTAL );

    wxBoxSizer* alignsizer = new wxBoxSizer( wxVERTICAL );
    alignsizer->Add( new wxStaticText( this, -1, _( "Alignment" ) ), 0, wxLEFT | wxRIGHT | wxTOP, 5 );
    wxGridSizer* alignsizer2 = new wxGridSizer( 3 );
#ifdef __WXMSW__
    // use wingdings font for graphical representation of arrows on windows
    wxButton* button;
    wxFont font( 12, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "WingDings" ) );
    button = new wxButton( this, wxID_ALIGN_TL, wxT( "\xeb" ), wxDefaultPosition, wxSize( 32, 32 ), wxBU_EXACTFIT );
    button->SetFont( font );
    alignsizer2->Add( button, 0, 0, 0 );
    button = new wxButton( this, wxID_ALIGN_T, wxT( "\xe9" ), wxDefaultPosition, wxSize( 32, 32 ), wxBU_EXACTFIT );
    button->SetFont( font );
    alignsizer2->Add( button, 0, 0, 0 );
    button = new wxButton( this, wxID_ALIGN_TR, wxT( "\xec" ), wxDefaultPosition, wxSize( 32, 32 ), wxBU_EXACTFIT );
    button->SetFont( font );
    alignsizer2->Add( button, 0, 0, 0 );
    button = new wxButton( this, wxID_ALIGN_L, wxT( "\xe7" ), wxDefaultPosition, wxSize( 32, 32 ), wxBU_EXACTFIT );
    button->SetFont( font );
    alignsizer2->Add( button, 0, 0, 0 );
    button = new wxButton( this, wxID_ALIGN_C, wxT( "\x6c" ), wxDefaultPosition, wxSize( 32, 32 ), wxBU_EXACTFIT );
    button->SetFont( font );
    alignsizer2->Add( button, 0, 0, 0 );
    button = new wxButton( this, wxID_ALIGN_R, wxT( "\xe8" ), wxDefaultPosition, wxSize( 32, 32 ), wxBU_EXACTFIT );
    button->SetFont( font );
    alignsizer2->Add( button, 0, 0, 0 );
    button = new wxButton( this, wxID_ALIGN_BL, wxT( "\xed" ), wxDefaultPosition, wxSize( 32, 32 ), wxBU_EXACTFIT );
    button->SetFont( font );
    alignsizer2->Add( button, 0, 0, 0 );
    button = new wxButton( this, wxID_ALIGN_B, wxT( "\xea" ), wxDefaultPosition, wxSize( 32, 32 ), wxBU_EXACTFIT );
    button->SetFont( font );
    alignsizer2->Add( button, 0, 0, 0 );
    button = new wxButton( this, wxID_ALIGN_BR, wxT( "\xee" ), wxDefaultPosition, wxSize( 32, 32 ), wxBU_EXACTFIT );
    button->SetFont( font );
    alignsizer2->Add( button, 0, 0, 0 );
#else // __WXMSW__
    alignsizer2->Add( new wxButton( this, wxID_ALIGN_TL, wxT( "" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ), 1, wxEXPAND, 0 );
    alignsizer2->Add( new wxButton( this, wxID_ALIGN_T, wxT( "^" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ), 1, wxEXPAND, 0 );
    alignsizer2->Add( new wxButton( this, wxID_ALIGN_TR, wxT( "" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ), 1, wxEXPAND, 0 );
    alignsizer2->Add( new wxButton( this, wxID_ALIGN_L, wxT( "<" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ), 1, wxEXPAND, 0 );
    alignsizer2->Add( new wxButton( this, wxID_ALIGN_C, wxT( "o" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ), 1, wxEXPAND, 0 );
    alignsizer2->Add( new wxButton( this, wxID_ALIGN_R, wxT( ">" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ), 1, wxEXPAND, 0 );
    alignsizer2->Add( new wxButton( this, wxID_ALIGN_BL, wxT( "" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ), 1, wxEXPAND, 0 );
    alignsizer2->Add( new wxButton( this, wxID_ALIGN_B, wxT( "v" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ), 1, wxEXPAND, 0 );
    alignsizer2->Add( new wxButton( this, wxID_ALIGN_BR, wxT( "" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ), 1, wxEXPAND, 0 );
#endif // __WXMSW__
    alignsizer->Add( alignsizer2, 0, wxALL | wxEXPAND, 5 );
    m_aligncheck = new wxCheckBox( this, wxID_CHECK_ALIGN, _( "Align on bbox" ) );
    m_aligncheck->SetValue( (m_alignment & wxBASELINE) > 0  );
    alignsizer->Add( m_aligncheck, 0, 0 );

    m_textframe = new wxCheckBox( this, wxID_CHECK_FRAME, _( "Frame around text" ) );
    m_textframe->SetValue( (m_textflags & a2dText::a2dCANOBJTEXT_FRAME) > 0 );
    alignsizer->Add( m_textframe, 0, 0 );

    m_textbackground = new wxCheckBox( this, wxID_CHECK_BACKGROUND, _( "Fill text background" ) );
    m_textbackground->SetValue( (m_textflags & a2dText::a2dCANOBJTEXT_BACKGROUND) > 0 );
    alignsizer->Add( m_textbackground, 0, 0 );

    bottomsizer->Add( alignsizer, 0, wxALL | wxEXPAND, 5 );

    m_canvas = new a2dCanvas( this, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER );
    m_canvas->SetYaxis( true );
    m_canvas->SetScaleOnResize( false );
    a2dCanvasObject* root = m_canvas->GetDrawing()->GetRootObject();
    m_canvastext = new a2dText( _T( "ABCDEFGabcdefg12345" ) );
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
    bottomsizer->Add( m_canvas, 1, wxALL | wxEXPAND, 5 );

    panelsizer->Add( bottomsizer, 1, wxALL | wxEXPAND, 5 );

    wxBoxSizer* buttonsizer = new wxBoxSizer( wxHORIZONTAL );
    buttonsizer->Add( new wxButton( this, wxID_FONT_SEARCHPATH, _( "Search Path" ) ), 0, wxALL, 5 );
    buttonsizer->Add( new wxButton( this, wxID_OK, _( "OK" ) ), 0, wxALL, 5 );
    buttonsizer->Add( new wxButton( this, wxID_CANCEL, _( "Cancel" ) ), 0, wxALL, 5 );
    panelsizer->Add( buttonsizer, 0, wxALL | wxEXPAND, 5 );

    if ( m_typelist->GetCount() )
    {
        int idx = 0;
        idx = m_typelist->FindString( m_currentinfo.GetType() );
        if ( idx == wxNOT_FOUND )
            idx = 0;
        m_typelist->SetSelection( idx );
    }

    wxCommandEvent dummy;
    OnChangeFontType( dummy );

    SetSizer( panelsizer );
    panelsizer->SetSizeHints( this );
}

const a2dFont& a2dTextPropDlg::GetFontData() const
{
    return m_font;
}

a2dTextPropDlg::~a2dTextPropDlg()
{
}

void a2dTextPropDlg::OnChangeFontType( wxCommandEvent& event )
{
    wxString type = m_typelist->GetStringSelection();
    wxString oldname = m_fontlist->GetStringSelection();

    while ( m_fontlist->GetCount() )
        m_fontlist->Delete( 0 );

    wxArrayString fonts;
    for ( size_t i = 0; i < m_fonts.GetCount(); i++ )
    {
        wxString fonttype = m_fonts.Item( i )->GetData()->GetType();
        if ( fonttype == type || type == _( "* (All types)" ) )
        {
            wxString fontname = m_fonts.Item( i )->GetData()->GetName();
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
    m_fontlist->Append( fonts );

    if ( m_fontlist->GetCount() )
    {
        int idx;
        idx = m_fontlist->FindString( m_currentinfo.GetName() );
        if( idx == -1 )
            idx = m_fontlist->FindString( oldname );
        if ( idx == wxNOT_FOUND )
            idx = 0;
        m_fontlist->SetSelection( idx );
    }
    OnChangeFontName( event );
}

void a2dTextPropDlg::OnChangeFontName( wxCommandEvent& event )
{
    wxString type = m_typelist->GetStringSelection();
    wxString name = m_fontlist->GetStringSelection();
    wxString oldstyle = m_stylelist->GetStringSelection();

    while ( m_stylelist->GetCount() )
        m_stylelist->Delete( 0 );

    wxArrayString fonts;
    for ( size_t i = 0; i < m_fonts.GetCount(); i++ )
    {
        wxString fontname = m_fonts.Item( i )->GetData()->GetName();
        wxString fonttype = m_fonts.Item( i )->GetData()->GetType();
        if ( ( fonttype == type || type == _( "* (All types)" ) )
                && fontname == name )
        {
            wxString fontstyle = m_fonts.Item( i )->GetData()->GetStyle();
            size_t j;
            for ( j = 0; j < fonts.GetCount(); j++ )
            {
                if ( fonts.Item( j ) == fontstyle )
                    break;
            }
            if ( j == fonts.GetCount() )
                fonts.Add( fontstyle );
        }
    }
    m_stylelist->Append( fonts );

    if ( m_stylelist->GetCount() )
    {
        int idx;
        idx = m_fontlist->FindString( m_currentinfo.GetStyle() );
        if( idx == -1 )
            idx = m_stylelist->FindString( oldstyle );
        if ( idx == wxNOT_FOUND )
            idx = m_stylelist->FindString( _( "Normal" ) );
        if ( idx == wxNOT_FOUND )
            idx = m_stylelist->FindString( _( "Regular" ) );
        if ( idx == wxNOT_FOUND )
            idx = 0;
        m_stylelist->SetSelection( idx );
    }
    OnChangeFontStyle( event );
}

void a2dTextPropDlg::OnChangeFontStyle( wxCommandEvent& event )
{
    OnChangeFont();
}

void a2dTextPropDlg::OnChangeFontSize( wxCommandEvent& event )
{
    m_sizelist->SetSelection( wxNOT_FOUND );
    OnChangeFont();
}

void a2dTextPropDlg::OnChangeFontSizeList( wxCommandEvent& event )
{
    int idx = m_sizelist->GetSelection();
    m_sizeedit->SetValue( m_sizelist->GetStringSelection() );
    m_sizelist->SetSelection( idx );
    OnChangeFont();
}

void a2dTextPropDlg::OnChangeFont()
{
    wxString type = m_typelist->GetStringSelection();
    wxString name = m_fontlist->GetStringSelection();
    wxString style = m_stylelist->GetStringSelection();

    wxArrayString fonts;
    for ( size_t i = 0; i < m_fonts.GetCount(); i++ )
    {
        wxString fonttype = m_fonts.Item( i )->GetData()->GetType();
        wxString fontname = m_fonts.Item( i )->GetData()->GetName();
        wxString fontstyle = m_fonts.Item( i )->GetData()->GetStyle();
        if ( ( fonttype == type || type == _( "* (All types)" ) )
                && fontname == name && fontstyle == style )
        {
            double size;
            m_sizeedit->GetValue().ToDouble( &size );
            a2dFontInfo fontinfo = *m_fonts.Item( i )->GetData();

            //! \todo alignment
            // fontinfo.SetAlignment( m_alignment );
            fontinfo.SetSize( size );
            m_font = a2dFont::CreateFont( fontinfo );
            m_canvastext->SetFont( m_font );
            m_canvastext->SetPending( true );
            m_canvastext->SetAlignment( m_alignment );
            m_canvastext->SetTextFlags( m_textflags );
            a2dBoundingBox bbox = m_canvastext->GetCalculatedBoundingBox( 1 );

            m_canvasline1->SetPosXY12( -2.0 * bbox.GetWidth(), 0.0, 2.0 * bbox.GetWidth(), 0.0 );
            m_canvasline2->SetPosXY12( 0.0, -2.0 * bbox.GetWidth(), 0.0, 2.0 * bbox.GetWidth() );

            // To have some better visual feedback of alignment, move the text around a little.
            a2dBoundingBox bboxnew = bbox;
            bboxnew.Enlarge( bbox.GetHeight() / 6.0 );
            if ( m_alignment & wxMINX )
                bboxnew.Translate( -bbox.GetHeight() / 8.0, 0.0 );
            if ( m_alignment & wxMAXX )
                bboxnew.Translate( bbox.GetHeight() / 8.0, 0.0 );

            if ( m_alignment & wxMINY )
                bboxnew.Translate( 0.0, -bbox.GetHeight() / 8.0 );
            if ( m_alignment & wxMAXY )
                bboxnew.Translate( 0.0, bbox.GetHeight() / 8.0 );

            if ( m_alignment & wxBASELINE )
                m_canvastext->SetFill( wxColour( 0xf0, 0xf0, 0xff ) );
            if ( m_alignment & wxBASELINE_CONTRA )
                m_canvastext->SetFill( wxColour( 0xf0, 0xff, 0x00 ) );
            else
                m_canvastext->SetFill( *a2dTRANSPARENT_FILL );

            m_canvas->SetMappingWidthHeight ( bboxnew.GetMinX(), bboxnew.GetMinY(),
                                              bboxnew.GetWidth(), bboxnew.GetHeight(), false );
            m_canvas->Refresh();
            break;
        }
    }
}

void a2dTextPropDlg::OnSearchPath( wxCommandEvent& event )
{
    a2dFontSearchPathDialog dlg;
    if ( dlg.ShowModal() == wxID_OK )
    {
        m_fonts.Clear();
        a2dFont::GetInfoList( m_fonts );

        wxString oldtype = m_typelist->GetStringSelection();

        while ( m_typelist->GetCount() )
            m_typelist->Delete( 0 );
        wxArrayString types;
        types.Add( _( "* (All types)" ) );
        for ( size_t i = 0; i < m_fonts.GetCount(); i++ )
        {
            wxString type = m_fonts.Item( i )->GetData()->GetType();
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

void a2dTextPropDlg::OnAlignment( wxCommandEvent& event )
{
    switch( event.GetId() )
    {
        case wxID_ALIGN_TL: m_alignment = wxMAXY | wxMINX; break;
        case wxID_ALIGN_T: m_alignment = wxMAXY; break;
        case wxID_ALIGN_TR: m_alignment = wxMAXY | wxMAXX; break;
        case wxID_ALIGN_L: m_alignment = wxMAXX; break;
        case wxID_ALIGN_C: m_alignment = wxMIDX | wxMIDY; break;
        case wxID_ALIGN_R: m_alignment = wxMAXX; break;
        case wxID_ALIGN_BL: m_alignment = wxMINY | wxMINX; break;
        case wxID_ALIGN_B: m_alignment = wxMINY; break;
        case wxID_ALIGN_BR: m_alignment = wxMINY | wxMAXX; break;
        default: ;
    }
    if ( m_aligncheck->IsChecked() )
        m_alignment |= wxBASELINE;
    else
        m_alignment |= wxBASELINE_CONTRA;

    if ( m_textframe->IsChecked() )
        m_textflags |= a2dText::a2dCANOBJTEXT_FRAME;
    else
        m_textflags &= ~a2dText::a2dCANOBJTEXT_FRAME;

    if ( m_textbackground->IsChecked() )
        m_textflags |= a2dText::a2dCANOBJTEXT_BACKGROUND;
    else
        m_textflags &= ~a2dText::a2dCANOBJTEXT_BACKGROUND;

    OnChangeFont();
}

void a2dTextPropDlg::OnSize( wxSizeEvent& event )
{
    event.Skip();

    a2dBoundingBox bbox = m_canvastext->GetCalculatedBoundingBox( 1 );
    // To have some better visual feedback of alignment, move the text around a little.
    a2dBoundingBox bboxnew = bbox;
    bboxnew.Enlarge( bbox.GetHeight() / 6.0 );
    if ( m_alignment & wxMINX )
        bboxnew.Translate( -bbox.GetHeight() / 8.0, 0.0 );
    if ( m_alignment & wxMAXX )
        bboxnew.Translate( bbox.GetHeight() / 8.0, 0.0 );

    if ( m_alignment & wxMINY )
        bboxnew.Translate( 0.0, -bbox.GetHeight() / 8.0 );
    if ( m_alignment & wxMAXY )
        bboxnew.Translate( 0.0, bbox.GetHeight() / 8.0 );

    if ( m_alignment & wxBASELINE )
        m_canvastext->SetFill( wxColour( 0xf0, 0xf0, 0xff ) );
    if ( m_alignment & wxBASELINE_CONTRA )
        m_canvastext->SetFill( wxColour( 0xf0, 0xff, 0x00 ) );
    else
        m_canvastext->SetFill( *a2dTRANSPARENT_FILL );

    m_canvas->SetMappingWidthHeight ( bboxnew.GetMinX(), bboxnew.GetMinY(),
                                      bboxnew.GetWidth(), bboxnew.GetHeight(), false );
    m_canvas->Refresh();
}
