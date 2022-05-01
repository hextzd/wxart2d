/*! \file canvas/src/rectangle.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: rectangle.cpp,v 1.15 2009/07/24 16:35:01 titato Exp $
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
#include "wx/canvas/rectangle.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/vpath.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/canvas.h"
#include "wx/canvas/wire.h"
#include "wx/canvas/canwidget.h"

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_CLASS( a2dRectMM, a2dCanvasObject )
IMPLEMENT_CLASS( a2dWindowMM, a2dRectMM )
IMPLEMENT_CLASS( a2dRectWindowT2, a2dRectMM )
IMPLEMENT_CLASS( a2dRectWindow, a2dRectMM )

//----------------------------------------------------------------------------
// a2dRectMM
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dRectMM, a2dCanvasObject )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dRectMM::OnHandleEvent )
END_EVENT_TABLE()

a2dRectMM::a2dRectMM()
    : a2dCanvasObject()
{
    m_minx = 0;
    m_miny = 0;
    m_maxx = 0;
    m_maxy = 0;
    m_radius  = 0;
    m_contourwidth = 0;
    m_border = 0;
}

a2dRectMM::a2dRectMM( double x, double y, double w, double h , double radius, double contourwidth )
    : a2dCanvasObject( x, y )
{
    m_minx = 0;
    m_miny = 0;
    m_maxx = w;
    m_maxy = h;
    m_radius = radius;
    m_contourwidth = contourwidth;
    m_border = 0;
}

a2dRectMM::a2dRectMM( const a2dPoint2D& p1,  const a2dPoint2D& p2, double radius, double contourwidth )
    : a2dCanvasObject( p1.m_x, p1.m_y )
{
    m_minx = 0;
    m_miny = 0;
    m_maxx = p2.m_x - p1.m_x;
    m_maxy = p2.m_y - p1.m_y;
    m_radius = radius;
    m_contourwidth = contourwidth;
    m_border = 0;
}

a2dRectMM::a2dRectMM( const a2dBoundingBox& bbox, double radius, double contourwidth )
    : a2dCanvasObject( bbox.GetMinX(), bbox.GetMinY() )
{
    m_minx = bbox.GetMinX();
    m_miny = bbox.GetMinY();
    m_maxx = bbox.GetMaxX();
    m_maxy = bbox.GetMaxY();
    m_radius = radius;
    m_contourwidth = contourwidth;
    m_border = 0;
}

a2dRectMM::~a2dRectMM()
{
}

a2dRectMM::a2dRectMM( const a2dRectMM& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_minx = other.m_minx;
    m_miny = other.m_miny;
    m_maxx = other.m_maxx;
    m_maxy = other.m_maxy;
    m_radius = other.m_radius;
    m_contourwidth = other.m_contourwidth;
    m_border = other.m_border;
}

a2dObject* a2dRectMM::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dRectMM( *this, options, refs );
};

a2dVertexList* a2dRectMM::GetAsVertexList( bool& returnIsPolygon ) const
{
    returnIsPolygon = true;

    a2dVertexList* pointlist = new a2dVertexList();

    a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths();
    a2dVpath* segments = wxStaticCast( vectorpaths->front().Get(), a2dVectorPath ) ->GetSegments();
    double AberArcToPoly = m_root->GetAberArcToPolyScaled();
    segments->ConvertToLines(AberArcToPoly);
    a2dLineSegment* point;
    unsigned int i;
    for ( i = 0; i < segments->size(); i++ )
    {
        a2dVpathSegmentPtr seg = segments->Item( i );
        point = new a2dLineSegment( seg->m_x1, seg->m_y1 );
        pointlist->push_back( point );
    }

    delete vectorpaths;

    return pointlist;
}

a2dCanvasObjectList* a2dRectMM::GetAsCanvasVpaths( bool transform ) const
{
    a2dAffineMatrix pworld;
    if ( transform )
        pworld = m_lworld;

    a2dVpath* segments = new a2dVpath();
    a2dVectorPath* canpath = new a2dVectorPath( segments );
    canpath->SetStroke( this->GetStroke() );
    canpath->SetFill( this->GetFill() );
    canpath->SetLayer( m_layer );
    /*
        if (m_radius)
        {

            if ( m_radius < 0 )
            {
                double rx = -m_radius;
                if ( m_maxx-m_minxx < 0 )
                    rx = -rx;

                double ry = -m_radius;
                if ( m_maxy-m_minxy < 0 )
                    ry = -ry;

                a2dVpathSegment* seg = new a2dVpathSegment( rx, 0 , a2dPATHSEG_MOVETO );
                segments->Add(seg);

                seg = new a2dVpathArcSegment( *seg, rx, ry, 0.0, ry,  0.0, 0.0, a2dPATHSEG_ARCTO );
                segments->Add(seg);
                seg = new a2dVpathSegment( 0, m_height - ry, a2dPATHSEG_LINETO );
                segments->Add(seg);

                seg = new a2dVpathArcSegment( *seg, rx, m_height - ry, rx, m_height, 0.0, m_height, a2dPATHSEG_ARCTO );
                segments->Add(seg);
                seg = new a2dVpathSegment( m_width - rx, m_height, a2dPATHSEG_LINETO );
                segments->Add(seg);

                seg = new a2dVpathArcSegment( *seg, m_width - rx, m_height - ry, m_width, m_height - ry, m_width, m_height, a2dPATHSEG_ARCTO );
                segments->Add(seg);
                seg = new a2dVpathSegment( m_width, ry, a2dPATHSEG_LINETO );
                segments->Add(seg);

                seg = new a2dVpathArcSegment( *seg, m_width - rx, ry, m_width - rx, 0.0, m_width, 0.0, a2dPATHSEG_ARCTO, a2dPATHSEG_END_CLOSED  );
                segments->Add(seg);

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
                segments->Add(seg);

                seg = new a2dVpathArcSegment( *seg,  0.0, 0.0, -rx, 0.0, -rx, -ry, a2dPATHSEG_ARCTO );
                segments->Add(seg);
                seg = new a2dVpathSegment( -rx, m_height, a2dPATHSEG_LINETO );
                segments->Add(seg);

                seg = new a2dVpathArcSegment( *seg, 0.0, m_height, 0.0, m_height + ry, -rx, m_height + ry, a2dPATHSEG_ARCTO );
                segments->Add(seg);
                seg = new a2dVpathSegment( m_width, m_height + ry, a2dPATHSEG_LINETO );
                segments->Add(seg);

                seg = new a2dVpathArcSegment( *seg, m_width, m_height, m_width + rx, m_height, m_width + rx, m_height + ry, a2dPATHSEG_ARCTO );
                segments->Add(seg);
                seg = new a2dVpathSegment( m_width + rx, 0.0, a2dPATHSEG_LINETO );
                segments->Add(seg);

                seg = new a2dVpathArcSegment( *seg, m_width, 0.0, m_width, -ry, m_width + rx, -ry, a2dPATHSEG_ARCTO, a2dPATHSEG_END_CLOSED  );
                segments->Add(seg);

                segments->Transform( pworld );
            }
            canpath->SetContourWidth( GetContourWidth() );
            canpath->SetPathType( a2dPATH_END_SQAURE_EXT );
        }
        else
        {
            if ( GetContourWidth() )
            {
                double x,y;
                double w = GetContourWidth()/2.0;
                pworld.TransformPoint( -w, -w, x, y );
                a2dVpathSegment* seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
                segments->Add(seg);
                pworld.TransformPoint( -w , m_height + w, x, y );
                seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
                segments->Add(seg);
                pworld.TransformPoint( m_width + w , m_height + w, x, y );
                seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
                segments->Add(seg);
                pworld.TransformPoint( m_width + w, -w, x, y );
                seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
                segments->Add(seg);
                pworld.TransformPoint( -w , -w, x, y );
                seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
                segments->Add(seg);
                pworld.TransformPoint( w, w, x, y );
                seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO_NOSTROKE );
                segments->Add(seg);
                pworld.TransformPoint( m_width - w, w, x, y );
                seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO );
                segments->Add(seg);
                pworld.TransformPoint( m_width - w, m_height - w, x, y );
                seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO );
                segments->Add(seg);
                pworld.TransformPoint( w, m_height - w, x, y );
                seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO );
                segments->Add(seg);
                pworld.TransformPoint( w, w, x, y );
                seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO );
                segments->Add(seg);
                pworld.TransformPoint( -w, -w, x, y );
                seg = new a2dVpathSegment( x, y, a2dPATHSEG_LINETO_NOSTROKE, a2dPATHSEG_END_CLOSED );
                segments->Add(seg);
            }
            else
            {
                double x,y;
                pworld.TransformPoint( 0, 0, x, y );
                a2dVpathSegment* seg = new a2dVpathSegment( x, y , a2dPATHSEG_MOVETO );
                segments->Add(seg);
                pworld.TransformPoint( 0 , m_height, x, y );
                seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
                segments->Add(seg);
                pworld.TransformPoint( m_width , m_height, x, y );
                seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO );
                segments->Add(seg);
                pworld.TransformPoint( m_width , 0, x, y );
                seg = new a2dVpathSegment( x, y , a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED );
                segments->Add(seg);
            }
        }
    */
    a2dCanvasObjectList* canpathlist = new a2dCanvasObjectList();
    canpathlist->push_back( canpath );
    return canpathlist;
}


