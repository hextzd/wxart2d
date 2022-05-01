/*! \file canvas/src/canobj.cpp
    \author Robert Roebling and Klaas Holwerda

    Copyright: 2000-2004 (c) Robert Roebling

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

#include "wx/canvas/canobj.h"
#include "wx/canvas/tools.h"
#include "wx/canvas/sttool.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/connectgen.h"
#include "wx/canvas/wire.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"
#include "wx/canvas/cameleon.h"
#include <float.h>

#include <algorithm>

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

//----------------------------------------------------------------------------
// a2dConnectionGenerator
//----------------------------------------------------------------------------

a2dConnectionGenerator::a2dConnectionGenerator()
{
    SetAnyPinClass( a2dPinClass::Standard );

    m_anypinclass = NULL;
    m_returnPinClass = NULL;
    m_returnConnect = NULL;
    a2dWirePolylineL* wirestandard = new a2dWirePolylineL();
    wirestandard->SetStroke( a2dStroke( wxColour( 170, 0, 170 ), 2, a2dSTROKE_SOLID ) );
    wirestandard->SetStartPinClass( a2dPinClass::Standard );
    wirestandard->SetEndPinClass( a2dPinClass::Standard );
    m_tmplObject = wirestandard;
    m_wiresEditCopies = new a2dCanvasObjectList();
    m_wires = new a2dCanvasObjectList();
    m_routeWhenDrag = true;
    //m_routeWhenDrag = false;
    m_routeMethod = StraightEndSegment;//StraightSegment;//GridRouting;
    m_routeMethod = GridRouting;
    m_allowredirect = true;
    m_generatePins = true;
    m_routedata = NULL;
    m_offset = 100;

    m_noEditCopy = false;
    m_withBusyEvent = true;
}

a2dConnectionGenerator::~a2dConnectionGenerator()
{
    delete m_wiresEditCopies;
    delete m_wires;
}

void a2dConnectionGenerator::RotateRouteMethod()
{
    if ( m_routeMethod == StraightEndSegment )
        m_routeMethod = StraightEndSegmentUnlessPins;
    else if ( m_routeMethod == StraightEndSegmentUnlessPins )
        m_routeMethod = StraightSegment;
    else if ( m_routeMethod == StraightSegment )
        m_routeMethod = StraightSegmentUnlessPins;
    else if ( m_routeMethod == StraightSegmentUnlessPins )
        m_routeMethod = ManhattanSegments;
    else if ( m_routeMethod == ManhattanSegments )
        m_routeMethod = ManhattanEndSegments;
    else if ( m_routeMethod == ManhattanEndSegments )
        m_routeMethod = ManhattanEndSegmentsStay;
    else if ( m_routeMethod == ManhattanEndSegmentsStay )
        m_routeMethod = ManhattanEndSegmentsConvertAndStay;
    else if ( m_routeMethod == ManhattanEndSegmentsConvertAndStay )
        m_routeMethod = GridRouting;
    else
        m_routeMethod = StraightEndSegment;
}

wxString a2dConnectionGenerator::GetRouteMethodAsString()
{
    if ( m_routeMethod == StraightEndSegment )
        return "StraightEndSegment";
    else if ( m_routeMethod == StraightEndSegmentUnlessPins )
        return "StraightEndSegmentUnlessPins";
    else if ( m_routeMethod == StraightSegment )
        return "StraightSegment";
    else if ( m_routeMethod == StraightSegmentUnlessPins )
        return "StraightSegmentUnlessPins";
    else if ( m_routeMethod == ManhattanSegments )
        return "ManhattanSegments";
    else if ( m_routeMethod == ManhattanEndSegments )
        return "ManhattanEndSegments";
    else if ( m_routeMethod == ManhattanEndSegmentsStay )
        return "ManhattanEndSegmentsStay";
    else if ( m_routeMethod == ManhattanEndSegmentsConvertAndStay )
        return "ManhattanEndSegmentsConvertAndStay";
    else
        return "GridRouting";
}

void a2dConnectionGenerator::ReRouteNow( a2dCanvasObject* parent, a2dCanvasObjectList& objects )
{
    a2dRefMap refs;
    a2dCanvasObjectList copies; //required for generating wire copies correctly.

	for( a2dCanvasObjectList::iterator iter = objects.begin();
            iter != objects.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;

        a2dCanvasObject* copy = obj->TClone( clone_toDrag | clone_childs | clone_seteditcopy | clone_setoriginal | clone_noCameleonRef, &refs );
        obj->SetBin( true );
        copies.push_back( copy );
        copy->SetSelected( false );
    }

    PrepareForRewire( parent, objects, true, false, false, true, NULL, &refs );

	refs.LinkReferences();

    RerouteWires( true );

    for( a2dCanvasObjectList::iterator iter = copies.begin(); iter != copies.end(); ++iter )
    {
        a2dCanvasObject* copy = *iter;
        a2dWalker_RemovePropertyCandoc setp( a2dCanvasObject::PROPID_Original );
        setp.SetSkipNotRenderedInDrawing( true );
        setp.Start( copy );
    }
    copies.clear();
}

void a2dConnectionGenerator::PrepareForRewire( a2dCanvasObject* parent, a2dCanvasObjectList& dragList, bool walkWires, bool selected, bool stopAtSelectedWire, bool CreateExtraWires, a2dBaseTool* tool, a2dRefMap* refs )
{
    a2dCanvasCommandProcessor* cmdh = parent->GetRoot()->GetCanvasCommandProcessor();
    if ( cmdh )
    {
        if ( m_withBusyEvent )
            cmdh->SendBusyEvent( true );
        else
            cmdh->PushSendBusyEvent( false );
    }
    bool onlyNonSelected = true;

    m_parent = parent;

    //drag is comming so first create wires where there are non.
	if ( CreateExtraWires )
		CreateWiresOnPins( m_parent, dragList, true, onlyNonSelected );

    //find all wires going to this object.
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
    setflags.SetSkipNotRenderedInDrawing( true );
    setflags.Start( m_parent, false );
    // find all wires which do go to the objects, and are to be rerouted.
    a2dCanvasObjectList connectedwires;

    a2dCanvasObjectList::iterator iter;
    for( iter = dragList.begin(); iter != dragList.end(); ++iter )
    {
        a2dCanvasObject* original = *iter;
        original->SetBin( true );
        original->FindConnectedWires( connectedwires, NULL, walkWires, selected, stopAtSelectedWire );
    }

    iter = connectedwires.begin();
    while( iter != connectedwires.end() )
    {
        a2dCanvasObjectList::value_type wire = *iter;
        if ( std::find( dragList.begin(), dragList.end(), wire ) !=  dragList.end() )
            iter = connectedwires.erase( iter );
        else
        {
            wire->SetBin( false );
            wire->EliminateMatrix();
            iter++;
        }
    }
    // All non selected wires found connected to m_objects found!
    AddRerouteWires( &connectedwires, m_parent );
    OptimizeRerouteWires();
    PrepareForRerouteWires( tool, refs );
    setflags.Start( m_parent, false );

    if ( cmdh )
    {
        if ( m_withBusyEvent )
            cmdh->SendBusyEvent( false );
        else
            cmdh->PopSendBusyEvent();
    }
}

void a2dConnectionGenerator::CreateWiresOnPins( a2dCanvasObject* parent, const a2dCanvasObjectList& objectsToDrag, bool undo, bool onlyNonSelected )
{
    a2dCanvasCommandProcessor* cmdh = parent->GetRoot()->GetCanvasCommandProcessor();

    for( a2dCanvasObjectList::const_iterator iterdrags = objectsToDrag.begin(); iterdrags != objectsToDrag.end(); ++iterdrags )
    {
        a2dCanvasObjectPtr original = *iterdrags;

        if ( !original->GetChildObjectsCount() )
            continue;

        if ( original->HasPins() )
        {
            bool done = false;
            forEachIn( a2dCanvasObjectList, original->GetChildObjectList() )
            {
                a2dCanvasObject* obj = *iter;
                a2dPin* pinc = wxDynamicCast( obj, a2dPin );
                if ( pinc && !pinc->GetRelease() )
                {
                    a2dPinList::const_iterator iterconp;
                    for ( iterconp = pinc->GetConnectedPins().begin( ) ; iterconp != pinc->GetConnectedPins().end( ) ; iterconp++ )
                    {
                        a2dPin* other = *iterconp;
                        if ( !other || other->GetRelease() )
                            continue;

                        //if already connected at this pin create a connect if needed.
                        if ( !onlyNonSelected || onlyNonSelected && !other->GetParent()->IsSelected() )
                        {
                            a2dConnectionGenerator* pinConnectGen = pinc->GetPinClass()->GetConnectionGenerator();
                            wxASSERT_MSG( pinConnectGen == other->GetPinClass()->GetConnectionGenerator(), wxT( "incompatible connection generator on connected pins" ) );
                            if ( !pinConnectGen )
                                pinConnectGen = this;

                            a2dCanvasObject* otherobj = other->GetParent();
                            if ( otherobj->IsConnect() )
                            {
                                switch ( m_routeMethod )
                                {
                                case GridRouting:
                                {
                                    a2dWirePolylineL* wire = wxStaticCast( otherobj, a2dWirePolylineL );
                                    // if connected object is already a wire, it can be reused in general.
									// If the end or begin pin is connected to the object to drag, we can reuse.
									// But if both end and begin are connected to the drag objects, we prefer an extra wire also.
                                    if (
											( wire->FindBeginPin() != other && wire->FindEndPin() != other ) ||
											( wire->FindBeginPin()->IsConnectedTo( pinc ) && wire->FindEndPin()->IsConnectedTo( pinc ) )
									   )
                                    {
                                        if ( undo )
                                            cmdh->Submit( new a2dCommand_DisConnectPins( pinc, other ), true );
                                        else
                                            pinc->Disconnect( other );
                                        a2dCanvasObject* newwire = pinConnectGen->CreateConnectObject( parent, pinc, other, undo );
                                        newwire->SetStroke( wire->GetStroke() );
                                    }
                                }
                                break;
                                case StraightSegment:
                                case StraightEndSegment:
                                case StraightEndSegmentUnlessPins:
                                case StraightSegmentUnlessPins:
                                {
                                    a2dWirePolylineL* wire = wxStaticCast( otherobj, a2dWirePolylineL );
                                    // if connected object is already a wire, it can be reused in general.
									// If the end or begin pin is connected to the object to drag, we can reuse.
									// But if both end and begin are connected to the drag objects, we prefer an extra wire also.
                                    if (
											( wire->FindBeginPin() != other && wire->FindEndPin() != other ) ||
											( wire->FindBeginPin()->IsConnectedTo( pinc ) && wire->FindEndPin()->IsConnectedTo( pinc ) )
									   )	
                                    {
                                        if ( undo )
                                            cmdh->Submit( new a2dCommand_DisConnectPins( pinc, other ), true );
                                        else
                                            pinc->Disconnect( other );
                                        a2dCanvasObject* newwire = pinConnectGen->CreateConnectObject( parent, pinc, other, undo );
                                        newwire->SetStroke( wire->GetStroke() );
                                    }
                                    else if ( wire->GetPinCount() > 2 && 
                                              ( m_routeMethod == StraightEndSegmentUnlessPins || m_routeMethod == StraightSegmentUnlessPins )
                                            )
                                    {
                                        a2dVertexList* newpoints = wire->GetSegments();
                                        //its the begin or end pin
                                        // The other pins are somewhere on the wire.
                                        // For a two point wire, just add a segment.
                                        if ( wire->GetNumberOfSegments() <= 2 ) 
                                        {
                                            if( other->GetName() == wxT( "begin" ) )
                                            {
                                                newpoints->push_front( new a2dLineSegment( other->GetAbsXY() ) );
                                            }
                                            else if( other->GetName() == wxT( "end" ) )
                                            {
                                                newpoints->push_back( new a2dLineSegment( other->GetAbsXY() ) );
                                            }
                                            if ( cmdh && undo )
                                            {    
                                                wire->GetRoot()->GetCommandProcessor()->Submit( new a2dCommand_SetSegments( wire, newpoints, true ), true );
                                            }
                                            else
                                                wire->SetSegments( newpoints );
                                        }
                                        // For a more than 2 segment wire, check if pin is on the segment to be rerouted.
                                        else
                                        {
                                            for( a2dCanvasObjectList::iterator iterpins = wire->GetChildObjectList()->begin(); iterpins != wire->GetChildObjectList()->end(); ++iterpins )
                                            {
                                                a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                                                if( !pin ) continue;
                                                if( !pin->IsConnectedTo( pinc ) ) continue;
                                                if( pin->GetName() != wxT( "begin" ) && pin->GetName() != wxT( "end" ) )
                                                {
                                                    int index = wire->FindPinSegmentIndex( pin );
                                                    if ( index == 0 && other->GetName() == wxT( "begin" ) )
                                                        newpoints->push_front( new a2dLineSegment( other->GetAbsXY() ) );
                                                    else if ( index == wire->GetNumberOfSegments()-2 && other->GetName() == wxT( "end" ) )
                                                        newpoints->push_back( new a2dLineSegment( other->GetAbsXY() ) );
                                                    if ( cmdh && undo )
                                                    {    
                                                        wire->GetRoot()->GetCommandProcessor()->Submit( new a2dCommand_SetSegments( wire, newpoints, true ), true );
                                                    }
                                                    else
                                                        wire->SetSegments( newpoints );
                                                }
                                            } 
                                        }
                                    }
                                }
                                break;
                                default:
                                break;
                                }
                            }
                            else
                            {   // a connection on a pin straight to a second object which is not a wire.
                                if ( undo )
                                    cmdh->Submit( new a2dCommand_DisConnectPins( pinc, other ), true );
                                else
                                    pinc->Disconnect( other );

								//sometimes an object gets connected to itself, which is at least disconnected above, no need for wire.
								if ( pinc != other )
								{
									a2dCanvasObjectPtr old = GetConnectObject();
									if ( original->IsConnect() )
									{
										 a2dWirePolylineL* wire = wxDynamicCast( original.Get(), a2dWirePolylineL );
										 a2dStroke preserve = wire->GetStroke();
										 a2dCanvasObject* newwire = pinConnectGen->CreateConnectObject( parent, pinc, other, undo );
										 newwire->SetStroke( wire->GetStroke() );
									}
									else
										pinConnectGen->CreateConnectObject( parent, pinc, other, undo );
								}
                            }
                        }
                    }
                }
            }
        }
    }
}

void a2dConnectionGenerator::OptimizeRerouteWires( bool removeZero )
{
    wxASSERT_MSG( m_parent, wxT( "parent object must be set" ) );

    a2dCanvasCommandProcessor* cmdh = m_parent->GetRoot()->GetCanvasCommandProcessor();

    //first remove all unconnected pins (not begin or end), since that looks better.
    a2dCanvasObjectList::iterator iter;
    for( iter = m_wires->begin(); iter != m_wires->end(); ++iter )
    {
        a2dCanvasObject* connect = *iter;

        if ( !connect->GetRelease() && connect->IsConnect() )
        {
            a2dWirePolylineL* wire = wxStaticCast( connect, a2dWirePolylineL );
            a2dPin* begin = wire->FindBeginPin();
            a2dPin* end = wire->FindEndPin();
            for( a2dCanvasObjectList::iterator iterpins = wire->GetChildObjectList()->begin(); iterpins != wire->GetChildObjectList()->end(); ++iterpins )
            {
                a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                if( !pin || pin->GetRelease( ) )
                    continue;

                if ( begin != pin && end != pin )
                {
                    if( !pin->IsConnectedTo()  )
                    {
                        // cleanup unconnected pins when not at ends
                        cmdh->Submit( new a2dCommand_ReleaseObject( wire, pin ) );
                    }
                    continue;
                }
            }
        }
    }

    if ( removeZero )
    {
        for( iter = m_wires->begin(); iter != m_wires->end(); ++iter )
        {
            a2dCanvasObject* connect = *iter;

            if ( !connect->GetRelease() && connect->IsConnect() )
            {
                a2dWirePolylineL* wire = wxStaticCast( connect, a2dWirePolylineL );
                if ( wire->GetSegments()->Length() < a2dACCUR )
                {
                    a2dPin* begin = wire->FindBeginPin();
                    a2dPin* end   = wire->FindEndPin();
                    if ( begin->GetConnectedPinsNr() == 1 && end->GetConnectedPinsNr() == 1 )
                    	// merge pins into one, and remove this wire.
	                    cmdh->Submit(  new a2dCommand_RemoveZeroWire( m_parent, wire ) );
                }
            }
        }
    }

    for( iter = m_wires->begin(); iter != m_wires->end(); ++iter )
    {
        a2dCanvasObject* connect = *iter;

        if ( !connect->GetRelease() && connect->IsConnect() )
        {
            a2dWirePolylineL* wire = wxStaticCast( connect, a2dWirePolylineL );
            if ( wire->GetSegments()->Length() < a2dACCUR )
                continue;

			wire->OptimizeRerouteWires( m_parent, true, m_allowredirect );
        }
    }

    iter = m_wires->begin();
    while( iter != m_wires->end() )
    {
        a2dWirePolylineL* wire = wxStaticCast( iter->Get(), a2dWirePolylineL );
        if ( !wire->GetRelease() )
        {
            wire->EliminateMatrix();
            iter++;
        }
        else
            iter = m_wires->erase( iter );
    }


    // cleanup all that happened above
    iter = m_wires->begin();
    while( iter != m_wires->end() )
    {
        a2dWirePolylineL* wire = wxStaticCast( iter->Get(), a2dWirePolylineL );
        if ( !wire->GetRelease() )
        {
/*
            a2dPin* begin = wire->FindBeginPin();
            a2dPin* end = wire->FindEndPin();
            a2dCanvasObjectList::iterator iterconpins = wire->GetChildObjectList()->begin();
            while( iterconpins != wire->GetChildObjectList()->end() )
            {
                a2dPin* pinc = wxDynamicCast( iterconpins->Get(), a2dPin );
                if( !pinc || pinc->GetRelease() )
                {
                    // update redraw?? iterconpins = wire->GetChildObjectList()->erase( iterconpins );
                    iterconpins++;
                }
                else if( !pinc->IsConnectedTo() )
                {
                    if ( begin != pinc && end != pinc )
                        // cleanup unconnected pins when not at ends
                        pinc->SetRelease( false );
                    iterconpins++;
                }
                else 
                    iterconpins++;
            }
*/
            wire->EliminateMatrix();
            iter++;
        }
    }
}

