/*! \file wx/canvas/drawer.h
    \brief the a2dDrawingPart is a a2dView specially designed for displaying
    parts of a a2dDrawing. It uses a a2dDrawer2D to actually redraw things from the document,
    by giving that a2dDrawer2D as drawing context to the document, and telling the document to redraw
    a certain rectangular area. At that last is what this class is for.
    It optimizes the areas to be redrawn after object in the document were changed.
    To do that it combines redraw areas to a minimal set of redrawing areas.
    All the administration for this and the way things will be redrawn is from this view.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: drawer.h,v 1.49 2008/10/21 21:48:30 titato Exp $
*/

#ifndef __WXDRAWER_H__
#define __WXDRAWER_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/dnd.h"
#include "wx/dataobj.h"
#include "wx/clipbrd.h"
#include "wx/metafile.h"
#include <wx/wfstream.h>
#include <wx/stdstream.h>
#include "wx/image.h"
#include "wx/geometry.h"

#include "wx/artbase/drawer2d.h"
#include "wx/artbase/dcdrawer.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/tools.h"
#include "wx/canvas/edit.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/algos.h"

#include <vector>

class a2dCursorStack : public std::vector<wxCursor>
{
public:

    a2dCursorStack();
};

class a2dPushInStack : public std::vector< a2dCanvasObjectPtr >
{
public:

    a2dPushInStack();
};


//! record in update list of a2dDrawingPart.
/*!
    \ingroup drawer
*/
class A2DCANVASDLLEXP a2dUpdateArea: public wxRect
{
#ifdef CLASS_MEM_MANAGEMENT
    //! memory manager for speed up to replace system calls allocation and deallocation
    static a2dMemManager sm_memManager;
public:
    //! overloaded operator new for this class and it all derived classes
    void* operator new( size_t bytes )
    {
        return sm_memManager.Allocate( bytes );
    }

    //! overloaded operator delete for this class and it all derived classes
    /*!
        This function doesn't free to OS-system memory block by pointer 'space'.
        It adds memory block by pointer 'space' to internal lists.
        It is speed up.
    */
    void operator delete( void* space, size_t bytes )
    {
        sm_memManager.Deallocate( space, bytes );
    }
#endif //CLASS_MEM_MANAGEMENT

public:
    a2dUpdateArea( int x, int y, int width, int height, wxUint8 id = 0 );
    ~a2dUpdateArea();

public:

    // update done blit awaiting
    bool m_update_done;
    bool m_update_direct;

    //! buffer id
    wxUint8 m_id;
};

#include <wx/listimpl.cpp>

//! Holds all updateareas within a a2dDrawingPart
WX_DECLARE_LIST_WITH_DECL( a2dUpdateArea, a2dUpdateListBase, class A2DCANVASDLLEXP );

//! maintains a list of areas on a view to be redrawn.
/*!
    update areas reported directly to a view or extracted from tiles or stored here.
*/
class A2DCANVASDLLEXP a2dUpdateList : public a2dUpdateListBase
{

#ifdef CLASS_MEM_MANAGEMENT
    //! memory manager for speed up to replace system calls allocation and deallocation
    static a2dMemManager sm_memManager;
public:
    //! overloaded operator new for this class and it all derived classes
    void* operator new( size_t bytes )
    {
        return sm_memManager.Allocate( bytes );
    }

    //! overloaded operator delete for this class and it all derived classes
    /*!
    This function doesn't free to OS-system memory block by pointer 'space'.
    It adds memory block by pointer 'space' to internal lists.
    It is speed up.
    */
    void operator delete( void* space, size_t bytes )
    {
        sm_memManager.Deallocate( space, bytes );
    }
#endif //CLASS_MEM_MANAGEMENT

public:
    a2dUpdateList( void ) : a2dUpdateListBase()
    {
    }

};

#define a2d_TILESHIFT 8
#define a2d_TILESIZE 256 //(1 << a2d_TILESHIFT) 

//! a2dTileBox is a subarea of a tile.
/*!
    The drawing area is divided into tiles, where each tile is 256 * 256 pixels.
    The part of the tile which needs a redraw is stored in tile coordinates.
*/
class A2DCANVASDLLEXP a2dTileBox
{
public:

    //! constructor
    a2dTileBox( int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0 );

    //! destructor
    ~a2dTileBox();

    //! Initialize a tile
    void Init( int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0 );

    //! expand a tile with this box
    void Expand( int x1, int y1, int x2, int y2 );

    //! get x1 in pixel coordinates
    inline int x1p() { return m_x1 << a2d_TILESHIFT; }
    //! get y1 in pixel coordinates
    inline int y1p() { return m_y1 << a2d_TILESHIFT; }
    //! get x2 in pixel coordinates
    inline int x2p() { return m_x2 << a2d_TILESHIFT; }
    //! get y2 in pixel coordinates
    inline int y2p() { return m_y2 << a2d_TILESHIFT; }

    //! if set, true
    bool m_valid;

    //! x1 in pixel coordinates
    int m_x1;
    //! y1 in pixel coordinates
    int m_y1;
    //! x2 in pixel coordinates
    int m_x2;
    //! y2 in pixel coordinates
    int m_y2;

    //! prev row pointer to combine tiles into rectangles
    a2dUpdateArea* m_rectPrevRow;

};

#define a2d_INIT_TILES 100

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DCANVASDLLEXP a2dArrayGrow < a2dTileBox >;
#endif

//! a storage for a a tiled area
/*!
    An array of tiles width * height is maintained here.
    The tiles can be filled using scaning functions which take basic primitives as input.
    Later on the covered tiles can be extracted as a list of rectangles which
    cover the same area but with less rectangles in general.
*/
class A2DCANVASDLLEXP a2dTiles
{
public:

    //! constructor
    /*!
        \param width number of horizontal tiles
        \param height number of vertical tiles
    */
    a2dTiles( int width, int height, wxUint8 m_id );

    //! destructor
    ~a2dTiles();

    //! change tile area
    void SetSize( int width, int height );

    //! All tiles become empty
    void Clear();

    //! draw tiles to given view in device coordinates.
    void DrawTiles( a2dDrawer2D* drawer );

    //! draw optimized rectangles to given view in device coordinates.
    void DrawRects( a2dDrawer2D* drawer );

    //! convert internal tile to pixel coordinate tiles area
    inline int toP( int tilexy ) { return tilexy << a2d_TILESHIFT; }

    //! convert pixel to  internal tile coordinate tiles area
    inline int toT( int xy ) { return xy >> a2d_TILESHIFT; }

    //!  xy modules 256
    inline int ModT( int xy ) { return xy & ( a2d_TILESIZE - 1 ); }

    //! fill tiles covering the rect given see FillTiles(  int x, int y, int w, int h, bool expand )
    void FillTiles( const wxRect& rect, bool expand = true );

    //! fill tiles covering the rect given
    /*!
        The rect formed by x,y,w,h, is divided over the tiles, filling the the part of the tiles that
        are covered by the rectangle.

        \param x x of rectangle
        \param y y of rectangle
        \param w width of rectangle
        \param h height of rectangle
        \param expand if true keep existing filled tiles, else clear all.
    */
    void FillTiles( int x, int y, int w, int h, bool expand );

    //! are there filled tiles available?
    bool HasFilledTiles();

    //! generate from files tiles a list of semi optimal covering rectangles
    /*!
        The tiles are iterated in horizontal rows, combining filed tiles found
        in the rows when at same height and connecting with the previous tile.
        The found rect is then combined a rectangle already found in a previous row.
    */
    a2dUpdateList* GenerateUpdateRectangles();

    //! see GenerateUpdateRectangles(), this one adds to the list given.
    void GenerateUpdateRectangles( a2dUpdateList* rects );

    //! number of horizontal tiles
    int m_width;
    //! number of vertical tiles
    int m_height;

    //! get tile at index i
    inline a2dTileBox tile( int i )
    {
        assert( i < ( m_width * m_height - 1 ) );
        return m_tiles[ i ];
    }

    //! array of tiles ( normally m_width * m_height )
    vector< a2dTileBox > m_tiles;

    //! id for update areas generated from tiles, which maye depend on layer of the object etc.
    wxUint8 m_id;

private:
    //!how many references to this object do exist
    int m_refcount;

    //! Call to have a new owner for this object
    /*! This function should only be called by a2dSmrtPtr

        \remark owning mean that the object calling this member needs to call Release at some time,
                in order to actually release/delete the object.

        \return The return value is the object itself, which is now owned on time extra.

        increment refcount by 1 ( use when adding a reference to this object)
    */
    a2dTiles* SmrtPtrOwn() { m_refcount++; return this; }

    //!To release the object, it is not longer owned by the calling object.
    /*! This function should only be called by a2dSmrtPtr
    */
    bool SmrtPtrRelease()
    {
        m_refcount--;
        wxASSERT_MSG( m_refcount >= 0, wxT( "a2dTiles Own/Release not matched (extra Release calls)" ) );
        if ( m_refcount <= 0 )
        {
            delete this;
            return true;
        }
        return false;
    }

private:
    friend class a2dSmrtPtrBase;
};

class A2DCANVASDLLEXP a2dTilesVec: public std::vector< a2dSmrtPtr<a2dTiles> >
{
public:
    a2dTilesVec();

    ~a2dTilesVec();

    void SetSize( int width, int height );

    void Clear();

    void GenerateUpdateRectangles( a2dUpdateList* rects );

    void FillTiles( int x, int y, int w, int h, bool expand );

    bool HasFilledTiles();
};

class A2DCANVASDLLEXP a2dCanvas;
#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
class A2DCANVASDLLEXP a2dOglCanvas;
#endif

//! Used by a2dDrawingPart to decide what layers are to be rendered.
/*!
    The a2dDrawing is checked via a2dCanvasObject::Addpending() etc. which
    layers contain objects. The array containing this information is stored
    seperate for each a2dDrawingPart. This is because each view can show different parts
    of a document, and one part can contain more layers then the other.
    This information is used to skip rendering layers which do not contain objects,
    or which are set invisible.

    \ingroup canvasobject

    \ingroup docview
*/
class A2DCANVASDLLEXP a2dLayerView
{

public:

    //!constructor
    a2dLayerView()
    {
        m_layervisible = true;
        m_layeravailable = false;
        m_check = true;
        m_canvasObjectCount = 0;
        m_canvasPreviousObjectCount = 0;
    }

    ~a2dLayerView() {};

    //!is the layer visible
    bool GetVisible() { return m_layervisible; }

    //! set layer visible
    void SetVisible( bool status ) { m_layervisible = status; }

    //!are the objects on this layer
    bool GetAvailable() { return m_layeravailable; }

    //! set layer available (will be rendered)
    void SetAvailable( bool status ) { m_layeravailable = status; }

    //! check this layer?
    bool GetCheck() { return m_check; }

    //! set the layer to be checked in idle time.
    void SetCheck( bool status ) { m_check = status; }

    //! should this layer be rendered
    bool DoRenderLayer()
    {
        return ( m_layeravailable && m_layervisible ) || m_check;
    }

    //! get currently counted number of objects on this layer inside a document.
    wxUint32 GetObjectCount() { return m_canvasObjectCount; }

    //! set currently counted number of objects on this layer inside a document.
    void SetObjectCount( wxUint32 numberOfObjects ) { m_canvasObjectCount = numberOfObjects; }

    //! get currently counted number of objects on this layer inside a document.
    wxUint32 GetPreviousObjectCount() { return m_canvasPreviousObjectCount; }

    //! set currently counted number of objects on this layer inside a document.
    void SetPreviousObjectCount( wxUint32 numberOfObjects ) { m_canvasPreviousObjectCount = numberOfObjects; }

    //! increment currently counted number of objects on this layer inside a document.
    void IncrementObjectCount() { m_canvasObjectCount++; }

    //! decrement currently counted number of objects on this layer inside a document.
    void DecrementObjectCount() { m_canvasObjectCount--; }

protected:

    //! Gives the number of object on this layer in a document.
    //! Only accurate after a SetAvailable
    mutable wxUint32 m_canvasObjectCount;

    mutable wxUint32 m_canvasPreviousObjectCount;

    //!is the layer visible
    bool m_layervisible;

    //!is the layer filled with primitives
    bool m_layeravailable;

