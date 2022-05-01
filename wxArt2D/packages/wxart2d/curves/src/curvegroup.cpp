/*! \file curves/src/curvegroup.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: curvegroup.cpp,v 1.42 2008/08/19 23:17:11 titato Exp $
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

#include "wx/curves/meta.h"

#include "wx/canvas/canglob.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/canvas.h"

#if wxART2D_USE_EDITOR
#include "wx/canvas/edit.h"
#endif //wxART2D_USE_EDITOR

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dCurvesArea, a2dPolygonLClipper )
IMPLEMENT_DYNAMIC_CLASS( a2dCurvesAreaList, a2dObject )
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasXYDisplayGroupAreas, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasXYDisplayGroup, a2dCanvasXYDisplayGroupAreas )


#ifdef wxUSE_INTPOINT
#define wxMAX_COORDINATE INT_MAX
#define wxMIN_COORDINATE INT_MIN
#else
#define wxMAX_COORDINATE 100e99
#define wxMIN_COORDINATE 100e-99
#endif

a2dBboxHash::a2dBboxHash( void )
    : a2dObject()
{
}
a2dBboxHash::a2dBboxHash( const a2dBboxHash& other, CloneOptions options, a2dRefMap* refs )
    : a2dObject( other, options, refs )
{
}

a2dObject* a2dBboxHash::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dBboxHash( *this, options, refs );
}


a2dPropertyIdCanvasObject* a2dCurvesArea::PROPID_curvesarea = NULL;
a2dPropertyIdString* a2dCurvesArea::PROPID_text = NULL;


INITIALIZE_PROPERTIES( a2dCurvesArea, a2dPolygonLClipper )
{
    A2D_PROPID_D( a2dPropertyIdCanvasObject, curvesarea, 0 );
    A2D_PROPID_D( a2dPropertyIdString, text,  wxT( "" ) );
    return true;
}

a2dCurvesArea::a2dCurvesArea( const wxString curveAreaName )
    : a2dPolygonLClipper(), m_group( NULL )
{
    m_showyaxis = true;
    SetName( curveAreaName );

    a2dBoundingBox extbox( 0, 0, 1, 1 );
    SetInternalBoundaries( extbox, 0, 0, 1, 1 );

    m_axisY = new a2dCurveAxisLin( 0, true );
    m_axisY->SetCurvesArea( this );
    m_colour = wxNullColour;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in other.m_axesarea->Clone()
    CurrentSmartPointerOwner = this;
#endif
}

a2dCurvesArea::a2dCurvesArea( const a2dCurvesArea& other, CloneOptions options, a2dRefMap* refs )
    : a2dPolygonLClipper( other, options, refs ), m_group( NULL )
{
    m_showyaxis = other.m_showyaxis;

    if( options & clone_members )
        m_axisY = ( a2dCurveAxis* ) other.m_axisY->Clone( CloneOptions( options & ~ clone_seteditcopy ) );
    else
        m_axisY = other.m_axisY;
    m_axisY->SetCurvesArea( this );

    m_intrect = other.m_intrect;
    m_colour = other.m_colour;

    a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin();
    while( iter != GetChildObjectList()->end() )
    {
        a2dCurveObject* item = wxDynamicCast( ( *iter ).Get(), a2dCurveObject );
        if ( item )
            item->SetCurvesArea( this );
        ++iter;
    }

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in other.m_axesarea->Clone()
    CurrentSmartPointerOwner = this;
#endif
}

a2dCurvesArea::~a2dCurvesArea()
{
    if( m_markerShow2 != ( a2dMarkerShow* )NULL )
    {
        m_markerShow2->SetRelease( true );
    }
    if( m_markerShow != ( a2dMarkerShow* )NULL )
    {
        m_markerShow->SetRelease( true );
    }
    m_axisY->ReleaseChildObjects();
}

a2dObject* a2dCurvesArea::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCurvesArea( *this, options, refs );
}

wxString a2dCurvesArea::GetAxisText() const
{
    //first search if a reserved property exists
    const a2dNamedProperty* p = PROPID_text->GetPropertyListOnly( this );
    if ( p )
        return p->StringValueRepresentation();
    return GetName();
}

bool a2dCurvesArea::SetAxisText( const wxString& name )
{
    PROPID_text->SetPropertyToObject( this, name );
    return true;
}

void a2dCurvesArea::World2Curve( double xw, double yw, double& xcurve, double& ycurve )
{
    a2dAffineMatrix mat = m_iworld;
    mat.Invert();
    mat.TransformPoint( xw, yw, xcurve, ycurve );
}

void a2dCurvesArea::Curve2World( double xcurve, double ycurve, double& xw, double& yw )
{
    m_iworld.TransformPoint( xcurve, ycurve, xw, yw );
}

void a2dCurvesArea::SetBoundaries( const a2dBoundingBox& extbox )
{
    a2dBoundingBox box(  extbox.GetMinX() , extbox.GetMinY(), extbox.GetMaxX() , extbox.GetMaxY() );
    SetClippingFromBox( box );
}

void a2dCurvesArea::UpdateInternalBoundaries( const a2dBoundingBox& extbox )
{
    m_iworld.Identity();
    m_iworld.Translate( m_intrect.GetMinX() - extbox.GetMinX(), m_intrect.GetMinY() - extbox.GetMinY() );

    wxASSERT_MSG( extbox.GetWidth() != 0, wxT( "pixel size can not be zero" ) );
    wxASSERT_MSG( extbox.GetHeight() != 0, wxT( "pixel size can not be zero" ) );

    m_iworld.Scale( m_intrect.GetWidth() / extbox.GetWidth(), m_intrect.GetHeight() / extbox.GetHeight(),
                    m_intrect.GetMinX(), m_intrect.GetMinY() );
    //the curves are directly transformed to world coordinates by setting the
    //inverse to the axisarea.
    m_iworld.Invert();
    if( m_axisY )
        m_axisY->SetPending( true );
    SetPending( true );
}

void a2dCurvesArea::AddCurve( a2dCurve* curve, const wxString curvename )
{
    if ( !curvename.IsEmpty() )
        curve->SetName( curvename );
    Append( curve );
    curve->SetCurvesArea( this );
}

void a2dCurvesArea::AddMarker( a2dMarker* marker )
{
    Append( marker );
    marker->SetCurvesArea( this );
}

void a2dCurvesArea::RemoveMarker( a2dMarker* marker )
{
    ReleaseChild( marker );
}

void a2dCurvesArea::InsertCurve( size_t before, a2dCurve* curve, const wxString curvename )
{
    if ( !curvename.IsEmpty() )
        curve->SetName( curvename );

    Insert( before, curve );
}

a2dCurve* a2dCurvesArea::GetCurve( const wxString curvename )
{
    forEachIn( a2dCanvasObjectList, GetChildObjectList() )
    {
        a2dCanvasObject* obj = *iter;
        if( obj->GetName() == curvename )
            return ( a2dCurve* )obj;
    }
    return NULL;
}

a2dBoundingBox a2dCurvesArea::GetCurvesBoundaries()
{
    a2dBoundingBox curvebox;

    forEachIn( a2dCanvasObjectList, GetChildObjectList() )
    {
        a2dCurve* curve = wxDynamicCast( ( a2dCanvasObject* ) * iter, a2dCurve );
        if ( curve )
            curvebox.Expand( curve->GetCurveBoundaries() );
    }

    if ( !curvebox.GetValid() ) //no child or empty child secure
    {
        //no objects, therefore make the bounding box the x,y of this object
        curvebox.Expand( 0, 0 );
    }

    return curvebox;
}

bool a2dCurvesArea::ProcessCanvasObjectEvent( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    a2dPolygonLClipper::ProcessCanvasObjectEvent( ic, hitEvent );
    m_axisY->ProcessCanvasObjectEvent( ic, hitEvent );
    return hitEvent.m_processed;
}

void a2dCurvesArea::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dPolygonLClipper::DoWalker( parent, handler );

    if ( m_axisY )
        m_axisY->Walker( this, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}

a2dCanvasObject* a2dCurvesArea::IsHitCurvesWorld ( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    return IsHitWorld( ic, hitEvent );
}


a2dBoundingBox& a2dCurvesArea::Expand( a2dBoundingBox& bbox )
{
    bbox.Expand( m_axisY->GetBbox() );
    return bbox;
}

void a2dCurvesArea::DoAddPending( a2dIterC& ic )
{
    a2dPolygonLClipper::DoAddPending( ic );
    m_axisY->AddPending( ic );
}

void a2dCurvesArea::DependencyPending( a2dWalkerIOHandler* handler )
{
    /*
        a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin();
        while( iter != GetChildObjectList()->end() )
        {
            a2dCurveObject* item = wxDynamicCast( (*iter).Get(), a2dCurveObject );
            if ( item )
                item->SetCurvesArea( this );
            ++iter;
        }

        iter = m_axisY->GetChildObjectList()->begin();
        while( iter != m_axisY->GetChildObjectList()->end() )
        {
            a2dCurveObject* item = wxDynamicCast( (*iter).Get(), a2dCurveObject );
            if ( item )
                item->SetCurvesArea( this );
            ++iter;
        }
    */

    m_axisY->DependencyPending( handler );
    if( m_axisY->GetPending() && !GetPending() )
        SetPending( true );
    a2dPolygonLClipper::DependencyPending( handler );
    if( GetPending() && m_group && !m_group->GetPending() )
        m_group->SetPending( true );
}

