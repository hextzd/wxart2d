/*! \file wx/canvas/canvas.h
    \brief a2dCanvasOgl uses a2dDrawingPart for displaying a view on a a2dDrawing.

    a2dCanvas is doing no more then displaying the view in a window, and taking care that
    the view is correctly set when scrolling the view, or when resizing the window.
    When receiving a paint event on the window, it tells the view to redisplay the
    damaged parts. The view then takes those parts from its drawing buffer or if there is not one,
    start redrawing the document where needed.

    \author Robert Roebling and Klaas Holwerda

    Copyright: 2000-2004 (c) Robert Roebling

    Licence: wxWidgets Licence

    RCS-ID: $Id: canvas.h,v 1.17 2008/09/02 19:42:00 titato Exp $
*/

#ifndef __A2DCANVAS_H__
#define __A2DCANVAS_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/geometry.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/canobj.h"
#include "wx/artbase/bbox.h"
#include "wx/canvas/drawer.h"

//! a2dCanvas is used to display one of the a2dCanvasObjects which are part of a a2dCanvasDocument object and all that is nested in that choosen object.
/*!
This is a canvas that defines the viewport in world coordinates.
The area of the a2dCanvasDocument drawing in world coordinates that is visible on the canvas
can be set, which is called mapping.
Parts of this area can be zoomed into resulting in scroll bars
to be displayed.

Most functions here just redirect the calls to the member a2dDrawingPart.
They are available for convenience, but all of them can be accessed through the GetCanvasView()
member function also.

\sa a2dDrawingPart for more documentation.

The main thing done is to have the result of the a2dDrawingPart rendering, blitted to the screen.
The scrolling bars adjust the a2dDrawingPart mapping to have the right mapping again after a scroll.
It is optimized to miminize redrawing.

An update of the damaged parts will blitted from the a2dDrawingPart buffer to the screen.
This is done in Idle time, but can also be forced.

Settings like background drawing and grid drawing is done through the a2dDrawingPart member.

The area of the drawing in world coordinates that is visible on the canvas
can be set.
Parts of this area can be zoomed into resulting in scroll bars
to be displayed.

\sa a2dDrawingPart
\sa a2dMemDcDrawer
\sa a2dCanvasDocument
\sa a2dCanvasObject

    \ingroup drawer docview
*/
class A2DCANVASDLLEXP a2dCanvas: public wxWindow
{
public:

    //!constructor
    /*!
    construct a canvas window.

    Internal a a2dDrawingPart and  a2dDrawing are created
    to render all objects stored in the a2dDrawing into this a2dCanvas window.

    The document and view or deleted in the destructor.

    \remark used for standalone a2dCanvas windows.

    \param parent parent window (use wxNO_FULL_REPAINT_ON_RESIZE on parent wxFrame)
    \param id window id
    \param pos position of window
    \param size size of window
    \param style type of window (wxHSCROLL|wxVSCROLL)
    \param drawer2D the drawer to be used to render to the window ( construct with zero size, it will be set to the canvas size )
        If 0 the default is a2dMemDcDrawer()
    */
    a2dCanvas( wxWindow* parent, wxWindowID id = -1,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxScrolledWindowStyle, a2dDrawer2D* drawer2D = 0 );

    //!constructor
    /*!
    construct a canvas window, normally created by a a2dDrawingPart.

    \remark used when using a2dDocumentCommandProcessor classes.

    \param view a2dDrawingPart object used to render into this window.
    \param parent parent window (use wxNO_FULL_REPAINT_ON_RESIZE on parent wxFrame)
    \param id window id
    \param pos position of window
    \param size size of window
    \param style type of window (wxHSCROLL|wxVSCROLL)
    */
    a2dCanvas( a2dDrawingPart* drawingPart, wxWindow* parent, wxWindowID id = -1,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxScrolledWindowStyle );

    //!destructor
    /*!when a tool controller was set is will be deleted also.
    */
    virtual ~a2dCanvas();

	a2dDrawingPart* GetDrawingPart() { return m_drawingPart; }

	void SetDrawingPart( a2dDrawingPart* drawpart );

	a2dDrawing* GetDrawing() const;

	void SetDrawing( a2dDrawing* drawing );

    //! Get the drawer of the view
    a2dDrawer2D* GetDrawer2D() const { return m_drawingPart->GetDrawer2D(); }

