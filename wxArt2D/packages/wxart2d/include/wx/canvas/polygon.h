/*! \file wx/canvas/polygon.h
    \brief all polygon and polyline a2dCanvasObject are here.

    Next to that the handle classes that are specialized for editing polygonn types.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: polygon.h,v 1.35 2009/10/01 19:22:35 titato Exp $
*/

#ifndef __WXPOLYGON_H__
#define __WXPOLYGON_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/artbase/afmatrix.h"
#include "wx/geometry.h"
#include "wx/artbase/bbox.h"
#include "wx/artbase/polyver.h"

#include "wx/canvas/canobj.h"

class A2DCANVASDLLEXP a2dPolyHandleL;
class A2DCANVASDLLEXP a2dSurface;

//----------------------------------------------------------------------------
// a2dPolygonL
//----------------------------------------------------------------------------

//!polygon defined with list of points.
/*!
    The point in the polygon are relative to 0,0, without the local matrix aplied.
    So all points will be transformed with the local matrix before drawing them.
    The position of the polygon is defined by the local matrix its translation.
    A polygon point (0,0) would be exactly at the translation set in the local matrix.

    \ingroup canvasobject

*/
class A2DCANVASDLLEXP a2dPolygonL: public a2dCanvasObject
{
    DECLARE_EVENT_TABLE()

public:

    a2dPolygonL();
    a2dPolygonL( a2dVertexListPtr points, bool spline = false  );
    a2dPolygonL( const a2dPolygonL& poly, CloneOptions options, a2dRefMap* refs );
    ~a2dPolygonL();

    void SetPending( bool pending );

    a2dSurface* GetAsSurface() const;

    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    virtual a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const;

    virtual a2dCanvasObjectList* GetAsLinesArcs( bool transform = true );

    virtual a2dCanvasObjectList* GetSnapVpath( a2dSnapToWhatMask snapToWhat );

