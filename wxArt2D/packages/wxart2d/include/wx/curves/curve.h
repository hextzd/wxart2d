/*! \file wx/curves/curve.h
    \brief classes for plotting curve and pie data, and editing them.

    Data stored in a derived a2dCanvasObject, can be plotted as a curve of pie chart.
    One can add markers on the curves, and several curves can be plot as a group in one plot.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: curve.h,v 1.16 2009/10/01 19:22:35 titato Exp $
*/

#ifndef __WXCURVE_H__
#define __WXCURVE_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/curves/meta.h"
#include <wx/general/a2dlist.h>

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

//! a2dCurveObject for objects needing to know its parent a2dCurvesArea.
/*!
    a2dCurveObject is the base class for curves and markers etc. The m_curvesArea is set from
    a2dCanvasXYDisplayGroup to which it be longs. This happens in its OnUpdate() function.
    The m_curveGroup can be used to clip the curves and makers to the plotting area.
    Next to that conversion of curve data from and to plotting area coordinates is done from the
    m_curveGroup. The data stored in derived object ( like coordinates of a curve ), can be in their own units.
    They can be converted to world coordinates ( as used in the a2dCanvasDocument ), and m_curveGroup
    supplies the functions to do this.

    \ingroup canvasobject
*/
class A2DCURVESDLLEXP a2dCurveObject: public a2dCanvasObject
{

public:
    //!default constructor
    a2dCurveObject();

    //!destructor
    ~a2dCurveObject();

    //!copy constructor
    a2dCurveObject( const a2dCurveObject& other, CloneOptions options, a2dRefMap* refs );

    //! used for conversion of curve data to curvegroup area
    /*!

    */
    void SetCurvesArea( a2dCurvesArea* curvesArea ) { m_curvesArea = curvesArea; }

    //! used by a2dCurve to transform its own curve coordinates to the curve plotting area
    /*!
        In general m_curvesArea is used to covert the curve coordinates to the plotting area coordinates.
        Meaning that the m_curvesArea has the same coordinates system as the curve itself.
        Like for instance the grid and axis of the curveArea are fiiting the curve coordinates.
        But if the curve coordinates would be logarithmic etc., one will need an extra consversion.
        This can be done here.
    */
    virtual void World2Curve( double xw, double yw, double& xcurve, double& ycurve ) const;

    //! used by a2dCurve to transform its own curve coordinates to the curve plotting area
    /*!
        In general m_curvesArea is used to covert the curve coordinates to the plotting area coordinates.
        Meaning that the m_curvesArea has the same coordinates system as the curve itself.
        Like for instance the grid and axis of the curveArea are fiiting the curve coordinates.
        But if the curve coordinates would be logarithmic etc., one will need an extra consversion.
        This can be done here.
    */
    virtual void Curve2World( double xcurve, double ycurve, double& xw, double& yw ) const;

    DECLARE_CLASS( a2dCurveObject )

protected:
    //! This is here so that this class cannot be used directly
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const = 0;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //! parent of curve
    a2dCurvesArea* m_curvesArea;

    //! Untransformed bounding box.
    a2dBoundingBox m_untransbbox;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCurveObject( const a2dCurveObject& other );
};

//! Base class for curves
/*! Use this as a base class for Curve classes.

    A Curve is defined as a function or set of points that can be plotted in X/Y plot.

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dCurve: public a2dCurveObject
{
public:

    //! these flags define how curves are drawn
    enum a2dCurveRenderFlags
    {
        a2dCURVES_RENDER_NO_POINTS = 0x0001,       /*!< show no points on the curve */
        a2dCURVES_RENDER_NORMAL    = 0x0002,       /*!< normal curve */
        a2dCURVES_RENDER_FILL_LINE = 0x0004,       /*!< curve filled with lines */
        a2dCURVES_RENDER_FILL      = 0x0008,       /*!< curve filled completely */
        a2dCURVES_RENDER_3D        = 0x0010,       /*!< 3d bars or 3d look */
        a2dCURVES_RENDER_DEFAULT   = a2dCURVES_RENDER_NORMAL /*!< like normal curve */
    };

    //! these flags define how (points on) curves can be moved/edited
    enum a2dCurveEditFlags
    {
        a2dCURVES_EDIT_NORMAL      = 0x0001,       /*!< points can be moved in any direction */
        a2dCURVES_EDIT_FIXED_X     = 0x0002,       /*!< points have fixed x coord. */
        a2dCURVES_EDIT_FIXED_Y     = 0x0004,       /*!< points have fixed y coord. */
        a2dCURVES_EDIT_ALLOW_ADD   = 0x0008,       /*!< allow adding new points to curve */
        a2dCURVES_EDIT_DEFAULT     = a2dCURVES_EDIT_NORMAL
    };

    a2dCurve();

    virtual ~a2dCurve();

    a2dCurve( const a2dCurve& other, CloneOptions options, a2dRefMap* refs );

    //! define which part of the curve will be displayed.
    void SetBoundaries( double xmin, double ymin, double xmax, double ymax );


    //! get clipped boundary box of plot boundary in curve coordinates
    /*!
        The highest of clipping box and curve box and curve group box is returned.
    */
    a2dBoundingBox GetClippedCurveBoundaries();

    //! get boundary box of plot boundary in curve coordinates
    /*!
    The highest of curve box is returned.
    */
    a2dBoundingBox GetCurveBoundaries();