    //! Clears the canvas (like wxWindow::ClearBackground)
    /*!
     * Calls internally a2dCanvas::ClearBackground()
     * Clear() was renamed in wxWin 2.5 to ClearBackground()
     *
     * \todo
     * Remove this method after wxWin 2.6 release.
     */
    void ClearBackground();

    //! Refresh window
    /*!
        Next to base class, makes sure all pending objects are processed, and scrolling is set right.
    */
    virtual void Refresh( bool eraseBackground = true, const wxRect* rect = NULL );

    //!background fill for the canvas
    void SetBackgroundFill( const a2dFill& backgroundfill );

    //!Set grid setting for drawing grid in front or back
    void SetGridAtFront( bool gridatfront ) { m_drawingPart->SetGridAtFront( gridatfront ); }

    //!Get grid setting for drawing grid in front or back
    bool GetGridAtFront() { return m_drawingPart->GetGridAtFront(); }

    //!set stroke used for grid drawing
    void SetGridStroke( const a2dStroke& gridstroke ) { m_drawingPart->SetGridStroke( gridstroke ); }

    //!set size of grid circle
    void SetGridSize( wxUint16 gridsize ) { m_drawingPart->SetGridSize( gridsize ); }

    //!set brush used for grid drawing
    void SetGridFill( const a2dFill& gridfill ) { m_drawingPart->SetGridFill( gridfill ); }

    //!Get grid distance in X
    double GetGridX() { return m_drawingPart->GetGridX(); }

    //!Set grid distance in X
    void SetGridX( double gridx ) { m_drawingPart->SetGridX( gridx ); }

    //!Get grid distance in Y
    double GetGridY() { return m_drawingPart->GetGridY(); }

    //!Set grid distance in Y
    void SetGridY( double gridy ) { m_drawingPart->SetGridY( gridy ); }

    //!Set grid on/off
    void SetGrid( bool grid ) { m_drawingPart->SetGrid( grid ); }

    //!Get grid setting on/off
    bool GetGrid() { return m_drawingPart->GetGrid(); }

    //!Get grid setting for line drawing
    /*!Note: SetGridSize must be 0, if gridlines=false */
    void SetGridLines( bool gridlines ) { m_drawingPart->SetGridLines( gridlines ); }

    //!set grid to draw lines instead of points
    bool GetGridLines() { return m_drawingPart->GetGridLines(); }

    //!Set showorigin on/off
    void SetShowOrigin( bool show ) { m_drawingPart->SetShowOrigin( show ); }

    //!set if the Yaxis goes up or down
    void SetYaxis( bool up );

    //!get currently used Yaxis setting
    bool GetYaxis() const;

    //!set if the drawing should be resclaed on a window resize
    /*!
       When set, each resize of the window, result in scaling the visible draing up or down.
       The center of the window in world coordinates is kept the same.
       The drawing itself is scaled to the smallest of the width or height.

       When m_ContinuesSizeUpdate is set each resize will scale the drawing, else
       only after a double buffer enlargement.
    */
    void SetScaleOnResize( bool val ) { m_scaleonresize = val; }

    //!get current setting for ScaleOnResize
    bool GetScaleOnResize() { return m_scaleonresize; }

    //!set if the drawing should be rescaled on a window resize
    void SetContinuesSizeUpdate( bool val ) { m_ContinuesSizeUpdate = val; }

    //!get current setting for ContinuesSizeUpdate flag
    bool GetContinuesSizeUpdate() { return m_ContinuesSizeUpdate; }

    //!append an eventhandler to the list, this event handler will be called if the other skipped the event to process.
    void AppendEventHandler( wxEvtHandler* handler );

    //!remove last event handler in the list
    wxEvtHandler* RemoveLastEventHandler( bool deleteHandler );

    //!Give the virtual size to be displayed, the mappingmatrix will be calculated.
    /*!
    The current window size in pixels is used to calculate the mapping such that
    at least it will display all of the area given.

    Setting virtual area to boundingbox of  a drawing (currently visible ShowObject()
     \code
     m_worldcanvas->SetMappingWidthHeight(m_worldcanvas->GetShowObject()->GetXMin(),
                               m_worldcanvas->GetShowObject()->GetYMin(),
                               m_worldcanvas->GetShowObject()->GetWidth(),
                               m_worldcanvas->GetShowObject()->GetHeight())
     \endcode

    \remark do not use during start up since window size is not well defined in that case resulting in
    \remark bad settings for the mapping.

    \param vx1 minimum world x coordinate
    \param vy1 minimum world y coordiante ( either Lower or Upper Left corner depending on SetYaxis() )
    \param width minimum width in world coordinates which we want to display on this window
    \param height minimum height in world coordinates which we want to display on this window
    \param scrollbars: The scroll region are set to display this also.
    */
    void SetMappingWidthHeight( double vx1, double vy1, double width, double height, bool scrollbars );

