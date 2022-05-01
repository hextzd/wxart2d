/*! \file canvas/src/canobj2.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canobj2.cpp,v 1.435 2009/09/30 18:38:57 titato Exp $
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

//----------------------------------------------------------------------------
// a2dCanvasObjectFilterLayerMask
//----------------------------------------------------------------------------
bool a2dCanvasObjectFilterLayerMask::Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    if ( canvasObject->m_layer != m_layer && m_layer != wxLAYER_ALL && !canvasObject->GetIgnoreLayer() )
    {
        //the object itself will NOT be rendered for sure, but maybe its children still will!

        //object not on this layer, then children will NOT be rendered also in following cases
        if ( canvasObject->m_childobjects == wxNullCanvasObjectList ||
                !canvasObject->m_flags.m_visiblechilds || canvasObject->m_flags.m_childrenOnSameLayer )
            return false;
    }

    if ( m_layer != wxLAYER_ALL && !canvasObject->GetIgnoreLayer() &&

            ( !ic.GetDrawingPart()->GetLayerRenderArray()[ m_layer ].DoRenderLayer() ||
              canvasObject->m_root && canvasObject->m_root->GetLayerSetup() &&
              !canvasObject->m_root->GetLayerSetup()->GetVisible( m_layer ) //&&
              //!canvasObject->m_root->GetLayerSetup()->GetSelectable(m_layer)
            )

       )
        return false;

    if ( m_layer == wxLAYER_ALL && !canvasObject->GetIgnoreLayer() && canvasObject->m_root->GetLayerSetup() &&
         ! canvasObject->m_root->GetLayerSetup()->GetVisible( canvasObject->GetLayer() ) 
       )
        return false;

    if ( !( canvasObject->CheckMask( m_mask ) || ic.GetLevel() == 0 ) )
        return false;

    if ( !( m_antimask == a2dCanvasOFlags::NON || !canvasObject->CheckMask( m_antimask )  || ic.GetLevel() == 0 ) )
        return false;

    return true;
}


//----------------------------------------------------------------------------
// a2dCanvasObjectFilterLayerMaskNoToolNoEdit
//----------------------------------------------------------------------------

bool a2dCanvasObjectFilterLayerMaskNoToolNoEdit::Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    if ( a2dCanvasObjectFilterLayerMask::Filter( ic, canvasObject ) )
        return !canvasObject->m_flags.m_editingCopy && ! a2dCanvasObject::PROPID_ToolObject->GetPropertyValue( canvasObject );
    return false;
}

void a2dCanvasObjectFilterLayerMaskNoToolNoEdit::EndFilter( a2dIterC& WXUNUSED( ic ), a2dCanvasObject* WXUNUSED( canvasObject ) )
{
}


//----------------------------------------------------------------------------
// a2dCanvasObjectFilterOnlyNoMaskBlind
//----------------------------------------------------------------------------
bool a2dCanvasObjectFilterOnlyNoMaskBlind::Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    if ( canvasObject->CheckMask( m_mask ) )
    {
        m_maskedCanvasObject = canvasObject;
        ic.GetDrawer2D()->SetDisableDrawing( false );
        return true;
    }

    if ( !m_maskedCanvasObject )
        ic.GetDrawer2D()->SetDisableDrawing( true );
    return true;
}

void a2dCanvasObjectFilterOnlyNoMaskBlind::EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    if ( canvasObject == m_maskedCanvasObject )
        ic.GetDrawer2D()->SetDisableDrawing( true );
}

//----------------------------------------------------------------------------
// a2dCanvasObjectFilterPropertyNoMaskBlind
//----------------------------------------------------------------------------
bool a2dCanvasObjectFilterPropertyNoMaskBlind::Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    if ( m_maskedCanvasObject ) // we are in an object with the property set ( or deeper down in it)
    {
        //ic.GetDrawer2D()->SetDisableDrawing( false );
        return canvasObject->CheckMask( m_mask );
    }
    else if ( canvasObject->HasProperty( m_id ) )
    {
        //switch on drawing, and remember the object where this happened
        m_maskedCanvasObject = canvasObject;
        ic.GetDrawer2D()->SetDisableDrawing( false );
        return canvasObject->CheckMask( m_mask );
    }

    //all other parts of the drawing ( objects not containing the property), will be drawn, but invisible.
    //so drawn invisible.
    ic.GetDrawer2D()->SetDisableDrawing( true );
    return true;
}

void a2dCanvasObjectFilterPropertyNoMaskBlind::EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    //switch the filter back to non drawing when all children and the object
    // itself are drawn.
    if ( canvasObject == m_maskedCanvasObject )
    {
        m_maskedCanvasObject = NULL; //prepare for new search.
        ic.GetDrawer2D()->SetDisableDrawing( true ); //blind drawing again
    }
}

//----------------------------------------------------------------------------
// a2dCanvasObjectFilterSelected
//----------------------------------------------------------------------------
bool a2dCanvasObjectFilterSelected::Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    if ( m_maskedCanvasObject )
        // all below a selected object will be rendered is right mask.
        return canvasObject->CheckMask( m_mask );
    else
    {
        if ( canvasObject->GetSelected() && !canvasObject->GetSelected2() )
        {
            //switch on drawing, and remember the object where this happened
            m_maskedCanvasObject = canvasObject;
            ic.GetDrawer2D()->SetDisableDrawing( false );
            return canvasObject->CheckMask( m_mask );
        }
        else
        {
            //all non selected will not be rendered.
            return ic.GetDrawingPart()->GetShowObject() == canvasObject;
            //canvasObject->CheckMask(m_mask);
        }
        /*
                if ( canvasObject->GetHasSelectedObjectsBelow() )
                {
                    if ( canvasObject->GetSelected() )
                    {   //switch on drawing, and remember the object where this happened
                        m_maskedCanvasObject = canvasObject;
                        ic.GetDrawer2D()->SetDisableDrawing( false );
                        return canvasObject->CheckMask(m_mask);
                    }
                    else
                    {
                        return canvasObject->CheckMask(m_mask);
                    }
                }
                else
                {
                    //all other parts of the drawing ( objects not selected), will be drawn, but invisible.
                    //so drawn invisible.
                    ic.GetDrawer2D()->SetDisableDrawing( true );
                    return false;
                }
        */
    }
}