    //!if true layer need to be check again document for availability.
    bool m_check;

private:
    //!how many references to this object do exist
    int m_refcount;

    //! Call to have a new owner for this object
    /*! This function should only be called by a2dSmrtPtr

        \remark owning mean that the object calling this member needs to call Release at some time,
                in order to actually release/delete the object.

        \return The return value is the object itself, which is now owned on time extra.

        increment refcount by 1 ( use when adding a reference to this object)
    */
    a2dLayerView* SmrtPtrOwn() { m_refcount++; return this; }

    //!To release the object, it is not longer owned by the calling object.
    /*! This function should only be called by a2dSmrtPtr
    */
    bool SmrtPtrRelease()
    {
        m_refcount--;
        wxASSERT_MSG( m_refcount >= 0, wxT( "a2dLayerView Own/Release not matched (extra Release calls)" ) );
        if ( m_refcount <= 0 )
        {
            delete this;
            return true;
        }
        return false;
    }

private:
    friend class a2dSmrtPtrBase;

};

typedef std::map< wxUint16, a2dLayerView > a2dLayerViewList;

//! Display Part of a a2dDrawing, in which a2dCanvasObjects are shown.
/*!
    a2dDrawingPart is a specialized view to display parts of a a2dDrawing.
    Such a part always starts at one a2dCanvasObject which is inside of a a2dDrawing object.
    The a2dCanvasObject itself contains as children a2dCanvasObject derived drawable objects. A hierarchy
    of recursively nexted objects is what forms the actual drawing.
    The parent object to start the drawing is called the ShowObject.
    The member functions SetShowObject(...) are used to set the ShowObject to be displayed.
    All objects are defined in relative world coordinates, which are relative to the parent object(s).

    a2dDrawing is given as a drawing and/or updating context an a2dDrawingPart.  E.g. in
    a2dDrawingPart::RenderTopObject() is used by the a2dCanvasObject render functions to get to
    a2dDrawingPart::m_drawer2D, which is the Drawing Context used to do the actual drawing within
    the a2dCanvasObjects. But a2dDrawingPart::RenderTopObject() defines what drawing style is used to draw.

    The real purpose of a2dDrawingPart, is to maintain a list of damaged/changed areas in the view/drawing, and
    when time is ready, start redrawing those areas. When a a2dCanvasObject did change in position or size etc.,
    the a2dDrawing reports this change as a rectangular redraw areas to the a2dDrawingPart's of that drawing.
    The update areas are based on the boundingbox of the object in its old state and in its new state.
    This reporting is done in idle time, and for all changed objects at once. When reporting of all changed areas is done,
    each a2dDrawingPart knows what parts of its drawing need to be redrawn. It will then start redrawing those areas,
    but only after optimizing to the minimum areas to redraw. So overlapping areas will only be redrawn once.
    The mechanism for that is called tilling.

    The size of the drawing in world coordinates and the size of the view in pixels is all indirectly defined by
    a2dDrawingPart::m_drawer2D and a2dView::m_display. The area of the drawing in world coordinates that is visible,
    can be set via a2dDrawingPart::m_drawer2D. Also if World coordinates is with the Y axis going up are down.
    a2dDrawer2D has methods to convert from world to device coordinates and visa versa.
    a2dDrawingPart::m_drawer2D draws into a buffer. After rendering an update of all damaged parts into this buffer,
    it will be blitted from the buffer to the a2dView::m_display of the a2dDrawingPart. This is done in Idle time,
    but can also be forced. In the end a2dDrawingPart automatically always displays an up to date part of the drawing
    which is stored inside the a2dDrawing. The user just changes a a2dCanvasObject inside the a2dDrawing,
    and the redrawing on all the views will be done automatically.

    A a2dDrawingPart its a2dDrawingPart::m_drawer2D knows where to draw to, this can be a bitmap buffer or a window etc.
    The job to update a window in case of drawing to a bitmap buffer, is not part of the a2dDrawer2D.
    This is/needs to be done by the class using the drawer, like a2dDrawingPart and indirectly a2dCanvas here.
    The a2dCanvas receives a paint event, e.g. when moving an overlapping window or dialog, and it then blits the right
    parts from a2dDrawingPart::m_drawer2D its drawing buffer to the canvas window.
    a2dDrawingPart also takes care of scrolling the view, it does this by re-using the contents of the a2dDrawer2D drawing
    buffer when possible.
    The drawing buffer can be bigger then the size of the canvas window. a2dDrawingPart always makes sure the whole buffer
    contains an up to date contents, as being the drawing to display form its a2dDrawing. Therefore if the canvas windows
    which uses the a2dDrawingPart for drawing, implements scrolling and resizing, the canvas window is responsible for
    optimizing the scroll to take maximum advantage from the buffer of a2dDrawingPart. Like if the buffer is bigger in
    size then the canvas windows size, a resize of the canvas window only needs to increase the buffer size of
    a2dDrawingPart, when the size of the window exeeds the buffer size.
    One can even decide to set the buffer to the size of the complete virtual area that can be scrolled.
    This way one can make a trade of between buffer size and scrolling/resizing speed.
    Knowing this, one needs to realize that mouse coordinates as received inside a canvas window, are different
    if the origin in the drawing context is not at the orginin of the canvas window.
    The a2dDrawingPart::m_drawer2D takes (0,0) of the buffer as the origin of device coodinates.
*/
class A2DCANVASDLLEXP a2dDrawingPart: public a2dObject
{
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    a2dInitCurrentSmartPointerOwner m_initCurrentSmartPointerOwner;
#endif
    DECLARE_EVENT_TABLE()

public:

#if defined(_DEBUG) 
    //! shows where updates did take place.
    void SetUpdateVisible( bool updatesVisible ) { m_updatesVisible = updatesVisible; }

    bool m_updatesVisible;
#endif

    //!constructor
    /*!
    */
    a2dDrawingPart( int width = 1000, int height = 1000 );

    //!constructor
    /*!
        Width and hight is given, mapping is as in drawer given.
    */
    a2dDrawingPart( int width, int height, a2dDrawer2D* drawer2D );

    //!constructor
    /*!
        Width and hight and mapping is as in drawer given.
    */
    a2dDrawingPart( a2dDrawer2D* drawer );

    //!constructor
    /*!
    */
    a2dDrawingPart( const wxSize& size );

    //!copy constructor
    a2dDrawingPart( const a2dDrawingPart& other );

    //! next to the base its m_display, this also sets m_drawer2d to this display
    virtual void SetDisplayWindow( wxWindow* display );

    virtual wxWindow* GetDisplayWindow() { return m_drawingDisplay; }

    //! Get the Display window of the a2dView. But casted to a a2dCanvas
    /*! when a a2dCanvas is used in this a2dDrawingPart return it else 0.
        a2dDrawingPart needs to know the window (a2dCanvas) to display itself.
        The a2dDrawingPart updates to this window in idel time.
        Paint event to the a2dCanvas are using the a2dDrawingPart its buffer for
        quick updating the a2dCanvas window.

        \sa a2dView::SetDisplayWindow()
    */
    a2dCanvas* GetCanvas() const;

#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
    a2dOglCanvas* GetOglCanvas() const;
#endif //wxUSE_GLCANVAS 

#if wxUSE_PRINTING_ARCHITECTURE
    //! to create a a2dDrawingPrintOut, used to print a view or its document
    /*!
        The a2dDrawingPrintOut created will take as much possible from the this drawing part.
        a2dDrawingPrintOut will create itself a view which fits the size of the paper, but important setting are taken from this view.
    */
    virtual wxPrintout* OnCreatePrintout( a2dPrintWhat typeOfPrint, const wxPageSetupDialogData& pageSetupData );
#endif

    //! zoomout leafs a border of this amount of pixels around the drawing
    void SetZoomOutBorder( wxUint16 border ) { m_border = border; }

    //! \see SetZoomOutBorder()
    wxUint16 GetZoomOutBorder() { return m_border; }

    //! sets buffersize ( if used ) for the a2dDrawer2D
    virtual void SetBufferSize( int w, int h );

    virtual bool HasAlhpa() { return m_drawer2D->HasAlpha(); }

    //!destructor
    virtual ~a2dDrawingPart();

    //! if set true, drawing can contain objects that depend on this view (size etc).
    /*! If set those objects will be automatically updated for this view.
        Else the drawing is assumed to not contain viewdependent objects for this view, and that update cycle is skipped.
    */
    void SetViewDependentObjects( bool viewDependentObjects ) { m_viewDependentObjects = viewDependentObjects; }

    //! see SetViewDependentObjects()
    bool GetViewDependentObjects() const { return m_viewDependentObjects; }

    //! get the layer render array
    a2dLayerViewList& GetLayerRenderArray() { return m_layerRenderArray; }

    //!update layers available needed?
    /*!
        \sa Set_UpdateAvailableLayers
    */
    bool Get_UpdateAvailableLayers() const { return m_update_available_layers; }

    //! check which layers do contain objects as seen from the ShowObject()
    void SetAvailable();

    //! signals the need to check the given layer for visibility/availibility as seen from this drawing part.
    void SetLayerCheck( wxUint16 layer );

    //! Sets a flag for updating available layers checking, which makes sure layers will be checked first when needed.
    /*!
        Eventually results in updating the layers table settings on layers available in drawing
        before rendering starts or simular functions.
    */
    void Set_UpdateAvailableLayers( bool value ) { m_update_available_layers = value; }

    //!Set to draw layers in reverse order
    void SetReverseOrder( bool revorder );

    //!Get Setting for draw layers in reverse order
    bool GetReverseOrder() const { return m_reverse_order; }

    //! used to extend a hittest with the number of pixels.
    /*! to be able to hit a line of width zero, a margin is needed to hit it,
        which is set here.

        \remark default value is 2 pixels
    */
    inline void SetHitMarginDevice( wxUint16 pixels ) { m_hitmargin = pixels; }

    //! Get HitMargin used to extend a hittest with the number of pixels.
    inline wxUint16 GetHitMarginDevice() const { return m_hitmargin; }

    //! Get HitMargin used to extend a hittest in world units.
    double GetHitMarginWorld() const;

    //! Special event handling for a2dDrawingPart class
    /*!
        If eventprocessing is enabled and the view is enabled. OR in case this event ( wxEVT_ENABLE_VIEW | wxEVT_ENABLE_VIEWS )
        is to enable this a2dView, the event is processed.

        wxEVT_IDLE is first processed by the view, and next by the m_toolcontroller is set.
        wxEVT_PAINT is first processed by the m_display if set, next by the view, and next by the m_toolcontroller is set.
        This garantees proper redrawing of tools on top of the window.

      Next to this check the command processor for its current parent object, and sent
        a command to set it right if change is needed.
    */
    virtual bool ProcessEvent( wxEvent& event );

    //! If not set do not process mouse events.
    /*!
        Mouse events are handled by the canvas.
        They are redirected to the a2dCanvasObject hit.
        You can switch this off here, and skip the event from being processed.
        This is often used in tools, to prevent a2dCanvasObjects from receiving events.

        \remark most events go to the m_toolcontroller first if set. \see ProcessEvent()
    */
    void SetMouseEvents( bool onoff );

    //! return true if this a2dDrawingPart allows mouse events to be processed.
    bool GetMouseEvents() const { return m_mouseevents; }

    //! use in combination with the a2dIterC class to set a corridor path for events.
    /*!
        A corridor path leads event to a specific a2dCanvasObject in a a2dDrawing.
        This feature is used to do editing of nested a2dCanvasObject 's, and to captures events
        to such objects.
        The a2dCanvasObject's on a corridor path get the m_flags.m_isOnCorridorPath set, which result
        in redirecting events to the m_endCorridorObject.
    */
    void SetCorridor( a2dCanvasObject* start,  a2dCanvasObject* end );

    //! return the corridor object if set else NULL \see SetEndCorridorObject()
    a2dCanvasObject* GetEndCorridorObject() const { return m_endCorridorObject; }

    //! return the first corridor object if set else NULL \see SetCorridor()
    inline a2dCanvasObject* GetStartCorridorObject() const { return m_startCorridorObject; }

    //!set object available in the a2dDrawing to be shown on the drawer
    /*!
        \param name name of top object
        \return pointer to the object found else NULL
    */
    a2dCanvasObject* SetShowObject( const wxString& name );

