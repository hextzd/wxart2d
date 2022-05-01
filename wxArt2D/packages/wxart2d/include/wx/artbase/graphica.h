/*! \file wx/artbase/graphica.h

    \brief the wxGraphicContext based drawing context

    Vector path drawing the wx way.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: graphica.h,v 1.10 2009/10/06 18:40:31 titato Exp $
*/

#ifndef __WXGRAPHICSA_H__
#define __WXGRAPHICSA_H__

// where we keep all our configuration symbol (wxART2D_USE_*)
#include "a2dprivate.h"

#if wxART2D_USE_GRAPHICS_CONTEXT

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/dcprint.h"

#include "wx/graphics.h"
#if wxCHECK_VERSION(2, 9, 0)
#include "graphics.h"
#endif

#include "wx/artbase/artglob.h"
#include "wx/artbase/stylebase.h"
#include "wx/genart/imagergba.h"
#include "wx/artbase/polyver.h"

#include <vector>

class a2dStrokeData : public wxGraphicsObjectRefData
{
public:
    a2dStrokeData( wxGraphicsRenderer* renderer = 0, const wxPen& pen = wxNullPen );
    a2dStrokeData( wxGraphicsRenderer* renderer, const a2dStroke& stroke );
    ~a2dStrokeData();

    void Init();

    virtual void Apply( wxGraphicsContext* context );
    virtual wxDouble GetWidth() { return m_stroke.GetWidth(); }

private :

    a2dStroke m_stroke;
};

class a2dFillData : public wxGraphicsObjectRefData
{
public:
    a2dFillData( wxGraphicsRenderer* renderer = 0 );
    a2dFillData( wxGraphicsRenderer* renderer, const wxBrush& brush );
    a2dFillData( wxGraphicsRenderer* renderer, const a2dFill& fill );
    ~a2dFillData ();

    virtual void Apply( wxGraphicsContext* context );


#if wxCHECK_VERSION(2, 9, 1)
    void CreateLinearGradientBrush( wxDouble x1, wxDouble y1, wxDouble x2, wxDouble y2,
                                    const wxGraphicsGradientStops& stops );
    void CreateRadialGradientBrush( wxDouble xo, wxDouble yo, wxDouble xc, wxDouble yc, wxDouble radius,
                                    const wxGraphicsGradientStops& stops );
#else
    void CreateLinearGradientBrush( wxDouble x1, wxDouble y1, wxDouble x2, wxDouble y2,
                                    const wxColour& c1, const wxColour& c2 );
    void CreateRadialGradientBrush( wxDouble xo, wxDouble yo, wxDouble xc, wxDouble yc, wxDouble radius,
                                    const wxColour& oColor, const wxColour& cColor );
#endif


protected:
    virtual void Init();

private :

    a2dFill m_fill;

};

class a2dFontData : public wxGraphicsObjectRefData
{
public:
    a2dFontData( wxGraphicsRenderer* renderer, const wxFont& font, const wxColour& col );
    a2dFontData( wxGraphicsRenderer* renderer, const a2dFont& font );
    ~a2dFontData();

    virtual void Apply( wxGraphicsContext* context );
private :

    a2dFont m_font;
};

class a2dBitmapData : public wxGraphicsObjectRefData
{
public:
    a2dBitmapData( wxGraphicsRenderer* renderer, const wxBitmap& bmp );
    a2dBitmapData( wxGraphicsRenderer* renderer, const wxImage& image );
    a2dBitmapData( wxGraphicsRenderer* renderer, const wxGraphicsBitmap& a2dbitmap );
    ~a2dBitmapData();

    virtual wxImage GetImage() const { return m_image; }
    virtual wxSize GetSize() const { return wxSize( m_width, m_height ); }
private :

    wxImage m_image;
    int m_width;
    int m_height;
};

class a2dMatrixData : public wxGraphicsMatrixData
{
public :
    a2dMatrixData( wxGraphicsRenderer* renderer, const a2dAffineMatrix* matrix = NULL ) ;
    virtual ~a2dMatrixData() ;

    virtual wxGraphicsObjectRefData* Clone() const ;