    //! use the boundingbox of the ShowObject to set the mapping such that it will be displayed completely.
    /*!
        \param centre if true centre on window, else to (0,0) of device
    */
    void SetMappingShowAll( bool centre = true );

    //! zoom out from the center by a factor n (zoomin if <1 )
    void ZoomOut( double n );

    //! zoom out from position x,y  by a factor n (zoomin if <1 )
    void ZoomOutAtXy( int x, int y, double n );

    //!Give the virtual size to be displayed, the mappingmatrix will be calculated.
    /*!
    To display all of a drawing, set this here to the boundingbox of the show object
    of the canvas.
    So vx1 and vx2 to the miminum x and y of the boundingbox.
    Calculate xpp and ypp in such a manner that it will show the whole drawing.

    Setting virtual area to boundingbox of  a drawing (currently visible group)

     \code
     int dx2,dy2;
     m_canvas->GetClientSize(&dx2,&dy2);
     double xupp=(m_canvas->GetShowObject()->GetWidth())/dx2;
     double yupp=(m_canvas->GetShowObject()->GetHeight())/dy2;
     if (yupp > xupp)
         xupp=yupp;
     m_worldcanvas->SetMappingUpp(m_worldcanvas->GetShowObject->GetXMin(),
                               m_worldcanvas->GetShowObject->GetYMin(),xupp,xupp);
     \endcode

    If a scrollable area is set, it will be called also to adjust it.
    \param vx1 minimum world x coordinate
    \param vy1 minimum world y coordiante ( either Lower or Upper Left corner depending on SetYaxis() )
    \param xpp: Number of world units per pixel in x
    \param ypp: Number of world units per pixel in y
    */
    void SetMappingUpp( double vx1, double vy1, double xpp, double ypp );

    //! Like SetMappingUpp() but uses position x,y as the fixed point for zooming.
    void SetMappingUppAtXy( double x, double y, double vx1, double vy1, double xpp, double ypp );

    //! set scrollbars
    /*!
    \param pixelsPerUnitX the number of pixels per world unit in X
    \param pixelsPerUnitY the number of pixels per world unit in Y
    \param noUnitsX number of scroll units in X
    \param noUnitsY number of scroll units in Y
    \param xPos position of thumb in X
    \param yPos position of thumb in Y
    */
    void SetScrollbars( double pixelsPerUnitX, double pixelsPerUnitY,
                        double noUnitsX, double noUnitsY,
                        double xPos = 0, double yPos = 0 );

    //!get Minimal X of the visible part in world coordinates
    double GetVisibleMinX() const;
    //!get Minimal X of the visible part in world coordinates
    double GetVisibleMinY() const;
    //!get Maximum X of the visible part in world coordinates
    double GetVisibleMaxX() const;
    //!get Maximum Y of the visible part in world coordinates
    double GetVisibleMaxY() const;
    //!get Width of visible part in world coordinates
    double GetVisibleWidth() const;
    //!get Height of visible part in world coordinates
    double GetVisibleHeight() const;

    //!\return xpp Number of world units per pixel in x
    double GetUppX() const;

    //!\return ypp Number of world units per pixel in y
    double GetUppY() const;

    //!scroll the window in world coordinates
    /*!
        In case of FixScrollMaximum it will be done
        if it fits within the scrollmaximum else best possible.
        \return true if succesfull.
    */
    bool ScrollWindowConstrained( double dx, double dy );

    //!to set the total area in world coordinates that can be scrolled to.
    /*!To be able to scroll the whole of a drawing, set this here to the boundingbox
    of the root group of the canvas.
    If the current Virtual display area does not fit inside the scrollable area,
    and the FixScrollMaximum is not set, the scroll area will be adjusted to have it fit.
    else scrollbars are disabled and return value is false.
    */
    bool SetScrollMaximum( double vx1, double vy1, double vx2, double vy2 );

