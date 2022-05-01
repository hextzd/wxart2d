/*! \file canextobj/src/canobj3d.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canobj3d.cpp,v 1.31 2008/07/19 18:29:43 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/wfstream.h>
#include <math.h>

#include "wx/canextobj/canobj3d.h"

#include "wx/editor/candoc.h"
#include "wx/canvas/polygon.h"
#include "wx/canvas/drawer.h"

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

IMPLEMENT_CLASS( a2d3DShape, a2dCanvasObject )

#define CIRCLE_STEPS 128

//----------------------------------------------------------------------------
// a2d3DShape
//----------------------------------------------------------------------------

a2d3DShape::a2d3DShape( a2dCanvasObject* toshadow, double depth, double angle ): a2dCanvasObject()
{
    m_shape = toshadow;

    SetFill( *a2dBLACK_FILL );
    SetStroke( *a2dBLACK_STROKE );
    m_depth = depth;
    m_angle3d  = wxDegToRad( angle );
}

a2d3DShape::~a2d3DShape()
{
}

a2d3DShape::a2d3DShape( const a2d3DShape& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_depth = other.m_depth;
    m_angle3d = other.m_angle3d;
    m_shape = other.m_shape;
}

a2dObject* a2d3DShape::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2d3DShape( *this, options, refs );
};

void a2d3DShape::SetExtrudeFillFromShape()
{
    SetFill( m_shape->GetFill() );
    SetPending( true );
}
void a2d3DShape::SetExtrudeStrokeFromShape()
{
    SetStroke( m_shape->GetStroke() );
    SetPending( true );
}

void a2d3DShape::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    a2dCanvasObject::DoWalker( parent, handler );
    if ( m_shape )
        m_shape->Walker( this, handler );
}

bool a2d3DShape::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    //the object does not have to be part of a group
    //calculate boundingbox here AND in group
    //since we do not know which one is reached first
    if ( m_shape->GetRoot() == m_root ) //only if part of this root
    {
        calc = m_shape->Update( mode );
        if ( ( mode & updatemask_force ) || calc || !m_bbox.GetValid() )
        {
            m_bbox.SetValid( false );
            m_bbox.Expand( m_shape->GetMappedBbox( m_lworld ) );

            a2dAffineMatrix tworld = m_lworld;
            tworld.Translate( cos( m_angle3d )*m_depth, sin( m_angle3d )*m_depth );
            m_bbox.Expand( m_shape->GetMappedBbox( tworld ) );

            calc = true;
        }
    }

    return calc;
}

void a2d3DShape::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    a2dAffineMatrix inverse =  m_lworld;
    inverse.Invert();

    a2dAffineMatrix tworld = ic.GetTransform();
    tworld *= inverse;

    a2dRectC* rec = wxDynamicCast( m_shape.Get(), a2dRectC );
    if ( rec )
    {
        a2dVertexArray* cpoints = new a2dVertexArray;
        a2dAffineMatrix tworld = ic.GetTransform();
        tworld *= m_shape->GetTransformMatrix();

        //here the Drawer gets a new relative transform
        //Every call for drawing something on it, will use it.
        ic.GetDrawer2D()->SetTransform( tworld );

        if ( m_angle3d > 0 )
        {
            cpoints->push_back( new a2dLineSegment( -rec->GetWidth() / 2,  rec->GetHeight() / 2 ) );
            cpoints->push_back( new a2dLineSegment( -rec->GetWidth() / 2 + cos( m_angle3d - wxPI / 30 )*m_depth, rec->GetHeight() / 2 + sin( m_angle3d - wxPI / 30 )*m_depth ) );
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2 + cos( m_angle3d )*m_depth, rec->GetHeight() / 2 + sin( m_angle3d - wxPI / 30 )*m_depth ) );
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2, rec->GetHeight() / 2 ) );

            ic.GetDrawer2D()->DrawPolygon( cpoints, false, wxWINDING_RULE );
            cpoints->clear();
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2, rec->GetHeight() / 2 ) );
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2 + cos( m_angle3d )*m_depth, rec->GetHeight() / 2 + sin( m_angle3d - wxPI / 30 )*m_depth ) );
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2 + cos( m_angle3d )*m_depth, -rec->GetHeight() / 2 + sin( m_angle3d )*m_depth ) );
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2, -rec->GetHeight() / 2 ) );

            ic.GetDrawer2D()->DrawPolygon( cpoints, false, wxWINDING_RULE );
        }
        else// (m_angle3d <= 0)
        {
            //set up a polygon in world coordinates that represents the cubicle
            cpoints->push_back( new a2dLineSegment( -rec->GetWidth() / 2,  -rec->GetHeight() / 2 ) );
            cpoints->push_back( new a2dLineSegment( -rec->GetWidth() / 2 + cos( m_angle3d + wxPI / 30 )*m_depth, -rec->GetHeight() / 2 + sin( m_angle3d + wxPI / 30 )*m_depth ) );
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2 + cos( m_angle3d )*m_depth, -rec->GetHeight() / 2 + sin( m_angle3d + wxPI / 30 )*m_depth ) );
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2, -rec->GetHeight() / 2 ) );

            ic.GetDrawer2D()->DrawPolygon( cpoints, false, wxWINDING_RULE );
            cpoints->clear();
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2, rec->GetHeight() / 2 ) );
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2 + cos( m_angle3d )*m_depth, rec->GetHeight() / 2 + sin( m_angle3d )*m_depth ) );
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2 + cos( m_angle3d )*m_depth, -rec->GetHeight() / 2 + sin( m_angle3d + wxPI / 30 )*m_depth ) );
            cpoints->push_back( new a2dLineSegment( rec->GetWidth() / 2, -rec->GetHeight() / 2 ) );

            ic.GetDrawer2D()->DrawPolygon( cpoints, false, wxWINDING_RULE );
        }

        cpoints->push_back( new a2dLineSegment( -rec->GetWidth() / 2,  -rec->GetHeight() / 2 ) );
        cpoints->push_back( new a2dLineSegment( -rec->GetWidth() / 2,   rec->GetHeight() / 2 ) );
        cpoints->push_back( new a2dLineSegment(  rec->GetWidth() / 2,   rec->GetHeight() / 2 ) );
        cpoints->push_back( new a2dLineSegment(  rec->GetWidth() / 2,  -rec->GetHeight() / 2 ) );

        ic.GetDrawer2D()->DrawPolygon( cpoints, false, wxWINDING_RULE );

        cpoints->clear();

        delete cpoints;
    }
}

#if wxART2D_USE_CVGIO
void a2d3DShape::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        //only write the referenced object once
        if ( m_shape->GetRoot() == m_root )
        {
            m_shape->Save( this, out, towrite );
        }
    }
}

void a2d3DShape::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
}

#endif //wxART2D_USE_CVGIO

bool a2d3DShape::DoIsHitWorld( a2dIterC& WXUNUSED( ic ), a2dHitEvent& hitEvent )
{
    hitEvent.m_how = a2dHit::stock_fill;
    return true;
    //TODO
    //return  (m_shape->IsHitWorld( cworld, x, y, total, distance, margin, layer, mask ));
}