/*
bool a2dCurvesArea::GetPending(void)
{
    bool retCode = false;
    retCode = retCode || a2dPolygonLClipper::GetPending( );
    retCode = retCode || m_axisY->GetPending( );
    return retCode;
}
*/

void a2dCurvesArea::DoUpdateViewDependentObjects( a2dIterC& ic )
{
    a2dPolygonLClipper::DoUpdateViewDependentObjects( ic );
    m_axisY->UpdateViewDependentObjects( ic );
}


bool a2dCurvesArea::Update( UpdateMode mode )
{
    /*
        a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin();
        while( iter != GetChildObjectList()->end() )
        {
            a2dCurveObject* item = wxDynamicCast( (*iter).Get(), a2dCurveObject );
            if ( item )
                item->SetCurvesArea( this );
            ++iter;
        }
        iter = m_axisY->GetChildObjectList()->begin();
        while( iter != m_axisY->GetChildObjectList()->end() )
        {
            a2dCurveObject* item = wxDynamicCast( (*iter).Get(), a2dCurveObject );
            if ( item )
                item->SetCurvesArea( this );
            ++iter;
        }
    */
    bool retCode = a2dPolygonLClipper::Update( mode );
    retCode = m_axisY->Update( mode ) || retCode;
    return retCode;
}


bool a2dCurvesArea::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    /*
        a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin();
        while( iter != GetChildObjectList()->end() )
        {
            a2dCurveObject* item = wxDynamicCast( (*iter).Get(), a2dCurveObject );
            if ( item )
                item->SetCurvesArea( this );
            ++iter;
        }
        iter = m_axisY->GetChildObjectList()->begin();
        while( iter != m_axisY->GetChildObjectList()->end() )
        {
            a2dCurveObject* item = wxDynamicCast( (*iter).Get(), a2dCurveObject );
            if ( item )
                item->SetCurvesArea( this );
            ++iter;
        }
    */
//  bool retCode = m_axisY->Update(mode);
    bool retCode = false;

    //curren state of those flags are known after m_axesarea->Update() etc are called.
    m_flags.m_HasToolObjectsBelow = m_flags.m_HasToolObjectsBelow
                                    || m_axisY->GetHasToolObjectsBelow();

    m_flags.m_HasSelectedObjectsBelow = m_flags.m_HasSelectedObjectsBelow
                                        || m_axisY->GetHasSelectedObjectsBelow();
    return retCode;
}

void a2dCurvesArea::SetAxes( const a2dBoundingBox& extbox, double x, double y, double& zerox, double& zeroy, double& ixmin, double& ixmax )
{
    GetCurveAreaTransform().TransformPoint( x, y, zerox, zeroy );
    ixmin = m_intrect.GetMinX();
    ixmax = m_intrect.GetMaxX();

    if ( zerox < extbox.GetMinX() )
        zerox = extbox.GetMinX();
    else if ( zerox > extbox.GetMaxX() )
        zerox = extbox.GetMaxX();

    m_axisY->SetPosXY( zerox, extbox.GetMinY() );
    m_axisY->SetLength( extbox.GetHeight() );

    m_axisY->SetBoundaries( m_intrect.GetMinY(), m_intrect.GetMaxY() );
    m_axisY->Update( a2dCanvasObject::updatemask_normal );
    if( m_group && !m_group->GetPending() )
        m_group->SetPending( true );
}

bool a2dCurvesArea::IsCurvesHighlighted() const
{
    const a2dCanvasObjectList* curvesList = GetCurves();
    const_forEachIn( a2dCanvasObjectList, curvesList )
    {
        a2dCurve* curve = wxDynamicCast( ( a2dCanvasObject* ) * iter, a2dCurve );
        if ( curve && curve->IsHighlighted() )
            return true;
    }
    return false;
}

void a2dCurvesArea::SetMarkerShow( a2dMarkerShow* showm )
{
    m_markerShow = showm;
}


void a2dCurvesArea::SetMarkerShow2( a2dMarkerShow* showm )
{
    m_markerShow2 = showm;
}

a2dMarker* a2dCurvesArea::GetCursorMarker() const
{
    const_forEachIn( a2dCanvasObjectList, GetChildObjectList() )
    {
        a2dMarker* marker = wxDynamicCast( ( a2dCanvasObject* ) * iter, a2dMarker );
        if ( marker )
            return marker;
    }
    return NULL;
}

void a2dCurvesArea::SetGroup( a2dCanvasXYDisplayGroupAreas* aGroup )
{
    m_group = aGroup;
    if( m_group )
    {
        UpdateInternalBoundaries( m_group->GetPlotAreaRect() );
        if( m_group->GetRoot() )
        {
            SetRoot( m_group->GetRoot() );
            if( m_axisY )
                m_axisY->SetRoot( m_group->GetRoot() );
        }
    }
}

void a2dCurvesArea::SetShowYaxis( bool showyaxis )
{
    m_showyaxis = showyaxis;
    if( m_axisY ) m_axisY->SetPending( true );
    SetPending( true );
//  if(m_group) m_group->SetPending(true);
}


#if wxART2D_USE_CVGIO
void a2dCurvesArea::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite )
{
    a2dPolygonLClipper::DoSave( parent, out, xmlparts, towrite );
    //TODO: make serializing for m_colour, m_markerShow and m_markerShow2
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "showyaxis" ) , m_showyaxis );
        out.WriteNewLine();

        out.WriteAttribute( wxT( "ixmin" ) , m_intrect.GetMinX() );
        out.WriteAttribute( wxT( "iymin" ) , m_intrect.GetMinY() );
        out.WriteAttribute( wxT( "ixmax" ) , m_intrect.GetMaxX() );
        out.WriteAttribute( wxT( "iymax" ) , m_intrect.GetMaxY() );
        out.WriteNewLine();
    }
    else
    {
        out.WriteStartElement( wxT( "derived" ) );
        m_axisY->Save( this, out, towrite );
        out.WriteEndElement();
    }
}

void a2dCurvesArea::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dPolygonLClipper::DoLoad( parent, parser, xmlparts );
    //TODO: make serializing for m_colour, m_markerShow and m_markerShow2
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_showyaxis = parser.RequireAttributeValueBool( wxT( "showyaxis" ) );

        m_intrect.SetMin( parser.RequireAttributeValueDouble( wxT( "ixmin" ) ),
                          parser.RequireAttributeValueDouble( wxT( "iymin" ) ) );
        m_intrect.SetMax( parser.RequireAttributeValueDouble( wxT( "ixmax" ) ),
                          parser.RequireAttributeValueDouble( wxT( "iymax" ) ) );

// TODO call SetInternalBoundaries() with extbox from a2dCanvasXYDisplayGroupAreas
//      SetInternalBoundaries( m_ixmin, m_iymin, m_ixmax, m_iymax);
    }
    else
    {
        parser.Require( START_TAG, wxT( "derived" ) );
        parser.Next();

        m_axisY->Load( parent, parser );
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_axisY );

        parser.Require( END_TAG, wxT( "derived" ) );
        parser.Next();
    }
}
#endif

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( a2dCurvesAreaListBase );

a2dCurvesAreaList::a2dCurvesAreaList()
    : m_group( NULL )
{
    m_autoshowyaxes = false;
    a2dBoundingBox extbox( 0, 0, 1, 1 );
    a2dSmrtPtr<a2dCurvesArea> curvesarea = new a2dCurvesArea();
    curvesarea->SetInternalBoundaries( extbox, 0, 0, 1, 1 );
    Add( curvesarea );
    curvesarea->SetGroup( m_group );
    m_leftAxisY = curvesarea;
}

a2dCurvesAreaList::a2dCurvesAreaList( const a2dCurvesAreaList& other, CloneOptions options, a2dRefMap* refs )
    : m_group( NULL )
{
    m_autoshowyaxes = other.m_autoshowyaxes;
    for( size_t i = 0; i < other.GetCount(); i++ )
    {
        a2dSmrtPtr<a2dCurvesArea> area = ( a2dCurvesArea* )( other.Item( i ).Get()->Clone( a2dObject::CloneOptions( options & ~ a2dObject::clone_seteditcopy ) ) );
        Add( area );
        area->SetGroup( m_group );
        if( other[i].Get() == other.m_leftAxisY )
            m_leftAxisY = area;
        if( other[i].Get() == other.m_rightAxisY )
            m_rightAxisY = area;
    }
}

a2dCurvesAreaList::~a2dCurvesAreaList()
{
    Clear();
}

a2dObject* a2dCurvesAreaList::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    a2dCurvesAreaList* cloneArea = new a2dCurvesAreaList( *this, options, refs );
    return cloneArea;
}

a2dCurvesArea* a2dCurvesAreaList::GetCurvesArea( const wxString& curvesAreaName )
{
    a2dCurvesArea* emptyArea = NULL;
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        if( area->GetName().IsEmpty() )
            emptyArea = area;
        else if( area->GetName() == curvesAreaName )
            return area;
    }
    if( emptyArea != NULL && !curvesAreaName.IsEmpty() )
        emptyArea->SetName( curvesAreaName );
    return emptyArea;
}

a2dCurvesArea* a2dCurvesAreaList::GetCurvesArea( const wxString& curvesAreaName ) const
{
    a2dCurvesArea* emptyArea = NULL;
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        if( area->GetName() == curvesAreaName )
            return area;
    }
    return emptyArea;
}

void a2dCurvesAreaList::SetBoundaries( const a2dBoundingBox& extbox )
{
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        area->SetBoundaries( extbox );
    }
}

bool a2dCurvesAreaList::ProcessCanvasObjectEvent( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        area->ProcessCanvasObjectEvent( ic, hitEvent );
    }
    return hitEvent.m_processed;
}

