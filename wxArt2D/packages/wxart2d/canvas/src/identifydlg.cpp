/*! \file editor/src/identifydlg.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: identifydlg.cpp,v 1.7 2009/09/03 20:09:53 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/layerinf.h"

#include "wx/canvas/identifydlg.h"


IMPLEMENT_DYNAMIC_CLASS( IdentifyDialog, wxDialog )

BEGIN_EVENT_TABLE( IdentifyDialog, wxDialog )
    EVT_BUTTON    ( ID_IDENT_HIDE,  IdentifyDialog::hide )
    EVT_BUTTON    ( ID_IDENT_UPDATE, IdentifyDialog::Update )
    EVT_ACTIVATE  ( IdentifyDialog::OnActivate )
    //EVT_ACTIVATE_VIEW( IdentifyDialog::OnActivateView )
    EVT_CLOSE     ( IdentifyDialog::OnCloseWindow )
    EVT_SIZE      ( IdentifyDialog::OnSize )
    EVT_DO        ( IdentifyDialog::OnDoEvent )
    EVT_COM_EVENT ( IdentifyDialog::OnComEvent )
    //EVT_CHANGED_DOCUMENT( IdentifyDialog::OnChangedDocument )
END_EVENT_TABLE()

/*!
 * IdentifyDialog constructors
 */
IdentifyDialog::IdentifyDialog( )
{
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );

/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_REDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );
*/
    m_showObject = NULL;
}

IdentifyDialog::IdentifyDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );
    Create( parent, id, caption, pos, size, style );
/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_REDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );
*/
}

IdentifyDialog::~IdentifyDialog( )
{
    a2dCanvasGlobals->DisconnectEvent( a2dEVT_COM_EVENT, this );

/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_REDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( a2dEVT_COM_EVENT, this );
*/
}


/*!
 * IdentifyDialog creator
 */
bool IdentifyDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_showObject = NULL;
    m_objectsList = NULL;
    m_update = NULL;

    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    //GetSizer()->SetSizeHints(this);
    //GetSizer()->Fit(this);
    Centre();
    return TRUE;
}

/*!
 * Control creation for IdentifyDialog
 */
void IdentifyDialog::CreateControls()
{
    IdentifyDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxString* m_objectsListStrings = NULL;
    m_objectsList = new wxListBox( itemDialog1, ID_IDENT_LISTBOX, wxDefaultPosition, wxDefaultSize, 0, m_objectsListStrings, wxLB_SINGLE | wxLB_NEEDED_SB | wxLB_SORT );
    m_objectsList->SetHelpText( _( "Object selected for identify" ) );
    if ( ShowToolTips() )
        m_objectsList->SetToolTip( _( "Object selected for identify" ) );
    itemBoxSizer2->Add( m_objectsList, 1, wxGROW | wxALL, 0 );

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_IDENT_HIDE, _( "Hide" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton5->SetHelpText( _( "Hide dialog" ) );
    if ( ShowToolTips() )
        itemButton5->SetToolTip( _( "Hide dialog" ) );
    itemBoxSizer4->Add( itemButton5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0 );

    m_update = new wxButton( itemDialog1, ID_IDENT_UPDATE, _( "Update" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_update->SetHelpText( _( "Update dialog for selected objects in view" ) );
    if ( ShowToolTips() )
        m_update->SetToolTip( _( "Update dialog for selected objects in view" ) );
    itemBoxSizer4->Add( m_update, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0 );
}

/*!
 * Should we show tooltips?
 */
bool IdentifyDialog::ShowToolTips()
{
    return TRUE;
}

void IdentifyDialog::hide( wxCommandEvent& )
{
    m_objectsList->Clear();
    Show( FALSE );
}

void IdentifyDialog::OnCloseWindow( wxCloseEvent& event )
{
    m_objectsList->Clear();
    Show( FALSE );
}

void IdentifyDialog::Update( wxCommandEvent& )
{
    Init( m_showObject );
}

void IdentifyDialog::Init( a2dCanvasObject* showObject )
{
    if ( !m_objectsList )
        return;
    m_objectsList->Clear();
    if ( !showObject )
        return;

    m_showObject = showObject;

    a2dCanvasObjectList* childList = m_showObject->GetChildObjectList();
    for( a2dCanvasObjectList::iterator iter = childList->begin(); iter != childList->end(); ++iter )
    {
        a2dCanvasObject* obj = ( *iter );
        if ( obj->GetSelected() )
        {
            wxString layerName;
            a2dDrawing* root = obj->GetRoot();
            a2dLayerInfo* layerobj = wxNullLayerInfo;
            if ( root->GetLayerSetup() )
                layerobj = root->GetLayerSetup()->GetLayerIndex()[obj->GetLayer()];
            if ( layerobj != wxNullLayerInfo )
                layerName = layerobj->GetName();

            wxString listline;
            listline = wxString::Format( wxT( "layerId=%d layer=%s " ), obj->GetLayer(), layerName.c_str() );
            listline += wxString::Format( wxT( "x=%lg, y=%lg " ), obj->GetPosX(), obj->GetPosY() );
            listline << wxT( "name=" ) << obj->GetName() << wxT( " class=" ) << obj->GetClassInfo()->GetClassName();
            m_objectsList->Append( listline );
        }
    }
}

void IdentifyDialog::OnActivate( wxActivateEvent& windowevent )
{
    windowevent.Skip(); //skip to base OnActivate Handler which sets proper focus to child window(s)
}

/*
void IdentifyDialog::OnActivateView( a2dViewEvent& viewevent )
{
    a2dView* view = wxStaticCast( viewevent.GetEventObject(), a2dView );
    a2dDrawingPart* canview = wxDynamicCast( view, a2dDrawingPart );
    viewevent.Skip();

    if ( ! canview )
        return;

    if ( m_showObject != canview->GetDrawingPart()->GetShowObject() )
        Init( canview->GetDrawingPart()->GetShowObject() );
}


void IdentifyDialog::OnChangedDocument( a2dCommandEvent& event )
{
    Init( NULL );
}
*/

// Size the subwindows when the frame is resized
void IdentifyDialog::OnSize( wxSizeEvent& WXUNUSED( event ) )
{
    Layout();
}

void IdentifyDialog::OnDoEvent( a2dCommandProcessorEvent& event )
{
    wxString cmdName = event.GetCommand()->GetName();

    if ( event.GetCommand()->GetCommandId() == &a2dCommand_SetShowObject::Id )
    {
        a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawingPart )
            return;

        m_showObject = drawingPart->GetShowObject();
        Init( m_showObject );
    }
}

void IdentifyDialog::OnComEvent( a2dComEvent& event )
{
    if ( event.GetId() == a2dDrawingPart::sig_changedShowObject )
    {
        // an existing corridor on a view will become invalid when the ShowObject changes.
        // Most tools can handle a change in corridor, for those there is no need to remove them after a change
        // in ShowObject.
        a2dCanvasObject* newtop = wxStaticCast( event.GetProperty()->GetRefObject(), a2dCanvasObject );
        if ( newtop )
        {
            m_showObject = newtop;
            Init( m_showObject );
        }
    }
    else if ( event.GetId() == a2dCanvasGlobal::sig_changedActiveDrawing )
    {
		a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();
		if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
		{
            m_showObject = part->GetShowObject();
            Init( m_showObject );
		}
    }
    else
        event.Skip();
}

