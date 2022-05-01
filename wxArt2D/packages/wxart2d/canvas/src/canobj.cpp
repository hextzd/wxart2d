/*! \file canvas/src/canobj.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canobj.cpp,v 1.435 2009/09/30 18:38:57 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/module.h"

#include <wx/wfstream.h>
#include <wx/tokenzr.h>

#include <algorithm>
#include <math.h>

#include "wx/canvas/canobj.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/canvas.h"
#include "wx/canvas/tools.h"
#include "wx/canvas/wire.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif


//#include "wx/general/id.inl"
#include "wx/general/smrtptr.inl"

//#define CANVASDEBUG
//#define _DEBUG_REPORTHIT

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

#define CIRCLE_STEPS 128

#if (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))

// MSVC warning 4660 is quite stupid. It says that the template is already instantiated
// by using it, but it is not fully instantiated as required for a library
#ifdef _MSC_VER
#pragma warning(disable: 4660)
#endif

template class a2dPropertyIdTyped<a2dCanvasObjectPtr, class a2dCanvasObjectPtrProperty>;
template class a2dPropertyIdTyped<a2dBoundingBox, a2dBoudingBoxProperty>;


#ifdef _MSC_VER
#pragma warning(default: 4660)
#endif

#endif // (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))

#ifdef _DEBUG
// Here are two globals that can be used as registers in the debugger
a2dCanvasObject* _dbco1 = 0;
a2dCanvasObject* _dbco2 = 0;
#endif

//----------------------------------------------------------------------------
// a2dCanvasObject
//----------------------------------------------------------------------------

bool operator < ( const a2dSmrtPtr<a2dCanvasObject>& a, const a2dSmrtPtr<a2dCanvasObject>& b )
{
    return ( *( s_a2dCanvasObjectSorter ) ) ( a, b );
}

a2dCanvasObjectSorter s_a2dCanvasObjectSorter = NULL;

bool a2dCanvasObject::m_ignoreAllSetpending = false;

a2dPropertyIdMatrix* a2dCanvasObject::PROPID_TransformMatrix = NULL;
a2dPropertyIdPoint2D* a2dCanvasObject::PROPID_Position = NULL;
a2dPropertyIdUint16* a2dCanvasObject::PROPID_Layer = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Selected = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Selectable = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_SubEdit = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_SubEditAsChild = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Visible = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Draggable = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Showshadow = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Filled = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_GroupA = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_GroupB = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_GeneratePins = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Bin = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Bin2 = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Pending = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Snap = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_SnapTo = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Pushin = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Prerenderaschild = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Visiblechilds = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Editable = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Editing = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_EditingRender = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_ChildrenOnSameLayer = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_DoConnect = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_IsOnCorridorPath = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_HasPins = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_IsProperty = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_MouseInObject = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_HighLight = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Template = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_External = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Used = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Release = NULL;
a2dPropertyIdCanvasObject* a2dCanvasObject::PROPID_Begin = NULL;
a2dPropertyIdCanvasObject* a2dCanvasObject::PROPID_End = NULL;
a2dPropertyIdDouble* a2dCanvasObject::PROPID_EndScaleX = NULL;
a2dPropertyIdDouble* a2dCanvasObject::PROPID_EndScaleY = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Spline = NULL;
a2dPropertyIdDouble* a2dCanvasObject::PROPID_ContourWidth = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_DisableFeedback = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Allowrotation = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Allowsizing = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_Allowskew = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_SkipBase = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_IncludeChildren = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_preserveAspectRatio = NULL;
a2dPropertyIdRefObjectAutoZero* a2dCanvasObject::PROPID_Controller = NULL;
a2dPropertyIdCanvasObject* a2dCanvasObject::PROPID_Original = NULL;
a2dPropertyIdCanvasObject* a2dCanvasObject::PROPID_Editcopy = NULL;
a2dPropertyIdCanvasObject* a2dCanvasObject::PROPID_Parent = NULL;
a2dPropertyIdCanvasObject* a2dCanvasObject::PROPID_Objecttip = NULL;
a2dPropertyIdUint16* a2dCanvasObject::PROPID_Editmode = NULL;
a2dPropertyIdUint16* a2dCanvasObject::PROPID_Editstyle = NULL;
a2dPropertyIdUint16* a2dCanvasObject::PROPID_Index = NULL;
a2dPropertyIdCanvasShadowStyle* a2dCanvasObject::PROPID_Shadowstyle = NULL;
a2dPropertyIdFill* a2dCanvasObject::PROPID_Fill = NULL;
a2dPropertyIdStroke* a2dCanvasObject::PROPID_Stroke = NULL;
a2dPropertyIdUint32* a2dCanvasObject::PROPID_RefDesCount = NULL;
a2dPropertyIdUint32* a2dCanvasObject::PROPID_RefDesNr = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_ToolDecoration = NULL;
a2dPropertyIdVoidPtr* a2dCanvasObject::PROPID_ToolObject = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_TemporaryObject = NULL;
a2dPropertyIdBoundingBox* a2dCanvasObject::PROPID_BoundingBox = NULL;
a2dPropertyIdColour* a2dCanvasObject::PROPID_StrokeColour = NULL;
a2dPropertyIdColour* a2dCanvasObject::PROPID_FillColour = NULL;
a2dPropertyIdRefObject* a2dCanvasObject::PROPID_ViewSpecific = NULL;
a2dPropertyIdBool* a2dCanvasObject::PROPID_FirstEventInObject = NULL;
a2dPropertyIdUint16* a2dCanvasObject::PROPID_Datatype = NULL;
a2dPropertyIdRefObject* a2dCanvasObject::PROPID_ViewDependent = NULL;
a2dPropertyIdMatrix* a2dCanvasObject::PROPID_IntViewDependTransform = NULL;
a2dPropertyIdDateTime* a2dCanvasObject::PROPID_DateTime = NULL;
a2dPropertyIdDateTime* a2dCanvasObject::PROPID_ModificationDateTime = NULL;
a2dPropertyIdDateTime* a2dCanvasObject::PROPID_AccessDateTime = NULL;
a2dPropertyIdMenu* a2dCanvasObject::PROPID_PopupMenu = NULL;
a2dPropertyIdWindow* a2dCanvasObject::PROPID_TipWindow = NULL;
a2dPropertyIdTagVec* a2dCanvasObject::PROPID_Tags = NULL;

//INITIALIZE_PROPERTIES( a2dCanvasObject, a2dObject )
a2dDynamicIdMap& a2dCanvasObject::GetPropertyIdMap()
{
    return sm_dymPropIds;
}
bool a2dCanvasObject::AddPropertyId( a2dPropertyId* dynproperty )
{
    if ( sm_dymPropIds.find( dynproperty->GetName() ) == sm_dymPropIds.end() )
        sm_dymPropIds[ dynproperty->GetName() ] = dynproperty;
    else
        wxASSERT_MSG( 0, _( "The property id name '" ) + dynproperty->GetName() + _( "' already exists in #a2dCanvasObject" ) );
    return true;
}
bool a2dCanvasObject::HasPropertyId( const a2dPropertyId* id ) const
{
    a2dDynamicIdMap::iterator iter = sm_dymPropIds.find( id->GetName() );
    if ( iter != sm_dymPropIds.end() )
        return true;
    return a2dObject::HasPropertyId( id );
}
a2dPropertyId* a2dCanvasObject::HasPropertyId( const wxString& name )
{
    a2dDynamicIdMap::iterator iter = sm_dymPropIds.find( name );
    if ( iter != sm_dymPropIds.end() )
        return sm_dymPropIds[ name ];

    return a2dObject::HasPropertyId( name );
}
const a2dNamedProperty* a2dCanvasObject::FindProperty( const wxString& idName ) const
{
    a2dDynamicIdMap::iterator iter = sm_dymPropIds.find( idName );
    if ( iter != sm_dymPropIds.end() )
    {
        a2dPropertyIdPtr propId = ( *iter ).second;
        /* if ( propId->GetName() == idName ) //assume right */
            return GetProperty( propId );
    }
    return a2dObject::FindProperty( idName );
}
void a2dCanvasObject::CollectProperties2( a2dNamedPropertyList* total, const a2dPropertyId* id, a2dPropertyId::Flags flags ) const
{
    for ( a2dDynamicIdMap::iterator i = sm_dymPropIds.begin(); i != sm_dymPropIds.end(); i++ )
    {
        //wxString name = ( *i ).first;
        a2dPropertyIdPtr p = ( *i ).second;
        if( p->CheckCollect( id, flags ) )
        {
            a2dNamedProperty* dprop = p->GetPropertyAsNamedProperty( this );
            if ( dprop )
                total->push_back( dprop );
        }
    }
    return a2dObject::CollectProperties2( total, id, flags );
}
a2dDynamicIdMap a2dCanvasObject::sm_dymPropIds;
static bool initPropa2dCanvasObject = a2dCanvasObject::InitializePropertyIds();
bool a2dCanvasObject::InitializePropertyIds()
{
    PROPID_TransformMatrix = new a2dPropertyIdMatrix(  wxT( "TransformMatrix" ),
            a2dPropertyId::flag_none, a2dIDENTITY_MATRIX,
            a2dPropertyIdMatrix::Get( &a2dCanvasObject::GetTransform ),
            a2dPropertyIdMatrix::Set( &a2dCanvasObject::SetTransform ) );
    AddPropertyId( PROPID_TransformMatrix );

    PROPID_Position = new a2dPropertyIdPoint2D(  wxT( "Position" ),
            a2dPropertyId::flag_none, a2dPoint2D( 0, 0 ),
            a2dPropertyIdPoint2D::Get( &a2dCanvasObject::GetPosXY ),
            a2dPropertyIdPoint2D::ConstSet( &a2dCanvasObject::SetPosXyPoint ) );
    AddPropertyId( PROPID_Position );

    PROPID_Layer = new a2dPropertyIdUint16(  wxT( "Layer" ),
            a2dPropertyId::flag_none, 0,
            a2dPropertyIdUint16::Get( &a2dCanvasObject::GetLayer ),
            a2dPropertyIdUint16::Set( &a2dCanvasObject::SetLayer ) );
    AddPropertyId( PROPID_Layer );

    PROPID_Selected = new a2dPropertyIdBool(  wxT( "Selected" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetSelected ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetSelected ) );
    AddPropertyId( PROPID_Selected );

    PROPID_Selectable = new a2dPropertyIdBool(  wxT( "Selectable" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetSelectable ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetSelectable ) );
    AddPropertyId( PROPID_Selectable );

    PROPID_SubEdit = new a2dPropertyIdBool(  wxT( "SubEdit" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetSubEdit ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetSubEdit ) );
    AddPropertyId( PROPID_SubEdit );

    PROPID_SubEditAsChild = new a2dPropertyIdBool(  wxT( "SubEditAsChild" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetSubEditAsChild ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetSubEditAsChild ) );
    AddPropertyId( PROPID_SubEditAsChild );

    PROPID_Visible = new a2dPropertyIdBool(  wxT( "Visible" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetVisible ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetVisible ) );
    AddPropertyId( PROPID_Visible );

    PROPID_Draggable = new a2dPropertyIdBool(  wxT( "Draggable" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetDraggable ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetDraggable ) );
    AddPropertyId( PROPID_Draggable );

    PROPID_Showshadow = new a2dPropertyIdBool(  wxT( "Showshadow" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetShowshadow ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetShowshadow ) );
    AddPropertyId( PROPID_Showshadow );

    PROPID_Filled = new a2dPropertyIdBool(  wxT( "Filled" ),
                                            a2dPropertyId::flag_none, false,
                                            a2dPropertyIdBool::Get( &a2dCanvasObject::GetFilled ),
                                            a2dPropertyIdBool::Set( &a2dCanvasObject::SetFilled ) );
    AddPropertyId( PROPID_Filled );

    PROPID_GroupA = new a2dPropertyIdBool(  wxT( "GroupA" ),
                                            a2dPropertyId::flag_none, false,
                                            a2dPropertyIdBool::Get( &a2dCanvasObject::GetGroupA ),
                                            a2dPropertyIdBool::Set( &a2dCanvasObject::SetGroupA ) );
    AddPropertyId( PROPID_GroupA );

    PROPID_GroupB = new a2dPropertyIdBool(  wxT( "GroupB" ),
                                            a2dPropertyId::flag_none, false,
                                            a2dPropertyIdBool::Get( &a2dCanvasObject::GetGroupB ),
                                            a2dPropertyIdBool::Set( &a2dCanvasObject::SetGroupB ) );
    AddPropertyId( PROPID_GroupB );

    PROPID_GeneratePins = new a2dPropertyIdBool(  wxT( "GeneratePins" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetGeneratePins ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetGeneratePins ) );
    AddPropertyId( PROPID_GeneratePins );

    PROPID_Bin = new a2dPropertyIdBool(  wxT( "Bin" ),
                                         a2dPropertyId::flag_none, false,
                                         a2dPropertyIdBool::Get( &a2dCanvasObject::GetBin ),
                                         a2dPropertyIdBool::Set( &a2dCanvasObject::SetBin ) );
    AddPropertyId( PROPID_Bin );

    PROPID_Bin2 = new a2dPropertyIdBool(  wxT( "Bin2" ),
                                          a2dPropertyId::flag_none, false,
                                          a2dPropertyIdBool::Get( &a2dCanvasObject::GetBin2 ),
                                          a2dPropertyIdBool::Set( &a2dCanvasObject::SetBin2 ) );
    AddPropertyId( PROPID_Bin2 );

    PROPID_Pending = new a2dPropertyIdBool(  wxT( "Pending" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetPending ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetPending ) );
    AddPropertyId( PROPID_Pending );

    PROPID_Snap = new a2dPropertyIdBool(  wxT( "Snap" ),
                                          a2dPropertyId::flag_none, false,
                                          a2dPropertyIdBool::Get( &a2dCanvasObject::GetSnap ),
                                          a2dPropertyIdBool::Set( &a2dCanvasObject::SetSnap ) );
    AddPropertyId( PROPID_Snap );

    PROPID_SnapTo = new a2dPropertyIdBool(  wxT( "SnapTo" ),
                                            a2dPropertyId::flag_none, false,
                                            a2dPropertyIdBool::Get( &a2dCanvasObject::GetSnapTo ),
                                            a2dPropertyIdBool::Set( &a2dCanvasObject::SetSnapTo ) );
    AddPropertyId( PROPID_SnapTo );

    PROPID_Pushin = new a2dPropertyIdBool(  wxT( "Pushin" ),
                                            a2dPropertyId::flag_none, false,
                                            a2dPropertyIdBool::Get( &a2dCanvasObject::GetPushin ),
                                            a2dPropertyIdBool::Set( &a2dCanvasObject::SetPushin ) );
    AddPropertyId( PROPID_Pushin );

    PROPID_Prerenderaschild = new a2dPropertyIdBool(  wxT( "Prerenderaschild" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetPrerenderaschild ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetPrerenderaschild ) );
    AddPropertyId( PROPID_Prerenderaschild );

    PROPID_Visiblechilds = new a2dPropertyIdBool(  wxT( "Visiblechilds" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetVisiblechilds ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetVisiblechilds ) );
    AddPropertyId( PROPID_Visiblechilds );

    PROPID_Editable = new a2dPropertyIdBool(  wxT( "Editable" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetEditable ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetEditable ) );
    AddPropertyId( PROPID_Editable );

    PROPID_Editing = new a2dPropertyIdBool(  wxT( "Editing" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetEditing ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetEditing ) );
    AddPropertyId( PROPID_Editing );

    PROPID_EditingRender = new a2dPropertyIdBool(  wxT( "EditingRender" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetEditingRender ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetEditingRender ) );
    AddPropertyId( PROPID_EditingRender );

    PROPID_ChildrenOnSameLayer = new a2dPropertyIdBool(  wxT( "ChildrenOnSameLayer" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetChildrenOnSameLayer ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetChildrenOnSameLayer ) );
    AddPropertyId( PROPID_ChildrenOnSameLayer );

    PROPID_DoConnect = new a2dPropertyIdBool(  wxT( "DoConnect" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetDoConnect ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetDoConnect ) );
    AddPropertyId( PROPID_DoConnect );

    PROPID_IsOnCorridorPath = new a2dPropertyIdBool(  wxT( "IsOnCorridorPath" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetIsOnCorridorPath ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetIsOnCorridorPath ) );
    AddPropertyId( PROPID_IsOnCorridorPath );

    PROPID_HasPins = new a2dPropertyIdBool(  wxT( "HasPins" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetHasPins ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetHasPins ) );
    AddPropertyId( PROPID_HasPins );

    PROPID_IsProperty = new a2dPropertyIdBool(  wxT( "IsProperty" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetIsProperty ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetIsProperty ) );
    AddPropertyId( PROPID_IsProperty );

    PROPID_MouseInObject = new a2dPropertyIdBool(  wxT( "MouseInObject" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetMouseInObject ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetMouseInObject ) );
    AddPropertyId( PROPID_MouseInObject );

    PROPID_HighLight = new a2dPropertyIdBool(  wxT( "HighLight" ),
            a2dPropertyId::flag_notify, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetHighLight ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetHighLight ) );
    AddPropertyId( PROPID_HighLight );

    PROPID_Template = new a2dPropertyIdBool(  wxT( "Template" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetTemplate ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetTemplate ) );
    AddPropertyId( PROPID_Template );

    PROPID_External = new a2dPropertyIdBool(  wxT( "External" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetExternal ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetExternal ) );
    AddPropertyId( PROPID_External );

    PROPID_Used = new a2dPropertyIdBool(  wxT( "Used" ),
                                          a2dPropertyId::flag_none, false,
                                          a2dPropertyIdBool::Get( &a2dCanvasObject::GetUsed ),
                                          a2dPropertyIdBool::Set( &a2dCanvasObject::SetUsed ) );
    AddPropertyId( PROPID_Used );

    PROPID_Release = new a2dPropertyIdBool(  wxT( "Release" ),
            a2dPropertyId::flag_none, false,
            a2dPropertyIdBool::Get( &a2dCanvasObject::GetRelease ),
            a2dPropertyIdBool::Set( &a2dCanvasObject::SetRelease ) );
    AddPropertyId( PROPID_Release );

    PROPID_DisableFeedback = new a2dPropertyIdBool(  wxT( "DisableFeedback" ),
            a2dPropertyId::flag_none, false );
    AddPropertyId( PROPID_DisableFeedback );

    PROPID_Allowrotation = new a2dPropertyIdBool(  wxT( "Allowrotation" ),
            a2dPropertyId::flag_none, false );
    AddPropertyId( PROPID_Allowrotation );

    PROPID_preserveAspectRatio = new a2dPropertyIdBool(  wxT( "preserveAspectRatio" ),
            a2dPropertyId::flag_none, false );
    AddPropertyId( PROPID_preserveAspectRatio );

    PROPID_Allowsizing = new a2dPropertyIdBool(  wxT( "Allowsizing" ),
            a2dPropertyId::flag_none, false );
    AddPropertyId( PROPID_Allowsizing );

    PROPID_Allowskew = new a2dPropertyIdBool(  wxT( "Allowskew" ),
            a2dPropertyId::flag_none, false );
    AddPropertyId( PROPID_Allowskew );

    PROPID_SkipBase = new a2dPropertyIdBool(  wxT( "SkipBase" ),
            a2dPropertyId::flag_none, false );
    AddPropertyId( PROPID_SkipBase );

    PROPID_IncludeChildren = new a2dPropertyIdBool(  wxT( "IncludeChildren" ),
            a2dPropertyId::flag_none, false );
    AddPropertyId( PROPID_IncludeChildren );

    //! set for objects that act as tool decorations, when a tool is in action.
    PROPID_ToolDecoration = new a2dPropertyIdBool(  wxT( "ToolDecoration" ),
            a2dPropertyId::flag_none, false );
    AddPropertyId( PROPID_ToolDecoration );

    //! set for objects that do not have to be saved
    PROPID_TemporaryObject = new a2dPropertyIdBool(  wxT( "TemporaryObject" ),
            a2dPropertyId::flag_none, false );
    AddPropertyId( PROPID_TemporaryObject );

    //! set in Startedit(), to be used to detect first (mouse)event sent to object.
    PROPID_FirstEventInObject = new a2dPropertyIdBool(  wxT( "FirstEventInObject" ),
            a2dPropertyId::flag_none, false );
    AddPropertyId( PROPID_FirstEventInObject );

    PROPID_Begin = new a2dPropertyIdCanvasObject(  wxT( "Begin" ),
            a2dPropertyId::flag_transfer | a2dPropertyId::flag_onlymemberhint, 0 );
    AddPropertyId( PROPID_Begin );

    PROPID_End = new a2dPropertyIdCanvasObject(  wxT( "End" ),
            a2dPropertyId::flag_transfer | a2dPropertyId::flag_onlymemberhint, 0 );
    AddPropertyId( PROPID_End );

    PROPID_EndScaleX = new a2dPropertyIdDouble(  wxT( "EndScaleX" ),
            a2dPropertyId::flag_transfer | a2dPropertyId::flag_onlymemberhint, 0 );
    AddPropertyId( PROPID_EndScaleX );

    PROPID_EndScaleY = new a2dPropertyIdDouble(  wxT( "EndScaleY" ),
            a2dPropertyId::flag_transfer | a2dPropertyId::flag_onlymemberhint, 0 );
    AddPropertyId( PROPID_EndScaleY );

    PROPID_Spline = new a2dPropertyIdBool(  wxT( "Spline" ),
                                            a2dPropertyId::flag_transfer | a2dPropertyId::flag_onlymemberhint, false );
    AddPropertyId( PROPID_Spline );

    PROPID_ContourWidth = new a2dPropertyIdDouble(  wxT( "ContourWidth" ),
            a2dPropertyId::flag_none, 0,
            a2dPropertyIdDouble::Get( &a2dCanvasObject::GetContourWidth ),
            a2dPropertyIdDouble::Set( &a2dCanvasObject::SetContourWidth ) );
    AddPropertyId( PROPID_ContourWidth );

    PROPID_Original = new a2dPropertyIdCanvasObject(  wxT( "Original" ),
            (a2dPropertyId::Flags) (a2dPropertyId::flag_temporary & ~a2dPropertyId::flag_clonedeep), 0 );
    AddPropertyId( PROPID_Original );

    PROPID_Editcopy = new a2dPropertyIdCanvasObject(  wxT( "Editcopy" ),
            (a2dPropertyId::Flags) (a2dPropertyId::flag_temporary & ~a2dPropertyId::flag_clonedeep), 0 );
    AddPropertyId( PROPID_Editcopy );

    PROPID_Parent = new a2dPropertyIdCanvasObject(  wxT( "Parent" ),
            (a2dPropertyId::Flags) (a2dPropertyId::flag_temporary & ~a2dPropertyId::flag_clonedeep), 0 );
    AddPropertyId( PROPID_Parent );

    PROPID_Objecttip = new a2dPropertyIdCanvasObject(  wxT( "Objecttip" ),
            a2dPropertyId::flag_notify | a2dPropertyId::flag_temporary, 0 );
    AddPropertyId( PROPID_Objecttip );

    PROPID_Controller = new a2dPropertyIdRefObjectAutoZero(  wxT( "Controller" ),
            a2dPropertyId::flag_temporary, 0 );
    AddPropertyId( PROPID_Controller );

    PROPID_Editmode = new a2dPropertyIdUint16(  wxT( "Editmode" ),
            a2dPropertyId::flag_none, 0 );
    AddPropertyId( PROPID_Editmode );

    PROPID_Editstyle = new a2dPropertyIdUint16(  wxT( "Editstyle" ),
            a2dPropertyId::flag_none, 0 );
    AddPropertyId( PROPID_Editstyle );

    PROPID_Index = new a2dPropertyIdUint16(  wxT( "Index" ),
            a2dPropertyId::flag_none, 0 );
    AddPropertyId( PROPID_Index );
    PROPID_Index = PROPID_Index;

    PROPID_RefDesCount = new a2dPropertyIdUint32(  wxT( "RefDesCount" ),
            a2dPropertyId::flag_none, 0 );
    AddPropertyId( PROPID_RefDesCount );

    PROPID_RefDesNr = new a2dPropertyIdUint32(  wxT( "RefDesNr" ),
            a2dPropertyId::flag_none, 0 );
    AddPropertyId( PROPID_RefDesNr );

    PROPID_Shadowstyle = new a2dPropertyIdCanvasShadowStyle(  wxT( "Shadowstyle" ),
            a2dPropertyId::flag_notify );
    AddPropertyId( PROPID_Shadowstyle );

    PROPID_Fill = new a2dPropertyIdFill(  wxT( "Fill" ),
                                          a2dPropertyId::flag_notify | a2dPropertyId::flag_transfer | a2dPropertyId::flag_listfirst | a2dPropertyId::flag_clonedeep,
                                          *a2dNullFILL, static_cast < a2dPropertyIdFill::ConstGet >( &a2dCanvasObject::MX_GetFill ), static_cast < a2dPropertyIdFill::ConstSet >( &a2dCanvasObject::MX_SetFill ) );
    AddPropertyId( PROPID_Fill );

    PROPID_Stroke = new a2dPropertyIdStroke(  wxT( "Stroke" ),
            a2dPropertyId::flag_notify | a2dPropertyId::flag_transfer | a2dPropertyId::flag_listfirst | a2dPropertyId::flag_clonedeep,
            *a2dNullSTROKE, static_cast < a2dPropertyIdStroke::ConstGet >( &a2dCanvasObject::MX_GetStroke ), static_cast < a2dPropertyIdStroke::ConstSet >( &a2dCanvasObject::MX_SetStroke ) );
    AddPropertyId( PROPID_Stroke );

    PROPID_ToolObject = new a2dPropertyIdVoidPtr(  wxT( "ToolObject" ),
            a2dPropertyId::flag_none | a2dPropertyId::flag_temporary, 0 );
    AddPropertyId( PROPID_ToolObject );

    PROPID_BoundingBox = new a2dPropertyIdBoundingBox(  wxT( "BoundingBox" ),
            a2dPropertyId::flag_temporary | a2dPropertyId::flag_multiple, a2dBoundingBox() );
    AddPropertyId( PROPID_BoundingBox );

    PROPID_FillColour = new a2dPropertyIdColour(  wxT( "FillColour" ), a2dPropertyId::flag_none, wxColour(),
            static_cast < a2dPropertyIdColour::Get >( &a2dCanvasObject::GetFillColour ), static_cast < a2dPropertyIdColour::ConstSet >( &a2dCanvasObject::SetFillColour ) );
    AddPropertyId( PROPID_FillColour );

    PROPID_StrokeColour = new a2dPropertyIdColour(  wxT( "StrokeColour" ), a2dPropertyId::flag_none, wxColour(),
            static_cast < a2dPropertyIdColour::Get >( &a2dCanvasObject::GetStrokeColour ), static_cast < a2dPropertyIdColour::ConstSet >( &a2dCanvasObject::SetStrokeColour ) );
    AddPropertyId( PROPID_StrokeColour );

    PROPID_ViewSpecific = new a2dPropertyIdRefObject(  wxT( "ViewSpecific" ), a2dPropertyId::flag_temporary | a2dPropertyId::flag_listonly, 0 );
    AddPropertyId( PROPID_ViewSpecific );

    PROPID_ViewDependent = new a2dPropertyIdRefObject(  wxT( "ViewDependent" ), a2dPropertyId::flag_temporary | a2dPropertyId::flag_listonly, 0 );
    AddPropertyId( PROPID_ViewDependent );

    PROPID_IntViewDependTransform = new a2dPropertyIdMatrix(  wxT( "IntViewDependTransform" ), a2dPropertyId::flag_none, a2dIDENTITY_MATRIX );
    AddPropertyId( PROPID_IntViewDependTransform );

    PROPID_Datatype = new a2dPropertyIdUint16(  wxT( "Datatype" ), a2dPropertyId::flag_none, 0 );
    AddPropertyId( PROPID_Datatype );

    PROPID_DateTime = new a2dPropertyIdDateTime(  wxT( "DateTime" ),
            a2dPropertyId::flag_norender | a2dPropertyId::flag_listfirst | a2dPropertyId::flag_clonedeep, wxDefaultDateTime );
    AddPropertyId( PROPID_DateTime );

    PROPID_ModificationDateTime = new a2dPropertyIdDateTime(  wxT( "ModificationDateTime" ),
            a2dPropertyId::flag_norender | a2dPropertyId::flag_listfirst | a2dPropertyId::flag_clonedeep, wxDefaultDateTime );
    AddPropertyId( PROPID_ModificationDateTime );

    PROPID_AccessDateTime = new a2dPropertyIdDateTime(  wxT( "AccessDateTime" ),
            a2dPropertyId::flag_norender | a2dPropertyId::flag_listfirst | a2dPropertyId::flag_clonedeep, wxDefaultDateTime );
    AddPropertyId( PROPID_AccessDateTime );

    PROPID_PopupMenu = new a2dPropertyIdMenu(  wxT( "PopupMenu" ), a2dPropertyId::flag_listfirst | a2dPropertyId::flag_clonedeep, NULL );
    AddPropertyId( PROPID_PopupMenu );

    PROPID_TipWindow = new a2dPropertyIdWindow(  wxT( "TipWindow" ), a2dPropertyId::flag_notify | a2dPropertyId::flag_listfirst | a2dPropertyId::flag_clonedeep, NULL );
    AddPropertyId( PROPID_TipWindow );

    PROPID_Tags = new a2dPropertyIdTagVec(  wxT( "Tags" ), a2dPropertyId::flag_notify | a2dPropertyId::flag_listfirst | a2dPropertyId::flag_clonedeep, a2dTagVec() );
    AddPropertyId( PROPID_Tags );

    return true;
}

