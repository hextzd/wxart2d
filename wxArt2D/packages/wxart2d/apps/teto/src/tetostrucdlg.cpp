/////////////////////////////////////////////////////////////////////////////
// Name:        strucdlg.cpp
// Author:      Klaas Holwerda
// Created:     XX/XX/XX
// Copyright:   2000 (c) Klaas Holwerda
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "tetostrucdlg.h"
#include "wx/canvas/canmod.h"
#include "wx/editor/editmod.h"

#define STRUCT_HSIZE        220
#define STRUCT_VSIZE        400

const int    GROUP_BUTTON_OK     = wxID_HIGHEST + 5401 ;
const int    GROUP_BUTTON_CANCEL = wxID_HIGHEST + 5402 ;
const int    GROUP_BUTTON_SHOW   = wxID_HIGHEST + 5403 ;
const int    GROUP_UPDATE_DIALOG = wxID_HIGHEST + 5404 ;


BEGIN_EVENT_TABLE( TetoObjectsDialog, wxDialog )
    EVT_BUTTON    ( GROUP_BUTTON_OK,  TetoObjectsDialog::CmOk )
    EVT_BUTTON    ( GROUP_BUTTON_CANCEL ,  TetoObjectsDialog::CmCancel )
    EVT_CLOSE     ( TetoObjectsDialog::OnCloseWindow )
END_EVENT_TABLE()


TetoObjectsDialog::TetoObjectsDialog( wxFrame* parent, a2dCanvasDocument* root, a2dCanvasObjectList* total, long style ):
    wxDialog( parent, GROUP_UPDATE_DIALOG, wxT( "View Groups" ), wxPoint( 0, 0 ), wxSize( STRUCT_HSIZE, STRUCT_VSIZE ), style, _( "Group dialog" ) )
{
    m_parent = parent;

    m_panel     = new wxPanel( this, -1, wxDefaultPosition, wxDefaultSize, 0, wxT( "panel" ) );
    m_gb        = new wxStaticBox(  m_panel, -1, _( "Groups" ), wxDefaultPosition, wxDefaultSize, 0, wxT( "groupbox" ) );
    m_mess1     = new wxStaticText( m_panel, -1, wxT( "nop" ) );
    m_mess2     = new wxStaticText( m_panel, -1, _( "Select a Group in the list above" ) );
    m_listbox   = new wxListBox(    m_panel, -1, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_NEEDED_SB | wxLB_SINGLE );
    m_button1   = new wxButton( m_panel, GROUP_BUTTON_OK,        _( "Ok" ), wxDefaultPosition, wxSize( 55, 20 ) );
    m_button2   = new wxButton( m_panel, GROUP_BUTTON_CANCEL,    _( "Cancel" ), wxDefaultPosition, wxSize( 55, 20 ) );

    // Set constraints for panel subwindow
    wxLayoutConstraints* c1 = new wxLayoutConstraints;
    c1->left.SameAs       ( this, wxLeft, 1 );
    c1->top.SameAs        ( this, wxTop, 1 );
    c1->right.SameAs      ( this, wxRight, 1 );
    c1->bottom.SameAs     ( this, wxBottom, 1 );
    m_panel->SetConstraints( c1 );

    //staticbox
    wxLayoutConstraints* c3 = new wxLayoutConstraints;
    c3->left.SameAs       ( m_panel, wxLeft, 2 );
    c3->top.SameAs        ( m_panel, wxTop, 2 );
    c3->right.LeftOf      ( m_button2, 5 );
    c3->bottom.SameAs     ( m_mess2, wxTop, 2 );
    m_gb->SetConstraints( c3 );

    //mess2
    wxLayoutConstraints* c2 = new wxLayoutConstraints;
    c2->left.SameAs       ( m_gb, wxLeft, 10 );
    c2->top.SameAs        ( m_gb, wxTop, 15 );
    c2->height.AsIs();
    c2->width.AsIs();
    m_mess1->SetConstraints( c2 );

    //listbox
    wxLayoutConstraints* c9 = new wxLayoutConstraints;
    c9->left.SameAs       ( m_gb, wxLeft, 12 );
    c9->top.SameAs        ( m_mess1, wxBottom, 2 );
    c9->right.SameAs      ( m_gb, wxRight, 12 );
    c9->bottom.SameAs     ( m_gb, wxBottom, 12 );
    m_listbox->SetConstraints( c9 );

    //OKbutton
    wxLayoutConstraints* c4 = new wxLayoutConstraints;
    c4->left.RightOf      ( m_gb, 5 );
    c4->top.SameAs       ( m_gb, wxTop, 5 );
    c4->height.AsIs();
    c4->width.AsIs();
    m_button1->SetConstraints( c4 );



    //Cancel button
    wxLayoutConstraints* c5 = new wxLayoutConstraints;
    c5->right.SameAs      ( m_panel, wxRight, 5 );
    c5->top.SameAs       ( m_button1, wxBottom, 5 );
    c5->height.AsIs();
    c5->width.AsIs();
    m_button2->SetConstraints( c5 );


    //mess2
    wxLayoutConstraints* c8 = new wxLayoutConstraints;
    c8->left.SameAs       ( m_panel, wxLeft );
    c8->bottom.SameAs     ( m_panel, wxBottom, 5 );
    c8->height.AsIs();
    c8->width.AsIs();
    m_mess2->SetConstraints( c8 );

    wxString mess = wxT( "" );

    m_structurelist = total;
    m_listbox->Clear();

    for( a2dCanvasObjectList::iterator iter = m_structurelist->begin(); iter != m_structurelist->end(); ++iter )
    {
        a2dCanvasObject* obj = ( *iter );
        if ( obj == root->GetDrawing()->GetRootObject() )
        {
            m_listbox->Append( obj->GetName() + wxT( "=>top" ) );
        }
        else
            m_listbox->Append( obj->GetName() );
    }

    mess.Printf( _( "Number of structures: %i" ), m_structurelist->size() - 1 );
    m_mess1->SetLabel( mess );

    Centre( wxBOTH );

    // set the first structure as default and give it the focus
    if ( m_structurelist->size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;

    SetAutoLayout( true );
    Layout();
}

TetoObjectsDialog::~TetoObjectsDialog()
{
}

void TetoObjectsDialog::CmOk( wxCommandEvent& )
{
    wxString buf;
    buf = m_listbox->GetStringSelection();
    for( a2dCanvasObjectList::iterator iter = m_structurelist->begin(); iter != m_structurelist->end(); ++iter )
    {
        a2dCanvasObject* obj = ( *iter );
        wxString name = obj->GetName();
        if ( !name.Cmp( buf ) )
        {
            m_object = obj;
            break;
        }
        else if ( name + wxT( "=>top" ) == buf )
        {
            m_object = obj;
            break;
        }
    }
    EndModal( wxID_OK );
}

a2dCanvasObject* TetoObjectsDialog::GetObject()
{
    return m_object;
}

void TetoObjectsDialog::CmCancel( wxCommandEvent& )
{
    EndModal( wxID_CANCEL );
}

void TetoObjectsDialog::OnCloseWindow( wxCloseEvent& event )
{
    wxCommandEvent eventc;
    CmCancel( eventc );
}

