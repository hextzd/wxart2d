/*! \file wx/canvas/canprim.h
    \brief all basic primitives derived from a2dCanvasObject

    Circle rectangle arc line arrow etc. or placed in here.
    And Pins and pin classes to connect objects.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canprim.h,v 1.37 2009/07/17 16:03:34 titato Exp $
*/

#ifndef __WXCANPRIM_H__
#define __WXCANPRIM_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/canobj.h"

class A2DARTBASEDLLEXP a2dVpath;

//! a2dOrigin stays at World Coordinate Zero (0,0) not matter what.
/*! You can define width and Height of the cross.  \ingroup canvasobject */
class A2DCANVASDLLEXP a2dOrigin: public a2dCanvasObject
{
public:


    //! constructor
    a2dOrigin();

    //! constructor
    /*!
        \param w width in pixels of origin
        \param h height in pixels of origin
    */
    a2dOrigin( double w, double h );

    a2dOrigin( const a2dOrigin& other, CloneOptions options, a2dRefMap* refs );

    ~a2dOrigin();

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    //!return width
    double  GetWidth() { return m_width; }

    //!return height
    double  GetHeight() { return m_height; }

    //!set width
    /*!
        \param width width of origin
    */
    void SetWidth( double width ) { m_width = width; SetPending( true ); }

    //!set height
    /*!
        \param height height origin
    */
    void SetHeight( double height ) { m_height = height; SetPending( true ); }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dOrigin )

    // to temporarily disable rendering ( like when rendering from a2dCameleonInst )
    static void SetDoRender( bool doRender ) { m_doRender = doRender; }

    static double m_widthDefault;
    static double m_heightDefault;
    static bool m_doRender;

protected:
    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    //! width in pixels
    double m_width;

    //! height in pixels
    double m_height;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dOrigin( const a2dOrigin& other );
};

//! a2dRectC is a centered rectangle
/*!
    Rectangle around a center point.

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dRectC: public a2dCanvasObject
{
public:

    //! constructor
    a2dRectC();

    //! construct a centered rectangle
    /*!
        \param xc center x
        \param yc center y
        \param w  width of rectangle
        \param h  height of rectangle
        \param angle angle of rotation of rectangle
        \param radius for a rounded rectangle
    */
    a2dRectC( double xc, double yc, double w, double h, double angle = 0 , double radius = 0 );

    //! constructor
    a2dRectC( const a2dRectC& ori, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dRectC();

    //! set all paramters for the centered rectangle
    /*!
        \param xc center x
        \param yc center y
        \param w  width of rectangle
        \param h  height of rectangle
        \param angle angle of rotation of rectangle
        \param radius for a rounded rectangle
    */
    void    Set( double xc, double yc, double w, double h, double angle = 0 , double radius = 0 );

    //!set width of rectangle
    /*!
        \param w width of rectangle
    */
    void    SetWidth( double w )  { m_width = w; SetPending( true ); }

    //!set height of rectangle
    /*!
        \param h height of rectangle
    */
    void    SetHeight( double h ) { m_height = h; SetPending( true ); }

    //!return width
    double  GetWidth() const { return m_width; }
    //!return height
    double  GetHeight() const { return m_height; }
    //!return radius
    double  GetRadius() const { return m_height; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    virtual a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const;


    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    DECLARE_DYNAMIC_CLASS( a2dRectC )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    //! width of rectangle
    double m_width;
    //! height of rectangle
    double m_height;
    //!radius in case of rounded rectangle
    double m_radius;

public:

    static a2dPropertyIdDouble* PROPID_Width;
    static a2dPropertyIdDouble* PROPID_Height;
    static a2dPropertyIdDouble* PROPID_Radius;

    DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dRectC( const a2dRectC& ori );
};

//!a2dArrow is used for having line begin and ends on specific objects.
/*! Those objects do position the arrow at the right place, and scale if needed.
    \sa a2dEndsLine
    \sa a2dPolylineL

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dArrow: public a2dCanvasObject
{
public:

    //! constructor
    /*!
        \param xt x top of arrow (normal 0)
        \param yt y top of arrow (normal 0)
        \param l1 How long is the arrow.
        \param l2 Distance from top to the inside of the arrow.
        \param b  How broad is the basis of the arrow.
        \param spline Draw the arrow in a spline fashion
    */
    a2dArrow( double xt, double yt, double l1, double l2, double b, bool spline = false );

    //! constructor
    a2dArrow();

    //! copy constructor
    a2dArrow( const a2dArrow& ori, CloneOptions options, a2dRefMap* refs );

    ~a2dArrow();

    //! Sets top, length, height and broad parameters
    /*!
        \param xt: x top of arrow (normal 0)
        \param yt: y top of arrow (normal 0)
        \param l1: How long is the arrow.
        \param l2: Distance from top to the inside of the arrow.
        \param b: How broad is the basis of the arrow.
    */
    void    Set( double xt, double yt, double l1, double l2, double b, bool spline = false );

    //! Sets length, height and broad parameters
    /*!
        \param l1: How long is the arrow.
        \param l2: Distance from top to the inside of the arrow.
        \param b: How broad is the basis of the arrow.
    */
    void    Set( double l1, double l2, double b, bool spline = false );

    //! Returns the arrow length.
    double GetL1() { return m_l1; }

    //! return Distance from top to the inside of the arrow.
    double GetL2() { return m_l2; }

    //! return how broad the basis of the arrow is.
    double GetBase() { return m_b; }

    //! is the arrwo drawn as a splined polygon?
    bool GetSpline() { return m_spline; }

    //! return the arrow converted to a polygon.
    virtual a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const;


    //! return the arrow converted to a vector path.
    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    DECLARE_DYNAMIC_CLASS( a2dArrow )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    //! length from top of arrow to outside points in X
    double m_l1;

    //! length from top of arrow to inside point in X
    double m_l2;

    //! base of arrow
    double m_b;

    //! draw splined?
    bool m_spline;


private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dArrow( const a2dArrow& ori );
};

