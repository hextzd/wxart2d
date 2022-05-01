/*! \file gdiplusdrawer/src/gdiplusdrawer.cpp
    \brief a2dGDIPlusDrawer - Drawer using the gdiplus library
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: gdiplusdrawer.cpp,v 1.11 2009/10/01 19:22:36 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <gdiplus.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "wx/msw/dc.h"
#include "wx/artbase/stylebase.h"
#include "wx/gdiplusdrawer/gdiplusdrawer.h"


#if wxART2D_USE_GDIPLUSDRAWER

WX_DEFINE_LIST( RegionList );

a2dGDIPlusDrawer::a2dGDIPlusDrawer( int width, int height ) : a2dDrawer2D( width, height )
{
    Init();
}

a2dGDIPlusDrawer::a2dGDIPlusDrawer( const wxSize& size ) : a2dDrawer2D( size )
{
    Init();
}

void a2dGDIPlusDrawer::Init()
{
    GdiplusStartupInput input;
    GdiplusStartupOutput output;
    GdiplusStartup( &m_gdiplus_token, &input, &output );

    m_buffer = wxBitmap( m_width, m_height );
    m_dc = new wxMemoryDC( );
    m_dc->SelectObject( m_buffer );

#if wxCHECK_VERSION(2, 9, 0)
    const wxMSWDCImpl* impl;
    impl = wxDynamicCast ( m_dc->GetImpl(), wxMSWDCImpl );
    m_context = new Graphics( ( HDC ) impl->GetHDC( ) );
#else
    m_context = new Graphics( ( HDC ) m_dc->GetHDC( ) );
#endif
    m_context->SetSmoothingMode( SmoothingModeHighQuality );
    m_context->SetSmoothingMode( SmoothingModeAntiAlias );

    m_current_pen = new Pen( Color( 0, 0, 0 ), 1 );
    m_current_brush = new SolidBrush( Color( 0, 0, 0 ) );
    m_matrix = Matrix().Clone();
    m_penImage = NULL;
    m_penBrush = NULL;
    m_brushImage = NULL;
    m_brushPath = NULL;

    m_externalDc = false;
    m_deviceDC = 0;
    m_clip.Clear();
    m_OpacityFactor = 255;
}

a2dGDIPlusDrawer::a2dGDIPlusDrawer( const a2dGDIPlusDrawer& other ) : a2dDrawer2D( other )
{
    m_externalDc = false;
    m_deviceDC =  0;
    m_clip.Clear();

    m_buffer = other.m_buffer;
}

a2dGDIPlusDrawer::~a2dGDIPlusDrawer( )
{
    if ( m_matrix )
        delete m_matrix;
    delete m_context;
    delete m_current_pen;
    delete m_current_brush;
    delete m_penImage;
    delete m_penBrush;
    delete m_brushImage;
    delete m_brushPath;
    delete m_dc;
    GdiplusShutdown( m_gdiplus_token );
}

Matrix* a2dGDIPlusDrawer::_get_gdiplus_user_to_device_transform()
{
    if ( m_matrix )
        delete m_matrix;
    const a2dAffineMatrix& rmtx = GetUserToDeviceTransform();
    Matrix mat(
        ( REAL )rmtx( 0, 0 ), ( REAL )rmtx( 0, 1 ),
        ( REAL )rmtx( 1, 0 ), ( REAL )rmtx( 1, 1 ),
        ( REAL )rmtx( 2, 0 ), ( REAL )rmtx( 2, 1 )   );

    m_matrix = mat.Clone();
    return m_matrix;
}

void a2dGDIPlusDrawer::SetBufferSize( int w, int h )
{
    m_width = w;
    m_height = h;

    delete m_context;

    wxBitmap helpbuf  = wxBitmap( w, h );
    m_dc->SelectObject( wxNullBitmap );

    //make sure to preserve what was in the old buffer bitmap,
    //to prevent the need for total redraws
    wxMemoryDC mdc;

    m_dc->SelectObject( m_buffer );
    mdc.SelectObject( helpbuf );
    m_dc->Blit( 0, 0, w, h, &mdc, 0, 0 );
    mdc.SelectObject( wxNullBitmap );
    m_dc->SelectObject( wxNullBitmap );

    m_buffer = helpbuf;
    m_dc->SelectObject( m_buffer );
#if wxCHECK_VERSION(2, 9, 0)
    const wxMSWDCImpl* m_impl;
    m_impl = wxDynamicCast ( m_dc->GetImpl(), wxMSWDCImpl );
    m_context = new Graphics( ( HDC ) m_impl->GetHDC( ) );
#else
    m_context = new Graphics( ( HDC ) m_dc->GetHDC( ) );
#endif
    m_current_pen = new Pen( Color( 0, 0, 0 ), 1 );
    m_current_brush = new SolidBrush( Color( 0, 0, 0 ) );
}

wxBitmap a2dGDIPlusDrawer::GetSubBitmap( wxRect rect ) const
{
    wxBitmap ret( rect.width, rect.height, m_buffer.GetDepth() );
    wxASSERT_MSG( ret.Ok(), wxT( "GetSubImage error" ) );

    wxMemoryDC dcm;
    dcm.SelectObject( const_cast<wxBitmap&>( m_buffer ) );

    //do NOT use getsubbitmap, renderDc is already set for the buffer therefore will not work properly
    wxMemoryDC dcb;
    dcb.SelectObject( ret );
    dcb.Blit( 0, 0, rect.width, rect.height, const_cast<wxMemoryDC*>( &dcm ), rect.x, rect.y, wxCOPY, false );
    dcb.SelectObject( wxNullBitmap );

    dcm.SelectObject( wxNullBitmap );
    return ret;
}

void a2dGDIPlusDrawer::CopyIntoBuffer( const wxBitmap& bitm )
{
    delete m_context;

    m_dc->SelectObject( wxNullBitmap );
    m_buffer = bitm;
    m_dc->SelectObject( m_buffer );
#if wxCHECK_VERSION(2, 9, 0)
    const wxMSWDCImpl* m_impl;
    m_impl = wxDynamicCast ( m_dc->GetImpl(), wxMSWDCImpl );
    m_context = new Graphics( ( HDC ) m_impl->GetHDC( ) );
#else
    m_context = new Graphics( ( HDC ) m_dc->GetHDC( ) );
#endif
}

void a2dGDIPlusDrawer::SetTransform( const a2dAffineMatrix& userToWorld )
{
    a2dDrawer2D::SetTransform( userToWorld );

    Matrix m(
        m_usertodevice( 0, 0 ), m_usertodevice( 0, 1 ),
        m_usertodevice( 1, 0 ), m_usertodevice( 1, 1 ),
        m_usertodevice( 2, 0 ), m_usertodevice( 2, 1 ) );
    m_context->SetTransform( &m );
}

void a2dGDIPlusDrawer::PushTransform()
{
    a2dDrawer2D::PushTransform();

    m_context->SetTransform( _get_gdiplus_user_to_device_transform() );
}

void a2dGDIPlusDrawer::PushTransform( const a2dAffineMatrix& affine )
{
    a2dDrawer2D::PushTransform( affine );

    m_context->SetTransform( _get_gdiplus_user_to_device_transform() );
}

void a2dGDIPlusDrawer::PushIdentityTransform()
{
    a2dDrawer2D::PushIdentityTransform(  );

    m_context->SetTransform( _get_gdiplus_user_to_device_transform() );
}

void a2dGDIPlusDrawer::PopTransform( void )
{
    a2dDrawer2D::PopTransform();

    m_context->SetTransform( _get_gdiplus_user_to_device_transform() );
}

void a2dGDIPlusDrawer::BeginDraw( )
{
    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dGDIPlusDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_display && m_beginDraw_endDraw == 0 )
    {
        DestroyClippingRegion();

        SetDrawStyle( m_drawstyle );
        SetActiveStroke( m_currentstroke );
        SetActiveFill( m_currentfill );

        m_deviceDC = new wxClientDC( m_display );
        m_display->PrepareDC( *m_deviceDC );
    }

    m_beginDraw_endDraw++;
}

void a2dGDIPlusDrawer::EndDraw( )
{
    m_beginDraw_endDraw--;

    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_display &&  m_beginDraw_endDraw == 0 )
    {
        delete m_deviceDC;
        m_deviceDC = NULL;
    }
}

void a2dGDIPlusDrawer::SetClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle )
{
    /*
        unsigned int n = points->size();
        m_cpointsDouble.resize(n);

        unsigned int i = 0;
        double x,y;
        forEachIn( a2dVertexList, points )
        {
            a2dPoint2D point = (*iter)->GetPoint();
            //transform to device
            GetUserToDeviceTransform().TransformPoint( point.m_x, point.m_y, x, y );
            m_cpointsDouble[i].x = x;
            m_cpointsDouble[i].y = y;
            i++;
        }

        if ( spline )
            n = ConvertSplinedPolygon2(n);

        wxPoint* intCPoints = new wxPoint[n];

        for ( i = 0; i < n; i++)
        {
            intCPoints[i].x = Round(m_cpointsDouble[i].x);
            intCPoints[i].y = Round(m_cpointsDouble[i].y);
        }

        m_clip = wxRegion( n, intCPoints, fillStyle );

        delete[] intCPoints;
    */
    GraphicsPath* path = createGraphicsPath( points, spline );
    m_context->SetClip( path, CombineModeReplace );
}

