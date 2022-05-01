

typedef wxUint64 a2dCanvasObjectFlagsMask
typedef unsigned int a2dCursorId 
typedef wxUint32 a2dSnapToWhatMask
typedef unsigned int a2dCanvasObject::a2dBboxFlags

class a2dBaseToolPtr
{
};

class %delete a2dCanvasObjectPtr
{
    a2dCanvasObject* Get() const
};

class a2dDrawing : public a2dObject
{
    void SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which, const wxString& classname = "", a2dCanvasObjectFlagsMask whichobjects = a2dCanvasOFlags::ALL )
    void SetInitialSize( double sizeX, double sizeY )
    double GetInitialSizeX()
    double GetInitialSizeY()
    void SetUpdatesPending(bool onoff)
    void DependencyPending()
    bool GetUpdatesPending()
    void SetIgnorePendingObjects( bool onoff )
    bool GetIgnorePendingObjects()

    //void Prepend( a2dCanvasObject* obj )
    //void Append( a2dCanvasObject* obj )
    //void Insert( size_t before, a2dCanvasObject* obj )
    //a2dCanvasObject* Find( a2dCanvasObject* obj )
    //a2dCanvasObject* Find( const wxString& objectname,  const wxString& classname = "", wxInt64 id = -1 )

    void SetAvailable();
    void SetRootRecursive();
    void SetLayerSetup( a2dLayers* layersetup );
    a2dLayers* GetLayerSetup() 
    a2dCanvasObject* GetRootObject() const 
    void SetRootObject( a2dCanvasObject* newRoot );
    void CreateCommandProcessor();
    a2dCommandProcessor* GetCommandProcessor() const
    a2dCanvasCommandProcessor* GetCanvasCommandProcessor();
    void SetCommandProcessor( a2dCommandProcessor* proc ) 
    void SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which, const wxString& classname = "", a2dCanvasObjectFlagsMask whichobjects = a2dCanvasOFlags::ALL );
    void UpdateAllViews( unsigned int hint, wxObject* objecthint = NULL );
    bool AddPendingUpdatesOldNew();
    bool IsModified() const;
    void Modify( bool mod );
    double GetUnitsAccuracy();
    void   SetUnitsAccuracy( double accuracy )
    double GetUnitsScale()
    void   SetUnitsScale( double scale )
    double GetNormalizeScale()
    void SetNormalizeScale( double scale )
    wxString GetUnits()
    void SetUnits( const wxString& unitString ) 
    //a2dDrawingId GetDrawingId() 
    //void SetDrawingId( a2dDrawingId id )   
    bool LoadFromFile( const wxString& filename, a2dIOHandlerStrIn* handler );
    bool SaveToFile( const wxString& filename,  a2dIOHandlerStrOut* handler ) const;  
    bool LinkReference( a2dObject* other );
    
};

class a2dLayerGroup : public a2dlist
{
    a2dLayerGroup()
    void clear()
    bool empty()
    size_t size()
	wxUint16 front()
	wxUint16 back()
	void push_front( wxUint16 )
	void pop_front()
	void push_back( wxUint16 )
	void pop_back()
};

class a2dPropertyIdList
{
};

class a2dPropertyIdCanvasObject
{
};

class a2dPropertyIdCanvasShadowStyle
{
};

class a2dPropertyIdBoundingBox
{
};

class a2dPropertyIdCanvasClipPath
{
};

class a2dLayerMapNr
{
};

class a2dLayerIndex
{
};

class a2dLayerMapName
{
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectList
// ---------------------------------------------------------------------------

class a2dCanvasObjectListIter
{
    a2dCanvasObjectListIter()
    
        %rename GetItem a2dCanvasObjectPtr operator*() const;
        
		//%operator a2dCanvasObjectPtr operator->() const;

        a2dCanvasObjectListIter& operator=(const a2dCanvasObjectListIter& other);
        
        bool operator ==( const a2dCanvasObjectListIter& iter ) const;
        bool operator !=( const a2dCanvasObjectListIter& iter ) const;
        
		a2dCanvasObjectListIter& operator++();
		a2dCanvasObjectListIter& operator++( int );
       
		a2dCanvasObjectListIter& operator--();
		a2dCanvasObjectListIter& operator--(int);
   
};


#include "wx/canvas/objlist.h"
class a2dCanvasObjectList : public a2dSmrtPtrList
{
    a2dCanvasObjectList()

    %rename lbegin a2dCanvasObjectListIter begin()
    %rename lend   a2dCanvasObjectListIter end()

    bool empty()
    size_t size()
	a2dCanvasObject* front()
	a2dCanvasObject* back()
	void push_front( a2dCanvasObject* obj )
	void pop_front()
	void push_back( a2dCanvasObject* obj )
	void pop_back()
    
