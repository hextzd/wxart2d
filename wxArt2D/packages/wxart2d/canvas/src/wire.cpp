/*! \file canvas/src/wire.cpp
    \author Michael S�trop

    Copyright: 2003-2004 (c) Michael S�trop

    Licence: wxWidgets Licence

    RCS-ID: $Id: wire.cpp,v 1.71 2008/11/09 11:16:53 bionic-badger Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/polygon.h"

#include "wx/canvas/wire.h"

#include <float.h>
#include "wx/canvas/canobj.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/canvas.h"
#include "wx/canvas/tools.h"
#include "wx/canvas/canglob.h"

//#include "wx/genart/prfl.h"
//#include "wx/genart/prfltmr.h"
//#include "wx/genart/prflcntmean.h"
//#include "wx/genart/prflenbl.h"

//#define DUMP_FINAL

//----------------------------------------------------------------------------
// a2dCommand_SplitAtPin
//----------------------------------------------------------------------------

const a2dCommandId a2dCommand_SplitAtPin::Id( wxT( "SplitAtPin" ) );

a2dCommand_SplitAtPin::a2dCommand_SplitAtPin( a2dCanvasObject* parent, a2dPin* splitPin, double hitMargin )
{
    m_parent = parent; 
    m_splitPin = splitPin;
    m_wire = wxStaticCast( m_splitPin->GetParent(), a2dWirePolylineL );
    m_clone = NULL;
    m_hitMargin = hitMargin;
}

a2dCommand_SplitAtPin::~a2dCommand_SplitAtPin()
{
}

bool a2dCommand_SplitAtPin::Do()
{
    m_splitPinClone = m_wire->SplitAtPin( m_parent, m_splitPin, NULL, NULL, m_hitMargin );
    m_clone = wxStaticCast( m_splitPinClone->GetParent(), a2dWirePolylineL );

    if ( m_splitPinClone )
        return true;
    return false;
}

bool a2dCommand_SplitAtPin::Undo()
{
    a2dWirePolylineL* wire = wxStaticCast( m_splitPin->GetParent(), a2dWirePolylineL );
    bool done = wire->JoinAtPin( m_parent, m_splitPin, m_splitPinClone );
    wxASSERT( done );
    return done;
}

bool a2dCommand_SplitAtPin::Redo()
{
    m_wire->SplitAtPin( m_parent, m_splitPin, m_splitPinClone, m_clone, m_hitMargin );
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_JoinAtPin
//----------------------------------------------------------------------------

const a2dCommandId a2dCommand_JoinAtPin::Id( wxT( "JoinAtPin" ) );

a2dCommand_JoinAtPin::a2dCommand_JoinAtPin( a2dCanvasObject* parent, a2dPin* joinPin, a2dPin* joinPin2 )
{
    m_parent = parent; 
    m_joinPin = joinPin;
    m_joinPin2 = joinPin2;
    m_wire = wxStaticCast( m_joinPin->GetParent(), a2dWirePolylineL );
    m_wirePin2 = wxStaticCast( m_joinPin2->GetParent(), a2dWirePolylineL );
    m_secondRedirect = false;
    m_inFront = false;
}

a2dCommand_JoinAtPin::~a2dCommand_JoinAtPin()
{
}

bool a2dCommand_JoinAtPin::Do()
{
    a2dPin* beginConnected = m_wirePin2->FindBeginPin();
    a2dPin* endConnected = m_wirePin2->FindEndPin();
    m_oldwire = wxStaticCast( m_wire->Clone( clone_members | clone_properties ), a2dWirePolylineL );

    bool done = m_wire->MergAtPin( m_parent, m_joinPin, m_joinPin2, m_inFront, m_secondRedirect );

    if ( m_secondRedirect )
    {
        if ( m_inFront )
            m_renamedPin = endConnected;
        else
            m_renamedPin = beginConnected;
    }

    wxASSERT( done );
    return done;
}

bool a2dCommand_JoinAtPin::Undo()
{
    m_wire->SetSegments( m_oldwire->GetSegments() );
    m_wirePin2->SetRelease( false );

    // remove all pins on the wire, in order to replace with the old ones.
    a2dCanvasObjectList::iterator iterpins = m_wire->GetChildObjectList()->begin();
    while( iterpins != m_wire->GetChildObjectList()->end() )
    {
        a2dCanvasObject* obj = *iterpins;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if ( pin && !pin->GetRelease() )
        {
            a2dPin* pin = wxDynamicCast( obj, a2dPin );
            iterpins = m_wire->GetChildObjectList()->erase( iterpins );           
        }
        else
            iterpins++;
    }

    //add pins and set old parent again.
    iterpins = m_oldwire->GetChildObjectList()->begin();
    while( iterpins != m_oldwire->GetChildObjectList()->end() )
    {
        a2dCanvasObject* obj = *iterpins;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if ( pin && !pin->GetRelease() )
        {
            pin->SetParent( m_wire );
            m_wire->GetChildObjectList()->push_back( pin );
        }
        iterpins++;
    }
    iterpins = m_wirePin2->GetChildObjectList()->begin();
    while( iterpins != m_wirePin2->GetChildObjectList()->end() )
    {
        a2dCanvasObject* obj = *iterpins;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if ( pin && !pin->GetRelease() )
        {
            pin->SetParent( m_wirePin2 );
        }
        iterpins++;
    }

    m_parent->Append( m_wirePin2 );

    if ( m_secondRedirect && m_renamedPin )
    {
        if ( m_inFront )
            m_renamedPin->SetName( wxT( "end" ) );
        else
            m_renamedPin->SetName( wxT( "begin" ) );
    }

    //m_joinPin2->DuplicateConnectedPins( m_joinPin );
    m_joinPin->ConnectTo( m_joinPin2 );

    return true;
}

bool a2dCommand_JoinAtPin::Redo()
{
    bool done = m_wire->MergAtPin( m_parent, m_joinPin, m_joinPin2, m_inFront, m_secondRedirect );
    wxASSERT( done );
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_EliminateMatrix
//----------------------------------------------------------------------------

const a2dCommandId a2dCommand_EliminateMatrix::Id( wxT( "EliminateMatrix" ) );

a2dCommand_EliminateMatrix::a2dCommand_EliminateMatrix( a2dWirePolylineL* wire )
{
    m_wire = wire;
}

a2dCommand_EliminateMatrix::~a2dCommand_EliminateMatrix()
{
}

bool a2dCommand_EliminateMatrix::Do()
{
    m_matrix = m_wire->GetTransformMatrix();
    if ( !m_matrix.IsIdentity() )
        m_wire->EliminateMatrix();
    return true;
}

bool a2dCommand_EliminateMatrix::Undo()
{
    if ( !m_matrix.IsIdentity() )
    {
        a2dAffineMatrix invert = m_matrix;
        invert.Invert();
        m_wire->GetSegments()->Transform( invert );

        if ( m_wire->GetChildObjectList() != wxNullCanvasObjectList )
        {
            forEachIn( a2dCanvasObjectList, m_wire->GetChildObjectList() )
            {
                a2dCanvasObject* obj = *iter;
                if ( obj )
                    obj->Transform( invert );
            }
        }
        m_wire->SetTransformMatrix( m_matrix );
    }
    return true;
}

bool a2dCommand_EliminateMatrix::Redo()
{
    return Do();
}

//----------------------------------------------------------------------------
// a2dCommand_RemoveZeroWire
//----------------------------------------------------------------------------

const a2dCommandId a2dCommand_RemoveZeroWire::Id( wxT( "RemoveZeroWire" ) );

a2dCommand_RemoveZeroWire::a2dCommand_RemoveZeroWire( a2dCanvasObject* parent, a2dWirePolylineL* wire )
{
    m_parent = parent; 
    m_joinPin = NULL;
    m_joinPin2 = NULL;
    m_wire = wire;
}

a2dCommand_RemoveZeroWire::~a2dCommand_RemoveZeroWire()
{
}

bool a2dCommand_RemoveZeroWire::Do()
{
    a2dPin* begin = m_wire->FindBeginPin();
    a2dPin* end   = m_wire->FindEndPin();

    wxASSERT_MSG( begin->GetConnectedPinsNr() == 1, "Can only be used with one connected pin" );
    wxASSERT_MSG( end->GetConnectedPinsNr() == 1, "Can only be used with one connected pin" );

    m_joinPin  = begin->IsConnectedTo();
    m_joinPin2 = end->IsConnectedTo();

    m_joinPin->Disconnect( begin );
    m_joinPin2->Disconnect( end );

	if ( m_joinPin != m_joinPin2 ) //like zero to same pin of other object.
		m_joinPin->ConnectTo( m_joinPin2 );

    // remove the connected wire from parent.
    a2dCanvasObjectList::iterator iterp = m_parent->GetChildObjectList()->begin();
    while( iterp != m_parent->GetChildObjectList()->end() )
    {
        a2dCanvasObject* obj = *iterp;

        if ( obj && obj == m_wire )
        {
            m_wire->SetRelease( true ); //Used to detect this event 
            iterp = m_parent->GetChildObjectList()->erase( iterp );
        }
        else
            iterp++;
    }
    return true;
}

bool a2dCommand_RemoveZeroWire::Undo()
{
    m_wire->SetRelease( false );

    m_parent->Append( m_wire );

    //m_joinPin2->DuplicateConnectedPins( m_joinPin );
    a2dPin* begin = m_wire->FindBeginPin();
    a2dPin* end   = m_wire->FindEndPin();
	if ( m_joinPin != m_joinPin2 ) //like zero to same pin of other object.
		m_joinPin->Disconnect( m_joinPin2 );
    m_joinPin->ConnectTo( begin );
    m_joinPin2->ConnectTo( end );

    return true;
}

bool a2dCommand_RemoveZeroWire::Redo()
{
    return Do();
}

IMPLEMENT_DYNAMIC_CLASS( a2dWirePolylineL, a2dPolylineL )
IMPLEMENT_DYNAMIC_CLASS( a2dWireEnd, a2dCanvasObject )

//----------------------------------------------------------------------------
// a2dWirePolylineL
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dWirePolylineL, a2dPolylineL )
    EVT_CHAR( a2dWirePolylineL::OnChar )
END_EVENT_TABLE()

a2dWirePolylineL::a2dWirePolylineL()
{
    m_inverted   = false;
    m_objBegin   = NULL;
    m_wireBegin  = NULL;
    m_wireEnd    = NULL;
    m_objEnd     = NULL;
    m_wasVertical = false;
    m_oneLine = false;
    m_reroute = false;
    m_rerouteAdd = false;
    m_priority = 0;
    m_flags.m_generatePins = true; 
}

a2dWirePolylineL::a2dWirePolylineL( a2dVertexList* points, bool spline )
    : a2dPolylineL( points, spline )
{
    m_inverted   = false;
    m_objBegin   = NULL;
    m_wireBegin  = NULL;
    m_wireEnd    = NULL;
    m_objEnd     = NULL;
    m_wasVertical = false;
    m_oneLine = false;
    m_reroute = false;
    m_rerouteAdd = false;
    m_priority = 0;
    m_flags.m_generatePins = true; 
}

a2dWirePolylineL::a2dWirePolylineL( const a2dWirePolylineL& poly, CloneOptions options, a2dRefMap* refs )
    : a2dPolylineL( poly, options, refs )
{
    // wxLogDebug( "Cloing a2dWirePolylineL %p into %p", &poly, this );

    m_flags.m_draggable = poly.m_flags.m_draggable;
    m_wasVertical = poly.m_wasVertical;
    m_inverted   = poly.m_inverted;
    m_objBegin   = poly.m_objBegin;
    m_wireBegin  = poly.m_wireBegin;
    m_wireEnd    = poly.m_wireEnd;
    m_objEnd     = poly.m_objEnd;
    m_oneLine    = poly.m_oneLine;
    m_reroute = false;
    m_rerouteAdd = false;
    m_priority = 0;
    m_flags.m_generatePins = poly.m_flags.m_generatePins; 
}

a2dWirePolylineL::~a2dWirePolylineL()
{
}

a2dObject* a2dWirePolylineL::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dWirePolylineL( *this, options, refs );
}

void a2dWirePolylineL::SetConnectionInfo(
    bool inverted,
    a2dPinClass* objBegin,
    a2dPinClass* objEnd
)
{
    m_inverted   = inverted;
    m_objBegin   = objBegin;
    m_objEnd     = objEnd;
}

bool a2dWirePolylineL::IsConnect() const
{
    return true;
}

bool a2dWirePolylineL::NeedsUpdateWhenConnected() const
{
    return true;
}

void a2dWirePolylineL::AdjustBeginEndPinAngle()
{
    if ( m_lsegments->size() < 2 )
        return;

    for( a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin(); iter != GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if( !pin ) continue;

        double xOld = pin->GetPosX();
        double yOld = pin->GetPosY();

        if( pin->GetName() == wxT( "begin" ) )
        {
            // regular begin pin
            double xb, yb;
            double xbp, ybp;
            GetPosXYSegment( 0, xb, yb, false );
            GetPosXYSegment( 1, xbp, ybp, false );
            double angle = wxRadToDeg( atan2( yb-ybp, xb-xbp ) );
            pin->SetRotation( angle );
            pin->SetPosXY( xb, yb );
        }
        else if( pin->GetName() == wxT( "end" ) )
        {
            // regular end pin
            double xe, ye;
            double xep, yep;
            GetPosXYSegment( -1, xe, ye, false );
            GetPosXYSegment( m_lsegments->size()-2, xep, yep, false );
            double angle = wxRadToDeg( atan2( ye-yep, xe-xep ) );
            pin->SetRotation( angle );
            pin->SetPosXY( xe, ye );
        }
    }
    return;
}

bool a2dWirePolylineL::AdjustAfterChange( bool final )
{
    bool changed = false;
    for( a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin(); iter != GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if( !pin ) continue;

        double xOld = pin->GetPosX();
        double yOld = pin->GetPosY();

//        a2dPin *pinori = wxStaticCastNull( PROPID_Original->GetPropertyValue( pin ).Get(), a2dPin );

        if( pin->IsDynamicPin() )
        {
            // intermediate "dynamic" pin
            if( MoveDynamicPinCloseTo( pin, pin->GetAbsXY(), final ) )
                changed = true;
        }
        else if( pin->GetName() == wxT( "begin" ) )
        {
            // regular begin pin
            double xNew, yNew;
            GetPosXYSegment( 0, xNew, yNew, false );
            if( xNew != xOld || yNew != yOld )
            {
                if ( m_root->GetCommandProcessor() && final )
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( pin, PROPID_Position, a2dPoint2D( xNew, yNew ), -1, false ), final );
                else
                    pin->SetPosXY( xNew, yNew );

                changed = true;
            }
        }
        else if( pin->GetName() == wxT( "end" ) )
        {
            // regular end pin
            double xNew, yNew;
            GetPosXYSegment( -1, xNew, yNew, false );
            if( xNew != xOld || yNew != yOld )
            {
                if ( m_root->GetCommandProcessor() && final )
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( pin, PROPID_Position, a2dPoint2D( xNew, yNew ), -1, false ), final );
                else
                    pin->SetPosXY( xNew, yNew );

                changed = true;
            }
        }
    }

    return changed;
}

a2dPin* a2dWirePolylineL::SplitAtPin( a2dCanvasObject* parent, a2dPin* splitPin, a2dPin* splitPinClone, a2dWirePolylineL* clone, double hitMargin )
{
    wxASSERT_MSG( splitPin->GetParent() == this, "splitPin not part of wire" );

    if ( splitPin->GetName() == wxT( "end" ) || splitPin->GetName() == wxT( "begin" ) )
        return NULL;

    //find on which segment pin is. 
    a2dHit hit = m_lsegments->HitTestPolyline( splitPin->GetPosXY(), hitMargin );  
    if ( hit.IsHit() )
    {
        a2dVertexList::iterator iter = m_lsegments->item( hit.m_index );
        if ( !clone )
            clone = wxStaticCast( Clone( clone_members | clone_properties ), a2dWirePolylineL );
        // we now have all segments in clone, but we want only from the splitpoint towards end
        clone->m_lsegments->clear();
        clone->m_childobjects->clear();
        a2dVertexList::iterator iterc = clone->m_lsegments->begin();
        clone->m_lsegments->splice( iterc, *m_lsegments, ++iter, m_lsegments->end() );
        //extra segments at splitpoint

        // todo ( for the moment always add extra vertexes at split point, else in Undo not know if it happened or not,
        // and a non added vertex would be removed.)
        //if ( !hit.IsVertexHit() )
            m_lsegments->push_back( new a2dLineSegment( splitPin->GetPosXY() ) );
        // always add to other half.
        clone->m_lsegments->push_front( new a2dLineSegment( splitPin->GetPosXY() ) );

        //decide where end is, to name the split pin in both parts.
        //Move pins which are not on this wire, to the cloned (part2) wire, after the splitpoint.

		a2dPin* begin = FindBeginPin();
		a2dPin* end = FindEndPin();
		a2dPin* thisPinToPart2 = begin;
		
		bool beginAtStart = FindPinSegmentIndex( begin ) == 0;
		//if begin and end pin are at the same location (head to tail case), the end is forced to wire part 2. 
		//Pins which are both on part1 and part2 (overlapping segments), stay on part1. 
		if ( beginAtStart )
			thisPinToPart2 = end;

        bool foundSplitPin = false;
        a2dCanvasObjectList::iterator iterpins = m_childobjects->begin();
        while( iterpins != m_childobjects->end() )
        {
            a2dCanvasObject* obj = *iterpins;
            if( !obj )
            {
                iterpins++;           
                continue;
            }
            a2dPin* pin = wxDynamicCast( obj, a2dPin );
            if ( pin == splitPin )
                foundSplitPin = true;
            if( pin->IsTemporaryPin() )
            {
                iterpins = m_childobjects->erase( iterpins );           
                continue;
            }
			if ( pin != thisPinToPart2  )
			{
				a2dLineSegment* seg = FindPinSegment( pin );
				if ( !seg ) 
				{
					clone->m_childobjects->push_back( pin );
					pin->SetParent( clone );
					iterpins = m_childobjects->erase( iterpins );           
				}
				else
					iterpins++;
			}
			else 
			{
				clone->m_childobjects->push_back( pin );
				pin->SetParent( clone );
				iterpins = m_childobjects->erase( iterpins );           
			}
        }

        if ( !foundSplitPin )
            m_childobjects->push_back( splitPin );

        if ( !splitPinClone )
            splitPinClone = wxStaticCast( splitPin->Clone( clone_members | clone_properties ), a2dPin );
        splitPinClone->SetDynamicPin( false );
        if ( beginAtStart )
        {
            splitPinClone->SetName( wxT( "begin" ) );
            splitPin->SetName( wxT( "end" ) );
        }
        else
        {
            splitPin->SetName( wxT( "begin" ) );
            splitPinClone->SetName( wxT( "end" ) );
        }
        splitPin->SetDynamicPin( false );
        clone->Append( splitPinClone ); //sets parent to clone also
        splitPinClone->DuplicateConnectedPins( splitPin );
        splitPin->ConnectTo( splitPinClone );
        parent->Append( clone );

        return splitPinClone;
    }
    return NULL;
}

bool a2dWirePolylineL::JoinAtPin( a2dCanvasObject* parent, a2dPin* joinPin, a2dPin* joinPinOther )
{
    wxASSERT_MSG( joinPin->GetParent() == this, "splitPin not part of wire" );

    if ( joinPin->GetName() != wxT( "end" ) && joinPin->GetName() != wxT( "begin" ) )
        return false;

    if ( joinPinOther->GetName() != wxT( "end" ) && joinPinOther->GetName() != wxT( "begin" ) )
        return false;

    a2dWirePolylineL* clone = wxStaticCast( joinPinOther->GetParent(), a2dWirePolylineL );

    a2dVertexList::iterator iter = m_lsegments->end();
    m_lsegments->pop_back();
    clone->m_lsegments->pop_front();

    m_lsegments->insert ( iter, clone->m_lsegments->begin(), clone->m_lsegments->end() );
    //clone->m_lsegments->clear();

    joinPinOther->Disconnect( joinPin );
    joinPinOther->RemoveDuplicateConnectedPins( joinPin );

    //move all pins to this wire
    a2dCanvasObjectList::iterator iterpins = clone->m_childobjects->begin();
    while( iterpins != clone->m_childobjects->end() )
    {
        a2dCanvasObject* obj = *iterpins;
        if( !obj )
        {
            iterpins++;           
            continue;
        }
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if( pin->IsTemporaryPin() )
        {
            //iterpins = m_childobjects->erase( iterpins );           
            iterpins++;
            continue;
        }
        if ( pin != joinPinOther )
            m_childobjects->push_back( pin );
        iterpins = clone->m_childobjects->erase( iterpins );           
        pin->SetParent( this );
    }

    joinPin->SetName( wxT( "dynamic" ) );
    joinPin->SetDynamicPin( true );

    a2dCanvasObjectList::iterator iterp = parent->GetChildObjectList()->begin();
    while( iterp != parent->GetChildObjectList()->end() )
    {
        a2dCanvasObject* obj = *iterp;

        if ( obj && obj == clone )
            iterp = parent->GetChildObjectList()->erase( iterp );
        else
            iterp++;
    }
    return true;
}

bool a2dWirePolylineL::MergAtPin( a2dCanvasObject* parent, a2dPin* joinPin, a2dPin* joinPinOther, bool& inFront, bool& secondRedirect )
{
    wxASSERT_MSG( joinPin->GetParent() == this, "splitPin not part of wire" );

    if ( joinPin->GetName() != wxT( "end" ) && joinPin->GetName() != wxT( "begin" ) )
        return false;

    if ( joinPinOther->GetName() != wxT( "end" ) && joinPinOther->GetName() != wxT( "begin" ) )
        return false;

    a2dWirePolylineL* wireConnected = wxStaticCast( joinPinOther->GetParent(), a2dWirePolylineL );

    a2dPin* begin = FindBeginPin();
    a2dPin* end = FindEndPin();

    a2dPin* beginConnected = wireConnected->FindBeginPin();
    a2dPin* endConnected = wireConnected->FindEndPin();

    inFront = false;
    if ( begin == joinPin && joinPinOther == endConnected ) //same direction
    {
        inFront = true;
        a2dVertexList::reverse_iterator iwireConnected = wireConnected->GetSegments()->rbegin();
        iwireConnected++;
        for( iwireConnected; iwireConnected != wireConnected->GetSegments()->rend(); ++iwireConnected )
             m_lsegments->push_front( *iwireConnected );
    }
    else if ( end == joinPin && joinPinOther == endConnected ) //wrong direction, redirect
    {
        secondRedirect = true;
		if ( beginConnected )
			beginConnected->SetName( "end" );
        a2dVertexList::reverse_iterator iwireConnected = wireConnected->GetSegments()->rbegin();
        iwireConnected++;
        for( iwireConnected; iwireConnected != wireConnected->GetSegments()->rend(); ++iwireConnected )
             m_lsegments->push_back( *iwireConnected );
    }
    else if ( end == joinPin && joinPinOther == beginConnected ) //same direction
    {
        a2dVertexList::iterator iwireConnected = wireConnected->GetSegments()->begin();
        iwireConnected++;
        for( iwireConnected; iwireConnected != wireConnected->GetSegments()->end(); ++iwireConnected )
             m_lsegments->push_back( *iwireConnected );
    }
    else if ( begin == joinPin && joinPinOther == beginConnected ) //wrong direction, redirect
    {
        inFront = true;
        secondRedirect = true;
		if ( endConnected )
			endConnected->SetName( "begin" );
        a2dVertexList::iterator iwireConnected = wireConnected->GetSegments()->begin();
        iwireConnected++;
        for( iwireConnected; iwireConnected != wireConnected->GetSegments()->end(); ++iwireConnected )
             m_lsegments->push_front( *iwireConnected );
    }

    joinPinOther->Disconnect( joinPin );
    joinPinOther->RemoveDuplicateConnectedPins( joinPin );

    //remove join pin
    a2dCanvasObjectList::iterator iterpins = m_childobjects->begin();
    while( iterpins != m_childobjects->end() )
    {
        a2dCanvasObject* obj = *iterpins;
        if( !obj || obj->GetRelease() )
        {
            iterpins++;           
            continue;
        }
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if ( pin == joinPin )
            iterpins = m_childobjects->erase( iterpins );           
        else
            iterpins++;           
    }

    //move all pins to this wire and remove but not joinPinOther
    iterpins = wireConnected->m_childobjects->begin();
    while( iterpins != wireConnected->m_childobjects->end() )
    {
        a2dCanvasObject* obj = *iterpins;
        if( !obj || obj->GetRelease() )
        {
            iterpins++;           
            continue;
        }
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if( pin->IsTemporaryPin() )
        {
            //iterpins = m_childobjects->erase( iterpins );           
            iterpins++;
            continue;
        }
        if ( pin != joinPinOther )
            m_childobjects->push_back( pin );
        iterpins++;
        pin->SetParent( this );
    }

    joinPin->CleanUpNonConnected();
    joinPinOther->CleanUpNonConnected();

    // remove the connected wire from parent.
    a2dCanvasObjectList::iterator iterp = parent->GetChildObjectList()->begin();
    while( iterp != parent->GetChildObjectList()->end() )
    {
        a2dCanvasObject* obj = *iterp;

        if ( obj && obj == wireConnected )
        {
            wireConnected->SetRelease( true ); //Used to detect this event 
            iterp = parent->GetChildObjectList()->erase( iterp );
        }
        else
            iterp++;
    }
    return true;
}

void a2dWirePolylineL::Reverse()
{
    a2dPin* begin = FindBeginPin();
    a2dPin* end = FindEndPin();
    if ( begin )
        begin->SetName( "end" );
    if ( end )
        end->SetName( "begin" );
    m_lsegments->reverse();
}

void a2dWirePolylineL::OnChar( wxKeyEvent& event )
{
    if ( m_flags.m_editingCopy )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_DELETE:
            {
                a2dWirePolylineL* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dWirePolylineL );
                a2dPin* begin = original->FindBeginPin();
                a2dPin* end = original->FindEndPin();


				if ( m_lsegments->size() <= 2 )
				{
					wxLogWarning( _( "stopped removing segment or vertexes, object should keep at least one segment" ) );
                    break; // leaf at least one segment
				}
                //first delete selected segments ( nodes maybe selcted, and those nodes maybe removed because of this. (notice --m_childobjects->end())
                for( a2dCanvasObjectList::iterator iter = ++m_childobjects->begin(); iter != --m_childobjects->end(); ++iter )
                {
                    if ( m_lsegments->size() <= 2 )
                    {
                        wxLogWarning( _( "stopped removing segment or vertexes, object should keep at least one segment" ) );
                        break; // leaf at least one segment
                    }
                    a2dCanvasObject* obj = *iter;

                    a2dPolyHandleL* handle = wxDynamicCast( obj, a2dPolyHandleL );

                    if ( handle &&  !handle->GetRelease( )  )
                    {
                        a2dVertexList::iterator iterpoly = handle->GetNode();
                        a2dLineSegment* segment = *( iterpoly );
                        if ( segment->GetBin() ) //selected?
                        {
                            if ( handle->GetPin() == begin || handle->GetPin() == end )
                            {
                                wxLogWarning( _( "end or begin pins maybe not be deleted this way, select and remove whole wire instead" ) );
                            }
                            else
                            {
                                iter++;
                                a2dPolyHandleL* handle2 = wxDynamicCast( (*iter).Get(), a2dPolyHandleL );
                                iter--;
                                if ( handle2->GetPin() == begin || handle2->GetPin() == end )
                                {
                                    wxLogWarning( _( "end or begin pins maybe not be deleted this way, select and remove whole wire instead" ) );
                                }
                                else
                                {
                                    a2dLineSegment* segmentOrgNext = *( handle2->GetNodeOrg() );
                                    a2dLineSegment* segmentOrg = *( handle->GetNodeOrg() );

                                    double xn,yn;
                                    xn = (handle->GetPosX() + handle2->GetPosX())/2.0;
                                    yn = (handle->GetPosY() + handle2->GetPosY())/2.0;
                                    handle2->SetPosXY( xn, yn );
                                    m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( original, segmentOrg ) );
                                    m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveSegment( original, xn, yn, segmentOrgNext, false ) );
                                    iterpoly = m_lsegments->erase( iterpoly );
                                    (*iterpoly)->m_x = xn; (*iterpoly)->m_y = yn;
                                    ReleaseChild( handle );
                                }
                            }
                        }
                    }
                }
                
                //now delete selected handles (only those which were not released above).
                for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
                {
                    a2dCanvasObject* obj = *iter;

                    a2dPolyHandleL* handle = wxDynamicCast( obj, a2dPolyHandleL );

                    if ( handle &&  !handle->GetRelease( ) && handle->GetSelected() )
                    {
                        if ( handle->GetPin() == begin || handle->GetPin() == end )
                        {
                            wxLogWarning( _( "end or begin pins maybe not be deleted this way, select and remove whole wire instead" ) );
                        }
                        else
                        {
                            a2dVertexList::iterator iterpoly = handle->GetNode();
                            a2dLineSegment* segment = *( iterpoly );
                            a2dVertexList::iterator iterorg = handle->GetNodeOrg();
                            a2dLineSegment* segmentOrg = *( iterorg );

							if ( m_lsegments->size() <= 2 )
							{
								wxLogWarning( _( "stopped removing segment or vertexes, object should keep at least one segment" ) );
						        break; // leaf at least one segment
							}
							else
							{
								m_root->GetCommandProcessor()->Submit( new a2dCommand_RemoveSegment( original, segmentOrg ) );
								m_lsegments->erase( iterpoly );
								ReleaseChild( handle );
							}
                        }
                    }
                }
                original->AdjustAfterChange( true );
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();
}

bool a2dWirePolylineL::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        a2dWirePolylineL* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dWirePolylineL );

        if ( editmode == 1 )
        {
            if( ! ( editstyle & wxEDITSTYLE_NOHANDLES ) )
            {
                a2dPin* begin = original->FindBeginPin();
                a2dPin* end = original->FindEndPin();

                m_flags.m_visiblechilds = true;
                m_flags.m_childrenOnSameLayer = true;

                //add properties and handles as children in order to edit the object.
                a2dVertexList::iterator iter = m_lsegments->begin();
                a2dVertexList::iterator iterorg = original->GetSegments()->begin();
                a2dLineSegment* seg;
                while ( iter != GetSegments()->end() )
                {
                    seg = ( *iter );

					/*
                    if ( ( iter != m_lsegments->begin() && iter != --m_lsegments->end() ) ||
                         ( iter == m_lsegments->begin() && !begin->IsConnectedTo() ) || 
                         ( iter == --m_lsegments->end() && !end->IsConnectedTo() )
                       )
					*/
                    {

                        //not inclusive matrix so relative to polygon
                        a2dPolyHandleL* handle = new a2dPolyHandleL( this, iter, iterorg, m_lsegments, original->GetSegments(), wxT( "__index__" ) );
                        Append( handle );
                        handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                        handle->SetLayer( m_layer );

                        if ( seg->GetArc() )
                        {
                            // todo split in line segments ( maybe under certain conditions of matrix not?? ).
                            a2dArcSegment* cseg = ( a2dArcSegment* ) seg;

                            handle = new a2dPolyHandleL( this, iter, iterorg, m_lsegments, original->GetSegments(), cseg->m_x2, cseg->m_y2, wxT( "__indexarc__" ) );
                            handle->SetArcHandle( true );
                            handle->SetLayer( m_layer );
                            Append( handle );
                            handle->SetSpecificFlags( true, a2dCanvasOFlags::BIN2 );
                        }

						if ( handle->GetPosXY() == begin->GetPosXY() )
							handle->SetPin( begin );
						if ( handle->GetPosXY() == end->GetPosXY() )
							handle->SetPin( end );
						
                    }
                    iter++;
                    iterorg++;
                }

                m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dPolyHandleL" ) );
                m_flags.m_editingCopy = true;

                //calculate bbox's elase mouse events may take place when first idle event is not yet
                //processed to do this calculation.
                Update( updatemask_force );
                //stil set it pending to do the redraw ( in place )
                SetPending( true );
            }
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