void a2dGDIPlusDrawer::ExtendAndPushClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle, a2dBooleanClip clipoperation )
{
    Region* push = new Region( );
    m_context->GetClip( push );
    m_clipping_region_stack.Insert( push );

    ExtendClippingRegion( points, spline, fillStyle, clipoperation );
    return;
}

void a2dGDIPlusDrawer::ExtendClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle, a2dBooleanClip clipoperation )
{
    GraphicsPath* path = createGraphicsPath( points, spline, fillStyle );

    if( !m_context->IsClipEmpty( ) )
    {
        switch( clipoperation )
        {
            case a2dCLIP_AND:
                m_context->SetClip( path, CombineModeIntersect );
                break ;

            case a2dCLIP_OR:
                m_context->SetClip( path, CombineModeUnion );
                break ;

            case a2dCLIP_XOR:
                m_context->SetClip( path, CombineModeXor );
                break ;

            case a2dCLIP_DIFF:
                m_context->SetClip( path, CombineModeExclude );
                break ;

            case a2dCLIP_COPY:
            default:
                m_context->SetClip( path, CombineModeReplace );
                break ;
        }
    }
    else
    {
        m_context->SetClip( path, CombineModeReplace );
    }
}

void a2dGDIPlusDrawer::PopClippingRegion( )
{
    wxASSERT( m_clipping_region_stack.GetCount( ) > 0 );

    Region* region = m_clipping_region_stack.GetFirst()->GetData( );
    //delete m_clipping_region_stack.GetFirst()->GetData();
    m_clipping_region_stack.DeleteNode( m_clipping_region_stack.GetLast( ) );
    m_context->SetClip( region, CombineModeReplace );
}

void a2dGDIPlusDrawer::SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height )
{
    m_clip = wxRegion( minx, miny, width, height );
    m_clipboxdev = wxRect( minx, miny, width, height );
    m_clipboxworld = ToWorld( m_clipboxdev );

    //we don't know which part of the transform set in m_context is to go from world to device.
    //remove all first.
    PushIdentityTransform();
    m_context->ResetClip();
    Rect clip_box( m_clipboxdev.x, m_clipboxdev.y, m_clipboxdev.GetWidth(), m_clipboxdev.GetHeight() );
    m_context->SetClip( clip_box, CombineModeReplace );
    PopTransform();
}

void a2dGDIPlusDrawer::SetClippingRegion( double minx, double miny, double maxx, double maxy )
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
    Rect clip_box( m_clipboxdev.x, m_clipboxdev.y, m_clipboxdev.GetWidth(), m_clipboxdev.GetHeight() );
    m_context->SetClip( clip_box, CombineModeReplace );

    PopTransform();
}

void a2dGDIPlusDrawer::DestroyClippingRegion()
{
    m_context->ResetClip( );
    m_clipboxdev = wxRect( 0, 0, m_width, m_height );
    m_clipboxworld = ToWorld( m_clipboxdev );
}

