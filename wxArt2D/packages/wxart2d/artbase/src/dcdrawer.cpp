/*! \file artbase/src/dcdrawer.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: dcdrawer.cpp,v 1.87 2009/10/06 18:40:31 titato Exp $
*/


#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include <wx/wfstream.h>
#include "wx/artbase/drawer2d.h"
#include "wx/artbase/stylebase.h"
#include "wx/artbase/dcdrawer.h"

#if defined(__WXMSW__)
#include "wx/artbase/mswfont.h"
#endif
#endif

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
/*
#elif __WXGTK__
#include <gtk/gtk.h>
//#include <gdk/gdkrgb.h>
#include <wx/dcps.h>
#if wxCHECK_VERSION(2,9,0)
#include "wx/gtk/dcclient.h"
#else
#include "wx/gtk/win_gtk.h"
#endif
#endif
*/

#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
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


IMPLEMENT_DYNAMIC_CLASS( a2dDcDrawer, a2dDrawer2D )
IMPLEMENT_DYNAMIC_CLASS( a2dMemDcDrawer, a2dDcDrawer )

/*******************************************************************
a2dDcDrawer
********************************************************************/

a2dDcDrawer::a2dDcDrawer( const wxSize& size ): a2dDrawer2D( size )
{
    m_externalDc = false;
    m_renderDC = m_deviceDC = 0;
    m_clip.Clear();
}

a2dDcDrawer::a2dDcDrawer( int width, int height ): a2dDrawer2D( width, height )
{
    m_externalDc = false;
    m_renderDC = m_deviceDC =  0;
    m_clip.Clear();
}

a2dDcDrawer::~a2dDcDrawer()
{}

a2dDcDrawer::a2dDcDrawer( const a2dDcDrawer& other )
    : a2dDrawer2D( other )
{
    m_externalDc = false;
    m_renderDC = m_deviceDC =  0;
    m_clip.Clear();
}

a2dDcDrawer::a2dDcDrawer( const a2dDrawer2D& other )
    : a2dDrawer2D( other )
{
    m_externalDc = false;
    m_renderDC = m_deviceDC =  0;
    m_clip.Clear();
}

void a2dDcDrawer::BeginDraw()
{
    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
        if ( m_externalDc )
        {
            wxASSERT_MSG( m_renderDC != NULL, wxT( "a2dMemDcDrawer,  wxClientDc not set yet" ) );
        }
        else
        {
            wxASSERT_MSG( m_renderDC == NULL, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );

            if ( m_display )
            {
                m_deviceDC = new wxClientDC( m_display );
                m_display->PrepareDC( *m_deviceDC );
            }
            m_renderDC = m_deviceDC;
        }

        DestroyClippingRegion();
        SetDrawerFill( m_currentfill );
        SetDrawerStroke( m_currentstroke );
        SetDrawStyle( m_drawstyle );
        m_OpacityFactor = 255;
        m_renderDC->SetMapMode( wxMM_TEXT );
    }
    m_beginDraw_endDraw++;
}

void a2dDcDrawer::EndDraw()
{
    m_beginDraw_endDraw--;

    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
        wxASSERT_MSG( m_renderDC != NULL, wxT( "a2dDcDrawer, unbalanced BeginDraw EndDraw" ) );
        if ( m_externalDc )
        {
            wxASSERT_MSG( true == m_externalDc, wxT( "a2dDcDrawer,  wxClientDc not set yet" ) );
        }
        else
        {
            if ( m_display )
                delete m_deviceDC;
        }

        m_renderDC = m_deviceDC = NULL;
    }

}

void a2dDcDrawer::SetBufferSize( int w, int h )
{
    m_width = w;
    m_height  = h;
}

wxBitmap a2dDcDrawer::GetSubBitmap( wxRect rect ) const
{
//klion
#ifdef __WXGTK__
    wxFAIL_MSG( wxT( "a2dDcDrawer has no buffer" ) );
#endif
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

#ifdef __WXGTK__
    wxBitmap ret( rect.width, rect.height, 32 );
#else
    wxBitmap ret( rect.width, rect.height, m_renderDC->GetDepth() );
#endif
    wxASSERT_MSG( ret.Ok(), wxT( "GetSubImage error" ) );

    if ( m_renderDC )
    {
        //do NOT use getsubbitmap, renderDc is already set for the buffer therefor will not work properly
        wxMemoryDC dcb;
        dcb.SelectObject( ret );
        dcb.Blit( 0, 0, rect.width, rect.height, m_renderDC, rect.x, rect.y, wxCOPY, false );
        dcb.SelectObject( wxNullBitmap );
    }
    return ret;
}

void a2dDcDrawer::SetRenderDC( wxDC* dc )
{
    if ( dc == NULL )
    {
        m_externalDc = false;
        m_renderDC = m_deviceDC = NULL;
    }
    else
    {
        m_externalDc = true;
        m_renderDC = dc;
    }
}

void a2dDcDrawer::SetClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle )
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
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

    wxCoord originX, originY;
    m_renderDC->GetDeviceOrigin( &originX, &originY );
    m_clip.Offset( originX, originY );
    m_renderDC->DestroyClippingRegion();
#if wxCHECK_VERSION(2,9,0)
    m_renderDC->SetDeviceClippingRegion( m_clip );
#else
    m_renderDC->SetClippingRegion( m_clip );
#endif
    m_clipboxdev = m_clip.GetBox();
    m_clipboxdev.Offset( -originX, -originY );
    m_clipboxworld = ToWorld( m_clipboxdev );
}

void a2dDcDrawer::ExtendAndPushClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode fillStyle, a2dBooleanClip clipoperation )
{

    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    wxRegion* push = new wxRegion( m_clip );

    m_clipregionlist.Insert( push );

    ExtendClippingRegion( points, spline, fillStyle, clipoperation );
}

void a2dDcDrawer::ExtendClippingRegion( a2dVertexList* points, bool spline, wxPolygonFillMode WXUNUSED( fillStyle ), a2dBooleanClip clipoperation )
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

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
    wxCoord originX, originY;
    m_renderDC->GetDeviceOrigin( &originX, &originY );
    add.Offset( originX, originY );
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
            m_renderDC->DestroyClippingRegion();
#if wxCHECK_VERSION(2,9,0)
            m_renderDC->SetDeviceClippingRegion( m_clip );
#else
            m_renderDC->SetClippingRegion( m_clip );
#endif
        }
    }
    else
    {
        m_clip = totaladd;
        m_renderDC->DestroyClippingRegion();
#if wxCHECK_VERSION(2,9,0)
        m_renderDC->SetDeviceClippingRegion( m_clip );
#else
        m_renderDC->SetClippingRegion( m_clip );
#endif
    }

    m_clipboxdev = m_clip.GetBox();
    m_clipboxdev.Offset( -originX, -originY );
    m_clipboxworld = ToWorld( m_clipboxdev );
}

void a2dDcDrawer::PopClippingRegion()
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
    if ( !m_clipregionlist.GetCount() )
        return;

    m_clip = *m_clipregionlist.GetFirst()->GetData();
    delete m_clipregionlist.GetFirst()->GetData();
    m_clipregionlist.DeleteNode( m_clipregionlist.GetFirst() );

    m_renderDC->DestroyClippingRegion();
#if wxCHECK_VERSION(2,9,0)
    m_renderDC->SetDeviceClippingRegion( m_clip );
#else
    m_renderDC->SetClippingRegion( m_clip );
#endif
    m_clipboxdev = m_clip.GetBox();
    wxCoord originX, originY;
    m_renderDC->GetDeviceOrigin( &originX, &originY );
    m_clipboxdev.Offset( -originX, -originY );
    m_clipboxworld = ToWorld( m_clipboxdev );

}

void a2dDcDrawer::SetClippingRegionDev( wxCoord minx, wxCoord miny, wxCoord width, wxCoord height )
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
    m_clip = wxRegion( minx, miny, width, height );
    wxCoord originX, originY;
    m_renderDC->GetDeviceOrigin( &originX, &originY );
    m_clip.Offset( originX, originY );
    m_renderDC->DestroyClippingRegion();
#if wxCHECK_VERSION(2,9,0)
    m_renderDC->SetDeviceClippingRegion( m_clip );
#else
    m_renderDC->SetClippingRegion( m_clip );
#endif

    m_clipboxdev = wxRect( minx, miny, width, height );
    m_clipboxworld = ToWorld( m_clipboxdev );
}

void a2dDcDrawer::SetClippingRegion( double minx, double miny, double maxx, double maxy )
{
    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
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

    m_clipboxworld = ToWorld( m_clipboxdev );

    m_renderDC->DestroyClippingRegion();
    wxCoord originX, originY;
    m_renderDC->GetDeviceOrigin( &originX, &originY );
    m_clip.Offset( originX, originY );
#if wxCHECK_VERSION(2,9,0)
    m_renderDC->SetDeviceClippingRegion( m_clip );
#else
    m_renderDC->SetClippingRegion( m_clip );
#endif
}

void a2dDcDrawer::DestroyClippingRegion()
{
    m_clip.Clear();
    if ( m_renderDC )
        m_renderDC->DestroyClippingRegion();

    m_clipboxdev = wxRect( 0, 0, m_width, m_height );
    m_clipboxworld = ToWorld( m_clipboxdev );
}

void a2dDcDrawer::ResetStyle()
{
    a2dDrawer2D::ResetStyle();
    SetDrawerFill( *a2dBLACK_FILL ); //set to a sure state
    SetDrawerStroke( *a2dBLACK_STROKE );
    SetDrawStyle( m_drawstyle );
}

void a2dDcDrawer::DoSetActiveStroke()
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
                    int widthDev = ( int ) WorldToDeviceXRel( m_activestroke.GetWidth() );
                    dcpen.SetWidth( widthDev );
                }
                m_renderDC->SetPen( dcpen );
                m_renderDC->SetTextForeground( m_activestroke.GetColour() );
            }

        }
        else if ( m_activestroke.GetType() == a2dSTROKE_BITMAP )
        {
            wxPen dcpen;

            if ( m_drawstyle == a2dWIREFRAME_ZERO_WIDTH )
                dcpen = wxPen( m_activestroke.GetColour(), 0, wxPENSTYLE_SOLID );
            else
            {
                int width = ( int ) m_activestroke.GetWidth();
                if ( !m_activestroke.GetPixelStroke() )
                    width = ( int )WorldToDeviceXRel( width );
                dcpen = wxPen( m_activestroke.GetColour(), width, wxPENSTYLE_SOLID );
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

            m_renderDC->SetPen( dcpen );
            m_renderDC->SetTextBackground( *wxBLACK );
            m_renderDC->SetTextForeground( m_activestroke.GetColour() );
        }

    }
}

void a2dDcDrawer::DoSetActiveFill()
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

void a2dDcDrawer::DoSetDrawStyle( a2dDrawStyle drawstyle )
{
    m_drawstyle = drawstyle;

    switch( drawstyle )
    {
        case a2dWIREFRAME_INVERT:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
            SetLogicalFunction( wxINVERT );
            break;

        case a2dWIREFRAME:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
            SetLogicalFunction( wxCOPY );
            break;

        case a2dWIREFRAME_ZERO_WIDTH:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
            SetLogicalFunction( wxCOPY );
            break;

        case a2dWIREFRAME_INVERT_ZERO_WIDTH:
            SetActiveStroke( *a2dBLACK_STROKE );
            SetActiveFill( *a2dTRANSPARENT_FILL );
            SetLogicalFunction( wxINVERT );
            break;

        case a2dFILLED:
            SetActiveStroke( m_currentstroke );
            SetActiveFill( m_currentfill );
            SetLogicalFunction( wxCOPY );
            break;

        case a2dFIX_STYLE:
            //preserve this
            m_fixStrokeRestore = m_currentstroke;
            m_fixFillRestore = m_currentfill;
            m_fixDrawstyle = m_drawstyle;
            SetActiveStroke( m_currentstroke );
            SetActiveFill( m_currentfill );

            // don't adjust style
            SetLogicalFunction( wxCOPY );
            break;

        case a2dFIX_STYLE_INVERT:
            // don't adjust style
            SetLogicalFunction( wxINVERT );
            break;

        default:
            wxASSERT( 0 );
    }
}


