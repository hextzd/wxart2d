/*! \file canvas/src/route.cpp
    \author Michael S�trop

    Copyright: 2003-2004 (c) Michael S�trop

    Licence: wxWidgets Licence

    RCS-ID: $Id: wire.cpp,v 1.71 2008/11/09 11:16:53 bionic-badger Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/route.h"
#include "wx/canvas/wire.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/canpin.h"
#include "wx/canvas/drawing.h"

//#include "wx/genart/prfl.h"
//#include "wx/genart/prfltmr.h"
//#include "wx/genart/prflcntmean.h"
//#include "wx/genart/prflenbl.h"

//#define DUMP_FINAL

IMPLEMENT_CLASS( a2dRouteData, a2dObject );

double a2dRouteData::m_raster = 4;

a2dRouteData::BorderQueue::BorderQueue()
{
    m_mincostindex = 0;
    memset( m_costring, 0, sizeof( m_costring ) );
    m_freelist = 0;
    m_memory = new AllocBlock();
    m_memory->m_next = 0;
    m_freememory = 0;
#ifdef _DEBUG
    m_mincost = 0;
#endif
#ifdef PRFL_ENBL
    m_count = 0;
#endif
}

a2dRouteData::BorderQueue::~BorderQueue()
{
    AllocBlock* current, *next;

    for( current = m_memory; current; current = next )
    {
        next = current->m_next;
        delete current;
    }
}

void a2dRouteData::BorderQueue::Add( const BorderPoint& brdr )
{
#ifdef PRFL_ENBL
    m_count++;
#endif

    // Allocate memory
    BorderPoint* newobjc;

    if( m_freelist )
    {
        // an object in the free list
        newobjc = m_freelist;
        m_freelist = m_freelist->m_next;
    }
    else if( m_freememory < AllocBlock::m_pointsperblock )
    {
        // an object in the allocation block
        newobjc = m_memory->m_memory + m_freememory;
        m_freememory ++;
    }
    else
    {
        // create new allocation block
        AllocBlock* newblock = new AllocBlock();
        newblock->m_next = m_memory;
        m_memory = newblock;
        newobjc = m_memory->m_memory;
        m_freememory = 1;
    }

    // insert into allocation ring
    *newobjc = brdr;
#ifdef _DEBUG
    wxASSERT( brdr.m_cost - m_mincost < m_ncost );
#endif
    int index = brdr.m_cost & m_costmask;
    newobjc->m_next = m_costring[index];
    m_costring[index] = newobjc;

    if( !m_costring[ m_mincostindex ] )
    {
        // This happens only the first time
#ifdef _DEBUG
        wxASSERT( m_mincostindex == 0 && m_mincost == 0 );
#endif
        m_mincostindex = index;
#ifdef _DEBUG
        m_mincost = brdr.m_cost;
#endif
    }
}

void a2dRouteData::BorderQueue::RmvBest()
{
#ifdef PRFL_ENBL
    m_count--;
#endif

    // Get and remove best object
    BorderPoint* obj = m_costring[ m_mincostindex ];
    wxASSERT( obj );
    m_costring[ m_mincostindex ] = obj->m_next;
    // Insert into free list
    obj->m_next = m_freelist;
    m_freelist = obj;
    // Increent cost
    if( !m_costring[ m_mincostindex ] )
    {
        unsigned int i;
        for( i = m_mincostindex; i < m_ncost; i++ )
        {
            if( m_costring[ i ] )
                goto found;
        }

        for( i = 0; i < m_mincostindex; i++ )
        {
            if( m_costring[ i ] )
                goto found;
        }

        // Queue is empty
        m_mincostindex = 0;
#ifdef _DEBUG
        m_mincost = 0;
#endif
        return;

found:;
        m_mincostindex = i;
#ifdef _DEBUG
        wxASSERT( m_costring[ m_mincostindex ] );
        wxASSERT( m_costring[ m_mincostindex ]->m_cost > m_mincost );
        wxASSERT( ( m_costring[ m_mincostindex ]->m_cost & m_costmask ) == m_mincostindex );
        m_mincost = m_costring[ m_mincostindex ]->m_cost;
#endif
    }
}

a2dRouteData::a2dRouteData( a2dCanvasObject* showobject, bool final )
{
    //PRFL_AUTO_TMR( RerouteInit );

#ifdef DUMP_FINAL
    m_dump = fopen( "Routerdump.txt", "w" );
#endif

    m_ok = true;
    m_showobject = showobject;

    // Determine the raster size
    a2dBoundingBox bbox;

    // Go through all direct childs of the show object and add them to the bounding box
    a2dCanvasObjectList::iterator iter;
    for( iter = m_showobject->GetChildObjectList()->begin(); iter != m_showobject->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = ( *iter );

        // toolobjects are not considered directly
        // but if they are the editcopy of another object, they are
        if( obj->GetAlgoSkip() && a2dCanvasObject::PROPID_ToolObject->GetPropertyValue( obj ) )
            continue;

        // the editcopies are the objects that move around, so its new location is made part
        // of the routing grid, because the newly routed wire has to reach to these objects.
        a2dCanvasObject* editcopy = a2dCanvasObject::PROPID_Editcopy->GetPropertyValue( obj );
        if( !final && editcopy )
            obj = editcopy;

        a2dBoundingBox objbbox = CalculateRoutingBbox( obj );
        bbox.Expand( objbbox );
        //wxLogDebug("Objc %d %p %p %lg %lg %lg %lg %s", final, obj, editcopy, objbbox.GetMinX(), objbbox.GetMaxX(), objbbox.GetMinY(), objbbox.GetMaxY(), obj->GetClassInfo()->GetClassName() );
    }

    m_rasterborder = 20;
    bbox.Enlarge( m_rasterborder );

    m_rasterinv = 1.0 / m_raster;
    m_rasterminx = ( int ) floor( bbox.GetMinX() * m_rasterinv );
    m_rastermaxx = ( int ) ceil( bbox.GetMaxX() * m_rasterinv );
    m_rasterminy = ( int ) floor( bbox.GetMinY() * m_rasterinv );
    m_rastermaxy = ( int ) ceil( bbox.GetMaxY() * m_rasterinv );

    // wxLogDebug("BBox %d %lg %lg %lg %lg", final, m_rasterminx, m_rastermaxx, m_rasterminy, m_rastermaxy );

    // Limit raster size to something reasonable
    if( m_rastermaxx - m_rasterminx > 10000 )
    {
        m_ok = false;
        return;
    }
    if( m_rastermaxy - m_rasterminy > 10000 )
    {
        m_ok = false;
        return;
    }

    // Allocate and clear memory
    m_width = m_rastermaxx - m_rasterminx;
    m_widthp1 = m_width + 1;
    m_height = m_rastermaxy - m_rasterminy;
    m_heightp1 = m_height + 1;

    m_verticaloccupation = new unsigned short[ m_widthp1 * m_heightp1 ];
    m_horizontaloccupation = new unsigned short[ m_widthp1 * m_heightp1 ];
    m_routepoints = new RoutePoint[ m_widthp1 * m_heightp1 * dir_count ];

    if( !m_verticaloccupation || !m_horizontaloccupation || !m_routepoints )
    {
        m_ok = false;
        return;
    }

    memset( m_verticaloccupation, 0, m_widthp1 * m_heightp1 * sizeof( *m_verticaloccupation ) );
    memset( m_horizontaloccupation, 0, m_widthp1 * m_heightp1 * sizeof( *m_horizontaloccupation ) );
    memset( m_routepoints, 0, m_widthp1 * m_heightp1 * dir_count * sizeof( *m_routepoints ) );

    //PRFL_CNT_MEAN( RerouteInitSize, m_widthp1 * m_heightp1 );

    // Go through all direct childs of the show object and add them to the occupation arrays
    for( iter = m_showobject->GetChildObjectList()->begin(); iter != m_showobject->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = ( *iter );
        // toolobjects are not considered directly
        // but if they are the editcopy of another object, they are
        if( obj->GetAlgoSkip() && a2dCanvasObject::PROPID_ToolObject->GetPropertyValue( obj ) )
            continue;

        a2dCanvasObject* editcopy = a2dCanvasObject::PROPID_Editcopy->GetPropertyValue( obj );
        // To prevent that the dragged objects (editcopies) get rerouted wires on top of themselfs.
        if( !final && editcopy )
            obj = editcopy;

        // wires that are to be rerouted are also not added
        a2dWirePolylineL* wire = wxDynamicCast( obj, a2dWirePolylineL );
        if( wire && wire->GetReroute() )
        {
            wire->SetRerouteAdded( false );
            continue;
        }

        if( wxDynamicCast( obj, a2dPolygonL ) )
        {
            AddOccupationPolyline( wxStaticCast( obj, a2dPolygonL )->GetSegments(), obj->GetTransformMatrix(), 1 );
        }
        else
        {
            a2dBoundingBox bbox = CalculateRoutingBbox( obj );
            AddOccupationRect( bbox, 1 );
        }
    }

#ifdef DUMP_FINAL
    if( final )
    {
        fprintf( m_dump, "Route init\n" );
        DumpOccupation( m_dump );
    }
#endif
}

a2dRouteData::~a2dRouteData()
{
    delete [] m_verticaloccupation;
    delete [] m_horizontaloccupation;
    delete [] m_routepoints;
#ifdef DUMP_FINAL
    //fclose( m_dump );
#endif
}

// WX_DEFINE_SORTED_ARRAY( a2dRouteData::BorderPoint*, BorderQueue );

void a2dRouteData::AddBorderPoint(
    BorderQueue* queue, RoutePoint* current,
    int nextx, int nexty,
    int dir, int prevdir
)
{
    //PRFL_AUTO_TMR( AddBorderPoint );
    //PRFL_CNT_MEAN( AddBorderPointC, queue->m_count );

    wxASSERT( nextx >= 0 && nextx <= m_width );
    wxASSERT( nexty >= 0 && nexty <= m_height );
    wxASSERT( dir >= dir_min && dir <= dir_max );
    wxASSERT( prevdir >= dir_min && prevdir <= dir_max );

    RoutePoint* next = &GetRoutePoint( nextx, nexty, dir );
    if( next->m_flags & flag_reachable )
    {
        return;
    }

    // Don't go back to where we came from
    if( ( dir ^ prevdir ) == dir_invxor )
    {
        return;
    }

    BorderPoint border;

    border.m_x = nextx;
    border.m_y = nexty;
    border.m_direction = dir;
    border.m_prevdir = prevdir;

    border.m_cost = current->m_cost;

    if( next->m_flags & flag_original )
    {
        // general path cost for points along the original wire
        border.m_cost += 1;
    }
    else
    {
        // general path cost for points not along the original wire
        border.m_cost += 3;
    }

    // extra path cost for corners
    if(
        // either point is non-original
        !( ( current->m_flags & flag_original ) && ( next->m_flags & flag_original ) ) &&
        // and this is a corner
        current->m_direction != border.m_direction
    )
    {
        border.m_cost += 10;
    }

    // extra path cost for crossing occupied areas
    if( IsHorizontalOccupied( nextx, nexty ) || IsVerticalOccupied( nextx, nexty ) )
    {
        border.m_cost += 10;
    }

    queue->Add( border );
}

bool a2dRouteData::RerouteWire( a2dWirePolylineL* wire, a2dPin* dispin, a2dPin* startpin, bool startisbegin, bool final )
{
    a2dPin* maxDisLocated = dispin->IsDislocated();

    // Note on parameteres:
    // startpin may be Null if a wire was half routed

    // wxLogDebug("Route %p %d", wire, wire->GetSegments()->GetCount() );

    //PRFL_AUTO_TMR( RerouteWire );
    if( !m_ok )
    {
        return false;
    }

    // Clear routing point data
    memset( m_routepoints, 0, m_widthp1 * m_heightp1 * dir_count * sizeof( *m_routepoints ) );

    /*  This is for dumping the last pre-final and the final route
        After the first routing you have 2 seconds to get close to final, then after
        2 seconds make a final move and release the mouse
        static clock_t start = clock();
        if( clock()-start > 2000 || final )
        {
            wxLogDebug("Route %p %d", wire, final );
            DumpOccupation();
        }
    */

