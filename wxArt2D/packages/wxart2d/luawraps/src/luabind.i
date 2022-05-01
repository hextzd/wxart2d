
#include "a2dprec.h"

#include "wx/docview/doccom.h"
#include "wx/artbase/drawer2d.h"
#include "wx/artbase/artglob.h"
#include "wx/canvas/layerinf.h"
#include "wx/canvas/booloper.h"
#include "wx/luawraps/luawrap.h"

#define_string  a2dLUAWRAP_VERSION
   
%function a2dCentralCanvasCommandProcessor* a2dGetCmdh()

// ---------------------------------------------------------------------------
// a2dCanvasObjectLua
// ---------------------------------------------------------------------------

#include "wx/luawraps/luawrap.h"
class a2dCanvasObjectLua : public a2dCanvasObject
{
    a2dCanvasObjectLua( double x = 0 , double y = 0, const wxString& script = "", const wxString& function = "" )
    a2dCanvasObjectLua( const a2dCanvasObjectLua &other, a2dObject::CloneOptions options )
    virtual a2dObject* Clone( a2dObject::CloneOptions options ) const
};

class a2dLuaEditorFrame : public a2dEditorFrame
{
};
   
class %delete a2dLuaCentralCommandProcessor : public a2dCentralCanvasCommandProcessor
{    
    %member static const a2dCommandId COMID_ShowLuaExecDlg;
 
    #define_pointer a2dLuaWP
    
    a2dLuaEditorFrame* GetActiveEditorFrame()
    
    a2dLuaCentralCommandProcessor(long flags = a2dDEFAULT_DOCUMENT_FLAGS, bool initialize = true, int maxCommands = -1 )
    
    bool Execute( const wxString& commandsString, bool withUndo = TRUE )

    bool ExecuteFile( const wxString& fileName )
    
    bool ShowDlg( const a2dCommandId* comID, bool modal = false, bool onTop = true )
        
    bool FileNew()
        
    bool FileOpen( const wxString& path )
    
    bool FileSaveAs( const wxString& path )
    bool FileImport( const wxString& path, const wxString& description )
    bool FileExport( const wxString& path, const wxString& description, bool silent = true ) 
    
    bool Update() 
    bool Message(const wxString& message) 
    
    bool PushTool( const a2dCommandId& which )

    bool SetDocumentLayers( const wxString& propertyname, const wxString& value )
 
    a2dLayers* GetLayersDocument() 
    a2dLayers* LoadLayers( const wxString& filename ) 
    a2dLayers* GetLayersGlobal() 

    bool SetLayersDocument( a2dLayers* layers ) 
    bool SetLayersGlobal( a2dLayers* layers ) 

    bool SaveLayersDocument( const wxString& filename ) 
    bool SaveLayersGlobal( const wxString& filename ) 
 
    bool SetLayer( wxUint16 layer )

    bool SetTarget( wxUint16 target )  
   
    bool ClearGroup( const wxString& group )    

    void AddGroupA( wxUint16 layer )

    void AddGroupB( wxUint16 layer )
    
    bool DeleteGroupA()

    bool MoveGroupA()

    bool CopyGroupA()
    
    bool UnGroup(  bool selected, bool deep )    
    bool SetShowObject( long id )
    bool DrawWireFrame( bool onOff = true )
    bool DrawGridLines( bool onOff = true )
    bool DrawGridAtFront( bool onOff = true )
    bool DrawGrid( bool onOff = true )
    bool Zoom( double x1, double y1, double x2, double y2, bool upp = false )
    bool ZoomOut()
    //bool Select( double x1, double y1, double x2, double y2 )
    //bool DeSelect( double x1, double y1, double x2, double y2 )
    bool Refresh()
    bool InsertGroupRef( double x, double y )
    bool AddGroupObject(  const wxString& groupName, double x = 0, double y = 0, wxUint16 layer = 0 )
    bool NewPin( double x, double y, const wxString& groupname )
    bool PushInto( const wxString& name )
    bool SelectAll()
    bool DeSelectAll()
    bool SelectedChangeLayer( long layer )
    bool SetFillStrokeSelected()
    bool ToTop()
    bool ToBack()
    bool DeleteSelected()
    bool MoveSelected( double x, double y, long layer )
    bool CopySelected( double x, double y, long layer )
    bool TransformSelected( const wxString& str )
    bool Group()
    bool UnGroup()
    bool SetUrl()
            
