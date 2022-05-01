/*! \file docview/src/doccom.cpp
    \brief Document/view classes
    \author Klaas Holwerda
    \date Created 05/07/2003

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: doccom.cpp,v 1.157 2009/09/26 19:01:05 titato Exp $
*/

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

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
#endif


#ifdef __WXGTK__
#include "wx/mdi.h"
#endif

#if wxUSE_PRINTING_ARCHITECTURE
#include "wx/prntbase.h"
#include "wx/printdlg.h"
#endif

#include "wx/msgdlg.h"
#include "wx/choicdlg.h"
#include "wx/confbase.h"
#include "wx/file.h"
#include "wx/cmdproc.h"
#include "wx/log.h"
#include "wx/textdlg.h"

#include <wx/tokenzr.h>
#include <wx/regex.h>

#if wxUSE_PRINTING_ARCHITECTURE
#include <wx/paper.h>
#endif

#include <stdio.h>
#include <string.h>

#include "wx/docview.h"
#include "wx/docview/docviewref.h"
#include "wx/general/gen.h"
#include "wx/docview/doccom.h"

// ----------------------------------------------------------------------------
// template instantiations
// ----------------------------------------------------------------------------

//#include "wx/general/id.inl"

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DDOCVIEWDLLEXP a2dSmrtPtrList< a2dCommandLanguageWrapper >;
#endif

// ============================================================================
// declarations
// ============================================================================

DEFINE_EVENT_TYPE( wxEVT_ADD_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_REMOVE_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_CHANGED_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_CANNOT_OPEN_DOCUMENT )

//! select one or more files
/*!
    \param message message to display
    \param default_path path to go for selecting files
    \param default_filename file to open by default
    \param indexDefaultExtension Returns the index into the list of filters supplied,
            optionally, in the wildcard parameter. Before the dialog is shown,
            this is the index which will be used when the dialog is first displayed.
            After the dialog is shown, this is the index selected by the user.
    \param wildcard display files according to this filter
    \param flags flags for multiple files etc. see wxFileDialog
    \param parent parent window
    \param returnPaths all returned files in case fo multiple files selection
    \param x x display coordinate of dialog
    \param y y display coordinate of dialog
*/
a2dError a2dFileSelectorEx( const wxString& message = wxFileSelectorPromptStr,
                            const wxString& default_path = wxEmptyString,
                            const wxString& default_filename = wxEmptyString,
                            int* indexDefaultExtension = NULL,
                            const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
                            int flags = 0,
                            wxWindow* parent = NULL,
                            wxArrayString* returnPaths = NULL,
                            int x = wxDefaultCoord, int y = wxDefaultCoord )
{
    wxASSERT_MSG( returnPaths, wxT( "must have return path wxArrayString" ) );
    returnPaths->Clear();

    wxFileDialog fileDialog( parent,
                             message,
                             default_path,
                             default_filename,
                             wildcard,
                             flags, wxPoint( x, y ) );

    //set position to the preferred template
    if ( indexDefaultExtension && *indexDefaultExtension > 0 )
        fileDialog.SetFilterIndex( *indexDefaultExtension );

    if ( fileDialog.ShowModal() == wxID_OK )
    {
        if ( indexDefaultExtension )
            *indexDefaultExtension = fileDialog.GetFilterIndex();

        if( flags & wxFD_MULTIPLE )
            fileDialog.GetPaths( *returnPaths );
        else
            returnPaths->Add( fileDialog.GetPath() );
        return a2dError_NoError;
    }
    return a2dError_Canceled;
}


// ----------------------------------------------------------------------------
// wxWindows macros
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dDocumentCommandProcessor, a2dCommandProcessor )

//!@{ \ingroup menus
DEFINE_MENU_ITEMID( CmdMenu_Exit, wxTRANSLATE("&Exit\tAlt-X"), wxTRANSLATE("Exit application" ) )
DEFINE_MENU_ITEMID( CmdMenu_FileClose, wxTRANSLATE("&Close"), wxTRANSLATE("Close current file" ) )
DEFINE_MENU_ITEMID( CmdMenu_FileCloseAll, wxTRANSLATE("&Close All"), wxTRANSLATE("Close all open files" ) )
DEFINE_MENU_ITEMID( CmdMenu_FileOpen, wxTRANSLATE("&Open"), wxTRANSLATE("Open existing file" ) )
DEFINE_MENU_ITEMID( CmdMenu_FileNew, wxTRANSLATE("&New"), wxTRANSLATE("Create new file" ) )
DEFINE_MENU_ITEMID( CmdMenu_FileSave, wxTRANSLATE("Save"), wxTRANSLATE("Save file" ) )
DEFINE_MENU_ITEMID( CmdMenu_FileSaveAll, wxTRANSLATE("&Save All"), wxTRANSLATE("Save all open files" ) )
DEFINE_MENU_ITEMID( CmdMenu_FileSaveAs, wxTRANSLATE("Save &As"), wxTRANSLATE("Save file with different Name" ) )
DEFINE_MENU_ITEMID( CmdMenu_FileImport, wxTRANSLATE("&Import"), wxTRANSLATE("Import file into current document" ) )
DEFINE_MENU_ITEMID( CmdMenu_FileExport, wxTRANSLATE("&Export"), wxTRANSLATE("Export file to formatX" ) )
DEFINE_MENU_ITEMID( CmdMenu_FileRevert, wxTRANSLATE("&Revert"), wxTRANSLATE("reload file" ) )
DEFINE_MENU_ITEMID( CmdMenu_CreateView, wxTRANSLATE("&CreateView"), wxTRANSLATE("create a view on current document" ) )
DEFINE_MENU_ITEMID( CmdMenu_Print, wxTRANSLATE("Print"), wxTRANSLATE("&Print" ) )
DEFINE_MENU_ITEMID( CmdMenu_Preview, wxTRANSLATE("Preview"), wxTRANSLATE("Preview" ) )
DEFINE_MENU_ITEMID( CmdMenu_PrintView, wxTRANSLATE("Print View"), wxTRANSLATE("Print View" ) )
DEFINE_MENU_ITEMID( CmdMenu_PreviewView, wxTRANSLATE("Preview View"), wxTRANSLATE("Preview Print View" ) )
DEFINE_MENU_ITEMID( CmdMenu_PrintDocument, wxTRANSLATE("Print Document"), wxTRANSLATE("Print Document" ) )
DEFINE_MENU_ITEMID( CmdMenu_PreviewDocument, wxTRANSLATE("Preview Document"), wxTRANSLATE("Preview Print Document" ) )
DEFINE_MENU_ITEMID( CmdMenu_PrintSetup, wxTRANSLATE("Print Setup"), wxTRANSLATE("Setup Print" ) )
DEFINE_MENU_ITEMID( CmdMenu_EmptyDocument, wxTRANSLATE("Empty Document"), wxTRANSLATE("Remove contents of document" ) )
//!@}

// ----------------------------------------------------------------------------
// a2dDocumentCommandProcessor
// ----------------------------------------------------------------------------


BEGIN_EVENT_TABLE( a2dDocumentCommandProcessor, a2dCommandProcessor )
    EVT_MENU( CmdMenu_Exit().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_FileClose().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_FileCloseAll().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_FileOpen().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_FileNew().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_FileSave().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_FileSaveAll().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_FileSaveAs().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_FileImport().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_FileExport().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_FileRevert().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_CreateView().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Print().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Preview().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PrintView().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PreviewView().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PrintDocument().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PreviewDocument().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PrintSetup().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_EmptyDocument().GetId(), a2dDocumentCommandProcessor::OnMenu )
    EVT_UPDATE_UI( CmdMenu_FileOpen().GetId(), a2dDocumentCommandProcessor::OnUpdateFileOpen )
    EVT_UPDATE_UI( CmdMenu_FileClose().GetId(), a2dDocumentCommandProcessor::OnUpdateFileClose )
    EVT_UPDATE_UI( CmdMenu_FileCloseAll().GetId(), a2dDocumentCommandProcessor::OnUpdateFileCloseAll )
    EVT_UPDATE_UI( CmdMenu_FileRevert().GetId(), a2dDocumentCommandProcessor::OnUpdateFileRevert )
    EVT_UPDATE_UI( CmdMenu_FileNew().GetId(), a2dDocumentCommandProcessor::OnUpdateFileNew )
    EVT_UPDATE_UI( CmdMenu_FileSave().GetId(), a2dDocumentCommandProcessor::OnUpdateFileSave )
    EVT_UPDATE_UI( CmdMenu_FileSaveAll().GetId(), a2dDocumentCommandProcessor::OnUpdateFileSaveAll )
    EVT_UPDATE_UI( CmdMenu_FileSaveAs().GetId(), a2dDocumentCommandProcessor::OnUpdateFileSaveAs )
    EVT_UPDATE_UI( CmdMenu_CreateView().GetId(), a2dDocumentCommandProcessor::OnUpdateCreateView )
    EVT_UPDATE_UI( wxID_UNDO, a2dDocumentCommandProcessor::OnUpdateUndo )
    EVT_UPDATE_UI( wxID_REDO, a2dDocumentCommandProcessor::OnUpdateRedo )

#if wxUSE_PRINTING_ARCHITECTURE
    EVT_UPDATE_UI( CmdMenu_Print().GetId(), a2dDocumentCommandProcessor::OnUpdatePrint )
    EVT_UPDATE_UI( CmdMenu_Preview().GetId(), a2dDocumentCommandProcessor::OnUpdatePreview )
#endif

    EVT_ACTIVATE_VIEW( a2dDocumentCommandProcessor::OnActivateView )
    EVT_REMOVE_VIEW( a2dDocumentCommandProcessor::OnRemoveView )
    EVT_ADD_VIEW( a2dDocumentCommandProcessor::OnAddView )

    EVT_CANNOT_OPEN_DOCUMENT( a2dDocumentCommandProcessor::OnCannotOpenDocument )

END_EVENT_TABLE()

a2dDocumentCommandProcessor::a2dDocumentCommandProcessor( long flags, bool WXUNUSED( initialize ), int maxCommands )
    : a2dCommandProcessor( maxCommands )
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    , m_initCurrentSmartPointerOwner( this )
#endif
    ,m_preferredImportTemplate( NULL )
    ,m_preferredExportTemplate( NULL )
    ,m_preferredOpenTemplate( NULL )
{
    m_allowOpenSameFile = true;
    m_defaultDocumentNameCounter = 1;
    m_flags = flags;
    m_maxDocsOpen = 10000;
    m_fileHistory = ( a2dFileHistory* ) NULL;
#if wxUSE_PRINTING_ARCHITECTURE
    m_pageSetupData = new wxPageSetupDialogData;
#endif
    Initialize();
    m_currentDocument = NULL;
    m_currentView = ( a2dView* ) NULL;
    m_busyExit = false;
}

a2dDocumentCommandProcessor::a2dDocumentCommandProcessor( a2dDocumentCommandProcessor* other )
    : a2dCommandProcessor( other->m_maxNoCommands )
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    , m_initCurrentSmartPointerOwner( this )
#endif
{
    m_defaultDocumentNameCounter = 1;
    m_flags = other->m_flags;
    m_maxDocsOpen = 10000;

#if wxUSE_PRINTING_ARCHITECTURE
    m_pageSetupData = new wxPageSetupDialogData( *other->m_pageSetupData );
#endif
    a2dCommandProcessor::Initialize();

    m_fileHistory = new a2dFileHistory;
    size_t j;
    for ( j = 0; j < other->m_fileHistory->GetCount(); j++ )
    {
        m_fileHistory->AddFileToHistory( other->GetHistoryFile( j ) );
    }

    m_currentDocument = NULL;
    m_currentView = NULL;

    m_docTemplates = other->m_docTemplates;
    m_viewTemplates = other->m_viewTemplates;
    m_preferredImportTemplate = other->m_preferredImportTemplate;
    m_preferredExportTemplate = other->m_preferredExportTemplate;
    m_preferredOpenTemplate = other->m_preferredOpenTemplate;
    m_allowOpenSameFile = other->m_allowOpenSameFile;

    m_busyExit = false;
}

