/*! \file wx/canvas/restrict.h
    \brief snapping engine, for snapping while editing a drawing
    \author Michael Sögtrop
    \date Created 10/27/2003

    Copyright: 2003-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: restrict.h,v 1.22 2008/09/05 19:01:10 titato Exp $
*/

#ifndef __RESTRICT_H__
#define __RESTRICT_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/general/smrtptr.h"
#include "wx/general/gen.h"
#include "wx/artbase/artglob.h"
#include "wx/canvas/candefs.h"

//! mask for a2dSnapToWhat flags
typedef wxUint32 a2dSnapToWhatMask;

//!Restriction engine for editing restrictions like snapping
/*!
    The restriction engine is used by editing tools to restrict/snap editing
    of points, handles, pins, segments to the same things in other objects or
    to a grid on the whole drawing area. We call the object which needs to be snapped the source,
    while the object and points to which we want to snap the targets.
    When the source is a single vertex, the is no extra feature to be set.
    But if the source is a a2dCanvasObject, one can specify which features in it can be snapped to.
    This set by a2dSnapToWhat flags in m_snapSourceFeatures.
    For the targets, there are in general more options, set by a2dSnapToWhat flags in m_snapTargetFeatures.

    With rational snapping is meant snapping to angles whose tangent is a rational
    number with a small integer nominatior and denominator. The max
    nominator/denominator is the snapping parameter. For slanting angles,
    the absolute slanting angle (that is angle to the coordinate axis) is used,
    not the slanting angle relative to the rotated coordinate system. With
    rational snapping, you can solve many problems of slant and angle snapping,
    where you want to snap also to the horizontal and vertical grid. If we snap
    to angles with a small rational tangent, the resulting points will also snap to
    a small fraction of the horizontal or vertical grid.

    Each of these snapping modes may be enabled separately.

    What snapping features in the targets need to be enabled in this specific case is in several cases set
    by the parameter called sourceRequired. The member m_snapTargetFeatures contains the default target snapping
    features which are enabled, and the sourceRequired is indicating what the source object (point, canvasobject),
    requires to be enabled in that. m_snapSourceFeatures stays the same, and is only indicating
    what vertexes/features will be snapped, taking them from the input source object (point/canvas object).
    IOW The source canvas object in e.g the dragtool, can decide/require to which vertexes in the target
    objects it wants to snap. It does not always make sense to snap to all enabled target features.
    This way one can enable it central, and a tool use/require it in a local manner.

    A same approach could be implemented for the source objects, meaning enabling/disabling m_snapSourceFeatures
    parts when calling restrict on a source, but sofar there was no need for it.

    An example menu handler to enable/disable snapping:

    \code
    void MyDrawFrm::OnDrawRasterSnap(wxCommandEvent &event)
    {
        if( event.IsChecked() )
        {
            a2dRestrictionEngine
                *restrict=new a2dRestrictionEngine();
            restrict->SetSnapGrid(20,20);
            restrict->SetRotationAngle(15);
            restrict->SetSnapTargetFeatures( snapToGridPos |
                   snapToObjectPos | snapToPinsUnconnected  |
                   snapToObjectVertexes | snapToPointAngleMod );

            a2dCanvasGlobals->SetRestrictionEngine(restrict);
        }
        else
        {
            a2dCanvasGlobals->SetRestrictionEngine(0);
        }
    }
    \endcode

*/
class A2DCANVASDLLEXP a2dRestrictionEngine : public a2dObject
{
    DECLARE_EVENT_TABLE()

public:
    //! Constructor
    a2dRestrictionEngine();

    //! Destructor
    ~a2dRestrictionEngine();

    //! Flags for defining to what should be snapped to.
    //! To the drawing/snapping grid, and inside other a2dCanvasObject's
    enum a2dSnapToWhat
    {
        snapToNone       = 0x00000000, //!< no snap to modes specified
        snapToObjectPosX = 0x00000001, //!< perform x position snapping
        snapToObjectPosY = 0x00000002, //!< perform y position snapping

        snapToObjectPos    = snapToObjectPosX | snapToObjectPosY,

        snapToStart    = 0x00000100, //!< snap start point of line
        snapToEnd      = 0x00000200, //!< snap end point of line
        snapToPins     = 0x00000400, //!< snap to pins in other objects
        snapToPinsUnconnected
        = 0x00000800, //!< snap to pins in other objects when not connected
        snapToObjectIntersection
        = 0x00001000, //!< snap to intersections in other objects
        snapToObjectVertexes
        = 0x00002000, //!< snap to other objects its vertexes, which are decided in a2dCanvasObject::RestrictToObject()
        snapToObjectSegmentsMiddle
        = 0x00004000, //!< snap segments middle of other objects in a2dCanvasObject::RestrictToObject()
        snapToObjectSegments
        = 0x00008000, //!< snap segments of other objects in a2dCanvasObject::RestrictToObject()

        snapToGridPosX     = 0x00010000, //!< perform x grid snapping
        snapToGridPosY     = 0x00020000, //!< perform y grid snapping
        snapToGridPos      = snapToGridPosX | snapToGridPosY,