void a2dConnectionGenerator::AddRerouteWires( a2dCanvasObjectList* wires, a2dCanvasObject* parentOfWires )
{
    m_parent = parentOfWires;
    m_wires->TakeOverFrom( wires );
}

void a2dConnectionGenerator::AddRerouteWire( a2dCanvasObject* wire, a2dCanvasObject* parentOfWires )
{
    m_parent = parentOfWires;
    if ( wire->IsConnect() )
        m_wires->push_back( wire );
}

bool PrioritySorter ( const a2dCanvasObjectPtr& a, const a2dCanvasObjectPtr& b )
{
    a2dWirePolylineL* aw = wxDynamicCast( a.Get(), a2dWirePolylineL );
    a2dWirePolylineL* bw = wxDynamicCast( b.Get(), a2dWirePolylineL );

    int pa = aw? aw->GetPriority() : 0;
    int pb = bw? bw->GetPriority() : 0;
    return pa < pb;
}


//remove references on pin which do not point to the pin notThis.
void RemoveRefsNotThis( a2dRefMap* refs, a2dPin* pin, a2dPin* notThis )
{
    a2dPinList::const_iterator iterconp;
    for ( iterconp = pin->GetConnectedPins().begin( ) ; iterconp != pin->GetConnectedPins().end( ) ; iterconp++ )
    {
        a2dPin* connectedPin = *iterconp;
        if( !connectedPin )
            continue;
        if ( connectedPin != notThis )
        {
			wxLongLong id = connectedPin->GetId();
			wxASSERT( id != 0 );
			wxString resolveKey;
			resolveKey << id;
            refs->RemoveReference( resolveKey );
        }
    }
}

void a2dConnectionGenerator::PrepareForRerouteWires( a2dBaseTool* tool, a2dRefMap* refs )
{
    wxASSERT_MSG( m_parent, wxT( "parent object must be set" ) );

    if ( m_noEditCopy )
    {
        for( a2dCanvasObjectList::iterator iter = m_wires->begin(); iter != m_wires->end(); ++iter )
        {
            a2dCanvasObject* connect = *iter;
            if ( connect->IsConnect() )
            {
                a2dCanvasObject *wirecopy = connect;
                m_wiresEditCopies->push_back( wirecopy );
                for( a2dCanvasObjectList::iterator iterpins = connect->GetChildObjectList()->begin(); iterpins != connect->GetChildObjectList()->end(); ++iterpins )
                {
                    a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                    if( !pin || pin->GetRelease( ) )
                        continue;
                }

            }
        }
    }
    else
    {
        for( a2dCanvasObjectList::iterator iter = m_wires->begin(); iter != m_wires->end(); ++iter )
        {
            a2dCanvasObject* connect = *iter;

            if ( connect->IsConnect() )
            {
                a2dCanvasObject *wirecopy = connect->TClone( clone_childs | clone_seteditcopy | clone_setoriginal, refs );
                m_wiresEditCopies->push_back( wirecopy );
                // this makes the wire visible as editcopy like objects
                wirecopy->SetSpecificFlags( true, a2dCanvasOFlags::EditingCopy );
                // and this makes sure those editcopy wire are not treated as editable.
                // The main edit copy object to which those wires are connected is of course editable.
                wirecopy->SetEditable( false );

                for( a2dCanvasObjectList::iterator iterpins = connect->GetChildObjectList()->begin(); iterpins != connect->GetChildObjectList()->end(); ++iterpins )
                {
                    a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                    if( !pin || pin->GetRelease( ) )
                        continue;

                    a2dPinList::const_iterator iterconp;
                    for ( iterconp = pin->GetConnectedPins().begin( ) ; iterconp != pin->GetConnectedPins().end( ) ; iterconp++ )
                    {
                        a2dPin* otherpin = *iterconp;
                        if( !otherpin )
                            continue;

                        a2dCanvasObject* connectedobj = otherpin->GetParent();
                        if ( !connectedobj->IsConnect() && !connectedobj->GetBin() ) //objects dragged have bin set.
                        {
                            if ( 1 )
                            {
                                //all object that are not connects and are at the end of (in)directly connected wires
                                // to the first object, are terminated with a special object in order to recognize
                                // the type of connected objects/pins when rerouting.
                                a2dCanvasObject* endWire = new a2dWireEnd( pin->GetPosX(), pin->GetPosY() );
                                a2dPin* endpin = wxStaticCast( otherpin->TClone( clone_setoriginal, refs ) , a2dPin );
                                endpin->SetInternal( true ); //prevents adding it as occupied in Rerouting.
                                endpin->SetPosXY( 0, 0 );
                                endWire->Append( endpin );
                                // above also pins in otherpin which are not connected to pin where added as not resolved.
                                // We remove them here, since the are not part of the wires to reroute, and can not be resolved within that cloned group.
                                RemoveRefsNotThis( refs, otherpin, pin );
                                a2dCanvasObject::PROPID_Original->SetPropertyToObject( endWire, connectedobj );
                                //NOT the following because of filtering in routing initializing, also not needed here.
                                //PROPID_Editcopy->SetPropertyToObject( connectobj, endWire );
                                endWire->SetSpecificFlags( true, a2dCanvasOFlags::EditingCopy );
                                m_wiresEditCopies->push_back( endWire );
                            }
                        }
                    }
                }
            }
        }
    }

    if ( !m_noEditCopy && m_routeWhenDrag && tool )
    {
        for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
        {
            tool->AddEditobject( ( *iter ) );
            ( *iter )->SetVisible( true );
        }
    }
    else
    {
        m_wires->SetSpecificFlags( true, a2dCanvasOFlags::VISIBLE );
    }

    switch ( m_routeMethod )
    {
        case GridRouting:
        {
            PrepareGridRoute();
        }
    }

/*
    switch ( m_routeMethod )
    {
    case GridRouting:
    {
    }
    break;
    case StraightEndSegmentUnlessPins:
    case StraightSegmentUnlessPins:
    case StraightEndSegment:
    {
        for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
        {
            if ( (*iter)->IsConnect() )
            {
                a2dWirePolylineL* wire = wxStaticCast( (*iter).Get(), a2dWirePolylineL );
                for( a2dCanvasObjectList::iterator iterpins = wire->GetChildObjectList()->begin(); iterpins != wire->GetChildObjectList()->end(); ++iterpins )
                {
                    a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                    if( !pin ) continue;
                    if( !pin->ConnectedTo() ) continue;
                    a2dPin* otherPin = pin->ConnectedTo();
                    
                    a2dVertexList* newpoints = wire->GetSegments();
                    if ( (wire->FindBeginPin() != otherPin && wire->FindEndPin() != otherPin )
                         || ( wire->GetPinCount() > 2 && wire->GetNumberOfSegments() <= 2 )
                       )
                    {
                        if( pin->GetName() == wxT( "begin" ) )
                        {
                            newpoints->push_front( new a2dLineSegment( pin->ConnectedTo()->GetAbsXY() ) );
                        }
                        else if( pin->GetName() == wxT( "end" ) )
                        {
                            newpoints->push_back( new a2dLineSegment( pin->ConnectedTo()->GetAbsXY() ) );
                        }
                        else
                        {
                        }
                    }
                    wire->SetSegments( newpoints );
                    
                }
            }
        }
    }
    break;
    default:
    break;
    }
*/
}

