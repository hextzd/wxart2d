/*! \file artbase/src/drawer2d.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: drawer2d.cpp,v 1.159 2009/10/06 18:40:31 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/artbase/drawer2d.h"
#include "wx/artbase/stylebase.h"
#if defined(__WXMSW__)
#include "wx/artbase/mswfont.h"
#endif
#endif

#if wxUSE_STD_IOSTREAM
#include <wx/ioswrap.h>
#else
#include <wx/wfstream.h>
#include <wx/sstream.h>
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

#include "wx/general/comevt.h"

#define     THRESHOLD   5
// 16 hierarchy levels should be more then sufficient in general.
#define AFFINESTACKSIZE 16
#define TEXT_MAXSIZE_INPIXELS 3000 //related to screen size in pixels

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// a2dDrawer2D
//----------------------------------------------------------------------------

const a2dSignal a2dDrawer2D::sig_changedZoom = wxNewId();


a2dDrawer2D::a2dDrawer2D( const wxSize& size )
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    : m_initCurrentSmartPointerOwner( this )
#endif
{
    m_width = size.GetWidth();
    m_height = size.GetHeight();
    m_display = NULL;

    Init();
}

a2dDrawer2D::a2dDrawer2D( int width, int height )
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    : m_initCurrentSmartPointerOwner( this )
#endif
{
    m_width = width;
    m_height = height;
    m_display = NULL;

    Init();
}

a2dDrawer2D::a2dDrawer2D( const a2dDrawer2D& other )
    : wxObject( other )
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    , m_initCurrentSmartPointerOwner( this )
#endif
{
    m_cpointsDouble.clear();
    m_cpointsInt.clear();

    m_display = other.m_display;

    Init();

    m_realScale = other.m_realScale;
    m_width = other.m_width;
    m_height = other.m_height;

    m_xpp =  other.m_xpp;
    m_ypp =  other.m_ypp;

    m_virt_minX =  other.m_virt_minX;
    m_virt_minY =  other.m_virt_minY;

    m_worldtodevice    = other.m_worldtodevice;
    SetTransform( other.GetTransform() );

    m_virtualarea_set =  other.m_virtualarea_set;

    m_printingMode = other.m_printingMode;
    m_yaxis = other.m_yaxis ;

}

void a2dDrawer2D::Init()
{
    DeleteShapeIds();
    m_preserveCache = false;
    m_realScale = true;
    m_disableDrawing = false;
    m_forceNormalizedFont = false;
    m_smallTextThreshold = SMALLTEXTTHRESHOLD;

    m_drawingthreshold = a2dGlobals->GetPrimitiveThreshold();
    m_polygonFillThreshold = a2dGlobals->GetPolygonFillThreshold();
    m_asrectangle = a2dGlobals->GetThresholdDrawRectangle();

    m_refreshDisplay = 0;

    m_frozen = false;

    //set one to one mapping as default
    m_xpp = 1;
    m_ypp = 1;

    m_mapX = m_mapY = 0;
    m_mapWidth = m_width;
    m_mapHeight = m_height;

    m_virt_minX = 0;
    m_virt_minY = 0;

    m_worldtodevice.Identity();
    SetTransform( a2dIDENTITY_MATRIX );

    m_drawstyle = a2dFILLED;
    m_virtualarea_set = false;

    m_yaxis = false;
    m_yaxisDevice = false;

    m_currentstroke = *a2dBLACK_STROKE;
    m_currentfill = *a2dBLACK_FILL;
    m_currentfont = *a2dNullFONT;
    m_activestroke = m_currentstroke;
    m_activefill = m_currentfill;
    m_layer = 0;

    m_OpacityFactor = 255;

    m_fixStrokeRestore = *a2dBLACK_STROKE;
    m_fixFillRestore = *a2dBLACK_FILL;
    m_fixDrawstyle = m_drawstyle;

    m_splineaberration = 10;

    m_displayaberration = 0.5;

    m_beginDraw_endDraw = 0;
    m_printingMode = false;
    m_pendingSig_changedZoom = false;
}

a2dDrawer2D::~a2dDrawer2D()
{
    DeleteShapeIds();
    m_preserveCache = false;
    wxASSERT_MSG( m_affineStack.empty(), _( "Unbalanced Push/PopTransform" ) );
}


void a2dDrawer2D::BlitBuffer()
{
    wxRect rect( 0, 0, m_width, m_height );
    BlitBuffer( rect,  wxPoint( 0, 0 ) );
}

void a2dDrawer2D::BlitBuffer( int x, int y, int width, int height, int xbuf, int ybuf )
{
    wxRect rect( x, y, width, height );
    BlitBuffer( rect, wxPoint( xbuf, ybuf ) );
}

void a2dDrawer2D::SetSplineAberration( double aber )
{
    m_splineaberration = aber;
}

//------------------------------------------------------------------------
// mapping of device to world and visa versa
//------------------------------------------------------------------------

void a2dDrawer2D::EndRefreshDisplayDisable() 
{ 
    m_refreshDisplay--; 
    a2dComEvent changedZoom( this, a2dDrawer2D::sig_changedZoom );
    if ( m_pendingSig_changedZoom && m_refreshDisplay <= 0 )
    {
        m_pendingSig_changedZoom = false;
        if ( m_display )
            m_display->GetEventHandler()->ProcessEvent( changedZoom );
    }
}

void a2dDrawer2D::SetMappingDeviceRect( int mapx, int mapy, int mapWidth, int mapHeight, bool remap )
{
    m_mapX = mapx;
    m_mapY = mapy;
    m_mapWidth = mapWidth;
    m_mapHeight = mapHeight;
    if ( remap )
        SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, m_virt_minX, m_virt_minY, m_xpp, m_ypp );
}

void a2dDrawer2D::SetMinX( double x )
{
    m_virt_minX = x;
    SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, m_virt_minX, m_virt_minY, m_xpp, m_ypp );
}

void a2dDrawer2D::SetMinY( double y )
{
    m_virt_minY = y;
    SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, m_virt_minX, m_virt_minY, m_xpp, m_ypp );
}

void a2dDrawer2D::SetUppX( double x )
{
    m_xpp = x;
    SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, m_virt_minX, m_virt_minY, m_xpp, m_ypp );
}

void a2dDrawer2D::SetUppY( double y )
{
    m_ypp = y;
    SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, m_virt_minX, m_virt_minY, m_xpp, m_ypp );
}

void a2dDrawer2D::SetMappingUpp( double vx1, double vy1, double xpp, double ypp )
{
    SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, vx1, vy1, xpp, ypp );
}

// maps the virtual window (Real drawing to the window coordinates
// also used for zooming
void a2dDrawer2D::SetMappingUpp( double x, double y, double wx, double wy, double vx1, double vy1, double xpp, double ypp )
{
//    static bool recur = false;
//    if ( recur )
//        return;
//    recur = true;

    m_mapX = ( int ) x;
    m_mapY = ( int ) y;
    m_mapWidth = ( int ) wx;
    m_mapHeight = ( int ) wy;

    m_virt_minX = vx1;
    m_virt_minY = vy1;
    m_xpp = xpp;
    m_ypp = ypp;

    //wxLogDebug( _T( "mapX=%d, mapY=%d mapWidth=%d mapHeight=%d virt_minX=%f, virt_minY=%f xpp=%f ypp=%f" ),  m_mapX, m_mapY, m_mapWidth, m_mapHeight, m_virt_minX, m_virt_minY, m_xpp, m_ypp );

    //!transformation from world to device coordinates
    double scalex, scaley, transx, transy;

    // scale the drawing so that one pixel is the right number of units
    scalex = 1 / m_xpp;
    scaley = 1 / m_ypp;

    // initialize the mapping used for mapping the
    // virtual window to the pixel window

    // translate the drawing to 0,0
    if ( m_yaxis )
    {
        double virt_maxY = m_virt_minY + wy * m_ypp;
        transx = -m_virt_minX + m_mapX * m_xpp;
        transy = -virt_maxY   + m_mapY * m_ypp;
    }
    else
    {
        transx = -m_virt_minX + m_mapX * m_xpp;
        transy = -m_virt_minY + m_mapY * m_ypp;
    }

    transx = transx * scalex;
    transy = transy * scaley;

    // because of coordinates origin change, mirror over X.
    // Like 0,0 in graphic computerscreens: upperleft corner.
    // While 0,0 in cartesian: lowerleft corner
    if ( m_yaxisDevice != m_yaxis )
    {
        scaley = -scaley;
        transy = -transy;
    }

    //now we have a valid virtual area that is visible
    m_virtualarea_set = true;

    //setup the transform to reflect the mapping.
    m_worldtodevice.SetValue( 2 , 0, transx );
    m_worldtodevice.SetValue( 2 , 1, transy );
    m_worldtodevice.SetValue( 0 , 0, scalex );
    m_worldtodevice.SetValue( 1 , 1, scaley );

    // for speed matrix we have a matrix for going directly from user to device space.
    m_usertodevice = m_worldtodevice * m_usertoworld;
    a2dComEvent changedZoom( this, a2dDrawer2D::sig_changedZoom );
    if ( m_display && m_refreshDisplay <= 0 )
    {
        m_display->GetEventHandler()->ProcessEvent( changedZoom );
        m_display->Refresh();
    }
    else
        m_pendingSig_changedZoom = true;
    DestroyClippingRegion();

    DoSetMappingUpp();

//    recur = false;
}

void a2dDrawer2D::SetMappingWidthHeight( double vx1, double vy1, double width, double height )
{
    double xupp = width / m_mapWidth;
    double yupp = height / m_mapHeight;

    if ( yupp == 0 || xupp == 0 ) //no drawing at all
    {
        yupp = 1; xupp = 1; //some value
    }

    if( m_realScale )
    {
        if ( yupp > xupp )
        {
            SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, vx1, vy1, yupp, yupp );
        }
        else
        {
            SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, vx1, vy1, xupp, xupp );
        }
    }
    else
        SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, vx1, vy1, xupp, yupp );
}

void a2dDrawer2D::SetMappingWidthHeight( const a2dBoundingBox& box )
{
    double vx1, vy1, width, height;
    vx1 = box.GetMinX();
    vy1 = box.GetMinY();
    width = box.GetWidth();
    height = box.GetHeight();

    double xupp = width / m_mapWidth;
    double yupp = height / m_mapHeight;

    if ( yupp == 0 || xupp == 0 ) //no drawing at all
    {
        yupp = 1; xupp = 1; //some value
    }

    double xc = vx1 + width/2.0;
    double yc = vy1 + height/2.0;

    if( m_realScale )
    {
        if ( yupp > xupp )
        {
            vx1 = xc - yupp* m_mapWidth/2.0;
            vy1 = yc - yupp* m_mapHeight/2.0;
            SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, vx1, vy1, yupp, yupp );
        }
        else
        {
            vx1 = xc - xupp* m_mapWidth/2.0;
            vy1 = yc - xupp* m_mapHeight/2.0;
            SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, vx1, vy1, xupp, xupp );
        }
    }
    else
        SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, vx1, vy1, xupp, yupp );
}

void a2dDrawer2D::SetMappingMatrix( const a2dAffineMatrix& mapping )
{
    m_worldtodevice = mapping;
    // for speed matrix we have a matrix for going directly from user to device space.
    m_usertodevice = m_worldtodevice * m_usertoworld;
}

double a2dDrawer2D::GetVisibleMinX() const
{
    return m_virt_minX;
}

double a2dDrawer2D::GetVisibleMinY() const
{
    return m_virt_minY;
}

double a2dDrawer2D::GetVisibleMaxX() const
{
    return m_virt_minX + m_xpp * m_mapWidth;
}

double a2dDrawer2D::GetVisibleMaxY() const
{
    return m_virt_minY + m_ypp * m_mapHeight;
}

double a2dDrawer2D::GetVisibleWidth() const
{
    return m_xpp * m_mapWidth;
}

double a2dDrawer2D::GetVisibleHeight() const
{
    return m_ypp * m_mapHeight;
}

a2dBoundingBox a2dDrawer2D::GetVisibleBbox() const
{
    return a2dBoundingBox( m_virt_minX, m_virt_minY, m_virt_minX + m_xpp * m_mapWidth, m_virt_minY + m_ypp * m_mapHeight );
}

void a2dDrawer2D::SetYaxis( bool up )
{
    m_yaxis = up;
    SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, m_virt_minX, m_virt_minY, m_xpp, m_ypp );
}

void a2dDrawer2D::SetYaxisDevice( bool up )
{
    m_yaxisDevice = up;
    SetMappingUpp( m_mapX, m_mapY, m_mapWidth, m_mapHeight, m_virt_minX, m_virt_minY, m_xpp, m_ypp );
}

void a2dDrawer2D::SetTransform( const a2dAffineMatrix& userToWorld )
{
    m_usertoworld = userToWorld;
    m_usertodevice = m_worldtodevice * m_usertoworld;
}

void a2dDrawer2D::PushTransform()
{
    m_affineStack.push_back( m_usertoworld );
    m_affineStack.push_back( m_worldtodevice );
    m_affineStack.push_back( m_usertodevice );
}

void a2dDrawer2D::PushTransform( const a2dAffineMatrix& affine )
{
    m_affineStack.push_back( m_usertoworld );
    m_affineStack.push_back( m_worldtodevice );
    m_affineStack.push_back( m_usertodevice );

    m_usertoworld = m_usertoworld * affine;
    m_usertodevice = m_worldtodevice * m_usertoworld;
}

void a2dDrawer2D::PushIdentityTransform()
{
    m_affineStack.push_back( m_usertoworld );
    m_affineStack.push_back( m_worldtodevice );
    m_affineStack.push_back( m_usertodevice );

    m_usertoworld = a2dIDENTITY_MATRIX;
    m_usertodevice = a2dIDENTITY_MATRIX;
    m_worldtodevice = a2dIDENTITY_MATRIX;
}

void a2dDrawer2D::PopTransform( void )
{
    wxASSERT_MSG( !m_affineStack.empty(), _( "Unbalanced Push/PopTransform" ) );
    m_usertodevice = m_affineStack.back();
    m_affineStack.pop_back();
    m_worldtodevice = m_affineStack.back();
    m_affineStack.pop_back();
    m_usertoworld = m_affineStack.back();
    m_affineStack.pop_back();
}

a2dBoundingBox a2dDrawer2D::ToWorld( const wxRect& rect )
{
    return a2dBoundingBox( DeviceToWorldX( rect.x ),
                           DeviceToWorldY( rect.y ),
                           DeviceToWorldX( rect.x + rect.width ),
                           DeviceToWorldY( rect.y + rect.height ) );
}

wxRect a2dDrawer2D::ToDevice( const a2dBoundingBox& bbox )
{
    int xd1, yd1, xd2, yd2;
    xd1 = WorldToDeviceX( bbox.GetMinX() );
    yd1 = WorldToDeviceY( bbox.GetMinY() );
    xd2 = WorldToDeviceX( bbox.GetMaxX() );
    yd2 = WorldToDeviceY( bbox.GetMaxY() );

    if ( xd1 > xd2 )
    {
        int tmp = xd1;
        xd1 = xd2;
        xd2 = tmp;
    }
    if ( yd1 > yd2 )
    {
        int tmp = yd1;
        yd1 = yd2;
        yd2 = tmp;
    }

    wxRect tmparea;
    tmparea.x = xd1;
    tmparea.y = yd1;
    tmparea.width = xd2 - xd1;
    tmparea.height = yd2 - yd1;

    return tmparea;
}

wxPoint* a2dDrawer2D::_convertToIntPointCache( int n, wxRealPoint* pts )
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

//------------------------------------------------------------------------
// platform indepent style settings
//------------------------------------------------------------------------

void a2dDrawer2D::ResetStyle()
{
    m_fixedStyledOverRuled = false;
}

void a2dDrawer2D::SetOpacityFactor( wxUint8 OpacityFactor )
{
    m_OpacityFactor = OpacityFactor;
    SetActiveStroke( m_currentstroke );
    SetActiveFill( m_currentfill );
}

void a2dDrawer2D::SetDrawStyle( a2dDrawStyle drawstyle )
{
    if ( m_drawstyle != a2dFIX_STYLE && m_drawstyle != a2dFIX_STYLE_INVERT )
    {
        DoSetDrawStyle( drawstyle );
    }
}

void a2dDrawer2D::SetDrawerStroke( const a2dStroke& stroke )
{
    if ( m_drawstyle != a2dFIX_STYLE && m_drawstyle != a2dFIX_STYLE_INVERT )
    {
        //I prefer to see penwidth changing,
        //therefore not ignore ( m_drawstyle == a2dWIREFRAME_INVERT )

        if ( !stroke.IsSameAs( m_currentstroke ) )
        {
            //DEPENDING on the current drawstyle, make the wxDC in sync or not.
            m_currentstroke = stroke;
            SetActiveStroke( m_currentstroke );
        }
    }
}

void a2dDrawer2D::SetActiveStroke( const a2dStroke& stroke )
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

void a2dDrawer2D::SetDrawerFill( const a2dFill& fill )
{
    if ( m_drawstyle != a2dFIX_STYLE && m_drawstyle != a2dFIX_STYLE_INVERT )
    {
        if ( !fill.IsSameAs( m_currentfill ) )
        {
            m_currentfill = fill;
            SetActiveFill( m_currentfill );
        }
    }
}

void a2dDrawer2D::SetActiveFill( const a2dFill& fill )
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

void a2dDrawer2D::ResetFixedStyle()
{
    if ( m_drawstyle == a2dFIX_STYLE || m_drawstyle == a2dFIX_STYLE_INVERT )
    {
        m_drawstyle = a2dFILLED;
        ResetStyle();
        m_fixFillRestore = m_currentfill;
        m_fixStrokeRestore = m_currentstroke;
        m_fixDrawstyle = m_drawstyle;
        m_fixedStyledOverRuled = false;
    }
}

void a2dDrawer2D::OverRuleFixedStyle()
{
    if ( !m_fixedStyledOverRuled &&
            ( m_drawstyle == a2dFIX_STYLE || m_drawstyle == a2dFIX_STYLE_INVERT )
       )
    {
        m_fixFillRestore = m_currentfill;
        m_fixStrokeRestore = m_currentstroke;
        m_fixDrawstyle = m_drawstyle;
        m_drawstyle = a2dFILLED;
        m_fixedStyledOverRuled = true;
    }
}

void a2dDrawer2D::ReStoreFixedStyle()
{
    if ( m_fixedStyledOverRuled )
    {
        SetDrawerFill( m_fixFillRestore );
        SetDrawerStroke( m_fixStrokeRestore );
        SetDrawStyle( m_fixDrawstyle );
        m_fixedStyledOverRuled = false;
    }
}

void a2dDrawer2D::SetFont( const a2dFont& font )
{
    if ( !font.IsSameAs( m_currentfont ) )
    {
        m_currentfont = font;
    }
    DoSetActiveFont( m_currentfont );
}

wxPen a2dDrawer2D::ConvertActiveToPen()
{
    wxPen dcpen = *wxTRANSPARENT_PEN;
    wxColour stroke1;
    stroke1.Set( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );

    if ( m_activestroke.GetStyle() == a2dSTROKE_TRANSPARENT )
    {
        dcpen = *wxTRANSPARENT_PEN;
    }
    else if ( m_activestroke.IsNoStroke() )
    {
        dcpen = *wxTRANSPARENT_PEN;
    }
    else if ( m_drawstyle == a2dWIREFRAME_INVERT_ZERO_WIDTH )
    {
        dcpen = *wxBLACK_PEN;
    }
    else if ( m_activestroke.GetType() == a2dSTROKE_ONE_COLOUR )
    {
        wxPenStyle style;
        switch( m_activestroke.GetStyle() )
        {
            case a2dSTROKE_SOLID: style = wxPENSTYLE_SOLID;
                break;
            case a2dSTROKE_TRANSPARENT: style = wxPENSTYLE_TRANSPARENT;
                break;
            case a2dSTROKE_DOT: style = wxPENSTYLE_DOT;
                break;
            case a2dSTROKE_DOT_DASH: style = wxPENSTYLE_DOT_DASH;
                break;
            case a2dSTROKE_LONG_DASH: style = wxPENSTYLE_LONG_DASH;
                break;
            case a2dSTROKE_SHORT_DASH: style = wxPENSTYLE_SHORT_DASH;
                break;
            case a2dSTROKE_BDIAGONAL_HATCH: style = wxPENSTYLE_BDIAGONAL_HATCH;
                break;
            case a2dSTROKE_CROSSDIAG_HATCH: style = wxPENSTYLE_CROSSDIAG_HATCH;
                break;
            case a2dSTROKE_FDIAGONAL_HATCH: style = wxPENSTYLE_FDIAGONAL_HATCH;
                break;
            case a2dSTROKE_CROSS_HATCH: style = wxPENSTYLE_CROSS_HATCH;
                break;
            case a2dSTROKE_HORIZONTAL_HATCH: style = wxPENSTYLE_HORIZONTAL_HATCH;
                break;
            case a2dSTROKE_VERTICAL_HATCH: style = wxPENSTYLE_VERTICAL_HATCH;
                break;
            case a2dSTROKE_STIPPLE: style = wxPENSTYLE_STIPPLE;
                break;
            case a2dSTROKE_STIPPLE_MASK_OPAQUE: style = wxPENSTYLE_STIPPLE_MASK_OPAQUE;
                break;
            default: style = wxPENSTYLE_SOLID;
        }

        if ( m_drawstyle == a2dWIREFRAME_ZERO_WIDTH )
        {
            dcpen = wxPen( stroke1, 0, style );
        }
        else
        {
            dcpen = wxPen( stroke1, ( int ) m_activestroke.GetWidth(), style );
            dcpen.SetJoin( m_activestroke.GetJoin() );
            dcpen.SetCap( m_activestroke.GetCap() );
            if ( m_activestroke.GetPixelStroke() )
                dcpen.SetWidth( ( int ) DeviceToWorldXRel( m_activestroke.GetWidth() ) );
        }
    }
    else if ( m_activestroke.GetType() == a2dSTROKE_BITMAP )
    {
        if ( m_drawstyle == a2dWIREFRAME_ZERO_WIDTH )
            dcpen = wxPen( stroke1, 0, wxPENSTYLE_SOLID );
        else
        {
            int width = ( int ) m_activestroke.GetWidth();
            if ( m_activestroke.GetPixelStroke() )
                width = ( int ) DeviceToWorldXRel( width );
            dcpen = wxPen( stroke1, width, wxPENSTYLE_SOLID );
            dcpen.SetJoin( m_activestroke.GetJoin() );
            dcpen.SetCap( m_activestroke.GetCap() );
        }

#if defined(__WXMSW__)
        wxPenStyle style;
        switch( m_activestroke.GetStyle() )
        {
            case a2dSTROKE_STIPPLE:
                style = wxPENSTYLE_STIPPLE;
                dcpen.SetStyle( style );
                dcpen.SetStipple( m_activestroke.GetStipple() );
                break;
            case a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT:
            case a2dSTROKE_STIPPLE_MASK_OPAQUE:
                if ( 0 ) //m_printingMode )
                {
                    wxBitmap noMask = m_activestroke.GetStipple();
                    noMask.SetMask( NULL );
                    style = wxPENSTYLE_STIPPLE;
                    dcpen.SetStyle( style );
                    dcpen.SetStipple( noMask );
                }
                else
                {
                    style = wxPENSTYLE_STIPPLE_MASK_OPAQUE;
                    dcpen.SetStyle( style );
                    dcpen.SetStipple( m_activestroke.GetStipple() );
                }
                break;
            default: style = wxPENSTYLE_SOLID;
                dcpen.SetStyle( style );
        }
#endif

    }
    return dcpen;
}

wxBrush a2dDrawer2D::ConvertActiveToBrush()
{
    wxBrush dcbrush = *wxTRANSPARENT_BRUSH;
    wxColour textb;
    wxBrushStyle style;
    wxColour fill1;
    fill1.Set( m_colour1redFill, m_colour1greenFill, m_colour1blueFill, m_FillOpacityCol1 );
    wxColour fill2;
    fill2.Set( m_colour2redFill, m_colour2greenFill, m_colour2blueFill, m_FillOpacityCol2 );

    if ( !m_activefill.GetFilling() )
    {
        dcbrush = *wxTRANSPARENT_BRUSH;
        textb = *wxBLACK;
    }
    else if ( m_activefill.GetStyle() == a2dFILL_TRANSPARENT )
    {
        dcbrush = *wxTRANSPARENT_BRUSH;
        textb = *wxBLACK;
    }
    else if ( m_activefill.IsNoFill() )
    {
        dcbrush = *wxTRANSPARENT_BRUSH;
        textb = *wxBLACK;
    }
    else if ( m_activefill.GetType() == a2dFILL_ONE_COLOUR )
    {
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_TRANSPARENT:
                style = wxBRUSHSTYLE_TRANSPARENT;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_SOLID: style = wxBRUSHSTYLE_SOLID;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_BDIAGONAL_HATCH:
                style = wxBRUSHSTYLE_BDIAGONAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_CROSSDIAG_HATCH:
                style = wxBRUSHSTYLE_CROSSDIAG_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_FDIAGONAL_HATCH:
                style = wxBRUSHSTYLE_FDIAGONAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_CROSS_HATCH:
                style = wxBRUSHSTYLE_CROSS_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_HORIZONTAL_HATCH:
                style = wxBRUSHSTYLE_HORIZONTAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_VERTICAL_HATCH:
                style = wxBRUSHSTYLE_VERTICAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            default: style = wxBRUSHSTYLE_SOLID;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
        }
    }
    else if ( m_activefill.GetType() == a2dFILL_HATCH_TWO_COLOUR )
    {
        wxBrushStyle style;
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_TRANSPARENT:
                style = wxBRUSHSTYLE_TRANSPARENT;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_BDIAGONAL_HATCH:
                style = wxBRUSHSTYLE_BDIAGONAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_CROSSDIAG_HATCH:
                style = wxBRUSHSTYLE_CROSSDIAG_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_FDIAGONAL_HATCH:
                style = wxBRUSHSTYLE_FDIAGONAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_CROSS_HATCH:
                style = wxBRUSHSTYLE_CROSS_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_HORIZONTAL_HATCH:
                style = wxBRUSHSTYLE_HORIZONTAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_VERTICAL_HATCH:
                style = wxBRUSHSTYLE_VERTICAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            default:
                style = wxBRUSHSTYLE_SOLID;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
        }
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP )
    {
        wxBrushStyle style;
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_STIPPLE:
                style = wxBRUSHSTYLE_STIPPLE;
                dcbrush.SetColour( fill1 );
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
                    dcbrush.SetColour( fill1 );
                    dcbrush.SetStipple( noMask );
                }
                else
                {
                    style = wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE;
                    dcbrush.SetColour( fill1 );
                    dcbrush.SetStipple( m_activefill.GetStipple() );
                }
                dcbrush.SetStyle( style );
                break;
            default: style = wxBRUSHSTYLE_SOLID;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
        }
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR || m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        wxFAIL_MSG( wxT( "can not convert gradients to wxBrush" ) );
    }
    return dcbrush;
}

#if wxART2D_USE_GRAPHICS_CONTEXT

wxGraphicsPen a2dDrawer2D::ConvertActiveToGraphicsPen( wxGraphicsContext* context )
{
    wxPen dcpen = *wxTRANSPARENT_PEN;
    wxColour stroke1;
    stroke1.Set( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );

    if ( m_activestroke.GetStyle() == a2dSTROKE_TRANSPARENT )
    {
        dcpen = *wxTRANSPARENT_PEN;
    }
    else if ( m_activestroke.IsNoStroke() )
    {
        dcpen = *wxTRANSPARENT_PEN;
    }
    else if ( m_drawstyle == a2dWIREFRAME_INVERT_ZERO_WIDTH )
    {
        dcpen = *wxBLACK_PEN;
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
            dcpen = wxPen( stroke1, 0, style );
        }
        else
        {
            dcpen = wxPen( stroke1, ( int ) m_activestroke.GetWidth(), style );
            dcpen.SetJoin( m_activestroke.GetJoin() );
            dcpen.SetCap( m_activestroke.GetCap() );
            if ( m_activestroke.GetPixelStroke() )
                dcpen.SetWidth( DeviceToWorldXRel( m_activestroke.GetWidth() ) );
        }
    }
    else if ( m_activestroke.GetType() == a2dSTROKE_BITMAP )
    {
        if ( m_drawstyle == a2dWIREFRAME_ZERO_WIDTH )
            dcpen = wxPen( stroke1, 0, wxBRUSHSTYLE_SOLID );
        else
        {
            int width = ( int ) m_activestroke.GetWidth();
            if ( m_activestroke.GetPixelStroke() )
                width = DeviceToWorldXRel( width );
            dcpen = wxPen( stroke1, width, wxBRUSHSTYLE_SOLID );
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

    }
    return context->CreatePen( dcpen );
}

wxGraphicsBrush a2dDrawer2D::ConvertActiveToGraphicsBrush( wxGraphicsContext* context )
{
    wxBrush dcbrush = *wxTRANSPARENT_BRUSH;
    wxColour textb;
    wxBrushStyle style;
    wxColour fill1;
    fill1.Set( m_colour1redFill, m_colour1greenFill, m_colour1blueFill, m_FillOpacityCol1 );
    wxColour fill2;
    fill2.Set( m_colour2redFill, m_colour2greenFill, m_colour2blueFill, m_FillOpacityCol2 );

    if ( !m_activefill.GetFilling() )
    {
        dcbrush = *wxTRANSPARENT_BRUSH;
        textb = *wxBLACK;
        return context->CreateBrush( dcbrush );
    }
    else if ( m_activefill.GetStyle() == a2dFILL_TRANSPARENT )
    {
        dcbrush = *wxTRANSPARENT_BRUSH;
        textb = *wxBLACK;
        return context->CreateBrush( dcbrush );
    }
    else if ( m_activefill.IsNoFill() )
    {
        dcbrush = *wxTRANSPARENT_BRUSH;
        textb = *wxBLACK;
        return context->CreateBrush( dcbrush );
    }
    else if ( m_activefill.GetType() == a2dFILL_ONE_COLOUR )
    {
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_TRANSPARENT:
                style = wxBRUSHSTYLE_TRANSPARENT;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_SOLID: style = wxBRUSHSTYLE_SOLID;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_BDIAGONAL_HATCH:
                style = wxBRUSHSTYLE_BDIAGONAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_CROSSDIAG_HATCH:
                style = wxBRUSHSTYLE_CROSSDIAG_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_FDIAGONAL_HATCH:
                style = wxBRUSHSTYLE_FDIAGONAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_CROSS_HATCH:
                style = wxBRUSHSTYLE_CROSS_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_HORIZONTAL_HATCH:
                style = wxBRUSHSTYLE_HORIZONTAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_VERTICAL_HATCH:
                style = wxBRUSHSTYLE_VERTICAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            default: style = wxBRUSHSTYLE_SOLID;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
        }
        return context->CreateBrush( dcbrush );
    }
    else if ( m_activefill.GetType() == a2dFILL_HATCH_TWO_COLOUR )
    {
        wxBrushStyle style;
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_TRANSPARENT:
                style = wxBRUSHSTYLE_TRANSPARENT;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_BDIAGONAL_HATCH:
                style = wxBRUSHSTYLE_BDIAGONAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_CROSSDIAG_HATCH:
                style = wxBRUSHSTYLE_CROSSDIAG_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_FDIAGONAL_HATCH:
                style = wxBRUSHSTYLE_FDIAGONAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_CROSS_HATCH:
                style = wxBRUSHSTYLE_CROSS_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_HORIZONTAL_HATCH:
                style = wxBRUSHSTYLE_HORIZONTAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            case a2dFILL_TWOCOL_VERTICAL_HATCH:
                style = wxBRUSHSTYLE_VERTICAL_HATCH;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
                break;
            default:
                style = wxBRUSHSTYLE_SOLID;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
        }
        return context->CreateBrush( dcbrush );
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP )
    {
        wxBrushStyle style;
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_STIPPLE:
                style = wxBRUSHSTYLE_STIPPLE;
                dcbrush.SetColour( fill1 );
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
                    dcbrush.SetColour( fill1 );
                    dcbrush.SetStipple( noMask );
                }
                else
                {
                    style = wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE;
                    dcbrush.SetColour( fill1 );
                    dcbrush.SetStipple( m_activefill.GetStipple() );
                }
                dcbrush.SetStyle( style );
                break;
            default: style = wxBRUSHSTYLE_SOLID;
                dcbrush.SetColour( fill1 );
                dcbrush.SetStyle( style );
        }
        return context->CreateBrush( dcbrush );
    }

    if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
    {
        double x1 = m_activefill.GetStart().m_x;
        double y1 = m_activefill.GetStart().m_y;
        double x2 = m_activefill.GetStop().m_x;
        double y2 = m_activefill.GetStop().m_y;
        return context->CreateLinearGradientBrush( x1, y1, x2, y2, fill1, fill2 );
    }
    if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        double xf = m_activefill.GetFocal().m_x;
        double yf = m_activefill.GetFocal().m_y;
        double xc = m_activefill.GetCenter().m_x;
        double yc = m_activefill.GetCenter().m_y;
        double r = m_activefill.GetRadius();
        return context->CreateRadialGradientBrush( xf, yf, xc, yc, r, fill1, fill2 );
    }
    return wxNullGraphicsBrush;
}

#endif  // wxART2D_USE_GRAPHICS_CONTEXT

//------------------------------------------------------------------------
// basic platform independent drawing of primitives
//------------------------------------------------------------------------
int a2dDrawer2D::ToDeviceLines( a2dVertexArray* points, a2dBoundingBox& devbbox, bool& smallPoly, bool replaceByRectangle )
{
    unsigned int segments = 0;

    unsigned int i;
    for ( i = 0; i < points->size(); i++ )
    {
        const a2dLineSegmentPtr seg = points->Item( i );

        if ( seg->GetArc() )
        {
            const a2dArcSegment* cseg = ( const a2dArcSegment* ) seg.Get();

            double radius, center_x, center_y, beginrad, midrad, endrad, phit;

            if ( cseg->CalcR( *( points->Item( i ? i - 1 : 0 ) ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
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
        const a2dLineSegmentPtr seg = points->Item( i );

        if ( !seg->GetArc() )
        {
            GetUserToDeviceTransform().TransformPoint( seg->m_x, seg->m_y, x, y );

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
            const a2dArcSegment* cseg = ( const a2dArcSegment* ) seg.Get();

            double radius, center_x, center_y, beginrad, midrad, endrad, phit;

            if ( cseg->CalcR( *( points->Item( i ? i - 1 : 0 ) ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
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
                    GetUserToDeviceTransform().TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );

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
                GetUserToDeviceTransform().TransformPoint( cseg->m_x, cseg->m_y, x, y );

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

    if ( devbbox.GetSize() < m_polygonFillThreshold )
    {
        smallPoly = true;
        if ( replaceByRectangle )
        {
            m_cpointsDouble[0].x = devbbox.GetMinX(); m_cpointsDouble[0].y = devbbox.GetMinY();
            m_cpointsDouble[1].x = devbbox.GetMinX(); m_cpointsDouble[1].y = devbbox.GetMaxY();
            m_cpointsDouble[2].x = devbbox.GetMaxX(); m_cpointsDouble[2].y = devbbox.GetMaxY();
            m_cpointsDouble[3].x = devbbox.GetMaxX(); m_cpointsDouble[3].y = devbbox.GetMinY();
            return 4;
        }
    }
    return count;
}

int a2dDrawer2D::ToDeviceLines( std::vector<wxRealPoint>& cpointsDouble, const a2dVertexList* list, a2dBoundingBox& devbbox, bool& smallPoly, bool replaceByRectangle )
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
    cpointsDouble.resize( segments * 2 );

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
            GetUserToDeviceTransform().TransformPoint( seg->m_x, seg->m_y, x, y );

            //to reduce the points count for lines or a polygon on the screen
            if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
            {
                cpointsDouble[count].x = x;
                cpointsDouble[count].y = y;
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
                    GetUserToDeviceTransform().TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );

                    //to reduce the points count for lines or a polygon on the screen
                    if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                    {
                        cpointsDouble[count].x = x;
                        cpointsDouble[count].y = y;
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
                GetUserToDeviceTransform().TransformPoint( cseg->m_x, cseg->m_y, x, y );

                //to reduce the points count for lines or a polygon on the screen
                if( !count || fabs( x - lastx ) > 0.001  ||  fabs( y - lasty ) > 0.001 )
                {
                    cpointsDouble[count].x = x;
                    cpointsDouble[count].y = y;
                    lastx = x;
                    lasty = y;
                    devbbox.Expand( x, y );
                    count++;
                }
            }
        }
        iterprev = iter++;
    }

    if ( devbbox.GetSize() < m_polygonFillThreshold )
    {
        smallPoly = true;
        if ( replaceByRectangle )
        {
            cpointsDouble[0].x = devbbox.GetMinX(); cpointsDouble[0].y = devbbox.GetMinY();
            cpointsDouble[1].x = devbbox.GetMinX(); cpointsDouble[1].y = devbbox.GetMaxY();
            cpointsDouble[2].x = devbbox.GetMaxX(); cpointsDouble[2].y = devbbox.GetMaxY();
            cpointsDouble[3].x = devbbox.GetMaxX(); cpointsDouble[3].y = devbbox.GetMinY();
            return 4;
        }
    }
    return count;
}

int a2dDrawer2D::ToDeviceLines( const a2dVertexList* list, a2dBoundingBox& devbbox, bool& smallPoly, bool replaceByRectangle )
{
    return ToDeviceLines( m_cpointsDouble, list, devbbox, smallPoly, replaceByRectangle );
}

void a2dDrawer2D::DrawVpath( const a2dVpath* path )
{
    if ( m_disableDrawing )
        return;

    double tstep = 1 / ( double ) SPLINE_STEP;
    unsigned int i;

    m_tempPoints.clear();
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
                    m_tempPoints.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
                    count++;
                }
                else
                {
                    i--;
                    move = true;
                }
                break;

            case a2dPATHSEG_LINETO:
                m_tempPoints.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
                count++;
                break;
            case a2dPATHSEG_LINETO_NOSTROKE:
                m_tempPoints.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
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
                a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                    yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
                    m_tempPoints.push_back( new a2dLineSegment( xw, yw ) );
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
                    m_tempPoints.push_back( new a2dLineSegment( xw, yw ) );
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
                        m_tempPoints.push_back( new a2dLineSegment( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ) ) );
                        count++;
                        theta = theta + dphi;
                    }
                }
                else
                {
                    m_tempPoints.push_back( new a2dLineSegment( cseg->m_x1, cseg->m_y1 ) );
                    count++;
                }
            }
            default:
                break;
                break;
        }

        if ( move )
        {
            DrawLines( &m_tempPoints, false );
            move = false;
            count = 0;
            m_tempPoints.clear();
        }
        else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
        {
            if ( nostrokeparts || seg->GetClose() == a2dPATHSEG_END_CLOSED_NOSTROKE )
            {
                a2dStroke stroke = m_activestroke;
                SetDrawerStroke( *a2dTRANSPARENT_STROKE );
                DrawPolygon( &m_tempPoints, false, wxODDEVEN_RULE );
                SetDrawerStroke( stroke );
                nostrokeparts = true;
            }
            else
            {
                DrawPolygon( &m_tempPoints, false, wxODDEVEN_RULE );
            }

            move = false;
            count = 0;
            m_tempPoints.clear();
        }
        else if ( i == path->size() - 1 )  //last segment?
        {
            DrawLines( &m_tempPoints, false );
        }
    }

    if ( nostrokeparts )
    {
        m_tempPoints.clear();
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
                        m_tempPoints.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
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
                    m_tempPoints.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
                    count++;
                    break;

                case a2dPATHSEG_LINETO_NOSTROKE:
                case a2dPATHSEG_CBCURVETO_NOSTROKE:
                case a2dPATHSEG_QBCURVETO_NOSTROKE:
                case a2dPATHSEG_ARCTO_NOSTROKE:
                    if ( count == 0 )
                    {
                        m_tempPoints.push_back( new a2dLineSegment( seg->m_x1, seg->m_y1 ) );
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
                    a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                        yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
                        m_tempPoints.push_back( new a2dLineSegment( xw, yw ) );
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
                    a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                        yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
                        m_tempPoints.push_back( new a2dLineSegment( xw, yw ) );
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
                            m_tempPoints.push_back( new a2dLineSegment( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ) ) );
                            count++;
                            theta = theta + dphi;
                        }
                    }
                    else
                    {
                        m_tempPoints.push_back( new a2dLineSegment( cseg->m_x1, cseg->m_y1 ) );
                        count++;
                    }
                }
                default:
                    break;

                    break;
            }

            if ( move || nostrokeparts )
            {
                DrawLines( &m_tempPoints, false );
                move = false;
                nostrokeparts = false;
                m_tempPoints.clear();
                count = 0;
            }
            else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
            {
                if ( seg->GetClose() == a2dPATHSEG_END_CLOSED )
                {
                    m_tempPoints.push_back( new a2dLineSegment( lastmovex, lastmovey ) );
                    count++;
                }
                DrawLines( &m_tempPoints, false );
                nostrokeparts = false;
                move = false;
                m_tempPoints.clear();
                count = 0;
            }
            else if ( i == path->size() )
            {
                DrawLines( &m_tempPoints, false );
            }
        }
    }
}

void a2dDrawer2D::DrawPolyPolygon( a2dListOfa2dVertexList polylist, wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    for( a2dListOfa2dVertexList::iterator iterp = polylist.begin(); iterp != polylist.end(); iterp++ )
    {
        DrawPolygon( *iterp, false, fillStyle );
    }
}

void a2dDrawer2D::DrawPolygon( a2dVertexArray* points, bool spline, wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    a2dBoundingBox devbox;
    bool smallPoly = false;
    int segments = ToDeviceLines( points, devbox, smallPoly );

    if ( segments == 0 )
        return;

    if ( smallPoly )
    {
        DeviceDrawLines( segments, spline );
        DeviceDrawLine( m_cpointsDouble[0].x, m_cpointsDouble[0].y, m_cpointsDouble[segments - 1].x, m_cpointsDouble[segments - 1].y );
    }
    else
        DeviceDrawPolygon( segments, spline, fillStyle );
}

void a2dDrawer2D::DrawPolygon( const a2dVertexList* list, bool spline, wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    a2dBoundingBox devbox;
    bool smallPoly = false;
    int segments = ToDeviceLines( list, devbox, smallPoly );

    if ( segments == 0 )
        return;

    if ( smallPoly )
    {
        DeviceDrawLines( segments, spline );
        DeviceDrawLine( m_cpointsDouble[0].x, m_cpointsDouble[0].y, m_cpointsDouble[segments - 1].x, m_cpointsDouble[segments - 1].y );
    }
    else
        DeviceDrawPolygon( segments, spline, fillStyle );
}

void a2dDrawer2D::DrawLines( a2dVertexArray* points, bool spline )
{
    if ( m_disableDrawing )
        return;

    a2dBoundingBox devbox;
    bool smallPoly = false;
    int segments = ToDeviceLines( points, devbox, smallPoly );

    if ( segments > 0 )
        DeviceDrawLines( segments, spline );
}

void a2dDrawer2D::DrawLines( const a2dVertexList* list, bool spline )
{
    if ( m_disableDrawing )
        return;

    a2dBoundingBox devbox;
    bool smallPoly = false;
    int segments = ToDeviceLines( list, devbox, smallPoly );

    if ( segments > 0 )
        DeviceDrawLines( segments, spline );
}

void a2dDrawer2D::DrawArc( double x1, double y1, double x2, double y2, double xc, double yc, bool chord )
{
    if ( m_disableDrawing )
        return;

    double dx = x1 - xc;
    double dy = y1 - yc;

    double start;
    double end;
    double radius = sqrt( dx * dx + dy * dy );

    if ( radius < a2dACCUR )
        return;

    if ( x1 == x2 && y1 == y2 )
    {
        start = 0;
        end =  2.0 * wxPI;
    }
    else
    {
        if ( !dx && !dy )
            start =  0;
        else
            start =  atan2( dy, dx );

        dx = x2 - xc;
        dy = y2 - yc;

        if ( !dx && !dy )
            end = 0;
        else
            end = atan2( dy, dx );

        end = end - start;
        if ( end <= 0 )   end += 2.0 * wxPI;
        if ( start < 0 )  start += 2.0 * wxPI;
        end = end + start;
    }

    DrawEllipticArc( xc, yc, 2.0 * radius, 2.0 * radius, start * 180.0 / wxPI, end * 180.0 / wxPI, chord );
}

void a2dDrawer2D::DrawEllipticArc( double xc, double yc, double width, double height, double sa, double ea, bool chord )
{
    if ( m_disableDrawing )
        return;

    double start, end;

    start = wxDegToRad( sa );
    end = wxDegToRad( ea );

    end = end - start;
    if ( end <= 0 )   end += 2.0 * wxPI;

    if ( start < 0 )  start += 2.0 * wxPI;

    end = end + start;

    double dphi;
    unsigned int segments = 20;

    //circular approximation of radius.
    double radiusDev = m_usertodevice.TransformDistance( wxMax( width, height ) );
    Aberration( m_displayaberration, end - start, radiusDev , dphi, segments );

    m_tempPoints.clear();
    double theta = start;
    unsigned int i;
    for ( i = 0; i < segments + 1; i++ )
    {
        m_tempPoints.push_back( new a2dLineSegment( xc + width / 2 * cos ( theta ), yc + height / 2 * sin ( theta ) ) );
        theta = theta + dphi;
    }

    if ( !chord )
    {
        m_tempPoints.push_back( new a2dLineSegment( xc, yc ) );
        DrawPolygon( &m_tempPoints );
    }
    else
        DrawLines( &m_tempPoints );
}

void a2dDrawer2D::DrawCenterRoundedRectangle( double xc, double yc, double width, double height, double radius, bool pixelsize )
{
    if ( m_disableDrawing )
        return;

    DrawRoundedRectangle( xc - width / 2, yc - height / 2, width, height, radius, pixelsize );
}

void a2dDrawer2D::DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize )
{
    if ( m_disableDrawing )
        return;

    if ( pixelsize )
    {
        width = DeviceToWorldXRel( width );
        height = DeviceToWorldYRel( height );
        radius = DeviceToWorldXRel( radius );
        if( m_yaxis )
            height = -height;
    }

    if ( radius )
    {
        static const double sintab[9] =
        {
            1 - 0,
            1 - 0.19509032201612826784828486847702,
            1 - 0.38268343236508977172845998403040,
            1 - 0.55557023301960222474283081394853,
            1 - 0.70710678118654752440084436210485,
            1 - 0.83146961230254523707878837761791,
            1 - 0.92387953251128675612818318939679,
            1 - 0.98078528040323044912618223613424,
            1 - 1
        };

        static const double costab[9] =
        {
            1, 
            0.98078528040323044912618223613424,
            0.92387953251128675612818318939679,
            0.83146961230254523707878837761791,
            0.70710678118654752440084436210485,
            0.55557023301960222474283081394853,
            0.38268343236508977172845998403040,
            0.19509032201612826784828486847702,
            0,
        };

        m_tempPoints.clear();
        int i;
        if ( height  > 0 )
        {
            if ( width > 0 )
            {
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + radius * sintab[i], y + radius * sintab[8 - i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + radius * sintab[8 - i], y + height - radius * sintab[i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + width - radius * sintab[i], y + height - radius * sintab[8 - i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + width - radius * sintab[8 - i], y + radius * sintab[i] ) );
            }
            else
            {
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x - radius * sintab[i], y + radius * sintab[8 - i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x - radius * sintab[8 - i], y + height - radius * sintab[i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + width + radius * sintab[i], y + height - radius * sintab[8 - i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + width + radius * sintab[8 - i], y + radius * sintab[i] ) );
            }
        }
        else 
        {
            if ( width > 0 )
            {
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + radius * sintab[8-i], y - radius * sintab[i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + width - radius * sintab[i], y - radius * sintab[8-i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + width - radius * sintab[8-i], y + height + radius * sintab[i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + radius * sintab[i], y + height + radius * sintab[8-i] ) );
            }
            else
            {
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x - radius + radius * costab[8-i], y - radius + radius * costab[i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x - radius + radius * costab[i], y + height + radius - radius * costab[8-i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + width + radius - radius * costab[8-i], y + height + radius - radius * costab[i] ) );
                for( i = 0; i <= 8; i++ )
                    m_tempPoints.push_back( new a2dLineSegment( x + width + radius - radius * costab[i], y - radius + radius * costab[8-i] ) );
            }
        }

        DrawPolygon( &m_tempPoints );
    }
    else
    {
		//in a rectangle of say 5 by 5, the last pixel is 4,4 but that should be handle at device level i think, not here in world coordinates
        m_tempPoints.clear();
        m_tempPoints.push_back( new a2dLineSegment( x, y ) );
        m_tempPoints.push_back( new a2dLineSegment( x, y + height ) );
        m_tempPoints.push_back( new a2dLineSegment( x + width, y + height ) );
        m_tempPoints.push_back( new a2dLineSegment( x + width, y ) );
        DrawPolygon( &m_tempPoints );
    }
}

void a2dDrawer2D::DrawCircle( double xc, double yc, double radius )
{
    DrawEllipse( xc, yc, 2.0 * radius, 2.0 * radius );
}

void a2dDrawer2D::DrawEllipse( double xc, double yc, double width, double height )
{
    if ( m_disableDrawing )
        return;

    double dphi;
    unsigned int segments = 20;

    if ( width == 0 && height == 0 )
    {
        DrawPoint( xc, yc );
        return;
    }

    //circular approximation of radius.
    double radiusDev = m_usertodevice.TransformDistance( wxMax( width, height ) );
    Aberration( m_displayaberration, wxPI * 2, radiusDev , dphi, segments );

    /* limit the number of segments on a circle
        if the circle is small only a few segments need to be drawn.
        ie for a circle of 4x4 pixels, roughly 8 segments are needed
        The number of segments could be tuned with a fiddle factor (now 1.0).
        n = (width_pixels + height_pixels) * fiddle_factor;
     */
    double m00 = m_usertodevice.GetValue( 0, 0 ) * width;
    double m01 = m_usertodevice.GetValue( 0, 1 ) * width;
    double m10 = m_usertodevice.GetValue( 1, 0 ) * height;
    double m11 = m_usertodevice.GetValue( 1, 1 ) * height;
    double n = fabs( m00 + m10 ) + fabs( m01 + m11 ); // * fiddle_factor
    if ( segments > n )
    {
        segments = ( int ) ceil( n );
        dphi = 2.0 * wxPI / segments;
    }

    m_tempPoints.clear();
    double theta;
    unsigned int i;
    for ( i = 0; i < segments ; i++ )
    {
        theta = i * dphi;
        m_tempPoints.push_back( new a2dLineSegment( xc + width / 2 * cos ( theta ), yc - height / 2 * sin ( theta ) ) );
    }
    DrawPolygon( &m_tempPoints );
}

