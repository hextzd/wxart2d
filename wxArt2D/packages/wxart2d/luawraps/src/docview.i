
// *****************************************************************
// wx/docview\cparser.h
// *****************************************************************

typedef unsigned int a2dTemplateFlagMask;
typedef unsigned int a2dDocumentFlagMask;

class %delete a2dDocumentPtr
{
    a2dDocument* Get() const
};

class %delete a2dViewPtr
{
    a2dView* Get() const
};

class %delete a2dDocumentTemplatePtr
{
    a2dDocumentTemplate* Get() const
};

class %delete a2dViewTemplatePtr
{
    a2dViewTemplate* Get() const
};

// ---------------------------------------------------------------------------
// a2dFileHistoryItem
// ---------------------------------------------------------------------------

class %delete a2dFileHistoryItem : public a2dObject
{
    a2dFileHistoryItem( const wxFileName& filename, a2dDocumentTemplate* docTemplate, a2dViewTemplate* viewTemplate )

    %member a2dDocumentTemplatePtr m_docTemplate;
    %member a2dViewTemplatePtr m_viewTemplate;
    %member wxFileName m_filename;
};

// ---------------------------------------------------------------------------
// a2dFileHistory
// ---------------------------------------------------------------------------

class %delete a2dFileHistory : public wxObject
{
    a2dFileHistory(int maxFiles = 9, wxWindowID idBase = wxID_FILE1)

    void AddFileToHistory( const wxFileName& file, a2dDocumentTemplate* docTemplate = NULL, a2dViewTemplate* viewTemplate = NULL )
    void AddFilesToMenu()
    void AddFilesToMenu(wxMenu* menu)
    a2dFileHistoryItem* GetHistoryFileItem(size_t i) const;
    int GetMaxFiles() const
    size_t GetCount() const
    void Load(wxConfigBase& config)
    void RemoveFileFromHistory(size_t i)
    void RemoveMenu(wxMenu* menu)
    void Save(wxConfigBase& config)
    void UseMenu(wxMenu* menu)
    
};



// ---------------------------------------------------------------------------
// a2dDocumentList
// ---------------------------------------------------------------------------

class a2dDocumentListIter
{
    a2dDocumentListIter()
    
        %operator %rename GetItem a2dDocumentPtr operator*() const
        
        %operator a2dDocumentListIter& operator=(const a2dDocumentListIter& other)
        
        %operator bool operator ==( const a2dDocumentListIter& iter ) const
        %operator bool operator !=( const a2dDocumentListIter& iter ) const
        
		//%operator a2dDocumentListIter& operator++()
		//%operator a2dDocumentListIter& operator++( int )
       
		//%operator a2dDocumentListIter& operator--()
		//%operator a2dDocumentListIter& operator--(int)
    
};

class a2dDocumentList
{

    %rename lbegin a2dDocumentListIter begin()
    %rename lend   a2dDocumentListIter end()

    bool empty()
    size_t size()
	a2dDocument* front()
	a2dDocument* back()
	void push_front( a2dDocument* obj )
	void pop_front()
	void push_back( a2dDocument* obj )
	void pop_back()
    
};

// ---------------------------------------------------------------------------
// a2dViewList
// ---------------------------------------------------------------------------

class a2dViewListIter
{
    a2dViewListIter()
    
        %operator %rename GetItem a2dViewPtr operator*() const
        
        %operator a2dViewListIter& operator=(const a2dViewListIter& other)
        
        %operator bool operator ==( const a2dViewListIter& iter ) const
        %operator bool operator !=( const a2dViewListIter& iter ) const
        
		//%operator a2dViewListIter& operator++()
		//%operator a2dViewListIter& operator++( int )
       
		//%operator a2dViewListIter& operator--()
		//%operator a2dViewListIter& operator--(int)
    
};

class a2dViewList
{

    %rename lbegin a2dViewListIter begin()
    %rename lend   a2dViewListIter end()

    bool empty()
    size_t size()
	a2dView* front()
	a2dView* back()
	void push_front( a2dView* obj )
	void pop_front()
	void push_back( a2dView* obj )
	void pop_back()
    
};

// ---------------------------------------------------------------------------
// a2dViewTemplateList
// ---------------------------------------------------------------------------

class a2dViewTemplateListIter
{
    a2dViewTemplateListIter()
    
        %operator %rename GetItem a2dViewTemplatePtr operator*() const
        