#if wxART2D_USE_CVGIO
void a2dWirePolylineL::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dPolylineL::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "invert" ),      m_inverted );
        out.WriteAttribute( wxT( "pcobjbegin" ),  m_objBegin->GetName() );
        out.WriteAttribute( wxT( "pcwirebegin" ), m_wireBegin->GetName() );
        out.WriteAttribute( wxT( "pcwireend" ),   m_wireEnd->GetName() );
    }
    else
    {
    }
}
void a2dWirePolylineL::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dPolylineL::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_inverted = parser.RequireAttributeValueBool( wxT( "invert" ) );

        wxString pinclass;

        pinclass = parser.RequireAttributeValue( wxT( "pcobjbegin" ) );
        m_objBegin = a2dPinClass::GetClassByName( pinclass );
        if( !m_objBegin )
        {
            m_objBegin = a2dPinClass::Standard;
            wxString reference_name = GetName();
            a2dGeneralGlobals->ReportErrorF( a2dError_NoPinClass, _( "a2dWirePolylineL with name %s \n Pin class %s not found (replaced with Standard)" ), reference_name.c_str(), pinclass.c_str() );
        }

        pinclass = parser.RequireAttributeValue( wxT( "pcwirebegin" ) );
        m_wireBegin = a2dPinClass::GetClassByName( pinclass );
        if( !m_wireBegin )
        {
            m_wireBegin = a2dPinClass::Standard;
            wxString reference_name = GetName();
            a2dGeneralGlobals->ReportErrorF( a2dError_NoPinClass, _( "a2dWirePolylineL with name %s \n Pin class %s not found (replaced with Standard)" ), reference_name.c_str(), pinclass.c_str() );
        }

        pinclass = parser.RequireAttributeValue( wxT( "pcwireend" ) );
        m_wireEnd = a2dPinClass::GetClassByName( pinclass );
        if( !m_wireEnd )
        {
            m_wireEnd = a2dPinClass::Standard;
            wxString reference_name = GetName();
            a2dGeneralGlobals->ReportErrorF( a2dError_NoPinClass, _( "a2dWirePolylineL with name %s \n Pin class %s not found (replaced with Standard)" ), reference_name.c_str(), pinclass.c_str() );
        }
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dWirePolylineL::SetEndPoint( int iEnd, int iNext, double x, double y, bool final )
{
    double xPre, yPre;
    GetPosXYSegment( iEnd, xPre, yPre, true );
    m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveSegment( this, x, y, iEnd, true ), final );

    if( GetNumberOfSegments() > 2 )
    {
        double xNext, yNext;
        GetPosXYSegment( iNext, xNext, yNext, true );
        bool isHor = fabs( yPre - yNext ) <= 1e-10 * wxMax( fabs( yPre ), fabs( yNext ) );
        bool isVert = fabs( xPre - xNext ) <= 1e-10 * wxMax( fabs( xPre ), fabs( xNext ) );
        // If the line segment adjacent to the end point is horizontal
        // move the adjacent point only vertically
        if( isHor && ! isVert || isHor && isVert && !m_wasVertical )
        {
            m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveSegment( this, xNext, yNext + y - yPre, iNext, true ), final );
            m_wasVertical = false;
        }
        // If the line segment adjacent to the end point is vertical
        // move the adjacent point only horizontally
        if( isVert && ! isHor || isHor && isVert && m_wasVertical )
        {
            m_root->GetCommandProcessor()->Submit( new a2dCommand_MoveSegment( this, xNext + x - xPre, yNext, iNext, true ), final );
            m_wasVertical = true;
        }
    }
}

