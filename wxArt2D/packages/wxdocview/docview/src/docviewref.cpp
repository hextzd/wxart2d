/*! \file docview/src/docviewref.cpp
    \brief Document/view classes
    \author Julian Smart
    \date Created 01/02/97

    Copyright: (c) Julian Smart and Markus Holzem

    Licence: wxWidgets licence

    Modified by: Klaas Holwerda

    RCS-ID: $Id: docviewref.cpp,v 1.191 2009/10/01 19:22:35 titato Exp $
*/

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

//#if wxUSE_DOC_VIEW_ARCHITECTUR

#ifndef WX_PRECOMP
#include "wx/wx.h"
/*
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
*/
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
#include "wx/tokenzr.h"
#include <wx/xrc/xmlres.h>

#include <stdio.h>
#include <string.h>

#include "wx/docview.h"

// ----------------------------------------------------------------------------
// template instantiations
// ----------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
#include "wx/docview/doccom.h"

#if wxUSE_STD_IOSTREAM
#include "wx/ioswrap.h"
#if wxUSE_IOSTREAMH
#include <fstream.h>
#else
#include <fstream>
#endif
#else
#include "wx/wfstream.h"
#endif


//#include "wx/general/id.inl"
#include "wx/general/smrtptr.inl"

//----------------------------------------------------------------------------
// template instanitiations
//----------------------------------------------------------------------------

// MSVC warning 4660 is quite stupid. It says that the template is already instantiated
// by using it, but it is not fully instantiated as required for a library
#ifdef _MSC_VER
#pragma warning(disable: 4660)
#endif

//#if defined(WXDOCVIEW_USINGDLL) || defined( __UNIX__ )
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dView>;
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dDocument>;
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dIOHandler>;
template class A2DDOCVIEWDLLEXP a2dlist<class a2dSmrtPtr<class a2dDocument> >;
template class A2DDOCVIEWDLLEXP a2dSmrtPtrList<a2dDocument>;
template class A2DDOCVIEWDLLEXP a2dlist<class a2dSmrtPtr<class a2dView> >;
template class A2DDOCVIEWDLLEXP a2dSmrtPtrList<a2dView>;
template class A2DDOCVIEWDLLEXP a2dlist<class a2dSmrtPtr<class a2dDocumentTemplate> >;
template class A2DDOCVIEWDLLEXP a2dSmrtPtrList<a2dDocumentTemplate>;
template class A2DDOCVIEWDLLEXP a2dlist<class a2dSmrtPtr<class a2dViewTemplate> >;
template class A2DDOCVIEWDLLEXP a2dSmrtPtrList<a2dViewTemplate>;
//#endif

#ifdef _MSC_VER
#pragma warning(default: 4660)
#endif

// ----------------------------------------------------------------------------
// wxWindows macros
// ----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS( a2dDocument, a2dObject )
IMPLEMENT_ABSTRACT_CLASS( a2dView, a2dObject )
IMPLEMENT_CLASS( a2dDocumentTemplate, wxObject )
IMPLEMENT_CLASS( a2dDocumentTemplateAuto, a2dDocumentTemplate )
IMPLEMENT_CLASS( a2dViewTemplate, wxObject )

IMPLEMENT_ABSTRACT_CLASS( a2dViewConnector, a2dObject )
IMPLEMENT_ABSTRACT_CLASS( a2dFrameViewConnector, a2dViewConnector )

IMPLEMENT_CLASS( a2dCloseViewEvent, wxEvent )
IMPLEMENT_CLASS( a2dDocumentEvent, wxEvent )
IMPLEMENT_CLASS( a2dViewEvent, wxEvent )
IMPLEMENT_CLASS( a2dTemplateEvent, wxEvent )

IMPLEMENT_CLASS( a2dCloseDocumentEvent, wxCloseEvent )

DEFINE_EVENT_TYPE( wxEVT_SIGNAL )
DEFINE_EVENT_TYPE( wxEVT_DISCONNECT_ALLVIEWS )
DEFINE_EVENT_TYPE( wxEVT_SAVE_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_SAVEAS_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_EXPORT_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_IMPORT_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_CLOSE_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_OPEN_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_NEW_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_CREATE_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_ADD_VIEW )
DEFINE_EVENT_TYPE( wxEVT_REMOVE_VIEW )
DEFINE_EVENT_TYPE( wxEVT_UPDATE_VIEWS )
DEFINE_EVENT_TYPE( wxEVT_ENABLE_VIEWS )
DEFINE_EVENT_TYPE( wxEVT_DISCONNECT_VIEW )

DEFINE_EVENT_TYPE( wxEVT_REPORT_VIEWS )
DEFINE_EVENT_TYPE( wxEVT_CHANGEDFILENAME_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_CHANGEDTITLE_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_CHANGEDMODIFY_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_POST_LOAD_DOCUMENT )

DEFINE_EVENT_TYPE( wxEVT_CLOSE_VIEW )
DEFINE_EVENT_TYPE( wxEVT_CREATE_VIEW )
DEFINE_EVENT_TYPE( wxEVT_SET_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_ACTIVATE_VIEW )
DEFINE_EVENT_TYPE( wxEVT_ACTIVATE_VIEW_SENT_FROM_CHILD )
DEFINE_EVENT_TYPE( wxEVT_ENABLE_VIEW )
DEFINE_EVENT_TYPE( wxEVT_CHANGEDFILENAME_VIEW )
DEFINE_EVENT_TYPE( wxEVT_CHANGEDTITLE_VIEW )

DEFINE_EVENT_TYPE( wxEVT_PRE_ADD_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_POST_CREATE_DOCUMENT )
DEFINE_EVENT_TYPE( wxEVT_POST_CREATE_VIEW )

#if wxUSE_PRINTING_ARCHITECTURE
IMPLEMENT_DYNAMIC_CLASS( a2dDocumentPrintout, wxPrintout )
#endif

// ----------------------------------------------------------------------------
// function prototypes
// ----------------------------------------------------------------------------

static inline wxString FindExtension( const wxChar* path );

// ----------------------------------------------------------------------------
// local constants
// ----------------------------------------------------------------------------

const a2dPrintWhat a2dPRINT_Print = 1; //!< general print
const a2dPrintWhat a2dPRINT_Preview = 2;           //!<  general print preview
const a2dPrintWhat a2dPRINT_PrintView = 3;         //!< print a2dView
const a2dPrintWhat a2dPRINT_PreviewView = 4;       //!< preview print a2dView
const a2dPrintWhat a2dPRINT_PrintDocument = 5;     //!< print a2dDocument
const a2dPrintWhat a2dPRINT_PreviewDocument = 6;   //!< preview print a2dDocument
const a2dPrintWhat a2dPRINT_PrintSetup = 9;

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// local functions
// ----------------------------------------------------------------------------

static wxString FindExtension( const wxChar* path )
{
    wxString ext;
    wxFileName::SplitPath( path, NULL, NULL, &ext );

#ifdef __WXMSW__
    // extension is shown in case, so why not use it that way!
    //return ext.MakeLower();
    return ext;
#else
    return ext;
#endif
}

// ----------------------------------------------------------------------------
// class a2dTemplateFlag
// ----------------------------------------------------------------------------

const a2dTemplateFlagMask a2dTemplateFlag::NON        = 0x00000000;  /*!< no flags set */
const a2dTemplateFlagMask a2dTemplateFlag::VISIBLE    = 0x00000001;  /*!< is the template visible */
const a2dTemplateFlagMask a2dTemplateFlag::MARK       = 0x00000002;  /*!< for general use to temporary mark a template*/
const a2dTemplateFlagMask a2dTemplateFlag::LOAD       = 0x00000004;  /*!< a template for loading files*/
const a2dTemplateFlagMask a2dTemplateFlag::SAVE       = 0x00000008;  /*!< a template for saving  files*/
const a2dTemplateFlagMask a2dTemplateFlag::IMPORTING  = 0x00000010;  /*!< a template for importing files*/
const a2dTemplateFlagMask a2dTemplateFlag::EXPORTING  = 0x00000020;  /*!< a template for exporting files*/
const a2dTemplateFlagMask a2dTemplateFlag::ALL     = 0x00000040;  /*!< all flags set */
const a2dTemplateFlagMask a2dTemplateFlag::DEFAULT = a2dTemplateFlag::VISIBLE |
        a2dTemplateFlag::LOAD | a2dTemplateFlag::SAVE | a2dTemplateFlag::IMPORTING | a2dTemplateFlag::EXPORTING;  /*!< default */
const a2dTemplateFlagMask a2dTemplateFlag::LOAD_IMPORT = a2dTemplateFlag::VISIBLE |
        a2dTemplateFlag::LOAD | a2dTemplateFlag::IMPORTING;  /*!< for load + import */
const a2dTemplateFlagMask a2dTemplateFlag::SAVE_EXPORT = a2dTemplateFlag::VISIBLE |
        a2dTemplateFlag::SAVE | a2dTemplateFlag::EXPORTING;  /*!< for save + export */

a2dTemplateFlag::a2dTemplateFlag( a2dTemplateFlagMask newmask )
{
    SetFlags( newmask );
}

bool a2dTemplateFlag::CheckMask( a2dTemplateFlagMask mask )
{
    if ( mask == a2dTemplateFlag::NON )
        return true;
    if ( mask == a2dTemplateFlag::ALL )
        return true;

    //check the bits in the mask, is set, the corresponding bit in m_flags must be true.
    if ( 0 < ( mask & a2dTemplateFlag::VISIBLE )                 && !m_visible ) return false;
    if ( 0 < ( mask & a2dTemplateFlag::MARK )                    && !m_mark ) return false;
    if ( 0 < ( mask & a2dTemplateFlag::LOAD )                    && !m_load ) return false;
    if ( 0 < ( mask & a2dTemplateFlag::SAVE )                    && !m_save ) return false;
    if ( 0 < ( mask & a2dTemplateFlag::IMPORTING )               && !m_import ) return false;
    if ( 0 < ( mask & a2dTemplateFlag::EXPORTING )               && !m_export ) return false;

    return true;
}

void a2dTemplateFlag::SetFlags( a2dTemplateFlagMask which )
{
    if ( which == a2dTemplateFlag::ALL )
    {
        m_visible = true;
        m_mark = true;
        m_load = true;
        m_save = true;
        m_import = true;
        m_export = true;

        return;
    }

    m_visible =  0 < ( which & a2dTemplateFlag::VISIBLE );
    m_mark =  0 < ( which & a2dTemplateFlag::MARK );
    m_load =  0 < ( which & a2dTemplateFlag::LOAD );
    m_save =  0 < ( which & a2dTemplateFlag::SAVE );
    m_import =  0 < ( which & a2dTemplateFlag::IMPORTING );
    m_export =  0 < ( which & a2dTemplateFlag::EXPORTING );

}

a2dTemplateFlagMask a2dTemplateFlag::GetFlags() const
{
    a2dTemplateFlagMask oflags = a2dTemplateFlag::NON;

    if ( m_visible )   oflags = oflags | a2dTemplateFlag::VISIBLE;
    if ( m_mark )   oflags = oflags | a2dTemplateFlag::MARK;
    if ( m_load )   oflags = oflags | a2dTemplateFlag::LOAD;
    if ( m_save )   oflags = oflags | a2dTemplateFlag::SAVE;
    if ( m_import )   oflags = oflags | a2dTemplateFlag::IMPORTING;
    if ( m_export )   oflags = oflags | a2dTemplateFlag::EXPORTING;

    return oflags;
}