//! a2dWH
/*!
   Virtual base class for canvas objects with width and height parameters.
   The object has an extension from (x,y) to (x+w,y+h).

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dWH: public a2dCanvasObject
{
    DECLARE_EVENT_TABLE()

public:
    //!default constructor
    a2dWH();

    //!standard constructor
    /*!
    \param x: x minimum
    \param y: y minimum
    \param w: Width of object
    \param h: Heigth of object
    */
    a2dWH( double x, double y, double w, double h );

    //!destructor
    ~a2dWH();

    //!copy constructor
    a2dWH( const a2dWH& other, CloneOptions options, a2dRefMap* refs );

    //!set width of rectangle
    /*!
        \param w width of rectangle
    */
    void    SetWidth( double w )  { m_width = w; SetPending( true ); }

    //!set height of rectangle
    /*!
        \param h height of rectangle
    */
    void    SetHeight( double h ) { m_height = h; SetPending( true ); }

    //!return width
    double  GetWidth() const { return m_width; }
    //!return height
    double  GetHeight() const { return m_height; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    virtual bool RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld );

    DECLARE_CLASS( a2dWH )

protected:

    //! This is here so that this class cannot be used directly
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const = 0;

    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );
    void OnHandleEvent( a2dHandleMouseEvent& event );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );
    void OnResizeObjectToChildBox( a2dCanvasObjectEvent& event );

    //! width of rectangle
    double m_width;
    //! height of rectangle
    double m_height;

public:

    //! property to get m_width
    static a2dPropertyIdDouble* PROPID_Width;
    //! property to get m_height
    static a2dPropertyIdDouble* PROPID_Height;

    DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dWH( const a2dWH& other );
};

//! a2dWHCenter
/*!
   Virtual base class for canvas objects with width and height parameters.
   The object has an extension from (x-w/2,y-h/2) to (x+w/2,y+h/2).

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dWHCenter: public a2dWH
{
    DECLARE_EVENT_TABLE()

public:
    //!default constructor
    a2dWHCenter();

    //!standard constructor
    /*!
    \param xc: x center
    \param yc: y center
    \param w: Width (double radius) of object.
    \param h: Heigth (double radius) of object
    */
    a2dWHCenter( double xc, double yc, double w, double h );

    //!destructor
    ~a2dWHCenter();

    //!copy constructor
    a2dWHCenter( const a2dWHCenter& other, CloneOptions options, a2dRefMap* refs );

    DECLARE_CLASS( a2dWHCenter )

public:

    static a2dPropertyIdDouble* PROPID_Width;
    static a2dPropertyIdDouble* PROPID_Height;

    DECLARE_PROPERTIES()