bool a2dWirePolylineL::IsDislocated()
{
    return FindBeginPin()->IsDislocated() || FindEndPin()->IsDislocated();
}

bool a2dWirePolylineL::GeneratePinsPossibleConnections( a2dPinClass* pinClass, a2dConnectTask task, double x, double y, double margin )
{
    wxASSERT_MSG( pinClass->GetConnectionGenerator()
		, wxT( "pinclass connection generator not set" ) );

    // m_wireBegin is used normally for creating new wires and pisn on that wire.
    // HERE it is used also for testing pin generation towards this line.
    // For task a2d_GeneratePinsForStartWire, a pin for a new wire is required, and
    // m_wireBegin is used to define/get the connection generator to use.

    if ( task == a2d_GeneratePinsForStartWire )
        //return m_wireBegin->GetConnectionGenerator()->GeneratePossibleConnections( this, m_wireBegin, task, x, y );
        //ask this wire, if it can connect to pinClass based on the m_wireBegin class
        return m_wireBegin->GetConnectionGenerator()->GeneratePossibleConnections( this, pinClass, task, x, y, margin );
    else if ( task == a2d_GeneratePinsForFinishWire )
        //return m_wireEnd->GetConnectionGenerator()->GeneratePossibleConnections( this, m_wireEnd, task, x, y );
        //ask this wire, if it can connect to pinClass based on the m_wireEnd class
        return m_wireEnd->GetConnectionGenerator()->GeneratePossibleConnections( this, pinClass, task, x, y, margin );

    return false;
}

