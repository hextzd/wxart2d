/*! \file artbase/src/liner.cpp
    \brief Mainly used for calculating crossings
    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: liner.cpp,v 1.23 2009/07/06 21:06:30 titato Exp $
*/

#include "a2dprec.h"

#include <math.h>

#include <stdlib.h>
#include <float.h>

#include "wx/artbase/liner.h"

a2dLine::a2dLine( double x1, double y1, double x2, double y2 )
{
    m_AA = 0.0;
    m_BB = 0.0;
    m_CC = 0.0;

    m_a = a2dPoint2D( x1, y1 );
    m_b = a2dPoint2D( x2, y2 );

    m_valid_parameters = false;
    CalculateLineParameters();
}

a2dLine::a2dLine( const a2dPoint2D& a, const a2dPoint2D& b )
{
    m_a = a;
    m_b = b;
    m_valid_parameters = false;
    CalculateLineParameters();
}

a2dLine::a2dLine( const a2dLine& other )
{
    m_AA = other.m_AA;
    m_BB = other.m_BB;
    m_CC = other.m_CC;

    m_a = other.m_a;
    m_b = other.m_b;
    m_valid_parameters = other.m_valid_parameters;
    CalculateLineParameters();
}

a2dLine::~a2dLine()
{}

//! makes a a2dLine same as these
//! usage : a2dLine1 = a2dLine2;
a2dLine& a2dLine::operator=( const a2dLine& a_line )
{
    m_AA = a_line.m_AA;
    m_BB = a_line.m_BB;
    m_CC = a_line.m_CC;

    m_a = a_line.m_a;
    m_b = a_line.m_b;
    m_valid_parameters = a_line.m_valid_parameters;
    CalculateLineParameters();
    return *this;
}

// ActionOnTable1
// This function decide which action must be taken, after PointInLine
// has given the results of two points in relation to a a2dLine. See table 1 in the report
//
// input Result_beginPoint:
//          Result_endPoint :
//       The results can be R_R_LEFT_SIDE, R_R_RIGHT_SIDE, R_R_ON_AREA, R_R_IN_AREA
//
// return -1: Illegal combination
//         0: No action, no crosspoints
//         1: Investigate results points in relation to the other a2dLine
//         2: endPoint is a crosspoint, no further investigation
//         3: beginPoint is a crosspoint, no further investigation
//            4: beginPoint and endPoint are crosspoints, no further investigation
//         5: beginPoint is a crosspoint, need further investigation
//         6: endPoint is a crosspoint, need further investigation
int a2dLine::ActionOnTable1( R_PointStatus Result_beginPoint, R_PointStatus Result_endPoint ) const
{
    // beginPoint and endPoint are crosspoints
    if (
        ( Result_beginPoint == R_IN_AREA )
        &&
        ( Result_endPoint == R_IN_AREA )
    )
        return 4;
    // there are no crosspoints, no action
    if (
        (
            ( Result_beginPoint == R_LEFT_SIDE )
            &&
            ( Result_endPoint == R_LEFT_SIDE )
        )
        ||
        (
            ( Result_beginPoint == R_RIGHT_SIDE )
            &&
            ( Result_endPoint == R_RIGHT_SIDE )
        )
    )
        return 0;
    // maybe there is a crosspoint, further investigation needed
    if (
        (
            ( Result_beginPoint == R_LEFT_SIDE )
            &&
            (
                ( Result_endPoint == R_RIGHT_SIDE )
                ||
                ( Result_endPoint == R_ON_AREA )
            )
        )
        ||
        (
            ( Result_beginPoint == R_RIGHT_SIDE )
            &&
            (
                ( Result_endPoint == R_LEFT_SIDE )
                ||
                ( Result_endPoint == R_ON_AREA )
            )
        )
        ||
        (
            ( Result_beginPoint == R_ON_AREA )
            &&
            (
                ( Result_endPoint == R_LEFT_SIDE )
                ||
                ( Result_endPoint == R_RIGHT_SIDE )
                ||
                ( Result_endPoint == R_ON_AREA )
            )
        )
    )
        return 1;
    //there is a crosspoint
    if (
        (
            ( Result_beginPoint == R_LEFT_SIDE )
            ||
            ( Result_beginPoint == R_RIGHT_SIDE )
        )
        &&
        ( Result_endPoint == R_IN_AREA )
    )
        return 2;
    // there is a crosspoint
    if (
        ( Result_beginPoint == R_IN_AREA )
        &&
        (
            ( Result_endPoint == R_LEFT_SIDE )
            ||
            ( Result_endPoint == R_RIGHT_SIDE )
        )
    )
        return 3;
    // beginPoint is a crosspoint, further investigation needed
    if (
        ( Result_beginPoint == R_IN_AREA )
        &&
        ( Result_endPoint == R_ON_AREA )
    )
        return 5;
    // endPoint is a crosspoint, further investigation needed
    if (
        ( Result_beginPoint == R_ON_AREA )
        &&
        ( Result_endPoint == R_IN_AREA )
    )
        return 6;
    // All other combinations are illegal
    return -1;
}