#ifdef DUMP_FINAL
    if( final )
    {
        fprintf( m_dump, "Route Final 1 %p %d\n", wire, wire->GetSegments()->GetCount() );
        DumpOccupation( m_dump );
    }
#endif

    //PRFL_BGN_TMR( RerouteWireInit );
    // Set the original flags for all points on the wire
    SetFlagPolyline( wire->GetSegments(), wire->GetTransformMatrix(), flag_original );

    // Set the wire as not occupied
    if( wire->GetRerouteAdded() )
    {
        AddOccupationPolyline( wire->GetSegments(), wire->GetTransformMatrix(), -1 );
    }

    a2dBoundingBox bboxStartPin;
    a2dBoundingBox bboxDisPin;

    //if a connected pin is internal, make sure that the parent of that object is not occupied.
    //This here works for original and editcopies, because for editcopies a a2dWireEnd,
    //instead of the real object which is there,
    //is added at the ends of the non editing objects which are indirectly connected via wires.
    //Wires itself are already non occupied.
    //In case of a a2dWireEnd the original object is found via the property PROPID_Original.
    if ( startpin && startpin->IsConnectedTo() && startpin->IsConnectedTo()->IsInternal() )
    {
        a2dWireEnd* end = wxDynamicCast( startpin->IsConnectedTo()->GetParent(), a2dWireEnd );
        if ( end )
        {
            a2dCanvasObject* org = a2dCanvasObject::PROPID_Original->GetPropertyValue( end );
            bboxStartPin = CalculateRoutingBbox( org );
        }
        else
        {
            bboxStartPin = CalculateRoutingBbox( startpin->IsConnectedTo()->GetParent() );
        }
        AddOccupationRect( bboxStartPin, -1 );
    }

    if ( maxDisLocated && maxDisLocated->IsInternal() )
    {
        a2dWireEnd* end = wxDynamicCast( maxDisLocated->GetParent(), a2dWireEnd );
        if ( end )
        {
            a2dCanvasObject* org = a2dCanvasObject::PROPID_Original->GetPropertyValue( end );
            bboxDisPin = CalculateRoutingBbox( org );
        }
        else
        {
            bboxDisPin = CalculateRoutingBbox( maxDisLocated->GetParent() );
        }
        AddOccupationRect( bboxDisPin, -1 );
    }

    // Check if the target pin is at an exact grid location
    a2dPoint2D targetpos = maxDisLocated->GetAbsXY();

    // TARGET is where to route the wire needs to route to.
    // Check if the target is a wire
    if( wxDynamicCast( maxDisLocated->GetParent(), a2dWirePolylineL ) )
    {
        // Target is a wire
        a2dWirePolylineL* target = wxStaticCast( maxDisLocated->GetParent(), a2dWirePolylineL );

        SetFlagPolyline( target->GetSegments(), target->GetTransformMatrix(), flag_targetwire );
    }
    else if( startpin && startpin->IsConnectedTo() && wxDynamicCast( startpin->IsConnectedTo()->GetParent(), a2dWirePolylineL ) )
    {
        a2dWirePolylineL* target = wxStaticCast( startpin->IsConnectedTo()->GetParent(), a2dWirePolylineL );

        // we start on a wire to an object, so routing in reverse direction is usually better
        // because we have more freedom, so swap the route
        a2dPin* h;
        h = dispin;
        dispin = startpin;
        startpin = h;
        startisbegin = !startisbegin;

        // The startpin (was dispin)  is dislocated, so first relocate it
        // Get the wire local coordinate of the new position of the start pin
        a2dPoint2D newstartpos = maxDisLocated->GetAbsXY();

        // target was switched with startpin, but maybe startpin was not dislocated at all, so take the best choice.
        h = dispin->IsDislocated();
        if ( h )
            targetpos = h->GetAbsXY();
        else
            targetpos = dispin->IsConnectedTo()->GetAbsXY();

        a2dAffineMatrix trns = wire->GetTransformMatrix();
        trns.Invert();
        trns.TransformPoint( &newstartpos );
        // Set this position to the start pin
        if ( target->GetRoot()->GetCommandProcessor() && final )
            target->GetRoot()->GetCommandProcessor()->Submit(
                new a2dCommand_SetCanvasProperty( startpin, a2dCanvasObject::PROPID_Position, newstartpos, -1, true ), final );
        else
            startpin->SetPosXyPoint( newstartpos );

        SetFlagPolyline( target->GetSegments(), target->GetTransformMatrix(), flag_targetwire );
    }

    if(
        floor( targetpos.m_x * m_rasterinv ) * m_raster == targetpos.m_x &&
        floor( targetpos.m_y * m_rasterinv ) * m_raster == targetpos.m_y
    )
    {
        // Pin is at exact raster location
        int x = ( int ) floor( ( targetpos.m_x + 0.5 * m_raster ) * m_rasterinv ) - m_rasterminx;
        int y = ( int ) floor( ( targetpos.m_y + 0.5 * m_raster ) * m_rasterinv ) - m_rasterminy;
        SetFlagRoutePointAllDirs( x, y, flag_targetpin );
    }
    else
    {
        // Approximate raster location
        int x1 = ( int ) floor( targetpos.m_x * m_rasterinv ) - m_rasterminx;
        int y1 = ( int ) floor( targetpos.m_y * m_rasterinv ) - m_rasterminy;
        int x2 = ( int ) ceil( targetpos.m_x * m_rasterinv ) - m_rasterminx;
        int y2 = ( int ) ceil( targetpos.m_y * m_rasterinv ) - m_rasterminy;
        SetFlagRoutePointAllDirs( x1, y1, flag_targetapprox );
        SetFlagRoutePointAllDirs( x2, y1, flag_targetapprox );
        SetFlagRoutePointAllDirs( x1, y2, flag_targetapprox );
        SetFlagRoutePointAllDirs( x2, y2, flag_targetapprox );
    }