void a2dDrawer2D::DrawLine( double x1, double y1, double x2, double y2 )
{
    if ( m_disableDrawing )
        return;

    m_tempPoints.clear();
    m_tempPoints.push_back( new a2dLineSegment( x1, y1 ) );
    m_tempPoints.push_back( new a2dLineSegment( x2, y2 ) );
    DrawLines( &m_tempPoints );
}

void a2dDrawer2D::DrawCharUnknown( wxChar c )
{
    // set text linestroke.
    a2dStroke stroke = a2dStroke( m_activestroke.GetColour(), m_currentfont.GetSize() / 20.0 );
    a2dStroke oldstroke = m_activestroke;
    SetActiveStroke( stroke );

    if ( c != wxChar( ' ' ) )
    {
        double desc = m_currentfont.GetDescent();
        double size = m_currentfont.GetSize();
        double width = m_currentfont.GetWidth( c );
        DrawLine( 0.1 * width, desc, 0.9 * width, desc + size );
        DrawLine( 0.9 * width, desc, 0.1 * width, desc + size );
    }

    // restore context
    SetActiveStroke( oldstroke );
}

void a2dDrawer2D::DrawTextUnknown( const wxString& text, double x, double y, bool words )
{
    if ( m_disableDrawing )
        return;

    const double h = m_currentfont.GetLineHeight();
    x = y = 0;
    if ( words && m_currentfont.Ok() )
    {
        // Draw a piece of line for each word.
        // set text linestroke.
        a2dStroke strokew = a2dStroke( m_activestroke.GetColour(), 1 );
        a2dStroke oldstroke = m_activestroke;
        SetActiveStroke( strokew );

        double w;
        wxChar c;
        double start = x;
        for ( size_t i = 0; i < text.Length(); i++ )
        {
            c = text[i];
            w = m_currentfont.GetWidth( c );
            if ( c == wxChar( ' ' ) )
            {
                if ( x != start )
                    DrawLine( start, h / 2.0, x, h / 2.0 );
                start = x + w;
            }
            x += w;
        }
        if ( x != start )
            DrawLine( start, h / 2.0, x, h / 2.0 );

        // restore context
        SetActiveStroke( oldstroke );
    }
    else
    {
        a2dBoundingBox linebbox = m_currentfont.GetTextExtent( text, m_yaxis );

        DrawRoundedRectangle( linebbox.GetMinX(), linebbox.GetMinY(),
                              linebbox.GetWidth(), linebbox.GetHeight(), 0 );
        DrawLine( linebbox.GetMinX(), linebbox.GetMinY(), linebbox.GetMaxX(), linebbox.GetMaxY() );
        DrawLine( linebbox.GetMinX(), linebbox.GetMaxY(), linebbox.GetMaxX(), linebbox.GetMinY() );
    }
}