    virtual bool RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld );

    //! add point/segment after a segment.
    /*!
    \param x x of segment to add.
    \param y y of segment to add.
    \param seg insert new segment starting at x, y here. If NULL, add at end.
    \param afterinversion if true, x,y is first transformed to the polygon its segments, using its matrix.

    \return pointer to new inserted segment
    */
    a2dLineSegment* AddPoint( double x, double y, a2dLineSegmentPtr seg = NULL, bool afterinversion = true );
    //! add point/segment after index given
    a2dLineSegment* AddPoint( const a2dPoint2D& P, int index, bool afterinversion = true );
    //! add point/segment after index given
    a2dLineSegment* AddPoint( double x, double y, int index, bool afterinversion = true );
    //! insert segment after index given
    virtual void InsertSegment( unsigned int index, a2dLineSegmentPtr segin ) {  m_lsegments->Insert( index, segin ); }

    //! remove all points that are redundant
    /*! currently this only removes successive points at the same coordinate,
        but it could also remove points with an angle of 180 deg
        \param sendCommandsTo if not NULL, send a2dCommand_RemovePoint commands to this object
    */
    void RemoveRedundantPoints( a2dCanvasObject* sendCommandsTo = NULL );

    //!removes point at index
    /*!
        \param x x value of removed point
        \param y y value of removed point
        \param seg which segment
        \param transformed if true returned values x,y are transformed with local matrix
    */
    virtual void RemoveSegment( double& x, double& y , a2dLineSegmentPtr seg = NULL, bool transformed = true  );

    //!removes point at index
    /*!
        \param x x value of removed point
        \param y y value of removed point
        \param index index of point to remove (-1 means last point)
        \param transformed if true returned values x,y are transformed with local matrix
    */
    void RemoveSegment( double& x, double& y , int index = -1, bool transformed = true  );

    //! Remove and delete all points in the polygon
    void Clear();

    //! reduce matrix to identity without replacing object
    bool EliminateMatrix();

    //!modify point at index to x,y
    /*!
        Set the coordinates of the point at index to x,y.
        \param index the index of the point to change
        \param x new x for the point
        \param y new y for the point
        \param afterinversion if true the input x,y or first inverted with the local matrix of the object.
        The inverted values are used to set the point.

        \remark afterinversion = true should be used if you have x,y relative to the polygon object,
        while afterinversion = false should be used when x,y are relative to the point in the polygon.
    */
    void SetPosXYSegment( int index, double x, double y, bool afterinversion = true );

    //!modify point at index to x,y
    /*!
        Set the coordinates of the point at index to x,y.
        \param seg the segment to change
        \param x new x for the point
        \param y new y for the point
        \param afterinversion if true the input x,y or first inverted with the local matrix of the object.
        The inverted values are used to set the point.

        \remark afterinversion = true should be used if you have x,y relative to the polygon object,
        while afterinversion = false should be used when x,y are relative to the point in the polygon.
    */
    virtual void SetPosXYSegment( a2dLineSegmentPtr seg, double x, double y, bool afterinversion = true );


    //!modify middle point of arcsegment at index to x,y
    /*!
        Set the coordinates of the point at index to x,y.
        \param seg the segment to change
        \param x new x for the middle point
        \param y new y for the middle point
        \param afterinversion if true the input x,y or first inverted with the local matrix of the object.
        The inverted values are used to set the point.

        \remark afterinversion = true should be used if you have x,y relative to the polygon object,
        while afterinversion = false should be used when x,y are relative to the point in the polygon.
    */
    void SetPosXYMidSegment( a2dLineSegmentPtr seg, double x, double y, bool afterinversion = true );

    //!modify middle point of arcsegment at index to x,y
    /*!
        Set the coordinates of the point at index to x,y.
        \param index the index of the point to change
        \param x new x for the middle point
        \param y new y for the middle point
        \param afterinversion if true the input x,y or first inverted with the local matrix of the object.
        The inverted values are used to set the point.

        \remark afterinversion = true should be used if you have x,y relative to the polygon object,
        while afterinversion = false should be used when x,y are relative to the point in the polygon.
    */
    void SetPosXYMidSegment( int index, double x, double y, bool afterinversion = true );

    //! get point x and y at index
    /*!
        \param index the index of the point to get (-1 for last segment)
        \param x new x for the point
        \param y new y for the point
        \param transform if true return x y transformed with local matrix
    */
    void GetPosXYSegment( int index, double& x, double& y, bool transform = true ) const;

    //! get point x and y at index
    /*!
        \param seg the segment to change
        \param x new x for the point
        \param y new y for the point
        \param transform if true return x y transformed with local matrix
    */
    virtual void GetPosXYSegment( a2dLineSegmentPtr seg, double& x, double& y, bool transform = true ) const;

    //! get middle point of arcsegment at index to x,y
    /*!
        \param index the index of the point to get
        \param x new x for the point
        \param y new y for the point
        \param transform if true return x y transformed with local matrix
    */
    void GetPosXYMidSegment( int index, double& x, double& y, bool transform = true ) const;

    //! get middle point of arcsegment at index to x,y
    /*!
        \param seg the segment to change
        \param x new x for the point
        \param y new y for the point
        \param transform if true return x y transformed with local matrix
    */
    void GetPosXYMidSegment( a2dLineSegmentPtr seg, double& x, double& y, bool transform = true ) const;

    //! adjust the polygon after a point change
    /*! This version does nothing, but derived classes, notably wires, might want to update
        something when the point coordinates change. This is not part of SetPosXYPoint,
        because these updates should only be done by interactive editing functions on copies
        by e.g. handle events, and not by commands.
        As an example a2dWirePolylineL uses this function to adjust a2dPin location with its segments.
        \param final if true make changes by issuing stored commands
        \return true if a change was made
    */
    virtual bool AdjustAfterChange( bool final );

    //! Get the list of points ( this is not a copy! )
    a2dVertexListPtr GetSegments() const { return m_lsegments; }

    //! Set the list of points ( the old list is NOT DELETED !!! )
    /*!
        \param points list of points
        \param delold if true, the old points list is deleted. Be carefull, if the points list is still needed for undo redo.
    */
    void SetSegments( a2dVertexList* points );

    //! get the number of points in the pointlist
    size_t GetNumberOfSegments() const { return m_lsegments->size(); }

    //! set to true, the polygon will be drawn as a spline
    void SetSpline( bool on ) { m_spline = on; SetPending( true ); }

    //! Get the polygon spline setting
    bool GetSpline() const { return m_spline; }

    void SetContourWidth( double width );

    //!get the Contour width of the shape
    double GetContourWidth() const { return m_contourwidth; }

    //!Set Polygon filling mode wxODDEVEN_RULE or wxWINDING_RULE
    void SetFillRule( wxPolygonFillMode val ) { m_oddeven = val; }

    //!Get Polygon filling mode wxODDEVEN_RULE or wxWINDING_RULE
    wxPolygonFillMode GetFillRule() { return m_oddeven; }

    //! a handle in editing is updated to a segment at index given
    virtual void SetHandleToIndex( a2dPolyHandleL* handle, unsigned int index );

    virtual int GetIndexSegment( a2dLineSegmentPtr seg );

    virtual a2dVertexList::iterator GetSegmentAtIndex( unsigned int index );

    virtual a2dVertexList* GetSegmentListAtIndex( unsigned int index );