void a2dGDIPlusDrawer::ResetStyle()
{
    a2dDrawer2D::ResetStyle();
    SetDrawerFill( *a2dBLACK_FILL ); //set to a sure state
    SetDrawerStroke( *a2dBLACK_STROKE );
    SetDrawStyle( m_drawstyle );
}

void a2dGDIPlusDrawer::DoSetActiveStroke()
{
    if (  m_activestroke.IsNoStroke() || m_activestroke.GetStyle() == a2dSTROKE_TRANSPARENT )
    {
        wxColour color = m_activestroke.GetColour( );
        m_current_pen = new Pen( Color( m_StrokeOpacityCol1, color.Red( ), color.Green( ), color.Blue( ) ), 1 );
    }
    else if ( m_drawstyle == a2dWIREFRAME_INVERT_ZERO_WIDTH )
    {
        wxColour color = m_activestroke.GetColour( );
        m_current_pen = new Pen( Color( m_StrokeOpacityCol1, color.Red( ), color.Green( ), color.Blue( ) ), 1 );
    }
    else if ( m_activestroke.GetType() == a2dSTROKE_ONE_COLOUR )
    {
        wxColour color = m_activestroke.GetColour( );
        m_current_pen = new Pen( Color( m_StrokeOpacityCol1, color.Red( ), color.Green( ), color.Blue( ) ) );
        DashStyle dashStyle = DashStyleSolid;
        switch( m_activestroke.GetStyle() )
        {
            case a2dSTROKE_SOLID:
                break;
            case a2dSTROKE_DOT:
                dashStyle = DashStyleDot;
                break;
            case a2dSTROKE_DOT_DASH:
                dashStyle = DashStyleDashDot;
                break;
            case a2dSTROKE_LONG_DASH:
                dashStyle = DashStyleDash;
                break;
            case a2dSTROKE_SHORT_DASH:
                dashStyle = DashStyleDash;
                break;
            case a2dSTROKE_TRANSPARENT:
                dashStyle = DashStyleDash;
                break;
            case a2dSTROKE_STIPPLE:
            case a2dSTROKE_STIPPLE_MASK_OPAQUE:
            {
                wxBitmap bmp = m_activestroke.GetStipple();
                if ( bmp.Ok() )
                {
                    m_penImage = Bitmap::FromHBITMAP( ( HBITMAP )bmp.GetHBITMAP(), ( HPALETTE )bmp.GetPalette()->GetHPALETTE() );
                    m_penBrush = new TextureBrush( m_penImage );
                    m_current_pen->SetBrush( m_penBrush );
                }

            }
            default :
                //if ( m_activestroke.GetStyle() >= wxFIRST_HATCH && m_activestroke.GetStyle() <= wxLAST_HATCH )
            {
                HatchStyle style = HatchStyleHorizontal;
                switch( m_activestroke.GetStyle() )
                {
                    case a2dSTROKE_BDIAGONAL_HATCH:
                        style = HatchStyleBackwardDiagonal;
                        break;
                    case a2dSTROKE_CROSSDIAG_HATCH:
                        style = HatchStyleDiagonalCross;
                        break;
                    case a2dSTROKE_FDIAGONAL_HATCH:
                        style = HatchStyleForwardDiagonal;
                        break;
                    case a2dSTROKE_CROSS_HATCH:
                        style = HatchStyleCross;
                        break;
                    case a2dSTROKE_HORIZONTAL_HATCH:
                        style = HatchStyleHorizontal;
                        break;
                    case a2dSTROKE_VERTICAL_HATCH:
                        style = HatchStyleVertical;
                        break;
                }
                m_penBrush = new HatchBrush( style, Color( m_StrokeOpacityCol1, m_activestroke.GetColour().Red() ,
                                             m_activestroke.GetColour().Green() , m_activestroke.GetColour().Blue() ), Color::Transparent );
                m_current_pen->SetBrush( m_penBrush );
            }
            break;
        }
        if ( dashStyle != DashStyleSolid )
            m_current_pen->SetDashStyle( dashStyle );


        double strokewidth;

        strokewidth = m_activestroke.GetWidth();
        if ( m_drawstyle == a2dWIREFRAME_ZERO_WIDTH )
        {
            m_current_pen->SetLineJoin( LineJoinRound );
            m_current_pen->SetLineCap( LineCapRound, LineCapRound, DashCapFlat );
            m_current_pen->SetWidth( 1 );
            strokewidth = DeviceToWorldXRel( 1 );
        }
        else
        {
            LineJoin join;
            switch( m_activestroke.GetJoin( ) )
            {
                case wxJOIN_MITER:
                    join = LineJoinMiter;
                    break;
                case wxJOIN_ROUND:
                    join = LineJoinRound;
                    break;
                case wxJOIN_BEVEL:
                    join = LineJoinBevel;
                    break;
                default:
                    join = LineJoinRound;
                    break;
            }
            m_current_pen->SetLineJoin( join );

            LineCap cap;
            switch( m_activestroke.GetCap( ) )
            {
                case wxCAP_BUTT:
                    cap = LineCapFlat;
                    break;
                case wxCAP_ROUND:
                    cap = LineCapRound;
                    break;
                    // Stavros: What is projecting? I shall write it as triangle and discuss.
                case wxCAP_PROJECTING:
                    cap = LineCapSquare;
                    break;
                default:
                    cap = LineCapFlat;
                    break;
            }
            m_current_pen->SetLineCap( cap, cap, DashCapFlat );

            if ( !m_activestroke.GetPixelStroke() )
            {
                strokewidth = WorldToDeviceXRel( m_activestroke.GetWidth() );
            }
            //if ( m_activestroke.GetPixelStroke() )
            //    strokewidth = WorldToDeviceXRel( m_activestroke.GetWidth() );
        }

        strokewidth = !strokewidth ? 1 : strokewidth;
        m_current_pen->SetWidth( strokewidth );
    }
}