void a2dDrawer2D::DrawTextStroke( const wxString& text, double x, double y )
{
    // set text linestroke.
    //!todo what about the style (pixel or not of stroke font )
    a2dStroke oldstroke = m_activestroke;

    double width = m_currentfont.GetStrokeWidth();
    double widthdevice = m_usertodevice.TransformDistance( width );
    if ( widthdevice < 1 )//width 
    {
        a2dStroke stroke = a2dStroke( m_activestroke.GetColour(), 0 );
        SetDrawerStroke( stroke );
        //SetActiveStroke( stroke );
    }
    else
    {
        a2dStroke stroke = a2dStroke( m_activestroke.GetColour(), width );
        SetDrawerStroke( stroke );
        //SetActiveStroke( stroke );
    }
    DrawTextGeneric( text, x, y, &a2dDrawer2D::DrawCharStrokeCb ); 
    SetActiveStroke( oldstroke );
}

void a2dDrawer2D::DrawCharStroke( wxChar c )
{
    // scale character to size
    a2dAffineMatrix affine;
    affine.Scale( m_currentfont.GetSize() );
    PushTransform( affine );

    a2dVertexList** ptr = m_currentfont.GetGlyphStroke( c );
    if ( ptr )
    {
        while ( *ptr )
        {
            DrawLines( *ptr, false );
            ptr++;
        }
    }

    // restore context
    PopTransform();
}

