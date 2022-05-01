/*! \file wx/docview/doccom.h
    \brief Docview framework its controlling class

    All classes in the document view framework are under control of the a2dDocumentCommandProcessor.
    It sends events to documents, templates classes for new document and views are stored here.
    At last the class is able to function as a central command processor for the whole framework.

    \author Klaas Holwerda
    \date Created 05/07/03

    Copyright: (c)

    Licence: wxWidgets licence

    RCS-ID: $Id: doccom.h,v 1.55 2009/09/26 19:01:04 titato Exp $
*/

#ifndef _WX_DOCCOMH__
#define _WX_DOCCOMH__

#include "wx/cmdproc.h"

#if wxUSE_PRINTING_ARCHITECTURE
#include "wx/paper.h"
#include "wx/print.h"
#endif

#include <wx/module.h>

#include <wx/general/genmod.h>
#include <wx/docview.h>
#include <wx/docview/docviewref.h>
#include <wx/filedlg.h>

//! find a parent wxWindow pointer to place a control into
extern wxWindow* wxFindSuitableParent();

//! extra menu ideas for docview framework of wxDocview.
enum
{
    wxID_ART2D_LOWEST = 5300,          /*!< reserved range within total range used by wxWindows */
};

class A2DDOCVIEWDLLEXP a2dDocumentCommandProcessor;
template class A2DDOCVIEWDLLEXP a2dSmrtPtr<a2dDocumentCommandProcessor>;

/**********************************************
 New events Document and View events.
**********************************************/
BEGIN_DECLARE_EVENT_TYPES()
//! see a2dCommandEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_ADD_DOCUMENT, 1 )
//! see a2dCommandEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_REMOVE_DOCUMENT, 1 )
//! see a2dCommandEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_CHANGED_DOCUMENT, 1 )
//! see a2dCommandEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DDOCVIEWDLLEXP, wxEVT_CANNOT_OPEN_DOCUMENT, 1 )
END_DECLARE_EVENT_TYPES()

//! Event sent to/from a2dDocumentCommandProcessor
/*!
     - ::wxEVT_ADD_DOCUMENT is sent from a2dDocumentCommandProcessor when a document is added.
     - ::wxEVT_REMOVE_DOCUMENT is sent from a2dDocumentCommandProcessor when a document is removed.
     - ::wxEVT_CHANGED_DOCUMENT is sent from a2dDocumentCommandProcessor when the current document has changed.
     - ::wxEVT_DO is sent from a2dDocumentCommandProcessor or any other commandprocessor when a command is issued.
     - ::wxEVT_UNDO is sent from a2dDocumentCommandProcessor or any other commandprocessor when a command is issued.
     - ::wxEVT_REDO is sent from a2dDocumentCommandProcessor or any other commandprocessor when a command is issued.

     - ::wxEVT_MENUSTRINGS is sent from a2dCommandProcessor when menu string need to be updated after a command.

     - ::wxEVT_CANNOT_OPEN_DOCUMENT is sent by a2dDocumentCommandProcessor::CreateDocument() when a file could not be opened

    Use a2dObject::ConnectEvent to connect your own wxEvtHandler to the a2CommandProcessor
    to receive them.

    \ingroup docview docviewevents
*/
class A2DDOCVIEWDLLEXP a2dCommandEvent : public a2dCommandProcessorEvent
{

public:

    //! constructor
    /*!
        type should be ::wxEVT_ADD_DOCUMENT or ::wxEVT_REMOVE_DOCUMENT or ::wxEVT_CHANGED_DOCUMENT
    */
    a2dCommandEvent( wxEventType type, a2dDocument* doc )
        : a2dCommandProcessorEvent( type, 0 )
    {
        m_doc = doc;
    }

    //! constructor
    /*!
        type should be ::wxEVT_MENUSTRINGS ::wxEVT_DO ::wxEVT_UNDO

        For ::wxEVT_MENUSTRINGS, you can use the following function, to get the info to set the menu string.
            wxCommandProcessor::GetUndoMenuLabel()
            wxCommandProcessor::GetRedoMenuLabel()
            wxCommandProcessor::CanUndo()
            wxCommandProcessor::CanRedo()
    */
    a2dCommandEvent( wxEventType type, a2dCommand* cmd, a2dDocument* doc = NULL )
        : a2dCommandProcessorEvent( type, 0 )
    {
        m_doc = doc;
        m_cmd = cmd;
    }

    //! constructor
    /*!
        type ::wxEVT_MENUSTRINGS
    */
    a2dCommandEvent( a2dCommand* cmd,
                     const wxString& WXUNUSED( undoLabel ), bool WXUNUSED( canUndo ),
                     const wxString& WXUNUSED( redoLabel ), bool WXUNUSED( canRedo ),
                     a2dDocument* doc = NULL
                   )
        : a2dCommandProcessorEvent( wxEVT_MENUSTRINGS, 0 )
    {
        m_doc = doc;
        m_cmd = cmd;
    }

    //! constructor
    /*!
        type ::wxEVT_RECORD
    */
    a2dCommandEvent( const wxString& record )
        : a2dCommandProcessorEvent( wxEVT_RECORD, 0 )
    {
        m_doc = NULL;
        m_cmd = NULL;
        m_record = record;
    }

    //! constructor
    a2dCommandEvent( const a2dCommandEvent& event )
        : a2dCommandProcessorEvent( event )
    {
        m_doc = event.m_doc;
        m_cmd = event.m_cmd;
        m_record = event.m_record;
    }

    //! clone the event.
    virtual wxEvent* Clone( bool WXUNUSED( deep ) = true ) const { return new a2dCommandEvent( *this ); }

    //! the document created/removed or on which the command was applied.
    a2dDocument* GetDocument() { return m_doc; }

    //! the command ( if there was one ) that did it.
    a2dCommand* GetCommand() { return m_cmd; }

    wxString& GetRecord() { return m_record; }

private:

    //! see GetDocument()
    a2dDocument* m_doc;

    wxString m_record;
};


//! internal event function for static event tables declaration
typedef void ( wxEvtHandler::*wxDocCommandProcessorEventFunction )( a2dCommandEvent& );