#if 0
    //! get miminum X of plot boundary in curve coordinates
    /*!
        The highest of clipping and minx curve and minx curve group is returned.
    */
    virtual double GetBoundaryMinX();

    //! get maximum X of plot boundary in curve coordinates
    /*!
        The lowest of clipping and maxx curve and maxx curve group is returned.
    */
    virtual double GetBoundaryMaxX();

    //! get miminum Y of plot boundary in curve coordinates
    /*!
        The highest of clipping and miny curve and miny curve group is returned.
    */
    virtual double GetBoundaryMinY();

    //! get maximum Y of plot boundary in curve coordinates
    /*!
        The lowest of clipping and maxy curve and maxy curve group is returned.
    */
    virtual double GetBoundaryMaxY();
#endif

    //! return  Points in plot at the value of sweep parameter.
    /*!
        In general the sweep parameter is the x value in the plot data, and returned will be the x,y and that x.
        But in case of complex data x,y = Function( freq ), this here will return the x,y and freq.

        \return false when the sweep is outside of range.
    */
    virtual bool GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const = 0;

    a2dlist< a2dFill >& GetPointFillList() { return m_pointsFill; }

    void SetPointFillList( const a2dlist< a2dFill >& fillList ) { m_pointsFill = fillList; }

    bool IsHighlighted() const { return m_highlightmode == 1; }

    void SetRenderLogicalFunction( wxRasterOperationMode logFunction ) { m_logFunction = logFunction; SetPending( true );}

    wxRasterOperationMode GetRenderLogicalFunction() { return m_logFunction; }


    DECLARE_CLASS( a2dCurve )