// ActionOnTable2
// This function decide which action must be taken, after PointInLine
// has given the results of two points in relation to a a2dLine. It can only give a
// correct decision if first the relation of the points from the a2dLine
// are investigated in relation to the a2dLine wich can be constucted from the points.
//
// input Result_beginPoint:
//          Result_endPoint :
//       The results can be R_LEFT_SIDE, R_RIGHT_SIDE, R_ON_AREA, R_IN_AREA
//
// return -1: Illegal combination
//         0: No action, no crosspoints
//         1: Calculate crosspoint
//         2: endPoint is a crosspoint
//         3: beginPoint is a crosspoint
//         4: beginPoint and endPoint are crosspoints
int a2dLine::ActionOnTable2( R_PointStatus Result_beginPoint, R_PointStatus Result_endPoint ) const
{
    // beginPoint and eindpoint are crosspoints
    if (
        ( Result_beginPoint == R_IN_AREA )
        &&
        ( Result_endPoint == R_IN_AREA )
    )
        return 4;
    // there are no crosspoints
    if (
        (
            ( Result_beginPoint == R_LEFT_SIDE )
            &&
            (
                ( Result_endPoint == R_LEFT_SIDE )
                ||
                ( Result_endPoint == R_ON_AREA )
            )
        )
        ||
        (
            ( Result_beginPoint == R_RIGHT_SIDE )
            &&
            (
                ( Result_endPoint == R_RIGHT_SIDE )
                ||
                ( Result_endPoint == R_ON_AREA )
            )
        )
        ||
        (
            ( Result_beginPoint == R_ON_AREA )
            &&
            (
                ( Result_endPoint == R_LEFT_SIDE )
                ||
                ( Result_endPoint == R_RIGHT_SIDE )
                ||
                ( Result_endPoint == R_ON_AREA )
            )
        )
    )
        return 0;
    // there is a real intersection, which must be calculated
    if (
        (
            ( Result_beginPoint == R_LEFT_SIDE )
            &&
            ( Result_endPoint == R_RIGHT_SIDE )
        )
        ||
        (
            ( Result_beginPoint == R_RIGHT_SIDE )
            &&
            ( Result_endPoint == R_LEFT_SIDE )
        )
    )
        return 1;
    // endPoint is a crosspoint
    if (
        (
            ( Result_beginPoint == R_LEFT_SIDE )
            ||
            ( Result_beginPoint == R_RIGHT_SIDE )
            ||
            ( Result_beginPoint == R_ON_AREA )
        )
        &&
        ( Result_endPoint == R_IN_AREA )
    )
        return 2;
    // beginPoint is a crosspoint
    if (
        ( Result_beginPoint == R_IN_AREA )
        &&
        (
            ( Result_endPoint == R_LEFT_SIDE )
            ||
            ( Result_endPoint == R_RIGHT_SIDE )
            ||
            ( Result_endPoint == R_ON_AREA )
        )
    )
        return 3;
    // All other combinations are illegal
    return -1;
}

