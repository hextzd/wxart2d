
// *****************************************************************
// wx/artbase\afmatrix.h
// *****************************************************************

class a2dPropertyIdMatrix  
{
};

class a2dPropertyIdPoint2D  
{
};

%function bool ParseCvgTransForm( a2dAffineMatrix& matrix, const wxString& str, wxString& error )

// ---------------------------------------------------------------------------
// a2dMatrixProperty
// ---------------------------------------------------------------------------

#include "wx/artbase/afmatrix.h"
class a2dMatrixProperty : public a2dNamedProperty
{
    a2dMatrixProperty()
    a2dMatrixProperty( const a2dPropertyIdMatrix* id, const a2dAffineMatrix& value = a2dIDENTITY_MATRIX )
    a2dMatrixProperty( const a2dMatrixProperty* ori )
    a2dMatrixProperty( const a2dMatrixProperty &other )
    a2dNamedProperty* Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    void SetValue( const a2dAffineMatrix& value )
    a2dAffineMatrix& GetValue()
    const a2dAffineMatrix& GetValue() const
    a2dAffineMatrix* GetValuePtr()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    static a2dMatrixProperty *CreatePropertyFromString( const a2dPropertyIdMatrix* id, const wxString &value )
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dPoint2DProperty
// ---------------------------------------------------------------------------

#include "wx/artbase/afmatrix.h"
class a2dPoint2DProperty : public a2dNamedProperty
{
    a2dPoint2DProperty()
    a2dPoint2DProperty( const a2dPropertyIdPoint2D* id, const a2dPoint2D& value, int index = 0, bool afterinversion = false )
    a2dPoint2DProperty( const a2dPoint2DProperty* ori )
    a2dPoint2DProperty( const a2dPoint2DProperty &other )
    a2dNamedProperty* Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dPoint2DProperty *CreatePropertyFromString( const a2dPropertyIdPoint2D* id, const wxString &value )
    void SetValue( double x, double y, int index = 0, bool afterinversion = false )
    void SetValue( const a2dPoint2D& P, int index = 0, bool afterinversion = false )
    a2dPoint2D& GetValue()
    const a2dPoint2D& GetValue() const
    a2dPoint2D* GetValuePtr()
    wxInt32 GetIndex()
    bool GetAfterinversion()
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dAffineMatrix
// ---------------------------------------------------------------------------

#include "wx/artbase/afmatrix.h"
class  a2dAffineMatrix
{
    a2dAffineMatrix( double xt, double yt, double scalex = 1, double scaley = 1, double degrees = 0 )
    a2dAffineMatrix()
    a2dAffineMatrix( const a2dAffineMatrix& mat )
    double GetValue( int col, int row ) const
    void SetValue( int col, int row, double value )
    %operator void operator = ( const a2dAffineMatrix& mat )
    %operator bool operator== ( const a2dAffineMatrix& a )
    %operator bool operator!= ( const a2dAffineMatrix& a )
    %operator a2dAffineMatrix& operator*=( const a2dAffineMatrix& m )
    %operator a2dAffineMatrix operator*( const a2dAffineMatrix& m ) const
    %operator a2dAffineMatrix operator-( const a2dAffineMatrix& a )
    static a2dAffineMatrix Mul( const a2dAffineMatrix &m1, const a2dAffineMatrix &m2 )
    //%operator double& operator()( int col, int row )
    //%operator double operator()( int col, int row ) const
    bool Invert()
    bool Identity()
    static const a2dAffineMatrix &GetIdentityMatrix()
    bool IsIdentity() const
    bool IsIdentity1() const
    bool IsTranslate() const
    bool Scale( double scale )
    bool RemoveScale()
    a2dAffineMatrix&  Scale( const double &xs, const double &ys, const double &xc, const double &yc )
    a2dAffineMatrix&  Mirror( bool y = true, bool x = false )
    bool Translate( double x, double y )
    bool Rotate( double angle )
    bool SkewX( double degrees )
    bool SkewY( double degrees )
    a2dAffineMatrix&  Rotate( const double &r, const double &x, const double &y )
    void TransformPoint( double x, double y, double& tx, double& ty ) const
    void TransformPoint( a2dPoint2D *point ) const
    void TransformPoint( const a2dPoint2D& src, a2dPoint2D *dest ) const
    void TransformVector( double dx, double dy, double& tdx, double& tdy ) const
    double TransformDistance( double distance ) const
    double Get_scaleX() const
    double Get_scaleY() const
    double GetRotation() const
    void   SetRotation( double rotation )
    a2dPoint2D GetTranslation() const
    double GetDeterminant() const
    double GetDeterminant1() const
    double GetDeterminant2() const
    double GetMaximum() const
    double GetMaximum1() const
    double GetMaximum2() const
    //void DebugDump( const wxChar *text, double w, double h ) const
    //%member double  m_matrix[3][2]
    %member bool    m_isIdentity
};


#define a2dACCUR 1e-6
//#define Round( x ) (int) floor( (x) + 0.5 )

%function double wxDegToRad( double deg )
%function double wxRadToDeg( double rad )
%function void Aberration( double aber, double angle, double radius, double& dphi, unsigned int& segments )
%function double wxSqr( double x )

// ---------------------------------------------------------------------------
// a2dArtBaseModule
// ---------------------------------------------------------------------------

#include "wx/artbase/artglob.h"
class a2dArtBaseModule : public wxModule
{
    a2dArtBaseModule()
    virtual bool OnInit()
    virtual void OnExit()
};

// ---------------------------------------------------------------------------
// a2dGlobal
// ---------------------------------------------------------------------------

#include "wx/artbase/artglob.h"
class a2dGlobal : public a2dGeneralGlobal
{
    #define_pointer a2dGlobals

