/*! \file wx/artbase/graphica.cpp

    \brief the wxGraphicContext based drawing context

    Vector path drawing the wx way.

    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: graphica.cpp,v 1.10 2009/10/06 18:40:31 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/image.h"
#include "wx/window.h"
#include "wx/dc.h"
#include "wx/utils.h"
#include "wx/dialog.h"
#include "wx/app.h"
#include "wx/bitmap.h"
#include "wx/dcmemory.h"
#include "wx/log.h"
#include "wx/icon.h"
#include "wx/dcprint.h"
#include "wx/module.h"
#endif

#include "wx/artbase/graphica.h"

#include "wx/dc.h"

#include "wx/graphics.h"
#include "wx/rawbmp.h"

#ifdef __WXMSW__
    #include "wx/msw/dcclient.h"
    #include "wx/msw/dcmemory.h"
    #include "wx/msw/dcscreen.h"
#endif

#ifdef __WXGTK3__
    #include "wx/gtk/dc.h"
#elif defined __WXGTK20__
#include "wx/gtk/dcclient.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/dcclient.h"
    #include "wx/gtk1/dcmemory.h"
    #include "wx/gtk1/dcscreen.h"
#endif

#ifdef __WXMAC__
    #include "wx/osx/dcclient.h"
    #include "wx/osx/dcmemory.h"
    #include "wx/osx/dcscreen.h"
#endif

#ifdef __WXPM__
    #include "wx/os2/dcclient.h"
    #include "wx/os2/dcmemory.h"
    #include "wx/os2/dcscreen.h"
#endif

#ifdef __WXCOCOA__
    #include "wx/cocoa/dcclient.h"
    #include "wx/cocoa/dcmemory.h"
    #include "wx/cocoa/dcscreen.h"
#endif

#ifdef __WXMOTIF__
    #include "wx/motif/dcclient.h"
    #include "wx/motif/dcmemory.h"
    #include "wx/motif/dcscreen.h"
#endif

#ifdef __WXX11__
    #include "wx/x11/dcclient.h"
    #include "wx/x11/dcmemory.h"
    #include "wx/x11/dcscreen.h"
#endif

#ifdef __WXDFB__
    #include "wx/dfb/dcclient.h"
    #include "wx/dfb/dcmemory.h"
    #include "wx/dfb/dcscreen.h"
#endif

#if defined(__WXMSW__)
    #include <wx/msw/private.h>
#elif defined(__WXGTK__)
    #include <wx/dcps.h>
#endif

#if wxART2D_USE_FREETYPE
#include <ft2build.h>
#ifdef _MSC_VER
// Do not use the define include for MSVC, because of a bug with precompiled
// headers. See (freetype)/doc/INSTALL.ANY for (some) more information
#include <freetype.h>
#include <ftoutln.h>
#else // _MSC_VER
// This is the way it should be included.
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#endif // _MSC_VER
#endif

#if wxART2D_USE_GRAPHICS_CONTEXT

#include <vector>

using namespace std;

//-----------------------------------------------------------------------------
// constants
//-----------------------------------------------------------------------------

const double RAD2DEG = 180.0 / M_PI;

//-----------------------------------------------------------------------------
// Local functions
//-----------------------------------------------------------------------------

static inline double dmin( double a, double b )
{
    return a < b ? a : b;
}
static inline double dmax( double a, double b )
{
    return a > b ? a : b;
}

static inline double DegToRad( double deg )
{
    return ( deg * M_PI ) / 180.0;
}
static inline double RadToDeg( double deg )
{
    return ( deg * 180.0 ) / M_PI;
}


//-----------------------------------------------------------------------------
// wxGraphicsPath implementation
//-----------------------------------------------------------------------------

// TODO remove this dependency (gdiplus needs the macros)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


//-----------------------------------------------------------------------------
// a2dStrokeData implementation
//-----------------------------------------------------------------------------

a2dStrokeData::~a2dStrokeData()
{
}

void a2dStrokeData::Init()
{
}

a2dStrokeData::a2dStrokeData( wxGraphicsRenderer* renderer, const wxPen& pen )
    : wxGraphicsObjectRefData( renderer )
{
    Init();
    m_stroke = a2dStroke( pen );
}

a2dStrokeData::a2dStrokeData( wxGraphicsRenderer* renderer, const a2dStroke& stroke )
    : wxGraphicsObjectRefData( renderer )
{
    Init();
    m_stroke = stroke;
}

void a2dStrokeData::Apply( wxGraphicsContext* context )
{
    a2dContext* ctext = ( a2dContext* ) context->GetNativeContext();
    ctext->SetStroke( m_stroke );
}

//-----------------------------------------------------------------------------
// a2dFillData implementation
//-----------------------------------------------------------------------------

a2dFillData::a2dFillData( wxGraphicsRenderer* renderer )
    : wxGraphicsObjectRefData( renderer )
{
    Init();
}

a2dFillData::a2dFillData( wxGraphicsRenderer* renderer, const wxBrush& brush )
    : wxGraphicsObjectRefData( renderer )
{
    Init();
    m_fill = a2dFill( brush );
}

a2dFillData::a2dFillData( wxGraphicsRenderer* renderer, const a2dFill& fill )
    : wxGraphicsObjectRefData( renderer )
{
    Init();
    m_fill = fill;
}

a2dFillData::~a2dFillData ()
{
}

void a2dFillData::Apply( wxGraphicsContext* context )
{
    a2dContext* ctext = ( a2dContext* ) context->GetNativeContext();
    ctext->SetFill( m_fill );
}

#if wxCHECK_VERSION(2, 9, 1)
void a2dFillData::CreateLinearGradientBrush( wxDouble x1, wxDouble y1, wxDouble x2, wxDouble y2,
        const wxGraphicsGradientStops& stops )
{
    //todo
    m_fill = a2dFill();
}

void a2dFillData::CreateRadialGradientBrush( wxDouble xo, wxDouble yo, wxDouble xc, wxDouble yc, wxDouble radius,
        const wxGraphicsGradientStops& stops )
{
    //todo
    m_fill = a2dFill();
}
#else
void a2dFillData::CreateLinearGradientBrush( wxDouble x1, wxDouble y1, wxDouble x2, wxDouble y2,
        const wxColour& c1, const wxColour& c2 )
{
    //todo
    m_fill = a2dFill();
}

void a2dFillData::CreateRadialGradientBrush( wxDouble xo, wxDouble yo, wxDouble xc, wxDouble yc, wxDouble radius,
        const wxColour& oColor, const wxColour& cColor )
{
    //todo
    m_fill = a2dFill();
}
#endif


void a2dFillData::Init()
{
}

//-----------------------------------------------------------------------------
// a2dFontData implementation
//-----------------------------------------------------------------------------

a2dFontData::a2dFontData( wxGraphicsRenderer* renderer, const wxFont& font,
                          const wxColour& col ) : wxGraphicsObjectRefData( renderer )
{
    m_font = a2dFont( font, font.GetPointSize() );
}

a2dFontData::a2dFontData( wxGraphicsRenderer* renderer, const a2dFont& font )
    : wxGraphicsObjectRefData( renderer )
{
    m_font = font;
}

a2dFontData::~a2dFontData()
{
}

void a2dFontData::Apply( wxGraphicsContext* context )
{
    a2dContext* ctext = ( a2dContext* ) context->GetNativeContext();
    ctext->SetFont( m_font );
}

//-----------------------------------------------------------------------------
// a2dPathData implementation
//-----------------------------------------------------------------------------

a2dPathData::a2dPathData( wxGraphicsRenderer* renderer, a2dVpath* path )
    : wxGraphicsPathData( renderer )
{
    if ( path )
        m_path = *path;
    else
        m_path = a2dVpath();

}

a2dPathData::~a2dPathData()
{
}

wxGraphicsObjectRefData* a2dPathData::Clone() const
{
    a2dVpath* p = new a2dVpath( m_path );
    return new a2dPathData( GetRenderer() , p );
}


void* a2dPathData::GetNativePath() const
{
    return ( void* ) &m_path;
}

void a2dPathData::UnGetNativePath( void* p ) const
{
}

//
// The Primitives
//

void a2dPathData::MoveToPoint( wxDouble x , wxDouble y )
{
    m_path.MoveTo( x, y );
}

void a2dPathData::AddLineToPoint( wxDouble x , wxDouble y )
{
    m_path.LineTo( x, y );
}

void a2dPathData::AddPath( const wxGraphicsPathData* path )
{
    a2dVpath* p = ( a2dVpath* )path->GetNativePath();
    m_path = *p;
}

void a2dPathData::CloseSubpath()
{
    m_path.back()->SetClose( a2dPATHSEG_END_CLOSED );
}

void a2dPathData::AddCurveToPoint( wxDouble cx1, wxDouble cy1, wxDouble cx2, wxDouble cy2, wxDouble x, wxDouble y )
{
    m_path.CBCurveTo( cx1, cy1, cx2, cy2, x, y );
}

// gets the last point of the current path, (0,0) if not yet set
void a2dPathData::GetCurrentPoint( wxDouble* x, wxDouble* y ) const
{
    double dx, dy;
    a2dVpathSegmentPtr seg = m_path.back();
    dx = seg->m_x1;
    dy = seg->m_y1;
    if ( x )
        *x = dx;
    if ( y )
        *y = dy;
}

void a2dPathData::AddArc( wxDouble xc, wxDouble yc, wxDouble radius, double startAngle, double endAngle, bool clockwise )
{
    if ( ( endAngle - startAngle ) >= 2 * M_PI )
    {
        double x1 = xc + radius;
        double y1 = yc;
        double x2 = xc + radius;
        double y2 = yc;
        double xm = xc - radius;
        double ym = yc;
        m_path.ArcTo( xc, yc, x2 , y2, xm, ym  );
    }
    else if ( clockwise || ( endAngle - startAngle ) >= 2 * M_PI )
    {
        double x1 = xc + radius * cos( startAngle );
        double y1 = yc + radius * sin( startAngle );
        double x2 = xc + radius * cos( endAngle );
        double y2 = yc + radius * sin( endAngle );

        //if going counterclockwise from begin to end we pass trough middle => counterclockwise
        //if going counterclockwise from begin to end we do NOT pass trough middle => clockwise
        double er;

        //rotate such that startAngle = zero
        er = endAngle - startAngle;
        //make positive
        if ( er <= 0 ) er += 2.0 * M_PI;
        if ( er >= 2.0 * M_PI ) er -= 2.0 * M_PI;

        double midrad;
        midrad = er / 2.0 + startAngle;

        double xm = xc + radius * cos( midrad );
        double ym = yc + radius * sin( midrad );

        //m_path.MoveTo( x1, y1 );
        m_path.ArcTo( xc, yc, x2 , y2, xm, ym  );
    }
    else
    {
        double x1 = xc + radius * cos( startAngle );
        double y1 = yc + radius * sin( startAngle );
        double x2 = xc + radius * cos( endAngle );
        double y2 = yc + radius * sin( endAngle );

        //if going counterclockwise from begin to end we pass trough middle => counterclockwise
        //if going counterclockwise from begin to end we do NOT pass trough middle => clockwise
        double er;

        //rotate such that startAngle = zero
        er = endAngle - startAngle;
        //make positive
        if ( er <= 0 ) er += 2.0 * M_PI;
        if ( er >= 2.0 * M_PI ) er -= 2.0 * M_PI;

        double midrad;
        midrad = ( 2.0 * M_PI - er / 2.0 );

        double xm = xc + radius * cos( midrad );
        double ym = yc + radius * sin( midrad );

        //m_path.MoveTo( x1, y1 );
        m_path.ArcTo( xc, yc, x2 , y2, xm, ym  );
    }
}

// transforms each point of this path by the matrix
void a2dPathData::Transform( const wxGraphicsMatrixData* matrix )
{
    // as we don't have a true path object, we have to apply the inverse
    // matrix to the context
    a2dAffineMatrix m = *( ( a2dAffineMatrix* ) matrix->GetNativeMatrix() );
    m_path.Transform( m );
}

// gets the bounding box enclosing all points (possibly including control points)
void a2dPathData::GetBox( wxDouble* x, wxDouble* y, wxDouble* w, wxDouble* h ) const
{
    double x1, y1, x2, y2;

    //cairo_stroke_extents( m_pathContext, &x1, &y1, &x2, &y2 );
    x1 = 0;
    y1 = 0;
    x2 = 1000;
    y2 = 1000;

    if ( x2 < x1 )
    {
        *x = x2;
        *w = x1 - x2;
    }
    else
    {
        *x = x1;
        *w = x2 - x1;
    }

    if( y2 < y1 )
    {
        *y = y2;
        *h = y1 - y2;
    }
    else
    {
        *y = y1;
        *h = y2 - y1;
    }
}

bool a2dPathData::Contains( wxDouble x, wxDouble y, wxPolygonFillMode fillStyle ) const
{
    return false;//cairo_in_stroke( m_pathContext, x, y) != 0;
}

//-----------------------------------------------------------------------------
// a2dMatrixData implementation
//-----------------------------------------------------------------------------

a2dMatrixData::a2dMatrixData( wxGraphicsRenderer* renderer, const a2dAffineMatrix* matrix )
    : wxGraphicsMatrixData( renderer )
{
    if ( matrix )
        m_matrix = *matrix;
}

a2dMatrixData::~a2dMatrixData()
{
    // nothing to do
}

wxGraphicsObjectRefData* a2dMatrixData::Clone() const
{
    return new a2dMatrixData( GetRenderer(), &m_matrix );
}

// concatenates the matrix
void a2dMatrixData::Concat( const wxGraphicsMatrixData* t )
{
    m_matrix *= *( ( a2dAffineMatrix* ) t->GetNativeMatrix() );
}

// sets the matrix to the respective values
void a2dMatrixData::Set( wxDouble a, wxDouble b, wxDouble c, wxDouble d,
                         wxDouble tx, wxDouble ty )
{
    m_matrix =  a2dAffineMatrix( a, b, c, d, tx, ty );
}

// gets the component valuess of the matrix
void a2dMatrixData::Get( wxDouble* a, wxDouble* b,  wxDouble* c,
                         wxDouble* d, wxDouble* tx, wxDouble* ty ) const
{
    if ( a )  *a = m_matrix( 0, 0 );
    if ( b )  *b = m_matrix( 0, 1 );
    if ( c )  *c = m_matrix( 1, 0 );
    if ( d )  *d = m_matrix( 1, 1 );
    if ( tx ) *tx = m_matrix( 2, 0 );
    if ( ty ) *ty = m_matrix( 2, 1 );
}

// makes this the inverse matrix
void a2dMatrixData::Invert()
{
    m_matrix.Invert();
}

// returns true if the elements of the transformation matrix are equal ?
bool a2dMatrixData::IsEqual( const wxGraphicsMatrixData* t ) const
{
    return m_matrix == *( ( a2dAffineMatrix* ) t->GetNativeMatrix() );
}

// return true if this is the identity matrix
bool a2dMatrixData::IsIdentity() const
{
    return m_matrix.IsIdentity();
}

//
// transformation
//

// add the translation to this matrix
void a2dMatrixData::Translate( wxDouble dx , wxDouble dy )
{
    m_matrix.Translate( dx, dy ) ;
}

// add the scale to this matrix
void a2dMatrixData::Scale( wxDouble xScale , wxDouble yScale )
{
    m_matrix.Scale( xScale, yScale, m_matrix( 2, 0 ), m_matrix( 2, 0 ) ) ;
}

// add the rotation to this matrix (radians)
void a2dMatrixData::Rotate( wxDouble angle )
{
    m_matrix.Rotate( angle ) ;
}

//
// apply the transforms
//

// applies that matrix to the point
void a2dMatrixData::TransformPoint( wxDouble* x, wxDouble* y ) const
{
    double lx = *x, ly = *y ;
    m_matrix.TransformPoint( lx, ly );
    *x = lx;
    *y = ly;
}

// applies the matrix except for translations
void a2dMatrixData::TransformDistance( wxDouble* dx, wxDouble* dy ) const
{
    double lx = *dx, ly = *dy ;
    //todo
    m_matrix.TransformDistance( lx, ly );
    *dx = lx;
    *dy = ly;
}

// returns the native representation
void* a2dMatrixData::GetNativeMatrix() const
{
    return ( void* ) &m_matrix;
}

//-----------------------------------------------------------------------------
// wxAggBitmap implementation
//-----------------------------------------------------------------------------

a2dBitmapData::a2dBitmapData( wxGraphicsRenderer* renderer, const wxBitmap& bmp ) : wxGraphicsObjectRefData( renderer )
{
    wxCHECK_RET( bmp.IsOk(), wxT( "Invalid bitmap in a2dBitmapData" ) );

    m_width = bmp.GetWidth();
    m_height = bmp.GetHeight();
    m_image = bmp.ConvertToImage();
}

a2dBitmapData::a2dBitmapData( wxGraphicsRenderer* renderer, const wxImage& image ) : wxGraphicsObjectRefData( renderer )
{
    wxCHECK_RET( image.IsOk(), wxT( "Invalid image in a2dBitmapData" ) );

    m_width = image.GetWidth();
    m_height = image.GetHeight();
    m_image = image;
}

a2dBitmapData::a2dBitmapData( wxGraphicsRenderer* renderer, const wxGraphicsBitmap& a2dbitmap ) : wxGraphicsObjectRefData( renderer )
{
    a2dBitmapData* data = ( a2dBitmapData* ) a2dbitmap.GetRefData();

    m_width = data->GetSize().GetWidth();
    m_height = data->GetSize().GetHeight();
    m_image = data->GetImage();
}

a2dBitmapData::~a2dBitmapData()
{
}

//-----------------------------------------------------------------------------
// a2dContext implementation
//-----------------------------------------------------------------------------

a2dContext::a2dContext( wxGraphicsRenderer* renderer, a2dContext* context )
    :
    m_width( context->m_width ),
    m_height( context->m_height ),
    m_drawstyle( a2dFILLED ),
    wxGraphicsContext( renderer )
{
    Init();
}

a2dContext::a2dContext( wxGraphicsRenderer* renderer, int width, int height )
    :
    m_width( width ),
    m_height( height ),
    m_drawstyle( a2dFILLED ),
    wxGraphicsContext( renderer )
{
    Init();
}

a2dContext::a2dContext( wxGraphicsRenderer* renderer )
    :
    m_width( 0 ),
    m_height( 0 ),
    m_drawstyle( a2dFILLED ),
    wxGraphicsContext( renderer )
{
    Init();
}

void a2dContext::Init()
{
    m_activestroke = *a2dBLACK_STROKE;
    m_activefill = *a2dBLACK_FILL;
    m_a2dfont = *a2dNullFONT;
    m_OpacityFactor = 255;
    m_displayaberration = 0.5;
    m_yaxis = false;
    m_drawingthreshold = a2dGlobals->GetPrimitiveThreshold();
}

/*
a2dContext::a2dContext( wxGraphicsRenderer* renderer, const wxImage& drawable )
:
    m_width( drawable.GetWidth() ),
    m_height( drawable.GetHeight() ),
    wxGraphicsContext(renderer)
{
}

a2dContext::a2dContext( wxGraphicsRenderer* renderer, wxWindow *window)
:
    m_width( window->GetSize().GetWidth() ),
    m_height( window->GetSize().GetHeight() ),
    wxGraphicsContext(renderer)
{
}
*/