    // concatenates the matrix
    virtual void Concat( const wxGraphicsMatrixData* t );

    // sets the matrix to the respective values
    virtual void Set( wxDouble a = 1.0, wxDouble b = 0.0, wxDouble c = 0.0, wxDouble d = 1.0,
                      wxDouble tx = 0.0, wxDouble ty = 0.0 );

    // gets the component valuess of the matrix
    virtual void Get( wxDouble* a = NULL, wxDouble* b = NULL,  wxDouble* c = NULL,
                      wxDouble* d = NULL, wxDouble* tx = NULL, wxDouble* ty = NULL ) const;

    // makes this the inverse matrix
    virtual void Invert();

    // returns true if the elements of the transformation matrix are equal ?
    virtual bool IsEqual( const wxGraphicsMatrixData* t ) const ;

    // return true if this is the identity matrix
    virtual bool IsIdentity() const;

    //
    // transformation
    //

    // add the translation to this matrix
    virtual void Translate( wxDouble dx , wxDouble dy );

    // add the scale to this matrix
    virtual void Scale( wxDouble xScale , wxDouble yScale );

    // add the rotation to this matrix (radians)
    virtual void Rotate( wxDouble angle );

    //
    // apply the transforms
    //

    // applies that matrix to the point
    virtual void TransformPoint( wxDouble* x, wxDouble* y ) const;

    // applies the matrix except for translations
    virtual void TransformDistance( wxDouble* dx, wxDouble* dy ) const;

    // returns the native representation
    virtual void* GetNativeMatrix() const;
private:
    a2dAffineMatrix m_matrix ;
} ;

class a2dPathData : public wxGraphicsPathData
{
public :
    a2dPathData( wxGraphicsRenderer* renderer, a2dVpath* path = NULL );
    ~a2dPathData();

    virtual wxGraphicsObjectRefData* Clone() const;

    //
    // These are the path primitives from which everything else can be constructed
    //

    // begins a new subpath at (x,y)
    virtual void MoveToPoint( wxDouble x, wxDouble y );

    // adds a straight line from the current point to (x,y)
    virtual void AddLineToPoint( wxDouble x, wxDouble y );

    // adds a cubic Bezier curve from the current point, using two control points and an end point
    virtual void AddCurveToPoint( wxDouble cx1, wxDouble cy1, wxDouble cx2, wxDouble cy2, wxDouble x, wxDouble y );


    // adds an arc of a circle centering at (x,y) with radius (r) from startAngle to endAngle
    virtual void AddArc( wxDouble x, wxDouble y, wxDouble r, wxDouble startAngle, wxDouble endAngle, bool clockwise ) ;

    // gets the last point of the current path, (0,0) if not yet set
    virtual void GetCurrentPoint( wxDouble* x, wxDouble* y ) const;

    // adds another path
    virtual void AddPath( const wxGraphicsPathData* path );

    // closes the current sub-path
    virtual void CloseSubpath();

    //
    // These are convenience functions which - if not available natively will be assembled
    // using the primitives from above
    //

    /*

    // appends a rectangle as a new closed subpath
    virtual void AddRectangle( wxDouble x, wxDouble y, wxDouble w, wxDouble h ) ;
    // appends an ellipsis as a new closed subpath fitting the passed rectangle
    virtual void AddEllipsis( wxDouble x, wxDouble y, wxDouble w , wxDouble h ) ;

    // draws a an arc to two tangents connecting (current) to (x1,y1) and (x1,y1) to (x2,y2), also a straight line from (current) to (x1,y1)
    virtual void AddArcToPoint( wxDouble x1, wxDouble y1 , wxDouble x2, wxDouble y2, wxDouble r )  ;
    */

    // returns the native path
    virtual void* GetNativePath() const ;

    // give the native path returned by GetNativePath() back (there might be some deallocations necessary)
    virtual void UnGetNativePath( void* p ) const;

    // transforms each point of this path by the matrix
    virtual void Transform( const wxGraphicsMatrixData* matrix ) ;

