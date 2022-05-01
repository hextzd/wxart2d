/*! \file canvas/src/restrict.cpp
    \author Michael S�trop
    \date Created 10/27/2003
    Copyright: 2003-2004 (c) Michael S�trop


    Licence: wxWidgets Licence

    RCS-ID: $Id: restrict.cpp,v 1.39 2008/07/30 21:54:00 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <float.h>
#include "wx/artbase/afmatrix.h"
#include "wx/general/gen.h"
#include "wx/canvas/restrict.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/drawer.h"
#include "wx/artbase/drawer2d.h"

static inline double sqr( double val ) {return val * val;}

#if 0
//EW: remains of something, or to be used in the future?
static int CmprDbl( double* val1, double* val2 )
{
    if( *val1 < *val2 )
        return -1;
    else if( *val1 > *val2 )
        return 1;
    else
        return 0;
}
#endif

a2dPropertyIdUint32* a2dRestrictionEngine::PROPID_SnapSourceFeaturesMem;
a2dPropertyIdUint32* a2dRestrictionEngine::PROPID_SnapSourceFeatures;
a2dPropertyIdUint32* a2dRestrictionEngine::PROPID_SnapTargetFeatures;
a2dPropertyIdDouble* a2dRestrictionEngine::PROPID_RotationAngle;
a2dPropertyIdUint32* a2dRestrictionEngine::PROPID_RotationRationalNom;
a2dPropertyIdUint32* a2dRestrictionEngine::PROPID_RotationRationalDen;
a2dPropertyIdInt32* a2dRestrictionEngine::PROPID_SnapThresHold;
a2dPropertyIdPoint2D* a2dRestrictionEngine::PROPID_PointToSnap;
a2dPropertyIdPoint2D* a2dRestrictionEngine::PROPID_PointToRestrictTo;
a2dPropertyIdBool* a2dRestrictionEngine::PROPID_SnapOnlyVisbleObjects;
a2dPropertyIdBool* a2dRestrictionEngine::PROPID_Snap;
a2dPropertyIdBool* a2dRestrictionEngine::PROPID_SnapGetSet;
a2dPropertyIdBool* a2dRestrictionEngine::PROPID_AltDown;
a2dPropertyIdBool* a2dRestrictionEngine::PROPID_ShiftDown;

INITIALIZE_PROPERTIES( a2dRestrictionEngine, a2dObject )
{
    A2D_PROPID_GS( a2dPropertyIdBool, a2dRestrictionEngine, SnapGetSet, false, GetSnap, SetSnap );
    A2D_PROPID_GSI( a2dPropertyIdUint32, a2dRestrictionEngine, SnapSourceFeatures, 0 );
    A2D_PROPID_GSI( a2dPropertyIdUint32, a2dRestrictionEngine, SnapTargetFeatures, 0 );
    A2D_PROPID_GSI( a2dPropertyIdDouble, a2dRestrictionEngine, RotationAngle, 0 );
    A2D_PROPID_M( a2dPropertyIdUint32, a2dRestrictionEngine, RotationRationalNom, 0, m_rotationRationalNom );
    A2D_PROPID_M( a2dPropertyIdUint32, a2dRestrictionEngine, RotationRationalDen, 0, m_rotationRationalDen );
    A2D_PROPID_M( a2dPropertyIdInt32, a2dRestrictionEngine, SnapThresHold, 0, m_snapThresHold );
    A2D_PROPID_M( a2dPropertyIdPoint2D, a2dRestrictionEngine, PointToSnap, a2dPoint2D( 0, 0 ), m_pointToSnap );
    A2D_PROPID_M( a2dPropertyIdPoint2D, a2dRestrictionEngine, PointToRestrictTo, a2dPoint2D( 0, 0 ), m_pointToRestrictTo );
    A2D_PROPID_M( a2dPropertyIdBool, a2dRestrictionEngine, SnapOnlyVisbleObjects, false, m_snapOnlyVisbleObjects );
    A2D_PROPID_M( a2dPropertyIdBool, a2dRestrictionEngine, Snap, false, m_snap );
    A2D_PROPID_M( a2dPropertyIdBool, a2dRestrictionEngine, AltDown, false, m_releaseSnap );
    A2D_PROPID_M( a2dPropertyIdBool, a2dRestrictionEngine, ShiftDown, false, m_shiftDown );
    A2D_PROPID_M( a2dPropertyIdUint32, a2dRestrictionEngine, SnapSourceFeaturesMem, 0, m_snapSourceFeatures );
    return true;
}

const a2dSignal a2dRestrictionEngine::sig_changed = wxNewId();

BEGIN_EVENT_TABLE( a2dRestrictionEngine, a2dObject )
END_EVENT_TABLE()

void a2dRestrictionEngine::SignalChange()
{
    a2dComEvent changed( this, sig_changed );
	ProcessEvent( changed );
}

double a2dRestrictionEngine::GetSnapThresHoldWorld() const
{
    if ( a2dCanvasGlobals->GetActiveDrawingPart() )
    {
        return a2dCanvasGlobals->GetActiveDrawingPart()->GetDrawer2D()->DeviceToWorldXRel( m_snapThresHold );
    }
    return 0;
}

void a2dRestrictionEngine::SetInternal()
{
    m_docSnapDistX  = m_snapDistX;
    m_docSnapDistY  = m_snapDistY;
    m_docOriginX  = m_originX;
    m_docOriginY  = m_originY;
    if ( a2dCanvasGlobals->GetActiveDrawingPart() && a2dCanvasGlobals->GetActiveDrawingPart()->GetDrawing() )
    {
        a2dDrawing* drawing = a2dCanvasGlobals->GetActiveDrawingPart()->GetDrawing();
        m_docSnapDistX  /= drawing->GetUnitsScale();
        m_docSnapDistY  /= drawing->GetUnitsScale();
        m_docOriginX  /= drawing->GetUnitsScale();
        m_docOriginY  /= drawing->GetUnitsScale();
    }
}

a2dRestrictionEngine::a2dRestrictionEngine()
    : m_lineToSnap( 0, 0, 1, 1 )
{
    m_snapDistX = 1.0;
    m_snapDistY = 1.0;
    m_originX = 0.0;
    m_originY = 0.0;
    m_rotationAngle = 10;
    m_rotationThreshold = 1; //degrees
    m_rotationRationalNom = 0x0000003f; // set first 6 bits allow 0..5
    m_rotationRationalDen = 0x00000024; // set bits 3 and 6 allow n/2 and n/5
    m_angleList = 0;
    m_nAngleList = 0;

    m_snapThresHold = 6; //pixels

    m_snapSourceFeatures = snapToBoundingBox | snapToObjectPos | snapToPinsUnconnected  | snapToObjectVertexes;

    m_snapTargetFeatures = snapToGridPos | snapToObjectPos | snapToPinsUnconnected  | snapToObjectVertexes;

    //m_snapTargetFeatures = snapToGridPos | snapToObjectPos | snapToPinsUnconnected  | snapToObjectVertexes | snapToPointAngleMod;
    m_snapTargetFeatures = snapToBoundingBox | snapToObjectPos | snapToPinsUnconnected  | snapToObjectVertexes | snapToObjectSnapVPath;

    m_snap = true;
    m_releaseSnap = false;
    m_shiftDown = false;
    m_snapOnlyVisbleObjects = true;

    SetInternal();

    //PROPID_SnapGetSet->SetPropertyToObject( this, true );
    //bool val = PROPID_SnapGetSet->GetPropertyValue( this );

    //PROPID_SnapSourceFeaturesMem->SetPropertyToObject( this, snapToBoundingBox );
    //wxUint32 mask = PROPID_SnapSourceFeaturesMem.GetPropertyValue( this );
}

a2dRestrictionEngine::~a2dRestrictionEngine()
{
    if( m_angleList )
        delete [] m_angleList;
}

/* !!!! not used
static double mod1(double x)
{
    return x-floor(x);
}
*/