/*! \addtogroup eventhandlers
*  @{
*/

//! event sent to a2dDocumentCommandProcessor when a document has been added to the a2dDocumentCommandProcessor
#define EVT_ADD_DOCUMENT(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_ADD_DOCUMENT, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast< wxDocCommandProcessorEventFunction > (& func), (wxObject *) NULL ),
//! event sent to a2dDocumentCommandProcessor when a document will be removed from the a2dDocumentCommandProcessor
#define EVT_REMOVE_DOCUMENT(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_REMOVE_DOCUMENT, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast< wxDocCommandProcessorEventFunction > (& func), (wxObject *) NULL ),
//! event sent to a2dDocumentCommandProcessor when the current document has changed in the a2dDocumentCommandProcessor
#define EVT_CHANGED_DOCUMENT(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_CHANGED_DOCUMENT, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast< wxDocCommandProcessorEventFunction > (& func), (wxObject *) NULL ),
//! event sent when a file could not be opened in the a2dDocumentCommandProcessor
#define EVT_CANNOT_OPEN_DOCUMENT(func) DECLARE_EVENT_TABLE_ENTRY( wxEVT_CANNOT_OPEN_DOCUMENT, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast< wxDocCommandProcessorEventFunction > (& func), (wxObject *) NULL ),
//! event sent to a2dDocumentCommandProcessor when a document has been added to the a2dDocumentCommandProcessor
#define EVT_RECORD(func)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_RECORD, wxID_ANY, wxID_ANY, (wxObjectEventFunction) static_cast< wxDocCommandProcessorEventFunction > (& func), (wxObject *) NULL ),


class A2DDOCVIEWDLLEXP a2dFileHistory;
class A2DDOCVIEWDLLEXP a2dFileHistoryItem;

DECLARE_MENU_ITEMID( CmdMenu_Exit )
DECLARE_MENU_ITEMID( CmdMenu_FileClose )
DECLARE_MENU_ITEMID( CmdMenu_FileCloseAll )
DECLARE_MENU_ITEMID( CmdMenu_FileOpen )
DECLARE_MENU_ITEMID( CmdMenu_FileNew )
DECLARE_MENU_ITEMID( CmdMenu_FileSave )
DECLARE_MENU_ITEMID( CmdMenu_FileSaveAll )
DECLARE_MENU_ITEMID( CmdMenu_FileSaveAs )
DECLARE_MENU_ITEMID( CmdMenu_FileImport )
DECLARE_MENU_ITEMID( CmdMenu_FileExport )
DECLARE_MENU_ITEMID( CmdMenu_FileRevert )
DECLARE_MENU_ITEMID( CmdMenu_CreateView )
DECLARE_MENU_ITEMID( CmdMenu_Print )
DECLARE_MENU_ITEMID( CmdMenu_Preview )
DECLARE_MENU_ITEMID( CmdMenu_PrintView )
DECLARE_MENU_ITEMID( CmdMenu_PreviewView )
DECLARE_MENU_ITEMID( CmdMenu_PrintDocument )
DECLARE_MENU_ITEMID( CmdMenu_PreviewDocument )
DECLARE_MENU_ITEMID( CmdMenu_PrintSetup )
DECLARE_MENU_ITEMID( CmdMenu_EmptyDocument )

//!  @} eventhandlers

//! One object of this class may be created in an application, to manage all the templates and documents.
/*!
    Events not handled by a a2dDocumentFrame or derived class, are redirected to this class
    Events in the Application Child frames are also routed to the Parent Frame and from there
    to this class.
    This is convenient for combining the event table that is used for menu's in child as well as
    parent frames.

    The a2dDocumentCommandProcessor class is part of the document/view framework supported by wxDocview,
    and cooperates with the a2dView, a2dDocument, a2dDocumentTemplate, a2dViewTemplate and
    a2dViewConnector classes.

    All open documents and templates are maintained here, and functions to open new documents and views
    ( via its templates ), are setup from within this class.
    As such it is the communication class within the docview framework.

    It is derived from a2dCommandProcessor, in order to use it as a command interpreter.
    Although not so much for Undo and Redo in this case. Much more to have a in between level for the
    Graphical User Interfaces and the commands that open up new documents and views.
    Defining your own docview commands in a derived a2dDocumentCommandProcessor, makes it easy
    to implement macro recording to a file, and replay the same macro from a file.
    Calling all member function of a2dDocumentCommandProcessor directly from the GUI, would make this
    impractible.
    So by issueing commands through the Submit() function, you will have a central point
    for calling member functions on the a2dDocumentCommandProcessor. But you are still free to call all member function
    directly.

    \ingroup docview

*/
class A2DDOCVIEWDLLEXP a2dDocumentCommandProcessor: public a2dCommandProcessor
{
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    a2dInitCurrentSmartPointerOwner m_initCurrentSmartPointerOwner;
#endif
    DECLARE_DYNAMIC_CLASS( a2dDocumentCommandProcessor )

    DECLARE_EVENT_TABLE()

public:

    //!Constructor.
    /*!Create a document manager instance dynamically near the start of your application
    before doing any document or view operations.

    \param flags is currently unused.

    \param  initialize if true, the Initialize function will be called to create a default
            history list object. If you derive from a2dDocumentCommandProcessor, you may wish to call the base
            constructor with false, and then call Initialize in your own constructor, to allow your
            own Initialize or OnCreateFileHistory functions to be called.

    \param maxCommands maximum of commands on the command stack, default -1 is unlimited.
    */
    a2dDocumentCommandProcessor( long flags = a2dDEFAULT_DOCUMENT_FLAGS,
                                 bool initialize = true,
                                 int maxCommands = -1 );

    //! construct and initilize based on other.
    a2dDocumentCommandProcessor( a2dDocumentCommandProcessor* other );

    //! destructor
    ~a2dDocumentCommandProcessor();