void a2dConnectionGenerator::ResetPositionsToOrignals()
{
    wxASSERT_MSG( m_parent, wxT( "parent object must be set" ) );

    for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
    {
        if ( !(*iter)->IsConnect() )
            continue;
        a2dWirePolylineL* wire = wxStaticCast( (*iter).Get(), a2dWirePolylineL );
        if ( wire )
        {
            a2dWirePolylineL* target = wire;

            // Check if there is an original property, that indicates we do want it,
            // because only editcopies have this property.
            a2dWirePolylineL* original = wxStaticCastNull( a2dCanvasObject::PROPID_Original->GetPropertyValue( wire ), a2dWirePolylineL );
            // Only do this for editcopy wires
            if( !original )
                continue;

            // Reassign the original location to the point list of the editcopy
            {
                a2dVertexList* newpoints = new a2dVertexList;
                *newpoints = *original->GetSegments();
                target->SetSegments( newpoints );
            }

            // Reassign the pin positions
            for( a2dCanvasObjectList::iterator iterpins = target->GetChildObjectList()->begin(); iterpins != target->GetChildObjectList()->end(); ++iterpins )
            {
                a2dCanvasObject* obj = *iterpins;
                a2dPin* pin = wxDynamicCast( obj, a2dPin );
                if( !pin ) continue;

                //every pin also has the PROPID_Original property set, to get to the equivalent pin in the original.
                a2dPin* pinori = wxStaticCast( a2dCanvasObject::PROPID_Original->GetPropertyValue( pin ).Get(), a2dPin );

                pin->SetTransformMatrix( pinori->GetTransformMatrix() );
            }

            // One Adjust at the beginning also shouldn't hurt
            // In draging modes this shouldn't do anything, because the wires don't change
            // So this will only have an effect for wire editing tools
            target->AdjustAfterChange( false );
        }
    }
}

void a2dConnectionGenerator::PrepareGridRoute()
{
    wxASSERT_MSG( m_parent, wxT( "parent object must be set" ) );

    s_a2dCanvasObjectSorter = &PrioritySorter;
    m_wiresEditCopies->sort();

    // In phase 1, the routing order of wires is determined
    // First all wires that connect objects with objects are routed
    // A wire that is connected to another wire at end or begin, is routed after that wire.
    // The idea is that wires further away from object get routed latest.
    bool again = true;
    while ( again )
    {    
        again = false;
        for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
        {
            if ( !(*iter)->IsConnect() )
                continue;
            a2dWirePolylineL* wire = wxStaticCast( (*iter).Get(), a2dWirePolylineL );
            if ( wire )
            {
                a2dWirePolylineL* target = wire;

/* They are only edit copies anyway.
                // Check if there is an original property, that indicates we do want it,
                // because only editcopies have this property.
                a2dWirePolylineL* original = wxStaticCastNull( a2dCanvasObject::PROPID_Original->GetPropertyValue( wire ), a2dWirePolylineL );
                // Only do this for editcopy wires
                if( !original )
                    continue;
*/
                int oldpriority = target->GetPriority();
                int priority = 0;

                // Go through all pins and adjust priority to connected wires
                for( a2dCanvasObjectList::iterator iterpins = target->GetChildObjectList()->begin(); iterpins != target->GetChildObjectList()->end(); ++iterpins )
                {
                    a2dCanvasObject* obj = *iterpins;
                    a2dPin* pin = wxDynamicCast( obj, a2dPin );
                    if( !pin || pin->GetRelease() ) continue;

                    // Only begin or end pins
                    if( pin->GetName() != wxT( "begin" ) && pin->GetName() != wxT( "end" ) )
                        continue;

                    a2dPinList::const_iterator iterconp;
                    for ( iterconp = pin->GetConnectedPins().begin( ) ; iterconp != pin->GetConnectedPins().end( ) ; iterconp++ )
                    {
                        a2dPin* otherpin = *iterconp;
                        if( !otherpin )
                            continue;

                        // Only pins connected to a wire can delay routing
                        if( !otherpin->GetParent()->IsConnect() )
                            continue;

                        if( otherpin->GetName() == wxT( "begin" ) || otherpin->GetName() == wxT( "end" ) )
                            continue;

                        a2dWirePolylineL* wireconnected = wxDynamicCast( otherpin->GetParent(), a2dWirePolylineL );
                        int minpriority = 1 + (wireconnected? wireconnected->GetPriority() : 0);
                        if( minpriority > priority && minpriority < 255 ) //limit at some value else sometimes infinite loop
                            priority = minpriority;
                    }
                }

                target->SetPriority( priority );
                target->SetReroute( true );

                if( oldpriority != priority )
                    // redo this, because the priority of other wires could depend on us
                    again = true;
            }
        }
    }
    s_a2dCanvasObjectSorter = &PrioritySorter;
    m_wiresEditCopies->sort();

    //if ( !a2dCanvasGlobals->GetHabitat()->GetRouteOneLine() )
    if ( !m_wiresEditCopies->empty() )
    {
        m_routedata = NULL;
        m_routedata = new a2dRouteData( m_parent, false );
    }
}

void a2dConnectionGenerator::RerouteWire( a2dWirePolylineL* wire, bool& again )
{
    bool oneLine = a2dCanvasGlobals->GetHabitat()->GetRouteOneLine();

    // In phase 3, the wires will be rerouted
    // Check if any pin got dislocated
    // Note: target is zero for dynamically created wires
    if ( wire->GetReroute() )
    {
        wire->SetPriority( 0 );
        wire->SetReroute( false );

        for( a2dCanvasObjectList::iterator iter = wire->GetChildObjectList()->begin(); iter != wire->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pin = wxDynamicCast( obj, a2dPin );
            if( !pin || pin->GetRelease() ) continue;

            a2dPin* otherPin = pin->IsDislocated();
            if( otherPin )
            {
                // only connected pins can be dislocated
                if( pin->IsDynamicPin() )
                {
                    // often a dynamic pin is connected to end of begin of another wire, but with draging selections,
                    // it becomes possible that a dynamic pin is dislocated, but its connected wire is selected and will not be rerouted.
                    // In such a situation an extra wire should have bin created in CreateWiresOnPins()
                    a2dPoint2D otherPos = otherPin->GetAbsXY();
                }
                else
                {
                    if( pin->GetName() == wxT( "begin" ) )
                    {
                        if ( oneLine || wire->GetRouteOneLine() )
                        {
                            a2dPoint2D otherEnd = wire->FindEndPin()->GetAbsXY();
                            a2dVertexList* newpoints = new a2dVertexList;
                            newpoints->push_back( new a2dLineSegment( otherPin->GetAbsXY() ) );
                            newpoints->push_back( new a2dLineSegment( otherEnd ) );
                            // Adjust the existing vertex list to the new vertex list
                            wire->SetSegments( newpoints );
                            wire->SetPending( true );
                            wire->SetRerouteAdded( true );
                        }
                        else
                        {
                            // Reroute wire
                            m_routedata->RerouteWire( wire, pin, wire->FindEndPin(), false, false );
                            // target->SetEndPoint( 0, 1, otherPos.m_x, otherPos.m_y, data->m_final );
                        }

                        // This could dislocates other pins, so repeat routing.
                        // Dynamic pins on the wire are moved towards new segments.
                        if( wire->AdjustAfterChange( false ) )
                            again = true;
                    }
                    else if( pin->GetName() == wxT( "end" ) )
                    {
                        if ( oneLine || wire->GetRouteOneLine() )
                        {
                            a2dPoint2D otherEnd = wire->FindBeginPin()->GetAbsXY();
                            a2dVertexList* newpoints = new a2dVertexList;
                            newpoints->push_back( new a2dLineSegment( otherEnd ) );
                            newpoints->push_back( new a2dLineSegment( otherPin->GetAbsXY() ) );
                            // Adjust the existing vertex list to the new vertex list
                            wire->SetSegments( newpoints );
                            wire->SetPending( true );
                            wire->SetRerouteAdded( true );
                        }
                        else
                        {
                            // Reroute wire
                            m_routedata->RerouteWire( wire, pin, wire->FindBeginPin(), true, false );
                            // wire->SetEndPoint( GetNumberOfSegments()-1, GetNumberOfSegments()-2, otherPos.m_x, otherPos.m_y, data->m_final );
                        }

                        // This could dislocates other dynamic pins also, so repeat routing if so.
                        // Dynamic pins on the wire are moved towards new segments.
                        if( wire->AdjustAfterChange( false ) )
                            again = true;
                    }
                    else
                    {
                        wxASSERT_MSG( 0, wxT( "Unknown pin type in wire" ) );
                    }
                }
            }
        }
        //Rerouting was done, dynamic pins on wire were moved with it.
        //Pins on wires connected to those dynamic pins are now dislocated,
        // and will be rerouted.

/*      try removing segments that are on 
        for( a2dCanvasObjectList::iterator iter = wire->GetChildObjectList()->begin(); iter != wire->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pin = wxDynamicCast( obj, a2dPin );
            if( !pin || pin->GetRelease() ) continue;

            if( pin->GetName() == wxT( "begin" ) )
            {
                a2dPinList::const_iterator iterconp;
                for ( iterconp = pin->GetConnectedPins().begin( ) ; iterconp != pin->GetConnectedPins().end( ) ; iterconp++ )
                {
                    a2dPin* otherPin = *iterconp;
                    if( !otherPin || otherPin->GetRelease() )
                        continue;

                    if( otherPin->IsDynamicPin() && otherPin->GetParent()->IsConnect() )
                    {
                        a2dWirePolylineL* wireconnected = wxStaticCast( otherPin->GetParent(), a2dWirePolylineL );
                        if ( wireconnected )
                        {
                            a2dPoint2D beginPos = pin->GetAbsXY();
                            a2dHit result = wireconnected->GetSegments()->HitTestPolyline( beginPos, a2dCanvasGlobals->GetHabitat()->GetCoordinateEpsilon() );
                            while( result.IsHit() && wire->GetSegments()->size() > 2 )
                            {
                                //remove begin segment on wire, and move pin there.
                                if ( wire->GetFirstPoint()->GetPoint() == beginPos )
                                {
                                    wire->GetSegments()->pop_front();
                                    beginPos = wire->GetFirstPoint()->GetPoint();
                                }
                                else
                                {
                                    wire->GetSegments()->pop_back();
                                    beginPos = wire->GetLastPoint()->GetPoint();
                                }
                                result = wireconnected->GetSegments()->HitTestPolyline( beginPos, a2dCanvasGlobals->GetHabitat()->GetCoordinateEpsilon() );
                            }
                            pin->SetPosXY( beginPos.m_x, beginPos.m_y ); 
                        }

                        wire->SetPending( true );
                        wire->SetRerouteAdded( true );
                        // This could dislocates other pins, so repeat routing.
                        // Dynamic pins on the wire are moved towards new segments.
                        if( wire->AdjustAfterChange( false ) )
                            again = true;

                    }
                }
            }
        }
*/
    }
}

void a2dConnectionGenerator::AutoConnect( a2dWirePolylineL* wire, bool allowreconnectbegin, bool allowreconnectend )
{
    for( a2dCanvasObjectList::iterator iter = wire->GetChildObjectList()->begin(); iter != wire->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        a2dPin* pin = wxDynamicCast( obj, a2dPin );
        if( !pin ) continue;

        if( pin->GetName() == wxT( "begin" ) )
        {
            if( allowreconnectbegin )
            {
                pin->AutoConnect( m_parent, 1.0 );
            }
        }
        else if( pin->GetName() == wxT( "end" ) )
        {
            if( allowreconnectend )
            {
                pin->AutoConnect( m_parent, 1.0 );
            }
        }
    }
}

