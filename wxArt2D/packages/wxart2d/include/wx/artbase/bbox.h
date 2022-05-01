/*! \file wx/artbase/bbox.h
    \brief bounding class for optimizing drawing speed.
    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: bbox.h,v 1.14 2009/06/05 19:41:03 titato Exp $
*/

#ifndef __WXBOUNDINGBOX_H__
#define __WXBOUNDINGBOX_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/artbase/afmatrix.h"
#include "wx/geometry.h"


//! Result of a a2dBoundingBox intersection or hittest \ingroup canvasobject
enum OVERLAP
{
    _IN  = 0x01, /*!< Test was completely in the boundingbox */
    _ON  = 0x02, /*!< Test was overlapping the boundingbox its border */
    _OUT = 0x04  /*!< Test was completely out of the boundingbox */
};

//!The a2dBoundingBox class stores one a2dBoundingBox of a a2dCanvasObject.
/*!
    The a2dBoundingBox is defined by two coordiates,
    a upperleft coordinate and a lowerright coordinate.
    It has a valid flag to tell if the boundingbox contains valid information or not.

    \ingroup canvasobject
*/
class A2DARTBASEDLLEXP a2dBoundingBox
{
public:

    //! constructor
    a2dBoundingBox();

    //! boundingbox constructor given another boundingbox
    a2dBoundingBox( const a2dBoundingBox& box );

    //! boundingbox constructor given a point
    a2dBoundingBox( const a2dPoint2D& point );

    //! boundingbox constructor given two sets of coordinates
    a2dBoundingBox( double x1, double y1, double x2, double y2 );

    //! And operation on two boxes
    a2dBoundingBox&  operator+( a2dBoundingBox& );

    //! OR box to this
    const a2dBoundingBox& operator+=( const a2dBoundingBox& box );

    //! set this boundingbox to another boundingbox
    a2dBoundingBox&  operator=(  const a2dBoundingBox& );

    //!intersect the boundingbox with another, return true if the result is non zero
    bool And( a2dBoundingBox*, double Marge = 0 );

    //!enlarge with the given amount
    void Enlarge( const double Marge );

    //!enlarge with the given amount
    void EnlargeXY( const double MargeX, const double MargeY );

    //!shrink with the given amount
    void Shrink( const double Marge );

    //!expand boundingbox width two points
    void Expand( const a2dPoint2D& , const a2dPoint2D& );

    //!expand boundingbox width one points
    void Expand( const a2dPoint2D& );

    //!expand boundingbox width one coordinate
    void Expand( double x, double y );

    //!expand boundingbox width another boundingbox
    void Expand( const a2dBoundingBox& bbox );

    //!check intersection
    //!\return OVERLAP: _IN,_ON,_OUT
    OVERLAP Intersect( const a2dBoundingBox&, double Marge = 0 ) const;

    //!intersection with a line
    //!\return true if interseting
    bool LineIntersect( const a2dPoint2D& begin, const a2dPoint2D& end ) const;

    //!is the point within the boundingbox
    bool PointInBox( const a2dPoint2D&, double Marge = 0 ) const;

    //!is the coordinate within the boundingbox
    bool PointInBox( double x, double y, double Marge = 0 )  const;

    //!is the coordinate on the border of the boundingbox
    bool PointOnBox( double x, double y, double Marge ) const;

    //!set invalid
    void Reset();

    //!translate with given vector
    const a2dBoundingBox& Translate( a2dPoint2D& );

    //!translate with given vector
    const a2dBoundingBox& Translate( double x, double y );

    //!map the boundingbox using the matrix.
    //!The boundingbox is first transformed, and at the new  position and angle etc.
    //!recalculated.
    void MapBbox( const a2dAffineMatrix& matrix );

    //! returns width of the boundingbox
    double  GetWidth() const;
    //! returns height of the boundingbox
    double  GetHeight() const;

    //! returns true if boundingbox is calculated properly and therefore its valid flag is set.
    bool GetValid()  const;

    //! \see GetValid()
    void SetValid( bool );

    //! set the bounding box to be this point
    void SetBoundingBox( const a2dPoint2D& a_point );

    //! set the bounding box its maximum
    void SetMin( double px, double py );

    //! set the bounding box its minimum
    void SetMax( double px, double py );

    //! get the bounding box its minimum
    a2dPoint2D GetMin() const;

    //! get the bounding box its maximum
    a2dPoint2D GetMax() const;

    //! get minimum X of the boundingbox
    double GetMinX() const;

    //! get minimum Y of the boundingbox
    double GetMinY() const;

    //! get maximum X of the boundingbox
    double GetMaxX() const;

    //! get maximum Y of the boundingbox
    double GetMaxY() const;

    inline double GetSize()
    {
        if ( !m_validbbox )
            return 0;

        return wxMax( fabs( m_maxx - m_minx ), fabs( m_maxy - m_miny ) );
    }

    //! get centre
    a2dPoint2D GetCentre() const;

    //! set the bounding box its minimum X, does not validate the box
    void SetMinX( double minx ) { m_minx = minx; }
    //! set the bounding box its minimum Y, does not validate the box
    void SetMinY( double miny ) { m_miny = miny; }
    //! set the bounding box its maximum X, does not validate the box
    void SetMaxX( double maxx ) { m_maxx = maxx; }
    //! set the bounding box its maximum Y, does not validate the box
    void SetMaxY( double maxy ) { m_maxy = maxy; }

#ifdef _DEBUG
    //! dump to debug screen
    void Dump() const;
#endif

protected:

    //!mininum X of bounding box in world coordinates
    double        m_minx;
    //!mininum Y of bounding box in world coordinates
    double        m_miny;
    //!maximum X of bounding box in world coordinates
    double        m_maxx;
    //!maximum Y of bounding box in world coordinates
    double        m_maxy;

    //!true if boundingbox is valid
    bool          m_validbbox;
};

//! global non valid boundingbox to use as default argument etc.
A2DARTBASEDLLEXP_DATA( extern a2dBoundingBox ) wxNonValidBbox;

#endif
