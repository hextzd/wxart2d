/*! \file canvas/src/polygon.cpp
   \author Klaas Holwerda

   Copyright: 2000-2004 (c) Klaas Holwerda

   Licence: wxWidgets Licence

   RCS-ID: $Id: polygon.cpp,v 1.209 2009/07/24 16:35:01 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/polygon.h"

#include <float.h>
#include "wx/canvas/canobj.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/tools.h"
#include "wx/canvas/canglob.h"

#if wxART2D_USE_KBOOL
#include "kbool/booleng.h"
#endif

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif


IMPLEMENT_DYNAMIC_CLASS( a2dPolyHandleL, a2dHandle )

IMPLEMENT_DYNAMIC_CLASS( a2dPolygonL, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dPolylineL, a2dPolygonL )

IMPLEMENT_DYNAMIC_CLASS( a2dPolygonLClipper, a2dPolygonL )
IMPLEMENT_DYNAMIC_CLASS( a2dPolygonLClipper2, a2dPolygonL )

static inline double sqr( double x ) { return x * x; }

static double PointDistSqr( double x1, double y1, double x2, double y2 )
{
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqr( dx ) + sqr( dy );
}

//----------------------------------------------------------------------------
// a2dPolygonL
//----------------------------------------------------------------------------

#define a2dCrossAtVertexSize 2

bool a2dPolygonL::m_allowCrossAtVertex = false;

BEGIN_EVENT_TABLE( a2dPolygonL, a2dCanvasObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dPolygonL::OnCanvasObjectMouseEvent )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dPolygonL::OnHandleEvent )
    EVT_CANVASHANDLE_MOUSE_EVENT_ENTER( a2dPolygonL::OnHandleEventEnter )
    EVT_CANVASHANDLE_MOUSE_EVENT_LEAVE( a2dPolygonL::OnHandleEventLeave )
    EVT_CANVASOBJECT_ENTER_EVENT( a2dPolygonL::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT( a2dPolygonL::OnLeaveObject )
    EVT_CHAR( a2dPolygonL::OnChar )
END_EVENT_TABLE()

a2dPolygonL::a2dPolygonL()
    : 
	m_crossAtVertex( false ),
	m_spline( false ),
	m_contourwidth( 0 ),
	a2dCanvasObject()
{
    m_lsegments = new a2dVertexList();
}

a2dPolygonL::a2dPolygonL( a2dVertexListPtr points, bool spline  )
    : 
	m_spline( spline ),
	m_contourwidth( 0 ),
	m_crossAtVertex( false ),
	a2dCanvasObject()
{
    m_lsegments = points;
}

a2dPolygonL::~a2dPolygonL()
{
}

void a2dPolygonL::Clear()
{
    m_lsegments->clear();
}

void a2dPolygonL::SetPending( bool pending )
{
    a2dCanvasObject::SetPending( pending );
}


a2dPolygonL::a2dPolygonL( const a2dPolygonL& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_lsegments = new a2dVertexList();
    *m_lsegments = *other.m_lsegments;
    m_spline = other.m_spline;
    m_contourwidth = other.m_contourwidth;
	m_crossAtVertex = other.m_crossAtVertex;
}

a2dObject* a2dPolygonL::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dPolygonL( *this, options, refs );
}

void a2dPolygonL::SetContourWidth( double width ) 
{  
    m_contourwidth = width; 
    SetFilled( true ); 
    SetPending( true ); 
}

a2dSurface* a2dPolygonL::GetAsSurface() const
{
#if wxART2D_USE_KBOOL
    kbBool_Engine booleng;

    double marge = m_root->GetHabitat()->GetBooleanEngineMarge();
    booleng.SetMarge( marge / m_root->GetUnitsScale() );
    booleng.SetGrid( m_root->GetHabitat()->GetBooleanEngineGrid() );
    booleng.SetDGrid( m_root->GetHabitat()->GetBooleanEngineDGrid() );
    booleng.SetMaxlinemerge( double( m_root->GetHabitat()->GetBooleanEngineMaxlinemerge() ) / m_root->GetUnitsScale() );
    booleng.SetWindingRule( m_root->GetHabitat()->GetBooleanEngineWindingRule() );
    booleng.SetLinkHoles( false );

    a2dPolygonL* poly = const_cast<a2dPolygonL*>(this);
    if ( poly && poly->GetSegments()->HasArcs() )
    {
        a2dVertexList* points = new a2dVertexList( *poly->GetSegments() );
        poly = new a2dPolygonL( points );
        double m_AberArcToPoly = 0;
        poly->GetSegments()->ConvertToLines( m_AberArcToPoly );
    }

    if ( poly && poly->GetNumberOfSegments() > 2 && booleng.StartPolygonAdd(  GROUP_A ) )
    {
        for( a2dVertexList::const_iterator iter = poly->GetSegments()->begin(); iter != poly->GetSegments()->end(); ++iter )
        {
            a2dLineSegment* seg = *iter;
            booleng.AddPoint( seg->m_x, seg->m_y, seg->GetArcPiece() );
        }
        booleng.EndPolygonAdd();
    }

    if ( poly != this )
        delete poly;

    booleng.Do_Operation( BOOL_OR );

    bool hole = false;
    bool normal = false;
    a2dSurface* surface = NULL;
    a2dVertexListPtr points;
    a2dVertexListPtr holepoints;
    while ( booleng.StartPolygonGet() )
    {
        hole = false;
        normal = false;
        // foreach point in the polygon
        while ( booleng.PolygonHasMorePoints() )
        {
            if ( booleng.GetHoleConnectionSegment() )
            {
                wxFAIL_MSG( wxT( "this type should not be here" ) );
            }
            else if ( booleng.GetHoleSegment() )
            {
                if ( !hole )
                {
                    hole = true;
                    holepoints = new a2dVertexList;
                }
                a2dLineSegment* seg = new a2dLineSegment( booleng.GetPolygonXPoint(), booleng.GetPolygonYPoint() );
                seg->SetSegType( a2dHOLE_SEG );
                holepoints->push_back( seg );
            }
            else //normal
            {
                if ( !normal )
                {
                    normal = true;
                    points = new a2dVertexList;
                }
                a2dLineSegment* seg = new a2dLineSegment( booleng.GetPolygonXPoint(), booleng.GetPolygonYPoint() );
                points->push_back( seg );
            }
        }
        booleng.EndPolygonGet();
        if ( normal )
        {
            surface = new a2dSurface( points );
            surface->SetLayer( m_layer );
        }
        if ( hole )
        {
            wxASSERT_MSG( surface, wxT( "need surface first to place holes within" ) );
            surface->AddHole( holepoints );
        }
    }

    return surface;
#else
    return NULL;
#endif
}

a2dVertexList* a2dPolygonL::GetAsVertexList( bool& returnIsPolygon ) const
{
    if ( m_contourwidth )
    {
        a2dVertexList* contourpoints = m_lsegments->ConvertToContour(  m_contourwidth / 2, a2dPATH_END_ROUND );
        if ( !returnIsPolygon )
            contourpoints->push_back( contourpoints->front()->Clone() );
        contourpoints->Transform( m_lworld );
        return contourpoints;
    }
    else
    {
        a2dVertexList* contourpoints = new a2dVertexList( *(m_lsegments.Get()) );
        contourpoints->Transform( m_lworld );
        if ( !returnIsPolygon )
            contourpoints->push_back( contourpoints->front()->Clone() );
        return contourpoints;
    }
}

a2dCanvasObjectList* a2dPolygonL::GetAsCanvasVpaths( bool transform ) const
{
    a2dVpath* segments = m_lsegments->ConvertToVpath( true, true );
    a2dVectorPath* canpath = new a2dVectorPath( segments );
    if ( transform )
        segments->Transform( m_lworld );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );
    canpath->SetContourWidth( GetContourWidth() );
    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}

a2dCanvasObjectList* a2dPolygonL::GetSnapVpath( a2dSnapToWhatMask snapToWhat )
{
    a2dVpath* segments = new a2dVpath();
    a2dVectorPath* snappath = new a2dVectorPath( segments );
    a2dCanvasObjectList* snappathlist = new a2dCanvasObjectList();
    snappathlist->push_back( snappath );

    if ( snapToWhat & a2dRestrictionEngine::snapToPins ||
         snapToWhat & a2dRestrictionEngine::snapToPinsUnconnected
       )
    {
        if ( m_childobjects != wxNullCanvasObjectList && m_flags.m_hasPins )
        {
            forEachIn( a2dCanvasObjectList, m_childobjects )
            {
                a2dCanvasObject* obj = *iter;
                a2dPin* pin = wxDynamicCast( obj, a2dPin );
                if ( pin && !pin->GetRelease( ) )
                {
                    if ( snapToWhat & a2dRestrictionEngine::snapToPins ||
                            ( snapToWhat & a2dRestrictionEngine::snapToPinsUnconnected  && !pin->IsConnectedTo() )
                       )
                    {
                        a2dVpathSegment* seg = new a2dVpathSegment( pin->GetAbsX(), pin->GetAbsY(), a2dPATHSEG_MOVETO );
                        segments->Add( seg );
                    }
                }
            }
        }
    }

    if ( snapToWhat & a2dRestrictionEngine::snapToObjectPos )
    {
        a2dVpathSegment* seg = new a2dVpathSegment( GetPosX(), GetPosY(), a2dPATHSEG_MOVETO );
        segments->Add( seg );
    }

    if ( snapToWhat & a2dRestrictionEngine::snapToObjectVertexes )
    {
        a2dVpath* segments = m_lsegments->ConvertToVpath( true, true );
        a2dVectorPath* snappathvertex = new a2dVectorPath( segments );
        segments->Transform( m_lworld );
        snappathlist->push_back( snappathvertex );
    }

    return snappathlist;
}

a2dCanvasObjectList* a2dPolygonL::GetAsLinesArcs( bool transform )
{
    a2dCanvasObjectList* linesAnArcs = new a2dCanvasObjectList();

    double xp = m_lsegments->back()->m_x;
    double yp = m_lsegments->back()->m_y;
    if ( transform )
        m_lworld.TransformPoint( xp, yp, xp, yp );
    a2dCanvasObject* obj;
    for( a2dVertexList::iterator iter = m_lsegments->begin(); iter != m_lsegments->end(); ++iter )
    {
        double x, y, xm, ym;
        a2dLineSegment* seg = *iter;
        x = seg->m_x; y = seg->m_y;
        if ( transform )
            m_lworld.TransformPoint( x, y, x, y );
        if ( seg->GetArc() )
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
            xm = cseg->m_x2; ym = cseg->m_y2;
            if ( transform )
                m_lworld.TransformPoint( xm, ym, xm, ym );
            a2dArc* arc = new a2dArc();
            arc->SetChord( true );
            obj = arc;
            arc->Set( xp, yp, xm, ym, x, y );
            linesAnArcs->push_back( arc );
        }
        else
        {
            a2dSLine* line = new a2dSLine( xp, yp, x, y );
            obj = line;
            linesAnArcs->push_back( line );
        }

        obj->SetStroke( this->GetStroke() );
        obj->SetFill( this->GetFill() );
        obj->SetLayer( m_layer );
        obj->SetContourWidth( GetContourWidth() );
        obj->SetRoot( m_root, false );

        xp = x;
        yp = y;
    }

    return linesAnArcs;
}

bool a2dPolygonL::RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld )
{
    // object has disabled snap?
    if ( !m_flags.m_snap_to )
        return false;

    double StrokeExtend = 0;
    a2dStroke stroke = GetStroke();

    if ( stroke.IsNoStroke() ) //IsNoStroke() stroke means use layer
    {
        if ( m_root && m_root->GetLayerSetup() )
        {
            a2dStroke layerpen = m_root->GetLayerSetup()->GetStroke( m_layer );
            if ( layerpen.GetPixelStroke() )
                StrokeExtend = layerpen.GetWidth() / 2;
            else
                StrokeExtend = layerpen.GetWidth() / 2;
        }
    }
    else if ( !stroke.IsSameAs( *a2dTRANSPARENT_STROKE ) )
    {
        if ( stroke.GetPixelStroke() )
        {
            StrokeExtend = ic.GetDrawer2D()->DeviceToWorldXRel( stroke.GetWidth() / 2 );
        }
        else
            StrokeExtend = stroke.GetWidth() / 2;
    }
    ic.SetStrokeWorldExtend( StrokeExtend );

    bool res = false;
    if ( snapToWhat & a2dRestrictionEngine::snapToObjectVertexes ||
         snapToWhat & a2dRestrictionEngine::snapToObjectSegmentsMiddle ||
         snapToWhat & a2dRestrictionEngine::snapToObjectSegments ||
         snapToWhat & a2dRestrictionEngine::snapToObjectIntersection
       )
    {
        a2dVertexList::iterator iter = m_lsegments->begin();
        double lastx, lasty;
        if ( iter != m_lsegments->end() )
            m_lworld.TransformPoint( m_lsegments->back()->m_x, m_lsegments->back()->m_y, lastx, lasty );
        a2dLineSegment* seg;
        while ( iter != m_lsegments->end() )
        {
            seg = ( *iter );

            double x, y;
            m_lworld.TransformPoint( seg->m_x, seg->m_y, x, y );
            a2dPoint2D p2( x, y );

            if ( snapToWhat & a2dRestrictionEngine::snapToObjectIntersection )
            {
            }
            if ( snapToWhat & a2dRestrictionEngine::snapToObjectSegments )
            {
                a2dPoint2D p1( lastx, lasty );
                a2dLine line( p1, p2 );
                a2dPoint2D pBestPoint;
                double dist = line.PointDistance( pointToSnapTo, &pBestPoint );
                double pw = ic.GetWorldStrokeExtend();
                if( dist < thresHoldWorld + pw )
                    res |= SetPointIfCloser( pointToSnapTo, pBestPoint, bestPointSofar, thresHoldWorld + pw );
            }
            if ( snapToWhat & a2dRestrictionEngine::snapToObjectSegmentsMiddle )
            {
                res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( x + lastx / 2.0, y + lasty / 2.0 ), bestPointSofar, thresHoldWorld );
            }
            if ( snapToWhat & a2dRestrictionEngine::snapToObjectVertexes )
            {
            //if within the threshold, do snap to vertex position.
                res |= SetPointIfCloser( pointToSnapTo, p2, bestPointSofar, thresHoldWorld );
            }
            lastx = x; lasty = y;
            iter++;
        }
    }

    snapToWhat = snapToWhat & ( a2dRestrictionEngine::snapToAll ^ a2dRestrictionEngine::snapToObjectVertexes ); //disable this for base now.

    return res || a2dCanvasObject::RestrictToObject( ic, pointToSnapTo, bestPointSofar, snapToWhat, thresHoldWorld );
}

int a2dPolygonL::GetIndexSegment( a2dLineSegmentPtr seg )
{
    if ( seg.Get() == NULL )
    {
        if (  !m_lsegments->empty() )
            return m_lsegments->IndexOf( m_lsegments->back() );
        return -1;
    }
    else
    {
        return m_lsegments->IndexOf( seg );
    }
}

a2dVertexList::iterator a2dPolygonL::GetSegmentAtIndex( unsigned int index )
{
    return m_lsegments->item( index );
}

a2dVertexList* a2dPolygonL::GetSegmentListAtIndex( unsigned int index )
{
    if ( index < m_lsegments->size() + 1 )
    {
        return m_lsegments;
    }
    return NULL;
}


a2dLineSegment* a2dPolygonL::AddPoint( double x, double y, a2dLineSegmentPtr seg, bool afterinversion )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x, y, x, y );
    }

    a2dLineSegment* point = new a2dLineSegment( x, y );

    if ( seg.Get() == NULL )
        m_lsegments->push_back( point );
    else
    {
        a2dVertexList::iterator iter = m_lsegments->begin();
        while ( iter != m_lsegments->end() )
        {
            if ( ( *iter ) == seg )
            {
                m_lsegments->insert( iter, point );
                break;
            }
            ++iter;
        }
    }

    SetPending( true );
    return point;
}

a2dLineSegment* a2dPolygonL::AddPoint( const a2dPoint2D& P, int index, bool afterinversion )
{
    return AddPoint( P.m_x, P.m_y, index, afterinversion  );
}

a2dLineSegment* a2dPolygonL::AddPoint( double x, double y, int index, bool afterinversion  )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x, y, x, y );
    }

    a2dLineSegment* point = new a2dLineSegment( x, y );

    if ( index == -1 )
        m_lsegments->push_back( point );
    else
    {
        a2dVertexList::iterator iter = m_lsegments->begin();
        int i = 0;
        while ( i < index ) { ++iter; i++; }
        m_lsegments->insert( iter, point );
    }

    SetPending( true );
    return point;
}

void a2dPolygonL::RemoveRedundantPoints( a2dCanvasObject* sendCommandsTo )
{
    a2dVertexList::iterator iterp = m_lsegments->end();
    a2dVertexList::iterator iterpp = m_lsegments->end();
    a2dVertexList::iterator iter = m_lsegments->begin();

    int inode;

    // remove equal points
    iter = m_lsegments->begin();
    inode = 0;
    if( iter != m_lsegments->end() )
    {
        iterp = iter++;
        inode++;
    }
    while ( iter != m_lsegments->end() )
    {
        a2dLineSegment* line1 = *iterp;
        a2dLineSegment* line2 = *iter;
        if( line1->GetPoint() == line2->GetPoint() && !line1->GetArc() ) //this is arc save on line2
        {
            if( sendCommandsTo )
                m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( sendCommandsTo, inode ) );
            m_lsegments->erase( iterp );
        }
        else
        {
            inode++;
        }

        iterp = iter++;
    }

    //remove points on same x/y
    iter = m_lsegments->begin();
    inode = 0;
    if( iter != m_lsegments->end() )
    {
        iterp = iter++;
        inode++;
    }
    if( iter != m_lsegments->end() )
    {
        iterpp = iterp;
        iterp = iter++;
        inode++;
    }
    while ( iter != m_lsegments->end() )
    {
        a2dLineSegment* point0 = *iterpp;
        a2dLineSegment* point1 = *iterp;
        a2dLineSegment* point2 = *iter;

        if ( !point0->GetArc() && !point1->GetArc() && 
             ( ( point1->GetPoint().m_x == point0->GetPoint().m_x &&  point1->GetPoint().m_x == point2->GetPoint().m_x ) ||
               ( point1->GetPoint().m_y == point0->GetPoint().m_y &&  point1->GetPoint().m_y == point2->GetPoint().m_y )
             )
           ) 
        {
            if( sendCommandsTo )
                m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( sendCommandsTo, inode -1 ) );
            m_lsegments->erase( iterp );
        }
        else
        {
            inode++;
            iterpp = iterp;
        }
        iterp = iter++;
    }

    /*
    // remove 180 deg points
    iter = m_lsegments->begin();
    inode = 0;
    if( iter != m_lsegments->end() )
    {
        iterp = iter++;
        inode++;
    }
    if( iter != m_lsegments->end() )
    {
        iterpp = iterp;
        iterp = iter++;
        inode++;
    }
    while ( iter != m_lsegments->end() )
    {
        a2dLineSegment* point0 = *iterpp;
        a2dLineSegment* point1 = *iterp;
        a2dLineSegment* point2 = *iter;

        double dx10 = point1->m_x - point0->m_x;
        double dy10 = point1->m_y - point0->m_y;
        double dx21 = point2->m_x - point1->m_x;
        double dy21 = point2->m_y - point1->m_y;

        if(  !point0->GetArc() && !point1->GetArc() && dx10* dy21 - dx21* dy10 == 0 )
        {
            if( sendCommandsTo )
                m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( sendCommandsTo, inode - 1 ) );
            m_lsegments->erase( iterp );
        }
        else
        {
            inode++;
            iterpp = iterp;
        }
        iterp = iter++;
    }
    */
}