bool a2dRectMM::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        if ( editmode == 1 )
        {
            PROPID_Allowrotation->SetPropertyToObject( this, false );
            PROPID_Allowskew->SetPropertyToObject( this, false );

            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            //add properties and handles as children in order to edit the object.
            double rx = m_radius;
            if ( ( m_radius < 0 && m_maxx - m_minx < 0 ) || ( m_radius > 0 && m_maxx - m_minx > 0 ) )
                rx = -rx;

            a2dHandle* handle = new a2dHandle( this, m_minx + rx , m_miny, wxT( "__rounding__" ),
                                               Round( GetHabitat()->GetHandle()->GetWidth() * 1.5 ),
                                               Round( GetHabitat()->GetHandle()->GetHeight() * 1.5 ) );
            Append( handle );
            handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
            handle->SetLayer( m_layer );

            double radius = m_radius;
            m_radius = 0;
            double x, y, w, h;
            x = m_minx;
            y = m_miny;
            w = m_maxx - m_minx;
            h = m_maxy - m_miny;
            m_radius = radius;
            /*
                        a2dRectMM* around = new a2dRectMM( x, y, w, h );
                        around->SetStroke( *wxRED, 0, a2dSTROKE_DOT_DASH );
                        around->SetFill( *a2dTRANSPARENT_FILL );
                        around->SetPreRenderAsChild( false );
                        around->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                        around->SetHitFlags( a2dCANOBJ_EVENT_NON );
                        around->SetLayer( m_layer );
                        Append( around );
            */
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

            m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );
            m_childobjects->SetSpecificFlags( true, a2dCanvasOFlags::BIN2, wxT( "a2dHandle" ) );

            //calculate bbox's else mouse events may take place when first idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );
            //still set it pending to do the redraw ( in place )
            SetPending( true );
            return true;
        }
        else
        {
            PROPID_IncludeChildren->SetPropertyToObject( this, false );
            PROPID_Allowrotation->SetPropertyToObject( this, false );
            PROPID_Allowskew->SetPropertyToObject( this, false );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

void a2dRectMM::OnHandleEvent( a2dHandleMouseEvent& event )
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

            a2dRectMM* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dRectMM );

            double w, h;
            w = m_maxx - m_minx;
            h = m_maxy - m_miny;

            a2dAffineMatrix origworld = m_lworld;
            double radius = m_radius;
            if ( event.GetMouseEvent().LeftDown() )
            {
            }
            else if ( event.GetMouseEvent().LeftUp() )
            {
                if ( draghandle->GetName() == wxT( "__rounding__" ) )
                {
                    original->SetRadius( m_radius );
                }
                else
                {
                    original->SetMin( m_minx, m_miny );
                    original->SetMax( m_maxx, m_maxy );
                }
            }
            else if ( event.GetMouseEvent().Dragging() )
            {
                if ( restrictEngine )
                    restrictEngine->RestrictPoint( xw, yw );
                ic->GetInverseTransform().TransformPoint( xw, yw, xwi, ywi );

                if ( draghandle->GetName() == wxT( "__rounding__" ) )
                {
                    //assume outward
                    if ( ( m_maxx - m_minx < 0 && xwi < 0 ) || ( m_maxx - m_minx > 0 && xwi > 0 ) )
                    {
                        //inward
                        if ( fabs( xwi ) < fabs( ( m_maxx - m_minx ) / 2 ) && fabs( xwi ) < fabs( ( m_maxy - m_miny ) / 2 ) )
                        {
                            m_radius = -fabs( xwi );
                            draghandle->SetPosXY( xwi, m_miny );
                            SetPending( true );
                        }
                    }
                    else
                    {
                        m_radius = fabs( xwi );
                        draghandle->SetPosXY( xwi, m_miny );
                        SetPending( true );
                    }
                }
                else if ( draghandle->GetName() == wxT( "handle1" ) )
                {
                    m_minx = xwi;
                    m_miny = ywi;
                }
                else if ( draghandle->GetName() == wxT( "handle2" ) )
                {
                    m_minx = xwi;
                    m_maxy = ywi;
                }
                else if ( draghandle->GetName() == wxT( "handle3" ) )
                {
                    m_maxx = xwi;
                    m_maxy = ywi;
                }
                else if ( draghandle->GetName() == wxT( "handle4" ) )
                {
                    m_maxx = xwi;
                    m_miny = ywi;
                }
                else if ( draghandle->GetName() == wxT( "handle12" ) )
                {
                    m_minx = xwi;
                }
                else if ( draghandle->GetName() == wxT( "handle23" ) )
                {
                    m_maxy = ywi;
                }
                else if ( draghandle->GetName() == wxT( "handle34" ) )
                {
                    m_maxx = xwi;
                }
                else if ( draghandle->GetName() == wxT( "handle41" ) )
                {
                    m_miny = ywi;
                }
                else
                    event.Skip();

                SetHandlePos( wxT( "__rounding__" ), m_minx + m_radius, m_miny );
                SetHandlePos( wxT( "handle1" ), m_minx, m_miny );
                SetHandlePos( wxT( "handle2" ), m_minx, m_maxy );
                SetHandlePos( wxT( "handle3" ), m_maxx, m_maxy );
                SetHandlePos( wxT( "handle4" ), m_maxx, m_miny );
                SetHandlePos( wxT( "handle12" ), m_minx, m_miny + h / 2 );
                SetHandlePos( wxT( "handle23" ), m_minx + w / 2, m_maxy );
                SetHandlePos( wxT( "handle34" ), m_maxx, m_miny + h / 2 );
                SetHandlePos( wxT( "handle41" ), m_minx + w / 2, m_miny );
                SetPending( true );
            }
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