#ifdef DUMP_FINAL
    if( final )
    {
        fprintf( m_dump, "Route Final 2 %p %d\n", wire, wire->GetSegments()->GetCount() );
        DumpOccupation( m_dump );
    }
#endif

    // Start routing
    a2dPoint2D startpos;
    if( !startpin )
    {
        // There might be no startpin when a wire is dangling
        // In this case use wire vertex coordinates
        startpos = startisbegin ? wire->GetSegments()->front()->GetPoint() : wire->GetSegments()->back()->GetPoint();
        wire->GetTransformMatrix().TransformPoint( &startpos );
    }
    else
    {
        startpos = startpin->GetAbsXY();
    }

    int startx = ( int ) floor( ( startpos.m_x + 0.5 * m_raster ) * m_rasterinv ) - m_rasterminx;
    int starty = ( int ) floor( ( startpos.m_y + 0.5 * m_raster ) * m_rasterinv ) - m_rasterminy;

    BorderQueue queue;

    for( int dir = dir_min; dir <= dir_max; dir++ )
    {
        BorderPoint start;
        start.m_x = startx;
        start.m_y = starty;
        start.m_direction = dir;
        start.m_prevdir = dir_start;
        start.m_cost = 0;

        queue.Add( start );
    }

    RoutePoint* end = 0;

    //PRFL_END_TMR( RerouteWireInit );

    while( queue.IsNotEmpty() )
    {
        // Get current minimum cost point
        const BorderPoint& border = queue.GetBest();

        RoutePoint* point = &GetRoutePoint( border.m_x, border.m_y, border.m_direction );

        if( point->m_flags & flag_reachable )
        {
            // The point was already reached at lower cost
            queue.RmvBest();
            continue;
        }

        point->m_x = border.m_x;
        point->m_y = border.m_y;
        point->m_direction = border.m_direction;
        point->m_prevdir = border.m_prevdir;
        point->m_cost = border.m_cost;

        // Check if we reached a target point
        if( point->m_flags & ( flag_targetwire | flag_targetpin | flag_targetapprox ) )
        {
            // Ok, target reached
            end = point;
            break;
        }

        // Check occupation of this point depending on direction
        if( border.m_prevdir != dir_start )
        {
            switch( border.m_direction )
            {
                case dir_xminus:
                case dir_xplus:
                    if( IsHorizontalOccupied( border.m_x, border.m_y ) )
                    {
                        // The point is occupied in this direction
                        queue.RmvBest();
                        continue;
                    }
                    break;
                case dir_yminus:
                case dir_yplus:
                    if( IsVerticalOccupied( border.m_x, border.m_y ) )
                    {
                        // The point is occupied in this direction
                        queue.RmvBest();
                        continue;
                    }
                    break;
            }
        }

        // the point is reachable
        point->m_flags |= flag_reachable;

        //PRFL_BGN_TMR( RerouteWireAddBlock );

        if( !IsVerticalOccupied( point->m_x, point->m_y ) || border.m_prevdir == dir_start )
        {
            // Check going into Y- (top) direction
            // The dir_xxx given is the direction of source viewed from the target, so it is inverse
            if( point->m_y > 0 )
            {
                AddBorderPoint(
                    &queue, point,
                    point->m_x, point->m_y - 1,
                    dir_yplus, point->m_direction
                );
            }

            // Check going into Y+ (bottom) direction
            // The dir_xxx given is the direction of source viewed from the target, so it is inverse
            if( point->m_y < m_height )
            {
                AddBorderPoint(
                    &queue, point,
                    point->m_x, point->m_y + 1,
                    dir_yminus, point->m_direction
                );
            }
        }

        if( !IsHorizontalOccupied( point->m_x, point->m_y ) || border.m_prevdir == dir_start  )
        {
            // Check going into X- (left) direction
            // The dir_xxx given is the direction of source viewed from the target, so it is inverse
            if( point->m_x > 0 )
            {
                AddBorderPoint(
                    &queue, point,
                    point->m_x - 1, point->m_y,
                    dir_xplus, point->m_direction
                );
            }

            // Check going into X+ (right) direction
            // The dir_xxx given is the direction of source viewed from the target, so it is inverse
            if( point->m_x < m_width )
            {
                AddBorderPoint(
                    &queue, point,
                    point->m_x + 1, point->m_y,
                    dir_xminus, point->m_direction
                );
            }
        }

        //PRFL_END_TMR( RerouteWireAddBlock );

        wxASSERT( &queue.GetBest() == &border );
        queue.RmvBest();
    }

    // queue.Clear();