#if wxART2D_USE_CVGIO
    void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    virtual a2dHit PointInPolygon( const a2dPoint2D& P, double marge );

	static void SetAllowCrossAtVertex( bool allowCrossAtVertex ) { m_allowCrossAtVertex = allowCrossAtVertex; }

    DECLARE_DYNAMIC_CLASS( a2dPolygonL )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void SyncHandlesWithLineSegments();

    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    void OnChar( wxKeyEvent& event );

	void OnEnterObject( a2dCanvasObjectMouseEvent& event );

	void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    void OnHandleEvent( a2dHandleMouseEvent& event );

    void OnHandleEventEnter( a2dHandleMouseEvent& event );

    void OnHandleEventLeave( a2dHandleMouseEvent& event );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    //return this object if one of the objects it references is hit
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    a2dSmrtPtr<a2dVertexList> m_lsegments;

    static bool m_allowCrossAtVertex; 

    void convertToIntPointCache();

    bool m_spline: 1; 
    bool m_crossAtVertex: 1; 

    wxPolygonFillMode m_oddeven; //: 2; make this a bitfield if new members are added

    //! if != 0 the polygon is contoured at distance m_contourwidth/2
    double m_contourwidth;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dPolygonL( const a2dPolygonL& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dPolygonL>;
#endif

//----------------------------------------------------------------------------
// a2dPolylineL
//----------------------------------------------------------------------------

//!polyline defined with list of points.
/*!
    The point in the polyline are relative to 0,0, without the local matrix aplied.
    So all points will be transformed with the local matrix before drawing them.
    The position of the polyline is defined by the local matrix its translation.
    A polyline point (0,0) would be exactly at the translation set in the local matrix.

    \ingroup canvasobject

*/
class A2DCANVASDLLEXP a2dPolylineL: public a2dPolygonL
{
    DECLARE_EVENT_TABLE()

public:
    a2dPolylineL();
    a2dPolylineL( a2dVertexListPtr segments, bool spline = false  );
    a2dPolylineL( const a2dPolylineL& poly, CloneOptions options, a2dRefMap* refs );
    ~a2dPolylineL();

    virtual a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const;

    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    virtual a2dCanvasObjectList* GetAsLinesArcs( bool transform = true );

    virtual bool RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld );

    //! remove all points that are redundant
    /*! removes successive points at the same coordinate,
        or if 3 successive points are one the same vertical or horizontal line, remove the middle point
        \param sendCommandsTo if not NULL, send a2dCommand_RemovePoint commands to this object
    */
    void RemoveRedundantPoints( a2dCanvasObject* sendCommandsTo = NULL );

    //! remove segments which are opposite direction overlapping.
    /*!
        \param sendCommandsTo if not NULL, send a2dCommand_RemovePoint commands to this object
        \param removeZeroFirst if true first zero lenght segments are removed.
    */
    void Remove180DegPoints( a2dCanvasObject* sendCommandsTo = NULL, bool removeZeroFirst = true );

#if wxART2D_USE_CVGIO
    void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    a2dHit PointOnPolyline( const a2dPoint2D& P, double marge );

    void SetContourWidth( double width ); 

    //! set object to draw as begin point
    void SetBegin( a2dCanvasObject* begin );

    a2dCanvasObject* GetBegin() { return m_begin; }

    //! set object to draw as end point
    void SetEnd( a2dCanvasObject* end );

    a2dCanvasObject* GetEnd() { return m_end; }

    //!scale begin and end object with this factor in X
    /*!
        \remark set object pending for redraw
    */
    void SetEndScaleX( double xs ) { m_xscale = xs; SetPending( true ); }

    //!scale begin and end object with this factor in X
    double GetEndScaleX() { return m_xscale; }

    //!scale begin and end object with this factor in Y
    /*!
        \remark set object pending for redraw
    */
    void SetEndScaleY( double ys ) { m_yscale = ys; SetPending( true ); }

    //!scale begin and end object with this factor in X
    double GetEndScaleY() { return m_yscale; }

    //! Set Pathtype, when m_contourwidth != 0 defines what the end of the line should be.
    void SetPathType( a2dPATH_END_TYPE pathtype ) { m_pathtype = pathtype;  SetPending( true ); }

    //! Get Pathtype, when m_contourwidth != 0 defines what the end of the line should be.
    a2dPATH_END_TYPE GetPathType() { return m_pathtype; }

    //! Set PathCorner type, when m_contourwidth != 0 defines what the corner of the lines should be.
    void SetPathCornerType( a2dPATH_CORNER_TYPE pathCornerType ) { m_pathCornerType = pathCornerType;  SetPending( true ); }

    //! Get PathCorner type, when m_contourwidth != 0 defines what the corner of the lines should be.
    a2dPATH_CORNER_TYPE GetPathCornerType() { return m_pathCornerType; }

    DECLARE_DYNAMIC_CLASS( a2dPolylineL )

    //! Find the point on the polyline closest to the given world point
    /*! \param cworld is the accumulated affine transformation WITHOUT the local transform. If NULL, all coordinates are local
        \param xIn x-world coordinate of point to which nearest point on line is searched
        \param yIn y-world coordinate of point to which nearest point on line is searched
        \param xOut x-world coordinate of nearest point on line
        \param yOut y-world coordinate of nearest point on line
    */
    bool FindNearPoint( const a2dAffineMatrix* cworld, double xIn, double yIn, double* xOut, double* yOut );

    //! Find the point on the polyline that is the closest orthogonal projection to the given world point
    /*! \param cworld is the accumulated affine transformation WITHOUT the local transform. If NULL, all coordinates are local
        \param xIn x-world coordinate of point to which nearest point on line is searched
        \param yIn y-world coordinate of point to which nearest point on line is searched
        \param xOut x-world coordinate of nearest point on line
        \param yOut y-world coordinate of nearest point on line
    */
    bool FindNearPointOrtho( const a2dAffineMatrix* cworld, double xIn, double yIn, double* xOut, double* yOut );

    //! Move the given dynamic pin close to the given absolute point
    /*! \return true if the pin was really moved */
    bool MoveDynamicPinCloseTo( a2dPin* pin, const a2dPoint2D& point, bool final );

    //! pins on the begin and end of the wire are created, and after that
    //! point along the line if possible at position  x,y
    virtual bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y, double margin = 0 );

    //! Get the begin pin from the childs
    a2dPin* FindBeginPin();
    //! Get the end pin from the childs
    a2dPin* FindEndPin();
    //! Get all pins one by one. i is updated by this function.
    a2dPin* FindPin( int* i );

    a2dLineSegment* GetFirstPoint() const;
    a2dLineSegment* GetLastPoint() const;

    int FindSegmentIndex( const a2dPoint2D& point, double margin = a2dACCUR );

    //! find the index of the segment where the pin is on.
    int FindPinSegmentIndex( a2dPin* pinToFind, double margin = a2dACCUR );

    //! find the segment where the pin is on.
    a2dLineSegment* FindPinSegment( a2dPin* pinToFind, double margin = a2dACCUR );

    //! new object willl generate pins or not  based on this value
    static bool SetDefaultGeneratePins( bool value ) { m_generatePins = value; }

    //! new object willl generate pins or not  based on this value
    static bool GetDefaultGeneratePins() { return m_generatePins; }

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    bool DoCanConnectWith( a2dIterC& ic, a2dPin* pin, double margin, bool autocreate );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    //!return this object if one of the objects it references is hit
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void OnChar( wxKeyEvent& event );
    
    a2dCanvasObjectPtr m_begin;
    a2dCanvasObjectPtr m_end;

    double  m_xscale;
    double  m_yscale;

    static bool m_generatePins; 

    //! when m_contourwidth != 0 what is the end of the line looking like.
    a2dPATH_END_TYPE m_pathtype;

    a2dPATH_CORNER_TYPE m_pathCornerType;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dPolylineL( const a2dPolylineL& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dPolylineL>;
#endif

//----------------------------------------------------------------------------
// a2dSurface
//----------------------------------------------------------------------------

//!polygon defined with list of points for outer contour plus a list of hole polygons
/*!
    As a a2dPolygonL but, and extra list of polygons is used to define holes within the outer contour.

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dSurface: public a2dPolygonL
{
    DECLARE_EVENT_TABLE()

public:

    a2dSurface();
    a2dSurface( a2dVertexListPtr points, bool spline = false  );
    a2dSurface( const a2dSurface& poly, CloneOptions options, a2dRefMap* refs );
    ~a2dSurface();

    virtual a2dHit PointInPolygon( const a2dPoint2D& P, double marge );

    a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const;

    //! convert contour to a polygons and holes to seperate polygons.
    /*!
        Holes will be lost in this case.
    */
    virtual a2dCanvasObjectList* GetAsPolygons( bool transform = true ) const;

    //! convert contour to a polyline and holes to seperate polylines.
    /*!
    */
    virtual a2dCanvasObjectList* GetAsPolylines( bool transform = true ) const;

    virtual a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    virtual a2dCanvasObjectList* GetAsLinesArcs( bool transform = true );

    void AddHole( a2dVertexListPtr holepoints );

    //! remove all points that are redundant
    /*! removes successive points at the same coordinate,
        or if 3 successive points are one the same vertical or horizontal line, remove the middle point
        \param sendCommandsTo if not NULL, send a2dCommand_RemovePoint commands to this object
    */
    void RemoveRedundantPoints( a2dCanvasObject* sendCommandsTo = NULL );

    //! Remove and delete all points in the polygon
    void Clear();

    //! reduce matrix to identity without replacing object
    bool EliminateMatrix();

    //! Get the list of points ( this is not a copy! )
    a2dVertexListPtr GetSegments() const { return m_lsegments; }

    //! Set the list of points ( the old list is NOT DELETED !!! )
    /*!
        \param points list of points
        \param delold if true, the old points list is deleted. Be carefull, if the points list is still needed for undo redo.
    */
    void SetSegments( a2dVertexList* points );

    //! get the number of points in the pointlist
    size_t GetNumberOfSegments() const { return m_lsegments->size(); }

#if wxART2D_USE_CVGIO
    void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void SetContourWidth( double width ) {}

    //!get the Contour width of the shape
    double GetContourWidth() const { return 0; }

    a2dListOfa2dVertexList& GetHoles() { return m_holes; }

    void SetHandleToIndex( a2dPolyHandleL* handle, unsigned int index );

    int GetIndexSegment( a2dLineSegmentPtr seg );

    virtual a2dVertexList::iterator GetSegmentAtIndex( unsigned int index );

    virtual a2dVertexList* GetSegmentListAtIndex( unsigned int index );

    void RemoveSegment( double& x, double& y , a2dLineSegmentPtr seg = NULL, bool transformed = true  );

    void InsertSegment( unsigned int index, a2dLineSegmentPtr segin );

    void GetPosXYSegment( a2dLineSegmentPtr seg, double& x, double& y, bool transform = true ) const;

    void SetPosXYSegment( a2dLineSegmentPtr seg, double x, double y, bool afterinversion = true );

    DECLARE_DYNAMIC_CLASS( a2dSurface )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    void OnHandleEvent( a2dHandleMouseEvent& event );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    //return this object if one of the objects it references is hit
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    a2dListOfa2dVertexList m_holes;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dSurface( const a2dSurface& other );
};