a2dCanvasOFlags a2dCanvasObject::m_flagsInit =
    a2dCanvasOFlags
    (
        a2dCanvasOFlags::VISIBLE | a2dCanvasOFlags::DRAGGABLE | a2dCanvasOFlags::EDITABLE | a2dCanvasOFlags::FILLED | a2dCanvasOFlags::SELECTABLE |
        a2dCanvasOFlags::SHOWSHADOW | a2dCanvasOFlags::VISIBLECHILDS | a2dCanvasOFlags::PUSHIN |
        a2dCanvasOFlags::DoConnect | a2dCanvasOFlags::SNAP | a2dCanvasOFlags::SNAP_TO | a2dCanvasOFlags::generatePins

    );

IMPLEMENT_DYNAMIC_CLASS( a2dCanvasObject, a2dObject )

BEGIN_EVENT_TABLE( a2dCanvasObject, a2dObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dCanvasObject::OnCanvasObjectMouseEvent )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dCanvasObject::OnHandleEvent )
    EVT_CANVASOBJECT_ENTER_EVENT( a2dCanvasObject::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT( a2dCanvasObject::OnLeaveObject )
    EVT_CANVASOBJECT_POPUPMENU_EVENT( a2dCanvasObject::OnPopUpEvent )
    //EVT_COM_EVENT_ID( a2dComEvent::sm_changedProperty, a2dCanvasObject::OnPropertyChanged )
    EVT_COM_EVENT( a2dCanvasObject::OnPropertyChanged )
    EVT_CHAR( a2dCanvasObject::OnChar )
END_EVENT_TABLE()

a2dCanvasObject::a2dCanvasObject( double x, double y )
{
    m_lworld.Translate( x, y );
    //m_bflags[a2dCanvasOFlags::HasSelectedObjectsBelow]=true;
    //if ( m_bflags[a2dCanvasOFlags::HasSelectedObjectsBelow] )
    //    m_childobjects = wxNullCanvasObjectList;


    m_childobjects = wxNullCanvasObjectList;


    SetHitFlags( a2dCANOBJ_EVENT_VISIBLE );

    m_root = NULL;

    m_flags = m_flagsInit;

    m_layer = wxLAYER_DEFAULT;

    //each new object is pending
    SetPending( true );

    m_worldExtend = 0;
    m_pixelExtend = 0;
    m_shapeIdsValid = false;
}

a2dCanvasObject::~a2dCanvasObject()
{
    if ( m_childobjects != wxNullCanvasObjectList )
    {
        delete m_childobjects;
        m_childobjects = wxNullCanvasObjectList;
    }
}

a2dObject* a2dCanvasObject::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCanvasObject( *this, options, refs );
}

a2dCanvasObject::a2dCanvasObject( const a2dCanvasObject& other, CloneOptions options, a2dRefMap* refs )
    : a2dObject( other, options, refs )
{
    m_root = other.m_root;
    if( options & clone_resetRoot )
        m_root = NULL;

    m_flags = other.m_flags;

    m_hitflags = other.m_hitflags;

    m_flags.m_showshadow = true;

    m_layer = other.m_layer;

    m_lworld = other.m_lworld;

    m_childobjects = wxNullCanvasObjectList;
    if ( other.m_childobjects != wxNullCanvasObjectList )
    {
        m_childobjects = new a2dCanvasObjectList();

        forEachIn( a2dCanvasObjectList, other.m_childobjects )
        {
            a2dCanvasObject* obj = *iter;

            if ( obj && !obj->GetRelease() )
            {
                if( options & clone_childs )
                {
                    a2dCanvasObject* objn = obj->TClone( CloneOptions( options & ~ clone_seteditcopy ), refs );
                    //all object having a single parent, will get this as new parent.
                    // e.g a2dPin and a2dVisibleProperty
                    objn->SetParent( this );
                    m_childobjects->push_back( objn );
                }
                else
                {
                    m_childobjects->push_back( obj );
                }
            }
        }
    }

    if( options & clone_seteditcopy )
    {
        // The copy constructor is not supposed to change the original, but with clone_seteditcopy
        // it will add the editcopy property, so we must use a const_cast here
        PROPID_Editcopy->SetPropertyToObject( const_cast<a2dCanvasObject*>( &other ), this );
    }

    if( options & clone_setoriginal )
    {
        // There are no "const" properties, so we need some const_casting here
        PROPID_Original->SetPropertyToObject( this, const_cast<a2dCanvasObject*>( &other ) );
    }

    m_bbox = other.m_bbox;
    m_bbox.SetValid( false );

    m_shapeIdsValid = false;

    m_worldExtend = other.m_worldExtend;
    m_pixelExtend = other.m_pixelExtend;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in property->TClone()
    CurrentSmartPointerOwner = this;
#endif
}

a2dHabitat* a2dCanvasObject::GetHabitat() const
{
    if ( m_root )
        return m_root->GetHabitat();

    //return NULL;
    return a2dCanvasGlobals->GetHabitat();
}

a2dCanvasObjectList* a2dCanvasObject::GetAsCanvasVpaths( bool WXUNUSED( transform ) ) const
{
    return wxNullCanvasObjectList;
}

a2dCanvasObjectList* a2dCanvasObject::GetAsPolygons( bool transform ) const
{
    bool returnIsPolygon = true;
    a2dVertexList* segList = GetAsVertexList( returnIsPolygon );
    if ( segList && returnIsPolygon )
    {
        a2dCanvasObjectList* polylist = new a2dCanvasObjectList();
        polylist->push_back( new a2dPolygonL( segList ) );
        return polylist;
    }
    else if ( segList )
        delete segList;

    a2dCanvasObjectList* polylist = NULL;
    a2dCanvasObjectList* vpath = GetAsCanvasVpaths( true );

    // now we have vector path, which we will convert to polygons and polylines.
    // Next only the polygons will be used for boolean operations.
    if ( vpath != wxNullCanvasObjectList )
    {
        a2dCanvasObjectList::iterator iter = vpath->begin();
        while ( iter != vpath->end() )
        {
            a2dVectorPath* obj = ( a2dVectorPath* ) ( *iter ).Get();
            polylist = obj->GetAsPolygons();

            iter = vpath->erase( iter );
            iter = vpath->begin();
        }
        delete vpath;
        return polylist;
    }
    return wxNullCanvasObjectList;
}

a2dCanvasObjectList* a2dCanvasObject::GetAsPolylines( bool transform ) const
{
    bool returnIsPolygon = false;
    a2dVertexList* segList = GetAsVertexList( returnIsPolygon );
    if ( segList && returnIsPolygon )
    {
        a2dCanvasObjectList* polylist = new a2dCanvasObjectList();
        polylist->push_back( new a2dPolylineL( segList ) );
        return polylist;
    }
    else if ( segList )
    {
        a2dCanvasObjectList* polylist = new a2dCanvasObjectList();
        polylist->push_back( new a2dPolylineL( segList ) );
        return polylist;
    }


    a2dCanvasObjectList* polylist = NULL;
    a2dCanvasObjectList* vpath = GetAsCanvasVpaths( true );

    // now we have vector path, which we will convert to polygons and polylines.
    // Next only the polygons will be used for boolean operations.
    if ( vpath != wxNullCanvasObjectList )
    {
        a2dCanvasObjectList::iterator iter = vpath->begin();
        while ( iter != vpath->end() )
        {
            a2dVectorPath* obj = ( a2dVectorPath* ) ( *iter ).Get();
            polylist = obj->GetAsPolygons();

            iter = vpath->erase( iter );
            iter = vpath->begin();
        }
        delete vpath;
    }

    if ( !polylist )
        return wxNullCanvasObjectList;

    int i, count = polylist->size();
    a2dCanvasObjectList::iterator iterp = polylist->begin();
    for( i = 0 ; i < count ; i++ )
    {
        a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
        a2dPolylineL* polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
        if ( polyline && polyline->GetNumberOfSegments() > 2 )
        {
            iterp++;
        }
        else if ( poly && poly->GetNumberOfSegments() > 2 )
        {
            a2dPolylineL* conv = new a2dPolylineL( poly->GetSegments() );
            poly->SetLayer( m_layer );
            polylist->push_back( conv );
            iterp = polylist->erase( iterp );
        }
    }
    return polylist;
}

bool a2dCanvasObject::SetPosXYRestrict( double& x, double& y )
{
    double xold = m_lworld.GetValue( 2, 0 );
    double yold = m_lworld.GetValue( 2, 1 );
    if( m_flags.m_snap )
    {
        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();
        if ( restrictEngine )
            restrictEngine->RestrictPoint( x, y );
    }
    if( xold != x || yold != y )
    {
        m_lworld.SetValue( 2, 0, x ); m_lworld.SetValue( 2, 1, y ); SetPending( true );
        return true;
    }
    return false;
}

void a2dCanvasObject::SetPosXY( double x, double y, bool restrict )
{
    double xold = m_lworld.GetValue( 2, 0 );
    double yold = m_lworld.GetValue( 2, 1 );

    if( restrict )
    {
        SetPosXYRestrict( x, y );
    }

    if( xold != x || yold != y )
    {
        m_lworld.SetValue( 2, 0, x ); m_lworld.SetValue( 2, 1, y ); SetPending( true );
    }
}

a2dCanvasObject* a2dCanvasObject::StartEdit( a2dBaseTool* tool, wxUint16 editmode, wxEditStyle editstyle, a2dRefMap* refs )
{
    if ( m_flags.m_editable )
    {
        a2dCanvasObjectPtr editcopy;
        editcopy = TClone( clone_members | clone_childs | clone_seteditcopy | clone_setoriginal | clone_sameName /*| clone_noCameleonRef*/, refs );

        editcopy->DoConnect( false );
        SetSnapTo( false );
        editcopy->SetSnapTo( false );
        //preserve of selection is done in a2dObjectEditTool, to improve interactive editing selection should not be visible.
        // But if a dialog is shown instead, it can be overuled in derived DoStartEdit()
        SetSelected( false );
        editcopy->SetSelected( false );

        if ( ! HasProperty( PROPID_Allowrotation ) )
            PROPID_Allowrotation->SetPropertyToObject( editcopy, true );
        if ( ! HasProperty( PROPID_Allowsizing ) )
            PROPID_Allowsizing->SetPropertyToObject( editcopy, true );
        if ( ! HasProperty( PROPID_Allowskew ) )
            PROPID_Allowskew->SetPropertyToObject( editcopy, true );

        PROPID_Parent->SetPropertyToObject( editcopy, tool->GetParentObject() );
        editcopy->m_flags.m_editingCopy = true;

        editcopy->Update( updatemask_force );

        m_flags.m_editing = true;

        PROPID_Editmode->SetPropertyToObject( editcopy, editmode );
        PROPID_Editstyle->SetPropertyToObject( editcopy, ( wxUint16 ) editstyle );

        //the next maybe overruled in derived class
        PROPID_IncludeChildren->SetPropertyToObject( editcopy, true );
        PROPID_Controller->SetPropertyToObject( editcopy, tool->GetToolController() );
        PROPID_ViewSpecific->SetPropertyToObject( editcopy, tool->GetDrawingPart() );
        //new start edit, so no event yet
        PROPID_FirstEventInObject->SetPropertyToObject( editcopy, true );

        if ( !editcopy->DoStartEdit( editmode, editstyle ) )
        {
            editcopy->EndEdit();
            return NULL;
        }

        // Add the editcopy to the parent and set some flags
        tool->AddEditobject( editcopy );

        return editcopy;
    }

    return NULL;
}

void a2dCanvasObject::ReStartEdit( wxUint16 editmode )
{
    //prevent EditEnd() from deleting the editcopy.
    a2dCanvasObjectPtr editcopy = this;

    //get the value of properties which need to be restored.
    a2dCanvasObject* original = PROPID_Original->GetPropertyValue( this );
    a2dCanvasObject* parent = PROPID_Parent->GetPropertyValue( this );
    a2dToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dToolContr );
    wxUint16 editstyle = PROPID_Editstyle->GetPropertyValue( this );

    //stop editing,but the editcopy will be kept.
    EndEdit();

    //RESET the release that was flagged in endedit.
    m_release = false;
    SetVisible( true );
    SetIgnoreSetpending( false );
    SetSelected( false );
    PROPID_preserveAspectRatio->SetPropertyToObject( editcopy, false );
    PROPID_Allowrotation->SetPropertyToObject( editcopy, true );
    PROPID_Allowsizing->SetPropertyToObject( editcopy, true );
    PROPID_Allowskew->SetPropertyToObject( editcopy, true );
    PROPID_Original->SetPropertyToObject( editcopy, original );
    PROPID_Parent->SetPropertyToObject( editcopy, parent );

    PROPID_Editstyle->SetPropertyToObject( editcopy, editstyle );

    PROPID_Editcopy->SetPropertyToObject( original, editcopy );

    m_flags.m_editingCopy = true;

    Update( updatemask_force );

    original->m_flags.m_editing = true;

    PROPID_Editmode->SetPropertyToObject( editcopy, editmode );
    //the next maybe overruled in derived class
    PROPID_IncludeChildren->SetPropertyToObject( editcopy, true );
    PROPID_Controller->SetPropertyToObject( editcopy, controller );


    DoStartEdit( editmode , ( wxEditStyle ) editstyle );

    parent->Append( this );
}

bool a2dCanvasObject::DoStartEdit( wxUint16 WXUNUSED( editmode ), wxEditStyle WXUNUSED( editstyle ) )
{
    if ( m_flags.m_editable )
    {
        bool allowrotation = PROPID_Allowrotation->GetPropertyValue( this );
        bool allowsizing = PROPID_Allowsizing->GetPropertyValue( this );
        bool allowskew = PROPID_Allowskew->GetPropertyValue( this );
        bool preserveAspectRatio = PROPID_preserveAspectRatio->GetPropertyValue( this );

        m_flags.m_visiblechilds = true;
        m_flags.m_childrenOnSameLayer = true;

        //! \todo edit of bbox ( children or not included )
        //add properties and handles as children in order to edit the object.
        //a2dBoundingBox untrans = GetUnTransformedBbox( true );
        a2dBoundingBox untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_EDIT );
        if ( untrans.GetWidth() == 0 || untrans.GetWidth() == 0 )
            untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_CHILDREN | a2dCANOBJ_BBOX_EDIT );

        double x, y, w, h;
        x = untrans.GetMinX();
        y = untrans.GetMinY();
        w = untrans.GetWidth();
        h = untrans.GetHeight();


        a2dRect* around = new a2dRect( x, y, w, h );
        around->SetName( "around" );
        around->SetStroke( *wxRED, 0, a2dSTROKE_DOT_DASH );
        around->SetFill( *a2dTRANSPARENT_FILL );
        around->SetPreRenderAsChild( false );
        around->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
        around->SetHitFlags( a2dCANOBJ_EVENT_NON );
        around->SetLayer( m_layer );
        Append( around );

        a2dHandle* handle = NULL;
        if ( allowsizing )
        {
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
        }

        if ( allowrotation )
        {
            a2dSLine* rotline = new a2dSLine( x + w / 2, y + h / 2, x + w * 3 / 4, y + h / 2 );
            rotline->SetName( "rotline" );
            rotline->SetStroke( *wxRED, 0 );
            rotline->SetPreRenderAsChild( false );
            rotline->SetHitFlags( a2dCANOBJ_EVENT_NON );
            Append( rotline );
            rotline->SetLayer( m_layer );

            Append( handle = new a2dHandle( this, x + w * 3 / 4, y + h / 2, wxT( "rotate" ) ) );
            handle->SetLayer( m_layer );
            rotline->SetBin2( true );
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

    return false;
}

void a2dCanvasObject::EndEdit()
{
    //! call virtual to do object specific ending
    DoEndEdit();

    SetSnapTo( true );
    m_flags.m_editingCopy = false;

    //remove properties and handles as children that were added just to edit the object.
    if ( m_childobjects != wxNullCanvasObjectList )
        m_childobjects->Release( a2dCanvasOFlags::BIN2 );

    // it is not sure thata the editcopy will be deleted, e.g. when restarting,
    // all handles and such are removed, but re-added later in a different manner.

    a2dCanvasObject* original = PROPID_Original->GetPropertyValue( this );

    //SetSelected( m_preserve_select );
    RemoveProperty( PROPID_Original );

    RemoveProperty( PROPID_Allowrotation );
    RemoveProperty( PROPID_Allowsizing );
    RemoveProperty( PROPID_Allowskew );
    RemoveProperty( PROPID_preserveAspectRatio );

    RemoveProperty( PROPID_Editmode );
    RemoveProperty( PROPID_Editstyle );
    RemoveProperty( PROPID_Controller );
    RemoveProperty( PROPID_IncludeChildren );

    a2dCanvasObject* parent = PROPID_Parent->GetPropertyValue( this );
    RemoveProperty( PROPID_Parent );

    SetPending( true );

    original->m_flags.m_editing = false;
    // I am not sure if this ASSERT can fail.
    // If it fails, think about it and then convert it to an if for RemoveProperty
    //wxASSERT( PROPID_Editcopy->GetPropertyValue( original ) == this );
    original->RemoveProperty( PROPID_Editcopy );

    //! set invisible, deletion is done by tool later.
    SetVisible( false );
    SetIgnoreSetpending( true );
    //next marks editcopy for release, it will happen in idle time.
    if ( parent )
        parent->ReleaseChild( this );
}

a2dCanvasObject* a2dCanvasObject::GetOriginal()
{
    return PROPID_Original->GetPropertyValue( this ).Get();
}

a2dHandle* a2dCanvasObject::SetHandlePos( wxString name, double x, double y )
{
    a2dHandle* handle = NULL;

    if ( m_childobjects != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            handle = wxDynamicCast( ( *iter ).Get(), a2dHandle );
            if ( handle && handle->GetName() == name )
            {
                handle->SetPosXY( x, y );
                return handle;
            }
        }
    }
    return NULL;
}

void a2dCanvasObject::OnChar( wxKeyEvent& event )
{
    if ( m_flags.m_editingCopy && m_flags.m_editable )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_SPACE:
            {
                EndEdit();
                event.Skip();
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();
}

void a2dCanvasObject::OnPopUpEvent( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( HasDynamicProperties() )
    {
        const a2dMenuProperty* prop = PROPID_PopupMenu->GetPropertyListOnly( this );
        if ( prop )
        {
            wxWindow* win = ic->GetDrawingPart()->GetDisplayWindow();
            win->PopupMenu( prop->GetValue(), event.GetMouseEvent().GetX(), event.GetMouseEvent().GetY() );
            //wxLogDebug( wxT(" reached vertex curve with left down ") );
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

void a2dCanvasObject::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( HasDynamicProperties() )
    {
        a2dCanvasObjectPtrProperty* prop = PROPID_Objecttip->GetPropertyListOnly( this );
        if ( prop )
        {
            prop->SetVisible( true );
            SetPending( true );
        }

        a2dTipWindowProperty* tipProp = wxDynamicCast( PROPID_TipWindow->GetPropertyListOnly( this ), a2dTipWindowProperty );
        if ( tipProp )
        {
            wxTipWindow* tipWindow = wxDynamicCast( tipProp->GetValue(), wxTipWindow );
            wxWindow* win = ic->GetDrawingPart()->GetDisplayWindow();
            if( tipWindow )
            {
                // this happens normally in idle time, but this makes it happen for sure.
                tipWindow->SetTipWindowPtr( NULL );
                tipWindow->Close();
            }

            wxRect tipRect = tipProp->GetRect();
            wxPoint aWinPos( 0, 0 );
            aWinPos = win->ClientToScreen( aWinPos );
            wxRect aRect( aWinPos.x + event.GetMouseEvent().GetX() - tipRect.GetX(),
                          aWinPos.y + event.GetMouseEvent().GetY() - tipRect.GetY(), tipRect.GetWidth(), tipRect.GetHeight() );
            if ( tipProp->GetUseObjRect() )
            {
                aRect = GetAbsoluteArea( *ic );
                wxPoint aWinPos1 = win->ClientToScreen( aRect.GetTopLeft() );
                wxPoint aWinPos2 = win->ClientToScreen( aRect.GetBottomRight() );
                aRect = wxRect( aWinPos1, aWinPos2 );
            }
            tipWindow = new wxTipWindow( win, tipProp->GetString(), 200 , ( wxTipWindow** )tipProp->GetValuePtr(), &aRect );
            tipProp->SetValue( tipWindow );
        }
    }

#ifdef _DEBUG_REPORTHIT
    wxLogDebug( wxT( "%s %p" ), wxT( "enter" ), this );
#endif


    // more object can be in editing state in case of subediting, only the one captured may change the cursor.
    if ( m_flags.m_editingCopy && m_flags.m_editable )
    {
        ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CanvasObjectEnter ) );
    }
}

void a2dCanvasObject::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( HasDynamicProperties() )
    {
        a2dCanvasObjectPtrProperty* prop = PROPID_Objecttip->GetPropertyListOnly( this );
        if ( prop )
        {
            prop->SetVisible( false );
            SetPending( true );
        }
        const a2dWindowProperty* tipProp = PROPID_TipWindow->GetPropertyListOnly( this );;
        if ( tipProp )
        {
            wxTipWindow* tipWindow = wxDynamicCast( tipProp->GetValue(), wxTipWindow );
            if( tipWindow )
            {
                // tipWindow->SetTipWindowPtr(NULL);
                // This OnLeaveObject() is called when tipWindow gets focus by SetFocus after
                // it was created in OnEnterObject(),
                // so here it is early to close the tipWindow
                // tipWindow->Close();
            }
//              tipProp->SetValue(NULL);
        }
    }

#ifdef _DEBUG_REPORTHIT
    wxLogDebug( wxT( "%s %p" ), wxT( "leaf" ), this );
#endif

    // more object can be in editing state in case of subediting, only the one captured may change the cursor.
    if ( m_flags.m_editingCopy && m_flags.m_editable  )
    {
        ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CanvasObjectLeave ) );
        //if( ic->GetDrawingPart()->GetEndCorridorObject() == this )
        //    ic->GetDrawingPart()->PopCursor();
        //else
        //    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Edit ) );
    }
}

void a2dCanvasObject::LeaveInObjects( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    a2dCanvasObjectList::reverse_iterator iter = m_childobjects->rbegin();
    while( iter != m_childobjects->rend() )
    {
        a2dCanvasObject* obj = *iter;
        // DO NOT assume only object with mouse inside when parent was also.
        obj->LeaveInObjects( ic, hitEvent );
        if ( obj->m_flags.m_MouseInObject )
        {
            //wxLogDebug(wxT("leaveFromTop %p"), obj );

            obj->m_flags.m_MouseInObject = false;
            wxMouseEvent* mouse = wxDynamicCast( hitEvent.m_event, wxMouseEvent );
            a2dCanvasObjectMouseEvent leave( &ic, obj, wxEVT_CANVASOBJECT_LEAVE_EVENT, hitEvent.m_x, hitEvent.m_y, *mouse );
            hitEvent.SetProcessed( obj->ProcessEvent( leave ) );
        }
        iter++;
    }
}

void a2dCanvasObject::OnHandleEvent( a2dHandleMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy && m_flags.m_editable )
    {
        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();

        a2dHandle* draghandle = event.GetCanvasHandle();

        //to world group coordinates to do hit test in world group coordinates
        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        //matrix to convert from absolute world coordinates to local object coordinates,
        //with m_lworld included.
        double xwi;
        double ywi;
        ic->GetInverseTransform().TransformPoint( xw, yw, xwi, ywi );

        bool __includeChildren__ = PROPID_IncludeChildren->GetPropertyValue( this );
        a2dBoundingBox untrans;
        if ( __includeChildren__ )
            untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_CHILDREN | a2dCANOBJ_BBOX_EDIT );
        else
            untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_EDIT );

        double xmin, ymin, xmax, ymax, w, h;
        xmin = untrans.GetMinX();
        ymin = untrans.GetMinY();
        xmax = untrans.GetMaxX();
        ymax = untrans.GetMaxY();
        w = untrans.GetWidth();
        h = untrans.GetHeight();

        a2dAffineMatrix origworld = m_lworld;
        double x1, y1, x2, y2;

        a2dCanvasObject* original = PROPID_Original->GetPropertyValue( this );

        if ( event.GetMouseEvent().LeftDown() )
        {
	        ic->GetDrawingPart()->PushCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HandleLeftDown ) );
            if ( restrictEngine )
                restrictEngine->SetRestrictPoint( xw, yw );
        }
		else if ( event.GetMouseEvent().Moving() )
		{
	        ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HandleMove ) );
		}
        else if ( event.GetMouseEvent().LeftUp() )
        {
            if ( m_lworld != original->GetTransformMatrix() )
            {
                m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_TransformMatrix, m_lworld ) );

                a2dGeneralGlobals->RecordF( this, wxT( "setproperty __M_Transform { matrix ( %g %g %g %g %g %g ) } transform" ),
                                            m_lworld.GetValue( 0, 0 ), m_lworld.GetValue( 0, 1 ),
                                            m_lworld.GetValue( 1, 0 ), m_lworld.GetValue( 1, 1 ),
                                            m_lworld.GetValue( 2, 0 ), m_lworld.GetValue( 2, 1 ) );
            }
        }
        else if ( event.GetMouseEvent().Dragging() )
        {
	        ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HandleDrag ) );
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

            bool preserveAspectRatio = PROPID_preserveAspectRatio->GetPropertyValue( this );

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
                if ( preserveAspectRatio )
                    if ( sy > sx ) sx = sy; else sy = sx;                    
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
                if ( preserveAspectRatio )
                    if ( sy > sx ) sx = sy; else sy = sx;                    
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
                if ( preserveAspectRatio )
                    if ( sy > sx ) sx = sy; else sy = sx;                    
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
                if ( preserveAspectRatio )
                    if ( sy > sx ) sx = sy; else sy = sx;                    
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
}

