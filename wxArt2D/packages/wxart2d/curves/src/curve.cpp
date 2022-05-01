/*! \file curves/src/curve.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: curve.cpp,v 1.42 2009/10/01 19:22:35 titato Exp $
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

IMPLEMENT_CLASS( a2dCurveObject, a2dCanvasObject )
IMPLEMENT_CLASS( a2dCurve, a2dCurveObject )
IMPLEMENT_DYNAMIC_CLASS( a2dFunctionCurve, a2dCurve )
IMPLEMENT_DYNAMIC_CLASS( a2dVertexCurve, a2dCurve )
IMPLEMENT_DYNAMIC_CLASS( a2dSweepCurve, a2dCurve )
IMPLEMENT_DYNAMIC_CLASS( a2dBandCurve, a2dCurve )
IMPLEMENT_DYNAMIC_CLASS( a2dBarCurve, a2dVertexCurve )
IMPLEMENT_DYNAMIC_CLASS( a2dVertexListCurve, a2dCurve )
IMPLEMENT_DYNAMIC_CLASS( a2dPieCurve, a2dCurve )


#ifdef wxUSE_INTPOINT
#define wxMAX_COORDINATE INT_MAX
#define wxMIN_COORDINATE INT_MIN
#else
#define wxMAX_COORDINATE 100e99
#define wxMIN_COORDINATE 100e-99
#endif


//! used to move points in VertexCurve objects
/*!
*/
class a2dCommand_VertexCurveMovePoint: public a2dCommand
{

public:

    static const a2dCommandId COMID_VertexCurveMovePoint;

    a2dCommand_VertexCurveMovePoint( a2dVertexCurve* curve, double x, double y, int index  );
    ~a2dCommand_VertexCurveMovePoint( void );

    bool Do( void );
    bool Undo( void );

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

protected:

    // x of point
    double m_x;

    // y of point
    double m_y;

    // index of point
    int m_index;

    a2dSmrtPtr<a2dVertexCurve> m_curve;
};

const a2dCommandId a2dCommand_VertexCurveMovePoint::COMID_VertexCurveMovePoint( wxT( "VertexCurveMovePoint" ) );


/*
*   a2dCommand_VertexCurveMovePoint
*/

a2dCommand_VertexCurveMovePoint::a2dCommand_VertexCurveMovePoint( a2dVertexCurve* curve, double x, double y, int index ):
    a2dCommand( true, COMID_VertexCurveMovePoint )
{
    m_x = x;
    m_y = y;
    m_index = index;

    m_curve = wxStaticCast( curve, a2dVertexCurve );
}

a2dCommand_VertexCurveMovePoint::~a2dCommand_VertexCurveMovePoint( void )
{
}

bool a2dCommand_VertexCurveMovePoint::Do( void )
{
    double x, y;
    m_curve->GetPosXYPoint( m_index, x, y );
    m_curve->SetPosXYPoint( m_index, m_x, m_y );
    m_x = x;
    m_y = y;

    m_curve->GetRoot()->Modify( true );
    m_curve->GetRoot()->UpdateAllViews( a2dCANVIEW_UPDATE_OLDNEW );

    return true;
}

bool a2dCommand_VertexCurveMovePoint::Undo( void )
{
    m_curve->SetPosXYPoint( m_index, m_x, m_y );

    m_curve->GetRoot()->Modify( true );
    m_curve->GetRoot()->UpdateAllViews( a2dCANVIEW_UPDATE_OLDNEW );

    a2dComEvent changed( this, m_curve );
    ProcessEvent( changed );

    return true;
}

//! used to move points in VertexCurve objects
/*!
*/
class a2dCommand_SweepCurveMovePoint: public a2dCommand
{

public:

    static const a2dCommandId COMID_SweepCurveMovePoint;

    a2dCommand_SweepCurveMovePoint( a2dSweepCurve* curve, double x, double y, int index );

    ~a2dCommand_SweepCurveMovePoint( void );

    bool Do( void );
    bool Undo( void );

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

protected:

    double m_sweep;

    // x of point
    double m_x;

    // y of point
    double m_y;

    // index of point
    int m_index;

    a2dSmrtPtr<a2dSweepCurve> m_curve;
};

const a2dCommandId a2dCommand_SweepCurveMovePoint::COMID_SweepCurveMovePoint( wxT( "SweepCurveMovePoint" ) );

/*
*   a2dCommand_SweepCurveMovePoint
*/

a2dCommand_SweepCurveMovePoint::a2dCommand_SweepCurveMovePoint( a2dSweepCurve* curve, double x, double y, int index ):
    a2dCommand( true, COMID_SweepCurveMovePoint )
{
    m_x = x;
    m_y = y;
    m_index = index;

    m_curve = wxStaticCast( curve, a2dSweepCurve );
}

a2dCommand_SweepCurveMovePoint::~a2dCommand_SweepCurveMovePoint( void )
{
}

bool a2dCommand_SweepCurveMovePoint::Do( void )
{
    double x, y;
    m_curve->GetPosXYPoint( m_index, x, y );
    m_curve->SetPosXYPoint( m_index, m_x, m_y );
    m_x = x;
    m_y = y;

    m_curve->GetRoot()->Modify( true );
    m_curve->GetRoot()->UpdateAllViews( a2dCANVIEW_UPDATE_OLDNEW );

    return true;
}

bool a2dCommand_SweepCurveMovePoint::Undo( void )
{
    m_curve->SetPosXYPoint( m_index, m_x, m_y );

    m_curve->GetRoot()->Modify( true );
    m_curve->GetRoot()->UpdateAllViews( a2dCANVIEW_UPDATE_OLDNEW );

    a2dComEvent changed( this, m_curve );
    ProcessEvent( changed );

    return true;
}

//! used to move points in BandCurve objects
/*!
*/
class a2dCommand_BandCurveMovePoint: public a2dCommand
{

public:

    static const a2dCommandId COMID_BandCurveMovePoint;

    a2dCommand_BandCurveMovePoint( a2dBandCurve* curve, double sweep, double y, bool up, int index );

    ~a2dCommand_BandCurveMovePoint( void );

    bool Do( void );
    bool Undo( void );

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

protected:

    bool m_up;

    double m_sweep;

    // y of point
    double m_y;

    // index of point
    int m_index;

    a2dSmrtPtr<a2dBandCurve> m_curve;
};

const a2dCommandId a2dCommand_BandCurveMovePoint::COMID_BandCurveMovePoint( wxT( "BandCurveMovePoint" ) );

/*
*   a2dCommand_BandCurveMovePoint
*/
a2dCommand_BandCurveMovePoint::a2dCommand_BandCurveMovePoint( a2dBandCurve* curve, double sweep, double y, bool up, int index ):
    a2dCommand( true, COMID_BandCurveMovePoint )
{
    m_sweep = sweep;
    m_up = up;
    m_y = y;

    m_index = index;

    m_curve = wxStaticCast( curve, a2dBandCurve );
}

a2dCommand_BandCurveMovePoint::~a2dCommand_BandCurveMovePoint( void )
{
}

bool a2dCommand_BandCurveMovePoint::Do( void )
{
    double sweep, y;

    m_curve->GetPosXYPoint( m_index, sweep, y, m_up );
    m_curve->SetPosXYPoint( m_index, m_sweep, m_y, m_up );
    m_sweep = sweep;
    m_y = y;

    m_curve->GetRoot()->Modify( true );
    m_curve->GetRoot()->UpdateAllViews( a2dCANVIEW_UPDATE_OLDNEW );

    return true;
}

bool a2dCommand_BandCurveMovePoint::Undo( void )
{
    m_curve->SetPosXYPoint( m_index, m_sweep, m_y, m_up );

    m_curve->GetRoot()->Modify( true );
    m_curve->GetRoot()->UpdateAllViews( a2dCANVIEW_UPDATE_OLDNEW );

    a2dComEvent changed( this, m_curve );
    ProcessEvent( changed );

    return true;
}


/*
*   a2dCommand_VertexListCurveMovePoint
*/


//! used to move points in VertexCurve objects
/*!
*/
class a2dCommand_VertexListCurveMovePoint: public a2dCommand
{

public:
    a2dCommand_VertexListCurveMovePoint( a2dVertexListCurve* curve, double x, double y, int index  );

    ~a2dCommand_VertexListCurveMovePoint( void );

    bool Do( void );
    bool Undo( void );

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

    static const a2dCommandId COMID_VertexListCurveMovePoint;

protected:

    // x of point
    double m_x;

    // y of point
    double m_y;

    // index of point
    int m_index;

    a2dSmrtPtr<a2dVertexListCurve> m_curve;
};

const a2dCommandId a2dCommand_VertexListCurveMovePoint::COMID_VertexListCurveMovePoint( wxT( "VertexListCurveMovePoint" ) );

a2dCommand_VertexListCurveMovePoint::a2dCommand_VertexListCurveMovePoint( a2dVertexListCurve* curve, double x, double y, int index ):
    a2dCommand( true, COMID_VertexListCurveMovePoint )
{
    m_x = x;
    m_y = y;
    m_index = index;

    m_curve = wxStaticCast( curve, a2dVertexListCurve ) ;
}

a2dCommand_VertexListCurveMovePoint::~a2dCommand_VertexListCurveMovePoint( void )
{
}

bool a2dCommand_VertexListCurveMovePoint::Do( void )
{
    double x, y;
    m_curve->GetPosXYPoint( m_index, x, y );
    m_curve->SetPosXYPoint( m_index, m_x, m_y );
    m_x = x;
    m_y = y;

    m_curve->GetRoot()->Modify( true );
    m_curve->GetRoot()->UpdateAllViews( a2dCANVIEW_UPDATE_OLDNEW );

    return true;
}

bool a2dCommand_VertexListCurveMovePoint::Undo( void )
{
    m_curve->SetPosXYPoint( m_index, m_x, m_y );

    m_curve->GetRoot()->Modify( true );
    m_curve->GetRoot()->UpdateAllViews( a2dCANVIEW_UPDATE_OLDNEW );

    a2dComEvent changed( this, m_curve );
    ProcessEvent( changed );

    return true;
}



//----------------------------------------------------------------------------
// a2dCurveObject
//----------------------------------------------------------------------------

a2dCurveObject::a2dCurveObject()
{
    m_curvesArea = NULL;
}

a2dCurveObject::~a2dCurveObject()
{
}

//!copy constructor
a2dCurveObject::a2dCurveObject( const a2dCurveObject& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_curvesArea = other.m_curvesArea;
    m_untransbbox = other.m_untransbbox;
}


void a2dCurveObject::World2Curve( double xw, double yw, double& xcurve, double& ycurve ) const
{
    //wxASSERT_MSG( m_curvesArea != NULL, wxT("m_curvesArea not set") );

    if ( m_curvesArea )
        m_curvesArea->World2Curve( xw, yw, xcurve, ycurve );
    else
    {
        a2dAffineMatrix mat = m_lworld;
        mat.Invert();
        mat.TransformPoint( xw, yw, xcurve, ycurve );
    }
}

void a2dCurveObject::Curve2World( double xcurve, double ycurve, double& xw, double& yw ) const
{
    //wxASSERT_MSG( m_curvesArea != NULL, wxT("m_curvesArea not set") );

    if ( m_curvesArea )
        m_curvesArea->Curve2World( xcurve, ycurve, xw, yw );
    else
    {
        m_lworld.TransformPoint( xcurve, ycurve, xw, yw );
    }
}

bool a2dCurveObject::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        m_untransbbox = m_bbox = DoGetUnTransformedBbox();
        if ( m_bbox.GetValid() )
        {
            m_bbox.MapBbox( m_lworld );
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------
// a2dCurve
//----------------------------------------------------------------------------

a2dCurve::a2dCurve(): a2dCurveObject()
{
    m_highlightmode = 0;
    m_logFunction = wxNO_OP;
    m_clipdata.SetValid( false );
    m_curveBox.SetValid( false );
}

a2dCurve::~a2dCurve()
{
}

a2dCurve::a2dCurve( const a2dCurve& other, CloneOptions options, a2dRefMap* refs )
    : a2dCurveObject( other, options, refs )
{
    m_clipdata = other.m_clipdata;
    m_highlightmode = other.m_highlightmode;
    m_pointsFill = other.m_pointsFill;
    m_logFunction = other.m_logFunction;
    m_curveBox = other.m_curveBox;
}

void a2dCurve::SetBoundaries( double xmin, double ymin, double xmax, double ymax )
{
    SetPending( true );
    m_clipdata.SetValid( false );
    m_clipdata.Expand( xmin, ymin );
    m_clipdata.Expand( xmax, ymax );
}

#if wxART2D_USE_CVGIO
void a2dCurve::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_clipdata.GetValid() )
        {
            out.WriteAttribute( wxT( "xmin" ) , m_clipdata.GetMinX() );
            out.WriteAttribute( wxT( "ymin" ) , m_clipdata.GetMinY() );
            out.WriteAttribute( wxT( "xmax" ) , m_clipdata.GetMaxX() );
            out.WriteAttribute( wxT( "ymax" ) , m_clipdata.GetMaxY() );
        }
    }
    else
    {
    }
}

void a2dCurve::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( parser.HasAttribute( wxT( "xmin" ) ) )
        {
            m_clipdata.SetValid( false );
            m_clipdata.Expand( parser.GetAttributeValueDouble( wxT( "xmin" ) ), parser.RequireAttributeValueDouble( wxT( "ymin" ) ) );
            m_clipdata.Expand( parser.GetAttributeValueDouble( wxT( "xmax" ) ), parser.RequireAttributeValueDouble( wxT( "ymax" ) ) );
        }
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO


a2dBoundingBox a2dCurve::GetClippedCurveBoundaries()
{
    a2dBoundingBox curvebox = GetCurveBoundaries();
    if ( m_clipdata.GetValid() )
    {
        if( !curvebox.And( &m_clipdata ) )
        {
            curvebox.SetValid( false );
            return curvebox;
        }
    }

    a2dBoundingBox areabox = m_curvesArea->GetInternalBoundaries();
    if( curvebox.GetValid() && !curvebox.And( &areabox ) )
        curvebox.SetValid( false );
    return curvebox;
}

a2dBoundingBox a2dCurve::GetCurveBoundaries()
{
    return m_curveBox;
    /*
        a2dBoundingBox curvebox;
        double xw1, yw1, xw2, yw2;
        World2Curve( m_bbox.GetMinX(), m_bbox.GetMinY(), xw1, yw1 );
        World2Curve( m_bbox.GetMaxX(), m_bbox.GetMaxY(), xw2, yw2 );
        curvebox.Expand(xw1,yw1);
        curvebox.Expand(xw1,yw2);
        curvebox.Expand(xw2,yw1);
        curvebox.Expand(xw2,yw2);
        return curvebox;
    */
}

