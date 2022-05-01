/*! \file curves/src/marker.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: marker.cpp,v 1.39 2008/10/31 22:43:43 titato Exp $
*/

#pragma warning(disable:4786)

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/wfstream.h>
#include <math.h>

#include "wx/canvas/eval.h"
#include "wx/canvas/canglob.h"
#include "wx/curves/meta.h"

#include "wx/canvas/canobj.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/canvas.h"

#if wxART2D_USE_EDITOR
#include "wx/canvas/edit.h"
#endif //wxART2D_USE_EDITOR

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dBaseMarker, a2dCurveObject )
IMPLEMENT_DYNAMIC_CLASS( a2dAxisMarker, a2dBaseMarker )
IMPLEMENT_DYNAMIC_CLASS( a2dCursorAxisMarker, a2dAxisMarker )
IMPLEMENT_DYNAMIC_CLASS( a2dMarker, a2dBaseMarker )
IMPLEMENT_DYNAMIC_CLASS( a2dBandMarker, a2dMarker )
IMPLEMENT_DYNAMIC_CLASS( a2dMarkerShow, a2dCanvasObject )

#ifdef wxUSE_INTPOINT
#define wxMAX_COORDINATE INT_MAX
#define wxMIN_COORDINATE INT_MIN
#else
#define wxMAX_COORDINATE 100e99
#define wxMIN_COORDINATE 100e-99
#endif


//----------------------------------------------------------------------------
// a2dBaseMarker
//----------------------------------------------------------------------------
#define fach 0.7
#define facv 1.78

a2dPropertyIdDouble* a2dBaseMarker::PROPID_PositionMarker = NULL;

INITIALIZE_PROPERTIES( a2dBaseMarker, a2dCurveObject )
{
    A2D_PROPID_GS( a2dPropertyIdDouble, a2dBaseMarker, PositionMarker, 0, GetPosition, SetPosition );
    return true;
}

BEGIN_EVENT_TABLE( a2dBaseMarker, a2dCurveObject )
    EVT_CANVASOBJECT_ENTER_EVENT(  a2dBaseMarker::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT(  a2dBaseMarker::OnLeaveObject )
END_EVENT_TABLE()

a2dBaseMarker::a2dBaseMarker( const wxString& format ): a2dCurveObject()
{
    m_position = 0;
    m_l1 = 12;
    m_l2 = 10;
    m_b  = 10;
    m_step = 0;
    m_spline = false;
    m_prompting = false;

    m_format = format;
    m_textheight = m_l1 / 5 ;
    m_font = a2dFont( m_textheight, wxFONTFAMILY_SWISS,  wxFONTSTYLE_ITALIC );

}

a2dBaseMarker::a2dBaseMarker( const wxString& format, double atx, double height ): a2dCurveObject()
{
    m_position = atx;

    m_l1 = 12;
    m_l2 = 10;
    m_b  = 10;
    m_step = 0;
    m_prompting = false;

    m_lworld.Translate( atx, 0 );

    m_spline = false;

    m_format = format;
    m_textheight = height;
    if ( height == 0 )
        m_textheight = m_l1 / 5 ;

    m_font = a2dFont( m_textheight, wxFONTFAMILY_SWISS,  wxFONTSTYLE_ITALIC );
}

a2dBaseMarker::a2dBaseMarker( const wxString& format, double atx, const a2dFont& font ): a2dCurveObject()
{
    m_position = atx;

    m_l1 = 12;
    m_l2 = 10;
    m_b  = 10;
    m_spline = false;

    m_lworld.Translate( atx, 0 );

    m_format = format;
    m_font = font;
    m_textheight = m_font.GetSize();
    m_step = 0;
    m_prompting = false;
}


a2dBaseMarker::~a2dBaseMarker()
{
}

a2dBaseMarker::a2dBaseMarker( const a2dBaseMarker& other, CloneOptions options, a2dRefMap* refs )
    : a2dCurveObject( other, options, refs )
{
    m_format = other.m_format;
    m_textheight = other.m_textheight;
    m_l1 = other.m_l1;
    m_l2 = other.m_l2;
    m_b  = other.m_b;
    m_step = other.m_step;
    m_spline = other.m_spline;
    m_prompting = other.m_prompting;
    m_position = other.m_position;
    m_font = other.m_font;
}

a2dObject* a2dBaseMarker::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dBaseMarker( *this, options, refs );
};


void a2dBaseMarker::Set( double xt, double yt, double l1, double l2, double b )
{
    SetPosXY( xt, yt );
    m_l1 = l1;
    m_l2 = l2;
    m_b  = b;

    SetPending( true );
}

void a2dBaseMarker::Set( double l1, double l2, double b )
{
    m_l1 = l1;
    m_l2 = l2;
    m_b  = b;

    SetPending( true );
}

a2dBoundingBox a2dBaseMarker::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    bbox.Expand( -m_b / 2, 0 );
    bbox.Expand(  m_b / 2, m_l1 );
    bbox.Expand(  0, m_l2 );

    if ( !m_format.IsEmpty() && m_textheight )
    {
        double xc, yc;
        m_curvesArea->World2Curve( GetPosX(), GetPosY(), xc, yc );
        wxString form;
        form.Printf( m_format, xc, yc );

        a2dBoundingBox bboxtext = m_font.GetTextExtent( form );
        bboxtext.Translate( -m_b / 2 , m_l1 + m_textheight * 2.5 );
        bbox.Expand( bboxtext );
    }
    return bbox;
}

#if wxART2D_USE_CVGIO

void a2dBaseMarker::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "l1" ) , m_l1 );
        out.WriteAttribute( wxT( "l2" ) , m_l2 );
        out.WriteAttribute( wxT( "b" ) , m_b );
    }
    else
    {
    }
}

void a2dBaseMarker::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_l1 = parser.RequireAttributeValueDouble( wxT( "l1" ) );
        m_l2 = parser.RequireAttributeValueDouble( wxT( "l2" ) );
        m_b = parser.RequireAttributeValueDouble( wxT( "b" ) );
    }
    else
    {
    }

    //m_curve = ;
}
#endif //wxART2D_USE_CVGIO

bool a2dBaseMarker::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    a2dVertexArray points;
    points.push_back( new a2dLineSegment( 0, 0 ) );
    points.push_back( new a2dLineSegment( m_b / 2.0, m_l1 ) );
    points.push_back( new a2dLineSegment( 0, m_l2 ) );
    points.push_back( new a2dLineSegment( -m_b / 2.0, m_l1 ) );

    a2dPoint2D P = a2dPoint2D( hitEvent.m_relx, hitEvent.m_rely );
    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

    hitEvent.m_how = points.HitTestPolygon( P, pw + margin );

    return hitEvent.m_how.IsHit();
}