void a2dDcDrawer::DrawImage( const wxImage& imagein, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    wxPoint centr( ( int ) width / 2, ( int )  height / 2 ); //this doesn't make any difference (BUG?)
    //anyway the rotation center is the middle of the image

    double angle = GetTransform().GetRotation();

    wxImage image = imagein;

    if ( fabs( angle ) > 0.5 /*degree*/ )
    {
        image.SetMask( true );
        if ( m_yaxis )
            image = imagein.Rotate( angle / 180.0 * wxPI, centr, true, NULL );
        else
            image = imagein.Rotate( -angle / 180.0 * wxPI, centr, true, NULL );
    }

    int devicew, deviceh;
    devicew = WorldToDeviceXRel( GetTransform().Get_scaleX() * width );
    deviceh = abs( WorldToDeviceYRel( GetTransform().Get_scaleY() * height ) );

    double hx, hy;
    GetUserToDeviceTransform().TransformPoint( x, y, hx, hy );
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
    int deviceClipX =  clipped.GetX();
    int deviceClipY =  clipped.GetY();
    int deviceClipW =  clipped.GetWidth();
    int deviceClipH =  clipped.GetHeight();
    // translate/rescale this to image coordinates
    clipped.SetX( ( int ) ( ( clipped.GetX() - devicex ) * sx ) );
    clipped.SetY( ( int ) ( ( clipped.GetY() - devicey ) * sy ) );
    clipped.SetWidth( ( int ) ( clipped.GetWidth() * sx ) );
    clipped.SetHeight( ( int ) ( clipped.GetHeight() * sy ) );

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
    //is there a mirror in the matrix?
    if ( GetUserToDeviceTransform().GetValue(1,1) < 0 )
        image = image.Mirror( false );//true );
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
    /*
        image.Rescale( devicew, deviceh );

        // we can not draw images, so first make it a bitmap, slow !
        wxBitmap Bitmap( image );

        if ( m_drawstyle == a2dFILLED )
        {
            wxMemoryDC MemoryDc;
            MemoryDc.SelectObject( Bitmap );
            // Copy the pre-stretched bitmap to the display device.
            m_renderDC->Blit( devicex, devicey, devicew, deviceh, &MemoryDc, 0, 0, wxCOPY, imgHasMask );
        }
        else
        {
            m_renderDC->DrawRectangle( devicex, devicey, devicew, deviceh );
        }
    */
}

void a2dDcDrawer::SetLogicalFunction( wxRasterOperationMode function )
{
#if __WXGTK__
    if ( m_renderDC &&  !wxDynamicCast( m_renderDC, wxPostScriptDC ) )
        m_renderDC->SetLogicalFunction( function );
#else
    if ( m_renderDC )
        m_renderDC->SetLogicalFunction( function );
#endif
}

/*
void a2dDcDrawer::DrawImage( const wxImage& imagein, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    wxASSERT_MSG( m_renderDC, wxT("no renderDc set") );

    wxPoint centr( (int) width/2, (int)  height/2); //this doesn't make any difference (BUG?)
    //anyway the rotation center is the middle of the image

    double angle = m_usertoworld.GetRotation();

    wxImage image = imagein;

    if ( angle != 0)
    {
        if ( m_yaxis )
            image = imagein.Rotate( angle/180.0 * wxPI, centr, true, NULL );
        else
            image = imagein.Rotate(-angle/180.0 * wxPI, centr, true, NULL );
    }

    int orgdevicew,devicew,orgdeviceh,deviceh;
    int imagew = image.GetWidth();
    int imageh = image.GetHeight();
    orgdevicew = devicew = WorldToDeviceXRel( m_usertoworld.Get_scaleX()*width );
    orgdeviceh = deviceh = abs(WorldToDeviceYRel( m_usertoworld.Get_scaleY()*height ));

    double hx,hy;
    GetUserToDeviceTransform().TransformPoint( x, y, hx, hy );
    int devicex = (int) (hx - devicew/2.0);  //devicex of projected (imaginairy) image
    int devicey = (int) (hy - deviceh/2.0);  //devicey of projected (imaginairy) image

    int clipx;
    int clipy;
    int clipwidth;
    int clipheight;
    m_clip.GetBox(clipx, clipy, clipwidth, clipheight);

    int mindx=0; //start in projected (imaginairy) image
    int mindy=0; //start in projected (imaginiairy) image

    // clip to clipping area
    if (devicex < clipx)
    {
        mindx = clipx-devicex;
        devicew -= clipx-devicex;
    }
    if (devicew <= 0) return;

    if (devicey < clipy)
    {
        mindy = clipy-devicey;
        deviceh -= clipy-devicey;
    }
    if (deviceh <= 0) return;

    if (devicex+orgdevicew >= clipx+clipwidth)
    {
        devicew -= devicex + orgdevicew - (clipx + clipwidth);
    }
    if (devicew <= 0) return;

    if (devicey+orgdeviceh >= clipy+clipheight)
    {
        deviceh -= devicey + orgdeviceh - (clipy + clipheight);
    }
    if (deviceh <= 0) return;

    //now the projected image to be drawn at the given width and height in world coordinates,
    //is clipped to the current clip rectangle.
    //Depending on the projection size and the actual size of the image,
    //we decide to draw ALL pixels in the projected part of the image,
    //or we draw the pixels from the original image as rectangles.


    int pixelwidth = (int) (orgdevicew/(double)imagew );

    // for drawing transparency
    bool imgHasMask = image.HasMask();

    //i don't know what the optimum is but at least 2 or higher
    if ( pixelwidth > 50000)
    {
        unsigned char rMask = ' ';
        unsigned char gMask = ' ';
        unsigned char bMask = ' ';

        // if image has a mask, we ask for the rgb value of mask
        if (imgHasMask)
        {
            rMask = image.GetMaskRed();
            gMask = image.GetMaskGreen();
            bMask = image.GetMaskBlue();
        }

        wxBrush pixelbrush;
        m_renderDC->SetPen( *wxTRANSPARENT_PEN );
        // We do (x, y) -> (x, y)*oldSize/newSize
        for ( long j = mindy; j < mindy+deviceh; j+=pixelwidth )
        {
            long yimage = (j * imageh) / orgdeviceh;

            for ( long i = mindx; i < mindx+devicew; i+=pixelwidth )
            {
                long ximage = (i * imagew) / orgdevicew;

                unsigned char r = image.GetRed(ximage,yimage);
                unsigned char g = image.GetGreen(ximage,yimage);
                unsigned char b = image.GetBlue(ximage,yimage);

                // Drawing is only necessary if we don't draw transparent
                if (!(imgHasMask && r == rMask && g == gMask && b == bMask) )
                {
                    wxColour gradcol(r, g, b);
                    pixelbrush.SetColour(gradcol);
                    m_renderDC->SetBrush(pixelbrush);
                    DevDrawRectangle( devicex + i , devicey + j, pixelwidth, pixelwidth);
                }
            }
        }
        //restore since drawer should not change fill or stroke itself
        SetDrawerStroke( m_currentstroke );
        SetDrawerFill( m_activefill );
    }
    else
    {

        wxRect SubImageRect( (mindx * imagew) / orgdevicew,
                             (mindy * imageh) / orgdeviceh,
                             (devicew * imagew) / orgdevicew,
                             (deviceh * imageh) / orgdeviceh
                           );

        if ( SubImageRect.GetHeight() <= 0 || SubImageRect.GetWidth() <= 0 )
            return;

        wxImage SubImage = image.GetSubImage(SubImageRect);
        // Scale it properly.
        SubImage.Rescale( devicew, deviceh);
        // Generate a bitmap and a memory device context and select the bitmap
        // into the memory device context.
        wxBitmap Bitmap( SubImage );

        wxMemoryDC MemoryDc;
        MemoryDc.SelectObject(Bitmap);
        // Copy the pre-stretched bitmap to the display device.
        m_renderDC->Blit( devicex + mindx, devicey + mindy,
                devicew, deviceh,
                &MemoryDc, 0, 0, wxCOPY, imgHasMask);

        // / *
        // We do (x, y) -> (x, y)*oldSize/newSize
        for ( long j = mindy; j < mindy+deviceh; j++ )
        {
            long yimage = (j * imageh) / orgdeviceh;

            for ( long i = mindx; i < mindx+devicew; i++ )
            {
                long ximage = (i * imagew) / orgdevicew;

                unsigned char r = image.GetRed(ximage,yimage);
                unsigned char g = image.GetGreen(ximage,yimage);
                unsigned char b = image.GetBlue(ximage,yimage);

                DeviceDrawPixel( devicex + i , devicey + j, r, g, b);
            }
        }
        ///
    }
}
*/


void a2dDcDrawer::DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    wxPoint centr( ( int ) width / 2, ( int )  height / 2 ); //this doesn't make any difference (BUG?)
    //anyway the rotation center is the middle of the image

    int devicew, deviceh;
    devicew = WorldToDeviceXRel( GetTransform().Get_scaleX() * width );
    deviceh = abs( WorldToDeviceYRel( GetTransform().Get_scaleY() * height ) );

    double hx, hy;
    GetUserToDeviceTransform().TransformPoint( x, y, hx, hy );
    int devicex = ( int ) ( hx - devicew / 2.0 );  //devicex of projected (imaginairy) image
    int devicey = ( int ) ( hy - deviceh / 2.0 );  //devicey of projected (imaginairy) image

    int clipx;
    int clipy;
    int clipwidth;
    int clipheight;
    m_clip.GetBox( clipx, clipy, clipwidth, clipheight );



    if ( devicew == 0 || deviceh == 0 )
        return;
    /*
        // now we first scale to screen projected size, often limits the amount of pixels to draw
        // when size is smaller then original pixel size.
        image.Rescale( devicew, deviceh);

        // we can not draw images, so first make it a bitmap, slow !
        wxBitmap Bitmap( image );

        if ( m_drawstyle == a2dFILLED )
        {
            wxMemoryDC MemoryDc;
            MemoryDc.SelectObject(Bitmap);
            // Copy the pre-stretched bitmap to the display device.
            m_renderDC->Blit( devicex, devicey, devicew, deviceh, &MemoryDc, 0, 0, wxCOPY, imgHasMask);
        }
        else
        {
            m_renderDC->DrawRectangle( devicex, devicey, devicew, deviceh );
        }
    */
}