void a2dDrawer2D::DrawTextGeneric( const wxString& text, double x, double y, void ( a2dDrawer2D::*drawchar )( wxChar ) )
{
    const double h = m_currentfont.GetLineHeight();
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
            m_usertoworld *= a2dAffineMatrix( w + m_currentfont.GetKerning( oldc, c ), 0.0 );
            m_usertodevice = m_worldtodevice * m_usertoworld;
        }
        w = m_currentfont.GetWidth( c );

        // get absolute character bounding box
        a2dBoundingBox bbox( 0.0, 0.0, w, h );
        bbox.MapBbox( m_usertoworld );

        // if character is not outside clipping box, draw it.
        if ( m_clipboxworld.Intersect( bbox ) != _OUT )
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
}

void a2dDrawer2D::DrawText( const wxString& text, double x, double y, int alignment, bool Background )
{
    if ( m_disableDrawing )
        return;

    // Get font size in device units, also takes stretching into account
    double dx, dy, size;
    dx = m_usertodevice.GetValue( 1, 0 );
    dy = m_usertodevice.GetValue( 1, 1 );
    size = m_currentfont.GetSize() * sqrt( dx * dx + dy * dy );
    //size = m_usertodevice.TransformDistance( m_a2dfont.GetSize() );

    if ( size > TEXT_MAXSIZE_INPIXELS )
        return;

    bool oldforce = m_forceNormalizedFont;
    // DC type fonts always needs to be set to the right pointsize.
    if ( m_currentfont.GetType() != a2dFONT_WXDC &&
         ( m_forceNormalizedFont || size > m_smallTextThreshold ) )
    {
        m_currentfont.SetDeviceHeight( NORMFONT );
        m_forceNormalizedFont = true;
    }
    else
        m_currentfont.SetDeviceHeight( size );

    // Get the bounding box, including alignment displacement.
    a2dBoundingBox bbox = m_currentfont.GetTextExtent( text, alignment );
    bbox.Translate( x, y );

    // Get unaligned coordinates for DrawTextXXX functions. By default these functions
    // use the lowerleft corner of the boundingbox, hence GetMinX() and GetMinY().
    x = bbox.GetMinX();
    y = bbox.GetMinY();

    // Transform user bbox to world bbox.
    a2dBoundingBox worldbbox( bbox );
    worldbbox.MapBbox( m_usertoworld );
    // If bbox not outside clipping area, draw the text
    if ( m_clipboxworld.Intersect( worldbbox ) != _OUT )
    {
        // Draw background fill
        if ( size > m_drawingthreshold &&
                Background && !GetDrawerFill().IsNoFill() && !GetDrawerFill().GetStyle() == a2dFILL_TRANSPARENT )
        {
            a2dStroke oldstroke = m_activestroke;
            SetDrawerStroke( *a2dTRANSPARENT_STROKE );
            DrawRoundedRectangle( bbox.GetMinX(), bbox.GetMinY(),
                                  bbox.GetWidth(), bbox.GetHeight(), 0 );
            SetDrawerStroke( oldstroke );
        }

        // mirror text, depending on y-axis orientation
        const double h = m_currentfont.GetLineHeight();
        a2dAffineMatrix affine;
        if ( !GetYaxis() )
        {
            affine.Translate( 0.0, -h );
            affine.Mirror( true, false );
        }
        // position text
        affine.Translate( x, y );
        PushTransform( affine );
        x = y = 0;

        // If text > threshold, draw text
        if ( size > m_drawingthreshold )
        {
            // Try to find the appropriate drawing function.
            switch ( m_currentfont.GetType() )
            {
                case a2dFONT_STROKED:
                    DrawTextStroke( text, x, y );
                    break;
                case a2dFONT_FREETYPE:
                    DrawTextFreetype( text, x, y );
                    break;
                case a2dFONT_WXDC:
                    DrawTextDc( text, x, y );
                    break;
                default:
                    DrawTextUnknown( text, x, y );
            }
        }
        // If text < threshold, draw a simple version.
        else
        {
            DrawTextUnknown( text, x, y, true );
        }

        PopTransform();

        /* DEBUG text boundingbox
                a2dStroke oldstroke = m_activestroke;
                SetDrawerStroke( *a2dBLACK_STROKE );
                DrawRoundedRectangle( bbox.GetMinX(), bbox.GetMinY(),
                                      bbox.GetWidth(), bbox.GetHeight(), 0 );
                SetDrawerStroke( oldstroke );
        */

    }

    m_forceNormalizedFont = oldforce;
    //m_currentfont.SetDeviceHeight( NORMFONT );

}