void a2dConnectionGenerator::RerouteWires( bool final, bool fromOriginal )
{
    if ( !m_wiresEditCopies->size() )
        return;
    a2dCanvasCommandProcessor* cmdh = m_parent->GetRoot()->GetCanvasCommandProcessor();
    if ( cmdh )
    {
        if ( m_withBusyEvent )
            cmdh->SendBusyEvent( true );
        else
            cmdh->PushSendBusyEvent( false );
    }

    wxASSERT_MSG( m_parent, wxT( "parent object must be set" ) );
    double offset = m_offset;

    if ( m_routeWhenDrag || ( !m_routeWhenDrag && final ) )
    {
        switch ( m_routeMethod )
        {
        case StraightSegment:
        case StraightSegmentUnlessPins:
        {
            // keep re-adjusting wires, until no more pin is dislocated 
            bool displaced = true;
            int i = 0; 
            while ( displaced && i < 1000 )
            {
                i++;
                displaced = false; // unless more displaced pins are found.
                for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
                {
                    if ( !(*iter)->IsConnect() )
                        continue;
                    a2dWirePolylineL* wire = wxStaticCast( (*iter).Get(), a2dWirePolylineL );
                    for( a2dCanvasObjectList::iterator iterpins = wire->GetChildObjectList()->begin(); iterpins != wire->GetChildObjectList()->end(); ++iterpins )
                    {
                        a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                        if( !pin || pin->GetRelease() ) continue;

                        // if a pin on a dragged object is not the same position as wire pin, the wire needs rerouting.
                        // Pins on the wires are adjusted to the new situation. And only when final the editcopy wires are transfered to the originals.
                        a2dPin* otherPin = pin->IsDislocated();
                        if( otherPin )
                        {
                            if( pin->IsDynamicPin() )
                            {
                                a2dPoint2D otherPos = otherPin->GetAbsXY();
                                if( wire->MoveDynamicPinCloseTo( pin, otherPos, false ) )
                                {
                                    displaced = true;
                                }
                            }
                            else
                            {
                                a2dVertexList* newpoints = wire->GetSegments();
                                if ( m_routeMethod != StraightSegmentUnlessPins )//wire->GetNumberOfSegments() == 2 )
                                {
                                    newpoints = new a2dVertexList;
                                    if( pin->GetName() == wxT( "begin" ) )
                                    {
                                        a2dPoint2D otherEnd = wire->FindEndPin()->GetAbsXY();
                                        newpoints->push_back( new a2dLineSegment( otherPin->GetAbsXY() ) );
                                        newpoints->push_back( new a2dLineSegment( otherEnd ) );
                                    }
                                    else if( pin->GetName() == wxT( "end" ) )
                                    {
                                        a2dPoint2D otherEnd = wire->FindBeginPin()->GetAbsXY();
                                        newpoints->push_back( new a2dLineSegment( otherEnd ) );
                                        newpoints->push_back( new a2dLineSegment( otherPin->GetAbsXY() ) );
                                    }
                                    else
                                    {
                                    }
                                }
                                else
                                {
                                    {
                                        if( pin->GetName() == wxT( "begin" ) )
                                        {
                                            a2dLineSegment* seg = newpoints->front();
                                            seg->SetPoint( otherPin->GetAbsXY() );
                                        }
                                        else if( pin->GetName() == wxT( "end" ) )
                                        {
                                            a2dLineSegment* seg = newpoints->back();
                                            seg->SetPoint( otherPin->GetAbsXY() );
                                        }
                                        else
                                        {
                                            //newpoints->push_back( new a2dLineSegment( pin->ConnectedTo()->GetAbsXY() ) );
                                        }
                                    }
                                }


                                // Adjust the existing vertex list to the new vertex list
                                wire->SetSegments( newpoints );
                                if( wire->AdjustAfterChange( false ) )
                                    displaced = true;
                                wire->SetPending( true );
                            }
                        }
                    }
                }
            }
        }
        break;
        case ManhattanSegments:
        {
            // keep re-adjusting wires, until no more pin is dislocated 
            bool displaced = true;
            int i = 0; 
            while ( displaced && i < 1000 )
            {
                i++;
                displaced = false; // unless more displaced pins are found.
                for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
                {
                    if ( !(*iter)->IsConnect() )
                        continue;
                    a2dWirePolylineL* wire = wxStaticCast( (*iter).Get(), a2dWirePolylineL );
                    for( a2dCanvasObjectList::iterator iterpins = wire->GetChildObjectList()->begin(); iterpins != wire->GetChildObjectList()->end(); ++iterpins )
                    {
                        a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                        if( !pin || pin->GetRelease() ) continue;

                        // if a pin on a dragged object is not the same position as wire pin, the wire needs rerouting.
                        // Pins on the wires are adjusted to the new situation. And only when final the editcopy wires are transfered to the originals.
                        a2dPin* otherPin = pin->IsDislocated();
                        if( otherPin )
                        {
                            if( pin->IsDynamicPin() )
                            {
                                a2dPoint2D otherPos = otherPin->GetAbsXY();
                                if( wire->MoveDynamicPinCloseTo( pin, otherPos, false ) )
                                {
                                    displaced = true;
                                }
                            }
                            else
                            {
                                a2dVertexList* newpoints = wire->GetSegments();
                                newpoints = new a2dVertexList;
                                if( pin->GetName() == wxT( "begin" ) )
                                {
                                    a2dSingleRoute routeWire( offset, 400 );
                                    a2dPin* otherEndpin = wire->FindEndPin()->IsConnectedTo();
                                    if ( !otherEndpin ) 
                                    {   
    				                    wire->AdjustBeginEndPinAngle();
                                        otherEndpin = wire->FindEndPin();
                                        routeWire.ComputeConToWireVertex( otherPin, otherEndpin->GetAbsX(), otherEndpin->GetAbsY(), otherEndpin->GetAbsAngle() - 180 );    
                                    }
                                    else
                                        routeWire.ComputeCon ( otherPin, otherEndpin );

                                    for ( int v = 0; v < routeWire.m_pnr; v++ )
                                    {
                                        newpoints->push_back( new a2dLineSegment( routeWire.m_p[v][0], routeWire.m_p[v][1] ) );
                                    }
                                    offset += m_offset;
                                }
                                else if( pin->GetName() == wxT( "end" ) )
                                {
                                    a2dSingleRoute routeWire( offset, 400 );
                                    a2dPin* otherEndpin = wire->FindBeginPin()->IsConnectedTo();
                                    if ( !otherEndpin ) 
                                    {   
    				                    wire->AdjustBeginEndPinAngle();
                                        otherEndpin = wire->FindBeginPin();
                                        routeWire.ComputeConToWireVertex( otherPin, otherEndpin->GetAbsX(), otherEndpin->GetAbsY(), otherEndpin->GetAbsAngle() - 180 );    
                                    }
                                    else
                                        routeWire.ComputeCon ( otherPin, otherEndpin );

                                    for ( int v = 0; v < routeWire.m_pnr; v++ )
                                    {
                                        newpoints->push_front( new a2dLineSegment( routeWire.m_p[v][0], routeWire.m_p[v][1] ) );
                                    }
                                    offset += m_offset;
                                }
                                else
                                {
                                }
                                // Adjust the existing vertex list to the new vertex list
                                wire->SetSegments( newpoints );
                                if( wire->AdjustAfterChange( false ) )
                                    displaced = true;
                                wire->SetPending( true );
                            }
                        }
                    }
                }
            }
        }
        break;
        case ManhattanEndSegments:
        {
            bool displaced = true;
            unsigned int trials = 0;
            while ( displaced && trials < 10 )
            {
                trials++;
                displaced = false; // unless more displaced pins are found.
                for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
                {
                    a2dCanvasObject* obj = *iter;
                    if ( !obj->IsConnect() )
                        continue;
                    a2dWirePolylineL* wire = wxDynamicCast( obj, a2dWirePolylineL );

                    for( a2dCanvasObjectList::iterator iterpins = wire->GetChildObjectList()->begin(); iterpins != wire->GetChildObjectList()->end(); ++iterpins )
                    {
                        a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                        if( !pin || pin->GetRelease() ) continue;

                        // if a pin on a dragged object is not the same position as wire pin, the wire needs rerouting.
                        // Pins on the wires are adjusted to the new situation. And only when final the editcopy wires are transfered to the originals.
                        a2dPin* otherPin = pin->IsDislocated();
                        if( otherPin )
                        {
                            bool hasbins = false;
                            a2dVertexList* newpoints = wire->GetSegments();
                            a2dVertexList::iterator iter = newpoints->begin();
                            while ( iter != newpoints->end() )
                            {
                                if ( (*iter)->GetBin() && newpoints->size() > 1 )
                                {
                                    iter = newpoints->erase( iter );
                                    hasbins = true;
                                }
                                else
                                    iter++;
                            }
                            wire->AdjustAfterChange( false );

                            if( pin->IsDynamicPin() )
                            {
                                a2dPoint2D otherPos = otherPin->GetAbsXY();
                                if( wire->MoveDynamicPinCloseTo( pin, otherPos, false ) )
                                {
                                    displaced = true;
                                }
                            }
                            else
                            {
                                if( pin->GetName() == wxT( "begin" ) )
                                {
                                    if ( !hasbins )
                                    {
                                        if ( newpoints->size() > 1 )
                                            newpoints->pop_front();
                                        if ( newpoints->size() > 1 )
                                            newpoints->pop_front();
                                        wire->AdjustAfterChange( false );
                                    }
				                    wire->AdjustBeginEndPinAngle();

                                    if ( newpoints->size() <= 2 )
                                    {
                                        newpoints->clear();
                                        a2dPin* otherEndpin = wire->FindEndPin()->IsConnectedTo();
								        a2dSingleRoute routeWire( offset, 400 );
                                        if ( !otherEndpin ) 
                                        {   
                                            otherEndpin = wire->FindEndPin();
                                            routeWire.ComputeConToWireVertex( otherPin, otherEndpin->GetAbsX(), otherEndpin->GetAbsY(), otherEndpin->GetAbsAngle() - 180 );    
                                        }
                                        else
                                            routeWire.ComputeCon ( otherPin, otherEndpin );

                                        for ( int v = 0; v < routeWire.m_pnr; v++ )
                                        {
                                            a2dLineSegment* seg = new a2dLineSegment( routeWire.m_p[v][0], routeWire.m_p[v][1] );
                                            seg->SetBin( true );
                                            newpoints->push_front( seg );
                                        }
                                    }
                                    else
                                    {
								        a2dSingleRoute routeWire( 0, 400 );
                                        routeWire.ComputeCon ( pin, otherPin );

                                        for ( int v = 1; v < routeWire.m_pnr; v++ )
                                        {
                                            a2dLineSegment* seg = new a2dLineSegment( routeWire.m_p[v][0], routeWire.m_p[v][1] );
                                            seg->SetBin( true );
                                            newpoints->push_front( seg );
                                        }
                                    }
                                    offset += m_offset;
                                }
                                else if( pin->GetName() == wxT( "end" ) )
                                {
                                    if ( !hasbins )
                                    {
                                        if ( newpoints->size() > 1 )
                                            newpoints->pop_back();
                                        if ( newpoints->size() > 1 )
                                            newpoints->pop_back();
                                        wire->AdjustAfterChange( false );
                                    }
				                    wire->AdjustBeginEndPinAngle();

                                    if ( newpoints->size() <= 2 )
                                    {
                                        newpoints->clear();
                                        a2dPin* otherEndpin = wire->FindBeginPin()->IsConnectedTo();
								        a2dSingleRoute routeWire( offset, 400 );
                                        if ( !otherEndpin ) 
                                        {   
                                            otherEndpin = wire->FindBeginPin();
                                            routeWire.ComputeConToWireVertex( otherPin, otherEndpin->GetAbsX(), otherEndpin->GetAbsY(), otherEndpin->GetAbsAngle() - 180 );    
                                        }
                                        else
                                            routeWire.ComputeCon ( otherPin, otherEndpin );

                                        for ( int v = 0; v < routeWire.m_pnr; v++ )
                                        {
                                            a2dLineSegment* seg = new a2dLineSegment( routeWire.m_p[v][0], routeWire.m_p[v][1] );
                                            seg->SetBin( true );
                                            newpoints->push_front( seg );
                                        }
                                    }
                                    else
                                    {
    								    a2dSingleRoute routeWire( 0, 400 );
                                        routeWire.ComputeCon ( pin, otherPin );

                                        for ( int v = 1; v < routeWire.m_pnr; v++ )
                                        {
                                            a2dLineSegment* seg = new a2dLineSegment( routeWire.m_p[v][0], routeWire.m_p[v][1] );
                                            seg->SetBin( true );
                                            newpoints->push_back( seg );
                                        }
                                    }
                                    offset += m_offset;
                                }
                                else
                                {
                                }
                                // Adjust the existing vertex list to the new vertex list
                                wire->SetSegments( newpoints );
                                if( wire->AdjustAfterChange( false ) )
                                    displaced = true;
                                wire->SetPending( true );
                            }
                        }
                    }
                }
            }
        }
        break;
        case ManhattanEndSegmentsStay:
        case ManhattanEndSegmentsConvertAndStay:
        {
            bool displaced = true;
            unsigned int trials = 0;
            while ( displaced && trials < 10 )
            {
                trials++;
                displaced = false; // unless more displaced pins are found.
                for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
                {
                    a2dCanvasObject* obj = *iter;
                    if ( !obj->IsConnect() )
                        continue;
                    a2dWirePolylineL* wire = wxDynamicCast( obj, a2dWirePolylineL );

                    for( a2dCanvasObjectList::iterator iterpins = wire->GetChildObjectList()->begin(); iterpins != wire->GetChildObjectList()->end(); ++iterpins )
                    {
                        a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                        if( !pin || pin->GetRelease() ) continue;

                        // if a pin on a dragged object is not the same position as wire pin, the wire needs rerouting.
                        // Pins on the wires are adjusted to the new situation. And only when final the editcopy wires are transfered to the originals.
                        a2dPin* otherPin = pin->IsDislocated();
                        if( otherPin )
                        {
                            a2dVertexList* newpoints = wire->GetSegments();

                            if( pin->IsDynamicPin() )
                            {
                                a2dPoint2D otherPos = otherPin->GetAbsXY();
                                if( wire->MoveDynamicPinCloseTo( pin, otherPos, false ) )
                                {
                                    displaced = true;
                                }
                            }
                            else
                            {
                                if( pin->GetName() == wxT( "begin" ) )
                                {
                                    a2dPin* otherEndpin = wire->FindEndPin()->IsConnectedTo();
                                    if ( !otherEndpin ) 
                                    {   
                                        a2dLineSegment* seg = newpoints->front();
                                        seg->SetPoint( otherPin->GetAbsXY() );
                                    }
                                    else if ( newpoints->size() > 2 )
                                    {
                                        a2dVertexList::iterator iter = newpoints->begin();
                                        a2dLineSegment* seg, *seg2, *seg3;
                                        if ( iter != newpoints->end() )
                                            seg = *iter;
                                        iter++;
                                        if ( iter != newpoints->end() )
                                            seg2 = *iter;
                                        iter++;
                                        if ( iter != newpoints->end() )
                                            seg3 = *iter;

                                        EndManhattanRoute( otherPin, seg, seg2, seg3, newpoints, false );
                                    }
                                    else
                                    {   
                                        a2dVertexList::iterator iter = newpoints->begin();
                                        a2dLineSegment* seg, *seg2;
                                        seg = seg2 = NULL;
                                        if ( iter != newpoints->end() )
                                            seg = *iter;
                                        iter++;
                                        if ( iter != newpoints->end() )
                                            seg2 = *iter;
                                        if ( seg2 && m_routeMethod == ManhattanEndSegmentsConvertAndStay )
                                        {   //convert straight endsegment to a manhattan pair of segments
                                            newpoints->push_front( new a2dLineSegment( otherPin->GetAbsXY() ) );
                                            seg->SetPoint( a2dPoint2D( seg2->m_x, otherPin->GetAbsY() ) );
                                        }
                                        else //if ( m_routeMethod == ManhattanEndSegmentsStay )
                                        {
                                            seg->SetPoint( otherPin->GetAbsXY() );
                                        }
                                    }
                                }
                                else if( pin->GetName() == wxT( "end" ) )
                                {
                                    a2dPin* otherEndpin = wire->FindBeginPin()->IsConnectedTo();
                                    if ( !otherEndpin ) 
                                    {   
                                        a2dLineSegment* seg = newpoints->back();
                                        seg->SetPoint( otherPin->GetAbsXY() );
                                    }
                                    else if ( newpoints->size() > 2 )
                                    {
                                        a2dVertexList::reverse_iterator iter = newpoints->rbegin();
                                        a2dLineSegment* seg, *seg2, *seg3;
                                        if ( iter != newpoints->rend() )
                                            seg = *iter;
                                        iter++;
                                        if ( iter != newpoints->rend() )
                                            seg2 = *iter;
                                        iter++;
                                        if ( iter != newpoints->rend() )
                                            seg3 = *iter;

                                        EndManhattanRoute( otherPin, seg, seg2, seg3, newpoints, true );
                                    }
                                    else
                                    {   
                                        a2dVertexList::reverse_iterator iter = newpoints->rbegin();
                                        a2dLineSegment* seg, *seg2;
                                        seg = seg2 = NULL;
                                        if ( iter != newpoints->rend() )
                                            seg = *iter;
                                        iter++;
                                        if ( iter != newpoints->rend() )
                                            seg2 = *iter;
                                        if ( seg2 && m_routeMethod == ManhattanEndSegmentsConvertAndStay )
                                        {   //convert straight endsegment to a manhattan pair of segments
                                            newpoints->push_back( new a2dLineSegment( otherPin->GetAbsXY() ) );
                                            seg->SetPoint( a2dPoint2D( seg2->m_x, otherPin->GetAbsY() ) );
                                        }
                                        else //if ( m_routeMethod == ManhattanEndSegmentsStay )
                                        {
                                            seg->SetPoint( otherPin->GetAbsXY() );
                                        }
                                    }
                                }
                                else
                                {
                                }
                                wire->Remove180DegPoints( NULL, false );
                                // Adjust the existing vertex list to the new vertex list
                                wire->SetSegments( newpoints );
                                if( wire->AdjustAfterChange( false ) )
                                    displaced = true;
                                wire->SetPending( true );
                            }
                        }
                    }
                }
            }
        }
        break;
        case GridRouting:
        {
            if ( fromOriginal )
                ResetPositionsToOrignals();
            PrepareGridRoute();

            bool again = true;
            while ( again )
            {    
                again = false;
                for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
                {
                    if ( !(*iter)->IsConnect() )
                        continue;
                    a2dWirePolylineL* wire = wxStaticCast( (*iter).Get(), a2dWirePolylineL );
                    if ( wire )
                    {

/* No need to do this, since we have only edit copy wires.
                        // Check if there is an original property, that indicates we do want it,
                        // because only editcopies have this property.
                        a2dWirePolylineL* original = wxStaticCastNull( a2dCanvasObject::PROPID_Original->GetPropertyValue( wire ), a2dWirePolylineL );
                        a2dWirePolylineL* target = wire;

                        // Only do this for editcopy wires
                        if( !original )
                            continue;
*/                        
                        RerouteWire( wire, again );
                    }
                }
            }
            if ( final )
                m_routedata = NULL;
        }
        break;
        case StraightEndSegment:
        case StraightEndSegmentUnlessPins:
        {
            bool displaced = true;
            unsigned int trials = 0;
            while ( displaced && trials < 10000 )
            {
                trials++;
                displaced = false; // unless more displaced pins are found.
                for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
                {
                    a2dCanvasObject* obj = *iter;
                    if ( !obj->IsConnect() )
                        continue;
                    a2dWirePolylineL* wire = wxDynamicCast( obj, a2dWirePolylineL );

                    for( a2dCanvasObjectList::iterator iterpins = wire->GetChildObjectList()->begin(); iterpins != wire->GetChildObjectList()->end(); ++iterpins )
                    {
                        a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                        if( !pin || pin->GetRelease() ) continue;

                        // if a pin on a dragged object is not the same position as wire pin, the wire needs rerouting.
                        // Pins on the wires are adjusted to the new situation. And only when final the editcopy wires are transfered to the originals.
                        a2dPin* otherPin = pin->IsDislocated();
                        if( otherPin )
                        {
                            if( pin->IsDynamicPin() )
                            {
                                a2dPoint2D otherPos = otherPin->GetAbsXY();
                                if( wire->MoveDynamicPinCloseTo( pin, otherPos, false ) )
                                {
                                    displaced = true;
                                }
                            }
                            else
                            {
                                a2dVertexList* newpoints = wire->GetSegments();
                                {
                                    if( pin->GetName() == wxT( "begin" ) )
                                    {
                                        a2dLineSegment* seg = newpoints->front();
                                        seg->SetPoint( otherPin->GetAbsXY() );
                                    }
                                    else if( pin->GetName() == wxT( "end" ) )
                                    {
                                        a2dLineSegment* seg = newpoints->back();
                                        seg->SetPoint( otherPin->GetAbsXY() );
                                    }
                                    else
                                    {
                                        //newpoints->push_back( new a2dLineSegment( pin->ConnectedTo()->GetAbsXY() ) );
                                    }
                                }

                                wire->SetSegments( newpoints );
                                if( wire->AdjustAfterChange( false ) )
                                    displaced = true;
                                wire->SetPending( true );
                            }
                        }
                    }
                }
            }
        }
        break;
        default:
        break;
        }

    }

    // but all information back to the originals.
    if ( final )
    {
/* Check for dislocated pins in editcopies.
        for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
        {
            a2dWirePolylineL* wire = wxStaticCast( (*iter).Get(), a2dWirePolylineL );
            for( a2dCanvasObjectList::iterator iterpins = wire->GetChildObjectList()->begin(); iterpins != wire->GetChildObjectList()->end(); ++iterpins )
            {
                a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                if( !pin ) continue;
                if( !pin->ConnectedTo() ) continue;

                // if a pin on a dragged object is not the same position as wire pin, the wire needs rerouting.
                // Pins on the wires are adjusted to the new situation. And only when final the editcopy wires are transfered to the originals.
                if( pin->IsDislocated() )
                {                            
                    wxLogDebug(wxT("route x=%12.6lf, y=%12.6lf"), pin->GetAbsX() , pin->GetAbsY() );
                    wxLogDebug(wxT("      xo=%12.6lf, y=o%12.6lf"), pin->ConnectedTo()->GetAbsX() , pin->ConnectedTo()->GetAbsY() );
                }
            }
        }
*/
        for( a2dCanvasObjectList::iterator iter = m_wiresEditCopies->begin(); iter != m_wiresEditCopies->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( !obj->IsConnect() )
                continue;

            a2dWirePolylineL* wire = wxStaticCast( obj, a2dWirePolylineL );
            a2dCommandProcessor* cmd = wire->GetRoot()->GetCommandProcessor();
            a2dWirePolylineL* target = wire;
            if ( !m_noEditCopy )
                target = wxStaticCastNull( a2dCanvasObject::PROPID_Original->GetPropertyValue( wire ).Get(), a2dWirePolylineL );
            a2dVertexList* newpoints = wire->GetSegments();
            if ( cmd  )
                cmd->Submit( new a2dCommand_SetSegments( target, newpoints, true ), final );
            else
                target->SetSegments( newpoints );
            for( a2dCanvasObjectList::iterator iterpins = wire->GetChildObjectList()->begin(); iterpins != wire->GetChildObjectList()->end(); ++iterpins )
            {
                a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                if( !pin ) continue;
                if( !pin->IsConnectedTo() ) continue;

                a2dPin* targetPin = pin;
                if ( !m_noEditCopy )
                    targetPin = wxStaticCastNull( a2dCanvasObject::PROPID_Original->GetPropertyValue( pin ).Get(), a2dPin );
                if ( cmd )
                    cmd->Submit( new a2dCommand_SetCanvasProperty( targetPin, a2dCanvasObject::PROPID_Position, pin->GetPosXY(), -1, false ), final );
                else
                    targetPin->SetPosXyPoint( pin->GetPosXY() );
            }
            //AutoConnect( target, true, true );//allowreconnectbegin, allowreconnectend );
        }

/* Check for dislocated pins in originals
        for( a2dCanvasObjectList::iterator iter = m_wires->begin(); iter != m_wires->end(); ++iter )
        {
            a2dWirePolylineL* wire = wxStaticCast( (*iter).Get(), a2dWirePolylineL );
            for( a2dCanvasObjectList::iterator iterpins = wire->GetChildObjectList()->begin(); iterpins != wire->GetChildObjectList()->end(); ++iterpins )
            {
                a2dPin* pin = wxDynamicCast( iterpins->Get(), a2dPin );
                if( !pin ) continue;
                if( !pin->ConnectedTo() ) continue;

                // if a pin on a dragged object is not the same position as wire pin, the wire needs rerouting.
                // Pins on the wires are adjusted to the new situation. And only when final the editcopy wires are transfered to the originals.
                if( pin->IsDislocated() )
                {                            
                    wxLogDebug(wxT("after x=%12.6lf, y=%12.6lf"), pin->GetAbsX() , pin->GetAbsY() );
                    wxLogDebug(wxT("      xo=%12.6lf, y=o%12.6lf"), pin->ConnectedTo()->GetAbsX() , pin->ConnectedTo()->GetAbsY() );
                }
            }
        }
*/
        m_wiresEditCopies->clear();
        for( a2dCanvasObjectList::iterator iter = m_wires->begin(); iter != m_wires->end(); ++iter )
        {
            a2dWalker_RemovePropertyCandoc setp( a2dCanvasObject::PROPID_Editcopy );
            setp.SetSkipNotRenderedInDrawing( true );
            setp.Start( *iter );
            a2dWirePolylineL* wire = wxStaticCast( (*iter).Get(), a2dWirePolylineL );
            wire->EliminateMatrix();
            wire->SetVisible( true );
        }

        OptimizeRerouteWires( true );
        m_wires->clear();
    }

    if ( cmdh )
    {
        if ( m_withBusyEvent )
            cmdh->SendBusyEvent( false );
        else
            cmdh->PopSendBusyEvent();
    }
}