    //!set top object available in the a2dDrawing to be shown on the drawer
    /*!
    \param obj: pointer to object to show
    */
    bool SetShowObject( a2dCanvasObject* obj );

    //! Used temporarely in rendering bitmap or in printing, to Set the show object and redraw the whole part.
    /*!
        Notification in the form of signals and/or events is not done.
        Also connection to event in other objects is not done.
    */
    void SetShowObjectAndRender( a2dCanvasObject* obj );
        
    //!return pointer of then currently shown object on the drawer.
    /*!
        \return: pointer to the current object that is shown.
    */
    a2dCanvasObject* GetShowObject() const
    {
        return m_top;
    }

    //! set given canvasobject as show object, and store the current on the stack
    void PushIn( a2dCanvasObject* pushin );

    //! pop last pushed canvasobject from the stack, now last becomes show object.
    a2dCanvasObject* PopOut();

    //! mak push in stack empty
    void ClearPushInStack();

    //! is there structure to popout (see PushIn )
    bool HasPopOut() const { return m_pushInStack.size() != 0; }

    //! get drawing via top object
    a2dDrawing* GetDrawing() const;

    //!add pending update for the area that is the boundingbox of the given object
    /*!
        Updates the boundingbox area of the given object at idle time.
        If obj is NULL nothing will be done
        If refalso is true then all references to this object will be updated also.

        \param obj the object where to take the bounding box from
        \param refsalso <code>true</code> to update all references, else <code>false</code>
    */
    void AddPendingUpdateArea( a2dCanvasObject* obj, bool refsalso = true );

    virtual void AddPendingUpdateArea( a2dCanvasObject* obj, const wxRect& recnew );

    //!recursive find pending objects and adds their areas to the updatelist
    /*! it does first test if the a2dDrawing has the flag set to tell it has pending objects inside.
    If that is the case, it traverses the a2dDrawing, and adds
    the absolute boundingbox of the pending object to the a2dDrawingPart updatelist.
    This is for all paths leading to the object from the current ShowObject, so also a2dCanvasObjectReference's.
    \return true if did add pendingobject's else false
    */
    bool AddObjectPendingUpdates( a2dCanViewUpdateFlagsMask how = a2dCANVIEW_UPDATE_PENDING );

    //!add boundingbox to update list for updating in idle time
    void AddPendingUpdateArea( a2dCanvasObject* obj, const a2dBoundingBox& box );

    //!add rectangle to update list for updating in idle time
    void AddPendingUpdateArea( const wxRect& recnew );

    //!add area to update list for updating in idle time
    /*!
    the given area will be added to the
    list of rectangles to be blitted to the screen later in idle time,
    at repaint or after an UpdateNow action.
    The given area will be combined (merged/ignored) with the already available update areas.
    This to prevent un necessary redraws.
    */
    virtual void AddPendingUpdateArea( int x, int y, int w, int h, wxUint8 id = 0 );

    //! see OnUpdate
    /*!by default adds a full pending update for the drawer and all in it, and redraws this to the device.
    Use how = a2dCANVIEW_UPDATE_ALL when the contents/data of the canvas has changed
    without specific updates applied to areas. This will do the redraw in idle time.
    For example: after adding many new objects to the root object
    or other objects referenced or after changing the root object

    \remark the default value is brute force redraw all, but in general not wise to use since it means a total redraw.
    instead most often a2dCANVIEW_UPDATE_OLDNEW is used, or if result needs to be displayed directly, use
    ( a2dCANVIEW_UPDATE_OLDNEW |a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT )

    */
    void Update( unsigned int how = ( a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_BLIT )
                                    , wxObject* hintObject = NULL );

    //!update/redraw part of the buffer, using the given a2dDrawing and ShowObject within that root.
    /*!
        This method also takes care of redrawing the background and the grid
    */
    virtual void UpdateArea( int x, int y, int width, int height, wxUint8 id = 0 );

    virtual void ClearArea( int x, int y, int width, int height );

    //! blit part of the drawing buffer to the canvas
    virtual void BlitBuffer( int x, int y, int width, int height, int xbuf, int ybuf );

    //!Function to draw the origin
    /*!override this function in a derived class to redefine painting of the origin*/
    virtual void DrawOrigin();

    //!Function to draw the grid
    /*!override this function in a derived class to redefine painting of the grid*/
    virtual void PaintGrid( int x, int y, int width, int height );

    //!(re)painting of background
    /*!override this function in a derived class to redefine painting of the background*/
    virtual void PaintBackground( int x, int y, int width, int height );

    //! set enable crosshair cursor
    void SetCrossHair( bool onoff );

    //! get enable crosshair cursor
    bool GetCrossHair() { return m_crosshair; }

    //! set crosshair cursor Length in X in pixels
    void SetCrossHairLengthX( int LengthX ) { m_crosshairLengthX = LengthX; }

    //! set crosshair cursor Length in Y in pixels
    void SetCrossHairLengthY( int LengthY ) { m_crosshairLengthY = LengthY; }

    //! get crosshair cursor Length in X in pixels
    int GetCrossHairLengthX() { return m_crosshairLengthX; }

    //! get crosshair cursor Length in Y in pixels
    int GetCrossHairLengthY() { return m_crosshairLengthY; }

    //!set stroke for crosshair
    void SetCrossHairStroke( const a2dStroke& stroke );

    //!get stroke for crosshair
    a2dStroke& GetCrossHairStroke() { return m_crosshairStroke; }

    //! blit old areas to remove last drawn crosshair and draw the cross hair at this new position.
    virtual void UpdateCrossHair( int x, int y );

    //! set a2dFill to use when RenderFIX_STYLE is set.
    void SetFixedStyleFill( const a2dFill& fixFill ) { m_fixFill = fixFill; }

    //! set a2dStroke to use when RenderFIX_STYLE is set.
    void SetFixedStyleStroke( const a2dStroke& fixStroke ) { m_fixStroke = fixStroke; }

    //! set a2dFill to use when RenderWIREFRAME_SELECT or RenderWIREFRAME_SELECT_INVERT is set.
    void SetSelectFill( const a2dFill& selectFill ) { m_selectFill = selectFill; }

    //! set a2dStroke to use when RenderWIREFRAME_SELECT or RenderWIREFRAME_SELECT_INVERT is set.
    void SetSelectStroke( const a2dStroke& selectStroke ) { m_selectStroke = selectStroke; }

    //! set a2dFill to use when RenderWIREFRAME_SELECT2 or RenderWIREFRAME_SELECT2_INVERT is set.
    void SetSelect2Fill( const a2dFill& select2Fill ) { m_select2Fill = select2Fill; }

    //! set a2dStroke to use when RenderWIREFRAME_SELECT2 or RenderWIREFRAME_SELECT2_INVERT is set.
    void SetSelect2Stroke( const a2dStroke& select2Stroke ) { m_select2Stroke = select2Stroke; }

    //! set a2dFill to use when RenderWIREFRAME_HighLight.
    void SetHighLight( const a2dFill& hightLightFill ) { m_highLightFill = hightLightFill; }

    //! set a2dStroke to use when RenderWIREFRAME_HighLight.
    void SetHighLight( const a2dStroke& hightLightStroke ) { m_highLightStroke = hightLightStroke; }

    //!background fill for the canvas
    void SetBackgroundFill( const a2dFill& backgroundfill );

    //!get current background fill for the canvas
    a2dFill& GetBackgroundFill() { return m_backgroundfill; }

    //!Set grid setting for drawing grid in front or back
    void SetGridAtFront( bool gridatfront ) { m_gridatfront = gridatfront; Update( a2dCANVIEW_UPDATE_ALL ); }

    //!Get grid setting for drawing grid in front or back
    bool GetGridAtFront() { return m_gridatfront;}

    //!set stroke used for grid drawing
    void SetGridStroke( const a2dStroke& gridstroke );

    //!set size of grid circle
    void SetGridSize( wxUint16 gridsize ) { m_gridsize = gridsize; Update( a2dCANVIEW_UPDATE_ALL );}

    //!set fill used for grid drawing
    void SetGridFill( const a2dFill& gridfill );

    //!Get grid distance in X
    double GetGridX() {return m_gridx;}

    //!Set grid distance in X
    void SetGridX( double gridx ) { m_gridx = gridx; Update( a2dCANVIEW_UPDATE_ALL );}

    //!Get grid distance in Y
    double GetGridY() {return m_gridy;}

    //!Set grid distance in Y
    void SetGridY( double gridy ) { m_gridy = gridy; Update( a2dCANVIEW_UPDATE_ALL );}

    //!Set grid on/off
    void SetGrid( bool grid ) { m_grid = grid; Update( a2dCANVIEW_UPDATE_ALL ); }

    //!Get grid setting on/off
    bool GetGrid() {return m_grid;}

    //!Get grid setting for line drawing
    /*!Note: SetGridSize must be 0, if gridlines=false */
    void SetGridLines( bool gridlines ) { m_gridlines = gridlines; Update( a2dCANVIEW_UPDATE_ALL ); }

    //!Get setting for grid to draw lines instead of points
    bool GetGridLines() { return m_gridlines; }

    //!Set grid threshold, if grid distance is below this in pixels, it will be increased
    //! by factors of 2 until it fits.
    void SetGridThreshold( wxUint16 gridthres ) { m_gridthres = gridthres; Update( a2dCANVIEW_UPDATE_ALL ); }

    //!Get grid threshold.
    wxUint16 GetGridThreshold() { return m_gridthres; }

    //!Get m_ignoreFocus setting on/off
    bool GetIgnoreFocus() {return m_ignoreFocus;}

    //!Set m_ignoreFocus on/off
    void SetIgnoreFocus( bool ignoreFocus ) { m_ignoreFocus = ignoreFocus; }

    //!Set showorigin on/off
    void SetShowOrigin( bool show ) { m_showorigin = show; Update( a2dCANVIEW_UPDATE_ALL ); }

    //!Returns if canvas is frozen.
    bool IsFrozen() const { return m_freezeCount != 0; }

    //!prevent changing the a2dDrawingPart buffer and blitting it to the window
    /*!
        This makes sure the contents displayed into the a2dDrawingPart buffer does not change.
        Pending objects inside a root will be added to the update list of the a2dDrawingPart,
        but not redrawn into the buffer until Thaw.

        Calls to Freeze() and Thaw() may be nested but to ensure that the view is properly
        repainted again, you must thaw it exactly as many times as you froze it.

        \see Thaw
    */
    void Freeze();

    //! to release Freeze()
    /*!
        The Frozen view ( Freeze() ), is released. Pending update araes in the update list
        will be redrawn in OnIdle at the next idle event.

        To really thaw the view, it must be called exactly the same number of times
        as Freeze().

        \see Freeze

        \param update if true all shown on this view will be redrawn.
    */
    void Thaw( bool update );

    //! set toolcontroller ( reset with NULL )
    /*! Toolcontrollers are meant to implement tools that manipulate the objects
    displayed on the a2dCanvas.
    All events to the canvas window or first redirected to the controller.
    When skipped in the controller the events will eventually reach the a2dCanvas itself.

    \return true is there was a controller set already.

    \remark The controller is owned by the canvas and will be deleted by the canvas on destruction or when changed.
    */
    bool SetCanvasToolContr( a2dToolContr* controller );

    a2dToolContr* GetCanvasToolContr() { return m_toolcontroller; }

    void SetCursor( const wxCursor& cursor );

    const wxCursor& GetCursor() const;

    //! push a cursor on the cursor stack, and set display cursor to new back being cursor.
    void PushCursor( const wxCursor& cursor );

    //! pop a cursor from the cursor stack, and set display cursor to back
    void PopCursor();

    //! clear the stack of cursor, and set display cursor ARROW.
    void ClearCursorStack();

    //!do a hittest on the view at coordinates x,y
    /*!
    \param x x of point to do hittest
    \param y y of point to do hittest
    \param layer test only if objects are on this layer or if set to wxLAYER_ALL test objects on all layers
    \param option ways to hit, default iterates on layers

    \return the top object that was hit (e.g.in case of groups)

    \remark hit margin is defined in a2dDrawing containing the root group
    */
    a2dCanvasObject* IsHitWorld(
        double x, double y,
        int layer = wxLAYER_ALL,
        a2dHitOption option = a2dCANOBJHITOPTION_LAYERS
    );

