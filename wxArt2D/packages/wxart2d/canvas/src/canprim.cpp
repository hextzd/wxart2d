/*! \file canvas/src/canprim.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canprim.cpp,v 1.351 2009/07/24 16:35:01 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/wfstream.h>
#include <wx/module.h>
#include <wx/clipbrd.h>

#include "wx/canvas/canobj.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/vpath.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/wire.h"

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif


#define CIRCLE_STEPS 128

IMPLEMENT_CLASS( a2dWH, a2dCanvasObject )
IMPLEMENT_CLASS( a2dWHCenter, a2dWH )

IMPLEMENT_DYNAMIC_CLASS( a2dOrigin, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dArrow, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dRectC, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dRect, a2dWH )
IMPLEMENT_DYNAMIC_CLASS( a2dCircle, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dEllipse, a2dWHCenter )
IMPLEMENT_DYNAMIC_CLASS( a2dEllipticArc, a2dWHCenter )
IMPLEMENT_DYNAMIC_CLASS( a2dArc, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dSLine, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dScaledEndLine, a2dEndsLine )
IMPLEMENT_DYNAMIC_CLASS( a2dEndsLine, a2dSLine )
IMPLEMENT_DYNAMIC_CLASS( a2dEndsEllipticChord, a2dEllipticArc )
IMPLEMENT_CLASS( a2dControl, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dWires, a2dCanvasObject )

//----------------------------------------------------------------------------
// a2dOrigin
//----------------------------------------------------------------------------
double a2dOrigin::m_widthDefault = 100;
double a2dOrigin::m_heightDefault = 100;
bool   a2dOrigin::m_doRender = true;

a2dOrigin::a2dOrigin()
    : a2dCanvasObject()
{
    m_width = m_widthDefault;
    m_height = m_heightDefault;
}

a2dOrigin::a2dOrigin( double w, double h )
    : a2dCanvasObject()
{
    m_width = w;
    m_height = h;
}

a2dOrigin::~a2dOrigin()
{
}

a2dOrigin::a2dOrigin( const a2dOrigin& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_width = other.m_width;
    m_height = other.m_height;
}

a2dObject* a2dOrigin::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dOrigin( *this, options, refs );
};

a2dBoundingBox a2dOrigin::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    bbox.Expand( -m_width, -m_height );
    bbox.Expand(  m_width,  m_height );
    return bbox;
}

void a2dOrigin::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    if ( !m_doRender )
        return;

    int w = ( int ) m_width;
    int h = ( int ) m_height;

    ic.GetDrawer2D()->DrawLine( -w, 0, w, 0 );
    ic.GetDrawer2D()->DrawLine(  0, -h, 0, h );
}

bool a2dOrigin::DoIsHitWorld( a2dIterC& WXUNUSED( ic ), a2dHitEvent& hitEvent )
{
    hitEvent.m_how.m_hit = a2dHit::hit_fill;
    return true;
}

#if wxART2D_USE_CVGIO
void a2dOrigin::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dArrow
//----------------------------------------------------------------------------

a2dArrow::a2dArrow( double xt, double yt, double l1, double l2, double b, bool spline )
    : a2dCanvasObject()
{
    m_lworld.Translate( xt, yt );
    m_l1 = l1;
    m_l2 = l2;
    m_b  = b;
    m_spline = spline;
}

a2dArrow::a2dArrow()
    : a2dCanvasObject()
{
    m_lworld.Translate( 0, 0 );
    m_l1 = 40;
    m_l2 = 20;
    m_b  = 10;
    m_spline = false;
}

a2dArrow::~a2dArrow()
{
}

a2dArrow::a2dArrow( const a2dArrow& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_l1 = other.m_l1;
    m_l2 = other.m_l2;
    m_b  = other.m_b;
    m_spline = other.m_spline;
}

void a2dArrow::Set( double xt, double yt, double l1, double l2, double b, bool spline )
{
    SetPosXY( xt, yt );
    m_l1 = l1;
    m_l2 = l2;
    m_b  = b;
    m_spline = spline;

    SetPending( true );
}

void a2dArrow::Set( double l1, double l2, double b, bool spline )
{
    m_l1 = l1;
    m_l2 = l2;
    m_b  = b;
    m_spline = spline;

    SetPending( true );
}

a2dObject* a2dArrow::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    a2dArrow* a = new a2dArrow( *this, options, refs );
    return a;
};

a2dVertexList* a2dArrow::GetAsVertexList( bool& returnIsPolygon ) const
{
    returnIsPolygon = true;
    returnIsPolygon = true;

    a2dVertexList* pointlist = new a2dVertexList();

    a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths();
    a2dVpath* segments = wxStaticCast( vectorpaths->front().Get(), a2dVectorPath ) ->GetSegments();
    double AberArcToPoly = m_root->GetAberArcToPolyScaled();
    segments->ConvertToLines( AberArcToPoly );
    a2dLineSegment* point;
    unsigned int i;
    for ( i = 0; i < segments->size(); i++ )
    {
        a2dVpathSegment* seg = segments->Item( i );
        point = new a2dLineSegment( seg->m_x1, seg->m_y1 );
        pointlist->push_back( point );
    }

    delete vectorpaths;

    return pointlist;
}

a2dCanvasObjectList* a2dArrow::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dVpath* segments = new a2dVpath();

    double x, y;
    pworld.TransformPoint( 0 , 0, x, y );
    a2dVpathSegment* seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
    segments->Add( seg );
    pworld.TransformPoint( m_l1, m_b / 2, x, y );
    seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
    segments->Add( seg );
    pworld.TransformPoint( m_l2, 0, x, y );
    seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
    segments->Add( seg );
    pworld.TransformPoint( m_l1, -m_b / 2, x, y );
    seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
    segments->Add( seg );

    a2dVectorPath* canpath = new a2dVectorPath( segments );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );
    canpath->SetContourWidth( GetContourWidth() );
    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}

a2dBoundingBox a2dArrow::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    bbox.Expand( 0 , - m_b / 2 );
    bbox.Expand( m_l1 , m_b / 2 );
    return bbox;
}

void a2dArrow::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    a2dVertexArray* cpoints = new a2dVertexArray;

    cpoints->push_back( new a2dLineSegment( 0, 0 ) );
    cpoints->push_back( new a2dLineSegment( m_l1, m_b / 2.0 ) );
    cpoints->push_back( new a2dLineSegment( m_l2, 0 ) );
    cpoints->push_back( new a2dLineSegment( m_l1, -m_b / 2.0 ) );

    ic.GetDrawer2D()->DrawPolygon( cpoints, m_spline, wxWINDING_RULE );

    cpoints->clear();
    delete cpoints;
}

bool a2dArrow::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    a2dVertexArray points;
    points.push_back( new a2dLineSegment( 0, 0 ) );
    points.push_back( new a2dLineSegment( m_l1, m_b / 2.0 ) );
    points.push_back( new a2dLineSegment( m_l2, 0 ) );
    points.push_back( new a2dLineSegment( m_l1, -m_b / 2.0 ) );

    a2dPoint2D P = a2dPoint2D( hitEvent.m_relx, hitEvent.m_rely );
    hitEvent.m_how = points.HitTestPolygon( P, ic.GetWorldStrokeExtend() + ic.GetTransformedHitMargin() );

    return hitEvent.m_how.IsHit();
}

#if wxART2D_USE_CVGIO
void a2dArrow::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "l1" ), m_l1 * out.GetScale() );
        out.WriteAttribute( wxT( "l2" ), m_l2 * out.GetScale() );
        out.WriteAttribute( wxT( "b" ), m_b * out.GetScale() );
        out.WriteAttribute( wxT( "spline" ), m_spline );
    }
    else
    {
    }
}

void a2dArrow::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_l1 = parser.RequireAttributeValueDouble( wxT( "l1" ) ) * parser.GetScale() ;
        m_l2 = parser.RequireAttributeValueDouble( wxT( "l2" ) ) * parser.GetScale() ;
        m_b = parser.GetAttributeValueDouble( wxT( "b" ) ) * parser.GetScale() ;
        m_spline = parser.GetAttributeValueBool( wxT( "spline" ) );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dRectC
//----------------------------------------------------------------------------

a2dPropertyIdDouble* a2dRectC::PROPID_Width = NULL;
a2dPropertyIdDouble* a2dRectC::PROPID_Height = NULL;
a2dPropertyIdDouble* a2dRectC::PROPID_Radius = NULL;

INITIALIZE_PROPERTIES( a2dRectC, a2dCanvasObject )
{
    PROPID_Width = new a2dPropertyIdDouble( wxT( "Width" ),
                                            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dRectC::m_width ) );
    AddPropertyId( PROPID_Width );
    PROPID_Height = new a2dPropertyIdDouble( wxT( "Height" ),
            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dRectC::m_height ) );
    AddPropertyId( PROPID_Height );
    PROPID_Radius = new a2dPropertyIdDouble( wxT( "Radius" ),
            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dRectC::m_radius ) );
    AddPropertyId( PROPID_Radius );

    return true;
}


a2dRectC::a2dRectC()
    : a2dCanvasObject()
{
    m_width = 10;
    m_height = 10;
    m_radius  = 0;
}

a2dRectC::a2dRectC( double xc, double yc, double w, double h, double angle, double radius )
    : a2dCanvasObject()
{
    Rotate( angle );
    m_lworld.Translate( xc, yc );
    m_width = w;
    m_height = h;
    m_radius  = radius;
}

a2dRectC::~a2dRectC()
{
}

a2dRectC::a2dRectC( const a2dRectC& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_width = other.m_width;
    m_height = other.m_height;
    m_radius = other.m_radius;
}

void a2dRectC::Set( double xc, double yc, double w, double h, double angle , double radius )
{
    SetRotation( angle );
    SetPosXY( xc, yc );
    m_width = w;
    m_height = h;
    m_radius  = radius;
    SetPending( true );
}


a2dObject* a2dRectC::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dRectC( *this, options, refs );
};

a2dVertexList* a2dRectC::GetAsVertexList( bool& returnIsPolygon ) const
{
    returnIsPolygon = true;
    a2dVertexList* pointlist = new a2dVertexList();

    a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths();
    a2dVpath* segments = wxStaticCast( vectorpaths->front().Get(), a2dVectorPath ) ->GetSegments();
    double AberArcToPoly = double( m_root->GetHabitat()->GetAberArcToPoly() ) / m_root->GetUnitsScale();
    segments->ConvertToLines( AberArcToPoly );
    a2dLineSegment* point;
    unsigned int i;
    for ( i = 0; i < segments->size(); i++ )
    {
        a2dVpathSegment* seg = segments->Item( i );
        point = new a2dLineSegment( seg->m_x1, seg->m_y1 );
        pointlist->push_back( point );
    }

    delete vectorpaths;
    return pointlist;
}

a2dCanvasObjectList* a2dRectC::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dVpath* segments = new a2dVpath();

    double w2 = fabs( m_width / 2 );
    double h2 = fabs( m_height / 2 );

    if ( m_radius && m_width && m_height )
    {

        if ( m_radius < 0 )
        {
            double rx = -m_radius;
            if ( m_width < 0 )
                rx = -rx;

            double ry = -m_radius;
            if ( m_height < 0 )
                ry = -ry;

            a2dVpathSegment* seg = new a2dVpathSegment( rx, 0 , a2dPATHSEG_MOVETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, rx, ry, 0.0, ry,  0.0, 0.0, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( 0, m_height - ry, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, rx, m_height - ry, rx, m_height, 0.0, m_height, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( m_width - rx, m_height, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, m_width - rx, m_height - ry, m_width, m_height - ry, m_width, m_height, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( m_width, ry, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, m_width - rx, ry, m_width - rx, 0.0, m_width, 0.0, a2dPATHSEG_ARCTO, a2dPATHSEG_END_CLOSED  );
            segments->push_back( seg );

            a2dAffineMatrix world;
            world.Translate( -w2, -h2 );
            world *= pworld;
            segments->Transform( world );
        }
        else
        {
            double rx = m_radius;
            if ( m_width < 0 )
                rx = -rx;

            double ry = m_radius;
            if ( m_height < 0 )
                ry = -ry;

            a2dVpathSegment* seg = new a2dVpathSegment( 0.0, -ry , a2dPATHSEG_MOVETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg,  0.0, 0.0, -rx, 0.0, -rx, -ry, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( -rx, m_height, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, 0.0, m_height, 0.0, m_height + ry, -rx, m_height + ry, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( m_width, m_height + ry, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, m_width, m_height, m_width + rx, m_height, m_width + rx, m_height + ry, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( m_width + rx, 0.0, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, m_width, 0.0, m_width, -ry, m_width + rx, -ry, a2dPATHSEG_ARCTO, a2dPATHSEG_END_CLOSED  );
            segments->push_back( seg );

            a2dAffineMatrix world;
            world.Translate( -w2, -h2 );
            world *= pworld;
            segments->Transform( world );
        }
    }
    else
    {
        double x, y;
        pworld.TransformPoint( -w2 , -h2, x, y );
        a2dVpathSegment* seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
        segments->push_back( seg );
        pworld.TransformPoint( -w2 , +h2, x, y );
        seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
        segments->push_back( seg );
        pworld.TransformPoint( +w2 , +h2, x, y );
        seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
        segments->push_back( seg );
        pworld.TransformPoint( +w2 , -h2, x, y );
        seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED );
        segments->push_back( seg );
    }

    a2dVectorPath* canpath = new a2dVectorPath( segments );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );
    canpath->SetContourWidth( GetContourWidth() );
    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}



a2dBoundingBox a2dRectC::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    bbox.Expand( - fabs( m_width / 2 ) , - fabs( m_height / 2 ) );
    bbox.Expand( + fabs( m_width / 2 ) , + fabs( m_height / 2 ) );
    if ( m_radius > 0 )
        bbox.Enlarge( m_radius );
    return bbox;
}

void a2dRectC::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    if ( m_radius <= 0 )
    {
        ic.GetDrawer2D()->DrawCenterRoundedRectangle( 0, 0, m_width, m_height, -m_radius );
    }
    else
    {
        ic.GetDrawer2D()->DrawCenterRoundedRectangle( 0, 0, m_width + 2 * m_radius, m_height + 2 * m_radius, m_radius );
    }
}

bool a2dRectC::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double margin = ic.GetTransformedHitMargin();

    double xmax = fabs( m_width / 2 );
    double ymax = fabs( m_height / 2 );
    double xmin = -xmax;
    double ymin = -ymax;

    if ( m_radius > 0 )
    {
        xmin -= m_radius;
        ymin -= m_radius;
        xmax += m_radius;
        ymax += m_radius;
    }

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, xmin, ymin, xmax, ymax, ic.GetWorldStrokeExtend() + margin );

    return hitEvent.m_how.IsHit();
}

#if wxART2D_USE_CVGIO
void a2dRectC::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "width" ), m_width * out.GetScale() );
        out.WriteAttribute( wxT( "height" ), m_height * out.GetScale() );
        if ( m_radius != 0.0 )
        {
            out.WriteAttribute( wxT( "radius" ), m_radius );
        }
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO


//----------------------------------------------------------------------------
// a2dWH
//----------------------------------------------------------------------------

a2dPropertyIdDouble* a2dWH::PROPID_Width = NULL;
a2dPropertyIdDouble* a2dWH::PROPID_Height = NULL;

INITIALIZE_PROPERTIES( a2dWH, a2dCanvasObject )
{
    PROPID_Width = new a2dPropertyIdDouble( wxT( "Width" ),
                                            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dWH::m_width ) );
    AddPropertyId( PROPID_Width );
    PROPID_Height = new a2dPropertyIdDouble( wxT( "Height" ),
            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dWH::m_height ) );
    AddPropertyId( PROPID_Height );

    return true;
}



BEGIN_EVENT_TABLE( a2dWH, a2dCanvasObject )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dWH::OnHandleEvent )
    EVT_CANVASOBJECT_RESIZE_EVENT( a2dWH::OnResizeObjectToChildBox )
END_EVENT_TABLE()

a2dWH::a2dWH()
    : a2dCanvasObject()
{
    m_width = 0;
    m_height = 0;
}

a2dWH::a2dWH( double x, double y, double w, double h )
    : a2dCanvasObject()
{
    m_lworld.Translate( x, y );
    m_width = w;
    m_height = h;
}

a2dWH::a2dWH( const a2dWH& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_width = other.m_width;
    m_height = other.m_height;
}

a2dWH::~a2dWH()
{
}

a2dCanvasObjectList* a2dWH::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dVpath* segments = new a2dVpath();
    a2dVectorPath* canpath = new a2dVectorPath( segments );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );

    double x, y;
    pworld.TransformPoint( 0, 0, x, y );
    a2dVpathSegment* seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
    segments->push_back( seg );
    pworld.TransformPoint( 0 , m_height, x, y );
    seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
    segments->push_back( seg );
    pworld.TransformPoint( m_width , m_height, x, y );
    seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
    segments->push_back( seg );
    pworld.TransformPoint( m_width , 0, x, y );
    seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED );
    segments->push_back( seg );

    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}

bool a2dWH::RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld )
{
    // object has disabled snap?
    if ( !m_flags.m_snap_to )
        return false;

    bool res = false;
    if ( !res )
        res = a2dCanvasObject::RestrictToObject( ic, pointToSnapTo, bestPointSofar, snapToWhat, thresHoldWorld );

    return res;
}

a2dBoundingBox a2dWH::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    bbox.Expand( 0, 0 );
    bbox.Expand( m_width , m_height );
    return bbox;
}

#if wxART2D_USE_CVGIO

void a2dWH::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "width" ), m_width * out.GetScale() );
        out.WriteAttribute( wxT( "height" ), m_height * out.GetScale() );
    }
    else
    {
    }
}

void a2dWH::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_width = parser.RequireAttributeValueDouble( wxT( "width" ) ) * parser.GetScale();
        m_height = parser.RequireAttributeValueDouble( wxT( "height" ) ) * parser.GetScale();
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dWH::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double margin = ic.GetTransformedHitMargin();

    double xmin = wxMin( 0 , m_width );
    double ymin = wxMin( 0 , m_height );
    double xmax = wxMax( 0 , m_width );
    double ymax = wxMax( 0 , m_height );

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, xmin, ymin, xmax, ymax, ic.GetWorldStrokeExtend() + margin );

    return hitEvent.m_how.IsHit();
}

bool a2dWH::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    PROPID_IncludeChildren->SetPropertyToObject( this, false );
    PROPID_Allowrotation->SetPropertyToObject( this, true );
    PROPID_Allowskew->SetPropertyToObject( this, true );

    return a2dCanvasObject::DoStartEdit( editmode, editstyle );
}

void a2dWH::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();

        a2dHandle* draghandle = event.GetCanvasHandle();

        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );

        if ( editmode == 1 )
        {
            double xw, yw;
            xw = event.GetX();
            yw = event.GetY();

            a2dAffineMatrix atWorld = ic->GetTransform();
            a2dAffineMatrix inverse = ic->GetInverseTransform();

            double xwi;
            double ywi;
            inverse.TransformPoint( xw, yw, xwi, ywi );

            a2dWH* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dWH );

            a2dBoundingBox untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_EDIT );

            double xmin, ymin, xmax, ymax, w, h;
            xmin = untrans.GetMinX();
            ymin = untrans.GetMinY();
            xmax = untrans.GetMaxX();
            ymax = untrans.GetMaxY();
            w = untrans.GetWidth();
            h = untrans.GetHeight();

            a2dAffineMatrix origworld = m_lworld;
            double x1, y1, x2, y2;

            if ( event.GetMouseEvent().LeftDown() )
            {
            }
            else if ( event.GetMouseEvent().LeftUp() )
            {
                m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty(  original, PROPID_TransformMatrix, m_lworld ) );
            }
            else if ( event.GetMouseEvent().Dragging() )
            {
                if ( restrictEngine )
                    restrictEngine->RestrictPoint( xw, yw );
                ic->GetInverseTransform().TransformPoint( xw, yw, xwi, ywi );

                if ( draghandle->GetName() == wxT( "handle1" ) )
                {
                    //modify object
                    double dx, dy;
                    dx = xwi - xmin;
                    dy = ywi - ymin;

                    double sx;
                    double sy;
                    if ( w )
                        sx = ( w - dx / 2 ) / w;
                    else
                        sx = 0;
                    if ( h )
                        sy = ( h - dy / 2 ) / h;
                    else
                        sy = 0;

                    origworld.TransformPoint( xmax, ymax, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    Scale( sx, sy );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmax, ymax, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else if ( draghandle->GetName() == wxT( "handle2" ) )
                {
                    //modify object
                    double dx, dy;
                    dx = xwi - xmin;
                    dy = ywi - ymax;

                    double sx;
                    double sy;
                    if ( w )
                        sx = ( w - dx / 2 ) / w;
                    else
                        sx = 0;
                    if ( h )
                        sy = ( h + dy / 2 ) / h;
                    else
                        sy = 0;

                    origworld.TransformPoint( xmax, ymin, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    Scale( sx, sy );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmax, ymin, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else if ( draghandle->GetName() == wxT( "handle3" ) )
                {
                    //modify object
                    double dx, dy;

                    dx = xwi - xmax;
                    dy = ywi - ymax;

                    double sx;
                    double sy;
                    if ( w )
                        sx = ( w + dx / 2 ) / w;
                    else
                        sx = 0;
                    if ( h )
                        sy = ( h + dy / 2 ) / h;
                    else
                        sy = 0;

                    origworld.TransformPoint( xmin, ymin, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    Scale( sx, sy );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmin, ymin, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else if ( draghandle->GetName() == wxT( "handle4" ) )
                {
                    //modify object
                    double dx, dy;

                    dx = xwi - xmax;
                    dy = ywi - ymin;

                    double sx;
                    double sy;
                    if ( w )
                        sx = ( w + dx / 2 ) / w;
                    else
                        sx = 0;
                    if ( h )
                        sy = ( h - dy / 2 ) / h;
                    else
                        sy = 0;

                    origworld.TransformPoint( xmin, ymax, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    Scale( sx, sy );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmin, ymax, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else if ( draghandle->GetName() == wxT( "rotate" ) )
                {
                    double xr, yr;
                    m_lworld.TransformPoint( xmin + w / 2, ymin + h / 2, xr, yr );

                    //modify object
                    double dx, dy;

                    dx = xw - xr;
                    dy = yw - yr;
                    double angn;
                    if ( !dx && !dy )
                        angn = 0;
                    else
                        angn = wxRadToDeg( atan2( dy, dx ) );

                    m_lworld = m_lworld.Rotate( angn - m_lworld.GetRotation(), xr, yr );

                    //rotate.Translate( xr, yr);
                    //rotate.Rotate(wxRadToDeg(-ang));
                    //rotate.Translate( xr, yr);
                    //Transform(rotate);
                }
                else if ( draghandle->GetName() == wxT( "skewx" ) )
                {
                    //modify object
                    double dx, dy;

                    dx = xwi - ( xmin + w * 3 / 4 );
                    dy = ywi - ( ymin + h / 2 );

                    origworld.TransformPoint( xmin + w / 2, ymin + h / 2, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    SkewX( wxRadToDeg( atan2( dx, dy ) ) );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmin + w / 2, ymin + h / 2, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else if ( draghandle->GetName() == wxT( "skewy" ) )
                {
                    //modify object
                    double dx, dy;

                    dx = xwi - ( xmin + w / 2 );
                    dy = ywi - ( ymin + h * 3 / 4 );

                    origworld.TransformPoint( xmin + w / 2, ymin + h / 2, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    SkewY( wxRadToDeg( atan2( dy, dx ) ) );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmin + w / 2, ymin + h / 2, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else if ( draghandle->GetName() == wxT( "handle12" ) )
                {
                    //modify object
                    double dx;

                    dx = xwi - xmin;

                    double sx;
                    if ( w )
                        sx = ( w - dx / 2 ) / w;
                    else
                        sx = 0;

                    origworld.TransformPoint( xmax, ymax, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    Scale( sx, 1 );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmax, ymax, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else if ( draghandle->GetName() == wxT( "handle23" ) )
                {
                    //modify object
                    double dy;

                    dy = ywi - ymax;

                    double sy;
                    if ( h )
                        sy = ( h + dy / 2 ) / h;
                    else
                        sy = 0;

                    origworld.TransformPoint( xmax, ymin, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    Scale( 1, sy );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmax, ymin, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else if ( draghandle->GetName() == wxT( "handle34" ) )
                {
                    //modify object
                    double dx;

                    dx = xwi - xmax;

                    double sx;
                    if ( w )
                        sx = ( w + dx / 2 ) / w;
                    else
                        sx = 0;

                    origworld.TransformPoint( xmin, ymin, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    Scale( sx, 1 );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmin, ymin, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else if ( draghandle->GetName() == wxT( "handle41" ) )
                {
                    //modify object
                    double dy;

                    dy = ywi - ymin;

                    double sy;
                    if ( h )
                        sy = ( h - dy / 2 ) / h;
                    else
                        sy = 0;

                    origworld.TransformPoint( xmin, ymax, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    Scale( 1, sy );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmin, ymax, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else
                    event.Skip();
                SetPending( true );
            }
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

void a2dWH::OnResizeObjectToChildBox( a2dCanvasObjectEvent& event )
{
    a2dBoundingBox box = event.GetBbox();
    SetPosXY( box.GetMinX(), box.GetMinY() );
    SetWidth( box.GetWidth() );
    SetHeight( -box.GetHeight() );
    SetPending( false );
}

bool a2dWH::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( !m_bbox.GetValid() )
    {
        calc = a2dCanvasObject::DoUpdate( mode, childbox, clipbox, propbox );

        m_shapeIdsValid = false;
    }
    return calc;
}

//----------------------------------------------------------------------------
// a2dWHCenter
//----------------------------------------------------------------------------

a2dPropertyIdDouble* a2dWHCenter::PROPID_Width = NULL;
a2dPropertyIdDouble* a2dWHCenter::PROPID_Height = NULL;

INITIALIZE_PROPERTIES( a2dWHCenter, a2dWH )
{
    PROPID_Width = new a2dPropertyIdDouble( wxT( "Width" ),
                                            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dWHCenter::m_width ) );
    AddPropertyId( PROPID_Width );
    PROPID_Height = new a2dPropertyIdDouble( wxT( "Height" ),
            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dWHCenter::m_height ) );
    AddPropertyId( PROPID_Height );

    return true;
}

BEGIN_EVENT_TABLE( a2dWHCenter, a2dWH )
END_EVENT_TABLE()

a2dWHCenter::a2dWHCenter()
    : a2dWH()
{
}

a2dWHCenter::a2dWHCenter( double xc, double yc, double w, double h )
    : a2dWH( xc, yc, w, h )
{
}

a2dWHCenter::a2dWHCenter( const a2dWHCenter& other, CloneOptions options, a2dRefMap* refs )
    : a2dWH( other, options, refs )
{
}

a2dWHCenter::~a2dWHCenter()
{
}

//----------------------------------------------------------------------------
// a2dRect
//----------------------------------------------------------------------------

a2dPropertyIdDouble* a2dRect::PROPID_Radius = NULL;
a2dPropertyIdBool* a2dRect::PROPID_AllowRounding = NULL;
int a2dRect::m_editModeScaleRadius = 1;


INITIALIZE_PROPERTIES( a2dRect, a2dWH )
{
    PROPID_Radius = new a2dPropertyIdDouble( wxT( "Radius" ),
            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dRect::m_radius ) );
    AddPropertyId( PROPID_Radius );

    PROPID_AllowRounding = new a2dPropertyIdBool(  wxT( "AllowRounding" ),
            a2dPropertyId::flag_none, false );
    AddPropertyId( PROPID_AllowRounding );

    return true;
}

BEGIN_EVENT_TABLE( a2dRect, a2dWH )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dRect::OnHandleEvent )
END_EVENT_TABLE()

a2dRect::a2dRect()
    : a2dWH()
{
    m_radius  = 0;
    m_fixedRadius = false;
    m_radiusScale = 1;
    m_contourwidth = 0;
}

a2dRect::a2dRect( double x, double y, double w, double h , double radius, double contourwidth )
    : a2dWH( x, y, w, h )
{
    m_radius = radius;
    m_fixedRadius = false;
    m_radiusScale = 1;
    m_contourwidth = contourwidth;
}

a2dRect::a2dRect( const a2dPoint2D& p1,  const a2dPoint2D& p2, double radius, double contourwidth )
    : a2dWH( p1.m_x, p1.m_y, p2.m_x - p1.m_x,  p2.m_y - p1.m_y )
{
    m_radius = radius;
    m_fixedRadius = false;
    m_radiusScale = 1;
    m_contourwidth = contourwidth;
}

a2dRect::a2dRect( const a2dBoundingBox& bbox, double radius, double contourwidth )
    : a2dWH( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth(), bbox.GetHeight() )
{
    m_radius = radius;
    m_fixedRadius = false;
    m_radiusScale = 1;
    m_contourwidth = contourwidth;
}

a2dRect::~a2dRect()
{
}

a2dRect::a2dRect( const a2dRect& other, CloneOptions options, a2dRefMap* refs )
    : a2dWH( other, options, refs )
{
    m_radius = other.m_radius;
    m_fixedRadius = other.m_fixedRadius;
    m_radiusScale = other.m_radiusScale;

    m_contourwidth = other.m_contourwidth;
}

a2dObject* a2dRect::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dRect( *this, options, refs );
};

a2dVertexList* a2dRect::GetAsVertexList( bool& returnIsPolygon ) const
{
    returnIsPolygon = true;
    a2dVertexList* pointlist = new a2dVertexList();

    a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths();
    a2dVpath* segments = wxStaticCast( vectorpaths->front().Get(), a2dVectorPath ) ->GetSegments();
    double AberArcToPoly = m_root->GetAberArcToPolyScaled();
    segments->ConvertToLines( AberArcToPoly );
    a2dLineSegment* point;
    unsigned int i;
    for ( i = 0; i < segments->size(); i++ )
    {
        a2dVpathSegment* seg = segments->Item( i );
        point = new a2dLineSegment( seg->m_x1, seg->m_y1 );
        pointlist->push_back( point );
    }

    delete vectorpaths;

    return pointlist;
}

a2dCanvasObjectList* a2dRect::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dVpath* segments = new a2dVpath();
    a2dVectorPath* canpath = new a2dVectorPath( segments );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );

    if ( m_radius )
    {

        if ( m_radius < 0 )
        {
            double rx = -m_radius;
            if ( m_width < 0 )
                rx = -rx;

            double ry = -m_radius;
            if ( m_height < 0 )
                ry = -ry;

            a2dVpathSegment* seg = new a2dVpathSegment( rx, 0 , a2dPATHSEG_MOVETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, rx, ry, 0.0, ry,  0.0, 0.0, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( 0, m_height - ry, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, rx, m_height - ry, rx, m_height, 0.0, m_height, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( m_width - rx, m_height, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, m_width - rx, m_height - ry, m_width, m_height - ry, m_width, m_height, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( m_width, ry, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, m_width - rx, ry, m_width - rx, 0.0, m_width, 0.0, a2dPATHSEG_ARCTO, a2dPATHSEG_END_CLOSED  );
            segments->push_back( seg );

            segments->Transform( pworld );
        }
        else
        {
            double rx = m_radius;
            if ( m_width < 0 )
                rx = -rx;

            double ry = m_radius;
            if ( m_height < 0 )
                ry = -ry;

            a2dVpathSegment* seg = new a2dVpathSegment( 0.0, -ry , a2dPATHSEG_MOVETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg,  0.0, 0.0, -rx, 0.0, -rx, -ry, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( -rx, m_height, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, 0.0, m_height, 0.0, m_height + ry, -rx, m_height + ry, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( m_width, m_height + ry, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, m_width, m_height, m_width + rx, m_height, m_width + rx, m_height + ry, a2dPATHSEG_ARCTO );
            segments->push_back( seg );
            seg = new a2dVpathSegment( m_width + rx, 0.0, a2dPATHSEG_LINETO );
            segments->push_back( seg );

            seg = new a2dVpathArcSegment( seg, m_width, 0.0, m_width, -ry, m_width + rx, -ry, a2dPATHSEG_ARCTO, a2dPATHSEG_END_CLOSED  );
            segments->push_back( seg );

            segments->Transform( pworld );
        }
        canpath->SetContourWidth( GetContourWidth() );
        canpath->SetPathType( a2dPATH_END_SQAURE_EXT );
    }
    else
    {
        if ( GetContourWidth() )
        {
            double x, y;
            double w = GetContourWidth() / 2.0;
            pworld.TransformPoint( -w, -w, x, y );
            a2dVpathSegment* seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
            segments->push_back( seg );
            pworld.TransformPoint( -w , m_height + w, x, y );
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
            segments->push_back( seg );
            pworld.TransformPoint( m_width + w , m_height + w, x, y );
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
            segments->push_back( seg );
            pworld.TransformPoint( m_width + w, -w, x, y );
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
            segments->push_back( seg );
            pworld.TransformPoint( -w , -w, x, y );
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
            segments->push_back( seg );
            pworld.TransformPoint( w, w, x, y );
            seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO_NOSTROKE );
            segments->push_back( seg );
            pworld.TransformPoint( m_width - w, w, x, y );
            seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO );
            segments->push_back( seg );
            pworld.TransformPoint( m_width - w, m_height - w, x, y );
            seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO );
            segments->push_back( seg );
            pworld.TransformPoint( w, m_height - w, x, y );
            seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO );
            segments->push_back( seg );
            pworld.TransformPoint( w, w, x, y );
            seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO );
            segments->push_back( seg );
            pworld.TransformPoint( -w, -w, x, y );
            seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO_NOSTROKE, a2dPATHSEG_END_CLOSED );
            segments->push_back( seg );
        }
        else
        {
            double x, y;
            pworld.TransformPoint( 0, 0, x, y );
            a2dVpathSegment* seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
            segments->push_back( seg );
            pworld.TransformPoint( 0 , m_height, x, y );
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
            segments->push_back( seg );
            pworld.TransformPoint( m_width , m_height, x, y );
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
            segments->push_back( seg );
            pworld.TransformPoint( m_width , 0, x, y );
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED );
            segments->push_back( seg );
        }
    }

    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}


bool a2dRect::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        if ( ! HasProperty( PROPID_AllowRounding ) )
            PROPID_AllowRounding->SetPropertyToObject( this, true );

        bool allowrotation = PROPID_Allowrotation->GetPropertyValue( this );
        bool allowskew = PROPID_Allowskew->GetPropertyValue( this );
        bool allowsrounding = PROPID_AllowRounding->GetPropertyValue( this );

        if ( editmode == 1 || editmode == 2 )
        {
            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            //add properties and handles as children in order to edit the object.
            double rx = m_radius;
            m_radiusScale = 1;
            if ( m_radius != 0 )
                m_radiusScale = wxMin( fabs( m_width ), fabs( m_height ) ) / m_radius;
            if ( m_width < 0 )
                rx = -rx;

            a2dHandle* handle = NULL;
            if ( allowsrounding )
            {
                handle = new a2dHandle( this, -rx, 0, wxT( "__rounding__" ),
                                        Round( GetHabitat()->GetHandle()->GetWidth() * 2 ),
                                        Round( GetHabitat()->GetHandle()->GetHeight() * 2 ), 0, GetHabitat()->GetHandle()->GetHeight() );
                handle->SetLayer( m_layer );
                Append( handle );
                handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
            }
            double radius = m_radius;
            m_radius = 0;
            double x, y, w, h;
            x = 0;
            y = 0;
            w = m_width;
            h = m_height;
            m_radius = radius;

            a2dRect* around = new a2dRect( x, y, w, h );
            around->SetName( "around" );
            around->SetStroke( *wxRED, 0, a2dSTROKE_DOT_DASH );
            around->SetFill( *a2dTRANSPARENT_FILL );
            around->SetPreRenderAsChild( false );
            around->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
            around->SetHitFlags( a2dCANOBJ_EVENT_NON );
            around->SetLayer( m_layer );
            Append( around );

            Append( handle = new a2dHandle( this, x, y, wxT( "handle1" ) ) );
            handle->SetLayer( m_layer );
            Append( handle = new a2dHandle( this, x, y + h, wxT( "handle2" ) ) );
            handle->SetLayer( m_layer );
            Append( handle = new a2dHandle( this, x + w, y + h , wxT( "handle3" ) ) );
            handle->SetLayer( m_layer );
            Append( handle = new a2dHandle( this, x + w, y, wxT( "handle4" ) ) );
            handle->SetLayer( m_layer );
            Append( handle = new a2dHandle( this, x, y + h / 2 , wxT( "handle12" ) ) );
            handle->SetLayer( m_layer );
            Append( handle = new a2dHandle( this, x + w / 2, y + h, wxT( "handle23" ) ) );
            handle->SetLayer( m_layer );
            Append( handle = new a2dHandle( this, x + w, y + h / 2 , wxT( "handle34" ) ) );
            handle->SetLayer( m_layer );
            Append( handle = new a2dHandle( this, x + w / 2, y, wxT( "handle41" ) ) );
            handle->SetLayer( m_layer );

            if ( allowrotation )
            {
                a2dSLine* rotline = new a2dSLine( x + w / 2, y + h / 2, x + w * 3 / 4, y + h / 2 );
                rotline->SetName( "rotline" );
                rotline->SetStroke( *wxRED, 0 );
                rotline->SetPreRenderAsChild( false );
                rotline->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                rotline->SetHitFlags( a2dCANOBJ_EVENT_NON );
                Append( rotline );
                rotline->SetLayer( m_layer );

                Append( handle = new a2dHandle( this, x + w * 3 / 4, y + h / 2, wxT( "rotate" ) ) );
                handle->SetLayer( m_layer );
            }
            if ( allowskew )
            {
                Append( handle = new a2dHandle( this, x + w * 3 / 4, y + h, wxT( "skewx" ) ) );
                handle->SetLayer( m_layer );
                Append( handle = new a2dHandle( this, x + w, y + h * 3 / 4, wxT( "skewy" ) ) );
                handle->SetLayer( m_layer );
            }

            m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );
            m_childobjects->SetSpecificFlags( true, a2dCanvasOFlags::BIN2, wxT( "a2dHandle" ) );

            //calculate bbox's else mouse events may take place when first idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );
            //stil set it pending to do the redraw ( in place )
            SetPending( true );
            return true;
        }
        else
        {
            PROPID_IncludeChildren->SetPropertyToObject( this, false );
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dWH::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

#define timesSign(a) ( ( (a) < 0 )  ?  -1   : 1 ) 

void a2dRect::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();

        a2dHandle* draghandle = event.GetCanvasHandle();

        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );

        if ( editmode == 1 || editmode == 2 )
        {
            double xw, yw;
            xw = event.GetX();
            yw = event.GetY();

            a2dAffineMatrix atWorld = ic->GetTransform();
            a2dAffineMatrix inverse = ic->GetInverseTransform();
          
            // size smaller makes handles on top of eachother, and difficult later on.
            double minsize = fabs( ic->GetDrawer2D()->DeviceToWorldXRel( a2dCanvasGlobals->GetHabitat()->GetHandleSize() )) * 1;

            double xwi;
            double ywi;
            inverse.TransformPoint( xw, yw, xwi, ywi );

            a2dRect* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dRect );

            double xmin, ymin, xmax, ymax, w, h;
            xmin = 0;
            ymin = 0;
            xmax = w = m_width;
            ymax = h = m_height;

            a2dAffineMatrix origworld = m_lworld;
            double x1, y1, x2, y2;

            if ( event.GetMouseEvent().LeftDown() )
            {
            }
            else if ( event.GetMouseEvent().LeftUp() )
            {
		        a2dCommandProcessor* cmd = m_root->GetCommandProcessor();
                a2dCommandGroup* commandgroup = cmd->CommandGroupBegin( wxT( "rectangle edit" ) );
                cmd->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_TransformMatrix, m_lworld ) );
                cmd->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_Width, m_width ) );
                cmd->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_Height, m_height ) );
                cmd->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_Radius, m_radius ) );
                cmd->CommandGroupEnd( commandgroup );
            }
            else if ( event.GetMouseEvent().Dragging() )
            {

                //wxLogDebug( "handle name: %s", draghandle->GetName() );

                double factorx = 1;
                double factory = 1;
                if ( restrictEngine )
                {
                    if ( draghandle->GetName() == "rotate" )
                    {
                        double xr, yr;
                        m_lworld.TransformPoint( xmin + w / 2, ymin + h / 2, xr, yr );
                        restrictEngine->SetRestrictPoint( xr, yr );
                        restrictEngine->RestrictPoint( xw, yw,  a2dRestrictionEngine::snapToPointAngle );
                    }
                    else
                        restrictEngine->RestrictPoint( xw, yw );
                }
                ic->GetInverseTransform().TransformPoint( xw, yw, xwi, ywi );

                if ( draghandle->GetName() == wxT( "handle1" ) )
                {                 
                    //modify object
                    double dx, dy;
                    dx = xwi - xmin;
                    dy = ywi - ymin;

                    if ( w )
                        m_width -= dx;

                    if ( h )
                        m_height -= dy;

                    if ( fabs( m_width ) <= minsize )
                    { m_width = minsize * timesSign( m_width ); dx = 0; }

                    if ( fabs( m_height ) <= minsize )
                    { m_height = minsize * timesSign( m_height ) ; dy = 0; }

                    origworld.TransformPoint( -dx, -dy, x2, y2 );
                    m_lworld.TransformPoint( 0, 0, x1, y1 );
                    Translate( x1 - x2, y1 - y2 );

                    if ( m_radius && ! m_fixedRadius && editmode == m_editModeScaleRadius )
                        m_radius = wxMin( fabs( m_width ), fabs( m_height ) ) /  m_radiusScale;
                }
                else if ( draghandle->GetName() == wxT( "handle2" ) )
                {
                    //modify object
                    double dx, dy;
                    dx = xwi - xmin;
                    dy = ywi - ymax;
   
                    if ( w )
                        m_width -= dx;

                    if ( h )
                        m_height += dy;

                    if ( fabs( m_width ) <= minsize )
                    { m_width = minsize * timesSign( m_width ); dx = 0; }

                    if ( fabs( m_height ) <= minsize )
                    { m_height = minsize * timesSign( m_height ) ; dy = 0; }

                    origworld.TransformPoint( dx, ymax+dy, x2, y2 );
                    m_lworld.TransformPoint( 0, ymax+dy, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );

                    if ( m_radius && ! m_fixedRadius && editmode == m_editModeScaleRadius )
                        m_radius = wxMin( fabs( m_width ), fabs( m_height ) ) /  m_radiusScale;
                }
                else if ( draghandle->GetName() == wxT( "handle3" ) )
                {
                    //modify object
                    double dx, dy;
                    dx = xwi - xmax;
                    dy = ywi - ymax;
   
                    if ( w )
                        m_width += dx;

                    if ( h )
                        m_height += dy;

                    if ( fabs( m_width ) <= minsize )
                    { m_width = minsize * timesSign( m_width ); dx = 0; }

                    if ( fabs( m_height ) <= minsize )
                    { m_height = minsize * timesSign( m_height ) ; dy = 0; }

                    if ( m_radius && ! m_fixedRadius && editmode == m_editModeScaleRadius )
                        m_radius = wxMin( fabs( m_width ), fabs( m_height ) ) /  m_radiusScale;
                }
                else if ( draghandle->GetName() == wxT( "handle4" ) )
                {
                    //modify object
                    double dx, dy;
                    dx = xwi - xmax;
                    dy = ywi - ymin;
   
                    if ( w )
                        m_width += dx;

                    if ( h )
                        m_height -= dy;

                    if ( fabs( m_width ) <= minsize )
                    { m_width = minsize * timesSign( m_width ); dx = 0; }

                    if ( fabs( m_height ) <= minsize )
                    { m_height = minsize * timesSign( m_height ) ; dy = 0; }

                    m_lworld.TransformPoint( xmax, ymin+dy, x2, y2 );
                    m_lworld.TransformPoint( xmax, ymin, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );

                    if ( m_radius && ! m_fixedRadius && editmode == m_editModeScaleRadius )
                        m_radius = wxMin( fabs( m_width ), fabs( m_height ) ) /  m_radiusScale;
                }
                else if ( draghandle->GetName() == wxT( "rotate" ) )
                {
                    double xr, yr;
                    m_lworld.TransformPoint( xmin + w / 2, ymin + h / 2, xr, yr );

                    //modify object
                    double dx, dy;

                    dx = xw - xr;
                    dy = yw - yr;
                    double angn;
                    if ( !dx && !dy )
                        angn = 0;
                    else
                        angn = wxRadToDeg( atan2( dy, dx ) );

                    m_lworld = m_lworld.Rotate( angn - m_lworld.GetRotation(), xr, yr );
                }
                else if ( draghandle->GetName() == wxT( "skewx" ) )
                {
                    //modify object
                    double dx, dy;

                    dx = xwi - ( xmin + w * 3 / 4 );
                    dy = ywi - ( ymin + h / 2 );

                    origworld.TransformPoint( xmin + w / 2, ymin + h / 2, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    SkewX( wxRadToDeg( atan2( dx, dy ) ) );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmin + w / 2, ymin + h / 2, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else if ( draghandle->GetName() == wxT( "skewy" ) )
                {
                    //modify object
                    double dx, dy;

                    dx = xwi - ( xmin + w / 2 );
                    dy = ywi - ( ymin + h * 3 / 4 );

                    origworld.TransformPoint( xmin + w / 2, ymin + h / 2, x2, y2 );

                    //reset matrix to identity
                    SetTransformMatrix();
                    SkewY( wxRadToDeg( atan2( dy, dx ) ) );
                    Transform( origworld );
                    m_lworld.TransformPoint( xmin + w / 2, ymin + h / 2, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                }
                else if ( draghandle->GetName() == wxT( "handle12" ) )
                {
                    //modify object
                    double dx, dy;
                    dx = xwi - xmin;
                    dy = ywi - ymax;
   
                    if ( w )
                        m_width -= dx;

                    if ( fabs( m_width ) <= minsize )
                    { m_width = minsize * timesSign( m_width ); dx = 0; }

                    m_lworld.TransformPoint( dx, ymax/2, x2, y2 );
                    m_lworld.TransformPoint( 0, ymax/2, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                    if ( m_radius && ! m_fixedRadius && editmode == m_editModeScaleRadius )
                        m_radius = wxMin( fabs( m_width ), fabs( m_height ) ) /  m_radiusScale;                }
                else if ( draghandle->GetName() == wxT( "handle23" ) )
                {
                    //modify object
                    double dy;
                    dy = ywi - ymax;

                    if ( h )
                        m_height += dy;

                    if ( fabs( m_height ) <= minsize )
                    { m_height = minsize * timesSign( m_height ) ; dy = 0; }

                    if ( m_radius && ! m_fixedRadius && editmode == m_editModeScaleRadius )
                        m_radius = wxMin( fabs( m_width ), fabs( m_height ) ) /  m_radiusScale;
                }
                else if ( draghandle->GetName() == wxT( "handle34" ) )
                {
                    //modify object
                    double dx;
                    dx = xwi - xmax;

                    if ( w )
                        m_width += dx;

                    if ( fabs( m_width ) <= minsize )
                    { m_width = minsize * timesSign( m_width ); dx = 0; }

                    if ( m_radius && ! m_fixedRadius && editmode == m_editModeScaleRadius )
                        m_radius = wxMin( fabs( m_width ), fabs( m_height ) ) /  m_radiusScale;
                }
                else if ( draghandle->GetName() == wxT( "handle41" ) )
                {
                    //modify object
                    double dx, dy;
                    dx = xwi - xmax;
                    dy = ywi - ymin;
   
                    if ( h )
                        m_height -= dy;

                    if ( fabs( m_height ) <= minsize )
                    { m_height = minsize * timesSign( m_height ) ; dy = 0; }

                    m_lworld.TransformPoint( xmax/2, ymin+dy, x2, y2 );
                    m_lworld.TransformPoint( xmax/2, ymin, x1, y1 );
                    Translate( x2 - x1, y2 - y1 );
                    if ( m_radius && ! m_fixedRadius && editmode == m_editModeScaleRadius )
                        m_radius = wxMin( fabs( m_width ), fabs( m_height ) ) /  m_radiusScale;
                }
                else if ( draghandle->GetName() == wxT( "__rounding__" ) )
                {
                    //modify object
                    double dx;
                    dx = xwi - xmin;

                    if ( ( m_width < 0 && dx < 0 ) || ( m_width > 0 && dx > 0 ) )
                    {
                        //inward
                        //limit to half of width or height
                        m_radius = -fabs( -dx );
                        if ( m_radius > fabs( m_width / 2 ) && m_radius < fabs( m_height / 2 ) )
                        {
                            m_radius = - wxMin( fabs( m_width / 2 ), fabs( m_height / 2 ) ) ;
                            SetPending( true );
                        }
                    }
                    else
                    {
                        if ( m_width < 0 )
                        {
                            m_radius = fabs(dx);
                            SetPending( true );
                        }
                        else
                        {
                            m_radius = fabs(dx);
                            SetPending( true );
                        }
                    }
                    m_radiusScale = 1;
                    if ( m_radius != 0 )
                        m_radiusScale = wxMin( fabs( m_width ), fabs( m_height ) ) / m_radius;
                }
                else
                    event.Skip();

                SetPending( true );

                double x, y, w, h;
                x = 0;
                y = 0;
                w = m_width;
                h = m_height;
                
                a2dRect* around = wxDynamicCast( m_childobjects->Find( "around" ), a2dRect );
                if ( around )
                {
                    around->SetWidth( m_width );
                    around->SetHeight( m_height );
                }
                bool allowrotation = PROPID_Allowrotation->GetPropertyValue( this );
                bool allowsizing = PROPID_Allowsizing->GetPropertyValue( this );
                bool allowskew = PROPID_Allowskew->GetPropertyValue( this );

                a2dHandle* handle = NULL;
                if ( allowsizing )
                {
                    a2dCanvasObject* handle = m_childobjects->Find( "handle1" );
                    handle->SetPosXY( x, y );
                    handle = m_childobjects->Find( "handle2" );
                    handle->SetPosXY( x, y + h );                    
                    handle = m_childobjects->Find( "handle3" );
                    handle->SetPosXY( x + w, y + h);                    
                    handle = m_childobjects->Find( "handle4" );
                    handle->SetPosXY( x + w, y );                    
                    handle = m_childobjects->Find( "handle12" );
                    handle->SetPosXY( x, y + h / 2 );
                    handle = m_childobjects->Find( "handle23" );
                    handle->SetPosXY( x + w / 2, y + h );
                    handle = m_childobjects->Find( "handle34" );
                    handle->SetPosXY( x + w, y + h / 2 );
                    handle = m_childobjects->Find( "handle41" );
                    handle->SetPosXY( x + w / 2, y );
                }

                if ( allowrotation )
                {
                    a2dCanvasObject* handle = m_childobjects->Find( "rotate" );
                    handle->SetPosXY(x + w * 3 / 4, y + h / 2 );
                    a2dSLine* line = wxStaticCast( m_childobjects->Find( "rotline" ), a2dSLine );
                    line->SetPosXY12(x + w / 2, y + h / 2, x + w * 3 / 4, y + h / 2 );

                }
                if ( allowskew )
                {
                    a2dCanvasObject* handle = m_childobjects->Find( "skewx" );
                    handle->SetPosXY( x + w * 3 / 4, y + h );
                    handle = m_childobjects->Find( "skewy" );
                    handle->SetPosXY( x + w, y + h * 3 / 4 );
                }

                a2dCanvasObject* handleRadius = m_childobjects->Find( "__rounding__" );
                if ( handleRadius )
                {
                    if ( m_radius < 0 )
                    {
                        //inward
                        //limit to half of width or height
                        if ( fabs( m_radius ) > wxMin( fabs( m_width / 2 ), fabs( m_height / 2 ) ) )
                        {
                            m_radius = - wxMin( fabs( m_width / 2 ), fabs( m_height / 2 ) ) ;
                        }
                        if ( m_width < 0 )
                            handleRadius->SetPosXY( m_radius, 0 );
                        else
                            handleRadius->SetPosXY( - m_radius, 0 );
                    }
                    else
                    {
                        if ( m_width < 0 )
                            handleRadius->SetPosXY( m_radius, 0 );
                        else
                            handleRadius->SetPosXY( - m_radius, 0 );
                    }
                }
            }
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

a2dBoundingBox a2dRect::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    bbox.Expand( 0, 0 );
    bbox.Expand( m_width , m_height );

    if ( !( flags & a2dCANOBJ_BBOX_EDIT ) )
    {
        if ( m_radius > 0 )
            bbox.Enlarge( m_radius );

        if ( m_contourwidth > 0 )
            bbox.Enlarge( m_contourwidth / 2 );
    }
    return bbox;
}

void a2dRect::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( !m_shapeIdsValid )
    {
        ic.GetDrawer2D()->DeleteShapeIds( m_shapeIds );
        m_shapeIds = a2dListId();
    }

    if ( m_shapeIds.m_offset )
    {
        ic.GetDrawer2D()->DrawShape( m_shapeIds );
    }
    else
    {
        if ( m_contourwidth )
        {
            a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths( false );

            forEachIn( a2dCanvasObjectList, vectorpaths )
            {
                a2dVectorPath* obj = wxStaticCast( ( *iter ).Get(), a2dVectorPath );
                obj->DoRender( ic, clipparent );
            }
            delete vectorpaths;
        }
        else
        {
            if ( m_radius <= 0 )
            {   //inwards
                if ( m_flags.m_editingCopy )
                    ic.GetDrawer2D()->DrawRoundedRectangle( 0, 0, m_width, m_height, -m_radius );
                else
                    ic.GetDrawer2D()->DrawRoundedRectangle( 0, 0, m_width, m_height, -m_radius );
            }
            else
            {
                double outwradiusw = m_radius;
                double outwradiush = m_radius;
                //outwards
                if ( m_width < 0 )
                    outwradiusw = -outwradiusw;
                if ( m_height < 0 )
                    outwradiush = -outwradiush;

                ic.GetDrawer2D()->DrawRoundedRectangle( -outwradiusw, -outwradiush, m_width + 2 * outwradiusw, m_height + 2 * outwradiush, m_radius );
            }
        }
        m_shapeIds = ic.GetDrawer2D()->GetShapeIds();
        m_shapeIdsValid = true;
    }

}

#if wxART2D_USE_CVGIO

void a2dRect::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dWH::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_radius != 0.0 )
        {
            out.WriteAttribute( wxT( "radius" ), m_radius  * out.GetScale() );
            if ( m_contourwidth )
                out.WriteAttribute( wxT( "contourwidth" ), m_contourwidth * out.GetScale() );
        }
    }
    else
    {
    }
}

void a2dRect::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dWH::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_radius = parser.GetAttributeValueDouble( wxT( "radius" ) );
        m_contourwidth = parser.GetAttributeValueDouble( wxT( "contourwidth" ), 0 );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dRect::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double margin = fabs( ic.GetTransformedHitMargin() );

    double xmin = wxMin( 0 , m_width );
    double ymin = wxMin( 0 , m_height );
    double xmax = wxMax( 0 , m_width );
    double ymax = wxMax( 0 , m_height );

    if ( m_radius > 0 )
    {
        xmin -= m_radius;
        ymin -= m_radius;
        xmax += m_radius;
        ymax += m_radius;
    }

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, xmin, ymin, xmax, ymax, ic.GetWorldStrokeExtend() + margin + m_contourwidth );

    return hitEvent.m_how.IsHit();
}

bool a2dRect::GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double WXUNUSED( x ), double WXUNUSED( y ), double WXUNUSED(margin) )
{
    a2dPinClass* toCreate;
    if ( GetGeneratePins() && ( toCreate = toConnectTo->GetPinClassForTask( task, this ) ) )
    {
        m_flags.m_visiblechilds = true;
        a2dPin* newPin = NULL;
        if ( !HasPinNamed( wxT( "pinc*" ), true ) )
        {
            newPin = AddPin( wxT( "pinc" ), m_width / 2, m_height / 2, a2dPin::temporaryObjectPin, toCreate );
            newPin->SetInternal( true );
        }
        if ( !HasPinNamed( wxT( "pin1" ), true ) )
            AddPin( wxT( "pin1" ), 0, 0, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin2" ), true ) )
            AddPin( wxT( "pin2" ), 0, m_height / 2, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin3" ), true ) )
            AddPin( wxT( "pin3" ), 0, m_height, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin4" ), true ) )
            AddPin( wxT( "pin4" ), m_width / 2, m_height , a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin5" ), true ) )
            AddPin( wxT( "pin5" ), m_width, m_height , a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin6" ), true ) )
            AddPin( wxT( "pin6" ), m_width, m_height / 2 , a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin7" ), true ) )
            AddPin( wxT( "pin7" ), m_width, 0, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin8" ), true ) )
            AddPin( wxT( "pin8" ), m_width / 2, 0, a2dPin::temporaryObjectPin, toCreate );
        /*
                Prepend( new a2dPin( this, wxT("pin1"), toCreate, 0, 0, -225 ));
                Prepend( new a2dPin( this, wxT("pin2"), toCreate, 0, m_height/2, 180 ));
                Prepend( new a2dPin( this, wxT("pin3"), toCreate, 0, m_height, -135 ));
                Prepend( new a2dPin( this, wxT("pin4"), toCreate, m_width/2, m_height, -90 ));
                Prepend( new a2dPin( this, wxT("pin5"), toCreate, m_width, m_height, -45 ));
                Prepend( new a2dPin( this, wxT("pin6"), toCreate, m_width, m_height/2, 0 ));
                Prepend( new a2dPin( this, wxT("pin7"), toCreate, m_width, 0, 45 ));
                Prepend( new a2dPin( this, wxT("pin8"), toCreate, m_width/2, 0, 90 ));
        */

        wxASSERT( HasPins( true ) );
        m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dPin" ) );
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// a2dCircle
//----------------------------------------------------------------------------