void a2dPolygonL::RemoveSegment( double& x, double& y , a2dLineSegmentPtr seg, bool transformed )
{
    if ( seg.Get() == NULL )
    {
        a2dLineSegment* point;
        point = m_lsegments->back();
        x = point->m_x;
        y = point->m_y;
        m_lsegments->pop_back();
    }
    else
    {
        a2dVertexList::iterator iter = m_lsegments->begin();
        a2dLineSegment* point;
        while ( iter != m_lsegments->end() )
        {
            if ( *iter == seg )
            {
                point = ( *iter );
                x = point->m_x;
                y = point->m_y;
                m_lsegments->erase( iter );
                break;
            }
            iter++;
        }
    }

    if ( transformed )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }

    SetPending( true );
}

void a2dPolygonL::RemoveSegment( double& x, double& y , int index, bool transformed )
{
    if ( index == -1 )
    {
        a2dLineSegment* point;
        point = m_lsegments->back();
        x = point->m_x;
        y = point->m_y;
        m_lsegments->pop_back();
    }
    else
    {
        int i = 0;
        a2dVertexList::iterator iter = m_lsegments->begin();
        a2dLineSegment* point;
        while ( iter != m_lsegments->end() )
        {
            if ( i == index )
            {
                point = ( *iter );
                x = point->m_x;
                y = point->m_y;
                m_lsegments->erase( iter );
                break;
            }
            i++;
            iter++;
        }
    }

    if ( transformed )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }

    SetPending( true );
}

void a2dPolygonL::GetPosXYSegment( int index, double& x, double& y, bool transform ) const
{
    if ( index == 0 )
    {
        a2dLineSegment* point;
        point = m_lsegments->front();
        x = point->m_x;
        y = point->m_y;
    }
    else if ( index == -1 || index == m_lsegments->size() - 1 )
    {
        a2dLineSegment* point;
        point = m_lsegments->back();
        x = point->m_x;
        y = point->m_y;
    }
    else
    {
        int i = 0;
        a2dVertexList::iterator iter = m_lsegments->begin();
        a2dLineSegment* point;
        while ( iter != m_lsegments->end() )
        {
            if ( i == index )
            {
                point = ( *iter );
                x = point->m_x;
                y = point->m_y;
                break;
            }
            i++;
            iter++;
        }
    }

    if ( transform )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }
}

void a2dPolygonL::GetPosXYSegment( a2dLineSegmentPtr seg, double& x, double& y, bool transform ) const
{
    if ( seg.Get() == NULL )
    {
        a2dLineSegment* point;
        point = m_lsegments->back();
        x = point->m_x;
        y = point->m_y;
    }
    else
    {
        x = seg->m_x;
        y = seg->m_y;
    }

    if ( transform )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }
}

void a2dPolygonL::GetPosXYMidSegment( int index, double& x, double& y, bool transform ) const
{
    if ( index == -1 )
    {
        a2dLineSegment* seg = m_lsegments->back();
        wxASSERT_MSG( seg->GetArc() , _T( "this is not an arc segment." ) );
        a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
        x = cseg->m_x2;
        y = cseg->m_y2;
    }
    else
    {
        int i = 0;
        a2dVertexList::iterator iter = m_lsegments->begin();
        a2dLineSegment* seg;
        while ( iter != m_lsegments->end() )
        {
            if ( i == index )
            {
                seg = ( *iter );
                wxASSERT_MSG( seg->GetArc() , _T( "this is not an arc segment." ) );
                a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
                x = cseg->m_x2;
                y = cseg->m_y2;
                break;
            }
            i++;
            iter++;
        }
    }

    if ( transform )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }
}

void a2dPolygonL::GetPosXYMidSegment( a2dLineSegmentPtr seg, double& x, double& y, bool transform ) const
{
    if ( seg.Get() == NULL )
    {
        a2dLineSegment* seg = m_lsegments->back();
        wxASSERT_MSG( seg->GetArc() , _T( "this is not an arc segment." ) );
        a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
        x = cseg->m_x2;
        y = cseg->m_y2;
    }
    else
    {
        wxASSERT_MSG( seg->GetArc() , _T( "this is not an arc segment." ) );
        a2dArcSegment* cseg = ( a2dArcSegment* ) seg.Get();
        x = cseg->m_x2;
        y = cseg->m_y2;
    }

    if ( transform )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }
}

void a2dPolygonL::SetPosXYSegment( int n, double x, double y, bool afterinversion )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x, y, x, y );
    }

    m_lsegments->SetPointAdjustArcs( n, x, y, true );
    SetPending( true );
}

void a2dPolygonL::SetPosXYSegment( a2dLineSegmentPtr seg, double x, double y, bool afterinversion )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x, y, x, y );
    }

    m_lsegments->SetPointAdjustArcs( seg, x, y, true );
    SetPending( true );
}

void a2dPolygonL::SetPosXYMidSegment( int index, double x, double y, bool afterinversion )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x, y, x, y );
    }

    a2dVertexList::iterator iterp = m_lsegments->end()--;
    a2dVertexList::iterator iter = m_lsegments->begin();
    a2dLineSegment* seg;
    int i = 0;
    while ( iter != m_lsegments->end() )
    {
        seg = ( *iter );
        if ( i == index )
        {
            wxASSERT_MSG( seg->GetArc() , _T( "this is not an arc segment." ) );
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
            cseg->SetMidPoint( *( *iterp ), x, y );
            break;
        }
        i++;
        iterp = iter;
        iter++;
    }
    SetPending( true );
}

void a2dPolygonL::SetPosXYMidSegment( a2dLineSegmentPtr seg, double x, double y, bool afterinversion )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x, y, x, y );
    }

    a2dVertexList::iterator iterp = m_lsegments->end()--;
    a2dVertexList::iterator iter = m_lsegments->begin();
    a2dLineSegment* segp;
    while ( iter != m_lsegments->end() )
    {
        segp = ( *iter );
        if ( segp == seg )
        {
            wxASSERT_MSG( segp->GetArc() , _T( "this is not an arc segment." ) );
            a2dArcSegment* cseg = ( a2dArcSegment* ) segp;
            cseg->SetMidPoint( *( *iterp ), x, y );
            break;
        }
        iterp = iter;
        iter++;
    }
    SetPending( true );
}

bool a2dPolygonL::EliminateMatrix()
{
    if ( !m_lworld.IsIdentity() )
    {
        double AberArcToPoly = m_root->GetAberArcToPolyScaled();
        m_lsegments->ConvertToLines( AberArcToPoly );
        a2dVertexList::iterator iter = m_lsegments->begin();
        //first include transform
        while ( iter != m_lsegments->end() )
        {
            a2dLineSegment* point = ( *iter );
            m_lworld.TransformPoint( point->m_x, point->m_y, point->m_x, point->m_y );
            iter++;
        }

    }
    return a2dCanvasObject::EliminateMatrix();
}

void a2dPolygonL::SetSegments( a2dVertexList* points )
{
    m_lsegments = points;
}

bool a2dPolygonL::AdjustAfterChange( bool WXUNUSED( final ) )
{
    return false;
}

void a2dPolygonL::SetHandleToIndex( a2dPolyHandleL* handle, unsigned int index )
{
    handle->SetNode( GetSegments()->item( index ) );
    a2dPolygonL* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dPolygonL );
    handle->SetNodeOrg( original->GetSegments()->item( index ) );
}

void a2dPolygonL::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    if ( m_allowCrossAtVertex && (!m_flags.m_editingCopy || !m_flags.m_editable) )
    {
		m_crossAtVertex = true;
		SetPending( true );
        return;
    }
    event.Skip();
}

void a2dPolygonL::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    if ( m_allowCrossAtVertex && (!m_flags.m_editingCopy || !m_flags.m_editable) )
    {
		m_crossAtVertex = false;
		SetPending( true );
        return;
    }
    event.Skip();
}

bool a2dPolygonL::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        a2dPolygonL* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dPolygonL );

        if ( editmode == 1 )
        {
            if( ! ( editstyle & wxEDITSTYLE_NOHANDLES ) )
            {
                m_flags.m_visiblechilds = true;
                m_flags.m_childrenOnSameLayer = true;

                //add properties and handles as children in order to edit the object.
                a2dVertexList::iterator iter = m_lsegments->begin();
                a2dVertexList::iterator iterorg = original->m_lsegments->begin();
                a2dLineSegment* seg;
                while ( iter != GetSegments()->end() )
                {
                    seg = ( *iter );
                    //not inclusive matrix so relative to polygon
                    a2dPolyHandleL* handle = new a2dPolyHandleL( this, iter, iterorg, m_lsegments, original->m_lsegments, wxT( "__index__" ) );
                    Append( handle );
                    handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                    handle->SetLayer( m_layer );

                    if ( seg->GetArc() )
                    {
                        // todo split in line segments ( maybe under certain conditions of matrix not?? ).
                        a2dArcSegment* cseg = ( a2dArcSegment* ) seg;

                        handle = new a2dPolyHandleL( this, iter, iterorg, m_lsegments, original->m_lsegments, cseg->m_x2, cseg->m_y2, wxT( "__indexarc__" ) );
                        handle->SetArcHandle( true );
                        handle->SetLayer( m_layer );
                        Append( handle );
                        handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                    }
                    iter++;
                    iterorg++;
                }

                m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dPolyHandleL" ) );
                m_flags.m_editingCopy = true;

                //calculate bbox's elase mouse events may take place when first idle event is not yet
                //processed to do this calculation.
                Update( updatemask_force );
                //stil set it pending to do the redraw ( in place )
                SetPending( true );
            }
            return true;
        }
        else
        {
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

void a2dPolygonL::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    if ( !m_flags.m_editingCopy || !m_flags.m_editable )
    {
        event.Skip();
        return;
    }

    a2dIterC* ic = event.GetIterC();

    a2dPolyHandleL* seghandle;

    double xw, yw;
    xw = event.GetX();
    yw = event.GetY();

    a2dPolygonL* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dPolygonL );
    wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );

    // this is the absolute matrix relative to the parent object, so with out this handle its
    // m_lworld included.
    // a2dAffineMatrix atWorld = ic->GetTransform();

    if ( editmode == 1 )
    {
        if ( event.GetMouseEvent().Moving() )
        {
            if ( event.GetHow().IsStrokeHit() )
            {
	            if ( !event.GetMouseEvent().AltDown() )
			    {
				    switch( event.GetHow().m_stroke2 )
				    {
					    case a2dHit::stroke2_edgehor:
					    case a2dHit::stroke2_edgevert:
					    case a2dHit::stroke2_edgeother:
						    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_WireSegmentInsert ) );
						    break;
					    case a2dHit::stroke2_vertex:
						    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireVertex ) );
						    break;
					    default:
						    break;
				    }
			    }
			    else
			    {
				    switch( event.GetHow().m_stroke2 )
				    {
					    case a2dHit::stroke2_edgehor:
						    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentHorizontal ) );
						    break;
					    case a2dHit::stroke2_edgevert:
						    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentVertical ) );
						    break;
					    case a2dHit::stroke2_edgeother:
						    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegment ) );
						    break;
					    case a2dHit::stroke2_vertex:
						    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireDeleteVertex ) );
						    break;
					    default:
						    break;
				    }
			    }
            }
            else
            {
                // skip has same effect
                //ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CanvasObjectHit ) );
                event.Skip();
            }
        }
        else if ( event.GetMouseEvent().LeftDown() )
        {
            a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();
            if ( restrictEngine )
                restrictEngine->SetRestrictPoint( xw, yw );

            if ( event.GetHow().IsEdgeHit() )
            {
                wxUint16 editstyle = PROPID_Editstyle->GetPropertyValue( this );
                if( ! ( editstyle & wxEDITSTYLE_NOHANDLES ) )
                {
                    a2dVertexList::iterator iterSeg = GetSegmentAtIndex( event.GetHow().m_index );
                    a2dVertexList::iterator iterSegOrg = original->GetSegmentAtIndex( event.GetHow().m_index );
                    a2dLineSegment* seg = *iterSeg;
                    if ( event.GetMouseEvent().AltDown() )
                        seghandle = new a2dPolyHandleL( this, iterSeg, iterSegOrg, m_lsegments, original->m_lsegments, wxT( "__segment__" ) );
                    else
                        seghandle = new a2dPolyHandleL( this, iterSeg, iterSegOrg, m_lsegments, original->m_lsegments, wxT( "__insert__" ) );
                    seghandle->SetLayer( m_layer );
                    Append( seghandle );
                    seghandle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
				    //canvas mouse events arrive without the polygon matrix included. 
                    a2dIterCU polygoncontext( *ic, this );
                    a2dIterCU handlecontext( *ic, seghandle );
                    seghandle->ProcessEvent( event );
                }
            }
            else if ( event.GetHow().IsHit() )
            {
                // If connected a move of a wire is weird, if not rewired.
                // Understand the wire edit copy is not connected (see edittool).
                if ( ! original->IsConnected( false ) ) 
                    event.Skip();
            }
            else
                EndEdit();
        }
        else if ( event.GetMouseEvent().Dragging() || event.GetMouseEvent().LeftUp() )
        {
            if ( ! original->IsConnected( false ) ) 
                event.Skip();
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

void a2dPolygonL::OnChar( wxKeyEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_TAB:
            {
                wxUint16* editmode = PROPID_Editmode->GetPropertyValuePtr( this );
                if ( editmode )
                {
                    ( *editmode )++;
                    if ( ( *editmode ) > 1 ) ( *editmode ) = 0;
                    ReStartEdit( ( *editmode ) );
                }
                break;
            }
            case WXK_DELETE:
            {
				if ( m_lsegments->size() <= 2 )
				{
					wxLogWarning( _( "stopped removing segment or vertexes, object should keep at least one segment" ) );
                    break; // leaf at least one segment
				}
				a2dPolygonL* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dPolygonL );
                //first delete selected segments ( nodes maybe selcted, and those nodes maybe removed because of this.
                for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
                {
                    if ( m_lsegments->size() <= 2 )
                    {
                        wxLogWarning( _( "stopped removing segment or vertexes, object should keep at least one segment" ) );
                        break; // leaf at least one segment
                    }
                    a2dCanvasObject* obj = *iter;

                    a2dPolyHandleL* handle = wxDynamicCast( obj, a2dPolyHandleL );

                    if ( handle && !handle->GetRelease( ) )
                    {
                        a2dVertexList::iterator iterpoly = handle->GetNode();
                        a2dLineSegment* segment = *( iterpoly );
                        if ( segment->GetBin() ) //selected?
                        {
                            iter = m_childobjects->GetNextAround( iter );
                            a2dPolyHandleL* handle2 = wxDynamicCast( (*iter).Get(), a2dPolyHandleL );
                            iter = m_childobjects->GetPreviousAround( iter );
                            a2dLineSegment* segmentOrgNext = *( handle2->GetNodeOrg() );
                            a2dLineSegment* segmentOrg = *( handle->GetNodeOrg() );

                            double xn,yn;
                            xn = (handle->GetPosX() + handle2->GetPosX())/2.0;
                            yn = (handle->GetPosY() + handle2->GetPosY())/2.0;
                            handle2->SetPosXY( xn, yn );
                            m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( original, segmentOrg ) );
                            m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveSegment( original, xn, yn, segmentOrgNext, false ) );
                            a2dVertexList::iterator iterpolyNext = m_lsegments->GetNextAround( iterpoly );
                            iterpoly = m_lsegments->erase( iterpoly );
                            (*iterpolyNext)->m_x = xn; (*iterpolyNext)->m_y = yn;
                            ReleaseChild( handle );
                        }
                    }
                }

                //now delete selected handles (only those which were not released above).
                for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
                {
                    a2dCanvasObject* obj = *iter;

                    a2dPolyHandleL* handle = wxDynamicCast( obj, a2dPolyHandleL );

                    if ( handle &&  !handle->GetRelease( ) && handle->GetSelected() )
                    {
                        a2dVertexList::iterator iterpoly = handle->GetNode();
                        a2dLineSegment* segment = *( iterpoly );
                        a2dVertexList::iterator iterorg = handle->GetNodeOrg();
                        a2dLineSegment* segmentOrg = *( iterorg );

						if ( m_lsegments->size() <= 2 )
						{
							wxLogWarning( _( "stopped removing segment or vertexes, object should keep at least one segment" ) );
						    break; // leaf at least one segment
						}
						else
						{
							m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( original, segmentOrg ) );
							m_lsegments->erase( iterpoly );
							ReleaseChild( handle );
						}
                    }
                }
                original->AdjustAfterChange( true );
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();
}

void a2dPolygonL::SyncHandlesWithLineSegments()
{
    for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;

        a2dPolyHandleL* handle = wxDynamicCast( obj, a2dPolyHandleL );

        if ( handle && !handle->GetRelease( ) )
        {
            a2dLineSegment* segment = *( handle->GetNode() );
            // handle a wxHandle position when its a arc segment
            if ( segment->GetArc() && handle->GetArcHandle() )
            {
                a2dArcSegment* cseg = ( a2dArcSegment* ) segment;
                // its an arc, so this handle can point to a midpoint
                handle->SetPosXyPoint( cseg->GetMidPoint() );
            }
            else
                handle->SetPosXY( segment->m_x, segment->m_y );
        }
    }
}

void a2dPolygonL::OnHandleEventEnter( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();
	//ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireVertex ) );
	//ic->GetDrawingPart()->PushCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireVertex ) );//a2dCURSOR_HandleEnter ) );
    //ic->GetDrawingPart()->PushCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HandleEnter ) );
}

void a2dPolygonL::OnHandleEventLeave( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();
	//ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_WireSegmentInsert ) );
	//ic->GetDrawingPart()->PopCursor();
}