bool a2dBaseMarker::DoStartEdit( wxUint16 WXUNUSED( editmode ), wxEditStyle WXUNUSED( editstyle ) )
{
    if ( m_flags.m_editable )
    {
        //calculate bbox's else mouse events may take place when first, and idle event is not yet
        //processed to do this calculation.
        Update( updatemask_force );
        //still set it pending to do the redraw ( in place )
        SetPending( true );
        return true;
    }

    return false;
}

#define BBOX2XYWH(bbox) (bbox).GetMinX(), (bbox).GetMinY(), (bbox).GetWidth(), (bbox).GetHeight()

void a2dBaseMarker::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    a2dVertexArray cpoints;

    // For debugging
//  ic.GetDrawer2D()->SetDrawerFill(a2dWHITE_FILL);
//  ic.GetDrawer2D()->DrawRoundedRectangle(BBOX2XYWH(m_untransbbox), 0);

    if ( m_flags.m_editingCopy )
        ic.GetDrawer2D()->SetDrawerFill( *a2dBLACK_FILL );

    cpoints.push_back( new a2dLineSegment( 0, 0 ) );
    cpoints.push_back( new a2dLineSegment( m_b / 2.0, m_l1 ) );
    if( m_l1 != m_l2 )
    {
        cpoints.push_back( new a2dLineSegment( 0, m_l2 ) );
    }
    cpoints.push_back( new a2dLineSegment( -m_b / 2.0, m_l1 ) );

    ic.GetDrawer2D()->DrawPolygon( &cpoints, m_spline, wxWINDING_RULE );

    cpoints.clear();

    if( !m_format.IsEmpty() )
    {
        double xc, yc;
        m_curvesArea->World2Curve( GetPosX(), GetPosY(), xc, yc );
        wxString form;
        form.Printf( m_format, xc, yc );

        ic.GetDrawer2D()->SetFont( m_font );
        ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );

        ic.GetDrawer2D()->DrawText( form, -m_b / 2, m_l1 + m_textheight * 2.5 );
        //ic.GetDrawer2D()->DrawText( form, 0, 0 );
        ic.GetDrawer2D()->SetFont( *a2dNullFONT );
    }
}

const wxString& a2dBaseMarker::GetMarkerString()
{
    if( !m_promptFormat.IsEmpty() )
    {
        double xc, yc;
        m_curvesArea->World2Curve( GetPosX(), GetPosY(), xc, yc );
        wxString form;
        m_promptString.Printf( m_promptFormat, xc, yc );
    }
    else
        m_promptString = wxT( "BaseMarker" );
    return m_promptString;
}

void a2dBaseMarker::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    return;
    a2dIterC* ic = event.GetIterC();

    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
    m_prompting = true;
    if( m_promptText )
    {
//      ic->GetTransform().TransformPoint(GetPosX(),GetPosY());
        m_promptText->SetText( GetMarkerString() );
        m_promptText->SetPosXY( m_b / 2, m_l1 );
        m_promptText->SetVisible( true );
    }
    SetPending( true );
    event.Skip();
}

void a2dBaseMarker::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    return;
    event.Skip();
    m_prompting = false;
    if( m_promptText )
        m_promptText->SetVisible( false );
    SetPending( true );
}

void a2dBaseMarker::SetPrompt( a2dText* prompttext )
{
    if( m_promptText )
        ReleaseChild( m_promptText );
    m_promptText = prompttext;
    prompttext->SetVisible( false );
    Append( prompttext );
    SetPending( true );
}

//----------------------------------------------------------------------------
// a2dAxisMarker
//----------------------------------------------------------------------------
#define fach 0.7
#define facv 1.78

a2dPropertyIdCanvasObject* a2dAxisMarker::PROPID_axis = NULL;

INITIALIZE_PROPERTIES( a2dAxisMarker, a2dBaseMarker )
{
    A2D_PROPID_D( a2dPropertyIdCanvasObject, axis, 0 );
    return true;
}

void a2dAxisMarker::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );

    a2dBaseMarker::DoWalker( this, handler );

    // the m_axis is just a pointer of which this object is part, so do not walk

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}

BEGIN_EVENT_TABLE( a2dAxisMarker, a2dBaseMarker )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dAxisMarker::OnCanvasObjectMouseEvent )
END_EVENT_TABLE()

void a2dAxisMarker::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        a2dAxisMarker* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dAxisMarker );

        static double xprev;
        static double yprev;

        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        a2dAffineMatrix atWorld = ic->GetTransform();
        a2dAffineMatrix inverse = ic->GetInverseTransform();

        double xh, yh;
        inverse.TransformPoint( xw, yw, xh, yh );

        if ( event.GetMouseEvent().LeftDown() )
        {
            ic->SetCorridorPathToObject( this );
            xprev = xh;
            yprev = yh;
        }
        else if ( event.GetMouseEvent().LeftUp() )
        {
            ic->SetCorridorPath( false );
            ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );

            if ( m_lworld != original->GetTransformMatrix() )
                m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_TransformMatrix, m_lworld ) );
        }
        else if ( event.GetMouseEvent().Dragging() )
        {
            if( m_axis->IsYAxis() )
            {
                double xy = m_axis->ConvertWorld2Axis( yh );
                if ( m_axis->GetAxisMin() < xy && m_axis->GetAxisMax() > xy )
                {
                    double x, y;
                    x = xh - ( xprev - GetPosX() );
                    y = yh - ( yprev - GetPosY() );

                    SetPosXY( 0, y );
                    original->SetPending( true );
                    original->SetPosXY( 0, y );
                }
                else if ( m_axis->GetAxisMin() > xy )
                {
                    double y = 0;
                    if ( m_axis->GetAxisMin() != m_axis->GetBoundaryMin() )
                        y = m_axis->GetLength();

                    SetPosXY( 0, y );

                    //the next takes care of imediate updating of
                    // the original a2dMarker AND a2dMarkerShow Objects
                    //The are dependency checking the original object.
                    original->SetPending( true );
                    original->SetPosXY( 0, y );
                }
                else if ( m_axis->GetAxisMax() < xy )
                {
                    double y = 0;
                    if ( m_axis->GetAxisMin() == m_axis->GetBoundaryMin() )
                        y = m_axis->GetLength();

                    SetPosXY( 0, y );

                    //the next takes care of imediate updating of
                    // the original a2dMarker AND a2dMarkerShow Objects
                    //The are dependency checking the original object.
                    original->SetPending( true );
                    original->SetPosXY( 0, y );
                }
            }
            else
            {
                double xy = m_axis->ConvertWorld2Axis( xh );
                if ( m_axis->GetAxisMin() < xy && m_axis->GetAxisMax() > xy )
                {
                    double x, y;
                    x = xh - ( xprev - GetPosX() );
                    y = yh - ( yprev - GetPosY() );

                    SetPosXY( x, 0 );
                    original->SetPending( true );
                    original->SetPosXY( x, 0 );
                }
                else if ( m_axis->GetAxisMin() > xy )
                {
                    double x = 0;
                    if ( m_axis->GetAxisMin() != m_axis->GetBoundaryMin() )
                        x = m_axis->GetLength();

                    SetPosXY( x, 0 );

                    //the next takes care of imediate updating of
                    // the original a2dMarker AND a2dMarkerShow Objects
                    //The are dependency checking the original object.
                    original->SetPending( true );
                    original->SetPosXY( x, 0 );
                }
                else if ( m_axis->GetAxisMax() < xy )
                {
                    double x = 0;
                    if ( m_axis->GetAxisMin() == m_axis->GetBoundaryMin() )
                        x = m_axis->GetLength();

                    SetPosXY( x, 0 );

                    //the next takes care of imediate updating of
                    // the original a2dMarker AND a2dMarkerShow Objects
                    //The are dependency checking the original object.
                    original->SetPending( true );
                    original->SetPosXY( x, 0 );
                }
            }
            xprev = xh;
            yprev = yh;
        }
        else
        {
            event.Skip();
        }
    }
    else
    {
        event.Skip();
    }
}