void a2dCurvesAreaList::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dObject::DoWalker( parent, handler );

    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        if ( area )
            area->Walker( this, handler );

    }
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}

a2dCanvasObject* a2dCurvesAreaList::IsHitCurvesWorld ( a2dCurvesArea* &area, a2dIterC& ic, a2dHitEvent& hitEvent )
{
    for( size_t i = 0; i < GetCount(); i++ )
    {
        area = Item( i );
        a2dCanvasObject* curve = area->IsHitCurvesWorld ( ic, hitEvent );
        if( curve )
            return curve;
    }
    return NULL;
}

int a2dCurvesAreaList::AppendInternalBoundaries( a2dBboxHash* irectHash )
{
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        const a2dBoundingBox& aRect = area->GetInternalBoundaries();
        ( *irectHash )[area->GetName()] = aRect;
    }
    return GetCount();
}

int a2dCurvesAreaList::AppendCurvesBoundaries( a2dBboxHash* irectHash )
{
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        a2dBoundingBox abox = area->GetCurvesBoundaries();
        a2dBoundingBox aRect( abox );
        ( *irectHash )[area->GetName()] = aRect;
    }
    return GetCount();
}

a2dBoundingBox& a2dCurvesAreaList::Expand( a2dBoundingBox& bbox ) const
{
    /*
        for(size_t i=0; i < GetCount(); i++)
        {
            a2dCurvesArea* area = Item(i);
            area->Expand(bbox);
        }
    */
    if( m_leftAxisY )
        m_leftAxisY->Expand( bbox );
    if( m_rightAxisY )
        m_rightAxisY->Expand( bbox );
    return bbox;
}

void a2dCurvesAreaList::AddPending( a2dIterC& ic )
{
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        area->AddPending( ic );
    }
}

bool a2dCurvesAreaList::Update( a2dCanvasObject::UpdateMode mode )
{
    bool retCode = false;
    a2dCurvesArea* leftAxisY = NULL;
    a2dCurvesArea* rightAxisY = NULL;
    bool leftOK = m_leftAxisY != ( a2dCurvesArea* )NULL ? m_leftAxisY->IsCurvesHighlighted() : false;
    bool rightOK = m_rightAxisY != ( a2dCurvesArea* )NULL ? m_rightAxisY->IsCurvesHighlighted() : false;
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
//      area->SetGroup(m_group);
        if( m_autoshowyaxes )
            area->SetShowYaxis( false );
        //      area->GetAxisY()->SetInvertTic(false);
        retCode = area->Update( mode ) || retCode;
        if( m_autoshowyaxes )
        {
            if( !leftOK || !rightOK )
            {
                if( area->IsCurvesHighlighted() )
                {
                    if( !leftAxisY )
                        leftAxisY = area;
                    else if( !rightAxisY )
                        rightAxisY = area;
                }
            }
        }
    }

    if( m_autoshowyaxes )
    {
        if( !leftOK )
        {
            if( leftAxisY )
            {
                if( leftAxisY != m_rightAxisY )
                {
                    m_leftAxisY = leftAxisY;
                    leftOK = true;
                }
            }
            if( !leftOK && rightAxisY )
            {
                if( rightAxisY != m_rightAxisY )
                {
                    m_leftAxisY = rightAxisY;
                    leftOK = true;
                }
            }
        }
        if( !rightOK )
        {
            if( rightAxisY )
            {
                if( rightAxisY != m_leftAxisY )
                {
                    m_rightAxisY = rightAxisY;
                    rightOK = true;
                }
            }
            if( !rightOK && leftAxisY )
            {
                if( leftAxisY != m_leftAxisY )
                {
                    m_rightAxisY = leftAxisY;
                    rightOK = true;
                }
            }
        }
    }
    if( m_leftAxisY != ( a2dCurvesArea* )NULL )
    {
        if( m_autoshowyaxes )
        {
            m_leftAxisY->SetShowYaxis( true );
            m_leftAxisY->GetAxisY()->SetInvertTic( false );
            retCode = m_leftAxisY->Update( mode ) || retCode;
        }
    }
    if( m_rightAxisY != ( a2dCurvesArea* )NULL )
    {
        if( m_autoshowyaxes )
        {
            m_rightAxisY->SetShowYaxis( true );
            m_rightAxisY->GetAxisY()->SetInvertTic( true );
            retCode = m_rightAxisY->Update( mode ) || retCode;
        }
    }
    return retCode;
}

void a2dCurvesAreaList::SetAxes( const a2dBoundingBox& extbox, double x, double y, double& zerox, double& zeroy, double& ixmin, double& ixmax )
{
    // klion TODO remake
    double tzerox, tzeroy, tixmin, tixmax;
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        if( area != m_leftAxisY && area != m_rightAxisY )
            area->SetAxes( extbox, area->GetAxisY()->GetPosition(), y, tzerox, tzeroy, tixmin, tixmax );
    }

    if( m_leftAxisY != ( a2dCurvesArea* )NULL )
    {
        if( m_autoshowyaxes )
            m_leftAxisY->SetAxes( extbox, m_leftAxisY->m_intrect.GetMinX(), m_leftAxisY->m_intrect.GetMinY(), zerox, zeroy, ixmin, ixmax );
        else
            m_leftAxisY->SetAxes( extbox, m_leftAxisY->GetAxisY()->GetPosition(), y, zerox, zeroy, ixmin, ixmax );

    }
    if( m_rightAxisY != ( a2dCurvesArea* )NULL )
    {
        if( m_autoshowyaxes )
            m_rightAxisY->SetAxes( extbox, m_rightAxisY->m_intrect.GetMaxX(), m_rightAxisY->m_intrect.GetMinY(), tzerox, tzeroy, tixmin, tixmax );
        else
            m_rightAxisY->SetAxes( extbox, m_rightAxisY->GetAxisY()->GetPosition(), y, tzerox, tzeroy, tixmin, tixmax );
    }
}

void a2dCurvesAreaList::RenderAxesY ( a2dIterC& ic, OVERLAP clipparent )
{
    if( m_leftAxisY != ( a2dCurvesArea* )NULL )
    {
        if( m_leftAxisY->m_showyaxis )
            m_leftAxisY->m_axisY->Render( ic, clipparent );
    }
    if( m_rightAxisY != ( a2dCurvesArea* )NULL )
    {
        if( m_rightAxisY->m_showyaxis )
            m_rightAxisY->m_axisY->Render( ic, clipparent );
    }
}

void a2dCurvesAreaList::Render ( a2dIterC& ic, OVERLAP clipparent )
{
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
        ic.GetDrawer2D()->SetDrawerStroke( *a2dTRANSPARENT_STROKE );
        area->Render( ic, clipparent );
    }
}

void a2dCurvesAreaList::SetShowLeftYaxis( const wxString& curveAreaName, bool showyaxis )
{
    //disable the previous a2dCurveArea which was set as m_leftAxisY
    //And set the new one
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        if( area->GetName() == curveAreaName )
        {
            if( area != m_leftAxisY )
            {
                if( m_leftAxisY != ( a2dCurvesArea* ) NULL )
                    m_leftAxisY->SetShowYaxis( false );
                m_leftAxisY = area;
            }
            m_leftAxisY->SetShowYaxis( showyaxis );
            if( m_autoshowyaxes )
                m_leftAxisY->GetAxisY()->SetInvertTic( false );
            break;
        }
    }
}

void a2dCurvesAreaList::SetShowRightYaxis( const wxString& curveAreaName, bool showyaxis )
{
    //disable the previous a2dCurveArea which was set as m_rightAxisY
    //And set the new one
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        if( area->GetName() == curveAreaName )
        {
            if( area != m_rightAxisY )
            {
                if( m_rightAxisY != ( a2dCurvesArea* ) NULL )
                {
                    if( m_autoshowyaxes )
                        m_rightAxisY->GetAxisY()->SetInvertTic( false );
                    m_rightAxisY->SetShowYaxis( false );
                }
                m_rightAxisY = area;
            }
            m_rightAxisY->SetShowYaxis( showyaxis );
            if( m_autoshowyaxes )
                m_rightAxisY->GetAxisY()->SetInvertTic( true );
            break;
        }
    }
}

/*
double a2dBoundsRound(double val, double inc)
{
    double expon = log10(fabs(val));  ///10.));
    int toInt = int(expon+(expon<0. ? -0.5 : 0.5));
    return double((int(val*pow(10.,-toInt)+inc))*pow(10.,toInt));
}
*/

inline double a2dInternal_BoundsRound( double val, bool tics )
{
    double abs_val = fabs( val );
    if( abs_val < 1E-10 )
        return 0.;
    double expon = log10( abs_val ); ///10.));
    int toInt = int( expon < 0 ? expon - 1 : expon );
    double standardized = val * pow( 10., -toInt );
    if( tics )
    {
        int sign = val < 0. ? -1 : 1;
        double abs_standardized = fabs( standardized );
        if( abs_standardized <= 1.0001 )
            return pow( 10., toInt ) * sign;
        else if( abs_standardized <= 1.2501 )
            return 1.25 * pow( 10., toInt ) * sign;
        else if( abs_standardized <= 2.0001 )
            return 2.*pow( 10., toInt ) * sign;
        else if( abs_standardized <= 2.5001 )
            return 2.5 * pow( 10., toInt ) * sign;
        else if( abs_standardized <= 5.0001 )
            return 5.*pow( 10., toInt ) * sign;
        return 10.*pow( 10., toInt ) * sign;
    }
//    double expand = val < 0. ? -0.999999 : 0.999999;
    double expand = val < 0. ? -0.5 : 0.5;
    return double( ( int( standardized + expand ) ) * pow( 10., toInt ) );
}

