/*! \file wx/canvas/algos2.h
    \brief a2dWalker based algorithms

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: algos.h,v 1.2 2009/09/03 20:09:53 titato Exp $
*/

#ifndef __WXALGOS2_H__
#define __WXALGOS2_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <map>
#include <set>

using namespace std;

#include "wx/canvas/canglob.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/xmlpars.h"
#include "wx/canvas/vpath.h"

//! algorithm to  delete a2dCanvasObjects on layers or to move, copy to other layers
/*!
    groupA and groupB are filled with layer numbers.
    Next one calls a valid operation, which is can be:
    - moveLayers
    - copyLayers
    - deleteLayers

    a2dCanvasObjects with the flag a2dCanvasOFlags::m_ignoreLayer set, will be ignored here.

    \ingroup docalgo
*/
class A2DCANVASDLLEXP a2dWalker_LayerCanvasObjects: public a2dWalker_LayerGroup
{
public:

    typedef long Operation;

    //! move layers to target
    static const long moveLayers;
    //! copy layers to target
    static const long copyLayers;
    //! delete layers
    static const long deleteLayers;
    //! convert segments in polygon/polyline objects in group A to Arcs where possible
    static const long ConvertToArcs;
    //!  convert to simple polygons and polylines preserve arcs
    static const long ConvertToPolygonPolylinesWithArcs;
    //!  convert to simple polygons and polylines
    static const long ConvertToPolygonPolylinesWithoutArcs;
    //! convert segments in polygon/polyline with width objects in group A to Arcs where possible
    static const long ConvertPolygonToArcs;
    //! convert segments in polyline objects in group A to Arcs where possible
    static const long ConvertPolylineToArcs;
    //! convert shapes to vector paths
    static const long ConvertToVPaths;
    //! convert shapes to seperate lines and arcs
    static const long ConvertLinesArcs;
    //! convert shapes to polylines even if polygons
    static const long ConvertToPolylines;
    //! remove redundant segment
    static const long RemoveRedundant;
    //! calculate area of objects
    static const long areaLayers;
    //! convert to polygons/lines with vertexes at every distance x around polygon 
    static const long ConvertPointsAtDistance;

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_LayerCanvasObjects( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_LayerCanvasObjects( const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_LayerCanvasObjects( const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! destructor
    ~a2dWalker_LayerCanvasObjects();

    //! used from constructor
    void Initialize();

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

    //! set type of operation
    void SetOperation( Operation operation ) { m_operation = operation; }

    //! Polygon/polyline to Arc Minimum radius to test
    void SetRadiusMin( double radiusMin ) { m_radiusMin = radiusMin; }
    //! Polygon/polyline to Arc Maximum radius to test
    void SetRadiusMax( double radiusMax ) { m_radiusMax = radiusMax; }
    //! Polygon/polyline to Arc Maximum abberation
    void SetAberPolyToArc( double aber ) { m_AberPolyToArc = aber; }
    //! used in conversion from arc segment to polygon, defined in document database coordinates
    void SetAberArcToPoly( double aber ) { m_AberArcToPoly = aber; }

    //! for operation ConvertPointsAtDistance, this will be the distance the vertexes will have
    void SetDistancePoints( double distancePoints ) { m_distancePoints = distancePoints; }
    //! for operation ConvertPointsAtDistance, this will creates vertexes at the original vertexes also if set true.
    void SetAtVertexPoints( bool atVertexPoints ) { m_atVertexPoints = atVertexPoints; }

    //! if after polygon conversion to arcs end up as one arc segment  forming a circle,
    //! add a circle object to target and not a polygon.
    void SetDetectCircle( bool detectCircle ) { m_detectCircle = detectCircle; }

    //! see SetDetectCircle
    bool GetDetectCircle() { return m_detectCircle; }

    //! in areaLayers returns calculated area sofar.
    double GetCalculatedArea() { return m_calculatedArea; }

    void SetLastPointCloseToFirst( bool lastPointCloseToFirst ) { m_lastPointCloseToFirst = lastPointCloseToFirst; }

    //! set what is seen as small.
    void SetSmall( double smallest ) { m_small = smallest; }

protected:

    void AddArea( a2dCanvasObject* getAsPolyObject );

    //! Polygon/polyline to Arc Minimum radius to test
    double m_radiusMin;
    //! Polygon/polyline to Arc Maximum radius to test
    double m_radiusMax;
    //! Polygon/polyline to Arc Maximum abberation
    double m_AberPolyToArc;

    double m_AberArcToPoly;

    double m_distancePoints;
    bool m_atVertexPoints;
    bool m_lastPointCloseToFirst;

    bool m_detectCircle;

    double m_calculatedArea;
    double m_small;

    //! id for operation
    Operation m_operation;
};

template< class Function >
class A2DCANVASDLLEXP a2dWalker_ForEachCanvasObject: public a2dWalker_LayerGroup
{
public:

    //! constructor with no a2dPropertyId added yet. For that use AddPropertyId()
    a2dWalker_ForEachCanvasObject( Function func, a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with a2dPropertyId added to the propertyIsList to search for.
    a2dWalker_ForEachCanvasObject( Function func, const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! constructor with the propertyIdList to search for as input
    a2dWalker_ForEachCanvasObject( Function func, const a2dPropertyIdList& idList , a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    //! destructor
    ~a2dWalker_ForEachCanvasObject();

    //! called from within a2dCanvasDocument
    virtual bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

protected:

    Function m_function;
};

//----------------------------------------------------------------------------
// a2dWalker_ForEachCanvasObject
//----------------------------------------------------------------------------

template< class Function >
a2dWalker_ForEachCanvasObject< Function >::a2dWalker_ForEachCanvasObject( Function func, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( mask )
{
    m_function = func;
    Initialize();
}

template< class Function >
a2dWalker_ForEachCanvasObject< Function >::a2dWalker_ForEachCanvasObject( Function func, const a2dPropertyId* id , a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( id, mask )
{
    m_function = func;
    Initialize();
}


template< class Function >
a2dWalker_ForEachCanvasObject< Function >::a2dWalker_ForEachCanvasObject( Function func, const a2dPropertyIdList& idList, a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerGroup( idList, mask )
{
    m_function = func;
    Initialize();
}

template< class Function >
a2dWalker_ForEachCanvasObject< Function >::~a2dWalker_ForEachCanvasObject()
{
}

template< class Function >
bool a2dWalker_ForEachCanvasObject< Function >::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{
    if ( m_currentDepth > m_depth  )
        SetStopWalking( true );
    else
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        a2dCanvasObject* parentobj =  wxDynamicCast( parent, a2dCanvasObject );
        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && parentobj && !obj->GetIgnoreLayer() && !obj->GetBin() && ObjectOke( obj ) && ClassOfObjectOke( obj ) )
        {
            m_result = m_function( this, parentobj, obj ) || m_result;
        }
    }

    if (  m_currentDepth <= m_depth   )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

class a2dMoveLayer 
{
public:

    a2dMoveLayer()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dMoveLayer >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            obj->SetLayer( walker->GetTarget() );
            return true;
        }
        return false;
	}
};

class a2dCopyLayer 
{
public:

    a2dCopyLayer()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dCopyLayer >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
            parent->Prepend( cobj );
            cobj->SetLayer( walker->GetTarget() );
            cobj->SetBin( true );
            return true;
        }
        return false;
	}
};

class a2dDeleteLayer 
{
public:

    a2dDeleteLayer()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dDeleteLayer >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            parent->ReleaseChild( obj );
            return true;
        }
        return false;
	}
};

class a2dConvertToArcs 
{
public:

    a2dConvertToArcs()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dConvertToArcs >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            if ( 0 != wxDynamicCast( obj, a2dPolylineL ) && obj->GetContourWidth() )
            {
                a2dPolylineL* objc = wxDynamicCast( obj, a2dPolylineL );
                bool returnIsPolygon;
                a2dPolygonL* contour = new a2dPolygonL( objc->GetAsVertexList( returnIsPolygon ) );
                contour->SetBin( true );
                //contour->GetSegments()->ConvertPolylineToArc( m_AberPolyToArc, m_radiusMin, m_radiusMax );
                parent->Append( contour );
                contour->SetLayer( walker->GetTarget() );
            }
            return true;
        }
        return false;
	}