/*
double a2dCurve::GetBoundaryMinX()
{
    double xw, yw;
    World2Curve( m_bbox.GetMinX(), m_bbox.GetMinY(), xw, yw );
    if ( m_clipdata.GetValid() )
    {
        double xmin = m_clipdata.GetMinX();
        xw = wxMax( xmin, xw );
    }
    return wxMax( m_curvesArea->GetBoundaryMinX(), xw );
}

double a2dCurve::GetBoundaryMaxX()
{
    double xw, yw;
    World2Curve( m_bbox.GetMaxX(), m_bbox.GetMaxY(), xw, yw );
    if ( m_clipdata.GetValid() )
    {
        double xmax = m_clipdata.GetMaxX();
        xw = wxMin( xmax, xw );
    }
    return wxMin( m_curvesArea->GetBoundaryMaxX(), xw );
}

double a2dCurve::GetBoundaryMinY()
{
    double xw, yw;
    World2Curve( m_bbox.GetMinX(), m_bbox.GetMinY(), xw, yw );
    if ( m_clipdata.GetValid() )
    {
        double ymin = m_clipdata.GetMinY();
        yw = wxMax( ymin, yw );
    }
    return wxMax( m_curvesArea->GetBoundaryMinY(), yw );
}

double a2dCurve::GetBoundaryMaxY()
{
    double xw, yw;
    World2Curve( m_bbox.GetMaxX(), m_bbox.GetMaxY(), xw, yw );
    if ( m_clipdata.GetValid() )
    {
        double ymax = m_clipdata.GetMaxY();
        yw = wxMin( ymax, yw );
    }
    return wxMin( m_curvesArea->GetBoundaryMaxY(), yw );
}
*/

//----------------------------------------------------------------------------
// a2dFunctionCurve
//----------------------------------------------------------------------------

a2dFunctionCurve::a2dFunctionCurve( const wxString& curve ): a2dCurve()
{
    m_curve = curve;
#if wxART2D_USE_CANEXTOBJ
    m_toeval.SetEvalString( m_curve );
#endif //wxART2D_USE_CANEXTOBJ

    m_xstart = wxMIN_COORDINATE;
    m_xstop = wxMAX_COORDINATE;
    m_xstep = ( m_xstop - m_xstart ) / 100.0;
}

a2dFunctionCurve::~a2dFunctionCurve()
{
}

a2dFunctionCurve::a2dFunctionCurve( const a2dFunctionCurve& other, CloneOptions options, a2dRefMap* refs )
    : a2dCurve( other, options, refs )
{
    m_curve = other.m_curve;
#if wxART2D_USE_CANEXTOBJ
    m_toeval.SetEvalString( m_curve );
#endif //wxART2D_USE_CANEXTOBJ
    m_xstep = other.m_xstep;
    m_xstart = other.m_xstart;
    m_xstop = other.m_xstop;
}

a2dObject* a2dFunctionCurve::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dFunctionCurve( *this, options, refs );
}

bool a2dFunctionCurve::GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const
{
#if wxART2D_USE_CANEXTOBJ
    TYPENUMBER  result;
    int   a;
    m_toeval.ClearAllVars();

    m_toeval.SetValue( wxT( "x" ) , sweepValue );
    m_toeval.Evaluate( &result, &a );
    m_toeval.GetValue( wxT( "y" ) , &point.m_y );

    point.m_x = sweepValue;
    return true;
#else
    point.m_x = 0;
    point.m_y = 0;
    return false;
#endif //wxART2D_USE_CANEXTOBJ
}

a2dBoundingBox a2dFunctionCurve::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    double x;
    if ( m_clipdata.GetValid() )
    {
        m_curveBox.SetValid( false );
        double start = wxMax( m_clipdata.GetMinX(), m_xstart );
        double stop = wxMin( m_clipdata.GetMaxX(), m_xstop );
        for ( x = start; x <= stop; x = x + m_xstep )
        {
            double xw, yw;
            a2dPoint2D point;
            GetXyAtSweep( x, point );
            m_curveBox.Expand( point );
            Curve2World( point.m_x, point.m_y, xw, yw );
            bbox.Expand( xw , yw );
        }
    }
    else
        bbox.Expand( 0, 0 );
    return bbox;
}

bool a2dFunctionCurve::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );

        if ( m_clipdata.GetValid() )
        {
            double xw, yw;
            Curve2World( m_clipdata.GetMaxX(), m_clipdata.GetMaxY(), xw, yw );
            a2dBoundingBox box;
            box.Expand( xw, yw );
            Curve2World( m_clipdata.GetMinX(), m_clipdata.GetMinY(), xw, yw );
            box.Expand( xw, yw );
            box.MapBbox( m_lworld );

            m_bbox.And( &box );
        }

        a2dBoundingBox box = m_curvesArea->GetGroup()->GetPlotAreaBbox();
        m_bbox.And( &box );
        return true;
    }

    return false;
}

void a2dFunctionCurve::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    a2dBoundingBox curvebox = GetClippedCurveBoundaries();
    if( !curvebox.GetValid() )
        return;
    double x;
    bool second = false;
    double xprev = 0, yprev = 0;
    if ( m_clipdata.GetValid() )
    {
        wxRasterOperationMode prevlogfunction = wxNO_OP;
        wxDC* aDC = NULL;
        if( m_logFunction != wxNO_OP )
        {
            aDC = ic.GetDrawer2D()->GetRenderDC();
            prevlogfunction = aDC->GetLogicalFunction();
            aDC->SetLogicalFunction( m_logFunction );
        }
        for ( x = curvebox.GetMinX(); x < curvebox.GetMaxX(); x = x + m_xstep )
        {
            double xw, yw;
            a2dPoint2D point;
            GetXyAtSweep( x, point );
            Curve2World( x, point.m_y, xw, yw );
            if ( second )
            {
                ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw );
            }
            xprev = xw;
            yprev = yw;
            second = true;
        }
        if( aDC )
            aDC->SetLogicalFunction( prevlogfunction );
    }
}

bool a2dFunctionCurve::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

    double xw, yw, xcurve, ycurve;
    World2Curve( hitEvent.m_relx, hitEvent.m_rely, xcurve, ycurve );

    a2dPoint2D point;
    GetXyAtSweep( xcurve, point );
    Curve2World( point.m_x, point.m_y, xw, yw );

    if ( fabs( yw - hitEvent.m_rely ) < pw + margin )
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

#if wxART2D_USE_CVGIO
void a2dFunctionCurve::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCurve::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dFunctionCurve::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCurve::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dVertexCurve
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE( a2dVertexCurve, a2dCurve )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dVertexCurve::OnCanvasObjectMouseEvent )
    EVT_CANVASOBJECT_ENTER_EVENT(  a2dVertexCurve::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT(  a2dVertexCurve::OnLeaveObject )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dVertexCurve::OnHandleEvent )
END_EVENT_TABLE()


a2dVertexCurve::a2dVertexCurve( a2dVertexArray* points ): a2dCurve()
{
    m_points = points;
    m_spline = false;
    m_rendermode = a2dCURVES_RENDER_DEFAULT;
    m_editmode = a2dCURVES_EDIT_DEFAULT;
}

a2dVertexCurve::a2dVertexCurve( ): a2dCurve()
{
    m_points = new a2dVertexArray;
    m_spline = false;
    m_rendermode = a2dCURVES_RENDER_DEFAULT;
    m_editmode = a2dCURVES_EDIT_DEFAULT;
}


a2dVertexCurve::~a2dVertexCurve()
{
    delete m_points;
}

a2dVertexCurve::a2dVertexCurve( const a2dVertexCurve& other, CloneOptions options, a2dRefMap* refs )
    : a2dCurve( other, options, refs )
{
    m_points = new a2dVertexArray;
    *m_points = *other.m_points;

    m_spline = other.m_spline;

    m_rendermode = other.m_rendermode;
    m_editmode = other.m_editmode;
}

a2dObject* a2dVertexCurve::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dVertexCurve( *this, options, refs );
};

bool a2dVertexCurve::EliminateMatrix()
{
    if ( !m_lworld.IsIdentity() )
    {
        for( a2dVertexArray::const_iterator iter = m_points->begin(); iter != m_points->end(); ++iter )
        {
            a2dLineSegmentPtr seg = ( *iter )->Clone();
            m_lworld.TransformPoint( seg->m_x, seg->m_y, seg->m_x, seg->m_y );
        }

    }

    return a2dCanvasObject::EliminateMatrix();
}

void a2dVertexCurve::SetPosXYPoint( int n, double x, double y )
{
    m_points->Item( n )->m_x = x;
    m_points->Item( n )->m_y = y;

    SetPending( true );
}

void a2dVertexCurve::RemovePoint( double& x, double& y , int index )
{
    if ( index == -1 )
        index = m_points->size();

    x = m_points->Item( index )->m_x;
    y = m_points->Item( index )->m_y;
    m_points->RemoveAt( index );
}

void a2dVertexCurve::RemovePointWorld( double& x, double& y , int index, bool transformed )
{
    if ( index == -1 )
        index = m_points->size();

    x = m_points->Item( index )->m_x;
    y = m_points->Item( index )->m_y;
    m_points->RemoveAt( index );

    Curve2World( x, y, x, y );

    if ( transformed )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }

}

void a2dVertexCurve::AddPoint( const a2dPoint2D& P, int index )
{
    AddPoint( P.m_x, P.m_y, index );
}

void a2dVertexCurve::AddPoint( double x, double y, int index )
{
    a2dLineSegment* point = new a2dLineSegment( x, y );

    if ( index == -1 )
        m_points->push_back( point );
    else
        m_points->Insert( point, index );

    SetPending( true );
}

void a2dVertexCurve::GetPosXYPoint( int index, double& x, double& y )
{
    if ( index == -1 )
        index = m_points->size();

    x = m_points->Item( index )->m_x;
    y = m_points->Item( index )->m_y;
}

void a2dVertexCurve::GetPosXYPointWorld( int index, double& x, double& y, bool transform )
{
    if ( index == -1 )
        index = m_points->size();

    x = m_points->Item( index )->m_x;
    y = m_points->Item( index )->m_y;
    Curve2World( x, y, x, y );

    if ( transform )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }
}

bool a2dVertexCurve::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        if ( editmode == 1 )
        {
            m_flags.m_draggable = false;

            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            //add properties and handles as children in order to edit the object.
            a2dVertexArray* array = GetPoints();
            size_t i;
            for ( i = 0; i < array->size(); i++ )
            {
                double xw, yw;
                Curve2World( array->Item( i )->m_x, array->Item( i )->m_y, xw, yw );

                //not inclusive matrix so relative to polygon
                a2dHandle* handle = new a2dHandle( this, xw, yw, wxT( "__index__" ) );
                handle->SetLayer( m_layer );
                Append( handle );
                handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                PROPID_Index->SetPropertyToObject( handle, i );
            }
            m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );

            //calculate bbox's else mouse events may take place when first, and idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );

            //still set it pending to do the redraw ( in place )
            SetPending( true );
            return true;
        }
        else
        {
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

bool a2dVertexCurve::GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const
{
    double xp, yp;
    if( !m_points->size() )
        return false;

    xp = m_points->Item( 0 )->m_x;
    yp = m_points->Item( 0 )->m_y;

    if ( sweepValue <= m_points->Item( 0 )->m_x )
    {
        point.m_x = m_points->Item( 0 )->m_x;
        point.m_y = m_points->Item( 0 )->m_y;
        return false;
    }

    unsigned int i;
    for ( i = 0; i < m_points->size(); i++ )
    {
        if ( xp < sweepValue && m_points->Item( i )->m_x >= sweepValue )
        {
            point.m_x = sweepValue;
            point.m_y = ( m_points->Item( i )->m_y - yp ) * ( sweepValue - xp ) / ( m_points->Item( i )->m_x - xp ) + yp;
            return true;
        }
        xp = m_points->Item( i )->m_x;
        yp = m_points->Item( i )->m_y;
    }

    //return the end, so at least a normal value
    point.m_x = m_points->Item( i - 1 )->m_x;
    point.m_y = m_points->Item( i - 1 )->m_y;

    return false;
}

a2dBoundingBox a2dVertexCurve::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    unsigned int i;
    double xw, yw;
    m_curveBox.SetValid( false );
    for ( i = 0; i < m_points->size(); i++ )
    {
        m_curveBox.Expand( m_points->Item( i )->m_x, m_points->Item( i )->m_y );
        Curve2World( m_points->Item( i )->m_x, m_points->Item( i )->m_y, xw, yw );
        bbox.Expand( xw, yw );
    }
    return bbox;
}

bool a2dVertexCurve::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() && m_points->size() )
    {
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );

        if ( m_clipdata.GetValid() )
        {
            double xw, yw;
            Curve2World( m_clipdata.GetMaxX(), m_clipdata.GetMaxY(), xw, yw );
            a2dBoundingBox box;
            box.Expand( xw, yw );
            Curve2World( m_clipdata.GetMinX(), m_clipdata.GetMinY(), xw, yw );
            box.Expand( xw, yw );
            box.MapBbox( m_lworld );

            m_bbox.And( &box );
        }

        a2dBoundingBox box = m_curvesArea->GetGroup()->GetPlotAreaBbox();
        m_bbox.And( &box );
        return true;
    }
    return false;
}