void a2dCanvasObjectFilterSelected::EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    //switch the filter back to non drawing when all children and the object
    // itself are drawn.
    if ( canvasObject == m_maskedCanvasObject )
    {
        m_maskedCanvasObject = NULL; //prepare for new search.
        ic.GetDrawer2D()->SetDisableDrawing( true ); //blind drawing again
    }
}

//----------------------------------------------------------------------------
// a2dCanvasObjectFilterSelected2
//----------------------------------------------------------------------------
bool a2dCanvasObjectFilterSelected2::Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    if ( m_maskedCanvasObject )
        // all below a selected object will be rendered is right mask.
        return canvasObject->CheckMask( m_mask );
    else
    {
        if ( canvasObject->GetSelected2() )
        {
            //switch on drawing, and remember the object where this happened
            m_maskedCanvasObject = canvasObject;
            ic.GetDrawer2D()->SetDisableDrawing( false );
            return canvasObject->CheckMask( m_mask );
        }
        else
        {
            //all non selected will not be rendered.
            return ic.GetDrawingPart()->GetShowObject() == canvasObject;
            //canvasObject->CheckMask(m_mask);
        }
    }
}

void a2dCanvasObjectFilterSelected2::EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    //switch the filter back to non drawing when all children and the object
    // itself are drawn.
    if ( canvasObject == m_maskedCanvasObject )
    {
        m_maskedCanvasObject = NULL; //prepare for new search.
        ic.GetDrawer2D()->SetDisableDrawing( true ); //blind drawing again
    }
}

