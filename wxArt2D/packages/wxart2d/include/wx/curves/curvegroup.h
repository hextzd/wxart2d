/*! \file wx/curves/curvegroup.h
    \brief classes for plotting curve and pie data, and editing them.

    Data stored in a derived a2dCanvasObject, can be plotted as a curve of pie chart.
    One can add markers on the curves, and several curves can be plot as a group in one plot.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: curvegroup.h,v 1.27 2008/09/05 19:01:12 titato Exp $
*/

#ifndef __WXCURVEGROUP_H__
#define __WXCURVEGROUP_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/curves/meta.h"

WX_DECLARE_STRING_HASH_MAP_WITH_DECL( a2dBoundingBox, _a2dBoundingBoxHash, class A2DCURVESDLLEXP );

class A2DCURVESDLLEXP a2dBboxHash : public _a2dBoundingBoxHash, public a2dObject
{
public:
    a2dBboxHash( void );
    a2dBboxHash( const a2dBboxHash& other, CloneOptions options, a2dRefMap* refs );

protected:
#if wxART2D_USE_CVGIO
    //! Save settings.
    virtual void DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) ) {}
    //! Load settings.
    virtual void DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) ) {}
#endif
private:
    a2dBboxHash( const a2dBboxHash& other );


private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
};

//! base class for curve area in a a2dCurvesAreaList, like used in a2dCanvasXYDisplayGroups
/*!
    A curve area is a rectangular area in curve coordinates that is displayed in e.g. a a2dCanvasXYDisplayGroups.
    The curve is defined in curve coordinates, and a curve area holds several curves.
    The range of curve coordinates in X and Y is choosen here.

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dCurvesArea : public a2dPolygonLClipper
{
    friend class a2dCurvesAreaList;
    DECLARE_DYNAMIC_CLASS( a2dCurvesArea )
public:

    //! constructor
    /*!
        Construct a curve area, used to place a2dCurve objects.

        \param curvesAreaName name of the curve arae
    */
    a2dCurvesArea( const wxString curvesAreaName = wxT( "" ) );

    //! constructor for cloning
    a2dCurvesArea( const a2dCurvesArea& other, CloneOptions options, a2dRefMap* refs );

    ~a2dCurvesArea();

    //! get text to display for axis label
    wxString GetAxisText() const;

    //! set text to display for axis label
    bool SetAxisText( const wxString& name );

    //! size of object in world coordinates, as displayed in the drawing.
    void SetBoundaries( const a2dBoundingBox& extbox );

    bool IsRecursive() { return true; }

    //! update conversion matrix to/from curve to world coordinates
    /*
        Based on the given values, the mapping of world coordinates to the plot its
        internal curve coordinates is calculated.
    */
    void UpdateInternalBoundaries( const a2dBoundingBox& extbox );

    //! set the internal curve boundaries, and update conversion to world matrix
    void SetInternalBoundaries( const a2dBoundingBox& extbox, const a2dBoundingBox& intbox )
    {
        m_intrect = intbox;
        UpdateInternalBoundaries( extbox );
    }

    //! set the internal curve boundaries, and update conversion to world matrix
    void SetInternalBoundaries( const a2dBoundingBox& extbox, double ximin, double yimin, double ximax, double yimax )
    {
        SetInternalBoundaries( extbox, a2dBoundingBox( ximin, yimin, ximax, yimax ) );
    }

    //! get the plotting rectangle in curve coordinates
    const a2dBoundingBox& GetInternalBoundaries() const { return m_intrect; }

    //! get the plotting matrix in curve coordinates as matrix.
    //! The matrix is used to plot curve in m_intrect
    const a2dAffineMatrix& GetCurveAreaTransform() { return m_iworld; }

    //! add a curve to the end of the exisiting curves.
    /*
    \param curve pointer to a curve object
    \param curvename if not empty, the curve will be named this.
    */
    void AddCurve( a2dCurve* curve, const wxString curvename = _T( "" ) );

    //! add a marker
    /*
    \param marker pointer to a marker object
    */
    void AddMarker( a2dMarker* marker );

    //! remove a marker
    /*
    \param marker pointer to a marker object
    */
    void RemoveMarker( a2dMarker* marker );

    //! add a curve to the end of the existing curves.
    /*
    \param before place this curve before the given index of another curve
    \param curve pointer to a curve object
    \param curvename if not empty, the curve will be named this.
    */
    void InsertCurve( size_t before, a2dCurve* curve, const wxString curvename = _T( "" ) );

    //! return curve named curvename, if not available returns NULL
    a2dCurve* GetCurve( const wxString curvename );

    //! returns the list where the curves are stored in.
    /*!
    Can be used to iterate over the curves, and/or add curves etc.
    */
    const a2dCanvasObjectList* GetCurves() const
    {
        return GetChildObjectList();
    }
    //! returns the list where the curves are stored in.
    /*!
    Can be used to iterate over the curves, and/or add curves etc.
    */
    a2dCanvasObjectList* GetCurves()
    {
        return GetChildObjectList();
    }

    //! used by a2dCurve to transform its own curve coordinates to the curve plotting area
    void World2Curve( double xw, double yw, double& xcurve, double& ycurve );

    //! used by a2dCurve to transform its own curve coordinates to the curve plotting area
    void Curve2World( double xcurve, double ycurve, double& xw, double& yw );

    //! get the boundingbox in internal curve coordinates of all the curves
    a2dBoundingBox GetCurvesBoundaries();

    //! get Y axis of this curve area
    a2dCurveAxis* GetAxisY() { return m_axisY; }

    //! set Y axis of this curve area
    void SetAxisY( a2dCurveAxis* axisY ) { m_axisY = axisY; }

    //! set show yaxis
    void SetShowYaxis( bool showyaxis );
    //! get show yaxis
    bool GetShowYaxis( void ) { return m_showyaxis; }

    //! sets m_axisY values
    void SetAxes( const a2dBoundingBox& extbox, double x, double y, double& zerox, double& zeroy, double& ixmin, double& ixmax );

    //!if a curve in this area is set, true returned
    bool IsCurvesHighlighted() const;

    void SetMarkerShow( a2dMarkerShow* showm );
    void SetMarkerShow2( a2dMarkerShow* showm );
    a2dMarkerShow* GetMarkerShow( void ) const { return m_markerShow; }
    a2dMarkerShow* GetMarkerShow2( void ) const { return m_markerShow2; }

    //! set color of axis text
    void SetColor( const wxColour& color ) { m_colour = color;   }
    //! get color of axis text
    const wxColour& GetColor() const { return m_colour;}

    //! search a marker ( placed to show a position on a curve )
    /*!
        A a2dMarker is added as a child object on a a2dCurveArea.
    */
    a2dMarker* GetCursorMarker() const;

    //! the group of which the area is a part
    a2dCanvasXYDisplayGroupAreas* GetGroup( void ) { return m_group; }
    //! the group of which the area is a part
    void SetGroup( a2dCanvasXYDisplayGroupAreas* aGroup );

    a2dBoundingBox& Expand( a2dBoundingBox& bbox );
    void DoAddPending( a2dIterC& ic );
    bool ProcessCanvasObjectEvent( a2dIterC& ic, a2dHitEvent& hitEvent );
    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );
    a2dCanvasObject* IsHitCurvesWorld ( a2dIterC& ic, a2dHitEvent& hitEvent );
    void DependencyPending( a2dWalkerIOHandler* handler );
    bool Update( UpdateMode mode );