a2dAxisMarker::a2dAxisMarker( const wxString& format, a2dCurveAxis* axis )
    : a2dBaseMarker( format )
{
    m_axis = axis;
}

a2dAxisMarker::a2dAxisMarker( const wxString& format, a2dCurveAxis* axis , double atx, double height )
    : a2dBaseMarker( format, atx, height )
{
    m_axis = axis;
}

a2dAxisMarker::a2dAxisMarker( const wxString& format, a2dCurveAxis* axis , double atx, const a2dFont& font )
    : a2dBaseMarker( format, atx, font )
{
    m_axis = axis;
}


a2dAxisMarker::~a2dAxisMarker()
{
}

a2dAxisMarker::a2dAxisMarker( const a2dAxisMarker& other, CloneOptions options, a2dRefMap* refs )
    : a2dBaseMarker( other, options, refs )
{
    m_axis = other.m_axis;
}

a2dObject* a2dAxisMarker::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dAxisMarker( *this, options, refs );
};

void a2dAxisMarker::DoAddPending( a2dIterC& WXUNUSED( ic ) )
{
    //klion 27.05.2005    if ( m_flags.m_pending )
    //        SetPending(true);
}

void a2dAxisMarker::DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) )
{
    if ( !m_flags.m_pending && m_axis->GetPending() )
    {
        //wxLogDebug( "marker");
        SetPending( true );
    }
}

bool a2dAxisMarker::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( m_axis && !m_bbox.GetValid() )
    {
        double worldPos = m_axis->ConvertAxis2World( m_position );
        if( m_axis->IsYAxis() )
            SetPosXY( 0, worldPos );
        else
            SetPosXY( worldPos, 0 );

        m_untransbbox = m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

#if wxART2D_USE_CVGIO

void a2dAxisMarker::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dBaseMarker::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_axis )
        {
            out.WriteAttribute( wxT( "axisname" ) , m_axis->GetName() );
            out.WriteAttribute( wxT( "axis" ), m_axis->GetId() );
        }
    }
    else
    {
    }
}

void a2dAxisMarker::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dBaseMarker::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( parser.HasAttribute( wxT( "axis" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_axis, parser.GetAttributeValue( wxT( "axis" ) ) );
        }
    }
    else
    {
    }

    //m_curve = ;
}
#endif //wxART2D_USE_CVGIO

const wxString& a2dAxisMarker::GetMarkerString()
{
    double val;
    if( !m_promptFormat.IsEmpty() && m_axis->GetLevelValue( GetName(), val ) )
    {
        m_promptString.Printf( m_promptFormat, val );
    }
    else if( !GetName().IsEmpty() )
        m_promptString = GetName();
    else
        m_promptString = wxT( "AxisMarker" );
    return m_promptString;
}

//----------------------------------------------------------------------------
// a2dCursorAxisMarker
//----------------------------------------------------------------------------

a2dPropertyIdCanvasObject* a2dCursorAxisMarker::PROPID_cursor = NULL;

INITIALIZE_PROPERTIES( a2dCursorAxisMarker, a2dAxisMarker )
{
    A2D_PROPID_D( a2dPropertyIdCanvasObject, cursor, 0 );
    return true;
}

a2dCursorAxisMarker::a2dCursorAxisMarker( const wxString& format, a2dCurveAxis* axis,  a2dCursor* cursor )
    : a2dAxisMarker( format, axis )
{
    m_cursor = cursor;
    m_positionY = 0;
}

a2dCursorAxisMarker::a2dCursorAxisMarker( const wxString& format, a2dCurveAxis* axis , a2dCursor* cursor, double atx, double aty, double height )
    : a2dAxisMarker( format, axis, atx, height )
{
    m_cursor = cursor;
    m_positionY = aty;
}

a2dCursorAxisMarker::a2dCursorAxisMarker( const wxString& format, a2dCurveAxis* axis , a2dCursor* cursor, double atx, double aty, const a2dFont& font )
    : a2dAxisMarker( format, axis, atx, font )
{
    m_cursor = cursor;
    m_positionY = aty;
}


a2dCursorAxisMarker::~a2dCursorAxisMarker()
{
}

a2dCursorAxisMarker::a2dCursorAxisMarker( const a2dCursorAxisMarker& other, CloneOptions options, a2dRefMap* refs )
    : a2dAxisMarker( other, options, refs )
{
    m_cursor = other.m_cursor;
    m_positionY = other.m_positionY;
}

a2dObject* a2dCursorAxisMarker::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCursorAxisMarker( *this, options, refs );
};

void a2dCursorAxisMarker::DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) )
{
    if ( !m_flags.m_pending && ( m_axis && m_axis->GetPending() || m_cursor && m_cursor->GetPending() ) )
    {
        //wxLogDebug( "marker");
        SetPending( true );
    }
}

bool a2dCursorAxisMarker::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( m_axis && !m_bbox.GetValid() )
    {
        double worldPosY = 0.;
        if( m_cursor )
            worldPosY = m_cursor->ConvertAxis2World( m_positionY );
        double worldPos = m_axis->ConvertAxis2World( m_position );
        if( m_axis->IsYAxis() )
            SetPosXY( worldPosY, worldPos );
        else
            SetPosXY( worldPos, worldPosY );

        m_untransbbox = m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

#if wxART2D_USE_CVGIO

void a2dCursorAxisMarker::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dAxisMarker::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_cursor )
        {
            out.WriteAttribute( wxT( "cursor" ), m_cursor->GetId() );
        }