//! Calculate the Y when the X is given
double a2dLine::Calculate_Y( double X ) const
{
    assert( m_valid_parameters );
    if ( m_AA != 0 )
        return -( m_AA * X + m_CC ) / m_BB;
    else
        // horizontal a2dLine
        return m_a.m_y;
}

void a2dLine::Virtual_Point( a2dPoint2D& a_point, double distance )  const
{
    assert( m_valid_parameters );

    //calculate the distance using the slope of the a2dLine
    //and rotate 90 degrees

    a_point.m_y = a_point.m_y + ( distance * -m_BB );
    a_point.m_x = a_point.m_x - ( distance * m_AA );
}

a2dPoint2D a2dLine::DistancePoint( double distance, bool begin ) const
{
    assert( m_valid_parameters );
    a2dPoint2D a_point;

    if ( begin )
    {
        a_point.m_y = m_a.m_y + ( distance * m_AA );
        a_point.m_x = m_a.m_x + ( distance * -m_BB );
    }
    else
    {
        a_point.m_y = m_b.m_y + ( distance * m_AA );
        a_point.m_x = m_b.m_x + ( distance * -m_BB );
    }
    return a_point;
}

a2dLine* a2dLine::CreatePerpendicularLineAt( const a2dPoint2D& a_point )  const
{
    assert( m_valid_parameters );

    double distance = GetLength() * 2;
    a2dPoint2D begin, end;

    begin.m_y = a_point.m_y + ( distance * -m_BB );
    begin.m_x = a_point.m_x - ( distance * m_AA );

    end.m_y = a_point.m_y - ( distance * -m_BB );
    end.m_x = a_point.m_x + ( distance * m_AA );

    return new a2dLine( begin, end );
}

double a2dLine::GetLength() const
{
    return sqrt( wxSqr( m_b.m_y - m_a.m_y ) + wxSqr( m_a.m_x - m_b.m_x ) );
}

//! Calculate the lineparameters for the a2dLine if nessecary
void a2dLine::CalculateLineParameters()
{
    // if not valid_parameters calculate the parameters
    if ( !m_valid_parameters )
    {
        double length;

        // bp AND ep may not be the same
        if ( m_a == m_b )
        {
            //assume it is a horizontal segment, but very small
            m_AA = 0;
            m_BB = -1e-12;
            //assert ( 0 );
        }
        else
        {
            m_AA = ( m_b.m_y - m_a.m_y ); // A = (Y2-Y1)
            m_BB = ( m_a.m_x - m_b.m_x ); // B = (X1-X2)
        }

        // the parameters A end B can now be normalized
        length = sqrt( m_AA * m_AA + m_BB * m_BB );

        assert( length != 0 );

        m_AA = ( m_AA / length );
        m_BB = ( m_BB / length );

        m_CC = -( ( m_AA * m_a.m_x ) + ( m_a.m_y * m_BB ) );

        m_valid_parameters = true;
    }
}