    a2dGlobal()
    void Aberration( double angle, double radius, double& dphi, unsigned int& segments )
    static void SetDisplayAberration( double aber )
    static double GetDisplayAberration()
    static void SetRoundFactor( double roundFactor )
    static double GetRoundFactor()
    static void SetPrimitiveThreshold( wxUint16 pixels, bool asrect = true )
    void SetPolygonFillThreshold( wxUint16 pixels )
    wxUint16 GetPolygonFillThreshold()
    static wxUint16 GetPrimitiveThreshold()
    static bool GetThresholdDrawRectangle()
    a2dPathList& GetFontPathList()
    a2dPathList& GetImagePathList()
    a2dPathList& GetIconPathList()
};

// ---------------------------------------------------------------------------
// a2dDoMu
// ---------------------------------------------------------------------------

#include "wx/artbase/artglob.h"
class %delete a2dDoMu
{
    a2dDoMu()
    a2dDoMu( double number, double multi = 1, bool normalize = true)
    a2dDoMu( double number, const wxString& multi )
    a2dDoMu( const wxString& number, const wxString& multi )
    a2dDoMu( const wxString& numberMulti )
    bool Eval( const wxString& param )
    double GetValue() const
    double GetNumber() const
    double GetMultiplier() const
    wxString GetValueString() const
    wxString GetMultiplierString() const
    wxString GetNumberString() const
    static wxString GetMultiplierString( double multi )
    static bool GetMultiplierFromString( const wxString& mul, double& multi )
    %operator a2dDoMu& operator=( const a2dDoMu& )
    //%operator a2dDoMu& operator=( const wxChar* )
    //%operator operator double() const
    %operator a2dDoMu&   operator=( double )
    %operator int operator==( const a2dDoMu& ) const
    %operator int operator!=( const a2dDoMu& ) const
    static void SetAccuracy( int accur )
};

// *****************************************************************
// wx/artbase\artmod.h
// *****************************************************************

// *****************************************************************
// wx/artbase\bbox.h
// *****************************************************************

enum OVERLAP
{
    _IN,
    _ON,
    _OUT
};

// ---------------------------------------------------------------------------
// a2dBoundingBox
// ---------------------------------------------------------------------------

#include "wx/artbase/bbox.h"
class  a2dBoundingBox
{
    a2dBoundingBox()
    a2dBoundingBox( const a2dBoundingBox& box )
    a2dBoundingBox( const a2dPoint2D& point )
    a2dBoundingBox( double x1, double y1, double x2, double y2 )
    %operator a2dBoundingBox&  operator+( a2dBoundingBox& )
    //%operator const a2dBoundingBox& operator+=( const a2dBoundingBox& box )
    %operator a2dBoundingBox&  operator=(  const a2dBoundingBox& )
    bool And( a2dBoundingBox*, double Marge = 0 )
    void Enlarge( const double Marge )
    void EnlargeXY( const double MargeX, const double MargeY )
    void Shrink( const double Marge )
    void Expand( const a2dPoint2D& , const a2dPoint2D& )
    void Expand( const a2dPoint2D& )
    void Expand( double x, double y )
    void Expand( const a2dBoundingBox& bbox )
    OVERLAP Intersect( const a2dBoundingBox &, double Marge = 0 ) const
    bool LineIntersect( const a2dPoint2D& begin, const a2dPoint2D& end ) const
    bool PointInBox( const a2dPoint2D&, double Marge = 0 ) const
    bool PointInBox( double x, double y, double Marge = 0 )  const
    void Reset()
    const a2dBoundingBox& Translate( a2dPoint2D& )
    const a2dBoundingBox& Translate( double x, double y )
    void MapBbox( const a2dAffineMatrix& matrix )
    double  GetWidth() const
    double  GetHeight() const
    bool GetValid()  const
    void SetValid( bool )
    void SetBoundingBox( const a2dPoint2D& a_point )
    void SetMin( double px, double py )
    void SetMax( double px, double py )
    a2dPoint2D GetMin() const
    a2dPoint2D GetMax() const
    double GetMinX() const
    double GetMinY() const
    double GetMaxX() const
    double GetMaxY() const
    double GetSize()
    void SetMinX( double minx )
    void SetMinY( double miny )
    void SetMaxX( double maxx )
    void SetMaxY( double maxy )
};

// *****************************************************************
// wx/artbase\dcdrawer.h
// *****************************************************************

// ---------------------------------------------------------------------------
// a2dDcDrawer
// ---------------------------------------------------------------------------

#include "wx/artbase/dcdrawer.h"
class a2dDcDrawer : public a2dDrawer2D
{
    a2dDcDrawer( int width = 0, int height = 0 )
    a2dDcDrawer( const wxSize& size )
    void SetBufferSize( int w, int h )
    wxBitmap GetBuffer()
    wxBitmap GetSubBitmap( wxRect sub_rect )
    a2dDcDrawer( const a2dDcDrawer& other )
    a2dDcDrawer( const a2dDrawer2D& other )
    virtual void BlitBuffer( wxRect rect, const wxPoint& bufferpos )
    virtual void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos  )
    void ResetStyle()
    void SetDrawStyle( a2dDrawStyle drawstyle )
    virtual void BeginDraw()
    virtual void EndDraw()
    virtual wxDC* GetRenderDC()
    void  SetRenderDC( wxDC* dc ) const
    wxDC* GetDeviceDC() const
    void SetClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE )
    virtual void ExtendClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND )
    virtual void ExtendAndPushClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND )
    void PopClippingRegion()
    void SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height )
    void SetClippingRegion( double minx, double miny, double maxx, double maxy )
    void DestroyClippingRegion()
    void DrawImage( const wxImage& image, double x, double y, double width, double height )
    void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height )
    void DrawVpath( const a2dVpath* path )
    void DrawCircle( double xc, double yc, double radius )
    void DrawPoint( double xc, double yc )
};

// ---------------------------------------------------------------------------
// a2dMemDcDrawer
// ---------------------------------------------------------------------------

#include "wx/artbase/dcdrawer.h"
class a2dMemDcDrawer : public a2dDcDrawer
{
    a2dMemDcDrawer( int width = 0, int height = 0 )
    a2dMemDcDrawer( const wxSize& size )
    virtual void BeginDraw()
    virtual void EndDraw()
    wxBitmap GetBuffer()
    void SetBufferSize( int w, int h )
    wxBitmap GetSubBitmap( wxRect sub_rect )
    a2dMemDcDrawer( const a2dMemDcDrawer& other )
    virtual void BlitBuffer( wxRect rect, const wxPoint& bufferpos )
    virtual void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos )
    virtual void ShiftBuffer( int dxy, bool yshift )
};

// *****************************************************************
// wx/artbase\drawer2d.h
// *****************************************************************

//#define Round( x ) (int) floor( (x) + 0.5 )
#define WX_COLOUR_HAS_ALPHA 1

enum a2dDrawStyle
{
    a2dFILLED, 
    a2dWIREFRAME, 
    a2dWIREFRAME_ZERO_WIDTH, 
    a2dWIREFRAME_INVERT, 
    a2dWIREFRAME_INVERT_ZERO_WIDTH, 
    a2dFIX_STYLE, 
    a2dFIX_STYLE_INVERT 
};

enum a2dBooleanClip
{
    a2dCLIP_AND,  
    a2dCLIP_COPY, 
    a2dCLIP_DIFF, 
    a2dCLIP_OR,   
    a2dCLIP_XOR   
};

// ---------------------------------------------------------------------------
// a2dDrawer2D
// ---------------------------------------------------------------------------