a2dContext::~a2dContext()
{
    PopState();
    PopState();
}

void a2dContext::SetYaxis( bool up )
{
    m_yaxis = up;
}

void a2dContext::SetPen( const wxGraphicsPen& pen )
{
    wxGraphicsContext::SetPen( pen );

    if ( !pen.IsNull() )
        ( ( a2dStrokeData* )pen.GetRefData() )->Apply( this );
    else
        SetStroke( *a2dTRANSPARENT_STROKE );
}

void a2dContext::SetBrush( const wxGraphicsBrush& brush )
{
    wxGraphicsContext::SetBrush( brush );

    if ( !brush.IsNull() )
        ( ( a2dFillData* )brush.GetRefData() )->Apply( this );
    else
        SetFill( *a2dTRANSPARENT_FILL );
}

void a2dContext::SetFont( const wxGraphicsFont& font )
{
    wxGraphicsContext::SetFont( font );
    ( ( a2dFontData* ) font.GetRefData() )->Apply( this );
}

void a2dContext::SetStroke( const a2dStroke& stroke )
{
    switch( m_drawstyle )
    {
        case a2dWIREFRAME_INVERT:
        case a2dWIREFRAME:
        case a2dWIREFRAME_ZERO_WIDTH:
            m_activestroke = stroke;
            break;
        case a2dWIREFRAME_INVERT_ZERO_WIDTH:
            m_activestroke = *a2dBLACK_STROKE;
            break;
        case a2dFILLED:
            m_activestroke = stroke;
            break;
        case a2dFIX_STYLE:
            break;
        case a2dFIX_STYLE_INVERT:
            break;
        default:
            wxASSERT( 0 );
    }

    m_colour1redStroke = m_activestroke.GetColour().Red();
    m_colour1greenStroke  = m_activestroke.GetColour().Green();
    m_colour1blueStroke = m_activestroke.GetColour().Blue();

    if (  m_activestroke.IsNoStroke() || m_activestroke.GetStyle() == a2dSTROKE_TRANSPARENT )
    {
        m_StrokeOpacityCol1 = 0;
        m_StrokeOpacityCol2 = 0;
    }
    else
    {
        m_StrokeOpacityCol1 = m_activestroke.GetColour().Alpha() * m_OpacityFactor / 255 ;
        m_StrokeOpacityCol2 = m_activestroke.GetColour().Alpha() * m_OpacityFactor / 255 ;
    }
    DoSetActiveStroke();
}

void a2dContext::SetFill( const a2dFill& fill )
{
    switch( m_drawstyle )
    {
        case a2dWIREFRAME_INVERT:
        case a2dWIREFRAME:
        case a2dWIREFRAME_ZERO_WIDTH:
        case a2dWIREFRAME_INVERT_ZERO_WIDTH:
            m_activefill = *a2dTRANSPARENT_FILL;
            break;
        case a2dFILLED:
            m_activefill = fill;
            break;
        case a2dFIX_STYLE:
        case a2dFIX_STYLE_INVERT:
            break;
        default:
            wxASSERT( 0 );
    }

    m_colour1redFill = m_activefill.GetColour().Red();
    m_colour1greenFill  = m_activefill.GetColour().Green();
    m_colour1blueFill = m_activefill.GetColour().Blue();
    m_colour2redFill = m_activefill.GetColour2().Red();
    m_colour2greenFill  = m_activefill.GetColour2().Green();
    m_colour2blueFill = m_activefill.GetColour2().Blue();

    if ( !m_activefill.GetFilling() )
    {
        m_FillOpacityCol1 = 0;
        m_FillOpacityCol2 = 0;
    }
    else if ( m_activefill.GetStyle() == a2dFILL_TRANSPARENT )
    {
        m_FillOpacityCol1 = 0;
        m_FillOpacityCol2 = 0;
    }
    else if ( m_activefill.IsNoFill() )
    {
        m_FillOpacityCol1 = 0;
        m_FillOpacityCol2 = 0;
    }
    else
    {
        m_FillOpacityCol1 = m_activefill.GetColour().Alpha() * m_OpacityFactor / 255;
        m_FillOpacityCol2 = m_activefill.GetColour2().Alpha() * m_OpacityFactor / 255;
    }

    DoSetActiveFill();
}

void a2dContext::SetFont( const a2dFont& font )
{
    m_a2dfont = font;
}

void a2dContext::Clip( const wxRegion& region )
{
    // Create a path with all the rectangles in the region
    wxGraphicsPath path = GetRenderer()->CreatePath();
    wxRegionIterator ri( region );
    while ( ri )
    {
        path.AddRectangle( ri.GetX(), ri.GetY(), ri.GetW(), ri.GetH() );
        ri++;
    }

    //todo multiclipping render needed
}

void a2dContext::Clip( wxDouble x, wxDouble y, wxDouble w, wxDouble h )
{
    //need to calculate  clip in device coordinates from world coordinates
    // not even rectangular in the end, so assume a polygon as result.
    wxPoint trpoints[4];
    double dx, dy;

    m_usertodevice.TransformPoint( x, y, dx, dy );
    trpoints[0] = wxPoint( dx, dy );
    m_usertodevice.TransformPoint( x, y + h, dx, dy );
    trpoints[1] = wxPoint( dx, dy );
    m_usertodevice.TransformPoint( x + w, y + h, dx, dy );
    trpoints[2] = wxPoint( dx, dy );
    m_usertodevice.TransformPoint( x + w, y, dx, dy );
    trpoints[3] = wxPoint( dx, dy );

    double xmin = trpoints[0].x;
    double ymin = trpoints[0].y;
    double xmax = trpoints[0].x;
    double ymax = trpoints[0].y;
    for ( int i = 1; i < 4; i ++ )
    {
        xmin = wxMin( xmin, trpoints[i].x );
        xmax = wxMax( xmin, trpoints[i].x );
        ymin = wxMin( ymin, trpoints[i].y );
        ymax = wxMax( ymin, trpoints[i].y );
    }
    wxRect cliprect = wxRect( xmin, ymin, xmax - xmin, ymax - ymin );
    m_clipboxdev = cliprect;
}

void a2dContext::ResetClip()
{
    m_clipboxdev = wxRect( 0, 0, m_width, m_height );
}

void a2dContext::DrawPath( const wxGraphicsPath& path, wxPolygonFillMode fillStyle )
{
    if ( !m_pen.IsNull() || !m_brush.IsNull() )
    {
        a2dVpath* cp = ( a2dVpath* ) path.GetNativePath() ;
        /*
        if ( !m_pen.IsNull() )
            ((a2dStrokeData*)m_pen.GetRefData())->Apply(this);
        else
            SetStroke( *a2dTRANSPARENT_STROKE );
        if ( !m_brush.IsNull() )
            ((a2dFillData*)m_brush.GetRefData())->Apply(this);
        else
            SetFill( *a2dTRANSPARENT_FILL );
        */
        DrawVpath( cp );
    }
}

void a2dContext::StrokePath( const wxGraphicsPath& path )
{
    if ( !m_pen.IsNull() )
    {
        a2dVpath* cp = ( a2dVpath* ) path.GetNativePath() ;
        //((a2dStrokeData*)m_pen.GetRefData())->Apply(this);

        a2dFill curfill = m_activefill;
        SetFill( *a2dTRANSPARENT_FILL );
        DrawVpath( cp );
        SetFill( curfill );
    }
}

void a2dContext::FillPath( const wxGraphicsPath& path , wxPolygonFillMode fillStyle )
{
    if ( !m_brush.IsNull() )
    {
        a2dVpath* cp = ( a2dVpath* ) path.GetNativePath() ;
        //((a2dFillData*)m_brush.GetRefData())->Apply(this);

        a2dStroke curstroke = m_activestroke;
        m_activestroke = *a2dTRANSPARENT_STROKE;
        DrawVpath( cp );
        m_activestroke = curstroke;
    }
}

void a2dContext::Rotate( wxDouble angle )
{
    m_usertodevice.Rotate( angle ) ;
}

void a2dContext::Translate( wxDouble dx , wxDouble dy )
{
    m_usertodevice.Translate( dx, dy ) ;
}

void a2dContext::Scale( wxDouble xScale , wxDouble yScale )
{
    m_usertodevice.Scale( xScale, yScale, 0, 0 ) ;
}

// concatenates this transform with the current transform of this context
void a2dContext::ConcatTransform( const wxGraphicsMatrix& matrix )
{
    a2dAffineMatrix* m = ( a2dAffineMatrix* ) matrix.GetNativeMatrix();
    m_usertodevice *= *m;
}

// sets the transform of this context
void a2dContext::SetTransform( const wxGraphicsMatrix& matrix )
{
    a2dAffineMatrix* m = ( a2dAffineMatrix* ) matrix.GetNativeMatrix();
    m_usertodevice = *m;
}

// gets the matrix of this context
wxGraphicsMatrix a2dContext::GetTransform() const
{
    wxGraphicsMatrix matrix = CreateMatrix();
    a2dAffineMatrix* m = ( a2dAffineMatrix* ) matrix.GetNativeMatrix();
    *m = m_usertodevice;
    return matrix;
}

void a2dContext::PushState()
{
}

void a2dContext::PopState()
{
}

#if wxCHECK_VERSION(2,9,0)
void a2dContext::DrawBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h )
#else
void a2dContext::DrawGraphicsBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h )
#endif
{
    a2dBitmapData* data = static_cast<a2dBitmapData*>( bmp.GetRefData() );
    wxImage image = data->GetImage();
}

void a2dContext::DrawBitmap( const wxBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h )
{
    wxGraphicsBitmap bitmap = GetRenderer()->CreateBitmap( bmp );
#if wxCHECK_VERSION(2,9,0)
    DrawBitmap( bitmap, x, y, w, h );
#else
    DrawGraphicsBitmap( bitmap, x, y, w, h );
#endif
}

void a2dContext::DrawIcon( const wxIcon& icon, wxDouble x, wxDouble y, wxDouble w, wxDouble h )
{
    //DrawBitmap(icon, x, y, w, h);
}

#if wxCHECK_VERSION(2,9,0)
void a2dContext::DoDrawText( const wxString& str, wxDouble x, wxDouble y )
#else
void a2dContext::DrawText( const wxString& str, wxDouble x, wxDouble y )
#endif
{
    if ( str.empty() )
        return;

    //const wxWX2MBbuf buf(str.mb_str(wxConvUTF8));
    //wxWCharBuffer s = str.wc_str( *wxConvUI );

    // Get the bounding box, including alignment displacement.
    a2dBoundingBox bbox = m_a2dfont.GetTextExtent( str, wxMINX | wxMINY, false );
    bbox.Translate( x, y );

    // Get unaligned coordinates for DrawTextXXX functions. By default these functions
    // use the lowerleft corner of the boundingbox, hence GetMinX() and GetMinY().
    x = bbox.GetMinX();
    y = bbox.GetMinY();

    // Get font size in device units, also takes stretching into account
    double dx, dy, size;
    dx = m_usertodevice.GetValue( 1, 0 );
    dy = m_usertodevice.GetValue( 1, 1 );
    size = m_a2dfont.GetSize() * sqrt( dx * dx + dy * dy );

    // Transform user bbox to world bbox.
    a2dBoundingBox worldbbox( bbox );
    worldbbox.MapBbox( m_usertodevice );
    wxRect devb = wxRect( worldbbox.GetMinX(), worldbbox.GetMinY(), worldbbox.GetWidth(), worldbbox.GetHeight() );

    // If bbox not outside clipping area, draw the text
    if ( m_clipboxdev.Intersects( devb ) )
    {
        // add this to see tranformed boundingbox.
        //DrawRoundedRectangle( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth(), bbox.GetHeight(), 0 );

        // If text > threshold, draw text
        if ( size > m_drawingthreshold )
        {
            // Try to find the appropriate drawing function.
            switch ( m_a2dfont.GetType() )
            {
                case a2dFONT_STROKED:
                    DrawTextStroke( str, x, y );
                    break;
                case a2dFONT_FREETYPE:
                    DrawTextFreetype( str, x, y );
                    break;
                case a2dFONT_WXDC:
                    DrawTextDc( str, x, y );
                    break;
                default:
                    DrawTextUnknown( str, x, y );
            }
        }
        // If text < threshold, draw a simple version.
        else
            DrawTextUnknown( str, x, y, true );
    }
}