void a2dGDIPlusDrawer::DoSetActiveFill()
{
    wxColour color1 = m_activefill.GetColour();
    wxColour color2 = m_activefill.GetColour2();

    if ( !m_activefill.GetFilling() )
    {
        Color gdicolor1( m_FillOpacityCol1, color1.Red( ), color1.Green( ), color1.Blue( ) );
        m_current_brush = new SolidBrush( gdicolor1 );
        return;
    }
    else if ( m_activefill.GetStyle() == a2dFILL_TRANSPARENT )
    {
        Color gdicolor1( m_FillOpacityCol1, color1.Red( ), color1.Green( ), color1.Blue( ) );
        m_current_brush = new SolidBrush( gdicolor1 );
        return;
    }
    else if ( m_activefill.IsNoFill() )
    {
        Color gdicolor1( m_FillOpacityCol1, color1.Red( ), color1.Green( ), color1.Blue( ) );
        m_current_brush = new SolidBrush( gdicolor1 );
        return;
    }

    if ( m_activefill.GetType() == a2dFILL_ONE_COLOUR )
    {
        Color gdicolor1( m_FillOpacityCol1, color1.Red( ), color1.Green( ), color1.Blue( ) );
        Color gdicolor2( m_FillOpacityCol2, color2.Red( ), color2.Green( ), color2.Blue( ) );

        bool nohatch = false;
        HatchStyle style = HatchStyleHorizontal;
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_TRANSPARENT:
                nohatch = true;
                break;
            case a2dFILL_SOLID:
                nohatch = true;
                break;
            case a2dFILL_BDIAGONAL_HATCH:
                style = HatchStyleBackwardDiagonal;
                break;
            case a2dFILL_CROSSDIAG_HATCH:
                style = HatchStyleDiagonalCross;
                break;
            case a2dFILL_FDIAGONAL_HATCH:
                style = HatchStyleForwardDiagonal;
                break;
            case a2dFILL_CROSS_HATCH:
                style = HatchStyleCross;
                break;
            case a2dFILL_HORIZONTAL_HATCH:
                style = HatchStyleHorizontal;
                break;
            case a2dFILL_VERTICAL_HATCH:
                style = HatchStyleVertical;
                break;
            default:
                nohatch = true;
                break;
        }
        if ( nohatch )
            m_current_brush = new SolidBrush( gdicolor1 );
        else
            m_current_brush = new HatchBrush( style, gdicolor1, gdicolor2 );
    }
    else if ( m_activefill.GetType() == a2dFILL_HATCH_TWO_COLOUR )
    {
        Color gdicolor1( m_FillOpacityCol1, color1.Red( ), color1.Green( ), color1.Blue( ) );
        Color gdicolor2( m_FillOpacityCol2, color2.Red( ), color2.Green( ), color2.Blue( ) );

        bool nohatch = false;
        HatchStyle style = HatchStyleHorizontal;
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_TWOCOL_BDIAGONAL_HATCH:
                style = HatchStyleBackwardDiagonal;
                break;
            case a2dFILL_TWOCOL_CROSSDIAG_HATCH:
                style = HatchStyleDiagonalCross;
                break;
            case a2dFILL_TWOCOL_FDIAGONAL_HATCH:
                style = HatchStyleForwardDiagonal;
                break;
            case a2dFILL_TWOCOL_CROSS_HATCH:
                style = HatchStyleCross;
                break;
            case a2dFILL_TWOCOL_HORIZONTAL_HATCH:
                style = HatchStyleHorizontal;
                break;
            case a2dFILL_TWOCOL_VERTICAL_HATCH:
                style = HatchStyleVertical;
                break;
            default:
                nohatch = true;
                break;
        }
        if ( nohatch )
            m_current_brush = new SolidBrush( gdicolor1 );
        else
            m_current_brush = new HatchBrush( style, gdicolor1, gdicolor2 );
    }
    else if ( m_activefill.GetType() == a2dFILL_BITMAP )
    {
        wxBrush dcbrush;
        //wxBrushStyle style;
        switch( m_activefill.GetStyle() )
        {
            case a2dFILL_STIPPLE:
            case a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT:
            case a2dFILL_STIPPLE_MASK_OPAQUE:
            {
                wxBitmap bmp = m_activefill.GetStipple();
                if ( bmp.Ok() )
                {
                    wxDELETE( m_brushImage );
                    m_brushImage = Bitmap::FromHBITMAP( ( HBITMAP )bmp.GetHBITMAP(), ( HPALETTE )bmp.GetPalette()->GetHPALETTE() );
                    m_current_brush = new TextureBrush( m_brushImage );
                }
            }
            break;
            default:
                break;
        }
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
    {
        a2dPoint2D p1 = m_activefill.GetStart();
        a2dPoint2D p2 = m_activefill.GetStop();
        Color gdicolor1( m_FillOpacityCol1, color1.Red( ), color1.Green( ), color1.Blue( ) );
        Color gdicolor2( m_FillOpacityCol2, color2.Red( ), color2.Green( ), color2.Blue( ) );
        m_current_brush = new LinearGradientBrush( PointF( p1.m_x, p1.m_y ) , PointF( p2.m_x, p2.m_x ), gdicolor1, gdicolor2 );
    }
    else if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        Color gdicolor1( m_FillOpacityCol1, color1.Red( ), color1.Green( ), color1.Blue( ) );
        Color gdicolor2( m_FillOpacityCol2, color2.Red( ), color2.Green( ), color2.Blue( ) );
        double xc = m_activefill.GetCenter().m_x;
        double yc = m_activefill.GetCenter().m_y;
        double xo = m_activefill.GetFocal().m_x;
        double yo = m_activefill.GetFocal().m_y;
        double radius = m_activefill.GetRadius();

        // Create a path that consists of a single circle.
        m_brushPath = new GraphicsPath();
        m_brushPath->AddEllipse( ( REAL )( xc - radius ), ( REAL )( yc - radius ), ( REAL )( 2 * radius ), ( REAL )( 2 * radius ) );

        PathGradientBrush* b = new PathGradientBrush( m_brushPath );
        m_current_brush = b;
        b->SetCenterPoint( PointF( xo, yo ) );
        b->SetCenterColor( gdicolor1 );

        Color colors[] = {gdicolor2};
        int count = 1;
        b->SetSurroundColors( colors, &count );
    }

}

// Stavros: This is a pure virtual function which just sets the
// m_drawstyle member. But why pure virtual?
void a2dGDIPlusDrawer::DoSetDrawStyle( a2dDrawStyle drawstyle )
{
    m_drawstyle = drawstyle;
}