bool a2dTemplateFlag::GetFlag( a2dTemplateFlagMask which ) const
{
    switch ( which )
    {
        case a2dTemplateFlag::VISIBLE:        return m_visible;
        case a2dTemplateFlag::MARK:           return m_mark;
        case a2dTemplateFlag::LOAD:           return m_load;
        case a2dTemplateFlag::SAVE:           return m_save;
        case a2dTemplateFlag::IMPORTING:      return m_import;
        case a2dTemplateFlag::EXPORTING:      return m_export;
        default:
            wxFAIL_MSG( wxT( "This Flag not implemented" ) );
            return false;
    }
    return false;
}

// ----------------------------------------------------------------------------
// Definition of a2dDocument
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dDocument, a2dObject )
    EVT_CLOSE_VIEW( a2dDocument::OnCloseView )
    EVT_ADD_VIEW( a2dDocument::OnAddView )
    EVT_REMOVE_VIEW( a2dDocument::OnRemoveView )
    EVT_CLOSE_DOCUMENT( a2dDocument::OnCloseDocument )
    EVT_OPEN_DOCUMENT( a2dDocument::OnOpenDocument )
    EVT_SAVE_DOCUMENT( a2dDocument::OnSaveDocument )
    EVT_SAVEAS_DOCUMENT( a2dDocument::OnSaveAsDocument )
    EVT_EXPORT_DOCUMENT( a2dDocument::OnExportDocument )
    EVT_IMPORT_DOCUMENT( a2dDocument::OnImportDocument )
    EVT_NEW_DOCUMENT( a2dDocument::OnNewDocument )
    EVT_CREATE_DOCUMENT( a2dDocument::OnCreateDocument )
END_EVENT_TABLE()


a2dDocument::a2dDocument( a2dDocument* parent )
{
    m_askSaveWhenOwned = true;

    m_documentModified = 0;
    m_documentParent = parent;
    m_commandProcessor = ( a2dCommandProcessor* ) NULL;

    m_documentTitle = wxT( "" );

    m_documentTypeName = wxT( "" );
    m_documentFile = wxFileName();

    m_savedYet = false;
    m_isClosed = false;
    m_id = wxNewId();
    m_lastError = a2dError_NoError;
    m_modificationtime = wxDateTime::Now();
}

a2dDocument::a2dDocument( const a2dDocument& other )
    : a2dObject()
{
    m_askSaveWhenOwned = other.m_askSaveWhenOwned;

    m_documentFile = other.m_documentFile;
    m_documentTitle = other.m_documentTitle;
    m_documentTypeName = other.m_documentTypeName;
    m_documentModified = 0;
    m_documentParent = other.m_documentParent;
    m_commandProcessor = other.m_commandProcessor;
    m_savedYet = other.m_savedYet;
    m_isClosed = other.m_isClosed;
    m_id = wxNewId();
    m_lastError = a2dError_NoError;
    m_modificationtime = other.m_modificationtime;
}

void a2dDocument::DeleteContents()
{
    return;
}

a2dDocument::~a2dDocument()
{
    DisConnectAllViews();
    DeleteContents();
}

a2dObject* a2dDocument::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dDocument( *this );
}

a2dObject* a2dDocument::SmrtPtrOwn()
{
    m_refcount++;
    return this;
}

bool a2dDocument::SmrtPtrRelease()
{
    m_refcount--;

    if ( m_refcount <= 0  )
    {
        SaveModifiedDialog();
    }

    wxASSERT_MSG( m_refcount >= 0, wxT( "a2dDocument::Release Negative reference count" ) );
    if ( m_refcount <= 0 )
    {
        delete this;
        return true;
    }
    return false;
}

void a2dDocument::OnCloseView( a2dCloseViewEvent& event )
{
    //a2dView* viewtoclose = (a2dView*) event.GetEventObject();

    if ( m_refcount == 1 || m_askSaveWhenOwned ) //will become zero when view is allowed to close.
    {
        if ( SaveModifiedDialog() != wxCANCEL )//default ask user to save or not.
        {
            //if saved or not, that is what the users wants.
            //Still the view will be closed.
        }
        else if ( event.CanVeto() )
            event.Veto();
    }
}

void a2dDocument::OnRemoveView( a2dDocumentEvent& event )
{
    //a2dView* viewtoremove = (a2dView*) event.GetView();

    if ( m_refcount == 1 || m_askSaveWhenOwned ) //refcount will become zero
    {
        if ( SaveModifiedDialog() != wxCANCEL )//default ask user to save or not.
        {
            //if saved or not, that is what the users wants.
            //Still the view will be removed
        }
        else
            event.Veto();
    }
}

void a2dDocument::OnAddView( a2dDocumentEvent& event )
{
}

void a2dDocument::DisConnectAllViews()
{
    a2dDocumentEvent eventremoveall( wxEVT_DISCONNECT_ALLVIEWS, m_documentFile );
    eventremoveall.SetEventObject( this );

    ProcessEvent( eventremoveall );
}

bool a2dDocument::Close( bool force )
{
    //prevent recursive calls
    if ( m_isClosed )
        return true; //was already closed

    m_isClosed = true;

    a2dCloseDocumentEvent event( m_id );
    event.SetEventObject( this );
    event.SetCanVeto( !force );

    // return false if window wasn't closed because the application vetoed the
    // close event
    if ( ProcessEvent( event )  && !event.GetVeto() )
    {
        return true;
    }
    m_isClosed = false;
    return false;
}

void a2dDocument::OnCloseDocument( a2dCloseDocumentEvent& event )
{
    if ( SaveModifiedDialog( event.CanVeto() ) != wxCANCEL ) //default ask user to save or not.
    {
        //emptying a document while other views are using it, is dangerous.
        //views may refer or use object in the document.
        //Therefore only clear the contents when there or no more references to the document.
        //Remember a view also Owns a reference to a document, so certainly all views need to be gone.
        a2dREFOBJECTPTR_KEEPALIVE;

        DisConnectAllViews();
        DeleteContents();
        //undo stack can hold Owners to Documents etc.
		if ( GetCommandProcessor() )
            GetCommandProcessor()->ClearCommands();
        Modify( false );
    }
    else if ( event.CanVeto() )
        event.Veto();
    else
        wxFAIL_MSG( wxT( "You can not Cancel this Close Document event" ) );
}

void a2dDocument::OnNewDocument( a2dDocumentEvent& WXUNUSED( event ) )
{
    m_isClosed = false;
    Modify( false );
    SetDocumentSaved( false );

    wxString name = wxT( "" );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->MakeDefaultName( name );

    SetTitle( name );

    //set file name for views and document
    SetFilename( name, true );
}

bool a2dDocument::Save()
{
    a2dDocumentEvent event( wxEVT_SAVE_DOCUMENT, m_documentFile );
    event.SetFileName( m_documentFile );
    event.SetEventObject( this );

    if ( ProcessEvent( event )  && event.IsAllowed() )
    {
        return true;
    }
    return false;
}

bool a2dDocument::SaveAs( const wxFileName& fileTosaveTo, a2dDocumentFlagMask flags )
{
    wxString filename  = fileTosaveTo.GetFullPath();
    a2dDocumentEvent event( wxEVT_SAVEAS_DOCUMENT, filename.IsEmpty() ? m_documentFile : fileTosaveTo, flags );

    event.SetEventObject( this );

    if ( ProcessEvent( event )  && event.IsAllowed() )
    {
        return true;
    }
    return false;
}

void a2dDocument::OnSaveAsDocument( a2dDocumentEvent& event )
{
    a2dDocumentTemplate* docTemplate = GetDocumentTemplate();
    if ( ! docTemplate )
    {
        a2dDocviewGlobals->ReportError( a2dError_NoDocTemplateRef );
        event.Veto();
        event.SetError( a2dError_NoDocTemplateRef );
        return;
    }

    //wxString fullname = event.GetFileName().GetFullPath();
    wxString fullname = event.GetFileName().GetFullName();
    wxString filter = docTemplate->GetDescription()
                    + wxT( " (" ) << docTemplate->GetFileFilter() << wxT( ") |" )
                    << docTemplate->GetFileFilter(); 

    if ( !( event.GetFlags() & a2dREFDOC_SILENT ) )
    {
        fullname = wxFileSelector( _( "Save as" ),
                                   docTemplate->GetDirectory(),
                                   fullname,
                                   docTemplate->GetDefaultExtension(),
                                   filter, 
                                   wxFD_SAVE | wxFD_OVERWRITE_PROMPT,
                                   GetAssociatedWindow() );

        if ( fullname.IsEmpty() )
        {
            event.Veto();
            return;
        }
    }

    wxString ext;
    wxFileName::SplitPath( fullname, NULL, NULL, &ext );

    if ( ext.IsEmpty() )
    {
        fullname += wxT( "." );
        fullname += docTemplate->GetDefaultExtension();
    }

#if wxUSE_STD_IOSTREAM
    a2dDocumentFileOutputStream store( fullname.mb_str(), wxSTD ios_base::out | wxSTD ios_base::binary );
    if ( store.fail() || store.bad() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for saving" ), fullname.c_str() );
        // Saving error
        event.Veto();
        event.SetError( a2dError_FileCouldNotOpen );
        return;
    }
#else
    wxFileOutputStream storeUnbuf( fullname );
    if ( storeUnbuf.GetLastError() != wxSTREAM_NO_ERROR )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for saving" ), fullname.c_str() );
        // Saving error
        event.Veto();
        event.SetError( a2dError_FileCouldNotOpen );
        return;
    }
    wxBufferedOutputStream store( storeUnbuf );
#endif

    wxBusyCursor wait;
    m_lastError = a2dError_NoError;

    a2dIOHandlerStrOut* handler = docTemplate->GetDocumentIOHandlerForSave( fullname, this );
    if ( handler )
        handler->SetFileName( fullname );
    SaveObject( store, handler );

    if ( m_lastError != a2dError_NoError )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_SaveFile, _( "Sorry, could not save document to file %s" ), fullname.c_str() );
        // Saving error
        event.Veto();
        event.SetError( m_lastError );
        return;
    }
    Modify( false );

    // Inform events of changed filename, too
    SetFilename( fullname, true );
    SetDocumentSaved( true );
}

void a2dDocument::OnSaveDocument( a2dDocumentEvent& event )
{
    wxString filetosave = event.GetFileName().GetFullPath();

    if ( !IsModified() && m_savedYet )
    {
        event.Veto();
        return;
    }

    if ( filetosave.IsEmpty() || !m_savedYet )
    {
        if ( !SaveAs() )
            event.Veto();
        return;
    }

#if wxUSE_STD_IOSTREAM
    a2dDocumentFileOutputStream store( filetosave.mb_str(), wxSTD ios_base::out | wxSTD ios_base::binary );
    if ( store.fail() || store.bad() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for saving." ), filetosave.c_str() );
        // Saving error
        event.Veto();
        event.SetError( a2dError_FileCouldNotOpen );
        return;
    }
#else
    wxFileOutputStream storeUnbuf( filetosave );
    if ( storeUnbuf.GetLastError() != wxSTREAM_NO_ERROR )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for saving." ), filetosave.c_str() );
        // Saving error
        event.Veto();
        event.SetError( a2dError_FileCouldNotOpen );
        return;
    }
    wxBufferedOutputStream store( storeUnbuf );