a2dBoundingBox a2dRectMM::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    bbox.Expand( m_minx, m_miny );
    bbox.Expand( m_maxx, m_maxy );

    if ( !( flags & a2dCANOBJ_BBOX_EDIT ) )
    {
        if ( m_radius > 0 )
            bbox.Enlarge( m_radius );

        if ( m_contourwidth > 0 )
            bbox.Enlarge( m_contourwidth / 2 );
    }
    return bbox;
}

void a2dRectMM::DoRender( a2dIterC& ic, OVERLAP clipparent )
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
        {
            ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, -m_radius );
        }
        else
        {
            ic.GetDrawer2D()->DrawRoundedRectangle( m_minx - m_radius, m_miny - m_radius, m_maxx - m_minx + 2 * m_radius, m_maxy - m_miny + 2 * m_radius, m_radius );
        }
    }
}

#if wxART2D_USE_CVGIO

void a2dRectMM::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_radius != 0.0 )
        {
            out.WriteAttribute( wxT( "minx" ), m_minx );
            out.WriteAttribute( wxT( "miny" ), m_miny );
            out.WriteAttribute( wxT( "maxx" ), m_maxx );
            out.WriteAttribute( wxT( "maxy" ), m_maxy );
            out.WriteAttribute( wxT( "radius" ), m_radius );
            if ( m_contourwidth )
                out.WriteAttribute( wxT( "contourwidth" ), m_contourwidth * out.GetScale() );
        }
    }
    else
    {
    }
}

void a2dRectMM::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_minx = parser.GetAttributeValueDouble( wxT( "minx" ) );
        m_miny = parser.GetAttributeValueDouble( wxT( "miny" ) );
        m_maxx = parser.GetAttributeValueDouble( wxT( "maxx" ) );
        m_maxy = parser.GetAttributeValueDouble( wxT( "maxy" ) );
        m_radius = parser.GetAttributeValueDouble( wxT( "radius" ) );
        m_contourwidth = parser.GetAttributeValueDouble( wxT( "contourwidth" ), 0 );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dRectMM::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double margin = ic.GetTransformedHitMargin();

    if ( m_radius > 0 )
    {
        m_minx -= m_radius;
        m_miny -= m_radius;
        m_maxx += m_radius;
        m_maxy += m_radius;
    }

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, m_minx, m_miny, m_maxx, m_maxy, ic.GetWorldStrokeExtend() + margin + m_contourwidth );

    return hitEvent.m_how.IsHit();
}