    a2dCanvasObjectList& operator=( const a2dCanvasObjectList& other )
    a2dCanvasObjectList* Clone( a2dObject::CloneOptions options ) const
    // a2dCanvasObjectList* Clone( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, a2dObject::CloneOptions options = a2dObject::clone_deep, a2dlist< long >* objectsIndex = NULL, const a2dBoundingBox& bbox = wxNonValidBbox ) const
    // a2dCanvasObjectList* CloneChecked( a2dObject::CloneOptions options = a2dObject::clone_deep, a2dlist< long >* objectsIndex = NULL ) const
    void SortXY()
    void SortXRevY()
    void SortYX()
    void SortYRevX()
    void MakeUnique()
    bool SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which, const wxString& classname = "",a2dCanvasObjectFlagsMask whichobjects = a2dCanvasOFlags::ALL, const a2dBoundingBox& bbox = wxNonValidBbox,const a2dAffineMatrix& tworld = a2dIDENTITY_MATRIX )
    bool ChangeLayer( wxUint16 layer, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    int BringToTop( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, bool check = false )
    int BringToBack( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, bool check = false )
    int Release( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL,  const wxString& classname = "", const a2dPropertyId *id = NULL, const wxString& name = "", bool now = true )
    int Release( a2dCanvasObject* object, bool backwards = false, bool all = true, bool now = true, const a2dPropertyId *id = NULL )
    int Copy( double x, double y, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, long target = -1, bool check = false )
    int Move( double x, double y, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, long target = -1, bool check = false )
    a2dCanvasObject* Find( a2dCanvasObject* obj ) const
    a2dCanvasObject* Find( const wxString& objectname, const wxString& classname = "",a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL , const a2dPropertyId *propid = NULL, const wxString& valueAsString = "",wxUint32 id = 0 ) const
    bool SwitchObjectNamed( const wxString& objectname, a2dCanvasObject* newobject )
    void Transform( const a2dAffineMatrix& tworld , const wxString& type="",a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL ,const a2dPropertyId *id = NULL )
    void SetTransform( const a2dAffineMatrix& tworld , const wxString& type="",a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL ,const a2dPropertyId *id = NULL )
    int TakeOverTo( a2dCanvasObjectList* total, const wxString& type="",a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL ,const a2dPropertyId *id = NULL )
    int TakeOverFrom( a2dCanvasObjectList* total, const wxString& type="",a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL ,const a2dPropertyId *id = NULL )
    int CollectObjects( a2dCanvasObjectList* total, const wxString& type="",a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL ,const a2dPropertyId *id = NULL,const a2dBoundingBox& bbox = wxNonValidBbox
    bool SetDrawerStyle( const a2dFill& brush, const a2dStroke& stroke, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    // void RestoreConnectionsAfterCloning( class a2dCanvasCommandProcessor *cp = 0 )
    void AssertUnique()
    void Insert( size_t before, a2dCanvasObject* obj, bool ignoreReleased )
};

#if wxART2D_USE_KBOOL
 // *****************************************************************
// wx/canvas\booloper.h
// *****************************************************************

class kbBool_Engine
{
};

// ---------------------------------------------------------------------------
// a2dBooleanWalkerHandler
// ---------------------------------------------------------------------------

#include "wx/canvas/booloper.h"
class a2dBooleanWalkerHandler : public a2dWalker_LayerCanvasObjects
{
    %member static const long Boolean_NON
    %member static const long Boolean_OR
    %member static const long Boolean_AND
    %member static const long Boolean_EXOR
    %member static const long Boolean_A_SUB_B
    %member static const long Boolean_B_SUB_A
    %member static const long Boolean_CORRECTION
    %member static const long Boolean_SMOOTHEN
    %member static const long Boolean_MAKERING
    %member static const long Boolean_Polygon2Surface
    %member static const long Boolean_Surface2Polygon

    a2dBooleanWalkerHandler( a2dCanvasObjectFlagsMask mask =a2dCanvasOFlags::ALL )
    kbBool_Engine* GetBooleanEngine()
    void Initialize()
    void SetOperation( long operation )   
    void SetClearTarget( bool clearTarget )
    void SetMarge( double marge )
    void SetGrid( long grid )
    void SetDGrid( double dgrid )
    void SetCorrectionAber( double aber )
    void SetCorrectionFactor( double aber )
    void SetMaxlinemerge( double maxline )
    void SetWindingRule( bool rule )
    void SetRoundfactor( double roundfac )
    void SetSmoothAber( double aber )
    void SetLinkHoles( bool doLinkHoles )
    void SetOrientationEntryMode( bool orientationEntryMode )
    void SetAberArcToPoly( double aber )
    void SetReleaseOrignals( bool releaseOrignals )
    void SetResultStroke( const a2dStroke& resultStroke )
    void SetResultFill( const a2dFill& resultFill )
    void SetStyleOfFirstChild( bool useStyle )
    virtual bool Start( a2dCanvasObject* object )
    bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
};

#endif //wxART2D_USE_KBOOL

// ---------------------------------------------------------------------------
// a2dCanvasCommandProcessor
// ---------------------------------------------------------------------------

#include "wx/canvas/drawing.h"
class a2dCanvasCommandProcessor : public a2dCommandProcessor
{
    a2dCanvasCommandProcessor( a2dDrawing* drawing, int maxCommands = -1)
    virtual bool Submit(a2dCommand* command, bool storeIt = true)
    virtual bool SubmitMultiple( a2dCommand* command, a2dCanvasObjectList *objects, bool storeIt = true )
    
    a2dCanvasObject* Add_a2dCanvasObject( double x = 0, double y = 0)
    a2dCanvasObjectReference* Add_a2dCanvasObjectReference( double x, double y, a2dCanvasObject* obj )
    a2dOrigin* Add_a2dOrigin( double w, double h )
    a2dHandle* Add_a2dHandle( double xc, double yc, int w, int h, double angle = 0 , int radius = 0 )
    a2dRectC* Add_a2dRectC( double xc, double yc, double w, double h, double angle =0 , double radius=0 )
    a2dArrow* Add_a2dArrow( double xt, double yt, double l1, double l2, double b, bool spline = false )
    a2dRect* Add_a2dRect( double x, double y, double w, double h , double radius=0 )
    a2dCircle* Add_a2dCircle( double x, double y, double radius )
    a2dEllipse* Add_a2dEllipse( double xc, double yc, double width, double height )
    a2dEllipticArc* Add_a2dEllipticArc( double xc, double yc, double width, double height, double start, double end )
    a2dArc* Add_a2dArc( double xc, double yc, double radius, double start, double end )
    a2dSLine* Add_a2dSLine( double x1, double y1, double x2, double y2 )
    a2dEndsLine* Add_a2dEndsLine( double x1, double y1, double x2, double y2 )
    a2dImage* Add_a2dImage( const wxImage &image, double xc, double yc, double w, double h )
    a2dImage* Add_a2dImage( const wxString &imagefile, wxBitmapType type, double xc, double yc, double w, double h )
    a2dText* Add_a2dText( const wxString &text, double x, double y, double angle, const a2dFont& font )
    a2dPolygonL* Add_a2dPolygonL(a2dVertexList* points, bool spline = false )
    a2dPolylineL* Add_a2dPolylineL(a2dVertexList* points, bool spline = false )
    bool Add_Point( double x, double y )
    bool Move_Point( int index ,double x, double y )
    a2dBaseTool* PopTool()
    
    bool SetParentObject( a2dCanvasObject* obj = 0 )
    a2dCanvasObject* GetParentObject()
   
    a2dCanvasObject* GetCurrentCanvasObject()
    void SetCurrentCanvasObject( a2dCanvasObject* currentcanvasobject, bool takeOverProperties = true )
    bool Select( double x1,double y1, double x2, double y2 )
    bool DeSelect( double x1,double y1, double x2, double y2 )
   
    a2dDrawingPart* GetActiveDrawingPart()
   
    bool IsShownToolDlg()
    bool IsShowna2dStyleDialog()
    bool IsShownLayerPropertiesDialog()
    bool IsShownLayerOrderDialog()
    bool IsShownIdentifyDialog()
    bool IsShowna2dSnapSettings()
    bool IsShowna2dCoordinateEntry()
    bool IsShowna2dMeasureDlg()
    bool IsShowna2dPathSettings()
    bool IsShowna2dSettings()
    bool IsShowna2dTransDlg()
    //bool IsShownPropEditDlg()
   
    %member static const a2dCommandId COMID_PopTool
    %member static const a2dCommandId COMID_InsertGroupRef
    %member static const a2dCommandId COMID_ShowDlgLayers
    %member static const a2dCommandId COMID_ShowDlgLayersDocument
    %member static const a2dCommandId COMID_ShowDlgLayersGlobal
    %member static const a2dCommandId COMID_ShowDlgLayerOrderDocument
    %member static const a2dCommandId COMID_ShowDlgLayerOrderGlobal
    %member static const a2dCommandId COMID_ShowDlgTools
    %member static const a2dCommandId COMID_ShowDlgStyle
    %member static const a2dCommandId COMID_ShowDlgSnap
    %member static const a2dCommandId COMID_ShowDlgGroups
    %member static const a2dCommandId COMID_ShowDlgIdentify
    %member static const a2dCommandId COMID_ShowDlgCoordEntry
    %member static const a2dCommandId COMID_ShowDlgMeasure
    %member static const a2dCommandId COMID_ShowDlgStructure
    %member static const a2dCommandId COMID_ShowDlgPathSettings
    %member static const a2dCommandId COMID_ShowDlgSettings
    %member static const a2dCommandId COMID_ShowDlgTrans
    %member static const a2dCommandId COMID_ShowDlgPropEdit
   
};

// *****************************************************************
// wx/canvas\candefs.h
// *****************************************************************

#define wxMAXLAYER 2000

enum wxLayerNames
{
    wxLAYER_DEFAULT,
    wxLAYER_ALL
};

enum a2dDocumentRenderStyle
{
    RenderLAYERED,
    RenderWIREFRAME,
    RenderWIREFRAME_ZERO_WIDTH,
    RenderWIREFRAME_INVERT,
    RenderWIREFRAME_INVERT_ZERO_WIDTH,
    RenderFIX_STYLE,
    RenderRectangles,
    RenderWIREFRAME_SELECT,
    RenderWIREFRAME_SELECT_INVERT,
    RenderTOOL_OBJECTS,
    RenderTOOL_OBJECTS_STYLED,
    RenderTOOL_DECORATIONS,
    RenderRectangleTOOL_OBJECTS,
    RenderDefault
};

// *****************************************************************
// wx/canvas\canglob.h
// *****************************************************************

enum a2dNextSeg
{
	WIREEND_STRAIGHT,
	WIREEND_MANHATTAN_X,
	WIREEND_MANHATTAN_Y
};

// ---------------------------------------------------------------------------
// a2dHabitat
// ---------------------------------------------------------------------------


class a2dHabitat: public a2dObject
{

    a2dHabitat();
    virtual ~a2dHabitat();


    void SetDefaultUnitsScale( double scale );
    double GetDefaultUnitsScale();

    double ACCUR();

    a2dLayers* GetLayerSetup();

    void SetLayerSetup( a2dLayers* layersetup );

    void SetSymbolLayer( wxUint16 layerId );
    wxUint16 GetSymbolLayer();

    bool LoadLayers( const wxString& filename );

    void SetPinSize( double size );

    double GetPinSize();

    void SetHandleSize( double size );

    double GetHandleSize();

    void SetPin( a2dPin* newpin );

    a2dPin* GetPin();

    void SetPinCanConnect( a2dPin* newpin );

    a2dPin* GetPinCanConnect();

    void SetPinCannotConnect( a2dPin* newpin );

    a2dPin* GetPinCannotConnect();

    void SetHandle( a2dHandle* newhandle );

    a2dHandle* GetHandle();

    a2dText* GetPropertyText();

    a2dStroke& GetHighLightStroke();

    a2dFill& GetHighLightFill();

    void SetHighLightStroke( const a2dStroke& stroke );

    void SetHighLightFill( const a2dFill& fill );

    a2dStroke& GetSelectStroke();

    a2dFill& GetSelectFill();

    a2dStroke& GetSelect2Stroke();

    a2dFill& GetSelect2Fill();

    void SetSelectStroke( const a2dStroke& stroke );

    void SetSelectFill( const a2dFill& fill );

    void SetSelect2Stroke( const a2dStroke& stroke );

    void SetSelect2Fill( const a2dFill& fill );

    void SetSelectDrawStyle( a2dDocumentRenderStyle drawstyle );

    a2dDocumentRenderStyle GetSelectDrawStyle() const;

    a2dStroke& GetDefaultStroke();

    a2dFill& GetDefaultFill();

    void SetDefaultStroke( const a2dStroke& stroke );

    void SetDefaultFill( const a2dFill& fill );

    void SetRestrictionEngine( a2dRestrictionEngine* restrict );

    a2dRestrictionEngine* GetRestrictionEngine();

    void SetConnectionGenerator( a2dConnectionGenerator* connectionGenerator );

    a2dConnectionGenerator* GetConnectionGenerator() const;

    wxUint16 GetHitMarginDevice() const;

    void SetHitMarginDevice( wxUint16 val );

    double GetHitMarginWorld() const;

    void SetHitMarginWorld( double val );

    double GetCopyMinDistance() const;

    void SetCopyMinDistance( double val );

    double GetCoordinateEpsilon() const;

    void SetCoordinateEpsilon( double val );

    void SetReverseOrder( bool reverse_order );

    bool GetReverseOrder() const ;

    void SetRouteOneLine( bool oneLine ) ;

    bool GetRouteOneLine() const ;

    void SetGroupA( const a2dLayerGroup& groupA );

    void SetGroupB( const a2dLayerGroup& groupB );

    a2dLayerGroup& GetGroupA();

    a2dLayerGroup& GetGroupB();

    void AddGroupA( wxUint16 layer );

    void AddGroupB( wxUint16 layer );

    void SetTarget( wxUint16 target );

    wxUint16 GetTarget();

    void SetSelectedOnlyA( bool selectedOnlyA );

    bool GetSelectedOnlyA() const;

    void SetSelectedOnlyB( bool selectedOnlyB );

    bool GetSelectedOnlyB() const;

    void SetBooleanEngineMarge( const a2dDoMu& marge );
    void SetBooleanEngineGrid( long grid );
    void SetBooleanEngineDGrid( double dgrid );
    void SetBooleanEngineCorrectionAber( const a2dDoMu& aber ) ;
    void SetBooleanEngineCorrectionFactor( const a2dDoMu& aber ) ;
    void SetBooleanEngineMaxlinemerge( const a2dDoMu& maxline ) ;
    void SetBooleanEngineWindingRule( bool rule ) ;
    void SetBooleanEngineRoundfactor( double roundfac );
    void SetBooleanEngineSmoothAber( const a2dDoMu& aber );
    void SetBooleanEngineLinkHoles( bool doLinkHoles ) ;
    void SetRadiusMin( const a2dDoMu& radiusMin ) ;
    void SetRadiusMax( const a2dDoMu& radiusMax );
    void SetAberPolyToArc( const a2dDoMu& aber );
    void SetAberArcToPoly( const a2dDoMu& aber );
    void SetSmall( const a2dDoMu& smallest );
    void SetDistancePoints( const a2dDoMu& distancePoints );
    void SetAtVertexPoints( bool atVertexPoints );

    bool GdsIoSaveTextAsPolygon( bool onOff );
    bool KeyIoSaveTextAsPolygon( bool onOff );

    bool GetGdsIoSaveTextAsPolygon() const;
    bool GetKeyIoSaveTextAsPolygon() const;

    const a2dDoMu&  GetRadiusMin() const;
    const a2dDoMu& GetRadiusMax() const ;
    const a2dDoMu& GetAberPolyToArc() const ;
    const a2dDoMu& GetAberArcToPoly() const ;
    const a2dDoMu& GetSmall() const ;
    const a2dDoMu& GetDistancePoints();
    bool GetAtVertexPoints();

    void SetGridX( const a2dDoMu& gridX );
    void SetGridY( const a2dDoMu& gridY );

    const a2dDoMu& GetBooleanEngineMarge() const;
    long GetBooleanEngineGrid() const;
    double GetBooleanEngineDGrid() const ;
    const a2dDoMu& GetBooleanEngineCorrectionAber() const;
    const a2dDoMu& GetBooleanEngineCorrectionFactor() const ;
    const a2dDoMu& GetBooleanEngineMaxlinemerge() const ;
    bool GetBooleanEngineWindingRule() const  ;
    double GetBooleanEngineRoundfactor() const ;
    const a2dDoMu& GetBooleanEngineSmoothAber() const  ;
    bool GetBooleanEngineLinkHoles() const ;

    void SetClearTarget( bool clearTarget );
    bool GetClearTarget() const ;

    void SetLastXyEntry( double x, double y ) ;
    double GetLastXEntry() const ;
    double GetLastYEntry() const ;

    void SetRelativeStart( double x, double y );

    double GetRelativeStartX() const;
    double GetRelativeStartY() const;
    const a2dStroke& GetStroke() const;
    const a2dFill& GetFill() const;
    void SetStroke( const a2dStroke& stroke );
    void SetStroke( const unsigned char red, const unsigned char green, const unsigned char blue );
    void SetFill( const a2dFill& fill );
    void SetFill( const unsigned char red, const unsigned char green, const unsigned char blue );
    bool GetSpline() const;
    void SetSpline( bool spline );
    a2dCanvasObject* GetLineBegin() const;
    a2dCanvasObject* GetLineEnd() const ;
    void SetLineBegin( a2dCanvasObject* begin );
    void SetLineEnd( a2dCanvasObject* end );
    void SetEndScaleX( double xs );
    void SetEndScaleY( double ys );
    double GetEndScaleX() const ;
    double GetEndScaleY() const;
    void SetContourWidth( const a2dDoMu& currentContourWidth );

    const a2dDoMu& GetContourWidth() const ;
    void SetPathType( a2dPATH_END_TYPE pathtype );
    a2dPATH_END_TYPE GetPathType();

    void SetFont( const a2dFont& font );
    const a2dFont& GetFont() const ;
    void SetTextOwnStyle( bool ownStyle ) ;
    bool GetTextOwnStyle() const;
    void SetTextStroke( const a2dStroke& stroke );
    a2dStroke GetTextStroke() const;
    void SetTextFill( const a2dFill& fill );
    a2dFill GetTextFill() const;
    void SetTextFont( const a2dFont& font );
    a2dFont GetTextFont() const;
    a2dText* GetTextTemplateObject();
    void SetTextTemplateObject( a2dText* textTemplateobject ) ;
    void SetLayer( wxUint16 layer, bool setStyleOfLayer = false );
    inline wxUint16 GetLayer() const;
    void SetObjectGridSize( double objectGridSize ) ;
    double GetObjectGridSize();

    void SetDrawingTemplate( a2dDrawing* drawingTemplate);

    a2dDrawing* GetDrawingTemplate() const;

    void SetSymbolDrawingTemplate( a2dDrawing* drawingTemplate);

    a2dDrawing* GetSymbolDrawingTemplate() const;

    void SetBuildInDrawingTemplate( a2dDrawing* drawingTemplate);

    a2dDrawing* GetBuildInDrawingTemplate() const;

    void SetDiagramDrawingTemplate( a2dDrawing* drawingTemplate);

    a2dDrawing* GetDiagramDrawingTemplate() const;

    void SetGuiDrawingTemplate( a2dDrawing* drawingTemplate);

    a2dDrawing* GetGuiDrawingTemplate() const;

	a2dNextSeg GetEndSegmentMode();

	void SetEndSegmentMode( a2dNextSeg mode );
};

// ---------------------------------------------------------------------------
// a2dCanvasGlobal
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dCanvasGlobal : public a2dGeneralGlobal
{
    a2dCanvasGlobal()
    %member wxPoint m_initialDocChildFramePos
    %member wxSize m_initialDocChildFrameSize
    %member long m_initialDocChildFrameStyle
    %member long m_initialWorldCanvasStyle
    %member bool WINDINGRULE

    a2dPathList& GetLayersPathList() const
    a2dArtProvider* GetArtProvider()
    void SetArtProvider( a2dArtProvider* artProvider )
    wxCursor& GetCursor( a2dCursorId id )
    %member static const a2dPropertyIdPathList PROPID_layerPathList
    
    void SetPopBeforePush( bool popBeforePush )
    bool GetPopBeforePush()
    void SetOneShotTools( bool oneShotTools )
    bool GetOneShotTools()
    void SetEditAtEndTools( bool editAtEndTools )
    bool GetEditAtEndTools()
};

// ---------------------------------------------------------------------------
// a2dWalker_RemoveProperty
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_RemoveProperty : public a2dWalkerIOHandler
{
    a2dWalker_RemoveProperty( bool all = true )
    a2dWalker_RemoveProperty( a2dPropertyId* id , bool all = true )
    a2dWalker_RemoveProperty( const a2dPropertyIdList& idList , bool all = true )
    void AddPropertyId( a2dPropertyId* id )
    bool Start( a2dObject* object )
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
    %member bool m_all
    %member a2dPropertyIdList m_propertyList
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectEvent
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dCanvasObjectEvent : public wxEvent
{
    a2dCanvasObjectEvent( a2dIterC* ic, wxEventType type, a2dCanvasObject* object, double x, double y, int id = 0 )
    a2dCanvasObjectEvent( a2dCanvasObject* object, const a2dBoundingBox& box, int id = 0 )
    a2dCanvasObjectEvent( const a2dCanvasObjectEvent& other )
    virtual wxEvent *Clone() const
    a2dCanvasObject* GetCanvasObject()
    double GetX()
    double GetY()
    a2dBoundingBox& GetBbox()
    a2dIterC* GetIterC()
    void SetIterC( a2dIterC* ic)
};

// ---------------------------------------------------------------------------
// a2dWalker_SetAvailable
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_SetAvailable : public a2dWalkerIOHandler
{
    a2dWalker_SetAvailable( a2dLayers* layerSetup )
    a2dWalker_SetAvailable( a2dDrawingPart* drawingPart )
    bool Start( a2dObject* object )
    bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
};

// ---------------------------------------------------------------------------
// a2dWalker_SetRoot
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_SetRoot : public a2dWalkerIOHandler
{
    a2dWalker_SetRoot( a2dDrawing* root )
    bool Start( a2dCanvasObject* object )
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
};

// ---------------------------------------------------------------------------
// a2dDumpWalker
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dDumpWalker : public a2dWalkerIOHandler
{
    a2dDumpWalker()
    void Initialize()
    bool Start( a2dObject* object )
    bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
};

// ---------------------------------------------------------------------------
// a2dWalker_FilterCanvasObjects
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_FilterCanvasObjects : public a2dWalkerIOHandler
{
    a2dWalker_FilterCanvasObjects( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_FilterCanvasObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_FilterCanvasObjects( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    void AddPropertyId( const a2dPropertyId* id )
    void Initialize()
    bool Start( a2dCanvasObject* object )
	void AddClassName( wxString className )
	void RemoveClassName( wxString className )
	void SetAllowClassList( bool allow ) 
	bool GetAllowClassList() 
    void SetObjectName( const wxString& objectname = "" )
    void SetUniqueSerializationId( wxInt64 id )
    void SetBoundingBox( const a2dBoundingBox& bbox )
    void SetRecursionDepth( int depth = INT_MAX )
    void SetCheckLayerVisible( bool check )
    void SetCheckLayerSelectable( bool check )
    void SetSkipStart( bool skip )
    %member a2dPropertyIdList m_propertyList
    %member a2dCanvasObjectFlagsMask m_mask
    %member wxString m_objectname
    %member wxInt64 m_id
    %member int m_depth
    %member a2dBoundingBox m_bbox
    %member bool m_layervisible
    %member bool m_layerselectable
    %member bool m_skipStartObject
};


// ---------------------------------------------------------------------------
// a2dWalker_DetectSmall
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_DetectSmall : public a2dWalker_LayerGroup
{
    a2dWalker_DetectSmall( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_DetectSmall( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_DetectSmall( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    void Initialize()
	void SetSmall( double smallest )
    void SetAsString( bool asString ) 
    bool GetAsString()
	bool Start( a2dCanvasObject* object )
    const wxString GetString()
};

// ---------------------------------------------------------------------------
// a2dWalker_DetectSmallSegments
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_DetectSmallSegments : public a2dWalker_LayerGroup
{
    a2dWalker_DetectSmallSegments( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_DetectSmallSegments( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_DetectSmallSegments( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    void Initialize()
	void SetSmall( double smallest )
    void SetAsString( bool asString ) 
    bool GetAsString()
	bool Start( a2dCanvasObject* object )
    const wxString GetString()
};


// ---------------------------------------------------------------------------
// a2dWalker_AllowedObjects
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_AllowedObjects : public a2dWalker_LayerGroup
{
    a2dWalker_AllowedObjects( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_AllowedObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_AllowedObjects( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    void Initialize()
    void SetAsString( bool asString ) 
    bool GetAsString()
	bool Start( a2dCanvasObject* object )
    const wxString GetString()
};

// ---------------------------------------------------------------------------
// a2dWalker_FindCanvasObject
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_FindCanvasObject : public a2dWalkerIOHandler
{
    a2dWalker_FindCanvasObject( a2dCanvasObject* toSearch )
    bool Start( a2dCanvasObject* object )
    void Initialize()
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
    %member a2dCanvasObjectPtr m_search
};

// ---------------------------------------------------------------------------
// a2dWalker_FindAndSetCorridorPath
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_FindAndSetCorridorPath : public a2dWalker_FindCanvasObject
{
    a2dWalker_FindAndSetCorridorPath( a2dCanvasObject* findObject )
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectMouseEvent
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dCanvasObjectMouseEvent : public a2dCanvasObjectEvent
{
    a2dCanvasObjectMouseEvent( a2dIterC* ic, a2dCanvasObject* object, a2dHit& how, double x, double y, const wxMouseEvent& event )
    a2dCanvasObjectMouseEvent( a2dIterC* ic, a2dCanvasObject* object, wxEventType type, double x, double y, const wxMouseEvent& event )
    a2dCanvasObjectMouseEvent( const a2dCanvasObjectMouseEvent& other )
    virtual wxEvent *Clone( bool deep = true ) const
    %member a2dHit m_how
};

// ---------------------------------------------------------------------------
// a2dTimer
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dTimer : public wxTimer
{
    a2dTimer()
    a2dTimer( wxEvtHandler *owner, int id = -1 )
    void SetOwner(wxEvtHandler *owner, int id = -1)
    virtual void Notify()
};

// ---------------------------------------------------------------------------
// a2dWalker_MakeTree
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_MakeTree : public a2dWalker_FilterCanvasObjects
{
    a2dWalker_MakeTree( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_MakeTree( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_MakeTree( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    bool Start( a2dCanvasObject* object )
    void Initialize()
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
};

// ---------------------------------------------------------------------------
// a2dWalker_SetViewDependent
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_SetViewDependent : public a2dWalkerIOHandler
{
    a2dWalker_SetViewDependent( a2dDrawingPart* aDrawingPart, bool viewdependent, bool viewspecific = false, bool onlyinternalarea = false  )
    bool Start( a2dCanvasObject* object )
    void Initialize()
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
};

// ---------------------------------------------------------------------------
// a2dWalker_RemoveHierarchy
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_RemoveHierarchy : public a2dWalker_FilterCanvasObjects
{
    a2dWalker_RemoveHierarchy( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_RemoveHierarchy( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_RemoveHierarchy( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    bool Start( a2dCanvasObject* object )
    void Initialize()
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
};

// ---------------------------------------------------------------------------
// a2dWalker_SetSpecificFlagsCanvasObjects
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_SetSpecificFlagsCanvasObjects : public a2dWalker_FilterCanvasObjects
{
    a2dWalker_SetSpecificFlagsCanvasObjects( a2dCanvasObjectFlagsMask which, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_SetSpecificFlagsCanvasObjects( a2dCanvasObjectFlagsMask which, const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_SetSpecificFlagsCanvasObjects( a2dCanvasObjectFlagsMask which, const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    void Initialize()
    void Start( a2dCanvasObject* object, bool setTo )
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
    %member a2dCanvasObjectFlagsMask m_which
    %member bool m_setOrClear
};

// ---------------------------------------------------------------------------
// a2dArtProvider
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dArtProvider : public wxObject
{
    a2dArtProvider()
    virtual wxCursor& GetCursor( a2dCursorId id )
};

// ---------------------------------------------------------------------------
// a2dCanvasModule
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"

// ---------------------------------------------------------------------------
// a2dHandleMouseEvent
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dHandleMouseEvent : public a2dCanvasObjectEvent
{
    a2dHandleMouseEvent( a2dIterC* ic, a2dHandle* handle, double x, double y, const wxMouseEvent& event )
    a2dHandleMouseEvent( const a2dHandleMouseEvent& other )
    virtual wxEvent *Clone( bool deep = true ) const
};

// ---------------------------------------------------------------------------
// a2dWalker_LayerGroup
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_LayerGroup : public a2dWalker_FilterCanvasObjects
{
    a2dWalker_LayerGroup( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_LayerGroup( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_LayerGroup( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    void Initialize()
    bool Start( a2dCanvasObject* object )
    void SetGroupA( const a2dLayerGroup& groupA )
    void SetGroupB( const a2dLayerGroup& groupB )
    void SetTarget( wxUint16 targetlayer)
    void SetSelectedOnlyA( bool selectedOnlyA )
    bool GetSelectedOnlyA()
    void SetSelectedOnlyB( bool selectedOnlyB )
    bool GetSelectedOnlyB()
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
};


// ---------------------------------------------------------------------------
// a2dWalker_LayerCanvasObjects
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_LayerCanvasObjects : public a2dWalker_LayerGroup
{    
    %member static const long moveLayers
    %member static const long copyLayers
    %member static const long deleteLayers
    %member static const long ConvertToArcs
    %member static const long ConvertToPolygonPolylinesWithArcs
    %member static const long ConvertToPolygonPolylinesWithoutArcs
    %member static const long ConvertPolygonToArcs
    %member static const long ConvertPolylineToArcs
    %member static const long ConvertToVPaths
    %member static const long ConvertLinesArcs
    %member static const long ConvertToPolylines
    %member static const long RemoveRedundant
    %member static const long areaLayers
    %member static const long ConvertPointsAtDistance;
    
    a2dWalker_LayerCanvasObjects( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_LayerCanvasObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_LayerCanvasObjects( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    void Initialize()
    void SetOperation( long operation )
    void SetRadiusMin( double radiusMin )
    void SetRadiusMax( double radiusMax ) 
    void SetAberPolyToArc( double aber ) 
    void SetAberArcToPoly( double aber ) 
    void SetDetectCircle( bool detectCircle )
    void SetDistancePoints( double distancePoints )
    void SetAtVertexPoints( bool atVertexPoints )
    void SetLastPointCloseToFirst( bool lastPointCloseToFirst )
	
    bool GetDetectCircle()
    double GetCalculatedArea()
    
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
};


class a2dMoveLayer
{
    a2dMoveLayer();
}; 

// typedef a2dWalker_ForEachCanvasObject<a2dMoveLayer> a2dWalker_MoveLayer;
typedef a2dWalker_ForEachCanvasObject<a2dCopyLayer> a2dWalker_CopyLayer;
typedef a2dWalker_ForEachCanvasObject<a2dCopyLayer> a2dWalker_DeleteLayer;
typedef a2dWalker_ForEachCanvasObject<a2dConvertToArcs> a2dWalker_ConvertToArcs;
typedef a2dWalker_ForEachCanvasObject<a2dConvertPolygonToArcs> a2dWalker_ConvertPolygonToArcs;
typedef a2dWalker_ForEachCanvasObject<a2dConvertPolylineToArcs> a2dWalker_ConvertPolylineToArcs;
typedef a2dWalker_ForEachCanvasObject<a2dConvertToPolygonPolylinesWithArcs> a2dWalker_ConvertToPolygonPolylinesWithArcs;
typedef a2dWalker_ForEachCanvasObject<a2dConvertToPolygonPolylinesWithoutArcs> a2dWalker_ConvertToPolygonPolylinesWithoutArcs;
typedef a2dWalker_ForEachCanvasObject<a2dConvertToVPaths> a2dWalker_ConvertToVPaths;
typedef a2dWalker_ForEachCanvasObject<a2dConvertToPolylines> a2dWalker_ConvertToPolylines;
typedef a2dWalker_ForEachCanvasObject<a2dConvertLinesArcs> a2dWalker_ConvertLinesArcs;
typedef a2dWalker_ForEachCanvasObject<a2dRemoveRedundant> a2dWalker_RemoveRedundant;
typedef a2dWalker_ForEachCanvasObject<a2dConvertPointsAtDistance> a2dWalker_ConvertPointsAtDistance;
typedef a2dWalker_ForEachCanvasObject<a2dAreaLayer> a2dWalker_AreaLayer;

class a2dWalker_MoveLayer: public a2dWalker_LayerGroup
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_MoveLayer( a2dMoveLayer func, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_MoveLayer( a2dMoveLayer func, const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_MoveLayer( a2dMoveLayer func, const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

protected:

    a2dMoveLayer m_function;
};


// ---------------------------------------------------------------------------
// a2dWalker_RemovePropertyCandoc
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_RemovePropertyCandoc : public a2dWalkerIOHandler
{
    a2dWalker_RemovePropertyCandoc( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, bool all = true )
    a2dWalker_RemovePropertyCandoc( a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, bool all = true )
    a2dWalker_RemovePropertyCandoc( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, bool all = true )
    void AddPropertyId( a2dPropertyId* id )
    bool Start( a2dCanvasObject* object )
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
    void SetMakePending( bool makePending )
    %member bool m_all
    %member a2dPropertyIdList m_propertyList
    %member a2dCanvasObjectFlagsMask m_mask
    %member int m_depth
    %member bool m_makePending
};

// ---------------------------------------------------------------------------
// a2dWalker_CollectCanvasObjects
// ---------------------------------------------------------------------------

#include "wx/canvas/canglob.h"
class a2dWalker_CollectCanvasObjects : public a2dWalker_FilterCanvasObjects
{
    a2dWalker_CollectCanvasObjects( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_CollectCanvasObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    a2dWalker_CollectCanvasObjects( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    void Initialize()
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
    %member a2dCanvasObjectList m_found
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectFilter
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dCanvasObjectFilter : public a2dObject
{
    // a2dCanvasObjectFilter()
    // virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
    // virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
    // virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
    // virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
};

// *****************************************************************
// wx/canvas\canimage.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dRgbaImage
// ---------------------------------------------------------------------------

#include "wx/canvas/canimage.h"
class a2dRgbaImage : public RectMM
{
    a2dRgbaImage()
    a2dRgbaImage( double x, double y, wxImage& image, unsigned char alpha )
    a2dRgbaImage( const a2dRgbaImage& image, a2dObject::CloneOptions options, a2dRefMap* refs )
    void    SetWidth( double w)
    void    SetHeight( double h)
    a2dImageRGBA& GetImage()
    virtual a2dObject* Clone( a2dObject::CloneOptions options ) const
};

// *****************************************************************
// wx/canvas\canmod.h
// *****************************************************************

// *****************************************************************
// wx/canvas\canobj.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dConnectionGenerator
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dConnectionGenerator : public a2dObject
{
    a2dConnectionGenerator()
    virtual a2dCanvasObject* CreateConnectObject( a2dCanvasObject* parent, a2dPin* pinThis, a2dPin* pinOther, bool undo = false ) const
    virtual a2dCanvasObject* GetConnectTemplate( const a2dCanvasObject* object, a2dPinClass* mapObject, const a2dCanvasObject* other, a2dPinClass* mapOther ) const
    virtual bool GeneratePossibleConnections( a2dCanvasObject* object, a2dPinClass* pinClass, a2dConnectTask task, double x, double y, double margin ) const
    virtual a2dPinClass* GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj ) const
    a2dPinClass *GetAnyPinClass() const
    void SetAnyPinClass( a2dPinClass* pinClass )
    bool GetLastConnectCreationDirection() const
	void SetGeneratePins( bool onOff )
	bool GetGeneratePins() const
};

enum a2dHitOption
{
    a2dCANOBJHITOPTION_NONE,
    a2dCANOBJHITOPTION_LAYERS,
    a2dCANOBJHITOPTION_NOROOT,
    a2dCANOBJHITOPTION_ALL
};

enum a2dConnectTask
{
    a2d_FeedBackWireStartPin, //!< Return pinclass and pin 
    a2d_StartWire,      //!< find wire/connect pinclass, given start pinclass of start pin
    a2d_StartWire_BasedOnClassStartPin, 
    a2d_StartWire_BasedOnWireClassRequired, 
    a2d_StartWire_BasedOnObjectClassRequired, 

    a2d_GeneratePinsForStartWire, //!< generate pins, given Connect/wire pinclass
    a2d_GeneratePinsForFinishWire, //!< generate pins, given Connect/wire pinclass

    a2d_SearchPinForFinishWire, //!< find normal object pinclass, given Connect/wire pinclass
    a2d_FinishWire,     //!< find normal object pinclass, given Connect/wire pinclass

    a2d_PinToPin, //! connect pins 

    a2d_GeneratePinsForPinClass, //! generate pins on object given a a2dPinClass
};

enum a2dCanvasObject::a2dBboxFlag
{
    a2dCANOBJ_BBOX_NON,
    a2dCANOBJ_BBOX_EDIT,
    a2dCANOBJ_BBOX_CHILDREN 
};

enum a2dCanvasObject::UpdateMode
{
    update_save,
    update_includebboxprop,
    updatemask_save_includebboxprop,
    updatemask_force,
    updatemask_normal
};

enum a2dCanvasObjectHitFlags
{
    a2dCANOBJ_EVENT_NON,
    a2dCANOBJ_EVENT_FILL,
    a2dCANOBJ_EVENT_STROKE,
    a2dCANOBJ_EVENT_FILL_NON_TRANSPARENT,
    a2dCANOBJ_EVENT_STROKE_NON_TRANSPARENT,
    a2dCANOBJ_EVENT_VISIBLE,
    a2dCANOBJ_EVENT_ALL
};

enum wxEditStyle
{
    wxEDITSTYLE_NONE,
    wxEDITSTYLE_COPY,
    wxEDITSTYLE_NOHANDLES
};

// ---------------------------------------------------------------------------
// a2dCanvasOHitFlags
// ---------------------------------------------------------------------------

struct a2dCanvasOHitFlags
{
    %member bool m_non
    %member bool m_fill
    %member bool m_stroke
    %member bool m_fill_non_transparent
    %member bool m_stroke_non_transparent
    %member bool m_visible
    %member bool m_all
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectFilterToolObjects
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dCanvasObjectFilterToolObjects : public CanvasObjectFilterOnlyNoMaskBlind
{
    a2dCanvasObjectFilterToolObjects( const a2dPropertyId *id, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
};

// ---------------------------------------------------------------------------
// a2dCanvasObject
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dCanvasObject : public a2dObject
{
    a2dCanvasObject( double x = 0 , double y = 0)
    a2dCanvasObject( const a2dCanvasObject &other, a2dObject::CloneOptions options, a2dRefMap* refs )
    a2dCanvasObject* TClone( a2dObject::CloneOptions options, a2dRefMap* refs )
    
    bool AddPropertyId( a2dPropertyId *dynproperty )
    bool HasPropertyId( const a2dPropertyId* id ) const
    a2dPropertyId* HasPropertyId( const wxString& name )
    
    virtual bool IsRecursive()
    virtual void MakeReferencesUnique()
    void Rotate(double rotation)
    void SetRotation(double rotation)
    void Scale( double scalex, double scaley )
    void Mirror(bool x=true, bool y=false)
    void SkewX( double angle )
    void SkewY( double angle )
    const a2dAffineMatrix& GetTransformMatrix() const
    a2dAffineMatrix GetTransform() const
    void SetTransform( a2dAffineMatrix mat = a2dIDENTITY_MATRIX )
    void SetTransformMatrix(const a2dAffineMatrix& mat = a2dIDENTITY_MATRIX )
    void SetTransformMatrix( double xt, double yt, double scalex = 1, double scaley = 1, double degrees = 0 )
    double GetPosX() const
    double GetPosY() const
    a2dPoint2D GetPosXY() const
    void SetPosXY( double x, double y, bool restrict = false )
    void SetPosXyPoint( const a2dPoint2D &pos )
    bool SetPosXYRestrict( double& x, double& y )
    void Translate( double x, double y )
    void Transform( const a2dAffineMatrix& tworld )
    bool RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld  )
    virtual a2dCanvasObjectList* GetSnapVpath( a2dSnapToWhatMask snapToWhat )
    virtual bool EliminateMatrix()
    double  GetBboxMinX()
    double  GetBboxMinY()
    double  GetBboxMaxX()
    double  GetBboxMaxY()
    double  GetBboxWidth()
    double  GetBboxHeight()
    a2dBoundingBox& GetBbox()
    virtual a2dBoundingBox GetUnTransformedBbox( a2dCanvasObject::a2dBboxFlags flags = a2dCanvasObject::a2dCANOBJ_BBOX_CHILDREN ) const
    a2dBoundingBox GetCalculatedBoundingBox( int nChildLevels )
    a2dBoundingBox GetMappedBbox( a2dIterC& ic, bool withExtend  = true )
    a2dBoundingBox GetMappedBbox( const a2dAffineMatrix& cworld )
    wxRect GetAbsoluteArea( a2dIterC& ic )
    float GetWorldExtend() const
    int GetPixelExtend() const
    OVERLAP GetClipStatus( a2dIterC& ic, OVERLAP clipparent )
    a2dBoundingBox GetClipBox( a2dIterC& ic )
    a2dCanvasObject* IsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
    void SetHitFlags( a2dCanvasObjectHitFlags mask )
    virtual bool ProcessCanvasObjectEvent( a2dIterC& ic, a2dHitEvent& hitEvent )
    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
    void OnChar(wxKeyEvent& event)
    void OnPopUpEvent(a2dCanvasObjectMouseEvent &event)
    void OnEnterObject(a2dCanvasObjectMouseEvent &event)
    void OnLeaveObject(a2dCanvasObjectMouseEvent &event)
    void LeaveInObjects( a2dIterC& ic, a2dHitEvent& hitEvent )
    void OnHandleEvent(a2dHandleMouseEvent &event)
    a2dDrawing *GetRoot() const
    void SetRoot( a2dDrawing *root, bool recurse = true )
    void Prepend( a2dCanvasObject* obj )
    void Append( a2dCanvasObject* obj )
    void Insert( size_t before, a2dCanvasObject* obj, bool ignoreReleased = true )
    a2dCanvasObjectList* GetChildObjectList()
    const a2dCanvasObjectList* GetChildObjectList() const
    a2dCanvasObjectList* CreateChildObjectList()
    unsigned int GetChildObjectsCount() const
    a2dCanvasObject* CreateHierarchy( a2dCanvasObjectFlagsMask mask, bool createref = true )
    virtual void RemoveHierarchy()
    int ReleaseChild( a2dCanvasObject* obj, bool backwards = false, bool all = false, bool now = false )
    bool ReleaseChildObjects( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    int IndexOf( a2dCanvasObject* obj ) const
    a2dCanvasObject* Find( const wxString& objectname, const wxString& classname = "",a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL , const a2dPropertyId *propid = NULL, const wxString& valueAsString = "",wxUint32 id = 0 ) const
    a2dCanvasObject* Find( a2dCanvasObject* obj ) const
    bool SwitchChildNamed( const wxString& objectname, a2dCanvasObject* newobject )
    int CollectObjects( a2dCanvasObjectList* total,const wxString& classname = "",a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL,const a2dPropertyId *id = 0,const a2dBoundingBox& bbox = wxNonValidBbox
    bool GetPending() const
    virtual void SetPending( bool pending )
    void AddPending( a2dIterC& ic )
    virtual void DependencyPending( a2dWalkerIOHandler* handler )
    virtual void Render( a2dIterC& ic, OVERLAP clipparent )
    virtual void DrawHighLighted( a2dIterC& ic )
    void SetVisible(bool visible)
    bool GetVisible() const
    bool IsVisible() const
    void SetFill( const a2dFill& fill )
    a2dFill GetFill() const
    void SetFill( const wxColour& fillcolor, a2dFillStyle style = a2dFILL_SOLID )
    void SetFill(  const wxColour& fillcolor, const wxColour& fillcolor2, a2dFillStyle style = a2dFILL_SOLID )
    void SetFillColour( const wxColour& colour )
    wxColour GetFillColour() const
    void SetStroke(  const wxColour& strokecolor, double width = 0,  a2dStrokeStyle style = a2dSTROKE_SOLID )
    void SetStroke(  const wxColour& strokecolor, int width ,  a2dStrokeStyle style = a2dSTROKE_SOLID )
    void SetStroke( const a2dStroke& stroke)
    a2dStroke GetStroke() const
    void SetStrokeColour( const wxColour& colour )
    wxColour GetStrokeColour() const
    virtual void SetContourWidth(double width )
    virtual double GetContourWidth() const
    void SetDrawerStyle( a2dIterC& ic, a2dStyleProperty* style )
    void SetChildrenOnSameLayer(bool samelayer)
    bool GetChildrenOnSameLayer() const
    void SetViewDependent( a2dDrawingPart* drawingPart, bool viewdependent, bool viewspecific = false, bool onlyinternalarea = false, bool deep = false )
    void SetChildOnlyTranslate(bool onlytranslate)
    bool GetChildOnlyTranslate() const
    void SetPreRenderAsChild(bool prerender)
    bool GetPreRenderAsChild() const
    bool GetFilled() const
    void  SetFilled(bool filled)
    bool GetShowShadow() const
    void SetShowShadow(bool showshadow)
    bool GetResizeOnChildBox() const
    void SetResizeOnChildBox(bool resizeToChilds )
    const a2dShadowStyleProperty* GetShadowStyle() const
    void SetEditable(bool editable)
    bool GetEditable() const
    bool IsEditable()  const
    bool GetSelected() const
    bool IsSelected() const
    void  SetSelected(bool selected)
    bool GetSelectable() const
    bool IsSelectable() const
    void  SetSelectable( bool selectable )
    void SetDraggable(bool draggable)
    bool GetDraggable() const
    bool IsDraggable() const
    bool GetSnap() const
    void SetSnap( bool snap )
    void SetSnapTo( bool snap )
    bool GetSnapTo() const
    virtual void SetMode( int mode )
    virtual int GetMode() const
    virtual a2dCanvasObject* StartEdit( a2dBaseTool* tool, wxUint16 editmode, wxEditStyle editstyle = wxEDITSTYLE_COPY )
    virtual void EndEdit()
    void ReStartEdit( wxUint16 editmode )
    a2dHandle* SetHandlePos( wxString name, double x, double y )
    void CaptureMouse( a2dIterC& ic )
    void ReleaseMouse( a2dIterC& ic )
    bool IsCapturedMouse( a2dIterC& ic ) const
    bool GetHasSelectedObjectsBelow() const
    void SetHasSelectedObjectsBelow(bool value)
    bool GetHasToolObjectsBelow() const
    void SetHasToolObjectsBelow(bool value)
    void SetMode( int mode )
    int GetMode()
    virtual bool IsTemporary_DontSave() const
    virtual bool IsConnect() const
    virtual bool NeedsUpdateWhenConnected() const
    bool DoConnect()
    void DoConnect( bool doconnect )
    bool CanConnectWith( a2dIterC &ic, a2dCanvasObject* toConnect, bool autocreate )
    a2dPin* CanConnectWith( a2dIterC &ic, a2dPin* pin, double margin, bool autocreate )
    bool IsConnected(bool needsupdate, a2dCanvasObject* toConnect = 0 )
    bool GetConnected(a2dCanvasObjectList* connected, bool needsupdate)
    virtual bool ConnectWith( a2dCanvasObject* parent, a2dCanvasObject* toconnect, const wxString& pinname="", double margin = 1, bool undo = false )
    virtual bool ConnectWith( a2dCanvasObject* parent, a2dPin* pin, double margin = 1, bool undo = false )
    virtual void ConnectPinsCreateConnect( a2dCanvasObject* parent, a2dPin* pinc, a2dPin* pinother, bool undo = false )
    void ConnectPins( a2dCanvasObject* parent, a2dPin* pinc, a2dPin* pinother, bool undo = false )
    virtual bool DisConnectWith( a2dCanvasObject* toDisConnect = NULL, const wxString& pinname = "", bool undo = false );
    virtual bool DisConnectAt( a2dPin* pin, bool undo = false )
    void ReWireConnected( a2dCanvasObject* parent, bool undo = false )
    bool CreateWiresOnPins( a2dCanvasObject* parent, bool undo, bool onlyNonSelected = false )
    bool SetConnectedPending( bool onoff, bool needsupdateonly )
    bool HasPins( bool realcheck = false )
    a2dPin* HasPinNamed( const wxString pinName, bool NotConnected = false )
    int GetPinCount()
    virtual bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y )
    virtual bool AdjustPinLocation()
    virtual bool GeneratePinsPossibleConnections( a2dPinClass* pinClass, a2dConnectTask task, double x, double y )
    virtual a2dCanvasObject* GetConnectTemplate( a2dPinClass* mapThis, a2dCanvasObject* other, a2dPinClass* mapOther ) const
    virtual a2dCanvasObject* CreateConnectObject( a2dCanvasObject* parent, a2dPin* pinThis, a2dPin* pinOther, bool undo = false ) const
    a2dPin* AddPin( const wxString name, double x, double y, wxUint32 a2dpinFlags, a2dPinClass* pinClass )
    void RemovePins( bool NotConnected = false, bool onlyTemporary = false, bool now = false )
    void ShowPins( bool onoff )
    void SetRenderConnectedPins( bool onoff )
    virtual void ClearAllPinConnections( bool withundo = true )
    virtual void SetParent( a2dCanvasObject* parent )
    void SetIsProperty(bool IsProperty )
    bool GetIsProperty() const
    a2dText* SetObjectTip( const wxString& tip, double x, double y,  double size, double angle, const a2dFont& font )
    void SetTipWindow( const wxString& tip )
    a2dObject* GetObjectTip()
    virtual bool EditProperties( const a2dPropertyId *id, bool withUndo )
    void SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which )
    bool CheckMask(  a2dCanvasObjectFlagsMask mask)
    void SetFlags( a2dCanvasObjectFlagsMask newmask )
    bool GetFlag( const a2dCanvasObjectFlagsMask which ) const
    a2dCanvasObjectFlagsMask GetFlags() const
    void SetBin(bool bin)
    bool GetBin() const
    bool GetGroupA() const
    void SetGroupA( bool value )
    bool GetGroupB() const
    void SetGroupB( bool value )
    bool GetGeneratePins() const
    void SetGeneratePins( bool value )
    void SetIgnoreSetpending( bool value = true )
    bool GetIgnoreSetpending( ) const
    static void SetIgnoreAllSetpending( bool value = true )
    static bool GetIgnoreAllSetpending( )
    void SetIgnoreLayer( bool value = true )
    bool GetIgnoreLayer( ) const
    void SetSubEdit( bool value )
    bool GetSubEdit( ) const
    void SetSubEditAsChild( bool value )
    bool GetSubEditAsChild( ) const
    void SetShowshadow( bool value )
    bool GetShowshadow( ) const
    void SetPushin( bool value )
    bool GetPushin( ) const
    void SetBin2( bool value )
    bool GetBin2( ) const
    void SetPrerenderaschild( bool value )
    bool GetPrerenderaschild( ) const
    void SetVisiblechilds( bool value )
    bool GetVisiblechilds( ) const
    void SetEditing( bool value )
    bool GetEditing( ) const
    void SetEditingRender( bool value )
    bool GetEditingRender( ) const
    void SetDoConnect( bool value )
    bool GetDoConnect( ) const
    void SetIsOnCorridorPath( bool value )
    bool GetIsOnCorridorPath( ) const
    void SetHasPins( bool value )
    bool GetHasPins( ) const
    void SetMouseInObject( bool value )
    bool GetMouseInObject( ) const
    void SetHighLight( bool value )
    bool GetHighLight( ) const
    wxUint16 GetLayer() const
    virtual void SetLayer( wxUint16 layer )
    virtual a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const
    virtual a2dCanvasObjectList* GetAsPolygons( bool transform = true ) const
    virtual a2dCanvasObjectList* GetAsPolylines( bool transform = true ) const
    void SetTemplate( bool b = true )
    void SetExternal( bool b = true )
    void SetUsed( bool b = true )
    bool GetTemplate() const
    bool GetExternal() const
    bool GetUsed() const
    %member static bool m_ignoreAllSetpending
    %member static a2dPropertyIdMatrix* PROPID_TransformMatrix
    %member static a2dPropertyIdPoint2D* PROPID_Position
    %member static a2dPropertyIdUint16* PROPID_Layer
    %member static a2dPropertyIdBool* PROPID_Selected
    %member static a2dPropertyIdBool* PROPID_Selectable
    %member static a2dPropertyIdBool* PROPID_SubEdit
    %member static a2dPropertyIdBool* PROPID_SubEditAsChild
    %member static a2dPropertyIdBool* PROPID_Visible
    %member static a2dPropertyIdBool* PROPID_Draggable
    %member static a2dPropertyIdBool* PROPID_Showshadow
    %member static a2dPropertyIdBool* PROPID_Filled
    %member static a2dPropertyIdBool* PROPID_GroupA
    %member static a2dPropertyIdBool* PROPID_GroupB
    %member static a2dPropertyIdBool* PROPID_GeneratePins
    %member static a2dPropertyIdBool* PROPID_Bin
    %member static a2dPropertyIdBool* PROPID_Bin2
    %member static a2dPropertyIdBool* PROPID_Pending
    %member static a2dPropertyIdBool* PROPID_Snap
    %member static a2dPropertyIdBool* PROPID_SnapTo
    %member static a2dPropertyIdBool* PROPID_Pushin
    %member static a2dPropertyIdBool* PROPID_Prerenderaschild
    %member static a2dPropertyIdBool* PROPID_Visiblechilds
    %member static a2dPropertyIdBool* PROPID_Editable
    %member static a2dPropertyIdBool* PROPID_Editing
    %member static a2dPropertyIdBool* PROPID_EditingRender
    %member static a2dPropertyIdBool* PROPID_ChildrenOnSameLayer
    %member static a2dPropertyIdBool* PROPID_DoConnect
    %member static a2dPropertyIdBool* PROPID_IsOnCorridorPath
    %member static a2dPropertyIdBool* PROPID_HasPins
    %member static a2dPropertyIdBool* PROPID_IsProperty
    %member static a2dPropertyIdBool* PROPID_MouseInObject
    %member static a2dPropertyIdBool* PROPID_HighLight
    %member static a2dPropertyIdBool* PROPID_Template
    %member static a2dPropertyIdBool* PROPID_External
    %member static a2dPropertyIdBool* PROPID_Used
    %member static a2dPropertyIdBool* PROPID_Release
    %member static a2dPropertyIdCanvasObject* PROPID_Begin
    %member static a2dPropertyIdCanvasObject* PROPID_End
    %member static a2dPropertyIdDouble* PROPID_EndScaleX
    %member static a2dPropertyIdDouble* PROPID_EndScaleY
    %member static a2dPropertyIdBool* PROPID_Spline
    %member static a2dPropertyIdDouble* PROPID_ContourWidth
    %member static a2dPropertyIdBool* PROPID_Allowrotation
    %member static a2dPropertyIdBool* PROPID_Allowsizing
    %member static a2dPropertyIdBool* PROPID_Allowskew
    %member static a2dPropertyIdBool* PROPID_IncludeChildren
    // %member static a2dPropertyIdRefObjectAutoZero* PROPID_Controller
    %member static a2dPropertyIdCanvasObject* PROPID_Original
    %member static a2dPropertyIdCanvasObject* PROPID_Editcopy
    %member static a2dPropertyIdCanvasObject* PROPID_Parent
    %member static a2dPropertyIdCanvasObject* PROPID_Objecttip
    %member static a2dPropertyIdUint16* PROPID_Editmode
    %member static a2dPropertyIdUint16* PROPID_Editstyle
    %member static a2dPropertyIdUint16* PROPID_Index
    %member static a2dPropertyIdCanvasShadowStyle* PROPID_Shadowstyle
    %member static a2dPropertyIdFill* PROPID_Fill
    %member static a2dPropertyIdStroke* PROPID_Stroke
    %member static a2dPropertyIdUint32* PROPID_RefDesCount
    %member static a2dPropertyIdUint32* PROPID_RefDesNr
    %member static a2dPropertyIdBool* PROPID_ToolDecoration
    %member static a2dPropertyIdVoidPtr* PROPID_ToolObject
    %member static a2dPropertyIdBool* PROPID_TemporaryObject
    %member static a2dPropertyIdBoundingBox* PROPID_BoundingBox
    %member static a2dPropertyIdColour* PROPID_StrokeColour
    %member static a2dPropertyIdColour* PROPID_FillColour
    %member static a2dPropertyIdRefObject* PROPID_ViewSpecific
    %member static a2dPropertyIdBool* PROPID_FirstEventInObject
    %member static a2dPropertyIdUint16* PROPID_Datatype
    %member static a2dPropertyIdRefObject* PROPID_ViewDependent
    %member static a2dPropertyIdMatrix   * PROPID_IntViewDependTransform
    %member static a2dPropertyIdDateTime* PROPID_DateTime
    %member static a2dPropertyIdDateTime* PROPID_ModificationDateTime
    %member static a2dPropertyIdDateTime* PROPID_AccessDateTime
    %member static a2dPropertyIdMenu* PROPID_PopupMenu
    %member static a2dPropertyIdWindow* PROPID_TipWindow
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectFilterPropertyNoMaskBlind
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dCanvasObjectFilterPropertyNoMaskBlind : public CanvasObjectFilterOnlyNoMaskBlind
{
    a2dCanvasObjectFilterPropertyNoMaskBlind( const a2dPropertyId *id, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
};

// ---------------------------------------------------------------------------
// a2dIterCU
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dIterCU
{
    a2dIterCU( a2dIterC &ic, a2dCanvasObject *object )
    a2dIterCU( a2dIterC &ic, const a2dAffineMatrix& matrix = a2dIDENTITY_MATRIX )
    a2dIterCU( const a2dIterCU& cu )
    a2dCanvasObject *GetObject()
    const a2dAffineMatrix &GetTransform()
    const a2dAffineMatrix &GetInverseTransform()
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectFilterSelected
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dCanvasObjectFilterSelected : public a2dCanvasObjectFilterOnlyNoMaskBlind
{
    a2dCanvasObjectFilterSelected( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
};

// ---------------------------------------------------------------------------
// a2dIterC
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dIterC
{
    a2dIterC()
    a2dIterC( a2dDrawingPart* drawingPart, int level = 0 )
    void SetDisableInvert( bool disableInvert )
    bool GetDisableInvert()
    void Reset()
    void SetHitMarginDevice( int pixels )
    void SetHitMarginWorld( double world )
    double GetHitMarginWorld()
    double ExtendDeviceToWorld( int extend )
    double GetTransformedHitMargin()
    wxUint16 GetLayer()
    void SetLayer( wxUint16 layer )
    bool GetGenerateCommands()
    void SetGenerateCommands( bool generateCommands )
    const a2dAffineMatrix& GetMappingTransform() const
    const a2dAffineMatrix& GetUserToDeviceTransform() const
    a2dDrawingPart* GetDrawingPart() const
    a2dDrawer2D* GetDrawer2D() const
    void SetDrawStyle( a2dDocumentRenderStyle drawstyle )
    a2dDocumentRenderStyle GetDrawStyle()
    a2dCanvasObject* GetObject()
    const a2dAffineMatrix& GetTransform() const
    const a2dAffineMatrix& GetInverseTransform() const
    const a2dAffineMatrix& GetParentTransform() const
    const a2dAffineMatrix& GetInverseParentTransform() const
    int GetLevel() const
    a2dCanvasObject* GetParent() const
    void SetCorridorPath( bool OnOff )
    void SetCorridorPathToParent()
    void SetCorridorPathToObject( a2dCanvasObject* object )
    bool GetFoundCorridorEnd()
    void SetFoundCorridorEnd( bool foundCorridorEnd )
    void SetStrokeWorldExtend( double worldStrokeExtend )
    double GetWorldStrokeExtend()
    a2dCanvasObjectFilter* GetObjectFilter()
    void SetObjectFilter( a2dCanvasObjectFilter* filter )
    bool FilterObject( a2dCanvasObject* canvasObject )
    void EndFilterObject( a2dCanvasObject* canvasObject )
    void SetPerLayerMode( bool value )
    bool GetPerLayerMode()
    void SetDeepestHit( a2dCanvasObject* canvasObject )
    a2dCanvasObject* GetDeepestHit() const
};

// ---------------------------------------------------------------------------
// a2dExtendedResultItem
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dExtendedResultItem
{
    a2dExtendedResultItem()
    a2dExtendedResultItem( a2dCanvasObject *object, a2dCanvasObject *parent, a2dHit type, int level, int typeex )
    a2dCanvasObject *GetObject()
    a2dCanvasObject *GetParent()
    const a2dHit &GetHitType()
    int GetLevel()
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectFilterOnlyNoMaskBlind
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dCanvasObjectFilterOnlyNoMaskBlind : public a2dCanvasObjectFilter
{
    a2dCanvasObjectFilterOnlyNoMaskBlind( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL )
    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
};

// ---------------------------------------------------------------------------
// a2dCanvasOFlags
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dCanvasOFlags
{
    %member static const a2dCanvasObjectFlagsMask NON
    %member static const a2dCanvasObjectFlagsMask SELECTED
    %member static const a2dCanvasObjectFlagsMask SELECTABLE
    %member static const a2dCanvasObjectFlagsMask HasSelectedObjectsBelow
    %member static const a2dCanvasObjectFlagsMask SubEdit
    %member static const a2dCanvasObjectFlagsMask SubEditAsChild
    %member static const a2dCanvasObjectFlagsMask VISIBLE
    %member static const a2dCanvasObjectFlagsMask DRAGGABLE
    %member static const a2dCanvasObjectFlagsMask SHOWSHADOW
    %member static const a2dCanvasObjectFlagsMask FILLED
    %member static const a2dCanvasObjectFlagsMask A
    %member static const a2dCanvasObjectFlagsMask B
    %member static const a2dCanvasObjectFlagsMask BIN
    %member static const a2dCanvasObjectFlagsMask BIN2
    %member static const a2dCanvasObjectFlagsMask PENDING
    %member static const a2dCanvasObjectFlagsMask SNAP
    %member static const a2dCanvasObjectFlagsMask SNAP_TO
    %member static const a2dCanvasObjectFlagsMask PUSHIN
    %member static const a2dCanvasObjectFlagsMask PRERENDERASCHILD
    %member static const a2dCanvasObjectFlagsMask VISIBLECHILDS
    %member static const a2dCanvasObjectFlagsMask EDITABLE
    %member static const a2dCanvasObjectFlagsMask ChildrenOnSameLayer
    %member static const a2dCanvasObjectFlagsMask DoConnect
    %member static const a2dCanvasObjectFlagsMask IsOnCorridorPath
    %member static const a2dCanvasObjectFlagsMask HasPins
    %member static const a2dCanvasObjectFlagsMask Editing
    %member static const a2dCanvasObjectFlagsMask EditingCopy
    %member static const a2dCanvasObjectFlagsMask IsProperty
    %member static const a2dCanvasObjectFlagsMask MouseInObject
    %member static const a2dCanvasObjectFlagsMask HighLight
    %member static const a2dCanvasObjectFlagsMask AlgoSkip
    %member static const a2dCanvasObjectFlagsMask ignoreSetpending
    %member static const a2dCanvasObjectFlagsMask HasToolObjectsBelow
    %member static const a2dCanvasObjectFlagsMask ChildOnlyTranslate
    %member static const a2dCanvasObjectFlagsMask ignoreLayer
    %member static const a2dCanvasObjectFlagsMask generatePins
    %member static const a2dCanvasObjectFlagsMask ALL
    a2dCanvasOFlags( a2dCanvasObjectFlagsMask newmask = a2dCanvasOFlags::NON )
    %member bool m_pushin
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectFilterLayerMaskNoToolNoEdit
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dCanvasObjectFilterLayerMaskNoToolNoEdit : public a2dCanvasObjectFilterLayerMask
{
    a2dCanvasObjectFilterLayerMaskNoToolNoEdit( wxUint16 layer, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, a2dCanvasObjectFlagsMask antimask = a2dCanvasOFlags::NON )
    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
    virtual void EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectFilterLayerMask
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dCanvasObjectFilterLayerMask : public a2dCanvasObjectFilter
{
    a2dCanvasObjectFilterLayerMask( wxUint16 layer, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL, a2dCanvasObjectFlagsMask antimask = a2dCanvasOFlags::NON )
    virtual bool Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
};

// ---------------------------------------------------------------------------
// a2dHitEvent
// ---------------------------------------------------------------------------

#include "wx/canvas/canobj.h"
class a2dHitEvent
{
    a2dHitEvent( double absx = 0, double absy = 0, bool continues = true,a2dHitOption option = a2dCANOBJHITOPTION_NONE, bool wantExtended = false )
    %member double m_relx
    %member double m_rely
    %member double m_x
    %member double m_y
    %member bool m_xyRelToChildren
    %member wxUint32 m_option
    %member a2dHit m_how
    // %member a2dExtendedResult m_extended
    %member bool m_extendedWanted
    %member int m_maxlevel
    %member wxEvent* m_event
    %member bool m_processed
    %member bool m_continue
    %member bool m_isHit
    %member int m_id
};

// *****************************************************************
// wx/canvas\canprim.h
// *****************************************************************

enum a2dPinClass::a2dPinClassFlags
{
    PC_angle,
    PC_ALLSET
};

enum a2dPin::a2dPinFlags
{
    NON,
    dynamic,
    temporary,
    objectPin,
    internal,
    temporaryObjectPin,
    ALLSET
};

// ---------------------------------------------------------------------------
// a2dPin
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dPin : public a2dHandle
{
    a2dPin()
    a2dPin( a2dCanvasObject* parent, const wxString& name, a2dPinClass* pinclass,  double xc, double yc, double angle = 0 , int w = 0, int h = 0,  int radius = 0 )
    a2dPin( const a2dPin &other, a2dObject::CloneOptions options, a2dRefMap* refs )
    void SetRenderConnected(bool RenderConnected)
    bool GetRenderConnected()
    virtual void SetPending( bool pending )
    virtual void SetParent( a2dCanvasObject* parent )
    void Set(double xc, double yc, double angle =0 , const wxString& name = "", bool dynamic = false )
    a2dPoint2D GetAbsXY() const
    double GetAbsX() const
    double GetAbsY() const
    void SetAbsXY( double x, double y )
    void SetAbsXY( const a2dPoint2D & point )
    double GetAbsAngle() const
    a2dPinClass *GetPinClass() const
    void SetPinClass( a2dPinClass* pinClass )
    virtual bool MayConnectTo( a2dPin* connectto )
    void ConnectTo( a2dPin* connectto )
    a2dPin* IsConnectedTo( a2dPin* pin ) const
    a2dPin* FindConnectablePin( a2dCanvasObject *root, double margin, bool autocreate )
    void AutoConnect( a2dCanvasObject *root, double margin )
    a2dPin* IsDislocated() const
    bool IsDynamicPin() const
    void SetDynamicPin( bool dynamicPin )
    bool IsTemporaryPin() const
    void SetTemporaryPin( bool temporaryPin )
    void SetInternal( bool internal )
    bool IsInternal() const
    void SetObjectPin( bool objectPin )
    bool IsObjectPin() const
    virtual bool IsTemporary_DontSave() const
    virtual bool AlwaysWriteSerializationId() const
    virtual bool LinkReference( a2dObject *other )
    %member const static long sm_PinUnConnected
    %member const static long sm_PinConnected
    %member const static long sm_PinCanConnect
    %member const static long sm_PinCannotConnect
    %member const static long sm_PinCanConnectToPinClass
};

// ---------------------------------------------------------------------------
// a2dEndsEllipticChord
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dEndsEllipticChord : public a2dEllipticArc
{
    a2dEndsEllipticChord()
    a2dEndsEllipticChord( double xc, double yc, double width, double height, double start, double end )
    a2dEndsEllipticChord( const a2dEndsEllipticChord& ori, a2dObject::CloneOptions options, a2dRefMap* refs )
    void SetBeginObj(a2dCanvasObject* begin)
    a2dCanvasObject* GetBeginObj()
    void SetEndObj(a2dCanvasObject* endob)
    a2dCanvasObject* GetEndObj()
    void SetEndScaleX(double xs)
    double GetEndScaleX()
    void SetEndScaleY(double ys)
    double GetEndScaleY()
    void DoWalker( wxObject *parent, a2dWalkerIOHandler& handler )
};

// ---------------------------------------------------------------------------
// a2dEllipticArc
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dEllipticArc : public a2dWHCenter
{
    a2dEllipticArc()
    a2dEllipticArc( double xc, double yc, double width, double height, double start, double end, bool chord = false, double contourwidth = 0 )
    a2dEllipticArc( const a2dEllipticArc& ori, a2dObject::CloneOptions options, a2dRefMap* refs )
    void SetStart(double start)
    double  GetStart()
    void SetEnd(double end)
    double GetEnd()
    void SetChord( bool chord )
    bool GetChord()
    a2dVertexList* GetAsVertexList( bool& returnIsPolygon )
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
    void SetContourWidth(double width)
    double GetContourWidth() const
    %member static const a2dPropertyIdDouble* PROPID_StartAngle
    %member static const a2dPropertyIdDouble* PROPID_EndAngle
    %member static const a2dPropertyIdBool* PROPID_Chord
};

// ---------------------------------------------------------------------------
// a2dRect
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dRect : public a2dWH
{
    a2dRect()
    a2dRect( double x, double y, double w, double h , double radius=0, double contourwidth = 0 )
    a2dRect( const a2dPoint2D& p1,  const a2dPoint2D& p2, double radius=0, double contourwidth = 0 )
    a2dRect( const a2dBoundingBox& bbox, double radius=0, double contourwidth = 0 )
    a2dRect( const a2dRect& rec, a2dObject::CloneOptions options, a2dRefMap* refs )
    void SetContourWidth(double width)
    double GetContourWidth() const
    void    SetRadius( double radius )
    double  GetRadius() const
    void SetBoxType( int type )
    int GetBoxType() const
    a2dVertexList* GetAsVertexList( bool& returnIsPolygon )
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
    bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y  )
    void OnHandleEvent(a2dHandleMouseEvent &event)
    %member static const a2dPropertyIdDouble* PROPID_Radius
};

// ---------------------------------------------------------------------------
// a2dEndsLine
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dEndsLine : public a2dSLine
{
    a2dEndsLine()
    a2dEndsLine( double x1, double y1, double x2, double y2 )
    a2dEndsLine( const a2dEndsLine& ori, a2dObject::CloneOptions options, a2dRefMap* refs )
    void SetBegin(a2dCanvasObject* begin)
    a2dCanvasObject* GetBegin()
    void SetEnd(a2dCanvasObject* end)
    a2dCanvasObject* GetEnd()
    void SetEndScaleX(double xs)
    double GetEndScaleX()
    void SetEndScaleY(double ys)
    double GetEndScaleY()
    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
};

// ---------------------------------------------------------------------------
// a2dWires
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dWires : public a2dCanvasObject
{
    a2dWires()
    a2dWires( a2dCanvasObject* toconnect, const wxString& pinname )
    a2dWires( a2dCanvasObject* toconnect, a2dPin* pinc )
    virtual bool IsConnect() const
    bool NeedsUpdateWhenConnected() const
    bool ConnectWith(  a2dCanvasObject* parent, a2dCanvasObject* graph, const wxString& pinname, double margin = 1, bool undo = false )
    bool ConnectWith(  a2dCanvasObject* parent, a2dPin* pin, double margin = 1, bool undo = false )
    bool GetFrozen()
    void SetFrozen( bool freeze )
};

// ---------------------------------------------------------------------------
// a2dCircle
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dCircle : public a2dCanvasObject
{
    a2dCircle( const a2dCircle& ori, a2dObject::CloneOptions options, a2dRefMap* refs )
    a2dCircle()
    a2dCircle( double x, double y, double radius, double width = 0 )
    void SetRadius(double radius)
    double  GetRadius() const
    void SetContourWidth(double width)
    double GetContourWidth() const
    a2dVertexList* GetAsVertexList( bool& returnIsPolygon )
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
    bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y )
    %member static const a2dPropertyIdDouble* PROPID_Radius
};

// ---------------------------------------------------------------------------
// a2dArrow
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dArrow : public a2dCanvasObject
{
    a2dArrow( double xt, double yt, double l1, double l2, double b, bool spline = false)
    a2dArrow()
    a2dArrow( const a2dArrow& ori, a2dObject::CloneOptions options, a2dRefMap* refs )
    void    Set(double xt, double yt, double l1, double l2, double b)
    void    Set( double l1, double l2, double b)
    double GetL1()
    double GetL2()
    double GetBase()
    bool GetSpline()
    a2dVertexList* GetAsVertexList( bool& returnIsPolygon )
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
};

// ---------------------------------------------------------------------------
// a2dOrigin
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dOrigin : public a2dCanvasObject
{
    a2dOrigin()
    a2dOrigin( double w, double h)
    a2dOrigin( const a2dOrigin &other, a2dObject::CloneOptions options, a2dRefMap* refs )
    double  GetWidth()
    double  GetHeight()
    void SetWidth( double width )
    void SetHeight( double height )
};

// ---------------------------------------------------------------------------
// a2dHandle
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dHandle : public a2dCanvasObject
{
    a2dHandle()
    a2dHandle( a2dCanvasObject* parent, double xc, double yc, const wxString& name = "", int w = 0, int h = 0, double angle = 0 , int radius = 0 )
    a2dHandle( const a2dHandle& other, a2dObject::CloneOptions options, a2dRefMap* refs )
    wxString GetName() const
    void SetName(const wxString& name)
    void Set(double xc, double yc, int w, int h, double angle = 0 , int radius = 0 )
    void Set2( double xc, double yc, const wxString& name = "" )
    void SetParent( a2dCanvasObject* parent )
    a2dCanvasObject* GetParent() const
    int  GetWidth() const
    int  GetHeight() const
    int  GetRadius() const
    void SetMode( int mode )
    int GetMode() const
    virtual bool IsTemporary_DontSave() const
    %member static a2dPropertyIdInt32* PROPID_Width
    %member static a2dPropertyIdInt32* PROPID_Height
    %member static a2dPropertyIdInt32* PROPID_Radius
    %member const static long sm_HandleNoHit
    %member const static long sm_HandleHit
};

// ---------------------------------------------------------------------------
// a2dEllipse
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dEllipse : public a2dWHCenter
{
    a2dEllipse()
    a2dEllipse( double xc, double yc, double width, double height, double contourwidth = 0 )
    a2dEllipse( const a2dEllipse& ori, a2dObject::CloneOptions options, a2dRefMap* refs )
    a2dVertexList* GetAsVertexList( bool& returnIsPolygon )
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
    void SetContourWidth(double width)
    double GetContourWidth() const
    bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y )
};

// ---------------------------------------------------------------------------
// a2dRectC
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dRectC : public a2dCanvasObject
{
    a2dRectC()
    a2dRectC( double xc, double yc, double w, double h, double angle =0 , double radius=0)
    a2dRectC( const a2dRectC& ori, a2dObject::CloneOptions options, a2dRefMap* refs )
    void    Set(double xc, double yc, double w, double h, double angle =0 , double radius=0)
    void    SetWidth( double w)
    void    SetHeight( double h)
    double  GetWidth() const
    double  GetHeight() const
    double  GetRadius() const
    a2dVertexList* GetAsVertexList( bool& returnIsPolygon )
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
    %member static a2dPropertyIdDouble* PROPID_Width
    %member static a2dPropertyIdDouble* PROPID_Height
    %member static a2dPropertyIdDouble* PROPID_Radius
};

// ---------------------------------------------------------------------------
// a2dArc
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dArc : public a2dCanvasObject
{
    a2dArc()
    a2dArc( double xc, double yc, double radius, double start, double end, bool chord = false, double contourwidth = 0 )
    a2dArc( double x1, double y1, double x2, double y2, double xc, double yc, bool chord = false, double contourwidth = 0 )
    a2dArc( const a2dArc& ori, a2dObject::CloneOptions options, a2dRefMap* refs )
    double GetX1() const
    double GetY1() const
    double GetX2() const
    double GetY2() const
    double GetStartAngle() const
    double GetEndAngle() const
    a2dPoint2D GetMidPoint() const
    void SetX1( double x1 )
    void SetY1( double y1 )
    void SetX2( double x2 )
    void SetY2( double y2 )
    void SetStartAngle( double a )
    void SetEndAngle( double a )
    void Set( double xs, double ys,  double xm, double ym, double xe, double ye )
    void SetRadius( double radius )
    double GetRadius() const
    void SetChord( bool chord )
    bool GetChord() const
    a2dVertexList* GetAsVertexList( bool& returnIsPolygon )
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
    void SetContourWidth(double width)
    double GetContourWidth() const
    %member static a2dPropertyIdDouble* PROPID_X1
    %member static a2dPropertyIdDouble* PROPID_X2
    %member static a2dPropertyIdDouble* PROPID_Y1
    %member static a2dPropertyIdDouble* PROPID_Y2
    %member static a2dPropertyIdBool* PROPID_Chord
    %member static a2dPropertyIdDouble* PROPID_StartAngle
    %member static a2dPropertyIdDouble* PROPID_EndAngle
};

// ---------------------------------------------------------------------------
// a2dPinClass
// ---------------------------------------------------------------------------
    
#include "wx/canvas/canprim.h"
class a2dPinClass
{
    a2dPinClass( const wxString &name )
    void AddConnect( a2dPinClass* pinClass )
    void RemoveConnect( a2dPinClass* pinClass )
    static void InitializeStockPinClasses()
    static void DeleteStockPinClasses()
    const wxString &GetName() const
    bool HasAngleLine() const
    void SetAngleLine( bool value )
    wxUint32 GetFlags()
    a2dPinClass*  CanConnectTo( a2dPinClass *other = NULL ) const
    // std::list< a2dPinClass* >& GetConnectList()
    static a2dPinClass *GetClassByName( const wxString &name )
    void SetPin( a2dPin* newpin )
    a2dPin* GetPin()
    void SetPinCanConnect( a2dPin* newpin )
    a2dPin* GetPinCanConnect()
    void SetPinCannotConnect( a2dPin* newpin )
    a2dPin* GetPinCannotConnect()
    void SetConnectionGenerator( a2dConnectionGenerator *connectionGenerator )
    a2dConnectionGenerator* GetConnectionGenerator() const
    a2dPinClass* GetPinClassForTask( a2dConnectTask task, a2dCanvasObject* obj ) const
    // %member static std::list< a2dPinClass* > m_allPinClasses
    %member static a2dPinClass* Any
    %member static a2dPinClass* Standard
};

// ---------------------------------------------------------------------------
// a2dWHCenter
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dWH : public a2dCanvasObject
{
};

#include "wx/canvas/canprim.h"
class a2dWHCenter : public a2dWH
{
    //a2dWHCenter()
    //a2dWHCenter( double xc, double yc, double w, double h)
    //a2dWHCenter( const a2dWHCenter& other, a2dObject::CloneOptions options, a2dRefMap* refs )
    //%member static a2dPropertyIdDouble* PROPID_Width
    //%member static a2dPropertyIdDouble* PROPID_Height
};

// ---------------------------------------------------------------------------
// a2dImage
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dImage : public a2dCanvasObject
{
    a2dImage()
    a2dImage( const wxImage &image, double xc, double yc, double w, double h )
    a2dImage( const wxString &imagefile, wxBitmapType type, double xc, double yc, double w, double h )
    a2dImage( a2dCanvasObject* torender, double xc, double yc, double w, double h, int imagew = 100, int imageh = 100 )
    a2dImage( const a2dImage& ori, a2dObject::CloneOptions options, a2dRefMap* refs )
    a2dCanvasObjectList* GetAsRectangles( const wxColour& col1,  const wxColour& col2, bool transform )
    void RenderObject( a2dCanvasObject* torender, int imagew = 100 , int imageh = 100 )
    double GetWidth() const
    double GetHeight() const
    void SetWidth( double width )
    void SetHeight( double height )
    wxImage& GetImage()
    void SetImage( const wxImage& image )
    wxString& GetFilename()
    void SetFilename( const wxString filename, wxBitmapType type, bool doread = true )
    void SetImageType( wxBitmapType type )
    wxBitmapType GetImageType()
    void SetDrawPatternOnTop(bool drawPatternOnTop)
    bool GetDrawPatternOnTop()
    %member static a2dPropertyIdDouble* PROPID_Width
    %member static a2dPropertyIdDouble* PROPID_Height
};

// ---------------------------------------------------------------------------
// a2dSLine
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dSLine : public a2dCanvasObject
{
    a2dSLine()
    a2dSLine( double x1, double y1, double x2, double y2, double contourwidth = 0 )
    a2dSLine( const a2dSLine& ori, a2dObject::CloneOptions options, a2dRefMap* refs )
    double  GetPosX1( bool transform = true ) const
    double  GetPosY1( bool transform = true ) const
    double  GetPosX2( bool transform = true ) const
    double  GetPosY2( bool transform = true ) const
    double GetLength() const
    bool EliminateMatrix()
    void    SetPosXY1( double x, double y, bool afterinversion = true )
    void    SetPosXY2( double x, double y, bool afterinversion = true )
    void    SetPosXY12( double x1, double y1, double x2, double y2, bool afterinversion = true )
    virtual void SetBegin(a2dCanvasObject *begin)
    virtual void SetEnd(a2dCanvasObject *end )
    virtual void SetEndScaleX(double xs )
    virtual void SetEndScaleY(double ys )
    void SetContourWidth(double width)
    double GetContourWidth() const
    void SetPathType( a2dPATH_END_TYPE pathtype )
    a2dPATH_END_TYPE GetPathType()
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
    bool RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld  )
    %member static a2dPropertyIdDouble* PROPID_X1
    %member static a2dPropertyIdDouble* PROPID_X2
    %member static a2dPropertyIdDouble* PROPID_Y1
    %member static a2dPropertyIdDouble* PROPID_Y2
};

// ---------------------------------------------------------------------------
// a2dControl
// ---------------------------------------------------------------------------

#include "wx/canvas/canprim.h"
class a2dControl : public a2dRect
{
    a2dControl( double x, double y, double width, double height, wxWindow *control )
    a2dControl( const a2dControl& ori, a2dObject::CloneOptions options, a2dRefMap* refs )
    wxWindow* GetControl()
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectPtrProperty
// ---------------------------------------------------------------------------

#include "wx/canvas/canprop.h"
class a2dCanvasObjectPtrProperty : public a2dNamedProperty
{
    a2dCanvasObjectPtrProperty()
    a2dCanvasObjectPtrProperty( const a2dPropertyIdCanvasObject* id, a2dCanvasObject* object, bool visible=false, bool render=false )
    //a2dCanvasObjectPtrProperty(const a2dCanvasObjectPtrProperty &other, a2dObject::CloneOptions options, a2dRefMap* refs )
    virtual void Assign( const a2dNamedProperty &other )
    void SetObject( wxObject* object )
    a2dCanvasObject* GetCanvasObject() const
    virtual a2dObject* GetRefObject() const
    virtual a2dObject* GetRefObjectNA() const
    virtual void SetVisible(bool visible)
    virtual bool GetVisible() const
    void SetCanRender(bool render)
    bool GetCanRender() const
    void SetPreRender( bool prerender )
    bool GetPreRender() const
    void SetSelectedOnly( bool selectedonly )
    bool GetSelectedOnly() const
    a2dCanvasObject *GetValue() const
    void SetValue( a2dCanvasObject *newvalue )
    a2dCanvasObjectPtr *GetValuePtr()
    a2dCanvasObjectPtr &GetValueRef()
    const a2dCanvasObjectPtr *GetValuePtr() const
    const a2dCanvasObjectPtr &GetValueRef() const
    static a2dCanvasObjectPtrProperty *CreatePropertyFromString( const a2dPropertyIdCanvasObject* id, const wxString &value )
};

// ---------------------------------------------------------------------------
// a2dShadowStyleProperty
// ---------------------------------------------------------------------------

#include "wx/canvas/canprop.h"
class a2dShadowStyleProperty : public a2dStyleProperty
{
    a2dShadowStyleProperty()
    a2dShadowStyleProperty( const a2dPropertyIdCanvasShadowStyle* id, double depth, double angle = 30)
    a2dShadowStyleProperty(const a2dShadowStyleProperty &other)
    double GetExtrudeDepth() const
    double GetExtrudeAngle() const
    void SetExtrudeDepth( double depth )
    void SetExtrudeAngle( double angle3d )
};

// ---------------------------------------------------------------------------
// a2dStyleProperty
// ---------------------------------------------------------------------------

#include "wx/canvas/canprop.h"
class a2dStyleProperty : public a2dNamedProperty
{
    a2dStyleProperty()
    a2dStyleProperty( const a2dPropertyId* id )
    a2dStyleProperty(const a2dStyleProperty &other)
    virtual void Assign( const a2dNamedProperty &other )
    bool AllNo()
    const a2dFill& GetFill() const
    const a2dStroke& GetStroke() const
    void SetFill( const a2dFill& fill )
    void SetFill( const wxColour& fillcolor, a2dFillStyle style = a2dFILL_SOLID )
    void SetFill(  const wxColour& fillcolor, const wxColour& fillcolor2, a2dFillStyle style = a2dFILL_SOLID )
    void SetStroke(  const wxColour& strokecolor, float width = 0,  a2dStrokeStyle style = a2dSTROKE_SOLID )
    void SetStroke(  const wxColour& strokecolor, int width ,  a2dStrokeStyle style = a2dSTROKE_SOLID )
    void SetStroke( const a2dStroke& stroke)
    wxString StringRepresentation() const
    wxString StringValueRepresentation() const
};

// ---------------------------------------------------------------------------
// a2dBoudingBoxProperty
// ---------------------------------------------------------------------------

#include "wx/canvas/canprop.h"
class a2dBoudingBoxProperty : public a2dNamedProperty
{
    a2dBoudingBoxProperty()
    a2dBoudingBoxProperty( const a2dPropertyIdBoundingBox* id, const a2dBoundingBox& value )
    a2dBoudingBoxProperty( const a2dPropertyIdBoundingBox* id, const wxString& value )
    a2dBoudingBoxProperty( const a2dBoudingBoxProperty &other )
    virtual void Assign( const a2dNamedProperty &other )
    static a2dBoudingBoxProperty *CreatePropertyFromString( const a2dPropertyIdBoundingBox* id, const wxString &value )
    void SetValue( const a2dBoundingBox& value )
    a2dBoundingBox& GetValue()
    a2dBoundingBox* GetValuePtr()
    const a2dBoundingBox& GetValue() const
    const a2dBoundingBox* GetValuePtr() const
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
};

// ---------------------------------------------------------------------------
// a2dVisibleProperty
// ---------------------------------------------------------------------------

#include "wx/canvas/canprop.h"
class a2dVisibleProperty : public a2dText
{
    a2dVisibleProperty()
    a2dVisibleProperty( const a2dVisibleProperty &other, a2dObject::CloneOptions options, a2dRefMap* refs )
    a2dVisibleProperty( a2dCanvasObject* parent, const a2dPropertyId* property, double x, double y, double angle = 0 )
    // a2dVisibleProperty( a2dCanvasObject* parent, const a2dPropertyId* property, double x, double y, bool visible = true,const a2dFont& font = *a2dDEFAULT_CANVASFONT, double angle = 0)
    void ShowName( bool show = false )
    void SetParent( a2dCanvasObject* parent )
    a2dCanvasObject* GetParent() const
    const a2dPropertyId* GetPropId()
    void OnChar(wxKeyEvent& event)
    void OnMouseEvent(a2dCanvasObjectMouseEvent &event)
};

// ---------------------------------------------------------------------------
// a2dClipPathProperty
// ---------------------------------------------------------------------------

#include "wx/canvas/canprop.h"
class a2dClipPathProperty : public a2dNamedProperty
{
    a2dClipPathProperty()
    a2dClipPathProperty( const a2dPropertyIdCanvasClipPath* id, a2dPolygonL* clip )
    a2dClipPathProperty(const a2dClipPathProperty &other, a2dObject::CloneOptions options )
    void Assign( const a2dNamedProperty &other )
    virtual a2dPolygonL* GetClipObject() const
    void SetCanvasObject( a2dPolygonL* clip )
    void PushClip( a2dDrawingPart* drawer, a2dBooleanClip clipoperation = a2dCLIP_AND )
    void PopClip( a2dDrawingPart* drawer )
    virtual void SetVisible(bool visible)
    virtual bool GetVisible() const
    void SetCanRender(bool render)
    bool GetCanRender() const
    virtual a2dObject* GetRefObject() const
    virtual a2dObject* GetRefObjectNA() const
};

// *****************************************************************
// wx/canvas\cansim.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dCanvasSim
// ---------------------------------------------------------------------------

#include "wx/canvas/cansim.h"
class a2dCanvasSim : public a2dDocumentViewScrolledWindow
{
    a2dCanvasSim( wxWindow *parent, wxWindowID id = -1,const wxPoint& pos = wxDefaultPosition,const wxSize& size = wxDefaultSize,long style = wxScrolledWindowStyle )
    void SetMappingShowAll( bool centre = true )
    a2dDrawer2D *GetDrawer2D() const
    void ClearBackground()
    void SetBackgroundFill( const a2dFill& backgroundfill )
    virtual void Refresh( bool eraseBackground = true, const wxRect* rect = NULL )
    void SetGridAtFront(bool gridatfront)
    bool GetGridAtFront()
    void SetGridStroke( const a2dStroke& gridstroke)
    void SetGridSize( wxUint16 gridsize)
    void SetGridFill( const a2dFill& gridfill)
    double GetGridX()
    void SetGridX(double gridx)
    double GetGridY()
    void SetGridY(double gridy)
    void SetGrid(bool grid)
    bool GetGrid()
    void SetGridLines(bool gridlines)
    bool GetGridLines()
    void SetShowOrigin(bool show)
    void SetYaxis(bool up)
    bool GetYaxis() const
    void SetZoomOutBorder( wxUint16 border )
    bool IsFrozen()
    void Freeze()
    void Thaw()
    void SetMappingWidthHeight( double vx1, double vy1, double width, double height )
    void SetMappingUpp( double vx1, double vy1, double xpp, double ypp)
    double DeviceToWorldX(int x) const
    double DeviceToWorldY(int y) const
    double DeviceToWorldXRel(int x) const
    double DeviceToWorldYRel(int y) const
    int WorldToDeviceX(double x) const
    int WorldToDeviceY(double y) const
    int WorldToDeviceXRel(double x) const
    int WorldToDeviceYRel(double y) const
    a2dCanvasObject* SetShowObject(const wxString& name)
    bool SetShowObject(a2dCanvasObject* obj)
    a2dCanvasObject* GetShowObject() const
    a2dCanvasObject* IsHitWorld( double x, double y, int layer = wxLAYER_ALL, a2dHitOption option = a2dCANOBJHITOPTION_NOROOT | a2dCANOBJHITOPTION_LAYERS )
    void SetMouseEvents(bool onoff)
    bool GetMouseEvents()
    bool WriteSVG(const wxString& filename, double Width, double Height, wxString unit)
};

// *****************************************************************
// wx/canvas\cantext.h
// *****************************************************************

enum a2dText::a2dTextFlags
{
    a2dCANOBJTEXT_MULTILINE,
    a2dCANOBJTEXT_READONLY,
    a2dCANOBJTEXT_CARETVISIBLE,
    a2dCANOBJTEXT_NEXTLINEUP,
    a2dCANOBJTEXT_ENGINEERING,
    a2dCANOBJTEXT_BACKGROUND,
    a2dCANOBJTEXT_FRAME,
    a2dCANVASTEXT_DEFAULTFLAGS
};

// ---------------------------------------------------------------------------
// a2dText
// ---------------------------------------------------------------------------

#include "wx/canvas/cantext.h"
class a2dText : public a2dCanvasObject
{
    a2dText( const wxString &text = "not specified" )
    a2dText( const wxString &text, double x, double y,const a2dFont& font, double angle = 0.0,bool up = false, int alignment = a2dDEFAULT_ALIGNMENT )
    virtual a2dObject* Clone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
    int GetLines() const
    int GetAsArray( wxArrayString& array )
    wxString GetLine(int line) const
    void SetText( const wxString& text )
    wxString GetText() const
    void SetTextHeight( double height )
    double GetTextHeight() const
    double GetLineHeight() const
    void SetLineSpacing( double linespace )
    double GetLineSpacing() const
    void SetTextFlags( unsigned int textflags )
    unsigned int GetTextFlags() const
    void SetMultiLine(bool multiline = true)
    bool GetMultiLine() const
    void SetReadOnly(bool readonly = true)
    bool GetReadOnly()
    void SetNextLineDirection( bool up )
    bool GetNextLineDirection() const
    void SetEngineeringText( bool engineering = true )
    bool GetEngineeringText() const
    void SetBackGround(bool background = true)
    bool GetBackGround() const
    void SetDrawFrame(bool frame = true)
    bool GetDrawFrame() const
    void SetFont( const a2dFont& font )
    a2dFont GetFont() const
    void SetCaret( int position )
    int GetCaret() const
    bool SetCaret( a2dIterC& ic, double x, double y )
    void SetCaretShow( bool visible = true )
    bool GetCaretShow() const
    void SetAlignment(int alignment)
    int GetAlignment() const
    bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y )
    bool AdjustPinLocation()
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
};

// *****************************************************************
// wx/canvas\canvas.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dCanvas
// ---------------------------------------------------------------------------

#include "wx/canvas/canvas.h"
class a2dCanvas : public wxWindow
{
    a2dCanvas( wxWindow *parent, wxWindowID id = -1,const wxPoint& pos = wxDefaultPosition,const wxSize& size = wxDefaultSize,long style = wxScrolledWindowStyle, a2dDrawer2D *drawer2D = 0 )
    a2dCanvas( wxWindow *parent, wxWindowID id = -1,const wxPoint& pos = wxDefaultPosition,const wxSize& size = wxDefaultSize,long style = wxScrolledWindowStyle )
    a2dDrawer2D *GetDrawer2D() const
    void ClearBackground()
    virtual void Refresh( bool eraseBackground = true, const wxRect* rect = NULL )
    void SetBackgroundFill( const a2dFill& backgroundfill )
    void SetGridAtFront(bool gridatfront)
    bool GetGridAtFront()
    void SetGridStroke( const a2dStroke& gridstroke)
    void SetGridSize(wxUint16 gridsize)
    void SetGridFill( const a2dFill& gridfill)
    double GetGridX()
    void SetGridX(double gridx)
    double GetGridY()
    void SetGridY(double gridy)
    void SetGrid(bool grid)
    bool GetGrid()
    void SetGridLines(bool gridlines)
    bool GetGridLines()
    void SetShowOrigin(bool show)
    void SetYaxis(bool up)
    bool GetYaxis() const
    void SetScaleOnResize(bool val)
    bool GetScaleOnResize()
    void SetContinuesSizeUpdate(bool val)
    bool GetContinuesSizeUpdate()
    void AppendEventHandler(wxEvtHandler *handler)
    wxEvtHandler* RemoveLastEventHandler(bool deleteHandler)
    bool IsFrozen()
    void Freeze()
    void Thaw()
    void SetMappingWidthHeight( double vx1, double vy1, double width, double height,bool scrollbars )
    void SetMappingShowAll( bool centre = true )
    void ZoomOut( double n )
    void ZoomOutAtXy( int x, int y, double n )
    void SetMappingUpp( double vx1, double vy1, double xpp, double ypp)
    void SetScrollbars(double pixelsPerUnitX,double pixelsPerUnitY,double noUnitsX, double noUnitsY,double xPos = 0, double yPos = 0 )
    double GetVisibleMinX() const
    double GetVisibleMinY() const
    double GetVisibleMaxX() const
    double GetVisibleMaxY() const
    double GetVisibleWidth() const
    double GetVisibleHeight() const
    double GetUppX() const
    double GetUppY() const
    bool ScrollWindowConstrained( double dx, double dy )
    bool SetScrollMaximum(double vx1, double vy1, double vx2, double vy2)
    void SetMinX( double vxmin )
    void SetMinY( double vymin )
    double GetScrollMinX() const
    double GetScrollMinY() const
    double GetScrollMaxX() const
    double GetScrollMaxY() const
    void SetScrollStepX(double x)
    void SetScrollStepY(double y)
    void FixScrollMaximum(bool fixed)
    void SetScrollBarsVisible(bool onoff)
    void ClipToScrollMaximum(bool clip)
    bool CheckInsideScrollMaximum( double worldminx, double worldminy)
    a2dCanvasObject* SetShowObject(const wxString& name)
    bool SetShowObject(a2dCanvasObject* obj)
    a2dCanvasObject* GetShowObject() const
    a2dCanvasObject* IsHitWorld( double x, double y, int layer = wxLAYER_ALL, a2dHitOption option = a2dCANOBJHITOPTION_NOROOT | a2dCANOBJHITOPTION_LAYERS )
    void SetMouseEvents(bool onoff)
    bool GetMouseEvents()
    bool WriteSVG(const wxString& filename, double Width, double Height, wxString unit)
};

// *****************************************************************
// wx/canvas\canwidget.h
// *****************************************************************

enum a2dWidgetButton::WidgetMode
{
    Boundingbox,
    BoundingboxSize,
    ScaledContent,
    ScaledContentKeepAspect
};

// ---------------------------------------------------------------------------
// a2dWidgetButtonGroup
// ---------------------------------------------------------------------------

#include "wx/canvas/canwidget.h"
class a2dWidgetButtonGroup : public a2dCanvasObject
{
    a2dWidgetButtonGroup( a2dCanvasObject* parent, double x, double y, float extra=0 )
    virtual wxEvtHandler* GetParentEvtHandler()
    void SetSingleSelect( bool singleSelect )
    virtual void SetParent( a2dCanvasObject* parent )
};

// ---------------------------------------------------------------------------
// a2dWidgetButtonCommand
// ---------------------------------------------------------------------------

#include "wx/canvas/canwidget.h"
class a2dWidgetButtonCommand : public a2dWidgetButton
{
    a2dWidgetButtonCommand( a2dCanvasObject* parent, int buttonId, double x, double y, double w, double h, a2dWidgetButton::WidgetMode mode = a2dWidgetButton::Boundingbox )
    void SetCanvasCommand( a2dCommand* command )
    void SetEvent( wxEvent *event )
    void SetClientData( wxClientData *clientData )
};

// ---------------------------------------------------------------------------
// a2dWidgetButton
// ---------------------------------------------------------------------------
enum a2dWidgetButton::WidgetMode 
{
    Boundingbox,
    BoundingboxSize,
    ScaledContent,
    ScaledContentKeepAspect
};

#include "wx/canvas/canwidget.h"
class a2dWidgetButton : public a2dWindowMM
{
    a2dWidgetButton( a2dCanvasObject* parent, int buttonId, double x, double y, double w, double h, a2dWidgetButton::WidgetMode mode = a2dWidgetButton::Boundingbox )
    a2dWidgetButton( const a2dWidgetButton& other, a2dObject::CloneOptions options, a2dRefMap* refs )
    virtual void SetParent( a2dCanvasObject* parent )
    void SetRoot(a2dDrawing* root)
    virtual wxEvtHandler* GetParentEvtHandler()
    void SetContentObject( a2dCanvasObject* content )
    a2dCanvasObject* GetContentObject()
    void SetContentBorder( double contentBorder )
    double GetContentBorder()
    void SetSelectedStroke( const a2dStroke& selectedStroke )
    void SetSelectedFill( const a2dFill& selectedFill )
    void SetHighLightStroke( const a2dStroke& highLightStroke )
    void SetHighLightFill( const a2dFill& highLightFill )
    int GetButtonId()
};

// *****************************************************************
// wx/canvas\drawer.h
// *****************************************************************

enum a2dCanViewUpdateFlags
{
    a2dCANVIEW_UPDATE_OLDNEW,
    a2dCANVIEW_UPDATE_PENDING,
    a2dCANVIEW_UPDATE_ALL,
    a2dCANVIEW_UPDATE_AREAS,
    a2dCANVIEW_UPDATE_AREAS_NOBLIT,
    a2dCANVIEW_UPDATE_BLIT,
    a2dCANVIEW_UPDATE_VIEWDEPENDENT,
    a2dCANVIEW_UPDATE_VIEWDEPENDENT_RIGHTNOW,
    a2dCANVIEW_UPDATE_SYNC_DRAWERS
};

// ---------------------------------------------------------------------------
// a2dLayerView
// ---------------------------------------------------------------------------

#include "wx/canvas/drawer.h"
class a2dLayerView
{
    a2dLayerView()
    bool GetVisible()
    void SetVisible( bool status )
    bool GetAvailable()
    void SetAvailable( bool status )
    bool GetCheck()
    void SetCheck( bool status )
    bool DoRenderLayer()
};

#include "wx/canvas/drawer.h"
class a2dDrawingPart : public a2dObject
{
    a2dDrawingPart( int width = 1000, int height = 1000 )
    a2dDrawingPart( int width, int height, a2dDrawer2D *drawer2D )
    a2dDrawingPart( a2dDrawer2D * drawer )
    a2dDrawingPart( const wxSize& size )
    a2dDrawingPart( const a2dDrawingPart& other)
    virtual void SetDisplayWindow( wxWindow* display )
    a2dCanvas* GetCanvas() const
    void SetBufferSize( int w, int h )
    void SetViewDependentObjects( bool viewDependentObjects )
    bool GetViewDependentObjects() const
    bool Get_UpdateAvailableLayers() const
    void SetAvailable()
    void SetLayerCheck( wxUint16 layer )
    void Set_UpdateAvailableLayers( bool value )
    void SetReverseOrder(bool revorder)
    bool GetReverseOrder() const
    void SetHitMarginDevice( wxUint16 pixels )
    wxUint16 GetHitMarginDevice() const
    double GetHitMarginWorld() const
    virtual bool ProcessEvent(wxEvent& event)
    void SetMouseEvents(bool onoff)
    bool GetMouseEvents() const
    void SetCorridor( a2dCanvasObject* start,  a2dCanvasObject* end );
    a2dCanvasObject* GetEndCorridorObject() const
    a2dCanvasObject* SetShowObject(const wxString& name)
    bool SetShowObject(a2dCanvasObject* obj)
    a2dCanvasObject* GetShowObject() const
    void Update( unsigned int how = a2dCANVIEW_UPDATE_ALL )
    void UpdateArea( int x, int y, int width, int height, wxUint8 id )
    virtual void DrawOrigin()
    virtual void PaintGrid( int x, int y, int width, int height )
    virtual void PaintBackground( int x, int y, int width, int height )
    void SetCrossHair( bool onoff )
    bool GetCrossHair()
    void SetCrossHairLengthX( int LengthX )
    void SetCrossHairLengthY( int LengthY )
    int GetCrossHairLengthX()
    int GetCrossHairLengthY()
    void SetCrossHairStroke( const a2dStroke& stroke )
    a2dStroke& GetCrossHairStroke()
    virtual void UpdateCrossHair( int x, int y )
    void SetFixedStyleFill( const a2dFill& fixFill )
    void SetFixedStyleStroke( const a2dStroke& fixStroke )
    void SetSelectFill( const a2dFill& selectFill )
    void SetSelectStroke( const a2dStroke& selectStroke )
    void SetBackgroundFill( const a2dFill& backgroundfill )
    a2dFill& GetBackgroundFill()
    void SetGridAtFront(bool gridatfront)
    bool GetGridAtFront()
    void SetGridStroke( const a2dStroke& gridstroke)
    void SetGridSize(wxUint16 gridsize)
    void SetGridFill( const a2dFill& gridfill)
    double GetGridX()
    void SetGridX(double gridx)
    double GetGridY()
    void SetGridY(double gridy)
    void SetGrid(bool grid)
    bool GetGrid()
    void SetGridLines(bool gridlines)
    bool GetGridLines()
    void SetGridThreshold( wxUint16 gridthres )
    wxUint16 GetGridThreshold()
    void SetShowOrigin(bool show)
    bool IsFrozen()
    void Freeze()
    void Thaw( bool update )
    bool SetCanvasToolContr( a2dToolContr* controller )
    a2dToolContr* GetCanvasToolContr()
    void SetCursor(const wxCursor&cursor)
    void PushCursor(const wxCursor& cursor)
    void PopCursor()
    void ClearCursorStack()
    a2dCanvasObject* IsHitWorld( double x, double y, int layer = wxLAYER_ALL, a2dHitOption option = a2dCANOBJHITOPTION_NONE )
    virtual bool ProcessCanvasObjectEvent( wxEvent& event, bool& isHit,double x, double y, int margin,int layer = wxLAYER_ALL )
    virtual bool ProcessCanvasObjectEvent( a2dCanvasObjectEvent& event, bool& isHit,double x, double y, int margin,int layer = wxLAYER_ALL )
    bool FindAndSetCorridorPath( a2dCanvasObject* findObject )
    void SetCorridorPath( const a2dCorridor& corridor )
    void ClearCorridorPath(
    void SetMappingShowAll()
    void SetDrawer2D( a2dDrawer2D* drawer2d, bool noDelete = false )
    a2dDrawer2D* GetDrawer2D()
    void SetDocumentDrawStyle( wxUint16 drawstyle )
    void RestoreDrawStyle()
    wxUint16 GetDocumentDrawStyle()
    virtual void Scroll( int dxy, bool yscroll, bool  total)
    void UpdateViewDependentObjects()
    void SetPrintTitle( bool val )
    void SetPrintFilename( bool val )
    void SetPrintScaleLimit( double val )
    void SetPrintFrame( bool val )
    void SetPrintFitToPage( bool val )
    int GetMouseX()
    int GetMouseY()
    int GetWorldMouseX()
    int GetWorldMouseY()
    void MouseToToolWorld( int x, int y, double& xWorldLocal, double& yWorldLocal )
    %member static a2dPropertyIdUint16* PROPID_drawstyle
    %member static a2dPropertyIdBool* PROPID_gridlines
    %member static a2dPropertyIdBool* PROPID_grid
    %member static a2dPropertyIdBool* PROPID_showorigin
    %member static a2dPropertyIdUint16* PROPID_hitmargin
    %member static a2dPropertyIdBool* PROPID_gridatfront
    %member static a2dPropertyIdUint16* PROPID_gridsize
    %member static a2dPropertyIdUint16* PROPID_gridthres
    %member static a2dPropertyIdDouble* PROPID_gridx
    %member static a2dPropertyIdDouble* PROPID_gridy
    %member const a2dSignal sig_changedLayers
    %member const a2dSignal sig_changedLayerAvailable
    %member const a2dSignal sig_changedLayerVisibleInView
    %member const a2dSignal sig_changedShowObject
};

// *****************************************************************
// wx/canvas\hittest.h
// *****************************************************************

%function a2dHit HitTestRectangle( double xtest, double ytest, double xmin, double ymin, double xmax, double ymax, double margin )

// *****************************************************************
// wx/canvas\layerinf.h
// *****************************************************************

typedef a2dSmrtPtr<a2dLayerInfo> a2dLayerInfoPtr 

class a2dSpan
{
    a2dSpan()
    a2dSpan( const wxString& spanString )
    bool IsValid()
    wxString GetFrom() const
    wxString GetTo() const
    wxString GetAsString() const
};

// ---------------------------------------------------------------------------
// a2dLayerInfo
// ---------------------------------------------------------------------------

#include "wx/canvas/layerinf.h"
class a2dLayerInfo : public a2dCanvasObject
{
    a2dLayerInfo()
    a2dLayerInfo( wxUint16 index, wxString name )
    a2dLayerInfo( const a2dLayerInfo& other, a2dObject::CloneOptions options, a2dRefMap* refs )
    virtual wxString GetName() const
    virtual void SetName(const wxString& name)
    bool GetVisible()
    bool GetSelectable()
    int GetOrder() const
    bool GetRead()
    int GetInMapping()
    int GetOutMapping()
    bool GetPixelStroke()
    bool GetAvailable()
    void SetAvailable( bool status )
    a2dPinClass *GetPinClass() const
    void SetPinClass( a2dPinClass* pinClass )
    void SetVisible( bool status )
    void SetSelectable( bool status )
    void SetOrder( wxUint16 order )
    void SetRead( bool status )
    void SetInMapping( wxUint16 layer )
    void SetOutMapping( wxUint16 layer )
    void SetPixelStroke( bool pixel)
    const wxString& GetFeature() const
    void SetFeature( const wxString& feature )
    const wxString& GetSubFeature() const
    void SetSubFeature( const wxString& subfeature )
    const wxString& GetType() const
    void SetType( const wxString& type )
    const wxString& GetSide() const
    void SetSide( const wxString& type )
    a2dSpan GetSpan() const
    void SetSpan( a2dSpan span )
    void SetSpan( const wxString& spanString )
    wxString& GetDescription()
    void SetDescription( const wxString& description )
    double GetArea()
    void SetArea( double area )
    bool GetPolarity()
    void SetPolarity( bool polarity )
    bool GetPlated()
    void SetPlated( bool plated )
    %member static a2dPropertyIdString* PROPID_layerName
    %member static a2dPropertyIdBool* PROPID_layerSelectable
    %member static a2dPropertyIdBool* PROPID_layerVisible
    %member static a2dPropertyIdBool* PROPID_readlayer
    %member static a2dPropertyIdUint16* PROPID_order
    %member static a2dPropertyIdUint16* PROPID_inmap
    %member static a2dPropertyIdUint16* PROPID_outmap
    %member const a2dSignal sig_changedLayerInfo
};

// ---------------------------------------------------------------------------
// a2dLayers
// ---------------------------------------------------------------------------

#include "wx/canvas/layerinf.h"

enum a2dLayers::Index
{
    OnLayer,
    OnOrder,
    OnReverseOrder
}; 

class a2dLayers : public a2dCanvasObject
{
    a2dLayers()
    a2dLayers( const a2dLayers& other, a2dObject::CloneOptions options, a2dRefMap* refs )
    void SetFileName( const wxFileName& filename )
    wxFileName& GetFileName()
    void SetPending(bool pending)
    void AddIfMissing( int layer )
    wxUint16 AddIfMissing( const wxString& layerName,  )
    a2dLayerInfo* operator[] (int indx)
    const a2dLayerInfo* operator[] (int indx) const
    a2dLayerInfo* GetLayerInfo( int index )
    a2dLayerMapNr& GetLayerIndex()
    a2dLayerIndex& GetLayerSort()
    a2dLayerIndex& GetOrderSort()
    a2dLayerIndex& GetReverseOrderSort()
    bool LoadLayers( const wxString& filename )
    bool SaveLayers( const wxString& filename )
    void InitWith( int highlayer, bool blackWhite )
    wxString GetName( int layernumber )
    int   GetNumber( wxString name, bool createIfMissing = false )
    a2dStroke GetStroke( int layernumber )
    bool  GetVisible( int layernumber )
    bool  GetSelectable( int layernumber )
    a2dFill GetFill( int layernumber )
    int   GetOrder( int layernumber )
    bool  GetRead( int layernumber )
    int   GetInMapping( int layernumber )
    int   GetOutMapping( int layernumber )
    bool  GetPixelStroke( int layernumber )
    void SetName( int layernumber, wxString name )
    void SetStroke( int layernumber, const a2dStroke& stroke )
    void SetVisible( int layernumber, bool = true )
    void SetAvailable( int layernumber, bool = true )
    void SetSelectable( int layernumber, bool = true )
    void SetFill( int layernumber, const a2dFill& fill )
    void SetOrder( int layernumber, int status )
    void SetRead( int layernumber, bool = true )
    void SetInMapping( int layernumber, wxUint16 layer )
    void SetOutMapping( int layernumber, wxUint16 layer )
    void SetPattern( int layernumber, const wxBitmap& stipple )
    void SetPixelStroke( int layernumber, bool = true )
    void SetAll_Layers_Visible( bool onoff )
    void SetAll_Layers_Read( bool onoff )
    void SetAll_Layers_Selectable( bool onoff )
    void SetAll_Layers_Outline( bool onoff )
    void SetAllLayersAvailable( bool onoff )
    void SetAvailable( a2dDrawing* drawing )
    void SetAvailable( a2dLayers* other )
    void UpdateIndexes()
    %member static a2dPropertyIdBool* PROPID_visibleAll
    %member static a2dPropertyIdBool* PROPID_readAll
    %member static a2dPropertyIdBool* PROPID_selectableAll
};

// *****************************************************************
// wx/canvas\objlist.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dCorridor
// ---------------------------------------------------------------------------

#include "wx/canvas/objlist.h"
class a2dCorridor : public a2dCanvasObjectList
{
    a2dCorridor()
    a2dCorridor( const a2dIterC& context )
    a2dCorridor( const a2dDrawingPart& drawingPart )
    void Push( a2dCanvasObject* object )
    const a2dAffineMatrix& GetTransform() const
    const a2dAffineMatrix& GetInverseTransform() const
};

// *****************************************************************
// wx/canvas\polygon.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dPolygonLClipper2
// ---------------------------------------------------------------------------

#include "wx/canvas/polygon.h"
class a2dPolygonLClipper2 : public a2dPolygonL
{
    a2dPolygonLClipper2( a2dBoundingBox& bbox = wxNonValidBbox )
    a2dPolygonLClipper2( a2dVertexList* segments, bool spline = false )
    a2dPolygonLClipper2( const a2dPolygonLClipper2 &other, a2dObject::CloneOptions options, a2dRefMap* refs )
    void Render( a2dIterC& ic, OVERLAP clipparent )
};

// ---------------------------------------------------------------------------
// a2dPolygonLClipper
// ---------------------------------------------------------------------------

#include "wx/canvas/polygon.h"
class a2dPolygonLClipper : public a2dPolygonL
{
    a2dPolygonLClipper( a2dBoundingBox& bbox = wxNonValidBbox )
    a2dPolygonLClipper( a2dVertexList* points, bool spline = false )
    a2dPolygonLClipper( const a2dPolygonLClipper &other, a2dObject::CloneOptions options, a2dRefMap* refs )
    void SetClippingTransformMatrix(const a2dAffineMatrix& mat = a2dIDENTITY_MATRIX )
    void SetClippingFromBox( a2dBoundingBox& bbox = wxNonValidBbox )
    void Render( a2dIterC& ic, OVERLAP clipparent )
};

// ---------------------------------------------------------------------------
// a2dPolylineL
// ---------------------------------------------------------------------------

#include "wx/canvas/polygon.h"
class a2dPolylineL : public a2dPolygonL
{
    a2dPolylineL()
    a2dPolylineL(a2dVertexList* segments, bool spline = false  )
    a2dPolylineL( const a2dPolylineL& poly, a2dObject::CloneOptions options, a2dRefMap* refs )
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
    a2dHit PointOnPolyline(const a2dPoint2D& P, double marge)
    void SetBegin(a2dCanvasObject* begin)
    a2dCanvasObject* GetBegin()
    void SetEnd(a2dCanvasObject* end)
    a2dCanvasObject* GetEnd()
    void SetEndScaleX(double xs)
    double GetEndScaleX()
    void SetEndScaleY(double ys)
    double GetEndScaleY()
    void SetPathType( a2dPATH_END_TYPE pathtype )
    a2dPATH_END_TYPE GetPathType()
    bool FindNearPoint( const a2dAffineMatrix *cworld, double xIn, double yIn, double *xOut, double *yOut )
    bool FindNearPointOrtho( const a2dAffineMatrix *cworld, double xIn, double yIn, double *xOut, double *yOut )
    bool MoveDynamicPinCloseTo( a2dPin *pin, const a2dPoint2D &point, bool final )
    virtual bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y )
    a2dPin *FindBeginPin()
    a2dPin *FindEndPin()
    a2dPin *FindPin( int *i )
    a2dLineSegment* GetFirstPoint() const;
    a2dLineSegment* GetLastPoint() const;
    int FindSegmentIndex( const a2dPoint2D& point, double margin = a2dACCUR );
    int FindPinSegmentIndex( a2dPin* pinToFind, double margin = a2dACCUR );
	
};

// ---------------------------------------------------------------------------
// a2dPolygonL
// ---------------------------------------------------------------------------

#include "wx/canvas/polygon.h"
class a2dPolygonL : public a2dCanvasObject
{
    a2dPolygonL()
    a2dPolygonL(a2dVertexList* points, bool spline = false  )
    a2dPolygonL( const a2dPolygonL& poly, a2dObject::CloneOptions options, a2dRefMap* refs )
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
    bool RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld  )
    a2dLineSegment* AddPoint(const a2dPoint2D& P, int index = -1, bool afterinversion = true )
    a2dLineSegment* AddPoint( double x, double y, int index = -1, bool afterinversion = true )
    void RemoveRedundantPoints( a2dCanvasObject *sendCommandsTo = NULL)
    void RemoveSegment( double& x, double& y , int index = -1, bool transformed = true  )
    void Clear()
    bool EliminateMatrix()
    void SetPosXYSegment(int index, double x, double y, bool afterinversion = true )
    void SetPosXYMidSegment(int index, double x, double y, bool afterinversion = true )
    void GetPosXYSegment(int index, double& x, double& y, bool transform = true ) const
    void GetPosXYMidSegment(int index, double& x, double& y, bool transform = true ) const
    virtual bool AdjustAfterChange( bool final )
    a2dVertexList* GetSegments()
    void SetSegments( a2dVertexList* points )
    size_t GetNumberOfSegments()
    void SetSpline(bool on)
    bool GetSpline()
    void SetContourWidth(double width)
    double GetContourWidth() const
    //void SetFillRule( short int val )
    //short int GetFillRule()
    a2dHit PointInPolygon(const a2dPoint2D& P, double marge)
};

// *****************************************************************
// wx/canvas\rectangle.h
// *****************************************************************

enum a2dWindowMM::a2dWindowMMState
{
    NON,
    DISABLED,
    SELECTED,
    FOCUS,
    HOVER,
    ALL
};

enum a2dWindowMM::a2dWindowMMStyle
{
    SUNKEN,
    RAISED
};

// ---------------------------------------------------------------------------
// a2dRectMM
// ---------------------------------------------------------------------------

#include "wx/canvas/rectangle.h"
class a2dRectMM : public a2dCanvasObject
{
    a2dRectMM()
    a2dRectMM( double x, double y, double w, double h , double radius=0, double contourwidth = 0 )
    a2dRectMM( const a2dPoint2D& p1,  const a2dPoint2D& p2, double radius=0, double contourwidth = 0 )
    a2dRectMM( const a2dBoundingBox& bbox, double radius=0, double contourwidth = 0 )
    a2dRectMM( const a2dRectMM& rec, a2dObject::CloneOptions options, a2dRefMap* refs )
    void SetMin( double minx, double miny )
    void SetMax( double maxx, double maxy )
    void    SetWidth( double w)
    void    SetHeight( double h)
    double  GetWidth() const
    double  GetHeight() const
    void SetBorder(double width)
    double GetBorder() const
    void SetContourWidth(double width)
    double GetContourWidth() const
    void    SetRadius( double radius )
    double  GetRadius() const
    a2dVertexList* GetAsVertexList( bool& returnIsPolygon )
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
    bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y  )
    void OnHandleEvent(a2dHandleMouseEvent &event)
};

// *****************************************************************
// wx/canvas\recur.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dCanvasObjectArrayReference
// ---------------------------------------------------------------------------

#include "wx/canvas/recur.h"
class a2dCanvasObjectArrayReference : public a2dCanvasObjectReference
{
    a2dCanvasObjectArrayReference()
    a2dCanvasObjectArrayReference( double x, double y, int rows, int columns, double horzSpace, double vertSpace, a2dCanvasObject* obj )
    a2dCanvasObjectArrayReference( const a2dCanvasObjectArrayReference &other, a2dObject::CloneOptions options, a2dRefMap* refs )
    void SetHorzSpace( double horzSpace )
    double GetHorzSpace()
    void SetVertSpace( double vertSpace)
    double GetVertSpace()
    int	GetRows()
    void SetRows( int rows )
    int	GetColumns()
    void SetColumns( int columns )
    double  GetWidth()
    double  GetHeight()
    void RemoveHierarchy()
};

// ---------------------------------------------------------------------------
// a2dCanvasObjectReference
// ---------------------------------------------------------------------------

#include "wx/canvas/recur.h"
class a2dCanvasObjectReference : public a2dCanvasObject
{
    a2dCanvasObjectReference()
    a2dCanvasObjectReference(double x, double y, a2dCanvasObject* obj)
    a2dCanvasObjectReference( const a2dCanvasObjectReference &other, a2dObject::CloneOptions options, a2dRefMap* refs )
    a2dCanvasObject* GetCanvasObject()
    void SetCanvasObject( a2dCanvasObject* object )
    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
    virtual bool LinkReference( a2dObject *other )
    void MakeReferencesUnique()
    bool IsRecursive()
    void RemoveHierarchy()
};

// *****************************************************************
// wx/canvas\restrict.h
// *****************************************************************

typedef wxUint32 a2dSnapToWhatMask
typedef a2dSmrtPtr<a2dRestrictionEngine> a2dRestrictionEnginePtr 

enum a2dRestrictionEngineOld::ESnapWhat
{
    snapPosX,
    snapPosY,
    snapSizeX,
    snapSizeY,
    snapRot,
    snapSkew,
    snapWH,
    snapEndAngle,
    snapStart,
    snapEnd,
    snapPointI,
    snapPos,
    snapSize,
    snapPosSize,
    snapSizeWH
};

enum a2dRestrictionEngineOld::EPositionSnapModes
{
    posCenter,
    posTopLeft,
    posTop,
    posTopRight,
    posRight,
    posBottomRight,
    posBottom,
    posBottomLeft,
    posLeft,
    posLineTop,
    posLineHCenter,
    posLineBottom,
    posLineLeft,
    posLineVCenter,
    posLineRight,
    posOther,
    posSngl,
    posEqual,
    posNone,
    pos6L,
    pos1P,
    pos2P,
    pos3P,
    pos5P,
    pos9P,
    posAny,
    pos1P6L
};

enum a2dRestrictionEngineOld::ERotationSnapModes
{
    rotVectorAngleX,
    rotVectorRationalX,
    rotVectorAngleY,
    rotVectorRationalY,
    rotPureRotations,
    rotPureSlanting,
    rotEndpointAngle,
    rotEndpointRational,
    rotEndpointUntrans,
    rotNone,
    rotAllAngle,
    rotAllRational,
    rotAll
};

enum a2dRestrictionEngine::a2dSnapToWhat
{
    snapToNone,
    snapToObjectPosX,
    snapToObjectPosY,
    snapToObjectPos,
    snapToStart,
    snapToEnd,
    snapToPins,
    snapToPinsUnconnected,
    snapToObjectIntersection,
    snapToObjectVertexes,
    snapToObjectSegmentsMiddle,
    snapToObjectSegments,
    snapToGridPosX,
    snapToGridPosY,
    snapToGridPos,
    snapToPointPosX,
    snapToPointPosY,
    snapToPointPosXorY,
    snapToPointPosXorYForce,
    snapToPointPos,
    snapToPointAngleMod,
    snapToPointAngleRational,
    snapToPointAngle,
    snapToBoundingBox,
    snapToObjectSnapVPath,
    snapToGridPosForced,
    snapToAll
};

// ---------------------------------------------------------------------------
// a2dRestrictionEngine
// ---------------------------------------------------------------------------

#include "wx/canvas/restrict.h"
class a2dRestrictionEngine : public a2dObject
{
    a2dRestrictionEngine()
    void SetSnapGrid( const a2dDoMu& x,  const a2dDoMu& y)
    const a2dDoMu& GetSnapGridX() const
    const a2dDoMu& GetSnapGridY() const
    void SetSnapOrigin( const a2dDoMu& x, const a2dDoMu& y)
    void SetSnapOriginX( const a2dDoMu& x)
    void SetSnapOriginY( const a2dDoMu& y)
    const a2dDoMu& GetSnapOriginX() const
    const a2dDoMu& GetSnapOriginY() const
    void SetRotationAngle(double a)
    double GetRotationAngle() const
    int GetSnapThresHold() const
    void SetSnapThresHold( int thresHold )
    double GetSnapThresHoldWorld() const
    void SetSnap(bool snap)
    bool GetSnap() const
    void SetSnapOnlyVisibleObjects( bool snapOnlyVisbleObjects )
    bool GetSnapOnlyVisibleObjects() const
    void SetRotationRational(wxUint32 nomMask, wxUint32 denMask)
    void SetSnapSourceFeatures( wxUint32 snapSourceFeatures )
    void SetSnapSourceFeature( a2dRestrictionEngine::a2dSnapToWhat snapSourceFeature, bool value = true )
    wxUint32 GetSnapSourceFeatures() const
    void SetSnapTargetFeatures( wxUint32 snapToFeatures )
    void SetSnapTargetFeature( a2dRestrictionEngine::a2dSnapToWhat snapToFeature, bool value = true )
    bool GetSnapTargetFeature( a2dRestrictionEngine::a2dSnapToWhat snapToFeature ) const
    wxUint32 GetSnapTargetFeatures() const
    virtual bool RestrictCanvasObjectAtVertexes( a2dCanvasObject* object, a2dPoint2D& point, double& dx, double& dy, wxUint32 sourceRequired = a2dRestrictionEngine::snapToAll, bool ignoreEngine = false )
    virtual bool RestrictVertexes( a2dVertexArray* segments, a2dPoint2D& point, double& dx, double& dy, wxUint32 sourceRequired = a2dRestrictionEngine::snapToAll, bool ignoreEngine = false )
    virtual bool RestrictVertexes( a2dVertexList* lsegments, a2dPoint2D& point, double& dx, double& dy, wxUint32 sourceRequired = a2dRestrictionEngine::snapToAll, bool ignoreEngine = false )
    virtual bool RestrictPoint(double& x, double& y,  wxUint32 sourceRequired = a2dRestrictionEngine::snapToAll, bool ignoreEngine = false )
    virtual bool RestrictLine( a2dLine& line,  wxUint32 sourceRequired = a2dRestrictionEngine::snapToAll, bool ignoreEngine = false )
    virtual bool RestrictAngle( double *ang,  wxUint32 sourceRequired = a2dRestrictionEngine::snapToAll, bool ignoreEngine = false )
    void SetRestrictPoint( double xSnap, double ySnap )
    const a2dPoint2D& GetRestrictPoint() const
    a2dPoint2D GetPointToSnap() const
    void SetPointSnapResult( const a2dPoint2D& p )
    a2dLine& GetLineToSnap()
    void SetParentSnapObjects(a2dCanvasObject* obj)
    a2dCanvasObject* GetParentSnapObjects() const
    void SetShiftKeyDown(bool shiftDown)
    bool GetShiftKeyDown() const
    void SetReleaseSnap( bool releaseSnap )
    bool GetReleaseSnap() const
    %member static a2dPropertyIdUint32* PROPID_SnapSourceFeaturesMem
    %member static a2dPropertyIdUint32* PROPID_SnapSourceFeatures
    %member static a2dPropertyIdUint32* PROPID_SnapTargetFeatures
    %member static a2dPropertyIdDouble* PROPID_RotationAngle
    %member static a2dPropertyIdUint32* PROPID_RotationRationalNom
    %member static a2dPropertyIdUint32* PROPID_RotationRationalDen
    %member static a2dPropertyIdInt32 * PROPID_SnapThresHold
    %member static a2dPropertyIdPoint2D* PROPID_PointToSnap
    %member static a2dPropertyIdPoint2D* PROPID_PointToRestrictTo
    %member static a2dPropertyIdBool* PROPID_SnapOnlyVisbleObjects
    %member static a2dPropertyIdBool* PROPID_Snap
    %member static a2dPropertyIdBool* PROPID_SnapGetSet
    %member static a2dPropertyIdBool* PROPID_AltDown
    %member static a2dPropertyIdBool* PROPID_ShiftDown
};

// ---------------------------------------------------------------------------
// a2dBaseTool
// ---------------------------------------------------------------------------

#include "wx/canvas/tools.h"
class a2dBaseTool : public a2dObject
{
    %member const a2dSignal sig_toolPushed
    %member const a2dSignal sig_toolPoped
    %member const a2dSignal sig_toolBeforePush
    %member const a2dSignal sig_toolDoPopBeforePush
    %member const a2dSignal sig_toolComEvent
    bool ProcessEvent( wxEvent& event )
    void SetEvtHandler( a2dObject* handler )
    a2dObject* GetEventHandler()
    a2dBaseTool(a2dToolContr* controller)
    a2dBaseTool( const a2dBaseTool& other, a2dObject::CloneOptions options, a2dRefMap* refs  )
    virtual bool StartTool( a2dBaseTool* currenttool )
    void SetOneShot()
    virtual bool ZoomSave()=0
    virtual bool AllowPop()
    void StopTool( bool abort = false )
    bool GetStopTool()
    bool GetActive()
    virtual void SetActive( bool active = true )
    bool GetBusy()
    virtual bool EnterBusyMode()
    virtual void FinishBusyMode( bool closeCommandGroup = true )
    virtual void AbortBusyMode()
    void SetFill( const a2dFill& fill)
    const a2dFill& GetFill() const
    void SetStroke( const a2dStroke& stroke)
    const a2dStroke& GetStroke() const
    void SetLayer(wxUint16 layer)
    wxUint16 GetLayer()
    void SetShowAnotation( bool show )
    void SetAnotationFont( const wxFont& font )
    void SetCursorType(const wxCursor& cursor)
    void SetBusyCursorType(const wxCursor& cursor)
    wxCursor GetCursorType()
    wxCursor GetBusyCursorType()
    virtual void SetMode( int mode )
    int GetMode()
    void SetPending( bool pending = true )
    bool GetPending()
    virtual void Render()
    a2dToolContr *GetToolController()
    a2dDrawer2D *GetDrawer2D()
    wxWindow* GetDisplayWindow()
    a2dCanvasCommandProcessor *GetCanvasCommandProcessor()
    virtual void OpenCommandGroup( bool restart )
    virtual void OpenCommandGroupNamed( const wxString &name )
    virtual void CloseCommandGroup()
    virtual wxString GetCommandGroupName()
    // a2dCommandGroup* GetCommandgroup()
    wxMenu* GetMousePopupMenu()
    void SetMousePopupMenu( wxMenu* mousemenu )
    a2dCanvasObject* GetParentObject()
    void AddEditobject( a2dCanvasObject *object )
    void RemoveEditobject( a2dCanvasObject *object )
    void AddDecorationObject( a2dCanvasObject *object )
    void RemoveAllDecorations()
    void SetIgnorePendingObjects( bool onoff )
    bool GetIgnorePendingObjects()
    a2dCorridor& GetCorridor()
    void SetCorridor( const a2dCorridor& corridor )
    void SetContourWidth(double width)
    double GetContourWidth() const
    void ResetContext()
    %member static a2dPropertyIdBool* PROPID_Oneshot
    %member static a2dPropertyIdBool* PROPID_Stop
    %member static a2dPropertyIdFill* PROPID_Fill
    %member static a2dPropertyIdStroke* PROPID_Stroke
    %member static a2dPropertyIdUint16* PROPID_Layer
};

// ---------------------------------------------------------------------------
// a2dToolContr
// ---------------------------------------------------------------------------

#include "wx/canvas/tools.h"
class a2dToolContr : public a2dObject
{
    a2dToolContr(a2dDrawingPart* view)
    void StopAllTools()
    virtual void ReStart()
    void Disable()
    a2dBaseTool *GetFirstTool() const
    const a2dToolList& GetToolList() const
    bool ProcessEvent(wxEvent& event)
    virtual bool PushTool( a2dBaseTool *handler )
    virtual bool PopTool( a2dBaseToolPtr& poped, bool force = true )
    void AppendTool(a2dBaseTool *handler)
    bool EnableTool(const wxString& tool , bool disableothers)
    bool EnableTool(a2dBaseTool* tool , bool disableothers)
    bool DisableTool(const wxString& tool )
    bool DisableTool(a2dBaseTool* tool )
    a2dBaseTool* SearchTool(const wxString& tool )
    void ActivateTop( bool active )
    bool Activate(const wxString& tool, bool disableothers)
    virtual bool StartEditingObject( a2dCanvasObject* objectToEdit )
    virtual bool StartEditingObject( a2dCanvasObject* objectToEdit, a2dIterC& ic )
    bool SetCorridor( const a2dCorridor& corridor )
    virtual void Render()
    void SetSnap( bool doSnap )
    bool GetSnap()
};

// ---------------------------------------------------------------------------
// a2dToolList
// ---------------------------------------------------------------------------

#include "wx/canvas/tools.h"
class a2dToolList : public a2dSmrtPtrList
{
    a2dToolList()
};

// *****************************************************************
// wx/canvas\vpath.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dVectorPath
// ---------------------------------------------------------------------------

#include "wx/canvas/vpath.h"
class a2dVectorPath : public a2dCanvasObject
{
    a2dVectorPath()
    a2dVectorPath( a2dVpath* path )
    a2dVectorPath( const a2dVectorPath &other, a2dObject::CloneOptions options, a2dRefMap* refs )
    double Length()
    bool EliminateMatrix()
    void ConvertToLines()
    a2dCanvasObjectList* GetAsPolygons()
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true )
    bool RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld  )
    a2dVpath* GetSegments()
    size_t GetCount()
    void Add( a2dVpathSegment *item )
    void Alloc(size_t count)
    void Clear()
    bool IsEmpty() const
    a2dVpathSegmentPtr operator[](size_t index) const
    a2dVpathSegmentPtr Item(size_t index) const
    a2dVpathSegmentPtr Last() const
    void SetContourWidth(double width)
    double GetContourWidth() const
    void SetDataType( int type )
    int GetDataType()
    void SetPathType( a2dPATH_END_TYPE pathtype )
    a2dPATH_END_TYPE GetPathType()
    void DoRender( a2dIterC& ic, OVERLAP clipparent )
};

// ---------------------------------------------------------------------------
// a2dWireEnd
// ---------------------------------------------------------------------------

#include "wx/canvas/wire.h"
class a2dWireEnd : public a2dCanvasObject
{
    a2dWireEnd( double x = 0 , double y = 0)
    a2dWireEnd( const a2dWireEnd& obj, a2dObject::CloneOptions options, a2dRefMap* refs )
};

// ---------------------------------------------------------------------------
// a2dWirePolylineL
// ---------------------------------------------------------------------------

#include "wx/canvas/wire.h"
class a2dWirePolylineL : public a2dPolylineL
{
    a2dWirePolylineL()
    a2dWirePolylineL(a2dVertexList* points, bool spline = false  )
    a2dWirePolylineL( const a2dWirePolylineL& poly, a2dObject::CloneOptions options, a2dRefMap* refs )
    // void SetConnectionInfo(
    a2dPinClass* GetStartPinClass() const
    void SetStartPinClass( a2dPinClass* startPinClass )
    a2dPinClass* GetEndPinClass() const
    void SetEndPinClass( a2dPinClass* endPinClass )
    void SetRouteOneLine( bool oneLine )
    bool GetRouteOneLine()
    virtual bool IsConnect() const
    virtual bool NeedsUpdateWhenConnected() const
    virtual bool AdjustAfterChange( bool final )
    void SetEndPoint( int iEnd, int iNext, double x, double y, bool final )
    bool IsDislocated()
    void SetReroute( bool onOff )
    bool GetReroute() const
    void SetRerouteAdded( bool onOff )
    bool GetRerouteAdded() const
    void SetPriority( wxUint8 priority )
    wxUint8 GetPriority()
    
};

// *****************************************************************
// wx/canvas\xmlpars.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dIOHandlerCVGIn
// ---------------------------------------------------------------------------

#include "wx/canvas/xmlpars.h"
class a2dIOHandlerCVGIn : public a2dIOHandlerXmlSerIn
{
    virtual bool CanLoad( a2dDocumentInputStream& stream, const wxObject* obj )
    bool LoadLayers( a2dDocumentInputStream& stream, a2dLayers* layers )
    // bool Load( a2dDocumentStringInputStream& stream, a2dCanvasDocument* doc, a2dCanvasObject* parent )
};

// ---------------------------------------------------------------------------
// a2dIOHandlerCVGOut
// ---------------------------------------------------------------------------

#include "wx/canvas/xmlpars.h"
class a2dIOHandlerCVGOut : public a2dIOHandlerXmlSerOut
{
    a2dIOHandlerCVGOut()
    bool CanSave( const wxObject* obj = NULL )
    bool SaveStartAt( a2dDocumentOutputStream& stream, const a2dDrawing* doc, a2dCanvasObject* start )
    bool SaveLayers( a2dDocumentOutputStream& stream, a2dLayers* layers )
};


// ---------------------------------------------------------------------------
// a2dCommand_GroupAB
// ---------------------------------------------------------------------------
enum a2dCommand_GroupAB::a2dDoWhat
{
    DeleteGroupA,
    MoveGroupA,
    CopyGroupA,
    ConvertToArcs,
    ConvertToPolygonPolylinesWithArcs,
    ConvertToPolygonPolylinesWithoutArcs,
    ConvertPolygonToArcs,
    ConvertPolylineToArcs,
    ConvertToVPaths,
    ConvertLinesArcs,
    ConvertToPolylines,
    DetectSmall,
    RemoveRedundant,
    Boolean_OR,
    Boolean_AND,
    Boolean_EXOR,
    Boolean_A_SUB_B,
    Boolean_B_SUB_A,
    Boolean_CORRECTION,
    Boolean_SMOOTHEN,
    Boolean_MAKERING,
    Boolean_Polygon2Surface,
    Boolean_Surface2Polygon
};

#include "wx/editor/cancom.h"
class a2dCommand_GroupAB : public a2dCommand
{
    %member static const a2dCommandId Id
    
    // %override a2dCommand_GroupAB( a2dCanvasObject* obj, const Args& args = Args() )
    // C++ Func: a2dCommand_GroupAB( const Args& args = Args() ) 
    a2dCommand_GroupAB( LuaTable parTable )
    
};


// *****************************************************************
// wx/canvas/fontdlg.h
// *****************************************************************

#include "wx/canvas/fontdlg.h"

// ---------------------------------------------------------------------------
// a2dTextPropDlg
// ---------------------------------------------------------------------------

class a2dTextPropDlg : public wxDialog
{
    a2dTextPropDlg( wxWindow* parent, const a2dFont& currentfont, unsigned int textflags, int alignment );
    //a2dTextPropDlg( wxWindow* parent = NULL, const a2dFont& currentfont = *a2dDEFAULT_CANVASFONT, unsigned int textflags = a2dText::a2dCANVASTEXT_DEFAULTFLAGS, int alignment = a2dDEFAULT_ALIGNMENT );
    const a2dFont& GetFontData() const;
    void SetAlignment( int alignment );
    int GetAlignment() const;

    void SetTextFlags( unsigned int textflags ) ;
    unsigned int GetTextFlags() const;
};