a2dPropertyIdDouble* a2dCircle::PROPID_Radius = NULL;

BEGIN_EVENT_TABLE( a2dCircle, a2dCanvasObject )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dCircle::OnHandleEvent )
END_EVENT_TABLE()

INITIALIZE_PROPERTIES( a2dCircle, a2dCanvasObject )
{
    PROPID_Radius = new a2dPropertyIdDouble( wxT( "Radius" ),
            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dCircle::m_radius ) );
    AddPropertyId( PROPID_Radius );

    return true;
}

a2dCircle::a2dCircle()
    : a2dCanvasObject()
{
    m_radius = 0;
    m_contourwidth = 0;
}

a2dCircle::a2dCircle( double x, double y, double radius, double width  )
    : a2dCanvasObject()
{
    m_lworld.Translate( x, y );
    m_radius = radius;
    m_contourwidth = width;
}

a2dCircle::~a2dCircle()
{
}

a2dCircle::a2dCircle( const a2dCircle& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_radius = other.m_radius;
    m_contourwidth = other.m_contourwidth;
}

a2dObject* a2dCircle::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCircle( *this, options, refs );
}

a2dVertexList* a2dCircle::GetAsVertexList( bool& returnIsPolygon ) const
{
    returnIsPolygon = true;
    a2dVertexList* pointlist = new a2dVertexList();

    a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths();
    a2dVpath* segments = wxStaticCast( vectorpaths->front().Get(), a2dVectorPath ) ->GetSegments();
    double AberArcToPoly = m_root->GetAberArcToPolyScaled();
    segments->ConvertToLines( AberArcToPoly );
    a2dLineSegment* point;
    unsigned int i;
    for ( i = 0; i < segments->size(); i++ )
    {
        a2dVpathSegment* seg = segments->Item( i );
        point = new a2dLineSegment( seg->m_x1, seg->m_y1 );
        pointlist->push_back( point );
    }

    delete vectorpaths;

    return pointlist;
}