a2dDocumentCommandProcessor::~a2dDocumentCommandProcessor()
{
    m_busyExit = true;
    Clear();
    if ( m_fileHistory )
        delete m_fileHistory;
#if wxUSE_PRINTING_ARCHITECTURE
    if( m_pageSetupData )
        delete m_pageSetupData;
#endif
}

void a2dDocumentCommandProcessor::OnMenu( wxCommandEvent& event )
{
    if ( event.GetId() == CmdMenu_Exit().GetId() )
    {
        Exit( false );
    }
    else if ( event.GetId() == CmdMenu_FileClose().GetId() )
    {
        FileClose( false );
    }
    else if ( event.GetId() == CmdMenu_FileCloseAll().GetId() )
    {
        CloseDocuments( false );
    }
    else if ( event.GetId() == CmdMenu_FileOpen().GetId() )
    {
        a2dDocumentPtr doc = NULL;
        a2dError result = FileOpen( doc, wxFileName( wxT( "" ) ), a2dTemplateFlag::VISIBLE | a2dTemplateFlag::LOAD );
        if ( !doc )
        {
            if ( result == a2dError_Canceled )
            {
            }
        }
    }
    else if ( event.GetId() == CmdMenu_FileNew().GetId() )
    {
        a2dDocumentPtr doc;
        a2dError result = FileNew( doc );
    }
    else if ( event.GetId() == CmdMenu_FileSave().GetId() )
    {
        FileSave();
    }
    else if ( event.GetId() == CmdMenu_FileSaveAll().GetId() )
    {
        FileSaveAll();
    }
    else if ( event.GetId() == CmdMenu_FileSaveAs().GetId() )
    {
        FileSaveAs();
    }
    else if ( event.GetId() == CmdMenu_FileImport().GetId() )
    {
        FileImport();
    }
    else if ( event.GetId() == CmdMenu_FileExport().GetId() )
    {
        FileExport();
    }
    else if ( event.GetId() == CmdMenu_FileRevert().GetId() )
    {
        FileRevert();
    }
    else if ( event.GetId() == CmdMenu_CreateView().GetId() )
    {
        CreateView( m_currentDocument );
    }
    else if ( event.GetId() == CmdMenu_Print().GetId() )
    {
        Print( a2dPRINT_PrintView );
    }
    else if ( event.GetId() == CmdMenu_PrintView().GetId() )
    {
        Print( a2dPRINT_PrintView );
    }
    else if ( event.GetId() == CmdMenu_PreviewView().GetId() )
    {
        Preview( a2dPRINT_PreviewView );
    }
    else if ( event.GetId() == CmdMenu_PrintDocument().GetId() )
    {
        Print( a2dPRINT_PrintDocument );
    }
    else if ( event.GetId() == CmdMenu_PreviewDocument().GetId() )
    {
        Preview( a2dPRINT_PreviewDocument );
    }
    else if ( event.GetId() == CmdMenu_PrintSetup().GetId() )
    {
        PrintSetup( a2dPRINT_PrintSetup );
    }
    else if ( event.GetId() == CmdMenu_EmptyDocument().GetId() )
    {
        if ( !m_currentDocument )
        {
            a2dDocviewGlobals->SendToLogTarget();
            return;
        }

        m_currentDocument->DeleteContents();

        if ( !m_currentDocument->GetCommandProcessor() )
        {
            a2dDocviewGlobals->SendToLogTarget();
            return;
        }
        m_currentDocument->GetCommandProcessor()->Initialize();
    }
    a2dDocviewGlobals->SendToLogTarget();
}

void a2dDocumentCommandProcessor::OnExit()
{
}

void a2dDocumentCommandProcessor::Initialize()
{
    a2dCommandProcessor::Initialize();

    if ( m_fileHistory )
        delete m_fileHistory;
    m_fileHistory = OnCreateFileHistory();
}

bool a2dDocumentCommandProcessor::SubmitToDocument( a2dCommand* command, bool storeIt )
{
    if ( !m_currentDocument )
        return false;

    if ( !m_currentDocument->GetCommandProcessor() )
        return false;

    return m_currentDocument->GetCommandProcessor()->Submit( command, storeIt );
}

bool a2dDocumentCommandProcessor::CloseDocuments( bool force )
{
    bool all = true;

    a2dDocumentList::iterator iter = m_docs.begin();
    while( iter != m_docs.end() )
    {
        //An extra reference to make absolutely sure that the document is deleted HERE,
        //and not deep down in views or documents.
        //If we don't do this some other call may already release the document from here, while closing
        //and event resulting from that are not yet finished. A typical case is that
        // a2dDocumentCommandProcessor::FileClose() is called by the user after removing view from a document,
        // unaware that it will be released to soon that way.
        a2dDECLARE_LOCAL_ITEM( a2dDocumentList::value_type, doc, *iter );

        iter++;
        if ( doc->Close( force ) || force )
        {
            // in case of non veto in document closing, this already did happen,
            // but if force is true this makes sure it happens.
            doc->DisConnectAllViews();

            // release document from a2dDocumentCommandProcessor
            // This assumes that documents are not connected in
            // any way, i.e. deleting one document does NOT
            // delete another.
            RemoveDocument( doc );

            // normally the views for this document should have bin closed/disconnected already.
            // And therefore m_currentView and m_currentDocument already be different or al least NULL.
            // In case not we leaf this in a save state.
            if ( m_currentDocument && m_currentDocument == doc )
            {
                m_currentDocument = NULL;
                if ( m_currentView && m_currentView->GetDocument() == doc )
                    m_currentView = NULL;
            }
        }
        else
            all = false;

    }
    if ( all )
    {
        m_currentView = NULL;
        m_currentDocument = NULL;
    }
    return all;
}

bool a2dDocumentCommandProcessor::Clear( bool force )
{
    if ( !CloseDocuments( force ) )
        return false;

    m_docTemplates.clear();
    m_viewTemplates.clear();
    return true;
}

bool a2dDocumentCommandProcessor::Exit( bool force )
{
    if ( m_busyExit )
        return true;
    m_busyExit = true;

    if ( !CloseDocuments( force ) )
	{
		m_busyExit = false;
        a2dDocument* doc = GetCurrentDocument();
        if ( !doc )
        {
            a2dDocument* docFirst = *m_docs.begin();
            SetCurrentDocument( docFirst );
        }
        return false;
	}

    OnExit();

    // the next will destroy all child window of view on documents
    if ( Clear( force ) )
    {
        // e.g. toplevel windows, which or not THE toplevel window.

        // now check if the exit is via the windows close mechanism, or a script line.
        // In case of window close, the toplevel window is already marked for destroy.
        wxFrame* pf = ( wxFrame* ) wxTheApp->GetTopWindow();
        if ( wxTopLevelWindows.Find( pf ) && !wxPendingDelete.Member( pf ) )
        {
            pf->Close( true );
        }
        return true;
    }
    return false;
}

a2dFileHistory* a2dDocumentCommandProcessor::OnCreateFileHistory()
{
    return new a2dFileHistory;
}

bool a2dDocumentCommandProcessor::FileClose( bool force )
{
    a2dDocument* doc = GetCurrentDocument();
    if ( !doc )
        return false;

    if ( doc->Close( force ) ) // if not vetod, closes the view(s) of this document
    {
        RemoveDocument( doc );
        return true;
    }
    return false;
}

a2dError a2dDocumentCommandProcessor::FileNew( a2dDocumentPtr& doc, a2dTemplateFlagMask docTemplateFlags )
{
    doc = NULL;
    a2dError result;
    result = CreateDocuments( wxT( "" ), a2dREFDOC_NEW | a2dREFDOC_INIT, NULL, wxFD_OPEN, docTemplateFlags );
    if ( result == a2dError_NoError )
        doc = m_docs.back();
    else if ( result == a2dError_Canceled )
        doc = NULL;
    else
    {
        a2dDocviewGlobals->ReportError( result, _( "could not create document, a2dDocumentCommandProcessor::FileNew" ) );
        doc = NULL;
    }

    return result;
}

a2dError a2dDocumentCommandProcessor::FileOpen( a2dDocumentPtr& doc, const wxFileName& file, a2dTemplateFlagMask docTemplateFlags )
{
    a2dError result ;
    wxString filename  = file.GetFullPath();

    doc = NULL;
    if ( filename.IsEmpty() )
    {
        result = CreateDocuments( wxT( "" ), a2dREFDOC_NON, NULL, wxFD_OPEN, docTemplateFlags );
        if ( result == a2dError_NoError )
            doc = m_docs.back();
        else if ( result == a2dError_Canceled )
            doc = NULL;
        else if ( result == a2dError_FileVersion )
        {
            doc = NULL;            
        }
        else if ( result == a2dError_NoDocTemplateRef )
        {
            a2dDocviewGlobals->ReportError( a2dError_CouldNotCreateDocument, _( "No templates, could not create document, a2dDocumentCommandProcessor::FileOpen" ) );
            doc = NULL;
        }
        else
        {
            a2dDocviewGlobals->ReportError( result, _( "could not create document, a2dDocumentCommandProcessor::FileOpen" ) );
            doc = NULL;
        }
    }
    else
    {
        result = CreateDocuments( filename, a2dREFDOC_SILENT, NULL, wxFD_OPEN, docTemplateFlags );
        if ( result == a2dError_NoError )
            doc = m_docs.back();
        else if ( result == a2dError_Canceled )
            doc = NULL;
        else if ( result == a2dError_FileVersion )
        {
            doc = NULL;            
        }
        else if ( result == a2dError_NoDocTemplateRef )
        {
            a2dDocviewGlobals->ReportError( a2dError_CouldNotCreateDocument, _( "No templates, could not create document, a2dDocumentCommandProcessor::FileOpen" ) );
            doc = NULL;
        }
        else
        {
            a2dDocviewGlobals->ReportError( result, _( "could not create document, a2dDocumentCommandProcessor::FileOpen" ) );
            doc = NULL;
        }
    }

    // returns (a2dDocument*) NULL if creation fails
    return result;
}

a2dError a2dDocumentCommandProcessor::FileOpenCheck( a2dDocumentPtr& doc, const wxFileName& file, bool checkModification )
{
    a2dError result ;

    wxString filename  = file.GetFullPath();
    wxASSERT_MSG( ! filename.IsEmpty(), wxT( "need a filename that is not empty" ) );

    doc = NULL;
    if ( filename.IsEmpty() )
    {
        result = CreateDocuments( wxT( "" ), a2dREFDOC_NON, NULL, wxFD_OPEN, a2dTemplateFlag::VISIBLE | a2dTemplateFlag::LOAD );
        if ( result == a2dError_NoError )
            doc = m_docs.back();
        else if ( result == a2dError_Canceled )
            doc = NULL;
        else if ( result == a2dError_NoDocTemplateRef )
        {
            a2dDocviewGlobals->ReportError( a2dError_CouldNotCreateDocument, _( "No templates, could not create document, a2dDocumentCommandProcessor::FileOpen" ) );
            doc = NULL;
        }
        else
        {
            a2dDocviewGlobals->ReportError( result, _( "could not create document, a2dDocumentCommandProcessor::FileOpen" ) );
            doc = NULL;
        }
    }
    else
    {
        bool alreadyOpen = false;

        a2dDocumentList::iterator iter = m_docs.begin();
        while( iter != m_docs.end() )
        {
            a2dDocumentPtr doc = *iter;
            if ( doc->GetFilename().GetFullPath() == filename )
            {
                alreadyOpen = true;
                break;
            }
            iter++;
        }
        if ( !alreadyOpen )
        {
            return FileOpen( doc, file );
        }
        else
        {
            doc = *iter;

            wxDateTime dtAccess;
            wxDateTime dtMod;
            wxDateTime dtCreate;
            file.GetTimes( &dtAccess, &dtMod, &dtCreate );

            if ( dtMod > doc->GetModificationTime() )
                return FileOpen( doc, file );

            size_t n = 0;

            a2dDocumentTemplate* useDocTemplate = doc->GetDocumentTemplate();
            if ( !useDocTemplate )
            {
                // check is there is only one template visible, meaning we do not have to choose one.
                const_forEachIn( a2dDocumentTemplateList, &m_docTemplates )
                {
                    a2dDocumentTemplateList::value_type temp = *iter;
                    if ( temp->CheckMask( a2dTemplateFlag::VISIBLE ) )
                    {
                        if ( n == 0 )
                            useDocTemplate = temp;
                        n ++;
                    }
                }

                if ( n == 0 )
                {
                    return a2dError_NoDocTemplateRef;
                }
            }
            useDocTemplate->SentPostCreateDocumentEvent( doc, a2dREFDOC_NON );
        }
    }
    return result;
}