protected:

    //! This is here so that this class cannot be used directly
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const = 0;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dWHCenter( const a2dWHCenter& other );
};

//! a2dRect
/*!
   Rectangle with radius

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dRect: public a2dWH
{

    DECLARE_EVENT_TABLE()

public:

    a2dRect();

    //!constructor
    /*!
    \param x: x minimum
    \param y: y minimum
    \param w: Width of rectangle.
    \param h: Heigth of rectangle
    \param radius Radius at corners (negatif inwards positif outwards).
    \param contourwidth contour around rectangle.
    */
    a2dRect( double x, double y, double w, double h , double radius = 0, double contourwidth = 0 );

    //!constructor
    /*!
    \param p1 one point of rectangle
    \param p2 second point on diagonal
    \param radius Radius at corners (negatif inwards positif outwards).
    \param contourwidth contour around rectangle.
    */
    a2dRect( const a2dPoint2D& p1,  const a2dPoint2D& p2, double radius = 0, double contourwidth = 0 );

    //!constructor
    /*!
    \param bbox used for size of rectangle
    \param radius Radius at corners (negatif inwards positif outwards).
    \param contourwidth contour around rectangle.
    */
    a2dRect( const a2dBoundingBox& bbox, double radius = 0, double contourwidth = 0 );

    a2dRect( const a2dRect& rec, CloneOptions options, a2dRefMap* refs );

    ~a2dRect();

    //!set the Contour width of the shape
    void SetContourWidth( double width ) { m_contourwidth = width; SetPending( true ); }

    //!get the Contour width of the shape
    double GetContourWidth() const { return m_contourwidth; }

    //!set corner radius of rectangle
    /*!
        \param radius  rounded corner radius of rectangle
    */
    void    SetRadius( double radius ) { m_radius = radius; SetPending( true ); }

    //!return radius
    double  GetRadius() const { return m_radius; }

    //! when editing can radius change proportional when scaling.
    void SetRadiusFixed( bool fixedRadius ) { m_fixedRadius = fixedRadius; }

    //! when editing can radius change proportional when scaling.
    bool GetRadiusFixed() const { return m_fixedRadius; }

    //! GDSII compatible to sub identify types of rectangles.
    /*! you can use it as a special tagged rectangle */
    void SetBoxType( int type ) { m_boxtype = type; }

    //! GDSII compatible to sub identify types of rectangles.
    /*! you can use it as a special tagged rectangle */
    int GetBoxType() const { return m_boxtype; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    virtual a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const;


    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y, double margin = 0 );

    void OnHandleEvent( a2dHandleMouseEvent& event );

    DECLARE_DYNAMIC_CLASS( a2dRect )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    //! GDSII compatible to sub identify types of rectangles.
    int m_boxtype;

    //!radius in case of rounded rectangle  (negatif inwards positif outwards)
    double m_radius;

    //! when editing can radius change proportional when scaling.
    bool m_fixedRadius;

    //! if != 0 you get a contour around the rectangle ( donut like ).
    double m_contourwidth;

    double m_radiusScale;

public:

    static int m_editModeScaleRadius;

    static a2dPropertyIdDouble* PROPID_Radius;
    static a2dPropertyIdBool* PROPID_AllowRounding;

    DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dRect( const a2dRect& other );
};

//! a2dCircle at x,y, and with radius
/*!
    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dCircle: public a2dCanvasObject
{
    DECLARE_EVENT_TABLE()

public:
    a2dCircle( const a2dCircle& ori, CloneOptions options, a2dRefMap* refs );

    a2dCircle();

    //! constructor
    /*!
        \param x center X
        \param y center Y
        \param radius radius of circle
        \param width contour width
    */
    a2dCircle( double x, double y, double radius, double width = 0 );

    //! destructor
    ~a2dCircle();

    //!set radius
    /*!
        \remark As side-effect this circle is set pending for redraw
        \param radius radius of circle
    */
    void SetRadius( double radius )        { m_radius = radius; SetPending( true );  }

    //! return radius
    double  GetRadius() const { return m_radius; }

    //!set the Contour width of the shape
    void SetContourWidth( double width ) { m_contourwidth = width; SetPending( true ); }

    //!get the Contour width of the shape
    double GetContourWidth() const { return m_contourwidth; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    virtual a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const;


    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y, double margin = 0 );

    DECLARE_DYNAMIC_CLASS( a2dCircle )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );
    void OnHandleEvent( a2dHandleMouseEvent& event );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //!radius of circle
    double m_radius;

    //! if != 0 you get a donut
    double m_contourwidth;