//! Checks if a a2dLine intersect with another a2dLine
/*!
 inout    a2dLine : another a2dLine
         Marge: optional, standard on MARGE (declared in MISC.CPP)

 return   true : wxLines are crossing
          false: wxLines are not crossing
*/
bool a2dLine::CheckIntersect ( a2dLine& lijn, double Marge ) const
{
    double distance = 0;

    // bp AND ep may not be the same
    if ( m_a == m_b )
        assert ( 0 );

    int Take_Action1, Take_Action2;
    bool Total_Result = false;
    R_PointStatus Result_beginPoint, Result_endPoint;

    Result_beginPoint = PointInLine( lijn.m_a, distance, Marge );
    Result_endPoint   = PointInLine( lijn.m_b, distance, Marge );
    Take_Action1 = ActionOnTable1( Result_beginPoint, Result_endPoint );
    switch ( Take_Action1 )
    {
        case 0: Total_Result = false ; break;
        case 1:
        {
            Result_beginPoint = lijn.PointInLine( m_a, distance, Marge );
            Result_endPoint   = lijn.PointInLine( m_b, distance, Marge );
            Take_Action2 = ActionOnTable2( Result_beginPoint, Result_endPoint );
            switch ( Take_Action2 )
            {
                case 0: Total_Result = false; break;
                case 1: case 2: case 3: case 4: Total_Result = true; break;
            }
        }
        ; break; // This break belongs to the switch(Take_Action1)
        case 2: case 3: case 4: case 5: case 6: Total_Result = true; break;
    }
    return Total_Result; //This is the final decision
}


//! Get the beginPoint from the a2dLine
//! usage: Point aPoint = a_line.GetBeginPoint()
a2dPoint2D a2dLine::GetBeginPoint() const
{
    return m_a;
}



//! Get the endPoint from the a2dLine
//! usage: Point aPoint = a_line.GetEndPoint()
a2dPoint2D a2dLine::GetEndPoint() const
{
    return m_b;
}

int a2dLine::Intersect( a2dLine& lijn, a2dPoint2D& c1 , a2dPoint2D& c2 , double Marge ) const
{
    double distance = 0;

    // bp AND ep may not be the same
    if ( m_a == m_b )
        assert ( 0 );

    R_PointStatus Result_beginPoint, Result_endPoint;
    int Take_Action1, Take_Action2, Number_of_Crossings = 0;

    Result_beginPoint = PointInLine( lijn.m_a, distance, Marge );
    Result_endPoint   = PointInLine( lijn.m_b, distance, Marge );

    Take_Action1 = ActionOnTable1( Result_beginPoint, Result_endPoint );
// 0: No action, no crosspoints
// 1: Investigate results points in relation to the other a2dLine
// 2: endPoint is a crosspoint, no further investigation
// 3: beginPoint is a crosspoint, no further investigation
// 4: beginPoint and endPoint are crosspoints, no further investigation
// 5: beginPoint is a crosspoint, need further investigation
// 6: endPoint is a crosspoint, need further investigation

    // The first switch will insert a crosspoint immediatly
    switch ( Take_Action1 )
    {
        case 2: case 6: c1 = lijn.m_b;
            Number_of_Crossings = 1;
            break;
        case 3: case 5: c1 = lijn.m_a;
            Number_of_Crossings = 1;
            break;
        case 4:         c1 = lijn.m_a;
            c2 = lijn.m_b;
            Number_of_Crossings = 2;
            break;
        default:
            break;
    }

    // This switch wil investigate the points of this a2dLine in relation to lijn
    // 1: Investigate results points in relation to the other a2dLine
    // 5: beginPoint is a crosspoint, need further investigation
    // 6: endPoint is a crosspoint, need further investigation
    switch ( Take_Action1 )
    {
        case 1: case 5: case 6:
        {
            Result_beginPoint = lijn.PointInLine( m_a, distance, Marge );
            Result_endPoint   = lijn.PointInLine( m_b, distance, Marge );
            Take_Action2 = ActionOnTable2( Result_beginPoint, Result_endPoint );
            // return -1: Illegal combination
            //         0: No action, no crosspoints
            //         1: Calculate crosspoint
            //         2: endPoint is a crosspoint
            //         3: beginPoint is a crosspoint
            //         4: beginPoint and endPoint are crosspoints
            switch ( Take_Action2 )
            {
                    // for the cases see the returnvalue of ActionTable2
                case 1:
                {
                    // begin of scope to calculate the intersection
                    double X, Y, Denominator;
                    assert( m_valid_parameters );
                    Denominator  = ( m_AA * lijn.m_BB ) - ( lijn.m_AA * m_BB );
                    // Denominator may not be 0
                    assert( Denominator != 0.0 );
                    // Calculate intersection of both linesegments
                    X = ( ( m_BB * lijn.m_CC ) - ( lijn.m_BB * m_CC ) ) / Denominator;
                    Y = ( ( lijn.m_AA * m_CC ) - ( m_AA * lijn.m_CC ) ) / Denominator;

                    c1.m_x = X;
                    c1.m_y = Y;
                }
                Number_of_Crossings++;
                break;
                case 2:  c2 = m_a;
                    Number_of_Crossings++;
                    break;
                case 3:  c2 = m_b;
                    Number_of_Crossings++;
                    break;
                case 4:  c1 = m_a;
                    c2 = m_b;
                    Number_of_Crossings = 2;
                    break;
            }
        };
        break;
        default:
            break;
    }
    return Number_of_Crossings; //This is de final number of crossings
}