bool a2dWirePolylineL::GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y, double margin )
{
    a2dPinClass* toCreate = toConnectTo->GetPinClassForTask( task, this );
    if ( GetGeneratePins() && ( toCreate == m_wireBegin || toCreate == m_wireEnd ) )
    {
        m_flags.m_visiblechilds = true;
        bool end = false;
        bool begin = false;
        if ( !HasPinNamed( wxT( "begin" ) ) )
        {
            a2dVertexList::iterator iter = m_lsegments->begin();
            assert( m_lsegments->size() );
            AddPin( wxT( "begin" ), ( *iter )->m_x, ( *iter )->m_y, a2dPin::temporary, m_wireBegin );
        }
        else
            begin = true;

        if ( !HasPinNamed( wxT( "end" ) ) )
        {
            a2dVertexList::iterator iter = m_lsegments->end();
            assert( m_lsegments->size() );
            iter--;
            AddPin( wxT( "end" ), ( *iter )->m_x, ( *iter )->m_y, a2dPin::temporary, m_wireEnd );
        }
        else
            end = true;

        if ( /*begin && end &&*/ m_lsegments->size() > 0 )
        {
            // Find the rastered point on the line closest to the polygon line
            double xNear, yNear;
            if( FindNearPoint( &a2dIDENTITY_MATRIX, x, y, &xNear, &yNear ) )
            {
				//We do an accurate hit, to see on what type of segment xNear, yNear is situated.
				//If on vertical or horizontal segment we do a grid snap on that segment.
				//For diagonal lines, we just add a pin at the location of xNear, yNear.
				//This all if the nearest point (xNear, yNear) releative to (x,y), is within the required margin.
                double xNearLocal, yNearLocal;
                a2dHit hit = m_lsegments->HitTestPolyline( a2dPoint2D( xNear, yNear ), a2dACCUR );  
                if ( hit.IsHit() ) //xNear, yNear should be one the segment, but test anyway.
                {
                    if ( hit.m_stroke2 == a2dHit::stroke2_edgevert  )
                    {
                        a2dRestrictionEngine* restrict = GetHabitat()->GetRestrictionEngine();
                        if( restrict )
                        {
                            wxUint32 was = restrict->GetSnapSourceFeatures();
                            restrict->SetSnapSourceFeature( a2dRestrictionEngine::snapToPins );
                            //only modes which are usefull in dragging
                            wxUint32 snapTo =
                                a2dRestrictionEngine::snapToGridPos |
                                a2dRestrictionEngine::snapToGridPosForced;

                            restrict->RestrictPoint( xNear, yNear, snapTo, true );
                            restrict->SetSnapSourceFeatures( was );
                        }
                        double dx = fabs( xNear - x );
                        double dy = fabs( yNear - y );
                        if ( dx > margin )
							xNear = x;
                        a2dAffineMatrix inverse = m_lworld;
                        inverse.Invert();
                        inverse.TransformPoint( xNear, yNear, xNearLocal, yNearLocal );
                    }
                    else if ( hit.m_stroke2 == a2dHit::stroke2_edgehor )
                    {
                        a2dRestrictionEngine* restrict = GetHabitat()->GetRestrictionEngine();
                        if( restrict )
                        {
                            wxUint32 was = restrict->GetSnapSourceFeatures();
                            restrict->SetSnapSourceFeature( a2dRestrictionEngine::snapToPins );
                            //only modes which are usefull in dragging
                            wxUint32 snapTo =
                                a2dRestrictionEngine::snapToGridPos |
                                a2dRestrictionEngine::snapToGridPosForced;

                            restrict->RestrictPoint( xNear, yNear, snapTo, true );
                            restrict->SetSnapSourceFeatures( was );
                        }
                        double dx = fabs( xNear - x );
                        double dy = fabs( yNear - y );
                        if ( dy > margin )
							yNear = y;
                        a2dAffineMatrix inverse = m_lworld;
                        inverse.Invert();
                        inverse.TransformPoint( xNear, yNear, xNearLocal, yNearLocal );
                    }
                    else
                    {
                        a2dAffineMatrix inverse = m_lworld;
                        inverse.Invert();
                        inverse.TransformPoint( xNear, yNear, xNearLocal, yNearLocal );
                        double dx = fabs( xNear - x );
                        double dy = fabs( yNear - y );
                        if ( dx > margin || dy > margin )
                            return false; 
                    }
                }
                else
                {
					//how come??
                    a2dAffineMatrix inverse = m_lworld;
                    inverse.Invert();
                    inverse.TransformPoint( xNear, yNear, xNearLocal, yNearLocal );
                    double dx = fabs( xNear - x );
                    double dy = fabs( yNear - y );
                    if ( dx > margin || dy > margin )
                        return false; 
                }

				//we search for a temporary pin, we prefer to modify its position instead of deleting it, and create a new on.
                bool reuseTempPin = false;
                for( a2dCanvasObjectList::iterator iterpins = m_childobjects->begin(); iterpins != m_childobjects->end(); ++iterpins )
                {
                    a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                    if( !pin || pin->GetRelease() ) continue;
                    if ( pin->IsTemporaryPin() )
                    {
                        pin->SetPosXY( xNearLocal, yNearLocal );
                        pin->SetRelease( false );
                        reuseTempPin = true;
                        //wxLogDebug(wxT("reuse x=%12.6lf, y=%12.6lf"), xNearLocal, yNearLocal );
                    }
                }

                if ( !reuseTempPin )
                {
                    //now we create a dynamic pin of the same class as a connected pin.
                    a2dPin* pin = AddPin( wxT( "dynamic" ), xNearLocal, yNearLocal, a2dPin::temporary | a2dPin::dynamic, toCreate );
                    pin->SetMode( a2dPin::sm_PinCanConnectToPinClass );
                    pin->SetMouseInObject( true );
                    //wxLogDebug(wxT("add x=%12.6lf, y=%12.6lf"), xNearLocal, yNearLocal );
                }
            }
        }

        wxASSERT( HasPins() );
        m_childobjects->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD, wxT( "a2dPin" ) );

        return true;
    }
    return false;
}