    //! Set horizontal scrollbar position in world coordinates
    void SetMinX( double vxmin );
    //! Set vertical scrollbar position in world coordinates
    void SetMinY( double vymin );

    //!get the minimum scroll X in world coordinates that can be scrolled to
    double GetScrollMinX() const { return m_virtm_minX; }
    //!get the minimum scroll Y in world coordinates that can be scrolled to
    double GetScrollMinY() const { return m_virtm_minY; }
    //!get the maximum scroll X in world coordinates that can be scrolled to
    double GetScrollMaxX() const { return m_virtm_maxX; }
    //!get the maximum scroll Y in world coordinates that can be scrolled to
    double GetScrollMaxY() const { return m_virtm_maxY; }

    //!set granularity (step size) for scrolling in world units for Horizontal Scrollbar
    /*!
        Scrolling is done when receiving  wxEVT_SCROLLWIN_LINEUP wxEVT_SCROLLWIN_LINEDOWN.
        This is function is also internaly used to redraw the scrollbar
    */
    void SetScrollStepX( double x );

    //!set granularity (step size) for scrolling in world units for Vertical Scrollbar
    /*!
        Scrolling is done when receiving wxEVT_SCROLLWIN_LINEUP wxEVT_SCROLLWIN_LINEDOWN.
        This is function is also internaly used to redraw the scrollbar
    */
    void SetScrollStepY( double y );

    //!at all times the virtual displayed area will stay within the maximum scrollable area
    void FixScrollMaximum( bool fixed ) { m_scrollmaxfixed = fixed; }

    //! show scrollbars if set true.
    void SetScrollBarsVisible( bool onoff );

    //! if set, when dragging scrollbar handles, do a redraw, esle only at release.
    void SetScrollTrackDraw( bool scrollTrackDraw ) { m_scrollTrackDraw = scrollTrackDraw; }

    //! if set, when dragging scrollbar handles, do a redraw, esle only at release.
    bool GetScrollTrackDraw() const { return m_scrollTrackDraw; }

    //! when scrolling outside the maximum scroll region and
    /*!
        m_scrollmaxfixed is true, do clip virtual area if this is true
    */
    void ClipToScrollMaximum( bool clip ) { m_clipToScrollMax = clip; }

    //!check if the given scroll in worldcoordinates (X and Y) will stay within ScrollMaximum.
    /*!
        \remark ONLY if  m_scrollmaxfixed is set, else it return true in all cases.
    */
    bool CheckInsideScrollMaximum( double worldminx, double worldminy );

    //!set object available in the a2dCanvasDocument to be shown on the canvas
    /*!
        \param name: name of top object
        \return pointer to the object found else NULL
    */
    a2dCanvasObject* SetShowObject( const wxString& name );

    //!set top object available in the a2dCanvasDocument to be shown on the canvas
    /*!
        \param obj: pointer to object to show
    */
    bool SetShowObject( a2dCanvasObject* obj );

    //!return pointer of then currently shown object on the canvas.
    /*!
        \return pointer to the current object that is shown.
    */
    a2dCanvasObject* GetShowObject() const { return  m_drawingPart->GetShowObject(); }

    //!do a hittest on the canvas at coordinates x,y
    /*!
    \param x: x of point to do hittest
    \param y: y of point to do hittest
    \param layer only if object is on this layer or if set to wxLAYER_ALL ignore layer id.
    \param option in which way the object was hit (stroke, fill, ...)
    \return the top object that was hit (e.g.in case of groups)
    \remark hit margin is defined in a2dCanvasDocument containing the root group
    */
    a2dCanvasObject* IsHitWorld(
        double x, double y,
        int layer = wxLAYER_ALL,
        a2dHitOption option = a2dCANOBJHITOPTION_NOROOT | a2dCANOBJHITOPTION_LAYERS
    );

    //! \see a2dDrawingPart::SetMouseEvents()
    void SetMouseEvents( bool onoff );

    //! \see a2dDrawingPart::GetMouseEvents()
    bool GetMouseEvents();

    //! write what you see to an SVG( scalable vector graphics file )
    bool WriteSVG( const wxString& filename, double Width, double Height, wxString unit );

	bool ProcessEvent( wxEvent& event );

    void SetSizeOgl(){}

protected:

    //! adjust either the maximum scrollable area to contain the current visible area or
    //! if fixed, change the visible area to stay within.
    bool MakeVirtualMaxFitVisibleArea();