//! test if a point lies in the linesegment.
/*!
If the point isn't on the a2dLine
the function returns a value that indicates on which side of the
a2dLine the point is (in linedirection from first point to second point

 returns R_LEFT_SIDE, when point lies on the left side of the a2dLine
         R_RIGHT_SIDE, when point lies on the right side of the a2dLine
         R_ON_AREA, when point lies on the infinite a2dLine within a range
         R_IN_AREA, when point lies in the area of the linesegment
        the returnvalues are declared in (a2dLine.H)
*/
R_PointStatus a2dLine::PointInLine( const a2dPoint2D& a_Point, double& Distance, double Marge ) const
{
    Distance = 0;

    int Result_ofm_BBox = false;
    R_PointStatus Result_of_Online;

    //quick test if point is begin or endpoint
    if ( a_Point == m_a || a_Point == m_b )
        return R_IN_AREA;

    // Handle degenerate lines
    if( m_a == m_b )
    {
        if (
            a_Point.m_x >= ( m_a.m_x - Marge ) && a_Point.m_x <= ( m_a.m_x + Marge ) &&
            a_Point.m_y >= ( m_a.m_y - Marge ) && a_Point.m_y <= ( m_a.m_y + Marge )
        )
            return R_IN_AREA;
        else
            return R_LEFT_SIDE;
    }

    // Checking if point is in bounding-box with marge
    double xmin = wxMin( m_a.m_x, m_b.m_x );
    double xmax = wxMax( m_a.m_x, m_b.m_x );
    double ymin = wxMin( m_a.m_y, m_b.m_y );
    double ymax = wxMax( m_a.m_y, m_b.m_y );

    if (  a_Point.m_x >= ( xmin - Marge ) && a_Point.m_x <= ( xmax + Marge ) &&
            a_Point.m_y >= ( ymin - Marge ) && a_Point.m_y <= ( ymax + Marge ) )
        Result_ofm_BBox = true;

    // Checking if point is on the infinite a2dLine
    Result_of_Online = PointOnLine( a_Point, Distance, Marge );

    // point in boundingbox of the a2dLine and is on the a2dLine then the point is R_IN_AREA
    if ( ( Result_ofm_BBox ) && ( Result_of_Online == R_ON_AREA ) )
        return R_IN_AREA;
    else
        return Result_of_Online;
}


//! test if a point lies on the a2dLine.
/*!If the point isn't on the a2dLine
 the function returns a value that indicates on which side of the
 a2dLine the point is (in linedirection from first point to second point

 returns R_LEFT_SIDE, when point lies on the left side of the a2dLine
         R_ON_AREA, when point lies on the infinite a2dLine within a range
         R_RIGHT_SIDE, when point lies on the right side of the a2dLine
         R_LEFT_SIDE , R_RIGHT_SIDE , R_ON_AREA
*/
R_PointStatus a2dLine::PointOnLine( const a2dPoint2D& a_Point, double& Distance, double Marge ) const
{
    Distance = 0;
    // Point may not be queal
    assert( m_a != m_b );

    //quick test if point is begin or endpoint
    if ( a_Point == m_a || a_Point == m_b )
        return R_ON_AREA;

    assert( m_valid_parameters );
    // calculate the distance of a_Point in relation to the a2dLine
    Distance = ( m_AA * a_Point.m_x ) + ( m_BB * a_Point.m_y ) + m_CC;

    if ( Distance < -Marge )
        return R_LEFT_SIDE;
    else
    {
        if ( Distance > Marge )
            return R_RIGHT_SIDE;
        else
            return R_ON_AREA;
    }
}