a2dError a2dDocumentCommandProcessor::FilesOpen( const wxString& openPath, int dialogFlags, a2dTemplateFlagMask mask )
{
    a2dError result;
    result = CreateDocuments( openPath, a2dREFDOC_NON, NULL, dialogFlags, mask );
    if ( result == a2dError_NoError )
    {
    }
    else if ( result == a2dError_Canceled )
    {
    }
    else
    {
        a2dDocviewGlobals->ReportError( result, _( "could not create document, a2dDocumentCommandProcessor::FilesOpen" ) );
    }

    return result;
}

bool a2dDocumentCommandProcessor::FileRevert()
{
    a2dDocument* doc = GetCurrentDocument();
    if ( !doc )
        return false;
    doc->Revert();
    return true;
}

bool a2dDocumentCommandProcessor::FileSave()
{
    a2dDocument* doc = GetCurrentDocument();
    if ( !doc )
        return false;
    bool result = doc->Save();

	// keep trac of last used directory
	m_lastDirectory = doc->GetFilename().GetPath();
	doc->GetDocumentTemplate()->SetDirectory( m_lastDirectory );
    if ( result )
        AddFileToHistory( doc->GetFilename().GetFullPath() );

    return true;
}

bool a2dDocumentCommandProcessor::FileSaveAll()
{
    a2dDocumentList::iterator iter = m_docs.begin();
    while( iter != m_docs.end() )
    {
        a2dDECLARE_LOCAL_ITEM( a2dDocumentList::value_type, doc, *iter );
        if ( doc && doc->IsModified() )
        {
            bool result = doc->Save();
            if ( result )
                AddFileToHistory( doc->GetFilename().GetFullPath() );
        }
        iter++;
    }
    return true;
}

bool a2dDocumentCommandProcessor::FileSaveAs( const wxFileName& file, a2dDocumentFlagMask flags )
{
    a2dDocument* doc = GetCurrentDocument();
    if ( !doc )
        return false;

    wxString foundfile = file.GetFullPath();
    bool result = doc->SaveAs( foundfile, flags );
	// keep trac of last used directory
	m_lastDirectory = doc->GetFilename().GetPath();
	doc->GetDocumentTemplate()->SetDirectory( m_lastDirectory );
    AddFileToHistory( doc->GetFilename().GetFullPath() );
	return result;
}

bool a2dDocumentCommandProcessor::FileExport( const wxFileName& file, const wxString& description, a2dDocumentFlagMask flags )
{
    a2dDocument* doc = GetCurrentDocument();
    if ( !doc )
        return false;

    // Find the templates for this type of document.
    a2dDocumentTemplate* docTemplate = NULL;
    a2dDocumentTemplate* preferredExportTemplate = NULL;
    size_t n = 0;
    a2dDocumentTemplateList onlyThisDocTemplates;
    const_forEachIn( a2dDocumentTemplateList, &m_docTemplates )
    {
        a2dDocumentTemplateList::value_type temp = *iter;
        if ( temp->CheckMask( a2dTemplateFlag::EXPORTING ) )
        {
            // check on description, or else on file extension.
            if ( ( description.IsEmpty() && !file.GetExt().IsEmpty() && file.GetExt() == temp->GetDefaultExtension() ) ||
                   temp->GetDescription() == description ||
                   file.GetExt().IsEmpty()
               )
            {
                if ( temp->GetDocumentTypeName() == doc->GetDocumentTemplate()->GetDocumentTypeName() &&
                     ( !temp->GetDocumentIOHandlerStrOut() || temp->GetDocumentIOHandlerStrOut()->CanSave( doc ) )
                   ) 
                {
                    onlyThisDocTemplates.push_back( temp );
                    docTemplate = temp;
                    n++;
                    if ( m_preferredExportTemplate == temp )
                        preferredExportTemplate = temp;
                }
            }
        }
    }

    if ( !docTemplate )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_NoDocTemplateRef, _( "Sorry, there is no document template for exporting this document." ) );
        return false;
    }

    wxString foundfile = file.GetFullPath();
    wxString fileName( foundfile );
    wxString dir, name, ext;
    wxFileName::SplitPath( fileName, & dir, & name, & ext );

    if ( n > 1 )
    {
        //reset the preferred template if not in the list of templates.
        if ( !preferredExportTemplate )
            m_preferredExportTemplate = NULL;
        else if ( dir.IsEmpty() && !m_preferredExportTemplate->GetDirectory().IsEmpty() )
                dir = m_preferredExportTemplate->GetDirectory();
        // let the user choose a template and file.
        wxArrayString selectedPaths;
        a2dError result =   SelectDocumentPath( 
							_( "Export File" ), 
							onlyThisDocTemplates,
							dir,
							a2dREFDOC_NON,
							&selectedPaths,
							&docTemplate,
							/*wxHIDE_READONLY |*/ wxFD_SAVE | wxFD_OVERWRITE_PROMPT,
							a2dTemplateFlag::VISIBLE | a2dTemplateFlag::EXPORTING,
							m_preferredExportTemplate );
        if( result == a2dError_NoError )
        {
            foundfile = selectedPaths.Item( 0 );
            docTemplate->SetDirectory( wxPathOnly( foundfile ) );
            m_preferredExportTemplate = docTemplate;
        }
        else if( result == a2dError_Canceled )
        {
            return false;
        }
        else
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "suitable template I/O handler for loading not available in document templates." ) );
            return false;
        }
    }
    else if ( !( flags & a2dREFDOC_SILENT ) )
    {
        if ( dir.IsEmpty() )
        {
            dir = docTemplate->GetDirectory();
            name = foundfile;
            if ( ext.IsEmpty() )
                ext = docTemplate->GetDefaultExtension();
        }
        foundfile = wxFileSelector( _( "Export as" ),
                               dir,
                               name,
                               ext,
                               docTemplate->GetFileFilter(),
                               wxFD_SAVE | wxFD_OVERWRITE_PROMPT,
                               doc->GetAssociatedWindow() );

        if ( foundfile.IsEmpty() )
            return false;

        wxFileName::SplitPath( foundfile, & dir, & name, & ext );
    }

    if ( ext.IsEmpty() )
    {
        fileName += wxT( "." );
        fileName += docTemplate->GetDefaultExtension();
    }

    // let the user choose a template and file if not found
    return doc->Export( docTemplate, foundfile, flags );
}

bool a2dDocumentCommandProcessor::FileImport( const wxFileName& file, const wxString& description, a2dDocumentFlagMask flags )
{
    a2dDocument* doc = GetCurrentDocument();
    if ( !doc )
        return false;

    // Find the templates for this type of document.
    a2dDocumentTemplate* docTemplate = NULL;
    a2dDocumentTemplate* preferredImportTemplate = NULL;
    size_t n = 0;
    a2dDocumentTemplateList onlyThisDocTemplates;
    const_forEachIn( a2dDocumentTemplateList, &m_docTemplates )
    {
        a2dDocumentTemplateList::value_type temp = *iter;
        if ( temp->CheckMask( a2dTemplateFlag::IMPORTING ) )
        {
            // check on description, or else on file extension.
            if ( ( description.IsEmpty() && !file.GetExt().IsEmpty() && file.GetExt() == temp->GetDefaultExtension() ) ||
                   temp->GetDescription() == description ||
                   file.GetExt().IsEmpty()
               )
            {
                if ( temp->GetDocumentTypeName() == doc->GetDocumentTemplate()->GetDocumentTypeName() &&
                     ( !temp->GetDocumentIOHandlerStrOut() || temp->GetDocumentIOHandlerStrOut()->CanSave( doc ) )
                   ) 
                {
                    onlyThisDocTemplates.push_back( temp );
                    docTemplate = temp;
                    n++;
                    if ( m_preferredImportTemplate == temp )
                        preferredImportTemplate = temp;
                }
            }
        }
    }

    if ( !docTemplate )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_NoDocTemplateRef, _( "Sorry, there is no document template for importing this document." ) );
        return false;
    }

    wxString foundfile = file.GetFullPath();
    wxString fileName( foundfile );
    wxString dir, name, ext;
    wxFileName::SplitPath( fileName, & dir, & name, & ext );

    if ( n > 1 )
    {
        //reset the preferred template if not in the list of templates.
        if ( !preferredImportTemplate )
            m_preferredImportTemplate = NULL;
        else if ( dir.IsEmpty() && !m_preferredImportTemplate->GetDirectory().IsEmpty() )
                dir = m_preferredImportTemplate->GetDirectory();
        // let the user choose a template and file.
        wxArrayString selectedPaths;
        a2dError result = SelectDocumentPath( 
                          _( "File import" ),
                          onlyThisDocTemplates,
                          dir,
                          a2dREFDOC_NON,
                          &selectedPaths,
                          &docTemplate,
                          wxFD_OPEN,
                          a2dTemplateFlag::VISIBLE | a2dTemplateFlag::IMPORTING,
                          m_preferredImportTemplate );
        if( result == a2dError_NoError )
        {
            foundfile = selectedPaths.Item( 0 );
            docTemplate->SetDirectory( wxPathOnly( foundfile ) );
            m_preferredImportTemplate = docTemplate;
        }
        else if( result == a2dError_Canceled )
        {
            return false;
        }
        else
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "suitable template I/O handler for import not available in document templates." ) );
            return false;
        }
    }
    else if ( !( flags & a2dREFDOC_SILENT ) )
    {
        foundfile = wxFileSelector( _( "File import" ),
                               docTemplate->GetDirectory(),
                               foundfile,
                               docTemplate->GetDefaultExtension(),
                               docTemplate->GetDescription()
                                 << wxT( " (" ) << docTemplate->GetFileFilter() << wxT( ") |" )
                                 << docTemplate->GetFileFilter(),
                               wxFD_OPEN,
                               doc->GetAssociatedWindow() );

        if ( foundfile.IsEmpty() )
            return false; // canceled

        docTemplate->SetDirectory( wxPathOnly( foundfile ) );
    }

    if ( ext.IsEmpty() )
    {
        fileName += wxT( "." );
        fileName += docTemplate->GetDefaultExtension();
    }

    // let the user choose a template and file if not found
    return doc->Import( docTemplate, foundfile, flags );
}


