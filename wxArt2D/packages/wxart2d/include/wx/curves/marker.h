/*! \file wx/curves/marker.h
    \brief classes for plotting curve and pie data, and editing them.

    Data stored in a derived a2dCanvasObject, can be plotted as a curve of pie chart.
    One can add markers on the curves, and several curves can be plot as a group in one plot.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: marker.h,v 1.17 2008/09/05 19:01:12 titato Exp $
*/

#ifndef __WXMARKER_H__
#define __WXMARKER_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/curves/meta.h"

//! For Markers on a Plot
/*!
Markers can be dragged along the curve

\ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dBaseMarker: public a2dCurveObject
{
public:
    // !!!! The cast in the initialization is bad
    a2dBaseMarker( const wxString& format = _T( "%f" ) );

    a2dBaseMarker( const wxString& format, double atx, double height = 0 );

    a2dBaseMarker( const wxString& format, double atx,  const a2dFont& font );

    a2dBaseMarker( const a2dBaseMarker& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dBaseMarker();

    //!height of text at marker according to format string
    //! \remark 0 height means no text displayed.
    void SetTextHeight ( double height ) { m_textheight = height; SetPending( true ); }

    /*! set all parameters
    \param xt: x top of arrow (normal 0)
    \param yt: y top of arrow (normal 0)
    \param l1: How long is the arrow.
    \param l2: Distance from top to the inside of the arrow.
    \param b: How broad is the basis of the arrow.
    */
    void Set( double xt, double yt, double l1, double l2, double b );

    /*! set parameters
    \param l1: How long is the arrow.
    \param l2: Distance from top to the inside of the arrow.
    \param b: How broad is the basis of the arrow.
    */
    void Set( double l1, double l2, double b );

    void    SetFont(  const a2dFont& font )
    {
        m_font = font; SetPending( true );
    }

    void SetStep( double step ) { m_step = step; SetPending( true ); }

    void SetPrompt( a2dText* prompttext );

    a2dSmrtPtr<a2dText> GetPrompt() const
    {
        return m_promptText;
    }

    void SetPromptFormat( const wxString& promptFromat )
    {
        m_promptFormat = promptFromat; SetPending( true );
    }
    virtual const wxString& GetMarkerString();

    double GetL1() const { return m_l1; }

    double GetL2() const { return m_l2; }

    double GetBase() const { return m_b; }

    bool GetSpline() const { return m_spline; }

    void SetPosition( double position ) { m_position = position;  SetPending( true ); }

    double GetPosition() const { return m_position; }

    DECLARE_DYNAMIC_CLASS( a2dBaseMarker )

    DECLARE_EVENT_TABLE()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void OnEnterObject( a2dCanvasObjectMouseEvent& event );
    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    wxString m_format;

    double m_textheight;

    //! length from top of arrow to outside points in X
    double m_l1;

    //! length from top of arrow to inside point in X
    double m_l2;

    //! base of arrow
    double m_b;

    //! draw splined?
    bool m_spline;

    //! marker font
    a2dFont m_font;

    //! step to move when dragging/moving marker
    double m_step;

    //! prompting when OnEnterObject
    bool m_prompting;

    //! the current X where the marker is located
    double m_position;

    a2dSmrtPtr<a2dText> m_promptText;
    wxString m_promptString;
    wxString m_promptFormat;