    bool ConvertToArcsGroupA( bool detectCircle = false )
	bool ConvertPolygonToArcsGroupA( bool detectCircle = false )
	bool ConvertToPolygonPolylinesWithArcsGroupA()
	bool ConvertToPolygonPolylinesWithoutArcsGroupA()
	bool ConvertPolylineToArcsGroupA()
	bool ConvertToVPathsGroupA()
	bool ConvertLinesArcsGroupA()
	bool ConvertToPolylinesGroupA()
	bool DetectSmallGroupA()
    bool RemoveRedundantGroupA()
                
#if wxART2D_USE_KBOOL
    bool BoolOperation_GroupAB( a2dCommand_GroupAB::a2dDoWhat operation, bool clearTarget, bool selectedA, bool selectedB )
#endif //wxART2D_USE_KBOOL
        
    bool SetVariable( const wxString& varname, const wxString& varvalue )   
    wxString GetVariable( const wxString& varname )
        
    void AddConfigPath( const wxString& path )
    void AddLayersPath( const wxString& path )
    void AddFontPath( const wxString& path )
    void AddImagePath( const wxString& path )
    void AddIconPath( const wxString& path )
        
    bool SetSnap( bool snap )
    bool SetSnapFeatures( wxUint32 features )
        
    bool ViewAsImageAdv( const wxFileName& file, wxBitmapType type, bool onView = false )
    
    void SetCursor( double x, double y )    
        
    bool Find(  const wxString& objectname )

    bool Ask( const wxString&  variablename, const wxString& mes )

    bool AskFile( const wxString& storeInVariable, const wxString& message =  "Give Name of file", const wxString& defaultDir = "./", const wxString& extension = "*", const wxString& fileFilter = "*" )

    wxFileName AskFile2( const wxString& message =  "Give Name of file", const wxString& defaultDir = "./", const wxString& extension = "*", const wxString& fileFilter = "*" )
    
    a2dError CreateDocuments( const wxString& path, long flags = a2dREFDOC_NON, a2dDocumentTemplate* wantedDocTemplate = NULL, int dialogflags = wxFD_OPEN )
                        
    bool CloseDocuments( bool force )

    bool Clear( bool force )

    bool FileRevert() 

    bool ShowVariable( const wxString& variableName, const wxString& message )
    
    //! add specific object
    a2dCanvasObject* Add_a2dCanvasObject( double x = 0, double y = 0)

    //! add specific object
    a2dCanvasObjectReference* Add_a2dCanvasObjectReference( double x, double y, a2dCanvasObject* obj )

    //a2dNameReference* Add_a2dNameReference( double x, double y, a2dCanvasObject* obj, const wxString &text, double size, double angle )

    //! add specific object
    a2dOrigin* Add_a2dOrigin( double w, double h )

    //! add specific object
    a2dRectC* Add_a2dRectC( double xc, double yc, double w, double h, double angle =0 , double radius=0 )

    //! add specific object
    a2dArrow* Add_a2dArrow( double xt, double yt, double l1, double l2, double b, bool spline = false )

    //! add specific object
    a2dRect* Add_a2dRect( double x, double y, double w, double h , double radius=0 )

    //! add specific object
    a2dCircle* Add_a2dCircle( double x, double y, double radius )

    //! add specific object
    a2dEllipse* Add_a2dEllipse( double xc, double yc, double width, double height )

    //! add specific object
    a2dEllipticArc* Add_a2dEllipticArc( double xc, double yc, double width, double height, double start, double end )

    //! add specific object
    a2dArc* Add_a2dArc( double xc, double yc, double radius, double start, double end )

    //! add specific object
    a2dSLine* Add_a2dSLine( double x1, double y1, double x2, double y2 )

    //! add specific object
    a2dEndsLine* Add_a2dEndsLine( double x1, double y1, double x2, double y2 )

    //! add specific object
    a2dImage* Add_a2dImage( const wxImage &image, double xc, double yc, double w, double h )

    //! add specific object
    a2dImage* Add_a2dImage( const wxString &imagefile, wxBitmapType type, double xc, double yc, double w, double h )

    //! add specific object
    // a2dText* Add_a2dText( const wxString &text, double x, double y, double angle, a2dFont* font )

    //! add specific object
    //a2dPolygonL* Add_a2dPolygonL(a2dVertexList* points, bool spline = false )

    //! add specific object
    //a2dPolylineL* Add_a2dPolylineL(a2dVertexList* points, bool spline = false )

    //! add a point to the current a2dCanvasObject
    bool Add_Point( double x, double y )

    //! add a point to the current a2dCanvasObject
    bool Move_Point( int index ,double x, double y )

    //bool SetParentObject( a2dCanvasObject* obj = 0 )
        
    bool Exit()    
};

%function a2dLuaCentralCommandProcessor* a2dGetLuaWP()    

%function a2dDrawer2D* a2dGetDrawer2D()



