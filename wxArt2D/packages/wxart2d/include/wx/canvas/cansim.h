/*! \file wx/canvas/cansim.h
    \brief simple canvas which takes as view the whole of the scrollable area.
    While a2dCanvas display and draws only what is visible, this a2dCanvasSim always
    draws all that will be scrollable. So scrolling is quick.

    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: cansim.h,v 1.15 2008/07/19 18:29:43 titato Exp $
*/

#ifndef __A2DCANVASSIM_H__
#define __A2DCANVASSIM_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/geometry.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/canobj.h"
#include "wx/artbase/bbox.h"
#include "wx/canvas/drawer.h"

//! Simple canvas using a whole view for all of the scrolled window
/*!
    This canvas use a wxScrolledWindow as basis.
    The a2dDrawingPart used to draw has a buffer the size of the virtual scrollable area.
    The scrollable/virtual area is defined in pixels. The window show the part of the virtual area
    which is visible. One can scroll through the contents of the buffer, and the scroll bars show which
    part of the buffer is visble.
    So the scrollable area calculated in world coordinates, defines the maximum boundaries of what can be seen
    of the drawing in the associated a2dCanvasDocument. One can set the mapping matrixes which calculates
    device to world coordinates and visa versa, but this does not influence the scrollbars, since the always
    only show what part of the buffer is visible. Resizing a window, sets the drawing buffer to the new virtual
    size of the scrolled window, but mapping from world to device coordinates stays the same.

    This is rather different from what is done in a2dCanvas, where the scrollable area is defined in
    worldcoordinates, and zooming recalculates the scrollbar positions in such a manner that they indicate
    what part of the drawing is shown in the canvas window as compared to the total visible area.

    \sa a2dCanvas
        a2dDrawingPart
        a2dMemDcDrawer
        a2dCanvasDocument
        a2dCanvasObject

    \ingroup drawer docview
*/
class A2DCANVASDLLEXP a2dCanvasSim: public wxScrolledWindow
{
public:

    //!constructor
    /*!
    construct a canvas window.

    Internal a a2dDrawingPart and  a2dCanvasDocument are created
    to render all objects stored in the a2dCanvasDocument into this a2dCanvas window.

    The document and drawer or deleted in the destructor.


    \remark used for standalone a2dCanvas windows.

    \param parent parent window (use wxNO_FULL_REPAINT_ON_RESIZE on parent wxFrame)
    \param id window id
    \param pos position of window
    \param size size of window
    \param style type of window (wxHSCROLL|wxVSCROLL)

    \remark with new unknown drawer types you can derive or extend the library
    */
    a2dCanvasSim( wxWindow* parent, wxWindowID id = -1,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = wxScrolledWindowStyle );

    //!destructor
    /*!when a tool controller was set is will be deleted also.
    */
    virtual ~a2dCanvasSim();

    //! use the boundingbox of the ShowObject to set the mapping such that it will be displayed completely.
    /*!
        \param centre if true centre on window, else to (0,0) of device
    */
    void SetMappingShowAll( bool centre = true );

	a2dDrawingPart* GetDrawingPart() { return m_drawingPart; }

	void SetDrawingPart( a2dDrawingPart* drawpart );

	a2dDrawing* GetDrawing() const;

    //! Get the drawer of the view
    a2dDrawer2D* GetDrawer2D() const { return m_drawingPart->GetDrawer2D(); }

    //! Clears the canvas (like wxWindow::ClearBackground)
    /*!
     * Calls internally a2dCanvasSim::ClearBackground()
     * Clear() was renamed in wxWin 2.5 to ClearBackground()
     *
     * \todo
     * Remove this method after wxWin 2.6 release.
     */
    void ClearBackground();

    //!background fill for the canvas
    void SetBackgroundFill( const a2dFill& backgroundfill );

    //! Refresh window
    /*!
        Next to base class, makes sure all pending objects are processed, and scrolling is set right.
    */
    virtual void Refresh( bool eraseBackground = true, const wxRect* rect = NULL );

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
    void SetGridLines( bool gridlines ) { m_drawingPart->SetGridLines( gridlines ); }

    //!set grid to draw lines instead of points
    bool GetGridLines() { return m_drawingPart->GetGridLines(); }

    //!Set showorigin on/off
    void SetShowOrigin( bool show ) { m_drawingPart->SetShowOrigin( show ); }

    //!set if the Yaxis goes up or down
    void SetYaxis( bool up );

    //!get currently used Yaxis setting
    bool GetYaxis() const;

