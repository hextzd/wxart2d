/*! \file wx/canvas/route.h
    \brief routing of wires.

    Classes for auto (re)routing wires (a2dWirePolylineL) are here.

    \author Michael Sögtrop

    Copyright: 2003-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: wire.h,v 1.9 2006/12/13 21:43:24 titato Exp $
*/

#ifndef __WXROUTE_H__
#define __WXROUTE_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/candefs.h"

class a2dWirePolylineL;

//! Class for rerouting wires
/*! This class implements a usual Lee router. The one special thing is, that there are
    penalties for corners. This makes routing a bit complicated, because the lowest cost
    to reach one point might not be the best for succeeding from this point, as the following
    example shows:

    0-1    0- 3
    |         |
    1-14     16
    |         |
    2-15     19
    |         |
    3-16     22
       |      |
      29     25

    The numbers are cost. 0 is the routing start point. The points with cost 1..3 are
    lying on the original line and have a low cost for this reason. The destination
    is the point with a cost of 29/25. The important point is, that the target can
    be reached via the right path with higher intermediate cost with lower final cost.

    To get true least cost routing, the cost is stored for every raster point for the
    four incomming directions.
*/

class A2DCANVASDLLEXP a2dRouteData : public a2dObject
{
public:
    //! Standard constructor takes the show object under which is routed
    /*! The boundingbox of the child objects in showobject is taken as the area in which to route
    a wire. This with a certain margin added, in order to route around the outer objects too.
    Temporary tool objects are skipped from this boundingbox.
    The boundingbox is divided into a rectangular grid, each grid of size m_raster.
    The grid points are points where a routed wire can be routed onto.
    Buffers are allocated to maintain information on the grid points, e.g. which grid points are
    occupied by objects, and therefore a wire can not be routed there.
    a2dWirePolylineL objects marked with property PROPID_rerouteadded, indicates a wire to be rerouted,
    and will not be added to the occupation area. Only polylines are currently added accurate to the occuption area.
    Meaning the right grid points are disabled for routing. For the other objects currently the boundingbox is used.
    */
    a2dRouteData( a2dCanvasObject* showobject, bool final );

    //! Destructor
    ~a2dRouteData();

    //! the size of the raster
    static void SetRaster( double raster ) { m_raster = raster; }

    //! Reroute a wire
    /*!
        Reroute the given wire, between a start and end pin.
        The start or end pin is used as a start for producing a border wave,
        which sets route points to a cost to reach that route point.
        At the same time the wave itself is expanded at the point with the best cost first.
        If that point is already reached through another part of the border wave,
        that point is removed from the wave, else based on the current border point a new point a new
        border point is created, followed by removing the current best border point.
        In the end the target pin or wire will be reached and the border wave will be empty.
        While the border wave is working itself like this through the routing points, the best route is
        via pointer in the route points.

        \param wire on input this is the wire in its original (edited but unadjusted) state
        \param dispin the pin that became disconnected
        \param startpin the pin where routing starts
        \param startisbegin the pin where routing starts is the begin point of the wire
        \param final last route to destination
        \return true if wire was changed
    */
    bool RerouteWire( a2dWirePolylineL* wire, a2dPin* dispin, a2dPin* startpin, bool startisbegin, bool final );

    //! flags for RoutePoint
    enum RoutePointFlag
    {
        flag_original     = 0x01,
        flag_targetwire   = 0x02,
        flag_targetpin    = 0x04,
        flag_targetapprox = 0x08,
        flag_reachable    = 0x10,
        flag_final        = 0x20
    };
    //! directions for RoutePoint
    enum RoutePointDirection
    {
        dir_xminus    = 0,
        dir_xplus     = 1,
        dir_yminus    = 2,
        dir_yplus     = 3,

        //! start point
        dir_start     = 4,