protected:
   
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    void DoUpdateViewDependentObjects( a2dIterC& ic );
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );
#if wxART2D_USE_CVGIO
    //! Save settings.
    virtual void DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) );
    //! Load settings.
    virtual void DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) ) ;
#endif

    //! if true show the Y axis object
    bool m_showyaxis;

    //! y axis object
    a2dSmrtPtr<a2dCurveAxis> m_axisY;

    //internal coordinates defined as rectangle
    a2dBoundingBox m_intrect;
    //! internal coordinates used for curve plotting expressed as matrix
    a2dAffineMatrix m_iworld;

    a2dSmrtPtr<a2dMarkerShow> m_markerShow;
    a2dSmrtPtr<a2dMarkerShow> m_markerShow2;

    //! color of axis text
    wxColour m_colour;

    //! the group of which the area is a part
    a2dCanvasXYDisplayGroupAreas* m_group;

public:
    static a2dPropertyIdCanvasObject* PROPID_curvesarea;
    static a2dPropertyIdString* PROPID_text;

    DECLARE_PROPERTIES()

};

template class A2DCURVESDLLEXP a2dSmrtPtr<a2dCurvesArea>;

#include <wx/dynarray.h>
WX_DECLARE_OBJARRAY_WITH_DECL( a2dSmrtPtr<a2dCurvesArea>, a2dCurvesAreaListBase, class A2DCURVESDLLEXP );