#endif

    if ( ! GetDocumentTemplate() )
    {
        a2dDocviewGlobals->ReportError( a2dError_NoDocTemplateRef );
        event.Veto();
        event.SetError( a2dError_NoDocTemplateRef );
        return;
    }

    wxBusyCursor wait;
    m_lastError = a2dError_NoError;

    a2dIOHandlerStrOut* handler = m_documentTemplate->GetDocumentIOHandlerForSave( filetosave, this );
    if ( handler )
        handler->SetFileName( filetosave );
    SaveObject( store, handler );
    
    if ( m_lastError != a2dError_NoError )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_SaveFile, _( "Sorry, could not save document to file %s" ), filetosave.c_str() );
        // Saving error
        event.Veto();
        event.SetError( m_lastError );
        return;
    }
    Modify( false );
    SetFilename( filetosave );
    SetDocumentSaved( true );
}

bool a2dDocument::Export( a2dDocumentTemplate* doctemplate, const wxFileName& fileTosaveTo, a2dDocumentFlagMask flags )
{
    wxString filename  = fileTosaveTo.GetFullPath();
    a2dDocumentEvent event( wxEVT_EXPORT_DOCUMENT, doctemplate, filename.IsEmpty() ? m_documentFile : fileTosaveTo, flags );
    event.SetEventObject( this );

    if ( ProcessEvent( event )  && event.IsAllowed() )
    {
        return true;
    }
    return false;
}

void a2dDocument::OnExportDocument( a2dDocumentEvent& event )
{
    a2dDocumentTemplate* docTemplate = event.GetDocumentTemplate();
    wxString path = event.GetFileName().GetFullPath();
    if ( !docTemplate )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_NoDocTemplateRef, _( "Sorry, no document template for %s export." ), path.c_str() );
        // Export error
        event.Veto();
        event.SetError( a2dError_NoDocTemplateRef );
        return;
    }

    if ( path.IsEmpty() )
    {
        event.Veto();
        return;
    }

    wxBusyCursor wait;

    wxString fileName( path );
    wxString name, ext;
    wxFileName::SplitPath( fileName, & path, & name, & ext );

    if ( ext.IsEmpty() || ext == wxT( "" ) )
    {
        fileName += wxT( "." );
        fileName += docTemplate->GetDefaultExtension();
    }

#if wxUSE_STD_IOSTREAM
    a2dDocumentFileOutputStream store( fileName.mb_str(), wxSTD ios_base::out | wxSTD ios_base::binary );
    if ( store.fail() || store.bad() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for export." ), fileName.c_str() );
        // Export error
        event.Veto();
        event.SetError( a2dError_FileCouldNotOpen );
        return;
    }
#else
    wxFileOutputStream storeUnbuf( fileName );
    if ( storeUnbuf.GetLastError() != wxSTREAM_NO_ERROR )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for export." ), fileName.c_str() );
        // Export error
        event.Veto();
        event.SetError( a2dError_FileCouldNotOpen );
        return;
    }
    wxBufferedOutputStream store( storeUnbuf );
#endif
    m_lastError = a2dError_NoError;

    a2dIOHandlerStrOut* handler = docTemplate->GetDocumentIOHandlerForSave( fileName, this );
    if ( handler )
    {
        handler->SetFileName( fileName );
        SaveObject( store, handler );
    }
    else
        a2dDocviewGlobals->ReportErrorF( a2dError_ExportFile, _( "could not export document to file, no io handler %s" ), fileName.c_str() );
    if ( m_lastError != a2dError_NoError )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_ExportFile, _( "Sorry, could not export document to file %s" ), fileName.c_str() );
        // export error
        event.Veto();
        event.SetError( m_lastError );
        return;
    }
}

bool a2dDocument::Import( a2dDocumentTemplate* doctemplate, const wxFileName& fileToImport, a2dDocumentFlagMask flags )
{
    wxString filename  = fileToImport.GetFullPath();
    a2dDocumentEvent event( wxEVT_IMPORT_DOCUMENT, doctemplate, filename, flags );
    event.SetEventObject( this );

    if ( ProcessEvent( event )  && event.IsAllowed() )
    {
        return true;
    }
    return false;
}

void a2dDocument::OnImportDocument( a2dDocumentEvent& event )
{
    a2dDocumentTemplate* docTemplate = event.GetDocumentTemplate();

    wxFileName file = event.GetFileName();
    wxString path = file.GetFullPath();

    if ( !docTemplate )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_NoDocTemplateRef, _( "Sorry, no document template for %s import." ), path.c_str() );
        event.Veto();
        event.SetError( a2dError_NoDocTemplateRef );
        return;
    }

    if ( path.IsEmpty() )
    {
        event.Veto();
        event.SetError( a2dError_CouldNotEvaluatePath );
        return;
    }

    wxString fileName( path );
    wxString name, ext;
    wxFileName::SplitPath( fileName, & path, & name, & ext );

    if ( ext.IsEmpty() )
    {
        fileName += wxT( "." );
        fileName += docTemplate->GetDefaultExtension();
    }

    wxBusyCursor wait;

#if wxUSE_STD_IOSTREAM
    a2dDocumentFileInputStream store( fileName.mb_str(), wxSTD ios_base::in | wxSTD ios_base::binary );
    if ( store.fail() || store.bad() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for import" ), fileName.c_str() );
        event.Veto();
        event.SetError( a2dError_FileCouldNotOpen );
        return;
    }
#else
    wxFileInputStream storeUnbuf( fileName );
    if ( !storeUnbuf.Ok() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for import" ), fileName.c_str() );
        event.Veto();
        event.SetError( a2dError_FileCouldNotOpen );
        return;
    }
    wxBufferedInputStream store( storeUnbuf );
#endif

    a2dIOHandlerStrIn* handler = docTemplate->GetDocumentIOHandlerForLoad( store, this );

    m_lastError = a2dError_NoError;
    if ( handler )
        handler->SetFileName( path );

#if wxUSE_STD_IOSTREAM
    LoadObject( store, handler );
    if ( m_lastError != a2dError_NoError || ( !store && !store.eof() ) )
#else
    int res = LoadObject( store, handler ).GetLastError();
    if ( m_lastError != a2dError_NoError || ( ( res != wxSTREAM_NO_ERROR ) && ( res != wxSTREAM_EOF ) ) )
#endif
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_ExportFile, _( "template I/O handler not capable of importing this format from file %s" ), fileName.c_str() );
        event.Veto();
        event.SetError( m_lastError );
    }
    UpdateAllViews();
}

void a2dDocument::OnOpenDocument( a2dDocumentEvent& event )
{
    wxFileName file = event.GetFileName();

    if ( SaveModifiedDialog() == wxCANCEL )
    {
        event.Veto();
        event.SetError( a2dError_Canceled );
        return;
    }

#if wxUSE_STD_IOSTREAM
    a2dDocumentFileInputStream store( file.GetFullPath().mb_str(), wxSTD ios_base::in | wxSTD ios_base::binary );
    if ( store.fail() || store.bad() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for loading" ), file.GetFullPath().c_str() );
        event.Veto();
        event.SetError( a2dError_FileCouldNotOpen );

        return;
    }
#else
    wxFileInputStream storeUnbuf( file.GetFullPath() );
    if ( !storeUnbuf.Ok() )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for loading" ), file.GetFullPath().c_str() );
        event.Veto();
        event.SetError( a2dError_FileCouldNotOpen );
        return;
    }
    wxBufferedInputStream store( storeUnbuf );
#endif
    if ( ! m_documentTemplate->GetAutoZeroPtrList() )
    {
        a2dDocviewGlobals->ReportError( a2dError_NoDocTemplateRef );
        event.Veto();
        event.SetError( a2dError_NoDocTemplateRef );
        return;
    }

    a2dIOHandlerStrIn* handler = m_documentTemplate->GetDocumentIOHandlerForLoad( store, this );

    wxBusyCursor wait;

    m_lastError = a2dError_NoError;
    if ( handler )
        handler->SetFileName( file.GetFullPath() );

#if wxUSE_STD_IOSTREAM
    LoadObject( store, handler );
    if ( m_lastError != a2dError_NoError || ( !store && !store.eof() ) )
#else

    int res = LoadObject( store, handler ).GetLastError();
    if ( m_lastError != a2dError_NoError || ( ( res != wxSTREAM_NO_ERROR ) && ( res != wxSTREAM_EOF ) ) )
#endif
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not Load file %s" ), file.GetFullPath().c_str() );
        event.Veto();
        event.SetError( m_lastError );
    }
    SetFilename( file, true );
    Modify( false );
    m_savedYet = true;
    m_isClosed = false;

    UpdateAllViews();
}

a2dDocumentInputStream& a2dDocument::LoadObject( a2dDocumentInputStream& stream, a2dIOHandlerStrIn* handler )
{
#if wxUSE_STD_IOSTREAM
    int isOK = ( stream.fail() || stream.bad() ) == 0;
#else
    int isOK = stream.IsOk ();
#endif

    bool oke = false;
    if ( isOK )
    {
        if ( handler )
        {
            if ( handler->CanLoad( stream, this, m_documentTemplate ? m_documentTemplate->GetDocumentClassInfo() : NULL )  )
            {
                if ( !handler->Load( stream, this ) )
                {
                    a2dDocviewGlobals->ReportError( a2dError_CouldNotLoad );
                    m_lastError = a2dError_CouldNotLoad;
                }
                else
                {
                    oke = true;
                    a2dDocumentEvent event( wxEVT_POST_LOAD_DOCUMENT );
                    event.SetEventObject( this );
                    ProcessEvent( event );
                }
            }
            else
            {
                a2dDocviewGlobals->ReportError( a2dError_IOHandler );
                m_lastError = a2dError_IOHandler;
            }
        }
        else
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "I/O handler must be != NULL, maybe template I/O handler not available in document template." ) );
            m_lastError = a2dError_IOHandler;
        }
    }
    else
    {
        a2dDocviewGlobals->ReportError( a2dError_ImportObject );
        m_lastError = a2dError_ImportObject;
    }

    return stream;
}

#if wxART2D_USE_CVGIO
void a2dDocument::Load( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& parser )
{
    parser.Next();
    parser.Require( START_TAG, wxT( "cvg" ) );
    parser.Next();
    if ( parser.GetName() == wxT( "title" ) )
    {
        m_documentTitle = parser.GetContent();
        parser.Next();
        parser.Require( END_TAG, wxT( "title" ) );
        parser.Next();
    }

    parser.Require( END_TAG, wxT( "cvg" ) );
}

void a2dDocument::Save( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& out, a2dObjectList* WXUNUSED( towrite ) )
{
    out.WriteStartDocument( wxT( "1.0" ), wxT( "UTF-8" ) , true );

    out.WriteStartElementAttributes( wxT( "cvg" ) );
    out.WriteAttribute( wxT( "classname" ), GetClassInfo()->GetClassName() );
    out.WriteEndAttributes();

    out.WriteElement( wxT( "title" ), GetPrintableName() );

    out.WriteEndElement();
}


void a2dDocument::Save( a2dIOHandlerXmlSerOut& out, wxObject* WXUNUSED( start ) )
{
    out.WriteStartDocument( wxT( "1.0" ), wxT( "UTF-8" ) , true );

    out.WriteStartElementAttributes( wxT( "cvg" ) );
    out.WriteAttribute( wxT( "classname" ), GetClassInfo()->GetClassName() );
    out.WriteEndAttributes();

    out.WriteElement( wxT( "title" ), GetPrintableName() );

    out.WriteEndElement();
}