//! clips all children to a polygon
/*!

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dPolygonLClipper: public a2dPolygonL
{
public:

    //! construct the clipping polygon according to the given box
    a2dPolygonLClipper( a2dBoundingBox& bbox = wxNonValidBbox );

    //! construct the clipping polygon according to the given polygon
    a2dPolygonLClipper( a2dVertexList* points, bool spline = false );

    //! destructor
    ~a2dPolygonLClipper();

    //! copy with clone options
    a2dPolygonLClipper( const a2dPolygonLClipper& other, CloneOptions options, a2dRefMap* refs );

    //!Set the matrix used for the clipping polygon
    /*!
        Children object are clipped to a polygon.
        The child object are transformed with the m_lworld transform.
        But the clipping polygon is using m_ClipLworld to place/draw itself.

        \param mat matrix to be used for the clipping polygon
    */
    void SetClippingTransformMatrix( const a2dAffineMatrix& mat = a2dIDENTITY_MATRIX );

    //! set the clipping polygon according to the given box
    void SetClippingFromBox( a2dBoundingBox& bbox = wxNonValidBbox );

    //! render by first setting clipping box and next start rendering children and itself.
    void Render( a2dIterC& ic, OVERLAP clipparent );

    virtual bool Update( UpdateMode mode );

protected:

    //! clone it
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    //! clipping matrix.
    a2dAffineMatrix m_ClipLworld;

    DECLARE_DYNAMIC_CLASS( a2dPolygonLClipper )

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dPolygonLClipper( const a2dPolygonLClipper& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dPolygonLClipper>;
#endif

//! clips all children to a polygon
/*!

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dPolygonLClipper2: public a2dPolygonL
{
public:

    a2dPolygonLClipper2( a2dBoundingBox& bbox = wxNonValidBbox );

    a2dPolygonLClipper2( a2dVertexList* segments, bool spline = false );

    ~a2dPolygonLClipper2();

    a2dPolygonLClipper2( const a2dPolygonLClipper2& other, CloneOptions options, a2dRefMap* refs );

    void Render( a2dIterC& ic, OVERLAP clipparent );

    virtual bool Update( UpdateMode mode  );

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    DECLARE_DYNAMIC_CLASS( a2dPolygonLClipper2 )

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dPolygonLClipper2( const a2dPolygonLClipper2& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dPolygonLClipper2>;
#endif


//! handle holds a pointer to a polygon/polyline segment
/*! this class is used for editing, to quickly update handles from vertexes.
*/
class A2DCANVASDLLEXP a2dPolyHandleL: public a2dHandle
{
public:

    DECLARE_DYNAMIC_CLASS( a2dPolyHandleL )

public:
    //! constructor
    a2dPolyHandleL();

    //! construct for use in a2dVertexList
    /*!
        node based

        \param parent polygon that contains this handle
        \param segNode pointer to the node of the segment for which this handle is generated.
        \param segNodeOrg pointer to the node in original of the segment for which this handle is generated.
        \param lsegments list of segments where segNode points to
        \param lsegments list of segments where segNodeOrg points to
        \param name name of handle
    */
    a2dPolyHandleL( a2dPolygonL* parent,
                    a2dVertexList::iterator segNode,
                    a2dVertexList::iterator segNodeOrg,
                    a2dSmrtPtr<a2dVertexList> lsegments,
                    a2dSmrtPtr<a2dVertexList> lsegmentsOrg,
                    const wxString& name = wxT( "" ) );

    //! construct for use in a2dVertexList
    /*!
        node based

        \param parent polygon that contains this handle
        \param segNode pointer to the node of the segment for which this handle is generated.
        \param segNodeOrg pointer to the node in original of the segment for which this handle is generated.
        \param lsegments list of segments where segNode points to
        \param lsegments list of segments where segNodeOrg points to
        \param xc center x
        \param yc center y
        \param name name of handle
    */
    a2dPolyHandleL( a2dPolygonL* parent,
                    a2dVertexList::iterator segNode,
                    a2dVertexList::iterator segNodeOrg,
                    a2dSmrtPtr<a2dVertexList> lsegments,
                    a2dSmrtPtr<a2dVertexList> lsegmentsOrg,
                    double xc, double yc, const wxString& name = wxT( "" ) );

    //! copy constructor
    a2dPolyHandleL( const a2dPolyHandleL& other, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dPolyHandleL();

    a2dVertexList::iterator GetNode() { return m_segNode; }

    void SetNode( const a2dVertexList::iterator& iter ) { m_segNode = iter; }

    a2dVertexList::iterator GetNodeOrg() { return m_segNodeOrg; }

    void SetNodeOrg( const a2dVertexList::iterator& iter ) { m_segNodeOrg = iter; }

    //! set handle on segment as arc modifier
    void SetArcHandle( bool arcHandle ) { m_arcHandle = arcHandle; }
    //! is the handle on segment an arc modifier?
    bool GetArcHandle() { return m_arcHandle; }

    //! get to segments chain to which this handle belongs in the original ( not edit copy )
    a2dVertexList* GetSegments() const { return m_lsegments; }

    //! get to segments chain to which this handle belongs in the original ( not edit copy )
    a2dVertexList* GetSegmentsOrg() const { return m_lsegmentsOrg; }

    //! if handle is at the position of a pin, it can be set here, to make it easy moving it in sync with the handle.
    void SetPin( a2dPin* pin ) { m_pin = pin; }
    //! see SetPin()
    a2dPin* GetPin() { return m_pin; }

public:

    //! indicates dragging was/is going on
    bool m_dragging;

    // to group command as a result of various mouse events 
    a2dCommandGroup* m_group;

    static a2dPropertyIdDouble* PROPID_tmpXIH;
    static a2dPropertyIdDouble* PROPID_tmpYIH;
    static a2dPropertyIdDouble* PROPID_tmpXIV1;
    static a2dPropertyIdDouble* PROPID_tmpYIV1;
    static a2dPropertyIdDouble* PROPID_tmpXIV2;
    static a2dPropertyIdDouble* PROPID_tmpYIV2;

    DECLARE_PROPERTIES()

protected:
    a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

private:

    //! to the node of the segment for which this handle is used
    a2dVertexList::iterator m_segNode;

    //! to the node of the segment for which this handle is used
    a2dVertexList::iterator m_segNodeOrg;

    //! handle is for arc segments
    bool m_arcHandle;

    //! segments of handle
    a2dSmrtPtr<a2dVertexList> m_lsegments;

    //! segments of original of handle
    a2dSmrtPtr<a2dVertexList> m_lsegmentsOrg;

	a2dPinPtr m_pin;

    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dPolyHandleL( const a2dPolyHandleL& other );

};
#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dPolyHandleL>;
#endif

#endif