        //! min usual direction value
        dir_min       = 0,
        //! max usual direction value
        dir_max       = 3,
        //! number of usual direction values
        dir_count     = 4,
        //! result of dir1^dir2 for two opposing directions
        dir_invxor    = 1
    };
    //! the data structure holding the per point information
    /*!
        a two dimensional array of RoutePoint is used for routing information, like which points are excluded
        from routing because they are occupied by other objects.
        Next to that the route os the wire to be routed is stored via previous and next pointers.
        Per grid point four RoutePoint are maintained, for the four directions a wire can enter this grid point.
    */
    struct RoutePoint
    {
        //! minimum cost to reach this point
        unsigned int m_cost;
        //! various flags
        unsigned short m_flags;
        //! direction to the source (only set if reachable)
        unsigned char m_direction;
        //! direction of the previous point
        unsigned char m_prevdir;
        //! coordinates of this point (only set if reachable)
        unsigned short m_x, m_y;
    };

    //! An entry in the border queue
    /*!
        Used in BorderQueue, for storing the route of a wire.
    */
    struct BorderPoint
    {
        //! minimum cost to reach this point
        unsigned int m_cost;
        //! direction to the source
        unsigned char m_direction;
        //! direction of the previous point
        unsigned char m_prevdir;
        //! coordinates of this point
        unsigned short m_x, m_y;
        //! pointer to the next point with same cost in the queue or to a free point
        BorderPoint* m_next;
    };

    //! This is a priority queue for border points
    /*! This queue makes use of the following facts:
        - The priorities (costs) are discrete
        - As the relative cost from one point to its next is limited,
          there can only be priorities from the current min-priority to
          min-priority + max-dist in the queue, so a ring buffer can be used
        - priorities added to the list are always larger than the smallest
          priority in the list (step cost is >0).
    */
    class BorderQueue
    {
    public:
        //! constructor
        BorderQueue();
        //! destructor
        ~BorderQueue();
        //! Add a border object
        void Add( const BorderPoint& brdr );
        //! Get the best border point
        const BorderPoint& GetBest()
        {
            wxASSERT( m_costring[ m_mincostindex ] );
            return *m_costring[ m_mincostindex ];
        }
        //! Remove the best border point
        void RmvBest();
        //! Check if the queue is empty
        bool IsNotEmpty()
        {
            return m_costring[ m_mincostindex ] != 0;
        }

    protected:
        //! some constants
        enum
        {
            //! number of descrete cost values ( a power of 2 )
            m_ncost = 1024,
            //! bit mask to project a cost value to a ring buffer index (1-m_ncost)
            m_costmask = 1023,
        };

        //! Memory allocation block
        struct AllocBlock
        {
            enum
            {
                //! number of border points in an allocation block
                m_pointsperblock = 1024
            };
            BorderPoint m_memory[m_pointsperblock];
            AllocBlock* m_next;
        };

        //! ring index of the current minimum cost (starting point or ring buffer )
        unsigned m_mincostindex;
        //! The starting points of the cost lists
        BorderPoint* m_costring[m_ncost];
        //! The starting point of the free list
        BorderPoint* m_freelist;
        //! Current memory allocation block
        AllocBlock* m_memory;
        //! Number of free elements in memory allocation block
        int m_freememory;

    public:
#ifdef _DEBUG
        unsigned m_mincost;
#endif
#ifdef PRFL_ENBL
        int m_count;
#endif
    };

protected:

    //! Get an element of the vertical occupation array
    unsigned short& GetVerticalOccupation( int x, int y )
    {
        wxASSERT( x >= 0 && x <= m_width );
        wxASSERT( y >= 0 && y <= m_height );
        return m_verticaloccupation[ m_widthp1 * y + x ];
    }

    //! Get an element of the horizontal occupation array
    unsigned short& GetHorizontalOccupation( int x, int y )
    {
        wxASSERT( x >= 0 && x <= m_width );
        wxASSERT( y >= 0 && y <= m_height );
        return m_horizontaloccupation[ m_widthp1 * y + x ];
    }

    //! Is an element of the vertical occupation array
    bool IsVerticalOccupied( int x, int y )
    {
        wxASSERT( x >= 0 && x <= m_width );
        wxASSERT( y >= 0 && y <= m_height );
        return m_verticaloccupation[ m_widthp1 * y + x ] > 0;
    }

    //! Is an element of the horizontal occupation array
    bool IsHorizontalOccupied( int x, int y )
    {
        wxASSERT( x >= 0 && x <= m_width );
        wxASSERT( y >= 0 && y <= m_height );
        return m_horizontaloccupation[ m_widthp1 * y + x ] > 0;
    }

