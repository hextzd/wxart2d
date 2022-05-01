/*! \file artbase/src/polyver.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: polyver.cpp,v 1.56 2009/09/30 18:38:57 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include <float.h>
#include "wx/general/gen.h"
#include "wx/general/id.h"
#include "wx/artbase/polyver.h"
#include "wx/artbase/liner.h"
#include "wx/artbase/artglob.h"

/*
IMPLEMENT_DYNAMIC_CLASS(a2dLineSegmentProperty,a2dNamedProperty)

//----------------------------------------------------------------------------
// a2dLineSegmentProperty
//----------------------------------------------------------------------------

a2dLineSegmentProperty::a2dLineSegmentProperty(): a2dNamedProperty()
{
    m_segment = NULL;
}

a2dLineSegmentProperty::a2dLineSegmentProperty( const a2dPropertyIdLineSegment &id, a2dLineSegment* segment )
                        : a2dNamedProperty( id )
{
    m_segment = NULL;
}

a2dLineSegmentProperty::~a2dLineSegmentProperty()
{
}

a2dLineSegmentProperty::a2dLineSegmentProperty( const a2dLineSegmentProperty &other)
        :a2dNamedProperty( other )
{
    m_segment = other.m_segment;
}

a2dNamedProperty *a2dLineSegmentProperty::Clone( a2dObject::CloneOptions options ) const
{
    return new a2dLineSegmentProperty(*this);
};

void a2dLineSegmentProperty::Assign( const a2dNamedProperty &other )
{
    a2dLineSegmentProperty *propcast = wxStaticCast( &other, a2dLineSegmentProperty);
    m_segment = propcast->m_segment;
}

a2dLineSegmentProperty *a2dLineSegmentProperty::CreatePropertyFromString( const a2dPropertyIdLineSegment &id, const wxString &value )
{
    return new a2dLineSegmentProperty( id, NULL );
}

#if wxART2D_USE_CVGIO
void a2dLineSegmentProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    wxASSERT(0);
}

void a2dLineSegmentProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    wxASSERT(0);
}
#endif //wxART2D_USE_CVGIO
*/

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

a2dHit a2dHit::stock_nohit
( a2dHit::hit_none,   a2dHit::stroke1_none,    a2dHit::stroke2_none,      0, 0 );
a2dHit a2dHit::stock_fill
( a2dHit::hit_fill,   a2dHit::stroke1_none,    a2dHit::stroke2_none,      0, 0 );
a2dHit a2dHit::stock_strokeoutside
( a2dHit::hit_stroke, a2dHit::stroke1_outside, a2dHit::stroke2_edgeother, 0, 0 );
a2dHit a2dHit::stock_strokeinside
( a2dHit::hit_stroke, a2dHit::stroke1_inside,  a2dHit::stroke2_edgeother, 0, 0 );


//! Calculate the square distance between a point and a line
/*! This returns DBL_MAX if the point is beyond the edges */

double ClclDistSqrPntLine( const a2dPoint2D& p, const a2dPoint2D& p1, const a2dPoint2D& p2 )
{
    double dx12 = double( p2.m_x ) - double( p1.m_x );
    double dy12 = double( p2.m_y ) - double( p1.m_y );

    double dx1p = double( p.m_x ) - double( p1.m_x );
    double dy1p = double( p.m_y ) - double( p1.m_y );

    // Calculate the square length
    double length = dx12 * dx12 + dy12 * dy12;
    // Calculate the projection * length
    double projection = dx12 * dx1p + dy12 * dy1p;

    if( projection <= 0 || length == 0 )
    {
        // The point is beyond p1
        return DBL_MAX;
    }
    else if( projection >= length )
    {
        // note projection and length can be compared, because both contain an extra
        // length factor.
        // The point is beyond p2
        return DBL_MAX;
    }
    else
    {
        // The point is between p1 und p2

        // First check vertical and horizontal lines before doing sqrt and / operations
        if( dx12 == 0 )
            return dx1p * dx1p;
        if( dy12 == 0 )
            return dy1p * dy1p;

        // Note: we don't want the projection length, but the projection fraction
        // between 0..1. As projection is the projection length * length, we must devide
        // by length^2 here
        projection /= length;
        dx1p -= projection * dx12;
        dy1p -= projection * dy12;
        return dx1p * dx1p + dy1p * dy1p;
    }
}

A2DARTBASEDLLEXP bool InArc( double angle, double start, double end, bool clockwise )
{

    //if going counterclockwise from begin to end we pass trough middle => counterclockwise
    //if going counterclockwise from begin to end we do NOT pass trough middle => clockwise

    //rotate such that start = zero
    double mr, er;
    mr = angle - start;
    er = end - start;

    //make positive
    if ( mr < 0 ) mr += 2.0 * M_PI;
    if ( er < 0 ) er += 2.0 * M_PI;
    if ( mr > 2.0 * M_PI ) mr -= 2.0 * M_PI;
    if ( er > 2.0 * M_PI ) er -= 2.0 * M_PI;

    //middle gives direction
    if ( mr < er )
    {
        if ( clockwise )
            return false;
        return true;
    }
    else
    {
        if ( clockwise )
            return true;
        return false;
    }
}

// ---------------------------------------------------------------------------
// spline drawing code
// ---------------------------------------------------------------------------
static void gds_quadratic_spline( a2dVertexList* org, double a1, double b1, double a2, double b2,
                                  double a3, double b3, double a4, double b4, double aber );
static void gds_clear_stack();
static int gds_spline_pop( double* x1, double* y1, double* x2, double* y2, double* x3,
                           double* y3, double* x4, double* y4 );
static void gds_spline_push( double x1, double y1, double x2, double y2, double x3, double y3,
                             double x4, double y4 );

//----------------------------------------------------------------------------
// polygon/line segments
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// a2dLineSegment
//----------------------------------------------------------------------------

#ifdef CLASS_MEM_MANAGEMENT
a2dMemManager a2dLineSegment::sm_memManager( wxT( "a2dLineSegment memory manager" ) );
#endif //CLASS_MEM_MANAGEMENT

a2dLineSegment::a2dLineSegment( double x, double y )
{
    m_x = x;
    m_y = y;
    m_arc = false;
    m_arcPiece = false;
    m_bin = false;
    m_bin2 = false;
    m_refcount = 0;
    m_segtype = a2dNORMAL_SEG;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_ownerlist = 0;
#endif
}

a2dLineSegment::a2dLineSegment( const a2dPoint2D& point )
{
    m_x = point.m_x;
    m_y = point.m_y;
    m_arc = false;
    m_arcPiece = false;
    m_bin = false;
    m_bin2 = false;
    m_refcount = 0;
    m_segtype = a2dNORMAL_SEG;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_ownerlist = 0;
#endif
}

a2dLineSegment::a2dLineSegment( const a2dLineSegment& other )
{
    m_x = other.m_x;
    m_y = other.m_y;
    m_arc = other.m_arc;
    m_bin = other.m_bin;
    m_bin2 = other.m_bin2;
    m_arcPiece = other.m_arcPiece;

    m_refcount = 0;
    m_segtype = other.m_segtype;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_ownerlist = 0;
#endif
}

a2dLineSegment::~a2dLineSegment()
{
    wxASSERT_MSG(  m_refcount == 0, wxT( "deleting a2dLineSegment while referenced" ) );
}

a2dLineSegment* a2dLineSegment::Clone()
{
    a2dLineSegment* a = new a2dLineSegment( *this );
    return a;
}

double a2dLineSegment::Length( const a2dLineSegment& prev )
{
    return sqrt( pow( prev.m_x - m_x, 2 ) + pow( prev.m_y - m_y, 2 ) );
}

a2dBoundingBox a2dLineSegment::GetBbox( const a2dLineSegment& prev, const a2dAffineMatrix& WXUNUSED( lworld ) ) const
{
    a2dBoundingBox bbox;
    bbox.Expand( prev.m_x, prev.m_y );
    bbox.Expand( m_x, m_y );
    return bbox;
}

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)

void a2dLineSegment::DumpOwners()
{
    wxLogDebug( _T( "Owner list (%d) for (a2dLineSegment*)0x%p" ), m_refcount, this );
    for( a2dLineSegmentPtr* crnt = m_ownerlist; crnt; crnt = crnt->m_next )
    {
        if( crnt->m_owner )
        {
            switch( crnt->m_ownertype )
            {
                case a2dSmrtPtr<a2dLineSegment>::owner_none:
                    wxLogDebug( _T( "SmrtPtr @ 0x%p in unknown %p" ), crnt, crnt->m_owner );
                    break;
                case a2dSmrtPtr<a2dLineSegment>::owner_wxobject:
                    wxLogDebug( _T( "SmrtPtr @ 0x%p in (%s*)0x%p offs=0x%x" ), crnt, ( ( wxObject* )crnt->m_owner )->GetClassInfo()->GetClassName(), crnt->m_owner, ( char* )crnt - ( char* )crnt->m_owner );
                    break;
                case a2dSmrtPtr<a2dLineSegment>::owner_smartpointerlist:
                    wxLogDebug( _T( "SmrtPtr @ 0x%p in (wxSmrtPtrNode<a2dLineSegment>*)0x%p offs=0x%x" ), crnt, crnt->m_owner, ( char* )crnt - ( char* )crnt->m_owner );
                    break;
            }
        }
        else
            wxLogDebug( _T( "SmrtPtr @ 0x%p in unknown" ), crnt );
    }
}

#endif

//----------------------------------------------------------------------------
// a2dArcSegment
//----------------------------------------------------------------------------

a2dArcSegment::a2dArcSegment( double x1, double y1, double x2, double y2 )
    : a2dLineSegment( x1, y1 )
{
    m_arc = true;
    m_x2 = x2;
    m_y2 = y2;
}

a2dArcSegment::a2dArcSegment( const a2dLineSegment& prev, double xc, double yc, double angle )
    : a2dLineSegment( 0, 0 )
{
    m_arc = true;

    double radius = sqrt( pow( prev.m_x - xc, 2 ) + pow( prev.m_y - yc, 2 ) );

    double endrad = atan2( prev.m_y - yc, prev.m_x - xc ) + wxDegToRad( angle );

    m_x = xc + radius * cos( endrad );
    m_y = yc + radius * sin( endrad );

    double midrad = atan2( prev.m_y - yc, prev.m_x - xc ) + wxDegToRad( angle / 2.0 );

    m_x2 = xc + radius * cos( midrad );
    m_y2 = yc + radius * sin( midrad );
}

a2dArcSegment::a2dArcSegment( const a2dLineSegment& prev, double xc, double yc, double x1, double y1, double x2, double y2 )
    : a2dLineSegment( x1, y1 )
{
    m_arc = true;

    double radius = sqrt( pow( prev.m_x - xc, 2 ) + pow( prev.m_y - yc, 2 ) );

    double endrad = atan2( m_y - yc, m_x - xc );

    m_x = xc + radius * cos( endrad );
    m_y = yc + radius * sin( endrad );

    double midrad = atan2( y2 - yc, x2 - xc );

    m_x2 = xc + radius * cos( midrad );
    m_y2 = yc + radius * sin( midrad );
}

a2dArcSegment::a2dArcSegment( const a2dArcSegment& other ): a2dLineSegment( other )
{
    m_x2 = other.m_x2;
    m_y2 = other.m_y2;
    m_arc = true;
}

a2dArcSegment::~a2dArcSegment()
{}

a2dLineSegment* a2dArcSegment::Clone()
{
    a2dArcSegment* a = new a2dArcSegment( m_x, m_y, m_x2, m_y2 );
    return a;
}

void a2dArcSegment::SetMidPoint( const a2dLineSegment& prev, double xm, double ym )
{
    a2dPoint2D middleOrProj = a2dPoint2D( ( prev.m_x + m_x ) * 0.5, ( prev.m_y + m_y ) * 0.5 );
    a2dLine line( prev.m_x, prev.m_y, m_x, m_y );
    line.CalculateLineParameters();
    a2dLine* middleLine = line.CreatePerpendicularLineAt( middleOrProj );
    middleOrProj = a2dPoint2D( xm, ym );
    middleOrProj = middleLine->ProjectedPoint( middleOrProj );
    delete middleLine;
    m_x2 = middleOrProj.m_x;
    m_y2 = middleOrProj.m_y;
}