#ifdef DUMP_FINAL
    if( final )
    {
        fprintf( m_dump, "Route Final 3 %p %d\n", wire, wire->GetSegments()->GetCount() );
        DumpOccupation( m_dump );
    }
#endif

    if ( bboxStartPin.GetValid() )
    {
        AddOccupationRect( bboxStartPin, 1 );
    }

    if ( bboxDisPin.GetValid() )
    {
        AddOccupationRect( bboxDisPin, 1 );
    }

    if( !end )
    {
        //wxLogDebug( "missed" );
        // Target not reached => create a straight line wire
        a2dVertexList* newpoints = new a2dVertexList;
        if( startisbegin )
        {
            newpoints->push_back( new a2dLineSegment( startpos ) );
            newpoints->push_back( new a2dLineSegment( targetpos ) );
        }
        else
        {
            newpoints->push_back( new a2dLineSegment( targetpos ) );
            newpoints->push_back( new a2dLineSegment( startpos ) );
        }
        // Adjust the existing vertex list to the new vertex list
        if ( wire->GetRoot()->GetCommandProcessor() && final )
            wire->GetRoot()->GetCommandProcessor()->Submit( new a2dCommand_SetSegments( wire, newpoints, true ), final );
        else
            wire->SetSegments( newpoints );

        // Set the wire as occupied, to prevent other wires that will still be rerouted, are reroute on top.
        AddOccupationPolyline( wire->GetSegments(), wire->GetTransformMatrix(), 1 );
        wire->SetRerouteAdded( true );
        return false;
    }
    else
    {
        //wxLogDebug( "oke" );
        // Create a new vertex list for the wire
        a2dVertexList* newpoints = new a2dVertexList;
        RoutePoint* current = end;
        int prevdir = -1;

        a2dPoint2D endpoint( ( current->m_x + m_rasterminx ) * m_raster, ( current->m_y + m_rasterminy ) * m_raster );

        if( end->m_flags & flag_targetapprox )
        {
            // Target is only approximate, so add a point at the real target
            if( startisbegin )
                newpoints->push_front( new a2dLineSegment( targetpos ) );
            else
                newpoints->push_back( new a2dLineSegment( targetpos ) );

            // Add extra point, to avoid 45 degree line pieces
            a2dLineSegment* p = NULL;
            a2dPoint2D point( ( current->m_x + m_rasterminx ) * m_raster, ( current->m_y + m_rasterminy ) * m_raster );
            if ( current->m_direction == dir_xminus || current->m_direction == dir_xplus )
            {
                if ( point.m_y != targetpos.m_y )
                    p = new a2dLineSegment( targetpos.m_x, point.m_y );
                prevdir = current->m_direction;
            }
            else if ( current->m_direction == dir_yminus || current->m_direction == dir_yplus )
            {
                if ( point.m_x != targetpos.m_x )
                    p = new a2dLineSegment( point.m_x, targetpos.m_y );
                prevdir = current->m_direction;
            }
            if ( p )
            {
                if( startisbegin )
                    newpoints->push_front( p );
                else
                    newpoints->push_back( p );
            }
        }
        else if( end->m_flags & flag_targetwire )
        {
            // If this doesn't fit exactly, insert a line
            a2dPoint2D dispinpos = targetpos;

            if( dispinpos != endpoint )
            {
                if( startisbegin )
                    newpoints->push_front( new a2dLineSegment( dispinpos ) );
                else
                    newpoints->push_back( new a2dLineSegment( dispinpos ) );

                // Add extra point, to avoid 45 degree line pieces
                a2dLineSegment* p = NULL;
                a2dPoint2D point( ( current->m_x + m_rasterminx ) * m_raster, ( current->m_y + m_rasterminy ) * m_raster );
                if ( current->m_direction == dir_xminus || current->m_direction == dir_xplus )
                {
                    if ( point.m_y != dispinpos.m_y )
                        p = new a2dLineSegment( dispinpos.m_x, point.m_y );
                    prevdir = current->m_direction;
                }
                else if ( current->m_direction == dir_yminus || current->m_direction == dir_yplus )
                {
                    if ( point.m_x != dispinpos.m_x )
                        p = new a2dLineSegment( point.m_x, dispinpos.m_y );
                    prevdir = current->m_direction;
                }
                if ( p )
                {
                    if( startisbegin )
                        newpoints->push_front( p );
                    else
                        newpoints->push_back( p );
                }
            }
        }


        //reconstruct poinst from route path found.
        for( ;; )
        {
            current->m_flags |= flag_final;

            a2dLineSegment point( ( current->m_x + m_rasterminx ) * m_raster, ( current->m_y + m_rasterminy ) * m_raster );

            if( current->m_prevdir == dir_start )
            {
                //reached the special marked point, with direction dir_start.
                if( startisbegin )
                    newpoints->push_front( new a2dLineSegment( point ) );
                else
                    newpoints->push_back( new a2dLineSegment( point ) );

                // at the beginning the startpin location was rounded to the grid.
                // if the startpin was not on the grid, add an extra segment to reach it.
                if( startpin )
                {
                    if (
                        floor( point.m_x * m_rasterinv ) * m_raster == startpin->GetPosX() &&
                        floor( point.m_y * m_rasterinv ) * m_raster == startpin->GetPosY()
                    )
                    {
                        //DONE reconstruction of wire from routing points.
                    }
                    else
                    {
                        if( startisbegin )
                            newpoints->push_front( new a2dLineSegment( startpin->GetPosX(), startpin->GetPosY() ) );
                        else
                            newpoints->push_back( new a2dLineSegment( startpin->GetPosX(), startpin->GetPosY() ) );
                    }
                }
                break;
            }

            if( current->m_direction != prevdir )
            {
                if( startisbegin )
                    newpoints->push_front( new a2dLineSegment( point ) );
                else
                    newpoints->push_back( new a2dLineSegment( point ) );
                prevdir = current->m_direction;
            }

            int x = current->m_x;
            int y = current->m_y;
            switch( current->m_direction )
            {
                case dir_xminus:
                    x--;
                    break;
                case dir_xplus:
                    x++;
                    break;
                case dir_yminus:
                    y--;
                    break;
                case dir_yplus:
                    y++;
                    break;
            }

            current = &GetRoutePoint( x, y, current->m_prevdir );
        }

        // Adjust the existing vertex list to the new vertex list
        if ( wire->GetRoot()->GetCommandProcessor() && final )
            wire->GetRoot()->GetCommandProcessor()->Submit( new a2dCommand_SetSegments( wire, newpoints, true ), final );
        else
            wire->SetSegments( newpoints );
        wire->SetPending( true );

        // Set the wire as occupied, to prevent other wires that will still be rerouted, are reroute on top.
        AddOccupationPolyline( wire->GetSegments(), wire->GetTransformMatrix(), 1 );
        wire->SetRerouteAdded( true );

#ifdef DUMP_FINAL
        if( final )
        {
            fprintf( m_dump, "Route Final 4 %p %d\n", wire, wire->GetSegments()->GetCount() );
            DumpOccupation( m_dump );
        }
#endif

        return true;
    }
}