void a2dConnectionGenerator::EndManhattanRoute( a2dPin* pin, a2dLineSegment* seg1, a2dLineSegment* seg2, a2dLineSegment* seg3, a2dVertexList* points, bool reverse )
{
    double seg1seg2Slope = 0;
    double seg2seg3Slope = 0;
    bool seg1seg2Same = false;
    bool seg2seg3Same = false;

    if ( seg1->m_x == seg2->m_x )
    {
        if ( seg1->m_y == seg2->m_y )
        {
            seg1seg2Same = true;
            //point seg1 and seg2 same, use pinangle
            double pinangle = pin->GetAbsAngle();
            if ( pinangle == 0 || pinangle == 180 )
                seg1seg2Slope = 0;
            else
                seg1seg2Slope = DBL_MAX;
        }
        else
            seg1seg2Slope = DBL_MAX;
    }
    else
        seg1seg2Slope = ( seg1->m_y - seg2->m_y ) / ( seg1->m_x - seg2->m_x );

    if ( seg2->m_x == seg3->m_x )
    {
        if ( seg2->m_y == seg3->m_y )
        {
            seg2seg3Same = true;
            seg2seg3Slope = 0;
            if ( seg1seg2Same )
                seg2seg3Slope = seg1seg2Slope;
        }
        else
            seg2seg3Slope = DBL_MAX;
    }
    else
        seg2seg3Slope = ( seg2->m_y - seg3->m_y ) / ( seg2->m_x - seg3->m_x );

    //axis aligned straight angles
    // If bin flag for segment is set, it mean there was a straight segment, at an angle at the start.
    // In that case when m_routeMethod == ManhattanEndSegmentsStay, the segement will stay straight.
    if ( !seg1->GetBin() &&
         (
             ( seg1seg2Slope == 0 && seg2seg3Slope == 0 )||
             ( seg1seg2Slope == DBL_MAX && seg2seg3Slope == DBL_MAX ) ||
             ( seg1seg2Slope == 0 && seg2seg3Slope == DBL_MAX ) ||
             ( seg1seg2Slope == DBL_MAX && seg2seg3Slope == 0 )
         )
       )
    {
        //wxLogDebug( wxString::Format("MS %p", seg1 ) );

        seg1->SetPoint( pin->GetAbsXY() );

        if ( seg1seg2Slope == 0 && seg2seg3Slope == 0 )
        {
            if ( seg1seg2Same && seg2seg3Same )
                seg2->SetPoint( a2dPoint2D( seg2->m_x, pin->GetAbsY() ) );
            else if ( seg2seg3Same )
                seg2->SetPoint( a2dPoint2D( seg2->m_x, pin->GetAbsY() ) );
            else
            {
                if ( reverse )
                    points->push_back( new a2dLineSegment( pin->GetAbsXY() ) );
                else
                    points->push_front( new a2dLineSegment( pin->GetAbsXY() ) );
                seg1->SetPoint( a2dPoint2D( seg2->m_x, pin->GetAbsY() ) );
            }
        }
        else if ( seg1seg2Slope == DBL_MAX && seg2seg3Slope == DBL_MAX )
        {
            if ( seg1seg2Same && seg2seg3Same )
                seg2->SetPoint( a2dPoint2D( seg2->m_x, pin->GetAbsY() ) );
            else if ( seg2seg3Same )
                seg2->SetPoint( a2dPoint2D( seg2->m_x, pin->GetAbsY() ) );
            else
            {
                if ( reverse )
                    points->push_back( new a2dLineSegment( pin->GetAbsXY() ) );
                else
                    points->push_front( new a2dLineSegment( pin->GetAbsXY() ) );
                seg1->SetPoint( a2dPoint2D( seg2->m_x, pin->GetAbsY() ) );
            }
        }
        else if ( seg1seg2Slope == 0 && seg2seg3Slope == DBL_MAX )
            seg2->SetPoint( a2dPoint2D( seg2->m_x, pin->GetAbsY() ) );
        else if ( seg1seg2Slope == DBL_MAX && seg2seg3Slope == 0 )
            seg2->SetPoint( a2dPoint2D( pin->GetAbsX(), seg2->m_y ) );
    }
    else
    {
        if ( m_routeMethod == ManhattanEndSegmentsStay )
        {
            //wxLogDebug( wxString::Format("S %p", seg1 ) );
            seg1->SetBin( true );
            seg1->SetPoint( pin->GetAbsXY() );
        }
        else if ( m_routeMethod == ManhattanEndSegmentsConvertAndStay )
        {   //convert straight endsegment to a manhattan pair of segments
            if ( reverse )
                points->push_back( new a2dLineSegment( pin->GetAbsXY() ) );
            else
                points->push_front( new a2dLineSegment( pin->GetAbsXY() ) );
            seg1->SetPoint( a2dPoint2D( seg2->m_x, pin->GetAbsY() ) );
        }
        else
        {   // not written yet method
            seg1->SetPoint( pin->GetAbsXY() );
        }
    }
}