/* !!!! not used
static bool IsOddMultiple(double val, double grid)
{
    // If val is an odd multiple of grid, mod is 0
    double mod=mod1(val/(2*grid))-0.5;

    return fabs(mod)<1e-10*fabs(val);
}
*/

/*! If the distance between val and the closest grid point,
 *  as sepcified by grid and origin,
 *  is smaller than *best, *best is set to this distance
 */
static void SnapIfBetterPos( double val, double grid, double origin, double* best )
{
    assert( grid );
    double
    newVal = floor( ( val - origin ) / grid + 0.5 ) * grid + origin,
    dist = newVal - val;
    if( fabs( dist ) < fabs( *best ) )
    {
        *best = dist;
    }
}

/*! If the distance between oldval and newval
 *  is smaller than *best, *best is set to this distance
 */
static void SnapIfBetterSize( double oldVal, double newVal, double* best )
{
    double
    dist = newVal - oldVal;
    if( fabs( dist ) < fabs( *best ) )
    {
        *best = dist;
    }
}

/*! return the closest grid point of val
 */
static double SnapGrid( double val, double grid )
{
    assert( grid );
    return floor( ( val ) / grid + 0.5 ) * grid;
}

void a2dRestrictionEngine::SetPointSnapResult( const a2dPoint2D& p )
{
    m_snappedPoint = p;
}

bool a2dRestrictionEngine::IsWithInThresHold( const a2dPoint2D& p )
{
    double dx = p.m_x - m_pointToSnap.m_x;
    double dy = p.m_y - m_pointToSnap.m_y;

    double thres = GetSnapThresHoldWorld();

    return (  fabs( dx ) < thres &&  fabs( dy ) < thres );
}