void a2dGDIPlusDrawer::DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize )
{
    if ( m_disableDrawing )
        return;

    if ( !width || !height )
        return;

    if ( pixelsize )
    {
        width = DeviceToWorldXRel( width );
        height = DeviceToWorldYRel( height );
        radius = DeviceToWorldXRel( radius );
        if( m_yaxis )
            height = -height;
    }
    if ( width < 0 )
    {
        x += width; width = -width;
    }
    if ( height < 0 )
    {
        y += height;
        height = -height;
    }

    if ( fabs( radius ) <= 0.00000001 )
    {
        m_context->SetTransform( _get_gdiplus_user_to_device_transform() );
        m_context->FillRectangle( m_current_brush, ( REAL ) x, ( REAL ) y, ( REAL ) width, ( REAL ) height );
        if (  !m_currentstroke.IsNoStroke() && !m_currentstroke.GetStyle() == a2dSTROKE_TRANSPARENT  )
            m_context->DrawRectangle( m_current_pen, ( REAL ) x, ( REAL ) y, ( REAL ) width, ( REAL )height );
        return;
    }

    m_context->SetTransform( _get_gdiplus_user_to_device_transform() );
    m_context->FillRectangle( m_current_brush, ( REAL ) x, ( REAL ) y, ( REAL ) width, ( REAL ) height );
    if (  !m_currentstroke.IsNoStroke() && !m_currentstroke.GetStyle() == a2dSTROKE_TRANSPARENT  )
        m_context->DrawRectangle( m_current_pen, ( REAL ) x, ( REAL ) y, ( REAL ) width, ( REAL ) height );
}

void a2dGDIPlusDrawer::DrawCircle( double x, double y, double radius )
{
    DrawEllipse( x, y, radius * 2, radius * 2 );
}

void a2dGDIPlusDrawer::DrawPoint( double xc, double yc )
{
    m_usertodevice.TransformPoint( xc, yc, xc, yc );
    DeviceDrawPixel( xc, yc, m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
}

void a2dGDIPlusDrawer::DrawEllipse( double xc, double yc, double width, double height )
{
    if ( m_disableDrawing )
        return;

    m_context->SetTransform( _get_gdiplus_user_to_device_transform() );
    m_context->FillEllipse( m_current_brush, ( REAL ) xc - width / 2, ( REAL ) yc - height / 2, ( REAL ) width, ( REAL ) height );

    if (  !m_currentstroke.IsNoStroke() && !m_currentstroke.GetStyle() == a2dSTROKE_TRANSPARENT  )
        m_context->DrawEllipse( m_current_pen, ( REAL ) xc - width / 2, ( REAL ) yc - height / 2, ( REAL ) width, ( REAL ) height );
}

void a2dGDIPlusDrawer::DrawImage(  const wxImage& image, double xc, double yc, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

//    Bitmap* bitmap = new Bitmap(width, height, width,
//            PixelFormat24bppRGB, (BYTE*) image.GetData() );

    bool hasAlpha = image.HasAlpha();
    size_t w = image.GetWidth();
    size_t h = image.GetHeight();
    Bitmap* bitmap = new Bitmap( w, h, PixelFormat32bppPARGB );
    Rect bounds( 0, 0, w, h );
    BitmapData data ;

    bitmap->LockBits( &bounds, ImageLockModeWrite,
                      bitmap->GetPixelFormat(), &data );


    // Write to the temporary buffer provided by LockBits.
    UINT* pixels = ( UINT* )data.Scan0;

    const int srcBytesPerLine = w * 3;
    const int dstBytesPerLine = w * 4;
    const unsigned char* src = image.GetData() + ( ( h - 1 ) * srcBytesPerLine );
    const unsigned char* alpha = hasAlpha ? image.GetAlpha() + ( h - 1 ) * w
                                 : NULL;
    unsigned char* dstLineStart = ( unsigned char* )pixels;
    for ( int y = 0; y < h; y++ )
    {
        // copy one DIB line
        unsigned char* dst = dstLineStart;
        if ( alpha )
        {
            for ( int x = 0; x < w; x++ )
            {
                // RGB order is reversed, and we need to premultiply
                // all channels by alpha value for use with ::AlphaBlend.
                const unsigned char a = *alpha++;
                *dst++ = ( unsigned char )( ( src[2] * a + 127 ) / 255 );
                *dst++ = ( unsigned char )( ( src[1] * a + 127 ) / 255 );
                *dst++ = ( unsigned char )( ( src[0] * a + 127 ) / 255 );
                *dst++ = a;
                src += 3;
            }
        }
        else // no alpha channel
        {
            for ( int x = 0; x < w; x++ )
            {
                // RGB order is reversed.
                *dst++ = ( unsigned char )( ( src[2] * m_OpacityFactor + 127 ) / 255 );
                *dst++ = ( unsigned char )( ( src[1] * m_OpacityFactor + 127 ) / 255 );
                *dst++ = ( unsigned char )( ( src[0] * m_OpacityFactor + 127 ) / 255 );
                *dst++ = m_OpacityFactor;
                src += 3;
            }
        }

        // pass to the previous line in the image
        src -= 2 * srcBytesPerLine;
        if ( alpha )
            alpha -= 2 * w;

        // and to the next one in the DIB
        dstLineStart += dstBytesPerLine;
    }
    bitmap->UnlockBits( &data );

    if ( bitmap )
        m_context->DrawImage( bitmap, ( REAL ) xc - width / 2.0, ( REAL ) yc - height / 2.0, ( REAL ) width, ( REAL ) height ) ;
    delete bitmap;
}

void a2dGDIPlusDrawer::DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    //m_context->DrawImage(image,(REAL) x,(REAL) y,(REAL) w,(REAL) h) ;
}

#include "Gdipluspixelformats.h"