        snapToPointPosX    = 0x00040000, //!< perform x position snapping to a specific point
        snapToPointPosY    = 0x00080000, //!< perform y position snapping to a specific point
        snapToPointPosXorY = 0x00100000, //!< perform snapping to X or Y of a specific point
        snapToPointPosXorYForce = 0x00200000, //!< perform alligning to X or Y of a specific point
        snapToPointPos     = snapToPointPosX | snapToPointPosY,

        snapToPointAngleMod       = 0x00400000, //!< perform angle snapping to multiples of m_rotationAngle to a specific point
        snapToPointAngleModForced = 0x00800000, //!< perform angle snapping to multiples of m_rotationAngle to a specific point, snap if if not within angle threshold.
        snapToPointAngleRational  = 0x01000000, //!< perform rational angle snapping to a specific point
        snapToPointAngle          = snapToPointAngleMod | snapToPointAngleModForced | snapToPointAngleRational,

        snapToBoundingBox         = 0x02000000, //!< perform snapping to boundingbox of objects

        snapToObjectSnapVPath     = 0x04000000, //!< perform snapping to snapping vector path returned by  a2dCanvasObject::GetSnapVpath()

        snapToGridPosForced       = 0x10000000, //!< If no other snap point was closer, force to grid, even if not within threshold.

        snapToAll = 0xFFFFFFFF //! mask to allow all possible snapping modes.

    };

    friend a2dSnapToWhat operator | ( a2dSnapToWhat a, a2dSnapToWhat b ) {return ( a2dSnapToWhat ) ( ( int )a | ( int )b );}

    //! Set snapping grid (only position)
    void SetSnapGrid( const a2dDoMu& x,  const a2dDoMu& y ) {m_snapDistX = x; m_snapDistY = y; SetInternal(); SignalChange(); }

    //! Get horizontal center snapping distance (zero if disabled)
    const a2dDoMu& GetSnapGridX() const {return m_snapDistX; }

    //! Get vertical center snapping distance (zero if disabled)
    const a2dDoMu& GetSnapGridY() const {return m_snapDistY; }

    //! Set snapping origin (position grid offest/shift)
    void SetSnapOrigin( const a2dDoMu& x, const a2dDoMu& y ) {m_originX = x; m_originY = y; SetInternal(); SignalChange(); }

    void SetSnapOriginX( const a2dDoMu& x ) { m_originX = x; SetInternal(); SignalChange(); }
    void SetSnapOriginY( const a2dDoMu& y ) { m_originY = y; SetInternal(); SignalChange(); }

    //! Get snapping origin X (position grid offset/shift)
    const a2dDoMu& GetSnapOriginX() const {return m_originX;}

    //! Get snapping origin Y (position grid offset/shift)
    const a2dDoMu& GetSnapOriginY() const {return m_originY;}

    //! Set rotation angle raster
    void SetRotationAngle( double a ) {m_rotationAngle = a; SignalChange(); }

    //! Get rotation angle raster
    double GetRotationAngle() const { return m_rotationAngle; }

    //! used to snap  vertexes to a pin or point like snapping features in objects.
    int GetSnapThresHold() const { return m_snapThresHold; }

    //! used to snap  vertexes to a pin or point like snapping features in objects.
    void SetSnapThresHold( int thresHold ) { m_snapThresHold = thresHold; SignalChange(); }

    //! used to snap vertexes to a pin or point, for snapping features in objects.
    /*!
        Calculated from m_snapThresHold, taking into account the active a2dDrawingPart.
    */
    double GetSnapThresHoldWorld() const;

    //! how close, defined in degress, must the current angle be, to a snap rotation angle, in order to snap.
    void SetSnapThresHoldAngle( double thresHoldAngle ) { m_rotationThreshold = thresHoldAngle; }

    //! See SetSnapThresHoldAngle()
    double GetSnapThresHoldAngle() const { return m_rotationThreshold; }

    //! enable all snapping features or not
    void SetSnap( bool snap ) { m_snap = snap; SignalChange(); }

    //! enable all snapping features or not
    bool GetSnap() const {return m_snap;}

    //! enable all snapping features for object only for visible object.
    void SetSnapOnlyVisibleObjects( bool snapOnlyVisbleObjects ) { m_snapOnlyVisbleObjects = snapOnlyVisbleObjects; SignalChange(); }

    //! all snapping features for object only for visible object?
    bool GetSnapOnlyVisibleObjects() const { return m_snapOnlyVisbleObjects; }

    //! Set rotation rational raster nominator/denominator mask
    /*!
        nomMask bitX / denMask bit Y present an angle in radians to which can be snapped.
        e.g. if bit 2 is set in nomMask, and bit 6 is set in denMask, we will have 1/5 snap to 2*PI/5

        \param nomMask  each bit set here presents a number (bit1 number 0 until bit32 number 31)
        \param denMask  each bit set here presents a number (bit1 number 0 until bit32 number 31)
    */
    void SetRotationRational( wxUint32 nomMask, wxUint32 denMask )
    {
        m_rotationRationalNom = nomMask; m_rotationRationalDen = denMask; SignalChange(); 
    }

    //! set what snapping features or enabled for the source to snap to.
    //! see a2dSnapToWhat for snapping features.
    void SetSnapSourceFeatures( wxUint32 snapSourceFeatures ) { m_snapSourceFeatures = snapSourceFeatures; SignalChange(); }