    // gets the bounding box enclosing all points (possibly including control points)
    virtual void GetBox( wxDouble* x, wxDouble* y, wxDouble* w, wxDouble* h ) const;

    virtual bool Contains( wxDouble x, wxDouble y, wxPolygonFillMode fillStyle = wxWINDING_RULE ) const;

private :
    a2dVpath m_path;
};

class a2dContext : public wxGraphicsContext
{
    DECLARE_NO_COPY_CLASS( a2dContext )

public:

    a2dContext( wxGraphicsRenderer* renderer, int width, int height );
    a2dContext( wxGraphicsRenderer* renderer );
    a2dContext( wxGraphicsRenderer* renderer, a2dContext* context );
    //a2dContext( wxGraphicsRenderer* renderer, const wxImage& image );
    //a2dContext( wxGraphicsRenderer* renderer, const a2dImage* drawable );
    //a2dContext( wxGraphicsRenderer* renderer, wxWindow *window);
    a2dContext();
    virtual ~a2dContext();

#if wxCHECK_VERSION(2,9,0)
    // sets the antialiasing mode, returns true if it supported
    virtual bool SetAntialiasMode( wxAntialiasMode antialias ) { return false; }

    // sets the compositing operator, returns true if it supported
    virtual bool SetCompositionMode( wxCompositionMode op )  { return false; }
#endif

    virtual void Clip( const wxRegion& region );

    // clips drawings to the rect in user coordinates
    virtual void Clip( wxDouble x, wxDouble y, wxDouble w, wxDouble h );

    // resets the clipping to original extent
    virtual void ResetClip();

    virtual void* GetNativeContext();

    virtual void StrokePath( const wxGraphicsPath& p );
    virtual void FillPath( const wxGraphicsPath& p , wxPolygonFillMode fillStyle = wxWINDING_RULE );

    void DrawPath( const wxGraphicsPath& path, wxPolygonFillMode fillStyle = wxWINDING_RULE );

    virtual void Translate( wxDouble dx , wxDouble dy );
    virtual void Scale( wxDouble xScale , wxDouble yScale );
    virtual void Rotate( wxDouble angle );

    // concatenates this transform with the current transform of this context
    virtual void ConcatTransform( const wxGraphicsMatrix& matrix );

    // sets the pen
    virtual void SetPen( const wxGraphicsPen& pen );

    // sets the brush for filling
    virtual void SetBrush( const wxGraphicsBrush& brush );

    // sets the font
    virtual void SetFont( const wxGraphicsFont& font );

    void SetStroke( const a2dStroke& stroke );

    void SetFill( const a2dFill& fill );

    void SetFont( const a2dFont& font );

    //! set drawstyle to use for drawing,
    /*!
        The draw style is used for drawing in a certain fashion.

        The current fill, stroke are preserved. So when switching back to a drawing style
        where the last set fill and stroke are used, they still will be intact.
        New settings for fill, stroke may or may not have effect, but do change the current fill are stroke,
        independent of the drawing style.
        This depends if it makes sense to have a different style when the drawstyle is in action.
        When drawstyle is set, the last set fill, stroke are made active when needed.

        \param  drawstyle one of the draw styles
    */
    virtual void SetDrawStyle( a2dDrawStyle drawstyle ) = 0;

    //!get drawstyle used for drawing.
    a2dDrawStyle GetDrawStyle() {return m_drawstyle;}

    // all rendering is done into a fully transparent temporary context
    void BeginLayer( wxDouble opacity ) {};

    // composites back the drawings into the context with the opacity given at
    // the BeginLayer call
    void EndLayer() {};

    // sets the transform of this context
    virtual void SetTransform( const wxGraphicsMatrix& matrix );

    // gets the matrix of this context
    virtual wxGraphicsMatrix GetTransform() const;

#if wxCHECK_VERSION(2,9,0)
    virtual void DrawBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h );
#else
    virtual void DrawGraphicsBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h );
#endif

    virtual void DrawBitmap( const wxBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h );
    virtual void DrawIcon( const wxIcon& icon, wxDouble x, wxDouble y, wxDouble w, wxDouble h );
    virtual void PushState();
    virtual void PopState();