double a2dBoundsRound( double val )
{
    return a2dInternal_BoundsRound( val, false );
}

double a2dTicsRound( double val )
{
    return a2dInternal_BoundsRound( val, true );
}

void a2dCurvesAreaList::SetTicY( int numLines )
{
    if( numLines < 2 )
        numLines = 2;
//    wxASSERT(numLines);
    double numLines2 = numLines / 2;
    int precision = 0;
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        a2dBoundingBox newRect = area->m_intrect;
        // klion TODO: for negative scale values
//      wxMax(fabs(curMaxY),fabs(curMinY);
//      ???????????
        double curHeight = newRect.GetHeight();
        double meanY = newRect.GetMinY() + curHeight / 2.;
        double newHeight = a2dTicsRound( curHeight / ( double )numLines ) * numLines;
        double aTic = newHeight / ( double )numLines;
        wxASSERT( aTic > 1E-12 );
        double newMeanY = int( ( meanY / aTic ) + ( meanY < 0. ? -0.5 : 0.5 ) ) * aTic;
        double newMinY = newMeanY - newHeight / 2.;
        if( newRect.GetMinY() == 0. && newMinY < 0. )
            newMinY = 0.;
//      double newMinY =  a2d Bounds Round((newRect.m_miny-(newHeight - curHeight)/2.)/aTic*2.)*aTic/2.;
        newRect.SetMinY( newMinY );
        double newMaxY = newMinY + newHeight;
        newRect.SetMaxY( newMaxY );
        area->SetInternalBoundaries( m_group->GetPlotAreaRect(), newRect );
        a2dCurveAxis* axisY = area->GetAxisY();
        axisY->SetTic( aTic );
        precision = m_group->SetAxisTicPrecision( axisY, newMinY, newMaxY );
    }
    if( precision )
        m_group->SetCommonTicPrecision( precision );
}

/*
a2dBoundingBox a2dCurvesAreaList::SetMarkersPos(const a2dBoundingBox& bbox)
{
    a2dBoundingBox tbox(bbox.GetMinX(), bbox.GetMaxY(), bbox.GetMinX(), bbox.GetMaxY());

    for(int i=GetCount()-1; i >= 0 ; i--)
    {
        a2dCurvesArea* area = Item(i);
        a2dMarkerShow* marker2 = wxDynamicCast( (a2dMarkerShow*)area->m_markerShow2, a2dMarkerShow );
        if(marker2)
        {
            marker2->SetPosXY(bbox.GetMinX(), tbox.GetMaxY());
//          marker2->SetLineWidth(bbox.GetWidth());
            marker2->Update( a2dCanvasObject::updatemask_normal );
            tbox.Expand(marker2->GetBbox());
        }
        a2dMarkerShow* marker = wxDynamicCast( (a2dMarkerShow*)area->m_markerShow, a2dMarkerShow );
        if(marker)
        {
            marker->SetPosXY(bbox.GetMinX(), tbox.GetMaxY());
//          marker->SetLineWidth(bbox.GetWidth());
            marker->Update( a2dCanvasObject::updatemask_normal );
            tbox.Expand(marker->GetBbox());
        }
    }
    return tbox;
}
*/

/*
void a2dCurvesAreaList::ChangeCursorPos(double shift)
{
    if(m_leftAxisY)
    {
        a2dMarker* marker = m_leftAxisY->GetCursorMarker();
        if(marker)
        {
            a2dBoundingBox ibox = m_leftAxisY->GetInternalBoundaries();
            double curX = marker->GetPosX() + shift;
            if(curX < ibox.GetMinX())
                curX = ibox.GetMinX();
            if(curX > ibox.GetMaxX())
                curX = ibox.GetMaxX();
            for(size_t i=0; i < GetCount() ; i++)
            {
                a2dCurvesArea* area = Item(i);
                a2dMarker* curMarker = area->GetCursorMarker();
                if(curMarker)
                {
                    a2dPoint2D point;
                    if(curMarker->GetCurve()->GetXyAtSweep(curX,point))
                        curMarker->SetPosXY(curX,point.m_y);
                }
            }
        }

    }
}
*/

void a2dCurvesAreaList::DependencyPending( a2dWalkerIOHandler* handler )
{
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
//      area->SetGroup(m_group);
        area->DependencyPending( handler );
    }
}

bool a2dCurvesAreaList::GetPending( void )
{
    bool retCode = false;
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        retCode = retCode || area->GetPending();
    }
    return retCode;
}

void a2dCurvesAreaList::UpdateViewDependentObjects( a2dIterC& ic )
{
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        area->UpdateViewDependentObjects( ic );
    }
}

void a2dCurvesAreaList::SetCursor( a2dCursor* cursor )
{
    cursor->RemoveMarkers();
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        a2dMarker* marker = area->GetCursorMarker();
        if( marker )
            cursor->AddMarker( marker );
    }
}

bool a2dCurvesAreaList::GetHasSelectedObjectsBelow()
{
    bool retCode = false;
    for( size_t i = 0; i < GetCount() && !retCode; i++ )
    {
        a2dCurvesArea* area = Item( i );
        retCode = retCode || area->GetHasSelectedObjectsBelow();
    }
    return retCode;
}

bool a2dCurvesAreaList::GetHasToolObjectsBelow()
{
    bool retCode = false;
    for( size_t i = 0; i < GetCount() && !retCode; i++ )
    {
        a2dCurvesArea* area = Item( i );
        retCode = retCode || area->GetHasToolObjectsBelow();
    }
    return retCode;
}

void a2dCurvesAreaList::SetClippingFromBox( a2dBoundingBox& bbox )
{
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        area->SetClippingFromBox( bbox );
    }
}

void a2dCurvesAreaList::SetGroup( a2dCanvasXYDisplayGroupAreas* aGroup )
{
    m_group = aGroup;
    for( size_t i = 0; i < GetCount(); i++ )
    {
        a2dCurvesArea* area = Item( i );
        area->SetGroup( aGroup );
    }
}

#if wxART2D_USE_CVGIO
void a2dCurvesAreaList::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite )
{
    a2dObject::DoSave( parent, out, xmlparts, towrite );
    //TODO: make serializing for m_leftAxisY
    //TODO: make serializing for m_rightAxisY
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        out.WriteStartElement( wxT( "derived" ) );

        for( size_t i = 0; i < GetCount(); i++ )
        {
            a2dCurvesArea* area = Item( i );
            area->Save( this, out, towrite );
        }

        out.WriteEndElement();
    }
}

void a2dCurvesAreaList::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dObject::DoLoad( parent, parser, xmlparts );
    //TODO: make serializing for m_leftAxisY
    //TODO: make serializing for m_rightAxisY
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        parser.Require( START_TAG, wxT( "derived" ) );
        parser.Next();

        // TODO: loading
        wxASSERT( 0 );

        parser.Require( END_TAG, wxT( "derived" ) );
        parser.Next();
    }
}
#endif

//----------------------------------------------------------------------------
// a2dCanvasXYDisplayGroupAreas
//----------------------------------------------------------------------------
const a2dPropertyIdRefObject a2dCanvasXYDisplayGroupAreas::PROPID_strokegrid( wxT( "strokegrid" ), a2dPropertyId::flag_none, 0 );
const a2dPropertyIdRefObject a2dCanvasXYDisplayGroupAreas::PROPID_fillAxisArea( wxT( "fillAxisArea" ), a2dPropertyId::flag_none, 0 );
//const a2dPropertyIdCanvasObject a2dCanvasXYDisplayGroupAreas::PROPID_axesarea(  wxT("axesarea"), a2dPropertyId::flag_none, 0 );

BEGIN_EVENT_TABLE( a2dCanvasXYDisplayGroupAreas, a2dCanvasObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dCanvasXYDisplayGroupAreas::OnCanvasObjectMouseEvent )
    EVT_CHAR( a2dCanvasXYDisplayGroupAreas::OnChar )
END_EVENT_TABLE()

void a2dCanvasXYDisplayGroupAreas::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dCanvasObject::DoWalker( parent, handler );

    if ( m_axesarealist )
        m_axesarealist->Walker( this, handler );

    if( m_axisX )
        m_axisX->Walker( this, handler );
//    if( m_axisY )
//        m_axisY->Walker( this, handler );
    if( m_cursor )
        m_cursor->Walker( this, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}

void a2dCanvasXYDisplayGroupAreas::DoAddPending( a2dIterC& ic )
{
    a2dCanvasObject::DoAddPending( ic );

    m_axisX->AddPending( ic );
    if( m_cursor )
        m_cursor->AddPending( ic );
    m_axesarealist->AddPending( ic );
}

bool a2dCanvasXYDisplayGroupAreas::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        PROPID_IncludeChildren->SetPropertyToObject( this, false );
        PROPID_Allowrotation->SetPropertyToObject( this, false );
        PROPID_Allowskew->SetPropertyToObject( this, false );

        return a2dCanvasObject::DoStartEdit( editmode, editstyle );
    }

    return false;
}