bool a2dRectMM::GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double WXUNUSED( x ), double WXUNUSED( y ), double WXUNUSED(margin) )
{
    a2dPinClass* toCreate;
    if ( GetGeneratePins() && ( toCreate = toConnectTo->GetPinClassForTask( task, this ) ) )
    {
        double width = m_maxx - m_minx;
        double height = m_maxy - m_miny;
        m_flags.m_visiblechilds = true;
        a2dPin* newPin = NULL;
        if ( !HasPinNamed( wxT( "pinc*" ), true ) )
        {
            newPin = AddPin( wxT( "pinc" ), width / 2, height / 2, a2dPin::temporaryObjectPin, toCreate );
            newPin->SetInternal( true );
        }
        if ( !HasPinNamed( wxT( "pin1" ), true ) )
            AddPin( wxT( "pin1" ), m_minx, m_miny, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin2" ), true ) )
            AddPin( wxT( "pin2" ), m_minx, m_miny + height / 2, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin3" ), true ) )
            AddPin( wxT( "pin3" ), m_minx, m_maxy, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin4" ), true ) )
            AddPin( wxT( "pin4" ), m_minx + width / 2, m_maxy, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin5" ), true ) )
            AddPin( wxT( "pin5" ), m_maxx, m_maxy, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin6" ), true ) )
            AddPin( wxT( "pin6" ), m_maxx, m_miny + height / 2 , a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin7" ), true ) )
            AddPin( wxT( "pin7" ), m_maxx, m_miny, a2dPin::temporaryObjectPin, toCreate );
        if ( !HasPinNamed( wxT( "pin8" ), true ) )
            AddPin( wxT( "pin8" ), m_maxx + width / 2, m_miny, a2dPin::temporaryObjectPin, toCreate );

        wxASSERT( HasPins() );
        m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dPin" ) );
        return true;
    }
    return false;
}


bool a2dRectMM::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( !m_bbox.GetValid() )
    {
        calc = a2dCanvasObject::DoUpdate( mode, childbox, clipbox, propbox );

        if( m_flags.m_resizeToChilds && !m_flags.m_editingCopy )
        {
            m_minx = wxMin( childbox.GetMinX(), m_minx );
            m_miny = wxMin( childbox.GetMinY(), m_miny );
            m_maxx = wxMax( childbox.GetMaxX(), m_maxx );
            m_maxy = wxMax( childbox.GetMaxY(), m_maxy );


            m_minx = childbox.GetMinX() - m_border;
            m_miny = childbox.GetMinY() - m_border;
            m_maxx = childbox.GetMaxX() + m_border;
            m_maxy = childbox.GetMaxY() + m_border;

        }
    }
    return calc;
}

//----------------------------------------------------------------------------
// a2dWindowMM
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dWindowMM, a2dCanvasObject )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dWindowMM::OnHandleEvent )
    EVT_CANVASOBJECT_ENTER_EVENT( a2dWindowMM::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT( a2dWindowMM::OnLeaveObject )
END_EVENT_TABLE()

a2dWindowMM::a2dWindowMM()
    : a2dRectMM()
{
    Init();
}

a2dWindowMM::a2dWindowMM( double x, double y, double w, double h )
    : a2dRectMM( x, y, w, h )
{
    Init();
}

a2dWindowMM::a2dWindowMM( const a2dPoint2D& p1,  const a2dPoint2D& p2 )
    : a2dRectMM( p1, p2 )
{
    Init();
}

a2dWindowMM::a2dWindowMM( const a2dBoundingBox& bbox )
    : a2dRectMM( bbox )
{
    Init();
}

a2dWindowMM::~a2dWindowMM()
{
}

a2dWindowMM::a2dWindowMM( const a2dWindowMM& other, CloneOptions options, a2dRefMap* refs )
    : a2dRectMM( other, options, refs )
{
    m_style = other.m_style;
    m_state = other.m_state;

    m_backStroke = other.m_backStroke;
    m_darkStroke = other.m_darkStroke;
    m_lightStroke = other.m_lightStroke;
    m_whiteStroke = other.m_whiteStroke;
    m_blackStroke = other.m_blackStroke;
    m_hoverStroke = other.m_hoverStroke;
}

a2dObject* a2dWindowMM::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dWindowMM( *this, options, refs );
};

void a2dWindowMM::Init()
{
    m_style = RAISED;
    m_state = NON;

    SetFill( a2dFill( wxColour( 212, 208, 200 ), a2dFILL_SOLID ) );
    m_backStroke = a2dStroke( wxColour( 212, 208, 200 ), a2dSTROKE_SOLID );
    m_darkStroke = a2dStroke( wxColour( 64, 64, 64 ), 0, a2dSTROKE_SOLID );
    m_lightStroke = a2dStroke( wxColour( 128, 128, 128 ), 0, a2dSTROKE_SOLID );
    m_hoverStroke = a2dStroke( wxColour( 255, 0, 0 ), 0, a2dSTROKE_SOLID );
    m_whiteStroke = *a2dWHITE_STROKE;
    m_blackStroke = *a2dBLACK_STROKE;
}

void a2dWindowMM::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    m_state |= HOVER;
    SetPending( true );
    event.Skip(); //e.g. for base class object tip
}

void a2dWindowMM::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    m_state &= ( ALL ^ HOVER );
    SetPending( true );
    event.Skip(); //e.g. for base class object tip
}

void a2dWindowMM::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    double oneP = ic.GetDrawer2D()->DeviceToWorldXRel( 1 );


    ic.GetDrawer2D()->OverRuleFixedStyle();
    ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_miny, 0 );
    if ( m_style & SUNKEN )
    {

        if ( m_state & FOCUS )
        {
            ic.GetDrawer2D()->SetDrawerStroke( m_lightStroke );
            ic.GetDrawer2D()->DrawLine( m_minx, m_miny + oneP, m_maxx, m_miny + oneP );
            ic.GetDrawer2D()->DrawLine( m_maxx - oneP, m_miny, m_maxx - oneP, m_maxy );
            ic.GetDrawer2D()->SetDrawerStroke( m_whiteStroke );
            ic.GetDrawer2D()->DrawLine( m_minx, m_miny, m_minx, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_minx, m_maxy, m_minx, m_maxy );
            ic.GetDrawer2D()->SetDrawerStroke( m_darkStroke );
            ic.GetDrawer2D()->DrawLine( m_maxx, m_miny, m_maxx, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_maxx, m_miny, m_minx, m_miny );
        }
        else
        {
            ic.GetDrawer2D()->SetDrawerStroke( m_backStroke );
            ic.GetDrawer2D()->DrawLine( m_minx, m_miny, m_minx, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_minx, m_maxy, m_maxx, m_maxy );

            ic.GetDrawer2D()->SetDrawerStroke( m_backStroke );
            ic.GetDrawer2D()->DrawLine( m_minx, m_miny, m_minx, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_minx, m_maxy, m_maxx, m_maxy );
            ic.GetDrawer2D()->SetDrawerStroke( m_darkStroke );
            ic.GetDrawer2D()->DrawLine( m_minx + oneP, m_miny + 2 * oneP, m_minx + oneP, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_minx + oneP, m_maxy - oneP, m_maxx, m_maxy - oneP );

            ic.GetDrawer2D()->SetDrawerStroke( m_lightStroke );
            ic.GetDrawer2D()->DrawLine( m_minx + oneP, m_miny + oneP, m_maxx, m_miny + oneP );
            ic.GetDrawer2D()->DrawLine( m_maxx - oneP, m_miny + oneP, m_maxx - oneP, m_maxy );
            ic.GetDrawer2D()->SetDrawerStroke( m_whiteStroke );
            ic.GetDrawer2D()->DrawLine( m_maxx, m_miny, m_maxx, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_minx, m_miny, m_maxx, m_miny );
        }
    }
    else
    {
        if ( m_state & FOCUS )
        {
            ic.GetDrawer2D()->SetDrawerStroke( m_blackStroke );
            ic.GetDrawer2D()->DrawLine( m_minx, m_miny, m_minx, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_maxx, m_miny, m_maxx, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_minx, m_miny, m_maxx, m_miny );
            ic.GetDrawer2D()->DrawLine( m_minx, m_maxy, m_maxx, m_maxy );
            ic.GetDrawer2D()->SetDrawerStroke( m_whiteStroke );
            ic.GetDrawer2D()->DrawLine( m_minx + oneP, m_miny + 2 * oneP, m_minx + oneP, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_minx + oneP, m_maxy - oneP, m_maxx, m_maxy - oneP );

            ic.GetDrawer2D()->SetDrawerStroke( m_lightStroke );
            ic.GetDrawer2D()->DrawLine( m_minx + 2 * oneP, m_miny + 2 * oneP, m_maxx - oneP, m_miny + 2 * oneP );
            ic.GetDrawer2D()->DrawLine( m_maxx - 2 * oneP, m_miny + 2 * oneP, m_maxx - 2 * oneP, m_maxy - oneP );

            ic.GetDrawer2D()->SetDrawerStroke( m_darkStroke );
            ic.GetDrawer2D()->DrawLine( m_minx + oneP, m_miny + oneP, m_maxx, m_miny + oneP );
            ic.GetDrawer2D()->DrawLine( m_maxx - oneP, m_miny + oneP, m_maxx - oneP, m_maxy );
        }
        else
        {
            ic.GetDrawer2D()->SetDrawerStroke( m_backStroke );
            ic.GetDrawer2D()->DrawLine( m_minx, m_miny, m_minx, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_minx, m_maxy, m_maxx, m_maxy );

            ic.GetDrawer2D()->SetDrawerStroke( m_backStroke );
            ic.GetDrawer2D()->DrawLine( m_minx, m_miny, m_minx, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_minx, m_maxy, m_maxx, m_maxy );
            ic.GetDrawer2D()->SetDrawerStroke( m_whiteStroke );
            ic.GetDrawer2D()->DrawLine( m_minx + oneP, m_miny + 2 * oneP, m_minx + oneP, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_minx + oneP, m_maxy - oneP, m_maxx, m_maxy - oneP );

            ic.GetDrawer2D()->SetDrawerStroke( m_lightStroke );
            ic.GetDrawer2D()->DrawLine( m_minx + oneP, m_miny + oneP, m_maxx, m_miny + oneP );
            ic.GetDrawer2D()->DrawLine( m_maxx - oneP, m_miny + oneP, m_maxx - oneP, m_maxy );
            ic.GetDrawer2D()->SetDrawerStroke( m_darkStroke );
            ic.GetDrawer2D()->DrawLine( m_maxx, m_miny, m_maxx, m_maxy );
            ic.GetDrawer2D()->DrawLine( m_minx, m_miny, m_maxx, m_miny );
        }
    }
    ic.GetDrawer2D()->ReStoreFixedStyle();

    if ( m_state &= HOVER )
    {
        ic.GetDrawer2D()->SetDrawerStroke( m_hoverStroke );
        ic.GetDrawer2D()->DrawLine( m_minx, m_miny, m_minx, m_maxy );
        ic.GetDrawer2D()->DrawLine( m_maxx, m_miny, m_maxx, m_maxy );
        ic.GetDrawer2D()->DrawLine( m_minx, m_miny, m_maxx, m_miny );
        ic.GetDrawer2D()->DrawLine( m_minx, m_maxy, m_maxx, m_maxy );
    }
    else
    {

    }
}

