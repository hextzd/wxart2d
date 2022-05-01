/*! \file wx/canvas/rectangle.h
    \brief rectangular shapes derived from a2dCanvasObject

    rectangle shapes

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: rectangle.h,v 1.9 2008/07/19 18:29:44 titato Exp $
*/

#ifndef __WXRECTPRIM_H__
#define __WXRECTPRIM_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/canobj.h"

class A2DARTBASEDLLEXP a2dVpath;

//! a2dRectMM
/*!
   Rectangle with radius and Min and Max coordinates.
   This rectangle has extra information such that its origin is not always the position of the matrix.
   It can be placed relative to its matrix in any way.
   The need for this type of rectangle, becomes clear when one needs to resize a rectangular shape around
   its children objects.


    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dRectMM: public a2dCanvasObject
{

    DECLARE_EVENT_TABLE()

public:

    a2dRectMM();

    //!constructor
    /*!
    \param x: x minimum
    \param y: y minimum
    \param w: Width of rectangle.
    \param h: Heigth of rectangle
    \param radius Radius at corners (negatif inwards positif outwards).
    \param contourwidth contour around rectangle.
    */
    a2dRectMM( double x, double y, double w, double h , double radius = 0, double contourwidth = 0 );

    //!constructor
    /*!
    \param p1 one point of rectangle
    \param p2 second point on diagonal
    \param radius Radius at corners (negatif inwards positif outwards).
    \param contourwidth contour around rectangle.
    */
    a2dRectMM( const a2dPoint2D& p1,  const a2dPoint2D& p2, double radius = 0, double contourwidth = 0 );

    //!constructor
    /*!
    \param bbox used for size of rectangle
    \param radius Radius at corners (negatif inwards positif outwards).
    \param contourwidth contour around rectangle.
    */
    a2dRectMM( const a2dBoundingBox& bbox, double radius = 0, double contourwidth = 0 );

    a2dRectMM( const a2dRectMM& rec, CloneOptions options, a2dRefMap* refs );

    ~a2dRectMM();

    //! set minimum in x and y
    void SetMin( double minx, double miny ) { m_minx = minx; m_miny = miny; SetPending( true ); }

    //! set maximum in x and y
    void SetMax( double maxx, double maxy ) { m_maxx = maxx; m_maxy = maxy; SetPending( true ); }

    //!set width of rectangle
    /*!
        \param w width of rectangle
    */
    void    SetWidth( double w )  { m_maxx = m_minx + w; SetPending( true ); }

    //!set height of rectangle
    /*!
        \param h height of rectangle
    */
    void    SetHeight( double h ) { m_maxy = m_miny + h; SetPending( true ); }

    //!return width
    double  GetWidth() const { return m_maxx - m_minx; }

    //!return height
    double  GetHeight() const { return m_maxy - m_miny; }

    //!set the Contour width of the shape
    void SetBorder( double width ) { m_border = width; SetPending( true ); }

    //!get the Contour width of the shape
    double GetBorder() const { return m_border; }

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

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    virtual a2dVertexList* GetAsVertexList( bool& returnIsPolygon ) const;

    a2dCanvasObjectList* GetAsCanvasVpaths( bool transform = true ) const;

    bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y, double margin = 0 );
    void OnHandleEvent( a2dHandleMouseEvent& event );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    DECLARE_DYNAMIC_CLASS( a2dRectMM )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );
    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );
    a2dBoundingBox DoGetUnTransformedBbox( a2dBboxFlags flags = a2dCANOBJ_BBOX_NON ) const;

    //!radius in case of rounded rectangle  (negatif inwards positif outwards)
    double m_radius;

    //! if != 0 you get a contour around the rectangle ( donut like ).
    double m_contourwidth;

    //! minimum x of rectangle
    double m_minx;

    //! minimum of rectangle
    double m_miny;

    //! maximum x of rectangle
    double m_maxx;

    //! maximum y of rectangle
    double m_maxy;

    bool m_flipIn;

    //! border around children towards the rectangle
    double m_border;


