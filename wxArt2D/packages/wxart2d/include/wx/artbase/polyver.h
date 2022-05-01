/*! \file wx/artbase/polyver.h
    \brief general vertexlist and array and vector path functions and classes.

    All ways to have polygons and polylines stored and drawn by a drawing context
    are placed here. Polygon or polylines can be list based or array based.
    The vector path is the structure which can be used to drawn almost anything.
    Basic primtives can always be converted to a vector path, and drawn that way.

    \author Klaas Holwerda, Michael Sögtrop, Faust Nijhuis
    \date Created 01/09/04

    Copyright: 2001-2004 (C) Klaas Holwerda, Michael Sögtrop

    Licence: wxWidgets licence

    RCS-ID: $Id: polyver.h,v 1.37 2009/09/30 18:38:56 titato Exp $
*/

#ifndef _WX_POLYVERH__
#define _WX_POLYVERH__

#include "wx/geometry.h"

#include "wx/general/gen.h"
#include "wx/artbase/artglob.h"
#include "wx/artbase/afmatrix.h"
#include "wx/artbase/liner.h"
#include "wx/artbase/bbox.h"

//! defines the way a polyline with a contour width is ended.
enum a2dPATH_END_TYPE {a2dPATH_END_SQAURE, a2dPATH_END_ROUND, a2dPATH_END_SQAURE_EXT};
enum a2dPATH_CORNER_TYPE 
{
    a2dPATH_CORNER_SQAURE, 
    a2dPATH_CORNER_MITERED, 
    a2dPATH_CORNER_ADAPTIVEMITER, 
    a2dPATH_CORNER_CURVE, 
    a2dPATH_CORNER_ROUNDED
};

//! is the angle within the arc segment taking into account drawing from
//! start to end in clockwise or anti clocwise direction.
A2DARTBASEDLLEXP bool InArc( double angle, double start, double end, bool clockwise );

//! struct for how a single object on one layer was hit
struct A2DARTBASEDLLEXP a2dHit
{
    //! Basic hit type
    /*! For a object without childs, this is either hit_none, hit_stroke or hit_fill.
        A stroke hit is also given if the test-point has a margin distance from the strokes fill.
        This way 0-width strokes can also be hit. A distinction between a true stroke hit
        and a stroke margin hit is currently not possible.
        If the stroke or fill of a child or member is hit, the hit_child or hit_member flag is also set.
        Note: a2dHit is only used temporarily, so there is no need to save
        memory using bitfields.
    */
    enum Hit
    {
        //! The object is not hit. This should not happen, because the parent should include hit childs
        hit_none   = 0x00,
        //! The point is on the stroke or stroke margin
        hit_stroke = 0x01,
        //! The point is in the fill area
        hit_fill   = 0x02,
        //! The point hits a child of the object
        hit_child  = 0x04,
        //! The point hits a member of the object (e.g. line begin/end object)
        hit_member = 0x08,
    } m_hit;

    friend inline Hit operator | ( Hit a, Hit b ) { return ( Hit )( ( int )a | ( int ) b ); }

    //! How the point is on the stroke ( in stroke perpendicular direction )
    enum Stroke1
    {
        stroke1_none,
        // the hit is on the outside (non-fill) margin of the stroke
        stroke1_outside,
        // the hit is on the inside (fill) margin of the stroke
        stroke1_inside
    } m_stroke1;

    //! How the point is on the stroke ( in stroke parallel direction )
    enum Stroke2
    {
        stroke2_none,
        stroke2_vertex,
        stroke2_edgehor,
        stroke2_edgevert,
        stroke2_edgeother
    } m_stroke2;

    //! For edge/vertex hits the index of the edge / vertex
    unsigned int m_index;

    //! For margin hits, the distance from the stroke center in fractions of the margin
    /*! Note: using a fraction of the margin makes the distance independent of local scaling */
    float m_distance;

    //! Default constructor
    a2dHit() { memset( this, 0, sizeof( *this ) ); }
    //! Standard constructor
    a2dHit( Hit hit, Stroke1 stroke1, Stroke2 stroke2, unsigned int index, float distance )
    {
        m_hit = hit;
        m_stroke1 = stroke1;
        m_stroke2 = stroke2;
        m_distance = distance;
        m_index = index;
    }
    //! true if this is a hit
    bool IsHit() const { return m_hit != hit_none; }
    //! true if this is a stroke hit (parent or child/member stroke)
    bool IsStrokeHit() const { return ( m_hit & hit_stroke ) != 0; }
    //! true if this is a fill hit (parent or child/member fill)
    bool IsFillHit() const { return ( m_hit & hit_fill ) != 0; }
    //! true if this is a fill hit or an inside stroke hit (parent or child/member)
    bool IsInsideHit() const { return ( m_hit & hit_fill ) != 0 || (( m_hit & hit_stroke ) != 0 && m_stroke1 == stroke1_inside); }

    //! true if this is a direct stroke hit (not a member or child object stroke hit )
    bool IsDirectStrokeHit() const { return m_hit == hit_stroke; }
    //! true if this is a direct fill hit (not a member or child object fill hit )
    bool IsDirectFillHit() const { return m_hit == hit_fill; }
    //! true if this is child hit
    bool IsChildHit() const { return ( m_hit & hit_child ) != 0; }
    //! true if this is member object hit (e.g. line begin/end object)
    bool IsMemberdHit() const { return ( m_hit & hit_member ) != 0; }

    //! true if hit on stroke at a vertex
    bool IsVertexHit() const { return ( IsStrokeHit() && m_stroke2 == stroke2_vertex ) != 0 ; }

    //! true if this is a stroke hit on an edge
    bool IsEdgeHit() const
    {
        return ( IsStrokeHit() &&
                 ( m_stroke2 == stroke2_edgehor || m_stroke2 == stroke2_edgevert || m_stroke2 == stroke2_edgeother ) ) != 0 ;
    }

    //! Stock object for no hit
    static a2dHit stock_nohit;
    //! Stock object for a fill hit
    static a2dHit stock_fill;
    //! Stock object for an outer stroke hit on objects without vertices/edges (like circles)
    static a2dHit stock_strokeoutside;
    //! Stock object for an inner stroke hit on objects without vertices/edges (like circles)
    static a2dHit stock_strokeinside;
};

//! defines the type of a segment in a a2dLineSegment
enum a2dSegType
{
    //! not specific or part of outer contour
    a2dNORMAL_SEG = 0x0000,
    //! links an outside contour with a hole
    a2dLINK_SEG = 0x0001,
    //! this segmnet is part of a hole
    a2dHOLE_SEG = 0x0002
};