//use by polygon filling
//moves the scanline up
//index is the index of the point where the search begins
//direction is +1 or -1 and indicates if the segment ascends or decends
bool a2dDrawer2D::MoveUp( int n, wxRealPoint points[] , double horline, int& index, int direction )
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
void a2dDrawer2D::DetectCriticalPoints( int n, wxRealPoint points[] )
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

static int SortonXs( const void* f, const void* s )
{
     const a2dAET** first =(const a2dAET **)f;
     const a2dAET** second =(const a2dAET **)s;

     if ( ( *first )->m_xs < ( *second )->m_xs )
         return -1;
     else if ( ( *first )->m_xs > ( *second )->m_xs )
         return 1;

     return 0;
}

void a2dDrawer2D::ColourXYLinear( int x1, int x2, int y )
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

        a2dStroke current = GetDrawerStroke();
        wxColour gradcol( ( unsigned char ) ( m_colour1redFill + dred * curcol / stepcol ),
                          ( unsigned char ) ( m_colour1greenFill + dgreen * curcol / stepcol ),
                          ( unsigned char ) ( m_colour1blueFill + dblue * curcol / stepcol ) );
        a2dStroke gradientstroke = a2dStroke( gradcol, 0 );
        SetDrawerStroke( gradientstroke );
        DeviceDrawHorizontalLine( x1, y, x2, true );
        SetDrawerStroke( current );
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

void a2dDrawer2D::ColourXYRadial( int x1, int x2, int y )
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