    //! Polygon/polyline to Arc Minimum radius to test
    void SetRadiusMin( double radiusMin ) { m_radiusMin = radiusMin; }
    //! Polygon/polyline to Arc Maximum radius to test
    void SetRadiusMax( double radiusMax ) { m_radiusMax = radiusMax; }
    //! Polygon/polyline to Arc Maximum abberation
    void SetAberPolyToArc( double aber ) { m_AberPolyToArc = aber; }

public:

    //! Polygon/polyline to Arc Minimum radius to test
    double m_radiusMin;
    //! Polygon/polyline to Arc Maximum radius to test
    double m_radiusMax;
    //! Polygon/polyline to Arc Maximum abberation
    double m_AberPolyToArc;
};

class a2dConvertPolygonToArcs 
{
public:

    a2dConvertPolygonToArcs()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dConvertPolygonToArcs >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            if ( 0 != wxDynamicCast( obj, a2dPolygonL ) )
            {
                a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                cobj->SetBin( true );
                a2dPolygonL* objc = wxDynamicCast( cobj, a2dPolygonL );
                objc->GetSegments()->ConvertPolygonToArc( m_AberPolyToArc, m_radiusMin, m_radiusMax );
                a2dPoint2D middle;
                double radius;
                if ( m_detectCircle && objc->GetSegments()->CheckForOneCircle( middle, radius ) )
                {
                    a2dCircle* cir = new a2dCircle( middle.m_x, middle.m_y, radius );
                    cir->Transform( obj->GetTransformMatrix() );
                    cir->SetBin( true );
                    parent->Prepend( cir );
                    cir->SetLayer( walker->GetTarget() );
                    delete cobj;
                }
                else
                {
                    parent->Prepend( cobj );
                    cobj->SetLayer( walker->GetTarget() );
                }
            }
            return true;
        }
        return false;
	}

    //! Polygon/polyline to Arc Minimum radius to test
    void SetRadiusMin( double radiusMin ) { m_radiusMin = radiusMin; }
    //! Polygon/polyline to Arc Maximum radius to test
    void SetRadiusMax( double radiusMax ) { m_radiusMax = radiusMax; }
    //! Polygon/polyline to Arc Maximum abberation
    void SetAberPolyToArc( double aber ) { m_AberPolyToArc = aber; }

    //! if after polygon conversion to arcs end up as one arc segment  forming a circle,
    //! add a circle object to target and not a polygon.
    void SetDetectCircle( bool detectCircle ) { m_detectCircle = detectCircle; }

    //! see SetDetectCircle
    bool GetDetectCircle() { return m_detectCircle; }