void a2dPolygonL::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( event.GetCanvasHandle()->GetRelease() )
        return;

    if ( m_flags.m_editingCopy )
    {
        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();
        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );
        a2dPolygonL* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dPolygonL );

        if ( editmode == 1 )
        {
            double xw, yw;
            xw = event.GetX();
            yw = event.GetY();
            a2dAffineMatrix atWorld = ic->GetTransform();

            //we need to get the matrix until the parent of the a2dPolyHandleL,
            // and not inclusif the handle itself.
            //and therefore apply inverted handle matrix.
            a2dAffineMatrix inverselocal = m_lworld;
            inverselocal.Invert();

            a2dAffineMatrix inverse = ic->GetInverseParentTransform();

            double xinternal, yinternal;
            inverse.TransformPoint( xw, yw, xinternal, yinternal );

            a2dPolyHandleL* draghandle = wxDynamicCast( event.GetCanvasHandle(), a2dPolyHandleL );
            if ( !draghandle )
                return; //bad
			a2dPinPtr pin = draghandle->GetPin();
            a2dVertexList::iterator iter = draghandle->GetNode();
            a2dLineSegment* segment = *( iter );
            a2dLineSegment* segmentNext = *( m_lsegments->GetNextAround( iter ) );
            a2dVertexList::iterator iterorg = draghandle->GetNodeOrg();
            a2dLineSegment* segmentOrg = *( iterorg );
            a2dLineSegment* segmentNextOrg = *( original->m_lsegments->GetNextAround( iterorg ) );

            double xwi;
            double ywi;
            inverselocal.TransformPoint( xinternal, yinternal, xwi, ywi );
            a2dAffineMatrix origworld = m_lworld;

            if ( draghandle->GetName() == wxT( "__index__" ) )
            {
				if( m_flags.m_snap )
				{
					if ( restrictEngine )
						restrictEngine->RestrictPoint( xwi, ywi );
				}
                if ( event.GetMouseEvent().AltDown() )
                {
					ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireDeleteVertex ) );
	                if ( event.GetMouseEvent().LeftDown() )
					{		
						m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( original, segmentOrg ) );
						//a2dToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dToolContr );
						//controller->TriggerReStartEdit( 1 );

						double x, y;
						RemoveSegment( x, y, segment );
						ReleaseChild( draghandle );

						// the handle generated this event in LeftDown, But before LeftUp event the
						// restart is already done, getting rid of handle.
						// So do set context right here.
						ic->SetCorridorPathToParent();
						ic->GetDrawingPart()->PopCursor();
					}
                }
				else
                {
	                if ( event.GetMouseEvent().LeftDown() )
					{		
                        draghandle->m_group = m_root->GetCommandProcessor()->CommandGroupBegin( wxT( "move vertex" ) );

    					ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireVertex ) );
                    }
                    else if ( event.GetMouseEvent().LeftUp() )
                    {
                        if ( draghandle->m_dragging )
                        {
                            draghandle->m_dragging = false;
                            double x, y;
                            x = segment->m_x; y = segment->m_y;
                            m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveSegment( original, x, y, segmentOrg, false ) );
                            //a2dDocviewGlobals->RecordF( this, wxT("movepoint %f %f %d"), x, y, index );
                            draghandle->SetPosXY( x, y );
                            //Klaas (code not good, handles are used and removed at same time )
                            //RemoveRedundantPoints( original );
                            original->AdjustAfterChange( true );


						    if ( pin )
						    {
							    original->SetAlgoSkip( true );
							    SetAlgoSkip( true );

                                double hitDistance = ic->GetHitMarginWorld();

                                a2dPin* pinother = pin->GetPinClass()->GetConnectionGenerator()->
	                                        SearchPinForFinishWire( ic->GetDrawingPart()->GetShowObject(), pin, NULL, hitDistance  );

                                if ( pinother && ! pin->IsConnectedTo( pinother ) )
                                {
                                    m_root->GetCanvasCommandProcessor()->Submit( new a2dCommand_ConnectPins( pin, pinother ), true );
                                }

							    original->SetAlgoSkip( false );
							    SetAlgoSkip( false );
						    }

                            m_root->GetCommandProcessor()->CommandGroupEnd( draghandle->m_group );

                        }
                        else
                            draghandle->SetSelected( !draghandle->GetSelected() );
                    }
                    else if ( event.GetMouseEvent().Dragging() )
                    {
					    if ( pin && !draghandle->m_dragging && pin->FindNonWirePin() )
                            pin->GetParent()->DisConnectAt( pin, true );

                        //wxLogDebug( _("Dragging %d"), index);
                        segment->m_x = xwi;
                        segment->m_y = ywi;
                        draghandle->m_dragging = true;
                        SetPending( true );
                        draghandle->SetPosXY( xwi, ywi );
                        SetPosXYSegment( segment, xwi, ywi, false  );
                        AdjustAfterChange( false );
                    }
                    else if ( event.GetMouseEvent().Moving() )
                    {
				    }
                    else
                        event.Skip(); //maybe on the base class handles
                }
            }
            else if ( draghandle->GetName() == wxT( "__indexarc__" ) )
            {
				if( m_flags.m_snap )
				{
					if ( restrictEngine )
						restrictEngine->RestrictPoint( xwi, ywi );
				}
                a2dArcSegment* csegment = ( a2dArcSegment* ) segment;
                if ( event.GetMouseEvent().LeftUp() )
                {
                    double x, y;
                    x = csegment->m_x2; y = csegment->m_y2;
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveMidSegment( original, x, y, segmentOrg, false ) );
                    //a2dDocviewGlobals->RecordF( this, wxT("movearcmidpoint %f %f %d"), x, y, index );
                    draghandle->SetPosXY( x, y );
                    //Klaas (code not good, handles are used and removed at same time )
                    //RemoveRedundantPoints( original );
                    original->AdjustAfterChange( true );
                }
                else if ( event.GetMouseEvent().Dragging() )
                {
                    csegment->m_x2 = xwi; csegment->m_y2 = ywi;
                    SetPending( true );
                    //csegment->SetMidPoint( *prevsegment, xwi, ywi );
                    //wxLogDebug( _("Dragging %d"), index);
                    draghandle->SetPosXYRestrict( xwi, ywi );
                    SyncHandlesWithLineSegments();
                    AdjustAfterChange( false );
                }
                else
                    event.Skip(); //maybe on the base class handles
            }
            else if ( draghandle->GetName() == wxT( "__insert__" ) )
            {
				ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_WireSegmentInsert ) );

                // The index is the segment index. The index of the new point is 1 larger
                // than the segment index.
                if ( event.GetMouseEvent().LeftUp() && draghandle->m_dragging )
                {
                    draghandle->m_dragging = false;
                    double x, y;
                    GetPosXYSegment( segmentNext, x, y, false );
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_AddPoint( original, x, y, segmentNextOrg, false ) );
                    // !!!!???? how to do this ????!!!!
                    // a2dDocviewGlobals->RecordF( this, wxT("addpoint %f %f %d"), x, y, index+1 );
                    draghandle->SetPosXY( x, y );
                    draghandle->SetName( wxT( "__index__" ) );
                    iter = m_lsegments->GetNextAround( iter );
                    iterorg = original->m_lsegments->GetNextAround( iterorg );
                    draghandle->SetNode( iter );
                    draghandle->SetNodeOrg( iterorg );

                    original->AdjustAfterChange( true );
                    segment->m_bin = false;
                }
                else if ( event.GetMouseEvent().LeftUp() )
                {
                    segment->m_bin = !segment->m_bin; //use bin for select
                    SetPending( true );
                    draghandle->m_dragging = false;
                    draghandle->SetRelease( true );
                    original->AdjustAfterChange( true );
                }
                else if ( event.GetMouseEvent().LeftDown() )
                {
                    draghandle->m_dragging = false;
                }
                else if ( event.GetMouseEvent().Dragging() && !draghandle->m_dragging )
                {
                    draghandle->m_dragging = true;
                    draghandle->SetPosXYRestrict( xwi, ywi );
                    AddPoint( xwi, ywi, segmentNext, false );
                    AdjustAfterChange( false );
                }
                else if ( event.GetMouseEvent().Dragging() )
                {
                    draghandle->SetPosXYRestrict( xwi, ywi );
                    SetPosXYSegment( segmentNext, xwi, ywi , false  );
                    AdjustAfterChange( false );
                }
                else
                    event.Skip(); //maybe on the base class handles
            }
            else if ( draghandle->GetName() == wxT( "__segment__" ) )
            {
                if ( event.GetMouseEvent().LeftUp() )
                {
                    if ( !draghandle->m_dragging )
                    {
                        segment->m_bin = !segment->m_bin; //use bin for select
                        SetPending( true );
                    }
                    else
                    {
                        segment->m_bin = false;
                        draghandle->m_dragging = false;

                        if( m_flags.m_snap )
                        {
                            double xi_h  = a2dPolyHandleL::PROPID_tmpXIH->GetPropertyValue( draghandle );
                            double yi_h  = a2dPolyHandleL::PROPID_tmpYIH->GetPropertyValue( draghandle );
                            double xi_v1 = a2dPolyHandleL::PROPID_tmpXIV1->GetPropertyValue( draghandle );
                            double yi_v1 = a2dPolyHandleL::PROPID_tmpYIV1->GetPropertyValue( draghandle );
                            double xi_v2 = a2dPolyHandleL::PROPID_tmpXIV2->GetPropertyValue( draghandle );
                            double yi_v2 = a2dPolyHandleL::PROPID_tmpYIV2->GetPropertyValue( draghandle );
                            xwi = segment->m_x + xi_v1 - xi_h;
                            ywi = segment->m_y + yi_v1 - yi_h;
                        }
                        draghandle->SetPosXY( xwi, ywi );

                        double x1 = segment->m_x;
                        double y1 = segment->m_y;
                        double x2 = segmentNext->m_x;
                        double y2 = segmentNext->m_y;

                        m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveSegment( original, x1, y1, segmentOrg, false ) );
                        m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveSegment( original, x2, y2, segmentNextOrg, false ) );

                        //a2dDocviewGlobals->RecordF( this, wxT("movepoint %f %f %d"), x1, y1, index );
                        //a2dDocviewGlobals->RecordF( this, wxT("movepoint %f %f %d"), x2, y2, index+1 );
                    }
                    //Klaas (code not good, handles are used and removed at same time )
                    //RemoveRedundantPoints( original );
                    //
                    SyncHandlesWithLineSegments();


                    // handles called "__segment__" are temporary handles, can be removed after drag
                    m_childobjects->Release( a2dCanvasOFlags::ALL, wxT( "a2dPolyHandleL" ), NULL, wxT( "__segment__" ), false );
                    original->AdjustAfterChange( true );
                }
                else if ( event.GetMouseEvent().LeftDown() )
                {
                    // set the initial handle coordinates
                    a2dPolyHandleL::PROPID_tmpXIH->SetPropertyToObject( draghandle, xwi );
                    a2dPolyHandleL::PROPID_tmpYIH->SetPropertyToObject( draghandle, ywi );
                    // set the initial vertex coordinates
                    double x, y;
                    x = segment->m_x;
                    y = segment->m_y;
                    a2dPolyHandleL::PROPID_tmpXIV1->SetPropertyToObject( draghandle, x );
                    a2dPolyHandleL::PROPID_tmpYIV1->SetPropertyToObject( draghandle, y );
                    x = segmentNext->m_x;
                    y = segmentNext->m_y;
                    a2dPolyHandleL::PROPID_tmpXIV2->SetPropertyToObject( draghandle, x );
                    a2dPolyHandleL::PROPID_tmpYIV2->SetPropertyToObject( draghandle, y );
                }
                else if ( event.GetMouseEvent().Dragging() )
                {
                    draghandle->m_dragging = true;

                    double xi_h  = a2dPolyHandleL::PROPID_tmpXIH->GetPropertyValue( draghandle );
                    double yi_h  = a2dPolyHandleL::PROPID_tmpYIH->GetPropertyValue( draghandle );
                    double xi_v1 = a2dPolyHandleL::PROPID_tmpXIV1->GetPropertyValue( draghandle );
                    double yi_v1 = a2dPolyHandleL::PROPID_tmpYIV1->GetPropertyValue( draghandle );
                    double xi_v2 = a2dPolyHandleL::PROPID_tmpXIV2->GetPropertyValue( draghandle );
                    double yi_v2 = a2dPolyHandleL::PROPID_tmpYIV2->GetPropertyValue( draghandle );

                    //wxASSERT( xi_h && yi_h && xi_v1 && yi_v1 && xi_v2 && yi_v2 );

                    if( m_flags.m_snap )
                    {
                        // snap vertex 1 of segment.
                        double x, y, dx, dy;

                        dx = xi_v1 - xi_h;
                        dy = yi_v1 - yi_h;
                        x = xwi + dx;
                        y = ywi + dy;
                        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();
                        if ( restrictEngine )
                            restrictEngine->RestrictPoint( x, y );
                        xwi = x - dx;
                        ywi = y - dy;
                    }
                    draghandle->SetPosXY( xwi, ywi );
                    double x1 = xi_v1 - xi_h + xwi;
                    double y1 = yi_v1 - yi_h + ywi;
                    double x2 = xi_v2 - xi_h + xwi;
                    double y2 = yi_v2 - yi_h + ywi;

                    segment->m_x = x1;
                    segment->m_y = y1;
                    segmentNext->m_x = x2;
                    segmentNext->m_y = y2;

                    SetPending( true );
                    AdjustAfterChange( false );
                }
                else if ( event.GetMouseEvent().Moving() )
                {
					//ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentHorizontal ) );
					//ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegmentVertical ) );
					ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireSegment ) );
				}
                else
                    event.Skip(); //maybe on the base class handles
            }
            else
                event.Skip(); //maybe the base class handles
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

a2dBoundingBox a2dPolygonL::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    if (  m_spline )
    {
        a2dVertexList* lpoints = new a2dVertexList();

        *lpoints = *m_lsegments;
        lpoints->ConvertIntoSplinedPolygon( double(GetHabitat()->GetAberPolyToArc()) / m_root->GetUnitsScale() );
        bbox = lpoints->GetBbox();
        delete lpoints;
    }
    else
        bbox = m_lsegments->GetBbox();

    if ( m_contourwidth > 0 )
        bbox.Enlarge( m_contourwidth / 2 );

    return bbox;
}

bool a2dPolygonL::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        if (  m_spline )
        {
            a2dVertexList* lpoints = new a2dVertexList();

            *lpoints = *m_lsegments;
            lpoints->ConvertIntoSplinedPolygon( double(GetHabitat()->GetAberPolyToArc()) / m_root->GetUnitsScale() );
            m_bbox = lpoints->GetBbox( m_lworld );
            delete lpoints;
        }

        a2dBoundingBox bboxs = DoGetUnTransformedBbox();
        bboxs.MapBbox( m_lworld );
        m_bbox.Expand( bboxs );

        if ( m_flags.m_editingCopy && m_spline )
        {
            m_spline = false;
            a2dBoundingBox bboxs = DoGetUnTransformedBbox();
            bboxs.MapBbox( m_lworld );
            m_bbox.Expand( bboxs );
            m_spline = true;
        }

        m_shapeIdsValid = false;
        return true;
    }

    return false;
}

void a2dPolygonL::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    if ( !m_shapeIdsValid )
    {
        ic.GetDrawer2D()->DeleteShapeIds( m_shapeIds );
        m_shapeIds = a2dListId();
    }

    if ( m_shapeIds.m_offset )
    {
        ic.GetDrawer2D()->DrawShape( m_shapeIds );
    }
    else
    {
        if ( m_contourwidth )
        {
            a2dVertexList* contourpoints = m_lsegments->ConvertToContour(  m_contourwidth / 2, a2dPATH_END_ROUND, true );
            ic.GetDrawer2D()->DrawPolygon( contourpoints, m_spline );
            delete contourpoints;
        }
        else
        {
            ic.GetDrawer2D()->DrawPolygon( m_lsegments, m_spline, wxWINDING_RULE );
        }
        m_shapeIds = ic.GetDrawer2D()->GetShapeIds();
        m_shapeIdsValid = true;
    }

    if ( m_crossAtVertex )
    {
        a2dStroke current = ic.GetDrawer2D()->GetDrawerStroke();
        a2dFill fillcurrent = ic.GetDrawer2D()->GetDrawerFill();
        ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetHandle()->GetFill() );
        ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetHandle()->GetStroke() );

        double x, y;
        a2dAffineMatrix matrix = ic.GetDrawer2D()->GetUserToDeviceTransform();
        ic.GetDrawer2D()->PushIdentityTransform();

        a2dVertexList::iterator iter = m_lsegments->begin();
        a2dLineSegment* seg;
        while ( iter != m_lsegments->end() )
        {
            seg = ( *iter );
            x = seg->m_x;
            y = seg->m_y;
            double xd, yd;
            matrix.TransformPoint( x, y, xd, yd );

            ic.GetDrawer2D()->DrawCircle( xd, yd, a2dCrossAtVertexSize ); 
            iter++;
        }
	    ic.GetDrawer2D()->PopTransform();
        ic.GetDrawer2D()->SetDrawerStroke( current );
        ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
	}

    if ( m_flags.m_editingCopy )
    {
        ic.GetDrawer2D()->OverRuleFixedStyle();
        a2dBoundingBox untrans = DoGetUnTransformedBbox();
        double x, y, w, h;
        x = untrans.GetMinX();
        y = untrans.GetMinY();
        w = untrans.GetWidth();
        h = untrans.GetHeight();

        if ( m_spline )
            ic.GetDrawer2D()->DrawPolygon( m_lsegments, false, wxWINDING_RULE );

        /*
        a2dSmrtPtr<a2dRect> around = new a2dRect( x, y, w, h );
        around->SetIgnoreSetpending();
        around->SetRoot( m_root, false );
        around->Update( updatemask_force );
        around->SetStroke( *wxRED, 0, a2dSTROKE_DOT_DASH );
        around->SetFill( *a2dTRANSPARENT_FILL );
        around->Render( ic, _ON );
        */

        ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
        ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( *wxRED, 0, a2dSTROKE_DOT_DASH) );
        ic.GetDrawer2D()->DrawRoundedRectangle( x, y, w, h, 0 );

        double xp, yp;
        a2dVertexList::iterator iter = m_lsegments->begin();
        a2dLineSegment* seg;
        while ( iter != m_lsegments->end() )
        {
            seg = ( *iter );
            if ( seg->GetBin() ) //selected?
            {
                x = seg->m_x;
                y = seg->m_y;
                a2dLineSegment* nextseg = *( m_lsegments->GetNextAround( iter ) );
                xp = nextseg->m_x;
                yp = nextseg->m_y;

                ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetSelectFill() );
                ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetSelectStroke() );
                ic.GetDrawer2D()->DrawLine( x, y, xp, yp ); 
            }
            iter++;
        }

        ic.GetDrawer2D()->ReStoreFixedStyle();
    }
}

#if wxART2D_USE_CVGIO
void a2dPolygonL::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_spline )
        {
            out.WriteAttribute( wxT( "spline" ), m_spline );
        }

        out.WriteAttribute( wxT( "points" ), m_lsegments->size() );
        if ( m_contourwidth )
            out.WriteAttribute( wxT( "contourwidth" ), m_contourwidth * out.GetScale() );
    }
    else
    {
        unsigned int i = 0;
        forEachIn( a2dVertexList, m_lsegments )
        {
            if ( i % 6 == 0 )
                out.WriteNewLine();
            a2dLineSegment* seg = *iter;
            out.WriteStartElementAttributes( wxT( "xy" ), false );
            out.WriteAttribute( wxT( "x" ), seg->m_x * out.GetScale()  );
            out.WriteAttribute( wxT( "y" ), seg->m_y * out.GetScale()  );

            if ( seg->GetArc() )
            {
                a2dArcSegment* arc = ( a2dArcSegment* ) seg;
                out.WriteAttribute( wxT( "xm" ), arc->m_x2 * out.GetScale()  );
                out.WriteAttribute( wxT( "ym" ), arc->m_y2 * out.GetScale()  );
            }

            out.WriteEndAttributes( true );
            out << " ";
            i++;
        }
    }
}