    //! All menu's ( also using a2dMenuIdItem ) can be intercepted here
    /*!
        Calling functions like a2dDocumentFrame::AddCmdMenu(), will generate a menu event in
        a2dDocumentFrame, but often the event is not handled there, but redirected to this command porcessor.
        The idea is that the menu triggers a certain action. In case of an action on a a2dDocument, often
        this is done by submitting a a2dCommand. That action can be undone, if implemented.
        Other actions, only call a member function.
        Having a central entry point for actions, instead of calling the function directly, makes it easy
        to get a notification when an action was issued.
        If you want to know about a menu/action that is taken, make a dynamic connect to this class
        for the menu id in question. Do Skip the event in your handler, in order to continue processing here.
    */
    void OnMenu( wxCommandEvent& event );

    //! ask for a file using a file selector.
    /*!
        If your path contains internal variables, they will be expanded.
    */
    wxString AskFile( const wxString& message, const wxString& default_path = "",
                      const wxString& default_filename = "", const wxString& default_extension = "",
                      const wxString& wildcard = "*.*", int flags = 0,
                      int x = -1, int y = -1 )
    {
        wxWindow* parent = wxFindSuitableParent();

        wxString expandedPath = default_path;
        a2dPathList path;
        if ( !default_path.IsEmpty() && !path.ExpandPath( expandedPath ) )
        {
            a2dGeneralGlobals->ReportWarningF( a2dError_CouldNotEvaluatePath, _( "Could not expand %s" ), default_path.c_str() );
            return wxT( "" );
        }
        return wxFileSelector( message, expandedPath, default_filename, default_extension, wildcard, flags, parent, x, y );
    }

    //! Called by Exit()
    /*!
        Use e.g. to clean up modeless dialogs created from here.

        It is common practice to create ( via commands to this class ) tool dialogs, color dialogs modeless,
        meaning without parent window. You can use this central command processor as owner of such dialogs,
        by just storing them in a derived commands processor.
        The trick is to destroy those dialogs when the application terminates. Implementing this can be done
        by overriding this function here.
    */
    virtual void OnExit();

    //!Initializes data; currently just calls OnCreateFileHistory.
    /*! Some data cannot always be initialized in the constructor
        because the programmer must be given the opportunity
        to override functionality. If OnCreateFileHistory was
        called from the constructor, an overridden virtual OnCreateFileHistory
        would not be called due to C++'s 'interesting' constructor semantics.
    */
    virtual void Initialize();

    //! redirect the command to the current document ( if available )
    virtual bool SubmitToDocument( a2dCommand* command, bool storeIt = true );

    // Handlers for common user commands

    //!Closes and deletes the currently active document unless Close was vetod.
    bool FileClose( bool force = true );

    //! Creates a document from a list of templates (if more than one template).
    a2dError FileNew( a2dDocumentPtr& doc, a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE );

    //! Return the preferred document template for Opening files.
    a2dDocumentTemplate* GetPreferredOpenDocumentTemplate() { return m_preferredOpenTemplate; }

    //! Set the preferred document template for Opening files.
    void SetPreferredOpenDocumentTemplate( a2dDocumentTemplate* docTemplate ) { m_preferredOpenTemplate = docTemplate; }

    //! Creates a new document and reads in the selected file.
    /*!
        CreateDocument() is called with an empty string as path, and therefore a
        filedialog will be displayed, including the filters from templates, and
        you need to choose a file like that.
        If file is not empty, the file will silently be opened, using the right template
        based on file extension or template IoHandler.

        \param doc returns pointer to document in smart pointer.
        \param file specification for the file to load
        \param docTemplateFlags flags for templates, default visible templates.
    */
    a2dError FileOpen( a2dDocumentPtr& doc, const wxFileName& file = wxFileName( wxT( "" ) ), a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE | a2dTemplateFlag::LOAD );

    //! open one or more files using a file dialog
    /*!
        \param openPath default path to open the file dialog in.
        \param dialogFlags flags for file dialog
        \param docTemplateFlags flags for templates, default visible templates.
    */
    a2dError FilesOpen( const wxString& openPath = wxT( "" ), int dialogFlags = wxFD_MULTIPLE | wxFD_OPEN, a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE | a2dTemplateFlag::LOAD );

    //! Return existing document, or open it from file
    /*!
        Check document list for this file.
        If it not in the list, it will open the file.

        \param doc returns pointer to document in smart pointer.
        \param file specification for the file to load
        \param checkModification check if file on disk is newer
    */
    a2dError FileOpenCheck( a2dDocumentPtr& doc, const wxFileName& file, bool checkModification );

    //! revert the current document to the non saved document on disk.
    /*!
        Currently call Revert() on current document.
    */
    bool FileRevert();

    //! Saves the current document by calling wxDocument::Save for the current document.
    bool FileSave();

    //! Saves the documents by calling wxDocument::Save for each document.
    bool FileSaveAll();

    //! Calls wxDocument::SaveAs for the current document.
    /*!
        This to save a document to a file of the same type it was created with.

        \param file specification for the file to saveas
        \param flags document flags e.g. a2dREFDOC_SILENT does not show dialog first.
    */
    bool FileSaveAs( const wxFileName& file = wxFileName( wxT( "" ) ), a2dDocumentFlagMask flags = a2dREFDOC_NON );

    //! Return the preferred document template for Exporting files.
    a2dDocumentTemplate* GetPreferredExportDocumentTemplate() { return m_preferredExportTemplate; }

    //! Set the preferred document template for Exporting files.
    void SetPreferredExportDocumentTemplate( a2dDocumentTemplate* docTemplate ) { m_preferredExportTemplate = docTemplate; }

    //! Calls a2dDocument::Export for the current document.
    /*!
        This to export a document to a file of the chosen type.
        \param file specification for the file to export , no description means use file extension, if empty file path ask user.
        \param description description of a2dDocumentTemplate to use, if empty use filename ext to search template.
        \param flags for way of export a2dREFDOC_SILENT for saving without filedialog
    */
    bool FileExport( const wxFileName& file = wxFileName( wxT( "" ) ), const wxString& description = wxT( "" ), a2dDocumentFlagMask flags = a2dREFDOC_NON );

    //! Return the preferred document template for Importing files.
    a2dDocumentTemplate* GetPreferredImportDocumentTemplate() { return m_preferredImportTemplate; }

    //! Set the preferred document template for Importing files.
    void SetPreferredImportDocumentTemplate( a2dDocumentTemplate* docTemplate ) { m_preferredImportTemplate = docTemplate; }