bool a2dCanvasXYDisplayGroupAreas::ProcessCanvasObjectEvent( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    if ( ic.GetDrawingPart()->GetEndCorridorObject() == this )
    {
        a2dCanvasObject::ProcessCanvasObjectEvent( ic, hitEvent );
        return hitEvent.m_processed;
    }

    hitEvent.m_isHit = false;

    // prepare relative to object coordinates for derived objects
    ic.GetInverseTransform().TransformPoint( hitEvent.m_x, hitEvent.m_y, hitEvent.m_relx, hitEvent.m_rely );
    if ( GetBbox().PointInBox( hitEvent.m_relx, hitEvent.m_rely, m_worldExtend + ic.GetHitMarginWorld() + ic.ExtendDeviceToWorld( m_pixelExtend ) ) )
    {
        a2dIterCU cu( ic, this );

        m_axesarealist->ProcessCanvasObjectEvent( ic, hitEvent );
        m_axisX->ProcessCanvasObjectEvent( ic, hitEvent );
    }

    a2dCanvasObject::ProcessCanvasObjectEvent( ic, hitEvent );

    return hitEvent.m_processed;
}


void a2dCanvasXYDisplayGroupAreas::OnChar( wxKeyEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_LEFT:
            {
                ChangeCursorPos( -1 );
            }
            break;
            case WXK_PAGEDOWN:
            {
                ChangeCursorPos( -10 );
            }
            break;
            case WXK_HOME:
            {
                ChangeCursorPos( -1024 );
            }
            break;
            case WXK_RIGHT:
            {
                ChangeCursorPos( 1 );
            }
            break;
            case WXK_PAGEUP:
            {
                ChangeCursorPos( 10 );
            }
            break;
            case WXK_END:
            {
                ChangeCursorPos( 1024 );
            }
            break;
            default:
                event.Skip();
                break;
        }
        /*
                switch(event.GetKeyCode())
                {

                    case 'Z':
                    case 'z':
                    {
                        a2dZoomTool* zoom = new a2dZoomTool(this);
                        PushTool(zoom);
                    }
                    break;
                    case WXK_SPACE:
                    {
                        StopTool();
                    }
                    break;
                    case WXK_ESCAPE:
                    {
                        StopTool();
                    }
                    break;

                    default:
                        event.Skip();
                }
        */
    }
    else
        event.Skip();
}

void a2dCanvasXYDisplayGroupAreas::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
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
            if ( event.GetMouseEvent().m_shiftDown )
            {
#if wxART2D_USE_EDITOR
                a2dCanvasXYDisplayGroupAreas* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dCanvasXYDisplayGroupAreas );
                a2dIterCU cu( *ic, original );
//klion             a2dIterCU cu2( *ic, original->m_axesarea );
                ic->SetCorridorPath( true );

                a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr );
                a2dDrawRectangleTool* drawrec = new a2dDrawRectangleTool( controller );
                controller->PushTool( drawrec );
                drawrec->SetEditAtEnd( true );
#else //wxART2D_USE_EDITOR
                wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR

                SetPending( true );
                ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
            }
            else if ( event.GetMouseEvent().m_controlDown )
            {
#if wxART2D_USE_EDITOR
                a2dCanvasXYDisplayGroupAreas* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dCanvasXYDisplayGroupAreas );
                a2dIterCU cu( *ic, original );
//klion             a2dIterCU cu2( *ic, original->m_axesarea );
                ic->SetCorridorPath( true );

                a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr );
                a2dDrawPolygonLTool* draw = new a2dDrawPolygonLTool( controller );
                controller->PushTool( draw );
                draw->SetEditAtEnd( true );
#else //wxART2D_USE_EDITOR
                wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR

                SetPending( true );
                ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
            }
            else
            {
                a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
                if ( IsHitWorld( *ic, hitevent ) )
                {
//                    a2dAffineMatrix areaworld = m_axesarea->GetTransformMatrix();

                    a2dCanvasXYDisplayGroupAreas* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dCanvasXYDisplayGroupAreas );

                    a2dHitEvent hitinfo( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
                    hitinfo.m_xyRelToChildren = true;
                    a2dCanvasObject* marker = original->IsHitWorld( *ic, hitinfo );
                    if ( marker  && marker->GetEditable() )
                    {
#if wxART2D_USE_EDITOR
                        a2dIterCU cu( *ic, original );
                        a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr );

                        ic->SetCorridorPath( true );
                        controller->StartEditingObject( marker, *ic );

#else //wxART2D_USE_EDITOR
                        wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR

                        SetPending( true );
                        ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
                        event.Skip(); //first event for new tool
                        return;
                    }

                    a2dIterCU cu( *ic, original );
//                    a2dCanvasObject* curve = original->m_axesarea->IsHitWorld( *ic, xw, yw, wxLAYER_ALL, a2dCANOBJHITOPTION_NOROOT );
                    a2dCurvesArea* curvesarea;
                    a2dHitEvent hitinfocurve( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
                    hitinfocurve.m_xyRelToChildren = true;
                    a2dCanvasObject* curve = original->m_axesarealist->IsHitCurvesWorld( curvesarea, *ic, hitinfocurve );
                    if ( curve && curve->GetEditable() )
                    {
#if wxART2D_USE_EDITOR
//                        a2dIterCU cu2( *ic, original->m_axesarea );
                        a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr );

                        ic->SetCorridorPath( true );
                        controller->StartEditingObject( curve, *ic );

#else //wxART2D_USE_EDITOR
                        wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR

                        SetPending( true );
                        ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
                        event.Skip(); //first event for new tool
                        return;
                    }

                    /*
                                        a2dHitEvent hitinfomarkerY( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
                                        hitinfomarkerY.m_xyRelToChildren = true;
                                        a2dCanvasObject* axisMarker = original->m_axisY->IsHitWorld( *ic, hitinfomarkerY );
                                        if ( axisMarker )
                                        {
                    #if wxART2D_USE_EDITOR
                                            a2dIterCU cu2( *ic, original->m_axisY );
                                            ic->SetCorridorPath( true, NULL );
                                            a2dStToolContr* controller = wxStaticCast( PROPID_controller->GetPropertyValue( this ).Get(), a2dStToolContr );

                                            ic->SetCorridorPath( true, NULL );
                                            controller->StartEditingObject( curve, *ic );

                    #else //wxART2D_USE_EDITOR
                        wxMessageBox( wxT("Need editor module enabled for this") );
                    #endif //wxART2D_USE_EDITOR

                                            SetPending( true );
                                            ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
                                            event.Skip(); //first event for new tool
                                            return;
                                        }
                    */

                    a2dHitEvent hitinfomarker( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
                    hitinfomarker.m_xyRelToChildren = true;
                    a2dCanvasObject* axisMarker = original->m_axisX->IsHitWorld( *ic, hitinfomarker );
                    if ( axisMarker && axisMarker->GetEditable() )
                    {
#if wxART2D_USE_EDITOR
                        a2dIterCU cu2( *ic, original->m_axisX );

                        a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr );

                        ic->SetCorridorPath( true );
                        controller->StartEditingObject( axisMarker, *ic );

#else //wxART2D_USE_EDITOR
                        wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR

                        SetPending( true );
                        ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
                        event.Skip(); //first event for new tool
                        return;
                    }

                    else
                        event.Skip();
                }
                else
                    EndEdit();
            }
        }
        else if ( event.GetMouseEvent().LeftDClick()  )
        {
            EndEdit();
        }
        else if ( event.GetMouseEvent().Moving() )
        {
            /*
                        a2dAffineMatrix cworld;
                        a2dHit how;
                        a2dAffineMatrix areaworld = m_axesarea->GetTransformMatrix();
                        a2dAffineMatrix tworld;
                        tworld *= m_lworld;
                        tworld *= areaworld;

                        a2dCanvasObject* curve = m_axesarea->WhichIsHitWorld( &tworld, xw, yw, NULL, how );
                        if ( curve )
                        {
                            event.Skip();
                        }
                        else
                        {
                            event.Skip();
                        }
            */
            event.Skip();
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

a2dCanvasXYDisplayGroupAreas::a2dCanvasXYDisplayGroupAreas( double x, double y )
    : a2dCanvasObject(), m_plotrect( 0, 0, 1, 1 )
{
    m_lworld.Translate( x, y );

    m_layer = wxLAYER_DEFAULT;
    m_cursorOnlyByCurves = true;

    m_showgrid = true;
    m_showxaxis = true;
    m_autoTicYPrecision = true;

    m_strokegrid = a2dStroke( wxColour( 10, 17, 25 ), 16, a2dSTROKE_LONG_DASH  );
    m_fillAxisArea = *a2dTRANSPARENT_FILL;
    m_strokeAxisArea = *a2dTRANSPARENT_STROKE;

    m_axesarealist = new a2dCurvesAreaList();
    m_axesarealist->SetGroup( this );

    m_axisX = new a2dCurveAxisLin();
}

a2dCanvasXYDisplayGroupAreas::~a2dCanvasXYDisplayGroupAreas()
{
//  m_axisY->ReleaseChildObjects();
    m_axisX->ReleaseChildObjects();
    if( m_cursor )
    {
        m_cursor->RemoveMarkers();
        m_cursor = NULL;
    }
    /*
        Dump();
        DumpOwners();
        m_axisX->Dump();
        m_axisX->DumpOwners();
        m_axisY->Dump();
        m_axisY->DumpOwners();
    */
}

a2dCanvasXYDisplayGroupAreas::a2dCanvasXYDisplayGroupAreas( const a2dCanvasXYDisplayGroupAreas& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_cursorOnlyByCurves = other.m_cursorOnlyByCurves;
    m_showgrid = other.m_showgrid;
    m_showxaxis = other.m_showxaxis;
    m_autoTicYPrecision = other.m_autoTicYPrecision;

    if( options & clone_members )
    {
        m_axisX = ( a2dCurveAxis* ) other.m_axisX->Clone( CloneOptions( options & ~ clone_seteditcopy ) );
        if( other.m_cursor )
            m_cursor = ( a2dCursor* ) other.m_cursor->Clone( CloneOptions( options & ~ clone_seteditcopy ) );
        m_strokegrid = other.m_strokegrid;
        m_fillAxisArea = other.m_fillAxisArea;
        m_strokeAxisArea = other.m_strokeAxisArea;
    }
    else
    {
        m_axisX = other.m_axisX;
        m_cursor = other.m_cursor;
        m_strokegrid = other.m_strokegrid;
        m_fillAxisArea = other.m_fillAxisArea;
        m_strokeAxisArea = other.m_strokeAxisArea;
    }

    m_plotrect = other.m_plotrect;

    if( options & clone_members )
        m_axesarealist = ( a2dCurvesAreaList* ) other.m_axesarealist->Clone( CloneOptions( options & ~ clone_seteditcopy ) );
    else
        m_axesarealist = other.m_axesarealist;
    m_axesarealist->SetGroup( this );
    if( m_axisX && GetRoot() )
        m_axisX->SetRoot( GetRoot() );

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in other.m_axesarea->Clone()
    CurrentSmartPointerOwner = this;
#endif
}

a2dObject* a2dCanvasXYDisplayGroupAreas::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCanvasXYDisplayGroupAreas( *this, options, refs );
}