#endif //wxART2D_USE_CVGIO

a2dDocumentOutputStream& a2dDocument::SaveObject( a2dDocumentOutputStream& stream, a2dIOHandlerStrOut* handler )
{
#if wxUSE_STD_IOSTREAM
    int isOK = ( stream.fail() || stream.bad() ) == 0;
#else
    int isOK = stream.IsOk ();
#endif

    if ( isOK )
    {
        if ( handler )
        {
            if ( handler->CanSave( this )  )
            {
                if ( !handler->Save( stream, this ) )
                {
                    a2dDocviewGlobals->ReportError( a2dError_IOHandler );
                    m_lastError = a2dError_IOHandler;
                }
            }
            else
            {
                a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "I/O handler cannot save document" ) );
                m_lastError = a2dError_IOHandler;
            }
        }
        else
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "I/O handler must be != NULL, maybe not available in template" ) );
            m_lastError = a2dError_IOHandler;
        }
    }
    else
    {
        a2dDocviewGlobals->ReportError( a2dError_SaveObject );
        m_lastError = a2dError_SaveObject;
    }

    return stream;
}

bool a2dDocument::Revert()
{
    return false;
}


// Get title, or filename if no title, else unnamed
wxString a2dDocument::GetPrintableName() const
{
    if ( m_documentTitle != wxT( "" ) )
    {
        return m_documentTitle;
    }
    else if ( m_documentFile.GetFullPath() != wxT( "" ) )
    {
        return m_documentFile.GetFullPath();
    }
    else
    {
        return _( "unnamed" );
    }
}

wxWindow* a2dDocument::GetAssociatedWindow() const
{
    return wxTheApp->GetTopWindow();
}


void a2dDocument::CreateCommandProcessor()
{
    m_commandProcessor = new a2dCommandProcessor;
    // the next signals look like a good idea to always get from the document
    // its command processor.
    m_commandProcessor->ConnectEvent( wxEVT_MENUSTRINGS, this );
    m_commandProcessor->ConnectEvent( wxEVT_DO, this );
    m_commandProcessor->ConnectEvent( wxEVT_UNDO, this );
    m_commandProcessor->ConnectEvent( wxEVT_REDO, this );
}

int a2dDocument::SaveModifiedDialog( bool canCancel )
{
    if ( IsModified() )
    {
        wxString title = _( "unnamed" );

        if ( !m_documentFile.GetFullPath().IsEmpty() )
            title = m_documentFile.GetFullPath();

        wxString msgTitle;
        if ( wxTheApp->GetAppName() != wxT( "" ) )
            msgTitle = wxTheApp->GetAppName();
        else
            msgTitle = _( "Warning" );

        wxString prompt;
        prompt.Printf( _( "Do you want to save changes to document %s?" ),
                       ( const wxChar* )title );

        int res = wxMessageBox( prompt, msgTitle,
                                canCancel ? wxYES_NO | wxCANCEL | wxICON_QUESTION : wxYES_NO | wxICON_QUESTION,
                                GetAssociatedWindow() );

        if ( res == wxNO )
        {
            Modify( false );
            return wxNO;
        }
        else if ( res == wxYES )
        {
            if ( Save() )
                return wxYES;
            else
                return wxCANCEL;
        }
        else if ( res == wxCANCEL )
            return wxCANCEL;
    }
    return wxNO;
}

bool a2dDocument::Draw( wxDC& WXUNUSED( context ) )
{
    return true;
}

void a2dDocument::OnCreateDocument( a2dDocumentEvent& WXUNUSED( event ) )
{
}

void a2dDocument::Modify( bool mod )
{
    if ( mod )
        m_documentModified++;
    else
        m_documentModified = 0;
    a2dDocumentEvent event( wxEVT_CHANGEDMODIFY_DOCUMENT );
    event.SetEventObject( this );
    ProcessEvent( event );
}

void a2dDocument::ReportViews( a2dViewList* allviews )
{
    a2dDocumentEvent event( wxEVT_REPORT_VIEWS, allviews );
    event.SetEventObject( this );
    ProcessEvent( event );
}

void a2dDocument::UpdateAllViews( a2dView* sender, unsigned int hint, wxObject* objecthint )
{
    a2dDocumentEvent event( wxEVT_UPDATE_VIEWS, sender );
    event.SetEventObject( this );
    event.SetUpdateHint( hint );
    event.SetUpdateObject( objecthint );
    ProcessEvent( event );
}

void a2dDocument::EnableAllViews( bool enable, a2dView* sender )
{
    a2dDocumentEvent event( wxEVT_ENABLE_VIEWS, sender, enable );
    event.SetEventObject( this );
    ProcessEvent( event );
}

void a2dDocument::SetFilename( const wxFileName& filename, bool notifyViews )
{
    //if (m_documentFile != filename)
    {
        m_documentFile = filename;
        if ( notifyViews )
        {
            a2dDocumentEvent event( wxEVT_CHANGEDFILENAME_DOCUMENT, m_documentFile, ( a2dDocumentFlagMask ) a2dREFDOC_NON, m_id );
            event.SetEventObject( this );
            ProcessEvent( event );
        }
    }
}

void a2dDocument::SetTitle( const wxString& title, bool notifyViews )
{
    //if (m_documentTitle != title)
    {
        m_documentTitle = title;
        if ( notifyViews )
        {
            a2dDocumentEvent event( wxEVT_CHANGEDTITLE_DOCUMENT, m_documentFile, ( a2dDocumentFlagMask ) a2dREFDOC_NON, m_id );
            event.SetEventObject( this );
            ProcessEvent( event );
        }
    }
}

// ----------------------------------------------------------------------------
// Document view
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dView, a2dObject )
    EVT_CLOSE_VIEW( a2dView::OnCloseView )
    EVT_CHANGEDFILENAME_DOCUMENT( a2dView::OnChangeFilename )
    EVT_CHANGEDTITLE_DOCUMENT( a2dView::OnChangeTitle )
    EVT_ENABLE_VIEW( a2dView::OnEnable )
    EVT_UPDATE_VIEWS( a2dView::OnUpdate )
    EVT_ENABLE_VIEWS( a2dView::OnEnableViews )
    EVT_REPORT_VIEWS( a2dView::OnReport )
    EVT_DISCONNECT_ALLVIEWS( a2dView::OnDisConnectView )
END_EVENT_TABLE()

a2dView::a2dView()
{
    m_display = NULL;

    m_id = wxNewId();

    m_isClosed = false;

    m_viewDocument = ( a2dDocument* ) NULL;

    m_viewTypeName = wxT( "" );

    m_active = false;

    m_viewEnabled = true;

    //! the view owns itself until Close() or SetClosed()
    m_keepalive = this;
}

a2dView::a2dView( const a2dView& other )
    : a2dObject()
{
    m_isClosed = false;

    m_display = other.m_display;
    m_viewDocument = other.m_viewDocument;
    m_viewTypeName = other.m_viewTypeName;
    m_viewTemplate = other.m_viewTemplate;
    m_viewEnabled = other.m_viewEnabled;
    m_active = other.m_active;

    m_id = wxNewId();
}

a2dView::~a2dView()
{
    wxASSERT_MSG(  m_refcount == 0, wxT( "deleting a2dView while referenced" ) );
    if ( m_viewDocument )
        m_viewDocument->DisconnectEventAll( this );
    DisconnectEventAll( a2dDocviewGlobals->GetDocviewCommandProcessor() );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->CheckCurrentView( this );
}

a2dObject* a2dView::SmrtPtrOwn()
{
    m_refcount++;
    return this;
}

bool a2dView::SmrtPtrRelease()
{
    m_refcount--;

    wxASSERT_MSG( m_refcount >= 0, wxT( "a2dView::Release Negative reference count" ) );
    if ( m_refcount <= 0 )
    {
        delete this;
        return true;
    }
    return false;
}

void a2dView::SetDisplayWindow( wxWindow* display )
{
    if ( GetEvtHandlerEnabled() )
    {
        if ( m_display != display )
            Activate( false );
        m_display = display;
        if ( m_display )
            Activate( true );
    }
}

bool a2dView::Enable( bool enable )
{
    if ( enable != m_viewEnabled ) //only if needed.
    {
        bool restorestate = GetEvtHandlerEnabled();
        if ( m_display )
        {
            //sent event to inform the display window
            //( and if handled right, in all above since it travels up in wxWindow classes )
            a2dViewEvent event( wxEVT_ENABLE_VIEW, enable, m_id );
            event.SetEventObject( this );

            SetEvtHandlerEnabled( false ); //this event coming back from the display will not be handled
            event.ResumePropagation( wxEVENT_PROPAGATE_MAX );
            m_display->GetEventHandler()->ProcessEvent( event );
            event.StopPropagation();
        }

        //this event always will be processed
        SetEvtHandlerEnabled( true );
        if ( enable )
        {
            //inform view that it just was enabled or disabled
            a2dViewEvent Event( wxEVT_ENABLE_VIEW, enable, m_id );
            Event.SetEventObject( this );
            ProcessEvent( Event );
        }

        m_viewEnabled = enable;

        SetEvtHandlerEnabled( restorestate ); //restore state.
        return true;
    }
    else
    {
        return false; //no change in viewEnable state
    }
}

void a2dView::SetDocument( a2dDocument* doc )
{
    if( m_viewDocument != doc )
    {
        Activate( false );
        bool viewRemoved = false;
        if ( m_viewDocument )
        {
            //a2dView belongs to other a2dDocument so remove it from that document first.
            viewRemoved = true;
            a2dDocumentEvent eventremove( wxEVT_REMOVE_VIEW, this, true, m_id );
            eventremove.SetEventObject( this );
            m_viewDocument->ProcessEvent( eventremove );
            m_viewDocument->DisconnectEventAll( this );
        }

        m_viewDocument = doc;
        if ( doc )
        {
            m_viewDocument->ConnectEvent( wxEVT_CLOSE_VIEW, this );
            m_viewDocument->ConnectEvent( wxEVT_CHANGEDFILENAME_DOCUMENT, this );
            m_viewDocument->ConnectEvent( wxEVT_CHANGEDTITLE_DOCUMENT, this );
            m_viewDocument->ConnectEvent( wxEVT_ENABLE_VIEW, this );
            m_viewDocument->ConnectEvent( wxEVT_UPDATE_VIEWS, this );
            m_viewDocument->ConnectEvent( wxEVT_ENABLE_VIEWS, this );
            m_viewDocument->ConnectEvent( wxEVT_REPORT_VIEWS, this );
            m_viewDocument->ConnectEvent( wxEVT_DISCONNECT_ALLVIEWS, this );

            Activate( true );

            a2dDocumentEvent eventadd( wxEVT_ADD_VIEW, this, viewRemoved , m_id );
            eventadd.SetEventObject( this );
            m_viewDocument->ProcessEvent( eventadd );
        }

        a2dViewEvent viewevent( wxEVT_SET_DOCUMENT, ( a2dDocumentFlagMask ) a2dREFDOC_NON, m_id );
        viewevent.SetEventObject( this );
        ProcessEvent( viewevent );
    }
}