void a2dWirePolylineL::OptimizeRerouteWires( a2dCanvasObject* parent, bool removeZero, bool allowredirect, bool undo )
{
    a2dCanvasCommandProcessor* cmdh = GetRoot()->GetCanvasCommandProcessor();

    //first remove all unconnected pins (not begin or end), since that looks better.

    if ( GetRelease() )
		return;

    a2dPin* begin = FindBeginPin();
    a2dPin* end = FindEndPin();
    for( a2dCanvasObjectList::iterator iterpins = GetChildObjectList()->begin(); iterpins != GetChildObjectList()->end(); ++iterpins )
    {
        a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
        if( !pin || pin->GetRelease( ) )
            continue;

        if ( begin != pin && end != pin )
        {
            if( !pin->IsConnectedTo()  )
            {
                if ( undo )
                // cleanup unconnected pins when not at ends
                    cmdh->Submit( new a2dCommand_ReleaseObject( this, pin ) );
                else
                    pin->SetRelease( true ); //it is just an unconnected pin, no complication.
            }
            continue;
        }
    }
 
    if ( removeZero )
    {
        if ( GetSegments()->Length() < a2dACCUR )
        {
            if ( begin->GetConnectedPinsNr() == 1 && end->GetConnectedPinsNr() == 1 )
            {
                    // merge pins into one, and remove this wire.
                if ( undo )
    	            cmdh->Submit(  new a2dCommand_RemoveZeroWire( parent, this ), undo );
                else
                {
                    a2dPin* begin = FindBeginPin();
                    a2dPin* end   = FindEndPin();

                    wxASSERT_MSG( begin->GetConnectedPinsNr() == 1, "Can only be used with one connected pin" );
                    wxASSERT_MSG( end->GetConnectedPinsNr() == 1, "Can only be used with one connected pin" );

                    a2dPinPtr joinPin  = begin->IsConnectedTo();
                    a2dPinPtr joinPin2 = end->IsConnectedTo();

                    joinPin->Disconnect( begin );
                    joinPin2->Disconnect( end );

	                if ( joinPin != joinPin2 ) //like zero to same pin of other object.
		                joinPin->ConnectTo( joinPin2 );

                    // remove the connected wire from parent.
                    a2dCanvasObjectList::iterator iterp = parent->GetChildObjectList()->begin();
                    while( iterp != parent->GetChildObjectList()->end() )
                    {
                        a2dCanvasObject* obj = *iterp;

                        if ( obj && obj == this )
                        {
                            SetRelease( true ); //Used to detect this event 
                            iterp = parent->GetChildObjectList()->erase( iterp );
                        }
                        else
                            iterp++;
                    }
                }
            }
        }
    }

    if ( GetSegments()->Length() > a2dACCUR )
    {
		//For all connected objects, if a wire is only connected once, we can 
		//possibly optimize it.
		// We need to iterate and delete nested (removing pins and wires etc.), 
		// therefore we save the current situation to a list first.
		a2dCanvasObjectList savepins;
		for( a2dCanvasObjectList::iterator iterpins = GetChildObjectList()->begin(); iterpins != GetChildObjectList()->end(); ++iterpins )
		{
			a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
			if( !pin || pin->GetRelease( ) )
				continue;

			if ( pin->GetConnectedPinsNr() != 1 ) //just a single pin connection can be merged
				continue;

			savepins.push_back( pin );
		}

		// with the candiate connection, check the other connected wire 
		for( a2dCanvasObjectList::iterator iterpins = savepins.begin(); iterpins != savepins.end(); ++iterpins )
		{
			a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
			a2dPinList::const_iterator iterconp;

			//we have a one to one pin connection at end or begin pin towards another object.
			//If that is the same on a connected wire, we can merge the lines.

			a2dPinList savepinsother = pin->GetConnectedPins();
			for ( iterconp = savepinsother.begin( ) ; iterconp != savepinsother.end( ) ; iterconp++ )
			{
				a2dPin* otherpin = *iterconp;

				if( !otherpin || otherpin->GetRelease() )
					continue;
				if( !otherpin->IsConnectedTo()  )
					continue;
				if ( otherpin->GetConnectedPinsNr() != 1 )
					continue;

				a2dCanvasObject* connectedobj = otherpin->GetParent();
				a2dWirePolylineL* wireConnected = wxDynamicCast( connectedobj, a2dWirePolylineL );
				if ( wireConnected &&
					 ( wireConnected != this ) && // needs to be a different wire, in order to join.
					 connectedobj->IsConnect() && 
					 !connectedobj->GetBin() && !connectedobj->GetRelease() ) //selected wires are dragged and have bin flags set.
				{
					//If this wire can be combined with the connected wire, take over its pins and segments.
					//The two connected pins will be removed, and the connected wire also.
					SetPending( true ); //redraw the result/changed area.

					a2dPin* beginConnected = wireConnected->FindBeginPin();
					a2dPin* endConnected = wireConnected->FindEndPin();

					//do not join line of different style, unless they have zero lenght.
					if ( wireConnected->GetStroke() != GetStroke() && wireConnected->GetSegments()->Length() > a2dACCUR )
						continue;

					if ( ( begin == pin && otherpin == endConnected ) || //same direction
                         ( allowredirect && end == pin && otherpin == endConnected ) || //wrong direction, redirect
                         ( end == pin && otherpin == beginConnected ) || //same direction
                         ( allowredirect && begin == pin && otherpin == beginConnected ) //wrong direction, redirect
                       ) 

					{
                        if ( undo )
                        {
                            SetSelected( wireConnected->GetSelected() ); //preserve select state of removed wire.
	    					cmdh->Submit( new a2dCommand_JoinAtPin( parent, pin, otherpin ) );
                        }
                        else
                        {
                            a2dSmrtPtr<a2dWirePolylineL> wire = wxStaticCast( pin->GetParent(), a2dWirePolylineL );
                            a2dSmrtPtr<a2dWirePolylineL> wirePin2 = wxStaticCast( otherpin->GetParent(), a2dWirePolylineL );
                            a2dPin* beginConnected = wirePin2->FindBeginPin();
                            a2dPin* endConnected = wirePin2->FindEndPin();
                            bool wheref;
                            bool redirect;
                            MergAtPin( parent, pin, otherpin, wheref, redirect );
                        }
					}

				}
			}
		}
	}

	EliminateMatrix();
}