#include "wx/artbase/drawer2d.h"
class a2dDrawer2D : public wxObject
{
    a2dDrawer2D( int width = 0, int height = 0 )
    a2dDrawer2D( const wxSize& size )
    a2dDrawer2D( const a2dDrawer2D& other )
    void SetDisplay( wxWindow* window )
    void SetPrimitiveThreshold( wxUint16 pixels, bool asrect = true )
    wxUint16 GetPrimitiveThreshold() const
    bool GetThresholdDrawRectangle() const
    void SetPolygonFillThreshold( wxUint16 pixels )
    wxUint16 GetPolygonFillThreshold()
    virtual wxDC* GetRenderDC() const
    virtual wxDC* GetDeviceDC() const
    void SetDisplayAberration( double aber )
    double GetDisplayAberration() const
    virtual wxBitmap GetBuffer() const = 0
    virtual void SetBufferSize( int w, int h ) = 0
    void BlitBuffer()
    void BlitBuffer( int x, int y, int width, int height, int xbuf = 0, int ybuf = 0  )
    virtual void BlitBuffer( wxRect rect, const wxPoint& bufferpos ) = 0
    virtual void BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos ) = 0
    virtual void ShiftBuffer( int dxy, bool yshift )
    virtual wxBitmap GetSubBitmap( wxRect sub_rect ) = 0
    bool GetYaxis() const
    virtual void SetYaxis( bool up )
    void SetMappingDeviceRect( int mapx, int mapy, int mapWidth, int mapHeight, bool remap = false )
    virtual void SetMappingWidthHeight( double vx1, double vy1, double width, double height )
    void SetMappingWidthHeight( const a2dBoundingBox& box )
    void SetMappingUpp( double vx1, double vy1, double xpp, double ypp )
    double GetUppX() const
    double GetUppY() const
    void SetMinX( double x )
    void SetMinY( double y )
    void SetUppX( double x )
    void SetUppY( double y )
    bool GetVirtualAreaSet()
    void StartRefreshDisplayDisable()
    void EndRefreshDisplayDisable()
    bool GetRefreshDisplayDisable()
    int GetWidth()
    int GetHeight()
    int GetMapWidth()
    int GetMapHeight()
    int GetMapX()
    int GetMapY()
    double GetVisibleMinX() const
    double GetVisibleMinY() const
    virtual double GetVisibleMaxX() const
    virtual double GetVisibleMaxY() const
    virtual double GetVisibleWidth() const
    virtual double GetVisibleHeight() const
    a2dBoundingBox GetVisibleBbox() const
    wxRect ToDevice( const a2dBoundingBox& bbox )
    a2dBoundingBox ToWorld( const wxRect& rect )
    double DeviceToWorldX( double x ) const
    double DeviceToWorldY( double y ) const
    double DeviceToWorldXRel( double x ) const
    double DeviceToWorldYRel( double y ) const
    int WorldToDeviceX( double x ) const
    int WorldToDeviceY( double y ) const
    int WorldToDeviceXRel( double x ) const
    int WorldToDeviceYRel( double y ) const
    double WorldToDeviceXRelNoRnd( double x ) const
    double WorldToDeviceYRelNoRnd( double y ) const
    const a2dAffineMatrix& GetMappingMatrix()
    virtual void SetTransform( const a2dAffineMatrix& userToWorld )
    const a2dAffineMatrix& GetTransform() const
    const a2dAffineMatrix& GetUserToDeviceTransform() const
    virtual void PushTransform()
    virtual void PushTransform( const a2dAffineMatrix& affine )
    virtual void PopTransform()
    void SetSplineAberration( double aber )
    virtual void ResetStyle()
    void SetDrawerStroke( const a2dStroke& stroke )
    a2dStroke GetDrawerStroke() const
    void SetDrawerFill( const a2dFill& fill )
    a2dFill GetDrawerFill() const
    void SetFont( const a2dFont& font )
    a2dFont GetFont()
    virtual void DrawVpath( const a2dVpath* path )
    virtual void DrawPolygon( a2dVertexArray* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE )
    virtual void DrawPolygon( const a2dVertexList* list, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE )
    virtual void DrawLines( a2dVertexArray* points, bool spline = false )
    virtual void DrawLines( const a2dVertexList* list, bool spline = false )
    virtual void DrawLine( double x1, double y1, double x2, double y2 )
    virtual void DrawArc( double x1, double y1, double x2, double y2, double xc, double yc, bool chord )
    virtual void DrawEllipticArc( double xc, double yc, double width, double height , double sa, double ea, bool chord )
    virtual void DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize = false )
    virtual void DrawCenterRoundedRectangle( double xc, double yc, double width, double height, double radius, bool pixelsize = false )
    virtual void DrawCircle( double x, double y, double radius )
    void DrawPoint( double xc, double yc )
    virtual void DrawEllipse( double x, double y, double width, double height )
    virtual void DrawImage( const wxImage& image, double x, double y, double width, double height ) = 0
    virtual void DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height ) = 0
    void DrawImage( const wxImage& image, double x = 0, double y = 0 )
    void SetRealScale( bool realScale )
    void DrawText( const wxString& text, double x, double y, int alignment = wxMINX | wxMAXY, bool Background = true )
    virtual void SetDrawStyle( a2dDrawStyle drawstyle ) = 0
    a2dDrawStyle GetDrawStyle() const
    void SetPrintMode( bool onOff )
    void ReStoreFixedStyle()
    void SetDisableDrawing( bool disableDrawing )
    bool GetDisableDrawing()
    virtual void SetClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE ) = 0
    virtual void ExtendClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND ) = 0
    virtual void ExtendAndPushClippingRegion( a2dVertexList* points, bool spline = false, wxPolygonFillMode fillStyle = wxODDEVEN_RULE, a2dBooleanClip clipoperation = a2dCLIP_AND ) = 0
    virtual void PopClippingRegion() = 0
    virtual void SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord maxx, wxCoord maxy ) = 0
    virtual void SetClippingRegion( double minx, double miny, double maxx, double maxy ) = 0
    virtual void DestroyClippingRegion() = 0
    void GetClippingBox( double& x, double& y, double& w, double& h ) const
    a2dBoundingBox& GetClippingBox()
    void GetClippingMinMax( double& xmin, double& ymin, double& xmax, double& ymax ) const
    wxRect& GetClippingBoxDev()
    void GetClippingBoxDev( int& x, int& y, int& w, int& h ) const
    void Init()
    virtual void BeginDraw() = 0
    virtual void EndDraw() = 0
    void SetOpacityFactor( wxUint8 OpacityFactor )
    wxUint8 GetOpacityFactor() const
    %member const a2dSignal sig_changedZoom
};

// ---------------------------------------------------------------------------
// a2dImageRGBA
// ---------------------------------------------------------------------------

#include "wx/artbase/drawer2d.h"
class a2dImageRGBA
{
    a2dImageRGBA()
    a2dImageRGBA( wxImage& image, unsigned char alpha = 255 )
    %operator a2dImageRGBA& operator=( const a2dImageRGBA& other )
    int GetWidth() const
    int GetHeight() const
    void SetAlpha( unsigned char alpha )
    //void SetAlpha( unsigned char* alphadata )
    unsigned char* GetData() const
};