    //!do an advanged hittest on the view
    /*!
    \param hitEvent stores hit information
    \param layer test only if objects are on this layer or if set to wxLAYER_ALL test obejcts on all layers

    \return the top object that was hit (e.g.in case of groups)

    \remark hit margin is defined in a2dDrawing containing the root group
    */
    a2dCanvasObject* IsHitWorld(
        a2dHitEvent& hitEvent,
        int layer = wxLAYER_ALL
    );

    //! Corridor and captured object event processing.
    /*!
        Follow corridor set, and redirect the event to last object in the corridor.
        The event is first sent to the child objects, and if not processed there,
        testing for a hit on the object itself is done, and if true wxEvtHandler::ProcessEvent is called.

        \param event the event to process
        \param isHit return if there was a hit on an object
        \param x x of point to do hittest
        \param y y of point to do hittest
        \param margin margin that still gives a valid hittest.
        \param layer only if object is on this layer or if set to wxLAYER_ALL ignore layer id.

        \return true if Object (or a child ) did process the event and did not call event.Skip()
    */
    virtual bool ProcessCanvasObjectEvent( wxEvent& event, bool& isHit,
                                           double x, double y, int margin,
                                           int layer = wxLAYER_ALL );

/*
    //! Corridor and captured object event processing.
    /! This is the same as the function
        ProcessCanvasObjectEvent( wxEvent& event, bool& isHit, double x, double y, int margin, int layer = wxLAYER_ALL ),
        but it sets the iteration context pointer of the event.
        This is used when sending a2dCanvasObjectEvents from a simple (handle less) tool.

        \param event the event to process
        \param isHit return if there was a hit on an object
        \param x x of point to do hittest
        \param y y of point to do hittest
        \param margin margin that still gives a valid hittest.
        \param layer only if object is on this layer or if set to wxLAYER_ALL ignore layer id.

        \return true if Object (or a child ) did process the event and did not call event.Skip()
    /
    virtual bool ProcessCanvasObjectEvent( a2dCanvasObjectEvent& event, bool& isHit,
                                           double x, double y, int margin,
                                           int layer = wxLAYER_ALL );
*/

    //! This function is called after a property changed
    /*! This is overloaded to update the view if needed.
    */
    void OnPropertyChanged( const a2dPropertyId* id );

    //! Find the show-object child object, set the path to the given (nested) child object
    /*! \see a2dIterC::SetCorridorPath
        This is used to redirect events to a specific child object, e.g. one found by
        extended hit testing.

        \param findObject (child) object to be searched for.
        \return true if findObject was found
    */
    bool FindAndSetCorridorPath( a2dCanvasObject* findObject );

    //! find object on the current corridor path.
    /*!
        The a2dCanvasObject in the document with the m_flags.m_isOnCorridorPath set, are
        pushed into a list, which is returned. If non found the return is false, else true and the list
        of objects leading to the end of the corridor.
    */
    // bool FindCorridorPath( a2dCorridor& result );

    //! set a corridor from a list of objects
    void SetCorridorPath( const a2dCorridor& corridor );

    //! Reset all corridor paths and uncapture object, if set
    /*! \see a2dIterC::SetCorridorPath
        Reset a corridor path set with FindAndSetCorridorPath
    */
    void ClearCorridorPath();


    //! use the boundingbox of the ShowObject to set the mapping such that it will be displayed completely on the device.
    /*!
        \see SetMappingDeviceRect to map to a different device/buffer rectangle.
    */
    void SetMappingShowAll();

    //! as SetMappingShowAll(), but via the canvas to set scrollbars and such.
    void SetMappingShowAllCanvas();

    //! sets the mapping to display all selected objects of the ShowObject completely on the device.
    /*!
        \see SetMappingShowAll to display all objects of the ShowObject completely on the device.
    */
    void SetMappingShowSelectedObjects( double minWidth = 1, double minHeight = 1 );

    //! set the internal m_drawer2D to be used for rendering the document
    /*! The drawing context is owned by a2dDrawingPart, it will delete it.
        Still you set it to NULL or othere context without deletion using the noDelete parameter.

        \param drawer2d the drawing context object to set
        \param noDelete default the old drawing context object is deleted, but not is this is set true.
                  This can be used to simulate ownership of the drawing context by another class.
    */
    virtual void SetDrawer2D( a2dDrawer2D* drawer2d, bool noDelete = false );

    //! get the internal m_drawer2D that is used for rendering the document
    a2dDrawer2D* GetDrawer2D() { return m_drawer2D; }

    //!set drawstyles to use for drawing the document
    /*!
        The internal m_drawer2D is set to this style before rendering of the document starts.

        \remark m_drawstyleRestore is set to the current drawstyle. So a temporary change can easily be restored.

        \param  drawstyle one of the draw styles
    */
    void SetDocumentDrawStyle( wxUint32 drawstyle );

    //! restore drawstyle to the one before the last change
    void RestoreDrawStyle() {  m_documentDrawStyle = m_documentDrawStyleRestore; }

    //!get drawstyles used for drawing the document
    wxUint32 GetDocumentDrawStyle() { return m_documentDrawStyle; }

    //! scroll up down or left right
    /*! \param dxy scroll distance in X or Y
        \param yscroll if true scrolling is in Y else X
        \param total update whole device after scroll
    */
    virtual void Scroll( int dxy, bool yscroll, bool  total );

    //! used to drop a drawing object on the drawing part during Drag and Drop.
    void OnDrop(wxCoord x, wxCoord y, a2dDrawing* drawing);

    //! does render the top object in the given style.
    /*!
        This function can be used from tools to render the tool its objects which do become
        part of the document in a certain style.
    */
    virtual void RenderTopObject( wxUint32 documentDrawStyle, wxUint8 id );

    //! single drawstyle render cycle called on document
    /*!
        Render the given topobject and all that is below it
        Will recursive call the render routines of all the objects seen from the topobject.
        This rendering iterates over layers.
        \param mask object must have this mask
        \param drawstyle which drawstyle should be used for this render action .

        \remark If a layer is Invisible it will not be rendered.
    */
    virtual void RenderTopObjectOneStyle( a2dCanvasObjectFlagsMask mask, a2dDocumentRenderStyle drawstyle, wxUint8 id );

    //! update the transform matrix for objects with property 'PROPID_viewDependent'
    /*!
        will recursive call the UpdateViewDependentObjects routines of all the objects seen from the topobject.

        \remark TODO: Check all child objects (TODO for optimize: with flag 'm_childpixelsize=true').
    */
    void UpdateViewDependentObjects();

    //! If true render the printout with a title string, otherwise not
    void SetPrintTitle( bool val ) { m_printtitle = val; }

    //! If true render the printout with a filename string, otherwise not
    void SetPrintFilename( bool val ) { m_printfilename = val; }

    //! Set the scaling limit for printing, so that small stuff is not zoomed to full page
    void SetPrintScaleLimit( double val ) { m_printscalelimit = val; }

    //! If true, draw a frame around printouts
    void SetPrintFrame( bool val ) { m_printframe = val; }

    //! If true, draw a view on all page without real scale
    void SetPrintFitToPage( bool val ) { m_printfittopage = val; }

    //! get mouse position X
    int GetMouseX() { return m_mouse_x; }
    //! get mouse position Y
    int GetMouseY() { return m_mouse_y; }

    //! get mouse position X in world coordinates
    int GetWorldMouseX() { return ( int )m_drawer2D->DeviceToWorldX( m_mouse_x ); }
    //! get mouse position Y in world coordinates
    int GetWorldMouseY() { return ( int )m_drawer2D->DeviceToWorldY( m_mouse_y ); }

    //! convert mouse position as seen from the display window, into world coordinates.
    //! If no display window is defined,
    void MouseToToolWorld( int x, int y, double& xWorldLocal, double& yWorldLocal );

    //! get mouse position as seen from the display window, in world coordinates.
    //! If no display window is defined,
    void ToolWorldToMouse( double xWorld, double yWorld, int& x, int& y  );

    //! if tool want an event to be handled by the view, even if already processed.
    void SetToolWantsEventToContinue( bool continueEvent = true ) { m_toolWantsIt = continueEvent; }

    //! Get a maximum pixels per user unit for zooming in, if  0 ignored
    double GetUppXyMax() const { return m_uppXyMax; }

    //! Set a maximum pixels per user unit for zooming in, if  0 ignored
    /*!
    *  This will limit the zoom in to the given vale of  m_uppXyMax
    */
    void SetUppXyMax(double uppXyMax) { m_uppXyMax = uppXyMax; }

    /*
    Setting virtual area to boundingbox of  a drawing(currently visible ShowObject()

        \remark do not use during start up since window size is not well defined in that case resulting in
        \remark bad settings for the mapping.

        \param vx1 minimum world x coordinate
        \param vy1 minimum world y coordiante(either Lower or Upper Left corner depending on SetYaxis())
        \param width minimum width in world coordinates which we want to display on this window
        \param height minimum height in world coordinates which we want to display on this window
        \param scrollbars : The scroll region are set to display this also.
    */
    void SetMappingWidthHeight(double vx1, double vy1, double width, double height );

    //! Like SetMappingUpp() but uses position x,y as the fixed point for zooming, and width and height different from buffersize.
    /*!
        \param vx1 minimum world x coordinate
        \param vy1 minimum world y coordiante(either Lower or Upper Left corner depending on SetYaxis())
        \param xpp amount of world coordinates per pixel in x
        \param ypp amount of world coordinates per pixel in y
        \param x when m_uppXyMax is reached, this will be the center  point x for the zoom limit
        \param y when m_uppXyMax is reached, this will be the center  point y for the zoom limit
    */
    void SetMappingUpp(double vx1, double vy1, double xpp, double ypp, double x = -1, double y = -1 );

protected:

    //void OnActivate( a2dViewEvent& activateEvent );
    //void OnCloseView( a2dCloseViewEvent& event );

    void OnSetFocus( wxFocusEvent& focusevent );

    void OnKillFocus( wxFocusEvent& focusevent );

    void OnEnter( wxMouseEvent& WXUNUSED( event ) );

    //! redraw and/or blit pending areas to the device
    void OnIdle( wxIdleEvent& event );

    //! normally sent from wxWindow containing the view, via its ProcessEvent(wxEvent& event)
    void OnMouseEvent( wxMouseEvent& event );

    //! normally sent from wxWindow containing the view, via its ProcessEvent(wxEvent& event)
    /*!

    */
    void OnCharEvent( wxKeyEvent& event );

    //! called for a2dComEvent events.
    void OnComEvent( a2dComEvent& event );

#ifdef _DEBUG
    //! 
    void OnDoEvent( a2dCommandProcessorEvent& event );

    //! 
    void OnRedoEvent( a2dCommandProcessorEvent& event );

    //! 
    void OnUndoEvent( a2dCommandProcessorEvent& event );
#endif //_DEBUG

    //! do this at command start
    void OnBusyBegin( a2dCommandProcessorEvent& WXUNUSED( event ) );

    //! do this at command end
    void OnBusyEnd( a2dCommandProcessorEvent& WXUNUSED( event ) );

    //! based on angle and radius and m_aberration calculate a proper delta phi and number of segments
    /*!
        The calculation is used for circular arc segments
    */
    void Aberration( double angle, double radius, double& dphi, unsigned int& segments );

    //! buffer updating activity possible or not
    /*!
        number of Freeze() calls minus the number of Thaw() calls: buffer is
        frozen (i.e. not being updated) if it is positive
    */
    unsigned int m_freezeCount;

    //! update all shown on this view when thawing
    bool m_thawUpdateAll;

    //! when a corridor is active, this is set as first object in corridor.
    a2dCanvasObjectPtr m_startCorridorObject;

    //! when a corridor is active, this is set as last object in corridor.
    a2dCanvasObjectPtr m_endCorridorObject;

    //!enable/ disable mouse events handling by canvas
    bool m_mouseevents;

    //!top object for drawer object, from here the rendering starts
    a2dCanvasObjectPtr m_top;

    //!background fill of canvas and background color of background fill in case of mono colour fill
    a2dFill m_backgroundfill;