private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dRectMM( const a2dRectMM& other );
};

//! a2dWindowMM
/*!
   Rectangular Window with Min and Max coordinates.

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP a2dWindowMM: public a2dRectMM
{

    DECLARE_EVENT_TABLE()

public:

    enum a2dWindowMMStyle
    {
        SUNKEN = 0x0001, //!<
        RAISED = 0x0002 //!<
    };
    enum a2dWindowMMState
    {
        NON     = 0x0000, //!<
        DISABLED = 0x0001, //!<
        SELECTED = 0x0002, //!<
        FOCUS   = 0x0004, //!<
        HOVER   = 0x0008, //!<
        ALL     = 0xFFFF //!<
    };

    a2dWindowMM();

    //!constructor
    /*!
    \param x: x minimum
    \param y: y minimum
    \param w: Width of rectangle.
    \param h: Heigth of rectangle
    */
    a2dWindowMM( double x, double y, double w, double h );

    //!constructor
    /*!
    \param p1 one point of rectangle
    \param p2 second point on diagonal
    */
    a2dWindowMM( const a2dPoint2D& p1,  const a2dPoint2D& p2 );

    //!constructor
    /*!
    \param bbox used for size of rectangle
    */
    a2dWindowMM( const a2dBoundingBox& bbox );

    a2dWindowMM( const a2dWindowMM& rec, CloneOptions options, a2dRefMap* refs );

    ~a2dWindowMM();

    //! Set drawing style for window see a2dWindowMMStyle
    void SetStyle( long style ) { m_style = style; }

    //! Get drawing style for window see a2dWindowMMStyle
    long GetStyle() { return m_style;}

    //! initialize
    void Init();

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    DECLARE_DYNAMIC_CLASS( a2dWindowMM )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    void OnEnterObject( a2dCanvasObjectMouseEvent& event );

    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    long m_style;
    long m_state;

    a2dStroke m_backStroke;
    a2dStroke m_darkStroke;
    a2dStroke m_lightStroke;
    a2dStroke m_whiteStroke;
    a2dStroke m_blackStroke;
    a2dStroke m_hoverStroke;


private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dWindowMM( const a2dWindowMM& other );
};


class A2DCANVASDLLEXP a2dWidgetButton;

class A2DCANVASDLLEXP a2dRectWindowT2: public a2dRectMM
{

    DECLARE_EVENT_TABLE()

public:

    a2dRectWindowT2();

    //!constructor
    /*!
    \param parent parent object of this object
    \param x x minimum
    \param y y minimum
    \param w Width of rectangle.
    \param h Heigth of rectangle
    \param radius Radius at corners (negatif inwards positif outwards).
    */
    a2dRectWindowT2( a2dCanvasObject* parent, double x, double y, double w, double h , double radius = 0 );

    //!constructor
    /*!
    \param parent parent object of this object
    \param p1 one point of rectangle
    \param p2 second point on diagonal
    \param radius Radius at corners (negatif inwards positif outwards).
    */
    a2dRectWindowT2( a2dCanvasObject* parent, const a2dPoint2D& p1,  const a2dPoint2D& p2, double radius = 0 );

    //!constructor
    /*!
    \param parent parent object of this object
    \param bbox used for size of rectangle
    \param radius Radius at corners (negatif inwards positif outwards).
    */
    a2dRectWindowT2( a2dCanvasObject* parent, const a2dBoundingBox& bbox, double radius = 0 );

    //! constructor
    a2dRectWindowT2( const a2dRectWindowT2& rec, CloneOptions options, a2dRefMap* refs );

    //! destructor
    ~a2dRectWindowT2();

    //! initialization used in construction
    void Init( a2dCanvasObject* parent );