void a2dVertexCurve::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
//    double minx = GetBoundaryMinX();
//    double maxx = GetBoundaryMaxX();
    a2dBoundingBox curvebox = GetClippedCurveBoundaries();
    if( !curvebox.GetValid() )
        return;

    unsigned int i = 0;
    double xw, yw;
    if ( !( m_rendermode & a2dCURVES_RENDER_NO_POINTS ) )
    {
        a2dStroke current = ic.GetDrawer2D()->GetDrawerStroke();
        a2dFill fillcurrent = ic.GetDrawer2D()->GetDrawerFill();

        a2dStroke highlight = a2dStroke( wxColour( 239, 2, 0 ), 3 );

        if ( m_highlightmode )
        {
            ic.GetDrawer2D()->SetDrawerStroke( highlight );
            ic.GetDrawer2D()->SetDrawerFill( *a2dBLACK_FILL );
        }
        else
        {
            ic.GetDrawer2D()->SetDrawerStroke( *a2dBLACK_STROKE );
            ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
        }

        a2dAffineMatrix matrix = ic.GetDrawer2D()->GetUserToDeviceTransform();
        ic.GetDrawer2D()->PushIdentityTransform();

        bool firstPoint = true;
        for ( i = 0; i < m_points->size(); i++ )
        {
            if ( m_points->Item( i )->m_x >= curvebox.GetMinX() )
            {
                if( firstPoint && i )
                {
                    Curve2World( m_points->Item( i - 1 )->m_x, m_points->Item( i - 1 )->m_y, xw, yw );
                    double xd, yd;
                    matrix.TransformPoint( xw, yw, xd, yd );

                    int radius = 2; //pixels
                    ic.GetDrawer2D()->DrawCircle( xd, yd, radius );
                }
                firstPoint = false;
                Curve2World( m_points->Item( i )->m_x, m_points->Item( i )->m_y, xw, yw );
                double xd, yd;
                matrix.TransformPoint( xw, yw, xd, yd );

                int radius = 2; //pixels
                ic.GetDrawer2D()->DrawCircle( xd, yd, radius );
            }
            if ( m_points->Item( i )->m_x > curvebox.GetMaxX() )
            {
                Curve2World( m_points->Item( i )->m_x, m_points->Item( i )->m_y, xw, yw );
                double xd, yd;
                matrix.TransformPoint( xw, yw, xd, yd );

                int radius = 2; //pixels
                ic.GetDrawer2D()->DrawCircle( xd, yd, radius );
                break;
            }
        }
        ic.GetDrawer2D()->PopTransform();

        ic.GetDrawer2D()->SetDrawerStroke( current );
        ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
    }

    wxRasterOperationMode prevlogfunction = wxNO_OP;
    wxDC* aDC = NULL;
    if( m_logFunction != wxNO_OP )
    {
        aDC = ic.GetDrawer2D()->GetRenderDC();
        wxASSERT_MSG( aDC, wxT( "TODO: aDC not defined for aggdrawer" ) );
        prevlogfunction = aDC->GetLogicalFunction();
        aDC->SetLogicalFunction( m_logFunction );
    }
    if ( m_rendermode & a2dCURVES_RENDER_NORMAL )
    {
        double  xprev = 0, yprev = 0;
        if( m_points->size() )
            Curve2World( m_points->Item( 0 )->m_x, m_points->Item( 0 )->m_y, xprev, yprev );
        int     iprev = 0;
        for ( i = 1; i < m_points->size(); i++ )
        {
            if ( m_points->Item( i )->m_x >= curvebox.GetMinX() )
            {
                Curve2World( m_points->Item( i )->m_x, m_points->Item( i )->m_y, xw, yw );
                if( iprev != i - 1 )
                    Curve2World( m_points->Item( i - 1 )->m_x, m_points->Item( i - 1 )->m_y, xprev, yprev );
                ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw );
                xprev = xw; yprev = yw;
                iprev = i;
            }
            if ( m_points->Item( i )->m_x > curvebox.GetMaxX() )
                break;
        }
    }
    else if ( m_rendermode & a2dCURVES_RENDER_FILL_LINE )
    {
        double xprev = 0, yprev = 0;
        double yw0;
        Curve2World( m_points->Item( i )->m_x, 0, xw, yw0 );
        for ( i = 1; i < m_points->size(); i++ )
        {
            if ( m_points->Item( i )->m_x >= curvebox.GetMinX() )
            {
                Curve2World( m_points->Item( i )->m_x, m_points->Item( i )->m_y, xw, yw );
                ic.GetDrawer2D()->DrawLine( xw, yw0, xw, yw );
                Curve2World( m_points->Item( i - 1 )->m_x, m_points->Item( i - 1 )->m_y, xprev, yprev );
                ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw );
            }
            if ( m_points->Item( i )->m_x > curvebox.GetMaxX() )
                break;
        }
    }
    else if ( m_rendermode & a2dCURVES_RENDER_FILL )
    {
        a2dVertexArray* cpoints = new a2dVertexArray;

        bool first = false;

        for ( i = 0; i < m_points->size(); i++ )
        {
            if ( m_points->Item( i )->m_x >= curvebox.GetMinX() )
            {
                Curve2World( m_points->Item( i )->m_x, m_points->Item( i )->m_y, xw, yw );

                if ( ! first )
                {
                    double xw0, yw0;
                    Curve2World( m_points->Item( i )->m_x, 0, xw0, yw0 );
                    cpoints->push_back( new a2dLineSegment( xw0, yw0 ) );
                }
                cpoints->push_back( new a2dLineSegment( xw, yw ) );
                first = true;
            }
            if ( m_points->Item( i )->m_x > curvebox.GetMaxX() )
                break;
        }
        double xw0, yw0;
        Curve2World( m_points->Item( i - 1 )->m_x, 0, xw0, yw0 );
        cpoints->push_back( new a2dLineSegment( xw0, yw0 ) );

        ic.GetDrawer2D()->DrawPolygon( cpoints, false, wxWINDING_RULE );
        delete cpoints;
    }
    if( aDC )
        aDC->SetLogicalFunction( prevlogfunction );
}

#if wxART2D_USE_CVGIO
void a2dVertexCurve::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCurve::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_spline )
        {
            out.WriteAttribute( wxT( "spline" ) , m_spline );
        }
        out.WriteAttribute( wxT( "points" ) , m_points->size() );
        out.WriteAttribute( wxT( "rendermode" ) , m_rendermode );
        out.WriteAttribute( wxT( "editmode" ) , m_editmode );
    }
    else
    {
        unsigned int i;
        for ( i = 0; i < m_points->size();  i++ )
        {
            if ( i % 6 == 0 )
                out.WriteNewLine();
            out.WriteStartElementAttributes( wxT( "xy" ) );
            out.WriteAttribute( wxT( "x" ) , m_points->Item( i )->m_x );
            out.WriteAttribute( wxT( "y" ) , m_points->Item( i )->m_y );
            out.WriteEndAttributes( true );
        }
    }
}

void a2dVertexCurve::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCurve::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_spline =  parser.GetAttributeValueBool( wxT( "spline" ), false );
        m_rendermode =  parser.GetAttributeValueInt( wxT( "rendermode" ) );
        m_editmode =  parser.GetAttributeValueInt( wxT( "editmode" ) );
    }
    else
    {
        while( parser.GetTagName() == wxT( "xy" )  )
        {
            double x = parser.GetAttributeValueDouble( wxT( "x" ) );
            double y = parser.GetAttributeValueDouble( wxT( "y" ) );
            a2dLineSegment* point = new a2dLineSegment( x, y );
            m_points->push_back( point );

            parser.Next();
            parser.Require( END_TAG, wxT( "xy" ) );
            parser.Next();
        }
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dVertexCurve::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{

    double xcurve, ycurve;
    World2Curve( hitEvent.m_relx, hitEvent.m_rely, xcurve, ycurve );

    a2dPoint2D P = a2dPoint2D( xcurve, ycurve );
    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

    if ( PointOnCurve( P, pw + margin ) )
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

bool a2dVertexCurve::PointOnCurve( const a2dPoint2D& P, double margin )
{
    bool    result = false;
    double  distance;
    a2dPoint2D p1, p2;

    if( m_points->size() )
    {
        p2 = m_points->Item( 0 )->GetPoint();
        unsigned int i;
        for ( i = 0; i < m_points->size() - 1; i++ )
        {
            p1 = p2;
            p2 = m_points->Item( i + 1 )->GetPoint();
            if ( margin > sqrt( pow( p1.m_x - P.m_x, 2 ) + pow( p1.m_y - P.m_y, 2 ) ) )
            {
                result = true;
                break;
            }
            else if ( !( ( p1.m_x == p2.m_x ) && ( p1.m_y == p2.m_y ) ) )
            {
                a2dLine line1( p1, p2 );
                if ( line1.PointInLine( P, distance, margin ) == R_IN_AREA )
                {
                    result = true;
                    break;
                }
            }
        }
    }

    return result;
}

void a2dVertexCurve::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        a2dAffineMatrix atWorld = ic->GetTransform();

        //we need to get the matrix until the parent of the a2dHandle,
        // and not inclusif the handle itself.
        //and therefore apply inverted handle matrix.
        a2dAffineMatrix inverselocal = m_lworld;
        inverselocal.Invert();

        a2dAffineMatrix inverse = ic->GetInverseParentTransform();

        double xinternal, yinternal;
        inverse.TransformPoint( xw, yw, xinternal, yinternal );
        //now in relative world coordinates, and so one more to get to curve coordinates.
        World2Curve( xinternal, yinternal, xinternal, yinternal );

        a2dHandle* draghandle = event.GetCanvasHandle();
        wxUint16 index = PROPID_Index->GetPropertyValue( draghandle );

        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );

        if ( editmode == 1 )
        {
            if ( !( m_editmode & a2dCURVES_EDIT_NORMAL ) )
            {
                a2dPoint2D p1;
                p1 = m_points->Item( index )->GetPoint();

                if ( m_editmode & a2dCURVES_EDIT_FIXED_X )
                {
                    xinternal = p1.m_x;
                }
                else if ( m_editmode & a2dCURVES_EDIT_FIXED_Y )
                {
                    yinternal = p1.m_y;
                }
            }

            a2dVertexCurve* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dVertexCurve );

            double xwi;
            double ywi;
            Curve2World( xinternal, yinternal, xwi, ywi );

            if ( event.GetMouseEvent().LeftUp() )
            {
                if ( draghandle->GetName() == wxT( "__index__" ) )
                {
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_VertexCurveMovePoint( original, xinternal, yinternal, index ) );
                }
                else
                    event.Skip(); //maybe the base class handles
            }
            else if ( event.GetMouseEvent().Dragging() )
            {
                SetPosXYPoint( index,  xinternal, yinternal );
                draghandle->SetPosXY( xwi, ywi );
            }
            else
                event.Skip(); //maybe on the base class handles
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

void a2dVertexCurve::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        //to world group coordinates to do hit test in world group coordinates
        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        if ( event.GetMouseEvent().LeftDown() && ( m_editmode & a2dCURVES_EDIT_ALLOW_ADD ) )
        {
            if ( event.GetHow().IsEdgeHit() && !PROPID_FirstEventInObject->GetPropertyValue( this ) )
            {
                a2dAffineMatrix inverse = ic->GetInverseParentTransform();

                double xinternal, yinternal;
                inverse.TransformPoint( xw, yw, xw, yw );
                // now in relative world coordinates, and so one more to get to curve coordinates.
                World2Curve( xw, yw, xinternal, yinternal );

                //add point to original and editcopy, at the right place
                a2dVertexCurve* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dVertexCurve );
                double x, y;
                size_t i;
                int insertpos = -1;
                for ( i = 0; i < GetPoints()->size(); i++ )
                {
                    GetPosXYPoint( i, x, y );
                    if ( x < xinternal )
                        insertpos = i;
                    else
                        break;
                }
                insertpos++;

                original->AddPoint( xinternal, yinternal, insertpos );
                AddPoint( xinternal, yinternal, insertpos );

                if ( m_childobjects != wxNullCanvasObjectList )
                {
                    forEachIn( a2dCanvasObjectList, m_childobjects )
                    {
                        a2dCanvasObject* obj = *iter;
                        a2dHandle* handle = wxDynamicCast( obj, a2dHandle );
                        if ( handle && !handle->GetRelease( ) )
                        {
                            wxUint16 index = PROPID_Index->GetPropertyValue( handle );
                            if ( index >= insertpos )
                                PROPID_Index->SetPropertyToObject( handle, index + 1 );
                        }
                    }
                }

                //add handle for point
                a2dHandle* handle = new a2dHandle( this, xw, yw, wxT( "__index__" ) );
                handle->SetSpecificFlags( true, a2dCanvasOFlags::EditingCopy );
                Append( handle );

                handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                PROPID_Index->SetPropertyToObject( handle, insertpos );
                ic->SetCorridorPathToObject( handle );
            }
            else
                event.Skip();
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

void a2dVertexCurve::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    //ic->GetTransform().DebugDump( "group", 0, 0);

    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
    // PushLayer(wxLAYER_ONTOP);
    m_highlightmode = 1;
    SetPending( true );
    event.Skip();
}

void a2dVertexCurve::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    //a2dIterC* ic = event.GetIterC();

    event.Skip();
    // PopLayer();
    m_highlightmode = 0;
    SetPending( true );
}



//----------------------------------------------------------------------------
// a2dSweepCurve
//----------------------------------------------------------------------------
a2dSweepPoint::a2dSweepPoint( double sweep, double a, double b )
{
    m_sweep = sweep;
    m_a = a;
    m_b = b;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( a2dSweepPointArray );

BEGIN_EVENT_TABLE( a2dSweepCurve, a2dCurve )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dSweepCurve::OnCanvasObjectMouseEvent )
    EVT_CANVASOBJECT_ENTER_EVENT(  a2dSweepCurve::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT(  a2dSweepCurve::OnLeaveObject )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dSweepCurve::OnHandleEvent )
END_EVENT_TABLE()


a2dSweepCurve::a2dSweepCurve( a2dSweepPointArray* points ): a2dCurve()
{
    m_points = points;
    m_spline = false;
    m_rendermode = a2dCURVES_RENDER_DEFAULT;
    m_editmode = a2dCURVES_EDIT_DEFAULT;
}

a2dSweepCurve::a2dSweepCurve( ): a2dCurve()
{
    m_points = new a2dSweepPointArray;
    m_spline = false;
    m_rendermode = a2dCURVES_RENDER_DEFAULT;
    m_editmode = a2dCURVES_EDIT_DEFAULT;
}


a2dSweepCurve::~a2dSweepCurve()
{
    delete m_points;
}

a2dSweepCurve::a2dSweepCurve( const a2dSweepCurve& other, CloneOptions options, a2dRefMap* refs )
    : a2dCurve( other, options, refs )
{
    m_points = new a2dSweepPointArray;
    unsigned int i;
    for ( i = 0; i < other.m_points->size(); i++ )
        m_points->push_back( new a2dSweepPoint( other.m_points->Item( i ) ) );

    m_spline = other.m_spline;

    m_rendermode = other.m_rendermode;
    m_editmode = other.m_editmode;
}

a2dObject* a2dSweepCurve::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dSweepCurve( *this, options, refs );
};

bool a2dSweepCurve::EliminateMatrix()
{
    unsigned int i;
    if ( !m_lworld.IsIdentity() )
    {
        for ( i = 0; i < m_points->size(); i++ )
        {
            m_lworld.TransformPoint( m_points->Item( i ).m_a, m_points->Item( i ).m_b, m_points->Item( i ).m_a, m_points->Item( i ).m_b );
        }

    }

    return a2dCanvasObject::EliminateMatrix();
}

void a2dSweepCurve::SetPosXYPoint( int n, double x, double y )
{
    m_points->Item( n ).m_a = x;
    m_points->Item( n ).m_b = y;

    SetPending( true );
}

void a2dSweepCurve::RemovePoint( double& x, double& y , int index )
{
    if ( index == -1 )
        index = m_points->size();

    x = m_points->Item( index ).m_a;
    y = m_points->Item( index ).m_b;
    m_points->RemoveAt( index );
}

void a2dSweepCurve::RemovePointWorld( double& x, double& y , int index, bool transformed )
{
    if ( index == -1 )
        index = m_points->size();

    x = m_points->Item( index ).m_a;
    y = m_points->Item( index ).m_b;
    m_points->RemoveAt( index );

    Curve2World( x, y, x, y );

    if ( transformed )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }

}

void a2dSweepCurve::AddPoint( double sweep, const a2dPoint2D& P )
{
    AddPoint( sweep, P.m_x, P.m_y );
}

void a2dSweepCurve::AddPoint( double sweep, double x, double y )
{
    a2dSweepPoint* point = new a2dSweepPoint( sweep, x, y );

    m_points->push_back( point );
    SetPending( true );
}

void a2dSweepCurve::GetPosXYPoint( int index, double& x, double& y )
{
    if ( index == -1 )
        index = m_points->size();

    x = m_points->Item( index ).m_a;
    y = m_points->Item( index ).m_b;
}

void a2dSweepCurve::GetPosXYPointWorld( int index, double& x, double& y, bool transform )
{
    if ( index == -1 )
        index = m_points->size();

    x = m_points->Item( index ).m_a;
    y = m_points->Item( index ).m_b;
    Curve2World( x, y, x, y );

    if ( transform )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }
}

