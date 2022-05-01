/*! \file editor/src/orderdlg.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: orderdlg.cpp,v 1.8 2009/10/01 19:22:35 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifdef __GNUG__
#pragma implementation
#endif

#include "wx/canvas/canmod.h"
#include "wx/editor/candocproc.h"
#include "wx/canvas/layerinf.h"
#include "wx/canvas/canorderdlg.h"

#include "wx/editor/orderdlg.h"

const int    ORDER_LIST_SELECT   = wxID_HIGHEST + 5108 ;

const int    ORDER_BUTTON_OK     = wxID_HIGHEST + 5101 ;
const int    ORDER_BUTTON_CANCEL = wxID_HIGHEST + 5102 ;

const int    ORDER_BUTTON_UP     = wxID_HIGHEST + 5103 ;

const int    ORDER_BUTTON_DOWN   = wxID_HIGHEST + 5104 ;

const int    ORDER_BUTTON_SHOW   = wxID_HIGHEST + 5106 ;

const int    ORDER_UPDATE_DIALOG = wxID_HIGHEST + 5107 ;


BEGIN_EVENT_TABLE( a2dLayerOrderDialog, wxDialog )

    EVT_CHANGED_DOCUMENT( a2dLayerOrderDialog::OnChangedDocument )

    EVT_BUTTON   ( ORDER_BUTTON_OK,      a2dLayerOrderDialog::CmOk )

    EVT_BUTTON   ( ORDER_BUTTON_CANCEL,  a2dLayerOrderDialog::CmCancel )

    EVT_BUTTON    ( ORDER_BUTTON_UP,   a2dLayerOrderDialog::CmUp )

    EVT_BUTTON    ( ORDER_BUTTON_DOWN,   a2dLayerOrderDialog::CmDown )

    EVT_BUTTON    ( ORDER_BUTTON_SHOW,   a2dLayerOrderDialog::CmShow )

    EVT_LISTBOX  ( ORDER_LIST_SELECT,   a2dLayerOrderDialog::CmListBox )

    EVT_CLOSE    ( a2dLayerOrderDialog::OnCloseWindow )

    EVT_ACTIVATE_VIEW( a2dLayerOrderDialog::OnActivateView )

END_EVENT_TABLE()


a2dLayerOrderDialog::a2dLayerOrderDialog( wxWindow* parent, a2dLayers* layersetup, const wxString& title, long style, const wxString& name ):
    wxDialog( parent, ORDER_UPDATE_DIALOG, title, wxPoint( 0, 0 ), wxSize( 200, 500 ), style, name )
{
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );

    a2dLayerOrderDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxHORIZONTAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxString* m_listboxStrings = NULL;
    m_listbox = new a2dDragListBox( itemDialog1, ORDER_LIST_SELECT, wxDefaultPosition, wxSize( 100, 500 ), 0, m_listboxStrings, wxLB_SINGLE | wxLB_NEEDED_SB );
    m_listbox->SetHelpText( _( "Select a layer and press up or down button" ) );
    m_listbox->SetToolTip( _( "Select a layer and press up or down button" ) );
    itemBoxSizer2->Add( m_listbox, 1, wxGROW | wxALL, 0 );

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer2->Add( itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0 );

    m_buttonOK = new wxButton( itemDialog1, ORDER_BUTTON_OK, _( "Hide" ), wxDefaultPosition, wxSize( 55, 22 ), 0 );
    itemBoxSizer4->Add( m_buttonOK, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

    m_buttonCANCEL = new wxButton( itemDialog1, ORDER_BUTTON_CANCEL, _( "Cancel" ), wxDefaultPosition, wxSize( 55, 22 ), 0 );
    itemBoxSizer4->Add( m_buttonCANCEL, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

    itemBoxSizer4->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    m_buttonUP = new wxButton( itemDialog1, ORDER_BUTTON_UP, _( "Up" ), wxDefaultPosition, wxSize( 55, 22 ), 0 );
    itemBoxSizer4->Add( m_buttonUP, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

    m_buttonDOWN = new wxButton( itemDialog1, ORDER_BUTTON_DOWN, _( "Down" ), wxDefaultPosition, wxSize( 55, 22 ), 0 );
    itemBoxSizer4->Add( m_buttonDOWN, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

    m_buttonSHOW = new wxButton( itemDialog1, ORDER_BUTTON_SHOW, _( "Apply" ), wxDefaultPosition, wxSize( 55, 22 ), 0 );
    itemBoxSizer4->Add( m_buttonSHOW, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
    Centre();

    Init( layersetup );
}

a2dLayerOrderDialog::~a2dLayerOrderDialog()
{
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( a2dEVT_COM_EVENT, this );
}

void a2dLayerOrderDialog::Init( a2dLayers* layersetup )
{
    if ( !layersetup )
        return;

    m_layersetup = layersetup;

    m_listbox->Clear();

    for ( a2dLayerIndex::iterator it= m_layersetup->GetOrderSort().begin(); it != m_layersetup->GetOrderSort().end(); ++it)
    {
        a2dLayerInfo* layerinfo = *it;
        wxUint16 j = (*it)->GetLayer();

        wxString buf;
        // insert the layer in the listbox
        buf.Printf( "%04d  %s", j, layerinfo->GetName().c_str() );

        m_listbox->Append( buf );
    }
}

void a2dLayerOrderDialog::OnActivateView( a2dViewEvent& viewevent )
{
    a2dCanvasView* view = wxDynamicCast( viewevent.GetEventObject(), a2dCanvasView );
    if ( view && view->GetDrawingPart()->GetShowObject() && view->GetDrawingPart()->GetShowObject()->GetRoot() )
    {
        view->GetDrawingPart()->GetShowObject()->GetRoot()->SetAvailable();
        Init( view->GetDrawingPart()->GetShowObject()->GetRoot()->GetLayerSetup() );
    }
}

void a2dLayerOrderDialog::OnChangedDocument( a2dCommandEvent& event )
{
/*
    a2dCanvasDocument* doc = ( a2dCanvasDocument* ) event.GetDocument();
    if ( doc && doc->GetLayerSetup() != m_layersetup )
    {
        doc->GetLayerSetup()->SetAvailable( doc );
        Init( doc->GetLayerSetup() );
    }
*/
}