public:

    static a2dPropertyIdDouble* PROPID_Radius;

    DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dCircle( const a2dCircle& other );
};

//! a2dEllipse  centered at x,y.
/*!
    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dEllipse: public a2dWHCenter
{
public:

    a2dEllipse();

    //! constructor for ellipse
    /*!
        \param xc x center
        \param yc y center
        \param width Width of ellipse.
        \param height Heigth of ellipse.
        \param contourwidth contour around rectangle.
    */
    a2dEllipse( double xc, double yc, double width, double height, double contourwidth = 0 );
    a2dEllipse( const a2dEllipse& ori, CloneOptions options, a2dRefMap* refs );
    ~a2dEllipse();

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    virtual a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const;


    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    //!set the Contour width of the shape
    void SetContourWidth( double width ) { m_contourwidth = width; SetPending( true ); }

    //!get the Contour width of the shape
    double GetContourWidth() const { return m_contourwidth; }

    bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y, double margin = 0 );

    DECLARE_DYNAMIC_CLASS( a2dEllipse )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dEllipse( const a2dEllipse& other );

    //! if != 0 you get a contour around the rectangle ( donut like ).
    double m_contourwidth;

};

//! a2dEllipticArc centered at x,y
/*!
    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dEllipticArc: public a2dWHCenter
{
    DECLARE_EVENT_TABLE()

public:

    //! constructor
    a2dEllipticArc();

    //! Constructor.
    /*!
        \param xc: x center
        \param yc: y center
        \param width: Width of Arc
        \param height: Heigth of Arc
        \param start: Start Angle degrees
        \param end: End Angle in degrees
        \param chord if true only show outline.
        \param contourwidth to give a width to the cord
    */
    a2dEllipticArc( double xc, double yc, double width, double height, double start, double end, bool chord = false, double contourwidth = 0 );

    //! constructor for cloning
    a2dEllipticArc( const a2dEllipticArc& ori, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dEllipticArc();

    //!set start angle
    /*!
        \remark As side-effect the ellipse is set pending for redraw
        \param start start angle in degrees
    */
    void SetStart( double start ) { m_start = start; SetPending( true ); }

    //!return start angle
    double  GetStart() { return m_start; }

    //!set end angle
    /*!
        \remark As side-effect the ellipse is set pending for redraw
        \param end end angle in degrees
    */
    void SetEnd( double end ) { m_end = end; SetPending( true ); }

    //!return end angle
    double GetEnd() { return m_end; }

    //! if true draw as a chord ( no fill end no lines to center )
    void SetChord( bool chord );

    //! get chord setting
    bool GetChord() { return m_chord; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    virtual a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const;


    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    //!set the Contour width of the shape
    void SetContourWidth( double width ) { m_contourwidth = width; SetPending( true ); }

    //!get the Contour width of the shape
    double GetContourWidth() const { return m_contourwidth; }

    DECLARE_DYNAMIC_CLASS( a2dEllipticArc )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnChar( wxKeyEvent& event );

    void OnHandleEvent( a2dHandleMouseEvent& event );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //! drawn as a chord ( no fill end no lines to center )
    bool   m_chord;

    //! start angle
    double m_start;

    //! end angle
    double m_end;

    //! if != 0 you get a contour around the rectangle ( donut like ).
    double m_contourwidth;

public:

    static a2dPropertyIdDouble* PROPID_StartAngle;
    static a2dPropertyIdDouble* PROPID_EndAngle;
    static a2dPropertyIdBool* PROPID_Chord;

    DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dEllipticArc( const a2dEllipticArc& other );
};

#if (defined(__WXMSW__) && defined(WXUSINGDLL) )
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dEllipticArc>;
#endif

//! a2dArc centered at x,y
/*!
     The Arc is defined by three vertexes, one for the center and two for the start and end point of
     the arc.
     The arc is drawn in an anticlockwise direction from the start point to the end point when Y axis is down
     else clockwise direction.


     \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dArc: public a2dCanvasObject
{
    DECLARE_EVENT_TABLE()

public:

    //! constructor
    a2dArc();

    //!constructor
    /*!
        \param xc x center
        \param yc y center
        \param radius radius of the Arc.
        \param start Start Angle .
        \param end End Angle .
        \param chord When only the arc contour is shown, not the lines to the center and no fill.
        \param contourwidth to give a width to the cord

        \remark The angles are transfered to x,y positions only once.
     */
    a2dArc( double xc, double yc, double radius, double start, double end, bool chord = false, double contourwidth = 0 );

    //!constructor
    /*!
        \param x1 x1 used for starting angle and radius
        \param y1 y1 used for starting angle and radius
        \param x2 x2 used for end angle
        \param y2 y2 used for end angle
        \param xc x center
        \param yc y center
        \param chord When only the arc contour is shown, not the lines to the center and no fill.
        \param contourwidth to give a width to the cord
     */
    a2dArc( double x1, double y1, double x2, double y2, double xc, double yc, bool chord = false, double contourwidth = 0 );

    //! copy constructor
    a2dArc( const a2dArc& ori, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dArc();

    //! Get X1 of arc being begin point of arc
    double GetX1() const { return m_x1; }
    //! Get Y1 of arc being begin point of arc
    double GetY1() const { return m_y1; }
    //! Get X2 of arc being end point of arc
    double GetX2() const { return m_x2; }
    //! Get Y2 of arc being end point of arc
    double GetY2() const { return m_y2; }
    //! Get the start angle (calculated form x1/y1)
    double GetStartAngle() const;
    //! Get the end angle (calculated form x2/y2)
    double GetEndAngle() const;

    //! get point in between begin and end
    a2dPoint2D GetMidPoint() const;

    //! Set X1 of arc being begin point of arc
    void SetX1( double x1 ) { m_x1 = x1; SetPending( true ); }
    //! Set Y1 of arc being begin point of arc
    void SetY1( double y1 ) { m_y1 = y1; SetPending( true ); }
    //! Set X2 of arc being end point of arc
    void SetX2( double x2 ) { m_x2 = x2; SetPending( true ); }
    //! Set Y2 of arc being end point of arc
    void SetY2( double y2 ) { m_y2 = y2; SetPending( true ); }
    //! Set the start angle (actually sets x1/y1)
    void SetStartAngle( double a );
    //! Set the end angle (actually sets x2/y2)
    void SetEndAngle( double a );

    //! set arc using begin, end and middle point.
    void Set( double xs, double ys,  double xm, double ym, double xe, double ye );

    //! set radius of the Arc
    void SetRadius( double radius );

    //! get radius of the Arc
    double GetRadius() const;

    //! if true draw as a chord ( no fill end no lines to center )
    void SetChord( bool chord );

    //! get chord setting
    bool GetChord() const { return m_chord; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    //! convert to a polygon and return the vertexlist.
    virtual a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const;


    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    void SetContourWidth( double width ) { m_contourwidth = width; SetPending( true ); }

    //!get the Contour width of the shape
    double GetContourWidth() const { return m_contourwidth; }

    DECLARE_DYNAMIC_CLASS( a2dArc )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnChar( wxKeyEvent& event );

    void OnHandleEvent( a2dHandleMouseEvent& event );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //! drawn as a chord ( no fill end no lines to center )
    bool   m_chord;

    //! x of begin point
    double m_x1;
    //! y of begin point
    double m_y1;

    //! x of end point
    double m_x2;

    //! y of end point
    double m_y2;

    //! if != 0 the line is contoured at distance m_contourwidth/2
    double m_contourwidth;

public:

    //! property for class member
    static a2dPropertyIdDouble* PROPID_X1;
    //! property for class member
    static a2dPropertyIdDouble* PROPID_X2;
    //! property for class member
    static a2dPropertyIdDouble* PROPID_Y1;
    //! property for class member
    static a2dPropertyIdDouble* PROPID_Y2;
    //! property for class member
    static a2dPropertyIdBool* PROPID_Chord;
    //! property for class member
    static a2dPropertyIdDouble* PROPID_StartAngle;
    //! property for class member
    static a2dPropertyIdDouble* PROPID_EndAngle;

    DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dArc( const a2dArc& other );
};

//! a2dSLine
/*!
    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dSLine: public a2dCanvasObject
{
    DECLARE_EVENT_TABLE()

public:

    a2dSLine();

    //!constructor
    /*!
        \param x1 x line start
        \param y1 y line start
        \param x2 x line end
        \param y2 y line end
        \param contourwidth to give the line a width
     */
    a2dSLine( double x1, double y1, double x2, double y2, double contourwidth = 0 );

    //! constructor
    a2dSLine( const a2dSLine& ori, CloneOptions options, a2dRefMap* refs );


    //! destructor
    ~a2dSLine();

    //! Get (transformed) position of X1
    /*!
        \param transform if true return x transformed with local matrix
    */
    double  GetPosX1( bool transform = true ) const;

    //! Get (transformed) position of Y1
    /*!
        \param transform if true return y transformed with local matrix
    */
    double  GetPosY1( bool transform = true ) const;

    //! Get (transformed) position of X2
    /*!
        \param transform if true return x transformed with local matrix
    */
    double  GetPosX2( bool transform = true ) const;

    //! Get (transformed) position of Y2
    /*!
        \param transform if true return y transformed with local matrix
    */
    double  GetPosY2( bool transform = true ) const;

    //! Returns length of line
    /*!
        \return line length
    */
    double GetLength() const;

    //! reduce matrix to identity without replacing object
    bool EliminateMatrix();

    //! sets position of first point
    /*!
        \param x new x for the first point
        \param y new y for the first point
        \param afterinversion if true the input x,y or first inverted with the local matrix of the object.
        The inverted values are used to set the point.

        \remark afterinversion = true should be used if you have x,y relative to the line object,
        while afterinversion = false should be used when x,y are relative to the point in the line
    */
    void    SetPosXY1( double x, double y, bool afterinversion = true );

    //! sets position of second point
    /*!
        \param x new x for the second point
        \param y new y for the second point
        \param afterinversion if true the input x,y or first inverted with the local matrix of the object.
        The inverted values are used to set the point.

        \remark afterinversion = true should be used if you have x,y relative to the line object,
        while afterinversion = false should be used when x,y are relative to the point in the line
    */
    void    SetPosXY2( double x, double y, bool afterinversion = true );

    //! sets both positions of line
    /*!
        \param x1 new x for the first point
        \param y1 new y for the first point
        \param x2 new x for the second point
        \param y2 new y for the second point
        \param afterinversion if true the input x,y or first inverted with the local matrix of the object.
        The inverted values are used to set the point.

        \remark afterinversion = true should be used if you have x,y relative to the line object,
        while afterinversion = false should be used when x,y are relative to the point in the line
    */
    void    SetPosXY12( double x1, double y1, double x2, double y2, bool afterinversion = true );

    //! dummies to be compatible with a2dEndsLine
    virtual void SetBegin( a2dCanvasObject* WXUNUSED( begin ) ) {}
    //! dummies to be compatible with a2dEndsLine
    virtual void SetEnd( a2dCanvasObject* WXUNUSED( end ) ) {}

    //! dummies to be compatible with a2dEndsLine
    virtual a2dCanvasObject* GetBegin() { return NULL; }

    //! dummies to be compatible with a2dEndsLine
    virtual a2dCanvasObject* GetEnd() { return NULL; }

    //! dummies to be compatible with a2dEndsLine
    virtual void SetEndScaleX( double WXUNUSED( xs ) ) {}
    //! dummies to be compatible with a2dEndsLine
    virtual void SetEndScaleY( double WXUNUSED( ys ) ) {}

    //! dummies to be compatible with a2dEndsLine
    double GetEndScaleX() { return 1; }
    //! dummies to be compatible with a2dEndsLine
    double GetEndScaleY() { return 1; }

    //! set a contour width
    void SetContourWidth( double width ) { m_contourwidth = width; SetPending( true ); }

    //!get the Contour width of the shape
    double GetContourWidth() const { return m_contourwidth; }

    //! Set when m_contourwidth != 0 what is the end of the line should be.
    void SetPathType( a2dPATH_END_TYPE pathtype ) { m_pathtype = pathtype; }

    //! get when m_contourwidth != 0 what is the end of the line looks like.
    a2dPATH_END_TYPE GetPathType() { return m_pathtype; }

    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    virtual bool RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dSLine )

protected:


    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnChar( wxKeyEvent& event );

    void OnHandleEvent( a2dHandleMouseEvent& event );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    //! start x
    double m_x1;
    //! start y
    double m_y1;
    //! end x
    double m_x2;
    //! end y
    double m_y2;

    //! if != 0 the line is contoured at distance m_contourwidth/2
    double m_contourwidth;

    //! when m_contourwidth != 0 what is the end of the line looking like.
    a2dPATH_END_TYPE m_pathtype;

public:
    static a2dPropertyIdDouble* PROPID_X1;
    static a2dPropertyIdDouble* PROPID_X2;
    static a2dPropertyIdDouble* PROPID_Y1;
    static a2dPropertyIdDouble* PROPID_Y2;

    DECLARE_PROPERTIES()

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dSLine( const a2dSLine& other );
};