bool a2dDocumentCommandProcessor::Print( a2dPrintWhat printWhat )
{
#if wxUSE_PRINTING_ARCHITECTURE
    a2dView* view = GetCurrentView();
    if ( !view )
        return false;

    wxPageSetupDialogData* aPageSetupData = NULL;
    if ( view && view->GetViewTemplate() )
        aPageSetupData = view->GetViewTemplate()->GetPageSetupData();
    if( !aPageSetupData )
    {
        a2dDocument* aDoc = GetCurrentDocument();
        if ( aDoc && aDoc->GetDocumentTemplate() )
            aPageSetupData = aDoc->GetDocumentTemplate()->GetPageSetupData();
    }
    if( !aPageSetupData )
        aPageSetupData = GetPageSetupData();

    wxPrintDialogData printDialogData( aPageSetupData->GetPrintData() );

    wxPrintout* printout = view->OnCreatePrintout( printWhat, *aPageSetupData );
    if ( printout )
    {
        wxPrinter printer( &printDialogData );
        if( printer.Print( view->GetDisplayWindow(), printout, true ) )
            aPageSetupData->SetPrintData( printer.GetPrintDialogData().GetPrintData() );

        delete printout;
    }
#endif // wxUSE_PRINTING_ARCHITECTURE
    return true;
}

bool a2dDocumentCommandProcessor::Preview( a2dPrintWhat printWhat )
{
#if wxUSE_PRINTING_ARCHITECTURE
    a2dView* view = GetCurrentView();
    if ( !view )
        return false;
    wxPageSetupDialogData* aPageSetupData = NULL;
    if ( view && view->GetViewTemplate() )
        aPageSetupData = view->GetViewTemplate()->GetPageSetupData();
    if( !aPageSetupData )
    {
        a2dDocument* aDoc = GetCurrentDocument();
        if ( aDoc && aDoc->GetDocumentTemplate() )
            aPageSetupData = aDoc->GetDocumentTemplate()->GetPageSetupData();
    }
    if( !aPageSetupData )
        aPageSetupData = GetPageSetupData();

    wxPrintDialogData printDialogData( aPageSetupData->GetPrintData() );

    wxPrintout* printout = view->OnCreatePrintout( printWhat, *aPageSetupData );
    if ( printout )
    {
        // Pass two printout objects: for preview, and possible printing.
        wxPrintPreviewBase* preview = ( wxPrintPreviewBase* ) NULL;
        preview = new wxPrintPreview( printout, view->OnCreatePrintout( printWhat, *aPageSetupData ), &printDialogData );
        if ( !preview->Ok() )
        {
            delete preview;
            wxMessageBox( _( "Sorry, print preview needs a printer to be installed." ) );
            return false;
        }

        wxPreviewFrame* frame = new wxPreviewFrame( preview, ( wxFrame* )wxTheApp->GetTopWindow(), _( "Print Preview" ),
                wxPoint( 100, 100 ), wxSize( 600, 650 ) );
        frame->Centre( wxBOTH );
        frame->Initialize();
        frame->Show( true );
    }
#endif // wxUSE_PRINTING_ARCHITECTURE
    return true;
}

void a2dDocumentCommandProcessor::SetPageSetupData( wxPageSetupDialogData* pageSetupData )
{ 
#if wxUSE_PRINTING_ARCHITECTURE
    if( m_pageSetupData )
        delete m_pageSetupData;
    m_pageSetupData = pageSetupData; 
#endif
}

bool a2dDocumentCommandProcessor::PrintSetup( a2dPrintWhat printWhat )
{
#if wxUSE_PRINTING_ARCHITECTURE
    wxPageSetupDialogData* aPageSetupData = GetPrintSetup( printWhat );
    wxPageSetupDialog pageSetupDialog( ( wxFrame* )wxTheApp->GetTopWindow(), aPageSetupData );
    pageSetupDialog.ShowModal();
    *aPageSetupData = pageSetupDialog.GetPageSetupData();
#endif // wxUSE_PRINTING_ARCHITECTURE
    return true;
}

wxPageSetupDialogData a2dDocumentCommandProcessor::GetDefaultPrintSettings()
{
    wxPageSetupDialogData* d;

    if ( m_pageSetupData )
        d = m_pageSetupData;
    else 
        d = new wxPageSetupDialogData();

    d->SetDefaultInfo(true); // set PSD_RETURNDEFAULT flag
    // This doesn't actually show any dialog thanks to the
    // above flag; it returns default data instead.
    
#ifdef __WXGTK__
    wxPageSetupDialog pageSetupDialog(NULL, d);
    if ( pageSetupDialog.ShowModal() != wxID_OK )
      { /* fail somehow */ }

    if ( !m_pageSetupData )
        delete d;

    return pageSetupDialog.GetPageSetupData();
#else    
    wxPageSetupDialog pageSetupDialog(NULL, d);
    if ( pageSetupDialog.ShowModal() != wxID_OK )
      { /* fail somehow */ }
    d->SetDefaultInfo(false); // set PSD_RETURNDEFAULT flag

    if ( !m_pageSetupData )
        delete d;

    return pageSetupDialog.GetPageSetupData();
#endif
}

wxPageSetupDialogData* a2dDocumentCommandProcessor::GetPrintSetup( a2dPrintWhat printWhat )
{
#if wxUSE_PRINTING_ARCHITECTURE

    wxPageSetupDialogData* aPageSetupData = NULL;

    if ( printWhat == a2dPRINT_PrintView )
    {
        a2dView* view = GetCurrentView();
        if ( view && view->GetViewTemplate() )
            aPageSetupData = view->GetViewTemplate()->GetPageSetupData();
    }
    else if ( printWhat == a2dPRINT_PrintDocument || printWhat == a2dPRINT_PreviewDocument )
    {
        a2dDocument* aDoc = GetCurrentDocument();
        if ( aDoc && aDoc->GetDocumentTemplate() )
            aPageSetupData = aDoc->GetDocumentTemplate()->GetPageSetupData();
    }
    else if ( printWhat == a2dPRINT_PrintSetup )
    {
        a2dView* view = GetCurrentView();
        if ( view && view->GetViewTemplate() )
            aPageSetupData = view->GetViewTemplate()->GetPageSetupData();
        if ( !aPageSetupData )
        {
            a2dDocument* aDoc = GetCurrentDocument();
            if ( aDoc && aDoc->GetDocumentTemplate() )
                aPageSetupData = aDoc->GetDocumentTemplate()->GetPageSetupData();
        }
    }
    else
        aPageSetupData = GetPageSetupData();

    if ( !aPageSetupData )
        aPageSetupData = GetPageSetupData();

    return aPageSetupData;
#endif // wxUSE_PRINTING_ARCHITECTURE
    return NULL;
}

a2dView* a2dDocumentCommandProcessor::GetCurrentView() const
{
    return m_currentView;
}

void a2dDocumentCommandProcessor::OnCannotOpenDocument( a2dCommandEvent& event )
{
    a2dDocument* newDoc = event.GetDocument();
    newDoc->DisConnectAllViews();
    newDoc->DeleteContents();
}

a2dError a2dDocumentCommandProcessor::CreateDocuments( const wxString& path, a2dDocumentFlagMask documentflags,
        a2dDocumentTemplate* wantedDocTemplate,
        int dialogflags,
        a2dTemplateFlagMask docTemplateFlags )
{
    // remember last used template
    size_t n = 0;
    if ( !wantedDocTemplate )
    {
        // check is there is only one template visible, meaning we do not have to choose one.
        const_forEachIn( a2dDocumentTemplateList, &m_docTemplates )
        {
            a2dDocumentTemplateList::value_type temp = *iter;
            if ( temp->CheckMask( docTemplateFlags ) )
            {
                if ( n == 0 )
                    wantedDocTemplate = temp;
                n ++;
            }
        }

        if ( n == 0 )
        {
            return a2dError_NoDocTemplateRef;
        }
    }
    else
        n = 1;

    //in an application with only one view and document,
    //the application wants to close ( and save ) the document when creating a new document.
    //so call Close to generate a a2dCloseDocumentEvent.
    //In such a case m_maxDocsOpen should be one.

    // If we've reached the max number of docs, close the
    // first one.
    if ( m_docs.size() >= m_maxDocsOpen )
    {
        a2dDECLARE_LOCAL_ITEM( a2dDocumentList::value_type, doc, m_docs.front() );
        //The handler that intercepts EVT_CLOSE_DOCUMENT must decide to Save and/or Empty the document
        if ( doc->Close( false ) )
        {
            doc->DisConnectAllViews();
            RemoveDocument( doc );
        }
        else
        {
            //clean up
            return a2dError_ToManyOpen;
        }
    }

    // New document: user chooses a template, unless there's only one or one was given as input.
    if ( documentflags & a2dREFDOC_NEW )
    {
        a2dDocumentTemplate* fileTemplate = ( a2dDocumentTemplate* ) NULL;
        if ( n == 1 )
            fileTemplate = wantedDocTemplate;
        else
            fileTemplate = SelectDocumentType( false, docTemplateFlags ); //choose document template from list of templates

        if ( fileTemplate )
        {
            a2dDocumentList openedDocList;
            a2dSmrtPtr< a2dDocument > newDoc = fileTemplate->CreateDocument( path, documentflags );

            // arrived here, the new document is ready for use.  But the document is not loaded or initiated yet.
            // For that the wxEVT_NEW_DOCUMENT will take care.
            if ( newDoc )
            {
                a2dDocumentEvent event( wxEVT_NEW_DOCUMENT );
                event.SetEventObject( newDoc );
                newDoc->ProcessEvent( event );

				if ( !event.IsAllowed() )
				{
                    // The document was already created, but since it is vetod, it will be deleted.
                    // The next makes sure to leaf application in a stable state. Either last active view or NULL.
                    SetCurrentView( m_currentView ); 
					//newDoc will be released if Veto was set.
					return a2dError_Canceled;
				}

                a2dDocument* ret = fileTemplate->SentPreAddCreatedDocumentEvent( newDoc, documentflags );

                //and only now if the new document is not closed because document data may have bin merged/copied
                //to another already opened document. In that case ret contains that document.
                if ( ret == newDoc && !newDoc->IsClosed() ) // really use this new document?
                {
                    // add the document because every thing is properly set
                    AddDocument( newDoc );
                    fileTemplate->SentPostCreateDocumentEvent( newDoc, documentflags );
                    // add the new document to opened documents list which update all views
                    openedDocList.push_back( newDoc );

                    // arrived here, the new document is ready and its views ( and related windows )
                    // are in place.

                    // now that frames and views are created, set the filename once more, this will generate proper events
                    // for setting e.g. title of frames
                    newDoc->SetFilename( newDoc->GetFilename(), true );
                    newDoc->SetTitle( newDoc->GetTitle(), true );
                }
            }
            else
            {
                return a2dError_CouldNotCreateDocument;
            }

            a2dDocumentList::iterator iter = openedDocList.begin();
            for( ; iter != openedDocList.end(); iter++ )
            {
                a2dDocument* aDoc = *iter;
                // in the previous often view are created, are existing views
                // are connected, update them now.
                aDoc->UpdateAllViews();
            }
            return a2dError_NoError;

        }
        else
        {
            return a2dError_NoDocTemplateRef;
        }
    }

    //Create new documents using the path to the file or by selecting a file or more.
    a2dDocumentTemplate* fileTemplate = ( a2dDocumentTemplate* ) NULL;

    wxArrayString selectedPaths;

    if ( documentflags & a2dREFDOC_SILENT )
    {
        if ( n == 1 )
        {
            fileTemplate = wantedDocTemplate;
            selectedPaths.Add( path );
        }
        else
        {
            // find a template which is able to load this file ( use file ext or iohandler of template )
            fileTemplate = FindTemplateForPath( m_docTemplates, path ); //File extension or format test to find template
            if( fileTemplate )
                selectedPaths.Add( path ); // just one file is selected this way.
            else
            {
                return a2dError_NoDocTemplateRef;
            }
        }
    }
    else
    {
        if ( n == 1 )
        {
            a2dDocumentTemplateList docTemplates;
            docTemplates.push_back( wantedDocTemplate );
            fileTemplate = wantedDocTemplate;
			wxString pathInOut = path;
            // selection of one or more files.
            if( a2dError_NoError == SelectDocumentPath( _("File Open"), docTemplates, pathInOut, documentflags, &selectedPaths, &fileTemplate, dialogflags, docTemplateFlags, m_preferredOpenTemplate ) )
			{
                m_preferredOpenTemplate = fileTemplate;
				m_lastDirectory = pathInOut;
			}
            else
                return a2dError_Canceled;
        }
        else
        {
			wxString pathInOut = path;
            // selection of one or more files.
            if( a2dError_NoError == SelectDocumentPath( _("File Open"), m_docTemplates, pathInOut, documentflags, &selectedPaths, &fileTemplate, dialogflags, docTemplateFlags, m_preferredOpenTemplate ) )
			{
                m_preferredOpenTemplate = fileTemplate;
				m_lastDirectory = pathInOut;
			}
            else
                return a2dError_Canceled;
        }
    }

    // the selected files are in selectedPaths
    if ( fileTemplate )
    {
        fileTemplate->SetDirectory( m_lastDirectory );

        // start creating documents + views for all opened files.
        a2dDocumentList openedDocList; // temporary list of document added here, will be used to update its views.
        a2dSmrtPtr< a2dDocument > newDoc;
        for( size_t i = 0; i < selectedPaths.GetCount(); i++ )
        {
            bool alreadyOpen = false;
            wxString filename = selectedPaths[i];

            a2dDocumentList::iterator iter = m_docs.begin();
            if ( !m_allowOpenSameFile )
            {
                while( iter != m_docs.end() )
                {
                    a2dDocumentPtr doc = *iter;
                    if ( doc->GetFilename().GetFullPath() == filename )
                    {
                        alreadyOpen = true;
                        break;
                    }
                    iter++;
                }
            }
            if ( !alreadyOpen )
            {
                newDoc = fileTemplate->CreateDocument( filename, documentflags );

                // arrived here, the new document is ready for use.  But the document is not loaded or initiated yet.
                // For that the wxEVT_OPEN_DOCUMENT will take care.
                if ( newDoc )
                {
                    a2dDocumentEvent eventopen( wxEVT_OPEN_DOCUMENT );
                    eventopen.SetFileName( filename );
                    eventopen.SetEventObject( newDoc );
                    if ( !newDoc->ProcessEvent( eventopen ) || !eventopen.IsAllowed() )
                    {
                        a2dCommandEvent event( wxEVT_CANNOT_OPEN_DOCUMENT, newDoc );
                        event.SetEventObject( this );
                        ProcessEvent( event );
                        return eventopen.GetError(); 
                    }

                    a2dDocument* ret = fileTemplate->SentPreAddCreatedDocumentEvent( newDoc, documentflags );

                    //and only now if the new document is not closed because document data may have bin merged/copied
                    //to another already opened document. In that case ret contains that document.
                    if ( ret == newDoc && !newDoc->IsClosed() ) // really use this new document?
                    {
                        // add the document because every thing is properly set
                        AddDocument( newDoc );
                        AddFileToHistory( filename, fileTemplate );

                        fileTemplate->SentPostCreateDocumentEvent( newDoc, documentflags );
                        // add the new document to opened documents list which update all views
                        openedDocList.push_back( newDoc );

                        // arrived here, the new document is ready and its views ( and related windows )
                        // are in place.

                        // now that frames and views are created, set the filename once more, this will generate proper events
                        // for setting e.g. title of frames
                        newDoc->SetFilename( newDoc->GetFilename(), true );
                        newDoc->SetTitle( newDoc->GetTitle(), true );
                    }
                }
                else
                {
                    return a2dError_CouldNotCreateDocument;
                }
            }
            else
            {
                //give first view on this document the focus.
                a2dDocument* aDoc = *iter;
                a2dViewList allviews;
                aDoc->ReportViews( &allviews );
                if ( allviews.size() )
                {
                    a2dViewPtr view = *(allviews.begin());
                    view->Activate( true );
                }
            }
        }
        a2dDocumentList::iterator iter = openedDocList.begin();
        for( ; iter != openedDocList.end(); iter++ )
        {
            a2dDocument* aDoc = *iter;
            // in the previous often views are created, or existing views are connected, update them now.
            aDoc->UpdateAllViews();
        }

        return a2dError_NoError;
    }

    return a2dError_NoDocTemplateRef;
}