bool a2dSweepCurve::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        if ( editmode == 1 )
        {
            m_flags.m_draggable = false;

            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            //add properties and handles as children in order to edit the object.
            a2dSweepPointArray* array = GetPoints();
            size_t i;
            for ( i = 0; i < array->size(); i++ )
            {
                double xw, yw;
                Curve2World( array->Item( i ).m_a, array->Item( i ).m_b, xw, yw );
                a2dHandle* handle = new a2dHandle( this, xw, yw, wxT( "__index__" ) );
                handle->SetLayer( m_layer );
                Append( handle );
                handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                PROPID_Index->SetPropertyToObject( handle, i );

                //not inclusive matrix so relative to polygon
            }
            m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );

            //calculate bbox's else mouse events may take place when first, and idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );
            //still set it pending to do the redraw ( in place )
            SetPending( true );
            return true;
        }
        else
        {
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

bool a2dSweepCurve::GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const
{
    double xp, yp;
    if( !m_points->size() )
        return false;

    xp = m_points->Item( 0 ).m_a;
    yp = m_points->Item( 0 ).m_b;
    if ( sweepValue <= m_points->Item( 0 ).m_a )
    {
        point.m_x = m_points->Item( 0 ).m_a;
        point.m_y = m_points->Item( 0 ).m_b;
        return false;
    }

    unsigned int i;
    for ( i = 0; i < m_points->size(); i++ )
    {
        if ( xp < sweepValue && m_points->Item( i ).m_a >= sweepValue )
        {
            point.m_x = sweepValue;
            point.m_y = ( m_points->Item( i ).m_b - yp ) * ( sweepValue - xp ) / ( m_points->Item( i ).m_a - xp ) + yp;
            return true;
        }
        xp = m_points->Item( i ).m_a;
        yp = m_points->Item( i ).m_b;
    }

    //return the end, so at least a normal value
    point.m_x = m_points->Item( i - 1 ).m_a;
    point.m_y = m_points->Item( i - 1 ).m_b;

    return false;
}

a2dBoundingBox a2dSweepCurve::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    unsigned int i;
    double xw, yw;
    m_curveBox.SetValid( false );
    for ( i = 0; i < m_points->size(); i++ )
    {
        m_curveBox.Expand( m_points->Item( i ).m_a, m_points->Item( i ).m_b );
        Curve2World( m_points->Item( i ).m_a, m_points->Item( i ).m_b, xw, yw );
        bbox.Expand( xw, yw );
    }
    return bbox;
}

bool a2dSweepCurve::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() && m_points->size() )
    {
        //trigger children ( markers etc. which normally do change when curve changes )
        if ( m_childobjects != wxNullCanvasObjectList )
            m_childobjects->SetSpecificFlags( true, a2dCanvasOFlags::PENDING );

        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );

        if ( m_clipdata.GetValid() )
        {
            double xw, yw;
            Curve2World( m_clipdata.GetMaxX(), m_clipdata.GetMaxY(), xw, yw );
            a2dBoundingBox box;
            box.Expand( xw, yw );
            Curve2World( m_clipdata.GetMinX(), m_clipdata.GetMinY(), xw, yw );
            box.Expand( xw, yw );
            box.MapBbox( m_lworld );

            m_bbox.And( &box );
        }

        a2dBoundingBox box = m_curvesArea->GetGroup()->GetPlotAreaBbox();
        m_bbox.And( &box );
        return true;
    }
    return false;
}

void a2dSweepCurve::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    // use next for clipping
    //double minx = GetBoundaryMinX();
    //double maxx = GetBoundaryMaxX();

    unsigned int i = 0;
    double xw, yw;
    if ( !( m_rendermode & a2dCURVES_RENDER_NO_POINTS ) )
    {
        a2dStroke current = ic.GetDrawer2D()->GetDrawerStroke();
        a2dFill fillcurrent = ic.GetDrawer2D()->GetDrawerFill();

        a2dStroke highlight = a2dStroke( wxColour( 239, 2, 0 ), 3 );

        if ( m_highlightmode )
        {
            ic.GetDrawer2D()->SetDrawerStroke( highlight );
            ic.GetDrawer2D()->SetDrawerFill( *a2dBLACK_FILL );
        }
        else
        {
            ic.GetDrawer2D()->SetDrawerStroke( *a2dBLACK_STROKE );
            ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
        }

        a2dAffineMatrix matrix = ic.GetDrawer2D()->GetUserToDeviceTransform();
        ic.GetDrawer2D()->PushIdentityTransform();

        for ( i = 0; i < m_points->size(); i++ )
        {
            Curve2World( m_points->Item( i ).m_a, m_points->Item( i ).m_b, xw, yw );
            double xd, yd;
            matrix.TransformPoint( xw, yw, xd, yd );

            int radius = 2; //pixels
            ic.GetDrawer2D()->DrawCircle( xd, yd, radius );
        }
        ic.GetDrawer2D()->PopTransform();

        ic.GetDrawer2D()->SetDrawerStroke( current );
        ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
    }

    wxRasterOperationMode prevlogfunction = wxNO_OP;
    wxDC* aDC = NULL;
    if( m_logFunction != wxNO_OP )
    {
        aDC = ic.GetDrawer2D()->GetRenderDC();
        if( aDC )
        {
            prevlogfunction = aDC->GetLogicalFunction();
            aDC->SetLogicalFunction( m_logFunction );
        }
    }
    if ( m_rendermode & a2dCURVES_RENDER_NORMAL )
    {
        double xprev = 0, yprev = 0;
        for ( i = 1; i < m_points->size(); i++ )
        {
            Curve2World( m_points->Item( i ).m_a, m_points->Item( i ).m_b, xw, yw );
            Curve2World( m_points->Item( i - 1 ).m_a, m_points->Item( i - 1 ).m_b, xprev, yprev );
            ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw );
        }
    }
    else if ( m_rendermode & a2dCURVES_RENDER_FILL_LINE )
    {
        double xprev = 0, yprev = 0;
        double yw0;
        Curve2World( m_points->Item( i ).m_a, 0, xw, yw0 );
        for ( i = 1; i < m_points->size(); i++ )
        {
            Curve2World( m_points->Item( i ).m_a, m_points->Item( i ).m_b, xw, yw );
            ic.GetDrawer2D()->DrawLine( xw, yw0, xw, yw );
            Curve2World( m_points->Item( i - 1 ).m_a, m_points->Item( i - 1 ).m_b, xprev, yprev );
            ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw );
        }
    }
    else if ( m_rendermode & a2dCURVES_RENDER_FILL )
    {
        a2dVertexArray* cpoints = new a2dVertexArray();

        bool first = false;

        for ( i = 0; i < m_points->size(); i++ )
        {
            Curve2World( m_points->Item( i ).m_a, m_points->Item( i ).m_b, xw, yw );

            if ( ! first )
            {
                double xw0, yw0;
                Curve2World( m_points->Item( i ).m_a, 0, xw0, yw0 );
                cpoints->push_back( new a2dLineSegment( xw0, yw0 ) );
            }
            cpoints->push_back( new a2dLineSegment( xw, yw ) );
            first = true;
        }
        double xw0, yw0;
        Curve2World( m_points->Item( i - 1 ).m_a, 0, xw0, yw0 );
        cpoints->push_back( new a2dLineSegment( xw0, yw0 ) );

        ic.GetDrawer2D()->DrawPolygon( cpoints, false, wxWINDING_RULE );
        delete cpoints;

        double xprev = 0, yprev = 0;
        for ( i = 1; i < m_points->size(); i++ )
        {
            Curve2World( m_points->Item( i ).m_a, m_points->Item( i ).m_b, xw, yw );
            Curve2World( m_points->Item( i - 1 ).m_a, m_points->Item( i - 1 ).m_b, xprev, yprev );
            ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw );
        }
    }
    if( aDC )
        aDC->SetLogicalFunction( prevlogfunction );
}

#if wxART2D_USE_CVGIO
void a2dSweepCurve::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCurve::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_spline )
        {
            out.WriteAttribute( wxT( "spline" ) , m_spline );
        }
        out.WriteAttribute( wxT( "points" ) , m_points->size() );
        out.WriteAttribute( wxT( "rendermode" ) , m_rendermode );
        out.WriteAttribute( wxT( "editmode" ) , m_editmode );
    }
    else
    {
        unsigned int i;
        for ( i = 0; i < m_points->size();  i++ )
        {
            if ( i % 6 == 0 )
                out.WriteNewLine();
            out.WriteStartElementAttributes( wxT( "xy" ) );
            out.WriteAttribute( wxT( "x" ) , m_points->Item( i ).m_a );
            out.WriteAttribute( wxT( "y" ) , m_points->Item( i ).m_b );
            out.WriteEndAttributes( true );
        }
    }
}

void a2dSweepCurve::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCurve::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_spline =  parser.GetAttributeValueBool( wxT( "spline" ), false );
        m_rendermode =  parser.GetAttributeValueInt( wxT( "rendermode" ) );
        m_editmode =  parser.GetAttributeValueInt( wxT( "editmode" ) );
    }
    else
    {
        while( parser.GetTagName() == wxT( "xy" )  )
        {
            double sweep = parser.GetAttributeValueDouble( wxT( "sweep" ) );
            double x = parser.GetAttributeValueDouble( wxT( "x" ) );
            double y = parser.GetAttributeValueDouble( wxT( "y" ) );
            a2dSweepPoint* point = new a2dSweepPoint( sweep, x, y );
            m_points->push_back( point );

            parser.Next();
            parser.Require( END_TAG, wxT( "xy" ) );
            parser.Next();
        }
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dSweepCurve::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double xcurve, ycurve;
    World2Curve( hitEvent.m_relx, hitEvent.m_rely, xcurve, ycurve );

    a2dPoint2D P = a2dPoint2D( xcurve, ycurve );
    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

    if ( PointOnCurve( P, pw + margin ) )
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

bool a2dSweepCurve::PointOnCurve( const a2dPoint2D& P, double margin )
{
    bool    result = false;
    double  distance;
    a2dPoint2D p1, p2;

    if( m_points->size() )
    {
        p2 = m_points->Item( 0 ).GetPoint();
        unsigned int i;
        for ( i = 0; i < m_points->size() - 1; i++ )
        {
            p1 = p2;
            p2 = m_points->Item( i + 1 ).GetPoint();
            if ( margin > sqrt( pow( p1.m_x - P.m_x, 2 ) + pow( p1.m_y - P.m_y, 2 ) ) )
            {
                result = true;
                break;
            }
            else if ( !( ( p1.m_x == p2.m_x ) && ( p1.m_y == p2.m_y ) ) )
            {
                a2dLine line1( p1, p2 );
                if ( line1.PointInLine( P, distance, margin ) == R_IN_AREA )
                {
                    result = true;
                    break;
                }
            }
        }
    }

    return result;
}

void a2dSweepCurve::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        a2dAffineMatrix atWorld = ic->GetTransform();

        //we need to get the matrix until the parent of the a2dHandle,
        // and not inclusif the handle itself.
        //and therefore apply inverted handle matrix.
        a2dAffineMatrix inverselocal = m_lworld;
        inverselocal.Invert();

        a2dAffineMatrix inverse = ic->GetInverseParentTransform();

        double xinternal, yinternal;
        inverse.TransformPoint( xw, yw, xinternal, yinternal );
        //now in relative world coordinates, and so one more to get to curve coordinates.
        World2Curve( xinternal, yinternal, xinternal, yinternal );

        a2dHandle* draghandle = event.GetCanvasHandle();
        wxUint16 index = PROPID_Index->GetPropertyValue( draghandle );

        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );

        if ( editmode == 1 )
        {
            if ( !( m_editmode & a2dCURVES_EDIT_NORMAL ) )
            {
                a2dPoint2D p1;
                p1 = m_points->Item( index ).GetPoint();

                if ( m_editmode & a2dCURVES_EDIT_FIXED_X )
                {
                    xinternal = p1.m_x;
                }
                else if ( m_editmode & a2dCURVES_EDIT_FIXED_Y )
                {
                    yinternal = p1.m_y;
                }
            }

            a2dSweepCurve* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dSweepCurve );

            double xwi;
            double ywi;
            Curve2World( xinternal, yinternal, xwi, ywi );

            if ( event.GetMouseEvent().LeftUp() )
            {
                if ( draghandle->GetName() == wxT( "__index__" ) )
                {
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_SweepCurveMovePoint( original, xinternal, yinternal, index ) );
                }
                else
                    event.Skip(); //maybe the base class handles
            }
            else if ( event.GetMouseEvent().Dragging() )
            {
                SetPosXYPoint( index,  xinternal, yinternal );
                draghandle->SetPosXY( xwi, ywi );
            }
            else
                event.Skip(); //maybe on the base class handles
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

void a2dSweepCurve::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    event.Skip();
}

void a2dSweepCurve::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
    // PushLayer(wxLAYER_ONTOP);
    m_highlightmode = 1;
    SetPending( true );
    event.Skip();
}

void a2dSweepCurve::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    //a2dIterC* ic = event.GetIterC();

    event.Skip();
    // PopLayer();
    m_highlightmode = 0;
    SetPending( true );
}


//----------------------------------------------------------------------------
// a2dCanvasBandCurveHandle
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasBandCurveHandle, a2dHandle )

a2dCanvasBandCurveHandle::a2dCanvasBandCurveHandle()
    : a2dHandle( NULL, 0, 0, wxT( "dummy" ), 5, 5 )
{
    m_index = 0;
    m_arcMid = false;
}

a2dCanvasBandCurveHandle::a2dCanvasBandCurveHandle( a2dBandCurve* parent,
        unsigned int index, bool arcMid, double xc, double yc, const wxString& name )
    : a2dHandle( parent, xc, yc, name )
{
    m_index   = index;
    m_arcMid  = arcMid;
}

a2dCanvasBandCurveHandle::a2dCanvasBandCurveHandle( const a2dCanvasBandCurveHandle& other, CloneOptions options, a2dRefMap* refs )
    : a2dHandle( other, options, refs )
{
    m_index   = other.m_index;
    m_arcMid   = other.m_arcMid;
}

a2dCanvasBandCurveHandle::~a2dCanvasBandCurveHandle()
{
}

//----------------------------------------------------------------------------
// a2dBandCurve
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dBandCurve, a2dCurve )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dBandCurve::OnCanvasObjectMouseEvent )
    EVT_CANVASOBJECT_ENTER_EVENT(  a2dBandCurve::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT(  a2dBandCurve::OnLeaveObject )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dBandCurve::OnHandleEvent )
END_EVENT_TABLE()


a2dBandCurve::a2dBandCurve( a2dSweepPointArray* points ): a2dCurve()
{
    m_points = points;
    m_spline = false;
    m_rendermode = a2dCURVES_RENDER_DEFAULT;
    m_editmode = a2dCURVES_EDIT_DEFAULT;
    m_markerUpLow = true;
}

a2dBandCurve::a2dBandCurve( ): a2dCurve()
{
    m_points = new a2dSweepPointArray;
    m_spline = false;
    m_rendermode = a2dCURVES_RENDER_DEFAULT;
    m_editmode = a2dCURVES_EDIT_DEFAULT;
    m_markerUpLow = true;
}


a2dBandCurve::~a2dBandCurve()
{
    delete m_points;
}