void a2dView::Activate( bool activate )
{
    //wxLogDebug(wxT("a2dView::Activate(this=0x%p'%s' %s) (%s,%d,%s)"),
    //        this, GetClassInfo()->GetClassName(), activate ? wxT("true") : wxT("false"), __A2DFILE__, __LINE__, __DATE__);

    if( !activate && !m_active )
    {
        //wxLogDebug(wxT("a2dView::Activate(false) Ignoring (%s,%d,%s)"),
        //        __A2DFILE__, __LINE__, __DATE__);
        // ignoring activate event if already not active
        return;
    }

    m_active = activate; //first set it.

    a2dViewEvent activateEvent( wxEVT_ACTIVATE_VIEW, activate, m_id );
    activateEvent.SetEventObject( this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ProcessEvent( activateEvent );

    bool oldEnableState = GetEvtHandlerEnabled();
    SetEvtHandlerEnabled( false );

    if ( m_display )
    {
        a2dViewEvent activateEvent( wxEVT_ACTIVATE_VIEW_SENT_FROM_CHILD, activate, m_id );
        activateEvent.SetEventObject( this );
        activateEvent.ResumePropagation( wxEVENT_PROPAGATE_MAX );
        m_display->GetEventHandler()->ProcessEvent( activateEvent );
        activateEvent.StopPropagation();
    }

    SetEvtHandlerEnabled( oldEnableState ); //back to normal

    // the event to the view itself, but also to registered event handlers
    ProcessEvent( activateEvent );
}

// Extend event processing to search the document's event table
bool a2dView::ProcessEvent( wxEvent& event )
{
    if ( ( GetEvtHandlerEnabled() && m_viewEnabled ) ||
            ( event.GetEventType() == wxEVT_ENABLE_VIEW ) ||
            ( event.GetEventType() == wxEVT_ENABLE_VIEWS )
       )
    {
        //prevent the object from being deleted until this process event is at an end
        a2dREFOBJECTPTR_KEEPALIVE;
//        a2dObjectPtr keepAlive = this;
        bool processed = false;

        if ( event.GetEventType() == wxEVT_ACTIVATE )
        {
            Activate( ( ( wxActivateEvent& ) event ).GetActive() );

            //wxLogDebug(wxT("Activate Event(this=0x%p'%s' %s) (%s,%d)"),
            //this, GetClassInfo()->GetClassName(), m_active ? wxT("true") : wxT("false"), __A2DFILE__, __LINE__);

            return true;
        }
        else if ( event.GetEventType() == wxEVT_SET_FOCUS )
        {
            processed = a2dObject::ProcessEvent( event );

            //wxLogDebug(wxT("Focus SET Event(this=0x%p'%s' %s) (%s,%d)"),
            //this, GetClassInfo()->GetClassName(), m_active ? wxT("true") : wxT("false"), __A2DFILE__, __LINE__);

            //Activate( true );
            return processed;
        }
        else if ( event.GetEventType() == wxEVT_KILL_FOCUS )
        {

            //wxLogDebug(wxT("Focus KILL Event(this=0x%p'%s' %s) (%s,%d)"),
            //this, GetClassInfo()->GetClassName(), m_active ? wxT("true") : wxT("false"), __A2DFILE__, __LINE__);

            processed = a2dObject::ProcessEvent( event );
            //Activate( false );
            return processed;
        }
        else if ( event.GetEventType() == wxEVT_CLOSE_WINDOW )
        {
            wxCloseEvent& closeevent = ( wxCloseEvent& ) event;

            //close window events received either through frame of window containing
            //this view, always go through this base class its Close()
            if ( !GetDocument() )
            {
                processed = Close( !closeevent.CanVeto() ); //a view without a document? strange but handle it.
                return processed;
            }

            //view to close and cleanup
            processed = Close( !closeevent.CanVeto() );
            return processed;
        }
        else if ( event.GetEventType() == wxEVT_IDLE )
        {
            //first to document then (if not handled ) normal processing via view
            if ( GetDocument() && GetDocument()->ProcessEvent( event ) )
                return true;
        }

        //normal processing via view
        event.Skip(); //skipped because not processed so prepare for view
        processed = a2dObject::ProcessEvent( event );
        return processed;
    }
    return false;
}

void a2dView::OnCloseView( a2dCloseViewEvent& event )
{
    if ( this != event.GetEventObject() )
        return;
}

void a2dView::OnDisConnectView( a2dDocumentEvent& event )
{
    //prevent recursive calls
    if ( m_isClosed )
        return; //was already closed and diconnected.

    if ( m_viewDocument != event.GetEventObject() )
        return;

    bool processed = false;
    Activate( false );

    // we first try the connector to solve the problem of disconnecting a view.
    // This prevents need for deriving new views just to change disconnect behaviour.
    if ( m_viewTemplate && m_viewTemplate->GetViewConnector() )
    {
        a2dTemplateEvent event( wxEVT_DISCONNECT_VIEW, this, a2dREFDOC_NON, m_id );
        event.SetEventObject( this );
        processed = m_viewTemplate->GetViewConnector()->ProcessEvent( event );
    }

    // if a view was not disconnected (and re-used for a new/old document) or closed by the connector,
    // we proceed with closing down a view.
    if ( !processed )
    {
        Close( true );
    }
}

void a2dView::SetClosed()
{
    m_isClosed = true;
    if ( m_viewDocument )
        m_viewDocument->DisconnectEventAll( this );
    m_viewDocument = 0;
    // release the self owning
    m_keepalive = 0;
}

bool a2dView::Close( bool force )
{
    //prevent recursive calls
    if ( m_isClosed )
        return true; //was already closed

    //prevent the object from being deleted until this process event is at an end
    wxASSERT( m_keepalive == this );

    m_isClosed = true; //prevent recursive calls.

    a2dCloseViewEvent event( m_id );
    event.SetEventObject( this );
    event.SetCanVeto( !force );

    //first inform document about the attempt to close the view.
    if ( m_viewDocument )
    {
        SetEvtHandlerEnabled( false ); //prevent handling via dynamic connect
        m_viewDocument->ProcessEvent( event );
        SetEvtHandlerEnabled( true );
    }

    // document finds it oke, try the connector to see if it can do it.
    if ( !event.GetVeto() )
    {
        // now if a viewtemplate and connector does exist, we assume that the display is handled by that.
        if ( m_viewTemplate.Get() && m_viewTemplate->GetViewConnector() )
            m_viewTemplate->GetViewConnector()->ProcessEvent( event );
        else
        {
            //we do it ourselfs
            if ( m_display )
            {
                bool oldEnableState = GetEvtHandlerEnabled();
                SetEvtHandlerEnabled( false ); //this event coming back from the display will not be handled
                event.ResumePropagation( wxEVENT_PROPAGATE_MAX );
                //second inform display about the attempt to close the view.
                m_display->GetEventHandler()->ProcessEvent( event );
                event.StopPropagation();
                SetEvtHandlerEnabled( oldEnableState ); //back to normal
            }
        }
    }

    if ( !event.GetVeto() )
    {
        //not vetod by m_display or parent frame/windows.
        // That means that the m_display is disconnected from the view, and maybe also destroyed.
        // That depends on the a2dViewConnector.
        // In any case the m_display is not valid anymore right now.
        m_display = NULL;

        //third inform view about the attempt to close the view.
        ProcessEvent( event );
        if ( !event.GetVeto() )
        {
            Activate( false );
            if ( m_viewDocument )
            {
                a2dDocumentEvent eventremove( wxEVT_REMOVE_VIEW, this, true, m_id );
                eventremove.SetEventObject( this );
                m_viewDocument->ProcessEvent( eventremove );
            }
            if ( m_viewDocument )
                m_viewDocument->DisconnectEventAll( this );
            m_viewDocument = 0;

            //! release of itself
            m_keepalive = 0;
            return true;
        }
    }

    m_isClosed = false;

    //because the view is not allowed to close, we do not release itself using m_keepalive = 0
    return false;
}

void a2dView::OnPrint( wxDC* dc, wxObject* WXUNUSED( info ) )
{
    OnDraw( dc );
}


void a2dView::Update( unsigned int hint, wxObject* hintObject )
{
    a2dDocumentEvent event( wxEVT_UPDATE_VIEWS, this );
    event.SetEventObject( m_viewDocument ); //Document receiver is a2dView itself, but
    // event also sent from outside a2dView class in a2dDocument, therefore eventObject is document.
    event.SetUpdateHint( hint );
    event.SetUpdateObject( hintObject );
    ProcessEvent( event );
}

void a2dView::OnUpdate( a2dDocumentEvent& event )
{
    if ( m_viewDocument == event.GetEventObject() && GetEvtHandlerEnabled() && m_display )
        m_display->Refresh();
}

void a2dView::OnReport( a2dDocumentEvent& event )
{
    if ( m_viewDocument == event.GetEventObject() && !m_isClosed )
        event.GetReportList()->push_back( this );
}

void a2dView::OnEnable( a2dViewEvent& event )
{
    if ( this != event.GetEventObject() )
        return;

    if ( event.GetEnable() )
        Update();
}

void a2dView::OnEnableViews( a2dDocumentEvent& event )
{
    if ( m_viewDocument == event.GetEventObject() &&
            (  event.GetView() != this ||  event.GetView() == NULL ) //sender is skipped id not NULL
       )
        Enable( event.GetEnable() );
}

void a2dView::OnChangeFilename( a2dDocumentEvent& event )
{
    if ( m_viewDocument != event.GetEventObject() )
        return;

    bool oldEnableState = GetEvtHandlerEnabled();
    SetEvtHandlerEnabled( false ); //this event coming back from the display will not be handled

    if ( m_display )
    {
        a2dViewEvent eventView( wxEVT_CHANGEDFILENAME_VIEW, ( a2dDocumentFlagMask ) a2dREFDOC_NON, m_id );
        eventView.SetEventObject( this );
        eventView.ResumePropagation( wxEVENT_PROPAGATE_MAX  );
        m_display->GetEventHandler()->ProcessEvent( eventView );
        eventView.StopPropagation();
    }

    SetEvtHandlerEnabled( oldEnableState ); //back to normal
}

void a2dView::OnChangeTitle( a2dDocumentEvent& event )
{
    if ( m_viewDocument != event.GetEventObject() )
        return;

    bool oldEnableState = GetEvtHandlerEnabled();
    SetEvtHandlerEnabled( false ); //this event coming back from the display will not be handled

    if ( m_display )
    {
        a2dViewEvent eventView( wxEVT_CHANGEDTITLE_VIEW, ( a2dDocumentFlagMask ) a2dREFDOC_NON, m_id );
        eventView.SetEventObject( this );
        eventView.ResumePropagation( wxEVENT_PROPAGATE_MAX  );
        m_display->GetEventHandler()->ProcessEvent( eventView );
        eventView.StopPropagation();
    }

    SetEvtHandlerEnabled( oldEnableState ); //back to normal
}

#if wxUSE_PRINTING_ARCHITECTURE
wxPrintout* a2dView::OnCreatePrintout( a2dPrintWhat WXUNUSED( typeOfPrint ), const wxPageSetupDialogData& pageSetupData )
{
    return new a2dDocumentPrintout( this );
}
#endif // wxUSE_PRINTING_ARCHITECTURE

void a2dView::OnSetFocus( wxFocusEvent& event )
{
    event.Skip( true );
}

void a2dView::OnKillFocus( wxFocusEvent& event )
{
    event.Skip( true );
}

// ----------------------------------------------------------------------------
// a2dDocumentTemplate
// ----------------------------------------------------------------------------
a2dDocumentTemplate::a2dDocumentTemplate(
    const wxString& descr,
    const wxString& filter,
    const wxString& dir,
    const wxString& ext,
    const wxString& docTypeName,
    wxClassInfo* docClassInfo,
    a2dViewConnector* connector,
    long flags,
    a2dIOHandlerStrIn* handlerIn,
    a2dIOHandlerStrOut* handlerOut )
{
    m_description = descr;
    m_directory = dir;
    m_defaultExt = ext;
    m_fileFilter = filter;
    m_flags = flags;
    m_docTypeName = docTypeName;
    m_connector = connector;

    m_docClassInfo = docClassInfo;
    m_documentCreate = NULL;

    m_ioHandlerStrIn = handlerIn;
    m_ioHandlerStrOut = handlerOut;
#if wxUSE_PRINTING_ARCHITECTURE
    m_pageSetupData = new wxPageSetupDialogData( *a2dDocviewGlobals->GetDocviewCommandProcessor()->GetPageSetupData() );
#endif
}

a2dDocumentTemplate::~a2dDocumentTemplate()
{
#if wxUSE_PRINTING_ARCHITECTURE
    if( m_pageSetupData )
        delete m_pageSetupData;
#endif
}

a2dObject* a2dDocumentTemplate::SmrtPtrOwn()
{
    m_refcount++;
    return this;
}

bool a2dDocumentTemplate::SmrtPtrRelease()
{
    m_refcount--;

    wxASSERT_MSG( m_refcount >= 0, wxT( "a2dDocumentTemplate::Release Negative reference count" ) );
    if ( m_refcount <= 0 )
    {
        delete this;
        return true;
    }
    return false;
}

a2dObject* a2dDocumentTemplate::DoClone( CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    wxLogMessage( _( "Not implemented" ) );
    return NULL;
}

void a2dDocumentTemplate::SetPageSetupData( wxPageSetupDialogData* pageSetupData )
{ 
#if wxUSE_PRINTING_ARCHITECTURE
    if( m_pageSetupData )
        delete m_pageSetupData;
    m_pageSetupData = pageSetupData; 
#endif
}

a2dIOHandlerStrIn* a2dDocumentTemplate::GetDocumentIOHandlerForLoad( a2dDocumentInputStream& stream, a2dDocument* document )
{
    if ( m_ioHandlerStrIn && m_ioHandlerStrIn->CanLoad( stream, document, m_docClassInfo ) )
        return m_ioHandlerStrIn;
    return NULL;
}

a2dIOHandlerStrOut* a2dDocumentTemplate::GetDocumentIOHandlerForSave( const wxString& filename, a2dDocument* document )
{
    if ( m_ioHandlerStrOut && m_ioHandlerStrOut->CanSave( document ) )
        return m_ioHandlerStrOut;
    return NULL;
}


#if wxART2D_USE_CVGIO
void a2dDocumentTemplate::DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) )
{
    wxLogMessage( _( "Not implemented" ) );
}

