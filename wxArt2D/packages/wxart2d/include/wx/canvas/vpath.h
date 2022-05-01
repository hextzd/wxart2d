/*! \file wx/canvas/vpath.h
    \brief vector path a2dVectorPath derived from a2dCanvasObject

    a2dVectorPath holds internal a plain a2dVpath, which no can be styled and placed
    inside a document.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: vpath.h,v 1.17 2009/04/04 15:04:56 titato Exp $
*/

/*!
    \defgroup vpath vector path a2dCanvasObject plus segment classes

    \ingroup canvasobject
*/

#ifndef __WXVPATH_H__
#define __WXVPATH_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/artbase/drawer2d.h"
#include "wx/canvas/canobj.h"

//----------------------------------------------------------------------------
// a2dVectorPath
//----------------------------------------------------------------------------


//!a2dCanvasObject for a Vector Path
/*!
    This class is mainly a wrapper around a2dVpath,
    a2dVpath is a wxArray like structure.
    So see wxArray for more documentation.


    \sa a2dVpath
    \sa a2dVpathqBCurveSegment
    \sa a2dVpathCBCurveSegment
    \sa a2dVpathArcSegment

    \sa wxArray

    \remark do use Shrink() after filling a a2dVectorPath, to reduce memory usage

    \ingroup canvasobject vpath
*/
class A2DCANVASDLLEXP a2dVectorPath: public a2dCanvasObject
{
public:

    //! construct with empty array of segments
    a2dVectorPath();

    //! initialize using a vector path array
    a2dVectorPath( a2dVpath* path );

    //! construct with a pointer to another a2dVectorPath
    a2dVectorPath( const a2dVectorPath& other, CloneOptions options, a2dRefMap* refs );

    ~a2dVectorPath();

    //! calculate Length
    double Length() { return m_segments->Length(); }

    //! reduce matrix to identity without replacing object
    bool EliminateMatrix();

    //! Convert complex segments to line segments.
    void ConvertToLines();

    //! convert to a list of a2dPolygonL and a2dPolylineL
    /*!
        For vectorpaths can be split into closed sections ( polygon filled areas),
        non closed sections ( polyline ). Those two type can be stroked or not.
        If m_contourwidth is set the two type above are contoured and a distance of
        m_contourwidth/2.0 and those parts are stroked.
        Effectively this brings only pure polygons as output.
    */
    a2dCanvasObjectList* GetAsPolygons();

    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    virtual bool RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld );

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
    //void SetPosXYSegment(int index, double x, double y, bool afterinversion = true );

    //! get point x and y at index
    /*!
        \param index the index of the point to change
        \param x new x for the point
        \param y new y for the point
        \param transform if true return x y transformed with local matrix
    */
    //void GetPosXYSegment(int index, double& x, double& y, bool transform = true );

    //! use this to get to other function in underlying wxArray
    a2dVpath* GetSegments() { return m_segments; }

    //!Return the number of segments.
    size_t GetCount() { return m_segments->size(); }

    //!Append a new segment, which is owned by this class now
    void Add( a2dVpathSegment* item ) { m_segments->push_back( item ); }

    //!Preallocates memory for a given number of array elements.
    /*!
     It is worth calling when the number of items which are going to be added to the array
     is known in advance because it will save unneeded memory reallocation.
     If the array already has enough memory for the given number of items, nothing happens.
    */
    void Alloc( size_t count ) { m_segments->reserve( count ); }

    //!This function does the same as Empty() and additionally frees the memory allocated to the array.
    void Clear() { m_segments->clear(); }

    //!Returns true if the array is empty, false otherwise.
    bool IsEmpty() const  { return m_segments->empty(); }

    //! get i'th segment
    a2dVpathSegmentPtr operator[]( size_t index ) const { return m_segments->operator[]( index ); }

    //!Returns a reference to the segment at the given position in the array.
    //!If index is out of bounds, an assert failure is raised in
    //!the debug builds but nothing special is done in the release build.
    a2dVpathSegmentPtr Item( size_t index ) const { return m_segments->operator[]( index ); }

    //!Returns a reference to the last segment in de array, i.e. is the same as
    //!Item(GetCount() - 1). An assert failure is raised in the debug mode if the array is empty.
    a2dVpathSegmentPtr Last() const { return m_segments->back(); }

    //!Removes a segment from the array by index. When a segment is removed
    //!it is deleted - use Detach() if you don't want this to happen.
    //void RemoveAt(size_t index) { m_segments->erase( index ); }

    void SetContourWidth( double width ) { m_contourwidth = width; SetPending( true ); }

    //!get the Contour width of the shape
    double GetContourWidth() const { return m_contourwidth; }

    //! GDSII compatible to sub identify this object.
    /*! you can use it as a special tagged object */
    void SetDataType( int type ) { m_datatype = type; }

    //! GDSII compatible to sub identify this object.
    /*! you can use it as a special tagged object */
    int GetDataType() { return m_datatype; }

    //! Set when m_contourwidth != 0 what is the end of the lines should be.
    void SetPathType( a2dPATH_END_TYPE pathtype ) { m_pathtype = pathtype; }

    //! get when m_contourwidth != 0 what is the end of the lines looks like.
    a2dPATH_END_TYPE GetPathType() { return m_pathtype; }

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    DECLARE_DYNAMIC_CLASS( a2dVectorPath )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

#if wxART2D_USE_CVGIO
    void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dVpath* m_segments;

    //! if != 0 the continues path parts are contoured at distance m_contourwidth/2
    double m_contourwidth;

    //! GDSII compatible to sub identify this object
    int m_datatype;

    //! when m_contourwidth != 0 what is the end of the line looking like.
    a2dPATH_END_TYPE m_pathtype;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dVectorPath( const a2dVectorPath& other );
};

#endif