a2dBandCurve::a2dBandCurve( const a2dBandCurve& other, CloneOptions options, a2dRefMap* refs )
    : a2dCurve( other, options, refs )
{
    m_points = new a2dSweepPointArray;
    unsigned int i;
    for ( i = 0; i < other.m_points->size(); i++ )
        m_points->push_back( new a2dSweepPoint( other.m_points->Item( i ) ) );

    m_spline = other.m_spline;

    m_rendermode = other.m_rendermode;
    m_editmode = other.m_editmode;
    m_markerUpLow = other.m_markerUpLow;
}

a2dObject* a2dBandCurve::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dBandCurve( *this, options, refs );
};

bool a2dBandCurve::EliminateMatrix()
{
    unsigned int i;
    if ( !m_lworld.IsIdentity() )
    {
        for ( i = 0; i < m_points->size(); i++ )
        {
            m_lworld.TransformPoint( m_points->Item( i ).m_a, m_points->Item( i ).m_b, m_points->Item( i ).m_a, m_points->Item( i ).m_b );
        }

    }

    return a2dCanvasObject::EliminateMatrix();
}

void a2dBandCurve::SetPosXYPoint( int index, double sweep, double y, bool upper )
{
    m_points->Item( index ).m_sweep = sweep;
    if ( upper )
        m_points->Item( index ).m_b = y;
    else
        m_points->Item( index ).m_a = y;

    SetPending( true );
}

void a2dBandCurve::RemovePoint( double& sweep, double& y, bool upper, int index )
{
    if ( index == -1 )
        index = m_points->size();

    sweep = m_points->Item( index ).m_sweep;
    if ( upper )
        y = m_points->Item( index ).m_b;
    else
        y = m_points->Item( index ).m_a;

    m_points->RemoveAt( index );
}

void a2dBandCurve::RemovePointWorld( double& sweep, double& y, bool upper, int index, bool transformed )
{
    if ( index == -1 )
        index = m_points->size();

    sweep = m_points->Item( index ).m_sweep;
    if ( upper )
        y = m_points->Item( index ).m_b;
    else
        y = m_points->Item( index ).m_a;

    m_points->RemoveAt( index );

    Curve2World( sweep, y, sweep, y );

    if ( transformed )
    {
        m_lworld.TransformPoint( sweep, y, sweep, y );
    }

}

void a2dBandCurve::AddPoint( double sweep, const a2dPoint2D& P )
{
    AddPoint( sweep, P.m_x, P.m_y );
}

void a2dBandCurve::AddPoint( double sweep, double x, double y )
{
    a2dSweepPoint* point = new a2dSweepPoint( sweep, x, y );

    m_points->push_back( point );
    SetPending( true );
}

void a2dBandCurve::GetPosXYPoint( int index, double& sweep, double& y, bool upper )
{
    if ( index == -1 )
        index = m_points->size();

    sweep = m_points->Item( index ).m_sweep;
    if ( upper )
        y = m_points->Item( index ).m_b;
    else
        y = m_points->Item( index ).m_a;
}

void a2dBandCurve::GetPosXYPointWorld( int index, double& sweep, double& y, bool transform, bool upper )
{
    if ( index == -1 )
        index = m_points->size();

    sweep = m_points->Item( index ).m_sweep;
    if ( upper )
        y = m_points->Item( index ).m_b;
    else
        y = m_points->Item( index ).m_a;
    Curve2World( sweep, y, sweep, y );

    if ( transform )
    {
        m_lworld.TransformPoint( sweep, y, sweep, y );
    }
}

bool a2dBandCurve::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        if ( editmode == 1 )
        {
            m_flags.m_draggable = false;

            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            //add properties and handles as children in order to edit the object.
            a2dSweepPointArray* array = GetPoints();
            size_t i;
            for ( i = 0; i < array->size(); i++ )
            {
                double xw, yw, yw2;
                Curve2World( array->Item( i ).m_sweep, array->Item( i ).m_a, xw, yw );
                a2dCanvasBandCurveHandle* handle = new a2dCanvasBandCurveHandle( this, i, false, xw, yw, wxT( "__indexlow__" ) );
                handle->SetLayer( m_layer );
                Append( handle );
                Curve2World( array->Item( i ).m_sweep, array->Item( i ).m_b, xw, yw2 );
                handle = new a2dCanvasBandCurveHandle( this, i, false, xw, yw2, wxT( "__indexhigh__" ) );
                Append( handle );
            }
            m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );

            //calculate bbox's else mouse events may take place when first, and idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );
            //still set it pending to do the redraw ( in place )
            SetPending( true );
            return true;
        }
        else
        {
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

void a2dBandCurve::SyncHandlesWithLineSegments()
{
    for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;

        a2dCanvasBandCurveHandle* handle = wxDynamicCast( obj, a2dCanvasBandCurveHandle );

        if ( handle && !handle->GetRelease( ) )
        {
            unsigned int index = handle->GetIndex();
            double xw, yw;
            if ( handle->GetName() == wxT( "__indexlow__" ) )
            {
                Curve2World( m_points->Item( index ).m_sweep, m_points->Item( index ).m_a, xw, yw );
                handle->SetPosXY( xw, yw );
            }
            else if ( handle->GetName() == wxT( "__indexhigh__" ) )
            {
                Curve2World( m_points->Item( index ).m_sweep, m_points->Item( index ).m_b, xw, yw );
                handle->SetPosXY( xw, yw );
            }
        }
    }
}

void a2dBandCurve::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        a2dAffineMatrix atWorld = ic->GetTransform();

        //we need to get the matrix until the parent of the a2dHandle,
        // and not inclusif the handle itself.
        //and therefore apply inverted handle matrix.
        a2dAffineMatrix inverselocal = m_lworld;
        inverselocal.Invert();

        a2dAffineMatrix inverse = ic->GetInverseParentTransform();

        double xinternal, yinternal;
        inverse.TransformPoint( xw, yw, xinternal, yinternal );
        //now in relative world coordinates, and so one more to get to curve coordinates.
        World2Curve( xinternal, yinternal, xinternal, yinternal );

        a2dCanvasBandCurveHandle* draghandle = wxDynamicCast( event.GetCanvasHandle(), a2dCanvasBandCurveHandle );
        wxUint16 index = draghandle->GetIndex();

        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );

        if ( editmode == 1 )
        {
            if ( !( m_editmode & a2dCURVES_EDIT_NORMAL ) )
            {
                if ( m_editmode & a2dCURVES_EDIT_FIXED_X )
                {
                    xinternal = m_points->Item( index ).m_sweep;
                }
                else if ( m_editmode & a2dCURVES_EDIT_FIXED_Y )
                {
                    if ( draghandle->GetName() == wxT( "__indexlow__" ) )
                        yinternal = m_points->Item( index ).m_a;
                    else
                        yinternal = m_points->Item( index ).m_b;
                }
            }

            a2dBandCurve* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dBandCurve );

            double xwi;
            double ywi;
            Curve2World( xinternal, yinternal, xwi, ywi );

            if ( event.GetMouseEvent().LeftUp() )
            {
                if ( draghandle->GetName() == wxT( "__indexlow__" ) )
                {
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_BandCurveMovePoint( original, xinternal, yinternal, false, index ) );
                    SyncHandlesWithLineSegments();
                }
                else if ( draghandle->GetName() == wxT( "__indexhigh__" ) )
                {
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_BandCurveMovePoint( original, xinternal, yinternal, true, index ) );
                    SyncHandlesWithLineSegments();
                }
                else
                    event.Skip(); //maybe the base class handles
            }
            else if ( event.GetMouseEvent().Dragging() )
            {
                if ( draghandle->GetName() == wxT( "__indexlow__" ) )
                {
                    SetPosXYPoint( index,  xinternal, yinternal, false );
                }
                else if ( draghandle->GetName() == wxT( "__indexhigh__" ) )
                {
                    SetPosXYPoint( index,  xinternal, yinternal, true );
                }
                draghandle->SetPosXY( xwi, ywi );
            }
            else
                event.Skip(); //maybe on the base class handles
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

bool a2dBandCurve::GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const
{
    double xp, yp;
    if( !m_points->size() )
        return false;


    xp = m_points->Item( 0 ).m_sweep;
    if ( m_markerUpLow )
        yp = m_points->Item( 0 ).m_b;
    else
        yp = m_points->Item( 0 ).m_b;
    if ( sweepValue <= m_points->Item( 0 ).m_sweep )
    {
        point.m_x = m_points->Item( 0 ).m_sweep;
        if ( m_markerUpLow )
            point.m_y = m_points->Item( 0 ).m_b;
        else
            point.m_y = m_points->Item( 0 ).m_a;
        return false;
    }

    unsigned int i;
    for ( i = 0; i < m_points->size(); i++ )
    {
        if ( xp < sweepValue && m_points->Item( i ).m_sweep >= sweepValue )
        {
            point.m_x = sweepValue;
            if ( m_markerUpLow )
                point.m_y = ( m_points->Item( i ).m_b - yp ) * ( sweepValue - xp ) / ( m_points->Item( i ).m_sweep - xp ) + yp;
            else
                point.m_y = ( m_points->Item( i ).m_a - yp ) * ( sweepValue - xp ) / ( m_points->Item( i ).m_sweep - xp ) + yp;

            return true;
        }
        xp = m_points->Item( i ).m_sweep;
        if ( m_markerUpLow )
            yp = m_points->Item( i ).m_b;
        else
            yp = m_points->Item( i ).m_a;
    }

    //return the end, so at least a normal value
    point.m_x = m_points->Item( i - 1 ).m_sweep;
    if ( m_markerUpLow )
        point.m_y = m_points->Item( i - 1 ).m_b;
    else
        point.m_y = m_points->Item( i - 1 ).m_a;
    return false;
}

a2dBoundingBox a2dBandCurve::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    unsigned int i;
    double xw, yw;
    m_curveBox.SetValid( false );
    for ( i = 0; i < m_points->size(); i++ )
    {
        m_curveBox.Expand( m_points->Item( i ).m_sweep, m_points->Item( i ).m_a );
        Curve2World( m_points->Item( i ).m_sweep, m_points->Item( i ).m_a, xw, yw );
        bbox.Expand( xw, yw );
        m_curveBox.Expand( m_points->Item( i ).m_sweep, m_points->Item( i ).m_b );
        Curve2World( m_points->Item( i ).m_sweep, m_points->Item( i ).m_b, xw, yw );
        bbox.Expand( xw, yw );
    }
    return bbox;
}

bool a2dBandCurve::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() && m_points->size() )
    {
        //trigger children ( markers etc. which normally do change when curve changes )
        if ( m_childobjects != wxNullCanvasObjectList )
            m_childobjects->SetSpecificFlags( true, a2dCanvasOFlags::PENDING );

        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );

        if ( m_clipdata.GetValid() )
        {
            double xw, yw;
            Curve2World( m_clipdata.GetMaxX(), m_clipdata.GetMaxY(), xw, yw );
            a2dBoundingBox box;
            box.Expand( xw, yw );
            Curve2World( m_clipdata.GetMinX(), m_clipdata.GetMinY(), xw, yw );
            box.Expand( xw, yw );
            box.MapBbox( m_lworld );

            m_bbox.And( &box );
        }

        a2dBoundingBox box = m_curvesArea->GetGroup()->GetPlotAreaBbox();
        m_bbox.And( &box );
        return true;
    }
    return false;
}

void a2dBandCurve::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
//    double minx = GetBoundaryMinX();
//    double maxx = GetBoundaryMaxX();
    a2dBoundingBox curvebox = GetClippedCurveBoundaries();
    if( !curvebox.GetValid() )
        return ;

    unsigned int i;
    double xw, yw, yw2;
    if ( !( m_rendermode & a2dCURVES_RENDER_NO_POINTS ) )
    {
        a2dStroke current = ic.GetDrawer2D()->GetDrawerStroke();
        a2dFill fillcurrent = ic.GetDrawer2D()->GetDrawerFill();

        a2dStroke highlight = a2dStroke( wxColour( 239, 2, 0 ), 3 );

        if ( m_highlightmode )
        {
            ic.GetDrawer2D()->SetDrawerStroke( highlight );
            ic.GetDrawer2D()->SetDrawerFill( *a2dBLACK_FILL );
        }
        else
        {
            ic.GetDrawer2D()->SetDrawerStroke( *a2dBLACK_STROKE );
            ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
        }

        a2dAffineMatrix matrix = ic.GetDrawer2D()->GetUserToDeviceTransform();
        ic.GetDrawer2D()->PushIdentityTransform();

        int radius = 2; //pixels
        for ( i = 0; i < m_points->size(); i++ )
        {
            if ( m_points->Item( i ).m_sweep >= curvebox.GetMinX() )
            {
                Curve2World( m_points->Item( i ).m_sweep, m_points->Item( i ).m_a, xw, yw );
                Curve2World( m_points->Item( i ).m_sweep, m_points->Item( i ).m_b, xw, yw2 );
                double xd, yd, xd2, yd2;

                matrix.TransformPoint( xw, yw, xd, yd );
                matrix.TransformPoint( xw, yw2, xd2, yd2 );
                ic.GetDrawer2D()->DrawCircle( xd, yd, radius );
                ic.GetDrawer2D()->DrawCircle( xd2, yd2, radius );
            }
            if ( m_points->Item( i ).m_sweep > curvebox.GetMaxX() )
                break;
        }
        ic.GetDrawer2D()->PopTransform();

        ic.GetDrawer2D()->SetDrawerStroke( current );
        ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
    }

    wxRasterOperationMode prevlogfunction = wxNO_OP;
    wxDC* aDC = NULL;
    if( m_logFunction != wxNO_OP )
    {
        aDC = ic.GetDrawer2D()->GetRenderDC();
        prevlogfunction = aDC->GetLogicalFunction();
        aDC->SetLogicalFunction( m_logFunction );
    }
    if ( m_rendermode & a2dCURVES_RENDER_NORMAL )
    {
        bool second = false;
        double xprev = 0, yprev = 0;
        for ( i = 0; i < m_points->size(); i++ )
        {
            if ( second && m_points->Item( i ).m_sweep >= curvebox.GetMinX() )
            {
                Curve2World( m_points->Item( i ).m_sweep, m_points->Item( i ).m_a, xw, yw );
                Curve2World( m_points->Item( i - 1 ).m_sweep, m_points->Item( i - 1 ).m_a, xprev, yprev );
                ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw );
                Curve2World( m_points->Item( i ).m_sweep, m_points->Item( i ).m_b, xw, yw2 );
                Curve2World( m_points->Item( i - 1 ).m_sweep, m_points->Item( i - 1 ).m_b, xprev, yprev );
                ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw2 );
            }
            if ( m_points->Item( i ).m_sweep > curvebox.GetMaxX() )
                break;
            second = true;
        }
    }
    else if ( m_rendermode & a2dCURVES_RENDER_FILL_LINE )
    {
        bool second = false;
        double xprev = 0, yprev = 0;
        for ( i = 0; i < m_points->size(); i++ )
        {
            if ( second && m_points->Item( i ).m_sweep >= curvebox.GetMinX() )
            {
                Curve2World( m_points->Item( i ).m_sweep, m_points->Item( i ).m_a, xw, yw );
                Curve2World( m_points->Item( i - 1 ).m_sweep, m_points->Item( i - 1 ).m_a, xprev, yprev );
                Curve2World( m_points->Item( i ).m_sweep, m_points->Item( i ).m_b, xw, yw2 );
                ic.GetDrawer2D()->DrawLine( xw, yw, xw, yw2 );
                ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw );
                Curve2World( m_points->Item( i - 1 ).m_sweep, m_points->Item( i - 1 ).m_b, xprev, yprev );
                ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw2 );

            }
            if ( m_points->Item( i ).m_sweep > curvebox.GetMaxX() )
                break;
            second = true;
        }
    }
    else if ( m_rendermode & a2dCURVES_RENDER_FILL )
    {
        a2dVertexArray* cpoints = new a2dVertexArray();
        for ( i = 0; i < m_points->size(); i++ )
        {
            Curve2World( m_points->Item( i ).m_sweep, m_points->Item( i ).m_a, xw, yw );
            Curve2World( m_points->Item( i ).m_sweep, m_points->Item( i ).m_b, xw, yw2 );

            cpoints->Insert( new a2dLineSegment( xw, yw ), 0 );

            cpoints->push_back( new a2dLineSegment( xw, yw2 ) );
        }

        ic.GetDrawer2D()->DrawPolygon( cpoints, false, wxWINDING_RULE );
        delete cpoints;
    }
    if( aDC )
        aDC->SetLogicalFunction( prevlogfunction );
}