//----------------------------------------------------------------------------
// a2dCanvasObjectFilterHighLighted
//----------------------------------------------------------------------------
bool a2dCanvasObjectFilterHighLighted::Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    if ( m_maskedCanvasObject )
        // all below a selected object will be rendered is right mask.
        return canvasObject->CheckMask( m_mask );
    else
    {
        if ( canvasObject->GetHighLight() )
        {
            //switch on drawing, and remember the object where this happened
            m_maskedCanvasObject = canvasObject;
            ic.GetDrawer2D()->SetDisableDrawing( false );
            return canvasObject->CheckMask( m_mask );
        }
        else
        {
            //all non selected will not be rendered.
            return ic.GetDrawingPart()->GetShowObject() == canvasObject;
            //canvasObject->CheckMask(m_mask);
        }
        /*
                if ( canvasObject->GetHasSelectedObjectsBelow() )
                {
                    if ( canvasObject->GetSelected() )
                    {   //switch on drawing, and remember the object where this happened
                        m_maskedCanvasObject = canvasObject;
                        ic.GetDrawer2D()->SetDisableDrawing( false );
                        return canvasObject->CheckMask(m_mask);
                    }
                    else
                    {
                        return canvasObject->CheckMask(m_mask);
                    }
                }
                else
                {
                    //all other parts of the drawing ( objects not selected), will be drawn, but invisible.
                    //so drawn invisible.
                    ic.GetDrawer2D()->SetDisableDrawing( true );
                    return false;
                }
        */
    }
}

void a2dCanvasObjectFilterHighLighted::EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    //switch the filter back to non drawing when all children and the object
    // itself are drawn.
    if ( canvasObject == m_maskedCanvasObject )
    {
        m_maskedCanvasObject = NULL; //prepare for new search.
        ic.GetDrawer2D()->SetDisableDrawing( true ); //blind drawing again
    }
}

//----------------------------------------------------------------------------
// a2dCanvasObjectFilterToolObjects
//----------------------------------------------------------------------------
bool a2dCanvasObjectFilterToolObjects::Filter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    if ( m_maskedCanvasObject )
        return canvasObject->CheckMask( m_mask );
    else
    {
        if ( canvasObject->GetHasToolObjectsBelow() )
        {
            if ( canvasObject->HasProperty( m_id ) )
            {
                //switch on drawing, and remember the object where this happened
                m_maskedCanvasObject = canvasObject;
                ic.GetDrawer2D()->SetDisableDrawing( false );
                return canvasObject->CheckMask( m_mask );
            }
            else
            {
                return canvasObject->CheckMask( m_mask );
            }
        }
        else
        {
            //all other parts of the drawing ( objects not selected), will be drawn, but invisible.
            //so drawn invisible.
            ic.GetDrawer2D()->SetDisableDrawing( true );
            return false;
        }
    }
}

void a2dCanvasObjectFilterToolObjects::EndFilter( a2dIterC& ic, a2dCanvasObject* canvasObject )
{
    //switch the filter back to non drawing when all children and the object
    // itself are drawn.
    if ( canvasObject == m_maskedCanvasObject )
    {
        m_maskedCanvasObject = NULL; //prepare for new search.
        ic.GetDrawer2D()->SetDisableDrawing( true ); //blind drawing again
    }
}

//----------------------------------------------------------------------------
// a2dIterCU
//----------------------------------------------------------------------------
a2dIterCU::a2dIterCU( a2dIterC& ic, const a2dAffineMatrix& matrix, OVERLAP clip )
{
	m_pp = ic.Push( matrix, clip );
	m_iterC = &ic;
}

a2dIterCU::a2dIterCU( a2dIterC& ic, a2dCanvasObject* object, OVERLAP clip )
{
	m_pp = ic.Push( object, clip );
	m_iterC = &ic;
}

a2dIterCU::~a2dIterCU()
{
    m_iterC->Pop();
}

a2dIterCU::a2dIterCU( const a2dIterCU& cu )
{
}

//----------------------------------------------------------------------------
// a2dIterPP
//----------------------------------------------------------------------------
a2dIterPP::a2dIterPP( a2dIterC& ic, const a2dAffineMatrix& matrix, OVERLAP clip )
{
	m_refcount = 0;

	m_localtransform = matrix;
	m_objectLevel = false;
    m_clip = clip;

    //m_iterC->m_drawingPart->GetDrawer2D()->PushTransform( matrix );
    if( ic.m_contextList.size() )
    {
        m_object = NULL;

        m_relativetransform = ic.m_contextList.back()->m_relativetransform;
        m_relativetransform *= matrix;
    }
    else
    {
        m_object = NULL;
        m_relativetransform = matrix;
    }

    if ( !ic.m_disableInvert )
    {
        m_inverseRelativetransform = m_relativetransform;
        m_inverseRelativetransform.Invert();
    }
}

