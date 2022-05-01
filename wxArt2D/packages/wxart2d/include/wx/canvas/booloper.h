/*! \file wx/canvas/booloper.h
    \brief boolean operation on primitives in a a2dDocument.


    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: booloper.h,v 1.19 2009/04/30 14:21:44 titato Exp $
*/

#ifndef __WXBOOLOPER_H__
#define __WXBOOLOPER_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "a2dprivate.h"

#if wxART2D_USE_KBOOL

#include "kbool/booleng.h"
#include "wx/artbase/artmod.h"
#include "wx/canvas/algos.h"

//! walk a document tree and perform boolean opration on primitives.
/*!
    For each object fitting the group and mask, and is at the same child level,
    can be merged with other objects at that same level.
    Only if the object can be converted to polygons, this will be the case.
    For new polygons created this way, the BIN2 flags is set, so one is able to find them.
    The reuslting polygons are added to the target layer.

    \ingroup canvasobject
*/
class A2DCANVASDLLEXP  a2dBooleanWalkerHandler
    : public a2dWalker_LayerCanvasObjects
{
public:

    static const long Boolean_NON; /*!< No operation */
    static const long Boolean_OR; /*!< boolean OR operation */
    static const long Boolean_AND; /*!< boolean AND operation */
    static const long Boolean_EXOR; /*!< boolean EX_OR operation */
    static const long Boolean_A_SUB_B; /*!< boolean Group A - Group B operation */
    static const long Boolean_B_SUB_A; /*!< boolean Group B - Group A operation */
    static const long Boolean_CORRECTION; /*!< polygon correction/offset operation */
    static const long Boolean_SMOOTHEN; /*!< smooth operation */
    static const long Boolean_MAKERING; /*!< create a ring on all polygons */
    static const long Boolean_Polygon2Surface; /*!< convert selected polygon shapes to surfaces */
    static const long Boolean_Surface2Polygon; /*!< convert selected surfaces shapes to polygons */

    a2dBooleanWalkerHandler( a2dCanvasObjectFlagsMask mask = a2dCanvasOFlags::ALL );

    ~a2dBooleanWalkerHandler();

    kbBool_Engine* GetBooleanEngine() { return m_booleng; }

    //!log file will be created when set True
    void SetLog( bool OnOff ) { m_booleng->SetLog( OnOff ); }

    //!when set debug information and output will be generated.
    void SetDebug( bool OnOff )  { m_booleng->SetDebug( OnOff ); }

    void Initialize();

    //! clear the target layer before adding result.
    void SetClearTarget( bool clearTarget ) { m_clearTarget = clearTarget; }

    void SetMarge( double marge ) { m_booleng->SetMarge( marge ); }
    void SetGrid( long grid ) { m_booleng->SetGrid( grid ); }
    void SetDGrid( double dgrid ) { m_booleng->SetDGrid( dgrid ); }
    void SetCorrectionAber( double aber ) { m_booleng->SetCorrectionAber( aber ); }
    void SetCorrectionFactor( double aber ) { m_booleng->SetCorrectionFactor( aber ); }
    void SetMaxlinemerge( double maxline ) { m_booleng->SetMaxlinemerge( maxline ); }
    void SetWindingRule( bool rule ) { m_booleng->SetWindingRule( rule ); }
    void SetRoundfactor( double roundfac ) { m_booleng->SetRoundfactor( roundfac ); }
    void SetSmoothAber( double aber ) { m_booleng->SetSmoothAber( aber ); }
    //! If set false a2dSurfaces will be produced, meaning holes stored seperately
    //! If true, the result will be a2dPolygonL objects, with holes linked in to outer contour.
    //! Only if operation is Boolean_Polygon2Surface, linked holes is ignored, and holes are not linked.
    void SetLinkHoles( bool doLinkHoles ) { m_booleng->SetLinkHoles( doLinkHoles ); }
    void SetOrientationEntryMode( bool orientationEntryMode ) { m_booleng->SetOrientationEntryMode( orientationEntryMode ); }
    //! used in conversion from arc segment to polygon, defined in document database coordinates
    void SetAberArcToPoly( double aber ) { m_AberArcToPoly = aber; }

    //! when an object takes part in operation, and this flag is true, the original is removed.
    void SetReleaseOrignals( bool releaseOrignals ) { m_releaseOrignals = releaseOrignals; }

    //! if set, the result will be placed on same layer as the first object in the operation
    void SetResultOnSameLayer( bool resultOnSameLayer ) { m_resultOnSameLayer = resultOnSameLayer; }

    //! Set the Result stroke style
    /*!
        When set ( non wxNullStroke ), the style will be used for polygons
        resulting from a boolean operation.
    */
    void SetResultStroke( const a2dStroke& resultStroke ) { m_resultStroke = resultStroke; }

    //! Set the Result fill style
    /*!
        When set ( non wxNullFill ), the style will be used for polygons
        resulting from a boolean operation.
    */
    void SetResultFill( const a2dFill& resultFill ) { m_resultFill = resultFill; }

    //! when set the style of the resulting polygons
    /*!
        will be the same as the first child (being part of the operation) found in a parent object.
        Unless m_resultStroke or m_resultFill or set.
    */
    void SetStyleOfFirstChild( bool useStyle ) { m_useFirstChildStyle = useStyle; }

    //! called for boolean operation
    /*!
        All child objects of this parent and below, will be converted to polygons,
        and next a boolean operation will be applied.
    */
    virtual bool Start( a2dCanvasObject* object );

    //! called from within a2dCanvasDocument
    bool WalkTask( wxObject* parent, wxObject* object, a2dWalkEvent event );

protected:

    bool CallEngine( a2dCanvasObject* canobjToAddChildren );

    //! search objects that are convertable to Surface and add them to m_foundA
    bool GetObjectsForSurface( a2dCanvasObject* booleanobject, GroupType A_or_B );

    //! search objects that are Surface and add them to m_foundA
    bool GetObjectsSurface( a2dCanvasObject* booleanobject, GroupType A_or_B );

    //! vector path split ino polygons, and added to engine
    bool VectorPathPolygonsToEngine( a2dCanvasObjectList* vpath, GroupType A_or_B );

    //! convert obj to polygons and add to m_foundA and m_foundB
    bool ConvertToPolygons( a2dCanvasObject* booleanobject, GroupType A_or_B );

    //! the boolean engine
    kbBool_Engine* m_booleng;

    //! \see resultStroke
    a2dStroke m_resultStroke;

    //! \see resultFill
    a2dFill m_resultFill;

    //! use  first child style or not.
    bool m_useFirstChildStyle;

    //! clear the target layer before adding result.
    bool m_clearTarget;

    //! when an object takes part in operation, and this flag is true, the original is removed.
    bool m_releaseOrignals;

    //! objects found for groupA
    a2dCanvasObjectList m_foundA;

    //! objects found for groupB
    a2dCanvasObjectList m_foundB;

    //! to keep track of levels to do boolean operations
    wxUint16 m_lastdepth;

    a2dStroke m_useFirstChildStyleStroke;

    a2dFill   m_useFirstChildStyleFill;

    wxUint16 m_useFirstChildLayer;

    bool m_firstLevelChild;

    bool m_resultOnSameLayer;

    double m_AberArcToPoly;

    //DECLARE_CLASS(a2dBooleanWalkerHandler)
};

#endif //wxART2D_USE_KBOOL

#endif