// *****************************************************************
// wx/artbase\liner.h
// *****************************************************************

#define __WXLINER_H

enum OUTPRODUCT {R_IS_LEFT, R_IS_ON, R_IS_RIGHT};
enum R_PointStatus {R_LEFT_SIDE, R_RIGHT_SIDE, R_ON_AREA, R_IN_AREA};

// ---------------------------------------------------------------------------
// a2dLine
// ---------------------------------------------------------------------------

#include "wx/artbase/liner.h"
class a2dLine
{
    a2dLine( double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0 )
    a2dLine( const a2dPoint2D& a, const a2dPoint2D& b )
    a2dLine( const a2dLine& other )
    %operator a2dLine& operator = ( const a2dLine& other )
    a2dPoint2D GetBeginPoint() const
    a2dPoint2D GetEndPoint() const
    bool CheckIntersect( a2dLine& line, double Marge ) const
    int Intersect( a2dLine& line,  a2dPoint2D& bp , a2dPoint2D& ep , double Marge ) const
    bool Intersect( a2dLine& lijn, a2dPoint2D& crossing ) const
    R_PointStatus   PointOnLine( const a2dPoint2D& a_Point, double& Distance, double Marge ) const
    R_PointStatus   PointInLine( const a2dPoint2D& a_Point, double& Distance, double Marge ) const
    a2dPoint2D ProjectedPoint( const a2dPoint2D& p ) const
    double PointDistance( const a2dPoint2D& p, a2dPoint2D *nearest ) const
    double PointDistanceOrhto( const a2dPoint2D& p, a2dPoint2D *nearest ) const
    OUTPRODUCT OutProduct( const a2dLine& two, double accur ) const
    double Calculate_Y( double X ) const
    void Virtual_Point( a2dPoint2D& a_point, double distance ) const
    a2dLine* CreatePerpendicularLineAt( const a2dPoint2D& a_point )  const
    void CalculateLineParameters()
    void OffsetContour( const a2dLine& nextline, double factor, a2dPoint2D& offsetpoint ) const
    double GetLength() const
};

// *****************************************************************
// wx/artbase\polyver.h
// *****************************************************************

class a2dListOfa2dVertexList
{
};

enum a2dHit::Stroke1
{
    stroke1_none,
    stroke1_outside,
    stroke1_inside
};

enum a2dSegType
{
    a2dNORMAL_SEG,
    a2dLINK_SEG,
    a2dHOLE_SEG
};

enum a2dPATH_END_TYPE
{
    a2dPATH_END_SQAURE,
    a2dPATH_END_ROUND,
    a2dPATH_END_SQAURE_EXT
};

enum a2dPATHSEG_END
{
    a2dPATHSEG_END_OPEN,
    a2dPATHSEG_END_CLOSED,
    a2dPATHSEG_END_CLOSED_NOSTROKE
};

enum a2dPATHSEG
{
    a2dPATHSEG_MOVETO,
    a2dPATHSEG_LINETO,
    a2dPATHSEG_LINETO_NOSTROKE,
    a2dPATHSEG_QBCURVETO,
    a2dPATHSEG_QBCURVETO_NOSTROKE,
    a2dPATHSEG_CBCURVETO,
    a2dPATHSEG_CBCURVETO_NOSTROKE,
    a2dPATHSEG_ARCTO,
    a2dPATHSEG_ARCTO_NOSTROKE
};

enum a2dHit::Stroke2
{
    stroke2_none,
    stroke2_vertex,
    stroke2_edgehor,
    stroke2_edgevert,
    stroke2_edgeother
};

enum a2dHit::Hit
{
    hit_none,
    hit_stroke,
    hit_fill,
    hit_child,
    hit_member
};

// ---------------------------------------------------------------------------
// a2dVpathQBCurveSegment
// ---------------------------------------------------------------------------

#include "wx/artbase/polyver.h"
class a2dVpathQBCurveSegment : public a2dVpathSegment
{
    a2dVpathQBCurveSegment( double x1, double y1, double x2, double y2, a2dPATHSEG type = a2dPATHSEG_QBCURVETO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN  )
    a2dVpathQBCurveSegment(  a2dVpathSegmentPtr prev, double x1, double y1, a2dPATHSEG type = a2dPATHSEG_QBCURVETO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN )
    a2dVpathQBCurveSegment( const a2dVpathQBCurveSegment &other )
    virtual a2dVpathSegment* Clone()
    virtual double Length( a2dVpathSegmentPtr prev )
    void PositionAt( a2dVpathSegmentPtr prev, double t, double& xt, double& yt )
    %member double m_x2
    %member double m_y2
};

// ---------------------------------------------------------------------------
// a2dArcSegment
// ---------------------------------------------------------------------------

#include "wx/artbase/polyver.h"
class a2dArcSegment : public a2dLineSegment
{
    a2dArcSegment( double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0 )
    a2dArcSegment( const a2dLineSegment& prev, double xc, double yc, double angle )
    a2dArcSegment( const a2dLineSegment& prev, double xc, double yc, double x1, double y1, double x2, double y2 )
    a2dArcSegment( const a2dArcSegment &other )
    virtual a2dLineSegment* Clone()
    bool CalcR( const a2dLineSegment& prev, double& radius, double& center_x, double& center_y, double& beginrad, double& midrad, double& endrad, double& phit ) const
    virtual double Length( const a2dLineSegment& prev )
    void CalcMidPoint( const a2dLineSegment& prev, double center_x, double center_y, double radius, bool clockwise )
    a2dPoint2D GetMidPoint()
    void SetMidPoint( const a2dLineSegment& prev, double xm, double ym )
    double GetOx( const a2dLineSegment& prev )
    double GetOy( const a2dLineSegment& prev )
    a2dPoint2D GetOrigin( const a2dLineSegment& prev )
    a2dBoundingBox GetBbox( const a2dLineSegment& prev, const a2dAffineMatrix& lworld = a2dIDENTITY_MATRIX )
    %member double m_x2
    %member double m_y2
};

// ---------------------------------------------------------------------------
// a2dVpathSegment
// ---------------------------------------------------------------------------

#include "wx/artbase/polyver.h"
class a2dVpathSegment
{
    a2dVpathSegment( double x, double y, a2dPATHSEG type = a2dPATHSEG_LINETO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN )
    a2dVpathSegment( const a2dVpathSegment &other )
    virtual a2dVpathSegment* Clone()
    a2dPATHSEG GetType() const
    bool GetBin()  const
    void SetBin( bool bin )
    a2dPATHSEG_END GetClose() const
    void SetClose( a2dPATHSEG_END close )
    virtual double Length( a2dVpathSegmentPtr prev )
    %member double m_x1
    %member double m_y1
};

class %delete a2dVpathSegmentPtr
{
    a2dVpathSegment* Get() const
};