void a2dDocumentTemplate::DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) )
{
    wxLogMessage( _( "Not implemented" ) );
}

#endif //wxART2D_USE_CVGIO

// Tries to dynamically construct an object of the right class.
a2dDocument* a2dDocumentTemplate::CreateDocument( const wxFileName& path, a2dDocumentFlagMask flags )
{
    if ( !m_docClassInfo )
        return ( a2dDocument* ) NULL;

    a2dDocument* doc;
    if ( m_documentCreate )
        doc = ( a2dDocument* ) m_documentCreate->Clone( clone_deep );
    else
        doc = ( a2dDocument* ) m_docClassInfo->CreateObject();

    if ( doc )
    {
        //do not yet add the document to the docmanager
        doc->SetFilename( path, false ); // no events to be send internal, to early
        doc->SetDocumentTemplate( this );
        if ( !doc->GetCommandProcessor() )
            doc->CreateCommandProcessor();
        doc->SetDocumentTypeName( m_docTypeName );

        a2dDocumentEvent createdDocument( wxEVT_CREATE_DOCUMENT, path, flags );
        createdDocument.SetEventObject( this );
        doc->ProcessEvent( createdDocument );

        return doc;
    }
    return ( a2dDocument* ) NULL;
}

a2dDocument* a2dDocumentTemplate::SentPreAddCreatedDocumentEvent( a2dDocument* newDoc, a2dDocumentFlagMask flags )
{
    bool ret = false;
    // we only sent this event to the viewconnector of the template.
    // This makes it easier to couple specific templates to specific
    // connector classes.
    a2dTemplateEvent preAddCreatedDocumentEvent( newDoc, wxEVT_PRE_ADD_DOCUMENT, flags );
    preAddCreatedDocumentEvent.SetEventObject( this );
    if ( m_connector )
        ret = m_connector->ProcessEvent( preAddCreatedDocumentEvent );
    else
        // in applications without viewconnectors, try this one for registered event handlers
        ret = ProcessEvent( preAddCreatedDocumentEvent );

    return preAddCreatedDocumentEvent.GetDocument();
}

bool a2dDocumentTemplate::SentPostCreateDocumentEvent( a2dDocument* newDoc, a2dDocumentFlagMask flags )
{
    bool ret = false;
    // we only sent this event to the viewconnector of the template.
    // This makes it easier to couple specific templates to specific
    // connector classes.
    a2dTemplateEvent postCreatedDocumentEvent( newDoc, wxEVT_POST_CREATE_DOCUMENT, flags );
    postCreatedDocumentEvent.SetEventObject( this );
    if ( m_connector )
        ret = m_connector->ProcessEvent( postCreatedDocumentEvent );
    else
        // in applications without viewconnectors, try this one for registered event handlers
        ret = ProcessEvent( postCreatedDocumentEvent );

    return ret;
}

bool a2dDocumentTemplate::FileMatchesTemplate( const wxString& path )
{
    // first test for default extension
    if ( FindExtension( path ) != wxT( "" ) )
        if ( GetDefaultExtension().IsSameAs( FindExtension( path ) ) )
            return true;


#if wxUSE_STD_IOSTREAM
    a2dDocumentFileInputStream store( path.mb_str(), wxSTD ios_base::in | wxSTD ios_base::binary );
    if ( store.fail() || store.bad() )
    {
        //a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for testing format" ), path.c_str() );
        return false;
    }
#else
    wxFileInputStream storeUnbuf( path );
    if ( !storeUnbuf.Ok() )
    {
        //a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for testing format" ), path.c_str() );
        return false;
    }
    wxBufferedInputStream store( storeUnbuf );
#endif

    a2dIOHandlerStrIn* handler = GetDocumentIOHandlerForLoad( store, NULL );
    if ( handler )
        return true;

    wxStringTokenizer parser ( GetFileFilter(), wxT( ";" ) );
    wxString anything = wxT ( "*" );
    while ( parser.HasMoreTokens() )
    {
        wxString filter = parser.GetNextToken();
        wxString filterExt = FindExtension ( filter );
        if ( filter.IsSameAs ( anything )    ||
                filterExt.IsSameAs ( anything ) ||
                filterExt.IsSameAs ( FindExtension ( path ) ) )
            return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// a2dDocumentTemplateAuto
//----------------------------------------------------------------------------

a2dDocumentTemplateAuto::a2dDocumentTemplateAuto(
    const wxString& descr,
    const wxString& filter,
    const wxString& dir,
    const wxString& ext,
    const wxString& docTypeName,
    wxClassInfo* docClassInfo,
    a2dViewConnector* connector,
    long flags ):
    a2dDocumentTemplate( descr, filter, dir, ext, docTypeName, docClassInfo, connector, flags, NULL )
{
}

a2dDocumentTemplateAuto::~a2dDocumentTemplateAuto()
{
}

a2dIOHandlerStrIn* a2dDocumentTemplateAuto::GetDocumentIOHandlerForLoad( a2dDocumentInputStream& stream, a2dDocument* document )
{
    // Find the template for this type of document.
    const a2dDocumentTemplateList& allDocTemplates = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocTemplates();
    for( a2dDocumentTemplateList::const_iterator iter = allDocTemplates.begin(); iter != allDocTemplates.end(); ++iter )
    {
        a2dDocumentTemplateList::value_type temp = *iter;
        if ( temp != this && temp->GetDocumentTypeName() == GetDocumentTypeName() &&
                temp->GetDocumentIOHandlerStrIn() &&
                temp->GetDocumentIOHandlerStrIn()->CanLoad( stream, document, temp->GetDocumentClassInfo() ) )
            return temp->GetDocumentIOHandlerStrIn();
    }
    return NULL;
}

a2dIOHandlerStrOut* a2dDocumentTemplateAuto::GetDocumentIOHandlerForSave( const wxString& filename, a2dDocument* document )
{
    // Find the template for this type of document.
    const a2dDocumentTemplateList& allDocTemplates = a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocTemplates();
    for( a2dDocumentTemplateList::const_iterator iter = allDocTemplates.begin(); iter != allDocTemplates.end(); ++iter )
    {
        a2dDocumentTemplateList::value_type temp = *iter;
        if ( temp != this && temp->GetDocumentTypeName() == GetDocumentTypeName() && temp->GetDocumentIOHandlerStrOut() && temp->GetDocumentIOHandlerStrOut()->CanSave( document ) )
        {
            if ( filename.IsEmpty() )
                return temp->GetDocumentIOHandlerStrOut();
            else
            {
                wxString path, name, ext;
                wxFileName::SplitPath( filename, & path, & name, & ext );
                if ( ext == temp->GetDefaultExtension() )
                    return temp->GetDocumentIOHandlerStrOut();
            }
        }
    }
    return NULL;
}

// ----------------------------------------------------------------------------
// a2dViewTemplate
// ----------------------------------------------------------------------------
a2dViewTemplate::a2dViewTemplate(
    const wxString& descr,
    const wxString& docTypeName,
    const wxString& viewTypeName,
    wxClassInfo* viewClassInfo,
    a2dViewConnector* connector,
    long flags,
    const wxSize& size
)
{
    m_description = descr;
    m_flags = flags;
    m_docTypeName = docTypeName;
    m_viewTypeName = viewTypeName;
    m_connector = connector;

    m_viewClassInfo = viewClassInfo;

    m_initialSize = size;
#if wxUSE_PRINTING_ARCHITECTURE
    m_pageSetupData = new wxPageSetupDialogData( *a2dDocviewGlobals->GetDocviewCommandProcessor()->GetPageSetupData() );
#endif
}

a2dViewTemplate::~a2dViewTemplate()
{
#if wxUSE_PRINTING_ARCHITECTURE
    if( m_pageSetupData )
        delete m_pageSetupData;
#endif
}

a2dObject* a2dViewTemplate::SmrtPtrOwn()
{
    m_refcount++;
    return this;
}

bool a2dViewTemplate::SmrtPtrRelease()
{
    m_refcount--;

    wxASSERT_MSG( m_refcount >= 0, wxT( "a2dViewTemplate::Release Negative reference count" ) );
    if ( m_refcount <= 0 )
    {
        delete this;
        return true;
    }
    return false;
}

a2dObject* a2dViewTemplate::DoClone( CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    wxLogMessage( _( "Not implemented" ) );
    return NULL;
}

void a2dViewTemplate::SetPageSetupData( wxPageSetupDialogData* pageSetupData )
{ 
#if wxUSE_PRINTING_ARCHITECTURE
    if( m_pageSetupData )
        delete m_pageSetupData;
    m_pageSetupData = pageSetupData; 
#endif
}

#if wxART2D_USE_CVGIO
void a2dViewTemplate::DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) )
{
    wxLogMessage( _( "Not implemented" ) ) ;
}