    //! Get an element of the route point array
    RoutePoint& GetRoutePoint( int x, int y, int dir )
    {
        wxASSERT( x >= 0 && x <= m_width );
        wxASSERT( y >= 0 && y <= m_height );
        wxASSERT( dir >= dir_min && dir <= dir_max );
        return m_routepoints[ ( m_widthp1 * y + x ) * dir_count + ( dir - dir_min ) ];
    }

    //! add a bounding box to the occupied area
    /*!
        All grid points within the rectangle are incremented by incr in the
        vertical and horizontal occupation buffers.
    */
    void AddOccupationRect( const a2dBoundingBox& bbox, short incr );

    //! add a polyline to the occupation area
    /*!
        All vertical and horizontal lines are set into the vertical and horizontal occupation areas
        as being occupied. That is the grid points where those lines pass, are incremented by incr.
    */
    void AddOccupationPolyline( const a2dVertexList* points, const a2dAffineMatrix& trns, short incr );

    //! Set a RoutePoint flag in a rectangle
    void SetFlagRect( const a2dBoundingBox& bbox, RoutePointFlag flag );

    //! Set a RoutePoint flag along a polyline
    /*!
        Vertical and horizontal segments result in a set in the routepoints which the cover.
        That is for all directions in a routepoint.
    */
    void SetFlagPolyline( const a2dVertexList* points, const a2dAffineMatrix& trns, RoutePointFlag flag );

    //! Set a flag in the route points for all directions
    void SetFlagRoutePointAllDirs( int x, int y, RoutePointFlag flag )
    {
        RoutePoint* routepoints = &GetRoutePoint( x, y, dir_min );
        routepoints[0].m_flags |= flag;
        routepoints[1].m_flags |= flag;
        routepoints[2].m_flags |= flag;
        routepoints[3].m_flags |= flag;
    }

    //! Add a new point on the route
    /*! Based on the point where we are and a next point and direction,
        a new border point is added (unless already reached).
        The cost for the new point is calculated based on:
        \li if is on the orginal wire +1
        \li not on original wire +3
        \li corner compared to current point direction +10
        \li crossing occupied area +10 (in case of diagonal lines?)

        This is a large function, but it is inlined for speed
    */
    inline void AddBorderPoint(
        BorderQueue* queue, RoutePoint* current,
        int nextx, int nexty,
        int dir, int prevdir
    );

    //! Calculates the routing relevant bounding box of an object
    /*! This includes one level of childs, but no pins
        Pins are not included because their size may change during routing and
        this might lead to pre-final / final inconsistencies.
    */
    a2dBoundingBox CalculateRoutingBbox( a2dCanvasObject* object );

    //! Dump the occupation arrays
    void DumpOccupation( FILE* file );
    void DumpCost();
    void DumpVertexList( a2dVertexList* list );

    //! if true, the raster is initialized
    bool m_ok;
    //! the show object given in the constructor
    a2dCanvasObject* m_showobject;
    //! width of the routing raster array
    int m_width;
    //! width of the routing raster array + 1
    int m_widthp1;
    //! height of the routing raster array
    int m_height;
    //! height of the routing raster array + 1
    int m_heightp1;
    //! the size of the raster
    static double m_raster;
    //! the inverse size of the raster
    double m_rasterinv;
    //! the limits of the raster area
    int m_rasterminx, m_rastermaxx, m_rasterminy, m_rastermaxy;
    //! an extra border in the raster area around the bounding box
    double m_rasterborder;

    //! occupation counts for vertical edges ( a 2d array )
    unsigned short* m_verticaloccupation;
    //! occupation counts for horizontal edges ( a 2d array )
    unsigned short* m_horizontaloccupation;
    //! Routing points
    RoutePoint* m_routepoints;

    //! File for debug dumps
#ifdef DUMP_FINAL
    FILE* m_dump;
#endif

    //! for wxStaticCast
    DECLARE_DYNAMIC_CLASS( a2dRouteData )

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; };

#if wxART2D_USE_CVGIO

    virtual void DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) )
    {
        wxASSERT( 0 );
    }
    virtual void DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) )
    {
        wxASSERT( 0 );
    }

#endif //wxART2D_USE_CVGIO
};

#endif