void a2dConnectionGenerator::SetConnectObject( a2dCanvasObject* tmplObject )
{
    m_tmplObject = tmplObject;
}

a2dCanvasObject* a2dConnectionGenerator::GetConnectObject() const
{
    return m_tmplObject;
}

a2dCanvasObject* a2dConnectionGenerator::CreateConnectObject(  a2dCanvasObject* parent, a2dPin* pinThis, a2dPin* pinOther, bool undo ) const
{
    a2dPinClass* thisPinClass = NULL;
    a2dPinClass* otherPinClass = NULL;
    a2dPinClass* wirePinClassThis = NULL;
    a2dPinClass* wirePinClassOther = NULL;
    a2dWirePolylineL* wire = NULL;
    a2dCanvasObject* connect = NULL;
    m_reverseCreate = false;

    wire = wxStaticCast( m_tmplObject.Get(), a2dWirePolylineL );
    if ( wire )
    {
        wirePinClassThis = wire->GetStartPinClass();
        wirePinClassOther = wire->GetEndPinClass();
        if ( pinThis && pinOther )
        {
            thisPinClass = pinThis->GetPinClass();
            otherPinClass = pinOther->GetPinClass();

            if ( thisPinClass->CanConnectTo( wire->GetStartPinClass() ) && otherPinClass->CanConnectTo( wire->GetEndPinClass() ) )
            {
                m_reverseCreate = false;
            }
            else if ( otherPinClass->CanConnectTo( wire->GetStartPinClass() ) && thisPinClass->CanConnectTo( wire->GetEndPinClass() ) )
            {
                m_reverseCreate = true;
            }
            else
            {   
                wire = NULL;
            }
        }
        else if ( pinThis )
        {
            thisPinClass = pinThis->GetPinClass();
            if ( thisPinClass->CanConnectTo( wire->GetStartPinClass() ) )
            {
                m_reverseCreate = false;
            }
            else if ( thisPinClass->CanConnectTo( wire->GetEndPinClass() ) )
            {
                m_reverseCreate = true;
            }
            else
            {   
                wire = NULL;
            }
        }
        else if ( pinOther )
        {
            otherPinClass = pinOther->GetPinClass();
            if ( otherPinClass->CanConnectTo( wire->GetEndPinClass() ) )
            {
                m_reverseCreate = false;
            }
            else if ( otherPinClass->CanConnectTo( wire->GetStartPinClass() ) )
            {
                m_reverseCreate = true;
            }
            else
            {   
                wire = NULL;
            }
        }

        connect = wire;
    }
    else
    {
        connect = m_tmplObject;
        wirePinClassThis = a2dPinClass::Standard;
        wirePinClassOther = a2dPinClass::Standard;
    }

    // make wires always from pinThis to pinOther
    // So begin pin is connected at pinThis
    // So end pin is connected at pinOther
    if ( m_reverseCreate )
    {
        a2dPinPtr swappin = pinThis;
        pinThis = pinOther;
        pinOther = swappin;
        a2dPinClass* swapPinClass = thisPinClass;
        thisPinClass = otherPinClass;
        otherPinClass = swapPinClass;
    }

    if ( connect )
    {
        connect = wxStaticCast( connect->Clone( clone_deep ), a2dCanvasObject );
        wire = wxStaticCast( connect, a2dWirePolylineL );

        if ( !undo )
        {
            parent->Append( connect );
            if ( pinThis )
            {
                if (wire )
                    wire->AddPoint( pinThis->GetAbsX(), pinThis->GetAbsY(), -1 , true );
                a2dPin* begin = wire->AddPin( wxT( "begin" ), pinThis->GetAbsX(), pinThis->GetAbsY(), a2dPin::NON,
                                              wirePinClassThis );
                connect->ConnectPins( parent, pinThis, begin, undo );
            }
            else
                connect->AddPin( wxT( "begin" ), pinOther->GetAbsX(), pinOther->GetAbsY(), a2dPin::NON,
                              wirePinClassThis );

            if ( pinOther )
            {
                if (wire )
                    wire->AddPoint( pinOther->GetAbsX(), pinOther->GetAbsY(), -1 , true );
                a2dPin* end = connect->AddPin( wxT( "end" ), pinOther->GetAbsX(), pinOther->GetAbsY(), a2dPin::NON,
                                            wirePinClassOther );
                connect->ConnectPins( parent, pinOther, end, undo );
            }
            else
                connect->AddPin( wxT( "end" ), pinThis->GetAbsX(), pinThis->GetAbsY(), a2dPin::NON,
                              wirePinClassOther );
        }
        else
        {
            a2dCanvasCommandProcessor* cmp = parent->GetRoot()->GetCanvasCommandProcessor();
            cmp->Submit( new a2dCommand_AddObject( parent, wire ) );
            if ( pinThis )
            {
                if (wire )
                    cmp->Submit( new a2dCommand_AddPoint( wire, pinThis->GetAbsX(), pinThis->GetAbsY(), -1 ) );
                a2dPin* begin = connect->AddPin( wxT( "begin" ), pinThis->GetAbsX(), pinThis->GetAbsY(), a2dPin::NON,
                                              wirePinClassThis, true );

                //cmp->Submit( new a2dCommand_AddObject( wire, begin ), true );
                cmp->Submit( new a2dCommand_ConnectPins( pinThis, begin ), true );
            }
            else
                connect->AddPin( wxT( "begin" ), pinOther->GetAbsX(), pinOther->GetAbsY(), a2dPin::NON,
                              wirePinClassThis, true );

            if ( pinOther )
            {
                if (wire )
                    cmp->Submit( new a2dCommand_AddPoint( wire, pinOther->GetAbsX(), pinOther->GetAbsY(), -1 ) );
                a2dPin* end = connect->AddPin( wxT( "end" ), pinOther->GetAbsX(), pinOther->GetAbsY(), a2dPin::NON,
                                            wirePinClassOther, true );

                //cmp->Submit( new a2dCommand_AddObject( wire, end ), true );
                cmp->Submit( new a2dCommand_ConnectPins( pinOther, end ), true );
            }
            else
                connect->AddPin( wxT( "end" ), pinThis->GetAbsX(), pinThis->GetAbsY(), a2dPin::NON,
                              wirePinClassOther, true );
        }

        if (wire )
            wire->SetConnectionInfo( true, thisPinClass, otherPinClass );
        return wire;
    }
    return NULL;
}

a2dCanvasObject* a2dConnectionGenerator::GetConnectTemplate( const a2dCanvasObject* object, a2dPinClass* thisPinClass, const a2dCanvasObject* other, a2dPinClass* otherPinClass ) const
{
    a2dWirePolylineL* wire = NULL;
    a2dCanvasObject* connect = NULL;
    wire = wxStaticCast( m_tmplObject.Get(), a2dWirePolylineL );
    if ( wire )
    {
        if ( thisPinClass && otherPinClass )
        {
            if ( thisPinClass->CanConnectTo( wire->GetStartPinClass() ) && otherPinClass->CanConnectTo( wire->GetEndPinClass() ) )
            {
                m_reverseCreate = false;
            }
            else if ( otherPinClass->CanConnectTo( wire->GetStartPinClass() ) && thisPinClass->CanConnectTo( wire->GetEndPinClass() ) )
            {
                m_reverseCreate = true;
            }
            else
            {   
                wire = NULL;
            }
        }
        else if ( thisPinClass )
        {
            if ( thisPinClass->CanConnectTo( wire->GetStartPinClass() ) )
            {
                m_reverseCreate = false;
            }
            else if ( thisPinClass->CanConnectTo( wire->GetEndPinClass() ) )
            {
                m_reverseCreate = true;
            }
            else
            {   
                wire = NULL;
            }
        }
        else if ( otherPinClass )
        {
            if ( otherPinClass->CanConnectTo( wire->GetEndPinClass() ) )
            {
                m_reverseCreate = false;
            }
            else if ( otherPinClass->CanConnectTo( wire->GetStartPinClass() ) )
            {
                m_reverseCreate = true;
            }
            else
            {   
                wire = NULL;
            }
        }
        connect = wire;
    }
    else
    {
        connect = m_tmplObject;
    }

    return connect;
}


bool a2dConnectionGenerator::GeneratePossibleConnections( a2dCanvasObject* object, a2dPinClass* pinClass, a2dConnectTask task, double x, double y, double margin ) const
{
    if ( !m_generatePins )
    {
        if ( ! wxDynamicCast( object, a2dWirePolylineL ) ) // this object should always deliver pins
        return false;
    }

    assert( pinClass );
    bool found = object->GeneratePins( pinClass, task, x, y, margin );
    return found;
}

a2dPinClass* a2dConnectionGenerator::GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj, a2dPinClass* pinClassTo, a2dPin* pinFrom ) const
{
    wxASSERT_MSG( pinClass, wxT( "pinClass cannot be NULL, maybe use a2dPinClass::Any?" ) );
/*
    // no pinclass, only happens when trying to start a new wire on an object/wire, without knowing start pinclass.
    if ( pinClassTo == a2dPinClass::Any )
    {
        if ( obj && obj->GetRoot() && obj->GetRoot()->GetLayerSetup() )
        {
            a2dLayerInfo* info = obj->GetRoot()->GetLayerSetup()->GetLayerIndex()[ obj->GetLayer() ];
            if ( info != wxNullLayerInfo )
                return info->GetPinClass();

        }

        a2dPinClass* objectPinClassStart = pinClass->CanConnectToAsObject();
        if ( objectPinClassStart )
            return objectPinClassStart;

        if ( obj->IsConnect() )
            return m_anypinclass;
        else
            return m_anypinclass;
    }
*/

    switch ( task )
    {
        case a2d_StartWire:
        {
            return pinClass;
        }
        break;
        case a2d_FinishWire:
        {
            return pinClassTo;
        }
        break;

        case a2d_GeneratePinsForPinClass:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }

        case a2d_GeneratePinsForStartWire:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }
        break;

        case a2d_GeneratePinsForFinishWire:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }
        break;

        default:

            break;
    }

    return NULL;
}