    //! same as normal client size, but when dx or dy is zero, it will be set to 1000.
    /*!
        This is to prevent an irational mapping and/or divide by zero problems.
    */
    void GetSaveClientSize( int* dvx, int* dvy ) const;

    //!virtual coordinates of total scrollable area for the drawing, used to set scrollbars
    double m_virtm_minX, m_virtm_minY, m_virtm_maxX, m_virtm_maxY;

    virtual void DoFreeze();
    virtual void DoThaw();

	void OnActivate( wxActivateEvent& event );

    //! repaint damaged araes, taking into acount non updated araes in a2dDrawingPart.
    void OnPaint( wxPaintEvent& event );

    //! resize, adjusting buffer of a2dDrawingPart if needed.
    void OnSize( wxSizeEvent& event );

    //! mouse wheel handler
    void OnWheel( wxMouseEvent& event );

    //! Not yet implemented
    void OnEraseBackground( wxEraseEvent& event );

    //!what to do while scrollling
    void OnScroll( wxScrollWinEvent& event );

    //!what to do for characters received
    void OnChar( wxKeyEvent& event );

    //! remove all pending update areas in a2dDrawingPart
    void DeleteAllPendingAreas();

    //!are scroll bars active?
    bool m_scaleonresize;

    //! set true by wxWindow style parameter wxVSCROLL | wxHSCROLL
    bool m_wantScroll;

    //! when because resizing or setting maximum scrollable area, it becomes smaller then the visible area,
    /*! we can adjust it or adjust units per pixel to make it fit again.
    */
    bool m_aboveScrolledFixedAdjustUpp;

    //!check if the given scroll in pixels (X and Y) will be oke to stay within ScrollMaximum.
    /*!
        \remark ONLY if  m_scrollmaxfixed is set, else it return true in all cases.
        \param dy > contents of window moves up
        \param dx > contents of window moves right
    */
    bool CheckInsideScrollMaximum( int dx, int dy );

    void Init( int dvx, int dvy );

    //! Total range of X scroll bar, which gets always adjusted by the m_scrollstepx
    /*!
        \remark used intenal
    */
    int m_scrollrangex;

    //! Total range of Y scroll bar, which gets always adjusted by the m_scrollstepy
    /*!
        \remark used internal
    */
    int m_scrollrangey;

    //!current thumbsize of X scrollbar which gets always adjusted by the m_scrollstepx
    /*!
        \remark used internal
    */
    int m_thumbx;

    //!current thumbsize of Y scrollbar which gets always adjusted by the m_scrollstepy
    /*!
        \remark used internal
    */
    int m_thumby;

    //!step for X line up and down in world coordinates
    double m_scrollstepx;

    //!step for Y line up and down in world coordinates
    double m_scrollstepy;

    //!are scroll bars active?
    bool m_scrolled;

    //!a flag for ignoring size events when scrollbars change
    bool m_xscroll_changed;

    //!a flag for ignoring size events when scrollbars change
    bool m_yscroll_changed;

    //!flag if there is a horizontal scrollbar
    bool m_has_x_scroll;

    //!flag if there is a vertical scrollbar
    bool m_has_y_scroll;

    //!stay within maximum scrollable area
    bool m_scrollmaxfixed;

    //! see ClipToScrollMaximum()
    bool m_clipToScrollMax;

    //! redraw at drag fo scrollbars
    bool m_scrollTrackDraw;

    //!to optimize onsize events
    int m_oldw, m_oldh, m_prevh, m_prevw, m_prevClientW, m_prevClientH;

    //!oversize of buffer compared to screen width and height
    int m_delta;

    //!in future the offset from window to drawer in X
    int m_oversizeX;

    //!in future the offset from window to drawer in Y
    int m_oversizeY;

	a2dSmrtPtr<a2dDrawingPart> m_drawingPart;

    //! prevent SetScrollMaximum beeing called recursively
    bool m_inSetScrollMaximum;

    //! at small resize also update the buffer directly.
    /*!
        When false, updates only happen when a new buffersize is allocated.
    */
    bool m_ContinuesSizeUpdate;

    //! used to normalize scroll thumb and range
    int m_normalize;

    //! set if the canvas own the drawing
	a2dDrawingPtr m_drawing;

    DECLARE_CLASS( a2dCanvas )
    DECLARE_EVENT_TABLE()

};

#endif /* __A2DCANVAS_H__ */


