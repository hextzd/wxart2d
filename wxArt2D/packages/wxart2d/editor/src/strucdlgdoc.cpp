/*! \file editor/src/strucdlg.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: strucdlg.cpp,v 1.28 2009/10/05 20:03:12 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "wx/editor/strucdlgdoc.h"
#include "wx/canvas/canmod.h"
#include "wx/editor/candoc.h"
#include "wx/editor/cancom.h"

#define STRUCT_HSIZE        220
#define STRUCT_VSIZE        400

const int    GROUP_BUTTON_OK     = wxID_HIGHEST + 5401 ;
const int    GROUP_BUTTON_CANCEL = wxID_HIGHEST + 5402 ;
const int    GROUP_BUTTON_SHOW   = wxID_HIGHEST + 5403 ;
const int    GROUP_UPDATE_DIALOG = wxID_HIGHEST + 5404 ;
const int    GROUP_BUTTON_APPLY  = wxID_HIGHEST + 5405 ;
const int    GROUP_DCLICK  = wxID_HIGHEST + 5406 ;


BEGIN_EVENT_TABLE( a2dCanvasObjectsDocDialog, wxDialog )
    EVT_CLOSE( a2dCanvasObjectsDocDialog::OnCloseWindow )
    EVT_LISTBOX( STRUCT_ID_LISTBOX, a2dCanvasObjectsDocDialog::OnStructIdListboxSelected )
    EVT_LISTBOX_DCLICK( STRUCT_ID_LISTBOX, a2dCanvasObjectsDocDialog::OnStructIdListboxDoubleClicked )
    EVT_BUTTON( STRUCT_ID_HIDE, a2dCanvasObjectsDocDialog::OnStructIdHideClick )
    EVT_BUTTON( STRUCT_ID_APPLY, a2dCanvasObjectsDocDialog::OnStructIdApplyClick )
    EVT_CHANGED_DOCUMENT( a2dCanvasObjectsDocDialog::OnChangedDocument )
    EVT_DO( a2dCanvasObjectsDocDialog::OnDoEvent )
    EVT_COM_EVENT( a2dCanvasObjectsDocDialog::OnComEvent )
END_EVENT_TABLE()


a2dCanvasObjectsDocDialog::a2dCanvasObjectsDocDialog( wxWindow* parent, a2dCanvasDocument* document, bool structOnly, bool modal, long style,
        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size )
{
    m_modal = modal;
    Create( parent, id, caption, pos, size, style );

    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );

    m_document = document;

    m_structureSet.clear();

    Init( document );

    Centre( wxBOTH );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

a2dCanvasObjectsDocDialog::a2dCanvasObjectsDocDialog( wxWindow* parent, a2dCanvasObjectList* total, bool modal, long style,
        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size )
{
    m_modal = modal;
    Create( parent, id, caption, pos, size, style );

    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );

    m_document = NULL;

    m_structureSet.clear();

    Init( total );

    Centre( wxBOTH );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

bool a2dCanvasObjectsDocDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_object = NULL;

    m_listbox = NULL;
    m_hide = NULL;
    m_Apply = NULL;
    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    a2dCanvasObjectsDocDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    m_numberObjects = new wxStaticText( itemDialog1, STRUCT_ID_STATIC, _( "Number of objects:" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_numberObjects->SetHelpText( _( "Displays number of objects in list" ) );
    if ( ShowToolTips() )
        m_numberObjects->SetToolTip( _( "Displays number of objects in list" ) );
    itemBoxSizer2->Add( m_numberObjects, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer4, 1, wxGROW | wxALL, 1 );

    wxString* m_listboxStrings = NULL;
    m_listbox = new wxListBox( itemDialog1, STRUCT_ID_LISTBOX, wxDefaultPosition, wxDefaultSize, 0, m_listboxStrings, wxLB_SINGLE | wxLB_NEEDED_SB );
    m_listbox->SetHelpText( _( "list of objects to choose from, Double click to choose." ) );
    if ( ShowToolTips() )
        m_listbox->SetToolTip( _( "list of objects to choose from, Double click to choose." ) );
    itemBoxSizer4->Add( m_listbox, 1, wxGROW | wxALL, 1 );

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer6, 0, wxGROW | wxALL, 1 );

    if ( m_modal )
    {
        m_hide = new wxButton( itemDialog1, STRUCT_ID_HIDE, _( "Ok" ), wxDefaultPosition, wxDefaultSize, 0 );
        m_Apply = new wxButton( itemDialog1, STRUCT_ID_APPLY, _( "Cancel" ), wxDefaultPosition, wxDefaultSize, 0 );
    }
    else
    {
        m_hide = new wxButton( itemDialog1, STRUCT_ID_HIDE, _( "Hide" ), wxDefaultPosition, wxDefaultSize, 0 );
        m_Apply = new wxButton( itemDialog1, STRUCT_ID_APPLY, _( "Apply" ), wxDefaultPosition, wxDefaultSize, 0 );
    }

    m_hide->SetHelpText( _( "Hide this dialog" ) );
    if ( ShowToolTips() )
        m_hide->SetToolTip( _( "Hide this dialog" ) );
    itemBoxSizer6->Add( m_hide, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    m_Apply->SetHelpText( _( "Set this object as top in canvas" ) );
    if ( ShowToolTips() )
        m_Apply->SetToolTip( _( "Set this object as top in canvas" ) );
    itemBoxSizer6->Add( m_Apply, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
    Centre();
    return TRUE;
}

a2dCanvasObjectsDocDialog::~a2dCanvasObjectsDocDialog()
{
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( a2dEVT_COM_EVENT, this );
}

void a2dCanvasObjectsDocDialog::Init( a2dCanvasObjectList* objects )
{
    m_listbox->Clear();

    s_a2dCanvasObjectSorter = &NameSorter;
    for( a2dCanvasObjectList::iterator iter = objects->begin(); iter != objects->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( m_structureSet.find( obj ) == m_structureSet.end() )
            m_structureSet.insert( obj );
    }

    for( a2dCanvasObjectsSet::iterator iter = m_structureSet.begin(); iter != m_structureSet.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj == m_document->GetDrawing()->GetRootObject() )
        {
            m_listbox->Append( obj->GetName() + wxT( "=>top" ) );
        }
        else
            m_listbox->Append( obj->GetName() );
    }

    wxString mess = wxT( "" );
    mess.Printf( _( "Number of structures: %ld" ), m_structureSet.size() - 1 );
    m_numberObjects->SetLabel( mess );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

void a2dCanvasObjectsDocDialog::Init( a2dCanvasDocument* document )
{
    s_a2dCanvasObjectSorter = &NameSorter;
    m_structureSet.clear();
    if ( document->GetDrawing()->GetDrawingId() == a2dDrawingId_cameleonrefs() )
    {
        a2dCanvasObjectList::iterator rootchild = document->GetDrawing()->GetRootObject()->GetChildObjectList()->begin();
        while ( rootchild != document->GetDrawing()->GetRootObject()->GetChildObjectList()->end() )
        {
            a2dCameleon* ref = wxDynamicCast( (*rootchild).Get(), a2dCameleon );
            if ( ref )
            {
                a2dLayout* layout = ref->GetAppearance<a2dSymbol>();
                if ( layout )
                {
                    if ( m_structureSet.find( layout ) == m_structureSet.end() )
                        m_structureSet.insert( layout );
                }
            }
            rootchild++;
        }

        m_listbox->Clear();
        for( a2dCanvasObjectsSet::iterator iter = m_structureSet.begin(); iter != m_structureSet.end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj->GetHighLight() )
            {
                m_listbox->Append( obj->GetName() + wxT( "=>top" ) );
            }
            else
                m_listbox->Append( obj->GetName() );
        }
    }
    else
    {
        a2dWalker_CollectCanvasObjectsSet collector;
        collector.SetWithChilds( true );
        collector.SortOn( a2dWalker_CollectCanvasObjectsSet::onName );
        collector.SetSkipNotRenderedInDrawing( true );
        collector.Start( document->GetDrawing()->GetRootObject() );

        m_structureSet = collector.m_found;
        m_listbox->Clear();

        for( a2dCanvasObjectsSet::iterator iter = m_structureSet.begin(); iter != m_structureSet.end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj == m_document->GetDrawing()->GetRootObject() )
            {
                m_listbox->Append( obj->GetName() + wxT( "=>top" ) );
            }
            else
                m_listbox->Append( obj->GetName() );
        }
    }

    wxString mess = wxT( "" );
    mess.Printf( _( "Number of structures: %i" ), m_structureSet.size() - 1 );
    m_numberObjects->SetLabel( mess );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

void a2dCanvasObjectsDocDialog::OnStructIdListboxSelected( wxCommandEvent& event )
{
    event.Skip();
}

void a2dCanvasObjectsDocDialog::OnStructIdHideClick( wxCommandEvent& event )
{
    if ( IsModal() )
    {
        CmApply();
        EndModal( wxID_OK );
    }
    else
    {
        CmApply();
        SetReturnCode( wxID_OK );
        Show( false );
    }
}

void a2dCanvasObjectsDocDialog::OnStructIdApplyClick( wxCommandEvent& event )
{
    if ( IsModal() )
    {
        EndModal( wxID_CANCEL );
        return;
    }
}

void a2dCanvasObjectsDocDialog::OnStructIdListboxDoubleClicked( wxCommandEvent& event )
{
    CmApply();
}

void a2dCanvasObjectsDocDialog::CmApply()
{
    wxString buf;
    buf = m_listbox->GetStringSelection();
    for( a2dCanvasObjectsSet::iterator iter = m_structureSet.begin(); iter != m_structureSet.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
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
    if ( m_document->GetDrawing()->GetDrawingId() == a2dDrawingId_cameleonrefs() )
    {
        m_object = wxStaticCast( m_object, a2dDiagram )->GetDrawing()->GetRootObject();
    }

    a2dCanvasView* drawer = a2dGetCmdh()->CheckDrawingView();
    a2dCommand_SetShowObject* command = new a2dCommand_SetShowObject( drawer->GetDrawingPart(), a2dCommand_SetShowObject::Args().canvasobject( m_object ) );
    a2dGetCmdh()->Submit( command, false );
    a2dGetCmdh()->GetDrawingCmdProcessor()->ZoomOut();
}

a2dCanvasObject* a2dCanvasObjectsDocDialog::GetCanvasObject()
{
    return m_object;
}

void a2dCanvasObjectsDocDialog::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    if ( IsModal() )
        EndModal( wxID_OK );
    else
    {
        SetReturnCode( wxID_OK );
        Show( false );
    }
}

void a2dCanvasObjectsDocDialog::OnChangedDocument( a2dCommandEvent& event )
{
    if ( !m_document )
        return;

    a2dCanvasDocument* doc = ( a2dCanvasDocument* ) event.GetDocument();
    if ( !IsModal() && doc )
    {
        m_document = doc;
        Init( doc );
    }
}

void a2dCanvasObjectsDocDialog::OnComEvent( a2dComEvent& event )
{
    if ( !m_document )
        return;

    if ( event.GetId() == a2dDrawingPart::sig_changedShowObject )
    {
        a2dCanvasObject* newtop = wxStaticCast( event.GetProperty()->GetRefObject(), a2dCanvasObject );
    }
    else
        event.Skip();
}

void a2dCanvasObjectsDocDialog::OnDoEvent( a2dCommandProcessorEvent& event )
{
    if ( !m_document )
        return;

    if ( event.GetCommand()->GetCommandId() == &a2dCommand_SetShowObject::Id )
    {
        Init( m_document );
    }
    else if ( event.GetCommand()->GetCommandId() == &a2dCommand_NewGroup::Id )
    {
        Init( m_document );
    }
    else
        event.Skip();
}