        %operator a2dViewTemplateListIter& operator=(const a2dViewTemplateListIter& other)
        
        %operator bool operator ==( const a2dViewTemplateListIter& iter ) const
        %operator bool operator !=( const a2dViewTemplateListIter& iter ) const
        
		//%operator a2dViewTemplateListIter& operator++()
		//%operator a2dViewTemplateListIter& operator++( int )
       
		//%operator a2dViewTemplateListIter& operator--()
		//%operator a2dViewTemplateListIter& operator--(int)
    
};

class a2dViewTemplateList
{
    %rename lbegin a2dViewTemplateListIter begin()
    %rename lend   a2dViewTemplateListIter end()

    bool empty()
    size_t size()
	a2dViewTemplate* front()
	a2dViewTemplate* back()
	void push_front( a2dViewTemplate* obj )
	void pop_front()
	void push_back( a2dViewTemplate* obj )
	void pop_back()
    
};

// ---------------------------------------------------------------------------
// a2dDocumentTemplateList
// ---------------------------------------------------------------------------

class a2dDocumentTemplateListIter
{
    a2dDocumentTemplateListIter()
    
        %operator %rename GetItem a2dDocumentTemplatePtr operator*() const
        
        %operator a2dDocumentTemplateListIter& operator=(const a2dDocumentTemplateListIter& other)
        
        %operator bool operator ==( const a2dDocumentTemplateListIter& iter ) const
        %operator bool operator !=( const a2dDocumentTemplateListIter& iter ) const
        
		//%operator a2dDocumentTemplateListIter& operator++()
		//%operator a2dDocumentTemplateListIter& operator++( int )
       
		//%operator a2dDocumentTemplateListIter& operator--()
		//%operator a2dDocumentTemplateListIter& operator--(int)
    
};

class a2dDocumentTemplateList
{
    %rename lbegin a2dDocumentTemplateListIter begin()
    %rename lend   a2dDocumentTemplateListIter end()

    bool empty()
    size_t size()
	a2dDocumentTemplate* front()
	a2dDocumentTemplate* back()
	void push_front( a2dDocumentTemplate* obj )
	void pop_front()
	void push_back( a2dDocumentTemplate* obj )
	void pop_back()
    
};



// *****************************************************************
// wx/docview\doccom.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dTemplateFlag
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dTemplateFlag
{
    %member static const a2dTemplateFlagMask NON
    %member static const a2dTemplateFlagMask VISIBLE
    %member static const a2dTemplateFlagMask MARK
    %member static const a2dTemplateFlagMask LOAD
    %member static const a2dTemplateFlagMask SAVE
    %member static const a2dTemplateFlagMask IMPORTING
    %member static const a2dTemplateFlagMask EXPORTING
    %member static const a2dTemplateFlagMask ALL
    %member static const a2dTemplateFlagMask DEFAULT
    %member static const a2dTemplateFlagMask LOAD_IMPORT
    %member static const a2dTemplateFlagMask SAVE_EXPORT
    a2dTemplateFlag( a2dTemplateFlagMask newmask = a2dTemplateFlag::NON )
    bool CheckMask( a2dTemplateFlagMask mask )
    void SetFlags( a2dTemplateFlagMask which )
    a2dTemplateFlagMask GetFlags() const
    bool GetFlag( a2dTemplateFlagMask which ) const
};

// ---------------------------------------------------------------------------
// a2dDocumentCommandProcessor
// ---------------------------------------------------------------------------
enum a2dPrintWhat
{
    a2dPRINT_Print,
    a2dPRINT_Preview,  
    a2dPRINT_PrintView,  
    a2dPRINT_PreviewView,   
    a2dPRINT_PrintDocument,   
    a2dPRINT_PreviewDocument,      
    a2dPRINT_PrintSetup
};