void a2dPolygonL::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_spline =  parser.GetAttributeValueBool( wxT( "spline" ), false );
        m_contourwidth = parser.GetAttributeValueDouble( wxT( "contourwidth" ), 0 ) * parser.GetScale() ;
        SetContourWidth( m_contourwidth ); //makes sure filled flag is set correctly.
    }
    else
    {
        while( parser.GetTagName() == wxT( "xy" )  )
        {
            double x = parser.GetAttributeValueDouble( wxT( "x" ) ) * parser.GetScale() ;
            double y = parser.GetAttributeValueDouble( wxT( "y" ) ) * parser.GetScale() ;

            a2dLineSegment* seg;
            if ( parser.HasAttribute( wxT( "xm" ) ) )
            {
                double xm = parser.GetAttributeValueDouble( wxT( "xm" ) ) * parser.GetScale() ;
                double ym = parser.GetAttributeValueDouble( wxT( "ym" ) ) * parser.GetScale() ;

                seg = new a2dArcSegment( x, y, xm, ym );
            }
            else
                seg = new a2dLineSegment( x, y );

            m_lsegments->push_back( seg );

            parser.Next();
            parser.Require( END_TAG, wxT( "xy" ) );
            parser.Next();
        }
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dPolygonL::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double xh, yh;
    ic.GetInverseTransform().TransformPoint( hitEvent.m_x, hitEvent.m_y, xh, yh );

    a2dPoint2D P = a2dPoint2D( xh, yh );
    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

    hitEvent.m_how = PointInPolygon( P, pw + margin );

    return hitEvent.m_how.IsHit();
}

a2dHit a2dPolygonL::PointInPolygon( const a2dPoint2D& P, double marge )
{
    a2dVertexList* rlist;
    if (  m_spline )
    {
        a2dVertexList* lpoints = new a2dVertexList();
        *lpoints = *m_lsegments;
        double AberArcToPoly = m_root->GetAberArcToPolyScaled();
        lpoints->ConvertIntoSplinedPolygon( AberArcToPoly );
        rlist = lpoints;
    }
    else if ( m_lsegments->HasArcs() )
    {
        double AberArcToPoly = m_root->GetAberArcToPolyScaled();
        a2dVertexList* lpoints = new a2dVertexList();
        *lpoints = *m_lsegments;
        lpoints->ConvertToLines(AberArcToPoly);
        rlist = lpoints;
    }
    else
        rlist = m_lsegments;

    a2dHit result = rlist->HitTestPolygon( P, marge );

    if (  m_spline || m_lsegments->HasArcs() )
        delete rlist;

    return result;
}

//----------------------------------------------------------------------------
// a2dPolylineL
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dPolylineL, a2dPolygonL )
    EVT_CHAR( a2dPolylineL::OnChar )
END_EVENT_TABLE()

bool a2dPolylineL::m_generatePins = false; 

a2dPolylineL::a2dPolylineL()
    : a2dPolygonL()
{
    m_xscale = m_yscale = 1;
    m_pathtype = a2dPATH_END_SQAURE;
    m_pathCornerType = a2dPATH_CORNER_SQAURE;
    m_flags.m_generatePins = m_generatePins; 
    m_flags.m_filled = false;
}

a2dPolylineL::a2dPolylineL( a2dVertexListPtr points, bool spline  )
    : a2dPolygonL( points, spline )
{
    m_xscale = m_yscale = 1;
    m_pathtype = a2dPATH_END_SQAURE;
    m_pathCornerType = a2dPATH_CORNER_SQAURE;
    m_flags.m_generatePins = m_generatePins; 
    m_flags.m_filled = false;
}

a2dPolylineL::a2dPolylineL( const a2dPolylineL& other, CloneOptions options, a2dRefMap* refs )
    : a2dPolygonL( other, options, refs )
{
    //if ( options & clone_members  )
    //{
    m_begin = NULL;
    m_end = NULL;
    if ( other.m_begin )
        m_begin = other.m_begin->TClone( options );
    if ( other.m_end )
        m_end = other.m_end->TClone( options );
    //}
    //else
    //{
    //m_begin = other.m_begin;
    //m_end = other.m_end;
    //}
    m_xscale = other.m_xscale;
    m_yscale = other.m_yscale;
    m_pathtype = other.m_pathtype;
    m_pathCornerType = other.m_pathCornerType;
    m_flags.m_generatePins = other.m_generatePins; 
    m_flags.m_filled = other.m_flags.m_filled;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in Clone ( )
    CurrentSmartPointerOwner = this;
#endif
}

a2dPolylineL::~a2dPolylineL()
{
}

void a2dPolylineL::SetContourWidth( double width ) 
{  
    m_contourwidth = width; 
    SetFilled( width != 0 ); 
    SetPending( true ); 
}

void a2dPolylineL::SetBegin( a2dCanvasObject* begin )
{
    SetPending( true );
    m_begin = begin;
    if ( m_begin && m_root  )
        m_begin->SetRoot( m_root );
}

void a2dPolylineL::SetEnd( a2dCanvasObject* end )
{
    SetPending( true );
    m_end = end;
    if ( m_end && m_root )
        m_end->SetRoot( m_root );
}

a2dObject* a2dPolylineL::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dPolylineL( *this, options, refs );
}

a2dVertexList* a2dPolylineL::GetAsVertexList( bool& returnIsPolygon ) const
{
    if ( m_contourwidth )
    {
        a2dVertexList* contourpoints = m_lsegments->ConvertToContour(  m_contourwidth / 2, m_pathtype );
        if ( !returnIsPolygon )
            contourpoints->push_back( contourpoints->front()->Clone() );
        contourpoints->Transform( m_lworld );
        return contourpoints;
    }
    else
    {
        a2dVertexList* contourpoints = new a2dVertexList( *(m_lsegments.Get()) );
        contourpoints->Transform( m_lworld );
        returnIsPolygon = false;
        return contourpoints;
    }
}

a2dCanvasObjectList* a2dPolylineL::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dVpath* segments = NULL;

    if ( m_contourwidth )
    {
        if ( a2dGlobals->GetRoundFactor() == 0 && m_pathtype == a2dPATH_END_ROUND )
        {
            double AberArcToPoly = m_root->GetAberArcToPolyScaled();
            segments = new a2dVpath();

            a2dVertexList contourpoints = *m_lsegments;
            contourpoints.ConvertToLines(AberArcToPoly);

            a2dLineSegment* segprev = NULL;
            a2dLineSegment* seg = NULL;
            a2dPoint2D first;
            a2dPoint2D second;
            a2dPoint2D offsetpointleft;
            a2dPoint2D offsetpointright;

            a2dVertexList::iterator iter = contourpoints.begin();
            while( iter != contourpoints.end() )
            {
                a2dLineSegment* seg = *iter;
                seg = ( *iter );

                if ( segprev )
                {
                    first = a2dPoint2D( segprev->m_x, segprev->m_y );
                    second = a2dPoint2D( seg->m_x, seg->m_y );
                    offsetpointleft = first;
                    offsetpointright = first;
                    a2dLine line1( first, second );
                    line1.CalculateLineParameters();
                    line1.Virtual_Point( offsetpointleft, m_contourwidth / 2 );
                    line1.Virtual_Point( offsetpointright, -m_contourwidth / 2 );

                    a2dVpathSegment* segv = new a2dVpathSegment( offsetpointright.m_x, offsetpointright.m_y, a2dPATHSEG_MOVETO );
                    segments->Add( segv );

                    a2dLine  lineoffset( offsetpointright, offsetpointleft );
                    lineoffset.CalculateLineParameters();
                    a2dPoint2D offsetpointfirst;
                    offsetpointfirst = first;
                    lineoffset.Virtual_Point( offsetpointfirst, m_contourwidth / 2 );
                    segments->Add( new a2dVpathArcSegment( offsetpointleft.m_x, offsetpointleft.m_y,
                                                           offsetpointfirst.m_x, offsetpointfirst.m_y,
                                                           a2dPATHSEG_ARCTO, a2dPATHSEG_END_OPEN ) );

                    offsetpointright = second;
                    line1.Virtual_Point( offsetpointright, -m_contourwidth / 2 );
                    offsetpointleft = second;
                    line1.Virtual_Point( offsetpointleft, m_contourwidth / 2 );

                    segv = new a2dVpathSegment( offsetpointleft.m_x, offsetpointleft.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN );
                    segments->Add( segv );

                    a2dLine lineoffset2( offsetpointleft, offsetpointright );
                    lineoffset2.CalculateLineParameters();
                    a2dPoint2D offsetpointend;
                    offsetpointend = second;
                    lineoffset2.Virtual_Point( offsetpointend, m_contourwidth / 2 );
                    segments->Add( new a2dVpathArcSegment( offsetpointright.m_x, offsetpointright.m_y,
                                                           offsetpointend.m_x, offsetpointend.m_y,
                                                           a2dPATHSEG_ARCTO, a2dPATHSEG_END_OPEN ) );

                    segv = new a2dVpathSegment( offsetpointright.m_x, offsetpointright.m_y, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED );
                    segments->Add( segv );
                }
                iter++;
                segprev = seg;
            }
        }
        else
        {
            segments = new a2dVpath( *m_lsegments, true );
            segments->Contour( m_contourwidth / 2, m_pathtype );
        }
    }
    else
    {
        a2dVertexList contourpoints = *m_lsegments;
        segments = new a2dVpath( contourpoints, true, false );
    }
    if ( transform )
        segments->Transform( pworld );

    a2dVectorPath* canpath = new a2dVectorPath( segments );
    canpath->SetRoot( m_root );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );
    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}

a2dCanvasObjectList* a2dPolylineL::GetAsLinesArcs( bool transform )
{
    a2dCanvasObjectList* linesAnArcs = new a2dCanvasObjectList();

    double xp = m_lsegments->front()->m_x;
    double yp = m_lsegments->front()->m_y;
    if ( transform )
        m_lworld.TransformPoint( xp, yp, xp, yp );
    a2dCanvasObject* obj;
    a2dVertexList::iterator iter = m_lsegments->begin();
    iter++;
    while( iter != m_lsegments->end() )
    {
        double x, y, xm, ym;
        a2dLineSegment* seg = *iter;
        x = seg->m_x; y = seg->m_y;
        if ( transform )
            m_lworld.TransformPoint( x, y, x, y );
        if ( seg->GetArc() )
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
            xm = cseg->m_x2; ym = cseg->m_y2;
            if ( transform )
                m_lworld.TransformPoint( xm, ym, xm, ym );
            a2dArc* arc = new a2dArc();
            arc->SetChord( true );
            obj = arc;
            arc->Set( xp, yp, xm, ym, x, y );
            linesAnArcs->push_back( arc );
        }
        else
        {
            a2dSLine* line = new a2dSLine( xp, yp, x, y );
            obj = line;
            linesAnArcs->push_back( line );
        }

        obj->SetStroke( this->GetStroke() );
        obj->SetFill( this->GetFill() );
        obj->SetLayer( m_layer );
        obj->SetContourWidth( GetContourWidth() );
        obj->SetRoot( m_root, false );

        xp = x;
        yp = y;
        ++iter;
    }

    return linesAnArcs;
}

bool a2dPolylineL::RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld )
{
    // object has disabled snap?
    if ( !m_flags.m_snap_to )
        return false;

    double StrokeExtend = 0;
    a2dStroke stroke = GetStroke();

    if ( stroke.IsNoStroke() ) //IsNoStroke() stroke means use layer
    {
        if ( m_root && m_root->GetLayerSetup() )
        {
            a2dStroke layerpen = m_root->GetLayerSetup()->GetStroke( m_layer );
            if ( layerpen.GetPixelStroke() )
                StrokeExtend = layerpen.GetWidth() / 2;
            else
                StrokeExtend = layerpen.GetWidth() / 2;
        }
    }
    else if ( !stroke.IsSameAs( *a2dTRANSPARENT_STROKE ) )
    {
        if ( stroke.GetPixelStroke() )
        {
            StrokeExtend = ic.GetDrawer2D()->DeviceToWorldXRel( stroke.GetWidth() / 2 );
        }
        else
            StrokeExtend = stroke.GetWidth() / 2;
    }
    ic.SetStrokeWorldExtend( StrokeExtend );

    bool res = false;
    if ( snapToWhat & a2dRestrictionEngine::snapToObjectVertexes ||
         snapToWhat & a2dRestrictionEngine::snapToObjectSegmentsMiddle ||
         snapToWhat & a2dRestrictionEngine::snapToObjectSegments ||
         snapToWhat & a2dRestrictionEngine::snapToObjectIntersection
       )
    {
        a2dVertexList::iterator iter = m_lsegments->begin();
        double lastx, lasty;
        m_lworld.TransformPoint( m_lsegments->front()->m_x, m_lsegments->front()->m_y, lastx, lasty );
        a2dLineSegment* seg;
        iter++;
        while ( iter != m_lsegments->end() )
        {
            seg = ( *iter );

            double x, y;
            m_lworld.TransformPoint( seg->m_x, seg->m_y, x, y );
            a2dPoint2D p2( x, y );

            if ( snapToWhat & a2dRestrictionEngine::snapToObjectIntersection )
            {
            }
            if ( snapToWhat & a2dRestrictionEngine::snapToObjectSegments )
            {
                a2dPoint2D p1( lastx, lasty );
                a2dLine line( p1, p2 );
                a2dPoint2D pBestPoint;
                double dist = line.PointDistance( pointToSnapTo, &pBestPoint );
                double pw = ic.GetWorldStrokeExtend();
                if( dist < thresHoldWorld + pw )
                    res |= SetPointIfCloser( pointToSnapTo, pBestPoint, bestPointSofar, thresHoldWorld + pw );
            }
            if ( snapToWhat & a2dRestrictionEngine::snapToObjectSegmentsMiddle )
            {
                res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( x + lastx / 2.0, y + lasty / 2.0 ), bestPointSofar, thresHoldWorld );
            }
            if ( snapToWhat & a2dRestrictionEngine::snapToObjectVertexes )
            {
            //if within the threshold, do snap to vertex position.
                res |= SetPointIfCloser( pointToSnapTo, p2, bestPointSofar, thresHoldWorld );
            }
            lastx = x; lasty = y;
            iter++;
        }
    }

    snapToWhat = snapToWhat & ( a2dRestrictionEngine::snapToAll ^ a2dRestrictionEngine::snapToObjectVertexes ); //disable this for base now.

    return res || a2dCanvasObject::RestrictToObject( ic, pointToSnapTo, bestPointSofar, snapToWhat, thresHoldWorld );
}

void a2dPolylineL::RemoveRedundantPoints( a2dCanvasObject* sendCommandsTo )
{
    a2dVertexList::iterator iterp = m_lsegments->end();
    a2dVertexList::iterator iterpp = m_lsegments->end();
    a2dVertexList::iterator iter = m_lsegments->begin();

    int inode;

    // remove equal points
    iter = m_lsegments->begin();
    inode = 0;
    if( iter != m_lsegments->end() )
    {
        iterp = iter++;
        inode++;
    }
    while ( iter != m_lsegments->end() )
    {
        a2dLineSegment* line1 = *iterp;
        a2dLineSegment* line2 = *iter;
        if( line1->GetPoint() == line2->GetPoint() && !line1->GetArc() ) //this is arc save on line2
        {
            if( sendCommandsTo )
                m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( sendCommandsTo, inode ) );
            m_lsegments->erase( iterp );
        }
        else
        {
            inode++;
        }

        iterp = iter++;
    }

    //remove points on same x/y
    iter = m_lsegments->begin();
    inode = 0;
    if( iter != m_lsegments->end() )
    {
        iterp = iter++;
        inode++;
    }
    if( iter != m_lsegments->end() )
    {
        iterpp = iterp;
        iterp = iter++;
        inode++;
    }
    while ( iter != m_lsegments->end() )
    {
        a2dLineSegment* point0 = *iterpp;
        a2dLineSegment* point1 = *iterp;
        a2dLineSegment* point2 = *iter;

        if ( !point0->GetArc() && !point1->GetArc() && 
             ( ( point1->GetPoint().m_x == point0->GetPoint().m_x &&  point1->GetPoint().m_x == point2->GetPoint().m_x ) ||
               ( point1->GetPoint().m_y == point0->GetPoint().m_y &&  point1->GetPoint().m_y == point2->GetPoint().m_y )
             )
           ) 
        {
            if( sendCommandsTo )
                m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( sendCommandsTo, inode -1 ) );
            m_lsegments->erase( iterp );
        }
        else
        {
            inode++;
            iterpp = iterp;
        }
        iterp = iter++;
    }

    /*
    // remove 180 deg points
    iter = m_lsegments->begin();
    inode = 0;
    if( iter != m_lsegments->end() )
    {
        iterp = iter++;
        inode++;
    }
    if( iter != m_lsegments->end() )
    {
        iterpp = iterp;
        iterp = iter++;
        inode++;
    }
    while ( iter != m_lsegments->end() )
    {
        a2dLineSegment* point0 = *iterpp;
        a2dLineSegment* point1 = *iterp;
        a2dLineSegment* point2 = *iter;

        double dx10 = point1->m_x - point0->m_x;
        double dy10 = point1->m_y - point0->m_y;
        double dx21 = point2->m_x - point1->m_x;
        double dy21 = point2->m_y - point1->m_y;

        if(  !point0->GetArc() && !point1->GetArc() && dx10* dy21 - dx21* dy10 == 0 )
        {
            if( sendCommandsTo )
                m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( sendCommandsTo, inode - 1 ) );
            m_lsegments->erase( iterp );
        }
        else
        {
            inode++;
            iterpp = iterp;
        }
        iterp = iter++;
    }
    */
}