double a2dArcSegment::Length( const a2dLineSegment& prev )
{
    double len, radius, center_x, center_y, beginrad, midrad, endrad, phit;

    if ( CalcR( prev, radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
    {
        len = phit * radius;
    }
    else
        len = sqrt( pow( prev.m_x - m_x, 2 ) + pow( prev.m_y - m_y, 2 ) );

    return len;
}

void a2dArcSegment::CalcMidPoint( const a2dLineSegment& prev, double center_x, double center_y, double radius, bool clockwise )
{
    double start = atan2( prev.m_y - center_y, prev.m_x - center_x );
    double end   = atan2( m_y - center_y, m_x - center_x );

    double b = ( prev.m_x - m_x );
    double a = ( prev.m_y - m_y );
    double l = sqrt( a * a + b * b );
    a /= l; b /= l;

    m_x2 = center_x - radius * b;
    m_y2 = center_y - radius * a;

    double mid   = atan2( m_y2 - center_y, m_x2 - center_x );

    if ( InArc( mid, start, end, clockwise ) )
        return;

    m_x2 = center_x + radius * b;
    m_y2 = center_y + radius * a;
}

bool a2dArcSegment::CalcR( const a2dLineSegment& prev, double& radius, double& center_x, double& center_y, double& beginrad, double& midrad, double& endrad, double& phit, double marge ) const
{
    return ::CalcR( prev.m_x, prev.m_y, m_x2, m_y2, m_x, m_y,
                    radius, center_x, center_y, beginrad, midrad, endrad, phit, marge );
}

double a2dArcSegment::GetOx( const a2dLineSegment& prev ) const
{
    double ax, ay, bx, by, cx, cy;
    double ax2, ay2, bx2, by2, cx2, cy2;
    double d;
    bool straight = false;

    ax = prev.m_x;   ay = prev.m_y;  ax2 = pow( ax, 2 ); ay2 = pow( ay, 2 );
    bx = m_x2;       by = m_y2;      bx2 = pow( bx, 2 ); by2 = pow( by, 2 );
    cx = m_x;        cy = m_y;       cx2 = pow( cx, 2 ); cy2 = pow( cy, 2 );

    if ( ax == cx && ay == cy )
    {
        //assume the middle is exact 180 opposite.
        return ( bx + ax ) / 2.0;
    }
    else
    {
        d = 2 * ( ay * cx + by * ax - by * cx - ay * bx - cy * ax + cy * bx );

        if ( fabs( d ) < a2dACCUR ) // (infinite radius)
        {
            d = a2dACCUR;
            straight = true;
        }

        return (  by * ax2 - cy * ax2 - by2 * ay + cy2 * ay + bx2 * cy + ay2 * by
                  + cx2 * ay - cy2 * by - cx2 * by - bx2 * ay + by2 * cy - ay2 * cy
               ) / d;
    }
}

double a2dArcSegment::GetOy( const a2dLineSegment& prev ) const
{
    double ax, ay, bx, by, cx, cy;
    double ax2, ay2, bx2, by2, cx2, cy2;
    double d;
    bool straight = false;

    ax = prev.m_x;   ay = prev.m_y;  ax2 = pow( ax, 2 ); ay2 = pow( ay, 2 );
    bx = m_x2;       by = m_y2;      bx2 = pow( bx, 2 ); by2 = pow( by, 2 );
    cx = m_x;        cy = m_y;       cx2 = pow( cx, 2 ); cy2 = pow( cy, 2 );

    if ( ax == cx && ay == cy )
    {
        //assume the middle is exact 180 opposite.
        return ( by + ay ) / 2.0;
    }
    else
    {
        d = 2 * ( ay * cx + by * ax - by * cx - ay * bx - cy * ax + cy * bx );

        if ( fabs( d ) < a2dACCUR ) // (infinite radius)
        {
            d = a2dACCUR;
            straight = true;
        }

        return (  ax2 * cx + ay2 * cx + bx2 * ax - bx2 * cx + by2 * ax - by2 * cx
                  - ax2 * bx - ay2 * bx - cx2 * ax + cx2 * bx - cy2 * ax + cy2 * bx
               ) / d ;
    }
}

a2dPoint2D a2dArcSegment::GetOrigin( const a2dLineSegment& prev ) const
{
    double ax, ay, bx, by, cx, cy;
    double ax2, ay2, bx2, by2, cx2, cy2;
    double d;
    bool straight = false;

    ax = prev.m_x;   ay = prev.m_y;  ax2 = pow( ax, 2 ); ay2 = pow( ay, 2 );
    bx = m_x2;       by = m_y2;      bx2 = pow( bx, 2 ); by2 = pow( by, 2 );
    cx = m_x;        cy = m_y;       cx2 = pow( cx, 2 ); cy2 = pow( cy, 2 );

    if ( ax == cx && ay == cy )
    {
        //assume the middle is exact 180 opposite.
        return a2dPoint2D( ( bx + ax ) / 2.0, ( by + ay ) / 2.0 );
    }
    else
    {
        d = 2 * ( ay * cx + by * ax - by * cx - ay * bx - cy * ax + cy * bx );

        if ( fabs( d ) < a2dACCUR ) // (infinite radius)
        {
            d = a2dACCUR;
            straight = true;
        }

        double x = (  by * ax2 - cy * ax2 - by2 * ay + cy2 * ay + bx2 * cy + ay2 * by
                      + cx2 * ay - cy2 * by - cx2 * by - bx2 * ay + by2 * cy - ay2 * cy
                   ) / d;

        double y = (  ax2 * cx + ay2 * cx + bx2 * ax - bx2 * cx + by2 * ax - by2 * cx
                      - ax2 * bx - ay2 * bx - cx2 * ax + cx2 * bx - cy2 * ax + cy2 * bx
                   ) / d ;
        return a2dPoint2D( x, y );
    }
}

a2dBoundingBox a2dArcSegment::GetBbox( const a2dLineSegment& prev, const a2dAffineMatrix& lworld ) const
{
    a2dBoundingBox bbox;
    double radius, centerx, centery, start, mid, end, phit;

    if ( ! CalcR( prev, radius, centerx, centery, start, mid, end, phit ) )
    {
        bbox.Expand( prev.m_x, prev.m_y );
        bbox.Expand( m_x, m_y );
        return bbox;
    }

    if ( !lworld.IsIdentity() )
    {
        double alphax = atan2( lworld.GetValue( 1, 0 ), lworld.GetValue( 0, 0 ) );
        double alphay = atan2( lworld.GetValue( 1, 1 ), lworld.GetValue( 0, 1 ) );

        double x, y;

        //start point
        lworld.TransformPoint( centerx + radius * cos ( start ), centery + radius * sin ( start ), x, y );
        bbox.Expand( x, y );
        //end point
        lworld.TransformPoint( centerx + radius * cos ( end ), centery + radius * sin ( end ), x, y );
        bbox.Expand( x, y );

        //outer points
        if ( InArc( alphax, start, end, phit < 0 ) )
        {
            lworld.TransformPoint( centerx + radius * cos ( alphax ), centery + radius * sin ( alphax ), x, y );
            bbox.Expand( x, y );
        }
        alphax += wxPI;
        if ( InArc( alphax, start, end, phit < 0 ) )
        {
            lworld.TransformPoint( centerx + radius * cos ( alphax ), centery + radius * sin ( alphax ), x, y );
            bbox.Expand( x, y );
        }
        alphax -= 2 * wxPI;
        if ( InArc( alphax, start, end, phit < 0 ) )
        {
            lworld.TransformPoint( centerx + radius * cos ( alphax ), centery + radius * sin ( alphax ), x, y );
            bbox.Expand( x, y );
        }
        if ( InArc( alphay, start, end, phit < 0 ) )
        {
            lworld.TransformPoint( centerx + radius * cos ( alphay ), centery + radius * sin ( alphay ), x, y );
            bbox.Expand( x, y );
        }
        alphay += wxPI;
        if ( InArc( alphay, start, end, phit < 0 ) )
        {
            lworld.TransformPoint( centerx + radius * cos ( alphay ), centery + radius * sin ( alphay ), x, y );
            bbox.Expand( x, y );
        }
        alphay -= 2 * wxPI;
        if ( InArc( alphay, start, end, phit < 0 ) )
        {
            lworld.TransformPoint( centerx + radius * cos ( alphay ), centery + radius * sin ( alphay ), x, y );
            bbox.Expand( x, y );
        }
    }
    else
    {
        //start point
        bbox.Expand( centerx + radius * cos ( start ), centery + radius * sin ( start ) );
        //end point
        bbox.Expand( centerx + radius * cos ( end ), centery + radius * sin ( end ) );

        //outer points
        if ( InArc( wxPI, start, end, phit < 0 ) )
            bbox.Expand( centerx - radius, centery );
        if ( InArc( 1.5 * wxPI, start, end, phit < 0 ) )
            bbox.Expand( centerx, centery - radius );
        if ( InArc( 0, start, end, phit < 0 ) )
            bbox.Expand( centerx + radius, 0 );
        if ( InArc( wxPI / 2, start, end, phit < 0 ) )
            bbox.Expand( centerx, centery + radius );
    }
    return bbox;
}

//----------------------------------------------------------------------------
// a2dVertexArray
//----------------------------------------------------------------------------

a2dVertexArray::a2dVertexArray()
{}

a2dVertexArray::a2dVertexArray( const a2dVertexArray& other )
{
    *this = other;
}

a2dVertexArray::a2dVertexArray( const a2dVertexList& other )
{
    clear();
    for( a2dVertexList::const_iterator iter = other.begin(); iter != other.end(); ++iter )
    {
        a2dLineSegmentPtr seg = ( *iter )->Clone();
        push_back( seg );
    }
}

a2dVertexArray::~a2dVertexArray()
{}

void a2dVertexArray::RemoveAt( size_t index )
{
    size_t i = 0;
    for( a2dVertexArray::iterator iter = begin(); iter != end(); ++iter )
    {
        if ( i == index )
        {
            erase( iter );
            break;
        }
        i++;
    }
}

void a2dVertexArray::Insert( a2dLineSegment* segment, size_t index )
{
    size_t i = 0;
    for( a2dVertexArray::iterator iter = begin(); iter != end(); ++iter )
    {
        if ( i == index )
        {
            insert( iter, segment );
            break;
        }
        i++;
    }
}

a2dVertexArray& a2dVertexArray::operator=( const a2dVertexArray& other )
{
    clear();
    for( a2dVertexArray::const_iterator iter = other.begin(); iter != other.end(); ++iter )
    {
        a2dLineSegmentPtr seg = ( *iter )->Clone();
        push_back( seg );
    }

    return *this;
}

a2dVertexArray& a2dVertexArray::operator=( const a2dVertexList& other )
{
    clear();
    for( a2dVertexList::const_iterator iter = other.begin(); iter != other.end(); ++iter )
    {
        a2dLineSegmentPtr seg =  ( *iter )->Clone();
        push_back( seg );
    }
    return *this;
}

a2dLineSegmentPtr a2dVertexArray::GetPreviousAround( wxUint32 index ) const
{
    if ( index == 0 )
        return  Item( size() );
    return Item( index - 1 );
}

a2dLineSegmentPtr a2dVertexArray::GetNextAround( wxUint32 index ) const
{
    if ( index == size() )
        return  Item( 0 );
    return Item( index + 1 );
}

bool a2dVertexArray::HasArcs() const
{
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        if ( Item( i )->GetArc()  )
            return true;
    }
    return false;
}

double a2dVertexArray::Length() const
{
    double len = 0;
    unsigned int i;
    for ( i = 1; i < size(); i++ )
    {
        len = len + Item( i )->Length( *( Item( i - 1 ) ) );
    }

    return len;
}


void a2dVertexArray::AddPoint( const a2dPoint2D& point, bool atEnd )
{
    if ( atEnd )
        push_back( new a2dLineSegment( point.m_x, point.m_y ) );
    else
        insert( begin(), new a2dLineSegment( point.m_x, point.m_y ) );
}

void a2dVertexArray::AddPoint( double x, double y, bool atEnd )
{
    if ( atEnd )
        push_back( new a2dLineSegment( x, y ) );
    else
        insert( begin(), new a2dLineSegment( x, y ) );
}

void a2dVertexArray::SetPointAdjustArcs( unsigned int n, double x, double y, bool polygon )
{
    Item( n )->m_x = x;
    Item( n )->m_y = y;

    if ( Item( n )->GetArc() )
    {
        a2dArcSegment* cseg = ( a2dArcSegment* ) Item( n ).Get();

        a2dLineSegmentPtr prev;
        double radius, center_x, center_y, beginrad, midrad, endrad, phit;
        if ( polygon )
        {
            prev = Item( ( n - 1 ) % size() );

            if ( cseg->CalcR( *prev, radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                cseg->CalcMidPoint( *prev, center_x, center_y, radius, phit < 0 );
        }
        else
        {
            if ( n > 1 )
            {
                prev = Item( n - 1 );
                if ( cseg->CalcR( *prev, radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                    cseg->CalcMidPoint( *prev, center_x, center_y, radius, phit < 0 );
            }
        }
    }

    a2dLineSegmentPtr next;
    if ( polygon )
        next = Item( ( n + 1 ) % size() );
    else
    {
        if ( n < size() - 1 )
            next = Item( n + 1 );
        else
            return;
    }

    if ( next->GetArc() )
    {
        a2dArcSegment* cseg = ( a2dArcSegment* ) next->GetArc();

        double radius, center_x, center_y, beginrad, midrad, endrad, phit;

        if ( cseg->CalcR( *( Item( n ) ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
            cseg->CalcMidPoint( *( Item( n ) ), center_x, center_y, radius, phit < 0 );
    }
}

double a2dVertexArray::CalcArea() const
{
    if ( size() < 3 )
        return 0.0;

    double area = 0.0;

    a2dVertexArray* workOn = const_cast<a2dVertexArray*>( this );
    double miny = workOn->Item( 0 )->m_y;
    bool hasarcs = HasArcs();
    if ( hasarcs )
    {
        workOn = new a2dVertexArray( *this );
        workOn->ConvertToLines();
    }

    a2dLineSegmentPtr seg;
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        seg = workOn->Item( i );
        if ( seg->m_y < miny )
            miny = seg->m_y;
    }
    if ( miny > 0 )
        miny = 0;

    for ( int i = 0; i < workOn->size(); i++ )
    {
        double x = workOn->Item( i )->m_x;
        double y = workOn->Item( i )->m_y - miny;
        seg = Item( i + 1 != workOn->size() ? i + 1 : 0 );
        area += ( seg->m_y - miny ) * x;
        area -= seg->m_x * y;
    }

    if ( hasarcs )
        delete workOn;

    return fabs( area / 2.0 );
}

void a2dVertexArray::Transform( const a2dAffineMatrix& cworld )
{
    if ( !cworld.IsIdentity() )
    {
        if ( !cworld.IsTranslate() )
            ConvertToLines();

        unsigned int i;
        for ( i = 0; i < size(); i++ )
        {
            a2dLineSegmentPtr seg = Item( i );
            if ( seg->GetArc() )
            {
                a2dArcSegment* cseg = ( a2dArcSegment* ) seg.Get();
                cworld.TransformPoint( cseg->m_x2, cseg->m_y2, cseg->m_x2, cseg->m_y2 );
            }
            cworld.TransformPoint( seg->m_x, seg->m_y, seg->m_x, seg->m_y );
        }
    }
}

a2dBoundingBox a2dVertexArray::GetBbox( const a2dAffineMatrix& lworld )
{
    a2dBoundingBox bbox;

    double x, y;
    lworld.TransformPoint( 0, 0, x, y );
    if ( !size() )
        bbox.Expand( x, y );
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        a2dLineSegmentPtr seg = Item( i );
        if ( seg->GetArc() )
            bbox.Expand( seg->GetBbox( *( Item( i ? i - 1 : 0 ) ), lworld ) );
        else
        {
            lworld.TransformPoint( seg->m_x, seg->m_y, x, y );
            bbox.Expand( x, y );
        }
    }
    return bbox;
}

void a2dVertexArray::ConvertToLines( double aberation )
{
    double xw, yw;
    a2dLineSegmentPtr segn;

    a2dVertexArray::iterator iterp = end();
    a2dVertexArray::iterator iter = begin();
    while ( iter != end() )
    {
        a2dLineSegment* seg = *iter;
        if ( seg->GetArc() )
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;

            double radius, center_x, center_y, beginrad, midrad, endrad, phit;

            if ( cseg->CalcR( *( *iterp ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
            {
                double dphi;
                unsigned int segments = 20;
                if ( aberation == 0 )
                    Aberration( radius / 200, phit, radius , dphi, segments );
                //a2dGlobals->Aberration( phit, radius , dphi, segments );
                else
                    Aberration( aberation, phit, radius , dphi, segments );

                double theta = beginrad;
                unsigned int step;

                for ( step = 0; step < segments + 1; step++ )
                {
                    xw = center_x + radius * cos ( theta );
                    yw = center_y + radius * sin ( theta );
                    segn = new a2dLineSegment( xw, yw );

                    iter = insert( iter, segn );
                    iter++;
                    theta = theta + dphi;
                }
            }
            else
            {
                segn = new a2dLineSegment( cseg->m_x, cseg->m_y );
                iter = insert( iter, segn );
                iter++;
            }

            iter--; //last inserted segment
            iterp = iter;
            iter++;
            iter = erase( iter );
        }
        else
        {
            iterp = iter;
            iter++;
        }
    }
}

a2dVpath* a2dVertexArray::ConvertToVpath( bool arc, bool closed )
{
    a2dVpath* vpathsegments = new a2dVpath();

    double xw, yw;
    unsigned int i;

    for ( i = 0; i < size(); i++ )
    {
        a2dLineSegmentPtr seg = Item( i );
        if ( seg->GetArc() )
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg.Get();
            if ( arc )
            {
                a2dVpathArcSegment* arcseg;
                if ( i == 0 )
                {
                    a2dVpathSegment* segn = new a2dVpathSegment( cseg->m_x, cseg->m_y, a2dPATHSEG_MOVETO  );
                    vpathsegments->Add( segn );
                    arcseg = new a2dVpathArcSegment( cseg->m_x, cseg->m_y, cseg->m_x2, cseg->m_y2, a2dPATHSEG_ARCTO );
                }
                else if ( i == size() - 1 )
                    arcseg = new a2dVpathArcSegment( cseg->m_x, cseg->m_y, cseg->m_x2, cseg->m_y2, a2dPATHSEG_ARCTO, closed ? a2dPATHSEG_END_CLOSED :  a2dPATHSEG_END_OPEN );
                else
                    arcseg = new a2dVpathArcSegment( cseg->m_x, cseg->m_y, cseg->m_x2, cseg->m_y2, a2dPATHSEG_ARCTO );
                vpathsegments->Add( arcseg );
            }
            else
            {
                // to line segments
                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                if ( cseg->CalcR( *( Item( i ? i - 1 : 0 ) ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                {
                    double dphi;
                    unsigned int segments = 20;
                    Aberration( radius / 200, phit, radius , dphi, segments );
                    //a2dGlobals->Aberration( phit, radius , dphi, segments );

                    double theta = beginrad;
                    unsigned int step;

                    for ( step = 0; step < segments + 1; step++ )
                    {
                        xw = center_x + radius * cos ( theta );
                        yw = center_y + radius * sin ( theta );
                        a2dVpathSegment* lineseg = new a2dVpathSegment( xw, yw );
                        vpathsegments->Add( lineseg );
                        theta = theta + dphi;
                    }
                }
                else
                {
                    a2dVpathSegment* segn = new a2dVpathSegment( cseg->m_x, cseg->m_y );
                    vpathsegments->Add( segn );
                }
            }
        }
        else
        {
            a2dVpathSegment* lineseg;
            if ( i == 0 )
                lineseg = new a2dVpathSegment( seg->m_x, seg->m_y , a2dPATHSEG_MOVETO );
            else if ( i == size() - 1 )
                lineseg = new a2dVpathSegment( seg->m_x, seg->m_y, a2dPATHSEG_LINETO, closed ? a2dPATHSEG_END_CLOSED :  a2dPATHSEG_END_OPEN );
            else
                lineseg = new a2dVpathSegment( seg->m_x, seg->m_y, a2dPATHSEG_LINETO );
            vpathsegments->Add( lineseg );
        }
    }
    return vpathsegments;
}

a2dVertexArray* a2dVertexArray::Contour( double distance, a2dPATH_END_TYPE pathtype )
{
    a2dVertexArray* converted = new a2dVertexArray();
    a2dPoint2D _first;
    a2dPoint2D _middle;
    a2dPoint2D _end;
    a2dPoint2D offsetpointleft;
    a2dPoint2D offsetpointright;

    unsigned int insertAt = converted->size();
    a2dVertexArray::iterator insertHere = end();
    unsigned int segments = size();

    //create start of subpart
    a2dLineSegmentPtr seg = Item( 0 );
    if ( segments == 1 )
    {
        converted->push_back( new a2dLineSegment( seg->m_x - distance, seg->m_y ) );
        switch ( pathtype )
        {
            case a2dPATH_END_SQAURE:
            {
                //not defined
            }
            break;
            case a2dPATH_END_ROUND:
            {
                converted->push_back( new a2dArcSegment( seg->m_x + distance, seg->m_y, seg->m_x - distance, seg->m_y ) );
            }
            break;
            case a2dPATH_END_SQAURE_EXT:
            {
                //not defined
            }
            break;
            default:
                break;
        }
        return converted;
    }
    else
    {
        //first begin point
        _first = a2dPoint2D( seg->m_x, seg->m_y );
        a2dLineSegmentPtr segnext = Item( 1 );
        _middle = a2dPoint2D( segnext->m_x, segnext->m_y );
        a2dLine line1( _first, _middle );
        line1.CalculateLineParameters();
        offsetpointleft = _first;
        offsetpointright = _first;
        line1.Virtual_Point( offsetpointleft, distance );
        line1.Virtual_Point( offsetpointright, -distance );

        switch ( pathtype )
        {
            case a2dPATH_END_SQAURE:
            {
                converted->push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
                insertHere = converted->insert( insertHere, new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
            }
            break;
            case a2dPATH_END_ROUND:
            {
                a2dLine  lineoffset( offsetpointright, offsetpointleft );
                lineoffset.CalculateLineParameters();
                a2dPoint2D offsetpointfirst;

                offsetpointfirst = _first;
                lineoffset.Virtual_Point( offsetpointfirst, distance );
                converted->push_back( new a2dArcSegment( offsetpointleft.m_x, offsetpointleft.m_y,
                                      offsetpointfirst.m_x, offsetpointfirst.m_y ) );
                insertHere = converted->insert( insertHere, new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
                break;
            }
            case a2dPATH_END_SQAURE_EXT:
            {
                a2dLine  lineoffset( offsetpointright, offsetpointleft );
                lineoffset.CalculateLineParameters();
                lineoffset.Virtual_Point( offsetpointleft, distance );
                lineoffset.Virtual_Point( offsetpointright, distance );

                converted->push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
                insertHere = converted->insert( insertHere, new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
                break;
            }
            default:
                break;
        }

    }

    //in between start and end
    unsigned int i;
    for ( i = 1; i < segments - 1; i++ )
    {
        a2dLineSegmentPtr seg = Item( i - 1 );
        _first = a2dPoint2D( seg->m_x, seg->m_y );
        a2dLineSegmentPtr segnext = Item( i );
        _middle = a2dPoint2D( segnext->m_x, segnext->m_y );
        a2dLine line1( _first, _middle );
        a2dLineSegmentPtr segend = Item( i + 1 );
        _end = a2dPoint2D( segend->m_x, segend->m_y );
        a2dLine line2( _middle, _end );
        line1.CalculateLineParameters();
        line2.CalculateLineParameters();

        OUTPRODUCT _outproduct;
        _outproduct = line1.OutProduct( line2, 1e-9 );

        switch ( _outproduct )
        {
                // Line 2 lies on  leftside of this line
            case R_IS_RIGHT :
            {
                line1.OffsetContour( line2, distance, offsetpointleft );
                line1.OffsetContour( line2, -distance, offsetpointright );
            }
            break;
            case R_IS_LEFT :
            {
                line1.OffsetContour( line2, distance, offsetpointleft );
                line1.OffsetContour( line2, -distance, offsetpointright );
            }
            break;
            // Line 2 lies on this line
            case R_IS_ON     :
            {
                offsetpointleft = _middle;
                offsetpointright = _middle;
                line1.Virtual_Point( offsetpointleft, distance );
                line1.Virtual_Point( offsetpointright, -distance );
            }
            break;
            default:
                wxFAIL_MSG( wxT( "wrong line code" ) );
                break;
        }//end switch

        converted->push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
        insertHere = converted->insert( insertHere, new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
    }

    seg = Item( i - 1 ); //i = start+segments-1

    //first begin point
    _first = a2dPoint2D( seg->m_x, seg->m_y );
    a2dLineSegmentPtr segnext = Item( i );
    _end = a2dPoint2D( segnext->m_x, segnext->m_y );
    a2dLine line1( _first, _end );
    line1.CalculateLineParameters();
    offsetpointleft = _end;
    offsetpointright = _end;
    line1.Virtual_Point( offsetpointleft, distance );
    line1.Virtual_Point( offsetpointright, -distance );

    switch ( pathtype )
    {
        case a2dPATH_END_SQAURE:
        {
            converted->push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
            insertHere = converted->insert( insertHere, new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
        }
        break;
        case a2dPATH_END_ROUND:
        {
            a2dLine  lineoffset( offsetpointleft, offsetpointright );
            lineoffset.CalculateLineParameters();
            a2dPoint2D offsetpointend;

            offsetpointend = _end;
            lineoffset.Virtual_Point( offsetpointend, distance );
            converted->push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
            converted->push_back( new a2dArcSegment( offsetpointright.m_x, offsetpointright.m_y,
                                  offsetpointend.m_x, offsetpointend.m_y ) );
            insertHere = converted->insert( insertHere, new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
            break;
        }
        case a2dPATH_END_SQAURE_EXT:
        {
            a2dLine  lineoffset( offsetpointright, offsetpointleft );
            lineoffset.CalculateLineParameters();
            lineoffset.Virtual_Point( offsetpointleft, -distance );
            lineoffset.Virtual_Point( offsetpointright, -distance );

            converted->push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
            insertHere = converted->insert( insertHere, new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
            break;
        }
        default:
            break;
    }

    return converted;
}

a2dVertexArray* a2dVertexArray::ConvertSplinedPolygon( double Aber ) const
{
    a2dVertexList h;
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        h.push_back( Item( i )->Clone() );
    }

    //TODO need an array version for speed
    h.ConvertIntoSplinedPolygon( Aber );

    unsigned int n = h.size();
    a2dVertexArray* spoints = new a2dVertexArray;

    a2dVertexList::iterator iter = h.begin();
    for ( i = 0; i < n; i++ )
    {
        spoints->push_back( ( *iter ) );
        iter = h.erase( iter );
    }

    return spoints;
}

a2dVertexArray* a2dVertexArray::ConvertSplinedPolyline( double Aber ) const
{
    a2dVertexList h;
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        h.push_back( Item( i )->Clone() );
    }

    //TODO need an array version for speed
    h.ConvertIntoSplinedPolyline( Aber );

    unsigned int n = h.size();
    a2dVertexArray* spoints = new a2dVertexArray;

    a2dVertexList::iterator iter = h.begin();
    for ( i = 0; i < n; i++ )
    {
        spoints->push_back( ( *iter )->Clone() );
        iter = h.erase( iter );
    }
    return spoints;
}


/*! This function checks if a point is inside, outside or on the stroke of a polygon.
This is done intersecting a vertical line through the test point with every edge of
the polygon. Then the number of intersections above (y+) of the test point is counted
+1 for left to right and -1 for right to left lines.
There is an almost identical function for point list polygons. These functions should
be kept identical except of the first two lines the handle lists or arrays.
*/

a2dHit a2dVertexArray::HitTestPolygon( const a2dPoint2D& ptest, double margin )
{
    wxASSERT( margin >= 0 );

    a2dPoint2D p1, p2, pm;

    double minDistSqrVertex = margin * margin;
    double minDistSqrStroke = margin * margin;
    bool vertexhit = false;
    int intersection_count = 0;
    a2dHit rslt;

    //iterate across points until we are sure that the given point is in or out
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        // Get the two points of edge #i
        p1 = Item( i )->GetPoint();
        if ( i == size() - 1 )
            p2 = Item( 0 )->GetPoint();
        else
            p2 = Item( i + 1 )->GetPoint();

        //--------------------------------------------------------------------
        // From here on, this function is identical to the hittest for
        // point array polygons. Keep these two identical !!
        //--------------------------------------------------------------------

        // check relative horizontal positions of line end points and point
        double xmin;
        double xmax;
        int direction;
        // Note: the case p1.m_x == p2.m_x is ignored during direction counting
        // See below.
        if( p1.m_x < p2.m_x )
        {
            xmin = p1.m_x;
            xmax = p2.m_x;
            direction = 1;
        }
        else
        {
            xmin = p2.m_x;
            xmax = p1.m_x;
            direction = -1;
        }

        if( ptest.m_x < xmin - margin || ptest.m_x > xmax + margin )
            continue;

        // check relative horizontal positions of line end points and point
        double ymin;
        double ymax;
        if( p1.m_y < p2.m_y )
        {
            ymin = p1.m_y;
            ymax = p2.m_y;
        }
        else
        {
            ymin = p2.m_y;
            ymax = p1.m_y;
        }

        // if ptest its y is beneath the minimum y of the segment, the ray does
        // intersect if x p1 and x p2 are on opposite sides.
        if( ptest.m_y < ymin - margin )
        {
            // The test point is definitely below the margin.
            // This cannot be a stroke hit
            // but the vertical line through ptest can intersect the edge
            if( ptest.m_x >= xmin && ptest.m_x < xmax && p1.m_x != p2.m_x )
            {
                intersection_count += direction;
            }
        }
        else if( ptest.m_y <= ymax + margin )
        {
            // The test point ptest is inside the margin extended bounding box+margin of the edge
            // This means we make vertex and stroke hit tests
            // and an intersection test between edge and the vertical line through ptest

            // Vertex hit test for point 1
            //
            // Note1: double ifs are MUCH slower than double calculations (at least on intel)
            // So it is faster calculating the distance square than doing a rectangle test
            //
            // Note2: vertex hits have higher priority than edge hits
            //
            // Note3: every vertex is once point1 and once point 2, so only one point needs
            // to be tested.
            //
            // Note4: Even if there is a stroke or vertex hit, serach is continued, because
            // there could be better hit. A stroke or vertex hit is rarely a random event,
            // so it doesn't make much sense to optimize this.

            double distSqr = ClclDistSqrPntPnt( ptest, p1 );

            if( distSqr < minDistSqrVertex )
            {
                minDistSqrVertex = distSqr;
                rslt.m_hit = a2dHit::hit_stroke;
                rslt.m_index = i;
                rslt.m_stroke2 = a2dHit::stroke2_vertex;
                vertexhit = true;
            }

            // Stroke center hit-test
            // This is done to allow hitting the stroke even if the stroke length
            // is leth than the hit margin. As vertex hits have priority, the stroke
            // coldn't be hit then. So the stroke center is treated as pseudo-vertex.
            pm = a2dPoint2D( 0.5 * ( p1.m_x + p2.m_x ), 0.5 * ( p1.m_y + p2.m_y ) );
            distSqr = ClclDistSqrPntPnt( ptest, pm );
            if( distSqr < minDistSqrVertex )
            {
                minDistSqrVertex = distSqr;
                rslt.m_hit = a2dHit::hit_stroke;
                rslt.m_index = i;
                if( p1.m_x == p2.m_x )
                    rslt.m_stroke2 = a2dHit::stroke2_edgevert;
                else if( p1.m_y == p2.m_y )
                    rslt.m_stroke2 = a2dHit::stroke2_edgehor;
                else
                    rslt.m_stroke2 = a2dHit::stroke2_edgeother;
                vertexhit = true;
            }

            // Edge hit test
            // Note: The edge hittest is even done if a higher prioroity
            // vertex hit exists, and no edgehit can accur. This is done
            // because the distance of a vertex hit shall not be worse
            // then the distance from the closest edge, which is calculated
            // here.
            distSqr = ClclDistSqrPntLine( ptest, p1, p2 );

            if( distSqr < minDistSqrStroke )
            {
                minDistSqrStroke = distSqr;
                if( !vertexhit )
                {
                    rslt.m_hit = a2dHit::hit_stroke;
                    rslt.m_index = i;
                    if( p1.m_x == p2.m_x )
                        rslt.m_stroke2 = a2dHit::stroke2_edgevert;
                    else if( p1.m_y == p2.m_y )
                        rslt.m_stroke2 = a2dHit::stroke2_edgehor;
                    else
                        rslt.m_stroke2 = a2dHit::stroke2_edgeother;
                }
            }

            // Intersection test for inside/outside test
            // Vertical lines are ignored in the inside/outside test.
            // They are really not important for this.
            // You can define that on the vertical line you get the same result as
            // above or below the vertical line.
            // The right side of the ray is the side where inside outside is defined.
            // Lines starting/ending at the ray going to the right are seen, while lines
            // ending/starting on the ray coming from the left are not.
            if( ptest.m_x >= xmin && ptest.m_x < xmax && p1.m_x != p2.m_x )
            {
                if( p1.m_y == p2.m_y )
                {
                    if( ptest.m_y <= p1.m_y )
                        intersection_count += direction;
                }
                else
                {
                    double y = ( ptest.m_x - p1.m_x ) * ( p2.m_y - p1.m_y ) / ( p2.m_x - p1.m_x ) + p1.m_y;
                    if( ptest.m_y <= y )
                        intersection_count += direction;
                }
            }
        }
    }

    if( rslt.m_hit == a2dHit::hit_stroke )
    {
        if( intersection_count )
            rslt.m_stroke1 = a2dHit::stroke1_inside;
        else
            rslt.m_stroke1 = a2dHit::stroke1_outside;

        if( vertexhit )
            rslt.m_distance = ( sqrt( wxMin( minDistSqrVertex, minDistSqrStroke ) ) / margin );
        else
            rslt.m_distance = ( sqrt( minDistSqrStroke ) / margin );
    }
    else
    {
        if( intersection_count )
            rslt.m_hit = a2dHit::hit_fill;
        else
            rslt.m_hit = a2dHit::hit_none;
    }

    return rslt;
}

a2dHit a2dVertexArray::HitTestPolyline( const a2dPoint2D& ptest, double margin )
{
    wxASSERT( margin >= 0 );

    a2dPoint2D p1, p2, pm;

    double minDistSqrVertex = margin * margin;
    double minDistSqrStroke = margin * margin;
    bool vertexhit = false;
    bool lastpoint = false;
    a2dHit rslt;

    //iterate across points until we are sure that the given point is in or out
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        // Get the two points of edge #i
        p1 = Item( i )->GetPoint();
        if( i < size() - 1 )
            p2 = Item( i + 1 )->GetPoint();
        else
            lastpoint = true;

        //--------------------------------------------------------------------
        // From here on, this function is identical to the hittest for
        // point array polylines. Keep these two identical !!
        //--------------------------------------------------------------------

        // check relative horizontal positions of line end points and point
        double xmin;
        double xmax;

        if( p1.m_x < p2.m_x )
        {
            xmin = p1.m_x;
            xmax = p2.m_x;
        }
        else
        {
            xmin = p2.m_x;
            xmax = p1.m_x;
        }

        if( ptest.m_x < xmin - margin || ptest.m_x > xmax + margin )
            continue;

        // check relative horizontal positions of line end points and point
        double ymin;
        double ymax;
        if( p1.m_y < p2.m_y )
        {
            ymin = p1.m_y;
            ymax = p2.m_y;
        }
        else
        {
            ymin = p2.m_y;
            ymax = p1.m_y;
        }

        if( ptest.m_y < ymin - margin || ptest.m_y > ymax + margin )
            continue;

        // The test point ptest is inside the margin extended bounding box+margin of the edge
        // This means we make vertex and stroke hit tests
        // and an intersection test between edge and the vertical line through ptest

        // Vertex hit test for point 1
        //
        // Note1: double ifs are MUCH slower than double calculations (at least on intel)
        // So it is faster calculating the distance square than doing a rectangle test
        //
        // Note2: vertex hits have higher priority than edge hits
        //
        // Note3: every vertex is once point1 and once point 2, so only one point needs
        // to be tested.
        //
        // Note4: Even if there is a stroke or vertex hit, search is continued, because
        // there could be better hit. A stroke or vertex hit is rarely a random event,
        // so it doesn't make much sense to optimize this.

        double distSqr = ClclDistSqrPntPnt( ptest, p1 );

        if( distSqr < minDistSqrVertex )
        {
            minDistSqrVertex = distSqr;
            rslt.m_hit = a2dHit::hit_stroke;
            rslt.m_index = i;
            rslt.m_stroke2 = a2dHit::stroke2_vertex;
            vertexhit = true;
        }

        if( lastpoint )
            break;

        // Stroke center hit-test
        // This is done to allow hitting the stroke even if the stroke length
        // is less than the hit margin. As vertex hits have priority, the stroke
        // couldn't be hit then. So the stroke center is treated as pseudo-vertex.
        pm = a2dPoint2D( 0.5 * ( p1.m_x + p2.m_x ), 0.5 * ( p1.m_y + p2.m_y ) );
        distSqr = ClclDistSqrPntPnt( ptest, pm );
        if( distSqr < minDistSqrVertex )
        {
            minDistSqrVertex = distSqr;
            rslt.m_hit = a2dHit::hit_stroke;
            rslt.m_index = i;
            if( p1.m_x == p2.m_x )
                rslt.m_stroke2 = a2dHit::stroke2_edgevert;
            else if( p1.m_y == p2.m_y )
                rslt.m_stroke2 = a2dHit::stroke2_edgehor;
            else
                rslt.m_stroke2 = a2dHit::stroke2_edgeother;
            vertexhit = true;
        }

        // Edge hit test
        // Note: The edge hittest is even done if a higher prioroity
        // vertex hit exists, and no edgehit can accur. This is done
        // because the distance of a vertex hit shall not be worse
        // then the distance from the closest edge, which is calculated
        // here.
        distSqr = ClclDistSqrPntLine( ptest, p1, p2 );

        if( distSqr < minDistSqrStroke )
        {
            minDistSqrStroke = distSqr;
            if( !vertexhit )
            {
                rslt.m_hit = a2dHit::hit_stroke;
                rslt.m_index = i;
                if( p1.m_x == p2.m_x )
                    rslt.m_stroke2 = a2dHit::stroke2_edgevert;
                else if( p1.m_y == p2.m_y )
                    rslt.m_stroke2 = a2dHit::stroke2_edgehor;
                else
                    rslt.m_stroke2 = a2dHit::stroke2_edgeother;
            }
        }
    }

    if( rslt.m_hit == a2dHit::hit_stroke )
    {
        rslt.m_stroke1 = a2dHit::stroke1_outside;
        if( vertexhit )
            rslt.m_distance = ( sqrt( wxMin( minDistSqrVertex, minDistSqrStroke ) ) / margin );
        else
            rslt.m_distance = ( sqrt( minDistSqrStroke ) / margin );
    }

    return rslt;
}

bool a2dVertexArray::RemoveRedundant( bool polygon )
{
    bool did = false;
    a2dLineSegmentPtr segprev = NULL;
    a2dLineSegmentPtr seg = NULL;
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        seg = Item( i );
        if ( !seg->GetArc() && segprev && seg->GetPoint() == segprev->GetPoint() )
        {
            RemoveAt( i );
            i--;
            did = true;
        }
        else
            segprev = seg;
    }
    seg = Item( 0 );
    if ( polygon && !seg->GetArc() && segprev && seg->GetPoint() == segprev->GetPoint() )
    {
        RemoveAt( size() - 1 );
        did = true;
    }
    return did;
}

a2dVertexList* a2dVertexArray::GetRedundant( bool polygon, double smallest )

{
    a2dVertexList* redunDant = new a2dVertexList();
    a2dLineSegmentPtr segprev = NULL;
    a2dLineSegmentPtr seg = NULL;
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        seg = Item( i );
        if ( !seg->GetArc() && segprev &&
                ( seg->GetPoint() == segprev->GetPoint() || ClclDistSqrPntPnt( seg->GetPoint(), segprev->GetPoint() ) <= smallest ) )

        {
            redunDant->push_back( seg->Clone() );
        }
        segprev = seg;
    }
    seg = Item( 0 );
    if ( polygon && !seg->GetArc() && segprev &&
            ( seg->GetPoint() == segprev->GetPoint() || ClclDistSqrPntPnt( seg->GetPoint(), segprev->GetPoint() ) <= smallest ) )

        redunDant->push_back( seg->Clone() );

    if ( redunDant->empty() )
    {
        delete redunDant;
        redunDant = NULL;
    }
    return redunDant;
}


//----------------------------------------------------------------------------
// a2dVertexList
//----------------------------------------------------------------------------

a2dVertexList::a2dVertexList()
{
    m_refcount = 0;
}

a2dVertexList::a2dVertexList( const a2dVertexList& other )
{
    m_refcount = 0;
    *this = other;
}

a2dVertexList::a2dVertexList( const a2dVertexArray& other )
{
    m_refcount = 0;
    clear();
    for( a2dVertexArray::const_iterator iter = other.begin(); iter != other.end(); ++iter )
    {
        a2dLineSegmentPtr seg = ( *iter )->Clone();
        push_back( seg );
    }
}

a2dVertexList::~a2dVertexList()
{
    wxASSERT_MSG(  m_refcount == 0, wxT( "deleting a2dLineSegment while referenced" ) );
    clear();
}

a2dVertexList& a2dVertexList::operator=( const a2dVertexList& other )
{
    clear();

    for( a2dVertexList::const_iterator iter = other.begin(); iter != other.end(); ++iter )
    {
        a2dLineSegment* seg = ( *iter )->Clone();
        push_back( seg );
    }

    return *this;
}

a2dVertexList& a2dVertexList::operator=( const a2dVertexArray& other )
{
    clear();
    for( a2dVertexArray::const_iterator iter = other.begin(); iter != other.end(); ++iter )
    {
        a2dLineSegment* seg = ( *iter )->Clone();
        push_back( seg );
    }
    return *this;
}

a2dVertexList::iterator a2dVertexList::GetPreviousAround( a2dVertexList::iterator iter )
{
    if ( iter == begin() )
        return  --end();
    return --iter;
}

a2dVertexList::iterator a2dVertexList::GetNextAround( a2dVertexList::iterator iter )
{
    if ( iter == --end() )
        return  begin();
    return ++iter;
}

a2dVertexList::const_iterator a2dVertexList::GetPreviousAround( a2dVertexList::const_iterator iter ) const
{
    if ( iter == begin() )
        return  --end();
    return --iter;
}

a2dVertexList::const_iterator a2dVertexList::GetNextAround( a2dVertexList::const_iterator iter ) const
{
    if ( iter == --end() )
        return  begin();
    return ++iter;
}

void a2dVertexList::MakeBegin( a2dVertexList::iterator iter )
{
    a2dVertexList::iterator iterr = begin();
    while( iterr != iter )
    {
        a2dLineSegmentPtr seg = *iterr;
        ++iterr;
        pop_front();
        push_back( seg );
    }
}

bool a2dVertexList::HasArcs() const
{
    for( a2dVertexList::const_iterator iter = begin(); iter != end(); ++iter )
    {
        a2dLineSegment* seg = *iter;
        if ( seg->GetArc() )
            return true;
    }
    return false;
}

double a2dVertexList::Length()
{
    double len = 0;

    a2dVertexList::iterator iterp = begin();
    a2dVertexList::iterator iter = iterp;
    iter++;
    while ( iter != end() )
    {
        len = len + ( *iter )->Length( *( *iterp ) );
        iterp = iter++;
    }

    return len;
}

int a2dVertexList::IndexOf( a2dLineSegment* object ) const
{
    int i = 0;
    for( a2dVertexList::const_iterator iter = begin(); iter != end(); ++iter )
    {
        if( *iter == object )
            return i;
        i++;
    }
    return -1;
}

void a2dVertexList::Insert( unsigned int index, a2dLineSegmentPtr segin )
{
    int i = 0;
    a2dVertexList::iterator iter;
    for( iter = begin(); iter != end(); ++iter )
    {
        if( i == index )
        {
            insert( iter, segin );
            break;
        }
        i++;
    }
    if( iter == end() && i == index )
        insert( iter, segin );
    wxASSERT_MSG( i <= size() , wxT( "index higher than list size" ) );
}

void a2dVertexList::AddPoint( const a2dPoint2D& point, bool atEnd )
{
    if ( atEnd )
        push_back( new a2dLineSegment( point.m_x, point.m_y ) );
    else
        push_front( new a2dLineSegment( point.m_x, point.m_y ) );
}

void a2dVertexList::AddPoint( double x, double y, bool atEnd )
{
    if ( atEnd )
        push_back( new a2dLineSegment( x, y ) );
    else
        push_front( new a2dLineSegment( x, y ) );
}

void a2dVertexList::SetPointAdjustArcs( a2dLineSegmentPtr segin, double x, double y, bool polygon )
{
    a2dVertexList::iterator iter = begin();
    a2dVertexList::iterator prev = end();
    prev--;
    a2dLineSegment* seg;
    while ( iter != end() )
    {
        seg = *iter;
        if ( seg == segin )
        {
            seg->m_x = x;
            seg->m_y = y;

            if ( seg->GetArc() )
            {
                a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
                double radius, center_x, center_y, beginrad, midrad, endrad, phit;
                if ( prev != end() )
                {
                    if ( cseg->CalcR( *( *prev ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                        cseg->CalcMidPoint( *( *prev ), center_x, center_y, radius, phit < 0 );
                }
            }

            a2dVertexList::iterator next = iter;
            next++;
            if ( next == end() && polygon )
                next = begin();

            if ( next != end() && ( *next )->GetArc() )
            {
                a2dArcSegment* cseg = ( a2dArcSegment* ) ( *next ).Get();
                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                if ( cseg->CalcR( *( *iter ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                    cseg->CalcMidPoint( *( *iter ), center_x, center_y, radius, phit < 0 );
            }
            break;
        }
        prev = iter;
        iter++;
    }
}

void a2dVertexList::SetPointAdjustArcs( unsigned int n, double x, double y, bool polygon )
{
    unsigned int i;

    a2dVertexList::iterator iter = begin();
    a2dVertexList::iterator prev = end();
    prev--;
    a2dLineSegment* seg;
    i = 0;
    while ( iter != end() )
    {
        seg = *iter;
        if ( i == n )
        {
            seg->m_x = x;
            seg->m_y = y;

            if ( seg->GetArc() )
            {
                a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
                double radius, center_x, center_y, beginrad, midrad, endrad, phit;
                if ( prev != end() )
                {
                    if ( cseg->CalcR( *( *prev ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                        cseg->CalcMidPoint( *( *prev ), center_x, center_y, radius, phit < 0 );
                }
            }

            a2dVertexList::iterator next = iter;
            next++;
            if ( next == end() && polygon )
                next = begin();

            if ( next != end() && ( *next )->GetArc() )
            {
                a2dArcSegment* cseg = ( a2dArcSegment* ) ( *next ).Get();
                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                if ( cseg->CalcR( *( *iter ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                    cseg->CalcMidPoint( *( *iter ), center_x, center_y, radius, phit < 0 );
            }
            break;
        }
        i++;
        prev = iter;
        iter++;
    }
}

void a2dVertexList::Transform( const a2dAffineMatrix& cworld )
{
    if ( !cworld.IsIdentity() )
    {
        if ( !cworld.IsTranslate() )
            ConvertToLines();

        for( a2dVertexList::iterator iter = begin(); iter != end(); ++iter )
        {
            a2dLineSegment* seg = *iter;
            if ( seg->GetArc() )
            {
                a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
                cworld.TransformPoint( cseg->m_x2, cseg->m_y2, cseg->m_x2, cseg->m_y2 );
            }
            cworld.TransformPoint( seg->m_x, seg->m_y, seg->m_x, seg->m_y );
        }
    }
}

void a2dVertexList::ConvertToLines( double aberation )
{
    double xw, yw;
    a2dLineSegment* segn;

    a2dVertexList::iterator iterp = end();
    a2dVertexList::iterator iter = begin();
    while ( iter != end() )
    {
        a2dLineSegment* seg = *iter;
        if ( seg->GetArc() )
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;

            double radius, center_x, center_y, beginrad, midrad, endrad, phit;

            if ( cseg->CalcR( *( *iterp ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
            {
                double dphi;
                unsigned int segments = 20;
                if ( aberation == 0 )
                    Aberration( radius / 200, phit, radius , dphi, segments );
                //a2dGlobals->Aberration( phit, radius , dphi, segments );
                else
                    Aberration( aberation, phit, radius , dphi, segments );

                double theta = beginrad + dphi; //first point already there
                unsigned int step;
                for ( step = 0; step < segments; step++ )
                {
                    xw = center_x + radius * cos ( theta );
                    yw = center_y + radius * sin ( theta );
                    segn = new a2dLineSegment( xw, yw );
                    segn->SetArcPiece( true );

                    iterp = insert( iter, segn );
                    theta = theta + dphi;
                }
            }
            else
            {
                segn = new a2dLineSegment( cseg->m_x, cseg->m_y );
                iterp = insert( iter, segn );
            }

            // last inserted segment interp, and iter is the arc segment.
            iter = erase( iter ); //erase arc segment
        }
        else
        {
            iterp = iter;
            iter++;
        }
    }
}

void a2dVertexList::ConvertToLines( double dphi, int minseg )
{
    //convert dphi to radians first
    dphi = dphi * M_PI / 180.0;

    double xw, yw;
    a2dLineSegment* segn;

    a2dVertexList::iterator iterp = end();
    a2dVertexList::iterator iter = begin();
    while ( iter != end() )
    {
        a2dLineSegment* seg = *iter;
        if ( seg->GetArc() )
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;

            double radius, center_x, center_y, beginrad, midrad, endrad, phit;
            if ( cseg->CalcR( *( *iterp ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
            {
                unsigned int segments;
                segments = ( unsigned int ) ( ceil( fabs(phit / dphi) ) < minseg ? minseg : ceil( fabs(phit / dphi) ) );

                dphi = phit / ( segments );

                double theta = beginrad;
                unsigned int step;

                for ( step = 0; step < segments + 1; step++ )
                {
                    xw = center_x + radius * cos ( theta );
                    yw = center_y + radius * sin ( theta );
                    segn = new a2dLineSegment( xw, yw );
                    segn->SetArcPiece( true );

                    iterp = insert( iter, segn );
                    theta = theta + dphi;
                }
            }
            else
            {
                segn = new a2dLineSegment( cseg->m_x, cseg->m_y );
                iterp = insert( iter, segn );
            }
            // last inserted segment interp, and iter is the arc segment.
            iter = erase( iter ); //erase arc segment
        }
        else
        {
            iterp = iter;
            iter++;
        }
    }
}

a2dVpath* a2dVertexList::ConvertToVpath( bool arc, bool closed )
{
    a2dVpath* vpathsegments = new a2dVpath();

    double xw, yw;

    a2dVertexList::iterator iterp = end();
    a2dVertexList::iterator iter = begin();
    while ( iter != end() )
    {
        a2dLineSegment* seg = *iter;
        if ( seg->GetArc() )
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
            if ( arc )
            {
                a2dVpathArcSegment* arcseg;
                a2dVertexList::iterator iternext = iter;
                iternext++;
                if ( iter == begin() )
                {
                    a2dVpathSegment* segn = new a2dVpathSegment( cseg->m_x, cseg->m_y, a2dPATHSEG_MOVETO  );
                    vpathsegments->Add( segn );
                    arcseg = new a2dVpathArcSegment( cseg->m_x, cseg->m_y, cseg->m_x2, cseg->m_y2, a2dPATHSEG_MOVETO );
                }
                else if ( iternext == end() )
                    arcseg = new a2dVpathArcSegment( cseg->m_x, cseg->m_y, cseg->m_x2, cseg->m_y2, a2dPATHSEG_ARCTO, closed ? a2dPATHSEG_END_CLOSED :  a2dPATHSEG_END_OPEN );
                else
                    arcseg = new a2dVpathArcSegment( cseg->m_x, cseg->m_y, cseg->m_x2, cseg->m_y2, a2dPATHSEG_ARCTO );
                vpathsegments->Add( arcseg );
            }
            else
            {
                // to line segments
                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                if ( cseg->CalcR( *( *iterp ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                {
                    double dphi;
                    unsigned int segments = 20;
                    Aberration( radius / 200, phit, radius , dphi, segments );
                    //a2dGlobals->Aberration( phit, radius , dphi, segments );

                    double theta = beginrad;
                    unsigned int step;

                    for ( step = 0; step < segments + 1; step++ )
                    {
                        xw = center_x + radius * cos ( theta );
                        yw = center_y + radius * sin ( theta );
                        a2dVpathSegment* lineseg = new a2dVpathSegment( xw, yw );
                        lineseg->SetArcPiece( true );
                        vpathsegments->Add( lineseg );
                        theta = theta + dphi;
                    }
                }
                else
                {
                    a2dVpathSegment* segn = new a2dVpathSegment( cseg->m_x, cseg->m_y );
                    segn->SetArcPiece( true );
                    vpathsegments->Add( segn );
                }
            }
        }
        else
        {
            a2dVpathSegment* lineseg;
            a2dVertexList::iterator iternext = iter;
            iternext++;
            if ( iter == begin() )
                lineseg = new a2dVpathSegment( seg->m_x, seg->m_y , a2dPATHSEG_MOVETO );
            else if ( iternext == end() )
                lineseg = new a2dVpathSegment( seg->m_x, seg->m_y, a2dPATHSEG_LINETO, closed ? a2dPATHSEG_END_CLOSED :  a2dPATHSEG_END_OPEN );
            else
                lineseg = new a2dVpathSegment( seg->m_x, seg->m_y, a2dPATHSEG_LINETO );
            lineseg->SetArcPiece( seg->GetArcPiece() );
            vpathsegments->Add( lineseg );
        }

        iterp = iter;
        iter++;
    }
    return vpathsegments;
}

//A = 1/2 * (x1*y2 - x2*y1 + x2*y3 - x3*y2 + ... + x(n-1)*yn - xn*y(n-1) + xn*y1 - x1*yn)
double a2dVertexList::CalcArea() const
{
    if ( size() < 3 )
        return 0.0;

    double area = 0.0;

    a2dVertexList* workOn = const_cast<a2dVertexList*>( this );
    double miny = ( *workOn->begin() )->m_y;
    bool hasarcs = HasArcs();
    if ( hasarcs )
    {
        workOn = new a2dVertexList( *this );
        workOn->ConvertToLines( 10, 3 );
    }
    a2dVertexList::const_iterator iter;

/* next seems to give problems in result, why?
    for( iter = workOn->begin(); iter != workOn->end(); ++iter )
    {
        a2dLineSegment* seg = *iter;
        if ( seg->m_y < miny )
            miny = seg->m_y;
    }
    if ( miny > 0 )
*/
        miny = 0;

    iter = workOn->begin();
    for ( int i = 0; i < workOn->size(); i++ )
    {
        double x = ( *iter )->m_x;
        double y = ( *iter )->m_y - miny;
        iter = workOn->GetNextAround( iter );
        area += ( ( *iter )->m_y - miny ) * x;
        area -= ( *iter )->m_x * y;
    }

    if ( hasarcs )
        delete workOn;

    return fabs( area / 2.0 );
}

bool a2dVertexList::DirectionIsClockWise()
{
    return ( bool )( CalcArea() < 0.0 );
}

void a2dVertexList::InsertArc( a2dVertexList::iterator& iter, int n, double center_x, double center_y )
{
    a2dPoint2D begin_p, middle_p;
    int middlepoint, i;

    middlepoint = n / 2;


    begin_p.m_x = ( *iter )->m_x;
    begin_p.m_y = ( *iter )->m_y; // begin point of ARC

    // leaf current intact, eventuallly next segment should become an arc.
    iter = GetNextAround( iter );

    //remove segmenet until end of arc segment
    for ( i = 1; i < middlepoint - 1; i++ ) // all points to middle point removed
        iter = erase( iter );

    middle_p.m_x = ( *iter )->m_x;
    middle_p.m_y = ( *iter )->m_y; // middle point of ARC

    for ( i = middlepoint; i < n; i++ ) // all points to end point removed
        iter = erase( iter );

    if ( ( *iter )->m_x == begin_p.m_x && ( *iter )->m_y == begin_p.m_y )
    {
        double dx = center_x - begin_p.m_x;
        double dy = center_y - begin_p.m_y;
        //a2dArcSegment* aseg = new a2dArcSegment( center_x + dx, center_y + dy, center_x - dx, center_y + dy );
        //insert( iter, aseg );
        //aseg = new a2dArcSegment( begin_p.m_x, begin_p.m_y, center_x + dx, center_y - dy );
        //insert( iter, aseg );
        a2dArcSegment* aseg = new a2dArcSegment( begin_p.m_x, begin_p.m_y, center_x + dx, center_y + dy );
        insert( iter, aseg );
    }
    else
    {
        a2dArcSegment* aseg = new a2dArcSegment( ( *iter )->m_x, ( *iter )->m_y, middle_p.m_x, middle_p.m_y );
        insert( iter, aseg );
        //remove end of arc segment, which is replaced by Arc segment to previous point.
    }
    iter = erase( iter );
}

bool a2dVertexList::CheckForOneCircle( a2dPoint2D& middle, double& radius )
{
    a2dVertexList::iterator iter = begin();
    a2dLineSegment* seg = *iter;
    if ( size() == 1 && seg->GetArc() )
    {
        iter = GetPreviousAround( iter );
        double x = ( *iter )->m_x;
        double y = ( *iter )->m_y;
        a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
        middle.m_x = cseg->GetOx( *seg );
        middle.m_y = cseg->GetOy( *seg );
        radius = sqrt( pow( x - middle.m_x, 2.0 ) + pow( y - middle.m_y, 2.0 ) );

        //now check if the end point is equal
        if ( x == seg->m_x && y == seg->m_y )
            return true;
    }
    /*
        if ( size() == 2 )
        {
            if ( seg->GetArc() )
            {
                GetPreviousAround( iter );
                if ( (*iter)->GetArc() )
                    return false;
                double x = (*iter).m_x;
                double y = (*iter).m_y;
                a2dArcSegment * cseg = ( a2dArcSegment* ) seg;
                middle_p.m_x = cseg.GetOx( *iter );
                middle_p.m_y = cseg.GetOy( *iter );
                radius = sqrt(pow(x-aseg->GetOX(),2.0) + pow(y-aseg->GetOY(),2.0));
                return true;
            }
            else
            {
                GetPreviousAround( iter );
                if ( !(*iter)->GetArc() )
                    return false;
                double x = (*iter).m_x;
                double y = (*iter).m_y;
                a2dArcSegment * cseg = ( a2dArcSegment* ) (*iter);
                middle_p.m_x = cseg.GetOx( begin() );
                middle_p.m_y = cseg.GetOy( begin() );
                radius = sqrt(pow(x-aseg->GetOX(),2.0) + pow(y-aseg->GetOY(),2.0));
                return true;
            }
        }
    */
    return false;
}

void a2dVertexList::ConvertPolygonToArc( double aber, double Rmin, double Rmax )
{
    int n, minimum_points;

    a2dPoint2D center_p;
    minimum_points = 3;     // alleen polygonen met meer dan 3 punten kunnen
    // geconverteerd worden
    a2dLineSegment* lsegn;
    if ( size() )
    {
        lsegn = new a2dLineSegment( front()->m_x, front()->m_y );
        push_back( lsegn );
    }
    else
        return;

    // redundant as a polyline, not polygon, else added point above is gone
    RemoveRedundant( false );

    if ( size() > minimum_points )
    {

        // Polygon wordt getest op ARC's, wordt er een ARC gevonden dan wordt het beginpunt naar
        // het eindpunt van de ARC gezet. Dit voorkomt dat een ARC niet herkent wordt wanneer
        // het beginpunt zich ergens op de ARC bevind.
        // In het geval dat alle punten voldoen aan een ARC (circel) vind de conversie direct plaats.
        // In alle andere gevallen wordt het testen op ARC's en het converteren gedaan door
        // G_Polyline::ConvertLine.

        a2dVertexList::iterator iter = begin();
        for ( int count = 0; count < size(); count++ )
        {
            n = TestArc( iter, aber, Rmin, Rmax, center_p );

            if ( n < size() && n > minimum_points )
            {
                for ( int t = 1; t < n; t++ ) iter++; // verschuiving beginpunt naar eindpunt ARC.
                //remove last clossing line
                pop_back();
                MakeBegin( iter );
                lsegn = new a2dLineSegment( ( *iter )->m_x, ( *iter )->m_y );
                push_back( lsegn );

                break;
            }
            else  if ( n > minimum_points ) // alle punten van de polygon voldoen aan een ARC (circle)
            {
                iter = begin();

                double dx = center_p.m_x - ( *iter )->m_x;
                double dy = center_p.m_y - ( *iter )->m_y;
                a2dArcSegment* aseg = new a2dArcSegment( ( *iter )->m_x, ( *iter )->m_y, center_p.m_x + dx, center_p.m_y + dy );
                clear();
                push_back( aseg );

                //InsertArc( iter, n, center_p.m_x, center_p.m_y );
                return;
            }

            iter++;
            pop_back();
            MakeBegin( iter );
            lsegn = new a2dLineSegment( ( *iter )->m_x, ( *iter )->m_y );
            push_back( lsegn );
        }

        ConvertPolylineToArc( aber, Rmin, Rmax ); // De punten van de polyline die voldoen aan een circelboog
    }                        // worden geconverteerd naar een ARC.

    //in case the last found arc is also endpoint of the tested polyline, the extra added
    //segment above (to close the polygon), is replaced by an arc segment, and should stay.
    if ( !back()->GetArc() )
        pop_back();
}

void a2dVertexList::ConvertPolylineToArc( double aber, double Rmin, double Rmax )
{
    int n, minimum_points;

    a2dPoint2D center_p;   //center punt van een ARC

    minimum_points = 3;

    RemoveRedundant( false );

    a2dVertexList::iterator iter = begin();
    if ( size() > minimum_points )  // Alleen polylijnen met meer dan 3 punten kunnen
    {
        // worden geconverteerd naar ARC's
        iter++;
        iter++;
        iter++;
        while( iter != end() )
        {
            iter--;
            iter--;
            iter--;
            a2dLineSegment* seg = *iter;
            n = TestArc( iter, aber, Rmin, Rmax, center_p );
            if ( n > minimum_points )
                InsertArc( iter, n, center_p.m_x, center_p.m_y );
            else
                iter++;

            if ( iter != end() ) iter++; // Als het resterende aantal punten minder is dan 3
            if ( iter != end() ) iter++; // wordt het testen op ARC's gestopt.
            if ( iter != end() ) iter++;
        }
    }
}

#define KLEIN 1.0e-30
#define GROOT 1.0e30
#define PHI_MARGE 0.3  //must be lager than 0, 0.3 -> 30%  (graden)
#define SEG_MARGE 0.3  //must be lager than 0, 0.3 -> 30%

int a2dVertexList::TestArc( a2dVertexList::iterator& iter, double aber, double Rmin, double Rmax ,  a2dPoint2D& center_p_old )
{
    a2dPoint2D begin_p, middle_p, test_p, end_p, center_p ;

    a2dVertexList::iterator  middle = iter;
    a2dVertexList::iterator  test = iter;
    a2dVertexList::iterator  endc = iter;

    double error1, error2, Radius, Radius_old;
    double x_between, y_between, dx, dy, x_prev, y_prev;
    double inp, phi, phi_old, phi_fac, a, b, lseg, lseg_old, seg_fac;
    double t_inp;
    lseg = 0;
    int    i, j, ii, minimum_points;

    Radius   = GROOT;
    error1   = GROOT;
    error2   = GROOT;
    phi      = 0.0;
    lseg_old = 0.0;
    phi_old  = 0.0;

    if ( aber < KLEIN )     // aber <= 0 ; test op "perfecte circelbogen"
    {
        phi_fac  = 1 + PHI_MARGE;   // hoek factor   ; geeft aan hoeveel de segmenthoeken onderling mogen afwijken
        seg_fac  = 1 + SEG_MARGE;   // segment factor; geeft aan hoeveel de segmentlengtes onderling mogen afwijken

        if ( aber < 0.0 )   // aber < 0; zie punt 3 in de header
            aber = -aber;
        else            // aber = 0; zie punt 2 in de header
            aber = GROOT;
    }
    else                // aber > 0; zie punt 1 in de header
    {
        phi_fac  = GROOT;
        seg_fac  = GROOT;
    }

    minimum_points = 3;

    begin_p.m_x = ( *iter )->m_x;
    begin_p.m_y = ( *iter )->m_y; // Beginpunt

    for ( ii = 1; ii <= minimum_points; ii++ )  // Eindpunt wordt 3 posities opgeschoven en er wordt tevens
    {
        // getest of een van de punten niet behoord tot het type ARC
        if ( !( *endc )->GetArc() )
            endc++;
        else
            return ( 0 );       // Verlaat de functie indien een van de punten van het type ARC is.
    }

    middle++;

    i = minimum_points;

    do
    {
        i++;
        if ( i % 2 != 0 )      // Middelpunt schuift 1 punt op als aantal punten oneven is
            middle++;

        end_p.m_x = ( *endc )->m_x;
        end_p.m_y = ( *endc )->m_y; // End point

        middle_p.m_x = ( *middle )->m_x;
        middle_p.m_y = ( *middle )->m_y; // Middle point

        Radius_old   = Radius;
        center_p_old = center_p;

        // Als het laatste punt gelijk is aan beginpunt dan niet opnieuw ARC berekenen

        if ( !( ( fabs( end_p.m_x - begin_p.m_x ) < a2dACCUR ) && ( fabs( end_p.m_y - begin_p.m_y ) < a2dACCUR ) ) )
            CalcR( begin_p.m_x,  begin_p.m_y, middle_p.m_x, middle_p.m_y, end_p.m_x, end_p.m_y,
                   Radius, center_p );  // Berekening radius en centerpunt

        test = iter;

        j = 1;

        test_p.m_x = ( *test )->m_x;
        test_p.m_y = ( *test )->m_y; // first test point

        while ( j < i ) // Alle punten liggende tussen begin en eindpunt worden getest
        {
            // of ze voldoen aan de berekende circelboog
            j++;
            x_prev = test_p.m_x; // Test point (1)
            y_prev = test_p.m_y;

            test++;

            test_p.m_x = ( *test )->m_x;
            test_p.m_y = ( *test )->m_y; // Test point (2)

            dx = center_p.m_x - test_p.m_x;
            dy = center_p.m_y - test_p.m_y;

            error1 = fabs( Radius - sqrt( pow( dx, 2 ) + pow( dy, 2 ) ) ); // Afwijking tussen berekende circelboog en testpunt

            x_between = ( test_p.m_x + x_prev ) / 2.0; // Berekening punt tussen twee testpunt 1 en testpunt 2,
            y_between = ( test_p.m_y + y_prev ) / 2.0; // dit is nodig voor berekening van de aberatie

            dx = center_p.m_x - x_between;
            dy = center_p.m_y - y_between;

            error2 = fabs( Radius - sqrt( pow( dx, 2 ) + pow( dy, 2 ) ) ); // Aberatie

            inp = ( center_p.m_x - x_prev ) * ( test_p.m_y - center_p.m_y ) - // a x b (vectoren)
                  ( center_p.m_y - y_prev ) * ( test_p.m_x - center_p.m_x );

            a = sqrt( pow( ( center_p.m_x - x_prev ), 2 ) + pow( ( center_p.m_y - y_prev ), 2 ) );
            b = sqrt( pow( ( center_p.m_x - test_p.m_x ), 2 ) + pow( ( center_p.m_y - test_p.m_y ), 2 ) );

            phi_old = phi;

            t_inp = inp / ( a * b );

            if ( t_inp > 1.0 )   // t_inp mag niet groter worden dan 1 of kleiner dan -1
                t_inp = 1.0;
            else if ( t_inp < -1.0 )
                t_inp = -1.0;

            phi = 180.0 * asin( t_inp ) / M_PI; // Segment hoek

            lseg_old = lseg;

            lseg = sqrt( pow( ( test_p.m_x - x_prev ), 2 ) + pow( ( test_p.m_y - y_prev ), 2 ) ); // segment lengte

            if ( j == 2 )
            {
                phi_old    = phi;
                lseg_old   = lseg;
            }

            if ( error1 > aber ||                          /* Afwijking punt groter dan aber */
                    error2 > aber ||                           /* Aberatie lijn groter dan aber */
                    fabs( phi ) > fabs( phi_old )*phi_fac || /* Segment hoek groter vorige hoek */
                    fabs( phi ) < fabs( phi_old ) / phi_fac || /* Segment hoek kleiner vorige hoek */
                    phi * phi_old < 0.0 ||                     /* Segment hoek tegengestelde richting van vorige hoek */
                    lseg > lseg_old * seg_fac ||           /* Segment lengte groter vorige segment */
                    lseg < lseg_old / seg_fac )                /* Segment lengte kleiner vorige segment */
            {
                if ( Radius_old > Rmin && Radius_old < Rmax ) // Als de nieuw gevonden circelboog niet
                    // voldoet dan wordt getest of de vorige
                    // radius binnen Rmin en Rmax ligt.
                    return ( i - 1 ); // Return aantal gevonden punten -1
                // (laatste punt voldoet niet)
                else
                    return ( 0 );   // Gevonden radius voldoet niet
            }
        }
        //een ARC segment is gevonden dat aan alle regels voldoet
        //indien mogelijk proberen we nog een punt verder te komen
        if ( ( *endc )->GetArc() )  // stop het huidige segment is een arc
            break;

        endc++; // probeer met het nieuwe eindpunt een nieuwe circelboog te berekenen.
    }
    while( endc != end() );

    if ( Radius > Rmin && Radius < Rmax ) // nieuw gevonden circelboog oke?
    {
        center_p_old = center_p;
        return ( i );
    }
    else
        return ( 0 ); // Gevonden radius voldoet niet
}

a2dBoundingBox a2dVertexList::GetBbox( const a2dAffineMatrix& lworld )
{
    a2dBoundingBox bbox;

    double x, y;
    lworld.TransformPoint( 0, 0, x, y );
    if ( !size() )
        bbox.Expand( x, y );

    a2dLineSegment* segprev = NULL;
    if ( size() )
        segprev = back();
    for( a2dVertexList::iterator iter = begin(); iter != end(); ++iter )
    {
        a2dLineSegment* seg = *iter;
        if ( seg->GetArc() && segprev )
            bbox.Expand( seg->GetBbox( *segprev, lworld ) );
        else
        {
            lworld.TransformPoint( seg->m_x, seg->m_y, x, y );
            bbox.Expand( x, y );
        }
        segprev = ( *iter );
    }
    return bbox;
}

a2dVertexList* a2dVertexList::ConvertToContour( double distance, a2dPATH_END_TYPE pathtype, bool asPolygon )
{
    a2dVertexList* converted = new a2dVertexList();
    *converted = *this;
    converted->RemoveRedundant( asPolygon );

    converted->Contour( distance, pathtype, asPolygon );
    return converted;
}

void a2dVertexList::Contour( double distance, a2dPATH_END_TYPE pathtype, bool asPolygon )
{
    bool rounded  = true;

    a2dPoint2D _first;
    a2dPoint2D _middle;
    a2dPoint2D _end;
    a2dPoint2D offsetpointleft;
    a2dPoint2D offsetpointright;

    if ( asPolygon )
        push_front( new a2dLineSegment( back()->m_x, back()->m_y ) );

    unsigned int size = this->size();
    a2dVertexList::iterator iter = begin();
    a2dLineSegment* seg = *iter;
    a2dVertexList::iterator prev = iter;

    //create start of subpart
    if ( size == 1 )
    {
        a2dLineSegment* seg = *iter;
        push_back( new a2dLineSegment( seg->m_x - distance, seg->m_y ) );
        switch ( pathtype )
        {
            case a2dPATH_END_SQAURE:
            {
                //not defined
            }
            break;
            case a2dPATH_END_ROUND:
            {
                push_back( new a2dArcSegment( seg->m_x + distance, seg->m_y, seg->m_x - distance, seg->m_y ) );
            }
            break;
            case a2dPATH_END_SQAURE_EXT:
            {
                //not defined
            }
            break;
            default:
                break;
        }
        return;
    }
    // in three parts ( begin middle end ), by pushing back and front, both side of the contour are
    // constructed.

    //first begin point
    _first = a2dPoint2D( seg->m_x, seg->m_y );
    iter++;
    a2dLineSegment* segnext = *iter;

    _middle = a2dPoint2D( segnext->m_x, segnext->m_y );
    a2dLine line1( _first, _middle );
    line1.CalculateLineParameters();
    offsetpointleft = _first;
    offsetpointright = _first;
    line1.Virtual_Point( offsetpointleft, distance );
    line1.Virtual_Point( offsetpointright, -distance );

    switch ( pathtype )
    {
        case a2dPATH_END_SQAURE:
        {
            push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
            push_front( new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
        }
        break;
        case a2dPATH_END_ROUND:
        {
            a2dLine  lineoffset( offsetpointright, offsetpointleft );
            lineoffset.CalculateLineParameters();
            a2dPoint2D offsetpointfirst;

            offsetpointfirst = _first;
            lineoffset.Virtual_Point( offsetpointfirst, distance );
            push_back( new a2dArcSegment( offsetpointleft.m_x, offsetpointleft.m_y,
                                          offsetpointfirst.m_x, offsetpointfirst.m_y ) );
            push_front( new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
            break;
        }
        case a2dPATH_END_SQAURE_EXT:
        {
            a2dLine  lineoffset( offsetpointright, offsetpointleft );
            lineoffset.CalculateLineParameters();
            lineoffset.Virtual_Point( offsetpointleft, distance );
            lineoffset.Virtual_Point( offsetpointright, distance );

            push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
            push_front( new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
            break;
        }
        default:
            break;
    }

    size--;
    size--;
    while ( size )
    {
        a2dLineSegment* seg = *prev;
        _first = a2dPoint2D( seg->m_x, seg->m_y );
        a2dLineSegment* segnext = *iter;
        _middle = a2dPoint2D( segnext->m_x, segnext->m_y );
        a2dLine line1( _first, _middle );
        erase( prev );
        prev = iter;
        iter++;
        a2dLineSegment* segend = *iter;
        _end = a2dPoint2D( segend->m_x, segend->m_y );
        a2dLine line2( _middle, _end );
        line1.CalculateLineParameters();
        line2.CalculateLineParameters();

        OUTPRODUCT _outproduct;
        _outproduct = line1.OutProduct( line2, 1e-9 );

        switch ( _outproduct )
        {
                // Line 2 lies on  leftside of this line
            case R_IS_RIGHT :
            {
                if ( !rounded )
                {
                    line1.OffsetContour( line2, distance, offsetpointleft );
                    line1.OffsetContour( line2, -distance, offsetpointright );
                    push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
                    push_front( new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
                }
                else
                {
                    OffsetContour_rounded( line1, line2, distance, false );
                    line1.OffsetContour( line2, -distance, offsetpointright );
                    push_front( new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
                }
            }
            break;
            case R_IS_LEFT :
            {
                if ( !rounded )
                {
                    line1.OffsetContour( line2, distance, offsetpointleft );
                    line1.OffsetContour( line2, -distance, offsetpointright );
                    push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
                    push_front( new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
                }
                else
                {
                    line1.OffsetContour( line2, distance, offsetpointleft );
                    push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
                    OffsetContour_rounded( line1, line2, -distance, true );
                }
            }
            break;
            // Line 2 lies on this line
            case R_IS_ON     :
            {
                offsetpointleft = _middle;
                offsetpointright = _middle;
                line1.Virtual_Point( offsetpointleft, distance );
                line1.Virtual_Point( offsetpointright, -distance );
                push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
                push_front( new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
            }
            break;
            default:
                wxFAIL_MSG( wxT( "wrong line code" ) );
                break;
        }//end switch

        size--;
    }

    seg = *prev;

    //first begin point
    _first = a2dPoint2D( seg->m_x, seg->m_y );
    segnext = *iter;
    _end = a2dPoint2D( segnext->m_x, segnext->m_y );
    a2dLine line2( _first, _end );
    line2.CalculateLineParameters();
    offsetpointleft = _end;
    offsetpointright = _end;
    line2.Virtual_Point( offsetpointleft, distance );
    line2.Virtual_Point( offsetpointright, -distance );

    prev = erase( prev );
    prev = erase( prev );

    switch ( pathtype )
    {
        case a2dPATH_END_SQAURE:
        {
            push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
            push_front( new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
        }
        break;
        case a2dPATH_END_ROUND:
        {
            a2dLine  lineoffset( offsetpointleft, offsetpointright );
            lineoffset.CalculateLineParameters();
            a2dPoint2D offsetpointend;

            offsetpointend = _end;
            lineoffset.Virtual_Point( offsetpointend, distance );
            push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
            push_back( new a2dArcSegment( offsetpointright.m_x, offsetpointright.m_y,
                                          offsetpointend.m_x, offsetpointend.m_y ) );
            push_front( new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
            break;
        }
        case a2dPATH_END_SQAURE_EXT:
        {
            a2dLine  lineoffset( offsetpointright, offsetpointleft );
            lineoffset.CalculateLineParameters();
            lineoffset.Virtual_Point( offsetpointleft, -distance );
            lineoffset.Virtual_Point( offsetpointright, -distance );

            push_back( new a2dLineSegment( offsetpointleft.m_x, offsetpointleft.m_y ) );
            push_front( new a2dLineSegment( offsetpointright.m_x, offsetpointright.m_y ) );
            break;
        }
        default:
            break;
    }
}

a2dVertexList* a2dVertexList::ConvertPointsAtDistance( double distance, bool asPolygon, bool atVertex, bool lastPointCloseToFirst, bool alongLine ) const
{
    a2dVertexList* converted = new a2dVertexList();

    a2dLineSegment* seg = NULL;
    a2dLineSegment* segn = NULL;
    a2dVertexList::const_iterator iter = begin();
    seg  = *iter;
    iter = GetNextAround( iter );
	segn = *iter;
    iter = begin();

    unsigned int count = size();
    if ( !asPolygon )
        count -= 1;

    unsigned int i = 0;

    double distanceToMake = distance; 
    double distanceMade; 
    bool atEnd = false;
    a2dPoint2D PointOnLine( seg->m_x, seg->m_y );
    a2dPoint2D pointAtDistance = PointOnLine;
    converted->AddPoint( PointOnLine.m_x, PointOnLine.m_y );
    while ( i < count )
    {
        if ( alongLine )
        {
            distanceMade = 0;
            atEnd = false;
            PointOnLine = DistancePoint( PointOnLine, seg, segn, distanceToMake, distanceMade, atEnd );
            if ( distanceToMake - distanceMade > 0 )
            {
                distanceToMake = distanceToMake - distanceMade;
                if ( !atEnd || atVertex )
                    converted->AddPoint( PointOnLine.m_x, PointOnLine.m_y );
            }
            else
            {
                distanceToMake = distance;
                converted->AddPoint( PointOnLine.m_x, PointOnLine.m_y );
            }
        }
        else
        {
            atEnd = DistancePoint2( PointOnLine, pointAtDistance, seg, segn, distance, distanceMade );
            if ( distance - distanceMade > 0 )
            {   // need more
                if ( atEnd && atVertex )
                    converted->AddPoint( PointOnLine.m_x, PointOnLine.m_y );
            }
            else
            {   // reached point at distance from previous
                distanceMade = 0;
                PointOnLine = pointAtDistance;
                converted->AddPoint( PointOnLine.m_x, PointOnLine.m_y );
            }
        }
        if ( atEnd ) //goto next segment
        {
            iter = GetNextAround( iter );
	        seg = *iter;
            iter = GetNextAround( iter );
	        segn = *iter;
            iter = GetPreviousAround( iter );
            i++;
        }
    }

    if ( lastPointCloseToFirst && asPolygon && converted->size() > 1 ) //check last point if within distance to first remove it.
    {
        a2dPoint2D plast = converted->back()->GetPoint();
        a2dPoint2D pfirst = converted->front()->GetPoint();
        if ( plast.GetDistance( pfirst ) < distance )
            converted->pop_back();
    }

    return converted;
}

a2dPoint2D a2dVertexList::DistancePoint( a2dPoint2D startPoint, a2dLineSegment* seg, a2dLineSegment* segn, double distance, double& distanceMade, bool& atEnd ) const
{
    double dx, dy;
    dx = segn->m_x - seg->m_x;
    dy = segn->m_y - seg->m_y;
    double dxs, dys;
    dxs = segn->m_x - startPoint.m_x;
    dys = segn->m_y - startPoint.m_y;
    double len = sqrt( dx*dx + dy*dy );
    double lenstartPoint = sqrt( dxs*dxs + dys*dys );
    if ( lenstartPoint < distance )
    {
        distanceMade = lenstartPoint;
        atEnd = true;
        return a2dPoint2D( segn->m_x, segn->m_y );
    }
    else
    {
        distanceMade = distance;
        atEnd = false;
        return a2dPoint2D( startPoint.m_x + distance * dx/len, startPoint.m_y + distance * dy/len );
    }
}

bool a2dVertexList::DistancePoint2( a2dPoint2D startPoint, a2dPoint2D& PointDistance, a2dLineSegment* seg, a2dLineSegment* segn, double distance, double& distanceMade ) const
{
    double dx, dy;
    dx = segn->m_x - seg->m_x;
    dy = segn->m_y - seg->m_y;

    double distanceToStartPoint = startPoint.GetDistance( PointDistance );
    double len = sqrt( dx*dx + dy*dy );
    double step = distance/100;
    bool atEnd = false;
    while ( distanceToStartPoint < distance && !atEnd )
    {
        PointDistance = a2dPoint2D( PointDistance.m_x + step/len * dx, PointDistance.m_y + step/len * dy );
        atEnd = seg->GetPoint().GetDistance( PointDistance ) >= len; 
        distanceToStartPoint = startPoint.GetDistance( PointDistance );
    }
    if ( atEnd ) //return point at end of segment.
        PointDistance = segn->GetPoint();

    distanceMade = startPoint.GetDistance( PointDistance );

    //PointDistance at distance is on the segment or at end if not reached yet.
    return atEnd;
}

a2dVertexList* a2dVertexList::ConvertSplinedPolygon( double Aber ) const
{
    a2dVertexList* h = new a2dVertexList();

    *h = *this;

    //TODO need an array version for speed
    h->ConvertIntoSplinedPolygon( Aber );

    return h;
}

a2dVertexList* a2dVertexList::ConvertSplinedPolyline( double Aber ) const
{
    a2dVertexList* h = new a2dVertexList();

    *h = *this;

    h->ConvertIntoSplinedPolyline( Aber );

    return h;
}

void a2dVertexList::ConvertIntoSplinedPolygon( double Aber )
{
    //! circle segments are converted to lines first.
    ConvertToLines( Aber );

    a2dLineSegment* point;
    double           cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4;
    double           x1, y1, x2, y2;

    if ( size() < 2 )
        return;

    a2dVertexList::iterator iter = --end();
    x1 = ( *iter )->m_x;
    y1 = ( *iter )->m_y;

    iter = begin();
    x2 = ( *iter )->m_x;
    y2 = ( *iter )->m_y;

    point = new a2dLineSegment( x2, y2 );
    push_back( point );

    cx1 = ( x1 + x2 ) / 2.0;
    cy1 = ( y1 + y2 ) / 2.0;
    cx2 = ( cx1 + x2 ) / 2.0;
    cy2 = ( cy1 + y2 ) / 2.0;

    iter = erase( iter );
    iter = begin();
    x1 = ( *iter )->m_x;
    y1 = ( *iter )->m_y;
    point = new a2dLineSegment( x1, y1 );
    push_back( point );

    unsigned int i = 1;
    unsigned int count = size();
    while ( i < count )
    {
        x1 = x2;
        y1 = y2;
        x2 = ( *iter )->m_x;
        y2 = ( *iter )->m_y;
        cx4 = ( x1 + x2 ) / 2.0;
        cy4 = ( y1 + y2 ) / 2.0;
        cx3 = ( x1 + cx4 ) / 2.0;
        cy3 = ( y1 + cy4 ) / 2.0;

        gds_quadratic_spline( this, cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4, Aber );

        cx1 = cx4;
        cy1 = cy4;
        cx2 = ( cx1 + x2 ) / 2.0;
        cy2 = ( cy1 + y2 ) / 2.0;
        iter = erase( iter );
        iter = begin();
        i++;
    }

    iter = begin();
    erase( iter );
}

void a2dVertexList::ConvertIntoSplinedPolyline( double Aber )
{
    double cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4;
    double x1, y1, x2, y2;


    if ( size() < 2 )
        return;

    a2dVertexList::iterator iter = begin();

    x1 = ( *iter )->m_x;
    y1 = ( *iter )->m_y;

    iter = erase( iter );
    iter = begin();
    x2 = ( *iter )->m_x;
    y2 = ( *iter )->m_y;
    cx1 = ( x1 + x2 ) / 2.0;
    cy1 = ( y1 + y2 ) / 2.0;
    cx2 = ( cx1 + x2 ) / 2.0;
    cy2 = ( cy1 + y2 ) / 2.0;

    a2dLineSegment* point = new a2dLineSegment( x1, y1 );
    push_back( point );

    iter = erase( iter );

    unsigned int i = 1;
    unsigned int count = size();
    while ( i < count )
    {
        x1 = x2;
        y1 = y2;
        x2 = ( *iter )->m_x;
        y2 = ( *iter )->m_y;
        cx4 = ( x1 + x2 ) / 2.0;
        cy4 = ( y1 + y2 ) / 2.0;
        cx3 = ( x1 + cx4 ) / 2.0;
        cy3 = ( y1 + cy4 ) / 2.0;

        gds_quadratic_spline( this, cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4, Aber );

        cx1 = cx4;
        cy1 = cy4;
        cx2 = ( cx1 + x2 ) / 2.0;
        cy2 = ( cy1 + y2 ) / 2.0;
        iter = erase( iter );
        i++;
    }

    point = new a2dLineSegment( cx1, cy1 );
    push_back( point );

    point = new a2dLineSegment( x2, y2 );
    push_back( point );
}

bool a2dVertexList::RemoveRedundant( bool polygon, double smallest )
{
    smallest = smallest * smallest;
    bool did = false;
    a2dLineSegment* segprev = NULL;
    a2dLineSegment* seg = NULL;
    a2dVertexList::iterator iter = begin();
    if ( polygon )
    {
        iter = GetPreviousAround( iter );
		segprev = *iter;
        iter = begin();
    }
    while( iter != end() )
    {
        seg = *iter;
        if ( !seg->GetArc() && segprev &&
                ( seg->GetPoint() == segprev->GetPoint() || ClclDistSqrPntPnt( seg->GetPoint(), segprev->GetPoint() ) <= smallest ) )
        {
            iter = erase( iter );
            did = true;
        }
        else
        {
            segprev = seg;
            iter++;
        }
    }
    iter = begin();
    seg = *iter;
    if ( polygon && !seg->GetArc() && segprev &&
            ( seg->GetPoint() == segprev->GetPoint() || ClclDistSqrPntPnt( seg->GetPoint(), segprev->GetPoint() ) <= smallest ) )
    {
        iter = erase( iter );
        did = true;
    }
    return did;
}

a2dVertexList* a2dVertexList::GetRedundant( bool polygon, double smallest )
{
    smallest = smallest * smallest;
    a2dVertexList* redunDant = new a2dVertexList();
    a2dLineSegment* segprev = NULL;
    a2dLineSegmentPtr seg;
    a2dVertexList::iterator iter = begin();
    while( iter != end() )
    {
        seg = *iter;
        if ( !seg->GetArc() && segprev &&
                ( seg->GetPoint() == segprev->GetPoint() || ClclDistSqrPntPnt( seg->GetPoint(), segprev->GetPoint() ) <= smallest ) )
            redunDant->push_back( seg->Clone() );
        segprev = seg;
        iter++;
    }
    iter = begin();
    seg = *iter;
    if ( polygon && !seg->GetArc() && segprev &&
            ( seg->GetPoint() == segprev->GetPoint() || ClclDistSqrPntPnt( seg->GetPoint(), segprev->GetPoint() ) <= smallest ) )
        redunDant->push_back( seg->Clone() );

    if ( redunDant->empty() )
    {
        delete redunDant;
        redunDant = NULL;
    }
    return redunDant;
}

a2dHit a2dVertexList::HitTestPolygon( const a2dPoint2D& ptest, double margin )
{
    wxASSERT( margin >= 0 );

    a2dPoint2D p1, p2, pm;

    double minDistSqrVertex = margin * margin;
    double minDistSqrStroke = margin * margin;
    bool vertexhit = false;
    int intersection_count = 0;
    a2dHit rslt;

    int i = 0;
    for( a2dVertexList::iterator iter = begin(); iter != end(); ++iter, i++ )
    {
        a2dLineSegment* seg = *iter;
        p1 = seg->GetPoint();
        if ( iter == --end() )
        {
            p2 = front()->GetPoint();
        }
        else
        {
            iter++;
            p2 = ( *iter )->GetPoint();
            iter--;
        }

        //--------------------------------------------------------------------
        // From here on, this function is identical to the hittest for
        // point array polygons. Keep these two identical !!
        //--------------------------------------------------------------------

        // check relative horizontal positions of line end points and point
        double xmin;
        double xmax;
        int direction;
        // Note: the case p1.m_x == p2.m_x is ignored during direction counting
        // See below.
        if( p1.m_x < p2.m_x )
        {
            xmin = p1.m_x;
            xmax = p2.m_x;
            direction = 1;
        }
        else
        {
            xmin = p2.m_x;
            xmax = p1.m_x;
            direction = -1;
        }

        if( ptest.m_x < xmin - margin || ptest.m_x > xmax + margin )
            continue;

        // check relative horizontal positions of line end points and point
        double ymin;
        double ymax;
        if( p1.m_y < p2.m_y )
        {
            ymin = p1.m_y;
            ymax = p2.m_y;
        }
        else
        {
            ymin = p2.m_y;
            ymax = p1.m_y;
        }

        // if ptest its y is beneath the minimum y of the segment, the ray does
        // intersect if x p1 and x p2 are on opposite sides.
        if( ptest.m_y < ymin - margin )
        {
            // The test point is definitely below the margin.
            // This cannot be a stroke hit
            // but the vertical line through ptest can intersect the edge
            if( ptest.m_x >= xmin && ptest.m_x < xmax && p1.m_x != p2.m_x )
            {
                intersection_count += direction;
            }
        }
        else if( ptest.m_y <= ymax + margin )
        {
            // The test point ptest is inside the margin extended bounding box+margin of the edge
            // This means we make vertex and stroke hit tests
            // and an intersection test between edge and the vertical line through ptest

            // Vertex hit test for point 1
            //
            // Note1: double ifs are MUCH slower than double calculations (at least on intel)
            // So it is faster calculating the distance square than doing a rectangle test
            //
            // Note2: vertex hits have higher priority than edge hits
            //
            // Note3: every vertex is once point1 and once point 2, so only one point needs
            // to be tested.
            //
            // Note4: Even if there is a stroke or vertex hit, serach is continued, because
            // there could be better hit. A stroke or vertex hit is rarely a random event,
            // so it doesn't make much sense to optimize this.

            double distSqr = ClclDistSqrPntPnt( ptest, p1 );

            if( distSqr < minDistSqrVertex )
            {
                minDistSqrVertex = distSqr;
                rslt.m_hit = a2dHit::hit_stroke;
                rslt.m_index = i;
                rslt.m_stroke2 = a2dHit::stroke2_vertex;
                vertexhit = true;
            }

            // Stroke center hit-test
            // This is done to allow hitting the stroke even if the stroke length
            // is leth than the hit margin. As vertex hits have priority, the stroke
            // coldn't be hit then. So the stroke center is treated as pseudo-vertex.
            pm = a2dPoint2D( 0.5 * ( p1.m_x + p2.m_x ), 0.5 * ( p1.m_y + p2.m_y ) );
            distSqr = ClclDistSqrPntPnt( ptest, pm );
            if( distSqr < minDistSqrVertex )
            {
                minDistSqrVertex = distSqr;
                rslt.m_hit = a2dHit::hit_stroke;
                rslt.m_index = i;
                if( p1.m_x == p2.m_x )
                    rslt.m_stroke2 = a2dHit::stroke2_edgevert;
                else if( p1.m_y == p2.m_y )
                    rslt.m_stroke2 = a2dHit::stroke2_edgehor;
                else
                    rslt.m_stroke2 = a2dHit::stroke2_edgeother;
                vertexhit = true;
            }

            // Edge hit test
            // Note: The edge hittest is even done if a higher prioroity
            // vertex hit exists, and no edgehit can accur. This is done
            // because the distance of a vertex hit shall not be worse
            // then the distance from the closest edge, which is calculated
            // here.
            distSqr = ClclDistSqrPntLine( ptest, p1, p2 );

            if( distSqr < minDistSqrStroke )
            {
                minDistSqrStroke = distSqr;
                if( !vertexhit )
                {
                    rslt.m_hit = a2dHit::hit_stroke;
                    rslt.m_index = i;
                    if( p1.m_x == p2.m_x )
                        rslt.m_stroke2 = a2dHit::stroke2_edgevert;
                    else if( p1.m_y == p2.m_y )
                        rslt.m_stroke2 = a2dHit::stroke2_edgehor;
                    else
                        rslt.m_stroke2 = a2dHit::stroke2_edgeother;
                }
            }

            // Intersection test for inside/outside test
            // Vertical lines are ignored in the inside/outside test.
            // They are really not important for this.
            // You can define that on the vertical line you get the same result as
            // above or below the vertical line.
            if( ptest.m_x >= xmin && ptest.m_x < xmax && p1.m_x != p2.m_x )
            {
                if( p1.m_y == p2.m_y )
                {
                    if( ptest.m_y <= p1.m_y )
                        intersection_count += direction;
                }
                else
                {
                    double y = ( ptest.m_x - p1.m_x ) * ( p2.m_y - p1.m_y ) / ( p2.m_x - p1.m_x ) + p1.m_y;
                    if( ptest.m_y <= y )
                        intersection_count += direction;
                }
            }
        }
    }

    if( rslt.m_hit == a2dHit::hit_stroke )
    {
        if( intersection_count )
            rslt.m_stroke1 = a2dHit::stroke1_inside;
        else
            rslt.m_stroke1 = a2dHit::stroke1_outside;

        if( vertexhit )
            rslt.m_distance = ( sqrt( wxMin( minDistSqrVertex, minDistSqrStroke ) ) / margin );
        else
            rslt.m_distance = ( sqrt( minDistSqrStroke ) / margin );
    }
    else
    {
        if( intersection_count )
            rslt.m_hit = a2dHit::hit_fill;
        else
            rslt.m_hit = a2dHit::hit_none;
    }

    return rslt;
}

a2dHit a2dVertexList::HitTestPolyline( const a2dPoint2D& ptest, double margin )
{
    wxASSERT( margin >= 0 );

    a2dPoint2D p1, p2, pm;

    double minDistSqrVertex = margin * margin;
    double minDistSqrStroke = margin * margin;
    bool vertexhit = false;
    bool lastpoint = false;
    a2dHit rslt;

    int i = 0;
    for( a2dVertexList::iterator iter = begin(); iter != end(); ++iter, i++ )
    {
        a2dLineSegment* seg = *iter;
        p1 = seg->GetPoint();

        if( iter != --end() )
        {
            iter++;
            p2 = ( *iter )->GetPoint();
            iter--;
        }
        else
            lastpoint = true;

        //--------------------------------------------------------------------
        // From here on, this function is identical to the hittest for
        // point array polylines. Keep these two identical !!
        //--------------------------------------------------------------------

        // check relative horizontal positions of line end points and point
        double xmin;
        double xmax;

        if( p1.m_x < p2.m_x )
        {
            xmin = p1.m_x;
            xmax = p2.m_x;
        }
        else
        {
            xmin = p2.m_x;
            xmax = p1.m_x;
        }

        if( ptest.m_x < xmin - margin || ptest.m_x > xmax + margin )
            continue;

        // check relative horizontal positions of line end points and point
        double ymin;
        double ymax;
        if( p1.m_y < p2.m_y )
        {
            ymin = p1.m_y;
            ymax = p2.m_y;
        }
        else
        {
            ymin = p2.m_y;
            ymax = p1.m_y;
        }

        if( ptest.m_y < ymin - margin || ptest.m_y > ymax + margin )
            continue;

        // The test point ptest is inside the margin extended bounding box+margin of the edge
        // This means we make vertex and stroke hit tests
        // and an intersection test between edge and the vertical line through ptest

        // Vertex hit test for point 1
        //
        // Note1: double ifs are MUCH slower than double calculations (at least on intel)
        // So it is faster calculating the distance square than doing a rectangle test
        //
        // Note2: vertex hits have higher priority than edge hits
        //
        // Note3: every vertex is once point1 and once point 2, so only one point needs
        // to be tested.
        //
        // Note4: Even if there is a stroke or vertex hit, search is continued, because
        // there could be better hit. A stroke or vertex hit is rarely a random event,
        // so it doesn't make much sense to optimize this.

        double distSqr = ClclDistSqrPntPnt( ptest, p1 );

        if( distSqr < minDistSqrVertex )
        {
            minDistSqrVertex = distSqr;
            rslt.m_hit = a2dHit::hit_stroke;
            rslt.m_index = i;
            rslt.m_stroke2 = a2dHit::stroke2_vertex;
            vertexhit = true;
        }

        if( lastpoint )
            break;

        // Stroke center hit-test
        // This is done to allow hitting the stroke even if the stroke length
        // is less than the hit margin. As vertex hits have priority, the stroke
        // coldn't be hit then. So the stroke center is treated as pseudo-vertex.
        pm = a2dPoint2D( 0.5 * ( p1.m_x + p2.m_x ), 0.5 * ( p1.m_y + p2.m_y ) );
        distSqr = ClclDistSqrPntPnt( ptest, pm );
        if( distSqr < minDistSqrVertex )
        {
            minDistSqrVertex = distSqr;
            rslt.m_hit = a2dHit::hit_stroke;
            rslt.m_index = i;
            if( p1.m_x == p2.m_x )
                rslt.m_stroke2 = a2dHit::stroke2_edgevert;
            else if( p1.m_y == p2.m_y )
                rslt.m_stroke2 = a2dHit::stroke2_edgehor;
            else
                rslt.m_stroke2 = a2dHit::stroke2_edgeother;
            vertexhit = true;
        }

        // Edge hit test
        // Note: The edge hittest is even done if a higher prioroity
        // vertex hit exists, and no edgehit can accur. This is done
        // because the distance of a vertex hit shall not be worse
        // then the distance from the closest edge, which is calculated
        // here.
        distSqr = ClclDistSqrPntLine( ptest, p1, p2 );

        if( distSqr < minDistSqrStroke )
        {
            minDistSqrStroke = distSqr;
            if( !vertexhit )
            {
                rslt.m_hit = a2dHit::hit_stroke;
                rslt.m_index = i;
                if( p1.m_x == p2.m_x )
                    rslt.m_stroke2 = a2dHit::stroke2_edgevert;
                else if( p1.m_y == p2.m_y )
                    rslt.m_stroke2 = a2dHit::stroke2_edgehor;
                else
                    rslt.m_stroke2 = a2dHit::stroke2_edgeother;
            }
        }
    }

    if( rslt.m_hit == a2dHit::hit_stroke )
    {
        rslt.m_stroke1 = a2dHit::stroke1_outside;
        if( vertexhit )
            rslt.m_distance = ( sqrt( wxMin( minDistSqrVertex, minDistSqrStroke ) ) / margin );
        else
            rslt.m_distance = ( sqrt( minDistSqrStroke ) / margin );
    }

    return rslt;
}

void a2dVertexList::CreateArc( const a2dPoint2D& center, const a2dPoint2D& begin, const a2dPoint2D& end, double radius, bool clock, double aber, bool addAtFront )
{
    double phi, dphi, dx, dy;
    int Segments;
    int i;
    double ang1, ang2, phit;

    dx = begin.m_x - center.m_x;
    dy = begin.m_y - center.m_y;
    ang1 = atan2( dy, dx );
    if ( ang1 < 0 ) ang1 += 2.0 * M_PI;
    dx = end.m_x - center.m_x;
    dy = end.m_y - center.m_y;
    ang2 = atan2( dy, dx );
    if ( ang2 < 0 ) ang2 += 2.0 * M_PI;

    if ( clock )
    {
        //clockwise
        if ( ang2 > ang1 )
            phit = 2.0 * M_PI - ang2 + ang1;
        else
            phit = ang1 - ang2;
    }
    else
    {
        //counter_clockwise
        if ( ang1 > ang2 )
            phit = -( 2.0 * M_PI - ang1 + ang2 );
        else
            phit = -( ang2 - ang1 );
    }

    //what is the delta phi to get an accurancy of aber
    dphi = 2 * acos( ( radius - aber ) / radius );

    //set the number of segments
    if ( phit > 0 )
        Segments = ( int )ceil( phit / dphi );
    else
        Segments = ( int )ceil( -phit / dphi );

    if ( Segments <= 1 )
        Segments = 1;
    if ( Segments > 6 )
        Segments = 6;

    dphi = phit / ( Segments );

    dx = begin.m_x - center.m_x;
    dy = begin.m_y - center.m_y;
    phi = atan2( dy, dx );

    //phi -= dphi;
    for ( i = 0; i <= Segments; i++ )
    {
        if ( addAtFront )
            push_front( new a2dLineSegment( center.m_x + radius * cos( phi ), center.m_y + radius * sin( phi ) ) );
        else
            push_back( new a2dLineSegment( center.m_x + radius * cos( phi ), center.m_y + radius * sin( phi ) ) );
        phi -= dphi;
    }
}

void a2dVertexList::CreateArc( const a2dPoint2D& center, const a2dLine& incoming,  const a2dPoint2D& end, double radius, double aber, bool addAtFront )
{
    double distance = 0;
    if ( incoming.PointOnLine( center, distance, a2dACCUR ) == R_RIGHT_SIDE )
        CreateArc( center, incoming.GetEndPoint(), end, radius, true, aber, addAtFront );
    else
        CreateArc( center, incoming.GetEndPoint(), end, radius, false, aber, addAtFront );
}

void a2dVertexList::OffsetContour_rounded( const a2dLine& currentline, const a2dLine& nextline, double factor, bool addAtFront )
{
    a2dPoint2D offs_begin;
    a2dPoint2D offs_end;
    a2dPoint2D medial_axes_point;
    a2dPoint2D offs_bgn_next;
    a2dPoint2D offs_end_next;

    // make a node from this point
    offs_end = currentline.GetEndPoint();
    offs_begin = currentline.GetBeginPoint();
    currentline.Virtual_Point( offs_begin, factor );
    currentline.Virtual_Point( offs_end, factor );
    a2dLine offs_currentline( offs_begin, offs_end );

    offs_bgn_next = nextline.GetBeginPoint();
    offs_end_next = nextline.GetEndPoint();
    nextline.Virtual_Point( offs_bgn_next, factor );
    nextline.Virtual_Point( offs_end_next, factor );

    a2dLine  offs_nextline( offs_bgn_next, offs_end_next );

    offs_currentline.CalculateLineParameters();
    offs_nextline.CalculateLineParameters();
    offs_currentline.Intersect( offs_nextline, medial_axes_point );

    double result_offs = sqrt( pow( currentline.GetEndPoint().m_x - medial_axes_point.m_x, 2 ) +
                               pow( currentline.GetEndPoint().m_y - medial_axes_point.m_y, 2 ) );

    double roundFactor = a2dGlobals->GetRoundFactor();
    if ( result_offs < fabs( roundFactor * factor ) )
    {
        if ( addAtFront )
            push_front( new a2dLineSegment( medial_axes_point ) );
        else
            push_back( new a2dLineSegment( medial_axes_point ) );
    }
    else
    {
        //let us create a circle
        CreateArc( currentline.GetEndPoint(), offs_currentline, offs_nextline.GetBeginPoint(), fabs( factor ),
                   a2dGlobals->GetAberArcToPoly(), addAtFront );
    }
}

//----------------------------------------------------------------------------
// vector path segments
//----------------------------------------------------------------------------
const int VPATHSPLINE_STEP = 20;


a2dVpathSegment::a2dVpathSegment( double x, double y, a2dPATHSEG type, a2dPATHSEG_END close )
{
    m_type = type;
    m_close = close;
    m_x1 = x;
    m_y1 = y;
    m_refcount = 0;
    m_arcPiece = false;
    m_bin = false;
}

a2dVpathSegment::a2dVpathSegment( const a2dVpathSegment& other )
{
    m_type = other.m_type;
    m_close = other.m_close;
    m_x1 = other.m_x1;
    m_y1 = other.m_y1;
    m_arcPiece = other.m_arcPiece;
    m_bin = other.m_bin;
    m_refcount = 0;
}

a2dVpathSegment::~a2dVpathSegment()
{
    wxASSERT_MSG(  m_refcount == 0, wxT( "deleting a2dVpathSegment while referenced" ) );
}

a2dVpathSegment*    a2dVpathSegment::Clone()
{
    a2dVpathSegment* a = new a2dVpathSegment( m_x1, m_y1, m_type, m_close );
    return a;
}

double a2dVpathSegment::Length( a2dVpathSegmentPtr prev )
{
    return sqrt( pow( prev->m_x1 - m_x1, 2 ) + pow( prev->m_y1 - m_y1, 2 ) );
}


a2dVpathQBCurveSegment::a2dVpathQBCurveSegment( double x1, double y1, double x2, double y2, a2dPATHSEG type, a2dPATHSEG_END close )
    : a2dVpathSegment( x1, y1 )
{
    m_type = type;
    wxASSERT_MSG( a2dPATHSEG_QBCURVETO == m_type || a2dPATHSEG_QBCURVETO_NOSTROKE == m_type , wxT( "wrong type" ) );

    m_close = close;

    m_x2 = x2;
    m_y2 = y2;
}

a2dVpathQBCurveSegment::a2dVpathQBCurveSegment( a2dVpathSegmentPtr prev, double x1, double y1, a2dPATHSEG type, a2dPATHSEG_END close )
    : a2dVpathSegment( x1, y1 )
{
    m_type = type;
    wxASSERT_MSG( a2dPATHSEG_QBCURVETO == m_type || a2dPATHSEG_QBCURVETO_NOSTROKE == m_type , wxT( "wrong type" ) );
    m_close = close;

    switch ( prev->GetType()  )
    {
        case a2dPATHSEG_QBCURVETO:
        case a2dPATHSEG_QBCURVETO_NOSTROKE:
        {
            a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) prev.Get();
            m_x2 = prev->m_x1 + ( cseg->m_x1 - cseg->m_x2 );
            m_y2 = prev->m_y1 + ( cseg->m_y1 - cseg->m_y2 );
            break;
        }
        default:
        {
            m_x2 = prev->m_x1;
            m_y2 = prev->m_y1;
        }
    }
}



a2dVpathQBCurveSegment::a2dVpathQBCurveSegment( const a2dVpathQBCurveSegment& other ): a2dVpathSegment( other )
{
    m_x2 = other.m_x2;
    m_y2 = other.m_y2;
}

a2dVpathQBCurveSegment::~a2dVpathQBCurveSegment()
{}

a2dVpathSegment* a2dVpathQBCurveSegment::Clone()
{
    a2dVpathQBCurveSegment* a = new a2dVpathQBCurveSegment( m_x1, m_y1, m_x2, m_y2, m_type, m_close );
    return a;
}

double a2dVpathQBCurveSegment::Length( a2dVpathSegmentPtr prev )
{
    int step;
    double t = 0;
    double len = 0;
    double xwl = prev->m_x1;
    double ywl = prev->m_y1;
    double xt, yt;

    for ( step = 1; step <= VPATHSPLINE_STEP; step++ )
    {
        PositionAt( prev, t, xt, yt );
        len = len + sqrt( pow( xwl - xt, 2 ) + pow( ywl - yt, 2 ) );
        t = t + 1 / ( double )VPATHSPLINE_STEP;
        xwl = xt;
        ywl = yt;

    }
    return len;
}

void a2dVpathQBCurveSegment::PositionAt( a2dVpathSegmentPtr prev, double t, double& xt, double& yt )
{
    xt = prev->m_x1 * pow( 1 - t, 2 ) + m_x2 * ( 1 - t ) * t * 2 + m_x1 * pow( t, 2 );
    yt = prev->m_y1 * pow( 1 - t, 2 ) + m_y2 * ( 1 - t ) * t * 2 + m_y1 * pow( t, 2 );
}

a2dVpathCBCurveSegment::a2dVpathCBCurveSegment( double x1, double y1, double x2, double y2, double x3, double y3, a2dPATHSEG type, a2dPATHSEG_END close )
    : a2dVpathSegment( x1, y1 )
{
    m_type = type;
    wxASSERT_MSG( ( a2dPATHSEG_CBCURVETO == m_type || a2dPATHSEG_CBCURVETO_NOSTROKE == m_type ) , wxT( "wrong type" ) );

    m_close = close;

    m_x2 = x2;
    m_y2 = y2;

    m_x3 = x3;
    m_y3 = y3;
}

a2dVpathCBCurveSegment::a2dVpathCBCurveSegment( a2dVpathSegmentPtr prev, double x1, double y1, double x3, double y3, a2dPATHSEG type, a2dPATHSEG_END close )
    : a2dVpathSegment( x1, y1 )
{
    m_type = type;
    wxASSERT_MSG( ( a2dPATHSEG_CBCURVETO == m_type || a2dPATHSEG_CBCURVETO_NOSTROKE == m_type ) , wxT( "wrong type" ) );

    m_close = close;

    switch ( prev->GetType()  )
    {
        case a2dPATHSEG_CBCURVETO:
        case a2dPATHSEG_CBCURVETO_NOSTROKE:
        {
            a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) prev.Get();
            m_x2 = prev->m_x1 + ( cseg->m_x1 - cseg->m_x3 );
            m_y2 = prev->m_y1 + ( cseg->m_y1 - cseg->m_y3 );
            break;
        }
        default:
        {
            m_x2 = prev->m_x1;
            m_y2 = prev->m_y1;
        }
    }

    m_x3 = x3;
    m_y3 = y3;
}

a2dVpathCBCurveSegment::a2dVpathCBCurveSegment( const a2dVpathCBCurveSegment& other ): a2dVpathSegment( other )
{
    m_x2 = other.m_x2;
    m_y2 = other.m_y2;

    m_x3 = other.m_x3;
    m_y3 = other.m_y3;

}

a2dVpathCBCurveSegment::~a2dVpathCBCurveSegment()
{}

a2dVpathSegment* a2dVpathCBCurveSegment::Clone()
{
    a2dVpathCBCurveSegment* a = new a2dVpathCBCurveSegment( m_x1, m_y1, m_x2, m_y2, m_x3, m_y3, m_type, m_close );
    return a;
}

double a2dVpathCBCurveSegment::Length( a2dVpathSegmentPtr prev )
{
    int step;
    double t = 0;
    double len = 0;
    double xwl = prev->m_x1;
    double ywl = prev->m_y1;
    double xt, yt;

    for ( step = 1; step <= VPATHSPLINE_STEP; step++ )
    {
        PositionAt( prev, t, xt, yt );
        len = len + sqrt( pow( xwl - xt, 2 ) + pow( ywl - yt, 2 ) );
        t = t + 1 / ( double )VPATHSPLINE_STEP;
        xwl = xt;
        ywl = yt;

    }
    return len;
}

void a2dVpathCBCurveSegment::PositionAt(  a2dVpathSegmentPtr prev, double t, double& xt, double& yt )
{
    xt = prev->m_x1 * pow( 1 - t, 3 ) + m_x2 * pow( 1 - t, 2 ) * t * 3 + m_x3 * ( 1 - t ) * t * t * 3 + m_x1 * pow( t, 3 );
    yt = prev->m_y1 * pow( 1 - t, 3 ) + m_y2 * pow( 1 - t, 2 ) * t * 3 + m_y3 * ( 1 - t ) * t * t * 3 + m_y1 * pow( t, 3 );
}

a2dVpathArcSegment::a2dVpathArcSegment( double x1, double y1, double x2, double y2, a2dPATHSEG type, a2dPATHSEG_END close )
    : a2dVpathSegment( x1, y1 )
{
    m_type = type;
    wxASSERT_MSG( a2dPATHSEG_ARCTO == m_type || a2dPATHSEG_ARCTO_NOSTROKE == m_type , wxT( "wrong type" ) );

    m_close = close;

    m_x2 = x2;
    m_y2 = y2;
}

a2dVpathArcSegment::a2dVpathArcSegment( a2dVpathSegmentPtr prev, double xc, double yc, double angle, a2dPATHSEG type, a2dPATHSEG_END close )
    : a2dVpathSegment( 0, 0 )
{
    m_type = type;
    wxASSERT_MSG( a2dPATHSEG_ARCTO == m_type || a2dPATHSEG_ARCTO_NOSTROKE == m_type , wxT( "wrong type" ) );

    m_close = close;

    double radius = sqrt( pow( prev->m_x1 - xc, 2 ) + pow( prev->m_y1 - yc, 2 ) );

    double endrad = atan2( prev->m_y1 - yc, prev->m_x1 - xc ) + wxDegToRad( angle );

    m_x1 = xc + radius * cos( endrad );
    m_y1 = yc + radius * sin( endrad );

    double midrad = atan2( prev->m_y1 - yc, prev->m_x1 - xc ) + wxDegToRad( angle / 2.0 );

    m_x2 = xc + radius * cos( midrad );
    m_y2 = yc + radius * sin( midrad );
}

a2dVpathArcSegment::a2dVpathArcSegment( a2dVpathSegmentPtr prev, double xc, double yc, double x1, double y1, double x2, double y2, a2dPATHSEG type, a2dPATHSEG_END close )
    : a2dVpathSegment( x1, y1 )
{
    m_type = type;
    wxASSERT_MSG( a2dPATHSEG_ARCTO == m_type || a2dPATHSEG_ARCTO_NOSTROKE == m_type , wxT( "wrong type" ) );

    m_close = close;

    double radius = sqrt( pow( prev->m_x1 - xc, 2 ) + pow( prev->m_y1 - yc, 2 ) );

    double endrad = atan2( m_y1 - yc, m_x1 - xc );

    m_x1 = xc + radius * cos( endrad );
    m_y1 = yc + radius * sin( endrad );

    double midrad = atan2( y2 - yc, x2 - xc );

    m_x2 = xc + radius * cos( midrad );
    m_y2 = yc + radius * sin( midrad );
}

a2dVpathArcSegment::a2dVpathArcSegment( const a2dVpathArcSegment& other ): a2dVpathSegment( other )
{
    m_x2 = other.m_x2;
    m_y2 = other.m_y2;
}

a2dVpathArcSegment::~a2dVpathArcSegment()
{}

a2dVpathSegment* a2dVpathArcSegment::Clone()
{
    a2dVpathArcSegment* a = new a2dVpathArcSegment( m_x1, m_y1, m_x2, m_y2, m_type, m_close );
    return a;
}

double a2dVpathArcSegment::Length( a2dVpathSegmentPtr prev )
{
    double len, radius, center_x, center_y, beginrad, midrad, endrad, phit;

    if ( CalcR( prev, radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
    {
        len = fabs( phit * radius );
    }
    else
        len = sqrt( pow( prev->m_x1 - m_x1, 2 ) + pow( prev->m_y1 - m_y1, 2 ) );

    return len;
}

bool a2dVpathArcSegment::CalcR( a2dVpathSegmentPtr prev, double& radius, double& center_x, double& center_y, double& beginrad, double& midrad, double& endrad, double& phit, double marge )
{
    return ::CalcR( prev->m_x1, prev->m_y1, m_x2, m_y2, m_x1, m_y1,
                    radius, center_x, center_y, beginrad, midrad, endrad, phit, marge );
}


//----------------------------------------------------------------------------
// a2dVpath
//----------------------------------------------------------------------------

a2dVpath::a2dVpath()
{}

a2dVpath::a2dVpath( a2dVertexArray& vertexArray, bool moveToFirst, bool closeLast )
{
    Add( vertexArray, moveToFirst, closeLast );
}

a2dVpath::a2dVpath( a2dVertexList& vertexList, bool moveToFirst, bool closeLast )
{
    Add( vertexList, moveToFirst, closeLast );
}

a2dVpath::~a2dVpath()
{}

a2dVpath& a2dVpath::operator=( const a2dVpath& other )
{
    clear();
    unsigned int i;
    for ( i = 0; i < other.size(); i++ )
    {
        a2dVpathSegment* seg = other.Item( i )->Clone();
        Add( seg );
    }

    return *this;
}

void a2dVpath::Add( a2dVertexArray& vertexArray, bool moveToFirst, bool closeLast )
{
    unsigned int i;
    for ( i = 0; i < vertexArray.size(); i++ )
    {
        a2dLineSegmentPtr seg = vertexArray.Item( i );
        a2dPATHSEG_END close = a2dPATHSEG_END_OPEN;
        if ( i == vertexArray.size() - 1 && closeLast )
            close = a2dPATHSEG_END_CLOSED;
        if ( seg->GetArc() )
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg.Get();
            a2dVpathArcSegment* segn = new a2dVpathArcSegment( cseg->m_x, cseg->m_y, cseg->m_x2, cseg->m_y2, ( moveToFirst && i == 0 ) ? a2dPATHSEG_MOVETO : a2dPATHSEG_ARCTO, close );
            push_back( segn );
        }
        else
        {
            a2dVpathSegment* segn = new a2dVpathSegment( seg->m_x, seg->m_y, ( moveToFirst && i == 0 ) ? a2dPATHSEG_MOVETO : a2dPATHSEG_LINETO, close );
            push_back( segn );
        }
    }
}

void a2dVpath::Add( a2dVertexList& vertexList, bool moveToFirst, bool closeLast )
{
    for( a2dVertexList::iterator iter = vertexList.begin(); iter != vertexList.end(); ++iter )
    {
        a2dLineSegment* seg = *iter;
        a2dPATHSEG_END close = a2dPATHSEG_END_OPEN;
        if ( *iter == vertexList.back() && closeLast )
            close = a2dPATHSEG_END_CLOSED;
        if ( seg->GetArc() )
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
            a2dVpathArcSegment* segn = new a2dVpathArcSegment( cseg->m_x, cseg->m_y, cseg->m_x2, cseg->m_y2, ( moveToFirst && iter == vertexList.begin() ) ? a2dPATHSEG_MOVETO : a2dPATHSEG_ARCTO, close );
            push_back( segn );
        }
        else
        {
            a2dVpathSegment* segn = new a2dVpathSegment( seg->m_x, seg->m_y, ( moveToFirst && iter == vertexList.begin() ) ? a2dPATHSEG_MOVETO : a2dPATHSEG_LINETO, close );
            push_back( segn );
        }
    }
}

void a2dVpath::MoveTo( double x, double y )
{
    a2dVpathSegment* seg = new a2dVpathSegment( x, y, a2dPATHSEG_MOVETO, a2dPATHSEG_END_OPEN );
    Add( seg );
}

void a2dVpath::LineTo( double x, double y, bool withStroke )
{
    a2dPATHSEG cmd = withStroke ? a2dPATHSEG_LINETO : a2dPATHSEG_LINETO_NOSTROKE;
    a2dVpathSegment* seg = new a2dVpathSegment( x, y, cmd );
    Add( seg );
}

void a2dVpath::QBCurveTo( double x1, double y1, double x2, double y2, bool withStroke )
{
    a2dPATHSEG cmd = withStroke ? a2dPATHSEG_QBCURVETO : a2dPATHSEG_QBCURVETO_NOSTROKE;
    a2dVpathQBCurveSegment* seg =
        new a2dVpathQBCurveSegment( x1, y1, x2, y2, cmd );
    Add( seg );
}

void a2dVpath::QBCurveTo( double x1, double y1, bool withStroke )
{
    a2dPATHSEG cmd = withStroke ? a2dPATHSEG_QBCURVETO : a2dPATHSEG_QBCURVETO_NOSTROKE;
    a2dVpathQBCurveSegment* seg = new a2dVpathQBCurveSegment(
        back().Get(), x1, y1, cmd );
    Add( seg );
}

void a2dVpath::CBCurveTo( double x1, double y1, double x2, double y2, double x3, double y3, bool withStroke )
{
    a2dPATHSEG cmd = withStroke ? a2dPATHSEG_CBCURVETO : a2dPATHSEG_CBCURVETO_NOSTROKE;
    a2dVpathCBCurveSegment* seg = new a2dVpathCBCurveSegment(
        x1, y1, x2, y2, x3, y3, cmd );
    Add( seg );
}

void a2dVpath::CBCurveTo( double x1, double y1, double x3, double y3, bool withStroke )
{
    a2dPATHSEG cmd = withStroke ? a2dPATHSEG_CBCURVETO : a2dPATHSEG_CBCURVETO_NOSTROKE;
    a2dVpathCBCurveSegment* seg = new a2dVpathCBCurveSegment(
        back().Get(), x1, y1, x3, y3, cmd );
    Add( seg );
}

void a2dVpath::ArcTo( double x1, double y1, double x2, double y2, bool withStroke )
{
    a2dPATHSEG cmd = withStroke ? a2dPATHSEG_ARCTO : a2dPATHSEG_ARCTO_NOSTROKE;
    a2dVpathArcSegment* seg = new a2dVpathArcSegment(
        x1, y1, x2, y2, cmd );
    Add( seg );
}

void a2dVpath::ArcTo( double xc, double yc, double angle, bool withStroke )
{
    a2dPATHSEG cmd = withStroke ? a2dPATHSEG_ARCTO : a2dPATHSEG_ARCTO_NOSTROKE;
    a2dVpathArcSegment* seg = new a2dVpathArcSegment(
        back().Get(), xc, yc, angle, cmd );
    Add( seg );
}

void a2dVpath::ArcTo( double xc, double yc, double x1, double y1, double x2, double y2, bool withStroke )
{
    a2dPATHSEG cmd = withStroke ? a2dPATHSEG_ARCTO : a2dPATHSEG_ARCTO_NOSTROKE;
    a2dVpathArcSegment* seg = new a2dVpathArcSegment(
        back().Get(), xc, yc, x1, y1, x2, y2, cmd );
    Add( seg );
}

void a2dVpath::Close( bool withStroke )
{
    if ( !size() )
    {
        return;
    }
    back()->SetClose( withStroke ? a2dPATHSEG_END_CLOSED : a2dPATHSEG_END_CLOSED_NOSTROKE );
}


double a2dVpath::Length()
{
    double len = 0;
    unsigned int i;
    for ( i = 1; i < size(); i++ )
    {
        len = len + Item( i )->Length( Item( i - 1 ) );
    }

    return len;
}

void a2dVpath::RemoveAt( size_t index )
{
    size_t i = 0;
    for( a2dVpath::iterator iter = begin(); iter != end(); ++iter )
    {
        if ( i == index )
        {
            erase( iter );
            break;
        }
        i++;
    }
}

void a2dVpath::Insert( a2dVpathSegment* segment, size_t index )
{
    size_t i = 0;
    for( a2dVpath::iterator iter = begin(); iter != end(); ++iter )
    {
        if ( i == index )
        {
            insert( iter, segment );
            break;
        }
        i++;
    }
}


bool a2dVpath::IsPolygon( bool allowArc )
{
    if ( size() && Item( size() - 1 )->GetClose() != a2dPATHSEG_END_CLOSED ) //last segment oke?
        return false;
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        a2dVpathSegmentPtr seg = Item( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_MOVETO:
                if ( i != 0 )
                    return false;
            case a2dPATHSEG_LINETO:
            case a2dPATHSEG_ARCTO:
                if ( !allowArc )
                    return false;
                break;
            default:
                return false;
        }
    }
    return true;
}

bool a2dVpath::IsPolyline( bool allowArc )
{
    if ( size() && Item( size() - 1 )->GetClose() != a2dPATHSEG_END_OPEN ) //last segment oke?
        return false;
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        a2dVpathSegmentPtr seg = Item( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_MOVETO:
                if ( i != 0 )
                    return false;
            case a2dPATHSEG_LINETO:
            case a2dPATHSEG_ARCTO:
                if ( !allowArc )
                    return false;
                break;
            default:
                return false;
        }
    }
    return true;
}

void a2dVpath::Transform( const a2dAffineMatrix& cworld )
{
    if ( !cworld.IsIdentity() )
    {
        if ( !cworld.IsTranslate() )
            ConvertToLines();

        unsigned int i;
        for ( i = 0; i < size(); i++ )
        {
            a2dVpathSegmentPtr seg = Item( i );
            switch ( seg->GetType()  )
            {
                case a2dPATHSEG_CBCURVETO_NOSTROKE:
                case a2dPATHSEG_CBCURVETO:
                {
                    a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();
                    cworld.TransformPoint( cseg->m_x2, cseg->m_y2, cseg->m_x2, cseg->m_y2 );
                    cworld.TransformPoint( cseg->m_x3, cseg->m_y3, cseg->m_x3, cseg->m_y3 );
                }
                break;

                case a2dPATHSEG_QBCURVETO_NOSTROKE:
                case a2dPATHSEG_QBCURVETO:
                {
                    a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();
                    cworld.TransformPoint( cseg->m_x2, cseg->m_y2, cseg->m_x2, cseg->m_y2 );
                }
                break;

                case a2dPATHSEG_ARCTO_NOSTROKE:
                case a2dPATHSEG_ARCTO:
                {
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();
                    cworld.TransformPoint( cseg->m_x2, cseg->m_y2, cseg->m_x2, cseg->m_y2 );
                }
                break;
                default:
                    break;
            }

            cworld.TransformPoint( seg->m_x1, seg->m_y1, seg->m_x1, seg->m_y1 );
        }
    }
}

a2dBoundingBox a2dVpath::GetBbox( const a2dAffineMatrix& lworld )
{
    a2dBoundingBox bbox;
    double x, y;
    unsigned int i;
    for ( i = 0; i < size(); i++ )
    {
        a2dVpathSegmentPtr seg = Item( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_CBCURVETO:
            case a2dPATHSEG_CBCURVETO_NOSTROKE:
            {
                a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();
                lworld.TransformPoint( cseg->m_x2, cseg->m_y2, x, y );
                bbox.Expand( x, y );
                lworld.TransformPoint( cseg->m_x3, cseg->m_y3, x, y );
                bbox.Expand( x, y );
            }
            break;

            case a2dPATHSEG_QBCURVETO:
            case a2dPATHSEG_QBCURVETO_NOSTROKE:
            {
                a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();
                lworld.TransformPoint( cseg->m_x2, cseg->m_y2, x, y );
                bbox.Expand( x, y );
            }
            break;

            case a2dPATHSEG_ARCTO:
            case a2dPATHSEG_ARCTO_NOSTROKE:
            {
                a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();

                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                //TODO soemthing to get the actual boundingbox of the arc
                if ( cseg->CalcR( Item( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                {
                    lworld.TransformPoint( center_x + radius , center_y + radius, x, y );
                    bbox.Expand( x, y );
                    lworld.TransformPoint( center_x + radius , center_y - radius, x, y );
                    bbox.Expand( x, y );
                    lworld.TransformPoint( center_x - radius , center_y + radius, x, y );
                    bbox.Expand( x, y );
                    lworld.TransformPoint( center_x - radius , center_y - radius, x, y );
                    bbox.Expand( x, y );
                }

                lworld.TransformPoint( cseg->m_x2, cseg->m_y2, x, y );
                bbox.Expand( x, y );
            }
            break;
            default:
                break;
        }
        lworld.TransformPoint( Item( i )->m_x1, Item( i )->m_y1, x, y );
        bbox.Expand( x, y );
    }
    return bbox;
}

void a2dVpath::ConvertToLines( double aberation )
{
    double xw, yw;
    bool nostroke = false;
    unsigned int i;
    a2dVpathSegment* segn;
    double tstep = 1 / ( double ) VPATHSPLINE_STEP;

    for ( i = 0; i < size(); i++ )
    {
        a2dVpathSegmentPtr seg = Item( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_CBCURVETO_NOSTROKE:
                nostroke = true;
            case a2dPATHSEG_CBCURVETO:
            {
                double xw, yw;

                double xwl = Item( i ? i - 1 : 0 )->m_x1;
                double ywl = Item( i ? i - 1 : 0 )->m_y1;
                a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();

                int step;
                double t = 0;
                for ( step = 0; step <= VPATHSPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                    yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
                    if ( !nostroke )
                        segn = new a2dVpathSegment( xw, yw, a2dPATHSEG_LINETO );
                    else
                        segn = new a2dVpathSegment( xw, yw, a2dPATHSEG_LINETO_NOSTROKE );

                    if ( step == VPATHSPLINE_STEP )
                        segn->m_close = seg->m_close;

                    Insert( segn, i++ );
                    t = t + tstep;
                }
                RemoveAt( i-- );
            }
            break;

            case a2dPATHSEG_QBCURVETO_NOSTROKE:
                nostroke = true;
            case a2dPATHSEG_QBCURVETO:
            {
                double xw, yw;

                double xwl = Item( i ? i - 1 : 0 )->m_x1;
                double ywl = Item( i ? i - 1 : 0 )->m_y1;
                a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();

                int step;
                double t = 0;
                for ( step = 0; step <= VPATHSPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                    yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
                    if ( !nostroke )
                        segn = new a2dVpathSegment( xw, yw, a2dPATHSEG_LINETO );
                    else
                        segn = new a2dVpathSegment( xw, yw, a2dPATHSEG_LINETO_NOSTROKE );

                    if ( step == VPATHSPLINE_STEP )
                        segn->m_close = seg->m_close;

                    Insert( segn, i++ );
                    t = t + tstep;
                }
                RemoveAt( i-- );
            }
            break;

            case a2dPATHSEG_ARCTO_NOSTROKE:
                nostroke = true;
            case a2dPATHSEG_ARCTO:
            {
                a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();

                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                if ( cseg->CalcR( Item( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                {
                    double dphi;
                    unsigned int segments = 20;
                    if ( aberation )
                        Aberration( radius / aberation, phit, radius , dphi, segments );
                    else 
                        Aberration( radius / 200, phit, radius , dphi, segments );
                    //a2dGlobals->Aberration( phit, radius , dphi, segments );

                    double theta = beginrad;
                    unsigned int step;
                    theta = theta + dphi; //skip first point, which was from previous segment.

                    for ( step = 0; step < segments; step++ )
                    {
                        xw = center_x + radius * cos ( theta );
                        yw = center_y + radius * sin ( theta );
                        if ( !nostroke )
                            segn = new a2dVpathSegment( xw, yw, a2dPATHSEG_LINETO );
                        else
                            segn = new a2dVpathSegment( xw, yw, a2dPATHSEG_LINETO_NOSTROKE );

                        if ( step == segments-1 )
                            segn->m_close = seg->m_close;

                        Insert( segn, i++ );
                        theta = theta + dphi;
                    }
                }
                else
                {
                    if ( !nostroke )
                        segn = new a2dVpathSegment( cseg->m_x1, cseg->m_y1, a2dPATHSEG_LINETO );
                    else
                        segn = new a2dVpathSegment( cseg->m_x1, cseg->m_y1, a2dPATHSEG_LINETO_NOSTROKE );

                    segn->m_close = seg->m_close;
                    Insert( segn, i++ );
                }
                RemoveAt( i-- );
            }
            break;
            default:
                break;
        }
        nostroke = false;
    }
}

void a2dVpath::ConvertToPolygon( a2dListOfa2dVertexList& addTo, bool arc )
{
    double xw, yw;
    bool nostroke = false;
    unsigned int i;
    double tstep = 1 / ( double ) VPATHSPLINE_STEP;
    a2dVertexList* result = NULL;

    for ( i = 0; i < size(); i++ )
    {
        a2dVpathSegmentPtr seg = Item( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_MOVETO:
            {
                result = new a2dVertexList();
                addTo.push_back( result );

                result->push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
                break;
            }
            case a2dPATHSEG_LINETO_NOSTROKE:
                nostroke = true;
            case a2dPATHSEG_LINETO:
            {
                result->push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
            }
            break;
            case a2dPATHSEG_CBCURVETO_NOSTROKE:
                nostroke = true;
            case a2dPATHSEG_CBCURVETO:
            {
                double xw, yw;

                double xwl = Item( i ? i - 1 : 0 )->m_x1;
                double ywl = Item( i ? i - 1 : 0 )->m_y1;
                a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();

                int step;
                double t = 0;
                for ( step = 0; step <= VPATHSPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                    yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
                    result->push_back( new a2dLineSegment( xw, yw ) );
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_QBCURVETO_NOSTROKE:
                nostroke = true;
            case a2dPATHSEG_QBCURVETO:
            {
                double xw, yw;

                double xwl = Item( i ? i - 1 : 0 )->m_x1;
                double ywl = Item( i ? i - 1 : 0 )->m_y1;
                a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();

                int step;
                double t = 0;
                for ( step = 0; step <= VPATHSPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                    yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
                    result->push_back( new a2dLineSegment( xw, yw ) );
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_ARCTO_NOSTROKE:
                nostroke = true;
            case a2dPATHSEG_ARCTO:
            {
                a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();

                if ( arc )
                {
                    double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                    if ( cseg->CalcR( Item( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                    {
                        double dphi;
                        unsigned int segments = 20;
                        Aberration( radius / 200, phit, radius , dphi, segments );
                        //a2dGlobals->Aberration( phit, radius , dphi, segments );

                        double theta = beginrad;
                        unsigned int step;

                        for ( step = 0; step < segments + 1; step++ )
                        {
                            xw = center_x + radius * cos ( theta );
                            yw = center_y + radius * sin ( theta );
                            result->push_back( new a2dLineSegment( xw, yw ) );
                            theta = theta + dphi;
                        }
                    }
                    else
                    {
                        result->push_back( new a2dLineSegment( cseg->m_x1, cseg->m_y1 ) );
                    }
                }
                else
                    result->push_back( new a2dArcSegment( cseg->m_x1, cseg->m_y1, cseg->m_x2, cseg->m_y2 ) );
            }
            break;
            default:
                break;
        }
        nostroke = false;
    }
}

void a2dVpath::Contour( double distance, a2dPATH_END_TYPE pathtype )
{
    ConvertToLines();

    a2dVpath totalconverted;
    bool move = false;

    if ( distance )
    {
        unsigned int i = 0;
        int subpartstart = 0;
        int subpartLength = 0;
        while ( i < size() )
        {
            a2dVpathSegmentPtr seg = Item( i );
            switch ( seg->GetType()  )
            {
                case a2dPATHSEG_MOVETO:
                {
                    if ( i == 0 ) // the first should always be Move
                        subpartLength++;
                    else
                        move = true;
                    break;
                }
                case a2dPATHSEG_LINETO:
                case a2dPATHSEG_LINETO_NOSTROKE:
                    subpartLength++;
                    break;
                default:
                    break;
            }

            if ( move )
            {
                //we have found one subpart, process it
                SingleContour( totalconverted, subpartstart, subpartLength, distance, pathtype );
                move = false;
                subpartLength = 1;
                subpartstart = i;
            }
            i++;
        }

        if ( subpartLength )
        {
            //we have found one subpart, process it
            SingleContour( totalconverted, subpartstart, subpartLength, distance, pathtype );
        }

        *this = totalconverted;
    }
}

void a2dVpath::SingleContour( a2dVpath& converted, unsigned int start, unsigned int segments, double distance, a2dPATH_END_TYPE pathtype )
{
    //bool rounded  = true;
    a2dPoint2D _first;
    a2dPoint2D _middle;
    a2dPoint2D _end;
    a2dPoint2D offsetpointleft;
    a2dPoint2D offsetpointright;

    unsigned int insertAt = converted.size();

    //create start of subpart
    a2dVpathSegmentPtr seg = Item( start );
    if ( segments == 1 )
    {
        converted.Add( new a2dVpathSegment( seg->m_x1 - distance, seg->m_y1 , a2dPATHSEG_MOVETO, seg->GetClose() ) );
        switch ( pathtype )
        {
            case a2dPATH_END_SQAURE:
            {
                //not defined
            }
            break;
            case a2dPATH_END_ROUND:
            {
                a2dVpathSegment* segn = new a2dVpathArcSegment( seg->m_x1 + distance, seg->m_y1,
                        seg->m_x1 - distance, seg->m_y1, a2dPATHSEG_ARCTO, a2dPATHSEG_END_OPEN );

                converted.Add( segn );
            }
            break;
            case a2dPATH_END_SQAURE_EXT:
            {
                //not defined
            }
            break;
            default:
                break;
        }
        return;
    }
    else
    {
        //first begin point
        _first = a2dPoint2D( seg->m_x1, seg->m_y1 );
        a2dVpathSegmentPtr segnext = Item( start + 1 );
        _middle = a2dPoint2D( segnext->m_x1, segnext->m_y1 );
        a2dLine line1( _first, _middle );
        line1.CalculateLineParameters();
        offsetpointleft = _first;
        offsetpointright = _first;
        line1.Virtual_Point( offsetpointleft, distance );
        line1.Virtual_Point( offsetpointright, -distance );

        switch ( pathtype )
        {
            case a2dPATH_END_SQAURE:
            {
                converted.Add( new a2dVpathSegment( offsetpointleft.m_x, offsetpointleft.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN ) );
                converted.Insert( new a2dVpathSegment( offsetpointright.m_x, offsetpointright.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN ), insertAt );
            }
            break;
            case a2dPATH_END_ROUND:
            {
                a2dLine  lineoffset( offsetpointright, offsetpointleft );
                lineoffset.CalculateLineParameters();
                a2dPoint2D offsetpointfirst;

                offsetpointfirst = _first;
                lineoffset.Virtual_Point( offsetpointfirst, distance );
                converted.Add( new a2dVpathArcSegment( offsetpointleft.m_x, offsetpointleft.m_y,
                                                       offsetpointfirst.m_x, offsetpointfirst.m_y,
                                                       a2dPATHSEG_ARCTO, a2dPATHSEG_END_OPEN ) );
                converted.Insert( new a2dVpathSegment( offsetpointright.m_x, offsetpointright.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN ), insertAt );
                break;
            }
            case a2dPATH_END_SQAURE_EXT:
            {
                a2dLine  lineoffset( offsetpointright, offsetpointleft );
                lineoffset.CalculateLineParameters();
                lineoffset.Virtual_Point( offsetpointleft, distance );
                lineoffset.Virtual_Point( offsetpointright, distance );

                converted.Add( new a2dVpathSegment( offsetpointleft.m_x, offsetpointleft.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN ) );
                converted.Insert( new a2dVpathSegment( offsetpointright.m_x, offsetpointright.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN ), insertAt  );
                break;
            }
            default:
                break;
        }

    }

    //in between start and end
    unsigned int i;
    for ( i = start + 1; i < start + segments - 1; i++ )
    {
        a2dVpathSegmentPtr seg = Item( i - 1 );
        _first = a2dPoint2D( seg->m_x1, seg->m_y1 );
        a2dVpathSegmentPtr segnext = Item( i );
        _middle = a2dPoint2D( segnext->m_x1, segnext->m_y1 );
        a2dLine line1( _first, _middle );
        a2dVpathSegmentPtr segend = Item( i + 1 );
        _end = a2dPoint2D( segend->m_x1, segend->m_y1 );
        a2dLine line2( _middle, _end );
        line1.CalculateLineParameters();
        line2.CalculateLineParameters();

        OUTPRODUCT _outproduct;
        _outproduct = line1.OutProduct( line2, 1e-9 );

        switch ( _outproduct )
        {
                // Line 2 lies on  leftside of this line
            case R_IS_RIGHT :
            {
                line1.OffsetContour( line2, distance, offsetpointleft );
                line1.OffsetContour( line2, -distance, offsetpointright );
            }
            break;
            case R_IS_LEFT :
            {
                line1.OffsetContour( line2, distance, offsetpointleft );
                line1.OffsetContour( line2, -distance, offsetpointright );
            }
            break;
            // Line 2 lies on this line
            case R_IS_ON     :
            {
                offsetpointleft = _middle;
                offsetpointright = _middle;
                line1.Virtual_Point( offsetpointleft, distance );
                line1.Virtual_Point( offsetpointright, -distance );
            }
            break;
            default:
                wxFAIL_MSG( wxT( "wrong line code" ) );
                break;
        }//end switch

        converted.Add( new a2dVpathSegment( offsetpointleft.m_x, offsetpointleft.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN ) );
        converted.Insert( new a2dVpathSegment( offsetpointright.m_x, offsetpointright.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN ), insertAt );
    }

    seg = Item( i - 1 );
    a2dVpathSegmentPtr segnext = Item( i );

    if ( segnext->GetClose() == a2dPATHSEG_END_CLOSED )
    {
        _first = a2dPoint2D( seg->m_x1, seg->m_y1 );
        _middle = a2dPoint2D( segnext->m_x1, segnext->m_y1 );
        a2dLine line1( _first, _middle );
        a2dVpathSegmentPtr segend = Item( i - segments + 1 );
        _end = a2dPoint2D( segend->m_x1, segend->m_y1 );
        a2dLine line2( _middle, _end );
        line1.CalculateLineParameters();
        line2.CalculateLineParameters();

        OUTPRODUCT _outproduct;
        _outproduct = line1.OutProduct( line2, 1e-9 );

        switch ( _outproduct )
        {
                // Line 2 lies on  leftside of this line
            case R_IS_RIGHT :
            {
                line1.OffsetContour( line2, distance, offsetpointleft );
                line1.OffsetContour( line2, -distance, offsetpointright );
            }
            break;
            case R_IS_LEFT :
            {
                line1.OffsetContour( line2, distance, offsetpointleft );
                line1.OffsetContour( line2, -distance, offsetpointright );
            }
            break;
            // Line 2 lies on this line
            case R_IS_ON     :
            {
                offsetpointleft = _middle;
                offsetpointright = _middle;
                line1.Virtual_Point( offsetpointleft, distance );
                line1.Virtual_Point( offsetpointright, -distance );
            }
            break;
            default:
                wxFAIL_MSG( wxT( "wrong line code" ) );
                break;
        }//end switch

        converted.Add( new a2dVpathSegment( offsetpointleft.m_x, offsetpointleft.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN ) );
        converted.Insert( new a2dVpathSegment( offsetpointright.m_x, offsetpointright.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN ), insertAt );

        seg = Item( i );
        segnext = Item( i - segments + 1 );
    }


    //if ( seg.GetClose() == a2dPATHSEG_END_OPEN || a2dPATHSEG_END_CLOSED_NOSTROKE )
    {
        //first begin point
        _first = a2dPoint2D( seg->m_x1, seg->m_y1 );
        _end = a2dPoint2D( segnext->m_x1, segnext->m_y1 );
        a2dLine line1( _first, _end );
        line1.CalculateLineParameters();
        offsetpointleft = _end;
        offsetpointright = _end;
        line1.Virtual_Point( offsetpointleft, distance );
        line1.Virtual_Point( offsetpointright, -distance );

        switch ( pathtype )
        {
            case a2dPATH_END_SQAURE:
            {
                converted.Add( new a2dVpathSegment( offsetpointleft.m_x, offsetpointleft.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED ) );
                converted.Insert( new a2dVpathSegment( offsetpointright.m_x, offsetpointright.m_y, a2dPATHSEG_MOVETO, a2dPATHSEG_END_OPEN ), insertAt );
            }
            break;
            case a2dPATH_END_ROUND:
            {
                a2dLine  lineoffset( offsetpointleft, offsetpointright );
                lineoffset.CalculateLineParameters();
                a2dPoint2D offsetpointend;

                offsetpointend = _end;
                lineoffset.Virtual_Point( offsetpointend, distance );
                converted.Add( new a2dVpathSegment( offsetpointleft.m_x, offsetpointleft.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN ) );
                converted.Add( new a2dVpathArcSegment( offsetpointright.m_x, offsetpointright.m_y,
                                                       offsetpointend.m_x, offsetpointend.m_y,
                                                       a2dPATHSEG_ARCTO, a2dPATHSEG_END_CLOSED ) );
                converted.Insert( new a2dVpathSegment( offsetpointright.m_x, offsetpointright.m_y, a2dPATHSEG_MOVETO, a2dPATHSEG_END_OPEN ), insertAt );
                break;
            }
            case a2dPATH_END_SQAURE_EXT:
            {
                a2dLine  lineoffset( offsetpointright, offsetpointleft );
                lineoffset.CalculateLineParameters();
                lineoffset.Virtual_Point( offsetpointleft, -distance );
                lineoffset.Virtual_Point( offsetpointright, -distance );

                converted.Add( new a2dVpathSegment( offsetpointleft.m_x, offsetpointleft.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED ) );
                converted.Insert( new a2dVpathSegment( offsetpointright.m_x, offsetpointright.m_y, a2dPATHSEG_MOVETO, a2dPATHSEG_END_OPEN ), insertAt );
                break;
            }
            default:
                break;
        }
    }
}

/********************* CURVES FOR SPLINES *****************************

  The following spline drawing routine is from

    "An Algorithm for High-Speed Curve Generation"
    by George Merrill Chaikin,
    Computer Graphics and Image Processing, 3, Academic Press,
    1974, 346-349.

      and

        "On Chaikin's Algorithm" by R. F. Riesenfeld,
        Computer Graphics and Image Processing, 4, Academic Press,
        1975, 304-310.

***********************************************************************/

#define     half(z1, z2)    ((z1+z2)/2.0)

/* iterative version */

static void gds_quadratic_spline( a2dVertexList* org, double a1, double b1, double a2, double b2, double a3, double b3, double a4,
                                  double b4, double Aber )
{
    register double  xmid, ymid;
    double           x1, y1, x2, y2, x3, y3, x4, y4;
    a2dLineSegment* point;

    gds_clear_stack();
    gds_spline_push( a1, b1, a2, b2, a3, b3, a4, b4 );

    while ( gds_spline_pop( &x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4 ) )
    {
        xmid = half( x2, x3 );
        ymid = half( y2, y3 );
        if ( fabs( x1 - xmid ) < Aber && fabs( y1 - ymid ) < Aber &&
                fabs( xmid - x4 ) < Aber && fabs( ymid - y4 ) < Aber )
        {
            point = new a2dLineSegment( x1, y1 );
            org->push_back( point );
            point = new a2dLineSegment( xmid, ymid );
            org->push_back( point );
        }
        else
        {
            gds_spline_push( xmid, ymid, half( xmid, x3 ), half( ymid, y3 ),
                             half( x3, x4 ), half( y3, y4 ), x4, y4 );
            gds_spline_push( x1, y1, half( x1, x2 ), half( y1, y2 ),
                             half( x2, xmid ), half( y2, ymid ), xmid, ymid );
        }
    }
}

bool CalcR( double begin_x, double begin_y, double middle_x, double middle_y, double end_x, double end_y,
            double& radius, a2dPoint2D& center_p )
{
    double center_x, center_y, beginrad, midrad, endrad, phit;
    bool ret = CalcR( begin_x, begin_y, middle_x, middle_y, end_x, end_y,
                      radius, center_x, center_y, beginrad, midrad, endrad, phit );
    center_p.m_x = center_x;
    center_p.m_y = center_y;
    return ret;
}

bool CalcR( double begin_x, double begin_y, double middle_x, double middle_y, double end_x, double end_y,
            double& radius, double& center_x, double& center_y, double& beginrad, double& midrad, double& endrad, double& phit, double marge )
{
    double ax, ay, bx, by, cx, cy;
    double ax2, ay2, bx2, by2, cx2, cy2;
    double d;
    bool straight = false;

    ax = begin_x;   ay = begin_y;  ax2 = pow( ax, 2 ); ay2 = pow( ay, 2 );
    bx = middle_x;  by = middle_y; bx2 = pow( bx, 2 ); by2 = pow( by, 2 );
    cx = end_x;     cy = end_y;    cx2 = pow( cx, 2 ); cy2 = pow( cy, 2 );

    if ( fabs( ax - cx ) < marge && fabs( ay - cy ) <  marge )
    {
        //assume the middle is exact 180 opposite.
        center_x = ( bx + ax ) / 2.0;
        center_y = ( by + ay ) / 2.0;
        radius = sqrt( pow( bx - ax, 2 ) + pow( by - ay, 2 ) ) / 2.0;
    }
    else
    {
        d = 2 * ( ay * cx + by * ax - by * cx - ay * bx - cy * ax + cy * bx );

        if ( fabs( d ) <  marge ) // (infinite radius)
        {
            d =  marge;
            straight = true;
        }

        center_x = (  by * ax2 - cy * ax2 - by2 * ay + cy2 * ay + bx2 * cy + ay2 * by
                      + cx2 * ay - cy2 * by - cx2 * by - bx2 * ay + by2 * cy - ay2 * cy
                   ) / d;

        center_y = (  ax2 * cx + ay2 * cx + bx2 * ax - bx2 * cx + by2 * ax - by2 * cx
                      - ax2 * bx - ay2 * bx - cx2 * ax + cx2 * bx - cy2 * ax + cy2 * bx
                   ) / d ;

        radius = sqrt( pow( ax - center_x, 2 ) + pow( ay - center_y, 2 ) );
    }

    if ( radius )
    {
        //beginrad between PI and -PI, assume counterclockwise, make endrad > beginrad.
        beginrad = atan2( begin_y - center_y, begin_x - center_x );
        midrad   = atan2( middle_y - center_y, middle_x - center_x );
        endrad   = atan2( end_y - center_y, end_x - center_x );

        double mr, er;

        //if going counterclockwise from begin to end we pass trough middle => counterclockwise
        //if going counterclockwise from begin to end we do NOT pass trough middle => clockwise

        //rotate such that beginrad = zero
        mr = midrad - beginrad;
        er = endrad - beginrad;

        //make positive
        if ( mr <= 0 ) mr += 2.0 * M_PI;
        if ( er <= 0 ) er += 2.0 * M_PI;
        if ( mr >= 2.0 * M_PI ) mr -= 2.0 * M_PI;
        if ( er >= 2.0 * M_PI ) er -= 2.0 * M_PI;

        bool counterclock;

        //middle gives direction
        if ( mr < er )
        {
            phit = er; //counterclockwise
            counterclock = true;
        }
        else
        {
            phit = -( 2.0 * M_PI - er ); //Clockwise
            counterclock = false;
        }
    }
    else
    {
        beginrad = 0;
        midrad = 0;
        endrad = 0;
    }

    return !straight;
}

/* utilities used by spline drawing routines */

// splining stack.
typedef struct gds_spline_stack_struct
{
    double           x1, y1, x2, y2, x3, y3, x4, y4;
}
Stack;

#define         SPLINE_STACK_DEPTH             20
static Stack    gds_spline_stack[SPLINE_STACK_DEPTH];
static Stack*   gds_stack_top;
static int      gds_stack_count;

static void gds_clear_stack()
{
    gds_stack_top = gds_spline_stack;
    gds_stack_count = 0;
}

static void gds_spline_push( double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4 )
{
    gds_stack_top->x1 = x1;
    gds_stack_top->y1 = y1;
    gds_stack_top->x2 = x2;
    gds_stack_top->y2 = y2;
    gds_stack_top->x3 = x3;
    gds_stack_top->y3 = y3;
    gds_stack_top->x4 = x4;
    gds_stack_top->y4 = y4;
    gds_stack_top++;
    gds_stack_count++;
}

static int gds_spline_pop( double* x1, double* y1, double* x2, double* y2,
                           double* x3, double* y3, double* x4, double* y4 )
{
    if ( gds_stack_count == 0 )
        return ( 0 );
    gds_stack_top--;
    gds_stack_count--;
    *x1 = gds_stack_top->x1;
    *y1 = gds_stack_top->y1;
    *x2 = gds_stack_top->x2;
    *y2 = gds_stack_top->y2;
    *x3 = gds_stack_top->x3;
    *y3 = gds_stack_top->y3;
    *x4 = gds_stack_top->x4;
    *y4 = gds_stack_top->y4;
    return ( 1 );
}