void a2dRouteData::AddOccupationRect( const a2dBoundingBox& bbox, short incr )
{
    int minx = ( int ) floor( ( bbox.GetMinX() + 0.75 * m_raster ) * m_rasterinv );
    int maxx = ( int ) ceil ( ( bbox.GetMaxX() - 0.75 * m_raster ) * m_rasterinv );
    int miny = ( int ) floor( ( bbox.GetMinY() + 0.75 * m_raster ) * m_rasterinv );
    int maxy = ( int ) ceil ( ( bbox.GetMaxY() - 0.75 * m_raster ) * m_rasterinv );

    if( minx < m_rasterminx ) minx = m_rasterminx;
    if( maxx > m_rastermaxx ) maxx = m_rastermaxx;
    if( miny < m_rasterminy ) miny = m_rasterminy;
    if( maxy > m_rastermaxy ) maxy = m_rastermaxy;

    int minxi = ( minx - m_rasterminx );
    int maxxi = ( maxx - m_rasterminx );
    int minyi = ( miny - m_rasterminy );
    int maxyi = ( maxy - m_rasterminy );

    int x, y;

    for( y = minyi; y <= maxyi; y++ )
    {
        for( x = minxi; x <= maxxi; x++ )
        {
            unsigned short& h = GetHorizontalOccupation( x, y );
            unsigned short& v = GetVerticalOccupation( x, y );
            if ( incr < 0 )
            {
                // Klaas If this is a problem,fix it.
                // < 0 should be possible, to keep track of the number of time not occupied!
                //wxASSERT( h >= -incr &&  v >= -incr );
            }
            h += incr;
            v += incr;
        }
    }
}