#if wxART2D_USE_CVGIO
void a2dBandCurve::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCurve::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_spline )
        {
            out.WriteAttribute( wxT( "spline" ) , m_spline );
        }
        out.WriteAttribute( wxT( "points" ) , m_points->size() );
        out.WriteAttribute( wxT( "rendermode" ) , m_rendermode );
        out.WriteAttribute( wxT( "editmode" ) , m_editmode );
    }
    else
    {
        unsigned int i;
        for ( i = 0; i < m_points->size();  i++ )
        {
            if ( i % 6 == 0 )
                out.WriteNewLine();
            out.WriteStartElementAttributes( wxT( "ab" ) );
            out.WriteAttribute( wxT( "sweep" ) , m_points->Item( i ).m_sweep );
            out.WriteAttribute( wxT( "a" ) , m_points->Item( i ).m_a );
            out.WriteAttribute( wxT( "b" ) , m_points->Item( i ).m_b );
            out.WriteEndAttributes( true );
        }
    }
}

void a2dBandCurve::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCurve::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_spline =  parser.GetAttributeValueBool( wxT( "spline" ), false );
        m_rendermode =  parser.GetAttributeValueInt( wxT( "rendermode" ) );
        m_editmode =  parser.GetAttributeValueInt( wxT( "editmode" ) );
    }
    else
    {
        while( parser.GetTagName() == wxT( "ab" )  )
        {
            double sweep = parser.GetAttributeValueDouble( wxT( "sweep" ) );
            double x = parser.GetAttributeValueDouble( wxT( "a" ) );
            double y = parser.GetAttributeValueDouble( wxT( "b" ) );
            a2dSweepPoint* point = new a2dSweepPoint( sweep, x, y );
            m_points->push_back( point );

            parser.Next();
            parser.Require( END_TAG, wxT( "ab" ) );
            parser.Next();
        }
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dBandCurve::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double xh, yh;
    ic.GetInverseTransform().TransformPoint( hitEvent.m_x, hitEvent.m_y, xh, yh );

    double xcurve, ycurve;
    World2Curve( xh, yh, xcurve, ycurve );

    a2dPoint2D P = a2dPoint2D( xcurve, ycurve );
    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

    if ( PointOnCurve( P, pw + margin ) )
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

bool a2dBandCurve::PointOnCurve( const a2dPoint2D& P, double margin )
{
    bool    result = false;
    double  distance;
    a2dPoint2D pup1, pup2, plow1, plow2;

    if( m_points->size() )
    {
        pup2 = m_points->Item( 0 ).GetPoint();
        unsigned int i;
        for ( i = 0; i < m_points->size() - 1; i++ )
        {
            R_PointStatus up = R_RIGHT_SIDE;
            R_PointStatus low = R_RIGHT_SIDE;

            pup1 = pup2;
            pup2 = m_points->Item( i + 1 ).GetPointUp();
            if ( margin > sqrt( pow( pup1.m_x - P.m_x, 2 ) + pow( pup1.m_y - P.m_y, 2 ) ) )
            {
                result = true;
                break;
            }
            else if ( !( ( pup1.m_x == pup2.m_x ) && ( pup1.m_y == pup2.m_y ) ) )
            {
                a2dLine line1( pup1, pup2 );
                if ( ( up = line1.PointInLine( P, distance, margin ) ) == R_IN_AREA )
                {
                    result = true;
                    break;
                }
            }
            plow1 = plow2;
            plow2 = m_points->Item( i + 1 ).GetPointLow();
            if ( margin > sqrt( pow( plow1.m_x - P.m_x, 2 ) + pow( plow1.m_y - P.m_y, 2 ) ) )
            {
                result = true;
                break;
            }
            else if ( !( ( plow1.m_x == plow2.m_x ) && ( plow1.m_y == plow2.m_y ) ) )
            {
                a2dLine line1( pup1, pup2 );
                if ( ( low = line1.PointInLine( P, distance, margin ) ) == R_IN_AREA )
                {
                    result = true;
                    break;
                }
            }

            if ( pup1.m_x > plow1.m_x && up == R_RIGHT_SIDE && low == R_LEFT_SIDE )
            {
                result = true;
                break;
            }
            if ( pup1.m_x < plow1.m_x && low == R_RIGHT_SIDE && up == R_LEFT_SIDE )
            {
                result = true;
                break;
            }

        }
    }

    return result;
}

void a2dBandCurve::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    event.Skip();
}

void a2dBandCurve::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
    // PushLayer(wxLAYER_ONTOP);
    m_highlightmode = 1;
    SetPending( true );
    event.Skip();
}

void a2dBandCurve::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    //a2dIterC* ic = event.GetIterC();

    event.Skip();
    // PopLayer();
    m_highlightmode = 0;
    SetPending( true );
}



//----------------------------------------------------------------------------
// a2dBarCurve
//----------------------------------------------------------------------------
const a2dPropertyIdRefObject a2dBarCurve::PROPID_extrudeFill( wxT( "extrudeFill" ), a2dPropertyId::flag_none, 0 );

void a2dBarCurve::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dCanvasObject::DoWalker( parent, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}

a2dBarCurve::a2dBarCurve( a2dVertexArray* points ): a2dVertexCurve( points )
{
    m_barwidth = 0;
    m_depth = m_barwidth / 2;
    m_angle3d  = wxDegToRad( 30 );
    m_extrudeFill = *a2dBLACK_FILL;
    m_rendermode = a2dBAR_RENDER_NORMAL;
}

a2dBarCurve::a2dBarCurve( ): a2dVertexCurve()
{
    m_barwidth = 0;
    m_depth = m_barwidth / 2;
    m_angle3d  = wxDegToRad( 30 );
    m_extrudeFill = *a2dBLACK_FILL;
    m_rendermode = a2dBAR_RENDER_NORMAL;
}


a2dBarCurve::~a2dBarCurve()
{
}

a2dBarCurve::a2dBarCurve( const a2dBarCurve& other, CloneOptions options, a2dRefMap* refs )
    : a2dVertexCurve( other, options, refs )
{
    m_barwidth = other.m_barwidth;
    m_depth = other.m_depth;
    m_angle3d = other.m_angle3d;

    m_extrudeFill = other.m_extrudeFill;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in property->TClone()
    CurrentSmartPointerOwner = this;
#endif
}

a2dObject* a2dBarCurve::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dBarCurve( *this, options, refs );
};

void a2dBarCurve::SetExtrudeFill( const a2dFill& fill )
{
    m_extrudeFill = fill;
}

void a2dBarCurve::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    double xw, yw;
    a2dStroke current = ic.GetDrawer2D()->GetDrawerStroke();
    a2dFill fillcurrent = ic.GetDrawer2D()->GetDrawerFill();
    if ( m_highlightmode )
    {
        a2dStroke tmps = a2dStroke( wxColour( 239, 232, 0 ), 3 );
        ic.GetDrawer2D()->SetDrawerStroke( tmps );
        ic.GetDrawer2D()->SetDrawerFill( *a2dBLACK_FILL );
    }
    else
    {
        ic.GetDrawer2D()->SetDrawerStroke( *a2dBLACK_STROKE );
        ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
    }

    a2dAffineMatrix matrix = ic.GetDrawer2D()->GetUserToDeviceTransform();
    ic.GetDrawer2D()->PushIdentityTransform();

    if ( !( m_rendermode & a2dCURVES_RENDER_NO_POINTS ) )
    {
        unsigned int i;
        for ( i = 0; i < m_points->size(); i++ )
        {
            Curve2World( m_points->Item( i )->m_x, m_points->Item( i )->m_y, xw, yw );

            double xd, yd;
            matrix.TransformPoint( xw, yw, xd, yd );
            int radius = 2; //pixels
            ic.GetDrawer2D()->DrawCircle( xd, yd, radius );
        }
    }
    ic.GetDrawer2D()->PopTransform();

    ic.GetDrawer2D()->SetDrawerStroke( current );
    ic.GetDrawer2D()->SetDrawerFill( fillcurrent );

    wxRasterOperationMode prevlogfunction = wxNO_OP;
    wxDC* aDC = NULL;
    if( m_logFunction != wxNO_OP )
    {
        aDC = ic.GetDrawer2D()->GetRenderDC();
        prevlogfunction = aDC->GetLogicalFunction();
        aDC->SetLogicalFunction( m_logFunction );
    }

    double yw0;
    Curve2World( m_points->Item( 0 )->m_x, 0, xw, yw0 );
    switch ( m_rendermode )
    {
        default:
        case a2dBAR_RENDER_NORMAL:
        {
            double w2 = m_barwidth / 2;
            unsigned int i;
            a2dlist< a2dFill >::iterator iter = m_pointsFill.begin();
            for ( i = 0; i < m_points->size(); i++ )
            {
                if ( m_pointsFill.size() )
                    ic.GetDrawer2D()->SetDrawerFill( *iter );

                Curve2World( m_points->Item( i )->m_x, m_points->Item( i )->m_y, xw, yw );
                ic.GetDrawer2D()->DrawRoundedRectangle( xw - w2, yw0, m_barwidth, yw - yw0, 0 );

                iter = m_pointsFill.GetNextAround( iter );
            }
            break;
        }
        case a2dBAR_RENDER_3D:
        {
            a2dVertexArray* cpoints = new a2dVertexArray;
            double w2 = m_barwidth / 2;

            a2dlist< a2dFill >::iterator iter = m_pointsFill.begin();
            unsigned int i;
            for ( i = 0; i < m_points->size(); i++ )
            {
                Curve2World( m_points->Item( i )->m_x, m_points->Item( i )->m_y, xw, yw );
                double barx = xw;
                double barlow = yw0;
                double barheight = yw;

                if ( barheight < 0 )
                {
                    barlow = yw;
                    barheight = 0;
                }

                ic.GetDrawer2D()->SetDrawerFill( m_extrudeFill );

                //top plate
                cpoints->clear();
                cpoints->push_back( new a2dLineSegment( barx - w2, barheight ) );
                cpoints->push_back( new a2dLineSegment( barx - w2 + cos( m_angle3d - wxPI / 30 )*m_depth, barheight + sin( m_angle3d - wxPI / 30 )*m_depth ) );
                cpoints->push_back( new a2dLineSegment( barx + w2 + cos( m_angle3d )*m_depth, barheight + sin( m_angle3d - wxPI / 30 )*m_depth ) );
                cpoints->push_back( new a2dLineSegment( barx + w2, barheight ) );
                ic.GetDrawer2D()->DrawPolygon( cpoints, false, wxWINDING_RULE );

                //side plate
                cpoints->clear();
                cpoints->push_back( new a2dLineSegment( barx + w2, barheight ) );
                cpoints->push_back( new a2dLineSegment( barx + w2 + cos( m_angle3d )*m_depth, barheight + sin( m_angle3d - wxPI / 30 )*m_depth ) );
                cpoints->push_back( new a2dLineSegment( barx + w2 + cos( m_angle3d )*m_depth, barlow + sin( m_angle3d )*m_depth ) );
                cpoints->push_back( new a2dLineSegment( barx + w2, barlow ) );

                ic.GetDrawer2D()->DrawPolygon( cpoints, false, wxWINDING_RULE );

                if ( m_pointsFill.size() )
                    ic.GetDrawer2D()->SetDrawerFill( *iter );
                else
                    ic.GetDrawer2D()->SetDrawerFill( fillcurrent );

                ic.GetDrawer2D()->DrawRoundedRectangle( xw - w2, yw0, m_barwidth, yw - yw0, 0 );

                iter = m_pointsFill.GetNextAround( iter );
            }
            delete cpoints;
            break;
        }
        case a2dBAR_RENDER_LINE:
        {
            unsigned int i;
            for ( i = 0; i < m_points->size(); i++ )
            {
                Curve2World( m_points->Item( i )->m_x, m_points->Item( i )->m_y, xw, yw );
                ic.GetDrawer2D()->DrawLine( xw, yw0, xw, yw );
            }
            break;
        }
    }
    if( aDC )
        aDC->SetLogicalFunction( prevlogfunction );

    ic.GetDrawer2D()->SetDrawerStroke( current );
    ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
}

#if wxART2D_USE_CVGIO
void a2dBarCurve::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dVertexCurve::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "barwidth" ) , m_barwidth );
        out.WriteAttribute( wxT( "depth" ) , m_depth );
        out.WriteAttribute( wxT( "angle3d" ) , m_angle3d );
    }
    else
    {
        out.WriteStartElement( wxT( "derived" ) );
        m_extrudeFill.Save( this, out, towrite );
        out.WriteEndElement();
    }
}

void a2dBarCurve::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dVertexCurve::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_barwidth = parser.RequireAttributeValueDouble( wxT( "barwidth" ) );
        m_depth = parser.RequireAttributeValueDouble( wxT( "depth" ) );
        m_angle3d = parser.RequireAttributeValueDouble( wxT( "angle3d" ) );
    }
    else
    {
        parser.Require( START_TAG, wxT( "derived" ) );
        parser.Next();
        m_extrudeFill.Load( parent, parser );
        parser.Require( END_TAG, wxT( "derived" ) );
        parser.Next();
    }
}
#endif //wxART2D_USE_CVGIO



//----------------------------------------------------------------------------
// a2dVertexListCurve
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE( a2dVertexListCurve, a2dCurve )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dVertexListCurve::OnCanvasObjectMouseEvent )
    EVT_CANVASOBJECT_ENTER_EVENT(  a2dVertexListCurve::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT(  a2dVertexListCurve::OnLeaveObject )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dVertexListCurve::OnHandleEvent )
END_EVENT_TABLE()


a2dVertexListCurve::a2dVertexListCurve( a2dVertexList* points ): a2dCurve()
{
    m_lpoints = points;
    m_spline = false;
    m_rendermode = a2dCURVES_RENDER_DEFAULT;
    m_editmode = a2dCURVES_EDIT_DEFAULT;
}

a2dVertexListCurve::a2dVertexListCurve( ): a2dCurve()
{
    m_lpoints = new a2dVertexList();
    m_spline = false;
    m_rendermode = a2dCURVES_RENDER_DEFAULT;
    m_editmode = a2dCURVES_EDIT_DEFAULT;
}