void a2dGDIPlusDrawer::DeviceDrawPolygon( unsigned int n, bool spline, wxPolygonFillMode fillStyle )
{
    if ( spline )
        n = ConvertSplinedPolygon2( n );

    Matrix ident;
    m_context->SetTransform( &ident );

    //just 2 points in wxINVERT mode makes it invisible
    if ( m_drawstyle == a2dWIREFRAME_INVERT || n == 2 )
    {
        GraphicsPath path;
        unsigned int i;
        for ( i = 0; i < n; i++ )
        {
            if ( i == 0 )
                path.StartFigure();
            else
                path.AddLine( ( REAL ) m_cpointsDouble[i - 1].x, ( REAL ) m_cpointsDouble[i - 1].y, ( REAL ) m_cpointsDouble[i].x, ( REAL ) m_cpointsDouble[i].y );
        }
        path.CloseFigure();
        m_context->DrawPath( m_current_pen, &path );
    }
    else if ( m_currentfill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR || m_currentfill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        GraphicsPath path;
        unsigned int i;
        for ( i = 0; i < n; i++ )
        {
            if ( i == 0 )
                path.StartFigure();
            else
                path.AddLine( ( REAL ) m_cpointsDouble[i - 1].x, ( REAL ) m_cpointsDouble[i - 1].y, ( REAL ) m_cpointsDouble[i].x, ( REAL ) m_cpointsDouble[i].y );
        }
        path.CloseFigure();
        m_context->FillPath( m_current_brush, &path );

        if (  !m_currentstroke.IsNoStroke() && !m_currentstroke.GetStyle() == a2dSTROKE_TRANSPARENT  )
            m_context->DrawPath( m_current_pen, &path );
    }
    else if ( m_currentfill.GetType() == a2dFILL_BITMAP )
    {
        GraphicsPath path;
        unsigned int i;
        for ( i = 0; i < n; i++ )
        {
            if ( i == 0 )
                path.StartFigure();
            else
                path.AddLine( ( REAL ) m_cpointsDouble[i - 1].x, ( REAL ) m_cpointsDouble[i - 1].y, ( REAL ) m_cpointsDouble[i].x, ( REAL ) m_cpointsDouble[i].y );
        }
        path.CloseFigure();
        m_context->FillPath( m_current_brush, &path );

        if (  !m_currentstroke.IsNoStroke() && !m_currentstroke.GetStyle() == a2dSTROKE_TRANSPARENT  )
            m_context->DrawPath( m_current_pen, &path );
    }
    else
    {
        GraphicsPath path;
        unsigned int i;
        for ( i = 0; i < n; i++ )
        {
            if ( i == 0 )
                path.StartFigure();
            else
                path.AddLine( ( REAL ) m_cpointsDouble[i - 1].x, ( REAL ) m_cpointsDouble[i - 1].y, ( REAL ) m_cpointsDouble[i].x, ( REAL ) m_cpointsDouble[i].y );
        }
        path.CloseFigure();
        m_context->FillPath( m_current_brush, &path );

        if (  !m_currentstroke.IsNoStroke() && !m_currentstroke.GetStyle() == a2dSTROKE_TRANSPARENT  )
            m_context->DrawPath( m_current_pen, &path );
    }

    if ( !m_currentstroke.IsSameAs( *a2dTRANSPARENT_STROKE ) && m_currentstroke.GetType() == a2dSTROKE_OBJECT )
    {
        //we must make a copy of the device points now because recursive call
        //can be made in here, which will change the points stored in de drawer.
        //First point repeated extra since it is for outline
        wxRealPoint* copy = new wxRealPoint[n + 1];
        memcpy( copy, &m_cpointsDouble[0], n * sizeof( wxRealPoint ) );
        copy[n].x = m_cpointsDouble[0].x;
        copy[n].y = m_cpointsDouble[0].y;

        //m_currentstroke.Render( this, m_clipboxworld, n+1, copy );
        delete [] copy;
    }

}

void a2dGDIPlusDrawer::DeviceDrawLines( unsigned int n, bool spline )
{
    if ( spline )
        n = ConvertSplinedPolyline2( n );

    Matrix ident;
    m_context->SetTransform( &ident );

    GraphicsPath path;
    unsigned int i;
    for ( i = 0; i < n; i++ )
    {
        if ( i == 0 )
            path.StartFigure();
        else
            path.AddLine( ( REAL ) m_cpointsDouble[i - 1].x, ( REAL ) m_cpointsDouble[i - 1].y, ( REAL ) m_cpointsDouble[i].x, ( REAL ) m_cpointsDouble[i].y );
    }
    m_context->DrawPath( m_current_pen, &path );
}

void a2dGDIPlusDrawer::DeviceDrawLine( double x1, double y1, double x2, double y2 )
{
    Matrix ident;
    m_context->SetTransform( &ident );
    if (  !m_currentstroke.IsNoStroke() && !m_currentstroke.GetStyle() == a2dSTROKE_TRANSPARENT  )
        m_context->DrawLine( m_current_pen, ( REAL ) x1, ( REAL ) y1, ( REAL ) x2, ( REAL ) y2 );
}

void a2dGDIPlusDrawer::DeviceDrawHorizontalLine( int x1, int y1, int x2, bool use_stroke_color )
{
    if ( x1 > x2 )
    {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }

    // clip to clipping region
    if ( x2 < m_clipboxdev.x )
        return;

    if ( x1 >= m_clipboxdev.x + m_clipboxdev.width )
        return;

    if ( x1 < m_clipboxdev.x ) x1 = m_clipboxdev.x;
    if ( x2 >= m_clipboxdev.x + m_clipboxdev.width ) x2 = m_clipboxdev.x + m_clipboxdev.width - 1;
    if ( y1 >=  m_clipboxdev.y && y1 < m_clipboxdev.y + m_clipboxdev.height )
    {
        //if (!use_stroke_color)
        Matrix ident;
        m_context->SetTransform( &ident );
        m_context->DrawLine( m_current_pen, ( REAL ) x1, ( REAL ) y1, ( REAL ) x2, ( REAL ) y1 );
    }
}

void a2dGDIPlusDrawer::DeviceDrawVerticalLine( int x1, int y1, int y2, bool use_stroke_color )
{
    if ( y1 > y2 )
    {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    // clip to clipping region
    if ( y2 < m_clipboxdev.y )
        return;

    if ( y1 >= m_clipboxdev.y + m_clipboxdev.height )
        return;

    if ( y1 < m_clipboxdev.y ) y1 = m_clipboxdev.y;
    if ( y2 >= m_clipboxdev.y + m_clipboxdev.height ) y2 = m_clipboxdev.y + m_clipboxdev.height - 1;
    if ( x1 >=  m_clipboxdev.x && x1 < m_clipboxdev.x + m_clipboxdev.width )
    {
    }
}

void a2dGDIPlusDrawer::DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
    Matrix ident;
    m_context->SetTransform( &ident );
    Pen tmp( Color( a, r, g, b ), 1 );
    m_context->DrawLine( &tmp, x1, y1, x1, y1 );
}