a2dIterPP::a2dIterPP( a2dIterC& ic, a2dCanvasObject* object, OVERLAP clip, a2dHit type )
{
	m_refcount = 0;
    m_objectLevel = true;
    m_clip = clip;
    m_type = type;

    m_object = object;

    if( ic.m_contextList.size() )
    {
        m_relativetransform = ic.m_contextList.back()->m_relativetransform;
        m_relativetransform *= object->GetTransformMatrix();
    }
    else
    {
        m_relativetransform *= object->GetTransformMatrix();
    }
    if ( !ic.m_disableInvert )
    {
        m_inverseRelativetransform = m_relativetransform;
        m_inverseRelativetransform.Invert();
    }
}

a2dIterPP::~a2dIterPP()
{
}

a2dIterPP::a2dIterPP( const a2dIterPP& cu )
{
    m_objectLevel = cu.m_objectLevel;
    m_object = cu.m_object;
    m_relativetransform = cu.m_relativetransform;
    m_inverseRelativetransform = cu.m_inverseRelativetransform;
}

//----------------------------------------------------------------------------
// a2dIterC
//----------------------------------------------------------------------------

a2dIterC::a2dIterC()
{
    m_disableInvert = false;
    m_levels = 0;
    m_foundCorridorEnd = false;

    m_hitmargin_world = a2dCanvasGlobals->GetHabitat()->GetHitMarginWorld();
    m_worldStrokeExtend = 0;

    m_drawingPart = new a2dDrawingPart( 100, 100, new a2dBlindDrawer2D() );
    m_ownDrawer = true;
    m_drawstyle = RenderLAYERED;
    m_layer = wxLAYER_ALL;
    m_perLayerMode = true;
    m_generateCommands = false;
    m_renderChildDerived = true;
    m_updateHint = a2dCANVIEW_UPDATE_NON;
}

a2dIterC::a2dIterC( a2dDrawingPart* drawer, int level )
{
    // variables set by a2dIterPP
    m_disableInvert = false;
    m_levels = level;
    m_foundCorridorEnd = false;

    m_drawingPart = drawer;
    m_drawstyle = RenderLAYERED;

    m_usertodevice = drawer->GetDrawer2D()->GetUserToDeviceTransform();

    m_mapping = drawer->GetDrawer2D()->GetMappingMatrix();

    m_hitmargin_world = drawer->GetHitMarginWorld();
    m_worldStrokeExtend = 0;
    m_layer = wxLAYER_ALL;
    m_generateCommands = false;
    m_ownDrawer = false;
    m_perLayerMode = true;
    m_renderChildDerived = true;
    m_updateHint = a2dCANVIEW_UPDATE_NON;
}

a2dIterC::~a2dIterC()
{
    //if ( m_ownDrawer )
    //    m_drawingPart->SetClosed();
}

a2dIterPP* a2dIterC::Push( const a2dAffineMatrix& matrix, OVERLAP clip )
{
	a2dIterPP* pp = new a2dIterPP( *this, matrix, clip );
	m_contextList.push_back( pp );

    m_drawingPart->GetDrawer2D()->PushTransform( matrix );

	if ( !GetDeepestHit() )
	{
		a2dIterPP* cup = new a2dIterPP( *this, matrix, clip );
		m_contextListDeep.push_back( cup );
	}
	return pp;
}

a2dIterPP* a2dIterC::Push( a2dCanvasObject* object, OVERLAP clip )
{
	a2dIterPP* pp = new a2dIterPP( *this, object );
	m_contextList.push_back( pp );
    m_levels++;

    m_drawingPart->GetDrawer2D()->PushTransform( object->GetTransformMatrix() );

	if ( !GetDeepestHit() )
	{
		a2dIterPP* cup = new a2dIterPP( *this, object );
		m_contextListDeep.push_back( cup );
	}
	return pp;
}