a2dVertexListCurve::~a2dVertexListCurve()
{
    delete m_lpoints;
}

a2dVertexListCurve::a2dVertexListCurve( const a2dVertexListCurve& other, CloneOptions options, a2dRefMap* refs )
    : a2dCurve( other, options, refs )
{
    m_lpoints = new a2dVertexList();
    *m_lpoints = *other.m_lpoints;

    m_spline = other.m_spline;
    m_rendermode = other.m_rendermode;
    m_editmode = other.m_editmode;
}

a2dObject* a2dVertexListCurve::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dVertexListCurve( *this, options, refs );
};

bool a2dVertexListCurve::EliminateMatrix()
{
    if ( !m_lworld.IsIdentity() )
    {
        for( a2dVertexList::iterator iter = m_lpoints->begin(); iter != m_lpoints->end(); ++iter )
        {
            a2dLineSegment* point = ( *iter );
            m_lworld.TransformPoint( point->m_x, point->m_y, point->m_x, point->m_y );
        }

    }
    return a2dCanvasObject::EliminateMatrix();
}

void a2dVertexListCurve::SetPosXYPoint( int n, double x, double y, bool afterinversion  )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x, y, x, y );
    }

    int i = 0;
    for( a2dVertexList::iterator iter = m_lpoints->begin(); iter != m_lpoints->end(); ++iter )
    {
        if ( i == n )
        {
            a2dLineSegment* point = ( *iter );
            point->m_x = x;
            point->m_y = y;
            break;
        }
        i++;
    }
    SetPending( true );
}

void a2dVertexListCurve::AddPoint( const a2dPoint2D& P, int index, bool afterinversion )
{
    AddPoint( P.m_x, P.m_y, index, afterinversion );
}

void a2dVertexListCurve::AddPoint( double x, double y, int index, bool afterinversion )
{
    if ( afterinversion )
    {
        a2dAffineMatrix inverse =  m_lworld;
        inverse.Invert();
        inverse.TransformPoint( x, y, x, y );
    }

    a2dLineSegment* point = new a2dLineSegment( x, y );

    if ( index == -1 )
        m_lpoints->push_back( point );
    else
    {
        a2dVertexList::iterator iter = m_lpoints->begin();
        int i = 0;
        while ( i < index ) { ++iter; i++; }
        m_lpoints->insert( iter, point );
    }
    SetPending( true );
}

void a2dVertexListCurve::RemovePoint( double& x, double& y , int index, bool transformed )
{
    if ( index == -1 )
    {
        a2dLineSegment* point;
        point = m_lpoints->back();
        x = point->m_x;
        y = point->m_y;
        m_lpoints->pop_back();
    }
    else
    {
        int i = 0;
        a2dVertexList::iterator iter = m_lpoints->begin();
        a2dLineSegment* point;
        while ( iter != m_lpoints->end() )
        {
            if ( i == index )
            {
                point = ( *iter );
                x = point->m_x;
                y = point->m_y;
                m_lpoints->erase( iter );
                break;
            }
            i++;
            iter++;
        }
    }

    if ( transformed )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }

    SetPending( true );
}

void a2dVertexListCurve::GetPosXYPoint( int index, double& x, double& y, bool transform )
{
    if ( index == -1 )
    {
        a2dLineSegment* point;
        point = m_lpoints->back();
        x = point->m_x;
        y = point->m_y;
    }
    else
    {
        int i = 0;
        a2dVertexList::iterator iter = m_lpoints->begin();
        a2dLineSegment* point;
        while ( iter != m_lpoints->end() )
        {
            if ( i == index )
            {
                point = ( *iter );
                x = point->m_x;
                y = point->m_y;
                break;
            }
            i++;
            iter++;
        }
    }

    if ( transform )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }
}

bool a2dVertexListCurve::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        if ( editmode == 1 )
        {
            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            //add properties and handles as children in order to edit the object.
            int index = 0;
            forEachIn( a2dVertexList, GetPoints() )
            {
                a2dLineSegment* point = *iter;
                //not inclusive matrix so relative to polygon
                a2dHandle* handle = new a2dHandle( this, point->m_x, point->m_y, wxT( "__index__" ) );
                handle->SetLayer( m_layer );
                Append( handle );
                handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                PROPID_Index->SetPropertyToObject( handle, index++ );
            }

            m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );
            m_flags.m_editingCopy = true;

            //calculate bbox's elase mouse events may take place when first idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );
            //stil set it pending to do the redraw ( in place )
            SetPending( true );
            return true;
        }
        else
        {
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

bool a2dVertexListCurve::GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const
{
    double xp, yp;
    if( !m_lpoints->size() )
        return false;

    a2dVertexList::iterator  iter = m_lpoints->begin();
    a2dPoint2D pointc = ( *iter )->GetPoint();
    xp = pointc.m_x;
    yp = pointc.m_y;

    while ( iter != m_lpoints->end() )
    {
        a2dPoint2D pointc = ( *iter )->GetPoint();
        if ( xp < sweepValue && pointc.m_x >= sweepValue )
        {
            point.m_x = sweepValue;
            point.m_y = ( pointc.m_y - yp ) * ( pointc.m_x - xp ) / ( pointc.m_x - xp ) + yp;
            return true;
        }
        xp = pointc.m_x;
        yp = pointc.m_y;
        iter++;
    }

    //return the end, so at least a normal value
    point.m_x = sweepValue;
    point.m_y = yp;

    return false;
}

a2dBoundingBox a2dVertexListCurve::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    double xw, yw;
    m_curveBox.SetValid( false );
    if (  m_spline )
    {
        a2dVertexList* lpoints = new a2dVertexList();
        a2dLineSegment* pointo;
        a2dVertexList::iterator iter;
        for( iter = m_lpoints->begin(); iter != m_lpoints->end(); ++iter )
        {
            pointo = ( *iter );
            a2dLineSegment* point = new a2dLineSegment( *pointo );
            lpoints->push_back( point );
        }

        lpoints->ConvertSplinedPolygon( 10 );
        for( iter = lpoints->begin(); iter != lpoints->end(); ++iter )
        {
            a2dLineSegment* point = ( *iter );
            m_curveBox.Expand( point->m_x, point->m_y );
            Curve2World( point->m_x, point->m_y, xw, yw );
            bbox.Expand( xw, yw );
        }
        delete lpoints;
    }
    else
    {
        for( a2dVertexList::iterator iter = m_lpoints->begin(); iter != m_lpoints->end(); ++iter )
        {
            a2dLineSegment* point = ( *iter );
            m_curveBox.Expand( point->m_x, point->m_y );
            Curve2World( point->m_x, point->m_y, xw, yw );
            bbox.Expand( xw, yw );
        }
    }
    return bbox;
}

bool a2dVertexListCurve::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        //trigger children ( markers etc. which normally do change when curve changes )
        if ( m_childobjects != wxNullCanvasObjectList )
            m_childobjects->SetSpecificFlags( true, a2dCanvasOFlags::PENDING );

        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );

        if ( m_flags.m_editingCopy && m_spline )
        {
            m_spline = false;
            a2dBoundingBox bboxs = DoGetUnTransformedBbox();
            bboxs.MapBbox( m_lworld );
            m_bbox.Expand( bboxs );
            m_spline = true;

            if ( m_clipdata.GetValid() )
            {
                double xw, yw;
                Curve2World( m_clipdata.GetMaxX(), m_clipdata.GetMaxY(), xw, yw );
                a2dBoundingBox box;
                box.Expand( xw, yw );
                Curve2World( m_clipdata.GetMinX(), m_clipdata.GetMinY(), xw, yw );
                box.Expand( xw, yw );
                box.MapBbox( m_lworld );

                m_bbox.And( &box );
            }
        }

        a2dBoundingBox box = m_curvesArea->GetGroup()->GetPlotAreaBbox();
        m_bbox.And( &box );
        return true;
    }
    return false;
}

void a2dVertexListCurve::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
//    double minx = GetBoundaryMinX();
//    double maxx = GetBoundaryMaxX();
    a2dBoundingBox curvebox = GetClippedCurveBoundaries();
    if( !curvebox.GetValid() )
        return;

    double xw, yw;
    if ( !( m_rendermode & a2dCURVES_RENDER_NO_POINTS ) )
    {
        a2dStroke current = ic.GetDrawer2D()->GetDrawerStroke();
        a2dFill fillcurrent = ic.GetDrawer2D()->GetDrawerFill();

        if ( m_highlightmode )
        {
            a2dStroke tmps = a2dStroke( wxColour( 239, 232, 0 ), 3 );
            ic.GetDrawer2D()->SetDrawerStroke( tmps );
            ic.GetDrawer2D()->SetDrawerFill( *a2dBLACK_FILL );
        }
        else
        {
            ic.GetDrawer2D()->SetDrawerStroke( *a2dBLACK_STROKE );
            ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
        }

        a2dAffineMatrix matrix = ic.GetDrawer2D()->GetUserToDeviceTransform();
        ic.GetDrawer2D()->PushIdentityTransform();

        forEachIn( a2dVertexList, m_lpoints )
        {
            a2dLineSegment* point = *iter;
            if ( point->m_x >= curvebox.GetMinX() && point->m_x <= curvebox.GetMaxX() )
            {
                Curve2World( point->m_x, point->m_y, xw, yw );

                double xd, yd;
                matrix.TransformPoint( xw, yw, xd, yd );
                int radius = 2; //pixels
                ic.GetDrawer2D()->DrawCircle( xd, yd, radius );
            }
            if ( point->m_x > curvebox.GetMaxX() )
                break;
        }
        ic.GetDrawer2D()->PopTransform();

        ic.GetDrawer2D()->SetDrawerStroke( current );
        ic.GetDrawer2D()->SetDrawerFill( fillcurrent );
    }

    wxRasterOperationMode prevlogfunction = wxNO_OP;
    wxDC* aDC = NULL;
    if( m_logFunction != wxNO_OP )
    {
        aDC = ic.GetDrawer2D()->GetRenderDC();
        prevlogfunction = aDC->GetLogicalFunction();
        aDC->SetLogicalFunction( m_logFunction );
    }

    if ( m_rendermode & a2dCURVES_RENDER_NORMAL )
    {
        bool second = false;
        double xprev = 0, yprev = 0;
        a2dLineSegment* lastpoint = NULL;
        forEachIn( a2dVertexList, m_lpoints )
        {
            a2dLineSegment* point = *iter;
            if ( second && point->m_x >= curvebox.GetMinX() )
            {
                Curve2World( lastpoint->m_x, lastpoint->m_y, xprev, yprev );
                Curve2World( point->m_x, point->m_y, xw, yw );
                ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw );
            }
            if ( point->m_x > curvebox.GetMaxX() )
                break;

            lastpoint = point;

            second = true;
        }
    }
    else if ( m_rendermode & a2dCURVES_RENDER_FILL_LINE )
    {
        bool second = false;
        double xprev = 0, yprev = 0;
        double yw0;
        a2dLineSegment* point = *( m_lpoints->begin() );
        Curve2World( point->m_x, 0, xw, yw0 );
        a2dLineSegment* lastpoint = NULL;
        forEachIn( a2dVertexList, m_lpoints )
        {
            a2dLineSegment* point = *iter;
            if ( second && point->m_x >= curvebox.GetMinX()  )
            {
                Curve2World( lastpoint->m_x, lastpoint->m_y, xprev, yprev );
                Curve2World( point->m_x, point->m_y, xw, yw );
                ic.GetDrawer2D()->DrawLine( xw, yw0, xw, yw );
                ic.GetDrawer2D()->DrawLine( xprev, yprev, xw, yw );
            }
            if ( point->m_x > curvebox.GetMaxX() )
                break;

            lastpoint = point;
            second = true;
        }
    }
    if( aDC )
        aDC->SetLogicalFunction( prevlogfunction );
}

#if wxART2D_USE_CVGIO
void a2dVertexListCurve::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCurve::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_spline )
        {
            out.WriteAttribute( wxT( "spline" ) , m_spline );
        }
        out.WriteAttribute( wxT( "points" ) , m_lpoints->size() );
        out.WriteAttribute( wxT( "rendermode" ) , m_rendermode );
        out.WriteAttribute( wxT( "editmode" ) , m_editmode );
    }
    else
    {
        unsigned int i = 0;
        forEachIn( a2dVertexList, m_lpoints )
        {
            if ( i % 6 == 0 )
                out.WriteNewLine();
            a2dLineSegment* point = *iter;
            out.WriteStartElementAttributes( wxT( "xy" ) );
            out.WriteAttribute( wxT( "x" ) , point->m_x );
            out.WriteAttribute( wxT( "y" ) , point->m_y );
            out.WriteEndAttributes( true );
            i++;
        }
    }
}

void a2dVertexListCurve::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCurve::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_spline =  parser.GetAttributeValueBool( wxT( "spline" ), false );
        m_rendermode =  parser.GetAttributeValueInt( wxT( "rendermode" ) );
        m_editmode =  parser.GetAttributeValueInt( wxT( "editmode" ) );
    }
    else
    {
        while( parser.GetTagName() == wxT( "xy" )  )
        {
            double x = parser.GetAttributeValueDouble( wxT( "x" ) );
            double y = parser.GetAttributeValueDouble( wxT( "y" ) );
            a2dLineSegment* point = new a2dLineSegment( x, y );
            m_lpoints->push_back( point );

            parser.Next();
            parser.Require( END_TAG, wxT( "xy" ) );
            parser.Next();
        }
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dVertexListCurve::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double xcurve, ycurve;
    World2Curve( hitEvent.m_relx, hitEvent.m_rely, xcurve, ycurve );

    a2dPoint2D P = a2dPoint2D( xcurve, ycurve );
    double pw = ic.GetWorldStrokeExtend();
    double margin = ic.GetTransformedHitMargin();

    if ( PointOnCurve( P, pw + margin ) )
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

bool a2dVertexListCurve::PointOnCurve( const a2dPoint2D& P, double margin )
{
    bool    result = false;
    double  distance;
    a2dPoint2D p1, p2;

    a2dVertexList* rlist;
    if (  m_spline )
    {

        a2dVertexList* lpoints = new a2dVertexList();
        for( a2dVertexList::iterator iter = m_lpoints->begin(); iter != m_lpoints->end(); ++iter )
        {
            a2dLineSegment* point = new a2dLineSegment( *( *iter ) );
            lpoints->push_back( point );
        }

        lpoints->ConvertSplinedPolyline( 10 );
        rlist = lpoints;
    }
    else
        rlist = m_lpoints;

    if ( rlist->size() )
    {
        a2dVertexList::iterator iter = rlist->begin();
        p2 = ( *iter )->GetPoint();
        while ( iter != rlist->end() && !result )
        {
            p1 = p2;
            iter++;
            if ( iter != rlist->end() ) break;
            p2 = ( *iter )->GetPoint();

            if ( margin > sqrt( pow( p1.m_x - P.m_x, 2 ) + pow( p1.m_y - P.m_y, 2 ) ) )
                result = true;
            else if ( !( ( p1.m_x == p2.m_x ) && ( p1.m_y == p2.m_y ) ) )
            {
                a2dLine line1( p1, p2 );
                if ( line1.PointInLine( P, distance, margin ) == R_IN_AREA )
                    result = true;
            }
        }
    }

    if (  m_spline )
    {
        for( a2dVertexList::iterator iter = rlist->begin(); iter != rlist->end(); ++iter )
        {
            a2dLineSegment* point = ( *iter );
            GetBbox().Expand( point->m_x, point->m_y );
        }
        delete rlist;
    }

    return result;
}

void a2dVertexListCurve::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        a2dAffineMatrix atWorld = ic->GetTransform();

        //we need to get the matrix until the parent of the a2dHandle,
        // and not inclusif the handle itself.
        //and therefore apply inverted handle matrix.
        a2dAffineMatrix inverselocal = m_lworld;
        inverselocal.Invert();

        a2dAffineMatrix inverse = ic->GetInverseParentTransform();

        double xinternal, yinternal;
        inverse.TransformPoint( xw, yw, xinternal, yinternal );

        a2dHandle* draghandle = event.GetCanvasHandle();
        wxUint16 index = PROPID_Index->GetPropertyValue( draghandle );

        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );

        if ( editmode == 1 )
        {
            a2dVertexListCurve* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dVertexListCurve );

            double xwi;
            double ywi;
            inverselocal.TransformPoint( xinternal, yinternal, xwi, ywi );

            if ( event.GetMouseEvent().LeftUp() )
            {
                if ( draghandle->GetName() == wxT( "__index__" ) )
                {
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_VertexListCurveMovePoint( original, xinternal, yinternal, index ) );
                }
                else
                    event.Skip(); //maybe the base class handles
            }
            else if ( event.GetMouseEvent().Dragging() )
            {
                SetPosXYPoint( index,  xinternal, yinternal );
                draghandle->SetPosXY( xwi, ywi );
            }
            else
                event.Skip(); //maybe on the base class handles
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

