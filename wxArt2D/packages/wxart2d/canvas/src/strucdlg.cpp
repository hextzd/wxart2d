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


#include "wx/canvas/strucdlg.h"
#include "wx/canvas/canmod.h"

BEGIN_EVENT_TABLE( a2dCanvasObjectsDialog, wxDialog )
    EVT_CLOSE( a2dCanvasObjectsDialog::OnCloseWindow )
    EVT_LISTBOX( STRUCT_ID_LISTBOX, a2dCanvasObjectsDialog::OnStructIdListboxSelected )
    EVT_LISTBOX_DCLICK( STRUCT_ID_LISTBOX, a2dCanvasObjectsDialog::OnStructIdListboxDoubleClicked )
    EVT_BUTTON( STRUCT_ID_HIDE, a2dCanvasObjectsDialog::OnStructIdHideClick )
    EVT_BUTTON( STRUCT_ID_APPLY, a2dCanvasObjectsDialog::OnStructIdApplyClick )
    //EVT_CHANGED_DOCUMENT( a2dCanvasObjectsDialog::OnChangedDocument )
    EVT_DO( a2dCanvasObjectsDialog::OnDoEvent )
    EVT_COM_EVENT( a2dCanvasObjectsDialog::OnComEvent )
END_EVENT_TABLE()


a2dCanvasObjectsDialog::a2dCanvasObjectsDialog( wxWindow* parent, a2dDrawing* drawing, bool structOnly, bool modal, long style,
        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size )
{
    m_modal = modal;
    Create( parent, id, caption, pos, size, style );
/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_REDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );
*/
    m_drawing = drawing;

    m_structureSet.clear();

    Init( drawing );

    Centre( wxBOTH );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

a2dCanvasObjectsDialog::a2dCanvasObjectsDialog( wxWindow* parent, a2dCanvasObjectList* total, bool modal, long style,
        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size )
{
    m_modal = modal;
    Create( parent, id, caption, pos, size, style );
/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_REDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );
*/
    m_drawing = NULL;

    m_structureSet.clear();

    Init( total );

    Centre( wxBOTH );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