a2dCanvasObjectList* a2dCircle::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dVpath* segments = new a2dVpath();

    if ( m_contourwidth )
    {
        double w2 = m_contourwidth / 2;
        a2dVpathSegment* seg = new a2dVpathSegment( 0, m_radius + w2, a2dPATHSEG_MOVETO );
        segments->push_back( seg );
        seg = new a2dVpathArcSegment( 0, -m_radius - w2, m_radius + w2, 0, a2dPATHSEG_ARCTO );
        segments->push_back( seg );
        seg = new a2dVpathArcSegment( 0, m_radius + w2, -m_radius - w2, 0, a2dPATHSEG_ARCTO );
        segments->push_back( seg );

        seg = new a2dVpathSegment( 0, m_radius - w2, a2dPATHSEG_LINETO_NOSTROKE );
        segments->push_back( seg );

        seg = new a2dVpathArcSegment( 0, -m_radius + w2, -m_radius + w2, 0, a2dPATHSEG_ARCTO );
        segments->push_back( seg );
        seg = new a2dVpathArcSegment( 0, m_radius - w2, m_radius - w2, 0, a2dPATHSEG_ARCTO );
        segments->push_back( seg );

        seg = new a2dVpathSegment( 0, m_radius + w2, a2dPATHSEG_LINETO_NOSTROKE, a2dPATHSEG_END_CLOSED  );
        segments->push_back( seg );
    }
    else
    {
        a2dVpathSegment* seg = new a2dVpathSegment( 0, m_radius, a2dPATHSEG_MOVETO );
        segments->push_back( seg );
        seg = new a2dVpathArcSegment( 0, -m_radius, m_radius, 0, a2dPATHSEG_ARCTO );
        segments->push_back( seg );
        seg = new a2dVpathArcSegment( 0, m_radius, -m_radius, 0, a2dPATHSEG_ARCTO, a2dPATHSEG_END_CLOSED );
        segments->push_back( seg );
    }
    segments->Transform( pworld );

    a2dVectorPath* canpath = new a2dVectorPath( segments );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );
    //canpath->SetContourWidth( GetContourWidth() );
    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}

a2dBoundingBox a2dCircle::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;

    bbox.Expand( -m_radius, -m_radius );
    bbox.Expand(  m_radius,  m_radius );

    if ( !( flags & a2dCANOBJ_BBOX_EDIT ) )
    {
        if ( m_contourwidth > 0 )
            bbox.Enlarge( m_contourwidth / 2 );
    }

    return bbox;
}

bool a2dCircle::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    // Note: This calculates something different than the mapped untransformed
    // Bounding box. The box caclulated here might be smaller.
    if ( !m_bbox.GetValid() )
    {
        double rad = m_radius + m_contourwidth / 2;

        double alphax = atan2( m_lworld.GetValue( 1, 0 ), m_lworld.GetValue( 0, 0 ) );
        double alphay = atan2( m_lworld.GetValue( 1, 1 ), m_lworld.GetValue( 0, 1 ) );

        double x, y;
        m_lworld.TransformPoint( rad * cos ( alphax ), rad * sin ( alphax ), x, y );
        m_bbox.Expand( x, y );
        m_lworld.TransformPoint( rad * -cos ( alphax ), rad * -sin ( alphax ), x, y );
        m_bbox.Expand( x, y );

        m_lworld.TransformPoint( rad * cos ( alphay ), rad * sin ( alphay ), x, y );
        m_bbox.Expand( x, y );
        m_lworld.TransformPoint( rad * -cos ( alphay ), rad * -sin ( alphay ), x, y );
        m_bbox.Expand( x, y );

        m_shapeIdsValid = false;

        return true;
    }

    return false;
}

void a2dCircle::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( !m_shapeIdsValid )
    {
        ic.GetDrawer2D()->DeleteShapeIds( m_shapeIds );
        m_shapeIds = a2dListId();
    }

    if ( m_contourwidth )
    {
        a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths( false );

        forEachIn( a2dCanvasObjectList, vectorpaths )
        {
            a2dVectorPath* obj = wxStaticCast( ( *iter ).Get(), a2dVectorPath );
            obj->DoRender( ic, clipparent );
        }
        delete vectorpaths;
    }
    else
    {
        if ( m_shapeIds.m_offset )
        {
            ic.GetDrawer2D()->DrawShape( m_shapeIds );
        }
        else
        {
            ic.GetDrawer2D()->DrawCircle( 0, 0, m_radius );
            m_shapeIds = ic.GetDrawer2D()->GetShapeIds();
            m_shapeIdsValid = true;
        }
    }
}

#if wxART2D_USE_CVGIO
void a2dCircle::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "radius" ), m_radius * out.GetScale() );
        if ( m_contourwidth )
            out.WriteAttribute( wxT( "contourwidth" ), m_contourwidth * out.GetScale() );
    }
    else
    {
    }
}

void a2dCircle::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_radius = parser.RequireAttributeValueDouble( wxT( "radius" ) ) * parser.GetScale() ;
        m_contourwidth = parser.GetAttributeValueDouble( wxT( "contourwidth" ), 0 ) * parser.GetScale() ;
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dCircle::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();
    double r = sqrt( ( hitEvent.m_relx * hitEvent.m_relx ) + ( hitEvent.m_rely * hitEvent.m_rely ) );

    if ( m_contourwidth )
    {
        //donut hittest
        if ( m_radius + pw + m_contourwidth / 2 + margin > r )
        {
            //we are inside the donut its outer contour circle
            if ( m_radius - m_contourwidth / 2 - pw - margin > r )
            {
                hitEvent.m_how = a2dHit::stock_nohit;
                return false;
            }
            //we are outside the donut inner contour circle

            if ( m_radius + m_contourwidth / 2 < r )
                hitEvent.m_how = a2dHit::stock_strokeoutside;
            else if ( m_radius - m_contourwidth / 2 < r )
            {
                if ( m_radius + m_contourwidth / 2 - pw - margin > r &&
                        m_radius - m_contourwidth / 2 + pw + margin < r
                   )
                    hitEvent.m_how = a2dHit::stock_fill;
                else
                    hitEvent.m_how = a2dHit::stock_strokeinside;
            }
            else
                hitEvent.m_how = a2dHit::stock_strokeoutside;

            return true;
        }
        else
        {
            hitEvent.m_how = a2dHit::stock_nohit;
            return false;
        }
    }
    else
    {
        if ( m_radius + pw + margin > r )
        {
            if ( m_radius - pw - margin > r )
                hitEvent.m_how = a2dHit::stock_fill;
            else if ( m_radius > r )
                hitEvent.m_how = a2dHit::stock_strokeinside;
            else
                hitEvent.m_how = a2dHit::stock_strokeoutside;
            wxMouseEvent* mouse = wxDynamicCast( hitEvent.m_event, wxMouseEvent );
            /* KH debug
                        if (mouse && mouse->LeftDown())
                        {
                            if ( m_flags.m_editingCopy )
                                wxLogDebug(wxT("  inedit %p %s"), this, GetName() );
                            else
                                wxLogDebug(wxT("  in %p %s"), this, GetName() );
                        }
            */
            return true;
        }
        else
        {
            // KH debug wxLogDebug(wxT(" out %p %s"), this, GetName() );
            hitEvent.m_how = a2dHit::stock_nohit;
            return false;
        }
    }
}