void a2dCanvasObject::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();
    ic->SetPerLayerMode( false );

    if ( m_flags.m_editingCopy )
        ic->SetPerLayerMode( false );

    if ( event.GetMouseEvent().RightDown() )
    {
        //wxWindow* win = ic->GetDrawingPart()->GetDisplayWindow();
        //win->PopupMenu(mousemenu,event.GetX(), event.GetY());
        //wxLogDebug( wxT(" reached vertex curve with left down ") );
        a2dCanvasObjectMouseEvent popup( ic, this, wxEVT_CANVASOBJECT_POPUPMENU_EVENT, event.GetX(), event.GetY(), event.m_mouseevent );
        popup.SetEventObject( this );

        if ( !this->ProcessEvent( popup ) )
            event.Skip();
    }
    // editing of an object is based on the editcopy flag, which is only set at the top object of an editclone.
    // So the children of the object being edited does not have the flag set.
    else if ( m_flags.m_editingCopy && m_flags.m_editable  )
    {
        bool skipbase = PROPID_SkipBase->GetPropertyValue( this );

        if ( skipbase )
        {
            event.Skip();
            return;
        }

        a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();
        a2dCanvasObject* original = PROPID_Original->GetPropertyValue( this );
        a2dCanvasObject* parent = PROPID_Parent->GetPropertyValue( this );

        static double xshift;
        static double yshift;

        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        double xh, yh;
        ic->GetInverseTransform().TransformPoint( xw, yw, xh, yh );

        if ( event.GetMouseEvent().Moving() )///&& m_flags.m_subEditAsChild )
        {
            // all basic primitives rely on this to set cursor if no specialized move cursors (edges and such)
            if ( event.m_how.IsHit() )
                ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CanvasObjectHit ) );
            else
                ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Edit ) );
            event.Skip();
        }
        else if ( event.GetMouseEvent().LeftDClick()  )
        {
            // double click while the object is in edit mode (m_flags.m_editingCopy)
            // will result in stopping the oneshot edit tool.
            EndEdit();
        }
        else if ( event.GetMouseEvent().LeftDown() )
        {
            // if the object is hit, we can test for sub editing of child objects.
            // The special case is the object called LABEL, which has priority.
            // Next candidate is the child hit.

            // hit test done in ProcessCanvasObjectEvent()
            //a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
            //if ( IsHitWorld( *ic, hitevent ) )
            if ( event.GetHow().IsHit() )
            {
                if ( restrictEngine )
                    restrictEngine->SetRestrictPoint( xw, yw );

                if ( 1 )
                {
                    a2dCanvasObject* hit = NULL;
                    {
                        //labels have priority for editing
                        a2dCanvasObject* label = original->Find( wxT( "__LABEL__" ) );

                        if ( label )
                        {
                            a2dIterCU cu( *ic, original );
                            a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
                            hit = label->IsHitWorld( *ic, hitevent );
                        }
                        else if ( m_flags.m_subEdit ) // subedit allowed for other objects.
                        {
                            a2dHitEvent hitevent = a2dHitEvent( xw, yw, false, a2dCANOBJHITOPTION_NOROOT );
                            hit = original->IsHitWorld( *ic, hitevent );
                            if ( hit && !hit->m_flags.m_subEditAsChild )
                                hit = NULL;

                        }
                    }
                    if ( hit && hit->GetEditable() ) //subediting of child or labels, is we have a hit.
                    {
                        a2dIterCU cu( *ic, original );
                        a2dToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dToolContr );

                        //editing is always a oneshot, since editing of children here is under control of the
                        //a2dCanvasObject (can be different for derived classes). So editing tool should not take over
                        //the decision to edit one after another child.
                        ic->SetCorridorPathToParent();
                        controller->StartEditingObject( hit, *ic );
                    }
                    //subediting was not wanted or hit, therefore now start dragging the object itself
                    //at mouse LeftDown
                    else if ( IsDraggable() )
                    {
                        // Corridor is already in place, since edit tool is active.
                        ic->GetDrawingPart()->PushCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Move ) );

                        xshift = GetPosX() - xh;
                        yshift = GetPosY() - yh;
                    }
                }
            }
            // left down and no hit on this object, while the object is in edit mode (m_flags.m_editingCopy)
            // will result in stopping the oneshot edit tool.
            else
            {
                EndEdit();
            }
        }
        else if ( event.GetMouseEvent().LeftUp() && ic->GetDrawingPart()->GetEndCorridorObject() == this )
        {
            // the corridor was captured at Left Down.
            //ic->SetCorridorPathToParent();
            ic->GetDrawingPart()->PopCursor();

            if ( m_lworld != original->GetTransformMatrix() )
            {
                m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( original, PROPID_TransformMatrix, m_lworld ) );

                a2dGeneralGlobals->RecordF( this, wxT( "setproperty __M_Transform { matrix ( %g %g %g %g %g %g )} transform" ),
                                            m_lworld.GetValue( 0, 0 ), m_lworld.GetValue( 0, 1 ),
                                            m_lworld.GetValue( 1, 0 ), m_lworld.GetValue( 1, 1 ),
                                            m_lworld.GetValue( 2, 0 ), m_lworld.GetValue( 2, 1 ) );
            }
        }
        else if ( IsDraggable() && event.GetMouseEvent().Dragging() && ic->GetDrawingPart()->GetEndCorridorObject() == this )
        {
            double x, y;
            x = xh + xshift;
            y = yh + yshift;

            if( restrictEngine )
            {
                //temporarily restore position according to the mouse position, which is the unrestricted position.
                SetPosXY( x, y );

                a2dPoint2D point;
                double dx, dy; //detect restriction distance of any point.
                if ( restrictEngine->RestrictCanvasObjectAtVertexes( this, point, dx, dy ) )
                {
                    //restrict the object drawing to that point
                    x += dx;
                    y += dy;
                }
            }
            SetPosXY( x, y );
        }
        else
            event.Skip();
    }
    else
        event.Skip();

}

a2dCanvasObjectList* a2dCanvasObject::GetChildObjectList()
{
    return m_childobjects;
}

const a2dCanvasObjectList* a2dCanvasObject::GetChildObjectList() const
{
    return m_childobjects;
}

a2dCanvasObjectList* a2dCanvasObject::CreateChildObjectList()
{
    if ( m_childobjects == wxNullCanvasObjectList )
        m_childobjects = new a2dCanvasObjectList();

    return m_childobjects;
}


unsigned int a2dCanvasObject::GetChildObjectsCount() const
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return 0;
    return m_childobjects->size();
}

a2dCanvasObject* a2dCanvasObject::PushInto( a2dCanvasObject* parent ) 
{ 
    if ( m_flags.m_pushin )
        return this; 
    return NULL;
}


void a2dCanvasObject::SetPending( bool pending )
{
    if ( m_ignoreAllSetpending )
        return;

    m_flags.m_pending = pending;

    if ( pending && !m_flags.m_ignoreSetpending )
    {
        if ( m_flags.m_hasPins && !IsConnect() )
        {
            //This is not good for speed in tools.
            //SetConnectedPending( pending, true );
        }
        if ( m_root )
            m_root->SetUpdatesPending( true );
    }
}

void a2dCanvasObject::SetPendingSelect( bool pending )
{
    if ( m_ignoreAllSetpending )
        return;

    m_flags.m_selectPending = pending;

    if ( pending && !m_flags.m_ignoreSetpending )
    {
        if ( m_root )
            m_root->SetUpdatesPending( true );
    }
}

void a2dCanvasObject::SetTransformMatrix( double xt, double yt, double scalex, double scaley, double degrees )
{
    m_lworld = a2dAffineMatrix( xt, yt, scalex, scaley, degrees );  SetPending( true );
}

void a2dCanvasObject::SetRotation( double rotation )
{
    m_lworld.SetRotation( rotation );
    SetPending( true );
}

void a2dCanvasObject::Rotate( double rotation )
{
    m_lworld.Rotate( rotation );
    SetPending( true );
}

void a2dCanvasObject::SkewX( double angle )
{
    m_lworld.SkewX( angle );
    SetPending( true );
}

void a2dCanvasObject::SkewY( double angle )
{
    m_lworld.SkewY( angle );
    SetPending( true );
}

void a2dCanvasObject::Scale( double scalex, double scaley )
{
    m_lworld.Scale( scalex, scaley, m_lworld.GetValue( 2, 0 ), m_lworld.GetValue( 2, 1 ) );
    SetPending( true );
}

void a2dCanvasObject::Mirror( bool x, bool y )
{
    m_lworld.Mirror( x, y );
    SetPending( true );
}

void a2dCanvasObject::SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which )
{
    m_flags.SetSpecificFlags( setOrClear, which );
}

void a2dCanvasObject::SetFlags( a2dCanvasObjectFlagsMask which )
{
    m_flags.SetFlags( which );
}

bool a2dCanvasObject::GetFlag( a2dCanvasObjectFlagsMask which ) const
{
    return m_flags.GetFlag( which );
}

a2dCanvasObjectFlagsMask a2dCanvasObject::GetFlags() const
{
    return m_flags.GetFlags();
}

bool a2dCanvasObject::CheckMask( a2dCanvasObjectFlagsMask mask ) const
{
    return m_flags.CheckMask( mask );
}

void a2dCanvasObject::SetHitFlags( a2dCanvasObjectHitFlags mask )
{
    m_hitflags.m_non  =  ( mask & a2dCANOBJ_EVENT_NON ) > 0;
    m_hitflags.m_fill =  ( mask & a2dCANOBJ_EVENT_FILL ) > 0;
    m_hitflags.m_stroke = ( mask & a2dCANOBJ_EVENT_STROKE ) > 0;
    m_hitflags.m_fill_non_transparent = ( mask & a2dCANOBJ_EVENT_FILL_NON_TRANSPARENT ) > 0;
    m_hitflags.m_stroke_non_transparent = ( mask & a2dCANOBJ_EVENT_STROKE_NON_TRANSPARENT ) > 0;
    m_hitflags.m_visible = ( mask & a2dCANOBJ_EVENT_VISIBLE ) > 0;
    m_hitflags.m_all = ( mask & a2dCANOBJ_EVENT_ALL ) > 0 ;
}

a2dCanvasObjectFlagsMask a2dCanvasObject::GetHitFlags() const
{
    a2dCanvasObjectFlagsMask oflags = 0;
    if ( m_hitflags.m_non )                    oflags = oflags | a2dCANOBJ_EVENT_NON;
    if ( m_hitflags.m_fill )                   oflags = oflags | a2dCANOBJ_EVENT_FILL;
    if ( m_hitflags.m_stroke )                 oflags = oflags | a2dCANOBJ_EVENT_STROKE;
    if ( m_hitflags.m_fill_non_transparent )   oflags = oflags | a2dCANOBJ_EVENT_FILL_NON_TRANSPARENT;
    if ( m_hitflags.m_stroke_non_transparent ) oflags = oflags | a2dCANOBJ_EVENT_STROKE_NON_TRANSPARENT;
    if ( m_hitflags.m_visible )                oflags = oflags | a2dCANOBJ_EVENT_VISIBLE;
    if ( m_hitflags.m_all )                    oflags = oflags | a2dCANOBJ_EVENT_ALL;
    return oflags;
}

void a2dCanvasObject::foreach_f( void ( *fp ) ( a2dCanvasObject* item ) )
{
    fp ( this );
    if ( m_childobjects != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj )
                fp ( obj );
        }
    }
}

void a2dCanvasObject::foreach_mf( void ( a2dCanvasObject::*mfp ) () )
{
    ( this->*mfp )();
    if ( m_childobjects != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj )
                ( obj->*mfp )();
        }
    }
}

bool a2dCanvasObject::EditProperties( const a2dPropertyId* id, bool withUndo )
{
    a2dNamedPropertyList allprop;

    CollectProperties2( &allprop, id, a2dPropertyId::flag_none );

    bool res = false;

    a2dPropertyEditEvent event( this, &allprop );
    event.SetEventObject( this );

    // first try the object itself, this makes object specific property editing possible.
    ProcessEvent( event );

    if ( event.GetEdited() && !allprop.empty() )
    {
        res = true;
        a2dNamedPropertyList::iterator iter;
        for( iter = allprop.begin(); iter != allprop.end(); ++iter )
        {
            a2dNamedProperty* prop = *iter;
            if ( withUndo )
            {
                m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( this, prop ) );
                //\!todo
                //a2dGeneralGlobals->RecordF( this, wxT("setproperty ???")  );
            }
            else
                prop->SetToObject( this );
        }
    }

    return res;
}

bool a2dCanvasObject::DoIgnoreIfNotMember( const a2dPropertyId& id )
{
    return id.IsOnlyMember() || id.IsOnlyMemberHint();
}

void a2dCanvasObject::OnPropertyChanged( a2dComEvent& event )
{
    if ( event.GetPropertyId() && ! event.GetPropertyId()->CheckFlags( a2dPropertyId::flag_norender )  )
    {
        // we assume that the property does influence the object if its no_render flag is false
        SetPending( true );
    }
    // also a2dVisibleProperty showing this a2dPropertyId needs to be set pending.
    if ( m_childobjects != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dVisibleProperty* visProp = wxDynamicCast( ( *iter ).Get(), a2dVisibleProperty );
            if ( visProp && visProp->GetPropId() == event.GetPropertyId() )
                visProp->ProcessEvent( event );
        }
    }
}

a2dText* a2dCanvasObject::SetObjectTip( const wxString& tip, double x, double y, double size, double angle,  const a2dFont& font )
{
    a2dFont lfont = font;
    lfont.SetSize( size );
    a2dText* object = new a2dText( tip, x, y, lfont, angle, true );
    object->SetLayer( m_layer );
    object->SetStroke( *a2dBLACK_STROKE );
    object->SetFill( *a2dTRANSPARENT_FILL );
    object->SetRoot( m_root );
    object->SetIsProperty( true );
    RemoveProperty( PROPID_Objecttip );

    a2dCanvasObjectPtrProperty prop( PROPID_Objecttip, object, false, true );
    PROPID_Objecttip->SetPropertyToObject( this, &prop );
    return object;
}

void a2dCanvasObject::SetTipWindow( const wxString& tip )
{
    RemoveProperty( PROPID_TipWindow );

    a2dTipWindowProperty prop( PROPID_TipWindow, NULL, tip );
    PROPID_TipWindow->SetPropertyToObject( this, &prop );
}

a2dObject* a2dCanvasObject::GetObjectTip()
{
    if ( HasDynamicProperties() )
    {
        return PROPID_Objecttip->GetPropertyValue( this );
    }

    return 0;
}

void a2dCanvasObject::SetViewDependent( a2dDrawingPart* aView, bool viewdependent, bool viewspecific, bool onlyinternalarea, bool deep )
{
    wxASSERT_MSG( !aView || aView->GetViewDependentObjects() , wxT( "a2dDrawingPart::GetViewDependentObjects() is not set" ) );

    if ( deep )
    {
        a2dWalker_SetViewDependent setv( aView, viewdependent, viewspecific, onlyinternalarea );
        setv.SetSkipNotRenderedInDrawing( true );
        setv.Start( this );
        return;
    }

    bool enPending = FALSE;
    if( !aView )
    {
        enPending |= RemoveProperty( PROPID_ViewSpecific );
        enPending |= RemoveProperty( PROPID_ViewDependent );
        enPending |= RemoveProperty( PROPID_IntViewDependTransform );
    }
    else
    {
        if( viewdependent )
        {
            enPending = TRUE;
            PROPID_ViewDependent->SetPropertyToObject( this, aView );
            if( onlyinternalarea )
                PROPID_IntViewDependTransform->SetPropertyToObject( this, a2dIDENTITY_MATRIX );
        }
        else
        {
            a2dDrawingPart* view = wxStaticCastNull( PROPID_ViewDependent->GetPropertyValue( this ).Get(), a2dDrawingPart );
            if ( view == aView )
            {
                enPending |= RemoveProperty( PROPID_ViewDependent );
                enPending |= RemoveProperty( PROPID_IntViewDependTransform );
            }
        }
        if( viewspecific )
        {
            enPending = TRUE;
            PROPID_ViewSpecific->SetPropertyToObject( this, aView );
        }
        else
        {
            a2dDrawingPart* view = wxStaticCastNull( PROPID_ViewSpecific->GetPropertyValue( this ).Get(), a2dDrawingPart );
            if ( view == aView )
                enPending |= RemoveProperty( PROPID_ViewSpecific );
        }
    }
    if( enPending )
        SetPending( true );
}

const a2dShadowStyleProperty* a2dCanvasObject::GetShadowStyle() const
{
    return PROPID_Shadowstyle->GetProperty( this );
}

void a2dCanvasObject::SetFill( const a2dFill& fill )
{
    if ( !fill.IsNoFill() )
        PROPID_Fill->SetPropertyToObject( this, fill );
    else
        RemoveProperty( PROPID_Fill );
}

void a2dCanvasObject::SetFill( const wxColour& fillcolor, a2dFillStyle fillstyle )
{
    a2dFill fill = a2dFill( fillcolor, fillstyle );

    PROPID_Fill->SetPropertyToObject( this, fill );
}

void a2dCanvasObject::SetFill(  const wxColour& fillcolor, const wxColour& fillcolor2, a2dFillStyle fillstyle )
{
    a2dFill fill = a2dFill( fillcolor, fillcolor2, fillstyle );

    PROPID_Fill->SetPropertyToObject( this, fill );
}

a2dFill a2dCanvasObject::GetFill() const
{
    a2dFill propval = PROPID_Fill->GetPropertyValue( this );
    return propval;
}

wxColour a2dCanvasObject::GetFillColour() const
{
    if ( !GetFill().IsNoFill() )
        return GetFill().GetColour();

    if ( m_flags.m_filled )
    {
        if (  m_root && m_root->GetLayerSetup() )
            return m_root->GetLayerSetup()->GetFill( m_layer ).GetColour();
        return wxColour( 0, 0, 0 );
    }
    else
        return a2dTRANSPARENT_FILL->GetColour();
}

void a2dCanvasObject::SetFillColour( const wxColour& colour )
{
    a2dFill fill = PROPID_Fill->GetPropertyValue( this );
    fill.SetColour( colour );
    SetFill( fill );
}

void a2dCanvasObject::SetStroke( const wxColour& strokecolor, double width, a2dStrokeStyle strokestyle )
{
    a2dStroke stroke = a2dStroke( strokecolor, ( float ) width, strokestyle );

    PROPID_Stroke->SetPropertyToObject( this, stroke );
}

void a2dCanvasObject::SetStroke( const wxColour& strokecolor, int width, a2dStrokeStyle strokestyle )
{
    a2dStroke stroke = a2dStroke( strokecolor, width, strokestyle );

    PROPID_Stroke->SetPropertyToObject( this, stroke );
}

void a2dCanvasObject::SetStroke( const a2dStroke& stroke )
{
    if ( !stroke.IsNoStroke() )
        PROPID_Stroke->SetPropertyToObject( this, stroke );
    else
        RemoveProperty( PROPID_Stroke, true );
}

a2dStroke a2dCanvasObject::GetStroke() const
{
    a2dStroke propval = PROPID_Stroke->GetPropertyValue( this );
    return propval;
}

wxColour a2dCanvasObject::GetStrokeColour() const
{
    if ( !GetStroke().IsNoStroke() )
        return GetStroke().GetColour();

    if (  m_root && m_root->GetLayerSetup() )
        return m_root->GetLayerSetup()->GetStroke( m_layer ).GetColour();

    return wxColour( 0, 0, 0 );
}

void a2dCanvasObject::SetStrokeColour( const wxColour& colour )
{
    a2dStroke stroke = PROPID_Stroke->GetPropertyValue( this );
    stroke.SetColour( colour );
    SetStroke( stroke );
}

const a2dFill& a2dCanvasObject::MX_GetFill() const
{
    if ( HasDynamicProperties() )
    {
        a2dNamedPropertyList::const_iterator iter;
        for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
        {
            const a2dNamedProperty* prop = *iter;
            if ( prop->GetId() == PROPID_Fill )
            {
                a2dFillProperty* propfill = wxStaticCast( prop, a2dFillProperty );
                return propfill->GetValue();
            }
        }
    }

    return *a2dNullFILL;
}