#if wxART2D_USE_CVGIO

void a2dWindowMM::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_radius != 0.0 )
        {
        }
    }
    else
    {
    }
}

void a2dWindowMM::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dWindowMM::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    return a2dRectMM::DoUpdate( mode, childbox, clipbox, propbox );
}

//----------------------------------------------------------------------------
// a2dRectWindowT2
//----------------------------------------------------------------------------

double a2dRectWindowT2::m_initialTitleHeight = 20;


const long a2dCLOSE_BUTTON = wxNewId();

BEGIN_EVENT_TABLE( a2dRectWindowT2, a2dRectMM )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dRectWindowT2::OnHandleEvent )
    EVT_BUTTON( a2dCLOSE_BUTTON, a2dRectWindowT2::OnCloseWindow )
END_EVENT_TABLE()

void a2dRectWindowT2::Init( a2dCanvasObject* parent )
{
    m_flipIn = false;//true;
    m_titleObj = new a2dCanvasObject( 0, 0 );

    m_parent = parent;
    m_close = new a2dWidgetButton( this, a2dCLOSE_BUTTON, 0, 0, m_initialTitleHeight, m_initialTitleHeight, a2dWidgetButton::ScaledContentKeepAspect );
    m_close->SetHighLightStroke( a2dStroke( wxColour( 10, 222, 215 ), a2dSTROKE_SOLID ) );
    m_close->SetHighLightFill( a2dFill( wxColour( 255, 102, 102 ), a2dFILL_SOLID ) );
    m_close->SetFill( wxColour( 195, 195, 195 ) );
    m_close->SetStroke( wxColour( 195, 0, 0 ) );
    m_close->SetName( wxT( "__CLOSE_BUTTON__" ) );
    m_close->SetDraggable( false );

    a2dSLine* ll = new a2dSLine( 0, 0, m_initialTitleHeight, m_initialTitleHeight );
    ll->SetStroke( wxColour( 252, 0, 252 ), 1.0 );
    a2dSLine* ll2 = new a2dSLine( 0, m_initialTitleHeight, m_initialTitleHeight, 0 );
    ll2->SetStroke( wxColour( 252, 0, 252 ), 1.0 );
    a2dCanvasObject* cont = new a2dCanvasObject( 0, 0 );
    cont->Append( ll );
    cont->Append( ll2 );
    m_close->SetContentObject( cont );

    //a2dText* tip = m_close->SetObjectTip( _T("close this window object") , 0, 0, 10 );
    //tip->SetFill( wxColour( 195,5,195) );

    m_titleFill = *a2dWHITE_FILL;
    m_titleStroke = *a2dBLACK_STROKE;

    m_title = new a2dText( wxT( "" ), 0, 0, a2dFont( m_initialTitleHeight, wxFONTFAMILY_SWISS ) );
    m_title->SetName( wxT( "__TITLE__" ) );
    m_title->SetDraggable( false );
    m_title->SetSubEditAsChild( true );

    m_titleObj->Append( m_close );
    m_titleObj->Append( m_title );

}