void a2dPolylineL::Remove180DegPoints( a2dCanvasObject* sendCommandsTo, bool removeZeroFirst )
{
    a2dVertexList::iterator iterp = m_lsegments->end();
    a2dVertexList::iterator iterpp = m_lsegments->end();
    a2dVertexList::iterator iter = m_lsegments->begin();
    int inode;

    if ( removeZeroFirst )
    {
        // remove equal points
        iter = m_lsegments->begin();
        inode = 0;
        if( iter != m_lsegments->end() )
        {
            iterp = iter++;
            inode++;
        }
        while ( iter != m_lsegments->end() )
        {
            a2dLineSegment* line1 = *iterp;
            a2dLineSegment* line2 = *iter;
            if( line1->GetPoint() == line2->GetPoint() && !line1->GetArc() ) //this is arc save on line2
            {
                if( sendCommandsTo )
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( sendCommandsTo, inode ) );
                m_lsegments->erase( iterp );
            }
            else
            {
                inode++;
            }

            iterp = iter++;
        }
    }

    // remove 180 deg points
    iter = m_lsegments->begin();
    inode = 0;
    if( iter != m_lsegments->end() )
    {
        iterp = iter++;
        inode++;
    }
    if( iter != m_lsegments->end() )
    {
        iterpp = iterp;
        iterp = iter++;
        inode++;
    }
    while ( iter != m_lsegments->end() )
    {
        a2dLineSegment* point0 = *iterpp;
        a2dLineSegment* point1 = *iterp;
        a2dLineSegment* point2 = *iter;

        double dx10 = point1->m_x - point0->m_x;
        double dy10 = point1->m_y - point0->m_y;
        double dx21 = point2->m_x - point1->m_x;
        double dy21 = point2->m_y - point1->m_y;

        double angle = atan2( dy10, dx10 ) - atan2( dy21, dx21 );
        
        if( point1->Length( *point0 ) > 0 && point2->Length( *point1 ) > 0 &&
            !point0->GetArc() && !point1->GetArc() && ( angle == wxPI || angle == -wxPI ) )
        {
            if( sendCommandsTo )
                m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( sendCommandsTo, inode - 1 ) );
            m_lsegments->erase( iterp );
        }
        else
        {
            inode++;
            iterpp = iterp;
        }
        iterp = iter++;
    }
}

a2dBoundingBox a2dPolylineL::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    if (  m_spline )
    {
        a2dVertexList* lpoints = new a2dVertexList();

        *lpoints = *m_lsegments;
        lpoints->ConvertIntoSplinedPolygon( double(GetHabitat()->GetAberPolyToArc()) / m_root->GetUnitsScale() );
        bbox = lpoints->GetBbox();
        delete lpoints;
    }
    else
        bbox = m_lsegments->GetBbox();

    if ( m_contourwidth > 0 )
        bbox.Enlarge( m_contourwidth / 2 );

    if ( m_begin )
    {
        a2dVertexList::iterator  iter = m_lsegments->begin();
        a2dPoint2D point1 = GetPosXY();
        a2dPoint2D point2 = GetPosXY();
        if ( iter != m_lsegments->end() )
        {
            point1 = ( *iter )->GetPoint();
            iter++;
            if (  iter != m_lsegments->end()  )
                point2 = ( *iter )->GetPoint();
            else
                point2 = point1;
        }

        double dx, dy;
        dx = point2.m_x - point1.m_x;
        dy = point2.m_y - point1.m_y;
        double ang1;
        if ( !dx && !dy )
            ang1 = 0;
        else
            ang1 = wxRadToDeg( atan2( dy, dx ) );

        a2dAffineMatrix lworld;
        //clockwise rotation so minus
        lworld.Scale( m_xscale, m_yscale, 0, 0 );
        lworld.Rotate( -ang1 );
        lworld.Translate( point1.m_x, point1.m_y );

        bbox.Expand( m_begin->GetMappedBbox( lworld ) );

    }

    if ( m_end )
    {
        double dx = 0;
        double dy = 0;
        a2dVertexList::reverse_iterator  iter = m_lsegments->rbegin();
        a2dPoint2D point1 = GetPosXY();
        a2dPoint2D point2 = GetPosXY();
        if ( iter != m_lsegments->rend() )
        {
            point1 = ( *iter )->GetPoint();
            while ( !dx && !dy && iter != m_lsegments->rend() )
            {
                point1 = ( *iter )->GetPoint();
                iter++;
                if ( iter != m_lsegments->rend() )
                    point2 = ( *iter )->GetPoint();
                else
                    point2 = point1;
                dx = point2.m_x - point1.m_x;
                dy = point2.m_y - point1.m_y;
            }
        }

        double ang2;
        if ( !dx && !dy )
            ang2 = 0;
        else
            ang2 = wxRadToDeg( atan2( dy, dx ) );

        a2dAffineMatrix tworld;
        //clockwise rotation so minus
        tworld.Scale( m_xscale, m_yscale, 0, 0 );
        tworld.Rotate( -ang2 );
        tworld.Translate( point1.m_x, point1.m_y );

        bbox.Expand( m_end->GetMappedBbox( tworld ) );

    }

    return bbox;
}

bool a2dPolylineL::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( m_begin )
    {
        calc = m_begin->Update( mode );
    }
    if ( m_end )
    {
        calc = m_end->Update( mode );
    }

    if ( !m_bbox.GetValid() || calc )
    {
        // first convert then transform is better here, else sometimes missing things.
        if (  m_spline )
        {
            a2dVertexList* lpoints = new a2dVertexList();

            *lpoints = *m_lsegments;
            lpoints->ConvertIntoSplinedPolygon( double(GetHabitat()->GetAberPolyToArc()) / m_root->GetUnitsScale() );
            m_bbox = lpoints->GetBbox( m_lworld );
            delete lpoints;
        }

        a2dBoundingBox bboxs = DoGetUnTransformedBbox();
        bboxs.MapBbox( m_lworld );
        m_bbox.Expand( bboxs );

        if ( m_flags.m_editingCopy && m_spline )
        {
            m_spline = false;
            a2dBoundingBox bboxs = DoGetUnTransformedBbox();
            bboxs.MapBbox( m_lworld );
            m_bbox.Expand( bboxs );
            m_spline = true;
        }
        m_shapeIdsValid = false;

        return true;
    }
    return false;
}

void a2dPolylineL::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( !m_shapeIdsValid )
    {
        ic.GetDrawer2D()->DeleteShapeIds( m_shapeIds );
        m_shapeIds = a2dListId();
    }

    if ( m_shapeIds.m_offset )
    {
        ic.GetDrawer2D()->DrawShape( m_shapeIds );
    }
    else
    {
        if ( m_contourwidth )
        {
            //if ( a2dGlobals->GetRoundFactor() == 0 && m_pathtype == a2dPATH_END_ROUND )
            if ( m_pathtype == a2dPATH_END_ROUND )
            {
                double AberArcToPoly = m_root->GetAberArcToPolyScaled();
                a2dVertexList contourpoints = *m_lsegments;
                contourpoints.ConvertToLines(AberArcToPoly);

                a2dLineSegment* segprev = NULL;
                a2dLineSegment* seg = NULL;
                a2dPoint2D first;
                a2dPoint2D second;
                a2dPoint2D offsetpoint;

                a2dVertexList::iterator iter = contourpoints.begin();
                while( iter != contourpoints.end() )
                {
                    a2dVertexArray poly;
                    a2dLineSegment* seg = *iter;
                    seg = ( *iter );

                    if ( segprev )
                    {
                        first = a2dPoint2D( segprev->m_x, segprev->m_y );
                        second = a2dPoint2D( seg->m_x, seg->m_y );

                        if ( first != second )
                        {
                            ic.GetDrawer2D()->DrawCircle( first.m_x, first.m_y, m_contourwidth / 2 );
                            a2dLine line1( first, second );
                            line1.CalculateLineParameters();
                            offsetpoint = first;
                            line1.Virtual_Point( offsetpoint, -m_contourwidth / 2 );
                            poly.push_back( new a2dLineSegment( offsetpoint ) );
                            offsetpoint = first;
                            line1.Virtual_Point( offsetpoint, m_contourwidth / 2 );
                            poly.push_back( new a2dLineSegment( offsetpoint ) );
                            offsetpoint = second;
                            line1.Virtual_Point( offsetpoint, m_contourwidth / 2 );
                            poly.push_back( new a2dLineSegment( offsetpoint ) );
                            offsetpoint = second;
                            line1.Virtual_Point( offsetpoint, -m_contourwidth / 2 );
                            poly.push_back( new a2dLineSegment( offsetpoint ) );
                            ic.GetDrawer2D()->DrawPolygon( &poly );
                        }
                    }
                    iter++;
                    segprev = seg;
                }
                ic.GetDrawer2D()->DrawCircle( second.m_x, second.m_y, m_contourwidth / 2 );
            }
            else
            {
                a2dVertexList* contourpoints = m_lsegments->ConvertToContour(  m_contourwidth / 2, m_pathtype );
                ic.GetDrawer2D()->DrawPolygon( contourpoints, m_spline );
                delete contourpoints;
            }
        }
        else
            ic.GetDrawer2D()->DrawLines( m_lsegments, m_spline );

        m_shapeIds = ic.GetDrawer2D()->GetShapeIds();
        m_shapeIdsValid = true;
    }

    if ( m_crossAtVertex )
    {
        a2dStroke current = ic.GetDrawer2D()->GetDrawerStroke();
        a2dFill fillcurrent = ic.GetDrawer2D()->GetDrawerFill();
        ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetHandle()->GetFill() );
        ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetHandle()->GetStroke() );

        double x, y;
        a2dAffineMatrix matrix = ic.GetDrawer2D()->GetUserToDeviceTransform();
        ic.GetDrawer2D()->PushIdentityTransform();

        a2dVertexList::iterator iter = m_lsegments->begin();
        a2dLineSegment* seg;
        while ( iter != m_lsegments->end() )
        {
            seg = ( *iter );
            x = seg->m_x;
            y = seg->m_y;
            double xd, yd;
            matrix.TransformPoint( x, y, xd, yd );

            ic.GetDrawer2D()->DrawCircle( xd, yd, a2dCrossAtVertexSize ); 
            iter++;
        }
	    ic.GetDrawer2D()->PopTransform();
        ic.GetDrawer2D()->SetDrawerStroke( current );
        ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
	}

    if ( m_flags.m_editingCopy )
    {
        ic.GetDrawer2D()->OverRuleFixedStyle();

        if ( m_spline )
        {
			a2dBoundingBox untrans = DoGetUnTransformedBbox();
			double x, y, w, h;
			x = untrans.GetMinX();
			y = untrans.GetMinY();
			w = untrans.GetWidth();
			h = untrans.GetHeight();

			ic.GetDrawer2D()->DrawLines( m_lsegments, false );

			a2dSmrtPtr<a2dRect> around = new a2dRect( x, y, w, h );
			around->SetIgnoreSetpending();
			around->SetRoot( m_root, false );
			around->Update( updatemask_force );
			around->SetStroke( *wxRED, 0, a2dSTROKE_DOT_DASH );
			around->SetFill( *a2dTRANSPARENT_FILL );
			around->Render( ic, _ON );
        }

        double x, y;
        double xp, yp;
        a2dVertexList::iterator iter = m_lsegments->begin();
        a2dLineSegment* seg;
        for ( int i = 0; i < m_lsegments->size()-1; i++ )
        {
            seg = ( *iter );
            if ( seg->GetBin() ) //selected?
            {
                x = seg->m_x;
                y = seg->m_y;
                a2dLineSegment* nextseg = *( m_lsegments->GetNextAround( iter ) );
                xp = nextseg->m_x;
                yp = nextseg->m_y;

                ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetSelectFill() );
                ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetSelectStroke() );
                ic.GetDrawer2D()->DrawLine( x, y, xp, yp ); 
            }
            iter++;
        }

        ic.GetDrawer2D()->ReStoreFixedStyle();
    }

    if ( m_begin )
    {
        a2dVertexList::iterator  iter = m_lsegments->begin();
        a2dPoint2D point1 = GetPosXY();
        a2dPoint2D point2 = GetPosXY();
        if ( iter != m_lsegments->end() )
        {
            point1 = ( *iter )->GetPoint();
            iter++;
            if ( iter != m_lsegments->end() )
                point2 = ( *iter )->GetPoint();
            else
                point2 = point1;
        }

        double dx, dy;
        dx = point2.m_x - point1.m_x;
        dy = point2.m_y - point1.m_y;
        double ang1;
        if ( !dx && !dy )
            ang1 = 0;
        else
            ang1 = wxRadToDeg( atan2( dy, dx ) );

        a2dAffineMatrix lworld;
        //clockwise rotation so minus
        lworld.Scale( m_xscale, m_yscale, 0, 0 );
        lworld.Rotate( -ang1 );
        lworld.Translate( point1.m_x, point1.m_y );

        a2dIterCU cu( ic, lworld );

        m_begin->Render( ic, clipparent );
    }

    if ( m_end )
    {
        double dx = 0;
        double dy = 0;
        a2dVertexList::reverse_iterator  iter = m_lsegments->rbegin();
        a2dPoint2D point1 = GetPosXY();
        a2dPoint2D point2 = GetPosXY();
        if ( iter != m_lsegments->rend() )
        {
            point1 = ( *iter )->GetPoint();
            while ( !dx && !dy && iter != m_lsegments->rend() )
            {
                point1 = ( *iter )->GetPoint();
                iter++;
                if ( iter != m_lsegments->rend() )
                    point2 = ( *iter )->GetPoint();
                else
                    point2 = point1;
                dx = point2.m_x - point1.m_x;
                dy = point2.m_y - point1.m_y;
            }
        }

        double ang2;
        if ( !dx && !dy )
            ang2 = 0;
        else
            ang2 = wxRadToDeg( atan2( dy, dx ) );

        a2dAffineMatrix tworld;
        //clockwise rotation so minus
        tworld.Scale( m_xscale, m_yscale, 0, 0 );
        tworld.Rotate( -ang2 );
        tworld.Translate( point1.m_x, point1.m_y );

        a2dIterCU cu( ic, tworld );

        m_end->Render( ic, clipparent );
    }
}

#if wxART2D_USE_CVGIO
void a2dPolylineL::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dPolygonL::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_pathtype != 0 )
            out.WriteAttribute( wxT( "pathtype" ), m_pathtype );
        if ( m_pathCornerType != a2dPATH_CORNER_SQAURE )
            out.WriteAttribute( wxT( "pathCornerType" ), m_pathCornerType );
        if ( m_begin || m_end  )
        {
            out.WriteAttribute( wxT( "xscale" ), m_xscale * out.GetScale()  );
            out.WriteAttribute( wxT( "yscale" ), m_yscale * out.GetScale()  );
            if ( m_begin )
                out.WriteAttribute( wxT( "begin" ), m_begin->GetId() );
            if ( m_end )
                out.WriteAttribute( wxT( "end" ), m_end->GetId() );
        }
    }
    else
    {
        if ( ( m_begin && !m_begin->GetCheck() ) ||
                ( m_end && !m_end->GetCheck() )
           )
        {
            //trick the system to have multiple refs on those objects, else we will not get an id attribute
            a2dCanvasObjectPtr begin = m_begin;
            a2dCanvasObjectPtr end = m_end;

            out.WriteStartElement( wxT( "derived" ) );

            if ( m_begin && !m_begin->GetCheck() )
                m_begin->Save( this, out, towrite );
            if ( m_end  && !m_end->GetCheck() )
                m_end->Save( this, out, towrite );

            out.WriteEndElement();
        }
    }
}

void a2dPolylineL::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dPolygonL::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_pathtype = (a2dPATH_END_TYPE) parser.GetAttributeValueInt( "pathtype" );
        m_pathCornerType = (a2dPATH_CORNER_TYPE) parser.GetAttributeValueInt( "pathCornerType" );

        m_xscale = parser.GetAttributeValueDouble( wxT( "xscale" ) );
        m_yscale = parser.GetAttributeValueDouble( wxT( "yscale" ) );

        wxString resolveKey;
        if ( parser.HasAttribute( wxT( "begin" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_begin, parser.GetAttributeValue( wxT( "begin" ) ) );
        }
        if ( parser.HasAttribute( wxT( "end" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_end, parser.GetAttributeValue( wxT( "end" ) ) );
        }
    }
    else
    {
        if ( ! GetFilled() ) // polyline is only filled when contourwidth != 0, which controls filled flag.
            RemoveProperty( PROPID_Fill ); //old files contains it.

        if ( parser.GetTagName() != wxT( "derived" ) )
            return;

        parser.Require( START_TAG, wxT( "derived" ) );
        parser.Next();

        m_begin = (a2dCanvasObject*) parser.LoadOneObject( this );
        m_end = (a2dCanvasObject*) parser.LoadOneObject( this );

        parser.Require( END_TAG, wxT( "derived" ) );
        parser.Next();
    }
}
#endif //wxART2D_USE_CVGIO

void a2dPolylineL::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dPolygonL::DoWalker( parent, handler );
    if ( m_begin )
        m_begin->Walker( this, handler );

    if ( m_end )
        m_end->Walker( this, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}

bool a2dPolylineL::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    a2dPoint2D P = a2dPoint2D( hitEvent.m_relx, hitEvent.m_rely );
    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

	double finalHitWidth = pw + margin + m_contourwidth / 2;

	if ( finalHitWidth < GetHabitat()->ACCUR() )
		finalHitWidth = GetHabitat()->ACCUR();

    hitEvent.m_how = PointOnPolyline( P, finalHitWidth );

    if( hitEvent.m_how.IsHit() )
    {
        //wxLogDebug(wxT(" hit at x=%12.6lf, y=%12.6lf"), P.m_x , P.m_y );
        return true;
    }
    bool ishit = false;

    if ( m_begin )
    {
        a2dVertexList::iterator  iter = m_lsegments->begin();
        a2dPoint2D point1;
        a2dPoint2D point2;
        if ( iter != m_lsegments->end() )
        {
            point1 = ( *iter )->GetPoint();
            iter++;
            if ( iter != m_lsegments->end() )
                point2 = ( *iter )->GetPoint();
            else
                point2 = point1;
        }

        double dx, dy;
        dx = point2.m_x - point1.m_x;
        dy = point2.m_y - point1.m_y;
        double ang1;
        if ( !dx && !dy )
            ang1 = 0;
        else
            ang1 = wxRadToDeg( atan2( dy, dx ) );

        a2dAffineMatrix lworld;
        //clockwise rotation so minus
        lworld.Scale( m_xscale, m_yscale, 0, 0 );
        lworld.Rotate( -ang1 );
        lworld.Translate( point1.m_x, point1.m_y );
        a2dIterCU cu( ic, lworld );
        if( m_begin->IsHitWorld( ic, hitEvent ) != 0 )
        {
            // A hit on the begin object is treated like a hit on the begin vertex
            // To distinguish it, hit_member is set instead of hit_stroke
            hitEvent.m_how.m_hit = hitEvent.m_how.m_hit | a2dHit::hit_member;
            hitEvent.m_how.m_stroke1 = a2dHit::stroke1_outside;
            hitEvent.m_how.m_stroke2 = a2dHit::stroke2_vertex;
            hitEvent.m_how.m_index = 0;
            ishit = true;
        }
    }

    if ( m_end && ( !ishit || ( hitEvent.m_option & a2dCANOBJHITOPTION_ALL ) ) )
    {
        a2dVertexList::reverse_iterator  iter = m_lsegments->rbegin();
        a2dPoint2D point1 = GetPosXY();
        a2dPoint2D point2 = GetPosXY();
        double dx = 0;
        double dy = 0;
        if ( iter != m_lsegments->rend() )
        {
            point1 = ( *iter )->GetPoint();
            while ( !dx && !dy && iter != m_lsegments->rend() )
            {
                point1 = ( *iter )->GetPoint();
                iter++;
                if ( iter != m_lsegments->rend() )
                    point2 = ( *iter )->GetPoint();
                else
                    point2 = point1;
                dx = point2.m_x - point1.m_x;
                dy = point2.m_y - point1.m_y;
            }
        }
        double ang2;
        if ( !dx && !dy )
            ang2 = 0;
        else
            ang2 = wxRadToDeg( atan2( dy, dx ) );

        a2dAffineMatrix tworld;
        //clockwise rotation so minus
        tworld.Scale( m_xscale, m_yscale, 0, 0 );
        tworld.Rotate( -ang2 );
        tworld.Translate( point1.m_x, point1.m_y );
        a2dIterCU cu( ic, tworld );
        if ( m_end->IsHitWorld( ic, hitEvent ) != 0 )
        {
            // A hit on the end object is treated like a hit on the end vertex
            hitEvent.m_how.m_hit = hitEvent.m_how.m_hit | a2dHit::hit_member;
            hitEvent.m_how.m_stroke1 = a2dHit::stroke1_outside;
            hitEvent.m_how.m_stroke2 = a2dHit::stroke2_vertex;
            hitEvent.m_how.m_index = m_lsegments->size() - 1;
            ishit = true;
        }
    }

    return hitEvent.m_how.IsHit();
}