#if wxCHECK_VERSION(2,9,0)
    void DoDrawText( const wxString& str, wxDouble x, wxDouble y );
#else
    void DrawText( const wxString& str, wxDouble x, wxDouble y );
#endif
    virtual void GetTextExtent( const wxString& str, wxDouble* width, wxDouble* height,
                                wxDouble* descent, wxDouble* externalLeading ) const;
    virtual void GetPartialTextExtents( const wxString& text, wxArrayDouble& widths ) const;

    //! Set the display aberration of curved shapes
    /*!
        The drawing of curved shapes will not deviate more than this from the ideal curve.
        \remark the smaller the number the longer the drawing takes.

        \param aber maximum deviation in device coordinates
    */
    void SetDisplayAberration( double aber ) { m_displayaberration = aber; }

    //! Returns the display aberration of curved shapes
    /*!,
        The drawing of curved shapes will not deviate more than this from the ideal curve.
    */
    double GetDisplayAberration() { return m_displayaberration; }

    //!set at what size to stop drawing
    inline void SetPrimitiveThreshold( wxUint16 pixels ) { m_drawingthreshold = pixels; }

    //!get drawing threshold \sa SetDrawingThreshold
    inline wxUint16 GetPrimitiveThreshold() { return m_drawingthreshold; }

    //! when enabling m_useOpacityFactor, this is how transparent
    /*!
        \param OpacityFactor Real opacity is set opacity * m_OpacityFactor/255
    */
    void SetOpacityFactor( wxUint8 OpacityFactor ) { m_OpacityFactor = OpacityFactor; }

    //! see SetOpacityFactor()
    wxUint8 GetOpacityFactor() { return m_OpacityFactor; }

    //!get y axis orientation
    inline bool GetYaxis() const { return m_yaxis; }

    //!set if the Yaxis goes up or down
    virtual void SetYaxis( bool up );