void a2dCanvasObject::MX_SetFill( const a2dFill& value )
{
    a2dFill fill = value;
    if ( !fill.IsNoFill() )
    {
        if ( fill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
        {
            double x1 = fill.GetStart().m_x;
            double y1 = fill.GetStart().m_y;
            double x2 = fill.GetStop().m_x;
            double y2 = fill.GetStop().m_y;
            if ( x1 == x2 && y1 == y2 )
            {
                //do a linear fill vertical
                a2dBoundingBox& box = GetBbox();
                x2 = x1 = box.GetMaxX() - box.GetMinX();
                y1 = box.GetMaxY();
                y2 = box.GetMinY();

                fill = a2dFill( fill.GetColour(), fill.GetColour2(),  x1, y1, x2, y2 );
            }
        }
        else if ( fill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
        {
            double xfc = fill.GetFocal().m_x;
            double yfc = fill.GetFocal().m_y;
            double xe = fill.GetCenter().m_x;
            double ye = fill.GetCenter().m_y;
            if ( xfc == xe && yfc == ye )
            {
                //no start stop given
                a2dBoundingBox& box = GetBbox();
                xe = xfc = ( box.GetMaxX() + box.GetMinX() ) / 2;
                ye = yfc = ( box.GetMaxY() + box.GetMinY() ) / 2;
                double xr = box.GetMaxX();
                double yr = box.GetMaxY();
                double radius = wxMax( ( xe - xr ), ( ye - yr ) );
                fill = a2dFill( fill.GetColour(), fill.GetColour2(),  xfc, yfc, xe, ye, radius / 2 );
            }
        }
        
        if ( HasDynamicProperties() )
        {
            a2dNamedPropertyList::const_iterator iter;
            for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
            {
                const a2dNamedProperty* prop = *iter;
                if ( prop->GetId() == PROPID_Fill )
                {
                    a2dFillProperty* propfill = wxStaticCast( prop, a2dFillProperty );
                    propfill->SetValue( fill );
                    return;
                }
            }
        }

        a2dFillProperty* prop =  new a2dFillProperty( PROPID_Fill, fill );
        AddProperty( prop );
    }
    else
        RemoveProperty( PROPID_Fill );
}

const a2dStroke& a2dCanvasObject::MX_GetStroke() const
{
    if ( HasDynamicProperties() )
    {
        a2dNamedPropertyList::const_iterator iter;
        for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
        {
            const a2dNamedProperty* prop = *iter;
            if ( prop->GetId() == PROPID_Stroke )
            {
                a2dStrokeProperty* propstroke = wxStaticCast( prop, a2dStrokeProperty );
                return propstroke->GetValue();
            }
        }
    }

    return *a2dNullSTROKE;
}

void a2dCanvasObject::MX_SetStroke( const a2dStroke& value )
{
    if ( value.IsNoStroke() )
        RemoveProperty( PROPID_Stroke );
    else
    {
        if ( HasDynamicProperties() )
        {
            a2dNamedPropertyList::const_iterator iter;
            for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
            {
                const a2dNamedProperty* prop = *iter;
                if ( prop->GetId() == PROPID_Stroke )
                {
                    a2dStrokeProperty* propstroke = wxStaticCast( prop, a2dStrokeProperty );
                    propstroke->SetValue( value );
                    return;
                }
            }
        }

        a2dStrokeProperty* prop =  new a2dStrokeProperty( PROPID_Stroke, value );
        AddProperty( prop );
    }
}

void a2dCanvasObject::SetDrawerStyle( a2dIterC& ic, a2dStyleProperty* style )
{
    a2dLayers* layers = m_root->GetLayerSetup();

    if ( m_flags.m_filled )
    {
        if ( style )
        {
            if ( !style->GetFill().IsNoFill() && style->GetFill().IsSameAs( *a2dINHERIT_FILL ) )
            {
                //nothing i hope
            }
            else if ( style->GetFill().IsNoFill() )
            {
                if ( layers )
                    ic.GetDrawer2D()->SetDrawerFill( layers->GetFill( m_layer ) );
                else  //should not be
                    ic.GetDrawer2D()->SetDrawerFill( *a2dNullFILL );
            }
            else
                ic.GetDrawer2D()->SetDrawerFill( style->GetFill() );

        }
        else if ( layers )
        {
            ic.GetDrawer2D()->SetDrawerFill( layers->GetFill( m_layer ) );
        }
    }
    else
    {
        ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
    }

    if ( style )
    {
        if ( !style->GetStroke().IsNoStroke() && style->GetStroke().IsSameAs( *a2dINHERIT_STROKE ) )
        {
            //nothing i hope
        }
        else if ( style->GetStroke().IsNoStroke() )
        {
            if ( layers )
                ic.GetDrawer2D()->SetDrawerStroke( layers->GetStroke( m_layer ) );
            else  //should not be
                ic.GetDrawer2D()->SetDrawerStroke( *a2dNullSTROKE );
        }
        else
            ic.GetDrawer2D()->SetDrawerStroke( style->GetStroke() );
    }
    else if ( layers )
    {
        ic.GetDrawer2D()->SetDrawerStroke( layers->GetStroke( m_layer ) );
    }
}

void a2dCanvasObject::CaptureMouse( a2dIterC& ic )
{
    ic.SetCorridorPathToObject( this );
}

void a2dCanvasObject::ReleaseMouse( a2dIterC& ic )
{
    ic.SetCorridorPathToParent();
}

bool a2dCanvasObject::IsCapturedMouse( a2dIterC& ic ) const
{
    return ic.GetDrawingPart()->GetEndCorridorObject() == this;
}

bool a2dCanvasObject::GeneratePinsPossibleConnections( a2dPinClass* pinClass, a2dConnectTask task, double x, double y, double margin )
{
    wxASSERT_MSG( pinClass->GetConnectionGenerator()
		, wxT( "pinclass connection generator not set" ) );

    return pinClass->GetConnectionGenerator()->GeneratePossibleConnections( this, pinClass, task, x, y, margin );
}

a2dCanvasObject* a2dCanvasObject::GetConnectTemplate( a2dPinClass* mapThis, a2dCanvasObject* other, a2dPinClass* mapOther ) const
{
    return mapThis->GetConnectionGenerator()->GetConnectTemplate( this, mapThis, other, mapOther );
}

a2dCanvasObject* a2dCanvasObject::CreateConnectObject( a2dCanvasObject* parent, a2dPin* pinThis, a2dPin* pinOther, bool undo ) const
{
    return pinThis->GetPinClass()->GetConnectionGenerator()->CreateConnectObject( parent, pinThis, pinOther, undo );
}


a2dBoundingBox& a2dCanvasObject::GetBbox()
{
    // bounding boxes normally get re-calculated on the fly when an object is set pending,
    // or a child object was set pending.
    // When an object is created it is always set to pending. But calculating the boudingbox
    // at that moment is often useless, since it may depend on things that will be added
    // ( like child objects ).
    // This leads to the problem that sometimes one wants to have the boundingbox before it
    // is calculated in idle time.
    // So if not valid yet, we calculate it right now.
    // Still we set the object pending, so in the next Update cycle ( idle time ),
    // all objects in the parent chains will be updated also to reflect/include this objects
    // its boundingbox.
    // Doing it when a boundingbox is needed, is at the latest stage for sure, and automatic.

    if ( !m_bbox.GetValid() )
    {
        Update( updatemask_force ); //calculate on the fly
        SetPending( true );
    }

    return m_bbox;
}

wxRect a2dCanvasObject::GetAbsoluteArea( a2dIterC& ic, int inflate )
{
    int x1, y1, x2, y2;

    a2dBoundingBox tmp = GetBbox();
    if ( !ic.GetTransform().IsIdentity() )
        tmp.MapBbox( ic.GetTransform() );

    //different scaling in X and Y
    //goes wrong IF pen is included in GetBbox().
    //In reality the pen width stays constant in x and y while rendering,
    //so we add it here.
    //include the pen width now
    tmp.Enlarge( m_worldExtend );

    x1 = ic.GetDrawer2D()->WorldToDeviceX( tmp.GetMinX() );
    y1 = ic.GetDrawer2D()->WorldToDeviceY( tmp.GetMinY() );
    x2 = ic.GetDrawer2D()->WorldToDeviceX( tmp.GetMaxX() );
    y2 = ic.GetDrawer2D()->WorldToDeviceY( tmp.GetMaxY() );

    if ( x1 > x2 )
    {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if ( y1 > y2 )
    {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    wxRect tmparea;
    tmparea.x = x1;
    tmparea.y = y1;

    tmparea.width  = x2 - x1;
    tmparea.height = y2 - y1;
    tmparea.Inflate( m_pixelExtend );

    //EXTREMELY IMPORTANT
    tmparea.Inflate( inflate ); //pixels extra on each side for the "ONE BIT BUG" ;-)

    return tmparea;
}

a2dBoundingBox a2dCanvasObject::GetMappedBbox( const a2dAffineMatrix& cworld )
{
    //first map without extend width which is not included in the boundingbox
    a2dBoundingBox tmp = GetBbox();
    if ( !cworld.IsIdentity() )
        tmp.MapBbox( cworld );

    return tmp;
}

a2dBoundingBox a2dCanvasObject::GetMappedBbox( a2dIterC& ic, bool withExtend )
{
    //first map without extend width which is not included in the boundingbox
    a2dBoundingBox tmp = GetBbox();
    if ( !ic.GetTransform().IsIdentity() )
        tmp.MapBbox( ic.GetTransform() );

    if ( withExtend )
    {
        if ( m_worldExtend )
            tmp.Enlarge( m_worldExtend );
        if ( m_pixelExtend )
            tmp.Enlarge( ic.GetDrawer2D()->DeviceToWorldXRel( m_pixelExtend ) );
    }
    return tmp;
}

OVERLAP a2dCanvasObject::GetClipStatus( a2dIterC& ic, OVERLAP clipparent )
{
    // clipparent is either _IN or _ON      ( _OUT would not get us to this point )
    // testing clipping of children only needed when not totaly _IN.
    if ( clipparent == _ON )
    {
        //get bbox at the absolute position
        //first map without pen width which is not included in the boundingbox
        a2dBoundingBox absarea = GetMappedBbox( ic, true );
        return ic.GetDrawer2D()->GetClippingBox().Intersect( absarea );
    }

    return clipparent;
}

a2dCanvasObject* a2dCanvasObject::IsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    a2dAffineMatrix cworld;
    if ( hitEvent.m_xyRelToChildren )
    {
        hitEvent.m_xyRelToChildren = false;
        cworld = m_lworld;
        cworld.Invert();
    }
    a2dIterCU cuinvrt( ic, cworld );

    if ( m_hitflags.m_non || m_release || !m_flags.m_visible )
        return ( a2dCanvasObject* ) NULL;
    if ( hitEvent.m_option & a2dCANOBJHITOPTION_NOTSELECTED && GetSelected() )
        return ( a2dCanvasObject* ) NULL;

    if ( !ic.FilterObject( this ) ||
            ( ! GetIgnoreLayer() &&
              (
                  !ic.GetDrawingPart()->GetLayerRenderArray()[ m_layer ].DoRenderLayer() ||
                  ( m_root && m_root->GetLayerSetup() && !m_root->GetLayerSetup()->GetVisible( m_layer ) )
              )
            )
       )
    {
        ic.EndFilterObject( this );
        return ( a2dCanvasObject* ) NULL;
    }

    if ( ic.GetLayer() != m_layer && ic.GetLayer() != wxLAYER_ALL && ! GetIgnoreLayer() )
    {
        //the object itself will NOT be rendered for sure, but maybe its children still will!

        //object not on this layer, then children will NOT be rendered also in following cases
        if ( m_childobjects == wxNullCanvasObjectList || !m_flags.m_visiblechilds )
        {
            ic.EndFilterObject( this );
            return ( a2dCanvasObject* ) NULL;
        }
        if ( ic.GetLayer() != wxLAYER_ALL && m_flags.m_childrenOnSameLayer )
        {
            ic.EndFilterObject( this );
            return ( a2dCanvasObject* ) NULL;
        }
    }

    // Note: hit is not initialized to hit_nohit, because this is not wanted
    // with a2dCANOBJHITOPTION_ALL. This should not hurt as a a2dHit
    // initializes in its contructor to hit_nohit.

    //------------------------------------------------------------------------
    // Check if the object or any of its childs are visible and can be hit
    // on the given layer
    //------------------------------------------------------------------------

    // Check if the root object can be hit, and if not, if there are childs
    if( hitEvent.m_option & a2dCANOBJHITOPTION_NOROOT )
    {
        //the object itself cannot be hit, but maybe its children
        //object cannot be hit, then children will NOT be hit also in following cases
        if ( m_childobjects == wxNullCanvasObjectList || !m_flags.m_visiblechilds )
        {
            ic.EndFilterObject( this );
            return ( a2dCanvasObject* ) NULL;
        }
    }

    //------------------------------------------------------------------------
    // Calculate local coordinates and do a bounding box test
    //------------------------------------------------------------------------

    ic.GetInverseTransform().TransformPoint( hitEvent.m_x, hitEvent.m_y, hitEvent.m_relx, hitEvent.m_rely );

    //we add the extends since they are not part of the bbox.
    //We could do first a hit on the plain bbox, but this does not help much in speed i think.
    if ( ! GetBbox().PointInBox( hitEvent.m_relx, hitEvent.m_rely, m_worldExtend + ic.GetHitMarginWorld() + ic.ExtendDeviceToWorld( m_pixelExtend ) ) )
    {
        ic.EndFilterObject( this );
        return ( a2dCanvasObject* ) NULL;
    }

    //------------------------------------------------------------------------
    // Update the iteration context
    //------------------------------------------------------------------------
    a2dIterCU cu( ic, this );
    // prepare relative to object coordinates for derived objects
    ic.GetInverseTransform().TransformPoint( hitEvent.m_x, hitEvent.m_y, hitEvent.m_relx, hitEvent.m_rely );
    double xrel = hitEvent.m_relx;
    double yrel = hitEvent.m_rely;


    // properties and its associated objects must be on the same layer as the object itself.
    // Although in there iteration on layers may happen on nested objects, the object that is directly
    // referred to, should be on the same layer as this object itself.
    // REMARK is for some reason this is not exceptable, the rendering routines should be changed also,
    // since the hitting test is roughly the reverse of that.

    //------------------------------------------------------------------------
    // Do a hit test for properties
    //------------------------------------------------------------------------

    if (  HasDynamicProperties() && !( hitEvent.m_option & a2dCANOBJHITOPTION_NOROOT ) )
    {
        a2dNamedPropertyList::const_iterator iter;
        for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
        {
            const a2dNamedProperty* prop = *iter;
            a2dCanvasObjectPtrProperty* canvasprop = wxDynamicCast( prop, a2dCanvasObjectPtrProperty );
            if ( canvasprop && canvasprop->GetCanRender() && canvasprop->GetVisible() )
            {
                a2dCanvasObject* canvasobj = canvasprop->GetCanvasObject();
                if( canvasobj && canvasobj->GetVisible() )
                {
                    a2dCanvasObject* hit = canvasobj->IsHitWorld( ic, hitEvent );
                    if ( hit )
                    {
                        if( ! ( hitEvent.m_option & a2dCANOBJHITOPTION_ALL ) )
                        {
                            ic.EndFilterObject( this );
                            if( hitEvent.m_extendedWanted )
                            {
                                ic.Last()->m_type = hitEvent.m_how;
                                hitEvent.m_extended.push_back( a2dExtendedResultItem(
                                                                   hit,
                                                                   ic.GetParent(),
                                                                   hitEvent.m_how,
                                                                   ic.GetLevel(),
                                                                   hitEvent.m_id
                                                               ) );
                            }
                            return this;
                        }
                    }
                }
            }
        }
    }

    //------------------------------------------------------------------------
    // Do a hit test on child objects Pre render (of this object). It depends on the object and iteration context,
    // if this will be done on one layer, or by iterating through layers.
    // This stage also detect if there are post render children down there.
    //------------------------------------------------------------------------

    //detect while rendering children, if certain types of objects are there,
    //in order to render them later.
    RenderChild whichchilds;
    whichchilds.m_prerender = false;
    whichchilds.m_postrender = true;
    whichchilds.m_property = false;

    if ( m_childobjects != wxNullCanvasObjectList  && m_flags.m_visiblechilds )
    {
        //render "postrender" objects in children
        a2dCanvasObject* res = NULL;
        if ( res = IsHitWorldChildObjects( ic,  whichchilds, hitEvent ) )
        {
            ic.EndFilterObject( this );
            if( hitEvent.m_extendedWanted )
            {
                ic.Last()->m_type = hitEvent.m_how;

                hitEvent.m_extended.push_back( a2dExtendedResultItem(
                                                   res,
                                                   ic.GetParent(),
                                                   hitEvent.m_how,
                                                   ic.GetLevel(),
                                                   hitEvent.m_id
                                               ) );
            }
            return res;
        }
    }

    //------------------------------------------------------------------------
    // Do a hit test for the derived object
    //------------------------------------------------------------------------

    bool isHit = false;

    if ( !( hitEvent.m_option & a2dCANOBJHITOPTION_NOROOT ) &&
            ( ic.GetLayer() == m_layer || ic.GetLayer() == wxLAYER_ALL || m_flags.m_ignoreLayer )
       )
    {
        // Restore what might have been changed in children hits.
        // prepare relative to object coordinates for derived objects
        hitEvent.m_relx = xrel;
        hitEvent.m_rely = yrel;

        isHit = LocalHit( ic, hitEvent );

        if ( isHit )
        {
            if( hitEvent.m_extendedWanted )
            {
                ic.Last()->m_type = hitEvent.m_how;

                hitEvent.m_extended.push_back( a2dExtendedResultItem(
                                                    this,
                                                    ic.GetParent(),
                                                    hitEvent.m_how,
                                                    ic.GetLevel(),
                                                    hitEvent.m_id
                                                ) );
            }
            if( ! ( hitEvent.m_option & a2dCANOBJHITOPTION_ALL ) )
            {
				ic.SetDeepestHit( this );
                ic.EndFilterObject( this );
                return this;
            }
        }
    }

    //------------------------------------------------------------------------
    // Do a hit test on child objects Post render (of this object). It depends on the object and iteration context,
    // if this will be doenon one layer, or by iterating through layers.
    //------------------------------------------------------------------------

    if ( whichchilds.m_prerender && m_childobjects != wxNullCanvasObjectList && m_flags.m_visiblechilds )
    {
        whichchilds.m_prerender = true;
        whichchilds.m_postrender = false;
        whichchilds.m_property = false;

        //render "prerender" objects in children
        a2dCanvasObject* res = NULL;
        if ( res = IsHitWorldChildObjects( ic,  whichchilds, hitEvent ) )
        {
            if( hitEvent.m_extendedWanted )
            {
                ic.Last()->m_type = hitEvent.m_how;
                hitEvent.m_extended.push_back( a2dExtendedResultItem(
                                                   res,
                                                   ic.GetParent(),
                                                   hitEvent.m_how,
                                                   ic.GetLevel(),
                                                   hitEvent.m_id
                                               ) );
            }
            ic.EndFilterObject( this );
            return res;
        }
    }

    ic.EndFilterObject( this );
    if( hitEvent.m_option & a2dCANOBJHITOPTION_ALL && isHit )   
        return this;
    return ( a2dCanvasObject* ) NULL;
}

bool a2dCanvasObject::DoIsHitWorld( a2dIterC& WXUNUSED( ic ), a2dHitEvent& hitEvent )
{
    return false;
}

a2dCanvasObject* a2dCanvasObject::IsHitWorldChildObjects(
    a2dIterC& ic,
    RenderChild& whichchilds,
    a2dHitEvent& hitEvent
)
{
    if ( m_childobjects == wxNullCanvasObjectList || m_childobjects->empty() )
        return NULL;

    //------------------------------------------------------------------------
    // Do a layer iteration if required
    //------------------------------------------------------------------------

    // These two hit option are only valid for the first call from a2dCanvasObject::IsHitWorld()
    // Skipping the root object only makes sence for the first object.
    // Iteration on layers is only an option on root level.
    bool iterateLayerRoot = (hitEvent.m_option & ( a2dCANOBJHITOPTION_LAYERS ) ) > 0;
    wxUint32 optioncurrent = hitEvent.m_option;
    hitEvent.m_option = ( a2dHitOption ) ( hitEvent.m_option &~( a2dCANOBJHITOPTION_LAYERS | a2dCANOBJHITOPTION_NOROOT ) );

    a2dCanvasObject* foundsofar = ( a2dCanvasObject* ) NULL;

    //m_flags.m_childrenOnSameLayer == true  means render all children at once, together with the object itself.
    //Therefore even if the children objects do have other layer id's, they will appear on this object its layer in the drawing.
    //That is why we need to test them for hit together with this object.
    if (  ic.GetPerLayerMode() && ( iterateLayerRoot || m_flags.m_childrenOnSameLayer ) && m_root->GetLayerSetup() )
    {
        wxUint16 layer = ic.GetLayer();
        a2dLayerIndex::iterator itb = GetRoot()->GetLayerSetup()->GetReverseOrderSort().begin();
        a2dLayerIndex::iterator ite = GetRoot()->GetLayerSetup()->GetReverseOrderSort().end();
        // YES here we do the opposite of when rendering, hitting the top layer first
        if ( ic.GetDrawingPart()->GetReverseOrder() )
        {
            itb = GetRoot()->GetLayerSetup()->GetOrderSort().begin();
            ite = GetRoot()->GetLayerSetup()->GetOrderSort().end();
        }
        for ( a2dLayerIndex::iterator it= itb; it != ite; ++it)       
        {
            a2dLayerInfo* layerinfo = *it;

            //important!
            //if layer is visible it will be rendered
            //If an object on a layer is itself invisible it will not be drawn
            //wxLAYER_ALL is reserved and should never be part of the layersettings
            if ( ic.GetDrawingPart()->GetLayerRenderArray()[ layerinfo->GetLayer() ].DoRenderLayer()
                    && layerinfo->GetVisible() && layerinfo->GetSelectable() )
            {
                ic.SetLayer( layerinfo->GetLayer() );
                a2dCanvasObject* hit = HitChildObjectsOneLayer( ic, whichchilds, hitEvent );
                if ( hit )
                {
                    if( ! ( hitEvent.m_option & a2dCANOBJHITOPTION_ALL ) )
                    {
                        ic.SetLayer( layer );
                        hitEvent.m_option = optioncurrent;
                        return hit;
                    }
                    foundsofar = hit;
                }
            }
            ic.SetLayer( layer );
        }
    }
    else
    {
        //no layer iteration will be done and therefore only object on m_layer == layer will be rendered.
        //layer == wxLAYER_ALL is a special case, and means ignore layer order for rendering the child objects.
        //checking of availability and visibility of layer itself is handled in parent objects
        a2dCanvasObject* hit = HitChildObjectsOneLayer( ic, whichchilds, hitEvent );
        if ( hit )
        {
            if( ! ( hitEvent.m_option & a2dCANOBJHITOPTION_ALL ) )
            {
                hitEvent.m_option = optioncurrent;
                return hit;
            }
            foundsofar = hit;
        }
    }
    hitEvent.m_option = optioncurrent;
    return foundsofar;
}

a2dCanvasObject* a2dCanvasObject::HitChildObjectsOneLayer(
    a2dIterC& ic,
    RenderChild& whichchilds,
    a2dHitEvent& hitEvent
)
{
    a2dCanvasObject* foundsofar = ( a2dCanvasObject* ) NULL;

    RenderChild detectchilds;
    detectchilds.m_postrender = false;
    detectchilds.m_prerender = false;
    detectchilds.m_property = false;

    //search in reverse order, last drawn on a layer, and therefore on top, will be found first for hit
    a2dCanvasObjectList::reverse_iterator iter = m_childobjects->rbegin();
    while( iter !=  m_childobjects->rend() )
    {
        a2dCanvasObject* obj = *iter;
        int olayer = obj->GetLayer();

        detectchilds.m_postrender = detectchilds.m_prerender || !obj->GetPreRenderAsChild();
        detectchilds.m_prerender = detectchilds.m_postrender || obj->GetPreRenderAsChild();
        detectchilds.m_property = detectchilds.m_property || obj->GetIsProperty();

        //do a rough check to increase speed in common cases
        if ( (
                    ( obj->GetPreRenderAsChild() && whichchilds.m_prerender && !obj->GetIsProperty() ) ||
                    ( !obj->GetPreRenderAsChild() && whichchilds.m_postrender && !obj->GetIsProperty() ) ||
                    ( obj->GetIsProperty() && whichchilds.m_property )
                )
                // the next does work if child is a container like e.g. a2dCanvasObjectReference
                // because
                &&
                ( olayer == ic.GetLayer() || ic.GetLayer() == wxLAYER_ALL || obj->GetChildObjectsCount() || obj->GetIgnoreLayer() )
           )
        {
            if ( obj->m_flags.m_ChildOnlyTranslate )
            {
                a2dAffineMatrix iworld = m_lworld;
                iworld.Invert();
                a2dIterCU cu( ic, iworld );
                a2dAffineMatrix transworld;
                transworld.Translate( GetPosX(), GetPosY() );
                a2dIterCU cu2( ic, transworld );
                a2dCanvasObject* hit = obj->IsHitWorld( ic, hitEvent );
                if( hit )
                {
                    foundsofar = obj;
                    if( obj != hit )
                        hitEvent.m_how.m_hit = hitEvent.m_how.m_hit | a2dHit::hit_child;
                    if( ! ( hitEvent.m_option & a2dCANOBJHITOPTION_ALL ) )
                        break;
                }
            }
            else
            {
                a2dCanvasObject* hit = obj->IsHitWorld( ic, hitEvent );
                if( hit )
                {
                    foundsofar = obj;
                    if( obj != hit )
                        hitEvent.m_how.m_hit = hitEvent.m_how.m_hit | a2dHit::hit_child;
                    if( ! ( hitEvent.m_option & a2dCANOBJHITOPTION_ALL ) )
                        break;
                }
            }
        }
        iter++;
    }

    whichchilds = detectchilds;

    return foundsofar; //return top opject hit
}

static void ReportHit( a2dCanvasObject* obj, const wxString mes, bool editflag, a2dHitEvent& hitEvent )
{
#ifdef _DEBUG_REPORTHIT
    if ( editflag )
    {
        //else not event for this object
        if ( hitEvent.m_isHit )
            wxLogDebug( wxT( "hit = %s %p" ), mes, obj );
        else
            wxLogDebug( wxT( "nohit = %s %p" ), mes, obj );
    }
#else
#endif
}

bool a2dCanvasObject::LocalHit( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    hitEvent.m_isHit = false;

    //now do a proper hittest inside the derived canvas object

    // iteration context needs stroke extend, to be used for hittest in derived object
    // this extend is always in world coordinates, even if pixel pens
    double StrokeExtend = 0;
    a2dStroke stroke = GetStroke();

    if ( stroke.IsNoStroke() ) //IsNoStroke() stroke means use layer
    {
        if ( m_root && m_root->GetLayerSetup() )
        {
            a2dStroke layerpen = m_root->GetLayerSetup()->GetStroke( m_layer );
            if ( layerpen.GetPixelStroke() )
                StrokeExtend = layerpen.GetWidth() / 2;
            else
                StrokeExtend = layerpen.GetWidth() / 2;
        }
    }
    else if ( !stroke.IsSameAs( *a2dTRANSPARENT_STROKE ) )
    {
        if ( stroke.GetPixelStroke() )
            StrokeExtend = ic.GetDrawer2D()->DeviceToWorldXRel( stroke.GetWidth() / 2 );
        else
            StrokeExtend = stroke.GetWidth() / 2;
    }
    ic.SetStrokeWorldExtend( StrokeExtend );

    if ( DoIsHitWorld( ic, hitEvent ) )
    {
        //check hit mask.
        if ( m_hitflags.m_all || m_hitflags.m_visible )
            hitEvent.m_isHit = true;
        else if ( m_hitflags.m_stroke && hitEvent.m_how.IsStrokeHit() )
            hitEvent.m_isHit = true;
        else if ( m_hitflags.m_fill && hitEvent.m_how.IsFillHit() )
            hitEvent.m_isHit = true;
        else if ( m_hitflags.m_fill_non_transparent && hitEvent.m_how.IsFillHit() )
        {
            if ( !GetFill().IsSameAs( *a2dTRANSPARENT_FILL ) )
                hitEvent.m_isHit = true;
            else if ( m_root && m_root->GetLayerSetup() )
            {
                if ( !m_root->GetLayerSetup()->GetFill( m_layer ).IsSameAs( *a2dTRANSPARENT_FILL ) )
                    hitEvent.m_isHit = true;
            }
        }
        else if ( m_hitflags.m_stroke_non_transparent && hitEvent.m_how.IsStrokeHit() )
        {
            if ( !GetStroke().IsSameAs( *a2dTRANSPARENT_FILL ) )
                hitEvent.m_isHit = true;
            else if ( m_root && m_root->GetLayerSetup() )
            {
                if ( !m_root->GetLayerSetup()->GetStroke( m_layer ).IsSameAs( *a2dTRANSPARENT_STROKE ) )
                    hitEvent.m_isHit = true;
            }
        }
    }
    else
    {
        ReportHit( this, wxT( "no hit on object" ), m_flags.m_editingCopy, hitEvent );
    }
    ic.SetStrokeWorldExtend( 0 );

    return hitEvent.m_isHit;
}

bool a2dCanvasObject::ProcessCanvasObjectEvent( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    // If there is an event corridor path, and the end of the corridor was not yet reached, continue with just looking at the
    // childs. Otherwise continue processing this object.

    if ( ic.GetDrawingPart()->GetEndCorridorObject() == this )
        ic.SetFoundCorridorEnd( true );

    // If there is a corridor, go directly to childs, following the corridor flag.
    if ( ic.GetDrawingPart()->GetEndCorridorObject() && !ic.GetFoundCorridorEnd() )
    {
        if ( !m_flags.m_isOnCorridorPath )
            return false;
        else
        {
            a2dIterCU cu( ic, this );

            RenderChild whichchilds;
            whichchilds.m_prerender = true;
            whichchilds.m_postrender = true;
            whichchilds.m_property = true;

            ProcessCanvasEventChild( ic, whichchilds, hitEvent );

            return hitEvent.m_processed;
        }
    }

    if ( m_hitflags.m_non || m_release || !m_flags.m_visible )
        return false;

    //check if layer can recieve events (is rendered).
    if ( ! GetIgnoreLayer() &&
            (
                !ic.GetDrawingPart()->GetLayerRenderArray()[ m_layer ].DoRenderLayer() ||
                ( m_root && m_root->GetLayerSetup() && !m_root->GetLayerSetup()->GetVisible( m_layer ) )
            )
       )
    {
        return false;
    }

    if ( ic.GetLayer() != m_layer && ic.GetLayer() != wxLAYER_ALL && !GetIgnoreLayer() )
    {
        //the object itself will NOT be rendered for sure, but maybe its children still will!

        //object not on this layer, then children will NOT be rendered also in following cases
        if ( m_childobjects == wxNullCanvasObjectList || !m_flags.m_visiblechilds )
            return false;
        if ( ic.GetLayer() != wxLAYER_ALL && m_flags.m_childrenOnSameLayer )
            return false;
    }

    bool sentmousevent = false;
    bool localsentmousevent = false;
    // this is used for the eventual derived object hit, to generate a proper canvasobject mouse event
    bool wasHit = hitEvent.m_isHit;
    //detect for child starting at non hit nor processed
    hitEvent.m_isHit = false;

    double relx, rely;
    ic.GetInverseTransform().TransformPoint( hitEvent.m_x, hitEvent.m_y, hitEvent.m_relx, hitEvent.m_rely );
    relx = hitEvent.m_relx; 
    rely = hitEvent.m_rely;

    //first check if within bbox + its extends in world and pixels
    //will only work if they are always uptodate
    // IF at corridor end, always go here, even if not in box, event is captured by corridor.
    if ( ic.GetDrawingPart()->GetEndCorridorObject() == this ||         
         GetBbox().PointInBox( hitEvent.m_relx, hitEvent.m_rely, m_worldExtend + ic.GetHitMarginWorld() + ic.ExtendDeviceToWorld( m_pixelExtend ) ) 
       )
    {
        a2dIterCU cu( ic, this );
        // prepare relative to object coordinates for derived objects
        ic.GetInverseTransform().TransformPoint( hitEvent.m_x, hitEvent.m_y, hitEvent.m_relx, hitEvent.m_rely );

        // properties using a visible canvasobject can be hit. e.g. in case of an object tip
        a2dNamedPropertyList::const_iterator iter;
        for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
        {
            const a2dNamedProperty* prop = *iter;
            a2dCanvasObjectPtrProperty* canvasprop = wxDynamicCast( prop, a2dCanvasObjectPtrProperty );
            if ( canvasprop && canvasprop->GetVisible() )
            {
                a2dCanvasObject* canvasobj = canvasprop->GetCanvasObject();
                if( canvasobj )
                {
                    canvasobj->ProcessCanvasObjectEvent( ic, hitEvent );
                }
            }
        }

        RenderChild whichchilds;
        whichchilds.m_prerender = true;
        whichchilds.m_postrender = true;
        whichchilds.m_property = true;

        hitEvent.m_isHit = false;
        // next priority is to the children
        if ( m_flags.m_visiblechilds && m_childobjects != wxNullCanvasObjectList )
        {
            ProcessCanvasEventChild( ic, whichchilds, hitEvent );
        }

        // If the event was not processed at a deeper child level, this one ( parent object ) might be interested.
        // So events go deep first, and if hit and processed there we are done, else it travels up to its parent objects.
        if ( !hitEvent.m_processed )
        {
            if ( hitEvent.m_isHit ) // Ishit in childs but not processed.
            {
                //we had a hit on this object including its children
                sentmousevent = true;
            }
            else if ( ic.GetLayer() == m_layer || ic.GetLayer() == wxLAYER_ALL )
            {
                // end of corridor, always mouse events go through, even if not a hit.
                bool hitlocal = LocalHit( ic, hitEvent );
                localsentmousevent = ic.GetDrawingPart()->GetEndCorridorObject() == this || hitlocal;
                sentmousevent = hitEvent.m_isHit = localsentmousevent;
            }
            else
                ReportHit( this, wxT( "no hit or wrong layer" ), m_flags.m_editingCopy, hitEvent );
        }
        else
            ReportHit( this, wxT( "processed no hit" ), m_flags.m_editingCopy, hitEvent );
    }
    else
        ReportHit( this, wxT( "no box hit" ), m_flags.m_editingCopy, hitEvent );

    //restore to this level
    hitEvent.m_relx = relx; 
    hitEvent.m_rely = rely;

    // a motion event might enter an object
    if ( ( *hitEvent.m_event ).GetEventType() == wxEVT_MOTION )
    {
        wxMouseEvent* mouse = wxDynamicCast( hitEvent.m_event, wxMouseEvent );
        hitEvent.m_event->Skip( true );
        //there was a hit in a child or here.
        if ( hitEvent.m_isHit && !m_flags.m_MouseInObject )
        {
            ReportHit( this, wxT( "enter" ), m_flags.m_editingCopy, hitEvent );

            m_flags.m_MouseInObject = true;
            a2dCanvasObjectMouseEvent enter( &ic, this, wxEVT_CANVASOBJECT_ENTER_EVENT, hitEvent.m_x, hitEvent.m_y, *mouse );
            // it is needed for a tip window when the tip window is closed in a2dCanvasObject::OnEnterObject()
            // or in a2dCanvasObject::OnLeaveObject()
            hitEvent.SetProcessed( ProcessEvent( enter ) );
            // if processed, ignore that for higher level objects.
            hitEvent.m_event->Skip( true );
            hitEvent.SetProcessed( false );
        }
    }

    //an event is only sent if the event was not processed sofar
    if ( sentmousevent )
    {
        //the first object receiving an event, is the deepest hit child.
        if ( !ic.GetDeepestHit() )
        {
            //wxLogDebug(wxT("deepest %p"), this );

            ic.SetDeepestHit( this );
        }
        wxMouseEvent* mouse = wxDynamicCast( hitEvent.m_event, wxMouseEvent );
        if ( mouse )
        {
            a2dCanvasObjectMouseEvent CanvasObjectMouseEvent( &ic, this, hitEvent.m_how, hitEvent.m_x, hitEvent.m_y, *mouse );
            hitEvent.SetProcessed( ProcessEvent( CanvasObjectMouseEvent ) );
        }
        else
            hitEvent.SetProcessed( ProcessEvent( *hitEvent.m_event ) );

        if ( m_flags.m_editingCopy )
            // first event has passed.
            PROPID_FirstEventInObject->SetPropertyToObject( this, false );
    }

    // a motion event might leaf or enter an object
    if ( ( *hitEvent.m_event ).GetEventType() == wxEVT_MOTION )
    {
        wxMouseEvent* mouse = wxDynamicCast( hitEvent.m_event, wxMouseEvent );
        hitEvent.m_event->Skip( true );
        //there was a hit in a child or here.
        if ( !hitEvent.m_isHit && m_flags.m_MouseInObject )
        {
            // If child objects are on the border of a parent, they are not hittested,
            // because the mouse is already outside the parent its boundingbox.
            // No hit testing means no leave events will be generated even if a previous
            // mouse position was inside a child(s).
            // Therefore make sure all objects below are sending a leave event when mouse flag was in.
            LeaveInObjects( ic, hitEvent );

            m_flags.m_MouseInObject = false;
            a2dCanvasObjectMouseEvent leave( &ic, this, wxEVT_CANVASOBJECT_LEAVE_EVENT, hitEvent.m_x, hitEvent.m_y, *mouse );
            // it is needed for a tip window when the tip window is closed in a2dCanvasObject::OnEnterObject() or in a2dCanvasObject::OnLeaveObject()
            hitEvent.SetProcessed( ProcessEvent( leave ) );
            // if processed, ignore that for higher level objects.
            hitEvent.m_event->Skip( true );
            hitEvent.SetProcessed( false );

			ReportHit( this, wxT( "leaft" ), m_flags.m_editingCopy, hitEvent );
        }
    }

    //restore ORed result for higher level parent objects
    hitEvent.m_isHit |= wasHit;

    ic.SetStrokeWorldExtend( 0 );
    return hitEvent.m_processed;
}

bool SetPointIfCloser( const a2dPoint2D& pointToSnapTo, const a2dPoint2D& pointToSnap, a2dPoint2D& bestPointSofar, double thresHoldWorld )
{
    double dx = pointToSnap.m_x - pointToSnapTo.m_x;
    double dy = pointToSnap.m_y - pointToSnapTo.m_y;
    double snappedDx = bestPointSofar.m_x - pointToSnapTo.m_x;
    double snappedDy = bestPointSofar.m_y - pointToSnapTo.m_y;

    if (  fabs( dx ) < thresHoldWorld &&  fabs( dy ) < thresHoldWorld ) 
    {
        if ( fabs( dx ) < fabs( snappedDx ) && fabs( dy ) < fabs( snappedDy ) )
        {
            //wxLogDebug( "closer %f %f to %f %f", pointToSnap.m_x, pointToSnap.m_y, pointToSnapTo.m_x, pointToSnapTo.m_y ); 

            bestPointSofar = pointToSnap;
            return true;
        }
    }
    return false;
}

bool a2dCanvasObject::RestrictToObject( a2dIterC& ic, const a2dPoint2D& pointToSnapTo, a2dPoint2D& bestPointSofar, a2dSnapToWhatMask snapToWhat, double thresHoldWorld )
{
    // object has disabled snap?
    if ( !m_flags.m_snap_to )
        return false;

    bool res = false;
    if ( snapToWhat & a2dRestrictionEngine::snapToPins ||
         snapToWhat & a2dRestrictionEngine::snapToPinsUnconnected
       )
    {
        if ( DoConnect() && m_childobjects != wxNullCanvasObjectList && m_flags.m_hasPins )
        {
            forEachIn( a2dCanvasObjectList, m_childobjects )
            {
                a2dCanvasObject* obj = *iter;
                a2dPin* pin = wxDynamicCast( obj, a2dPin );
                if ( pin && !pin->GetRelease( ) )
                {
                    if ( snapToWhat & a2dRestrictionEngine::snapToPins ||
                         ( ( snapToWhat & a2dRestrictionEngine::snapToPinsUnconnected ) && !pin->IsConnectedTo() )
                       )
                    {
                        //if within the threshold, do snap to pin position.
                        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( pin->GetAbsX(), pin->GetAbsY() ), bestPointSofar, thresHoldWorld );
                    }
                }
            }
        }
    }
    if ( snapToWhat & a2dRestrictionEngine::snapToObjectPos )
    {
        res |= SetPointIfCloser( pointToSnapTo, GetPosXY(), bestPointSofar, thresHoldWorld );
    }

    if ( snapToWhat & a2dRestrictionEngine::snapToObjectVertexes )
    {
        //a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths( true );
        a2dCanvasObjectList* vectorpaths = GetSnapVpath( a2dRestrictionEngine::snapToObjectVertexes );
        if ( vectorpaths != wxNullCanvasObjectList )
        {
            forEachIn( a2dCanvasObjectList, vectorpaths )
            {
                a2dVectorPath* obj = wxStaticCast( ( *iter ).Get(), a2dVectorPath );
                if ( obj->RestrictToObject( ic, pointToSnapTo, bestPointSofar, a2dRestrictionEngine::snapToObjectVertexes, thresHoldWorld ) )
                {
                    res = true;
                }
            }
            delete vectorpaths;
        }
    }

    if ( snapToWhat & a2dRestrictionEngine::snapToBoundingBox )
    {
        a2dBoundingBox untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_EDIT );
        if ( untrans.GetWidth() == 0 || untrans.GetWidth() == 0 )
            untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_CHILDREN | a2dCANOBJ_BBOX_EDIT );

        double x, y, w, h;
        x = untrans.GetMinX();
        y = untrans.GetMinY();
        w = untrans.GetWidth();
        h = untrans.GetHeight();

        double tx, ty;
        m_lworld.TransformPoint( x , y, tx, ty );
        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( tx, ty ), bestPointSofar, thresHoldWorld );
        m_lworld.TransformPoint( x , y + h, tx, ty );
        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( tx, ty ), bestPointSofar, thresHoldWorld );
        m_lworld.TransformPoint( x + w , y + h, tx, ty );
        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( tx, ty ), bestPointSofar, thresHoldWorld );
        m_lworld.TransformPoint( x + w , y + h, tx, ty );
        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( tx, ty ), bestPointSofar, thresHoldWorld );
        m_lworld.TransformPoint( x + w , y, tx, ty );
        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( tx, ty ), bestPointSofar, thresHoldWorld );
        m_lworld.TransformPoint( x , y + h / 2, tx, ty );
        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( tx, ty ), bestPointSofar, thresHoldWorld );
        m_lworld.TransformPoint( x + w / 2 , y + h, tx, ty );
        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( tx, ty ), bestPointSofar, thresHoldWorld );
        m_lworld.TransformPoint( x + w , y + h / 2, tx, ty );
        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( tx, ty ), bestPointSofar, thresHoldWorld );
        m_lworld.TransformPoint( x + w / 2 , y, tx, ty );
        res |= SetPointIfCloser( pointToSnapTo, a2dPoint2D( tx, ty ), bestPointSofar, thresHoldWorld );
    }

    if ( snapToWhat & a2dRestrictionEngine::snapToObjectIntersection )
    {
    }
    if ( snapToWhat & a2dRestrictionEngine::snapToObjectSegmentsMiddle )
    {
    }
    if ( snapToWhat & a2dRestrictionEngine::snapToObjectSegments )
    {
    }

    return res;
}