void a2dViewTemplate::DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) )
{
    wxLogMessage( _( "Not implemented" ) );
}
#endif //wxART2D_USE_CVGIO

a2dView* a2dViewTemplate::CreateView( a2dDocument* doc, a2dDocumentFlagMask flags )
{
    if ( !m_viewClassInfo )
        return ( a2dView* ) NULL;

    a2dView* view = ( a2dView* )m_viewClassInfo->CreateObject();

    if ( view )
    {
        view->SetDocument( doc );
        view->SetViewTypeName( m_viewTypeName );
        view->SetViewTemplate( this );

        a2dViewEvent createdView( wxEVT_CREATE_VIEW, flags, view->GetId() );
        createdView.SetEventObject( this ); //Template, a specific view is receiver

        //we are not interested if it was handled or not here.
        view->ProcessEvent( createdView );

        a2dTemplateEvent postCreatedViewEvent( wxEVT_POST_CREATE_VIEW, view, flags );
        postCreatedViewEvent.SetEventObject( this ); //Template, a specific connector is receiver
        m_connector->ProcessEvent( postCreatedViewEvent );
        view->Activate( true );
        return view;
    }

    return ( a2dView* ) NULL;
}

// ----------------------------------------------------------------------------
// a2dViewConnector
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dViewConnector, a2dObject )
    EVT_POST_CREATE_DOCUMENT( a2dViewConnector::OnPostCreateDocument )
    EVT_DISCONNECT_VIEW( a2dViewConnector::OnDisConnectView )
    EVT_CLOSE_VIEW( a2dViewConnector::OnCloseView )
END_EVENT_TABLE()

a2dViewConnector::a2dViewConnector()
{
    m_initialPos = wxPoint( 0, 0 );
    m_initialSize = wxSize( 300, 400 );
    m_initialStyle = wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE;
    //m_initialStyle = wxDEFAULT_FRAME_STYLE | wxFRAME_NO_TASKBAR ;
    m_OnlyDisconnect = false;
}

a2dViewConnector::~a2dViewConnector()
{
    wxASSERT_MSG(  m_refcount == 0, wxT( "deleting a2dViewConnector while referenced" ) );
}

a2dViewConnector::a2dViewConnector( const a2dViewConnector& other )
{
    m_initialPos = other.m_initialPos;
    m_initialSize = other.m_initialSize;
    m_initialStyle = other.m_initialStyle;
    m_OnlyDisconnect = other.m_OnlyDisconnect;
}

void a2dViewConnector::AssociateViewTemplate( a2dViewTemplate* temp )
{
    if ( m_viewTemplates.end() == m_viewTemplates.Find( temp ) )
    {
        m_viewTemplates.push_back( temp );
    }
}

void a2dViewConnector::DisassociateViewTemplate( a2dViewTemplate* temp )
{
    m_viewTemplates.ReleaseObject( temp );
}

void a2dViewConnector::DisassociateViewTemplates()
{
    m_viewTemplates.clear();
}

void a2dViewConnector::OnPostCreateDocument( a2dTemplateEvent& event )
{
    //a2dDocumentTemplate* templ = (a2dDocumentTemplate*) event.GetEventObject();
    a2dViewTemplate* viewtempl;

    viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->SelectViewType( event.GetDocument(),
                m_viewTemplates.size() ? m_viewTemplates : a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates(),
                wxT( "" ),
                true, event.GetTemplateMaskFlags() );

    if ( !viewtempl )
    {
        wxLogMessage( _( "No view available for this document in a2dViewConnector" ) );
        return;
    }
    else
    {
        a2dView* view = viewtempl->CreateView( event.GetDocument(), event.GetFlags() );
        event.SetView( view );
        if ( view )
            return;
    }
    wxLogMessage( _( "a2dViewConnector no view was created" ) );
}

a2dView* a2dViewConnector::CreateView( a2dDocument* doc, const wxString& viewTypeName,
                                       a2dDocumentFlagMask flags,
                                       a2dTemplateFlagMask viewTemplateFlags )
{
    //a2dDocumentTemplate* templ = (a2dDocumentTemplate*) event.GetEventObject();
    a2dViewTemplate* viewtempl;

    viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->SelectViewType( doc,
                m_viewTemplates.size() ? m_viewTemplates :
                a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates(), viewTypeName,
                true, viewTemplateFlags );

    a2dView* createdView;

    if ( !viewtempl )
    {
        wxLogMessage( _( "No view available for this document in a2dViewConnector" ) );
        return NULL;
    }
    else
    {
        createdView = viewtempl->CreateView( doc, flags );
        if ( createdView )
            return createdView;
    }
    wxLogMessage( _( "a2dViewConnector no view was created" ) );
    return NULL;
}

void a2dViewConnector::OnDisConnectView(  a2dTemplateEvent& event )
{
    a2dSmrtPtr<a2dView> theViewToDisconnect = ( a2dView* ) event.GetEventObject();

    if ( !m_OnlyDisconnect )
        // fall back on view bahaviour which is closing down the view and its display window etc.
        theViewToDisconnect->Close( true );
    else
    {
        // keep the view and its display window, only remove it from the document.
        theViewToDisconnect->Activate( false );
        if ( theViewToDisconnect->GetDocument() )
        {
            a2dDocumentEvent eventremove( wxEVT_REMOVE_VIEW, theViewToDisconnect, true, theViewToDisconnect->GetId() );
            eventremove.SetEventObject( theViewToDisconnect );
            theViewToDisconnect->GetDocument()->ProcessEvent( eventremove );
        }
        theViewToDisconnect->SetDocument( NULL );
    }
}

void a2dViewConnector::OnCloseView( a2dCloseViewEvent& event )
{
    a2dSmrtPtr<a2dView> theViewToClose = ( a2dView* ) event.GetEventObject();

    if ( theViewToClose->GetDisplayWindow() )
    {
        bool oldEnableState = theViewToClose->GetEvtHandlerEnabled();
        theViewToClose->SetEvtHandlerEnabled( false ); //this event coming back from the display will not be handled

        event.ResumePropagation( wxEVENT_PROPAGATE_MAX );
        //second inform view about the attempt to close the view.
        theViewToClose->GetDisplayWindow()->GetEventHandler()->ProcessEvent( event );
        event.StopPropagation();

        theViewToClose->SetEvtHandlerEnabled( oldEnableState ); //back to normal
    }
}

// ----------------------------------------------------------------------------
// a2dFrameViewConnector
// ----------------------------------------------------------------------------

a2dFrameViewConnector::a2dFrameViewConnector(): a2dViewConnector()
{
    m_docframe = ( a2dDocumentFrame* ) NULL;
    m_createChildframe = false;
}

a2dFrameViewConnector::a2dFrameViewConnector( const a2dFrameViewConnector& other ): a2dViewConnector( other )
{
    m_docframe = other.m_docframe;
    m_createChildframe = other.m_createChildframe;
}

void a2dFrameViewConnector::Init( wxFrame* frame, bool createChildframe )
{
    m_docframe = frame;
    m_createChildframe = createChildframe;
}

void a2dFrameViewConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    a2dView* newview = event.GetView();
    //a2dDocument *doc = newview->GetDocument();

    if ( m_createChildframe )
    {
        a2dDocumentFrame* viewFrame = new a2dDocumentFrame( false,
                m_docframe, newview,  -1, newview->GetViewTypeName(),
                GetInitialPosition(),
                GetInitialSize(),
                GetInitialStyle()
                                                          );
        viewFrame->Show( true );
    }
    else
    {
        a2dView* oldview = ( ( a2dDocumentFrame* ) m_docframe )->GetView();
        if ( oldview )
        {
            oldview->SetDisplayWindow( NULL );
        }
        ( ( a2dDocumentFrame* ) m_docframe )->SetView( newview );
        newview->SetDisplayWindow( m_docframe );
    }
    newview->Update();
}

// ----------------------------------------------------------------------------
// a2dScrolledWindowViewConnector
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dScrolledWindowViewConnector, a2dViewConnector )

// ----------------------------------------------------------------------------
// a2dWindowViewConnector
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dWindowViewConnector, a2dViewConnector )

/********************************************************************
* a2dDocumentViewScrolledWindow
*********************************************************************/
IMPLEMENT_CLASS( a2dDocumentViewScrolledWindow, a2dViewWindow<wxScrolledWindow> )

BEGIN_EVENT_TABLE( a2dDocumentViewScrolledWindow, a2dViewWindow<wxScrolledWindow> )
    EVT_CLOSE_VIEW( a2dDocumentViewScrolledWindow::OnCloseView )
    EVT_PAINT( a2dDocumentViewScrolledWindow::OnPaint )
END_EVENT_TABLE()

a2dDocumentViewScrolledWindow::a2dDocumentViewScrolledWindow( a2dView* view, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, const long style, const wxString& name ):
    a2dViewWindow<wxScrolledWindow>( view, parent, id, pos, size, style, name )
{
}

a2dDocumentViewScrolledWindow::a2dDocumentViewScrolledWindow( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, const long style, const wxString& name ):
    a2dViewWindow<wxScrolledWindow>( parent, id, pos, size, style, name )
{
}

a2dDocumentViewScrolledWindow::~a2dDocumentViewScrolledWindow()
{
}

/********************************************************************
* a2dDocumentViewWindow
*********************************************************************/

IMPLEMENT_CLASS( a2dDocumentViewWindow, a2dViewWindow<wxWindow> )

BEGIN_EVENT_TABLE( a2dDocumentViewWindow, a2dViewWindow<wxWindow> )
    EVT_CLOSE_VIEW( a2dViewWindow<wxWindow>::OnCloseView )
    EVT_PAINT( a2dViewWindow<wxWindow>::OnPaint )
END_EVENT_TABLE()

a2dDocumentViewWindow::a2dDocumentViewWindow( a2dView* view, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, const long style, const wxString& name ):
    a2dViewWindow<wxWindow>( view, parent, id, pos, size, style, name )
{
}

a2dDocumentViewWindow::a2dDocumentViewWindow( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, const long style, const wxString& name ):
    a2dViewWindow<wxWindow>( parent, id, pos, size, style, name )
{
}

a2dDocumentViewWindow::~a2dDocumentViewWindow()
{
}

/********************************************************************
* a2dDocumentPrintout
*********************************************************************/


#if wxUSE_PRINTING_ARCHITECTURE

a2dDocumentPrintout::a2dDocumentPrintout( a2dView* view, const wxString& title )
    : wxPrintout( title )
{
    m_printoutView = view;
}

