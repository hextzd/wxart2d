/*! \file docview/src/docmdiref.cpp
    \brief Frame classes for MDI document/view applications
    \author Julian Smart
    \author Klaas Holwerda
    \date Created 01/02/97

    Copyright: (c) Julian Smart

    Licence: wxWidgets licence

    RCS-ID: $Id: docmdiref.cpp,v 1.44 2009/09/29 20:06:47 titato Exp $
*/

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if wxUSE_MDI_ARCHITECTURE && wxUSE_DOC_VIEW_ARCHITECTURE

#ifndef WX_PRECOMP
#include "wx/string.h"
#include "wx/utils.h"
#include "wx/app.h"
#include "wx/dc.h"
#include "wx/dialog.h"
#include "wx/menu.h"
#include "wx/list.h"
#include "wx/filedlg.h"
#include "wx/intl.h"
#include "wx/file.h"
#endif

#include "wx/docview/doccom.h"
#include "wx/docview/docmdiref.h"

// ----------------------------------------------------------------------------
// template instantiations
// ----------------------------------------------------------------------------

//#include "wx/general/id.inl"

//----------------------------------------------------------------------------
// template instanitiations
//----------------------------------------------------------------------------

/*
 * Docview MDI parent frame
 */

IMPLEMENT_CLASS( a2dDocumentMDIParentFrame, wxMDIParentFrame )

BEGIN_EVENT_TABLE( a2dDocumentMDIParentFrame, wxMDIParentFrame )
    EVT_MENU( wxID_EXIT, a2dDocumentMDIParentFrame::OnExit )
    EVT_MENU_RANGE( wxID_FILE1, wxID_FILE9, a2dDocumentMDIParentFrame::OnMRUFile )
    EVT_CLOSE( a2dDocumentMDIParentFrame::OnCloseWindow )
END_EVENT_TABLE()

a2dDocumentMDIParentFrame::a2dDocumentMDIParentFrame( wxFrame* frame, wxWindowID id, const wxString& title,
        const wxPoint& pos, const wxSize& size, long style, const wxString& name ):
    wxMDIParentFrame( frame, id, title, pos, size, style, name )
{
}

a2dDocumentMDIParentFrame::a2dDocumentMDIParentFrame()
{
}

bool a2dDocumentMDIParentFrame::Create(
    wxWindow* frame,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long  style,
    const wxString& name )
{
    bool res = wxMDIParentFrame::Create( frame, id, title, pos, size, style, name );
    return res;
}


void a2dDocumentMDIParentFrame::OnExit( wxCommandEvent& WXUNUSED( event ) )
{
    Close();
}

void a2dDocumentMDIParentFrame::OnMRUFile( wxCommandEvent& event )
{
    int n = event.GetId() - wxID_FILE1;  // the index in MRU list
    a2dFileHistoryItem* fileHistItem = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetHistoryFileItem( n );
    if ( fileHistItem )
    {
        // verify that the file exists before doing anything else
        if ( wxFile::Exists( fileHistItem->m_filename.GetFullPath() ) )
        {
            // try to open it
            a2dError returncode = a2dDocviewGlobals->GetDocviewCommandProcessor()->CreateDocuments( fileHistItem->m_filename.GetFullPath(), a2dREFDOC_SILENT, fileHistItem->m_docTemplate );
            if ( returncode == a2dError_NoDocTemplateRef )
            {
                a2dDocviewGlobals->ReportError( a2dError_CouldNotCreateDocument, _( "No templates to create document, removed from history, a2dDocumentCommandProcessor::OnMRUFile" ) );
                a2dDocviewGlobals->GetDocviewCommandProcessor()->RemoveFileFromHistory( n );
            }
        }
        else
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_FileHistory,  _( "The file '%s' doesn't exist and couldn't be opened.\nIt has been removed from the most recently used files list." ),
                                             fileHistItem->m_filename.GetFullPath().c_str() );

            // remove the bogus filename from the MRU list and notify the user
            // about it
            a2dDocviewGlobals->GetDocviewCommandProcessor()->RemoveFileFromHistory( n );
        }
    }
}

bool a2dDocumentMDIParentFrame::ProcessEvent( wxEvent& event )
{
    if ( !wxEvtHandler::ProcessEvent( event ) )
    {
        if ( a2dDocviewGlobals->GetDocviewCommandProcessor() ) //&& event.IsKindOf(CLASSINFO(wxCommandEvent)) )
            return a2dDocviewGlobals->GetDocviewCommandProcessor()->ProcessEvent( event );
        return false;
    }
    return true;
}

void a2dDocumentMDIParentFrame::OnCloseWindow( wxCloseEvent& event )
{
    if ( a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear( !event.CanVeto() ) )
    {
        this->Destroy();
    }
    else
        event.Veto();
}

void a2dDocumentMDIParentFrame::OnCmdMenuId( wxCommandEvent& event )
{
    event.Skip();
}