public:

    //! Polygon/polyline to Arc Minimum radius to test
    double m_radiusMin;
    //! Polygon/polyline to Arc Maximum radius to test
    double m_radiusMax;
    //! Polygon/polyline to Arc Maximum abberation
    double m_AberPolyToArc;
    bool m_detectCircle;
};

class a2dConvertPolylineToArcs 
{
public:

    a2dConvertPolylineToArcs()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dConvertPolylineToArcs >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            if ( 0 != wxDynamicCast( obj, a2dPolylineL ) && obj->GetContourWidth() )
            {
                a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                a2dPolylineL* objc = wxDynamicCast( cobj, a2dPolylineL );
                cobj->SetBin( true );
                parent->Prepend( cobj );
                cobj->SetLayer( walker->GetTarget() );
                objc->GetSegments()->ConvertPolylineToArc( m_AberPolyToArc, m_radiusMin, m_radiusMax );
            }
            else if ( 0 != wxDynamicCast( obj, a2dPolylineL ) )
            {
                a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                cobj->SetBin( true );
                parent->Prepend( cobj );
                cobj->SetLayer( walker->GetTarget() );
                a2dPolylineL* objc = wxDynamicCast( cobj, a2dPolylineL );
                objc->GetSegments()->ConvertPolylineToArc( m_AberPolyToArc, m_radiusMin, m_radiusMax );
            }
            return true;
        }
        return false;
	}

    //! Polygon/polyline to Arc Minimum radius to test
    void SetRadiusMin( double radiusMin ) { m_radiusMin = radiusMin; }
    //! Polygon/polyline to Arc Maximum radius to test
    void SetRadiusMax( double radiusMax ) { m_radiusMax = radiusMax; }
    //! Polygon/polyline to Arc Maximum abberation
    void SetAberPolyToArc( double aber ) { m_AberPolyToArc = aber; }

    //! if after polygon conversion to arcs end up as one arc segment  forming a circle,
    //! add a circle object to target and not a polygon.
    void SetDetectCircle( bool detectCircle ) { m_detectCircle = detectCircle; }

    //! see SetDetectCircle
    bool GetDetectCircle() { return m_detectCircle; }