// ---------------------------------------------------------------------------
// a2dLineSegment
// ---------------------------------------------------------------------------

#include "wx/artbase/polyver.h"
class a2dLineSegment
{
    a2dLineSegment( double x = 0, double y = 0 )
    a2dLineSegment( const a2dPoint2D& point )
    a2dLineSegment( const a2dLineSegment &other )
    virtual a2dLineSegment* Clone()
    bool GetArc() const
    bool GetBin()  const
    void SetBin( bool bin )
    virtual double Length( const a2dLineSegment& prev )
    a2dPoint2D GetPoint()
    void SetSegType( a2dSegType type )
    a2dSegType GetSegType()
    virtual a2dBoundingBox GetBbox( const a2dLineSegment& prev, const a2dAffineMatrix& lworld = a2dIDENTITY_MATRIX ) const
    %member double m_x
    %member double m_y
};

class %delete a2dLineSegmentPtr
{
    a2dLineSegment* Get() const
};

// ---------------------------------------------------------------------------
// a2dVertexListIter
// ---------------------------------------------------------------------------
class a2dVertexListIter
{
    a2dVertexListIter()
    
    %operator %rename GetItem a2dLineSegmentPtr operator*() const
    
    //%operator a2dLineSegmentPtr operator->() const

    %operator a2dVertexListIter& operator=(const a2dVertexListIter& other)
    
    %operator bool operator ==( const a2dVertexListIter& iter ) const
    %operator bool operator !=( const a2dVertexListIter& iter ) const
    
    //%operator a2dVertexListIter& operator++()
    //%operator a2dVertexListIter& operator++( int )
   
    //%operator a2dVertexListIter& operator--()
    //%operator a2dVertexListIter& operator--(int)
    
};

// ---------------------------------------------------------------------------
// a2dVertexList
// ---------------------------------------------------------------------------

#include "wx/artbase/polyver.h"
class a2dVertexList : public a2dSmrtPtrList
{
    a2dVertexList()
    a2dVertexList( const a2dVertexList &other )
    %operator a2dVertexList& operator=( const a2dVertexList &other )
    %rename lbegin a2dVertexListIter begin()
    %rename lend   a2dVertexListIter end()
    bool empty()
    size_t size()
	a2dLineSegment* front()
	a2dLineSegment* back()
	void push_front( a2dLineSegment* obj )
	void pop_front()
	void push_back( a2dLineSegment* obj )
	void pop_back()
    bool HasArcs()
    a2dVertexListIter GetPreviousAround( a2dVertexListIter iter )
    a2dVertexListIter GetNextAround( a2dVertexListIter iter )
    double Length()
    void AddPoint( const a2dPoint2D& point, bool atEnd = true )
    void AddPoint( double x, double y, bool atEnd = true )
    int IndexOf( a2dLineSegment* object ) const
    void SetPointAdjustArcs( unsigned int n, double x, double y, bool polygon )
    void ConvertToLines()
    a2dBoundingBox GetBbox( const a2dAffineMatrix& lworld = a2dIDENTITY_MATRIX )
    void Transform( const a2dAffineMatrix& world )
    a2dVertexList* ConvertSplinedPolygon( double Aber )
    a2dVertexList* ConvertSplinedPolyline( double Aber )
    void ConvertIntoSplinedPolygon( double Aber )
    void ConvertIntoSplinedPolyline( double Aber )
    a2dVertexList* ConvertToContour( double distance, a2dPATH_END_TYPE pathtype, bool asPolygon = false )
    void Contour( double distance, a2dPATH_END_TYPE pathtype, bool asPolygon = false )
    void CreateArc( const a2dPoint2D& center, const a2dPoint2D& begin, const a2dPoint2D& end, double radius, bool clock, double aber, bool addAtFront )
    void CreateArc( const a2dPoint2D& center, const a2dLine& incoming,  const a2dPoint2D& end, double radius, double aber, bool addAtFront )
    void OffsetContour_rounded( const a2dLine& currentline, const a2dLine& nextline, double factor, bool addAtFront )
    bool RemoveRedundant( bool polygon )
	a2dVertexList* GetRedundant( bool polygon )
    a2dHit HitTestPolygon( const a2dPoint2D& ptest, double margin )
    a2dHit HitTestPolyline( const a2dPoint2D& ptest, double margin )
};

// ---------------------------------------------------------------------------
// a2dVpathArcSegment
// ---------------------------------------------------------------------------

#include "wx/artbase/polyver.h"
class a2dVpathArcSegment : public a2dVpathSegment
{
    a2dVpathArcSegment( double x1, double y1, double x2, double y2, a2dPATHSEG type = a2dPATHSEG_ARCTO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN )
    a2dVpathArcSegment( a2dVpathSegmentPtr prev, double xc, double yc, double angle, a2dPATHSEG type = a2dPATHSEG_ARCTO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN )
    a2dVpathArcSegment( a2dVpathSegmentPtr prev, double xc, double yc, double x1, double y1, double x2, double y2, a2dPATHSEG type = a2dPATHSEG_ARCTO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN )
    a2dVpathArcSegment( const a2dVpathArcSegment &other )
    virtual a2dVpathSegment* Clone()
    bool CalcR( a2dVpathSegmentPtr prev, double& radius, double& center_x, double& center_y, double& beginrad, double& midrad, double& endrad, double& phit )
    virtual double Length( a2dVpathSegmentPtr prev )
    %member double m_x2
    %member double m_y2
};

%function bool InArc( double angle, double start, double end, bool clockwise )
%function double ClclDistSqrPntPnt( const a2dPoint2D& a, const a2dPoint2D& b )
%function double ClclDistSqrPntLine( const a2dPoint2D& p, const a2dPoint2D& p1, const a2dPoint2D& p2 )

// ---------------------------------------------------------------------------
// a2dVpath
// ---------------------------------------------------------------------------

#include "wx/artbase/polyver.h"
class a2dVpath
{
    a2dVpath()
    a2dVpath( a2dVertexArray& vertexArray, bool moveToFirst = true , bool closeLast = false )
    a2dVpath( a2dVertexList& vertexList, bool moveToFirst = true , bool closeLast = false )
    %operator a2dVpath& operator=( const a2dVpath &other )
    double Length()
    void Add( a2dVpathSegment* seg )
    void Add( a2dVertexArray& vertexArray, bool moveToFirst = true , bool closeLast = false )
    void Add( a2dVertexList& vertexList, bool moveToFirst = true , bool closeLast = false )
    void MoveTo( double x, double y )
    void LineTo( double x, double y, bool withStroke = true )
    void QBCurveTo( double x1, double y1, double x2, double y2, bool withStroke = true )
    void QBCurveTo( double x1, double y1, bool withStroke = true )
    void CBCurveTo( double x1, double y1, double x2, double y2, double x3, double y3, bool withStroke = true )
    void CBCurveTo( double x1, double y1, double x3, double y3, bool withStroke = true )
    void ArcTo( double x1, double y1, double x2, double y2, bool withStroke = true )
    void ArcTo( double xc, double yc, double angle, bool withStroke = true )
    void ArcTo( double xc, double yc, double x1, double y1, double x2, double y2, bool withStroke = true )
    void Close( bool withStroke = true )
    void ConvertToLines( double aberration = 0 )
    void ConvertToPolygon( a2dListOfa2dVertexList& addTo, bool arc = true )
    void Contour( double distance, a2dPATH_END_TYPE pathtype )
    void Transform( const a2dAffineMatrix& world )
};