//! a2dEndsLine with begin and/or end object.
/*!
     \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dEndsLine: public a2dSLine
{
public:

    a2dEndsLine();

    //!constructor
    /*!
        \param x1: x line start
        \param y1: y line start
        \param x2: x line end
        \param y2: y line end
     */
    a2dEndsLine( double x1, double y1, double x2, double y2, double contourwidth = 0 );


    //! constructor using a a2dEndsLine as input
    a2dEndsLine( const a2dEndsLine& ori, CloneOptions options, a2dRefMap* refs );

    //! constructor using a a2dSLine as input (end and begin are set to NULL )
    a2dEndsLine( const a2dSLine& ori, CloneOptions options, a2dRefMap* refs );

    ~a2dEndsLine();

    //! set object to draw as begin point
    void SetBegin( a2dCanvasObject* begin );

    a2dCanvasObject* GetBegin() { return m_begin; }

    //! set object to draw as end point
    void SetEnd( a2dCanvasObject* end );

    a2dCanvasObject* GetEnd() { return m_end; }

    //! Scale begin and end object with this factor in X
    /*!
        \remark As side-effect this object is set pending for redraw
    */
    void SetEndScaleX( double xs ) { m_xscale = xs; SetPending( true ); }

    double GetEndScaleX() { return m_xscale; }

    //! Scale begin and end object with this factor in Y
    /*!
        \remark As side-effect this object is set pending for redraw
    */
    void SetEndScaleY( double ys ) { m_yscale = ys; SetPending( true ); }

    double GetEndScaleY() { return m_yscale; }

    //! if set take style changes on line to the arrow objects
    void SetStyleToArrow( bool takeStyleToArrow ) { m_takeStyleToArrow = takeStyleToArrow; }
    //! if true take style changes on line to the arrow objects
    bool GetStyleToArrow() { return m_takeStyleToArrow; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    DECLARE_DYNAMIC_CLASS( a2dEndsLine )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //! object drawn at begin point
    a2dCanvasObjectPtr m_begin;

    //! object drawn at end point
    a2dCanvasObjectPtr m_end;

    //!scale begin and end object with this factor in X
    double m_xscale;

    //!scale begin and end object with this factor in Y
    double m_yscale;

    //! if set take style changes on line to the arrow objects
    bool m_takeStyleToArrow;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dEndsLine( const a2dEndsLine& other );
};