a2dCurvesArea* a2dCanvasXYDisplayGroupAreas::AddCurvesArea( const wxString& curveAreaName )
{
    a2dSmrtPtr<a2dCurvesArea> curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    if( curvesarea == ( a2dCurvesArea* )NULL )
    {
        curvesarea = new a2dCurvesArea( curveAreaName );
        wxASSERT_MSG( curvesarea != ( a2dCurvesArea* )NULL, wxString::Format( wxT( "can't create curves area with name '%s' " ), curveAreaName.c_str() ) );
        //      curvesarea->SetCanvasDocument(GetCanvasDocument());
        curvesarea->SetBoundaries( m_plotrect );
        m_axesarealist->Add( curvesarea );
        curvesarea->SetGroup( this );

        if( m_axesarealist->m_leftAxisY == ( a2dCurvesArea* )NULL )
            m_axesarealist->m_leftAxisY = curvesarea;
        else if( m_axesarealist->m_rightAxisY == ( a2dCurvesArea* )NULL )
        {
            m_axesarealist->m_rightAxisY = curvesarea;
            if( m_axesarealist->GetAutoShowYAxes() )
                m_axesarealist->m_rightAxisY->GetAxisY()->SetInvertTic( true );
        }
        else
        {
            m_axesarealist->m_rightAxisY->SetShowYaxis( false );
            //          m_axesarealist->m_rightAxisY->GetAxisY()->SetInvertTic(false);
            m_axesarealist->m_rightAxisY = curvesarea;
            if( m_axesarealist->GetAutoShowYAxes() )
                m_axesarealist->m_rightAxisY->GetAxisY()->SetInvertTic( true );
        }
        SetPending( true );
    }
    return curvesarea;
}

void a2dCanvasXYDisplayGroupAreas::AddCurveToArea( const wxString& curveAreaName, a2dCurve* curve, const wxString curvename )
{
    a2dSmrtPtr<a2dCurvesArea> curvesarea = AddCurvesArea( curveAreaName );
    if( curvesarea )
    {
        curvesarea->AddCurve( curve, curvename );
        SetPending( true );
    }
}

void a2dCanvasXYDisplayGroupAreas::InsertCurveToArea( const wxString& curveAreaName, size_t before, a2dCurve* curve, const wxString curvename )
{
    a2dSmrtPtr<a2dCurvesArea> curvesarea = AddCurvesArea( curveAreaName );
    if( curvesarea )
    {
        curvesarea->InsertCurve( before, curve, curvename );
        SetPending( true );
    }
}

void a2dCanvasXYDisplayGroupAreas::AddMarkerToArea( const wxString& curveAreaName, a2dMarker* marker )
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    if( curvesarea )
    {
        curvesarea->AddMarker( marker );
        SetPending( true );
    }
    if( m_cursor )
        m_cursor->AddMarker( marker );
}

void a2dCanvasXYDisplayGroupAreas::RemoveMarkerFromArea( const wxString& curveAreaName, a2dMarker* marker )
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    if( curvesarea )
    {
        curvesarea->RemoveMarker( marker );
        SetPending( true );
    }
    if( m_cursor )
        m_cursor->RemoveMarker( marker );
}

void a2dCanvasXYDisplayGroupAreas::SetCursor( a2dCursor* cursor )
{
    a2dCursor* oldcursor = m_cursor;
    m_cursor = NULL;
    if( oldcursor )
        ReleaseChild( m_cursor );
    m_cursor = cursor;

    if( m_cursor )
        m_axesarealist->SetCursor( m_cursor );

    SetPending( true );
}

void a2dCanvasXYDisplayGroupAreas::ChangeCursorPos( double step )
{
    if( !m_cursor )
        return;
    a2dCurvesArea* baseArea = m_axesarealist->GetBaseCurvesArea();
    double newposx, tempposy;
    a2dAffineMatrix mat = baseArea->GetCurveAreaTransform();
    a2dBoundingBox ibox = baseArea->GetInternalBoundaries();
    mat.Invert();
    mat.TransformPoint( m_cursor->GetPosX(), ibox.GetMinY(), newposx, tempposy );

    double currentPosition = newposx;
    double smallStep = step / ( fabs( step ) );
    newposx += step;
    if( newposx < ibox.GetMinX() )
    {
        newposx = ibox.GetMinX();
        smallStep = 1;
    }
    if( newposx > ibox.GetMaxX() )
    {
        newposx = ibox.GetMaxX();
        smallStep = -1;
    }
    if( m_cursorOnlyByCurves )
    {
        bool currentPositionOK = m_cursor->CheckPosition( currentPosition );
        double newposition = newposx;
        while( !m_cursor->CheckPosition( newposx ) )
        {
            newposx += smallStep;
            if( newposx < ibox.GetMinX() || newposx > ibox.GetMaxX() )
            {
                newposx = newposition;
                while( !m_cursor->CheckPosition( newposx ) )
                {
                    newposx -= smallStep;
                    if( newposx < ibox.GetMinX() || newposx > ibox.GetMaxX() )
                    {
                        if( currentPositionOK )
                            return;
                        newposx = newposition;
                        m_cursor->SetPosition( newposx, tempposy, baseArea->GetCurveAreaTransform() );
                        return;
                    }
                }
            }
        }
    }
    m_cursor->SetPosition( newposx, tempposy, baseArea->GetCurveAreaTransform() );
}

a2dCurve* a2dCanvasXYDisplayGroupAreas::GetCurveFromArea( const wxString& curveAreaName, const wxString curvename )
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curve by name '%s' " ), curveAreaName.c_str() ) );
    if( curvesarea )
        return curvesarea->GetCurve( curvename );
    return NULL;
}

a2dCanvasObjectList* a2dCanvasXYDisplayGroupAreas::GetCurvesFromArea( const wxString& curveAreaName )
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    if( curvesarea )
        return curvesarea->GetCurves();
    return NULL;
}

a2dBoundingBox a2dCanvasXYDisplayGroupAreas::GetCurvesBoundariesFromArea( const wxString& curveAreaName )
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    if( curvesarea )
    {
        return curvesarea->GetCurvesBoundaries();
    }
    a2dBoundingBox ibbox;
    return ibbox;
}

a2dCurveAxis* a2dCanvasXYDisplayGroupAreas::GetAreaAxisY( const wxString& curveAreaName ) const
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    return curvesarea->GetAxisY();
}

void a2dCanvasXYDisplayGroupAreas::SetAreaAxisY( const wxString& curveAreaName, a2dCurveAxis* axisY )
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    curvesarea->SetAxisY( axisY );
    SetPending( true );
}

void a2dCanvasXYDisplayGroupAreas::SetShowLeftYaxis( const wxString& curveAreaName, bool showyaxis )
{
    m_axesarealist->SetShowLeftYaxis( curveAreaName, showyaxis );
    SetPending( true );
}

void a2dCanvasXYDisplayGroupAreas::SetShowRightYaxis( const wxString& curveAreaName, bool showyaxis )
{
    m_axesarealist->SetShowRightYaxis( curveAreaName, showyaxis );
    SetPending( true );
}

void a2dCanvasXYDisplayGroupAreas::SetGridStroke( const a2dStroke& stroke )
{
    m_strokegrid = stroke;
    SetPending( true );
}

void a2dCanvasXYDisplayGroupAreas::SetAxisAreaFill( const a2dFill& fill )
{
    m_fillAxisArea = fill;
    SetPending( true );
}

void a2dCanvasXYDisplayGroupAreas::SetAxisAreaStroke( const a2dStroke& stroke )
{
    m_strokeAxisArea = stroke;
    SetPending( true );
}

void a2dCanvasXYDisplayGroupAreas::SetBoundaries( const a2dBoundingBox& extbox )
{
    SetPending( true );
    m_plotrect = extbox;

    m_axesarealist->SetBoundaries( extbox );
}

void a2dCanvasXYDisplayGroupAreas::SetInternalBoundaries( double ximin, double yimin, double ximax, double yimax, const wxString& curveAreaName )
{
    a2dCurvesArea* curvesarea = AddCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curve area by name '%s' " ), curveAreaName.c_str() ) );
    if( curvesarea )
    {
        curvesarea->SetInternalBoundaries( m_plotrect, ximin, yimin, ximax, yimax );
    }
    SetPending( true );
}