    //! set one of the snapping features for the source to true or false, leaf others as is
    void SetSnapSourceFeature( a2dSnapToWhat snapSourceFeature, bool value = true )
    {
        m_snapSourceFeatures = value ? m_snapSourceFeatures | snapSourceFeature :
                               m_snapSourceFeatures & ( snapToAll ^ snapSourceFeature );
        SignalChange(); 
    }

    //! set what snapping features or enabled for the source to snap to.
    //! see a2dSnapToWhat for snapping features.
    a2dSnapToWhatMask GetSnapSourceFeatures() const { return m_snapSourceFeatures; }

    //! set what snapping features or enabled for the target to snap to.
    //! see a2dSnapToWhat for snapping features.
    void SetSnapTargetFeatures( wxUint32 snapTargetFeatures ) { m_snapTargetFeatures = snapTargetFeatures; SignalChange(); }

    //! set one of the snapping features for targets to true or false, leaf others as is
    void SetSnapTargetFeature( a2dSnapToWhat snapTargetFeature, bool value = true )
    {
        m_snapTargetFeatures = value ? m_snapTargetFeatures | snapTargetFeature :
                           m_snapTargetFeatures & ( snapToAll ^ snapTargetFeature );
        SignalChange(); 
    }

    //! return the setting of a specific snapping feature
    bool GetSnapTargetFeature( a2dSnapToWhat snapTargetFeature ) const
    {
        return ( m_snapTargetFeatures & snapTargetFeature ) > 0;
    }

    //! set what snapping features or enabled for the target to snap to.
    //! see a2dSnapToWhat for snapping features.
    a2dSnapToWhatMask GetSnapTargetFeatures() const { return m_snapTargetFeatures; }

    a2dSnapToWhatMask GetSnapTargetFeaturesNoAngle() const { return m_snapTargetFeatures & (~ (a2dRestrictionEngine::snapToPointAngleMod | a2dRestrictionEngine::snapToPointAngleRational)); }

    //! return the clossest vertex which can be snapped if any.
    /*!
        The source a2dCanvasObject is asked for its snapping features/vertexes via a2dCanvasObject::GetSnapVpath(),
        but only those ementioned in  m_snapSourceFeatures. All the features returned will be tried to snap to neighbouring
        canvas objects, and eventually the grid. e.g. It calls for each vertex in the snapping path the function RestrictPoint(),
        and the clossest snapped point will be snapped to.

        \param object object to be snapped to other snapping targets
        \param point if a point to snap is found, it is stored here.
        \param dx x distance from the returned point to the snapping point.
        \param dy y distance from the returned point to the snapping point.
        \param sourceRequired what the caller wants the snapping points in the object to restrict to. ( is ANDed with m_snapTargetFeatures )
        \param ignoreEngine set to ignore engine its own snap modes.
    */
    virtual bool RestrictCanvasObjectAtVertexes( a2dCanvasObject* object, a2dPoint2D& point, double& dx, double& dy, wxUint32 sourceRequired = snapToAll & ~a2dRestrictionEngine::snapToPointAngle, bool ignoreEngine = false );

    //! return the clossest vertex which can be snapped if any.
    /*!
        \param segments segments to be snapped to other snapping targets
        \param point if a point to snap is found, it is stored here.
        \param dx x distance from the returned point to the snapping point.
        \param dy y distance from the returned point to the snapping point.
        \param sourceRequired what the caller wants the snapping points to restrict to. ( is ANDed with m_snapTargetFeatures )
        \param ignoreEngine set to ignore engine its own snap modes.
    */
    virtual bool RestrictVertexes( a2dVertexArray* segments, a2dPoint2D& point, double& dx, double& dy, wxUint32 sourceRequired = snapToAll & ~a2dRestrictionEngine::snapToPointAngle, bool ignoreEngine = false );

    //! return the clossest vertex which can be snapped if any.
    /*!
        \param lsegments segments to be snapped to other snapping targets
        \param point if a point to snap is found, it is stored here.
        \param dx x distance from the returned point to the snapping point.
        \param dy y distance from the returned point to the snapping point.
        \param sourceRequired what the caller wants the snapping points to restrict to. ( is ANDed with m_snapTargetFeatures )
        \param ignoreEngine set to ignore engine its own snap modes.
    */
    virtual bool RestrictVertexes( a2dVertexList* lsegments, a2dPoint2D& point, double& dx, double& dy, wxUint32 sourceRequired = snapToAll & ~a2dRestrictionEngine::snapToPointAngle, bool ignoreEngine = false );

    //! Restrict a single point of a line or polyline
    /*!
        The input point( x,y ) is snapped to the targets, being:
            - all child canvasobjects of m_parentObject by pins (connected or not).
            - all child canvasobjects of m_parentObject object vertexes.
            - all child canvasobjects of m_parentObject using a snap vector path, returned by object.
            - snap to grid
        Which features in the tragets can be snapped to, depends on sourceRequired.

        \param x input x and output x after restriction
        \param y input y and output y after restriction
        \param sourceRequired what the caller wants the (x,y) to restrict to inside the targets. 
               ( is ANDed with m_snapTargetFeatures )
        \param ignoreEngine set to ignore engine its own snap modes.

        \return true if the point was restricted
    */
    virtual bool RestrictPoint( double& x, double& y,  wxUint32 sourceRequired = snapToAll & ~a2dRestrictionEngine::snapToPointAngle, bool ignoreEngine = false );