// ---------------------------------------------------------------------------
// a2dVpathCBCurveSegment
// ---------------------------------------------------------------------------

#include "wx/artbase/polyver.h"
class a2dVpathCBCurveSegment : public a2dVpathSegment
{
    a2dVpathCBCurveSegment( double x1, double y1, double x2, double y2, double x3, double y3, a2dPATHSEG type = a2dPATHSEG_CBCURVETO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN )
    a2dVpathCBCurveSegment( a2dVpathSegmentPtr prev, double x1, double y1, double x3, double y3, a2dPATHSEG type = a2dPATHSEG_CBCURVETO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN )
    a2dVpathCBCurveSegment( const a2dVpathCBCurveSegment &other )
    virtual a2dVpathSegment* Clone()
    virtual double Length( a2dVpathSegmentPtr prev )
    void PositionAt( a2dVpathSegmentPtr prev, double t, double& xt, double& yt )
    %member double m_x2
    %member double m_y2
    %member double m_x3
    %member double m_y3
};

// ---------------------------------------------------------------------------
// a2dVertexArray
// ---------------------------------------------------------------------------

#include "wx/artbase/polyver.h"
class a2dVertexArray : public a2dVertexArrayBase
{
    a2dVertexArray()
    a2dVertexArray( const a2dVertexArray &other )
    %operator a2dVertexArray& operator=( const a2dVertexArray &other )
    a2dLineSegmentPtr GetPreviousAround( wxUint32 index )
    a2dLineSegmentPtr GetNextAround( wxUint32 index )
    double Length()
    void AddPoint( const a2dPoint2D& point, bool atEnd = true )
    void AddPoint( double x, double y, bool atEnd = true )
    void SetPointAdjustArcs( unsigned int n, double x, double y, bool polygon )
    void ConvertToLines()
    void Transform( const a2dAffineMatrix& world )
    bool HasArcs()
    a2dBoundingBox GetBbox( const a2dAffineMatrix& lworld = a2dIDENTITY_MATRIX )
    a2dVertexArray* Contour( double distance, a2dPATH_END_TYPE pathtype )
    a2dVertexArray* ConvertSplinedPolygon( double Aber )
    a2dVertexArray* ConvertSplinedPolyline( double Aber )
    a2dHit HitTestPolygon( const a2dPoint2D& ptest, double margin )
    a2dHit HitTestPolyline( const a2dPoint2D& ptest, double margin )
    bool RemoveRedundant( bool polygon )
	a2dVertexList* GetRedundant( bool polygon )
};

// ---------------------------------------------------------------------------
// a2dHit
// ---------------------------------------------------------------------------

#include "wx/artbase/polyver.h"
struct a2dHit
{
    a2dHit()
    a2dHit( a2dHit::Hit hit, a2dHit::Stroke1 stroke1, a2dHit::Stroke2 stroke2, unsigned int index, float distance )
    bool IsHit() const
    bool IsStrokeHit() const
    bool IsFillHit() const
    bool IsInsideHit() const
    bool IsDirectStrokeHit() const
    bool IsDirectFillHit() const
    bool IsChildHit() const
    bool IsMemberdHit() const
    bool IsEdgeHit() const
    
    %member static a2dHit stock_nohit
    %member static a2dHit stock_fill
    %member static a2dHit stock_strokeoutside
    %member static a2dHit stock_strokeinside
};

// *****************************************************************
// wx/artbase\stylebase.h
// *****************************************************************

#define NORMFONT 1000

typedef float a2dDash

class wxPenCap
{
};

class wxPenJoin  
{
};

//typedef int wxPenJoin
//typedef int wxPenCap

//class a2dStrokeGlyph
//};
 
class a2dPropertyIdFill
{
};
class a2dPropertyIdStroke
{
};
class a2dPropertyIdFont
{
};

enum a2dStrokeType
{
    a2dSTROKE_NULL,
    a2dSTROKE_OBJECT,
    a2dSTROKE_NORMAL,
    a2dSTROKE_ONE_COLOUR,
    a2dSTROKE_BITMAP
};

enum wxBrushStyle
{
    wxBRUSHSTYLE_INVALID,
    wxBRUSHSTYLE_SOLID,
    wxBRUSHSTYLE_TRANSPARENT,
    wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE,
    wxBRUSHSTYLE_STIPPLE_MASK,
    wxBRUSHSTYLE_STIPPLE,
    wxBRUSHSTYLE_BDIAGONAL_HATCH,
    wxBRUSHSTYLE_CROSSDIAG_HATCH,
    wxBRUSHSTYLE_FDIAGONAL_HATCH,
    wxBRUSHSTYLE_CROSS_HATCH,
    wxBRUSHSTYLE_HORIZONTAL_HATCH,
    wxBRUSHSTYLE_VERTICAL_HATCH,
    wxBRUSHSTYLE_FIRST_HATCH,
    wxBRUSHSTYLE_LAST_HATCH
};

enum a2dFillType
{
    a2dFILL_NULL,
    a2dFILL_ONE_COLOUR,
    a2dFILL_HATCH_TWO_COLOUR,
    a2dFILL_GRADIENT_FILL_LINEAR,
    a2dFILL_GRADIENT_FILL_RADIAL,
    a2dFILL_BITMAP
};

enum a2dFillStyle
{
    a2dFILL_TRANSPARENT,
    a2dFILL_SOLID,
    a2dFILL_BDIAGONAL_HATCH,
    a2dFILL_CROSSDIAG_HATCH,
    a2dFILL_FDIAGONAL_HATCH,
    a2dFILL_CROSS_HATCH,
    a2dFILL_HORIZONTAL_HATCH,
    a2dFILL_VERTICAL_HATCH,
    a2dFIRST_HATCH,
    a2dLAST_HATCH,
    a2dFILL_TWOCOL_BDIAGONAL_HATCH,
    a2dFILL_TWOCOL_CROSSDIAG_HATCH,
    a2dFILL_TWOCOL_FDIAGONAL_HATCH,
    a2dFILL_TWOCOL_CROSS_HATCH,
    a2dFILL_TWOCOL_HORIZONTAL_HATCH,
    a2dFILL_TWOCOL_VERTICAL_HATCH,
    a2dFIRST_TWOCOL_HATCH,
    a2dLAST_TWOCOL_HATCH,
    a2dFILL_STIPPLE,
    a2dFILL_STIPPLE_MASK_OPAQUE,
    a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT,
    a2dFILL_GRADIENT_FILL_XY_LINEAR,
    a2dFILL_GRADIENT_FILL_XY_RADIAL,
    a2dFILL_GRADIENT_FILL_XY_DROP,
    a2dFILL_INHERIT,
    a2dFILL_LAYER,
    a2dFILL_NULLFILL
};