//        out.WriteAttribute( wxT("positionY"), m_positionY );
    }
    else
    {
    }
}

void a2dCursorAxisMarker::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dAxisMarker::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( parser.HasAttribute( wxT( "cursor" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_cursor, parser.GetAttributeValue( wxT( "cursor" ) ) );
        }
    }
    else
    {
    }

    //m_curve = ;
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dMarker
//----------------------------------------------------------------------------
#define fach 0.7
#define facv 1.78

a2dPropertyIdCanvasObject* a2dMarker::PROPID_curve = NULL;

INITIALIZE_PROPERTIES( a2dMarker, a2dBaseMarker )
{
    A2D_PROPID_D( a2dPropertyIdCanvasObject, curve, 0 );
    return true;
}

void a2dMarker::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dBaseMarker::DoWalker( parent, handler );

    if ( m_curve )
        m_curve->Walker( this, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}


BEGIN_EVENT_TABLE( a2dMarker, a2dBaseMarker )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dMarker::OnCanvasObjectMouseEvent )
    EVT_CHAR( a2dMarker::OnChar )
END_EVENT_TABLE()

void a2dMarker::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        a2dMarker* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dMarker );

        static double xprev;
        static double yprev;

        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        a2dAffineMatrix atWorld = ic->GetTransform();
        a2dAffineMatrix inverse = ic->GetInverseTransform();

        double xh, yh;
        inverse.TransformPoint( xw, yw, xh, yh );

        double xcurve, ycurve;
        m_curvesArea->World2Curve( xh, yh, xcurve, ycurve );

        if ( event.GetMouseEvent().LeftDown() )
        {
            ic->SetCorridorPathToObject( this );
            xprev = xh;
            yprev = yh;
        }
        else if ( event.GetMouseEvent().LeftUp() )
        {
            ic->SetCorridorPath( false );
            ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );

            //if ( original->GetPosition() != m_position )
            m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_PositionMarker, m_position ) );
        }
        else if ( event.GetMouseEvent().Dragging() )
        {
            a2dBoundingBox curvebox = m_curve->GetClippedCurveBoundaries();
            if( curvebox.GetValid() )
            {
                if ( curvebox.GetMinX() < xcurve && curvebox.GetMaxX() > xcurve )
                {
                    double x, y;
                    x = xh - ( xprev - GetPosX() );
                    y = yh - ( yprev - GetPosY() );

                    m_position = xcurve;
                    CalculatePositionMarker( x, y );
                    SetPosXY( x, y );

                    //the next takes care of imediate updating of
                    // the original a2dMarker AND a2dMarkerShow Objects
                    //The are dependency checking the original object.
                    original->SetPosition( m_position );
                }
                else if ( curvebox.GetMinX() > xcurve )
                {
                    double x, y ;
                    m_position = curvebox.GetMinX();
                    CalculatePositionMarker( x, y );
                    SetPosXY( x, y );

                    //the next takes care of imediate updating of
                    // the original a2dMarker AND a2dMarkerShow Objects
                    //The are dependency checking the original object.
                    original->SetPosition( m_position );
                }
                else if ( curvebox.GetMaxX() < xcurve )
                {
                    double x, y ;
                    m_position = curvebox.GetMaxX();
                    CalculatePositionMarker( x, y );
                    SetPosXY( x, y );

                    //the next takes care of imediate updating of
                    // the original a2dMarker AND a2dMarkerShow Objects
                    //The are dependency checking the original object.
                    original->SetPosition( m_position );
                }
            }
            xprev = xh;
            yprev = yh;
        }
        else
        {
            event.Skip();
        }
    }
    else
    {
        event.Skip();
    }
}

void a2dMarker::OnChar( wxKeyEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        a2dBoundingBox curvebox = m_curve->GetClippedCurveBoundaries();
        if( !curvebox.GetValid() )
        {
            return;
        }

        a2dMarker* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dMarker );

        double x;
        x = m_position; //GetPosX();

        double step = ( curvebox.GetMaxX() - curvebox.GetMinX() ) / 100;
        if ( m_step )
            step = m_step;

        switch( event.GetKeyCode() )
        {
            case WXK_LEFT:
                x = x - step;
                break;

            case WXK_RIGHT:
                x = x + step;
                break;

                //case WXK_PAGEUP:
            case WXK_PAGEUP:
                x =  x + step * 10;
                break;

                //case WXK_PAGEDOWN:
            case WXK_PAGEDOWN:
                x = x - step * 10;
                break;

            case WXK_HOME:
                x = curvebox.GetMinX();
                break;

            case WXK_END:
                x = curvebox.GetMaxX();
                break;

                break;
            default:
                event.Skip();
                return;
        }

        if ( curvebox.GetMinX() <= x && curvebox.GetMaxX() >= x )
        {
            double yw, xw;
            m_position = x;
            CalculatePositionMarker( yw, xw );

            SetPosXY( yw, xw );

            //the next takes care of imediate updating of
            // the original a2dMarker AND a2dMarkerShow Objects
            //They are dependency checking the original object.
            original->SetPosition( m_position );
        }
        else if ( curvebox.GetMinX() > x )
        {
            double yw, xw;
            m_position = curvebox.GetMinX();
            CalculatePositionMarker( yw, xw );
            SetPosXY( yw, xw );

            //the next takes care of imediate updating of
            // the original a2dMarker AND a2dMarkerShow Objects
            //They are dependency checking the original object.
            original->SetPosition( m_position );
        }
        else if ( curvebox.GetMaxX() < x )
        {
            double yw, xw;
            m_position = curvebox.GetMaxX();
            CalculatePositionMarker( yw, xw );
            SetPosXY( yw, xw );

            //the next takes care of imediate updating of
            // the original a2dMarker AND a2dMarkerShow Objects
            //They are dependency checking the original object.
            original->SetPosition( m_position );
        }

    }
    else
        event.Skip();
}

a2dMarker::a2dMarker( const wxString& format, a2dCurve* curve )
    : a2dBaseMarker( format )
{
    m_curve = curve;
}

a2dMarker::a2dMarker( const wxString& format, a2dCurve* curve , double atx, double height )
    : a2dBaseMarker( format, atx, height )
{
    m_curve = curve;
}

a2dMarker::a2dMarker( const wxString& format, a2dCurve* curve , double atx, const a2dFont& font )
    : a2dBaseMarker( format, atx, font )
{
    m_curve = curve;
}


a2dMarker::~a2dMarker()
{
}

a2dMarker::a2dMarker( const a2dMarker& other, CloneOptions options, a2dRefMap* refs )
    : a2dBaseMarker( other, options, refs )
{
    m_curve = other.m_curve;
}

a2dObject* a2dMarker::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dMarker( *this, options, refs );
};

