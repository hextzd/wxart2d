/*! \file canvas/src/booloper.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: booloper.cpp,v 1.39 2009/04/23 19:35:23 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "a2dprivate.h"

#if wxART2D_USE_KBOOL

#include "wx/canvas/canmod.h"
#include "wx/canvas/booloper.h"

const long a2dBooleanWalkerHandler::Boolean_NON = wxGenNewId();
const long a2dBooleanWalkerHandler::Boolean_OR = wxGenNewId();
const long a2dBooleanWalkerHandler::Boolean_AND = wxGenNewId();
const long a2dBooleanWalkerHandler::Boolean_EXOR = wxGenNewId();
const long a2dBooleanWalkerHandler::Boolean_A_SUB_B = wxGenNewId();
const long a2dBooleanWalkerHandler::Boolean_B_SUB_A = wxGenNewId();
const long a2dBooleanWalkerHandler::Boolean_CORRECTION = wxGenNewId();
const long a2dBooleanWalkerHandler::Boolean_SMOOTHEN = wxGenNewId();
const long a2dBooleanWalkerHandler::Boolean_MAKERING = wxGenNewId();
const long a2dBooleanWalkerHandler::Boolean_Polygon2Surface = wxGenNewId();
const long a2dBooleanWalkerHandler::Boolean_Surface2Polygon = wxGenNewId();

//IMPLEMENT_CLASS(a2dBooleanWalkerHandler, a2dWalkerIOHandler)

a2dBooleanWalkerHandler::a2dBooleanWalkerHandler( a2dCanvasObjectFlagsMask mask )
    : a2dWalker_LayerCanvasObjects( mask )
{
    Initialize();
    m_mask = mask;
    m_AberArcToPoly = 0;
}

a2dBooleanWalkerHandler::~a2dBooleanWalkerHandler()
{
    delete m_booleng;
}

void a2dBooleanWalkerHandler::Initialize()
{
    m_useCheck = true;
    m_depthFirst = true;
    m_clearTarget = true;
    m_resultOnSameLayer = false;
    m_releaseOrignals = false;
    m_firstLevelChild = false;

    m_resultStroke = *a2dNullSTROKE;
    m_resultFill = *a2dNullFILL;
    m_useFirstChildStyle = false;//true;

    m_booleng = new kbBool_Engine();

    // set some global values to arm the boolean engine
    double DGRID = 0.01;  // round coordinate X or Y value in calculations to this
    double MARGE = 0.1;   // snap with in this range points to lines in the intersection routines
    // should always be > DGRID  a  MARGE >= 10*DGRID is oke
    // this is also used to remove small segments and to decide when
    // two segments are in line.
    double CORRECTIONFACTOR = 500.0;  // correct the polygons by this number
    double CORRECTIONABER   = 1.0;    // the accuracy for the rounded shapes used in correction
    double ROUNDFACTOR      = 1.5;    // when will we round the correction shape to a circle
    double SMOOTHABER       = 0.001;   // accuracy when smoothing a polygon
    double MAXLINEMERGE     = 1000.0; // leave as is, segments of this length in smoothen

    if ( MARGE <= DGRID )
    {
        MARGE = DGRID * 10;
    }

    // DGRID is only meant to make fractional parts of input data which
    // are doubles, part of the integers used in vertexes within the boolean algorithm.
    // And therefore bigger than 1 is not usefull.
    // Within the algorithm all input data is multiplied with DGRID
    if ( DGRID < 1 )
        DGRID = 1 / DGRID;
    else
        DGRID = 1;

    // space for extra intersection inside the boolean algorithms
    // only change this if there are problems
    int GRID = 100;

    m_booleng->SetMarge( MARGE );
    m_booleng->SetGrid( GRID );
    m_booleng->SetDGrid( DGRID );

    // each 64 bit integer coordinate is calculated from the document it dataunits by:
    //B_INT intXorY = ( ( B_INT ) ( XorY * m_DGRID ) ) * m_GRID;
    // all the rest of the factor down here, are in the same unit as the document.

    m_booleng->SetCorrectionFactor( CORRECTIONFACTOR );
    m_booleng->SetCorrectionAber( CORRECTIONABER );
    m_booleng->SetSmoothAber( SMOOTHABER );
    m_booleng->SetMaxlinemerge( MAXLINEMERGE );
    m_booleng->SetRoundfactor( ROUNDFACTOR );
    m_booleng->SetOrientationEntryMode( false );
}

bool a2dBooleanWalkerHandler::Start( a2dCanvasObject* object )
{
    m_lastdepth = 0;

    a2dWalker_SetCheck setp( false );
    setp.Start( object );
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN | a2dCanvasOFlags::BIN2 );
    setflags.SetSkipNotRenderedInDrawing( true );
    setflags.Start( object, false );

    try
    {
        object->Walker( NULL, *this );
    }
    catch ( kbBool_Engine_Error& error )
    {
        wxString str( error.GetErrorMessage().data(), wxConvUTF8, error.GetErrorMessage().size() );
        a2dGeneralGlobals->ReportErrorF( a2dError_CommandError, str );
    }
    catch ( ... )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_CommandError, _( "Problem performing boolean operation" ) );
        return false;
    }
    return true;
}

bool a2dBooleanWalkerHandler::WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event )
{

    if ( m_currentDepth > m_depth || event == a2dWalker_a2dPropertyStart )
        SetStopWalking( true );

    if ( event == a2dWalker_a2dObjectStart )
    {
        //wxLogDebug(_T("a2dobj start %s"), wxDynamicCast( object, a2dObject )->GetName() );

        a2dCanvasObject* obj =  wxDynamicCast( object, a2dCanvasObject );
        if ( obj && m_currentDepth == 2 && ObjectOke( obj ) && !obj->GetCheck() )
        {
            if ( !m_selectedOnlyA || ( m_selectedOnlyA && obj->GetSelected() ) )
                m_firstLevelChild = true;
            if ( !m_selectedOnlyB || ( m_selectedOnlyB && obj->GetSelected() ) )
                m_firstLevelChild = true;
            obj->SetCheck( true );
        }
    }

    // every canvas object is tested at the start of its walk, if it is  oke, it will be stored in m_foundA or m_foundB
    // Next when all canvas objects at that level are checked, we go one level up where m_foundA/B will be processed.
    if ( event == a2dWalker_a2dCanvasObjectStart )
    {
        m_lastdepth = m_currentDepth;

        a2dCanvasObject* canobj = ( a2dCanvasObject* ) object;

        //wxLogDebug( wxT( "a2dWalker_a2dCanvasObjectStart classname = %s depth = %d" ), canobj->GetClassInfo()->GetClassName(), m_currentDepth );
        //wxLogDebug( wxT( "name = %s" ), canobj->GetName().c_str() );
        if ( canobj && parent && !canobj->GetBin() && !canobj->GetIgnoreLayer() && m_firstLevelChild )
        {
            // check if obj is in one of the group arrays (A or B)
            bool hasLayer = false;
            forEachIn( a2dLayerGroup, &m_groupA )
            {
                wxUint16 layer = *iter;
                // if one of the layers in the group is wxLAYER_ALL, that means all available layers in document
                if ( wxLAYER_ALL == layer )
                {
                    hasLayer = true;
                    break;
                }
                if ( canobj->GetLayer() == layer )
                    hasLayer = true;
            }

            // if in group A, we want this object for the boolean operation
            if ( hasLayer )
                m_foundA.push_back( canobj );

            {
                hasLayer = false;
                forEachIn( a2dLayerGroup, &m_groupB )
                {
                    wxUint16 layer = *iter;
                    // if one of the layers in the group is wxLAYER_ALL, that means all available layers in document
                    if ( wxLAYER_ALL == layer )
                    {
                        hasLayer = true;
                        break;
                    }
                    if ( canobj->GetLayer() == layer )
                        hasLayer = true;
                }

                // if in group B, we want this object for the boolean operation
                if ( hasLayer )
                    m_foundB.push_back( canobj );
            }
        }


        if ( ( !m_foundA.empty() || !m_foundB.empty() ) &&
                ( m_operation == Boolean_Polygon2Surface || m_operation == Boolean_Surface2Polygon ) )
        {
            a2dCanvasObject* canobjToAddTo = ( a2dCanvasObject* ) parent;
            CallEngine( canobjToAddTo );
        }
    }

    if ( !m_foundA.empty() || !m_foundB.empty() )
    {
        if ( event == a2dWalker_a2dCanvasObjectEnd && m_lastdepth > m_currentDepth )
        {
            a2dCanvasObject* canobj = ( a2dCanvasObject* ) object;
#if defined(_DEBUG)
            wxString name = canobj->GetName();
#endif
            // the m_foundA and m_foundB can now be processed.
            CallEngine( canobj );
            // the result is available, and even if target is the same layer as originals, we can delete
            // the objects which were used in the boolean operation now if they are on the target layer.
            if ( m_clearTarget )
            {
                forEachIn( a2dCanvasObjectList, canobj->GetChildObjectList() )
                {
                    a2dCanvasObject* obj = ( *iter );
                    if ( !obj->GetBin2() && obj->GetLayer() == m_target && !obj->GetIgnoreLayer() )
                        obj->SetRelease( true );
                }
            }
        }
    }
    else if ( m_clearTarget &&
              ( m_operation == Boolean_Polygon2Surface || m_operation == Boolean_Surface2Polygon ) )
    {
        if ( event == a2dWalker_a2dCanvasObjectEnd && m_lastdepth > m_currentDepth )
        {
            a2dCanvasObject* canobj = ( a2dCanvasObject* ) object;
            forEachIn( a2dCanvasObjectList, canobj->GetChildObjectList() )
            {
                a2dCanvasObject* obj = ( *iter );
                if ( !obj->GetBin2() && obj->GetLayer() == m_target && !obj->GetIgnoreLayer() )
                    obj->SetRelease( true );
            }
        }
    }

    if ( event == a2dWalker_a2dCanvasObjectEnd && m_lastdepth > m_currentDepth )
    {
        a2dCanvasObject* canobj = ( a2dCanvasObject* ) object;
        //wxLogDebug( wxT( "a2dWalker_a2dCanvasObjectEnd classname = %s depth = %d" ), canobj->GetClassInfo()->GetClassName(), m_currentDepth );
        //wxLogDebug( wxT( "name = %s" ), canobj->GetName().c_str() );
        m_lastdepth = m_currentDepth;
    }

    if ( event == a2dWalker_a2dObjectEnd )
    {
        //wxLogDebug(_T("a2dobj end %s"), wxDynamicCast( object, a2dObject )->GetName() );
        if ( m_currentDepth <= 2 )
        {
            m_firstLevelChild = false;
        }
    }

    if (  m_currentDepth <= m_depth && event == a2dWalker_a2dPropertyEnd )
        SetStopWalking( false );

    if ( event == a2dWalker_a2dCanvasObjectPostChild )
        return false; //do not go here

    return true;
}

bool a2dBooleanWalkerHandler::GetObjectsForSurface( a2dCanvasObject* booleanobject, GroupType A_or_B )
{
    a2dCanvasObjectList* vpath = wxNullCanvasObjectList;
    if ( 0 != wxDynamicCast( booleanobject, a2dRect ) && booleanobject->GetContourWidth() )
    {
        a2dRect* obj = wxDynamicCast( booleanobject, a2dRect );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dCircle ) && booleanobject->GetContourWidth() )
    {
        a2dCircle* obj = wxDynamicCast( booleanobject, a2dCircle );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dEllipse ) && booleanobject->GetContourWidth() )
    {
        a2dEllipse* obj = wxDynamicCast( booleanobject, a2dEllipse );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dEllipticArc ) && booleanobject->GetContourWidth() )
    {
        a2dEllipticArc* obj = wxDynamicCast( booleanobject, a2dEllipticArc );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dArc ) && booleanobject->GetContourWidth() )
    {
        a2dArc* obj = wxDynamicCast( booleanobject, a2dArc );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dEndsLine ) && booleanobject->GetContourWidth() )
    {
        a2dEndsLine* obj = wxDynamicCast( booleanobject, a2dEndsLine );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dSLine ) && booleanobject->GetContourWidth() )
    {
        a2dSLine* obj = wxDynamicCast( booleanobject, a2dSLine );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dPolylineL ) && booleanobject->GetContourWidth() )
    {
        a2dPolylineL* obj = wxDynamicCast( booleanobject, a2dPolylineL );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dSurface ) )
    {
        // we must intercept a2dSurface before a2dPolygonL from which it is derived.
        a2dSurface* obj = wxDynamicCast( booleanobject, a2dSurface );
        // nothing to do, is already a surface
        obj->SetBin( true );
        obj->SetBin2( true );
        obj->SetLayer( !m_resultOnSameLayer ? m_target : m_useFirstChildLayer );
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxDynamicCast( booleanobject, a2dPolygonL );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dVectorPath ) )
    {
        a2dVectorPath* obj = wxDynamicCast( booleanobject, a2dVectorPath );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dText ) )
    {
        a2dText* obj = wxDynamicCast( booleanobject, a2dText );
        vpath = obj->GetAsCanvasVpaths();
    }
    else
    {
        // rest does not have surface and go to result as is.
        booleanobject->SetBin( true );
        booleanobject->SetBin2( true );
        booleanobject->SetLayer( !m_resultOnSameLayer ? m_target : m_useFirstChildLayer );
    }
    // now we have vector path, which we will convert to polygons and polylines.
    // Next only the polygons will be used for boolean operations.
    if ( vpath != wxNullCanvasObjectList )
    {
        bool ret = VectorPathPolygonsToEngine( vpath, A_or_B );
        delete vpath;
        return ret;
    }
    return false;
}

bool a2dBooleanWalkerHandler::GetObjectsSurface( a2dCanvasObject* booleanobject, GroupType A_or_B )
{
    a2dCanvasObjectList* vpath = wxNullCanvasObjectList;
    if ( 0 != wxDynamicCast( booleanobject, a2dSurface ) )
    {
        // treat surfaces without conversion to vector paths
        a2dSmrtPtr<a2dSurface> poly = wxDynamicCast( booleanobject, a2dSurface );
        poly = wxDynamicCast( poly->Clone( a2dObject::clone_deep ), a2dSurface );

        if ( poly && poly->GetSegments()->HasArcs() )
            poly->GetSegments()->ConvertToLines( m_AberArcToPoly );

        double tx, ty;
        if ( poly && poly->GetNumberOfSegments() > 2 && m_booleng->StartPolygonAdd( A_or_B ) )
        {
            for( a2dVertexList::const_iterator iter = poly->GetSegments()->begin(); iter != poly->GetSegments()->end(); ++iter )
            {
                a2dLineSegment* seg = *iter;
                poly->GetTransformMatrix().TransformPoint( seg->m_x, seg->m_y, tx, ty );
                m_booleng->AddPoint( tx, ty, seg->GetArcPiece() );
            }
            m_booleng->EndPolygonAdd( poly->GetSegments()->front()->GetArcPiece() );
        }
        a2dListOfa2dVertexList& holes = poly->GetHoles();
        for( a2dListOfa2dVertexList::iterator iterp = holes.begin(); iterp != holes.end(); iterp++ )
        {
            a2dVertexListPtr vlist = ( *iterp );
            if ( vlist->HasArcs() )
                vlist->ConvertToLines( m_AberArcToPoly );
            if ( vlist->size() > 2 && m_booleng->StartHolePolygonAdd( A_or_B ) )
            {
                for( a2dVertexList::iterator iter = vlist->begin(); iter != vlist->end(); ++iter )
                {
                    a2dLineSegment* seg = *iter;
                    poly->GetTransformMatrix().TransformPoint( seg->m_x, seg->m_y, tx, ty );
                    m_booleng->AddPoint( tx, ty, seg->GetArcPiece() );
                }
            }
            m_booleng->EndPolygonAdd( vlist->front()->GetArcPiece() );
        }
        return true;
    }
    return false;
}

bool a2dBooleanWalkerHandler::ConvertToPolygons( a2dCanvasObject* booleanobject, GroupType A_or_B )
{
    //wxLogDebug( wxT( "classname = %s" ), booleanobject->GetClassInfo()->GetClassName() );

    a2dCanvasObjectList* vpath = wxNullCanvasObjectList;

    if ( 0 != wxDynamicCast( booleanobject, a2dRect ) )
    {
        a2dRect* obj = wxDynamicCast( booleanobject, a2dRect );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dRectC ) )
    {
        a2dRectC* obj = wxDynamicCast( booleanobject, a2dRectC );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dArrow ) )
    {
        a2dArrow* obj = wxDynamicCast( booleanobject, a2dArrow );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dCircle ) )
    {
        a2dCircle* obj = wxDynamicCast( booleanobject, a2dCircle );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dEllipse ) )
    {
        a2dEllipse* obj = wxDynamicCast( booleanobject, a2dEllipse );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dEllipticArc ) )
    {
        a2dEllipticArc* obj = wxDynamicCast( booleanobject, a2dEllipticArc );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dArc ) )
    {
        a2dArc* obj = wxDynamicCast( booleanobject, a2dArc );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dEndsLine ) && booleanobject->GetContourWidth() )
    {
        a2dEndsLine* obj = wxDynamicCast( booleanobject, a2dEndsLine );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dSLine ) && booleanobject->GetContourWidth() )
    {
        a2dSLine* obj = wxDynamicCast( booleanobject, a2dSLine );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dPolylineL ) && booleanobject->GetContourWidth() )
    {
        a2dPolylineL* obj = wxDynamicCast( booleanobject, a2dPolylineL );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dSurface ) )
    {
        // treat surfaces without conversion to vector paths
        a2dSmrtPtr<a2dSurface> poly = wxDynamicCast( booleanobject, a2dSurface );
        poly = wxDynamicCast( poly->Clone( a2dObject::clone_deep ), a2dSurface );

        if ( poly && poly->GetSegments()->HasArcs() )
            poly->GetSegments()->ConvertToLines( m_AberArcToPoly );

        double tx, ty;
        if ( poly && poly->GetNumberOfSegments() > 2 && m_booleng->StartPolygonAdd( A_or_B ) )
        {
            for( a2dVertexList::const_iterator iter = poly->GetSegments()->begin(); iter != poly->GetSegments()->end(); ++iter )
            {
                a2dLineSegment* seg = *iter;
                poly->GetTransformMatrix().TransformPoint( seg->m_x, seg->m_y, tx, ty );
                m_booleng->AddPoint( tx, ty, seg->GetArcPiece() );
            }
            m_booleng->EndPolygonAdd();
        }
        a2dListOfa2dVertexList& holes = poly->GetHoles();
        for( a2dListOfa2dVertexList::iterator iterp = holes.begin(); iterp != holes.end(); iterp++ )
        {
            a2dVertexListPtr vlist = ( *iterp );
            if ( vlist->HasArcs() )
                vlist->ConvertToLines( m_AberArcToPoly );
            if ( vlist->size() > 2 && m_booleng->StartHolePolygonAdd( A_or_B ) )
            {
                for( a2dVertexList::iterator iter = vlist->begin(); iter != vlist->end(); ++iter )
                {
                    a2dLineSegment* seg = *iter;
                    poly->GetTransformMatrix().TransformPoint( seg->m_x, seg->m_y, tx, ty );
                    m_booleng->AddPoint( tx, ty, seg->GetArcPiece() );
                }
            }
            m_booleng->EndPolygonAdd();
        }
        return true;
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxDynamicCast( booleanobject, a2dPolygonL );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dVectorPath ) )
    {
        a2dVectorPath* obj = wxDynamicCast( booleanobject, a2dVectorPath );
        vpath = obj->GetAsCanvasVpaths();
    }
    else if ( 0 != wxDynamicCast( booleanobject, a2dText ) )
    {
        a2dText* obj = wxDynamicCast( booleanobject, a2dText );
        vpath = obj->GetAsCanvasVpaths();
    }

    // now we have vector path, which we will convert to polygons and polylines.
    // Next only the polygons will be used for boolean operations.
    if ( vpath != wxNullCanvasObjectList )
    {
        bool ret = VectorPathPolygonsToEngine( vpath, A_or_B );
        delete vpath;
        return ret;
    }
    return false;
}

bool a2dBooleanWalkerHandler::VectorPathPolygonsToEngine( a2dCanvasObjectList* vpath, GroupType A_or_B )
{
    if ( vpath != wxNullCanvasObjectList )
    {
        a2dCanvasObjectList::iterator iter = vpath->begin();
        while ( iter != vpath->end() )
        {
            a2dCanvasObjectList* polylist = NULL;
            a2dVectorPath* obj = ( a2dVectorPath* ) ( *iter ).Get();
            polylist = obj->GetAsPolygons();

            for( a2dCanvasObjectList::iterator iterp = polylist->begin(); iterp != polylist->end(); ++iterp )
            {
                a2dSmrtPtr<a2dPolylineL> polyl = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                if ( polyl )
                {
                    bool returnIsPolygon;
                    a2dSmrtPtr<a2dPolygonL> poly = new a2dPolygonL( polyl->GetAsVertexList( returnIsPolygon ) );
                    if ( poly && poly->GetSegments()->HasArcs() )
                        poly->GetSegments()->ConvertToLines( m_AberArcToPoly );

                    if ( poly )
                    {
                        if ( poly->GetNumberOfSegments() > 2 && m_booleng->StartPolygonAdd( A_or_B ) )
                        {
                            for( a2dVertexList::const_iterator iter = poly->GetSegments()->begin(); iter != poly->GetSegments()->end(); ++iter )
                            {
                                a2dLineSegment* seg = *iter;
                                m_booleng->AddPoint( seg->m_x, seg->m_y, seg->GetArcPiece() );
                            }
                            m_booleng->EndPolygonAdd( poly->GetSegments()->front()->GetArcPiece() );
                        }
                    }
                }
                else
                {
                    a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                    if ( poly && poly->GetSegments()->HasArcs() )
                        poly->GetSegments()->ConvertToLines( m_AberArcToPoly );

                    if ( poly && poly->GetNumberOfSegments() > 2 && m_booleng->StartPolygonAdd( A_or_B ) )
                    {
                        for( a2dVertexList::const_iterator iter = poly->GetSegments()->begin(); iter != poly->GetSegments()->end(); ++iter )
                        {
                            a2dLineSegment* seg = *iter;
                            m_booleng->AddPoint( seg->m_x, seg->m_y, seg->GetArcPiece() );
                        }
                        m_booleng->EndPolygonAdd( poly->GetSegments()->front()->GetArcPiece() );
                    }
                }
            }
            delete polylist;

            iter = vpath->erase( iter );
            iter = vpath->begin();
        }
        return true;
    }
    return false;
}

bool a2dBooleanWalkerHandler::CallEngine( a2dCanvasObject* canobjToAddChildren )
{
    bool foundfirst = false;
    if ( !m_foundA.empty() || !m_foundB.empty() )
    {
        //add all objects which are convertable to polygons, to the boolean engine.
        forEachIn( a2dCanvasObjectList, &m_foundA )
        {
            a2dCanvasObject* obj = ( *iter );
            bool foundObjects = false;
            if ( m_operation == Boolean_Polygon2Surface )
                foundObjects = GetObjectsForSurface( obj, GROUP_A );
            else if ( m_operation == Boolean_Surface2Polygon )
                foundObjects = GetObjectsSurface( obj, GROUP_A );
            else
                foundObjects = ConvertToPolygons( obj, GROUP_A );
            if ( foundObjects )
            {
                if ( !foundfirst )
                {
                    m_useFirstChildStyleStroke = obj->GetStroke();
                    m_useFirstChildStyleFill = obj->GetFill();
                    m_useFirstChildLayer = obj->GetLayer();
                    foundfirst = true;
                }

                // if this object has itself children too, preserve those childs by creating
                // a new object at the end containing those childs.
                wxString classname = obj->GetClassInfo()->GetClassName();
                if ( classname != wxT( "a2dCanvasObject" ) && obj->GetChildObjectsCount() )
                {
                    a2dCanvasObject* holdChildRes = new a2dCanvasObject();
                    holdChildRes->SetTransformMatrix( obj->GetTransformMatrix() );
                    holdChildRes->CreateChildObjectList()->TakeOverFrom( obj->GetChildObjectList() );
                    canobjToAddChildren->Prepend( holdChildRes );
                }

                if ( m_releaseOrignals )
                    //now the old object can be released.
                    obj->SetRelease( true );
                //canobjToAddChildren->ReleaseChild( obj, false, false, false );
            }
        }

        {
            //scope
            forEachIn( a2dCanvasObjectList, &m_foundB )
            {
                a2dCanvasObject* obj = ( *iter );
                bool foundObjects = false;
                if ( m_operation != Boolean_Polygon2Surface && m_operation != Boolean_Surface2Polygon )
                    foundObjects = ConvertToPolygons( obj, GROUP_B );
                if ( foundObjects )
                {
                    if ( !foundfirst )
                    {
                        m_useFirstChildStyleStroke = obj->GetStroke();
                        m_useFirstChildStyleFill = obj->GetFill();
                        m_useFirstChildLayer = obj->GetLayer();
                        foundfirst = true;
                    }

                    // if this object has itself children too, preserve those childs by creating
                    // a new object at the end containing those childs.
                    wxString classname = obj->GetClassInfo()->GetClassName();
                    if ( classname != wxT( "a2dCanvasObject" ) && obj->GetChildObjectsCount() )
                    {
                        a2dCanvasObject* holdChildRes = new a2dCanvasObject();
                        holdChildRes->SetTransformMatrix( obj->GetTransformMatrix() );
                        holdChildRes->CreateChildObjectList()->TakeOverFrom( obj->GetChildObjectList() );
                        canobjToAddChildren->Prepend( holdChildRes );
                    }

                    if ( m_releaseOrignals )
                        //now the old object can be released.
                        obj->SetRelease( true );
                    //canobjToAddChildren->ReleaseChild( obj, false, false, false );
                }
            }
        }//endscope



        BOOL_OP boolOp = BOOL_NON;
        if ( m_operation == Boolean_NON )
        {
        }
        else if ( m_operation == Boolean_OR )
            boolOp = BOOL_OR;
        else if ( m_operation == Boolean_AND )
            boolOp = BOOL_AND;
        else if ( m_operation == Boolean_EXOR )
            boolOp = BOOL_EXOR;
        else if ( m_operation == Boolean_A_SUB_B )
            boolOp = BOOL_A_SUB_B;
        else if ( m_operation == Boolean_B_SUB_A )
            boolOp = BOOL_B_SUB_A;
        else if ( m_operation == Boolean_CORRECTION )
            boolOp = BOOL_CORRECTION;
        else if ( m_operation == Boolean_SMOOTHEN )
            boolOp = BOOL_SMOOTHEN;
        else if ( m_operation == Boolean_MAKERING )
            boolOp = BOOL_MAKERING;
        else if ( m_operation == Boolean_Polygon2Surface )
        {
            // this merges the polygons and gives non linked holes, leading too surfaces in the end.
            boolOp = BOOL_OR;
            m_booleng->SetLinkHoles( false );
        }
        else if ( m_operation == Boolean_Surface2Polygon )
            boolOp = BOOL_OR;

        m_booleng->Do_Operation( boolOp );

        // foreach resultant polygon in the booleng ...
        // When we want surfaces, that will only work if holes were not linked by engine.
        if ( m_booleng->GetLinkHoles() )
        {
            while ( m_booleng->StartPolygonGet() )
            {
                a2dVertexList* points = new a2dVertexList;
                // foreach point in the polygon
                while ( m_booleng->PolygonHasMorePoints() )
                {
                    a2dLineSegment* seg = new a2dLineSegment( m_booleng->GetPolygonXPoint(), m_booleng->GetPolygonYPoint() );
                    points->push_back( seg );
                    if ( m_booleng->GetHoleConnectionSegment() )
                        seg->SetSegType( a2dLINK_SEG );
                    else if ( m_booleng->GetHoleSegment() )
                        seg->SetSegType( a2dHOLE_SEG );
                    seg->SetArcPiece( m_booleng->GetArcPiece() );
                }
                m_booleng->EndPolygonGet();

                a2dPolygonL* polygon = new a2dPolygonL( points );
                // prepend, because object with children are appended at the end, and will still be processed
                canobjToAddChildren->Prepend( polygon );
                polygon->SetLayer( !m_resultOnSameLayer ? m_target : m_useFirstChildLayer );
                if ( !m_resultStroke.IsNoStroke()  )
                    polygon->SetStroke( m_resultStroke );
                else if ( m_useFirstChildStyle )
                    polygon->SetStroke( m_useFirstChildStyleStroke );

                if ( !m_resultFill.IsNoFill() )
                    polygon->SetFill( m_resultFill );
                else if ( m_useFirstChildStyle )
                    polygon->SetFill( m_useFirstChildStyleFill );
                polygon->SetBin( true );
                polygon->SetBin2( true );
            }
        }
        else
        {
            bool hole = false;
            bool normal = false;
            a2dSurface* surface = NULL;
            a2dVertexListPtr points;
            a2dVertexListPtr holepoints;
            while ( m_booleng->StartPolygonGet() )
            {
                hole = false;
                normal = false;
                // foreach point in the polygon
                while ( m_booleng->PolygonHasMorePoints() )
                {
                    if ( m_booleng->GetHoleConnectionSegment() )
                    {
                        wxFAIL_MSG( wxT( "this type should not be here" ) );
                    }
                    else if ( m_booleng->GetHoleSegment() )
                    {
                        if ( !hole )
                        {
                            hole = true;
                            holepoints = new a2dVertexList;
                        }
                        a2dLineSegment* seg = new a2dLineSegment( m_booleng->GetPolygonXPoint(), m_booleng->GetPolygonYPoint() );
                        seg->SetSegType( a2dHOLE_SEG );
                        seg->SetArcPiece( m_booleng->GetArcPiece() );
                        holepoints->push_back( seg );
                    }
                    else //normal
                    {
                        if ( !normal )
                        {
                            normal = true;
                            points = new a2dVertexList;
                        }
                        a2dLineSegment* seg = new a2dLineSegment( m_booleng->GetPolygonXPoint(), m_booleng->GetPolygonYPoint() );
                        seg->SetArcPiece( m_booleng->GetArcPiece() );
                        points->push_back( seg );
                    }
                }
                m_booleng->EndPolygonGet();
                if ( normal )
                {
                    surface = new a2dSurface( points );
                    // prepend, because object with children are appended at the end, and will still be processed
                    canobjToAddChildren->Prepend( surface );
                    surface->SetLayer( !m_resultOnSameLayer ? m_target : m_useFirstChildLayer );
                    if ( !m_resultStroke.IsNoStroke()  )
                        surface->SetStroke( m_resultStroke );
                    else if ( m_useFirstChildStyle )
                        surface->SetStroke( m_useFirstChildStyleStroke );

                    if ( !m_resultFill.IsNoFill() )
                        surface->SetFill( m_resultFill );
                    else if ( m_useFirstChildStyle )
                        surface->SetFill( m_useFirstChildStyleFill );
                    surface->SetBin( true );
                    surface->SetBin2( true );
                }
                if ( hole )
                {
                    wxASSERT_MSG( surface, wxT( "need surface first to place holes within" ) );
                    surface->AddHole( holepoints );
                }
            }
        }

        // not of use anymore.
        m_foundA.clear();
        m_foundB.clear();
        canobjToAddChildren->SetBin( true );
    }
    return true;
}
#endif //wxART2D_USE_KBOOL