//! Normal straight line segment in a2dVertexList and a2dVertexArray
/*! baseclass for segments in a a2dVertexList and a2dVertexArray
Every segment type in a a2dVertexList and a2dVertexArray has this class as Baseclass.
It has/maintains the end position of a segment.
But for the first point it is a2dVertexList and a2dVertexArray to give the begin point too.

    \ingroup vpath
*/
class A2DARTBASEDLLEXP a2dLineSegment
{

#ifdef CLASS_MEM_MANAGEMENT

    //! memory manager for speed up to replace system calls allocation and deallocation
    static a2dMemManager sm_memManager;
public:
    //! overloaded operator new for this class and it all derived classes
    void* operator new( size_t bytes )
    {
        return sm_memManager.Allocate( bytes );
    }

    //! overloaded operator delete for this class and it all derived classes
    /*!
    This function doesn't free to OS-system memory block by pointer 'space'.
    It adds memory block by pointer 'space' to internal lists.
    It is speed up.
    */
    void operator delete( void* space, size_t bytes )
    {
        sm_memManager.Deallocate( space, bytes );
    }
#endif //CLASS_MEM_MANAGEMENT

public:

    //! constructor
    /*!
      \param x endpoint of line
      \param y endpoint of line
    */
    a2dLineSegment( double x = 0, double y = 0 );

    a2dLineSegment( const a2dPoint2D& point );

    a2dLineSegment( const a2dLineSegment& other );

    virtual ~a2dLineSegment();

    //! create exact copy
    virtual a2dLineSegment* Clone();

    inline bool GetArc() const { return m_arc; }

    inline bool GetBin()  const { return m_bin; }

    inline void SetBin( bool bin ) { m_bin = bin; }

    inline bool GetBin2()  const { return m_bin2; }

    inline void SetBin2( bool bin ) { m_bin2 = bin; }

    inline bool GetArcPiece() const { return m_arcPiece; }

    inline void SetArcPiece( bool arcPiece ) { m_arcPiece = arcPiece; }

    //! calculate length
    virtual double Length( const a2dLineSegment& prev );

    a2dPoint2D GetPoint() { return a2dPoint2D( m_x, m_y ); }

    void SetPoint( a2dPoint2D pos ) { m_x = pos.m_x; m_y = pos.m_y; }

    //! Set the type of the segment
    void SetSegType( a2dSegType type ) { m_segtype = type; }

    //! get the type of the segment
    a2dSegType GetSegType() { return m_segtype; }

    virtual a2dBoundingBox GetBbox( const a2dLineSegment& prev, const a2dAffineMatrix& lworld = a2dIDENTITY_MATRIX ) const;

    //!x endpoint of line
    double m_x;

    //!y endpoint of line
    double m_y;

    //! Marker for walking over the segments
    bool m_bin: 1;

    //! Marker for walking over the segments
    bool m_bin2: 1;

    //! arc segment
    bool m_arc: 1;

    //! used when converting arc segment to small line segment, those line segment are marked like arc piece.
    //! this information can be used when reconstructing arcs after a boolean operation.
    bool m_arcPiece: 1;

    //! type of segment
    a2dSegType m_segtype: 3;

public:

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //! this is a list of all smart pointers pointing to this object
    a2dSmrtPtr<a2dLineSegment> *m_ownerlist;
    //! this is needed inside the smart pointer template code
    typedef a2dSmrtPtr<a2dLineSegment> TOwnerListClass;
    //! Make a Dump of the objects owner list to the Debug console
    /*! This function is usually called from the Debuggers watch window */
    void DumpOwners();
#endif

private:
    //!how many references to this object do exist
    int m_refcount;

    //! Call to have a new owner for this object
    /*! This function should only be called by a2dSmrtPtr

        \remark owning mean that the object calling this member needs to call Release at some time,
                in order to actually release/delete the object.

        \return The return value is the object itself, which is now owned on time extra.

        increment refcount by 1 ( use when adding a reference to this object)
    */
    a2dLineSegment* SmrtPtrOwn() { m_refcount++; return this; }

    //!To release the object, it is not longer owned by the calling object.
    /*! This function should only be called by a2dSmrtPtr
    */
    bool SmrtPtrRelease()
    {
        m_refcount--;
        wxASSERT_MSG( m_refcount >= 0, wxT( "a2dLineSegment Own/Release not matched (extra Release calls)" ) );
        if ( m_refcount <= 0 )
        {
            delete this;
            return true;
        }
        return false;
    }

private:
    friend class a2dSmrtPtrBase;
};

//! smart pointer to line segment
typedef a2dSmrtPtr<a2dLineSegment> a2dLineSegmentPtr;

#if defined(WXART2D_USINGDLL)
template class A2DARTBASEDLLEXP a2dSmrtPtr<a2dLineSegment>;
#endif

#if defined(WXART2D_USINGDLL)
// it must be after all internal template declarations
template class A2DARTBASEDLLEXP std::allocator<class a2dSmrtPtr<class a2dLineSegment> >;
template class A2DARTBASEDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dLineSegment>, std::allocator<class a2dSmrtPtr<class a2dLineSegment> > >::_Node >;
template class A2DARTBASEDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dLineSegment>, std::allocator<class a2dSmrtPtr<class a2dLineSegment> > >::_Nodeptr >;
template class A2DARTBASEDLLEXP std::list<class a2dSmrtPtr<class a2dLineSegment> >::iterator;
template class A2DARTBASEDLLEXP std::list<class a2dSmrtPtr<class a2dLineSegment> >;
template class A2DARTBASEDLLEXP a2dlist< a2dSmrtPtr<a2dLineSegment> >;
template class A2DARTBASEDLLEXP a2dSmrtPtrList<a2dLineSegment>;
#endif

#if 0
class a2dLineSegmentProperty;
typedef a2dPropertyIdTyped< a2dLineSegment, a2dLineSegmentProperty> a2dPropertyIdLineSegment;

//! property to hold a pointer to a linesegment
///*!

a2dPolyHandle::PROPID_linesegment.SetPropertyObjectToObject(
    handle, new a2dLineSegmentProperty( a2dPolyHandle::PROPID_linesegment, seg ) );

a2dLineSegment* segment = a2dHandle::PROPID_linesegment.GetPropertyValuePtr( handle );

\ingroup property
///
class a2dLineSegmentProperty: public a2dNamedProperty
{
public:

    a2dLineSegmentProperty();

    a2dLineSegmentProperty( const a2dPropertyIdLineSegment& id, a2dLineSegment* segment );

    a2dLineSegmentProperty( const a2dLineSegmentProperty& other );

    virtual a2dNamedProperty* Clone( a2dObject::CloneOptions options ) const;

    virtual void Assign( const a2dNamedProperty& other );

    virtual ~a2dLineSegmentProperty();