a2dView* a2dDocumentCommandProcessor::AddDocumentCreateView(
    a2dDocument* newDoc, const wxString& viewTypeName,
    a2dDocumentFlagMask documentflags,
    a2dTemplateFlagMask documentTemplateFlags,
    a2dTemplateFlagMask viewTemplateFlags )
{
    AddCreatedDocument( newDoc,  false, false, documentflags, documentTemplateFlags );
    return CreateView( newDoc, viewTypeName, documentflags, viewTemplateFlags );
}

a2dError a2dDocumentCommandProcessor::AddCreatedDocument( a2dDocument* newDoc,
        bool sentPreAddCreatedDocumentEvent, bool sentPostCreateDocumentEvent,
        a2dDocumentFlagMask documentflags,
        a2dTemplateFlagMask docTemplateFlags )
{
    //search a template for it.
    a2dDocumentTemplate* foundtemplate = newDoc->GetDocumentTemplate();

    // in case the template is set already, use it.
    if ( !newDoc->GetDocumentTemplate() )
    {
        const_forEachIn( a2dDocumentTemplateList, &m_docTemplates )
        {
            a2dDocumentTemplateList::value_type temp = *iter;
            if ( temp->CheckMask( docTemplateFlags ) )
            {
                if ( temp->GetDocumentTypeName() == newDoc->GetDocumentTypeName() )
                {
                    foundtemplate = temp;
                    break;
                }
            }
        }
    }

    wxASSERT_MSG( foundtemplate,
                  _( "a2dDocumentCommandProcessor::AddCreatedDocument Could not find template for document type name" ) );

    wxASSERT_MSG( foundtemplate->GetDocumentTypeName() == newDoc->GetDocumentTypeName(),
                  _( "a2dDocumentCommandProcessor::AddCreatedDocument template DocumentTypeName different from document" ) );

    if ( !foundtemplate )
        return a2dError_NoDocTemplateRef;

    //in an application with only one view and document,
    //the application wants to close ( and save ) the document when creating a new document.
    //so call Close to generate a a2dCloseDocumentEvent.
    //In such a case m_maxDocsOpen should be one.

    // If we've reached the max number of docs, close the
    // first one.
    if ( m_docs.size() >= m_maxDocsOpen )
    {
        a2dDocument* doc = GetDocuments().front();
        //The handler that intercepts EVT_CLOSE_DOCUMENT must decide to Save and/or Empty the document
        if ( doc->Close( false ) )
        {
            doc->DisConnectAllViews();
            RemoveDocument( doc );
        }
        else
        {
            //clean up
            return a2dError_Canceled;
        }
    }

    //instead of the document template generating a new document,
    //here we already have that document, and we set things as if template did create the new document.
    newDoc->SetFilename( wxFileName( wxT( "" ) ) );
    newDoc->CreateCommandProcessor();
    newDoc->SetDocumentTypeName( foundtemplate->GetDocumentTypeName() );
    newDoc->SetDocumentTemplate( foundtemplate );
    a2dDocumentEvent event( wxEVT_NEW_DOCUMENT );
    event.SetEventObject( newDoc );
    newDoc->ProcessEvent( event );

	if ( !event.IsAllowed() )
	{
		//newDoc will be released if Veto was set.
		return a2dError_Canceled;
	}

    if ( sentPreAddCreatedDocumentEvent )
    {
        foundtemplate->SentPreAddCreatedDocumentEvent( newDoc, documentflags );
    }

    //now add the document because every thing is properly set
    AddDocument( newDoc );

    if ( sentPostCreateDocumentEvent )
    {
        foundtemplate->SentPostCreateDocumentEvent( newDoc, documentflags );

        // in the previous often view are created, are existing views
        // are connected, update them now.
        newDoc->UpdateAllViews();
    }

    return a2dError_NoError;
}

a2dView* a2dDocumentCommandProcessor::CreateView( a2dDocument* doc, const wxString& viewTypeName,
        a2dDocumentFlagMask flags, a2dTemplateFlagMask viewTemplateFlags )
{
    //let the user/program choose a view from the list
    a2dViewTemplate* temp = SelectViewType( doc, m_viewTemplates, viewTypeName, false, viewTemplateFlags );
    if ( temp )
    {
        return temp->CreateView( doc, flags );
    }

    wxASSERT_MSG( viewTypeName.IsEmpty(), _( "a2dDocumentCommandProcessor::CreateView could not find template of given type" ) );

    return ( a2dView* ) NULL;
}

// Not yet implemented
void a2dDocumentCommandProcessor::ReleaseTemplate( a2dDocumentTemplate* temp, long flags )
{
    a2dSmrtPtrList<a2dDocumentTemplate>::itSmart iter;
    iter = m_docTemplates.Find( temp );
    if ( m_docTemplates.end() !=  iter )
    {
        m_docTemplates.erase( iter );
    }
}

// Not yet implemented
bool a2dDocumentCommandProcessor::FlushDoc( a2dDocument* WXUNUSED( doc ) )
{
    return false;
}

a2dDocument* a2dDocumentCommandProcessor::GetCurrentDocument() const
{
    if ( m_currentDocument )
        return m_currentDocument;
    else
        return ( a2dDocument* ) NULL;
}

a2dCommandProcessor* a2dDocumentCommandProcessor::GetCurrentDocumentCommandProcessor() const
{
    if ( !m_currentDocument )
        return NULL;

    if ( !m_currentDocument->GetCommandProcessor() )
        return NULL;

    return m_currentDocument->GetCommandProcessor();
}

// Make a default document name
bool a2dDocumentCommandProcessor::MakeDefaultName( wxString& name )
{
    name.Printf( _( "unnamed%d" ), m_defaultDocumentNameCounter );
    m_defaultDocumentNameCounter++;

    return true;
}

// Make a frame title (override this to do something different)
// If docName is empty, a document is not currently active.
wxString a2dDocumentCommandProcessor::MakeFrameTitle( a2dDocument* doc, const wxString& modifiedIndicator )
{
    wxString appName = wxTheApp->GetAppName();
    wxString title;
    if ( !doc )
        title = appName;
    else
    {
        wxString docName = doc->GetPrintableName();

        if ( doc->IsModified() )
            docName = docName + modifiedIndicator;

        title = docName + wxString( wxT( " - " ) ) + appName;
    }
    return title;
}


a2dDocumentTemplate* a2dDocumentCommandProcessor::MatchTemplate( const wxString& path )
{
    return FindTemplateForPath( m_docTemplates, path ); //File extension or format test to find template
}

// File history management
void a2dDocumentCommandProcessor::AddFileToHistory(  const wxFileName& file, a2dDocumentTemplate* docTemplate, a2dViewTemplate* viewTemplate )
{
    if ( m_fileHistory )
        m_fileHistory->AddFileToHistory( file, docTemplate, viewTemplate );
}

void a2dDocumentCommandProcessor::RemoveFileFromHistory( size_t i )
{
    if ( m_fileHistory )
        m_fileHistory->RemoveFileFromHistory( i );
}

wxString a2dDocumentCommandProcessor::GetHistoryFile( size_t i ) const
{
    wxString histFile;

    if ( m_fileHistory )
    {
        a2dFileHistoryItem* item = m_fileHistory->GetHistoryFileItem( i );
        if  ( item )
            histFile = item->m_filename.GetFullPath();
    }
    return histFile;
}

a2dFileHistoryItem* a2dDocumentCommandProcessor::GetHistoryFileItem( size_t i ) const
{
    if ( m_fileHistory )
        return m_fileHistory->GetHistoryFileItem( i );
    return NULL;
}

void a2dDocumentCommandProcessor::FileHistoryUseMenu( wxMenu* menu )
{
    if ( m_fileHistory )
        m_fileHistory->UseMenu( menu );
}

