/*! \file artbase/src/bbox.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: bbox.cpp,v 1.16 2009/06/05 19:41:03 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/artbase/bbox.h"

a2dBoundingBox wxNonValidBbox;

a2dBoundingBox::a2dBoundingBox()
{
    m_minx = m_miny = m_maxx =  m_maxy = 0.0;
    m_validbbox = false;
}


a2dBoundingBox::a2dBoundingBox( const a2dBoundingBox& other )
{
    m_minx = other.m_minx;
    m_miny = other.m_miny;
    m_maxx = other.m_maxx;
    m_maxy = other.m_maxy;
    m_validbbox = other.m_validbbox;
}


a2dBoundingBox::a2dBoundingBox( const a2dPoint2D& a )
{
    m_validbbox = false;
    Expand( a.m_x, a.m_y );
}

a2dBoundingBox::a2dBoundingBox( double x1, double y1, double x2, double y2 )
{
    m_validbbox = false;
    Expand( x1, y1 );
    Expand( x2, y2 );
}

// This function intersects
bool a2dBoundingBox::And( a2dBoundingBox* bbox, double Marge )
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );
    wxASSERT_MSG( bbox->m_validbbox, wxT( "invalid bbox" ) );

    if ( ( ( m_minx - Marge )  > ( bbox->m_maxx + Marge ) ) ||
            ( ( m_maxx + Marge ) < ( bbox->m_minx - Marge ) ) ||
            ( ( m_maxy + Marge ) < ( bbox->m_miny - Marge ) ) ||
            ( ( m_miny - Marge ) > ( bbox->m_maxy + Marge ) ) )
        return false;

    // Check if other.bbox is inside this bbox
    if ( ( m_minx <= bbox->m_minx ) &&
            ( m_maxx >= bbox->m_maxx ) &&
            ( m_maxy >= bbox->m_maxy ) &&
            ( m_miny <= bbox->m_miny ) )
    {
        m_minx = bbox->m_minx;
        m_maxx = bbox->m_maxx;
        m_miny = bbox->m_miny;
        m_maxy = bbox->m_maxy;
        return true;
    }

    //overlap
    m_minx = wxMax( m_minx, bbox->m_minx );
    m_maxx = wxMin( m_maxx, bbox->m_maxx );
    m_miny = wxMax( m_miny, bbox->m_miny );
    m_maxy = wxMin( m_maxy, bbox->m_maxy );
    return true;
}

// Shrink the boundingbox with the given marge
void a2dBoundingBox::Shrink( const double Marge )
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );

    m_minx += Marge;
    m_maxx -= Marge;
    m_miny += Marge;
    m_maxy -= Marge;
}


// Expand the boundingbox with another boundingbox
void a2dBoundingBox::Expand( const a2dBoundingBox& other )
{
    if ( !m_validbbox )
    {
        *this = other;
    }
    else
    {
        m_minx = wxMin( m_minx, other.m_minx );
        m_maxx = wxMax( m_maxx, other.m_maxx );
        m_miny = wxMin( m_miny, other.m_miny );
        m_maxy = wxMax( m_maxy, other.m_maxy );
    }
}


// Expand the boundingbox with a point
void a2dBoundingBox::Expand( const a2dPoint2D& a_point )
{
    if ( !m_validbbox )
    {
        m_minx = m_maxx = a_point.m_x;
        m_miny = m_maxy = a_point.m_y;
        m_validbbox = true;
    }
    else
    {
        m_minx = wxMin( m_minx, a_point.m_x );
        m_maxx = wxMax( m_maxx, a_point.m_x );
        m_miny = wxMin( m_miny, a_point.m_y );
        m_maxy = wxMax( m_maxy, a_point.m_y );
    }
}

// Expand the boundingbox with a point
void a2dBoundingBox::Expand( double x, double y )
{
    if ( !m_validbbox )
    {
        m_minx = m_maxx = x;
        m_miny = m_maxy = y;
        m_validbbox = true;
    }
    else
    {
        m_minx = wxMin( m_minx, x );
        m_maxx = wxMax( m_maxx, x );
        m_miny = wxMin( m_miny, y );
        m_maxy = wxMax( m_maxy, y );
    }
}


// Expand the boundingbox with two points
void a2dBoundingBox::Expand( const a2dPoint2D& a, const a2dPoint2D& b )
{
    Expand( a );
    Expand( b );
}

// Enlarge the boundingbox with the given marge
void a2dBoundingBox::Enlarge( const double marge )
{
    if ( !m_validbbox )
    {
        m_minx = -marge;
        m_miny = -marge;
        m_maxx = marge;
        m_maxy = marge;
        m_validbbox = true;
    }
    else
    {
        m_minx -= marge;
        m_maxx += marge;
        m_miny -= marge;
        m_maxy += marge;
    }
}

// Enlarge the boundingbox with the given marge
void a2dBoundingBox::EnlargeXY( const double margeX, const double margeY )
{
    if ( !m_validbbox )
    {
        m_minx = -margeX;
        m_miny = -margeY;
        m_maxx = margeX;
        m_maxy = margeY;
        m_validbbox = true;
    }
    else
    {
        m_minx -= margeX;
        m_maxx += margeX;
        m_miny -= margeY;
        m_maxy += margeY;
    }
}

// Calculates if two boundingboxes intersect. If so, the function returns _ON.
// If they do not intersect, two scenario's are possible:
// other is outside this -> return _OUT
// other is inside this -> return _IN
OVERLAP a2dBoundingBox::Intersect( const a2dBoundingBox& other, double Marge ) const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );
    wxASSERT_MSG( other.m_validbbox, wxT( "invalid bbox" ) );


    if ( ( ( m_minx - Marge ) > ( other.m_maxx + Marge ) ) ||
            ( ( m_maxx + Marge ) < ( other.m_minx - Marge ) ) ||
            ( ( m_maxy + Marge ) < ( other.m_miny - Marge ) ) ||
            ( ( m_miny - Marge ) > ( other.m_maxy + Marge ) ) )
        return _OUT;

    // Check if other.bbox is inside this bbox
    if ( ( m_minx <= other.m_minx + Marge ) &&
            ( m_maxx >= other.m_maxx - Marge ) &&
            ( m_maxy >= other.m_maxy - Marge ) &&
            ( m_miny <= other.m_miny + Marge ) )
        return _IN;

    // Boundingboxes intersect
    return _ON;
}


// Checks if a line intersects the boundingbox
bool a2dBoundingBox::LineIntersect( const a2dPoint2D& begin, const a2dPoint2D& end ) const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );

    return ( bool )
           !( ( ( begin.m_y > m_maxy ) && ( end.m_y > m_maxy ) ) ||
              ( ( begin.m_y < m_miny ) && ( end.m_y < m_miny ) ) ||
              ( ( begin.m_x > m_maxx ) && ( end.m_x > m_maxx ) ) ||
              ( ( begin.m_x < m_minx ) && ( end.m_x < m_minx ) ) );
}


// Is the given point in the boundingbox ??
bool a2dBoundingBox::PointInBox( double x, double y, double Marge ) const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );

    if (  x >= ( m_minx - Marge ) && x <= ( m_maxx + Marge ) &&
            y >= ( m_miny - Marge ) && y <= ( m_maxy + Marge ) )
        return true;
    return false;
}

//
// Is the given point in the boundingbox ??
//
bool a2dBoundingBox::PointInBox( const a2dPoint2D& a, double Marge ) const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );

    return PointInBox( a.m_x, a.m_y, Marge );
}

// Is the given point in the boundingbox ??
bool a2dBoundingBox::PointOnBox( double x, double y, double Marge ) const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );

    if (  x >= ( m_minx - Marge ) && x <= ( m_maxx + Marge ) &&
            y >= ( m_miny - Marge ) && y <= ( m_maxy + Marge ) &&
            ( x <= ( m_minx + Marge ) || x >= ( m_maxx - Marge ) ||
              y <= ( m_miny + Marge ) || y >= ( m_maxy - Marge ) )
       )
        return true;
    return false;
}

a2dPoint2D a2dBoundingBox::GetCentre() const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );

    return a2dPoint2D( (m_maxx + m_minx)/2.0, (m_maxy + m_miny)/2.0 );
}

a2dPoint2D a2dBoundingBox::GetMin() const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );

    return a2dPoint2D( m_minx, m_miny );
}


a2dPoint2D a2dBoundingBox::GetMax() const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );

    return a2dPoint2D( m_maxx, m_maxy );
}

bool a2dBoundingBox::GetValid() const
{
    return m_validbbox;
}

double a2dBoundingBox::GetMinX() const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );
    return m_minx;
}

double a2dBoundingBox::GetMinY() const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );
    return m_miny;
}

double a2dBoundingBox::GetMaxX() const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );
    return m_maxx;
}

double a2dBoundingBox::GetMaxY() const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );
    return m_maxy;
}

double a2dBoundingBox::GetWidth() const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );
    return fabs( m_maxx - m_minx );
}

double a2dBoundingBox::GetHeight() const
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );
    return fabs( m_maxy - m_miny );
}

void a2dBoundingBox::SetMin( double px, double py )
{
    m_minx = px;
    m_miny = py;
    if ( !m_validbbox )
    {
        m_maxx = px;
        m_maxy = py;
        m_validbbox = true;
    }
}

void a2dBoundingBox::SetMax( double px, double py )
{
    m_maxx = px;
    m_maxy = py;
    if ( !m_validbbox )
    {
        m_minx = px;
        m_miny = py;
        m_validbbox = true;
    }
}

void a2dBoundingBox::SetValid( bool value )
{
    m_validbbox = value;
}

// usage : a_boundingbox.Translate(a_point);
const a2dBoundingBox& a2dBoundingBox::Translate( a2dPoint2D& offset )
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );

    m_minx += offset.m_x;
    m_maxx += offset.m_x;
    m_miny += offset.m_y;
    m_maxy += offset.m_y;
    return *this;
}

const a2dBoundingBox& a2dBoundingBox::Translate( double x, double y )
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );

    m_minx += x;
    m_maxx += x;
    m_miny += y;
    m_maxy += y;
    return *this;
}


// clears the bounding box settings
void a2dBoundingBox::Reset()
{
    m_minx = 0.0;
    m_maxx = 0.0;
    m_miny = 0.0;
    m_maxy = 0.0;
    m_validbbox = false;
}


void a2dBoundingBox::SetBoundingBox( const a2dPoint2D& a_point )
{
    m_minx = a_point.m_x;
    m_maxx = a_point.m_x;
    m_miny = a_point.m_y;
    m_maxy = a_point.m_y;
}

a2dBoundingBox& a2dBoundingBox::operator+( a2dBoundingBox& other )
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );
    wxASSERT_MSG( other.m_validbbox, wxT( "invalid bbox" ) );

    Expand( other );
    return *this;
}

const a2dBoundingBox& a2dBoundingBox::operator+=( const a2dBoundingBox& box )
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );
    wxASSERT_MSG( box.m_validbbox, wxT( "invalid bbox" ) );

    Expand( box );
    return *this;
}

// makes a boundingbox same as the other
a2dBoundingBox& a2dBoundingBox::operator=( const a2dBoundingBox& other )
{
    //DO not assert here, an invalid boudingbox should be copied without problems.
    //It is used for initializing in certain cases.
    //wxASSERT_MSG( other.m_validbbox, wxT("invalid bbox") );

    m_minx = other.m_minx;
    m_maxx = other.m_maxx;
    m_miny = other.m_miny;
    m_maxy = other.m_maxy;
    m_validbbox = other.m_validbbox;
    return *this;
}

void a2dBoundingBox::MapBbox( const a2dAffineMatrix& matrix )
{
    wxASSERT_MSG( m_validbbox, wxT( "invalid bbox" ) );

    if ( matrix.IsIdentity() )
        return;

    double x1, y1, x2, y2, x3, y3, x4, y4;

    matrix.TransformPoint( m_minx, m_miny, x1, y1 );
    matrix.TransformPoint( m_minx, m_maxy, x2, y2 );
    matrix.TransformPoint( m_maxx, m_maxy, x3, y3 );
    matrix.TransformPoint( m_maxx, m_miny, x4, y4 );

    m_minx = wxMin( x1, x2 );
    m_minx = wxMin( m_minx, x3 );
    m_minx = wxMin( m_minx, x4 );

    m_maxx = wxMax( x1, x2 );
    m_maxx = wxMax( m_maxx, x3 );
    m_maxx = wxMax( m_maxx, x4 );

    m_miny = wxMin( y1, y2 );
    m_miny = wxMin( m_miny, y3 );
    m_miny = wxMin( m_miny, y4 );

    m_maxy = wxMax( y1, y2 );
    m_maxy = wxMax( m_maxy, y3 );
    m_maxy = wxMax( m_maxy, y4 );
}

#ifdef _DEBUG

void a2dBoundingBox::Dump() const
{
    wxLogDebug( _T( "boundingbox minx=%f, miny=%f maxx=%f maxy=%f" ),  m_minx, m_miny, m_maxx, m_maxy );
    wxLogDebug( _T( "boundingbox     width=%f, height=%f" ),  GetWidth(), GetHeight() );
}

#endif