//! list for a2dCurvesArea objects
/*!
   A list of curve areas, one of them can be set as Yaxis Left and as Yaxis Right.
*/
class A2DCURVESDLLEXP a2dCurvesAreaList : public a2dCurvesAreaListBase, public a2dObject
{
    DECLARE_DYNAMIC_CLASS( a2dCurvesAreaList )
public:
    a2dCurvesAreaList();

    a2dCurvesAreaList( const a2dCurvesAreaList& other, a2dObject::CloneOptions options, a2dRefMap* refs );

    ~a2dCurvesAreaList();

    //  a2dCurvesAreaList& operator=( const a2dCurvesAreaList &other);

    //! get a2dCurvesArea with the given name
    a2dCurvesArea* GetCurvesArea( const wxString& curvesAreaName );
    //! get a2dCurvesArea with the given name
    a2dCurvesArea* GetCurvesArea( const wxString& curvesAreaName ) const;

    //! this object is recursive (has children ).
    bool IsRecursive() { return true; }

    //! sets for all curve areas in the list the same boundaries
    void SetBoundaries( const a2dBoundingBox& extbox );

    //Each a2dCurvesArea is shown in an X range and Y range as defined by SetBoundaries.
    void SetClippingFromBox( a2dBoundingBox& bbox = wxNonValidBbox );


    //! fills a2dBboxHash with rectangles representing the plotting area of a curve
    int AppendInternalBoundaries( a2dBboxHash* irectHash );
    //! fills a2dBboxHash with rectangles representing the plotting area of a curve
    int AppendCurvesBoundaries( a2dBboxHash* irectHash );

    a2dBoundingBox& Expand( a2dBoundingBox& bbox ) const;

    void AddPending( a2dIterC& ic );

    bool Update( a2dCanvasObject::UpdateMode mode );

    //! sets m_axisY values
    void SetAxes( const a2dBoundingBox& extbox, double x, double y, double& zerox, double& zeroy, double& ixmin, double& ixmax );

    //! tic distance in Y for the m_leftAxisY area
    double GetTicY() const
    {
        wxASSERT_MSG( m_leftAxisY != ( a2dCurvesArea* )NULL, wxT( "left axis was not defined" ) );
        return m_leftAxisY->m_axisY->GetTic();
    }

    //! the a2dCurveArea which has bin set to be the m_leftAxisY
    /*!
        This area is called the base, and is used for drawing the grid and cursor lines etc.
        The base area is created by a2dCurvesAreaList at construction.
    */
    a2dSmrtPtr<a2dCurvesArea> GetBaseCurvesArea() const
    {
        wxASSERT_MSG( m_leftAxisY != ( a2dCurvesArea* )NULL, wxT( "left axis was not defined" ) );
        return m_leftAxisY;
    }

    //! render all Yaxis objects ( of Left and Right a2dCurveArea )
    void RenderAxesY ( a2dIterC& ic, OVERLAP clipparent );

    void Render ( a2dIterC& ic, OVERLAP clipparent );

    //! the a2dCurveArea with the given name is set as holding the left Yaxis
    void SetShowLeftYaxis( const wxString& curveAreaName, bool showyaxis );

    //! the a2dCurveArea with the given name is set as holding the rigth Yaxis
    void SetShowRightYaxis( const wxString& curveAreaName, bool showyaxis );