void a2dDcDrawer::DrawVpath( const a2dVpath* path )
{
    if ( m_disableDrawing )
        return;

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
                a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                    yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
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
                a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                    yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
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
                SetDrawerStroke( *a2dTRANSPARENT_STROKE );
                DeviceDrawPolygon( count, false, wxODDEVEN_RULE );
                SetDrawerStroke( stroke );
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
                    a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg.Get();

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                        yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
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
                    a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg.Get();

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                        yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
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

void a2dDcDrawer::DrawPolyPolygon( a2dListOfa2dVertexList polylist, wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    a2dBoundingBox devbox;
    bool smallPoly = false;
    int j = 0;
    int totalsize = 0;
    std::vector<int> pcount;
    pcount.resize( polylist.size() );

    m_cpointsInt.clear();
    for( a2dListOfa2dVertexList::iterator iterp = polylist.begin(); iterp != polylist.end(); iterp++ )
    {
        int segments = ToDeviceLines( ( *iterp ).Get(), devbox, smallPoly );
        pcount[ j++ ] = segments;
        m_cpointsInt.resize( totalsize + segments );
        // Convert to integer coords by rounding
        for ( int i = 0; i < segments; i++ )
        {
            m_cpointsInt[ totalsize + i ].x = Round( m_cpointsDouble[i].x );
            m_cpointsInt[ totalsize + i ].y = Round( m_cpointsDouble[i].y );
        }
        totalsize += segments;
    }

    if ( m_cpointsInt.size() == 0 )
        return;

    m_renderDC->DrawPolyPolygon( j, &pcount[0], &m_cpointsInt[0], 0, 0, fillStyle );

}

void a2dDcDrawer::DrawCircle( double xc, double yc, double radius )
{
    if ( m_disableDrawing )
        return;

    if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR ||
            m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL ||
            m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
    }
    else if ( m_usertodevice.IsIdentity() )
    {
        wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
        m_renderDC->DrawCircle( Round( xc ), Round( yc ), Round( radius ) );
        return;
    }

    if ( radius == 0 )
    {
        double x, y;
        m_usertodevice.TransformPoint( xc, yc, x, y );
        m_renderDC->DrawPoint( Round( x ), Round( y ) );
        return;
    }

    if ( m_activefill.GetType() != a2dFILL_GRADIENT_FILL_LINEAR &&
            m_activefill.GetType() != a2dFILL_GRADIENT_FILL_RADIAL &&
            !( m_activefill.GetType() == a2dFILL_BITMAP && m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
       )
    {
        /* draw native (hardware accelerated) circle/ellipse when possible.
            Special cases, where native circles may apply:
            only rotated if: m(0,0) == m(1,1) && m(1,0) == m(0,1)
            stretched and not rotated: m(0,1) == 0.0 && m(1,0) == 0.0;
            stretched and 90deg rotated: m(0,0) == 0.0 && m(1,1) == 0.0;
        */
        double m00 = fabs( m_usertodevice.GetValue( 0, 0 ) );
        double m11 = fabs( m_usertodevice.GetValue( 1, 1 ) );
        double m10 = fabs( m_usertodevice.GetValue( 1, 0 ) );
        double m01 = fabs( m_usertodevice.GetValue( 0, 1 ) );

        /*  the use of == in the next statement is valid, even though it is a comparision between doubles.
            This is because when no shearing/stretching has taken place, the values have had exactly
            the same values and multiplications. Therefore these are exactly equal.
        */
        if ( m00 == m11 && m10 == m01 )
        {
            // only rotated
            double x, y;
            m_usertodevice.TransformPoint( xc, yc, x, y );
#if defined(__WXMSW__)
            m_renderDC->DrawCircle( Round( x ), Round( y ), Round( _hypot( m00, m01 ) * radius ) );
#else
            m_renderDC->DrawCircle( Round( x ), Round( y ), Round( hypot( m00, m01 ) * radius ) );
#endif
        }
        else if ( ( 100.0 * m10 < m00 ) && ( 100.0 * m01 < m11 ) )
        {
            // since checking for == is dangerous with doubles, a check for small angles is used instead
            // 100 * m10 < m00 actually means: rotation angle < 0.5 degrees

            // stretched but not rotated
            double x, y, rx, ry;
            m_usertodevice.TransformPoint( xc, yc, x, y );
            rx = m00 * radius;
            ry = m11 * radius;
            m_renderDC->DrawEllipse( Round( x - rx ), Round( y - ry ), Round( 2.0 * rx ), Round( 2.0 * ry ) );
        }
        else if ( ( 100.0 * m00 < m10 ) && ( 100.0 * m11 < m01 ) )
        {
            // stretched and 90 degrees rotated
            double x, y, rx, ry;
            m_usertodevice.TransformPoint( xc, yc, x, y );
            rx = m10 * radius;
            ry = m01 * radius;
            m_renderDC->DrawEllipse( Round( x - rx ), Round( y - ry ), Round( 2.0 * rx ), Round( 2.0 * ry ) );
        }
        else
        {
            // use a polygon ellipse.
            DrawEllipse( xc, yc, 2.0 * radius, 2.0 * radius );
        }
    }
    else
    {
        // use a polygon ellipse.
        a2dDrawer2D::DrawEllipse( xc, yc, 2.0 * radius, 2.0 * radius );
    }
}

void a2dDcDrawer::DrawEllipse( double xc, double yc, double width, double height )
{
    if ( m_disableDrawing )
        return;

    if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR ||
            m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL ||
            m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        a2dDrawer2D::DrawEllipse( xc, yc, width, height );
    }
    else if ( m_usertodevice.IsIdentity() )
    {
        wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
        m_renderDC->DrawEllipse( Round( xc - width / 2.0 ), Round( yc - height / 2.0 ), Round( width ), Round( height ) );
    }
    else
        a2dDrawer2D::DrawEllipse( xc, yc, width, height );
}

void a2dDcDrawer::DrawLine( double x1, double y1, double x2, double y2 )
{
    if ( m_disableDrawing )
        return;

    if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR ||
            m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL ||
            m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        a2dDrawer2D::DrawLine( x1, y1, x2, y2 );
    }
    else if ( m_usertodevice.IsIdentity() )
    {
        wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
        m_renderDC->DrawLine( Round( x1 ), Round( y1 ), Round( x2 ), Round( y2 ) );
    }
    else
        a2dDrawer2D::DrawLine( x1, y1, x2, y2 );
}


void a2dDcDrawer::DrawRoundedRectangle( double x, double y, double width, double height, double radius, bool pixelsize )
{
    if ( m_disableDrawing )
        return;

    if ( m_activefill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR ||
            m_activefill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL ||
            m_activefill.GetStyle() == a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT )
    {
        a2dDrawer2D::DrawRoundedRectangle( x, y, width, height, radius, pixelsize );
    }
    else if ( m_usertodevice.IsIdentity() )
    {
        if ( pixelsize )
        {
            width = DeviceToWorldXRel( width );
            height = DeviceToWorldYRel( height );
            radius = DeviceToWorldXRel( radius );
            if( m_yaxis )
                height = -height;
        }

        wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
        m_renderDC->DrawRoundedRectangle( Round( x ), Round( y ), Round( width ), Round( height ), Round( radius ) );
    }
    else
        a2dDrawer2D::DrawRoundedRectangle( x, y, width, height, radius, pixelsize );
}

void a2dDcDrawer::DrawPoint( double xc, double yc )
{
    if ( m_disableDrawing )
        return;

    double xt, yt;
    GetUserToDeviceTransform().TransformPoint( xc, yc, xt, yt );
    DeviceDrawPixel( ( int ) xt, ( int ) yt, m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );

    //m_renderDC->DrawPoint( Round( xt ), Round( yt ) );
}