bool a2dCircle::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        if ( editmode == 1 )
        {
            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            a2dHandle* handle = NULL;
            Append( handle = new a2dHandle( this, m_radius, 0, wxT( "__radius__" ) ) );
            handle->SetLayer( m_layer );
            handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );

            a2dRect* around = new a2dRect( -m_radius, -m_radius, 2*m_radius, 2*m_radius );
            around->SetName( "around" );
            around->SetStroke( *wxRED, 0, a2dSTROKE_DOT_DASH );
            around->SetFill( *a2dTRANSPARENT_FILL );
            around->SetPreRenderAsChild( false );
            around->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
            around->SetHitFlags( a2dCANOBJ_EVENT_NON );
            around->SetLayer( m_layer );
            Append( around );
            m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );
            m_childobjects->SetSpecificFlags( true, a2dCanvasOFlags::BIN2, wxT( "a2dHandle" ) );

            //calculate bbox's else mouse events may take place when first idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );
            //stil set it pending to do the redraw ( in place )
            SetPending( true );
            return true;
        }
        else
        {
            PROPID_IncludeChildren->SetPropertyToObject( this, false );
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );;
        }
    }

    return false;
}

void a2dCircle::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();

        a2dHandle* draghandle = event.GetCanvasHandle();

        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );

        if ( editmode == 1  )
        {
            double xw, yw;
            xw = event.GetX();
            yw = event.GetY();

            a2dAffineMatrix atWorld = ic->GetTransform();
            a2dAffineMatrix inverse = ic->GetInverseTransform();
          
            double xwi;
            double ywi;
            inverse.TransformPoint( xw, yw, xwi, ywi );

            a2dCircle* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dCircle );

            a2dAffineMatrix origworld = m_lworld;

            if ( event.GetMouseEvent().LeftDown() )
            {
            }
            else if ( event.GetMouseEvent().LeftUp() )
            {
		        a2dCommandProcessor* cmd = m_root->GetCommandProcessor();
                a2dCommandGroup* commandgroup = cmd->CommandGroupBegin( wxT( "circle edit" ) );
                cmd->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_TransformMatrix, m_lworld ) );
                cmd->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_Radius, m_radius ) );
                cmd->CommandGroupEnd( commandgroup );
            }
            else if ( event.GetMouseEvent().Dragging() )
            {
                double dx, dy;
                dx = xwi;
                dy = ywi;

                //wxLogDebug( "handle name: %s", draghandle->GetName() );

                if ( restrictEngine )
                {
                     restrictEngine->RestrictPoint( xw, yw );
                }
                ic->GetInverseTransform().TransformPoint( xw, yw, xwi, ywi );
                dx = xwi;
                dy = ywi;

                if ( draghandle->GetName() == wxT( "__radius__" ) )
                {                 
                    //modify object
                    m_radius = sqrt( dx*dx + dy*dy );
                    if ( m_radius < 0 )
                        draghandle->SetPosXY( dx, dy );
                    else
                        draghandle->SetPosXY( dx, dy );
                }
                else
                    event.Skip();

                SetPending( true );
               
                a2dRect* around = wxDynamicCast( m_childobjects->Find( "around" ), a2dRect );
                if ( around )
                {
                    around->SetPosXY( -m_radius, -m_radius );
                    around->SetWidth( m_radius*2 );
                    around->SetHeight( m_radius*2 );
                }
            }
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}


bool a2dCircle::GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double WXUNUSED( x ), double WXUNUSED( y ), double WXUNUSED(margin) )
{
    a2dPinClass* toCreate;
    if ( GetGeneratePins() && ( toCreate = toConnectTo->GetPinClassForTask( task, this ) ) )
    {
        double radsrt2 = sqrt( wxSqr( m_radius / 2 ) + wxSqr( m_radius / 2 ) );
        m_flags.m_visiblechilds = true;
        a2dPin* newPin = NULL;
        if ( !HasPinNamed( wxT( "pinc*" ), true ) )
        {
            newPin = AddPin( wxT( "pinc" ), 0, 0, a2dPin::temporaryObjectPin, toCreate );
            newPin->SetInternal( true );
        }
        if ( !HasPinNamed( wxT( "pin1" ) ) )
            AddPin( wxT( "pin1" ), 0, m_radius, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin2" ) ) )
            AddPin( wxT( "pin2" ), radsrt2, radsrt2, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin3" ) ) )
            AddPin( wxT( "pin3" ), m_radius, 0, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin4" ) ) )
            AddPin( wxT( "pin4" ), radsrt2, -radsrt2 , a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin5" ) ) )
            AddPin( wxT( "pin5" ), 0, -m_radius , a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin6" ) ) )
            AddPin( wxT( "pin6" ), -radsrt2, -radsrt2 , a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin7" ) ) )
            AddPin( wxT( "pin7" ), -m_radius, 0, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin8" ) ) )
            AddPin( wxT( "pin8" ), -radsrt2, radsrt2, a2dPin::temporaryObjectPin, toCreate );

        wxASSERT( HasPins( true ) );
        m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dPin" ) );

        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// a2dEllipse
//----------------------------------------------------------------------------

a2dEllipse::a2dEllipse()
    : a2dWHCenter()
{
    m_contourwidth = 0;
}

a2dEllipse::a2dEllipse( double xc, double yc, double width, double height, double contourwidth )
    : a2dWHCenter( xc, yc, width, height )
{
    m_contourwidth = contourwidth;
}

a2dEllipse::~a2dEllipse()
{
}

a2dEllipse::a2dEllipse( const a2dEllipse& other, CloneOptions options, a2dRefMap* refs )
    : a2dWHCenter( other, options, refs )
{
    m_contourwidth = other.m_contourwidth;
}

a2dObject* a2dEllipse::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dEllipse( *this, options, refs );
};

a2dVertexList* a2dEllipse::GetAsVertexList( bool& returnIsPolygon ) const
{
    returnIsPolygon = true;
    a2dVertexList* pointlist = new a2dVertexList();

    a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths();
    a2dVpath* segments = wxStaticCast( vectorpaths->front().Get(), a2dVectorPath ) ->GetSegments();
    double AberArcToPoly = m_root->GetAberArcToPolyScaled();
    segments->ConvertToLines( AberArcToPoly );
    a2dLineSegment* point;
    unsigned int i;
    for ( i = 0; i < segments->size(); i++ )
    {
        a2dVpathSegment* seg = segments->Item( i );
        point = new a2dLineSegment( seg->m_x1, seg->m_y1 );
        pointlist->push_back( point );
    }

    delete vectorpaths;

    return pointlist;
}

a2dCanvasObjectList* a2dEllipse::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    //TODO how to approximate ellipse with spline

    a2dVpath* segments = new a2dVpath();

    double dphi = wxPI * 2.0 / CIRCLE_STEPS;

    a2dVpathSegment* seg;
    double x, y;
    double theta;
    int i;
    for ( i = 0; i < CIRCLE_STEPS; i++ )
    {
        theta = i * dphi;
        pworld.TransformPoint( m_width / 2 * cos ( theta ), -m_height / 2 * sin ( theta ), x, y );
        if ( i == 0 )
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
        else if ( i == CIRCLE_STEPS - 1  )
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED );
        else
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
        segments->push_back( seg );
    }

    a2dVectorPath* canpath = new a2dVectorPath( segments );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );
    canpath->SetContourWidth( GetContourWidth() );
    canpath->SetPathType( a2dPATH_END_ROUND );
    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}

a2dBoundingBox a2dEllipse::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;

    bbox.Expand( -m_width / 2, -m_height / 2 );
    bbox.Expand(  m_width / 2,  m_height / 2 );

    if ( !( flags & a2dCANOBJ_BBOX_EDIT ) )
    {
        if ( m_contourwidth > 0 )
            bbox.Enlarge( m_contourwidth / 2 );
    }

    return bbox;
}

bool a2dEllipse::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    // Note: This calculates something different than the mapped untransformed
    // Bounding box. The box caclulated here might be smaller.
    if ( !m_bbox.GetValid() )
    {
        double width = m_width + m_contourwidth / 2;
        double height = m_height + m_contourwidth / 2;

        double alphax = atan2( m_lworld.GetValue( 1, 0 ) * height, m_lworld.GetValue( 0, 0 ) * width );
        double alphay = atan2( m_lworld.GetValue( 1, 1 ) * height, m_lworld.GetValue( 0, 1 ) * width );

        double x, y;
        m_lworld.TransformPoint( width / 2 * cos ( alphax ), height / 2 * sin ( alphax ), x, y );
        m_bbox.Expand( x, y );
        m_lworld.TransformPoint( width / 2 * -cos ( alphax ), height / 2 * -sin ( alphax ), x, y );
        m_bbox.Expand( x, y );

        m_lworld.TransformPoint( width / 2 * cos ( alphay ), height / 2 * sin ( alphay ), x, y );
        m_bbox.Expand( x, y );
        m_lworld.TransformPoint( width / 2 * -cos ( alphay ), height / 2 * -sin ( alphay ), x, y );
        m_bbox.Expand( x, y );

        return true;
    }
    return false;
}

void a2dEllipse::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( GetContourWidth() )
    {
        a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths( false );

        forEachIn( a2dCanvasObjectList, vectorpaths )
        {
            a2dVectorPath* obj = wxStaticCast( ( *iter ).Get(), a2dVectorPath );
            obj->DoRender( ic, clipparent );
        }
        delete vectorpaths;
    }
    else
        ic.GetDrawer2D()->DrawEllipse( 0, 0, m_width, m_height );
}

#if wxART2D_USE_CVGIO
void a2dEllipse::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dWHCenter::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_contourwidth )
            out.WriteAttribute( wxT( "contourwidth" ), m_contourwidth * out.GetScale() );
    }
    else
    {
    }
}

void a2dEllipse::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dWHCenter::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_contourwidth = parser.GetAttributeValueDouble( wxT( "contourwidth" ), 0 ) * parser.GetScale() ;
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dEllipse::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double xh = hitEvent.m_relx;
    double yh = hitEvent.m_rely;

    double pw =  ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

    double a = 0.5 * m_width + pw + margin ;
    double b = 0.5 * m_height + pw + margin;
    double c = wxSqr( xh / a ) + wxSqr( yh / b );
    if ( 1 > c )
    {
        a = 0.5 * m_width - pw - margin ;
        b = 0.5 * m_height - pw - margin;
        c = wxSqr( xh / a ) + wxSqr( yh / b );

        if( 1 > c )
        {
            hitEvent.m_how = a2dHit::stock_fill;
            return true;
        }

        a = 0.5 * m_width;
        b = 0.5 * m_height;
        c = wxSqr( xh / a ) + wxSqr( yh / b );

        if( 1 > c )
        {
            hitEvent.m_how = a2dHit::stock_strokeinside;
            return true;
        }
        else
        {
            hitEvent.m_how = a2dHit::stock_strokeoutside;
            return true;
        }

    }
    else
    {
        hitEvent.m_how = a2dHit::stock_nohit;
        return false;
    }
}

bool a2dEllipse::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        PROPID_IncludeChildren->SetPropertyToObject( this, false );
        return a2dWHCenter::DoStartEdit( editmode, editstyle );
    }

    return false;
}

bool a2dEllipse::GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double WXUNUSED( x ), double WXUNUSED( y ), double WXUNUSED(margin) )
{
    a2dPinClass* toCreate;
    if ( GetGeneratePins() && ( toCreate = toConnectTo->GetPinClassForTask( task, this ) ) )
    {
        double rx2 = m_width / 2 * cos( wxDegToRad( 45 ) );
        double ry2 = m_height / 2 * sin( wxDegToRad( 45 ) );

        m_flags.m_visiblechilds = true;
        a2dPin* newPin = NULL;
        if ( !HasPinNamed( wxT( "pinc*" ), true ) )
        {
            newPin = AddPin( wxT( "pinc" ), 0, 0, a2dPin::temporaryObjectPin, toCreate );
            newPin->SetInternal( true );
        }
        if ( !HasPinNamed( wxT( "pin1" ) ) )
            AddPin( wxT( "pin1" ), 0, m_height / 2, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin2" ) ) )
            AddPin( wxT( "pin2" ), rx2, ry2, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin3" ) ) )
            AddPin( wxT( "pin3" ), m_width / 2, 0, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin4" ) ) )
            AddPin( wxT( "pin4" ), rx2, -ry2, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin5" ) ) )
            AddPin( wxT( "pin5" ), 0, -m_height / 2, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin6" ) ) )
            AddPin( wxT( "pin6" ), -rx2, -ry2 , a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin7" ) ) )
            AddPin( wxT( "pin7" ), -m_width / 2, 0, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin8" ) ) )
            AddPin( wxT( "pin8" ), -rx2, ry2, a2dPin::temporaryObjectPin, toCreate );

        wxASSERT( HasPins( true ) );
        m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dPin" ) );

        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// a2dEllipticArc
//----------------------------------------------------------------------------
a2dPropertyIdDouble* a2dEllipticArc::PROPID_StartAngle = NULL;
a2dPropertyIdDouble* a2dEllipticArc::PROPID_EndAngle = NULL;
a2dPropertyIdBool* a2dEllipticArc::PROPID_Chord = NULL;

INITIALIZE_PROPERTIES( a2dEllipticArc, a2dWHCenter )
{
    PROPID_StartAngle = new a2dPropertyIdDouble( wxT( "StartAngle" ),
            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dEllipticArc::m_start ) );
    AddPropertyId( PROPID_StartAngle );
    PROPID_EndAngle = new a2dPropertyIdDouble( wxT( "EndAngle" ),
            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dEllipticArc::m_end ) );
    AddPropertyId( PROPID_EndAngle );
    PROPID_Chord = new a2dPropertyIdBool( wxT( "Radius" ),
                                          a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdBool::Mptr >( &a2dEllipticArc::m_chord ) );
    AddPropertyId( PROPID_Chord );

    return true;
}

BEGIN_EVENT_TABLE( a2dEllipticArc, a2dWHCenter )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dEllipticArc::OnHandleEvent )
    EVT_CHAR( a2dEllipticArc::OnChar )
END_EVENT_TABLE()

a2dEllipticArc::a2dEllipticArc()
    : a2dWHCenter()
{
    m_start  = 0;
    m_end    = 0;
    m_chord  = false;
    m_contourwidth = 0;
}

a2dEllipticArc::a2dEllipticArc( double xc, double yc, double width, double height, double start, double end, bool chord, double contourwidth )
    : a2dWHCenter( xc, yc, width, height )
{
    m_start  = start;
    m_end    = end;
    m_chord  = chord;
    m_contourwidth = contourwidth;
}

a2dEllipticArc::~a2dEllipticArc()
{
}

a2dEllipticArc::a2dEllipticArc( const a2dEllipticArc& other, CloneOptions options, a2dRefMap* refs )
    : a2dWHCenter( other, options, refs )
{
    m_start = other.m_start;
    m_end = other.m_end;
    m_chord = other.m_chord;
    m_contourwidth = other.m_contourwidth;
}

void a2dEllipticArc::SetChord( bool chord )
{
    m_chord = chord;
    SetPending( true );
}

a2dObject* a2dEllipticArc::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dEllipticArc( *this, options, refs );
};

a2dVertexList* a2dEllipticArc::GetAsVertexList( bool& returnIsPolygon ) const
{
    returnIsPolygon = true;
    a2dVertexList* pointlist = new a2dVertexList();

    a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths();
    a2dVpath* segments = wxStaticCast( vectorpaths->front().Get(), a2dVectorPath ) ->GetSegments();
    double AberArcToPoly = m_root->GetAberArcToPolyScaled();
    segments->ConvertToLines( AberArcToPoly );
    a2dLineSegment* point;
    unsigned int i;
    for ( i = 0; i < segments->size(); i++ )
    {
        a2dVpathSegment* seg = segments->Item( i );
        point = new a2dLineSegment( seg->m_x1, seg->m_y1 );
        pointlist->push_back( point );
    }

    delete vectorpaths;

    return pointlist;
}

a2dCanvasObjectList* a2dEllipticArc::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    //TODO how to approximate ellipse with spline

    a2dVpath* segments = new a2dVpath();

    double start, end;

    start = wxDegToRad( m_start );
    end = wxDegToRad( m_end );

    end = end - start;
    if ( end <= 0 )   end += 2.0 * wxPI;

    if ( start < 0 )  start += 2.0 * wxPI;

    end = end + start;

    double dphi = wxPI * 2.0 / CIRCLE_STEPS;
    int steps = ( int )( ( end - start ) / dphi );
    if ( steps == 0 )
        steps = 1;
    dphi = ( end - start ) / steps; //make it fit.

    a2dVpathSegment* seg;

    double theta = start;
    int i;
    double x, y;
    for ( i = 0; i < steps + 1; i++ )
    {
        pworld.TransformPoint( m_width / 2 * cos ( theta ), m_height / 2 * sin ( theta ), x, y );
        if ( i == 0 )
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
        else
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
        segments->push_back( seg );
        theta = theta + dphi;
    }

    if ( !m_chord )
    {
        pworld.TransformPoint( 0, 0, x, y );
        seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED );
        segments->push_back( seg );
    }

    a2dVectorPath* canpath = new a2dVectorPath( segments );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );
    canpath->SetContourWidth( GetContourWidth() );
    canpath->SetPathType( a2dPATH_END_ROUND );
    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}

a2dBoundingBox a2dEllipticArc::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;

    double start, end;

    start = wxDegToRad( m_start );
    end = wxDegToRad( m_end );

    if ( !m_chord )
        bbox.Expand( 0, 0 );
    //start point
    bbox.Expand( m_width / 2 * cos ( start ), m_height / 2 * sin ( start ) );
    //end point
    bbox.Expand( m_width / 2 * cos ( end ), m_height / 2 * sin ( end ) );

    //outer points
    //if ( InArc( 0, start, end, false ) )
        bbox.Expand( m_width / 2, 0 );
    //if ( InArc( wxPI / 2, start, end, false ) )
        bbox.Expand( 0, m_height / 2 );
    //if ( InArc( wxPI, start, end, false ) )
        bbox.Expand( -m_width / 2, 0 );
    //if ( InArc( 1.5 * wxPI, start, end, false ) )
        bbox.Expand( 0, -m_height / 2 );

    if ( !( flags & a2dCANOBJ_BBOX_EDIT ) )
    {
        if ( m_contourwidth > 0 )
            bbox.Enlarge( m_contourwidth / 2 );
    }

    return bbox;
}

bool a2dEllipticArc::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    // Note: This calculates something different than the mapped untransformed
    // Bounding box. The box caclulated here might be smaller.
    if ( !m_bbox.GetValid() )
    {
        double width = m_width + m_contourwidth / 2;
        double height = m_height + m_contourwidth / 2;

        double alphax = atan2( m_lworld.GetValue( 1, 0 ) * height, m_lworld.GetValue( 0, 0 ) * width );
        double alphay = atan2( m_lworld.GetValue( 1, 1 ) * height, m_lworld.GetValue( 0, 1 ) * width );

        double x, y;

        double start, end;

        start = wxDegToRad( m_start );
        end = wxDegToRad( m_end );

        //wxLogDebug(wxT("start = %12.6lf"), start );
        //wxLogDebug(wxT("end = %12.6lf"), end);
        //wxLogDebug(wxT("alphax = %12.6lf"), alphax);
        //wxLogDebug(wxT("alphay = %12.6lf"), alphay);

        //start point
        m_lworld.TransformPoint( width / 2 * cos ( start ), height / 2 * sin ( start ), x, y );
        m_bbox.Expand( x, y );
        //end point
        m_lworld.TransformPoint( width / 2 * cos ( end ), height / 2 * sin ( end ), x, y );
        m_bbox.Expand( x, y );

        if ( !m_chord )
        {
            m_lworld.TransformPoint( 0, 0, x, y );
            m_bbox.Expand( x, y );
        }

        //outer points
        if ( InArc( alphax, start, end, false ) )
        {
            m_lworld.TransformPoint( width / 2 * cos ( alphax ), height / 2 * sin ( alphax ), x, y );
            m_bbox.Expand( x, y );
        }
        alphax += wxPI;
        if ( InArc( alphax, start, end, false ) )
        {
            m_lworld.TransformPoint( width / 2 * cos ( alphax ), height / 2 * sin ( alphax ), x, y );
            m_bbox.Expand( x, y );
        }
        alphax -= 2 * wxPI;
        if ( InArc( alphax, start, end, false ) )
        {
            m_lworld.TransformPoint( width / 2 * cos ( alphax ), height / 2 * sin ( alphax ), x, y );
            m_bbox.Expand( x, y );
        }
        if ( InArc( alphay, start, end, false ) )
        {
            m_lworld.TransformPoint( width / 2 * cos ( alphay ), height / 2 * sin ( alphay ), x, y );
            m_bbox.Expand( x, y );
        }
        alphay += wxPI;
        if ( InArc( alphay, start, end, false ) )
        {
            m_lworld.TransformPoint( width / 2 * cos ( alphay ), height / 2 * sin ( alphay ), x, y );
            m_bbox.Expand( x, y );
        }
        alphay -= 2 * wxPI;
        if ( InArc( alphay, start, end, false ) )
        {
            m_lworld.TransformPoint( width / 2 * cos ( alphay ), height / 2 * sin ( alphay ), x, y );
            m_bbox.Expand( x, y );
        }

        return true;
    }
    return false;
}