    //! border zoomout but leaf around a border of this amount of pixels.
    wxUint16 m_border;

    //!showorigin?
    bool m_showorigin;

    //!grid drawn at front or back
    bool m_gridatfront;

    //! is croshair visible
    bool m_crosshair;

    //!crosshair x
    int m_crosshairx;

    //!crosshair y
    int m_crosshairy;

    //! crosshair cursor Length in X in pixels
    int  m_crosshairLengthX;

    //! crosshair cursor Length in Y in pixels
    int  m_crosshairLengthY;

    //! stroke to use for crosshair
    a2dStroke m_crosshairStroke;

    //! last mouse position
    int m_mouse_x, m_mouse_y;

    //!grid stroke
    a2dStroke m_gridstroke;

    //!grid fill
    a2dFill m_gridfill;

    //!fixed style stroke
    a2dStroke m_fixStroke;

    //!fixed style fill
    a2dFill m_fixFill;

    //!overlay style stroke
    a2dStroke m_overlayStroke;

    //!overlay style fill
    a2dFill m_overlayFill;

    //!select style stroke
    a2dStroke m_selectStroke;

    //!select style fill
    a2dFill m_selectFill;

    //!select style stroke
    a2dStroke m_select2Stroke;

    //!select style fill
    a2dFill m_select2Fill;

    //!highLight style stroke
    a2dStroke m_highLightStroke;

    //!highLight style fill
    a2dFill m_highLightFill;

    //!grid distance in x
    double m_gridx;

    //!grid distance in y
    double m_gridy;

    //!grid point size
    wxUint16 m_gridsize;

    //!show grid as lines
    bool m_gridlines;

    //! threshold for grid.
    wxUint16 m_gridthres;

    //!grid on/off
    bool m_grid;

    bool m_ignoreFocus;

    //!is the virtual area set already (used during startup)
    bool m_virtualarea_set;

    //!device size width
    int m_width;

    //!device size height
    int m_height;

    //! accuracy of spline
    double m_splineaberration;

    //! pushed clipping regions
    a2dClipRegionList   m_clipregionlist;

    //!accuracy of arc segment calculation etc. in device coordinates
    double m_displayaberration;

    //!to prevent recursive updates
    bool m_recur;

    //! toolscontroller plugged in as first event handler
    a2dSmrtPtr<a2dToolContr> m_toolcontroller;

    a2dDrawer2D* m_drawer2D;

    wxWindow* m_drawingDisplay; 

    //! how close does a hit need to be to the object you are trying to hit.
    /*! This is given in device units (pixels) and tranformed to world units
        using the drawers transform
    */
    wxUint16 m_hitmargin;

    //! underneath the threshold draw rectangles.
    bool m_asrectangle;

    //! drawstyles to use when rendering document
    wxUint32 m_documentDrawStyle;

    //! to restore style after a temporary change.
    wxUint32 m_documentDrawStyleRestore;

    //! if true, a printout is done with title (document name (description?)), otherwise not
    bool m_printtitle;

    //! if true, a printout is done with filename (document file path), otherwise not
    bool m_printfilename;

    //! Set the scaling limit for printing, so that small stuff is not zoomed to full page
    double m_printscalelimit;

    //! If true, draw a frame around printouts
    bool m_printframe;

    //! If true, draw a view on all page without real scale
    bool m_printfittopage;

    //! which layer should be rendered ( visible and/or available )
    a2dLayerViewList m_layerRenderArray;

    //!flag to updatelayers that are available.
    /*!
        It is set when m_layerRenderArray does not contain all layers used by a2dCanvasObject's
    */
    bool m_update_available_layers;

    //!draw in reverse order if set
    bool m_reverse_order;

    bool m_viewDependentObjects;

    a2dCursorStack m_cursorStack;

    a2dPushInStack m_pushInStack;

    bool m_toolWantsIt;

    //! to set a maximum number of pixels for zooming in.
    double m_uppXyMax;

public:

    DECLARE_DYNAMIC_CLASS( a2dDrawingPart )

private:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }

    //initialize a drawer
    void Init();

public:

    static a2dPropertyIdUint16* PROPID_drawstyle;
    static a2dPropertyIdBool* PROPID_gridlines;
    static a2dPropertyIdBool* PROPID_grid;
    static a2dPropertyIdBool* PROPID_showorigin;
    static a2dPropertyIdUint16* PROPID_hitmargin;
    static a2dPropertyIdBool* PROPID_gridatfront;
    static a2dPropertyIdUint16* PROPID_gridsize;
    static a2dPropertyIdUint16* PROPID_gridthres;
    static a2dPropertyIdDouble* PROPID_gridx;
    static a2dPropertyIdDouble* PROPID_gridy;

    //! when more layers changed ( rerendering view is needed).
    static const a2dSignal sig_changedLayers;
    //! when an object is added to a layer, and therefore makes this layer available.
    static const a2dSignal sig_changedLayerAvailable;
    //! when one layer is set visible in a2dDrawingPart
    static const a2dSignal sig_changedLayerVisibleInView;

    //! id for changed a2dObject
    //! a new show object was chosen in a2dDrawingPart
    static const a2dSignal sig_changedShowObject;
    static const a2dSignal sig_triggerNewView;

    //! swap buffer signal for canvas in use.
    static const a2dSignal sig_swapBuffer;

    DECLARE_PROPERTIES()

};

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dDrawingPart>;
#endif


class A2DCANVASDLLEXP a2dDrawingPartTiled: public a2dDrawingPart
{
    DECLARE_EVENT_TABLE()

public:

    //!constructor
    /*!
    */
    a2dDrawingPartTiled( int width = 1000, int height = 1000 );

    //!constructor
    /*!
        Width and hight is given, mapping is as in drawer given.
    */
    a2dDrawingPartTiled( int width, int height, a2dDrawer2D* drawer2D );

    //!constructor
    /*!
        Width and hight and mapping is as in drawer given.
    */
    a2dDrawingPartTiled( a2dDrawer2D* drawer );

    //!constructor
    /*!
    */
    a2dDrawingPartTiled( const wxSize& size );

    //!copy constructor
    a2dDrawingPartTiled( const a2dDrawingPartTiled& other );

    virtual void SetDrawer2D( a2dDrawer2D* drawer2d, bool noDelete = false );

    //! sets buffersize ( if used ) for the a2dDrawer2D
    virtual void SetBufferSize( int w, int h );

    //!destructor
    virtual ~a2dDrawingPartTiled();

    virtual void AddPendingUpdateArea( int x, int y, int w, int h, wxUint8 id = 0 );

    //! depending on the hint value performs specific updating on the view.
    /*!
    Hint is unsigned int containing flags of type \sa a2dCanViewUpdateFlags which actions will be executed in the right order.
    In principle the view its internal data ( this is mostly the buffer ) should be updated, and be made ready
    for redisplay on the view its DisplayWindow().
    A reason for updating a view, is a change in the a2dDrawing. The document is checked for changes in idle
    time, and updates all views on the document at once when a change exists.
    First all changed areas on the view because of changes to the document, are reported by the document to the a2dDrawingPart.
    The a2dDrawingPart keeps an internal list of areas needing a redraw.
    The areas reported are found on the basis of the old and new boundingboxes of the changed objects.
    Next the areas in the updatelist are redrawn in the buffer of the a2dDrawingPart, and at last those areas will be blitted.

    The above is done automatic, but one is free for whatever reason to add pending areas directly to the
    a2dDrawingPart its update arealist. Those will be updated also in idle time, along with all others areas as a result
    of changing objects.

    \remark OnUpdate is the key routine to speedy redraws, proper use asures only redraw of changed or
    damaged areas in idle time.

    \remark Called in a2dDrawing from UpdateAllViews() and AddPendingUpdatesOldNew() to update this view.

    \remark a2dCANVIEW_UPDATE_BLIT means, blit all updated areas for this a2dCanvas now to the screen,
    else it will happen in idle time.
    Use a2dCANVIEW_UPDATE_OLDNEW |a2dCANVIEW_UPDATE_BLIT | a2dCANVIEW_UPDATE_BLIT
    to support dragging for instance, because in such cases idle time
    will take to long. Other a2dCanvas using the same a2dDrawing will not be redrawn and blitted
    when double buffered.
    */
    void OnUpdate( a2dDrawingEvent& event );

    //! scroll up down or left right
    /*! \param dxy scroll distance in X or Y
        \param yscroll if true scrolling is in Y else X
        \param total update whole device after scroll
    */
    virtual void Scroll( int dxy, bool yscroll, bool  total );

    void OnIdle( wxIdleEvent& event );

protected:

    //!list of rectangles that need to be blited to the screen.
    /*!
        This list holds several rectangles, which should be updated.
        The are blitted from the buffer, since they were damaged they first need to be updated.
    */
    a2dUpdateList m_updateareas;

    //!redraw the pending update areas to the buffer given in device coordinates.
    void RedrawPendingUpdateAreas( bool noblit = false );

    //!pending update areas in the update list are deleted.
    /*! use when ready with updates, or when they become useless because of scroll etc.
    */
    void DeleteAllPendingAreas();

    //! blit pending update areas, that are already updated to the buffer, now to the screen.
    /*! does the second stage in the two stage process of updating
    */
    virtual bool BlitPendingUpdateAreas();

    //! tiles on drawing surface, used to optimize update areas.
    a2dTilesVec m_tileVec;

public:

    DECLARE_DYNAMIC_CLASS( a2dDrawingPartTiled )

private:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }
};

#if wxUSE_PRINTING_ARCHITECTURE
#include "wx/prntbase.h"


// Don't use the Windows printer if we're in wxUniv mode and using
// the PostScript architecture
#if wxUSE_PRINTING_ARCHITECTURE && (!defined(__WXUNIVERSAL__) || !wxUSE_POSTSCRIPT_ARCHITECTURE_IN_MSW)


class a2dPrintFactory: public wxNativePrintFactory
{
public:
    virtual wxPrinterBase *CreatePrinter( wxPrintDialogData *data );

    virtual wxPrintPreviewBase *CreatePrintPreview( wxPrintout *preview,
                                                    wxPrintout *printout = NULL,
                                                    wxPrintDialogData *data = NULL );
    virtual wxPrintPreviewBase *CreatePrintPreview( wxPrintout *preview,
                                                    wxPrintout *printout,
                                                    wxPrintData *data );
};

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

#if defined(__WXMSW__)
#include "wx/msw/printwin.h"

class WXDLLIMPEXP_CORE a2dWindowsPrintPreview : public wxWindowsPrintPreview
{
public:
    a2dWindowsPrintPreview(wxPrintout *printout,
                          wxPrintout *printoutForPrinting = NULL,
                          wxPrintDialogData *data = NULL);
    a2dWindowsPrintPreview(wxPrintout *printout,
                          wxPrintout *printoutForPrinting,
                          wxPrintData *data);
    virtual ~a2dWindowsPrintPreview();


protected:
    virtual bool RenderPageIntoBitmap(wxBitmap& bmp, int pageNum);

    DECLARE_DYNAMIC_CLASS_NO_COPY(a2dWindowsPrintPreview)
};

#endif

#endif


#endif // wxUSE_PRINTING_ARCHITECTURE




#if wxUSE_DRAG_AND_DROP
//! Copy paste data objects for a2dDrawing.
/*!
    To copy paste data to and from a a2dDrawing and the a2dDrawingPart currently active.

    At least a bitmap and text and url can be pasted from another application.
    Within wxArt2d a2dCanvasObject's can be copied and pasted internally, 
    and external this will result in a bitmap or metafile to pasted.
*/
template < class parserTypeIn, class parserTypeOut >
class a2dDnDCanvasObjectData : public wxDataObject
{
public:

    a2dDnDCanvasObjectData( a2dDrawingPart* drawingPart = NULL, wxDataObject::Direction dir = wxDataObject::Set );

    virtual ~a2dDnDCanvasObjectData();

    //! add a canvasopbject to the copy data
    void Append( a2dCanvasObject* canvasobject );

    //! objects appended as clone, will be reconnected if connected.
    void Reconnect();

    a2dDrawing* GetDrawing() const { return m_drawing; }

    a2dCanvasObjectList* GetObjects() const;

	//if URL available return it
	wxString  GetURL () const;  

	//if text available return it
	wxString GetText() const;