void a2dDcDrawer::DeviceDrawPolygon( unsigned int n, bool spline, wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    if ( spline )
        n = ConvertSplinedPolygon2( n );

    if ( n <= 1 )
      return;

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
            m_renderDC->DrawLines( n, int_cpts );
            m_renderDC->DrawLine( int_cpts[0].x, int_cpts[0].y, int_cpts[n - 1].x, int_cpts[n - 1].y );
/*
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
*/
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

        if ( !wxDynamicCast( m_renderDC, wxPostScriptDC ) )
        {
            wxPen dcpen( m_activestroke.GetColour(), ( int ) m_activestroke.GetWidth(), wxPENSTYLE_SOLID );
            if ( !m_activestroke.GetPixelStroke() )
                dcpen.SetWidth( ( int )WorldToDeviceXRel( m_activestroke.GetWidth() ) );
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
            SetActiveStroke( m_activestroke );
            m_renderDC->DrawPolygon( n, int_cpts, 0, 0, fillStyle );
            m_renderDC->SetLogicalFunction( wxCOPY );
            m_renderDC->SetTextForeground( m_activestroke.GetColour() );
            m_renderDC->SetTextBackground( m_activefill.GetColour() );
        }
        else
        {
            wxPen dcpen( m_activestroke.GetColour(), ( int ) m_activestroke.GetWidth(), wxPENSTYLE_SOLID );
            if ( !m_activestroke.GetPixelStroke() )
                dcpen.SetWidth( ( int )WorldToDeviceXRel( m_activestroke.GetWidth() ) );
            m_renderDC->SetPen( dcpen );

            m_renderDC->DrawPolygon( n, int_cpts, 0, 0, fillStyle );
        }
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

void a2dDcDrawer::DeviceDrawLines( unsigned int n, bool spline )
{
    if ( m_disableDrawing )
        return;

    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
    if ( spline )
        n = ConvertSplinedPolyline2( n );

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

void a2dDcDrawer::DeviceDrawLine( double x1, double y1, double x2, double y2 )
{
    if ( m_disableDrawing )
        return;

    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    m_renderDC->DrawLine( Round( x1 ), Round( y1 ), Round( x2 ), Round( y2 ) );
}

void a2dDcDrawer::DeviceDrawHorizontalLine( int x1, int y1, int x2, bool use_stroke_color )
{
    if ( m_disableDrawing )
        return;

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

void a2dDcDrawer::DeviceDrawVerticalLine( int x1, int y1, int y2, bool use_stroke_color )
{
    if ( m_disableDrawing )
        return;

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

void a2dDcDrawer::DeviceDrawPixel( int x1, int y1, unsigned char r, unsigned char g, unsigned char b , unsigned char a )
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
    m_renderDC->DrawPoint( x1, y1 );
/*    
    wxWindowDCImpl* dcimpl = wxStaticCast( m_renderDC->GetImpl(), wxWindowDCImpl );
    GdkWindow* gtkwin =  dcimpl->GetGDKWindow();
    if ( gtkwin )
        gdk_draw_point( gtkwin, dcimpl->m_penGC, x1, y1 );
*/    
#else
    wxColour col( r, g, b );
    col.CalcPixel( ( ( wxWindowDC* )m_renderDC )->m_cmap );
    gdk_gc_set_foreground( ( ( wxWindowDC* )m_renderDC )->m_penGC, col.GetColor() );
    gdk_draw_point( ( ( wxWindowDC* ) m_renderDC )->m_window, ( ( wxWindowDC* )m_renderDC )->m_penGC, x1, y1 );
#endif

#else

#endif
}

void a2dDcDrawer::DeviceDrawBitmap( const wxBitmap& bmp, double x, double y, bool useMask )
{
    if ( m_disableDrawing )
        return;

    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
    if ( m_drawstyle == a2dWIREFRAME_INVERT )
    {
        m_renderDC->DrawRectangle( Round( x ), Round( y ), bmp.GetWidth(), bmp.GetHeight() );
    }
    else
        m_renderDC->DrawBitmap( bmp, Round( x ), Round( y ), useMask );
}

void a2dDcDrawer::DrawCharDc( wxChar c )
{
#if 0
    if ( m_currentfont.GetType() == a2dFONT_WXDC && m_currentfont.GetFreetypeFont().Ok() )
    {
        a2dFont oldfont = m_currentfont;
        m_currentfont = m_currentfont.GetFreetypeFont();
        DrawCharFreetype( c );
        m_currentfont = oldfont;
    }
    else
        a2dDrawer2D::DrawCharDc( c );
#else
    double x, y;
    x = y = 0.0;
    double dx, dy;
    double angle = GetUserToDeviceTransform().GetRotation();
    GetUserToDeviceTransform().TransformPoint( x, y + m_currentfont.GetLineHeight(), dx, dy );
    //round to integer pixel important when text is drawn small
    m_renderDC->DrawRotatedText( wxString( c ), Round( dx ), Round( dy ), -angle );
#endif
}

void a2dDcDrawer::DrawTextDc( const wxString& text, double x, double y )
{

#if 0
    a2dFont oldfont = m_currentfont;
    m_currentfont = m_currentfont.GetFreetypeFont();
    DrawTextFreetype( text, x, y );
    m_currentfont = oldfont;
#else

    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );
    double dx = GetUserToDeviceTransform().GetValue( 1, 0 );
    double dy = GetUserToDeviceTransform().GetValue( 1, 1 );

    // Get font size in device units
    unsigned int fontsize;
    fontsize = ( unsigned int ) fabs( m_currentfont.GetSize() * sqrt( dx * dx + dy * dy ) );
    if ( fontsize < 1 )
    {
        fontsize = 1;
        m_currentfont.GetFont().SetPointSize( fontsize );
    }

    m_renderDC->SetFont( m_currentfont.GetFont() );
    m_renderDC->SetBackgroundMode( wxTRANSPARENT );
    m_renderDC->SetTextForeground( m_activestroke.GetColour() );

    // When text editing, text extends are different.
    // When possible drawn directly as a string
    // Understand that wxDc text cannot be scaled in x different from y,
    // and therefore scaling will only work correctly if that is taken into account.
    // Meaning matrix should have scaling in both x and y eqaul when using wxDC text.
    if ( m_forceNormalizedFont || ! GetUserToDeviceTransform().IsTranslateScaleIso() )
    {
        // a2dFont can be drawn as freetype if found ttf font file
        //DrawTextGeneric( text, x, y, ( void ( a2dDrawer2D::* )( wxChar ) ) & a2dDcDrawer::DrawCharFreetype );
        DrawTextGeneric( text, x, y, ( void ( a2dDrawer2D::* )( wxChar ) ) & a2dDcDrawer::DrawCharDc );
    }
    else
    {
        // mirror text, depending on y-axis orientation
        const double h = m_currentfont.GetLineHeight();
        //x = y = 0.0;
        double angle = GetUserToDeviceTransform().GetRotation();
        GetUserToDeviceTransform().TransformPoint( x, y + m_currentfont.GetLineHeight(), dx, dy );
        m_renderDC->DrawRotatedText( text, ( int ) dx, ( int ) dy, -angle );
    }

#endif
}

#if wxART2D_USE_FREETYPE
extern FT_Library g_freetypeLibrary;

//! used in freetype rendering
typedef struct
{
    wxColour colour;
    int xmin, ymax, ymin;
    int stride;
    int height;
    int descent;
    unsigned char* buf;
    int sizebuf;
    bool modeUp; 
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
    int scanLine;

    //mirror in Y or not
    if ( user->modeUp )
        scanLine = ( user->height - user->descent - y ) * user->stride + user->xmin * 3;
    else
        scanLine = ( user->ymax - y ) * user->stride - user->xmin * 3;

    buf = user->buf + scanLine;

    do
    {
        buf2 = buf + spans->x * 3;
        len = spans->len;
        int pos;

        alpha = spans->coverage;
        if ( alpha == 255 )
        {
            pos = scanLine + spans->x * 3 + spans->len;
            if ( pos > user->sizebuf || pos < 0 )
                return;

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
            pos = scanLine + spans->x * 3 + spans->len;
            if ( pos > user->sizebuf || pos < 0 )
                return;
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

void a2dDcDrawer::DrawCharFreetype( wxChar c )
{
    double x, y;
    x = y = 0.0;

    if ( m_disableDrawing )
        return;

    // scale font to size
    // mirror text, depending on y-axis orientation
    const double h = m_currentfont.GetLineHeight();

#if wxART2D_USE_FREETYPE
    a2dAffineMatrix affine;
    if ( !GetYaxis() )
    {
        //affine.Translate( 0.0, -h );
        //affine.Mirror( true, false );
    }
    // position text, works but only translation done.
    //affine.Scale( scale );
    //affine.Translate( x, y );

    a2dVpath vpath;
    m_currentfont.GetVpath( c, vpath, affine );

    // save context
    PushTransform( affine );

    // the resulting vector path contains contours and holes.
    // Like character 'O' contains on contour and one hole.
    // Contours are clockwise, while holes or counter clockwise.
    // Using PolyPolygon, we can draw such polygons correctly.
    //DrawVpath( &vpath );

    //Text is drawn filled with the current pen.
    //Brush is used earlier to draw a rectangular background.

    wxColour color = wxColour( m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
    wxBrush dcbrush( color );
    wxBrush cur_dcbrush = m_renderDC->GetBrush();
    m_renderDC->SetBrush( dcbrush );
    wxPen dcpen = *wxTRANSPARENT_PEN;// wxNullPen;//wxPen( color, 0 );
    wxPen cur_dcpen = m_renderDC->GetPen();
    m_renderDC->SetPen( dcpen );

    a2dListOfa2dVertexList onlyPolygons;
    vpath.ConvertToPolygon( onlyPolygons, false );
    DrawPolyPolygon( onlyPolygons );

    m_renderDC->SetBrush( cur_dcbrush );
    m_renderDC->SetPen( cur_dcpen );

    // restore context
    PopTransform();
#else // wxART2D_USE_FREETYPE
    a2dDrawer2D::DrawCharFreetype( c );
#endif // wxART2D_USE_FREETYPE
}


void a2dDcDrawer::DeviceDrawAnnotation( const wxString& text, wxCoord x, wxCoord y, const wxFont& font )
{
    if ( m_disableDrawing )
        return;

    wxASSERT_MSG( m_renderDC, wxT( "no renderDc set" ) );

    m_renderDC->SetFont( font );
    wxCoord w, h;
    m_renderDC->GetTextExtent( text, &w, &h );
    m_renderDC->SetPen( *wxBLACK_PEN );
    m_renderDC->SetBrush( *wxCYAN_BRUSH );
    m_renderDC->DrawRectangle( x - 1, y - 1, w + 2, h + 2 );
    m_renderDC->SetTextBackground( *wxCYAN );
    m_renderDC->SetTextForeground( *wxBLACK );
    m_renderDC->DrawText( text, x, y );
}

void a2dDcDrawer::BlitBuffer( wxRect WXUNUSED( rect ), const wxPoint& WXUNUSED( bufferpos ) )
{
    //wxFAIL_MSG( wxT( "not useful here" ) );
}

void a2dDcDrawer::BlitBuffer( wxDC* WXUNUSED( dc ), wxRect WXUNUSED( rect ), const wxPoint& WXUNUSED( bufferpos ) )
{
    //wxFAIL_MSG( wxT( "not useful here" ) );
}

/*******************************************************************
a2dMemDcDrawer
********************************************************************/

a2dMemDcDrawer::a2dMemDcDrawer( const wxSize& size ): a2dDcDrawer( size )
{
    if  ( m_width == 0 || m_height == 0 )
        m_buffer = wxBitmap( 100, 100 ); //something reasonable.
    else
        m_buffer = wxBitmap( m_width, m_height );
#if defined(__WXMSW__)
    m_hdc1 = ::GetDC( NULL );
    m_hdc2 = ::CreateCompatibleDC( m_hdc1 );
#endif
}

a2dMemDcDrawer::a2dMemDcDrawer( int width, int height ): a2dDcDrawer( width, height )
{
    if  ( width == 0 || height == 0 )
        m_buffer = wxBitmap( 100, 100 ); //something reasonable.
    else
        m_buffer = wxBitmap( width, height );
#if defined(__WXMSW__)
    m_hdc1 = ::GetDC( NULL );
    m_hdc2 = ::CreateCompatibleDC( m_hdc1 );
#endif
}

a2dMemDcDrawer::~a2dMemDcDrawer()
{
#if defined(__WXMSW__)
    ::DeleteDC( m_hdc1 );
    ::ReleaseDC( NULL, m_hdc2 );
#endif // defined( __WXMSW__ )
}

a2dMemDcDrawer::a2dMemDcDrawer( const a2dMemDcDrawer& other )
    : a2dDcDrawer( other )
{
    m_buffer = other.m_buffer;
#if defined(__WXMSW__)
    m_hdc1 = ::GetDC( NULL );
    m_hdc2 = ::CreateCompatibleDC( m_hdc1 );
#endif
}

void a2dMemDcDrawer::BeginDraw()
{
    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
        if ( m_externalDc )
        {
            wxASSERT_MSG( m_renderDC != NULL, wxT( "a2dMemDcDrawer,  wxClientDc not set yet" ) );
        }
        else
        {
            wxASSERT_MSG( m_renderDC == NULL, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );
            m_memdc = new wxMemoryDC();
            m_memdc->SelectObject( m_buffer );
            m_renderDC = m_memdc;
        }

        SetDrawerFill( m_currentfill );
        SetDrawerStroke( m_currentstroke );

        DestroyClippingRegion();

        SetDrawStyle( m_drawstyle );

        m_renderDC->SetMapMode( wxMM_TEXT );

        if ( m_display )
        {
            m_deviceDC = new wxClientDC( m_display );
            m_display->PrepareDC( *m_deviceDC );
        }
    }

    m_beginDraw_endDraw++;
}

void a2dMemDcDrawer::EndDraw()
{
    m_beginDraw_endDraw--;

    wxASSERT_MSG( m_renderDC != NULL, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );
    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
        wxASSERT_MSG( m_renderDC != NULL, wxT( "a2dDcDrawer, unbalanced BeginDraw EndDraw" ) );
        if ( m_externalDc )
        {
            wxASSERT_MSG( m_renderDC != NULL, wxT( "a2dDcDrawer,  wxClientDc not set yet" ) );
        }
        else
        {
            m_memdc->SelectObject( wxNullBitmap );
            delete m_memdc;
        }
        if ( m_display )
            delete m_deviceDC;
        m_renderDC = m_deviceDC = NULL;
    }
}

void a2dMemDcDrawer::SetBufferSize( int w, int h )
{
    m_width = w;
    m_height  = h;
    wxBitmap helpbuf  = wxBitmap( w, h );

    //make sure to preserve what was in the old buffer bitmap,
    //to prevent the need for total redraws
    m_mdc1.SelectObject( m_buffer );
    m_mdc2.SelectObject( helpbuf );
    m_mdc2.Blit( 0, 0, w, h, &m_mdc1, 0, 0 );
    m_mdc1.SelectObject( wxNullBitmap );
    m_mdc2.SelectObject( wxNullBitmap );

    m_buffer = helpbuf;
}

void a2dMemDcDrawer::DrawPolygon( const a2dVertexList* list, bool spline, wxPolygonFillMode fillStyle )
{
    if ( m_disableDrawing )
        return;

    a2dDrawer2D::DrawPolygon( list, spline, fillStyle );
}

void a2dMemDcDrawer::DeviceDrawPolygon( unsigned int n, bool spline, wxPolygonFillMode fillStyle )
{
    a2dDcDrawer::DeviceDrawPolygon( n, spline, fillStyle );
}

wxBitmap a2dMemDcDrawer::GetSubBitmap( wxRect rect ) const
{
    wxBitmap ret( rect.width, rect.height, m_buffer.GetDepth() );
    wxASSERT_MSG( ret.Ok(), wxT( "GetSubImage error" ) );

    if ( m_renderDC )
    {
        //do NOT use getsubbitmap, renderDc is already set for the buffer therefor will not work properly
        m_mdc1.SelectObject( ret );
        m_mdc1.Blit( 0, 0, rect.width, rect.height, m_renderDC, rect.x, rect.y, wxCOPY, false );
        m_mdc1.SelectObject( wxNullBitmap );
    }
    else
    {
        m_mdc2.SelectObject( const_cast<wxBitmap&>( m_buffer ) );

        //do NOT use getsubbitmap, renderDc is already set for the buffer therefore will not work properly
        m_mdc1.SelectObject( ret );
        m_mdc1.Blit( 0, 0, rect.width, rect.height, &m_mdc2, rect.x, rect.y, wxCOPY, false );
        m_mdc1.SelectObject( wxNullBitmap );

        m_mdc2.SelectObject( wxNullBitmap );
    }

    return ret;
}

void a2dMemDcDrawer::CopyIntoBuffer( const wxBitmap& bitm )
{
    m_buffer = bitm;
}

void a2dMemDcDrawer::BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos )
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

    if ( !m_renderDC )
    {
        m_mdc1.SelectObject( m_buffer );

        dc->Blit( rect.x - bufferpos.x, rect.y - bufferpos.y, rect.width, rect.height, &m_mdc1, rect.x, rect.y, wxCOPY, false );

        m_mdc1.SelectObject( wxNullBitmap );
    }
    else
    {
        dc->Blit( rect.x - bufferpos.x, rect.y - bufferpos.y, rect.width, rect.height, m_renderDC, rect.x, rect.y, wxCOPY, false );
    }

}

void a2dMemDcDrawer::BlitBuffer( wxRect rect, const wxPoint& bufferpos )
{
    BeginDraw();
    BlitBuffer( m_deviceDC, rect, bufferpos );
    EndDraw();
}

void a2dMemDcDrawer::ShiftBuffer( int dxy, bool yshift )
{
    int bw = GetBuffer().GetWidth();
    int bh = GetBuffer().GetHeight();
    if ( yshift )
    {
        if ( dxy > 0 && dxy < bh )
        {
            wxRect rect( 0, 0, bw, bh - dxy );
            wxBitmap sub_bitmap( m_buffer.GetSubBitmap( rect ) );
            m_mdc1.SelectObject( m_buffer );
            m_mdc1.DrawBitmap( sub_bitmap, 0, dxy, true );
            m_mdc1.SelectObject( wxNullBitmap );
        }
        else  if ( dxy < 0 && dxy > -bh )
        {
            wxRect rect( 0, -dxy, bw, bh + dxy );
            wxBitmap sub_bitmap( m_buffer.GetSubBitmap( rect ) );
            m_mdc1.SelectObject( m_buffer );
            m_mdc1.DrawBitmap( sub_bitmap, 0, 0, true );
            m_mdc1.SelectObject( wxNullBitmap );
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
            m_mdc1.SelectObject( m_buffer );
            m_mdc1.DrawBitmap( sub_bitmap, dxy, 0, true );
            m_mdc1.SelectObject( wxNullBitmap );
        }
        else if ( dxy < 0 && dxy > -bw )
        {
            wxRect rect( -dxy, 0, bw + dxy, bh );
            wxBitmap sub_bitmap( m_buffer.GetSubBitmap( rect ) );
            m_mdc1.SelectObject( m_buffer );
            m_mdc1.DrawBitmap( sub_bitmap, 0, 0, true );
            m_mdc1.SelectObject( wxNullBitmap );
        }
        else
            wxFAIL_MSG( wxT( "you can only shift within width of buffer" ) );
    }
}

void a2dMemDcDrawer::DrawImage(  const wxImage& image, double x, double y, double width, double height, wxUint8 Opacity )
{
#if wxART2D_USE_AGGDRAWER
    if ( m_disableDrawing )
        return;

    if ( width == 0 || height == 0 )
        return;

    if ( m_drawstyle == a2dFILLED )
    {
        double angle = GetTransform().GetRotation();
        bool alwaysAggImages = false;
#if defined(__WXGTK__)
        alwaysAggImages = image.HasAlpha();
#endif
        //if ( fabs( fmod(angle,90) ) > 0.5  )
        if ( alwaysAggImages || fabs( angle ) > 0.5 || Opacity != 255 )
        {
            a2dAggDrawer* aggDrawer;
            //stop current drawer, to release its m_memdc on m_buffer
			if ( !m_externalDc )
			{
                aggDrawer = new a2dAggDrawer( m_buffer );
			}
            else
			{
                wxMemoryDC* memprint = wxDynamicCast( m_renderDC, wxMemoryDC );
                aggDrawer = new a2dAggDrawer( memprint->GetSelectedBitmap() );
            }
            
            aggDrawer->SetYaxis( m_yaxis );
            aggDrawer->SetTransform( GetTransform() );
            aggDrawer->SetMappingMatrix( GetMappingMatrix() );
            aggDrawer->SetDrawerFill( m_activefill );
            aggDrawer->SetDrawerStroke( m_activestroke );
            aggDrawer->SetDrawStyle( m_drawstyle );
            aggDrawer->BeginDraw();
            aggDrawer->SetClippingRegionDev( m_clipboxdev.x, m_clipboxdev.y, m_clipboxdev.x + m_clipboxdev.width, m_clipboxdev.y + m_clipboxdev.height );
            aggDrawer->DrawImage( image, x , y, width, height, Opacity );
            aggDrawer->EndDraw();

            wxMemoryDC MemoryDc;
            wxBitmap bufBitmap = aggDrawer->GetBuffer();
            MemoryDc.SelectObject( bufBitmap );
            m_renderDC->Blit( m_clipboxdev.x, m_clipboxdev.y, m_clipboxdev.x + m_clipboxdev.width, m_clipboxdev.y + m_clipboxdev.height, &MemoryDc,  m_clipboxdev.x, m_clipboxdev.y, wxCOPY, TRUE );
            delete aggDrawer;
        }
        else
            a2dDcDrawer::DrawImage( image, x, y, width, height, Opacity );
    }
    else
    {
        DrawCenterRoundedRectangle( x, y, width, height, 0 );
    }
#else
    a2dDcDrawer::DrawImage( image, x, y, width, height, Opacity );
#endif
}

void a2dMemDcDrawer::DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity )
{
#if wxART2D_USE_AGGDRAWER
    if ( m_disableDrawing )
        return;

    if ( width == 0 || height == 0 )
        return;

    if ( m_drawstyle == a2dFILLED )
    {
        double angle = GetTransform().GetRotation();

        //if ( fabs( fmod(angle,90) ) > 0.5  )
        if ( 1 ) //fabs(angle) > 0.5 || Opacity != 255 )
        {
            //stop current drawer, to release its m_memdc on m_buffer
			if ( !m_externalDc )
			{
				m_memdc->SelectObject( wxNullBitmap );
			}
            //EndDraw();

            a2dAggDrawer aggDrawer = a2dAggDrawer( m_buffer );
            aggDrawer.SetYaxis( m_yaxis );
            aggDrawer.SetTransform( GetTransform() );
            aggDrawer.SetMappingMatrix( GetMappingMatrix() );
            aggDrawer.SetDrawerFill( m_activefill );
            aggDrawer.SetDrawerStroke( m_activestroke );
            aggDrawer.SetDrawStyle( m_drawstyle );
            aggDrawer.BeginDraw();
            aggDrawer.SetClippingRegionDev( m_clipboxdev.x, m_clipboxdev.y, m_clipboxdev.x + m_clipboxdev.width, m_clipboxdev.y + m_clipboxdev.height );
            aggDrawer.DrawImage( image, x , y, width, height );
            aggDrawer.EndDraw();

            m_buffer = aggDrawer.GetBuffer();

            //continue current drawer
			if ( !m_externalDc )
			{
				m_memdc->SelectObject( m_buffer );
			}
            //BeginDraw();
        }
        else
            a2dDcDrawer::DrawImage( image, x, y, width, height, Opacity );
    }
    else
    {
        DrawRoundedRectangle( x, y, width, height, 0 );
    }
#else
    a2dDcDrawer::DrawImage( image, x, y, width, height, Opacity );
#endif
}

#define BBOX_GLYPH_EXTEND 1

//#undef __WXMSW__

#if wxART2D_USE_FREETYPE

void DrawCharFreetypeHorizontal( wxColour col, const a2dGlyph* a2dglyph, unsigned char* buf, int bufwidth, int bufheight, wxChar c, int dx, int deviceDescent, FT_BBox bbox )
{
    // Use next to test on screen a2dDcDrawer vpath drawing for printing.
    //a2dDcDrawer::DrawCharFreetype( c );
    //return;

    FT_Glyph image = a2dglyph->m_glyph;

    // If size == 0, nothing has to be drawn.
    if ( bbox.xMax - bbox.xMin <= 0 || bbox.yMax - bbox.yMin <= 0 )
    {
        return;
    }

    // Render glyph to an image
    if ( image->format == FT_GLYPH_FORMAT_OUTLINE )
    {
        a2dSpanData spandata;
        spandata.colour = col;
        spandata.xmin = dx;
        spandata.height = bufheight;
        spandata.descent = deviceDescent;
        spandata.ymin = bbox.yMin;
        spandata.ymax = bbox.yMax;
        spandata.stride = bufwidth * 3;
        spandata.sizebuf = bufwidth * bufheight * 3;
        spandata.modeUp = true;
#if defined(__WXMSW__)
        // calculate image stride ( = w + padding )
        spandata.stride = ( spandata.stride + sizeof( DWORD ) - 1 ) / sizeof( DWORD ) * sizeof( DWORD );
        spandata.sizebuf = ( spandata.stride + sizeof( DWORD ) - 1 ) / sizeof( DWORD ) * sizeof( DWORD ) * (spandata.height + 1);
#endif // defined( __WXMSW__ )
        spandata.buf = buf;
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
        }
    }
    else if ( image->format == FT_GLYPH_FORMAT_BITMAP )
    {
        wxFAIL_MSG( _( "Non vector bitmap fonts are not supported" ) );
    }
    else
        wxFAIL_MSG( _( "Non vector fonts are not supported" ) );

}
#endif // wxART2D_USE_FREETYPE