    //! Calls a2dDocument::Import for the current document.
    /*!
        This to import into the current document the data stored in a file.
        \param file specification for the file to import , no description means use file extension, if empty file path ask user.
        \param description description of a2dDocumentTemplate to use, if empty use filename ext to search template.
        \param flags for way of export a2dREFDOC_SILENT for saving without filedialog
    */
    bool FileImport( const wxFileName& file = wxFileName( wxT( "" ) ), const wxString& description = wxT( "" ), a2dDocumentFlagMask flags = a2dREFDOC_NON );

    //! print the current active view.
    bool Print( a2dPrintWhat printWhat = a2dPRINT_Print );

    //! print preview of the current active view.
    bool Preview( a2dPrintWhat printWhat = a2dPRINT_Preview );

    //! printer setup the current active document or central command processor.
    bool PrintSetup( a2dPrintWhat printWhat );

    //! Return page setup data, as set in the current default printer.
    wxPageSetupDialogData GetDefaultPrintSettings();

    //! get printer setup the current active view or document or the one from central command processor.
    wxPageSetupDialogData* GetPrintSetup( a2dPrintWhat printWhat );

    //! default handler when a file could not be opened
    /*!
        The document is already created, and it depends on the document its a2dDocument::OnOpenDocument()
        how much of the wrong file is stored in it. When nothing is done, to store this document here in the command
        processor, it will automatically be released on return from this event.
        If you decide to display what has bin stored in the doc sofar, it can be done here.

        The default implementation closes all views that were created on the document, delete its contents,
        and generates an error log.
    */
    void OnCannotOpenDocument( a2dCommandEvent& event );

    //! Creates new documents in a manner determined by the flags parameter, which can be:
    /*!
        wxDOC_NEW Creates a fresh document.

        wxDOC_SILENT Silently loads the given document file.

        If wxDOC_NEW is present, a new document will be created and returned,
        possibly after asking the user for a template to use if there is more
        than one document template.

        If wxDOC_SILENT is present, a new document
        will be created and the given file loaded into it, using the first template which fits
        the file format to read, either based on extension or on the iohandler of the template.
        See FindTemplateForPath().

        If neither of these flags is present, the user will be presented with a file selector for
        the file to load, and the template to use will be determined by the
        extension (Windows) or by popping up a template choice list (other platforms).

        When a new document is created and added to the document list the event
        ::wxEVT_POST_CREATE_DOCUMENT is sent to the a2dDocumentTemplate::GetViewConnector().
        It depends on the type of a2dViewConnector what happens, e.g one could create
        an a2dView instance of the type chosen by the user from a list of a2dViewTemplates.
        The actual a2dView is created by calling the a2dViewTemplate::CreateView().
        And this last sends the ::wxEVT_POST_CREATE_VIEW event to the a2dViewConnector.
        And that is the time to set the created view into a a2dDocumentViewWindow and
        maybe create frames and windows first followed by setting the new view to one of its windows.
        Another  possibility is that the a2dViewConnector does not create new views, but instead of that uses
        existing views. The things happing in responds to ::wxEVT_POST_CREATE_DOCUMENT, is what makes this
        docview framework flexible. Replacing the a2dViewConnector is in general enough to swicth from
        on type of application (MDI SDI etc. ) to another.

        If the maximum number of documents has been reached,
        this function will delete the oldest currently loaded
        document before creating a new one.

        \param path path to file in case of flags != wxDOC_NEW and flags == wxDOC_SILENT
        \param documentflags Use at will
        \param wantedDocTemplate if given, only use this template to create new documents
        \param dialogflags if not a new document or creating silenet a document, this will be the flags towards the selection dialog
        \param docTemplateFlags template mask for document template, to filter shown templates.

        \return a2dError
    */
    virtual a2dError CreateDocuments(  const wxString& path,
                                       a2dDocumentFlagMask documentflags = a2dREFDOC_NON,
                                       a2dDocumentTemplate* wantedDocTemplate = NULL,
                                       int dialogflags = wxFD_OPEN,
                                       a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE
                                    );

    //!adds the given document, and creates a view for it.
    /*! If more than one view is allowed for the document
        (by virtue of multiple templates mentioning the same document type),
         a choice of view is presented to the user.

        \param newDoc document to add and  for which to create a new view
        \param viewTypeName is not empty, this type of view will be searched in the available templates
        \param documentflags Use at will, default a2dREFDOC_NEW
        \param docTemplateFlags template mask for document template, to filter shown templates.
        \param viewTemplateFlags template mask for view template, to filter shown templates.
    */
    a2dView* AddDocumentCreateView( a2dDocument* newDoc,
                                    const wxString& viewTypeName = wxT( "" ),
                                    a2dDocumentFlagMask documentflags = a2dREFDOC_NEW,
                                    a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE,
                                    a2dTemplateFlagMask viewTemplateFlags = a2dTemplateFlag::VISIBLE
                                  );

    //! add a in memory created document, but do not create a view.
    /*!
        The first document template with the same GetDocumentTypeName() as the document,
        is set to the document.
        The sentPreAddCreatedDocumentEvent can be sent to the a2dViewConnector via the document template,
        to take an action before the new document is add to the document list. For instance to close
        other files or disconnect views to use them again for the new document.
        The sentPostCreateDocumentEvent should be sent to use the a2dViewConnector system to
        create new frames with views.

        \param newDoc The document to be added.
        \param sentPreAddDocumentEvent if true this event is sent to document template
        \param sentPostCreateDocumentEvent if true this event is sent to document template
        \param documentflags Use at will, default a2dREFDOC_NEW
        \param docTemplateFlags template mask for document template, to filter shown templates.

        \return a2dError
    */
    a2dError AddCreatedDocument( a2dDocument* newDoc, bool sentPreAddCreatedDocumentEvent = false,
                             bool sentPostCreateDocumentEvent = false,
                             a2dDocumentFlagMask documentflags = a2dREFDOC_NEW,
                             a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE );

    //!Creates a new view for the given document.
    /*! If more than one view is allowed for the document
        (by virtue of multiple templates mentioning the same document type),
         a choice of view is presented to the user.

        \param doc document for which to create a new view
        \param viewTypeName is not empty, this type of view will be searched in the available templates
        \param flags Document flags
        \param mask mask for views to show.
    */
    virtual a2dView* CreateView( a2dDocument* doc, const wxString& viewTypeName = wxT( "" ),
                                 a2dDocumentFlagMask flags = a2dREFDOC_NON,
                                 a2dTemplateFlagMask viewTemplateFlags = a2dTemplateFlag::VISIBLE );

    //! remove/release a template
    /*!
        Keep in mind that templates or reference counted and placed in smart pointer lists.
        So releasing it here, might keep it intact when placed/used somewhere else
        ( e.g. a2dViewConnector ).
    */
    void ReleaseTemplate( a2dDocumentTemplate* temp, long flags = 0 );

    virtual bool FlushDoc( a2dDocument* doc );

    //! return template suitable for loading the file in path, using FindTemplateForPath.
    a2dDocumentTemplate* MatchTemplate( const wxString& path );

    //! pops up a file selector with optional a list of filters
    /*! corresponding to the document templates to choose from.

        Based on the list a2dDocumentTemplateList the list of file filters is setup.
        Next a file dialog is displayed, to select one or more file, depending on the dialogflags.
        In case of opening a file, it is checked for existence.

        The file dialog opens in path, after that
        path is set to the directory from which the file(s) were selected.

		\param title title for the dialog
        \param docTemplates the doctemplates which are used to present the file filters/types in the file dialog,
               and/or which are used to test the file chosen against.
        \param path initial directory where the dialog should be opened, returns chosen directory.
        \param flags flags = a2dREFDOC_NON
        \param selectedPaths the files choosen are stored in here
        \param chosenTemplate The a2dDocumentTemplate corresponding to the selected file's extension/filter or its iohandler, is returned.
        \param dialogflags may be a combination of wxFD_OPEN, wxFD_SAVE, wxFD_OVERWRITE_PROMPT, wxHIDE_READONLY, wxFD_FILE_MUST_EXIST.
        \param docTemplateFlags use only templates matching this mask
        \param preferedTemplate the template to show in the file selection dialog as default.

        \return a2dFileDialogReturn

        This function is used in a2dDocumentCommandProcessor::CreateDocument() and other locations, to
        present a dialog for loading and saving a document according to document templates.
    */
    virtual a2dError SelectDocumentPath( const wxString& title, const a2dDocumentTemplateList& docTemplates,
                                         wxString& path,
                                         a2dDocumentFlagMask flags,
                                         wxArrayString* selectedPaths,
                                         a2dDocumentTemplate** chosenTemplate,
                                         int dialogflags = wxFD_OPEN,
                                         a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE,
                                         const a2dDocumentTemplate* preferedTemplate  = NULL );

    //!Returns a document template by asking the user
    /*!(if there is more than one template). This function is used in a2dDocumentCommandProcessor::CreateDocument.

    \param  sort If more than one template
            then this parameter indicates whether the list of templates that the user
            will have to choose from is sorted or not when shown the choice box dialog.
            Default is false.
    \param docTemplateFlag template filter
    */
    virtual a2dDocumentTemplate* SelectDocumentType( bool sort = false,
            a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE );

    //! function used in CreateDocument() when a2dREFDOC_SILENT is used for creating it.
    /*!
        It iterates over the document templates, to find a template for the file
        that is to be loaded. To test each template it uses a2dDocumentTemplate::FileMatchesTemplate()
        The first matching template is returned. This is based on the file extension, or if not given on
        the input iohandler of a template, which can load the file.

        \param docTemplates the templates to test the filepath against.
        \param path input filename to an existing file, for which we want to find a template
        \param docTemplateFlag template filter

        \return The first matching template is returned.
    */
    virtual a2dDocumentTemplate* FindTemplateForPath( const a2dDocumentTemplateList& docTemplates, const wxString& path, a2dTemplateFlagMask mask = a2dTemplateFlag::VISIBLE );

    //!Returns a view template by asking the user
    /*!(if there is more than one template), displaying a list of valid views.

    This function is used in a2dDocumentCommandProcessor::CreateView. The dialog normally will
    not appear because the array of templates only contains those relevant to the
    document in question, and often there will only be one such.

    a2dViewConnector::CreateView() also uses his function to present the templates
    a2dViewTemplate's associated with a a2dViewConnector, or if not to present all
    a2dViewTemplate from the a2dDocumentCommandProcessor itself.

        \param doc the document to select a viewtype for.
        \param list template list to choose  a view template from
        \param viewTypeName the name of the view, if empty a list is presented, else a check is done
        if the given name is indeed a view template.

        \param sort If more than one template, then this parameter
        indicates whether the list of templates that the user will have to choose from is
        sorted or not when shown the choice box dialog. Default is false.
        \param mask which view are allowed.
    */
    virtual a2dViewTemplate* SelectViewType( a2dDocument* doc,
            const a2dViewTemplateList& list,
            const wxString& viewTypeName = wxT( "" ),
            bool sort = false,
            a2dTemplateFlagMask viewTemplateFlags = a2dTemplateFlag::VISIBLE );

    //! add a reference to a a2dDocumentTemplate to the a2dDocumentTemplateList
    void AssociateDocTemplate( a2dDocumentTemplate* temp );

    //! remove a reference to a a2dDocumentTemplate to the a2dDocumentTemplateList
    void DisassociateDocTemplate( a2dDocumentTemplate* temp );

    //! add a reference to a a2dViewTemplate to the wxViewTemplateReflist
    void AssociateViewTemplate( a2dViewTemplate* temp );

    //! remove a reference to a a2dViewTemplate to the wxViewTemplateReflist
    void DisassociateViewTemplate( a2dViewTemplate* temp );

    //! set the current document, only needed in cases
    /*!  where it was not set right already (rarely), and you need to have it set
        to be able to use a2dDocumentCommandProcessor to get to a document.

        It also used internal, in order to sent the ::wxEVT_CHANGED_DOCUMENT event.
        This event can be intercepted by other classes, when they need to know this.
        \see GetCurrentDocument()
    */
    void SetCurrentDocument( a2dDocument* document );