bool a2dRestrictionEngine::RestrictPoint( double& x, double& y,  wxUint32 sourceRequired, bool ignoreEngine )
{
    SetInternal();
    a2dCanvasObject* parentObject = m_parentObject;
    if ( !m_parentObject )
        a2dCanvasGlobals->GetActiveDrawingPart()->GetShowObject();

    bool res = false;
    if ( !m_snap || m_releaseSnap )
        return res;

    if ( !ignoreEngine )
        sourceRequired = sourceRequired & m_snapTargetFeatures;

    m_pointToSnap = a2dPoint2D( x, y );
    //FAR AWAY
    SetPointSnapResult( a2dPoint2D( DBL_MAX, DBL_MAX ) );

    double thresWorld = GetSnapThresHoldWorld();

    // for object find the clossest snap vertex in a neighbour objects and snap to that
    if ( sourceRequired & snapToPins ||
         sourceRequired & snapToPinsUnconnected ||
         sourceRequired & snapToObjectPos ||
         sourceRequired & snapToObjectVertexes ||
         sourceRequired & snapToObjectSegmentsMiddle ||
         sourceRequired & snapToObjectSegments ||
         sourceRequired & snapToObjectIntersection
       )
    {
        //iterate over objects in document as seen from the showObject in a a2dDrawingPart,
        //and snap to vertexes returned from the snap vector path in those objects.
        a2dCanvasObjectList::iterator iter = parentObject->GetChildObjectList()->begin();
        while( iter != parentObject->GetChildObjectList()->end() )
        {
            a2dCanvasObjectList::value_type objchild = *iter;
            if ( !objchild->GetRelease() &&
                  objchild->GetSnapTo() &&
                  ( !m_snapOnlyVisbleObjects || objchild->GetVisible() ) //todo &&
                  //( !a2dCanvasGlobals->GetActiveDrawingPart()->GetLayerRenderArray()[ objchild->GetLayer() ].DoRenderLayer() ) 
			   )
            {
                if ( objchild->GetBbox().PointInBox( x, y, thresWorld ) )
                {
                    // m_pointToSnap is restricted ( e.g.  the clossest points is returned in m_snappedPoint )
                    a2dIterC ic( a2dCanvasGlobals->GetActiveDrawingPart() );
                    if ( objchild->RestrictToObject( ic, m_pointToSnap, m_snappedPoint, sourceRequired, thresWorld ) )
                    {
                        // RESULT IN m_snappedPoint
                        // the end result of the x,y snapped 
                        x = m_snappedPoint.m_x;
                        y = m_snappedPoint.m_y;
                        //wxLogDebug( "res %f %f", x, y ); 
                        res = true;
                    }
                }
            }
            iter++;
        }
        if ( res )
            return res;
    }


    if ( sourceRequired & snapToPointAngleMod && !m_shiftDown )
    {
        double rad2deg = 180 / wxPI;
        double dx = x - m_pointToRestrictTo.m_x;
        double dy = y - m_pointToRestrictTo.m_y;
        double radius = sqrt( dx * dx + dy * dy );

        double ang;
        if ( !dx && !dy )
            ang = 0;
        else
            ang = atan2( dy, dx );

        if ( AngleRestrictWithinThreshold( ang ) )
        {
            dx = cos( ang ) * radius;
            dy = sin( ang ) * radius;
            x = m_pointToRestrictTo.m_x + dx;
            y = m_pointToRestrictTo.m_y + dy;
            return true;
        }
    }

    if ( sourceRequired & snapToPointAngleModForced && !m_shiftDown )
    {
        double rad2deg = 180 / wxPI;
        double dx = x - m_pointToRestrictTo.m_x;
        double dy = y - m_pointToRestrictTo.m_y;
        double radius = sqrt( dx * dx + dy * dy );

        double ang;
        if ( !dx && !dy )
            ang = 0;
        else
            ang = atan2( dy, dx );

        ang = AngleRestrict( ang );
        dx = cos( ang ) * radius;
        dy = sin( ang ) * radius;
        x = m_pointToRestrictTo.m_x + dx;
        y = m_pointToRestrictTo.m_y + dy;
        return true;
    }

    if ( sourceRequired & snapToPointAngleRational && !m_shiftDown )
    {
        double rad2deg = 180 / wxPI;
        double dx = x - m_pointToRestrictTo.m_x;
        double dy = y - m_pointToRestrictTo.m_y;
        double radius = sqrt( dx * dx + dy * dy );

        double ang;
        if ( !dx && !dy )
            ang = 0;
        else
            ang = atan2( dy, dx );

        ang = RationalRestrict( ang );

        dx = cos( ang ) * radius;
        dy = sin( ang ) * radius;
        x = m_pointToRestrictTo.m_x + dx;
        y = m_pointToRestrictTo.m_y + dy;
        return true;
    }

    if ( sourceRequired & snapToPointPos ||
         sourceRequired & snapToPointPosX ||
         sourceRequired & snapToPointPosY ||
         sourceRequired & snapToPointPosXorY ||
         sourceRequired & snapToPointPosXorYForce ||
         m_shiftDown
       )
    {
        if( sourceRequired & snapToPointPos )
        {
            if ( fabs( x - m_pointToRestrictTo.m_x ) < thresWorld &&
                 fabs( y - m_pointToRestrictTo.m_y ) < thresWorld
               )
            {
                x = m_pointToRestrictTo.m_x;
                y = m_pointToRestrictTo.m_y;
                res = true;
                return res;
            }
        }
        if( sourceRequired & snapToPointPosX )
        {
            if ( fabs( x - m_pointToRestrictTo.m_x ) < thresWorld )
            {
                x = m_pointToRestrictTo.m_x;
                res = true;
                return res;
            }
        }
        if( sourceRequired & snapToPointPosY )
        {
            if ( fabs( y - m_pointToRestrictTo.m_y ) < thresWorld )
            {
                y = m_pointToRestrictTo.m_y;
                res = true;
                return res;
            }
        }
        if( sourceRequired & snapToPointPosXorYForce || m_shiftDown )
        {
            if ( fabs( x - m_pointToRestrictTo.m_x ) < fabs( y - m_pointToRestrictTo.m_y ) )
            {
                x = m_pointToRestrictTo.m_x;

                double sy;
                sy = y;

                // Restrict y-Position
                if( sourceRequired & snapToGridPosY && m_snapDistY )
                {
                    y = floor( ( y - m_docOriginY ) / m_docSnapDistY + 0.5 ) * m_docSnapDistY + m_docOriginY;
                }
            }
            else
            {
                y = m_pointToRestrictTo.m_y;

                double sx;
                sx = x;
                // Restrict x-Position
                if( sourceRequired & snapToGridPosX && m_snapDistX )
                {
                    x = floor( ( x - m_docOriginX ) / m_docSnapDistX + 0.5 ) * m_docSnapDistX + m_docOriginX;
                }
            }
            res = true;
            return res;
        }
    }

    // for m_pointToSnap find the clossest snap vertex in a neighbour object, and snap to that
    if ( sourceRequired & snapToObjectSnapVPath )
    {
        double dx = DBL_MAX;
        double dy = DBL_MAX;

        //iterate over object in document as seen from the showObject in a a2dDrawingPart,
        //and snap to vertexes returned from the snap vector path in those objects.
        a2dCanvasObjectList::iterator iter = parentObject->GetChildObjectList()->begin();
        while( iter != parentObject->GetChildObjectList()->end() )
        {
            a2dCanvasObjectList::value_type objchild = *iter;
            if ( !objchild->GetRelease() &&
                 objchild->GetSnapTo() &&
                 ( !m_snapOnlyVisbleObjects || objchild->GetVisible() ) //todo &&
                 //( !a2dCanvasGlobals->GetActiveDrawingPart()->GetLayerRenderArray()[ objchild->GetLayer() ].DoRenderLayer() )
			   )
            {
                if ( objchild->GetBbox().PointInBox( x, y, thresWorld ) )
                {
                    a2dCanvasObjectList* vectorpaths = objchild->GetSnapVpath( sourceRequired );
                    if ( vectorpaths != wxNullCanvasObjectList )
                    {
                        forEachIn( a2dCanvasObjectList, vectorpaths )
                        {
                            a2dVectorPath* obj = wxStaticCast( ( *iter ).Get(), a2dVectorPath );
                            unsigned int i;
                            for ( i = 0; i < obj->GetSegments()->size(); i++ )
                            {
                                a2dVpathSegment* seg = obj->GetSegments()->Item( i );
                                switch ( seg->GetType() )
                                {
                                    case a2dPATHSEG_MOVETO:
                                    case a2dPATHSEG_LINETO:
                                    case a2dPATHSEG_LINETO_NOSTROKE:
                                    case a2dPATHSEG_CBCURVETO_NOSTROKE:
                                    case a2dPATHSEG_CBCURVETO:
                                    case a2dPATHSEG_QBCURVETO_NOSTROKE:
                                    case a2dPATHSEG_QBCURVETO:
                                    case a2dPATHSEG_ARCTO_NOSTROKE:
                                    case a2dPATHSEG_ARCTO:
                                    {
                                        if ( SetPointIfCloser( m_pointToSnap, a2dPoint2D( seg->m_x1 ,seg->m_y1 ), m_snappedPoint, thresWorld ) )
                                        { 
                                            x = seg->m_x1;
                                            y = seg->m_y1;
                                            res = true;
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        delete vectorpaths;
                    }
                }
            }
            iter++;
        }
    }

    if ( sourceRequired & snapToGridPos || sourceRequired & snapToGridPosForced )
    {
        double sx, sy;
        sx = x;
        sy = y;
        // Restrict x-Position
        if( sourceRequired & snapToGridPosX && m_snapDistX )
        {
            sx = floor( ( x - m_docOriginX ) / m_docSnapDistX + 0.5 ) * m_docSnapDistX + m_docOriginX;
        }
        // Restrict y-Position
        if( sourceRequired & snapToGridPosY && m_snapDistY )
        {
            sy = floor( ( y - m_docOriginY ) / m_docSnapDistY + 0.5 ) * m_docSnapDistY + m_docOriginY;
        }

        // make sure that the point can always be snapped to a grid point in this mode
        if (sourceRequired & snapToGridPosForced )
            thresWorld = wxMax( m_docSnapDistX*0.51, m_docSnapDistY*0.51 );

        if ( SetPointIfCloser( a2dPoint2D( sx ,sy ), m_pointToSnap, m_snappedPoint, thresWorld ) )
        { 
            x = sx; y = sy; 
            res = true;
        }
    }
    return res;
}

bool a2dRestrictionEngine::RestrictLine( a2dLine& line,  wxUint32 sourceRequired, bool ignoreEngine )
{
    return false;
}

bool a2dRestrictionEngine::RestrictAngle( double* ang,  wxUint32 sourceRequired, bool ignoreEngine )
{
    if ( !ignoreEngine )
        sourceRequired = sourceRequired & m_snapTargetFeatures;

    if ( sourceRequired & snapToPointAngleMod )
    {
        *ang = AngleRestrict( *ang );
        return true;
    }

    if ( sourceRequired & snapToPointAngleRational )
    {
        *ang = RationalRestrict( *ang );
        return true;
    }
    return false;
}

static int CmprDbl( const void* val1, const void* val2 )
{
    if( *( double* )val1 < * ( double* )val2 )
        return -1;
    else if( *( double* )val1 > *( double* )val2 )
        return 1;
    else
        return 0;
}

bool a2dRestrictionEngine::AngleRestrictWithinThreshold( double& angle )
{
    double
    angleSnapRad = m_rotationAngle / 180 * wxPI;
    double snapedAngleRad = floor( angle / angleSnapRad + 0.5 ) * angleSnapRad;
    if ( fabs( snapedAngleRad - angle ) < m_rotationThreshold / 180 * wxPI )
    {
        angle = snapedAngleRad;
        return true;
    }
    return false;
}

double a2dRestrictionEngine::AngleRestrict( double angle )
{
    double
    angleSnapRad = m_rotationAngle / 180 * wxPI;
    return floor( angle / angleSnapRad + 0.5 ) * angleSnapRad;
}

void a2dRestrictionEngine::AngleRestrictVectorRot( double* vecx, double* vecy )
{
    double
    angle = atan2( *vecy, *vecx ),
    len = sqrt( sqr( *vecx ) + sqr( *vecy ) );
    angle = AngleRestrict( angle );
    *vecx = cos( angle ) * len;
    *vecy = sin( angle ) * len;
}

void a2dRestrictionEngine::AngleRestrictVectorSkew( double* vecx, double* vecy, double otherx, double othery )
{
    double
    angleOld   = atan2( *vecy, *vecx ),
    angleNew   = AngleRestrict( angleOld ),
    angleOther = atan2( othery, otherx ),
    proj       = *vecx * othery - *vecy * otherx;

    if( fabs( angleNew - angleOther ) < 1e-5 )
    {
        // If the new angle is very close to the other vectors angle
        // the transformation will become singular.
        // To avoid this, use the closest other snap angle
        double
        angleSnapRad = m_rotationAngle / 180 * wxPI,
        angleNew1 = angleNew - angleSnapRad,
        angleNew2 = angleNew + angleSnapRad;
        if( fabs( angleOld - angleNew1 ) < fabs( angleOld - angleNew2 ) )
            angleNew = angleNew1;
        else
            angleNew = angleNew2;
    }

    double
    newx = cos( angleNew ),
    newy = sin( angleNew ),
    newpro = newx * othery - newy * otherx;

    if( !newpro )
        return;

    *vecx = newx * ( proj / newpro );
    *vecy = newy * ( proj / newpro );
}

double a2dRestrictionEngine::RationalRestrict( double angle )
{
    // map angle to first quadrant 0..45 deg
    int
    quad = 0;
    bool
    mirr = false;

    if( angle < 0 )
        angle += 2 * wxPI;

    if( angle > wxPI )
    {
        quad += 2;
        angle -= wxPI;
    }
    if( angle > wxPI / 2 )
    {
        quad += 1;
        angle -= wxPI / 2;
    }
    if( angle > wxPI / 4 )
    {
        mirr = true;
        angle = wxPI / 2 - angle;
    }

    if( !m_angleList )
    {
        // create the list of valid angles
        const int
        mAngles = 31 * 30 / 2 + 1;
        int
        nAngles = 0;
        double
        angles[mAngles];

        angles[nAngles++] = 0;
        int i;
        for( i = 1; i <= 31; i++ )
        {
            if( m_rotationRationalNom & ( 1 << i ) )
            {
                for( int j = i; j <= 31; j++ )
                {
                    if( m_rotationRationalDen & ( j << i ) )
                    {
                        assert( nAngles < mAngles );
                        angles[nAngles++] = atan( double( i ) / double( j ) );
                    }
                }
            }
        }

        // sort the list
        qsort( angles, nAngles, sizeof( *angles ), CmprDbl );

        // remove multiple equal values
        m_nAngleList = 1;
        for( i = 1; i < nAngles; i++ )
        {
            if( angles[i] - angles[m_nAngleList - 1] > 1e-13 )
            {
                angles[m_nAngleList++] = angles[i];
            }
        }

        // copy the temporary list
        m_angleList = new double[m_nAngleList];
        memcpy( m_angleList, angles, sizeof( *m_angleList )*m_nAngleList );
    }

    // find nearest angle in list
    if( m_nAngleList <= 1 )
    {
        angle = 0;
    }
    else
    {
        int
        low = 0,
        high = m_nAngleList - 1,
        mid;

        while( high - low > 1 )
        {
            mid = ( low + high ) >> 1;
            if( m_angleList[mid] < angle )
                low = mid;
            else
                high = mid;
        }

        // angle is between m_angleList[low] and m_angleList[high]
        // One of these two values must be the closest value
        if( angle - m_angleList[low] < m_angleList[high] - angle )
            angle = m_angleList[low];
        else
            angle = m_angleList[high];
    }

    // put the angle back to its original quadrant
    if( mirr )
        angle = wxPI / 2 - angle;
    angle += quad * wxPI / 2;

    return angle;
}

void a2dRestrictionEngine::RationalRestrictVector( double* vecx, double* vecy )
{
    double
    angle = atan2( *vecy, *vecx ),
    len = sqrt( sqr( *vecx ) + sqr( *vecy ) );
    angle = RationalRestrict( angle );
    *vecx = cos( angle ) * len;
    *vecy = sin( angle ) * len;
}

bool a2dRestrictionEngine::RestrictCanvasObjectAtVertexes( a2dCanvasObject* object, a2dPoint2D& point, double& dx, double& dy, wxUint32 sourceRequired, bool ignoreEngine )
{
    bool res = false;
    dx = DBL_MAX;
    dy = DBL_MAX;

    if ( !m_snap || m_releaseSnap )
        return res;

    if ( !ignoreEngine )
        sourceRequired = sourceRequired & m_snapTargetFeatures;

    // object has enabled snap?
    if ( object->GetSnap() )
    {
        a2dCanvasObjectList* vectorpaths = object->GetSnapVpath( m_snapSourceFeatures );
        if ( vectorpaths != wxNullCanvasObjectList )
        {
            forEachIn( a2dCanvasObjectList, vectorpaths )
            {
                a2dVectorPath* obj = wxStaticCast( ( *iter ).Get(), a2dVectorPath );
                unsigned int i;
                for ( i = 0; i < obj->GetSegments()->size(); i++ )
                {
                    a2dVpathSegment* seg = obj->GetSegments()->Item( i );
                    switch ( seg->GetType() )
                    {
                        case a2dPATHSEG_MOVETO:
                        case a2dPATHSEG_LINETO:
                        case a2dPATHSEG_LINETO_NOSTROKE:
                        case a2dPATHSEG_CBCURVETO_NOSTROKE:
                        case a2dPATHSEG_CBCURVETO:
                        case a2dPATHSEG_QBCURVETO_NOSTROKE:
                        case a2dPATHSEG_QBCURVETO:
                        case a2dPATHSEG_ARCTO_NOSTROKE:
                        case a2dPATHSEG_ARCTO:
                        {
                            //point to snap
                            double x = seg->m_x1;
                            double y = seg->m_y1;
                            if ( RestrictPoint( x, y, sourceRequired, ignoreEngine ) )
                            {
                                //if closser, take it
                                if ( fabs( x - seg->m_x1 ) < fabs( dx ) && fabs( y - seg->m_y1 ) < fabs( dy ) )
                                {
                                    dx = x - seg->m_x1;
                                    dy = y - seg->m_y1;
                                    point = a2dPoint2D( seg->m_x1, seg->m_y1 );
                                    res = true;
                                }
                            }
                        }
                        break;
                    }
                }
            }
            delete vectorpaths;
        }
    }
    return res;
}

bool a2dRestrictionEngine::RestrictVertexes( a2dVertexArray* segments, a2dPoint2D& point, double& dx, double& dy, wxUint32 sourceRequired, bool ignoreEngine )
{
    dx = DBL_MAX;
    dy = DBL_MAX;

    bool res = false;
    int i = 0;
    for ( i = 0; i < segments->size(); i++ )
    {
        a2dLineSegmentPtr seg = segments->Item( i );
        double x = seg->m_x;
        double y = seg->m_y;
        if ( RestrictPoint( x, y, sourceRequired, ignoreEngine ) )
        {
            //if closser, take it
            if ( fabs( x - seg->m_x ) < dx && fabs( y - seg->m_y ) < dy )
            {
                dx = fabs( x - seg->m_x );
                dy = fabs( y - seg->m_y );
                point = a2dPoint2D( seg->m_x, seg->m_y );
                res = true;
            }
        }
    }
    return res;
}

bool a2dRestrictionEngine::RestrictVertexes( a2dVertexList* lsegments, a2dPoint2D& point, double& dx, double& dy, wxUint32 sourceRequired, bool ignoreEngine )
{
    dx = DBL_MAX;
    dy = DBL_MAX;
    bool res = false;

    int i = 0;
    for( a2dVertexList::iterator iter = lsegments->begin(); iter != lsegments->end(); ++iter, i++ )
    {
        a2dLineSegment* seg = *iter;
        double x = seg->m_x;
        double y = seg->m_y;
        if ( RestrictPoint( x, y, sourceRequired, ignoreEngine ) )
        {
            //if closser, take it
            if ( fabs( x - seg->m_x ) < dx && fabs( y - seg->m_y ) < dy )
            {
                dx = fabs( x - seg->m_x );
                dy = fabs( y - seg->m_y );
                point = a2dPoint2D( seg->m_x, seg->m_y );
                res = true;
            }
        }
    }
    return res;
}











a2dRestrictionEngineOld::a2dRestrictionEngineOld()
{
    m_posModesX = posNone;
    m_posModesY = posNone;
    m_sizeModesX = sizeNone;
    m_sizeModesY = sizeNone;
    m_rotModes = rotNone;
    m_snapDistX = 1;
    m_snapDistY = 1;
    m_originX = 0;
    m_originY = 0;
    m_sizeX = 1;
    m_sizeY = 1;
    m_rotationAngle = 2;
    m_rotationRationalNom = 0x0000003f; // allow 0..5
    m_rotationRationalDen = 0x00000024; // allow n/2 and n/5
    m_angleList = 0;
    m_nAngleList = 0;
    m_minSizeX = 1;
    m_minSizeY = 1;
    m_maxSizeX = DBL_MAX;
    m_maxSizeY = DBL_MAX;
}

a2dRestrictionEngineOld::~a2dRestrictionEngineOld()
{
    if( m_angleList )
        delete [] m_angleList;
}

//! Relative tolerance for detecting vertical and horizontal lines
static const double Eps = 1e-10;

void a2dRestrictionEngineOld::RestrictAffine( a2dAffineMatrix* mNew, const a2dAffineMatrix* mOld, ESnapWhat snapWhat, SnapObjectInfo* info, double* w, double* h )
{
    // mNew->DebugDump(wxT("PreRestrict"),w ? *w : 0, h ? *h : 0);

    // If this is called with an empty restriction engine, do nothing
    if( !this )
        return;

    double
    wfa  = w ? fabs( *w ) : 1,
    hfa  = h ? fabs( *h ) : 1,
    a00 = mNew->m_matrix[0][0],
    a01 = mNew->m_matrix[0][1],
    a10 = mNew->m_matrix[1][0],
    a11 = mNew->m_matrix[1][1],
    a20 = mNew->m_matrix[2][0],
    a21 = mNew->m_matrix[2][1];

    //------------------------------------------------------------------------
    // Calculate the fixpoints between old and new transformation
    //------------------------------------------------------------------------
    double
    fixX = 0.0,
    fixY = 0.0;
    bool
    hasFix = false,
    linearZero = false,
    shiftZero = false;
    if( mOld )
    {
        a2dAffineMatrix
        diff = *mNew - *mOld;
        double
        maxNew = mNew->GetMaximum(),
        maxOld = mOld->GetMaximum(),
        maxDiff = diff.GetMaximum(),
        detDiff = diff.GetDeterminant();

        linearZero = maxDiff <= wxMax( maxNew, maxOld ) * Eps ;

        shiftZero  =
            fabs( diff.m_matrix[2][0] ) <= wxMax( fabs( mNew->m_matrix[2][0] ), fabs( mOld->m_matrix[2][0] ) ) * Eps &&
            fabs( diff.m_matrix[2][1] ) <= wxMax( fabs( mNew->m_matrix[2][1] ), fabs( mOld->m_matrix[2][1] ) ) * Eps ;

        if( linearZero && shiftZero )
        {
            // both matrices are identical => nothing to do
            return;
        }
        else if( shiftZero )
        {
            // If the shifts are identical (differential shift 0) x=0, y=0 is a fixpoint
            // There may be more fixpoints, but this is a good choice
            hasFix = true;
        }
        else if( linearZero )
        {
            // diff is a zero matrix, but shift is non zero => no fixpoint
        }
        else if( fabs( detDiff ) < sqr( maxDiff ) * Eps )
        {
            // the matrix is irregular, but not zero
            // A fixpointline exists, if D1==0 and D2==0
            double
            det1 = diff.GetDeterminant1(),
            det2 = diff.GetDeterminant2(),
            max1 = diff.GetMaximum1(),
            max2 = diff.GetMaximum2();
            if( fabs( det1 ) <= max1 * Eps && fabs( det2 ) <= max2 * Eps )
            {
                // Both transformed coordinate axis vectors are parallel to the shift vector
                // => a fixpoint line exists
                hasFix = true;

                // select a pivot row in the matrix
                if( fabs( diff.m_matrix[0][0] ) + fabs( diff.m_matrix[1][0] ) > fabs( diff.m_matrix[0][1] ) + fabs( diff.m_matrix[1][1] ) )
                {
                    // Take the first row equation a00*x + a10*y + a20 = 0
                    // select a pivot element
                    if( fabs( diff.m_matrix[0][0] ) > fabs( diff.m_matrix[1][0] ) )
                    {
                        fixX = -diff.m_matrix[2][0] / diff.m_matrix[0][0];
                    }
                    else
                    {
                        fixY = -diff.m_matrix[2][0] / diff.m_matrix[1][0];
                    }
                }
                else
                {
                    // Take the second row equation a01*x + a11*y + a21 = 0
                    // select a pivot element
                    if( fabs( diff.m_matrix[0][1] ) > fabs( diff.m_matrix[1][1] ) )
                    {
                        fixX = -diff.m_matrix[2][1] / diff.m_matrix[0][1];
                    }
                    else
                    {
                        fixY = -diff.m_matrix[2][1] / diff.m_matrix[1][1];
                    }
                }
            }
            // else there is no fixpoint, e.g. a shift with a perpendicular scaling
        }
        else
        {
            // the matrix is regular => there is a single fixpoint
            fixX = - diff.GetDeterminant1() / detDiff;
            fixY = - diff.GetDeterminant2() / detDiff;
            hasFix = true;
        }
        /*
                #ifdef _DEBUG
                if( hasFix )
                {
                    double
                        dxn,dyn,
                        dxo,dyo;

                    mNew->TransformPoint(fixX, fixY, dxn, dyn);
                    mOld->TransformPoint(fixX, fixY, dxo, dyo);

                    assert(fabs(dxn-dxo) <= Eps * wxMax(fabs(dxn),fabs(dxo)));
                    assert(fabs(dyn-dyo) <= Eps * wxMax(fabs(dyn),fabs(dyo)));
                }
                #endif
        */
    }

    //------------------------------------------------------------------------
    // shift the affine transformation, so that the fixpoint is at 0
    // The operations below assume, that the transformation is centered
    //------------------------------------------------------------------------
    if( hasFix )
    {
        a20 += a00 * fixX + a10 * fixY;
        a21 += a01 * fixX + a11 * fixY;
    }

    //------------------------------------------------------------------------
    // restrict rotatition
    //------------------------------------------------------------------------
    if( ( snapWhat & snapRot ) && ( m_rotModes & info->m_rotModes ) )
    {
        int
        modes = m_rotModes & info->m_rotModes;

        // Check angle rotations
        if( m_rotationAngle )
        {
            if( modes & rotVectorAngleX )
            {
                // restrict angle of X-axis of target coordinate system
                AngleRestrictVectorRot( &a00, &a01 );
            }
            if( modes & rotVectorAngleY )
            {
                // restrict angle of Y-axis of target coordinate system
                AngleRestrictVectorRot( &a10, &a11 );
            }
        }

        // Check rational rotations
        if( m_rotationRationalNom && m_rotationRationalDen )
        {
            if( modes & rotVectorRationalX )
            {
                // restrict angle of X-axis of target coordinate system
                RationalRestrictVector( &a00, &a01 );
            }
            if( modes & rotVectorRationalY )
            {
                // restrict angle of Y-axis of target coordinate system
                RationalRestrictVector( &a10, &a11 );
            }
        }

        // Restrict angle of Y-axis of target coordinate system to X-axis +/- 90 deg
        if( modes & rotPureRotations )
        {
            double
            lenY = sqrt( sqr( a11 ) + sqr( a10 ) ),
            lenX = sqrt( sqr( a01 ) + sqr( a00 ) );
            if( lenX )
            {
                if( a00* a11 - a01* a10 >= 0 )
                {
                    // right handed purely rotated (and scaled) coordinate system
                    a10 = -a01 * lenY / lenX;
                    a11 = a11 * lenY / lenX;
                }
                else
                {
                    // left handed purely rotated (and scaled) coordinate system
                    a10 = a01 * lenY / lenX;
                    a11 = -a00 * lenY / lenX;
                }
            }
        }
    }

    //------------------------------------------------------------------------
    // restrict skews
    //------------------------------------------------------------------------
    if( ( snapWhat & snapSkew ) && ( m_rotModes & info->m_rotModes ) )
    {
        int
        modes = m_rotModes & info->m_rotModes;

        // Check angle rotations
        if( m_rotationAngle )
        {
            if( modes & rotVectorAngleX )
            {
                // restrict angle of X-axis of target coordinate system
                AngleRestrictVectorSkew( &a00, &a01, a10, a11 );
            }
            if( modes & rotVectorAngleY )
            {
                // restrict angle of Y-axis of target coordinate system
                AngleRestrictVectorSkew( &a10, &a11, a00, a01 );
            }
        }

        // If the X-Axis is rotated, the Y-Axis must not be rotated
        if( modes & rotPureSlanting )
        {

            if( a01 )
            {
                double
                lenY = sqrt( sqr( a11 ) + sqr( a10 ) );
                a11 = lenY;
                a10 = 0;
            }
        }
    }

    //------------------------------------------------------------------------
    // Restrict X-size
    //------------------------------------------------------------------------
    if( ( snapWhat & snapSize ) && ( m_sizeModesX & info->m_sizeModesX ) )
    {
        // This operation scales the a00/a01 axis vector
        // Various snapping methods are tried, and the one best
        // fitting the current length is chosen.
        int
        modes = m_sizeModesX & info->m_sizeModesX;
        double
        oldLen = wfa * sqrt( sqr( a00 ) + sqr( a01 ) ),
        newLen = oldLen;

        if( ( modes & sizeMin ) && oldLen < m_minSizeX )
        {
            newLen = m_minSizeX;
        }
        else if( ( modes & sizeMax ) && oldLen > m_maxSizeX )
        {
            newLen = m_maxSizeX;
        }
        else if( m_sizeX )
        {
            double
            best = DBL_MAX;

            if( modes & sizeLength )
            {
                SnapIfBetterSize( oldLen, SnapGrid( wfa * sqrt( sqr( a00 ) + sqr( a01 ) ), m_sizeX ), &best );
            }
            if( modes & sizeProject )
            {
                // axis vector is (a00,a01)
                // other vector is (a10,a11);
                // orthogonal of other vector is (-a11,a10)
                double
                otherLen = sqrt( sqr( a10 ) + sqr( a11 ) );
                if( otherLen )
                {
                    double
                    prjc = wfa * ( - a00 * a11 + a01 * a10 ) / otherLen;
                    if( prjc )
                    {
                        double
                        newLen = oldLen * SnapGrid( prjc, m_sizeX ) / prjc;
                        SnapIfBetterSize( oldLen, newLen, &best );
                    }
                }
            }
            if( modes & sizeMajor )
            {
                double
                major = wfa * wxMax( a00, a01 );
                if( major )
                {
                    double
                    newLen = oldLen * SnapGrid( major, m_sizeX ) / major;
                    SnapIfBetterSize( oldLen, newLen, &best );
                }
            }
            if( modes & sizeAxis )
            {
                double
                axis = wfa * a00;
                if( axis )
                {
                    double
                    newLen = oldLen * SnapGrid( axis, m_sizeX ) / axis;
                    SnapIfBetterSize( oldLen, newLen, &best );
                }
            }

            if( best != DBL_MAX )
            {
                newLen = oldLen + best;
                if( ( modes & sizeMin ) && newLen < m_minSizeX )
                    newLen = m_minSizeX;
                else if( ( modes & sizeMax ) && newLen > m_maxSizeX )
                    newLen = m_maxSizeX;
            }
        }
        if( newLen != oldLen )
        {
            if( oldLen )
            {
                if( w && ( snapWhat & snapWH ) )
                {
                    *w *= newLen / oldLen;
                }
                else
                {
                    a00 *= newLen / oldLen;
                    a01 *= newLen / oldLen;
                }
            }
            /*
                        else {
                            // a00/a01 vector is degenerated (0), so restore it
                            double
                                aLen = sqrt( sqr(a00)+sqr(a01) );
                            if(!aLen)
                            {
                                double
                                    oLen = sqrt( sqr(a10)+sqr(a11) );
                                if(oLen)
                                {
                                    // restore perpendicular to y-vector
                                    a00=a11/oLen;
                                    a01=-a10/oLen;
                                }
                                else {
                                    // restore standard x vector
                                    a00=1;
                                    a01=0;
                                }
                                aLen=1;
                            }
                            if(w && (snapWhat & snapWH))
                            {
                                *w = newLen/aLen;
                            }
                            else {
                                a00 *= newLen/aLen;
                                a01 *= newLen/aLen;
                            }
                        }
            */
        }
    }

    //------------------------------------------------------------------------
    // Restrict Y-size
    //------------------------------------------------------------------------
    if(
        ( snapWhat & snapSize ) && ( m_sizeModesY & info->m_sizeModesY ) &&
        // If w==h, only one needs to be restricted
        !( ( snapWhat & snapWH ) && w && h == w )

    )
    {
        // This operation scales the a10/a11 axis vector
        // Various snapping methods are tried, and the one best
        // fitting the current length is chosen.
        int
        modes = m_sizeModesY & info->m_sizeModesY;
        double
        oldLen = hfa * sqrt( sqr( a10 ) + sqr( a11 ) ),
        newLen = oldLen;

        if( ( modes & sizeMin ) && oldLen < m_minSizeY )
        {
            newLen = m_minSizeY;
        }
        else if( ( modes & sizeMax ) && oldLen > m_maxSizeY )
        {
            newLen = m_maxSizeY;
        }
        else if( m_sizeY )
        {
            double
            best = DBL_MAX;

            if( modes & sizeLength )
            {
                SnapIfBetterSize( oldLen, SnapGrid( hfa * sqrt( sqr( a10 ) + sqr( a11 ) ), m_sizeY ), &best );
            }
            if( modes & sizeProject )
            {
                // axis vector is (a10,a11)
                // other vector is (a00,a01);
                // orthogonal of other vector is (-a01,a00)
                double
                otherLen = sqrt( sqr( a00 ) + sqr( a01 ) );
                if( otherLen )
                {
                    double
                    prjc = hfa * ( - a10 * a01 + a11 * a00 ) / otherLen;
                    if( prjc )
                    {
                        double
                        newLen = oldLen * SnapGrid( prjc, m_sizeY ) / prjc;
                        SnapIfBetterSize( oldLen, newLen, &best );
                    }
                }
            }
            if( modes & sizeMajor )
            {
                double
                major = hfa * wxMax( a10, a11 );
                if( major )
                {
                    double
                    newLen = oldLen * SnapGrid( major, m_sizeY ) / major;
                    SnapIfBetterSize( oldLen, newLen, &best );
                }
            }
            if( modes & sizeAxis )
            {
                double
                axis = hfa * a11;
                if( axis )
                {
                    double
                    newLen = oldLen * SnapGrid( axis, m_sizeY ) / axis;
                    SnapIfBetterSize( oldLen, newLen, &best );
                }
            }

            if( best != DBL_MAX )
            {
                newLen = oldLen + best;
                if( ( modes & sizeMin ) && newLen < m_minSizeY )
                    newLen = m_minSizeY;
                else if( ( modes & sizeMax ) && newLen > m_maxSizeY )
                    newLen = m_maxSizeY;
            }
        }
        if( newLen != oldLen )
        {
            if( oldLen )
            {
                if( h && ( snapWhat & snapWH ) )
                {
                    *h *= newLen / oldLen;
                }
                else
                {
                    a10 *= newLen / oldLen;
                    a11 *= newLen / oldLen;
                }
            }
            /*
                        else {
                            // a10/a11 vector is degenerated (0), so restore it
                            double
                                aLen = sqrt( sqr(a10)+sqr(a11) );
                            if(!aLen)
                            {
                                double
                                    oLen = sqrt( sqr(a00)+sqr(a01) );
                                if(oLen)
                                {
                                    // restore perpendicular to x-vector
                                    a10=-a01/oLen;
                                    a11=a00/oLen;
                                }
                                else {
                                    // restore standard y vector
                                    a10=0;
                                    a11=1;
                                }
                                aLen=1;
                            }
                            if(h && (snapWhat & snapWH))
                            {
                                *h = newLen/aLen;
                            }
                            else {
                                a10 *= newLen/aLen;
                                a11 *= newLen/aLen;
                            }
                        }
            */
        }
    }

    //------------------------------------------------------------------------
    // Shift the transformation back
    //------------------------------------------------------------------------
    if( hasFix )
    {
        a20 -= a00 * fixX + a10 * fixY;
        a21 -= a01 * fixX + a11 * fixY;
    }

    mNew->m_matrix[0][0] = a00;
    mNew->m_matrix[0][1] = a01;
    mNew->m_matrix[1][0] = a10;
    mNew->m_matrix[1][1] = a11;

    //------------------------------------------------------------------------
    // Mutlitply wf/hf into mNew
    //------------------------------------------------------------------------
    {
        double
        wf  = w ? *w : 1,
        hf  = h ? *h : 1;
        a00 *= wf;
        a01 *= wf;
        a10 *= hf;
        a11 *= hf;
    }

    //------------------------------------------------------------------------
    // Restrict x-Position
    //------------------------------------------------------------------------
    if( ( snapWhat & snapPosX ) && ( m_posModesX & info->m_posModesX ) && m_docSnapDistX )
    {
        int
        modes = m_posModesX & info->m_posModesX;
        double
        bestShift = DBL_MAX;
        if( modes & posCenter )
            SnapIfBetterPos( a00 * info->m_centerX + a10 * info->m_centerY + a20, m_docSnapDistX, m_docOriginX, &bestShift );
        if( modes & posTopLeft )
            SnapIfBetterPos( a00 * info->m_left    + a10 * info->m_top     + a20, m_docSnapDistX, m_docOriginX, &bestShift );
        if( modes & posTop )
            SnapIfBetterPos( a00 * info->m_centerX + a10 * info->m_top     + a20, m_docSnapDistX, m_docOriginX, &bestShift );
        if( modes & posTopRight )
            SnapIfBetterPos( a00 * info->m_right   + a10 * info->m_top     + a20, m_docSnapDistX, m_docOriginX, &bestShift );
        if( modes & posRight )
            SnapIfBetterPos( a00 * info->m_right   + a10 * info->m_centerY + a20, m_docSnapDistX, m_docOriginX, &bestShift );
        if( modes & posBottomRight )
            SnapIfBetterPos( a00 * info->m_right   + a10 * info->m_bottom  + a20, m_docSnapDistX, m_docOriginX, &bestShift );
        if( modes & posBottom )
            SnapIfBetterPos( a00 * info->m_centerX + a10 * info->m_bottom  + a20, m_docSnapDistX, m_docOriginX, &bestShift );
        if( modes & posBottomLeft )
            SnapIfBetterPos( a00 * info->m_left    + a10 * info->m_bottom  + a20, m_docSnapDistX, m_docOriginX, &bestShift );
        if( modes & posLeft )
            SnapIfBetterPos( a00 * info->m_left    + a10 * info->m_centerY + a20, m_docSnapDistX, m_docOriginX, &bestShift );
        if( ( modes & posOther ) && info->m_other )
        {
            forEachIn( a2dVertexList, info->m_other )
            {
                a2dPoint2D point = ( *iter )->GetPoint();
                SnapIfBetterPos( a00 * point.m_x + a10 * point.m_y + a20, m_docSnapDistX, m_docOriginX, &bestShift );
            }
        }
        if( modes & pos6L )
        {
            // check if transformed x-axis is vertical
            if( fabs( a00 ) < Eps * fabs( a01 ) )
            {
                if( modes & posLineTop )
                    SnapIfBetterPos( a00 * info->m_centerX + a10 * info->m_top     + a20, m_docSnapDistX, m_docOriginX, &bestShift );
                if( modes & posLineHCenter )
                    SnapIfBetterPos( a00 * info->m_centerX + a10 * info->m_centerY + a20, m_docSnapDistX, m_docOriginX, &bestShift );
                if( modes & posLineBottom )
                    SnapIfBetterPos( a00 * info->m_centerX + a10 * info->m_bottom  + a20, m_docSnapDistX, m_docOriginX, &bestShift );
            }
            // check if transformed y-axis is vertical
            if( fabs( a10 ) < Eps * fabs( a11 ) )
            {
                if( modes & posLineLeft )
                    SnapIfBetterPos( a00 * info->m_left    + a10 * info->m_centerY + a20, m_docSnapDistX, m_docOriginX, &bestShift );
                if( modes & posLineVCenter )
                    SnapIfBetterPos( a00 * info->m_centerX + a10 * info->m_centerY + a20, m_docSnapDistX, m_docOriginX, &bestShift );
                if( modes & posLineRight )
                    SnapIfBetterPos( a00 * info->m_right   + a10 * info->m_centerY + a20, m_docSnapDistX, m_docOriginX, &bestShift );
            }
        }
        assert( fabs( bestShift ) <= 0.50001 * m_docSnapDistX );
        a20 += bestShift;
    }

    //------------------------------------------------------------------------
    // Restrict y-Position
    //------------------------------------------------------------------------
    if( ( snapWhat & snapPosY ) && ( m_posModesY & info->m_posModesY ) && m_docSnapDistY )
    {
        int
        modes = m_posModesY & info->m_posModesY;
        double
        bestShift = DBL_MAX;
        if( modes & posCenter )
            SnapIfBetterPos( a01 * info->m_centerX + a11 * info->m_centerY + a21, m_docSnapDistY, m_docOriginY, &bestShift );
        if( modes & posTopLeft )
            SnapIfBetterPos( a01 * info->m_left    + a11 * info->m_top     + a21, m_docSnapDistY, m_docOriginY, &bestShift );
        if( modes & posTop )
            SnapIfBetterPos( a01 * info->m_centerX + a11 * info->m_top     + a21, m_docSnapDistY, m_docOriginY, &bestShift );
        if( modes & posTopRight )
            SnapIfBetterPos( a01 * info->m_right   + a11 * info->m_top     + a21, m_docSnapDistY, m_docOriginY, &bestShift );
        if( modes & posRight )
            SnapIfBetterPos( a01 * info->m_right   + a11 * info->m_centerY + a21, m_docSnapDistY, m_docOriginY, &bestShift );
        if( modes & posBottomRight )
            SnapIfBetterPos( a01 * info->m_right   + a11 * info->m_bottom  + a21, m_docSnapDistY, m_docOriginY, &bestShift );
        if( modes & posBottom )
            SnapIfBetterPos( a01 * info->m_centerX + a11 * info->m_bottom  + a21, m_docSnapDistY, m_docOriginY, &bestShift );
        if( modes & posBottomLeft )
            SnapIfBetterPos( a01 * info->m_left    + a11 * info->m_bottom  + a21, m_docSnapDistY, m_docOriginY, &bestShift );
        if( modes & posLeft )
            SnapIfBetterPos( a01 * info->m_left    + a11 * info->m_centerY + a21, m_docSnapDistY, m_docOriginY, &bestShift );
        if( ( modes & posOther ) && info->m_other )
        {
            forEachIn( a2dVertexList, info->m_other )
            {
                a2dPoint2D point = ( *iter )->GetPoint();
                SnapIfBetterPos( a01 * point.m_x + a11 * point.m_y + a21, m_docSnapDistY, m_docOriginY, &bestShift );
            }
        }
        if( modes & pos6L )
        {
            // check if transformed x-axis is vertical
            if( fabs( a01 ) < Eps * fabs( a11 ) )
            {
                if( modes & posLineTop )
                    SnapIfBetterPos( a01 * info->m_centerX + a11 * info->m_top     + a21, m_docSnapDistY, m_docOriginY, &bestShift );
                if( modes & posLineHCenter )
                    SnapIfBetterPos( a01 * info->m_centerX + a11 * info->m_centerY + a21, m_docSnapDistY, m_docOriginY, &bestShift );
                if( modes & posLineBottom )
                    SnapIfBetterPos( a01 * info->m_centerX + a11 * info->m_bottom  + a21, m_docSnapDistY, m_docOriginY, &bestShift );
            }
            // check if transformed y-axis is vertical
            if( fabs( a01 ) < Eps * fabs( a11 ) )
            {
                if( modes & posLineLeft )
                    SnapIfBetterPos( a01 * info->m_left    + a11 * info->m_centerY + a21, m_docSnapDistY, m_docOriginY, &bestShift );
                if( modes & posLineVCenter )
                    SnapIfBetterPos( a01 * info->m_centerX + a11 * info->m_centerY + a21, m_docSnapDistY, m_docOriginY, &bestShift );
                if( modes & posLineRight )
                    SnapIfBetterPos( a01 * info->m_right   + a11 * info->m_centerY + a21, m_docSnapDistY, m_docOriginY, &bestShift );
            }
        }
        assert( fabs( bestShift ) <= 0.50001 * m_docSnapDistY );
        a21 += bestShift;
    }

    //------------------------------------------------------------------------
    // write back shift
    //------------------------------------------------------------------------
    mNew->m_matrix[2][0] = a20;
    mNew->m_matrix[2][1] = a21;

    // mNew->DebugDump(wxT("PostRestrict"), w ? *w : 0, h ? *h : 0);
}

void a2dRestrictionEngineOld::RestrictEndpointAngle( double* angle, const a2dAffineMatrix& WXUNUSED( matrix ), ESnapWhat snapWhat )
{
    if( snapWhat & snapEndAngle )
    {
        double
        deg2rad = wxPI / 180,
        rad2deg = 180 / wxPI;
        if( m_rotModes & rotEndpointAngle )
        {
            *angle = AngleRestrict( *angle * deg2rad ) * rad2deg;
        }
        else if( m_rotModes & rotEndpointRational )
        {
            *angle = RationalRestrict( *angle * deg2rad ) * rad2deg;
        }
    }
    // TODO !!!!! untransformed restriction
}

void a2dRestrictionEngineOld::RestrictPoint( double* x, double* y )
{
    // Restrict x-Position
    if( ( m_posModesX & posSngl ) && m_docSnapDistX )
    {
        *x = floor( ( *x - m_docOriginX ) / m_docSnapDistX + 0.5 ) * m_docSnapDistX + m_docOriginX;
    }
    // Restrict y-Position
    if( ( m_posModesY & posSngl ) && m_docSnapDistY )
    {
        *y = floor( ( *y - m_docOriginY ) / m_docSnapDistY + 0.5 ) * m_docSnapDistY + m_docOriginY;
    }
}

double a2dRestrictionEngineOld::AngleRestrict( double angle )
{
    double
    angleSnapRad = m_rotationAngle / 180 * wxPI;
    return floor( angle / angleSnapRad + 0.5 ) * angleSnapRad;
}

void a2dRestrictionEngineOld::AngleRestrictVectorRot( double* vecx, double* vecy )
{
    double
    angle = atan2( *vecy, *vecx ),
    len = sqrt( sqr( *vecx ) + sqr( *vecy ) );
    angle = AngleRestrict( angle );
    *vecx = cos( angle ) * len;
    *vecy = sin( angle ) * len;
}

void a2dRestrictionEngineOld::AngleRestrictVectorSkew( double* vecx, double* vecy, double otherx, double othery )
{
    double
    angleOld   = atan2( *vecy, *vecx ),
    angleNew   = AngleRestrict( angleOld ),
    angleOther = atan2( othery, otherx ),
    proj       = *vecx * othery - *vecy * otherx;

    if( fabs( angleNew - angleOther ) < 1e-5 )
    {
        // If the new angle is very close to the other vectors angle
        // the transformation will become singular.
        // To avoid this, use the closest other snap angle
        double
        angleSnapRad = m_rotationAngle / 180 * wxPI,
        angleNew1 = angleNew - angleSnapRad,
        angleNew2 = angleNew + angleSnapRad;
        if( fabs( angleOld - angleNew1 ) < fabs( angleOld - angleNew2 ) )
            angleNew = angleNew1;
        else
            angleNew = angleNew2;
    }

    double
    newx = cos( angleNew ),
    newy = sin( angleNew ),
    newpro = newx * othery - newy * otherx;

    if( !newpro )
        return;

    *vecx = newx * ( proj / newpro );
    *vecy = newy * ( proj / newpro );
}

double a2dRestrictionEngineOld::RationalRestrict( double angle )
{
    // map angle to first quadrant 0..45 deg
    int
    quad = 0;
    bool
    mirr = false;

    if( angle < 0 )
        angle += 2 * wxPI;

    if( angle > wxPI )
    {
        quad += 2;
        angle -= wxPI;
    }
    if( angle > wxPI / 2 )
    {
        quad += 1;
        angle -= wxPI / 2;
    }
    if( angle > wxPI / 4 )
    {
        mirr = true;
        angle = wxPI / 2 - angle;
    }

    if( !m_angleList )
    {
        // create the list of valid angles
        const int
        mAngles = 31 * 30 / 2 + 1;
        int
        nAngles = 0;
        double
        angles[mAngles];

        angles[nAngles++] = 0;
        int i;
        for( i = 1; i <= 31; i++ )
        {
            if( m_rotationRationalNom & ( 1 << i ) )
            {
                for( int j = i; j <= 31; j++ )
                {
                    if( m_rotationRationalDen & ( j << i ) )
                    {
                        assert( nAngles < mAngles );
                        angles[nAngles++] = atan( double( i ) / double( j ) );
                    }
                }
            }
        }

        // sort the list
        qsort( angles, nAngles, sizeof( *angles ), CmprDbl );

        // remove multiple equal values
        m_nAngleList = 1;
        for( i = 1; i < nAngles; i++ )
        {
            if( angles[i] - angles[m_nAngleList - 1] > 1e-13 )
            {
                angles[m_nAngleList++] = angles[i];
            }
        }

        // copy the temporary list
        m_angleList = new double[m_nAngleList];
        memcpy( m_angleList, angles, sizeof( *m_angleList )*m_nAngleList );
    }

    // find nearest angle in list
    if( m_nAngleList <= 1 )
    {
        angle = 0;
    }
    else
    {
        int
        low = 0,
        high = m_nAngleList - 1,
        mid;

        while( high - low > 1 )
        {
            mid = ( low + high ) >> 1;
            if( m_angleList[mid] < angle )
                low = mid;
            else
                high = mid;
        }

        // angle is between m_angleList[low] and m_angleList[high]
        // One of these two values must be the closest value
        if( angle - m_angleList[low] < m_angleList[high] - angle )
            angle = m_angleList[low];
        else
            angle = m_angleList[high];
    }

    // put the angle back to its original quadrant
    if( mirr )
        angle = wxPI / 2 - angle;
    angle += quad * wxPI / 2;

    return angle;
}

void a2dRestrictionEngineOld::RationalRestrictVector( double* vecx, double* vecy )
{
    double
    angle = atan2( *vecy, *vecx ),
    len = sqrt( sqr( *vecx ) + sqr( *vecy ) );
    angle = RationalRestrict( angle );
    *vecx = cos( angle ) * len;
    *vecy = sin( angle ) * len;
}