void a2dMarker::CalculatePositionMarker( double& xw, double& yw )
{
    if ( m_curve )
    {
        a2dPoint2D point;
        m_curve->GetXyAtSweep( m_position, point );
        m_position = point.m_x;
        m_curvesArea->Curve2World( point.m_x, point.m_y, xw, yw );
    }
    else
    {
        xw = 0;
        yw = 0;
        m_position = 0;
    }
}

bool a2dMarker::GetPositionCurvePoint( a2dPoint2D& point )
{
    if ( m_curve )
    {
        return m_curve->GetXyAtSweep( m_position, point );
    }
    else
    {
        point.m_x = 0;
        point.m_y = 0;
        return false;
    }
}

void a2dMarker::DoAddPending( a2dIterC& WXUNUSED( ic ) )
{
//klion 27.05.2005    if ( m_flags.m_pending )
//        SetPending(true);
}

void a2dMarker::DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) )
{
    if ( !m_flags.m_pending && m_curve->GetPending() )
    {
        //wxLogDebug( "marker");
        SetPending( true );
    }
}

bool a2dMarker::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( m_curve && !m_bbox.GetValid() )
    {
        double x, y;
        CalculatePositionMarker( x, y );
        SetPosXY( x, y );

        //wxLogDebug( "on update marker %d %f %f ", GetId(), x, y );

        m_untransbbox = m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

#if wxART2D_USE_CVGIO

void a2dMarker::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dBaseMarker::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_curve )
        {
            out.WriteAttribute( wxT( "curvename" ) , m_curve->GetName() );
            out.WriteAttribute( wxT( "curve" ), m_curve->GetId() );
        }
    }
    else
    {
    }
}

void a2dMarker::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dBaseMarker::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( parser.HasAttribute( wxT( "curve" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_curve, parser.GetAttributeValue( wxT( "curve" ) ) );
        }
    }
    else
    {
    }

    //m_curve = ;
}
#endif //wxART2D_USE_CVGIO

const wxString& a2dMarker::GetMarkerString()
{
    if( !m_promptFormat.IsEmpty() )
    {
        double xc, yc;
        m_curvesArea->World2Curve( GetPosX(), GetPosY(), xc, yc );
        wxString form;
        m_promptString.Printf( m_promptFormat, xc, yc );
    }
    else if( !GetName().IsEmpty() )
        m_promptString = GetName();
    else
        m_promptString = wxT( "CurveMarker" );
    return m_promptString;
}

//----------------------------------------------------------------------------
// a2dBandMarker
//----------------------------------------------------------------------------

a2dBandMarker::a2dBandMarker( const wxString& format, a2dBandCurve* curve )
    : a2dMarker( format, curve )
{
    m_markerUpLow = true;
    m_curveBand = curve;
    m_curveBand->SetMarkerUpLow( m_markerUpLow );
}

a2dBandMarker::a2dBandMarker( const wxString& format, a2dBandCurve* curve , double atx, double height )
    : a2dMarker( format, curve, atx, height )
{
    m_markerUpLow = true;
    m_curveBand = curve;
    m_curveBand->SetMarkerUpLow( m_markerUpLow );
}

a2dBandMarker::a2dBandMarker( const wxString& format, a2dBandCurve* curve , double atx, const a2dFont& font )
    : a2dMarker( format, curve, atx, font )
{
    m_markerUpLow = true;
    m_curveBand = curve;
    m_curveBand->SetMarkerUpLow( m_markerUpLow );
}

a2dBandMarker::~a2dBandMarker()
{
}

a2dBandMarker::a2dBandMarker( const a2dBandMarker& other, CloneOptions options, a2dRefMap* refs )
    : a2dMarker( other, options, refs )
{
    m_markerUpLow = other.m_markerUpLow;
    m_curveBand = other.m_curveBand;
}

a2dObject* a2dBandMarker::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dBandMarker( *this, options, refs );
};

void a2dBandMarker::CalculatePositionMarker( double& xw, double& yw )
{
    if ( m_curve )
    {
        m_curveBand->SetMarkerUpLow( m_markerUpLow );
        a2dMarker::CalculatePositionMarker( xw, yw );
    }
    else
    {
        xw = 0;
        yw = 0;
        m_position = 0;
    }
}

bool a2dBandMarker::GetPositionCurvePoint( a2dPoint2D& point )
{
    if ( m_curve )
    {
        m_curveBand->SetMarkerUpLow( m_markerUpLow );
        a2dMarker::GetPositionCurvePoint( point );
        return true;
    }
    else
    {
        point.m_x = 0;
        point.m_y = 0;
        return false;
    }
}

#if wxART2D_USE_CVGIO

void a2dBandMarker::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dMarker::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "uplow" ) , m_markerUpLow );
    }
    else
    {
    }
}

void a2dBandMarker::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dMarker::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_markerUpLow =  parser.GetAttributeValueBool( wxT( "spline" ), false );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dBandMarker::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    a2dVertexArray cpoints;

    if ( m_flags.m_editingCopy )
        ic.GetDrawer2D()->SetDrawerFill( *a2dBLACK_FILL );

    cpoints.push_back( new a2dLineSegment( 0, 0 ) );
    cpoints.push_back( new a2dLineSegment( m_b / 2.0, m_l1 ) );
    if( m_l1 != m_l2 )
    {
        cpoints.push_back( new a2dLineSegment( 0, m_l2 ) );
    }
    cpoints.push_back( new a2dLineSegment( -m_b / 2.0, m_l1 ) );

    double xw, yw, yw2;
    CalculatePositionMarker( xw, yw2 );
    m_markerUpLow = !m_markerUpLow;
    CalculatePositionMarker( xw, yw );
    m_markerUpLow = !m_markerUpLow;
    m_curveBand->SetMarkerUpLow( m_markerUpLow );

    ic.GetDrawer2D()->DrawPolygon( &cpoints, m_spline, wxWINDING_RULE );

    ic.GetDrawer2D()->DrawLine( 0, 0, 0, yw - yw2 );

    cpoints.clear();

    if( !m_format.IsEmpty() )
    {
        double xc, yc;
        m_curvesArea->World2Curve( GetPosX(), GetPosY(), xc, yc );
        wxString form;
        form.Printf( m_format, xc, yc );

        ic.GetDrawer2D()->SetFont( m_font );
        ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );

        ic.GetDrawer2D()->DrawText( form, -m_b / 2, m_l1 + m_textheight * 2.5 );
        //ic.GetDrawer2D()->DrawText( form, 0, 0 );
        ic.GetDrawer2D()->SetFont( *a2dNullFONT );
    }
}

//----------------------------------------------------------------------------
// a2dMarkerShow
//----------------------------------------------------------------------------
#define fach 0.7
#define facv 1.78