    //! Get the current Document, which is the document that was last accessed from a view.
    /*!
        The current document is the document that is set when:
        -When a view is activated (has focus in case of a wxWindow), via SetActive()
        -Last document added, but which does not have a view yet, via AddDocument()
        -When set from the outside with SetCurrentDocument()

        When a a2dView is deactivated, the current document
        is not modified until another view is activated. So you can still get to the document
        if the active view is deactivated. This happens for instance when a modeless dialog is activated.

        The current document is used by a2dDocumentCommandProcessor to issue commands
        on the document its private commandprocessor.
        And only in very special cases you will need to set it yourself, normally it will
        be set to the view which has the focus.
    */
    a2dDocument* GetCurrentDocument() const;

    //! get the command processor of the current document.
    /*!
        When the current document is set and it has a commandprocessor, this will be returned.
    */
    a2dCommandProcessor* GetCurrentDocumentCommandProcessor() const;

    //!Sets the maximum number of documents that can be open at a time.
    /*!By default, this is 10,000. If you set it to 1, existing documents will be saved and
        deleted when the user tries to open or create a new one (similar to the behaviour of
        Windows Write, for example). Allowing multiple documents gives behaviour more akin to
        MS Word and other Multiple Document Interface applications.
    */
    void SetMaxDocsOpen( size_t n ) { m_maxDocsOpen = n; }

    //!Gets the maximum number of documents that can be open at a time.
    size_t GetMaxDocsOpen() const { return m_maxDocsOpen; }

    //! Add  a document to the manager's list
    /*!
        The a2dCommandEvent of type ::wxEVT_ADD_DOCUMENT is sent, this can be used to do some action
        after a new document is added. (e.g add to overview window of all open documents)
    */
    void AddDocument( a2dDocument* doc );

    //! remove a document from the manager's list
    /*!
        The a2dCommandEvent of type ::wxEVT_REMOVE_DOCUMENT is sent, this can be used to do some action
        after a document is removed. (e.g remove from an overview window of all open documents)
    */
    void RemoveDocument( a2dDocument* doc );

    //! closes all currently open documents
    /*!
     If force is true the closing of the document can not be vetod.
     If force is false only the documents which did not veto the close are Closed.

     \remark closing a document means its views are closed, by calling a2dView::Close()

     \return If Close of a document is vetod, returns false else true.

    */
    virtual bool CloseDocuments( bool force = true );

    //! Clear remaining documents and templates
    /*!
        releases templates and documents, the documents are first closed, if vetod,
        the clearing of documents and templates is skipped, and return is false.
    */
    bool Clear( bool force = true );

    //! Called to Exit the application properly.
    /*!
        Set m_busyExit, and next calls CleanUp() and Clear( true ).

        One normally exits the main event loop (and the application) by deleting the top window.
        In that case this function is called automatically.
        But in case of a script command this function makes sure the top window is closed.
    */
    bool Exit( bool force = true );

    //! set this when you are exiting the application
    //! When this class is destructed, it is set automatically.
    void SetBusyExit( bool exitBusy ) { m_busyExit = exitBusy; }

    //! return true if the application is bussy exiting.
    bool GetBusyExit() { return m_busyExit; }

    //! set if a file can be opened twice in the framework, is true, the second time a file is openened,
    //! it is treated as a new document internal. If false, openeing the file again, the already existing 
    //! document is used.
    void SetAllowOpenSameFile( bool allow ) { m_allowOpenSameFile = allow; }

    //! \see SetAllowOpenSameFile()
    bool GetAllowOpenSameFile() const { return m_allowOpenSameFile; }

    //! return the one that is active right now (e.g. has focus in case of a wxWindow), or NULL
    a2dView* GetCurrentView() const;

    //! returns a reference  to the a2dDocumentList, which contains all a2dDocument's that are open.
    const a2dDocumentList& GetDocuments() const { return m_docs; }

    //! returns a reference  to the a2dDocumentTemplateList, which contains all a2dDocumentTemplate's.
    const a2dDocumentTemplateList& GetDocTemplates() { return m_docTemplates; }

    //! returns a reference  to the a2dViewTemplateList, which contains all a2dViewTemplate's.
    const a2dViewTemplateList& GetViewTemplates() { return m_viewTemplates; }

    //! Make a default document name
    virtual bool MakeDefaultName( wxString& buf );

    //! Make a frame title (override this to do something different)
    /*!
        This method asks the document for its printable name and adds this
        and the application name to the returned string.
        If no document is given the returned string is the application name.
        The returned string will have a format like "Printable name - Application name"

        \remark
        If you use the modified indicator you've to update the frame title (call this
        method again) if you save the document, otherwise the modified indicator will
        stay at your frame.
        You may want to catch the ::wxEVT_CHANGEDMODIFY_DOCUMENT event to update the frame title.

        \param doc               The document to ask for the printable name or <code>NULL</code>
        \param modifiedIndicator If set (i.e. a "*"), the indicator will be attached to the printable name if doc is modified
    */
    virtual wxString MakeFrameTitle( a2dDocument* doc, const wxString& modifiedIndicator = wxT( "" ) );

    //! A hook to allow a derived class to create a different type of file history. Called from Initialize.
    virtual a2dFileHistory* OnCreateFileHistory();

    //! returns a pointer to the file history list
    virtual a2dFileHistory* GetFileHistory() const { return m_fileHistory; }

    //! File history management
    virtual void AddFileToHistory( const wxFileName& file, a2dDocumentTemplate* docTemplate = NULL, a2dViewTemplate* viewTemplate = NULL );

    //! Remove a file from history
    virtual void RemoveFileFromHistory( size_t i );

    //! Return number fo files in history
    virtual size_t GetHistoryFilesCount() const;

    //! return the a2dFileHistoryItem of the i'th file in the history
    virtual a2dFileHistoryItem* GetHistoryFileItem( size_t i ) const;

    //! return the filename of the i'th file in the history
    virtual wxString GetHistoryFile( size_t i ) const;

    //! Use this menu for appending recently-visited document filenames,
    /*! for convenient access. Calling this function with a valid menu
        pointer enables the history list functionality.

        Note that you can add multiple menus using this function,
        to be managed by the file history object.
    */
    virtual void FileHistoryUseMenu( wxMenu* menu );