    void SetTicY( int numLines );

//  a2dBoundingBox SetMarkersPos(const a2dBoundingBox& bbox);
//  virtual void ChangeCursorPos(double shift);
    void SetCursor( a2dCursor* cursor );

    void DependencyPending( a2dWalkerIOHandler* handler );
    bool GetPending( void );
    void UpdateViewDependentObjects( a2dIterC& ic );

    //! selected object itself or one of its recursive children
    bool GetHasSelectedObjectsBelow();
    bool GetHasToolObjectsBelow();

    a2dCanvasXYDisplayGroupAreas* GetGroup( void ) { return m_group; }

    void SetGroup( a2dCanvasXYDisplayGroupAreas* aGroup );

    void SetAutoShowYAxes( bool autoshowyaxes ) { m_autoshowyaxes = autoshowyaxes; }

    bool GetAutoShowYAxes( void ) { return m_autoshowyaxes; }

    bool ProcessCanvasObjectEvent( a2dIterC& ic, a2dHitEvent& hitEvent );
    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );
    a2dCanvasObject* IsHitCurvesWorld ( a2dCurvesArea* &area, a2dIterC& ic, a2dHitEvent& hitEvent );

    //! the curve area to display the left Y axis
    a2dSmrtPtr<a2dCurvesArea> m_leftAxisY;
    //! the curve area to display the right Y axis
    a2dSmrtPtr<a2dCurvesArea> m_rightAxisY;

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    //! Save settings.
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite );
    //! Load settings.
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts ) ;
#endif

    bool m_autoshowyaxes;

    //! the group of which this object is a part
    a2dCanvasXYDisplayGroupAreas* m_group;
};

template class A2DCURVESDLLEXP a2dSmrtPtr<a2dCurvesAreaList>;

WX_DECLARE_STRING_HASH_MAP( a2dBoundingBox, a2dCurvesBboxes );