//----------------------------------------------------------------------------
// a2dWireEnd
//----------------------------------------------------------------------------
a2dWireEnd::a2dWireEnd( double x , double y )
    : a2dCanvasObject( x, y )
{
}

a2dWireEnd::a2dWireEnd( const a2dWireEnd& obj, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( obj, options, refs )
{
}

a2dObject* a2dWireEnd::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dWireEnd( *this, options, refs );
}

//----------------------------------------------------------------------------
// a2dCommand_VirtWire
//----------------------------------------------------------------------------

//! specialized command using the Assign method of Taco to transfer changes in editcopy to original.
class A2DCANVASDLLEXP a2dCommand_VirtWire: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_VirtWire() { m_target = NULL; }
    a2dCommand_VirtWire( a2dVirtWire* target, const wxString& VirtWireName );
    ~a2dCommand_VirtWire( void );

    bool Do();
    bool Undo();

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

protected:
    a2dSmrtPtr<a2dVirtWire> m_target;
    wxString m_VirtWireName;
};

const a2dCommandId a2dCommand_VirtWire::Id( "a2dVirtWire" );

a2dCommand_VirtWire::a2dCommand_VirtWire( a2dVirtWire* target, const wxString& VirtWireName )
    : a2dCommand( true, a2dCommand_VirtWire::Id )
{
    m_target = target;
    m_VirtWireName = VirtWireName;
}

a2dCommand_VirtWire::~a2dCommand_VirtWire( void )
{
}

bool a2dCommand_VirtWire::Do( void )
{
    wxString VirtWireName = m_target->GetVirtWireName();
    m_target->SetVirtWireName( m_VirtWireName );
    m_target->SetPending( true );
    m_VirtWireName = VirtWireName;
    return true;
}

bool a2dCommand_VirtWire::Undo( void )
{
    Do();
    return true;
}

//----------------------------------------------------------------------------
// a2dVirtWire
//----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS( a2dVirtWire, a2dText )

a2dVirtWire::a2dVirtWire( a2dCanvasObject* parent, const wxString& VirtWireName, a2dPinClass* pinclass )
    : a2dText( "", 0.0, 0.0, *a2dDEFAULT_CANVASFONT, 0, false, wxMINX | wxMIDY ),
    m_VirtWireName( VirtWireName ),
    m_parent( parent )
{
    m_flags.m_generatePins = false;
    m_text = " " + VirtWireName;

    a2dPin* pin = new a2dPin( this, wxT( "1" ), a2dPinClass::Standard, 0.0, 0.0 );
    Append( pin );
    pin->SetPinClass( pinclass );
}

a2dVirtWire::a2dVirtWire( const a2dVirtWire &other, CloneOptions options, a2dRefMap* refs )
    : a2dText( other, options, refs ),
    m_parent( other.m_parent )
{
    m_VirtWireName = other.m_VirtWireName;
    m_text = " " + m_VirtWireName;
}

a2dObject* a2dVirtWire::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dVirtWire( *this, options, refs );
}

bool a2dVirtWire::FindConnectedPins( a2dCanvasObjectList& result, a2dPin* pin, bool walkWires, a2dPinClass* searchPinClass, a2dCanvasObject* isConnectedTo )
{
    bool hasConnectedPins = false;
    hasConnectedPins |= a2dText::FindConnectedPins( result, pin, walkWires, searchPinClass, isConnectedTo );

    SetBin( true );
    // find other a2dVirtWire object with the same name as this one, and go on on its pins (one for the moment).
    for( a2dCanvasObjectList::iterator iterp = m_parent->GetChildObjectList()->begin(); iterp != m_parent->GetChildObjectList()->end(); ++iterp )
    {
        a2dCanvasObject* obj = *iterp;
        a2dVirtWire* virtConnect = wxDynamicCast( obj, a2dVirtWire );
        if ( virtConnect && !virtConnect->GetBin() && virtConnect  != this && !virtConnect->GetRelease() && virtConnect->m_VirtWireName == m_VirtWireName )
        {
            for( a2dCanvasObjectList::iterator iterpinssignal = virtConnect->m_childobjects->begin(); iterpinssignal != virtConnect->m_childobjects->end(); ++iterpinssignal )
            {
                a2dCanvasObject* obj = *iterpinssignal;
                if ( !obj )
                    continue;
                a2dPin* pinOtherSignal = wxDynamicCast( obj, a2dPin );
                if ( pinOtherSignal && !pinOtherSignal->GetRelease( ) && !pinOtherSignal->GetBin() )
                {
                    hasConnectedPins |= virtConnect->FindConnectedPins( result, NULL, walkWires, searchPinClass, isConnectedTo );
                }
            }
        }
    }
    return hasConnectedPins;
}

bool a2dVirtWire::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        a2dVirtWire* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dVirtWire );
        original->SetSelected( true );

        a2dVirtWireDlg dlg( NULL, this );
        int result = dlg.ShowModal();
        if (result == wxID_OK)
        {
            m_root->GetCommandProcessor()->Submit( new a2dCommand_VirtWire( original, dlg.GetVirtWire()->GetVirtWireName() ) );
        }
        // we trigger end editing directly, since all editing is finished after closing the dialog.
        original->SetEditing( false );
        return true;
    }
    return false;
}

void a2dVirtWire::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dText::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "virtName" ), m_VirtWireName );
    }
    else
    {
    }
}

void a2dVirtWire::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dText::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_VirtWireName = parser.RequireAttributeValue( wxT( "virtName" ) );
    }
    else
    {
    }
}

#define BBOX2XYWH(bbox) (bbox).GetMinX(), (bbox).GetMinY(), (bbox).GetWidth(), (bbox).GetHeight()

void a2dVirtWire::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    a2dDrawer2D* d(ic.GetDrawer2D());

    d->DrawRoundedRectangle( BBOX2XYWH( m_untransbbox ), 0 );

/*
    a2dVertexList* array = new a2dVertexList();
    array->push_back( new a2dLineSegment( 0.0, 0.0 ) );
    array->push_back( new a2dLineSegment( -1.5, 1.5 ) );
    array->push_back( new a2dLineSegment( -4.0, 1.5 ) );
    array->push_back( new a2dLineSegment( -4.0, -1.5 ) );
    array->push_back( new a2dLineSegment( -1.5, -1.5 ) );
*/
    a2dText::DoRender( ic, clipparent );
}

//----------------------------------------------------------------------------
// a2dVirtDlg
//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE( a2dVirtWireDlg, wxDialog )
wxEND_EVENT_TABLE()

