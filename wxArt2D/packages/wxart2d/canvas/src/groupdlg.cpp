/*! \file editor/src/groupdlg.cpp
    \brief Document/view classes
    \author Klaas Holwerda
    \date Created 05/07/2003

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: groupdlg.cpp,v 1.7 2008/10/21 21:48:30 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/platform.h>
#include <wx/clipbrd.h>
#include <algorithm>

#include "wx/canvas/groupdlg.h"
#include "wx/canvas/canglob.h"

#define GROUPDLG_HSIZE      400
#define GROUPDLG_VSIZE      400

const int    GROUP_BUTTON_OK       =  wxNewId();
const int    GROUP_BUTTON_CANCEL   =  wxNewId();
const int    GROUP_BUTTON_ADD_A    =  wxNewId();
const int    GROUP_BUTTON_REMOVE_A =  wxNewId();
const int    GROUP_BUTTON_CLR_A    =  wxNewId();
const int    GROUP_BUTTON_ADD_B    =  wxNewId();
const int    GROUP_BUTTON_REMOVE_B =  wxNewId();
const int    GROUP_BUTTON_CLR_B    =  wxNewId();
const int    GROUP_BUTTON_ADD_D    =  wxNewId();
const int    GROUP_BUTTON_REMOVE_D    =  wxNewId();

const int    ID_LISTBOX =  wxNewId();
const int    ID_CHECKBOX1 =  wxNewId();
const int    ID_CHECKBOX2 =  wxNewId();
const int    ID_CHECKBOX3 =  wxNewId();
const int    ID_TEXTCTRL =  wxNewId();

BEGIN_EVENT_TABLE( a2dGroupDlg, wxDialog )
    EVT_BUTTON    ( GROUP_BUTTON_OK, a2dGroupDlg::CmOk )
    EVT_BUTTON    ( GROUP_BUTTON_CANCEL, a2dGroupDlg::CmCancel )
    EVT_BUTTON    ( GROUP_BUTTON_ADD_A, a2dGroupDlg::CmAdd_A )
    EVT_BUTTON    ( GROUP_BUTTON_REMOVE_A, a2dGroupDlg::CmRemove_A )
    EVT_BUTTON    ( GROUP_BUTTON_CLR_A, a2dGroupDlg::CmClear_A )
    EVT_BUTTON    ( GROUP_BUTTON_ADD_B, a2dGroupDlg::CmAdd_B )
    EVT_BUTTON    ( GROUP_BUTTON_REMOVE_B, a2dGroupDlg::CmRemove_B )
    EVT_BUTTON    ( GROUP_BUTTON_CLR_B, a2dGroupDlg::CmClear_B )
    EVT_BUTTON    ( GROUP_BUTTON_ADD_D, a2dGroupDlg::CmAdd_D )
    EVT_BUTTON    ( GROUP_BUTTON_REMOVE_D, a2dGroupDlg::CmRemove_D )
    EVT_CLOSE( a2dGroupDlg::OnCloseWindow )
END_EVENT_TABLE()

a2dGroupDlg::a2dGroupDlg( a2dHabitat* habitat, wxFrame* parent, const wxString& title, long style, const wxString& name ):
    wxDialog( parent, -1, title, wxDefaultPosition, wxSize( GROUPDLG_HSIZE, GROUPDLG_VSIZE ), style, name )
{
    m_habitat = habitat;
/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_REDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );
*/
    wxSizer* item0 = new wxBoxSizer( wxVERTICAL );

    wxSizer* item1 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticBox* item3 = new wxStaticBox( this, -1, _( "layers" ) );
    wxSizer* item2 = new wxStaticBoxSizer( item3, wxVERTICAL );

    m_listbox_main = new wxListCtrl( this, ID_LISTBOX, wxDefaultPosition, wxSize( 40, -1 ), wxLC_REPORT | wxLC_NO_HEADER );
    item2->Add( m_listbox_main, 1, wxEXPAND | wxALL );
    m_listbox_main->InsertColumn( 0, _T( "Layers" ) );

    item1->Add( item2, 1, wxGROW | wxALL, 0 );

    wxSizer* item5 = new wxBoxSizer( wxVERTICAL );

    wxStaticBox* item7 = new wxStaticBox( this, -1, _( "group A" ) );
    wxSizer* item6 = new wxStaticBoxSizer( item7, wxHORIZONTAL );

    wxSizer* item8 = new wxBoxSizer( wxVERTICAL );

    wxButton* item9 = new wxButton( this, GROUP_BUTTON_ADD_A, _( "add >>" ), wxDefaultPosition, wxDefaultSize, 0 );
    item8->Add( item9, 0, wxALIGN_CENTRE | wxALL, 2 );

    wxButton* item10 = new wxButton( this, GROUP_BUTTON_REMOVE_A, _( "<< remove" ), wxDefaultPosition, wxDefaultSize, 0 );
    item8->Add( item10, 0, wxALIGN_CENTRE | wxALL, 2 );

    wxButton* item11 = new wxButton( this, GROUP_BUTTON_CLR_A, _( "clear" ), wxDefaultPosition, wxDefaultSize, 0 );
    item8->Add( item11, 0, wxALIGN_CENTRE | wxALL, 2 );

    m_onlyselect_A = new wxCheckBox( this, ID_CHECKBOX1, _( "selected" ), wxDefaultPosition, wxDefaultSize, 0 );
    item8->Add( m_onlyselect_A, 0, wxALIGN_CENTRE | wxALL, 2 );

    item6->Add( item8, 0, wxALIGN_CENTRE | wxALL, 0 );

    m_listbox_a = new wxListCtrl( this, ID_LISTBOX, wxDefaultPosition, wxSize( 40, 100 ), wxLC_REPORT | wxLC_NO_HEADER );
    item6->Add( m_listbox_a, 1, wxEXPAND | wxALL );
    m_listbox_a->InsertColumn( 0, _T( "Group A" ) );

    item5->Add( item6, 1, wxGROW | wxALL, 0 );

    wxStaticBox* item15 = new wxStaticBox( this, -1, _( "group B" ) );
    wxSizer* item14 = new wxStaticBoxSizer( item15, wxHORIZONTAL );

    wxSizer* item16 = new wxBoxSizer( wxVERTICAL );

    wxButton* item17 = new wxButton( this, GROUP_BUTTON_ADD_B, _( "add >>" ), wxDefaultPosition, wxDefaultSize, 0 );
    item16->Add( item17, 0, wxALIGN_CENTRE | wxALL, 2 );

    wxButton* item18 = new wxButton( this, GROUP_BUTTON_REMOVE_B, _( "<< remove" ), wxDefaultPosition, wxDefaultSize, 0 );
    item16->Add( item18, 0, wxALIGN_CENTRE | wxALL, 2 );

    wxButton* item19 = new wxButton( this, GROUP_BUTTON_CLR_B, _( "clear" ), wxDefaultPosition, wxDefaultSize, 0 );
    item16->Add( item19, 0, wxALIGN_CENTRE | wxALL, 2 );

    m_onlyselect_B = new wxCheckBox( this, ID_CHECKBOX1, _( "selected" ), wxDefaultPosition, wxDefaultSize, 0 );
    item16->Add( m_onlyselect_B, 0, wxALIGN_CENTRE | wxALL, 2 );

    item14->Add( item16, 0, wxALIGN_CENTRE | wxALL, 0 );

    m_listbox_b = new wxListCtrl( this, ID_LISTBOX, wxDefaultPosition, wxSize( 40, 100 ), wxLC_REPORT | wxLC_NO_HEADER );
    item14->Add( m_listbox_b, 1, wxGROW | wxALL );
    m_listbox_b->InsertColumn( 0, _T( "Group B" ) );

    item5->Add( item14, 1, wxGROW | wxALL, 0 );

    wxStaticBox* item23 = new wxStaticBox( this, -1, _( "Destination" ) );
    wxSizer* item22 = new wxStaticBoxSizer( item23, wxHORIZONTAL );

    wxSizer* item24 = new wxBoxSizer( wxVERTICAL );

    wxButton* item25 = new wxButton( this, GROUP_BUTTON_ADD_D, _( "Set Target >>" ), wxDefaultPosition, wxDefaultSize, 0 );
    item24->Add( item25, 0, wxALIGN_CENTRE | wxALL, 2 );

    m_clearTarget = new wxCheckBox( this, ID_CHECKBOX3, _( "Clear" ), wxDefaultPosition, wxDefaultSize, 0 );
    item24->Add( m_clearTarget, 0, wxALIGN_CENTRE | wxALL, 2 );

    item22->Add( item24, 0, wxALIGN_CENTRE | wxALL, 0 );

    m_destination = new wxTextCtrl( this, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize( 40, -1 ), 0 );
    item22->Add( m_destination, 1, wxALIGN_CENTRE | wxALL, 0 );

    item5->Add( item22, 0, wxGROW | wxALL, 0 );

    item1->Add( item5, 1, wxGROW | wxALL, 0 );

    item0->Add( item1, 1, wxGROW, 0 );

    wxSizer* item28 = new wxBoxSizer( wxHORIZONTAL );

    wxButton* item29 = new wxButton( this, GROUP_BUTTON_OK, _( "OK" ), wxDefaultPosition, wxDefaultSize, 0 );
    item28->Add( item29, 0, wxALIGN_CENTRE, 0 );

    wxButton* item30 = new wxButton( this, GROUP_BUTTON_CANCEL, _( "Cancel" ), wxDefaultPosition, wxDefaultSize, 0 );
    item28->Add( item30, 0, wxALIGN_CENTRE, 0 );

    item0->Add( item28, 0, wxALIGN_CENTRE, 0 );

    Init();

    m_listbox_main->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    m_listbox_a->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    m_listbox_b->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    m_listbox_main->SetColumnWidth( 0, 200 );
    m_listbox_a->SetColumnWidth( 0, 200 );
    m_listbox_b->SetColumnWidth( 0, 200 );

    SetAutoLayout( true );
    SetSizer( item0 );
    item0->SetSizeHints( this );
    item0->Fit( this );
}