void a2dMemDcDrawer::DrawTextFreetype( const wxString& text, double x, double y )
{
    // if text font is normalized (large in pixels when set), or if we have on even scaled and/or rotated text, we go for
    // no device draw of text (no hinting and grid fitting).

    if ( m_forceNormalizedFont || ! GetUserToDeviceTransform().IsTranslateScaleIso() )
    {
        DrawTextGeneric( text, x, y, ( void ( a2dDrawer2D::* )( wxChar ) ) & a2dDrawer2D::DrawCharFreetypeCb );
        return;
    }


#if wxART2D_USE_FREETYPE

    // Draw text at device height using hinting and all, so even small size text looks fine.
    const double h = m_currentfont.GetLineHeight();
    bool textwasvisible = false;

    m_usertodevice = m_worldtodevice * m_usertoworld;

    double hdx, hdy;
    int yBboxMinY;
    GetUserToDeviceTransform().TransformPoint( 0.0, 0.0, hdx, hdy );
    yBboxMinY = Round( hdy );

    // x,y on lower-left point of bbox.
    // Add offset, to put anchor at text start
    double worldDescent = m_currentfont.GetDescent();
    m_usertodevice *= a2dAffineMatrix(  0.0, worldDescent );

    // scale = m_currentfont.GetSize() / deviceh;
    const a2dGlyph* a2dglyphprev = NULL;

    wxColour stroke = GetDrawerStroke().GetColour();
    
    if ( 1 )
    {
        int dx, dy;
        m_usertodevice.TransformPoint( 0.0, 0.0, hdx, hdy );
        dx = Round( hdx ); dy = Round( hdy ); 

        // compensate 0 pixel at low y, and high y 1 pixel, 1 extra here.
        wxUint16 deviceh = ( wxUint16 ) ceil( m_usertodevice.TransformDistance( h ) ) + 1;
        wxUint16 deviceDescent = ( wxUint16 ) ceil( m_usertodevice.TransformDistance( worldDescent ) ) + 1; // one lower

        int dxstartrel = 0;
        int dxrel = 0;

        // first how long in x is the image we need.
#if wxUSE_UNICODE
        const wxStringCharType *cp(text.wx_str());
#else
        const wxChar* cp(text.c_str());
#endif // wxUSE_UNICODE
        wxChar c = 0;
        size_t n = text.Length();
        for ( size_t i = 0; i < n; i++ )
        {
            c = *cp++;
            const a2dGlyph* a2dglyph = m_currentfont.GetGlyphFreetype( c );
            // The glyph is now grid fitted and hinted.
            if( !a2dglyph )
                return;

            if ( i > 0 )
            {
                FT_Face  face = a2dglyph->m_face;
                FT_Vector  kern;
                FT_Get_Kerning( a2dglyph->m_face, a2dglyphprev->m_index, a2dglyph->m_index, FT_KERNING_DEFAULT, &kern );
                dxrel += ( int ) ( kern.x / 64.0 );

                if ( a2dglyphprev->m_rsb - a2dglyph->m_lsb >= 32 )
                    dxrel -= 1;
                else if (  a2dglyphprev->m_rsb - a2dglyph->m_lsb < -32 )
                    dxrel += 1;
            }
            else
            {            
                FT_Glyph glyphimage = a2dglyph->m_glyph;
                FT_BBox  bbox;
                FT_Glyph_Get_CBox( glyphimage, FT_GLYPH_BBOX_PIXELS, &bbox );
                dxstartrel = bbox.xMin;
            }

            a2dglyphprev = a2dglyph;
            FT_Glyph glyphimage = a2dglyph->m_glyph;

            FT_BBox  bbox;
            FT_Glyph_Get_CBox( glyphimage, FT_GLYPH_BBOX_PIXELS, &bbox );

            int advance = ( int ) ( glyphimage->advance.x / 64.0 );

            dxrel += advance;
        }
        int devicew = dxrel - dxstartrel;
        
        wxRect stringBbox( dx, yBboxMinY - deviceh, devicew, deviceh);
        if ( stringBbox.GetWidth() == 0 || stringBbox.GetHeight() == 0 )
            return;

        wxBitmap sub_bitmap = GetSubBitmap( stringBbox );

        unsigned char* buf;

        #if defined(__WXMSW__)
            BITMAPINFO bi;
            bi.bmiHeader.biSize = sizeof( bi.bmiHeader );
            bi.bmiHeader.biWidth = devicew;
            bi.bmiHeader.biHeight = -deviceh;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 24;
            bi.bmiHeader.biCompression = BI_RGB;
            bi.bmiHeader.biSizeImage = 0;
            bi.bmiHeader.biClrUsed = 0;
            bi.bmiHeader.biClrImportant = 0;

            HBITMAP hbitmap = ( HBITMAP ) sub_bitmap.GetHBITMAP();
            buf = ( unsigned char* ) malloc( ( 3 * devicew + sizeof( DWORD ) - 1 ) / sizeof( DWORD ) * sizeof( DWORD ) * (deviceh + 1) ); 
            ::GetDIBits( m_hdc2, hbitmap, 0, deviceh, buf, &bi, DIB_RGB_COLORS );
        #else // defined( __WXMSW__ )
            wxImage strImage( sub_bitmap.ConvertToImage() );
            buf = strImage.GetData();
            //strImage.Clear( 204 );
        #endif // defined( __WXMSW__ )

        dxrel = -dxstartrel;
#if wxUSE_UNICODE
        cp = text.wx_str();
#else
        cp = text.c_str();
#endif // wxUSE_UNICODE
        for ( size_t i = 0; i < n; i++ )
        {
            c = *cp++;
            const a2dGlyph* a2dglyph = m_currentfont.GetGlyphFreetype( c );
            // The glyph is now grid fitted and hinted.
            if( !a2dglyph )
                return;

            if ( i > 0 )
            {
                FT_Face  face = a2dglyph->m_face;
                FT_Vector  kern;
                FT_Get_Kerning( a2dglyph->m_face, a2dglyphprev->m_index, a2dglyph->m_index, FT_KERNING_DEFAULT, &kern );
                dxrel += ( int ) ( kern.x / 64.0 );

                if ( a2dglyphprev->m_rsb - a2dglyph->m_lsb >= 32 )
                    dxrel -= 1;
                else if (  a2dglyphprev->m_rsb - a2dglyph->m_lsb < -32 )
                    dxrel += 1;

            }

            a2dglyphprev = a2dglyph;

            FT_Glyph glyphimage = a2dglyph->m_glyph;

            FT_BBox  bbox;
            FT_Glyph_Get_CBox( glyphimage, FT_GLYPH_BBOX_PIXELS, &bbox );
            int px = dx + dxrel + bbox.xMin;
            int py = dy - bbox.yMax;
            int wbox = bbox.xMax - bbox.xMin;
            int hbox = bbox.yMax - bbox.yMin;

            int advance = ( int ) ( glyphimage->advance.x / 64.0 );

            if ( wbox || hbox )
            {
                wxRect bboxglyph( px, py, wbox, hbox );
                //bboxglyph.Inflate( 1 ); //if not, some redrawpixels missed.

                // check character bounding box in device coordinates
                // To optimize take text height and advance width to check if characters need to be drawn.
                // If not, the rest of the string is outside the clipping rectangle for sure.
                // With just the glyph boundingbox this is not possible, and each character needs to be checked separately
                wxRect bboxd( px, yBboxMinY - deviceh, advance, deviceh );

                // if character is not outside clipping box, draw it.
                if ( m_clipboxdev.Intersects( bboxd ) )
                {
                    if ( m_clipboxdev.Intersects( bboxglyph ) )
                    {
                        DrawCharFreetypeHorizontal( stroke, a2dglyph, buf, devicew, deviceh, c, dxrel, deviceDescent, bbox );
                    }
                    textwasvisible = true;
                }
                else if ( textwasvisible )
                {
                    //wxLogDebug("NO bboxd x=%d, y=%d w=%d h=%d", bboxd.GetLeft(), bboxd.GetTop(), bboxd.GetWidth(), bboxd.GetHeight() );
                    // If characters of a string had been visible, and this character is not
                    // visible, then also the other succeeding characters will also be invisible. i.o.w. we can stop drawing.
                    break;
                }
            }
            dxrel += advance;
        }

        if ( textwasvisible )
        {
        #if defined( __WXMSW__ )
            ::SetDIBits( m_hdc2, hbitmap, 0, deviceh, buf, &bi, DIB_RGB_COLORS );
        #else // defined( __WXMSW__ )
            wxBitmap sub_bitmap( strImage );
        #endif // defined( __WXMSW__ )

            // drawn in this image
            DeviceDrawBitmap( sub_bitmap, dx, yBboxMinY  - deviceh );
        }

#if defined( __WXMSW__ )
        free( buf );
#endif // defined( __WXMSW__ )


/* use this to draw rectangle on top of the rendered glyph, for debugging.
        dxrel = -dxstartrel;
#if wxUSE_UNICODE
        cp = text.wx_str();
#else
        cp = text.c_str();
#endif // wxUSE_UNICODE
        for ( size_t i = 0; i < n; i++ )
        {
            c = *cp++;
            const a2dGlyph* a2dglyph = m_currentfont.GetGlyphFreetype( c );
            // The glyph is now grid fitted and hinted.
            if( !a2dglyph )
                return;

            if ( i > 0 )
            {
                FT_Face  face = a2dglyph->m_face;
                FT_Vector  kern;
                FT_Get_Kerning( a2dglyph->m_face, a2dglyphprev->m_index, a2dglyph->m_index, FT_KERNING_DEFAULT, &kern );
                dxrel += ( int ) ( kern.x / 64.0 );

                if ( a2dglyphprev->m_rsb - a2dglyph->m_lsb >= 32 )
                    dxrel -= 1;
                else if (  a2dglyphprev->m_rsb - a2dglyph->m_lsb < -32 )
                    dxrel += 1;

            }
            a2dglyphprev = a2dglyph;

            FT_Glyph glyphimage = a2dglyph->m_glyph;

            FT_BBox  bbox;
            FT_Glyph_Get_CBox( glyphimage, FT_GLYPH_BBOX_PIXELS, &bbox );
            int px = dx + dxrel + bbox.xMin;
            int py = dy - bbox.yMax;
            int wbox = bbox.xMax - bbox.xMin;
            int hbox = bbox.yMax - bbox.yMin;

            int advance = ( int ) ( glyphimage->advance.x / 64.0 );

            if ( wbox || hbox )
            {
                wxRect bboxglyph( px, py, wbox, hbox );
                //bboxglyph.Inflate( 1 ); //if not, some redrawpixels missed.

                // check character bounding box in device coordinates
                // To optimize take text height and advance width to check if characters need to be drawn.
                // If not, the rest of the string is outside the clipping rectangle for sure.
                // With just the glyph boundingbox this is not possible, and each character needs to be checked separately
                wxRect bboxd( px, yBboxMinY - deviceh, advance, deviceh );

                // if character is not outside clipping box, draw it.
                if ( m_clipboxdev.Intersects( bboxd ) )
                {
                    if ( m_clipboxdev.Intersects( bboxglyph ) )
                    {
                        wxPen pen = m_renderDC->GetPen();
                        wxBrush brush = m_renderDC->GetBrush();
                        wxColour color( 5, 50, 255, 0 );
                        m_renderDC->SetPen( wxPen( color ) );
                        m_renderDC->SetBrush( *wxTRANSPARENT_BRUSH );
                        m_renderDC->DrawRectangle(  px, py, wbox, hbox );
                        m_renderDC->SetPen( pen );
                        m_renderDC->SetBrush( brush );
                    }
                    textwasvisible = true;
                }
                else if ( textwasvisible )
                {
                    //wxLogDebug("NO bboxd x=%d, y=%d w=%d h=%d", bboxd.GetLeft(), bboxd.GetTop(), bboxd.GetWidth(), bboxd.GetHeight() );
                    // If characters of a string had been visible, and this character is not
                    // visible, then so will all succeeding. i.o.w. we can stop drawing.
                    break;
                }
            }
            dxrel += advance;
        }

        wxPen pen = m_renderDC->GetPen();
        wxBrush brush = m_renderDC->GetBrush();
        wxColour color( 5, 50, 255, 0 );
        m_renderDC->SetPen( wxPen( color ) );
        m_renderDC->SetBrush( *wxTRANSPARENT_BRUSH );
        m_renderDC->DrawRectangle( dx, yBboxMinY, dxrel, -deviceh);
        m_renderDC->SetPen( pen );
        m_renderDC->SetBrush( brush );
*/

    }
    //EVENTUALLY get rid of this
    else
    {        
        int dx, dy;
        m_usertodevice.TransformPoint( 0.0, 0.0, hdx, hdy );
        dx = Round( hdx ); dy = Round( hdy ); 

        // compensate 1 pixel at low y, and high y also 1 pixel, 2 extra here.
        wxUint16 deviceh = ( wxUint16 ) ceil( m_usertodevice.TransformDistance( h ) ) + 2;
        wxUint16 deviceDescent = ( wxUint16 ) ceil( m_usertodevice.TransformDistance( worldDescent ) ) + 1; // one lower

        yBboxMinY -= 1; // compensate 1 pixel lower
        dy -= 1;    // compensate 1 pixel lower
        int dxrel = 0;

        wxChar c = 0;
        size_t n = text.Length();
        for ( size_t i = 0; i < n; i++ )
        {
            const wxChar oldc = c;
            c = text[i];
            const a2dGlyph* a2dglyph = m_currentfont.GetGlyphFreetype( c );
            // The glyph is now grid fitted and hinted.
            if( !a2dglyph )
                return;

            if ( i > 0 )
            {
                FT_Face  face = a2dglyph->m_face;
                FT_Vector  kern;
                FT_Get_Kerning( a2dglyph->m_face, a2dglyphprev->m_index, a2dglyph->m_index, FT_KERNING_DEFAULT, &kern );
                dxrel += ( int ) ( kern.x / 64.0 );

                if ( a2dglyphprev->m_rsb - a2dglyph->m_lsb >= 32 )
                    dxrel -= 1;
                else if (  a2dglyphprev->m_rsb - a2dglyph->m_lsb < -32 )
                    dxrel += 1;

            }
            a2dglyphprev = a2dglyph;


            FT_Glyph glyphimage = a2dglyph->m_glyph;

            FT_BBox  bbox;
            FT_Glyph_Get_CBox( glyphimage, FT_GLYPH_BBOX_PIXELS, &bbox );
            int px = dx + dxrel + bbox.xMin;
            int py = dy - bbox.yMax;
            int wbox = bbox.xMax - bbox.xMin;
            int hbox = bbox.yMax - bbox.yMin;

            int advance = ( int ) ( glyphimage->advance.x / 64.0 );

            wxRect bboxglyph( px, py, wbox, hbox );
            bboxglyph.Inflate( 1 ); //if not, some redrawpixels missed.

            // check character bounding box in device coordinates
            // To optimize take take text height and advance width to define is charecters needs to be drawn.
            // If not, the rest of the string is outside the clipping rectangle for sure.
            // With just the glyph boundingbox this is not possible, and each character needs to be checked separately
            wxRect bboxd( px, yBboxMinY - deviceh, advance, deviceh );

            // if character is not outside clipping box, draw it.
            if ( m_clipboxdev.Intersects( bboxd ) )
            {
                if ( m_clipboxdev.Intersects( bboxglyph ) )
                {
                    FT_Vector vec;
                    FT_Matrix trans_matrix;
                    //Scale in x and y is the same, and there is a translation, No rotation.
                    // The size/scale is already loaded in the text glyph.
                    /*
                        double angle = GetUserToDeviceTransform().GetRotation();
                        // set up transform (a rotation here)
                        trans_matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
                        trans_matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
                        trans_matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
                        trans_matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );
                    */

                    trans_matrix.xx = ( FT_Fixed )( 0x10000L );
                    trans_matrix.xy = ( FT_Fixed )( 0 );
                    trans_matrix.yx = ( FT_Fixed )( 0 );
                    trans_matrix.yy = ( FT_Fixed )( 0x10000L );
                    vec.x = 0;
                    vec.y = 0;
                    
                    DrawCharFreetype( stroke, c, dx + dxrel, dy, trans_matrix, vec );
                }
                textwasvisible = true;
            }
            else if ( textwasvisible )
            {
                //wxLogDebug("NO bboxd x=%d, y=%d w=%d h=%d", bboxd.GetLeft(), bboxd.GetTop(), bboxd.GetWidth(), bboxd.GetHeight() );
                // If characters of a string had been visible, and this character is not
                // visible, then so will all succeeding. i.o.w. we can stop drawing.
                break;
            }
            dxrel += advance;
            //wdx += m_currentfont.GetWidth( c );
        }

        /* DEBUG bbox in device coordinates
            wxPen pen = m_renderDC->GetPen();
            wxBrush brush = m_renderDC->GetBrush();
            wxColour color( 245, 150, 255, 0 );
            m_renderDC->SetPen( wxPen( color ) );
            m_renderDC->SetBrush( *wxTRANSPARENT_BRUSH );
            m_renderDC->DrawRectangle( floor( hdx ) , yBboxMinY, dxrel, -deviceh);
            m_renderDC->SetPen( pen );
            m_renderDC->SetBrush( brush );
        */

    }
#else // wxART2D_USE_FREETYPE
    DrawTextGeneric( text, x, y, ( void ( a2dDrawer2D::* )( wxChar ) ) & a2dDrawer2D::DrawCharFreetypeCb );
#endif //wxART2D_USE_FREETYPE

}