a2dHit a2dPolylineL::PointOnPolyline( const a2dPoint2D& P, double margin )
{
    a2dVertexList* rlist;
    if (  m_spline )
    {
        a2dVertexList* lpoints = new a2dVertexList();
        *lpoints = *m_lsegments;
        double AberArcToPoly = m_root->GetAberArcToPolyScaled();
        lpoints->ConvertIntoSplinedPolyline( AberArcToPoly );
        rlist = lpoints;
    }
    else if ( m_lsegments->HasArcs() )
    {
        double AberArcToPoly = m_root->GetAberArcToPolyScaled();
        a2dVertexList* lpoints = new a2dVertexList();
        *lpoints = *m_lsegments;
        lpoints->ConvertToLines(AberArcToPoly);
        rlist = lpoints;
    }
    else
        rlist = m_lsegments;

    a2dHit result = rlist->HitTestPolyline( P, margin );

    if (  m_spline || m_lsegments->HasArcs() )
        delete rlist;

    return result;
}

bool a2dPolylineL::DoCanConnectWith( a2dIterC& ic, a2dPin* pin, double WXUNUSED( margin ), bool autocreate )
{
    RemovePins( true, true );

    //if pin is of this wire itself, we do not want it to connect.
    if( pin->GetParent() == this )
        return false;

    //the original has this property set
    if ( a2dCanvasObject::PROPID_Editcopy->GetPropertyValue( this ) )
        return false;

    if ( autocreate )
    {
        // Create dynamic pins
        // First test if it is a hit, it is useless to create pins not on the wire.
        a2dHitEvent hitevent = a2dHitEvent( pin->GetAbsX(), pin->GetAbsY(), false );
        if( IsHitWorld( ic, hitevent ) )
        {
            bool found = false;
            // generate pins which can connect this pin
            found = GeneratePins( pin->GetPinClass(), a2d_GeneratePinsForPinClass, pin->GetPosX(), pin->GetPosY() ) || found;
            return found;
        }
    }
    return false;
}

bool a2dPolylineL::GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y, double margin )
{
    a2dPinClass* toCreate;
    if ( GetGeneratePins() && ( toCreate = toConnectTo->GetPinClassForTask( task, this ) ) )
    {
        m_flags.m_visiblechilds = true;
        bool end = false;
        bool begin = false;
        if ( !HasPinNamed( wxT( "begin" ) ) )
        {
            a2dVertexList::iterator iter = m_lsegments->begin();
            assert( m_lsegments->size() );
            AddPin( wxT( "begin" ), ( *iter )->m_x, ( *iter )->m_y, a2dPin::temporaryObjectPin, toCreate );
        }
        else
            begin = true;

        if ( !HasPinNamed( wxT( "end" ) ) )
        {
            a2dVertexList::iterator iter = m_lsegments->end();
            assert( m_lsegments->size() );
            iter--;
            AddPin( wxT( "end" ), ( *iter )->m_x, ( *iter )->m_y, a2dPin::temporaryObjectPin, toCreate );
        }
        else
            end = true;

        if ( /*begin && end &&*/ m_lsegments->size() > 0 )
        {
            // Find the rastered point on the line closest to the polygon line
            double xNear, yNear;
            if( FindNearPoint( &m_lworld, x, y, &xNear, &yNear ) )
            {
                a2dAffineMatrix inverse = m_lworld;
                inverse.Invert();
                double xNearLocal, yNearLocal;
                inverse.TransformPoint( xNear, yNear, xNearLocal, yNearLocal );

                //now we create a dynamic pin of the same class as a connected pin.
                a2dPin* pin = AddPin( wxT( "dynamic" ), xNearLocal, yNearLocal, a2dPin::temporaryObjectPin | a2dPin::dynamic, toCreate );
                pin->SetMode( a2dPin::sm_PinCanConnectToPinClass );
                pin->SetMouseInObject( true );
            }
        }

        wxASSERT( HasPins() );
        m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dPin" ) );

        return true;
    }
    return false;
}

bool a2dPolylineL::FindNearPoint( const a2dAffineMatrix* cworld, double xIn, double yIn, double* xOut, double* yOut )
{
    if( GetNumberOfSegments() < 2 )
        return false;

    a2dAffineMatrix tworld;
    if( cworld )
    {
        tworld = *cworld;
        tworld *= m_lworld;
    }
    a2dAffineMatrix inverse = tworld;
    inverse.Invert();

    double xh, yh;
    inverse.TransformPoint( xIn, yIn, xh, yh );

    double dist;
    double minDist = DBL_MAX;

    a2dPoint2D p = a2dPoint2D( xh, yh );
    a2dPoint2D p1, p2, pBest;

    // If the polyline is splined, first convert the vertex list to a linearized spline
    a2dVertexList* rlist;
    if( m_spline )
        rlist = m_lsegments->ConvertSplinedPolyline( 10 );
    else
        rlist = m_lsegments;

    // Find the closest point on the line
    a2dVertexList::iterator iter = rlist->begin();
    p2 = ( *iter )->GetPoint();
    while( iter != rlist->end() )
    {
        p1 = p2;
        iter++;
        if ( iter == rlist->end() ) break;
        p2 = ( *iter )->GetPoint();

        a2dLine line( p1, p2 );
        a2dPoint2D pBestLine;
        dist = line.PointDistance( p, &pBestLine );
        if( dist < minDist )
        {
            minDist = dist;
            pBest = pBestLine;
        }
    }

    // Clean up the linearized vertex list
    if (  m_spline )
        delete rlist;

    // Transform the point back
    tworld.TransformPoint( pBest.m_x, pBest.m_y, *xOut, *yOut );

    return true;
}

bool a2dPolylineL::FindNearPointOrtho( const a2dAffineMatrix* cworld, double xIn, double yIn, double* xOut, double* yOut )
{
    if( GetNumberOfSegments() < 2 )
        return false;

    bool result = false;

    a2dAffineMatrix tworld;
    if( cworld )
    {
        tworld = *cworld;
        tworld *= m_lworld;
    }
    a2dAffineMatrix inverse = tworld;
    inverse.Invert();

    double xh, yh;
    inverse.TransformPoint( xIn, yIn, xh, yh );

    double dist;
    double minDist = DBL_MAX;

    a2dPoint2D p = a2dPoint2D( xh, yh );
    a2dPoint2D p1, p2, pBest;

    // If the polyline is splined, first convert the vertex list to a linearized spline
    a2dVertexList* rlist;
    if( m_spline )
        rlist = m_lsegments->ConvertSplinedPolyline( 10 );
    else
        rlist = m_lsegments;

    // Find the closest point on the line
    a2dVertexList::iterator iter = rlist->begin();
    p2 = ( *iter )->GetPoint();
    while( iter != rlist->end() )
    {
        p1 = p2;
        iter++;
        if ( iter == rlist->end() ) break;
        p2 = ( *iter )->GetPoint();

        a2dLine line( p1, p2 );
        a2dPoint2D pBestLine;
        dist = line.PointDistanceOrhto( p, &pBestLine );
        if( dist < minDist )
        {
            minDist = dist;
            pBest = pBestLine;
            result = true;
        }
    }

    // Clean up the linearized vertex list
    if (  m_spline )
        delete rlist;

    // Transform the point back
    tworld.TransformPoint( pBest.m_x, pBest.m_y, *xOut, *yOut );

    return result;
}

bool a2dPolylineL::MoveDynamicPinCloseTo( a2dPin* pin, const a2dPoint2D& point, bool final )
{
    wxASSERT( pin );
    wxASSERT( pin->IsDynamicPin() );
    wxASSERT( pin->GetParent() == this );

    a2dPoint2D pointLocal;
    a2dAffineMatrix trns = pin->GetParent()->GetTransformMatrix();
    trns.Invert();
    trns.TransformPoint( point, &pointLocal );

    double xNew, yNew, xNewO, yNewO;
    bool orthook = FindNearPointOrtho( 0, pointLocal.m_x, pointLocal.m_y, &xNewO, &yNewO );
    bool straightok = FindNearPoint( 0, pointLocal.m_x, pointLocal.m_y, &xNew, &yNew );

    if(
        orthook && PointDistSqr( pointLocal.m_x, pointLocal.m_y, xNewO, yNewO ) < PointDistSqr( pointLocal.m_x, pointLocal.m_y, xNew, yNew ) * 4 ||
        orthook && !straightok
    )
    {
        xNew = xNewO;
        yNew = yNewO;
    }

    a2dPoint2D pos = pin->GetPosXY();

    if( ( xNew != pos.m_x || yNew != pos.m_y  ) && ( orthook || straightok ) )
    {
        if ( final )
            m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( pin, PROPID_Position, a2dPoint2D( xNew, yNew ), -1, false ), final );
        else
            pin->SetPosXY( xNew, yNew ); 
        return true;
    }
    return false;
}

a2dPin* a2dPolylineL::FindBeginPin()
{
    return HasPinNamed( wxT( "begin" ) );
}

a2dPin* a2dPolylineL::FindEndPin()
{
    return HasPinNamed( wxT( "end" ) );
}

a2dPin* a2dPolylineL::FindPin( int* i )
{
    switch( *i )
    {
        case 0:
        {
            a2dPin* pin = FindBeginPin();
            ( *i )++;
            if( pin ) return pin;
        }
        // fall through if no pin found
        case 1:
        {
            a2dPin* pin = FindEndPin();
            ( *i )++;
            if( pin ) return pin;
        }
        // fall through if no pin found
        default:
            return 0;
    }
}

int a2dPolylineL::FindSegmentIndex( const a2dPoint2D& point, double margin )
{
    a2dHit hit = m_lsegments->HitTestPolyline( point, margin );
    
    if ( hit.IsHit() )
    {
        return hit.m_index;
    }
    return -1;
}

int a2dPolylineL::FindPinSegmentIndex( a2dPin* pinToFind, double margin )
{
    a2dHit hit = m_lsegments->HitTestPolyline( pinToFind->GetPosXY(), margin );
    
    if ( hit.IsHit() )
    {
        return hit.m_index;
    }
    return -1;
}

a2dLineSegment* a2dPolylineL::FindPinSegment( a2dPin* pinToFind, double margin )
{
    a2dHit hit = m_lsegments->HitTestPolyline( pinToFind->GetPosXY(), margin );
    
    if ( hit.IsHit() )
    {
        a2dVertexList::iterator iter = GetSegmentAtIndex( hit.m_index );
        return *iter;
    }
    return NULL;
}

a2dLineSegment* a2dPolylineL::GetFirstPoint() const
{
    a2dVertexList::const_iterator iter = m_lsegments->begin();
    if ( iter != m_lsegments->end() )
        return iter->Get();
    return NULL;
}

a2dLineSegment* a2dPolylineL::GetLastPoint() const
{
    if ( m_lsegments->size() )
    {
        a2dVertexList::const_iterator iter = m_lsegments->end();
        iter--;
        return iter->Get();
    }
    return NULL;
}

void a2dPolylineL::OnChar( wxKeyEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_DELETE:
            {
				if ( m_lsegments->size() <= 2 )
				{
					wxLogWarning( _( "stopped removing segment or vertexes, object should keep at least one segment" ) );
                    break; // leaf at least one segment
				}

                a2dPolygonL* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dPolygonL );
                //first delete selected segments ( nodes maybe selcted, and those nodes maybe removed because of this. (notice --m_childobjects->end())
                for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != --m_childobjects->end(); ++iter )
                {
                    if ( m_lsegments->size() <= 2 )
                    {
                        wxLogWarning( _( "stopped removing segment or vertexes, object should keep at least one segment" ) );
                        break; // leaf at least one segment
                    }
                    a2dCanvasObject* obj = *iter;

                    a2dPolyHandleL* handle = wxDynamicCast( obj, a2dPolyHandleL );

                    if ( handle && !handle->GetRelease( ) )
                    {
                        a2dVertexList::iterator iterpoly = handle->GetNode();
                        a2dLineSegment* segment = *( iterpoly );
                        if ( segment->GetBin() ) //selected?
                        {
                            iter++;
                            a2dPolyHandleL* handle2 = wxDynamicCast( (*iter).Get(), a2dPolyHandleL );
                            iter--;
                            a2dLineSegment* segmentOrgNext = *( handle2->GetNodeOrg() );
                            a2dLineSegment* segmentOrg = *( handle->GetNodeOrg() );

                            double xn,yn;
                            xn = (handle->GetPosX() + handle2->GetPosX())/2.0;
                            yn = (handle->GetPosY() + handle2->GetPosY())/2.0;
                            handle2->SetPosXY( xn, yn );
                            m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( original, segmentOrg ) );
                            m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveSegment( original, xn, yn, segmentOrgNext, false ) );
                            iterpoly = m_lsegments->erase( iterpoly );
                            (*iterpoly)->m_x = xn; (*iterpoly)->m_y = yn;
                            ReleaseChild( handle );
                        }
                    }
                }

                //now delete selected handles (only those which were not released above).
                for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
                {
                    a2dCanvasObject* obj = *iter;

                    a2dPolyHandleL* handle = wxDynamicCast( obj, a2dPolyHandleL );

                    if ( handle &&  !handle->GetRelease( ) && handle->GetSelected() )
                    {
                        a2dVertexList::iterator iterpoly = handle->GetNode();
                        a2dLineSegment* segment = *( iterpoly );
                        a2dVertexList::iterator iterorg = handle->GetNodeOrg();
                        a2dLineSegment* segmentOrg = *( iterorg );

						if ( m_lsegments->size() <= 2 )
						{
							wxLogWarning( _( "stopped removing segment or vertexes, object should keep at least one segment" ) );
						    break; // leaf at least one segment
						}
						else
						{
							m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( original, segmentOrg ) );
							m_lsegments->erase( iterpoly );
							ReleaseChild( handle );
						}
                    }
                }
                original->AdjustAfterChange( true );
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();
}

//----------------------------------------------------------------------------
// a2dPolygonLClipper
//----------------------------------------------------------------------------

a2dPolygonLClipper::a2dPolygonLClipper( a2dBoundingBox& bbox )
    : a2dPolygonL()
{
    if ( &bbox != &wxNonValidBbox )
    {
        AddPoint( bbox.GetMinX(), bbox.GetMinY() );
        AddPoint( bbox.GetMinX(), bbox.GetMaxY() );
        AddPoint( bbox.GetMaxX(), bbox.GetMaxY() );
        AddPoint( bbox.GetMaxX(), bbox.GetMinY() );
    }

    m_spline = false;
}

a2dPolygonLClipper::a2dPolygonLClipper( a2dVertexList* points, bool spline )
    : a2dPolygonL( points, spline )
{
}

a2dPolygonLClipper::~a2dPolygonLClipper()
{
}

void a2dPolygonLClipper::SetClippingFromBox( a2dBoundingBox& bbox )
{
    Clear();
    if ( &bbox != &wxNonValidBbox )
    {
        AddPoint( bbox.GetMinX(), bbox.GetMinY() );
        AddPoint( bbox.GetMinX(), bbox.GetMaxY() );
        AddPoint( bbox.GetMaxX(), bbox.GetMaxY() );
        AddPoint( bbox.GetMaxX(), bbox.GetMinY() );
    }
}

a2dObject* a2dPolygonLClipper::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dPolygonLClipper( *this, options, refs );
};

a2dPolygonLClipper::a2dPolygonLClipper( const a2dPolygonLClipper& other, CloneOptions options, a2dRefMap* refs )
    : a2dPolygonL( other, options, refs )
{
}

void a2dPolygonLClipper::Render( a2dIterC& ic, OVERLAP clipparent )
{
    if ( m_lsegments->size() )
    {
        a2dIterCU cu( ic, m_ClipLworld );

        //if ( m_flags.m_editingCopy )
        ic.GetDrawer2D()->DrawPolygon( m_lsegments, m_spline, wxWINDING_RULE );
        ic.GetDrawer2D()->ExtendAndPushClippingRegion( m_lsegments, m_spline, wxWINDING_RULE, a2dCLIP_AND );
    }

    a2dCanvasObject::Render( ic, clipparent );

    if ( m_lsegments->size() )
        ic.GetDrawer2D()->PopClippingRegion();
}

bool a2dPolygonLClipper::Update( UpdateMode mode )
{
    a2dCanvasObject::Update( mode );

    m_bbox = DoGetUnTransformedBbox();
    m_bbox.MapBbox( m_ClipLworld );

    if ( m_flags.m_editingCopy && m_spline )
    {
        m_spline = false;
        a2dBoundingBox bboxs = DoGetUnTransformedBbox();
        bboxs.MapBbox( m_ClipLworld );
        m_bbox.Expand( bboxs );
        m_spline = true;
    }

    return false;
}

void a2dPolygonLClipper::DoRender( a2dIterC& WXUNUSED( ic ), OVERLAP WXUNUSED( clipparent ) )
{
    if ( m_flags.m_editingCopy )
    {
        //ic.GetDrawer2D()->DrawPolygon( m_lsegments, m_spline, wxWINDING_RULE);
    }
}

void a2dPolygonLClipper::SetClippingTransformMatrix( const a2dAffineMatrix& mat )
{
    m_ClipLworld = mat; SetPending( true );
}

//----------------------------------------------------------------------------
// a2dPolygonLClipper2
//----------------------------------------------------------------------------

a2dPolygonLClipper2::a2dPolygonLClipper2( a2dBoundingBox& bbox )
    : a2dPolygonL()
{
    if ( &bbox != &wxNonValidBbox )
    {
        AddPoint( bbox.GetMinX(), bbox.GetMinY() );
        AddPoint( bbox.GetMinX(), bbox.GetMaxY() );
        AddPoint( bbox.GetMaxX(), bbox.GetMaxY() );
        AddPoint( bbox.GetMaxX(), bbox.GetMinY() );
    }

    m_spline = false;
}

a2dPolygonLClipper2::a2dPolygonLClipper2( a2dVertexList* points, bool spline )
    : a2dPolygonL( points, spline )
{
}

a2dPolygonLClipper2::~a2dPolygonLClipper2()
{
}

a2dObject* a2dPolygonLClipper2::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dPolygonLClipper2( *this, options, refs );
};