a2dGroupDlg::~a2dGroupDlg()
{
/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_REDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( a2dEVT_COM_EVENT, this );
*/
}

void a2dGroupDlg::Init()
{
    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawingPart )
        return;

    m_layersetup = drawingPart->GetDrawing()->GetLayerSetup();

    a2dLayerGroup& groupa = m_habitat->GetGroupA();
    a2dLayerGroup& groupb = m_habitat->GetGroupB();
    int target = m_habitat->GetTarget();

    // fill the listboxes
    wxString layerLine;
    wxString available;


    for ( a2dLayerMapNr::reverse_iterator it= m_layersetup->GetLayerIndex().rbegin(); it != m_layersetup->GetLayerIndex().rend(); ++it)
    {
        a2dLayerInfo* lobj = it->second;
        wxUint16 j = it->first;

        layerLine.Printf( _T( "%04d     %s" ), lobj->GetLayer(), lobj->GetName().c_str() );

        wxListItem itemCol;
        itemCol.SetText( layerLine );
        itemCol.SetImage( -1 );
        itemCol.SetData( lobj ); //store layerinfo with item
        itemCol.SetId( 0 );
        if ( lobj->GetAvailable() )
            itemCol.SetTextColour( *wxRED );

        m_listbox_main->InsertItem( itemCol );

        // are there selected?
        a2dLayerGroup::iterator iter = wxSTD find( groupa.begin(), groupa.end(), lobj->GetLayer() );
        if ( iter != groupa.end() )
            m_listbox_a->InsertItem( itemCol );
        iter = wxSTD find( groupb.begin(), groupb.end(), lobj->GetLayer() );
        if ( iter != groupb.end() )
            m_listbox_b->InsertItem( itemCol );

        if ( target == lobj->GetLayer() )
        {
            m_destination->SetValue( layerLine );
            m_target = lobj->GetLayer();
        }
    }
    m_onlyselect_A->SetValue( m_habitat->GetSelectedOnlyA() );
    m_onlyselect_B->SetValue( m_habitat->GetSelectedOnlyB() );
    m_clearTarget->SetValue( m_habitat->GetClearTarget() );
}