protected:

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    int m_highlightmode;
    wxRasterOperationMode m_logFunction;

    //! defines boundaries in which the curve will be calculated / displayed
    a2dBoundingBox m_clipdata;

    //! defines boundaries in which the curve has points
    mutable a2dBoundingBox m_curveBox;

    a2dlist< a2dFill > m_pointsFill;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCurve( const a2dCurve& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dCurve>;
#endif

//! Object to plot a Curve defined by a Function in form of a string
/*!
The Function string will be evaluated for each X to get Y on the Curve

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dFunctionCurve: public a2dCurve
{
public:

    a2dFunctionCurve( const wxString& curve = _T( "Not Specified" ) );

    a2dFunctionCurve( const a2dFunctionCurve& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dFunctionCurve();

    void SetXstep( double xstep ) { m_xstep = xstep; SetPending( true ); }

    virtual bool GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const;

    DECLARE_DYNAMIC_CLASS( a2dFunctionCurve )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    wxString m_curve;

    double m_xstep;

    double m_xstart;

    double m_xstop;

private:

#if wxART2D_USE_CANEXTOBJ
    mutable a2dEval m_toeval;
#endif //wxART2D_USE_CANEXTOBJ

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dFunctionCurve( const a2dFunctionCurve& other );
};

//! Curve represented by a number of vertexes.
/*!

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dVertexCurve: public a2dCurve
{
    DECLARE_EVENT_TABLE()

public:

    a2dVertexCurve( a2dVertexArray* points );

    a2dVertexCurve( );

    a2dVertexCurve( const a2dVertexCurve& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dVertexCurve();

    //! calculate Y at X
    virtual bool GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const;

    //! return the array of points.
    a2dVertexArray* GetPoints() { return m_points; }

    //! return the number of point in the curve
    int  GetNumberOfPoints() { return m_points->size(); }

    //!modify point at index to x,y
    /*!
        Set the coordinates of the point at index to x,y.
        \param index the index of the point to change
        \param x new x for the point in curve coordinates
        \param y new y for the point in curve coordinates

        \remark afterinversion = true should be used if you have x,y relative to the polygon object,
        while afterinversion = false should be used when x,y are relative to the point in the polygon.
    */
    void SetPosXYPoint( int index, double x, double y );

    //! get point x and y at index
    /*!
        \param index the index of the point to change
        \param x new x for the point in curve coordinates
        \param y new y for the point in curve coordinates
    */
    void GetPosXYPoint( int index, double& x, double& y );

    //! get point x and y at index in world coordinates
    /*!
        \param index the index of the point to change
        \param x new x for the point in world coordinates
        \param y new y for the point in world coordinates
        \param transform if true return x y transformed with local matrix
    */
    void GetPosXYPointWorld( int index, double& x, double& y, bool transform );

    //!removes point at index
    /*!
        \param x x value of removed point in curve coordinates
        \param y y value of removed point in curve coordinates
        \param index index of point to remove (-1 means last point)
    */
    void RemovePoint( double& x, double& y , int index = -1 );

    //!removes point at index
    /*!
        \param x x value of removed point in world coordinates
        \param y y value of removed point in world coordinates
        \param index index of point to remove (-1 means last point)
        \param transformed if true returned values x,y are transformed with local matrix
    */
    void RemovePointWorld( double& x, double& y , int index = -1, bool transformed = true  );

    //! reduce matrix to identity without replacing object
    bool EliminateMatrix();

    //! set to true, the polygon will be drawn as a spline
    void SetSpline( bool on ) { m_spline = on; SetPending( true ); }

    //! Get the polygon spline setting
    bool GetSpline() { return m_spline; }

    //!Append a new point in curve coordinates
    void AddPoint( const a2dPoint2D& P, int index = -1 );

    //!Append a new point in curve coordinates
    void AddPoint( double x, double y, int index = -1 );

    //!Preallocates memory for a given number of array elements.
    /*!
     It is worth calling when the number of items which are going to be added to the array
     is known in advance because it will save unneeded memory reallocation.
     If the array already has enough memory for the given number of items, nothing happens.
    */
    void Alloc( size_t count ) { m_points->reserve( count ); }

    //!This function does the same as Empty() and additionally frees the memory allocated to the array.
    void Clear() { m_points->clear(); }

    //!Returns true if the array is empty, false otherwise.
    bool IsEmpty() const  { return m_points->empty(); }

    //! get i'th point
    a2dLineSegmentPtr operator[]( size_t index ) const { return m_points->operator[]( index ); }

    //!Returns the point pointer at the given position in the array.
    /*!
        If index is out of bounds, an assert failure is raised in
        the debug builds but nothing special is done in the release build.
    */
    a2dLineSegmentPtr Item( size_t index ) const { return m_points->operator[]( index ); }

    //!Returns the last element in the array.
    /*!
        I.e. is the same as Item(GetCount() - 1).
        An assert failure is raised in the debug mode if the array is empty.
    */
    a2dLineSegmentPtr Last() const { return m_points->back(); }

    //!Removes a point from the array by index.
    /*!
        When a point is removed
        it is deleted - use Detach() if you don't want this to happen.
    */
    void RemoveAt( size_t index ) { m_points->RemoveAt( index ); }

    //! test if a point is on the curve.
    bool PointOnCurve( const a2dPoint2D& P, double marge );

    //! sets which parts or how the curve is rendered
    /*!
        See wxCurveRenderFlags for flags in the mask.
    */
    void SetRenderMode( wxUint32 rendermode ) { m_rendermode = rendermode; }

    //! get which parts or how the curve is rendered
    int GetRenderMode() { return m_rendermode; }

    //! sets how the curve is edited
    /*!
        See wxCurveEditFlags for flags in the mask.
    */
    void SetEditMode( wxUint32 editmode ) { m_editmode = editmode; }

    //! get how the curve is edited
    int GetEditMode() { return m_editmode; }

    DECLARE_DYNAMIC_CLASS( a2dVertexCurve )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnHandleEvent( a2dHandleMouseEvent& event );

    void OnEnterObject( a2dCanvasObjectMouseEvent& event );
    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    bool m_spline;

    wxUint32 m_rendermode, m_editmode;

    a2dVertexArray* m_points;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dVertexCurve( const a2dVertexCurve& other );
};

//! old a complex point at a sweep value
/*!
    e.g. x,y = function( sweep )
*/
class A2DCURVESDLLEXP a2dSweepPoint
{
public:
    a2dSweepPoint( double sweep, double a, double b );