    //! Construct a new property object from a string
    //! If this is not appropriate, this may return NULL
    static a2dLineSegmentProperty* CreatePropertyFromString( const a2dPropertyIdLineSegment& id, const wxString& value );

    a2dLineSegment* GetValuePtr() { return m_segment; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dLineSegmentProperty )

protected:

    a2dLineSegment* m_segment;
};
#endif

//! Arc Segment in a2dVertexList
/*! Create an circular Arc segment.
    From the previous segment to this segment position, create an arc
    passing through a thrid point.
    The Third point defines the Arc segment going clockwise or anticlockwise from begin to end point.

    \ingroup vpath
*/
class A2DARTBASEDLLEXP a2dArcSegment : public a2dLineSegment
{
public:

    //! create arc segment
    /*!
        Begin point of arc is position/endpoint of previous segment
        \param x1 x of arc endpoint
        \param y1 y of arc endpoint
        \param x2 x of arc midpoint
        \param y2 y of arc midpoint
    */
    a2dArcSegment( double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0 );

    //! create arc segment
    /*!
        Begin point of arc is position/endpoint of previous segment
        \param prev previous segment
        \param xc x of arc center
        \param yc y of arc center
        \param angle angle of arc in degrees relative from the previous segment endpoint (negatif for clockwise)
    */
    a2dArcSegment( const a2dLineSegment& prev, double xc, double yc, double angle );

    //! create arc segment
    /*!
        Begin point of arc is position/endpoint of previous segment
        \param prev previous segment
        \param xc x of arc center
        \param yc y of arc center
        \param x1 x of arc endpoint
        \param y1 y of arc endpoint
        \param x2 x (xc,yc) ( x2,y2) define a line which the arc will cross
        \param y2 y (xc,yc) ( x2,y2) define a line which the arc will cross

        \remark radius is defined by prev segment endpoint and (xc,yc), x1,y1 is adjusted to fit radius.
    */
    a2dArcSegment( const a2dLineSegment& prev, double xc, double yc, double x1, double y1, double x2, double y2 );

    //! copy constructor
    a2dArcSegment( const a2dArcSegment& other );

    //! destructor
    ~a2dArcSegment();

    //! create exact copy
    virtual a2dLineSegment* Clone();

    //! Calculation of center for the Arc
    /*! output :
        \param prev the previous segment, to get start point of arc.
        \param radius radius of the circle calculated
        \param center_x x of the center calculated
        \param center_y y of the center calculated
        \param beginrad calculated starting angle in radians
        \param midrad calculated middle angle in radians
        \param endrad calculated end angle in radians
        \param phit total calculated in radians (AntiClockwise positif, else negatif )

        \return true if the arc is indeed an Arc if a straight line return false.
    */
    bool CalcR( const a2dLineSegment& prev, double& radius, double& center_x, double& center_y, double& beginrad, double& midrad, double& endrad, double& phit, double marge = a2dACCUR ) const;

    //! calculate length
    virtual double Length( const a2dLineSegment& prev );

    //! calculate new mid point bsed on prev segment radius and center.
    /*!
           \param prev the previous segment, to get start point of arc.
           \param radius radius of the circle calculated
           \param center_x x of the center calculated
           \param center_y y of the center calculated
           \param clockwise if true a clockwise arc segment is assumed for creating the mid point
    */
    void CalcMidPoint( const a2dLineSegment& prev, double center_x, double center_y, double radius, bool clockwise );

    //! get middle on arc segment between end and start
    a2dPoint2D GetMidPoint() const { return a2dPoint2D( m_x2, m_y2 ); }

    //! set middle point of arc segment
    /*!
        \param prev the previous segment, to get start point of arc.
        \param xm x of middle point
        \param ym y of middle point
    */
    void SetMidPoint( const a2dLineSegment& prev, double xm, double ym );

    //! Get origin X of arc
    double GetOx( const a2dLineSegment& prev ) const;

    //! Get origin Y of arc
    double GetOy( const a2dLineSegment& prev ) const;

    //! Get origin of arc
    a2dPoint2D GetOrigin( const a2dLineSegment& prev ) const;

    //! Get bounding box of arc
    a2dBoundingBox GetBbox( const a2dLineSegment& prev, const a2dAffineMatrix& lworld = a2dIDENTITY_MATRIX ) const;

    //! x2 x of arc midpoint
    double m_x2;
    //! y2 y of arc midpoint
    double m_y2;
};

#include <vector>
#include <wx/listimpl.cpp>

//! array of polygon vertexes a2dLineSegment
/*!
 \ingroup canvasobject
*/

class A2DARTBASEDLLEXP a2dVertexList;
class A2DARTBASEDLLEXP a2dVpath;

//! vertex array of line and arc segments.
/*!
    Holds a wxArray of a2dLineSegment objects.
    Drawing the sequence of segments, represents the form of the shape.
    Shape depends on the type of segment and style of that segment.
 There are two type, straight line segments and arc segments
*/
class A2DARTBASEDLLEXP a2dVertexArray : public std::vector<a2dLineSegmentPtr>
{
public:

    //! constructor
    a2dVertexArray();

    //! constructor
    a2dVertexArray( const a2dVertexArray& other );

    a2dVertexArray( const a2dVertexList& other );

    //! destructor
    ~a2dVertexArray();

    //! operator =
    a2dVertexArray& operator=( const a2dVertexArray& other );

    a2dVertexArray& operator=( const a2dVertexList& other );

    //! get the previous segment as a polygon ( GetLast() is no previous )
    a2dLineSegmentPtr GetPreviousAround( wxUint32 index ) const;

    //! get the next segment as a polygon ( GetFirst() is no next )
    a2dLineSegmentPtr GetNextAround( wxUint32 index ) const;

    inline a2dLineSegmentPtr Item( wxUint32 index ) { return this->operator[]( index ); }

    inline a2dLineSegmentPtr Item( wxUint32 index ) const { return this->operator[]( index ); }

    void RemoveAt( size_t index );

    void Insert( a2dLineSegment* segment, size_t index );

    //! calculate length of path
    double Length() const;

    //! add point to end or begin
    void AddPoint( const a2dPoint2D& point, bool atEnd = true );

    //! add point to end or begin
    void AddPoint( double x, double y, bool atEnd = true );

    //! sets a point of a segment and adjusts arc it midpoints.
    void SetPointAdjustArcs( unsigned int n, double x, double y, bool polygon );

    //! sets a point of a segment and adjusts arc it midpoints.
    void SetPointAdjustArcs( a2dLineSegmentPtr seg, double x, double y, bool polygon );

    //! calculate the area of simple polygons (not selfintersecting)
    //! coordinates may be negatif
    double CalcArea() const;

    //! Convert complex segments to line segments.
    void ConvertToLines( double aberation = 0 );