void a2dDocumentMDIParentFrame::AddCmdMenu( wxMenu* parentMenu, wxMenuItem* item )
{
    Connect( item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentMDIParentFrame::OnCmdMenuId ) );
    parentMenu->Append( item );
}

void a2dDocumentMDIParentFrame::AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
{
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentMDIParentFrame::OnCmdMenuId ) );
    parentMenu->Append( cmdId.GetId(), cmdId.GetLabel(), cmdId.GetHelp(), cmdId.IsCheckable() );
}

void a2dDocumentMDIParentFrame::RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
{
    Disconnect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentMDIParentFrame::OnCmdMenuId ) );
    parentMenu->Delete( cmdId.GetId() );
}

void a2dDocumentMDIParentFrame::AddCmdToToolbar( const a2dMenuIdItem& cmdId )
{
    if ( ! GetToolBar() )
        return;
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentMDIParentFrame::OnCmdMenuId ) );
    wxString error = _T( "No Bitmap for a2dToolCmd found for:" ) + cmdId.GetIdName();
    wxASSERT_MSG( cmdId.GetBitmap().Ok(), error );
    GetToolBar()->AddTool( cmdId.GetId(), cmdId.GetLabel(), cmdId.GetBitmap(), cmdId.GetHelp(), cmdId.GetKind() );
}

/********************************************************************
* a2dDocumentMDIChildFrame
*********************************************************************/

IMPLEMENT_CLASS( a2dDocumentMDIChildFrame, wxMDIChildFrame )

BEGIN_EVENT_TABLE( a2dDocumentMDIChildFrame, wxMDIChildFrame )
    EVT_ACTIVATE( a2dDocumentMDIChildFrame::OnActivate )
    EVT_CLOSE( a2dDocumentMDIChildFrame::OnCloseWindow )
    EVT_PAINT( a2dDocumentMDIChildFrame::OnPaint )
    EVT_ACTIVATE_VIEW_SENT_FROM_CHILD( a2dDocumentMDIChildFrame::OnActivateViewSentFromChild )
    EVT_CLOSE_VIEW( a2dDocumentMDIChildFrame::OnCloseView )
END_EVENT_TABLE()


a2dDocumentMDIChildFrame::a2dDocumentMDIChildFrame()
{
    m_destroyOnCloseView = true;
    m_view = NULL;
}


a2dDocumentMDIChildFrame::a2dDocumentMDIChildFrame( wxMDIParentFrame* frame,  a2dView* view, wxWindowID  id,
        const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name ):
    wxMDIChildFrame( frame, id, title, pos, size, style, name )
{
    m_destroyOnCloseView = true;
    m_view = view;
    if ( m_view )
    {
        m_view->SetDisplayWindow( this );
    }
}

bool a2dDocumentMDIChildFrame::Create(
    wxMDIParentFrame* frame,
    a2dView* view,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long  style,
    const wxString& name )
{
    bool res = wxMDIChildFrame::Create( frame, id, title, pos, size, style, name );
    m_view = view;
    if ( m_view )
    {
        m_view->SetDisplayWindow( this );
    }
    return res;
}

a2dDocumentMDIChildFrame::~a2dDocumentMDIChildFrame( void )
{
    m_view = ( a2dView* ) NULL;
}

void a2dDocumentMDIChildFrame::OnCmdMenuId( wxCommandEvent& event )
{
    event.Skip();
}

void a2dDocumentMDIChildFrame::AddCmdMenu( wxMenu* parentMenu, wxMenuItem* item )
{
    Connect( item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentMDIChildFrame::OnCmdMenuId ) );
    parentMenu->Append( item );
}

void a2dDocumentMDIChildFrame::AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
{
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentMDIChildFrame::OnCmdMenuId ) );
    parentMenu->Append( cmdId.GetId(), cmdId.GetLabel(), cmdId.GetHelp(), cmdId.IsCheckable() );
}

void a2dDocumentMDIChildFrame::RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
{
    Disconnect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentMDIChildFrame::OnCmdMenuId ) );
    parentMenu->Delete( cmdId.GetId() );
}

void a2dDocumentMDIChildFrame::AddCmdToToolbar( const a2dMenuIdItem& cmdId )
{
    if ( ! GetToolBar() )
        return;
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentMDIChildFrame::OnCmdMenuId ) );
    wxString error = _T( "No Bitmap for a2dToolCmd found for:" ) + cmdId.GetIdName();
    wxASSERT_MSG( cmdId.GetBitmap( false ).Ok(), error );
    GetToolBar()->AddTool( cmdId.GetId(), cmdId.GetLabel(), cmdId.GetBitmap( false ), cmdId.GetHelp(), cmdId.GetKind() );
}