    a2dPoint2D GetPoint() { return a2dPoint2D( m_a, m_b ); }
    a2dPoint2D GetPointUp() { return a2dPoint2D( m_sweep, m_a ); }
    a2dPoint2D GetPointLow() { return a2dPoint2D( m_sweep, m_b ); }
    double m_sweep;
    double m_a;
    double m_b;
};

WX_DECLARE_OBJARRAY_WITH_DECL( a2dSweepPoint, a2dSweepPointArray, class A2DCURVESDLLEXP );

//! Curve represented by a number of vertexes.
/*!

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dSweepCurve: public a2dCurve
{
    DECLARE_EVENT_TABLE()

public:

    a2dSweepCurve( a2dSweepPointArray* points );

    a2dSweepCurve( );

    a2dSweepCurve( const a2dSweepCurve& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dSweepCurve();

    //! calculate Y at X
    virtual bool GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const;

    //! return the array of points.
    a2dSweepPointArray* GetPoints() { return m_points; }

    //! return the number of point in the curve
    int  GetNumberOfPoints() { return m_points->GetCount(); }

    //!modify point at index to x,y
    /*!
        Set the coordinates of the point at index to x,y.
        \param index the index of the point to change
        \param x new x for the point in curve coordinates
        \param y new y for the point in curve coordinates

        \remark afterinversion = true should be used if you have x,y relative to the polygon object,
        while afterinversion = false should be used when x,y are relative to the point in the polygon.
    */
    void SetPosXYPoint( int index, double x, double y );

    //! get point x and y at index
    /*!
        \param index the index of the point to change
        \param x new x for the point in curve coordinates
        \param y new y for the point in curve coordinates
    */
    void GetPosXYPoint( int index, double& x, double& y );

    //! get point x and y at index in world coordinates
    /*!
        \param index the index of the point to change
        \param x new x for the point in world coordinates
        \param y new y for the point in world coordinates
        \param transform if true return x y transformed with local matrix
    */
    void GetPosXYPointWorld( int index, double& x, double& y, bool transform );

    //!removes point at index
    /*!
        \param x x value of removed point in curve coordinates
        \param y y value of removed point in curve coordinates
        \param index index of point to remove (-1 means last point)
    */
    void RemovePoint( double& x, double& y , int index = -1 );

    //!removes point at index
    /*!
        \param x x value of removed point in world coordinates
        \param y y value of removed point in world coordinates
        \param index index of point to remove (-1 means last point)
        \param transformed if true returned values x,y are transformed with local matrix
    */
    void RemovePointWorld( double& x, double& y , int index = -1, bool transformed = true  );

    //! reduce matrix to identity without replacing object
    bool EliminateMatrix();

    //! set to true, the polygon will be drawn as a spline
    void SetSpline( bool on ) { m_spline = on; SetPending( true ); }

    //! Get the polygon spline setting
    bool GetSpline() { return m_spline; }

    //!Append a new point in curve coordinates
    void AddPoint( double sweep, const a2dPoint2D& P );

    //!Append a new point in curve coordinates
    void AddPoint( double sweep, double x, double y );

    //!Preallocates memory for a given number of array elements.
    /*!
     It is worth calling when the number of items which are going to be added to the array
     is known in advance because it will save unneeded memory reallocation.
     If the array already has enough memory for the given number of items, nothing happens.
    */
    void Alloc( size_t count ) { m_points->Alloc( count ); }

    //!This function does the same as Empty() and additionally frees the memory allocated to the array.
    void Clear() { m_points->Clear(); }

    //! Removes the element from the array, but, unlike, Remove() doesn't delete it.
    //! The function returns the pointer to the removed element.
    a2dSweepPoint* Detach( size_t index ) { return m_points->Detach( index ); }

    //!Empties the array. For wxObjArray classes, this destroys all of the array elements.
    //!this function does not free the allocated memory, use Clear() for this.
    void Empty() { m_points->Empty(); }

    //!Insert a new point into the array before the point n - thus, Insert(something, 0u)
    //!will insert a point in such way that it will become the first array element.
    void Insert( a2dSweepPoint* item, size_t n )  { m_points->Insert( item, n ); }

    //!Returns true if the array is empty, false otherwise.
    bool IsEmpty() const  { return m_points->IsEmpty(); }

    //! get i'th point
    a2dSweepPoint* operator[]( size_t index ) const { return &( m_points->Item( index ) ); }

    //!Returns the point pointer at the given position in the array.
    //!If index is out of bounds, an assert failure is raised in
    //!the debug builds but nothing special is done in the release build.
    a2dSweepPoint* Item( size_t index ) const { return &( m_points->Item( index ) ); }

    //!Returns the last element in the array, i.e. is the same as
    //!Item(GetCount() - 1). An assert failure is raised in the debug mode if the array is empty.
    a2dSweepPoint* Last() const { return &( m_points->Last() ); }

    //!Removes a point from the array by index. When a point is removed
    //!it is deleted - use Detach() if you don't want this to happen.
    void RemoveAt( size_t index ) { m_points->RemoveAt( index ); }

    //!Frees all memory unused by the array.
    /*!If the program knows that no new items will be added to the
     array it may call Shrink() to reduce its memory usage. However, if a
     new item is added to the array, some extra memory will be allocated again.
    */
    void Shrink() { m_points->Shrink(); }

    //! test if a point is on the curve.
    bool PointOnCurve( const a2dPoint2D& P, double marge );

    //! sets which parts or how the curve is rendered
    /*!
        See wxCurveRenderFlags for flags in the mask.
    */
    void SetRenderMode( wxUint32 rendermode ) { m_rendermode = rendermode; }

    //! get which parts or how the curve is rendered
    int GetRenderMode() { return m_rendermode; }

    //! sets how the curve is edited
    /*!
        See wxCurveEditFlags for flags in the mask.
    */
    void SetEditMode( wxUint32 editmode ) { m_editmode = editmode; }

    //! get how the curve is edited
    int GetEditMode() { return m_editmode; }

    DECLARE_DYNAMIC_CLASS( a2dSweepCurve )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnHandleEvent( a2dHandleMouseEvent& event );

    void OnEnterObject( a2dCanvasObjectMouseEvent& event );
    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    bool m_spline;

    wxUint32 m_rendermode, m_editmode;

    a2dSweepPointArray* m_points;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dSweepCurve( const a2dSweepCurve& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dSweepCurve>;
#endif
//! Curve represented by a number of vertexes.
/*!

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dBandCurve: public a2dCurve
{
    DECLARE_EVENT_TABLE()

public:

    a2dBandCurve( a2dSweepPointArray* points );

    a2dBandCurve( );

    a2dBandCurve( const a2dBandCurve& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dBandCurve();

    //! calculate Y at X
    virtual bool GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const;

    //! return the array of points.
    a2dSweepPointArray* GetPoints() { return m_points; }

    //! return the number of point in the curve
    int  GetNumberOfPoints() { return m_points->GetCount(); }

    //! return of GetXyAtSweep() is upper side of curve if true, else low
    void SetMarkerUpLow( bool markerUpLow ) { m_markerUpLow = markerUpLow; }

    //!modify point at index to x,y
    /*!
        Set the coordinates of the point at index to x,y.
        \param index the index of the point to change
        \param sweep new sweep for the point in curve coordinates
        \param y new y for the point in curve coordinates

        \remark afterinversion = true should be used if you have x,y relative to the polygon object,
        while afterinversion = false should be used when x,y are relative to the point in the polygon.
    */
    void SetPosXYPoint( int index, double sweep, double y, bool upper );

    //! get point x and y at index
    /*!
        \param index the index of the point to change
        \param sweep new sweep for the point in curve coordinates
        \param y new y for the point in curve coordinates
    */
    void GetPosXYPoint( int index, double& sweep, double& y, bool upper );

    //! get point x and y at index in world coordinates
    /*!
        \param index the index of the point to change
        \param sweep new sweep for the point in world coordinates
        \param y new y for the point in world coordinates
        \param transform if true return x y transformed with local matrix
    */
    void GetPosXYPointWorld( int index, double& sweep, double& y, bool upper, bool transform );

    //!removes point at index
    /*!
        \param sweep sweep value of removed point in curve coordinates
        \param y y value of removed point in curve coordinates
        \param index index of point to remove (-1 means last point)
    */
    void RemovePoint( double& sweep, double& y , bool upper, int index = -1 );

    //!removes point at index
    /*!
        \param sweep sweep value of removed point in world coordinates
        \param y y value of removed point in world coordinates
        \param index index of point to remove (-1 means last point)
        \param transformed if true returned values x,y are transformed with local matrix
    */
    void RemovePointWorld( double& sweep, double& y , bool upper, int index = -1, bool transformed = true );

    //! reduce matrix to identity without replacing object
    bool EliminateMatrix();

    //! set to true, the polygon will be drawn as a spline
    void SetSpline( bool on ) { m_spline = on; SetPending( true ); }

    //! Get the polygon spline setting
    bool GetSpline() { return m_spline; }

    //!Append a new point in curve coordinates
    void AddPoint( double sweep, const a2dPoint2D& P );

    //!Append a new point in curve coordinates
    void AddPoint( double sweep, double x, double y );

    //!Preallocates memory for a given number of array elements.
    /*!
     It is worth calling when the number of items which are going to be added to the array
     is known in advance because it will save unneeded memory reallocation.
     If the array already has enough memory for the given number of items, nothing happens.
    */
    void Alloc( size_t count ) { m_points->Alloc( count ); }

    //!This function does the same as Empty() and additionally frees the memory allocated to the array.
    void Clear() { m_points->Clear(); }

    //! Removes the element from the array, but, unlike, Remove() doesn't delete it.
    //! The function returns the pointer to the removed element.
    a2dSweepPoint* Detach( size_t index ) { return m_points->Detach( index ); }

    //!Empties the array. For wxObjArray classes, this destroys all of the array elements.
    //!this function does not free the allocated memory, use Clear() for this.
    void Empty() { m_points->Empty(); }

    //!Insert a new point into the array before the point n - thus, Insert(something, 0u)
    //!will insert a point in such way that it will become the first array element.
    void Insert( a2dSweepPoint* item, size_t n )  { m_points->Insert( item, n ); }

    //!Returns true if the array is empty, false otherwise.
    bool IsEmpty() const  { return m_points->IsEmpty(); }

    //! get i'th point
    a2dSweepPoint* operator[]( size_t index ) const { return &( m_points->Item( index ) ); }

    //!Returns the point pointer at the given position in the array.
    //!If index is out of bounds, an assert failure is raised in
    //!the debug builds but nothing special is done in the release build.
    a2dSweepPoint* Item( size_t index ) const { return &( m_points->Item( index ) ); }

    //!Returns the last element in the array, i.e. is the same as
    //!Item(GetCount() - 1). An assert failure is raised in the debug mode if the array is empty.
    a2dSweepPoint* Last() const { return &( m_points->Last() ); }

    //!Removes a point from the array by index. When a point is removed
    //!it is deleted - use Detach() if you don't want this to happen.
    void RemoveAt( size_t index ) { m_points->RemoveAt( index ); }

    //!Frees all memory unused by the array.
    /*!If the program knows that no new items will be added to the
     array it may call Shrink() to reduce its memory usage. However, if a
     new item is added to the array, some extra memory will be allocated again.
    */
    void Shrink() { m_points->Shrink(); }

    //! test if a point is on the curve.
    bool PointOnCurve( const a2dPoint2D& P, double marge );

    //! sets which parts or how the curve is rendered
    /*!
        See wxCurveRenderFlags for flags in the mask.
    */
    void SetRenderMode( wxUint32 rendermode ) { m_rendermode = rendermode; }

    //! get which parts or how the curve is rendered
    int GetRenderMode() { return m_rendermode; }

    //! sets how the curve is edited
    /*!
        See wxCurveEditFlags for flags in the mask.
    */
    void SetEditMode( wxUint32 editmode ) { m_editmode = editmode; }

    //! get how the curve is edited
    int GetEditMode() { return m_editmode; }

    DECLARE_DYNAMIC_CLASS( a2dBandCurve )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void SyncHandlesWithLineSegments();

    void OnHandleEvent( a2dHandleMouseEvent& event );

    void OnEnterObject( a2dCanvasObjectMouseEvent& event );
    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    bool m_spline;

    wxUint32 m_rendermode, m_editmode;

    a2dSweepPointArray* m_points;

    //! return of GetXyAtSweep() is upper side of curve if true, else low
    bool m_markerUpLow;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dBandCurve( const a2dBandCurve& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dBandCurve>;
#endif

//! handle holds a pointer to a polygon/polyline segment
/*! this class is used for editing, to quickly update handles from vertexes.
*/
class A2DCURVESDLLEXP a2dCanvasBandCurveHandle: public a2dHandle
{
public:

    DECLARE_DYNAMIC_CLASS( a2dCanvasBandCurveHandle )

    //! constructor
    a2dCanvasBandCurveHandle();

    //! construct for use in a2dVertexArray
    /*!
        index based handle

        \param parent polygon that contains this handle
        \param index  index of segment for which this handle is generated.
        \param arcMid if true this handle is placed on the middle of an arc segment
        \param xc center x
        \param yc center y
        \param name name of handle
    */
    a2dCanvasBandCurveHandle( a2dBandCurve* parent, unsigned int index, bool arcMid, double xc, double yc, const wxString& name = wxT( "" ) );

    //! copy constructor
    a2dCanvasBandCurveHandle( const a2dCanvasBandCurveHandle& other, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dCanvasBandCurveHandle();

    //! handle for middle point of arc
    bool m_arcMid;

    unsigned int GetIndex() { return m_index; }

private:

    //! in case of vertex array
    unsigned int m_index;
};


// instantiate
#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dCanvasBandCurveHandle>;
#endif

//! Bars in a Curve represented by a number of vertexes.
/*!

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dBarCurve: public a2dVertexCurve
{

public:

    //! these flags define how curves are drawn
    enum a2dBarRenderFlags
    {
        a2dBAR_RENDER_NO_POINTS = 0x0001,       /*!< show no points on the curve */
        a2dBAR_RENDER_NORMAL    = 0x0002,       /*!< rectangular bar */
        a2dBAR_RENDER_3D        = 0x0004,       /*!< 3d bars or 3d look */
        a2dBAR_RENDER_LINE      = 0x0008,       /*!< just one line */
        a2dBAR_RENDER_DEFAULT   = a2dCURVES_RENDER_NORMAL /*!< like normal curve */
    };

    a2dBarCurve( a2dVertexArray* points );

    a2dBarCurve( );

    ~a2dBarCurve( );

    a2dBarCurve( const a2dBarCurve& other, CloneOptions options, a2dRefMap* refs );

    void SetBarWidth( double barwidth ) { m_barwidth = barwidth; }

    double GetBarWidth() { return m_barwidth; }

    double GetExtrudeDepth() { return m_depth; }
    double GetExtrudeAngle() { return wxRadToDeg( m_angle3d ); }

    void SetExtrudeDepth( double depth )  { m_depth = depth; SetPending( true ); }
    void SetExtrudeAngle( double angle3d )  { m_angle3d = wxDegToRad( angle3d ); SetPending( true ); }

    void SetExtrudeFill( const a2dFill& fill );

    const a2dFill& GetExtrudeFill() { return m_extrudeFill; }

    DECLARE_DYNAMIC_CLASS( a2dBarCurve )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    double m_barwidth;
    double m_depth;
    double m_angle3d;

    a2dFill m_extrudeFill;

    static const a2dPropertyIdRefObject PROPID_extrudeFill;


private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dBarCurve( const a2dBarCurve& other );
};

//! Curve represented by a number of vertexes in a List.
/*!
    Use this for time running curves, since vertexes can be added and removed
    at both sides quickly.

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dVertexListCurve: public a2dCurve
{
    DECLARE_EVENT_TABLE()

public:

    a2dVertexListCurve( a2dVertexList* points );

    a2dVertexListCurve( );

    a2dVertexListCurve( const a2dVertexListCurve& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dVertexListCurve();

    //! calculate Y at X
    virtual bool GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const;

    //! return the array of points.
    a2dVertexList* GetPoints() { return m_lpoints; }

    //! return the number of point in the curve
    int  GetNumberOfPoints() { return m_lpoints->size(); }

    //!Append a new point
    void AddPoint( const a2dPoint2D& P, int index = -1, bool afterinversion = true );

    //!Append a new point, which is owned by this class now
    void AddPoint( double x, double y, int index = -1, bool afterinversion = true );

    //!removes point at index
    /*!
        \param x x value of removed point
        \param y y value of removed point
        \param index index of point to remove (-1 means last point)
        \param transformed if true returned values x,y are transformed with local matrix
    */
    void RemovePoint( double& x, double& y , int index = -1, bool transformed = true  );

    //! reduce matrix to identity without replacing object
    bool EliminateMatrix();

    //!modify point at index to x,y
    /*!
        Set the coordinates of the point at index to x,y.
        \param index the index of the point to change
        \param x new x for the point
        \param y new y for the point
        \param afterinversion if true the input x,y or first inverted with the local matrix of the object.
        The inverted values are used to set the point.

        \remark afterinversion = true should be used if you have x,y relative to the polygon object,
        while afterinversion = false should be used when x,y are relative to the point in the polygon.
    */
    void SetPosXYPoint( int index, double x, double y, bool afterinversion = true );

    //! get point x and y at index
    /*!
        \param index the index of the point to change
        \param x new x for the point
        \param y new y for the point
        \param transform if true return x y transformed with local matrix
    */
    void GetPosXYPoint( int index, double& x, double& y, bool transform = true );

    //! set to true, the polygon will be drawn as a spline
    void SetSpline( bool on ) { m_spline = on; SetPending( true ); }

    //! Get the polygon spline setting
    bool GetSpline() { return m_spline; }

    //! test if a point is on the curve.
    bool PointOnCurve( const a2dPoint2D& P, double marge );

    //! sets which parts or how the curve is rendered
    /*!
        See wxCurveRenderFlags for flags in the mask.
    */
    void SetRenderMode( wxUint32 rendermode ) { m_rendermode = rendermode; }

    //! get which parts or how the curve is rendered
    int GetRenderMode() { return m_rendermode; }

    //! sets how the curve is edited
    /*!
        See wxCurveEditFlags for flags in the mask.
    */
    void SetEditMode( wxUint32 editmode ) { m_editmode = editmode; }

    //! get how the curve is edited
    int GetEditMode() { return m_editmode; }

    DECLARE_DYNAMIC_CLASS( a2dVertexListCurve )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnHandleEvent( a2dHandleMouseEvent& event );

    void OnEnterObject( a2dCanvasObjectMouseEvent& event );
    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    bool m_spline;

    int m_rendermode, m_editmode;

    a2dVertexList* m_lpoints;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dVertexListCurve( const a2dVertexListCurve& other );
};