    //! Restrict a single line
    /*!
        \param line the line to restrict
        \param sourceRequired what the caller wants the (x,y) to restrict to. ( is ANDed with m_snapTargetFeatures )
        \param ignoreEngine set to ignore engine its own snap modes.

        \return true if the point was restricted
    */
    virtual bool RestrictLine( a2dLine& line,  wxUint32 sourceRequired = snapToAll, bool ignoreEngine = false );

    //! Restrict angle
    /*!
        \param ang input angle and output angle after restriction
        \param sourceRequired what the caller wants the (x,y) to restrict to. ( is ANDed with m_snapTargetFeatures )
        \param ignoreEngine set to ignore engine its own snap modes.

        \return true if the point was restricted
    */
    virtual bool RestrictAngle( double* ang,  wxUint32 sourceRequired = snapToAll, bool ignoreEngine = false );

    //! sets the point for snapping to
    /*!
       In modes:
        - a2dSnapToWhat::snapToPointPos
        - a2dSnapToWhat::snapToPointPosX
        - a2dSnapToWhat::snapToPointPosY
        - a2dSnapToWhat::snapToPointPos
        - a2dSnapToWhat::snapToPointPosXorY
    */
    void SetRestrictPoint( double xSnap, double ySnap ) { m_pointToRestrictTo = a2dPoint2D( xSnap, ySnap ); SignalChange(); }

    //! to ask engine for the restrict point
    /*!
        For snapping modes that restrict to X,Y of the restrict Point.
    */
    const a2dPoint2D& GetRestrictPoint() const { return m_pointToRestrictTo; }

    //! to ask engine for the point that needs to be snapped to a a2dCanvasObject
    /*!
        When engine is asked to snap a vertex/point, the result point to snap is set.
        This can be used by e.g. a2dCanvasObject::RestrictToObject() to snap to pins etc.
    */
    a2dPoint2D GetPointToSnap() const { return m_pointToSnap; }

    //! set the point that was snapped to a a2dCanvasObject vertex
    /*!
        When engine is asked to snap a vertex/point, the result point is set here.
        This is used e.g. a2dCanvasObject::RestrictToObject() to snap to pins etc.
    */
    void SetPointSnapResult( const a2dPoint2D& p );

    //! check if point is within threshold to m_pointToSnap
    bool IsWithInThresHold( const a2dPoint2D& p );

    //! to ask engine for the line that needs to be snapped to a a2dCanvasObject
    /*!
        When engine is asked to snape a line, the line to snap is set.
        This can be used by e.g. a2dCanvasObject::RestrictToObject() to snap to other lines, pins etc.
    */
    a2dLine& GetLineToSnap() { return m_lineToSnap; }

    //! parent object of objects that need to be snapped to
    /*!
        For snapping features where canvas objects are involved, this needs to be set to the
        parent object of the objects which can be snaped or can be snapped to.
        For example when needing to snap a vertex/point to the objects in a a2dDrawingPart,
        set this parent object to the a2dDrawingPart::GetShowObject().

        \remark when using the restriction engine, you much make sure this is set right each time.

        \param obj: pointer to object to set as parent
    */
    void SetParentSnapObjects( a2dCanvasObject* obj ) { m_parentObject = obj; }

    //! return pointer of the current parent object
    /*!
        \return: pointer to the current parent object.
    */
    a2dCanvasObject* GetParentSnapObjects() const { return m_parentObject; }


    //! some snapping may depend on shift key being down
    void SetShiftKeyDown( bool shiftDown ) { m_shiftDown = shiftDown;}

    //! some snapping may depend on shift key being down
    bool GetShiftKeyDown() const { return m_shiftDown; }

    //! do not snap if set
    void SetReleaseSnap( bool releaseSnap ) { m_releaseSnap = releaseSnap;}

    //! do not snap if set
    bool GetReleaseSnap() const { return m_releaseSnap; }

    //! react on activation of a view, to set snap margin
    //void OnActivate( a2dViewEvent& viewevent );

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const { return NULL; }


#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) ) { wxASSERT( 0 ); }
    virtual void DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) ) { wxASSERT( 0 ); }
#endif //wxART2D_USE_CVGIO