//! a2dScaledEndLine
/*!
    The begin and end object on the line are scaled to the contour width.
    In case of a a2dArrow derived object, rendering is optimized to look good.
    All types of end objects are scalled up m_contourwidth.
    If m_contourwidth is zero, there will be no endpoints drawn.

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dScaledEndLine: public a2dEndsLine
{
    DECLARE_EVENT_TABLE()

public:

    a2dScaledEndLine();

    //!constructor
    /*!
        \param x1 x line start
        \param y1 y line start
        \param x2 x line end
        \param y2 y line end
        \param contourwidth to give the line a width
     */
    a2dScaledEndLine( double x1, double y1, double x2, double y2, double contourwidth = 0 );

    //! constructor using a a2dEndsLine as input
    a2dScaledEndLine( const a2dScaledEndLine& ori, CloneOptions options, a2dRefMap* refs );

    //! constructor
    a2dScaledEndLine( const a2dSLine& ori, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dScaledEndLine();

    //! set begin shape factors if it is a2dArrow derived
    void SetBegin( double l1, double l2, double b, bool spline = false );
    //! set end shape factors if it is a2dArrow derived
    void SetEnd( double l1, double l2, double b, bool spline = false );

    DECLARE_DYNAMIC_CLASS( a2dScaledEndLine )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dScaledEndLine( const a2dScaledEndLine& other );
};