void a2dCanvasXYDisplayGroupAreas::SetInternalXBoundaries( double ximin, double ximax )
{
    for( size_t i = 0; i < m_axesarealist->GetCount(); i++ )
    {
        a2dCurvesArea* area = m_axesarealist->Item( i );
        a2dBoundingBox newRect = area->GetInternalBoundaries();
        newRect.SetMinX( ximin );
        newRect.SetMaxX( ximax );
        area->SetInternalBoundaries( m_plotrect, newRect );
    }
}

a2dBoundingBox a2dCanvasXYDisplayGroupAreas::GetInternalBoundaries( const wxString& curveAreaName ) const
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    if( curvesarea )
    {
        return curvesarea->GetInternalBoundaries();
    }
    a2dBoundingBox ibbox;
    return ibbox;
}

int a2dCanvasXYDisplayGroupAreas::AppendInternalBoundaries( a2dBboxHash* irectHash )
{
    return m_axesarealist->AppendInternalBoundaries( irectHash );
}

int a2dCanvasXYDisplayGroupAreas::AppendCurvesBoundaries( a2dBboxHash* irectHash )
{
    return m_axesarealist->AppendCurvesBoundaries( irectHash );
}

a2dAffineMatrix a2dCanvasXYDisplayGroupAreas::GetCurvesAreaTransform( const wxString& curveAreaName )
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    if( curvesarea )
    {
        return curvesarea->GetCurveAreaTransform();
    }
    a2dAffineMatrix amatrix;
    return amatrix;
}

void a2dCanvasXYDisplayGroupAreas::DependencyPending( a2dWalkerIOHandler* handler )
{
    /*
        m_axesarealist->SetGroup(this);
        a2dCurvesArea* curvesarea = m_axesarealist->GetBaseCurvesArea();
        if(curvesarea)
        {
            a2dCanvasObjectList::iterator iter = m_axisX->GetChildObjectList()->begin();
            while( iter != m_axisX->GetChildObjectList()->end() )
            {
                a2dCurveObject* item = wxDynamicCast( (*iter).Get(), a2dCurveObject );
                if ( item )
                    item->SetCurvesArea( curvesarea );
                ++iter;
            }
        }
    */
    if ( m_flags.m_HighLight )
    {
        bool pendingchild = false;
        pendingchild = pendingchild || m_axesarealist->GetPending();
        pendingchild = pendingchild || m_axisX->GetPending();
        if( m_cursor )
            pendingchild = pendingchild || m_cursor->GetPending();

        if ( !m_flags.m_pending )
            SetPending( pendingchild );
    }
    a2dCanvasObject::DependencyPending( handler );
}

void a2dCanvasXYDisplayGroupAreas::DoUpdateViewDependentObjects( a2dIterC& ic )
{
    a2dCanvasObject::DoUpdateViewDependentObjects( ic );
    m_axesarealist->UpdateViewDependentObjects( ic );
    m_axisX->UpdateViewDependentObjects( ic );
    if( m_cursor )
        m_cursor->UpdateViewDependentObjects( ic );
}

a2dBoundingBox a2dCanvasXYDisplayGroupAreas::GetPlotAreaBbox() const
{
    return m_plotrect;
}

a2dBoundingBox a2dCanvasXYDisplayGroupAreas::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox = GetPlotAreaBbox();

    bbox.Expand( m_axisX->GetBbox() );
    m_axesarealist->Expand( bbox );
//  if(m_cursor)
//      bbox.Expand( m_cursor->GetBbox());

    return bbox;
}

bool a2dCanvasXYDisplayGroupAreas::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;

    calc = m_axesarealist->Update( mode );
    calc = m_axisX->Update( mode ) || calc;
    if( m_cursor )
        calc = m_cursor->Update( mode ) || calc;

    //current state of those flags are known after m_axesarea->Update() etc are called.
    m_flags.m_HasToolObjectsBelow = m_flags.m_HasToolObjectsBelow
                                    || m_axesarealist->GetHasToolObjectsBelow()
                                    || m_axisX->GetHasToolObjectsBelow();
    //|| m_axisY->GetHasToolObjectsBelow();

    m_flags.m_HasSelectedObjectsBelow = m_flags.m_HasSelectedObjectsBelow
                                        || m_axesarealist->GetHasSelectedObjectsBelow()
                                        || m_axisX->GetHasSelectedObjectsBelow();
    //|| m_axisY->GetHasSelectedObjectsBelow();

    if ( !m_bbox.GetValid() || calc )
    {
        if ( m_flags.m_isOnCorridorPath || m_flags.m_editingCopy )
        {
            a2dBoundingBox bbox = GetPlotAreaBbox();
            bbox.Enlarge( bbox.GetWidth() / 10 );
            m_axesarealist->SetClippingFromBox( bbox );
        }
        else
        {
            a2dBoundingBox bbox = GetPlotAreaBbox();
            m_axesarealist->SetClippingFromBox( bbox );
        }

        //where is the origin in plot coordinates
        double zerox, zeroy;
        double ixmin, ixmax;
        m_axesarealist->SetAxes( m_plotrect, 0, m_axisX->GetPosition(), zerox, zeroy, ixmin, ixmax );

        if ( zerox < m_plotrect.GetMinX() )
            zerox = m_plotrect.GetMinX();
        else if ( zerox > m_plotrect.GetMaxX() )
            zerox = m_plotrect.GetMaxX();

        if ( zeroy < m_plotrect.GetMinY() )
            zeroy = m_plotrect.GetMinY();
        else if ( zeroy > m_plotrect.GetMaxY() )
            zeroy = m_plotrect.GetMaxY();

        if( m_cursor )
        {
            a2dCurvesArea* baseArea = m_axesarealist->GetBaseCurvesArea();
            a2dBoundingBox ibox = baseArea->GetInternalBoundaries();
            a2dAffineMatrix mat = baseArea->GetCurveAreaTransform();
            m_cursor->SetHSize( m_plotrect.GetHeight() );
            m_cursor->SetWSize( m_plotrect.GetWidth() );
//klion200602           m_cursor->UpdatePosition(m_plotrect.GetMinX(),zeroy, mat);
            m_cursor->UpdatePosition( m_plotrect.GetMinX(), m_plotrect.GetMinY(), mat );
            m_cursor->Update( a2dCanvasObject::updatemask_normal );
        }

        m_axisX->SetPosXY( m_plotrect.GetMinX(), zeroy );
        m_axisX->SetLength( m_plotrect.GetWidth() );
        m_axisX->SetBoundaries( ixmin, ixmax );

        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );

        return true;
    }

    return false;
}

void a2dCanvasXYDisplayGroupAreas::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    //the text objects like all children are rendered after this
    a2dBoundingBox bbox = DoGetUnTransformedBbox();
    ic.GetDrawer2D()->DrawRoundedRectangle( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth(), bbox.GetHeight() , 0 );

    //rectangle for the background of the axisarea
    //The fill and outline set for this object will be used for it.
    ic.GetDrawer2D()->SetDrawerFill( m_fillAxisArea );
    ic.GetDrawer2D()->SetDrawerStroke( m_strokeAxisArea );
    ic.GetDrawer2D()->DrawRoundedRectangle( m_plotrect.GetMinX(), m_plotrect.GetMinY(), m_plotrect.GetWidth(), m_plotrect.GetHeight(), 0 );

    if ( m_showgrid )
        DrawGrid( ic );

    if( m_cursor )
        m_cursor->Render( ic, clipparent );

    if ( m_showxaxis )
        m_axisX->Render( ic, clipparent );
    m_axesarealist->RenderAxesY( ic, clipparent );
    //curves are rendered last
    m_axesarealist->Render( ic, clipparent );
}

void a2dCanvasXYDisplayGroupAreas::DrawGrid( a2dIterC& ic )
{
    double ticx = m_axisX->GetTic();
    double ticy = m_axesarealist->GetTicY();

    double x;
    double y;

    double ticstartx;
    double ticstarty;

    a2dSmrtPtr<a2dCurvesArea> baseArea = m_axesarealist->GetBaseCurvesArea();
    a2dBoundingBox intrect = baseArea->GetInternalBoundaries();

    ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
    ic.GetDrawer2D()->SetDrawerStroke( m_strokegrid );

    a2dAffineMatrix mat = m_axesarealist->GetBaseCurvesArea()->GetCurveAreaTransform();

    ticstartx = floor( intrect.GetMinX() / ticx + 0.5 ) * ticx;
    if ( ( ticx > 0. && ticstartx <= intrect.GetMinX() ) || ( ticx < 0. && ticstartx >= intrect.GetMinX() ) )
        ticstartx += ticx;
    ticstarty = floor( intrect.GetMinY() / ticy + 0.5 ) * ticy;
    if ( ( ticy > 0. && ticstarty <= intrect.GetMinY() ) || ( ticy < 0. && ticstarty >= intrect.GetMinY() ) )
        ticstarty += ticy;

    //draw gridlines
    for ( x = ticstartx; ( ticx > 0. && x < intrect.GetMaxX() ) || ( ticx < 0. && x > intrect.GetMaxX() ) ; x = x + ticx )
    {
        double xw, yw;
        //where is the x in world coordinates
        mat.TransformPoint( x, 0, xw, yw );
        ic.GetDrawer2D()->DrawLine( xw, m_plotrect.GetMinY(), xw, m_plotrect.GetMaxY() );
    }

    for ( y = ticstarty; ( ticy > 0. && y < intrect.GetMaxY() ) || ( ticy < 0. && y > intrect.GetMaxY() ) ; y = y + ticy )
    {
        double xw, yw;
        //where is the y in world coordinates
        mat.TransformPoint( 0, y, xw, yw );
        ic.GetDrawer2D()->DrawLine( m_plotrect.GetMinX(), yw, m_plotrect.GetMaxX(), yw );
    }
}