protected:

    void SignalChange();

    void SetInternal();

    bool AngleRestrictWithinThreshold( double& angle );

    //! Restrict an angle
    double AngleRestrict( double angle );
    //! Restrict the angle of a vector, keeping the vectors length
    void AngleRestrictVectorRot( double* vecx, double* vecy );
    //! Restrict the angle of a vector, keeping the vectors projection on the orthogonal of another vector
    void AngleRestrictVectorSkew( double* vecx, double* vecy, double otherx, double othery );

    //! Rational restrict an angle
    double RationalRestrict( double angle );
    //! Rational restrict the angle of a vector
    void RationalRestrictVector( double* vecx, double* vecy );

    //! set to the a2dSnapToWhat features enabled by the engine for the source object
    a2dSnapToWhatMask m_snapSourceFeatures;

    //! set to the a2dSnapToWhat target features enabled by the engine and/or snap source object
    a2dSnapToWhatMask m_snapTargetFeatures;

    a2dDoMu m_snapDistX;      //!< horizontal position snapping grid distance
    a2dDoMu m_snapDistY;      //!< vertical position snapping grid distance
    a2dDoMu m_originX;   //!< horizontal grid snapping origin vs. coordinate origin
    a2dDoMu m_originY;   //!< vertical grid snapping origins vs. coordinate origin
    double m_rotationAngle;      //!< angle snapping angle in degrees
    wxUint32 m_rotationRationalNom; //!< bit map of possible nominators (bit1->0.. bit32->31) for rational angle snapping
    wxUint32 m_rotationRationalDen; //!< bit map of possible denominators (bit1->0.. bit32->31) for rational angle snapping
    double* m_angleList;  //!< sorted list of allowed rational angles
    int m_nAngleList;     //!< number of allowed rational angles

    //! threshold in pixels towards the snapping point.
    int m_snapThresHold;

    //! angle will be snapped if within this threshold defined in degrees.
    double m_rotationThreshold;

    //! set to point that is currently being snapped.
    a2dPoint2D m_pointToSnap;

    //! set to point that is snapped to m_pointToSnap
    a2dPoint2D m_snappedPoint;

    a2dPoint2D m_pointToRestrictTo;

    a2dLine m_lineToSnap;

    bool m_snapOnlyVisbleObjects; //!< if true snapping modes for object is only on visible object.
    bool m_snap; //!< if true snapping modes are enabled, else non.
    bool m_releaseSnap; //!< to not snap inside snapping engine.
    bool m_shiftDown; //!< snapping modifier when shift key is pressed

    //! parent canvas object in a2dCanvasDocument on which snapping needs to be done.
    a2dCanvasObject* m_parentObject;

    double m_docSnapDistX;
    double m_docSnapDistY;
    double m_docOriginX;
    double m_docOriginY;

public:

    static a2dPropertyIdUint32* PROPID_SnapSourceFeaturesMem;
    static a2dPropertyIdUint32* PROPID_SnapSourceFeatures;
    static a2dPropertyIdUint32* PROPID_SnapTargetFeatures;
    static a2dPropertyIdDouble* PROPID_RotationAngle;
    static a2dPropertyIdUint32* PROPID_RotationRationalNom;
    static a2dPropertyIdUint32* PROPID_RotationRationalDen;
    static a2dPropertyIdInt32*  PROPID_SnapThresHold;
    static a2dPropertyIdPoint2D* PROPID_PointToSnap;
    static a2dPropertyIdPoint2D* PROPID_PointToRestrictTo;
    static a2dPropertyIdBool* PROPID_SnapOnlyVisbleObjects;
    static a2dPropertyIdBool* PROPID_Snap;
    static a2dPropertyIdBool* PROPID_SnapGetSet;
    static a2dPropertyIdBool* PROPID_AltDown;
    static a2dPropertyIdBool* PROPID_ShiftDown;

    static const a2dSignal sig_changed;

    DECLARE_PROPERTIES()
};

//! Smart pointer type for restriction engine
#if defined(WXART2D_USINGDLL)
template class A2DCANVASDLLEXP a2dSmrtPtr<a2dRestrictionEngine>;
#endif
typedef a2dSmrtPtr<a2dRestrictionEngine> a2dRestrictionEnginePtr;










//! DEPRECATED Restriction engine for editing restrictions like snapping
/*!
The restriction engine is used by editing tools to restrict editing
of points and affine transformations.

- For affine transformations, the following restrictions are implemented

  -# Snapping of position
  -# Snapping of the size
  -# Snapping of the rotation in frations of a full rotation or rational
     snapping.
  -# Snapping of the slanting angles in fractions of a full rotation or
     rational snapping
  -# Minimum and maximum size

  With rational snapping i mean snapping to angles whose tangent is a rational
  number with a small integer nominatior and denominator. The max
  nominator/denominator is the snapping parameter. For slanting angles,
  the absolute slanting angle (that is angle to the coordinate axis) is used,
  not the slanting angle relative to the rotated coordinate system. With
  rational snapping, you can solve many problems of slant and angle snapping,
  where you want to snap also to the horizontal and vertical grid. If we snap
  to angles with a small rational tan, the resulting points will also snap to
  a small fraction of the horiontal or vertical grid.

- For position snapping of affine transformations, any of the 4 corner,
  4 midline or the center point can snap to a grid. Each point can be enabled
  separately. Snapping of the non-center points can also be enabled only
  in the case, that they are connected to a vertical or horizontal border line.

- For size snapping of affine transformations assume that the affine transformation
  transformes the origin and two unity length coordinate axis vectors to a different
  origin with different axis orientation and length. This way, the transformation
  consits of three vectors, the origin vector, the x-axis vector and the
  y-axis vector. The x-size can be one of the following:

  -# the length of the transformed x-axis vector
  -# the projection of the transformed x-axis vector on a line perpendicular
     to the transformed y-axis
  -# the larger component of the x-axis vector
  -# the first (horizontal) component of the x-axis vector.

  Each of these snapping modes may be enabled separately.

- Points and lines are simply snapped to a grid.

- An example menu handler to enable/disable snapping:

\code
void MyDrawFrm::OnDrawRasterSnap(wxCommandEvent &event)
{
    if( event.IsChecked() )
    {
        a2dRestrictionEngineOld
            *restrict=new a2dRestrictionEngineOld();
        restrict->SetSnapGrid(20,20);
        restrict->SetPosModes(a2dRestrictionEngineOld::pos3P|a2dRestrictionEngineOld::posSngl|a2dRestrictionEngineOld::posOther);
        restrict->SetSizeModes(a2dRestrictionEngineOld::sizeLength|a2dRestrictionEngineOld::sizeMin);
        restrict->SetRotationModes(a2dRestrictionEngineOld::rotAllAngle);
        restrict->SetRotationAngle(15);
        a2dCanvasGlobals->SetRestrictionEngine(restrict);
    }
    else
    {
        a2dCanvasGlobals->SetRestrictionEngine(0);
    }
}
\endcode

\todo Implement angle snapping for lines with length snapping or snapping of
the endpoints to vertical or horizontal grodlines.
*/

