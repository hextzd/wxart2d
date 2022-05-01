/*! \file wx/artbase/liner.h
    \brief basic 2 point line class for intersection and contouring routines.
    \author Klaas Holwerda
    \date Created 1/10/2000

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: liner.h,v 1.11 2009/07/06 21:06:29 titato Exp $
*/
#ifndef __WXLINER_H
#define __WXLINER_H


#include "wx/artbase/afmatrix.h"

enum OUTPRODUCT {R_IS_LEFT, R_IS_ON, R_IS_RIGHT};

// Status of a point to a a2dLine
enum R_PointStatus {R_LEFT_SIDE, R_RIGHT_SIDE, R_ON_AREA, R_IN_AREA};

#undef _BB
#undef _AA
#undef _CC

//!Line calculations.
/*! A line class to intersect lines and line segments.
Next to that the are point on/in Line and Line Segment tests.
The class is designed to be accurate and can deal with vertical and horizontal lines.
A snap factor (marge) is often required to do proper calculations

    \ingroup general
    \ingroup drawer
*/
class A2DARTBASEDLLEXP a2dLine
{
public:

    //!create a line with given end and begin point
    /*!
    \remark line parameters will not be calculated automatically.
    \param x1 X begin point
    \param y1 Y begin point
    \param x2 X end point
    \param y2 Y end point
    */
    a2dLine( double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0 );

    //!create a line with given end and begin point
    /*!
    \remark line parameters will not be calculated automatically.
    \param a  begin point
    \param b  end point
    */
    a2dLine( const a2dPoint2D& a, const a2dPoint2D& b );

    a2dLine( const a2dLine& other );

    //! destructor
    ~a2dLine();

    a2dLine& operator = ( const a2dLine& other );

    //! Get the beginpoint from a a2dLine
    a2dPoint2D GetBeginPoint() const;

    //! Get the endpoint from a a2dLine
    a2dPoint2D GetEndPoint() const;

    //! Check if two wxLines intersects
    /*! \param line line to intersect with
        \param Marge within volume at this distance still an intersection
        \return true is intersecting
    */
    bool CheckIntersect( a2dLine& line, double Marge ) const;

    //! Intersects two wxLines
    /*! \param line line to intersect with
        \param bp beginpoint
        \param ep endpoint
        \param Marge within volume at this distance still an intersection

        \return   0: If there are no crossings
                  1: If there is one crossing
                  2: If there are two crossings
    */
    int Intersect( a2dLine& line,  a2dPoint2D& bp , a2dPoint2D& ep , double Marge ) const;

    //!intersect two (infinit) lines
    bool Intersect( a2dLine& lijn, a2dPoint2D& crossing ) const;

    //!For an infinite a2dLine
    R_PointStatus   PointOnLine( const a2dPoint2D& a_Point, double& Distance, double Marge ) const;

    //!For a non-infinite a2dLine
    R_PointStatus   PointInLine( const a2dPoint2D& a_Point, double& Distance, double Marge ) const;

    //! return point after projecting p to this line.
    a2dPoint2D ProjectedPoint( const a2dPoint2D& p ) const;

    //!Calculate the distance of a point from the line. Works even if l==0
    /*!\param p the point to which the closest point on the line is searched
       \param nearest if not 0, the closest point to p on the line is assigned to nearest
    */
    double PointDistance( const a2dPoint2D& p, a2dPoint2D* nearest ) const;

    //!Calculate the orthogonal distance of a point from the line. Works even if l==0
    /*!\param p the point to which the closest point on the line is searched
       \param nearest if not 0, the closest point to p on the line is assigned to nearest
    */
    double PointDistanceOrhto( const a2dPoint2D& p, a2dPoint2D* nearest ) const;

    //! outproduct of two wxLines
    OUTPRODUCT OutProduct( const a2dLine& two, double accur ) const;

    //! Caclulate Y if X is known
    double Calculate_Y( double X ) const;

    //!calculate point Perpendicula at distance from the line, through given point
    void Virtual_Point( a2dPoint2D& a_point, double distance ) const;

    //!calculate point on line at distance from the begin or end of line
    a2dPoint2D DistancePoint( double distance, bool begin ) const;

    //! create a line through a_point and which is pperpendicular to this
    a2dLine* CreatePerpendicularLineAt( const a2dPoint2D& a_point )  const;

    //! Calculate the parameters if invalid
    void CalculateLineParameters();

    //!Calculate point for a contour at a given distance
    void OffsetContour( const a2dLine& nextline, double factor, a2dPoint2D& offsetpoint ) const;

    //! return length of vector
    double GetLength() const;

private:

    //! Function needed for Intersect
    int   ActionOnTable1( R_PointStatus, R_PointStatus ) const;

    //! Function needed for Intersect
    int   ActionOnTable2( R_PointStatus, R_PointStatus ) const;


    //! delta y (normalized by length)
    double      m_AA;

    //! delta x (normalized by length)
    double      m_BB;

    //! vector (normalized by length)
    double      m_CC;

    //!beginpoint
    a2dPoint2D m_a;

    //! end point
    a2dPoint2D m_b;

    //! are the line parameters valid
    bool        m_valid_parameters;
};

#endif