void a2dDrawer2D::FillPolygon( int n, wxRealPoint points[] )
{
    wxRect clip;
    GetClippingBoxDev( clip.x, clip.y, clip.width, clip.height );


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

//-----------------------------------------------------------------------------------
//  Clipping region
//-----------------------------------------------------------------------------------

// Clipping of line in world coordinates
// Function return a value of where the point is in the space.
// The screen is the one with the double lines below (in the middle)
// If the given point is on the right of this window -> 2 is returned
int a2dDrawer2D::GetClipCode( double x, double y )
{
    //    9    |     8    |    10
    //         |          |
    //  ------============--------- Y-max
    //    1   ||     0   ||     2
    //        ||         ||
    //  ------============--------- Y-min
    //    5    |     4    |     6
    //         |          |
    //        X-min      X-max
    //
    int p = 0;

    if ( x < m_clipboxworld.GetMinX() )
        p |= 0x0001;
    else if ( x > m_clipboxworld.GetMaxX() )
        p |= 0x0002;
    if ( y < m_clipboxworld.GetMinY() )
        p |= 0x0004;
    else if ( y > m_clipboxworld.GetMaxY() )
        p |= 0x0008;

    return p;
}

// return true if the line is inside the window
bool a2dDrawer2D::Clipping( double& x1, double& y1, double& x2, double& y2 )
{
    int clipcode1, clipcode2;
    int nr_of_intersections = 0;

    // intersection points are used if the line falls from the screen
    double intersectionpoint1[2];
    double intersectionpoint2[2];
    double A, B, temppoint;
    double RC = 0;
    double T = 0;

    // call clipcode from each point from the line
    clipcode1 = GetClipCode( x1, y1 );
    clipcode2 = GetClipCode( x2, y2 );

    // are both points within the window?
    if ( !clipcode1 && !clipcode2 )    return true;
    // then draw the line

    // is point (x1,y1) inside the window?
    if ( !clipcode1 )
    {
        intersectionpoint1[0] = x1;
        intersectionpoint2[0] = y1;

        nr_of_intersections = 1;
    }

    // is point (x2,y2) inside the window?
    if ( !clipcode2 )
    {
        intersectionpoint1[0] = x2;
        intersectionpoint2[0] = y2;

        nr_of_intersections = 1;
    }

    clipcode1 ^= clipcode2;

    // We can make the following linefunction:
    //     y = (A/B)x + C
    // calculate the slope of the line
    A = y2 - y1;
    B = x2 - x1;

    if ( B )
    {
        RC = A / B;    // RC is slope
        T = y1 - x1 * RC;
    }

    if ( clipcode1 & 0x0004 )
    {
        // intersect lowerbound with the line
        if ( B ) temppoint = ( m_clipboxworld.GetMinY() - T ) / RC;
        else temppoint = x1;        // perpendicular line

        // calculate intersection points
        if ( temppoint >= m_clipboxworld.GetMinX() && temppoint <= m_clipboxworld.GetMaxX() )
        {
            intersectionpoint1[nr_of_intersections] = temppoint;
            intersectionpoint2[nr_of_intersections++] = m_clipboxworld.GetMinY();
        }
    }

    if ( clipcode1 & 0x0008 )
    {
        // intersect upperbound with the line
        if ( B ) temppoint = ( m_clipboxworld.GetMaxY() - T ) / RC;
        else temppoint = x1;        // loodrechte lijn

        if ( temppoint >= m_clipboxworld.GetMinX() && temppoint <= m_clipboxworld.GetMaxX() )
        {
            intersectionpoint1[nr_of_intersections] = temppoint;
            intersectionpoint2[nr_of_intersections++] = m_clipboxworld.GetMaxY();
        }
    }

    if ( clipcode1 & 0x0001 )
    {
        // intersect leftbound with the line
        temppoint = RC * m_clipboxworld.GetMinX() + T;
        if ( temppoint > m_clipboxworld.GetMinY() && temppoint < m_clipboxworld.GetMaxY() )
        {
            intersectionpoint2[nr_of_intersections] = temppoint;
            intersectionpoint1[nr_of_intersections++] = m_clipboxworld.GetMinX();
        }
    }

    if ( clipcode1 & 0x0002 )
    {
        // intersect rightbound with the line
        temppoint = RC * m_clipboxworld.GetMaxX() + T;
        if ( temppoint > m_clipboxworld.GetMinY() && temppoint < m_clipboxworld.GetMaxY() )
        {
            intersectionpoint2[nr_of_intersections] = temppoint;
            intersectionpoint1[nr_of_intersections++] = m_clipboxworld.GetMaxX();
        }
    }

    if ( nr_of_intersections != 2 )    return false;

    // set the new values in the parameters (these are references)
    x1 = intersectionpoint1[0];
    y1 = intersectionpoint2[0];
    x2 = intersectionpoint1[1];
    y2 = intersectionpoint2[1];

    return true;
}

void a2dDrawer2D::GetClippingBoxDev( int& x, int& y, int& w, int& h ) const
{
    x = m_clipboxdev.x;
    y = m_clipboxdev.y;
    w = m_clipboxdev.width;
    h = m_clipboxdev.height;
}

void a2dDrawer2D::GetClippingMinMax( double& xmin, double& ymin, double& xmax, double& ymax ) const
{
    xmin = m_clipboxworld.GetMinX();
    ymin = m_clipboxworld.GetMinY();
    xmax = m_clipboxworld.GetMaxX();
    ymax = m_clipboxworld.GetMaxY();
}

void a2dDrawer2D::GetClippingBox( double& x, double& y, double& w, double& h ) const
{
    x = m_clipboxworld.GetMinX();
    y = m_clipboxworld.GetMinY();
    w = m_clipboxworld.GetWidth();
    h = m_clipboxworld.GetHeight();
}

unsigned int a2dDrawer2D::ConvertSplinedPolygon2( unsigned int n )
{
    a2dVertexList h;
    unsigned int i;
    for ( i = 0; i < n; i++ )
    {
        h.push_back( new a2dLineSegment( m_cpointsDouble[i].x, m_cpointsDouble[i].y ) );
    }

    double deviceaber = WorldToDeviceXRel( m_splineaberration );
    if ( deviceaber < 5 )
        deviceaber = THRESHOLD;

    //TODO need an array version for speed
    h.ConvertIntoSplinedPolygon( deviceaber );

    n = h.size();

    m_cpointsDouble.resize( n );

    a2dVertexList::iterator iter = h.begin();
    for ( i = 0; i < n; i++ )
    {
        m_cpointsDouble[i].x = ( *iter )->m_x;
        m_cpointsDouble[i].y = ( *iter )->m_y;
        iter++;
    }
    h.clear();

    return n;
}

unsigned int a2dDrawer2D::ConvertSplinedPolyline2( unsigned int n )
{
    a2dVertexList h;
    unsigned int i;
    for ( i = 0; i < n; i++ )
    {
        h.push_back( new a2dLineSegment( m_cpointsDouble[i].x, m_cpointsDouble[i].y ) );
    }

    double deviceaber = WorldToDeviceXRel( m_splineaberration );
    if ( deviceaber < 5 )
        deviceaber = THRESHOLD;

    //TODO need an array version for speed
    h.ConvertIntoSplinedPolyline( deviceaber );

    n = h.size();
    m_cpointsDouble.resize( n );

    a2dVertexList::iterator iter = h.begin();
    for ( i = 0; i < n; i++ )
    {
        m_cpointsDouble[i].x = ( *iter )->m_x;
        m_cpointsDouble[i].y = ( *iter )->m_y;
        iter = h.erase( iter );
    }
    return n;
}

void a2dDrawer2D::DeviceDrawPolygon( unsigned int n, bool spline , wxPolygonFillMode fillStyle )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

void a2dDrawer2D::DeviceDrawLines( unsigned int n, bool spline )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

void a2dDrawer2D::DeviceDrawLine( double x1, double y1, double x2, double y2 )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

void a2dDrawer2D::DeviceDrawHorizontalLine( int x1, int y1, int x2, bool WXUNUSED( use_pen ) )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

void a2dDrawer2D::DeviceDrawVerticalLine( int x1, int y1, int y2, bool WXUNUSED( use_pen ) )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

void a2dDrawer2D::DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
    wxFAIL_MSG( wxT( "if used, must be implemented in derived class" ) );
}

IMPLEMENT_CLASS( a2dDrawer2D, wxObject )
IMPLEMENT_DYNAMIC_CLASS( a2dBlindDrawer2D, a2dDrawer2D )

/*******************************************************************
a2dBlindDrawer2D
********************************************************************/

a2dBlindDrawer2D::a2dBlindDrawer2D( const wxSize& size ): a2dDrawer2D( size )
{}

a2dBlindDrawer2D::a2dBlindDrawer2D( int w, int h ): a2dDrawer2D( w, h )
{}

a2dBlindDrawer2D::~a2dBlindDrawer2D()
{}

a2dBlindDrawer2D::a2dBlindDrawer2D( const a2dBlindDrawer2D& other )
    : a2dDrawer2D( other )
{}

a2dBlindDrawer2D::a2dBlindDrawer2D( const a2dDrawer2D& other )
    : a2dDrawer2D( other )
{}

void a2dBlindDrawer2D::BeginDraw()
{
    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dBlindDrawer2D, unbalanced BeginDraw EndDraw" ) );
    m_beginDraw_endDraw++;
}

void a2dBlindDrawer2D::EndDraw()
{
    m_beginDraw_endDraw--;
    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dBlindDrawer2D, unbalanced BeginDraw EndDraw" ) );
}

void a2dBlindDrawer2D::SetBufferSize( int w, int h )
{
    m_width = w;
    m_height  = h;
}

wxBitmap a2dBlindDrawer2D::GetSubBitmap( wxRect WXUNUSED( rect ) ) const
{
    wxFAIL_MSG( wxT( "a2dBlindDrawer2D has no buffer" ) );
    return wxBitmap( m_width, m_height );
}

void a2dBlindDrawer2D::SetClippingRegion( a2dVertexList* WXUNUSED( points ), bool WXUNUSED( spline ), wxPolygonFillMode WXUNUSED( fillStyle ) )
{}

void a2dBlindDrawer2D::ExtendAndPushClippingRegion( a2dVertexList* WXUNUSED( points ), bool WXUNUSED( spline ), wxPolygonFillMode WXUNUSED( fillStyle ), a2dBooleanClip WXUNUSED( clipoperation ) )
{}

void a2dBlindDrawer2D::ExtendClippingRegion( a2dVertexList* WXUNUSED( points ), bool WXUNUSED( spline ), wxPolygonFillMode WXUNUSED( fillStyle ), a2dBooleanClip WXUNUSED( clipoperation ) )
{}

void a2dBlindDrawer2D::PopClippingRegion()
{}

void a2dBlindDrawer2D::SetClippingRegionDev( wxCoord WXUNUSED( minx ), wxCoord WXUNUSED( miny ), wxCoord WXUNUSED( width ), wxCoord WXUNUSED( height ) )
{}

void a2dBlindDrawer2D::SetClippingRegion( double WXUNUSED( minx ), double WXUNUSED( miny ), double WXUNUSED( maxx ), double WXUNUSED( maxy ) )
{}

void a2dBlindDrawer2D::DestroyClippingRegion()
{}

void a2dBlindDrawer2D::ResetStyle()
{
    a2dDrawer2D::ResetStyle();
}

void a2dBlindDrawer2D::DoSetActiveStroke()
{}

void a2dBlindDrawer2D::DoSetActiveFill()
{}

void a2dBlindDrawer2D::DoSetDrawStyle( a2dDrawStyle WXUNUSED( drawstyle ) )
{}

void a2dBlindDrawer2D::DrawImage( const wxImage& WXUNUSED( imagein ), double WXUNUSED( x ), double WXUNUSED( y ), double WXUNUSED( width ), double WXUNUSED( height ), wxUint8 WXUNUSED( Opacity ) )
{}

void a2dBlindDrawer2D::DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity )
{}

void a2dBlindDrawer2D::DrawVpath( const a2dVpath* WXUNUSED( path ) )
{}

void a2dBlindDrawer2D::DrawPoint( double xc, double yc )
{}

void a2dBlindDrawer2D::DrawPolygon( a2dVertexArray* points, bool spline, wxPolygonFillMode fillStyle )
{
}

void a2dBlindDrawer2D::DrawPolygon( const a2dVertexList* list, bool spline, wxPolygonFillMode fillStyle )
{
}

void a2dBlindDrawer2D::DrawLines( a2dVertexArray* points, bool spline )
{
}

void a2dBlindDrawer2D::DrawLines( const a2dVertexList* list, bool spline )
{
}

void a2dBlindDrawer2D::DrawArc( double x1, double y1, double x2, double y2, double xc, double yc, bool chord )
{
}

void a2dBlindDrawer2D::DrawEllipticArc( double xc, double yc, double width, double height, double sa, double ea, bool chord )
{
}

void a2dBlindDrawer2D::DrawCenterRoundedRectangle( double xc, double yc, double width, double height, double radius, bool pixelsize )
{
}

void a2dBlindDrawer2D::DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize )
{
}

void a2dBlindDrawer2D::DrawCircle( double xc, double yc, double radius )
{
}

void a2dBlindDrawer2D::DrawEllipse( double xc, double yc, double width, double height )
{
}

void a2dBlindDrawer2D::DrawLine( double x1, double y1, double x2, double y2 )
{
}

void a2dBlindDrawer2D::DeviceDrawAnnotation( const wxString& WXUNUSED( text ), wxCoord WXUNUSED( x ), wxCoord WXUNUSED( y ), const wxFont& WXUNUSED( font ) )
{}

void a2dBlindDrawer2D::BlitBuffer( wxRect WXUNUSED( rect ), const wxPoint& WXUNUSED( bufferpos ) )
{
    wxFAIL_MSG( wxT( "not useful here" ) );
}

void a2dBlindDrawer2D::BlitBuffer( wxDC* WXUNUSED( dc ), wxRect WXUNUSED( rect ), const wxPoint& WXUNUSED( bufferpos ) )
{
    wxFAIL_MSG( wxT( "not useful here" ) );
}




#if wxART2D_USE_GRAPHICS_CONTEXT

IMPLEMENT_CLASS( a2dGcBaseDrawer, a2dDrawer2D )

/*******************************************************************
a2dGcBaseDrawer
********************************************************************/

a2dGcBaseDrawer::a2dGcBaseDrawer( int width, int height, wxGraphicsRenderer* render, wxGraphicsContext* context ): a2dDrawer2D( width, height )
{
    m_render = render;
    m_context = context;
}

a2dGcBaseDrawer::a2dGcBaseDrawer( const a2dGcBaseDrawer& other )
    : a2dDrawer2D( other )
{
    m_render = other.m_render;
    m_context = other.m_context;
}

a2dGcBaseDrawer::a2dGcBaseDrawer( const a2dDrawer2D& other )
    : a2dDrawer2D( other )
{
    m_render = NULL;
    m_context = NULL;
}

a2dGcBaseDrawer::~a2dGcBaseDrawer()
{
}

void a2dGcBaseDrawer::SetTransform( const a2dAffineMatrix& userToWorld )
{
    a2dDrawer2D::SetTransform( userToWorld );

    wxGraphicsMatrix m = m_context->CreateMatrix(
                             m_usertodevice( 0, 0 ), m_usertodevice( 0, 1 ),
                             m_usertodevice( 1, 0 ), m_usertodevice( 1, 1 ),
                             m_usertodevice( 2, 0 ), m_usertodevice( 2, 1 ) );
    m_context->SetTransform( m );
}