a2dRectWindowT2::a2dRectWindowT2()
    : a2dRectMM()
{
    Init( NULL );
}

a2dRectWindowT2::a2dRectWindowT2( a2dCanvasObject* parent, double x, double y, double w, double h , double radius )
    : a2dRectMM( x, y, w, h, radius, 0 )
{
    Init( parent );
}

a2dRectWindowT2::a2dRectWindowT2( a2dCanvasObject* parent, const a2dPoint2D& p1,  const a2dPoint2D& p2, double radius )
    : a2dRectMM( p1, p2, radius, 0 )
{
    Init( parent );
}

a2dRectWindowT2::a2dRectWindowT2( a2dCanvasObject* parent, const a2dBoundingBox& bbox, double radius )
    : a2dRectMM( bbox, radius, 0 )
{
    Init( parent );
}

a2dRectWindowT2::~a2dRectWindowT2()
{
}

a2dRectWindowT2::a2dRectWindowT2( const a2dRectWindowT2& other, CloneOptions options, a2dRefMap* refs )
    : a2dRectMM( other, options, refs )
{
    m_titleFill = other.m_titleFill;
    m_titleStroke = other.m_titleStroke;
    m_parent = other.m_parent;
    m_flipIn = other.m_flipIn;
    m_titleObj = other.m_titleObj->TClone( clone_flat );

    m_title = ( a2dText* ) m_titleObj->Find( wxT( "__TITLE__" ) );
    m_close = ( a2dWidgetButton* ) m_titleObj->Find( wxT( "__CLOSE_BUTTON__" ) );
    m_close->SetParent( this );
}

a2dObject* a2dRectWindowT2::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dRectWindowT2( *this, options, refs );
};

void a2dRectWindowT2::SetTitle( const wxString& title )
{
    a2dText* textobj = ( a2dText* ) m_titleObj->Find( _T( "__TITLE__" ) );
    if ( title.IsEmpty() )
        ReleaseChild( textobj );
    else
    {
        if ( !textobj )
        {
            textobj = new a2dText( title, 0, 0, a2dFont( 10, wxFONTFAMILY_SWISS ), 0 );
            textobj->SetDraggable( false );
            Append( textobj );
            textobj->SetName( _T( "__TITLE__" ) );
        }
        else
            textobj->SetText( title );
    }
}

void a2dRectWindowT2::SetTitle( a2dText* textObj )
{
    m_titleObj->ReleaseChild( m_title );
    m_title = textObj;
    m_titleObj->Append( m_title );
    m_title->SetName( _T( "__TITLE__" ) );
};

void a2dRectWindowT2::OnCloseWindow( wxCommandEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        a2dCanvasObject* original = PROPID_Original->GetPropertyValue( this );
        //delete object from document (actually move to command for redo)
        m_root->GetCanvasCommandProcessor()->Submit(
            new a2dCommand_ReleaseObject( m_parent, original, false )
        );
        EndEdit();
    }
    else
    {
        m_root->GetCanvasCommandProcessor()->Submit(
            new a2dCommand_ReleaseObject( m_parent, this, false )
        );
    }
}

bool a2dRectWindowT2::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        PROPID_IncludeChildren->SetPropertyToObject( this, false );
        PROPID_Allowrotation->SetPropertyToObject( this, false );
        PROPID_Allowskew->SetPropertyToObject( this, false );

        return a2dRectMM::DoStartEdit( editmode, editstyle );
    }

    return false;
}

void a2dRectWindowT2::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dRectMM::DoWalker( parent, handler );

    m_titleObj->Walker( this, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}

bool a2dRectWindowT2::ProcessCanvasObjectEvent( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    a2dRectMM::ProcessCanvasObjectEvent( ic, hitEvent );
    a2dIterCU cu( ic, m_lworld );
    m_titleObj->ProcessCanvasObjectEvent( ic, hitEvent );
    return hitEvent.m_processed;
}

void a2dRectWindowT2::DoAddPending( a2dIterC& ic )
{
    a2dRectMM::DoAddPending( ic );
    m_titleObj->AddPending( ic );
}

bool a2dRectWindowT2::Update( a2dCanvasObject::UpdateMode mode )
{
    m_titleObj->Update( mode );
    return a2dRectMM::Update( mode );
}