a2dPropertyIdCanvasObject* a2dMarkerShow::PROPID_marker = NULL;
a2dPropertyIdCanvasObject* a2dMarkerShow::PROPID_xText = NULL;
a2dPropertyIdCanvasObject* a2dMarkerShow::PROPID_yText = NULL;
a2dPropertyIdCanvasObject* a2dMarkerShow::PROPID_Xlabel = NULL;
a2dPropertyIdCanvasObject* a2dMarkerShow::PROPID_Ylabel = NULL;

INITIALIZE_PROPERTIES( a2dMarkerShow, a2dCanvasObject )
{
    A2D_PROPID_D( a2dPropertyIdCanvasObject, marker, 0 );
    A2D_PROPID_D( a2dPropertyIdCanvasObject, xText, 0 );
    A2D_PROPID_D( a2dPropertyIdCanvasObject, yText, 0 );
    A2D_PROPID_D( a2dPropertyIdCanvasObject, Xlabel, 0 );
    A2D_PROPID_D( a2dPropertyIdCanvasObject, Ylabel, 0 );
    return true;
}

void a2dMarkerShow::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dCanvasObject::DoWalker( parent, handler );
    if ( m_marker )
        m_marker->Walker( this, handler );
    if ( m_xText )
        m_xText->Walker( this, handler );
    if ( m_yText )
        m_yText->Walker( this, handler );
    if ( m_Xlabel )
        m_Xlabel->Walker( this, handler );
    if ( m_Ylabel )
        m_Ylabel->Walker( this, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}

BEGIN_EVENT_TABLE( a2dMarkerShow, a2dCanvasObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dMarkerShow::OnCanvasObjectMouseEvent )
    EVT_COM_EVENT( a2dMarkerShow::OnPropertyChanged )
END_EVENT_TABLE()

a2dMarkerShow::a2dMarkerShow( double x, double y,
                              const wxString& XLabel, const wxString& YLabel,
                              const wxString& XValueformat, const wxString& YValueformat,
                              a2dMarker* marker, double height )
    : a2dCanvasObject( x, y )
{
    m_marker = marker;

    m_XValueformat = XValueformat;
    m_YValueformat = YValueformat;

    m_textheight = height;
    if ( height == 0 )
        m_textheight = 5 ;

    m_Xlabel = new a2dText( XLabel, 0, 0, a2dFont( m_textheight, wxFONTFAMILY_SWISS ) );
    Append( m_Xlabel );
    m_Xlabel->SetName( wxT( "__XLABEL__" ) );
    m_Xlabel->SetSpecificFlags( false, a2dCanvasOFlags::DRAGGABLE );

    m_Ylabel = new a2dText( YLabel, 0, -1.2 * m_textheight, a2dFont( m_textheight, wxFONTFAMILY_SWISS ) );
    Append( m_Ylabel );
    m_Ylabel->SetName( wxT( "__YLABEL__" ) );
    m_Ylabel->SetSpecificFlags( false, a2dCanvasOFlags::DRAGGABLE );

    m_xText = new a2dText( wxT( "" ), 0, 0, a2dFont( m_textheight, wxFONTFAMILY_SWISS ) );
    Append( m_xText );
    m_xText->SetName( wxT( "__XTEXT__" ) );
    m_xText->SetSpecificFlags( false, a2dCanvasOFlags::DRAGGABLE );

    m_yText = new a2dText( wxT( "" ), 0, -1.2 * m_textheight, a2dFont( m_textheight, wxFONTFAMILY_SWISS ) );
    Append( m_yText );
    m_yText->SetName( wxT( "__YTEXT__" ) );
    m_yText->SetSpecificFlags( false, a2dCanvasOFlags::DRAGGABLE );
}

a2dMarkerShow::~a2dMarkerShow()
{
}

a2dMarkerShow::a2dMarkerShow( const a2dMarkerShow& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_marker = other.m_marker;
    m_XValueformat = other.m_XValueformat;
    m_YValueformat = other.m_YValueformat;
    m_textheight = other.m_textheight;
    // the next are just quick access pointers, no cloning on those
    m_xText = ( a2dText* ) Find( wxT( "__XTEXT__" ) );
    m_xText->SetSpecificFlags( false, a2dCanvasOFlags::DRAGGABLE );
    m_yText = ( a2dText* ) Find( wxT( "__YTEXT__" ) );
    m_yText->SetSpecificFlags( false, a2dCanvasOFlags::DRAGGABLE );
    m_Xlabel = ( a2dText* ) Find( wxT( "__XLABEL__" ) );
    m_Xlabel->SetSpecificFlags( false, a2dCanvasOFlags::DRAGGABLE );
    m_Ylabel = ( a2dText* ) Find( wxT( "__YLABEL__" ) );
    m_Ylabel->SetSpecificFlags( false, a2dCanvasOFlags::DRAGGABLE );
}

a2dObject* a2dMarkerShow::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dMarkerShow( *this, options, refs );
};

a2dBoundingBox a2dMarkerShow::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;

    m_Xlabel->SetPosXY( 0.0, 0.0 );
    a2dBoundingBox Xlabelbox = m_Xlabel->GetFont().GetTextExtent( m_Xlabel->GetText(), m_Xlabel->GetAlignment() );
    Xlabelbox.Translate( m_Xlabel->GetPosX(), m_Xlabel->GetPosY() );
    bbox.Expand( Xlabelbox );
    m_Xlabel->Update( updatemask_normal );

    m_Ylabel->SetPosXY( 0.0, -1.2 * Xlabelbox.GetHeight() );
    a2dBoundingBox Ylabelbox = m_Ylabel->GetFont().GetTextExtent( m_Ylabel->GetText(), m_Ylabel->GetAlignment() );
    Ylabelbox.Translate( m_Ylabel->GetPosX(), m_Ylabel->GetPosY() );
    bbox.Expand( Ylabelbox );
    m_Ylabel->Update( updatemask_normal );

    double labelw = bbox.GetMaxX();

    m_xText->SetPosXY( labelw, 0.0 );
    a2dBoundingBox Xtextbox = m_xText->GetFont().GetTextExtent( m_xText->GetText(), m_xText->GetAlignment() );
    Xtextbox.Translate( m_xText->GetPosX(), m_xText->GetPosY() );
    bbox.Expand( Xtextbox );
    m_xText->Update( updatemask_normal );

    m_yText->SetPosXY( labelw, m_Ylabel->GetPosY() );
    a2dBoundingBox Ytextbox = m_yText->GetFont().GetTextExtent( m_yText->GetText(), m_yText->GetAlignment() );
    Ytextbox.Translate( m_yText->GetPosX(), m_yText->GetPosY() );
    bbox.Expand( Ytextbox );
    m_yText->Update( updatemask_normal );

    return bbox;
}