//! a2dEndsEllipticChord with begin and/or end object.
/*!
     \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dEndsEllipticChord: public a2dEllipticArc
{
public:

    //! constructor
    a2dEndsEllipticChord();

    //!constructor
    /*!
        \param xc x cord start
        \param yc y cord start
        \param width width of cord
        \param height height of cord
        \param start start angle
        \param end end angle
     */
    a2dEndsEllipticChord( double xc, double yc, double width, double height, double start, double end );


    //! constructor using a a2dEndsEllipticChord as input
    a2dEndsEllipticChord( const a2dEndsEllipticChord& ori, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dEndsEllipticChord();

    //! set object to draw as begin point
    void SetBeginObj( a2dCanvasObject* begin );

    //! return begin object
    a2dCanvasObject* GetBeginObj() { return m_beginobj; }

    //! set object to draw as end point
    void SetEndObj( a2dCanvasObject* endob );

    //! return end object
    a2dCanvasObject* GetEndObj() { return m_endobj; }

    //! Scale begin and end object with this factor in X
    /*!
        \remark As side-effect this object is set pending for redraw
    */
    void SetEndScaleX( double xs ) { m_xscale = xs; SetPending( true ); }

    //! see SetEndScaleX()
    double GetEndScaleX() { return m_xscale; }

    //! Scale begin and end object with this factor in Y
    /*!
        \remark As side-effect this object is set pending for redraw
    */
    void SetEndScaleY( double ys ) { m_yscale = ys; SetPending( true ); }

    //! see SetEndScaleY()
    double GetEndScaleY() { return m_yscale; }

    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    DECLARE_DYNAMIC_CLASS( a2dEndsEllipticChord )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //! Get the coordinate transform for the begin object
    a2dAffineMatrix GetBeginTransform() const;
    //! Get the coordinate transform for the end object
    a2dAffineMatrix GetEndTransform() const;

    //! object drawn at begin point
    a2dCanvasObjectPtr m_beginobj;

    //! object drawn at end point
    a2dCanvasObjectPtr m_endobj;

    //!scale begin and end object with this factor in X
    double m_xscale;

    //!scale begin and end object with this factor in Y
    double m_yscale;
public:

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dEndsEllipticChord( const a2dEndsEllipticChord& other );
};