a2dCanvasObjectList* a2dCanvasObject::GetSnapVpath( a2dSnapToWhatMask snapToWhat )
{
    a2dVpath* segments = new a2dVpath();
    a2dVectorPath* snappath = new a2dVectorPath( segments );
    a2dCanvasObjectList* snappathlist = new a2dCanvasObjectList();
    snappathlist->push_back( snappath );

    if ( snapToWhat & a2dRestrictionEngine::snapToPins ||
         snapToWhat & a2dRestrictionEngine::snapToPinsUnconnected
       )
    {
        if ( m_childobjects != wxNullCanvasObjectList && m_flags.m_hasPins )
        {
            forEachIn( a2dCanvasObjectList, m_childobjects )
            {
                a2dCanvasObject* obj = *iter;
                a2dPin* pin = wxDynamicCast( obj, a2dPin );
                if ( pin && !pin->GetRelease( ) )
                {
                    if ( snapToWhat & a2dRestrictionEngine::snapToPins ||
                            ( snapToWhat & a2dRestrictionEngine::snapToPinsUnconnected  && !pin->IsConnectedTo() )
                       )
                    {
                        a2dVpathSegment* seg = new a2dVpathSegment( pin->GetAbsX(), pin->GetAbsY(), a2dPATHSEG_MOVETO );
                        segments->Add( seg );
                    }
                }
            }
        }
    }
    if ( snapToWhat & a2dRestrictionEngine::snapToObjectPos )
    {
        a2dVpathSegment* seg = new a2dVpathSegment( GetPosX(), GetPosY(), a2dPATHSEG_MOVETO );
        segments->Add( seg );
    }

    if ( snapToWhat & a2dRestrictionEngine::snapToObjectVertexes )
    {
        // if the next GetAsCanvasVpaths( true ) is to complex for snapping, derive  a2dCanvasObject::GetSnapVpath() for that object.
        a2dCanvasObjectList* vectorpaths = GetAsCanvasVpaths( true );
        if ( vectorpaths != wxNullCanvasObjectList )
        {
            snappathlist->TakeOverFrom( vectorpaths );
            delete vectorpaths;
        }
    }

    if ( snapToWhat & a2dRestrictionEngine::snapToBoundingBox )
    {
        a2dBoundingBox untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_EDIT );
        if ( untrans.GetWidth() == 0 || untrans.GetWidth() == 0 )
            untrans = GetUnTransformedBbox( a2dCANOBJ_BBOX_CHILDREN | a2dCANOBJ_BBOX_EDIT );

        double x, y, w, h;
        x = untrans.GetMinX();
        y = untrans.GetMinY();
        w = untrans.GetWidth();
        h = untrans.GetHeight();

        double tx, ty;
        m_lworld.TransformPoint( x , y, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x , y + h, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w , y + h, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w , y + h, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w , y, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        /*
        m_lworld.TransformPoint( x , y + h / 2, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w / 2 , y + h, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w , y + h / 2, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        m_lworld.TransformPoint( x + w / 2 , y, tx, ty );
        segments->Add( new a2dVpathSegment( tx, ty, a2dPATHSEG_MOVETO ) );
        */
    }

    if ( snapToWhat & a2dRestrictionEngine::snapToObjectSegments )
    {
    }

    return snappathlist;
}

a2dVertexArray* a2dCanvasObject::GetAlignmentPoints()
{
	a2dVertexArray* alignPoints = new  a2dVertexArray();
	a2dBoundingBox bbox;

	//bbox like points
    if ( m_childobjects != wxNullCanvasObjectList && m_flags.m_hasPins )
    {
		a2dBoundingBox childbox;
		a2dBoundingBox childboxOnlyTranslate;

		if ( m_flags.m_visiblechilds )
		{
			a2dAffineMatrix transworld;
			transworld.Translate( GetPosX(), GetPosY() );
			forEachIn( a2dCanvasObjectList, m_childobjects )
			{
				a2dCanvasObject* obj = *iter;
				//calculate bbox only for visible objects
				a2dPin* pin = wxDynamicCast( obj, a2dPin );
				if ( !pin && obj && obj->GetVisible() )
				{
					if ( obj->m_flags.m_ChildOnlyTranslate )
					{
						childboxOnlyTranslate = obj->GetBbox();
						childboxOnlyTranslate.MapBbox( transworld );
					}
					else
						childbox.Expand( obj->GetBbox() );
				}
			}
		}

		if ( childboxOnlyTranslate.GetValid() ) //no child or empty child secure
			childbox.Expand( childboxOnlyTranslate );

		if ( childbox.GetValid() ) //no child or empty child secure
		{
			childbox.MapBbox( m_lworld );
			bbox.Expand( childbox );
		}

		if ( !m_bbox.GetValid() )
		{
			m_bbox = DoGetUnTransformedBbox();
			if ( m_bbox.GetValid() )
			{
				m_bbox.MapBbox( m_lworld );
			}
		}
		bbox.Expand( m_bbox );

		if ( !bbox.GetValid() )
		{
			//no objects, therefore make the bounding box the x,y of the object
			bbox.Expand( m_lworld.GetValue( 2, 0 ), m_lworld.GetValue( 2, 1 ) );
		}
    }
	else
	{   //just bbox will do.
		bbox = GetBbox();
	}

	alignPoints->push_back( new a2dLineSegment( bbox.GetMinX(), bbox.GetMinY() ));
	alignPoints->push_back( new a2dLineSegment( bbox.GetMinX(), bbox.GetMaxY() ));
	alignPoints->push_back( new a2dLineSegment( bbox.GetMaxX(), bbox.GetMaxY() ));
	alignPoints->push_back( new a2dLineSegment( bbox.GetMaxX(), bbox.GetMinY() ));

	alignPoints->push_back( new a2dLineSegment( bbox.GetMinX(), bbox.GetCentre().m_y ));
	alignPoints->push_back( new a2dLineSegment( bbox.GetCentre().m_x, bbox.GetMaxY() ));
	alignPoints->push_back( new a2dLineSegment( bbox.GetMaxX(), bbox.GetCentre().m_y ));
	alignPoints->push_back( new a2dLineSegment( bbox.GetCentre().m_x, bbox.GetMinY() ));

	alignPoints->push_back( new a2dLineSegment( bbox.GetMinX(), bbox.GetCentre().m_y ));

	return alignPoints;
}

a2dBoundingBox a2dCanvasObject::GetAlignmentBbox()
{
	a2dBoundingBox alignbox;
	a2dVertexArray* alignpoints = GetAlignmentPoints();

	alignbox.Expand( alignpoints->Item(0)->m_x, alignpoints->Item(0)->m_y );
	alignbox.Expand( alignpoints->Item(1)->m_x, alignpoints->Item(1)->m_y );
	alignbox.Expand( alignpoints->Item(2)->m_x, alignpoints->Item(2)->m_y );
	alignbox.Expand( alignpoints->Item(3)->m_x, alignpoints->Item(3)->m_y );
	delete alignpoints;

	return alignbox;
}

bool a2dCanvasObject::SwitchChildNamed( const wxString& objectname, a2dCanvasObject* newobject )
{
    if ( m_childobjects != wxNullCanvasObjectList )
        return m_childobjects->SwitchObjectNamed( objectname, newobject );

    return false;
}

int a2dCanvasObject::CollectObjects( a2dCanvasObjectList* total, const wxString& classname, a2dCanvasObjectFlagsMask mask,
                                     const a2dPropertyId* id, const a2dBoundingBox& bbox ) const
{
    if ( m_childobjects != wxNullCanvasObjectList )
    {
        return m_childobjects->CollectObjects( total, classname, mask, id, bbox );
    }
    return 0;
}

a2dCanvasObject* a2dCanvasObject::Find( const wxString& objectname, const wxString& classname, a2dCanvasObjectFlagsMask mask, const a2dPropertyId* propid, const wxString& valueAsString, wxUint32 id ) const
{
    if ( m_childobjects != wxNullCanvasObjectList )
        return m_childobjects->Find( objectname, classname, mask, propid, valueAsString, id );

    return ( a2dCanvasObject* ) NULL;
}

a2dCanvasObject* a2dCanvasObject::Find( a2dCanvasObject* obj ) const
{
    if ( m_childobjects != wxNullCanvasObjectList  )
        return m_childobjects->Find( obj );
    else
        return 0;
}

void a2dCanvasObject::AddPending( a2dIterC& ic )
{
    bool childpending = false;
    {
        a2dIterCU cu( ic, this );

        if ( m_childobjects != wxNullCanvasObjectList && m_flags.m_visiblechilds )
        {
            forEachIn( a2dCanvasObjectList, m_childobjects )
            {
                a2dCanvasObject* obj = *iter;
                //no need to test clipping of bounding against ic.GetDrawer2D(),
                //since only pending objects are tested which is not expensive
                if ( obj )
                {
                    childpending = ( obj->IsPending() || obj->m_release ) || childpending;
                    obj->AddPending( ic );
                }
            }
        }

        DoAddPending( ic );
    }

    // automatically set layers available for new objects on empty layers.
    ic.GetDrawingPart()->GetLayerRenderArray()[ m_layer ].IncrementObjectCount();
    if ( m_root && m_root->GetLayerSetup() )
    {
        a2dLayerInfo* thislayer = m_root->GetLayerSetup()->GetLayerIndex()[ m_layer ];
        thislayer->IncrementObjectCount();
    }

    // Resize to children, requires to set it pending here, in order to add current bbox.
    // And in a2dCanvasObject::Update() in DoUpdate() m_flags.m_resizeToChilds must be tested by the
    // derived object to resize itself to its children.

    if( m_flags.m_resizeToChilds )
        m_flags.m_pending |= childpending;

    if ( IsPending() || m_release )
    {
        // store/update or trigger the availability of layers for this a2dDrawingPart.
        if ( m_release )
        {
            //releasing an object, may lead to a layer becoming empty.
            //Understand that the object can be shown on one view and not another
            //view.
            if (  ic.GetUpdateHint() & a2dCANVIEW_UPDATE_PENDING_POSTUPDATE )
            {
                //a2dLayerInfo* thislayer = m_root->GetLayerSetup()->GetLayerIndex()[ m_layer ];
                //thislayer->DecrementObjectCount();
                ic.GetDrawingPart()->GetLayerRenderArray()[ m_layer ].DecrementObjectCount();
            }
        }

        //first we search for bbox properties, which are taking priority above complete bbox updating
        bool foundMiniUpdate = false;
        a2dNamedPropertyList::const_iterator iter;
        for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
        {
            const a2dNamedProperty* prop = *iter;
            if ( prop->GetId() == PROPID_BoundingBox )
            {
                ic.GetDrawingPart()->AddPendingUpdateArea( this, wxStaticCast( prop, a2dBoudingBoxProperty )->GetValue() );
                foundMiniUpdate = true;
            }
        }

        // if no bbox properties are found, we use the complete boudingbox instead.
        if ( !foundMiniUpdate )
        {
            //wxLogDebug( "add bbox area %p", this );

            wxRect absarea = GetAbsoluteArea( ic );
            ic.GetDrawingPart()->AddPendingUpdateArea( this, absarea );
        }
    }

}

void a2dCanvasObject::DoAddPending( a2dIterC& WXUNUSED( ic ) )
{
}

void a2dCanvasObject::UpdateViewDependentObjects( a2dIterC& ic )
{
    //if an object is view specific, then it is only visible on a certain view,
    //so also its children and all below will not be visible, therefore no need to check them.
    a2dDrawingPart* view = wxStaticCastNull( PROPID_ViewSpecific->GetPropertyValue( this ).Get(), a2dDrawingPart );
    if ( view != ic.GetDrawingPart() )
        return;

    //objects which depend on a certain view when it concerns size, can be found
    // at each level, so we really need to recurse always.
    a2dDrawingPart* view2 = wxStaticCastNull( PROPID_ViewDependent->GetPropertyValue( this ).Get(), a2dDrawingPart );
    if ( view2 == ic.GetDrawingPart() )
    {
        DoUpdateViewDependentTransform( ic );
    }

    a2dIterCU cu( ic, this );

    DoUpdateViewDependentObjects( ic );

    if ( m_childobjects != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
//          if(obj->m_childpixelsize)
            if ( obj )
                obj->UpdateViewDependentObjects( ic );
        }
    }
}

void a2dCanvasObject::DoUpdateViewDependentTransform( a2dIterC& ic )
{
    a2dNamedProperty* propSpec = GetProperty( PROPID_IntViewDependTransform );
    if ( !propSpec )
    {
        a2dAffineMatrix tworld = ic.GetTransform();

        a2dAffineMatrix withoutScale( ic.GetMappingTransform() * tworld * m_lworld );
        withoutScale.RemoveScale();
        a2dAffineMatrix invertMapping( ic.GetMappingTransform() );
        invertMapping.Invert();
        a2dAffineMatrix newRelative( ic.GetInverseTransform() );
        newRelative *= invertMapping;
        newRelative *= withoutScale;
        m_lworld = newRelative;
        SetPending( true );
        delete propSpec;
    }
}

void a2dCanvasObject::DoUpdateViewDependentObjects( a2dIterC& ic )
{
    a2dNamedProperty* propSpec = GetProperty( PROPID_IntViewDependTransform );
    if ( propSpec )
    {
        a2dMatrixProperty* propMatrix = wxStaticCast( propSpec, a2dMatrixProperty );

        a2dAffineMatrix tworld = ic.GetTransform();
        a2dAffineMatrix withoutScale( ic.GetMappingTransform() * tworld );
        withoutScale.RemoveScale();
        a2dAffineMatrix invertMapping( ic.GetMappingTransform() );
        invertMapping.Invert();
        a2dAffineMatrix newRelative( ic.GetInverseTransform() );
        newRelative *= invertMapping;
        newRelative *= withoutScale;

        propMatrix->SetValue( newRelative );
        SetPending( true );
        delete propSpec;
    }

}

void a2dCanvasObject::DependencyPending( a2dWalkerIOHandler* handler )
{
    //the default highlight is a boundingbox, therefore if a nested child has changed also
    //set this object pending.
    a2dCanvasObject* parent = wxDynamicCast( handler->GetParent(), a2dCanvasObject );
    if ( parent && parent->GetHighLight() && !parent->GetPending() && GetPending() )
    {
        parent->SetPending( true );
    }
}

void a2dCanvasObject::DependentCommands( a2dCanvasObject* parent, a2dCommand* command )
{
}

void a2dCanvasObject::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    if ( m_childobjects->size() == 0 )
    {
        double x1;
        double y1;
        ic.GetTransform().TransformPoint( 0, 0, x1, y1 );
        int dx = ic.GetDrawer2D()->WorldToDeviceX( x1 );
        int dy = ic.GetDrawer2D()->WorldToDeviceY( y1 );

        ic.GetDrawer2D()->PushIdentityTransform();
        ic.GetDrawer2D()->SetDrawerStroke( *a2dBLACK_STROKE );
        ic.GetDrawer2D()->DrawLine( dx - 3, dy,  dx + 4, dy );
        ic.GetDrawer2D()->DrawLine( dx, dy + 3,  dx, dy - 4 );
        ic.GetDrawer2D()->PopTransform();
    }
}

void a2dCanvasObject::Render( a2dIterC& ic, OVERLAP clipparent )
{
    // Check if the object is not about to be deleted
    if ( m_release )
        return;

    if ( !ic.FilterObject( this ) )
    {
        ic.EndFilterObject( this );
        return;
    }

    //Now we will either render the object or children or both on the given layer.
    //OR in case layer == wxLAYER_ALL, everything will be rendered at once.
    //Style of objects will be according to their own style properties of using the object
    //m_layer id.

    //get bbox at the absolute position
    //first map without pen width which is not included in the boundingbox

    // if 0 in width or height, nothing will be rendered in principle.
    // But the stroke width is not included in bbox, so it might still be visible.
    if ( !m_flags.m_NoRenderCanvasObject && GetBbox().GetWidth() && GetBbox().GetHeight() )
    {
        a2dBoundingBox absarea = GetBbox();
        if ( !ic.GetTransform().IsIdentity() )
            absarea.MapBbox( ic.GetTransform() );

        if ( m_worldExtend )
            absarea.Enlarge( m_worldExtend );
        if ( m_pixelExtend )
            absarea.Enlarge( ic.GetDrawer2D()->DeviceToWorldXRel( m_pixelExtend ) );

        double size = wxMax( absarea.GetWidth(), absarea.GetHeight() );
        if ( ic.GetDrawer2D()->GetPrimitiveThreshold() != 0
                && size < ic.GetDrawer2D()->DeviceToWorldXRel( ic.GetDrawer2D()->GetPrimitiveThreshold() )  )
        {
            if ( ic.GetDrawer2D()->GetThresholdDrawRectangle() )
            {
                //set only style
                //if the propertylist NOT contains style properties,
                //we need to set the style according to the layer setting.
                a2dLayers* layers = m_root->GetLayerSetup();
                bool fillset = false;
                bool strokeset = false;
                bool viewSpecific = false;
                if ( HasDynamicProperties() )
                {
                    a2dNamedPropertyList::const_iterator iter;
                    for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
                    {
                        const a2dNamedProperty* prop = *iter;
                        a2dObject* obj = prop->GetRefObjectNA();
                        if ( wxDynamicCast( prop , a2dFillProperty ) )
                        {
                            a2dFill fill = wxStaticCast( prop, a2dFillProperty )->GetValue();
                            if ( m_flags.m_filled )
                            {
                                if ( fill.IsSameAs( *a2dINHERIT_FILL ) )
                                    fillset = true;
                                else if ( fill.IsNoFill() )
                                    fillset = false;
                                else
                                {
                                    ic.GetDrawer2D()->SetDrawerFill( fill );
                                    fillset = true;
                                }
                            }
                            else
                            {
                                ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
                                fillset = true;
                            }

                        }
                        else if ( wxDynamicCast( prop, a2dStrokeProperty ) )
                        {
                            a2dStroke stroke = wxStaticCast( prop, a2dStrokeProperty )->GetValue();
                            if ( stroke.IsSameAs( *a2dINHERIT_STROKE ) )
                                strokeset = true;
                            else if ( stroke.IsNoStroke() )
                                strokeset = false;
                            else
                            {
                                strokeset = true;
                                ic.GetDrawer2D()->SetDrawerStroke( stroke );
                            }

                        }
                        else if ( prop->GetId() == PROPID_ViewSpecific && prop->GetRefObject() != ic.GetDrawingPart() )
                        {
                            viewSpecific = true;
                        }
                    }
                }

                if ( viewSpecific )
                {
                    ic.EndFilterObject( this );
                    return;
                }

                //if style not set by the properties ,do it now using the layers.
                if ( layers )
                {
                    if ( !fillset  )
                        ic.GetDrawer2D()->SetDrawerFill( layers->GetFill( m_layer ) );
                    if ( !strokeset )
                        ic.GetDrawer2D()->SetDrawerStroke( layers->GetStroke( m_layer ) );
                    //ic.GetDrawer2D()->SetLayer( m_layer );
                }
                else
                {
                    if ( !fillset  )
                        ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetDefaultFill() );
                    if ( !strokeset )
                        ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetDefaultStroke() );
                    ic.GetDrawer2D()->SetLayer( 0 );
                }


                ic.GetDrawer2D()->DrawRoundedRectangle( GetBbox().GetMinX(), GetBbox().GetMinY(),
                                                        GetBbox().GetWidth(), GetBbox().GetHeight(), 0 );

                if ( m_flags.m_HighLight )
                    DrawHighLighted( ic );
            }
            ic.EndFilterObject( this );
            return;
        }
    }

    // a2dIterCU scope ends for DrawHighLighted( ic )
    {
        //here the Drawer gets a new relative transform
        //Every call for drawing something on it, will use it from now on.
        // we go one level deeper in transform to the child level
        a2dIterCU cu( ic, this );

        //if ( m_flags.m_HighLight )
        //    DrawHighLighted( ic );


        //if the propertylist NOT contains style properties,
        //we need to set the style according to the layer setting.
        bool fillset = false;
        bool strokeset = false;
        bool viewSpecific = false;

        a2dLayers* layers = m_root->GetLayerSetup();

        //investigate if there are properties which influence the rendering.
        //
        if ( HasDynamicProperties() )
        {
            bool firstclip = false;

            a2dNamedPropertyList::const_iterator iter;
            for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
            {
                const a2dNamedProperty* prop = *iter;
                a2dObject* obj = prop->GetRefObjectNA();
                if ( wxDynamicCast( prop , a2dFillProperty ) )
                {
                    a2dFill fill = wxStaticCast( prop, a2dFillProperty )->GetValue();
                    if ( m_flags.m_filled )
                    {
                        if ( fill.IsSameAs( *a2dINHERIT_FILL ) )
                            fillset = true;
                        else if ( fill.IsNoFill() )
                            fillset = false;
                        else
                        {
                            ic.GetDrawer2D()->SetDrawerFill( fill );
                            fillset = true;
                        }
                    }
                    else
                    {
                        ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
                        fillset = true;
                    }

                }
                else if ( wxDynamicCast( prop, a2dStrokeProperty ) )
                {
                    a2dStroke stroke = wxStaticCast( prop, a2dStrokeProperty )->GetValue();
                    if ( stroke.IsSameAs( *a2dINHERIT_STROKE ) )
                        strokeset = true;
                    else if ( stroke.IsNoStroke() )
                        strokeset = false;
                    else
                    {
                        strokeset = true;
                        ic.GetDrawer2D()->SetDrawerStroke( stroke );
                    }

                }
                else if ( !m_flags.m_editingCopy && wxDynamicCast( prop, a2dClipPathProperty ) )
                {
                    //Pushing a clipping area is relative to the drawer its matrix.
                    wxStaticCast( prop, a2dClipPathProperty )->PushClip( ic.GetDrawingPart(), firstclip ? a2dCLIP_OR : a2dCLIP_AND );
                    firstclip = true;
                }
                else if ( m_flags.m_showshadow && wxDynamicCast( prop, a2dShadowStyleProperty ) )
                {
                    a2dShadowStyleProperty* shadow = wxStaticCast( prop, a2dShadowStyleProperty );
                    double dx = cos( shadow->GetExtrudeAngle() ) * shadow->GetExtrudeDepth();
                    double dy = sin( shadow->GetExtrudeAngle() ) * shadow->GetExtrudeDepth();

                    a2dAffineMatrix tworld = ic.GetTransform();

                    SetDrawerStyle( ic, shadow );
                    tworld.Translate( dx, dy );

                    //here the Drawer gets a new relativetoabsolute transform
                    //Every call for drawing something on it, will use it.
                    ic.GetDrawer2D()->SetTransform( tworld );

                    DoRender( ic, clipparent );
                    tworld.Translate( -dx, -dy );

                    //restore
                    ic.GetDrawer2D()->SetTransform( tworld );
                }
                else if ( wxDynamicCast( prop, a2dCanvasObjectPtrProperty ) )
                {
                    a2dCanvasObjectPtrProperty* torender = wxStaticCast( prop, a2dCanvasObjectPtrProperty );

                    if (
                        torender->GetCanRender() &&
                        torender->GetPreRender() &&
                        torender->GetCanvasObject() &&
                        torender->GetVisible() &&
                        ( !torender->GetSelectedOnly() || IsSelected() )
                    )
                        torender->GetCanvasObject()->Render( ic, clipparent );
                }
                else if ( prop->GetId() == PROPID_ViewSpecific && prop->GetRefObject() != ic.GetDrawingPart() )
                {
                    viewSpecific = true;
                }
            }
        }

        if ( viewSpecific )
        {
            ic.EndFilterObject( this );
            return;
        }

        //if style not set by the properties ,do it now using the layers.
        if ( layers )
        {
            if ( !fillset  )
                ic.GetDrawer2D()->SetDrawerFill( layers->GetFill( m_layer ) );
            if ( !strokeset )
                ic.GetDrawer2D()->SetDrawerStroke( layers->GetStroke( m_layer ) );
            //ic.GetDrawer2D()->SetLayer( m_layer );
        }
        else
        {
            if ( !fillset  )
                ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetDefaultFill() );
            if ( !strokeset )
                ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetDefaultStroke() );
            ic.GetDrawer2D()->SetLayer( 0 );
        }

        //save the current style state of the a2dDrawingPart, in order to restore style of object when needed.
        a2dFill currentdrawer_fill = ic.GetDrawer2D()->GetDrawerFill();
        a2dStroke currentdrawer_stroke = ic.GetDrawer2D()->GetDrawerStroke();

        //detect while rendering children, if certain types of objects are there,
        //in order to render them later.
        RenderChild whichchilds;
        whichchilds.m_prerender = true;
        whichchilds.m_postrender = false;
        whichchilds.m_property = false;

        if ( m_childobjects != wxNullCanvasObjectList && m_flags.m_visiblechilds )
        {
            //render "prerender" objects in children
            RenderChildObjects( ic, whichchilds, clipparent );

            //RenderChildObject restores style as was before calling it
        }

        //if the object has sub objects (apart from the childobjects which are handled here),
        //those subobjects must be rendered by iterating on layer when needed/wanted, simular to child objects.
        //We do not iterate here, since that is only needed if indeed there or subobjects.
        //This will be know in a "wxDerivedCanvasObject DoRender".
        //SO parent objects that call this function here, MUST:
        // 1-  clip object against area to redraw.
        // 2-  iterate on layers when needed.
        if ( ic.GetLayer() == m_layer || ic.GetLayer() == wxLAYER_ALL || m_flags.m_ignoreLayer )
        {

            // DoRender() should not change the matrix of the drawing context, or at least restore it.
            // To prevent taking time for this.
            //a2dAffineMatrix tworld = ic.GetTransform();

            DoRender( ic, clipparent );

            //restore
            //ic.GetDrawer2D()->SetTransform( tworld );

            //never know what happened in DoRender, so better restore object style
            ic.GetDrawer2D()->SetDrawerFill( currentdrawer_fill );
            ic.GetDrawer2D()->SetDrawerStroke( currentdrawer_stroke );
        }

        if ( whichchilds.m_postrender && m_childobjects != wxNullCanvasObjectList  && m_flags.m_visiblechilds )
        {
            whichchilds.m_prerender = false;
            whichchilds.m_postrender = true;
            whichchilds.m_property = false;

            //render "postrender" objects in children
            RenderChildObjects( ic, whichchilds, clipparent );

            //RenderChildObject restores style as was before calling it
        }

        //renderproperties is set true when a special a2dCanvasObject is used to display normally
        //invisible properties. This is indicated by the flag IsProperty()
        //So the objects rendering in some way a property, follow on top of all
        //other children.
        if ( whichchilds.m_property && m_childobjects != wxNullCanvasObjectList && m_flags.m_visiblechilds )
        {
            whichchilds.m_prerender = false;
            whichchilds.m_postrender = false;
            whichchilds.m_property = true;
            //render post object children
            RenderChildObjects( ic, whichchilds, clipparent );

            //RenderChildObject restores style as was before calling it
        }

        // rendering object tip on top.
        if ( HasDynamicProperties() )
        {

            a2dNamedPropertyList::const_iterator iter;
            for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
            {
                const a2dNamedProperty* prop = *iter;
                a2dObject* obj = prop->GetRefObjectNA();
                if ( wxDynamicCast( prop, a2dClipPathProperty ) && !m_flags.m_editingCopy )
                {
                    wxStaticCast( prop, a2dClipPathProperty )->PopClip( ic.GetDrawingPart() );
                }


                if ( wxDynamicCast( prop, a2dCanvasObjectPtrProperty ) )
                {
                    //__OBJECTTIP__ can be post and prerendered
                    a2dCanvasObjectPtrProperty* torender = wxStaticCast( prop, a2dCanvasObjectPtrProperty );

                    if (
                        torender->GetCanRender() &&
                        !torender->GetPreRender() &&
                        torender->GetCanvasObject() &&
                        torender->GetVisible() &&
                        ( !torender->GetSelectedOnly() || IsSelected() )
                    )
                    {
                        torender->GetCanvasObject()->Render( ic, clipparent );
                    }
                }
                // properties that have a a2dCanvasObject in some manner, can be rendered aswell.
                // But only if the SetCanRender is set true.
                // a2dClipPathProperty is an example of that.
                else if ( prop->GetCanRender() && obj && prop->GetVisible()
                        )
                {
                    a2dCanvasObject* canobj = wxDynamicCast( obj, a2dCanvasObject );
                    if ( canobj )
                        canobj->Render( ic, clipparent );
                }
            }
        }
    }

    if ( m_flags.m_HighLight )
        DrawHighLighted( ic );

    ic.EndFilterObject( this );
}