    //! transform all segments with given matrix
    /*! \remark complex segments will be broken down to lines. */
    void Transform( const a2dAffineMatrix& world );

    //! return true if there are a2dArcSegment segments.
    bool HasArcs() const;

    //! return a boundingbox of a transformed vertexarray
    a2dBoundingBox GetBbox( const a2dAffineMatrix& lworld = a2dIDENTITY_MATRIX );

    //! create a contour around polygon/polyline
    a2dVertexArray* Contour( double distance, a2dPATH_END_TYPE pathtype );

    //!Spline conversion for polygon.
    a2dVertexArray* ConvertSplinedPolygon( double Aber ) const;

    //!Spline conversion for polyline.
    a2dVertexArray* ConvertSplinedPolyline( double Aber ) const;

    //! return converted vector Vpath, arc segments stay intact if arc is true
    a2dVpath* ConvertToVpath( bool arc, bool closed = false );

    //! extensive hittesting on vertex list seen as polygon.
    /*!
        \param ptest point to test against polygon.
        \param margin point with this margin around.
    */
    a2dHit HitTestPolygon( const a2dPoint2D& ptest, double margin );

    //! extensive hittesting on vertex list seen as polyline.
    /*!
        \param ptest point to test against polyline.
        \param margin point with this margin around.
    */
    a2dHit HitTestPolyline( const a2dPoint2D& ptest, double margin );

    //! line segments ( not arcs ) with same point are removed
    bool RemoveRedundant( bool polygon );

    //! line segments ( not arcs ) with same point are returned
    a2dVertexList* GetRedundant( bool polygon, double smallest = 0 );
};

//! vertex list of line and arc segments.
/*!
    Holds a wxList of a2dLineSegment objects.
    Drawing the sequence of segments, represents the form of the shape.
    Shape depends on the type of segment and style of that segment.
 There are two type, straight line segments and arc segments
*/
class A2DARTBASEDLLEXP a2dVertexList : public  a2dSmrtPtrList< a2dLineSegment >
{
public:
    a2dVertexList();

    a2dVertexList( const a2dVertexList& other );

    a2dVertexList( const a2dVertexArray& other );

    ~a2dVertexList();

    a2dVertexList& operator=( const a2dVertexList& other );

    a2dVertexList& operator=( const a2dVertexArray& other );

    //! return true if there are a2dArcSegment segments.
    bool HasArcs() const;

    //! get the previous segment as a polygon ( --end() is no previous )
    a2dVertexList::iterator GetPreviousAround( a2dVertexList::iterator iter );

    //! get the next segment as a polygon ( begin() is no next )
    a2dVertexList::iterator GetNextAround( a2dVertexList::iterator iter );

    //! get the previous segment as a polygon ( --end() is no previous )
    a2dVertexList::const_iterator GetPreviousAround( a2dVertexList::const_iterator iter ) const;

    //! get the next segment as a polygon ( begin() is no next )
    a2dVertexList::const_iterator GetNextAround( a2dVertexList::const_iterator iter ) const;

    //! make the segmenet where iter point to the beginning of the list and shift the rest
    void MakeBegin( a2dVertexList::iterator iter );

    //! calculate length of path
    double Length();

    //! Find the index of a specific object
    int IndexOf( a2dLineSegment* object ) const;

    //! insert before segment with index given
    void Insert( unsigned int index,  a2dLineSegmentPtr segin );

    //! sets a point of a segment and adjusts arc it midpoints.
    void SetPointAdjustArcs( unsigned int n, double x, double y, bool polygon );

    void SetPointAdjustArcs( a2dLineSegmentPtr segin, double x, double y, bool polygon );

    //! Convert complex segments to line segments.
    void ConvertToLines( double aberation = 0 );

    //! variation on ConvertToLines, this function converts an arc into segments were
    //! the number of segments is determined by dphi (maximum angle of a piece) and
    //! minseg (the arc will contain a minimum of minseg segments).
    //! e.g. if you specify dphi on 10 a circle will be converted into 36 segments,
    //!      however if you specify minseg as 40, it will be converted to 40 segments.
    void ConvertToLines( double dphi, int minseg );

    //! calculate the area of simple polygons (not selfintersecting)
    //! coordinates may be negatif
    double CalcArea() const;

    //! determines the direction of the polygon, all polygons must be in a
    //! clockwise order to avoid conflics with certain algortihms. When
    //! the polygon is in a non-clockwise order the area of the polygon is in
    //! a clockwise order.
    //! returns true if the polygon is in clockwise order
    bool DirectionIsClockWise();

    bool CheckForOneCircle( a2dPoint2D& middle, double& radius );

    //! The point on the polyline which form an arc meeting the constraints, will be replaced
    //! by an Arc segment.
    //! The Arc segment will be with the following 4 coordinates:
    //!                     - start point
    //!                     - end  point
    //!                     - mid point (a point half way on the arc between start and end)
    //!                     - center point
    //! In principle only 3 point are need to define a circle, but if start and end point are the same,
    //! the center point will be needed to.
    //!
    //! \param iter start of first point
    //! \param n ( number of point2 to replace with arc )
    //! \param center_x center_x (center point of the ARC)
    //! \param center_y center_y (center point of the ARC)
    void InsertArc( a2dVertexList::iterator& iter, int n, double center_x, double center_y );

    //! De punten van de polygon worden getest of ze converteerd kunnen
    //! worden naar ARC's.
    //! Conversie vind plaats als;  aberatie < aber
    //!             Rmin < radius < Rmax
    void ConvertPolylineToArc( double aber, double Rmin, double Rmax );

    void ConvertPolygonToArc( double aber, double Rmin, double Rmax );

    //! a row of point (minimum 4 ) will be tested to see if its an arc.
    /*!
        3 options:
         - aber > 0 arc should be within this margin to all points
         - aber = 0 arc points should be exact on the arc. The largets distance of segments to arc
         is not important.
         - aber < 0 combines the first two options.

         Abberation means maximum distance along a segment towards the perfect arc.

         \param iter start testing here
         \param aber see above
         \param Rmin (minimum radius)
         \param Rmax (maximum radius)
         \param center_p_old (centerpunt van circelboog)

         \return number of points found to be an arc
    */
    int TestArc( a2dVertexList::iterator& iter, double aber, double Rmin, double Rmax , a2dPoint2D& center_p_old );

    //! return a boundingbox of a transformed vertexarray
    a2dBoundingBox GetBbox( const a2dAffineMatrix& lworld = a2dIDENTITY_MATRIX );

    //! transform all segments with given matrix
    /*! \remark complex segments will be broken down to lines. */
    void Transform( const a2dAffineMatrix& world );

    //!Spline conversion for polygon.
    a2dVertexList* ConvertSplinedPolygon( double Aber ) const;