a2dPolygonLClipper2::a2dPolygonLClipper2( const a2dPolygonLClipper2& other, CloneOptions options, a2dRefMap* refs )
    : a2dPolygonL( other, options, refs )
{
}

void a2dPolygonLClipper2::Render( a2dIterC& ic, OVERLAP clipparent )
{
    if ( m_lsegments->size() )
    {
        a2dIterCU cu( ic, this );

        if ( !m_flags.m_editingCopy )
            ic.GetDrawer2D()->ExtendAndPushClippingRegion( m_lsegments, m_spline, wxWINDING_RULE, a2dCLIP_AND );
    }

    a2dCanvasObject::Render( ic, clipparent );

    if ( m_lsegments->size() )
        if ( !m_flags.m_editingCopy )
            ic.GetDrawer2D()->PopClippingRegion();
}

bool a2dPolygonLClipper2::Update( UpdateMode mode )
{
    a2dCanvasObject::Update( mode );

    m_bbox = DoGetUnTransformedBbox();
    m_bbox.MapBbox( m_lworld );

    if ( m_flags.m_editingCopy && m_spline )
    {
        m_spline = false;
        a2dBoundingBox bboxs = DoGetUnTransformedBbox();
        bboxs.MapBbox( m_lworld );
        m_bbox.Expand( bboxs );
        m_spline = true;
    }

    return false;
}

void a2dPolygonLClipper2::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    //if ( m_flags.m_editingCopy )
    {
        ic.GetDrawer2D()->DrawPolygon( m_lsegments, m_spline, wxWINDING_RULE );
    }
}

//----------------------------------------------------------------------------
// a2dPolyHandleL
//----------------------------------------------------------------------------

a2dPropertyIdDouble* a2dPolyHandleL::PROPID_tmpXIH = NULL;
a2dPropertyIdDouble* a2dPolyHandleL::PROPID_tmpYIH = NULL;
a2dPropertyIdDouble* a2dPolyHandleL::PROPID_tmpXIV1 = NULL;
a2dPropertyIdDouble* a2dPolyHandleL::PROPID_tmpYIV1 = NULL;
a2dPropertyIdDouble* a2dPolyHandleL::PROPID_tmpXIV2 = NULL;
a2dPropertyIdDouble* a2dPolyHandleL::PROPID_tmpYIV2 = NULL;

INITIALIZE_PROPERTIES( a2dPolyHandleL, a2dHandle )
{
    A2D_PROPID_D_F( a2dPropertyIdDouble, tmpXIH, 0, a2dPropertyId::flag_temporary );
    A2D_PROPID_D_F( a2dPropertyIdDouble, tmpYIH, 0, a2dPropertyId::flag_temporary );
    A2D_PROPID_D_F( a2dPropertyIdDouble, tmpXIV1, 0, a2dPropertyId::flag_temporary );
    A2D_PROPID_D_F( a2dPropertyIdDouble, tmpYIV1, 0, a2dPropertyId::flag_temporary );
    A2D_PROPID_D_F( a2dPropertyIdDouble, tmpXIV2, 0, a2dPropertyId::flag_temporary );
    A2D_PROPID_D_F( a2dPropertyIdDouble, tmpYIV2, 0, a2dPropertyId::flag_temporary );
    return true;
}

a2dPolyHandleL::a2dPolyHandleL()
    : a2dHandle( NULL, 0, 0, wxT( "dummy" ), GetHabitat()->GetHandle()->GetWidth(), GetHabitat()->GetHandle()->GetHeight() )
{
    m_arcHandle = false;
    m_dragging = true;
    m_lsegments = NULL;
    m_lsegmentsOrg = NULL;
}

a2dPolyHandleL::a2dPolyHandleL(  a2dPolygonL* parent, a2dVertexList::iterator segNode, a2dVertexList::iterator segNodeOrg,
                                 a2dSmrtPtr<a2dVertexList> lsegments,
                                 a2dSmrtPtr<a2dVertexList> lsegmentsOrg,
                                 const wxString& name )
    : a2dHandle( parent, ( *segNode )->m_x, ( *segNode )->m_y, name )
{
    m_segNode = segNode;
    m_segNodeOrg = segNodeOrg;
    m_lsegments = lsegments;
    m_lsegmentsOrg = lsegmentsOrg;
    m_arcHandle = false;
    m_dragging = false;
}

a2dPolyHandleL::a2dPolyHandleL(  a2dPolygonL* parent, a2dVertexList::iterator segNode, a2dVertexList::iterator segNodeOrg,
                                 a2dSmrtPtr<a2dVertexList> lsegments,
                                 a2dSmrtPtr<a2dVertexList> lsegmentsOrg,
                                 double xc, double yc, const wxString& name )
    : a2dHandle( parent, xc, yc, name )
{
    m_segNode = segNode;
    m_segNodeOrg = segNodeOrg;
    m_lsegments = lsegments;
    m_lsegmentsOrg = lsegmentsOrg;
    m_arcHandle = false;
    m_dragging = false;
}

a2dPolyHandleL::a2dPolyHandleL( const a2dPolyHandleL& other, CloneOptions options, a2dRefMap* refs )
    : a2dHandle( other, options, refs )
{
    m_segNode = other.m_segNode;
    m_segNodeOrg = other.m_segNodeOrg;
    m_arcHandle = other.m_arcHandle;
    m_lsegments = other.m_lsegments;
    m_lsegmentsOrg = other.m_lsegmentsOrg;
    m_dragging = false;
}

a2dPolyHandleL::~a2dPolyHandleL()
{
}

a2dObject* a2dPolyHandleL::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dPolyHandleL( *this, options, refs );
};

//----------------------------------------------------------------------------
// a2dSurface
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dSurface, a2dPolygonL )

BEGIN_EVENT_TABLE( a2dSurface, a2dPolygonL )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dSurface::OnCanvasObjectMouseEvent )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dSurface::OnHandleEvent )
    EVT_CHAR( a2dSurface::OnChar )
END_EVENT_TABLE()

a2dSurface::a2dSurface()
    : a2dPolygonL()
{
    m_lsegments = new a2dVertexList();
}

a2dSurface::a2dSurface( a2dVertexListPtr points, bool spline  )
    : a2dPolygonL( points, spline )
{
}

a2dSurface::~a2dSurface()
{
}

void a2dSurface::Clear()
{
    m_lsegments->clear();
    m_holes.clear();
}

a2dSurface::a2dSurface( const a2dSurface& other, CloneOptions options, a2dRefMap* refs )
    : a2dPolygonL( other, options, refs )
{
    for( a2dListOfa2dVertexList::const_iterator iterp = other.m_holes.begin(); iterp != other.m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        a2dVertexListPtr holepoints = new a2dVertexList();
        *holepoints = *vlist;
        m_holes.push_back( holepoints );

    }
}

a2dObject* a2dSurface::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dSurface( *this, options, refs );
};

a2dVertexList* a2dSurface::GetAsVertexList( bool& returnIsPolygon ) const
{
    return NULL;
}

a2dHit a2dSurface::PointInPolygon( const a2dPoint2D& P, double marge )
{
    a2dHit result = a2dPolygonL::PointInPolygon( P, marge );

    if ( result.IsHit() )
    {
        for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
        {
            a2dVertexListPtr vlist = ( *iterp );
            a2dHit holeHit = vlist->HitTestPolygon( P, marge );
            if ( holeHit.IsHit() )
            {
                a2dHit noHit;
                return  noHit;
            }
        }
    }
    return result;
}

a2dCanvasObjectList* a2dSurface::GetAsPolygons( bool transform ) const
{
    a2dCanvasObjectList* polygons = new a2dCanvasObjectList();

    a2dVertexList* conSegm = new a2dVertexList( *(m_lsegments.Get()) );
    a2dPolygonL* contour = new a2dPolygonL( new a2dVertexList( *(m_lsegments.Get()) ) ); 
    contour->SetStroke( this->GetStroke() );
    contour->SetFill( this->GetFill() );
    contour->SetLayer( m_layer );
    contour->SetContourWidth( GetContourWidth() );
    contour->SetRoot( m_root, false );
    if ( transform )
        contour->EliminateMatrix();
    polygons->push_back( contour );

    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        a2dPolygonL* hole = new a2dPolygonL( new a2dVertexList( *(vlist.Get()) ) ); 
        hole->SetStroke( this->GetStroke() );
        hole->SetFill( this->GetFill() );
        hole->SetLayer( m_layer );
        hole->SetContourWidth( GetContourWidth() );
        hole->SetRoot( m_root, false );
        if ( hole )
            hole->EliminateMatrix();
        polygons->push_back( hole );
    }
    return polygons;
}

a2dCanvasObjectList* a2dSurface::GetAsPolylines( bool transform ) const
{
    a2dCanvasObjectList* polygons = new a2dCanvasObjectList();

    a2dVertexList* conSegm = new a2dVertexList( *(m_lsegments.Get()) );
    conSegm->push_back( conSegm->front()->Clone() );
    a2dPolylineL* contour = new a2dPolylineL( conSegm ); 
    contour->SetStroke( this->GetStroke() );
    contour->SetFill( this->GetFill() );
    contour->SetLayer( m_layer );
    contour->SetContourWidth( GetContourWidth() );
    contour->SetRoot( m_root, false );
    if ( transform )
        contour->EliminateMatrix();
    polygons->push_back( contour );

    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        a2dVertexListPtr vlistcopy = new a2dVertexList( *(vlist.Get()) );
        vlistcopy->push_back( vlistcopy->front()->Clone() );
        a2dPolylineL* hole = new a2dPolylineL( vlistcopy ); 
        hole->SetStroke( this->GetStroke() );
        hole->SetFill( this->GetFill() );
        hole->SetLayer( m_layer );
        hole->SetContourWidth( GetContourWidth() );
        hole->SetRoot( m_root, false );
        if ( hole )
            hole->EliminateMatrix();
        polygons->push_back( hole );
    }
    return polygons;
}

a2dCanvasObjectList* a2dSurface::GetAsCanvasVpaths( bool transform ) const
{
    a2dVpath* segments = m_lsegments->ConvertToVpath( true, true );

    a2dVpathSegment* lastseg = segments->back();
    lastseg->SetClose( a2dPATHSEG_END_OPEN );

    a2dVpathSegment* segvec = NULL;
    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        if ( vlist->size() > 1 )
        {
            a2dVertexList::iterator iter = vlist->begin();
            a2dLineSegmentPtr seg = *iter;
            segvec = new a2dVpathSegment( seg->m_x, seg->m_y, a2dPATHSEG_LINETO_NOSTROKE );
            segments->push_back( segvec );

            while ( iter != vlist->end() )
            {
                a2dLineSegmentPtr seg = *iter;
                segvec = new a2dVpathSegment( seg->m_x, seg->m_y, a2dPATHSEG_LINETO );
                segments->push_back( segvec );
                iter++;
            }
            seg = *( vlist->begin() );
            segvec = new a2dVpathSegment( seg->m_x, seg->m_y, a2dPATHSEG_LINETO );
            segments->push_back( segvec );
            segvec = new a2dVpathSegment( lastseg->m_x1, lastseg->m_y1, a2dPATHSEG_LINETO_NOSTROKE );
            segments->push_back( segvec );
        }
    }
    if ( segvec )
        segvec->SetClose( a2dPATHSEG_END_CLOSED );

    a2dVectorPath* canpath = new a2dVectorPath( segments );
    if ( transform )
        segments->Transform( m_lworld );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );
    canpath->SetContourWidth( GetContourWidth() );
    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );

    return canpathlist;
}

a2dCanvasObjectList* a2dSurface::GetAsLinesArcs( bool transform )
{
    a2dCanvasObjectList* linesAnArcs = new a2dCanvasObjectList();

    double xp = m_lsegments->back()->m_x;
    double yp = m_lsegments->back()->m_y;
    if ( transform )
        m_lworld.TransformPoint( xp, yp, xp, yp );
    a2dCanvasObject* obj;
    for( a2dVertexList::iterator iter = m_lsegments->begin(); iter != m_lsegments->end(); ++iter )
    {
        double x, y, xm, ym;
        a2dLineSegment* seg = *iter;
        x = seg->m_x; y = seg->m_y;
        if ( transform )
            m_lworld.TransformPoint( x, y, x, y );
        if ( seg->GetArc() )
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;
            xm = cseg->m_x2; ym = cseg->m_y2;
            if ( transform )
                m_lworld.TransformPoint( xm, ym, xm, ym );
            a2dArc* arc = new a2dArc();
            arc->SetChord( true );
            obj = arc;
            arc->Set( xp, yp, xm, ym, x, y );
            linesAnArcs->push_back( arc );
        }
        else
        {
            a2dSLine* line = new a2dSLine( xp, yp, x, y );
            obj = line;
            linesAnArcs->push_back( line );
        }

        obj->SetStroke( this->GetStroke() );
        obj->SetFill( this->GetFill() );
        obj->SetLayer( m_layer );
        obj->SetContourWidth( GetContourWidth() );
        obj->SetRoot( m_root, false );

        xp = x;
        yp = y;
    }

    return linesAnArcs;
}

void a2dSurface::AddHole( a2dVertexListPtr holepoints )
{
    m_holes.push_back( holepoints );
}

void a2dSurface::RemoveRedundantPoints( a2dCanvasObject* sendCommandsTo )
{
    a2dPolygonL::RemoveRedundantPoints( sendCommandsTo );
}

bool a2dSurface::EliminateMatrix()
{
    a2dPolygonL::EliminateMatrix();
    return a2dCanvasObject::EliminateMatrix();
}

a2dBoundingBox a2dSurface::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    if (  m_spline )
    {
        a2dVertexList* lpoints = new a2dVertexList();

        *lpoints = *m_lsegments;
        lpoints->ConvertIntoSplinedPolygon( double(GetHabitat()->GetAberPolyToArc()) / m_root->GetUnitsScale() );
        bbox = lpoints->GetBbox();
        delete lpoints;
    }
    else
        bbox = m_lsegments->GetBbox();

    if ( m_contourwidth > 0 )
        bbox.Enlarge( m_contourwidth / 2 );

    return bbox;
}

bool a2dSurface::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        if (  m_spline )
        {
            a2dVertexList* lpoints = new a2dVertexList();

            *lpoints = *m_lsegments;
            lpoints->ConvertIntoSplinedPolygon( double(GetHabitat()->GetAberPolyToArc()) / m_root->GetUnitsScale() );
            m_bbox = lpoints->GetBbox( m_lworld );
            delete lpoints;
        }

        a2dBoundingBox bboxs = DoGetUnTransformedBbox();
        bboxs.MapBbox( m_lworld );
        m_bbox.Expand( bboxs );

        if ( m_flags.m_editingCopy && m_spline )
        {
            m_spline = false;
            a2dBoundingBox bboxs = DoGetUnTransformedBbox();
            bboxs.MapBbox( m_lworld );
            m_bbox.Expand( bboxs );
            m_spline = true;
        }
        return true;
    }
    return false;
}

void a2dSurface::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    // x windows requires last point to be eqaul to first for polypolygons.
    a2dListOfa2dVertexList drawable;
    a2dLineSegmentPtr last = m_lsegments->front();
    m_lsegments->push_back( last );
    drawable.push_back( m_lsegments );
    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        if ( vlist->size() > 1 )
        {
            a2dLineSegmentPtr last = vlist->front();
            vlist->push_back( last );
            drawable.push_back( vlist );
        }
    }
    ic.GetDrawer2D()->DrawPolyPolygon( drawable, wxWINDING_RULE );
    m_lsegments->pop_back();
    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        if ( vlist->size() > 1 )
        {
            vlist->pop_back();
        }
    }
}

#if wxART2D_USE_CVGIO
void a2dSurface::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dPolygonL::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "holes" ), m_holes.size() );
    }
    else
    {
        unsigned int i = 0;
        for( a2dListOfa2dVertexList::iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
        {
            out.WriteStartElement( wxT( "hole" ) );
            forEachIn( a2dVertexList, ( *iterp ).Get() )
            {
                if ( i % 6 == 0 )
                    out.WriteNewLine();
                a2dLineSegment* seg = *iter;
                out.WriteStartElementAttributes( wxT( "xy" ) );
                out.WriteAttribute( wxT( "x" ), seg->m_x * out.GetScale()  );
                out.WriteAttribute( wxT( "y" ), seg->m_y * out.GetScale()  );

                if ( seg->GetArc() )
                {
                    a2dArcSegment* arc = ( a2dArcSegment* ) seg;
                    out.WriteAttribute( wxT( "xm" ), arc->m_x2 * out.GetScale()  );
                    out.WriteAttribute( wxT( "ym" ), arc->m_y2 * out.GetScale()  );
                }

                out.WriteEndAttributes( true );
                i++;
            }
            out.WriteEndElement();
        }

    }
}

void a2dSurface::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dPolygonL::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        int holes = parser.GetAttributeValueBool( wxT( "holes" ), false );
    }
    else
    {
        while( parser.GetTagName() == wxT( "hole" )  )
        {
            a2dVertexListPtr holepoints = new a2dVertexList();
            parser.Next();
            while( parser.GetTagName() == wxT( "xy" )  )
            {
                double x = parser.GetAttributeValueDouble( wxT( "x" ) ) * parser.GetScale() ;
                double y = parser.GetAttributeValueDouble( wxT( "y" ) ) * parser.GetScale() ;

                a2dLineSegment* seg;
                if ( parser.HasAttribute( wxT( "xm" ) ) )
                {
                    double xm = parser.GetAttributeValueDouble( wxT( "xm" ) ) * parser.GetScale() ;
                    double ym = parser.GetAttributeValueDouble( wxT( "ym" ) ) * parser.GetScale() ;

                    seg = new a2dArcSegment( x, y, xm, ym );
                }
                else
                    seg = new a2dLineSegment( x, y );

                holepoints->push_back( seg );

                parser.Next();
                parser.Require( END_TAG, wxT( "xy" ) );
                parser.Next();

            }
            parser.Require( END_TAG, wxT( "hole" ) );
            parser.Next();
            AddHole( holepoints );
        }
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dSurface::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double xh, yh;
    ic.GetInverseTransform().TransformPoint( hitEvent.m_x, hitEvent.m_y, xh, yh );

    a2dPoint2D P = a2dPoint2D( xh, yh );
    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

    unsigned int indexn = m_lsegments->size() + 1;
    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        hitEvent.m_how = vlist->HitTestPolygon( P, margin );
        if ( hitEvent.m_how.IsHit() )
        {
            hitEvent.m_how.m_index += indexn;
            return  true;
        }
        indexn += vlist->size();
        indexn++; // dummy index for end
    }
    if ( !hitEvent.m_how.IsHit() )
        hitEvent.m_how = PointInPolygon( P, pw + margin );
    return hitEvent.m_how.IsHit();
}

