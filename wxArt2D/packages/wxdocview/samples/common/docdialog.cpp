/*! \file docview/samples/common/docdialog.cpp
    \brief View classes
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: docdialog.cpp,v 1.2 2008/10/30 23:18:09 titato Exp $
*/

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "docdialog.h"


const int    DOCS_BUTTON_OK     = wxID_HIGHEST + 5401 ;
const int    DOCS_BUTTON_CANCEL = wxID_HIGHEST + 5402 ;
const int    DOCS_BUTTON_SHOW   = wxID_HIGHEST + 5403 ;
const int    DOCS_UPDATE_DIALOG = wxID_HIGHEST + 5404 ;


BEGIN_EVENT_TABLE( DocumentListDialog, wxDialog )
    EVT_BUTTON    ( DOCS_BUTTON_OK,  DocumentListDialog::CmOk )
    EVT_BUTTON    ( DOCS_BUTTON_CANCEL ,  DocumentListDialog::CmCancel )
    EVT_CLOSE     ( DocumentListDialog::OnCloseWindow )
END_EVENT_TABLE()


DocumentListDialog::DocumentListDialog( wxFrame* parent, const a2dDocumentList& total ):
    wxDialog( parent, DOCS_UPDATE_DIALOG, _T( "documents" ), wxPoint( 0, 0 ), wxSize( 200, 500 ), wxCAPTION, _T( "Documents dialog" ) )
{
    m_parent = parent;

    wxBoxSizer* sizerw = new wxBoxSizer( wxVERTICAL );

    wxStaticBox* box = new wxStaticBox( this, -1, _T( "Document" ) );
    wxStaticBoxSizer* sizer = new wxStaticBoxSizer( box, wxVERTICAL );

    wxStaticText* mess1 = new wxStaticText( this, -1, _T( "nop" ) );
    wxStaticText* mess2 = new wxStaticText( this, -1, _T( "Select a Document in the list above" ) );
    m_listbox   = new wxListBox( this, -1, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_NEEDED_SB | wxLB_SINGLE );

    sizer->Add( mess1 ,  0, wxEXPAND  );
    sizer->Add( m_listbox ,  1, wxEXPAND  );
    sizer->Add( mess2 ,  0, wxEXPAND  );
    sizer->Add( new wxButton( this, DOCS_BUTTON_OK,      _T( "Ok" ), wxDefaultPosition, wxSize( 55, 20 ) ) ,  0 );
    sizer->Add( new wxButton( this, DOCS_BUTTON_CANCEL,  _T( "Cancel" ), wxDefaultPosition, wxSize( 55, 20 ) ) ,  0 );

    wxString mess;

    m_documentlist = &total;
    m_listbox->Clear();

    const_forEachIn( a2dDocumentList, m_documentlist )
    {
        a2dDocument* obj = *iter;
        m_listbox->Append( obj->GetFilename().GetFullName() );
    }
    mess.Printf( _T( "Number of Documents: %i" ), m_documentlist->size() );
    mess1->SetLabel( mess );

    // set the first structure as default and give it the focus
    if ( m_documentlist->size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;

    sizerw->Add( sizer, 1, wxEXPAND );
    SetSizer( sizerw );
    SetAutoLayout( true );
    sizerw->SetSizeHints( this );
    sizerw->Fit( this );

    Layout();
    Centre( wxBOTH );
}

DocumentListDialog::~DocumentListDialog()
{
}

void DocumentListDialog::CmOk( wxCommandEvent& event )
{
    wxString buf;
    buf = m_listbox->GetStringSelection();
    const_forEachIn( a2dDocumentList, m_documentlist )
    {
        a2dDocument* obj = *iter;
        wxString name = obj->GetFilename().GetFullName();
        if ( !name.Cmp( buf ) )
        {
            m_object = obj;
            break;
        }
    }
    EndModal( wxID_OK );
}

a2dDocument* DocumentListDialog::GetDocument()
{
    return m_object;
}

void DocumentListDialog::CmCancel( wxCommandEvent& event )
{
    EndModal( wxID_CANCEL );
}

void DocumentListDialog::OnCloseWindow( wxCloseEvent& event )
{
    wxCommandEvent eventc;
    CmCancel( eventc );
}

const int    VIEW_BUTTON_OK     = wxID_HIGHEST + 5405 ;
const int    VIEW_BUTTON_CANCEL = wxID_HIGHEST + 5406 ;
const int    VIEW_BUTTON_SHOW   = wxID_HIGHEST + 5407 ;
const int    VIEW_UPDATE_DIALOG = wxID_HIGHEST + 5408 ;

BEGIN_EVENT_TABLE( ViewListDialog, wxDialog )
    EVT_BUTTON    ( VIEW_BUTTON_OK,  ViewListDialog::CmOk )
    EVT_BUTTON    ( VIEW_BUTTON_CANCEL ,  ViewListDialog::CmCancel )
    EVT_CLOSE     ( ViewListDialog::OnCloseWindow )
END_EVENT_TABLE()


ViewListDialog::ViewListDialog( wxFrame* parent, const a2dViewList& allviews ):
    wxDialog( parent, VIEW_UPDATE_DIALOG, _T( "documents" ), wxPoint( 0, 0 ), wxSize( 200, 500 ), wxCAPTION, _T( "Documents dialog" ) )
{
    m_parent = parent;

    wxBoxSizer* sizerw = new wxBoxSizer( wxVERTICAL );

    wxStaticBox* box = new wxStaticBox( this, -1, _T( "Document" ) );
    wxStaticBoxSizer* sizer = new wxStaticBoxSizer( box, wxVERTICAL );

    wxStaticText* mess1 = new wxStaticText( this, -1, _T( "nop" ) );
    wxStaticText* mess2 = new wxStaticText( this, -1, _T( "Select a Document in the list above" ) );
    m_listbox   = new wxListBox( this, -1, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_NEEDED_SB | wxLB_SINGLE );

    sizer->Add( mess1 ,  0, wxEXPAND  );
    sizer->Add( m_listbox ,  1, wxEXPAND  );
    sizer->Add( mess2 ,  0, wxEXPAND  );
    sizer->Add( new wxButton( this, VIEW_BUTTON_OK,      _T( "Ok" ), wxDefaultPosition, wxSize( 55, 20 ) ) ,  0 );
    sizer->Add( new wxButton( this, VIEW_BUTTON_CANCEL,  _T( "Cancel" ), wxDefaultPosition, wxSize( 55, 20 ) ) ,  0 );

    wxString mess;

    m_viewlist = &allviews;
    m_listbox->Clear();

    int n = 0;
    const_forEachIn( a2dViewList, m_viewlist )
    {
        a2dView* obj = *iter;
        wxString s;
        s << _T( "View" ) << n << _T( " " ) << obj->GetViewTypeName();
        m_listbox->Append( s );
        n++;
    }

    mess.Printf( _( "Number of Documents: %i" ), m_viewlist->size() );
    mess1->SetLabel( mess );

    // set the first structure as default and give it the focus
    if ( m_viewlist->size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;

    sizerw->Add( sizer, 1, wxEXPAND );
    SetSizer( sizerw );
    SetAutoLayout( true );
    sizerw->SetSizeHints( this );
    sizerw->Fit( this );

    Layout();
    Centre( wxBOTH );
}

ViewListDialog::~ViewListDialog()
{
}

void ViewListDialog::CmOk( wxCommandEvent& event )
{
    wxString buf;
    int n = m_listbox->GetSelection();
    int i;
    a2dViewList::const_iterator iter = m_viewlist->begin();
    for ( i = 0; i <= n; i++ )
    {
        m_object = *iter;
        iter++;
    }
    EndModal( wxID_OK );
}

a2dView* ViewListDialog::GetView()
{
    return m_object;
}

void ViewListDialog::CmCancel( wxCommandEvent& event )
{
    EndModal( wxID_CANCEL );
}

void ViewListDialog::OnCloseWindow( wxCloseEvent& event )
{
    wxCommandEvent eventc;
    CmCancel( eventc );
}