void a2dRectWindowT2::Render( a2dIterC& ic, OVERLAP clipparent )
{
    if ( !m_flipIn )
        a2dRectMM::Render( ic, clipparent );

    a2dIterCU cu( ic, m_lworld );
    m_titleObj->Render( ic, clipparent );
}

void a2dRectWindowT2::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    a2dStroke current = ic.GetDrawer2D()->GetDrawerStroke();
    a2dFill fillcurrent = ic.GetDrawer2D()->GetDrawerFill();

    ic.GetDrawer2D()->SetDrawerFill( m_titleFill );
    ic.GetDrawer2D()->SetDrawerStroke( m_titleStroke );

    if ( m_radius <= 0 )
    {
        ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_maxy - m_titleheight, m_maxx - m_minx, m_titleheight, 0 );
        ic.GetDrawer2D()->SetDrawerStroke( current );
        if ( !m_flipIn )
        {
            ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
            ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_titleheight - m_miny, -m_radius );
        }
    }
    else
    {
        ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_maxy - m_titleheight, m_maxx - m_minx, m_titleheight, 0 );
        ic.GetDrawer2D()->SetDrawerStroke( current );
        if ( !m_flipIn )
        {
            ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
            ic.GetDrawer2D()->DrawRoundedRectangle( m_minx - m_radius, m_miny - m_radius, m_maxx - m_minx + 2 * m_radius, m_maxy - m_miny + 2 * m_radius, m_radius );
        }
    }
}

#if wxART2D_USE_CVGIO

void a2dRectWindowT2::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dRectMM::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dRectWindowT2::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dRectMM::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dRectWindowT2::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( !m_bbox.GetValid() )
    {
        calc = a2dRectMM::DoUpdate( mode, childbox, clipbox, propbox );

        m_title->SetPosXY( m_minx, m_maxy );
        a2dBoundingBox titlebox = m_title->GetFont().GetTextExtent( m_title->GetText(), m_title->GetAlignment() );
        titlebox.Translate( m_title->GetPosX(), m_title->GetPosY() );
        m_title->Update( updatemask_normal );
        m_titleheight = titlebox.GetHeight();
        m_close->SetPosXY( m_maxx - m_close->GetWidth(), m_maxy - m_titleheight );
        m_close->SetHeight( m_titleheight );

        if( m_flags.m_resizeToChilds && !m_flags.m_editingCopy )
        {
            m_minx = wxMin( childbox.GetMinX(), m_minx );
            m_miny = wxMin( childbox.GetMinY(), m_miny );
            m_maxx = wxMax( childbox.GetMaxX(), m_maxx );
            m_maxy = wxMax( childbox.GetMaxY(), m_maxy );
            /*
                        m_minx = childbox.GetMinX();
                        m_miny = childbox.GetMinY();
                        m_maxx = childbox.GetMaxX();
                        m_maxy = childbox.GetMaxY();
                        childbox.Dump();
            */
        }
    }
    return calc;
}










//----------------------------------------------------------------------------
// a2dRectWindow
//----------------------------------------------------------------------------

double a2dRectWindow::m_initialTitleHeight = 20;

BEGIN_EVENT_TABLE( a2dRectWindow, a2dWindowMM )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dRectWindow::OnHandleEvent )
    EVT_BUTTON( a2dCLOSE_BUTTON, a2dRectWindow::OnCloseWindow )
END_EVENT_TABLE()

void a2dRectWindow::Init( a2dCanvasObject* parent )
{
    m_parent = parent;
    m_close = new a2dWidgetButton( this, a2dCLOSE_BUTTON, 0, 0, m_initialTitleHeight, m_initialTitleHeight );
    Append( m_close );
    m_close->SetHighLightStroke( a2dStroke( wxColour( 10, 222, 215 ), a2dSTROKE_SOLID ) );
    m_close->SetHighLightFill( a2dFill( wxColour( 255, 102, 102 ), a2dFILL_SOLID ) );
    m_close->SetFill( wxColour( 195, 195, 195 ) );
    m_close->SetStroke( wxColour( 195, 0, 0 ) );
    m_close->SetName( wxT( "__CLOSE_BUTTON__" ) );
    m_close->SetDraggable( false );
    //a2dText* tip = m_close->SetObjectTip( _T("close this window object") , 0, 0, 10 );
    //tip->SetFill( wxColour( 195,5,195) );

    m_titleFill = *a2dWHITE_FILL;
    m_titleFill = a2dFill( wxColour( 0, 202, 202 ), a2dFILL_SOLID );
    m_titleStroke = *a2dBLACK_STROKE;

    m_title = new a2dText( wxT( "" ), 0, 0, a2dFont( m_initialTitleHeight, wxFONTFAMILY_SWISS ) );
    Append( m_title );
    m_title->SetName( wxT( "__TITLE__" ) );
    m_title->SetDraggable( false );
    m_title->SetSubEditAsChild( true );
    m_title->SetMultiLine( false );

    m_canvas = new a2dWindowMM( 0, 0, 300, 310 );
    m_canvas->SetBorder( 20 );
    m_canvas->SetResizeOnChildBox( true );
    m_canvas->SetSubEdit( true );
    m_canvas->SetName( wxT( "__CANVAS__" ) );
    m_canvas->SetSubEdit( true );
    m_canvas->SetSubEditAsChild( true );
    Append( m_canvas );
}


a2dRectWindow::a2dRectWindow()
    : a2dWindowMM()
{
    Init( NULL );
}

a2dRectWindow::a2dRectWindow( a2dCanvasObject* parent, double x, double y, double w, double h )
    : a2dWindowMM( x, y, w, h )
{
    Init( parent );
}

a2dRectWindow::a2dRectWindow( a2dCanvasObject* parent, const a2dPoint2D& p1,  const a2dPoint2D& p2 )
    : a2dWindowMM( p1, p2 )
{
    Init( parent );
}

a2dRectWindow::a2dRectWindow( a2dCanvasObject* parent, const a2dBoundingBox& bbox )
    : a2dWindowMM( bbox )
{
    Init( parent );
}

a2dRectWindow::~a2dRectWindow()
{
}