a2dVirtWireDlg::a2dVirtWireDlg( wxWindow *parent, a2dVirtWire* virtWire )
{
    wxDialog::Create( parent, -1, _("Wire-Name"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL );
    m_virtWire = virtWire;
    CreateControls();
    GetSizer()->SetSizeHints(this);
    Centre();
}

void a2dVirtWireDlg::CreateControls()
{
    wxStaticText *st;

    bs = new wxBoxSizer(wxVERTICAL);
    SetSizer(bs);

    // Choose Signal Name
    bsN = new wxBoxSizer(wxHORIZONTAL);
    bs->Add(bsN, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    st = new wxStaticText(this, wxID_ANY, _("Wire-Name") + ":");
    bsN->Add(st, 0, wxALIGN_CENTER_VERTICAL);
    bsN->AddSpacer(5);
  
    wxArrayString names; 
    for( a2dCanvasObjectList::iterator iterp = m_virtWire->GetParent()->GetChildObjectList()->begin(); iterp != m_virtWire->GetParent()->GetChildObjectList()->end(); ++iterp )
    {
        a2dCanvasObject* obj = *iterp;
        a2dVirtWire* virtConnect = wxDynamicCast( obj, a2dVirtWire );
        if ( virtConnect && !virtConnect->GetRelease() )
            if (names.Index( virtConnect->GetVirtWireName() ) == wxNOT_FOUND)
               names.Add( virtConnect->GetVirtWireName() );
    }
    m_CBName = new wxComboBox( this, wxID_ANY, m_virtWire->GetVirtWireName(), wxDefaultPosition, wxSize(130, -1), names, wxCB_DROPDOWN );
    bsN->Add(m_CBName, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Buttons
    wxStdDialogButtonSizer *sdbs = new wxStdDialogButtonSizer();
    bs->Add(sdbs, 0, wxGROW|wxALL, 5);
    wxButton *bOk = new wxButton( this, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    sdbs->AddButton(bOk);
    wxButton *bCancel = new wxButton( this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    sdbs->AddButton(bCancel);
    wxButton *bHelp = new wxButton( this, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
    sdbs->AddButton(bHelp);
    sdbs->Realize();    

    m_CBName->Clear();
    m_CBName->Set(names);
    m_CBName->SetStringSelection( m_virtWire->GetVirtWireName() );

}



#define		DIRECT				0
#define		INVERSE				1

#define		LEFT				0x00
#define		RIGHT				0x01
#define		BOTTOM				0x02
#define		TOP					0x03

#define		EQSIGN(x1, x2, x3, x4)		( ( x2 - x1 ) * ( x3 - x4 ) > 0 )

#define		CODE(x, y, w, h, px, py, c)	{				\
				c = 0x00;								\
				if ( px < x ) c = 0x01;					\
				else if ( px >= x + w ) c = 0x02;		\
				if ( py < y ) c |= 0x04;				\
				else if ( py >= y + h ) c |= 0x08;		\
			}


a2dSingleRoute::a2dSingleRoute( double offset, double smax )
{
	m_offset = offset;//100;
	m_smax   = smax;//400;
}

void a2dSingleRoute::ComputeCon ( a2dPin* outPad, a2dPin* inPad )
{
    int diro = 0;
    double anglerad = wxDegToRad( outPad->GetAbsAngle() );
    if ( cos( anglerad ) > 0  )
    {
        if ( fabs( cos( anglerad ) ) > fabs( sin( anglerad )) )
            diro = RIGHT;
        else if ( sin( anglerad ) > 0 )
            diro = TOP;
        else
            diro = BOTTOM;
    }
    else
    {
        if ( fabs( cos( anglerad ) ) > fabs( sin( anglerad )) )
            diro = LEFT;
        else if ( sin( anglerad ) > 0 )
            diro = TOP;
        else
            diro = BOTTOM;
    }
    int diri = 0;
    anglerad = wxDegToRad( inPad->GetAbsAngle() );
    if ( cos( anglerad ) > 0  )
    {
        if ( fabs( cos( anglerad ) ) > fabs( sin( anglerad )) )
            diri = RIGHT;
        else if ( sin( anglerad ) > 0 )
            diri = TOP;
        else
            diri = BOTTOM;
    }
    else
    {
        if ( fabs( cos( anglerad ) ) > fabs( sin( anglerad )) )
            diri = LEFT;
        else if ( sin( anglerad ) > 0 )
            diri = TOP;
        else
            diri = BOTTOM;
    }

	Connect(outPad->GetAbsX(), outPad->GetAbsY(), diro, inPad->GetAbsX(), inPad->GetAbsY(), diri );

    a2dWirePolylineL* wire = NULL;
    a2dCanvasObject* startObj = outPad->GetParent();
    if ( startObj )
    {
        if ( wxDynamicCast( startObj, a2dWireEnd ) )  
            startObj = a2dCanvasObject::PROPID_Original->GetPropertyValue( startObj );
        wire = wxDynamicCast( startObj, a2dWirePolylineL );
        if ( wire )
	        Correct( outPad->GetAbsX(), outPad->GetAbsY(), 0, 0 );
        else
        {
            a2dBoundingBox bbox = startObj->GetCalculatedBoundingBox( 0 );
	        Correct( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth(), bbox.GetHeight() );
        }
    }

    a2dCanvasObject* endObj = inPad->GetParent();
    if ( endObj )
    {
        if ( wxDynamicCast( endObj, a2dWireEnd ) )  
            endObj = a2dCanvasObject::PROPID_Original->GetPropertyValue( endObj );
        wire = wxDynamicCast( endObj, a2dWirePolylineL );
        if ( wire )
    	    Correct( inPad->GetAbsX(), inPad->GetAbsY(), 0, 0 );
        else
        {
            a2dBoundingBox bbox = endObj->GetCalculatedBoundingBox( 0 );
    	    Correct( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth(), bbox.GetHeight() );
        }
    }

    return;
}

void a2dSingleRoute::ComputeConToWireVertex( a2dPin* outPad, double xw, double yw, double angle )
{
    int diro = 0;
    double anglerad = wxDegToRad( outPad->GetAbsAngle() );
    if ( cos( anglerad ) > 0  )
    {
        if ( fabs( cos( anglerad ) ) > fabs( sin( anglerad )) )
            diro = RIGHT;
        else if ( sin( anglerad ) > 0 )
            diro = TOP;
        else
            diro = BOTTOM;
    }
    else
    {
        if ( fabs( cos( anglerad ) ) > fabs( sin( anglerad )) )
            diro = LEFT;
        else if ( sin( anglerad ) > 0 )
            diro = TOP;
        else
            diro = BOTTOM;
    }
    int diri = 0;
    anglerad = wxDegToRad( angle );
    if ( cos( anglerad ) > 0  )
    {
        if ( fabs( cos( anglerad ) ) > fabs( sin( anglerad )) )
            diri = RIGHT;
        else if ( sin( anglerad ) > 0 )
            diri = TOP;
        else
            diri = BOTTOM;
    }
    else
    {
        if ( fabs( cos( anglerad ) ) > fabs( sin( anglerad )) )
            diri = LEFT;
        else if ( sin( anglerad ) > 0 )
            diri = TOP;
        else
            diri = BOTTOM;
    }

	Connect(outPad->GetAbsX(), outPad->GetAbsY(), diro, xw, yw, diri );

    a2dWirePolylineL* wire = NULL;
    a2dCanvasObject* startObj = outPad->GetParent();
    if ( startObj )
    {
        if ( wxDynamicCast( startObj, a2dWireEnd ) )  
            startObj = a2dCanvasObject::PROPID_Original->GetPropertyValue( startObj );
        wire = wxDynamicCast( startObj, a2dWirePolylineL );
        if ( wire )
	        Correct( outPad->GetAbsX(), outPad->GetAbsY(), 0, 0 );
        else
        {
            a2dBoundingBox bbox = startObj->GetCalculatedBoundingBox( 0 );
	        Correct( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth(), bbox.GetHeight() );
        }
    }
    return;
}

void a2dSingleRoute::Connect ( double xo, double yo, int diro, double xi, double yi, int diri )
{
	static	int	directs[4][4] = {
					{ TOP,    BOTTOM, LEFT,   RIGHT  }, 
					{ BOTTOM, TOP,    RIGHT,  LEFT   }, 
					{ RIGHT,  LEFT,   TOP,    BOTTOM }, 
					{ LEFT,   RIGHT,  BOTTOM, TOP    }
				};
	int	i;
    double txi, tyi;

	Transform(DIRECT, diro, xo, yo, xi, yi, &txi, &tyi);
	switch ( directs[diro][diri] ) {			/* In pad direction	*/
		case LEFT:
			if ( tyi > 0 ) {					/*	T	*/
				if ( txi > 0 ) {				/*	34	*/
					Shape3P(txi, tyi);
				}
				else {							/*	12	*/
					Shape5PB(txi, tyi, -1);
				}
			}
			else {								/*	B	*/
				Shape5PB(txi, tyi, -1);
			}
			break;
		case RIGHT:
			if ( tyi > 0 ) {					/*	T	*/
				if ( txi > 0 ) {				/*	34	*/
					Shape5PB(txi, tyi, 1);
				}
				else {							/*	12	*/
					Shape3P(txi, tyi);
				}
			}
			else {								/*	B	*/
				Shape5PB(txi, tyi, 1);
			}
			break;
		case BOTTOM:
			if ( tyi > 0 ) {					/*	T	*/
				Shape4PD(txi, tyi);
			}
			else {								/*	B	*/
				Shape6P(txi, tyi, -1);
			}
			break;
		case TOP:
			if ( txi < - m_smax || txi > m_smax ) {	/*	14	*/
				Shape4PU(txi, tyi);
			}
			else {								/*	23	*/
				Shape6P(txi, tyi, 1);
			}
			break;
	}
	Optimize();

	for ( i = 0 ; i < m_pnr ; i++ ) 
		Transform(INVERSE, diro, xo, yo, m_p[i][0], m_p[i][1], &m_p[i][0], &m_p[i][1]);
}


/*****************************************************************************
Translates & rotates a point according to the Out pad direction.

Entry:
	mode - transformation mode (DIRECT or INVERSE)
	dout - Out pad direction
	xout - Out pad x coordinate
	yout - Out pad y coordinate
	x, y - point to be transformed
Exit:
	tx, ty - transformed point
*****************************************************************************/

void a2dSingleRoute::Transform ( int mode, int dout, double xout, double yout, double x, double y, double* tx, double* ty )
{
	double ox, oy;

	ox = x;
	oy = y;
	switch ( mode ) {
		case DIRECT:
			switch ( dout ) {
				case LEFT:
					*tx = oy - yout;
					*ty = xout - ox;
					break;
				case RIGHT:
					*tx = yout - oy;
					*ty = ox - xout;
					break;
				case BOTTOM:
					*tx = xout - ox;
					*ty = yout - oy;
					break;
				case TOP:
					*tx = ox - xout;
					*ty = oy - yout;
					break;
			}
			break;
		case INVERSE:
			switch ( dout ) {
				case LEFT:
					*tx = xout - oy;
					*ty = yout + ox;
					break;
				case RIGHT:
					*tx = xout + oy;
					*ty = yout - ox;
					break;
				case BOTTOM:
					*tx = xout - ox;
					*ty = yout - oy;
					break;
				case TOP:
					*tx = xout + ox;
					*ty = yout + oy;
					break;
			}
			break;
	}
}


/*****************************************************************************
Corrects the connection path moving the overcrossing segments out of the
crossed object.

Entry:
	x, y - object lower left corner
	w, h - object width & height
	np   - number of connection point
	p    - original path points list
Exit:
	p - eventually corrected path points list
*****************************************************************************/

void a2dSingleRoute::Correct ( double x, double y, double w, double h )
{
	int	i, j, c;
    double op[2], od[2];
/*
    double m_po[8][2];
    for ( i = 0 ; i < m_pnr ; i++ ) 
	{
		m_po[m_pnr-i-1][0] = m_p[i][0];
		m_po[m_pnr-i-1][1] = m_p[i][1];
    }
    for ( i = 0 ; i < m_pnr ; i++ ) 
	{
		m_p[i][0] = m_po[i][0];
		m_p[i][1] = m_po[i][1];
    }
*/
	op[0] = x;
	op[1] = y;
	od[0] = w;
	od[1] = h;
	for ( j = 1, i = 2 ; i < m_pnr - 1 ; i++, j++ ) 
    {
		if ( Overcross(x, y, w, h, m_p[j][0], m_p[j][1], m_p[i][0], m_p[i][1]) ) 
        {
			c = ( m_p[j][0] == m_p[i][0] ) ? 0 : 1; //horizontal or vertical
            /*
                    ----- Sn
                    | Same direction
              Sp----|

                    ----- Sn
                    |
                    | Different direction
                    ----- Sp
            */
			if ( EQSIGN(m_p[j-1][c], m_p[j][c], m_p[i][c], m_p[i+1][c]) ) 
            {
				if ( m_p[j][c] < m_p[j-1][c] ) 
					m_p[j][c] = m_p[i][c] = op[c] - m_offset + 2;
				else 
                    m_p[j][c] = m_p[i][c] = op[c] + od[c] - 1 + m_offset - 2;
			}
			else 
            {
				if ( m_p[i][c] < m_p[i+1][c] ) 
					m_p[j][c] = m_p[i][c] = op[c] - m_offset + 2;
				else 
                    m_p[j][c] = m_p[i][c] = op[c] + od[c] - 1 + m_offset - 2;
			}
		}
	}
/*
    for ( i = 0 ; i < m_pnr ; i++ ) 
	{
		m_po[m_pnr-i-1][0] = m_p[i][0];
		m_po[m_pnr-i-1][1] = m_p[i][1];
    }
    for ( i = 0 ; i < m_pnr ; i++ ) 
	{
		m_p[i][0] = m_po[i][0];
		m_p[i][1] = m_po[i][1];
    }
*/
}


/*****************************************************************************
Implementation of line-rectangle intersection test as the first test in 
Cohen-Sutherland clipping algorithm.

Entry:
	x, y   - rectangle lower left corner
	w, h   - rectangle width & height
	x1, y1 - line start point
	x2, y2 - line end point
Return:
	TRUE if the line overcrosses the rectangle, FALSE otherwise
*****************************************************************************/

int	a2dSingleRoute::Overcross ( double x, double y, double w, double h,	double x1, double y1, double x2, double y2 )
{
	int	c1, c2;

	CODE(x, y, w, h, x1, y1, c1);
	CODE(x, y, w, h, x2, y2, c2);
	return(! ( c1 & c2));
}


/*****************************************************************************
Shape functions.

Entry:
	xi, yi - In pad coordinate
	[sign] - sign flag
Exit:
	xy - the computed path points list
*****************************************************************************/

void a2dSingleRoute::Shape3P ( double xi, double yi )
{
	m_p[0][0] = 0;
	m_p[0][1] = 0;
	m_p[1][0] = 0;
	m_p[1][1] = yi;
	m_p[2][0] = xi;
	m_p[2][1] = yi; 
	m_pnr = 3;
}


void a2dSingleRoute::Shape4PD ( double xi, double yi )
{
	m_p[0][0] = 0;
	m_p[0][1] = 0;
	if ( m_offset )
	{
		m_p[1][0] = 0;
		m_p[1][1] = m_offset;
		m_p[2][0] = xi;
		m_p[2][1] = m_p[1][1]; 
		m_p[3][0] = xi;
		m_p[3][1] = yi; 
		m_pnr = 4;
	}
	else
	{
		m_p[1][0] = xi;
		m_p[1][1] = 0; 
		m_p[2][0] = xi;
		m_p[2][1] = yi; 
		m_pnr = 3;
	}
}


void a2dSingleRoute::Shape4PU ( double xi, double yi )
{
	m_p[0][0] = 0;
	m_p[0][1] = 0;
	if ( m_offset )
	{
		m_p[1][0] = 0;
		m_p[1][1] = ( ( yi <= 0 ) ? 0 : yi ) + m_offset;
		m_p[2][0] = xi;
		m_p[2][1] = m_p[1][1]; 
		m_p[3][0] = xi;
		m_p[3][1] = yi; 
		m_pnr = 4;
	}
	else
	{
		m_p[1][0] = 0;
		m_p[1][1] = ( yi <= 0 ) ? 0 : yi; 
		m_p[2][0] = xi;
		m_p[2][1] = m_p[1][1]; 
		m_p[3][0] = xi;
		m_p[3][1] = yi; 
		m_pnr = 4;
	}
}


void a2dSingleRoute::Shape5PB ( double xi, double yi, int sign )
{
	m_p[0][0] = 0;
	m_p[0][1] = 0;
	if ( m_offset )
	{
		m_p[1][0] = 0;
		m_p[1][1] = m_offset;
		if ( sign * xi < - m_smax ) 
			m_p[2][0] = xi + sign * m_offset;
		else 
			m_p[2][0] = ( ( sign * xi < 0 ) ? 0 : xi ) + sign * m_offset;
		m_p[2][1] = m_p[1][1]; 
		m_p[3][0] = m_p[2][0];
		m_p[3][1] = yi; 
		m_p[4][0] = xi;
		m_p[4][1] = yi; 
		m_pnr = 5;
	}
	else
	{
		if ( sign * xi < - m_smax ) 
			m_p[1][0] = xi;
		else 
			m_p[1][0] = ( ( sign * xi < 0 ) ? 0 : xi );
		m_p[1][1] = m_p[0][1]; 
		m_p[2][0] = m_p[1][0];
		m_p[2][1] = yi; 
		m_p[3][0] = xi;
		m_p[3][1] = yi; 
		m_pnr = 4;
	}
}


void a2dSingleRoute::Shape5PT ( double xi, double yi, int sign )
{
	m_p[0][0] = 0;
	m_p[0][1] = 0;
	if ( m_offset )
	{
		m_p[1][0] = 0;
		m_p[1][1] = m_offset;
		m_p[2][0] = xi + sign * m_offset;
		m_p[2][1] = m_p[1][1]; 
		m_p[3][0] = m_p[2][0];
		m_p[3][1] = yi; 
		m_p[4][0] = xi;
		m_p[4][1] = yi; 
		m_pnr = 5;
	}
	else
	{
		m_p[1][0] = xi;
		m_p[1][1] = 0; 
		m_p[2][0] = xi;
		m_p[2][1] = yi; 
		m_pnr = 3;
	}
}


void a2dSingleRoute::Shape6P ( double xi, double yi, int sign )
{
	double	xm;

	xm       = xi / 2;
	m_p[0][0] = 0;
	m_p[0][1] = 0;
	if ( m_offset )
	{
		m_p[1][0] = 0;
		m_p[1][1] = m_offset;
		if ( yi < 0 ) 
		{
			if ( xi > 0 ) 
				m_p[2][0] = ( xm > m_smax ) ? xm : m_smax;
			else 
				m_p[2][0] = ( xm < -m_smax ) ? xm : - m_smax;
		}
		else 
		{
			if ( xi > 0 ) 
				m_p[2][0] = ( xm > m_smax ) ? xm : - m_smax;
			else 
				m_p[2][0] = ( xm < -m_smax ) ? xm : m_smax;
		}
		m_p[2][1] = m_p[1][1]; 
		m_p[3][0] = m_p[2][0];
		m_p[3][1] = yi + sign * m_offset;
		m_p[4][0] = xi;
		m_p[4][1] = m_p[3][1]; 
		m_p[5][0] = xi;
		m_p[5][1] = yi; 
		m_pnr = 6;
	}
	else
	{
		if ( yi < 0 ) 
		{
			if ( xi > 0 ) 
				m_p[1][0] = ( xm > m_smax ) ? xm : m_smax;
			else 
				m_p[1][0] = ( xm < -m_smax ) ? xm : - m_smax;
		}
		else 
		{
			if ( xi > 0 ) 
				m_p[1][0] = ( xm > m_smax ) ? xm : - m_smax;
			else 
				m_p[1][0] = ( xm < -m_smax ) ? xm : m_smax;
		}
		m_p[1][1] = 0; 
		m_p[2][0] = m_p[1][0];
		m_p[2][1] = yi;
		m_p[3][0] = xi;
		m_p[3][1] = yi; 
		m_pnr = 4;
	}
}


void a2dSingleRoute::Optimize ()
{
	int i,j;
/*  
    double m_po[8][2];
	j = 0;

	m_po[0][0] = m_p[0][0];
	m_po[0][1] = m_p[0][1];
	for ( i = 1 ; i < m_pnr ; i++ ) 
	{
		if ( m_p[i][0] == m_p[j][0] && m_p[i][1] == m_p[j][1] )
		{
		}
		else
		{
			j++;
			m_po[j][0] = m_p[i][0];
			m_po[j][1] = m_p[i][1];
		}
	}

    m_pnr = j+1;
    for ( i = 0 ; i < m_pnr ; i++ ) 
	{
		m_p[i][0] = m_po[i][0];
		m_p[i][1] = m_po[i][1];
    }
    return;
*/    
   
	j = 0;
	for ( i = 1 ; i < m_pnr ; i++ ) 
	{
		if ( m_p[i][0] == m_p[j][0] && m_p[i][1] == m_p[j][1] )
		    continue;
		j++;
		if ( j != i )
		{
			m_p[j][0] = m_p[i][0];
			m_p[j][1] = m_p[i][1];
		}
	}
	m_pnr = j+1;

	j = 0;
    if ( m_pnr > 3 )
    {
	    for ( i = 1 ; i < m_pnr-1 ; i++ ) 
	    {
		    if ( m_p[i][0] == m_p[j][0] && m_p[i][0] == m_p[i+1][0] || 
                 m_p[i][1] == m_p[j][1] && m_p[i][1] == m_p[i+1][1] )
    		    continue;
		    j++;
		    if ( j != i )
		    {
			    m_p[j][0] = m_p[i][0];
			    m_p[j][1] = m_p[i][1];
		    }
	    }
    }
	m_pnr = j+2;
}
