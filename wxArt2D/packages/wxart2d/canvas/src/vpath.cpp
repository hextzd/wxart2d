/*! \file canvas/src/vpath.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: vpath.cpp,v 1.66 2009/05/06 21:33:23 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/vpath.h"
#include "wx/canvas/drawer.h"

IMPLEMENT_DYNAMIC_CLASS( a2dVectorPath, a2dCanvasObject )

//----------------------------------------------------------------------------
// a2dVectorPath
//----------------------------------------------------------------------------
a2dVectorPath::a2dVectorPath()
    : a2dCanvasObject()
{
    m_contourwidth = 0;
    m_datatype = 0;
    m_pathtype = a2dPATH_END_SQAURE;
    m_segments = new a2dVpath();
}

a2dVectorPath::a2dVectorPath( a2dVpath* path )
    : a2dCanvasObject()
{
    m_contourwidth = 0;
    m_datatype = 0;
    m_pathtype = a2dPATH_END_SQAURE;
    m_segments = path;
}

a2dVectorPath::~a2dVectorPath()
{
    delete m_segments;
}

a2dVectorPath::a2dVectorPath( const a2dVectorPath& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_segments = new a2dVpath();
    *m_segments = *other.m_segments;
    m_contourwidth = other.m_contourwidth;
    m_datatype = other.m_datatype;
    m_pathtype = other.m_pathtype;
}

a2dObject* a2dVectorPath::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dVectorPath( *this, options, refs );
};

a2dCanvasObjectList* a2dVectorPath::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    a2dVectorPath* copy = wxStaticCast( this->Clone( clone_deep ), a2dVectorPath );
    copy->m_segments->Transform( pworld );

    canpathlist->push_back( copy );

    return canpathlist;
}

bool a2dVectorPath::RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld )
{
    bool res = false;
    if ( m_segments && snapToWhat & a2dRestrictionEngine::snapToObjectVertexes )
    {
        unsigned int i;
        for ( i = 0; i < m_segments->size(); i++ )
        {
            a2dVpathSegmentPtr seg = m_segments->Item( i );
            switch ( seg->GetType() )
            {
                case a2dPATHSEG_MOVETO:
                case a2dPATHSEG_LINETO:
                case a2dPATHSEG_LINETO_NOSTROKE:
                case a2dPATHSEG_CBCURVETO_NOSTROKE:
                case a2dPATHSEG_CBCURVETO:
                case a2dPATHSEG_QBCURVETO_NOSTROKE:
                case a2dPATHSEG_QBCURVETO:
                case a2dPATHSEG_ARCTO_NOSTROKE:
                case a2dPATHSEG_ARCTO:
                {
                    //if within the threshold, do snap to vertex position.
                    res  |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( seg->m_x1, seg->m_y1 ), bestPointSofar, thresHoldWorld );
                }
                break;
            }
        }
    }

    snapToWhat = snapToWhat & ( a2dRestrictionEngine::snapToAll ^ a2dRestrictionEngine::snapToObjectVertexes ); //disable this for base now.

    return res || a2dCanvasObject::RestrictToObject( ic, pointToSnapTo, bestPointSofar, snapToWhat, thresHoldWorld );
}

a2dCanvasObjectList* a2dVectorPath::GetAsPolygons()
{
    a2dCanvasObjectList* ret = new a2dCanvasObjectList();

    double tstep = 1 / ( double ) SPLINE_STEP;
    unsigned int i;
    double x, y, xm, ym;
    bool move = false;
    int count = 0;
    bool nostrokeparts = false;

    if ( m_lworld.IsTranslate() && m_segments->IsPolygon( true ) )
    {
        a2dVertexListPtr lpoints = new a2dVertexList();

        for ( i = 0; i < m_segments->size(); i++ )
        {
            a2dVpathSegment* seg = Item( i );
            a2dLineSegment* polyseg;
            switch ( seg->GetType()  )
            {
                case a2dPATHSEG_MOVETO:
                    m_lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    polyseg = new a2dLineSegment( x, y );
                    break;
                case a2dPATHSEG_LINETO:
                    m_lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    polyseg = new a2dLineSegment( x, y );
                    break;
                case a2dPATHSEG_ARCTO:
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg;
                    m_lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    m_lworld.TransformPoint( cseg->m_x2, cseg->m_y2, xm, ym );
                    polyseg = new a2dArcSegment( x, y, xm, ym );
                    break;
            }
            polyseg->SetArcPiece( seg->GetArcPiece() );
            lpoints->push_back( polyseg );
            /* no need to add this, since a polygon is closed by itself.
            if ( seg->GetClose() == a2dPATHSEG_END_CLOSED )
            {
                lpoints->push_back( new a2dLineSegment( Item( 0 )->m_x1, Item( 0 )->m_y1 ) );
            }
            */
        }

        a2dPolygonL* poly = new a2dPolygonL( lpoints );
        poly->SetContourWidth( m_contourwidth );
        ret->push_back( poly );
        return ret;
    }

    if ( m_lworld.IsTranslate() && m_segments->IsPolyline( true ) )
    {
        a2dVertexListPtr lpoints = new a2dVertexList();

        for ( i = 0; i < m_segments->size(); i++ )
        {
            a2dVpathSegmentPtr seg = Item( i );
            a2dLineSegment* polyseg;
            switch ( seg->GetType()  )
            {
                case a2dPATHSEG_MOVETO:
                    m_lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    polyseg = new a2dLineSegment( x, y );
                    break;
                case a2dPATHSEG_LINETO:
                    m_lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    polyseg = new a2dLineSegment( x, y );
                    break;
                case a2dPATHSEG_ARCTO:
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();
                    m_lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    m_lworld.TransformPoint( cseg->m_x2, cseg->m_y2, xm, ym );
                    polyseg = new a2dArcSegment( x, y, xm, ym );
                    break;
            }
            polyseg->SetArcPiece( seg->GetArcPiece() );
            lpoints->push_back( polyseg );
            if ( seg->GetClose() == a2dPATHSEG_END_CLOSED )
            {
                lpoints->push_back( new a2dLineSegment( Item( 0 )->m_x1, Item( 0 )->m_y1 ) );
            }
        }

        a2dPolylineL* polyl = new a2dPolylineL( lpoints );
        polyl->SetContourWidth( m_contourwidth );
        polyl->SetPathType( m_pathtype );
        ret->push_back( new a2dPolylineL( lpoints ) );
        return ret;
    }

    a2dVertexListPtr lpoints = new a2dVertexList();
    //first draw as much as possible ( nostroke parts may stop this first round )
    for ( i = 0; i < m_segments->size(); i++ )
    {
        a2dVpathSegmentPtr seg = m_segments->Item( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_MOVETO:
                if ( count == 0 )
                {
                    m_lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    lpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                }
                else
                {
                    i--;
                    move = true;
                }
                break;

            case a2dPATHSEG_LINETO_NOSTROKE:
                nostrokeparts = true;
                break;
            case a2dPATHSEG_LINETO:
                m_lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                lpoints->push_back( new a2dLineSegment( x, y ) );
                count++;
                break;

            case a2dPATHSEG_CBCURVETO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_CBCURVETO:
            {
                double xw, yw;

                double xwl = m_segments->Item( i ? i - 1 : 0 )->m_x1;
                double ywl = m_segments->Item( i ? i - 1 : 0 )->m_y1;
                a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                    yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
                    m_lworld.TransformPoint( xw, yw, x, y );
                    lpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_QBCURVETO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_QBCURVETO:
            {
                double xw, yw;

                double xwl = m_segments->Item( i ? i - 1 : 0 )->m_x1;
                double ywl = m_segments->Item( i ? i - 1 : 0 )->m_y1;
                a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                    yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
                    m_lworld.TransformPoint( xw, yw, x, y );
                    lpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_ARCTO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_ARCTO:
            {
                a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();

                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                if ( cseg->CalcR( m_segments->Item( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                {
                    double dphi;
                    unsigned int segments = 20;
                    a2dGlobals->Aberration( phit, radius , dphi, segments );

                    double theta = beginrad;
                    unsigned int step;

                    double x, y;
                    for ( step = 0; step < segments + 1; step++ )
                    {
                        m_lworld.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                        lpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                        theta = theta + dphi;
                    }
                }
                else
                {
                    double x, y;
                    m_lworld.TransformPoint( cseg->m_x1, cseg->m_y1, x, y );
                    lpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                }
            }
            break;
        }

        if ( move )
        {
            a2dPolylineL* polyl = new a2dPolylineL( lpoints );
            polyl->SetContourWidth( m_contourwidth );
            polyl->SetPathType( m_pathtype );
            ret->push_back( polyl );
            move = false;
            lpoints = new a2dVertexList;
            count = 0;
        }
        else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
        {
            if ( nostrokeparts || seg->GetClose() == a2dPATHSEG_END_CLOSED_NOSTROKE )
            {
                a2dPolygonL* poly = new a2dPolygonL( lpoints );
                poly->SetContourWidth( m_contourwidth );
                ret->push_back( poly );
                nostrokeparts = true;
            }
            else
            {
                a2dPolygonL* poly = new a2dPolygonL( lpoints );
                poly->SetContourWidth( m_contourwidth );
                ret->push_back( poly );
            }

            move = false;
            lpoints = new a2dVertexList;
            count = 0;
        }
        else if ( i == m_segments->size() - 1 ) //last segment?
        {
            a2dPolylineL* polyl = new a2dPolylineL( lpoints );
            polyl->SetContourWidth( m_contourwidth );
            polyl->SetPathType( m_pathtype );
            ret->push_back( polyl );
        }
    }

    if ( nostrokeparts )
    {
        move = false;
        count = 0;
        a2dVertexList* lpoints = new a2dVertexList();

        nostrokeparts = false;

        double lastmovex = 0;
        double lastmovey = 0;

        for ( i = 0; i < m_segments->size(); i++ )
        {
            a2dVpathSegmentPtr seg = m_segments->Item( i );
            switch ( seg->GetType()  )
            {
                case a2dPATHSEG_MOVETO:
                    if ( count == 0 )
                    {
                        m_lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                        lpoints->push_back( new a2dLineSegment( x, y ) );
                        lastmovex = x;
                        lastmovey = y;
                        count++;
                    }
                    else
                    {
                        i--;
                        move = true;
                    }
                    break;

                case a2dPATHSEG_LINETO:
                    m_lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    lpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                    break;

                case a2dPATHSEG_LINETO_NOSTROKE:
                case a2dPATHSEG_CBCURVETO_NOSTROKE:
                case a2dPATHSEG_QBCURVETO_NOSTROKE:
                case a2dPATHSEG_ARCTO_NOSTROKE:
                    if ( count == 0 )
                    {
                        m_lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                        lpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                    }
                    else
                    {
                        i--;
                        nostrokeparts = true;
                    }
                    break;

                case a2dPATHSEG_CBCURVETO:
                {
                    double xw, yw;

                    double xwl = m_segments->Item( i ? i - 1 : 0 )->m_x1;
                    double ywl = m_segments->Item( i ? i - 1 : 0 )->m_y1;
                    a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                        yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
                        m_lworld.TransformPoint( xw, yw, x, y );
                        lpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                        t = t + tstep;
                    }
                }
                break;

                case a2dPATHSEG_QBCURVETO:
                {
                    double xw, yw;

                    double xwl = m_segments->Item( i ? i - 1 : 0 )->m_x1;
                    double ywl = m_segments->Item( i ? i - 1 : 0 )->m_y1;
                    a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                        yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
                        m_lworld.TransformPoint( xw, yw, x, y );
                        lpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                        t = t + tstep;
                    }
                }
                break;

                case a2dPATHSEG_ARCTO:
                {
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();

                    double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                    if ( cseg->CalcR( m_segments->Item( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                    {
                        double dphi;
                        unsigned int segments = 20;
                        a2dGlobals->Aberration( phit, radius , dphi, segments );

                        double theta = beginrad;
                        unsigned int step;

                        double x, y;
                        for ( step = 0; step < segments + 1; step++ )
                        {
                            m_lworld.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                            lpoints->push_back( new a2dLineSegment( x, y ) );
                            count++;
                            theta = theta + dphi;
                        }
                    }
                    else
                    {
                        double x, y;
                        m_lworld.TransformPoint( cseg->m_x1, cseg->m_y1, x, y );
                        lpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                    }
                }
                break;
            }

            if ( move || nostrokeparts )
            {
                a2dPolylineL* polyl = new a2dPolylineL( lpoints );
                polyl->SetContourWidth( m_contourwidth );
                polyl->SetPathType( m_pathtype );
                ret->push_back( polyl );
                move = false;
                nostrokeparts = false;
                lpoints = new a2dVertexList();
                count = 0;
            }
            else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
            {
                if ( seg->GetClose() == a2dPATHSEG_END_CLOSED )
                {
                    lpoints->push_back( new a2dLineSegment( lastmovex, lastmovey ) );
                    count++;
                }
                a2dPolylineL* polyl = new a2dPolylineL( lpoints );
                polyl->SetContourWidth( m_contourwidth );
                polyl->SetPathType( m_pathtype );
                ret->push_back( polyl );
                nostrokeparts = false;
                move = false;
                lpoints = new a2dVertexList;
                count = 0;
            }
            else if ( i == m_segments->size() )
            {
                a2dPolylineL* polyl = new a2dPolylineL( lpoints );
                polyl->SetContourWidth( m_contourwidth );
                polyl->SetPathType( m_pathtype );
                ret->push_back( polyl );
            }
        }
    }
    return ret;
}

void a2dVectorPath::ConvertToLines()
{
    double AberArcToPoly = m_root->GetAberArcToPolyScaled();
    m_segments->ConvertToLines(AberArcToPoly);
}

bool a2dVectorPath::EliminateMatrix()
{
    if ( !m_lworld.IsIdentity() )
    {
        double AberArcToPoly = m_root->GetAberArcToPolyScaled();
        m_segments->ConvertToLines( AberArcToPoly );
        unsigned int i;
        for ( i = 0; i < m_segments->size() ; i++ )
        {
            m_lworld.TransformPoint( m_segments->Item( i )->m_x1, m_segments->Item( i )->m_y1,
                                     m_segments->Item( i )->m_x1, m_segments->Item( i )->m_y1 );
        }

    }
    return a2dCanvasObject::EliminateMatrix();
}

a2dBoundingBox a2dVectorPath::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    unsigned int i;
    for ( i = 0; i < m_segments->size(); i++ )
    {
        a2dVpathSegmentPtr seg = m_segments->Item( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_CBCURVETO:
            case a2dPATHSEG_CBCURVETO_NOSTROKE:
            {
                a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();
                bbox.Expand( cseg->m_x2, cseg->m_y2 );
                bbox.Expand( cseg->m_x3, cseg->m_y3 );
            }
            break;

            case a2dPATHSEG_QBCURVETO:
            case a2dPATHSEG_QBCURVETO_NOSTROKE:
            {
                a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();
                bbox.Expand( cseg->m_x2, cseg->m_y2 );
            }
            break;

            case a2dPATHSEG_ARCTO:
            case a2dPATHSEG_ARCTO_NOSTROKE:
            {
                a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();

                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                //TODO soemthing to get the actual boundingbox of the arc
                if ( cseg->CalcR( m_segments->Item( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                {
                    bbox.Expand( center_x + radius , center_y + radius );
                    bbox.Expand( center_x + radius , center_y - radius );
                    bbox.Expand( center_x - radius , center_y + radius );
                    bbox.Expand( center_x - radius , center_y - radius );
                }

                bbox.Expand( cseg->m_x1, cseg->m_y1 );
                bbox.Expand( cseg->m_x2, cseg->m_y2 );
            }
            break;
            default:
                break;
        }
        bbox.Expand( m_segments->Item( i )->m_x1, m_segments->Item( i )->m_y1 );
    }
    bbox.Enlarge( m_contourwidth );
    return bbox;
}

bool a2dVectorPath::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    //bezier curves stays within convex hull of polygon formed by control points
    if ( !m_bbox.GetValid() )
    {
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;

}

void a2dVectorPath::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    /* conversion test
        a2dVpath* segments = new a2dVpath();
        *segments = *m_segments;
        double AberArcToPoly = m_root->GetAberArcToPolyScaled();
        segments->ConvertToLines( AberArcToPoly );
        DRAWER->DrawVpath( segments );
        delete segments;
    */

    //a2dBoundingBox bbox = DoGetUnTransformedBbox();
    //DRAWER->DrawRoundedRectangle( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth() ,bbox.GetHeight(), 0);

    if ( m_contourwidth )
    {
        a2dVpath* segments = new a2dVpath();
        *segments = *m_segments;
        segments->Contour( m_contourwidth / 2, m_pathtype );
        ic.GetDrawer2D()->DrawVpath( segments );
        delete segments;
    }
    else
        ic.GetDrawer2D()->DrawVpath( m_segments );
}

#if wxART2D_USE_CVGIO

void a2dVectorPath::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "segments" ), m_segments->size() );
    }
    else
    {

        unsigned int i;
        for ( i = 0; i < m_segments->size(); i++ )
        {
            if ( i % 2 == 0 )
                out.WriteNewLine();
            a2dVpathSegmentPtr seg = m_segments->Item( i );

            out.WriteStartElementAttributes( wxT( "seg" ) );
            switch ( seg->m_type )
            {
                case a2dPATHSEG_MOVETO:
                {
                    out.WriteAttribute( wxT( "type" ), wxT( "moveto" ) );
                    out.WriteAttribute( wxT( "x1" ), seg->m_x1 );
                    out.WriteAttribute( wxT( "y1" ), seg->m_y1 );
                    break;
                }
                case a2dPATHSEG_LINETO:
                {
                    out.WriteAttribute( wxT( "type" ), wxT( "lineto" ) );
                    out.WriteAttribute( wxT( "x1" ), seg->m_x1 );
                    out.WriteAttribute( wxT( "y1" ), seg->m_y1 );
                    break;
                }
                case a2dPATHSEG_LINETO_NOSTROKE:
                {
                    out.WriteAttribute( wxT( "type" ), wxT( "lineto_nostroke" ) );
                    out.WriteAttribute( wxT( "x1" ), seg->m_x1 );
                    out.WriteAttribute( wxT( "y1" ), seg->m_y1 );
                    break;
                }

                case a2dPATHSEG_CBCURVETO:
                {
                    out.WriteAttribute( wxT( "type" ), wxT( "cb_curveto" ) );
                    out.WriteAttribute( wxT( "x1" ), seg->m_x1 );
                    out.WriteAttribute( wxT( "y1" ), seg->m_y1 );
                    a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();
                    out.WriteAttribute( wxT( "x2" ), cseg->m_x2 );
                    out.WriteAttribute( wxT( "y2" ), cseg->m_y2 );
                    out.WriteAttribute( wxT( "x3" ), cseg->m_x3 );
                    out.WriteAttribute( wxT( "y3" ), cseg->m_y3 );
                    break;
                }
                case a2dPATHSEG_CBCURVETO_NOSTROKE:
                {
                    out.WriteAttribute( wxT( "type" ), wxT( "cb_curveto_nostroke" ) );
                    out.WriteAttribute( wxT( "x1" ), seg->m_x1 );
                    out.WriteAttribute( wxT( "y1" ), seg->m_y1 );
                    a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();
                    out.WriteAttribute( wxT( "x2" ), cseg->m_x2 );
                    out.WriteAttribute( wxT( "y2" ), cseg->m_y2 );
                    out.WriteAttribute( wxT( "x3" ), cseg->m_x3 );
                    out.WriteAttribute( wxT( "y3" ), cseg->m_y3 );
                    break;
                }

                case a2dPATHSEG_QBCURVETO:
                {
                    out.WriteAttribute( wxT( "type" ), wxT( "qb_curveto" ) );
                    out.WriteAttribute( wxT( "x1" ), seg->m_x1 );
                    out.WriteAttribute( wxT( "y1" ), seg->m_y1 );
                    a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();
                    out.WriteAttribute( wxT( "x2" ), cseg->m_x2 );
                    out.WriteAttribute( wxT( "y2" ), cseg->m_y2 );
                    break;
                }
                case a2dPATHSEG_QBCURVETO_NOSTROKE:
                {
                    out.WriteAttribute( wxT( "type" ), wxT( "qb_curveto_nostroke" ) );
                    out.WriteAttribute( wxT( "x1" ), seg->m_x1 );
                    out.WriteAttribute( wxT( "y1" ), seg->m_y1 );
                    a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();
                    out.WriteAttribute( wxT( "x2" ), cseg->m_x2 );
                    out.WriteAttribute( wxT( "y2" ), cseg->m_y2 );
                    break;
                }

                case a2dPATHSEG_ARCTO:
                {
                    out.WriteAttribute( wxT( "type" ), wxT( "arcto" ) );
                    out.WriteAttribute( wxT( "x1" ), seg->m_x1 );
                    out.WriteAttribute( wxT( "y1" ), seg->m_y1 );
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();
                    out.WriteAttribute( wxT( "x2" ), cseg->m_x2 );
                    out.WriteAttribute( wxT( "y2" ), cseg->m_y2 );
                    break;
                }
                case a2dPATHSEG_ARCTO_NOSTROKE:
                {
                    out.WriteAttribute( wxT( "type" ), wxT( "arcto_nostroke" ) );
                    out.WriteAttribute( wxT( "x1" ), seg->m_x1 );
                    out.WriteAttribute( wxT( "y1" ), seg->m_y1 );
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();
                    out.WriteAttribute( wxT( "x2" ), cseg->m_x2 );
                    out.WriteAttribute( wxT( "y2" ), cseg->m_y2 );
                    break;
                }
                default:
                    break;
            }

            switch ( seg->m_close )
            {
                case a2dPATHSEG_END_OPEN:
                {
                    out.WriteAttribute( wxT( "end" ), wxT( "open" ) );
                    break;
                }
                case a2dPATHSEG_END_CLOSED:
                {
                    out.WriteAttribute( wxT( "end" ), wxT( "closed" ) );
                    break;
                }
                case a2dPATHSEG_END_CLOSED_NOSTROKE:
                {
                    out.WriteAttribute( wxT( "end" ), wxT( "closed_nostroke" ) );
                    break;
                }
            }
            out.WriteEndAttributes( true );
        }
    }
}

void a2dVectorPath::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        while( parser.GetTagName() == wxT( "seg" )  )
        {

            a2dVpathSegment* seg = NULL;

            wxString s1, s2;
            double x1 = parser.GetAttributeValueDouble( wxT( "x1" ) );
            double y1 = parser.GetAttributeValueDouble( wxT( "y1" ) );
            double x2 = parser.GetAttributeValueDouble( wxT( "x2" ) ); //used when available
            double y2 = parser.GetAttributeValueDouble( wxT( "y2" ) ); //used when available
            double x3 = parser.GetAttributeValueDouble( wxT( "x3" ) ); //used when available
            double y3 = parser.GetAttributeValueDouble( wxT( "y3" ) ); //used when available

            a2dPATHSEG_END end = a2dPATHSEG_END_OPEN;
            wxString send = parser.GetAttributeValue( wxT( "end" ) );
            if ( send == wxT( "open" ) )
                end = a2dPATHSEG_END_OPEN;
            else if ( send == wxT( "closed" ) )
                end = a2dPATHSEG_END_CLOSED;
            else if ( send == wxT( "closed_nostroke" ) )
                end = a2dPATHSEG_END_CLOSED_NOSTROKE;

            wxString type = parser.GetAttributeValue( wxT( "type" ) );

            if ( type == wxT( "moveto" ) )
            {
                seg = new a2dVpathSegment( x1, y1 , a2dPATHSEG_MOVETO, end );
            }
            else if ( type == wxT( "lineto" ) )
            {
                seg = new a2dVpathSegment( x1, y1 , a2dPATHSEG_LINETO, end );
            }
            else if ( type == wxT( "lineto_nostroke" ) )
            {
                seg = new a2dVpathSegment( x1, y1 , a2dPATHSEG_LINETO_NOSTROKE, end );
            }

            else if ( type == wxT( "cb_curveto" ) )
            {
                seg = new a2dVpathCBCurveSegment( x1, y1 , x2, y2, x3, y3, a2dPATHSEG_CBCURVETO, end  );
            }
            else if ( type == wxT( "cb_curveto_nostroke" ) )
            {
                seg = new a2dVpathCBCurveSegment( x1, y1 , x2, y2, x3, y3, a2dPATHSEG_CBCURVETO_NOSTROKE, end  );
            }

            else if ( type == wxT( "qb_curveto" ) )
            {
                seg = new a2dVpathQBCurveSegment( x1, y1 , x2, y2, a2dPATHSEG_QBCURVETO, end  );
            }
            else if ( type == wxT( "qb_curveto_nostroke" ) )
            {
                seg = new a2dVpathQBCurveSegment( x1, y1 , x2, y2, a2dPATHSEG_QBCURVETO_NOSTROKE, end  );
            }

            else if ( type == wxT( "arcto" ) )
            {
                seg = new a2dVpathArcSegment( x1, y1 , x2, y2, a2dPATHSEG_ARCTO, end  );
            }
            else if ( type == wxT( "arcto_nostroke" ) )
            {
                seg = new a2dVpathArcSegment( x1, y1 , x2, y2, a2dPATHSEG_ARCTO_NOSTROKE, end  );
            }

            m_segments->push_back( seg );

            parser.Next();
            parser.Require( END_TAG, wxT( "seg" ) );
            parser.Next();
        }
    }
}
#endif //wxART2D_USE_CVGIO


bool a2dVectorPath::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    a2dPoint2D P = a2dPoint2D( hitEvent.m_relx, hitEvent.m_rely );

    //TODO
    //double pw = ic.GetTransformedHitMargin();
    //how = PointInPolygon(P, pw/2+margin);

    hitEvent.m_how = a2dHit::stock_fill;
    return hitEvent.m_how.IsHit();
}