    //! if set non zero, the width or height of the imported a2dImage will be set to this.
    void SetImportWidthHeightImage( double importWidthHeightImage ) { m_importWidthHeightImage = importWidthHeightImage; }

    //! if non zero, the width or height of the imported a2dImage will be set to this.
    double GetImportWidthHeightImage() const { return m_importWidthHeightImage; }

    // if set non zero, the width or height of the exported a2dImage or metafile resolution will be set to this.
    void SetExportWidthHeightImage( double exportWidthHeightImage ) { m_exportWidthHeightImage = exportWidthHeightImage; }

    //! if non zero, the width or height of the imported a2dImage will be set to this.
    double GetExportWidthHeightImage() const { return m_exportWidthHeightImage; }

    // implement base class pure virtuals
    // ----------------------------------

    virtual wxDataFormat GetPreferredFormat(Direction dir = Get ) const;

    virtual size_t GetFormatCount(Direction dir = Get ) const;

    virtual void GetAllFormats(wxDataFormat *formats, Direction dir = Get ) const;

    virtual size_t GetDataSize(const wxDataFormat& format) const;

    virtual bool GetDataHere(const wxDataFormat& format, void *pBuf) const;

    virtual bool SetData(const wxDataFormat& format,
                         size_t WXUNUSED(len), const void *buf);
protected:

    wxBitmap RenderImage( a2dCanvasObject* canvasobject ) const;
    void RenderToDc( wxDC* dc, a2dCanvasObject* canvasobject ) const;

    // creates a bitmap and assigns it to m_dobjBitmap (also sets m_hasBitmap)
    void CreateBitmap() const;
#if wxUSE_METAFILE
    void CreateMetaFile() const;
#endif // wxUSE_METAFILE

    bool                m_hasShapes;
    wxDataFormat        m_formatShape;  // our custom format

    mutable a2dDocumentStringOutputStream m_mem;

    wxTextDataObject    m_dobjText;
    bool                m_hasText;

    wxBitmapDataObject  m_dobjBitmap;   // it handles bitmaps
    bool                m_hasBitmap;    // true if m_dobjBitmap has valid bitmap

    wxURLDataObject     m_dobjURL;
    bool                m_hasURL;   

#if wxUSE_METAFILE
    wxMetaFileDataObject m_dobjMetaFile;// handles metafiles
    bool                 m_hasMetaFile; // true if we have valid metafile
#endif // wxUSE_METAFILE

    a2dDrawingPtr       m_drawing;        // to hold cloned copies of selected objects.

    a2dRefMap m_refs;

    a2dSmrtPtr<a2dDrawingPart> m_drawingPart; // drawing part from which the copy was made.      
    a2dDrawingPtr       m_drawingOriginal;    // The drawing belong to m_drawingPart, to hold on to it in clipboard, even if deleted in application.

    double m_dragStartX;
    double m_dragStartY;

    double m_importWidthHeightImage;
    double m_exportWidthHeightImage;
};

typedef a2dDnDCanvasObjectData< a2dIOHandlerCVGIn, a2dIOHandlerCVGOut > a2dDnDCanvasObjectDataCVG;

extern const wxChar *a2dCanvasObjectFormatId;

template < class parserTypeIn, class parserTypeOut >
a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::a2dDnDCanvasObjectData( a2dDrawingPart* drawingPart, wxDataObject::Direction dir )
{
    m_drawingPart = drawingPart;
    if ( m_drawingPart )
        m_drawingOriginal = m_drawingPart->GetDrawing(); //via its m_top.   
    m_dragStartX = 0;
    m_dragStartY = 0;

    m_drawing = new a2dDrawing();
    m_drawing->SetCommandProcessor( NULL );

    // this string should uniquely identify our format, but is otherwise
    // arbitrary
    m_formatShape.SetId(a2dCanvasObjectFormatId);

    // we don't draw the shape to a bitmap until it's really needed (i.e.
    // we're asked to do so)
    m_hasShapes = false;
    m_hasText = false;
    m_hasURL = false;
    m_hasBitmap = false;
#if wxUSE_METAFILE
    m_hasMetaFile = false;
#endif // wxUSE_METAFILE
    m_importWidthHeightImage = 0;
    m_exportWidthHeightImage = 0;

    if ( dir == wxDataObject::Set )
        return;

    a2dCanvasObject::SetIgnoreAllSetpending( true );

    //search selected shapes, and copy them to the local drawing
    if ( m_drawingPart )
    {
        a2dCanvasObjectList* objects = m_drawingPart->GetShowObject()->GetChildObjectList();
        forEachIn( a2dCanvasObjectList, objects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj->GetRelease() || !obj->IsVisible()  || !obj->GetSelected() )
                continue;
            Append( obj );
            m_hasShapes = true;
        }
        Reconnect();
    }
    a2dCanvasObject::SetIgnoreAllSetpending( false );

    // selected text in editing of text has priority above selected objects.
    if ( m_drawingPart && wxDynamicCast( m_drawingPart->GetCanvasToolContr(), a2dStToolContr ) )
    {
        a2dStToolContr* contr = wxDynamicCast( m_drawingPart->GetCanvasToolContr(), a2dStToolContr );
        if ( contr )
        {
            a2dObjectEditTool* edittool = wxDynamicCast( contr->GetFirstTool(), a2dObjectEditTool );
            if ( edittool )
            {
                a2dText* text = wxDynamicCast( edittool->GetEditObjectCopy(), a2dText );
                if ( text )
                {
                    if ( !text->GetSelectedText().IsEmpty() && !m_hasURL )
                    {
                        m_hasURL = true;
                        m_dobjURL.SetURL( text->GetSelectedText() );
                    }
                    if ( !text->GetSelectedText().IsEmpty() && !m_hasText )
                    {
                        m_hasText = true;
                        m_dobjText.SetText( text->GetSelectedText() );
                    }
                }
            }
        }
    }
    // if no text or url was selected in editing text, see if there is a text object selected.
    if ( m_drawingPart && !m_hasURL && !m_hasText )
    {
        a2dCanvasObjectList* objects = m_drawingPart->GetShowObject()->GetChildObjectList();
        forEachIn( a2dCanvasObjectList, objects )
        {
            a2dCanvasObject* obj = *iter;
            a2dText* text = wxDynamicCast( obj, a2dText );
            if ( text && text->GetSelected() )
            {
                if ( !m_hasURL )
                {
                    m_hasURL = true;
                    m_dobjURL.SetURL( text->GetText() );
                }
                if ( !m_hasText )
                {
                    m_hasText = true;
                    m_dobjText.SetText( text->GetText() );
                }
            }
        }
    }
}

template < class parserTypeIn, class parserTypeOut >
a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::~a2dDnDCanvasObjectData() 
{
    m_drawingPart = NULL;
    m_drawing = NULL;
}

template < class parserTypeIn, class parserTypeOut >
void a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::Append( a2dCanvasObject* canvasobject )
{
    if ( !m_drawing )
    {
        m_drawing = new a2dDrawing();
        m_drawing->SetCommandProcessor( NULL );
    }

    if ( canvasobject )
    {
        a2dCanvasObject* copy = canvasobject->TClone( a2dObject::clone_deep | a2dObject::clone_noCameleonRef | a2dObject::clone_noReference, &m_refs );
        m_drawing->Append( copy );
    }
}

template < class parserTypeIn, class parserTypeOut >
void a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::Reconnect()
{
    // if template is a bunch of connected objects, the clone needs to be reconnected.
    m_refs.LinkReferences( true );
    a2dCanvasObjectPtr shape = m_drawing->GetRootObject();
    shape->GetBbox().SetValid( false );
    shape->Update( a2dCanvasObject::updatemask_force );
}

template < class parserTypeIn, class parserTypeOut >
a2dCanvasObjectList* a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::GetObjects() const
{
    return m_drawing->GetRootObject()->CreateChildObjectList();
}

template < class parserTypeIn, class parserTypeOut >
wxString a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::GetURL () const
{
    if ( m_hasURL )
        return m_dobjURL.GetURL();
    return wxEmptyString;
}

template < class parserTypeIn, class parserTypeOut >
wxString a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::GetText() const
{
    if ( m_hasText )
        return m_dobjText.GetText();
    return wxEmptyString;
}

template < class parserTypeIn, class parserTypeOut >
wxDataFormat a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::GetPreferredFormat(Direction WXUNUSED(dir)) const
{
    return m_formatShape;
}

template < class parserTypeIn, class parserTypeOut >
size_t a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::GetFormatCount(Direction dir) const
{
    // our custom formats are supported by both GetData() and SetData()
    size_t nFormats = 0;
    if ( dir == Get )
    {
        size_t index = 0;
        if ( m_hasShapes )
        {
            nFormats++;
        }

        if ( m_hasURL )
            nFormats += m_dobjURL.GetFormatCount(dir);
        if ( m_hasText )
            nFormats += m_dobjText.GetFormatCount(dir);

        //if ( m_hasBitmap )
            nFormats += m_dobjBitmap.GetFormatCount(dir);

#if wxUSE_METAFILE
        //if ( m_hasMetaFile )
            nFormats += m_dobjMetaFile.GetFormatCount(dir);
#endif // wxUS
    }
    else
    {
        size_t index = 0;
        nFormats++;

        nFormats += m_dobjURL.GetFormatCount(dir);
        nFormats += m_dobjText.GetFormatCount(dir);
        nFormats += m_dobjBitmap.GetFormatCount(dir);
    }
    return nFormats;
}

template < class parserTypeIn, class parserTypeOut >
void a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::GetAllFormats(wxDataFormat *formats, Direction dir) const
{
    if ( dir == Get )
    {
        size_t index = 0;
        if ( m_hasShapes )
        {
            formats[index++] = m_formatShape;
        }

        if ( m_hasURL )
        {
            m_dobjURL.GetAllFormats(&formats[index], dir);
            index += m_dobjURL.GetFormatCount(dir);
        }
        if ( m_hasText )
        {
            m_dobjText.GetAllFormats(&formats[index], dir);
            index += m_dobjText.GetFormatCount(dir);
        }
        // in Get direction we additionally support bitmaps and metafiles
        // under Windows
        //if ( m_hasBitmap )
        {
            m_dobjBitmap.GetAllFormats(&formats[index], dir);
            index += m_dobjBitmap.GetFormatCount(dir);
        }
#if wxUSE_METAFILE
        // don't assume that m_dobjBitmap has only 1 format
        //if ( m_hasMetaFile )
            m_dobjMetaFile.GetAllFormats(&formats[index], dir);
#endif // wxUSE_METAFILE
    }
    else
    {
        size_t index = 0;
        formats[index++] = m_formatShape;

        m_dobjURL.GetAllFormats(&formats[index], dir);
        index += m_dobjURL.GetFormatCount(dir);
        m_dobjText.GetAllFormats(&formats[index], dir);
        index += m_dobjText.GetFormatCount(dir);

        // in Get direction we additionally support bitmaps and metafiles
        // under Windows
        m_dobjBitmap.GetAllFormats(&formats[index], dir);
        index += m_dobjBitmap.GetFormatCount(dir);
    }
}

template < class parserTypeIn, class parserTypeOut >
size_t a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::GetDataSize(const wxDataFormat& format) const
{
    if ( format == m_formatShape )
    {
        if ( !m_hasShapes )
            return 0;

        // convert to a memory string by writing it to CVG.

        // Set LC_NUMERIC facet to "C" locale for this I/O operation
        // !!!! It would be better to set the locale of the stream itself,
        // but this does not work for two reasons:
        // 1. wxWindows streams don't support this
        // 2. CVG-IO uses e.g. wxString::ToDouble, which is independent of the stream
      //a2dCNumericLocaleSetter locSetter;

        m_mem.str("");
        if ( m_drawing->GetRootObject()->GetChildObjectsCount() )
        {
            parserTypeOut CVGwriter;
            CVGwriter.SaveStartAt( m_mem, m_drawing, m_drawing->GetRootObject() );
            //CVGwriter.SaveStartAt( m_mem, m_regler->GetDrawing(), m_regler->GetDrawing()->GetRootObject() );

    /* to debug contents
            wxFFileOutputStream file("c:/data/soft/out2.txt");
            wxStdOutputStream out(file);

            out << m_mem.str() << std::endl;
    */
        }
        m_mem.seekp(0);

        return m_mem.str().length() + 10;
    }
#if wxUSE_METAFILE
    else if ( m_dobjMetaFile.IsSupported(format) )
    {
        if ( !m_hasMetaFile )
            CreateMetaFile();

        return m_dobjMetaFile.GetDataSize(format);
    }
#endif // wxUSE_METAFILE
    else if ( m_dobjURL.IsSupported(format) )
    {
        return m_dobjURL.GetDataSize(format);
    }
    else if ( m_dobjText.IsSupported(format) )
    {
        return m_dobjText.GetDataSize();
    }
    else
    {
        wxASSERT_MSG( m_dobjBitmap.IsSupported(format),
                      wxT("unexpected format") );

        if ( !m_hasBitmap )
            CreateBitmap();

        return m_dobjBitmap.GetDataSize();
    }
}