void a2dDocumentCommandProcessor::FileHistoryRemoveMenu( wxMenu* menu )
{
    if ( m_fileHistory )
        m_fileHistory->RemoveMenu( menu );
}

#if wxUSE_CONFIG
void a2dDocumentCommandProcessor::FileHistoryLoad( wxConfigBase& config )
{
    if ( m_fileHistory )
        m_fileHistory->Load( config );
}

void a2dDocumentCommandProcessor::FileHistorySave( wxConfigBase& config )
{
    if ( m_fileHistory )
        m_fileHistory->Save( config );
}
#endif

void a2dDocumentCommandProcessor::FileHistoryAddFilesToMenu( wxMenu* menu )
{
    if ( m_fileHistory )
        m_fileHistory->AddFilesToMenu( menu );
}

void a2dDocumentCommandProcessor::FileHistoryAddFilesToMenu()
{
    if ( m_fileHistory )
        m_fileHistory->AddFilesToMenu();
}

size_t a2dDocumentCommandProcessor::GetHistoryFilesCount() const
{
    if ( m_fileHistory )
        return m_fileHistory->GetCount();
    else
        return 0;
}


// Find out the document template via matching in the document file format
// against that of the template
a2dDocumentTemplate* a2dDocumentCommandProcessor::FindTemplateForPath( const a2dDocumentTemplateList& docTemplates, const wxString& path, a2dTemplateFlagMask mask )
{
    a2dDocumentTemplate* theTemplate = ( a2dDocumentTemplate* ) NULL;

    // Find the template which this extension corresponds to
    const_forEachIn( a2dDocumentTemplateList, &docTemplates )
    {
        a2dDocumentTemplateList::value_type temp = *iter;
        if ( temp->FileMatchesTemplate( path ) && temp->CheckMask( mask ) )
        {
            theTemplate = temp;
            break;
        }
    }
    return theTemplate;
}

// Try to get a more suitable parent frame than the top window,
// for selection dialogs. Otherwise you may get an unexpected
// window being activated when a dialog is shown.
wxWindow* wxFindSuitableParent()
{
    wxWindow* parent = wxTheApp->GetTopWindow();

    wxWindow* focusWindow = wxWindow::FindFocus();
    if ( focusWindow )
    {
        while ( focusWindow &&
                !focusWindow->IsKindOf( CLASSINFO( wxDialog ) ) &&
                !focusWindow->IsKindOf( CLASSINFO( wxFrame ) ) )

            focusWindow = focusWindow->GetParent();

        if ( focusWindow )
            parent = focusWindow;
    }
    return parent;
}

// Prompts user to open one or more files, using file specs in templates.
a2dError a2dDocumentCommandProcessor::SelectDocumentPath( 
	    const wxString& title, 
	    const a2dDocumentTemplateList& docTemplates,
        wxString& path,
        a2dDocumentFlagMask WXUNUSED( flags ),
        wxArrayString* selectedPaths,
        a2dDocumentTemplate** chosenTemplate,
        int dialogflags,
        a2dTemplateFlagMask docTemplateFlags,
        const a2dDocumentTemplate* preferedTemplate )
{
    wxString descrBuf;
    int FilterIndex = -1;

    // We can only have multiple filters in Windows and GTK
#if defined(__WXMSW__) || defined(__WXGTK__)

    // search visible filters, and get position of preferred template
    int filterCount = 0;
    const_forEachIn( a2dDocumentTemplateList, &docTemplates )
    {
        a2dDocumentTemplateList::value_type temp = *iter;
        if ( temp->CheckMask( docTemplateFlags ) )
        {
            if( temp == preferedTemplate )
                FilterIndex = filterCount;

            filterCount++;
            // add a '|' to separate this filter from the previous one
            if ( !descrBuf.IsEmpty() )
                descrBuf << wxT( '|' );

            descrBuf << temp->GetDescription()
                     << wxT( " (" ) << temp->GetFileFilter() << wxT( ") |" )
                     << temp->GetFileFilter();
        }
    }
#else
    descrBuf << wxT( "*.*" );
#endif

    // if not given a specific directory to open in, use template dir
    if ( path.IsEmpty() )
	{
        // use last directory chosen for this specific type of document, as was stored in the template
        if ( preferedTemplate )
            path = preferedTemplate->GetDirectory();
		else if ( docTemplates.size() )
            path = docTemplates.front()->GetDirectory(); //just the first
    }

    wxWindow* parent = wxFindSuitableParent();

    *chosenTemplate = ( a2dDocumentTemplate* )NULL;

    if ( a2dError_Canceled == a2dFileSelectorEx( title, path, wxT( "" ), &FilterIndex,
            descrBuf, dialogflags, parent, selectedPaths ) )
        return a2dError_Canceled; // since canceled file dialog

    if ( selectedPaths->GetCount() )
    {
        if ( dialogflags & wxFD_OPEN )
        {
            // check files for existence
            for( size_t i = 0; i < selectedPaths->GetCount(); i++ )
                if( !wxFileExists( selectedPaths->Item( i ) ) )
                {
                    wxString msgTitle;
                    if ( !wxTheApp->GetAppName().IsEmpty() )
                        msgTitle = wxTheApp->GetAppName();
                    else
                        msgTitle = wxString( _( "File error" ) );

                    wxString buf;
                    buf.Printf( _( "Sorry, could not open the file: %s\n" ), selectedPaths->Item( i ).c_str() );
                    wxMessageBox( buf , msgTitle, wxOK | wxICON_EXCLAMATION );
                    return a2dError_FileCouldNotOpen;
                }
        }
        path = wxPathOnly( selectedPaths->Item( 0 ) );

        // first find the template that was choosen in file dialog, if this fails (i.e.
        // wxFileSelectorEx() didn't fill it), then use the path
        if ( FilterIndex != -1 )
        {
            int n = 0;
            const_forEachIn( a2dDocumentTemplateList, &docTemplates )
            {
                a2dDocumentTemplateList::value_type temp = *iter;
                // use same filter as used for descrBuf above.
                if ( temp->CheckMask( docTemplateFlags ) )
                {
                    if ( n == FilterIndex )
                    {
                        *chosenTemplate = temp;
                        break;
                    }
                    // count 'n' here because FilterIndex = filterCount which count if temp->CheckMask( docTemplateFlags )
                    n++;
                }
            }
        }
        // if not found, use the path to find a template
        if ( dialogflags & wxFD_OPEN && !*chosenTemplate )
            *chosenTemplate = FindTemplateForPath( docTemplates, path );

        return a2dError_NoError;
    }
    return a2dError_NoSelection;
}

a2dDocumentTemplate* a2dDocumentCommandProcessor::SelectDocumentType( bool sort, a2dTemplateFlagMask docTemplateFlags )
{
    wxArrayString strings;

    a2dDocumentTemplate** templates = new a2dDocumentTemplate *[m_docTemplates.size()];
    int n = 0;

    const_forEachIn( a2dDocumentTemplateList, &m_docTemplates )
    {
        a2dDocumentTemplateList::value_type temp = *iter;
        if ( temp->CheckMask( docTemplateFlags ) )
        {
            int j;
            bool want = true;
            for ( j = 0; j < n; j++ )
            {
                //filter out NOT unique document combinations
                if ( temp->GetDocumentTypeName() == templates[j]->GetDocumentTypeName() )
                    want = false;
            }

            if ( want )
            {
                strings.Add( temp->GetDescription() );
                templates[n] = temp;
                n ++;
            }
        }
    }  // for

    if ( sort )
    {
		strings.Sort(); // ascending sort
        // Yes, this will be slow, but template lists
        // are typically short.
        int i;
        n = strings.Count();
        for ( i = 0; i < n; i++ )
        {
            const_forEachIn( a2dDocumentTemplateList, &m_docTemplates )
            {
                a2dDocumentTemplateList::value_type temp = *iter;
                if ( strings[i] == temp->GetDescription() )
                    templates[i] = temp;
            }
        }
    }

    a2dDocumentTemplate* theTemplate;

    switch ( n )
    {
        case 0:
            // no visible templates, hence nothing to choose from
            theTemplate = NULL;
            break;

        case 1:
            // don't propose the user to choose if he heas no choice
            theTemplate = templates[0];
            break;

        default:
            // propose the user to choose one of several
            theTemplate = ( a2dDocumentTemplate* )wxGetSingleChoiceData
                          (
                              _( "Select a document template" ),
                              _( "Templates" ),
                              strings,
                              ( void** )templates,
                              wxFindSuitableParent()
                          );
    }

    delete[] templates;

    return theTemplate;
}

a2dViewTemplate* a2dDocumentCommandProcessor::SelectViewType( a2dDocument* doc,
        const a2dViewTemplateList& list,
        const wxString& viewTypeName,
        bool sort,
        a2dTemplateFlagMask mask )
{
    if ( !viewTypeName.IsEmpty() )
    {
        //a specific view type is asked for, search it and return it.
        const_forEachIn( a2dViewTemplateList, &list )
        {
            a2dViewTemplateList::value_type temp = *iter;
            if ( temp->GetViewTypeName() == viewTypeName )
            {
                return temp;
            }
        }
        return ( a2dViewTemplate* )NULL;
    }

    wxArrayString strings;
    a2dViewTemplate** templates = new a2dViewTemplate *[list.size()];

    int n = 0;

    //fill new array of templates with wanted views
    const_forEachIn( a2dViewTemplateList, &list )
    {
        a2dViewTemplateList::value_type temp = *iter;
        if ( !temp->GetViewTypeName().empty() && temp->CheckMask( mask ) &&
                temp->GetDocumentTypeName() == doc->GetDocumentTypeName() )
        {
            strings.Add( temp->GetViewTypeName() );
            templates[n] = temp;
            n ++;
        }
    }

    if ( sort )
    {
        strings.Sort();

        // Yes, this will be slow, but template lists
        // are typically short.
        int i;
        n = strings.Count();
        for ( i = 0; i < n; i++ )
        {
            const_forEachIn( a2dViewTemplateList, &list )
            {
                a2dViewTemplateList::value_type temp = *iter;
                if ( strings[i] == temp->GetViewTypeName() )
                {
                    templates[i] = temp;
                }
            }
        }
    }

    a2dViewTemplate* theTemplate;

    switch ( n )
    {
        case 0:
            // no visible templates, hence nothing to choose from
            theTemplate = ( a2dViewTemplate* )NULL;
            break;

        case 1:
            // don't propose the user to choose if he has no choice
            theTemplate = templates[0];
            break;

        default:
            // propose the user to choose one of several
            theTemplate = ( a2dViewTemplate* )wxGetSingleChoiceData
                          (
                              wxString::Format( _( "Select a document view for the file %s" ), doc->GetFilename().GetFullName().c_str() ),
                              _( "Views" ),
                              strings,
                              ( void** )templates,
                              wxFindSuitableParent()
                          );

    }

    delete[] templates;
    return theTemplate;
}

void a2dDocumentCommandProcessor::AssociateDocTemplate( a2dDocumentTemplate* temp )
{
    if ( m_docTemplates.end() == m_docTemplates.Find( temp ) )
    {
        m_docTemplates.push_back( temp );
    }
}

void a2dDocumentCommandProcessor::DisassociateDocTemplate( a2dDocumentTemplate* temp )
{
    m_docTemplates.ReleaseObject( temp );
}

void a2dDocumentCommandProcessor::AssociateViewTemplate( a2dViewTemplate* temp )
{
    if ( m_viewTemplates.end() == m_viewTemplates.Find( temp ) )
    {
        m_viewTemplates.push_back( temp );
    }
}

void a2dDocumentCommandProcessor::DisassociateViewTemplate( a2dViewTemplate* temp )
{
    m_viewTemplates.ReleaseObject( temp );
}