void a2dMemDcDrawer::DrawCharFreetype( wxChar c )
{
#if wxART2D_USE_FREETYPE
    // Add offset, to put anchor on lower-left point of bbox.
    double wdx, wdy;
    GetUserToDeviceTransform().TransformPoint( 0.0, m_currentfont.GetDescent(), wdx, wdy );

    int dx = Round( wdx );
    int dy = Round( wdy );

    FT_Matrix trans_matrix;
    FT_Vector vec;
    if ( 1 ) // ! GetUserToDeviceTransform().IsTranslateScaleIso() )
    {
        // Generate affine, to scale character from normalized to real size.
        a2dAffineMatrix glyphPos;
        glyphPos.Scale( m_currentfont.GetSize() / m_currentfont.GetDeviceHeight() );
        a2dAffineMatrix glyphToDevice = GetUserToDeviceTransform() * glyphPos;

        // Transform glyph and scale it to device coordinates. Also correct for sub-pixel drawing.
        trans_matrix.xx = ( FT_Fixed ) ( glyphToDevice.GetValue( 0, 0 ) * 0x10000 );
        trans_matrix.xy = ( FT_Fixed ) ( glyphToDevice.GetValue( 1, 0 ) * 0x10000 );
        trans_matrix.yx = ( FT_Fixed ) ( -glyphToDevice.GetValue( 0, 1 ) * 0x10000 );
        trans_matrix.yy = ( FT_Fixed ) ( -glyphToDevice.GetValue( 1, 1 ) * 0x10000 );
        vec.x = ( int ) ( ( wdx - dx ) * 64.0 );
        vec.y = ( int ) ( ( wdy - dy ) * -64.0 );
    }
    else
    {
        trans_matrix.xx = ( FT_Fixed )( 0x10000L );
        trans_matrix.xy = ( FT_Fixed )( 0 );
        trans_matrix.yx = ( FT_Fixed )( 0 );
        trans_matrix.yy = ( FT_Fixed )( 0x10000L );
        vec.x = 0;
        vec.y = 0;
    }

    wxColour stroke = GetDrawerStroke().GetColour();
    DrawCharFreetype( stroke, c, dx, dy, trans_matrix, vec );
#else // wxART2D_USE_FREETYPE
    DrawCharUnknown( c );
#endif // wxART2D_USE_FREETYPE

}