    //! Removes the given menu from the list of menus managed by the file history object.
    virtual void FileHistoryRemoveMenu( wxMenu* menu );

#if wxUSE_CONFIG
    //! Loads the file history from a config object.
    virtual void FileHistoryLoad( wxConfigBase& config );
    //! Saves the file history into a config object. This must be called explicitly by the application.
    virtual void FileHistorySave( wxConfigBase& config );
#endif // wxUSE_CONFIG

    //! Appends the files in the history list, to all menus managed by the file history object
    virtual void FileHistoryAddFilesToMenu();

    //! Appends the files in the history list, to the given menu only.
    virtual void FileHistoryAddFilesToMenu( wxMenu* menu );

    //! Gets the directory to be displayed to the user when opening a file. Initially this is empty.
    inline wxString GetLastDirectory() const { return m_lastDirectory; }

    //! Sets the directory to be displayed to the user when opening a file. Initially this is empty.
    inline void SetLastDirectory( const wxString& dir ) { m_lastDirectory = dir; }

    //! To set the curent view.
    /*!
        In general you will get two events, first for the view that is deactivated, and next
        for the new active view.
        m_currentView is set to the last activated or deactivated view.

        \param view if not NULL, the new active view is set to this view, else NULL.
    */
    virtual void SetCurrentView( a2dView* view );

    //! Views do inform the document manager when a view will be destroyed.
    /*!
        The function sets the m_currentView to NULL, if eqaul to the view destroyed.
        \param view if not NULL, set the m_currentView to NULL if equal to view. If view is NULL set the m_currentView to NULL.
    */
    void CheckCurrentView( a2dView* view );

#if wxUSE_PRINTING_ARCHITECTURE
    wxPageSetupDialogData* GetPageSetupData( void ) const { return m_pageSetupData; }
    void SetPageSetupData( const wxPageSetupDialogData& pageSetupData ) { *m_pageSetupData = pageSetupData; }
    void SetPageSetupData( wxPageSetupDialogData* pageSetupData );
#endif

    //!process an event with a protection of repeated working in MDI by klion
    //! .
    /*!
    \param event event that is to be processed by a2dDocumentCommandProcessor.
    */
    virtual bool ProcessEvent( wxEvent& event );

    //! set undo storage or not
    void SetUndo( bool withUndo ) { m_withUndo = withUndo; }

    //! get undo storage setting
    bool GetUndo() { return m_withUndo; }

protected:

    //! default handler for GUI event with id wxID_UNDO
    void OnUndo( wxCommandEvent& event );

    //! default handler for GUI event with id wxID_REDO
    void OnRedo( wxCommandEvent& event );

protected:
    //! Handlers for UI update commands
    void OnUpdateFileOpen( wxUpdateUIEvent& event );
    //! Handlers for UI update commands
    void OnUpdateFileClose( wxUpdateUIEvent& event );
    //! Handlers for UI update commands
    void OnUpdateFileCloseAll( wxUpdateUIEvent& event );
    //! Handlers for UI update commands
    void OnUpdateFileRevert( wxUpdateUIEvent& event );
    //! Handlers for UI update commands
    void OnUpdateFileNew( wxUpdateUIEvent& event );
    //! Handlers for UI update commands
    void OnUpdateFileSave( wxUpdateUIEvent& event );
    //! Handlers for UI update commands
    void OnUpdateFileSaveAll( wxUpdateUIEvent& event );
    //! Handlers for UI update commands
    void OnUpdateFileSaveAs( wxUpdateUIEvent& event );
    //! Handlers for UI update commands
    void OnUpdateCreateView( wxUpdateUIEvent& event );

    //! Handlers for UI update commands
    void OnUpdateUndo( wxUpdateUIEvent& event );
    //! Handlers for UI update commands
    void OnUpdateRedo( wxUpdateUIEvent& event );

    //! Handlers for UI update commands
    void OnUpdatePrint( wxUpdateUIEvent& event );
    //! Handlers for UI update commands
    void OnUpdatePrintSetup( wxUpdateUIEvent& event );
    //! Handlers for UI update commands
    void OnUpdatePreview( wxUpdateUIEvent& event );

    //! Views do inform the document manager
    /*! when a view is (in) activated ( the window containing the view is in or out of focus )
        The default handler sets the active view for the document manager.
        The current active view, is set to the last view which did sent an activate event
        with value true.
        If the view sending the event is the current and the event value is false,
        the current view is set to NULL.
        The last view that was m_currentView until now, is set inactive from here
        by calling a2dView::Activate().
    */
    void OnActivateView( a2dViewEvent& viewevent );

    //! Views do inform the document manager when a view will be removed.
    /*!
        The default handler sets the m_lastView and/or
        m_currentView to NULL, if eqaul to the view removed.
    */
    void OnRemoveView( a2dDocumentEvent& viewevent );

    //!It works when a2dView::SetDocument() executing
    void OnAddView( a2dDocumentEvent& docevent );

    //! user flags use at will ( not used internal )
    long m_flags;

    //! to create unique new names for file
    int m_defaultDocumentNameCounter;

    //! the maximum of documents allowed open
    size_t m_maxDocsOpen;

    //! list of all that are open
    a2dDocumentList m_docs;

    //! templates for documents
    a2dDocumentTemplateList m_docTemplates;

    //! templates for views
    a2dViewTemplateList m_viewTemplates;

    //! the current view (active or inactive)
    a2dView*        m_currentView;

    //! the current active document
    a2dDocument*    m_currentDocument;

    //! the file history
    a2dFileHistory*    m_fileHistory;

    //! the last visited directory
    wxString          m_lastDirectory;

    //! preferred document template for Importing files.
    a2dDocumentTemplate* m_preferredImportTemplate;

    //! preferred document template for Exporting files.
    a2dDocumentTemplate* m_preferredExportTemplate;

    //! preferred document template for Opening files.
    a2dDocumentTemplate* m_preferredOpenTemplate;

    //! if set, for commands which can undo, will be submitted like that.
    bool m_withUndo;

    //! set when terminating application
    bool m_busyExit;