protected:

    void Init();

    //! return true if the drawstyle in combination with the active stroke and fill,
    //! does not require filling
    inline bool IsStrokeOnly()
    {
        return ( m_drawstyle == a2dWIREFRAME ||
                 m_drawstyle == a2dWIREFRAME_ZERO_WIDTH ||
                 m_drawstyle == a2dWIREFRAME_INVERT ||
                 m_drawstyle == a2dWIREFRAME_INVERT_ZERO_WIDTH ||
                 m_activefill.IsNoFill() ||
                 m_activefill.GetStyle() == a2dFILL_TRANSPARENT
               );
    }

    //! return true if the the drawstyle in combination with the active stroke,
    //! does not require stroking
    inline bool IsStroked()
    {
        return !m_activestroke.IsNoStroke() && !m_activestroke.GetStyle() == a2dSTROKE_TRANSPARENT;
    }

    //! per drawer implementation
    virtual void DoSetActiveStroke() = 0;

    //! per drawer implementation
    virtual void DoSetActiveFill() = 0;

    //!Draw polygon in world coordinates using pointlist
    virtual void DrawPolygon( const a2dVertexList* list, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );

    //!Draw polyline in world coordinates using pointlist
    virtual void DrawLines( const a2dVertexList* list );

    //!Draw line in world coordinates
    virtual void DrawLine( double x1, double y1, double x2, double y2 );

    virtual void DrawVpath( const a2dVpath* path );

    //! convert vertex array containing line and arc segments in user coordinates to device coordinate lines.
    int ToDeviceLines( a2dVertexArray* points, a2dBoundingBox& devbbox, bool& smallPoly, bool replaceByRectangle = false );

    //! convert vertex list containing line and arc segments in user coordinates to device coordinate lines.
    int ToDeviceLines( const a2dVertexList* list, a2dBoundingBox& devbbox, bool& smallPoly, bool replaceByRectangle = false );

    //!Given the rectangle in world coordinate to be displayed, update the mappingmatrix.
    /*!
    To display all of a drawing, set this to the bounding box of the root object
    of the canvas.

    So vx1 and vx2 to the minimum x and y of the bounding box.
    Calculate xpp and ypp in such a manner that it will show the whole drawing.

    The buffer size or SetMappingDeviceRect() are not used when setting the mapping with this function.
    In fact all other mapping functions in a2dContext use this function to set the mapping for the a2dContext in the end.

    \remark when a relative world matrix SetTransform() is set, the m_usertodevice
       is recalculated to take the new mapping matrix and the already set
       user matrix into account.

    \param x:   map to device x (non zero if canvas window is smaller than drawer)
    \param y:   map to device y (non zero if canvas window is smaller than drawer)
    \param wx:  map to device of this size in x, normally same as width of buffer bitmap
    \param wy:  map to device of this size in y, normally same as height of buffer bitmap
    \param vx1: minimum x coordinate of display area
    \param vy1: minimum y coordinate of display area
    \param xpp: Number of world units per pixel in x
    \param ypp: Number of world units per pixel in y
    */
    void SetMappingUpp( double x, double y, double wx, double wy, double vx1, double vy1, double xpp, double ypp );

    //!Used for filling with Gradient fill style.
    void FillPolygon( int n, wxRealPoint points[] );

    //!Used for filling with Gradient fill style.
    bool MoveUp( int n, wxRealPoint points[] , double horline, int& index, int direction );

    //!Used for filling with Gradient fill style.
    void DetectCriticalPoints( int n, wxRealPoint points[] );

    // Core text drawing function.
    /*! This function will draw each character separately using the function drawchar.
        It will also advance and kern the characters, by adjusting the affine matrices.
        The character will also be checked against the clipbox.
        This function assumes (0,0) is the lowerleft bbox corner.
        This function will not draw the background fill.
        \param text The text to be drawn.
        \param x x-Position of the text.
        \param y y-Position of the text.
        \param drawchar A function which will draw a given character.
    */
    void DrawTextGeneric( const wxString& text, double x, double y, void ( a2dContext::*drawchar )( wxChar ) );

    //! Draw a cross instead of a character.
    virtual void DrawCharUnknown( wxChar c );

    //! Draw a cross, indicating an unsupported font type for this drawer.
    /*! Draws text at the given position in user coordinates.
        Adding rotation etc. to that matrix makes it possible to draw rotated text.
        \param text draw unknown text
        \param x x position
        \param y y position
        \param words If false, draw a boxed cross for the whole line.
                If true, draw a piece of line for each word.
    */
    virtual void DrawTextUnknown( const wxString& text, double x, double y, bool words = false );

    //! Draw a stroke character
    /*! This is an internal function, used by DrawTextStroke.
        This function assumes that the affine matrix has been set up by the calling function
        in such a way, that the (0,0) coordinate will match with the lowerleft bbox corner
        of the character. Also mirroring should be performed by the calling function.
    */
    virtual void DrawCharStroke( wxChar c );

    //! Wrapper for DrawCharStroke
    /*!
        ISO C++ seems(?) to forbid directly calling the address of virtual member functions.
        Calling &a2dContext::DrawCharStroke refers to the actual function in a2dContext and
        does not refer to the virtual function.
        Calling &(this->DrawCharStroke) should refer to the virtual function and is accepted by
        some compilers (MSVC, gcc <3.3), but according to gcc 3.4 this is not allowed by ISO C++.
        Therefore the virtual function is called through this intermediate function
    */
    void DrawCharStrokeCb( wxChar c ) { DrawCharStroke( c ); }

    //! Draw text in user coordinates, based on a stroke font.
    /* The text will be drawn with (0,0) being the lowerleft bbox corner.
        The background fill and alignment positioning are handled by DrawText.
    */
    virtual void DrawTextStroke( const wxString& text, double x, double y )
    { DrawTextGeneric( text, x, y, &a2dContext::DrawCharStrokeCb ); }

    //! Draw a freetype character
    /*! This is an internal function, used by DrawTextFreetype
        This function assumes that the affine matrix has been set up by the calling function
        in such a way, that the (0,0) coordinate will match with the lowerleft bbox corner
        of the character. Also mirroring should be performed by the calling function.
    */
    virtual void DrawCharFreetype( wxChar c ) { DrawCharUnknown( c ); }

    //! Wrapper for DrawCharFreetype
    /*! \sa DrawCharStrokeCb */
    void DrawCharFreetypeCb( wxChar c ) { DrawCharFreetype( c ); }

    //! Draw text in world coordinates, based on a freetype font .
    /* The text will be drawn with (0,0) being the lowerleft bbox corner.
        The background fill and alignment positioning are handled by DrawText.
    */
    virtual void DrawTextFreetype( const wxString& text, double x, double y )
    { DrawTextGeneric( text, x, y, &a2dContext::DrawCharFreetypeCb ); }

    //! Draw a dc character
    /*! This is an internal function, used by DrawTextDc
        This function assumes that the affine matrix has been set up by the calling function
        in such a way, that the (0,0) coordinate will match with the lowerleft bbox corner
        of the character. Also mirroring should be performed by the calling function.
    */
    virtual void DrawCharDc( wxChar c ) { DrawCharUnknown( c ); }

    //! Wrapper for DrawCharDc
    /*! \sa DrawCharStrokeCb */
    void DrawCharDcCb( wxChar c ) { DrawCharDc( c ); }

    //! Draw text in user coordinates, based on a dc font .
    /* The text will be drawn with (0,0) being the lowerleft bbox corner.
        The background fill and alignment positioning are handled by DrawText.
    */
    virtual void DrawTextDc( const wxString& text, double x, double y )
    { DrawTextGeneric( text, x, y, &a2dContext::DrawCharDcCb ); }

    //! Convert double points to integer coords in the point cache and return pointer.
    wxPoint* _convertToIntPointCache( int n, wxRealPoint* pts );

    //! draw an internal  polygon in device coordinates
    virtual void DeviceDrawPolygon( unsigned int n, bool spline , wxPolygonFillMode fillStyle );

    //! draw an internal polyline in device coordinates
    virtual void DeviceDrawLines( unsigned int n, bool spline );

    //!draw in pixels
    virtual void DeviceDrawLine( double x1, double y1, double x2, double y2 );

    //! Draw a pixel-width, unstroked horizontal line in device (pixel) coordinates
    /*! This method is useful for implementing low level fill routines.

    \param x1  x start of line
    \param x2  x end of line
    \param y1  y of line
    \param use_stroke_color If true, then use the current stroke color
                            If false, then use the current fill color.
    */
    virtual void DeviceDrawHorizontalLine( int x1, int y1, int x2, bool use_stroke_color );

    //! Draw a pixel-width, unstroked vertical line in device (pixel) coordinates
    /*! This method is useful for implementing low level fill routines.

    \param x1  x of line
    \param y1  y start of line
    \param y2  y end of line
    \param use_stroke_color If true, then use the current stroke color
                            If false, then use the current fill color.
    */
    virtual void DeviceDrawVerticalLine( int x1, int y1, int y2, bool use_stroke_color );

    //!draw a single, unstroked pixel in device coordinates with the given color
    virtual void DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255 );

    //! active stroke alpha
    wxUint8 m_StrokeOpacityCol1;
    //! active stroke alpha
    wxUint8 m_StrokeOpacityCol2;
    //! active fill alpha
    wxUint8 m_FillOpacityCol1;
    //! active fill alpha
    wxUint8 m_FillOpacityCol2;

    //! active stroke colours
    unsigned char m_colour1redStroke;
    //! active stroke colours
    unsigned char m_colour1greenStroke;
    //! active stroke colours
    unsigned char m_colour1blueStroke;

    //! active fill colours
    unsigned char m_colour1redFill;
    //! active fill colours
    unsigned char m_colour1greenFill;
    //! active fill colours
    unsigned char m_colour1blueFill;

    //! active fill colours
    unsigned char m_colour2redFill;
    //! active fill colours
    unsigned char m_colour2greenFill;
    //! active fill colours
    unsigned char m_colour2blueFill;

    //!used while rendering
    a2dStroke m_activestroke;

    //!used while rendering
    a2dFill m_activefill;

    //!current font set
    a2dFont m_a2dfont;

    //!drawstyle (like invert mode)
    a2dDrawStyle m_drawstyle;

    a2dAffineMatrix m_usertodevice;

    int m_width;

    int m_height;

    //! up or down
    bool m_yaxis;

    //! current clipping area in device coordinates
    wxRect m_clipboxdev;

    //! critical point list for polygon
    a2dCriticalPointList m_CRlist;

    //! scanline list of intersections with polygon
    a2dAETList m_AETlist;

    //!cached array for containing transformed device coordinates (integer)
    std::vector<wxPoint> m_cpointsInt;

    //!cached array for containing transformed device coordinates (double)
    std::vector<wxRealPoint> m_cpointsDouble;

    //!accuracy of arc segment calculation etc. in device coordinates
    double m_displayaberration;

    //! object smaller than this value will not be rendered
    wxUint16 m_drawingthreshold;

    //! opacity will be the one set, derived by this
    wxUint8 m_OpacityFactor;