void a2dLayerOrderDialog::StoreOrder()
{
    wxString layernr;
     wxUint16 i = 0;
    for ( a2dLayerIndex::iterator it= m_layersetup->GetOrderSort().begin(); it != m_layersetup->GetOrderSort().end(); ++it)
    {
        a2dLayerInfo* layerinfo = *it;
        layernr = m_listbox->GetString( i ).Left( 4 );
        layerinfo = m_layersetup->GetLayerIndex()[ wxAtoi( layernr ) ];
        layerinfo->SetOrder( i * 10 );
        i++;
    }
    m_layersetup->SetPending( true );

    SetEvtHandlerEnabled( false );
    a2dComEvent changedlayers( m_layersetup->GetRoot(), m_layersetup, a2dCanvasDocument::sig_layersetupChanged );
    ProcessEvent( changedlayers );
    SetEvtHandlerEnabled( true );
}

void a2dLayerOrderDialog::CmOk( wxCommandEvent& )
{
    int selection = m_listbox->GetSelection();
    m_listbox->SetSelection( selection, FALSE );

    StoreOrder();
    Show( FALSE );
}

void a2dLayerOrderDialog::CmShow( wxCommandEvent& )
{
    int selection = m_listbox->GetSelection();
    m_listbox->SetSelection( selection, FALSE );

    StoreOrder();
}

void a2dLayerOrderDialog::OnCloseWindow( wxCloseEvent& event )
{
    wxCommandEvent a = wxCommandEvent();
    CmCancel( a );
}

void a2dLayerOrderDialog::CmCancel( wxCommandEvent& )
{
    Show( FALSE );

    m_listbox->Clear();

    // old values reread
    //Init();
}