public:

    static a2dPropertyIdDouble* PROPID_PositionMarker;

    DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dBaseMarker( const a2dBaseMarker& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dBaseMarker>;
template class A2DCURVESDLLEXP std::allocator<class a2dSmrtPtr<class a2dBaseMarker> >;
template class A2DCURVESDLLEXP std::allocator< std::_List_nod<class a2dSmrtPtr<class a2dBaseMarker>, std::allocator<class a2dSmrtPtr<class a2dBaseMarker> > >::_Node >;
template class A2DCURVESDLLEXP std::allocator< std::_List_ptr<class a2dSmrtPtr<class a2dBaseMarker>, std::allocator<class a2dSmrtPtr<class a2dBaseMarker> > >::_Nodeptr >;
template class A2DCURVESDLLEXP std::list<class a2dSmrtPtr<class a2dBaseMarker> >;
template class A2DCURVESDLLEXP a2dlist<class a2dSmrtPtr<class a2dBaseMarker> >;
template class A2DCURVESDLLEXP a2dSmrtPtrList<a2dBaseMarker>;
#endif

//! For Markers on an Axis
/*!
Markers can draw self value on an axis

\ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dAxisMarker : public a2dBaseMarker
{
public:
    // !!!! The cast in the initialization is bad
    a2dAxisMarker( const wxString& format = _T( "%f" ) , a2dCurveAxis* axis = 0 );

    a2dAxisMarker( const wxString& format, a2dCurveAxis* curve, double atx, double height = 0 );

    a2dAxisMarker( const wxString& format, a2dCurveAxis* curve , double atx,  const a2dFont& font );

    a2dAxisMarker( const a2dAxisMarker& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dAxisMarker();

    void SetAxis( a2dCurveAxis* curve ) { m_axis = curve; SetPending( true ); }

    a2dCurveAxis* GetAxis() const { return m_axis; }

    virtual const wxString& GetMarkerString();

    DECLARE_DYNAMIC_CLASS( a2dAxisMarker )

    DECLARE_EVENT_TABLE()

public:
    static a2dPropertyIdCanvasObject* PROPID_axis;

    DECLARE_PROPERTIES()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    void DoAddPending( a2dIterC& ic );

    //void OnEnterObject(a2dCanvasObjectMouseEvent &event);

    //void OnLeaveObject(a2dCanvasObjectMouseEvent &event);

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    a2dSmrtPtr<a2dCurveAxis> m_axis;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dAxisMarker( const a2dAxisMarker& other );
};

//! For Markers on a Curve
/*!
    A marker is a shape which can be dragged along the curve.
    The marker has a pointer to the curve, but is placed one level higher in a a2dCurveArea.

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dMarker : public a2dBaseMarker
{
public:

    //! marker formatted
    /*!
        \param format printf format string for position of marker
        \param curve pointer to curve object to follow
    */
    a2dMarker( const wxString& format = _T( "%f %f" ) , a2dCurve* curve = 0 );

    //! marker formatted
    /*!
        \param format printf format string for position of marker
        \param curve pointer to curve object to follow
        \param atx x position on curve
        \param height text height for format string
    */
    a2dMarker( const wxString& format, a2dCurve* curve, double atx, double height = 0 );

    //! marker formatted
    /*!
        \param format printf format string for position of marker
        \param curve pointer to curve object to follow
        \param atx x position on curve
        \param font text font for format string
    */
    a2dMarker( const wxString& format, a2dCurve* curve , double atx,  const a2dFont& font );

    //! clone constructor
    a2dMarker( const a2dMarker& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dMarker();

    void CalculatePositionMarker( double& xw, double& yw );

    //! get point at position of marker.
    /*!
        If x,y on curve is a function of frequency, m_position stands for the frequency.
        What is returned, is the x,y at m_position in curve coordinates.
    */
    bool GetPositionCurvePoint( a2dPoint2D& point );

    void SetCurve( a2dCurve* curve ) { m_curve = curve; }

    //! return curve of the marker
    a2dCurve* GetCurve() const { return m_curve; }

    //! string for maker, as is rendered when entering marker with cursor.
    virtual const wxString& GetMarkerString();

    DECLARE_DYNAMIC_CLASS( a2dMarker )

    DECLARE_EVENT_TABLE()

public:
    static a2dPropertyIdCanvasObject* PROPID_curve;

    DECLARE_PROPERTIES()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void OnChar( wxKeyEvent& event );

    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    void DoAddPending( a2dIterC& ic );

    //void OnEnterObject(a2dCanvasObjectMouseEvent &event);

    //void OnLeaveObject(a2dCanvasObjectMouseEvent &event);

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    a2dSmrtPtr<a2dCurve> m_curve;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dMarker( const a2dMarker& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DCURVESDLLEXP a2dSmrtPtr<a2dMarker>;
#endif

//! marker for two value in one curve, called a a2dBandCurve.
/*!
    The marker travels up and down the band curve, and one can display
    the value on the upper or lower side of the curve.
*/
class A2DCURVESDLLEXP a2dBandMarker : public a2dMarker
{
public:
    // !!!! The cast in the initialization is bad
    a2dBandMarker( const wxString& format = _T( "%f" ) , a2dBandCurve* curve = 0 );

    a2dBandMarker( const wxString& format, a2dBandCurve* curve, double atx, double height = 0 );

    a2dBandMarker( const wxString& format, a2dBandCurve* curve , double atx,  const a2dFont& font );

    a2dBandMarker( const a2dBandMarker& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dBandMarker();

    //! marker on upper side of curve if true, else low
    void SetMarkerUpLow( bool markerUpLow ) { m_markerUpLow = markerUpLow; }

    void CalculatePositionMarker( double& xw, double& yw );

    //! get point at position of marker.
    /*!
        If x,y on curve is a function of frequency, m_position stands for the frequency.
        What is returned, is the x,y at m_position in curve coordinates.
    */
    bool GetPositionCurvePoint( a2dPoint2D& point );

    DECLARE_DYNAMIC_CLASS( a2dBandMarker )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

private:

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dBandMarker( const a2dBandMarker& other );

    bool m_markerUpLow;

    a2dSmrtPtr<a2dBandCurve> m_curveBand;
};


//! For Showing Marker position of Markers on a Curve
/*!
    Markers can be dragged along the curve, and the current value will be shown by
    this object

    \ingroup canvasobject meta curve
*/
class A2DCURVESDLLEXP a2dMarkerShow: public a2dCanvasObject
{
public:
    // !!!! The cast in the initialization is bad
    a2dMarkerShow( double x = 0, double y = 0,
                   const wxString& XLabel = _T( "X = " ), const wxString& YLabel = _T( "Y = " ),
                   const wxString& XValueformat = _T( "%f" ), const wxString& YValueformat = _T( "%f" ),
                   a2dMarker* marker = 0, double height = 0 );

    a2dMarkerShow( const a2dMarkerShow& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dMarkerShow();

    void SetMarker( a2dMarker* marker ) { m_marker = marker; }

    a2dMarker* GetMarker() const { return m_marker; }

    //!height of text used by format string
    void SetTextHeight ( double height ) { m_textheight = height; SetPending( true ); }

    a2dSmrtPtr<a2dText>& GetXLabel() { return m_Xlabel; }
    a2dSmrtPtr<a2dText>& GetYLabel() { return m_Ylabel; }
    a2dSmrtPtr<a2dText>& GetXText() { return m_xText; }
    a2dSmrtPtr<a2dText>& GetYText() { return m_yText; }

    DECLARE_DYNAMIC_CLASS( a2dMarkerShow )

    DECLARE_EVENT_TABLE()

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void OnPropertyChanged( a2dComEvent& event );

    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    void DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    wxString m_XValueformat;

    wxString m_YValueformat;

    double m_textheight;

    a2dSmrtPtr<a2dMarker> m_marker;

    a2dSmrtPtr<a2dText> m_xText;

    a2dSmrtPtr<a2dText> m_yText;

    a2dSmrtPtr<a2dText> m_Xlabel;

    a2dSmrtPtr<a2dText> m_Ylabel;

public:
    static a2dPropertyIdCanvasObject* PROPID_marker;
    static a2dPropertyIdCanvasObject* PROPID_xText;
    static a2dPropertyIdCanvasObject* PROPID_yText;
    static a2dPropertyIdCanvasObject* PROPID_Xlabel;
    static a2dPropertyIdCanvasObject* PROPID_Ylabel;

    DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dMarkerShow( const a2dMarkerShow& other );
};

//! List of markers
/*!
    Used in a2dCursor to store several markers on a vertical cursor line.
*/
class A2DCURVESDLLEXP a2dCursorMarkerList: public a2dSmrtPtrList<a2dBaseMarker>
{
public:
    a2dCursorMarkerList();

    ~a2dCursorMarkerList();

protected:

    a2dCursorMarkerList* DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const;

};

//! cursor on curve plot as a vertical line with markers
class A2DCURVESDLLEXP a2dCursor : public a2dCanvasObject
{
public:
    a2dCursor( double csize = 0. );

    a2dCursor( const a2dCursor& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dCursor();

    void SetHSize( double hsize )
    {
        if( m_hsize != hsize )
        {
            m_hsize = hsize;
            SetPending( true );
        }
    }
    void SetWSize( double wsize )
    {
        if( m_wsize != wsize )
        {
            m_wsize = wsize;
            SetPending( true );
        }
    }

    void SetCSize( double csize )
    {
        if( m_csize != csize )
        {
            m_csize = csize;
            SetPending( true );
        }
    }

    void AddMarker( a2dBaseMarker* marker );

    void RemoveMarker( a2dBaseMarker* marker );

    void RemoveMarkers();

    void SetPosition( double posx, double posy, const a2dAffineMatrix& mat );

    bool GetPosition( double& posx, double& posy );

    //! position is rounded to whole number if true
    void SetIntPosX( bool onlyIntPosX ) {m_onlyIntPosX = onlyIntPosX; }
    //! position is rounded to whole number
    void SetIntPosY( bool onlyIntPosY ) {m_onlyIntPosY = onlyIntPosY; }

    double ConvertAxis2World( double percentValue );

    bool CheckPosition( double posx );

    virtual void UpdatePosition( double minx, double miny, const a2dAffineMatrix& mat );

    virtual void SetPending( bool pending );

    DECLARE_DYNAMIC_CLASS( a2dCursor )

    DECLARE_EVENT_TABLE()

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON );
    void DoUpdateViewDependentTransform( a2dIterC& ic );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    double m_csize;
    double m_hsize;
    double m_wsize;
    double m_posx;
    double m_posy;

    //! position is rounded to whole number
    bool   m_onlyIntPosX;
    //! position is rounded to whole number
    bool   m_onlyIntPosY;

    //! list of markers on cursor line
    a2dCursorMarkerList m_markers;

    //! Untransformed bounding box.
    a2dBoundingBox m_untransbbox;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCursor( const a2dCursor& other );
};

typedef a2dSmrtPtr<a2dCursor> a2dCursorPtr;

class A2DCURVESDLLEXP a2dCursorAxisMarker : public a2dAxisMarker
{
public:
    // !!!! The cast in the initialization is bad
    a2dCursorAxisMarker( const wxString& format = _T( "%f" ) , a2dCurveAxis* axis = 0,  a2dCursor* cursor = 0 );

    a2dCursorAxisMarker( const wxString& format, a2dCurveAxis* curve, a2dCursor* cursor, double atx, double aty, double height = 0 );

    a2dCursorAxisMarker( const wxString& format, a2dCurveAxis* curve, a2dCursor* cursor, double atx, double aty,  const a2dFont& font );

    a2dCursorAxisMarker( const a2dCursorAxisMarker& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2dCursorAxisMarker();

    void SetCursor( a2dCursor* cursor ) { m_cursor = cursor; SetPending( true ); }

    a2dCursor* GetCursor() const { return m_cursor; }

    void SetPositionY( double posY ) { m_positionY = posY;}

    DECLARE_DYNAMIC_CLASS( a2dCursorAxisMarker )

    static a2dPropertyIdCanvasObject* PROPID_cursor;

    DECLARE_PROPERTIES()

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    double                m_positionY;
    a2dSmrtPtr<a2dCursor> m_cursor;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCursorAxisMarker( const a2dCursorAxisMarker& other );
};

#endif