private:

    void ColourXYLinear( int x1, int x2, int y );
    void ColourXYRadial( int x1, int x2, int y );

    //! gradient start stop
    double m_dx1, m_dy1, m_dx2, m_dy2, m_radiusd, m_length, m_max_x, m_min_x, m_max_y, m_min_y;
    a2dLine m_line;

};

class a2dDcContext : public a2dContext
{
    DECLARE_NO_COPY_CLASS( a2dDcContext )

public:

    a2dDcContext( wxGraphicsRenderer* renderer, wxMemoryDC* dc, wxBitmap* drawable );
    a2dDcContext( wxGraphicsRenderer* renderer, wxBitmap* drawable );
    virtual ~a2dDcContext();

    // clips drawings to the rect
    virtual void Clip( wxDouble x, wxDouble y, wxDouble w, wxDouble h );

    // resets the clipping to original extent
    virtual void ResetClip();

    //! set drawstyle to use for drawing,
    /*!
        The draw style is used for drawing in a certain fashion.

        The current fill, stroke are preserved. So when switching back to a drawing style
        where the last set fill and stroke are used, they still will be intact.
        New settings for fill, stroke may or may not have effect, but do change the current fill are stroke,
        independent of the drawing style.
        This depends if it makes sense to have a different style when the drawstyle is in action.
        When drawstyle is set, the last set fill, stroke are made active when needed.

        \param  drawstyle one of the draw styles
    */
    virtual void SetDrawStyle( a2dDrawStyle drawstyle );