    //! closing behaviour
    void OnCloseWindow( wxCommandEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    //! get title of window
    a2dSmrtPtr<a2dText>& GetTitle() { return m_title; }

    //! set title a2dText object
    void SetTitle( a2dText* textObj );

    //! set title string
    void SetTitle( const wxString& title );

    //! set parent object of the pin or some other objects that needs a parent
    virtual void SetParent( a2dCanvasObject* parent ) { m_parent = parent; }


    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );
    bool ProcessCanvasObjectEvent( a2dIterC& ic, a2dHitEvent& hitEvent );
    void DoAddPending( a2dIterC& ic );
    bool Update( a2dCanvasObject::UpdateMode mode );
    void Render( a2dIterC& ic, OVERLAP clipparent );


    DECLARE_DYNAMIC_CLASS( a2dRectWindowT2 )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    a2dCanvasObjectPtr m_titleObj;

    //! title a2dText object
    a2dSmrtPtr<a2dText> m_title;

    //! fill for title object
    a2dFill m_titleFill;
    //! stroke for title object
    a2dStroke m_titleStroke;

    //! close object
    a2dSmrtPtr<a2dWidgetButton> m_close;

    //* This is intentionally not a smart pointer to remove a reference loop */
    a2dCanvasObject* m_parent;

    //! cache to hold the title bar height, claculated from the Title text font and size.
    mutable double m_titleheight;

    //! default Title text height
    static double m_initialTitleHeight;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dRectWindowT2( const a2dRectWindowT2& other );
};

class A2DCANVASDLLEXP a2dRectWindow: public a2dWindowMM
{

    DECLARE_EVENT_TABLE()

public:

    a2dRectWindow();

    //!constructor
    /*!
    \param parent parent object of this object
    \param x x minimum
    \param y y minimum
    \param w Width of rectangle.
    \param h Heigth of rectangle
    \param radius Radius at corners (negatif inwards positif outwards).
    */
    a2dRectWindow( a2dCanvasObject* parent, double x, double y, double w, double h );

    //!constructor
    /*!
    \param parent parent object of this object
    \param p1 one point of rectangle
    \param p2 second point on diagonal
    \param radius Radius at corners (negatif inwards positif outwards).
    */
    a2dRectWindow( a2dCanvasObject* parent, const a2dPoint2D& p1,  const a2dPoint2D& p2 );

    //!constructor
    /*!
    \param parent parent object of this object
    \param bbox used for size of rectangle
    \param radius Radius at corners (negatif inwards positif outwards).
    */
    a2dRectWindow( a2dCanvasObject* parent, const a2dBoundingBox& bbox );

    a2dRectWindow( const a2dRectWindow& rec, CloneOptions options, a2dRefMap* refs );

    ~a2dRectWindow();

    void Init( a2dCanvasObject* parent );

    void OnCloseWindow( wxCommandEvent& event );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    a2dSmrtPtr<a2dText>& GetTitle() { return m_title; }

    //! set title a2dText object
    void SetTitle( a2dText* textObj );

    //! set title string
    void SetTitle( const wxString& title );

    //! Get canvas object pointer
    a2dSmrtPtr<a2dWindowMM>& GetCanvas() { return m_canvas; }

    //! Set canvas object
    void SetCanvas( a2dWindowMM* canvas );

    //! set parent object of the pin or some other objects that needs a parent
    virtual void SetParent( a2dCanvasObject* parent ) { m_parent = parent; }

    DECLARE_DYNAMIC_CLASS( a2dRectWindow )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    //! title a2dText object
    a2dSmrtPtr<a2dText> m_title;

    //! fill for title object
    a2dFill m_titleFill;
    //! stroke for title object
    a2dStroke m_titleStroke;

    //! close object for closing window
    a2dSmrtPtr<a2dWidgetButton> m_close;

    //! This is intentionally not a smart pointer to remove a reference loop
    a2dCanvasObject* m_parent;

    //! here object are placed.
    a2dSmrtPtr<a2dWindowMM> m_canvas;

    //! cache to hold the title bar height, claculated from the Title text font and size.
    mutable double m_titleheight;

    //! default Title text height
    static double m_initialTitleHeight;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dRectWindow( const a2dRectWindow& other );
};

#endif /* __WXRECTPRIM_H__ */