void a2dGcBaseDrawer::PushTransform()
{
    a2dDrawer2D::PushTransform();

    wxGraphicsMatrix m = m_context->CreateMatrix();
    m.Set(
        m_usertodevice( 0, 0 ), m_usertodevice( 0, 1 ),
        m_usertodevice( 1, 0 ), m_usertodevice( 1, 1 ),
        m_usertodevice( 2, 0 ), m_usertodevice( 2, 1 ) );
    m_context->SetTransform( m );
}

void a2dGcBaseDrawer::PushTransform( const a2dAffineMatrix& affine )
{
    a2dDrawer2D::PushTransform( affine );

    wxGraphicsMatrix m = m_context->CreateMatrix();
    m.Set(
        m_usertodevice( 0, 0 ), m_usertodevice( 0, 1 ),
        m_usertodevice( 1, 0 ), m_usertodevice( 1, 1 ),
        m_usertodevice( 2, 0 ), m_usertodevice( 2, 1 ) );
    m_context->SetTransform( m );
}

void a2dGcBaseDrawer::PushIdentityTransform()
{
    a2dDrawer2D::PushIdentityTransform(  );

    wxGraphicsMatrix m = m_context->CreateMatrix();
    m.Set(
        m_usertodevice( 0, 0 ), m_usertodevice( 0, 1 ),
        m_usertodevice( 1, 0 ), m_usertodevice( 1, 1 ),
        m_usertodevice( 2, 0 ), m_usertodevice( 2, 1 ) );
    m_context->SetTransform( m );
}

void a2dGcBaseDrawer::PopTransform( void )
{
    a2dDrawer2D::PopTransform();

    wxGraphicsMatrix m = m_context->CreateMatrix();
    m.Set(
        m_usertodevice( 0, 0 ), m_usertodevice( 0, 1 ),
        m_usertodevice( 1, 0 ), m_usertodevice( 1, 1 ),
        m_usertodevice( 2, 0 ), m_usertodevice( 2, 1 ) );
    m_context->SetTransform( m );
}

void a2dGcBaseDrawer::BeginDraw()
{
    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
        SetDrawerFill( m_currentfill );
        SetDrawerStroke( m_currentstroke );
        DestroyClippingRegion();
        SetDrawStyle( m_drawstyle );

    }

    m_beginDraw_endDraw++;
}

void a2dGcBaseDrawer::EndDraw()
{
    m_beginDraw_endDraw--;

    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
    }
}

void a2dGcBaseDrawer::BlitBuffer( wxRect rect, const wxPoint& bufferpos )
{
    BeginDraw();
    if ( m_display )
    {
        wxClientDC deviceDC( m_display );
        m_display->PrepareDC( deviceDC );
        BlitBuffer( &deviceDC, rect, bufferpos );
    }
    EndDraw();
}

void a2dGcBaseDrawer::SetClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle )
{
    unsigned int n = points->size();

    m_cpointsDouble.resize( n );

    unsigned int i = 0;
    double x, y;
    forEachIn( a2dVertexList, points )
    {
        a2dPoint2D point = ( *iter )->GetPoint();
        //transform to device
        GetUserToDeviceTransform().TransformPoint( point.m_x, point.m_y, x, y );
        m_cpointsDouble[i].x = x;
        m_cpointsDouble[i].y = y;
        i++;
    }

    if ( spline )
        n = ConvertSplinedPolygon2( n );

    wxPoint* intpoints = _convertToIntPointCache( n, &m_cpointsDouble[0] );
    m_clip = wxRegion( n, intpoints, fillStyle );

    m_clipboxdev = m_clip.GetBox();
    m_clipboxworld = ToWorld( m_clipboxdev );
}

void a2dGcBaseDrawer::ExtendAndPushClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle, a2dBooleanClip clipoperation )
{
    wxRegion* push = new wxRegion( m_clip );

    m_clipregionlist.Insert( push );

    ExtendClippingRegion( points, spline, fillStyle, clipoperation );
}

void a2dGcBaseDrawer::ExtendClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode WXUNUSED( fillStyle ), a2dBooleanClip clipoperation )
{
    wxRegion totaladd;
    bool first = false;

    a2dBoundingBox devbox;
    bool smallPoly = false;
    unsigned int segments = ToDeviceLines( points, devbox, smallPoly );

    if ( segments == 0 )
        return;

    if ( spline )
        segments = ConvertSplinedPolygon2( segments );

    wxPoint* int_cpts = _convertToIntPointCache( segments, &m_cpointsDouble[0] );
    wxRegion add = wxRegion( segments, int_cpts, wxWINDING_RULE );
    if ( !first )
    {
        totaladd = add;
        first = true;
    }
    else
        totaladd.Union( add );

    if ( !m_clip.Empty() )
    {
        bool result;
        switch ( clipoperation )
        {
            case a2dCLIP_AND:
                result = m_clip.Intersect( totaladd );
                break ;

            case a2dCLIP_OR:
                result = m_clip.Union( totaladd );
                break ;

            case a2dCLIP_XOR:
                result = m_clip.Xor( totaladd );
                break ;

            case a2dCLIP_DIFF:
                result = m_clip.Subtract( totaladd );
                break ;

            case a2dCLIP_COPY:
            default:
                m_clip = totaladd;
                result = true;
                break ;
        }
        if ( result )
        {
            m_context->ResetClip();
        }
    }
    else
    {
        m_clip = totaladd;
        m_context->ResetClip();
    }

    m_clipboxdev = m_clip.GetBox();
    m_clipboxworld = ToWorld( m_clipboxdev );
    m_context->Clip( m_clipboxworld.GetMinX(), m_clipboxworld.GetMinY(),
                     m_clipboxworld.GetWidth(), m_clipboxworld.GetHeight() );
}

void a2dGcBaseDrawer::PopClippingRegion()
{
    if ( !m_clipregionlist.GetCount() )
        return;

    m_clip = *m_clipregionlist.GetFirst()->GetData();
    delete m_clipregionlist.GetFirst()->GetData();
    m_clipregionlist.DeleteNode( m_clipregionlist.GetFirst() );

    m_clipboxdev = m_clip.GetBox();
    m_clipboxworld = ToWorld( m_clipboxdev );
    m_context->Clip( m_clipboxworld.GetMinX(), m_clipboxworld.GetMinY(),
                     m_clipboxworld.GetWidth(), m_clipboxworld.GetHeight() );
}

void a2dGcBaseDrawer::SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height )
{
    m_clip = wxRegion( minx, miny, width, height );
    m_clipboxdev = wxRect( minx, miny, width, height );
    m_clipboxworld = ToWorld( m_clipboxdev );

    //we don't know which part of the transform set in m_context is to go from world to device.
    //remove all first.
    PushIdentityTransform();
    m_context->ResetClip();
    m_context->Clip( m_clipboxdev.x, m_clipboxdev.y,
                     m_clipboxdev.GetWidth(), m_clipboxdev.GetHeight() );

    PopTransform();
}

void a2dGcBaseDrawer::SetClippingRegion( double minx, double miny, double maxx, double maxy )
{
    int iminx = WorldToDeviceX( minx );
    int iminy = WorldToDeviceY( miny );
    int imaxx = WorldToDeviceX( maxx );
    int imaxy = WorldToDeviceY( maxy );
    if ( m_yaxis )
    {
        m_clip = wxRegion( iminx, imaxy, imaxx - iminx, iminy - imaxy );
        m_clipboxdev = wxRect( iminx, imaxy, imaxx - iminx, iminy - imaxy );
    }
    else
    {
        m_clip = wxRegion( iminx, iminy, imaxx - iminx, imaxy - iminy );
        m_clipboxdev = wxRect( iminx, iminy, imaxx - iminx, imaxy - iminy );
    }

    m_context->ResetClip();
    //we don't know which part of the transform set in m_context is to go from world to device.
    //remove all first.
    PushIdentityTransform();
    m_context->Clip( m_clipboxdev.x, m_clipboxdev.y,
                     m_clipboxdev.GetWidth(), m_clipboxdev.GetHeight() );

    PopTransform();
}

void a2dGcBaseDrawer::DestroyClippingRegion()
{
    m_clip.Clear();
    m_clipboxdev = wxRect( 0, 0, m_width, m_height );
    m_clipboxworld = ToWorld( m_clipboxdev );

    m_context->ResetClip();
}

void a2dGcBaseDrawer::ResetStyle()
{
    a2dDrawer2D::ResetStyle();
    SetDrawerFill( *a2dBLACK_FILL ); //set to a sure state
    SetDrawerStroke( *a2dBLACK_STROKE );
    SetDrawStyle( m_drawstyle );
}

void a2dGcBaseDrawer::DoSetActiveFont( const a2dFont& font )
{
    // Get font size in device units
    unsigned int fontsize;
    fontsize = m_currentfont.GetSize();
    if ( fontsize < 1 )
        fontsize = 1;
    switch ( m_currentfont.GetType() )
    {
        case a2dFONT_WXDC:
        {
            m_currentfont.GetFont().SetPointSize( fontsize );
            wxGraphicsFont f = m_render->CreateFont( m_currentfont.GetFont() );
            m_context->SetFont( f );
            break;
        }
        default:
            wxFont wxfont = *wxNORMAL_FONT;
            wxfont.SetPointSize( fontsize );
            wxGraphicsFont f = m_render->CreateFont( wxfont );
            m_context->SetFont( f );
    }
}

void a2dGcBaseDrawer::DoSetActiveStroke()
{
    wxGraphicsPen graphpen = ConvertActiveToGraphicsPen( m_context );
    m_context->SetPen( graphpen );
    //wxPen dcpen = ConvertActiveToPen();
    //m_context->SetPen( dcpen );
}

void a2dGcBaseDrawer::DoSetActiveFill()
{
    wxGraphicsBrush graphbrush = ConvertActiveToGraphicsBrush( m_context );
    m_context->SetBrush( graphbrush );
    //wxBrush dcbrush = ConvertActiveToBrush();
    //m_context->SetBrush( dcbrush );
}

void a2dGcBaseDrawer::DoSetDrawStyle( a2dDrawStyle drawstyle )
{
    if ( m_drawstyle == a2dFIX_STYLE && m_drawstyle == a2dFIX_STYLE_INVERT )
        return;

    m_drawstyle = drawstyle;

    switch( drawstyle )
    {
        case a2dWIREFRAME_INVERT:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
#if wxCHECK_VERSION(2,9,0)
#else
            m_context->SetLogicalFunction( wxINVERT );
#endif
            break;

        case a2dWIREFRAME:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
#if wxCHECK_VERSION(2,9,0)
#if wxCHECK_VERSION(2,9,0)
#else
            m_context->SetLogicalFunction( wxCOPY );
#endif
#else
#endif
            break;

        case a2dWIREFRAME_ZERO_WIDTH:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
#if wxCHECK_VERSION(2,9,0)
#else
            m_context->SetLogicalFunction( wxCOPY );
#endif
            break;

        case a2dWIREFRAME_INVERT_ZERO_WIDTH:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
#if wxCHECK_VERSION(2,9,0)
#else
            m_context->SetLogicalFunction( wxINVERT );
#endif
            break;

        case a2dFILLED:
            SetActiveStroke( m_currentstroke );
            SetActiveFill( m_currentfill );
#if wxCHECK_VERSION(2,9,0)
#else
            m_context->SetLogicalFunction( wxCOPY );
#endif
            break;

        case a2dFIX_STYLE:
            //preserve this
            m_fixStrokeRestore = m_currentstroke;
            m_fixFillRestore = m_currentfill;
            m_fixDrawstyle = m_drawstyle;
            SetActiveStroke( m_currentstroke );
            SetActiveFill( m_currentfill );

            // don't adjust style
#if wxCHECK_VERSION(2,9,0)
#else
            m_context->SetLogicalFunction( wxCOPY );
#endif
            break;

        case a2dFIX_STYLE_INVERT:
            // don't adjust style
#if wxCHECK_VERSION(2,9,0)
#else
            m_context->SetLogicalFunction( wxINVERT );
#endif
            break;

        default:
            wxASSERT( 0 );
    }

    //a2dContext* context = (a2dContext*) m_context;
    //context->SetDrawStyle( m_drawstyle );
}


void a2dGcBaseDrawer::DrawImage( const wxImage& imagein, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    a2dAffineMatrix affine;
    if ( GetYaxis() )
    {
        affine.Translate( 0.0, -( y ) );
        affine.Mirror( true, false );
        affine.Translate( 0.0, y );
    }
    PushTransform( affine );

    wxGraphicsBitmap bitmap = m_render->CreateBitmap( imagein );
#if wxCHECK_VERSION(2,9,0)
    m_context->DrawBitmap( bitmap, x - width / 2.0, y - height / 2.0, width, height );
#else
    m_context->DrawGraphicsBitmap( bitmap, x - width / 2.0, y - height / 2.0, width, height );
#endif

    PopTransform();
}