    // sets the current logical function, returns true if it supported
    virtual bool SetLogicalFunction( wxRasterOperationMode function ) ;

#if wxCHECK_VERSION(2,9,0)
    virtual void DrawBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h );
#else
    virtual void DrawGraphicsBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h );
#endif

    virtual void DrawCharDc( wxChar c );
    virtual void DrawTextDc( const wxString& text, double x, double y );
    virtual void DrawCharFreetype( wxChar c );

protected:

    //! per drawer implementation
    virtual void DoSetActiveStroke();

    //! per drawer implementation
    virtual void DoSetActiveFill();

    virtual void DrawVpath( const a2dVpath* path );

    void DeviceDrawBitmap( const wxBitmap& bmp, double x, double y, bool useMask );

    //! draw an internal  polygon in device coordinates
    virtual void DeviceDrawPolygon( unsigned int n, bool spline , wxPolygonFillMode fillStyle );
    virtual void DeviceDrawLines( unsigned int n, bool spline );
    virtual void DeviceDrawLine( double x1, double y1, double x2, double y2 );
    virtual void DeviceDrawHorizontalLine( int x1, int y1, int x2, bool use_stroke_color );
    virtual void DeviceDrawVerticalLine( int x1, int y1, int y2, bool use_stroke_color );
    virtual void DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255 );

    //!the buffer that is used for rendering
    wxBitmap*  m_buffer;

    //!The DC to use to draw to the device or buffer ( depends on type of a2dDrawer2D )
    /*!In case of a m_buffer you must select the bitmap into the DC yourself. */
    wxDC*     m_renderDC;
};

//-----------------------------------------------------------------------------
// a2dRenderer declaration
//-----------------------------------------------------------------------------