// Add and remove a document from the manager's list
void a2dDocumentCommandProcessor::AddDocument( a2dDocument* doc )
{
    if ( m_docs.end() == m_docs.Find( doc ) )
    {
        m_docs.push_back( a2dDocumentPtr( doc ) );
        SetCurrentDocument( doc );
        a2dCommandEvent event( wxEVT_ADD_DOCUMENT, doc );
        event.SetEventObject( this );
        ProcessEvent( event );
    }
    else
        SetCurrentDocument( doc );
}

void a2dDocumentCommandProcessor::RemoveDocument( a2dDocument* doc )
{
    if ( m_currentDocument == doc )
        SetCurrentDocument( ( a2dDocument* ) NULL );

    if ( m_docs.ReleaseObject( doc ) )
    {
        a2dCommandEvent event( wxEVT_REMOVE_DOCUMENT, doc );
        event.SetEventObject( this );
        ProcessEvent( event );
    }
}

// Views or windows should inform the document manager
// when a view is going in or out of focus
void a2dDocumentCommandProcessor::SetCurrentView( a2dView* view )
{
    // SetActiveView() disables the current active view here, but in that event we are not
    // interested.
    static bool recur = false;
    if ( recur )
        return;
    recur = true;

    if ( m_currentView && m_currentView->IsClosed() )
        m_currentView = NULL;

    // Events from views are connected here, the view itself stays unaware of a2dDocviewGlobals->GetDocviewCommandProcessor().
    // Calling twice is not a problem, only one connection is made.
    // Disconnecting these events is done in a2dView::~a2dView()

    //deactivate the current view if different from the new one.
    if ( m_currentView )
    {
        if ( m_currentView != view )
        {
            if ( m_currentView->GetActive() ) // only one active at the time, so deactivate the old view.
                m_currentView->Activate( false );
            if ( view )
            {
                // now this is current
                m_currentView = view;
                m_currentView->ConnectEvent(  wxEVT_CLOSE_VIEW, this );
                //m_currentView->ConnectEvent(  wxEVT_ACTIVATE_VIEW, this );
                m_currentView->ConnectEvent(  wxEVT_ENABLE_VIEW, this );
                //m_currentView->ConnectEvent(  wxEVT_UPDATE_VIEWS, this ); //if wanted, connect directly to specific document
                m_currentView->ConnectEvent(  wxEVT_ENABLE_VIEWS, this );
                m_currentView->ConnectEvent(  wxEVT_CHANGEDFILENAME_DOCUMENT, this );
                m_currentView->ConnectEvent(  wxEVT_CHANGEDTITLE_DOCUMENT, this );
                m_currentView->ConnectEvent(  wxEVT_REPORT_VIEWS, this );
                m_currentView->ConnectEvent(  wxEVT_DISCONNECT_ALLVIEWS, this );
            }
            //else we keep the last activated view
        }
    }
    else if ( view )
    {
        // only if new view is active
        m_currentView = view;
        m_currentView->ConnectEvent(  wxEVT_CLOSE_VIEW, this );
        //m_currentView->ConnectEvent(  wxEVT_ACTIVATE_VIEW, this );
        m_currentView->ConnectEvent(  wxEVT_ENABLE_VIEW, this );
        //m_currentView->ConnectEvent(  wxEVT_UPDATE_VIEWS, this ); //if wanted, connect directly to specific document
        m_currentView->ConnectEvent(  wxEVT_ENABLE_VIEWS, this );
        m_currentView->ConnectEvent(  wxEVT_CHANGEDFILENAME_DOCUMENT, this );
        m_currentView->ConnectEvent(  wxEVT_CHANGEDTITLE_DOCUMENT, this );
        m_currentView->ConnectEvent(  wxEVT_REPORT_VIEWS, this );
        m_currentView->ConnectEvent(  wxEVT_DISCONNECT_ALLVIEWS, this );
    }

    if ( m_currentView )
		SetCurrentDocument( m_currentView->GetDocument() );

    recur = false;
}

void a2dDocumentCommandProcessor::CheckCurrentView( a2dView* view )
{
    if( !view )
        m_currentView = NULL;
    else
    {
        if( m_currentView == view )
            m_currentView = NULL;
    }
}

void a2dDocumentCommandProcessor::SetCurrentDocument( a2dDocument* document )
{
    bool changed = m_currentDocument != document;
    //wxLogDebug("Proc %p Doc old %p new  %p\n", this, m_currentDocument, document );

    m_currentDocument = document;
    // KLAAS next changed, since this function should do what it says.
    // sent changed current document event only if it is NULL or added to a2dDocumentCommandProcessor
    // if ( !document || m_docs.end() != m_docs.Find(document))
    if ( !document || changed )
    {
        if ( changed )
        {
            a2dCommandEvent event( wxEVT_CHANGED_DOCUMENT, m_currentDocument );
            event.SetEventObject( this );
            ProcessEvent( event );
        }
    }
}

void a2dDocumentCommandProcessor::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
    Undo();
}

void a2dDocumentCommandProcessor::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
    Redo();
}

// Handlers for UI update commands

void a2dDocumentCommandProcessor::OnUpdateFileOpen( wxUpdateUIEvent& event )
{
    event.Enable( true );
}

void a2dDocumentCommandProcessor::OnUpdateFileClose( wxUpdateUIEvent& event )
{
    a2dDocument* doc = GetCurrentDocument();
    event.Enable( ( doc != ( a2dDocument* ) NULL ) );
}

void a2dDocumentCommandProcessor::OnUpdateFileCloseAll( wxUpdateUIEvent& event )
{
    event.Enable( m_docs.size() != 0 );
}

void a2dDocumentCommandProcessor::OnUpdateFileRevert( wxUpdateUIEvent& event )
{
    a2dDocument* doc = GetCurrentDocument();
    event.Enable( ( doc != ( a2dDocument* ) NULL ) );
}

void a2dDocumentCommandProcessor::OnUpdateFileNew( wxUpdateUIEvent& event )
{
    event.Enable( true );
}

void a2dDocumentCommandProcessor::OnUpdateFileSave( wxUpdateUIEvent& event )
{
    a2dDocument* doc = GetCurrentDocument();
    event.Enable( doc && doc->IsModified() );
}

void a2dDocumentCommandProcessor::OnUpdateFileSaveAll( wxUpdateUIEvent& event )
{
    a2dDocumentList::iterator iter = m_docs.begin();
    while( iter != m_docs.end() )
    {
        a2dDECLARE_LOCAL_ITEM( a2dDocumentList::value_type, doc, *iter );
        if ( doc && doc->IsModified() )
        {
            event.Enable( true );
            return;
        }
        iter++;
    }
    event.Enable( false );
}

void a2dDocumentCommandProcessor::OnUpdateFileSaveAs( wxUpdateUIEvent& event )
{
    a2dDocument* doc = GetCurrentDocument();
    event.Enable( ( doc != ( a2dDocument* ) NULL ) );
}

void a2dDocumentCommandProcessor::OnUpdateCreateView( wxUpdateUIEvent& event )
{
    a2dDocument* doc = GetCurrentDocument();
    event.Enable( ( doc != ( a2dDocument* ) NULL ) );
}

void a2dDocumentCommandProcessor::OnUpdateUndo( wxUpdateUIEvent& event )
{
    a2dDocument* doc = GetCurrentDocument();
    event.Enable( ( doc && doc->GetCommandProcessor() && doc->GetCommandProcessor()->CanUndo() ) );
}

void a2dDocumentCommandProcessor::OnUpdateRedo( wxUpdateUIEvent& event )
{
    a2dDocument* doc = GetCurrentDocument();
    event.Enable( ( doc && doc->GetCommandProcessor() && doc->GetCommandProcessor()->CanRedo() ) );
}

void a2dDocumentCommandProcessor::OnUpdatePrint( wxUpdateUIEvent& event )
{
    a2dDocument* doc = GetCurrentDocument();
    event.Enable( ( doc != ( a2dDocument* ) NULL ) );
}

void a2dDocumentCommandProcessor::OnUpdatePreview( wxUpdateUIEvent& event )
{
    a2dDocument* doc = GetCurrentDocument();
    event.Enable( ( doc != ( a2dDocument* ) NULL ) );
}

// Views or windows should inform the document manager
// when a view is going in or out of focus
void a2dDocumentCommandProcessor::OnActivateView( a2dViewEvent& viewevent )
{
    a2dView* view = ( a2dView* ) viewevent.GetEventObject();
    SetCurrentView( view );
    viewevent.Skip();
}

//It is called when a2dView::SetDocument() executing
void a2dDocumentCommandProcessor::OnAddView( a2dDocumentEvent& docevent )
{
    a2dView* view = ( a2dView* ) docevent.GetEventObject();
    if( docevent.GetEnable() )
        SetCurrentView( view );
}

void a2dDocumentCommandProcessor::OnRemoveView( a2dDocumentEvent& viewevent )
{
    a2dView* view = ( a2dView* ) viewevent.GetEventObject();

    if ( view == m_currentView )
        SetCurrentView( NULL );
}

bool a2dDocumentCommandProcessor::ProcessEvent( wxEvent& event )
{
    /* klion: all it is not need for wxDocview, it was my mistake
              because from a2dCommandProcessor::SetMenuStrings we can't check for a2dCommandProcessorEvent
              so menus wrong update
        static wxEvent* lastProcessedEvent = NULL; // for any events
        static int  lastProcessedEventId = -1;     // for wxIdleEvent and wxUpdateUIEvent
        static bool lastProcessedRetCode = false;  // return code last event
        static long lastProcessedEventTimeStamp = -1; // for a2d...Events
        static wxObject* lastProcessedEventObject = NULL; // for any wxWidgets events (menu toolbars)
        static wxEventType lastProcessedEventType = wxEVT_NULL;

        if(&event != lastProcessedEvent || lastProcessedEventId != event.GetId()
        || lastProcessedEventObject != event.GetEventObject() || lastProcessedEventType != event.GetEventType()
        || lastProcessedEventTimeStamp != event.GetTimestamp())
        {
    //        if(event.GetEventType() != wxEVT_UPDATE_UI) {
    //            wxLogDebug(wxT("Stack dump:\n%s"), a2dGetStackTrace(1, true, 20, 10).c_str());
            lastProcessedRetCode = a2dCommandProcessor::ProcessEvent(event);
            lastProcessedEventId = event.GetId();
            lastProcessedEvent = &event;
            lastProcessedEventObject = event.GetEventObject();
            lastProcessedEventType = event.GetEventType();
            lastProcessedEventTimeStamp = event.GetTimestamp();
        }
        else
        {
    //        wxLogDebug(wxT("Stack dump:\n%s"), a2dGetStackTrace(1, true, 20, 10).c_str());
        }
        return lastProcessedRetCode;
    */
    return a2dCommandProcessor::ProcessEvent( event );
}

// ----------------------------------------------------------------------------
// a2dDocviewGlobal
// ----------------------------------------------------------------------------

//! a global pointer to get to global instance of important classes.
a2dDocviewGlobal* a2dDocviewGlobals = NULL;

a2dDocviewGlobal::a2dDocviewGlobal()
{
    m_docviewCommandProcessor = new a2dDocumentCommandProcessor();

    m_directlog = true;
}

void a2dDocviewGlobal::SetDocviewCommandProcessor( a2dDocumentCommandProcessor* docviewCommandProcessor )
{
    if ( docviewCommandProcessor )
    docviewCommandProcessor->CheckCurrentView( NULL );
    m_docviewCommandProcessor = docviewCommandProcessor;
}

a2dDocviewGlobal::~a2dDocviewGlobal()
{
}

void a2dDocviewGlobal::RecordF( wxObject* sender, const wxChar* Format, ... )
{
    va_list ap;

    wxString recordstring;
    va_start( ap, Format );

    //Format.Replace( wxT("%f"), wxT("%6.3f") );

    recordstring.PrintfV( Format, ap );
    va_end( ap );

    a2dCommandEvent event( recordstring );
    event.SetEventObject( sender );

    ProcessEvent( event );
}