#if wxART2D_USE_FREETYPE

//! \bug EW: DrawCharFreetype crashes when zoomed in extremely far.
void a2dMemDcDrawer::DrawCharFreetype( wxColour col, wxChar c, int dx, int dy, FT_Matrix trans_matrix, FT_Vector vec )
{
    // Use next to test on screen a2dDcDrawer vpath drawing for printing.
    //a2dDcDrawer::DrawCharFreetype( c );
    //return;


    // Get the glyph and make a copy of it
    const a2dGlyph* a2dglyph = m_currentfont.GetGlyphFreetype( c );
    // The glyph is now grid fitted and hinted.
    if( !a2dglyph )
        return;
    FT_Glyph image;
    if ( FT_Glyph_Copy( a2dglyph->m_glyph, &image ) != 0 )
        return;

    if ( FT_Glyph_Transform( image, &trans_matrix, &vec ) != 0 )
    {
        wxFAIL_MSG( _( "glyph transform wrong" ) );
        FT_Done_Glyph( image );
        return;
    }

    // Get bounding box of the area which should be drawn.
    FT_BBox bbox;
    FT_Glyph_Get_CBox( image, FT_GLYPH_BBOX_PIXELS, &bbox );
      
    // clip glyph device box against device drawing clipbox
    if ( m_clipboxdev.GetLeft() > dx + bbox.xMin )
        bbox.xMin = m_clipboxdev.GetLeft() - dx;
    if ( m_clipboxdev.GetRight() < dx + bbox.xMax )
        bbox.xMax = m_clipboxdev.GetRight() - dx;

    if ( m_clipboxdev.GetTop() > dy - bbox.yMax )
        bbox.yMax = dy - m_clipboxdev.GetTop();
    if ( m_clipboxdev.GetBottom() < dy - bbox.yMin )
        bbox.yMin = dy - m_clipboxdev.GetBottom();

    // If size == 0, nothing has to be drawn.
    if ( bbox.xMax - bbox.xMin <= 0 || bbox.yMax - bbox.yMin <= 0 )
    {
        FT_Done_Glyph( image );
        return;
    }
    // WHY: The glyph boundingbox is used to position and align text to draw (it is transformed)
    // This can rounded to one pixel more or less, coming from world coordinates.
    // It depends on the redraw area how the overlap of the glyph is rounded.
    // And therefore some small part of the text that should be (re)drawn in the clipping rectangle, is not.
    // This becomes a problem when glyph box is clipped to the whole drawing window, it sometimes a pixel off (even if just drawing rectangles and such)
    // Moving content of screen, it becomes visible.
    // When the update rectangle which fits completely in the window, it is not a problem, because it already has some extra pixels to redraw.
    // SOLUTION:
    // We assume the device clipping box is set to redraw part of the canvas (already made a few pixels more then needed, void a2dDrawingPart::RedrawPendingUpdateAreas() ).
    // The glyph bounding box part, which might overlap the redraw area (device clipping box), should be redrawn.
    // It does not hurt to extend the overlap part a pixel, as it will stay within the device clipping box.
    // We extend the boundingbox of a glyph, to make sure a bit more is drawn where needed.   
    // It seems adding one pixel like this enough, else add other also.
    //bbox.xMin -= BBOX_GLYPH_EXTEND;
    bbox.xMax += BBOX_GLYPH_EXTEND;
    //bbox.yMin -= BBOX_GLYPH_EXTEND;
    bbox.yMax += BBOX_GLYPH_EXTEND;

    //glyph boundingbox is clipped as if placed at right position, but not calculated/placed at device coordinate.
    //De bitmap we will render is (0,0,w,h) same size a glyph bbox

    int px = dx + bbox.xMin;
    int py = dy - bbox.yMax;
    int w = bbox.xMax - bbox.xMin + 1; //yes one more in pixels!
    int h = bbox.yMax - bbox.yMin + 1;

    /* TO DEBUG to see what is the glyph boundingbox
        //draw randum colours for glyph rectangle to see, what and where.
        wxPen pen = m_renderDC->GetPen();
        wxBrush brush = m_renderDC->GetBrush();
        wxColour color( 250, 50, 255, 0 );
        wxColour color2( 20, 30, 120, 0 );
        //wxColour color2( rand()%254,rand()%254,rand()%254, 30 );
        //m_renderDC->SetPen( wxPen( color ) );
        //wxColour color2 = pen.GetColour();
        m_renderDC->SetPen(*wxTRANSPARENT_PEN );
        m_renderDC->SetBrush( wxBrush( color2 ) );
        m_renderDC->SetPen( wxPen( color ) );
        //m_renderDC->SetBrush( *wxTRANSPARENT_BRUSH );
        m_renderDC->DrawRectangle( px, py, w, h);
        m_renderDC->SetPen( color );
        m_renderDC->SetBrush( brush );
    */

    // Obtain background image/bitmap at the position the glyph bitmap will be drawn later.
    wxRect absarea2( px, py, w, h );
    //if the absarea2 is/would not be not clipped to device clipbox, the resulting bitmap, will not be clipped either, so stays w*h in size.
    //Understand when drawing the bitmap later on, it will be clipped again, so a pixel more or less in width does not hurt.
    //Still better to clip the glyph bounding as in above code, in order to render only the part of the glyph that is needed.
    wxBitmap sub_bitmap = GetSubBitmap( absarea2 );

    //To prevent converting to wxImage and pack, we doe it like this, wonder if it gains much speed.
#if defined(__WXMSW__)
    BITMAPINFO bi;
    bi.bmiHeader.biSize = sizeof( bi.bmiHeader );
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h; //coordinate system freetype is Y up, and GTK and windows is Y down in pixels.
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = 0;
    bi.bmiHeader.biClrUsed = 0;
    bi.bmiHeader.biClrImportant = 0;

    HBITMAP hbitmap = ( HBITMAP ) sub_bitmap.GetHBITMAP();
    unsigned char* buf = ( unsigned char* ) malloc( ( 3 * w + sizeof( DWORD ) - 1 ) / sizeof( DWORD ) * sizeof( DWORD ) * h );
    ::GetDIBits( m_hdc2, hbitmap, 0, h, buf, &bi, DIB_RGB_COLORS );
#else // defined( __WXMSW__ )
    wxImage wximage( sub_bitmap.ConvertToImage() );
#endif // defined( __WXMSW__ )

    // Render glyph to an image
    if ( image->format == FT_GLYPH_FORMAT_OUTLINE )
    {
        a2dSpanData spandata;
        spandata.colour = col;
        //here the newly calculated clipped glyph bounding box, meaning freetype start rendering only part of the glyph
        spandata.xmin = bbox.xMin; 
        spandata.ymax = bbox.yMax; 
        spandata.stride = w * 3;
        spandata.height = h;
#if defined(__WXMSW__)
        // calculate image stride ( = w + padding )
        spandata.stride = ( spandata.stride + sizeof( DWORD ) - 1 ) / sizeof( DWORD ) * sizeof( DWORD );
        spandata.sizebuf = ( spandata.stride + sizeof( DWORD ) - 1 ) / sizeof( DWORD ) * sizeof( DWORD ) * (spandata.height + 1);
        spandata.buf = buf;
#else // defined( __WXMSW__ )
        spandata.sizebuf = w * h * 3;
        spandata.buf = wximage.GetData();
#endif // defined( __WXMSW__ )
        wxASSERT( spandata.buf != NULL );
        spandata.modeUp = false;

        FT_Raster_Params params;
        params.target = NULL;
        params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT | FT_RASTER_FLAG_CLIP;
        params.gray_spans = ( FT_SpanFunc ) & a2dSpanFuncGray;
        params.black_spans = NULL;
        params.bit_test = ( FT_Raster_BitTest_Func ) NULL;
        params.bit_set = ( FT_Raster_BitSet_Func ) NULL;
        params.user = &spandata;
        //should clip everything rendered to the buffer, but that is not really the case, 
        //inside a2dSpanFuncGray we have to make sure we do not go outside of the buffer.
        params.clip_box = bbox; 

        FT_Outline& outline = ( ( FT_OutlineGlyph ) image )->outline;
        if ( FT_Outline_Render( g_freetypeLibrary, &outline, &params ) == 0 )
        {
#if defined( __WXMSW__ )
            ::SetDIBits( m_hdc2, hbitmap, 0, h, buf, &bi, DIB_RGB_COLORS );
#else // defined( __WXMSW__ )
            wxBitmap sub_bitmap( wximage );
#endif // defined( __WXMSW__ )

            //Draw the rendered glyph inside the bitmap back to total buffer.
            DeviceDrawBitmap( sub_bitmap, px, py  );
        }
    }

    else
        wxFAIL_MSG( _( "Non vector fonts are not supported" ) );

#if defined(__WXMSW__)
    free( buf );
#endif // defined( __WXMSW__ )

    FT_Done_Glyph( image );
}