void a2dEllipticArc::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( GetContourWidth() )
    {
        if ( m_flags.m_editingCopy && m_chord )
            m_chord = false;

        a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths( false );

        if ( m_flags.m_editingCopy && m_chord )
            m_chord = true;

        forEachIn( a2dCanvasObjectList, vectorpaths )
        {
            a2dVectorPath* obj = wxStaticCast( ( *iter ).Get(), a2dVectorPath );
            obj->DoRender( ic, clipparent );
        }
        delete vectorpaths;
    }
    else
    {
        if ( m_flags.m_editingCopy && m_chord )
        {
            m_chord = false;
            ic.GetDrawer2D()->DrawEllipticArc( 0, 0 , m_width, m_height, m_start, m_end, m_chord );
            m_chord = true;
        }
        else
            ic.GetDrawer2D()->DrawEllipticArc( 0, 0 , m_width, m_height, m_start, m_end, m_chord );
    }
}

#if wxART2D_USE_CVGIO
void a2dEllipticArc::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dWHCenter::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "start" ), m_start );
        out.WriteAttribute( wxT( "end" ), m_end );
        out.WriteAttribute( wxT( "chord" ), m_chord );
        if ( m_contourwidth )
            out.WriteAttribute( wxT( "contourwidth" ), m_contourwidth * out.GetScale() );
    }
    else
    {
    }
}

void a2dEllipticArc::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dWHCenter::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_start = parser.GetAttributeValueDouble( wxT( "start" ) );
        m_end = parser.GetAttributeValueDouble( wxT( "end" ) );
        m_chord = parser.GetAttributeValueBool( wxT( "chord" ) );
        m_contourwidth = parser.GetAttributeValueDouble( wxT( "contourwidth" ), 0 ) * parser.GetScale() ;
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dEllipticArc::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double xh = hitEvent.m_relx;
    double yh = hitEvent.m_rely;

    if ( m_height == 0 && m_width == 0 )
    {
        hitEvent.m_how = a2dHit::stock_nohit;
        return false;
    }

    if ( m_height == 0 || m_width == 0 )
    {
        hitEvent.m_how = a2dHit::stock_nohit;
        return false;
    }

    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

    double a = m_width / 2 + pw + margin ;
    double b = m_height / 2 + pw + margin;
    double c = wxSqr( xh / a ) + wxSqr( yh / b );
    if ( 1 > c )
    {
        double dx = xh;
        double dy = yh;

        double start, end, angle;

        start = wxDegToRad( m_start );
        end = wxDegToRad( m_end );

        double rx, ry;
        double c = m_height / m_width;

        rx = sqrt( dx * dx + ( dy * dy ) / ( c * c ) );
        ry = rx * c;

        if ( dx* ry == 0 && dy* rx == 0 )
            angle = 0;
        else
            angle = atan2( dy * rx, dx * ry );

        angle = angle - start;
        end = end - start;

        if ( angle <= 0 ) angle += 2.0 * wxPI;

        if ( end <= 0 )   end += 2.0 * wxPI;
        if ( start < 0 )  start += 2.0 * wxPI;


        angle = angle + start;
        end = end + start;

        if ( angle > start && angle < end )

        {
            hitEvent.m_how = a2dHit::stock_fill;
            return true;
        }
    }

    hitEvent.m_how = a2dHit::stock_nohit;
    return false;
}

bool a2dEllipticArc::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        PROPID_IncludeChildren->SetPropertyToObject( this, false );
        if ( editmode == 1 )
        {
            double rx = m_width / 2 ;
            double ry = m_height / 2 ;

            double xs, ys, xe, ye ;
            xs = rx * cos ( wxDegToRad( m_start ) ) ;
            ys = ry * sin ( wxDegToRad( m_start ) ) ;
            xe = rx * cos ( wxDegToRad( m_end ) ) ;
            ye = ry * sin ( wxDegToRad( m_end ) ) ;


            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            a2dHandle* handle = NULL;
            Append( handle = new a2dHandle( this, xs * 3 / 4, ys * 3 / 4 , wxT( "startangle" ) ) );
            handle->SetLayer( m_layer );
            Append( handle = new a2dHandle( this, xe * 3 / 4, ye * 3 / 4 , wxT( "endangle" ) ) );
            handle->SetLayer( m_layer );

            m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );
            m_childobjects->SetSpecificFlags( true, a2dCanvasOFlags::BIN2, wxT( "a2dHandle" ) );
        }
        return a2dWHCenter::DoStartEdit( editmode, editstyle );
    }

    return false;
}

void a2dEllipticArc::OnChar( wxKeyEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_TAB:
            {
                wxUint16* editmode = PROPID_Editmode->GetPropertyValuePtr( this );
                if ( editmode )
                {
                    ( *editmode )++;
                    if ( ( *editmode ) > 1 ) ( *editmode ) = 0;
                    ReStartEdit( ( *editmode ) );
                }
                break;
            }
            default:
                event.Skip();
        }
    }
    else
        event.Skip();
}

void a2dEllipticArc::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        a2dRestrictionEngine* restrict = GetHabitat()->GetRestrictionEngine();

        a2dHandle* draghandle = event.GetCanvasHandle();

        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        a2dAffineMatrix atWorld = ic->GetTransform();
        a2dAffineMatrix inverse = ic->GetInverseTransform();

        double xwi;
        double ywi;
        inverse.TransformPoint( xw, yw, xwi, ywi );

        a2dEllipticArc* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dEllipticArc );

        if ( event.GetMouseEvent().LeftUp() )
        {
            if ( draghandle->GetName() == wxT( "startangle" ) )
            {
                m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty(  original, PROPID_StartAngle, m_start ) );
                //original->SetStart( m_start );
            }
            else if ( draghandle->GetName() == wxT( "endangle" ) )
            {
                m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty(  original, PROPID_EndAngle, m_end ) );
                //original->SetEnd( m_end );
            }
            else
                event.Skip(); //maybe the base class handles
        }
        else if ( event.GetMouseEvent().Dragging() )
        {
            double xs, ys, xe, ye, rx, ry;

            if ( draghandle->GetName() == wxT( "startangle" ) )
            {
                //modify object
                double c;
                xs = xwi;
                ys = ywi;

                c = m_height / m_width;

                rx = sqrt( xs * xs + ( ys * ys ) / ( c * c ) );
                ry = rx * c;

                if ( xs* ry ==  0 )
                {
                    if ( ys * rx > 0 )
                        m_start = 90;
                    else
                        m_start = 270;
                }
                else
                    m_start = wxRadToDeg( atan2( ys * rx, xs * ry ) );

                if( restrict )
                {
                    double startrad = wxDegToRad( m_start );
                    restrict->RestrictAngle( &startrad, a2dRestrictionEngine::snapToPointAngle );
                    m_start = wxRadToDeg( startrad );
                }
                xs = m_width / 2  * cos( wxDegToRad( m_start ) ) ;
                ys = m_height / 2 * sin( wxDegToRad( m_start ) ) ;

                draghandle->SetPosXY( xs * 3 / 4, ys * 3 / 4 );
                SetPending( true );
            }
            else if ( draghandle->GetName() == wxT( "endangle" ) )
            {
                //modify object
                double c;
                xe = xwi;
                ye = ywi;

                c = m_height / m_width;

                rx = sqrt( xe * xe + ( ye * ye ) / ( c * c ) );
                ry = rx * c;

                if ( xe* ry == 0 )
                {
                    if ( ye * rx > 0 )
                        m_end = 90;
                    else
                        m_end = 270;
                }
                else
                    m_end = wxRadToDeg( atan2( ye * rx, xe * ry ) );

                if( restrict )
                {
                    double endrad = wxDegToRad( m_end );
                    restrict->RestrictAngle( &endrad, a2dRestrictionEngine::snapToPointAngle );
                    m_end = wxRadToDeg( endrad );
                }

                xe = m_width / 2  * cos( wxDegToRad( m_end ) ) ;
                ye = m_height / 2 * sin( wxDegToRad( m_end ) ) ;
                draghandle->SetPosXY( xe * 3 / 4, ye * 3 / 4 );
                SetPending( true );
            }
            else
                event.Skip(); //maybe on the base class handles
        }
        else
            event.Skip(); //maybe on the base class handles
    }
    else
    {
        event.Skip();
    }
}

//----------------------------------------------------------------------------
// a2dArc
//----------------------------------------------------------------------------

a2dPropertyIdDouble* a2dArc::PROPID_X1 = NULL;
a2dPropertyIdDouble* a2dArc::PROPID_X2 = NULL;
a2dPropertyIdDouble* a2dArc::PROPID_Y1 = NULL;
a2dPropertyIdDouble* a2dArc::PROPID_Y2 = NULL;
a2dPropertyIdBool* a2dArc::PROPID_Chord = NULL;
a2dPropertyIdDouble* a2dArc::PROPID_StartAngle = NULL;
a2dPropertyIdDouble* a2dArc::PROPID_EndAngle = NULL;

INITIALIZE_PROPERTIES( a2dArc, a2dCanvasObject )
{
    PROPID_X1 = new a2dPropertyIdDouble( wxT( "X1" ),
                                         a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dArc::m_x1 ) );
    AddPropertyId( PROPID_X1 );
    PROPID_X2 = new a2dPropertyIdDouble( wxT( "X2" ),
                                         a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dArc::m_x2 ) );
    AddPropertyId( PROPID_X2 );
    PROPID_Y1 = new a2dPropertyIdDouble( wxT( "Y1" ),
                                         a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dArc::m_y1 ) );
    AddPropertyId( PROPID_Y1 );
    PROPID_Y2 = new a2dPropertyIdDouble( wxT( "Y2" ),
                                         a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dArc::m_y2 ) );
    AddPropertyId( PROPID_Y2 );
    PROPID_StartAngle = new a2dPropertyIdDouble( wxT( "StartAngle" ),
            a2dPropertyId::flag_none, 0,
            a2dPropertyIdDouble::Get( &a2dArc::GetStartAngle ),
            a2dPropertyIdDouble::Set( &a2dArc::SetStartAngle ) );
    AddPropertyId( PROPID_StartAngle );
    PROPID_EndAngle = new a2dPropertyIdDouble( wxT( "EndAngle" ),
            a2dPropertyId::flag_none, 0,
            a2dPropertyIdDouble::Get( &a2dArc::GetEndAngle ),
            a2dPropertyIdDouble::Set( &a2dArc::SetEndAngle ) );
    AddPropertyId( PROPID_EndAngle );
    PROPID_Chord = new a2dPropertyIdBool( wxT( "Chord" ),
                                          a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdBool::Mptr >( &a2dArc::m_chord ) );
    AddPropertyId( PROPID_Chord );

    return true;
}

BEGIN_EVENT_TABLE( a2dArc, a2dCanvasObject )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dArc::OnHandleEvent )
    EVT_CHAR( a2dArc::OnChar )
END_EVENT_TABLE()

a2dArc::a2dArc()
    : a2dCanvasObject()
{
    m_x1 = 0;
    m_x2 = 0;
    m_y1 = 0;
    m_y2 = 0;
    m_chord = false;
    m_contourwidth = 0;
}

a2dArc::a2dArc( double xc, double yc, double radius, double start, double end, bool chord, double contourwidth )
    : a2dCanvasObject()
{
    m_lworld.Translate( xc, yc );

    m_x1 = radius * cos ( wxDegToRad( start ) ) ;
    m_x2 = radius * cos ( wxDegToRad( end ) ) ;
    m_y1 = radius * sin ( wxDegToRad( start ) ) ;
    m_y2 = radius * sin ( wxDegToRad( end ) ) ;
    m_chord = chord;
    m_contourwidth = contourwidth;
}

a2dArc::a2dArc( double x1, double y1, double x2, double y2, double xc, double yc, bool chord, double contourwidth )
    : a2dCanvasObject()
{
    m_lworld.Translate( xc, yc );

    double dx = x1 - xc;
    double dy = y1 - yc;
    double radius = sqrt( dx * dx + dy * dy );

    m_x1 = dx;
    m_y1 = dy;

    //adjust second point to have the same radius
    dx = x2 - xc;
    dy = y2 - yc;
    double end = atan2( dy, dx );

    m_x2 = radius * cos ( end ) ;
    m_y2 = radius * sin ( end ) ;
    m_chord = chord;
    m_contourwidth = contourwidth;
}

a2dArc::~a2dArc()
{
}

void a2dArc::SetRadius( double radius )
{
    //adjust second point to have the same radius
    double dx = m_x1;
    double dy = m_y1;

    double start;

    if ( !dx && !dy )
        start = 0;
    else
        start = atan2( dy, dx );

    m_x1 = radius * cos ( start ) ;
    m_y1 = radius * sin ( start ) ;

    dx = m_x2;
    dy = m_y2;

    double end;

    if ( !dx && !dy )
        end = 0;
    else
        end = atan2( dy, dx );

    m_x2 = radius * cos ( end ) ;
    m_y2 = radius * sin ( end ) ;
}

double a2dArc::GetRadius() const
{
    return sqrt( m_x1 * m_x1 + m_y1 * m_y1 );
}

void a2dArc::SetChord( bool chord )
{
    m_chord = chord;
    SetPending( true );
}

void a2dArc::Set( double xs, double ys,  double xm, double ym, double xe, double ye )
{
    double radius;
    double center_x, center_y;
    double beginrad, midrad, endrad, phit;

    double ax, ay, bx, by, cx, cy;
    double ax2, ay2, bx2, by2, cx2, cy2;
    double d;
    bool straight = false;

    ax = xs;         ay = ys;  ax2 = pow( ax, 2 ); ay2 = pow( ay, 2 );
    bx = xm;         by = ym;  bx2 = pow( bx, 2 ); by2 = pow( by, 2 );
    cx = xe;         cy = ye;  cx2 = pow( cx, 2 ); cy2 = pow( cy, 2 );

    if ( ax == cx && ay == cy )
    {
        //assume the middle is exact 180 opposite.
        center_x = ( bx + ax ) / 2.0;
        center_y = ( by + ay ) / 2.0;
        radius = sqrt( pow( bx - ax, 2 ) + pow( by - ay, 2 ) ) / 2.0;
    }
    else
    {
        d = 2 * ( ay * cx + by * ax - by * cx - ay * bx - cy * ax + cy * bx );

        if ( fabs( d ) < a2dACCUR ) // (infinite radius)
        {
            d = a2dACCUR;
            straight = true;
        }

        center_x = (  by * ax2 - cy * ax2 - by2 * ay + cy2 * ay + bx2 * cy + ay2 * by
                      + cx2 * ay - cy2 * by - cx2 * by - bx2 * ay + by2 * cy - ay2 * cy
                   ) / d;

        center_y = (  ax2 * cx + ay2 * cx + bx2 * ax - bx2 * cx + by2 * ax - by2 * cx
                      - ax2 * bx - ay2 * bx - cx2 * ax + cx2 * bx - cy2 * ax + cy2 * bx
                   ) / d ;

        radius = sqrt( pow( ax - center_x, 2 ) + pow( ay - center_y, 2 ) );
    }

    if ( radius )
    {
        //beginrad between PI and -PI, assume counterclockwise, make endrad > beginrad.
        beginrad = atan2( ys - center_y, xs - center_x );
        midrad   = atan2( ym - center_y, xm - center_x );
        endrad   = atan2( ye - center_y, xe - center_x );

        double mr, er;

        //if going counterclockwise from begin to end we pass trough middle => counterclockwise
        //if going counterclockwise from begin to end we do NOT pass trough middle => clockwise

        //rotate such that beginrad = zero
        mr = midrad - beginrad;
        er = endrad - beginrad;

        //make positive
        if ( mr <= 0 ) mr += 2.0 * M_PI;
        if ( er <= 0 ) er += 2.0 * M_PI;

        bool counterclock;

        //middle gives direction
        if ( mr < er )
        {
            phit = er; //counterclockwise
            counterclock = true;
        }
        else
        {
            phit = -( 2.0 * M_PI - er ); //Clockwise
            counterclock = false;
        }

        if ( counterclock )
        {
            m_x1 = xs - center_x;
            m_y1 = ys - center_y;
            m_x2 = xe - center_x;
            m_y2 = ye - center_y;
        }
        else
        {
            m_x2 = xs - center_x;
            m_y2 = ys - center_y;
            m_x1 = xe - center_x;
            m_y1 = ye - center_y;
        }
    }
    else
    {
        m_x2 = center_x;
        m_y2 = center_y;
        m_x1 = center_x;
        m_y1 = center_y;
    }

    m_lworld.Translate( center_x, center_y );
}

a2dArc::a2dArc( const a2dArc& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_x1 = other.m_x1;
    m_y1 = other.m_y1;
    m_x2 = other.m_x2;
    m_y2 = other.m_y2;
    m_chord = other.m_chord;
    m_contourwidth = other.m_contourwidth;
}

a2dObject* a2dArc::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dArc( *this, options, refs );
};

double a2dArc::GetStartAngle() const
{
    double start;
    if ( !m_x1 && !m_y1 )
        start = 0;
    else
        start = atan2( m_y1, m_x1 );

    return start;
}

double a2dArc::GetEndAngle() const
{
    double end;
    if ( !m_x2 && !m_y2 )
        end = 0;
    else
        end = atan2( m_y2, m_x2 );

    return end;
}

void a2dArc::SetStartAngle( double a )
{
    double radius = GetRadius();
    m_x1 = radius * cos ( a ) ;
    m_y1 = radius * sin ( a ) ;
}

void a2dArc::SetEndAngle( double a )
{
    double radius = GetRadius();
    m_x2 = radius * cos ( a ) ;
    m_y2 = radius * sin ( a ) ;
}

a2dPoint2D a2dArc::GetMidPoint() const
{
    double dx = m_x1;

    double dy = m_y1;

    double start;
    double radius = sqrt( dx * dx + dy * dy );

    if ( !dx && !dy )
        start =  0;
    else
        start =  atan2( dy, dx );

    dx = m_x2;
    dy = m_y2;

    double end;

    if ( !dx && !dy )
        end = 0;
    else
        end = atan2( dy, dx );

    //the difference, is made end, as if start was made zero by rotation.
    end = end - start;
    if ( end <= 0 )   end += 2.0 * wxPI;

    if ( start < 0 )  start += 2.0 * wxPI;

    //rotate back
    end = end + start;

    double mid = end - ( end - start )/2.0;


    return a2dPoint2D( radius * cos( mid ), radius * sin( mid ) );
}


a2dVertexList* a2dArc::GetAsVertexList( bool& returnIsPolygon ) const
{
    returnIsPolygon = true;
    a2dVertexList* pointlist = new a2dVertexList();

    a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths();
    a2dVpath* segments = wxStaticCast( vectorpaths->front().Get(), a2dVectorPath ) ->GetSegments();
    double AberArcToPoly = m_root->GetAberArcToPolyScaled();
    segments->ConvertToLines( AberArcToPoly );
    a2dLineSegment* point;
    unsigned int i;
    for ( i = 0; i < segments->size(); i++ )
    {
        a2dVpathSegment* seg = segments->Item( i );
        point = new a2dLineSegment( seg->m_x1, seg->m_y1 );
        pointlist->push_back( point );
    }

    delete vectorpaths;

    return pointlist;
}

a2dCanvasObjectList* a2dArc::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    //TODO how to approximate ellipse with spline

    a2dVpath* segments = new a2dVpath();

    a2dVpathSegment* seg;

    double dx = m_x1;

    double dy = m_y1;

    double start;
    double radius = sqrt( dx * dx + dy * dy );

    if ( !dx && !dy )
        start =  0;
    else
        start =  atan2( dy, dx );

    dx = m_x2;
    dy = m_y2;

    double end;

    if ( !dx && !dy )
        end = 0;
    else
        end = atan2( dy, dx );

    end = end - start;
    if ( end <= 0 )   end += 2.0 * wxPI;

    if ( start < 0 )  start += 2.0 * wxPI;

    end = end + start;

    double dphi = wxPI * 2.0 / CIRCLE_STEPS;
    int steps = ( int ) ( ( end - start ) / dphi );
    if ( steps == 0 )
        steps = 1;
    dphi = ( end - start ) / steps; //make it fit.

    double theta = start;
    int i;


    double x, y;

    for ( i = 0; i < steps + 1; i++ )
    {
        pworld.TransformPoint( radius * cos ( theta ), radius * sin ( theta ), x, y );
        if ( i == 0 )
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
        else
            seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
        segments->push_back( seg );
        theta = theta + dphi;
    }


    if ( !m_chord )

    {
        pworld.TransformPoint( 0, 0, x, y );

        seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED );
        segments->push_back( seg );
    }

    a2dVectorPath* canpath = new a2dVectorPath( segments );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );
    canpath->SetContourWidth( GetContourWidth() );
    canpath->SetPathType( a2dPATH_END_ROUND );
    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}

a2dBoundingBox a2dArc::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;

    double dx = m_x1;
    double dy = m_y1;

    double start;
    double end;
    double radius = sqrt( dx * dx + dy * dy );


    if ( m_x1 == m_x2 && m_y1 == m_y2 )
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

        dx = m_x2;
        dy = m_y2;

        if ( !dx && !dy )
            end = 0;
        else
            end = atan2( dy, dx );

        end = end - start;
        if ( end <= 0 )   end += 2.0 * wxPI;
        end += start;
    }

    if ( !m_chord )
        bbox.Expand( 0, 0 );
    //start point
    bbox.Expand( radius * cos ( start ), radius * sin ( start ) );
    //end point
    bbox.Expand( radius * cos ( end ), radius * sin ( end ) );

    //outer points
    //if ( InArc( 0, start, end, false ) )
        bbox.Expand( radius, 0 );
    //if ( InArc( wxPI / 2, start, end, false ) )
        bbox.Expand( 0, radius );
    //if ( InArc( wxPI, start, end, false ) )
        bbox.Expand( -radius, 0 );
    //if ( InArc( 1.5 * wxPI, start, end, false ) )
        bbox.Expand( 0, -radius );

    if ( !( flags & a2dCANOBJ_BBOX_EDIT ) )
    {
        if ( m_contourwidth > 0 )
            bbox.Enlarge( m_contourwidth / 2 );
    }

    return bbox;
}