void a2dLayerOrderDialog::CmUp( wxCommandEvent& )
{
    int selection = m_listbox->GetSelection();
    if ( selection == -1 )
        ( void )wxMessageBox( _T( "You must select a layer first" ), _T( "a2dLayerOrderDialog" ), wxICON_INFORMATION | wxOK );
    else
    {
        if ( selection != 0 )
        {
            if ( selection == m_layersetup->GetOrderSort().size() - 1 ) m_buttonDOWN->Enable( TRUE );

            wxString selected = m_listbox->GetString( m_listbox->GetSelection() - 1 );

            m_listbox->SetString( selection - 1, m_listbox->GetString( selection ) );
            m_listbox->SetString( selection, selected );
            m_listbox->Show( TRUE );

            // to keep the original selection selected
            m_listbox->SetSelection( selection, FALSE );
            m_listbox->SetSelection( selection - 1, TRUE );
            if ( selection == 1 )
                m_buttonUP->Enable( FALSE );
        }
        else
        {
            ( void )wxMessageBox( _T( "layer is allready at the highest possible order" ), _T( "a2dLayerOrderDialog" ), wxICON_INFORMATION | wxOK );
        }
    }
}

void a2dLayerOrderDialog::CmDown( wxCommandEvent& )
{
    int selection = m_listbox->GetSelection();

    if ( selection == -1 )
        ( void )wxMessageBox( _T( "You must select a layer first" ), _T( "a2dLayerOrderDialog" ), wxICON_INFORMATION | wxOK );
    else
    {
        if ( selection != m_layersetup->GetOrderSort().size() - 1 )
        {
            if ( selection == 0 ) m_buttonUP->Enable( TRUE );

            wxString selected = m_listbox->GetString( m_listbox->GetSelection() + 1 );

            m_listbox->SetString( selection + 1, m_listbox->GetString( selection ) );
            m_listbox->SetString( selection, selected );
            m_listbox->Show( TRUE );

            // to keep the original selection selected
            m_listbox->SetSelection( selection, FALSE );
            m_listbox->SetSelection( selection + 1, TRUE );

            if ( selection == m_layersetup->GetOrderSort().size() - 2 ) m_buttonDOWN->Enable( FALSE );
        }
        else
        {
            ( void )wxMessageBox( _T( "layer is allready at the lowest possible order" ), _T( "a2dLayerOrderDialog" ), wxICON_INFORMATION | wxOK );
        }
    }
}

void a2dLayerOrderDialog::CmListBox( wxCommandEvent& )
{
    int selection = m_listbox->GetSelection();
    m_buttonUP->Enable( selection != 0 );
    m_buttonDOWN->Enable( selection != m_layersetup->GetLayerIndex().size() - 1 );

    wxString layernr = m_listbox->GetString( selection ).Left( 4 );
    a2dLayerInfo* layerinfo = m_layersetup->GetLayerIndex()[ wxAtoi( layernr ) ];

	a2dCanvasGlobals->GetHabitat()->SetLayer( layerinfo->GetLayer(), true );
}

void a2dLayerOrderDialog::OnComEvent( a2dComEvent& event )
{
    if ( GetEvtHandlerEnabled() )
    {
        if ( event.GetId() == a2dDrawingPart::sig_changedLayers )
        {
            a2dDrawing* doc = wxDynamicCast( event.GetEventObject(), a2dDrawing );
            if ( doc && doc == m_layersetup->GetRoot() )
                Init( doc->GetLayerSetup() );
        }
        else if ( event.GetId() == a2dDrawing::sig_layersetupChanged )
        {
            a2dDrawing* doc = wxDynamicCast( event.GetEventObject(), a2dDrawing );
            if ( doc && doc == m_layersetup->GetRoot() )
                Init( doc->GetLayerSetup() );
        }
        else if ( event.GetId() == a2dLayerInfo::sig_changedLayerInfo )
        {
            a2dDrawing* obj = wxDynamicCast( event.GetEventObject(), a2dDrawing );
            if ( m_layersetup->GetRoot() == obj->GetRootObject()->GetRoot() )
                Init( m_layersetup );
        }

    }
}