void a2dContext::GetTextExtent( const wxString& str, wxDouble* width, wxDouble* height,
                                wxDouble* descent, wxDouble* externalLeading ) const
{
    if ( str.empty() )
        return;

    if ( width )
    {
        const wxWX2MBbuf buf( str.mb_str( wxConvUTF8 ) );
    }

    if ( height || descent || externalLeading )
    {
    }
}

void a2dContext::GetPartialTextExtents( const wxString& text, wxArrayDouble& widths ) const
{
    widths.Empty();
    widths.Add( 0, text.length() );

    if ( text.empty() )
        return;

    // TODO
}

void* a2dContext::GetNativeContext()
{
    return this;
}

//------------------------------------------------------------------------
// basic platform independent drawing of primitives
//------------------------------------------------------------------------
int a2dContext::ToDeviceLines( a2dVertexArray* points, a2dBoundingBox& devbbox, bool& smallPoly, bool replaceByRectangle )
{
    unsigned int segments = 0;

    unsigned int i;
    for ( i = 0; i < points->size(); i++ )
    {
        const a2dLineSegmentPtr seg = points->operator[]( i );

        if ( seg->GetArc() )
        {
            const a2dArcSegment* cseg = ( const a2dArcSegment* ) ( seg.Get() );

            double radius, center_x, center_y, beginrad, midrad, endrad, phit;

            if ( cseg->CalcR( *( points->operator[]( i ? i - 1 : 0 ) ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
            {
                double dphi;
                unsigned int circlesegments = 20;
                double radiusDev = m_usertodevice.TransformDistance( radius );
                Aberration( m_displayaberration, phit, radiusDev , dphi, circlesegments );
                segments += circlesegments + 1;
            }
            else
                segments ++;
        }
        else
            segments++;
    }

    //TODO memory error because of wrong calculation of segments?? for the moment *2 to be save
    m_cpointsDouble.resize( segments * 2 );

    unsigned int count = 0;
    double x, y, lastx, lasty;
    for ( i = 0; i < points->size(); i++ )
    {
        const a2dLineSegmentPtr seg = points->operator[]( i );

        if ( !seg->GetArc() )
        {
            m_usertodevice.TransformPoint( seg->m_x, seg->m_y, x, y );

            //to reduce the points count for lines or a polygon on the screen
            if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
            {
                m_cpointsDouble[count].x = x;
                m_cpointsDouble[count].y = y;
                lastx = x;
                lasty = y;
                devbbox.Expand( x, y );
                count++;
            }
        }
        else
        {
            const a2dArcSegment* cseg = ( const a2dArcSegment* ) ( seg.Get() );

            double radius, center_x, center_y, beginrad, midrad, endrad, phit;

            if ( cseg->CalcR( *( points->operator[]( i ? i - 1 : 0 ) ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
            {
                double dphi;
                unsigned int segments = 20;
                double radiusDev = m_usertodevice.TransformDistance( radius );
                Aberration( m_displayaberration, phit, radiusDev , dphi, segments );

                double theta = beginrad;
                unsigned int step;

                double x, y;
                for ( step = 0; step < segments + 1; step++ )
                {
                    m_usertodevice.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );

                    //to reduce the points count for lines or a polygon on the screen
                    if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                    {
                        m_cpointsDouble[count].x = x;
                        m_cpointsDouble[count].y = y;
                        lastx = x;
                        lasty = y;
                        devbbox.Expand( x, y );
                        count++;
                    }
                    theta = theta + dphi;
                }
            }
            else
            {
                double x, y;
                m_usertodevice.TransformPoint( cseg->m_x, cseg->m_y, x, y );

                //to reduce the points count for lines or a polygon on the screen
                if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                {
                    m_cpointsDouble[count].x = x;
                    m_cpointsDouble[count].y = y;
                    lastx = x;
                    lasty = y;
                    devbbox.Expand( x, y );
                    count++;
                }
            }
        }
    }

    return count;
}

int a2dContext::ToDeviceLines( const a2dVertexList* list, a2dBoundingBox& devbbox, bool& smallPoly, bool replaceByRectangle )
{
    unsigned int segments = 0;

    if ( list->empty() )
        return 0;

    a2dVertexList::const_iterator iterprev = list->end();
    if ( iterprev != list->begin() )
        iterprev--;
    a2dVertexList::const_iterator iter = list->begin();
    while ( iter != list->end() )
    {
        a2dLineSegment* seg = ( *iter );

        if ( seg->GetArc() )
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;

            double radius, center_x, center_y, beginrad, midrad, endrad, phit;

            if ( cseg->CalcR( *( *iterprev ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
            {
                double dphi;
                unsigned int circlesegments = 20;
                double radiusDev = m_usertodevice.TransformDistance( radius );
                Aberration( m_displayaberration, phit, radiusDev , dphi, circlesegments );
                segments += circlesegments + 1;
            }
            else
                segments ++;
        }
        else
            segments++;

        iterprev = iter++;
    }

    //TODO memory error because of wrong calculation of segments?? for the moment *2 to be save
    m_cpointsDouble.resize( segments * 2 );

    unsigned int count = 0;
    double x, y, lastx, lasty;
    iterprev = list->end();
    if ( iterprev != list->begin() )
        iterprev--;
    iter = list->begin();
    while ( iter != list->end() )
    {
        a2dLineSegment* seg = ( *iter );

        if ( !seg->GetArc() )
        {
            m_usertodevice.TransformPoint( seg->m_x, seg->m_y, x, y );

            //to reduce the points count for lines or a polygon on the screen
            if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
            {
                m_cpointsDouble[count].x = x;
                m_cpointsDouble[count].y = y;
                lastx = x;
                lasty = y;
                devbbox.Expand( x, y );
                count++;
            }
        }
        else
        {
            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;

            double radius, center_x, center_y, beginrad, midrad, endrad, phit;

            if ( cseg->CalcR( *( *iterprev ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
            {
                double dphi;
                unsigned int segments = 20;
                double radiusDev = m_usertodevice.TransformDistance( radius );
                Aberration( m_displayaberration, phit, radiusDev , dphi, segments );

                double theta = beginrad;
                unsigned int step;

                double x, y;
                for ( step = 0; step < segments + 1; step++ )
                {
                    m_usertodevice.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );

                    //to reduce the points count for lines or a polygon on the screen
                    if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                    {
                        m_cpointsDouble[count].x = x;
                        m_cpointsDouble[count].y = y;
                        lastx = x;
                        lasty = y;
                        devbbox.Expand( x, y );
                        count++;
                    }
                    theta = theta + dphi;
                }
            }
            else
            {
                double x, y;
                m_usertodevice.TransformPoint( cseg->m_x, cseg->m_y, x, y );

                //to reduce the points count for lines or a polygon on the screen
                if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                {
                    m_cpointsDouble[count].x = x;
                    m_cpointsDouble[count].y = y;
                    lastx = x;
                    lasty = y;
                    devbbox.Expand( x, y );
                    count++;
                }
            }
        }
        iterprev = iter++;
    }

    return count;
}


void a2dContext::DrawVpath( const a2dVpath* path )
{
    double tstep = 1 / ( double ) SPLINE_STEP;
    unsigned int i;

    a2dVertexList points;
    bool move = false;
    int count = 0;
    bool nostrokeparts = false;

    //first draw as much as possible ( nostroke parts may stop this first round )
    for ( i = 0; i < path->size(); i++ )
    {
        a2dVpathSegmentPtr seg = path->operator[]( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_MOVETO:
                if ( count == 0 )
                {
                    points.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
                    count++;
                }
                else
                {
                    i--;
                    move = true;
                }
                break;

            case a2dPATHSEG_LINETO:
                points.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
                count++;
                break;
            case a2dPATHSEG_LINETO_NOSTROKE:
                points.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
                count++;
                nostrokeparts = true;
                break;

            case a2dPATHSEG_CBCURVETO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_CBCURVETO:
            {
                double xw, yw;

                double xwl = path->operator[]( i ? i - 1 : 0 )->m_x1;
                double ywl = path->operator[]( i ? i - 1 : 0 )->m_y1;
                a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) ( seg.Get() );

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                    yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
                    points.push_back( new a2dLineSegment( xw, yw ) );
                    count++;
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_QBCURVETO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_QBCURVETO:
            {
                double xw, yw;

                double xwl = path->operator[]( i ? i - 1 : 0 )->m_x1;
                double ywl = path->operator[]( i ? i - 1 : 0 )->m_y1;
                a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                    yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
                    points.push_back( new a2dLineSegment( xw, yw ) );
                    count++;
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_ARCTO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_ARCTO:
            {
                a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();

                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                if ( cseg->CalcR( path->operator[]( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                {
                    double dphi;
                    unsigned int segments = 20;
                    double radiusDev = m_usertodevice.TransformDistance( radius );
                    Aberration( m_displayaberration, phit, radiusDev , dphi, segments );

                    double theta = beginrad;
                    unsigned int step;

                    for ( step = 0; step < segments + 1; step++ )
                    {
                        points.push_back( new a2dLineSegment( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ) ) );
                        count++;
                        theta = theta + dphi;
                    }
                }
                else
                {
                    points.push_back( new a2dLineSegment( cseg->m_x1, cseg->m_y1 ) );
                    count++;
                }
            }
            default:
                break;
                break;
        }

        if ( move )
        {
            DrawLines( &points );
            move = false;
            count = 0;
            points.clear();
        }
        else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
        {
            if ( nostrokeparts || seg->GetClose() == a2dPATHSEG_END_CLOSED_NOSTROKE )
            {
                a2dStroke stroke = m_activestroke;
                m_activestroke = *a2dTRANSPARENT_STROKE;
                DrawPolygon( &points, wxODDEVEN_RULE );
                m_activestroke = stroke;
                nostrokeparts = true;
            }
            else
            {
                DrawPolygon( &points, wxODDEVEN_RULE );
            }

            move = false;
            count = 0;
            points.clear();
        }
        else if ( i == path->size() - 1 )  //last segment?
        {
            DrawLines( &points );
        }
    }

    if ( nostrokeparts )
    {
        points.clear();
        move = false;
        count = 0;
        nostrokeparts = false;

        double lastmovex = 0;
        double lastmovey = 0;

        for ( i = 0; i < path->size(); i++ )
        {
            a2dVpathSegmentPtr seg = path->operator[]( i );
            switch ( seg->GetType()  )
            {
                case a2dPATHSEG_MOVETO:
                    if ( count == 0 )
                    {
                        points.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
                        lastmovex = seg->m_x1;
                        lastmovey = seg->m_y1;
                        count++;
                    }
                    else
                    {
                        i--;
                        move = true;
                    }
                    break;

                case a2dPATHSEG_LINETO:
                    points.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
                    count++;
                    break;

                case a2dPATHSEG_LINETO_NOSTROKE:
                case a2dPATHSEG_CBCURVETO_NOSTROKE:
                case a2dPATHSEG_QBCURVETO_NOSTROKE:
                case a2dPATHSEG_ARCTO_NOSTROKE:
                    if ( count == 0 )
                    {
                        points.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
                        count++;
                    }
                    else
                    {
                        i--;
                        nostrokeparts = true;
                    }
                    break;

                case a2dPATHSEG_CBCURVETO:
                {
                    double xw, yw;

                    double xwl = path->operator[]( i ? i - 1 : 0 )->m_x1;
                    double ywl = path->operator[]( i ? i - 1 : 0 )->m_y1;
                    a2dVpathCBCurveSegment& cseg = ( a2dVpathCBCurveSegment& ) seg;

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 3 ) + cseg.m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg.m_x3 * ( 1 - t ) * t * t * 3 + cseg.m_x1 * pow( t, 3 );
                        yw = ywl * pow( 1 - t, 3 ) + cseg.m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg.m_y3 * ( 1 - t ) * t * t * 3 + cseg.m_y1 * pow( t, 3 );
                        points.push_back( new a2dLineSegment( xw, yw ) );
                        count++;
                        t = t + tstep;
                    }
                }
                break;

                case a2dPATHSEG_QBCURVETO:
                {
                    double xw, yw;

                    double xwl = path->operator[]( i ? i - 1 : 0 )->m_x1;
                    double ywl = path->operator[]( i ? i - 1 : 0 )->m_y1;
                    a2dVpathQBCurveSegment& cseg = ( a2dVpathQBCurveSegment& ) seg;

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 2 ) + cseg.m_x2 * ( 1 - t ) * t * 2 + cseg.m_x1 * pow( t, 2 );
                        yw = ywl * pow( 1 - t, 2 ) + cseg.m_y2 * ( 1 - t ) * t * 2 + cseg.m_y1 * pow( t, 2 );
                        points.push_back( new a2dLineSegment( xw, yw ) );
                        count++;
                        t = t + tstep;
                    }
                }
                break;

                case a2dPATHSEG_ARCTO:
                {
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();

                    double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                    if ( cseg->CalcR( path->operator[]( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                    {
                        double dphi;
                        unsigned int segments = 20;
                        double radiusDev = m_usertodevice.TransformDistance( radius );
                        Aberration( m_displayaberration, phit, radiusDev , dphi, segments );

                        double theta = beginrad;
                        unsigned int step;

                        for ( step = 0; step < segments + 1; step++ )
                        {
                            points.push_back( new a2dLineSegment( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ) ) );
                            count++;
                            theta = theta + dphi;
                        }
                    }
                    else
                    {
                        points.push_back( new a2dLineSegment( cseg->m_x1, cseg->m_y1 ) );
                        count++;
                    }
                }
                default:
                    break;

                    break;
            }

            if ( move || nostrokeparts )
            {
                DrawLines( &points );
                move = false;
                nostrokeparts = false;
                points.clear();
                count = 0;
            }
            else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
            {
                if ( seg->GetClose() == a2dPATHSEG_END_CLOSED )
                {
                    points.push_back( new a2dLineSegment( lastmovex, lastmovey ) );
                    count++;
                }
                DrawLines( &points );
                nostrokeparts = false;
                move = false;
                points.clear();
                count = 0;
            }
            else if ( i == path->size() )
            {
                DrawLines( &points );
            }
        }
    }

}

void a2dContext::DrawCharUnknown( wxChar c )
{
    a2dStroke oldstroke = m_activestroke;
    // set text linestroke.
    a2dStroke stroke = a2dStroke( m_activestroke.GetColour(), m_a2dfont.GetSize() / 20.0 );
    m_activestroke = stroke;

    if ( c != wxChar( ' ' ) )
    {
        double desc = m_a2dfont.GetDescent();
        double size = m_a2dfont.GetSize();
        double width = m_a2dfont.GetWidth( c );
        DrawLine( 0.1 * width, desc, 0.9 * width, desc + size );
        DrawLine( 0.9 * width, desc, 0.1 * width, desc + size );
    }

    // restore context
    m_activestroke = oldstroke;
}

void a2dContext::DrawTextUnknown( const wxString& text, double x, double y, bool words )
{
    if ( words && m_a2dfont.Ok() )
    {
        a2dStroke oldstroke = m_activestroke;
        // Draw a piece of line for each word.
        a2dBoundingBox linebbox = m_a2dfont.GetTextExtent( text, m_yaxis );

        if ( !m_activefill.IsNoFill() && !m_activefill.GetStyle() == a2dFILL_TRANSPARENT )
        {
            m_activestroke = *a2dTRANSPARENT_STROKE;
            DrawRoundedRectangle( x + linebbox.GetMinX(), y + linebbox.GetMinY(),
                                  linebbox.GetWidth(), linebbox.GetHeight(), 0 );
            m_activestroke = oldstroke;
        }

        // set text linestroke.
        a2dStroke strokew = a2dStroke( m_activestroke.GetColour(), 1 );
        m_activestroke = strokew;

        double h = linebbox.GetHeight();
        double w;
        wxChar c;
        double start = x;
        for ( size_t i = 0; i < text.Length(); i++ )
        {
            c = text[i];
            w = m_a2dfont.GetWidth( c );
            if ( c == wxChar( ' ' ) )
            {
                if ( x != start )
                    DrawLine( start, y + h / 2, x, y + h / 2 );
                start = x + w;
            }
            x += w;
        }
        if ( x != start )
            DrawLine( start, y + h / 2, x, y + h / 2 );

        // restore context
        m_activestroke = oldstroke;
    }
    else
    {
        a2dBoundingBox linebbox = m_a2dfont.GetTextExtent( text, m_yaxis );

        DrawRoundedRectangle( x + linebbox.GetMinX(), y + linebbox.GetMinY(),
                              linebbox.GetWidth(), linebbox.GetHeight(), 0 );
        DrawLine( x + linebbox.GetMinX(), y + linebbox.GetMinY(), x + linebbox.GetMaxX(), y + linebbox.GetMaxY() );
        DrawLine( x + linebbox.GetMinX(), y + linebbox.GetMaxY(), x + linebbox.GetMaxX(), y + linebbox.GetMinY() );
    }
}


void a2dContext::DrawCharStroke( wxChar c )
{
    a2dStroke strokeold = m_activestroke;
    // set text linestroke.
    //!todo what about the style (pixel or not of stroke font )
    a2dStroke stroke = a2dStroke( m_activestroke.GetColour(), 0.0 );
    stroke.SetWidth( m_a2dfont.GetStrokeWidth() );
    m_activestroke = stroke;

    a2dAffineMatrix affineold = m_usertodevice;

    // scale character to size
    a2dAffineMatrix affine;
    affine.Scale( m_a2dfont.GetSize() );
    m_usertodevice = affine;

    a2dVertexList** ptr = m_a2dfont.GetGlyphStroke( c );
    if ( ptr )
    {
        while ( *ptr )
        {
            DrawLines( *ptr );
            ptr++;
        }
    }

    m_usertodevice = affineold;
    m_activestroke = strokeold;
}

void a2dContext::DrawTextGeneric( const wxString& text, double x, double y, void ( a2dContext::*drawchar )( wxChar ) )
{
    // mirror text, depending on y-axis orientation
    const double h = m_a2dfont.GetLineHeight();
    a2dAffineMatrix affine;
    if ( !GetYaxis() )
    {
        affine.Translate( 0.0, -h );
        affine.Mirror( true, false );
    }
    // position text
    affine.Translate( x, y );

    a2dAffineMatrix affineold = m_usertodevice;
    a2dAffineMatrix textm = affine;

    m_usertodevice = m_usertodevice;

    bool textwasvisible = false;

    double w = 0.0;
    wxChar c = 0;
    const size_t n = text.Length();
    for ( size_t i = 0; i < n; i++ )
    {
        const wxChar oldc = c;
        c = text[i];
        if ( i > 0 )
        {
            textm *= a2dAffineMatrix( w + m_a2dfont.GetKerning( oldc, c ), 0.0 );
        }
        m_usertodevice = affineold * textm;

        w = m_a2dfont.GetWidth( c );

        // get absolute character bounding box
        a2dBoundingBox bbox( 0.0, 0.0, w, h );
        bbox.MapBbox( m_usertodevice );
        wxRect devb = wxRect( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth(), bbox.GetHeight() );

        // if character is not outside clipping box, draw it.
        if ( m_clipboxdev.Intersects( devb ) )
        {
            ( this->*drawchar )( c );
            textwasvisible = true;
        }
        else if ( textwasvisible )
        {
            // If characters of a string had been visible, and this character is not
            // visible, then so will all succeeding. i.o.w. we can stop drawing.
            break;
        }
    }

    // restore context
    m_usertodevice = affineold;
}

//use by polygon filling
//moves the scanline up
//index is the index of the point where the search begins
//direction is +1 or -1 and indicates if the segment ascends or decends
bool a2dContext::MoveUp( int n, wxRealPoint points[] , double horline, int& index, int direction )
{
    int walk = ( index + direction + n ) % n;
    while ( points[walk].y < horline )
    {
        if ( points[walk].y < points[index].y )
            return false;
        else
        {
            //modify index
            index = walk;
            walk = ( index + direction + n ) % n;
        }
    }
    return true;
}

//a crictical point is a point between a decending and a ascending segment
//collect those points for filling later
void a2dContext::DetectCriticalPoints( int n, wxRealPoint points[] )
{
    //candidate for critical point
    //true if Y is getting lower, unchanged i Y is unchanged
    //and if Y becomes higher and candidate was true: it is a critical point
    bool candidate = false;
    int i, j;

    for ( i = 0; i < n; i++ )
    {
        //j next point
        j =  ( i + 1 ) % n;

        //check if Y is smaller
        if ( points[i].y > points[j].y )
            //we have a candidate
            candidate = true;
        else if ( ( points[i].y < points[j].y ) && candidate )
        {
            //this is a critical point put in list
            bool inserted = false;
            a2dCriticalPointList::compatibility_iterator node = m_CRlist.GetFirst();
            while ( node )
            {
                //sorted on smallest Y value
                int* ind = node->GetData();
                if ( points[*ind].y > points[i].y )
                {
                    m_CRlist.Insert( node, new int( i ) );
                    inserted = true;
                    break;
                }
                node = node->GetNext();
            }
            if ( !inserted )
                m_CRlist.Append( new int( i ) );
            candidate = false;
        }
    }
    if ( candidate )
    {
        for ( i = 0; i < n; i++ )
        {
            //j next point
            j =  ( i + 1 ) % n;

            //check if Y is smaller
            if ( points[i].y > points[j].y )
                //we have a candidate
                candidate = true;
            else if ( ( points[i].y < points[j].y ) && candidate )
            {
                //this is a critical point put in list
                bool inserted = false;
                a2dCriticalPointList::compatibility_iterator node = m_CRlist.GetFirst();
                while ( node )
                {
                    //sorted on smallest Y value
                    int* ind = node->GetData();
                    if ( points[*ind].y > points[i].y )
                    {
                        m_CRlist.Insert( node, new int( i ) );
                        inserted = true;
                        break;
                    }
                    node = node->GetNext();
                }
                if ( !inserted )
                    m_CRlist.Append( new int( i ) );
                candidate = false;
            }
        }
    }
}


static int SortonXs( const a2dAET** first, const a2dAET** second )
{
    if ( ( *first )->m_xs < ( *second )->m_xs )
        return -1;
    else if ( ( *first )->m_xs > ( *second )->m_xs )
        return 1;

    return 0;
}

void a2dContext::ColourXYLinear( int x1, int x2, int y )
{
    int dred = m_colour2redFill - m_colour1redFill;
    int dgreen = m_colour2greenFill - m_colour1greenFill;
    int dblue = m_colour2blueFill - m_colour1blueFill;

    if ( m_dx1 == m_dx2 )
    {
        //total number of lines to go from m_textbg to m_textfg
        //gives the following number of steps for the gradient color
        double stepcol = m_max_y - m_min_y;

        double curcol = y - m_min_y;

        a2dStroke current = m_activestroke;
        wxColour gradcol( ( unsigned char ) ( m_colour1redFill + dred * curcol / stepcol ),
                          ( unsigned char ) ( m_colour1greenFill + dgreen * curcol / stepcol ),
                          ( unsigned char ) ( m_colour1blueFill + dblue * curcol / stepcol ) );
        a2dStroke gradientstroke = a2dStroke( gradcol, 0 );
        m_activestroke = gradientstroke;
        DeviceDrawHorizontalLine( x1, y, x2, true );
        m_activestroke = current;
    }
    else
    {
        int j;
        for ( j = x1; j <= x2; j++ )
        {
            a2dLine perstroke( j, y, j + m_dy2 - m_dy1, y + m_dx2 - m_dx1 );
            a2dPoint2D crossing;
            perstroke.CalculateLineParameters();
            perstroke.Intersect( m_line, crossing );

            double length = sqrt(  ( double ) ( crossing.m_x - m_dx1 ) * ( crossing.m_x - m_dx1 ) + ( crossing.m_y - m_dy1 ) * ( crossing.m_y - m_dy1 ) );

            if ( length > m_length )
                length = m_length;

            DeviceDrawPixel( j, y, m_colour1redFill   + dred * int( length / m_length ),
                             m_colour1greenFill + dgreen * int( length / m_length ),
                             m_colour1blueFill  + dblue * int( length / m_length ) );
        }
    }
}

void a2dContext::ColourXYRadial( int x1, int x2, int y )
{
    int dred = m_colour2redFill - m_colour1redFill;
    int dgreen = m_colour2greenFill - m_colour1greenFill;
    int dblue = m_colour2blueFill - m_colour1blueFill;

    int j;
    for ( j = x1; j <= x2; j++ )
    {
        double cradius = sqrt(  ( double ) ( j - m_dx1 ) * ( j - m_dx1 ) + ( y - m_dy1 ) * ( y - m_dy1 ) );

        double delta;
        if ( m_radiusd == 0 )
            delta = 0;
        else if ( cradius > m_radiusd )
            delta = 1;
        else
            delta = cradius / m_radiusd;

        DeviceDrawPixel( j, y, ( unsigned char ) ( m_colour1redFill   + dred * delta ),
                         ( unsigned char ) ( m_colour1greenFill + dgreen * delta ),
                         ( unsigned char ) ( m_colour1blueFill  + dblue * delta ) );
    }
}

void a2dContext::FillPolygon( int n, wxRealPoint points[] )
{
    wxRect clip;
    clip.x = m_clipboxdev.x;
    clip.y = m_clipboxdev.y;
    clip.width = m_clipboxdev.width;
    clip.height = m_clipboxdev.height;

    m_min_y = points[0].y;
    m_max_y = points[0].y;
    m_min_x = points[0].x;
    m_max_x = points[0].x;
    double  centroidx = 0;
    double  centroidy = 0;
    int i;
    for ( i = 0; i < n; i++ )
    {
        m_min_y = wxMin( m_min_y, points[i].y );
        m_max_y = wxMax( m_max_y, points[i].y );
        m_min_x = wxMin( m_min_x, points[i].x );
        m_max_x = wxMax( m_max_x, points[i].x );
        centroidx += points[i].x;
        centroidy += points[i].y;
    }

    centroidx /= double( n );
    centroidy /= double( n );

    int index;

    if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
    {
        double x1 = m_activefill.GetStart().m_x;
        double y1 = m_activefill.GetStart().m_y;
        double x2 = m_activefill.GetStop().m_x;
        double y2 = m_activefill.GetStop().m_y;
        m_usertodevice.TransformPoint( x1, y1, m_dx1, m_dy1 );
        m_usertodevice.TransformPoint( x2, y2, m_dx2, m_dy2 );
        if ( m_dx1 == m_dx2 && m_dy1 == m_dy2 )
        {
            //do a linear fill vertical
            m_dx1 = centroidx;
            m_dy1 = m_min_y;
            m_dx2 = centroidx;
            m_dy2 = m_max_y;
            if ( m_dx1 == m_dx2 && m_dy1 == m_dy2 )
                m_dx2 = m_dx2 + 100;
        }
        m_length = sqrt ( ( m_dx1 - m_dx2 ) * ( m_dx1 - m_dx2 ) + ( m_dy1 - m_dy2 ) * ( m_dy1 - m_dy2 ) );
        m_line = a2dLine( m_dx1, m_dy1, m_dx2, m_dy2 );
        m_line.CalculateLineParameters();
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        double xfc = m_activefill.GetFocal().m_x;
        double yfc = m_activefill.GetFocal().m_y;
        double xe = m_activefill.GetCenter().m_x;
        double ye = m_activefill.GetCenter().m_y;
        m_usertodevice.TransformPoint( xfc, yfc, m_dx1, m_dy1 );
        m_usertodevice.TransformPoint( xe, ye, m_dx2, m_dy2 );
        if ( m_dx1 == m_dx2 && m_dy1 == m_dy2 )
        {
            //no start stop given
            m_dx1 = centroidx;
            m_dy1 = centroidy;
            m_dx2 = centroidx;
            m_dy2 = m_max_y;

            if ( m_dx1 == m_dx2 && m_dy1 == m_dy2 )
                m_dx2 = m_dx2 + 10;
        }
        double dx3, dy3;
        m_usertodevice.TransformPoint( xe, ye + m_activefill.GetRadius(), dx3, dy3 );
        m_radiusd = sqrt ( ( dx3 - m_dx2 ) * ( dx3 - m_dx2 ) + ( dy3 - m_dy2 ) * ( dy3 - m_dy2 ) );
    }

    DetectCriticalPoints( n, points );

    int min = ( int ) wxMax ( m_min_y, clip.y );
    int max = ( int ) wxMin ( m_max_y, clip.y + clip.height );

    for ( i = min; i < max; i++ )
    {
        a2dAETList::compatibility_iterator node = m_AETlist.GetFirst();
        int count =  m_AETlist.size();
        while ( count > 0 )
        {
            a2dAET* ele = node->GetData();
            index =  ele->m_index;
            int direction = ele->m_direction;
            if ( !MoveUp( n, points, i, index, direction ) )
            {
                a2dAETList::compatibility_iterator h = node;
                //remove this node
                node = node->GetNext();
                delete h->GetData();
                m_AETlist.DeleteNode( h );
            }
            else
            {
                if ( ele->m_index != index )
                {
                    ele->m_index = index;
                    int h = ( index + direction + n ) % n;
                    ele->CalculateLineParameters( points[h], points[index] );
                }
                if ( ele->m_horizontal )
                    ele->m_xs = ( int ) points[index].x;
                else
                    ele->CalculateXs( i );
                node = node->GetNext();
            }
            count--;
        }

        a2dCriticalPointList::compatibility_iterator nodec = m_CRlist.GetFirst();
        while ( m_CRlist.size() && points[*nodec->GetData()].y <= i )
        {
            int DI;
            for ( DI = -1; DI <= 1 ; DI += 2 )
            {
                index = *nodec->GetData();
                if ( MoveUp( n, points, i, index, DI ) )
                {
                    a2dAET* ele = new a2dAET();
                    ele->m_index = index;
                    ele->m_direction = DI;
                    int h = ( index + DI + n ) % n;
                    ele->CalculateLineParameters( points[h], points[index] );
                    if ( ele->m_horizontal )
                        ele->m_xs = ( int ) points[index].x;
                    else
                        ele->CalculateXs( i );

                    //insert in sorted order of m_xs
                    bool inserted = false;
                    node = m_AETlist.GetFirst();
                    while ( node )
                    {
                        //sorted on smallest xs value
                        if ( ele->m_xs < ( node->GetData() )->m_xs )
                        {
                            m_AETlist.Insert( node, ele );
                            inserted = true;
                            break;
                        }
// TODO to make sorting work  (is eqaul in xs the sorting can be wrong
                        node = node->GetNext();
                    }
                    if ( !inserted )
                        m_AETlist.Append( ele );
                }
            }

            a2dCriticalPointList::compatibility_iterator h =  nodec;
            nodec = nodec->GetNext();
            delete h->GetData();
            m_CRlist.DeleteNode( h );
        }

        //and since i am not smart enough to insert sorted when xs is equal
        //sort again and again :-((
        if ( m_AETlist.GetCount() > 1 )
        {
            if ( m_AETlist.GetCount() > 2 )
                m_AETlist.Sort( SortonXs );
            else if ( m_AETlist.GetFirst()->GetData()->m_xs > m_AETlist.GetLast()->GetData()->m_xs )
            {
                a2dAET* f = m_AETlist.GetFirst()->GetData();
                m_AETlist.GetFirst()->SetData( m_AETlist.GetLast()->GetData() );
                m_AETlist.GetLast()->SetData( f );
            }
        }

        //m_AETlist must be sorted in m_xs at this moment
        //now draw all the line parts on one horizontal scanline (Winding Rule)
        int out =  0;
        node = m_AETlist.GetFirst();
        while ( node )
        {
            a2dAET* ele = node->GetData();
            out += ele->m_direction;

            if ( out != 0 )
            {
                int x1 = ( int ) ele->m_xs;
                node = node->GetNext();
                if ( node )
                {
                    ele = node->GetData();
                    int x2 = ( int ) ele->m_xs;

                    if ( x1 < clip.x ) x1 = clip.x;
                    if ( x2 > clip.x + clip.width ) x2 = clip.x + clip.width;
                    if ( i >=  clip.y && i <= clip.y + clip.height && ( x1 != x2 ) )
                    {
                        if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
                            ColourXYLinear( x1, x2, i );
                        else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
                            ColourXYRadial( x1, x2, i );
                        else
                            DeviceDrawHorizontalLine( x1, i, x2, false );
                    }
                }
                //something is wrong if we arrive here.
            }
            else
                node = node->GetNext();
        }
    }

    a2dAETList::compatibility_iterator node = m_AETlist.GetFirst();
    while ( node )
    {
        a2dAETList::compatibility_iterator h = node;
        delete h->GetData();
        m_AETlist.DeleteNode( h );
        node = m_AETlist.GetFirst();
    }

    a2dCriticalPointList::compatibility_iterator nodec = m_CRlist.GetFirst();
    while ( nodec )
    {
        a2dCriticalPointList::compatibility_iterator h = nodec;
        delete h->GetData();
        m_CRlist.DeleteNode( h );
        nodec = m_CRlist.GetFirst();
    }

}

void a2dContext::DrawPolygon( const a2dVertexList* list, wxPolygonFillMode fillStyle )
{
    a2dBoundingBox devbox;
    bool smallPoly = false;
    int segments = ToDeviceLines( list, devbox, smallPoly );

    if ( segments == 0 )
        return;

    DeviceDrawPolygon( segments, false, fillStyle );
}

void a2dContext::DrawLines( const a2dVertexList* list )
{
    a2dBoundingBox devbox;
    bool smallPoly = false;
    int segments = ToDeviceLines( list, devbox, smallPoly );

    if ( segments > 0 )
        DeviceDrawLines( segments, false );
}

void a2dContext::DrawLine( double x1, double y1, double x2, double y2 )
{
    a2dVertexList points;
    points.push_back( new a2dLineSegment( x1, y1 ) );
    points.push_back( new a2dLineSegment( x2, y2 ) );
    DrawLines( &points );
}

void a2dContext::DeviceDrawPolygon( unsigned int n, bool spline , wxPolygonFillMode fillStyle )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

void a2dContext::DeviceDrawLines( unsigned int n, bool spline )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

void a2dContext::DeviceDrawLine( double x1, double y1, double x2, double y2 )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

void a2dContext::DeviceDrawHorizontalLine( int x1, int y1, int x2, bool WXUNUSED( use_pen ) )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

void a2dContext::DeviceDrawVerticalLine( int x1, int y1, int y2, bool WXUNUSED( use_pen ) )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

void a2dContext::DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

wxPoint* a2dContext::_convertToIntPointCache( int n, wxRealPoint* pts )
{
    m_cpointsInt.resize( n );
    // Convert to integer coords by rounding
    for ( int i = 0; i < n; i++ )
    {
        m_cpointsInt[i].x = Round( pts[i].x );
        m_cpointsInt[i].y = Round( pts[i].y );
    }
    return &m_cpointsInt[0];
}


//HERE

//-----------------------------------------------------------------------------
// a2dDcContext implementation
//-----------------------------------------------------------------------------

a2dDcContext::a2dDcContext( wxGraphicsRenderer* renderer, wxMemoryDC* dc, wxBitmap* drawable )
    :
    a2dContext( renderer, dc->GetSize().GetWidth(), dc->GetSize().GetHeight() )
{
    m_buffer = drawable;
    m_renderDC = dc;
}

a2dDcContext::a2dDcContext( wxGraphicsRenderer* renderer, wxBitmap* drawable )
    :
    a2dContext( renderer, drawable->GetWidth(), drawable->GetHeight() )
{
    m_buffer = drawable;
    m_renderDC = new wxMemoryDC( *drawable );
}

a2dDcContext::~a2dDcContext()
{
}

void a2dDcContext::DoSetActiveStroke()
{
    if ( m_renderDC )
    {
        if ( m_activestroke.GetStyle() == a2dSTROKE_TRANSPARENT )
        {
            m_renderDC->SetPen( *wxTRANSPARENT_PEN );
        }
        else if ( m_activestroke.IsNoStroke() )
        {
            m_renderDC->SetPen( *wxTRANSPARENT_PEN );
            //nullpen gives assert in wxDC its pen (no ref data )
            //m_renderDC->SetPen( *wxNullPen );
        }
        else if ( m_drawstyle == a2dWIREFRAME_INVERT_ZERO_WIDTH )
        {
            m_renderDC->SetPen( *wxBLACK_PEN );
        }
        else if ( m_activestroke.GetType() == a2dSTROKE_ONE_COLOUR )
        {
            int style;
            switch( m_activestroke.GetStyle() )
            {
                case a2dSTROKE_SOLID: style = wxSOLID;
                    break;
                case a2dSTROKE_TRANSPARENT: style = wxTRANSPARENT;
                    break;
                case a2dSTROKE_DOT: style = wxDOT;
                    break;
                case a2dSTROKE_DOT_DASH: style = wxDOT_DASH;
                    break;
                case a2dSTROKE_LONG_DASH: style = wxLONG_DASH;
                    break;
                case a2dSTROKE_SHORT_DASH: style = wxSHORT_DASH;
                    break;
                case a2dSTROKE_BDIAGONAL_HATCH: style = wxBDIAGONAL_HATCH;
                    break;
                case a2dSTROKE_CROSSDIAG_HATCH: style = wxCROSSDIAG_HATCH;
                    break;
                case a2dSTROKE_FDIAGONAL_HATCH: style = wxFDIAGONAL_HATCH;
                    break;
                case a2dSTROKE_CROSS_HATCH: style = wxCROSS_HATCH;
                    break;
                case a2dSTROKE_HORIZONTAL_HATCH: style = wxHORIZONTAL_HATCH;
                    break;
                case a2dSTROKE_VERTICAL_HATCH: style = wxVERTICAL_HATCH;
                    break;
                case a2dSTROKE_STIPPLE: style = wxSTIPPLE;
                    break;
                case a2dSTROKE_STIPPLE_MASK_OPAQUE: style = wxSTIPPLE_MASK_OPAQUE;
                    break;
                default: style = a2dSTROKE_SOLID;
            }

            if ( m_drawstyle == a2dWIREFRAME_ZERO_WIDTH )
            {
                wxPen dcpen( m_activestroke.GetColour(), 0, style );
                m_renderDC->SetPen( dcpen );
                m_renderDC->SetTextForeground( m_activestroke.GetColour() );
            }
            else
            {
                wxPen dcpen( m_activestroke.GetColour(), ( int ) m_activestroke.GetWidth(), style );
                dcpen.SetJoin( m_activestroke.GetJoin() );
                dcpen.SetCap( m_activestroke.GetCap() );
                if ( !m_activestroke.GetPixelStroke() )
                {
                    double w = m_usertodevice.TransformDistance( m_activestroke.GetWidth() );
                    dcpen.SetWidth( ( int ) w );
                }
                m_renderDC->SetPen( dcpen );
                m_renderDC->SetTextForeground( m_activestroke.GetColour() );
            }

        }
        else if ( m_activestroke.GetType() == a2dSTROKE_BITMAP )
        {
            wxPen dcpen;

            if ( m_drawstyle == a2dWIREFRAME_ZERO_WIDTH )
                dcpen = wxPen( m_activestroke.GetColour(), 0, wxBRUSHSTYLE_SOLID );
            else
            {
                int width = ( int ) m_activestroke.GetWidth();
                if ( !m_activestroke.GetPixelStroke() )
                {
                    a2dAffineMatrix inverted = m_usertodevice;
                    inverted.Invert();
                    width = inverted.TransformDistance( m_activestroke.GetWidth() );
                }
                dcpen = wxPen( m_activestroke.GetColour(), width, wxBRUSHSTYLE_SOLID );
                dcpen.SetJoin( m_activestroke.GetJoin() );
                dcpen.SetCap( m_activestroke.GetCap() );
            }

#if defined(__WXMSW__)
            wxBrushStyle style;
            switch( m_activestroke.GetStyle() )
            {
                case a2dSTROKE_STIPPLE:
                    style = wxBRUSHSTYLE_STIPPLE;
                    dcpen.SetStyle( style );
                    dcpen.SetStipple( m_activestroke.GetStipple() );
                    break;
                case a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT:
                case a2dSTROKE_STIPPLE_MASK_OPAQUE:
                    if ( 0 ) //m_printingMode )
                    {
                        wxBitmap noMask = m_activestroke.GetStipple();
                        noMask.SetMask( NULL );
                        style = wxBRUSHSTYLE_STIPPLE;
                        dcpen.SetStyle( style );
                        dcpen.SetStipple( noMask );
                    }
                    else
                    {
                        style = wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE;
                        dcpen.SetStyle( style );
                        dcpen.SetStipple( m_activestroke.GetStipple() );
                    }
                    break;
                default: style = wxBRUSHSTYLE_SOLID;
                    dcpen.SetStyle( style );
            }
#endif

            m_renderDC->SetPen( dcpen );
            m_renderDC->SetTextBackground( *wxBLACK );
            m_renderDC->SetTextForeground( m_activestroke.GetColour() );
        }

    }
}

void a2dDcContext::DoSetActiveFill()
{
    if ( m_renderDC )
    {
        if ( !m_activefill.GetFilling() )
        {
            m_renderDC->SetBrush( *wxTRANSPARENT_BRUSH );
            m_renderDC->SetTextBackground( *wxBLACK );
        }
        else if ( m_activefill.GetStyle() == a2dFILL_TRANSPARENT )
        {
            m_renderDC->SetBrush( *wxTRANSPARENT_BRUSH );
            m_renderDC->SetTextBackground( *wxBLACK );
        }
        else if ( m_activefill.IsNoFill() )
        {
            m_renderDC->SetBrush( *wxTRANSPARENT_BRUSH );
            //m_renderDC->SetBrush( wxNullBrush );
            m_renderDC->SetTextBackground( *wxBLACK );
        }
        else if ( m_activefill.GetType() == a2dFILL_ONE_COLOUR )
        {
            wxBrush dcbrush;
            wxBrushStyle style;
            switch( m_activefill.GetStyle() )
            {
                case a2dFILL_TRANSPARENT:
                    style = wxBRUSHSTYLE_TRANSPARENT;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_SOLID: style = wxBRUSHSTYLE_SOLID;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_BDIAGONAL_HATCH:
                    style = wxBRUSHSTYLE_BDIAGONAL_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_CROSSDIAG_HATCH:
                    style = wxBRUSHSTYLE_CROSSDIAG_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_FDIAGONAL_HATCH:
                    style = wxBRUSHSTYLE_FDIAGONAL_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_CROSS_HATCH:
                    style = wxBRUSHSTYLE_CROSS_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_HORIZONTAL_HATCH:
                    style = wxBRUSHSTYLE_HORIZONTAL_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_VERTICAL_HATCH:
                    style = wxBRUSHSTYLE_VERTICAL_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                default: style = wxBRUSHSTYLE_SOLID;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
            }

            m_renderDC->SetBrush( dcbrush );
            m_renderDC->SetTextBackground( m_activefill.GetColour() );
        }
        else if ( m_activefill.GetType() == a2dFILL_HATCH_TWO_COLOUR )
        {
            wxBrush dcbrush;
            wxBrushStyle style;
            switch( m_activefill.GetStyle() )
            {
                case a2dFILL_TRANSPARENT:
                    style = wxBRUSHSTYLE_TRANSPARENT;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_TWOCOL_BDIAGONAL_HATCH:
                    style = wxBRUSHSTYLE_BDIAGONAL_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_TWOCOL_CROSSDIAG_HATCH:
                    style = wxBRUSHSTYLE_CROSSDIAG_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_TWOCOL_FDIAGONAL_HATCH:
                    style = wxBRUSHSTYLE_FDIAGONAL_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_TWOCOL_CROSS_HATCH:
                    style = wxBRUSHSTYLE_CROSS_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_TWOCOL_HORIZONTAL_HATCH:
                    style = wxBRUSHSTYLE_HORIZONTAL_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_TWOCOL_VERTICAL_HATCH:
                    style = wxBRUSHSTYLE_VERTICAL_HATCH;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
                    break;
                default:
                    style = wxBRUSHSTYLE_SOLID;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
            }

            m_renderDC->SetBrush( dcbrush );
            m_renderDC->SetTextBackground( m_activefill.GetColour() );
        }
        else if ( m_activefill.GetType() == a2dFILL_BITMAP )
        {
            wxBrush dcbrush;
            wxBrushStyle style;
            switch( m_activefill.GetStyle() )
            {
                case a2dFILL_STIPPLE:
                    style = wxBRUSHSTYLE_STIPPLE;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStipple( m_activefill.GetStipple() );
                    dcbrush.SetStyle( style );
                    break;
                case a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT:
                case a2dFILL_STIPPLE_MASK_OPAQUE:
                    if ( 0 ) //m_printingMode )
                    {
                        wxBitmap noMask = m_activefill.GetStipple();
                        noMask.SetMask( NULL );
                        style = wxBRUSHSTYLE_STIPPLE;
                        dcbrush.SetColour( m_activefill.GetColour() );
                        dcbrush.SetStipple( noMask );
                    }
                    else
                    {
                        style = wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE;
                        dcbrush.SetColour( m_activefill.GetColour() );
                        dcbrush.SetStipple( m_activefill.GetStipple() );
                    }
                    dcbrush.SetStyle( style );
                    break;
                default: style = wxBRUSHSTYLE_SOLID;
                    dcbrush.SetColour( m_activefill.GetColour() );
                    dcbrush.SetStyle( style );
            }

            m_renderDC->SetBrush( dcbrush );
            m_renderDC->SetTextBackground( m_activefill.GetColour() );
        }
    }
}

bool a2dDcContext::SetLogicalFunction( wxRasterOperationMode function )
{
    switch( function )
    {
        case wxINVERT:
            m_drawstyle = a2dWIREFRAME_INVERT;
            m_activestroke = *a2dBLACK_STROKE;
            m_activefill = *a2dTRANSPARENT_FILL;
            break;
        default:
            m_drawstyle = a2dFILLED;
    }

    if ( m_renderDC )
        m_renderDC->SetLogicalFunction( function );
    return true;
}

void a2dDcContext::SetDrawStyle( a2dDrawStyle drawstyle )
{
    m_drawstyle = drawstyle;

    switch( drawstyle )
    {
        case a2dWIREFRAME_INVERT:
            m_activestroke = *a2dBLACK_STROKE;
            m_activefill = *a2dTRANSPARENT_FILL;
            if ( m_renderDC )
                m_renderDC->SetLogicalFunction( wxINVERT );
            break;

        case a2dWIREFRAME:
            m_activestroke = *a2dBLACK_STROKE;
            m_activefill = *a2dTRANSPARENT_FILL;
            if ( m_renderDC )
                m_renderDC->SetLogicalFunction( wxCOPY );
            break;

        case a2dWIREFRAME_ZERO_WIDTH:
            m_activestroke = *a2dBLACK_STROKE;
            m_activefill = *a2dTRANSPARENT_FILL;
            if ( m_renderDC )
                m_renderDC->SetLogicalFunction( wxCOPY );
            break;

        case a2dWIREFRAME_INVERT_ZERO_WIDTH:
            m_activestroke = *a2dBLACK_STROKE;
            m_activefill = *a2dTRANSPARENT_FILL;
            if ( m_renderDC )
                m_renderDC->SetLogicalFunction( wxINVERT );
            break;

        case a2dFILLED:
            if ( m_renderDC )
                m_renderDC->SetLogicalFunction( wxCOPY );
            break;

        default:
            wxASSERT( 0 );
    }
}


void a2dDcContext::Clip( wxDouble x, wxDouble y, wxDouble w, wxDouble h )
{
    //need to calculate  clip in device coordinates from world coordinates
    // not even rectangular in the end, so assume a polygon as result.
    wxPoint trpoints[4];
    double dx, dy;

    m_usertodevice.TransformPoint( x, y, dx, dy );
    trpoints[0] = wxPoint( dx, dy );
    m_usertodevice.TransformPoint( x, y + h, dx, dy );
    trpoints[1] = wxPoint( dx, dy );
    m_usertodevice.TransformPoint( x + w, y + h, dx, dy );
    trpoints[2] = wxPoint( dx, dy );
    m_usertodevice.TransformPoint( x + w, y, dx, dy );
    trpoints[3] = wxPoint( dx, dy );

    double xmin = trpoints[0].x;
    double ymin = trpoints[0].y;
    double xmax = trpoints[0].x;
    double ymax = trpoints[0].y;
    for ( int i = 1; i < 4; i ++ )
    {
        xmin = wxMin( xmin, trpoints[i].x );
        xmax = wxMax( xmax, trpoints[i].x );
        ymin = wxMin( ymin, trpoints[i].y );
        ymax = wxMax( ymax, trpoints[i].y );
    }
    wxRect cliprect = wxRect( xmin, ymin, xmax - xmin, ymax - ymin );
    m_clipboxdev = cliprect;
    wxRegion clip( cliprect );
    m_renderDC->DestroyClippingRegion();
#if wxCHECK_VERSION(2,9,0)
    m_renderDC->SetDeviceClippingRegion( clip );
#else
    m_renderDC->SetClippingRegion( clip );
#endif
}

void a2dDcContext::ResetClip()
{
    if ( m_renderDC )
        m_renderDC->DestroyClippingRegion();
    m_clipboxdev = wxRect( 0, 0, m_width, m_height );
}

#if wxCHECK_VERSION(2,9,0)
void a2dDcContext::DrawBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h )
#else
void a2dDcContext::DrawGraphicsBitmap( const wxGraphicsBitmap& bmp, wxDouble x, wxDouble y, wxDouble w, wxDouble h )
#endif
{
    a2dBitmapData* data = static_cast<a2dBitmapData*>( bmp.GetRefData() );
    wxImage image = data->GetImage();

    if ( w == 0 || h == 0 )
        return;

    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    wxPoint centr( ( int ) w / 2, ( int )  h / 2 ); //this doesn't make any difference (BUG?)
    //anyway the rotation center is the middle of the image

    double angle = m_usertodevice.GetRotation();

    if ( fabs( angle ) > 0.5 /*degree*/ )
    {
        image.SetMask( true );
        if ( m_yaxis )
            image = image.Rotate( angle / 180.0 * wxPI, centr, true, NULL );
        else
            image = image.Rotate( -angle / 180.0 * wxPI, centr, true, NULL );
    }

    int devicew, deviceh;

    devicew = m_usertodevice.TransformDistance( w );
    deviceh = fabs( m_usertodevice.TransformDistance( h ) );

    double hx, hy;
    m_usertodevice.TransformPoint( x, y, hx, hy );
    int devicex = ( int ) ( hx - devicew / 2.0 );  //devicex of projected (imaginairy) image
    int devicey = ( int ) ( hy - deviceh / 2.0 );  //devicey of projected (imaginairy) image

    // for drawing transparency
    bool imgHasMask = image.HasMask();

    if ( devicew == 0 || deviceh == 0 )
        return;

    double sx = image.GetWidth() / ( double ) devicew;
    double sy = image.GetHeight() / ( double ) deviceh;

    //don't scale what we do not need, therefore get a subimage
    wxRect clipped = m_clipboxdev;
    clipped.Intersect( wxRect( devicex, devicey, devicew, deviceh ) );
    double deviceClipX =  clipped.GetX();
    double deviceClipY =  clipped.GetY();
    double deviceClipW =  clipped.GetWidth();
    double deviceClipH =  clipped.GetHeight();
    // translate/rescale this to image coordinates
    clipped.SetX( ( clipped.GetX() - devicex ) * sx );
    clipped.SetY( ( clipped.GetY() - devicey ) * sy );
    clipped.SetWidth( clipped.GetWidth() * sx );
    clipped.SetHeight( clipped.GetHeight() * sy );

    //if the subimage image is so small (e.g. extreme zoomin), just draw a rectangle
    if ( clipped.GetWidth() < 1 || clipped.GetHeight() < 1 )
    {
        m_renderDC->DrawRectangle( devicex, devicey, devicew, deviceh );
        return;
    }

    if ( deviceClipW != devicew || deviceClipH != deviceh )
        image = image.GetSubImage( clipped );

    // now we first scale to screen projected size, often limits the amount of pixels to draw
    // when size is smaller then original pixel size.
    image.Rescale(  deviceClipW, deviceClipH );//, wxIMAGE_QUALITY_HIGH  );

    // we can not draw images, so first make it a bitmap, slow !
    wxBitmap Bitmap( image );

    if ( m_drawstyle == a2dFILLED )
    {
        wxMemoryDC MemoryDc;
        MemoryDc.SelectObject( Bitmap );
        // Copy the pre-stretched bitmap to the display device.
        m_renderDC->Blit( deviceClipX, deviceClipY, deviceClipW, deviceClipH, &MemoryDc, 0, 0, wxCOPY, imgHasMask );
    }
    else
    {
        m_renderDC->DrawRectangle( devicex, devicey, devicew, deviceh );
    }
}

void a2dDcContext::DrawVpath( const a2dVpath* path )
{
    unsigned int segments = 0;

    double tstep = 1 / ( double ) SPLINE_STEP;
    unsigned int i;
    for ( i = 0; i < path->size(); i++ )
    {
        a2dVpathSegmentPtr seg = path->operator[]( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_CBCURVETO:
            case a2dPATHSEG_CBCURVETO_NOSTROKE:
                segments += SPLINE_STEP;
                break;

            case a2dPATHSEG_QBCURVETO:
            case a2dPATHSEG_QBCURVETO_NOSTROKE:
                segments += SPLINE_STEP;
                break;

            case a2dPATHSEG_ARCTO:
            case a2dPATHSEG_ARCTO_NOSTROKE:
            {
                a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();

                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                if ( cseg->CalcR( path->operator[]( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                {
                    double dphi;
                    unsigned int circlesegments = 20;
                    double radiusDev = m_usertodevice.TransformDistance( radius );
                    Aberration( m_displayaberration, phit, radiusDev , dphi, circlesegments );
                    segments += circlesegments + 1;
                }
                else
                    segments ++;
            }
            break;
            default:
                segments++;

        }
    }

    //TODO memory error because of wrong calculation of segments?? for the moment *2 to be save
    m_cpointsDouble.resize( segments * 2 );

    double x, y;
    bool move = false;
    int count = 0;
    bool nostrokeparts = false;

    //first draw as much as possible ( nostroke parts may stop this first round )
    for ( i = 0; i < path->size(); i++ )
    {
        a2dVpathSegmentPtr seg = path->operator[]( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_MOVETO:
                if ( count == 0 )
                {
                    m_usertodevice.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    m_cpointsDouble[count].x = x;
                    m_cpointsDouble[count].y = y;
                    count++;
                }
                else
                {
                    i--;
                    move = true;
                }
                break;

            case a2dPATHSEG_LINETO:
                m_usertodevice.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                m_cpointsDouble[count].x = x;
                m_cpointsDouble[count].y = y;
                count++;
                break;
            case a2dPATHSEG_LINETO_NOSTROKE:
                m_usertodevice.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                m_cpointsDouble[count].x = x;
                m_cpointsDouble[count].y = y;
                count++;
                nostrokeparts = true;
                break;

            case a2dPATHSEG_CBCURVETO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_CBCURVETO:
            {
                double xw, yw;

                double xwl = path->operator[]( i ? i - 1 : 0 )->m_x1;
                double ywl = path->operator[]( i ? i - 1 : 0 )->m_y1;
                a2dVpathCBCurveSegment& cseg = ( a2dVpathCBCurveSegment& ) seg;

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 3 ) + cseg.m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg.m_x3 * ( 1 - t ) * t * t * 3 + cseg.m_x1 * pow( t, 3 );
                    yw = ywl * pow( 1 - t, 3 ) + cseg.m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg.m_y3 * ( 1 - t ) * t * t * 3 + cseg.m_y1 * pow( t, 3 );
                    m_usertodevice.TransformPoint( xw, yw, x, y );
                    m_cpointsDouble[count].x = x;
                    m_cpointsDouble[count].y = y;
                    count++;
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_QBCURVETO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_QBCURVETO:
            {
                double xw, yw;

                double xwl = path->operator[]( i ? i - 1 : 0 )->m_x1;
                double ywl = path->operator[]( i ? i - 1 : 0 )->m_y1;
                a2dVpathQBCurveSegment& cseg = ( a2dVpathQBCurveSegment& ) seg;

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 2 ) + cseg.m_x2 * ( 1 - t ) * t * 2 + cseg.m_x1 * pow( t, 2 );
                    yw = ywl * pow( 1 - t, 2 ) + cseg.m_y2 * ( 1 - t ) * t * 2 + cseg.m_y1 * pow( t, 2 );
                    m_usertodevice.TransformPoint( xw, yw, x, y );
                    m_cpointsDouble[count].x = x;
                    m_cpointsDouble[count].y = y;
                    count++;
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_ARCTO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_ARCTO:
            {
                a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();

                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                if ( cseg->CalcR( path->operator[]( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                {
                    double dphi;
                    unsigned int segments = 20;
                    double radiusDev = m_usertodevice.TransformDistance( radius );
                    Aberration( m_displayaberration, phit, radiusDev , dphi, segments );

                    double theta = beginrad;
                    unsigned int step;

                    double x, y;
                    for ( step = 0; step < segments + 1; step++ )
                    {
                        m_usertodevice.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                        m_cpointsDouble[count].x = x;
                        m_cpointsDouble[count].y = y;
                        count++;
                        theta = theta + dphi;
                    }
                }
                else
                {
                    double x, y;
                    m_usertodevice.TransformPoint( cseg->m_x1, cseg->m_y1, x, y );
                    m_cpointsDouble[count].x = x;
                    m_cpointsDouble[count].y = y;
                    count++;
                }
            }
            default:
                break;
                break;
        }

        if ( move )
        {
            DeviceDrawLines( count, false );
            move = false;
            count = 0;
        }
        else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
        {
            if ( nostrokeparts || seg->GetClose() == a2dPATHSEG_END_CLOSED_NOSTROKE )
            {
                a2dStroke stroke = m_activestroke;
                m_activestroke = *a2dTRANSPARENT_STROKE;
                DeviceDrawPolygon( count, false, wxODDEVEN_RULE );
                m_activestroke = stroke;
                nostrokeparts = true;
            }
            else
            {
                DeviceDrawPolygon( count, false, wxODDEVEN_RULE );
            }

            move = false;
            count = 0;
        }
        else if ( i == path->size() - 1 )  //last segment?
        {
            DeviceDrawLines( count, false );
        }
    }

    if ( nostrokeparts )
    {
        move = false;
        count = 0;
        nostrokeparts = false;

        double lastmovex = 0;
        double lastmovey = 0;

        for ( i = 0; i < path->size(); i++ )
        {
            a2dVpathSegmentPtr seg = path->operator[]( i );
            switch ( seg->GetType()  )
            {
                case a2dPATHSEG_MOVETO:
                    if ( count == 0 )
                    {
                        m_usertodevice.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                        m_cpointsDouble[count].x = x;
                        m_cpointsDouble[count].y = y;
                        lastmovex = m_cpointsDouble[count].x;
                        lastmovey = m_cpointsDouble[count].y;
                        count++;
                    }
                    else
                    {
                        i--;
                        move = true;
                    }
                    break;

                case a2dPATHSEG_LINETO:
                    m_usertodevice.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    m_cpointsDouble[count].x = x;
                    m_cpointsDouble[count].y = y;
                    count++;
                    break;

                case a2dPATHSEG_LINETO_NOSTROKE:
                case a2dPATHSEG_CBCURVETO_NOSTROKE:
                case a2dPATHSEG_QBCURVETO_NOSTROKE:
                case a2dPATHSEG_ARCTO_NOSTROKE:
                    if ( count == 0 )
                    {
                        m_usertodevice.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                        m_cpointsDouble[count].x = x;
                        m_cpointsDouble[count].y = y;
                        count++;
                    }
                    else
                    {
                        i--;
                        nostrokeparts = true;
                    }
                    break;

                case a2dPATHSEG_CBCURVETO:
                {
                    double xw, yw;

                    double xwl = path->operator[]( i ? i - 1 : 0 )->m_x1;
                    double ywl = path->operator[]( i ? i - 1 : 0 )->m_y1;
                    a2dVpathCBCurveSegment& cseg = ( a2dVpathCBCurveSegment& ) seg;

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 3 ) + cseg.m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg.m_x3 * ( 1 - t ) * t * t * 3 + cseg.m_x1 * pow( t, 3 );
                        yw = ywl * pow( 1 - t, 3 ) + cseg.m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg.m_y3 * ( 1 - t ) * t * t * 3 + cseg.m_y1 * pow( t, 3 );
                        m_usertodevice.TransformPoint( xw, yw, x, y );
                        m_cpointsDouble[count].x = x;
                        m_cpointsDouble[count].y = y;
                        count++;
                        t = t + tstep;
                    }
                }
                break;

                case a2dPATHSEG_QBCURVETO:
                {
                    double xw, yw;

                    double xwl = path->operator[]( i ? i - 1 : 0 )->m_x1;
                    double ywl = path->operator[]( i ? i - 1 : 0 )->m_y1;
                    a2dVpathQBCurveSegment& cseg = ( a2dVpathQBCurveSegment& ) seg;

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 2 ) + cseg.m_x2 * ( 1 - t ) * t * 2 + cseg.m_x1 * pow( t, 2 );
                        yw = ywl * pow( 1 - t, 2 ) + cseg.m_y2 * ( 1 - t ) * t * 2 + cseg.m_y1 * pow( t, 2 );
                        m_usertodevice.TransformPoint( xw, yw, x, y );
                        m_cpointsDouble[count].x = x;
                        m_cpointsDouble[count].y = y;
                        count++;
                        t = t + tstep;
                    }
                }
                break;

                case a2dPATHSEG_ARCTO:
                {
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg.Get();

                    double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                    if ( cseg->CalcR( path->operator[]( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                    {
                        double dphi;
                        unsigned int segments = 20;
                        double radiusDev = m_usertodevice.TransformDistance( radius );
                        Aberration( m_displayaberration, phit, radiusDev , dphi, segments );

                        double theta = beginrad;
                        unsigned int step;

                        double x, y;
                        for ( step = 0; step < segments + 1; step++ )
                        {
                            m_usertodevice.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                            m_cpointsDouble[count].x = x;
                            m_cpointsDouble[count].y = y;
                            count++;
                            theta = theta + dphi;
                        }
                    }
                    else
                    {
                        double x, y;
                        m_usertodevice.TransformPoint( cseg->m_x1, cseg->m_y1, x, y );
                        m_cpointsDouble[count].x = x;
                        m_cpointsDouble[count].y = y;
                        count++;
                    }
                }
                default:
                    break;

                    break;
            }

            if ( move || nostrokeparts )
            {
                DeviceDrawLines( count, false );
                move = false;
                nostrokeparts = false;
                count = 0;
            }
            else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
            {
                if ( seg->GetClose() == a2dPATHSEG_END_CLOSED )
                {
                    m_cpointsDouble[count].x = lastmovex;
                    m_cpointsDouble[count].y = lastmovey;
                    count++;
                }
                DeviceDrawLines( count, false );
                nostrokeparts = false;
                move = false;
                count = 0;
            }
            else if ( i == path->size() )
            {
                DeviceDrawLines( count, false );
            }
        }
    }

}


void a2dDcContext::DeviceDrawPolygon( unsigned int n, bool spline, wxPolygonFillMode fillStyle )
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    wxPoint* int_cpts = _convertToIntPointCache( n, &m_cpointsDouble[0] );

    // filter the no filling, in order to draw polylines instead
    if ( IsStrokeOnly() )
    {
        if ( n == 2 )
        {
            //Draw polyline instead
            m_renderDC->DrawLines( n, int_cpts );
            // in invert mode do not draw the line back since it will result in drawing nothing.
            if ( m_drawstyle == a2dWIREFRAME || m_drawstyle == a2dWIREFRAME_ZERO_WIDTH )
                m_renderDC->DrawLine( int_cpts[0].x, int_cpts[0].y, int_cpts[n - 1].x, int_cpts[n - 1].y );
        }
        else
        {
            // i assume drawpolygon makes effective use of the fact that the polygon is transparent/not filled,
            // drawing just lines shows no difference in speed on MSW
#if defined(__WXMSW__)
#if wxCHECK_VERSION(2,9,0)
            wxMSWDCImpl* dcimpl = wxStaticCast( m_renderDC->GetImpl(), wxMSWDCImpl );
            WXHDC hdc = dcimpl->GetHDC();
#else
            WXHDC hdc = m_renderDC->GetHDC();
#endif
            SetPolyFillMode( ( struct HDC__* ) hdc, fillStyle == wxODDEVEN_RULE ? ALTERNATE : WINDING );
            //prevent to do things that are not needed (like bbox calculation and checking things)
            //This way is the most direct and fastest possible manner to draw a polygon using API
            ( void )Polygon( ( struct HDC__* ) hdc, ( POINT* )int_cpts, n );
//#elif __WXGTK__
#else
            m_renderDC->DrawPolygon( n, int_cpts, 0, 0, fillStyle );
#endif

        }
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR || m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        FillPolygon( n, &m_cpointsDouble[0] );

        if ( IsStroked() )
        {
            m_renderDC->DrawLines( n, int_cpts );
            m_renderDC->DrawLine( int_cpts[0].x, int_cpts[0].y, int_cpts[n - 1].x, int_cpts[n - 1].y );
        }
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP && m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
#if defined(__WXMSW__)
        m_renderDC->SetTextForeground( *wxBLACK );
        m_renderDC->SetTextBackground( *wxWHITE );
        m_renderDC->SetLogicalFunction( wxAND_INVERT );

        //the above
        COLORREF colFgOld, colBgOld;

#if wxCHECK_VERSION(2,9,0)
        wxMSWDCImpl* dcimpl = wxStaticCast( m_renderDC->GetImpl(), wxMSWDCImpl );
        WXHDC hdc = dcimpl->GetHDC();
#else
        WXHDC hdc = m_renderDC->GetHDC();
#endif

        colBgOld = ::GetBkColor( ( struct HDC__* ) hdc );
        colFgOld = ::GetTextColor( ( struct HDC__* ) hdc );

        //just the opposite from what is expected see help on pattern brush
        // 1 in mask becomes bk color
        ::SetBkColor( ( struct HDC__* ) hdc, m_renderDC->GetTextForeground().GetPixel() );
        //just the opposite from what is expected
        // 0 in mask becomes text color
        ::SetTextColor( ( struct HDC__* ) hdc, m_renderDC->GetTextBackground().GetPixel() );

        int prev = SetPolyFillMode( ( struct HDC__* ) hdc, WINDING );
        ( void )Polygon( ( struct HDC__* ) hdc, ( POINT* )int_cpts, n );

        // Set background and foreground colors for fill pattern
        //the previous blacked out pixels are now merged with the layer color
        //while the non blacked out pixels stay as they are.
        m_renderDC->SetTextForeground( *wxBLACK );
        //now define what will be the color of the fillpattern parts that are not transparent
        m_renderDC->SetTextBackground( m_activefill.GetColour() );
        m_renderDC->SetLogicalFunction( wxOR );

        //just the opposite from what is expected see help on pattern brush
        // 1 in mask becomes bk color
        ::SetBkColor( ( struct HDC__* ) hdc, m_renderDC->GetTextForeground().GetPixel() );
        //just the opposite from what is expected
        // 0 in mask becomes text color
        ::SetTextColor( ( struct HDC__* ) hdc, m_renderDC->GetTextBackground().GetPixel() );
        ( void )Polygon( ( struct HDC__* ) hdc, ( POINT* )int_cpts, n );
        SetPolyFillMode( ( struct HDC__* ) hdc, prev );

        // restore the colours we changed
        ::SetBkMode( ( struct HDC__* ) hdc, TRANSPARENT );
        ::SetTextColor( ( struct HDC__* ) hdc, colFgOld );
        ::SetBkColor( ( struct HDC__* ) hdc, colBgOld );

        m_renderDC->SetLogicalFunction( wxCOPY );
        m_renderDC->SetTextForeground( m_activestroke.GetColour() );
        m_renderDC->SetTextBackground( m_activefill.GetColour() );

        //now do the stroke drawing
        if ( IsStroked() )
        {
            m_renderDC->DrawLines( n, int_cpts );
            m_renderDC->DrawLine( int_cpts[0].x, int_cpts[0].y, int_cpts[n - 1].x, int_cpts[n - 1].y );
        }

//#elif __WXGTK__
#else
        wxPen dcpen( m_activestroke.GetColour(), ( int ) m_activestroke.GetWidth(), wxSOLID );
        //if ( !m_activestroke.GetPixelStroke() )
        //    dcpen.SetWidth( ( int )WorldToDeviceXRel( m_activestroke.GetWidth() ) );
        m_renderDC->SetPen( dcpen );

        m_renderDC->SetTextForeground( *wxBLACK );
        m_renderDC->SetTextBackground( *wxWHITE );
        m_renderDC->SetLogicalFunction( wxAND_INVERT );
        // BLACK OUT the opaque pixels and leave the rest as is
        m_renderDC->DrawPolygon( n, int_cpts, 0, 0, fillStyle );
        // Set background and foreground colors for fill pattern
        //the previous blacked out pixels are now merged with the layer color
        //while the non blacked out pixels stay as they are.
        m_renderDC->SetTextForeground( *wxBLACK );
        //now define what will be the color of the fillpattern parts that are not transparent
        m_renderDC->SetTextBackground( m_activefill.GetColour() );
        m_renderDC->SetLogicalFunction( wxOR );
        //don't understand how but the outline is also depending on logicalfunction
        SetStroke( m_activestroke );
        m_renderDC->DrawPolygon( n, int_cpts, 0, 0, fillStyle );
        m_renderDC->SetLogicalFunction( wxCOPY );
        m_renderDC->SetTextForeground( m_activestroke.GetColour() );
        m_renderDC->SetTextBackground( m_activefill.GetColour() );

        //now do the stroke drawing
        if ( IsStroked() )
        {
            m_renderDC->DrawLines( n, int_cpts );
            m_renderDC->DrawLine( int_cpts[0].x, int_cpts[0].y, int_cpts[n - 1].x, int_cpts[n - 1].y );
        }
#endif
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP &&
              ( m_activefill.GetStyle() == a2dFILL_STIPPLE ||
                m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE )
            )
    {
        m_renderDC->SetTextForeground( m_activefill.GetColour() );
        m_renderDC->SetTextBackground( m_activefill.GetColour2() );
        m_renderDC->DrawPolygon( n, int_cpts, 0, 0, fillStyle );
        m_renderDC->SetTextForeground( m_activestroke.GetColour() );
        m_renderDC->SetTextBackground( m_activefill.GetColour() );
    }
    else if ( m_activefill.GetType() == a2dFILL_HATCH_TWO_COLOUR )
    {
        wxBrush dcbrush( m_activefill.GetColour2() );
        wxBrush cur_dcbrush = m_renderDC->GetBrush();
        m_renderDC->SetBrush( dcbrush );
        m_renderDC->DrawPolygon( n, int_cpts, 0, 0, fillStyle );
        m_renderDC->SetBrush( cur_dcbrush );
        m_renderDC->DrawPolygon( n, int_cpts, 0, 0, fillStyle );
    }
    else
    {
#if defined(__WXMSW__)
        //prevent to do think that are not needed (like bbox calculation and checking things)
        //This way is the most direct and fastest possible manner to draw a polygon using API

        //SetPolyFillMode((struct HDC__ *) hdc,fillStyle==wxODDEVEN_RULE?ALTERNATE:WINDING);
        //(void)Polygon((struct HDC__ *) hdc, (POINT*)int_cpts, n);
        m_renderDC->DrawPolygon( n, int_cpts, 0, 0, fillStyle );
//#elif __WXGTK__
#else
        m_renderDC->DrawPolygon( n, int_cpts, 0, 0, fillStyle );
#endif
    }

    if ( IsStroked() && m_activestroke.GetType() == a2dSTROKE_OBJECT )
    {
        //we must make a copy of the device points now because recursive call
        //can be made in here, which will change the points stored in the drawer.
        //First point repeated extra since it is for outline
        wxRealPoint* copy = new wxRealPoint[n + 1];
        memcpy( copy, &m_cpointsDouble[0], n * sizeof( wxRealPoint ) );
        copy[n].x = m_cpointsDouble[0].x;
        copy[n].y = m_cpointsDouble[0].y;

        //!todo m_activestroke->Render( this, m_clipboxworld, n + 1, copy );
        delete [] copy;
    }
}

void a2dDcContext::DeviceDrawLines( unsigned int n, bool spline )
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    wxPoint* int_cpts = _convertToIntPointCache( n, &m_cpointsDouble[0] );

    if ( n == 1 )
        m_renderDC->DrawCircle( int_cpts[0].x, int_cpts[0].y, 2 );
//        m_renderDC->DrawPoint( int_cpts[0].x, int_cpts[0].y );
    else
    {
        m_renderDC->DrawLines( n, int_cpts );
#if defined(__WXMSW__)
        // Windows does not draw the last pixel if the pensize is 1.
        // especially disturbing for stroke fonts.
        if ( n > 1 && m_renderDC->GetPen().GetWidth() == 1 )
            m_renderDC->DrawPoint( int_cpts[n - 1].x, int_cpts[n - 1].y );
#endif
    }
}

void a2dDcContext::DeviceDrawLine( double x1, double y1, double x2, double y2 )
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    m_renderDC->DrawLine( Round( x1 ), Round( y1 ), Round( x2 ), Round( y2 ) );
}

void a2dDcContext::DeviceDrawHorizontalLine( int x1, int y1, int x2, bool use_stroke_color )
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    wxColour color;
    if ( use_stroke_color )
    {
        wxPen pen = m_renderDC->GetPen();
        color = wxColour( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
        m_renderDC->SetPen( wxPen( color ) );
        m_renderDC->DrawLine( Round( x1 ), Round( y1 ), Round( x2 ), Round( y1 ) );
        m_renderDC->SetPen( pen );
    }
    else
    {
        m_renderDC->DrawLine( Round( x1 ), Round( y1 ), Round( x2 ), Round( y1 ) );
    }
}

void a2dDcContext::DeviceDrawVerticalLine( int x1, int y1, int y2, bool use_stroke_color )
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    wxColour color;
    if ( use_stroke_color )
    {
        wxPen pen = m_renderDC->GetPen();
        color = wxColour( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
        m_renderDC->SetPen( wxPen( color ) );
        m_renderDC->DrawLine( Round( x1 ), Round( y1 ), Round( x1 ), Round( y2 ) );
        m_renderDC->SetPen( pen );
    }
    else
    {
        m_renderDC->DrawLine( Round( x1 ), Round( y1 ), Round( x1 ), Round( y2 ) );
    }
}

void a2dDcContext::DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b , unsigned char a )
{
#if defined(__WXMSW__)
#if wxCHECK_VERSION(2,9,0)
    wxMSWDCImpl* dcimpl = wxStaticCast( m_renderDC->GetImpl(), wxMSWDCImpl );
    WXHDC hdc = dcimpl->GetHDC();
#else
    WXHDC hdc = m_renderDC->GetHDC();
#endif
    ::SetPixelV( ( struct HDC__* )hdc, x1, y1, RGB( r, g, b ) );
#elif __WXGTK__

#if wxCHECK_VERSION(2,9,1)
    wxWindowDCImpl* dcimpl = wxStaticCast( m_renderDC->GetImpl(), wxWindowDCImpl );
    GdkWindow* gtkwin =  dcimpl->GetGDKWindow();
    if ( gtkwin )
        gdk_draw_point( gtkwin, dcimpl->m_penGC, x1, y1 );
#else
    wxColour col( r, g, b );
    col.CalcPixel( ( ( wxWindowDC* )m_renderDC )->m_cmap );
    gdk_gc_set_foreground( ( ( wxWindowDC* )m_renderDC )->m_penGC, col.GetColor() );
    gdk_draw_point( ( ( wxWindowDC* ) m_renderDC )->m_window, ( ( wxWindowDC* )m_renderDC )->m_penGC, x1, y1 );
#endif

#else

#endif
}

void a2dDcContext::DeviceDrawBitmap( const wxBitmap& bmp, double x, double y, bool useMask )
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
    if ( m_drawstyle == a2dWIREFRAME_INVERT )
    {
        m_renderDC->DrawRectangle( Round( x ), Round( y ), bmp.GetWidth(), bmp.GetHeight() );
    }
    else
        m_renderDC->DrawBitmap( bmp, Round( x ), Round( y ), useMask );
}

void a2dDcContext::DrawCharDc( wxChar c )
{
    double x, y;
    x = y = 0.0;
    double dx, dy;
    double angle = m_usertodevice.GetRotation();
    m_usertodevice.TransformPoint( x, y + m_a2dfont.GetLineHeight(), dx, dy );
    m_renderDC->DrawRotatedText( wxString( c ), ( int ) dx, ( int ) dy, -angle );
}

void a2dDcContext::DrawTextDc( const wxString& text, double x, double y )
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    // Get font size in device units
    unsigned int fontsize;
    double dx = m_usertodevice.GetValue( 1, 0 );
    double dy = m_usertodevice.GetValue( 1, 1 );
    fontsize = ( unsigned int ) fabs( m_a2dfont.GetSize() * sqrt( dx * dx + dy * dy ) );
    if ( fontsize < 1 )
        fontsize = 1;
    m_a2dfont.GetFont().SetPointSize( fontsize );

    m_renderDC->SetFont( m_a2dfont.GetFont() );
    m_renderDC->SetBackgroundMode( wxTRANSPARENT );
    m_renderDC->SetTextForeground( m_activestroke.GetColour() );

    DrawTextGeneric( text, x, y, ( void ( a2dContext::* )( wxChar ) ) & a2dDcContext::DrawCharDc );
}

#if wxART2D_USE_FREETYPE
extern FT_Library g_freetypeLibrary;

//! used in freetype rendering
typedef struct
{
    wxColour colour;
    int xmin, ymax;
    int stride;
    unsigned char* buf;
}
a2dSpanData;

//! used in freetype rendering
static void a2dSpanFuncGray( int y, int count, FT_Span* spans, a2dSpanData* user )
{
    unsigned int alpha, invalpha, len;
    // although colours are chars, using ints is faster in multiplications.
    unsigned int r, g, b, rpm, gpm, bpm;
    r = user->colour.Red();
    g = user->colour.Green();
    b = user->colour.Blue();

    unsigned char* buf, *buf2;
    buf = user->buf + ( user->ymax - y ) * user->stride - user->xmin * 3;
    do
    {
        buf2 = buf + spans->x * 3;
        len = spans->len;
        alpha = spans->coverage;
        if ( alpha == 255 )
        {
            while( len )
            {
                // do not try to replace by *buf2++ = r, etc.
                // it turns out that this code actually is faster (for x86).
#ifdef __WXGTK__
                buf2[0] = r;
                buf2[1] = g;
                buf2[2] = b;
#else
                buf2[0] = b;
                buf2[1] = g;
                buf2[2] = r;
#endif
                buf2 += 3;
                len--;
            }
        }
        else
        {
            rpm = r * alpha;
            gpm = g * alpha;
            bpm = b * alpha;
            invalpha = 255 - alpha;
            while( len )
            {
                // do not try to replace by *buf2++ = ..., etc.
                // it turns out that this code actually is faster (for x86).
#ifdef __WXGTK__
                buf2[0] = ( buf2[0] * invalpha + rpm ) / 255;
                buf2[1] = ( buf2[1] * invalpha + gpm ) / 255;
                buf2[2] = ( buf2[2] * invalpha + bpm ) / 255;
#else
                buf2[0] = ( buf2[0] * invalpha + bpm ) / 255;
                buf2[1] = ( buf2[1] * invalpha + gpm ) / 255;
                buf2[2] = ( buf2[2] * invalpha + rpm ) / 255;
#endif
                buf2 += 3;
                len--;
            }
        }
        spans++;
    }
    while ( --count );
}
#endif

void a2dDcContext::DrawCharFreetype( wxChar c )
{
#if wxART2D_USE_FREETYPE
    // Add offset, to put anchor on lower-left point of bbox.
    double dx, dy;
    m_usertodevice.TransformPoint( 0.0, m_a2dfont.GetDescent(), dx, dy );

    // Get the glyph and make a copy of it
    const a2dGlyph* a2dglyph = m_a2dfont.GetGlyphFreetype( c );
    if( !a2dglyph )
        return;
    FT_Glyph image;
    if ( FT_Glyph_Copy( a2dglyph->m_glyph, &image ) != 0 )
        return;

    // Generate affine, to scale character from normalized to real size.
    a2dAffineMatrix glyphPos;
    glyphPos.Scale( m_a2dfont.GetSize() / NORMFONT );
    a2dAffineMatrix glyphToDevice = m_usertodevice * glyphPos;

    // Transform glyph and scale it to device coordinates. Also correct for sub-pixel drawing.
    FT_Matrix trans_matrix;
    trans_matrix.xx = ( FT_Fixed ) ( glyphToDevice.GetValue( 0, 0 ) * 0x10000 );
    trans_matrix.xy = ( FT_Fixed ) ( glyphToDevice.GetValue( 1, 0 ) * 0x10000 );
    trans_matrix.yx = ( FT_Fixed ) ( -glyphToDevice.GetValue( 0, 1 ) * 0x10000 );
    trans_matrix.yy = ( FT_Fixed ) ( -glyphToDevice.GetValue( 1, 1 ) * 0x10000 );
    FT_Vector vec;
    vec.x = ( int ) ( ( dx - ( int ) dx ) * 64.0 );
    vec.y = ( int ) ( ( dy - ( int ) dy ) * -64.0 );
    if ( FT_Glyph_Transform( image, &trans_matrix, &vec ) != 0 )
    {
        wxFAIL_MSG( _( "glyph transform wrong" ) );
        FT_Done_Glyph( image );
        return;
    }

    // Get bounding box of the area which should be drawn.
    FT_BBox  bbox;
    FT_Glyph_Get_CBox( image, ft_glyph_bbox_pixels, &bbox );

    // clip against clipbox.
    if ( m_clipboxdev.GetLeft() > ( int ) dx + bbox.xMin )
        bbox.xMin = m_clipboxdev.GetLeft() - ( int ) dx;
    if ( m_clipboxdev.GetRight() < ( int ) dx + bbox.xMax )
        bbox.xMax = m_clipboxdev.GetRight() - ( int ) dx;

    if ( m_clipboxdev.GetTop() >= ( int ) dy - bbox.yMax )
        bbox.yMax = ( int ) dy - m_clipboxdev.GetTop();
    if ( m_clipboxdev.GetBottom() < ( int ) dy - bbox.yMin )
        bbox.yMin = ( int ) dy - m_clipboxdev.GetBottom();

    // If size == 0, nothing has to be drawn.
    if ( bbox.xMax - bbox.xMin <= 0 || bbox.yMax - bbox.yMin <= 0 )
    {
        FT_Done_Glyph( image );
        return;
    }

    int px = ( int ) dx + bbox.xMin;
    int py = ( int ) dy - bbox.yMax;
    int w = bbox.xMax - bbox.xMin + 1;
    int h = bbox.yMax - bbox.yMin + 1;

    // Obtain background image
    wxRect absarea2( px, py, w, h );
    wxBitmap sub_bitmap = m_buffer->GetSubBitmap( absarea2 );

#if defined(__WXMSW__)
    BITMAPINFO bi;
    bi.bmiHeader.biSize = sizeof( bi.bmiHeader );
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = 0;
    bi.bmiHeader.biClrUsed = 0;
    bi.bmiHeader.biClrImportant = 0;

    HBITMAP hbitmap = ( HBITMAP ) sub_bitmap.GetHBITMAP();
    HDC hdc = ::GetDC( NULL );
    HDC memdc = ::CreateCompatibleDC( hdc );
    unsigned char* buf = ( unsigned char* ) malloc( ( 3 * w + sizeof( DWORD ) - 1 ) / sizeof( DWORD ) * sizeof( DWORD ) * h );
    ::GetDIBits( memdc, hbitmap, 0, h, buf, &bi, DIB_RGB_COLORS );
#else // defined( __WXMSW__ )
    wxImage wximage( sub_bitmap.ConvertToImage() );
#endif // defined( __WXMSW__ )

    // Render glyph to an image
    if ( image->format == FT_GLYPH_FORMAT_OUTLINE )
    {
        a2dSpanData spandata;
        spandata.colour = m_activestroke.GetColour();
        spandata.xmin = bbox.xMin;
        spandata.ymax = bbox.yMax;
        spandata.stride = w * 3;
#if defined(__WXMSW__)
        // calculate image stride ( = w + padding )
        spandata.stride = ( spandata.stride + sizeof( DWORD ) - 1 ) / sizeof( DWORD ) * sizeof( DWORD );
        spandata.buf = buf;
#else // defined( __WXMSW__ )
        spandata.buf = wximage.GetData();
#endif // defined( __WXMSW__ )
        wxASSERT( spandata.buf != NULL );

        FT_Raster_Params params;
        params.target = NULL;
        params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT | FT_RASTER_FLAG_CLIP;
        params.gray_spans = ( FT_SpanFunc ) & a2dSpanFuncGray;
        params.black_spans = NULL;
        params.bit_test = ( FT_Raster_BitTest_Func ) NULL;
        params.bit_set = ( FT_Raster_BitSet_Func ) NULL;
        params.user = &spandata;
        params.clip_box = bbox;

        FT_Outline& outline = ( ( FT_OutlineGlyph ) image )->outline;
        if ( FT_Outline_Render( g_freetypeLibrary, &outline, &params ) == 0 )
        {
#if defined( __WXMSW__ )
            ::SetDIBits( memdc, hbitmap, 0, h, buf, &bi, DIB_RGB_COLORS );
#else // defined( __WXMSW__ )
            wxBitmap sub_bitmap( wximage );
#endif // defined( __WXMSW__ )
            DeviceDrawBitmap( sub_bitmap, px, py, false  );
        }
    }
    else
        wxFAIL_MSG( _( "Non vector fonts are not supported" ) );

#if defined(__WXMSW__)
    free( buf );
    ::DeleteDC( memdc );
    ::ReleaseDC( NULL, hdc );
#endif // defined( __WXMSW__ )

    FT_Done_Glyph( image );
#else // wxART2D_USE_FREETYPE
    DrawCharUnknown( c );
#endif // wxART2D_USE_FREETYPE
}
/*
void a2dDcContext::DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity )
{
}

void a2dDcContext::DrawImage( const wxImage& imagein, double x, double y, double width, double height, wxUint8 Opacity )
{
}
*/


//-----------------------------------------------------------------------------
// a2dRenderer implementation
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dRenderer, wxGraphicsRenderer )


wxGraphicsContext* a2dRenderer::CreateContext( const wxWindowDC& dc )
{
    wxFAIL_MSG( wxT( "only drawable buffers for a2dContext" ) );
    return NULL;
}

wxGraphicsContext* a2dRenderer::CreateContext( const wxMemoryDC& dc )
{
    //return new a2dDcContext( this, &dc, &(dc.GetSelectedBitmap()) );
    wxFAIL_MSG( wxT( "only drawable buffers for a2dContext" ) );
    return NULL;
}

wxGraphicsContext* a2dRenderer::CreateContext( wxMemoryDC* dc, wxBitmap* drawable )
{
    return new a2dDcContext( this, dc, drawable );
}

wxGraphicsContext* a2dRenderer::CreateContextFromNativeContext( void* context )
{
    wxFAIL_MSG( wxT( "only drawable buffers for a2dContext" ) );
    return NULL;
}

#if wxCHECK_VERSION(2, 9, 0)
#if wxUSE_PRINTING_ARCHITECTURE
wxGraphicsContext* a2dRenderer::CreateContext( const wxPrinterDC& dc )
{
    wxFAIL_MSG( wxT( "only drawable buffers for a2dContext" ) );
    return NULL;
}
#ifdef __WXMSW__
wxGraphicsContext* a2dRenderer::CreateContext( const wxEnhMetaFileDC& dc )
{
    wxFAIL_MSG( wxT( "only drawable buffers for a2dContext" ) );
    return NULL;
}
#endif
#endif
#endif //wxCHECK_VERSION(2, 9, 0)

wxGraphicsContext* a2dRenderer::CreateContextFromNativeWindow( void* window )
{
    wxFAIL_MSG( wxT( "only drawable buffers for a2dContext" ) );
    return NULL;
}

wxGraphicsContext* a2dRenderer::CreateMeasuringContext()
{
    return NULL;
    // TODO
}

wxGraphicsContext* a2dRenderer::CreateContext( wxWindow* window )
{
    wxFAIL_MSG( wxT( "only drawable buffers for a2dContext" ) );
    return NULL;
}

// Path

wxGraphicsPath a2dRenderer::CreatePath()
{
    wxGraphicsPath path;
    path.SetRefData( new a2dPathData( this ) );
    return path;
}


// Matrix

wxGraphicsMatrix a2dRenderer::CreateMatrix( wxDouble a, wxDouble b, wxDouble c, wxDouble d,
        wxDouble tx, wxDouble ty )

{
    wxGraphicsMatrix m;
    a2dMatrixData* data = new a2dMatrixData( this );
    data->Set( a, b, c, d, tx, ty ) ;
    m.SetRefData( data );
    return m;
}

wxGraphicsPen a2dRenderer::CreatePen( const wxPen& pen )
{
    if ( !pen.Ok() || pen.GetStyle() == wxTRANSPARENT )
        return wxNullGraphicsPen;
    else
    {
        wxGraphicsPen p;
        p.SetRefData( new a2dStrokeData( this, pen ) );
        return p;
    }
}

wxGraphicsBrush a2dRenderer::CreateBrush( const wxBrush& brush )
{
    if ( !brush.Ok() || brush.GetStyle() == wxTRANSPARENT )
        return wxNullGraphicsBrush;
    else
    {
        wxGraphicsBrush p;
        p.SetRefData( new a2dFillData( this, brush ) );
        return p;
    }
}

wxGraphicsPen a2dRenderer::CreateStroke( const a2dStroke& stroke )
{
    //if ( stroke.GetStyle() == a2dSTROKE_TRANSPARENT )
    //    return wxNullGraphicsPen;
    //else
    {
        wxGraphicsPen p;
        p.SetRefData( new a2dStrokeData( this, stroke ) );
        return p;
    }
}

wxGraphicsBrush a2dRenderer::CreateFill( const a2dFill& fill )
{
    //if ( fill.GetStyle() == a2dFILL_TRANSPARENT )
    //    return wxNullGraphicsBrush;
    //else
    {
        wxGraphicsBrush p;
        p.SetRefData( new a2dFillData( this, fill ) );
        return p;
    }
}

#if wxCHECK_VERSION(2, 9, 1)
wxGraphicsBrush a2dRenderer::CreateLinearGradientBrush( wxDouble x1, wxDouble y1,
        wxDouble x2, wxDouble y2,
        const wxGraphicsGradientStops& stops )
{
    wxGraphicsBrush p;
    a2dFillData* d = new a2dFillData( this );
    d->CreateLinearGradientBrush( x1, y1, x2, y2, stops );
    p.SetRefData( d );
    return p;
}

wxGraphicsBrush a2dRenderer::CreateRadialGradientBrush( wxDouble xo, wxDouble yo,
        wxDouble xc, wxDouble yc,
        wxDouble radius,
        const wxGraphicsGradientStops& stops )
{
    wxGraphicsBrush p;
    a2dFillData* d = new a2dFillData( this );
    d->CreateRadialGradientBrush( xo, yo, xc, yc, radius, stops );
    p.SetRefData( d );
    return p;
}

#else //wxCHECK_VERSION(2, 9, 1)

// sets the brush to a linear gradient, starting at (x1,y1) with color c1 to (x2,y2) with color c2
wxGraphicsBrush a2dRenderer::CreateLinearGradientBrush( wxDouble x1, wxDouble y1, wxDouble x2, wxDouble y2,
        const wxColour& c1, const wxColour& c2 )
{
    wxGraphicsBrush p;
    a2dFillData* d = new a2dFillData( this );
    d->CreateLinearGradientBrush( x1, y1, x2, y2, c1, c2 );
    p.SetRefData( d );
    return p;
}

// sets the brush to a radial gradient originating at (xo,yc) with color oColor and ends on a circle around (xc,yc)
// with radius r and color cColor
wxGraphicsBrush a2dRenderer::CreateRadialGradientBrush( wxDouble xo, wxDouble yo, wxDouble xc, wxDouble yc, wxDouble radius,
        const wxColour& oColor, const wxColour& cColor )
{
    wxGraphicsBrush p;
    a2dFillData* d = new a2dFillData( this );
    d->CreateRadialGradientBrush( xo, yo, xc, yc, radius, oColor, cColor );
    p.SetRefData( d );
    return p;
}
#endif //wxCHECK_VERSION(2, 9, 1)

// sets the font
wxGraphicsFont a2dRenderer::CreateFont( const wxFont& font , const wxColour& col )
{
    if ( font.Ok() )
    {
        wxGraphicsFont p;
        p.SetRefData( new a2dFontData( this , font, col ) );
        return p;
    }
    else
        return wxNullGraphicsFont;
}

wxGraphicsFont a2dRenderer::CreateFont( const a2dFont& font )
{
    wxGraphicsFont p;
    p.SetRefData( new a2dFontData( this, font ) );
    return p;
}

wxGraphicsBitmap a2dRenderer::CreateBitmap( const wxBitmap& bmp )
{
    if ( bmp.Ok() )
    {
        wxGraphicsBitmap p;
        p.SetRefData( new a2dBitmapData( this , bmp ) );
        return p;
    }
    else
        return wxNullGraphicsBitmap;
}

#if wxCHECK_VERSION(2, 9, 1)
wxGraphicsBitmap a2dRenderer::CreateBitmapFromNativeBitmap( void* bitmap )
{
    wxGraphicsBitmap p;
    p.SetRefData( new a2dBitmapData( this , ( const wxBitmap& ) bitmap ) );
    return p;
}
#endif //wxCHECK_VERSION(2, 9, 1)

wxGraphicsBitmap a2dRenderer::CreateBitmap( const wxImage& bmp )
{
    if ( bmp.Ok() )
    {
        wxGraphicsBitmap p;
        p.SetRefData( new a2dBitmapData( this , bmp ) );
        return p;
    }
    else
        return wxNullGraphicsBitmap;
}

wxGraphicsBitmap a2dRenderer::CreateSubBitmap( const wxGraphicsBitmap& bitmap, wxDouble x, wxDouble y, wxDouble w, wxDouble h  )
{
    wxGraphicsBitmap p;
    p.SetRefData( new a2dBitmapData( this , bitmap ) );
    return p;
}


#endif  // wxART2D_USE_GRAPHICS_CONTEXT