//! Curve represented by a number of vertexes.
/*!

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dPieCurve: public a2dCurve
{
    DECLARE_EVENT_TABLE()

public:

    a2dPieCurve( a2dVertexArray* points );

    a2dPieCurve( );

    a2dPieCurve( const a2dPieCurve& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dPieCurve();

    //! calculate Y at X
    virtual bool GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const;

    //! return the array of points.
    a2dVertexArray* GetPoints() { return m_points; }

    //! return the number of point in the curve
    int  GetNumberOfPoints() { return m_points->size(); }

    //! get point x and y at index in world coordinates
    /*!
        \param index the index of the point to change
        \param x new x for the point in world coordinates
        \param y new y for the point in world coordinates
        \param transform if true return x y transformed with local matrix
    */
    void GetPosXYPointWorld( int index, double& x, double& y, bool transform );

    //!removes point at index
    /*!
        \param x x value of removed point in world coordinates
        \param y y value of removed point in world coordinates
        \param index index of point to remove (-1 means last point)
        \param transformed if true returned values x,y are transformed with local matrix
    */
    void RemovePointWorld( double& x, double& y , int index = -1, bool transformed = true  );


    //!Append a new point in curve coordinates
    void AddPoint( const a2dPoint2D& P, int index = -1 );

    //!Append a new point in curve coordinates
    void AddPoint( double x, double y, int index = -1 );

    //!Preallocates memory for a given number of array elements.
    /*!
     It is worth calling when the number of items which are going to be added to the array
     is known in advance because it will save unneeded memory reallocation.
     If the array already has enough memory for the given number of items, nothing happens.
    */
    void Alloc( size_t count ) { m_points->reserve( count ); }

    //!This function does the same as Empty() and additionally frees the memory allocated to the array.
    void Clear() { m_points->clear(); }

    //!Returns true if the array is empty, false otherwise.
    bool IsEmpty() const  { return m_points->empty(); }

    //!Returns the point pointer at the given position in the array.
    /*!
        If index is out of bounds, an assert failure is raised in
        the debug builds but nothing special is done in the release build.
    */
    a2dLineSegmentPtr Item( size_t index ) const { return m_points->operator[]( index ); }

    //!Returns the last element in the array.
    /*!
        I.e. is the same as Item(GetCount() - 1).
        An assert failure is raised in the debug mode if the array is empty.
    */
    a2dLineSegmentPtr Last() const { return m_points->back(); }

    //!Removes a point from the array by index.
    /*!
        When a point is removed
        it is deleted - use Detach() if you don't want this to happen.
    */
    void RemoveAt( size_t index ) { m_points->RemoveAt( index ); }

    //! sets which parts or how the curve is rendered
    /*!
        See wxCurveRenderFlags for flags in the mask.
    */
    void SetRenderMode( wxUint32 rendermode ) { m_rendermode = rendermode; }

    //! get which parts or how the curve is rendered
    int GetRenderMode() { return m_rendermode; }

    DECLARE_DYNAMIC_CLASS( a2dPieCurve )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnHandleEvent( a2dHandleMouseEvent& event );

    void OnEnterObject( a2dCanvasObjectMouseEvent& event );
    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    wxUint32 m_rendermode;

    a2dVertexArray* m_points;

    double m_width;
    double m_height;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dPieCurve( const a2dPieCurve& other );
};

#endif