void a2dDocviewGlobal::RecordF( const wxChar* Format, ... )
{
    va_list ap;

    wxString recordstring;
    va_start( ap, Format );

    //Format.Replace( wxT("%f"), wxT("%6.3f") );

    recordstring.PrintfV( Format, ap );
    va_end( ap );

    a2dCommandEvent event( recordstring );
    event.SetEventObject( this );

    ProcessEvent( event );
}

// ----------------------------------------------------------------------------
// a2dFileHistory
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dFileHistory, wxObject )

// ----------------------------------------------------------------------------
// local constants
// ----------------------------------------------------------------------------

static const wxChar* s_MRUEntryFormat = wxT( "&%hd %s" );


static inline wxChar* MYcopystring( const wxString& s )
{
    wxChar* copy = new wxChar[s.length() + 1];
    return wxStrcpy( copy, s.c_str() );
}

static inline wxChar* MYcopystring( const wxChar* s )
{
    wxChar* copy = new wxChar[wxStrlen( s ) + 1];
    return wxStrcpy( copy, s );
}

a2dFileHistory::a2dFileHistory( size_t maxFiles, wxWindowID idBase )
{
    m_fileMaxFiles = maxFiles;
    m_idBase = idBase;
}

a2dFileHistory::~a2dFileHistory()
{
}

// File history management
void a2dFileHistory::AddFileToHistory(  const wxFileName& file, a2dDocumentTemplate* docTemplate, a2dViewTemplate* viewTemplate )
{
    a2dFileHistoryItemList::iterator iter = m_fileHistoryList.begin();
    wxInt16 i = 0;
    for( ; iter != m_fileHistoryList.end(); iter++ )
    {
        a2dFileHistoryItemPtr fileitem = *iter;
        if ( fileitem->m_filename == file )
        {
            // we do have it, move it to the top of the history
            RemoveFileFromHistory ( i );
            //recursive call, but item is laready removed, so size-1 now.
            AddFileToHistory( file, docTemplate, viewTemplate );
            return;
        }
        i++;
    }

    // if we already have a full history, delete the one at the end
    if ( m_fileMaxFiles == m_fileHistoryList.size() )
    {
        RemoveFileFromHistory ( m_fileHistoryList.size() - 1 );
        AddFileToHistory( file, docTemplate, viewTemplate );
        return;
    }

    // Add to the project file history:
    // Move existing files (if any) down so we can insert file at beginning.
    if ( m_fileHistoryList.size() < m_fileMaxFiles )
    {
        wxList::compatibility_iterator node = m_fileMenus.GetFirst();
        while ( node )
        {
            wxMenu* menu = ( wxMenu* ) node->GetData();
            if ( m_fileHistoryList.empty() && menu->GetMenuItemCount() )
            {
                menu->AppendSeparator();
            }
            // fill up with menu set to empty labels, will be set later
            menu->Append( m_idBase + m_fileHistoryList.size(), _( "[EMPTY]" ) );
            node = node->GetNext();
        }
    }
    m_fileHistoryList.push_front( new a2dFileHistoryItem( file, docTemplate, viewTemplate ) );

    // this is the directory of the last opened file
    iter = m_fileHistoryList.begin();
    wxString pathCurrent = ( *iter )->m_filename.GetPath();

    //refill menu from list
    i = 0;
    for( ; iter != m_fileHistoryList.end(); iter++ )
    {
        a2dFileHistoryItemPtr fileitem = *iter;

        // if in same directory just show the filename; otherwise the full
        // path
        wxString pathInMenu, path;
        path = fileitem->m_filename.GetPath();
        if ( path == pathCurrent )
            pathInMenu = fileitem->m_filename.GetFullName();
        else
        {
            // absolute path; could also set relative path
            pathInMenu = fileitem->m_filename.GetFullPath();
        }

        // we need to quote '&' characters which are used for mnemonics
        pathInMenu.Replace( _T( "&" ), _T( "&&" ) );
        wxString buf;
        buf.Printf( s_MRUEntryFormat, i + 1, pathInMenu.c_str() );
        wxList::compatibility_iterator node = m_fileMenus.GetFirst();
        while ( node )
        {
            wxMenu* menu = ( wxMenu* ) node->GetData();
            menu->SetLabel( m_idBase + i, buf );
            node = node->GetNext();
        }
        i++;
    }
}

void a2dFileHistory::RemoveFileFromHistory( size_t i )
{
    wxCHECK_RET( i < m_fileHistoryList.size(),
                 wxT( "invalid index in a2dFileHistory::RemoveFileFromHistory" ) );

    // delete the element from the list
    a2dFileHistoryItemList::iterator iter = m_fileHistoryList.begin();
    wxInt16 j = 0;
    for( ; iter != m_fileHistoryList.end(); iter++ )
    {
        a2dFileHistoryItemPtr fileitem = *iter;
        if ( j == i )
        {
            m_fileHistoryList.erase( iter );
            break;
        }
        j++;
    }

    wxList::compatibility_iterator node = m_fileMenus.GetFirst();
    while ( node )
    {
        wxMenu* menu = ( wxMenu* ) node->GetData();

        // re-fill menu labels
        wxString buf;
        a2dFileHistoryItemList::iterator iter = m_fileHistoryList.begin();
        wxInt16 j = 0;
        for( ; iter != m_fileHistoryList.end(); iter++ )
        {
            a2dFileHistoryItemPtr fileitem = *iter;
            buf.Printf( s_MRUEntryFormat, j + 1, fileitem->m_filename.GetFullPath().c_str() );
            menu->SetLabel( m_idBase + j, buf );
            j++;
        }

        node = node->GetNext();

        // delete the last menu item which is unused now
        wxWindowID lastItemId = m_idBase + wx_truncate_cast( wxWindowID, m_fileHistoryList.size() );
        if ( menu->FindItem( lastItemId ) )
        {
            menu->Delete( lastItemId );
        }

        // delete the last separator too if no more files are left
        if ( m_fileHistoryList.empty() )
        {
            wxMenuItemList::compatibility_iterator nodeLast = menu->GetMenuItems().GetLast();
            if ( nodeLast )
            {
                wxMenuItem* menuItem = nodeLast->GetData();
                if ( menuItem->IsSeparator() )
                {
                    menu->Delete( menuItem );
                }
                //else: should we search backwards for the last separator?
            }
            //else: menu is empty somehow
        }
    }
}

wxString a2dFileHistory::GetHistoryFile( size_t i ) const
{
    wxString s;
    if ( i < m_fileHistoryList.size() )
    {
        a2dFileHistoryItemList::const_iterator iter = m_fileHistoryList.item( i );
        a2dFileHistoryItemPtr fileitem = *iter;
        s = fileitem->m_filename.GetFullPath();
    }
    else
        wxFAIL_MSG( wxT( "bad index in a2dFileHistory::GetHistoryFile" ) );
    return s;
}

a2dFileHistoryItem* a2dFileHistory::GetHistoryFileItem( size_t i ) const
{
    wxString s;
    if ( i < m_fileHistoryList.size() )
    {
        a2dFileHistoryItemList::const_iterator iter = m_fileHistoryList.item( i );
        return *iter;
    }
    else
        wxFAIL_MSG( wxT( "bad index in a2dFileHistory::GetHistoryFile" ) );
    return NULL;
}

void a2dFileHistory::UseMenu( wxMenu* menu )
{
    if ( !m_fileMenus.Member( menu ) )
        m_fileMenus.Append( menu );
}

void a2dFileHistory::RemoveMenu( wxMenu* menu )
{
    m_fileMenus.DeleteObject( menu );
}

#if wxUSE_CONFIG
void a2dFileHistory::Load( wxConfigBase& config )
{
    wxString buf;
    buf.Printf( wxT( "file%d" ), 1 );
    wxString historyFile;
    int fileid = 2;
    while ( ( m_fileHistoryList.size() < m_fileMaxFiles ) && config.Read( buf, &historyFile ) && ( !historyFile.empty() ) )
    {
        bool has = false;
        a2dFileHistoryItemList::iterator iter = m_fileHistoryList.begin();
        for( ; iter != m_fileHistoryList.end(); iter++ )
        {
            a2dFileHistoryItemPtr fileitem = *iter;
            if ( fileitem->m_filename.GetFullPath() == historyFile )
                has = true;
        }
        if ( !has )
            m_fileHistoryList.push_back(  new a2dFileHistoryItem( wxFileName( historyFile ), NULL, NULL ) );
        buf.Printf( wxT( "file%d" ), fileid++ );
        historyFile = wxEmptyString;
    }
    AddFilesToMenu();
}

void a2dFileHistory::Save( wxConfigBase& config )
{
    a2dFileHistoryItemList::iterator iter = m_fileHistoryList.begin();
    size_t j = 0;
    for( ; iter != m_fileHistoryList.end(); iter++ )
    {
        a2dFileHistoryItemPtr fileitem = *iter;
        wxString buf;
        buf.Printf( wxT( "file%d" ), ( int )j + 1 );
        if ( j < m_fileHistoryList.size() )
            config.Write( buf, fileitem->m_filename.GetFullPath() );
        else
            config.Write( buf, wxEmptyString );
        j++;
    }
}
#endif // wxUSE_CONFIG

void a2dFileHistory::AddFilesToMenu()
{
    if ( !m_fileHistoryList.empty() )
    {
        wxList::compatibility_iterator node = m_fileMenus.GetFirst();
        while ( node )
        {
            wxMenu* menu = ( wxMenu* ) node->GetData();
            if ( menu->GetMenuItemCount() )
            {
                menu->AppendSeparator();
            }

            a2dFileHistoryItemList::iterator iter = m_fileHistoryList.begin();
            wxInt16 i = 0;
            for( ; iter != m_fileHistoryList.end(); iter++ )
            {
                a2dFileHistoryItemPtr fileitem = *iter;
                wxString buf;
                buf.Printf( s_MRUEntryFormat, i + 1, fileitem->m_filename.GetFullPath().c_str() );
                menu->Append( m_idBase + i, buf );
                i++;
            }
            node = node->GetNext();
        }
    }
}

void a2dFileHistory::AddFilesToMenu( wxMenu* menu )
{
    if ( !m_fileHistoryList.empty() )
    {
        if ( menu->GetMenuItemCount() )
        {
            menu->AppendSeparator();
        }

        a2dFileHistoryItemList::iterator iter = m_fileHistoryList.begin();
        wxInt16 i = 0;
        for( ; iter != m_fileHistoryList.end(); iter++ )
        {
            a2dFileHistoryItemPtr fileitem = *iter;
            wxString buf;
            buf.Printf( s_MRUEntryFormat, i + 1, fileitem->m_filename.GetFullPath().c_str() );
            menu->Append( m_idBase + i, buf );
            i++;
        }
    }
}


// ----------------------------------------------------------------------------
// a2dDocviewModule
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dDocviewModule, wxModule )

bool a2dDocviewModule::OnInit()
{
#if wxUSE_PRINTING_ARCHITECTURE
    if( wxThePrintPaperDatabase == NULL )
    {
        wxThePrintPaperDatabase = new wxPrintPaperDatabase;
        wxThePrintPaperDatabase->CreateDatabase();
        m_wxThePrintPaperDatabase = wxThePrintPaperDatabase;
    }
#endif
    a2dDocviewGlobals = new a2dDocviewGlobal();
    wxNullRefObjectList = new a2dObjectList();

    return true;
}

void a2dDocviewModule::OnExit()
{
    delete wxNullRefObjectList;
    delete a2dDocviewGlobals;
    a2dDocviewGlobals = NULL;
#if wxUSE_PRINTING_ARCHITECTURE
    if( m_wxThePrintPaperDatabase && m_wxThePrintPaperDatabase != wxThePrintPaperDatabase )
        delete m_wxThePrintPaperDatabase;
    m_wxThePrintPaperDatabase = NULL;
#endif
}