    bool m_allowOpenSameFile;

#if wxUSE_PRINTING_ARCHITECTURE
    //! this is the global printer page setup data for printer
    wxPageSetupDialogData*    m_pageSetupData;
#endif

};

//! One Global instance of this class exists, in order to get to
/*!  the global a2dDocumentCommandProcessor.

    -a2dDocumentCommandProcessor handles commands for opening new a2dDocument's and a2dView's.

    The idea is that the a2dDocumentCommandProcessor is the central place for checking errors,
    and decide what to do with them.

    See base class for more.

    \ingroup docview global
*/
class A2DDOCVIEWDLLEXP a2dDocviewGlobal : public a2dGeneralGlobal
{

public:

    //! constructor
    a2dDocviewGlobal();

    //! destructor
    ~a2dDocviewGlobal();

    //! Gets a2dDocumentCommandProcessor pointer
    a2dDocumentCommandProcessor* GetDocviewCommandProcessor() const { return m_docviewCommandProcessor.Get(); }

    //! Normally the docview commandprocessor is set in the constructor when creating new a2dDocviewGlobal
    void SetDocviewCommandProcessor( a2dDocumentCommandProcessor* docviewCommandProcessor );

    //! to sent a ::wxEVT_RECORD event
    /*!
        Record events or used for sending command strings, typical used for a commandline window or
        for recording macros to a file, which later can be replayed.

        The Format string is the sam as in Printf, but for %f %d the accuracy may be set internal.
    */
    virtual void RecordF( wxObject* sender, const wxChar* Format, ... );

    virtual void RecordF( const wxChar* Format, ... );

private:

    //! the only docview CommandProcessor central to the application.
    a2dSmrtPtr<a2dDocumentCommandProcessor> m_docviewCommandProcessor;
};

//! global object to get to the only a2dDocviewGlobal
A2DDOCVIEWDLLEXP_DATA( extern a2dDocviewGlobal* ) a2dDocviewGlobals;

// ----------------------------------------------------------------------------
// File history management
// ----------------------------------------------------------------------------

//! holds one file for placing in history list
/*!
    Next to the filename, also the template used for loading the file is stored here.
*/
class A2DDOCVIEWDLLEXP a2dFileHistoryItem : public a2dObject
{
public:
    a2dFileHistoryItem( const wxFileName& filename, a2dDocumentTemplate* docTemplate, a2dViewTemplate* viewTemplate )
    {
        m_docTemplate = docTemplate;
        m_viewTemplate = viewTemplate;
        m_filename = filename;
    }

    a2dDocumentTemplatePtr m_docTemplate;
    a2dViewTemplatePtr m_viewTemplate;
    wxFileName m_filename;

private:

    //! create an exact copy of this property
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const 
    { return new a2dFileHistoryItem( m_filename, m_docTemplate, m_viewTemplate ); };

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite ) {}
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts ) {}
#endif //wxART2D_USE_CVGIO

};

typedef a2dSmrtPtr<a2dFileHistoryItem> a2dFileHistoryItemPtr;

//! list of a2dFileHistoryItem
typedef a2dSmrtPtrList<a2dFileHistoryItem> a2dFileHistoryItemList;

class A2DDOCVIEWDLLEXP a2dFileHistory : public wxObject
{
public:
    a2dFileHistory( size_t maxFiles = 9, wxWindowID idBase = wxID_FILE1 );
    virtual ~a2dFileHistory();

    // Operations
    virtual void AddFileToHistory( const wxFileName& file, a2dDocumentTemplate* docTemplate = NULL, a2dViewTemplate* viewTemplate = NULL );
    virtual void RemoveFileFromHistory( size_t i );
    virtual int GetMaxFiles() const { return ( int )m_fileMaxFiles; }
    virtual void UseMenu( wxMenu* menu );

    // Remove menu from the list (MDI child may be closing)
    virtual void RemoveMenu( wxMenu* menu );

#if wxUSE_CONFIG
    virtual void Load( wxConfigBase& config );
    virtual void Save( wxConfigBase& config );
#endif // wxUSE_CONFIG

    virtual void AddFilesToMenu();
    virtual void AddFilesToMenu( wxMenu* menu ); // Single menu

    // Accessors
    virtual wxString GetHistoryFile( size_t i ) const;
    a2dFileHistoryItem* GetHistoryFileItem( size_t i ) const;
    virtual size_t GetCount() const { return m_fileHistoryList.size(); }

    const wxList& GetMenus() const { return m_fileMenus; }

#if wxABI_VERSION >= 20802
    // Set/get base id
    void SetBaseId( wxWindowID baseId ) { m_idBase = baseId; }
    wxWindowID GetBaseId() const { return m_idBase; }
#endif // wxABI 2.8.2+

protected:

    // file history list
    a2dFileHistoryItemList m_fileHistoryList;

    // Menus to maintain (may need several for an MDI app)
    wxList            m_fileMenus;
    // Max files to maintain
    size_t            m_fileMaxFiles;

private:
    // The ID of the first history menu item (Doesn't have to be wxID_FILE1)
    wxWindowID m_idBase;

    DECLARE_DYNAMIC_CLASS( a2dFileHistory )
    DECLARE_NO_COPY_CLASS( a2dFileHistory )
};


//--------------------------------------------------------------------
// a2dDocviewModule
//--------------------------------------------------------------------

//! A module to initialize the docview framework.
/*!
   The docview framework uses one global class to get acces to the event distibuter and a central commandprocessor.

   It is called a2dDocviewGlobals and is the only global instance of a2dDocviewGlobal.

    \ingroup global
*/
class A2DDOCVIEWDLLEXP a2dDocviewModule : public wxModule
{
public:

    //! constructor
    a2dDocviewModule()
    {
#if wxUSE_PRINTING_ARCHITECTURE
        m_wxThePrintPaperDatabase = NULL;
#endif
    }

    virtual bool OnInit();
    virtual void OnExit();


private:

    DECLARE_DYNAMIC_CLASS( a2dDocviewModule )

#if wxUSE_PRINTING_ARCHITECTURE
    wxPrintPaperDatabase* m_wxThePrintPaperDatabase;
#endif
};

#endif // _WX_DOCCOMH__