//! Special object group to plot a2dCurve Objects but also any other a2dCanvasObject.
/*!
    The class has special functions to make it easy to plot curves on a grid with tics etc.

    There is a list a2dCurvesAreaList with a2dCurvesArea objects, where each can contain a set of curves.
    Each a2dCurvesArea has his own drawing coordinates for the curve that are drawn on it.

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dCanvasXYDisplayGroupAreas: public a2dCanvasObject
{
public:

    //!construct a new a2dCanvasXYDisplayGroup at given position
    /*!
    \param x X position
    \param y Y position
    */
    a2dCanvasXYDisplayGroupAreas( double x = 0 , double y = 0 );

    //! constructor to clone
    a2dCanvasXYDisplayGroupAreas( const a2dCanvasXYDisplayGroupAreas& other, CloneOptions options, a2dRefMap* refs );

    //! return reference to list of all a2dCurveArea objects used
    const a2dCurvesAreaList& GetCurvesAreaList() const { return *m_axesarealist;}

    //! set list of all a2dCurveArea objects
    void SetCurvesAreaList( a2dCurvesAreaList* areaList )
    {
        m_axesarealist = areaList;
        m_axesarealist->SetGroup( this );
    }

    //! set the minimum and maximum values
    /*!
        this defines the size of the plot in world coordinates.
        The position is in the transformMatrix of the object, this here
        defines how it is placed relative to that matrix.

        \remark the data plotted in this are from curves etc., is set by SetInternalBoundaries()
    */
    void SetBoundaries( const a2dBoundingBox& extbox );
    //! see  SetBoundaries(const a2dBoundingBox& extbox)
    void SetBoundaries( double xmin, double ymin, double xmax, double ymax )
    {
        SetBoundaries( a2dBoundingBox( xmin, ymin, xmax, ymax ) );
    }

    //! set the minimum and maximum values to be displayed in the axisarea
    /*
        Based on the given values, the mapping of world coordinates to the plot its
        internal coordinates is calculated and visa versa.
        For instance the curves in the plot use this data to convert its data and plot itself.

        \param ximin minimum x curve coordinate
        \param yimin minimum y curve coordinate
        \param ximax maximum x curve coordinate
        \param yimax maximum y curve coordinate
        \param curveAreaName name of a2dCurveArea to set, if empty use the only
    */
    void SetInternalBoundaries( double ximin, double yimin, double ximax, double yimax, const wxString& curveAreaName = wxEmptyString );

    //! set the minimum and maximum values for axis X for all axisareas to be displayed
    /*
        changed X-axis bounds for all areas
    */
    void SetInternalXBoundaries( double ximin, double ximax );

    //! return curve coordinate boundaries of a given curve area
    a2dBoundingBox GetInternalBoundaries( const wxString& curveAreaName = wxEmptyString ) const;

    //! return plotting rectangle in world coordinates
    a2dBoundingBox GetPlotAreaBbox() const;

    //! fills a2dBboxHash with rectangles representing the plotting area of a curve
    int AppendInternalBoundaries( a2dBboxHash* irectHash );
    //! fills a2dBboxHash with rectangles representing the plotting area of a curve
    int AppendCurvesBoundaries( a2dBboxHash* irectHash );

    a2dAffineMatrix  GetCurvesAreaTransform( const wxString& curveAreaName );

    a2dCurvesArea* GetCurvesArea( const wxString& curveAreaName = wxEmptyString )
    { return m_axesarealist->GetCurvesArea( curveAreaName ); }

    //! add a2dCurveArea with this name, if not already available
    a2dCurvesArea* AddCurvesArea( const wxString& curveAreaName );

    //! add a curve to the end of the existing curves in a given a2dCurveArea
    /*
        \param curveAreaName name of the curve area to add the curve object
        \param curve pointer to a curve object
        \param curvename if not empty, the curve will be named this.
    */
    void AddCurveToArea( const wxString& curveAreaName, a2dCurve* curve, const wxString curvename = _T( "" ) );

    //! add a marker
    /*
        \param curveAreaName name of the curve area to add the curve object
        \param marker pointer to a marker object
    */
    void AddMarkerToArea( const wxString& curveAreaName, a2dMarker* marker );

    //! remove a marker
    /*
        \param curveAreaName name of the curve area to add the curve object
        \param marker pointer to a marker object
    */
    void RemoveMarkerFromArea( const wxString& curveAreaName, a2dMarker* marker );

    void SetCursor( a2dCursor* cursor );

    a2dCursor* GetCursor()
    {
        return m_cursor;
    }

    void ChangeCursorPos( double step );


    //! add a curve to the end of the exisiting curves.
    /*
        \param curveAreaName name of the curve area to add the curve object
        \param before place this curve before the given index of another curve
        \param curve pointer to a curve object
        \param curvename if not empty, the curve will be named this.
    */
    void InsertCurveToArea( const wxString& curveAreaName, size_t before, a2dCurve* curve, const wxString curvename = _T( "" ) );

    //! return curve named curvename, if not available returns NULL
    /*
        \param curveAreaName name of the curve area to add the curve object
        \param curvename the curve to search
    */
    a2dCurve* GetCurveFromArea( const wxString& curveAreaName, const wxString curvename );

    //! returns the list where the curves are stored in.
    /*!
        Can be used to iterate over the curves, and/or add curves etc.

        \param curveAreaName name of the curve area to add the curve object
    */
    a2dCanvasObjectList* GetCurvesFromArea( const wxString& curveAreaName );

    //! get the boundingbox in relative world coordinates of the plot area
    /*!
        This is the area in world coordinates that is used to plot the data.
        This is different from the total boundingbox, since that contains tics and text etc.
    */
    const a2dBoundingBox& GetPlotAreaRect() const { return m_plotrect; }

    //! get the boundingbox in internal curve coordinates of all the curves in a given area
    /*!
        \param curveAreaName name of the curve area to add the curve object
    */
    a2dBoundingBox GetCurvesBoundariesFromArea( const wxString& curveAreaName );

    //!destructor
    virtual ~a2dCanvasXYDisplayGroupAreas();

    //! get X axis object
    a2dCurveAxis* GetAxisX() const { return m_axisX; }

    //! set X axis object
    void SetAxisX( a2dCurveAxis* axisX ) { m_axisX = axisX; SetPending( true ); }

    //! get Y axis object for given curve area
    a2dCurveAxis* GetAreaAxisY( const wxString& curveAreaName ) const;

    //! show xaxis
    void SetShowXaxis( bool showxaxis ) { m_showxaxis = showxaxis; SetPending( true ); }

    //! show yaxis
    void SetShowYaxis( const wxString& curveAreaName, bool showyaxis )
    {
        SetShowLeftYaxis( curveAreaName, showyaxis );
    }

    //! set Y axis object for given curve area
    void SetAreaAxisY( const wxString& curveAreaName, a2dCurveAxis* axisY );

    //! show yaxis
    void SetShowLeftYaxis( const wxString& curveAreaName, bool showyaxis );
    //! show yaxis
    void SetShowRightYaxis( const wxString& curveAreaName, bool showyaxis );

    //! set
    void SetAutoTicYPrecision( bool autoPrecision ) { m_autoTicYPrecision = autoPrecision; SetPending( true );}

    //! get
    bool IsAutoTicYPrecision() const { return m_autoTicYPrecision; }

    //! show grid
    void SetShowGrid( bool showgrid ) { m_showgrid = showgrid; SetPending( true ); }

    //!Set a stroke for grid.
    void SetGridStroke( const a2dStroke& stroke );

    //!Set a fill for axis area
    void SetAxisAreaFill( const a2dFill& fill );

    //!Set a stroke around axis area
    void SetAxisAreaStroke( const a2dStroke& fill );

    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    bool ProcessCanvasObjectEvent( a2dIterC& ic, a2dHitEvent& hitEvent );

    //! if left axis a2dCurveArea is set, the label text will be set to this
    wxString GetLeftAxisText() const ;

    //! if right a2dCurveArea is set, the label text will be set to this
    wxString GetRightAxisText() const ;

    //! set axis text for given curve area
    /*!
        \param curveAreaName name of the curve area to add the curve object
        \param text text to placed at axis
        \param color color of the label
    */
    void SetAxisText( const wxString& curveAreaName, const wxString& text, const wxColour& color );

    void ClearCurvesAreas();

    void SetAreaMarkerShow( const wxString& curveAreaName, a2dMarkerShow* showm );
    void SetAreaMarkerShow2( const wxString& curveAreaName, a2dMarkerShow* showm );

    a2dMarkerShow* GetAreaMarkerShow( const wxString& curveAreaName ) const;
    a2dMarkerShow* GetAreaMarkerShow2( const wxString& curveAreaName ) const;

    void SetTicY( int numLines );

    virtual int SetAxisTicPrecision( a2dCurveAxis* axisY, double newMinY, double newMaxY );
    virtual void SetCommonTicPrecision( int precision );

    void SetAutoShowYAxes( bool autoshowyaxes ) { m_axesarealist->SetAutoShowYAxes( autoshowyaxes ); SetPending( true );}

    void SetCursorOnlyByCurves( bool cursorOnlyByCurves ) { m_cursorOnlyByCurves = cursorOnlyByCurves; SetPending( true );}
    bool GetCursorOnlyByCurves( void ) { return m_cursorOnlyByCurves; }

    void OnChar( wxKeyEvent& event );

    DECLARE_DYNAMIC_CLASS( a2dCanvasXYDisplayGroupAreas )

    DECLARE_EVENT_TABLE()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    virtual void DrawGrid( a2dIterC& ic );

    void DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) );

    void DoUpdateViewDependentObjects( a2dIterC& ic );

    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    void DoAddPending( a2dIterC& ic );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool m_showgrid;
    bool m_showxaxis;
    bool m_autoTicYPrecision;

    a2dStroke m_strokegrid;
    a2dFill   m_fillAxisArea;
    a2dStroke m_strokeAxisArea;

    //!placement in world coordinates
    a2dBoundingBox m_plotrect;

    //! internal object used to display the curves in internal coordinates
    a2dSmrtPtr<a2dCurvesAreaList> m_axesarealist;

    //! x axis object
    a2dSmrtPtr<a2dCurveAxis> m_axisX;

    //! cursor object
    a2dSmrtPtr<a2dCursor> m_cursor;

    static const a2dPropertyIdRefObject PROPID_strokegrid;
    static const a2dPropertyIdRefObject PROPID_fillAxisArea;