bool a2dArc::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    //klion canvas\src\canprim.cpp(4221) : warning C4189: 'radius' : local variable is initialized but not referenced
//    double radius = sqrt(m_x1*m_x1+m_y1*m_y1);
    // Note: This calculates something different than the mapped untransformed
    // Bounding box. The box caclulated here might be smaller.
    if ( !m_bbox.GetValid() )
    {
        double x, y;
        double dx = m_x1;
        double dy = m_y1;

        double start;
        double end;
        double radius = sqrt( dx * dx + dy * dy );
        radius += m_contourwidth / 2;

        if ( m_x1 == m_x2 && m_y1 == m_y2 )
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

            dx = m_x2;
            dy = m_y2;

            if ( !dx && !dy )
                end = 0;
            else
                end = atan2( dy, dx );
        }

        //start point
        m_lworld.TransformPoint( radius * cos ( start ), radius * sin ( start ), x, y );
        m_bbox.Expand( x, y );
        //end point
        m_lworld.TransformPoint( radius * cos ( end ), radius * sin ( end ), x, y );
        m_bbox.Expand( x, y );
/*
        //outer points
        //if ( InArc( 0, start, end, false ) )
        {
            m_lworld.TransformPoint( radius, radius, x, y );
            m_bbox.Expand( x, y );
        }
        //if ( InArc( wxPI / 2, start, end, false ) )
        {
            m_lworld.TransformPoint( -radius, radius, x, y );
            m_bbox.Expand( x, y );
        }
        //if ( InArc( wxPI, start, end, false ) )
        {
            m_lworld.TransformPoint( -radius, -radius, x, y );
            m_bbox.Expand( x, y );
        }
        //if ( InArc( 1.5 * wxPI, start, end, false ) )
        {
            m_lworld.TransformPoint( radius, -radius, x, y );
            m_bbox.Expand( x, y );
        }
*/

        
        double alphax = atan2( m_lworld.GetValue( 1, 0 ), m_lworld.GetValue( 0, 0 ) );
        double alphay = atan2( m_lworld.GetValue( 1, 1 ), m_lworld.GetValue( 0, 1 ) );

        //since that is how the filled part is defined.
        if ( end <= start )    end += 2.0 * wxPI;
        if ( alphax <= start ) alphax += wxPI;
        if ( alphay <= start ) alphay += wxPI;
        if ( alphax >= end )   alphax -= wxPI;
        if ( alphay >= end )   alphay -= wxPI;

        m_lworld.TransformPoint( 0, 0, x, y );
        m_bbox.Expand( x, y );

        //start point
        m_lworld.TransformPoint( radius * cos ( start ), radius * sin ( start ), x, y );
        m_bbox.Expand( x, y );
        //end point
        m_lworld.TransformPoint( radius * cos ( end ), radius * sin ( end ), x, y );
        m_bbox.Expand( x, y );

        //outer points
        if ( InArc( alphax, start, end, false ) )
        {
            m_lworld.TransformPoint( radius * cos ( alphax ), radius * sin ( alphax ), x, y );
            m_bbox.Expand( x, y );
        }
        alphax += wxPI;
        if ( InArc( alphax, start, end, false ) )
        {
            m_lworld.TransformPoint( radius * cos ( alphax ), radius * sin ( alphax ), x, y );
            m_bbox.Expand( x, y );
        }
        alphax -= 2 * wxPI;
        if ( InArc( alphax, start, end, false ) )
        {
            m_lworld.TransformPoint( radius * cos ( alphax ), radius * sin ( alphax ), x, y );
            m_bbox.Expand( x, y );
        }
        if ( InArc( alphay, start, end, false ) )
        {
            m_lworld.TransformPoint( radius * cos ( alphay ), radius * sin ( alphay ), x, y );
            m_bbox.Expand( x, y );
        }
        alphay += wxPI;
        if ( InArc( alphay, start, end, false ) )
        {
            m_lworld.TransformPoint( radius * cos ( alphay ), radius * sin ( alphay ), x, y );
            m_bbox.Expand( x, y );
        }
        alphay -= 2 * wxPI;
        if ( InArc( alphay, start, end, false ) )
        {
            m_lworld.TransformPoint( radius * cos ( alphay ), radius * sin ( alphay ), x, y );
            m_bbox.Expand( x, y );
        }

        return true;
    }

    return false;
}

void a2dArc::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( m_x1 == m_x2 && m_y1 == m_y2 )
        return;

    if ( GetContourWidth() )
    {
        if ( m_flags.m_editingCopy && m_chord )
            m_chord = false;

        a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths( false );

        if ( m_flags.m_editingCopy && m_chord )
            m_chord = true;

        forEachIn( a2dCanvasObjectList, vectorpaths )
        {
            a2dVectorPath* obj = wxStaticCast( ( *iter ).Get(), a2dVectorPath );
            obj->DoRender( ic, clipparent );
        }
        delete vectorpaths;
    }
    else if ( m_flags.m_editingCopy && m_chord )
    {
        m_chord = false;
        ic.GetDrawer2D()->DrawArc( m_x1, m_y1, m_x2, m_y2, 0, 0, m_chord );
        m_chord = true;
    }
    else
        ic.GetDrawer2D()->DrawArc( m_x1, m_y1, m_x2, m_y2, 0, 0, m_chord );

}

#if wxART2D_USE_CVGIO
void a2dArc::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "x1" ), m_x1 * out.GetScale()  );
        out.WriteAttribute( wxT( "y1" ), m_y1 * out.GetScale()  );
        out.WriteAttribute( wxT( "x2" ), m_x2 * out.GetScale()  );
        out.WriteAttribute( wxT( "y2" ), m_y2 * out.GetScale()  );
        out.WriteAttribute( "chord", m_chord );
        if ( m_contourwidth )
            out.WriteAttribute( wxT( "contourwidth" ), m_contourwidth * out.GetScale() );
    }
    else
    {
    }
}

void a2dArc::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_x1 = parser.GetAttributeValueDouble( wxT( "x1" ) ) * parser.GetScale() ;
        m_y1 = parser.GetAttributeValueDouble( wxT( "y1" ) ) * parser.GetScale() ;
        m_x2 = parser.GetAttributeValueDouble( wxT( "x2" ) ) * parser.GetScale() ;
        m_y2 = parser.GetAttributeValueDouble( wxT( "y2" ) ) * parser.GetScale() ;
        m_chord = parser.GetAttributeValueBool( "chord" );
        m_contourwidth = parser.GetAttributeValueDouble( wxT( "contourwidth" ), 0 ) * parser.GetScale();
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dArc::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double xh = hitEvent.m_relx;
    double yh = hitEvent.m_rely;
    double pw = ic.GetWorldStrokeExtend();

    double dx = m_x1;
    double dy = m_y1;
    double radius = sqrt( dx * dx + dy * dy );
    double margin = ic.GetTransformedHitMargin();

    if ( radius + pw + margin > sqrt( ( xh * xh ) + ( yh * yh ) ) )
    {

        dx = xh;
        dy = yh;

        double angle;

        if ( !dx && !dy )
            angle = 0;
        else
            angle = atan2( dy, dx );

        dx = m_x1;
        dy = m_y1;

        double start;

        if ( !dx && !dy )
            start = 0;
        else
            start = atan2( dy, dx );

        dx = m_x2;
        dy = m_y2;

        double end;

        if ( !dx && !dy )
            end = 0;
        else
            end = atan2( dy, dx );

        //rotate such that start = zero
        angle = angle - start;
        end = end - start;

        if ( angle <= 0 ) angle += 2.0 * wxPI;
        if ( end <= 0 )   end += 2.0 * wxPI;
        if ( angle >= 2.0 * M_PI ) angle -= 2.0 * M_PI;
        if ( end >= 2.0 * M_PI ) end -= 2.0 * M_PI;

        //middle gives direction
        if ( angle < end )
        {
            if ( m_chord &&  !m_flags.m_editable )
            {
                if ( radius < sqrt( ( xh * xh ) + ( yh * yh ) ) )
                    hitEvent.m_how = a2dHit::stock_strokeoutside;
                else if ( radius > sqrt( ( xh * xh ) + ( yh * yh ) ) && radius - pw - margin < sqrt( ( xh * xh ) + ( yh * yh ) ) )
                    hitEvent.m_how = a2dHit::stock_strokeinside;
                else
                {
                    hitEvent.m_how = a2dHit::stock_nohit;
                    return false;
                }
            }
            else
            {
                if ( radius < sqrt( ( xh * xh ) + ( yh * yh ) ) )
                    hitEvent.m_how = a2dHit::stock_strokeoutside;
                if ( radius > sqrt( ( xh * xh ) + ( yh * yh ) ) && radius - pw - margin < sqrt( ( xh * xh ) + ( yh * yh ) ) )
                    hitEvent.m_how = a2dHit::stock_strokeinside;
                else
                    hitEvent.m_how = a2dHit::stock_fill;
            }
            return true;
        }
        hitEvent.m_how = a2dHit::stock_nohit;
        return false;
    }
    else
    {
        hitEvent.m_how = a2dHit::stock_nohit;
        return false;
    }
}

bool a2dArc::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        PROPID_IncludeChildren->SetPropertyToObject( this, false );
        if ( editmode == 1 )
        {
            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            a2dHandle* handle = NULL;
            Append( handle = new a2dHandle( this, GetX1(), GetY1(), "_radius_" ) );
            handle->SetLayer( m_layer );
            Append( handle = new a2dHandle( this, GetX1() * 3 / 4, GetY1() * 3 / 4, "startangle" ) );
            handle->SetLayer( m_layer );
            Append( handle = new a2dHandle( this, GetX2() * 3 / 4, GetY2() * 3 / 4, "endangle" ) );
            handle->SetLayer( m_layer );

            double dx = m_x1;
            double dy = m_y1;
            double radius = sqrt( dx * dx + dy * dy );

            a2dRect* around = new a2dRect( -radius, -radius, 2*radius, 2*radius );
            around->SetName( "around" );
            around->SetStroke( *wxRED, 0, a2dSTROKE_DOT_DASH );
            around->SetFill( *a2dTRANSPARENT_FILL );
            around->SetPreRenderAsChild( false );
            around->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
            around->SetHitFlags( a2dCANOBJ_EVENT_NON );
            around->SetLayer( m_layer );
            Append( around );

            //calculate bbox's else mouse events may take place when first idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );
            //stil set it pending to do the redraw ( in place )
            SetPending( true );

            m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );
            m_childobjects->SetSpecificFlags( true, a2dCanvasOFlags::BIN2, wxT( "a2dHandle" ) );

            return true;
        }
        else
        {
            PROPID_IncludeChildren->SetPropertyToObject( this, false );
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

void a2dArc::OnChar( wxKeyEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_TAB:
            {
                wxUint16* editmode = PROPID_Editmode->GetPropertyValuePtr( this );
                if ( editmode )
                {
                    ( *editmode )++;
                    if ( ( *editmode ) > 1 ) ( *editmode ) = 0;
                    ReStartEdit( ( *editmode ) );
                }
                break;
            }
            default:
                event.Skip();
        }
    }
    else
        event.Skip();
}

void a2dArc::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();

        a2dHandle* draghandle = event.GetCanvasHandle();

        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );

        if ( editmode == 1  )
        {

            double xw, yw;
            xw = event.GetX();
            yw = event.GetY();

            a2dAffineMatrix atWorld = ic->GetTransform();
            a2dAffineMatrix inverse = ic->GetInverseTransform();

            double xwi;
            double ywi;
            inverse.TransformPoint( xw, yw, xwi, ywi );

            a2dArc* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dArc );

            if ( event.GetMouseEvent().LeftUp() )
            {
		        a2dCommandProcessor* cmd = m_root->GetCommandProcessor();
                a2dCommandGroup* commandgroup = cmd->CommandGroupBegin( wxT( "arc edit" ) );
                if ( draghandle->GetName() == wxT( "_radius_" ) )
                {
                    cmd->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_X1, m_x1 ) );
                    cmd->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_Y1, m_y1 ) );
                }
                else if ( draghandle->GetName() == wxT( "startangle" ) )
                {
                    double start = atan2( m_y1, m_x1 ); //angle
                    cmd->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_StartAngle, start ) );
                }
                else if ( draghandle->GetName() == wxT( "endangle" ) )
                {
                    double end = atan2( m_y2, m_x2 ); //angle
                    cmd->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_EndAngle, end ) );
                }
                cmd->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_TransformMatrix, m_lworld ) );
                cmd->CommandGroupEnd( commandgroup );
            }
            else if ( event.GetMouseEvent().Dragging() )
            {
                if ( draghandle->GetName() == "startangle" )
                {
                    //modify object
                    double dx, dy;
                    dx = xwi;
                    dy = ywi;

                    double start = atan2( dy, dx ); //angle

                    if( restrictEngine )
                    {
                        restrictEngine->RestrictAngle( &start, a2dRestrictionEngine::snapToPointAngle );
                    }

                    dx = GetX1();
                    dy = GetY1();

                    double radius = sqrt( dx * dx + dy * dy );
                    dx = radius * cos ( start );
                    dy = radius * sin ( start );
                    SetX1( dx );
                    SetY1( dy );

                    draghandle->SetPosXY( GetX1() * 3 / 4, GetY1() * 3 / 4 );
                    SetPending( true );

                    a2dHandle* radiusHandle = wxDynamicCast( m_childobjects->Find( "_radius_" ), a2dHandle );
                    radiusHandle->SetPosXY( GetX1(), GetY1() );
                }
                else if ( draghandle->GetName() == "_radius_" )
                {
                    //modify object
                    double dx, dy;
                    dx = xwi;
                    dy = ywi;

                    double radius = sqrt( dx * dx + dy * dy );

                    double start = atan2( m_y1, m_x1 ); //angle

                    dx = radius * cos ( start );
                    dy = radius * sin ( start );

                    SetX1( dx );
                    SetY1( dy );

                    draghandle->SetPosXY( GetX1(), GetY1() );
                    SetPending( true );

                    double end = atan2( m_y2, m_x2 ); //angle
                    dx = radius * cos ( end );
                    dy = radius * sin ( end );
                    SetX2( dx );
                    SetY2( dy );

                    a2dHandle* endHandle = wxDynamicCast( m_childobjects->Find( "endangle" ), a2dHandle );
                    endHandle->SetPosXY( GetX2() * 3 / 4, GetY2() * 3 / 4 );
                    a2dHandle* startHandle = wxDynamicCast( m_childobjects->Find( "startangle" ), a2dHandle );
                    startHandle->SetPosXY( GetX1() * 3 / 4, GetY1() * 3 / 4 );
                }
                else if ( draghandle->GetName() == "endangle" )
                {
                    //modify object
                    double dx, dy;
                    dx = xwi;
                    dy = ywi;

                    double end = atan2( dy, dx ); //angle

                    if( restrictEngine )
                    {
                        restrictEngine->RestrictAngle( &end, a2dRestrictionEngine::snapToPointAngle );
                    }

                    dx = GetX2();
                    dy = GetY2();

                    double radius = sqrt( dx * dx + dy * dy );
                    dx = radius * cos ( end );
                    dy = radius * sin ( end );
                    SetX2( dx );
                    SetY2( dy );

                    draghandle->SetPosXY( GetX2() * 3 / 4, GetY2() * 3 / 4 );
                    SetPending( true );
                }
                else
                    event.Skip();

                SetPending( true );
               
                a2dRect* around = wxDynamicCast( m_childobjects->Find( "around" ), a2dRect );
                if ( around )
                {
                    double dx = m_x1;
                    double dy = m_y1;
                    double radius = sqrt( dx * dx + dy * dy );
                    around->SetPosXY( -radius, -radius );
                    around->SetWidth( radius*2 );
                    around->SetHeight( radius*2 );
                }
            }
            else
                event.Skip(); //maybe on the base class handles
        }
        else
            event.Skip(); //maybe on the base class handles
    }
    else
    {
        event.Skip();
    }
}

//----------------------------------------------------------------------------
// a2dSLine
//----------------------------------------------------------------------------
a2dPropertyIdDouble* a2dSLine::PROPID_X1 = NULL;
a2dPropertyIdDouble* a2dSLine::PROPID_X2 = NULL;
a2dPropertyIdDouble* a2dSLine::PROPID_Y1 = NULL;
a2dPropertyIdDouble* a2dSLine::PROPID_Y2 = NULL;

INITIALIZE_PROPERTIES( a2dSLine, a2dCanvasObject )
{
    PROPID_X1 = new a2dPropertyIdDouble( wxT( "X1" ),
                                         a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dSLine::m_x1 ) );
    AddPropertyId( PROPID_X1 );
    PROPID_X2 = new a2dPropertyIdDouble( wxT( "X2" ),
                                         a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dSLine::m_x2 ) );
    AddPropertyId( PROPID_X2 );
    PROPID_Y1 = new a2dPropertyIdDouble( wxT( "Y1" ),
                                         a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dSLine::m_y1 ) );
    AddPropertyId( PROPID_Y1 );
    PROPID_Y2 = new a2dPropertyIdDouble( wxT( "Y2" ),
                                         a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdDouble::Mptr >( &a2dSLine::m_y2 ) );
    AddPropertyId( PROPID_Y2 );

    return true;
}

BEGIN_EVENT_TABLE( a2dSLine, a2dCanvasObject )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dSLine::OnHandleEvent )
    EVT_CHAR( a2dSLine::OnChar )
END_EVENT_TABLE()

a2dSLine::a2dSLine()
    : a2dCanvasObject()
{
    m_x1 = 0;
    m_y1 = 0;
    m_x2 = 0;
    m_y2 = 0;
    m_contourwidth = 0;
    m_pathtype = a2dPATH_END_SQAURE;
}

a2dSLine::a2dSLine( double x1, double y1, double x2, double y2, double contourwidth )
    : a2dCanvasObject()
{
    m_x1 = x1;
    m_y1 = y1;
    m_x2 = x2;
    m_y2 = y2;
    m_contourwidth = contourwidth;
    m_pathtype = a2dPATH_END_SQAURE;
}

a2dSLine::~a2dSLine()
{
}

a2dSLine::a2dSLine( const a2dSLine& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_x1 = other.m_x1;
    m_y1 = other.m_y1;
    m_x2 = other.m_x2;
    m_y2 = other.m_y2;
    m_contourwidth = other.m_contourwidth;
    m_pathtype = other.m_pathtype;
}

a2dObject* a2dSLine::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dSLine( *this, options, refs );
};

double a2dSLine::GetPosX1( bool transform ) const
{
    double x, y;
    if ( transform )
        m_lworld.TransformPoint( m_x1, m_y1, x, y );
    else
        x = m_x1;
    return x;
}

double a2dSLine::GetPosY1( bool transform ) const
{
    double x, y;
    if ( transform )
        m_lworld.TransformPoint( m_x1, m_y1, x, y );
    else
        y = m_y1;
    return y;
}

double a2dSLine::GetPosX2( bool transform ) const
{
    double x, y;
    if ( transform )
        m_lworld.TransformPoint( m_x2, m_y2, x, y );
    else
        x = m_x2;
    return x;
}

double a2dSLine::GetPosY2( bool transform ) const
{
    double x, y;
    if ( transform )
        m_lworld.TransformPoint( m_x2, m_y2, x, y );
    else
        y = m_y2;
    return y;
}

bool a2dSLine::EliminateMatrix()
{
    m_lworld.TransformPoint( m_x1, m_y1, m_x1, m_y1 );
    m_lworld.TransformPoint( m_x2, m_y2, m_x2, m_y2 );

    return a2dCanvasObject::EliminateMatrix();
}

double a2dSLine::GetLength() const
{
    double x1, y1, x2, y2;
    m_lworld.TransformPoint( m_x1, m_y1, x1, y1 );
    m_lworld.TransformPoint( m_x2, m_y2, x2, y2 );

    return sqrt( wxSqr( x2 - x1 ) + wxSqr( y2 - y1 ) );
}

void a2dSLine::SetPosXY1( double x, double y, bool afterinversion )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x, y, x, y );
    }

    m_x1 = x;
    m_y1 = y;

    SetPending( true );
}

void a2dSLine::SetPosXY2( double x, double y, bool afterinversion )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x, y, x, y );
    }

    m_x2 = x;
    m_y2 = y;

    SetPending( true );
}

void a2dSLine::SetPosXY12( double x1, double y1, double x2, double y2, bool afterinversion )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x1, y1, x1, y1 );
        inverse.TransformPoint( x2, y2, x2, y2 );
    }

    m_x1 = x1;
    m_y1 = y1;
    m_x2 = x2;
    m_y2 = y2;

    SetPending( true );
}

a2dCanvasObjectList* a2dSLine::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dVpath* segments = new a2dVpath();

    a2dVpathSegment* seg;

    seg = new a2dVpathSegment( m_x1, m_y1, a2dPATHSEG_MOVETO );
    segments->push_back( seg );
    seg = new a2dVpathSegment( m_x2, m_y2, a2dPATHSEG_LINETO, a2dPATHSEG_END_OPEN );
    segments->push_back( seg );

    segments->Transform( pworld );

    a2dVectorPath* canpath = new a2dVectorPath( segments );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );
    canpath->SetContourWidth( GetContourWidth() );
    canpath->SetPathType( a2dPATH_END_ROUND );
    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}

a2dBoundingBox a2dSLine::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    bbox.Expand( m_x1 , m_y1 );
    bbox.Expand( m_x2 , m_y2 );

    if ( !( flags & a2dCANOBJ_BBOX_EDIT ) )
    {
        if ( m_contourwidth > 0 )
            bbox.Enlarge( m_contourwidth / 2 );
    }

    return bbox;
}