//! a2dControl captures a wxWindow inside
/*!
    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dControl: public a2dRect
{
public:

    //! constructor
    /*!
        \param x: x left top
        \param y: y left top
        \param width: width of the control drawing
        \param height: height of the control drawing
        \param control pointer to wxWindow derived control
    */
    a2dControl( double x, double y, double width, double height, wxWindow* control );
    //! constructor
    a2dControl( const a2dControl& ori, CloneOptions options, a2dRefMap* refs );
    //! destructor
    ~a2dControl();

    //! Get control pointer
    wxWindow* GetControl() {return m_control;}

    DECLARE_CLASS( a2dControl )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    virtual void DoEndEdit();

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //! control with a a2dCanvas as parent.
    wxWindow*     m_control;

private:

    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dControl( const a2dControl& other );
};


//! a2dWires is used to generate wires to Connect other a2dCanvasObject's
/*!
 a2dWires is not just one wire, but a set of wires.
 This is done by taking the position of the pins of the conncted a2dCanvasObject's,
 and based on that generate the wires.
 When Connected Objects are moved and the wire set is not frozen,
 The wires will automatically be resized to keep the objects connected via the wire set.

 \ingroup canvasobject
 \sa a2dWirePolylineL
*/
class A2DCANVASDLLEXP a2dWires: public a2dCanvasObject
{
public:

    //! constructor
    a2dWires();

    //! constructor
    a2dWires( a2dCanvasObject* toconnect, const wxString& pinname );

    //! constructor
    a2dWires( a2dCanvasObject* toconnect, a2dPin* pinc );

    //! destructor
    ~a2dWires();

    //! type of wire
    virtual bool IsConnect() const;

    bool NeedsUpdateWhenConnected() const { return !m_frozen; }

    bool ConnectWith(  a2dCanvasObject* parent, a2dCanvasObject* graph, const wxString& pinname, double margin = 1, bool undo = false );

    bool ConnectWith(  a2dCanvasObject* parent, a2dPin* pin, double margin = 1, bool undo = false );

    bool GetFrozen() { return m_frozen; }
    void SetFrozen( bool freeze ) { m_frozen = freeze; }

    DECLARE_DYNAMIC_CLASS( a2dWires )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual void CreateWires();
    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );
    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    //!are the wires frozen (means will not resize)
    bool m_frozen;
};

#endif /* __WXCANPRIM_H__ */