//    static const a2dPropertyIdCanvasObject PROPID_axesarea;

    bool m_cursorOnlyByCurves;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCanvasXYDisplayGroupAreas( const a2dCanvasXYDisplayGroupAreas& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dCanvasXYDisplayGroupAreas>;
#endif

//! simple curve group with just one a2dCurvesArea in use
class A2DCURVESDLLEXP a2dCanvasXYDisplayGroup: public a2dCanvasXYDisplayGroupAreas
{
public:

    //!construct a new a2dCanvasXYDisplayGroup at given position
    /*!
    \param x X position
    \param y Y position
    */
    a2dCanvasXYDisplayGroup( double x = 0 , double y = 0 );

    a2dCanvasXYDisplayGroup( const a2dCanvasXYDisplayGroup& other, CloneOptions options, a2dRefMap* refs );

    //! add a curve to the end of the exisiting curves.
    /*
    \param curve pointer to a curve object
    \param curvename if not empty, the curve will be named this.
    */
    void AddCurve( a2dCurve* curve, const wxString curvename = _T( "" ) )
    {
        AddCurveToArea( ( const wxString& )wxEmptyString, curve, curvename );
    }

    //! add a marker
    /*
    \param marker pointer to a marker object
    */
    void AddMarker( a2dMarker* marker )
    {
        AddMarkerToArea( ( const wxString& )wxEmptyString, marker );
    }

    //! remove a marker
    /*
    \param marker pointer to a marker object
    */
    void RemoveMarker( a2dMarker* marker )
    {
        RemoveMarkerFromArea( ( const wxString& )wxEmptyString, marker );
    }

    //! add a curve to the end of the exisiting curves.
    /*
    \param before place this curve before the given index of another curve
    \param curve pointer to a curve object
    \param curvename if not empty, the curve will be named this.
    */
    void InsertCurve( size_t before, a2dCurve* curve, const wxString curvename = _T( "" ) )
    {
        InsertCurveToArea( ( const wxString& )wxEmptyString, before, curve, curvename );
    }

    //! return curve named curvename, if not available returns NULL
    a2dCurve* GetCurve( const wxString curvename )
    {
        return GetCurveFromArea( ( const wxString& )wxEmptyString, curvename );
    }

    //! returns the list where the curves are stored in.
    /*!
    Can be used to iterate over the curves, and/or add curves etc.
    */
    a2dCanvasObjectList* GetCurves( void )
    {
        return GetCurvesFromArea( ( const wxString& )wxEmptyString );
    }


    //! get the boundingbox in internal curve coordinates of all the curves
    a2dBoundingBox GetCurvesBbox( void )
    {
        return GetCurvesBoundariesFromArea( ( const wxString& )wxEmptyString );
    }

    //!destructor
    virtual ~a2dCanvasXYDisplayGroup();

    a2dCurveAxis* GetAxisY( void )
    {
        return GetAreaAxisY( ( const wxString& )wxEmptyString );
    }

    void SetAxisY( a2dCurveAxis* axisY )
    {
        SetAreaAxisY( ( const wxString& )wxEmptyString, axisY );
    }

    //! show yaxis
    void SetShowYaxis( bool showyaxis )
    {
        SetShowLeftYaxis( wxEmptyString, showyaxis );
    }

    DECLARE_DYNAMIC_CLASS( a2dCanvasXYDisplayGroup )

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
//  virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

//  void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCanvasXYDisplayGroup( const a2dCanvasXYDisplayGroup& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dCanvasXYDisplayGroup>;
#endif

extern A2DCURVESDLLEXP double a2dBoundsRound( double val );
extern A2DCURVESDLLEXP double a2dTicsRound( double val );

#endif