void a2dSLine::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( GetContourWidth() )
    {
        a2dLine line1( m_x1, m_y1, m_x2, m_y2 );
        line1.CalculateLineParameters();
        double distance = m_contourwidth / 2.0;
        a2dPoint2D offsetpoint1left = a2dPoint2D( m_x1, m_y1 );
        a2dPoint2D offsetpoint1right = a2dPoint2D( m_x1, m_y1 );
        a2dPoint2D offsetpoint2left = a2dPoint2D( m_x2, m_y2 );
        a2dPoint2D offsetpoint2right = a2dPoint2D( m_x2, m_y2 );
        line1.Virtual_Point( offsetpoint1left, distance );
        line1.Virtual_Point( offsetpoint1right, -distance );
        line1.Virtual_Point( offsetpoint2left, distance );
        line1.Virtual_Point( offsetpoint2right, -distance );

        a2dVertexArray contour;
        contour.AddPoint( offsetpoint1left );
        contour.AddPoint( offsetpoint2left );
        contour.AddPoint( offsetpoint2right );
        contour.AddPoint( offsetpoint1right );

        ic.GetDrawer2D()->DrawPolygon( &contour );
    }
    else
        ic.GetDrawer2D()->DrawLine(  m_x1, m_y1, m_x2, m_y2 );
}

bool a2dSLine::RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld )
{
    // object has disabled snap?
    if ( !m_flags.m_snap_to )
        return false;

    bool res = false;
    if ( snapToWhat & a2dRestrictionEngine::snapToStart )
    {
        double x, y;
        m_lworld.TransformPoint( m_x1, m_y1, x, y );

        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( x, y ), bestPointSofar, thresHoldWorld );
    }
    if ( snapToWhat & a2dRestrictionEngine::snapToEnd )
    {
        double x, y;
        m_lworld.TransformPoint( m_x2, m_y2, x, y );

        //if within the threshold, do snap to pin position.
        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( x, y ), bestPointSofar, thresHoldWorld );
    }

    res |= a2dCanvasObject::RestrictToObject( ic, pointToSnapTo, bestPointSofar, snapToWhat, thresHoldWorld );

    return res;
}

#if wxART2D_USE_CVGIO
void a2dSLine::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "x1" ), m_x1 * out.GetScale()  );
        out.WriteAttribute( wxT( "y1" ), m_y1 * out.GetScale()  );
        out.WriteAttribute( wxT( "x2" ), m_x2 * out.GetScale()  );
        out.WriteAttribute( wxT( "y2" ), m_y2 * out.GetScale()  );
        if ( m_contourwidth )
            out.WriteAttribute( wxT( "contourwidth" ), m_contourwidth * out.GetScale() );
    }
    else
    {
    }
}

void a2dSLine::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_x1 = parser.GetAttributeValueDouble( wxT( "x1" ) ) * parser.GetScale() ;
        m_y1 = parser.GetAttributeValueDouble( wxT( "y1" ) ) * parser.GetScale() ;
        m_x2 = parser.GetAttributeValueDouble( wxT( "x2" ) ) * parser.GetScale() ;
        m_y2 = parser.GetAttributeValueDouble( wxT( "y2" ) ) * parser.GetScale() ;
        m_contourwidth = parser.GetAttributeValueDouble( wxT( "contourwidth" ), 0 ) * parser.GetScale() ;
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dSLine::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    if ( m_x1 == m_x2 && m_y1 == m_y2 )
    {
        return fabs( hitEvent.m_relx - m_x1 ) < ic.GetWorldStrokeExtend() + ic.GetTransformedHitMargin() &&
               fabs( hitEvent.m_rely - m_y1 ) < ic.GetWorldStrokeExtend() + ic.GetTransformedHitMargin();
    }

    a2dLine line1( m_x1, m_y1, m_x2, m_y2 );
    a2dPoint2D P = a2dPoint2D( hitEvent.m_relx, hitEvent.m_rely );

    double distance;
    if ( line1.PointInLine( P, distance, m_contourwidth / 2.0 + ic.GetWorldStrokeExtend() + ic.GetTransformedHitMargin() ) == R_IN_AREA )
    {
        hitEvent.m_how = a2dHit::stock_strokeoutside;
        return true;
    }
    else
    {
        hitEvent.m_how = a2dHit::stock_nohit;
        return false;
    }
}

bool a2dSLine::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        if ( editmode == 1 )
        {
            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            a2dHandle* handle = NULL;
            Append( handle = new a2dHandle( this, m_x1, m_y1 , wxT( "__startpoint__" ) ) );
            handle->SetLayer( m_layer );
            Append( handle = new a2dHandle( this, m_x2, m_y2 , wxT( "__endpoint__" ) ) );
            handle->SetLayer( m_layer );

            m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );
            m_childobjects->SetSpecificFlags( true, a2dCanvasOFlags::BIN2, wxT( "a2dHandle" ) );

            //calculate bbox's elase mouse events may take place when first idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );
            //stil set it pending to do the redraw ( in place )
            SetPending( true );
            return true;
        }
        else
        {
            PROPID_IncludeChildren->SetPropertyToObject( this, false );
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

void a2dSLine::OnChar( wxKeyEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_TAB:
            {
                wxUint16* editmode = PROPID_Editmode->GetPropertyValuePtr( this );
                if ( editmode )
                {
                    ( *editmode )++;
                    if ( ( *editmode ) > 1 ) ( *editmode ) = 0;
                    ReStartEdit( ( *editmode ) );
                }
                break;
            }
            default:
                event.Skip();
        }
    }
    else
        event.Skip();
}


void a2dSLine::OnHandleEvent( a2dHandleMouseEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();

        a2dHandle* draghandle = event.GetCanvasHandle();

        //to world group coordinates to do hit test in world group coordinates
        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        double xwi;
        double ywi;
        a2dAffineMatrix inverse = m_lworld;
        inverse.Invert();
        inverse.TransformPoint( xw, yw, xwi, ywi );

        a2dSLine* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dSLine );
        a2dAffineMatrix origworld = m_lworld;

        if ( event.GetMouseEvent().LeftUp() )
        {
            if ( draghandle->GetName() == wxT( "__startpoint__" ) )
            {
                m_root->GetCommandProcessor()->Submit( new a2dCommand_MovePoint( original, m_x1, m_y1, true, false ) );
                draghandle->SetPosXY( m_x1, m_y1 );
            }
            else if ( draghandle->GetName() == wxT( "__endpoint__" ) )
            {
                m_root->GetCommandProcessor()->Submit( new a2dCommand_MovePoint( original, m_x2, m_y2, false, false ) );
                draghandle->SetPosXY( m_x2, m_y2 );
            }
            else
                event.Skip(); //maybe the base class handles
        }
        else if ( event.GetMouseEvent().Dragging() )
        {
            if ( draghandle->GetName() == wxT( "__startpoint__" ) )
            {
                //modify object
                if ( restrictEngine )
                    restrictEngine->RestrictPoint( xw, yw );
                SetPosXY1( xw, yw );
                draghandle->SetPosXY( m_x1, m_y1 );
                SetPending( true );
            }
            else if ( draghandle->GetName() == wxT( "__endpoint__" ) )
            {
                //modify object
                if ( restrictEngine )
                    restrictEngine->RestrictPoint( xw, yw );
                SetPosXY2( xw, yw );
                draghandle->SetPosXY( m_x2, m_y2 );
                SetPending( true );
            }
            else
                event.Skip(); //maybe on the base class handles
        }
        else
            event.Skip(); //maybe on the base class handles
    }
    else
    {
        event.Skip();
    }
}

//----------------------------------------------------------------------------
// a2dEndsLine
//----------------------------------------------------------------------------

a2dEndsLine::a2dEndsLine()
    : a2dSLine()
{
    m_xscale = m_yscale = 1;
    m_takeStyleToArrow = false;
}

a2dEndsLine::a2dEndsLine( double x1, double y1, double x2, double y2, double contourwidth )
    : a2dSLine( x1, y1, x2, y2, contourwidth )
{
    m_xscale = m_yscale = 1;
    m_takeStyleToArrow = false;
}

a2dEndsLine::a2dEndsLine( const a2dSLine& other, CloneOptions options, a2dRefMap* refs )
    : a2dSLine( other, options, refs )
{
    m_xscale = m_yscale = 1;
    m_takeStyleToArrow = false;
}

a2dEndsLine::a2dEndsLine( const a2dEndsLine& other, CloneOptions options, a2dRefMap* refs )
    : a2dSLine( other, options, refs )
{
    if ( options & clone_members )
    {
        if ( other.m_begin )
            m_begin = other.m_begin->TClone( options );
        if ( other.m_end )
            m_end = other.m_end->TClone( options );
    }
    else
    {
        m_begin = other.m_begin;
        m_end = other.m_end;
    }

    m_xscale = other.m_xscale;
    m_yscale = other.m_yscale;
    m_takeStyleToArrow = other.m_takeStyleToArrow;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in other. .. ->Clone( options )()
    CurrentSmartPointerOwner = this;
#endif
}

a2dObject* a2dEndsLine::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dEndsLine( *this, options, refs );
}

a2dEndsLine::~a2dEndsLine()
{
}

void a2dEndsLine::SetBegin( a2dCanvasObject* begin )
{
    SetPending( true );
    m_begin = begin;
    if ( m_begin && m_root  )
        m_begin->SetRoot( m_root );
}

void a2dEndsLine::SetEnd( a2dCanvasObject* end )
{
    SetPending( true );
    m_end = end;
    if ( m_end && m_root )
        m_end->SetRoot( m_root );
}

a2dBoundingBox a2dEndsLine::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    bbox = a2dSLine::DoGetUnTransformedBbox( flags );

    if ( m_begin )
    {
        double dx, dy;
        dx = m_x2 - m_x1;
        dy = m_y2 - m_y1;
        double ang1;
        if ( !dx && !dy )
            ang1 = 0;
        else
            ang1 = wxRadToDeg( atan2( dy, dx ) );

        a2dAffineMatrix lworld;
        //clockwise rotation so minus
        lworld.Scale( m_xscale, m_yscale, 0, 0 );
        lworld.Rotate( -ang1 );
        lworld.Translate( m_x1, m_y1 );

        bbox.Expand( m_begin->GetMappedBbox( lworld ) );
    }

    if ( m_end )
    {
        double dx, dy;
        dx = m_x2 - m_x1;
        dy = m_y2 - m_y1;
        double ang2;
        if ( !dx && !dy )
            ang2 = 0;
        else
            ang2 = wxRadToDeg( atan2( -dy, -dx ) );

        a2dAffineMatrix tworld;
        //clockwise rotation so minus
        tworld.Scale( m_xscale, m_yscale, 0, 0 );
        tworld.Rotate( -ang2 );
        tworld.Translate( m_x2, m_y2 );

        bbox.Expand( m_end->GetMappedBbox( tworld ) );
    }
    return bbox;
}

bool a2dEndsLine::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( m_begin )
    {
        if ( m_takeStyleToArrow )
        {
            m_begin->SetFill( GetFill() );
            m_begin->SetStroke( GetStroke() );
        }
        calc = m_begin->Update( mode );
    }
    if ( m_end )
    {
        if ( m_takeStyleToArrow )
        {
            m_end->SetFill( GetFill() );
            m_end->SetStroke( GetStroke() );
        }
        calc = m_end->Update( mode ) || calc;
    }
    if ( !m_bbox.GetValid() || calc )
    {
        m_bbox.SetValid( false );
        m_bbox = DoGetUnTransformedBbox();

        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return calc;
}

void a2dEndsLine::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    a2dSLine::DoRender( ic, clipparent );

    if ( m_begin )
    {
        double dx, dy;
        dx = m_x2 - m_x1;
        dy = m_y2 - m_y1;
        double ang1;
        if ( !dx && !dy )
            ang1 = 0;
        else
            ang1 = wxRadToDeg( atan2( dy, dx ) );

        a2dAffineMatrix lworld;
        //clockwise rotation so minus
        lworld.Scale( m_xscale, m_yscale, 0, 0 );
        lworld.Rotate( -ang1 );
        lworld.Translate( m_x1, m_y1 );
        a2dIterCU cu( ic, lworld );

        //The next switching of render layer is oke, imagine the begin and end objects
        //as normal child objects of the this line, in combination with the flag,
        //the effect is the same as starting a second layer iteration. Only this
        //one is optimized.
        bool old = m_begin->GetChildrenOnSameLayer();
        m_begin->SetChildrenOnSameLayer( true ); //
        m_begin->Render( ic, clipparent );
        m_begin->SetChildrenOnSameLayer( old ); //
    }

    if ( m_end )
    {
        double dx, dy;
        dx = m_x2 - m_x1;
        dy = m_y2 - m_y1;
        double ang2;
        if ( !dx && !dy )
            ang2 = 0;
        else
            ang2 = wxRadToDeg( atan2( -dy, -dx ) );

        a2dAffineMatrix tworld;
        //clockwise rotation so minus
        tworld.Scale( m_xscale, m_yscale, 0, 0 );
        tworld.Rotate( -ang2 );
        tworld.Translate( m_x2, m_y2 );

        a2dIterCU cu( ic, tworld );

        bool old = m_end->GetChildrenOnSameLayer();
        m_end->SetChildrenOnSameLayer( true );
        m_end->Render( ic, clipparent );
        m_end->SetChildrenOnSameLayer( old );
    }
}

#if wxART2D_USE_CVGIO
void a2dEndsLine::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dSLine::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_begin || m_end )
        {
            out.WriteAttribute( wxT( "xscale" ), m_xscale * out.GetScale()  );
            out.WriteAttribute( wxT( "yscale" ), m_yscale * out.GetScale()  );
            if ( m_begin )
                out.WriteAttribute( wxT( "begin" ), m_begin->GetId() );
            if ( m_end )
                out.WriteAttribute( wxT( "end" ), m_end->GetId() );
        }
    }
    else
    {
        if ( ( m_begin && !m_begin->GetCheck() ) ||
                ( m_end && !m_end->GetCheck() )
           )
        {
            //trick the system to have multiple refs on those objects, else we will not get an id attribute
            a2dCanvasObjectPtr begin = m_begin;
            a2dCanvasObjectPtr end = m_end;

            out.WriteStartElement( wxT( "derived" ) );

            if ( m_begin && !m_begin->GetCheck() )
                m_begin->Save( this, out, towrite );
            if ( m_end  && !m_end->GetCheck() )
                m_end->Save( this, out, towrite );

            out.WriteEndElement();
        }
    }
}

void a2dEndsLine::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dSLine::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_xscale = parser.GetAttributeValueDouble( wxT( "xscale" ) ) * parser.GetScale() ;
        m_yscale = parser.GetAttributeValueDouble( wxT( "yscale" ) ) * parser.GetScale() ;

        wxString resolveKey;
        if ( parser.HasAttribute( wxT( "begin" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_begin, parser.GetAttributeValue( wxT( "begin" ) ) );
        }
        if ( parser.HasAttribute( wxT( "end" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_end, parser.GetAttributeValue( wxT( "end" ) ) );
        }
    }
    else
    {
        if ( parser.GetTagName() != wxT( "derived" ) )
            return;

        parser.Require( START_TAG, wxT( "derived" ) );
        parser.Next();

        m_begin = (a2dCanvasObject*) parser.LoadOneObject( this );
        m_end = (a2dCanvasObject*) parser.LoadOneObject( this );

        parser.Require( END_TAG, wxT( "derived" ) );
        parser.Next();
    }
}
#endif //wxART2D_USE_CVGIO

void a2dEndsLine::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dCanvasObject::DoWalker( parent, handler );
    if ( m_begin )
        m_begin->Walker( this, handler );

    if ( m_end )
        m_end->Walker( this, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
}

bool a2dEndsLine::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    bool h = a2dSLine::DoIsHitWorld( ic, hitEvent );

    if ( !h )
    {
        if ( m_begin )
        {
            double dx, dy;
            dx = m_x2 - m_x1;
            dy = m_y2 - m_y1;
            double ang1;
            if ( !dx && !dy )
                ang1 = 0;
            else
                ang1 = wxRadToDeg( atan2( dy, dx ) );

            a2dAffineMatrix lworld;
            //clockwise rotation so minus
            lworld.Scale( m_xscale, m_yscale, 0, 0 );
            lworld.Rotate( -ang1 );
            lworld.Translate( m_x1, m_y1 );
            a2dIterCU cu( ic, lworld );
            h = m_begin->IsHitWorld( ic, hitEvent ) != 0;
        }

        if ( !h && m_end )
        {
            double dx, dy;
            dx = m_x2 - m_x1;
            dy = m_y2 - m_y1;
            double ang2;
            if ( !dx && !dy )
                ang2 = 0;
            else
                ang2 = wxRadToDeg( atan2( -dy, -dx ) );

            a2dAffineMatrix tworld;
            //clockwise rotation so minus
            tworld.Scale( m_xscale, m_yscale, 0, 0 );
            tworld.Rotate( -ang2 );
            tworld.Translate( m_x2, m_y2 );
            a2dIterCU cu( ic, tworld );
            h = m_end->IsHitWorld( ic, hitEvent ) != 0;
        }
    }

    if ( h )
    {
        hitEvent.m_how = a2dHit::stock_strokeoutside;
        return true;
    }
    hitEvent.m_how = a2dHit::stock_nohit;
    return false;
}

//----------------------------------------------------------------------------
// a2dScaledEndLine
//----------------------------------------------------------------------------
#define L1 2
#define L2 1
#define B 1

BEGIN_EVENT_TABLE( a2dScaledEndLine, a2dEndsLine )
END_EVENT_TABLE()

a2dScaledEndLine::a2dScaledEndLine()
    : a2dEndsLine()
{
    m_xscale = m_yscale = 1;
    m_begin = new a2dArrow( 0, 0, L1, L2, B );
    m_end = new a2dArrow( 0, 0, L1, L2, B );
}

a2dScaledEndLine::a2dScaledEndLine( double x1, double y1, double x2, double y2, double contourwidth )
    : a2dEndsLine( x1, y1, x2, y2, contourwidth )
{
    m_begin = new a2dArrow( 0, 0, L1, L2, B );
    m_end = new a2dArrow( 0, 0, L1, L2, B );

    m_xscale = m_yscale = 1;
}

a2dScaledEndLine::a2dScaledEndLine( const a2dSLine& other, CloneOptions options, a2dRefMap* refs )
    : a2dEndsLine( other, options, refs )
{
    m_xscale = m_yscale = 1;
}

a2dScaledEndLine::a2dScaledEndLine( const a2dScaledEndLine& other, CloneOptions options, a2dRefMap* refs )
    : a2dEndsLine( other, options, refs )
{
}

a2dObject* a2dScaledEndLine::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dScaledEndLine( *this, options, refs );
}

a2dScaledEndLine::~a2dScaledEndLine()
{
}

void a2dScaledEndLine::SetBegin( double l1, double l2, double b, bool spline )
{
    if ( m_begin && wxDynamicCast( m_begin.Get(), a2dArrow ) )
        wxStaticCast( m_begin.Get(), a2dArrow )->Set( l1, l2, b, spline );
    SetPending( true );
}

void a2dScaledEndLine::SetEnd( double l1, double l2, double b, bool spline )
{
    if ( m_end && wxDynamicCast( m_end.Get(), a2dArrow ) )
        wxStaticCast( m_end.Get(), a2dArrow )->Set( l1, l2, b, spline );
    SetPending( true );
}

bool a2dScaledEndLine::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( m_begin )
    {
        if ( m_takeStyleToArrow )
        {
            m_begin->SetFill( GetFill() );
            m_begin->SetStroke( GetStroke() );
        }
        m_xscale = m_contourwidth;
        m_yscale = m_contourwidth;
        calc = m_begin->Update( mode );
    }
    if ( m_end )
    {
        if ( m_takeStyleToArrow )
        {
            m_end->SetFill( GetFill() );
            m_end->SetStroke( GetStroke() );
        }
        m_xscale = m_contourwidth;
        m_yscale = m_contourwidth;
        calc = m_end->Update( mode ) || calc;
    }
    if ( !m_bbox.GetValid() || calc )
    {
        m_bbox.SetValid( false );
        m_bbox = DoGetUnTransformedBbox();

        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return calc;
}

void a2dScaledEndLine::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( m_contourwidth )
    {
        a2dLine line1( m_x1, m_y1, m_x2, m_y2 );
        line1.CalculateLineParameters();
        double distance = m_contourwidth / 2.0;
        if ( distance == 0 )
            distance = 1;//line1.GetLength()/10;

        a2dPoint2D layBackBegin( m_x1, m_y1 );
        a2dPoint2D layBackEnd( m_x2, m_y2 );

        //for arrow or derived objects, make it such that the line does not pass the arrow.
        if ( m_begin && wxDynamicCast( m_begin.Get(), a2dArrow ) )
        {
            a2dArrow* arrow1 = wxStaticCast( m_begin.Get(), a2dArrow );
            double baseD2B = arrow1->GetBase() / 2.0;
            double l1B = arrow1->GetL1();
            double layBackB = l1B * distance / baseD2B;
            layBackBegin = line1.DistancePoint( layBackB, true );
        }
        if ( m_end && wxDynamicCast( m_end.Get(), a2dArrow ) )
        {
            a2dArrow* arrow2 = wxStaticCast( m_end.Get(), a2dArrow );
            double baseD2E = arrow2->GetBase() / 2.0;
            double l1E = arrow2->GetL1();
            double layBackE = l1E * distance / baseD2E;
            layBackEnd = line1.DistancePoint( -layBackE, false );
        }

        a2dPoint2D offsetpoint1left = layBackBegin;
        a2dPoint2D offsetpoint1right = layBackBegin;
        a2dPoint2D offsetpoint2left = layBackEnd;
        a2dPoint2D offsetpoint2right = layBackEnd;

        line1.Virtual_Point( offsetpoint1left, distance );
        line1.Virtual_Point( offsetpoint1right, -distance );
        line1.Virtual_Point( offsetpoint2left, distance );
        line1.Virtual_Point( offsetpoint2right, -distance );

        a2dVertexArray contour;
        contour.AddPoint( offsetpoint1left );
        contour.AddPoint( offsetpoint2left );
        contour.AddPoint( offsetpoint2right );
        contour.AddPoint( offsetpoint1right );

        ic.GetDrawer2D()->DrawPolygon( &contour );

        if ( m_begin )
        {
            double dx, dy;
            dx = m_x2 - m_x1;
            dy = m_y2 - m_y1;
            double ang1;
            if ( !dx && !dy )
                ang1 = 0;
            else
                ang1 = wxRadToDeg( atan2( dy, dx ) );

            a2dAffineMatrix lworld;
            //clockwise rotation so minus
            lworld.Scale( m_xscale, m_yscale, 0, 0 );
            lworld.Rotate( -ang1 );
            lworld.Translate( m_x1, m_y1 );
            a2dIterCU cu( ic, lworld );

            //The next switching of render layer is oke, imagine the begin and end objects
            //as normal child objects of the this line, in combination with the flag,
            //the effect is the same as starting a second layer iteration. Only this
            //one is optimized.
            bool old = m_begin->GetChildrenOnSameLayer();
            m_begin->SetChildrenOnSameLayer( true ); //
            m_begin->Render( ic, clipparent );
            m_begin->SetChildrenOnSameLayer( old ); //
        }

        if ( m_end )
        {
            double dx, dy;
            dx = m_x2 - m_x1;
            dy = m_y2 - m_y1;
            double ang2;
            if ( !dx && !dy )
                ang2 = 0;
            else
                ang2 = wxRadToDeg( atan2( -dy, -dx ) );

            a2dAffineMatrix tworld;
            //clockwise rotation so minus
            tworld.Scale( m_xscale, m_yscale, 0, 0 );
            tworld.Rotate( -ang2 );
            tworld.Translate( m_x2, m_y2 );

            a2dIterCU cu( ic, tworld );

            bool old = m_end->GetChildrenOnSameLayer();
            m_end->SetChildrenOnSameLayer( true );
            m_end->Render( ic, clipparent );
            m_end->SetChildrenOnSameLayer( old );
        }
    }
    else
    {
        ic.GetDrawer2D()->DrawLine( m_x1, m_y1, m_x2, m_y2 );
    }

}

