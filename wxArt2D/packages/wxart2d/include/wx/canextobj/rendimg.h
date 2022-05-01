/*! \file wx/canextobj/rendimg.h
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: rendimg.h,v 1.10 2008/07/19 18:29:42 titato Exp $
*/

#ifndef __WXRENDIMG_H__
#define __WXRENDIMG_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canobj.h"

class A2DCANVASDLLEXP a2dCanvasDocument;

#include "wx/canvas/drawer.h"



//! a2dRenderImage is an a2dCanvasObject that is able to display a complete a2dCanvasDocument as a a2dCanvasObject.
/*!
 It uses a bitmap to render the specified a2dCanvasDocument (a2dCanvasObject topobject) into.
 And this bitmap is displayed when the a2dRenderImage is
 rendered within the a2dCanvasDocument where this object itself belongs to.

 re-rendering the bitmap happens only when size of the object changes.

 Settings for background drawing and grid drawing is done through the a2dDrawer2D on the a2dDrawingPart member.

\sa class: a2dDrawer2D
\sa class: a2dCanvasDocument
\sa class: a2dCanvasObject

    \ingroup drawer canvasobject meta
*/
class A2DEDITORDLLEXP a2dRenderImage: public a2dCanvasObject
{
public:

    a2dRenderImage( a2dCanvasObject* top, double xc, double yc, double w, double h );

    virtual ~a2dRenderImage();

    a2dRenderImage( const a2dRenderImage& other, CloneOptions options, a2dRefMap* refs );

    a2dDrawingPart* GetDrawingPart() { return m_drawingPart; }

    //!get y axis orientation
    inline bool GetYaxis() const { return m_drawingPart->GetDrawer2D()->GetYaxis(); }

    //!set if the Yaxis goes up or down
    virtual void SetYaxis( bool up ) { m_drawingPart->GetDrawer2D()->SetYaxis( up ); }

    //!background fill for the canvas
    void SetBackgroundFill( const a2dFill& backgroundfill ) { m_drawingPart->SetBackgroundFill( backgroundfill ); }

    //!get current background fill for the canvas
    a2dFill& GetBackgroundFill() { return  m_drawingPart->GetBackgroundFill(); }

    //!Set grid setting for drawing grid in front or back
    void SetGridAtFront( bool gridatfront )  { m_drawingPart->SetGridAtFront( gridatfront ); }

    //!Get grid setting for drawing grid in front or back
    bool GetGridAtFront() { return m_drawingPart->GetGridAtFront(); }

    //!set stroke used for grid drawing
    void SetGridStroke( const a2dStroke& gridstroke )  { m_drawingPart->SetGridStroke( gridstroke ); }

    //!set size of grid circle
    void SetGridSize( int gridsize )  { m_drawingPart->SetGridSize( gridsize ); };

    //!set fill used for grid drawing
    void SetGridFill( const a2dFill& gridfill )  { m_drawingPart->SetGridFill( gridfill ); }

    //!Get grid distance in X
    double GetGridX() {return m_drawingPart->GetGridX();}

    //!Set grid distance in X
    void SetGridX( double gridx )  { m_drawingPart->SetGridX( gridx ); }

    //!Get grid distance in Y
    double GetGridY() { return m_drawingPart->GetGridY(); }

    //!Set grid distance in Y
    void SetGridY( double gridy )  { m_drawingPart->SetGridY( gridy ); }

    //!Set grid on/off
    void SetGrid( bool grid )  { m_drawingPart->SetGrid( grid ); }

    //!Get grid setting on/off
    bool GetGrid()  { return m_drawingPart->GetGrid(); }

    //!Get grid setting for line drawing
    void SetGridLines( bool gridlines )  { m_drawingPart->SetGridLines( gridlines ); }

    //!set grid to draw lines instead of points
    bool GetGridLines() { return m_drawingPart->GetGridLines(); }

    //!Set showorigin on/off
    void SetShowOrigin( bool show )  { m_drawingPart->SetShowOrigin( show ); };

    /*! Give the virtual size to be displayed, the mappingmatrix will be calculated.
        The current window size is used to at least display all of the area given.
        !ex: Setting virtual area to boundingbox of  a drawing (currently visible group)
        !code: SetMappingWidthHeight(GetShowGroup()->GetXMin(),
        !code:            GetShowGroup()->GetYMin(),
        !code:            GetShowGroup()->GetWidth(),
        !code:            GetShowGroup()->GetHeight())
        !comm: do not use during start up since window size is not well defined in that case resulting in
        !comm: bad settings for the mapping.
    */
    void SetMappingWidthHeight( double vx1, double vy1, double width, double height );

    void SetMappingUpp( double vx1, double vy1, double xpp, double ypp );

    //!set mapping using absolute world coordinates of parent
    void SetMappingAbs( a2dIterC& ic, double vx1, double vy1, double width, double height );

    //!set object available in the a2dCanvasDocument to be shown on the canvas
    /*!
        \param  name: name of topobject
        \return pointer to the object found else NULL
    */
    a2dCanvasObject* SetShowObject( const wxString& name );

    //!set top object available in the a2dCanvasDocument to be shown on the canvas
    /*!
        \param obj pointer to object to show
    */
    bool SetShowObject( a2dCanvasObject* obj );

    //!return pointer of then currently shown object on the canvas.
    /*!
        \return pointer to the current group that is shown.
    */
    a2dCanvasObject* GetShowObject() const;

    //!calculate for top the boundingbox's that became invalid.
    /*!
        If force is true, do them all.
    */
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //!set if the drawing should be resclaed on a window resize
    void SetScaleOnResize( bool val ) { m_scaleonresize = val; }

    //!get current setting for ScaleOnResize
    virtual bool GetScaleOnResize() { return m_scaleonresize; }

    void SetWidth( double width );
    double  GetWidth() { return m_width; }

    void SetHeight( double height );
    double  GetHeight() { return m_height; }

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

protected:

    virtual a2dObject*  DoClone( CloneOptions options, a2dRefMap* refs ) const;
    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    //!background brush of canvas; and back ground color of background brush in case of mono colour brush
    a2dFill m_backgroundfill;

    //! showorigin?
    bool m_showorigin;

    //! are scroll bars active?
    bool m_scaleonresize;

    double      m_width;
    double      m_height;


private:

    a2dSmrtPtr<a2dDrawingPart> m_drawingPart;

    // cache
    bool        m_is_cached;
    wxImage     m_cImage;
    bool        m_b_is_cached;
    wxBitmap    m_cBitmap;
    int         m_cW;
    int         m_cH;
    double      m_cR;

    DECLARE_CLASS( a2dRenderImage )

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dRenderImage( const a2dRenderImage& other );
};


#endif /* __WXRENDIMG_H__ */