#include "wx/docview/doccom.h"
class a2dDocumentCommandProcessor : public a2dCommandProcessor
{
    a2dDocumentCommandProcessor(long flags = a2dDEFAULT_DOCUMENT_FLAGS,bool initialize = true,int maxCommands = -1 )
    a2dDocumentCommandProcessor( a2dDocumentCommandProcessor* other )
    virtual void OnExit()
    virtual void Initialize()
    virtual bool SubmitToDocument( a2dCommand* command, bool storeIt = true )
    bool FileClose( bool force = true )
    a2dError FileNew( a2dDocumentPtr& doc, a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE )
    a2dError FileOpen( a2dDocumentPtr& doc, const wxFileName& file, a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE )
    a2dError FilesOpen( const wxString& openPath = "", int dialogFlags = wxFD_MULTIPLE | wxFD_OPEN, a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE )
    bool FileRevert()
    bool FileSave()
    bool FileSaveAs( const wxFileName& file , a2dDocumentFlagMask flags = a2dREFDOC_NON )
    bool FileExport( const wxFileName& file , const wxString& description = "", a2dDocumentFlagMask flags = a2dREFDOC_NON )
    bool FileImport( const wxFileName& file , const wxString& description = "", a2dDocumentFlagMask flags = a2dREFDOC_NON )
    bool Print( a2dPrintWhat printWhat )
    bool Preview( a2dPrintWhat printWhat )
    bool PrintSetup( a2dPrintWhat printWhat )
    void OnCannotOpenDocument( a2dCommandEvent& event )
    virtual a2dError CreateDocuments(  const wxString& path,a2dDocumentFlagMask flags = a2dREFDOC_NON,a2dDocumentTemplate* wantedDocTemplate = NULL,int dialogflags = wxFD_OPEN,a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE
    a2dView* AddDocumentCreateView( a2dDocument* newDoc,const wxString& viewTypeName = "",a2dDocumentFlagMask flags = a2dREFDOC_NON )
    void AddCreatedDocument( a2dDocument* newDoc, bool sentEvent = false )
    virtual a2dView *CreateView( a2dDocument *doc, const wxString& viewTypeName = "",a2dDocumentFlagMask flags = a2dREFDOC_NON,a2dTemplateFlagMask viewTemplateFlags = a2dTemplateFlag::VISIBLE )
    void ReleaseTemplate(a2dDocumentTemplate *temp, long flags = 0)
    virtual bool FlushDoc(a2dDocument *doc)
    a2dDocumentTemplate *MatchTemplate(const wxString& path)
    virtual a2dError SelectDocumentPath( const wxString& title, const a2dDocumentTemplateList& docTemplates, wxString& path,a2dDocumentFlagMask flags,wxArrayString* selectedPaths,a2dDocumentTemplate** chosenTemplate,int dialogflags = wxFD_OPEN,a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE,const a2dDocumentTemplate* preferedTemplate  = NULL )
    virtual a2dDocumentTemplate *SelectDocumentType( bool sort = false,a2dTemplateFlagMask docTemplateFlags = a2dTemplateFlag::VISIBLE )
    virtual a2dDocumentTemplate *FindTemplateForPath( const a2dDocumentTemplateList& docTemplates, const wxString& path, a2dTemplateFlagMask mask = a2dTemplateFlag::VISIBLE )
    virtual a2dViewTemplate *SelectViewType( a2dDocument* doc,const a2dViewTemplateList& list,const wxString& viewTypeName = "",bool sort = false,a2dTemplateFlagMask viewTemplateFlags = a2dTemplateFlag::VISIBLE )
    void AssociateDocTemplate( a2dDocumentTemplate *temp )
    void DisassociateDocTemplate( a2dDocumentTemplate *temp )
    void AssociateViewTemplate( a2dViewTemplate *temp )
    void DisassociateViewTemplate( a2dViewTemplate *temp )
    void SetCurrentDocument( a2dDocument* document )
    a2dDocument* GetCurrentDocument() const
    a2dCommandProcessor* GetCurrentDocumentCommandProcessor() const
    void SetMaxDocsOpen( size_t n)
    size_t GetMaxDocsOpen() const
    void AddDocument(a2dDocument *doc)
    void RemoveDocument(a2dDocument *doc)
    bool CloseDocuments(bool force = true)
    bool Clear(bool force = true)
    bool Exit( bool force = true )
    void SetBusyExit( bool exitBusy )
    bool GetBusyExit()
    a2dView *GetCurrentView() const
    const a2dDocumentList& GetDocuments() const
    const a2dDocumentTemplateList& GetDocTemplates()
    const a2dViewTemplateList& GetViewTemplates()
    virtual bool MakeDefaultName(wxString& buf)
    virtual wxString MakeFrameTitle(a2dDocument* doc, const wxString& modifiedIndicator = "")
    virtual a2dFileHistory *OnCreateFileHistory()
    virtual a2dFileHistory *GetFileHistory() const
    virtual void AddFileToHistory(const wxString& file)
    virtual void RemoveFileFromHistory(size_t i)
    virtual size_t GetHistoryFilesCount() const
    virtual wxString GetHistoryFile(size_t i) const
    virtual void FileHistoryUseMenu(wxMenu *menu)
    virtual void FileHistoryRemoveMenu(wxMenu *menu)
    virtual void FileHistoryLoad(wxConfigBase& config)
    virtual void FileHistorySave(wxConfigBase& config)
    virtual void FileHistoryAddFilesToMenu()
    virtual void FileHistoryAddFilesToMenu(wxMenu* menu)
    wxString GetLastDirectory() const
    void SetLastDirectory(const wxString& dir)
    void SetCurrentView( a2dView* view )
    void CheckCurrentView( a2dView* view )
    // wxPageSetupData* GetPageSetupData() const
    // void SetPageSetupData(const wxPageSetupData& pageSetupData)
    virtual bool ProcessEvent(wxEvent& event)
};

// ---------------------------------------------------------------------------
// a2dCommandEvent
// ---------------------------------------------------------------------------

#include "wx/docview/doccom.h"
class a2dCommandEvent : public a2dCommandProcessorEvent
{
    a2dCommandEvent( wxEventType type, a2dDocument* doc )
    a2dCommandEvent( wxEventType type, a2dCommand* cmd, a2dDocument* doc = NULL )
    a2dCommandEvent( a2dCommand *cmd,const wxString& undoLabel, bool canUndo,const wxString& redoLabel, bool canRedo,a2dDocument* doc = NULL
    a2dCommandEvent( const wxString& record )
    a2dCommandEvent(const a2dCommandEvent & event)
    a2dDocument* GetDocument()
    a2dCommand* GetCommand()
    wxString& GetRecord()
};

// ---------------------------------------------------------------------------
// a2dDocviewGlobal
// ---------------------------------------------------------------------------

#include "wx/docview/doccom.h"
class a2dDocviewGlobal : public a2dGeneralGlobal
{
    a2dDocviewGlobal()
    a2dDocumentCommandProcessor* GetDocviewCommandProcessor() const
    void SetDocviewCommandProcessor( a2dDocumentCommandProcessor* docviewCommandProcessor )
    //virtual void RecordF( wxObject* sender, const wxChar* Format, ... )
    //virtual void RecordF( const wxChar* Format, ... )
};

// *****************************************************************
// wx/docview\docmdiref.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dDocumentMDIChildFrame
// ---------------------------------------------------------------------------

#include "wx/docview/docmdiref.h"
class a2dDocumentMDIChildFrame : public wxMDIChildFrame
{
    a2dDocumentMDIChildFrame()
    //a2dDocumentMDIChildFrame(
    //bool Create(
    void SetView( a2dView* view )
    a2dView *GetView() const
    void SetDestroyOnCloseView( bool destroyOnCloseView )
    virtual bool ProcessEvent(wxEvent& event)
    void OnActivate(wxActivateEvent& event)
    void OnCloseWindow(wxCloseEvent& event)
    bool Destroy()
    virtual void OnDraw(wxDC& dc)
    void AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
    void RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
    void AddCmdToToolbar( const a2dMenuIdItem& cmdId )
};

// ---------------------------------------------------------------------------
// a2dDocumentMDIParentFrame
// ---------------------------------------------------------------------------

#include "wx/docview/docmdiref.h"
class a2dDocumentMDIParentFrame : public wxMDIParentFrame
{
    a2dDocumentMDIParentFrame( wxFrame *parent, wxWindowID id,const wxString& title, const wxPoint& pos = wxDefaultPosition,const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE, const wxString& name = "frame" )
    a2dDocumentMDIParentFrame()
    // bool Create(
    virtual bool ProcessEvent(wxEvent& event)
    void OnExit(wxCommandEvent& event)
    void OnMRUFile(wxCommandEvent& event)
    void OnCloseWindow(wxCloseEvent& event)
    void AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
    void RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
    void AddCmdToToolbar( const a2dMenuIdItem& cmdId )
};

// *****************************************************************
// wx/docview\docviewdef.h
// *****************************************************************

enum a2dDocumentFlag
{
    a2dREFDOC_NON,
    a2dREFDOC_NEW,
    a2dREFDOC_SILENT,
    a2dREFDOC_ALL,
    a2dDEFAULT_DOCUMENT_FLAGS
};

enum a2dViewUpdateFlags
{
    a2dVIEW_UPDATE_ALL
};

// ---------------------------------------------------------------------------
// a2dDocumentTemplateAuto
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dDocumentTemplateAuto : public a2dDocumentTemplate
{
    a2dDocumentTemplateAuto( const wxString& descr, const wxString& filter, const wxString& dir, const wxString& ext, const wxString& docTypeName,                  wxClassInfo *docClassInfo = NULL, a2dViewConnector* connector = NULL, long flags = a2dTemplateFlag::DEFAULT )
    virtual a2dIOHandlerStrIn* GetDocumentIOHandlerForLoad( a2dDocumentInputStream& stream, a2dDocument* document )
    virtual a2dIOHandlerStrOut* GetDocumentIOHandlerForSave( const wxString& filename, a2dDocument* document )
};

// ---------------------------------------------------------------------------
// a2dDocumentViewWindow
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dDocumentViewWindow : public a2dViewWindow
{
    a2dDocumentViewWindow( a2dView* view,wxWindow* parent,wxWindowID id = -1,const wxPoint& pos = wxDefaultPosition,const wxSize& size = wxDefaultSize,long style = wxHSCROLL | wxVSCROLL,const wxString& name = "a2dDocumentViewWindow" )
    a2dDocumentViewWindow( wxWindow* parent,wxWindowID id = -1,const wxPoint& pos = wxDefaultPosition,const wxSize& size = wxDefaultSize,long style = wxHSCROLL | wxVSCROLL,const wxString& name = "a2dDocumentViewWindow" )
};

// ---------------------------------------------------------------------------
// a2dDocumentEvent
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dDocumentEvent : public wxEvent
{
    a2dDocumentEvent( wxEventType type, const wxFileName& filename, a2dDocumentFlagMask flags = a2dREFDOC_NON, int id = 0 )
    a2dDocumentEvent( wxEventType type, a2dDocumentTemplate* doctemplate, const wxFileName& filename , a2dDocumentFlagMask flags = a2dREFDOC_NON, int id = 0 )
    a2dDocumentEvent( wxEventType type, a2dViewList* reportlist )
    a2dDocumentEvent( wxEventType type, a2dView* view = NULL, bool enable = true, int id = 0 )
    a2dDocumentEvent(const a2dDocumentEvent & event)
    void Veto()
    void Allow()
    bool IsAllowed() const
    void SetFileName( const wxFileName& filename )
    wxFileName& GetFileName()
    virtual wxEvent *Clone() const
    a2dDocumentFlagMask GetFlags()
    a2dView* GetView()
    a2dDocumentTemplate* GetDocumentTemplate()
    void SetUpdateHint( unsigned int updateHint )
    unsigned int GetUpdateHint()
    void SetUpdateObject( wxObject* objectHint )
    wxObject* GetUpdateObject()
    a2dViewList* GetReportList()
    bool GetEnable()
};

// ---------------------------------------------------------------------------
// a2dCloseDocumentEvent
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dCloseDocumentEvent : public wxCloseEvent
{
    a2dCloseDocumentEvent(int id = 0)
    a2dCloseDocumentEvent(const a2dCloseDocumentEvent & event)
    virtual wxEvent *Clone() const
};

// ---------------------------------------------------------------------------
// a2dTemplateEvent
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dTemplateEvent : public wxEvent
{
    a2dTemplateEvent( a2dDocument* doc, wxEventType type, a2dDocumentFlagMask flags = a2dREFDOC_NON, int id = 0 )
    a2dTemplateEvent( wxEventType type, a2dView* view, a2dDocumentFlagMask flags = a2dREFDOC_NON, int id = 0)
    a2dTemplateEvent( const a2dTemplateEvent & event )
    a2dDocument* GetDocument()
    void SetDocument( a2dDocument* doc )
    a2dDocumentFlagMask GetFlags()
    a2dTemplateFlagMask GetTemplateMaskFlags()
    virtual wxEvent *Clone() const
    a2dView* GetView()
    void SetView( a2dView* view )
};

// ---------------------------------------------------------------------------
// a2dViewConnector
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dViewConnector : public a2dObject
{
    void SetInitialPosition( const wxPoint& position )
    void SetInitialSize( const wxSize& size )
    void SetInitialStyle( long style )
    wxPoint& GetInitialPosition()
    wxSize& GetInitialSize()
    long GetInitialStyle()
    a2dViewTemplateList& GetViewTemplates()
    void AssociateViewTemplate( a2dViewTemplate *temp )
    void DisassociateViewTemplate( a2dViewTemplate *temp )
    void DisassociateViewTemplates()
    virtual a2dView* CreateView( a2dDocument* doc, const wxString& viewTypeName = "",a2dDocumentFlagMask flags = a2dREFDOC_NON ,a2dTemplateFlagMask mask = a2dTemplateFlag::DEFAULT )
    void OnDisConnectView(  a2dTemplateEvent& event )
    void SetOnlyDisconnect( bool OnlyDisconnect )
    bool GetOnlyDisconnect()
};

// ---------------------------------------------------------------------------
// a2dDocument
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dDocument : public a2dObject
{
    a2dDocument( a2dDocument* parent = NULL )
    a2dDocument( const a2dDocument& other )
    void SetFilename( const wxFileName& filename, bool notifyViews = false)
    wxFileName GetFilename() const
    void SetTitle(const wxString& title, bool notifyViews = false)
    wxString GetTitle() const
    void SetDocumentTypeName(const wxString& name)
    wxString GetDocumentTypeName() const
    bool GetDocumentSaved() const
    bool IsDocumentSaved() const
    void SetDocumentSaved(bool saved = true)
    bool IsClosed()
    bool Close( bool force )
    bool Save()
    bool SaveAs( const wxFileName& fileTosaveTo, a2dDocumentFlagMask flags = a2dREFDOC_NON )
    bool Export( a2dDocumentTemplate* doctemplate, const wxFileName& fileTosaveTo , a2dDocumentFlagMask flags = a2dREFDOC_NON )
    bool Import( a2dDocumentTemplate* doctemplate, const wxFileName& fileToImport , a2dDocumentFlagMask flags = a2dREFDOC_NON )
    virtual bool Revert()
    virtual a2dDocumentInputStream& LoadObject( a2dDocumentInputStream& stream, a2dIOHandlerStrIn* handler )
    virtual a2dDocumentOutputStream& SaveObject(a2dDocumentOutputStream& stream, a2dIOHandlerStrOut* handler )
	void SetLastError( a2dError lastError ) 
	a2dError GetLastError() 
    void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser )
    void Save( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dObjectList* towrite )
    void Save( a2dIOHandlerXmlSerOut &out, wxObject* start = NULL )
    virtual int SaveModifiedDialog( bool canCancel = true )
    virtual void CreateCommandProcessor()
    a2dCommandProcessor *GetCommandProcessor() const
    void SetCommandProcessor(a2dCommandProcessor *proc)
    virtual void DeleteContents()
    virtual bool Draw(wxDC& context)
    bool IsModified() const
    void Modify(bool mod)
    void AskSaveWhenOwned( bool askSaveWhenOwned )
    virtual void UpdateAllViews(a2dView *sender = NULL, unsigned int hint = a2dVIEW_UPDATE_ALL, wxObject *objecthint = NULL )
    void ReportViews( a2dViewList* allviews )
    void EnableAllViews( bool enable, a2dView *sender = NULL )
    void DisConnectAllViews()
    a2dDocumentTemplate* GetDocumentTemplate() const
    void SetDocumentTemplate(a2dDocumentTemplate *temp)
    wxString GetPrintableName() const
    virtual wxWindow* GetAssociatedWindow() const
};

// ---------------------------------------------------------------------------
// a2dDocumentViewScrolledWindow
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dDocumentViewScrolledWindow : public a2dViewWindow
{
    a2dDocumentViewScrolledWindow( a2dView* view,wxWindow* parent,wxWindowID id = -1,const wxPoint& pos = wxDefaultPosition,const wxSize& size = wxDefaultSize,long style = wxHSCROLL | wxVSCROLL,const wxString& name = "a2dDocumentViewScrolledWindow")
    a2dDocumentViewScrolledWindow( wxWindow* parent,wxWindowID id = -1,const wxPoint& pos = wxDefaultPosition,const wxSize& size = wxDefaultSize,long style = wxHSCROLL | wxVSCROLL,const wxString& name = "a2dDocumentViewScrolledWindow")
};

// ---------------------------------------------------------------------------
// a2dViewEvent
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dViewEvent : public wxEvent
{
    a2dViewEvent( wxEventType type, a2dDocumentFlagMask flags, int id )
    a2dViewEvent( wxEventType type, bool value, int id )
    a2dViewEvent( const a2dViewEvent & event )
    void Veto()
    void Allow()
    bool IsAllowed() const
    virtual wxEvent *Clone() const
    a2dDocumentFlagMask GetFlags()
    bool GetActive()
    bool GetEnable()
};
   
// ---------------------------------------------------------------------------
// a2dDocumentFrame
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dDocumentFrame : public wxFrame
{
    a2dDocumentFrame()
    a2dDocumentFrame(
    // bool Create(
    void SetDestroyOnCloseView( bool destroyOnCloseView )
    void SetView( a2dView* view )
    a2dView* GetView()
    bool ProcessEvent(wxEvent& event)
    bool GetIsParentFrame()
    virtual void OnDraw(wxDC& dc)
    void AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
    void RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId )
    void AddCmdToToolbar( const a2dMenuIdItem& cmdId )
};




// ---------------------------------------------------------------------------
// a2dWindowConnectorFora2dDocumentViewWindow
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dWindowConnectorFora2dDocumentViewWindow : public a2dViewConnector
{ 
};

// ---------------------------------------------------------------------------
// a2dWindowConnectorFora2dDocumentViewScrolledWindow
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dWindowConnectorFora2dDocumentViewScrolledWindow : public a2dViewConnector 
{
    a2dDocumentViewScrolledWindow* GetDisplayWindow()
    void SetDisplayWindow( a2dDocumentViewScrolledWindow* window ) 
};

// ---------------------------------------------------------------------------
// a2dWindowViewConnector
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dWindowViewConnector : public a2dWindowConnectorFora2dDocumentViewWindow
{
    a2dDocumentViewWindow* GetDisplayWindow()
    void SetDisplayWindow( a2dDocumentViewWindow* window ) 
};

// ---------------------------------------------------------------------------
// a2dScrolledWindowViewConnector
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dScrolledWindowViewConnector : public a2dWindowConnector
{
};

// ---------------------------------------------------------------------------
// a2dDocumentPrintout
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dDocumentPrintout : public wxPrintout
{
    a2dDocumentPrintout(a2dView *view = NULL, const wxString& title = "Printout" )
    bool OnPrintPage(int page)
    bool HasPage(int page)
    bool OnBeginDocument(int startPage, int endPage)
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
    virtual a2dView *GetView()
};

// ---------------------------------------------------------------------------
// a2dViewTemplate
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dViewTemplate : public a2dObject
{
    a2dViewTemplate( const wxString& descr, const wxString& docTypeName, const wxString& viewTypeName, wxClassInfo *viewClassInfo = NULL, a2dViewConnector* connector = NULL, long flags = a2dTemplateFlag::DEFAULT, const wxSize& size = wxDefaultSize )
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
    virtual a2dObject* Clone( a2dObject::CloneOptions options ) const
    virtual a2dView *CreateView( a2dDocument *doc, a2dDocumentFlagMask flags = a2dREFDOC_NON )
    wxString GetDescription() const
    void SetDescription(const wxString& descr)
    wxString GetViewTypeName() const
    wxString GetDocumentTypeName() const
    const a2dTemplateFlag& GetFlags() const
    void SetFlags( a2dTemplateFlagMask flags)
    void SetFlags( a2dTemplateFlag flags)
    bool CheckMask( a2dTemplateFlagMask mask )
    void SetVisible( bool value )
    void SetMarked( bool value )
    bool IsVisible() const
    bool IsMarked() const
    a2dViewConnector* GetViewConnector()
    void SetInitialSize( const wxSize& size )
    wxSize& GetInitialSize()
    // wxPageSetupData* GetPageSetupData(void) const
    // void SetPageSetupData(const wxPageSetupData& pageSetupData)
};

// ---------------------------------------------------------------------------
// a2dFrameViewConnector
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dFrameViewConnector : public a2dViewConnector
{
    a2dFrameViewConnector()
    a2dFrameViewConnector( const a2dFrameViewConnector &other )
    void Init( wxFrame* docframe, bool createChildframe = true )
    wxFrame* GetFrame()
    void SetFrame( wxFrame* docframe )
    void SetCreateChild( bool createChildframe )
    bool GetCreateChild()
};

// ---------------------------------------------------------------------------
// a2dCloseViewEvent
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dCloseViewEvent : public wxEvent
{
    a2dCloseViewEvent(int id = 0)
    a2dCloseViewEvent(const a2dCloseViewEvent & event)
    virtual wxEvent *Clone() const
    void Veto(bool veto = true)
    void SetCanVeto(bool canVeto)
    bool CanVeto() const
    bool GetVeto() const
};

// ---------------------------------------------------------------------------
// a2dDocumentTemplate
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dDocumentTemplate : public a2dObject
{
    a2dDocumentTemplate( const wxString& descr, const wxString& filter, const wxString& dir, const wxString& ext, const wxString& docTypeName, wxClassInfo *docClassInfo = NULL, a2dViewConnector* connector = NULL, long flags = a2dTemplateFlag::DEFAULT, a2dIOHandlerStrIn* handlerIn = NULL, a2dIOHandlerStrOut* handlerOut = NULL )
    virtual a2dObject* Clone( a2dObject::CloneOptions options ) const
    virtual a2dDocument *CreateDocument( const wxFileName& path, a2dDocumentFlagMask flags = a2dREFDOC_NON )
    a2dDocument* SentPreAddCreatedDocumentEvent( a2dDocument* newDoc, a2dDocumentFlagMask flags )
    bool SentPostCreateDocumentEvent( a2dDocument* newDoc, a2dDocumentFlagMask flags )
    wxString GetDefaultExtension() const
    wxString GetDescription() const
    wxString GetDirectory() const
    wxString GetFileFilter() const
    const a2dTemplateFlag& GetFlags() const
    void SetFlags( a2dTemplateFlagMask flags)
    void SetFlags( a2dTemplateFlag flags)
    bool CheckMask( a2dTemplateFlagMask mask )
    wxString GetDocumentTypeName() const
    void SetFileFilter(const wxString& filter)
    void SetDirectory(const wxString& dir)
    void SetDescription(const wxString& descr)
    void SetDefaultExtension(const wxString& ext)
    void SetVisible( bool value )
    void SetMarked( bool value )
    bool IsVisible() const
    bool IsMarked() const
    virtual bool FileMatchesTemplate(const wxString& path)
    virtual a2dIOHandlerStrIn* GetDocumentIOHandlerStrIn()
    virtual a2dIOHandlerStrOut* GetDocumentIOHandlerStrOut()
    virtual a2dIOHandlerStrIn* GetDocumentIOHandlerForLoad( a2dDocumentInputStream& stream, a2dDocument* document )
    virtual a2dIOHandlerStrOut* GetDocumentIOHandlerForSave( const wxString& filename, a2dDocument* document )
    virtual void SetDocumentIOHandlerStrIn( a2dIOHandlerStrIn* handler )
    virtual void SetDocumentIOHandlerStrOut( a2dIOHandlerStrOut* handler )
    a2dViewConnector* GetViewConnector()
    // wxPageSetupData* GetPageSetupData(void) const
    // void SetPageSetupData(const wxPageSetupData& pageSetupData)
};

// ---------------------------------------------------------------------------
// a2dView
// ---------------------------------------------------------------------------

#include "wx/docview/docviewref.h"
class a2dView : public a2dObject
{
    a2dView()
    a2dView( const a2dView& other)
    bool IsClosed()
    void SetClosed()
    bool Enable(bool enable)
    bool GetEnabled()
    virtual void SetDisplayWindow( wxWindow* display )
    wxWindow* GetDisplayWindow()
    wxUint32 GetId()
    a2dDocument *GetDocument() const
    void SetDocument(a2dDocument *doc)
    wxString GetViewTypeName() const
    a2dViewTemplate* GetViewTemplate() const
    void SetViewTemplate( a2dViewTemplate* viewtemplate )
    void SetViewTypeName(const wxString& name)
    virtual void OnDraw(wxDC *dc) = 0
    virtual void OnPrint(wxDC *dc, wxObject *info)
    virtual void Update( unsigned int hint = 0, wxObject* hintObject = NULL )
    virtual bool Close( bool force = false)
    virtual bool ProcessEvent(wxEvent& event)
    void Activate(bool activate)
    bool GetActive()
    virtual wxPrintout *OnCreatePrintout( a2dPrintWhat typeOfPrint, const wxPageSetupDialogData& pageSetupData )
};