    //!Spline conversion for polyline.
    a2dVertexList* ConvertSplinedPolyline( double Aber ) const;

    void ConvertIntoSplinedPolygon( double Aber );

    void ConvertIntoSplinedPolyline( double Aber );

    //! create a contour around polygon/polyline
    a2dVertexList* ConvertToContour( double distance, a2dPATH_END_TYPE pathtype, bool asPolygon = false );

    //! return converted vector Vpath, arc segments stay intact if arc is true
    a2dVpath* ConvertToVpath( bool arc, bool closed = false );

    //! create a contour around polygon/polyline
    void Contour( double distance, a2dPATH_END_TYPE pathtype, bool asPolygon = false );

    //! create a polygon following the contour of this polygon, only now vertexes at a given distance.
    /*!
        \param distance of vertexes on created polygon
        \param asPolygon see the vertexlist as a polygon or polyline
        \param atVertex if passing a vertex in the original polygon, if true, always create a point on that vertex in the result polygon
        \param lastPointCloseToFirst if true, if last created point, is to close to first (< distance), it is not added.
        \param alongLine if true distance is calculated using the segments of the original polygon, else the real distance between added points is used
    */
    a2dVertexList*  ConvertPointsAtDistance( double distance, bool asPolygon, bool atVertex = false, bool lastPointCloseToFirst = true, bool alongLine = false ) const;

    a2dPoint2D DistancePoint( a2dPoint2D startPoint, a2dLineSegment* seg, a2dLineSegment* segn, double distance, double& distanceMade, bool& atEnd ) const;

    bool DistancePoint2( a2dPoint2D startPoint, a2dPoint2D& pointAtDistance, a2dLineSegment* seg, a2dLineSegment* segn, double distance, double& distanceMade ) const;

    //! add point to end or begin
    void AddPoint( const a2dPoint2D& point, bool atEnd = true );

    //! add point to end or begin
    void AddPoint( double x, double y, bool atEnd = true );

    //! create an arc and add it to the graph
    /*!
        \param center of circle
        \param begin point of arc
        \param end point of arc
        \param radius of arc
        \param clock if true clockwise
        \param aber aberation for generating the segments
        \param addAtFront where to add this segment
    */
    void CreateArc( const a2dPoint2D& center, const a2dPoint2D& begin, const a2dPoint2D& end, double radius, bool clock, double aber, bool addAtFront );

    //! create an arc and add it to the graph
    /*!
        \param center of circle
        \param incoming last segment starting this segment
        \param end point of arc
        \param radius of arc
        \param aber aberation for generating the segments
        \param addAtFront where to add this segment
    */
    void CreateArc( const a2dPoint2D& center, const a2dLine& incoming,  const a2dPoint2D& end, double radius, double aber, bool addAtFront );

    //! create a contour segements at a distance, using two segment
    /*!
        The angle of the two connected segments in combination with the factor is used to decide
        if the corner is rounded or straight.

        \param currentline
        \param nextline
        \param factor
        \param addAtFront where to add this segment
    */
    void OffsetContour_rounded( const a2dLine& currentline, const a2dLine& nextline, double factor, bool addAtFront );

    //! line segments ( not arcs ) with same point are removed
    bool RemoveRedundant( bool polygon, double smallest = 0 );

    //! line segments ( not arcs ) with same point are returned
    a2dVertexList* GetRedundant( bool polygon, double smallest = 0 );

    //! extensive hittesting on vertex list seen as polygon.
    /*!
        \param ptest point to test against polygon.
        \param margin point with this margin around.
    */
    a2dHit HitTestPolygon( const a2dPoint2D& ptest, double margin );

    //! extensive hittesting on vertex list seen as polyline.
    /*!
        \param ptest point to test against polyline.
        \param margin point with this margin around.
    */
    a2dHit HitTestPolyline( const a2dPoint2D& ptest, double margin );

private:
    //!how many references to this object do exist
    int m_refcount;

    //! Call to have a new owner for this object
    /*! This function should only be called by a2dSmrtPtr

        \remark owning mean that the object calling this member needs to call Release at some time,
                in order to actually release/delete the object.

        \return The return value is the object itself, which is now owned on time extra.

        increment refcount by 1 ( use when adding a reference to this object)
    */
    a2dVertexList* SmrtPtrOwn() { m_refcount++; return this; }

    //!To release the object, it is not longer owned by the calling object.
    /*! This function should only be called by a2dSmrtPtr
    */
    bool SmrtPtrRelease()
    {
        m_refcount--;
        wxASSERT_MSG( m_refcount >= 0, wxT( "a2dLineSegment Own/Release not matched (extra Release calls)" ) );
        if ( m_refcount <= 0 )
        {
            delete this;
            return true;
        }
        return false;
    }

private:
    friend class a2dSmrtPtrBase;
};

typedef a2dSmrtPtr< a2dVertexList > a2dVertexListPtr;

typedef a2dVertexList::iterator a2dVertexListIter;

typedef a2dSmrtPtrList< a2dVertexList > a2dListOfa2dVertexList;

//! how do we move to the point of the segment
/*!
    \ingroup vpath
*/
enum a2dPATHSEG
{
    a2dPATHSEG_MOVETO,  /*!< Move to point */
    a2dPATHSEG_LINETO,  /*!< Line to point */
    a2dPATHSEG_LINETO_NOSTROKE, /*!< Line to point without stroke (if not closed as polygon, same as moveto) */
    a2dPATHSEG_QBCURVETO, /*!< Quadratic Bezier Curve */
    a2dPATHSEG_QBCURVETO_NOSTROKE, /*!< Quadratic Bezier Curve to point without stroke (if not closed as polygon, same as moveto) */
    a2dPATHSEG_CBCURVETO, /*!< Cubic Bezier Curve */
    a2dPATHSEG_CBCURVETO_NOSTROKE, /*!< Cubic Bezier Curve to point without stroke (if not closed as polygon, same as moveto) */
    a2dPATHSEG_ARCTO, /*!< Arc */
    a2dPATHSEG_ARCTO_NOSTROKE /*!< arc to point without stroke (if not closed as polygon, same as moveto) */
};

//! end of a segment type
/*!
    \ingroup vpath
*/
enum a2dPATHSEG_END
{
    a2dPATHSEG_END_OPEN,  /*!< Open end */
    a2dPATHSEG_END_CLOSED,  /*!< Closed to last a2dPATHSEG_MOVETO segment */
    a2dPATHSEG_END_CLOSED_NOSTROKE,  /*!<  Closed to last a2dPATHSEG_MOVETO segment with No Stroke*/
};