void a2dGroupDlg::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    EndModal( 0 );
}


void a2dGroupDlg::CmAdd_A( wxCommandEvent& )
{
    wxString buf;
    int     selcount;

    selcount = m_listbox_main->GetSelectedItemCount();

    if ( selcount == 0 ) return;

    long item = -1;
    for ( ;; )
    {
        item = m_listbox_main->GetNextItem( item,
                                            wxLIST_NEXT_ALL,
                                            wxLIST_STATE_SELECTED );
        if ( item == -1 )
            break;

        buf = m_listbox_main->GetItemText( item );
        a2dLayerInfo* layerinfo = (a2dLayerInfo*) m_listbox_main->GetItemData( item );
        if ( m_listbox_a->FindItem( -1, buf ) == -1 )
        {
            wxListItem itemCol;
            itemCol.SetText( buf );
            itemCol.SetImage( -1 );
            itemCol.SetData( layerinfo ); //store layerinfo
            itemCol.SetId( 0 );
            m_listbox_a->InsertItem( itemCol );
        }
    }
    m_listbox_a->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    m_listbox_a->SetColumnWidth( 0, 200 );
}

// the user presses add to b
void a2dGroupDlg::CmAdd_B( wxCommandEvent& )
{
    wxString buf;
    int     selcount;

    selcount = m_listbox_main->GetSelectedItemCount();

    if ( selcount == 0 ) return;

    long item = -1;
    for ( ;; )
    {
        item = m_listbox_main->GetNextItem( item,
                                            wxLIST_NEXT_ALL,
                                            wxLIST_STATE_SELECTED );
        if ( item == -1 )
            break;

        buf = m_listbox_main->GetItemText( item );
        a2dLayerInfo* layerinfo = (a2dLayerInfo*) m_listbox_main->GetItemData( item );
        if ( m_listbox_b->FindItem( -1, buf ) == -1 )
        {
            wxListItem itemCol;
            itemCol.SetText( buf );
            itemCol.SetImage( -1 );
            itemCol.SetData( layerinfo ); //store layerinfo
            itemCol.SetId( 0 );
            m_listbox_b->InsertItem( itemCol );
        }
    }
    m_listbox_b->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    m_listbox_b->SetColumnWidth( 0, 200 );
}