void a2dCanvasObject::DrawHighLighted( a2dIterC& ic )
{
    //a2dAffineMatrix cworld = ic.GetParentTransform();
    //a2dAffineMatrix tworld = ic.GetTransform();

    ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetHighLightStroke() );
    ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetHighLightFill() );

    ic.GetDrawer2D()->DrawRoundedRectangle( m_bbox.GetMinX(), m_bbox.GetMinY(), m_bbox.GetWidth(), m_bbox.GetHeight() , 0 );
}

a2dBoundingBox a2dCanvasObject::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    return a2dBoundingBox(); // return a non valid boundingbox ( will be expanded later on )
}

a2dBoundingBox a2dCanvasObject::GetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox box = DoGetUnTransformedBbox( flags );
    if ( ( flags & a2dCANOBJ_BBOX_CHILDREN )
            && m_childobjects != wxNullCanvasObjectList
            && m_flags.m_visiblechilds )
    {
        a2dCanvasObjectList::iterator iter;
        for( iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj )
                obj->Update( a2dCanvasObject::update_save );
        }

        for( iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            // this is calculating bbox only for visible objects
            if( obj && obj->GetVisible() )
                box.Expand( obj->GetBbox() );
        }
    }

    //in case there is not a derived object, and there are no children,
    //we still like to have a boundingbox.
    if ( !box.GetValid() )
    {
        //no objects make the bounding box the x,y of the group
        //because the bouding box is untransformed, this is 0,0
        box.Expand( 0, 0 );
    }

    return box;
}

a2dBoundingBox a2dCanvasObject::GetCalculatedBoundingBox( int nChildLevels )
{
    a2dBoundingBox box = DoGetUnTransformedBbox();
    if ( nChildLevels >= 1 && m_childobjects != wxNullCanvasObjectList && m_flags.m_visiblechilds )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj )
                box.Expand( obj->GetCalculatedBoundingBox( nChildLevels - 1 ) );
        }
    }

    //in case there is not a derived object, and there are no children,
    //we still like to have a boundingbox.
    if ( !box.GetValid() )
    {
        //no objects make the bounding box the x,y of the group
        box.Expand( 0, 0 );
    }

    box.MapBbox( m_lworld );

    return box;
}

bool a2dCanvasObject::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        m_bbox = DoGetUnTransformedBbox();
        if ( m_bbox.GetValid() )
        {
            m_bbox.MapBbox( m_lworld );
            return true;
        }
    }
    return false;
}


bool a2dCanvasObject::RemoveReleased()
{
	bool changed = false;
	a2dCanvasObjectList::iterator iter = m_childobjects->begin();
    while( iter != m_childobjects->end() )
    {
        a2dCanvasObjectList::value_type obj = *iter;

        if ( obj && obj->m_release )
        {
			obj->RemoveReleased();

            // There may be multiple references, if just one is to be released, the parent property should have
            // bin set to obj.
            // Search parent property in obj, to know this is the one to delete the obj child from.
            a2dCanvasObject* parent = PROPID_Parent->GetPropertyValue( obj );
            if ( ( parent == NULL ) || ( parent != NULL && parent == this ) )
            {
                if ( obj->HasPins() )
                    obj->DisConnectWith();
                wxUint16 layer = obj->GetLayer();
                iter = m_childobjects->erase( iter );
                changed = true;
                if ( m_root && m_root->GetLayerSetup() )
                {
                    a2dLayerInfo* thislayer = m_root->GetLayerSetup()->GetLayerIndex()[ layer ];
                    if ( thislayer )
                        thislayer->DecrementObjectCount();
                }
            }
        }
        else
            iter++;
    }
	return changed;
}

bool a2dCanvasObject::Update( UpdateMode mode )
{
    a2dBoundingBox oldbbox = m_bbox;

    a2dLayers* layers = NULL;
    if ( m_root && m_root->GetLayerSetup() )
    {
        layers = m_root->GetLayerSetup();
    }

    bool changed = false; //true in the end if this level or child level has changed and needs recalculation
    // Release all childs with m_release = true
    if ( m_childobjects != wxNullCanvasObjectList )
    {
        wxString str = GetClassInfo()->GetClassName();
        m_flags.m_NoRenderCanvasObject = str.IsSameAs( wxT( "a2dCanvasObject" ) );
        if ( m_flags.m_NoRenderCanvasObject )
            m_flags.m_NoRenderCanvasObject = m_flags.m_NoRenderCanvasObject && m_childobjects->size() == 0;

		changed = RemoveReleased();

        if ( m_childobjects->empty() && !GetIgnoreLayer() )
        {
            delete m_childobjects;
            m_childobjects = wxNullCanvasObjectList;
        }
    }

    if ( ( mode & updatemask_countObjects ) )
    {
        if ( layers )
        {
            a2dLayerInfo* thislayer = layers->GetLayerIndex()[ m_layer ];
            thislayer->IncrementObjectCount();
        }
    }

    // Check if the object itself needs a bbox calculation
    // Calculate stroke and pixel extends
    if ( ( mode == updatemask_force ) || GetPending() || !m_bbox.GetValid() )
    {
        // ok, the object did change and needs a calculation
        changed = true;
        m_flags.m_HasToolObjectsBelow = m_flags.m_editingCopy;
        m_flags.m_HasSelectedObjectsBelow = m_flags.m_selected;
        m_flags.m_NoRenderCanvasObject = false;

        m_pixelExtend = 0;
        m_worldExtend = 0;

        bool hasStrokeProp = false;

        a2dNamedPropertyList::const_iterator iter;
        for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
        {
            const a2dNamedProperty* prop = *iter;
            a2dObject* obj = prop->GetRefObjectNA();
            if ( wxDynamicCast( prop , a2dStrokeProperty ) )
            {
                hasStrokeProp = true;
                a2dStroke stroke = wxStaticCast( prop , a2dStrokeProperty )->GetValue();

                if ( stroke.IsNoStroke() ) // IsNoStroke() stroke means use layer
                {
                    if ( layers )
                    {
                        a2dStroke layerpen = layers->GetStroke( m_layer );
                        if ( layerpen.GetPixelStroke() )
                        {
                            m_pixelExtend = ( wxUint16 ) ( layerpen.GetWidth() / 2 );
                        }
                        else
                        {
                            m_worldExtend = layerpen.GetWidth() / 2;
                        }
                    }
                }
                else if ( !stroke.IsSameAs( *a2dTRANSPARENT_STROKE ) )
                {
                    if ( stroke.GetPixelStroke() )
                    {
                        m_pixelExtend = ( wxUint16 ) ( stroke.GetWidth() / 2 );
                    }
                    else
                    {
                        m_worldExtend = stroke.GetWidth() / 2;
                    }
                }

            }
            else if ( GetAlgoSkip() && ( prop->GetId() == a2dCanvasObject::PROPID_ToolObject ||
                      prop->GetId() == a2dCanvasObject::PROPID_ToolDecoration )
                    )
                m_flags.m_HasToolObjectsBelow = true;
        }
        //no properties. meaning no stroke also.
        if ( !hasStrokeProp && layers )
        {
            a2dStroke layerpen = layers->GetStroke( m_layer );
            if ( layerpen.GetPixelStroke() )
            {
                m_pixelExtend = ( wxUint16 ) ( layerpen.GetWidth() / 2 );
            }
            else
            {
                m_worldExtend = layerpen.GetWidth() / 2;
            }
        }
    }

    //the boundingbox consists of three parts:
    // derived object
    // child objects
    // properties
    // If one or more of those changed, all of them need to be recalculated

    bool childchanged = false;
    //check childs for change by calling its Update()
    if ( m_childobjects != wxNullCanvasObjectList && m_flags.m_visiblechilds )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            if ( !obj )
                continue;

            childchanged = obj->Update( mode ) || childchanged;
            m_flags.m_HasToolObjectsBelow = m_flags.m_HasToolObjectsBelow || obj->m_flags.m_HasToolObjectsBelow;
            m_flags.m_HasSelectedObjectsBelow = m_flags.m_HasSelectedObjectsBelow || obj->m_flags.m_HasSelectedObjectsBelow;

            //if childchanged has become true now, this means that down here the current child object did change and
            //therefore has recalculated its boundingbox already.
        }
    }

    bool propertychanged = false;
    //check properties for change
    if ( HasDynamicProperties() )
    {
        a2dNamedPropertyList::const_iterator iter;
        for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
        {
            const a2dNamedProperty* prop = *iter;
            a2dObject* obj = prop->GetRefObjectNA();
            if ( wxDynamicCast( prop, a2dShadowStyleProperty ) )
            {
                propertychanged = changed;
            }
            else if ( wxDynamicCast( obj, a2dCanvasObject ) && prop->GetCanRender() && prop->GetVisible() &&
                      wxDynamicCast( obj, a2dCanvasObject )->GetVisible() )
            {
                propertychanged = wxStaticCast( obj, a2dCanvasObject )->Update( mode ) || propertychanged;
            }
            else if ( mode & update_includebboxprop && prop->GetId() == PROPID_BoundingBox )
                propertychanged = true;
        }
    }

    // in Update() this stores the children its boundingbox when changed.
    // This can be used in the DoUpdate() to e.g. resize itself to that.
    a2dBoundingBox childbox;
    // in Update() this stores the children its boundingbox when changed.
    // This can be used in the DoUpdate() to e.g. resize itself to that.
    a2dBoundingBox childboxOnlyTranslate;
    // in Update() this stores the properties its boundingbox when changed.
    // This can be used in the DoUpdate() to e.g. resize itself to that.
    a2dBoundingBox propbox;
    // holds clippingbox for current object defined by a2dClipPathProperty
    a2dBoundingBox clipbox;
    double maxworldextend = 0;
    double maxpixelextend = 0;
    a2dShadowStyleProperty* shadow = NULL;

    // Now if anything (this, child or properties) changed, recalculate the property and childrens boundingbox
    // to include all that.
    // If anything changed, invalidate the bounding box
    if ( changed || childchanged || propertychanged )
    {
        //BR2019 if ( !GetPendingSelect() )
            m_bbox.SetValid( false );
        changed = true;

        if ( m_childobjects != wxNullCanvasObjectList  && m_flags.m_visiblechilds )
        {
            a2dAffineMatrix transworld;
            transworld.Translate( GetPosX(), GetPosY() );
            forEachIn( a2dCanvasObjectList, m_childobjects )
            {
                a2dCanvasObject* obj = *iter;
                //calculate bbox only for visible objects
                if( obj && obj->GetVisible() )
                {
                    if ( obj->m_flags.m_ChildOnlyTranslate )
                    {
                        childboxOnlyTranslate = obj->GetBbox();
                        childboxOnlyTranslate.MapBbox( transworld );
                    }
                    else
                        childbox.Expand( obj->GetBbox() );

                    maxworldextend = wxMax( obj->m_worldExtend, maxworldextend );
                    maxpixelextend = wxMax( obj->m_pixelExtend, maxpixelextend );
                }
            }

            if ( childboxOnlyTranslate.GetValid() ) //no child or empty child secure
            {
                childboxOnlyTranslate.Enlarge( maxworldextend ); //enough to expand total childrens bbox with maximum stroke width
            }

            if ( childbox.GetValid() ) //no child or empty child secure
            {
                childbox.Enlarge( maxworldextend ); //enough to expand total childrens bbox with maximum stroke width
            }
            m_pixelExtend = ( wxUint16 ) wxMax( ( int ) maxpixelextend, m_pixelExtend );
        }

        if ( HasDynamicProperties() )
        {
            a2dNamedPropertyList::const_iterator iter;
            for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
            {
                const a2dNamedProperty* prop = *iter;
                a2dObject* obj = prop->GetRefObjectNA();
                if ( wxDynamicCast( prop, a2dClipPathProperty ) )
                {
                    if ( obj )
                    {
                        clipbox.Expand( wxStaticCast( obj, a2dCanvasObject )->GetMappedBbox( m_lworld ) );

                        // if the clipping object is visible, we do not only want the AND
                        // of the clippingbox with the rest, but also clipping object itself should be in  there.
                        if ( wxDynamicCast( obj, a2dCanvasObject ) && wxDynamicCast( obj, a2dCanvasObject )->GetVisible() )
                        {
                            propbox.Expand( wxStaticCast( obj, a2dCanvasObject )->GetMappedBbox( m_lworld ) );
                        }
                    }
                }
                else if ( wxDynamicCast( prop, a2dShadowStyleProperty ) )
                {
                    shadow = wxStaticCast( prop, a2dShadowStyleProperty );
                }
                else if ( wxDynamicCast( obj, a2dCanvasObject ) && prop->GetCanRender() && prop->GetVisible() && wxDynamicCast( obj, a2dCanvasObject )->GetVisible() )
                {
                    propbox.Expand( wxStaticCast( obj, a2dCanvasObject )->GetMappedBbox( m_lworld ) );
                }
                else if ( mode & update_includebboxprop && prop->GetId() == PROPID_BoundingBox )
                    propbox.Expand( wxStaticCast( prop, a2dBoudingBoxProperty )->GetValue() );
            }
        }
    }

    if ( childboxOnlyTranslate.GetValid() ) //no child or empty child secure
        childbox.Expand( childboxOnlyTranslate );

    //Ask DoUpdate if it thinks anything changed.
    //Add derived object specific features to the boundingbox.
    //If the derived object has internal child objects itself, they will be Updated too.
    //In case those children did change, the bbox will be invalidated and recalculated.
    //Also when the bbox is already invalid, this will happen.
    //If force is true then for sure everything will be recalculated.
    //The return value is true if indeed something did change/ was recalculated.
    changed = DoUpdate( mode, childbox, clipbox, propbox ) || changed;

    if ( childbox.GetValid() ) //no child or empty child secure
    {
        childbox.MapBbox( m_lworld );
        m_bbox.Expand( childbox );
    }

    if ( shadow )
    {
        double dx = cos( shadow->GetExtrudeAngle() ) * shadow->GetExtrudeDepth();
        double dy = sin( shadow->GetExtrudeAngle() ) * shadow->GetExtrudeDepth();

        a2dBoundingBox shadowbox = m_bbox;
        shadowbox.Translate( dx, dy );
        propbox.Expand( shadowbox );
    }
    if ( propbox.GetValid() )
        m_bbox.Expand( propbox );

    if ( clipbox.GetValid() && !m_flags.m_editingCopy )
        m_bbox.And( &clipbox );

    // If the bounding box may only be expanded, extend it by the old one
    if( ( mode & update_save ) )
    {
        // If the old bounding box is not inside the new one,
        // the bounding box is added as property which lead to an extra update area in AddPending()
        if ( oldbbox.GetValid() && oldbbox.GetWidth() && oldbbox.GetHeight() && m_bbox.GetValid() && m_bbox.Intersect( oldbbox ) != _IN )
        {
            PROPID_BoundingBox->SetPropertyToObject( this, oldbbox );
            //PROPID_BoundingBox->SetPropertyToObject( this, m_bbox );
            SetPending( true );
        }
    }

    if ( !m_bbox.GetValid() )
    {
        changed = true;
        //no objects, therefore make the bounding box the x,y of the object
        m_bbox.Expand( m_lworld.GetValue( 2, 0 ), m_lworld.GetValue( 2, 1 ) );
    }

    return changed;
}

bool a2dCanvasObject::IsRecursive()
{
    return GetChildObjectsCount() != 0;
}

void a2dCanvasObject::MakeReferencesUnique()
{
    if ( m_childobjects != wxNullCanvasObjectList )
        m_childobjects->MakeUnique();
}

void a2dCanvasObject::WalkerWithContext( a2dIterC& ic, wxObject* parent, a2dWalkerIOHandlerWithContext& handler )
{
    if ( handler.GetUseCheck() && GetCheck() )
        return;

    handler.IncCurrentDepth();

    handler.WalkTask( ic, parent, this, a2dWalker_a2dCanvasObjectStart );

    {
        //go to child context
        a2dIterCU cu( ic, this );
        DoWalkerWithContext( ic, parent, handler );
    }

    handler.WalkTask( ic, parent, this, a2dWalker_a2dCanvasObjectEnd );
    handler.DecCurrentDepth();
}

void a2dCanvasObject::DoWalkerWithContext( a2dIterC& ic, wxObject* parent, a2dWalkerIOHandlerWithContext& handler )
{
    // depthfirst means, first go to recursive object
    // and skipping them when iterating the other objects later on
    if ( handler.GetDepthFirst() && m_childobjects != wxNullCanvasObjectList && !handler.GetStopWalking() )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && !obj->GetRelease() && obj->IsRecursive() )
                obj->WalkerWithContext( ic, this, handler );
        }
    }

    if ( handler.WalkTask( ic, parent, this, a2dWalker_a2dCanvasObjectPreChild ) &&
            m_childobjects != wxNullCanvasObjectList && !handler.GetStopWalking() )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && !obj->GetRelease() &&
                    !( handler.GetDepthFirst() && obj->IsRecursive() )  )
                obj->WalkerWithContext( ic, this, handler );
        }
    }

    if ( handler.WalkTask( ic, parent, this, a2dWalker_a2dCanvasObjectPostChild ) &&
            m_childobjects != wxNullCanvasObjectList && !handler.GetStopWalking() )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && !obj->GetRelease() &&
                    !( handler.GetDepthFirst() && obj->IsRecursive() )  )
                obj->WalkerWithContext( ic, this, handler );
        }
    }
}

void a2dCanvasObject::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dCanvasObjectStart );

    a2dObject::DoWalker( parent, handler );

    // depthfirst means, first go to recursive object
    // and skipping them when iterating the other objects later on
    if ( handler.GetDepthFirst() && m_childobjects != wxNullCanvasObjectList && !handler.GetStopWalking() )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && !obj->GetRelease() && obj->IsRecursive() )
                obj->Walker( this, handler );
        }
    }

    if ( handler.WalkTask( parent, this, a2dWalker_a2dCanvasObjectPreChild ) &&
            m_childobjects != wxNullCanvasObjectList && !handler.GetStopWalking() )
    {
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
        int childNum = m_childobjects->size();
        int childPos = 0;
#endif
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && !obj->GetRelease() &&
                    !( handler.GetDepthFirst() && obj->IsRecursive() )  )
                obj->Walker( this, handler );
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
            childPos++;
#endif
        }
    }

    if ( handler.WalkTask( parent, this, a2dWalker_a2dCanvasObjectPostChild ) &&
            m_childobjects != wxNullCanvasObjectList && !handler.GetStopWalking() )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && !obj->GetRelease() &&
                    !( handler.GetDepthFirst() && obj->IsRecursive() )  )
                obj->Walker(  this, handler );
        }
    }

    handler.WalkTask( parent, this, a2dWalker_a2dCanvasObjectEnd );
}

#if wxART2D_USE_CVGIO

void AddFlag( wxString& flags, const wxString& name, bool value, bool defval )
{
    if ( value != defval )
    {
        if ( value )
            flags = flags + " " + name;
        else
            flags = flags + " " + name + "(0)";
    }
}

void a2dCanvasObject::DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts, a2dObjectList* towrite )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
        a2dObject::DoSave( this, out, xmlparts, towrite );

        if ( wxLAYER_DEFAULT != m_layer )
        {
            out.WriteAttribute( wxT( "layer" ), m_layer );
        }

        wxString s;

        AddFlag( s, wxT("selected"), m_flags.m_selected, m_flagsInit.m_selected );
        AddFlag( s, wxT("selected2"), m_flags.m_selected2, m_flagsInit.m_selected2 );

        AddFlag( s, wxT("selectable"), m_flags.m_selectable ,m_flagsInit.m_selectable );
        AddFlag( s, wxT("subedit"), m_flags.m_subEdit ,m_flagsInit.m_subEdit );
        AddFlag( s, wxT("subeditaschild"), m_flags.m_subEditAsChild ,m_flagsInit.m_subEditAsChild );
        AddFlag( s, wxT("visible"), m_flags.m_visible ,m_flagsInit.m_visible );
        AddFlag( s, wxT("draggable"), m_flags.m_draggable ,m_flagsInit.m_draggable );
        AddFlag( s, wxT("shadow"), m_flags.m_showshadow ,m_flagsInit.m_showshadow );
        AddFlag( s, wxT("filled"), m_flags.m_filled ,m_flagsInit.m_filled );
        AddFlag( s, wxT("a"), m_flags.m_a ,m_flagsInit.m_a );
        AddFlag( s, wxT("b"), m_flags.m_b ,m_flagsInit.m_b );
        AddFlag( s, wxT("fixedStyle"), m_flags.m_fixedStyle ,m_flagsInit.m_fixedStyle );
        AddFlag( s, wxT("generatePins"), m_flags.m_generatePins ,m_flagsInit.m_generatePins );
        AddFlag( s, wxT("bin"), m_flags.m_bin ,m_flagsInit.m_bin );
        AddFlag( s, wxT("snap"), m_flags.m_snap ,m_flagsInit.m_snap );
        AddFlag( s, wxT("pushin"), m_flags.m_pushin ,m_flagsInit.m_pushin );
        AddFlag( s, wxT("ChildOnlyTranslate"), m_flags.m_ChildOnlyTranslate ,m_flagsInit.m_ChildOnlyTranslate );

        if ( !s.IsEmpty() )
            out.WriteAttribute( wxT( "flags" ), s );

        s = wxT( "" );
        if ( m_hitflags.m_non )                    s = s + wxT( " non" ) ;
        if ( m_hitflags.m_fill )                   s = s + wxT( " fill" ) ;
        if ( m_hitflags.m_stroke )                 s = s + wxT( " stroke" ) ;
        if ( m_hitflags.m_fill_non_transparent )   s = s + wxT( " fill_non_transparent" );
        if ( m_hitflags.m_stroke_non_transparent ) s = s + wxT( " stroke_non_transparent" );
        if ( m_hitflags.m_visible )                s = s + wxT( " visible" ) ;
        if ( m_hitflags.m_all )                    s = s + wxT( " all" ) ;

        if ( GetHitFlags() != a2dCANOBJ_EVENT_VISIBLE  )           
        out.WriteAttribute( wxT( "hitflags" ), s );

        if ( !m_lworld.IsIdentity() )
        {
            out.WriteNewLine();
            if ( !m_lworld.IsTranslate() )
                s.Printf ( wxT( "matrix( %s %s %s %s %s %s )" ),
                           wxString::FromCDouble( m_lworld.GetValue( 0, 0 ) * out.GetScale() ),
                           wxString::FromCDouble( m_lworld.GetValue( 0, 1 ) * out.GetScale() ),
                           wxString::FromCDouble( m_lworld.GetValue( 1, 0 ) * out.GetScale() ),
                           wxString::FromCDouble( m_lworld.GetValue( 1, 1 ) * out.GetScale() ),
                           wxString::FromCDouble( m_lworld.GetValue( 2, 0 ) * out.GetScale() ),
                           wxString::FromCDouble( m_lworld.GetValue( 2, 1 ) * out.GetScale() )
                         );
            else
                s.Printf ( wxT( "translate( %s %s )" ),
                           wxString::FromCDouble( m_lworld.GetValue( 2, 0 ) * out.GetScale() ),
                           wxString::FromCDouble( m_lworld.GetValue( 2, 1 ) * out.GetScale() )
                         );
            out.WriteAttribute( wxT( "transform" ), s );
            out.WriteNewLine();
        }

        if ( GetFill().IsSameAs( *a2dINHERIT_FILL ) )
        {
            out.WriteAttribute( wxT( "fill" ), wxT( "inherit" ) );
        }
        if ( GetStroke().IsSameAs( *a2dINHERIT_STROKE ) )
        {
            out.WriteAttribute( wxT( "stroke" ), wxT( "inherit" ) );
        }
    }
    else
    {
        a2dObject::DoSave( this, out, xmlparts, towrite );

        if ( m_childobjects != wxNullCanvasObjectList )
        {
            forEachIn( a2dCanvasObjectList, m_childobjects )
            {
                a2dCanvasObject* obj = *iter;
                if ( obj )
                    obj->Save( this, out, towrite );
                //out.WriteNewLine();
            }
        }
    }
}

bool a2dCanvasObject::ParseCvgTransForm( a2dAffineMatrix& result, a2dIOHandlerXmlSerIn& parser )
{
    wxString str = parser.GetAttributeValue( wxT( "transform" ) );
    wxString error;
    if ( !str.IsEmpty() && !::ParseCvgTransForm( result, str, error ) )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "CVG : invalid transform %s at line %d" ), str.c_str(), parser.GetCurrentLineNumber() );
        return true; //just a warning, still continue
    }

	result.SetValue( 2, 0, result.GetValue( 2, 0 ) * parser.GetScale() );
	result.SetValue( 2, 1, result.GetValue( 2, 1 ) * parser.GetScale() );

    return true;
}

bool ParseFlag( wxString& str, wxString& flagname, bool& flagvalue, wxString& error )
{
    if ( str == wxT( "" ) )
        return false;

    wxString numstr;
    size_t i;
    for ( i = 0; i < str.Len(); i++ )
    {
        //skip space
        while ( wxIsspace( str[i] ) ) i++;

        flagname.Clear();
        while ( i < str.Len() &&  isalnum ( str[i] ) )
        {
            flagname += str.GetChar( i );
            i++;
        }

        //skip space
        while ( i < str.Len() && wxIsspace( str[i] ) ) i++;

        if ( i == str.Len() )
        {
            flagvalue = true;
            return true;
        }

        if ( str[i] != wxT( '(' ) )
        {
            error = _( "CVG parsing error: missing" );
            return false;
        }
        i++;

        while ( i < str.Len() && wxIsspace( str[i] ) ) i++;

        while ( i < str.Len() &&  str[i] != wxT( ')' ) )
        {
            numstr.Clear();
            if ( isdigit( str[i] ) )
            {
                numstr = str.GetChar( i );
            }
            i++;
        }

        if ( i == str.Len() || str[i] != wxT( ')' ) )
        {
            error = _( "CVG parsing error: missing" );
            return false;
        }
        flagvalue = numstr != "0";
        return true;
    }
    return false;
}