template < class parserTypeIn, class parserTypeOut >
bool a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::GetDataHere(const wxDataFormat& format, void *pBuf) const
{
    if ( format == m_formatShape )
    {
      
#if wxUSE_STD_IOSTREAM
        wxString str = m_mem.str();
        wxStrcpy( (char*) pBuf, str );
#else
        pBuf = m_mem.GetString();
#endif

       if ( str.Len() <= 10 )
           return false;
       return true;
    }
#if wxUSE_METAFILE
    else if ( m_dobjMetaFile.IsSupported(format) )
    {
        if ( !m_hasMetaFile )
            CreateMetaFile();

        return m_dobjMetaFile.GetDataHere(format, pBuf);
    }
#endif // wxUSE_METAFILE
    else if ( m_dobjURL.IsSupported(format) )
    {        
        return m_dobjURL.GetDataHere(format, pBuf);
    }
    else if ( m_dobjText.IsSupported(format) )
    {        
        return m_dobjText.GetDataHere(format, pBuf);
    }
    else
    {
        wxASSERT_MSG( m_dobjBitmap.IsSupported(format),
                      wxT("unexpected format") );

        if ( !m_hasBitmap )
        {
            CreateBitmap();
        }
        return m_dobjBitmap.GetDataHere(pBuf);
    }
}

template < class parserTypeIn, class parserTypeOut >
bool a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::SetData(const wxDataFormat& format, size_t len, const void *buf )
{
    if ( !m_drawing )
    {
        m_drawing = new a2dDrawing();
        m_drawing->SetCommandProcessor( NULL );
    }

    if ( format == m_formatShape )
    {      
        wxCharBuffer cvgString = wxCharBuffer( (const char*) buf );

        // Set LC_NUMERIC facet to "C" locale for this I/O operation
        // !!!! It would be better to set the locale of the stream itself,
        // but this does not work for two reasons:
        // 1. wxWindows streams don't support this
        // 2. CVG-IO uses e.g. wxString::ToDouble, which is independent of the stream
      //a2dCNumericLocaleSetter locSetter;

    #if wxART2D_USE_CVGIO 
        #if wxUSE_STD_IOSTREAM
            #if wxUSE_UNICODE
                a2dDocumentStringInputStream stream( cvgString.data(), wxSTD ios_base::in );
            #else
                a2dDocumentStringInputStream stream( cvgString, wxSTD ios_base::in );
            #endif // wxUSE_UNICODE
        #else
            a2dDocumentStringInputStream stream( cvgStringtoExecute );
        #endif

        parserTypeIn CvgString;

        a2dCanvasObject* root = m_drawing->GetRootObject();
        CvgString.Load( stream, m_drawing, root );
        a2dWalker_Renumber renumber;
        renumber.Start( root );

        a2dCanvasObject::SetIgnoreAllSetpending( true );
        a2dCanvasObjectList* objects = root->GetChildObjectList();
        forEachIn( a2dCanvasObjectList, objects )
        {
            a2dCanvasObject* obj = *iter;
            obj->Translate( -m_dragStartX, -m_dragStartY);
        }
        a2dCanvasObject::SetIgnoreAllSetpending( false );

        //we have shapes from the clipboard stored here
        m_hasShapes = true;

        m_hasURL = m_hasText = false;
        
        // if text object available in copied object, make text as paste available.
        forEachIn( a2dCanvasObjectList, objects )
        {
            a2dCanvasObject* obj = *iter;
            a2dText* text = wxDynamicCast( obj, a2dText );
            if ( text )
            {
                if ( !m_hasURL )
                {
                    m_hasURL = true;
                    m_dobjURL.SetURL( text->GetText() );
                }
                if ( !m_hasText )
                {
                    m_hasText = true;
                    m_dobjText.SetText( text->GetText() );
                }
            }
        }

        // the shape has changed
        m_hasBitmap = false;
        
        #if wxUSE_METAFILE
            m_hasMetaFile = false;
        #endif // wxUSE_METAFILE
        return true;
    #else
        return false;
    #endif //wxART2D_USE_CVGIO 
    
        //m_canvasobject = new a2dRect( 0,0, 3000,4000);
        return true;
    }
    else if ( m_dobjURL.IsSupported(format) )
    {
        // we create a pure text objects, and an a2dText object, the aplication decides what it wants to have.
        m_dobjURL.SetData( format, len, buf);
        m_hasURL = true;

        a2dText* text = NULL;
        //m_regler->GetDrawing()->Append( new a2dText( m_dobjURL.GetURL(), 0, 0, a2dCanvasGlobals->GetTextTemplateObject()->GetFont(), 0, true ) );
        text = a2dCanvasGlobals->GetHabitat()->GetTextTemplateObject()->TClone( a2dObject::clone_deep );
        text->SetText( m_dobjURL.GetURL() );
        m_drawing->Append( text );
        //we have shapes created from data on the clipboard stored here
        m_hasShapes = true;
        return true;
    }
    else if ( m_dobjText.IsSupported(format) )
    {
        // we create a pure text objects, and an a2dText object, the aplication decides what it wants to have.
        m_dobjText.SetData(len, buf);
        m_hasText = true;

        a2dText* text = NULL;
        //m_regler->GetDrawing()->Append( new a2dText( m_dobjURL.GetURL(), 0, 0, a2dCanvasGlobals->GetTextTemplateObject()->GetFont(), 0, true ) );
        text = a2dCanvasGlobals->GetHabitat()->GetTextTemplateObject()->TClone( a2dObject::clone_deep );
        text->SetText( m_dobjText.GetText() );
        m_drawing->Append( text );
        //we have shapes created from data on the clipboard stored here
        m_hasShapes = true;
        return true;
    }
    else if ( m_dobjBitmap.IsSupported(format) )
    {
        m_dobjBitmap.SetData(len, buf);
        m_drawing = new a2dDrawing();

        double w = m_dobjBitmap.GetBitmap().GetWidth();
        double h = m_dobjBitmap.GetBitmap().GetHeight();
        if ( m_importWidthHeightImage )
        {
            if ( w > h )
            {
                h = m_importWidthHeightImage * h/w;
                w = m_importWidthHeightImage;
            }
            else
            {
                w = m_importWidthHeightImage * w/h;
                h = m_importWidthHeightImage;
            }
            a2dImage* im = new a2dImage( m_dobjBitmap.GetBitmap().ConvertToImage(), 0, 0, w, h );
            im->SetFilename( im->GetName() + ".jpg", wxBITMAP_TYPE_JPEG, false );
            im->SetFill( *a2dTRANSPARENT_FILL );
            m_drawing->Append( im );
        }
        else
        {
            a2dImage* im = new a2dImage( m_dobjBitmap.GetBitmap().ConvertToImage(), 0, 0, w, h );
            im->SetFilename( im->GetName() + ".jpg", wxBITMAP_TYPE_JPEG, false );
            im->SetFill( *a2dTRANSPARENT_FILL );
            m_drawing->Append( im );
        }
        
        //we have shapes created from data on the clipboard stored here
        m_hasShapes = true;
        m_hasBitmap = true;
        return true;
    }
    return false;
}

#if wxUSE_METAFILE

template < class parserTypeIn, class parserTypeOut >
void a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::CreateMetaFile() const
{
    //wxPoint pos = m_canvasobject->GetPosXY();

    wxMetaFileDC dcMF;//(wxEmptyString, pos.x + size.x, pos.y + size.y);

    m_drawing->GetRootObject()->GetBbox().SetValid( false );
    m_drawing->GetRootObject()->Update( a2dCanvasObject::updatemask_force );
    RenderToDc( &dcMF, m_drawing->GetRootObject() );

    wxMetafile *mf = dcMF.Close();

    a2dDnDCanvasObjectData *self = (a2dDnDCanvasObjectData *)this; // const_cast
    self->m_dobjMetaFile.SetMetafile(*mf);
    self->m_hasMetaFile = true;

    delete mf;
}

#endif // wxUSE_METAFILE

template < class parserTypeIn, class parserTypeOut >
void a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::CreateBitmap() const
{
    wxBitmap bitmap = RenderImage( m_drawing->GetRootObject() );
    a2dDnDCanvasObjectData *self = (a2dDnDCanvasObjectData *)this; // const_cast
    self->m_dobjBitmap.SetBitmap(bitmap);
    self->m_hasBitmap = true;
}


template < class parserTypeIn, class parserTypeOut >
wxBitmap a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::RenderImage( a2dCanvasObject* canvasobject ) const
{
    wxSize imageSize = wxSize( 1000, 1000 );
    if ( m_drawingPart )
        imageSize = wxSize( m_drawingPart->GetDrawer2D()->GetWidth(), m_drawingPart->GetDrawer2D()->GetHeight() );

    int maxWidthHeight = wxMax( imageSize.x, imageSize.y );
    if ( m_exportWidthHeightImage )
        maxWidthHeight = m_exportWidthHeightImage;

    a2dBoundingBox viewBox;
    if ( canvasobject->GetBbox().GetValid() )
    {
        // view will be based on showobject its boundingbox
        viewBox = canvasobject->GetBbox();
        a2dAffineMatrix cworld = canvasobject->GetTransformMatrix();
        cworld.Invert();
        viewBox.MapBbox( cworld );
    }

    double w = viewBox.GetWidth();
    double h = viewBox.GetHeight();

    if ( w == 0 )
        w = 1000;
    if ( h == 0 )
        h = 1000;

    //virtual coordinates box its miminum X ( calculated to
    double virt_minX;
    //virtual coordinates box its miminum Y
    double virt_minY;
    //user units per pixel in x
    double xpp;
    //user units per pixel in y
    double ypp;

    //first calculate an image size that fits the drawing part size.
    xpp = w / maxWidthHeight;
    ypp = h / maxWidthHeight;
    if ( xpp > ypp )
       imageSize = wxSize( maxWidthHeight, ( int ) ( h / xpp ) );
    else
       imageSize = wxSize( ( int ) ( w / ypp ), maxWidthHeight );

    // the rect to show in world coordinates is calculated,
    // such that it fits within the image.
    double uppx;
    double uppy;
    int clientw = imageSize.GetWidth();
    int clienth = imageSize.GetHeight();

    int borderpercentual = 10;
    int border = ( int ) ( wxMin( clientw, clienth ) * borderpercentual / 100.0 ) ;

    if ( clientw - border > 0 )
        clientw = clientw - border;

    if ( clienth - border > 0 )
        clienth = clienth - border;

    uppx = w / clientw;
    uppy = h / clienth;
    if ( uppx < uppy )
        uppx = uppy;

    double middlexworld = viewBox.GetMinX() + w / 2.0;
    double middleyworld = viewBox.GetMinY() + h / 2.0;
    virt_minX = middlexworld - ( clientw + border ) / 2.0 * uppx;
    virt_minY = middleyworld - ( clienth + border ) / 2.0 * uppx;
    xpp = uppx;
    ypp = uppx;

    a2dMemDcDrawer dcdrawer( imageSize.GetWidth(), imageSize.GetHeight() );
    dcdrawer.SetMappingDeviceRect( 0, 0, imageSize.GetWidth(), imageSize.GetHeight() );

    a2dSmrtPtr<a2dDrawingPart> drawPart;
    if ( m_drawingPart )
        drawPart = new a2dDrawingPart( *m_drawingPart );    
    else
        drawPart = new a2dDrawingPart( imageSize.GetWidth(), imageSize.GetHeight() );   

    //drawView is a a2dView and therefore gets events, but this is not wanted here.
    drawPart->SetEvtHandlerEnabled( false );
    drawPart->SetDrawer2D( &dcdrawer );
    drawPart->SetShowOrigin( false );
    drawPart->SetGrid( false );
    drawPart->SetShowObject( canvasobject );
    dcdrawer.SetYaxis( false );
    if ( m_drawingPart )
    {
        drawPart->SetDocumentDrawStyle( m_drawingPart->GetDocumentDrawStyle() );
        dcdrawer.SetYaxis( m_drawingPart->GetDrawer2D()->GetYaxis() );
    }
    drawPart->SetAvailable();

    a2dWalker_SetSpecificFlagsCanvasObjects setflags2( a2dCanvasOFlags::SELECTED | a2dCanvasOFlags::SELECTED2 );
    setflags2.SetSkipNotRenderedInDrawing( true );
    setflags2.Start( canvasobject, false );

    drawPart->SetBackgroundFill( *a2dWHITE_FILL );
    dcdrawer.BeginDraw();
    drawPart->GetDrawer2D()->SetMappingUpp( virt_minX , virt_minY, xpp, ypp );
    drawPart->UpdateArea( 0, 0, imageSize.GetWidth(), imageSize.GetHeight() );

    dcdrawer.EndDraw();

    wxBitmap bitmap = dcdrawer.GetBuffer();

    drawPart->SetDrawer2D( NULL, true );

    return bitmap;
}

