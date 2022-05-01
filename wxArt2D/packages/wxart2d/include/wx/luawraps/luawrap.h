/*! \file wx/canextobj/canext.h
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: luawrap.h,v 1.49 2009/09/26 19:01:07 titato Exp $
*/

#ifndef __WXLUAWRAPS_H__
#define __WXLUAWRAPS_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canobj.h"
#include "wx/canvas/canprim.h"
#include "wx/editor/candoc.h"
#include "wx/canvas/sttool.h"
#include "wx/editor/candocproc.h"
#include "wx/canvas/tooldlg.h"
#include "wx/canvas/styledialg.h"
#include "wx/canvas/layerdlg.h"
#include "wx/editor/canedit.h"

#include "wxbind/include/wxcore_bind.h"

#include "wxlua/wxlstate.h"
#include "wxlua/wxlbind.h"


#include "wx/luawraps/wxledit.h"

#define a2dLUAWRAP_VERSION  "1.0"

#ifdef A2DLUAWRAPMAKINGDLL
#define A2DLUAWRAPDLLEXP WXEXPORT
#define A2DLUAWRAPDLLEXP_DATA(type) WXEXPORT type
#define A2DLUAWRAPDLLEXP_CTORFN
#elif defined(WXART2D_USINGDLL)
#define A2DLUAWRAPDLLEXP WXIMPORT
#define A2DLUAWRAPDLLEXP_DATA(type) WXIMPORT type
#define A2DLUAWRAPDLLEXP_CTORFN
#else // not making nor using DLL
#define A2DLUAWRAPDLLEXP
#define A2DLUAWRAPDLLEXP_DATA(type) type
#define A2DLUAWRAPDLLEXP_CTORFN
#endif


//! Lua script used to draw the object
/*!
    \ingroup canvasobject
*/
class A2DLUAWRAPDLLEXP a2dCanvasObjectLua: public a2dCanvasObject
{
public:

    //!construct at given position
    /*!
    \param x X position
    \param y Y position
    */
    a2dCanvasObjectLua( double x = 0 , double y = 0, const wxString& script = wxT( "" ), const wxString& function = wxT( "" ) );

    a2dCanvasObjectLua( const a2dCanvasObjectLua& other, CloneOptions options, a2dRefMap* refs = NULL );

    //!destructor
    virtual ~a2dCanvasObjectLua();

    DECLARE_DYNAMIC_CLASS( a2dCanvasObjectLua )

    DECLARE_EVENT_TABLE()

protected:

    void SetField( wxLuaState lst, const wxString& field,  const wxString& value );
    void SetField( wxLuaState lst, const wxString& field,  long value );
    void SetField( wxLuaState lst, const wxString& field,  double value );
    void SetField( wxLuaState lst, const wxString& field,  bool value );
    void SetField( wxLuaState lst, const wxString& name, wxColour value );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    wxString m_function;

    wxString m_script;

private:

    bool m_run;

    double m_xoffset;
    double m_yoffset;

    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCanvasObjectLua( const a2dCanvasObjectLua& other );

};

class A2DLUAWRAPDLLEXP a2dLuaConsole;
class A2DLUAWRAPDLLEXP a2dLuaExecDlg;
class a2dLuaCentralCommandProcessor;

//----------------------------------------------------------------------------
// a2dLuaConsole
//----------------------------------------------------------------------------

class A2DLUAWRAPDLLEXP a2dLuaConsole : public wxLuaIDE
{
public:
    a2dLuaConsole( a2dLuaCentralCommandProcessor* cmdh, wxWindow* parent, int id,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   long options = WXLUAIDE_DEFAULT,
                   const wxString& name = wxT( "a2dLuaConsole" ) );

    void OnLua( wxLuaEvent& event );

    ~a2dLuaConsole();

private:

    a2dLuaCentralCommandProcessor* m_cmdh;

    DECLARE_EVENT_TABLE();
    DECLARE_ABSTRACT_CLASS( a2dLuaConsole )
};

class A2DLUAWRAPDLLEXP a2dLuaExecDlg: public wxDialog
{
public:

    // Constructor.
    a2dLuaExecDlg( a2dLuaCentralCommandProcessor* commandProcessor, wxFrame* parent, const wxString& title = wxT( "commandline" ),
                   long style = ( wxDEFAULT_DIALOG_STYLE ), const wxString& name = _T( "frame" ) );

    // Destructor.
    ~a2dLuaExecDlg();

    // Close window if EXIT-button is pressed.
    void OnCloseWindow( wxCloseEvent& event );

protected:

    void OnRecord( a2dCommandEvent& event );

    void OnUndoEvent( a2dCommandProcessorEvent& event );

    void OnDoEvent( a2dCommandProcessorEvent& event );

    void OnCharHook( wxKeyEvent& event );

    a2dLuaCentralCommandProcessor* m_commandProcessor;

    // Pointer to mainwindow.
    wxFrame* m_parent;

    a2dLuaConsole* m_luaConsole;

    // Declare used events.
    DECLARE_EVENT_TABLE()

};