bool a2dCanvasXYDisplayGroupAreas::DoIsHitWorld( a2dIterC& WXUNUSED( ic ), a2dHitEvent& hitEvent )
{
    hitEvent.m_how = a2dHit::stock_fill;
    return true;
}

wxString a2dCanvasXYDisplayGroupAreas::GetLeftAxisText() const
{
    return m_axesarealist->m_leftAxisY != ( a2dCurvesArea* )NULL
           ? m_axesarealist->m_leftAxisY->GetAxisText() : wxT( "" );
}

wxString a2dCanvasXYDisplayGroupAreas::GetRightAxisText() const
{
    return m_axesarealist->m_rightAxisY != ( a2dCurvesArea* )NULL
           ? m_axesarealist->m_rightAxisY->GetAxisText() : wxT( "" );
}

void a2dCanvasXYDisplayGroupAreas::SetAxisText( const wxString& curveAreaName, const wxString& text, const wxColour& color )
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curve by name '%s' " ), curveAreaName.c_str() ) );
    curvesarea->SetAxisText( text );
    curvesarea->SetColor( color );
}

void a2dCanvasXYDisplayGroupAreas::ClearCurvesAreas()
{
    m_axesarealist->m_leftAxisY = ( a2dCurvesArea* ) NULL;
    m_axesarealist->m_rightAxisY = ( a2dCurvesArea* ) NULL;
    if( m_cursor )
        m_cursor->RemoveMarkers();
    m_axesarealist->Empty();
}

void a2dCanvasXYDisplayGroupAreas::SetAreaMarkerShow( const wxString& curveAreaName, a2dMarkerShow* showm )
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    curvesarea->SetMarkerShow( showm );
    SetPending( true );
}

void a2dCanvasXYDisplayGroupAreas::SetAreaMarkerShow2( const wxString& curveAreaName, a2dMarkerShow* showm )
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    curvesarea->SetMarkerShow2( showm );
    SetPending( true );
}

a2dMarkerShow* a2dCanvasXYDisplayGroupAreas::GetAreaMarkerShow( const wxString& curveAreaName ) const
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    return curvesarea->GetMarkerShow();
}

a2dMarkerShow* a2dCanvasXYDisplayGroupAreas::GetAreaMarkerShow2( const wxString& curveAreaName ) const
{
    a2dCurvesArea* curvesarea = m_axesarealist->GetCurvesArea( curveAreaName );
    wxASSERT_MSG( curvesarea != NULL, wxString::Format( wxT( "not found curves area by name '%s' " ), curveAreaName.c_str() ) );
    return curvesarea->GetMarkerShow2();
}

void a2dCanvasXYDisplayGroupAreas::SetTicY( int numLines )
{
    m_axesarealist->SetTicY( numLines );
}

int a2dCanvasXYDisplayGroupAreas::SetAxisTicPrecision( a2dCurveAxis* axisY, double newMinY, double newMaxY )
{
    if( m_autoTicYPrecision )
    {
        double aTic = axisY->GetTic();
        double abs_tic = fabs( aTic );
        int precision = 1;
        double maxVal = wxMax( fabs( newMinY ), fabs( newMaxY ) );
        if( maxVal > 999999. )
        {
            axisY->SetTicFormat( wxT( "%.6e" ) );
            precision = 5;
        }
        else if( abs_tic <= 1E-6 )
        {
            axisY->SetTicFormat( wxT( "%.6e" ) );
            if( precision != 5 )
                precision = 4;
        }
        else if( abs_tic <= 1E-4 )
        {
            axisY->SetTicFormat( wxT( "%.4e" ) );
            if( precision != 4 && precision != 5 )
                precision = 3;
        }
        else if( abs_tic <= 0.026 )
        {
            axisY->SetTicFormat( wxT( "%.3f" ) );
            if( precision != 1 )
                precision = 2;
        }
        else if( abs_tic >= 1. )
        {
            if( int( abs_tic ) * 10 == int( abs_tic * 10. ) )
                axisY->SetTicFormat( wxT( "%.0f" ) );
            else if( int( abs_tic * 10 ) * 10 == int( abs_tic * 100. ) )
                axisY->SetTicFormat( wxT( "%.1f" ) );
            else
                axisY->SetTicFormat( wxT( "%.2f" ) );
        }
        else
            axisY->SetTicFormat( wxT( "%.2f" ) );
        return precision;
    }
    return 0;
}

void a2dCanvasXYDisplayGroupAreas::SetCommonTicPrecision( int precision )
{
    if( m_autoTicYPrecision )
    {
        wxString commonTicFormat;
        switch( precision )
        {
            default:
            case 1: commonTicFormat = wxT( "%f2.2" ); break;
            case 2: commonTicFormat = wxT( "%f2.3" ); break;
            case 3: commonTicFormat = wxT( "%f4.2" ); break;
            case 4: case 5: commonTicFormat = wxT( "%g" ); break;
        }
        for( size_t i = 0; i < m_axesarealist->GetCount(); i++ )
        {
            a2dCurvesArea* area = m_axesarealist->Item( i );
            area->GetAxisY()->SetCommonTicFormat( commonTicFormat );
        }
    }
}

#if wxART2D_USE_CVGIO
void a2dCanvasXYDisplayGroupAreas::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "cursorOnlyByCurves" ) , m_cursorOnlyByCurves );
        out.WriteAttribute( wxT( "showgrid" ) , m_showgrid );
        out.WriteAttribute( wxT( "showxaxis" ) , m_showxaxis );
        out.WriteAttribute( wxT( "autoTicYPrecision" ) , m_autoTicYPrecision );
        out.WriteNewLine();

        out.WriteAttribute( _T( "xmin" ) , m_plotrect.GetMinX() );
        out.WriteAttribute( wxT( "ymin" ) , m_plotrect.GetMinY() );
        out.WriteAttribute( _T( "xmax" ) , m_plotrect.GetMaxX() );
        out.WriteAttribute( wxT( "ymax" ) , m_plotrect.GetMaxY() );
        out.WriteNewLine();
    }
    else
    {
        out.WriteStartElement( wxT( "derived" ) );

        m_strokegrid.Save( this, out, towrite );
        m_fillAxisArea.Save( this, out, towrite );
        m_strokeAxisArea.Save( this, out, towrite );

        m_axesarealist->Save( this, out, towrite );

        m_axisX->Save( this, out, towrite );
//        m_axisY->Save( this, out, towrite);

        if( m_cursor )
            m_cursor->Save( this, out, towrite );

        out.WriteEndElement();
    }
}

void a2dCanvasXYDisplayGroupAreas::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_cursorOnlyByCurves = parser.RequireAttributeValueBool( wxT( "cursorOnlyByCurves" ) );
        m_showgrid = parser.RequireAttributeValueBool( wxT( "showgrid" ) );
        m_showxaxis = parser.RequireAttributeValueBool( wxT( "showxaxis" ) );
        m_autoTicYPrecision = parser.RequireAttributeValueBool( wxT( "autoTicYPrecision" ) );

        m_plotrect.SetMin( parser.RequireAttributeValueDouble( wxT( "xmin" ) ),
                           parser.RequireAttributeValueDouble( wxT( "ymin" ) ) );
        m_plotrect.SetMax( parser.RequireAttributeValueDouble( wxT( "xmax" ) ),
                           parser.RequireAttributeValueDouble( wxT( "ymax" ) ) );
    }
    else
    {
        parser.Require( START_TAG, wxT( "derived" ) );
        parser.Next();

        // new strokes, since the current ones might be shared by other objects too.
        m_strokegrid = m_strokegrid;
        m_fillAxisArea = m_fillAxisArea;
        m_strokeAxisArea = m_strokeAxisArea;

        m_strokegrid.Load( parent, parser );
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_strokegrid );
        m_fillAxisArea.Load( parent, parser );
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_fillAxisArea );
        m_strokeAxisArea.Load( parent, parser );
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_strokeAxisArea );

        m_axesarealist->Load( parent, parser );
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_axesarealist );

        m_axisX->Load( parent, parser );
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_axisX );

        if ( parser.HasAttribute( wxT( "cursor" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_cursor, parser.GetAttributeValue( wxT( "cursor" ) ) );
        }

        parser.Require( END_TAG, wxT( "derived" ) );
        parser.Next();
    }
}

a2dCanvasXYDisplayGroup::a2dCanvasXYDisplayGroup( double x, double y )
    : a2dCanvasXYDisplayGroupAreas( x, y )
{
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in other.m_axesarea->Clone()
    CurrentSmartPointerOwner = this;
#endif
}

a2dCanvasXYDisplayGroup::~a2dCanvasXYDisplayGroup()
{
}

a2dCanvasXYDisplayGroup::a2dCanvasXYDisplayGroup( const a2dCanvasXYDisplayGroup& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasXYDisplayGroupAreas( other, options, refs )
{

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in other.m_axesarea->Clone()
    CurrentSmartPointerOwner = this;
#endif
}

a2dObject* a2dCanvasXYDisplayGroup::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCanvasXYDisplayGroup( *this, options, refs );
}


#endif //wxART2D_USE_CVGIO