bool a2dConnectionGenerator::GeneratePinsToConnect( a2dDrawingPart* part, a2dCanvasObject* root, a2dPinClass* pinClassToConnectTo, a2dConnectTask task, double xpin, double ypin, a2dCanvasObjectFlagsMask mask )
{
	double x = xpin;
	double y = ypin;
    double onePixel = part->GetDrawer2D()->DeviceToWorldXRel( 1 );

	//The margin down here can not be zero, since hittest on wire with zero width will not hit.
    a2dIterC ic( part );
    if ( a2dPin::GetWorldBased() )
        ic.SetHitMarginWorld( onePixel );//a2dCanvasGlobals->GetHabitat()->GetPinSize()/2.0 );
    else
        ic.SetHitMarginDevice( 1 );// a2dCanvasGlobals->GetHabitat()->GetPinSize()/2.0 );

    //double m = ic.GetHitMarginWorld();
    double m = a2dCanvasGlobals->GetHabitat()->GetPinSize()/2.0;

    //we still want to do this, else even wires will not have generated pins.
    //if ( !m_generatePins )
    //    return false;

    a2dWalker_GeneratePinsToConnect generateconnectpins( mask, pinClassToConnectTo, task, xpin, ypin );
    if ( root == part->GetShowObject() )
    {
        ic.SetLayer( wxLAYER_ALL ); //generate pins not layer specific (all generates pins)

        generateconnectpins.SetBoundingBoxCondition( _ON | _IN );
        generateconnectpins.SetBoundingBox( a2dBoundingBox( x - m , y - m , x + m , y + m ) );
        //Try Generate pins on all objects hit.
        a2dAffineMatrix cworld = root->GetTransformMatrix();
        cworld.Invert();

        a2dIterCU cu( ic, cworld );
        return generateconnectpins.Start( ic, root );
    }
    else
    {
        ic.SetLayer( wxLAYER_ALL ); //generate pins does not need to be layer specific in one object
        // Generate pins only on a specific object.
        return generateconnectpins.Start( ic, root, false );
    }
}

a2dPin* a2dConnectionGenerator::SearchPinForFinishWire( a2dCanvasObject* root, a2dPin* pinToConnectTo, a2dPinClass* pinClassToConnectTo, double margin, a2dCanvasObjectFlagsMask mask )
{
	double x = pinToConnectTo->GetAbsX();
	double y = pinToConnectTo->GetAbsY();

    double m = margin;

	a2dWalker_FindPinsToConnectToPin findconnectpins( mask, pinToConnectTo, a2d_FinishWire, margin );
    findconnectpins.SetBoundingBox( a2dBoundingBox( x - m , y - m , x + m , y + m ) );
    findconnectpins.SetBoundingBoxCondition( _ON | _IN );
    findconnectpins.StopAtFirst( true );
    findconnectpins.Start( root );

    if ( findconnectpins.m_found.size() )
        return wxDynamicCast( findconnectpins.m_found.front().Get(), a2dPin );
    return NULL;
}

a2dPin* a2dConnectionGenerator::SearchPinForStartWire( a2dCanvasObject* root, double xpin, double ypin, a2dPinClass* pinClassToConnectTo, double margin, a2dCanvasObjectFlagsMask mask )
{
            /* MIGHT BE INTERESTING extended testing  hits all pins, so can decide what it wants as hit.

    a2dHitEvent hitinfo( xw, yw, false, a2dCANOBJHITOPTION_NONE, true );
    a2dCanvasObject* hit = GetCanvasView()->IsHitWorld( hitinfo, wxLAYER_ALL );

            a2dPin *pin = 0;
            {
                //First try to find object pins which are not of connection/wire objects
                int i;
                float minDist = FLT_MAX;
                for( i=0; i< hitinfo.m_extended.size(); i++ )
                {
                    a2dPin *pin2 = wxDynamicCast( hitinfo.m_extended[i].GetObject(), a2dPin );
                    if(
                        pin2 &&
                        !pin2->ConnectedTo() &&
                        pin2->GetPinClass() && pin2->GetPinClass()->IsObjectPin() &&
                        hitinfo.m_extended[i].GetHitType().m_distance < minDist
                    )
                    {
                        pin = pin2;
                        how2 = hitinfo.m_extended[i].GetHitType();
                        minDist = how2.m_distance;
                    }
                }
                if ( !pin )
                    //Now try to find object pins which are of connection/wire objects
                    for( i=0; i< hitinfo.m_extended.size(); i++ )
                    {
                        a2dPin *pin2 = wxDynamicCast( hitinfo.m_extended[i].GetObject(), a2dPin );
                        if(
                            pin2 &&
                            !pin2->ConnectedTo() &&
                            pin2->GetPinClass() && !pin2->GetPinClass()->IsObjectPin() ) &&
                            hitinfo.m_extended[i].GetHitType().m_distance < minDist
                        )
                        {
                            pin = pin2;
                            how2 = hitinfo.m_extended[i].GetHitType();
                            minDist = how2.m_distance;
                        }
                    }
            }
            */

    double m = margin;
    a2dWalker_FindPinsToConnect findconnectpins( mask, pinClassToConnectTo, a2d_StartWire, xpin, ypin, margin );
    findconnectpins.SetBoundingBox( a2dBoundingBox( xpin - m , ypin - m , xpin + m , ypin + m ) );
    findconnectpins.SetBoundingBoxCondition( _ON | _IN );
    findconnectpins.StopAtFirst( true );
    findconnectpins.Start( root );

    m_returnPinClass = findconnectpins.m_returnPinclass;
    m_returnConnect = NULL;

    if ( findconnectpins.m_found.size() )
        return wxDynamicCast( findconnectpins.m_found.front().Get(), a2dPin );
    return NULL;
}

void a2dConnectionGenerator::SetPinsToBeginState( a2dCanvasObject* root, a2dCanvasObjectFlagsMask mask )
{
    a2dAffineMatrix cworld = root->GetTransformMatrix();
    cworld.Invert();

    a2dBoundingBox mapped = root->GetMappedBbox( cworld );

	a2dWalker_SetPinsToBeginState resetpins( mask );
	resetpins.SetBoundingBox( mapped );
	resetpins.SetBoundingBoxCondition( _ON | _IN );
	resetpins.Start( root );
}

void a2dConnectionGenerator::SetPinsToEndState( a2dCanvasObject* root, a2dCanvasObjectFlagsMask mask )
{
    a2dWalker_SetPinsToEndState endstate( mask );
    endstate.Start( root );
}

bool a2dConnectionGenerator::GeneratePinsToConnectObject( a2dDrawingPart* part, a2dCanvasObject* root, a2dCanvasObject* connectObject, a2dPinClass* pinClassToConnectTo, a2dCanvasObjectFlagsMask mask )
{
    if ( !m_generatePins )
    {
        if ( ! wxDynamicCast( connectObject, a2dWirePolylineL ) ) // this object should always deliver pins
        return false;
    }

    a2dIterC ic( part );
    if ( a2dPin::GetWorldBased() )
        ic.SetHitMarginWorld( a2dCanvasGlobals->GetHabitat()->GetPinSize()/2.0 );
    else
        ic.SetHitMarginDevice( a2dCanvasGlobals->GetHabitat()->GetPinSize()/2.0 );

    double m = ic.GetHitMarginWorld();

    a2dWalker_GeneratePinsToConnectObject feedbconnections( mask, connectObject, a2d_GeneratePinsForPinClass );
    feedbconnections.Start( ic, root );
    return true;
}

bool a2dConnectionGenerator::ConnectToPinsObject( a2dCanvasObject* root, a2dCanvasObject* connectObject, double margin, a2dPinClass* pinClassToConnectTo, a2dCanvasObjectFlagsMask mask )
{
    a2dWalker_ConnectToPinsObject connectWherePossible( mask, connectObject, a2d_GeneratePinsForPinClass, margin );
    connectWherePossible.Start( root, true );
    return true;
}

a2dPort* a2dConnectionGenerator::GetPortForPinClasses( a2dPin* one,  a2dPin* two )
{
    a2dPinClass* pclassOne = one->GetPinClass();
    a2dPinClass* pclassTwo = two->GetPinClass();
    a2dPort* port = new a2dPort( NULL, one->GetAbsX(), one->GetAbsY(), "1", pclassOne, pclassOne );

    return port;
}

//----------------------------------------------------------------------------
// a2dConnectionGeneratorInOut
//----------------------------------------------------------------------------

a2dPinClass* a2dConnectionGeneratorInOut::Input = NULL;
a2dPinClass* a2dConnectionGeneratorInOut::Output = NULL;

a2dConnectionGeneratorInOut::a2dConnectionGeneratorInOut()
    : a2dConnectionGenerator()
{
    m_anypinclass = NULL;

    //Input <=> Output

    Input = new a2dPinClass( wxT( "input" ) );
    Output->SetAngleLine( false );
    Output = new a2dPinClass( wxT( "output" ) );
    Input->SetAngleLine( false );
    Input->SetConnectionGenerator( this );
    Output->SetConnectionGenerator( this );
    Input->AddConnect( Output );
    Output->AddConnect( Input );
    a2dPinClass::m_allPinClasses.push_back( Input );
    a2dPinClass::m_allPinClasses.push_back( Output );

    //Setup a wire/pin to define which pins can connect, and with which wire
    a2dWirePolylineL* wireio = new a2dWirePolylineL();
    a2dStroke wirestroke = a2dStroke( wxColour( 0, 50, 0 ), 2, a2dSTROKE_LONG_DASH );
    wireio->SetStroke( wirestroke );
    wireio->SetStartPinClass( Input );
    wireio->SetEndPinClass( Output );
    a2dArrow* arrow2 = new  a2dArrow( 0, 0, 4, 0, 2 );
    arrow2->SetStroke( wirestroke );
    wireio->SetEnd( arrow2 );
    SetConnectObject( wireio );
}

a2dConnectionGeneratorInOut::~a2dConnectionGeneratorInOut()
{
}

a2dPinClass* a2dConnectionGeneratorInOut::GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj, a2dPinClass* pinClassTo, a2dPin* pinFrom ) const
{
    switch ( task )
    {
        case a2d_StartWire:
        {
            if ( pinClass == Input )
                return Output;

            if ( pinClass == Output )
                return Input;
        }
        break;
        case a2d_FinishWire:
        {
            if ( pinClass == Input )
                return Output;

            if ( pinClass == Output )
                return Input;
        }
        break;

        case a2d_GeneratePinsForPinClass:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }
        break;

        case a2d_GeneratePinsForStartWire:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }
        break;

        case a2d_GeneratePinsForFinishWire:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }
        break;

        default:

            break;
    }

    return NULL;
}

//----------------------------------------------------------------------------
// a2dConnectionGeneratorObjectWire
//----------------------------------------------------------------------------

a2dPinClass* a2dConnectionGeneratorObjectWire::Object = NULL;
a2dPinClass* a2dConnectionGeneratorObjectWire::Wire = NULL;


a2dConnectionGeneratorObjectWire::a2dConnectionGeneratorObjectWire()
    : a2dConnectionGenerator()
{
    m_anypinclass = NULL;

    //Object <=> Wire
    Object = new a2dPinClass( wxT( "object" ) );
    Object->SetConnectionGenerator( this );
    Object->SetAngleLine( false );
    Wire = new a2dPinClass( wxT( "wire" ) );
    Wire->SetConnectionGenerator( this );
    Wire->SetAngleLine( false );

    Object->AddConnect( Wire );
    Object->AddConnect( Object );
    Wire->AddConnect( Object );
    Wire->AddConnect( Wire );

    //Setup a wire/pin to define which pins can connect, and with which wire
    a2dWirePolylineL* wireio = new a2dWirePolylineL();
    a2dStroke wirestroke = a2dStroke( wxColour( 100, 250, 100 ), 2, a2dSTROKE_LONG_DASH );
    wireio->SetStroke( wirestroke );
    wireio->SetStartPinClass( Wire );
    wireio->SetEndPinClass( Wire );
    a2dPinClass::m_allPinClasses.push_back( Object );
    a2dPinClass::m_allPinClasses.push_back( Wire );

    a2dArrow* arrow2 = new  a2dArrow( 0, 0, 4, 0, 2 );
    arrow2->SetStroke( wirestroke );
    wireio->SetEnd( arrow2 );
    SetConnectObject( wireio );
}

a2dConnectionGeneratorObjectWire::~a2dConnectionGeneratorObjectWire()
{
}

a2dPinClass* a2dConnectionGeneratorObjectWire::GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj, a2dPinClass* pinClassTo, a2dPin* pinFrom ) const
{
    switch ( task )
    {
        case a2d_StartWire:
        {
            if ( pinClass == Object )
                return Wire;

            if ( pinClass == Wire )
                return Object;
        }
        break;
        case a2d_FinishWire:
        {
            if ( pinClass == Object )
                return Wire;

            if ( pinClass == Wire )
                return Object;
        }
        break;

        case a2d_GeneratePinsForPinClass:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }

        case a2d_GeneratePinsForStartWire:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }
        break;

        case a2d_GeneratePinsForFinishWire:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }
        break;

        default:

            break;
    }

    return NULL;
}

//----------------------------------------------------------------------------
// a2dConnectionGeneratorDirectedFlow
//----------------------------------------------------------------------------

a2dPinClass* a2dConnectionGeneratorDirectedFlow::ObjectInput = NULL;
a2dPinClass* a2dConnectionGeneratorDirectedFlow::ObjectOutput = NULL;
a2dPinClass* a2dConnectionGeneratorDirectedFlow::WireInput = NULL;
a2dPinClass* a2dConnectionGeneratorDirectedFlow::WireOutput = NULL;