    //! zoomout leafs a border of this amount of pixels around the drawing
    void SetZoomOutBorder( wxUint16 border ) { m_border = border; }

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
    */
    void SetMappingWidthHeight( double vx1, double vy1, double width, double height );

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

    //!convert x from window to virtual coordinates
    inline double DeviceToWorldX( int x ) const { return m_drawingPart->GetDrawer2D()->DeviceToWorldX( x ); }
    //!convert y from window to virtual coordinates
    inline double DeviceToWorldY( int y ) const { return m_drawingPart->GetDrawer2D()->DeviceToWorldY( y ); }
    //!convert x relative from window to virtual coordinates
    //!use this to convert a Length of a line for instance
    inline double DeviceToWorldXRel( int x ) const { return m_drawingPart->GetDrawer2D()->DeviceToWorldXRel( x ); }
    //!convert y relative from window to virtual coordinates
    //!use this to convert a Length of a line for instance
    inline double DeviceToWorldYRel( int y ) const  { return m_drawingPart->GetDrawer2D()->DeviceToWorldYRel( y ); }

    //!convert x from virtual to window coordinates
    inline int WorldToDeviceX( double x ) const { return m_drawingPart->GetDrawer2D()->WorldToDeviceX( x ); }
    //!convert y from virtual to window coordinates
    inline int WorldToDeviceY( double y ) const { return m_drawingPart->GetDrawer2D()->WorldToDeviceY( y ); }
    //!convert x relative from virtual to window coordinates
    //!use this to convert a Length of a line for instance
    inline int WorldToDeviceXRel( double x ) const { return m_drawingPart->GetDrawer2D()->WorldToDeviceXRel( x ); }
    //!convert y relative from virtual to window coordinates
    //!use this to convert a Length of a line for instance
    inline int WorldToDeviceYRel( double y ) const { return m_drawingPart->GetDrawer2D()->WorldToDeviceYRel( y ); }

    //!set object available in the a2dCanvasDocument to be shown on the canvas
    //!\param name: name of top object
    //!\return pointer to the object found else NULL
    a2dCanvasObject* SetShowObject( const wxString& name );

    //!set top object available in the a2dCanvasDocument to be shown on the canvas
    //!\param obj: pointer to object to show
    bool SetShowObject( a2dCanvasObject* obj );

    //!return pointer of then currently shown object on the canvas.
    //!\return pointer to the current object that is shown.
    a2dCanvasObject* GetShowObject() const { return m_drawingPart->GetShowObject(); }

    //!do a hittest on the canvas at coordinates x,y
    /*!
    \param x: x of point to do hittest
    \param y: y of point to do hittest
    \param layer only if object is on this layer or if set to wxLAYER_ALL ignore layer id.
    \param option special hittest options
    \return the top object that was hit (e.g.in case of groups)

    \remark hit margin is defined in a2dCanvasDocument containing the root group
    */
    a2dCanvasObject* IsHitWorld(
        double x, double y,
        int layer = wxLAYER_ALL,
        a2dHitOption option = a2dCANOBJHITOPTION_NOROOT | a2dCANOBJHITOPTION_LAYERS
    );

    //!Mouse events are handled by the canvas.
    //!They are redirected to the object hit.
    //!You can switch this off here, used in most tools.
    void SetMouseEvents( bool onoff );

    bool GetMouseEvents();

    //! write what you see to an SVG( scalable vector graphics file )
    bool WriteSVG( const wxString& filename, double Width, double Height, wxString unit );

protected:

    virtual void DoFreeze();
    virtual void DoThaw();

    //! resize, adjusting buffer of a2dDrawingPart if needed.
    void OnSize( wxSizeEvent& event );

    //! repaint damaged araes, taking into acount non updated araes in a2dDrawingPart.
    void OnPaint( wxPaintEvent& event );

    //! Not yet implemented
    void OnEraseBackground( wxEraseEvent& event );

    //! remove all pending update areas in a2dDrawingPart
    void DeleteAllPendingAreas();

private:

    //!oversize of buffer compared to screen width and height
    int m_delta;

	a2dSmrtPtr<a2dDrawingPart> m_drawingPart;

    //! set if the canvas own the drawing
	a2dDrawingPtr m_drawing;

    //! border zoomout but leaf around a border of this amount of pixels.
    wxUint16 m_border;

    DECLARE_CLASS( a2dCanvasSim )
    DECLARE_EVENT_TABLE()

};

#endif
// A2DCANVAS