#endif // wxART2D_USE_FREETYPE

#if wxART2D_USE_GRAPHICS_CONTEXT

IMPLEMENT_DYNAMIC_CLASS( a2dGcDrawer, a2dGcBaseDrawer )

/*******************************************************************
a2dGcDrawer
********************************************************************/

void a2dGcDrawer::InitContext()
{
    m_externalDc = false;
    m_clip.Clear();
    m_buffer = wxBitmap( m_width, m_height );
    m_memdc.SelectObject( m_buffer );
    m_render = new a2dRenderer();
    m_context = ( a2dDcContext* ) ( ( a2dRenderer* )m_render )->CreateContext( &m_memdc, &m_buffer );
}

a2dGcDrawer::a2dGcDrawer( const wxSize& size ): a2dGcBaseDrawer( size.GetWidth(), size.GetHeight() )
{
    InitContext();
}

a2dGcDrawer::a2dGcDrawer( int width, int height ): a2dGcBaseDrawer( width, height )
{
    InitContext();
}

a2dGcDrawer::a2dGcDrawer( const a2dGcDrawer& other )
    : a2dGcBaseDrawer( other )
{
    InitContext();
}

a2dGcDrawer::a2dGcDrawer( const a2dDrawer2D& other )
    : a2dGcBaseDrawer( other )
{
    InitContext();
}

a2dGcDrawer::~a2dGcDrawer()
{
    delete m_context;
    delete m_render;
    m_memdc.SelectObject( wxNullBitmap );
}

wxBitmap a2dGcDrawer::GetBuffer() const
{
    return m_buffer;
}

void a2dGcDrawer::BeginDraw()
{
    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
        m_memdc.SelectObject( m_buffer );

        SetDrawerFill( m_currentfill );
        SetDrawerStroke( m_currentstroke );

        DestroyClippingRegion();

        SetDrawStyle( m_drawstyle );

        m_memdc.SetMapMode( wxMM_TEXT );
    }

    m_beginDraw_endDraw++;
}

void a2dGcDrawer::EndDraw()
{
    m_beginDraw_endDraw--;

    wxASSERT_MSG( m_beginDraw_endDraw >= 0, wxT( "a2dMemDcDrawer, unbalanced BeginDraw EndDraw" ) );

    if ( m_beginDraw_endDraw == 0 )
    {
        m_memdc.SelectObject( wxNullBitmap );
    }
}

void a2dGcDrawer::SetBufferSize( int w, int h )
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
    m_context = ( a2dDcContext* ) ( ( a2dRenderer* )m_render )->CreateContext( &m_memdc, &m_buffer );
}

wxBitmap a2dGcDrawer::GetSubBitmap( wxRect rect ) const
{
    wxBitmap ret( rect.width, rect.height, m_buffer.GetDepth() );
    wxASSERT_MSG( ret.Ok(), wxT( "GetSubImage error" ) );

    //do NOT use getsubbitmap, renderDc is already set for the buffer therefor will not work properly
    wxMemoryDC dcb;
    dcb.SelectObject( ret );
    dcb.Blit( 0, 0, rect.width, rect.height, const_cast<wxMemoryDC*>( &m_memdc ), rect.x, rect.y, wxCOPY, false );
    dcb.SelectObject( wxNullBitmap );

    return ret;
}

void a2dGcDrawer::BlitBuffer( wxDC* dc, wxRect rect, const wxPoint& bufferpos )
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

void a2dGcDrawer::ShiftBuffer( int dxy, bool yshift )
{
    int bw = GetBuffer().GetWidth();
    int bh = GetBuffer().GetHeight();
    if ( yshift )
    {
        if ( dxy > 0 && dxy < bh )
        {
            wxRect rect( 0, 0, bw, bh - dxy );
            wxBitmap sub_bitmap( m_buffer.GetSubBitmap( rect ) );
            wxMemoryDC m_mdc1;
            m_mdc1.SelectObject( m_buffer );
            m_mdc1.DrawBitmap( sub_bitmap, 0, dxy, true );
            m_mdc1.SelectObject( wxNullBitmap );
        }
        else  if ( dxy < 0 && dxy > -bh )
        {
            wxRect rect( 0, -dxy, bw, bh + dxy );
            wxBitmap sub_bitmap( m_buffer.GetSubBitmap( rect ) );
            wxMemoryDC m_mdc1;
            m_mdc1.SelectObject( m_buffer );
            m_mdc1.DrawBitmap( sub_bitmap, 0, 0, true );
            m_mdc1.SelectObject( wxNullBitmap );
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
            wxMemoryDC m_mdc1;
            m_mdc1.SelectObject( m_buffer );
            m_mdc1.DrawBitmap( sub_bitmap, dxy, 0, true );
            m_mdc1.SelectObject( wxNullBitmap );
        }
        else if ( dxy < 0 && dxy > -bw )
        {
            wxRect rect( -dxy, 0, bw + dxy, bh );
            wxBitmap sub_bitmap( m_buffer.GetSubBitmap( rect ) );
            wxMemoryDC m_mdc1;
            m_mdc1.SelectObject( m_buffer );
            m_mdc1.DrawBitmap( sub_bitmap, 0, 0, true );
            m_mdc1.SelectObject( wxNullBitmap );
        }
        else
            wxFAIL_MSG( wxT( "you can only shift within width of buffer" ) );
    }
}

void a2dGcDrawer::SetYaxis( bool up )
{
    a2dDrawer2D::SetYaxis( up );
    //a2dContext* context = (a2dContext*) m_context;
    //context->SetYaxis( up );
}

void a2dGcDrawer::DoSetActiveFont( const a2dFont& font )
{
    a2dContext* context = ( a2dContext* ) m_context;
    a2dRenderer* render = ( a2dRenderer* ) m_render;
    wxGraphicsFont f = render->CreateFont( font );
    context->SetFont( f );
}

void a2dGcDrawer::DoSetActiveStroke()
{
    // wxGraphicsRenderer render class does understand a2dStroke and a2dFill,
    // so we will have a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT available, else lost.
    a2dRenderer* render = ( a2dRenderer* ) m_render;
    a2dContext* context = ( a2dContext* ) m_context;
    wxGraphicsPen p = render->CreateStroke( m_activestroke  );
    m_context->SetPen( p );
}

void a2dGcDrawer::DoSetActiveFill()
{
    // wxGraphicsRenderer render class does understand a2dStroke and a2dFill,
    // so we will have a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT available, else lost.
    a2dRenderer* render = ( a2dRenderer* ) m_render;
    a2dContext* context = ( a2dContext* ) m_context;
    wxGraphicsBrush b = render->CreateFill( m_activefill );
    m_context->SetBrush( b );
}

void a2dGcDrawer::DrawImage( const wxImage& imagein, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    // wxGraphicsRenderer render class does understand a2dStroke and a2dFill,
    // so we will have a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT available, else lost.
    a2dRenderer* render = ( a2dRenderer* ) m_render;
    a2dContext* context = ( a2dContext* ) m_context;
    wxGraphicsBitmap bitmap = render->CreateBitmap( imagein );
#if wxCHECK_VERSION(2,9,0)
    context->DrawBitmap( bitmap, x, y, width, height );
#else
    context->DrawGraphicsBitmap( bitmap, x, y, width, height );
#endif
}

void a2dGcDrawer::DrawImage( const a2dImageRGBA& image, double x, double y, double width, double height, wxUint8 Opacity )
{
    if ( m_disableDrawing )
        return;

    wxImage wximage = image.GetImage();
    a2dGcDrawer::DrawImage( wximage, x, y, width, height, Opacity );
}

void a2dGcDrawer::DrawPoint( double xc, double yc )
{
    if ( m_disableDrawing )
        return;

    double xt, yt;
    GetUserToDeviceTransform().TransformPoint( xc, yc, xt, yt );
    //DeviceDrawPixel( xt, yt, m_colour1redStroke, m_colour1greenStroke, m_colour1blueStroke, m_StrokeOpacityCol1 );
}

/* HANDLED IN m_context
void a2dGcDrawer::DrawCharDc( wxChar c )
{
    wxGraphicsMatrix m = m_context->CreateMatrix();
    m.Set(
        m_usertodevice( 0, 0), m_usertodevice( 0, 1),
        m_usertodevice( 1, 0), m_usertodevice( 1, 1),
        m_usertodevice( 2, 0), m_usertodevice( 2, 1) );
    m_context->SetTransform( m );

#if wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
    if ( m_currentfont.GetType() == a2dFONT_WXDC && m_currentfont.GetFreetypeFont().Ok() )
    {
        a2dFont oldfont = m_currentfont;
        m_currentfont = m_currentfont.GetFreetypeFont();
        static_cast<a2dContext*>(m_context)->SetFont( m_currentfont );
        static_cast<a2dDcContext*>(m_context)->DrawCharFreetype( c );
        m_currentfont = oldfont;
    }
    else
        static_cast<a2dDcContext*>(m_context)->DrawCharDc( c );
#else // wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
    static_cast<a2dDcContext*>(m_context)->DrawCharDc( c );
#endif // wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
}

//! \bug EW: DrawCharFreetype crashes when zoomed in extremely far.
void a2dGcDrawer::DrawCharFreetype( wxChar c )
{
    wxGraphicsMatrix m = m_context->CreateMatrix();
    m.Set(
        m_usertodevice( 0, 0), m_usertodevice( 0, 1),
        m_usertodevice( 1, 0), m_usertodevice( 1, 1),
        m_usertodevice( 2, 0), m_usertodevice( 2, 1) );
    m_context->SetTransform( m );
    static_cast<a2dDcContext*>(m_context)->DrawCharFreetype( c );
}
*/



#endif  // wxART2D_USE_GRAPHICS_CONTEXT



