a2dRectWindow::a2dRectWindow( const a2dRectWindow& other, CloneOptions options, a2dRefMap* refs )
    : a2dWindowMM( other, options, refs )
{
    m_titleFill = other.m_titleFill;
    m_titleStroke = other.m_titleStroke;
    m_parent = other.m_parent;

    m_title = ( a2dText* ) Find( wxT( "__TITLE__" ) );
    m_close = ( a2dWidgetButton* ) Find( wxT( "__CLOSE_BUTTON__" ) );
    m_close->SetParent( this );
    m_canvas = ( a2dWindowMM* ) Find( wxT( "__CANVAS__" ) );
    m_canvas->SetSubEdit( true );
    m_canvas->SetSubEditAsChild( true );
}

a2dObject* a2dRectWindow::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dRectWindow( *this, options, refs );
};

void a2dRectWindow::SetTitle( const wxString& title )
{
    a2dText* textobj = ( a2dText* ) Find( _T( "__TITLE__" ) );
    if ( title.IsEmpty() )
        ReleaseChild( textobj );
    else
    {
        if ( !textobj )
        {
            textobj = new a2dText( title, 0, 0, a2dFont( 10, wxFONTFAMILY_SWISS ), 0 );
            textobj->SetDraggable( false );
            Append( textobj );
            textobj->SetName( _T( "__TITLE__" ) );
        }
        else
            textobj->SetText( title );
    }
}

void a2dRectWindow::SetTitle( a2dText* textObj )
{
    ReleaseChild( m_title );
    m_title = textObj;
    Append( m_title );
    m_title->SetName( _T( "__TITLE__" ) );
};

void a2dRectWindow::SetCanvas( a2dWindowMM* canvas )
{
    ReleaseChild( m_canvas );
    m_canvas = canvas;
    Append( m_canvas );
    m_canvas->SetName( _T( "__CANVAS__" ) );
};

void a2dRectWindow::OnCloseWindow( wxCommandEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        a2dCanvasObject* original = PROPID_Original->GetPropertyValue( this );
        //delete object from document (actually move to command for redo)
        m_root->GetCanvasCommandProcessor()->Submit(
            new a2dCommand_ReleaseObject( m_parent, original, false )
        );
        EndEdit();
    }
    else
    {
        m_root->GetCanvasCommandProcessor()->Submit(
            new a2dCommand_ReleaseObject( m_parent, this, false )
        );
    }
}

bool a2dRectWindow::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        PROPID_IncludeChildren->SetPropertyToObject( this, false );
        PROPID_Allowrotation->SetPropertyToObject( this, false );
        PROPID_Allowskew->SetPropertyToObject( this, false );

        return a2dWindowMM::DoStartEdit( editmode, editstyle );
    }

    return false;
}

void a2dRectWindow::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    a2dStroke current = ic.GetDrawer2D()->GetDrawerStroke();
    a2dFill fillcurrent = ic.GetDrawer2D()->GetDrawerFill();

    ic.GetDrawer2D()->SetDrawerFill( m_titleFill );
    ic.GetDrawer2D()->SetDrawerStroke( m_titleStroke );

    if ( m_radius <= 0 )
    {
        ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_maxy - m_titleheight, m_maxx - m_minx, m_titleheight, 0 );
        ic.GetDrawer2D()->SetDrawerStroke( current );
        ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
        ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_miny, m_maxx - m_minx, m_maxy - m_titleheight - m_miny, -m_radius );
    }
    else
    {
        ic.GetDrawer2D()->DrawRoundedRectangle( m_minx, m_maxy - m_titleheight, m_maxx - m_minx, m_titleheight, 0 );
        ic.GetDrawer2D()->SetDrawerStroke( current );
        ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
        ic.GetDrawer2D()->DrawRoundedRectangle( m_minx - m_radius, m_miny - m_radius, m_maxx - m_minx + 2 * m_radius, m_maxy - m_miny + 2 * m_radius, m_radius );
    }
}

#if wxART2D_USE_CVGIO

void a2dRectWindow::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dWindowMM::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dRectWindow::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dWindowMM::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dRectWindow::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( !m_bbox.GetValid() )
    {
        calc = a2dRectMM::DoUpdate( mode, childbox, clipbox, propbox );

        m_title->SetPosXY( m_minx, m_maxy );
        a2dBoundingBox titlebox = m_title->GetFont().GetTextExtent( m_title->GetText(), m_title->GetAlignment() );
        //titlebox.Translate( m_title->GetPosX(), m_title->GetPosY() );
        m_title->Update( updatemask_normal );
        m_titleheight = titlebox.GetHeight();
        m_close->SetHeight( m_titleheight );

        a2dBoundingBox canvasbox = m_canvas->GetBbox();

        if( m_flags.m_resizeToChilds && !m_flags.m_editingCopy )
        {
            m_minx = wxMin( canvasbox.GetMinX(), m_minx );
            m_miny = wxMin( canvasbox.GetMinY(), m_miny );
            m_maxx = wxMax( canvasbox.GetMaxX(), m_maxx );
            m_maxy = wxMax( canvasbox.GetMaxY(), m_maxy );

            m_minx = canvasbox.GetMinX();
            m_miny = canvasbox.GetMinY();
            m_maxx = canvasbox.GetMaxX();
            m_maxy = canvasbox.GetMaxY() + m_titleheight;

            m_title->SetPosXY( m_minx, m_maxy );
            m_close->SetPosXY( m_maxx - m_close->GetWidth(), m_maxy - m_titleheight );

            /*
                        m_minx = wxMin( childbox.GetMinX(), m_minx );
                        m_miny = wxMin( childbox.GetMinY(), m_miny );
                        m_maxx = wxMax( childbox.GetMaxX(), m_maxx );
                        m_maxy = wxMax( childbox.GetMaxY(), m_maxy );
            */
            /*
                        m_minx = childbox.GetMinX();
                        m_miny = childbox.GetMinY();
                        m_maxx = childbox.GetMaxX();
                        m_maxy = childbox.GetMaxY();
                        childbox.Dump();
            */
        }
    }
    return calc;
}