// protected functions that only a2dGDIPlusDrawer may use.
GraphicsPath* a2dGDIPlusDrawer::createGraphicsPath( a2dVertexList* list, bool spline, wxPolygonFillMode fillStyle )
{
    unsigned int n = list->size();
    unsigned int i = 0;

    GraphicsPath ret;

    switch( fillStyle )
    {
        case wxODDEVEN_RULE:
            ret.SetFillMode( FillModeAlternate );
            break;
        default:
            ret.SetFillMode( FillModeWinding );
            break;
    }

#ifdef wxUSE_INTPOINT
    Point* points = new Point[ n ];
#else
    PointF* points = new PointF[ n ];
#endif

    forEachIn( a2dVertexList, list )
    {
        a2dPoint2D point = ( *iter )->GetPoint();
#ifdef wxUSE_INTPOINT
        points[ i ] = Point( point.m_x, point.m_y );
#else
        points[ i ] = PointF( point.m_x, point.m_y );
#endif
        i++;
    }

    if( spline == true )
        ret.AddCurve( points, n );
    else
        ret.AddLines( points, n );

    delete [ ] points;
    return ret.Clone( );
}

void a2dGDIPlusDrawer::BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos )
{
    if ( rect.x + rect.width > m_buffer.GetWidth() )
        rect.width = m_buffer.GetWidth() - rect.x;
    if ( rect.y + rect.height > m_buffer.GetHeight() )
        rect.height = m_buffer.GetHeight() - rect.y;

    if ( ( rect.width > 0 ) && ( rect.height > 0 ) )
    {
        dc->Blit( rect.x - bufferpos.x, rect.y - bufferpos.y, rect.width, rect.height, m_dc, rect.x, rect.y, wxCOPY, false );
    }
}

void a2dGDIPlusDrawer::BlitBuffer( wxRect rect, const wxPoint& bufferpos )
{
    BeginDraw();
    BlitBuffer( m_deviceDC, rect, bufferpos );
    EndDraw();
}

void a2dGDIPlusDrawer::ShiftBuffer( int dxy, bool yshift )
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

void a2dGDIPlusDrawer::DrawCharStroke( wxChar c )
{
    a2dDrawer2D::DrawCharStroke( c );
    return;

//TODO pen becomes far to big, effected by transform? Using above it is draw with no transform.

    // set text linestroke.
    a2dStroke stroke = a2dStroke( m_activestroke.GetColour(), m_currentfont.GetStrokeWidth() );
    a2dStroke oldstroke = m_activestroke;
    SetActiveStroke( stroke );

    // scale character to size
    a2dAffineMatrix affine;
    affine.Scale( m_currentfont.GetSize() );
    PushTransform( affine );

    GraphicsPath path;
    a2dVertexList** ptr = m_currentfont.GetGlyphStroke( c );
    if ( ptr )
    {
        while ( *ptr )
        {
            a2dVertexList::iterator iter = ( *ptr )->begin();
            if ( ( *ptr )->size() )
            {
                path.StartFigure();
            }
            while ( iter != ( *ptr )->end() )
            {
                a2dPoint2D point = ( *iter )->GetPoint();
                path.AddLine( ( REAL ) point.m_x, ( REAL ) point.m_y, ( REAL ) point.m_x, ( REAL ) point.m_y );
                iter++;
            }
            ptr++;
        }
    }

    m_context->DrawPath( m_current_pen, &path );

    // restore context
    PopTransform();
    SetActiveStroke( oldstroke );
}