void a2dMarkerShow::DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) )
{
    if ( !m_flags.m_pending )
    {
        //if marker was dragged, the show marker is to be updated
        if ( m_marker && m_marker->GetPending() )
        {
            //wxLogDebug( "onmarker" );
            SetPending( true );
        }
        //if text of show marker was edited, the marker will be updated too.
        else if ( ( m_xText->GetPending() || m_yText->GetPending() ) )
        {
            //wxLogDebug( "ontext" );
            double x;
            m_xText->GetText().ToDouble( &x );
            if ( m_marker )
                m_marker->SetPosition( x );
        }
    }
}

void a2dMarkerShow::OnPropertyChanged( a2dComEvent& event )
{
    a2dCanvasObject::OnPropertyChanged( event );
    if( event.GetPropertyId() == PROPID_Stroke )
    {
        a2dStroke stroke = GetStroke();
        m_xText->SetStroke( stroke );
        m_Xlabel->SetStroke( stroke );
        m_yText->SetStroke( stroke );
        m_Ylabel->SetStroke( stroke );
    }
    else if( event.GetPropertyId() == PROPID_Fill )
    {
        a2dFill fill = GetFill();
        m_xText->SetFill( fill );
        m_Xlabel->SetFill( fill );
        m_yText->SetFill( fill );
        m_Ylabel->SetFill( fill );
    }
}

bool a2dMarkerShow::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        wxString formX;
        wxString formY;
        if ( m_marker )
        {
            a2dPoint2D point;
            m_marker->GetPositionCurvePoint( point );
            formX.Printf( m_XValueformat, point.m_x );
            formY.Printf( m_YValueformat, point.m_y );
        }
        else
        {
            formX.Printf( m_XValueformat, 0.0 );
            formY.Printf( m_YValueformat, 0.0 );
        }
        m_xText->SetText( formX );
        m_yText->SetText( formY );
        m_xText->SetStroke( GetStroke() );
        m_xText->SetFill( GetFill() );
        m_Xlabel->SetStroke( GetStroke() );
        m_Xlabel->SetFill( GetFill() );
        m_yText->SetStroke( GetStroke() );
        m_yText->SetFill( GetFill() );
        m_Ylabel->SetStroke( GetStroke() );
        m_Ylabel->SetFill( GetFill() );

        m_xText->SetPending( false );
        m_yText->SetPending( false );

        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        return true;
    }

    return false;
}

void a2dMarkerShow::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    // This first part is basically a copy of the DoGetUnTransformedBbox method.
    // It is copied here, because proper DC-font information is now known.
    wxString formX;
    wxString formY;
    if ( m_marker )
    {
        a2dPoint2D point;
        m_marker->GetPositionCurvePoint( point );
        formX.Printf( m_XValueformat, point.m_x );
        formY.Printf( m_YValueformat, point.m_y );
    }
    else
    {
        formX.Printf( m_XValueformat, 0.0 );
        formY.Printf( m_YValueformat, 0.0 );
    }

    m_xText->SetText( formX );
    m_yText->SetText( formY );
    m_xText->SetPending( false );
    m_yText->SetPending( false );

    a2dBoundingBox bbox;

    m_Xlabel->SetPosXY( 0.0, 0.0 );
    a2dBoundingBox Xlabelbox = m_Xlabel->GetFont().GetTextExtent( m_Xlabel->GetText(), m_Xlabel->GetAlignment() );
    Xlabelbox.Translate( m_Xlabel->GetPosX(), m_Xlabel->GetPosY() );
    bbox.Expand( Xlabelbox );

    m_Ylabel->SetPosXY( 0.0, -1.2 * m_Xlabel->GetLineHeight() );
    a2dBoundingBox Ylabelbox = m_Ylabel->GetFont().GetTextExtent( m_Ylabel->GetText(), m_Ylabel->GetAlignment() );
    Ylabelbox.Translate( m_Ylabel->GetPosX(), m_Ylabel->GetPosY() );
    bbox.Expand( Ylabelbox );

    double labelw = bbox.GetMaxX();

    m_xText->SetPosXY( labelw, 0.0 );
    a2dBoundingBox Xtextbox = m_xText->GetFont().GetTextExtent( m_xText->GetText(), m_xText->GetAlignment() );
    Xtextbox.Translate( m_xText->GetPosX(), m_xText->GetPosY() );
    bbox.Expand( Xtextbox );

    m_yText->SetPosXY( labelw, m_Ylabel->GetPosY() );
    a2dBoundingBox Ytextbox = m_yText->GetFont().GetTextExtent( m_yText->GetText(), m_yText->GetAlignment() );
    Ytextbox.Translate( m_yText->GetPosX(), m_yText->GetPosY() );
    bbox.Expand( Ytextbox );

    bbox.Enlarge( bbox.GetWidth() / 80.0 );
    ic.GetDrawer2D()->DrawRoundedRectangle( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth() , bbox.GetHeight(), 0 );
}

#if wxART2D_USE_CVGIO
void a2dMarkerShow::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dMarkerShow::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_xText = ( a2dText* ) Find( wxT( "__XTEXT__" ) );
        m_yText = ( a2dText* ) Find( wxT( "__YTEXT__" ) );
        m_Xlabel = ( a2dText* ) Find( wxT( "__XLABEL__" ) );
        m_Ylabel = ( a2dText* ) Find( wxT( "__YLABEL__" ) );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dMarkerShow::DoIsHitWorld( a2dIterC& WXUNUSED( ic ), a2dHitEvent& hitEvent )
{
    return true;
}


bool a2dMarkerShow::DoStartEdit( wxUint16 WXUNUSED( editmode ), wxEditStyle WXUNUSED( editstyle ) )
{
    if ( m_flags.m_editable )
    {
        //calculate bbox's else mouse events may take place when first, and idle event is not yet
        //processed to do this calculation.
        Update( updatemask_force );
        //still set it pending to do the redraw ( in place )
        SetPending( true );
        return true;
    }

    return false;
}