void a2dGcBaseDrawer::DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    wxImage wximage = image.GetImage();
    wxGraphicsBitmap bitmap = m_render->CreateBitmap( wximage );
#if wxCHECK_VERSION(2,9,0)
    m_context->DrawBitmap( bitmap, x - width / 2.0, y - height / 2.0, width, height );
#else
    m_context->DrawGraphicsBitmap( bitmap, x - width / 2.0, y - height / 2.0, width, height );
#endif
}

void a2dGcBaseDrawer::DrawLines( const a2dVertexList* list, bool spline )
{
    if ( m_disableDrawing )
        return;

    wxGraphicsPath path = m_context->CreatePath();

    a2dVertexList::const_iterator iter = list->begin();
    int i = 0;
    while ( iter != list->end() )
    {
        a2dLineSegment* seg = ( *iter );
        if ( i == 0 )
            path.MoveToPoint( seg->m_x, seg->m_y );
        else
            path.AddLineToPoint( seg->m_x, seg->m_y );
        i++;
        iter++;
    }

    m_context->StrokePath( path );
}

void a2dGcBaseDrawer::DrawLines( a2dVertexArray* points, bool spline )
{
    if ( m_disableDrawing )
        return;

    wxGraphicsPath path = m_context->CreatePath();

    int i;
    for ( i = 0; i < points->size(); i++ )
    {
        const a2dLineSegmentPtr seg = points->Item( i );
        if ( i == 0 )
            path.MoveToPoint( seg->m_x, seg->m_y );
        else
            path.AddLineToPoint( seg->m_x, seg->m_y );
    }

    m_context->StrokePath( path );
}

void a2dGcBaseDrawer::DrawPolygon( a2dVertexArray* points, bool spline, wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    wxGraphicsPath path = m_context->CreatePath();

    int i;
    for ( i = 0; i < points->size(); i++ )
    {
        const a2dLineSegmentPtr seg = points->Item( i );
        if ( i == 0 )
            path.MoveToPoint( seg->m_x, seg->m_y );
        else
            path.AddLineToPoint( seg->m_x, seg->m_y );
    }
    path.CloseSubpath();

    m_context->DrawPath( path , fillStyle );
}

void a2dGcBaseDrawer::DrawPolygon( const a2dVertexList* list, bool spline, wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    wxGraphicsPath path = m_context->CreatePath();

    a2dVertexList::const_iterator iter = list->begin();
    int i = 0;
    while ( iter != list->end() )
    {
        a2dLineSegment* seg = ( *iter );
        if ( i == 0 )
            path.MoveToPoint( seg->m_x, seg->m_y );
        else
            path.AddLineToPoint( seg->m_x, seg->m_y );
        iter++;
        i++;
    }
    path.CloseSubpath();

    m_context->DrawPath( path , fillStyle );
}

void a2dGcBaseDrawer::DrawCircle( double xc, double yc, double radius )
{
    if ( m_disableDrawing )
        return;

    m_context->DrawEllipse( xc - radius, yc - radius, 2.0 * radius, 2.0 * radius );
}

void a2dGcBaseDrawer::DrawEllipse( double xc, double yc, double width, double height )
{
    if ( m_disableDrawing )
        return;

    m_context->DrawEllipse( xc - width / 2.0, yc - height / 2.0, width, height );
}

void a2dGcBaseDrawer::DrawLine( double x1, double y1, double x2, double y2 )
{
    if ( m_disableDrawing )
        return;

    m_context->StrokeLine( x1, y1, x2, y2 );
}


void a2dGcBaseDrawer::DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize )
{
    if ( m_disableDrawing )
        return;

    if ( height < 0 )
    {
        y += height;
        height = -height;
    }
    if ( width < 0 )
    {
        x += width;
        width = -width;
    }
    m_context->DrawRoundedRectangle( x, y, width, height, radius );
}

void a2dGcBaseDrawer::DrawPoint( double xc, double yc )
{
    if ( m_disableDrawing )
        return;

    double xt, yt;
    GetUserToDeviceTransform().TransformPoint( xc, yc, xt, yt );
    //DeviceDrawPixel( xt, yt, m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
}

void a2dGcBaseDrawer::DrawText( const wxString& text, double x, double y, int alignment, bool Background )
{
    if ( m_disableDrawing )
        return;

    // Get the bounding box, including alignment displacement.
    a2dBoundingBox bbox = m_currentfont.GetTextExtent( text, alignment );
    bbox.Translate( x, y );

    // Get unaligned coordinates for DrawTextXXX functions. By default these functions
    // use the lowerleft corner of the boundingbox, hence GetMinX() and GetMinY().
    x = bbox.GetMinX();
    y = bbox.GetMinY();

    // Get font size in device units, also takes stretching into account
    double dx, dy, size;
    dx = m_usertodevice.GetValue( 1, 0 );
    dy = m_usertodevice.GetValue( 1, 1 );
    size = m_currentfont.GetSize() * sqrt( dx * dx + dy * dy );

    // Transform user bbox to world bbox.
    a2dBoundingBox worldbbox( bbox );
    worldbbox.MapBbox( m_usertoworld );
    // If bbox not outside clipping area, draw the text
    if ( m_clipboxworld.Intersect( worldbbox ) != _OUT )
    {
        // Draw background fill
        if ( Background && !GetDrawerFill().IsNoFill() && !GetDrawerFill().GetStyle() == a2dFILL_TRANSPARENT )
        {
            a2dStroke oldstroke = m_activestroke;
            SetDrawerStroke( *a2dTRANSPARENT_STROKE );
            DrawRoundedRectangle( bbox.GetMinX(), bbox.GetMinY(),
                                  bbox.GetWidth(), bbox.GetHeight(), 0 );
            SetDrawerStroke( oldstroke );
        }

        const double h = m_currentfont.GetLineHeight();
        a2dAffineMatrix affine;
        if ( GetYaxis() )
        {
            affine.Translate( 0.0, -( y + h ) );
            affine.Mirror( true, false );
            affine.Translate( 0.0, y );
        }
        PushTransform( affine );

        //a2dFill oldfill = m_activefill;
        //SetDrawerFill( a2dFill( m_activestroke.GetColour() ) );

        // If text > threshold, draw text
        if ( size > m_drawingthreshold )
            m_context->DrawText( text, x, y );
        // If text < threshold, draw a simple version.
        else
            DrawTextUnknown( text, x, y, true );
        //SetDrawerFill( oldfill );
        PopTransform();
    }

}


IMPLEMENT_DYNAMIC_CLASS( a2dNativeGcDrawer, a2dGcBaseDrawer )

/*******************************************************************
a2dNativeGcDrawer
********************************************************************/

void a2dNativeGcDrawer::InitContext()
{
    m_clip.Clear();
    m_buffer = wxBitmap( m_width, m_height );
    m_memdc.SelectObject( m_buffer );
    if ( !m_render )
        m_render = wxGraphicsRenderer::GetDefaultRenderer();
    if ( !m_context )
        m_context = m_render->CreateContext( m_memdc );
}

a2dNativeGcDrawer::a2dNativeGcDrawer( const wxSize& size ): a2dGcBaseDrawer( size.GetWidth(), size.GetHeight() )
{
    InitContext();
}

a2dNativeGcDrawer::a2dNativeGcDrawer( int width, int height, wxGraphicsRenderer* render, wxGraphicsContext* context )
    : a2dGcBaseDrawer( width, height )
{
    m_render = render;
    m_context = context;
    InitContext();
}

a2dNativeGcDrawer::a2dNativeGcDrawer( const a2dNativeGcDrawer& other )
    : a2dGcBaseDrawer( other )
{
    m_render = NULL;
    m_context = NULL;
    InitContext();
}

a2dNativeGcDrawer::a2dNativeGcDrawer( const a2dDrawer2D& other )
    : a2dGcBaseDrawer( other )
{
    InitContext();
}

a2dNativeGcDrawer::~a2dNativeGcDrawer()
{
    m_memdc.SelectObject( wxNullBitmap );
}

wxBitmap a2dNativeGcDrawer::GetBuffer() const
{
    return m_buffer;
}

void a2dNativeGcDrawer::SetBufferSize( int w, int h )
{
    m_width = w;
    m_height  = h;
    wxBitmap helpbuf  = wxBitmap( w, h );

    //make sure to preserve what was in the old buffer bitmap,
    //to prevent the need for total redraws
    wxMemoryDC mdch;
    mdch.SelectObject( helpbuf );
    mdch.Blit( 0, 0, w, h, &m_memdc, 0, 0 );
    mdch.SelectObject( wxNullBitmap );

    m_memdc.SelectObject( wxNullBitmap );
    m_buffer = helpbuf;
    m_memdc.SelectObject( m_buffer );
    delete m_context;
    m_context = m_render->CreateContext( m_memdc );
}

wxBitmap a2dNativeGcDrawer::GetSubBitmap( wxRect rect ) const
{
    wxBitmap ret( rect.width, rect.height, m_buffer.GetDepth() );
    wxASSERT_MSG( ret.Ok(), wxT( "GetSubImage error" ) );

    //do NOT use getsubbitmap, renderDc is already set for the buffer therefor will not work properly
    wxMemoryDC dcb;
    dcb.SelectObject( ret );
    dcb.Blit( 0, 0, rect.width, rect.height, const_cast< wxMemoryDC* >( &m_memdc ), rect.x, rect.y, wxCOPY, false );
    dcb.SelectObject( wxNullBitmap );

    return ret;
}

void a2dNativeGcDrawer::CopyIntoBuffer( const wxBitmap& bitm )
{
    m_memdc.SelectObject( wxNullBitmap );
    m_buffer = bitm;
    m_memdc.SelectObject( m_buffer );
}

void a2dNativeGcDrawer::BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos )
{
    // clip to buffer
    if ( rect.x < 0 )
    {
        rect.width += rect.x;
        rect.x = 0;
    }
    if ( rect.width <= 0 ) return;

    if ( rect.y < 0 )
    {
        rect.height += rect.y;
        rect.y = 0;
    }
    if ( rect.height <= 0 ) return;

    if ( rect.x + rect.width > m_width )
    {
        rect.width = m_width - rect.x;
    }
    if ( rect.width <= 0 ) return;

    if ( rect.y + rect.height > m_height )
    {
        rect.height = m_height - rect.y;
    }
    if ( rect.height <= 0 ) return;

    dc->Blit( rect.x - bufferpos.x, rect.y - bufferpos.y, rect.width, rect.height, &m_memdc, rect.x, rect.y, wxCOPY, false );
}

void a2dNativeGcDrawer::ShiftBuffer( int dxy, bool yshift )
{
    int bw = GetBuffer().GetWidth();
    int bh = GetBuffer().GetHeight();
    if ( yshift )
    {
        if ( dxy > 0 && dxy < bh )
        {
            wxRect rect( 0, 0, bw, bh - dxy );
            wxBitmap sub_bitmap( m_buffer.GetSubBitmap( rect ) );
            wxMemoryDC dcm;
            dcm.SelectObject( m_buffer );
            dcm.DrawBitmap( sub_bitmap, 0, dxy, true );
            dcm.SelectObject( wxNullBitmap );
        }
        else  if ( dxy < 0 && dxy > -bh )
        {
            wxRect rect( 0, -dxy, bw, bh + dxy );
            wxBitmap sub_bitmap( m_buffer.GetSubBitmap( rect ) );
            wxMemoryDC dcm;
            dcm.SelectObject( m_buffer );
            dcm.DrawBitmap( sub_bitmap, 0, 0, true );
            dcm.SelectObject( wxNullBitmap );
        }
        else
            wxFAIL_MSG( wxT( "you can only shift within height of buffer" ) );

    }
    else
    {
        if ( dxy > 0 && dxy < bw )
        {
            wxRect rect( 0, 0, bw - dxy, bh );
            wxBitmap sub_bitmap( m_buffer.GetSubBitmap( rect ) );
            wxMemoryDC dcm;
            dcm.SelectObject( m_buffer );
            dcm.DrawBitmap( sub_bitmap, dxy, 0, true );
            dcm.SelectObject( wxNullBitmap );
        }
        else if ( dxy < 0 && dxy > -bw )
        {
            wxRect rect( -dxy, 0, bw + dxy, bh );
            wxBitmap sub_bitmap( m_buffer.GetSubBitmap( rect ) );
            wxMemoryDC dcm;
            dcm.SelectObject( m_buffer );
            dcm.DrawBitmap( sub_bitmap, 0, 0, true );
            dcm.SelectObject( wxNullBitmap );
        }
        else
            wxFAIL_MSG( wxT( "you can only shift within width of buffer" ) );
    }
}

void a2dNativeGcDrawer::DrawPoint( double xc, double yc )
{
    if ( m_disableDrawing )
        return;

    double xt, yt;
    GetUserToDeviceTransform().TransformPoint( xc, yc, xt, yt );
    //DeviceDrawPixel( xt, yt, m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
}

#endif  // wxART2D_USE_GRAPHICS_CONTEXT