void a2dGDIPlusDrawer::DrawCharFreetype( wxChar c )
{
    double x, y;
    x = y = 0.0;

    if ( m_disableDrawing )
        return;

    double scale = m_currentfont.GetSize() / ( 64 * m_currentfont.GetDeviceHeight() );

#if wxART2D_USE_FREETYPE
    y += m_currentfont.GetDescent();

    FT_Glyph glyph = m_currentfont.GetGlyphFreetype( c )->m_glyph;
    if ( glyph->format != FT_GLYPH_FORMAT_OUTLINE )
        return;
    FT_Outline& outline = ( ( FT_OutlineGlyph ) glyph )->outline;

    FT_Vector   v_last;
    FT_Vector   v_control;
    FT_Vector   v_start;

    FT_Vector*  point;
    FT_Vector*  limit;
    char*       tags;

    int   n;         // index of contour in outline
    int   first;     // index of first point in contour
    char  tag;       // current point's state

    first = 0;

    GraphicsPath path;
    a2dPoint2D pointprev;
    for( n = 0; n < outline.n_contours; n++ )
    {
        int  last;  // index of last point in contour

        last  = outline.contours[n];
        limit = outline.points + last;

        v_start = outline.points[first];
        v_last  = outline.points[last];

        v_control = v_start;

        point = outline.points + first;
        tags  = outline.tags  + first;
        tag   = FT_CURVE_TAG( tags[0] );

        // A contour cannot start with a cubic control point!
        if( tag == FT_CURVE_TAG_CUBIC ) return;

        // check first point to determine origin
        if( tag == FT_CURVE_TAG_CONIC )
        {
            // first point is conic control.  Yes, this happens.
            if( FT_CURVE_TAG( outline.tags[last] ) == FT_CURVE_TAG_ON )
            {
                // start at last point if it is on the curve
                v_start = v_last;
                limit--;
            }
            else
            {
                // if both first and last points are conic,
                // start at their middle and record its position
                // for closure
                v_start.x = ( v_start.x + v_last.x ) / 2;
                v_start.y = ( v_start.y + v_last.y ) / 2;

                v_last = v_start;
            }
            point--;
            tags--;
        }

        path.StartFigure();
        pointprev = a2dPoint2D( v_start.x, v_start.y );
        path.AddLine( ( REAL ) v_start.x, ( REAL ) v_start.y, ( REAL ) v_start.x, ( REAL ) v_start.y );

        while( point < limit )
        {
            point++;
            tags++;

            tag = FT_CURVE_TAG( tags[0] );
            switch( tag )
            {
                case FT_CURVE_TAG_ON:  // emit a single line_to
                {
                    path.AddLine( ( REAL ) point->x, ( REAL ) point->y, ( REAL ) point->x, ( REAL ) point->y );
                    continue;
                }

                case FT_CURVE_TAG_CONIC:  // consume conic arcs
                {
                    v_control.x = point->x;
                    v_control.y = point->y;

Do_Conic:
                    if( point < limit )
                    {
                        FT_Vector vec;
                        FT_Vector v_middle;

                        point++;
                        tags++;
                        tag = FT_CURVE_TAG( tags[0] );

                        vec.x = point->x;
                        vec.y = point->y;

                        if( tag == FT_CURVE_TAG_ON )
                        {
                            PointF c1( v_control.x, v_control.y );
                            PointF c2( v_control.x, v_control.y );
                            PointF end( vec.x, vec.y );
                            PointF start;
                            path.GetLastPoint( &start );
                            path.AddBezier( start, c1, c2, end );
                            continue;
                        }

                        if( tag != FT_CURVE_TAG_CONIC ) return;

                        v_middle.x = ( v_control.x + vec.x ) / 2;
                        v_middle.y = ( v_control.y + vec.y ) / 2;

                        PointF c1( v_control.x, v_control.y );
                        PointF c2( v_control.x, v_control.y );
                        PointF end( v_middle.x, v_middle.y );
                        PointF start;
                        path.GetLastPoint( &start );
                        path.AddBezier( start, c1, c2, end );

                        v_control = vec;
                        goto Do_Conic;
                    }
                    PointF c1( v_control.x, v_control.y );
                    PointF c2( v_control.x, v_control.y );
                    PointF end( v_start.x, v_start.y );
                    PointF start;
                    path.GetLastPoint( &start );
                    path.AddBezier( start, c1, c2, end );
                    goto Close;
                }

                default:  // FT_CURVE_TAG_CUBIC
                {
                    FT_Vector vec1, vec2;

                    if( point + 1 > limit || FT_CURVE_TAG( tags[1] ) != FT_CURVE_TAG_CUBIC )
                    {
                        return;
                    }

                    vec1.x = point[0].x;
                    vec1.y = point[0].y;
                    vec2.x = point[1].x;
                    vec2.y = point[1].y;

                    point += 2;
                    tags  += 2;

                    if( point <= limit )
                    {
                        FT_Vector vec;

                        vec.x = point->x;
                        vec.y = point->y;

                        PointF c1( vec1.x, vec1.y );
                        PointF c2( vec2.x, vec2.y );
                        PointF end( vec.x, vec.y );
                        PointF start;
                        path.GetLastPoint( &start );
                        path.AddBezier( start, c1, c2, end );

                        continue;
                    }

                    PointF c1( vec1.x, vec1.y );
                    PointF c2( vec2.x, vec2.y );
                    PointF end( v_start.x, v_start.y );
                    PointF start;
                    path.GetLastPoint( &start );
                    path.AddBezier( start, c1, c2, end );

                    goto Close;
                }
            }
        }

        path.CloseFigure();

Close:
        first = last + 1;
    }

    // scale font to size
    a2dAffineMatrix affine;
    affine.Scale( scale );
    affine.Translate( x, y );
    affine = GetUserToDeviceTransform() * affine;

    Matrix charTransform(
        ( REAL )affine( 0, 0 ), ( REAL )affine( 0, 1 ),
        ( REAL )affine( 1, 0 ), ( REAL )affine( 1, 1 ),
        ( REAL )affine( 2, 0 ), ( REAL )affine( 2, 1 )   );

    m_context->SetTransform( &charTransform );

    wxColour color1 = m_currentstroke.GetColour();
    Color gdicolor1( m_StrokeOpacityCol1, color1.Red( ), color1.Green( ), color1.Blue( ) );
    SolidBrush* text_brush = new SolidBrush( gdicolor1 );
    m_context->FillPath( text_brush, &path );
#else // wxART2D_USE_FREETYPE
    a2dDrawer2D::DrawCharFreetype( c );
#endif // wxART2D_USE_FREETYPE
}

void a2dGDIPlusDrawer::DrawCharDc( wxChar c )
{
#if wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
    if ( m_currentfont.GetType() == a2dFONT_WXDC && m_currentfont.GetFreetypeFont().Ok() )
    {
        a2dFont oldfont = m_currentfont;
        m_currentfont = m_currentfont.GetFreetypeFont();
        DrawCharFreetype( c );
        m_currentfont = oldfont;
    }
    else
        a2dDrawer2D::DrawCharDc( c );
#else // wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
    a2dDrawer2D::DrawCharDc( c );
#endif // wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
}

void a2dGDIPlusDrawer::DrawTextDc( const wxString& text, double x, double y )
{
    a2dDrawer2D::DrawTextDc( text, x, y );
    return;
    /*
        wxFont* wxdcfont = m_currentfont.GetFont();

        wxWCharBuffer s = wxdcfont->GetFaceName().wc_str( *wxConvUI );
        int size = wxdcfont->GetPointSize();
        int style = FontStyleRegular;
        if ( wxdcfont->GetStyle() == wxFONTSTYLE_ITALIC )
            style |= FontStyleItalic;
        if ( wxdcfont->GetUnderlined() )
            style |= FontStyleUnderline;
        if ( wxdcfont->GetWeight() == wxFONTWEIGHT_BOLD )
            style |= FontStyleBold;
        Font* gdifont = new Font( s , size , style );

        wxColour color1 = m_currentstroke.GetColour();
        Color gdicolor1( m_StrokeOpacityCol1, color1.Red( ), color1.Green( ), color1.Blue( ) );
        SolidBrush* text_brush = new SolidBrush( gdicolor1 );

        wxWCharBuffer convtext = text.wc_str( *wxConvUI );
        m_context->DrawString( convtext, -1 , gdifont, PointF( x , y ) , StringFormat::GenericTypographic() , text_brush );
    */
}

#endif // wxART2D_USE_GDIPLUS_DRAWER