// the user selected add to destination
void a2dGroupDlg::CmAdd_D( wxCommandEvent& )
{
    wxString buf;

    long item = m_listbox_main->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if ( item == -1 )
        return;

    buf = m_listbox_main->GetItemText( item );

    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawingPart )
        return;

    m_layersetup = drawingPart->GetDrawing()->GetLayerSetup();

    m_destination->SetValue( buf );

    a2dLayerInfo* layerinfo = (a2dLayerInfo*) m_listbox_main->GetItemData( item );

    m_target = layerinfo->GetLayer();
    m_listbox_main->SetItemState( item, 0, wxLIST_STATE_SELECTED );
}

void a2dGroupDlg::CmRemove_A( wxCommandEvent& )
{
    long item = -1;
    for ( ;; )
    {
        item = -1;
        item = m_listbox_a->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
        if ( item == -1 )
            break;

        m_listbox_a->DeleteItem( item );
    }
}

void a2dGroupDlg::CmRemove_B( wxCommandEvent& )
{
    long item = -1;
    for ( ;; )
    {
        item = -1;
        item = m_listbox_b->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
        if ( item == -1 )
            break;

        m_listbox_b->DeleteItem( item );
    }
}

void a2dGroupDlg::CmRemove_D( wxCommandEvent& )
{
    m_destination->SetValue( wxEmptyString );
}

void a2dGroupDlg::CmClear_A( wxCommandEvent& )
{
    m_listbox_a->DeleteAllItems();
}

void a2dGroupDlg::CmClear_B( wxCommandEvent& )
{
    m_listbox_b->DeleteAllItems();
}

// the user pressed OK. This function is called then
void a2dGroupDlg::CmOk( wxCommandEvent& )
{
    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawingPart )
        return;

    m_layersetup = drawingPart->GetDrawing()->GetLayerSetup();

    a2dLayerGroup groupa;
    long item = -1;
    for ( ;; )
    {
        item = m_listbox_a->GetNextItem( item );
        if ( item == -1 )
            break;
        groupa.push_back( ((a2dLayerInfo*) m_listbox_a->GetItemData( item ))->GetLayer() );
    }

    a2dLayerGroup groupb;
    item = -1;
    for ( ;; )
    {
        item = m_listbox_b->GetNextItem( item );
        if ( item == -1 )
            break;
        groupb.push_back( ((a2dLayerInfo*) m_listbox_b->GetItemData( item ))->GetLayer() );
    }

    m_habitat->SetGroupA( groupa );
    m_habitat->SetGroupB( groupb );
    m_habitat->SetSelectedOnlyA( m_onlyselect_A->GetValue() );
    m_habitat->SetSelectedOnlyB( m_onlyselect_B->GetValue() );
    m_habitat->SetClearTarget( m_clearTarget->GetValue() );
    m_habitat->SetTarget( m_target );

    EndModal( 0 );
}

void a2dGroupDlg::CmCancel( wxCommandEvent& )
{
    EndModal( 1 );
}