void a2dCanvasObject::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{

    if ( xmlparts == a2dXmlSer_attrib )
    {
        a2dObject::DoLoad( parent, parser, xmlparts );

        ParseCvgTransForm( m_lworld, parser );

        m_layer = parser.GetAttributeValueUint16( wxT( "layer" ), 0 );

        wxString flags = parser.GetAttributeValue( wxT( "flags" ) );
        wxStringTokenizer tkz( flags );

        while ( tkz.HasMoreTokens() )
        {
            wxString token = tkz.GetNextToken();
            wxString flagname, error;
            bool flagvalue = false;

            if ( !ParseFlag( token, flagname, flagvalue, error ) )
                a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "could not parse %s, %s, will be skipped line %d" ),
                                                     token.c_str(), error.c_str(), parser.GetCurrentLineNumber() );
            else
            {
                if ( flagname == wxT( "selected" ) )
                    m_flags.m_selected   = flagvalue;
                if ( flagname == wxT( "selected2" ) )
                    m_flags.m_selected2  = flagvalue;
                else if ( flagname == wxT( "selectable" ) )
                    m_flags.m_selectable = flagvalue;
                else if ( flagname == wxT( "subedit" ) )
                    m_flags.m_subEdit   = flagvalue;
                else if ( flagname == wxT( "subeditaschild" ) )
                    m_flags.m_subEditAsChild = flagvalue;
                else if ( flagname == wxT( "visible" ) )
                    m_flags.m_visible    = flagvalue;
                else if ( flagname == wxT( "draggable" ) )
                    m_flags.m_draggable  = flagvalue;
                else if ( flagname == wxT( "showshadow" ) )
                    m_flags.m_showshadow = flagvalue;
                else if ( flagname == wxT( "filled" ) )
                    m_flags.m_filled     = flagvalue;
                else if ( flagname == wxT( "a" ) )
                    m_flags.m_a          = flagvalue;
                else if ( flagname == wxT( "b" ) )
                    m_flags.m_b          = flagvalue;
                else if ( flagname == wxT( "fixedStyle" ) )
                    m_flags.m_fixedStyle = flagvalue;                
                else if ( flagname == wxT( "generatePins" ) )
                    m_flags.m_generatePins = flagvalue;
                else if ( flagname == wxT( "bin" ) )
                    m_flags.m_bin        = flagvalue;
                else if ( flagname == wxT( "snap" ) )
                    m_flags.m_snap       = flagvalue;
                else if ( flagname == wxT( "snap_to" ) )
                    m_flags.m_snap_to    = flagvalue;
                else if ( flagname == wxT( "pushin" ) )
                    m_flags.m_pushin     = flagvalue;
                else if ( flagname == wxT( "ChildOnlyTranslate" ) )
                    m_flags.m_ChildOnlyTranslate = flagvalue;
            }
        }


        // not hitflags, assume visible hitflag
        if ( parser.HasAttribute( wxT( "hitflags" ) ) )
        {
            wxString hitflags = parser.GetAttributeValue( wxT( "hitflags" ) );
            m_hitflags.m_non             = hitflags.Find( wxT( "non" ) )  != -1;
            m_hitflags.m_fill            = hitflags.Find( wxT( "fill" ) )  != -1;
            m_hitflags.m_stroke          = hitflags.Find( wxT( "stroke" ) )  != -1;
            m_hitflags.m_fill_non_transparent   = hitflags.Find( wxT( "fill_non_transparent" ) ) != -1;
            m_hitflags.m_stroke_non_transparent = hitflags.Find( wxT( "stroke_non_transparent" ) )  != -1;
            m_hitflags.m_visible         = hitflags.Find( wxT( "visible" ) )  != -1;
            m_hitflags.m_all             = hitflags.Find( wxT( "all" ) )  != -1;
        }
        else
            m_hitflags.m_visible         = true;
    }
    else
    {
        //! todo ignore until this down here is fully converted.
        //a2dObject::DoLoad( parent, parser, xmlparts );

        if ( parser.GetTagName() == wxT( "properties" ) )
        {
            parser.Next();
            while( parser.GetTagName() == wxT( "o" ) )
            {
                wxString classname = parser.GetAttributeValue( wxT( "classname" ) );
                wxObject* obj = parser.CreateObject( classname );
                a2dNamedPropertyPtr property = wxDynamicCast( obj, a2dNamedProperty );
                if ( property )
                {
                    property->Load( this, parser );
                    if ( parser.GetLastError() == a2dError_NoError )
                        property->SetToObject( this );
                }
                else if( wxDynamicCast( obj, a2dFill ) )
                {
                    // This is for compatitbility
                    wxStaticCast( obj, a2dFill )->Load( this, parser );
                    a2dCanvasObject::PROPID_Fill->SetPropertyToObject( this,  *wxStaticCast( obj, a2dFill ) );
                    delete obj;
                }
                else if( wxDynamicCast( obj, a2dStroke ) )
                {
                    // This is for compatitbility
                    wxStaticCast( obj, a2dStroke )->Load( this, parser );
                    a2dCanvasObject::PROPID_Stroke->SetPropertyToObject( this, *wxStaticCast( obj, a2dStroke ) );
                    delete obj;
                }
                else
                {
                    a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "could not create a2dNamedProperty %s, will be skipped line %d" ),
                                                     classname.c_str(), parser.GetCurrentLineNumber() );
                    parser.SkipSubTree();
                    parser.Require( END_TAG, wxT( "o" ) );
                    parser.Next();
                }
            }

            parser.Require( END_TAG, wxT( "properties" ) );
            parser.Next();
        }

        while( parser.GetEventType() == START_TAG && parser.GetTagName() == wxT( "o" ) )
        {
            // The refid attribute is used in a2dObject::Load() to point to other objects as a reference.
            // Meaning it will be a smart pointer to an object already load or still to be loaded later on.
            // We do not need to construct the real object just to load that. We just Create the most simple
            // a2dCanvasObject pointer here to hold/load it.
            if ( parser.HasAttribute( wxT( "refid" ) ) )
            {
                a2dCanvasObjectPtr object = new a2dCanvasObject();
                Append( object );
                object->Load( this, parser );
                a2dSmrtPtr<class a2dObject>* obj = ( a2dSmrtPtr<class a2dObject>* ) & ( m_childobjects->back() );
                parser.ResolveOrAdd( obj );
            }
            else
            {
                wxString classname = parser.GetAttributeValue( wxT( "classname" ) );
                a2dCanvasObject* object = wxDynamicCast( parser.CreateObject( classname ), a2dCanvasObject );
                if ( !object )
                {
                    a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "could not create a2dCanvasObject %s, will be skipped line %d" ),
                                                     classname.c_str(), parser.GetCurrentLineNumber() );
                    parser.SkipSubTree();
                    parser.Require( END_TAG, wxT( "o" ) );
                    parser.Next();
                }
                else
                {
                    Append( object );
                    object->Load( this, parser );
                    // if the object just load is a pure reference ( only has a refid attribute ),
                    // we can resolve here if the referenced object itself was already read.
                    // This is a speed issue, and if not resolved now, meaning object is still to be read, we will resolve it later.
                    // See a2dObject::Load() too.
                    a2dSmrtPtr<class a2dObject>* obj = ( a2dSmrtPtr<class a2dObject>* ) & ( m_childobjects->back() );
                    parser.ResolveOrAdd( obj );
                }
            }
        }
    }
}

#endif //wxART2D_USE_CVGIO

a2dBoundingBox a2dCanvasObject::GetClipBox( a2dIterC& WXUNUSED( ic ) )
{
    a2dBoundingBox bbox;
    if ( HasDynamicProperties() )
    {
        a2dNamedPropertyList::const_iterator iter;
        for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
        {
            const a2dNamedProperty* prop = *iter;
            a2dClipPathProperty* clipprop = wxDynamicCast( prop, a2dClipPathProperty );
            if ( clipprop )
            {
                a2dBoundingBox tmp;
                tmp.Expand( clipprop->GetClipObject()->GetMappedBbox( m_lworld ) );
                bbox.Expand( tmp );
            }
        }
    }
    return bbox;
}

void a2dCanvasObject::SetLayer( wxUint16 layer )
{
    wxASSERT_MSG( wxMAXLAYER > layer , wxT( " layer > wxMAXLAYER, index to high" ) );

    SetPending( true );
    m_layer = layer;
    if ( m_root && m_root->GetLayerSetup() )
    {
        m_root->GetLayerSetup()->GetLayerInfo(layer)->SetAvailable( true );
        //wxASSERT_MSG( m_root->GetLayerSetup()->GetLayerIndex().count( layer ), wxT( "layer not defined in layer table" ) );
    }
};

void a2dCanvasObject::SetRoot( a2dDrawing* root, bool recurse )
{
    m_root = root;
    /*
        if (m_root && m_root->GetLayerSetup() )
        {
            wxASSERT_MSG( wxNullLayerInfo != m_root->GetLayerSetup()->GetLayerIndex()[m_layer],
                wxT("layer not defined in layer table") );
        }
    */
    if ( recurse )
    {
        a2dWalker_SetRoot setRecursive( root );
        setRecursive.SetSkipNotRenderedInDrawing( true );
        setRecursive.Start( this );
    }
}

void a2dCanvasObject::RenderChildObjects( a2dIterC& ic, RenderChild& whichchilds, OVERLAP clipparent )
{
    if ( m_childobjects == wxNullCanvasObjectList || m_childobjects->empty() )
        return;

    a2dCanvasObjectList::iterator iter = m_childobjects->begin();

    //iterate over layers if the children are meant to be drawn on a given layer at once
    //OR we do NOT render per layer from a parent object.
    if (  ic.GetPerLayerMode() && m_flags.m_childrenOnSameLayer && m_root->GetLayerSetup() )
    {
        wxASSERT_MSG( ic.GetLayer() != wxLAYER_ALL, _T( " a2dIterC::GetPerLayerMode() can only render one layer at the time" ) );

        wxUint16 layer = ic.GetLayer();
        a2dLayerIndex::iterator itb = GetRoot()->GetLayerSetup()->GetOrderSort().begin();
        a2dLayerIndex::iterator ite = GetRoot()->GetLayerSetup()->GetOrderSort().end();
        // YES here we do the opposite of when rendering, hitting the top layer first
        if ( ic.GetDrawingPart()->GetReverseOrder() )
        {
            itb = GetRoot()->GetLayerSetup()->GetReverseOrderSort().begin();
            ite = GetRoot()->GetLayerSetup()->GetReverseOrderSort().end();
        }
        for ( a2dLayerIndex::iterator it= itb; it != ite; ++it)
        {
            a2dLayerInfo* layerinfo = *it;

            //important!
            //if layer is visible it will be rendered
            //If an object on a layer is itself invisible it will not be drawn
            if ( ic.GetLayer() == wxLAYER_ALL ||
                    (  ic.GetDrawingPart()->GetLayerRenderArray()[ layerinfo->GetLayer() ].DoRenderLayer()
                        && layerinfo->GetVisible()
                    )
                )
            {
                RenderChildObjectsOneLayer( ic, whichchilds, clipparent );
            }
        }
    }
    else
    {
        //no layer iteration will be done and therefore only object on m_layer == layer will be rendered.
        //layer == wxLAYER_ALL is a special case, and means ignore layer order for rendering the child objects.
        //checking of availability and visibility of layer itself is handled in parent objects
        RenderChildObjectsOneLayer( ic, whichchilds, clipparent );
    }
    //restore not needed, all object do restore themselfs the transform in the end
    //ic.GetDrawer2D()->SetRelativeTransform( *tworld );
}

void a2dCanvasObject::RenderChildObjectsOneLayer( a2dIterC& ic, RenderChild& whichchilds, OVERLAP clipparent )
{
    if ( m_childobjects == wxNullCanvasObjectList  )
        return;

    RenderChild detectchilds;
    detectchilds.m_postrender = false;
    detectchilds.m_prerender = false;
    detectchilds.m_property = false;

    //save the current style state of the a2dDrawingPart, in order to inherit and restore style of object
    a2dFill currentdrawer_fill = ic.GetDrawer2D()->GetDrawerFill();
    a2dStroke currentdrawer_stroke = ic.GetDrawer2D()->GetDrawerStroke();

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        if ( !obj )
            continue;

        int olayer = obj->GetLayer();

        //! detect specific objects, to prevent extra rendering calls for child objects, if they do not exist.
        detectchilds.m_postrender = detectchilds.m_postrender || !obj->GetPreRenderAsChild();
        detectchilds.m_prerender = detectchilds.m_prerender || obj->GetPreRenderAsChild();
        detectchilds.m_property = detectchilds.m_property || obj->GetIsProperty();

        //do a rough check to increase speed in common cases
        if ( (
                    ( obj->GetPreRenderAsChild() && whichchilds.m_prerender && !obj->GetIsProperty() ) ||
                    ( !obj->GetPreRenderAsChild() && whichchilds.m_postrender && !obj->GetIsProperty() ) ||
                    ( obj->GetIsProperty() && whichchilds.m_property )
                )
                // the next does work if child is a container like e.g. a2dCanvasObjectReference
                // because they have a flags GetIgnoreLayer() set.
                &&
                ( olayer == ic.GetLayer() || ic.GetLayer() == wxLAYER_ALL || obj->GetChildObjectsCount() || obj->GetIgnoreLayer() )
           )
        {
            /*LEAVE IN
            //test bbox per object like this!!
            ic.GetDrawer2D()->SetDrawerStroke( a2dBLACK_STROKE );
            ic.GetDrawer2D()->SetDrawerFill( a2dTRANSPARENT_FILL );
            wxRect absareadev = obj->GetAbsoluteArea( ic );
            ic.GetDrawer2D()->DevDrawRectangle( absareadev.x, absareadev.y, absareadev.width, absareadev.height);
            ic.GetDrawer2D()->SetDrawerStroke( currentdrawer_stroke );
            ic.GetDrawer2D()->SetDrawerFill( currentdrawer_fill );
            */

            // clipparent is either _IN or _ON      ( _OUT   would not get us to this point )
            // testing clipping of children only needed when not totaly _IN.

            OVERLAP childclip = _IN;
            if ( clipparent != _IN )
                childclip = obj->GetClipStatus( ic, clipparent );

            if ( childclip != _OUT ) //if a child is _OUT, no need to render it.
            {
                //optimization only when inheritance is used for child, we need to make sure the
                //current style is right.
                //else it will be set to another value anyway, therefore no need to
                //restore style that (maybe) was modified in previous rendered object of this group
                //The end effect of this is that objects with same style do not result in
                //unnecessary stroke and fill changes, which for certain (wxDC) a2dDrawingPart types
                //takes a lot of time.

                if ( obj->HasDynamicProperties() )
                {
                    if (  !obj->GetFill().IsNoFill() && obj->GetFill().IsSameAs( *a2dINHERIT_FILL ) )
                        ic.GetDrawer2D()->SetDrawerFill( currentdrawer_fill );
                    //if ( !obj->GetStroke()->IsNoStroke() && obj->GetStroke() == a2dINHERIT_STROKE )
                    //    ic.GetDrawer2D()->SetDrawerStroke( currentdrawer_stroke );
                }

                if ( obj->m_flags.m_ChildOnlyTranslate )
                {
                    a2dAffineMatrix iworld = m_lworld;
                    iworld.Invert();
                    a2dIterCU cu( ic, iworld );
                    a2dAffineMatrix transworld;
                    transworld.Translate( GetPosX(), GetPosY() );
                    a2dIterCU cu2( ic, transworld );
                    obj->Render( ic, childclip );
                }
                else
                    obj->Render( ic, childclip );
            }

        }
    }

    ic.GetDrawer2D()->SetDrawerFill( currentdrawer_fill );
    ic.GetDrawer2D()->SetDrawerStroke( currentdrawer_stroke );

    whichchilds = detectchilds;
}

a2dCanvasObject* a2dCanvasObject::CreateHierarchy( a2dCanvasObjectFlagsMask mask, bool createref )
{
    a2dCanvasObject* group = 0; //only make a group if there are object with the right mask

    if ( m_childobjects == wxNullCanvasObjectList  )
        m_childobjects = new a2dCanvasObjectList();

    a2dCanvasObjectList::iterator iter = m_childobjects->begin();
    while( iter != m_childobjects->end() )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        if ( obj && obj->CheckMask( mask ) )
        {
            if ( !group )
                group = new a2dCanvasObject();

            iter = m_childobjects->erase( iter );
            group->Append( obj );
        }
        else
            iter++;
    }

    if ( !group )
        return group;

    group->SetRoot( m_root );
    group->SetIgnoreLayer( true );
    //parent boxes don't change
    group->Update( updatemask_force );

    //let the bounding be the position
    double dx, dy;
    dx = group->GetBbox().GetMinX();
    dy = group->GetBbox().GetMinY();

    //translate the objects within the group in opposite direction
    for( iter = group->m_childobjects->begin(); iter != group->m_childobjects->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj )
            obj->Translate( -dx, -dy );
    }

    if ( createref )
    {
        a2dCanvasObjectReference* ref = new a2dCanvasObjectReference( dx, dy, group );
        ref->SetRoot( m_root );
        Append( ref );
    }
    else
    {
        group->SetPosXY( dx, dy );
        Append( group );
    }

    Update( updatemask_force );
    return group;
}

void a2dCanvasObject::RemoveHierarchy()
{
    if ( m_childobjects != wxNullCanvasObjectList  )
    {
        a2dCanvasObjectList::iterator iter = m_childobjects->begin();
        while( iter != m_childobjects->end() )
        {
            a2dCanvasObjectList::value_type directChild = *iter;

            if ( directChild && directChild->m_childobjects != wxNullCanvasObjectList )
            {
                a2dCanvasObjectList::iterator subiter = directChild->m_childobjects->begin();
                while( subiter != directChild->m_childobjects->end() )
                {
                    a2dCanvasObjectList::value_type subchild = *subiter;

                    if ( subchild && !subchild->GetRelease() )
                    {
                        subchild->Transform( directChild->GetTransformMatrix() );
                        m_childobjects->push_front( subchild );
                        subiter = directChild->m_childobjects->erase( subiter );
                    }
                    else
                        subiter++;
                }
            }
            iter++;
        }
    }
}

bool a2dCanvasObject::EliminateMatrix()
{
    if ( m_childobjects == wxNullCanvasObjectList )
    {
        m_lworld.Identity();
        return true;
    }

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj )
            obj->Transform( m_lworld );
    }
    m_lworld.Identity();
    return true;
}

void a2dCanvasObject::Prepend( a2dCanvasObject* obj )
{
    //very slow
    //wxASSERT_MSG( wxNOT_F OUND == IndexOf(obj) , wxT("object is already a child, use a reference") );

    if ( m_root )
        obj->SetRoot( m_root, true ); //do recurse here. ( Think like pins do get set).

    if ( m_childobjects == wxNullCanvasObjectList )
        m_childobjects = new a2dCanvasObjectList();

    m_childobjects->push_front( obj );
    obj->SetPending( true );
}

void a2dCanvasObject::Append( a2dCanvasObject* obj )
{
    //very slow
    //wxASSERT_MSG( wxNOT_FOUND == IndexOf(obj) , wxT("object is already a child, use a reference") );

    if ( m_root )
    {
        obj->SetRoot( m_root, true ); //do recurse here. ( Things like pins do get set).

        // to slow if many
        // a2dComEvent changedlayer( this, obj->GetLayer(), a2dCanvasDocument::sm_changedLayer );
        // m_root->ProcessEvent( changedlayer );
    }

    if ( m_childobjects == wxNullCanvasObjectList )
        m_childobjects = new a2dCanvasObjectList();

    m_childobjects->push_back( obj );
    obj->SetPending( true );
    obj->SetParent( this ); //if pin haspins will be set.
}

void a2dCanvasObject::Insert( size_t before, a2dCanvasObject* obj, bool ignoreReleased )
{
    //very slow
    //wxASSERT_MSG( wxNOT_FOUND == IndexOf(obj) , wxT("object is already a child, use a reference") );

    if ( m_root )
        obj->SetRoot( m_root, true ); //do recurse here. ( Think like pins do get set).

    if ( m_childobjects == wxNullCanvasObjectList )
        m_childobjects = new a2dCanvasObjectList();

    m_childobjects->Insert( before, obj, ignoreReleased );
}

int a2dCanvasObject::ReleaseChild( a2dCanvasObject* obj, bool backwards, bool all, bool now, bool undoCommands )
{
    if ( obj == 0 )
        return 0;
    if ( m_childobjects == wxNullCanvasObjectList )
        return 0;

    int totalreleased = 0;
    {
        // keep the object alive until we are done
        a2dCanvasObjectPtr keepAlive = obj;

        totalreleased = m_childobjects->Release( obj, backwards, all, now );
        if ( totalreleased )
        {
            if ( obj->HasPins() )
                obj->DisConnectWith( NULL, wxT( "" ), undoCommands );
        }
    }

    if ( now )
    {
        SetPending( true ); //yes parent! to do a proper update, since the real object is gone.
    }

    //delete when no more children, this is better since temporary editing childs handles should
    //not lead to always having a childlist
    if ( m_childobjects != wxNullCanvasObjectList  && !m_childobjects->size() )
    {
        delete m_childobjects;
        m_childobjects = wxNullCanvasObjectList;
    }

    return totalreleased;
}

bool a2dCanvasObject::ReleaseChildObjects( a2dCanvasObjectFlagsMask mask )
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return false;

    bool did = false;

    if ( m_childobjects->size() && m_childobjects->Release( mask ) )
    {
        SetPending( true );
        did = true;
    }

    //delete when no more children, this is better since temporary editing childs handles should
    //not lead to always having a childlist
    if ( m_childobjects != wxNullCanvasObjectList  && !m_childobjects->size() )
    {
        delete m_childobjects;
        m_childobjects = wxNullCanvasObjectList;
    }

    return did;
}

int a2dCanvasObject::IndexOf( a2dCanvasObject* obj ) const
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return -1;

    return m_childobjects->IndexOf( obj );
}



void a2dCanvasObject::SetTemplate( bool b )
{
    m_flags.m_template = b;
}


void a2dCanvasObject::SetExternal( bool b )
{
    m_flags.m_external = b;
}


void a2dCanvasObject::SetUsed( bool b )
{
    m_flags.m_used = b;
}


bool a2dCanvasObject::GetTemplate() const
{
    return m_flags.m_template;
}


bool a2dCanvasObject::GetExternal() const
{
    return m_flags.m_external;
}


bool a2dCanvasObject::GetUsed() const
{
    return m_flags.m_used;
}

bool a2dCanvasObject::IsTemporary_DontSave() const
{
    if ( PROPID_TemporaryObject->GetPropertyValue( this ) )
        return true;

    // Objects belonging to the tool definitely shouldn't be saved.
    if( HasProperty( a2dCanvasObject::PROPID_ToolObject ) )
        return true;

    if( m_flags.m_editingCopy )
        return true;

    return false;
}

void a2dCanvasObject::ShowPins( bool onoff )
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return;

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if ( pin )
            pin->SetVisible( onoff );
    }
}

void a2dCanvasObject::SetRenderConnectedPins( bool onoff )
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return;

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if ( pin )
            pin->SetRenderConnected( onoff );
    }
}

void a2dCanvasObject::ClearAllPinConnections( bool withundo )
{
    if (HasPins() )
        DisConnectWith( NULL, wxT( "" ), withundo );
}

bool a2dCanvasObject::HasPins( bool realcheck )
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return false;

    if ( !realcheck )
        return m_flags.m_hasPins;

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if ( pin && !pin->GetRelease( ) )
        {
            m_flags.m_hasPins = true;
            return true;
        }
    }
    m_flags.m_hasPins = false;
    return false;
}

a2dPin* a2dCanvasObject::HasPinNamed( const wxString pinName, bool NotConnected )
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return NULL;

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if ( pin && !pin->GetRelease( ) &&
                pin->GetName().Matches( pinName ) &&
                ( !NotConnected || !pin->IsConnectedTo() ) )
        {
            return pin;
        }
    }
    return NULL;
}

int a2dCanvasObject::GetPinCount()
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return 0;

    int nrpins = 0;

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        if ( !obj )
            continue;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if ( pin && !pin->GetRelease( ) )
            nrpins++;
    }
    return nrpins;
}

a2dPin* a2dCanvasObject::AddPin( const wxString name, double x, double y, wxUint32 a2dpinFlags, a2dPinClass* pinClass, bool undo )
{
    //wxASSERT_MSG( !HasPinNamed( name ) , wxT("this pin is already in the object, name must be unique") );

    wxASSERT_MSG( pinClass , wxT( "pinClass may not be NULL" ) );

    a2dPin* newPin = wxStaticCast( pinClass->GetPin()->Clone( clone_deep ), a2dPin );

    newPin->SetName( name );
    newPin->SetParent( this );
    newPin->SetPinClass( pinClass );
    newPin->SetPosXY( x, y );
    newPin->SetDynamicPin( ( a2dpinFlags & a2dPin::dynamic ) > 0 );
    newPin->SetTemporaryPin( ( a2dpinFlags & a2dPin::temporary ) > 0 );
    newPin->SetObjectPin( ( a2dpinFlags & a2dPin::objectPin ) > 0 );

    //and now there are pins.
    m_flags.m_hasPins = true;

    if ( undo )
    {
        a2dCanvasCommandProcessor* cmp = GetRoot()->GetCanvasCommandProcessor();
        cmp->Submit( new a2dCommand_AddObject( this, newPin ) );
    }
    else
        Append( newPin );

    return newPin;
}

void a2dCanvasObject::RemovePins( bool NotConnected, bool onlyTemporary, bool now )
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return;

    bool all = true; //when allpins are removed

    a2dCanvasObjectList::iterator iter = m_childobjects->begin();
    while( iter != m_childobjects->end() )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        a2dPin* pin = wxDynamicCast( obj.Get(), a2dPin );
        if ( pin )
        {
            // remove not connected pins if told to do so.
            // remove temporary pins if told so, but only if the mouse is not inside it.
            // This keeps pins a life until the mouse leafs the pin.
            if (
                ( !NotConnected || !pin->IsConnectedTo() ) &&
                ( !onlyTemporary || pin->IsTemporaryPin() ) //&& !pin->GetMouseInObject() ) //some tools depend on removing pins, even if mouse is on them.
            )
            {
                if ( now )
                {
                    iter = m_childobjects->erase( iter );
                }
                else
                {
                    pin->SetRelease( true );
                    pin->SetPending( true );
                    iter++;
                }
            }
            else
            {
                all = false;
                iter++;
            }
        }
        else
            iter++;
    }
    if ( all )
        m_flags.m_hasPins = false;
}

bool a2dCanvasObject::IsConnected( bool needsupdate, a2dCanvasObject* toConnect )
{
    bool res = false;

    if ( m_childobjects == wxNullCanvasObjectList || !m_flags.m_hasPins )
        return false;

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if ( pin && !pin->GetRelease( ) )
        {
            a2dPinList::const_iterator iter;
            for ( iter = pin->GetConnectedPins().begin( ) ; iter != pin->GetConnectedPins().end( ) ; iter++ )
            {
                a2dPin* otherpin = *iter;
                if ( !otherpin || otherpin->GetRelease() )
                    continue;
                if ( !toConnect || toConnect == otherpin->GetParent() )
                {
                    if ( needsupdate )
                    {
                        if ( otherpin->GetParent()->NeedsUpdateWhenConnected() )
                            return true;
                    }
                    else
                        return true;
                }
            }
        }
    }

    return res;
}

bool a2dCanvasObject::FindConnectedPins( a2dCanvasObjectList& result, a2dPin* pin, bool walkWires, a2dPinClass* searchPinClass, a2dCanvasObject* isConnectedTo )
{
    if ( m_childobjects == wxNullCanvasObjectList || !HasPins() )
        return false;

    bool hasConnectedPins = false;

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( !pinc || pinc->GetRelease( ) )
            continue;
        if ( !pinc->GetBin() && ( !pin || ( pinc == pin ) ) 
             //&&
             //std::find( result.begin(), result.end(), pinc ) == result.end()
           )
        {
            pinc->SetBin( true );
            a2dPinList::const_iterator iter;
            for ( iter = pinc->GetConnectedPins().begin( ) ; iter != pinc->GetConnectedPins().end( ) ; iter++ )
            {
                a2dPin* pincother = *iter;

                if ( !pincother || pincother->GetBin() || pincother->GetRelease() )
                    continue;

                wxASSERT_MSG( 
                  pincother->IsConnectedTo( pinc )
                  , _( "connected pinc not reflected in connected pin" ) );

                // recurse into a connected bin if we want to walk accross wires to
                // connected objects.
                if ( ( pincother->GetParent()->IsConnect() || pincother->GetParent()->IsVirtConnect() ) && 
                      walkWires )
                {
                    // search recursive on all pins of wire/connect object.
                    hasConnectedPins |= pincother->GetParent()->FindConnectedPins( result, NULL, walkWires, searchPinClass, isConnectedTo );
                }
                else
                {
                    // search recursive on the connected pin for other wires and objects.
                    hasConnectedPins |= pincother->GetParent()->FindConnectedPins( result, pincother, walkWires, searchPinClass, isConnectedTo );
                    if ( 
                        ( !searchPinClass || searchPinClass == pincother->GetPinClass() ) &&
                        ( !isConnectedTo || isConnectedTo == pincother->GetParent() )
                       )
                    {
                        hasConnectedPins = true;
                        result.push_back( pincother ); //non wires pin which is connected to some other object.
                    }
                }
            }
        }
    }
    return hasConnectedPins;
}