class A2DCANVASDLLEXP a2dRestrictionEngineOld : public a2dObject
{
public:
    //! Constructor
    a2dRestrictionEngineOld();

    //! Destructor
    ~a2dRestrictionEngineOld();

    //! Position snap modes/flags (once for x any y)
    enum EPositionSnapModes
    {
        posCenter      = 0x00000001, //!< enable position snapping of object center to grid
        posTopLeft     = 0x00000002, //!< enable position snapping of object top left to grid
        posTop         = 0x00000004, //!< enable position snapping of object top to grid
        posTopRight    = 0x00000008, //!< enable position snapping of object top right to grid
        posRight       = 0x00000010, //!< enable position snapping of object right to grid
        posBottomRight = 0x00000020, //!< enable position snapping of object bottom right to grid
        posBottom      = 0x00000040, //!< enable position snapping of object bottom to grid
        posBottomLeft  = 0x00000080, //!< enable position snapping of object bottom left to grid
        posLeft        = 0x00000100, //!< enable position snapping of object left to grid
        posLineTop     = 0x00000200, //!< enable position snapping of object top line to grid, if x axis parallel to main axis
        posLineHCenter = 0x00000400, //!< enable position snapping of object h-center line to grid, if x axis parallel to main axis
        posLineBottom  = 0x00000800, //!< enable position snapping of object bottom line to grid, if x axis parallel to main axis
        posLineLeft    = 0x00001000, //!< enable position snapping of object left line to grid, if y axis parallel to main axis
        posLineVCenter = 0x00002000, //!< enable position snapping of object v-center line to grid, if y axis parallel to main axis
        posLineRight   = 0x00004000, //!< enable position snapping of object right line to grid, if y axis parallel to main axis
        posOther       = 0x00008000, //!< enable position snapping of object other points to grid (e.g. triangle corner)
        posSngl        = 0x00010000, //!< enable restriction of single points (e.g. of polylines)

        posEqual       = 0x80000000, //!< used internally
        posNone        = 0x00000000, //!< disable all posizion restriction modes

        //! enable all 6 line snapping modes
        pos6L          = posLineTop | posLineHCenter | posLineBottom | posLineLeft | posLineVCenter | posLineRight,
        //! enable center point snapping modes
        pos1P          = posCenter,
        //! enable center + top left point snapping modes
        pos2P          = posCenter | posTopLeft,
        //! enable center + top left + bottom right point snapping modes
        pos3P          = posCenter | posTopLeft | posBottomRight,
        //! enable all center + 4 corner point snapping modes
        pos5P          = posCenter | posTopLeft | posTopRight | posBottomRight | posBottomLeft,
        //! enable all 9 point snapping modes
        pos9P          = posCenter | posTopLeft | posTop | posTopRight | posRight | posBottomRight | posBottom | posBottomLeft | posLeft,
        //! enable all position restriction modes, except other point modes
        posAny         = pos9P | pos6L | posSngl,
        //! enable center point and all 6 line snapping modes
        pos1P6L        = posCenter | pos6L,
    };
    friend EPositionSnapModes operator | ( EPositionSnapModes a, EPositionSnapModes b ) {return ( EPositionSnapModes ) ( ( int )a | ( int )b );}

    //! Size snap modes/flags (once for x and y)
    enum ESizeSnapModes
    {
        sizeLength     = 0x00000001, //!< enable size snapping of transformed axis vector length
        sizeProject    = 0x00000002, //!< enable snapping of projection of transformed axis vector length to orthogonal of other axis vector
        sizeMajor      = 0x00000004, //!< enable size snapping of transformed axis major component
        sizeAxis       = 0x00000008, //!< enable size snapping of projection to untronsformed axis
        sizePos        = 0x00000010, //!< snap size by snapping individual points
        sizeMin        = 0x00000020, //!< enable minimum size restriction
        sizeMax        = 0x00000040, //!< enable maximum size restriction
        sizeEqual      = 0x80000000, //!< used internally
        sizeNone       = 0x00000000, //!< disable all size restriction modes
        sizeAny        = 0x0000007F  //!< enable all size restriction modes
    };
    friend ESizeSnapModes operator | ( ESizeSnapModes a, ESizeSnapModes b ) {return ( ESizeSnapModes ) ( ( int )a | ( int )b );}