class A2DLUAWRAPDLLEXP a2dLuaEditorFrame;


class A2DEDITORDLLEXP a2dLuaCentralCommandProcessor : public a2dCentralCanvasCommandProcessor
{

    DECLARE_CLASS( a2dLuaCentralCommandProcessor )

public:

    static const a2dCommandId COMID_ShowLuaExecDlg;

public:

    //! \see a2dDocumentCommandProcessor
    a2dLuaCentralCommandProcessor( long flags = a2dDEFAULT_DOCUMENT_FLAGS,
                                   bool initialize = true,
                                   int maxCommands = -1 );
    //! initilize with lower level command processor
    a2dLuaCentralCommandProcessor( a2dDocumentCommandProcessor* other );


    //! destructor
    ~a2dLuaCentralCommandProcessor();

    //! if active view has as display frame a a2dLuaEditorFrame, return it.
    a2dLuaEditorFrame* GetActiveEditorFrame();

    //! cleaup modeless dialogs created from here
    virtual void OnExit();

    bool Execute( const wxString& commandsString, bool withUndo = TRUE );

    virtual bool ExecuteF( bool withUndo, wxChar* Format, ... );

    bool ExecuteFile( const wxString& fileName );

    bool ShowDlg( const a2dCommandId* comID, bool modal, bool onTop );

    bool FileNew();

    bool FileOpen( const wxString& path );

    a2dError FileOpen( a2dDocumentPtr& doc, wxFileName& path );

    bool FileSaveAs( const wxString& path, bool silent = true );

    bool FileImport( const wxString& path, const wxString& description );

    bool FileExport( const wxString& path, const wxString& description, bool silent = true );

    a2dBaseTool* PopTool();
    bool PushTool( const a2dCommandId& which, bool shiftadd = false, bool oneshot = false );

    bool SetDocumentLayers( const wxString& propertyname, const wxString& value );

    a2dLayers* GetLayersDocument();
    a2dLayers* LoadLayers( const wxString& filename );
    a2dLayers* GetLayersGlobal();

    bool SetLayersDocument( a2dLayers* layers );
    bool SetLayersGlobal( a2dLayers* layers );

    bool SaveLayersDocument( const wxString& filename );
    bool SaveLayersGlobal( const wxString& filename );

    bool SetLayer( wxUint16 layer );

    bool SetTarget( wxUint16 target );

    bool ClearGroup( const wxString& group );

    bool UnGroup( bool selected, bool deep );

    bool DeleteGroupA();

    bool MoveGroupA();

    bool CopyGroupA();
    bool ConvertToArcsGroupA( bool detectCircle = false );
    bool ConvertPolygonToArcsGroupA( bool detectCircle = false );
    bool ConvertToPolygonPolylinesWithArcsGroupA();
    bool ConvertToPolygonPolylinesWithoutArcsGroupA();
    bool ConvertPolylineToArcsGroupA();
    bool ConvertToVPathsGroupA();
    bool ConvertLinesArcsGroupA();
    bool ConvertToPolylinesGroupA();
    bool DetectSmallGroupA();
    bool RemoveRedundantGroupA();

    void AddGroupA( wxUint16 layer );
    void AddGroupB( wxUint16 layer );
    void SetSelectedOnlyA( bool selectedOnlyA );
    bool GetSelectedOnlyA();
    void SetSelectedOnlyB( bool selectedOnlyB );
    bool GetSelectedOnlyB();


#if wxART2D_USE_KBOOL
    bool BoolOperation_GroupAB( a2dCommand_GroupAB::a2dDoWhat operation, bool clearTarget, bool selectedA, bool selectedB );
#endif //wxART2D_USE_KBOOL

    bool Message( const wxString& message );

    bool SetVariable( const wxString& varname, const wxString& varvalue );

    wxString GetVariable( const wxString& varname );

    void AddConfigPath( const wxString& path );

    void AddLayersPath( const wxString& path );

    void AddFontPath( const wxString& path );

    void AddImagePath( const wxString& path );

    void AddIconPath( const wxString& path );

    bool SetSnap( bool snap );

    bool SetSnapFeatures( wxUint32 features );

    void SetCursor( double x, double y );

    bool Find( const wxString& objectname );

    bool Ask( const wxString&  variablename, const wxString& mes );

    bool AskFile( const wxString& storeInVariable,
                  const wxString& message =  _( "Give Name of file" ),
                  const wxString& defaultDir = _T( "./" ),
                  const wxString& extension = _T( "*" ),
                  const wxString& fileFilter = _T( "*" )
                );

    wxFileName AskFile2(
        const wxString& message =  _( "Give Name of file" ),
        const wxString& defaultDir = _T( "./" ),
        const wxString& extension = _T( "*" ),
        const wxString& fileFilter = _T( "*" )
    );

    bool ShowVariable( const wxString& variableName, const wxString& message );

    bool ViewAsImageAdv( const wxFileName& file, wxBitmapType type, bool onView = false );