bool a2dCanvasObject::FindConnectedWires( a2dCanvasObjectList& result, a2dPin* pin, bool walkWires, bool selectedEnds, bool stopAtSelectedWire, bool addToResult )
{    
    if ( m_childobjects == wxNullCanvasObjectList || !HasPins() )
        return false;

    //if one or all branches from this wire down are oke
    bool branchesOke = false;
    bool allBranchesOke = true;

    SetBin( true ); //start object, or recursive wires we walk on

    // when trying to find wire in between selected objects, we do not want wires returning to the start object.
    if ( selectedEnds && !addToResult )
    {
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pinc = wxDynamicCast( obj, a2dPin );
            if ( !pinc || pinc->GetRelease( ) )
                continue;

            pinc->SetBin( true ); //don't go here again.
        }
    }
    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( !pinc || pinc->GetRelease( ) )
            continue;

        if ( !pin || ( pinc == pin ) )
        {
            pinc->SetBin( true ); //don't go here again.
            //we are on he pin asked for, or we pass all pins, and no go onto branches from this wire.
            a2dPinList::const_iterator iterconp;
            if ( pinc->GetConnectedPins().empty() && !selectedEnds )
                 branchesOke = true;

            for ( iterconp = pinc->GetConnectedPins().begin( ) ; iterconp != pinc->GetConnectedPins().end( ) ; iterconp++ )
            {
                a2dPin* pincother = *iterconp;
                // the bin flag on pin prevent going back where we came from, or continuing to an already passed object.
                if ( pincother && ( pincother->GetRelease() || pincother->GetBin() ) )                
                    continue;

                if ( pincother )                
                {
                    wxASSERT_MSG( 
                          pincother->IsConnectedTo( pinc )
                          , _( "connected pinc not reflected in connected pin" ) );
                    a2dCanvasObject* parentOther = pincother->GetParent();
                    if ( !parentOther->GetBin() )
                    {
                        // recurse into a connected pin if we want to walk accross wires to
                        // connected objects.
                        if ( parentOther->IsConnect() )
                        {
                            if ( walkWires && ( !stopAtSelectedWire || stopAtSelectedWire && !parentOther->IsSelected() ) )
                            {
                                bool downHereOke = parentOther->FindConnectedWires( result, NULL, walkWires, selectedEnds, false, true );
                                branchesOke |= downHereOke; //one branch oke, we already want this wire, leading to that branch.
                                allBranchesOke &= downHereOke;
                            }
                        }
                        else
                        {
                            if ( !selectedEnds || selectedEnds && parentOther->IsSelected() )
                            {
                                branchesOke = true;
                            }
                            else
                                allBranchesOke = false;
                        }
                    }
                    else 
                    {
                        // connected object with bin flag set
                        if ( parentOther->IsConnect() )
                        {
                            //maybe needs also check if in result, to make it oke.
                            //if ( std::find( result.begin(), result.end(), parentOther ) !=  result.end() )
                            if ( !selectedEnds || selectedEnds && parentOther->IsSelected() )
                                branchesOke = true;
                        }
                        else
                        {
                            if ( !selectedEnds || selectedEnds && parentOther->IsSelected() )
                            {
                                branchesOke = true;
                            }
                            else
                                allBranchesOke = false;
                        }
                    }
                }
                else 
                {
                    //non connected pin (loose end ), for wanting all wires (non selected) we want this one too.
                    if ( !selectedEnds )
                        branchesOke = true;
                }
            }
        }
    }

    if ( addToResult && branchesOke ) //&& std::find( result.begin(), result.end(), this ) == result.end() )
        result.push_back( this );

    return branchesOke;
}

/* remove when FindWiresPinToPin2 functional
bool a2dCanvasObject::FindWiresPinToPin( a2dPin* pinFrom, a2dPin* pinTo, a2dCanvasObjectList* result, a2dPinPtr& wentToPin )
{    
    bool found = false;

    if ( m_childobjects == wxNullCanvasObjectList || !HasPins() )
        return false;

    SetBin( true ); //start object, or recursive wires we walked on

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( !pinc || pinc->GetRelease( ) )
            continue;

        if ( !pinFrom || ( pinc == pinFrom ) )
        {
            pinc->SetBin( true ); //don't go here again.
            //we are on the pin asked for, or we pass all pins, and not go onto branches from this wire.
            a2dPinList::const_iterator iterconp;
            for ( iterconp = pinc->GetConnectedPins().begin( ) ; iterconp != pinc->GetConnectedPins().end( ) ; iterconp++ )
            {
                a2dPin* pincother = *iterconp;
                // the bin flag on pin prevent going back where we came from, or continuing to an already passed object.
                if ( pincother && ( pincother->GetRelease() || pincother->GetBin() ) )                
                    continue;

                if ( pincother )                
                {
                    if ( pincother == pinTo )
                    {
                        wentToPin = pincother;
                        found = true;
                        return found;
                    }
                         
                    wxASSERT_MSG( 
                          pincother->IsConnectedTo( pinc )
                          , _( "connected pinc not reflected in connected pin" ) );
                    a2dCanvasObject* parentOther = pincother->GetParent();
                    if ( parentOther->IsConnect() || pincother->GetParent()->IsVirtConnect() )
                    {
						a2dPinPtr wentToPinHere = NULL;
                        // we are on a wire and go to all pins, and maybe find the path to pinTo
                        found = parentOther->FindWiresPinToPin( NULL, pinTo, result, wentToPinHere );
                        if ( found )
                        {    
                            wentToPin = pincother;
                            if ( !pinc->IsSameLocation( wentToPinHere ) ) 
                            {
                                parentOther->SetBin2( true );
                                if ( result )
                                    result->push_back( parentOther );
                            }
                            return found;
                        }
                    }
                }
            }
        }
    }

    return found;
}
*/

bool a2dCanvasObject::FindWiresPinToPin2( a2dPin* pinFrom, a2dPin* pinTo, a2dCanvasObjectList* result, a2dPinPtr& wentToPin )
{    
    bool found = false;

    if ( m_childobjects == wxNullCanvasObjectList || !HasPins() )
        return false;

    SetBin( true ); //start object, or recursive wires we walked on

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( !pinc || pinc->GetRelease( ) )
            continue;

        //pinFrom was a pin connected to an other pin, and called this function when found in a2dPin::FindWiresToPin()
        //That other pin was already maked "bin", so there it will not go again.
        //Still other pins connected to pinFrom can be there, contnue if not bin on them yet.
        if ( !pinc->GetBin() )
        {         
            found = pinc->FindWiresToPin( pinTo, result );
            if ( found )
            {
                wentToPin = pinc;
                return found;
            }
        }
    }

    return found;
}

bool a2dCanvasObject::GetConnected( a2dCanvasObjectList* connected, bool needsupdate )
{
    bool res = false;
    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( !pinc )
            continue;
        a2dPinList::const_iterator iterconp;
        for ( iterconp = pinc->GetConnectedPins().begin( ) ; iterconp != pinc->GetConnectedPins().end( ) ; iterconp++ )
        {
            a2dPin* pincother = *iterconp;
            if ( !pincother || pincother->GetRelease() )
                continue;
            if ( needsupdate )
            {
                if ( pincother->GetParent()->NeedsUpdateWhenConnected() )
                {
                    res = true;
                    connected->push_back( pincother->GetParent() );
                }
            }
            else
            {
                res = true;
                connected->push_back( pincother->GetParent() );
            }
        }
    }
    return res;
}

bool a2dCanvasObject::CanConnectWith( a2dIterC& ic, a2dCanvasObject* toConnect, bool autocreate )
{
    if ( m_childobjects == wxNullCanvasObjectList || !HasPins() )
        return false;

    bool done = false;

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( pinc && pinc->IsVisible() && !pinc->GetRelease( ) )
        {
            //search pins at same position in object to connect

            a2dPin* connect = toConnect->CanConnectWith( ic, pinc, ic.GetHitMarginWorld(), autocreate );
            if ( connect && !connect->IsConnectedTo( pinc ) && !pinc->GetBin() )
            {
                pinc->SetBin( true );
                done = true;
            }
        }
    }

    return done;
}

bool a2dCanvasObject::DoCanConnectWith( a2dIterC& ic, a2dPin* pin, double margin, bool autocreate )
{
    if ( autocreate )
    {
        //first remove autogenerated pins, which were (possible) not for a specific pinClass.
        RemovePins( true, true );
        a2dHitEvent hitevent = a2dHitEvent( pin->GetAbsX(), pin->GetAbsY(), false );
        a2dCanvasObject* hit = IsHitWorld( ic, hitevent );
        if( hit )
        {
            bool found = false;
            // generate pins which can connect this pin
            found = GeneratePinsPossibleConnections( pin->GetPinClass(), a2d_GeneratePinsForPinClass, pin->GetPosX(), pin->GetPosY(), margin ) || found;
            return found;
        }
    }
    return false;
}

a2dPin* a2dCanvasObject::CanConnectWith( a2dIterC& ic, a2dPin* pin, double margin, bool autocreate )
{
    if ( !m_flags.m_doConnect || !pin->IsVisible() )
        return ( a2dPin* ) NULL;

    DoCanConnectWith( ic, pin, margin, autocreate );

    if ( m_childobjects == wxNullCanvasObjectList )
        return ( a2dPin* ) NULL;

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( pinc && pinc->IsVisible() && !pinc->GetRelease( ) )
        {
            if ( fabs( pin->GetAbsX() - pinc->GetAbsX() ) < margin  &&
                    fabs( pin->GetAbsY() - pinc->GetAbsY() ) < margin  &&
                    pin->MayConnectTo( pinc ) )
            {
                return pinc;
            }
        }
    }

    return ( a2dPin* )NULL;
}

bool a2dCanvasObject::ConnectWith( a2dCanvasObject* parent, a2dPin* pin, double margin, bool undo )
{
    a2dIterC ic;
    a2dPin* pinfound = CanConnectWith( ic, pin, margin, true );
    if ( pinfound && pinfound->IsTemporaryPin() ) //those pins are generated without commands, not good for undo.
    {
        pinfound = wxStaticCast( pinfound->Clone( clone_deep ), a2dPin );
        pinfound->SetTemporaryPin( false );
        if ( undo )
            m_root->GetCommandProcessor()->Submit( new a2dCommand_AddObject( pinfound->GetParent(), pinfound ), true ); //this makes it save
        else
            pinfound->GetParent()->Append( pinfound );
        return true;
    }
    if ( pinfound )
    {
        ConnectPins( parent, pinfound, pin, undo );
        return true;
    }
    return false;
}

bool a2dCanvasObject::ConnectWith( a2dCanvasObject* parent, a2dCanvasObject* toConnect, const wxString& pinname, double margin, bool undo )
{
    if ( m_childobjects == wxNullCanvasObjectList || !m_flags.m_doConnect || toConnect == this )
        return false;

    bool done = false;
    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( pinc && pinc->IsVisible() && !pinc->GetRelease( ) )
        {
            if ( pinname.IsEmpty() )
            {
                //search pins at same position in object to connect

                for( a2dCanvasObjectList::iterator itero = toConnect->m_childobjects->begin(); itero != toConnect->m_childobjects->end(); ++itero )
                {
                    a2dCanvasObject* obj = *itero;
                    if ( !obj )
                        continue;
                    a2dPin* pinother = wxDynamicCast( obj, a2dPin );
                    if ( pinother && pinother->IsVisible() && !pinother->GetRelease( ) )
                    {
                        bool fits = false;
                        if ( fabs( pinother->GetAbsX() - pinc->GetAbsX() ) < margin  &&
                                fabs( pinother->GetAbsY() - pinc->GetAbsY() ) < margin )
                            fits = true;

                        if ( fits &&
                                ( !pinother->IsConnectedTo( pinc ) && //are NOT already connected?
                                  pinother->MayConnectTo( pinc ) ) //is allowed?
                           )
                        {
                            /*todo

                            //now we have two pins on top of eachother, and they are not connected with eachother,
                            //but connection is allowed, and also they may be already connected to others.

                            //if this object is already connected to the other object via a pin at this point,
                            //we do not connect to a second one.
                            //The reason for this situation becomes clear when all pins are placed at the same position.
                            //e.g. after a scaling action.
                            //I call this (zero wire or zero Length object)
                            if (
                                ( pinother->ConnectedTo() && pinother->ConnectedTo()->GetParent() == this ) ||
                                ( pinother->GetParent()->IsConnected( false, this ) )
                            ) //toConnect object already connected to this object?
                            {
                                if ( pinother->GetParent()->IsConnect() ) // is a wire?
                                {
                                    //zero wire, do not connect again.
                                }
                                else
                                {
                                    //zero object do not connect again.
                                }
                            }
                            //if poth pins are connected to a third object at both pins (at same position),
                            //currently they will not be connected.
                            else 
                            */    
                            if (
                                pinc->IsConnectedTo() && pinother->IsConnectedTo() //both connected?
                                //todo pinc->ConnectedTo()->GetParent() == pinother->ConnectedTo()->GetParent() //same connect object?
                            )
                            {
                                //zero wire or zero object
                                //TODO remove zero wires, and make a one to one connection between the pins ???
                            }
                            else
                            {
                                ConnectPins( parent, pinc, pinother, undo );
                            }
                            done = true;
                        }
                    }
                }
            }
            else
            {
                //search pin with pinname in object to connect ( should only be one! )
                for( a2dCanvasObjectList::iterator itero = toConnect->m_childobjects->begin(); itero != toConnect->m_childobjects->end(); ++itero )
                {
                    a2dCanvasObject* obj = *itero;
                    if ( !obj )
                        continue;
                    a2dPin* pinother = wxDynamicCast( obj, a2dPin );
                    if ( pinother && pinother->IsVisible() && !pinother->GetRelease( ) && pinname == pinother->GetName() )
                    {
                        if ( fabs( pinother->GetAbsX() - pinc->GetAbsX() ) < margin  &&
                                fabs( pinother->GetAbsY() - pinc->GetAbsY() ) < margin &&
                                pinother->MayConnectTo( pinc )
                           )
                        {
                            if  ( !pinother->IsConnectedTo( pinc ) )
                                ConnectPins( parent, pinc, pinother, undo );
                            return true;
                        }
                        else
                        {
                            CreateConnectObject( parent, pinc, pinother, undo );
                            return true;
                        }
                    }
                }
            }
        }
    }

    return done;
}

bool a2dCanvasObject::CleanWires( a2dCanvasObjectFlagsMask mask )
{
    bool did = false;

    for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj && obj->IsConnect() && obj->CheckMask( mask ) )
        {
            forEachIn( a2dCanvasObjectList, obj->m_childobjects )
            {
                a2dCanvasObject* objd = *iter;
                a2dPinPtr pin = wxDynamicCast( objd, a2dPin );
                if ( pin && !pin->GetRelease() ) 
                {
                    a2dPinList::const_iterator iterconp;
                    for ( iterconp = pin->GetConnectedPins().begin( ) ; iterconp != pin->GetConnectedPins().end( ) ; iterconp++ )
                    {
                        a2dPin* pincother = *iterconp;
                        if ( !pincother || pincother->GetRelease() || !pincother->GetParent()->IsConnect() )
                            continue;

                        wxASSERT_MSG( 
                              pincother->IsConnectedTo( pin )
                              , _( "connected pinc not reflected in connected pin" ) );

                        if ( pin->GetParent()->GetClassInfo()->IsKindOf( pincother->GetParent()->GetClassInfo() )	)
                        {
                            did = true;
                        }
                    }
                }
            }
        }
    }
    return did;
}

void a2dCanvasObject::ConnectPins( a2dCanvasObject* WXUNUSED( parent ), a2dPin* pinc, a2dPin* pinother, bool undo )
{
    wxASSERT_MSG( fabs( pinc->GetAbsX() - pinother->GetAbsX() ) < GetHabitat()->ACCUR() &&
                  fabs( pinc->GetAbsY() - pinother->GetAbsY() ) < GetHabitat()->ACCUR()
                  , _( "two pins are not at same position" ) );

    if ( undo )
    {
        m_root->GetCommandProcessor()->Submit( new a2dCommand_ConnectPins( pinc, pinother ), true );
    }
    else
    {
        pinc->ConnectTo( pinother );
    }
}

void a2dCanvasObject::ConnectPinsCreateConnect( a2dCanvasObject* parent, a2dPin* pinc, a2dPin* pinother, bool undo )
{
    if ( pinother->IsConnectedTo( pinc ) && pinc->IsConnectedTo( pinother ) )//already connected at this pin.
    {
        if ( fabs( pinc->GetAbsX() - pinother->GetAbsX() ) < GetHabitat()->ACCUR() &&
                fabs( pinc->GetAbsY() - pinother->GetAbsY() ) < GetHabitat()->ACCUR() )
        {
            //exactly the same position, no need to create a wire.
            return;
        }
        else
        {
            //disconnect in order to create wire.
            ConnectPins( parent, NULL, NULL, undo );
        }
    }

    if ( fabs( pinc->GetAbsX() - pinother->GetAbsX() ) < GetHabitat()->ACCUR() &&
            fabs( pinc->GetAbsY() - pinother->GetAbsY() ) < GetHabitat()->ACCUR() )
    {
        ConnectPins( parent, pinc,  pinother, undo );
    }
    else
    {
        CreateConnectObject( parent, pinc, pinother, undo );
    }
}

bool a2dCanvasObject::DisConnectWith( a2dCanvasObject* toDisConnect, const wxString& pinname, bool undo )
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return false;

    bool done = false;
    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( pinc )
        {
            if ( toDisConnect  )
            {
                for( a2dCanvasObjectList::iterator itero = toDisConnect->m_childobjects->begin(); itero != toDisConnect->m_childobjects->end(); ++itero )
                {
                    a2dCanvasObject* obj = *itero;
                    a2dPin* pinother = wxDynamicCast( obj, a2dPin );
                    if ( pinother && pinother->IsConnectedTo( pinc ) ) //connected pin?
                    {
                        if ( pinname.IsEmpty() || pinname == pinc->GetName() )
                        {
                            if ( undo )
                                m_root->GetCommandProcessor()->Submit( new a2dCommand_DisConnectPins( pinc, pinother ), true );
                            else
                                pinc->Disconnect( pinother );
                            done = true;
                        }
                    }
                }
            }
            else 
            {
                if ( undo )
                {
                    a2dPinList::const_iterator iterconp;
                    for ( iterconp = pinc->GetConnectedPins().begin( ) ; iterconp != pinc->GetConnectedPins().end( ) ; iterconp++ )
                    {
                        a2dPin* pinother = *iterconp;
                        if ( !pinother || pinother->GetRelease() )
                            continue;
                        m_root->GetCommandProcessor()->Submit( new a2dCommand_DisConnectPins( pinc, pinother ), true );
                        done = true;
                    }
                }
                else
                    pinc->Disconnect( NULL );
            }
        }
    }
    return done;
}

bool a2dCanvasObject::DisConnectAt( a2dPin* toDisConnect, bool undo )
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return false;

    bool done = false;
    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( pinc && ( pinc == toDisConnect || !toDisConnect ) )           
            {
            a2dPinList::const_iterator iterconp;
            for ( iterconp = pinc->GetConnectedPins().begin( ) ; iterconp != pinc->GetConnectedPins().end( ) ; iterconp++ )
            {
                a2dPin* pinother = *iterconp;
                if ( !pinother || pinother->GetRelease() )
                    continue;
                if ( undo )
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_DisConnectPins( pinc, pinother ), true );
                else
                    pinc->Disconnect( pinother );
                done = true;
            }
        }
    }
    return done;
}

void a2dCanvasObject::ReWireConnected( a2dCanvasObject* parent, bool undo )
{
    if ( parent && HasPins() && !IsConnect() )
    {
        //connect if hit on pin of other object
        if ( DoConnect() )
        {
            a2dCanvasObjectList allpinobjects;
            parent->CollectObjects( &allpinobjects, wxT( "" ), a2dCanvasOFlags::HasPins | a2dCanvasOFlags::VISIBLE );
            for( a2dCanvasObjectList::const_iterator iter = allpinobjects.begin(); iter != allpinobjects.end(); ++iter )
            {
                a2dCanvasObject* obj = *iter;
                if ( obj != this && !obj->m_flags.m_editingCopy && !obj->IsConnect() && obj->DoConnect() )
                {
                    //connect this object with the other if possible.
                    // New wires will be added to the parent of objects.
                    ConnectWith( parent, obj , wxT( "" ), GetHabitat()->ACCUR(), undo );
                }
            }
        }
    }
}

bool a2dCanvasObject::CreateWiresOnPins( a2dCanvasObject* parent, bool undo, bool onlyNonSelected )
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return false;

    if ( HasPins() )//&& !IsConnect() && DoConnect() )
    {
        bool done = false;
        forEachIn( a2dCanvasObjectList, m_childobjects )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pinc = wxDynamicCast( obj, a2dPin );
            if ( pinc && pinc->IsVisible() && !pinc->GetRelease() )
            {
                a2dPinList::const_iterator iterconp;
                for ( iterconp = pinc->GetConnectedPins().begin( ) ; iterconp != pinc->GetConnectedPins().end( ) ; iterconp++ )
                {
                    a2dPin* other = *iterconp;
                    if ( !other || other->GetRelease() )
                        continue;

                    //if already connected at this pin create a connect if needed.
                    if ( other && other->IsVisible() &&
                         ( !onlyNonSelected || onlyNonSelected && !other->GetParent()->IsSelected() )
                       )
                    {
                        a2dCanvasObject* otherobj = other->GetParent();
                        if ( otherobj->IsConnect() )
                        {
                            a2dWirePolylineL* wire = wxStaticCast( otherobj, a2dWirePolylineL );
                            // if connected object is already a wire, it can be reused in general.
                            if ( (wire->FindBeginPin() != other && wire->FindEndPin() != other)
                                 || ( wire->GetPinCount() > 2 && wire->GetNumberOfSegments() <= 2 )
                               )
                            {
                                pinc->Disconnect();
                                CreateConnectObject( parent, pinc, other, undo );
                                done = true;
                            }
                        }
                        else
                        {
                            pinc->Disconnect();
                            CreateConnectObject( parent, pinc, other, undo );
                            done = true;
                        }
                    }
                }
            }
        }
        return done;
    }
    return false;
}

bool a2dCanvasObject::SetConnectedPending( bool WXUNUSED( onoff ), bool needsupdateonly )
{
    bool res = false;
    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pinc = wxDynamicCast( obj, a2dPin );
        if ( pinc && !pinc->GetRelease( ) )
        {
            a2dPinList::const_iterator iterconp;
            for ( iterconp = pinc->GetConnectedPins().begin( ) ; iterconp != pinc->GetConnectedPins().end( ) ; iterconp++ )
            {
                a2dPin* pincother = *iterconp;

                if ( !pincother || pincother->GetRelease() )
                    continue;

                a2dCanvasObject* conparent = pincother->GetParent();
                if ( conparent && !conparent->GetRelease() )
                {
                    if ( needsupdateonly )
                    {
                        if ( conparent->NeedsUpdateWhenConnected() )
                        {
                            res = true;
                            conparent->SetPending( true );
                        }
                    }
                    else
                    {
                        res = true;
                        conparent->SetPending( true );
                    }
                }
            }
        }
    }
    return res;
}

bool a2dCanvasObject::ProcessCanvasEventChild( a2dIterC& ic, RenderChild& whichchilds, a2dHitEvent& hitEvent )
{
    if ( m_childobjects == wxNullCanvasObjectList || m_childobjects->empty() )
        return false;

    // when corridor is set, we do not iterate layers, only follow corridor objects.
    // When arrived at the end of the corridor, we do normal event processing, unless an object in there was captured.
    if ( ic.GetDrawingPart()->GetEndCorridorObject() && 
         !ic.GetFoundCorridorEnd() 
       )
    {
        wxASSERT_MSG( GetIsOnCorridorPath(), _T( "corridor set, but object is not on corridor" ) );
        a2dCanvasObjectList::reverse_iterator iter = m_childobjects->rbegin();
        while( iter != m_childobjects->rend() )
        {
            a2dCanvasObject* obj = *iter;
            if ( !obj )
            {
                iter++;
                continue;
            }

            iter++;

            // if at end of corridor, go only there with events.
            if ( obj == ic.GetDrawingPart()->GetEndCorridorObject() )
            {
                obj->ProcessCanvasObjectEvent( ic, hitEvent );
                return hitEvent.m_processed; 
            }
            else if ( obj->GetIsOnCorridorPath() )
            {
                obj->ProcessCanvasObjectEvent( ic, hitEvent );
            }
        }
    }
    else 
    {
        //iterate over layers if the children are meant to be drawn on a given layer at once
        //OR we do NOT render per layer from a parent object.
        if( ic.GetPerLayerMode() && m_flags.m_childrenOnSameLayer &&  m_root->GetLayerSetup() )
        {
            wxASSERT_MSG( ic.GetLayer() != wxLAYER_ALL, _T( " a2dIterC::GetPerLayerMode() can only hit one layer at the time" ) );

            wxUint16 layer = ic.GetLayer();

            a2dLayerIndex::iterator itb = GetRoot()->GetLayerSetup()->GetReverseOrderSort().begin();
            a2dLayerIndex::iterator ite = GetRoot()->GetLayerSetup()->GetReverseOrderSort().end();
            // YES here we do the opposite of when rendering, hitting the top layer first
            if ( ic.GetDrawingPart()->GetReverseOrder() )
            {
                itb = GetRoot()->GetLayerSetup()->GetOrderSort().begin();
                ite = GetRoot()->GetLayerSetup()->GetOrderSort().end();
            }
            for ( a2dLayerIndex::iterator it= itb; it != ite; ++it)
            {
                a2dLayerInfo* layerinfo = *it;

                //important!
                //if layer is visible it will be rendered
                //If an object on a layer is itself invisible it will not be drawn
                //wxLAYER_ALL is reserved and should never be part of the layersettings
                if ( ic.GetDrawingPart()->GetLayerRenderArray()[ layerinfo->GetLayer() ].DoRenderLayer()
                        && layerinfo->GetVisible() )
                {
                    ic.SetLayer( layerinfo->GetLayer() );
                    ProcessCanvasEventChildOneLayer( ic, whichchilds, hitEvent );
                }
            }
            ic.SetLayer( layer );
        }
        else
        {
            //no layer iteration will be done and therefore only object on m_layer == layer will be rendered.
            //layer == wxLAYER_ALL is a special case, and means ignore layer order for rendering the child objects.
            //checking of availability and visibility of layer itself is handled in parent objects
            ProcessCanvasEventChildOneLayer( ic, whichchilds, hitEvent );
        }
    }
    return hitEvent.m_processed;
}

bool a2dCanvasObject::ProcessCanvasEventChildOneLayer( a2dIterC& ic, RenderChild& whichchilds, a2dHitEvent& hitEvent )
{
    RenderChild detectchilds;
    detectchilds.m_postrender = false;
    detectchilds.m_prerender = false;
    detectchilds.m_property = false;

    //search in reverse order, last drawn on a layer, and therefore on top, will be found first for hit

    a2dCanvasObjectList::reverse_iterator iter = m_childobjects->rbegin();
    while( iter != m_childobjects->rend() )
    {
        a2dCanvasObject* obj = *iter;
        if ( !obj )
        {
            iter++;
            continue;
        }
        int olayer = obj->GetLayer();

        detectchilds.m_postrender = detectchilds.m_prerender || !obj->GetPreRenderAsChild();
        detectchilds.m_prerender = detectchilds.m_postrender || obj->GetPreRenderAsChild();
        detectchilds.m_property = detectchilds.m_property || obj->GetIsProperty();

        iter++;

        //do a rough check to increase speed in common cases
        if ( (
                    ( obj->GetPreRenderAsChild() && whichchilds.m_prerender && !obj->GetIsProperty() ) ||
                    ( !obj->GetPreRenderAsChild() && whichchilds.m_postrender && !obj->GetIsProperty() ) ||
                    ( obj->GetIsProperty() && whichchilds.m_property )
                )
                // the next does work if child is a container like e.g. a2dCanvasObjectReference
                // because
                &&
                ( olayer == ic.GetLayer() || ic.GetLayer() == wxLAYER_ALL || obj->GetChildObjectsCount() || obj->GetIgnoreLayer() )
           )
        {
            obj->ProcessCanvasObjectEvent( ic, hitEvent );
        }
    }

    whichchilds = detectchilds;

    return hitEvent.m_processed; //return
}

#ifdef _DEBUG

void a2dCanvasObject::Dump( int indent )
{
    wxString line( wxT( ' ' ), indent );

    line += wxString::Format( wxT( "a ((%s*)0x%p), x=%lg, y=%lg" ), GetClassInfo()->GetClassName(), this, GetPosX(), GetPosY() );

    DoDump( indent, &line );

    wxLogDebug( line );

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;
        obj->Dump( indent + 2 );
    }
}

void a2dCanvasObject::DoDump( int WXUNUSED( indent ), wxString* line )
{
    if( HasProperty( PROPID_ToolObject ) )
        *line += wxT( " pTO" );
    if( HasProperty( PROPID_ToolDecoration ) )
        *line += wxT( " pTD" );
    if( HasProperty( PROPID_Editcopy ) )
        *line += wxT( " pEC" );
    if( HasProperty( PROPID_Original ) )
        *line += wxT( " pOR" );
    if( m_flags.m_editingCopy )
        *line += wxT( " fEC" );
    if( m_flags.m_editing )
        *line += wxT( " fED" );
}

#endif