a2dPoint2D a2dLine::ProjectedPoint( const a2dPoint2D& p ) const
{
    double dx12 = double( m_b.m_x ) - double( m_a.m_x );
    double dy12 = double( m_b.m_y ) - double( m_a.m_y );

    double dx1p = double( p.m_x ) - double( m_a.m_x );
    double dy1p = double( p.m_y ) - double( m_a.m_y );

    double length = dx12 * dx12 + dy12 * dy12;
    double projection = dx12 * dx1p + dy12 * dy1p;

    // Note: we don't want the projection length, but the projection fraction
    // between 0..1. As projection is the projection length * length, we must devide
    // by length^2 here
    projection /= length;
    return a2dPoint2D( m_a.m_x + projection * dx12, m_a.m_y + projection * dy12 );
}

double a2dLine::PointDistance( const a2dPoint2D& p, a2dPoint2D* nearest ) const
{
    double dx12 = double( m_b.m_x ) - double( m_a.m_x );
    double dy12 = double( m_b.m_y ) - double( m_a.m_y );

    double dx1p = double( p.m_x ) - double( m_a.m_x );
    double dy1p = double( p.m_y ) - double( m_a.m_y );

    double length = dx12 * dx12 + dy12 * dy12;
    double projection = dx12 * dx1p + dy12 * dy1p;

    if( projection <= 0 || length == 0 )
    {
        // The point is beyond m_a
        if( nearest ) * nearest = m_a;
        return sqrt( dx1p * dx1p + dy1p * dy1p );
    }
    else if( projection >= length )
    {
        // The point is beyond m_b
        if( nearest ) * nearest = m_b;
        double dx2p = double( p.m_x ) - double( m_b.m_x );
        double dy2p = double( p.m_y ) - double( m_b.m_y );
        return sqrt( dx2p * dx2p + dy2p * dy2p );
    }
    else
    {
        // The point is between m_a und m_b

        // First check vertical and horizontal lines before doing sqrt and / operations
        if( !nearest )
        {
            if( dx12 == 0 )
                return fabs( dx1p );
            if( dy12 == 0 )
                return fabs( dy1p );
        }

        // Note: we don't want the projection length, but the projection fraction
        // between 0..1. As projection is the projection length * length, we must devide
        // by length^2 here
        projection /= length;
        if( nearest ) * nearest = a2dPoint2D( m_a.m_x + projection * dx12, m_a.m_y + projection * dy12 );
        // orthogonal vector from point to line
        dx1p -= projection * dx12;
        dy1p -= projection * dy12;
        return sqrt( dx1p * dx1p + dy1p * dy1p );
    }
}