    //! Rotation/Slanting snap modes/flags
    enum ERotationSnapModes
    {
        // affine rotation/skew snapping
        rotVectorAngleX     = 0x00000001, //!< enable angle snapping for rotation of first coordinate vektor
        rotVectorRationalX  = 0x00000002, //!< enable rational snapping for rotations of first coordinate vektor
        rotVectorAngleY     = 0x00000004, //!< enable angle snapping for rotations of second coordinate vektor
        rotVectorRationalY  = 0x00000008, //!< enable rational snapping for rotations of second coordinate vektor
        rotPureRotations    = 0x00000010, //!< allow only pure rotations, no slanting
        rotPureSlanting     = 0x00000020, //!< allow only pure one axis slanting

        // endpoint angle snapping
        rotEndpointAngle    = 0x00000040, //!< enable snapping of arc endpoint angles
        rotEndpointRational = 0x00000080, //!< enable snapping of arc endpoint angles (rational)
        rotEndpointUntrans  = 0x00000100, //!< enable snapping of arc ednpoints in untransformed coordinate system

        rotNone             = 0x00000000, //!< disable all rotation restriction modes
        rotAllAngle         = rotVectorAngleX | rotVectorAngleY | rotEndpointAngle,
        rotAllRational      = rotVectorRationalX | rotVectorRationalY | rotEndpointRational,
        rotAll              = 0x000001FF
    };
    friend ERotationSnapModes operator | ( ERotationSnapModes a, ERotationSnapModes b ) {return ( ERotationSnapModes ) ( ( int )a | ( int )b );}

    //! Flags for what to touch during a restriction
    enum ESnapWhat
    {
        snapPosX     = 0x00000001, //!< perform x position snapping
        snapPosY     = 0x00000002, //!< perform y position snapping
        snapSizeX    = 0x00000004, //!< perform x size snapping
        snapSizeY    = 0x00000008, //!< perform y size snapping
        snapRot      = 0x00000010, //!< perform rotation snapping
        snapSkew     = 0x00000020, //!< perform skew snapping
        snapWH       = 0x00000040, //!< during size snapping, change w/h rather than transform
        snapEndAngle = 0x00000080, //!< snap angle of endpoints or arcs
        snapStart    = 0x00000100, //!< snap start point of line
        snapEnd      = 0x00000200, //!< snap end point of line
        snapPointI   = 0x80000000, //!< snap point i of polygon (ored with i, other flags are invalid)
        snapPos      = snapPosX | snapPosY,
        snapSize     = snapSizeX | snapSizeY,
        snapPosSize  = snapPos | snapSize,
        snapSizeWH   = snapSize | snapWH,
    };
    friend ESnapWhat operator | ( ESnapWhat a, ESnapWhat b ) {return ( ESnapWhat ) ( ( int )a | ( int )b );}

    //! Structure to descripe snapping properties of an object
    struct SnapObjectInfo
    {
        double m_left;    //!< minimum x coordinate (e.g. 0), transformed via affine transform and w
        double m_top;     //!< minimum y coordinate (e.g. 0), transformed via affine transform and h
        double m_right;   //!< maximum x coordinate (e.g. 1), transformed via affine transform and w
        double m_bottom;  //!< maximum y coordinate (e.g. 1), transformed via affine transform and h
        double m_centerX; //!< center x coordinate (e.g. 0.5), transformed via affine transform and w
        double m_centerY; //!< center y coordinate (e.g. 0.5), transformed via affine transform and h
        EPositionSnapModes m_posModesX;  //!< anded with m_posModesX of wxRetrictionEngine
        EPositionSnapModes m_posModesY;  //!< anded with m_posModesY of wxRetrictionEngine
        ESizeSnapModes m_sizeModesX;     //!< anded with m_sizeModesX of wxRetrictionEngine
        ESizeSnapModes m_sizeModesY;     //!< anded with m_sizeModesY of wxRetrictionEngine
        ERotationSnapModes m_rotModes;   //!< anded with m_rotModes of wxRetrictionEngine
        class a2dVertexList* m_other; //!< other snapping points (enabled by bits in m_posModesX/m_posModesY, NOT owned or deleted by this object)
    };

    //! Set position snap modes (ored EPositionSnapModes)
    void SetPosModes( EPositionSnapModes modeX, EPositionSnapModes modeY = posEqual ) {m_posModesX = modeX; m_posModesY = modeY == posEqual ? modeX : modeY;}
    //! Get position snap modes (ored EPositionSnapModes)
    EPositionSnapModes GetPosModesX() {return m_posModesX;}
    EPositionSnapModes GetPosModesY() {return m_posModesY;}

    //! Set size snap modes (ored ESizeSnapModes)
    void SetSizeModes( ESizeSnapModes modeX, ESizeSnapModes modeY = sizeEqual ) {m_sizeModesX = modeX; m_sizeModesY = modeY == sizeEqual ? modeX : modeY;}
    //! Get size snap modes (ored ESizeSnapModes)
    ESizeSnapModes GetSizeModesX() {return m_sizeModesX;}
    ESizeSnapModes GetSizeModesY() {return m_sizeModesY;}

    //! Set rotation/slanting snap modes (ored ERotationSnapModes)
    void SetRotationModes( ERotationSnapModes mode ) {m_rotModes = mode;}
    //! Get rotation/slanting snap modes (ored ERotationSnapModes)
    ERotationSnapModes GetRotationModes() {return m_rotModes;}