a2dConnectionGeneratorDirectedFlow::a2dConnectionGeneratorDirectedFlow()
    : a2dConnectionGenerator()
{
    m_anypinclass = NULL;

    // ObjectInput <=> ObjectOutput

    ObjectInput = new a2dPinClass( wxT( "objectinput" ) );
    ObjectOutput = new a2dPinClass( wxT( "objectoutput" ) );
    WireInput = new a2dPinClass( wxT( "wireinput" ) );
    WireOutput = new a2dPinClass( wxT( "wireoutput" ) );
    ObjectInput->SetAngleLine( false );
    ObjectOutput->SetAngleLine( false );
    WireInput->SetAngleLine( false );
    WireOutput->SetAngleLine( false );

    ObjectInput->SetConnectionGenerator( this );
    ObjectOutput->SetConnectionGenerator( this );
    WireInput->SetConnectionGenerator( this );
    WireOutput->SetConnectionGenerator( this );

    ObjectInput->AddConnect( WireOutput );
    ObjectInput->AddConnect( ObjectOutput );
    WireOutput->AddConnect( ObjectInput );
    WireOutput->AddConnect( WireInput );
    ObjectOutput->AddConnect( WireInput );
    ObjectOutput->AddConnect( ObjectInput );
    WireInput->AddConnect( ObjectOutput );
    WireInput->AddConnect( WireOutput );

	a2dPinClass::m_allPinClasses.push_back( ObjectInput );
    a2dPinClass::m_allPinClasses.push_back( ObjectOutput );
    a2dPinClass::m_allPinClasses.push_back( WireInput );
    a2dPinClass::m_allPinClasses.push_back( WireOutput );

    //Setup a wire/pin to define which pins can connect, and with which wire
    a2dWirePolylineL* wireio = new a2dWirePolylineL();
    a2dStroke wirestroke = a2dStroke( wxColour( 55, 250, 250 ), 2, a2dSTROKE_LONG_DASH );
    wireio->SetStroke( wirestroke );
    wireio->SetStartPinClass( WireInput );
    wireio->SetEndPinClass( WireOutput );
    a2dArrow* arrow2 = new  a2dArrow( 0, 0, 4, 0, 2 );
    arrow2->SetStroke( wirestroke );
    wireio->SetEnd( arrow2 );

    SetConnectObject( wireio );

}

a2dConnectionGeneratorDirectedFlow::~a2dConnectionGeneratorDirectedFlow()
{
}

a2dPinClass* a2dConnectionGeneratorDirectedFlow::GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj, a2dPinClass* pinClassTo, a2dPin* pinFrom ) const
{
/*
    if ( pinClass == a2dPinClass::Any )
    {
        if ( obj->IsConnect() )
        {
            if ( wxDynamicCast( obj, a2dWirePolylineL )->GetStartPinClass() ==  a2dPinClass::WireInput )
                return a2dPinClass::WireOutput;
            if ( wxDynamicCast( obj, a2dWirePolylineL )->GetStartPinClass() ==  a2dPinClass::WireOutput )
                return a2dPinClass::WireInput;
            else
                return NULL;
        }
        else
        {
            return a2dPinClass::Object;
        }
    }
*/

    switch ( task )
    {
        case a2d_StartWire:
        {
            if ( pinClass == ObjectInput )
                return WireOutput;

            if ( pinClass == ObjectOutput )
                return WireInput;

            if ( pinClass == WireInput )
                return ObjectOutput;

            if ( pinClass == WireOutput )
                return ObjectInput;
        }
        break;
        case a2d_FinishWire:
        {
            if ( pinClass == ObjectInput )
                return WireOutput;

            if ( pinClass == ObjectOutput )
                return WireInput;

            if ( pinClass == WireInput )
                return ObjectOutput;

            if ( pinClass == WireOutput )
                return ObjectInput;
        }
        break;

        case a2d_GeneratePinsForPinClass:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }

        case a2d_GeneratePinsForStartWire:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }
        break;

        case a2d_GeneratePinsForFinishWire:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }
        break;

        default:

            break;
    }

    return NULL;
}

//----------------------------------------------------------------------------
// a2dConnectionGeneratorTwoFlow
//----------------------------------------------------------------------------

a2dPinClass* a2dConnectionGeneratorTwoFlow::FlowAInput = NULL;
a2dPinClass* a2dConnectionGeneratorTwoFlow::FlowAOutput = NULL;
a2dPinClass* a2dConnectionGeneratorTwoFlow::FlowAWireInput = NULL;
a2dPinClass* a2dConnectionGeneratorTwoFlow::FlowAWireOutput = NULL;
a2dPinClass* a2dConnectionGeneratorTwoFlow::FlowBInput = NULL;
a2dPinClass* a2dConnectionGeneratorTwoFlow::FlowBOutput = NULL;
a2dPinClass* a2dConnectionGeneratorTwoFlow::FlowBWireInput = NULL;
a2dPinClass* a2dConnectionGeneratorTwoFlow::FlowBWireOutput = NULL;


a2dConnectionGeneratorTwoFlow::a2dConnectionGeneratorTwoFlow()
    : a2dConnectionGenerator()
{
    m_anypinclass = NULL;

    //FLOWA

    FlowAInput = new a2dPinClass( wxT( "FlowAinput" ) );
    FlowAOutput = new a2dPinClass( wxT( "FlowAoutput" ) );
    FlowAWireInput = new a2dPinClass( wxT( "FlowAwireinput" ) );
    FlowAWireOutput = new a2dPinClass( wxT( "FlowAwireoutput" ) );

    FlowAInput->SetConnectionGenerator( this );
    FlowAOutput->SetConnectionGenerator( this );
    FlowAWireInput->SetConnectionGenerator( this );
    FlowAWireOutput->SetConnectionGenerator( this );

    FlowAInput->AddConnect( FlowAWireOutput );
    FlowAInput->AddConnect( FlowAOutput );
    FlowAWireOutput->AddConnect( FlowAInput );
    FlowAOutput->AddConnect( FlowAWireInput );
    FlowAOutput->AddConnect( FlowAInput );
    FlowAWireInput->AddConnect( FlowAOutput );

    FlowAWireInput->AddConnect( FlowAWireOutput );
    FlowAWireOutput->AddConnect( FlowAWireInput );

	a2dPinClass::m_allPinClasses.push_back( FlowAInput );
    a2dPinClass::m_allPinClasses.push_back( FlowAOutput );
    a2dPinClass::m_allPinClasses.push_back( FlowAWireInput );
    a2dPinClass::m_allPinClasses.push_back( FlowAWireOutput );

    //Setup a wire/pin to define which pins can connect, and with which wire
    a2dWirePolylineL* wireio = new a2dWirePolylineL();
    a2dStroke wirestroke = a2dStroke( wxColour( 5, 250, 250 ), 2, a2dSTROKE_LONG_DASH );
    wireio->SetStroke( wirestroke );
    wireio->SetStartPinClass( FlowAWireInput );
    wireio->SetEndPinClass( FlowAWireOutput );
    a2dArrow* arrow2 = new  a2dArrow( 0, 0, 4, 0, 2 );
    arrow2->SetStroke( wirestroke );
    wireio->SetEnd( arrow2 );
    SetConnectObject( wireio );

    FlowAInput->SetAngleLine( false );
    FlowAWireInput->SetAngleLine( false );
    FlowAWireOutput->SetAngleLine( false );
    FlowAInput->SetAngleLine( false );
    FlowAOutput->SetAngleLine( false );


    //FLOWB

    FlowBInput = new a2dPinClass( wxT( "FlowBinput" ) );
    FlowBOutput = new a2dPinClass( wxT( "FlowBoutput" ) );
    FlowBWireInput = new a2dPinClass( wxT( "FlowBwireinput" ) );
    FlowBWireOutput = new a2dPinClass( wxT( "FlowBwireoutput" ) );

    FlowBInput->SetConnectionGenerator( this );
    FlowBOutput->SetConnectionGenerator( this );
    FlowBWireInput->SetConnectionGenerator( this );
    FlowBWireOutput->SetConnectionGenerator( this );

    FlowBInput->AddConnect( FlowBWireOutput );
    FlowBInput->AddConnect( FlowBOutput );
    FlowBWireOutput->AddConnect( FlowBInput );
    FlowBOutput->AddConnect( FlowBWireInput );
    FlowBOutput->AddConnect( FlowBInput );
    FlowBWireInput->AddConnect( FlowBOutput );

    FlowBWireInput->AddConnect( FlowBWireOutput );
    FlowBWireOutput->AddConnect( FlowBWireInput );

	a2dPinClass::m_allPinClasses.push_back( FlowBInput );
    a2dPinClass::m_allPinClasses.push_back( FlowBOutput );
    a2dPinClass::m_allPinClasses.push_back( FlowBWireInput );
    a2dPinClass::m_allPinClasses.push_back( FlowBWireOutput );

    //Setup a wire/pin to define which pins can connect, and with which wire
    a2dWirePolylineL* wireiob = new a2dWirePolylineL();
    a2dStroke wirestrokeb = a2dStroke( wxColour( 25, 0, 25 ), 1, a2dSTROKE_SOLID );
    wireiob->SetStroke( wirestrokeb );
    wireiob->SetStartPinClass( FlowBWireInput );
    wireiob->SetEndPinClass( FlowBWireOutput );
    a2dArrow* arrow2b = new  a2dArrow( 0, 0, 4, 0, 2 );
    arrow2b->SetStroke( wirestrokeb );
    wireiob->SetEnd( arrow2b );
    SetConnectObjectFlowB( wireiob );

    FlowBInput->SetAngleLine( false );
    FlowBWireInput->SetAngleLine( false );
    FlowBWireOutput->SetAngleLine( false );
    FlowBInput->SetAngleLine( false );
    FlowBOutput->SetAngleLine( false );

}

a2dConnectionGeneratorTwoFlow::~a2dConnectionGeneratorTwoFlow()
{
}

void a2dConnectionGeneratorTwoFlow::SetConnectObjectFlowB( a2dCanvasObject* tmplObject )
{
    m_tmplObjectFlowB = tmplObject;
}

a2dCanvasObject* a2dConnectionGeneratorTwoFlow::GetConnectObjectFlowB() const
{
    return m_tmplObjectFlowB;
}

a2dPinClass* a2dConnectionGeneratorTwoFlow::GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj, a2dPinClass* pinClassTo, a2dPin* pinFrom ) const
{
    switch ( task )
    {
        case a2d_StartWire:
        {
            if ( pinClass == FlowAInput )
                return FlowAWireOutput;

            if ( pinClass == FlowAOutput )
                return FlowAWireInput;

            if ( pinClass == FlowAWireInput )
                return FlowAOutput;

            if ( pinClass == FlowAWireOutput )
                return FlowAInput;

            if ( pinClass == FlowBInput )
                return FlowBWireOutput;

            if ( pinClass == FlowBOutput )
                return FlowBWireInput;

            if ( pinClass == FlowAWireInput )
                return FlowBOutput;

            if ( pinClass == FlowBWireOutput )
                return FlowBInput;
        }
        break;
        case a2d_FinishWire:
        {
            if ( pinClass == FlowAInput )
                return FlowAWireOutput;

            if ( pinClass == FlowAOutput )
                return FlowAWireInput;

            if ( pinClass == FlowAWireInput )
                return FlowAOutput;

            if ( pinClass == FlowAWireOutput )
                return FlowAInput;

            if ( pinClass == FlowBInput )
                return FlowBWireOutput;

            if ( pinClass == FlowBOutput )
                return FlowBWireInput;

            if ( pinClass == FlowBWireInput )
                return FlowBOutput;

            if ( pinClass == FlowBWireOutput )
                return FlowBInput;
        }
        break;

        case a2d_GeneratePinsForPinClass:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }

        case a2d_GeneratePinsForStartWire:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }
        break;

        case a2d_GeneratePinsForFinishWire:
        {
            a2dPinClass* PinClass = pinClass->CanConnectTo( NULL );
            return PinClass;
        }
        break;

        default:

            break;
    }

    return NULL;
}

a2dCanvasObject* a2dConnectionGeneratorTwoFlow::CreateConnectObject(  a2dCanvasObject* parent, a2dPin* pinThis, a2dPin* pinOther, bool undo ) const
{
    a2dPinClass* thisPinClass = NULL;
    a2dPinClass* otherPinClass = NULL;
    if ( pinThis )
        thisPinClass = pinThis->GetPinClass();
    if ( pinOther )
        otherPinClass = pinOther->GetPinClass();

    if ( 
            pinThis && 
            ( thisPinClass == FlowAInput || thisPinClass == FlowAOutput || thisPinClass == FlowAWireInput || thisPinClass == FlowAWireOutput ) 
        ) 
        return a2dConnectionGenerator::CreateConnectObject( parent, pinThis, pinOther, undo );
    if ( 
            pinOther && 
            ( otherPinClass == FlowAInput || otherPinClass == FlowAOutput || otherPinClass == FlowAWireInput || otherPinClass == FlowAWireOutput ) 
        ) 
        return a2dConnectionGenerator::CreateConnectObject( parent, pinThis, pinOther, undo );

    // assume flow B

    a2dCanvasObjectPtr save = m_tmplObject; 
    m_tmplObject = m_tmplObjectFlowB;
    a2dCanvasObject* ret = a2dConnectionGenerator::CreateConnectObject( parent, pinThis, pinOther, undo );
    m_tmplObject = save; 
    return ret;
}