//! Normal straight line segment in a2dVpath
/*! baseclass for segments in a a2dVpath
Every segment type in a a2dVpath has this class as Baseclass.
It has/maintains the end position of a segment.
But for the first point it is a2dVpathSegment to give the begin point too.

    \ingroup vpath
*/
class A2DARTBASEDLLEXP a2dVpathSegment
{
public:

    //! constructor
    /*!
      \param x endpoint of line
      \param y endpoint of line
      \param type line type ( e.g  line to, move to, no stroking )
      \param close endpoint closes the shape.
    */
    a2dVpathSegment( double x, double y, a2dPATHSEG type = a2dPATHSEG_LINETO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN );

    //! constructor
    a2dVpathSegment( const a2dVpathSegment& other );

    //! destructor
    virtual ~a2dVpathSegment();

    //! create exact copy
    virtual a2dVpathSegment* Clone();

    //! easy way to test type of segment
    inline a2dPATHSEG GetType() const { return m_type; }

    //! used in processing
    inline bool GetBin()  const { return m_bin; }

    //! used in processing
    inline void SetBin( bool bin ) { m_bin = bin; }

    inline bool GetArcPiece() const { return m_arcPiece; }

    inline void SetArcPiece( bool arcPiece ) { m_arcPiece = arcPiece; }

    //! is this segment the closing a part since the last move
    inline a2dPATHSEG_END GetClose() const { return m_close; }

    //! set this segment is closing a part since the last move
    inline void SetClose( a2dPATHSEG_END close ) { m_close = close; }

    //! calculate length
    virtual double Length( a2dSmrtPtr<a2dVpathSegment> prev );

    //!x endpoint of line
    double m_x1;

    //!y endpoint of line
    double m_y1;

    //! Marker for walking over the segments
    bool    m_bin    : 1;

    //! used when converting arc segment to small line segment, those line segment are marked like arc piece.
    //! this information can be used when reconstructing arcs after a boolean operation.
    bool m_arcPiece : 1 ;

    //! easy way to test type of segment
    a2dPATHSEG  m_type   : 5;

    //! is the path closing here or not
    a2dPATHSEG_END  m_close  : 3;

private:
    //!how many references to this object do exist
    int m_refcount;

    //! Call to have a new owner for this object
    /*! This function should only be called by a2dSmrtPtr

        \remark owning mean that the object calling this member needs to call Release at some time,
                in order to actually release/delete the object.

        \return The return value is the object itself, which is now owned on time extra.

        increment refcount by 1 ( use when adding a reference to this object)
    */
    a2dVpathSegment* SmrtPtrOwn() { m_refcount++; return this; }

    //!To release the object, it is not longer owned by the calling object.
    /*! This function should only be called by a2dSmrtPtr
    */
    bool SmrtPtrRelease()
    {
        m_refcount--;
        wxASSERT_MSG( m_refcount >= 0, wxT( "a2dVpathSegment Own/Release not matched (extra Release calls)" ) );
        if ( m_refcount <= 0 )
        {
            delete this;
            return true;
        }
        return false;
    }

private:
    friend class a2dSmrtPtrBase;
};

//! smart pointer to path segment
typedef a2dSmrtPtr<a2dVpathSegment> a2dVpathSegmentPtr;

//! Quadratic Bezier curve
/*! Create a curved segment based on qaudratic Bezier spline
    let P0 be end point of previous segment
    let P1 be inbetween point of this segment (m_x2, m_y2)
    let P2 be end point of this segment (m_x1, m_y1)

    Then the formula describing points on the curve are:

    For ( 0 <= t <= 1 )
        Point(t) = (1-t)^2*P0 + 2*t*(1-t)*P1 + t^2*P2

    \ingroup vpath
*/
class A2DARTBASEDLLEXP a2dVpathQBCurveSegment : public a2dVpathSegment
{
public:

    //! constructor
    /*!
      \param x1 endpoint of curve
      \param y1 endpoint of curve
      \param x2 first control point coming from previous segment in path
      \param y2 first control point coming from previous segment in path
      \param type draw or move towards point
      \param close if true close this path with the last move command/segment
    */
    a2dVpathQBCurveSegment( double x1, double y1, double x2, double y2, a2dPATHSEG type = a2dPATHSEG_QBCURVETO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN  );

    //! constructor
    /*!
      \param prev previous segment
      \param x1 endpoint of curve
      \param y1 endpoint of curve
      \param type draw or move towards point
      \param close if true close this path with the last move command/segment
    */
    a2dVpathQBCurveSegment(  a2dVpathSegmentPtr prev, double x1, double y1, a2dPATHSEG type = a2dPATHSEG_QBCURVETO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN );

    //! copy constructor
    a2dVpathQBCurveSegment( const a2dVpathQBCurveSegment& other );

    //! destructor
    ~a2dVpathQBCurveSegment();

    //! create exact copy
    virtual a2dVpathSegment* Clone();

    //! calculate length
    virtual double Length( a2dVpathSegmentPtr prev );

    //! calculate position at t, used for length
    void PositionAt( a2dVpathSegmentPtr prev, double t, double& xt, double& yt );

    //! control point
    double m_x2;
    //! control point
    double m_y2;

};

//! Cubic Bezier curve
/*! Create a curved segment based on Cubic Bezier spline
    let P0 be end point of previous segment
    let P1 be inbetween point of this segment (m_x2, m_y2)
    let P2 be second inbetween point of this segment (m_x3, m_y3)
    let P3 be end point of this segment (m_x1, m_y1)

    Then the formula describing points on the curve are:

    For ( 0 <= t <= 1 )
        Point(t) = (1-t)^3*P0 + 3*t*(1-t)^2*P1 + 3*t^2*(1-t)*P2 + t^3*P3

    \ingroup vpath
*/
class A2DARTBASEDLLEXP a2dVpathCBCurveSegment : public a2dVpathSegment
{
public:

    //! constructor
    /*!
      \param x1 endpoint of curve
      \param y1 endpoint of curve
      \param x2 first control point coming from previous segment in path
      \param y2 first control point coming from previous segment in path
      \param x3 second control point coming from previous segment in path
      \param y3 second control point coming from previous segment in path
      \param type draw or move towards point
      \param close if true close this path with the last move command/segment
    */
    a2dVpathCBCurveSegment( double x1, double y1, double x2, double y2, double x3, double y3, a2dPATHSEG type = a2dPATHSEG_CBCURVETO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN );

    //! constructor
    /*!
      \param prev previous segment
      \param x1 endpoint of curve
      \param y1 endpoint of curve
      \param x3 second control point coming from previous segment in path
      \param y3 second control point coming from previous segment in path
      \param type draw or move towards point
      \param close if true close this path with the last move command/segment

      \remark x2 first control point is mirror of second control point of previous segment in path, if Cubic else last endpoint
      \remark y2 first control point is mirror of second control point of previous segment in path, if Cubic else last endpoint
    */
    a2dVpathCBCurveSegment( a2dVpathSegmentPtr prev, double x1, double y1, double x3, double y3, a2dPATHSEG type = a2dPATHSEG_CBCURVETO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN );

    //! copy constructor
    a2dVpathCBCurveSegment( const a2dVpathCBCurveSegment& other );

    //! destructor
    ~a2dVpathCBCurveSegment();

    //! create exact copy
    virtual a2dVpathSegment* Clone();

    //! calculate length
    virtual double Length( a2dVpathSegmentPtr prev );

    //! calculate position at t, used for length
    void PositionAt( a2dVpathSegmentPtr prev, double t, double& xt, double& yt );

    //! control point 1
    double m_x2;
    //! control point 1
    double m_y2;

    //! control point 2
    double m_x3;
    //! control point 2
    double m_y3;

};

//! Arc Segment
/*! Create an circular Arc segment.
    From the previous segment to this segment position, create an arc
    passing through a thrid point.
    The Third point defines the Arc segment going clockwise or anticlockwise from begin to end point.

    \ingroup vpath
*/
class A2DARTBASEDLLEXP a2dVpathArcSegment : public a2dVpathSegment
{
public:

    //! create arc segment
    /*!
        Begin point of arc is position/endpoint of previous segment
        \param x1 x of arc endpoint
        \param y1 y of arc endpoint
        \param x2 x of arc midpoint
        \param y2 y of arc midpoint
        \param type draw or move towards point
        \param close if true close this path with the last move command/segment
    */
    a2dVpathArcSegment( double x1, double y1, double x2, double y2, a2dPATHSEG type = a2dPATHSEG_ARCTO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN );

    //! create arc segment
    /*!
        Begin point of arc is position/endpoint of previous segment

        \param prev previous segment
        \param xc x of arc center
        \param yc y of arc center
        \param angle angle of arc in degrees relative from the previous segment endpoint (negatif for clockwise)
        \param type draw or move towards point
        \param close if true close this path with the last move command/segment
    */
    a2dVpathArcSegment( a2dVpathSegmentPtr prev, double xc, double yc, double angle, a2dPATHSEG type = a2dPATHSEG_ARCTO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN );

    //! create arc segment
    /*!
        Begin point of arc is position/endpoint of previous segment
        \param prev previous segment
        \param xc x of arc center
        \param yc y of arc center
        \param x1 x of arc endpoint
        \param y1 y of arc endpoint
        \param x2 x (xc,yc) ( x2,y2) define a line which the arc will cross
        \param y2 y (xc,yc) ( x2,y2) define a line which the arc will cross
        \param type draw or move towards point
        \param close if true close this path with the last move command/segment

        \remark radius is defined by prev segment endpoint and (xc,yc), x1,y1 is adjusted to fit radius.
    */
    a2dVpathArcSegment( a2dVpathSegmentPtr prev, double xc, double yc, double x1, double y1, double x2, double y2, a2dPATHSEG type = a2dPATHSEG_ARCTO, a2dPATHSEG_END close = a2dPATHSEG_END_OPEN );

    //! constructor
    a2dVpathArcSegment( const a2dVpathArcSegment& other );

    //! destructor
    ~a2dVpathArcSegment();

    //! create exact copy
    virtual a2dVpathSegment* Clone();

    //! Calculation of center for the Arc
    /*! output :
        \param prev the previous segment, to get start point of arc.
        \param radius radius of the circle calculated
        \param center_x x of the center calculated
        \param center_y y of the center calculated
        \param beginrad calculated starting angle in radians
        \param midrad calculated middle angle in radians
        \param endrad calculated end angle in radians
        \param phit total calculated in radians (AntiClockwise positif, else negatif )

        \return true if the arc is indeed an Arc if a straight line return false.
    */
    bool CalcR( a2dVpathSegmentPtr prev, double& radius, double& center_x, double& center_y, double& beginrad, double& midrad, double& endrad, double& phit, double marge = a2dACCUR );

    //! calculate length
    virtual double Length( a2dVpathSegmentPtr prev );

    //! second control point
    double m_x2;
    //! second control point
    double m_y2;
};


//!Vector Path
/*!
    Holds a wxArray of a2dVpathSegment objects.
    Drawing the sequence of segments, represents the form of the shape.
    Shape depends on the type of segment and style of that segment.

    With Vpath very complex shapes can be generated, also disjoint.
    e.g. polygons with holes, donuts.

    a2dVpathSegment is the baseclass for several types of segments.

    \sa a2dVpathQBCurveSegment
    \sa a2dVpathCBCurveSegment
    \sa a2dVpathArcSegment

    \remark do use Shrink() after filling a a2dVpath, to reduce memory usage

    \ingroup vpath
*/
class A2DARTBASEDLLEXP a2dVpath : public std::vector<a2dVpathSegmentPtr>
{
public:

    //constructor for a vector path array
    a2dVpath();

    //constructor for a vector path array, using as input a a2dVertexArray
    a2dVpath( a2dVertexArray& vertexArray, bool moveToFirst = true , bool closeLast = false );

    //constructor for a vector path array, using as input a a2dVertexList
    a2dVpath( a2dVertexList& vertexList, bool moveToFirst = true , bool closeLast = false );

    //! destructor
    ~a2dVpath();

    //! operator =
    a2dVpath& operator=( const a2dVpath& other );

    //! calculate length of path, assuming continues path.
    double Length();

    //! test if closed polygon ( a2dPATHSEG_MOVETO, a2dPATHSEG_LINETO, a2dPATHSEG_ARCTO )
    bool IsPolygon( bool allowArc = true );

    //! test if polyline ( a2dPATHSEG_MOVETO, a2dPATHSEG_LINETO, a2dPATHSEG_ARCTO )
    bool IsPolyline( bool allowArc = true );

    //! add a segment
    void Add( a2dVpathSegment* seg ) { push_back( seg ); }

    //! add a vertexArray to an existing path
    void Add( a2dVertexArray& vertexArray, bool moveToFirst = true , bool closeLast = false );

    //! add a vertexlist to an existing path
    void Add( a2dVertexList& vertexList, bool moveToFirst = true , bool closeLast = false );

    inline a2dVpathSegmentPtr Item( wxUint32 index ) { return this->operator[]( index ); }

    inline a2dVpathSegmentPtr Item( wxUint32 index ) const { return this->operator[]( index ); }

    void RemoveAt( size_t index );

    void Insert( a2dVpathSegment* segment, size_t index );

    //! add a MoveTo command to the path
    /*!
        \param x point to move to
        \param y point to move to
    */
    void MoveTo( double x, double y );