    bool DrawWireFrame( bool onOff = true );
    bool DrawGridLines( bool onOff = true );
    bool DrawGridAtFront( bool onOff = true );
    bool DrawGrid( bool onOff = true );
    bool ZoomOut();
    bool Refresh();
    bool InsertGroupRef( double x, double y );
    bool AddGroupObject(  const wxString& groupName, double x = 0, double y = 0, wxUint16 layer = 0 );
    bool NewPin( double x, double y, const wxString& groupname );
    bool PushInto( const wxString& name );
    bool SelectAll();
    bool DeSelectAll();
    bool SelectedChangeLayer( long layer );
    bool SetFillStrokeSelected();
    bool ToTop();
    bool ToBack();
    bool DeleteSelected();
    bool MoveSelected( double x, double y, long layer );
    bool CopySelected( double x, double y, long layer );
    bool TransformSelected( const wxString& str );
    bool Group();
    bool UnGroup();
    bool SetUrl();

    //! add specific object
    a2dCanvasObject* Add_a2dCanvasObject( double x = 0, double y = 0 );

    //! add specific object
    a2dCanvasObjectReference* Add_a2dCanvasObjectReference( double x, double y, a2dCanvasObject* obj );

    //a2dNameReference* Add_a2dNameReference( double x, double y, a2dCanvasObject* obj, const wxString &text, double size, double angle );

    //! add specific object
    a2dOrigin* Add_a2dOrigin( double w, double h );

    //! add specific object
    a2dRectC* Add_a2dRectC( double xc, double yc, double w, double h, double angle = 0 , double radius = 0 );

    //! add specific object
    a2dArrow* Add_a2dArrow( double xt, double yt, double l1, double l2, double b, bool spline = false );

    //! add specific object
    a2dRect* Add_a2dRect( double x, double y, double w, double h , double radius = 0 );

    //! add specific object
    a2dCircle* Add_a2dCircle( double x, double y, double radius );

    //! add specific object
    a2dEllipse* Add_a2dEllipse( double xc, double yc, double width, double height );

    //! add specific object
    a2dEllipticArc* Add_a2dEllipticArc( double xc, double yc, double width, double height, double start, double end );

    //! add specific object
    a2dArc* Add_a2dArc( double xc, double yc, double radius, double start, double end );

    //! add specific object
    a2dSLine* Add_a2dSLine( double x1, double y1, double x2, double y2 );

    //! add specific object
    a2dEndsLine* Add_a2dEndsLine( double x1, double y1, double x2, double y2 );

    //! add specific object
    a2dImage* Add_a2dImage( const wxImage& image, double xc, double yc, double w, double h );

    //! add specific object
    a2dImage* Add_a2dImage( const wxString& imagefile, wxBitmapType type, double xc, double yc, double w, double h );

    //! add specific object
    a2dText* Add_a2dText( const wxString& text, double x, double y, double angle, const a2dFont& font );

    //! add specific object
    a2dPolygonL* Add_a2dPolygonL( a2dVertexList* points, bool spline = false );

    //! add specific object
    a2dPolylineL* Add_a2dPolylineL( a2dVertexList* points, bool spline = false );

    bool Add_Point( double x, double y );

    bool Move_Point( int index , double x, double y );

    wxLuaState& GetLuaState() { return m_interp; }

    //! if m_luaExecDlg is visible or not
    bool IsShowna2dLuaExecDlg();

protected:

    a2dPropertyIdList m_listOfIdforAddPrims;

    wxLuaState m_interp;

    bool ShowLuaExecDlg();

    a2dLuaExecDlg* m_luaExecDlg;
};

extern a2dLuaCentralCommandProcessor* a2dLuaWP;
extern a2dLuaCentralCommandProcessor* a2dGetLuaWP();

extern const long SCRIPT_lua_script;
extern const long EXECDLG_lua;

//! sepcialized Frame for editor of a2dCanvas
class A2DLUAWRAPDLLEXP a2dLuaEditorFrame : public a2dEditorFrame
{
    DECLARE_EVENT_TABLE()

public:

    DECLARE_DYNAMIC_CLASS( a2dLuaEditorFrame )

    //! this makes dynamic creation possible ( e.g. a derived a2dLuaEditorFrame )
    /*!
        Can be used by connectors ( e.g. a2dEditorMultiFrameViewConnector ) to create
        a derived a2dLuaEditorFrame class using the ClassInfo.
        This makes one connector class enough for all derived a2dLuaEditorFrame's.
        This constructor is used in combination with Create() to initilize the a2dLuaEditorFrame.
    */
    a2dLuaEditorFrame();

    a2dLuaEditorFrame( bool isParent, wxFrame* parent,
                       const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE );

    ~a2dLuaEditorFrame();

protected:

    bool CallLuaScriptThemeFunction( const wxString& fileName, const wxString& function );

    void OnUpdateUI( wxUpdateUIEvent& event );

    void OnShowExecDlg2( wxCommandEvent& event );

    void OnRunScript( wxCommandEvent& event );

    void OnTheme( a2dEditorFrameEvent& themeEvent );

    void OnInit( a2dEditorFrameEvent& initEvent );
};


#endif