bool a2dDocumentPrintout::OnPrintPage( int WXUNUSED( page ) )
{
    wxDC* dc = GetDC();

    // Get the logical pixels per inch of screen and printer
    int ppiScreenX, ppiScreenY;
    GetPPIScreen( &ppiScreenX, &ppiScreenY );
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter( &ppiPrinterX, &ppiPrinterY );

    // This scales the DC so that the printout roughly represents the
    // the screen scaling. The text point size _should_ be the right size
    // but in fact is too small for some reason. This is a detail that will
    // need to be addressed at some point but can be fudged for the
    // moment.
    float scale = ( float )( ( float )ppiPrinterX / ( float )ppiScreenX );

    // Now we have to check in case our real page size is reduced
    // (e.g. because we're drawing to a print preview memory DC)
    int pageWidth, pageHeight;
    int w, h;
    dc->GetSize( &w, &h );
    GetPageSizePixels( &pageWidth, &pageHeight );

    // If printer pageWidth == current DC width, then this doesn't
    // change. But w might be the preview bitmap width, so scale down.
    float overallScale = scale * ( float )( w / ( float )pageWidth );
    dc->SetUserScale( overallScale, overallScale );

    if ( m_printoutView )
    {
        m_printoutView->OnPrint( dc, NULL );
    }
    return true;
}

bool a2dDocumentPrintout::HasPage( int pageNum )
{
    return ( pageNum == 1 );
}

bool a2dDocumentPrintout::OnBeginDocument( int startPage, int endPage )
{
    if ( !wxPrintout::OnBeginDocument( startPage, endPage ) )
        return false;

    return true;
}

void a2dDocumentPrintout::GetPageInfo( int* minPage, int* maxPage, int* selPageFrom, int* selPageTo )
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

#endif // wxUSE_PRINTING_ARCHITECTURE


/********************************************************************
* a2dDocumentFrame
*********************************************************************/

IMPLEMENT_CLASS( a2dDocumentFrame, wxFrame )

BEGIN_EVENT_TABLE( a2dDocumentFrame, wxFrame )
    EVT_MENU( wxID_EXIT, a2dDocumentFrame::OnExit )
    EVT_MENU_RANGE( wxID_FILE1, wxID_FILE9, a2dDocumentFrame::OnMRUFile )
    EVT_CHANGEDFILENAME_VIEW( a2dDocumentFrame::OnChangeFilename )
    EVT_CLOSE_VIEW( a2dDocumentFrame::OnCloseView )
    EVT_CLOSE( a2dDocumentFrame::OnCloseWindow )
    EVT_PAINT( a2dDocumentFrame::OnPaint )
    EVT_ACTIVATE_VIEW_SENT_FROM_CHILD( a2dDocumentFrame::OnActivateViewSentFromChild )
END_EVENT_TABLE()

a2dDocumentFrame::a2dDocumentFrame()
{
    m_destroyOnCloseView = true;
    m_isParentFrame = true;
    m_view = NULL;
}

a2dDocumentFrame::a2dDocumentFrame( bool parentFrame,
                                    wxWindow* frame,
                                    a2dView* view,
                                    wxWindowID id,
                                    const wxString& title,
                                    const wxPoint& pos,
                                    const wxSize& size,
                                    long  style,
                                    const wxString& name )
    : wxFrame( frame, id, title, pos, size, style, name )
{
    m_destroyOnCloseView = true;
    m_isParentFrame = parentFrame;
    m_view = view;
    if ( m_view )
    {
        m_view->SetDisplayWindow( this );
    }
}

a2dDocumentFrame::a2dDocumentFrame(
    wxWindow* frame,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long  style,
    const wxString& name )
    : wxFrame( frame, id, title, pos, size, style, name )
{
    m_destroyOnCloseView = true;
    m_isParentFrame = true;
    m_view = NULL;
    if ( m_view )
    {
        m_view->SetDisplayWindow( this );
    }
}

bool a2dDocumentFrame::Create(
    bool parentFrame,
    wxWindow* frame,
    a2dView* view,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long  style,
    const wxString& name )
{
    bool res = wxFrame::Create( frame, id, title, pos, size, style, name );
    m_destroyOnCloseView = true;
    m_isParentFrame = parentFrame;
    m_view = view;
    if ( m_view )
    {
        m_view->SetDisplayWindow( this );
    }
    return res;
}


a2dDocumentFrame::~a2dDocumentFrame()
{
}

bool a2dDocumentFrame::ProcessConnectedEvent( wxEvent& event )
{
    if ( ProcessEvent( event ) )
    {
        event.Skip( false );
        return true;
    }
    event.Skip( true );
    return false;
}

void a2dDocumentFrame::ConnectEvent( wxEventType type, wxEvtHandler* evtObject )
{
    Connect( type, wxObjectEventFunction( &a2dObject::ProcessConnectedEvent ), 0, evtObject );
}

bool a2dDocumentFrame::DisconnectEvent( wxEventType type, wxEvtHandler* evtObject )
{
    return Disconnect( type, wxObjectEventFunction( &a2dObject::ProcessConnectedEvent ), 0, evtObject );
}

void a2dDocumentFrame::SetView( a2dView* view )
{
    m_view = view;
}

// Clean up frames/windows used for displaying the view.
// They or not really deleted here only flagged for deletion using Destroy(),
// this means that they will really be deleted in idle time by wxWindows.
void a2dDocumentFrame::OnCloseView( a2dCloseViewEvent& event )
{
    //there is no need to handle the parent frame differently

    if ( m_view && m_view == event.GetEventObject() )
    {
        //frames can be destroyed savely ( happening in idle time )
        //This default, asumes a multi frame application, where each view has one frame
        if ( m_destroyOnCloseView )
            Destroy(); //very likely done somewhere else, but makes sure it happens.

        m_view = NULL; //this here prevents looping, when called a second time
        //or prevent whatever other of m_view
    }

    if ( m_isParentFrame && GetParent() )
        GetParent()->Refresh();
    else
        Refresh();

    // typically event is propogated and handled by the window chain
    event.Skip();
}

void a2dDocumentFrame::OnChangeFilename( a2dViewEvent& event )
{
    a2dDocument* doc = ( ( a2dView* )event.GetEventObject() )->GetDocument();

    if ( doc )
    {
        wxString title = doc->GetPrintableName();

        SetTitle( title );
    }
}

bool a2dDocumentFrame::ProcessEvent( wxEvent& event )
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
        //In case of the parent frame being closed all other child frames should be closed also.
        //Therefore the docmanager closes all documents, the document closes all views, and each view
        //sents a view close event to its display window.
        //The event is not handled in the window, and goes up to this a2dDocumentFrame.
        //The frame will then Destroy the child frame in a multi child application.
        //In the parent frame closes its own view. ( the parentFrame already
        //has flagged itself for Destroy. )
        //
        //In case of a child frame being closed only the child view will be closed and the frame flaged
        //for Destroy ( see OnCloseView )

        wxCloseEvent& closeevent = ( wxCloseEvent& ) event;

        if ( m_isParentFrame )
        {
            //First give the Frame the right to veto in for instance a OnCloseWindow which is called by
            // the wxEVT_CLOSE_WINDOW handler
            if ( wxEvtHandler::ProcessEvent( event ) && !closeevent.GetVeto() )
            {
                // Define the behaviour for the parent frame closing
                // - must delete all child frames except for the main one.
                // All Documents will be closed, and because of that views will
                // first be disconnected via a2dView::OnDisConnectView().
                // This by default also closes the view, and that will normally lead
                // to the m_display and parent windows/frames being closed.
                // The wxEVT_CLOSE_VIEW generated in a2dView::Close()
                // leads via its display window to this a2dDocumentFrame.
                // This may decide to destroy the Frame and window associated with the view.

                // You should normally exit the main event loop (and the application) by deleting the top window.
                // So if closing the top window, its is clear we want to exit.
                if ( wxTheApp->GetTopWindow() == this )
                {
                    if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->Exit( !closeevent.CanVeto() ) )
                        closeevent.Veto();
                }
                else
                {
                    if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear( !closeevent.CanVeto() ) )
                        closeevent.Veto();
                }
            }
            else
                closeevent.Veto();
            ret = true;
        }
        else
        {
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
    else if ( event.GetEventType() == wxEVT_IDLE )
    {
        // we need to get idle events to the document of the view, in order to have the document
        // sent update events if changed internally.
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

void a2dDocumentFrame::OnCloseWindow( wxCloseEvent& event )
{
    if ( event.CanVeto() )
    {
        if ( !GetIsParentFrame() )
        {
            if ( !m_view || m_view->Close( !event.CanVeto() ) )
            {
                Destroy();
                m_view = NULL;
            }
            else
                event.Veto( true );
        }
        else
        {
            if ( !a2dDocviewGlobals->GetDocviewCommandProcessor()->Clear( !event.CanVeto() ) )
                event.Veto( true );
        }
    }
    else
        Destroy();
}

void a2dDocumentFrame::OnExit( wxCommandEvent& WXUNUSED( event ) )
{
    if ( m_isParentFrame )
    {
        Close( true );
    }
}

void a2dDocumentFrame::OnMRUFile( wxCommandEvent& event )
{
    if ( m_isParentFrame )
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
}

void a2dDocumentFrame::OnPaint( wxPaintEvent& WXUNUSED( event ) )
{
    wxPaintDC dc( this );
    PrepareDC( dc );

    if ( m_view && m_view->GetDisplayWindow() == this )
        OnDraw( dc );
}

// Define the repainting behaviour
void a2dDocumentFrame::OnDraw( wxDC& dc )
{
    if ( m_view && m_view->GetDisplayWindow() == this )
        m_view->OnDraw( & dc );
}

void a2dDocumentFrame::OnActivateViewSentFromChild( a2dViewEvent& viewevent )
{
    if ( viewevent.GetActive() )
    {
        m_view = ( a2dView* ) viewevent.GetEventObject();
    }
}

void a2dDocumentFrame::OnCmdMenuId( wxCommandEvent& event )
{
    event.Skip();
}

void a2dDocumentFrame::AddCmdToToolbar( const a2dMenuIdItem& cmdId )
{
    if ( ! GetToolBar() )
        return;
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentFrame::OnCmdMenuId ) );

    wxASSERT_MSG( a2dMenuIdItem::GetInitialized(), _T( "call a2dMenuIdItem::InitializeBitmaps() before using AddCmdToToolbar" ) );

    wxString error = _T( "No Bitmap for a2dToolCmd found for:" ) + cmdId.GetIdName();
    wxASSERT_MSG( cmdId.GetBitmap().Ok(), error );
    GetToolBar()->AddTool( cmdId.GetId(), cmdId.GetLabel(), cmdId.GetBitmap(), cmdId.GetHelp(), cmdId.GetKind() );
}

void a2dDocumentFrame::ConnectCmdId( const a2dMenuIdItem& cmdId )
{
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentFrame::OnCmdMenuId ) );
}

void a2dDocumentFrame::AddCmdMenu( wxMenu* parentMenu, wxMenuItem* item )
{
    Connect( item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentFrame::OnCmdMenuId ) );
    parentMenu->Append( item );
}

void a2dDocumentFrame::AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
{
    Connect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentFrame::OnCmdMenuId ) );
    parentMenu->Append( cmdId.GetId(), wxGetTranslation( cmdId.GetText() ), wxGetTranslation( cmdId.GetHelp() ), cmdId.IsCheckable() );
}

void a2dDocumentFrame::RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
{
    Disconnect( cmdId.GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( a2dDocumentFrame::OnCmdMenuId ) );
    parentMenu->Delete( cmdId.GetId() );
}

void a2dDocumentFrame::OnExecuteCommand( wxCommandEvent& event )
{
    a2dCommandStore* command = ( a2dCommandStore* ) event.m_callbackUserData;
    a2dDocviewGlobals->GetDocviewCommandProcessor()->Submit( command->m_smrtP->TClone() );
}

//#endif // wxUSE_DOC_VIEW_ARCHITECTURE