void a2dVertexListCurve::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    event.Skip();
}

void a2dVertexListCurve::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();
    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
    // PushLayer(wxLAYER_ONTOP);
    m_highlightmode = 1;
    SetPending( true );
    event.Skip();
}

void a2dVertexListCurve::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    //a2dIterC* ic = event.GetIterC();
    event.Skip();
    // PopLayer();
    m_highlightmode = 0;
    SetPending( true );
}



//----------------------------------------------------------------------------
// a2dPieCurve
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE( a2dPieCurve, a2dCurve )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dPieCurve::OnCanvasObjectMouseEvent )
    EVT_CANVASOBJECT_ENTER_EVENT(  a2dPieCurve::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT(  a2dPieCurve::OnLeaveObject )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dPieCurve::OnHandleEvent )
END_EVENT_TABLE()


a2dPieCurve::a2dPieCurve( a2dVertexArray* points ): a2dCurve()
{
    m_points = points;
    m_rendermode = a2dCURVES_RENDER_DEFAULT;
}

a2dPieCurve::a2dPieCurve( ): a2dCurve()
{
    m_points = new a2dVertexArray;
    m_rendermode = a2dCURVES_RENDER_DEFAULT;
}


a2dPieCurve::~a2dPieCurve()
{
    delete m_points;
}

a2dPieCurve::a2dPieCurve( const a2dPieCurve& other, CloneOptions options, a2dRefMap* refs )
    : a2dCurve( other, options, refs )
{
    m_points = new a2dVertexArray;
    unsigned int i;
    for ( i = 0; i < other.m_points->size(); i++ )
        m_points->push_back( other.m_points->Item( i )->Clone() );


    m_rendermode = other.m_rendermode;
}

a2dObject* a2dPieCurve::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dPieCurve( *this, options, refs );
};

void a2dPieCurve::RemovePointWorld( double& x, double& y , int index, bool transformed )
{
    if ( index == -1 )
        index = m_points->size();

    x = m_points->Item( index )->m_x;
    y = m_points->Item( index )->m_y;
    m_points->RemoveAt( index );

    Curve2World( x, y, x, y );

    if ( transformed )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }

}

void a2dPieCurve::AddPoint( const a2dPoint2D& P, int index )
{
    AddPoint( P.m_x, P.m_y, index );
}

void a2dPieCurve::AddPoint( double x, double y, int index )
{
    a2dLineSegment* point = new a2dLineSegment( x, y );

    if ( index == -1 )
        m_points->push_back( point );
    else
        m_points->Insert( point, index );

    SetPending( true );
}

void a2dPieCurve::GetPosXYPointWorld( int index, double& x, double& y, bool transform )
{
    if ( index == -1 )
        index = m_points->size();

    x = m_points->Item( index )->m_x;
    y = m_points->Item( index )->m_y;
    Curve2World( x, y, x, y );

    if ( transform )
    {
        m_lworld.TransformPoint( x, y, x, y );
    }
}

bool a2dPieCurve::GetXyAtSweep( double sweepValue, a2dPoint2D& point ) const
{
    double xp, yp;
    if( !m_points->size() )
        return false;

    xp = m_points->Item( 0 )->m_x;
    yp = m_points->Item( 0 )->m_y;

    if ( sweepValue <= m_points->Item( 0 )->m_x )
    {
        point.m_x = m_points->Item( 0 )->m_x;
        point.m_y = m_points->Item( 0 )->m_y;
        return false;
    }

    unsigned int i;
    for ( i = 0; i < m_points->size(); i++ )
    {
        if ( xp < sweepValue && m_points->Item( i )->m_x >= sweepValue )
        {
            point.m_x = sweepValue;
            point.m_y = ( m_points->Item( i )->m_y - yp ) * ( sweepValue - xp ) / ( m_points->Item( i )->m_x - xp ) + yp;
            return true;
        }
        xp = m_points->Item( i )->m_x;
        yp = m_points->Item( i )->m_y;
    }

    //return the end, so at least a normal value
    point.m_x = m_points->Item( i - 1 )->m_x;
    point.m_y = m_points->Item( i - 1 )->m_y;

    return false;
}

a2dBoundingBox a2dPieCurve::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox box = m_curvesArea->GetGroup()->GetPlotAreaBbox();
    m_curveBox = box;
    return box;
}

bool a2dPieCurve::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() && m_points->size() )
    {
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        m_width = m_bbox.GetWidth() * 0.95;
        m_height = m_bbox.GetHeight() * 0.95;

        return true;
    }
    return false;
}

void a2dPieCurve::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    //double minx = GetBoundaryMinX();
    //double maxx = GetBoundaryMaxX();

    unsigned int i;
    double xw, yw;

    double yt = 0;
    for ( i = 0; i < m_points->size(); i++ )
        yt += m_points->Item( i )->m_y;

    double dangle = 2 * wxPI / yt;

    wxRasterOperationMode prevlogfunction = wxNO_OP;
    wxDC* aDC = NULL;
    if( m_logFunction != wxNO_OP )
    {
        aDC = ic.GetDrawer2D()->GetRenderDC();
        prevlogfunction = aDC->GetLogicalFunction();
        aDC->SetLogicalFunction( m_logFunction );
    }

    if ( m_rendermode & a2dCURVES_RENDER_NORMAL )
    {
        double theta = 0;
        double xn, yn;
        xn = m_width / 2 * cos ( theta );
        yn = m_height / 2 * sin ( theta );
        for ( i = 0; i < m_points->size(); i++ )
        {
            xw = xn; yw = yn;
            double theta2 = theta;
            theta += dangle * m_points->Item( i )->m_y;
            ic.GetDrawer2D()->DrawLine( 0, 0, xw, yw );
            theta2 += 0.02;
            xn = m_width / 2 * cos ( theta2 );
            yn = m_height / 2 * sin ( theta2 );
            while ( theta2 < theta )
            {
                ic.GetDrawer2D()->DrawLine( xw, yw, xn, yn );
                theta2 += 0.02;
                xw = xn; yw = yn;
                xn = m_width / 2 * cos ( theta2 );
                yn = m_height / 2 * sin ( theta2 );
            }
            xn = m_width / 2 * cos ( theta );
            yn = m_height / 2 * sin ( theta );
            ic.GetDrawer2D()->DrawLine( xw, yw, xn, yn );
        }
    }
    else
    {
        double theta = 0;
        double xn, yn;
        xn = m_width / 2 * cos ( theta );
        yn = m_height / 2 * sin ( theta );
        a2dlist< a2dFill >::iterator iter = m_pointsFill.begin();
        for ( i = 0; i < m_points->size(); i++ )
        {
            if ( m_pointsFill.size() )
                ic.GetDrawer2D()->SetDrawerFill( *iter );

            xw = xn; yw = yn;
            double theta2 = theta;
            theta += dangle * m_points->Item( i )->m_y;

            a2dVertexArray* cpoints = new a2dVertexArray;
            cpoints->push_back( new a2dLineSegment( 0, 0 ) );
            cpoints->push_back( new a2dLineSegment( xw, yw ) );

            theta2 += 0.02;
            xn = m_width / 2 * cos ( theta2 );
            yn = m_height / 2 * sin ( theta2 );
            while ( theta2 < theta )
            {
                cpoints->push_back( new a2dLineSegment( xn, yn ) );
                theta2 += 0.02;
                xw = xn; yw = yn;
                xn = m_width / 2 * cos ( theta2 );
                yn = m_height / 2 * sin ( theta2 );
            }
            xn = m_width / 2 * cos ( theta );
            yn = m_height / 2 * sin ( theta );
            cpoints->push_back( new a2dLineSegment( xn, yn ) );

            ic.GetDrawer2D()->DrawPolygon( cpoints, false, wxWINDING_RULE );
            delete cpoints;

            iter = m_pointsFill.GetNextAround( iter );
        }
    }
    if( aDC )
        aDC->SetLogicalFunction( prevlogfunction );
}

#if wxART2D_USE_CVGIO
void a2dPieCurve::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCurve::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "points" ) , m_points->size() );
        out.WriteAttribute( wxT( "rendermode" ) , m_rendermode );
    }
    else
    {
        unsigned int i;
        for ( i = 0; i < m_points->size();  i++ )
        {
            if ( i % 6 == 0 )
                out.WriteNewLine();
            out.WriteStartElementAttributes( wxT( "xy" ) );
            out.WriteAttribute( wxT( "x" ) , m_points->Item( i )->m_x );
            out.WriteAttribute( wxT( "y" ) , m_points->Item( i )->m_y );
            out.WriteEndAttributes( true );
        }
    }
}

void a2dPieCurve::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCurve::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_rendermode =  parser.GetAttributeValueInt( wxT( "rendermode" ) );
    }
    else
    {
        while( parser.GetTagName() == wxT( "xy" )  )
        {
            double x = parser.GetAttributeValueDouble( wxT( "x" ) );
            double y = parser.GetAttributeValueDouble( wxT( "y" ) );
            a2dLineSegment* point = new a2dLineSegment( x, y );
            m_points->push_back( point );

            parser.Next();
            parser.Require( END_TAG, wxT( "xy" ) );
            parser.Next();
        }
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dPieCurve::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double xh = hitEvent.m_relx, yh = hitEvent.m_rely;

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

    double yt = 0;
    size_t i;
    for ( i = 0; i < m_points->size(); i++ )
        yt += m_points->Item( i )->m_y;

    double dangle = 2 * wxPI / yt;

    double a = m_width / 2 + pw + margin ;
    double b = m_height / 2 + pw + margin;
    double c = wxSqr( xh / a ) + wxSqr( yh / b );
    if ( 1 > c )
    {
        double dx = xh;
        double dy = yh;
        double theta = 0;
        size_t i;
        for ( i = 0; i < m_points->size(); i++ )
        {

            double start, end, angle;
            start = theta;
            theta += dangle * m_points->Item( i )->m_y;
            end   = theta;

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
                //wxLogDebug( "pie %d", i );
                hitEvent.m_how = a2dHit::stock_fill;
                return true;
            }
        }
    }

    hitEvent.m_how = a2dHit::stock_nohit;
    return false;
}

bool a2dPieCurve::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        if ( editmode == 1 )
        {
            m_flags.m_draggable = false;

            m_flags.m_visiblechilds = true;
            m_flags.m_childrenOnSameLayer = true;

            //add properties and handles as children in order to edit the object.
            size_t i;
            double yt = 0;
            for ( i = 0; i < m_points->size(); i++ )
                yt += m_points->Item( i )->m_y;

            double dangle = 2 * wxPI / yt;
            double theta = 0;
            double xn, yn;
            for ( i = 0; i < m_points->size(); i++ )
            {
                xn = m_width / 2 * cos ( theta );
                yn = m_height / 2 * sin ( theta );
                theta += dangle * m_points->Item( i )->m_y;

                //not inclusive matrix so relative to polygon
                a2dHandle* handle = new a2dHandle( this, xn, yn, wxT( "__index__" ) );
                handle->SetLayer( m_layer );
                handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                PROPID_Index->SetPropertyToObject( handle, i );
                Append( handle );
            }
            m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dHandle" ) );

            //calculate bbox's else mouse events may take place when first, and idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );
            //still set it pending to do the redraw ( in place )
            SetPending( true );
            return true;
        }
        else
        {
            PROPID_Allowrotation->SetPropertyToObject( this, true );
            PROPID_Allowskew->SetPropertyToObject( this, true );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

void a2dPieCurve::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        a2dAffineMatrix atWorld = ic->GetTransform();

        //we need to get the matrix until the parent of the a2dHandle,
        // and not inclusif the handle itself.
        //and therefore apply inverted handle matrix.
        a2dAffineMatrix inverselocal = m_lworld;
        inverselocal.Invert();

        a2dAffineMatrix inverse = ic->GetInverseParentTransform();

        double xinternal, yinternal;
        inverse.TransformPoint( xw, yw, xinternal, yinternal );
        //now in relative world coordinates, and so one more to get to curve coordinates.
        World2Curve( xinternal, yinternal, xinternal, yinternal );

        a2dHandle* draghandle = event.GetCanvasHandle();
        //wxUint16 index = PROPID_Index->GetPropertyValue( draghandle );

        wxUint16 editmode = PROPID_Editmode->GetPropertyValue( this );

        if ( editmode == 1 )
        {
            //a2dPieCurve* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dPieCurve );

            double xwi;
            double ywi;
            Curve2World( xinternal, yinternal, xwi, ywi );

            if ( event.GetMouseEvent().LeftUp() )
            {
                if ( draghandle->GetName() == wxT( "__index__" ) )
                {
                    //m_root->GetCommandProcessor()->Submit(new a2dCommand_VertexCurveMovePoint( original, xinternal, yinternal, index ));
                }
                else
                    event.Skip(); //maybe the base class handles
            }
            else if ( event.GetMouseEvent().Dragging() )
            {
                //SetPosXYPointWorld( index,  xinternal, yinternal );
                draghandle->SetPosXY( xwi, ywi );
            }
            else
                event.Skip(); //maybe on the base class handles
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

void a2dPieCurve::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    event.Skip();
}

void a2dPieCurve::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
    // PushLayer(wxLAYER_ONTOP);
    m_highlightmode = 1;
    SetPending( true );
    event.Skip();
}

void a2dPieCurve::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    //a2dIterC* ic = event.GetIterC();

    event.Skip();
    // PopLayer(wxLAYER_ONTOP);
    m_highlightmode = 0;
    SetPending( true );
}