bool a2dSurface::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    a2dPolygonL::DoStartEdit( editmode, editstyle );
    if ( m_flags.m_editable )
    {
        if ( editmode == 1 )
        {
            if( ! ( editstyle & wxEDITSTYLE_NOHANDLES ) )
            {
                m_flags.m_visiblechilds = true;
                m_flags.m_childrenOnSameLayer = true;
                a2dSurface* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dSurface );

                //add properties and handles as children in order to edit the object.
                int index = 0;
                a2dListOfa2dVertexList::const_iterator iterporg = original->m_holes.begin();
                for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
                {
                    a2dVertexListPtr vlist = ( *iterp );
                    a2dVertexListPtr vlistorg = ( *iterporg );
                    a2dVertexList::iterator iter = vlist->begin();
                    a2dVertexList::iterator iterorg = vlistorg->begin();
                    a2dLineSegment* seg;
                    a2dLineSegment* nextseg;
                    while ( iter != vlist->end() )
                    {
                        seg = ( *iter );
                        iter++;
                        if ( iter != vlist->end() )
                            nextseg = ( *iter );
                        else
                            nextseg = vlist->front();
                        iter--;

                        //not inclusive matrix so relative to polygon
                        a2dPolyHandleL* handle = new a2dPolyHandleL( this, iter, iterorg, vlist, vlistorg, wxT( "__indexhole__" ) );
                        Append( handle );
                        handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                        handle->SetLayer( m_layer );
                        if ( seg->GetArc() )
                        {
                            // todo split in line segments ( maybe under certain conditions of matrix not?? ).
                            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;

                            handle = new a2dPolyHandleL( this, iter, iterorg, vlist, vlistorg, cseg->m_x2, cseg->m_y2, wxT( "__indexhole_arc__" ) );
                            handle->SetLayer( m_layer );
                            Append( handle );
                            handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                        }
                        index++;
                        iter++;
                        iterorg++;
                    }
                    iterporg++;
                }

                m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dPolyHandleL" ) );
                m_flags.m_editingCopy = true;

                //calculate bbox's elase mouse events may take place when first idle event is not yet
                //processed to do this calculation.
                Update( updatemask_force );
                //stil set it pending to do the redraw ( in place )
                SetPending( true );
            }
            return true;
        }
        else
        {
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

void a2dSurface::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    if ( !m_flags.m_editingCopy || !m_flags.m_editable )
    {
        event.Skip();
        return;
    }

    a2dIterC* ic = event.GetIterC();

    a2dPolyHandleL* seghandle;

    double xw, yw;
    xw = event.GetX();
    yw = event.GetY();

    a2dSurface* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dSurface );

    if ( event.GetMouseEvent().LeftDown() )
    {
        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();
        if ( restrictEngine )
            restrictEngine->SetRestrictPoint( xw, yw );

        if ( event.GetHow().IsEdgeHit() )
        {
            int index = event.GetHow().m_index;
            a2dVertexList::iterator iterSeg = GetSegmentAtIndex( index );
            a2dVertexList::iterator iterSegOrg = original->GetSegmentAtIndex( index );
            a2dVertexList* vlist = GetSegmentListAtIndex( index );
            if ( vlist == m_lsegments )
            {
                event.Skip();
                return;
            }
            a2dVertexList* vlistorg = original->GetSegmentListAtIndex( index );

            if ( event.GetMouseEvent().AltDown() )
                seghandle = new a2dPolyHandleL( this, iterSeg, iterSegOrg, vlist, vlistorg, wxT( "__inserthole__" ) );
            else
                seghandle = new a2dPolyHandleL( this, iterSeg, iterSegOrg, vlist, vlistorg, wxT( "__segmenthole__" ) );
            seghandle->SetLayer( m_layer );
            Append( seghandle );
            seghandle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );

			//canvas mouse events arrive without the polygon matrix included. 
            a2dIterCU polygoncontext( *ic, this );
            a2dIterCU handlecontext( *ic, seghandle );
            seghandle->ProcessEvent( event );
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

void a2dSurface::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( event.GetCanvasHandle()->GetRelease() )
        return;

    if ( m_flags.m_editingCopy )
    {
        a2dPolyHandleL* draghandle = wxDynamicCast( event.GetCanvasHandle(), a2dPolyHandleL );
        if ( draghandle->GetName() != wxT( "__indexhole__" ) &&
                draghandle->GetName() != wxT( "__indexhole_arc__" ) &&
                draghandle->GetName() != wxT( "__inserthole__" ) &&
                draghandle->GetName() != wxT( "__segmenthole__" )
           )
        {
            event.Skip(); //maybe on the base class handles
            return;
        }

        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();
        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );

        if ( editmode == 1 )
        {
            double xw, yw;
            xw = event.GetX();
            yw = event.GetY();
            a2dAffineMatrix atWorld = ic->GetTransform();

            //we need to get the matrix until the parent of the a2dPolyHandleL,
            // and not inclusif the handle itself.
            //and therefore apply inverted handle matrix.
            a2dAffineMatrix inverselocal = m_lworld;
            inverselocal.Invert();

            a2dAffineMatrix inverse = ic->GetInverseParentTransform();

            double xinternal, yinternal;
            inverse.TransformPoint( xw, yw, xinternal, yinternal );

            a2dPolyHandleL* draghandle = wxDynamicCast( event.GetCanvasHandle(), a2dPolyHandleL );
            a2dVertexListPtr holevlist = draghandle->GetSegments();
            a2dVertexListPtr holevlistorg = draghandle->GetSegmentsOrg();


            a2dLineSegment* segment = *( draghandle->GetNode() );
            a2dLineSegment* nextsegment = *( holevlist->GetNextAround( draghandle->GetNode() ) );
            a2dLineSegment* segmentorg = *( draghandle->GetNodeOrg() );
            a2dLineSegment* nextsegmentorg = *( holevlistorg->GetNextAround( draghandle->GetNodeOrg() ) );

            a2dPolygonL* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dPolygonL );

            double xwi;
            double ywi;
            if( m_flags.m_snap )
            {
                if ( restrictEngine )
                    restrictEngine->RestrictPoint( xinternal, yinternal );
            }
            inverselocal.TransformPoint( xinternal, yinternal, xwi, ywi );
            a2dAffineMatrix origworld = m_lworld;

            if ( draghandle->GetName() == wxT( "__indexhole__" ) )
            {
                if ( event.GetMouseEvent().ControlDown() && event.GetMouseEvent().LeftDown() )
                {
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( original, segmentorg ) );
                    double x, y;
                    RemoveSegment( x, y, segment );
                    ReleaseChild( draghandle );

                    // the handle generated this event in LeftDown, But before LeftUp event the
                    // restart is already done, getting rid of handle.
                    // So do set context right here.
                    ic->SetCorridorPathToParent();
                    ic->GetDrawingPart()->PopCursor();
                }
                if ( event.GetMouseEvent().LeftUp() )
                {
                    double x, y;
                    x = segment->m_x; y = segment->m_y;
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveSegment( original, x, y, segmentorg, false ) );
                    //a2dDocviewGlobals->RecordF( this, wxT("movepoint %f %f %d"), x, y, index );
                    draghandle->SetPosXY( x, y );
                    //Klaas (code not good, handles are used and removed at same time )
                    //RemoveRedundantPoints( original );
                    original->AdjustAfterChange( true );
                }
                else if ( event.GetMouseEvent().Dragging() )
                {
                    //wxLogDebug( _("Dragging %d"), index);
                    segment->m_x = xwi;
                    segment->m_y = ywi;
                    SetPending( true );
                    draghandle->SetPosXY( xwi, ywi );
                    SetPosXYSegment( segmentorg, xwi, ywi, false  );
                    AdjustAfterChange( false );
                }
            }
            else if ( draghandle->GetName() == wxT( "__indexhole_arc__" ) )
            {
                a2dArcSegment* csegment = ( a2dArcSegment* ) segment;
                a2dArcSegment* csegmentorg = ( a2dArcSegment* ) segmentorg;
                if ( event.GetMouseEvent().LeftUp() )
                {
                    double x, y;
                    x = csegment->m_x2; y = csegment->m_y2;
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveMidSegment( original, x, y, csegmentorg, false ) );
                    //a2dDocviewGlobals->RecordF( this, wxT("movearcmidpoint %f %f %d"), x, y, index );
                    draghandle->SetPosXY( x, y );
                    //Klaas (code not good, handles are used and removed at same time )
                    //RemoveRedundantPoints( original );
                    original->AdjustAfterChange( true );
                }
                else if ( event.GetMouseEvent().Dragging() )
                {
                    csegment->m_x2 = xwi; csegment->m_y2 = ywi;
                    SetPending( true );
                    //csegment->SetMidPoint( *prevsegment, xwi, ywi );
                    //wxLogDebug( _("Dragging %d"), index);
                    draghandle->SetPosXYRestrict( xwi, ywi );
                    SyncHandlesWithLineSegments();
                    AdjustAfterChange( false );
                }
                else
                    event.Skip(); //maybe on the base class handles
            }
            else if ( draghandle->GetName() == wxT( "__inserthole__" ) )
            {
                // The index is the segment index. The index of the new point is 1 larger
                // than the segment index.
                if ( event.GetMouseEvent().LeftUp() )
                {
                    double x, y;
                    x = nextsegment->m_x;
                    y = nextsegment->m_y;
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_AddPoint( original, x, y, nextsegmentorg, false ) );
                    // !!!!???? how to do this ????!!!!
                    // a2dDocviewGlobals->RecordF( this, wxT("addpoint %f %f %d"), x, y, index+1 );
                    draghandle->SetPosXY( x, y );
                    original->AdjustAfterChange( true );
                }
                else if ( event.GetMouseEvent().LeftDown() )
                {
                    AddPoint( xwi, ywi, nextsegment, false );
                    draghandle->SetPosXYRestrict( xwi, ywi );
                    AdjustAfterChange( false );
                }
                else if ( event.GetMouseEvent().Dragging() )
                {
                    SetPosXYSegment( nextsegment, xwi, ywi , false  );
                    draghandle->SetPosXYRestrict( xwi, ywi );
                    AdjustAfterChange( false );
                }
                else
                    event.Skip(); //maybe on the base class handles
            }
            else if ( draghandle->GetName() == wxT( "__segmenthole__" ) )
            {
                if ( event.GetMouseEvent().LeftUp() )
                {
                    double x1, y1;
                    double x2, y2;
                    x1 = segment->m_x;
                    y1 = segment->m_y;

                    x2 = nextsegment->m_x;
                    y2 = nextsegment->m_y;

                    double xwi, ywi, dx, dy;
                    dx = 0.5 * ( x2 - x1 );
                    dy = 0.5 * ( y2 - y1 );
                    xwi =  x1 + dx;
                    ywi =  y1 + dy;
                    draghandle->SetPosXYRestrict( xwi, ywi );
                    x1 = xwi - dx; x2 = xwi + dx;
                    y1 = ywi - dy; y2 = ywi + dy;

                    m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveSegment( original, x1, y1, segmentorg, false ) );
                    m_root->GetCommandProcessor()->Submit(
                        new a2dCommand_MoveSegment( original, x2, y2, nextsegmentorg, false ) );

                    //a2dDocviewGlobals->RecordF( this, wxT("movepoint %f %f %d"), x1, y1, index );
                    //a2dDocviewGlobals->RecordF( this, wxT("movepoint %f %f %d"), x2, y2, index+1 );


                    //Klaas (code not good, handles are used and removed at same time )
                    //RemoveRedundantPoints( original );
                    //
                    SyncHandlesWithLineSegments();


                    // handles called "__segment__" are temporary handles, can be removed after drag
                    m_childobjects->Release( a2dCanvasOFlags::ALL, wxT( "a2dPolyHandleL" ), NULL, wxT( "__segmenthole__" ), false );
                    original->AdjustAfterChange( true );
                }
                else if ( event.GetMouseEvent().LeftDown() )
                {
                    // set the initial handle coordinates
                    a2dPolyHandleL::PROPID_tmpXIH->SetPropertyToObject( draghandle, xwi );
                    a2dPolyHandleL::PROPID_tmpYIH->SetPropertyToObject( draghandle, ywi );
                    // set the initial vertex coordinates
                    double x, y;
                    x = segment->m_x;
                    y = segment->m_y;
                    a2dPolyHandleL::PROPID_tmpXIV1->SetPropertyToObject( draghandle, x );
                    a2dPolyHandleL::PROPID_tmpYIV1->SetPropertyToObject( draghandle, y );
                    x = nextsegment->m_x;
                    y = nextsegment->m_y;
                    a2dPolyHandleL::PROPID_tmpXIV2->SetPropertyToObject( draghandle, x );
                    a2dPolyHandleL::PROPID_tmpYIV2->SetPropertyToObject( draghandle, y );
                }
                else if ( event.GetMouseEvent().Dragging() )
                {
                    draghandle->m_dragging = true;

                    double xi_h  = a2dPolyHandleL::PROPID_tmpXIH->GetPropertyValue( draghandle );
                    double yi_h  = a2dPolyHandleL::PROPID_tmpYIH->GetPropertyValue( draghandle );
                    double xi_v1 = a2dPolyHandleL::PROPID_tmpXIV1->GetPropertyValue( draghandle );
                    double yi_v1 = a2dPolyHandleL::PROPID_tmpYIV1->GetPropertyValue( draghandle );
                    double xi_v2 = a2dPolyHandleL::PROPID_tmpXIV2->GetPropertyValue( draghandle );
                    double yi_v2 = a2dPolyHandleL::PROPID_tmpYIV2->GetPropertyValue( draghandle );

                    //wxASSERT( xi_h && yi_h && xi_v1 && yi_v1 && xi_v2 && yi_v2 );

                    if( m_flags.m_snap )
                    {
                        // snap vertex 1 of segment.
                        double x, y, dx, dy;

                        dx = xi_v1 - xi_h;
                        dy = yi_v1 - yi_h;
                        x = xwi + dx;
                        y = ywi + dy;
                        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();
                        if ( restrictEngine )
                            restrictEngine->RestrictPoint( x, y );
                        xwi = x - dx;
                        ywi = y - dy;
                    }
                    draghandle->SetPosXY( xwi, ywi );
                    double x1 = xi_v1 - xi_h + xwi;
                    double y1 = yi_v1 - yi_h + ywi;
                    double x2 = xi_v2 - xi_h + xwi;
                    double y2 = yi_v2 - yi_h + ywi;

                    segment->m_x = x1;
                    segment->m_y = y1;
                    nextsegment->m_x = x2;
                    nextsegment->m_y = y2;

                    SetPending( true );
                    AdjustAfterChange( false );
                }
                else
                    event.Skip(); //maybe on the base class handles
            }
            else
                event.Skip(); //maybe the base class handles
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

void a2dSurface::SetHandleToIndex( a2dPolyHandleL* handle, unsigned int index )
{
    handle->SetNode( GetSegmentAtIndex( index ) );
    a2dPolygonL* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dPolygonL );
    handle->SetNodeOrg( original->GetSegmentAtIndex( index ) );
}

int a2dSurface::GetIndexSegment( a2dLineSegmentPtr seg )
{
    int index = -1;
    if ( seg.Get() == NULL )
    {
        if (  !m_lsegments->empty() )
        return m_lsegments->IndexOf( m_lsegments->back() );
        return -1;
    }

    index = m_lsegments->IndexOf( seg );
    if ( index != -1 )
        return index;

    index = m_lsegments->size();
    index++;
    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        a2dVertexList::iterator iter = vlist->begin();
        while ( iter != vlist->end() )
        {
            if ( *iter == seg )
            {
                return index;
            }
            index++;
            iter++;
        }
        index++; // dummy index for ends
    }
    return -1;
}

a2dVertexList::iterator a2dSurface::GetSegmentAtIndex( unsigned int index )
{
    if ( index < m_lsegments->size() + 1 )
    {
        return m_lsegments->item( index );
    }
    unsigned int indexn = m_lsegments->size();
    indexn++;
    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        a2dVertexList::iterator iter = vlist->begin();
        while ( iter != vlist->end() )
        {
            if ( indexn == index )
            {
                return iter;
            }
            indexn++;
            iter++;
        }
        if( indexn == index )
        {
            return iter;
        }
        indexn++; // dummy index for end
    }
    return m_lsegments->end();
}

a2dVertexList* a2dSurface::GetSegmentListAtIndex( unsigned int index )
{
    if ( index < m_lsegments->size() + 1 )
    {
        return m_lsegments;
    }
    unsigned int indexn = m_lsegments->size();
    indexn++;
    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        a2dVertexList::iterator iter = vlist->begin();
        while ( iter != vlist->end() )
        {
            if ( indexn == index )
            {
                return vlist;
            }
            indexn++;
            iter++;
        }
        if( indexn == index )
        {
            return vlist;
        }
        indexn++; // dummy index for end
    }
    return NULL;
}


void a2dSurface::RemoveSegment( double& x, double& y , a2dLineSegmentPtr seg, bool transformed )
{
    if ( seg.Get() == NULL )
    {
        a2dLineSegment* point;
        point = m_lsegments->back();
        x = point->m_x;
        y = point->m_y;
        m_lsegments->pop_back();
    }
    else
    {
        a2dVertexList::iterator iter = m_lsegments->begin();
        a2dLineSegment* point;
        while ( iter != m_lsegments->end() )
        {
            if ( *iter == seg )
            {
                point = ( *iter );
                x = point->m_x;
                y = point->m_y;
                m_lsegments->erase( iter );
                break;
            }
            iter++;
        }
    }

    a2dLineSegment* point;
    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        a2dVertexList::iterator iter = vlist->begin();
        while ( iter != vlist->end() )
        {
            if ( *iter == seg )
            {
                point = ( *iter );
                x = point->m_x;
                y = point->m_y;
                vlist->erase( iter );
                break;
            }
            iter++;
        }
    }

    if ( transformed )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }

    SetPending( true );
}

void a2dSurface::InsertSegment( unsigned int index, a2dLineSegmentPtr segin )
{
    if ( index < m_lsegments->size() + 1 )
    {
        m_lsegments->Insert( index, segin );
        return;
    }
    unsigned int indexn = m_lsegments->size();
    indexn++;
    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        a2dVertexList::iterator iter = vlist->begin();
        while ( iter != vlist->end() )
        {
            if ( indexn == index )
            {
                vlist->insert( iter, segin );
                return;
            }
            indexn++;
            iter++;
        }
        if( indexn == index )
        {
            vlist->insert( iter, segin );
            return;
        }
        indexn++; // dummy index for end
    }

}


void a2dSurface::GetPosXYSegment( a2dLineSegmentPtr seg, double& x, double& y, bool transform ) const
{
    if ( seg.Get() == NULL )
    {
        a2dLineSegment* point;
        point = m_lsegments->back();
        x = point->m_x;
        y = point->m_y;
    }
    else
    {
        x = seg->m_x;
        y = seg->m_y;
    }

    if ( transform )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }
}

void a2dSurface::SetPosXYSegment( a2dLineSegmentPtr seg, double x, double y, bool afterinversion )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x, y, x, y );
    }

    m_lsegments->SetPointAdjustArcs( seg, x, y, true );

    for( a2dListOfa2dVertexList::const_iterator iterp = m_holes.begin(); iterp != m_holes.end(); iterp++ )
    {
        a2dVertexListPtr vlist = ( *iterp );
        vlist->SetPointAdjustArcs( seg, x, y, true );
    }

    SetPending( true );
}