void a2dIterC::Pop()
{
    m_drawingPart->GetDrawer2D()->PopTransform();

	if (  m_contextList.size () && m_contextList.back()->m_objectLevel )
        m_levels--;

	if ( !GetDeepestHit() )
		m_contextListDeep.pop_back();

	m_contextList.pop_back();
}

void a2dIterC::Reset()
{
    m_worldStrokeExtend = 0;
    m_perLayerMode = true;
    m_renderChildDerived = true;
    m_updateHint = a2dCANVIEW_UPDATE_NON;
}

void a2dIterC::SetHitMarginDevice( int pixels )
{
    m_hitmargin_world = GetDrawer2D()->DeviceToWorldXRel( pixels );
}

void a2dIterC::SetHitMarginWorld( double world )
{
    m_hitmargin_world = world;
}

double a2dIterC::GetHitMarginWorld()
{
    return m_hitmargin_world;
}

double a2dIterC::ExtendDeviceToWorld( int extend )
{
    return GetDrawer2D()->DeviceToWorldXRel( extend );
}

double a2dIterC::GetTransformedHitMargin()
{
    return GetInverseParentTransform().TransformDistance( m_hitmargin_world );
}

const a2dAffineMatrix& a2dIterC::GetMappingTransform() const
{
    return m_drawingPart->GetDrawer2D()->GetMappingMatrix();
}

const a2dAffineMatrix& a2dIterC::GetUserToDeviceTransform() const
{
    return m_drawingPart->GetDrawer2D()->GetUserToDeviceTransform();
}

a2dDrawingPart* a2dIterC::GetDrawingPart() const
{
    return m_drawingPart;
}

a2dDrawer2D* a2dIterC::GetDrawer2D() const
{
    return m_drawingPart->GetDrawer2D();
}

a2dCanvasObject* a2dIterC::GetParent() const
{
	if ( m_contextList.size() > 2 )
	{
		a2dSmrtPtrList< a2dIterPP >::const_iterator iter =  m_contextList.end();
		iter--;
		iter--;
		a2dIterPP* pp = *iter;
		return pp->GetObject();
	}
	else
		return NULL;
}

a2dCanvasObject* a2dIterC::GetObject() const
{
	if ( m_contextList.size() )
		return m_contextList.back()->GetObject();
	else
		return NULL;
}

const a2dAffineMatrix& a2dIterC::GetTransform() const
{
	if ( m_contextList.size() )
		return m_contextList.back()->m_relativetransform;
	else
        return a2dAffineMatrix::GetIdentityMatrix();
}

OVERLAP a2dIterC::GetClipStatus() const
{
	if ( m_contextList.size() )
		return m_contextList.back()->m_clip;
	else
        return _IN;
}

OVERLAP a2dIterC::GetParentClipStatus() const
{
	if ( m_contextList.size() > 2 )
	{
		a2dSmrtPtrList< a2dIterPP >::const_iterator iter =  m_contextList.end();
		iter--;
		iter--;
		a2dIterPP* pp = *iter;
		return pp->m_clip;
	}
	else
        return _ON;
}

void a2dIterC::SetClipStatus( OVERLAP status )
{
	if ( m_contextList.size() )
		m_contextList.back()->m_clip = status;
}

const a2dAffineMatrix& a2dIterC::GetInverseTransform() const
{
    assert( !m_disableInvert );

	if ( m_contextList.size() )
		return m_contextList.back()->m_inverseRelativetransform;
    else
        return a2dAffineMatrix::GetIdentityMatrix();
}

const a2dAffineMatrix& a2dIterC::GetParentTransform() const
{
	if ( m_contextList.size() > 2 )
	{
		a2dSmrtPtrList< a2dIterPP >::const_iterator iter =  m_contextList.end();
		iter--;
		iter--;
		a2dIterPP* pp = *iter;
		return pp->m_relativetransform;
	}
    else
        return a2dAffineMatrix::GetIdentityMatrix();
}