    //! Set snapping grid (position and size)
    void SetSnapGrid( double x,  double y ) {m_snapDistX = x; m_snapDistY = y; m_sizeX = x, m_sizeY = y; }
    //! Set snapping grid (only position)
    void SetPosGrid( double x, double y ) {m_snapDistX = x; m_snapDistY = y;}
    //! Set snapping grid (only size)
    void SetSizeGrid( double x, double y ) {m_sizeX = x; m_sizeY = y;}
    //! Set snapping origin (position grid offest/shift)
    void SetSnapOrigin( double x, double y ) {m_originX = x; m_originY = y;}
    //! Set minimum size
    void SetMinSize( double x, double y ) {m_minSizeX = x; m_minSizeY = y; }
    //! Set maximum size
    void SetMaxSize( double x, double y ) {m_maxSizeX = x; m_maxSizeY = y; }
    //! Set rotation angle raster
    void SetRotationAngle( double a ) {m_rotationAngle = a; }
    //! Set rotation rational raster nominator/denominator mask
    void SetRotationRational( wxUint32 nomMask, wxUint32 denMask )
    {
        m_rotationRationalNom = nomMask; m_rotationRationalDen = denMask;
    }

    //! Restrict an affine transformation
    /*! \param mNew the edited tranformation matrix, that will be restricted
     *  \param mOld the tranformation matrix as it was prior to editing
     *  \param snapWhat enable various snapping options
     *  \param info snapping info
     *  \param w if (snapWhat & snapWH) the w parameter reflects the width, rather than the matrix
     *  \param h if (snapWhat & snapWH) the h parameter reflects the height, rather than the matrix
    */
    void RestrictAffine( a2dAffineMatrix* mNew, const a2dAffineMatrix* mOld, ESnapWhat snapWhat, SnapObjectInfo* info, double* w = 0, double* h = 0 );

    //! Restrict a starting /ending angle of e.g. an elliptic arc
    //! The angle is in degrees
    //! The matrix is required for e.g. untransformed snapping
    void RestrictEndpointAngle( double* angle, const a2dAffineMatrix& matrix, ESnapWhat snapWhat );

    //! Restrict a single point of a line or polyline
    void RestrictPoint( double* x, double* y );

    //! Get horizontal center snapping distance (zero if disabled)
    double GetCenterSnapX() {return ( m_posModesX & posAny ) ? m_snapDistX : 0;}
    //! Get vertical center snapping distance (zero if disabled)
    double GetCenterSnapY() {return ( m_posModesY & posAny ) ? m_snapDistY : 0;}

private:
    //! Some stuff nedded by the ref counting base class
    virtual a2dObject* DoClone( CloneOptions WXUNUSED( options ) ) const { wxASSERT( 0 ); return 0; }

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) ) { wxASSERT( 0 ); }
    virtual void DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) ) { wxASSERT( 0 ); }
#endif //wxART2D_USE_CVGIO

protected:
    //! Restrict an angle
    double AngleRestrict( double angle );
    //! Restrict the angle of a vector, keeping the vectors length
    void AngleRestrictVectorRot( double* vecx, double* vecy );
    //! Restrict the angle of a vector, keeping the vectors projection on the orthogonal of another vector
    void AngleRestrictVectorSkew( double* vecx, double* vecy, double otherx, double othery );

    //! Rational restrict an angle
    double RationalRestrict( double angle );
    //! Rational restrict the angle of a vector
    void RationalRestrictVector( double* vecx, double* vecy );

    EPositionSnapModes m_posModesX;  //!< ored EPositionSnapModes of enabled modes for x
    EPositionSnapModes m_posModesY;  //!< ored EPositionSnapModes of enabled modes for y
    ESizeSnapModes m_sizeModesX;     //!< ored ESizeSnapModes of enabled modes for x
    ESizeSnapModes m_sizeModesY;     //!< ored ESizeSnapModes of enabled modes for y
    ERotationSnapModes m_rotModes;   //!< ored ERotationSnapModes of enabled modes
    double m_snapDistX;      //!< horizontal position snapping grid distance
    double m_snapDistY;      //!< vertical position snapping grid distance
    double m_originX;   //!< horizontal grid snapping origin vs. coordinate origin
    double m_originY;   //!< vertical grid snapping origins vs. coordinate origin
    double m_sizeX;     //!< horizontal size snapping grid distance
    double m_sizeY;     //!< vertical size snapping grid distance
    double m_minSizeX;  //!< minimal horiontal size
    double m_minSizeY;  //!< minimal vertical size
    double m_maxSizeX;  //!< maximal horiontal size
    double m_maxSizeY;  //!< maximal vertical size
    double m_rotationAngle;      //!< angle snapping angle in degrees
    wxUint32 m_rotationRationalNom; //!< bit map of possible nominators (0..31) for rational angle snapping
    wxUint32 m_rotationRationalDen; //!< bit map of possible denominators (0..31) for rational angle snapping
    double* m_angleList;  //!< sorted list of allowed rational angles
    int m_nAngleList;     //!< number of allowed rational angles

    double m_docSnapDistX;
    double m_docSnapDistY;
    double m_docOriginX;
    double m_docOriginY;

};

#endif // __RESTRICT_H__