void a2dRouteData::AddOccupationPolyline( const a2dVertexList* points, const a2dAffineMatrix& trns, short incr )
{
    a2dVertexList::const_iterator iter = points->begin();
    a2dVertexList::const_iterator prev = iter;
    iter++;

    for( ; iter != points->end(); ++iter )
    {
        double x1l = ( *prev )->m_x;
        double y1l = ( *prev )->m_y;
        double x2l = ( *iter )->m_x;
        double y2l = ( *iter )->m_y;
        double x1, y1, x2, y2;

        trns.TransformPoint( x1l, y1l, x1, y1 );
        trns.TransformPoint( x2l, y2l, x2, y2 );

        int minx = ( int ) floor( ( wxMin( x1, x2 ) + 0.5 * m_raster ) * m_rasterinv );
        int maxx = ( int ) ceil ( ( wxMax( x1, x2 ) - 0.5 * m_raster ) * m_rasterinv );
        int miny = ( int ) floor( ( wxMin( y1, y2 ) + 0.5 * m_raster ) * m_rasterinv );
        int maxy = ( int ) ceil ( ( wxMax( y1, y2 ) - 0.5 * m_raster ) * m_rasterinv );

        if( minx < m_rasterminx ) minx = m_rasterminx;
        if( maxx > m_rastermaxx ) maxx = m_rastermaxx;
        if( miny < m_rasterminy ) miny = m_rasterminy;
        if( maxy > m_rastermaxy ) maxy = m_rastermaxy;

        int minxi = ( minx - m_rasterminx );
        int maxxi = ( maxx - m_rasterminx );
        int minyi = ( miny - m_rasterminy );
        int maxyi = ( maxy - m_rasterminy );

        if( minxi == maxxi )
        {
            // vertical line
            for( int y = minyi; y <= maxyi; y++ )
            {
                GetVerticalOccupation( minxi, y ) += incr;
            }
        }
        else if( minyi == maxyi )
        {
            // horizontal line
            for( int x = minxi; x <= maxxi; x++ )
            {
                GetHorizontalOccupation( x, minyi ) += incr;
            }
        }
        // Diagonal lines don't accupy anything

        prev = iter;
    }
}