//----------------------------------------------------------------------------
// a2dEndsEllipticChord
//----------------------------------------------------------------------------

a2dEndsEllipticChord::a2dEndsEllipticChord()
    : a2dEllipticArc()
{
    m_xscale = m_yscale = 1;
}

a2dEndsEllipticChord::a2dEndsEllipticChord(  double xc, double yc, double width, double height, double start, double end )
    : a2dEllipticArc( xc, yc, width, height, start, end, true )
{
    m_xscale = m_yscale = 1;
}

a2dEndsEllipticChord::a2dEndsEllipticChord( const a2dEndsEllipticChord& other, CloneOptions options, a2dRefMap* refs )
    : a2dEllipticArc( other, options, refs )
{
    if ( options & clone_members )
    {
        if ( other.m_beginobj )
            m_beginobj = other.m_beginobj->TClone( options );
        if ( other.m_endobj )
            m_endobj = other.m_endobj->TClone( options );
    }
    else
    {
        m_beginobj = other.m_beginobj;
        m_endobj = other.m_endobj;
    }

    m_xscale = other.m_xscale;
    m_yscale = other.m_yscale;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in other. .. ->Clone( options )()
    CurrentSmartPointerOwner = this;
#endif
}

a2dObject* a2dEndsEllipticChord::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dEndsEllipticChord( *this, options, refs );
};

a2dEndsEllipticChord::~a2dEndsEllipticChord()
{
}

void a2dEndsEllipticChord::SetBeginObj( a2dCanvasObject* begin )
{
    SetPending( true );
    m_beginobj = begin;
    if ( m_beginobj && m_root  )
        m_beginobj->SetRoot( m_root );
}

void a2dEndsEllipticChord::SetEndObj( a2dCanvasObject* end )
{
    SetPending( true );
    m_endobj = end;
    if ( m_endobj && m_root )
        m_endobj->SetRoot( m_root );
}

a2dBoundingBox a2dEndsEllipticChord::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox = a2dEllipticArc::DoGetUnTransformedBbox( flags );

    if ( m_beginobj )
    {
        bbox.Expand( m_beginobj->GetMappedBbox( GetBeginTransform() ) );
    }

    if ( m_endobj )
    {
        bbox.Expand( m_endobj->GetMappedBbox( GetEndTransform() ) );
    }
    return bbox;
}

bool a2dEndsEllipticChord::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( m_beginobj )
    {
        calc = m_beginobj->Update( mode );
    }
    if ( m_endobj )
    {
        calc = m_endobj->Update( mode ) || calc;
    }
    if ( !m_bbox.GetValid() || calc )
    {
        m_bbox.SetValid( false );
        m_bbox = DoGetUnTransformedBbox();

        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return calc;
}

void a2dEndsEllipticChord::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    a2dEllipticArc::DoRender( ic, clipparent );

    if ( m_beginobj )
    {
        a2dIterCU cu( ic, GetBeginTransform() );

        //The next switching of render layer is oke, imagine the begin and end objects
        //as normal child objects of the this line, in combination with the flag,
        //the effect is the same as starting a second layer iteration. Only this
        //one is optimized.
        bool old = m_beginobj->GetChildrenOnSameLayer();
        m_beginobj->SetChildrenOnSameLayer( true ); //
        m_beginobj->Render( ic, clipparent );
        m_beginobj->SetChildrenOnSameLayer( old ); //
    }

    if ( m_endobj )
    {
        a2dIterCU cu( ic, GetEndTransform() );

        bool old = m_endobj->GetChildrenOnSameLayer();
        m_endobj->SetChildrenOnSameLayer( true );
        m_endobj->Render( ic, clipparent );
        m_endobj->SetChildrenOnSameLayer( old );
    }
}

void a2dEndsEllipticChord::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dCanvasObject::DoWalker( parent, handler );
    if ( m_beginobj )
        m_beginobj->Walker( this, handler );

    if ( m_endobj )
        m_endobj->Walker( this, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
}

#if wxART2D_USE_CVGIO
void a2dEndsEllipticChord::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dEllipticArc::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_beginobj || m_endobj )
        {
            out.WriteAttribute( wxT( "xscale" ), m_xscale * out.GetScale()  );
            out.WriteAttribute( wxT( "yscale" ), m_yscale * out.GetScale()  );
            if ( m_beginobj )
                out.WriteAttribute( wxT( "beginobj" ), m_beginobj->GetId() );
            if ( m_endobj )
                out.WriteAttribute( wxT( "endobj" ), m_endobj->GetId() );
        }
    }
    else
    {
        if ( ( m_beginobj && !m_beginobj->GetCheck() ) ||
                ( m_endobj && !m_endobj->GetCheck() )
           )
        {
            //trick the system to have multiple refs on those objects, else we will not get an id attribute
            a2dCanvasObjectPtr begin = m_beginobj;
            a2dCanvasObjectPtr end = m_endobj;

            out.WriteStartElement( wxT( "derived" ) );

            if ( m_beginobj && !m_beginobj->GetCheck() )
                m_beginobj->Save( this, out, towrite );
            if ( m_endobj  && !m_endobj->GetCheck() )
                m_endobj->Save( this, out, towrite );

            out.WriteEndElement();
        }
    }
}

void a2dEndsEllipticChord::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dEllipticArc::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_xscale = parser.GetAttributeValueDouble( wxT( "xscale" ) ) * parser.GetScale() ;
        m_yscale = parser.GetAttributeValueDouble( wxT( "yscale" ) ) * parser.GetScale() ;
        if ( parser.HasAttribute( wxT( "beginobj" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_beginobj, parser.GetAttributeValue( wxT( "beginobj" ) ) );
        }
        if ( parser.HasAttribute( wxT( "endobj" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_endobj, parser.GetAttributeValue( wxT( "endobj" ) ) );
        }
    }
    else
    {
        if ( parser.GetTagName() != wxT( "derived" ) )
            return;

        parser.Require( START_TAG, wxT( "derived" ) );
        parser.Next();

        m_beginobj = (a2dCanvasObject*) parser.LoadOneObject( this );
        m_endobj = (a2dCanvasObject*) parser.LoadOneObject( this );

        parser.Require( END_TAG, wxT( "derived" ) );
        parser.Next();
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dEndsEllipticChord::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    bool h = a2dEllipticArc::DoIsHitWorld( ic, hitEvent );

    if ( !h )
    {
        if ( m_beginobj )
        {
            a2dIterCU cu( ic, GetBeginTransform() );
            h = m_beginobj->IsHitWorld( ic, hitEvent ) != 0;
        }

        if ( !h && m_endobj )
        {
            a2dIterCU cu( ic, GetEndTransform() );
            h = m_endobj->IsHitWorld( ic, hitEvent ) != 0;
        }
    }

    if ( h )
    {
        hitEvent.m_how = a2dHit::stock_strokeoutside;
        return true;
    }
    hitEvent.m_how = a2dHit::stock_nohit;
    return false;
}

a2dAffineMatrix a2dEndsEllipticChord::GetBeginTransform() const
{
    double ang1 = wxRadToDeg( atan2( sin ( wxDegToRad( m_start + 90 ) ) * m_height, cos ( wxDegToRad( m_start + 90 ) ) * m_width ) );
    double x1 = m_width / 2 * cos( wxDegToRad( m_start ) );
    double y1 = m_height / 2 * sin( wxDegToRad( m_start ) );

    a2dAffineMatrix lworld;
    //clockwise rotation so minus
    lworld.Scale( m_xscale, m_yscale, 0, 0 );
    lworld.Rotate( -ang1 );
    lworld.Translate( x1, y1 );

    return lworld;
}

a2dAffineMatrix a2dEndsEllipticChord::GetEndTransform() const
{
    double ang2 = wxRadToDeg( atan2( sin ( wxDegToRad( m_end - 90 ) ) * m_height, cos ( wxDegToRad( m_end - 90 ) ) * m_width ) );
    double x2 = m_width / 2 * cos( wxDegToRad( m_end ) );
    double y2 = m_height / 2 * sin( wxDegToRad( m_end ) );

    a2dAffineMatrix lworld;
    //clockwise rotation so minus
    lworld.Scale( m_xscale, m_yscale, 0, 0 );
    lworld.Rotate( -ang2 );
    lworld.Translate( x2, y2 );

    return lworld;
}

//----------------------------------------------------------------------------
// a2dControl
//----------------------------------------------------------------------------

a2dControl::a2dControl( double x, double y, double width, double height, wxWindow* control )
    : a2dRect( x, y, width, height )
{
    m_control = control;
    m_control->Show( false );
}

a2dControl::~a2dControl()
{
    //not here since the control is owned by the window in which it was created.
    //and that may be gone before the deletion of the object.
    //m_control->Close();
    //m_control->Destroy();
}

a2dControl::a2dControl( const a2dControl& other, CloneOptions options, a2dRefMap* refs )
    : a2dRect( other, options, refs )
{
    m_control = other.m_control;
    int x, y ;
    other.m_control->GetPosition( &x, &y );
    m_control->Move( x, y );

    SetPending( true );
}

a2dObject* a2dControl::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dControl( *this, options, refs );
};

bool a2dControl::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        return a2dRect::DoUpdate( mode, childbox, clipbox, propbox );
    }
    return false;
}

void a2dControl::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( ic.GetDrawingPart()->GetDisplayWindow() && m_control->GetParent() != ic.GetDrawingPart()->GetDisplayWindow() ) //only one drawer which has the control on its display
        return;

    if ( m_flags.m_editingCopy )
    {
        int xd = ic.GetDrawer2D()->WorldToDeviceX( GetPosX() );
        int yd = ic.GetDrawer2D()->WorldToDeviceY( GetPosY() );
        m_control->Move( xd, yd );
    }
    //else
    a2dRect::DoRender( ic, clipparent );
}

bool a2dControl::DoIsHitWorld( a2dIterC& WXUNUSED( ic ), a2dHitEvent& hitEvent )
{
    hitEvent.m_how = a2dHit::stock_fill;
    return true;
}

bool a2dControl::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        if ( editmode == 1 )
        {
            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            m_control->Show( true );
            return true;
        }
        else
        {
            PROPID_IncludeChildren->SetPropertyToObject( this, false );
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dWH::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

void a2dControl::DoEndEdit()
{
    m_control->Show( false );
    SetPending( true );
}

//----------------------------------------------------------------------------
// a2dWires
//----------------------------------------------------------------------------
a2dWires::a2dWires()
{
    m_childobjects = new a2dCanvasObjectList();
    m_frozen = false;
    m_flags.m_draggable = false;
}

a2dWires::a2dWires( a2dCanvasObject* toconnect, const wxString& pinname )
    : a2dCanvasObject()
{
    m_flags.m_draggable = false;
    bool done = false;

    m_childobjects = new a2dCanvasObjectList();

    m_frozen = false;

    for( a2dCanvasObjectList::iterator iter = toconnect->GetChildObjectList()->begin(); iter != toconnect->GetChildObjectList()->end(); ++iter )
    {
        a2dPin* pinc = wxDynamicCast( ( *iter ).Get(), a2dPin );
        if ( pinc )
        {
            if ( pinname == pinc->GetName() )
            {
                a2dPin* newpin = new a2dPin( this, wxT( "pin1" ), a2dPinClass::Standard, pinc->GetAbsX(), pinc->GetAbsY(), 180 - pinc->GetAbsAngle() );
                Append( newpin );

                pinc->ConnectTo( newpin );
                done = true;
            }
        }
    }

    wxASSERT_MSG( done, wxT( "no such pin to connect to" ) );
}

a2dWires::a2dWires( a2dCanvasObject* toConnect, a2dPin* pinc )
    : a2dCanvasObject()
{
    bool done = false;
    wxASSERT_MSG( pinc->GetParent() == toConnect, wxT( "this pin has not connect object as parent" ) );

    for( a2dCanvasObjectList::iterator iter = toConnect->GetChildObjectList()->begin(); iter != toConnect->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinother = wxDynamicCast( obj, a2dPin );
        if ( pinother == pinc )
        {
            done = true;
            break;
        }
    }

    wxASSERT_MSG( done, wxT( "this pin is not in to connect object" ) );

    m_flags.m_hasPins = true;

    m_childobjects = new a2dCanvasObjectList();

    m_frozen = false;

    a2dPin* newpin = new a2dPin( this, wxT( "pin1" ), a2dPinClass::Standard, pinc->GetAbsX(), pinc->GetAbsY(), 180 - pinc->GetAbsAngle() );
    Append( newpin );

    pinc->ConnectTo( newpin );
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in new a2dPin()
    CurrentSmartPointerOwner = this;
#endif
}

a2dWires::~a2dWires()
{
}

a2dObject* a2dWires::DoClone( CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    a2dWires* a = new a2dWires();
    //TODO
    return a;
};

bool a2dWires::IsConnect() const
{
    return true;
}

bool a2dWires::ConnectWith( a2dCanvasObject* parent, a2dCanvasObject* toConnect, const wxString& pinname, double margin, bool WXUNUSED( undo ) )
{
    bool done = false;

    for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( pinc )
        {
            if ( pinname.IsEmpty() )
            {
                //search pin at same position in object to connect

                for( a2dCanvasObjectList::iterator itero = toConnect->GetChildObjectList()->begin(); itero != toConnect->GetChildObjectList()->end(); ++itero )
                {
                    a2dCanvasObject* obj = *itero;
                    a2dPin* pinother = wxDynamicCast( obj, a2dPin );
                    if ( pinother )
                    {
                        if ( fabs( pinother->GetAbsX() - pinc->GetAbsX() ) < margin  &&
                                fabs( pinother->GetAbsY() - pinc->GetAbsY() ) < margin )
                        {
                            ConnectPins( parent, pinc, pinother );
                            done = true;
                        }
                    }
                }
            }
            else
            {
                //search pin with pinname in object to connect
                for( a2dCanvasObjectList::iterator itero = toConnect->GetChildObjectList()->begin(); itero != toConnect->GetChildObjectList()->end(); ++itero )
                {
                    a2dCanvasObject* obj = *itero;
                    a2dPin* pinother = wxDynamicCast( obj, a2dPin );
                    if ( pinother && pinname == pinother->GetName() )
                    {
                        if ( fabs( pinother->GetAbsX() - pinc->GetAbsX() ) < margin  &&
                                fabs( pinother->GetAbsY() - pinc->GetAbsY() ) < margin )
                        {
                            ConnectPins( parent, pinc, pinother );
                            done = true;
                        }
                    }
                }
            }
        }
    }

    if ( !done )
    {
        wxString nextpin = wxT( "pin" );
        nextpin << GetPinCount() + 1;

        //search pin with pinname in object to connect
        for( a2dCanvasObjectList::iterator itero = toConnect->GetChildObjectList()->begin(); itero != toConnect->GetChildObjectList()->end(); ++itero )
        {
            a2dCanvasObject* obj = *itero;
            a2dPin* pinother = wxDynamicCast( obj, a2dPin );
            if ( pinother )
            {
                if ( pinname == pinother->GetName() )
                {
                    a2dPin* newpin = new a2dPin( this, nextpin, a2dPinClass::Standard, pinother->GetAbsX(), pinother->GetAbsY(), 180 - pinother->GetAbsAngle() );
                    Append( newpin );

                    pinother->ConnectTo( newpin );
                    done = true;
                }
            }
        }
    }

    SetPending( true );

    return done;
}

bool a2dWires::ConnectWith( a2dCanvasObject* WXUNUSED( parent ), a2dPin* pin, double margin, bool WXUNUSED( undo ) )
{
    if ( pin->GetParent() == this )
        return false;

    bool done = false;

    a2dCanvasObjectList::iterator iter;
    //if there the pin to connect is already in the wire, nothing to do.
    for( iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( pinc && pinc == pin )
        {
            return true;
        }
    }

    //if there is an un-connected pin at this position, use it.
    for( iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( pinc )
        {
            if ( fabs( pinc->GetAbsX() - pin->GetAbsX() ) < margin  &&
                    fabs( pinc->GetAbsY() - pin->GetAbsY() ) < margin )
            {
                if ( !pinc->IsConnectedTo( pin ) )
                {
                    pin->ConnectTo( pinc );
                }
                done = true;
            }
        }
    }

    if ( !done )
    {
        wxString nextpin = wxT( "pin" );
        nextpin << GetPinCount() + 1;


        a2dPin* newpin = new a2dPin( this, nextpin, a2dPinClass::Standard, pin->GetAbsX(), pin->GetAbsY(), 180 - pin->GetAbsAngle() );
        Append( newpin );

        pin->ConnectTo( newpin );
    }

    SetPending( true );

    return true;
}

bool a2dWires::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( pinc && pinc->IsConnectedTo() )
        {
            //todo 
            //pinc->Set( pinc->ConnectedTo()->GetAbsX(), pinc->ConnectedTo()->GetAbsY(), 180 - pinc->ConnectedTo()->GetAbsAngle() );
        }
    }


    if ( m_childobjects != wxNullCanvasObjectList && m_flags.m_visiblechilds )
    {
        for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            obj->Update( mode );

            //if calcchild has become true now, this means that down here the current child object did change and
            //therefore has recalculated its boundingbox
        }
    }
    /*
        //delete old wires
        node = GetChildObjectList()->GetFirst();
        while (node)
        {
            a2dCanvasObject *obj = node->GetData();
            if (obj->CheckMask(a2dCanvasOFlags::A))
            {
                obj->Release();

                a2dCanvasObjectList::compatibility_iterator nodeh=node;
                node = node->GetNext();
                GetChildObjectList()->DeleteNode(nodeh);
            }
            else
                node = node->GetNext();
        }

        //first create proper wires in group
        CreateWires();
        node = m_childobjects->GetFirst();
        while (node)
        {
            a2dCanvasObject *obj = node->GetData();
            obj->Update(force);
            node = node->GetNext();
        }
    */

    return true;
}

void a2dWires::DoRender( a2dIterC& ic,  OVERLAP WXUNUSED( clipparent ) )
{

    a2dAffineMatrix inverse =  m_lworld;
    inverse.Invert();

    a2dAffineMatrix untrans = ic.GetTransform();
    untrans *= inverse;

    //the boundingbox is relative to the parent.
    a2dBoundingBox absarea = GetMappedBbox( untrans );

#ifdef CANVASDEBUG

    a2dBoundingBox bbox = DoGetUnTransformedBbox( ic );
    a2dRect* tmp =  new a2dRect( absarea.GetMinX() , absarea.GetMinY() , absarea.GetWidth() , absarea.GetHeight() );
    tmp->SetIgnoreSetpending();
    tmp->Own();
    tmp->SetRoot( m_root );
    tmp->Update( ic, true );
    tmp->SetFill( a2dTRANSPARENT_FILL );
    tmp->SetStroke( ic.GetDrawer2D()->GetDrawerStroke() );
    tmp->Render( cworld, _ON, layer, mask );
    tmp->Release();
#endif

    if ( ic.GetDrawer2D()->GetClippingBox().Intersect( absarea ) != _OUT )
    {
        for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pinc = wxDynamicCast( obj, a2dPin );

            if ( pinc )
            {
                double x = pinc->GetAbsX();
                double y = pinc->GetAbsY();

                for( a2dCanvasObjectList::iterator itero = m_childobjects->begin(); itero != m_childobjects->end(); ++itero )
                {
                    a2dCanvasObject* obj = *itero;
                    a2dPin* pinother = wxDynamicCast( obj, a2dPin );

                    if ( pinother && pinother != pinc )
                    {
                        double x2 = pinother->GetAbsX();
                        double y2 = pinother->GetAbsY();

                        ic.GetDrawer2D()->DrawLine( x, y, x2, y2 );
                    }
                }
            }
        }
    }
}

void a2dWires::CreateWires()
{
    for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( pinc )
        {
            double x = pinc->GetPosX();
            double y = pinc->GetPosY();

            for( a2dCanvasObjectList::iterator itero = m_childobjects->begin(); itero != m_childobjects->end(); ++itero )
            {
                a2dCanvasObject* obj = *itero;
                a2dPin* pinother = wxDynamicCast( obj, a2dPin );
                if ( pinother && pinother != pinc )
                {
                    double x2 = pinother->GetPosX();
                    double y2 = pinother->GetPosY();

                    //direct maybe better?
                    //ic.GetDrawer2D()->DrawLine(x,y,x2,y2);

                    a2dSmrtPtr<a2dSLine> a = new a2dSLine( x, y, x2, y2 );
                    a->SetLayer( wxLAYER_DEFAULT );
                    //Slow
                    //a->SetStroke(a2dStroke(wxColour(252,0,2 ),1.0,wxSOLID));
                    a->SetSpecificFlags( true, a2dCanvasOFlags::A ); //to be able to delete them afterwords
                    a->SetRoot( m_root );
                    Append( a );
                }
            }
        }
    }
}