const a2dAffineMatrix& a2dIterC::GetInverseParentTransform() const
{
    assert( !m_disableInvert );

	if ( m_contextList.size() > 2 )
	{
		a2dSmrtPtrList< a2dIterPP >::const_iterator iter =  m_contextList.end();
		iter--;
		iter--;
		a2dIterPP* pp = *iter;
		return pp->m_inverseRelativetransform;
	}
    else
        return a2dAffineMatrix::GetIdentityMatrix();
}

void a2dIterC::SetCorridorPath( bool OnOff )
{
    GetDrawingPart()->ClearCorridorPath();

    m_foundCorridorEnd = false;
    a2dCanvasObject* startCorridor = NULL;

    //search first object in context list (non matrix only).
	for( a2dSmrtPtrList< a2dIterPP >::iterator iter = m_contextList.begin(); iter != m_contextList.end(); iter++ )
    {
		a2dIterPP* pp = *iter;
		a2dCanvasObject* obj = pp->GetObject();
        if ( obj )
        {
            if ( ! startCorridor )
                startCorridor = obj;
            obj->SetIsOnCorridorPath( OnOff );
            //wxLogDebug( "corridor %s ", iter->m_object->GetClassInfo()->GetClassName() );
        }
	}

    if ( OnOff )
    {
        //wxLogDebug( "on" );
		if ( m_contextList.size() )
            GetDrawingPart()->SetCorridor( startCorridor, m_contextList.back()->GetObject() );
        else
            GetDrawingPart()->SetCorridor( NULL, NULL );
    }
    else
    {
        //wxLogDebug( "off" );
        GetDrawingPart()->SetCorridor( NULL, NULL );
    }
}

void a2dIterC::SetCorridorPathToParent()
{
    m_foundCorridorEnd = false;
    a2dCanvasObject* startCorridor = NULL;

	for( a2dSmrtPtrList< a2dIterPP >::iterator iter = m_contextList.begin(); iter != m_contextList.end(); iter++ )
    {
		a2dIterPP* pp = *iter;
		a2dCanvasObject* obj = pp->GetObject();
        if ( obj )
        {
            if ( ! startCorridor )
                startCorridor = obj;
            obj->SetIsOnCorridorPath( true );
            //wxLogDebug( "corridor %s ", iter->m_object->GetClassInfo()->GetClassName() );
        }
	}
    if ( m_contextList.back()->m_object )
        m_contextList.back()->m_object->SetIsOnCorridorPath( false );

    //wxLogDebug( "on" );
    if ( GetParent() )
        GetDrawingPart()->SetCorridor( startCorridor, GetParent() );
    else
        GetDrawingPart()->SetCorridor( startCorridor, startCorridor );
}

void a2dIterC::SetCorridorPathToObject( a2dCanvasObject* captureObject )
{
    wxASSERT( captureObject );
    m_foundCorridorEnd = false;
    a2dCanvasObject* startCorridor = NULL;

	for( a2dSmrtPtrList< a2dIterPP >::iterator iter = m_contextList.begin(); iter != m_contextList.end(); iter++ )
    {
		a2dIterPP* pp = *iter;
		a2dCanvasObject* obj = pp->GetObject();
        if ( obj )
        {
            if ( ! startCorridor )
                startCorridor = obj;
            obj->SetIsOnCorridorPath( true );
            //wxLogDebug( "corridor %s ", iter->m_object->GetClassInfo()->GetClassName() );
        }
	}
    captureObject->SetIsOnCorridorPath( true );
    GetDrawingPart()->SetCorridor( startCorridor, captureObject );
}

bool a2dIterC::FilterObject( a2dCanvasObject* canvasObject )
{
    if ( m_objectFilter )
        return m_objectFilter->Filter( *this, canvasObject );
    else
        return true;
}

void a2dIterC::EndFilterObject( a2dCanvasObject* canvasObject )
{
    if ( m_objectFilter )
        m_objectFilter->EndFilter( *this, canvasObject );
}

//----------------------------------------------------------------------------
// a2dExtendedResult
//----------------------------------------------------------------------------


a2dExtendedResult::a2dExtendedResult()
{
}

a2dExtendedResult::~a2dExtendedResult()
{
}