public:

    //! Polygon/polyline to Arc Minimum radius to test
    double m_radiusMin;
    //! Polygon/polyline to Arc Maximum radius to test
    double m_radiusMax;
    //! Polygon/polyline to Arc Maximum abberation
    double m_AberPolyToArc;
    bool m_detectCircle;
};

class a2dConvertToPolygonPolylinesWithArcs 
{
public:

    a2dConvertToPolygonPolylinesWithArcs()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dConvertToPolygonPolylinesWithArcs >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            a2dCanvasObjectList* polylist = walker->ConvertToPolygons( obj, true );
            for( a2dCanvasObjectList::iterator iterp = polylist->begin(); iterp != polylist->end(); ++iterp )
            {
                a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                a2dPolylineL* polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                if ( poly && poly->GetNumberOfSegments() > 2 )
                {
                    poly->SetBin( true );
                    parent->Prepend( poly );
                    poly->SetLayer( walker->GetTarget() );
                }
                else if ( polyline && polyline->GetNumberOfSegments() > 2 )
                {
                    polyline->SetBin( true );
                    parent->Prepend( polyline );
                    polyline->SetLayer( walker->GetTarget() );
                }
            }
            delete polylist;
            return true;
        }
        return false;
	}

    //! Polygon/polyline to Arc Minimum radius to test
    void SetRadiusMin( double radiusMin ) { m_radiusMin = radiusMin; }
    //! Polygon/polyline to Arc Maximum radius to test
    void SetRadiusMax( double radiusMax ) { m_radiusMax = radiusMax; }
    //! Polygon/polyline to Arc Maximum abberation
    void SetAberPolyToArc( double aber ) { m_AberPolyToArc = aber; }

    //! if after polygon conversion to arcs end up as one arc segment  forming a circle,
    //! add a circle object to target and not a polygon.
    void SetDetectCircle( bool detectCircle ) { m_detectCircle = detectCircle; }

    //! see SetDetectCircle
    bool GetDetectCircle() { return m_detectCircle; }

public:

    //! Polygon/polyline to Arc Minimum radius to test
    double m_radiusMin;
    //! Polygon/polyline to Arc Maximum radius to test
    double m_radiusMax;
    //! Polygon/polyline to Arc Maximum abberation
    double m_AberPolyToArc;
    bool m_detectCircle;
};

class a2dConvertToPolygonPolylinesWithoutArcs 
{
public:

    a2dConvertToPolygonPolylinesWithoutArcs()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dConvertToPolygonPolylinesWithoutArcs >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            a2dCanvasObjectList* polylist = walker->ConvertToPolygons( obj, true );
            for( a2dCanvasObjectList::iterator iterp = polylist->begin(); iterp != polylist->end(); ++iterp )
            {
                a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                a2dPolylineL* polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                if ( poly && poly->GetNumberOfSegments() > 2 )
                {
                    poly->SetBin( true );
                    poly->GetSegments()->ConvertToLines( m_AberArcToPoly );
                    parent->Prepend( poly );
                    poly->SetLayer( walker->GetTarget() );
                }
                else if ( polyline && polyline->GetNumberOfSegments() > 2 )
                {
                    polyline->SetBin( true );
                    polyline->GetSegments()->ConvertToLines( m_AberArcToPoly );
                    parent->Prepend( polyline );
                    polyline->SetLayer(  walker->GetTarget() );
                }
            }
            delete polylist;
            return true;
        }
        return false;
	}

    //! used in conversion from arc segment to polygon, defined in document database coordinates
    void SetAberArcToPoly( double aber ) { m_AberArcToPoly = aber; }

public:

    double m_AberArcToPoly;

};

class a2dConvertToVPaths 
{
public:

    a2dConvertToVPaths()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dConvertToVPaths >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyB() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            a2dCanvasObjectList* vpath = walker->ConvertToVpath( obj, true );
            for( a2dCanvasObjectList::iterator iterp = vpath->begin(); iterp != vpath->end(); ++iterp )
            {
                a2dVectorPath* path = wxDynamicCast( ( *iterp ).Get(), a2dVectorPath );
                path->SetBin( true );
                parent->Prepend( path );
                path->SetLayer( walker->GetTarget() );
            }
            delete vpath;
            return true;
        }
        return false;
	}
};

class a2dConvertToPolylines 
{
public:

    a2dConvertToPolylines()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dConvertToPolylines >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            a2dCanvasObjectList* polylist = obj->GetAsPolylines( true );
            if ( polylist != wxNullCanvasObjectList )
            {
                int i, count = polylist->size();
                a2dCanvasObjectList::iterator iterp = polylist->begin();
                for( i = 0 ; i < count ; i++ )
                {
                    a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                    a2dPolylineL* polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                    if ( poly && poly->GetNumberOfSegments() > 2 )
                    {
                        a2dPolylineL* conv = new a2dPolylineL( poly->GetSegments() );

                        conv->SetBin( true );
                        parent->Prepend( conv );
                        conv->SetLayer( walker->GetTarget() );
                        iterp = polylist->erase( iterp );
                    }
                    else if ( polyline && polyline->GetNumberOfSegments() > 2 )
                    {
                        polyline->SetBin( true );
                        parent->Prepend( polyline );
                        polyline->SetLayer( walker->GetTarget() );
                        iterp++;
                    }
                }
                delete polylist;
            }

            return true;
        }
        return false;
	}
};


class a2dConvertLinesArcs 
{
public:

    a2dConvertLinesArcs()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dConvertLinesArcs >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            a2dCanvasObjectList* polylist = walker->ConvertToPolygons( obj, true );
            int i, count = polylist->size();
            a2dCanvasObjectList::iterator iterp = polylist->begin();
            for( i = 0 ; i < count ; i++ )
            {
                a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                poly->SetLayer( walker->GetTarget() );
                poly->SetBin( true );
                a2dCanvasObjectList* linesAndArcs = poly->GetAsLinesArcs();
                for( a2dCanvasObjectList::iterator iterl = linesAndArcs->begin(); iterl != linesAndArcs->end(); ++iterl )
                    parent->Prepend( *iterl );
                iterp = polylist->erase( iterp );
                delete linesAndArcs;
            }
            delete polylist;
            return true;
        }
        return false;
	}
};

class a2dRemoveRedundant 
{
public:

    a2dRemoveRedundant()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dRemoveRedundant >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            if ( 0 != wxDynamicCast( obj, a2dPolylineL ) )
            {
                a2dPolylineL* cobj = wxDynamicCast( obj, a2dPolylineL );
                cobj->GetSegments()->RemoveRedundant( false );
                cobj->SetLayer( walker->GetTarget() );
                cobj->SetBin( true );
            }
            else if ( 0 != wxDynamicCast( obj, a2dPolygonL ) )
            {
                a2dPolygonL* cobj = wxDynamicCast( obj, a2dPolygonL );
                cobj->GetSegments()->RemoveRedundant( true );
                cobj->SetLayer( walker->GetTarget() );
                cobj->SetBin( true );
            }
            return true;
        }
        return false;
	}
};

class a2dConvertPointsAtDistance 
{
public:

    a2dConvertPointsAtDistance()
    {
        m_distancePoints = 100;
        m_atVertexPoints = false;
        m_lastPointCloseToFirst = true;

    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dConvertPointsAtDistance >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyB() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            a2dCanvasObjectList* polylist = walker->ConvertToPolygons( obj, true );
            for( a2dCanvasObjectList::iterator iterp = polylist->begin(); iterp != polylist->end(); ++iterp )
            {
                a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                a2dPolylineL* polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                if ( poly && poly->GetNumberOfSegments() > 2 )
                {
                    poly->SetBin( true );
                    poly->GetSegments()->ConvertToLines( m_AberArcToPoly );
                    a2dVertexListPtr out = poly->GetSegments()->ConvertPointsAtDistance( m_distancePoints, true, m_atVertexPoints );
                    a2dVertexList::iterator iterp = out->begin();
                    a2dPolygonL* newpoly = new a2dPolygonL( out );
                    newpoly->SetPosXY( poly->GetPosX(), poly->GetPosY() );
                    parent->Prepend( newpoly );
                    newpoly->SetLayer( walker->GetTarget() );
                }
                else if ( polyline && polyline->GetNumberOfSegments() > 2 )
                {
                    polyline->SetBin( true );
                    polyline->GetSegments()->ConvertToLines( m_AberArcToPoly );
                    a2dVertexListPtr out = poly->GetSegments()->ConvertPointsAtDistance( m_distancePoints, false, m_atVertexPoints );
                    a2dVertexList::iterator iterp = out->begin();
                    a2dPolylineL* newpoly = new a2dPolylineL( out );
                    newpoly->SetPosXY( poly->GetPosX(), poly->GetPosY() );
                    parent->Prepend( newpoly );
                    newpoly->SetLayer( walker->GetTarget() );
                }
            }
            delete polylist;

            return true;
        }
        return false;
	}

    //! used in conversion from arc segment to polygon, defined in document database coordinates
    void SetAberArcToPoly( double aber ) { m_AberArcToPoly = aber; }

        //! for operation ConvertPointsAtDistance, this will be the distance the vertexes will have
    void SetDistancePoints( double distancePoints ) { m_distancePoints = distancePoints; }
    //! for operation ConvertPointsAtDistance, this will creates vertexes at the original vertexes also if set true.
    void SetAtVertexPoints( bool atVertexPoints ) { m_atVertexPoints = atVertexPoints; }

    void SetLastPointCloseToFirst( bool lastPointCloseToFirst ) { m_lastPointCloseToFirst = lastPointCloseToFirst; }

protected:

    double m_distancePoints;
    bool m_atVertexPoints;
    bool m_lastPointCloseToFirst;

    double m_AberArcToPoly;

};

class a2dAreaLayer 
{
public:

    a2dAreaLayer()
    {
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dAreaLayer >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj)  
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyB() && obj->GetSelected() ) ) )
        {
            AddArea( obj );
            return true;
        }
        return false;
	}

    void AddArea( a2dCanvasObject* getAsPolyObject )
    {
        a2dCanvasObjectList* vpath = wxNullCanvasObjectList;

        if ( 0 != wxDynamicCast( getAsPolyObject, a2dRect ) )
        {
            a2dRect* obj = wxDynamicCast( getAsPolyObject, a2dRect );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dRectC ) )
        {
            a2dRectC* obj = wxDynamicCast( getAsPolyObject, a2dRectC );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dArrow ) )
        {
            a2dArrow* obj = wxDynamicCast( getAsPolyObject, a2dArrow );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dCircle ) )
        {
            a2dCircle* obj = wxDynamicCast( getAsPolyObject, a2dCircle );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dEllipse ) )
        {
            a2dEllipse* obj = wxDynamicCast( getAsPolyObject, a2dEllipse );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dEllipticArc ) )
        {
            a2dEllipticArc* obj = wxDynamicCast( getAsPolyObject, a2dEllipticArc );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dArc ) )
        {
            a2dArc* obj = wxDynamicCast( getAsPolyObject, a2dArc );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dEndsLine ) && getAsPolyObject->GetContourWidth() )
        {
            a2dEndsLine* obj = wxDynamicCast( getAsPolyObject, a2dEndsLine );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dSLine ) && getAsPolyObject->GetContourWidth() )
        {
            a2dSLine* obj = wxDynamicCast( getAsPolyObject, a2dSLine );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dPolylineL ) && getAsPolyObject->GetContourWidth() )
        {
            a2dPolylineL* obj = wxDynamicCast( getAsPolyObject, a2dPolylineL );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dPolygonL ) )
        {
            a2dPolygonL* obj = wxDynamicCast( getAsPolyObject, a2dPolygonL );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dVectorPath ) )
        {
            a2dVectorPath* obj = wxDynamicCast( getAsPolyObject, a2dVectorPath );
            vpath = obj->GetAsCanvasVpaths();
        }
        else if ( 0 != wxDynamicCast( getAsPolyObject, a2dText ) )
        {
            a2dText* obj = wxDynamicCast( getAsPolyObject, a2dText );
            vpath = obj->GetAsCanvasVpaths();
        }

        // now we have vector path, which we will convert to polygons and polylines.
        // Next only the polygons will be used for boolean operations.
        if ( vpath != wxNullCanvasObjectList )
        {

            a2dCanvasObjectList::iterator iter = vpath->begin();
            while ( iter != vpath->end() )
            {
                a2dCanvasObjectList* polylist = NULL;
                a2dVectorPath* obj = ( a2dVectorPath* ) ( *iter ).Get();
                polylist = obj->GetAsPolygons();

                for( a2dCanvasObjectList::iterator iterp = polylist->begin(); iterp != polylist->end(); ++iterp )
                {
                    a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                    if ( poly && poly->GetNumberOfSegments() > 2 )
                    {
                        m_calculatedArea += poly->GetSegments()->CalcArea();
                    }
                }
                delete polylist;

                iter = vpath->erase( iter );
                iter = vpath->begin();
            }
            delete vpath;
        }
    }


    //! in areaLayers returns calculated area sofar.
    double GetCalculatedArea() { return m_calculatedArea; }