enum a2dFontAlignment
{
    wxMIDY,
    wxMIDX,
    wxMINX,
    wxMAXX,
    wxMINY,
    wxMAXY,
    wxBASELINE,
    wxBASELINE_CONTRA,
    a2dDEFAULT_ALIGNMENT
};

enum a2dStrokeStyle
{
    a2dSTROKE_TRANSPARENT,
    a2dSTROKE_SOLID,
    a2dSTROKE_DOT,
    a2dSTROKE_LONG_DASH,
    a2dSTROKE_SHORT_DASH,
    a2dSTROKE_DOT_DASH,
    a2dSTROKE_USER_DASH,
    a2dSTROKE_BDIAGONAL_HATCH,
    a2dSTROKE_CROSSDIAG_HATCH,
    a2dSTROKE_FDIAGONAL_HATCH,
    a2dSTROKE_CROSS_HATCH,
    a2dSTROKE_HORIZONTAL_HATCH,
    a2dSTROKE_VERTICAL_HATCH,
    a2dSTROKE_STIPPLE,
    a2dSTROKE_STIPPLE_MASK_OPAQUE,
    a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT,
    a2dSTROKE_OBJECTS,
    a2dSTROKE_INHERIT,
    a2dSTROKE_LAYER,
    a2dSTROKE_NULLSTROKE
};

enum a2dFontType
{
    a2dFONT_NULL,
    a2dFONT_WXDC,
    a2dFONT_FREETYPE,
    a2dFONT_STROKED
};

// ---------------------------------------------------------------------------
// a2dFill
// ---------------------------------------------------------------------------

#include "wx/artbase/stylebase.h"
class a2dFill : public wxObject
{
    a2dFill()
    a2dFill( const a2dFill& fill )
    a2dFill( const wxBrush& brush )
    a2dFill( const wxColour& col, a2dFillStyle style = a2dFILL_SOLID )
    a2dFill( const wxColour& col, const wxColour& col2, a2dFillStyle style )
    a2dFill( const wxBitmap& stipple, a2dFillStyle style = a2dFILL_STIPPLE )
    a2dFill( const wxBitmap& stipple, const wxColour& col, const wxColour& col2, a2dFillStyle style = a2dFILL_STIPPLE )
    a2dFill( const wxFileName& filename, a2dFillStyle style = a2dFILL_STIPPLE )
    a2dFill( const wxFileName& filename, const wxColour& col, const wxColour& col2, a2dFillStyle style = a2dFILL_STIPPLE )
    a2dFill( const wxColour& col, const wxColour& col2,double x1, double y1, double x2, double y2 )
    a2dFill( const wxColour& col, const wxColour& col2,double xfc, double yfc, double xe, double ye, double radius )
    %operator bool operator==(const a2dFill& a)
    %operator bool operator!=(const a2dFill& a)
    a2dFill& UnShare()
    wxObjectRefData* CreateRefData() const
    wxObjectRefData* CloneRefData(const wxObjectRefData * data) const
    a2dFillType GetType() const
    a2dFillStyle GetStyle() const
    void SetStyle( a2dFillStyle style )
    a2dFillType GetTypeForStyle( a2dFillStyle style ) const
    void SetStipple( const wxBitmap& stipple )
    const wxBitmap& GetStipple() const
    virtual wxColour GetColour() const
    virtual wxColour GetColour2() const
    virtual void SetColour( const wxColour& col )
    virtual void SetColour2( const wxColour& col )
    void SetAlpha( wxUint8 val )
    wxUint8 GetAlpha() const
    a2dPoint2D GetStart() const
    a2dPoint2D GetStop() const
    wxUint32 GetStamp() const
    a2dPoint2D GetCenter() const
    a2dPoint2D GetFocal() const
    double GetRadius() const
    bool IsNoFill() const
    a2dFill* SetNoFill()
    void SetFilling( bool OnOff )
    bool GetFilling() const
    virtual void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser )
    virtual void Save( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dObjectList* towrite )
};

// ---------------------------------------------------------------------------
// a2dGlyph
// ---------------------------------------------------------------------------

#include "wx/artbase/stylebase.h"
class a2dGlyph
{
    a2dGlyph()
};

// ---------------------------------------------------------------------------
// a2dFontProperty
// ---------------------------------------------------------------------------