double a2dLine::PointDistanceOrhto( const a2dPoint2D& p, a2dPoint2D* nearest ) const
{
    double dx12 = double( m_b.m_x ) - double( m_a.m_x );
    double dy12 = double( m_b.m_y ) - double( m_a.m_y );

    double dx1p = double( p.m_x ) - double( m_a.m_x );
    double dy1p = double( p.m_y ) - double( m_a.m_y );

    if( dx12 == 0 && dy12 == 0 )
    {
        if( nearest )
            * nearest = m_a;
        return DBL_MAX;
    }
    else if( fabs( dx12 ) < fabs( dy12 ) )
    {
        // line is more vertical => user horizontal distance
        if( p.m_y < wxMin( m_a.m_y, m_b.m_y ) || p.m_y > wxMax( m_a.m_y, m_b.m_y ) )
        {
            if( nearest )
                * nearest = p;
            return DBL_MAX;
        }
        else
        {
            double y = p.m_y;
            double x = dy1p * dx12 / dy12 + m_a.m_x;
            if( nearest )
            {
                nearest->m_x = x;
                nearest->m_y = y;
            }
            return fabs( p.m_x - x );
        }
    }
    else
    {
        // line is more horizontal => user vertical distance
        if( p.m_x < wxMin( m_a.m_x, m_b.m_x ) || p.m_x > wxMax( m_a.m_x, m_b.m_x ) )
        {
            if( nearest )
                * nearest = p;
            return DBL_MAX;
        }
        else
        {
            double x = p.m_x;
            double y = dx1p * dy12 / dx12 + m_a.m_y;
            if( nearest )
            {
                nearest->m_x = x;
                nearest->m_y = y;
            }
            return fabs( p.m_y - y );
        }
    }
}

void a2dLine::OffsetContour( const a2dLine& nextline, double factor, a2dPoint2D& offsetpoint )  const
{
    a2dPoint2D offs_begin( m_a );
    a2dPoint2D offs_end( m_b );

    a2dPoint2D offs_bgn_next( nextline.m_a );
    a2dPoint2D offs_end_next( nextline.m_b );
    // make a a2dPoint2D from this point

    Virtual_Point( offs_begin, factor );
    Virtual_Point( offs_end, factor );
    a2dLine  offs_currentline( offs_begin, offs_end );

    nextline.Virtual_Point( offs_bgn_next, factor );
    nextline.Virtual_Point( offs_end_next, factor );
    a2dLine  offs_nextline( offs_bgn_next, offs_end_next );

    offs_nextline.CalculateLineParameters();
    offs_currentline.CalculateLineParameters();
    offs_currentline.Intersect( offs_nextline, offsetpoint );
}

//! Return the position of the second a2dLine compared to this a2dLine
/*!
 Result = IS_ON | IS_LEFT | IS_RIGHT
 Here Left and Right is defined as being left or right from
 the this a2dLine towards the center (common) node
 direction of vetors taken as begin to endpoint with end of this at
 begin of a2dLine two
*/
OUTPRODUCT a2dLine::OutProduct( const a2dLine& two, double accur ) const
{
    R_PointStatus uitp;
    double distance;
    if ( two.m_a == two.m_b )
        assert( 0 );
    if ( m_a == m_b )
        assert( 0 );

    uitp = PointOnLine( two.m_b, distance, accur );


    /*double uitp=  (_x - first._x) * (third._y - _y) -
                    (_y - first._y) * (third._x - _x);
    if (uitp>0) return IS_LEFT;
    if (uitp<0) return IS_RIGHT;
    return IS_ON;*/

    //depending on direction of this link (going to or coming from centre)
    if ( uitp == R_LEFT_SIDE )
        return R_IS_LEFT;
    if ( uitp == R_RIGHT_SIDE )
        return R_IS_RIGHT;
    return R_IS_ON;
}

//! Intersects two lines if a crossing return true
//! else false
bool a2dLine::Intersect( a2dLine& lijn, a2dPoint2D& crossing ) const
{
    // lijn must exist
    assert( m_valid_parameters );
    assert( lijn.m_valid_parameters );

    double X, Y, Denominator;
    Denominator  = ( m_AA * lijn.m_BB ) - ( lijn.m_AA * m_BB );
    // Denominator may not be 0
    if ( Denominator == 0.0 )
        return false;
    // Calculate intersection of both linesegments
    X = ( ( m_BB * lijn.m_CC ) - ( lijn.m_BB * m_CC ) ) / Denominator;
    Y = ( ( lijn.m_AA * m_CC ) - ( m_AA * lijn.m_CC ) ) / Denominator;

    crossing.m_x = X;
    crossing.m_y = Y;
    return true;
}