class a2dRenderer : public wxGraphicsRenderer
{
public :
    a2dRenderer() {}

    virtual ~a2dRenderer() {}

    // Context

    virtual wxGraphicsContext* CreateContext( const wxWindowDC& dc );

    virtual wxGraphicsContext* CreateContext( const wxMemoryDC& dc );

    virtual wxGraphicsContext* CreateContext( wxMemoryDC* dc, wxBitmap* drawable );

    virtual wxGraphicsContext* CreateContextFromNativeContext( void* context );

    virtual wxGraphicsContext* CreateContextFromNativeWindow( void* window );

    virtual wxGraphicsContext* CreateContext( wxWindow* window );

    virtual wxGraphicsContext* CreateMeasuringContext();

#if wxCHECK_VERSION(2, 9, 0)
#if wxUSE_PRINTING_ARCHITECTURE
    virtual wxGraphicsContext* CreateContext( const wxPrinterDC& dc );
#ifdef __WXMSW__
    virtual wxGraphicsContext* CreateContext( const wxEnhMetaFileDC& dc );
#endif

#endif // wxUSE_PRINTING_ARCHITECTURE
#endif //wxCHECK_VERSION(2, 9, 0)


    // Path
    virtual wxGraphicsPath CreatePath();

    // Matrix
    virtual wxGraphicsMatrix CreateMatrix( wxDouble a = 1.0, wxDouble b = 0.0, wxDouble c = 0.0, wxDouble d = 1.0,
                                           wxDouble tx = 0.0, wxDouble ty = 0.0 );

    virtual wxGraphicsPen CreatePen( const wxPen& pen ) ;

    virtual wxGraphicsBrush CreateBrush( const wxBrush& brush ) ;

    virtual wxGraphicsPen CreateStroke( const a2dStroke& stroke ) ;

    virtual wxGraphicsBrush CreateFill( const a2dFill& fill ) ;

#if wxCHECK_VERSION(2, 9, 1)
    virtual wxGraphicsBrush
    CreateLinearGradientBrush( wxDouble x1, wxDouble y1,
                               wxDouble x2, wxDouble y2,
                               const wxGraphicsGradientStops& stops );

    virtual wxGraphicsBrush
    CreateRadialGradientBrush( wxDouble xo, wxDouble yo,
                               wxDouble xc, wxDouble yc,
                               wxDouble radius,
                               const wxGraphicsGradientStops& stops );

    // create a graphics bitmap from a native bitmap
    virtual wxGraphicsBitmap CreateBitmapFromNativeBitmap( void* bitmap );

#else
    // sets the brush to a linear gradient, starting at (x1,y1) with color c1 to (x2,y2) with color c2
    virtual wxGraphicsBrush CreateLinearGradientBrush( wxDouble x1, wxDouble y1, wxDouble x2, wxDouble y2,
            const wxColour& c1, const wxColour& c2 ) ;

    // sets the brush to a radial gradient originating at (xo,yc) with color oColor and ends on a circle around (xc,yc)
    // with radius r and color cColor
    virtual wxGraphicsBrush CreateRadialGradientBrush( wxDouble xo, wxDouble yo, wxDouble xc, wxDouble yc, wxDouble radius,
            const wxColour& oColor, const wxColour& cColor ) ;
#endif



    // sets the font
    virtual wxGraphicsFont CreateFont( const wxFont& font , const wxColour& col = *wxBLACK ) ;

    virtual wxGraphicsFont CreateFont( const a2dFont& font ) ;

    wxGraphicsBitmap CreateBitmap( const wxBitmap& bitmap ) ;

    wxGraphicsBitmap CreateBitmap( const wxImage& image ) ;

    virtual wxGraphicsBitmap CreateSubBitmap( const wxGraphicsBitmap& bitmap, wxDouble x, wxDouble y, wxDouble w, wxDouble h  );


private :
    DECLARE_DYNAMIC_CLASS_NO_COPY( a2dRenderer )
} ;



#endif //wxART2D_USE_GRAPHICS_CONTEXT


#endif
