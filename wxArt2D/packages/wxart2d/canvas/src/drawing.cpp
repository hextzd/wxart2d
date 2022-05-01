/*! \file canvas/src/candoc.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: candoc.cpp,v 1.190 2009/09/02 19:46:07 titato Exp $
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
#include <float.h>
#include <wx/tokenzr.h>
#include <locale.h>


#if wxUSE_STD_IOSTREAM
#include "wx/ioswrap.h"
#if wxUSE_IOSTREAMH
#include <fstream.h>
#else
#include <fstream>
#endif
#else
#include "wx/wfstream.h"
#endif

#include "wx/canvas/xmlpars.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/booloper.h"
#include "wx/canvas/canvas.h"
#include "wx/canvas/tools.h"
#include "wx/canvas/sttool.h"
#include "wx/canvas/sttool2.h"
#include "wx/canvas/edit.h"
#include "wx/canvas/identifydlg.h"
#include "wx/canvas/pathsettings.h"
#include "wx/canvas/strucdlg.h"
#include "wx/canvas/styledialg.h"
#include "wx/canvas/tooldlg.h"
#include "wx/canvas/layerdlg.h"
#include "wx/canvas/fontdlg.h"
#include "wx/canvas/groupdlg.h"
#include "wx/canvas/transdlg.h"
#include "wx/canvas/settingsdlg.h"
#include "wx/canvas/snap.h"
#include "wx/canvas/canorderdlg.h"
#include "wx/canvas/execdlg.h"
#include "wx/canvas/connectgen.h"
#include "wx/canvas/cameleon.h"
#include "wx/canvas/camcreate.h"

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

const a2dPrintWhat a2dPRINT_PrintDrawing = 20;      //!< print a2dDrawing
const a2dPrintWhat a2dPRINT_PreviewDrawing = 21;    //!< preview print a2dDrawing
const a2dPrintWhat a2dPRINT_PrintDrawingPart = 22;      //!< print a2dDrawing Part
const a2dPrintWhat a2dPRINT_PreviewDrawingPart = 23;    //!< preview print a2dDrawing Part



//----------------------------------------------------------------------------
// a2dDrawing
//----------------------------------------------------------------------------

bool a2dDrawing::m_saveLayersInit = true;
bool a2dDrawing::m_initLayersGlobalInit = true;

IMPLEMENT_DYNAMIC_CLASS( a2dDrawing, a2dObject )

const a2dSignal a2dDrawing::sig_layersetupChanged = wxNewId();
const a2dSignal a2dDrawing::sig_changedLayer = wxNewId();

DEFINE_EVENT_TYPE( wxEVT_UPDATE_DRAWING )
DEFINE_EVENT_TYPE( wxEVT_CHANGEDMODIFY_DRAWING )

//DEFINE_EVTIDFUNC( wxEVT_CHANGEDMODIFY_DRAWING )

BEGIN_EVENT_TABLE( a2dDrawing, a2dObject )
    EVT_IDLE( a2dDrawing::OnIdle )
    EVT_COM_EVENT( a2dDrawing::OnComEvent )
    EVT_NEW_CAMELEON( a2dDrawing::OnAddCameleon )  
    EVT_DO( a2dDrawing::OnDoEvent )
    EVT_UNDO( a2dDrawing::OnUndoEvent )
    EVT_REDO( a2dDrawing::OnRedoEvent )
    EVT_CHANGEDMODIFY_DRAWING( a2dDrawing::OnChangeDrawings )
END_EVENT_TABLE()


void a2dDrawing::OnDoEvent( a2dCommandProcessorEvent& event )
{
    if ( event.GetCommand()->Modifies() )
        Modify( true );
}

void a2dDrawing::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    if ( event.GetCommand()->Modifies() )
        Modify( true );
}

void a2dDrawing::OnUndoEvent( a2dCommandProcessorEvent& event )
{
    if ( event.GetCommand()->Modifies() )
        Modify( false );
}

void a2dDrawing::OnChangeDrawings( a2dDrawingEvent& event )
{
    event.Skip();
}

void a2dDrawing::OnAddCameleon( a2dCameleonEvent& event )
{
    if ( GetDrawingId() == a2dDrawingId_cameleonrefs() )
    {
        //a2dPoint2D pos = a2dCameleon::GetSuitblePoint();
        //SetPosXyPoint( pos );
        //m_rootObject->Append( this );
    }
}

void a2dDrawing::Modify( bool increment )
{
    if ( increment )
        m_documentModified++;
    else
        m_documentModified--;

    a2dDrawingEvent event( 0, wxEVT_CHANGEDMODIFY_DRAWING, this );
    event.SetEventObject( this );
    event.SetModified( increment ? +1 : -1 );
    if ( !ProcessEvent( event ) )
    {
        //parent is processed in ProcessEvent() already
    }
}

a2dDrawing::a2dDrawing( const wxString& name, a2dHabitat* habitat )
    : a2dObject()
{
	a2dDrawingId_noId();
	a2dDrawingId_cameleonrefs();
	a2dDrawingId_normal();
	a2dDrawingId_physical();
	a2dDrawingId_programming();
	a2dDrawingId_appearances();


    if ( habitat )
		m_habitat = habitat;
    else
        m_habitat = a2dCanvasGlobals->GetHabitat();
    m_parent = NULL;
    m_ignorePendingObjects = false;
    m_updatespending = false;

	m_saveLayers = m_saveLayersInit;
	m_initLayersGlobal = m_initLayersGlobalInit;

	if ( m_habitat && m_habitat->GetLayerSetup() )
        m_layersetup = wxStaticCastNull( m_habitat->GetLayerSetup()->Clone( clone_deep ), a2dLayers );
	else if ( a2dCanvasGlobals->GetHabitat()->GetLayerSetup() && m_initLayersGlobal )
        m_layersetup = wxStaticCastNull( a2dCanvasGlobals->GetHabitat()->GetLayerSetup()->Clone( clone_deep ), a2dLayers );

    m_initialSizeX = 100;
    m_initialSizeY = 100;
    m_units_accuracy = m_habitat->GetUnitsAccuracy();
    //m_units_scale = 1.0;
    m_units_scale = m_habitat->GetDefaultUnitsScale();
    m_normalize_scale = m_habitat->GetNormalizeScale();

    m_units = m_habitat->GetUnits();
    m_units_measure = m_habitat->GetUnitsMeasure();

    m_documentModified = 0;
    m_drawingId = a2dDrawingId_normal();
	m_rootObject = new a2dCanvasObject();
	m_rootObject->Update( a2dCanvasObject::updatemask_force );
    //m_rootObject contains object on layers, but is itself just an abstract object without layer
    m_rootObject->SetIgnoreLayer( true );
    m_saveStartObject = m_rootObject;
	SetRootRecursive();

    m_mayEdit = true;

    // now we have a root object in drawing, and we can create the command processor.
    CreateCommandProcessor();

    TriggerChangedTime();
    m_lastCheck = wxDateTime::Now();
    SetName( name );
}

a2dDrawing::a2dDrawing( const a2dDrawing& other, CloneOptions options, a2dRefMap* refs )
    : a2dObject( other, options, refs )
{
    m_ignorePendingObjects = false;
    m_updatespending = false;
    m_habitat = other.m_habitat;
    m_initialSizeX = other.m_initialSizeX;
    m_initialSizeY = other.m_initialSizeY;
    m_units_accuracy = other.m_units_accuracy;
    m_units = other.m_units;
    m_units_scale = other.m_units_scale;
    m_normalize_scale = other.m_normalize_scale;
    m_units_measure = other.m_units_measure;
    m_drawingId = other.m_drawingId;
    m_parent = other.m_parent;
    m_mayEdit = other.m_mayEdit;
	m_saveLayers = other.m_saveLayers;
	m_initLayersGlobal = other.m_initLayersGlobal;
    TriggerChangedTime();
    m_lastCheck = wxDateTime::Now();

    if ( other.m_layersetup )
        m_layersetup = wxStaticCastNull( other.m_layersetup->Clone( clone_deep ), a2dLayers );
    else 
        m_layersetup = NULL;

    m_documentModified = 0;
    if ( options & clone_members && other.m_rootObject )
    {
		if ( !refs )
		{
			a2dRefMap refs;
			m_rootObject = other.m_rootObject->TClone( CloneOptions( clone_resetRoot | (options & ~ clone_seteditcopy) ), &refs );
	        refs.LinkReferences();
		}
		else
		{
			m_rootObject = other.m_rootObject->TClone( CloneOptions( clone_resetRoot | (options & ~ clone_seteditcopy) ), refs );
	        refs->LinkReferences( true );
		}
    }
    else
        m_rootObject = other.m_rootObject;
    SetRootRecursive();
	m_rootObject->Update( a2dCanvasObject::updatemask_force );
    m_saveStartObject = m_rootObject;

    CreateCommandProcessor();

    TriggerChangedTime();
    m_lastCheck = wxDateTime::Now();
}

void a2dDrawing::AssignContent( const a2dDrawing& other )
{
    //clone content and relink connected objects.
    a2dRefMap refs;
    m_rootObject = other.m_rootObject->TClone( clone_resetRoot | clone_deep | clone_noCameleonRef | clone_sameName | clone_noReference, &refs );
    refs.LinkReferences();
	SetRootRecursive();
}

a2dDrawing::~a2dDrawing()
{
	m_commandProcessor = NULL;
}

a2dObject* a2dDrawing::SmrtPtrOwn()
{
    m_refcount++;
    return this;
}

bool a2dDrawing::SmrtPtrRelease()
{
    m_refcount--;

    wxASSERT_MSG( m_refcount >= 0, wxT( "a2dDrawing::Release Negative reference count" ) );
    if ( m_refcount <= 0 )
    {
        delete this;
        return true;
    }
    return false;
}

void a2dDrawing::SetLayerSetup( a2dLayers* layersetup )
{
	if ( layersetup ) 
		m_layersetup = layersetup;
	else if ( m_habitat && m_habitat->GetLayerSetup() )
        m_layersetup = wxStaticCastNull( m_habitat->GetLayerSetup()->Clone( clone_deep ), a2dLayers );
	else if ( a2dCanvasGlobals->GetHabitat()->GetLayerSetup() && m_initLayersGlobal )
        m_layersetup = wxStaticCastNull( a2dCanvasGlobals->GetHabitat()->GetLayerSetup()->Clone( clone_deep ), a2dLayers );
    else 
        m_layersetup = NULL;

    if ( m_layersetup )
        m_layersetup->SetRoot( this );

    a2dComEvent changedlayers( this, m_layersetup, sig_layersetupChanged );
    ProcessEvent( changedlayers );
};

bool a2dDrawing::ProcessEvent( wxEvent& event )
{
    bool processed = false;

    // An event handler can be enabled or disabled
    if ( GetEvtHandlerEnabled() )
    {
        //prevent the object from being deleted until this process event is at an end
        a2dREFOBJECTPTR_KEEPALIVE;

        if ( m_parent )
            processed = m_parent->ProcessEvent( event );

        event.Skip(); 
        processed = a2dObject::ProcessEvent( event );

/*
        if ( event.GetEventType() == wxEVT_UPDATE_DRAWING )
        {
            if ( m_parent )
                processed = m_parent->ProcessEvent( event );

            event.Skip(); 
            processed = a2dObject::ProcessEvent( event );
        }
        else
        {
            event.Skip(); //skipped because not processed so prepare for view
            processed = a2dObject::ProcessEvent( event );
        }
*/
    }
    return processed;
}

void a2dDrawing::DoAddPending( a2dIterC& ic )
{
    if ( m_rootObject )
        m_rootObject->AddPending( ic );
}

bool a2dDrawing::LinkReference( a2dObject* other )
{
    if( !other )
        return false;

#if defined(_DEBUG)
    wxString name = other->GetName();
#endif

    if ( m_rootObject.Get() )
        return false;

    m_rootObject = wxStaticCast( other, a2dCanvasObject );
    m_rootObject->SetCheck( true );
    return true;
}


a2dObject* a2dDrawing::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dDrawing( *this, options, refs );
}

void a2dDrawing::SetAvailable()
{
    if ( m_layersetup )
        m_layersetup->SetAvailable( this );
}

void a2dDrawing::SetRootObject( a2dCanvasObject* newRoot )
{
    wxASSERT_MSG( newRoot, wxT( "a2dDrawing its root object can not be NULL" ) );
    m_rootObject = newRoot;
    SetRootRecursive();
}

a2dCanvasCommandProcessor* a2dDrawing::GetCanvasCommandProcessor()
{
    return wxStaticCastNull( GetCommandProcessor(), a2dCanvasCommandProcessor );
}

void a2dDrawing::CreateCommandProcessor()
{
    m_commandProcessor = new a2dCanvasCommandProcessor( this );
    m_commandProcessor->ConnectEvent( wxEVT_DO, this );
    m_commandProcessor->ConnectEvent( wxEVT_UNDO, this );
    m_commandProcessor->ConnectEvent( wxEVT_REDO, this );
    m_commandProcessor->ConnectEvent( wxEVT_CANUNDO, this );
    m_commandProcessor->ConnectEvent( wxEVT_CANREDO, this );
    m_commandProcessor->ConnectEvent( wxEVT_MENUSTRINGS, this );
}

void a2dDrawing::DependencyPending()
{
    if ( !m_ignorePendingObjects )
    {
        bool old = m_updatespending;
        //continue calling DependencyPending until no more new pendingobjects are generated.
        while ( m_updatespending )
        {
            m_updatespending = false;
            a2dWalker_CallMemberFuncW walker;
            walker.Start( m_rootObject, &a2dCanvasObject::DependencyPending );
        }
        m_updatespending = old;
    }
}

void a2dDrawing::SetUpdatesPending( bool onoff )
{
    if ( !m_ignorePendingObjects )
    {
        //TriggerChangedTime();
        if ( !onoff )
        {
            a2dWalker_ResetPendingFlags setflags;
            setflags.SetSkipNotRenderedInDrawing( true );
            setflags.Start( m_rootObject, false );

            // although the layer setup is not displayed directly, and therefore its pending
            // flag is not of interest for that, it still needs a reset.
            setflags.Start( m_layersetup, false );
        }
        m_updatespending = onoff;
        //if ( onoff )
        //    wxLogDebug("Update on drawing %p\n", this );
    }
}

bool a2dDrawing::GetUpdatesPending()
{
    if ( m_ignorePendingObjects )
        return false;

    return m_updatespending;
}

void a2dDrawing::SetIgnorePendingObjects( bool onoff )
{
    m_ignorePendingObjects = onoff;
}

void a2dDrawing::Append( a2dCanvasObject* obj )
{ 
	m_rootObject->Append( obj ); 
	
	a2dCameleonInst* camInst;
	if ( camInst = wxDynamicCast( obj, a2dCameleonInst ) )
	{
		a2dCameleonEvent eventNewCameleon( this, camInst->GetCameleon() );
		ProcessEvent( eventNewCameleon );
	}
	a2dCameleon* cam;
	if ( cam = wxDynamicCast( obj, a2dCameleon ) )
	{
		a2dCameleonEvent eventNewCameleon( this, cam );
		ProcessEvent( eventNewCameleon );
	}
}

void a2dDrawing::Prepend( a2dCanvasObject* obj ) 
{ 
	m_rootObject->Prepend( obj ); 

	a2dCameleonInst* camInst;
	if ( camInst = wxDynamicCast( obj, a2dCameleonInst ) )
	{
		a2dCameleonEvent eventNewCameleon( this, camInst->GetCameleon() );
		ProcessEvent( eventNewCameleon );
	}
	a2dCameleon* cam;
	if ( cam = wxDynamicCast( obj, a2dCameleon ) )
	{
		a2dCameleonEvent eventNewCameleon( this, cam );
		ProcessEvent( eventNewCameleon );
	}
} 

void a2dDrawing::UpdateAllViews( unsigned int hint, wxObject* objecthint )
{
    //in case of a2dCANVIEW_UPDATE_ALL, first clear all pending objects, they are not of use any more
    //since all drawers will be updated/redrawn totally, no matter what.
    if ( ( hint & a2dCANVIEW_UPDATE_ALL ) && GetUpdatesPending() )
    {
        a2dDrawingEvent event( a2dCANVIEW_UPDATE_VIEWDEPENDENT, this );
        ProcessEvent( event );

        //  only invalid need recalculation so they will be rendered.
        m_rootObject->Update( a2dCanvasObject::updatemask_normal );
        //tell root there or no more pending objects.
        SetUpdatesPending( false );
    }

    // and now process the asked hint
    a2dDrawingEvent event( hint, this );
    ProcessEvent( event );
}

a2dCanvasObject* a2dDrawing::PushInto( a2dCanvasObject* parent )
{
    if ( m_rootObject )
        return m_rootObject;
    return NULL;
}

void a2dDrawing::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
	handler.WalkTask( parent, this, a2dWalker_a2dDrawingStart );

    if ( wxDynamicCast( &handler, a2dWalker_SetRoot ) )
    {
        return;
    }
    else
    {
        //when setting availability of layers, layers setup itself in drawing is skipped.
        if ( m_layersetup && ! wxDynamicCast( &handler, a2dWalker_SetAvailable ) )
            m_layersetup->Walker( this, handler );

        if ( m_rootObject )
            m_rootObject->Walker( this, handler );
    }
    a2dObject::DoWalker( parent, handler );

	handler.WalkTask( parent, this, a2dWalker_a2dDrawingEnd );
}
/*
bool a2dDrawing::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( m_rootObject )
    {
        if ( ( mode & updatemask_countObjects ) && m_layersetup )
        {
            wxUint16 j;
            for ( j = 0; j < wxMAXLAYER; j++ )
            {
                m_layersetup->GetLayerIndex()[j]->SetObjectCount( 0 );
            }

                calc = m_rootObject->Update( mode );

            //m_layersetup->SetAvailable( this );

            for ( j = 0; j < wxMAXLAYER; j++ )
            {
                a2dLayerInfo* layer = m_layersetup->GetLayerIndex()[ j ];
                layer->SetAvailable( layer->GetObjectCount() != 0 );
            }
        }
        else
                calc = m_rootObject->Update( mode );

        if ( calc && m_layersetup )
        {
            m_layersetup->SetAllLayersAvailable( false );
            //this one always available because editing handles are on it.
            m_layersetup->SetAvailable( wxLAYER_DEFAULT, TRUE );
        }
    }

    if ( !m_bbox.GetValid() || calc )
    {
        m_bbox.SetValid( false );
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
    }
    return calc;
}
*/

bool a2dDrawing::AddPendingUpdatesOldNew()
{
    if ( !GetUpdatesPending() )
        return false;

    // if layers are pending by a2dLayers::SetPending(bool pending)
    // we need to redraw all views, and no need for fine tuning.
    if ( m_layersetup  && m_layersetup->GetPending() )
    {
        a2dComEvent changedlayers( this, m_layersetup, sig_layersetupChanged );
        ProcessEvent( changedlayers );

        a2dDrawingEvent eventPreViewUpdate( a2dCANVIEW_UPDATE_PENDING  | a2dCANVIEW_UPDATE_PENDING_PREVIEWUPDATE, this );
        ProcessEvent( eventPreViewUpdate );

        DependencyPending();

        m_rootObject->Update( a2dCanvasObject::updatemask_normal ); //update all objects which are pending for an update

        //tell root there or no more pending objects.
        SetUpdatesPending( false );

        UpdateAllViews( a2dCANVIEW_UPDATE_ALL );


        return true;
    }

    // if a layer info object is pending, all needs to be redrawn
    if ( m_layersetup )
    {
        for ( a2dLayerMapNr::iterator it= m_layersetup->GetLayerIndex().begin(); it!= m_layersetup->GetLayerIndex().end(); ++it)
        {
            a2dLayerInfo* layerobj = it->second;

            if ( layerobj->GetPending() )
            {
                a2dDrawingEvent eventPreViewUpdate( a2dCANVIEW_UPDATE_PENDING  | a2dCANVIEW_UPDATE_PENDING_PREVIEWUPDATE, this );
                ProcessEvent( eventPreViewUpdate );

                DependencyPending();

                m_rootObject->Update( a2dCanvasObject::updatemask_normal ); //update all objects which are pending for an update
                //tell root there or no more pending objects.
                SetUpdatesPending( false );

                UpdateAllViews( a2dCANVIEW_UPDATE_ALL );
                return true;
            }
        }
    }

    DependencyPending();

    if ( m_layersetup )
    {
        for ( a2dLayerMapNr::iterator it= m_layersetup->GetLayerIndex().begin(); it!= m_layersetup->GetLayerIndex().end(); ++it)
        {
            a2dLayerInfo* layerobj = it->second;
            layerobj->SetObjectCount( 0 );
        }
    }

    a2dDrawingEvent eventPreViewUpdate( a2dCANVIEW_UPDATE_PENDING  | a2dCANVIEW_UPDATE_PENDING_PREVIEWUPDATE, this );
    ProcessEvent( eventPreViewUpdate );

    // inform views (and other user interface objects interested),
    // that it needs to update itself from the document.
    // Old boundingbox is still intact, and can be used to find update areas of old document state
    a2dDrawingEvent eventPreUpdate( a2dCANVIEW_UPDATE_PENDING  | a2dCANVIEW_UPDATE_PENDING_PREUPDATE, this );
    // in the next a2dCANVIEW_UPDATE_VIEWDEPENDENT is not needed, since a view does not change
    // when an canvasobject changes, but the other way around only.
    ProcessEvent( eventPreUpdate );

    if ( m_layersetup )
    {
        for ( a2dLayerMapNr::iterator it= m_layersetup->GetLayerIndex().begin(); it!= m_layersetup->GetLayerIndex().end(); ++it)
        {
            a2dLayerInfo* layerobj = it->second;
            layerobj->SetPreviousObjectCount( layerobj->GetObjectCount() );
        }
    }

    m_rootObject->Update( a2dCanvasObject::updatemask_normal ); //update all objects which are pending for an update

    // inform views (and other user interface objects interested),
    // that it needs to update itself from the document.
    // New boundingbox is calculated, and can be used to find update areas of new document state
    // use the new boundingbox of pending objects for update areas
    a2dDrawingEvent eventPostUpdate( a2dCANVIEW_UPDATE_PENDING  | a2dCANVIEW_UPDATE_PENDING_POSTUPDATE, this );
    ProcessEvent( eventPostUpdate );

    if ( m_layersetup )
    {
        for ( a2dLayerMapNr::iterator it= m_layersetup->GetLayerIndex().begin(); it!= m_layersetup->GetLayerIndex().end(); ++it)
        {
            a2dLayerInfo* layerobj = it->second;
            layerobj->SetAvailable( layerobj->GetObjectCount() != 0 );
        }

        for ( a2dLayerMapNr::iterator it= m_layersetup->GetLayerIndex().begin(); it!= m_layersetup->GetLayerIndex().end(); ++it)
        {
            a2dLayerInfo* layerobj = it->second;
            //check if first object put on a layer or last removed from a layer
            if ( layerobj->StateChange() )
            {
                a2dComEvent changedlayer( this, layerobj->GetLayer(), a2dDrawing::sig_changedLayer );
                ProcessEvent( changedlayer );
            }
        }
    }
    //tell root there or no more pending objects.
    SetUpdatesPending( false );

    // all have been processed so all temporary boundingbox properties
    // which are used for mini updates can now be removed.
    // Keep in mind that the Boundingbox of a canvas object and its parents do include those
    // boundingbox properties also, else those parts would not be rendered ( due to clipping ).
    // Therefore the objects containing those properties need to be recalculated, set them pending.

    a2dWalker_RemovePropertyCandoc removep( a2dCanvasObject::PROPID_BoundingBox );
    removep.SetSkipNotRenderedInDrawing( true );
    removep.SetMakePending( true );
    removep.Start( m_rootObject );

    return true;
}

bool a2dDrawing::LoadFromFile( const wxString& filename, a2dIOHandlerStrIn* handler )
{
#if wxUSE_STD_IOSTREAM
    a2dDocumentFileInputStream stream( filename.mb_str(), wxSTD ios_base::in | wxSTD ios_base::binary );
    if ( stream.fail() || stream.bad() )
#else
    wxFileInputStream stream( filename );
    if ( !stream.IsOk() )
#endif
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for loading" ), filename.c_str() );
        return false;
    }

    if ( handler )
    {
        if ( handler->CanLoad( stream, this, GetClassInfo() )  )
        {
            if ( !handler->Load( stream, this ) )
            {
                a2dGeneralGlobals->ReportErrorF( a2dError_IOHandler, _( "template I/O handler failed loading from stream." ) );
                handler->SetLastError( a2dError_CouldNotLoad );
            }
            else
            {
                return true;
            }
        }
        else
        {
            a2dGeneralGlobals->ReportErrorF( a2dError_IOHandler, _( "template I/O handler not capable of loading this format." ) );
            handler->SetLastError( a2dError_IOHandler );
        }
    }

    return false;
}

bool a2dDrawing::SaveToFile( const wxString& filename, a2dIOHandlerStrOut* handler ) const
{
    //check flag is used for writing once an object which is referenced many times
    a2dWalker_SetCheck setp( false );
    setp.Start( const_cast<a2dDrawing*>(this) );

#if wxUSE_STD_IOSTREAM
    a2dDocumentFileOutputStream stream( filename.mb_str(), wxSTD ios_base::out | wxSTD ios_base::binary );
    if ( stream.fail() || stream.bad() )
#else
    wxFileOutputStream stream( filename );
    if ( stream.GetLastError() != wxSTREAM_NO_ERROR )
#endif
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_SaveFile, _( "Sorry, could not open file %s for saving" ), filename.c_str() );

        // Saving error
        return false;
    }

    if ( handler )
    {
        if ( handler->CanSave( this )  )
        {
            if ( handler->Save( stream, this ) )
                return true;

            a2dGeneralGlobals->ReportErrorF( a2dError_SaveFile, _( "template I/O handler not succeeded to save in this format to file %s" ), filename.c_str() );
        }
        else
            a2dGeneralGlobals->ReportErrorF( a2dError_IOHandler, _( "template I/O handler not capable of saving in this format." ) );
    }
    else
        a2dGeneralGlobals->ReportErrorF( a2dError_IOHandler, _( "template I/O handler not set in document template" ) );
    
    return false;
}


#if wxART2D_USE_CVGIO

void a2dDrawing::Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser )
{
    if ( parent )
        a2dObject::Load( parent, parser );
    else
    {
        a2dWalker_SetCheck setp( false );
        setp.Start( m_rootObject );
        setp.Start( m_layersetup );

        parser.Next();
        parser.Require( START_TAG, wxT( "cvg" ) );
        parser.Next();

		wxString version = wxT("1");

	    parser.SetFormatVersion( version );

        a2dObject::Load( parent, parser );

        parser.Require( END_TAG, wxT( "cvg" ) );
    }
}

void a2dDrawing::Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite, a2dCanvasObject* start )
{
    if ( start )
        m_saveStartObject = start;
    else
        m_saveStartObject = m_rootObject;

    Save( parent, out, towrite );
}

void a2dDrawing::Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite )
{
    if ( parent )
        a2dObject::Save( parent, out, towrite );
    else
    {
        //check flag is used for writing once an object which is referenced many times
        a2dWalker_SetCheck setp( false );
        setp.Start( this );

        out.WriteStartDocument( wxT( "1.0" ), wxT( "UTF-8" ) , true );

        out.WriteStartElementAttributes( wxT( "cvg" ) );
        out.WriteEndAttributes();
      
        a2dObjectList towrite;
        a2dObject::Save( NULL, out, &towrite );

        out.WriteEndElement(); //cvg
        out.WriteEndDocument();
    }
}

void a2dDrawing::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_rootObject )
            out.WriteAttribute( wxT( "rootobject" ), m_rootObject->GetId() );
        out.WriteAttribute( wxT( "initialSizeX" ), m_initialSizeX );
        out.WriteAttribute( wxT( "initialSizeY" ), m_initialSizeY );
        out.WriteAttribute( wxT( "units_accuracy" ), m_units_accuracy );
        out.WriteAttribute( wxT( "units_scale" ), m_units_scale );
        out.WriteAttribute( wxT( "normalize_scale" ), m_normalize_scale );
        out.WriteAttribute( wxT( "units" ), m_units );
        out.WriteAttribute( wxT( "units_measure" ), m_units_measure );
        out.WriteAttribute( wxT( "mayEdit" ), m_mayEdit );
        out.WriteAttribute( wxT( "drawingId" ), m_drawingId.m_name );
    }
    else
    {
        out.WriteElement( wxT( "title" ), GetName() );
        out.WriteElement( wxT( "desc" ), GetDescription() );

        if ( m_saveLayers && m_layersetup )
            m_layersetup->Save( parent, out, towrite );
        //if object is not saved yet, we need to save it here.

        if ( m_saveStartObject && !m_saveStartObject->GetCheck() )
        {
            //trick the system to have multiple refs on those objects, else we will not get an id attribute
            a2dCanvasObjectPtr multiRef = m_rootObject;

            out.WriteStartElement( wxT( "drawing" ) );
            m_saveStartObject->Save( this, out, towrite );
            out.WriteEndElement();
        }

    }
    a2dObject::DoSave( parent, out, xmlparts, towrite );
}

void a2dDrawing::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( parser.HasAttribute( wxT( "rootobject" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_rootObject, parser.GetAttributeValue( wxT( "rootobject" ) ) );
        }
        m_initialSizeX = parser.GetAttributeValueDouble( wxT( "initialSizeX" ), 100 );
        m_initialSizeY = parser.GetAttributeValueDouble( wxT( "initialSizeY" ), 100 );
        m_units_accuracy = parser.GetAttributeValueDouble( wxT( "units_accuracy" ), 0.001 );
        m_units_scale = parser.GetAttributeValueDouble( wxT( "units_scale" ), m_units_scale );
		//TaConvert
	    //m_units_scale = GetDrawHabitat()->GetDefaultUnitsScale();

        m_normalize_scale = parser.GetAttributeValueDouble( wxT( "normalize_scale" ), 1 );
        m_units = parser.GetAttributeValue( wxT( "units" ), "non" );
        m_units_measure = parser.GetAttributeValue( wxT( "units_measure" ), "non" );
        m_mayEdit = parser.GetAttributeValueBool( wxT( "mayEdit" ), true );
		wxString drawingIdName = parser.GetAttributeValue( wxT( "drawingId" ), a2dDrawingId_normal().m_name );
        m_drawingId = a2dDrawingId::GetItemByName( drawingIdName );
		m_habitat = a2dCanvasGlobals->GetHabitatByDrawingId( m_drawingId );
    }
    else
    {
        if ( parser.GetTagName() == wxT( "title" ) )
        {
            m_title = parser.GetContent();
            parser.Next();
            parser.Require( END_TAG, wxT( "title" ) );
            parser.Next();
        }
        if ( parser.GetTagName() == wxT( "desc" ) )
        {
            m_description = parser.GetContent();
            parser.Next();
            parser.Require( END_TAG, wxT( "desc" ) );
            parser.Next();
        }

        if ( parser.GetTagName() == wxT( "o" ) && parser.GetAttributeValue( wxT( "classname" ) ) == wxT( "a2dLayers" ) )
        {
            m_layersetup = new a2dLayers();
            m_layersetup->Load( this, parser );
            parser.ResolveOrAdd(  (a2dSmrtPtr<class a2dObject>* ) (&m_layersetup) );
            m_layersetup->GetLayerIndex();
        }
        else
            SetLayerSetup( NULL );

        if ( parser.GetTagName() == wxT( "drawing" ) )
		{
			parser.Require( START_TAG, wxT( "drawing" ) );
			parser.Next();

            m_rootObject->Load( parent, parser );
            SetRootRecursive();
			parser.Require( END_TAG, wxT( "drawing" ) );
			parser.Next();
		}
    }
    a2dObject::DoLoad( parent, parser, xmlparts );
}
#endif //wxART2D_USE_CVGIO



void a2dDrawing::OnIdle( wxIdleEvent& event )
{
    if ( m_lastCheck + wxTimeSpan ( 0, 0, 4, 0) < wxDateTime::Now() )
    {
         DependencyPending();
         m_lastCheck = wxDateTime::Now();
    }

    if ( AddPendingUpdatesOldNew() )
    {
        //here we could wait for the a2dDrawingPart::OnIdle to do the next,
        //But we are idle so why not now.
        //Still this is here for all views directly while else it may be
        //distributed in time.
        UpdateAllViews( a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );
        event.Skip();
    }
    else
        event.Skip();
}

void a2dDrawing::SetSpecificFlags( bool setOrClear, a2dCanvasObjectFlagsMask which, const wxString& classname, a2dCanvasObjectFlagsMask whichobjects )
{
    a2dWalker_SetSpecificFlagsCanvasObjects setflags( which, whichobjects );
    setflags.Start( m_rootObject, setOrClear );
}

void a2dDrawing::SetRootRecursive()
{
    m_rootObject->SetRoot( this );
    if ( m_layersetup )
        m_layersetup->SetRoot( this );
}

void a2dDrawing::OnComEvent( a2dComEvent& event )
{
    if ( GetEvtHandlerEnabled() )
    {
/*
        if ( event.GetId() == a2dDrawingPart::sm_changedLayers )
        {
            a2dDrawing* doc = wxDynamicCast( event.GetEventObject(), a2dDrawing );
            if ( doc && doc == m_layersetup->GetRoot() )
                Init( doc->GetLayerSetup() );
        }
        else if ( event.GetId() == a2dDrawing::sm_layersetupChanged )
        {
            a2dDrawing* doc = wxDynamicCast( event.GetEventObject(), a2dDrawing );
            if ( doc && doc == m_layersetup->GetRoot() )
                Init( doc->GetLayerSetup() );
        }
*/
        if ( event.GetId() == a2dLayerInfo::sig_changedLayerInfo || event.GetId() == a2dLayers::sig_visibleAll )
        {
            a2dCanvasObject* obj = wxDynamicCast( event.GetEventObject(), a2dCanvasObject );
            if ( this == obj->GetRoot() )
                m_layersetup->SetPending( true ); // triggers redraw all.
        }
    }
}

void a2dDrawing::LineUpCameleons( int rowsize )
{
    double x, y;
    x = 50;
    y = 50;
    unsigned int i = 0;
    a2dCanvasObject* root = m_rootObject;
    a2dCanvasObjectList::iterator iter = root->GetChildObjectList()->begin();
    while( iter != root->GetChildObjectList()->end() )
    {
        a2dCameleon* cameleon = wxDynamicCast( (*iter).Get(), a2dCameleon );
        if ( cameleon && !cameleon->GetRelease() )
        {
            a2dBoundingBox bbox = cameleon->GetBbox();

            if ( i %  rowsize == 0 )
            {
                x = 50;
    		    if ( cameleon->GetNextLineDirection() )
                    y = y + bbox.GetHeight() + cameleon->GetLocationDeltaY();
                else
                    y = y + bbox.GetHeight() + cameleon->GetLocationDeltaY();
            }
            else
                x = x + bbox.GetWidth() + cameleon->GetLocationDeltaX();
            i++;
            //wxLogDebug( "%f %f %s %lld", x, y, cameleon->GetName(), cameleon->GetId() );
    	    cameleon->SetPosXY( x, y );     
        }
        iter++;
    }
}

/*
a2dCanvasObject* a2dCanvasDocument::Find( a2dCanvasObject* obj )
{
    a2dWalker_FindCanvasObject finder( obj );

    if ( finder.Start( m_drawing ) )
        return obj;
    return NULL;
}

a2dCanvasObject* a2dCanvasDocument::Find(  const wxString& objectname,  const wxString& classname, wxInt64 id )
{
    a2dWalker_CollectCanvasObjects finder( a2dCanvasOFlags::ALL );
    finder.SetObjectName( objectname );
    finder.AddClassName( classname );
    finder.SetUniqueSerializationId( id );
    finder.Start( m_drawing );

    if ( finder.m_found.size() )
        return finder.m_found.front();
    return NULL;
}
*/

//----------------------------------------------------------------------------
// a2dCanvasCommandProcessor
//----------------------------------------------------------------------------

const a2dCommandId a2dCommand_SetLayerGroup::Id( wxT( "SetLayerGroup" ) );

const a2dCommandId a2dCommand_SetStroke::Id( wxT( "SetStroke" ) );
const a2dCommandId a2dCommand_SetFill::Id( wxT( "SetFill" ) );
const a2dCommandId a2dCommand_SetStrokeColour::Id( wxT( "SetStrokeColour" ) );
const a2dCommandId a2dCommand_SetFillColour::Id( wxT( "SetFillColour" ) );

const a2dCommandId a2dCommand_NewGroup::Id( wxT( "NewGroup" ) );
const a2dCommandId a2dCommand_NewPin::Id( wxT( "NewPin" ) );

const a2dCommandId a2dCommand_SetCursor::Id( wxT( "SetCursor" ) );

const a2dCommandId a2dCanvasCommandProcessor::COMID_InsertGroupRef( wxT( "InsertGroupRef" ) );

const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgLayers( wxT( "ShowDlgLayers" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgLayersDocument( wxT( "ShowDlgLayersDocument" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgLayersGlobal( wxT( "ShowDlgLayersGlobal" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgLayerOrderDocument( wxT( "ShowDlgLayerOrderDocument" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgLayerOrderGlobal( wxT( "ShowDlgLayerOrderGlobal" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgTools( wxT( "ShowDlgTools" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgStyle( wxT( "ShowDlgStyle" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgSnap( wxT( "ShowDlgSnap" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgGroups( wxT( "ShowDlgGroups" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgIdentify( wxT( "ShowDlgIdentify" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgCoordEntry( wxT( "ShowDlgCoordEntry" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgMeasure( wxT( "ShowDlgMeasure" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgStructure( wxT( "ShowDlgStructure" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgPathSettings( wxT( "ShowDlgPathSettings" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgSettings( wxT( "ShowDlgSettings" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgTrans( wxT( "ShowDlgTrans" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_ShowDlgPropEdit( wxT( "ShowDlgPropEdit" ) );


//!@}

//!@{ \ingroup menus
//----------------------------------------------------------------------------
// globals a2dMenuIdItem's
//----------------------------------------------------------------------------

DEFINE_MENU_ITEMID( CmdMenu_Drawing_FileClose, wxTRANSLATE("&Close"), wxTRANSLATE("Close current file" ) )
DEFINE_MENU_ITEMID( CmdMenu_Drawing_FileOpen, wxTRANSLATE("&Open"), wxTRANSLATE("Open existing file" ) )
DEFINE_MENU_ITEMID( CmdMenu_Drawing_FileNew, wxTRANSLATE("&New"), wxTRANSLATE("Create new file" ) )
DEFINE_MENU_ITEMID( CmdMenu_Drawing_FileSave, wxTRANSLATE("Save"), wxTRANSLATE("Save file" ) )
DEFINE_MENU_ITEMID( CmdMenu_Drawing_FileSaveAs, wxTRANSLATE("Save &As"), wxTRANSLATE("Save file with different Name" ) )

DEFINE_MENU_ITEMID( CmdMenu_ChangeCanvasObjectStyle, wxTRANSLATE("set  current object style"), wxTRANSLATE("set the style of the current edited or drawn object") )
DEFINE_MENU_ITEMID( CmdMenu_ChangeCentralStyle, wxTRANSLATE("Set Central Style"), wxTRANSLATE("Set central stored style like fill and stroke") )
DEFINE_MENU_ITEMID( CmdMenu_SetLayer, wxTRANSLATE("Set Current layer"), wxTRANSLATE("set the current layer for drawing upon") )
DEFINE_MENU_ITEMID( CmdMenu_SetTarget, wxTRANSLATE("Set Current Target layer"), wxTRANSLATE("set the current target layer for actions involving a target") )

DEFINE_MENU_ITEMID_KIND( CmdMenu_SetDrawWireFrame, wxTRANSLATE("Draw only outlines"), wxTRANSLATE("Document drawn in outlines"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_DrawGridLines, wxTRANSLATE("Show Grid as Lines"), wxTRANSLATE("Grid lines instead of points"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_DrawGridAtFront, wxTRANSLATE("Grid on top or back"), wxTRANSLATE("draw grid on top or back of drawing"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_DrawGrid, wxTRANSLATE("Show Grid"), wxTRANSLATE("Grid On Off"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_SetSnap, wxTRANSLATE("Snap wile drawing"), wxTRANSLATE("snap inside tools while drawing"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_SetYAxis, wxTRANSLATE("Set YAxis up or down"), wxTRANSLATE("Set YAxis up or down"), wxITEM_CHECK );
DEFINE_MENU_ITEMID_KIND( CmdMenu_Option_SplinePoly, wxTRANSLATE("Spline Poly"), wxTRANSLATE("draw polygons/lines splined or not"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_Option_CursorCrosshair, wxTRANSLATE("Crosshair Cursor"), wxTRANSLATE("croshair cursor used in tools"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_Option_RescaleAtReSize, wxTRANSLATE("ReScale On ReSize"), wxTRANSLATE("Set rescale after resize"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_Option_ReverseLayers, wxTRANSLATE("Reverse layers"), wxTRANSLATE("Draw layers reversed"), wxITEM_CHECK )

DEFINE_MENU_ITEMID_KIND( CmdMenu_RouteMethod_StraightEndSegment, wxTRANSLATE("Routing StraightEndSegment"), wxTRANSLATE("Set routing method to straight end segment"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_RouteMethod_StraightEndSegmentUnlessPins, wxTRANSLATE("Routing StraightEndSegmentUnlessPins"), wxTRANSLATE("Set routing method to straight end segment unless pins on it"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_RouteMethod_StraightSegment, wxTRANSLATE("Routing StraightSegment"), wxTRANSLATE("Set routing method to straight segment"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_RouteMethod_StraightSegmentUnlessPins, wxTRANSLATE("Routing StraightSegmentUnlessPins"), wxTRANSLATE("Set routing method to straight segment unless pins on it"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_RouteMethod_GridRouting, wxTRANSLATE("Routing GridRouting"), wxTRANSLATE("Set routing automatic reroute on grid"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_RouteMethod_ManhattanSegments, wxTRANSLATE("Routing Manhattan"), wxTRANSLATE("Routing With fast Manhattan lines"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_RouteMethod_ManhattanEndSegments, wxTRANSLATE("Routing End Manhattan"), wxTRANSLATE("Routing With fast Manhattan lines for moved ends"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_RouteMethod_ManhattanEndSegmentsStay, wxTRANSLATE("Keep End Manhattan"), wxTRANSLATE("Keep Manhattan lines for moved ends"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_RouteMethod_ManhattanEndSegmentsConvertAndStay, wxTRANSLATE("conevrt and keep End Manhattan"), wxTRANSLATE("Convert to and keep Manhattan lines for moved ends"), wxITEM_CHECK )

DEFINE_MENU_ITEMID( CmdMenu_RotateObject90Left, wxTRANSLATE("Rotate object 90 Left"), wxTRANSLATE("rotate object left 90 degrees") )
DEFINE_MENU_ITEMID( CmdMenu_RotateObject90Right, wxTRANSLATE("Rotate object 90 Right"), wxTRANSLATE("rotate object right -90 degrees") )

DEFINE_MENU_ITEMID( CmdMenu_SetSnapFeatures, wxTRANSLATE("n"), wxTRANSLATE("n")  )
DEFINE_MENU_ITEMID( CmdMenu_SetSnapFeature, wxTRANSLATE("n"), wxTRANSLATE("n")  )
DEFINE_MENU_ITEMID( CmdMenu_SetStroke, wxTRANSLATE("n"), wxTRANSLATE("n")  )
DEFINE_MENU_ITEMID( CmdMenu_SetFill, wxTRANSLATE("n"), wxTRANSLATE("n")  )
DEFINE_MENU_ITEMID( CmdMenu_SetStrokeColour, wxTRANSLATE("n"), wxTRANSLATE("n")  )
DEFINE_MENU_ITEMID( CmdMenu_SetFillColour, wxTRANSLATE("n"), wxTRANSLATE("n")  )
DEFINE_MENU_ITEMID( CmdMenu_SetContourWidth, wxTRANSLATE("n"), wxTRANSLATE("n")  )
DEFINE_MENU_ITEMID( CmdMenu_SetPathType, wxTRANSLATE("n"), wxTRANSLATE("n")  )
DEFINE_MENU_ITEMID( CmdMenu_SetNormalizeFactor, wxTRANSLATE("Set Normalize factor"), wxTRANSLATE("scale normalized objects by this") )

DEFINE_MENU_ITEMID( CmdMenu_ShowDiagram, wxTRANSLATE("Show diagram"), wxTRANSLATE("Show diagram of current Cameleon") )
DEFINE_MENU_ITEMID( CmdMenu_ShowSymbol, wxTRANSLATE("Show symbol"), wxTRANSLATE("Show symbol of current Cameleon") )
DEFINE_MENU_ITEMID( CmdMenu_ShowGui, wxTRANSLATE("Show Gui"), wxTRANSLATE("Show Gui of current Cameleon") )

DEFINE_MENU_ITEMID( CmdMenu_Refresh, wxTRANSLATE("refresh"), wxTRANSLATE("refresh all that is drawn on the canvas") )
DEFINE_MENU_ITEMID( CmdMenu_ZoomOut, wxTRANSLATE("Zoom out"), wxTRANSLATE("zoom out to see whole drawing at currently shown object") )
DEFINE_MENU_ITEMID( CmdMenu_ZoomOut2, wxTRANSLATE("Zoom out factor n"), wxTRANSLATE("zoom out to see factor n as much") )
DEFINE_MENU_ITEMID( CmdMenu_ZoomIn2, wxTRANSLATE("Zoom in factor n"), wxTRANSLATE("zoom in a factor of n") )
DEFINE_MENU_ITEMID( CmdMenu_ZoomOut2CondSelected, wxTRANSLATE("Zoom out factor n"), wxTRANSLATE("zoom out to see factor n as much, if selected objects use its boundingbox as start") )
DEFINE_MENU_ITEMID( CmdMenu_ZoomIn2CondSelected, wxTRANSLATE("Zoom in factor n"), wxTRANSLATE("zoom in a factor of n, if selected objects use its boundingbox as start") )
DEFINE_MENU_ITEMID( CmdMenu_EmptyShownObject, wxTRANSLATE("Empty Shown Object"), wxTRANSLATE("remove children of shown object") )
//DEFINE_MENU_ITEMID( CmdMenu_GdsIoSaveTextAsPolygon, wxTRANSLATE("Gdsii Save text as Polgyon"), wxTRANSLATE("When possible/implemented, save text as polygon in GdsII format") ), wxITEM_CHECK );
//DEFINE_MENU_ITEMID( CmdMenu_KeyIoSaveTextAsPolygon, wxTRANSLATE("Key Save text as Polgyon"), wxTRANSLATE("When possible/implemented, save text as polygon in KEY format") ), wxITEM_CHECK );
//DEFINE_MENU_ITEMID( CmdMenu_SetGdsIoSaveFromView, wxTRANSLATE("Gdsii Save from View down"), wxTRANSLATE("Structure visible and all down will be saved") ), wxITEM_CHECK );
//DEFINE_MENU_ITEMID( CmdMenu_SetKeyIoSaveFromView, wxTRANSLATE("Key Save from View down"), wxTRANSLATE("Structure visible and all down will be saved") ), wxITEM_CHECK );

DEFINE_MENU_ITEMID( CmdMenu_PushInto, wxTRANSLATE("push into selected"), wxTRANSLATE("make the object selected the visible object") )
DEFINE_MENU_ITEMID( CmdMenu_PopOut, wxTRANSLATE("pop out current to previous"), wxTRANSLATE("make the previous shown object the visible object, unless no more") )
DEFINE_MENU_ITEMID( CmdMenu_NewGroup, wxTRANSLATE("new group"), wxTRANSLATE("create new group and set it as the current to show") )
DEFINE_MENU_ITEMID( CmdMenu_NewPin, wxTRANSLATE("new pin"), wxTRANSLATE("insert new pin in group") )
DEFINE_MENU_ITEMID( CmdMenu_SaveLayersDrawing, wxTRANSLATE("SaveLayers"), wxTRANSLATE("Save layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_SaveLayersDrawingHome, wxTRANSLATE("SaveLayers Home"), wxTRANSLATE("Save layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_SaveLayersDrawingLastDir, wxTRANSLATE("SaveLayers LastDir"), wxTRANSLATE("Save layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_SaveLayersDrawingCurrentDir, wxTRANSLATE("SaveLayers Current Dir"), wxTRANSLATE("Save layers to a layer file") )

DEFINE_MENU_ITEMID( CmdMenu_LoadLayersDrawing, wxTRANSLATE("LoadLayersDrawing"), wxTRANSLATE("Load layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_LoadLayersDrawingHome, wxTRANSLATE("LoadLayers Home"), wxTRANSLATE("Load layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_LoadLayersDrawingLastDir, wxTRANSLATE("LoadLayers LastDir"), wxTRANSLATE("Load layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_LoadLayersDrawingCurrentDir, wxTRANSLATE("LoadLayers Current Dir"), wxTRANSLATE("Load layers to a layer file") )
DEFINE_MENU_ITEMID( CmdMenu_AddLayerIfMissing, wxTRANSLATE("Add a layer"), wxTRANSLATE("add a layer if missing, else nothing") )

DEFINE_MENU_ITEMID( CmdMenu_SetCursor, wxTRANSLATE("Set cursor X,y"), wxTRANSLATE("Set cursor to given position")  )
DEFINE_MENU_ITEMID( CmdMenu_SetRelativeStart, wxTRANSLATE("start measure"), wxTRANSLATE("Set start for measuring") )
DEFINE_MENU_ITEMID( CmdMenu_AddMeasurement, wxTRANSLATE("add measure point"), wxTRANSLATE("Add a point to the path to measure") )
DEFINE_MENU_ITEMID( CmdMenu_ClearMeasurements, wxTRANSLATE("clear measure point"), wxTRANSLATE("clear measurement data") )

DEFINE_MENU_ITEMID( CmdMenu_Selected_SelectAll, wxTRANSLATE("select all objects"), wxTRANSLATE("select all objects visible") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_DeSelectAll, wxTRANSLATE("deselect all objects"), wxTRANSLATE("deselect all objects visible") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_SelectAll_NoUndo, wxTRANSLATE("select all objects"), wxTRANSLATE("select all objects visible") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_DeSelectAll_NoUndo, wxTRANSLATE("deselect all objects"), wxTRANSLATE("deselect all objects visible") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Delete, wxTRANSLATE("delete selected"), wxTRANSLATE("delete selected objects") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_GroupNamed, wxTRANSLATE("group selected"), wxTRANSLATE("group selected objects") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Group, wxTRANSLATE("group selected"), wxTRANSLATE("group selected objects") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ImageToRectangles, wxTRANSLATE("selected images to rectangle"), wxTRANSLATE("selected images are converted to rectangles per pixel") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ToTop, wxTRANSLATE("selected to top"), wxTRANSLATE("selected drawn first") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ToBack, wxTRANSLATE("selected to back"), wxTRANSLATE("selected drawn last") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_SetStyle, wxTRANSLATE("set fill stroke selected"), wxTRANSLATE("selected objects get the current fill and stroke") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_SetExtStyle, wxTRANSLATE("set extended fill stroke selected"), wxTRANSLATE("selected objects get the current extended fill and stroke") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_SetTextChanges, wxTRANSLATE("set text properties extended selected"), wxTRANSLATE("set text properties via dialog on selected objects") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_SetTextChanges2, wxTRANSLATE("set text properties selected"), wxTRANSLATE("set text properties via dialog on selected objects") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ConvertToPolygonPolylinesWithArcs, wxTRANSLATE("convert shapes to polylines and polygons with Arcs"), wxTRANSLATE("selected primitive shapes will be converted to polygons/polylines if possible") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ConvertToPolygonPolylinesWithoutArcs, wxTRANSLATE("convert shapes to polylines and polygons"), wxTRANSLATE("selected primitive shapes will be converted to polygons/polylines if possible") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ConvertToPolylines, wxTRANSLATE("convert shapes to polylines"), wxTRANSLATE("selected primitive shapes will be converted to polylines if possible") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ConvertLinesArcs, wxTRANSLATE("convert shapes to lines and arcs"), wxTRANSLATE("selected primitive shapes will be converted to arcs and lines if possible") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ConvertToVPaths, wxTRANSLATE("convert shapes to vector path"), wxTRANSLATE("selected primitive shapes will be converted to vector paths if possible") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ConvertPolygonToArcs, wxTRANSLATE("detect arcs in polygons"), wxTRANSLATE("selected polygon shapes to polygons with arcs") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ConvertPolylineToArcs, wxTRANSLATE("detect arcs in polylines"), wxTRANSLATE("selected polyline shapes to polylines with arcs") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ConvertPathToPolygon, wxTRANSLATE("Path2Polygon"), wxTRANSLATE("Convert Path to polygons") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ConvertSurfaceToPolygon, wxTRANSLATE("Surface to Polygons"), wxTRANSLATE("convert selected surfaces shapes to polygons") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_ConvertPolygonToSurface, wxTRANSLATE("Polygons to  Surface"), wxTRANSLATE("convert selected polygon shapes to surfaces") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_UnGroup, wxTRANSLATE("ungroup selected one level"), wxTRANSLATE("ungroup selected one level") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_UnGroupDeep, wxTRANSLATE("ungroup selected deep"), wxTRANSLATE("ungroup selected deep") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Merge, wxTRANSLATE("MergeSelected"), wxTRANSLATE("Merge selected objects") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Offset, wxTRANSLATE("OffsetSelected"), wxTRANSLATE("Offset selected objects") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_CreateRing, wxTRANSLATE("Create Ring"), wxTRANSLATE("Ring creation operation") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_CameleonDiagram, wxTRANSLATE("selected to Cameleon + Diagram"), wxTRANSLATE("group selected objects into a Cameleon with a a2dDiagram") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_CameleonSymbol, wxTRANSLATE("selected to Cameleon + Symbol"), wxTRANSLATE("group selected objects into a Cameleon with a a2dSymbol") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_CameleonSymbolDiagram, wxTRANSLATE("selected to Cameleon diagram + Symbol"), wxTRANSLATE("group selected objects into a Cameleon a2dDiagram and generate a a2dSymbol") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_FlattenCameleon, wxTRANSLATE("selected flatten Cameleon"), wxTRANSLATE("selected objects Cameleon objects in diagram") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_CloneCameleonFromInst, wxTRANSLATE("selected Cameleon Clone"), wxTRANSLATE("selected Cameleon is Cloned to a new Cameleon") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_MoveXY, wxTRANSLATE("move selected relative X,Y"), wxTRANSLATE("move selected objects with X,Y") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_MoveLayer, wxTRANSLATE("move selected to Layer"), wxTRANSLATE("move selected objects to layer") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_CopyXY, wxTRANSLATE("copy selected relative X,Y"), wxTRANSLATE("copy selected objects with X,Y") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_CopyLayer, wxTRANSLATE("copy selected to Layer"), wxTRANSLATE("copy selected objects to layer") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Transform, wxTRANSLATE("transform selected"), wxTRANSLATE("transform selected objects") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Rotate, wxTRANSLATE("rotate selected"), wxTRANSLATE("rotate selected objects") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MinX, wxTRANSLATE("Align selected left"), wxTRANSLATE("Align selected objects left side of boundingbox") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MaxX, wxTRANSLATE("Align selected right"), wxTRANSLATE("Align selected objects right side of boundingbox") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MinY, wxTRANSLATE("Align selected bottom"), wxTRANSLATE("Align selected objects bottom side of boundingbox") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MaxY, wxTRANSLATE("Align selected top"), wxTRANSLATE("Align selected objects top side of boundingbox") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MidX, wxTRANSLATE("Align selected middle X"), wxTRANSLATE("Align selected objects middle of boundingbox in X") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MidY, wxTRANSLATE("Align selected middle Y"), wxTRANSLATE("Align selected objects middle of boundingbox in Y") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorVert, wxTRANSLATE("Mirror selected vertical"), wxTRANSLATE("Mirror selected objects vertical") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorHorz, wxTRANSLATE("Mirror selected horizontal"), wxTRANSLATE("Mirror selected objects horizontal") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorVertBbox, wxTRANSLATE("Mirror selected vertical boundingbox"), wxTRANSLATE("Mirror selected objects vertical around centre boundingbox") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorHorzBbox, wxTRANSLATE("Mirror selected horizontal boundingbox"), wxTRANSLATE("Mirror selected objects horizontal around centre boundingbox") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_DistributeVert, wxTRANSLATE("Distribute selected vertical"), wxTRANSLATE("Distribute selected objects vertical") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_DistributeHorz, wxTRANSLATE("Distribute selected horizontal"), wxTRANSLATE("Distribute selected objects horizontal") )

DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MinX_Dest, wxTRANSLATE("Align selected left destination"), wxTRANSLATE("Align selected objects left side of boundingbox destination") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MaxX_Dest, wxTRANSLATE("Align selected right destination"), wxTRANSLATE("Align selected objects right side of boundingbox destination") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MinY_Dest, wxTRANSLATE("Align selected bottom destination"), wxTRANSLATE("Align selected objects bottom side of boundingbox destination") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MaxY_Dest, wxTRANSLATE("Align selected top destination"), wxTRANSLATE("Align selected objects top side of boundingbox destination") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MidX_Dest, wxTRANSLATE("Align selected middle X destination"), wxTRANSLATE("Align selected objects middle of boundingbox in X destination") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MidY_Dest, wxTRANSLATE("Align selected middle Y destination"), wxTRANSLATE("Align selected objects middle of boundingbox in Y destination") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorVert_Dest, wxTRANSLATE("Mirror selected vertical destination"), wxTRANSLATE("Mirror selected objects vertical destination") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorHorz_Dest, wxTRANSLATE("Mirror selected horizontal destination"), wxTRANSLATE("Mirror selected objects horizontal destination") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorVertBbox_Dest, wxTRANSLATE("Mirror selected vertical boundingbox destination"), wxTRANSLATE("Mirror selected objects vertical around centre boundingbox destination") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_Align_MirrorHorzBbox_Dest, wxTRANSLATE("Mirror selected horizontal boundingbox destination"), wxTRANSLATE("Mirror selected objects horizontal around centre boundingbox destination") )

DEFINE_MENU_ITEMID( CmdMenu_Selected_RotateObject90Left, wxTRANSLATE("Rotate selected 90 Left"), wxTRANSLATE("rotate selected left 90 degrees") )
DEFINE_MENU_ITEMID( CmdMenu_Selected_RotateObject90Right, wxTRANSLATE("Rotate selected 90 Right"), wxTRANSLATE("rotate selected right -90 degrees") )


DEFINE_MENU_ITEMID( CmdMenu_GroupAB_Offset, wxTRANSLATE("Offset"), wxTRANSLATE("Boolean Offset operation") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_Smooth, wxTRANSLATE("Smooth"), wxTRANSLATE("Smooth operation") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_CreateRing, wxTRANSLATE("Create Ring"), wxTRANSLATE("Ring creation operation") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_Delete, wxTRANSLATE("delete groupA"), wxTRANSLATE("delete groupA Objects") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_Move, wxTRANSLATE("move groupA"), wxTRANSLATE("move groupA Objects") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_Copy, wxTRANSLATE("copy groupA"), wxTRANSLATE("copy groupA Objects") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_ToArcs, wxTRANSLATE("toArcs groupA"), wxTRANSLATE("convert lines to arcs if possible in groupA Objects") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_Or, wxTRANSLATE("Boolean Or"), wxTRANSLATE("Boolean Or operation group A-B") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_And, wxTRANSLATE("Boolean And"), wxTRANSLATE("Boolean And operation group A-B") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_Exor, wxTRANSLATE("Boolean Exor"), wxTRANSLATE("Boolean Exor operation group A-B") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_AsubB, wxTRANSLATE("Boolean A-B"), wxTRANSLATE("Boolean Subtract operation group A-B") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_BsubA, wxTRANSLATE("Boolean B-A"), wxTRANSLATE("Boolean Subtract operation group A-B") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_ConvertSurfaceToPolygon, wxTRANSLATE("Surface to Polygons"), wxTRANSLATE("convert GroupA surfaces shapes to polygons") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_ConvertPolygonToSurface, wxTRANSLATE("Polygons to  Surface"), wxTRANSLATE("convert GroupA polygon shapes to surfaces") )
DEFINE_MENU_ITEMID( CmdMenu_GroupAB_ConvertPointsAtDistance, wxTRANSLATE("Polygons to points"), wxTRANSLATE("convert GroupA polygon shapes to polygons with points") )

DEFINE_MENU_ITEMID( CmdMenu_PopTool, wxTRANSLATE("Pop tool"), wxTRANSLATE("Pop the last pushed tool from the tool stack")  )


DEFINE_MENU_ITEMID( CmdMenu_PushTool_RenderImageZoom_OneShot, wxTRANSLATE("n"), wxTRANSLATE("n")  )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Zoom_OneShot, wxTRANSLATE("Zoom in and out"), wxTRANSLATE("Zoom in to the regtangle dragged") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Select_OneShot, wxTRANSLATE("Select Objects"), wxTRANSLATE("select object using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Select2_OneShot, wxTRANSLATE("Select Objects 2"), wxTRANSLATE("select object using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_RecursiveEdit_OneShot, wxTRANSLATE("Edit"), wxTRANSLATE("Edit a objects using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_MultiEdit_OneShot, wxTRANSLATE("Edit Matrix"), wxTRANSLATE("Edit a matrix of objects using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Drag_OneShot, wxTRANSLATE("Drag Objects"), wxTRANSLATE("Drag objects using mouse also (Ctrl Mouse LD)") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DragOriginal_OneShot, wxTRANSLATE("Drag Original Objects"), wxTRANSLATE("Drag original objects using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Copy_OneShot, wxTRANSLATE("Copy Objects"), wxTRANSLATE("Copy objects using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Rotate_OneShot, wxTRANSLATE("Rotate Objects"), wxTRANSLATE("Rotate objects using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Delete_OneShot, wxTRANSLATE("Delete Objects"), wxTRANSLATE("Delete objects using mouse also (Shift Mouse LD)") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawText_OneShot, wxTRANSLATE("Insert Text"), wxTRANSLATE("insert text") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Image_OneShot, wxTRANSLATE("Insert Image (linked)"), wxTRANSLATE("insert a2dImage in dragged rectangle, from a file") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Image_Embedded_OneShot, wxTRANSLATE("Insert Image (embedded)"), wxTRANSLATE("insert a2dImage in dragged rectangle, from a file") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawRectangle_OneShot, wxTRANSLATE("Draw Rectangle"), wxTRANSLATE("draw rectangle") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawCircle_OneShot, wxTRANSLATE("Draw Circle"), wxTRANSLATE("draw Circle") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawLine_OneShot, wxTRANSLATE("Draw line with ends"), wxTRANSLATE("add a line with ends using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawLineScaledArrow_OneShot, wxTRANSLATE("Draw line with arrows"), wxTRANSLATE("add a line with arrows using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawEllipse_OneShot, wxTRANSLATE("Draw Ellipse"), wxTRANSLATE("add a ellipse using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawEllipticArc_OneShot, wxTRANSLATE("Draw Elliptic Arc"), wxTRANSLATE("add a Elliptic Arc using mouse") )

DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawArc_OneShot, wxTRANSLATE("Draw Arc"), wxTRANSLATE("add a arc using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawArc_Chord_OneShot, wxTRANSLATE("Draw Arc Chord"), wxTRANSLATE("add a arc using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawPolylineL_OneShot, wxTRANSLATE("Draw polyline"), wxTRANSLATE("add a polyline using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawPolylineL_Splined_OneShot, wxTRANSLATE("Draw splined polyline"), wxTRANSLATE("add a splined polyline using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawPolygonL_OneShot, wxTRANSLATE("Draw polygon"), wxTRANSLATE("add a polygon using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawPolygonL_Splined_OneShot, wxTRANSLATE("Draw splined polygon"), wxTRANSLATE("add a splined polygon using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Property_OneShot, wxTRANSLATE("Edit Properties"), wxTRANSLATE("edit the properties of objects") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DragMulti_OneShot, wxTRANSLATE("Drag Selected Objects"), wxTRANSLATE("select object using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_CopyMulti_OneShot, wxTRANSLATE("Copy Selected Objects"), wxTRANSLATE("copy selected objects") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Measure_OneShot, wxTRANSLATE("Measure"), wxTRANSLATE("measure along a polyline using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawWirePolylineL_OneShot, wxTRANSLATE("Wire Tool"), wxTRANSLATE("Add wire between pins") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawVPath_OneShot, wxTRANSLATE("Draw vector path"), wxTRANSLATE("add a vector path using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_DrawEllipticArc_Chord_OneShot, wxTRANSLATE("Draw Elliptic Arc Chord"), wxTRANSLATE("add a Elliptic Arc using mouse") )
DEFINE_MENU_ITEMID( CmdMenu_PushTool_Origin_OneShot, wxTRANSLATE("Add Origin"), wxTRANSLATE("add Origin for symbol/diagram using mouse") )


DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_RenderImageZoom, wxTRANSLATE("n"), wxTRANSLATE("n") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Zoom, wxTRANSLATE("Zoom in and out"), wxTRANSLATE("Zoom in to the regtangle dragged"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Select, wxTRANSLATE("Select Objects"), wxTRANSLATE("select object using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Select2, wxTRANSLATE("Select Objects 2"), wxTRANSLATE("select object using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_RecursiveEdit, wxTRANSLATE("Edit"), wxTRANSLATE("Edit a objects using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_MultiEdit, wxTRANSLATE("Edit Matrix"), wxTRANSLATE("Edit a matrix of objects using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Drag, wxTRANSLATE("Drag Objects"), wxTRANSLATE("Drag objects using mouse also (Ctrl Mouse LD)"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DragOriginal, wxTRANSLATE("Drag Original Objects"), wxTRANSLATE("Drag original objects using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Copy, wxTRANSLATE("Copy Objects"), wxTRANSLATE("Copy objects using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Rotate, wxTRANSLATE("Rotate Objects"), wxTRANSLATE("Rotate objects using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Delete, wxTRANSLATE("Delete Objects"), wxTRANSLATE("Delete objects using mouse also (Shift Mouse LD)"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawText, wxTRANSLATE("Insert Text"), wxTRANSLATE("insert text"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Image, wxTRANSLATE("Insert Image (linked)"), wxTRANSLATE("insert a2dImage in dragged rectangle, from a file"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Image_Embedded, wxTRANSLATE("Insert Image (embedded)"), wxTRANSLATE("insert a2dImage in dragged rectangle, from a file"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawRectangle, wxTRANSLATE("Draw Rectangle"), wxTRANSLATE("draw rectangle"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawCircle, wxTRANSLATE("Draw Circle"), wxTRANSLATE("draw Circle"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawLine, wxTRANSLATE("Draw line with ends"), wxTRANSLATE("add a line with ends using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawLineScaledArrow, wxTRANSLATE("Draw line with arrows"), wxTRANSLATE("add a line with arrows using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawEllipse, wxTRANSLATE("Draw Ellipse"), wxTRANSLATE("add a ellipse using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawEllipticArc, wxTRANSLATE("Draw Elliptic Arc"), wxTRANSLATE("add a Elliptic Arc using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawEllipticArc_Chord, wxTRANSLATE("Draw Elliptic Arc Chord"), wxTRANSLATE("add a Elliptic Arc using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawArc, wxTRANSLATE("Draw Arc"), wxTRANSLATE("add a arc using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawArc_Chord, wxTRANSLATE("Draw Arc Chord"), wxTRANSLATE("add a arc using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawPolylineL, wxTRANSLATE("Draw polyline"), wxTRANSLATE("add a polyline using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawPolylineL_Splined, wxTRANSLATE("Draw splined polyline"), wxTRANSLATE("add a splined polyline using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawPolygonL, wxTRANSLATE("Draw polygon"), wxTRANSLATE("add a polygon using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawPolygonL_Splined, wxTRANSLATE("Draw splined polygon"), wxTRANSLATE("add a splined polygon using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Property, wxTRANSLATE("Edit Properties"), wxTRANSLATE("edit the properties of objects"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DragMulti, wxTRANSLATE("Drag Selected Objects"), wxTRANSLATE("select object using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_CopyMulti, wxTRANSLATE("Copy Selected Objects"), wxTRANSLATE("select object using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Measure, wxTRANSLATE("Measure"), wxTRANSLATE("measure along a polyline using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawWirePolylineL, wxTRANSLATE("Wire Tool"), wxTRANSLATE("Add wire between pins"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_DrawVPath, wxTRANSLATE("Draw vector path"), wxTRANSLATE("add a vector path using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Port, wxTRANSLATE("Add new Ports"), wxTRANSLATE("add Ports objects using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Cameleon, wxTRANSLATE("Add new Cameleons"), wxTRANSLATE("add Cameleon objects using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_Cameleon_BuildIn, wxTRANSLATE("Add new BuildIn Cameleons"), wxTRANSLATE("add _BuildIn Cameleon objects using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_CameleonInst, wxTRANSLATE("Add Cameleon Instance"), wxTRANSLATE("add Cameleon instances using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_CameleonInstDlg, wxTRANSLATE("Add Cameleon Instance from a list"), wxTRANSLATE("add Cameleon instance from a list, using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_CameleonRefDlg, wxTRANSLATE("Add Cameleon Reference from a list"), wxTRANSLATE("add Cameleon reference from a list, using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_CameleonRefDlg_Diagram, wxTRANSLATE("Add Cameleon Reference to a2dDiagram Instance from a list"), wxTRANSLATE("add Cameleon reference from a list, using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_CameleonRefDlg_Symbol, wxTRANSLATE("Add Cameleon Reference to a2dSymbol Instance from a list"), wxTRANSLATE("add Cameleon reference from a list, using mouse"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_TagGroups, wxTRANSLATE("Edit Mode Tag Groups"), wxTRANSLATE("push tool Tag Groups"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_TagGroups_WireMode, wxTRANSLATE("Edit Mode Tag Groups in Wire Mode"), wxTRANSLATE("push tool Tag Groups in its wire mode"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_TagGroups_WireMode_DlgMode, wxTRANSLATE("Edit Mode Tag Groups in Wire Mode"), wxTRANSLATE("push tool Tag Groups in its wire mode with dialog at DC"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_TagGroups_DlgMode, wxTRANSLATE("Edit Mode Tag Groups in Dlg Mode"), wxTRANSLATE("push tool Tag Groups in its dlg mode"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_PushTool_SingleClick, wxTRANSLATE("Single click Edit Mode"), wxTRANSLATE("Single click Edit Mode wxITEM_CHECK"), wxITEM_CHECK  )

DEFINE_MENU_ITEMID( CmdMenu_InsertGroupRef, wxTRANSLATE("&Insert Reference to canvas object"), wxTRANSLATE("Insert Reference to canvas object") )

DEFINE_MENU_ITEMID_KIND( CmdMenu_SetMasterTool_SelectFirst, wxTRANSLATE("Edit Mode select first"), wxTRANSLATE("set the main tool in mode zselect first"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_SetMasterTool_ZoomFirst, wxTRANSLATE("Edit Mode zoom first"), wxTRANSLATE("set the main tool in mode zoom first"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_SetMasterTool_TagGroups, wxTRANSLATE("Edit Mode Tag Groups"), wxTRANSLATE("set the main tool in mode Tag Groups"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_SetMasterTool_TagGroups_DlgMode, wxTRANSLATE("Edit Mode Tag Groups in Dlg Mode"), wxTRANSLATE("set the main tool in mode Tag Groups in its dlg mode"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_SetMasterTool_TagGroups_WireMode, wxTRANSLATE("Edit Mode Tag Groups in Wire Mode"), wxTRANSLATE("set the main tool in mode Tag Groups in its wire mode"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_SetMasterTool_TagGroups_WireMode_DlgMode, wxTRANSLATE("Edit Mode Tag Groups in Wire and Dlg Mode"), wxTRANSLATE("set the main tool in mode Tag Groups in its wire & dlg mode"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_SetMasterTool_SingleClick, wxTRANSLATE("Single Click Edit Mode "), wxTRANSLATE("set the main tool in mode single click edit mode"), wxITEM_CHECK )

DEFINE_MENU_ITEMID( CmdMenu_LineBegin, wxTRANSLATE("begin object"), wxTRANSLATE("(poly)Line tool begin Object") )
DEFINE_MENU_ITEMID( CmdMenu_LineEnd, wxTRANSLATE("end object"), wxTRANSLATE("(poly)Line tool end Object") )
DEFINE_MENU_ITEMID( CmdMenu_LineScale, wxTRANSLATE("scaling of point object"), wxTRANSLATE("(poly)Line tool begin&end  Object scalling") )

DEFINE_MENU_ITEMID( CmdMenu_SetLayerDlg, wxTRANSLATE("Set Current layer"), wxTRANSLATE("set the current layer for drawing upon") )
DEFINE_MENU_ITEMID_KIND( CmdMenu_SetLayerDlgModeless, wxTRANSLATE("layer dialog"), wxTRANSLATE("set the current layer for drawing upon"), wxITEM_CHECK );
DEFINE_MENU_ITEMID( CmdMenu_SetTargetDlg, wxTRANSLATE("Set Current Target layer"), wxTRANSLATE("set the current target layer for actions involving a target") )
DEFINE_MENU_ITEMID( CmdMenu_SetBackgroundStyle, wxTRANSLATE("Set BackGround"), wxTRANSLATE("Set Style for Background of canvas") ) 

DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgLayersDrawing, wxTRANSLATE("&Init Layers Drawing"), wxTRANSLATE("layer dialog initialized for editing layers drawing"), wxITEM_CHECK );
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgLayersGlobal, wxTRANSLATE("&Init Layer Global"), wxTRANSLATE("layer dialog initialized for editing layers global"), wxITEM_CHECK ); 
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgTools, wxTRANSLATE("&Tools Dialog"), wxTRANSLATE("tools dialog") , wxITEM_CHECK );
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgStyle, wxTRANSLATE("&Style Dialog"), wxTRANSLATE("style dialog") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgSnap, wxTRANSLATE("&Snap Settings Dialog"), wxTRANSLATE("Snap Settings dialog") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgGroups, wxTRANSLATE("Set Groups Dialog"), wxTRANSLATE("To set groups of layers for use in boolean operations etc.") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgIdentify, wxTRANSLATE("&Identify Dialog"), wxTRANSLATE("Identify selected objects") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgCoordEntry, wxTRANSLATE("&Coordinate Entry Dialog"), wxTRANSLATE("Coordinate Entry Dialog"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgMeasure, wxTRANSLATE("&Measure Dialog"), wxTRANSLATE("Measure Dialog"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgStructure, wxTRANSLATE("&Structure Dialog"), wxTRANSLATE("shows structure dialog") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgCameleonModal, wxTRANSLATE("&Cameleon Dialog"), wxTRANSLATE("shows Cameleon dialog") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgStructureModal, wxTRANSLATE("&Structure Dialog"), wxTRANSLATE("shows structure dialog modal") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgPathSettings, wxTRANSLATE("&Path Dialog"), wxTRANSLATE("path settings dialog"), wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgSettings, wxTRANSLATE("&Settings Dialog"), wxTRANSLATE("settings dialog") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgTrans, wxTRANSLATE("&Transformation Dialog"), wxTRANSLATE("transform selected dialog") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgPropEdit, wxTRANSLATE("&Property Edit Dialog"), wxTRANSLATE("property edit dialog") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgLayerOrderDrawing, wxTRANSLATE("&Order Layers Document"), wxTRANSLATE("order layer dialog initialized for layers drawing") , wxITEM_CHECK )
DEFINE_MENU_ITEMID_KIND( CmdMenu_ShowDlgLayerOrderGlobal, wxTRANSLATE("&Order Layers Global"), wxTRANSLATE("order layer dialog initialized for layers global") , wxITEM_CHECK )

DEFINE_MENU_ITEMID( CmdMenu_SetFont, wxTRANSLATE("Set Current font"), wxTRANSLATE("used in a2dText object" ) )

DEFINE_MENU_ITEMID( CmdMenu_PrintDrawing, wxTRANSLATE("Print Drawing"), wxTRANSLATE("Print Drawing" ) )
DEFINE_MENU_ITEMID( CmdMenu_PreviewDrawing, wxTRANSLATE("Preview Drawing"), wxTRANSLATE("Preview Print Drawing" ) )
DEFINE_MENU_ITEMID( CmdMenu_PrintDrawingPart, wxTRANSLATE("Print Drawing Part"), wxTRANSLATE("Print Drawing Part" ) )
DEFINE_MENU_ITEMID( CmdMenu_PreviewDrawingPart, wxTRANSLATE("Preview Drawing Part"), wxTRANSLATE("Preview Print Drawing Part" ) )

//!@}

const a2dCommandId a2dCommand_GroupAB::Id( wxT( "GroupAB" ) );
const a2dCommandId a2dCommand_SetLayerMask::Id( wxT( "SetLayerMask" ) );
const a2dCommandId a2dCommand_DeleteMask::Id( wxT( "DeleteMask" ) );
const a2dCommandId a2dCommand_ToTopMask::Id( wxT( "ToTopMask" ) );
const a2dCommandId a2dCommand_ToBackMask::Id( wxT( "ToBackMask" ) );
const a2dCommandId a2dCommand_CopyMask::Id( wxT( "CopyMask" ) );
const a2dCommandId a2dCommand_MoveMask::Id( wxT( "MoveMask" ) );
const a2dCommandId a2dCommand_ConvertMask::Id( wxT( "ConvertMask" ) );
const a2dCommandId a2dCommand_GroupMask::Id( wxT( "GroupMask" ) );
const a2dCommandId a2dCommand_UnGroupMask::Id( wxT( "UnGroupMask" ) );
const a2dCommandId a2dCommand_MergeMask::Id( wxT( "MergeMask" ) );
const a2dCommandId a2dCommand_PolygonToSurfaceMask::Id( wxT( "PolygonToSurfaceMask" ) );
const a2dCommandId a2dCommand_SurfaceToPolygonMask::Id( wxT( "SurfaceToPolygonMask" ) );
const a2dCommandId a2dCommand_OffsetMask::Id( wxT( "OffsetMask" ) );
const a2dCommandId a2dCommand_CreateRingMask::Id( wxT( "CreateRing" ) );
const a2dCommandId a2dCommand_UrlOnMask::Id( wxT( "UrlOnMask" ) );
const a2dCommandId a2dCommand_SetFillStrokeMask::Id( wxT( "SetFillStrokeMask" ) );
const a2dCommandId a2dCommand_ImagesToRectMask::Id( wxT( "ImagesToRect" ) );
const a2dCommandId a2dCommand_SetLayersProperty::Id( wxT( "SetLayersProperty" ) );
const a2dCommandId a2dCommand_SetShowObject::Id( wxT( "SetShowObject" ) );
const a2dCommandId a2dCanvasCommandProcessor::COMID_PopTool( wxT( "PopTool" ) ); 

const a2dCommandId a2dCommand_PushInto::Id( wxT( "PushInto" ) );
const a2dCommandId a2dCommand_AddObject::Id( wxT( "AddObject" ) );
const a2dCommandId a2dCommand_ReleaseObject::Id( wxT( "ReleaseObject" ) );
const a2dCommandId a2dCommand_ReplaceObject::Id( wxT( "ReplaceObject" ) );
const a2dCommandId a2dCommand_MoveObject::Id( wxT( "MoveObject" ) );
const a2dCommandId a2dCommand_SetFlags::Id( wxT( "SetFlags" ) );
const a2dCommandId a2dCommand_SetFlag::Id( wxT( "SetFlag" ) );
const a2dCommandId a2dCommand_SetSelectFlag::Id( wxT( "SetSelectFlag" ) );
const a2dCommandId a2dCommand_SetSelect2Flag::Id( wxT( "SetSelect2Flag" ) );
const a2dCommandId a2dCommand_TransformMask::Id( wxT( "SetTransformFlaged" ) );
const a2dCommandId a2dCommand_RotateMask::Id( wxT( "RotateFlaged" ) );
const a2dCommandId a2dCommand_AlignMask::Id( wxT( "AlignFlaged" ) );
const a2dCommandId a2dCommand_AddPoint::Id( wxT( "AddPoint" ) );
const a2dCommandId a2dCommand_MoveSegment::Id( wxT( "MoveSegment" ) );
const a2dCommandId a2dCommand_MovePoint::Id( wxT( "MovePoint" ) );
const a2dCommandId a2dCommand_MoveMidSegment::Id( wxT( "MoveMidSegment" ) );
const a2dCommandId a2dCommand_RemoveSegment::Id( wxT( "RemoveSegment" ) );
const a2dCommandId a2dCommand_SetSegments::Id( wxT( "SetSegments" ) );
const a2dCommandId a2dCommand_ChangeText::Id( wxT( "ChangeText" ) );
const a2dCommandId a2dCommand_ChangeCanvasObjectStyle::Id( wxT( "ChangeCanvasObjectStyle" ) );
const a2dCommandId a2dCommand_SetCanvasProperty::Id( wxT( "SetCanvasProperty" ) );
const a2dCommandId a2dCommand_ConnectPins::Id( wxT( "ConnectPins" ) );
const a2dCommandId a2dCommand_DisConnectPins::Id( wxT( "DisConnectPins" ) );
const a2dCommandId a2dCommand_Select::Id( wxT( "Select" ) );
const a2dCommandId a2dCommand_AddObjects::Id( wxT( "AddObjects" ) );

const a2dSignal a2dCanvasCommandProcessor::sig_ClearMeasurements = wxNewId(); 
const a2dSignal a2dCanvasCommandProcessor::sig_AddMeasurement = wxNewId(); 
const a2dSignal a2dCanvasCommandProcessor::sig_SetRelativeStart = wxNewId(); 

IMPLEMENT_CLASS( a2dCanvasCommandProcessor, a2dCommandProcessor );
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_SetLayerGroup, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_SetStroke, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_SetFill, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_SetStrokeColour, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_SetFillColour, a2dCommand )
//IMPLEMENT_DYNAMIC_CLASS( a2dCommand_ViewAsImage, a2dCommand )
//IMPLEMENT_DYNAMIC_CLASS( a2dCommand_ViewAsSvg, a2dCommand )
IMPLEMENT_DYNAMIC_CLASS( a2dCommand_SetCursor, a2dCommand )

BEGIN_EVENT_TABLE( a2dCanvasCommandProcessor, a2dCommandProcessor )

    EVT_MENU( CmdMenu_Drawing_FileClose().GetId(), a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Drawing_FileOpen().GetId(), a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Drawing_FileNew().GetId(), a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Drawing_FileSave().GetId(), a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Drawing_FileSaveAs().GetId(), a2dCanvasCommandProcessor::OnMenu )

    EVT_BEGINBUSY( a2dCanvasCommandProcessor::OnBusyBegin )
    EVT_ENDBUSY( a2dCanvasCommandProcessor::OnBusyEnd )

    EVT_MENU( CmdMenu_Selected_Merge().GetId(), a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ChangeCanvasObjectStyle().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_SetLayer().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetTarget().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetDrawWireFrame().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_DrawGridLines().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_DrawGridAtFront().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_DrawGrid().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetSnap().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetYAxis().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Option_SplinePoly().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Option_CursorCrosshair().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Option_RescaleAtReSize().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Option_ReverseLayers().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_RouteMethod_StraightEndSegment().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_RouteMethod_StraightEndSegmentUnlessPins().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_RouteMethod_StraightSegment().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_RouteMethod_StraightSegmentUnlessPins().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_RouteMethod_GridRouting().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_RouteMethod_ManhattanSegments().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_RouteMethod_ManhattanEndSegments().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_RouteMethod_ManhattanEndSegmentsStay().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_RouteMethod_ManhattanEndSegmentsConvertAndStay().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_RotateObject90Left().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_RotateObject90Right().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_SetSnapFeatures().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetSnapFeature().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetStroke().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetFill().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetStrokeColour().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetFillColour().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetContourWidth().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetPathType().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetNormalizeFactor().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_ShowDiagram().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowSymbol().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowGui().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_Refresh().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ZoomOut().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ZoomOut2().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ZoomIn2().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ZoomOut2CondSelected().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ZoomIn2CondSelected().GetId() , a2dCanvasCommandProcessor::OnMenu )
    
    EVT_MENU( CmdMenu_EmptyShownObject().GetId() , a2dCanvasCommandProcessor::OnMenu )
    //EVT_MENU( CmdMenu_GdsIoSaveTextAsPolygon().GetId() , a2dCanvasCommandProcessor::OnMenu )
    //EVT_MENU( CmdMenu_KeyIoSaveTextAsPolygon().GetId() , a2dCanvasCommandProcessor::OnMenu )
    //EVT_MENU( CmdMenu_SetGdsIoSaveFromView().GetId() , a2dCanvasCommandProcessor::OnMenu )
    //EVT_MENU( CmdMenu_SetKeyIoSaveFromView().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_PushInto().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PopOut().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_NewGroup().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_NewPin().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SaveLayersDrawing().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SaveLayersDrawingHome().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SaveLayersDrawingLastDir().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SaveLayersDrawingCurrentDir().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_LoadLayersDrawing().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_LoadLayersDrawingHome().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_LoadLayersDrawingLastDir().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_LoadLayersDrawingCurrentDir().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_AddLayerIfMissing().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_SetCursor().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetRelativeStart().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_AddMeasurement().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ClearMeasurements().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_Selected_SelectAll().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_DeSelectAll().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_SelectAll_NoUndo().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_DeSelectAll_NoUndo().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_Selected_Delete().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_GroupNamed().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Group().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ImageToRectangles().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ToTop().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ToBack().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_SetStyle().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_SetExtStyle().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_SetTextChanges().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_SetTextChanges2().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ConvertToPolygonPolylinesWithArcs().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ConvertToPolygonPolylinesWithoutArcs().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ConvertToPolylines().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ConvertLinesArcs().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ConvertToVPaths().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ConvertPolygonToArcs().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ConvertPolylineToArcs().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ConvertPathToPolygon().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ConvertPolygonToSurface().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_ConvertSurfaceToPolygon().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_UnGroup().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_UnGroupDeep().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Merge().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Offset().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_CreateRing().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_CameleonDiagram().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_CameleonSymbol().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_CameleonSymbolDiagram().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_FlattenCameleon().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_CloneCameleonFromInst().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_MoveXY().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_MoveLayer().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_CopyXY().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_CopyLayer().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Transform().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Rotate().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_Selected_Align_MinX().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MaxX().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MinY().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MaxY().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MidX().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MidY().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MirrorVert().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MirrorHorz().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MirrorVertBbox().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MirrorHorzBbox().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_DistributeVert().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_DistributeHorz().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MinX_Dest().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MaxX_Dest().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MinY_Dest().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MaxY_Dest().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MidX_Dest().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MidY_Dest().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MirrorVert_Dest().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MirrorHorz_Dest().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MirrorVertBbox_Dest().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_Align_MirrorHorzBbox_Dest().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_Selected_RotateObject90Left().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_Selected_RotateObject90Right().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_GroupAB_Offset().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_Smooth().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_CreateRing().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_Delete().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_Move().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_Copy().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_ToArcs().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_Or().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_And().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_Exor().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_AsubB().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_BsubA().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_ConvertPolygonToSurface().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_ConvertSurfaceToPolygon().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_GroupAB_ConvertPointsAtDistance().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_PushTool_Zoom_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Select_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Select2_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_RecursiveEdit_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_MultiEdit_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Drag_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DragOriginal_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Copy_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Rotate_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Delete_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawText_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Image_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Image_Embedded_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawRectangle_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawCircle_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawLine_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawLineScaledArrow_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawEllipse_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawEllipticArc_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawEllipticArc_Chord_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawArc_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawArc_Chord_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawPolylineL_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawPolylineL_Splined_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawPolygonL_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawPolygonL_Splined_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Property_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DragMulti_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_CopyMulti_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Measure_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawWirePolylineL_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawVPath_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Origin_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )


    EVT_MENU( CmdMenu_PushTool_Zoom().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Select().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Select2().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_RecursiveEdit().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_MultiEdit().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Drag().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DragOriginal().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Copy().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Rotate().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Delete().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawText().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Image().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Image_Embedded().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawRectangle().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawCircle().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawLine().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawLineScaledArrow().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawEllipse().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawEllipticArc().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawEllipticArc_Chord().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawArc().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawArc_Chord().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawPolylineL().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawPolylineL_Splined().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawPolygonL().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawPolygonL_Splined().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Property().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DragMulti().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_CopyMulti().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Measure().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawWirePolylineL().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_DrawVPath().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Port().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Cameleon().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_Cameleon_BuildIn().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_CameleonInst().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_CameleonInstDlg().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_CameleonRefDlg().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_CameleonRefDlg_Diagram().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_CameleonRefDlg_Symbol().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_TagGroups().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_TagGroups_WireMode().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_TagGroups_DlgMode().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_TagGroups_WireMode_DlgMode().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )
    EVT_MENU( CmdMenu_PushTool_SingleClick().GetId() , a2dCanvasCommandProcessor::OnPushToolMenu )

    EVT_MENU( CmdMenu_PopTool().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_InsertGroupRef().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_SetMasterTool_SelectFirst().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetMasterTool_ZoomFirst().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetMasterTool_TagGroups().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetMasterTool_TagGroups_WireMode().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetMasterTool_TagGroups_WireMode_DlgMode().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetMasterTool_TagGroups_DlgMode().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetMasterTool_SingleClick().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_LineBegin().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_LineEnd().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_LineScale().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_SetLayerDlg().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetTargetDlg().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetLayerDlgModeless().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_SetBackgroundStyle().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_PrintDrawing().GetId(), a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PreviewDrawing().GetId(), a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PrintDrawingPart().GetId(), a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_PreviewDrawingPart().GetId(), a2dCanvasCommandProcessor::OnMenu )

    EVT_UPDATE_UI( CmdMenu_SetLayerDlgModeless().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetMasterTool_SelectFirst().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetMasterTool_ZoomFirst().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetMasterTool_TagGroups().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetMasterTool_TagGroups_WireMode().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetMasterTool_TagGroups_WireMode_DlgMode().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetMasterTool_TagGroups_DlgMode().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetMasterTool_SingleClick().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    
    EVT_UPDATE_UI( CmdMenu_Drawing_FileOpen().GetId(), a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Drawing_FileClose().GetId(), a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Drawing_FileNew().GetId(), a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Drawing_FileSave().GetId(), a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Drawing_FileSaveAs().GetId(), a2dCanvasCommandProcessor::OnUpdateMenu )

    //EVT_UPDATE_UI( CmdMenu_GdsIoSaveTextAsPolygon().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    //EVT_UPDATE_UI( CmdMenu_KeyIoSaveTextAsPolygon().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    //EVT_UPDATE_UI( CmdMenu_SetGdsIoSaveFromView().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    //EVT_UPDATE_UI( CmdMenu_SetKeyIoSaveFromView().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetDrawWireFrame().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_DrawGridLines().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_DrawGridAtFront().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_DrawGrid().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetSnap().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_SetYAxis().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Option_SplinePoly().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Option_CursorCrosshair().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Option_RescaleAtReSize().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Option_ReverseLayers().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )

    EVT_UPDATE_UI( CmdMenu_RouteMethod_StraightEndSegment().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_RouteMethod_StraightEndSegmentUnlessPins().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_RouteMethod_StraightSegment().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_RouteMethod_StraightSegmentUnlessPins().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_RouteMethod_GridRouting().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_RouteMethod_ManhattanSegments().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_RouteMethod_ManhattanEndSegments().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_RouteMethod_ManhattanEndSegmentsStay().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_RouteMethod_ManhattanEndSegmentsConvertAndStay().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )

    EVT_UPDATE_UI( CmdMenu_RotateObject90Left().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_RotateObject90Right().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )

    //EVT_UPDATE_UI( wxID_UNDO, a2dCanvasCommandProcessor::OnUpdateUndo )
    //EVT_UPDATE_UI( wxID_REDO, a2dCanvasCommandProcessor::OnUpdateRedo )

    EVT_UPDATE_UI( CmdMenu_ZoomOut().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ZoomOut2().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ZoomIn2().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ZoomOut2CondSelected().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ZoomIn2CondSelected().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    
    EVT_UPDATE_UI( CmdMenu_Selected_SelectAll().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_DeSelectAll().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_SelectAll_NoUndo().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_DeSelectAll_NoUndo().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Delete().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_GroupNamed().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Group().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ImageToRectangles().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ToTop().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ToBack().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_SetStyle().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_SetExtStyle().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_SetTextChanges().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_SetTextChanges2().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ConvertToPolygonPolylinesWithArcs().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ConvertToPolygonPolylinesWithoutArcs().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ConvertToPolylines().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ConvertLinesArcs().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ConvertToVPaths().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ConvertPolygonToArcs().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ConvertPolylineToArcs().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ConvertPathToPolygon().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ConvertPolygonToSurface().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_ConvertSurfaceToPolygon().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_UnGroup().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_UnGroupDeep().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Merge().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Offset().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_CreateRing().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_CameleonDiagram().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_CameleonSymbol().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_CameleonSymbolDiagram().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_FlattenCameleon().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_CloneCameleonFromInst().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_MoveXY().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_MoveLayer().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_CopyXY().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_CopyLayer().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Transform().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Rotate().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )

    EVT_UPDATE_UI( CmdMenu_Selected_Align_MinX().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MaxX().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MinY().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MaxY().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MidX().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MidY().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MirrorVert().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MirrorHorz().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MirrorVertBbox().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MirrorHorzBbox().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_DistributeVert().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_DistributeHorz().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MinX_Dest().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MaxX_Dest().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MinY_Dest().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MaxY_Dest().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MidX_Dest().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MidY_Dest().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MirrorVert_Dest().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MirrorHorz_Dest().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MirrorVertBbox_Dest().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_Align_MirrorHorzBbox_Dest().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
	EVT_UPDATE_UI( CmdMenu_Selected_RotateObject90Left().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_Selected_RotateObject90Right().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )

    EVT_UPDATE_UI( CmdMenu_PushInto().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PopOut().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )

    EVT_UPDATE_UI( CmdMenu_ShowDiagram().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowSymbol().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowGui().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )

    EVT_MENU( CmdMenu_ShowDlgLayersDrawing().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgTools().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgStyle().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgSnap().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgGroups().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgIdentify().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgCoordEntry().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgMeasure().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgStructure().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgStructureModal().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgCameleonModal().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgPathSettings().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgSettings().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgTrans().GetId() , a2dCanvasCommandProcessor::OnMenu )
    EVT_MENU( CmdMenu_ShowDlgPropEdit().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_MENU( CmdMenu_SetFont().GetId() , a2dCanvasCommandProcessor::OnMenu )

    EVT_UPDATE_UI( CmdMenu_ShowDlgLayersDrawing().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgTools().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgStyle().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgSnap().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgGroups().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgIdentify().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgCoordEntry().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgMeasure().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgStructure().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgStructureModal().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgPathSettings().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgSettings().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgTrans().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgPropEdit().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_ShowDlgCameleonModal().GetId() , a2dCanvasCommandProcessor::OnUpdateMenu )

    EVT_UPDATE_UI( CmdMenu_PushTool_Zoom_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Select_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Select2_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_RecursiveEdit_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_MultiEdit_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Drag_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DragOriginal_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Copy_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Rotate_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Delete_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawText_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Image_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Image_Embedded_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawRectangle_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawCircle_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawLine_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawLineScaledArrow_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawEllipse_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawEllipticArc_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawEllipticArc_Chord_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawArc_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawArc_Chord_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawPolylineL_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawPolylineL_Splined_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawPolygonL_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawPolygonL_Splined_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Property_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DragMulti_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_CopyMulti_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Measure_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawWirePolylineL_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawVPath_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )

    EVT_UPDATE_UI( CmdMenu_PushTool_Zoom().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Select().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Select2().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_RecursiveEdit().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_MultiEdit().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Drag().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DragOriginal().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Copy().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Rotate().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Delete().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawText().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Image().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Image_Embedded().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawRectangle().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawCircle().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawLine().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawLineScaledArrow().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawEllipse().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawEllipticArc().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawEllipticArc_Chord().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawArc().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawArc_Chord().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawPolylineL().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawPolylineL_Splined().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawPolygonL().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawPolygonL_Splined().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Property().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DragMulti().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_CopyMulti().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Measure().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawWirePolylineL().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_DrawVPath().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Port().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Origin_OneShot().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Cameleon().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_Cameleon_BuildIn().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_CameleonInst().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_CameleonInstDlg().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_CameleonRefDlg().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_CameleonRefDlg_Diagram().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_CameleonRefDlg_Symbol().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_TagGroups().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_TagGroups_WireMode().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_TagGroups_DlgMode().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_TagGroups_WireMode_DlgMode().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )
    EVT_UPDATE_UI( CmdMenu_PushTool_SingleClick().GetId() , a2dCanvasCommandProcessor::OnPushToolUpdateMenu )


    EVT_PROPOBJECT_EDITPROPERTIES_EVENT( a2dCanvasCommandProcessor::OnEditProperties )

END_EVENT_TABLE()

ToolDlg* a2dCanvasCommandProcessor::m_tooldlg = NULL;
a2dStyleDialog* a2dCanvasCommandProcessor::m_styledlg = NULL;
LayerPropertiesDialog* a2dCanvasCommandProcessor::m_layersdlg = NULL;
a2dLayerOrderDlg* a2dCanvasCommandProcessor::m_layerOrderDlg = NULL;
IdentifyDialog* a2dCanvasCommandProcessor::m_identifyDlg = NULL;
a2dSnapSettings* a2dCanvasCommandProcessor::m_snapSettings = NULL;
a2dCoordinateEntry* a2dCanvasCommandProcessor::m_coordEntry = NULL;
a2dMeasureDlg* a2dCanvasCommandProcessor::m_measure = NULL;
a2dPathSettings* a2dCanvasCommandProcessor::m_pathsettings = NULL;
a2dSettings* a2dCanvasCommandProcessor::m_settings = NULL;
a2dTransDlg* a2dCanvasCommandProcessor::m_transset = NULL;
a2dPropertyEditorDlg* a2dCanvasCommandProcessor::m_propEdit = NULL;
a2dCanvasObjectsDialog* a2dCanvasCommandProcessor::m_structDlg = NULL;
a2dLayerDlg* a2dCanvasCommandProcessor::m_layerChooseDlg = NULL;

a2dCanvasCommandProcessor::a2dCanvasCommandProcessor( a2dDrawing* drawing, int maxCommands )
    : a2dCommandProcessor( maxCommands )
{
    m_drawing = drawing;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    CurrentSmartPointerOwner = this;
#endif
#if wxUSE_PRINTING_ARCHITECTURE
    m_pageSetupData = new wxPageSetupDialogData;
#endif

    m_parent = m_drawing->GetRootObject();

    m_AppendOrPrepend = true;

    m_currentobject = 0;

    m_withUndo = true;
	m_zoomfactor = 1.5;

    m_meas_x = m_meas_y = 0;

    m_propEdit = NULL;
    m_tooldlg = NULL;
    m_layersdlg = NULL;
    m_layerOrderDlg = NULL;
    m_layerChooseDlg = NULL;
    m_styledlg = NULL;
    m_snapSettings = NULL;
    m_identifyDlg = NULL;
    m_coordEntry = NULL;
    m_measure = NULL;
    m_pathsettings = NULL;
    m_settings = NULL;
    m_transset = NULL;
    m_structDlg = NULL;
}

a2dCanvasCommandProcessor::~a2dCanvasCommandProcessor()
{
    DeleteDlgs();
#if wxUSE_PRINTING_ARCHITECTURE
    if( m_pageSetupData )
        delete m_pageSetupData;
#endif
}

bool a2dCanvasCommandProcessor::IsShownToolDlg() { return m_tooldlg != NULL && m_tooldlg->IsShown(); }

bool a2dCanvasCommandProcessor::IsShowna2dStyleDialog() { return m_styledlg != NULL && m_styledlg->IsShown(); }

bool a2dCanvasCommandProcessor::IsShownLayerPropertiesDialog() { return m_layersdlg != NULL && m_layersdlg->IsShown(); }

bool a2dCanvasCommandProcessor::IsShownLayerOrderDialog() { return m_layerOrderDlg != NULL && m_layerOrderDlg->IsShown(); }

bool a2dCanvasCommandProcessor::IsShownIdentifyDialog() { return m_identifyDlg != NULL && m_identifyDlg->IsShown(); }

bool a2dCanvasCommandProcessor::IsShowna2dSnapSettings() { return m_snapSettings != NULL && m_snapSettings->IsShown(); }

bool a2dCanvasCommandProcessor::IsShowna2dCoordinateEntry() { return m_coordEntry != NULL && m_coordEntry->IsShown(); }

bool a2dCanvasCommandProcessor::IsShowna2dMeasureDlg() { return m_measure != NULL && m_measure->IsShown(); }

bool a2dCanvasCommandProcessor::IsShowna2dPathSettings() { return m_pathsettings != NULL && m_pathsettings->IsShown(); }

bool a2dCanvasCommandProcessor::IsShowna2dSettings() { return m_settings != NULL && m_settings->IsShown(); }

bool a2dCanvasCommandProcessor::IsShowna2dTransDlg() { return m_transset != NULL && m_transset->IsShown(); }

//bool a2dCanvasCommandProcessor::IsShownPropEditDlg() { return m_propEdit != NULL && m_propEdit->IsShown(); }

bool a2dCanvasCommandProcessor::IsShownStructureDlg() { return m_structDlg != NULL && m_structDlg->IsShown(); }

void a2dCanvasCommandProcessor::OnEditProperties( a2dPropertyEditEvent& event )
{
    static long lastProcessedEventTimeStamp = -1; // for a2d...Events
    if( lastProcessedEventTimeStamp == event.GetTimestamp() )
        return;
    lastProcessedEventTimeStamp = event.GetTimestamp();

    a2dNamedPropertyList* propertylist;
    propertylist = event.GetPropertyList();

    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawer )
        return;
/*
    a2dEditProperties dlg( NULL, ( a2dObject* ) event.GetEventObject(), propertylist );
    //a2dPropertyEditorDlg dlg( NULL, propertylist );
    if ( dlg.ShowModal() == wxID_OK )
        event.SetEdited( true );
    else
        event.SetEdited( false );
*/        
}

void a2dCanvasCommandProcessor::DeleteDlgs()
{
    if ( m_tooldlg )
        m_tooldlg->Destroy();

    if ( m_layersdlg )
        m_layersdlg->Destroy();

    if ( m_layerOrderDlg )
        m_layerOrderDlg->Destroy();

    if ( m_layerChooseDlg )
        m_layerChooseDlg->Destroy();

    if ( m_identifyDlg )
        m_identifyDlg->Destroy();

    if ( m_styledlg )
        m_styledlg->Destroy();

    if ( m_snapSettings )
        m_snapSettings->Destroy();

    if ( m_coordEntry )
        m_coordEntry->Destroy();

    if ( m_measure )
        m_measure->Destroy();

    if ( m_transset )
        m_transset->Destroy();

//    if ( m_propEdit )
//        m_propEdit->Destroy();

    if ( m_settings )
        m_settings->Destroy();

    if ( m_pathsettings )
        m_pathsettings->Destroy();

    if ( m_structDlg )
        m_structDlg->Destroy();

    m_tooldlg = NULL;
    m_layersdlg = NULL;
    m_layerOrderDlg = NULL;
    m_layerChooseDlg = NULL;
    m_styledlg = NULL;
    m_snapSettings = NULL;
    m_identifyDlg = NULL;
    m_coordEntry = NULL;
    m_measure = NULL;
    m_pathsettings = NULL;
    m_settings = NULL;
    m_transset = NULL;
    m_propEdit = NULL;
    m_structDlg = NULL;
}

bool a2dCanvasCommandProcessor::InsertGroupRef()
{
    if ( m_drawing )
    {
        double x = 0;
        double y = 0;

        a2dCanvasObjectsDialog groups( GetActiveDrawingPart()->GetDisplayWindow(), m_drawing, true, false, ( wxSTAY_ON_TOP | wxRESIZE_BORDER | wxCAPTION ) );
        if ( groups.ShowModal() == wxID_OK )
        {
            a2dCanvasObjectReference* groupref = new a2dCanvasObjectReference( x, y, groups.GetCanvasObject() );
            GetActiveDrawingPart()->GetShowObject()->Prepend( groupref );
        }
    }
    return true;
}

bool a2dCanvasCommandProcessor::StaticProcessEvent(  wxUpdateUIEvent& event  )
{
    DefaultOnUpdateMenu( event );
    return true;
}

void a2dCanvasCommandProcessor::DefaultOnUpdateMenu( wxUpdateUIEvent& event )
{
    /* debug some id
    if ( event.GetId() == CmdMenu_PushTool_Port().GetId() )
    {
		event.Enable( false );
        return;
    }
    */

    if ( event.GetId() == CmdMenu_PushTool_Zoom_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_Select_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_Select2_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_RecursiveEdit_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_MultiEdit_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DragOriginal_OneShot().GetId() || 
         event.GetId() == CmdMenu_PushTool_Drag_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_Copy_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_Rotate_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_Delete_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawText_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_Image_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_Image_Embedded_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawLine_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawLineScaledArrow_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawEllipse_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawEllipticArc_OneShot().GetId() || 
         event.GetId() == CmdMenu_PushTool_DrawEllipticArc_Chord_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawArc_OneShot().GetId() || 
         event.GetId() == CmdMenu_PushTool_DrawArc_Chord_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawPolylineL_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawPolygonL_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawPolylineL_Splined_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawPolygonL_Splined_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_Property_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_Measure_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawWirePolylineL_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DragMulti_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_CopyMulti_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawRectangle_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawCircle_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawVPath_OneShot().GetId() ||
         event.GetId() == CmdMenu_PushTool_Origin_OneShot().GetId() ||

         event.GetId() == CmdMenu_PushTool_Zoom().GetId() ||
         event.GetId() == CmdMenu_PushTool_Select().GetId() ||
         event.GetId() == CmdMenu_PushTool_Select2().GetId() ||
         event.GetId() == CmdMenu_PushTool_RecursiveEdit().GetId() ||
         event.GetId() == CmdMenu_PushTool_MultiEdit().GetId() ||
         event.GetId() == CmdMenu_PushTool_DragOriginal().GetId() ||
         event.GetId() == CmdMenu_PushTool_Drag().GetId() ||
         event.GetId() == CmdMenu_PushTool_Copy().GetId() ||
         event.GetId() == CmdMenu_PushTool_Rotate().GetId() ||
         event.GetId() == CmdMenu_PushTool_Delete().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawText().GetId() || 
         event.GetId() == CmdMenu_PushTool_Image().GetId() ||
         event.GetId() == CmdMenu_PushTool_Image_Embedded().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawLine().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawLineScaledArrow().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawEllipse().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawEllipticArc().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawEllipticArc_Chord().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawArc().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawArc_Chord().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawPolylineL().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawPolygonL().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawPolylineL_Splined().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawPolygonL_Splined().GetId() ||
         event.GetId() == CmdMenu_PushTool_Property().GetId() ||
         event.GetId() == CmdMenu_PushTool_Measure().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawWirePolylineL().GetId() ||
         event.GetId() == CmdMenu_PushTool_DragMulti().GetId() || 
         event.GetId() == CmdMenu_PushTool_CopyMulti().GetId() || 
         event.GetId() == CmdMenu_PushTool_DrawRectangle().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawCircle().GetId() ||
         event.GetId() == CmdMenu_PushTool_DrawVPath().GetId() ||
         event.GetId() == CmdMenu_PushTool_Port().GetId() ||
         event.GetId() == CmdMenu_PushTool_Cameleon().GetId() ||
         event.GetId() == CmdMenu_PushTool_Cameleon_BuildIn().GetId() ||
         event.GetId() == CmdMenu_PushTool_CameleonInst().GetId() ||
         event.GetId() == CmdMenu_PushTool_CameleonInstDlg().GetId() ||
         event.GetId() == CmdMenu_PushTool_CameleonRefDlg_Diagram().GetId() ||
         event.GetId() == CmdMenu_PushTool_CameleonRefDlg_Symbol().GetId() ||
         event.GetId() == CmdMenu_PushTool_CameleonRefDlg().GetId() ||
         event.GetId() == CmdMenu_PushTool_TagGroups().GetId() ||
         event.GetId() == CmdMenu_PushTool_TagGroups_WireMode().GetId() ||
         event.GetId() == CmdMenu_PushTool_TagGroups_DlgMode().GetId() ||
         event.GetId() == CmdMenu_PushTool_TagGroups_WireMode_DlgMode().GetId() ||
         event.GetId() == CmdMenu_PushTool_SingleClick().GetId() ||

        event.GetId() == CmdMenu_ZoomIn2().GetId() ||
        event.GetId() == CmdMenu_ZoomOut2().GetId() ||
        event.GetId() == CmdMenu_ZoomIn2CondSelected().GetId() ||
        event.GetId() == CmdMenu_ZoomOut2CondSelected().GetId() ||

        event.GetId() == CmdMenu_ZoomOut().GetId() ||
        event.GetId() == CmdMenu_Selected_RotateObject90Right().GetId() || 
        event.GetId() == CmdMenu_Selected_RotateObject90Left().GetId() ||       
        event.GetId() == CmdMenu_Selected_SelectAll().GetId() ||
        event.GetId() == CmdMenu_Selected_DeSelectAll().GetId() ||
        event.GetId() == CmdMenu_Selected_SelectAll_NoUndo().GetId() ||
        event.GetId() == CmdMenu_Selected_DeSelectAll_NoUndo().GetId() ||
        event.GetId() == CmdMenu_Selected_Delete().GetId() ||
        event.GetId() == CmdMenu_Selected_GroupNamed().GetId() ||
        event.GetId() == CmdMenu_Selected_Group().GetId() ||
        event.GetId() == CmdMenu_Selected_ImageToRectangles().GetId() ||
        event.GetId() == CmdMenu_Selected_ToTop().GetId() ||
        event.GetId() == CmdMenu_Selected_ToBack().GetId() ||
        event.GetId() == CmdMenu_Selected_SetStyle().GetId() ||
        event.GetId() == CmdMenu_Selected_SetExtStyle().GetId() ||
        event.GetId() == CmdMenu_Selected_SetTextChanges().GetId() ||
        event.GetId() == CmdMenu_Selected_ConvertToPolygonPolylinesWithArcs().GetId() ||
        event.GetId() == CmdMenu_Selected_ConvertToPolygonPolylinesWithoutArcs().GetId() ||
        event.GetId() == CmdMenu_Selected_ConvertToPolylines().GetId() ||
        event.GetId() == CmdMenu_Selected_ConvertLinesArcs().GetId() ||
        event.GetId() == CmdMenu_Selected_ConvertToVPaths().GetId() ||
        event.GetId() == CmdMenu_Selected_ConvertPolygonToArcs().GetId() ||
        event.GetId() == CmdMenu_Selected_ConvertPolylineToArcs().GetId() ||
        event.GetId() == CmdMenu_Selected_ConvertPathToPolygon().GetId() ||
        event.GetId() == CmdMenu_Selected_ConvertPolygonToSurface().GetId() ||
        event.GetId() == CmdMenu_Selected_ConvertSurfaceToPolygon().GetId() ||
        event.GetId() == CmdMenu_Selected_UnGroup().GetId() ||
        event.GetId() == CmdMenu_Selected_UnGroupDeep().GetId() ||
        event.GetId() == CmdMenu_Selected_Merge().GetId() ||
        event.GetId() == CmdMenu_Selected_Offset().GetId() ||
        event.GetId() == CmdMenu_Selected_CreateRing().GetId() ||
        event.GetId() == CmdMenu_Selected_CameleonDiagram().GetId() ||
        event.GetId() == CmdMenu_Selected_CameleonSymbol().GetId() ||
        event.GetId() == CmdMenu_Selected_CameleonSymbolDiagram().GetId() ||
        event.GetId() == CmdMenu_Selected_FlattenCameleon().GetId() ||
        event.GetId() == CmdMenu_Selected_CloneCameleonFromInst().GetId() ||
        event.GetId() == CmdMenu_Selected_MoveXY().GetId() ||
        event.GetId() == CmdMenu_Selected_MoveLayer().GetId() ||
        event.GetId() == CmdMenu_Selected_CopyXY().GetId() ||
        event.GetId() == CmdMenu_Selected_CopyLayer().GetId() ||
        event.GetId() == CmdMenu_Selected_Transform().GetId() ||
        event.GetId() == CmdMenu_Selected_Rotate().GetId() ||
        event.GetId() == CmdMenu_Selected_Align_MinX().GetId() ||       
        event.GetId() == CmdMenu_Selected_Align_MaxX().GetId() ||       
        event.GetId() == CmdMenu_Selected_Align_MinY().GetId() ||       
        event.GetId() == CmdMenu_Selected_Align_MaxY().GetId() ||       
        event.GetId() == CmdMenu_Selected_Align_MidX().GetId() ||       
        event.GetId() == CmdMenu_Selected_Align_MidY().GetId() ||              
        event.GetId() == CmdMenu_Selected_Align_MirrorVert().GetId() ||
        event.GetId() == CmdMenu_Selected_Align_MirrorHorz().GetId() ||
        event.GetId() == CmdMenu_Selected_Align_MirrorVertBbox().GetId() ||
        event.GetId() == CmdMenu_Selected_Align_MirrorHorzBbox().GetId() ||
        event.GetId() == CmdMenu_Selected_Align_DistributeVert().GetId() ||
        event.GetId() == CmdMenu_Selected_Align_DistributeHorz().GetId() ||
        event.GetId() == CmdMenu_Selected_Align_MinX_Dest().GetId() ||       
        event.GetId() == CmdMenu_Selected_Align_MaxX_Dest().GetId() ||       
        event.GetId() == CmdMenu_Selected_Align_MinY_Dest().GetId() ||       
        event.GetId() == CmdMenu_Selected_Align_MaxY_Dest().GetId() ||       
        event.GetId() == CmdMenu_Selected_Align_MidX_Dest().GetId() ||       
        event.GetId() == CmdMenu_Selected_Align_MidY_Dest().GetId() ||              
        event.GetId() == CmdMenu_Selected_Align_MirrorVert_Dest().GetId() ||
        event.GetId() == CmdMenu_Selected_Align_MirrorHorz_Dest().GetId() ||
        event.GetId() == CmdMenu_Selected_Align_MirrorVertBbox_Dest().GetId() ||
        event.GetId() == CmdMenu_Selected_Align_MirrorHorzBbox_Dest().GetId() ||
        event.GetId() == CmdMenu_Selected_SetExtStyle().GetId() ||
        event.GetId() == CmdMenu_Selected_SetTextChanges().GetId() || 
        event.GetId() == CmdMenu_PushInto().GetId() ||
        event.GetId() == CmdMenu_PopOut().GetId() ||

        event.GetId() == CmdMenu_ShowDiagram().GetId() ||
        event.GetId() == CmdMenu_ShowSymbol().GetId()  ||
        event.GetId() == CmdMenu_ShowGui().GetId() ||
        event.GetId() == CmdMenu_ShowDlgCameleonModal().GetId()

        )
    {
		event.Enable( false );
    }
}

void a2dCanvasCommandProcessor::OnUpdateMenu( wxUpdateUIEvent& event )
{
    if ( event.GetId() == CmdMenu_PushInto().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dCanvasObject* selected =  drawer->GetShowObject()->Find( "", "", a2dCanvasOFlags::SELECTED );
        event.Enable( selected != NULL );
    }
    else if ( event.GetId() == CmdMenu_PopOut().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        event.Enable( drawer->HasPopOut() );
    }
    else if ( event.GetId() == CmdMenu_InsertGroupRef().GetId() )
    {
    }
    else if ( event.GetId() == CmdMenu_ShowDlgLayersDrawing().GetId() )
    {
        event.Check( IsShownLayerPropertiesDialog() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgLayerOrderDrawing().GetId() ||
              event.GetId() == CmdMenu_ShowDlgLayerOrderGlobal().GetId() )
    {
        event.Check( IsShownLayerOrderDialog() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgTools().GetId() )
    {
        event.Check( IsShownToolDlg() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgStyle().GetId() )
    {
        event.Check( IsShowna2dStyleDialog() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgSnap().GetId() )
    {
        event.Check( IsShowna2dSnapSettings() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgGroups().GetId() )
    {
    }
    else if ( event.GetId() == CmdMenu_ShowDlgIdentify().GetId() )
    {
        event.Check( IsShownIdentifyDialog() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgCoordEntry().GetId() )
    {
        event.Check( IsShowna2dCoordinateEntry() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgMeasure().GetId() )
    {
        event.Check( IsShowna2dMeasureDlg() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgStructureModal().GetId() )
    {
    }
    else if ( event.GetId() == CmdMenu_ShowDlgStructure().GetId() )
    {
        event.Check( IsShownStructureDlg() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgPathSettings().GetId() )
    {
        event.Check( IsShowna2dPathSettings() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgSettings().GetId() )
    {
        event.Check( IsShowna2dSettings() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgTrans().GetId() )
    {
        event.Check( IsShowna2dTransDlg() );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgCameleonModal().GetId() )
    {
        event.Enable( false );
        if ( a2dCameleon::GetCameleonRoot() )
        {
            a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
            if ( !drawingPart )
                return;

            event.Enable( true );
        }
    }


/*
    else if ( event.GetId() == CmdMenu_ShowDlgPropEdit().GetId() )
    {
        event.Check( IsShownPropEditDlg() );
    }
*/
    else if ( event.GetId() == CmdMenu_SetLayerDlgModeless().GetId() )
    {
        event.Check( m_layerChooseDlg != NULL && !m_layerChooseDlg->IsModal() && m_layerChooseDlg->IsShown() );
    }
    else if ( event.GetId() == CmdMenu_SetMasterTool_SelectFirst().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        event.Check( wxDynamicCast( contr->GetFirstTool(), a2dMasterDrawSelectFirst ) != NULL );
    }
    else if ( event.GetId() == CmdMenu_SetMasterTool_ZoomFirst().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        event.Check( wxDynamicCast( contr->GetFirstTool(), a2dMasterDrawZoomFirst ) != NULL );
    }
    else if ( event.GetId() == CmdMenu_SetMasterTool_TagGroups().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        event.Check( wxDynamicCast( contr->GetFirstTool(), a2dMasterTagGroups ) != NULL );
    }
    else if ( event.GetId() == CmdMenu_SetMasterTool_TagGroups_WireMode().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        a2dMasterTagGroups* tool = wxDynamicCast( contr->GetFirstTool(), a2dMasterTagGroups );
        event.Check( tool && tool->GetWireMode() );
    }
    else if ( event.GetId() == CmdMenu_SetMasterTool_TagGroups_DlgMode().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        a2dMasterTagGroups* tool = wxDynamicCast( contr->GetFirstTool(), a2dMasterTagGroups );
        event.Check( tool && tool->GetWireMode() );
    }
    else if ( event.GetId() == CmdMenu_SetDrawWireFrame().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;

        bool zero = ( drawer->GetDocumentDrawStyle() & ( RenderWIREFRAME_ZERO_WIDTH ) ) > 0;

        if ( zero )
            event.Check( true );
        else
            event.Check( false );
    }
    else if ( event.GetId() == CmdMenu_DrawGridLines().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        event.Check( drawer->GetGridLines() );
    }
    else if ( event.GetId() == CmdMenu_DrawGridAtFront().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
            event.Check( drawer->GetGridAtFront() );
    }
    else if ( event.GetId() == CmdMenu_DrawGrid().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        event.Check( drawer->GetGrid() );
    }
    else if ( event.GetId() == CmdMenu_SetSnap().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        event.Check( drawer->GetCanvasToolContr()->GetSnap() );
    }
    else if ( event.GetId() == CmdMenu_SetYAxis().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        event.Check( drawer->GetDrawer2D()->GetYaxis() );
    }
    else if ( event.GetId() == CmdMenu_Option_SplinePoly().GetId() )
    {
        event.Check( m_drawing->GetHabitat()->GetSpline() );
    }
    else if ( event.GetId() == CmdMenu_Option_CursorCrosshair().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        event.Check( drawer->GetCrossHair() );
    }
    else if ( event.GetId() == CmdMenu_Option_RescaleAtReSize().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;

        a2dCanvas* canvas = wxDynamicCast( drawer->GetDisplayWindow(), a2dCanvas );
        if ( canvas )
            event.Check( canvas->GetScaleOnResize() );

#if wxART2D_USE_OPENGL && wxUSE_GLCANVAS 
        a2dOglCanvas* canvasogl = wxDynamicCast( drawer->GetDisplayWindow(), a2dOglCanvas );
        if ( canvasogl )
            event.Check( canvasogl->GetScaleOnResize() );
#endif //wxART2D_USE_OPENGL
    }
    else if ( event.GetId() == CmdMenu_Option_ReverseLayers().GetId() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        event.Check( drawer->GetReverseOrder() );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_StraightEndSegment().GetId() )
    {
		event.Check( m_drawing->GetHabitat()->GetConnectionGenerator()->GetRouteMethod() == a2dConnectionGenerator::StraightEndSegment );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_StraightEndSegmentUnlessPins().GetId() )
    {
		event.Check( m_drawing->GetHabitat()->GetConnectionGenerator()->GetRouteMethod() == a2dConnectionGenerator::StraightEndSegmentUnlessPins );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_StraightSegment().GetId() )
    {
		event.Check( m_drawing->GetHabitat()->GetConnectionGenerator()->GetRouteMethod() == a2dConnectionGenerator::StraightSegment );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_StraightSegmentUnlessPins().GetId() )
    {
		event.Check( m_drawing->GetHabitat()->GetConnectionGenerator()->GetRouteMethod() == a2dConnectionGenerator::StraightSegmentUnlessPins );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_GridRouting().GetId() )
    {
		event.Check( m_drawing->GetHabitat()->GetConnectionGenerator()->GetRouteMethod() == a2dConnectionGenerator::GridRouting );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_ManhattanSegments().GetId() )
    {
		event.Check( m_drawing->GetHabitat()->GetConnectionGenerator()->GetRouteMethod() == a2dConnectionGenerator::ManhattanSegments );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_ManhattanEndSegments().GetId() )
    {
		event.Check( m_drawing->GetHabitat()->GetConnectionGenerator()->GetRouteMethod() == a2dConnectionGenerator::ManhattanEndSegments );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_ManhattanEndSegmentsStay().GetId() )
    {
		event.Check( m_drawing->GetHabitat()->GetConnectionGenerator()->GetRouteMethod() == a2dConnectionGenerator::ManhattanEndSegmentsStay );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_ManhattanEndSegmentsConvertAndStay().GetId() )
    {
		event.Check( m_drawing->GetHabitat()->GetConnectionGenerator()->GetRouteMethod() == a2dConnectionGenerator::ManhattanEndSegmentsConvertAndStay );
    }
    else if ( event.GetId() == CmdMenu_RotateObject90Right().GetId() || 
              event.GetId() == CmdMenu_RotateObject90Left().GetId()
			)
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dCanvasObject* selected =  drawer->GetShowObject()->Find( "", "", a2dCanvasOFlags::SELECTED );
        event.Enable( true ); //!todo when exactly depend on tools.
    }
    else if (        
              event.GetId() == CmdMenu_ZoomIn2().GetId() ||
              event.GetId() == CmdMenu_ZoomOut2().GetId() ||
              event.GetId() == CmdMenu_ZoomIn2CondSelected().GetId() ||
              event.GetId() == CmdMenu_ZoomOut2CondSelected().GetId() ||
              
              event.GetId() == CmdMenu_ZoomOut().GetId() ||

              event.GetId() == CmdMenu_Selected_SelectAll().GetId() ||
              event.GetId() == CmdMenu_Selected_SelectAll_NoUndo().GetId()
			)
    {
        event.Enable( false );
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        event.Enable( true ); 
    }
    else if ( 
              event.GetId() == CmdMenu_Selected_RotateObject90Right().GetId() || 
              event.GetId() == CmdMenu_Selected_RotateObject90Left().GetId() ||       
              event.GetId() == CmdMenu_Selected_DeSelectAll().GetId() ||
              event.GetId() == CmdMenu_Selected_Delete().GetId() ||
              event.GetId() == CmdMenu_Selected_DeSelectAll_NoUndo().GetId() ||
              event.GetId() == CmdMenu_Selected_Group().GetId() ||
              event.GetId() == CmdMenu_Selected_ImageToRectangles().GetId() ||
              event.GetId() == CmdMenu_Selected_ToTop().GetId() ||
              event.GetId() == CmdMenu_Selected_ToBack().GetId() ||
              event.GetId() == CmdMenu_Selected_SetStyle().GetId() ||
              event.GetId() == CmdMenu_Selected_SetExtStyle().GetId() ||
              event.GetId() == CmdMenu_Selected_SetTextChanges().GetId() ||
              event.GetId() == CmdMenu_Selected_ConvertToPolygonPolylinesWithArcs().GetId() ||
              event.GetId() == CmdMenu_Selected_ConvertToPolygonPolylinesWithoutArcs().GetId() ||
              event.GetId() == CmdMenu_Selected_ConvertToPolylines().GetId() ||
              event.GetId() == CmdMenu_Selected_ConvertLinesArcs().GetId() ||
              event.GetId() == CmdMenu_Selected_ConvertToVPaths().GetId() ||
              event.GetId() == CmdMenu_Selected_ConvertPolygonToArcs().GetId() ||
              event.GetId() == CmdMenu_Selected_ConvertPolylineToArcs().GetId() ||
              event.GetId() == CmdMenu_Selected_ConvertPathToPolygon().GetId() ||
              event.GetId() == CmdMenu_Selected_ConvertPolygonToSurface().GetId() ||
              event.GetId() == CmdMenu_Selected_ConvertSurfaceToPolygon().GetId() ||
              event.GetId() == CmdMenu_Selected_UnGroup().GetId() ||
              event.GetId() == CmdMenu_Selected_UnGroupDeep().GetId() ||
              event.GetId() == CmdMenu_Selected_Merge().GetId() ||
              event.GetId() == CmdMenu_Selected_Offset().GetId() ||
              event.GetId() == CmdMenu_Selected_CreateRing().GetId() ||
              event.GetId() == CmdMenu_Selected_CameleonDiagram().GetId() ||
              event.GetId() == CmdMenu_Selected_CameleonSymbol().GetId() ||
              event.GetId() == CmdMenu_Selected_CameleonSymbolDiagram().GetId() ||
              event.GetId() == CmdMenu_Selected_FlattenCameleon().GetId() ||
              event.GetId() == CmdMenu_Selected_CloneCameleonFromInst().GetId() ||
              event.GetId() == CmdMenu_Selected_MoveXY().GetId() ||
              event.GetId() == CmdMenu_Selected_MoveLayer().GetId() ||
              event.GetId() == CmdMenu_Selected_CopyXY().GetId() ||
              event.GetId() == CmdMenu_Selected_CopyLayer().GetId() ||
              event.GetId() == CmdMenu_Selected_Transform().GetId() ||
              event.GetId() == CmdMenu_Selected_Rotate().GetId() ||
              event.GetId() == CmdMenu_Selected_Align_MinX().GetId() ||       
              event.GetId() == CmdMenu_Selected_Align_MaxX().GetId() ||       
              event.GetId() == CmdMenu_Selected_Align_MinY().GetId() ||       
              event.GetId() == CmdMenu_Selected_Align_MaxY().GetId() ||       
              event.GetId() == CmdMenu_Selected_Align_MidX().GetId() ||       
              event.GetId() == CmdMenu_Selected_Align_MidY().GetId() ||              
              event.GetId() == CmdMenu_Selected_Align_MirrorVert().GetId() ||
              event.GetId() == CmdMenu_Selected_Align_MirrorHorz().GetId() ||
              event.GetId() == CmdMenu_Selected_Align_MirrorVertBbox().GetId() ||
              event.GetId() == CmdMenu_Selected_Align_MirrorHorzBbox().GetId() ||
              event.GetId() == CmdMenu_Selected_Align_DistributeVert().GetId() ||
              event.GetId() == CmdMenu_Selected_Align_DistributeHorz().GetId() ||
              event.GetId() == CmdMenu_Selected_Align_MinX_Dest().GetId() ||       
              event.GetId() == CmdMenu_Selected_Align_MaxX_Dest().GetId() ||       
              event.GetId() == CmdMenu_Selected_Align_MinY_Dest().GetId() ||       
              event.GetId() == CmdMenu_Selected_Align_MaxY_Dest().GetId() ||       
              event.GetId() == CmdMenu_Selected_Align_MidX_Dest().GetId() ||       
              event.GetId() == CmdMenu_Selected_Align_MidY_Dest().GetId() ||              
              event.GetId() == CmdMenu_Selected_Align_MirrorVert_Dest().GetId() ||
              event.GetId() == CmdMenu_Selected_Align_MirrorHorz_Dest().GetId() ||
              event.GetId() == CmdMenu_Selected_Align_MirrorVertBbox_Dest().GetId() ||
              event.GetId() == CmdMenu_Selected_Align_MirrorHorzBbox_Dest().GetId()
        )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dCanvasObject* selected =  drawer->GetShowObject()->Find( "", "", a2dCanvasOFlags::SELECTED );
        event.Enable( selected != NULL );
    }
    else
        event.Skip();
}

void a2dCanvasCommandProcessor::OnPushToolUpdateMenu( wxUpdateUIEvent& event )
{
	event.Check( false );
    a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawer )
        return;

    a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
    if ( !contr )
    {
        event.Enable( false );
        return;
    }

    event.Enable( true );

    //One shot versions
    if ( event.GetId() == CmdMenu_PushTool_Zoom_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dZoomTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Select_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dSelectTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Select2_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dSelectTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_RecursiveEdit_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dRecursiveEditTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_MultiEdit_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dMultiEditTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DragOriginal_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragOrgTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Drag_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Copy_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dCopyTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Rotate_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dRotateTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Delete_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDeleteTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawText_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawTextTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Image_OneShot().GetId() )
    {
		a2dImageTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dImageTool );
        if ( tool && !tool->GetEmbed() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Image_Embedded_OneShot().GetId() )
    {
		a2dImageTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dImageTool );
        if ( tool && tool->GetEmbed() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawLine_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawLineTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawLineScaledArrow_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawLineTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipse_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawEllipseTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipticArc_OneShot().GetId() )
    {
		a2dDrawEllipticArcTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawEllipticArcTool );
        if ( tool && !tool->GetChord() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipticArc_Chord_OneShot().GetId() )
    {
		a2dDrawEllipticArcTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawEllipticArcTool );
        if ( tool && tool->GetChord() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawArc_OneShot().GetId() )
    {
		a2dDrawArcTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawArcTool );
        if ( tool && !tool->GetChord() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawArc_Chord_OneShot().GetId() )
    {
		a2dDrawArcTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawArcTool );
        if ( tool && tool->GetChord() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolylineL_OneShot().GetId() )
    {
		a2dDrawPolylineLTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawPolylineLTool );
        if ( tool && !tool->GetSpline() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolygonL_OneShot().GetId() )
    {
		a2dDrawPolygonLTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawPolygonLTool );
        if ( tool && !tool->GetSpline() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolylineL_Splined_OneShot().GetId() )
    {
		a2dDrawPolylineLTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawPolylineLTool );
        if ( tool && tool->GetSpline() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolygonL_Splined_OneShot().GetId() )
    {
		a2dDrawPolygonLTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawPolygonLTool );
        if ( tool && tool->GetSpline() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Property_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dPropertyTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Measure_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dMeasureTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawWirePolylineL_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawWirePolylineLTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DragMulti_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragMultiTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CopyMulti_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dCopyMultiTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawRectangle_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawRectangleTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawCircle_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawCircleTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawVPath_OneShot().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawVPathTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Origin_OneShot().GetId() )
    {
		a2dDragNewTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragNewTool );
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_Origin_OneShot() ) )
		        event.Check( true );
    }

    //Normal versions
    else if ( event.GetId() == CmdMenu_PushTool_Zoom().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dZoomTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Select().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dSelectTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Select2().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dSelectTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_RecursiveEdit().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dRecursiveEditTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_MultiEdit().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dMultiEditTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DragOriginal().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragOrgTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Drag().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Copy().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dCopyTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Rotate().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dRotateTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Delete().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDeleteTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawText().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawTextTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Image().GetId() )
    {
		a2dImageTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dImageTool );
        if ( tool && !tool->GetEmbed() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Image_Embedded().GetId() )
    {
		a2dImageTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dImageTool );
        if ( tool && tool->GetEmbed() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawLine().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawLineTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawLineScaledArrow().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawLineTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipse().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawEllipseTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipticArc().GetId() )
    {
		a2dDrawEllipticArcTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawEllipticArcTool );
        if ( tool && !tool->GetChord() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipticArc_Chord().GetId() )
    {
		a2dDrawEllipticArcTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawEllipticArcTool );
        if ( tool && tool->GetChord() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawArc().GetId() )
    {
		a2dDrawArcTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawArcTool );
        if ( tool && !tool->GetChord() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawArc_Chord().GetId() )
    {
		a2dDrawArcTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawArcTool );
        if ( tool && tool->GetChord() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolylineL().GetId() )
    {
		a2dDrawPolylineLTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawPolylineLTool );
        if ( tool && !tool->GetSpline() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolygonL().GetId() )
    {
		a2dDrawPolygonLTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawPolygonLTool );
        if ( tool && !tool->GetSpline() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolylineL_Splined().GetId() )
    {
		a2dDrawPolylineLTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawPolylineLTool );
        if ( tool && tool->GetSpline() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolygonL_Splined().GetId() )
    {
		a2dDrawPolygonLTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawPolygonLTool );
        if ( tool && tool->GetSpline() )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Property().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dPropertyTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Measure().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dMeasureTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawWirePolylineL().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawWirePolylineLTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DragMulti().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragMultiTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CopyMulti().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dCopyMultiTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawRectangle().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawRectangleTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawCircle().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawCircleTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawVPath().GetId() )
    {
		a2dStTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDrawVPathTool );
        if ( tool )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Port().GetId() )
    {
		a2dDragNewTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragNewTool );
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_Port() ) )
		        event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Cameleon().GetId() )
    {
		a2dDragNewTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragNewTool );
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_Cameleon() ) )
            event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Cameleon_BuildIn().GetId() )
    {
    }
    else if ( event.GetId() == CmdMenu_PushTool_CameleonInst().GetId() )
    {
		a2dDragNewTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragNewTool );
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_CameleonInst() ) )
            event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CameleonInstDlg().GetId() )
    {
		a2dDragNewTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragNewTool );
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_CameleonInstDlg() ) )
            event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CameleonRefDlg_Diagram().GetId() )
    {
		a2dDragNewTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragNewTool );
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_CameleonRefDlg_Diagram() ) )
            event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CameleonRefDlg_Symbol().GetId() )
    {
		a2dDragNewTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragNewTool );
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_CameleonRefDlg_Symbol() ) )
            event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CameleonRefDlg().GetId() )
    {
		a2dDragNewTool* tool = wxDynamicCast( contr->GetFirstTool(), a2dDragNewTool );
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_CameleonRefDlg() ) )
            event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_TagGroups().GetId() )
    {
		a2dBaseTool* tool = contr->GetFirstTool();
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_TagGroups() ) )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_TagGroups_WireMode().GetId() )
    {
		a2dBaseTool* tool = contr->GetFirstTool();
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_TagGroups_WireMode() ) )
		    event.Check( true );
    }    
	else if ( event.GetId() == CmdMenu_PushTool_TagGroups_DlgMode().GetId() )
    {
		a2dBaseTool* tool = contr->GetFirstTool();
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_TagGroups_DlgMode() ) )
		    event.Check( true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_TagGroups_WireMode_DlgMode().GetId() )
    {
		a2dBaseTool* tool = contr->GetFirstTool();
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_TagGroups_WireMode_DlgMode() ) )
		    event.Check( true );
    }    
    else if ( event.GetId() == CmdMenu_PushTool_SingleClick().GetId() )
    {
		a2dBaseTool* tool = contr->GetFirstTool();
        if ( tool && tool->GetInitiatingMenuId().SameId( CmdMenu_PushTool_SingleClick() ) )
		    event.Check( true );
    }    
}

void a2dCanvasCommandProcessor::OnMenu( wxCommandEvent& event )
{
    bool onTop = true;
    long style = wxDEFAULT_DIALOG_STYLE;
    if ( !onTop )
        style |= wxSTAY_ON_TOP;

    bool found = false;

    if ( !m_drawing )
        return;

    a2dDrawing* drawing = m_drawing;

    a2dCanvasObjectPtr top = GetActiveDrawingPart()->GetShowObject();

    if ( event.GetId() == CmdMenu_Drawing_FileClose().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_Drawing_FileOpen().GetId() )
    {
        found = true;

        a2dIOHandlerCVGIn* cvghin = new a2dIOHandlerCVGIn();

        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;

        static wxString cvg_dir = ".";//*( a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "APPLICATION_DATA" ) ) );

        static wxString filter = _T( "All files (*.*)| *.*| CVG files (*.cvg)| *.cvg" );

        wxFileName cvg_file;

        wxFileDialog dialog( drawer->GetDisplayWindow(), _T( "Select CVG File" ), cvg_dir, cvg_file.GetFullPath(), filter, wxFD_OPEN );

        if ( dialog.ShowModal() != wxID_OK )
            return;

        // save for the next time
        cvg_dir = dialog.GetPath();
        cvg_file = dialog.GetFilename();
        wxFileName cvg_dirf = wxFileName( cvg_dir );

        cvg_dir = cvg_dirf.GetVolume() + cvg_dirf.GetVolumeSeparator() + cvg_dirf.GetPathWithSep( wxPATH_UNIX );

        wxString cvg_fullpath = cvg_dir + cvg_file.GetFullName();

        if ( wxFileExists( cvg_fullpath ) )
        {
            m_drawing->GetRootObject()->ReleaseChildObjects();
            m_drawing->LoadFromFile( cvg_fullpath, cvghin );
        }

        if ( drawer )
            drawer->SetMappingShowAllCanvas();

    }
    else if ( event.GetId() == CmdMenu_Drawing_FileNew().GetId() )
    {
        found = true;
        m_drawing->GetRootObject()->ReleaseChildObjects();
    }
    else if ( event.GetId() == CmdMenu_Drawing_FileSave().GetId() )
    {
        found = true;

        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;

        static wxString cvg_dir = ".";//*( a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "APPLICATION_DATA" ) ) );

        static wxString filter = _T( "All files (*.*)| *.*| CVG files (*.cvg)| *.cvg" );

        wxFileName cvg_file;

        wxFileDialog dialog( drawer->GetDisplayWindow(), _T( "Select CVG File" ), cvg_dir, cvg_file.GetFullPath(), filter, wxFD_SAVE );

        if ( dialog.ShowModal() != wxID_OK )
            return;

        // save for the next time
        cvg_dir = dialog.GetPath();
        cvg_file = dialog.GetFilename();
        wxFileName cvg_dirf = wxFileName( cvg_dir );

        cvg_dir = cvg_dirf.GetVolume() + cvg_dirf.GetVolumeSeparator() + cvg_dirf.GetPathWithSep( wxPATH_UNIX );

        wxString cvg_fullpath = cvg_dir + cvg_file.GetFullName();

        a2dIOHandlerCVGOut* cvghout = new a2dIOHandlerCVGOut();
        m_drawing->SaveToFile( cvg_fullpath, cvghout );
    }
    else if ( event.GetId() == CmdMenu_Drawing_FileSaveAs().GetId() )
    {
        found = true;

        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;

        static wxString cvg_dir = ".";//*( a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "APPLICATION_DATA" ) ) );

        static wxString filter = _T( "All files (*.*)| *.*| CVG files (*.cvg)| *.cvg" );

        wxFileName cvg_file;

        wxFileDialog dialog( drawer->GetDisplayWindow(), _T( "Select CVG File" ), cvg_dir, cvg_file.GetFullPath(), filter, wxFD_SAVE );

        if ( dialog.ShowModal() != wxID_OK )
            return;

        // save for the next time
        cvg_dir = dialog.GetPath();
        cvg_file = dialog.GetFilename();
        wxFileName cvg_dirf = wxFileName( cvg_dir );

        cvg_dir = cvg_dirf.GetVolume() + cvg_dirf.GetVolumeSeparator() + cvg_dirf.GetPathWithSep( wxPATH_UNIX );

        wxString cvg_fullpath = cvg_dir + cvg_file.GetFullName();

        a2dIOHandlerCVGOut* cvghout = new a2dIOHandlerCVGOut();
        m_drawing->SaveToFile( cvg_fullpath, cvghout );
    }
    else if ( event.GetId() == CmdMenu_ChangeCanvasObjectStyle().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_SetLayer().GetId() )
    {
        found = true;
        long w = wxGetNumberFromUser( _( "Give layer number:" ), _( "Layer:" ), _( "layer number" ), m_drawing->GetHabitat()->m_layer, 0, wxMAXLAYER );

        if (  w != -1 )
        {
            if ( w >= 0 && w < ( long ) wxMAXLAYER )
            {
                m_drawing->GetHabitat()->SetLayer( w, true );
            }
            else
                ( void )wxMessageBox( _( "Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        }
    }
    else if ( event.GetId() == CmdMenu_SetTarget().GetId() )
    {
        found = true;
        long w = wxGetNumberFromUser( _( "Give layer number:" ), _( "Layer:" ), _( "Set Target Layer" ), m_drawing->GetHabitat()->m_target, 0, wxMAXLAYER );

        if (  w != -1 )
        {
            if ( w >= 0 && w < ( long ) wxMAXLAYER )
            {
                m_drawing->GetHabitat()->SetTarget( w );
            }
            else
                ( void )wxMessageBox( _( "Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        }
    }
    else if ( event.GetId() == CmdMenu_SetBackgroundStyle().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStyleDialog b( m_drawing->GetHabitat(), NULL, wxDEFAULT_DIALOG_STYLE | wxDIALOG_NO_PARENT | wxMINIMIZE_BOX | wxMAXIMIZE_BOX, true );
        b.SetFill( drawer->GetBackgroundFill() );
        if ( b.ShowModal() == wxID_OK )
            drawer->SetBackgroundFill( b.GetFill() );
    }
    else if ( event.GetId() == CmdMenu_SetDrawWireFrame().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;

        bool zero = ( drawer->GetDocumentDrawStyle() & ( RenderWIREFRAME_ZERO_WIDTH ) ) > 0;

        if ( zero )
        {
            drawer->SetDocumentDrawStyle( RenderLAYERED | RenderWIREFRAME_SELECT | RenderWIREFRAME_SELECT2 );
            drawer->Update();
        }
        else
        {
            drawer->SetDocumentDrawStyle( RenderWIREFRAME_ZERO_WIDTH | RenderWIREFRAME_SELECT | RenderWIREFRAME_SELECT2 );
            drawer->Update();
        }
    }
    else if ( event.GetId() == CmdMenu_DrawGridLines().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        drawer->SetGridLines( !drawer->GetGridLines() );
    }
    else if ( event.GetId() == CmdMenu_DrawGridAtFront().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
         drawer->SetGridAtFront( !drawer->GetGridAtFront() );
    }
    else if ( event.GetId() == CmdMenu_Refresh().GetId() )
    {
        found = true;
        Refresh();
    }
    else if ( event.GetId() == CmdMenu_ZoomOut().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        drawer->SetMappingShowAll();
    }
    else if ( event.GetId() == CmdMenu_ZoomOut2().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        if ( !drawer->GetDisplayWindow() )
            return;
        a2dDrawer2D* dr = drawer->GetDrawer2D();
        if ( !dr )
            return;

        int dx, dy;
        drawer->GetDisplayWindow()->GetClientSize( &dx, &dy );
        if ( dy > dx ) dx = dy;

        drawer->SetMappingUpp(
            dr->GetVisibleMinX() - dr->GetUppX()*dx*( m_zoomfactor*0.5 - 0.5 ),
            dr->GetVisibleMinY() - dr->GetUppY()*dy*( m_zoomfactor*0.5 - 0.5 ),
            dr->GetUppX() * m_zoomfactor, dr->GetUppY() * m_zoomfactor );
    }
    else if ( event.GetId() == CmdMenu_ZoomIn2().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        if ( !drawer->GetDisplayWindow() )
            return;
        a2dDrawer2D* dr = drawer->GetDrawer2D();
        if ( !dr )
            return;

        int dx, dy;
        drawer->GetDisplayWindow()->GetClientSize( &dx, &dy );
        if ( dy > dx ) dx = dy;

        drawer->SetMappingUpp(
            dr->GetVisibleMinX() + dr->GetUppX()*dx*( 0.5 - 1/(m_zoomfactor*2)),
            dr->GetVisibleMinY() + dr->GetUppY()*dy*( 0.5 - 1/(m_zoomfactor*2)),
            dr->GetUppX() / m_zoomfactor, dr->GetUppY() / m_zoomfactor );
    }
    else if ( event.GetId() == CmdMenu_ZoomOut2CondSelected().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        if ( !drawer->GetDisplayWindow() )
            return;
        a2dDrawer2D* dr = drawer->GetDrawer2D();
        if ( !dr )
            return;
        if ( ! drawer->GetShowObject() )
            return;

        int dx, dy;
        drawer->GetDisplayWindow()->GetClientSize( &dx, &dy );

        a2dBoundingBox bbox = drawer->GetShowObject()->GetChildObjectList()->GetBBox( a2dCanvasOFlags::VISIBLE | a2dCanvasOFlags::SELECTED );

        if ( bbox.GetValid() )
        {
            double upxy = dx;
            if ( dy > dx ) upxy = dy;
            drawer->SetMappingUpp(
                dr->GetVisibleMinX() - dr->GetUppX()*upxy*( m_zoomfactor*0.5 - 0.5 ),
                dr->GetVisibleMinY() - dr->GetUppY()*upxy*( m_zoomfactor*0.5 - 0.5 ),
                dr->GetUppX() * m_zoomfactor, dr->GetUppY() * m_zoomfactor ); 

            double xc, yc, xcn, ycn;
            xc = dr->GetVisibleMinX() + dr->GetUppX()*dx*0.5;
            yc = dr->GetVisibleMinY() + dr->GetUppY()*dy*0.5;
            xcn = bbox.GetCentre().m_x;
            ycn = bbox.GetCentre().m_y;
         
            a2dBoundingBox visBbox = dr->GetVisibleBbox(); 
            if (
                   //( visBbox.Intersect( bbox, bbox.GetWidth()/10 ) != _IN ) &&
                   ( fabs(xc - xcn) >  bbox.GetWidth()/10 || fabs( yc - ycn ) > bbox.GetWidth()/10 )
               )
            {
                a2dBoundingBox bsel( xcn - visBbox.GetWidth()/2.0, ycn - visBbox.GetHeight()/2.0, xcn + visBbox.GetWidth()/2.0, ycn + visBbox.GetHeight()/2.0 );
                dr->SetMappingDeviceRect( 0, 0 , dx, dy );
                dr->SetMappingWidthHeight( bsel );
            }
        }
        else
        {
            double upxy = dx;
            if ( dy > dx ) upxy = dy;

            drawer->SetMappingUpp(
                dr->GetVisibleMinX() - dr->GetUppX()*upxy*( m_zoomfactor*0.5 - 0.5 ),
                dr->GetVisibleMinY() - dr->GetUppY()*upxy*( m_zoomfactor*0.5 - 0.5 ),
                dr->GetUppX() * m_zoomfactor, dr->GetUppY() * m_zoomfactor );
        }
    }
    else if ( event.GetId() == CmdMenu_ZoomIn2CondSelected().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        if ( !drawer->GetDisplayWindow() )
            return;
        a2dDrawer2D* dr = drawer->GetDrawer2D();
        if ( !dr )
            return;

        if ( ! drawer->GetShowObject() )
            return;

        int dx, dy;
        drawer->GetDisplayWindow()->GetClientSize( &dx, &dy );
        
        a2dBoundingBox bbox = drawer->GetShowObject()->GetChildObjectList()->GetBBox( a2dCanvasOFlags::VISIBLE | a2dCanvasOFlags::SELECTED );

        if ( bbox.GetValid() )
        {
            double upxy = dx;
            if ( dy > dx ) upxy = dy;

            drawer->SetMappingUpp(
                dr->GetVisibleMinX() + dr->GetUppX()*upxy*( 0.5 - 1/(m_zoomfactor*2)),
                dr->GetVisibleMinY() + dr->GetUppY()*upxy*( 0.5 - 1/(m_zoomfactor*2)),
                dr->GetUppX() / m_zoomfactor, dr->GetUppY() / m_zoomfactor ); 

            double xc, yc, xcn, ycn;
            xc = dr->GetVisibleMinX() + dr->GetUppX()*dx*0.5;
            yc = dr->GetVisibleMinY() + dr->GetUppY()*dy*0.5;
            xcn = bbox.GetCentre().m_x;
            ycn = bbox.GetCentre().m_y;
            
            a2dBoundingBox visBbox = dr->GetVisibleBbox(); 
            if (
                   //( visBbox.Intersect( bbox, bbox.GetWidth()/10 ) == _IN ) &&
                   ( fabs(xc - xcn) >  bbox.GetWidth()/20 || fabs( yc - ycn ) > bbox.GetWidth()/20 )
               )
            {
                a2dBoundingBox bsel( xcn - visBbox.GetWidth()/2.0, ycn - visBbox.GetHeight()/2.0, xcn + visBbox.GetWidth()/2.0, ycn + visBbox.GetHeight()/2.0 );
                dr->SetMappingDeviceRect( 0, 0 , dx, dy );
                dr->SetMappingWidthHeight( bsel );
            }
        }
        else
        {
            double upxy = dx;
            if ( dy > dx ) upxy = dy;

            drawer->SetMappingUpp(
                dr->GetVisibleMinX() + dr->GetUppX()*upxy*( 0.5 - 1/(m_zoomfactor*2)),
                dr->GetVisibleMinY() + dr->GetUppY()*upxy*( 0.5 - 1/(m_zoomfactor*2)),
                dr->GetUppX() / m_zoomfactor, dr->GetUppY() / m_zoomfactor );
        }
    }
    else if ( event.GetId() == CmdMenu_EmptyShownObject().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        if ( ! drawer->GetShowObject() )
            return;

        drawer->GetShowObject()->ReleaseChildObjects();
    }
    else if ( event.GetId() == CmdMenu_DrawGrid().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        drawer->SetGrid( !drawer->GetGrid() );
    }
    else if ( event.GetId() == CmdMenu_SetSnap().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;
        m_drawing->GetHabitat()->GetRestrictionEngine()->SetSnap( !GetActiveDrawingPart()->GetCanvasToolContr()->GetSnap() );
    }
    else if ( event.GetId() == CmdMenu_SetYAxis().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;
        if ( !GetActiveDrawingPart()->GetDrawer2D() )
            return;

        a2dDiagram::SetNextLineDirection( GetActiveDrawingPart()->GetDrawer2D()->GetYaxis() );
        a2dCameleon::SetNextLineDirection( GetActiveDrawingPart()->GetDrawer2D()->GetYaxis() );

        GetActiveDrawingPart()->GetDrawer2D()->SetYaxis( !GetActiveDrawingPart()->GetDrawer2D()->GetYaxis() );
        GetActiveDrawingPart()->SetMappingShowAll();
    }
    else if ( event.GetId() == CmdMenu_Option_SplinePoly().GetId() )
    {
        found = true;
        m_drawing->GetHabitat()->SetSpline( !m_drawing->GetHabitat()->GetSpline() );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_StraightEndSegment().GetId() )
    {
        found = true;
		m_drawing->GetHabitat()->GetConnectionGenerator()->SetRouteMethod( a2dConnectionGenerator::StraightEndSegment );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_StraightEndSegmentUnlessPins().GetId() )
    {
        found = true;
		m_drawing->GetHabitat()->GetConnectionGenerator()->SetRouteMethod( a2dConnectionGenerator::StraightEndSegmentUnlessPins );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_StraightSegment().GetId() )
    {
        found = true;
		m_drawing->GetHabitat()->GetConnectionGenerator()->SetRouteMethod( a2dConnectionGenerator::StraightSegment );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_StraightSegmentUnlessPins().GetId() )
    {
        found = true;
		m_drawing->GetHabitat()->GetConnectionGenerator()->SetRouteMethod( a2dConnectionGenerator::StraightSegmentUnlessPins );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_GridRouting().GetId() )
    {
        found = true;
		m_drawing->GetHabitat()->GetConnectionGenerator()->SetRouteMethod( a2dConnectionGenerator::GridRouting );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_ManhattanSegments().GetId() )
    {
		m_drawing->GetHabitat()->GetConnectionGenerator()->SetRouteMethod( a2dConnectionGenerator::ManhattanSegments );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_ManhattanEndSegments().GetId() )
    {
		m_drawing->GetHabitat()->GetConnectionGenerator()->SetRouteMethod( a2dConnectionGenerator::ManhattanEndSegments );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_ManhattanEndSegmentsStay().GetId() )
    {
		m_drawing->GetHabitat()->GetConnectionGenerator()->SetRouteMethod( a2dConnectionGenerator::ManhattanEndSegmentsStay );
    }
    else if ( event.GetId() == CmdMenu_RouteMethod_ManhattanEndSegmentsConvertAndStay().GetId() )
    {
		m_drawing->GetHabitat()->GetConnectionGenerator()->SetRouteMethod( a2dConnectionGenerator::ManhattanEndSegmentsConvertAndStay );
    }
    else if ( event.GetId() == CmdMenu_RotateObject90Right().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;

        a2dStToolContr* contr = wxDynamicCast( GetActiveDrawingPart()->GetCanvasToolContr(), a2dStToolContr );
        if ( contr )
        {
            contr->RotateObject90LeftRight( true );
        }
    }
    else if ( event.GetId() == CmdMenu_RotateObject90Left().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;

        a2dStToolContr* contr = wxDynamicCast( GetActiveDrawingPart()->GetCanvasToolContr(), a2dStToolContr );
        if ( contr )
        {
            contr->RotateObject90LeftRight( false );
        }
    }
    else if ( event.GetId() == CmdMenu_Option_CursorCrosshair().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;
        if ( !GetActiveDrawingPart()->GetDrawer2D() )
            return;
        GetActiveDrawingPart()->SetCrossHair( !GetActiveDrawingPart()->GetCrossHair() );
    }
    else if ( event.GetId() == CmdMenu_Option_RescaleAtReSize().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;
        if ( !GetActiveDrawingPart()->GetCanvas() )
            return;
        GetActiveDrawingPart()->GetCanvas()->SetScaleOnResize( !GetActiveDrawingPart()->GetCanvas()->GetScaleOnResize() );
    }
    else if ( event.GetId() == CmdMenu_Option_ReverseLayers().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;
        GetActiveDrawingPart()->SetReverseOrder( !GetActiveDrawingPart()->GetReverseOrder() );
    }
    else if ( event.GetId() == CmdMenu_SetSnapFeatures().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_SetSnapFeature().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_SetStroke().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_SetFill().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_SetStrokeColour().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_SetFillColour().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_SetContourWidth().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_SetPathType().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_SetNormalizeFactor().GetId() )
    {
        found = true;
        if ( !m_drawing )
            return;

        double norm = wxGetDoubleNumberFromUser( _( "Give normalize factor:" ), _( "Normalize" ), _( "Normalize factor" ), 1, 0, 10000 );

        if (  norm != DBL_MAX )
        {
            if ( norm >= 0 && norm < 10000 )
            {
                m_drawing->SetNormalizeScale( norm );
                m_drawing->GetHabitat()->SetTarget( norm );
            }
            else
                ( void )wxMessageBox( _( "Normlaize factor must be > 0" ), _( "Normalize" ), wxICON_INFORMATION | wxOK );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDiagram().GetId() )
    {
        found = true;
        if ( !m_drawing )
            return;

        if ( !GetActiveDrawingPart() )
            return;
        if ( !GetActiveDrawingPart()->GetDrawer2D() )
            return;
        if ( ! GetActiveDrawingPart()->GetShowObject() )
            return;

        a2dCameleon* cam = wxDynamicCast( GetActiveDrawingPart()->GetShowObject()->GetRoot()->GetParent(), a2dCameleon );
        if ( !cam )
        {
            a2dAppear* appear = wxDynamicCast( GetActiveDrawingPart()->GetShowObject()->GetRoot()->GetParent(), a2dAppear );
            if ( appear && appear->GetCameleon() )
                cam = appear->GetCameleon();
        }
        if ( cam )
        {
            a2dDiagram* diagram = cam->GetDiagram();
            if ( ! diagram )
            {
                int answer = wxMessageBox("No Diagram found, Create it?:", "Diagram Creation",
                                        wxYES_NO | wxCANCEL );
                if (answer == wxYES)
                {
                    diagram = cam->GetDiagram( true );
                }
                else 
                    return;
            }
            a2dDrawing* drawing = diagram->GetDrawing(); 
            GetActiveDrawingPart()->SetShowObject( drawing->GetRootObject() );
            GetActiveDrawingPart()->SetMappingShowAll();
        }
        else
            ( void )wxMessageBox( _( "Does not contain appearance asked for" ), _( "Set View" ), wxICON_INFORMATION | wxOK );
    }
    else if ( event.GetId() == CmdMenu_ShowSymbol().GetId() )
    {
        found = true;
        if ( !m_drawing )
            return;

        if ( !GetActiveDrawingPart() )
            return;
        if ( !GetActiveDrawingPart()->GetDrawer2D() )
            return;
        if ( ! GetActiveDrawingPart()->GetShowObject() )
            return;

        a2dCameleon* cam = wxDynamicCast( GetActiveDrawingPart()->GetShowObject()->GetRoot()->GetParent(), a2dCameleon );
        if ( !cam )
        {
            a2dAppear* appear = wxDynamicCast( GetActiveDrawingPart()->GetShowObject()->GetRoot()->GetParent(), a2dAppear );
            if ( appear && appear->GetCameleon() )
                cam = appear->GetCameleon();
        }
        if ( cam )
        {
            a2dSymbol* symbol = cam->GetSymbol();
            if ( ! symbol )
            {
                int answer = wxMessageBox("No Symbol found, Create it?:", "Symbol Creation",
                                        wxYES_NO | wxCANCEL );
                if (answer == wxYES)
                {
                    symbol = cam->GetSymbol( true );
                }
                else 
                    return;
            }
            a2dDrawing* drawing = symbol->GetDrawing(); 
            GetActiveDrawingPart()->SetShowObject( drawing->GetRootObject() );
            GetActiveDrawingPart()->SetMappingShowAll();
        }
        else
            ( void )wxMessageBox( _( "Does not contain appearance asked for" ), _( "Set View" ), wxICON_INFORMATION | wxOK );

    }
    else if ( event.GetId() == CmdMenu_ShowGui().GetId() )
    {
        found = true;
        if ( !m_drawing )
            return;

        if ( !GetActiveDrawingPart() )
            return;
        if ( !GetActiveDrawingPart()->GetDrawer2D() )
            return;
        if ( ! GetActiveDrawingPart()->GetShowObject() )
            return;

        a2dAppear* appear = wxDynamicCast( GetActiveDrawingPart()->GetShowObject()->GetRoot()->GetParent(), a2dAppear );
        if ( appear && appear->GetCameleon() )
        {
            a2dGui* gui = appear->GetCameleon()->GetGui();
            if ( ! gui )
            {
                int answer = wxMessageBox("No Gui found, Create it?:", "Gui Creation",
                                        wxYES_NO | wxCANCEL );
                if (answer == wxYES)
                {
                    gui = appear->GetCameleon()->GetGui( true );
                }
                else 
                    return;
            }
            //a2dDrawing* drawing = symbol->GetDrawing(); 
            // GetActiveDrawingPart()->SetShowObject( drawing->GetRootObject() );
            // GetActiveDrawingPart()->SetMappingShowAll();
        }
        else
            ( void )wxMessageBox( _( "Does not contain appearance asked for" ), _( "Set View" ), wxICON_INFORMATION | wxOK );
    }

    if ( found )
    {}
    else if ( event.GetId() == CmdMenu_PushInto().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;
        if ( ! GetActiveDrawingPart()->GetShowObject() )
            return;

        a2dCanvasObject* pushIn = GetActiveDrawingPart()->GetShowObject()->Find( wxT( "" ),  wxT( "" ), a2dCanvasOFlags::SELECTED );

        if ( pushIn )
            pushIn = pushIn->PushInto( GetActiveDrawingPart()->GetShowObject() );
        else if ( GetActiveDrawingPart()->GetShowObject()->GetChildObjectsCount() == 1 )
        {
            pushIn = *(GetActiveDrawingPart()->GetShowObject()->GetChildObjectList()->begin());
            pushIn = pushIn->PushInto( GetActiveDrawingPart()->GetShowObject() );
        }
        if ( pushIn )
        {
            pushIn->SetSelected( false );
            pushIn->SetSelected2( false );
            pushIn->GetChildObjectList()->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED | a2dCanvasOFlags::SELECTED2 );
            GetActiveDrawingPart()->PushIn( pushIn );
            GetActiveDrawingPart()->SetMappingShowAll();
        }
        else
            ( void )wxMessageBox( _( "Does not contain a group, unable to push into" ), _( "push into" ), wxICON_INFORMATION | wxOK );
    }
    else if ( event.GetId() == CmdMenu_PopOut().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;
        if ( !GetActiveDrawingPart()->GetDrawer2D() )
            return;
        if ( ! GetActiveDrawingPart()->GetShowObject() )
            return;

        GetActiveDrawingPart()->PopOut();
        GetActiveDrawingPart()->SetMappingShowAll();
    }
    else if ( event.GetId() == CmdMenu_NewGroup().GetId() )
    {
        found = true;
        Submit( new a2dCommand_NewGroup( top, a2dCommand_NewGroup::Args() ) );
    }
    else if ( event.GetId() == CmdMenu_NewPin().GetId() )
    {
        found = true;
        Submit( new a2dCommand_NewPin( top, a2dCommand_NewPin::Args() ) );
    }
    else if ( event.GetId() == CmdMenu_SaveLayersDrawing().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of output Layer file" ),
                                 _T( "@{layerFileApplication}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
        SaveLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_SaveLayersDrawingLastDir().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of output Layer file" ),
                                 _T( "@{lastLayerFileSavePath}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
        SaveLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_SaveLayersDrawingHome().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of output Layer file" ),
                                 _T( "@{layerFileSavePath}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
        SaveLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_SaveLayersDrawingCurrentDir().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of output Layer file" ),
                                 _T( "./" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
        SaveLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_LoadLayersDrawing().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of input Layer file" ),
                                 _T( "@{layerFileApplication}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_OPEN | wxFD_FILE_MUST_EXIST );
        LoadLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_LoadLayersDrawingHome().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of input Layer file" ),
                                 _T( "@{layerFileSavePath}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_OPEN | wxFD_FILE_MUST_EXIST );
        LoadLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_LoadLayersDrawingLastDir().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of input Layer file" ),
                                 _T( "@{lastLayerFileSavePath}" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_OPEN | wxFD_FILE_MUST_EXIST );
        LoadLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_LoadLayersDrawingCurrentDir().GetId() )
    {
        found = true;
        wxString file = AskFile( _( "Give Name of input Layer file" ),
                                 _T( "./" ),
                                 _T( "" ),
                                 _T( "*.cvg" ),
                                 _T( "*.cvg" ),
                                 wxFD_OPEN | wxFD_FILE_MUST_EXIST );
        LoadLayers( wxFileName( file ) );
    }
    else if ( event.GetId() == CmdMenu_AddLayerIfMissing().GetId() )
    {
        found = true;

        wxString layerName = wxGetTextFromUser( "give name for new layer:", "Add Layer", "" );
        if ( !layerName.IsEmpty() )
        {
            m_drawing->GetLayerSetup()->AddIfMissing( layerName );

            a2dComEvent changedlayer( m_drawing, m_drawing->GetLayerSetup(), a2dDrawing::sig_layersetupChanged );
            ProcessEvent( changedlayer );
        }
    }
    else if ( event.GetId() == CmdMenu_SetCursor().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_SetRelativeStart().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_AddMeasurement().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_ClearMeasurements().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_Selected_SelectAll().GetId() )
    {
        found = true;
        Submit( new a2dCommand_Select( top, a2dCommand_Select::Args().what( a2dCommand_Select::SelectAll ) ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_DeSelectAll().GetId() )
    {
        found = true;
        Submit( new a2dCommand_Select( top, a2dCommand_Select::Args().what( a2dCommand_Select::DeSelectAll ) ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_SelectAll_NoUndo().GetId() )
    {
        found = true;
        Submit( new a2dCommand_Select( top, a2dCommand_Select::Args().what( a2dCommand_Select::SelectAll ) ), false );
    }
    else if ( event.GetId() == CmdMenu_Selected_DeSelectAll_NoUndo().GetId() )
    {
        found = true;
        Submit( new a2dCommand_Select( top, a2dCommand_Select::Args().what( a2dCommand_Select::DeSelectAll ) ), false );
    }
    else if ( event.GetId() == CmdMenu_Selected_Delete().GetId() )
    {
        found = true;
        Submit( new a2dCommand_DeleteMask( top ), true );
    }
    else if ( event.GetId() == CmdMenu_Selected_GroupNamed().GetId() )
    {
        found = true;
		wxString groupName = wxGetTextFromUser( "give name for group:", "Group Naming", "" );
        Submit( new a2dCommand_GroupMask( top, groupName ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Group().GetId() )
    {
        found = true;
		wxString groupName;
        Submit( new a2dCommand_GroupMask( top, groupName ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_ImageToRectangles().GetId() )
    {
        found = true;
        Submit( new a2dCommand_ImagesToRectMask( top ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_ToTop().GetId() )
    {
        found = true;
        Submit( new a2dCommand_ToTopMask( top ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_ToBack().GetId() )
    {
        found = true;
        Submit( new a2dCommand_ToBackMask( top ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_SetStyle().GetId() )
    {
        found = true;

        if ( !m_styledlg )
        {
            m_styledlg  = new a2dStyleDialog( m_drawing->GetHabitat(), NULL, style | wxRESIZE_BORDER  );
        }

        m_styledlg->SetFill( m_drawing->GetHabitat()->GetFill() );
        m_styledlg->SetStroke( m_drawing->GetHabitat()->GetStroke() );

        if ( ! m_styledlg->IsShown() )
            m_styledlg->Show( true );

        Submit( new a2dCommand_SetFillStrokeMask( top ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_SetExtStyle().GetId() )
    {
        found = true;

        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dCanvasObject* top = drawer->GetShowObject();

        a2dExtFill eFi;
        a2dExtStroke eSt;
        wxUint32 nrst = 0;
        wxUint32 nrfi = 0;

        if ( !top || !top->GetRoot() )
            return;

        bool withFill = false;
        a2dCanvasObjectList* objects = top->GetChildObjectList();
        nrst = a2dSetExtStroke( eSt, objects, a2dCanvasOFlags::SELECTED, drawer->GetDrawing()->GetLayerSetup() );
        nrfi = a2dSetExtFill( eFi, objects, a2dCanvasOFlags::SELECTED, drawer->GetDrawing()->GetLayerSetup() );
        if ( nrst + nrfi == 0 )
        {
            eFi.Set( m_drawing->GetHabitat()->GetFill() );  
            eSt.Set( m_drawing->GetHabitat()->GetStroke() ); 
            withFill = true; // we want to be able to choose a fill
        }
        else
            withFill = 0 != nrfi;

        a2dDialogStyle dlg( NULL, 0 == nrst + nrfi, withFill, true );
        	dlg.SetUnitsScale( drawer->GetDrawing()->GetUnitsScale() );
            dlg.SetExtFill(eFi);
            dlg.SetExtStroke(eSt);

	    if(wxID_OK == dlg.ShowModal())
	    {
		    eSt = dlg.GetExtStroke(); // get edited ExtStroke from dialog
		    eFi = dlg.GetExtFill(); // get edited ExtFill from dialog

            if ( nrst + nrfi == 0 )
            {
	            m_drawing->GetHabitat()->SetFill( eFi.Get( m_drawing->GetHabitat()->GetFill() ) );
	            m_drawing->GetHabitat()->SetStroke( eSt.Get( m_drawing->GetHabitat()->GetStroke() ) );
            }
            else
		        Submit( new a2dCommand_SetFillStrokeExtMask( top, eSt, eFi, a2dCanvasOFlags::BIN2 ) );
        }
    }
    else if ( event.GetId() == CmdMenu_Selected_SetTextChanges().GetId() )
    {
        found = true;

        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dCanvasObject* top = drawer->GetShowObject();

        a2dExtStroke eSt;
		a2dExtFill eFi;
        a2dTextChanges eFo;
		wxUint32 nrst = 0;
        wxUint32 nrfi = 0;
        wxUint32 nrfo = 0;

        if ( !top || !top->GetRoot() )
            return;

        bool withFill = false;

        a2dCanvasObjectList* objects = top->GetChildObjectList();
		a2dCanvasObjectList textobjects;

		//filter objects
		forEachIn( a2dCanvasObjectList, objects )
		{
			a2dCanvasObject* obj = *iter;
    		a2dText* text = wxDynamicCast( obj, a2dText );

			if ( text )
				textobjects.push_back( obj );
		}
		objects = &textobjects;
        
		nrst = a2dSetExtStroke( eSt, objects, a2dCanvasOFlags::SELECTED, drawer->GetDrawing()->GetLayerSetup() );
        nrfi = a2dSetExtFill( eFi, objects, a2dCanvasOFlags::SELECTED, drawer->GetDrawing()->GetLayerSetup() );
		nrfo = a2dSetTextChanges( eFo, objects, a2dCanvasOFlags::SELECTED, drawer->GetDrawing()->GetLayerSetup() );

        if ( nrst + nrfi + nrfo == 0 )
        {
			eFi.Set( m_drawing->GetHabitat()->GetTextFill() );
            eSt.Set( m_drawing->GetHabitat()->GetTextStroke() );
            eFo = a2dTextChanges( m_drawing->GetHabitat()->GetTextFont(), m_drawing->GetHabitat()->GetTextTemplateObject()->GetTextFlags(), m_drawing->GetHabitat()->GetTextTemplateObject()->GetAlignment() ); 

			withFill = true; // we want to be able to choose a fill  
        }
		else
			withFill = 0 != nrfi;

		a2dTextPropDlgExt dlg( m_drawing->GetHabitat(), NULL, 0 == nrst + nrfi + nrfo, withFill, eFo.GetFontInfoList(), false );
		dlg.SetUnitsScale( drawer->GetDrawing()->GetUnitsScale() );
		dlg.SetExtFill(eFi);
		dlg.SetExtStroke(eSt);
		dlg.SetExtFont(eFo);
		//dlg.SetCustomColors( XXX.GetCustomColors());
        if ( dlg.ShowModal() == wxID_OK )
        {
			eSt = dlg.GetExtStroke(); // get edited ExtStroke from dialog
			eFi = dlg.GetExtFill(); // get edited ExtFill from dialog
			eFo = dlg.GetExtFont(); // get edited ExtFont from dialog

			if ( nrst + nrfi + nrfo == 0 )
			{
				m_drawing->GetHabitat()->SetTextFill( eFi.Get( m_drawing->GetHabitat()->GetTextFill() ) );
	            m_drawing->GetHabitat()->SetTextStroke( eSt.Get( m_drawing->GetHabitat()->GetTextStroke() ) );
				m_drawing->GetHabitat()->SetTextFont( eFo.GetFont( m_drawing->GetHabitat()->GetTextFont() ) );
				m_drawing->GetHabitat()->GetTextTemplateObject()->SetAlignment( eFo.GetAlignment( m_drawing->GetHabitat()->GetTextTemplateObject()->GetAlignment() ) );
				m_drawing->GetHabitat()->GetTextTemplateObject()->SetTextFlags( eFo.GetTextFlags( m_drawing->GetHabitat()->GetTextTemplateObject()->GetTextFlags() ) );
			}
			else
            {
			    drawer->GetDrawing()->GetCanvasCommandProcessor()->Submit( new a2dCommand_SetFillStrokeExtMask( top, eSt, eFi, a2dCanvasOFlags::BIN2 ) );
				drawer->GetDrawing()->GetCanvasCommandProcessor()->Submit( new a2dCommand_SetTextChangesMask( top, eFo, eFo.GetAlignment(), a2dCanvasOFlags::BIN2 ) );
            }
        }
        objects->SetSpecificFlags( false, a2dCanvasOFlags::BIN2 );
    }
    else if ( event.GetId() == CmdMenu_Selected_SetTextChanges2().GetId() )
    {
        found = true;

        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dCanvasObject* top = drawer->GetShowObject();

        a2dTextChanges eFont;
        wxUint32 nrfont = 0;

        if ( !top || !top->GetRoot() )
            return;

        bool withFill = false;
        a2dCanvasObjectList* objects = top->GetChildObjectList();
        nrfont = SetTextChanges( eFont, objects, a2dCanvasOFlags::SELECTED, drawer->GetDrawing()->GetLayerSetup() );
        if ( nrfont == 0 )
        {
            eFont = a2dTextChanges( m_drawing->GetHabitat()->GetTextTemplateObject()->GetFont(), m_drawing->GetHabitat()->GetTextTemplateObject()->GetTextFlags(), m_drawing->GetHabitat()->GetTextTemplateObject()->GetAlignment() );  
        }

        a2dTextPropDlg dialog( NULL, eFont.GetFont(), eFont.GetTextFlags(), eFont.GetAlignment() );
        if ( dialog.ShowModal() == wxID_OK )
        {
			eFont = a2dTextChanges( dialog.GetFontData(), dialog.GetAlignment(), dialog.GetTextFlags() );
			if ( nrfont == 0 )
			{
				m_drawing->GetHabitat()->GetTextTemplateObject()->SetFont( eFont.GetFont( m_drawing->GetHabitat()->GetTextTemplateObject()->GetFont() ) );
                m_drawing->GetHabitat()->GetTextTemplateObject()->SetAlignment( dialog.GetAlignment() );
                m_drawing->GetHabitat()->GetTextTemplateObject()->SetTextFlags( dialog.GetTextFlags() );
			}
			else
            {
			    drawer->GetDrawing()->GetCanvasCommandProcessor()->Submit( new a2dCommand_SetTextChangesMask( top, eFont, dialog.GetAlignment(), a2dCanvasOFlags::BIN2 ) );
            }
        }
        objects->SetSpecificFlags( false, a2dCanvasOFlags::BIN2 );
    }
    else if ( event.GetId() == CmdMenu_Selected_ConvertToPolygonPolylinesWithArcs().GetId() )
    {
        found = true;
        Submit( new a2dCommand_ConvertMask( top, a2dCommand_ConvertMask::ConvertToPolygonPolylinesWithArcs ), true );
    }
    else if ( event.GetId() == CmdMenu_Selected_ConvertToPolygonPolylinesWithoutArcs().GetId() )
    {
        found = true;
        Submit( new a2dCommand_ConvertMask( top, a2dCommand_ConvertMask::ConvertToPolygonPolylinesWithoutArcs ), true );
    }
    else if ( event.GetId() == CmdMenu_Selected_ConvertToPolylines().GetId() )
    {
        found = true;
        Submit( new a2dCommand_ConvertMask( top, a2dCommand_ConvertMask::ConvertToPolylines ), true );
    }
    else if ( event.GetId() == CmdMenu_Selected_ConvertLinesArcs().GetId() )
    {
        found = true;
        Submit( new a2dCommand_ConvertMask( top, a2dCommand_ConvertMask::ConvertLinesArcs ), true );
    }
    else if ( event.GetId() == CmdMenu_Selected_CameleonDiagram().GetId() )
    {
        found = true;
        Submit( new a2dCommand_CreateCameleonMask( top, wxT( "" ), a2dCanvasOFlags::SELECTED, a2dCommand_CreateCameleonMask::CreateDiagram ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_CameleonSymbol().GetId() )
    {
        found = true;
        Submit( new a2dCommand_CreateCameleonMask( top, wxT( "" ), a2dCanvasOFlags::SELECTED, a2dCommand_CreateCameleonMask::CreateSymbol ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_CameleonSymbolDiagram().GetId() )
    {
        found = true;
        Submit( new a2dCommand_CreateCameleonMask( top, wxT( "" ), a2dCanvasOFlags::SELECTED, a2dCommand_CreateCameleonMask::CreateSymbolDiagram ) );
    }    
    else if ( event.GetId() == CmdMenu_Selected_FlattenCameleon().GetId() )
    {
        found = true;
        Submit( new a2dCommand_FlattenCameleonMask( top, wxT( "" ) ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_CloneCameleonFromInst().GetId() )
    {
        found = true;
        Submit( new a2dCommand_CloneCameleonFromInstMask( top ) );
    }
    if ( found )
    {
    }
    else if ( event.GetId() == CmdMenu_Selected_ConvertToVPaths().GetId() )
    {
        found = true;
        Submit( new a2dCommand_ConvertMask( top, a2dCommand_ConvertMask::ConvertToVPaths ), true );
    }
    else if ( event.GetId() == CmdMenu_Selected_ConvertPolygonToArcs().GetId() )
    {
        found = true;
        Submit( new a2dCommand_ConvertMask( top, a2dCommand_ConvertMask::ConvertPolygonToArcs ), true );
    }
    else if ( event.GetId() == CmdMenu_Selected_ConvertPolylineToArcs().GetId() )
    {
        found = true;
        Submit( new a2dCommand_ConvertMask( top, a2dCommand_ConvertMask::ConvertPolylineToArcs ), true );
    }
    else if ( event.GetId() == CmdMenu_Selected_ConvertPathToPolygon().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_Selected_ConvertSurfaceToPolygon().GetId() )
    {
        found = true;
        Submit( new a2dCommand_SurfaceToPolygonMask( top ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_ConvertPolygonToSurface().GetId() )
    {
        found = true;
        Submit( new a2dCommand_PolygonToSurfaceMask( top ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_UnGroup().GetId() )
    {
        found = true;
        Submit( new a2dCommand_UnGroupMask( top ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_UnGroupDeep().GetId() )
    {
        found = true;
        a2dCommand_UnGroupMask* command = new a2dCommand_UnGroupMask( top, true );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_Selected_Merge().GetId() )
    {
        found = true;
        a2dCommand_MergeMask* command = new a2dCommand_MergeMask( top );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_Selected_Offset().GetId() )
    {
        found = true;
        a2dCommand_OffsetMask* command = new a2dCommand_OffsetMask( top );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_Selected_CreateRing().GetId() )
    {
        found = true;
        a2dCommand_CreateRingMask* command = new a2dCommand_CreateRingMask( top );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_Offset().GetId() )
    {
        found = true;
        a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
                what( a2dCommand_GroupAB::Boolean_CORRECTION ).
                selectedA( true ) );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_Smooth().GetId() )
    {
        found = true;
        a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
                what( a2dCommand_GroupAB::Boolean_SMOOTHEN ).
                selectedA( true ) );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_CreateRing().GetId() )
    {
        found = true;
        a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
                what( a2dCommand_GroupAB::Boolean_MAKERING ).
                selectedA( true ) );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_Delete().GetId() )
    {
        found = true;
        Submit( new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().what( a2dCommand_GroupAB::DeleteGroupA ) ) );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_Move().GetId() )
    {
        found = true;
        Submit( new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().what( a2dCommand_GroupAB::MoveGroupA ) ) );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_Copy().GetId() )
    {
        found = true;
        Submit( new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().what( a2dCommand_GroupAB::CopyGroupA ) ) );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_ToArcs().GetId() )
    {
        found = true;
        Submit( new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().what( a2dCommand_GroupAB::ConvertToArcs ) ) );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_Or().GetId() )
    {
        found = true;
        a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
                what( a2dCommand_GroupAB::Boolean_OR ) );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_And().GetId() )
    {
        found = true;
        a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
                what( a2dCommand_GroupAB::Boolean_AND ) );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_Exor().GetId() )
    {
        found = true;
        a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
                what( a2dCommand_GroupAB::Boolean_EXOR ) );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_AsubB().GetId() )
    {
        found = true;
        a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
                what( a2dCommand_GroupAB::Boolean_A_SUB_B ) );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_BsubA().GetId() )
    {
        found = true;
        a2dCommand_GroupAB* command = new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().
                what( a2dCommand_GroupAB::Boolean_B_SUB_A ) );
        Submit( command );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_ConvertSurfaceToPolygon().GetId() )
    {
        found = true;
        Submit( new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().what( a2dCommand_GroupAB::Boolean_Surface2Polygon ) ) );
    }
    else if ( event.GetId() == CmdMenu_GroupAB_ConvertPointsAtDistance().GetId() )
    {
        found = true;
        Submit( new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().what( a2dCommand_GroupAB::ConvertPointsAtDistance ) ) );
    } 
    else if ( event.GetId() == CmdMenu_GroupAB_ConvertPolygonToSurface().GetId() )
    {
        found = true;
        Submit( new a2dCommand_GroupAB( top, a2dCommand_GroupAB::Args().what( a2dCommand_GroupAB::Boolean_Polygon2Surface ) ) );
    }

    else if ( event.GetId() == CmdMenu_PopTool().GetId() )
    {
        found = true;
    }
    else if ( event.GetId() == CmdMenu_PopTool().GetId() )
    {
    }
    else if ( event.GetId() == CmdMenu_InsertGroupRef().GetId() )
    {
        found = true;
    }
    /*
    else if ( event.GetId() == CmdMenu_LineBegin().GetId() )
    {
        found = true;
        a2dCanvasObjectList total;

        if ( !m_librarypoints )
        {
            wxMessageBox( _( "point library not loaded" ), _( "Points" ), wxICON_INFORMATION | wxOK );
            return;
        }
        m_librarypoints->GetDrawing()->CollectObjects( &total, _T( "a2dLibraryReference" ), a2dCanvasOFlags::ALL );

        a2dCanvasObjectsDialog objects( NULL, &total, true, ( wxSTAY_ON_TOP | wxRESIZE_BORDER | wxCAPTION ) );
        if ( objects.ShowModal() == wxID_OK )
        {
            a2dCanvasObject* begin = ( ( a2dLibraryReference* )objects.GetCanvasObject() )->GetCanvasObject();
            begin->SetPosXY( 0, 0 );
			m_drawing->GetHabitat()->SetLineBegin( begin );
        }
    }
    
    else if ( event.GetId() == CmdMenu_LineEnd().GetId() )
    {
        found = true;
        a2dCanvasObjectList total;

        if ( !m_librarypoints )
        {
            wxMessageBox( _( "point library not loaded" ), _( "Points" ), wxICON_INFORMATION | wxOK );
            return;
        }
        m_librarypoints->GetDrawing()->CollectObjects( &total, _T( "a2dLibraryReference" ), a2dCanvasOFlags::ALL );

        a2dCanvasObjectsDialog objects( NULL, &total, true, ( wxSTAY_ON_TOP | wxRESIZE_BORDER | wxCAPTION ) );
        if ( objects.ShowModal() == wxID_OK )
        {
            a2dCanvasObject* end = ( ( a2dLibraryReference* )objects.GetCanvasObject() )->GetCanvasObject();
            end->SetPosXY( 0, 0 );
		    m_drawing->GetHabitat()->SetLineEnd( end );
        }
    }
*/
    else if ( event.GetId() == CmdMenu_LineScale().GetId() )
    {
        found = true;
        long w = wxGetNumberFromUser( _( "Give scalex:" ), _( "scalex:" ), _( "scale in x for point" ), 1, 0, 100 );
        if ( w == 0 ) w = 1;
        m_drawing->GetHabitat()->SetEndScaleX( ( int ) w );
        w = wxGetNumberFromUser( _( "Give scaley:" ), _( "scaley:" ), _( "scale in y for point" ), 1, 0, 100 );
        if ( w == 0 ) w = 1;
        m_drawing->GetHabitat()->SetEndScaleY( ( int ) w );
    }

    if ( !found && event.GetId() == CmdMenu_Selected_MoveXY().GetId() )
    {
        found = true;
        m_drawing->GetHabitat()->SetLastXyEntry( 0, 0 );
        a2dCoordinateEntry entry( NULL , -1, true, _( "XY for copy" ) );
        if ( entry.ShowModal() == wxID_OK )
        {
            Submit( new a2dCommand_MoveMask( top, entry.m_xcalc, entry.m_ycalc, -1, false ) );
        }
    }
    else if ( event.GetId() == CmdMenu_Selected_MoveLayer().GetId() )
    {
        found = true;
        if ( !m_drawing )
            return;
        a2dLayerDlg dialog( m_drawing->GetHabitat(), NULL, m_drawing->GetLayerSetup(), false, true, _( "Set Target Layer" ) );
        if ( dialog.ShowModal() == wxID_OK )
        {
            m_drawing->GetHabitat()->SetTarget( dialog.GetSelectedLayer() );
            Submit( new a2dCommand_MoveMask( top, 0, 0, -1, true ) );
        }
    }
    else if ( event.GetId() == CmdMenu_Selected_CopyXY().GetId() )
    {
        found = true;
        m_drawing->GetHabitat()->SetLastXyEntry( 0, 0 );
        a2dCoordinateEntry entry( NULL , -1, true, _( "XY for copy" ) );
        if ( entry.ShowModal() == wxID_OK )
        {
            Submit( new a2dCommand_CopyMask( top, entry.m_xcalc, entry.m_ycalc, -1, false ) );
        }
    }
    else if ( event.GetId() == CmdMenu_Selected_CopyLayer().GetId() )
    {
        found = true;
        if ( !m_drawing )
            return;
        a2dLayerDlg dialog( m_drawing->GetHabitat(), NULL, m_drawing->GetLayerSetup(), false, true, _( "Set Target Layer" ) );
        if ( dialog.ShowModal() == wxID_OK )
        {
            m_drawing->GetHabitat()->SetTarget( dialog.GetSelectedLayer() );
            Submit( new a2dCommand_CopyMask( top, 0, 0, -1, true ) );
        }
    }
    else if ( event.GetId() == CmdMenu_Selected_Transform().GetId() )
    {
        found = true;
        a2dTransDlg transset( NULL, true );
        if ( transset.ShowModal() == wxID_OK )
        {
            Submit( new a2dCommand_TransformMask( top, transset.m_lworld ) );
        }
    }
    else if ( event.GetId() == CmdMenu_Selected_Rotate().GetId() )
    {
        found = true;
        double angle = wxGetDoubleNumberFromUser( _( "Give angle to rotate:" ), _( "Rotate" ), _( "Angle" ), 0, -180, 180 );

        if ( angle >= -180 && angle < 180 )
        {
            Submit( new a2dCommand_RotateMask( top, angle, true ) );
        }
        else
            ( void )wxMessageBox( _( "Angle must be > -180 and < 180" ), _( "Angle" ), wxICON_INFORMATION | wxOK );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MinX().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MINX ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MaxX().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MAXX ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MinY().GetId() )
    {
        found = true;
        if ( GetActiveDrawingPart()->GetDrawer2D()->GetYaxis() )
            Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MINY ) );
        else
            Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MAXY ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MaxY().GetId() )
    {
        found = true;
        if ( GetActiveDrawingPart()->GetDrawer2D()->GetYaxis() )
            Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MAXY ) );
        else
            Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MINY ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MidX().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MIDX ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MidY().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MIDY ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MirrorVert().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MirrorVert ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MirrorHorz().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MirrorHorz ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MirrorVertBbox().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MirrorVertBbox ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MirrorHorzBbox().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MirrorHorzBbox ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_DistributeVert().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::DistributeVert ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_DistributeHorz().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::DistributeHorz ) );
    }

    else if ( event.GetId() == CmdMenu_Selected_Align_MinX_Dest().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MINX_Dest ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MaxX_Dest().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MAXX_Dest ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MinY_Dest().GetId() )
    {
        found = true;
        if ( GetActiveDrawingPart()->GetDrawer2D()->GetYaxis() )
            Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MINY_Dest ) );
        else
            Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MAXY_Dest ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MaxY_Dest().GetId() )
    {
        found = true;
        if ( GetActiveDrawingPart()->GetDrawer2D()->GetYaxis() )
            Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MAXY_Dest ) );
        else
            Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MINY_Dest ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MidX_Dest().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MIDX_Dest ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MidY_Dest().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MIDY_Dest ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MirrorVert_Dest().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MirrorVert_Dest ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MirrorHorz_Dest().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MirrorHorz_Dest ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MirrorVertBbox_Dest().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MirrorVertBbox_Dest ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_Align_MirrorHorzBbox_Dest().GetId() )
    {
        found = true;
        Submit( new a2dCommand_AlignMask( top, a2dCommand_AlignMask::MirrorHorzBbox_Dest ) );
    }

    else if ( event.GetId() == CmdMenu_Selected_RotateObject90Right().GetId() )
    {
        found = true;
        Submit( new a2dCommand_RotateMask( top, 90, true ) );
    }
    else if ( event.GetId() == CmdMenu_Selected_RotateObject90Left().GetId() )
    {
        found = true;
        Submit( new a2dCommand_RotateMask( top, -90, true ) );
    }
    
    if ( !found && event.GetId() == CmdMenu_SetMasterTool_SelectFirst().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        a2dMasterDrawSelectFirst* tool = new a2dMasterDrawSelectFirst( contr );
        tool->SetAllowMultiEdit( false );
        contr->StopAllTools();
        contr->SetTopTool( tool );
    }
    else if ( event.GetId() == CmdMenu_SetMasterTool_ZoomFirst().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        a2dMasterDrawZoomFirst* tool = new a2dMasterDrawZoomFirst( contr );
        tool->SetAllowMultiEdit( false );
        contr->StopAllTools();
        contr->SetTopTool( tool );
    }
    else if ( event.GetId() == CmdMenu_SetMasterTool_TagGroups().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        a2dMasterTagGroups3* tool = new a2dMasterTagGroups3( contr );
        contr->StopAllTools();
        contr->SetTopTool( tool );
    }
    else if ( event.GetId() == CmdMenu_SetMasterTool_TagGroups_WireMode().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        a2dMasterTagGroups3* tool = new a2dMasterTagGroups3( contr );
        tool->SetWireMode( true );
        contr->StopAllTools();
        contr->SetTopTool( tool );
    }
    else if ( event.GetId() == CmdMenu_SetMasterTool_TagGroups_WireMode_DlgMode().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        a2dMasterTagGroups3* tool = new a2dMasterTagGroups3( contr );
        tool->SetWireMode( true );
        tool->SetDlgOrEdit( true );
		tool->SetStyleDlgSimple( true );
        tool->SetDlgOrEditModal( true );
        contr->StopAllTools();
        contr->SetTopTool( tool );
    }
    else if ( event.GetId() == CmdMenu_SetMasterTool_TagGroups_DlgMode().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        a2dMasterTagGroups3* tool = new a2dMasterTagGroups3( contr );
        //tool->SetWireMode( true );
		tool->SetStyleDlgSimple( true );
        tool->SetDlgOrEdit( true );
        tool->SetDlgOrEditModal( true );
        contr->StopAllTools();
        contr->SetTopTool( tool );
    }
    else if ( event.GetId() == CmdMenu_SetMasterTool_SingleClick().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        a2dStToolContr* contr = wxDynamicCast( drawer->GetCanvasToolContr(), a2dStToolContr );
        if ( !contr )
            return;

        a2dMasterSingleClick* tool = new a2dMasterSingleClick( contr );
        //tool->SetWireMode( true );
        //tool->SetDlgOrEdit( true );
        //tool->SetDlgOrEditModal( true );
        contr->StopAllTools();
        contr->SetTopTool( tool );
    }
    else if ( event.GetId() == CmdMenu_SetLayerDlg().GetId() )
    {
        found = true;

        a2dLayerDlg dialog( m_drawing->GetHabitat(), NULL, m_drawing->GetLayerSetup(), false, true, _( "Set Layer" ) );
        if ( dialog.ShowModal() == wxID_OK )
        {
            m_drawing->GetHabitat()->SetLayer( dialog.GetSelectedLayer(), false );
        }
    }
    else if ( event.GetId() == CmdMenu_SetTargetDlg().GetId() )
    {
        found = true;

        a2dLayerDlg dialog( m_drawing->GetHabitat(), NULL, m_drawing->GetLayerSetup(), false, true, _( "Set Target Layer" ) );
        if ( dialog.ShowModal() == wxID_OK )
        {
            m_drawing->GetHabitat()->SetTarget( dialog.GetSelectedLayer() );
        }
    }
    else if ( event.GetId() == CmdMenu_SetLayerDlgModeless().GetId() )
    {
        found = true;

        if ( !m_layerChooseDlg )
        {
            m_layerChooseDlg = new a2dLayerDlg( m_drawing->GetHabitat(), NULL, m_drawing->GetLayerSetup(), false, false, _( "Set Layer" ) );
        }

        if ( !m_layerChooseDlg->IsShown() )
        {
            m_layerChooseDlg->Init( m_drawing->GetLayerSetup() );
            m_layerChooseDlg->Show( true );
        }
        else
        {
            m_layerChooseDlg->Show( false );
        }
    }
    else if ( 
              event.GetId() == CmdMenu_ShowDlgLayersDrawing().GetId() ||
              event.GetId() == CmdMenu_ShowDlgLayersGlobal().GetId()
            )
    {
        found = true;

        if ( !m_layersdlg )
        {
            m_layersdlg = new LayerPropertiesDialog( NULL, m_drawing->GetLayerSetup(),  false, _( "Layer of Document" ),  style | wxRESIZE_BORDER );
        }

        if ( !m_layersdlg->IsShown() )
        {
            m_layersdlg->Show( true );
            if ( event.GetId() == CmdMenu_ShowDlgLayersDrawing().GetId() )
            {
                m_layersdlg->Init( m_drawing->GetLayerSetup() );
            }
            else
                m_layersdlg->Init( m_drawing->GetHabitat()->GetLayerSetup() );
        }
        else
        {
            m_layersdlg->Show( false );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgLayerOrderDrawing().GetId() ||
              event.GetId() == CmdMenu_ShowDlgLayerOrderGlobal().GetId() )
    {
        found = true;

        if ( !m_layerOrderDlg )
        {
            m_layerOrderDlg = new a2dLayerOrderDlg( m_drawing->GetHabitat(), NULL, NULL, _( "Layer of Document" ),  style | wxRESIZE_BORDER );
        }

        if ( !m_layerOrderDlg->IsShown() )
        {
            if ( event.GetId() == CmdMenu_ShowDlgLayerOrderDrawing().GetId() )
            {
                m_layerOrderDlg->Init( m_drawing->GetLayerSetup() );
            }
            else
                m_layerOrderDlg->Init( m_drawing->GetHabitat()->GetLayerSetup() );
            m_layerOrderDlg->Show( true );
        }
        else
        {
            m_layerOrderDlg->Show( false );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgTools().GetId() )
    {
        found = true;
        if ( !m_tooldlg )
            m_tooldlg = new ToolDlg( NULL );

        if ( m_tooldlg->IsShown() )
        {
            m_tooldlg->Show( false );
        }
        else
        {
            m_tooldlg->Show( true );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgStyle().GetId() )
    {
        found = true;
        if ( !m_styledlg )
        {
            m_styledlg  = new a2dStyleDialog( m_drawing->GetHabitat(), NULL, style | wxRESIZE_BORDER  );
        }

        m_styledlg->SetFill( m_drawing->GetHabitat()->GetFill() );
        m_styledlg->SetStroke( m_drawing->GetHabitat()->GetStroke() );

        if ( m_styledlg->IsShown() )
        {
            m_styledlg->Show( false );
        }
        else
        {
            m_styledlg->Show( true );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgSnap().GetId() )
    {
        found = true;
        if ( !m_snapSettings )
            m_snapSettings = new a2dSnapSettings( m_drawing->GetHabitat(), NULL );
        if ( m_snapSettings->IsShown() )
        {
            m_snapSettings->Show( false );
        }
        else
        {
            m_snapSettings->Show( true );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgGroups().GetId() )
    {
        found = true;
        a2dGroupDlg dialog( m_drawing->GetHabitat(), NULL, _( "choose layer groups" ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER  );
        if ( dialog.ShowModal() == wxID_OK )
        {
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgIdentify().GetId() )
    {
        found = true;
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;

        if ( !m_identifyDlg )
        {
            m_identifyDlg = new IdentifyDialog( NULL, -1, _( "Identify Selected" ) );
        }

        m_identifyDlg->Init( drawer->GetShowObject() );

        if ( !m_identifyDlg->IsShown() )
        {
            m_identifyDlg->Show( true );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgCoordEntry().GetId() )
    {
        found = true;
        if ( !m_coordEntry )
            m_coordEntry = new a2dCoordinateEntry( NULL );
        if ( m_coordEntry->IsShown() )
        {
            m_coordEntry->Show( false );
        }
        else
        {
            m_coordEntry->Show( true );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgMeasure().GetId() )
    {
        found = true;
        if ( !m_measure )
            m_measure = new a2dMeasureDlg( m_drawing->GetHabitat(), NULL );
        if ( m_measure->IsShown() )
        {
            m_measure->Show( false );
        }
        else
        {
            m_measure->Show( true );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgStructure().GetId() )
    {
        found = true;
        if ( !m_structDlg )
        {
            m_structDlg = new a2dCanvasObjectsDialog( NULL, m_drawing, true, false, ( wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxRESIZE_BORDER ) );
        }
        if ( m_structDlg->IsShown() )
        {
            m_structDlg->Show( false );
        }
        else
        {
            m_structDlg->Show( true );
            m_structDlg->Init( m_drawing );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgStructureModal().GetId() )
    {
        found = true;
        a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawingPart )
            return;

        a2dCanvasObjectsDialog groups( NULL, m_drawing, true, true, ( wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxRESIZE_BORDER ) );
        if ( groups.ShowModal() == wxID_OK )
        {
            a2dCommand_SetShowObject* command = new a2dCommand_SetShowObject( drawingPart, a2dCommand_SetShowObject::Args().canvasobject( groups.GetCanvasObject() ) );
            m_drawing->GetCanvasCommandProcessor()->Submit( command );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgCameleonModal().GetId() )
    {

        found = true;
        if ( a2dCameleon::GetCameleonRoot() )
        {
            a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
            if ( !drawingPart )
                return;

            a2dCanvasObjectsChooseDialog structDlg( NULL, a2dCameleon::GetCameleonRoot()->GetChildObjectList(), "" );
            if ( structDlg.ShowModal() == wxID_OK )
            {       
                a2dCameleon* cam = wxDynamicCast( structDlg.GetCanvasObject(), a2dCameleon );
                if ( cam )
                {
                    drawingPart->SetShowObject( cam->GetAppearances()->GetRootObject() );
/*
                    if ( cam->GetBuildIn() )
                    {
                        a2dCanvasObject* inst = cam->GetBuildIn()->GetBuildIn();
                        drawingPart->SetShowObject( inst )
                    }
                    else if ( cam->GetSymbol() )
                    {
                        a2dCameleonInst* caminst = new a2dCameleonInst( 0,0, cam->GetSymbol() );
                        drawingPart->SetShowObject( inst )
                    }
                    else if ( cam->GetDiagram() )
                    {
                        a2dCameleonInst* caminst = new a2dCameleonInst( 0,0, cam->GetDiagram() );
                        drawingPart->SetShowObject( inst )
                    }
*/
                    drawingPart->SetMappingShowAll();
                }
            }
        }
        else
            wxLogWarning( _T( "a2dCameleon::GetCameleonRoot() not available" ) );
    }
    else if ( event.GetId() == CmdMenu_ShowDlgPathSettings().GetId() )
    {
        found = true;
        if ( !m_pathsettings )
            m_pathsettings = new a2dPathSettings( m_drawing->GetHabitat(), NULL );
        if ( m_pathsettings->IsShown() )
        {
            m_pathsettings->Show( false );
        }
        else
        {
            m_pathsettings->Show( true );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgSettings().GetId() )
    {
        found = true;
        if ( !m_settings )
            m_settings = new a2dSettings( m_drawing->GetHabitat(), NULL );
        if ( m_settings->IsShown() )
        {
            m_settings->Show( false );
        }
        else
        {
            m_settings->Show( true );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgTrans().GetId() )
    {
        found = true;
        if ( !m_transset )
            m_transset = new a2dTransDlg( NULL );
        if ( m_transset->IsShown() )
        {
            m_transset->Show( false );
        }
        else
        {
            m_transset->Show( true );
        }
    }
    else if ( event.GetId() == CmdMenu_ShowDlgPropEdit().GetId() )
    {
        found = true;
        /*
                if ( !m_propEdit )
                    m_propEdit = new a2dPropertyEditorDlg(NULL );
                if ( m_propEdit->IsShown() )
                {
                    m_propEdit->Show(false);
                }
                else
                {
                    m_propEdit->Show(true);
                }
        */
    }
    else if ( event.GetId() == CmdMenu_SetFont().GetId() )
    {
        found = true;
        a2dTextPropDlg dialog( NULL );
        if ( dialog.ShowModal() == wxID_OK )
        {
            // Assume it for text
            //m_drawing->GetHabitat()->SetFont( dialog.GetFontData() );
    		m_drawing->GetHabitat()->SetTextFont( dialog.GetFontData() );
        }
    }
    else if ( event.GetId() == CmdMenu_PrintDrawing().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;

        Print( a2dPRINT_PrintDrawing );
    }
    else if ( event.GetId() == CmdMenu_PreviewDrawing().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;

        Preview( a2dPRINT_PreviewDrawing );
    }
    else if ( event.GetId() == CmdMenu_PrintDrawingPart().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;

        Print( a2dPRINT_PrintDrawingPart );
    }
    else if ( event.GetId() == CmdMenu_PreviewDrawingPart().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;

        Preview( a2dPRINT_PreviewDrawingPart );
    }


    if ( !found )
        event.Skip();
    else
        a2dGeneralGlobals->SendToLogTarget();
}

void a2dCanvasCommandProcessor::OnPushToolMenu( wxCommandEvent& event )
{
    bool found = false;

    if ( !m_drawing )
        return;

    a2dDrawing* drawing = m_drawing;

    a2dCanvasObject* top = GetActiveDrawingPart()->GetShowObject();

    //One shot versions
    if ( event.GetId() == CmdMenu_PushTool_Zoom_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dZoomTool::COMID_PushTool_Zoom, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Select_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dSelectTool::COMID_PushTool_Select, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Select2_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dSelectTool::COMID_PushTool_Select, true, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_RecursiveEdit_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dRecursiveEditTool::COMID_PushTool_RecursiveEdit, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_MultiEdit_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dMultiEditTool::COMID_PushTool_MultiEdit, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DragOriginal_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDragOrgTool::COMID_PushTool_DragOrg, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Drag_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDragTool::COMID_PushTool_Drag, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Copy_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dCopyTool::COMID_PushTool_Copy, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Rotate_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dRotateTool::COMID_PushTool_Rotate, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Delete_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDeleteTool::COMID_PushTool_Delete, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawText_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawTextTool::COMID_PushTool_DrawText, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Image_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dImageTool::COMID_PushTool_Image, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Image_Embedded_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dImageTool::COMID_PushTool_Image_Embedded, false, true);
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawLine_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawLineTool::COMID_PushTool_DrawLine, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawLineScaledArrow_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawLineTool::COMID_PushTool_DrawLineScaledArrow, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipse_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawEllipseTool::COMID_PushTool_DrawEllipse, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipticArc_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawEllipticArcTool::COMID_PushTool_DrawEllipticArc, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipticArc_Chord_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawEllipticArcTool::COMID_PushTool_DrawEllipticArc_Chord, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawArc_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawArcTool::COMID_PushTool_DrawArc, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawArc_Chord_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawArcTool::COMID_PushTool_DrawArc_Chord, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolylineL_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawPolylineLTool::COMID_PushTool_DrawPolylineL, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolygonL_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawPolygonLTool::COMID_PushTool_DrawPolygonL, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolylineL_Splined_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawPolylineLTool::COMID_PushTool_DrawPolylineL_Splined, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolygonL_Splined_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawPolygonLTool::COMID_PushTool_DrawPolygonL_Splined, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Property_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dPropertyTool::COMID_PushTool_Property, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Measure_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dMeasureTool::COMID_PushTool_Measure, false, true );
/*todo
        if ( !m_measure )
            m_measure = new a2dMeasureDlg( NULL );
        if ( !m_measure->IsShown() )
            m_measure->Show( true );
*/
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawWirePolylineL_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawWirePolylineLTool::COMID_PushTool_DrawWirePolylineL, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DragMulti_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDragMultiTool::COMID_PushTool_DragMulti, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CopyMulti_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dCopyMultiTool::COMID_PushTool_CopyMulti, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawRectangle_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawRectangleTool::COMID_PushTool_DrawRectangle, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawCircle_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawCircleTool::COMID_PushTool_DrawCircle, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawVPath_OneShot().GetId() )
    {
        found = true;
        PushTool( a2dDrawVPathTool::COMID_PushTool_DrawVPath, false, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Origin_OneShot().GetId() )
    {
        found = true;

        a2dOrigin* origin = new a2dOrigin();
	    origin->SetStroke( a2dStroke( wxColour( 0, 24, 255 ), 0 ) );
		origin->SetFill( a2dFill( wxColour( 255, 0, 0 ) ) );
        PushToolDragNew( origin, CmdMenu_PushTool_Origin_OneShot(), true, true );
    }

    //N shot versions
    else if ( event.GetId() == CmdMenu_PushTool_Zoom().GetId() )
    {
        found = true;
        PushTool( a2dZoomTool::COMID_PushTool_Zoom );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Select().GetId() )
    {
        found = true;
        PushTool( a2dSelectTool::COMID_PushTool_Select );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Select2().GetId() )
    {
        found = true;
        PushTool( a2dSelectTool::COMID_PushTool_Select, true );
    }
    else if ( event.GetId() == CmdMenu_PushTool_RecursiveEdit().GetId() )
    {
        found = true;
        PushTool( a2dRecursiveEditTool::COMID_PushTool_RecursiveEdit );
    }
    else if ( event.GetId() == CmdMenu_PushTool_MultiEdit().GetId() )
    {
        found = true;
        PushTool( a2dMultiEditTool::COMID_PushTool_MultiEdit );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DragOriginal().GetId() )
    {
        found = true;
        PushTool( a2dDragOrgTool::COMID_PushTool_DragOrg );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Drag().GetId() )
    {
        found = true;
        PushTool( a2dDragTool::COMID_PushTool_Drag );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Copy().GetId() )
    {
        found = true;
        PushTool( a2dCopyTool::COMID_PushTool_Copy );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Rotate().GetId() )
    {
        found = true;
        PushTool( a2dRotateTool::COMID_PushTool_Rotate );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Delete().GetId() )
    {
        found = true;
        PushTool( a2dDeleteTool::COMID_PushTool_Delete );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawText().GetId() )
    {
        found = true;
        PushTool( a2dDrawTextTool::COMID_PushTool_DrawText );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Image().GetId() )
    {
        found = true;
        PushTool( a2dImageTool::COMID_PushTool_Image );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Image_Embedded().GetId() )
    {
        found = true;
        PushTool( a2dImageTool::COMID_PushTool_Image_Embedded );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawLine().GetId() )
    {
        found = true;
        PushTool( a2dDrawLineTool::COMID_PushTool_DrawLine );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawLineScaledArrow().GetId() )
    {
        found = true;
        PushTool( a2dDrawLineTool::COMID_PushTool_DrawLineScaledArrow );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipse().GetId() )
    {
        found = true;
        PushTool( a2dDrawEllipseTool::COMID_PushTool_DrawEllipse );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipticArc().GetId() )
    {
        found = true;
        PushTool( a2dDrawEllipticArcTool::COMID_PushTool_DrawEllipticArc );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawEllipticArc_Chord().GetId() )
    {
        found = true;
        PushTool( a2dDrawEllipticArcTool::COMID_PushTool_DrawEllipticArc_Chord );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawArc().GetId() )
    {
        found = true;
        PushTool( a2dDrawArcTool::COMID_PushTool_DrawArc );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawArc_Chord().GetId() )
    {
        found = true;
        PushTool( a2dDrawArcTool::COMID_PushTool_DrawArc_Chord );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolylineL().GetId() )
    {
        found = true;
        PushTool( a2dDrawPolylineLTool::COMID_PushTool_DrawPolylineL );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolygonL().GetId() )
    {
        found = true;
        PushTool( a2dDrawPolygonLTool::COMID_PushTool_DrawPolygonL );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolylineL_Splined().GetId() )
    {
        found = true;
        PushTool( a2dDrawPolylineLTool::COMID_PushTool_DrawPolylineL_Splined );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawPolygonL_Splined().GetId() )
    {
        found = true;
        PushTool( a2dDrawPolygonLTool::COMID_PushTool_DrawPolygonL_Splined );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Measure().GetId() )
    {
        found = true;
        PushTool( a2dMeasureTool::COMID_PushTool_Measure );
/*todo
        if ( !m_measure )
            m_measure = new a2dMeasureDlg( NULL );
        if ( !m_measure->IsShown() )
            m_measure->Show( true );
*/
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawWirePolylineL().GetId() )
    {
        found = true;
        PushTool( a2dDrawWirePolylineLTool::COMID_PushTool_DrawWirePolylineL );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DragMulti().GetId() )
    {
        found = true;
        PushTool( a2dDragMultiTool::COMID_PushTool_DragMulti );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CopyMulti().GetId() )
    {
        found = true;
        PushTool( a2dCopyMultiTool::COMID_PushTool_CopyMulti );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawRectangle().GetId() )
    {
        found = true;
        PushTool( a2dDrawRectangleTool::COMID_PushTool_DrawRectangle );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawCircle().GetId() )
    {
        found = true;
        PushTool( a2dDrawCircleTool::COMID_PushTool_DrawCircle );
    }
    else if ( event.GetId() == CmdMenu_PushTool_DrawVPath().GetId() )
    {
        found = true;
        PushTool( a2dDrawVPathTool::COMID_PushTool_DrawVPath );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Port().GetId() )
    {
        found = true;

        a2dPort* port = new a2dPort( NULL, 0, 0, "" );
        //port->GetPin()->SetPinClass( a2dPinClass::Standard );
	    port->SetStroke( a2dStroke( wxColour( 0, 24, 255 ), 0 ) );
		port->SetFill( a2dFill( wxColour( 255, 0, 0 ) ) );
        PushToolDragNew( port, CmdMenu_PushTool_Port(), false );
    }
    else if ( event.GetId() == CmdMenu_PushTool_Cameleon().GetId() )
    {
        found = true;

        a2dCamCreate camdlg( NULL,  m_drawing->GetHabitat() );
        if ( camdlg.ShowModal() == wxID_OK )
        {          
            a2dCameleon* cam = camdlg.m_cameleon;
		    a2dCameleonEvent eventNewCameleon( m_drawing, cam );
		    ProcessEvent( eventNewCameleon );

            a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
            if ( !drawingPart )
                return;
            if ( drawingPart->GetShowObject() != cam->GetCameleonRoot() )
            {
                a2dCameleonInst* caminst = new a2dCameleonInst( 0,0, cam->GetDiagram() );
                PushToolDragNew( caminst, CmdMenu_PushTool_Cameleon() );
            }
        }
    }
    else if ( event.GetId() == CmdMenu_PushTool_Cameleon_BuildIn().GetId() )
    {
        found = true;
        wxString name = wxGetTextFromUser( _( "give Build in Canvas Object it's ClassName:" ) );
        wxObject* create = wxCreateDynamicObject( name );
        a2dCanvasObject* buildIn = wxDynamicCast( create, a2dCanvasObject );
        if ( buildIn )
        {
            a2dCameleon* cam = new a2dCameleon( wxT("cameleon_") );
		    a2dCameleonEvent eventNewCameleon( m_drawing, cam );
		    ProcessEvent( eventNewCameleon );

            a2dBuildIn* appear = new a2dBuildIn( cam, 0,0, buildIn );
            appear->SetName( name );
            cam->AddAppearance( appear );
            cam->SetName( name );
            //PushToolDragNew( cam, CmdMenu_PushTool_Cameleon_BuildIn().GetId() );
        }
        else
            wxLogWarning( _T( "This BuildIn object not available" ) );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CameleonInst().GetId() )
    {
        found = true;
        if ( a2dCameleon::GetCameleonRoot() )
        {
            wxString name = wxGetTextFromUser( _( "give a2dCameleon name:" ) );
            if ( ! name.IsEmpty() )
            {
                a2dCameleon* cam = wxDynamicCast( a2dCameleon::GetCameleonRoot()->GetChildObjectList()->Find( name, "a2dCameleon" ), a2dCameleon );
                if ( cam )
                {
                    if ( cam->GetBuildIn() )
                    {
                        a2dCanvasObject* inst = cam->GetBuildIn()->GetBuildIn();
                        PushToolDragNew( inst, CmdMenu_PushTool_CameleonInst() );
                    }
                    else if ( cam->GetSymbol() )
                    {
                        a2dCameleonInst* caminst = new a2dCameleonInst( 0,0, cam->GetSymbol() );
                        PushToolDragNew( caminst, CmdMenu_PushTool_CameleonInst() );
                    }
                    else if ( cam->GetDiagram() )
                    {
                        a2dCameleonInst* caminst = new a2dCameleonInst( 0,0, cam->GetDiagram() );
                        PushToolDragNew( caminst, CmdMenu_PushTool_CameleonInst() );
                    }
                }
                else
                    wxLogWarning( _T( "A a2dcameleon with this name does not exist" ) );
            }
        }
        else
            wxLogWarning( _T( "a2dCameleon::GetCameleonRoot() not available" ) );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CameleonInstDlg().GetId() )
    {
        found = true;
        if ( a2dCameleon::GetCameleonRoot() )
        {
            a2dCanvasObjectsChooseDialog structDlg( NULL, a2dCameleon::GetCameleonRoot()->GetChildObjectList(), "", SYMBOL_A2DCHOOSE_STYLE, SYMBOL_A2DCHOOSE_IDNAME, SYMBOL_A2DCHOOSE_TITLE, SYMBOL_A2DCHOOSE_POSITION, wxSize( 200, 600) );
            if ( structDlg.ShowModal() == wxID_OK )
            {       
                a2dCameleon* cam = wxDynamicCast( structDlg.GetCanvasObject(), a2dCameleon );
                if ( cam )
                {
                    if ( cam->GetBuildIn() )
                    {
                        a2dCanvasObject* inst = cam->GetBuildIn()->GetBuildIn();
                        PushToolDragNew( inst, CmdMenu_PushTool_CameleonInstDlg() );
                    }
                    else if ( cam->GetSymbol() )
                    {
                        a2dCameleonInst* caminst = new a2dCameleonInst( 0,0, cam->GetSymbol() );
                        PushToolDragNew( caminst, CmdMenu_PushTool_CameleonInstDlg() );
                    }
                    else if ( cam->GetDiagram() )
                    {
                        a2dCameleonInst* caminst = new a2dCameleonInst( 0,0, cam->GetDiagram() );
                        PushToolDragNew( caminst, CmdMenu_PushTool_CameleonInstDlg() );
                    }
                }
            }
        }
        else
            wxLogWarning( _T( "a2dCameleon::GetCameleonRoot() not available" ) );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CameleonRefDlg_Diagram().GetId() )
    {
        found = true;
        if ( a2dCameleon::GetCameleonRoot() )
        {
            a2dCanvasObjectsChooseDialog structDlg( NULL, a2dCameleon::GetCameleonRoot()->GetChildObjectList(), "a2dDiagram" );
            if ( structDlg.ShowModal() == wxID_OK )
            {       
                a2dDiagram* diacam = wxDynamicCast( structDlg.GetCanvasObject(), a2dDiagram );
                if ( diacam )
                {
                    a2dCameleonSymbolicRef* caminst = new a2dCameleonSymbolicRef( 0,0, diacam );
                    PushToolDragNew( caminst, CmdMenu_PushTool_CameleonRefDlg_Diagram() );
                }
                else
                    wxLogWarning( _T( "Diagram not available choosen" ) );
            }
        }
        else
            wxLogWarning( _T( "a2dCameleon::GetCameleonRoot() not available" ) );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CameleonRefDlg_Symbol().GetId() )
    {
        found = true;
        if ( a2dCameleon::GetCameleonRoot() )
        {
            a2dCanvasObjectsChooseDialog structDlg( NULL, a2dCameleon::GetCameleonRoot()->GetChildObjectList(), "a2dSymbol" );
            if ( structDlg.ShowModal() == wxID_OK )
            {       
                a2dSymbol* symcam = wxDynamicCast( structDlg.GetCanvasObject(), a2dSymbol );
                if ( symcam )
                {
                    a2dCameleonSymbolicRef* caminst = new a2dCameleonSymbolicRef( 0,0, symcam );
                    PushToolDragNew( caminst, CmdMenu_PushTool_CameleonRefDlg_Symbol() );
                }
                else
                    wxLogWarning( _T( "Symbol not available choosen" ) );
            }
        }
        else
            wxLogWarning( _T( "a2dCameleon::GetCameleonRoot() not available" ) );
    }
    else if ( event.GetId() == CmdMenu_PushTool_CameleonRefDlg().GetId() )
    {
        found = true;
        if ( a2dCameleon::GetCameleonRoot() )
        {
            a2dCanvasObjectsChooseDialog structDlg( NULL, a2dCameleon::GetCameleonRoot()->GetChildObjectList(), "" );
            if ( structDlg.ShowModal() == wxID_OK )
            {       
                a2dCameleon* cam = wxDynamicCast( structDlg.GetCanvasObject(), a2dCameleon );
                if ( cam )
                {
                    a2dCameleonSymbolicRef* caminst = new a2dCameleonSymbolicRef( 0,0, cam );
                    PushToolDragNew( caminst, CmdMenu_PushTool_CameleonRefDlg() );
                }
            }
        }
        else
            wxLogWarning( _T( "a2dCameleon::GetCameleonRoot() not available" ) );
    }
    else if ( event.GetId() == CmdMenu_PushTool_TagGroups().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;

        a2dStToolContr* contr = wxDynamicCast( GetActiveDrawingPart()->GetCanvasToolContr(), a2dStToolContr );
        if ( contr )
        {
			a2dMasterTagGroups3* tool = new a2dMasterTagGroups3( contr );
            tool->SetInitiatingMenuId( CmdMenu_PushTool_TagGroups() );
			contr->PushTool( tool );
		}
    }
    else if ( event.GetId() == CmdMenu_PushTool_TagGroups_WireMode().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;

        a2dStToolContr* contr = wxDynamicCast( GetActiveDrawingPart()->GetCanvasToolContr(), a2dStToolContr );
        if ( contr )
        {
			a2dMasterTagGroups3* tool = new a2dMasterTagGroups3( contr );
            tool->SetInitiatingMenuId( CmdMenu_PushTool_TagGroups_WireMode() );
			tool->SetWireMode( true );
			contr->PushTool( tool );
		}
    }    
	else if ( event.GetId() == CmdMenu_PushTool_TagGroups_DlgMode().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;

        a2dStToolContr* contr = wxDynamicCast( GetActiveDrawingPart()->GetCanvasToolContr(), a2dStToolContr );
        if ( contr )
        {
			a2dMasterTagGroups3* tool = new a2dMasterTagGroups3( contr );
            tool->SetInitiatingMenuId( CmdMenu_PushTool_TagGroups_DlgMode() );
			tool->SetDlgOrEdit( true );
			tool->SetDlgOrEditModal( true );
			contr->PushTool( tool );
		}
    }
    else if ( event.GetId() == CmdMenu_PushTool_TagGroups_WireMode_DlgMode().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;

        a2dStToolContr* contr = wxDynamicCast( GetActiveDrawingPart()->GetCanvasToolContr(), a2dStToolContr );
        if ( contr )
        {
			a2dMasterTagGroups3* tool = new a2dMasterTagGroups3( contr );
            tool->SetInitiatingMenuId( CmdMenu_PushTool_TagGroups_WireMode_DlgMode() );
			tool->SetWireMode( true );
			tool->SetDlgOrEdit( true );
			tool->SetDlgOrEditModal( true );
			contr->PushTool( tool );
		}
    }    
    else if ( event.GetId() == CmdMenu_PushTool_SingleClick().GetId() )
    {
        found = true;
        if ( !GetActiveDrawingPart() )
            return;

        a2dStToolContr* contr = wxDynamicCast( GetActiveDrawingPart()->GetCanvasToolContr(), a2dStToolContr );
        if ( contr )
        {
            a2dMasterSingleClick* tool = new a2dMasterSingleClick( contr );
            tool->SetInitiatingMenuId( CmdMenu_PushTool_SingleClick() );
			contr->PushTool( tool );
		}
    }    

    if ( !found )
        event.Skip();
}

void a2dCanvasCommandProcessor::SetPageSetupData( wxPageSetupDialogData* pageSetupData )
{ 
#if wxUSE_PRINTING_ARCHITECTURE
    if( m_pageSetupData )
        delete m_pageSetupData;
    m_pageSetupData = pageSetupData; 
#endif
}

bool a2dCanvasCommandProcessor::Print( wxUint16 printWhat )
{
#if wxUSE_PRINTING_ARCHITECTURE
    a2dDrawingPart* drawingPart = GetActiveDrawingPart();
    if ( !drawingPart )
        return false;

    wxPageSetupDialogData* aPageSetupData = GetPageSetupData();

    wxPrintDialogData printDialogData( aPageSetupData->GetPrintData() );

    wxPrintout* printout = drawingPart->OnCreatePrintout( printWhat, *aPageSetupData );
    if ( printout )
    {
        wxPrinter printer( &printDialogData );
        if( printer.Print( drawingPart->GetDisplayWindow(), printout, true ) )
            aPageSetupData->SetPrintData( printer.GetPrintDialogData().GetPrintData() );

        delete printout;
    }
#endif // wxUSE_PRINTING_ARCHITECTURE
    return true;
}

bool a2dCanvasCommandProcessor::Preview( wxUint16 printWhat )
{
#if wxUSE_PRINTING_ARCHITECTURE
    a2dDrawingPart* drawingPart = GetActiveDrawingPart();
    if ( !drawingPart )
        return false;

    wxPageSetupDialogData* aPageSetupData = GetPageSetupData();

    wxPrintDialogData printDialogData( aPageSetupData->GetPrintData() );

    wxPrintout* printout = drawingPart->OnCreatePrintout( printWhat, *aPageSetupData );
    if ( printout )
    {
        // Pass two printout objects: for preview, and possible printing.
        wxPrintPreviewBase* preview = ( wxPrintPreviewBase* ) NULL;
        preview = new wxPrintPreview( printout, drawingPart->OnCreatePrintout( printWhat, *aPageSetupData ), &printDialogData );
        if ( !preview->Ok() )
        {
            delete preview;
            wxMessageBox( _( "Sorry, print preview needs a printer to be installed." ) );
            return false;
        }

        wxPreviewFrame* frame = new wxPreviewFrame( preview, ( wxFrame* )wxTheApp->GetTopWindow(), _( "Print Preview" ),
                wxPoint( 100, 100 ), wxSize( 600, 650 ) );
        frame->Centre( wxBOTH );
        frame->Initialize();
        frame->Show( true );
    }
#endif // wxUSE_PRINTING_ARCHITECTURE
    return true;
}

void a2dCanvasCommandProcessor::OnBusyBegin( a2dCommandProcessorEvent& event )
{
    if ( !event.IsCursorSet() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        drawer->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_WAIT ) );
    }
    event.Skip();
}

void a2dCanvasCommandProcessor::OnBusyEnd( a2dCommandProcessorEvent& event )
{
    if ( !event.IsCursorSet() )
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return;
        drawer->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  ) );
    }
    event.Skip();
}

bool a2dCanvasCommandProcessor::Submit( a2dCommand* command, bool storeIt )
{
    wxCHECK_MSG( command, false, _T( "no command in a2dCanvasCommandProcessor::Submit" ) );

    SendBusyEvent( true, command );

    // to properly delete it if needed
    a2dSmrtPtr<a2dCommand> lcommand = command;

    ( ( a2dCommand* )command )->SetCommandProcessor( this );
    if ( command->IsCommandGroup() )
    {
        a2dCommandGroup* groupcmd = wxStaticCast( command, a2dCommandGroup );
        groupcmd->SetParentGroup( m_currentGroup );
    }

    bool oke = false;
    try
    {
        //commands in here will be stored before the actual one. 
        oke = DoPreCommand( *command );

        //first store, and if not right command, remove it again.
        if ( storeIt )
            Store( command );
		else //assume that if not stored, it does not modify documents.
			command->SetModifies( false );

        oke |= DoCommand( *command );

        //commands in here will be stored after the actual one. 
        oke |= DoPostCommand( *command );
    }
    catch ( const a2dCommandException& e )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_CommandError, _T( "%s" ),  e.getMessage().c_str() );
        oke = false;
    }

    if ( !oke )
    {
        if ( storeIt )
        {
            m_currentGroup->Remove( command );
            SetCurrentToLastActive();
        }

        SendBusyEvent( false, command );
        // the user code expects the command to be released/deleted
        return false;
    }

    //UPDATES in idle time, of via forced commando in scripts.
    //GetCanvasDocument()->UpdateAllViews( NULL, a2dCANVIEW_UPDATE_OLDNEW );

    SendBusyEvent( false, command );
    return true;
}

bool a2dCanvasCommandProcessor::Undo()
{
    a2dCommandPtr cur = m_currentCommand;
	SendBusyEvent( true, m_currentCommand );
	DeselectAll();
	bool res = a2dCommandProcessor::Undo();

    // m_currentCommand has changed in baseclass Undo 
    SendBusyEvent( false, cur );
    return res;
}

bool a2dCanvasCommandProcessor::Redo()
{
    a2dCommandPtr cur = m_currentCommand;
	SendBusyEvent( true, m_currentCommand );
	DeselectAll();
	bool res = a2dCommandProcessor::Redo();

    // m_currentCommand has changed in baseclass Undo 
    SendBusyEvent( false, cur );
    return res;
}

bool a2dCanvasCommandProcessor::SubmitMultiple( a2dCommand* command, a2dCanvasObjectList* objects, bool storeIt )
{
    bool result = true;

    if( objects->size() )
    {
        a2dCommandGroup* group = 0;

        if( storeIt )
            group = CommandGroupBegin( _( "Multiple " ) + command->GetName() );

        for( a2dCanvasObjectList::iterator iter = objects->begin(); iter != objects->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            a2dCommand* clone = command->CloneAndBind( obj );

            if( !Submit( clone, storeIt ) )
                result = false;
        }

        if( storeIt )
            CommandGroupEnd( group );
    }

    delete command;

    return result;
}

void a2dCanvasCommandProcessor::Refresh()
{
    if ( !m_drawing || !GetActiveDrawingPart() )
        return;
    m_drawing->GetRootObject()->Update( a2dCanvasObject::updatemask_force );
    GetActiveDrawingPart()->Update( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_ALL );
}

void a2dCanvasCommandProcessor::DeselectAll()
{
    if ( 0 )
    {
        a2dCanvasObjectList objects;
        GetActiveDrawingPart()->GetShowObject()->CollectObjects( &objects, wxT( "" ), a2dCanvasOFlags::SELECTED );
        SubmitMultiple(
            new a2dCommand_SetFlag( 0, a2dCanvasOFlags::SELECTED, false  ),
            &objects
        );
        //GetCanvasCommandProcessor()->Submit( new a2dCommand_Select( m_parentobject, a2dCommand_Select::Args().what( a2dCommand_Select::DeSelectAll ) ) );
    }
    else
    {
        a2dCanvasObjectList* objects = GetActiveDrawingPart()->GetShowObject()->GetChildObjectList();
        objects->SetSpecificFlags( true, a2dCanvasOFlags::PENDING, "", a2dCanvasOFlags::SELECTED );
        objects->SetSpecificFlags( true, a2dCanvasOFlags::PENDING, "", a2dCanvasOFlags::SELECTED2 );
        //BR2019
        //objects->SetSpecificFlags( true, a2dCanvasOFlags::SELECTPENDING, "", a2dCanvasOFlags::SELECTED );
        //objects->SetSpecificFlags( true, a2dCanvasOFlags::SELECTPENDING, "", a2dCanvasOFlags::SELECTED2 );

        objects->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED );
		objects->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED2 );
        GetActiveDrawingPart()->GetShowObject()->GetRoot()->SetUpdatesPending( true );
    }
}

bool a2dCanvasCommandProcessor::ShowDlgStyle( bool onOff )
{
    bool changed = false;
    if ( !m_styledlg )
        m_styledlg  = new a2dStyleDialog( m_drawing->GetHabitat(), NULL, wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxRESIZE_BORDER  );

    m_styledlg->SetFill( m_drawing->GetHabitat()->GetFill() );
    m_styledlg->SetStroke( m_drawing->GetHabitat()->GetStroke() );

    if ( ! m_styledlg->IsShown() )
    {
        if ( onOff )
        {
            changed = true;
            m_styledlg->Show( true );
        }
    }
    else
    {
        if ( !onOff )
        {
            changed = true;
            m_styledlg->Show( false );
        }
    }
    return changed;
}

bool a2dCanvasCommandProcessor::Zoom( double x1, double y1, double x2, double y2, bool upp )
{
    if ( GetActiveDrawingPart() )
    {
        if ( !upp )
            GetActiveDrawingPart()->SetMappingWidthHeight( x1, y1, x2, y2 );
        else
            GetActiveDrawingPart()->SetMappingUpp( x1, y1, x2, y2 );
        return true;
    }

    a2dGeneralGlobals->ReportError( a2dError_NoView );
    return false;
}


bool a2dCanvasCommandProcessor::ZoomRealSize( double scaleFromRealSize, bool selected )
{
    a2dDrawingPart* part = GetActiveDrawingPart();
	if ( !part )
		return false;

	a2dDrawer2D* drawer = part->GetDrawer2D();

	wxScreenDC 	screendc; 	
	wxSize size = screendc.GetPPI();
    wxSize sizemm = screendc.GetSizeMM();
	double MmInPixels = size.x / 25.4;
	double dataUnitsPerMm = 1 / part->GetDrawing()->GetUnitsScale();
	double scale = dataUnitsPerMm / MmInPixels; //this many database units per pixel

	scale /= scaleFromRealSize;

	// size of window
    int dx, dy;
    part->GetDisplayWindow()->GetClientSize( &dx, &dy );
    if ( dy > dx ) dx = dy;

	a2dBoundingBox bbox;
	if ( selected )
	{
		//find bbox selected objects
		a2dCanvasObjectList* objects = part->GetShowObject()->GetChildObjectList();
		forEachIn( a2dCanvasObjectList, objects )
		{
			a2dCanvasObject* obj = *iter;
			if ( obj->GetRelease() || !obj->IsVisible() || !obj->GetSelected() )
				continue;
			bbox.Expand( obj->GetBbox() );
		}
	}

	//fixed point
	double x, y;
	if ( bbox.GetValid() )
	{
		// find centre bbox in world coordinates
		x = bbox.GetMinX() + bbox.GetWidth()/2.0;
		y = bbox.GetMinY() + bbox.GetHeight()/2.0;
		double xf = (x - drawer->GetVisibleMinX())/drawer->GetVisibleWidth(); // distance to origin as factor of fullsize
		double yf = (y - drawer->GetVisibleMinY())/drawer->GetVisibleHeight(); // distance to origin as factor of fullsize
		// origin for new mapping
		x -= xf * dx * scale;
		y -= yf * dy * scale;
	}
	else
	{
		// find centre in world coordinates
		x = drawer->GetVisibleMinX() + drawer->GetVisibleWidth()/2.0;
		y = drawer->GetVisibleMinY() + drawer->GetVisibleHeight()/2.0;
		// origin for new mapping
		x -= dx / 2.0 * scale;
		y -= dy / 2.0 * scale;
	}

    part->SetMappingUpp( x, y, scale, scale );
	return true;
}

bool a2dCanvasCommandProcessor::Select( double x1, double y1, double x2, double y2 )
{
    if ( GetActiveDrawingPart() )
    {
		Submit( new a2dCommand_SetFlags( m_parent, a2dCanvasOFlags::SELECTED, true, false, a2dCanvasOFlags::ALL, a2dBoundingBox( x1, y1, x2, y2 ) ), m_withUndo );
        return true;
    }

    a2dGeneralGlobals->ReportError( a2dError_NoView );
    return false;
}

bool a2dCanvasCommandProcessor::DeSelect( double x1, double y1, double x2, double y2 )
{
    if ( GetActiveDrawingPart() )
    {
        Submit( new a2dCommand_SetFlags( m_parent, a2dCanvasOFlags::SELECTED, false, false, a2dCanvasOFlags::ALL, a2dBoundingBox( x1, y1, x2, y2 ) ), m_withUndo );
        return true;
    }

    a2dGeneralGlobals->ReportError( a2dError_NoView );
    return false;
}

a2dCanvasObjectList* a2dCanvasCommandProcessor::ConvertToPolygons( a2dCanvasObject* canvasobject, bool transform )
{
   a2dCanvasObjectList* ret = canvasobject->GetAsPolygons( true );
   if ( ret != wxNullCanvasObjectList )
        return  ret;
   return NULL;
/*

    if ( wxDynamicCast( canvasobject, a2dSurface ) ||
         wxDynamicCast( canvasobject, a2dPolygonL ) ||
         wxDynamicCast( canvasobject, a2dPolylineL )
       )
    {
        return  canvasobject->GetAsPolygons( true );
    }
    else
    {
       a2dCanvasObjectList* polylist = NULL;
        a2dCanvasObjectList* vpath = ConvertToVpath( canvasobject, true );

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
        return polylist;
    }
*/
}

a2dCanvasObjectList* a2dCanvasCommandProcessor::ConvertToVpath( a2dCanvasObject* canvasobject, bool transform )
{
    a2dCanvasObjectList* vpath = wxNullCanvasObjectList;

    if ( wxDynamicCast( canvasobject, a2dRect ) ||
            wxDynamicCast( canvasobject, a2dRectC ) ||
            wxDynamicCast( canvasobject, a2dArrow ) ||
            wxDynamicCast( canvasobject, a2dCircle ) ||
            wxDynamicCast( canvasobject, a2dEllipse ) ||
            wxDynamicCast( canvasobject, a2dEllipticArc ) ||
            wxDynamicCast( canvasobject, a2dArc ) ||
            wxDynamicCast( canvasobject, a2dPolygonL ) ||
            wxDynamicCast( canvasobject, a2dVectorPath ) ||
            wxDynamicCast( canvasobject, a2dText ) ||
            wxDynamicCast( canvasobject, a2dEndsLine ) ||
            wxDynamicCast( canvasobject, a2dSLine ) ||
            wxDynamicCast( canvasobject, a2dPolylineL )
       )
    {
        vpath = canvasobject->GetAsCanvasVpaths( transform );
    }
    return vpath;
}

a2dCanvasObject* a2dCanvasCommandProcessor::AddCurrent( a2dCanvasObject* objectToAdd, bool withUndo, a2dPropertyIdList* setStyles, a2dCanvasObject* parentObject )
{
    if ( parentObject )
        m_parent = parentObject;

    m_currentobject = objectToAdd;

    if ( m_currentobject )
        m_currentobject->TakeOverProperties( this, setStyles );

    // submit to the current document.
    if ( withUndo )
    {
        a2dCommand_AddObject* command = new a2dCommand_AddObject( m_parent, m_currentobject );
        command->SetSendBusyEvent( false );
        Submit( command );

		a2dCameleonInst* camInst;
		if ( camInst = wxDynamicCast( m_currentobject, a2dCameleonInst ) )
		{
			a2dCameleonEvent eventNewCameleon( m_drawing, camInst->GetCameleon() );
			m_drawing->ProcessEvent( eventNewCameleon );
		}
		a2dCameleon* cam;
		if ( cam = wxDynamicCast( m_currentobject, a2dCameleon ) )
		{
			a2dCameleonEvent eventNewCameleon( m_drawing, cam );
			m_drawing->ProcessEvent( eventNewCameleon );
		}

    }
    else
    {
        if ( m_AppendOrPrepend )
            m_drawing->Append( m_currentobject );
        else
            m_drawing->Prepend( m_currentobject );
    }

    return m_currentobject;
}


void a2dCanvasCommandProcessor::SetCurrentCanvasObject( a2dCanvasObject* currentcanvasobject, bool takeOverProperties )
{
    if ( !currentcanvasobject )
        currentcanvasobject = 0;

    m_currentobject = currentcanvasobject;
    if ( !m_currentobject )
        return;

    if ( !takeOverProperties )
        return;

    // the current object its style and other central stored properties,
    // are set as the current context.
    a2dNamedPropertyList props;
    currentcanvasobject->CollectProperties2( &props, NULL, a2dPropertyId::flag_transfer );
    a2dNamedPropertyList::iterator iter;
    for( iter = props.begin(); iter != props.end(); ++iter )
    {
        a2dNamedProperty* prop = *iter;
        prop->SetToObjectClone( this );
    }
}

wxString a2dCanvasCommandProcessor::AskFile( const wxString& message, const wxString& default_path,
                  const wxString& default_filename, const wxString& default_extension,
                  const wxString& wildcard, int flags, int x, int y )
{
    wxWindow* parent = GetActiveDrawingPart()->GetDisplayWindow();

    wxString expandedPath = default_path;
    a2dPathList path;
    if ( !default_path.IsEmpty() && !path.ExpandPath( expandedPath ) )
    {
        a2dGeneralGlobals->ReportWarningF( a2dError_CouldNotEvaluatePath, _( "Could not expand %s" ), default_path.c_str() );
        return wxT( "" );
    }
    return wxFileSelector( message, expandedPath, default_filename, default_extension, wildcard, flags, parent, x, y );
} 


bool a2dCanvasCommandProcessor::SaveLayers( const wxFileName& fileName )
{
    if ( !m_drawing )
        return false;

    a2dPathList path;
    path.Add( wxT( "." ) );
    wxString foundfile = fileName.GetFullPath();
    if ( !path.ExpandPath( foundfile ) )
    {
        a2dCanvasGlobals->ReportErrorF( a2dError_CouldNotEvaluatePath, _( "Could not expand %s resulted in %s" ), fileName.GetFullPath().c_str(), foundfile.c_str() );
        return false;
    }
    if ( foundfile.IsEmpty() )
    {
        a2dCanvasGlobals->ReportErrorF( a2dError_NotSpecified, _( "Filename %s not in %s" ), foundfile.c_str(), path.GetAsString().c_str() );
        return false;
    }

    if ( m_drawing->GetLayerSetup() )
        m_drawing->GetLayerSetup()->SaveLayers( foundfile );
    else
        a2dCanvasGlobals->ReportErrorF( a2dError_NotSpecified, _( "Document does not have layer setup" ) );

    return true;
}

bool a2dCanvasCommandProcessor::LoadLayers( const wxFileName& fileName )
{
    if ( !m_drawing )
        return false;

    a2dPathList path;
    path.Add( wxT( "." ) );
    wxString foundfile = fileName.GetFullPath();
    if ( !path.ExpandPath( foundfile ) )
    {
        a2dCanvasGlobals->ReportErrorF( a2dError_CouldNotEvaluatePath, _( "Could not expand %s resulted in %s" ), fileName.GetFullPath().c_str(), foundfile.c_str() );
        return false;
    }
    if ( foundfile.IsEmpty() )
    {
        a2dCanvasGlobals->ReportErrorF( a2dError_NotSpecified, _( "Filename %s not in %s" ), foundfile.c_str(), path.GetAsString().c_str() );
        return false;
    }

#if wxUSE_STD_IOSTREAM
    a2dDocumentFileInputStream store( foundfile.mb_str() );
    if ( store.fail() || store.bad() )
    {
        a2dCanvasGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for import" ), foundfile.c_str() );
        return false;
    }
#else
    wxFileInputStream storeUnbuf( foundfile );
    if ( !storeUnbuf.Ok() )
    {
        a2dCanvasGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for import" ), foundfile.c_str() );
        return false;
    }
    wxBufferedInputStream store( storeUnbuf );
#endif

#if wxART2D_USE_CVGIO
    //assume it is the default CVG format.
    a2dIOHandlerCVGIn* handler = new a2dIOHandlerCVGIn();
    a2dLayers* layersetup = new a2dLayers();
    handler->LoadLayers( store, layersetup );

    delete handler;
#else
    wxFAIL_MSG( wxT( "wxART2D_USE_CVGIO is needed to load object from a file CVG file" ) );
#endif //wxART2D_USE_CVGIO

    m_drawing->SetLayerSetup( layersetup );
    return true;
}

bool a2dCanvasCommandProcessor::SetParentObject( a2dCanvasObject* obj )
{
    if ( !obj )
        obj = m_drawing->GetRootObject();

    if ( m_parent != obj )
    {
        a2dCanvasObject* found = m_drawing->GetRootObject()->Find( obj );

        if ( !found )
            m_parent = m_drawing->GetRootObject();
        else
            m_parent = found;

        m_currentobject = 0;

        return found != 0;
    }
    return true;
}

a2dCanvasObject* a2dCanvasCommandProcessor::Add_a2dCanvasObject( double x, double y  )
{
    return AddCurrent( new a2dCanvasObject( x, y ), m_withUndo );
}

a2dCanvasObjectReference* a2dCanvasCommandProcessor::Add_a2dCanvasObjectReference( double x, double y, a2dCanvasObject* obj  )
{
    return ( a2dCanvasObjectReference* ) AddCurrent( new a2dCanvasObjectReference( x, y, obj ), m_withUndo );
}

/* TODO
a2dNameReference* a2dCanvasCommandProcessor::Add_a2dNameReference( double x, double y, a2dCanvasObject* obj, const wxString &text, double size, double angle  )
{
    return (a2dNameReference*) AddCurrent( new a2dNameReference( x, y, obj, text, size, angle ) );
}
*/

a2dOrigin* a2dCanvasCommandProcessor::Add_a2dOrigin( double w, double h  )
{
    return ( a2dOrigin* ) AddCurrent( new a2dOrigin( w, h ), m_withUndo );
}

a2dHandle* a2dCanvasCommandProcessor::Add_a2dHandle( double xc, double yc, int w, int h, double angle , int radius  )
{
    return ( a2dHandle* ) AddCurrent( new a2dHandle( NULL, xc, yc, _T( "" ), w, h, angle, radius ), m_withUndo );
}

a2dRectC* a2dCanvasCommandProcessor::Add_a2dRectC( double xc, double yc, double w, double h, double angle, double radius  )
{
    return ( a2dRectC* ) AddCurrent( new a2dRectC( xc, yc, w, h, angle, radius ), m_withUndo );
}

a2dArrow* a2dCanvasCommandProcessor::Add_a2dArrow( double xt, double yt, double l1, double l2, double b, bool spline  )
{
    return ( a2dArrow* ) AddCurrent( new a2dArrow( xt, yt, l1, l2, b, spline ), m_withUndo );
}

a2dRect* a2dCanvasCommandProcessor::Add_a2dRect( double x, double y, double w, double h , double radius  )
{
    return ( a2dRect* ) AddCurrent( new a2dRect( x, y, w, h, radius ), m_withUndo );
}

a2dCircle* a2dCanvasCommandProcessor::Add_a2dCircle( double x, double y, double radius  )
{
    return ( a2dCircle* ) AddCurrent( new a2dCircle( x, y, radius ), m_withUndo );
}

a2dEllipse* a2dCanvasCommandProcessor::Add_a2dEllipse( double xc, double yc, double width, double height  )
{
    return ( a2dEllipse* ) AddCurrent( new a2dEllipse( xc, yc, width, height ), m_withUndo );
}

a2dEllipticArc* a2dCanvasCommandProcessor::Add_a2dEllipticArc( double xc, double yc, double width, double height, double start, double end  )
{
    return ( a2dEllipticArc* ) AddCurrent( new a2dEllipticArc( xc, yc, width, height, start, end ), m_withUndo );
}

a2dArc* a2dCanvasCommandProcessor::Add_a2dArc( double xc, double yc, double radius, double start, double end  )
{
    return ( a2dArc* ) AddCurrent( new a2dArc( xc, yc, radius, start, end ), m_withUndo );
}

a2dSLine* a2dCanvasCommandProcessor::Add_a2dSLine( double x1, double y1, double x2, double y2  )
{
    return ( a2dSLine* ) AddCurrent( new a2dSLine( x1, y1, x2, y2 ), m_withUndo );
}

a2dEndsLine* a2dCanvasCommandProcessor::Add_a2dEndsLine( double x1, double y1, double x2, double y2  )
{
    return ( a2dEndsLine* ) AddCurrent( new a2dEndsLine( x1, y1, x2, y2 ), m_withUndo );
}

a2dImage* a2dCanvasCommandProcessor::Add_a2dImage( const wxImage& image, double xc, double yc, double w, double h  )
{
    return ( a2dImage* ) AddCurrent( new a2dImage( image, xc, yc, w, h ), m_withUndo );
}

a2dImage* a2dCanvasCommandProcessor::Add_a2dImage( const wxString& imagefile, wxBitmapType type, double xc, double yc, double w, double h  )
{
    return ( a2dImage* ) AddCurrent( new a2dImage( imagefile, type, xc, yc, w, h ), m_withUndo );
}

a2dText* a2dCanvasCommandProcessor::Add_a2dText( const wxString& text, double x, double y, double angle,  const a2dFont& font )
{
    return ( a2dText* ) AddCurrent( new a2dText( text, x, y, font, angle ), m_withUndo );
}

a2dPolygonL* a2dCanvasCommandProcessor::Add_a2dPolygonL( a2dVertexList* points, bool spline  )
{
    return ( a2dPolygonL* ) AddCurrent( new a2dPolygonL( points, spline ), m_withUndo );
}

a2dPolylineL* a2dCanvasCommandProcessor::Add_a2dPolylineL( a2dVertexList* points, bool spline  )
{
    return ( a2dPolylineL* ) AddCurrent( new a2dPolylineL( points, spline ), m_withUndo );
}

bool a2dCanvasCommandProcessor::Add_Point( double x, double y )
{
    Submit( new a2dCommand_AddPoint( GetCurrentCanvasObject(), x, y, -1 ), m_withUndo );

    //SetProperty( __M_ADDPOINT )??

    return true;
}

bool a2dCanvasCommandProcessor::Move_Point( int index , double x, double y )
{
    Submit( new a2dCommand_MoveSegment( GetCurrentCanvasObject(), x, y, index ), m_withUndo );

    //SetProperty( __M_MOVEPOINT )??

    return true;
}

bool a2dCanvasCommandProcessor::PushToolDragNew( a2dCanvasObject* newObject, const a2dMenuIdItem& initiatingMenuId, bool dragCenter, bool oneshot, bool lateconnect )
{
    if ( !m_drawing )
    {
        a2dGeneralGlobals->ReportError( a2dError_NoView );
        return false;
    }

    a2dStToolContr* contr = ( a2dStToolContr* ) GetActiveDrawingPart()->GetCanvasToolContr();

    if ( wxDynamicCast( contr->GetFirstTool(), a2dDragNewTool ) )
    {
        a2dSmrtPtr< a2dBaseTool > tool;
        contr->PopTool( tool );
    }

    if ( newObject )
    {
        newObject->SetParent(  contr->GetDrawingPart()->GetShowObject() );
        a2dDragNewTool* dragnew = new a2dDragNewTool( contr, newObject, initiatingMenuId );
        //optional
        //dragnew->SetOneShot();
        dragnew->SetStroke( a2dStroke( *wxRED, 1 ) );
        dragnew->SetFill( a2dFill( wxColour( 0xc0, 0xff, 0xff ) ) );
        dragnew->SetBoxCenterDrag( dragCenter );
        dragnew->SetLateConnect( lateconnect );

        if ( oneshot ) dragnew->SetOneShot();
        contr->PushTool( dragnew );
        return true;
    }
    return false;
}

bool a2dCanvasCommandProcessor::PushTool( const a2dCommandId& whichTool, bool shiftadd, bool oneshot )
{
    const a2dCommandId* which = &whichTool;
    if ( !m_drawing )
    {
        a2dGeneralGlobals->ReportError( a2dError_NoView );
        return false;
    }

    a2dStToolContr* contr = ( a2dStToolContr* ) GetActiveDrawingPart()->GetCanvasToolContr();

    if ( !contr )
    {
        a2dGeneralGlobals->ReportError( a2dError_NoController );
        return false;
    }

    if ( 0 ) //done with a2dBaseTool::sig_toolBeforePush
    {
        a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
        objects->SetSpecificFlags( true, a2dCanvasOFlags::PENDING, "", a2dCanvasOFlags::SELECTED );
        objects->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED );
        m_parent->GetRoot()->SetUpdatesPending( true );
    }

    //Submit( new a2dCommand_Select( m_parent, a2dCommand_Select::Args().what( a2dCommand_Select::DeSelectAll ) ) );

    if ( a2dCanvasGlobals->GetPopBeforePush() )
    {
        a2dSmrtPtr<a2dBaseTool> poped;
        contr->PopTool( poped, false ); //set  true if abort is required here.
    }

    if ( which == &a2dSelectTool::COMID_PushTool_Select )
    {
        a2dSelectTool* selt = new a2dSelectTool( contr );

        if ( !shiftadd )
            selt->SetShiftIsAdd();

        if ( oneshot ) selt->SetOneShot();
        contr->PushTool( selt );
        selt->SetStroke( a2dStroke( wxColour( 200, 0, 0 ), 0, a2dSTROKE_LONG_DASH ) );
    }
    else if ( which == &a2dRecursiveEditTool::COMID_PushTool_RecursiveEdit )
    {
        a2dRecursiveEditTool* edit = new a2dRecursiveEditTool( contr );
        if ( oneshot ) edit->SetOneShot();
        contr->PushTool( edit );
    }
    else if ( which == &a2dObjectEditTool::COMID_PushTool_ObjectEdit )
    {
        a2dObjectEditTool* edit = new a2dObjectEditTool( contr );
        if ( oneshot ) edit->SetOneShot();
        contr->PushTool( edit );
    }
    else if ( which == &a2dMultiEditTool::COMID_PushTool_MultiEdit )
    {
        a2dMultiEditTool* edit = new a2dMultiEditTool( contr );
        if ( oneshot ) edit->SetOneShot();
        contr->PushTool( edit );
    }
    else if ( which == &a2dDragTool::COMID_PushTool_Drag )
    {
        a2dDragTool* drag = new a2dDragTool( contr );
        if ( oneshot ) drag->SetOneShot();
        contr->PushTool( drag );
    }
    else if ( which == &a2dDragOrgTool::COMID_PushTool_DragOrg )
    {
        a2dDragOrgTool* drag = new a2dDragOrgTool( contr );
        if ( oneshot ) drag->SetOneShot();
        contr->PushTool( drag );
    }
    else if ( which == &a2dCopyTool::COMID_PushTool_Copy )
    {
        a2dCopyTool* copy = new a2dCopyTool( contr );
        if ( oneshot ) copy->SetOneShot();
        contr->PushTool( copy );
    }
    else if ( which == &a2dRotateTool::COMID_PushTool_Rotate )
    {
        a2dRotateTool* rot = new a2dRotateTool( contr );
        if ( oneshot ) rot->SetOneShot();
        contr->PushTool( rot );
    }
    else if ( which == &a2dZoomTool::COMID_PushTool_Zoom )
    {
        a2dZoomTool* zoomt = new a2dZoomTool( contr );
        if ( oneshot ) zoomt->SetOneShot();
        contr->PushTool( zoomt );
        zoomt->SetStroke( a2dStroke( wxColour( 0, 0, 0 ), 1, a2dSTROKE_LONG_DASH ) );
    }
/*todo
    else if ( which == &a2dRenderImageZoomTool::COMID_PushTool_RenderImageZoom )
    {
#if wxART2D_USE_CANEXTOBJ
        a2dRenderImageZoomTool* zoomt = new a2dRenderImageZoomTool( contr );
        if ( oneshot ) zoomt->SetOneShot();
        contr->PushTool( zoomt );
        zoomt->SetStroke( a2dStroke( wxColour( 200, 0, 0 ), 0, a2dSTROKE_LONG_DASH ) );
#endif
    }
*/
    else if ( which == &a2dDeleteTool::COMID_PushTool_Delete )
    {
        a2dDeleteTool* deletet = new a2dDeleteTool( contr );
        if ( oneshot ) deletet->SetOneShot();
        contr->PushTool( deletet );
    }
    else if ( which == &a2dDrawTextTool::COMID_PushTool_DrawText )
    {
        a2dDrawTextTool* draw = new a2dDrawTextTool( contr, m_drawing->GetHabitat()->GetTextTemplateObject() );
        if ( oneshot ) draw->SetOneShot();
//                draw->SetFont( m_currentFont );
//                draw->SetTextHeight( m_currentfontsize );
        contr->PushTool( draw );
    }
    else if ( which == &a2dImageTool::COMID_PushTool_Image )
    {
        a2dImageTool* draw = new a2dImageTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
    }
    else if ( which == &a2dImageTool::COMID_PushTool_Image_Embedded )
    {
        a2dImageTool* draw = new a2dImageTool( contr );
        draw->SetEmbed( true );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
    }
    else if ( which == &a2dDrawRectangleTool::COMID_PushTool_DrawRectangle )
    {
        a2dDrawRectangleTool* draw = new a2dDrawRectangleTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
    }
    else if ( which == &a2dDrawCircleTool::COMID_PushTool_DrawCircle )
    {
        a2dDrawCircleTool* draw = new a2dDrawCircleTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
    }
    else if ( which == &a2dDrawLineTool::COMID_PushTool_DrawLine )
    {
        a2dDrawLineTool* draw = new a2dDrawLineTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
    }
    else if ( which == &a2dDrawLineTool::COMID_PushTool_DrawLineScaledArrow )
    {
        a2dDrawLineTool* draw = new a2dDrawLineTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
        draw->SetTemplateObject( new a2dScaledEndLine() );
    }
    else if ( which == &a2dDrawEllipseTool::COMID_PushTool_DrawEllipse )
    {
        a2dDrawEllipseTool* draw = new a2dDrawEllipseTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
    }
    else if ( which == &a2dDrawEllipticArcTool::COMID_PushTool_DrawEllipticArc )
    {
        a2dDrawEllipticArcTool* draw = new a2dDrawEllipticArcTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
    }
    else if ( which == &a2dDrawEllipticArcTool::COMID_PushTool_DrawEllipticArc_Chord )
    {
        a2dDrawEllipticArcTool* draw = new a2dDrawEllipticArcTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
        draw->SetChord( true );
    }
    else if ( which == &a2dDrawArcTool::COMID_PushTool_DrawArc )
    {
        a2dDrawArcTool* draw = new a2dDrawArcTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
    }
    else if ( which == &a2dDrawArcTool::COMID_PushTool_DrawArc_Chord )
    {
        a2dDrawArcTool* draw = new a2dDrawArcTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
        draw->SetChord( true );
    }
    else if ( which == &a2dDrawPolylineLTool::COMID_PushTool_DrawPolylineL )
    {
        a2dDrawPolylineLTool* draw = new a2dDrawPolylineLTool( contr );
        if ( oneshot ) draw->SetOneShot();
        draw->SetSpline( false );
            draw->SetLineBegin( m_drawing->GetHabitat()->GetLineBegin() );
            draw->SetLineEnd( m_drawing->GetHabitat()->GetLineEnd() );
            m_drawing->GetHabitat()->SetSpline( false );
        contr->PushTool( draw );
    }
    else if ( which == &a2dDrawPolylineLTool::COMID_PushTool_DrawPolylineL_Splined )
    {
        a2dDrawPolylineLTool* draw = new a2dDrawPolylineLTool( contr );
        if ( oneshot ) draw->SetOneShot();
        draw->SetSpline( true );
            draw->SetLineBegin( m_drawing->GetHabitat()->GetLineBegin() );
            draw->SetLineEnd( m_drawing->GetHabitat()->GetLineEnd() );
            m_drawing->GetHabitat()->SetSpline( true );
        contr->PushTool( draw );
    }
    else if ( which == &a2dDrawPolygonLTool::COMID_PushTool_DrawPolygonL )
    {
        a2dDrawPolygonLTool* draw = new a2dDrawPolygonLTool( contr );
        if ( oneshot ) draw->SetOneShot();
        draw->SetSpline( false );
            m_drawing->GetHabitat()->SetSpline( false );
        contr->PushTool( draw );
    }
    else if ( which == &a2dDrawPolygonLTool::COMID_PushTool_DrawPolygonL_Splined )
    {
        a2dDrawPolygonLTool* draw = new a2dDrawPolygonLTool( contr );
        if ( oneshot ) draw->SetOneShot();
        draw->SetSpline( true );
            m_drawing->GetHabitat()->SetSpline( true );
        contr->PushTool( draw );
    }
    else if ( which == &a2dDragMultiTool::COMID_PushTool_DragMulti )
    {
        a2dDragMultiTool* drag = new a2dDragMultiTool( contr );
        if ( oneshot ) drag->SetOneShot();
        contr->PushTool( drag );
    }
    else if ( which == &a2dCopyMultiTool::COMID_PushTool_CopyMulti )
    {
        a2dCopyMultiTool* copy = new a2dCopyMultiTool( contr );
        if ( oneshot ) copy->SetOneShot();
        contr->PushTool( copy );
    }
    else if ( which == &a2dPropertyTool::COMID_PushTool_Property )
    {
        a2dPropertyTool* propertyedit = new a2dPropertyTool( contr );
        if ( oneshot ) propertyedit->SetOneShot();
        contr->PushTool( propertyedit );
    }
    else if ( which == &a2dMeasureTool::COMID_PushTool_Measure )
    {
        a2dMeasureTool* draw = new a2dMeasureTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
    }
    else if ( which == &a2dDrawWirePolylineLTool::COMID_PushTool_DrawWirePolylineL )
    {
        a2dDrawWirePolylineLTool* draw = new a2dDrawWirePolylineLTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
    }
    else if ( which == &a2dCanvasCommandProcessor::COMID_PopTool )
    {
        return PopTool() != NULL;
    }
    else if ( which == &a2dDrawVPathTool::COMID_PushTool_DrawVPath )
    {
        a2dDrawVPathTool* draw = new a2dDrawVPathTool( contr );
        if ( oneshot ) draw->SetOneShot();
        contr->PushTool( draw );
    }
    else
        a2dGeneralGlobals->ReportError( a2dError_NoTool );

    if ( contr->GetFirstTool() )
    {
        a2dStTool* draw = ( a2dStTool* ) contr->GetFirstTool();
        if ( wxDynamicCast( draw, a2dStDrawTool ) )
            wxDynamicCast( draw, a2dStDrawTool )->SetEditAtEnd( a2dCanvasGlobals->GetEditAtEndTools()  );
        draw->SetActive();
    }
    return true;
}

a2dBaseTool* a2dCanvasCommandProcessor::PopTool()
{
    if ( GetActiveDrawingPart() )
    {
        a2dToolContr* contr = GetActiveDrawingPart()->GetCanvasToolContr();
        if ( contr )
        {
            a2dSmrtPtr< a2dBaseTool > tool;
            contr->PopTool( tool );
            if ( ! tool )
                a2dGeneralGlobals->ReportError( a2dError_NoTool );
            return ( a2dBaseTool* ) tool.Get();
        }
        a2dGeneralGlobals->ReportError( a2dError_NoController );
        return NULL;
    }
    a2dGeneralGlobals->ReportError( a2dError_NoView );
    return NULL;
} 

void a2dCanvasCommandProcessor::ClearMeasurements()
{
	a2dComEvent changed( this, sig_ClearMeasurements );
	ProcessEvent( changed );
}

void a2dCanvasCommandProcessor::AddMeasurement( double x, double y )
{
    m_meas_x = x;
    m_meas_y = y;

	a2dComEvent changed( this, sig_AddMeasurement );
	ProcessEvent( changed );
}

void a2dCanvasCommandProcessor::SetRelativeStart( double x, double y )
{
    m_drawing->GetHabitat()->SetRelativeStart( x, y );
	a2dComEvent changed( this, sig_SetRelativeStart );
	ProcessEvent( changed );
}

bool a2dCanvasCommandProcessor::ZoomOut()
{
    if ( GetActiveDrawingPart() )
        GetActiveDrawingPart()->SetMappingShowAllCanvas();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_AddObject
//----------------------------------------------------------------------------

a2dCommand_AddObject::a2dCommand_AddObject( a2dCanvasObject* parent, a2dCanvasObject* object ):
    a2dCommand( true, a2dCommand_AddObject::Id )
{
    wxASSERT( !m_canvasobject );
    wxASSERT( !m_parent );

    m_canvasobject = object;
    m_parent = parent;
}

a2dCommand_AddObject::~a2dCommand_AddObject( void )
{
}

bool a2dCommand_AddObject::Do( void )
{
    // When this is a redo, the release flag might still be set
    m_canvasobject->SetRelease( false );
    m_canvasobject->SetPending( true );

    m_canvasobject->SetParent( m_parent );
    m_parent->Append( m_canvasobject );
    // the next generates commands for pins that connect.
    //m_canvasobject->ReWireConnected( m_parent, true );

    return true;
}

bool a2dCommand_AddObject::Redo( void )
{
    // When this is a redo, the release flag might still be set
    m_canvasobject->SetRelease( false );
    m_canvasobject->SetPending( true );

    m_canvasobject->SetParent( m_parent );
    m_parent->Append( m_canvasobject );

    //connection to pins are/should be seperate commands in a a2dCommandGroup.

    return true;
}

bool a2dCommand_AddObject::Undo( void )
{
    // release the last added canvasobject
    m_canvasobject->SetParent( 0 );
    // if this object was connected to other objects, while being added to the drawing,
    // the tool has recorded those connection as a group, and they will be undone as a whole,
    // no need to combine it here.
    m_parent->ReleaseChild( m_canvasobject, false, false  );

    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_ReleaseObject
//----------------------------------------------------------------------------

a2dCommand_ReleaseObject::a2dCommand_ReleaseObject( a2dCanvasObject* parent, a2dCanvasObject* object, bool now ):
    a2dCommand( true, a2dCommand_ReleaseObject::Id )
{
    m_canvasobject = object;
    m_parent = parent;
    //for undo preserve location
    m_index = 0;
    m_now = now;
}

a2dCommand_ReleaseObject::~a2dCommand_ReleaseObject( void )
{
}

bool a2dCommand_ReleaseObject::Do( void )
{
    m_index = m_parent->IndexOf( m_canvasobject.Get() );

    if ( m_index == -1 )
        a2dGeneralGlobals->ReportError( a2dError_canvasObjectRelease );

    if ( m_now )
    {
        m_canvasobject->SetRelease( true );
        m_canvasobject->SetPending( true );
        //m_parent->ReleaseChild( m_canvasobject.Get(), false, false, false, true );
    }
    else
    {
        //! set property for proper release in idle time by a2dCanvasObject::Update()
        // see PROPID_Parent in bool a2dCanvasObject::Update( UpdateMode mode )
        a2dCanvasObject::PROPID_Parent->SetPropertyToObject( m_canvasobject, m_parent );
        m_canvasobject->SetRelease( true );
        m_canvasobject->SetPending( true );
    }

    return true;
}

bool a2dCommand_ReleaseObject::Undo( void )
{
    // The release flag was set in do, so we must reste it to false.
    // Otherwise the object would be immediately released again.
    m_canvasobject->SetRelease( false );
    m_canvasobject->SetPending( true );

    //insert the old object
    m_parent->Insert( m_index, m_canvasobject.Get(), false );

    return true;
}

bool a2dCommand_ReleaseObject::PreDo()
{
    m_canvasobject->DependentCommands( m_parent, this );

	//connected wires are stored until after disconnect is finished, next the will be optimized (e.g. merge lines)
	// If bin flags is set for a wire, it will not be optimized.
	a2dCanvasObjectList connectedwires;

    if ( m_canvasobject->HasPins() )
    {
		//find connected wires (not the ones with bin flag set, and not the object itself ).
        forEachIn( a2dCanvasObjectList, m_canvasobject->GetChildObjectList() )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pinc = wxDynamicCast( obj, a2dPin );
            if ( !pinc )
                continue;

			a2dPinList::const_iterator iterconp;
            a2dPinList savepinsother = pinc->GetConnectedPins();
            for ( iterconp = savepinsother.begin( ) ; iterconp != savepinsother.end( ) ; iterconp++ )
            {
                a2dPin* otherpin = *iterconp;

                if( !otherpin || otherpin->GetRelease() )
                    continue;
                if( !otherpin->IsConnectedTo()  )
                    continue;

                a2dCanvasObject* connectedobj = otherpin->GetParent();
                a2dWirePolylineL* wireConnected = wxDynamicCast( connectedobj, a2dWirePolylineL );

				if ( wireConnected && 
					 ( wireConnected != m_canvasobject ) && // needs to be a different wire, in order to join.
					 connectedobj->IsConnect() && 
					 !connectedobj->GetBin() && !connectedobj->GetRelease() ) //selected wires are dragged and have bin flags set.
                {
					connectedwires.push_back( wireConnected );
				}
			}
		}

        forEachIn( a2dCanvasObjectList, m_canvasobject->GetChildObjectList() )
        {
            a2dCanvasObject* obj = *iter;
            a2dPin* pinc = wxDynamicCast( obj, a2dPin );
            if ( !pinc )
                continue;
            pinc->DuplicateConnectedToOtherPins( true );
        }
    }

	// the objects will now be disconnected, so not optimized via this connected wires down here.
    m_canvasobject->DisConnectWith( NULL, wxT( "" ), true );

	// connected wires can now be optimized.	
    a2dCanvasObjectList::iterator iter;
    for( iter = connectedwires.begin(); iter != connectedwires.end(); ++iter )
    {
        a2dCanvasObject* connect = *iter;
        a2dWirePolylineL* wire = wxStaticCast( connect, a2dWirePolylineL );
		wire->OptimizeRerouteWires( m_parent, true, true );
	}
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_ReplaceObject
//----------------------------------------------------------------------------

a2dCommand_ReplaceObject::a2dCommand_ReplaceObject( a2dCanvasObject* parent, a2dCanvasObject* oldobject, a2dCanvasObject* newobject ):
    a2dCommand( true, a2dCommand_ReplaceObject::Id )
{
    m_parent = parent;
    m_canvasobject = oldobject;
    m_canvasobject2 = newobject;
    m_parent = parent;
}

a2dCommand_ReplaceObject::~a2dCommand_ReplaceObject( void )
{
}

bool a2dCommand_ReplaceObject::Do( void )
{
    int index = m_parent->IndexOf( m_canvasobject );

    //remove the old object
    m_parent->ReleaseChild( m_canvasobject, false, false  );
    //insert the modified copy
    m_parent->Insert( index, m_canvasobject2 );

    a2dCanvasObject* swap = m_canvasobject;
    m_canvasobject = m_canvasobject2;
    m_canvasobject2 = swap;

    return true;
}

bool a2dCommand_ReplaceObject::Undo( void )
{
    return Do();
}

//----------------------------------------------------------------------------
// a2dCommand_MoveObject
//----------------------------------------------------------------------------

a2dCommand_MoveObject::a2dCommand_MoveObject( a2dCanvasObject* parentFrom, a2dCanvasObject* parentTo, a2dCanvasObject* object ):
    a2dCommand( true, a2dCommand_MoveObject::Id )
{
    m_parentTo = parentTo;
    m_parentFrom = parentFrom;
    m_canvasobject = object;
    m_index = 0;
}

a2dCommand_MoveObject::~a2dCommand_MoveObject( void )
{
}

bool a2dCommand_MoveObject::Do( void )
{
    int index = 0;
    a2dCanvasObjectList* parentList = m_parentFrom->GetChildObjectList();
    a2dCanvasObjectList::iterator iterpins = parentList->begin();
    while( iterpins != parentList->end() )
    {
        a2dCanvasObject* obj = *iterpins;
        if ( obj == m_canvasobject )
        {
            (*iterpins)->SetRelease( true );
            break;
        }
        index++;
        iterpins++;
    }
    //m_parentTo->Insert( m_index, m_canvasobject );
    m_parentTo->Append( m_canvasobject );
    m_index = index;
    return true;
}

bool a2dCommand_MoveObject::Undo( void )
{
    int index = 0;
    a2dCanvasObjectList* parentList = m_parentTo->GetChildObjectList();
    a2dCanvasObjectList::iterator iterpins = parentList->begin();
    while( iterpins != parentList->end() )
    {
        a2dCanvasObject* obj = *iterpins;
        if ( obj == m_canvasobject )
        {
            (*iterpins)->SetRelease( true );
            break;
        }
        index++;
        iterpins++;
    }
    //m_parentFrom->Insert( m_index, m_canvasobject );
    m_parentFrom->Append( m_canvasobject );
    m_index = index;
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_Flags
//----------------------------------------------------------------------------

a2dCommand_SetFlags::a2dCommand_SetFlags( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask, bool value, bool preserve, a2dCanvasObjectFlagsMask whichobjects, const a2dBoundingBox& bbox ):
    a2dCommand( true, a2dCommand_SetFlags::Id )
{
    m_parentObject = parent;
    m_bbox = bbox;

    m_which = mask;

    m_maskedObjects = wxNullCanvasObjectList;

    m_bool = value;
	m_preserve = preserve;
	m_whichObjects = whichobjects;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in new ( )
    CurrentSmartPointerOwner = this;
#endif
}

a2dCommand_SetFlags::~a2dCommand_SetFlags( void )
{
    if ( m_maskedObjects != wxNullCanvasObjectList )
        delete m_maskedObjects;
}

bool a2dCommand_SetFlags::Do( void )
{
    a2dCanvasObjectList* objects = m_parentObject->GetChildObjectList();
    m_maskedObjects = objects->Clone( m_whichObjects, a2dObject::clone_flat, NULL, m_bbox );

    if ( m_maskedObjects != wxNullCanvasObjectList )
    {
		m_objectOldMask.clear();
        forEachIn( a2dCanvasObjectList,  m_maskedObjects )
        {
            a2dCanvasObject* obj = *iter;
            m_objectOldMask.push_back( obj->GetFlags() );
			obj->SetSpecificFlags( m_bool, obj->GetFlags() | m_which );
			obj->SetPending( true );
        }
    }

    return true;
}

bool a2dCommand_SetFlags::Undo( void )
{
    a2dCanvasObjectList* objects = m_parentObject->GetChildObjectList();

	//first reset all bits in mask for all objects
	if ( m_preserve )
	{
		forEachIn( a2dCanvasObjectList, objects )
		{
			a2dCanvasObject* obj = *iter;
			a2dCanvasObjectFlagsMask masknow = obj->GetFlags(); 
			if ( masknow & m_which )
			{
				obj->SetFlags( masknow & ~m_which );
				obj->SetPending( true );
			}
		}
	}

    if ( m_maskedObjects != wxNullCanvasObjectList )
    {
        a2dlist< a2dCanvasObjectFlagsMask >::iterator itermask = m_objectOldMask.begin();
        //while ( itermask != m_objectOldMask.end() )
        {
            forEachIn( a2dCanvasObjectList,  m_maskedObjects )
            {
                a2dCanvasObject* obj = *iter;
				if ( m_preserve )
					//set bit only for the object in the command
					obj->SetSpecificFlags( m_bool, obj->GetFlags() | m_which );
				else
					obj->SetFlags( *itermask );
                obj->SetPending( true );
                itermask++;
            }
        }
    }
    //m_maskedObjects->clear();
    return true;
}

bool a2dCommand_SetFlags::Redo( void )
{
	return Undo();
}

//----------------------------------------------------------------------------
// a2dCommand_SetFlag
//----------------------------------------------------------------------------

a2dCommand_SetFlag::a2dCommand_SetFlag( a2dCanvasObject* object, a2dCanvasObjectFlagsMask which, bool value ):
    a2dCommand( true, a2dCommand_SetFlag::Id )
{
    m_which = which;
    m_bool = value;
    m_canvasobject = object;
}

a2dCommand_SetFlag::~a2dCommand_SetFlag( void )
{
}

a2dCommand* a2dCommand_SetFlag::CloneAndBind( a2dObject* object )
{
    wxASSERT( !m_canvasobject );
    return new a2dCommand_SetFlag( wxStaticCast( object, a2dCanvasObject ), m_which, m_bool );
}

bool a2dCommand_SetFlag::Do( void )
{
    // !!!! ToDo: This must be the flag, that is changed, not the selected flag
    // This is saved here, so that it can be undone.
    bool oldvalue = m_canvasobject->GetSelected();

    m_canvasobject->SetSpecificFlags( m_bool, m_which );
    //the last call never sets objects pending, still we want it here to be able to see
    //the possible effects.
    m_canvasobject->SetPending( true );

    m_bool = oldvalue;
    return true;
}

bool a2dCommand_SetFlag::Undo( void )
{
    bool oldvalue = m_canvasobject->GetSelected();

    m_canvasobject->SetSpecificFlags( m_bool, m_which );
    //the last call never sets objects pending, still we want it here to be able to see
    //the possible effects.
    m_canvasobject->SetPending( true );

    m_bool = oldvalue;

    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_SetFlag
//----------------------------------------------------------------------------

a2dCommand_SetPinFlags::a2dCommand_SetPinFlags( a2dPin* pin, a2dPin::a2dPinFlagsMask which, bool value ):
    a2dCommand( true, a2dCommand_SetFlag::Id )
{
    m_which = which;
    m_value = value;
    m_pin = pin;
}

a2dCommand_SetPinFlags::~a2dCommand_SetPinFlags( void )
{
}

a2dCommand* a2dCommand_SetPinFlags::CloneAndBind( a2dObject* object )
{
    wxASSERT( !m_pin );
    return new a2dCommand_SetPinFlags( wxStaticCast( object, a2dPin ), m_which, m_value );
}

bool a2dCommand_SetPinFlags::Do( void )
{
    a2dPin::a2dPinFlagsMask old = m_pin->GetPinFlags();

    m_pin->SetPinFlags( m_which, m_value );
    m_oldmask = old;
    return true;
}

bool a2dCommand_SetPinFlags::Undo( void )
{
    m_pin->SetPinFlags( m_oldmask );
    return true;
}

//----------------------------------------------------------------------------
// a2dCommandMasked
//----------------------------------------------------------------------------

a2dCommandMasked::a2dCommandMasked( 
				a2dCanvasObject* parentObject,
				a2dCommandGroup* parentGroup,
				a2dCanvasObjectFlagsMask mask,
				a2dCanvasObjectFlagsMask targetMask,
				bool index, bool rewire,
                const a2dCommandId& commandId,
                const a2dCommandId& commandTypeId,
                const wxString& menuString
              )
    : a2dCommandGroup( parentGroup, commandId, commandTypeId, menuString )
{
	m_which = mask;
	m_targetMask = targetMask;
	m_parentObject = parentObject;
	m_index = index;
	m_rewire = rewire;

    wxString idName;
    idName.Printf( wxT( "gr_%s_%ld" ), m_commandId->GetName().c_str(), wxGenNewId() );

    m_groupName =  m_commandId->GetName();
	m_menuString = idName;
}

a2dCommandMasked::~a2dCommandMasked( void )
{
}

a2dObject* a2dCommandMasked::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    a2dCommandMasked* clgr = new a2dCommandMasked( m_parentObject, m_parentGroup, m_which, m_targetMask );

    a2dCommandList::const_iterator iter = m_subcommands.begin();
    while( iter != m_subcommands.end() )
    {
        a2dCommand* obj = *iter;
        a2dCommand* clone = ( a2dCommand* ) obj->Clone( options );
        clgr->m_subcommands.push_back( clone );
        iter++;
    }
    return clgr;
}

bool a2dCommandMasked::Do()
{
	int index = 0;
    for( a2dCanvasObjectList::iterator iter = m_parentObject->GetChildObjectList()->begin();
            iter != m_parentObject->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
		if ( !obj->GetRelease() )
		{
			if ( obj->CheckMask( m_which ) )
			{
				if ( m_index )
					m_objectsIndex.push_back( index );
				m_maskedObjects.push_back( obj );
			}

			if ( obj->CheckMask( m_targetMask ) )
				m_target = obj;
			
			index++;
		}
    }

	if ( !m_rewire )
		return true;

    bool EditCopy = true;

    a2dCanvasObjectList objects;
 
    m_parentObject->CollectObjects( &objects, _T( "" ), m_which );
	
    // remove wire from masked objects, since they will be rerouted.
    a2dCanvasObjectList::iterator iter = objects.begin();
    while( iter != objects.end() )
    {
        a2dCanvasObject* original = *iter;
		if ( original->IsConnect() )
			iter = objects.erase( iter );
		else
			iter++;
    }

	a2dRefMap refs;
    GetDrawHabitat()->GetConnectionGenerator()->SetNoEditCopy( true );
    if ( EditCopy )
    {
        GetDrawHabitat()->GetConnectionGenerator()->SetNoEditCopy( false );
	    for( a2dCanvasObjectList::iterator iter = objects.begin();
                iter != objects.end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj->CheckMask( m_which ) )
            {
                a2dCanvasObject* copy = obj->TClone( clone_toDrag | clone_childs | clone_seteditcopy | clone_setoriginal | clone_noCameleonRef, &refs );
                obj->SetBin( true );
                m_copies.push_back( copy );
                copy->SetSelected( false );
            }
        }
    }
	refs.LinkReferences( true);

    GetDrawHabitat()->GetConnectionGenerator()->PrepareForRewire( m_parentObject, objects, true, false, false, true, NULL, &refs );

	refs.LinkReferences( true );

	a2dCommandGroup::Do();

	return true;
}

bool a2dCommandMasked::PostDo()
{
	if ( !m_rewire )
		return true;

	bool EditCopy = true;
    if ( EditCopy )
    {
		//for reroute the wires need to be matrix free
		for( a2dCanvasObjectList::iterator iter = m_parentObject->GetChildObjectList()->begin();
				iter != m_parentObject->GetChildObjectList()->end(); ++iter )
		{
			a2dCanvasObject* obj = *iter;
			if ( obj->CheckMask( m_which ) && obj->IsConnect() )
			{
				a2dWirePolylineL* wire = wxStaticCast( obj, a2dWirePolylineL );
				if ( wire && ! wire->GetTransformMatrix().IsIdentity() )
					m_cmp->Submit( new a2dCommand_EliminateMatrix( wire ) );
			}
        }

        for( a2dCanvasObjectList::iterator iter = m_copies.begin(); iter != m_copies.end(); ++iter )
        {
            a2dCanvasObject* copy = *iter;
            a2dCanvasObject* original = a2dCanvasObject::PROPID_Original->GetPropertyValue( copy );
			a2dAffineMatrix mat = original->GetTransform();
            copy->SetTransform( mat );
        }
    }

    GetDrawHabitat()->GetConnectionGenerator()->RerouteWires( true );

    if ( EditCopy )
    {
        for( a2dCanvasObjectList::iterator iter = m_copies.begin(); iter != m_copies.end(); ++iter )
        {
            a2dCanvasObject* copy = *iter;
            a2dWalker_RemovePropertyCandoc setp( a2dCanvasObject::PROPID_Original );
            setp.SetSkipNotRenderedInDrawing( true );
            setp.Start( copy );
        }
        m_copies.clear();
    }
    for( a2dCanvasObjectList::iterator iter = m_maskedObjects.begin(); iter != m_maskedObjects.end(); ++iter )
    {
        a2dCanvasObject* original = *iter;
        a2dWalker_RemovePropertyCandoc setp( a2dCanvasObject::PROPID_Original );
        setp.SetSkipNotRenderedInDrawing( true );
        setp.AddPropertyId( a2dCanvasObject::PROPID_Editcopy );
        setp.Start( original );
    }

    GetDrawHabitat()->GetConnectionGenerator()->SetNoEditCopy( false );
	return true;
}

bool a2dCommandMasked::Undo()
{
    for( a2dCanvasObjectList::iterator iter = m_parentObject->GetChildObjectList()->begin();
            iter != m_parentObject->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
		if ( !obj->GetRelease() && obj->CheckMask( m_which ) )
		{
			obj->SetSpecificFlags( false, m_which );
			obj->SetSpecificFlags( false, m_targetMask );
			obj->SetPending( true );
		}
    }

	for( a2dCanvasObjectList::iterator iter = m_maskedObjects.begin();
            iter != m_maskedObjects.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        obj->SetSpecificFlags( true, m_which );
        if ( obj == m_target )
		{
			obj->SetSpecificFlags( true, m_targetMask );
			obj->SetPending( true );
		}
    }

	return a2dCommandGroup::Undo();
}

bool a2dCommandMasked::Redo()
{
    for( a2dCanvasObjectList::iterator iter = m_parentObject->GetChildObjectList()->begin();
            iter != m_parentObject->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
		if ( !obj->GetRelease() && obj->CheckMask( m_which ) )
		{
			obj->SetSpecificFlags( false, m_which );
			obj->SetSpecificFlags( false, m_targetMask );
			obj->SetPending( true );
		}
    }

	for( a2dCanvasObjectList::iterator iter = m_maskedObjects.begin();
            iter != m_maskedObjects.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        obj->SetSpecificFlags( true, m_which );
        if ( obj == m_target )
		{
			obj->SetSpecificFlags( true, m_targetMask );
			obj->SetPending( true );
		}
    }

	return a2dCommandGroup::Redo();
}

//----------------------------------------------------------------------------
// a2dCommand_SetSelectFlag
//----------------------------------------------------------------------------

a2dCommand_SetSelectFlag::a2dCommand_SetSelectFlag( a2dCanvasObject* object, bool value, bool preserve ):
    a2dCommand( true, a2dCommand_SetSelectFlag::Id )
{
    m_bool = value;
    m_canvasobject = object;
	m_oldvalue = false;
	m_preserve = preserve;
}

a2dCommand_SetSelectFlag::~a2dCommand_SetSelectFlag( void )
{
}

a2dCommand* a2dCommand_SetSelectFlag::CloneAndBind( a2dObject* object )
{
    wxASSERT( !m_canvasobject );
    return new a2dCommand_SetSelectFlag( wxStaticCast( object, a2dCanvasObject ), m_bool );
}

bool a2dCommand_SetSelectFlag::Do( void )
{
    // This is saved here, so that it can be undone.
    bool oldvalue = m_canvasobject->GetSelected();

    if ( m_canvasobject->GetRoot()->GetLayerSetup()  )
    {
        if ( m_canvasobject->GetRoot()->GetLayerSetup()->GetSelectable( m_canvasobject->GetLayer() ) )
        {
            m_canvasobject->SetSelected( m_bool );
            //the last call never sets objects pending, still we want it here to be able to see
            //the possible effects.
             m_canvasobject->GetRoot()->SetUpdatesPending( true );
        }
    }
    else
    {
        m_canvasobject->SetSelected( m_bool );
        //the last call never sets objects pending, still we want it here to be able to see
        //the possible effects.
        m_canvasobject->GetRoot()->SetUpdatesPending( true );
    }
    m_oldvalue = oldvalue;
    return true;
}

bool a2dCommand_SetSelectFlag::Undo( void )
{
    bool oldvalue = m_canvasobject->GetSelected();

	if ( m_preserve )
		m_canvasobject->SetSelected( m_bool );
	else
		m_canvasobject->SetSelected( m_oldvalue );
    //the last call never sets objects pending, still we want it here to be able to see
    //the possible effects.
    m_canvasobject->GetRoot()->SetUpdatesPending( true );

    m_oldvalue = oldvalue;

    return true;
}

bool a2dCommand_SetSelectFlag::Redo()
{
	return Undo();
}

//----------------------------------------------------------------------------
// a2dCommand_SetSelect2Flag
//----------------------------------------------------------------------------

a2dCommand_SetSelect2Flag::a2dCommand_SetSelect2Flag( a2dCanvasObject* object, bool value, bool preserve ):
    a2dCommand( true, a2dCommand_SetSelect2Flag::Id )
{
    m_bool = value;
	m_preserve = preserve;
    m_canvasobject = object;
}

a2dCommand_SetSelect2Flag::~a2dCommand_SetSelect2Flag( void )
{
}

a2dCommand* a2dCommand_SetSelect2Flag::CloneAndBind( a2dObject* object )
{
    wxASSERT( !m_canvasobject );
    return new a2dCommand_SetSelect2Flag( wxStaticCast( object, a2dCanvasObject ), m_bool );
}

bool a2dCommand_SetSelect2Flag::Do( void )
{
    // This is saved here, so that it can be undone.
    bool oldvalue = m_canvasobject->GetSelected2();

    if ( m_canvasobject->GetRoot()->GetLayerSetup()  )
    {
        if ( m_canvasobject->GetRoot()->GetLayerSetup()->GetSelectable( m_canvasobject->GetLayer() ) )
        {
            m_canvasobject->SetSelected2( m_bool );
            //the last call never sets objects pending, still we want it here to be able to see
            //the possible effects.
            m_canvasobject->SetPending( true );
        }
    }
    else
    {
        m_canvasobject->SetSelected2( m_bool );
        //the last call never sets objects pending, still we want it here to be able to see
        //the possible effects.
        m_canvasobject->SetPending( true );
    }
    m_bool = oldvalue;
    return true;
}

bool a2dCommand_SetSelect2Flag::Undo( void )
{
    bool oldvalue = m_canvasobject->GetSelected();

	if ( m_preserve )
		m_canvasobject->SetSelected2( m_bool );
	else
		m_canvasobject->SetSelected2( m_oldvalue );
    //the last call never sets objects pending, still we want it here to be able to see
    //the possible effects.
    m_canvasobject->SetPending( true );

    m_oldvalue = oldvalue;

    return true;
}

bool a2dCommand_SetSelect2Flag::Redo()
{
	return Undo();
}

//----------------------------------------------------------------------------
// a2dCommand_TransformMask
//----------------------------------------------------------------------------

a2dCommand_TransformMask::a2dCommand_TransformMask( a2dCanvasObject* parent, const a2dAffineMatrix& newtransform,
        a2dCanvasObjectFlagsMask which, bool rewire ):
	a2dCommandMasked( parent, NULL, which, a2dCanvasOFlags::SELECTED2, false, rewire, Id )
{
    m_modifies = true;

    m_transform = newtransform;
}

a2dCommand_TransformMask::~a2dCommand_TransformMask( void )
{
}

bool a2dCommand_TransformMask::Do( void )
{
	GetCanvasCmp()->SetCurrentGroup( this );

	a2dCommandMasked::Do();

    for( a2dCanvasObjectList::iterator iter = m_parentObject->GetChildObjectList()->begin();
            iter != m_parentObject->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj->CheckMask( m_which ) && ( !m_rewire || ( m_rewire && !obj->IsConnect() )) )
        {
			if ( obj->GetSelected2() )
				m_target = obj;
			m_objectsMatrix.push_back( a2dCommandMatrixData( m_parentObject, obj, obj->GetTransformMatrix() ) );
            //a2dAffineMatrix tworld = m_transform;
            //tworld *= obj->GetTransformMatrix();
            obj->Transform( m_transform );
        }
    }

//    GetCanvasCmp()->GetParentObject()->GetChildObjectList()->SetTransform( m_transform, _T("", m_which, _T("" );

    return true;
}

bool a2dCommand_TransformMask::PostDo(void)
{
	a2dCommandMasked::PostDo();
	GetCanvasCmp()->CommandGroupEnd( this );
    return true;
}

bool a2dCommand_TransformMask::Undo( void )
{
	a2dCommandMasked::Undo();

    a2dAffineMatrix inverse = m_transform;
    inverse.Invert();
    a2dObjectMatrixList::iterator iter = m_objectsMatrix.begin();
    while ( iter != m_objectsMatrix.end() )
    {
        a2dCommandMatrixData objm = *iter;
        objm.m_object->Transform( inverse );
        a2dCanvasObject* obj = objm.m_object;

        //obj->m_object->SetTransformMatrix( obj->m_transform  );
        iter++;
    }

    return true;
}

bool a2dCommand_TransformMask::Redo( void )
{
    a2dObjectMatrixList::iterator iter = m_objectsMatrix.begin();
    while ( iter != m_objectsMatrix.end() )
    {
        a2dCommandMatrixData objm = *iter;
        objm.m_object->Transform( m_transform );
        a2dCanvasObject* obj = objm.m_object;

        //obj->m_object->SetTransformMatrix( obj->m_transform  );
        iter++;
    }

	a2dCommandMasked::Redo();

    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_RotateMask
//----------------------------------------------------------------------------

a2dCommand_RotateMask::a2dCommand_RotateMask( a2dCanvasObject* parent, double angle, bool bboxCenter,
        a2dCanvasObjectFlagsMask which ):
	a2dCommandMasked( parent, NULL, which, a2dCanvasOFlags::SELECTED2, false, true, Id )
{
    m_modifies = true;

    m_angle = angle;
    m_bboxCenter = bboxCenter;
}

a2dCommand_RotateMask::~a2dCommand_RotateMask( void )
{
}

bool a2dCommand_RotateMask::Do( void )
{
	GetCanvasCmp()->SetCurrentGroup( this );

	a2dCommandMasked::Do();

    for( a2dCanvasObjectList::iterator iter = m_parentObject->GetChildObjectList()->begin();
            iter != m_parentObject->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj->CheckMask( m_which )  && !obj->IsConnect() )
        {
            m_objectsMatrix.push_back( a2dCommandMatrixData( m_parentObject, obj, obj->GetTransformMatrix() ) );
            if ( m_bboxCenter && !wxDynamicCast( obj, a2dPort ) ) //port is always around pin at (0,0)
            {
                a2dPoint2D p = obj->GetBbox().GetCentre();
                obj->Translate( -p.m_x, -p.m_y );
                obj->Rotate( -m_angle );
                obj->Translate( p.m_x, p.m_y );
            }
            else
            {
                a2dAffineMatrix mat = obj->GetTransform();
                mat.Rotate( m_angle, obj->GetPosX(), obj->GetPosY() );
                obj->SetTransformMatrix( mat );
            }
        }
    }

    return true;
}

bool a2dCommand_RotateMask::PostDo( void )
{
	a2dCommandMasked::PostDo();
	GetCanvasCmp()->CommandGroupEnd( this );
	return true;
}

bool a2dCommand_RotateMask::Undo( void )
{
    a2dCommandMasked::Undo();

    a2dObjectMatrixList::iterator iter = m_objectsMatrix.begin();
    while ( iter != m_objectsMatrix.end() )
    {
        a2dCommandMatrixData& matrixdata = *iter;
        a2dCanvasObject* obj = matrixdata.m_object;
        a2dAffineMatrix mat = obj->GetTransformMatrix();
        obj->SetTransformMatrix( matrixdata.m_transform  );
        matrixdata.m_transform = mat;

		iter++;
    }

    return true;
}


bool a2dCommand_RotateMask::Redo( void )
{
    a2dObjectMatrixList::iterator iter = m_objectsMatrix.begin();
    while ( iter != m_objectsMatrix.end() )
    {
        a2dCommandMatrixData& matrixdata = *iter;
        a2dCanvasObject* obj = matrixdata.m_object;
        a2dAffineMatrix mat = obj->GetTransformMatrix();
        obj->SetTransformMatrix( matrixdata.m_transform  );
        matrixdata.m_transform = mat;

		iter++;
    }

	a2dCommandMasked::Redo();

    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_AlignMask
//----------------------------------------------------------------------------

a2dCommand_AlignMask::a2dCommand_AlignMask( a2dCanvasObject* parent, Alignment align, a2dCanvasObjectFlagsMask which, bool doSnap ):
    a2dCommandMasked( parent, NULL, which, a2dCanvasOFlags::SELECTED2, false, true, a2dCommand_AlignMask::Id )
{
    m_modifies = true;

    m_parentObject = parent;
    m_objectsMatrix;
    m_which = which;
    m_align = align;
    m_doSnap = doSnap;
}

a2dCommand_AlignMask::~a2dCommand_AlignMask( void )
{
}

bool a2dCommand_AlignMask::Do( void )
{
    GetCanvasCmp()->SetCurrentGroup( this );
    a2dRestrictionEngine* restrict = NULL;
    if ( m_doSnap && m_parentObject->GetRoot()->GetHabitat() )
        restrict = m_parentObject->GetRoot()->GetHabitat()->GetRestrictionEngine();

    a2dSnapToWhatMask was;
    a2dSnapToWhatMask snapTargetFeatures;
    if ( restrict )
    {
        was = restrict->GetSnapSourceFeatures();
        //for time being only modes which are useful in dragging
        snapTargetFeatures = restrict->GetSnapTargetFeatures();
                    // OR specific a2dRestrictionEngine::snapToPinsUnconnected  | a2dRestrictionEngine::snapToPins;
                    // a2dRestrictionEngine::snapToGridPos | a2dRestrictionEngine::snapToGridPosForced ;

        restrict->SetSnapSourceFeatures( 
                a2dRestrictionEngine::snapToBoundingBox |
                a2dRestrictionEngine::snapToPins |
                a2dRestrictionEngine::snapToObjectVertexes |
                a2dRestrictionEngine::snapToObjectSnapVPath );
    }

	a2dCommandMasked::Do();

    double alignToMaxX = DBL_MIN;
    double alignToMinX = DBL_MAX;
    double alignToMaxY = DBL_MIN;
    double alignToMinY = DBL_MAX;
    double objectsHeight = 0;
    double objectsWidth = 0;
	a2dCanvasObject* lastSelect = 0;

	size_t HowManySelected = 0;
    for( a2dCanvasObjectList::iterator iter = m_parentObject->GetChildObjectList()->begin();
            iter != m_parentObject->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj->CheckMask( m_which ) && !obj->IsConnect() )
        {
			HowManySelected++;

			a2dBoundingBox box = obj->GetAlignmentBbox();

            alignToMaxY = wxMax( alignToMaxY, box.GetMaxY() );
            alignToMinY = wxMin( alignToMinY, box.GetMinY() );
            alignToMaxX = wxMax( alignToMaxX, box.GetMaxX() );
            alignToMinX = wxMin( alignToMinX, box.GetMinX() );
			objectsWidth += box.GetWidth();
			objectsHeight += box.GetHeight();
        }
    }

    switch ( m_align )
    {
		case MIDY_Dest:
		case MIDX_Dest:
		case MINX_Dest:
		case MAXX_Dest:
		case MINY_Dest:
		case MAXY_Dest:
		case MirrorVert_Dest:
		case MirrorHorz_Dest:
		case MirrorVertBbox_Dest:
		case MirrorHorzBbox_Dest:
        {
			for( a2dCanvasObjectList::iterator iter = m_parentObject->GetChildObjectList()->begin();
					iter != m_parentObject->GetChildObjectList()->end(); ++iter )
			{
				a2dCanvasObject* obj = *iter;
				if ( obj->GetSelected2() )
					lastSelect = obj;
			}
        }
	}

	if ( lastSelect )
	{
		a2dBoundingBox box = lastSelect->GetAlignmentBbox();

        alignToMaxY = box.GetMaxY();
        alignToMinY = box.GetMinY();
        alignToMaxX = box.GetMaxX();
        alignToMinX = box.GetMinX();
		switch ( m_align )
		{
			case MIDY_Dest:
			{
			}
			case MIDX_Dest:
			{
			}
			case MINX_Dest:
			{
			}
			case MAXX_Dest:
			{
			}
			case MINY_Dest:
			{
			}
			case MAXY_Dest:
			{
			}
			case MirrorVert_Dest:
			{
			}
			case MirrorHorz_Dest:
			{
			}
			case MirrorVertBbox_Dest:
			{
			}
			case MirrorHorzBbox_Dest:
			{
			}
		}
	}

    double alignMidX = ( alignToMaxX + alignToMinX ) / 2.0;
    double alignMidY = ( alignToMaxY + alignToMinY ) / 2.0;

	a2dCanvasObjectList sorted;

	if ( m_align == DistributeVert || m_align == DistributeHorz )
	{
		double dx = (alignToMaxX - alignToMinX - objectsWidth) / (HowManySelected-1);
		double dy = (alignToMaxY - alignToMinY - objectsHeight) / (HowManySelected-1);
		double posnext = alignToMinX;
		if ( m_align == DistributeVert )
			posnext = alignToMinY;

		for( a2dCanvasObjectList::iterator iter = m_parentObject->GetChildObjectList()->begin();
				iter != m_parentObject->GetChildObjectList()->end(); ++iter )
		{
			a2dCanvasObject* obj = *iter;
			if ( obj->CheckMask( m_which )  && !obj->IsConnect() )
			{
				a2dBoundingBox box = obj->GetAlignmentBbox();
				a2dCanvasObjectList::iterator itersort = sorted.begin();
				while ( itersort != sorted.end() )
				{
					a2dCanvasObject* objsort = *itersort;    
					a2dBoundingBox boxsort = objsort->GetAlignmentBbox();
					if ( m_align == DistributeVert && boxsort.GetMinY() <  box.GetMinY() )
						itersort++;
	    			else if ( m_align == DistributeHorz && boxsort.GetMinX() <  box.GetMinX() )
						itersort++;
					else
					{
						sorted.insert( itersort, obj );
						break;
					}
				}			 
				if ( itersort == sorted.end() )
					sorted.push_back( obj );
			}
		}
		for( a2dCanvasObjectList::iterator iter = sorted.begin(); iter != sorted.end(); ++iter )
		{
			a2dCanvasObject* obj = *iter;
			m_objectsMatrix.push_back( a2dCommandMatrixData( m_parentObject, obj, obj->GetTransformMatrix() ) );
			a2dAffineMatrix mat = obj->GetTransform();
			a2dBoundingBox box = obj->GetAlignmentBbox();
			switch ( m_align )
			{
				case DistributeVert:
					{
						double dyObjBbox = obj->GetPosY() - box.GetMinY();
                        double newposx, newposy;
                        newposx = obj->GetPosX();
                        newposy = posnext + dyObjBbox;
                        if( restrict )
                        {
                            a2dPoint2D point;
                            double dsx, dsy; //detect restriction distance of any point.
                            if ( restrict->RestrictCanvasObjectAtVertexes( obj, point, dsx, dsy, snapTargetFeatures, true ) )
                            {
                                //restrict the object drawing to that point
                                newposx += dsx;
                                newposy += dsy;
                            }
                        }
						obj->SetPosXY( newposx, newposy );
						posnext += box.GetHeight() + dy;
						break;
					}
				case DistributeHorz:
					{
						double dxObjBbox = obj->GetPosX() - box.GetMinX();
                        double newposx, newposy;
                        newposx = posnext + dxObjBbox;
                        newposy = obj->GetPosY();
                        if( restrict )
                        {
                            a2dPoint2D point;
                            double dsx, dsy; //detect restriction distance of any point.
                            if ( restrict->RestrictCanvasObjectAtVertexes( obj, point, dsx, dsy, snapTargetFeatures, true ) )
                            {
                                //restrict the object drawing to that point
                                newposx += dsx;
                                newposy += dsy;
                            }
                        }
						obj->SetPosXY( newposx, newposy );
						posnext += box.GetWidth() + dx;
						break;
					}
			}
		}
	}
    else
    {
	    //other aligns

	    for( a2dCanvasObjectList::iterator iter = m_parentObject->GetChildObjectList()->begin();
                iter != m_parentObject->GetChildObjectList()->end(); ++iter )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj->CheckMask( m_which ) && !obj->IsConnect() )
            {
                m_objectsMatrix.push_back( a2dCommandMatrixData( m_parentObject, obj, obj->GetTransformMatrix() ) );
                a2dAffineMatrix mat = obj->GetTransform();
			    a2dBoundingBox box = obj->GetAlignmentBbox();
                switch ( m_align )
                {
                case MIDY:
				    obj->SetPosXY( obj->GetPosX(), alignMidY + obj->GetPosY() - (box.GetMinY() + box.GetHeight()/2.0) ); 
                    break;
                case MIDY_Dest:
                    if ( obj != lastSelect )
    				    obj->SetPosXY( obj->GetPosX(), alignMidY + obj->GetPosY() - (box.GetMinY() + box.GetHeight()/2.0) ); 
                    break;
                case MIDX:
				    obj->SetPosXY( alignMidX + obj->GetPosX() - (box.GetMinX() + box.GetWidth()/2.0), obj->GetPosY() ); 
                    break;
                case MIDX_Dest:
                    if ( obj != lastSelect )
    				    obj->SetPosXY( alignMidX + obj->GetPosX() - (box.GetMinX() + box.GetWidth()/2.0), obj->GetPosY() ); 
                    break;
                case MINX:
                    obj->SetPosXY( alignToMinX + obj->GetPosX() - box.GetMinX(), obj->GetPosY()  ); 
                    break;
                case MINX_Dest:
                    if ( obj != lastSelect )
                        obj->SetPosXY( alignToMinX + obj->GetPosX() - box.GetMinX(), obj->GetPosY()  ); 
                    break;
                case MAXX:
				    obj->SetPosXY( alignToMaxX - box.GetMaxX() + obj->GetPosX(), obj->GetPosY()  ); 
                    break;
                case MAXX_Dest:
                    if ( obj != lastSelect )
				        obj->SetPosXY( alignToMaxX - box.GetMaxX() + obj->GetPosX(), obj->GetPosY()  ); 
                    break;
                case MINY:
				    obj->SetPosXY( obj->GetPosX(), alignToMinY + obj->GetPosY() - box.GetMinY() ); 
                    break;
                case MINY_Dest:
                    if ( obj != lastSelect )
    				    obj->SetPosXY( obj->GetPosX(), alignToMinY + obj->GetPosY() - box.GetMinY() ); 
                    break;
                case MAXY:
				    obj->SetPosXY( obj->GetPosX(), alignToMaxY - box.GetMaxY() + obj->GetPosY() ); 
                    break;
                case MAXY_Dest:
                    if ( obj != lastSelect )
    				    obj->SetPosXY( obj->GetPosX(), alignToMaxY - box.GetMaxY() + obj->GetPosY() ); 
                    break;
                case MirrorVert:
                    {
					    double x = (box.GetMinX() + box.GetMaxX())/2.0; 
					    double y = (box.GetMinY() + box.GetMaxY())/2.0; 
                        //double x = obj->GetPosX();
                        //double y = obj->GetPosY();
                        obj->Translate( -x, -y );
                        obj->Mirror( false, true );
                        obj->Translate( x, y );
                        break;
                    }
                case MirrorVert_Dest:
                    if ( obj != lastSelect )
                    {
					    double x = (box.GetMinX() + box.GetMaxX())/2.0; 
					    double y = (box.GetMinY() + box.GetMaxY())/2.0; 
                        //double x = obj->GetPosX();
                        //double y = obj->GetPosY();
                        obj->Translate( -x, -y );
                        obj->Mirror( false, true );
                        obj->Translate( x, y );
                        break;
                    }
                case MirrorHorz:
                    {
					    double x = (box.GetMinX() + box.GetMaxX())/2.0; 
					    double y = (box.GetMinY() + box.GetMaxY())/2.0; 
                        //double x = obj->GetPosX();
                        //double y = obj->GetPosY();
                        obj->Translate( -x, -y );
                        obj->Mirror( true, false );
                        obj->Translate( x, y );
                        break;
                    }
                case MirrorHorz_Dest:
                    if ( obj != lastSelect )
                    {
					    double x = (box.GetMinX() + box.GetMaxX())/2.0; 
					    double y = (box.GetMinY() + box.GetMaxY())/2.0; 
                        //double x = obj->GetPosX();
                        //double y = obj->GetPosY();
                        obj->Translate( -x, -y );
                        obj->Mirror( true, false );
                        obj->Translate( x, y );
                        break;
                    }
                case MirrorVertBbox:
                    {
                        double x = alignMidX; 
                        double y = alignMidY; 
                        obj->Translate( -x, -y );
                        obj->Mirror( false, true );
                        obj->Translate( x, y );
                        break;
                    }
                case MirrorVertBbox_Dest:
                    if ( obj != lastSelect )
                    {
                        double x = alignMidX; 
                        double y = alignMidY; 
                        obj->Translate( -x, -y );
                        obj->Mirror( false, true );
                        obj->Translate( x, y );
                        break;
                    }
                case MirrorHorzBbox:
                    {
                        double x = alignMidX; 
                        double y = alignMidY; 
                        obj->Translate( -x, -y );
                        obj->Mirror( true, false );
                        obj->Translate( x, y );
                        break;
                    }
                case MirrorHorzBbox_Dest:
                    if ( obj != lastSelect )
                    {
                        double x = alignMidX; 
                        double y = alignMidY; 
                        obj->Translate( -x, -y );
                        obj->Mirror( true, false );
                        obj->Translate( x, y );
                        break;
                    }
			    }

                double newposx, newposy;
                newposx = obj->GetPosX();
                newposy = obj->GetPosY();
                if( restrict )
                {
                    a2dPoint2D point;
                    double dsx, dsy; //detect restriction distance of any point.
                    if ( restrict->RestrictCanvasObjectAtVertexes( obj, point, dsx, dsy, snapTargetFeatures, true ) )
                    {
                        //restrict the object drawing to that point
                        newposx += dsx;
                        newposy += dsy;
    				    obj->SetPosXY( newposx, newposy );
                    }
                }
            }
        }
    }

    if( restrict )
        restrict->SetSnapSourceFeatures( was );

    return true;
}

bool a2dCommand_AlignMask::PostDo( void )
{
	a2dCommandMasked::PostDo();
	GetCanvasCmp()->CommandGroupEnd( this );
    return true;
}

bool a2dCommand_AlignMask::Undo( void )
{
	a2dCommandMasked::Undo();

    a2dObjectMatrixList::iterator iter = m_objectsMatrix.begin();
    while ( iter != m_objectsMatrix.end() )
    {
        a2dCommandMatrixData& matrixdata = *iter;
        a2dCanvasObject* obj = matrixdata.m_object;
        a2dAffineMatrix mat = obj->GetTransformMatrix();
        obj->SetTransformMatrix( matrixdata.m_transform  );
        matrixdata.m_transform = mat;

		iter++;
    }

    return true;
}

bool a2dCommand_AlignMask::Redo( void )
{
	a2dObjectMatrixList::iterator iter = m_objectsMatrix.begin();
    while ( iter != m_objectsMatrix.end() )
    {
        a2dCommandMatrixData& matrixdata = *iter;
        a2dCanvasObject* obj = matrixdata.m_object;
        a2dAffineMatrix mat = obj->GetTransformMatrix();
        obj->SetTransformMatrix( matrixdata.m_transform  );
        matrixdata.m_transform = mat;

		iter++;
    }

	a2dCommandMasked::Redo();

    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_AddPoint
//----------------------------------------------------------------------------

a2dCommand_AddPoint::a2dCommand_AddPoint( a2dCanvasObject* object, double x, double y, int index, bool afterinversion ):
    a2dCommand( true, a2dCommand_AddPoint::Id )
{
    m_x = x;
    m_y = y;
    m_canvasobject = object;
    m_afterinversion = afterinversion;

    a2dPolygonL* obj = wxStaticCast( m_canvasobject.Get(), a2dPolygonL );
    if ( index == -1 )
        m_seg = NULL;
    else 
    {
        a2dVertexList::iterator iter = obj->GetSegmentAtIndex( index );
        m_seg = *iter;
    }
}

a2dCommand_AddPoint::a2dCommand_AddPoint( a2dCanvasObject* object, double x, double y, a2dLineSegmentPtr seg, bool afterinversion ):
    a2dCommand( true, a2dCommand_AddPoint::Id )
{
    m_x = x;
    m_y = y;
    m_seg = seg;

    m_canvasobject = object;
    m_afterinversion = afterinversion;
}

a2dCommand_AddPoint::~a2dCommand_AddPoint( void )
{
}

bool a2dCommand_AddPoint::Do( void )
{
    if ( wxDynamicCast( m_canvasobject.Get(), a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxStaticCast( m_canvasobject.Get(), a2dPolygonL );
        m_segAdded = obj->AddPoint( m_x, m_y, m_seg, m_afterinversion );
        obj->SetPending( true );
    }
    return true;
}

bool a2dCommand_AddPoint::Undo( void )
{
    if ( wxDynamicCast( m_canvasobject.Get(), a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxStaticCast( m_canvasobject.Get(), a2dPolygonL );
        double xremoved, yxremoved;
        obj->RemoveSegment( xremoved, yxremoved, m_segAdded, m_afterinversion );
        obj->SetPending( true );
    }
    return true;
}

bool a2dCommand_AddPoint::Redo( void )
{
    //the redo re-uses the segment which was added in Do()
    if ( wxDynamicCast( m_canvasobject.Get(), a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxStaticCast( m_canvasobject.Get(), a2dPolygonL );
        int whereInsert = -1;
        if ( m_seg )
            whereInsert = obj->GetIndexSegment( m_seg );
        else
            whereInsert = obj->GetNumberOfSegments();
        obj->InsertSegment( whereInsert, m_segAdded );
        obj->SetPending( true );
    }
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_MoveSegment
//----------------------------------------------------------------------------

a2dCommand_MoveSegment::a2dCommand_MoveSegment( a2dCanvasObject* object, double x, double y, int index, bool afterinversion ):
    a2dCommand( true, a2dCommand_MoveSegment::Id )
{
    m_x = x;
    m_y = y;

    m_canvasobject = object;

    m_afterinversion = afterinversion;
}

a2dCommand_MoveSegment::a2dCommand_MoveSegment( a2dCanvasObject* object, double x, double y, a2dLineSegmentPtr seg, bool afterinversion ):
    a2dCommand( true, a2dCommand_MoveSegment::Id )
{
    m_x = x;
    m_y = y;
    m_seg = seg;

    m_canvasobject = object;

    m_afterinversion = afterinversion;
}

a2dCommand_MoveSegment::~a2dCommand_MoveSegment( void )
{
}

bool a2dCommand_MoveSegment::Do( void )
{

    if ( wxDynamicCast( m_canvasobject.Get(), a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxStaticCast( m_canvasobject.Get(), a2dPolygonL );
        double x, y;
        obj->GetPosXYSegment( m_seg, x, y, m_afterinversion );
        obj->SetPosXYSegment( m_seg, m_x, m_y, m_afterinversion );
		m_canvasobject->SetPending( true );

        // store undo information
        m_x = x;
        m_y = y;
    }
    else if ( wxDynamicCast( m_canvasobject.Get(), a2dSLine ) )
    {
        /*
                a2dSLine* obj = wxStaticCast( m_canvasobject.Get(), a2dSLine );
                double x, y;
                if ( m_seg == 0 )
                {
                    x = obj->GetPosX1( m_afterinversion );
                    y = obj->GetPosY1( m_afterinversion );
                    obj->SetPosXY1( m_x, m_y, m_afterinversion );
                }
                else
                {
                    x = obj->GetPosX2(m_afterinversion);
                    y = obj->GetPosY2(m_afterinversion);
                    obj->SetPosXY2( m_x, m_y, m_afterinversion );
                }

                m_x = x;
                m_y = y;
        */
    }

    return true;
}

bool a2dCommand_MoveSegment::Undo( void )
{
    // because the previous state is stored in Do, UnDo and Do are identical
    bool result = Do();

    // Don't know what this was good for and why it was in Undo, but not in Do
    //    a2dComEvent changed( this, m_canvasobject );
    //    ProcessEvent( changed );

    return result;
}

//----------------------------------------------------------------------------
// a2dCommand_MoveMidSegment
//----------------------------------------------------------------------------

a2dCommand_MoveMidSegment::a2dCommand_MoveMidSegment( a2dCanvasObject* object, double x, double y, a2dLineSegmentPtr seg, bool afterinversion ):
    a2dCommand( true, a2dCommand_MoveMidSegment::Id )
{
    m_x = x;
    m_y = y;
    m_seg = seg;

    m_canvasobject = object;

    m_afterinversion = afterinversion;
}

a2dCommand_MoveMidSegment::a2dCommand_MoveMidSegment( a2dCanvasObject* object, double x, double y, int index, bool afterinversion ):
    a2dCommand( true, a2dCommand_MoveMidSegment::Id )
{
    m_x = x;
    m_y = y;

    m_canvasobject = object;

    m_afterinversion = afterinversion;
}

a2dCommand_MoveMidSegment::~a2dCommand_MoveMidSegment( void )
{
}

bool a2dCommand_MoveMidSegment::Do( void )
{

    if ( wxDynamicCast( m_canvasobject.Get(), a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxStaticCast( m_canvasobject.Get(), a2dPolygonL );
        double x, y;
        obj->GetPosXYMidSegment( m_seg, x, y, m_afterinversion );
        obj->SetPosXYMidSegment( m_seg, m_x, m_y, m_afterinversion );
		m_canvasobject->SetPending( true );

        // store undo information
        m_x = x;
        m_y = y;
    }

    //TODO GetCanvasCmp()->GetCanvasDocument()->Modify( true );

    return true;
}

bool a2dCommand_MoveMidSegment::Undo( void )
{
    // because the previous state is stored in Do, UnDo and Do are identical
    bool result = Do();

    return result;
}

//----------------------------------------------------------------------------
// a2dCommand_RemoveSegmentoptimizew
//----------------------------------------------------------------------------

a2dCommand_RemoveSegment::a2dCommand_RemoveSegment( a2dCanvasObject* object, int index ):
    a2dCommand( true, a2dCommand_RemoveSegment::Id )
{
    m_canvasobject = object;
    m_index = 0;
}

a2dCommand_RemoveSegment::a2dCommand_RemoveSegment( a2dCanvasObject* object, a2dLineSegmentPtr seg ):
    a2dCommand( true, a2dCommand_RemoveSegment::Id )
{
    m_seg = seg;
    if ( wxDynamicCast( object, a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxStaticCast( object, a2dPolygonL );
        m_index = obj->GetIndexSegment( seg );
    }

    m_canvasobject = object;
}

a2dCommand_RemoveSegment::~a2dCommand_RemoveSegment( void )
{
}

bool a2dCommand_RemoveSegment::Do( void )
{

    if ( wxDynamicCast( m_canvasobject.Get(), a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxStaticCast( m_canvasobject.Get(), a2dPolygonL );
        double x, y;
        obj->RemoveSegment( x, y, m_seg, false );
		obj->SetPending( true );
    }
    return true;
}

bool a2dCommand_RemoveSegment::Undo( void )
{
    if ( wxDynamicCast( m_canvasobject.Get(), a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxStaticCast( m_canvasobject.Get(), a2dPolygonL );
        obj->InsertSegment( m_index, m_seg );
		obj->SetPending( true );
    }
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_SetSegments
//----------------------------------------------------------------------------

a2dCommand_SetSegments::a2dCommand_SetSegments( a2dCanvasObject* object, a2dVertexList* points, bool afterinversion )
    :  a2dCommand( true, a2dCommand_SetSegments::Id )
{
    m_canvasobject = object;
    m_segments = points;

    if( afterinversion )
    {
        // convert points list with inverse transform of object
        a2dAffineMatrix trans = object->GetTransformMatrix();
        trans.Invert();
        m_segments->Transform( trans );
    }
}

a2dCommand_SetSegments::~a2dCommand_SetSegments( void )
{
}

bool a2dCommand_SetSegments::Do( void )
{
    if ( wxDynamicCast( m_canvasobject.Get(), a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxStaticCast( m_canvasobject.Get(), a2dPolygonL );
        a2dSmrtPtr<a2dVertexList> old = obj->GetSegments();
        obj->SetSegments( m_segments );
        m_segments = old;
        m_canvasobject->SetPending( true );
    }
    else
    {
        // Doesn't make sense
        wxASSERT( 0 );
    }

    return true;
}

bool a2dCommand_SetSegments::Undo( void )
{
    // Do saves the old state, so Do and Undo are identical
    return Do();
}

//----------------------------------------------------------------------------
// a2dCommand_MovePoint
//----------------------------------------------------------------------------

a2dCommand_MovePoint::a2dCommand_MovePoint( a2dSLine* object, double x, double y, bool start, bool afterinversion ):
    a2dCommand( true, a2dCommand_MovePoint::Id )
{
    m_x = x;
    m_y = y;
    m_start = start;

    m_line = object;

    m_afterinversion = afterinversion;
}

a2dCommand_MovePoint::~a2dCommand_MovePoint( void )
{
}

bool a2dCommand_MovePoint::Do( void )
{
    double x, y;
    if ( m_start )
    {
        x = m_line->GetPosX1( m_afterinversion );
        y = m_line->GetPosY1( m_afterinversion );
        m_line->SetPosXY1( m_x, m_y, m_afterinversion );
    }
    else
    {
        x = m_line->GetPosX2( m_afterinversion );
        y = m_line->GetPosY2( m_afterinversion );
        m_line->SetPosXY2( m_x, m_y, m_afterinversion );
    }
	m_line->SetPending( true );

    m_x = x;
    m_y = y;
    return true;
}

bool a2dCommand_MovePoint::Undo( void )
{
    // because the previous state is stored in Do, UnDo and Do are identical
    bool result = Do();
    return result;
}


//----------------------------------------------------------------------------
// a2dCommand_ChangeText
//----------------------------------------------------------------------------

a2dCommand_ChangeText::a2dCommand_ChangeText( a2dText* object,
        wxString text, int caret,  const a2dFont& font, double linespace, unsigned int textflags )
    :  a2dCommand( true, a2dCommand_ChangeText::Id )
{
    m_canvasobject = object;
    m_text = text;
    m_caret = caret;
    m_font = font;
    m_linespace = linespace;
    m_textflags = textflags;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in Clone ( )
    CurrentSmartPointerOwner = this;
#endif
}

a2dCommand_ChangeText::~a2dCommand_ChangeText( void )
{
}

bool a2dCommand_ChangeText::Do( void )
{
    wxString oldtext = m_canvasobject->GetText();
    int oldcaret = m_canvasobject->GetCaret();
    a2dFont oldfont = m_canvasobject->GetFont();
    double oldlinespace = m_canvasobject->GetLineSpacing();
    unsigned int oldtextflags = m_canvasobject->GetTextFlags();

    m_canvasobject->SetText( m_text );
    m_canvasobject->SetCaret( m_caret );
    m_canvasobject->SetFont( m_font );
    m_canvasobject->SetLineSpacing( m_linespace );
    m_canvasobject->SetTextFlags( ( a2dText::a2dTextFlags ) m_textflags );
	m_canvasobject->SetPending( true );

    m_text = oldtext;
    m_caret = oldcaret;
    m_font = oldfont;
    m_linespace = oldlinespace;
    m_textflags = oldtextflags;

    return true;
}

bool a2dCommand_ChangeText::Undo( void )
{
    bool result = Do();
    return result;   
}

//----------------------------------------------------------------------------
// a2dCommand_ConnectPins
//----------------------------------------------------------------------------

a2dCommand_ConnectPins::a2dCommand_ConnectPins( a2dPin* pin1, a2dPin* pin2 )
    :   a2dCommand( true, a2dCommand_ConnectPins::Id )
{
    wxASSERT( pin1 );
    // pin2 may be 0

    m_pin1 = pin1;
    m_pin2 = pin2;
}

a2dCommand_ConnectPins::~a2dCommand_ConnectPins( void )
{
}

bool a2dCommand_ConnectPins::Do( void )
{
    wxASSERT( !m_pin1 || m_pin1->MayConnectTo( m_pin2 ) );
    wxASSERT( !m_pin2 || m_pin2->MayConnectTo( m_pin1 ) );

    m_pin1->ConnectTo( m_pin2 );
    return true;
}

bool a2dCommand_ConnectPins::Undo( void )
{
    m_pin1->Disconnect( m_pin2 );
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_DisConnectPins
//----------------------------------------------------------------------------

a2dCommand_DisConnectPins::a2dCommand_DisConnectPins( a2dPin* pin1, a2dPin* pin2 )
    :   a2dCommand( true, a2dCommand_DisConnectPins::Id )
{
    wxASSERT( pin1 );
    wxASSERT( pin2 );

    m_pin1 = pin1;
    m_pin2 = pin2; 
}

a2dCommand_DisConnectPins::~a2dCommand_DisConnectPins( void )
{
}

bool a2dCommand_DisConnectPins::Do( void )
{
    //reset pins to non connected
    m_pin1->Disconnect( m_pin2 );
    return true;
}

bool a2dCommand_DisConnectPins::Undo( void )
{
    m_pin1->ConnectTo( m_pin2 );
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_SetLayerMask
//----------------------------------------------------------------------------

bool a2dCommand_SetLayerMask::Do( void )
{
    if ( m_layer > wxMAXLAYER )
    {
        ( void )wxMessageBox( _( "Target Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        return false;
    }

    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    m_objects = objects->Clone( m_mask, a2dObject::clone_flat );

    if ( m_objects != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, m_objects )
        {
            a2dCanvasObject* obj = *iter;
            m_objectOldLayers.push_back( obj->GetLayer() );
        }
    }
    objects->ChangeLayer( m_layer, m_mask );

    return true;
}

bool a2dCommand_SetLayerMask::Undo( void )
{
    if ( m_objects != wxNullCanvasObjectList )
    {
        a2dLayerGroup::iterator iterl = m_objectOldLayers.begin();
        while ( iterl != m_objectOldLayers.end() )
        {
            forEachIn( a2dCanvasObjectList, m_objects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetLayer( ( *iterl ) );
                iterl++;
            }
        }
    }
    m_objects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_DeleteMask
//----------------------------------------------------------------------------
a2dCommand_DeleteMask::a2dCommand_DeleteMask( a2dCanvasObject* parent, a2dCanvasObjectFlagsMask mask ): 
	a2dCommandMasked( parent, NULL, mask, a2dCanvasOFlags::SELECTED2, false, false, a2dCommand_DeleteMask::Id )
{
    m_modifies = true;

}

bool a2dCommand_DeleteMask::Do( void )
{
    GetCanvasCmp()->SetCurrentGroup( this );

	a2dCommandMasked::Do();

    for( a2dCanvasObjectList::iterator iter = m_maskedObjects.begin(); iter != m_maskedObjects.end(); ++iter )
    {   
        a2dCanvasObject* obj = ( *iter );
		obj->SetBin( true );
    }

    for( a2dCanvasObjectList::iterator iter = m_maskedObjects.begin(); iter != m_maskedObjects.end(); ++iter )
    {   
        a2dCanvasObject* obj = ( *iter );
        a2dCommandPtr cmd = new a2dCommand_ReleaseObject( m_parentObject, obj );
        GetCanvasCmp()->Submit( cmd, true );
    }

    for( a2dCanvasObjectList::iterator iter = m_maskedObjects.begin(); iter != m_maskedObjects.end(); ++iter )
    {   
        a2dCanvasObject* obj = ( *iter );
		obj->SetBin( false );
    }

    GetCanvasCmp()->CommandGroupEnd( this );
    return true;
}

bool a2dCommand_DeleteMask::Undo( void )
{	
    a2dCommandMasked::Undo();
	return true;
}

bool a2dCommand_DeleteMask::Redo( void )
{
    a2dCommandMasked::Redo();
	return true;
}

//----------------------------------------------------------------------------
// a2dCommand_ToTopMask
//----------------------------------------------------------------------------

a2dCommand_ToTopMask::a2dCommand_ToTopMask( a2dCanvasObject* parent , a2dCanvasObjectFlagsMask mask ):
	a2dCommandMasked( parent, NULL, mask, a2dCanvasOFlags::SELECTED2, true, false, Id )
{
    m_modifies = true;

}

bool a2dCommand_ToTopMask::Do( void )
{
    GetCanvasCmp()->SetCurrentGroup( this );

	a2dCommandMasked::Do();

	a2dCanvasObjectList* objects = m_parentObject->GetChildObjectList();
	objects->BringToTop( m_which );

    GetCanvasCmp()->CommandGroupEnd( this );
    return true;
}

bool a2dCommand_ToTopMask::Undo( void )
{
	a2dCommandMasked::Undo();

    a2dCanvasObjectList* objects = m_parentObject->GetChildObjectList();
    a2dlist< long >::iterator index = m_objectsIndex.begin();
    while ( index != m_objectsIndex.end() )
    {
        forEachIn( a2dCanvasObjectList, &m_maskedObjects )
        {
            a2dCanvasObject* obj = *iter;
            objects->Release( obj, false, false, true );
            objects->Insert( *index, obj, true );
            index++;
        }
    }
    return true;
}

bool a2dCommand_ToTopMask::Redo( void )
{
	a2dCommandMasked::Redo();

	a2dCanvasObjectList* objects = m_parentObject->GetChildObjectList();
	objects->BringToTop( m_which );

	return true;
}


//----------------------------------------------------------------------------
// a2dCommand_ToBackMask
//----------------------------------------------------------------------------

a2dCommand_ToBackMask::a2dCommand_ToBackMask( a2dCanvasObject* parent , a2dCanvasObjectFlagsMask mask ):
	a2dCommandMasked( parent, NULL, mask, a2dCanvasOFlags::SELECTED2, true, false, Id )
{
    m_modifies = true;

}

bool a2dCommand_ToBackMask::Do( void )
{
    GetCanvasCmp()->SetCurrentGroup( this );

	a2dCommandMasked::Do();

	a2dCanvasObjectList* objects = m_parentObject->GetChildObjectList();
	objects->BringToBack( m_which );

    GetCanvasCmp()->CommandGroupEnd( this );
    return true;
}

bool a2dCommand_ToBackMask::Undo( void )
{
	a2dCommandMasked::Undo();

    a2dCanvasObjectList* objects = m_parentObject->GetChildObjectList();
    a2dlist< long >::iterator index = m_objectsIndex.begin();
    while ( index != m_objectsIndex.end() )
    {
        forEachIn( a2dCanvasObjectList, &m_maskedObjects )
        {
            a2dCanvasObject* obj = *iter;
            objects->Release( obj, false, false, true );
            objects->Insert( *index, obj, true );
            index++;
        }
    }
    return true;
}

bool a2dCommand_ToBackMask::Redo( void )
{
	a2dCommandMasked::Redo();

	a2dCanvasObjectList* objects = m_parentObject->GetChildObjectList();
	objects->BringToBack( m_which );

	return true;
}

//----------------------------------------------------------------------------
// a2dCommand_CopyMask
//----------------------------------------------------------------------------

bool a2dCommand_CopyMask::Do( void )
{
    m_target = m_target == -1 ? GetDrawHabitat()->GetTarget() : m_target;
    if ( m_target > wxMAXLAYER )
    {
        ( void )wxMessageBox( _( "Target Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        return false;
    }

    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();

    if ( objects != wxNullCanvasObjectList )
        objects->Copy( m_x, m_y, m_mask, m_toTarget == false ? -1 : m_target, true );

    m_objects = objects->CloneChecked( a2dObject::clone_flat );
    return true;
}

bool a2dCommand_CopyMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_objects != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, m_objects )
        {
            a2dCanvasObject* obj = *iter;
            //delete right now and not delayed in idle time, this is save for sure.
            objects->Release( obj, false, false, true );
        }
    }
    m_objects->clear();
    m_parent->SetPending( true );
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_MoveMask
//----------------------------------------------------------------------------

bool a2dCommand_MoveMask::Do( void )
{
    m_target = m_target == -1 ? GetDrawHabitat()->GetTarget() : m_target;
    if ( m_target > wxMAXLAYER )
    {
        ( void )wxMessageBox( _( "Target Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        return false;
    }

    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    m_objects = objects->Clone( m_mask, a2dObject::clone_flat );

    /* with depth maybe??
        a2dWalker_LayerCanvasObjects moveLayer( m_args.argValue.mask );
        moveLayer.SetRecursionDepth( 2 );
        moveLayer.SetOperation( a2dWalker_LayerCanvasObjects::moveLayers );
        moveLayer.SetTarget( m_args.argValue.target );
        moveLayer.Start( m_parent );
    */

    if ( m_objects != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, m_objects )
        {
            a2dCanvasObject* obj = *iter;
            m_objectOldLayers.push_back( obj->GetLayer() );
        }
        objects->Move( m_x, m_y, m_mask,  m_toTarget == false ? -1 : m_target );
        m_parent->SetPending( true );
    }

    return true;
}

bool a2dCommand_MoveMask::Undo( void )
{
    if ( m_objects != wxNullCanvasObjectList )
    {
        a2dLayerGroup::iterator iterl = m_objectOldLayers.begin();
        while ( iterl != m_objectOldLayers.end() )
        {
            forEachIn( a2dCanvasObjectList, m_objects )
            {
                a2dCanvasObject* obj = *iter;
                obj->Translate( -m_x, -m_y );
                obj->SetLayer( ( *iterl ) );
                iterl++;
            }
        }
    }
    m_objects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_GroupMask
//----------------------------------------------------------------------------

bool a2dCommand_GroupMask::Do( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    m_objects = objects->Clone( m_mask, a2dObject::clone_flat, &m_objectsIndex );
    if ( m_objects != wxNullCanvasObjectList )
    {
        m_groupobject = m_parent->CreateHierarchy( m_mask, false );
        if ( !m_name.IsEmpty() )
            m_groupobject->SetName( m_name );
        a2dWalker_SetSpecificFlagsCanvasObjects setflags( m_mask );
        setflags.SetSkipNotRenderedInDrawing( true );
        setflags.Start( m_parent, false );
        m_parent->SetPending( true );

        a2dDrawing* drawing = GetCanvasCmp()->m_drawing;

        if ( !m_name.IsEmpty() )//todo maybe event
        {
            a2dNameReference* ref = new a2dNameReference( 0, 0,  m_groupobject, m_groupobject->GetName() );
            ref->SetDrawFrame();
            drawing->GetRootObject()->Append( ref );
        }
    }
    return true;
}

bool a2dCommand_GroupMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_objects != wxNullCanvasObjectList )
    {
        objects->Release( m_groupobject, false, false, true );
        double x = m_groupobject->GetPosX();
        double y = m_groupobject->GetPosY();

        a2dlist< long >::iterator index = m_objectsIndex.begin();
        while ( index != m_objectsIndex.end() )
        {
            forEachIn( a2dCanvasObjectList, m_objects )
            {
                a2dCanvasObject* obj = *iter;
                obj->Translate( x, y );
                objects->Insert( *index, obj, true );
                index++;
            }
        }
    }
    m_objects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_UnGroupMask
//----------------------------------------------------------------------------

bool a2dCommand_UnGroupMask::Do( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    m_objects = objects->Clone( m_mask, a2dObject::clone_flat, &m_objectsIndex );
    if ( m_objects != wxNullCanvasObjectList )
    {
        a2dWalker_RemoveHierarchy remh( m_mask );
        if ( m_deep )
            remh.SetRecursionDepth( 20 );
        else
            remh.SetRecursionDepth( 1 );
        remh.SetSkipNotRenderedInDrawing( true );
        remh.Start( m_parent );
        m_parent->SetPending( true );
    }
    return true;
}

bool a2dCommand_UnGroupMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_objects != wxNullCanvasObjectList )
    {
        a2dlist< long >::iterator index = m_objectsIndex.begin();
        while ( index != m_objectsIndex.end() )
        {
            forEachIn( a2dCanvasObjectList, m_objects )
            {
                a2dCanvasObject* obj = *iter;
                objects->Insert( *index, obj, true );
                index++;
            }
        }
    }
    m_objects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_MergeMask
//----------------------------------------------------------------------------

bool a2dCommand_MergeMask::Do( void )
{
    m_target = m_target == -1 ? GetDrawHabitat()->GetTarget() : m_target;
    if ( m_target > wxMAXLAYER )
    {
        ( void )wxMessageBox( _( "Target Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        return false;
    }

    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_clearorg )
        m_objects = objects->Clone( m_mask, a2dObject::clone_flat, &m_objectsIndex );

    a2dDrawing* drawing = GetCanvasCmp()->m_drawing;

    if ( m_objects != wxNullCanvasObjectList )
    {
#if wxART2D_USE_KBOOL
        a2dBooleanWalkerHandler booloper( m_mask );
        booloper.SetReleaseOrignals( m_clearorg );
        booloper.SetClearTarget( false );
        a2dLayerGroup groupA;
        groupA.push_back( wxLAYER_ALL );
        booloper.SetGroupA( groupA );
        a2dLayerGroup groupB;
        booloper.SetGroupB( groupB );
        booloper.SetTarget( m_takeLayerFirstObject ? -1 : m_target );
        booloper.SetRecursionDepth( m_depth );
        booloper.SetSelectedOnlyA( true );
        booloper.SetSelectedOnlyB( true );

        double marge = GetDrawHabitat()->GetBooleanEngineMarge();
        booloper.SetMarge( marge / drawing->GetUnitsScale() );
        booloper.SetGrid( GetDrawHabitat()->GetBooleanEngineGrid() );
        booloper.SetDGrid( GetDrawHabitat()->GetBooleanEngineDGrid() );
        booloper.SetCorrectionAber( double( GetDrawHabitat()->GetBooleanEngineCorrectionAber() ) / drawing->GetUnitsScale() );
        booloper.SetCorrectionFactor( double( GetDrawHabitat()->GetBooleanEngineCorrectionFactor() ) / drawing->GetUnitsScale() );
        booloper.SetMaxlinemerge( double( GetDrawHabitat()->GetBooleanEngineMaxlinemerge() ) / drawing->GetUnitsScale() );
        booloper.SetWindingRule( GetDrawHabitat()->GetBooleanEngineWindingRule() );
        booloper.SetLinkHoles( GetDrawHabitat()->GetBooleanEngineLinkHoles() );
        booloper.SetRoundfactor( GetDrawHabitat()->GetBooleanEngineRoundfactor() );

        booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_OR );
        booloper.SetSkipNotRenderedInDrawing( true );
        booloper.Start( m_parent );

        // bin2 flags is set for resulting polygons.
        m_mergedObjects = objects->Clone( a2dCanvasOFlags::BIN2, a2dObject::clone_flat, NULL );

        a2dWalker_SetSpecificFlagsCanvasObjects setflags( m_mask );
        setflags.SetSkipNotRenderedInDrawing( true );
        setflags.Start( m_parent, false );
        m_parent->SetPending( true );
#endif //wxART2D_USE_KBOOL
    }
    return true;
}

bool a2dCommand_MergeMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_objects != wxNullCanvasObjectList )
    {
        a2dlist< long >::iterator index = m_objectsIndex.begin();
        while ( index != m_objectsIndex.end() )
        {
            forEachIn( a2dCanvasObjectList, m_objects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetRelease( false );
                objects->Insert( *index, obj, true );
                index++;
            }
        }
        m_objects->clear();
    }
    forEachIn( a2dCanvasObjectList, m_mergedObjects )
    {
        a2dCanvasObject* obj = *iter;
        //delete right now and not delayed in idle time, this is save for sure.
        objects->Release( obj, false, false, true );
    }
    m_mergedObjects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_OffsetMask
//----------------------------------------------------------------------------

bool a2dCommand_OffsetMask::Do( void )
{
    m_target = m_target == -1 ? GetDrawHabitat()->GetTarget() : m_target;
    if ( m_target > wxMAXLAYER )
    {
        ( void )wxMessageBox( _( "Target Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        return false;
    }
    a2dDrawing* drawing = GetCanvasCmp()->m_drawing;

    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_clearorg )
        m_objects = objects->Clone( m_mask, a2dObject::clone_flat, &m_objectsIndex );

    if ( m_objects != wxNullCanvasObjectList )
    {
#if wxART2D_USE_KBOOL
        a2dBooleanWalkerHandler booloper( m_mask );
        booloper.SetReleaseOrignals( m_clearorg );
        booloper.SetClearTarget( false );
        a2dLayerGroup groupA;
        groupA.push_back( wxLAYER_ALL );
        booloper.SetGroupA( groupA );
        a2dLayerGroup groupB;
        booloper.SetGroupB( groupB );
        booloper.SetTarget( m_takeLayerFirstObject ? -1 : m_target );
        booloper.SetRecursionDepth( m_depth );
        booloper.SetSelectedOnlyA( true );
        booloper.SetSelectedOnlyB( true );

        double marge = GetDrawHabitat()->GetBooleanEngineMarge();
        booloper.SetMarge( marge / drawing->GetUnitsScale() );
        booloper.SetGrid( GetDrawHabitat()->GetBooleanEngineGrid() );
        booloper.SetDGrid( GetDrawHabitat()->GetBooleanEngineDGrid() );
        booloper.SetCorrectionAber( double( GetDrawHabitat()->GetBooleanEngineCorrectionAber() ) / drawing->GetUnitsScale() );
        booloper.SetCorrectionFactor( double( GetDrawHabitat()->GetBooleanEngineCorrectionFactor() ) / drawing->GetUnitsScale() );
        booloper.SetMaxlinemerge( double( GetDrawHabitat()->GetBooleanEngineMaxlinemerge() ) / drawing->GetUnitsScale() );
        booloper.SetWindingRule( GetDrawHabitat()->GetBooleanEngineWindingRule() );
        booloper.SetLinkHoles( GetDrawHabitat()->GetBooleanEngineLinkHoles() );
        booloper.SetRoundfactor( GetDrawHabitat()->GetBooleanEngineRoundfactor() );

        booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_CORRECTION );
        booloper.SetSkipNotRenderedInDrawing( true );
        booloper.Start( m_parent );

        // bin2 flags is set for resulting polygons.
        m_offsetObjects = objects->Clone( a2dCanvasOFlags::BIN2, a2dObject::clone_flat, NULL );

        a2dWalker_SetSpecificFlagsCanvasObjects setflags( m_mask );
        setflags.SetSkipNotRenderedInDrawing( true );
        setflags.Start( m_parent, false );
        m_parent->SetPending( true );
#endif //wxART2D_USE_KBOOL
    }
    return true;
}

bool a2dCommand_OffsetMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_objects != wxNullCanvasObjectList )
    {
        a2dlist< long >::iterator index = m_objectsIndex.begin();
        while ( index != m_objectsIndex.end() )
        {
            forEachIn( a2dCanvasObjectList, m_objects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetRelease( false );
                objects->Insert( *index, obj, true );
                index++;
            }
        }
        m_objects->clear();
    }
    forEachIn( a2dCanvasObjectList, m_offsetObjects )
    {
        a2dCanvasObject* obj = *iter;
        //delete right now and not delayed in idle time, this is save for sure.
        objects->Release( obj, false, false, true );
    }
    m_offsetObjects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_CreateRingMask
//----------------------------------------------------------------------------

bool a2dCommand_CreateRingMask::Do( void )
{
    m_target = m_target == -1 ? GetDrawHabitat()->GetTarget() : m_target;
    if ( m_target > wxMAXLAYER )
    {
        ( void )wxMessageBox( _( "Target Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        return false;
    }
    a2dDrawing* drawing = GetCanvasCmp()->m_drawing;

    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_clearorg )
        m_objects = objects->Clone( m_mask, a2dObject::clone_flat, &m_objectsIndex );

    if ( m_objects != wxNullCanvasObjectList )
    {
#if wxART2D_USE_KBOOL
        a2dBooleanWalkerHandler booloper( m_mask );
        booloper.SetReleaseOrignals( m_clearorg );
        booloper.SetClearTarget( false );
        a2dLayerGroup groupA;
        groupA.push_back( wxLAYER_ALL );
        booloper.SetGroupA( groupA );
        a2dLayerGroup groupB;
        booloper.SetGroupB( groupB );
        booloper.SetTarget( m_takeLayerFirstObject ? -1 : m_target );
        booloper.SetRecursionDepth( m_depth );
        booloper.SetSelectedOnlyA( true );
        booloper.SetSelectedOnlyB( true );

        double marge = GetDrawHabitat()->GetBooleanEngineMarge();
        booloper.SetMarge( marge / drawing->GetUnitsScale() );
        booloper.SetGrid( GetDrawHabitat()->GetBooleanEngineGrid() );
        booloper.SetDGrid( GetDrawHabitat()->GetBooleanEngineDGrid() );
        booloper.SetCorrectionAber( double( GetDrawHabitat()->GetBooleanEngineCorrectionAber() ) / drawing->GetUnitsScale() );
        booloper.SetCorrectionFactor( double( GetDrawHabitat()->GetBooleanEngineCorrectionFactor() ) / drawing->GetUnitsScale() );
        booloper.SetMaxlinemerge( double( GetDrawHabitat()->GetBooleanEngineMaxlinemerge() ) / drawing->GetUnitsScale() );
        booloper.SetWindingRule( GetDrawHabitat()->GetBooleanEngineWindingRule() );
        booloper.SetLinkHoles( GetDrawHabitat()->GetBooleanEngineLinkHoles() );
        booloper.SetRoundfactor( GetDrawHabitat()->GetBooleanEngineRoundfactor() );

        booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_MAKERING );
        booloper.SetSkipNotRenderedInDrawing( true );
        booloper.Start( m_parent );

        // bin2 flags is set for resulting polygons.
        m_CreateRingObjects = objects->Clone( a2dCanvasOFlags::BIN2, a2dObject::clone_flat, NULL );

        a2dWalker_SetSpecificFlagsCanvasObjects setflags( m_mask );
        setflags.SetSkipNotRenderedInDrawing( true );
        setflags.Start( m_parent, false );
        m_parent->SetPending( true );
#endif //wxART2D_USE_KBOOL
    }
    return true;
}

bool a2dCommand_CreateRingMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_objects != wxNullCanvasObjectList )
    {
        a2dlist< long >::iterator index = m_objectsIndex.begin();
        while ( index != m_objectsIndex.end() )
        {
            forEachIn( a2dCanvasObjectList, m_objects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetRelease( false );
                objects->Insert( *index, obj, true );
                index++;
            }
        }
        m_objects->clear();
    }
    forEachIn( a2dCanvasObjectList, m_CreateRingObjects )
    {
        a2dCanvasObject* obj = *iter;
        //delete right now and not delayed in idle time, this is save for sure.
        objects->Release( obj, false, false, true );
    }
    m_CreateRingObjects->clear();
    return true;
}


//----------------------------------------------------------------------------
// a2dCommand_PolygonToSurfaceMask
//----------------------------------------------------------------------------

bool a2dCommand_PolygonToSurfaceMask::Do( void )
{
    m_target = m_target == -1 ? GetDrawHabitat()->GetTarget() : m_target;
    if ( m_target > wxMAXLAYER )
    {
        ( void )wxMessageBox( _( "Target Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        return false;
    }

    a2dDrawing* drawing = GetCanvasCmp()->m_drawing;

    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_clearorg )
        m_objects = objects->Clone( m_mask, a2dObject::clone_flat, &m_objectsIndex );

    if ( m_objects != wxNullCanvasObjectList )
    {
#if wxART2D_USE_KBOOL
        a2dBooleanWalkerHandler booloper( m_mask );
        booloper.SetReleaseOrignals( m_clearorg );
        booloper.SetClearTarget( false );
        a2dLayerGroup groupA;
        groupA.push_back( wxLAYER_ALL );
        booloper.SetGroupA( groupA );
        a2dLayerGroup groupB;
        booloper.SetGroupB( groupB );
        booloper.SetTarget( m_takeLayerFirstObject ? -1 : m_target );
        booloper.SetRecursionDepth( m_depth );
        booloper.SetSelectedOnlyA( true );
        booloper.SetSelectedOnlyB( true );

        double marge = GetDrawHabitat()->GetBooleanEngineMarge();
        booloper.SetMarge( marge / drawing->GetUnitsScale() );
        booloper.SetGrid( GetDrawHabitat()->GetBooleanEngineGrid() );
        booloper.SetDGrid( GetDrawHabitat()->GetBooleanEngineDGrid() );
        booloper.SetCorrectionAber( double( GetDrawHabitat()->GetBooleanEngineCorrectionAber() ) / drawing->GetUnitsScale() );
        booloper.SetCorrectionFactor( double( GetDrawHabitat()->GetBooleanEngineCorrectionFactor() ) / drawing->GetUnitsScale() );
        booloper.SetMaxlinemerge( double( GetDrawHabitat()->GetBooleanEngineMaxlinemerge() ) / drawing->GetUnitsScale() );
        booloper.SetWindingRule( GetDrawHabitat()->GetBooleanEngineWindingRule() );
        booloper.SetLinkHoles( false );
        booloper.SetRoundfactor( GetDrawHabitat()->GetBooleanEngineRoundfactor() );
        booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_Polygon2Surface );
        booloper.SetSkipNotRenderedInDrawing( true );

        booloper.Start( m_parent );

        // bin2 flags is set for resulting polygons.
        m_offsetObjects = objects->Clone( a2dCanvasOFlags::BIN2, a2dObject::clone_flat, NULL );

        a2dWalker_SetSpecificFlagsCanvasObjects setflags( m_mask );
        setflags.SetSkipNotRenderedInDrawing( true );
        setflags.Start( m_parent, false );
        m_parent->SetPending( true );
#endif //wxART2D_USE_KBOOL
    }
    return true;
}

bool a2dCommand_PolygonToSurfaceMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_objects != wxNullCanvasObjectList )
    {
        a2dlist< long >::iterator index = m_objectsIndex.begin();
        while ( index != m_objectsIndex.end() )
        {
            forEachIn( a2dCanvasObjectList, m_objects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetRelease( false );
                objects->Insert( *index, obj, true );
                index++;
            }
        }
        m_objects->clear();
    }
    forEachIn( a2dCanvasObjectList, m_offsetObjects )
    {
        a2dCanvasObject* obj = *iter;
        //delete right now and not delayed in idle time, this is save for sure.
        objects->Release( obj, false, false, true );
    }
    m_offsetObjects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_SurfaceToPolygonMask
//----------------------------------------------------------------------------

bool a2dCommand_SurfaceToPolygonMask::Do( void )
{
    m_target = m_target == -1 ? GetDrawHabitat()->GetTarget() : m_target;
    if ( m_target > wxMAXLAYER )
    {
        ( void )wxMessageBox( _( "Target Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        return false;
    }

    a2dDrawing* drawing = GetCanvasCmp()->m_drawing;

    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_clearorg )
        m_objects = objects->Clone( m_mask, a2dObject::clone_flat, &m_objectsIndex );

    if ( m_objects != wxNullCanvasObjectList )
    {
#if wxART2D_USE_KBOOL
        a2dBooleanWalkerHandler booloper( m_mask );
        booloper.SetReleaseOrignals( m_clearorg );
        booloper.SetClearTarget( false );
        a2dLayerGroup groupA;
        groupA.push_back( wxLAYER_ALL );
        booloper.SetGroupA( groupA );
        a2dLayerGroup groupB;
        booloper.SetGroupB( groupB );
        booloper.SetTarget( m_takeLayerFirstObject ? -1 : m_target );
        booloper.SetRecursionDepth( m_depth );
        booloper.SetSelectedOnlyA( true );
        booloper.SetSelectedOnlyB( true );

        double marge = GetDrawHabitat()->GetBooleanEngineMarge();
        booloper.SetMarge( marge / drawing->GetUnitsScale() );
        booloper.SetGrid( GetDrawHabitat()->GetBooleanEngineGrid() );
        booloper.SetDGrid( GetDrawHabitat()->GetBooleanEngineDGrid() );
        booloper.SetCorrectionAber( double( GetDrawHabitat()->GetBooleanEngineCorrectionAber() ) / drawing->GetUnitsScale() );
        booloper.SetCorrectionFactor( double( GetDrawHabitat()->GetBooleanEngineCorrectionFactor() ) / drawing->GetUnitsScale() );
        booloper.SetMaxlinemerge( double( GetDrawHabitat()->GetBooleanEngineMaxlinemerge() ) / drawing->GetUnitsScale() );
        booloper.SetWindingRule( GetDrawHabitat()->GetBooleanEngineWindingRule() );
        booloper.SetLinkHoles( true );
        booloper.SetRoundfactor( GetDrawHabitat()->GetBooleanEngineRoundfactor() );
        booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_Surface2Polygon );
        booloper.SetSkipNotRenderedInDrawing( true );

        booloper.Start( m_parent );

        // bin2 flags is set for resulting polygons.
        m_offsetObjects = objects->Clone( a2dCanvasOFlags::BIN2, a2dObject::clone_flat, NULL );

        a2dWalker_SetSpecificFlagsCanvasObjects setflags( m_mask );
        setflags.SetSkipNotRenderedInDrawing( true );
        setflags.Start( m_parent, false );
        m_parent->SetPending( true );
#endif //wxART2D_USE_KBOOL
    }
    return true;
}

bool a2dCommand_SurfaceToPolygonMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_objects != wxNullCanvasObjectList )
    {
        a2dlist< long >::iterator index = m_objectsIndex.begin();
        while ( index != m_objectsIndex.end() )
        {
            forEachIn( a2dCanvasObjectList, m_objects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetRelease( false );
                objects->Insert( *index, obj, true );
                index++;
            }
        }
        m_objects->clear();
    }
    forEachIn( a2dCanvasObjectList, m_offsetObjects )
    {
        a2dCanvasObject* obj = *iter;
        //delete right now and not delayed in idle time, this is save for sure.
        objects->Release( obj, false, false, true );
    }
    m_offsetObjects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_UrlOnMask
//----------------------------------------------------------------------------

bool a2dCommand_UrlOnMask::Do( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    m_oldobjects = objects->Clone( m_mask, a2dObject::clone_deep, &m_objectsIndex );
    m_objects = objects->Clone( m_mask, a2dObject::clone_flat );

    if ( m_objects != wxNullCanvasObjectList )
    {
        for( a2dCanvasObjectList::iterator iter = m_objects->begin(); iter != m_objects->end(); iter++ )
        {
            PROPID_URI->SetPropertyToObject( ( *iter ), m_uri.BuildURI() );
        }
    }

    return true;
}

bool a2dCommand_UrlOnMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_oldobjects != wxNullCanvasObjectList )
    {
        a2dlist< long >::iterator index = m_objectsIndex.begin();
        while ( index != m_objectsIndex.end() )
        {
            forEachIn( a2dCanvasObjectList, m_objects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetRelease( false );
                objects->Insert( *index, obj, true );
                index++;
            }
        }
        m_oldobjects->clear();
    }
    forEachIn( a2dCanvasObjectList, m_objects )
    {
        a2dCanvasObject* obj = *iter;
        //delete right now and not delayed in idle time, this is save for sure.
        objects->Release( obj, false, false, true );
    }
    m_objects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_SetFillStrokeMask
//----------------------------------------------------------------------------

bool a2dCommand_SetFillStrokeMask::Do( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    m_oldobjects = objects->Clone( m_mask, a2dObject::clone_deep, &m_objectsIndex );
    m_objects = objects->Clone( m_mask, a2dObject::clone_flat );

    if ( m_objects != wxNullCanvasObjectList )
    {
        m_objects->SetDrawerStyle( GetDrawHabitat()->GetFill(), GetDrawHabitat()->GetStroke(), m_mask );
    }

    return true;
}

bool a2dCommand_SetFillStrokeMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_oldobjects != wxNullCanvasObjectList )
    {
        a2dlist< long >::iterator index = m_objectsIndex.begin();
        while ( index != m_objectsIndex.end() )
        {
            forEachIn( a2dCanvasObjectList, m_oldobjects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetRelease( false );
                objects->Insert( *index, obj, true );
                index++;
            }
        }
        m_oldobjects->clear();
    }
    forEachIn( a2dCanvasObjectList, m_objects )
    {
        a2dCanvasObject* obj = *iter;
        //delete right now and not delayed in idle time, this is save for sure.
        objects->Release( obj, false, false, true );
    }
    m_objects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_SetFillStrokeExtMask
//----------------------------------------------------------------------------
const a2dCommandId a2dCommand_SetFillStrokeExtMask::Id( wxT( "SetFillStrokeExtMask" ) );

a2dCommand_SetFillStrokeExtMask::a2dCommand_SetFillStrokeExtMask( a2dCanvasObject* parent, const a2dExtStroke& extStroke, const a2dExtFill& extFill,  a2dCanvasObjectFlagsMask mask ):
	 a2dCommandMasked( parent, NULL, mask, a2dCanvasOFlags::SELECTED2, false, false, Id )
{
    m_modifies = true;
	m_extStroke = extStroke;
	m_extFill = extFill;
}

bool a2dCommand_SetFillStrokeExtMask::Do( void )
{
    GetCanvasCmp()->SetCurrentGroup( this );

	a2dCommandMasked::Do();

    for( a2dCanvasObjectList::iterator iter = m_maskedObjects.begin();
            iter != m_maskedObjects.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;

		a2dStroke oldStroke = obj->GetStroke();
		a2dFill   oldFill   = obj->GetFill();
		bool layerStroke = false;
		bool layerFill = false;

		// in case of layer stroke, use the stroke from the layer setup, as input to mix with the extended stroke.
		if ( oldStroke == *a2dNullSTROKE && obj->GetRoot()->GetLayerSetup() )
		{
			layerStroke = true;
			oldStroke = obj->GetRoot()->GetLayerSetup()->GetStroke( obj->GetLayer() );
		}
		// in case of layer fill, use the fill from the layer setup, as input to mix with the extended fill.
		if ( oldFill == *a2dNullFILL && obj->GetRoot()->GetLayerSetup() )
		{
			layerFill = true;
			oldFill = obj->GetRoot()->GetLayerSetup()->GetFill( obj->GetLayer() );
		}

		bool changed = false;
		a2dStroke s1Edited = m_extStroke.Get( oldStroke );
		if ( s1Edited != oldStroke && !( s1Edited == *a2dNullSTROKE && layerStroke ) )
		{
			changed = true;
			obj->SetPending( true );
			obj->SetStroke( s1Edited );
		}
		a2dFill f1Edited = m_extFill.Get( oldFill );
		if ( f1Edited != oldFill  && !( f1Edited == *a2dNullFILL && layerFill ) && obj->GetFilled() )
		{
			changed = true;
			obj->SetPending( true );
			obj->SetFill( f1Edited );
		}

		if ( changed )
		{
			m_modifiedObjects.push_back( obj );
			m_strokes.push_back( oldStroke );
			m_fills.push_back( oldFill );
		}
	}

    GetCanvasCmp()->CommandGroupEnd( this );
    return true;
}

bool a2dCommand_SetFillStrokeExtMask::Undo( void )
{
    a2dCommandMasked::Undo();

    RestorePreviousState();

    return true;
}

bool a2dCommand_SetFillStrokeExtMask::Redo( void )
{
    RestorePreviousState();

    a2dCommandMasked::Redo();

    return true;
}

void a2dCommand_SetFillStrokeExtMask::RestorePreviousState()
{
    a2dlist< a2dStroke > strokes;
    a2dlist< a2dFill > fills;

    a2dlist< a2dStroke >::iterator indexs = m_strokes.begin();
    a2dlist< a2dFill >::iterator indexf = m_fills.begin();
	forEachIn( a2dCanvasObjectList, &m_modifiedObjects )
	{
        a2dCanvasObject* obj = *iter;
		strokes.push_back( obj->GetStroke() );
		fills.push_back( obj->GetFill() );

		obj->SetStroke( *indexs );
		obj->SetFill( *indexf );
		obj->SetPending( true );

		indexs++; indexf++;
	}

	m_strokes = strokes;
	m_fills = fills;
}

//----------------------------------------------------------------------------
// a2dCommand_SetTextChangesMask
//----------------------------------------------------------------------------
const a2dCommandId a2dCommand_SetTextChangesMask::Id( wxT( "SetFontExtMask" ) );

a2dCommand_SetTextChangesMask::a2dCommand_SetTextChangesMask( a2dCanvasObject* parent, const a2dTextChanges& extFont, int alignment, a2dCanvasObjectFlagsMask mask ): 
	 a2dCommandMasked( parent, NULL, mask, a2dCanvasOFlags::SELECTED2, false, false, Id )
{
    m_modifies = true;
	m_extFont = extFont;
    m_alignment = alignment;
}

bool a2dCommand_SetTextChangesMask::Do( void )
{
    GetCanvasCmp()->SetCurrentGroup( this );

	a2dCommandMasked::Do();

	wxArrayString valid_styles, valid_extras;
	// Create a list of valid styles for the given font (from a2dExtChanges)
	if( !m_extFont.IsStyleValid() )
	{
		wxString common_fonttype = m_extFont.GetFont().GetFontInfo().GetType();
		wxString common_fontname = m_extFont.GetFont().GetFontInfo().GetName();

		a2dFontInfoList *fonts = m_extFont.GetFontInfoList();

		for ( size_t i = 0; i < fonts->GetCount(); i++ )
		{
			wxString fonttype = fonts->Item( i )->GetData()->GetType();
			wxString fontname = fonts->Item( i )->GetData()->GetName();
			wxString fontstyle = fonts->Item( i )->GetData()->GetStyle();
			wxString fontextra = fonts->Item( i )->GetData()->GetExtra();
				
			if ( ( fonttype == common_fonttype || common_fonttype == _( "* (All types)" ) )
					&& fontname == common_fontname )
			{
				valid_styles.push_back( fontstyle );
				valid_extras.push_back( fontextra );
			}
		}
	}

    for( a2dCanvasObjectList::iterator iter = m_maskedObjects.begin();
            iter != m_maskedObjects.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
    	a2dText* text = wxDynamicCast( obj, a2dText );

		if ( obj->GetRelease() || !text )
			continue;

		a2dFont oldFont = text->GetFont();
        int oldAlign = text->GetAlignment();
        unsigned int oldFlags = text->GetTextFlags();
		a2dFont f1Edited = m_extFont.GetFont( oldFont, valid_styles, valid_extras );
		int alignEdited = m_extFont.GetAlignment( oldAlign );
		unsigned int flagsEdited =	m_extFont.GetTextFlags( oldFlags );

		if( m_extFont.IsNameValid() && m_extFont.IsStyleValid() )
			text->SetWrongLoad( false );

		if ( f1Edited != oldFont || alignEdited != oldAlign || flagsEdited != oldFlags )
		{
			obj->SetPending( true );
			text->SetFont( f1Edited );
            text->SetAlignment( alignEdited  );
            text->SetTextFlags( flagsEdited );

			m_modifiedObjects.push_back( obj );
			m_fonts.push_back( oldFont );
            m_alignmentslist.push_back( oldAlign );
            m_textflagslist.push_back( oldFlags );
		}
	}

    GetCanvasCmp()->CommandGroupEnd( this );
    return true;
}

bool a2dCommand_SetTextChangesMask::Undo( void )
{
    a2dCommandMasked::Undo();

    RestorePreviousState();

    return true;
}

bool a2dCommand_SetTextChangesMask::Redo( void )
{
    RestorePreviousState();

    a2dCommandMasked::Redo();

    return true;
}

void a2dCommand_SetTextChangesMask::RestorePreviousState()
{
    a2dlist< a2dFont > fonts;
    a2dlist< int > alignmentslist;
    a2dlist< unsigned int > textflagslist;

    a2dlist< a2dFont >::iterator indexf = m_fonts.begin();
    a2dlist< int >::iterator indexal = m_alignmentslist.begin(); 
    a2dlist< unsigned int >::iterator indextf = m_textflagslist.begin();
	forEachIn( a2dCanvasObjectList, &m_modifiedObjects )
	{
        a2dCanvasObject* obj = *iter;
    	a2dText* text = wxDynamicCast( obj, a2dText );

		if ( obj->GetRelease() || !text )
			continue;
		fonts.push_back( text->GetFont() );
        alignmentslist.push_back( text->GetAlignment() );
        textflagslist.push_back( text->GetTextFlags() );

		text->SetFont( *indexf );
		text->SetAlignment( *indexal );
		text->SetTextFlags( *indextf );
		obj->SetPending( true );

		indexf++;
	}

	m_fonts = fonts;
    m_alignmentslist = alignmentslist;
    m_textflagslist = textflagslist;
}

//----------------------------------------------------------------------------
// a2dCommand_ImagesToRectMask
//----------------------------------------------------------------------------

bool a2dCommand_ImagesToRectMask::Do( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    m_oldobjects = objects->Clone( m_mask, a2dObject::clone_deep, &m_objectsIndex );
    m_objects = objects->Clone( m_mask, a2dObject::clone_flat );

    if ( m_objects != wxNullCanvasObjectList )
    {
        for( a2dCanvasObjectList::iterator iter = m_objects->begin(); iter != m_objects->end(); iter++ )
        {
            a2dImage* image = wxDynamicCast( ( *iter ).Get() , a2dImage );
            if ( image )
            {
                a2dCanvasObjectList* rectangles = image->GetAsRectangles( *wxBLACK, wxColour( 173, 173, 175 ), true );
                objects->TakeOverFrom( rectangles );
                delete rectangles;
            }
        }
    }

    return true;
}

bool a2dCommand_ImagesToRectMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_oldobjects != wxNullCanvasObjectList )
    {
        a2dlist< long >::iterator index = m_objectsIndex.begin();
        while ( index != m_objectsIndex.end() )
        {
            forEachIn( a2dCanvasObjectList, m_objects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetRelease( false );
                objects->Insert( *index, obj, true );
                index++;
            }
        }
        m_oldobjects->clear();
    }
    forEachIn( a2dCanvasObjectList, m_objects )
    {
        a2dCanvasObject* obj = *iter;
        //delete right now and not delayed in idle time, this is save for sure.
        objects->Release( obj, false, false, true );
    }
    m_objects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_SetLayersProperty
//----------------------------------------------------------------------------

bool a2dCommand_SetLayersProperty::Do( void )
{
    wxUint16 i = 0;
    for ( i = 0; i < m_layerSetup->GetChildObjectsCount(); i++ )
    {
        a2dLayerInfo* layerinfo = m_layerSetup->GetLayerIndex()[i];
        m_oldLayersVisible.push_back( layerinfo->GetVisible() );
        if ( layerinfo != wxNullLayerInfo )
        {
            switch ( m_setWhat )
            {
                case visible:
                    layerinfo->SetVisible( m_value );
                    break;
                case selectable:
                    layerinfo->SetSelectable( m_value );
                    break;
                case readable:
                    layerinfo->SetRead( m_value );
                    break;
                case filling:
                {
                    a2dFill fill = layerinfo->GetFill();
                    fill.SetFilling( m_value );
                    layerinfo->SetFill( fill );
                }
                break;
            }
        }
    }
    a2dDrawing* drawing = GetCanvasCmp()->m_drawing;
    a2dComEvent changedlayer( drawing, m_layerSetup, a2dDrawing::sig_layersetupChanged );
    drawing->ProcessEvent( changedlayer );
    return true;
}

bool a2dCommand_SetLayersProperty::Undo( void )
{
    wxUint16 layer = 0;
    a2dlist< bool >::iterator index = m_oldLayersVisible.begin();
    while ( index != m_oldLayersVisible.end() )
    {
        bool value = *index;
        a2dLayerInfo* layerinfo = m_layerSetup->GetLayerIndex()[layer];
        if ( layerinfo != wxNullLayerInfo )
        {
            switch ( m_setWhat )
            {
                case visible:
                    layerinfo->SetVisible( value );
                    break;
                case selectable:
                    layerinfo->SetSelectable( value );
                    break;
                case readable:
                    layerinfo->SetRead( value );
                    break;
            }
        }
    }
    a2dDrawing* drawing = GetCanvasCmp()->m_drawing;
    a2dComEvent changedlayer( drawing, m_layerSetup, a2dDrawing::sig_layersetupChanged );
    drawing->ProcessEvent( changedlayer );
    return true;
}

bool a2dCommand_Select::Do()
{
    a2dDrawing* drawing = GetCanvasCmp()->m_drawing;
    if ( m_args.argValue.what == SelectRect )
    {
        a2dBoundingBox bbox = a2dBoundingBox( m_args.argValue.x1, m_args.argValue.y1, m_args.argValue.x2, m_args.argValue.y2 );
        if ( m_parent->GetChildObjectList() != wxNullCanvasObjectList )
        {
            a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
            m_oldobjects = new a2dCanvasObjectList();
            double xmin, ymax;
            xmin = DBL_MAX;
            ymax = DBL_MIN;
            a2dCanvasObject* preserveLastSelected = 0;
            a2dCanvasObject* lastSelected = 0;

            forEachIn( a2dCanvasObjectList, objects )
            {
                a2dCanvasObject* obj = *iter;
                if ( obj->GetRelease() || !obj->IsVisible() )
				{
	                obj->SetSelected2( false );
                    continue;
				}               
                if ( obj->GetSelected() )
				{
	                if ( obj->GetSelected2() )
						preserveLastSelected = obj; //preserve it
	                obj->SetSelected2( false );
                    //continue;
				}
	            if ( obj->GetSelected2() ) // all others off
	                obj->SetSelected2( false );

                // filter on boundingbox, only object within it will pass.
                if ( bbox.GetValid() )
                {
                    a2dBoundingBox tmp;
                    tmp.Expand( obj->GetBbox() );
                    if ( bbox.Intersect( tmp ) != _IN )
                        continue;
                }

                //filter on visible and selectable layers
                if ( m_args.argValue.layervisible && m_args.argValue.layerselectable && obj->GetRoot() && obj->GetRoot()->GetLayerSetup() )
                {
                    if ( m_args.argValue.layervisible && !obj->GetRoot()->GetLayerSetup()->GetVisible( obj->GetLayer() ) )
                        continue;
                    if ( m_args.argValue.layerselectable && !obj->GetRoot()->GetLayerSetup()->GetSelectable( obj->GetLayer() ) )
                        continue;
                }
                //filter on layer if set.
                if ( !m_args.argSet.layer || ( m_args.argSet.layer && obj->GetLayer() == m_args.argValue.layer ) )
                {
                    //preserve for undo the select state of the objects 
                    m_oldobjectsSelected.push_back( obj->GetSelected() );
                    obj->SetSelected( true );
                    m_oldobjects->push_back( obj );
                }
            }
			if ( !lastSelected )
			{
				//search new lastselected
				// if for all objects: forEachIn( a2dCanvasObjectList, objects )
				forEachIn( a2dCanvasObjectList, m_oldobjects )
				{
					a2dCanvasObject* obj = *iter;
					obj->SetSelected2( false );
					if ( obj->GetRelease() || !obj->IsVisible() )
						continue;
					if ( !obj->GetSelected() )
						continue;

					if ( !lastSelected ) 
					{
						lastSelected = obj;
						xmin = obj->GetBboxMinX();
						ymax = obj->GetBboxMaxY();
					}
					else
					{
						if ( obj->GetBboxMinX() < xmin )
						{
							lastSelected = obj;
							xmin = obj->GetBboxMinX();
							ymax = obj->GetBboxMaxY();
						}
						else if ( obj->GetBboxMinX() == xmin )
						{
							if ( obj->GetBboxMaxY() < ymax )
							{
								lastSelected = obj;
								ymax = obj->GetBboxMaxY();
							}
						}
					}
				}
				if ( lastSelected ) 
					lastSelected->SetSelected2( true );
                else
                {
                    if ( preserveLastSelected )
					    preserveLastSelected->SetSelected2( true );
                }
			}
        }
        return true;
    }
    else if ( m_args.argValue.what == DeSelectRect )
    {
        a2dCanvasObject* preserveLastSelected = 0;
        a2dCanvasObject* lastSelected = 0;
        double xmin, ymax;
        xmin = DBL_MAX;
        ymax = DBL_MIN;
        a2dBoundingBox bbox = a2dBoundingBox( m_args.argValue.x1, m_args.argValue.y1, m_args.argValue.x2, m_args.argValue.y2 );
        if ( m_parent->GetChildObjectList() != wxNullCanvasObjectList )
        {
            a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
            m_oldobjects = new a2dCanvasObjectList();

            forEachIn( a2dCanvasObjectList, objects )
            {
                a2dCanvasObject* obj = *iter;
                if ( obj->GetRelease() || !obj->IsVisible()  || !obj->GetSelected() )
                    continue;
                if ( bbox.GetValid() )
                {
                    a2dBoundingBox tmp;
                    tmp.Expand( obj->GetBbox() );
                    if ( bbox.Intersect( tmp ) != _IN )
                        continue;
                }
                if ( m_args.argValue.layervisible && m_args.argValue.layerselectable && obj->GetRoot() && obj->GetRoot()->GetLayerSetup() )
                {
                    if ( m_args.argValue.layervisible && !obj->GetRoot()->GetLayerSetup()->GetVisible( obj->GetLayer() ) )
                        continue;
                    if ( m_args.argValue.layerselectable && !obj->GetRoot()->GetLayerSetup()->GetSelectable( obj->GetLayer() ) )
                        continue;
                }
                if ( !m_args.argSet.layer || ( m_args.argSet.layer && obj->GetLayer() == m_args.argValue.layer ) )
                {
                    m_oldobjectsSelected.push_back( obj->GetSelected() );
                    obj->SetSelected( false );
	                if ( obj->GetSelected2() )
						lastSelected = obj; //this was the last selected, need to search new one.
                    obj->SetSelected2( false );
                    m_oldobjects->push_back( obj );
                }
            }
			if ( lastSelected ) //lastselected was deselected
			{
				//search new lastselected
				// if for all objects: forEachIn( a2dCanvasObjectList, objects )
				forEachIn( a2dCanvasObjectList, m_oldobjects )
				{
					a2dCanvasObject* obj = *iter;
					obj->SetSelected2( false );
					if ( obj->GetRelease() || !obj->IsVisible() )
						continue;
					if ( !obj->GetSelected() )
						continue;

					if ( !lastSelected ) 
					{
						lastSelected = obj;
						xmin = obj->GetBboxMinX();
						ymax = obj->GetBboxMaxY();
					}
					else
					{
						if ( obj->GetBboxMinX() < xmin )
						{
							lastSelected = obj;
							xmin = obj->GetBboxMinX();
							ymax = obj->GetBboxMaxY();
						}
						else if ( obj->GetBboxMinX() == xmin )
						{
							if ( obj->GetBboxMaxY() < ymax )
							{
								lastSelected = obj;
								ymax = obj->GetBboxMaxY();
							}
						}
					}
				}
				if ( lastSelected ) 
					lastSelected->SetSelected2( true );
                else
                {
                    if ( preserveLastSelected )
					    preserveLastSelected->SetSelected2( true );
                }
			}

        }
        return true;
    }
    else if ( m_args.argValue.what == SelectAll )
    {
        if ( m_parent->GetChildObjectList() != wxNullCanvasObjectList )
        {
            double xmin, ymax;
            xmin = DBL_MAX;
            ymax = DBL_MIN;
            a2dCanvasObject* preserveLastSelected = 0;
            a2dCanvasObject* lastSelected = 0;
            a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
            m_oldobjects = new a2dCanvasObjectList();
            forEachIn( a2dCanvasObjectList, objects )
            {
                a2dCanvasObject* obj = *iter;
                if ( obj->GetRelease() || !obj->IsVisible() )
				{
	                obj->SetSelected2( false );
                    continue;
				}
                if ( obj->GetSelected() )
				{
	                if ( obj->GetSelected2() )
						preserveLastSelected = obj; //preserve it
	                obj->SetSelected2( false );
                    continue;
				}
	            if ( obj->GetSelected2() ) // all others off
	                obj->SetSelected2( false );

                if ( m_args.argValue.layervisible && m_args.argValue.layerselectable && obj->GetRoot() && obj->GetRoot()->GetLayerSetup() )
                {
                    if ( m_args.argValue.layervisible && !obj->GetRoot()->GetLayerSetup()->GetVisible( obj->GetLayer() ) )
                        continue;
                    if ( m_args.argValue.layerselectable && !obj->GetRoot()->GetLayerSetup()->GetSelectable( obj->GetLayer() ) )
                        continue;
                }
                if ( !m_args.argSet.layer || ( m_args.argSet.layer && obj->GetLayer() == m_args.argValue.layer ) )
                {
                    m_oldobjectsSelected.push_back( obj->GetSelected() );
                    obj->SetSelected( true );
                    m_oldobjects->push_back( obj );
                }
            }
			if ( !lastSelected )
			{
				//search new lastselected
				forEachIn( a2dCanvasObjectList, objects )
				{
					a2dCanvasObject* obj = *iter;
					obj->SetSelected2( false );
					if ( obj->GetRelease() || !obj->IsVisible() )
						continue;
					if ( !obj->GetSelected() )
						continue;

					if ( !lastSelected ) 
					{
						lastSelected = obj;
						xmin = obj->GetBboxMinX();
						ymax = obj->GetBboxMaxY();
					}
					else
					{
						if ( obj->GetBboxMinX() < xmin )
						{
							lastSelected = obj;
							xmin = obj->GetBboxMinX();
							ymax = obj->GetBboxMaxY();
						}
						else if ( obj->GetBboxMinX() == xmin )
						{
							if ( obj->GetBboxMaxY() < ymax )
							{
								lastSelected = obj;
								ymax = obj->GetBboxMaxY();
							}
						}
					}
				}
				if ( lastSelected ) 
					lastSelected->SetSelected2( true );
                else
                {
                    if ( preserveLastSelected )
					    preserveLastSelected->SetSelected2( true );
                }
			}
            drawing->UpdateAllViews( a2dCANVIEW_UPDATE_ALL );
        }
        return true;
    }
    else if ( m_args.argValue.what == DeSelectAll )
    {
        m_parent->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED );
        if ( m_parent->GetChildObjectList() != wxNullCanvasObjectList )
        {
            a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
            m_oldobjects = new a2dCanvasObjectList();
            forEachIn( a2dCanvasObjectList, objects )
            {
                a2dCanvasObject* obj = *iter;
                if ( obj->GetRelease() || !obj->IsVisible()  || !obj->GetSelected() )
                    continue;
                if ( m_args.argValue.layervisible && m_args.argValue.layerselectable && obj->GetRoot() && obj->GetRoot()->GetLayerSetup() )
                {
                    if ( m_args.argValue.layervisible && !obj->GetRoot()->GetLayerSetup()->GetVisible( obj->GetLayer() ) )
                        continue;
                    if ( m_args.argValue.layerselectable && !obj->GetRoot()->GetLayerSetup()->GetSelectable( obj->GetLayer() ) )
                        continue;
                }
                if ( !m_args.argSet.layer || ( m_args.argSet.layer && obj->GetLayer() == m_args.argValue.layer ) )
                {
                    m_oldobjectsSelected.push_back( obj->GetSelected() );
                    obj->SetSelected( false );
                    obj->SetSelected2( false );
                    m_oldobjects->push_back( obj );
                }
            }
            //drawing->UpdateAllViews( a2dCANVIEW_UPDATE_ALL );
        }
        return true;
    }
    return false;
}

bool a2dCommand_Select::Undo()
{
    a2dDrawing* drawing = GetCanvasCmp()->m_drawing;
    if ( m_args.argValue.what == SelectRect )
    {
        if ( m_oldobjects != wxNullCanvasObjectList )
        {
            a2dlist< bool >::iterator selectState = m_oldobjectsSelected.begin();
            forEachIn( a2dCanvasObjectList, m_oldobjects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetSelected( *selectState );
                selectState++;
            }
            drawing->UpdateAllViews( a2dCANVIEW_UPDATE_ALL );
        }
        return true;
    }
    else if ( m_args.argValue.what == DeSelectRect )
    {
        if ( m_oldobjects != wxNullCanvasObjectList )
        {
            a2dlist< bool >::iterator selectState = m_oldobjectsSelected.begin();
            forEachIn( a2dCanvasObjectList, m_oldobjects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetSelected( *selectState );
                selectState++;
            }
            drawing->UpdateAllViews( a2dCANVIEW_UPDATE_ALL );
        }
        return true;
    }
    else if ( m_args.argValue.what == SelectAll )
    {
        if ( m_oldobjects != wxNullCanvasObjectList )
        {
            a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
            objects->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED );
            a2dlist< bool >::iterator selectState = m_oldobjectsSelected.begin();
            forEachIn( a2dCanvasObjectList, m_oldobjects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetSelected( *selectState );
                selectState++;
            }
            drawing->UpdateAllViews( a2dCANVIEW_UPDATE_ALL );
        }
        return true;
    }
    else if ( m_args.argValue.what == DeSelectAll )
    {
        if ( m_oldobjects != wxNullCanvasObjectList )
        {
            a2dlist< bool >::iterator selectState = m_oldobjectsSelected.begin();
            forEachIn( a2dCanvasObjectList, m_oldobjects )
            {
                a2dCanvasObject* obj = *iter;
                obj->SetSelected( *selectState );
                selectState++;
            }
            drawing->UpdateAllViews( a2dCANVIEW_UPDATE_ALL );
        }
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// a2dCommand_ConvertMask
//----------------------------------------------------------------------------

bool a2dCommand_ConvertMask::PreDo()
{
    a2dCanvasObjectPtr parent = m_args.argValue.parent;
    a2dCanvasObjectList* objects = parent->GetChildObjectList();
    //select objects with this mask
    m_maskobjects = objects->Clone( m_args.argValue.mask, a2dObject::clone_flat, &m_objectsIndex );
    if ( m_args.argValue.deleteOriginal )
    {
        for( a2dCanvasObjectList::iterator iter = m_maskobjects->begin(); iter != m_maskobjects->end(); ++iter )
        {
            a2dCanvasObject* obj = ( *iter );
            // remove the pin connections in an undoable way
            obj->ClearAllPinConnections();
        }
        parent->ReleaseChildObjects( m_args.argValue.mask );
    }

    return true;
}

bool a2dCommand_ConvertMask::Do( void )
{
    a2dDrawing* drawing = GetCanvasCmp()->m_drawing;

    m_args.argValue.target = m_args.argValue.target == -1 ? GetDrawHabitat()->GetTarget() : m_args.argValue.target;
    a2dCanvasObjectPtr parent = m_args.argValue.parent;
    if ( m_args.argValue.target > wxMAXLAYER )
    {
        ( void )wxMessageBox( _( "Target Layer is not available" ), _( "Layers" ), wxICON_INFORMATION | wxOK );
        return false;
    }

    double radiusMin = double( GetDrawHabitat()->GetRadiusMin() ) / drawing->GetUnitsScale();
    double radiusMax = double( GetDrawHabitat()->GetRadiusMax() ) / drawing->GetUnitsScale();
    double AberPolyToArc = double( GetDrawHabitat()->GetAberPolyToArc() ) / drawing->GetUnitsScale();
    double AberArcToPoly = double( GetDrawHabitat()->GetAberArcToPoly() ) / drawing->GetUnitsScale();

    if ( m_maskobjects != wxNullCanvasObjectList )
    {
        m_newobjects = new a2dCanvasObjectList();

        long destLayer = m_args.argValue.target;
        forEachIn( a2dCanvasObjectList, m_maskobjects )
        {
            a2dCanvasObject* obj = *iter;
            if ( m_args.argValue.keepLayer )
                destLayer = obj->GetLayer();

            if ( m_args.argValue.what == ConvertPolygonToArcs )
            {
                if ( 0 != wxDynamicCast( obj, a2dPolylineL ) && obj->GetContourWidth() )
                {
                    a2dPolylineL* objc = wxDynamicCast( obj, a2dPolylineL );
                    bool returnIsPolygon;
                    a2dPolygonL* contour = new a2dPolygonL( objc->GetAsVertexList( returnIsPolygon ) );
                    contour->SetLayer( destLayer );
                    contour->SetBin( true );
                    contour->GetSegments()->ConvertPolylineToArc( AberPolyToArc, radiusMin, radiusMax );
                    parent->Append( contour );
                    m_newobjects->push_back( contour );
                }
                else if ( 0 != wxDynamicCast( obj, a2dPolygonL ) )
                {
                    a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                    cobj->SetLayer( destLayer );
                    cobj->SetBin( true );
                    parent->Append( cobj );
                    m_newobjects->push_back( cobj );
                    a2dPolygonL* objc = wxDynamicCast( cobj, a2dPolygonL );
                    objc->GetSegments()->ConvertPolygonToArc( AberPolyToArc, radiusMin, radiusMax );
                }
            }
            if ( m_args.argValue.what == ConvertPolylineToArcs )
            {
                if ( 0 != wxDynamicCast( obj, a2dPolylineL ) && obj->GetContourWidth() )
                {
                    a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                    a2dPolylineL* objc = wxDynamicCast( cobj, a2dPolylineL );
                    cobj->SetLayer( destLayer );
                    cobj->SetBin( true );
                    parent->Append( cobj );
                    m_newobjects->push_back( cobj );
                    objc->GetSegments()->ConvertPolylineToArc( AberPolyToArc, radiusMin, radiusMax );
                }
                else if ( 0 != wxDynamicCast( obj, a2dPolylineL ) )
                {
                    a2dCanvasObject* cobj = obj->TClone( a2dObject::clone_deep );
                    cobj->SetLayer( destLayer );
                    cobj->SetBin( true );
                    parent->Append( cobj );
                    m_newobjects->push_back( cobj );
                    a2dPolylineL* objc = wxDynamicCast( cobj, a2dPolylineL );
                    objc->GetSegments()->ConvertPolylineToArc( AberPolyToArc, radiusMin, radiusMax );
                }
            }
            if ( m_args.argValue.what == ConvertToPolygonPolylinesWithArcs )
            {
                a2dCanvasObjectList* polylist = GetCanvasCmp()->ConvertToPolygons( obj, true );
                if ( !polylist )
                    continue;
                for( a2dCanvasObjectList::iterator iterp = polylist->begin(); iterp != polylist->end(); ++iterp )
                {
                    a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                    a2dPolylineL* polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                    if ( polyline && polyline->GetNumberOfSegments() > 2 )
                    {
                        polyline->SetLayer( destLayer );
                        polyline->SetBin( true );
                        parent->Append( polyline );
                        m_newobjects->push_back( polyline );
                    }
                    else if ( poly && poly->GetNumberOfSegments() > 2 )
                    {
                        poly->SetLayer( destLayer );
                        poly->SetBin( true );
                        parent->Append( poly );
                        m_newobjects->push_back( poly );
                    }
                }
                delete polylist;
            }
            if ( m_args.argValue.what == ConvertToPolygonPolylinesWithoutArcs )
            {
                a2dCanvasObjectList* polylist = GetCanvasCmp()->ConvertToPolygons( obj, true );
                if ( !polylist )
                    continue;
                for( a2dCanvasObjectList::iterator iterp = polylist->begin(); iterp != polylist->end(); ++iterp )
                {

                    a2dSmrtPtr<a2dPolylineL> polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                    if ( polyline )
                    {
                        bool returnIsPolygon;
                        a2dSmrtPtr<a2dPolygonL> poly = new a2dPolygonL( polyline->GetAsVertexList( returnIsPolygon ) );
                        if ( poly && poly->GetSegments()->HasArcs() )
                            poly->GetSegments()->ConvertToLines( AberArcToPoly );

                        if ( poly )
                        {
                            if ( poly->GetNumberOfSegments() > 2 )
                            {
                                poly->SetLayer( destLayer );
                                poly->SetBin( true );
                                poly->GetSegments()->ConvertToLines( AberArcToPoly );
                                parent->Append( poly );
                                m_newobjects->push_back( poly.Get() );
                            }
                            else
                            {
                                polyline->SetLayer( destLayer );
                                polyline->SetBin( true );
                                polyline->GetSegments()->ConvertToLines( AberArcToPoly );
                                parent->Append( polyline );
                                m_newobjects->push_back( polyline.Get() );
                            }
                        }
                        else
                        {
                            polyline->SetLayer( destLayer );
                            polyline->SetBin( true );
                            polyline->GetSegments()->ConvertToLines( AberArcToPoly );
                            parent->Append( polyline );
                            m_newobjects->push_back( polyline.Get() );
                        }
                    }
                    else
                    {
                        a2dSmrtPtr<a2dPolygonL> poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                        if ( poly && poly->GetSegments()->HasArcs() )
                            poly->GetSegments()->ConvertToLines( AberArcToPoly );

                        if ( poly && poly->GetNumberOfSegments() > 2 )
                        {
                            poly->SetLayer( destLayer );
                            poly->SetBin( true );
                            poly->GetSegments()->ConvertToLines( AberArcToPoly );
                            parent->Append( poly );
                            m_newobjects->push_back( poly.Get() );
                        }
                    }
                }
                delete polylist;
            }
            if ( m_args.argValue.what == ConvertToVPaths )
            {
                a2dCanvasObjectList* vpath = GetCanvasCmp()->ConvertToVpath( obj, true );
                if ( vpath )
                {
                    for( a2dCanvasObjectList::iterator iterp = vpath->begin(); iterp != vpath->end(); ++iterp )
                    {
                        a2dVectorPath* path = wxDynamicCast( ( *iterp ).Get(), a2dVectorPath );
                        path->SetLayer( destLayer );
                        path->SetBin( true );
                        parent->Append( path );
                        m_newobjects->push_back( path );
                    }
                    delete vpath;
                }
            }
            if ( m_args.argValue.what == ConvertToPolylines )
            {
                a2dCanvasObjectList* polylist = obj->GetAsPolylines( true );
                if ( polylist == wxNullCanvasObjectList )
                    continue;
                int i, count = polylist->size();
                a2dCanvasObjectList::iterator iterp = polylist->begin();
                for( i = 0 ; i < count ; i++ )
                {
                    a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                    a2dPolylineL* polyline = wxDynamicCast( ( *iterp ).Get(), a2dPolylineL );
                    if ( polyline && polyline->GetNumberOfSegments() > 2 )
                    {
                        polyline->SetLayer( destLayer );
                        polyline->SetBin( true );
                        parent->Append( polyline );
                        m_newobjects->push_back( polyline );
                        iterp++;
                    }
                    else if ( poly && poly->GetNumberOfSegments() > 2 )
                    {
                        a2dPolylineL* conv = new a2dPolylineL( poly->GetSegments() );

                        poly->SetLayer( destLayer );
                        poly->SetBin( true );
                        parent->Append( conv );
                        m_newobjects->push_back( conv );
                        iterp = polylist->erase( iterp );
                    }
                }
                delete polylist;
            }
            if ( m_args.argValue.what == ConvertLinesArcs )
            {
                a2dCanvasObjectList* polylist = GetCanvasCmp()->ConvertToPolygons( obj, true );
                if ( !polylist )
                    continue;
                int i, count = polylist->size();
                a2dCanvasObjectList::iterator iterp = polylist->begin();
                for( i = 0 ; i < count ; i++ )
                {
                    a2dPolygonL* poly = wxDynamicCast( ( *iterp ).Get(), a2dPolygonL );
                    poly->SetLayer( destLayer );
                    poly->SetBin( true );
                    a2dCanvasObjectList* linesAndArcs = poly->GetAsLinesArcs();
                    forEachIn( a2dCanvasObjectList, linesAndArcs )
                    {
                        a2dCanvasObject* obj = *iter;
                        parent->Append( obj );
                    }
                    m_newobjects->TakeOverFrom( linesAndArcs );
                    iterp = polylist->erase( iterp );
                    delete linesAndArcs;
                }
                delete polylist;
            }
        }
        parent->SetPending( true );
    }

    return true;
}

bool a2dCommand_ConvertMask::Undo( void )
{
    if ( m_args.argValue.deleteOriginal && m_maskobjects != wxNullCanvasObjectList )
    {
        a2dlist< long >::iterator index = m_objectsIndex.begin();
        while ( index != m_objectsIndex.end() )
        {
            forEachIn( a2dCanvasObjectList, m_maskobjects )
            {
                a2dCanvasObject* obj = *iter;
                m_args.argValue.parent->Insert( *index, obj );
                index++;
            }
        }
    }
    m_maskobjects->clear();

    a2dCanvasObjectList* objects = m_args.argValue.parent->GetChildObjectList();
    if ( m_newobjects != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, m_newobjects )
        {
            a2dCanvasObject* obj = *iter;
            //delete right now and not delayed in idle time, this is save for sure.
            objects->Release( obj, false, false, true );
        }
    }
    m_newobjects->clear();
    m_args.argValue.parent->SetPending( true );
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_GroupAB
//----------------------------------------------------------------------------

bool a2dCommand_GroupAB::Do()
{
    m_args.argValue.target = m_args.argSet.target ? m_args.argValue.target : GetDrawHabitat()->GetTarget();
    m_args.argValue.clearTarget = m_args.argSet.clearTarget ? m_args.argValue.clearTarget : GetDrawHabitat()->GetClearTarget();
    m_args.argValue.selectedA = m_args.argSet.selectedA ? m_args.argValue.selectedA : GetDrawHabitat()->GetSelectedOnlyA();
    m_args.argValue.selectedB = m_args.argSet.selectedB ? m_args.argValue.selectedB : GetDrawHabitat()->GetSelectedOnlyB();

    a2dDrawing* drawing = GetCanvasCmp()->m_drawing;

    if ( m_args.argValue.what == ConvertToArcs )
    {
        a2dWalker_LayerCanvasObjects ConvertToArcs;
        ConvertToArcs.SetGroupA( GetDrawHabitat()->GetGroupA() );
        ConvertToArcs.SetOperation( a2dWalker_LayerCanvasObjects::ConvertToArcs );
        ConvertToArcs.SetRadiusMin( double( GetDrawHabitat()->GetRadiusMin() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetRadiusMax( double( GetDrawHabitat()->GetRadiusMax() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberPolyToArc( double( GetDrawHabitat()->GetAberPolyToArc() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberArcToPoly( double( GetDrawHabitat()->GetAberArcToPoly() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetTarget( m_args.argValue.target );
        ConvertToArcs.SetDetectCircle( m_args.argValue.detectCircle );
        ConvertToArcs.Start( m_parent );
    }
    if ( m_args.argValue.what == ConvertToPolygonPolylinesWithArcs )
    {
        a2dWalker_LayerCanvasObjects ConvertToArcs;
        ConvertToArcs.SetGroupA( GetDrawHabitat()->GetGroupA() );
        ConvertToArcs.SetOperation( a2dWalker_LayerCanvasObjects::ConvertToPolygonPolylinesWithArcs );
        ConvertToArcs.SetRadiusMin( double( GetDrawHabitat()->GetRadiusMin() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetRadiusMax( double( GetDrawHabitat()->GetRadiusMax() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberPolyToArc( double( GetDrawHabitat()->GetAberPolyToArc() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberArcToPoly( double( GetDrawHabitat()->GetAberArcToPoly() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetTarget( m_args.argValue.target );
        ConvertToArcs.Start( m_parent );
    }
    if ( m_args.argValue.what == ConvertToPolygonPolylinesWithoutArcs )
    {
        a2dWalker_LayerCanvasObjects ConvertToArcs;
        ConvertToArcs.SetGroupA( GetDrawHabitat()->GetGroupA() );
        ConvertToArcs.SetOperation( a2dWalker_LayerCanvasObjects::ConvertToPolygonPolylinesWithArcs );
        ConvertToArcs.SetAberPolyToArc( double( GetDrawHabitat()->GetAberPolyToArc() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberArcToPoly( double( GetDrawHabitat()->GetAberArcToPoly() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetTarget( m_args.argValue.target );
        ConvertToArcs.Start( m_parent );
    }
    if ( m_args.argValue.what == ConvertPolygonToArcs )
    {
        a2dWalker_LayerCanvasObjects ConvertToArcs;
        ConvertToArcs.SetGroupA( GetDrawHabitat()->GetGroupA() );
        ConvertToArcs.SetOperation( a2dWalker_LayerCanvasObjects::ConvertPolygonToArcs );
        ConvertToArcs.SetRadiusMin( double( GetDrawHabitat()->GetRadiusMin() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetRadiusMax( double( GetDrawHabitat()->GetRadiusMax() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberPolyToArc( double( GetDrawHabitat()->GetAberPolyToArc() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberArcToPoly( double( GetDrawHabitat()->GetAberArcToPoly() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetTarget( m_args.argValue.target );
        ConvertToArcs.SetDetectCircle( m_args.argValue.detectCircle );
        ConvertToArcs.Start( m_parent );
    }
    if ( m_args.argValue.what == ConvertPolylineToArcs )
    {
        a2dWalker_LayerCanvasObjects ConvertToArcs;
        ConvertToArcs.SetGroupA( GetDrawHabitat()->GetGroupA() );
        ConvertToArcs.SetOperation( a2dWalker_LayerCanvasObjects::ConvertPolylineToArcs );
        ConvertToArcs.SetRadiusMin( double( GetDrawHabitat()->GetRadiusMin() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetRadiusMax( double( GetDrawHabitat()->GetRadiusMax() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberPolyToArc( double( GetDrawHabitat()->GetAberPolyToArc() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberArcToPoly( double( GetDrawHabitat()->GetAberArcToPoly() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetTarget( m_args.argValue.target );
        ConvertToArcs.SetDetectCircle( m_args.argValue.detectCircle );
        ConvertToArcs.Start( m_parent );
    }
    if ( m_args.argValue.what == ConvertLinesArcs )
    {
        a2dWalker_LayerCanvasObjects ConvertToArcs;
        ConvertToArcs.SetGroupA( GetDrawHabitat()->GetGroupA() );
        ConvertToArcs.SetOperation( a2dWalker_LayerCanvasObjects::ConvertLinesArcs );
        ConvertToArcs.SetRadiusMin( double( GetDrawHabitat()->GetRadiusMin() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetRadiusMax( double( GetDrawHabitat()->GetRadiusMax() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberPolyToArc( double( GetDrawHabitat()->GetAberPolyToArc() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberArcToPoly( double( GetDrawHabitat()->GetAberArcToPoly() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetTarget( m_args.argValue.target );

        ConvertToArcs.Start( m_parent );
    }
    if ( m_args.argValue.what == ConvertToPolylines )
    {
        a2dWalker_LayerCanvasObjects ConvertToArcs;
        ConvertToArcs.SetGroupA( GetDrawHabitat()->GetGroupA() );
        ConvertToArcs.SetOperation( a2dWalker_LayerCanvasObjects::ConvertToPolylines );
        ConvertToArcs.SetAberPolyToArc( double( GetDrawHabitat()->GetAberPolyToArc() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberArcToPoly( GetDrawHabitat()->GetAberArcToPoly() / drawing->GetUnitsScale() );
        ConvertToArcs.SetTarget( m_args.argValue.target );

        ConvertToArcs.Start( m_parent );
    }
    if ( m_args.argValue.what == ConvertPointsAtDistance )
    {
        a2dConvertPointsAtDistance f;
        f.SetAberArcToPoly( GetDrawHabitat()->GetAberArcToPoly() / drawing->GetUnitsScale() );
        f.SetDistancePoints( GetDrawHabitat()->GetDistancePoints() / drawing->GetUnitsScale() );
        f.SetAtVertexPoints( GetDrawHabitat()->GetAtVertexPoints() );
        f.SetLastPointCloseToFirst( true );
        a2dWalker_ForEachCanvasObject< a2dConvertPointsAtDistance > ConvertTo( f );
        ConvertTo.SetGroupA( GetDrawHabitat()->GetGroupA() );
        ConvertTo.SetTarget( m_args.argValue.target );
        ConvertTo.Start( m_parent );

        /*
        a2dWalker_LayerCanvasObjects ConvertTo;
        ConvertTo.SetGroupA( GetDrawHabitat()->GetGroupA() );
        ConvertTo.SetOperation( a2dWalker_LayerCanvasObjects::ConvertPointsAtDistance );
        ConvertTo.SetAberPolyToArc( double( GetDrawHabitat()->GetAberPolyToArc() ) / drawing->GetUnitsScale() );
        ConvertTo.SetAberArcToPoly( GetDrawHabitat()->GetAberArcToPoly() / drawing->GetUnitsScale() );
        ConvertTo.SetDistancePoints( GetDrawHabitat()->m_distancePoints / drawing->GetUnitsScale() );
        ConvertTo.SetAtVertexPoints( GetDrawHabitat()->m_atVertexPoints );

        ConvertTo.SetTarget( m_args.argValue.target );
        ConvertTo.Start( m_parent );
        */
    }
    if ( m_args.argValue.what == ConvertToVPaths )
    {
        a2dWalker_LayerCanvasObjects ConvertToArcs;
        ConvertToArcs.SetGroupA( GetDrawHabitat()->GetGroupA() );
        ConvertToArcs.SetOperation( a2dWalker_LayerCanvasObjects::ConvertToVPaths );
        ConvertToArcs.SetAberPolyToArc( double( GetDrawHabitat()->GetAberPolyToArc() ) / drawing->GetUnitsScale() );
        ConvertToArcs.SetAberArcToPoly( GetDrawHabitat()->GetAberArcToPoly() / drawing->GetUnitsScale() );
        ConvertToArcs.SetTarget( m_args.argValue.target );

        ConvertToArcs.Start( m_parent );
    }
    if ( m_args.argValue.what == DeleteGroupA )
    {
        a2dWalker_LayerCanvasObjects deleteLayer;
        deleteLayer.SetGroupA( GetDrawHabitat()->GetGroupA() );
        deleteLayer.SetOperation( a2dWalker_LayerCanvasObjects::deleteLayers );
        deleteLayer.SetSkipNotRenderedInDrawing( true );

        deleteLayer.Start( m_parent );
    }
    if ( m_args.argValue.what == MoveGroupA )
    {
        a2dWalker_LayerCanvasObjects moveLayer;
        moveLayer.SetGroupA( GetDrawHabitat()->GetGroupA() );
        moveLayer.SetOperation( a2dWalker_LayerCanvasObjects::moveLayers );
        moveLayer.SetTarget( m_args.argValue.target );
        moveLayer.SetSkipNotRenderedInDrawing( true );

        moveLayer.Start( m_parent );
    }
    if ( m_args.argValue.what == CopyGroupA )
    {
        a2dWalker_LayerCanvasObjects copyLayer;
        copyLayer.SetGroupA( GetDrawHabitat()->GetGroupA() );
        copyLayer.SetOperation( a2dWalker_LayerCanvasObjects::copyLayers );
        copyLayer.SetTarget( m_args.argValue.target );
        copyLayer.SetSkipNotRenderedInDrawing( true );

        copyLayer.Start( m_parent );
    }
    else if (
        m_args.argValue.what == Boolean_OR ||
        m_args.argValue.what == Boolean_AND ||
        m_args.argValue.what == Boolean_EXOR ||
        m_args.argValue.what == Boolean_A_SUB_B ||
        m_args.argValue.what == Boolean_B_SUB_A ||
        m_args.argValue.what == Boolean_CORRECTION ||
        m_args.argValue.what == Boolean_SMOOTHEN ||
        m_args.argValue.what == Boolean_MAKERING ||
        m_args.argValue.what == Boolean_Polygon2Surface ||
        m_args.argValue.what == Boolean_Surface2Polygon
    )
    {
#if wxART2D_USE_KBOOL
        a2dBooleanWalkerHandler booloper( a2dCanvasOFlags::VISIBLE );
        booloper.SetReleaseOrignals( false );
        booloper.SetClearTarget( m_args.argValue.clearTarget );
        booloper.SetGroupA( GetDrawHabitat()->GetGroupA() );
        booloper.SetGroupB( GetDrawHabitat()->GetGroupB() );
        booloper.SetTarget( m_args.argValue.target );
        booloper.SetSelectedOnlyA( m_args.argValue.selectedA );
        booloper.SetSelectedOnlyB( m_args.argValue.selectedB );
        booloper.SetRecursionDepth( 20 );

        booloper.SetMarge( GetDrawHabitat()->GetBooleanEngineMarge() / drawing->GetUnitsScale() );
        booloper.SetGrid( GetDrawHabitat()->GetBooleanEngineGrid() );
        booloper.SetDGrid( GetDrawHabitat()->GetBooleanEngineDGrid() );
        booloper.SetCorrectionAber( GetDrawHabitat()->GetBooleanEngineCorrectionAber() / drawing->GetUnitsScale() );
        booloper.SetAberArcToPoly( GetDrawHabitat()->GetAberArcToPoly() / drawing->GetUnitsScale() );
        booloper.SetCorrectionFactor( GetDrawHabitat()->GetBooleanEngineCorrectionFactor() / drawing->GetUnitsScale() );
        booloper.SetMaxlinemerge( GetDrawHabitat()->GetBooleanEngineMaxlinemerge() / drawing->GetUnitsScale() );
        booloper.SetWindingRule( GetDrawHabitat()->GetBooleanEngineWindingRule() );
        booloper.SetRoundfactor( GetDrawHabitat()->GetBooleanEngineRoundfactor() );
        booloper.SetLinkHoles( true );

        long boolOp = a2dBooleanWalkerHandler::Boolean_NON;

        switch( m_args.argValue.what )
        {
            case  Boolean_OR:
            {
                boolOp = a2dBooleanWalkerHandler::Boolean_OR;
                break;
            }
            case  Boolean_AND:
            {
                boolOp = a2dBooleanWalkerHandler::Boolean_AND;
                break;
            }
            case  Boolean_EXOR:
            {
                boolOp = a2dBooleanWalkerHandler::Boolean_EXOR;
                break;
            }
            case  Boolean_A_SUB_B:
            {
                boolOp = a2dBooleanWalkerHandler::Boolean_A_SUB_B;
                break;
            }
            case  Boolean_B_SUB_A:
            {
                boolOp = a2dBooleanWalkerHandler::Boolean_B_SUB_A;
                break;
            }
            case  Boolean_CORRECTION:
            {
                booloper.SetCorrectionFactor( GetDrawHabitat()->GetBooleanEngineCorrectionFactor() / drawing->GetUnitsScale() );
                boolOp = a2dBooleanWalkerHandler::Boolean_CORRECTION;
                break;
            }
            case  Boolean_SMOOTHEN:
            {
                boolOp = a2dBooleanWalkerHandler::Boolean_SMOOTHEN;
                break;
            }
            case  Boolean_MAKERING:
            {
                booloper.SetCorrectionFactor( fabs( GetDrawHabitat()->GetBooleanEngineCorrectionFactor() / 2.0 / drawing->GetUnitsScale() ) );
                boolOp = a2dBooleanWalkerHandler::Boolean_MAKERING;
                break;
            }
            case  Boolean_Polygon2Surface:
            {
                boolOp = a2dBooleanWalkerHandler::Boolean_Polygon2Surface;
                break;
            }
            case  Boolean_Surface2Polygon:
            {
                boolOp = a2dBooleanWalkerHandler::Boolean_Surface2Polygon;
                break;
            }
            default:
                break;
        }
        booloper.SetOperation( boolOp );
        booloper.Start( m_parent );

#endif //wxART2D_USE_KBOOL
    }
    else if ( m_args.argValue.what == DetectSmall )
    {
        a2dWalker_DetectSmall walk;
        walk.SetGroupA( GetDrawHabitat()->GetGroupA() );
        walk.SetTarget( m_args.argValue.target );
        double smallest = double( GetDrawHabitat()->GetSmall() ) / drawing->GetUnitsScale();
        walk.SetSmall( smallest );
        walk.Start( m_parent );

        if ( m_args.argSet.fileNameOut )
        {
            wxString filename = m_args.argValue.fileNameOut;
#if wxUSE_STD_IOSTREAM
            a2dDocumentFileOutputStream store( filename.mb_str(), wxSTD ios_base::out | wxSTD ios_base::binary );
            if ( store.fail() || store.bad() )
            {
                a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for saving" ), filename.c_str() );
                return false;
            }
            store << walk.GetString();
#else
            wxFileOutputStream storeUnbuf( filename );
            if ( storeUnbuf.GetLastError() != wxSTREAM_NO_ERROR )
            {
                a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for saving" ), filename.c_str() );
                return false;
            }
            wxBufferedOutputStream store( storeUnbuf );
            store.Write( walk.GetString(), walk.GetString().Len() );
#endif
        }
        //wxMessageBox( walk.GetString(), _("Report"), wxICON_INFORMATION | wxOK );
    }
    else if ( m_args.argValue.what == RemoveRedundant )
    {
        a2dWalker_LayerCanvasObjects removeredundant;
        removeredundant.SetSmall( GetDrawHabitat()->GetSmall() / drawing->GetUnitsScale() );
        removeredundant.SetGroupA( GetDrawHabitat()->GetGroupA() );
        removeredundant.SetTarget( m_args.argValue.target );
        removeredundant.SetOperation( a2dWalker_LayerCanvasObjects::RemoveRedundant );
        removeredundant.Start( m_parent );
    }
    else
        return false;
    return true;
}

bool a2dCommand_GroupAB::Undo()
{
    return false;
}

//----------------------------------------------------------------------------
// a2dCommand_SetShowObject
//----------------------------------------------------------------------------

bool a2dCommand_SetShowObject::Do()
{
    a2dCanvasObject* g = NULL;
    if ( m_args.argSet.index )
    {
        g = GetCanvasCmp()->m_drawing->GetRootObject()->Find( _T( "" ), _T( "" ), m_args.argValue.index );
        if ( !g )
        {
            a2dGeneralGlobals->ReportErrorF( a2dError_NotSpecified, _T( "No Object with id %d in a2dCanvasCommandProcessor::SetShowObject()" ), m_args.argValue.index );
            return false;
        }
    }
    else if ( m_args.argSet.name )
    {
        g = GetCanvasCmp()->m_drawing->GetRootObject()->Find( m_args.argValue.name, _T( "" ), -1 );
        if ( !g )
        {
            a2dGeneralGlobals->ReportErrorF( a2dError_NotSpecified, _T( "No Object with id %d in a2dCanvasCommandProcessor::SetShowObject()" ), m_args.argValue.index );
            return false;
        }
    }
    else if ( m_args.argSet.selected )
        g = m_drawingPart->GetShowObject()->Find( wxT( "" ),  wxT( "" ), a2dCanvasOFlags::SELECTED );
    else if ( m_args.argSet.canvasobject )
        g = m_args.argValue.canvasobject;

    if ( g )
    {
        m_previous = m_drawingPart->GetShowObject();
        m_showobject = g;
        g->SetSelected( false );
        g->GetChildObjectList()->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED );
        m_drawingPart->SetShowObject( g );
    }
    //GetCanvasCmp()->SetLastXyEntry( 0, 0 );
    return true;
}

bool a2dCommand_SetShowObject::Undo()
{
    m_showobject = m_previous;
    m_previous = m_drawingPart->GetShowObject();
    m_showobject->SetSelected( false );
    m_showobject->GetChildObjectList()->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED );
    m_drawingPart->SetShowObject( m_showobject );
    return false;
}

//----------------------------------------------------------------------------
// a2dCommand_PushInto
//----------------------------------------------------------------------------

bool a2dCommand_PushInto::Do()
{
    if ( m_args.argSet.name )
        m_pushIn = m_drawingPart->GetShowObject()->Find( m_args.argValue.name );
    else if ( m_args.argSet.selected )
        m_pushIn = m_drawingPart->GetShowObject()->Find( wxT( "" ),  wxT( "" ), a2dCanvasOFlags::SELECTED );
    else if ( m_args.argSet.canvasobject )
        m_pushIn = m_args.argValue.canvasobject;

    if ( m_pushIn )
        m_pushIn = m_pushIn->PushInto( m_drawingPart->GetShowObject() );

    if ( m_pushIn )
    {
        m_previous = m_drawingPart->GetShowObject();
        //let drawer follow document if needed.
        //TODO when view switches its document when switching drawing
        //if ( doc->GetRootObject() != m_pushIn->GetRoot() )
        //    drawer->SetDocument( m_pushIn->GetCanvasDocument() );
        m_pushIn->SetSelected( false );
        m_pushIn->GetChildObjectList()->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED );
        m_drawingPart->PushIn( m_pushIn );
    }
    else
        ( void )wxMessageBox( _( "Does not contain a group, unable to push into" ), _( "push into" ), wxICON_INFORMATION | wxOK );

    return true;
}

bool a2dCommand_PushInto::Undo()
{
    if ( m_pushIn )
    {
        //let drawer follow document if needed.
        //TODO when view switches its document when switching drawing
        //if ( doc->GetRootObject() != m_previous->GetCanvasDocument()->GetRootObject() )
        //    drawer->SetDocument( m_previous->GetCanvasDocument() );
        m_pushIn = m_previous;
        m_previous = m_drawingPart->GetShowObject();
        m_previous->SetSelected( false );
        m_previous->GetChildObjectList()->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED );
        m_drawingPart->PopOut();
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// a2dCommand_SetCanvasProperty
//----------------------------------------------------------------------------

a2dCommand_SetCanvasProperty::a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdCanvasObject* id, a2dCanvasObject* value ):
    a2dCommand_SetProperty( object, id )
{
    a2dCanvasObjectPtrProperty* prop = new a2dCanvasObjectPtrProperty( id, value );

    m_property = prop;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in Clone ( )
    CurrentSmartPointerOwner = this;
#endif
}

a2dCommand_SetCanvasProperty::a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdMatrix* id,  const a2dAffineMatrix& value ):
    a2dCommand_SetProperty( object, id )
{
    a2dMatrixProperty* prop = new a2dMatrixProperty( id, value );

    m_property = prop;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in Clone ( )
    CurrentSmartPointerOwner = this;
#endif
}

a2dCommand_SetCanvasProperty::a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdPoint2D* id, const a2dPoint2D& point, int index, bool afterinversion ):
    a2dCommand_SetProperty( object, id )
{
    a2dPoint2DProperty* prop = new a2dPoint2DProperty( id, point, index, afterinversion );

    m_property = prop;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in Clone ( )
    CurrentSmartPointerOwner = this;
#endif
}

a2dCommand_SetCanvasProperty::a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdFill* id, const a2dFill& fill )
{
    a2dFillProperty* prop = new a2dFillProperty( id, fill );

    m_property = prop;
}

a2dCommand_SetCanvasProperty::a2dCommand_SetCanvasProperty( a2dObject* object, const a2dPropertyIdStroke* id, const a2dStroke& stroke )
{
    a2dStrokeProperty* prop = new a2dStrokeProperty( id, stroke );

    m_property = prop;
}

a2dCommand_SetCanvasProperty::~a2dCommand_SetCanvasProperty( void )
{
}

a2dCommand* a2dCommand_SetCanvasProperty::CloneAndBind( a2dObject* object )
{
    wxASSERT( !m_propRefObject );
    return new a2dCommand_SetCanvasProperty( object, m_property->Clone( a2dObject::clone_flat ) );
}

bool a2dCommand_SetCanvasProperty::Do( void )
{
    bool ret = a2dCommand_SetProperty::Do();


    return ret;
}

bool a2dCommand_SetCanvasProperty::Undo( void )
{
    bool ret = a2dCommand_SetProperty::Undo();

    return ret;
}

//----------------------------------------------------------------------------
// a2dCommand_ChangeCanvasObjectStyle
//----------------------------------------------------------------------------

const a2dCommandId a2dCommand_ChangeCanvasObjectStyle::sm_SetFillStyleCommand = a2dCommandId( wxT( "SetCanvasObjectFillStyleCommand" ) );
const a2dCommandId a2dCommand_ChangeCanvasObjectStyle::sm_SetStrokeStyleCommand = a2dCommandId( wxT( "SetCanvasObjectStrokeStyleCommand" ) );
const a2dCommandId a2dCommand_ChangeCanvasObjectStyle::sm_SetStyleCommand = a2dCommandId( wxT( "SetCanvasObjectStyleCommand" ) );

/*
*   a2dCommand_ChangeCanvasObjectStyle
*/
a2dCommand_ChangeCanvasObjectStyle::a2dCommand_ChangeCanvasObjectStyle( a2dObject* object, const a2dFill& fill ):
    a2dCommand( true, sm_SetFillStyleCommand, sm_SetStyleCommand )
{
    m_fill = fill;
    m_stroke = *a2dNullSTROKE;

    m_propObject = object;
}

a2dCommand_ChangeCanvasObjectStyle::a2dCommand_ChangeCanvasObjectStyle( a2dObject* object, const a2dStroke& stroke ):
    a2dCommand( true, sm_SetStrokeStyleCommand, sm_SetStyleCommand )
{
    m_fill = *a2dNullFILL;
    m_stroke = stroke;

    m_propObject = object;
}

a2dCommand_ChangeCanvasObjectStyle::a2dCommand_ChangeCanvasObjectStyle( a2dObject* object,
        const a2dFill& fill, const a2dStroke& stroke ):
    a2dCommand( true, sm_SetStyleCommand, sm_SetStyleCommand )
{
    m_fill = fill;
    m_stroke = stroke;

    m_propObject = object;
}


a2dCommand_ChangeCanvasObjectStyle::~a2dCommand_ChangeCanvasObjectStyle( void )
{
}

bool a2dCommand_ChangeCanvasObjectStyle::Do( void )
{
    if ( !m_fill.IsNoFill() )
    {
        a2dFill fillold = a2dCanvasObject::PROPID_Fill->GetPropertyValue( m_propObject );
        a2dCanvasObject::PROPID_Fill->SetPropertyToObject( m_propObject, m_fill );
        m_fill = fillold;
    }
    if ( !m_stroke.IsNoStroke() )
    {
        a2dStroke strokeold = a2dCanvasObject::PROPID_Stroke->GetPropertyValue( m_propObject );
        a2dCanvasObject::PROPID_Stroke->SetPropertyToObject( m_propObject, m_stroke );
        m_stroke = strokeold;
    }

    return true;
}

bool a2dCommand_ChangeCanvasObjectStyle::Undo( void )
{
    return a2dCommand_ChangeCanvasObjectStyle::Do();
}

// ----------------------------------------------------------------------------
// wxDoubleNumberEntryDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( wxDoubleNumberEntryDialog, wxDialog )
    EVT_BUTTON( wxID_OK, wxDoubleNumberEntryDialog::OnOK )
    EVT_BUTTON( wxID_CANCEL, wxDoubleNumberEntryDialog::OnCancel )
END_EVENT_TABLE()

IMPLEMENT_CLASS( wxDoubleNumberEntryDialog, wxDialog )

wxDoubleNumberEntryDialog::wxDoubleNumberEntryDialog( wxWindow* parent,
        const wxString& message,
        const wxString& prompt,
        const wxString& caption,
        double value,
        double min,
        double max,
        const wxPoint& pos )
    : wxDialog( parent, wxID_ANY, caption,
                pos, wxDefaultSize )
{
    m_value = value;
    m_max = max;
    m_min = min;

    wxBeginBusyCursor();

    wxBoxSizer* topsizer = new wxBoxSizer( wxVERTICAL );
#if wxUSE_STATTEXT
    // 1) text message
    topsizer->Add( CreateTextSizer( message ), 0, wxALL, 10 );
#endif

    // 2) prompt and text ctrl
    wxBoxSizer* inputsizer = new wxBoxSizer( wxHORIZONTAL );

#if wxUSE_STATTEXT
    // prompt if any
    if ( !prompt.empty() )
        inputsizer->Add( new wxStaticText( this, wxID_ANY, prompt ), 0, wxCENTER | wxLEFT, 10 );
#endif

    // spin ctrl
    wxString valStr;
    valStr.Printf( wxT( "%f" ), m_value );
    m_textctrl = new wxTextCtrl( this, wxID_ANY, valStr, wxDefaultPosition, wxSize( 140, wxDefaultCoord ) );
    inputsizer->Add( m_textctrl, 1, wxCENTER | wxLEFT | wxRIGHT, 10 );
    // add both
    topsizer->Add( inputsizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 5 );

    // 3) buttons if any
    wxSizer* buttonSizer = CreateSeparatedButtonSizer( wxOK | wxCANCEL );
    if ( buttonSizer )
    {
        topsizer->Add( buttonSizer, wxSizerFlags().Expand().DoubleBorder() );
    }

    SetSizer( topsizer );
    SetAutoLayout( true );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );

    Centre( wxBOTH );

    m_textctrl->SetSelection( -1, -1 );
    m_textctrl->SetFocus();

    wxEndBusyCursor();
}

void wxDoubleNumberEntryDialog::OnOK( wxCommandEvent& WXUNUSED( event ) )
{
    wxString tmp = m_textctrl->GetValue();

    if ( !tmp.ToDouble( &m_value ) )
        EndModal( wxID_CANCEL );
    if ( m_value < m_min || m_value > m_max )
    {
        // not a number or out of range
        m_value = -1;
        EndModal( wxID_CANCEL );
    }

    EndModal( wxID_OK );
}

void wxDoubleNumberEntryDialog::OnCancel( wxCommandEvent& WXUNUSED( event ) )
{
    EndModal( wxID_CANCEL );
}

// ----------------------------------------------------------------------------
// global functions
// ----------------------------------------------------------------------------

// wxGetTextFromUser is in utilscmn.cpp

double wxGetDoubleNumberFromUser( const wxString& msg,
                                  const wxString& prompt,
                                  const wxString& title,
                                  double value,
                                  double min,
                                  double max,
                                  wxWindow* parent,
                                  const wxPoint& pos )
{
    wxDoubleNumberEntryDialog dialog( parent, msg, prompt, title,
                                      value, min, max, pos );
    if ( dialog.ShowModal() == wxID_OK )
        return dialog.GetValue();

    return DBL_MAX;
}

bool a2dCommand_SetCursor::Do( void )
{
    double x = m_args.argValue.x;
    double y = m_args.argValue.y;

    //get current position
    m_xold = GetDrawHabitat()->GetLastXEntry();
    m_yold = GetDrawHabitat()->GetLastYEntry();

    //get current position
    if ( m_args.argValue.relative )
    {
        if ( m_args.argValue.polar )
        {
            double radius = y;
            double ang = x;

            x = m_xold + radius * cos( wxDegToRad( ang ) );
            y = m_yold + radius * sin( wxDegToRad( ang ) );
        }
        else
        {
            x += m_xold;
            y += m_yold;
        }
    }
    else
    {
        if ( m_args.argValue.polar )
        {
            double radius = y;
            double ang = x;

            x = radius * cos( wxDegToRad( ang ) );
            y = radius * sin( wxDegToRad( ang ) );
        }
    }


    int dx = GetCanvasCmp()->GetActiveDrawingPart()->GetDrawer2D()->WorldToDeviceX( x );
    int dy = GetCanvasCmp()->GetActiveDrawingPart()->GetDrawer2D()->WorldToDeviceY( y );
    GetCanvasCmp()->GetActiveDrawingPart()->GetDisplayWindow()->WarpPointer( dx, dy );
    GetDrawHabitat()->SetLastXyEntry( x, y );

    return true;
}

bool a2dCommand_SetCursor::Undo( void )
{
    int dx = GetCanvasCmp()->GetActiveDrawingPart()->GetDrawer2D()->WorldToDeviceX( m_xold );
    int dy = GetCanvasCmp()->GetActiveDrawingPart()->GetDrawer2D()->WorldToDeviceY( m_yold );
    GetCanvasCmp()->GetActiveDrawingPart()->GetDisplayWindow()->WarpPointer( dx, dy );
    return true;
}

// ----------------------------------------------------------------------------
// a2dCommand_AddObjects
// ----------------------------------------------------------------------------

a2dCommand_AddObjects::a2dCommand_AddObjects( a2dCanvasObject* parent,  a2dCanvasObjectList objects ):
    a2dCommand( true, a2dCommand_AddObjects::Id )
{
    m_objects = objects;
    m_parent = parent;
}

bool a2dCommand_AddObjects::Do()
{
    for( a2dCanvasObjectList::iterator iter = m_objects.begin(); iter != m_objects.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        m_parent->Append( obj );
    }
    
    return true;
}

bool a2dCommand_AddObjects::Undo()
{
    a2dCanvasObjectList* childs= m_parent->GetChildObjectList();

    a2dCanvasObjectList::iterator iter = childs->begin();
    while( iter != childs->end() )
    {
        a2dCanvasObject* obj = *iter;
        if ( m_objects.Find( obj ) )
            iter = childs->erase( iter );
        else
            ++iter;
    }
    m_parent->SetPending( true ); //redraw because object are removed without using pending mechanism
    return true;
}

bool a2dCommand_AddObjects::Redo()
{
    for( a2dCanvasObjectList::iterator iter = m_objects.begin(); iter != m_objects.end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        m_parent->Append( obj );
    }
    return true;
}