protected:
    double m_calculatedArea;

};


class a2dDeleteFlagged
{
public:

    a2dDeleteFlagged( a2dCanvasObjectFlagsMask which = a2dCanvasOFlags::ALL, bool deleteIfSet = true )
    {
        m_flagDelete = which;
        m_deleteIfSet  = deleteIfSet;
    }

	bool operator()( a2dWalker_ForEachCanvasObject< a2dDeleteFlagged >* walker, a2dCanvasObject* parent, a2dCanvasObject* obj) const 
    {
        bool hasLayer = walker->GetGroupA().InGroup( obj->GetLayer() );
        if ( hasLayer && ( !walker->GetSelectedOnlyA() || ( walker->GetSelectedOnlyA() && obj->GetSelected() ) ) )
        {
            if ( obj->GetFlag( m_flagDelete ) )
            {
                if ( m_deleteIfSet )
                    obj->SetRelease( true );
            }
            else
            {
                if ( !m_deleteIfSet )
                   obj->SetRelease( true );
            }
            return true;
        }
        return false;
	}

    a2dCanvasObjectFlagsMask m_flagDelete;
    bool m_deleteIfSet;
};


typedef a2dWalker_ForEachCanvasObject<a2dMoveLayer> a2dWalker_MoveLayer;
typedef a2dWalker_ForEachCanvasObject<a2dCopyLayer> a2dWalker_CopyLayer;
typedef a2dWalker_ForEachCanvasObject<a2dCopyLayer> a2dWalker_DeleteLayer;
typedef a2dWalker_ForEachCanvasObject<a2dConvertToArcs> a2dWalker_ConvertToArcs;
typedef a2dWalker_ForEachCanvasObject<a2dConvertPolygonToArcs> a2dWalker_ConvertPolygonToArcs;
typedef a2dWalker_ForEachCanvasObject<a2dConvertPolylineToArcs> a2dWalker_ConvertPolylineToArcs;
typedef a2dWalker_ForEachCanvasObject<a2dConvertToPolygonPolylinesWithArcs> a2dWalker_ConvertToPolygonPolylinesWithArcs;
typedef a2dWalker_ForEachCanvasObject<a2dConvertToPolygonPolylinesWithoutArcs> a2dWalker_ConvertToPolygonPolylinesWithoutArcs;
typedef a2dWalker_ForEachCanvasObject<a2dConvertToVPaths> a2dWalker_ConvertToVPaths;
typedef a2dWalker_ForEachCanvasObject<a2dConvertToPolylines> a2dWalker_ConvertToPolylines;
typedef a2dWalker_ForEachCanvasObject<a2dConvertLinesArcs> a2dWalker_ConvertLinesArcs;
typedef a2dWalker_ForEachCanvasObject<a2dRemoveRedundant> a2dWalker_RemoveRedundant;
typedef a2dWalker_ForEachCanvasObject<a2dConvertPointsAtDistance> a2dWalker_ConvertPointsAtDistance;
typedef a2dWalker_ForEachCanvasObject<a2dAreaLayer> a2dWalker_AreaLayer;
typedef a2dWalker_ForEachCanvasObject<a2dDeleteFlagged> a2dWalker_DeleteBin;


#endif