void a2dMarkerShow::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();
    if ( m_flags.m_editingCopy )
    {
        //to world group coordinates to do hit test in world group coordinates
        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        a2dAffineMatrix atWorld = ic->GetTransform();

        if ( event.GetMouseEvent().LeftDown() )
        {
            a2dMarkerShow* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dMarkerShow );;

            a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
            if ( IsHitWorld( *ic, hitevent ) )
            {
                a2dHitEvent hitinfo( xw, yw, false );
                hitinfo.m_xyRelToChildren = true;
                a2dCanvasObject* text = original->IsHitWorld( *ic, hitinfo );
                if ( text && text->GetEditable() )
                {
#if wxART2D_USE_EDITOR
                    a2dIterCU cu( *ic, original );
                    a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr ) ;

                    ic->SetCorridorPath( true );
                    controller->StartEditingObject( text, *ic );
#else //wxART2D_USE_EDITOR
                    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR

                    SetPending( true );
                    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
                }
                else
                    event.Skip();
            }
            else
                EndEdit();
        }
        else if ( event.GetMouseEvent().LeftDClick()  )
        {
            EndEdit();
        }
        else if ( event.GetMouseEvent().Moving() )
        {
            event.Skip();
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

//----------------------------------------------------------------------------
// a2dCursorMarkerList
//----------------------------------------------------------------------------

a2dCursorMarkerList::a2dCursorMarkerList()
{
}

a2dCursorMarkerList::~a2dCursorMarkerList()
{
}

a2dCursorMarkerList* a2dCursorMarkerList::DoClone( a2dObject::CloneOptions WXUNUSED( options ), a2dRefMap* refs ) const
{
    wxASSERT( 0 );
    return NULL;
}

//----------------------------------------------------------------------------
// a2dCursor
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dCursor, a2dCanvasObject )

BEGIN_EVENT_TABLE( a2dCursor, a2dCanvasObject )
//  EVT_CANVASOBJECT_MOUSE_EVENT( a2dCursorMarker::OnCanvasObjectMouseEvent )
END_EVENT_TABLE()

a2dCursor::a2dCursor( double csize )
    : a2dCanvasObject()
{
    m_csize = csize;
    m_hsize = 0;
    m_wsize = 0;
    m_posx = 0;
    m_posy = 0;
    m_onlyIntPosX = false;
    m_onlyIntPosY = false;
}

a2dCursor::~a2dCursor()
{
}

a2dCursor::a2dCursor( const a2dCursor& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_hsize = other.m_hsize;
    m_wsize = other.m_wsize;
    m_csize = other.m_csize;
    m_posx = other.m_posx;
    m_posy = other.m_posy;
    m_onlyIntPosX = other.m_onlyIntPosX;
    m_onlyIntPosY = other.m_onlyIntPosY;
}

a2dObject* a2dCursor::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCursor( *this, options, refs );
};

void a2dCursor::DoUpdateViewDependentTransform( a2dIterC& ic )
{
    //  a2dAffineMatrix oldTransf = m_lworld;
    a2dCanvasObject::DoUpdateViewDependentTransform( ic );
    /*
    if(oldTransf != m_lworld)
    {
    double tempwsize, oldhsize;
    oldTransf.TransformPoint(0, m_hsize, tempwsize, oldhsize);
    a2dAffineMatrix newInverseTransf = m_lworld;
    newInverseTransf.Invert();
    newInverseTransf.TransformPoint(0, oldhsize, tempwsize, m_hsize);
    }
    */
}

a2dBoundingBox a2dCursor::DoGetUnTransformedBbox( a2dBboxFlags flags )
{
    a2dBoundingBox bbox;

    double scalex = 1;
    const a2dMatrixProperty* propSpec = PROPID_IntViewDependTransform->GetPropertyListOnly( this );
    if ( propSpec )
    {
        a2dMatrixProperty* propMatrix = wxStaticCast( propSpec, a2dMatrixProperty );
        scalex = propMatrix->GetValue().Get_scaleX();
    }

    bbox.Expand( 0, 0 );
    bbox.Expand( m_csize * scalex, m_hsize );

    m_untransbbox = bbox;
    return bbox;
}

/*
bool a2dCursor::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
if ( m_curve && !m_bbox.GetValid() )
{

////        double y;
////        m_curve->CalculateYatX( GetPosX() , y );
////        SetPosXY( GetPosX(), y );

////        m_bbox=DoGetUnTransformedBbox();
////        m_bbox.MapBbox(m_lworld);

return true;
}
return false;
}
*/
#define BBOX2XYWH(bbox) (bbox).GetMinX(), (bbox).GetMinY(), (bbox).GetWidth(), (bbox).GetHeight()

void a2dCursor::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    // For debugging
//  ic.GetDrawer2D()->SetDrawerFill(a2dWHITE_FILL);
//  ic.GetDrawer2D()->DrawRoundedRectangle(BBOX2XYWH(m_untransbbox), 0);

    double scalex = 1.;
    const a2dMatrixProperty* propSpec = PROPID_IntViewDependTransform->GetPropertyListOnly( this );
    if ( propSpec )
    {
        a2dMatrixProperty* propMatrix = wxStaticCast( propSpec, a2dMatrixProperty );
        scalex = propMatrix->GetValue().Get_scaleX();
    }
    ic.GetDrawer2D()->DrawRoundedRectangle( 0, 0, m_csize * scalex, m_hsize, 0 ); //, true);
}

void a2dCursor::AddMarker( a2dBaseMarker* marker )
{
    m_markers.push_back( marker );
}

void a2dCursor::RemoveMarker( a2dBaseMarker* marker )
{
    m_markers.ReleaseObject( marker );
}

void a2dCursor::RemoveMarkers()
{
    m_markers.clear();
}

void a2dCursor::SetPosition( double posx, double posy, const a2dAffineMatrix& mat )
{
    m_posx = m_onlyIntPosX ? int( posx + 0.5 ) : posx;
    m_posy = m_onlyIntPosY ? int( posy + 0.5 ) : posy;
    forEachIn( a2dCursorMarkerList, &m_markers )
    {
        a2dBaseMarker* marker = *iter;
        marker->SetPosition( m_posx );
    }

    double xw, yw;
    mat.TransformPoint( m_posx, m_posy, xw, yw );

    SetPosXY( xw, yw );
}

bool a2dCursor::GetPosition( double& posx, double& posy )
{
    posx = m_posx;
    posy = m_posy;
    return true;
}

bool a2dCursor::CheckPosition( double posx )
{
    bool retCode = false;
    forEachIn( a2dCursorMarkerList, &m_markers )
    {
        a2dBaseMarker* baseMarker = *iter;
        a2dMarker* marker = wxDynamicCast( baseMarker, a2dMarker );
        if( marker && marker->GetCurve() )
        {
            a2dPoint2D apoint;
            if( marker->GetCurve()->GetXyAtSweep( posx, apoint ) )
                return true;
        }
    }
    return retCode;
}

void a2dCursor::UpdatePosition( double WXUNUSED( minx ), double miny, const a2dAffineMatrix& mat )
{
    double xw, yw;
    mat.TransformPoint( m_posx, m_posy, xw, yw );
    SetPosXY( xw, miny );
}

void a2dCursor::SetPending( bool pending )
{
    a2dCanvasObject::SetPending( pending );
    if( pending )
    {
        forEachIn( a2dCursorMarkerList, &m_markers )
        {
            a2dBaseMarker* marker = *iter;
            marker->SetPending( pending );
        }
    }
}

double a2dCursor::ConvertAxis2World( double percentValue )
{
    return percentValue / 100.*m_hsize;
}