bool a2dCanvasObjectsDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_object = NULL;

    m_listbox = NULL;
    m_hide = NULL;
    m_Apply = NULL;
    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    a2dCanvasObjectsDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    m_numberObjects = new wxStaticText( itemDialog1, STRUCT_ID_STATIC, _( "Number of objects:" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_numberObjects->SetHelpText( _( "Displays number of objects in list" ) );
    if ( ShowToolTips() )
        m_numberObjects->SetToolTip( _( "Displays number of objects in list" ) );
    itemBoxSizer2->Add( m_numberObjects, 0, wxALIGN_CENTER_HORIZONTAL | wxALL , 5 );

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

a2dCanvasObjectsDialog::~a2dCanvasObjectsDialog()
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

void a2dCanvasObjectsDialog::Init( a2dCanvasObjectList* objects )
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
        if ( obj == m_drawing->GetRootObject() )
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

void a2dCanvasObjectsDialog::Init( a2dDrawing* drawing )
{
    s_a2dCanvasObjectSorter = &NameSorter;
    m_structureSet.clear();
    if ( drawing->GetDrawingId() == a2dDrawingId_cameleonrefs() )
    {
        a2dCanvasObjectList::iterator rootchild = drawing->GetRootObject()->GetChildObjectList()->begin();
        while ( rootchild != drawing->GetRootObject()->GetChildObjectList()->end() )
        {
            a2dCameleonSymbolicRef* ref = wxDynamicCast( (*rootchild).Get(), a2dCameleonSymbolicRef );
            if ( ref && ref->GetCameleon() )
            {
                a2dLayout* layout = ref->GetCameleon()->GetAppearance<a2dDiagram>();
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
        collector.Start( drawing->GetRootObject() );

        m_structureSet = collector.m_found;
        m_listbox->Clear();

        for( a2dCanvasObjectsSet::iterator iter = m_structureSet.begin(); iter != m_structureSet.end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj == m_drawing->GetRootObject() )
            {
                m_listbox->Append( obj->GetName() + wxT( "=>top" ) );
            }
            else
                m_listbox->Append( obj->GetName() );
        }
    }

    wxString mess = wxT( "" );
    mess.Printf( _( "Number of structures: %ld" ), (long) m_structureSet.size() - 1 );
    m_numberObjects->SetLabel( mess );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

void a2dCanvasObjectsDialog::InitCameleons( a2dDrawing* drawing, const wxString& appearanceClassName )
{
    s_a2dCanvasObjectSorter = &NameSorter;
    m_structureSet.clear();
    a2dCanvasObjectList::iterator rootchild = drawing->GetRootObject()->GetChildObjectList()->begin();
    while ( rootchild != drawing->GetRootObject()->GetChildObjectList()->end() )
    {
        a2dCameleon* ref = wxDynamicCast( (*rootchild).Get(), a2dCameleon );
        if ( ref )
        {
            a2dAppear* appearance = ref->GetAppearanceByClassName( appearanceClassName );
            if ( appearance )
            {
                if ( m_structureSet.find( appearance ) == m_structureSet.end() )
                    m_structureSet.insert( appearance );
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

    wxString mess = wxT( "" );
    mess.Printf( _( "Number of structures: %ld" ), m_structureSet.size() - 1 );
    m_numberObjects->SetLabel( mess );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

void a2dCanvasObjectsDialog::OnStructIdListboxSelected( wxCommandEvent& event )
{
    event.Skip();
}

void a2dCanvasObjectsDialog::OnStructIdHideClick( wxCommandEvent& event )
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

void a2dCanvasObjectsDialog::OnStructIdApplyClick( wxCommandEvent& event )
{
    if ( IsModal() )
    {
        EndModal( wxID_CANCEL );
        return;
    }
}

void a2dCanvasObjectsDialog::OnStructIdListboxDoubleClicked( wxCommandEvent& event )
{
    CmApply();
}

void a2dCanvasObjectsDialog::CmApply()
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
    if ( m_drawing->GetDrawingId() == a2dDrawingId_cameleonrefs() )
    {
        m_object = wxStaticCast( m_object, a2dDiagram )->GetDrawing()->GetRootObject();
    }

    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawingPart )
        return;

    a2dCanvasObject* top = drawingPart->GetShowObject();

    a2dCommand_SetShowObject* command = new a2dCommand_SetShowObject( drawingPart, a2dCommand_SetShowObject::Args().canvasobject( m_object ) );
    drawingPart->GetDrawing()->GetCommandProcessor()->Submit( command, false );
    drawingPart->GetDrawing()->GetCanvasCommandProcessor()->ZoomOut();
}

a2dCanvasObject* a2dCanvasObjectsDialog::GetCanvasObject()
{
    return m_object;
}

void a2dCanvasObjectsDialog::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    if ( IsModal() )
        EndModal( wxID_OK );
    else
    {
        SetReturnCode( wxID_OK );
        Show( false );
    }
}

/*
void a2dCanvasObjectsDialog::OnChangedDocument( a2dCommandEvent& event )
{
    if ( !m_drawing )
        return;

    a2dCanvasDocument* doc = ( a2dCanvasDocument* ) event.GetDocument();
    if ( !IsModal() && doc )
    {
        m_drawing = doc;
        Init( doc );
    }
}
*/

void a2dCanvasObjectsDialog::OnComEvent( a2dComEvent& event )
{
    if ( !m_drawing )
        return;

    if ( event.GetId() == a2dDrawingPart::sig_changedShowObject )
    {
        a2dCanvasObject* newtop = wxStaticCast( event.GetProperty()->GetRefObject(), a2dCanvasObject );
    }
    else
        event.Skip();
}

void a2dCanvasObjectsDialog::OnDoEvent( a2dCommandProcessorEvent& event )
{
    if ( !m_drawing )
        return;

    if ( event.GetCommand()->GetCommandId() == &a2dCommand_SetShowObject::Id )
    {
        Init( m_drawing );
    }
    else if ( event.GetCommand()->GetCommandId() == &a2dCommand_NewGroup::Id )
    {
        Init( m_drawing );
    }
    else
        event.Skip();
}

//----------------------------------------------------------------------------
// a2dCanvasObjectsChooseDialog
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dCanvasObjectsChooseDialog, wxDialog )
    EVT_CLOSE( a2dCanvasObjectsChooseDialog::OnCloseWindow )
    EVT_LISTBOX( CHOOSE_ID_LISTBOX, a2dCanvasObjectsChooseDialog::OnStructIdListboxSelected )
    EVT_LISTBOX_DCLICK( CHOOSE_ID_LISTBOX, a2dCanvasObjectsChooseDialog::OnStructIdListboxDoubleClicked )
    EVT_BUTTON( CHOOSE_ID_HIDE, a2dCanvasObjectsChooseDialog::OnStructIdHideClick )
    EVT_BUTTON( CHOOSE_ID_APPLY, a2dCanvasObjectsChooseDialog::OnStructIdApplyClick )
END_EVENT_TABLE()


a2dCanvasObjectsChooseDialog::a2dCanvasObjectsChooseDialog( wxWindow* parent, a2dDrawing* drawing, bool structOnly, long style,
        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size )
{
    Create( parent, id, caption, pos, size, style );
    m_drawing = drawing;

    m_structureSet.clear();

    Init( drawing );

    Centre( wxBOTH );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

a2dCanvasObjectsChooseDialog::a2dCanvasObjectsChooseDialog( wxWindow* parent, a2dCanvasObjectList* total, const wxString& appearancename, long style,
        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size )
{
    Create( parent, id, caption, pos, size, style );
    m_drawing = NULL;

    m_structureSet.clear();

    InitCameleons( total, appearancename );

    Centre( wxBOTH );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

a2dCanvasObjectsChooseDialog::a2dCanvasObjectsChooseDialog( wxWindow* parent, a2dCanvasObjectList* total, long style,
        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size )
{
    Create( parent, id, caption, pos, size, style );
    m_drawing = NULL;

    m_structureSet.clear();

    Init( total );

    Centre( wxBOTH );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

bool a2dCanvasObjectsChooseDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_object = NULL;

    m_listbox = NULL;
    m_hide = NULL;
    m_Apply = NULL;
    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    a2dCanvasObjectsChooseDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    m_numberObjects = new wxStaticText( itemDialog1, CHOOSE_ID_STATIC, _( "Number of objects:" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_numberObjects->SetHelpText( _( "Displays number of objects in list" ) );
    if ( ShowToolTips() )
        m_numberObjects->SetToolTip( _( "Displays number of objects in list" ) );
    itemBoxSizer2->Add( m_numberObjects, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer4, 1, wxGROW | wxALL, 1 );

    wxString* m_listboxStrings = NULL;
    m_listbox = new wxListBox( itemDialog1, CHOOSE_ID_LISTBOX, wxDefaultPosition, wxDefaultSize, 0, m_listboxStrings, wxLB_SINGLE | wxLB_NEEDED_SB );
    m_listbox->SetHelpText( _( "list of objects to choose from, Double click to choose." ) );
    if ( ShowToolTips() )
        m_listbox->SetToolTip( _( "list of objects to choose from, Double click to choose." ) );
    itemBoxSizer4->Add( m_listbox, 1, wxGROW | wxALL, 1 );

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer6, 0, wxGROW | wxALL, 1 );

    m_hide = new wxButton( itemDialog1, CHOOSE_ID_HIDE, _( "Ok" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_Apply = new wxButton( itemDialog1, CHOOSE_ID_APPLY, _( "Cancel" ), wxDefaultPosition, wxDefaultSize, 0 );

    m_hide->SetHelpText( _( "Hide this dialog" ) );
    if ( ShowToolTips() )
        m_hide->SetToolTip( _( "Hide this dialog" ) );
    itemBoxSizer6->Add( m_hide, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    m_Apply->SetHelpText( _( "Set this object as top in canvas" ) );
    if ( ShowToolTips() )
        m_Apply->SetToolTip( _( "Set this object as top in canvas" ) );
    itemBoxSizer6->Add( m_Apply, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    //GetSizer()->Fit( this );
    //GetSizer()->SetSizeHints( this );
    Centre();
    return TRUE;
}

a2dCanvasObjectsChooseDialog::~a2dCanvasObjectsChooseDialog()
{
}

void a2dCanvasObjectsChooseDialog::InitCameleons( a2dCanvasObjectList* total, const wxString& appearanceClassName )
{
    s_a2dCanvasObjectSorter = &NameSorter;
    m_structureSet.clear();
    a2dCanvasObjectList::iterator rootchild = total->begin();
    while ( rootchild != total->end() )
    {
        a2dCameleon* ref = wxDynamicCast( (*rootchild).Get(), a2dCameleon );
        if ( ref )
        {
            if ( appearanceClassName.IsEmpty() )
            {
                if ( m_structureSet.find( ref ) == m_structureSet.end() )
                    m_structureSet.insert( ref );
            }
            else
            {
                a2dAppear* appearance = ref->GetAppearanceByClassName( appearanceClassName );
                if ( appearance )
                {
                    if ( m_structureSet.find( appearance ) == m_structureSet.end() )
                        m_structureSet.insert( appearance );
                }
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

    wxString mess = wxT( "" );
    mess.Printf( _( "Number of structures: %ld" ), (long) (m_structureSet.size() - 1) );
    m_numberObjects->SetLabel( mess );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

void a2dCanvasObjectsChooseDialog::Init( a2dCanvasObjectList* objects )
{
    m_listbox->Clear();

    s_a2dCanvasObjectSorter = &NameSorter;
    for( a2dCanvasObjectList::iterator iter = objects->begin(); iter != objects->end(); ++iter )
    {
        a2dCanvasObjectList::value_type obj = *iter;
		a2dCanvasObjectsSet::iterator iterf = m_structureSet.find( obj );
        if ( iterf == m_structureSet.end() )
            m_structureSet.insert( obj );
    }

    for( a2dCanvasObjectsSet::iterator iter = m_structureSet.begin(); iter != m_structureSet.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( m_drawing && obj == m_drawing->GetRootObject() )
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

void a2dCanvasObjectsChooseDialog::Init( a2dDrawing* drawing )
{
    s_a2dCanvasObjectSorter = &NameSorter;
    m_structureSet.clear();
    if ( drawing->GetDrawingId() == a2dDrawingId_cameleonrefs() )
    {
        a2dCanvasObjectList::iterator rootchild = drawing->GetRootObject()->GetChildObjectList()->begin();
        while ( rootchild != drawing->GetRootObject()->GetChildObjectList()->end() )
        {
            a2dCameleon* ref = wxDynamicCast( (*rootchild).Get(), a2dCameleon );
            if ( ref )
            {
                a2dLayout* layout = ref->GetAppearance<a2dDiagram>();
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
        collector.Start( drawing->GetRootObject() );

        m_structureSet = collector.m_found;
        m_listbox->Clear();

        for( a2dCanvasObjectsSet::iterator iter = m_structureSet.begin(); iter != m_structureSet.end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj == m_drawing->GetRootObject() )
            {
                m_listbox->Append( obj->GetName() + wxT( "=>top" ) );
            }
            else
                m_listbox->Append( obj->GetName() );
        }
    }

    wxString mess = wxT( "" );
    mess.Printf( _( "Number of structures: %ld" ), m_structureSet.size() - 1 );
    m_numberObjects->SetLabel( mess );

    // set the first structure as default and give it the focus
    if ( m_structureSet.size() )
        m_listbox->SetSelection( 0, true );
    m_object = NULL;
}

void a2dCanvasObjectsChooseDialog::OnStructIdListboxSelected( wxCommandEvent& event )
{
    event.Skip();
}

void a2dCanvasObjectsChooseDialog::OnStructIdHideClick( wxCommandEvent& event )
{
    CmApply();
    EndModal( wxID_OK );
}

void a2dCanvasObjectsChooseDialog::OnStructIdApplyClick( wxCommandEvent& event )
{
    EndModal( wxID_CANCEL );
}

void a2dCanvasObjectsChooseDialog::OnStructIdListboxDoubleClicked( wxCommandEvent& event )
{
    CmApply();
}

void a2dCanvasObjectsChooseDialog::CmApply()
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
}

a2dCanvasObject* a2dCanvasObjectsChooseDialog::GetCanvasObject()
{
    return m_object;
}

void a2dCanvasObjectsChooseDialog::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    if ( IsModal() )
        EndModal( wxID_OK );
    else
    {
        SetReturnCode( wxID_OK );
        Show( false );
    }
}

const int    BRUSH_BUTTON_OK     = wxID_HIGHEST + 5801 ;
const int    BRUSH_BUTTON_CANCEL = wxID_HIGHEST + 5802 ;

BEGIN_EVENT_TABLE( CanvasFillDialog, wxDialog )
    EVT_BUTTON    ( BRUSH_BUTTON_OK,  CanvasFillDialog::CmOk )
    EVT_BUTTON    ( BRUSH_BUTTON_CANCEL ,  CanvasFillDialog::CmCancel )
    EVT_CLOSE     ( CanvasFillDialog::OnCloseWindow )
END_EVENT_TABLE()

CanvasFillDialog::CanvasFillDialog( wxFrame* parent ):
    wxDialog( parent, -1, _T( "choose style" ), wxDefaultPosition, wxDefaultSize, ( wxCAPTION ), _T( "style dialog" ) )
{
    m_parent = parent;

    wxString choices[] =
    {
        _T( "Transparent (no fill)" ),
        _T( "Solid" ),
        _T( "Backward diagonal hatch" ),
        _T( "Cross-diagonal hatch" ),
        _T( "Forward diagonal hatch" ),
        _T( "Cross hatch" ),
        _T( "Horizontal hatch" ),
        _T( "Vertical hatch" ),
        _T( "wxSTIPPLE" ),
        _T( "wxSTIPPLE_MASK_OPAQUE" ),
        _T( "wxSTIPPLE_MASK_OPAQUE_TRANSPARENT" ),
        _T( "wxGRADIENT_FILL_XY_LINEAR" ),
        _T( "wxGRADIENT_FILL_XY_RADIAL" ),
        _T( "wxGRADIENT_FILL_XY_DROP" )
    };

    m_choice   = new wxChoice(  this, -1, wxDefaultPosition, wxDefaultSize, 14, choices );
    m_choice->SetSelection( 1 );
    m_button1  = new wxButton(  this, BRUSH_BUTTON_OK,       _T( "Ok" ), wxDefaultPosition, wxSize( 55, 20 ) );
    m_button2  = new wxButton(  this, BRUSH_BUTTON_CANCEL,   _T( "Cancel" ), wxDefaultPosition, wxSize( 55, 20 ) );

    wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );
    sizer->Add( m_choice, 1,   wxEXPAND | wxALL, 1 );

    wxBoxSizer* sizerb = new wxBoxSizer( wxHORIZONTAL );
    sizerb->Add( m_button1, 0,  wxALL, 1 );
    sizerb->Add( m_button2, 0,  wxALL, 1 );

    sizer->Add( sizerb, 0, wxALIGN_CENTER );

    SetAutoLayout( true );

    sizer->SetSizeHints( this );
    sizer->Fit( this );

    SetSizer( sizer );

    Layout();

}

void CanvasFillDialog::CmCancel( wxCommandEvent& )
{
    EndModal( wxID_CANCEL );
}

void CanvasFillDialog::CmOk( wxCommandEvent& )
{
    switch( m_choice->GetSelection() )
    {
        case 0: m_style=a2dFILL_TRANSPARENT;
            break;
        case 1: m_style=a2dFILL_SOLID;
            break;
        case 2: m_style=a2dFILL_BDIAGONAL_HATCH;
            break;
        case 3: m_style=a2dFILL_CROSSDIAG_HATCH;
            break;
        case 4: m_style=a2dFILL_FDIAGONAL_HATCH;
            break;
        case 5: m_style=a2dFILL_CROSS_HATCH;
            break;
        case 6: m_style=a2dFILL_HORIZONTAL_HATCH;
            break;
        case 7: m_style=a2dFILL_VERTICAL_HATCH;
            break;
        case 8: m_style=a2dFILL_STIPPLE;
            break;
        case 9: m_style=a2dFILL_STIPPLE_MASK_OPAQUE;
            break;
        case 10: m_style=a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT;
            break;
        case 11: m_style=a2dFILL_GRADIENT_FILL_XY_LINEAR;
            break;
        case 12: m_style=a2dFILL_GRADIENT_FILL_XY_RADIAL;
            break;
        case 13: m_style=a2dFILL_GRADIENT_FILL_XY_DROP;
            break;
        default: m_style=a2dFILL_SOLID;
    }
    EndModal( wxID_OK );
}

void CanvasFillDialog::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    EndModal( wxID_CANCEL );
}

const int    PEN_BUTTON_OK     = wxID_HIGHEST + 5901 ;
const int    PEN_BUTTON_CANCEL = wxID_HIGHEST + 5902 ;

BEGIN_EVENT_TABLE( PenDialog, wxDialog )
    EVT_BUTTON    ( PEN_BUTTON_OK,  PenDialog::CmOk )
    EVT_BUTTON    ( PEN_BUTTON_CANCEL ,  PenDialog::CmCancel )
    EVT_CLOSE     ( PenDialog::OnCloseWindow )
END_EVENT_TABLE()

PenDialog::PenDialog( wxFrame* parent ):
    wxDialog( parent, -1, _T( "choose style" ), wxDefaultPosition, wxDefaultSize, ( wxDEFAULT_DIALOG_STYLE ), _T( "style dialog" ) )
{
    m_parent = parent;

    wxString choices[] =
    {
        _T( "Solid style." ),
        _T( "No pen is used." ),
        _T( "Dotted style." ),
        _T( "Long dashed style." ),
        _T( "Short dashed style." ),
        _T( "Dot and dash style." ),
        _T( "Use the stipple bitmap." ),
        _T( "Backward diagonal hatch." ),
        _T( "Cross-diagonal hatch." ),
        _T( "Forward diagonal hatch." ),
        _T( "Cross hatch." ),
        _T( "Horizontal hatch." ),
        _T( "Vertical hatch." )
    };

    m_choice   = new wxChoice(  this, -1, wxDefaultPosition, wxDefaultSize, 13, choices );
    m_choice->SetSelection( 1 );
    m_button1  = new wxButton(  this, PEN_BUTTON_OK,     _T( "Ok" ), wxDefaultPosition, wxSize( 55, 20 ) );
    m_button2  = new wxButton(  this, PEN_BUTTON_CANCEL, _T( "Cancel" ), wxDefaultPosition, wxSize( 55, 20 ) );

    wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );
    sizer->Add( m_choice, 1,   wxEXPAND | wxALL, 1 );

    wxBoxSizer* sizerb = new wxBoxSizer( wxHORIZONTAL );
    sizerb->Add( m_button1, 0,  wxALL, 1 );
    sizerb->Add( m_button2, 0,  wxALL, 1 );

    sizer->Add( sizerb, 0, wxALIGN_CENTER );

    SetAutoLayout( true );

    sizer->SetSizeHints( this );
    sizer->Fit( this );

    SetSizer( sizer );


}

void PenDialog::CmCancel( wxCommandEvent& )
{
    EndModal( wxID_CANCEL );
}

void PenDialog::CmOk( wxCommandEvent& )
{
    switch( m_choice->GetSelection() )
    {
        case 0: m_style=a2dSTROKE_SOLID;
            break;
        case 1: m_style=a2dSTROKE_TRANSPARENT;
            break;
        case 2: m_style=a2dSTROKE_DOT;
            break;
        case 3: m_style=a2dSTROKE_LONG_DASH;
            break;
        case 4: m_style=a2dSTROKE_SHORT_DASH;
            break;
        case 5: m_style=a2dSTROKE_DOT_DASH;
            break;
        case 6: m_style=a2dSTROKE_BDIAGONAL_HATCH;
            break;
        case 7: m_style=a2dSTROKE_CROSSDIAG_HATCH;
            break;
        case 8: m_style=a2dSTROKE_FDIAGONAL_HATCH;
            break;
        case 9: m_style=a2dSTROKE_CROSS_HATCH;
            break;
        case 10: m_style=a2dSTROKE_HORIZONTAL_HATCH;
            break;
        case 11: m_style=a2dSTROKE_VERTICAL_HATCH;
            break;
        default: m_style=a2dSTROKE_SOLID;
    }
    EndModal( wxID_OK );
}

void PenDialog::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    EndModal( wxID_CANCEL );
}


const int    LOGICALFUNCTION_BUTTON_OK     = wxID_HIGHEST + 5801 ;
const int    LOGICALFUNCTION_BUTTON_CANCEL = wxID_HIGHEST + 5802 ;

BEGIN_EVENT_TABLE( LogicalFunction, wxDialog )
    EVT_BUTTON    ( LOGICALFUNCTION_BUTTON_OK,  LogicalFunction::CmOk )
    EVT_BUTTON    ( LOGICALFUNCTION_BUTTON_CANCEL ,  LogicalFunction::CmCancel )
    EVT_CLOSE     ( LogicalFunction::OnCloseWindow )
END_EVENT_TABLE()

LogicalFunction::LogicalFunction( wxFrame* parent ):
    wxDialog( parent, -1, _T( "choose style" ), wxDefaultPosition, wxDefaultSize, ( wxDEFAULT_DIALOG_STYLE ), _T( "style dialog" ) )
{
    m_parent = parent;

    wxString choices[] =
    {
        _T( "wxCLEAR" ),
        _T( "wxXOR" ),
        _T( "wxINVERT" ),
        _T( "wxOR_REVERSE" ),
        _T( "wxAND_REVERSE" ),
        _T( "wxCOPY" ),
        _T( "wxAND" ),
        _T( "wxAND_INVERT" ),
        _T( "wxNO_OP" ),
        _T( "wxNOR" ),
        _T( "wxEQUIV" ),
        _T( "wxSRC_INVERT" ),
        _T( "wxOR_INVERT" ),
        _T( "wxNAND" ),
        _T( "wxOR" ),
        _T( "wxSET" )
    };

    m_choice   = new wxChoice(  this, -1, wxDefaultPosition, wxDefaultSize, 16, choices );
    m_choice->SetSelection( 1 );
    m_button1  = new wxButton(  this, LOGICALFUNCTION_BUTTON_OK,     _T( "Ok" ), wxDefaultPosition, wxSize( 55, 20 ) );
    m_button2  = new wxButton(  this, LOGICALFUNCTION_BUTTON_CANCEL, _T( "Cancel" ), wxDefaultPosition, wxSize( 55, 20 ) );

    wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );
    sizer->Add( m_choice, 1,   wxEXPAND | wxALL, 1 );

    wxBoxSizer* sizerb = new wxBoxSizer( wxHORIZONTAL );
    sizerb->Add( m_button1, 0,  wxALL, 1 );
    sizerb->Add( m_button2, 0,  wxALL, 1 );

    sizer->Add( sizerb, 0, wxALIGN_CENTER );

    SetAutoLayout( true );

    sizer->SetSizeHints( this );
    sizer->Fit( this );

    SetSizer( sizer );


}

void LogicalFunction::CmCancel( wxCommandEvent& )
{
    EndModal( wxID_CANCEL );
}

void LogicalFunction::CmOk( wxCommandEvent& )
{
    m_function = m_choice->GetSelection();
    switch( m_choice->GetSelection() )
    {
        case 0: m_function=wxCLEAR;
            break;
        case 1: m_function=wxXOR;
            break;
        case 2: m_function=wxINVERT;
            break;
        case 3: m_function=wxOR_REVERSE;
            break;
        case 4: m_function=wxAND_REVERSE;
            break;
        case 5: m_function=wxCOPY;
            break;
        case 6: m_function=wxAND;
            break;
        case 7: m_function=wxAND_INVERT;
            break;
        case 8: m_function=wxNO_OP;
            break;
        case 9: m_function=wxNOR;
            break;
        case 10: m_function=wxEQUIV;
            break;
        case 11: m_function=wxSRC_INVERT;
            break;
        case 12: m_function=wxOR_INVERT;
            break;
        case 13: m_function=wxNAND;
            break;
        case 14: m_function=wxOR;
            break;
        case 15: m_function=wxSET;
            break;
        default: m_function=wxCOPY;
    }
    EndModal( wxID_OK );
}

void LogicalFunction::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    EndModal( wxID_CANCEL );
}



#define PAT_COLLUMNS 6
#define PAT_ROWS 6
#define PAT_ROWS_VIS 6

#define PD_PANEL1_X  0
#define PD_PANEL1_Y   0
#define PD_PANEL1_W   (PAT_COLLUMNS*40 + 20)
#define PD_PANEL1_H   (PAT_ROWS_VIS*40 )

#define PD_BUTTON_W 32
#define PD_BUTTON_H 32

BEGIN_EVENT_TABLE( PatternDialog, wxDialog )
    EVT_CUSTOM_RANGE( wxEVT_COMMAND_BUTTON_CLICKED, 800, 800 + NR_PATTERNS, PatternDialog::Cm_P_BitB )

    EVT_CLOSE    ( PatternDialog::OnCloseWindow )
    EVT_ACTIVATE ( PatternDialog::OnActivate )

END_EVENT_TABLE()



// the contructor calls it's parent
PatternDialog::PatternDialog( wxWindow* parent, FillPatterns* fills, const wxString& title, long style, const wxString& name ):
    wxDialog( parent, -1, title, wxPoint( 0, 0 ), wxSize( PD_PANEL1_W, PD_PANEL1_H ), style, name )
{
    m_parent = parent;

    m_choosen = 0;

    m_panel1 = new wxPanel( this, -1, wxPoint( PD_PANEL1_X, PD_PANEL1_Y ), wxSize( PD_PANEL1_W, PD_PANEL1_H ) );
    m_scroll    = new wxScrolledWindow( m_panel1, -1, wxPoint( PD_PANEL1_X, PD_PANEL1_Y ), wxSize( PD_PANEL1_W, PD_PANEL1_H ), wxVSCROLL, _T( "scrolledWindow" ) );
    m_scroll->SetScrollbars( 40, 40, 6, 12 );


    for ( int i = 0; i < NR_PATTERNS; i++ )
    {
        wxMemoryDC dcmemtemp2;
        wxColour white( _T( "WHITE" ) );
        wxColour black( _T( "BLACK" ) );

        wxBitmap* help = new wxBitmap( 32, 32, -1 );

        dcmemtemp2.SelectObject( *help );
        wxBrush brushbitm( *fills->GetPattern( i ) );
        dcmemtemp2.SetBrush( brushbitm );

        //convert monogrome to color bitmap
        //in WinNt any monochrome bitmapbrush is drawn to the dest. dc back and fore ground colors
        dcmemtemp2.SetTextForeground( white );
        dcmemtemp2.SetTextBackground( black );
        dcmemtemp2.DrawRectangle( 0, 0, 32, 32 );
        dcmemtemp2.SelectObject( wxNullBitmap );
        m_patternbut[i]     = new   wxBitmapButton( m_scroll, 800 + i, *help,
                wxPoint( ( ( i % PAT_COLLUMNS ) * 40 ), ( ( i / PAT_ROWS ) * 40 ) ) );
        delete help;
    }
    Fit();
    Center( wxBOTH );
}

// free used memory
PatternDialog::~PatternDialog()
{
}

wxColour* PatternDialog::ltowxc( long colour )
{
    unsigned char   red, green, blue;

    blue    = colour / 65536;
    green = ( colour % 65536 ) / 256;
    red     = colour % 256;

    return new wxColour( red, green, blue );
}

void PatternDialog::Cm_P_BitB( wxEvent& event )
{
    m_choosen = event.GetId() - 800;
    EndModal( m_choosen );
}

void PatternDialog::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
}

void PatternDialog::OnActivate( wxActivateEvent& WXUNUSED( event ) )
{

}

FillPatterns::FillPatterns()
{
    wxString file;
    for ( int i = 0; i < NR_PATTERNS; i++ )
    {
        file.Printf( wxT( "pat%-2.2d.bmp" ), i + 1 );

        //m_fillbitmaps[i] = new wxBitmap(32,32,-1);
        file = a2dGlobals->GetIconPathList().FindValidPath( file );

        wxImage image;

        if ( ! image.LoadFile( file, wxBITMAP_TYPE_BMP ) )
            a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "CVG : invalid bitmap %s" ), file.c_str() );

        m_fillbitmaps[i] = new wxBitmap( image );

        //set mask to monochrome bitmap based on color bitmap

        wxColour white( wxT( "WHITE" ) );
        wxColour black( wxT( "BLACK" ) );

        wxMask* monochrome_mask = new wxMask( *m_fillbitmaps[i], black );
        m_fillbitmaps[i]->SetMask( monochrome_mask );
    }

}

FillPatterns::~FillPatterns()
{
    for ( int i = 0; i < NR_PATTERNS; i++ )
        delete m_fillbitmaps[i];
}

wxBitmap* FillPatterns::GetPattern( short patternnr )
{
    return m_fillbitmaps[patternnr];
}