void a2dRouteData::SetFlagRect( const a2dBoundingBox& bbox, RoutePointFlag flag )
{
    int minx = ( int ) floor( ( bbox.GetMinX() + 0.5 * m_raster ) * m_rasterinv );
    int maxx = ( int ) ceil ( ( bbox.GetMaxX() - 0.5 * m_raster ) * m_rasterinv );
    int miny = ( int ) floor( ( bbox.GetMinY() + 0.5 * m_raster ) * m_rasterinv );
    int maxy = ( int ) ceil ( ( bbox.GetMaxY() - 0.5 * m_raster ) * m_rasterinv );

    if( minx < m_rasterminx ) minx = m_rasterminx;
    if( maxx > m_rastermaxx ) maxx = m_rastermaxx;
    if( miny < m_rasterminy ) miny = m_rasterminy;
    if( maxy > m_rastermaxy ) maxy = m_rastermaxy;

    int minxi = ( minx - m_rasterminx );
    int maxxi = ( maxx - m_rasterminx );
    int minyi = ( miny - m_rasterminy );
    int maxyi = ( maxy - m_rasterminy );

    int x, y;

    for( y = minyi; y <= maxyi; y++ )
    {
        for( x = minxi; x <= maxxi; x++ )
        {
            SetFlagRoutePointAllDirs( x, y, flag );
        }
    }
}

void a2dRouteData::SetFlagPolyline( const a2dVertexList* points, const a2dAffineMatrix& trns, RoutePointFlag flag )
{
    a2dVertexList::const_iterator iter = points->begin();
    a2dVertexList::const_iterator prev = iter;
    iter++;

    for( ; iter != points->end(); ++iter )
    {
        // klion: warning C4189: 'obj' : local variable is initialized but not referenced
        // a2dLineSegment *obj = (*iter);

        double x1l = ( *prev )->m_x;
        double y1l = ( *prev )->m_y;
        double x2l = ( *iter )->m_x;
        double y2l = ( *iter )->m_y;
        double x1, y1, x2, y2;

        trns.TransformPoint( x1l, y1l, x1, y1 );
        trns.TransformPoint( x2l, y2l, x2, y2 );

        //calculate the bbox of the line segment in raster coordinates.
        int minx = ( int ) floor( ( wxMin( x1, x2 ) + 0.5 * m_raster ) * m_rasterinv );
        int maxx = ( int ) ceil ( ( wxMax( x1, x2 ) - 0.5 * m_raster ) * m_rasterinv );
        int miny = ( int ) floor( ( wxMin( y1, y2 ) + 0.5 * m_raster ) * m_rasterinv );
        int maxy = ( int ) ceil ( ( wxMax( y1, y2 ) - 0.5 * m_raster ) * m_rasterinv );

        if( minx < m_rasterminx ) minx = m_rasterminx;
        if( maxx > m_rastermaxx ) maxx = m_rastermaxx;
        if( miny < m_rasterminy ) miny = m_rasterminy;
        if( maxy > m_rastermaxy ) maxy = m_rastermaxy;

        int minxi = ( minx - m_rasterminx );
        int maxxi = ( maxx - m_rasterminx );
        int minyi = ( miny - m_rasterminy );
        int maxyi = ( maxy - m_rasterminy );

        if( minxi == maxxi )
        {
            // vertical line
            for( int y = minyi; y <= maxyi; y++ )
            {
                SetFlagRoutePointAllDirs( minxi, y, flag );
            }
        }
        else if( minyi == maxyi )
        {
            // horizontal line
            for( int x = minxi; x <= maxxi; x++ )
            {
                SetFlagRoutePointAllDirs( x, minyi, flag );
            }
        }
        // Diagonal lines don't occupy anything

        prev = iter;
    }
}