void a2dDocumentMDIChildFrame::SetView( a2dView* view )
{
    if ( m_view )
    {
        m_view->SetDisplayWindow( NULL );
    }

    m_view = view;

    if ( m_view )
    {
        //Attach the canvas as the window for the view to display its stuff
        m_view->SetDisplayWindow( this );
    }
}

// Extend event processing to search the view's event table
bool a2dDocumentMDIChildFrame::ProcessEvent( wxEvent& event )
{
    static wxEvent* ActiveEvent = NULL;

    // Break recursion loops
    if ( ActiveEvent == &event )
        return false;

    ActiveEvent = &event;

    bool ret = false;
    if ( event.GetEventType() == wxEVT_CLOSE_WINDOW )
    {
        //window close events should lead to view close events.
        //
        //When a child frame is being closed, only the child view will be closed and the frame flaged
        //for Destroy ( see OnCloseView )

        wxCloseEvent& closeevent = ( wxCloseEvent& ) event;

        //give the Frame the right to veto in for instance a OnCloseWindow which is called by wxEVT_CLOSE
        if ( wxEvtHandler::ProcessEvent( event ) && !closeevent.GetVeto() )
        {
            closeevent.SetCanVeto( false );
            if ( m_view )
                m_view->ProcessEvent( closeevent );
            m_view = NULL;
        }
        else
            closeevent.Veto();
        ret = true;
    }
    else if ( event.GetEventType() == wxEVT_ACTIVATE )
    {
        // the wxEVT_ACTIVATE event should lead to a wxEVT_ACTIVATE_VIEW event here.
        // This last event is always sent to a2dDocviewGlobals->GetDocviewCommandProcessor()() from
        // within a2dView::Activate()

        if (  m_view && !m_view->IsClosed() )
            m_view->ProcessEvent( event );

        ret = wxEvtHandler::ProcessEvent( event );
    }
    else
    {
        // EVENTS needed for view directly, should be handled above.
        // events down here go only to the parent window or commandprocessor

        //In case the event is a commandevent, it will automatically be redirected to the parent window by
        // wxEvtHandler::ProcessEvent
        //There is no need to handle that case in here.
        if ( !wxEvtHandler::ProcessEvent( event ) )
        {
            // Try the document manager, then do default processing
            if ( a2dDocviewGlobals->GetDocviewCommandProcessor() )
            {
                ret = a2dDocviewGlobals->GetDocviewCommandProcessor()->ProcessEvent( event );
            }
            else
                ret = false;
        }
        else
            ret = true;
    }
    ActiveEvent = NULL;
    return ret;
}

void a2dDocumentMDIChildFrame::OnActivate( wxActivateEvent& event )
{
    wxMDIChildFrame::OnActivate( event );

    if ( event.GetActive() && m_view )
        m_view->Activate( event.GetActive() );
}

void a2dDocumentMDIChildFrame::OnActivateViewSentFromChild( a2dViewEvent& viewevent )
{
    if ( viewevent.GetActive() )
    {
        m_view = ( a2dView* ) viewevent.GetEventObject();
    }
}

// Clean up frames/windows used for displaying the view.
// They or not really deleted here only flagged for deletion using Destroy(),
// this means that they will really be deleted in idle time by wxWindows.
void a2dDocumentMDIChildFrame::OnCloseView( a2dCloseViewEvent& event )
{
    wxASSERT_MSG( m_view || event.GetEventObject(), wxT( "not view of frame closed" ) );

    //there is no need to handle the parent frame differently

    //frames can be destroyed savely ( happening in idle time )
    //This default, asumes a multi frame application, where each view has one frame
    if ( m_destroyOnCloseView )
        Destroy(); //very likely done somewhere else, but makes sure it happens.

    if ( m_view )
        m_view->SetDisplayWindow( NULL );
    m_view = NULL;

    Refresh();
}

void a2dDocumentMDIChildFrame::OnCloseWindow( wxCloseEvent& event )
{
    if ( !m_view )
        Destroy();

    // Close view but don't delete the frame while doing so!
    // ...since it will be deleted by wxWindows

    if ( event.CanVeto() )
    {
        if ( !m_view || m_view->Close( !event.CanVeto() ) )
            Destroy();
        else
            event.Veto( true );

    }
    else
        Destroy();
}

bool a2dDocumentMDIChildFrame::Destroy()
{
    m_view = ( a2dView* )NULL;
    return wxMDIChildFrame::Destroy();
}

void a2dDocumentMDIChildFrame::OnPaint( wxPaintEvent& WXUNUSED( event ) )
{
    wxPaintDC dc( this );
    PrepareDC( dc );

    if ( m_view && m_view->GetDisplayWindow() == this )
        OnDraw( dc );
}

// Define the repainting behaviour
void a2dDocumentMDIChildFrame::OnDraw( wxDC& dc )
{
    if ( m_view && m_view->GetDisplayWindow() == this )
        m_view->OnDraw( & dc );
}

#endif
// wxUSE_DOC_VIEW_ARCHITECTURE