template < class parserTypeIn, class parserTypeOut >
void a2dDnDCanvasObjectData< parserTypeIn, parserTypeOut >::RenderToDc( wxDC* dc, a2dCanvasObject* canvasobject ) const
{
    wxCoord dcw, dch;
    dc->GetSize( &dcw, &dch );

    wxSize imageSize = wxSize( 1000, 1000 );
    if ( m_drawingPart )
        imageSize = wxSize( m_drawingPart->GetDrawer2D()->GetWidth(), m_drawingPart->GetDrawer2D()->GetHeight() );

    int maxWidthHeight = wxMax( imageSize.x, imageSize.y );
    if ( m_exportWidthHeightImage )
        maxWidthHeight = m_exportWidthHeightImage;

    a2dBoundingBox viewBox;
    if ( canvasobject->GetBbox().GetValid() )
    {
        // view will be based on the boundingbox
        viewBox = canvasobject->GetBbox();
        a2dAffineMatrix cworld = canvasobject->GetTransformMatrix();
        cworld.Invert();
        viewBox.MapBbox( cworld );
    }

    double w = viewBox.GetWidth();
    double h = viewBox.GetHeight();

    if ( w == 0 )
        w = 1000;
    if ( h == 0 )
        h = 1000;

    //virtual coordinates box its miminum X ( calculated to
    double virt_minX;
    //virtual coordinates box its miminum Y
    double virt_minY;
    //user units per pixel in x
    double xpp;
    //user units per pixel in y
    double ypp;

    //first calculate an image size that fits the drawing part size.
    xpp = w / maxWidthHeight;
    ypp = h / maxWidthHeight;
    if ( xpp > ypp )
       imageSize = wxSize( maxWidthHeight, ( int ) ( h / xpp ) );
    else
       imageSize = wxSize( ( int ) ( w / ypp ), maxWidthHeight );

    // the rect to show in world coordinates is calculated,
    // such that it fits within the image.
    double uppx;
    double uppy;
    int clientw = imageSize.GetWidth();
    int clienth = imageSize.GetHeight();

    int borderpercentual = 10;
    int border = ( int ) ( wxMin( clientw, clienth ) * borderpercentual / 100.0 ) ;

    if ( clientw - border > 0 )
        clientw = clientw - border;

    if ( clienth - border > 0 )
        clienth = clienth - border;

    uppx = w / clientw;
    uppy = h / clienth;
    if ( uppx < uppy )
        uppx = uppy;

    double middlexworld = viewBox.GetMinX() + w / 2.0;
    double middleyworld = viewBox.GetMinY() + h / 2.0;
    virt_minX = middlexworld - ( clientw + border ) / 2.0 * uppx;
    virt_minY = middleyworld - ( clienth + border ) / 2.0 * uppx;
    xpp = uppx;
    ypp = uppx;

    a2dDcDrawer dcdrawer( imageSize.GetWidth(), imageSize.GetHeight() );
    dcdrawer.SetMappingDeviceRect( 0, 0, imageSize.GetWidth(), imageSize.GetHeight() );

    a2dSmrtPtr<a2dDrawingPart> drawPart;
    if ( m_drawingPart )
        drawPart = new a2dDrawingPart( *m_drawingPart );    
    else
        drawPart = new a2dDrawingPart( imageSize.GetWidth(), imageSize.GetHeight() );   

    //drawView is a a2dView and therefore gets events, but this is not wanted here.
    drawPart->SetEvtHandlerEnabled( false );
    drawPart->SetDrawer2D( &dcdrawer );
    dcdrawer.SetRenderDC( dc ); //a trick to set the drawers internal dc.
    drawPart->SetShowOrigin( false );
    drawPart->SetGrid( false );
    drawPart->SetShowObject( canvasobject );
    dcdrawer.SetYaxis( false );
    if ( m_drawingPart )
    {
        drawPart->SetDocumentDrawStyle( m_drawingPart->GetDocumentDrawStyle() );
        dcdrawer.SetYaxis( m_drawingPart->GetDrawer2D()->GetYaxis() );
    }
    drawPart->SetAvailable();

    a2dWalker_SetSpecificFlagsCanvasObjects setflags2( a2dCanvasOFlags::SELECTED | a2dCanvasOFlags::SELECTED2 );
    setflags2.SetSkipNotRenderedInDrawing( true );
    setflags2.Start( canvasobject, false );

    drawPart->SetBackgroundFill( *a2dWHITE_FILL );
    dcdrawer.BeginDraw();
    drawPart->GetDrawer2D()->SetMappingUpp( virt_minX , virt_minY, xpp, ypp );
    drawPart->UpdateArea( 0, 0, imageSize.GetWidth(), imageSize.GetHeight() );

    dcdrawer.EndDraw();
    dcdrawer.SetRenderDC( NULL );
    drawPart->SetDrawer2D( NULL, true );
/*
    dc->SetBrush( *wxTRANSPARENT_BRUSH );
    dc->SetPen( *wxRED_PEN );
    dc->DrawRectangle( 0,0, 100, 20 );
    dc->DrawRectangle( 20,20, 50, 30 );
*/
    return;
}

class a2dDnDCameleonData : public a2dDnDCanvasObjectDataCVG
{
public:

    a2dDnDCameleonData( a2dCanvasObject* canvasobject = (a2dCanvasObject*) NULL, double dragStartX = 0, double dragStartY = 0, a2dDrawingPart* drawingPart = NULL  );

    virtual ~a2dDnDCameleonData();

    // implement base class pure virtuals
    // ----------------------------------

    virtual size_t GetDataSize(const wxDataFormat& format) const;

    virtual bool SetData(const wxDataFormat& format,
                         size_t WXUNUSED(len), const void *buf);
private:

};

// ----------------------------------------------------------------------------
// wxDropTarget derivation for DnDShapes
// ----------------------------------------------------------------------------

class a2dDnDCanvasObjectDropTarget : public wxDropTarget
{
public:
    a2dDnDCanvasObjectDropTarget( a2dDrawingPart* drawingPart )
        : wxDropTarget(new a2dDnDCanvasObjectDataCVG( NULL ) )
    {
        m_drawingPart = drawingPart;
    }

    // override base class (pure) virtuals
    virtual wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def)
    {
#if wxUSE_STATUSBAR
        //if ( m_drawingPart && m_drawingPart->GetDisplayWindow() )
            //m_drawingPart->GetDisplayWindow()->SetStatusText(wxT("Mouse entered the frame"));
#endif // wxUSE_STATUSBAR
        return OnDragOver(x, y, def);
    }
    virtual void OnLeave()
    {
#if wxUSE_STATUSBAR
        //if ( m_drawingPart && m_drawingPart->GetDisplayWindow() )
            //m_frame->SetStatusText(wxT("Mouse left the frame"));
#endif // wxUSE_STATUSBAR
    }
    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def)
    {
        if ( !GetData() )
        {
            wxLogError(wxT("Failed to get drag and drop data"));

            return wxDragNone;
        }

        if ( m_drawingPart && m_drawingPart->GetDisplayWindow() )
        {
            m_drawingPart->OnDrop(x, y,
                        ((a2dDnDCanvasObjectDataCVG *)GetDataObject())->GetDrawing());
        }
        return def;
    }

private:
    a2dDrawingPart *m_drawingPart;
};

#endif //wxUSE_DRAG_AND_DROP


#if wxUSE_PRINTING_ARCHITECTURE
//! to print what is displayed on a a2dDrawingPart or the whole document as seen from the showobject of the drawer.
/*!
    Internal a second wxDarwer class is initiated with a mapping that nicely fits that mapping of the input
    a2dDrawingPart to a piece of paper. This mapping depends on the type of print. If only what is on the view needs
    to be printed, the mapping of the input drawer is used, but if the whole document is wanted as seen
    from the ShowObject() of the input drawer, the boundingbox of the showobject will be used for the mapping.


    \ingroup docview
*/
class A2DCANVASDLLEXP a2dDrawingPrintOut: public wxPrintout
{
public:

    //!initialize mapping based on an existing canvas
    /*!
       \param drawer the a2dDrawingPart from which the print is wanted.

       \param title title at top of the print

       \param filename the name of the file to be printed (may be empty )

       \param typeOfPrint When called from a2dDocumentCommandProcessor, the a2dCommand* which lead to this call.

        Depending on the command one can organize printing features.
        Like in the default implementation:
        \code
            a2dPrintWhat
            {
                Print,
                Preview,
                PrintView,
                PreviewView,
                PrintDocument,
                PreviewDocument,
                PrintSetup
            };
        \endcode

        Here View is to only print what is the visible view.
        Document print the document as seen from the
        a2dView::ShowObject(), it fits this to the paper.

        \param drawframe print a frame rectangle in bounding box of drawing/view
        \param scalelimit limits the scaling (world/pixel) to the given value, so that small graphics are not zoomed to full page
        \param fitToPage scale to fit the page
    */
    a2dDrawingPrintOut( const wxPageSetupDialogData& pageSetupData, a2dDrawingPart* drawingPart, const wxString& title, const wxString& filename, a2dPrintWhat typeOfPrint, bool drawframe, double scalelimit, bool fitToPage );

    //!destructor
    ~a2dDrawingPrintOut( void );

    //! if set, printing is done via a bitmap which is drawn into, and next bitmap is printed.
    static void SetPrintAsBitmap( bool printAsBitmap ) { m_printAsBitmap = printAsBitmap; }

    //! if set, printing is done via a bitmap which is drawn into, and next bitmap is printed.
    static bool GetPrintAsBitmap() { return m_printAsBitmap; }

    //! called for every page to print, for a2dDrawing in general just one.
    /*!
       It redraws/rerenders without double buffering the view or document on the pinter its wxDC.
       Internal a a2dDcDrawer is used to redraw the a2dDrawing on the device.
    */
    bool OnPrintPage( int );

    bool HasPage( int pageNum );

    bool OnBeginDocument( int startPage, int endPage );

    void GetPageInfo( int* minPage, int* maxPage, int* selPageFrom, int* selPageTo );

protected:

    const wxPageSetupDialogData& m_pageSetupData;

    //! type of print requested
    a2dPrintWhat m_typeOfPrint;

    //! maping defined by this canvas
    a2dDrawingPart*  m_drawingPart;

    //!title put above printout
    wxString    m_title;

    //!filename put below printout
    wxString    m_filename;

    //! limit scaling to this value (world/pixel)
    double m_scalelimit;

    //! draw a frame around the page
    bool m_drawframe;

    //! draw a view without real scale, the scaling in X and Y may differ.
    //! The drawing is adjusted in X and Y seperately to draw until the sides/border of the printer area
    bool m_fitToPage;

    //! if set print a bitmap that was drawn into
    static bool m_printAsBitmap;
};
#endif

#endif /* __WXDRAWER_H__ */