    //! add a LineTo command to the path
    /*!
        \param x endpoint of line
        \param y endpoint of line
        \param withStroke Do stroke or not this line segment
    */
    void LineTo( double x, double y, bool withStroke = true );

    //! add a quadratic bezier segment to the path
    /*!
        \param x1 endpoint of curve
        \param y1 endpoint of curve
        \param x2 first control point coming from previous segment in path
        \param y2 first control point coming from previous segment in path
        \param withStroke Do stroke or not this line segment
    */
    void QBCurveTo( double x1, double y1, double x2, double y2, bool withStroke = true );

    //! add a quadratic bezier segment to the path, using the previous segment.
    /*!
        \param x1 endpoint of curve
        \param y1 endpoint of curve
        \param withStroke Do stroke or not this line segment
    */
    void QBCurveTo( double x1, double y1, bool withStroke = true );

    //! add a quadratic bezier segment to the path
    /*!
        \param x1 endpoint of curve
        \param y1 endpoint of curve
        \param x2 first control point coming from previous segment in path
        \param y2 first control point coming from previous segment in path
        \param x3 second control point coming from previous segment in path
        \param y3 second control point coming from previous segment in path
        \param withStroke Do stroke or not this line segment
    */
    void CBCurveTo( double x1, double y1, double x2, double y2, double x3, double y3, bool withStroke = true );

    //! add a quadratic bezier segment to the path, using the previous segment.
    /*!
        \param x1 endpoint of curve
        \param y1 endpoint of curve
        \param x3 second control point coming from previous segment in path
        \param y3 second control point coming from previous segment in path
        \param withStroke Do stroke or not this line segment

        \remark x2 first control point is mirror of second control point of previous segment in path, if Cubic else last endpoint
        \remark y2 first control point is mirror of second control point of previous segment in path, if Cubic else last endpoint
    */
    void CBCurveTo( double x1, double y1, double x3, double y3, bool withStroke = true );

    //! add an arc segment to the path
    /*!
    Begin point of arc is position/endpoint of previous segment
        \param x1 x of arc endpoint
        \param y1 y of arc endpoint
        \param x2 x of arc midpoint
        \param y2 y of arc midpoint
        \param withStroke Do stroke or not this line segment
    */
    void ArcTo( double x1, double y1, double x2, double y2, bool withStroke = true );

    //! add an arc segment to the path, using the previous segment
    /*!
    Begin point of arc is position/endpoint of previous segment

        \param xc x of arc center
        \param yc y of arc center
        \param angle angle of arc in degrees relative from the previous segment endpoint (negatif for clockwise)
        \param withStroke Do stroke or not this line segment
    */
    void ArcTo( double xc, double yc, double angle, bool withStroke = true );

    //! add an arc segment to the path, using the previous segment
    /*!
    Begin point of arc is position/endpoint of previous segment
        \param xc x of arc center
        \param yc y of arc center
        \param x1 x of arc endpoint
        \param y1 y of arc endpoint
        \param x2 x (xc,yc) ( x2,y2) define a line which the arc will cross
        \param y2 y (xc,yc) ( x2,y2) define a line which the arc will cross
        \param withStroke Do stroke or not this line segment

    \remark radius is defined by prev segment endpoint and (xc,yc), x1,y1 is adjusted to fit radius.
    */
    void ArcTo( double xc, double yc, double x1, double y1, double x2, double y2, bool withStroke = true );

    //! Closing the path as a filled area
    /*!
        The part sinve the last move is close to that move vertex.

        \param withStroke Do stroke or not this line segment
    */
    void Close( bool withStroke = true );

    //! Convert complex segments to line segments.
    void ConvertToLines( double aberation = 0 );

    //! Convert to a a2dVertexList, taking and assuming it has only closed contours in path.
    //! Replacing non line (and arc segments if arc is true) with line segments.
    void ConvertToPolygon( a2dListOfa2dVertexList& addTo, bool arc = true );

    //! create an offset contour at distance
    /*!
        From all path parts a version is created
        that surrounds the original part at the given distance.
    */
    void Contour( double distance, a2dPATH_END_TYPE pathtype );

    //! transform all segments with given matrix
    /*!
        \remark complex segments will be broken down to lines.
    */
    void Transform( const a2dAffineMatrix& world );

    //! return a boundingbox of a transformed a2dVpath
    a2dBoundingBox GetBbox( const a2dAffineMatrix& lworld = a2dIDENTITY_MATRIX );


private:

    //! Create a path around this path, at a distance
    /*!
        This is used to create/draw paths with a thickness.
        The output is a polygon shape, which can be drawn filled as a polygon.
    */
    void SingleContour( a2dVpath& converted, unsigned int start, unsigned int segments, double distance, a2dPATH_END_TYPE pathtype );
};

//! Calculate the square distance between two points
inline double ClclDistSqrPntPnt( const a2dPoint2D& a, const a2dPoint2D& b )
{
    double dx = a.m_x - b.m_x;
    double dy = a.m_y - b.m_y;

    return dx * dx + dy * dy;
}

//! Calculate the square distance between a point and a line
/*! This returns DBL_MAX if the point is beyond the edges */
extern A2DARTBASEDLLEXP double ClclDistSqrPntLine( const a2dPoint2D& p, const a2dPoint2D& p1, const a2dPoint2D& p2 );

//! Calculation of center for the Arc
/*!
    \param begin_x x start point of arc.
    \param begin_y y start point of arc.
    \param middle_x x middle point of arc.
    \param middle_y y middle point of arc.
    \param end_x x end point of arc.
    \param end_y y end point of arc.
    \param radius radius of the circle calculated
    \param center_p the center calculated

    \return true if the arc is indeed an Arc if a straight line return false.
*/
bool CalcR( double begin_x, double begin_y, double middle_x, double middle_y, double end_x, double end_y,
            double& radius, a2dPoint2D& center_p );

//! Calculation of center for the Arc
/*!
    \param begin_x x start point of arc.
    \param begin_y y start point of arc.
    \param middle_x x middle point of arc.
    \param middle_y y middle point of arc.
    \param end_x x end point of arc.
    \param end_y y end point of arc.
    \param radius radius of the circle calculated
    \param center_x x of the center calculated
    \param center_y y of the center calculated
    \param beginrad calculated starting angle in radians
    \param midrad calculated middle angle in radians
    \param endrad calculated end angle in radians
    \param phit total calculated in radians (AntiClockwise positif, else negatif )

    \return true if the arc is indeed an Arc if a straight line return false.
*/
bool CalcR( double begin_x, double begin_y, double middle_x, double middle_y, double end_x, double end_y,
            double& radius, double& center_x, double& center_y, double& beginrad, double& midrad, double& endrad, double& phit, double marge = a2dACCUR );


#endif