a2dBoundingBox a2dRouteData::CalculateRoutingBbox( a2dCanvasObject* object )
{
    // This is not quite optimal.
    // It would probably be best to have a flag parameter to GetCalculatedBbox
    a2dBoundingBox box = object->GetUnTransformedBbox( false );

    forEachIn( a2dCanvasObjectList, object->GetChildObjectList() )
    {
        a2dCanvasObject* obj = *iter;
        if( !wxDynamicCast( obj, a2dPin ) )
        {
            box.Expand( obj->GetCalculatedBoundingBox( 0 ) );
        }
    }

    box.MapBbox( object->GetTransformMatrix() );

    return box;

}


void a2dRouteData::DumpOccupation( FILE* file )
{
#ifdef _DEBUG
    if( file )
        fprintf( file, "\nOccupation %d x %d\n", m_width, m_height );
    else
        wxLogDebug( _T( "\nOccupation %d x %d" ), m_width, m_height );

    int x, y;

    wxChar* buffer = new wxChar [ m_widthp1 * 2 + 1 ];
    wxChar* pos;

    for( y = 0; y <= m_height; y++ )
    {
        // First Dump horizontal edges and points
        pos = buffer;
        for( x = 0; x <= m_width; x++ )
        {
            unsigned short ho = GetHorizontalOccupation( x, y );
            unsigned short vo = GetVerticalOccupation( x, y );
            RoutePoint* point0 = &GetRoutePoint( x, y, 0 );
            RoutePoint* point1 = &GetRoutePoint( x, y, 1 );
            RoutePoint* point2 = &GetRoutePoint( x, y, 2 );
            RoutePoint* point3 = &GetRoutePoint( x, y, 3 );

            if( ho == 0 && vo == 0 )
                *pos++ = wxT( '.' );
            else if( ho == 1 && vo == 0 )
                *pos++ = wxT( '-' );
            else if( ho == 0 && vo == 1 )
                *pos++ = wxT( '|' );
            else if( ho == 1 && vo == 1 )
                *pos++ = wxT( 'x' );
            else if( ho == 2 && vo == 0 )
                *pos++ = wxT( '=' );
            else if( ho == 0 && vo == 2 )
                *pos++ = wxT( '"' );
            else
                *pos++ = wxT( 'X' );

            if(
                ( point0->m_flags & flag_final ) ||
                ( point1->m_flags & flag_final ) ||
                ( point2->m_flags & flag_final ) ||
                ( point3->m_flags & flag_final )
            )
                *pos++ = wxT( '#' );
            else if( point0->m_flags & flag_targetpin )
                *pos++ = wxT( 'P' );
            else if( point0->m_flags & flag_targetapprox )
                *pos++ = wxT( 'A' );
            else if( point0->m_flags & flag_targetwire )
                *pos++ = wxT( 'W' );
            else if(
                ( point0->m_flags & flag_reachable ) ||
                ( point1->m_flags & flag_reachable ) ||
                ( point2->m_flags & flag_reachable ) ||
                ( point3->m_flags & flag_reachable )
            )
                *pos++ = wxT( 'R' );
            else
                *pos++ = wxT( ' ' );
        }
        *pos++ = 0;
        if( file )
            fprintf( file, "%s\n", buffer );
        else
            wxLogDebug( buffer );
    }

    delete [] buffer;
#endif
}

void a2dRouteData::DumpCost()
{
#ifdef _DEBUG
    wxLogDebug( _T( "\nCost" ) );
    int x, y;

    wxChar* buffer = new wxChar [ m_widthp1 * 3 * 2 + 1 ];
    wxChar* pos;

    for( y = 0; y <= m_height; y++ )
    {
        pos = buffer;
        for( x = 0; x <= m_width; x++ )
        {
            RoutePoint* point = &GetRoutePoint( x, y, dir_xminus );
            int val = point->m_cost;

            *pos++ = val / 10 % 10 + wxT( '0' );
            *pos++ = val % 10 + wxT( '0' );
            *pos++ = wxT( ' ' );

            point = &GetRoutePoint( x, y, dir_yminus );
            val = point->m_cost;

            *pos++ = val / 10 % 10 + wxT( '0' );
            *pos++ = val % 10 + wxT( '0' );
            *pos++ = wxT( '|' );
        }
        *pos++ = 0;
        wxLogDebug( buffer );

        pos = buffer;
        for( x = 0; x <= m_width; x++ )
        {
            RoutePoint* point = &GetRoutePoint( x, y, dir_yplus );
            int val = point->m_cost;

            *pos++ = val / 10 % 10 + wxT( '0' );
            *pos++ = val % 10 + wxT( '0' );
            *pos++ = wxT( ' ' );

            point = &GetRoutePoint( x, y, dir_xplus );
            val = point->m_cost;

            *pos++ = val / 10 % 10 + wxT( '0' );
            *pos++ = val % 10 + wxT( '0' );
            *pos++ = wxT( '|' );
        }
        *pos++ = 0;
        wxLogDebug( buffer );
        wxLogDebug( _T( "" ) );
    }

    delete [] buffer;
#endif
}

void a2dRouteData::DumpVertexList( a2dVertexList* list )
{
#ifdef _DEBUG
    wxLogDebug( wxT( "VertexList" ) );
    forEachIn( a2dVertexList, list )
    {
        wxLogDebug( wxT( "%12.3lf %12.3lf" ), ( *iter )->m_x, ( *iter )->m_y );
    }
#endif
};


