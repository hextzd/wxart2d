/*! \file canvas/src/canorderdlg.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canorderdlg.cpp,v 1.8 2009/10/01 19:22:35 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifdef __GNUG__
#pragma implementation
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/layerinf.h"

#include "wx/canvas/canorderdlg.h"

const int    ORDER_LIST_SELECT   = wxID_HIGHEST + 5108 ;

BEGIN_EVENT_TABLE( a2dDragListBox, wxListBox )
    EVT_LISTBOX( ORDER_LIST_SELECT, a2dDragListBox::CmListBox )
    EVT_LEFT_DOWN( a2dDragListBox::OnMouseLeftDown )
    EVT_LEFT_UP( a2dDragListBox::OnMouseLeftUp )
END_EVENT_TABLE()

a2dDragListBox::a2dDragListBox( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                                int n, const wxString choices[], long style )
    : wxListBox   ( parent, id, pos, size, n, choices, style )
{
    m_selection = 0;
}

void a2dDragListBox::OnMouseLeftDown( class wxMouseEvent& event )
{
    wxPoint point = wxPoint( event.m_x, event.m_y );
    m_selection = HitTest( point );

    event.Skip();
}

void a2dDragListBox::OnMouseLeftUp( class wxMouseEvent& event )
{
    int pasteLoc = GetSelection();
    if ( m_selection != -1 && m_selection != pasteLoc )
    {
        if ( m_selection < pasteLoc )
        {
            int row;
            for ( row = m_selection; row < pasteLoc; row++ )
            {
                wxString selectLocStr = GetString( row );
                wxString copyLocStr = GetString( row + 1 );

                SetString( row, copyLocStr );
                SetString( row + 1, selectLocStr );
            }

        }
        else
        {
            int row;
            for ( row = m_selection; row > pasteLoc; row-- )
            {
                wxString selectLocStr = GetString( row );
                wxString copyLocStr = GetString( row - 1 );

                SetString( row, copyLocStr );
                SetString( row - 1, selectLocStr );
            }

        }
    }
    event.Skip();
}

void a2dDragListBox::CmListBox( wxCommandEvent& selevent )
{
    m_selection = GetSelection();
    selevent.Skip();
}


const int    ORDER_BUTTON_OK     = wxID_HIGHEST + 5101 ;
const int    ORDER_BUTTON_CANCEL = wxID_HIGHEST + 5102 ;

const int    ORDER_BUTTON_UP     = wxID_HIGHEST + 5103 ;

const int    ORDER_BUTTON_DOWN   = wxID_HIGHEST + 5104 ;

const int    ORDER_BUTTON_SHOW   = wxID_HIGHEST + 5106 ;

const int    ORDER_UPDATE_DIALOG = wxID_HIGHEST + 5107 ;


BEGIN_EVENT_TABLE( a2dLayerOrderDlg, wxDialog )

    EVT_BUTTON   ( ORDER_BUTTON_OK,      a2dLayerOrderDlg::CmOk )

    EVT_BUTTON   ( ORDER_BUTTON_CANCEL,  a2dLayerOrderDlg::CmCancel )

    EVT_BUTTON    ( ORDER_BUTTON_UP,   a2dLayerOrderDlg::CmUp )

    EVT_BUTTON    ( ORDER_BUTTON_DOWN,   a2dLayerOrderDlg::CmDown )

    EVT_BUTTON    ( ORDER_BUTTON_SHOW,   a2dLayerOrderDlg::CmShow )

    EVT_LISTBOX  ( ORDER_LIST_SELECT,   a2dLayerOrderDlg::CmListBox )

    EVT_CLOSE    ( a2dLayerOrderDlg::OnCloseWindow )

END_EVENT_TABLE()


a2dLayerOrderDlg::a2dLayerOrderDlg( a2dHabitat* habitat, wxWindow* parent, a2dLayers* layersetup, const wxString& title, long style, const wxString& name ):
    wxDialog( parent, ORDER_UPDATE_DIALOG, title, wxPoint( 0, 0 ), wxSize( 200, 500 ), style, name )
{
    m_habitat = habitat;
    //a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_ACTIVATE_VIEW, this );
    //a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    //a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );

    a2dLayerOrderDlg* itemDialog1 = this;

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

a2dLayerOrderDlg::~a2dLayerOrderDlg()
{
    //a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_ACTIVATE_VIEW, this );
    //a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    //a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( a2dEVT_COM_EVENT, this );
}

void a2dLayerOrderDlg::Init( a2dLayers* layersetup )
{
    if ( !layersetup )
        return;

    m_layersetup = layersetup;

    m_listbox->Clear();

    wxUint16 currentLayer = m_habitat->GetLayer();

    for ( a2dLayerIndex::iterator it= m_layersetup->GetOrderSort().begin(); it != m_layersetup->GetOrderSort().end(); ++it)
    {
        a2dLayerInfo* layerinfo = *it;
        wxUint16 j = layerinfo->GetLayer();

        wxString buf;
        // insert the layer in the listbox
        buf.Printf( "%04d  %s", j, layerinfo->GetName().c_str() );
        if ( currentLayer == layerinfo->GetLayer() )
            buf = buf + " <=";

        m_listbox->Append( buf );
    }
}

void a2dLayerOrderDlg::StoreOrder()
{
    wxString layernr;
    for ( a2dLayerIndex::iterator it= m_layersetup->GetOrderSort().begin(); it != m_layersetup->GetOrderSort().end(); ++it)
    {
        a2dLayerInfo* layerinfo = *it;
        wxUint16 i = layerinfo->GetLayer();

        layernr = m_listbox->GetString( i ).Left( 4 );
        layerinfo = m_layersetup->GetLayerIndex()[ wxAtoi( layernr ) ];
        layerinfo->SetOrder( i * 10 );
    }
     
    m_layersetup->SetPending( true );

    SetEvtHandlerEnabled( false );
    a2dComEvent changedlayers( m_layersetup->GetRoot(), m_layersetup, a2dDrawing::sig_layersetupChanged );
    ProcessEvent( changedlayers );
    SetEvtHandlerEnabled( true );
}

void a2dLayerOrderDlg::CmOk( wxCommandEvent& )
{
    int selection = m_listbox->GetSelection();
    m_listbox->SetSelection( selection, FALSE );

    StoreOrder();
    Show( FALSE );
}

void a2dLayerOrderDlg::CmShow( wxCommandEvent& )
{
    int selection = m_listbox->GetSelection();
    m_listbox->SetSelection( selection, FALSE );

    StoreOrder();
}

void a2dLayerOrderDlg::OnCloseWindow( wxCloseEvent& event )
{
    wxCommandEvent a = wxCommandEvent();
    CmCancel( a );
}

void a2dLayerOrderDlg::CmCancel( wxCommandEvent& )
{
    Show( FALSE );

    m_listbox->Clear();

    // old values reread
    //Init();
}

void a2dLayerOrderDlg::CmUp( wxCommandEvent& )
{
    int selection = m_listbox->GetSelection();
    if ( selection == -1 )
        ( void )wxMessageBox( _T( "You must select a layer first" ), _T( "a2dLayerOrderDlg" ), wxICON_INFORMATION | wxOK );
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
            ( void )wxMessageBox( _T( "layer is allready at the highest possible order" ), _T( "a2dLayerOrderDlg" ), wxICON_INFORMATION | wxOK );
        }
    }
}

void a2dLayerOrderDlg::CmDown( wxCommandEvent& )
{
    int selection = m_listbox->GetSelection();

    if ( selection == -1 )
        ( void )wxMessageBox( _T( "You must select a layer first" ), _T( "a2dLayerOrderDlg" ), wxICON_INFORMATION | wxOK );
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
            ( void )wxMessageBox( _T( "layer is allready at the lowest possible order" ), _T( "a2dLayerOrderDlg" ), wxICON_INFORMATION | wxOK );
        }
    }
}

void a2dLayerOrderDlg::CmListBox( wxCommandEvent& )
{
    int selection = m_listbox->GetSelection();
    m_buttonUP->Enable( selection != 0 );
    m_buttonDOWN->Enable( selection != m_layersetup->GetOrderSort().size() - 1 );

	m_habitat->SetLayer( selection, true );
}

void a2dLayerOrderDlg::OnComEvent( a2dComEvent& event )
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





BEGIN_EVENT_TABLE( a2dLayerDlg, wxDialog )
    EVT_BUTTON   ( ORDER_BUTTON_OK,      a2dLayerDlg::CmOk )
    EVT_BUTTON   ( ORDER_BUTTON_CANCEL,  a2dLayerDlg::CmCancel )
    EVT_LISTBOX  ( ORDER_LIST_SELECT,   a2dLayerDlg::CmListBox )
    EVT_CLOSE    ( a2dLayerDlg::OnCloseWindow )
END_EVENT_TABLE()


a2dLayerDlg::a2dLayerDlg( a2dHabitat* habitat, wxWindow* parent, a2dLayers* layersetup, bool onOrder, bool modal, const wxString& title, const wxString& name ):
    wxDialog( parent, ORDER_UPDATE_DIALOG, title, wxPoint( 0, 0 ),  wxDefaultSize, wxRESIZE_BORDER|wxDEFAULT_DIALOG_STYLE, name )
{
    m_habitat = habitat;
    m_layerselected = m_habitat->GetLayer();

    a2dLayerDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxHORIZONTAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxString* m_listboxStrings = NULL;
    m_listbox = new wxListBox( itemDialog1, ORDER_LIST_SELECT, wxDefaultPosition,  wxSize( 200, 500 ), 0, m_listboxStrings, wxLB_SINGLE | wxLB_NEEDED_SB );
    m_listbox->SetHelpText( _( "Select a layer" ) );
    m_listbox->SetToolTip( _( "Select a layer" ) );
    itemBoxSizer2->Add( m_listbox, 1, wxGROW | wxALL, 0 );

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer2->Add( itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0 );

    m_modal = modal;
    if ( m_modal )
        m_buttonOK = new wxButton( itemDialog1, ORDER_BUTTON_OK, _( "Oke" ), wxDefaultPosition, wxSize( 55, 22 ), 0 );
    else
        m_buttonOK = new wxButton( itemDialog1, ORDER_BUTTON_OK, _( "Hide" ), wxDefaultPosition, wxSize( 55, 22 ), 0 );
    itemBoxSizer4->Add( m_buttonOK, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

    m_buttonCANCEL = new wxButton( itemDialog1, ORDER_BUTTON_CANCEL, _( "Cancel" ), wxDefaultPosition, wxSize( 55, 22 ), 0 );
    itemBoxSizer4->Add( m_buttonCANCEL, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

    itemBoxSizer4->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
    Centre();

    m_layerselected = 0;
    m_onOrder = onOrder;
    Init( layersetup );
}

a2dLayerDlg::~a2dLayerDlg()
{
}

void a2dLayerDlg::Init( a2dLayers* layersetup )
{
    if ( !layersetup )
        return;

    m_layersetup = layersetup;

    m_listbox->Clear();

    if ( m_onOrder )
    {
        for ( a2dLayerIndex::iterator it= m_layersetup->GetOrderSort().begin(); it != m_layersetup->GetOrderSort().end(); ++it)
        {
            a2dLayerInfo* layerinfo = *it;
            wxUint16 j = layerinfo->GetLayer();

            wxString buf;
            // insert the layer in the listbox
            buf.Printf( "%04d  %s", j, layerinfo->GetName().c_str() );
            if ( m_layerselected == layerinfo->GetLayer() )
                buf = buf + " <=";

            m_listbox->Append( buf );
            
        }
    }
    else     
    {
        for ( a2dLayerIndex::iterator it= m_layersetup->GetLayerSort().begin(); it != m_layersetup->GetLayerSort().end(); ++it)
        {
            a2dLayerInfo* layerinfo = *it;
            wxUint16 j = layerinfo->GetLayer();

            wxString buf;
            // insert the layer in the listbox
            buf.Printf( "%04d  %s", j, layerinfo->GetName().c_str() );
            if ( m_layerselected == layerinfo->GetLayer() )
                buf = buf + " <=";

            m_listbox->Append( buf ); 
        }
    }
}

void a2dLayerDlg::CmOk( wxCommandEvent& )
{
    if ( IsModal() )
        EndModal( wxID_OK );
    else
    {
        SetReturnCode( wxID_OK );
        Show( false );
    }
}

void a2dLayerDlg::OnCloseWindow( wxCloseEvent& event )
{
    if ( IsModal() )
        EndModal( wxID_CANCEL );
    else
    {
        SetReturnCode( wxID_CANCEL );
        Show( false );
    }
}

void a2dLayerDlg::CmCancel( wxCommandEvent& )
{
    if ( IsModal() )
        EndModal( wxID_CANCEL );
    else
    {
        SetReturnCode( wxID_CANCEL );
        Show( false );
    }
}

void a2dLayerDlg::CmListBox( wxCommandEvent& )
{
    wxUint16 i = 0;
    wxUint16 index = m_listbox->GetSelection();

    if ( m_onOrder )
    {
        a2dLayerIndex::iterator it= m_layersetup->GetOrderSort().begin();
        while ( i < index )
        {
            ++it;
            i++;
        }
        a2dLayerInfo* layerinfo = *it;
        m_layerselected = layerinfo->GetLayer();
    }
    else     
    {
        a2dLayerIndex::iterator it= m_layersetup->GetLayerSort().begin();
        while ( i < index )
        {
            ++it;
            i++;
        }
        a2dLayerInfo* layerinfo = *it;
        m_layerselected = layerinfo->GetLayer();
    }

    Init( m_layersetup );

    if ( !IsModal() )
    {
		m_habitat->SetLayer( m_layerselected, false );
    }
}

void a2dLayerDlg::OnComEvent( a2dComEvent& event )
{
    if ( GetEvtHandlerEnabled() )
    {
		a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();

        if ( !part || !part->GetDrawing() )
            return;

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
            a2dCanvasObject* obj = wxDynamicCast( event.GetEventObject(), a2dCanvasObject );
            if ( m_layersetup->GetRoot() == obj->GetRoot() )
                Init( m_layersetup );
        }
		else if ( event.GetId() == a2dCanvasGlobal::sig_changedActiveDrawing )
		{
			a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();
			if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
			{
				if ( m_layersetup != part->GetShowObject()->GetRoot()->GetLayerSetup() )
				{
					part->GetShowObject()->GetRoot()->SetAvailable();
				    part->GetDrawing()->DisconnectEvent( a2dEVT_COM_EVENT, this );
					Init( part->GetShowObject()->GetRoot()->GetLayerSetup() );
				    part->GetDrawing()->ConnectEvent( a2dEVT_COM_EVENT, this );

				}
			}
		}
    }
}