#include "wx/artbase/stylebase.h"
class a2dFontProperty : public a2dNamedProperty
{
    a2dFontProperty()
    a2dFontProperty( const a2dPropertyIdFont* id, const a2dFont& value )
    a2dFontProperty( const a2dFontProperty* ori )
    a2dFontProperty(const a2dFontProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dFontProperty *CreatePropertyFromString( const a2dPropertyIdFont* id, const wxString &value )
    void SetValue( const a2dFont& value )
    const a2dFont& GetValue() const
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dFreetypeModule
// ---------------------------------------------------------------------------

#include "wx/artbase/stylebase.h"
class a2dFreetypeModule : public wxModule
{
    a2dFreetypeModule()
    virtual bool OnInit()
    virtual void OnExit()
};

// ---------------------------------------------------------------------------
// a2dFillProperty
// ---------------------------------------------------------------------------

#include "wx/artbase/stylebase.h"
class a2dFillProperty : public a2dNamedProperty
{
    a2dFillProperty()
    a2dFillProperty( const a2dPropertyIdFill* id, const a2dFill& value )
    a2dFillProperty( const a2dFillProperty* ori )
    a2dFillProperty(const a2dFillProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dFillProperty *CreatePropertyFromString( const a2dPropertyIdFill* id, const wxString &value )
    void SetValue( const a2dFill& value )
    const a2dFill& GetValue() const
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

// ---------------------------------------------------------------------------
// a2dStrokeProperty
// ---------------------------------------------------------------------------

#include "wx/artbase/stylebase.h"
class a2dStrokeProperty : public a2dNamedProperty
{
    a2dStrokeProperty()
    a2dStrokeProperty( const a2dPropertyIdStroke* id, const a2dStroke& value )
    a2dStrokeProperty( const a2dStrokeProperty* ori )
    a2dStrokeProperty(const a2dStrokeProperty &other )
    virtual a2dNamedProperty *Clone( a2dObject::CloneOptions options ) const
    virtual void Assign( const a2dNamedProperty &other )
    static a2dStrokeProperty *CreatePropertyFromString( const a2dPropertyIdStroke* id, const wxString &value )
    void SetValue( const a2dStroke& value )
    const a2dStroke& GetValue() const
    virtual wxString StringRepresentation() const
    virtual wxString StringValueRepresentation() const
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
};

%function void a2dCanvasInitializeStockObjects ()
%function void a2dCanvasDeleteStockObjects ()

// ---------------------------------------------------------------------------
// a2dFont
// ---------------------------------------------------------------------------

#include "wx/artbase/stylebase.h"
class a2dFont : public wxObject
{
    a2dFontType GetType() const
    a2dFont()
    a2dFont( const a2dFont& other )
    a2dFont( const wxFont& font, double size, a2dFontType type = a2dFONT_WXDC, wxFontEncoding encoding = wxFONTENCODING_DEFAULT  )
    a2dFont( double size, wxFontFamily family = wxFONTFAMILY_DEFAULT ,wxFontStyle style = wxFONTSTYLE_NORMAL, wxFontWeight weight = wxFONTWEIGHT_NORMAL, const bool underline = false, const wxString& faceName = "",wxFontEncoding encoding = wxFONTENCODING_DEFAULT )
    a2dFont( const wxString& filename, double size = 0.0, wxFontEncoding encoding = wxFONTENCODING_DEFAULT )
    a2dFont( const wxString& filename, double size, bool monoSpaced )
    %operator bool operator==(const a2dFont& a)
    %operator bool operator!=(const a2dFont& a)
    bool Ok() const
    const a2dFontInfo& GetFontInfo() const
    void SetSize( double size )
    double GetSize() const
    double GetLineHeight() const
    double GetDescent() const
    double GetWidth( wxChar c )
    double GetKerning( wxChar c1, wxChar c2 ) const
    double GetStrokeWidth() const
    // a2dStrokeGlyph GetGlyphStroke( wxChar c ) const
    const a2dGlyph* GetGlyphFreetype( wxChar c ) const
#if wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
    a2dFont GetFreetypeFont()
#endif 
    wxFont& GetFont()
    a2dBoundingBox GetTextExtent( const wxString& string, int alignment = wxMINX | wxMAXY,  double* w = NULL,double* h = NULL, double* descent = NULL,double* externalLeading = NULL ) const
    void GetTextExtent( const wxString& string, double& w, double& h,double& descent, double& externalLeading ) const
    bool GetVpath( wxChar c, a2dVpath& glyph, const a2dAffineMatrix& affine = a2dIDENTITY_MATRIX )
    void GetVpath( const wxString& text, a2dVpath& vpath, double x = 0, double y = 0, int alignment = wxMINX | wxMAXY, bool yaxis = true, const a2dAffineMatrix& affine = a2dIDENTITY_MATRIX )
    bool AlwaysWriteSerializationId() const
    //static void GetInfoList( a2dFontInfoList& list )
    static a2dFont CreateFont( const a2dFontInfo& info, bool force = false )
    wxString CreateString()
    void SetFilename( const wxString& filename )
    wxString GetFilename( bool filenameonly = false ) const
    double GetWeight() const
    void SetWeight( double weight )
    void SetWeight( const wxString& weight )
    wxObjectRefData* CreateRefData() const
    wxObjectRefData* CloneRefData(const wxObjectRefData * data) const
};

// ---------------------------------------------------------------------------
// a2dFontInfo
// ---------------------------------------------------------------------------

#include "wx/artbase/stylebase.h"
class a2dFontInfo
{
    a2dFontInfo()
    a2dFontInfo( const wxString& type, const wxString& name, const wxString& style = "",double size = 1.0, const wxString& extra = "" )
    a2dFontInfo( const wxString& type, const wxString& name,const wxString& style, const wxString& extra = "" )
    a2dFontInfo( const wxString& info )
    %operator a2dFontInfo& operator=( const a2dFontInfo& other )
    %operator bool operator== ( const a2dFontInfo& other ) const
    a2dFont CreateFont( double size )
    const wxString& GetType() const
    void SetType( const wxString& type )
    const wxString& GetName() const
    void SetName( const wxString& name )
    const wxString& GetStyle() const
    void SetStyle( const wxString& style )
    double GetSize() const
    void SetSize( double size )
    wxString CreateString() const
    void ParseString( wxString string )
    const wxString& GetExtra() const
    void SetExtra( const wxString& extra )
};

// ---------------------------------------------------------------------------
// a2dStroke
// ---------------------------------------------------------------------------

#include "wx/artbase/stylebase.h"
class a2dStroke : public wxObject
{
    a2dStroke()
    a2dStroke( const wxColour& col, a2dStrokeStyle style = a2dSTROKE_SOLID )
    a2dStroke( const wxColour& col, double width, a2dStrokeStyle style = a2dSTROKE_SOLID )
    a2dStroke( const wxColour& col, int width, a2dStrokeStyle style = a2dSTROKE_SOLID )
    a2dStroke( const wxBitmap& stipple, a2dStrokeStyle style = a2dSTROKE_STIPPLE )
    a2dStroke( const wxBitmap& stipple, const wxColour& col, const wxColour& col2, a2dStrokeStyle style = a2dSTROKE_STIPPLE )
    a2dStroke( const wxFileName& filename, a2dStrokeStyle style = a2dSTROKE_STIPPLE )
    a2dStroke( const wxFileName& filename, const wxColour& col, const wxColour& col2, a2dStrokeStyle style = a2dSTROKE_STIPPLE )
    a2dStroke( const a2dStroke& stroke )
    a2dStroke( const wxPen& stroke )
    a2dStroke& UnShare()
    %operator bool operator==(const a2dStroke& a)
    %operator bool operator!=(const a2dStroke& a)
    wxObjectRefData* CreateRefData() const
    wxObjectRefData* CloneRefData(const wxObjectRefData * data) const
    virtual a2dStrokeType GetType() const
    void SetStipple( const wxBitmap& stipple )
    const wxBitmap& GetStipple() const
    wxColour GetColour() const
    void SetColour( const wxColour& col )
    void SetJoin( wxPenJoin join_style )
    void SetCap( wxPenCap cap_style )
    wxPenCap GetCap() const
    wxPenJoin GetJoin() const
    void SetWidth( float width )
    virtual float GetWidth() const
    virtual float GetExtend() const
    void SetPixelStroke( bool pixelstroke )
    bool GetPixelStroke() const
    void SetAlpha( wxUint8 val )
    wxUint8 GetAlpha() const
    a2dStrokeStyle GetStyle() const
    void SetStyle( a2dStrokeStyle style )
    bool IsNoStroke() const
    a2dStroke* SetNoStroke()
    a2dStrokeType GetTypeForStyle( a2dStrokeStyle style ) const
    virtual void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser )
    virtual void Save( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dObjectList* towrite )
};


