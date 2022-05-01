/*! \file canvas/src/tools.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: tools.cpp,v 1.133 2009/06/03 17:38:13 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"

#include "wx/canvas/tools.h"

IMPLEMENT_CLASS( a2dBaseTool, a2dObject )
IMPLEMENT_CLASS( a2dToolContr, a2dObject )

//----------------------------------------------------------------------------
// a2dToolList
//----------------------------------------------------------------------------

a2dToolList::a2dToolList()
{
}

a2dToolList::~a2dToolList()
{
}

a2dToolList* a2dToolList::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    a2dToolList* a = new a2dToolList();

    for( a2dToolList::const_iterator iter = begin(); iter != end(); ++iter )
    {
        a2dToolList::value_type toolobj = *iter;

        if ( options & a2dObject::clone_childs )
        {
            a2dBaseTool* objnew = wxStaticCast( toolobj->Clone( options ), a2dBaseTool );
            a->push_back( objnew );
        }
        else
            a->push_back( toolobj );
    }

    return a;
}

//----------------------------------------------------------------------------
// a2dToolContr
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dToolContr, a2dObject )
    EVT_PAINT( a2dToolContr::OnPaint )
    EVT_IDLE( a2dToolContr::OnIdle )
    EVT_ENTER_WINDOW( a2dToolContr::OnEnter )
END_EVENT_TABLE()

a2dToolContr::a2dToolContr( a2dDrawingPart* drawingPart )
	:m_drawingPart( drawingPart )
{
    SetSnap( false );
    m_bussyPoping = false;

    if ( m_drawingPart )
    {
        if ( m_drawingPart->GetDrawing() && !m_drawingPart->GetDrawing()->GetCanvasCommandProcessor() )
            m_drawingPart->GetDrawing()->CreateCommandProcessor();
        m_drawingPart->SetCanvasToolContr( this );
        SetEvtHandlerEnabled( true );
    }
    else
        SetEvtHandlerEnabled( false );
}

a2dToolContr::~a2dToolContr()
{
}

a2dBaseTool* a2dToolContr::GetFirstTool() const
{
    return m_tools.size() ? m_tools.front() : NULL;
}

void a2dToolContr::SetDrawingPart( a2dDrawingPart* drawingPart )
{
    //prevent deletion by drawer
    a2dREFOBJECTPTR_KEEPALIVE;

    if ( m_drawingPart )
    {
        //disable the controller for the current drawer
        m_drawingPart->SetCanvasToolContr( NULL );
        SetEvtHandlerEnabled( false );
    }

    m_drawingPart = drawingPart;

    if ( m_drawingPart )
    {
        m_drawingPart->SetCanvasToolContr( this );
        SetEvtHandlerEnabled( true );
    }
    else
        SetEvtHandlerEnabled( false );
}

void a2dToolContr::ReStart()
{
}

void a2dToolContr::StopAllTools( bool abort )
{
    a2dSmrtPtr< a2dBaseTool > tool;
    a2dToolContr::PopTool( tool, abort );
    while ( tool )
    {
        tool = NULL;
        a2dToolContr::PopTool( tool, abort );
    }
}

void a2dToolContr::Disable()
{
    SetEvtHandlerEnabled( false );
    StopAllTools();
    m_drawingPart = 0;
}

void a2dToolContr::ActivateTop( bool active )
{
    // first all other tools, and last the first tool, so its m_corridor will be set in the end.
    for( a2dToolList::reverse_iterator iter = m_tools.rbegin(); iter != m_tools.rend(); ++iter )
    {
        a2dToolList::value_type toolobj = *iter;
        if ( *iter == m_tools.front() )
            toolobj->SetActive( active );
        else
            toolobj->SetActive( false );
    }
}

bool a2dToolContr::Activate( const wxString& tool, bool disableothers )
{
    bool found = false;

    for( a2dToolList::iterator iter = m_tools.begin(); iter != m_tools.end(); ++iter )
    {
        a2dToolList::value_type toolobj = *iter;
        if ( !tool.CmpNoCase( toolobj->GetClassInfo()->GetClassName() ) )
        {
            toolobj->SetActive( true );
            found = true;
        }
        else if ( disableothers )
            toolobj->SetActive( false );
    }
    return found;
}

bool a2dToolContr::EnableTool( const wxString& tool, bool disableothers )
{
    bool found = false;

    for( a2dToolList::iterator iter = m_tools.begin(); iter != m_tools.end(); ++iter )
    {
        a2dToolList::value_type toolobj = *iter;

        if ( !tool.CmpNoCase( toolobj->GetClassInfo()->GetClassName() ) )
        {
            toolobj->SetEvtHandlerEnabled( true );
            found = true;
        }
        else if ( disableothers )
            toolobj->SetEvtHandlerEnabled( false );
    }
    return found;
}

bool a2dToolContr::EnableTool( a2dBaseTool* tool, bool disableothers )
{
    bool found = false;
    for( a2dToolList::iterator iter = m_tools.begin(); iter != m_tools.end(); ++iter )
    {
        a2dToolList::value_type toolobj = *iter;
        if ( tool == toolobj )
        {
            toolobj->SetEvtHandlerEnabled( true );
            found = true;
        }
        else if ( disableothers )
            toolobj->SetEvtHandlerEnabled( false );
    }
    return found;
}

bool a2dToolContr::DisableTool( const wxString& tool )
{
    bool found = false;
    for( a2dToolList::iterator iter = m_tools.begin(); iter != m_tools.end(); ++iter )
    {
        a2dToolList::value_type toolobj = *iter;
        if ( !tool.CmpNoCase( toolobj->GetClassInfo()->GetClassName() ) )
        {
            toolobj->SetEvtHandlerEnabled( false );
            found = true;
        }
    }
    return found;
}

bool a2dToolContr::DisableTool( a2dBaseTool* tooldis )
{
    bool found = false;
    for( a2dToolList::iterator iter = m_tools.begin(); iter != m_tools.end(); ++iter )
    {
        a2dToolList::value_type toolobj = *iter;
        if ( toolobj == tooldis )
        {
            tooldis->SetEvtHandlerEnabled( false );
            found = true;
        }
    }
    return found;
}

a2dBaseTool* a2dToolContr::SearchTool( const wxString& toolsearch )
{
    for( a2dToolList::iterator iter = m_tools.begin(); iter != m_tools.end(); ++iter )
    {
        a2dToolList::value_type toolobj = *iter;
        if ( !toolsearch.CmpNoCase( toolobj->GetClassInfo()->GetClassName() ) )
            return toolobj;
    }
    return ( a2dBaseTool* ) 0;
}

void a2dToolContr::SetStateString( const wxString& message, size_t field )
{
}

bool a2dToolContr::ProcessEvent( wxEvent& event )
{
    if ( !m_drawingPart )
    {
        return false;
    }

    a2dObject* active = this;

    //the next is almost a copy of a2dObject::ProcessEvent
    //The base class could not be used since in case of an event skip
    //in a derived a2dToolContr class, the event would go directly to the a2dCanvas,
    //without passing it to the tools first.

    //So the order here is:
    //first test for static event tables in a derived controller class if event is skipped or !handled in there,
    //Process the events down the chain of tools (each tool that skips the event means the next one is called)
    //If still not processed, the event is redirected to the rest of the wxEvtHandler in the a2dCanvas window.
    //This is in general the a2dCanvas window itself.
    //In case a tool has stopped itself, the tool is poped of the stack, and if the event was not handled yet
    //it is sent to the tool that is now first on the stack.

    // An event handler can be enabled or disabled
    if ( active->GetEvtHandlerEnabled() )
    {
        if ( !m_bussyPoping && GetFirstTool() && GetFirstTool()->GetStopTool() )
        {
            a2dSmrtPtr<a2dBaseTool> poped;
            PopTool( poped );
        }

        if ( a2dObject::ProcessEvent( event ) )
            return true;

        return ToolsProcessEvent( event );
    }

    return false;
}

bool a2dToolContr::ToolsProcessEvent( wxEvent& event )
{
    bool handled = false;

    if ( GetFirstTool() )
    {
        //process event return false if event was skipped or not handled.
        //the firstool redirects to the next tool on the stack etc.
        //if not handled in any tool the next return false

        size_t nrTools = m_tools.size();

        // If a new tool was pushed, give the new tool a chance to process the event.
        // This is a typical situation when the tool pushed a new tool while handling the event.
        // The same event directly goes to that tool.
        // This pushed tool can do the same itself, so repeat until no more pushed tools.

        a2dToolList::iterator iter = m_tools.begin();
        while( !handled && iter != m_tools.end() )
        {
            a2dBaseTool* first = ( *iter ).Get();
            iter++; // go to the next in case of a pop, we will be save.
            handled = first->ProcessEvent( event );
            if ( nrTools < m_tools.size() ) //there was a push
            {
                iter = m_tools.begin();   //start all over
                nrTools = m_tools.size();
            }
            //was there a pop request?
            if ( GetFirstTool()->GetStopTool() )
            {
                if ( !m_bussyPoping )
                {
                    a2dSmrtPtr<a2dBaseTool> poped;
                    PopTool( poped );
					iter = m_tools.begin();
                }
            }
            //ready to go to the next tool where the iterator is on.
        }

        return handled;
    }
    return false;
}

bool a2dToolContr::PushTool( a2dBaseTool* handler )
{
    a2dBaseToolPtr handlerres = handler;
    //first allow the current first tool to react on a the pushing of the new tool.
    if ( GetFirstTool() )
    {
        a2dComEvent event( GetFirstTool(), handler,  a2dBaseTool::sig_toolBeforePush );
        if ( GetFirstTool()->ProcessEvent( event ) && event.GetEventComIdReturn() == a2dBaseTool::sig_toolDoPopBeforePush )
        {
            // if a push attempt must result in the current first tool to be stopped,
            // we pop it directly.
            a2dSmrtPtr<a2dBaseTool> poped;
            if ( GetFirstTool()->GetStopTool() )
                PopTool( poped );
        }
    }

    if ( handler->StartTool( GetFirstTool() ) )
    {
        ActivateTop( false );
        m_tools.push_front( handler );
        a2dComEvent event( GetFirstTool(), a2dBaseTool::sig_toolPushed );
        GetFirstTool()->ProcessEvent( event );
        ActivateTop( true );
        return true;
    }
    return false;
}

bool a2dToolContr::PopTool( a2dSmrtPtr<a2dBaseTool>& poped, bool abort )
{
    m_bussyPoping = true;

    if ( GetFirstTool() && ( GetFirstTool()->AllowPop() || abort ) )
    {
        GetFirstTool()->StopTool( abort );
        poped = GetFirstTool();
        if ( poped )
        {
            //release the object from the tool list, but argument
            //poped will keep a reference, so no delete, this is good, since we want to sent an event to the new
            // first tool, with as parameter the the poped tool.
            m_tools.ReleaseObject( poped );

            // inform new first tool, e.g. to take style of the poped tool.
            if ( GetFirstTool() )
            {
                a2dComEvent event( GetFirstTool(), poped, a2dBaseTool::sig_toolPoped );
                GetFirstTool()->ProcessEvent( event );
            }
            poped->SetActive( false );
        }

        m_bussyPoping = false;
        return true;
    }
    m_bussyPoping = false;
    poped  = NULL;
    return false;
}

void a2dToolContr::AppendTool( a2dBaseTool* handler )
{
    m_tools.push_back( handler );
}

void a2dToolContr::OnEnter( wxMouseEvent& event )
{
    event.Skip( true );
}

void a2dToolContr::OnPaint( wxPaintEvent& event )
{
    if ( GetFirstTool() )
    {
        GetFirstTool()->ProcessEvent( event );
    }

    //no skipping here (onPaint event is/should be handled)
    event.Skip( false );
}

void a2dToolContr::OnIdle( wxIdleEvent& event )
{
    if ( GetFirstTool() )
    {
        GetFirstTool()->ProcessEvent( event );
    }
}

bool a2dToolContr::StartEditingObject( a2dCanvasObject* objectToEdit )
{
    return false;
}

bool a2dToolContr::StartEditingObject( a2dCanvasObject* objectToEdit, a2dIterC& ic )
{
    return false;
}

bool a2dToolContr::TriggerReStartEdit( wxUint16 editmode )
{
    return false;
}

void a2dToolContr::Render()
{
    for( a2dToolList::reverse_iterator iter = m_tools.rbegin(); iter != m_tools.rend(); ++iter )
        ( *iter )->Render();
}

bool a2dToolContr::SetCorridor( const a2dCorridor& corridor )
{
    wxASSERT_MSG( GetFirstTool(), wxT( "No tool set in controller" ) );
    GetFirstTool()->SetCorridor( corridor );
    return true;
}

void a2dToolContr::SetSnap( bool snap )
{
    m_snap = snap;
    a2dRestrictionEngine* restrict = a2dCanvasGlobals->GetHabitat()->GetRestrictionEngine();
    if ( m_drawingPart && m_drawingPart->GetDrawing() )
        restrict = m_drawingPart->GetDrawing()->GetHabitat()->GetRestrictionEngine();

    if( restrict )
        restrict->SetSnap( m_snap );
}


/********************************************************************************
a2dBaseTool
********************************************************************************/

a2dPropertyIdBool* a2dBaseTool::PROPID_Oneshot = NULL;
a2dPropertyIdBool* a2dBaseTool::PROPID_Stop = NULL;
a2dPropertyIdFill* a2dBaseTool::PROPID_Fill = NULL;
a2dPropertyIdStroke* a2dBaseTool::PROPID_Stroke = NULL;
a2dPropertyIdUint16* a2dBaseTool::PROPID_Layer = NULL;

INITIALIZE_PROPERTIES( a2dBaseTool, a2dObject )
{
    A2D_PROPID_M( a2dPropertyIdBool, a2dBaseTool, Oneshot, false, m_oneshot );
    A2D_PROPID_M( a2dPropertyIdBool, a2dBaseTool, Stop, false, m_stop );
    A2D_PROPID_M( a2dPropertyIdUint16, a2dBaseTool, Layer, 0, m_layer );

    PROPID_Fill = new a2dPropertyIdFill( wxT( "Fill" ),
                                         a2dPropertyId::flag_transfer | a2dPropertyId::flag_clonedeep,
                                         *a2dNullFILL,
                                         static_cast < a2dPropertyIdFill::ConstGet >( &a2dBaseTool::GetFill ),
                                         static_cast < a2dPropertyIdFill::ConstSet >( &a2dBaseTool::SetFill ) );
    AddPropertyId( PROPID_Fill );

    PROPID_Stroke = new a2dPropertyIdStroke( wxT( "Stroke" ),
            a2dPropertyId::flag_transfer | a2dPropertyId::flag_clonedeep,
            *a2dNullSTROKE,
            static_cast < a2dPropertyIdStroke::ConstGet >( &a2dBaseTool::GetStroke ),
            static_cast < a2dPropertyIdStroke::ConstSet >( &a2dBaseTool::SetStroke ) );
    AddPropertyId( PROPID_Stroke );
    return true;
}

const a2dSignal a2dBaseTool::sig_toolPushed = wxNewId();
const a2dSignal a2dBaseTool::sig_toolPoped = wxNewId();
const a2dSignal a2dBaseTool::sig_toolBeforePush = wxNewId();
const a2dSignal a2dBaseTool::sig_toolDoPopBeforePush = wxNewId();
const a2dSignal a2dBaseTool::sig_toolComEvent = wxNewId();

DEFINE_MENU_ITEMID( CmdMenu_NoMenu, wxTRANSLATE("No menu"), wxTRANSLATE("No Menu") )

BEGIN_EVENT_TABLE( a2dBaseTool, a2dObject )
    EVT_MOUSE_EVENTS( a2dBaseTool::OnMouseEvent )
    EVT_CHAR( a2dBaseTool::OnChar )
    EVT_KEY_DOWN( a2dBaseTool::OnKeyDown )
    EVT_KEY_UP( a2dBaseTool::OnKeyUp )
    EVT_COM_EVENT( a2dBaseTool::OnComEvent )
    EVT_DO( a2dBaseTool::OnDoEvent )
    EVT_UNDO( a2dBaseTool::OnUndoEvent )
    EVT_REDO( a2dBaseTool::OnRedoEvent )
END_EVENT_TABLE()


a2dBaseTool::a2dBaseTool( a2dToolContr* controller, const a2dMenuIdItem& initiatingMenuId )
    : m_initiatingMenuId( &initiatingMenuId )
{
    m_mode = 0;

    m_anotate = true;

    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  );

    m_annotateFont = wxFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );

    // drawing wifth this stroke and fill is based on layers style in document
    m_fill = *a2dNullFILL;
    m_stroke = *a2dNullSTROKE;
    m_contourwidth = 0;

    m_layer = wxLAYER_DEFAULT;
    m_controller = controller;
    m_busy = false;
    m_halted = false;
    m_oneshot = false;
    m_active = true;
    m_stop = false;
    m_bussyStopping = 0;
    m_pending = false;
    m_isEditTool = true;
    m_canvas_mouseevents_restore = GetDrawingPart()->GetMouseEvents();

    m_mousemenu = NULL;

    m_corridor = a2dCorridor( *GetDrawingPart() );
    if ( !m_corridor.empty() )
    {
        m_parentobject = m_corridor.back();
        wxASSERT_MSG( m_parentobject->GetRoot() != 0 , wxT( "parentobject of tools has no drawing root set" ) );
    }
    else
        m_parentobject = GetDrawingPart()->GetShowObject();

    // the default event handler is just this object
    m_eventHandler = NULL;

    m_ignorePendingObjects = false;
    m_deselectAtStart = true;
}

a2dBaseTool::a2dBaseTool( const a2dBaseTool& other, CloneOptions options, a2dRefMap* refs )
    : a2dObject( other, options, refs )
{
    m_parentobject = other.m_parentobject;

    m_initiatingMenuId = other.m_initiatingMenuId;
    m_mode = other.m_mode;

    m_anotate = other.m_anotate;

    m_toolcursor = other.m_toolcursor;
    m_toolBusyCursor = other.m_toolBusyCursor;

    m_annotateFont = other.m_annotateFont;

    // drawing wifth this stroke and fill is based on layers style in document
    m_fill = other.m_fill;
    m_stroke = other.m_stroke;
    m_contourwidth = 0;

    m_layer = other.m_layer;
    m_controller = other.m_controller;
    m_busy = false;
    m_halted = false;
    m_oneshot = other.m_oneshot;
    m_active = true;
    m_stop = false;
    m_bussyStopping = 0;
    m_pending = false;
    m_isEditTool = other.m_isEditTool;

    m_canvas_mouseevents_restore = GetDrawingPart()->GetMouseEvents();

    m_mousemenu = NULL; //other.m_mousemenu;

    m_corridor = other.m_corridor;
    if ( !m_corridor.empty() )
        m_parentobject = m_corridor.back();

    // the default event handler is just this object
    m_eventHandler = other.m_eventHandler;

    m_ignorePendingObjects = other.m_ignorePendingObjects;
    m_deselectAtStart = other.m_deselectAtStart;
}

a2dBaseTool::~a2dBaseTool()
{
    if ( GetDrawingPart() )
        GetDrawingPart()->SetMouseEvents( m_canvas_mouseevents_restore );

    if( m_mousemenu )
        delete m_mousemenu;
}

wxString a2dBaseTool::GetToolString() const
{
	return GetClassInfo()->GetClassName();
}

void a2dBaseTool::ResetContext()
{
    m_corridor = a2dCorridor( *GetDrawingPart() );
    if ( !m_corridor.empty() )
        m_parentobject = m_corridor.back();
    else
        m_parentobject = GetDrawingPart()->GetShowObject();
}

void a2dBaseTool::SetIgnorePendingObjects( bool onoff )
{
    m_ignorePendingObjects = onoff;
    GetDrawing()->SetIgnorePendingObjects( onoff );
}

void a2dBaseTool::SetMousePopupMenu( wxMenu* mousemenu )
{
    if ( m_mousemenu )
        delete m_mousemenu;
    m_mousemenu = mousemenu;
}

bool a2dBaseTool::StartTool( a2dBaseTool* WXUNUSED( currenttool ) )
{
    m_stop = false;
    GetDrawingPart()->PushCursor( m_toolcursor );
    return true;
}

void a2dBaseTool::SetActive( bool active )
{
    m_active = active;
    //reactivate this state for document.
    if ( GetDrawing() )
        GetDrawing()->SetIgnorePendingObjects( m_ignorePendingObjects );

	if ( GetDrawingPart() )
	{
		if ( active )
		{
			GetDrawingPart()->SetCorridorPath( m_corridor );
		}
		else
		{
			GetDrawingPart()->ClearCorridorPath();
		}
	}
}

bool a2dBaseTool::ProcessEvent( wxEvent& event )
{
    if ( !GetEvtHandlerEnabled() )
        return false;

    if ( m_eventHandler )
    {
        bool processed = m_eventHandler->ProcessEvent( event );
        if ( processed )
            return processed;
    }
    return a2dObject::ProcessEvent( event );
}

void a2dBaseTool::OnComEvent( a2dComEvent& event )
{
    if ( GetDrawingPart() && event.GetId() == a2dDrawingPart::sig_changedShowObject )
    {
        if ( event.GetEventObject() == GetDrawingPart() )
        {
            // an existing corridor on a view will become invalid when the ShowObject changes.
            // Most tools can handle a change in corridor, for those there is no need to remove them after a change
            // in ShowObject.
            a2dCanvasObject* newtop = wxStaticCast( event.GetProperty()->GetRefObject(), a2dCanvasObject );
            if ( newtop )
            {
                a2dCorridor corridor;
                corridor.Push( newtop );
                SetCorridor( corridor );
            }
            else
                StopTool();
        }
        event.Skip();
    }
    else if ( m_active && GetEvtHandlerEnabled() && GetDrawingPart() )
    {
        event.Skip();
    }
    else
        event.Skip();
}

void a2dBaseTool::OnDoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}

void a2dBaseTool::OnUndoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}

void a2dBaseTool::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    OnUndoEvent( event );
    event.Skip();
}

bool a2dBaseTool::EnterBusyMode()
{
    wxASSERT( !m_busy );
    if ( m_isEditTool && !GetDrawingPart()->GetDrawing()->GetMayEdit() )
    {
        wxLogWarning( _T( "You may not edit this drawing" ) );              
        return false;   
    }

    m_busy = true;
    m_pending = true;
    OpenCommandGroup( false );
    GetDrawingPart()->PushCursor( m_toolBusyCursor );
    return true;
}

bool a2dBaseTool::EnterBusyModeNoGroup()
{
    wxASSERT( !m_busy );
    if ( m_isEditTool && !GetDrawingPart()->GetDrawing()->GetMayEdit() )
    {
        wxLogWarning( _T( "You may not edit this drawing" ) );              
        return false;   
    }

    m_busy = true;
    m_pending = true;
    GetDrawingPart()->PushCursor( m_toolBusyCursor );
    return true;
}

void a2dBaseTool::FinishBusyMode( bool closeCommandGroup )
{
    wxASSERT( m_busy );
    m_busy = false;
    m_pending = true;
    if ( closeCommandGroup )
        CloseCommandGroup();
    if( GetDrawingPart() )
    {
        GetDrawingPart()->PopCursor();
    }
    if( m_oneshot && !m_halted )
        StopTool();
}

void a2dBaseTool::AbortBusyMode()
{
    wxASSERT( m_busy );
    m_busy = false;
    m_pending = true;
    CloseCommandGroup();
    if( GetDrawingPart() )
    {
        GetDrawingPart()->PopCursor();
        if( a2dCanvasCommandProcessor* cmdh = GetCanvasCommandProcessor() )
        {
            cmdh->Undo(); //commands done with in the tool are in a group, last command (e.g. object added by the tool) is undone in that group.
            a2dCommand *current = cmdh->GetCurrentCommand();
            if( current ) 
            {
                a2dCommandGroup *curGroup = cmdh->GetCurrentGroup();
                curGroup->ClearAfterCurrentCommand( current );
            }
            else // no current command -> all commands have been undone
            {
                cmdh->ClearCommands(); 
            }
        }
    }
    if ( m_oneshot )
        StopTool();
}

void a2dBaseTool::StopTool( bool abort )
{
    m_bussyStopping++;
    if ( !m_stop  )
    {
        DoStopTool( abort );
        if ( GetDrawingPart() )
           GetDrawingPart()->PopCursor();
        m_stop = true;
    }
    m_bussyStopping--;
}

void a2dBaseTool::DoStopTool( bool abort )
{
    if ( m_busy )
    {
        if ( abort )
            AbortBusyMode();
        else
            FinishBusyMode();
    }
}

bool a2dBaseTool::GetStopTool()
{
    return m_stop && m_bussyStopping == 0;
}

void a2dBaseTool::SetFill( const a2dFill& fill )
{
    m_fill = fill;
};

void a2dBaseTool::SetStroke( const a2dStroke& stroke )
{
    m_stroke = stroke;
};

void a2dBaseTool::SetContourWidth( double width )
{
    m_contourwidth = width;
}

void a2dBaseTool::SetLayer( wxUint16 layer )
{
    m_layer = layer;
}

void a2dBaseTool::OnMouseEvent( wxMouseEvent& event )
{
    event.Skip();
}

void a2dBaseTool::OnKeyDown( wxKeyEvent& event )
{
    event.Skip();
}

void a2dBaseTool::OnKeyUp( wxKeyEvent& event )
{
    event.Skip();
}

void a2dBaseTool::OnChar( wxKeyEvent& event )
{
    event.Skip();
}

void a2dBaseTool::OpenCommandGroup( bool restart )
{
    if ( restart )
    {
        if ( !m_commandgroup )
        {
            wxString name = GetCommandGroupName();

            if( restart )
                name += _( " (continued)" );

            OpenCommandGroupNamed( name );
        }
    }
    else
    {
        if ( !m_commandgroup )
        {
            wxString name = GetCommandGroupName();
            OpenCommandGroupNamed( name );
        }
    }
}

void a2dBaseTool::OpenCommandGroupNamed( const wxString& name )
{
    wxASSERT_MSG( !m_commandgroup,  wxT( "Unclosed command group" ) );

    m_commandgroup = GetCanvasCommandProcessor()->CommandGroupBegin( name );
}

void a2dBaseTool::CloseCommandGroup()
{
    if ( m_commandgroup && this == m_controller->GetFirstTool() )
    {
        if( GetDrawingPart() )
        {
            GetCanvasCommandProcessor()->CommandGroupEnd( m_commandgroup );
        }
        m_commandgroup = 0;
    }
}

wxString a2dBaseTool::GetCommandGroupName()
{
    return GetClassInfo()->GetClassName();
}

void a2dBaseTool::SetCorridor( const a2dCorridor& corridor )
{
    m_corridor = corridor;
    if ( m_corridor.empty() )
    {
        m_corridor.Push( GetDrawingPart()->GetShowObject() );
        m_parentobject = GetDrawingPart()->GetShowObject();
    }
    else
    {
        m_parentobject = m_corridor.back();
    }
    wxASSERT_MSG( m_parentobject->GetRoot() != 0 , wxT( "parentobject of tools has no drawing root set" ) );
    GetDrawingPart()->SetCorridorPath( corridor );
}

void a2dBaseTool::AddEditobject( a2dCanvasObject* object )
{
    object->SetAlgoSkip( true );
    a2dCanvasObject::PROPID_ToolObject->SetPropertyToObject( object, this );
    a2dCanvasObject::PROPID_ViewSpecific->SetPropertyToObject( object, GetDrawingPart() );
    wxASSERT_MSG( m_parentobject->GetRoot() != 0 , wxT( "parentobject of tools has no drawing root set" ) );
    m_parentobject->Append( object );
    object->SetSnapTo( false );
}

void a2dBaseTool::RemoveEditobject( a2dCanvasObject* object )
{
    // Usually shortly added, so scanning backwards is usually faster
    m_parentobject->GetChildObjectList()->Release( object, true, false, true, a2dCanvasObject::PROPID_ToolObject );
}

void a2dBaseTool::AddDecorationObject( a2dCanvasObject* object )
{
    object->SetAlgoSkip( true );
    //a2dCanvasObject::PROPID_ToolObject->SetPropertyToObject( obj, this );
    a2dCanvasObject::PROPID_ToolDecoration->SetPropertyToObject( object, true );
    a2dCanvasObject::PROPID_ViewSpecific->SetPropertyToObject( object, GetDrawingPart() );

    object->SetHitFlags( a2dCANOBJ_EVENT_NON );
    wxASSERT_MSG( m_parentobject->GetRoot() != 0 , wxT( "parentobject of tools has no drawing root set" ) );
    m_parentobject->Append( object );
    object->SetSnapTo( false );
}

void a2dBaseTool::RemoveAllDecorations()
{
    a2dCanvasObjectList::iterator iter = m_parentobject->GetChildObjectList()->begin();
    while( iter !=  m_parentobject->GetChildObjectList()->end() )
    {
        a2dCanvasObject* obj = *iter;
        iter++;
        if( obj->GetAlgoSkip() && a2dCanvasObject::PROPID_ToolDecoration->GetPropertyValue( obj ) )
            m_parentobject->ReleaseChild( obj );
    }

    //first update the document, in order to get the right boundingboxes again ( also of the tool object )
    GetDrawing()->AddPendingUpdatesOldNew();
}

void a2dBaseTool::RemoveDecorationObject( a2dCanvasObject* obj )
{
    a2dCanvasObjectList::iterator iter = m_parentobject->GetChildObjectList()->begin();
    while( iter !=  m_parentobject->GetChildObjectList()->end() )
    {
        a2dCanvasObject* obj = *iter;
        iter++;
        if( obj->GetAlgoSkip() && a2dCanvasObject::PROPID_ToolDecoration->GetPropertyValue( obj ) )
            m_parentobject->ReleaseChild( obj );
    }
    /*
    a2dCanvasObjectList* childs =  m_parentobject->GetChildObjectList();
    a2dCanvasObjectList::iterator iter = childs->begin();
    while( iter != childs->end() )
    {
        a2dCanvasObjectList::value_type iobj = *iter;
        if ( ( iobj == obj || obj == NULL ) &&  iobj->GetAlgoSkip() && a2dCanvasObject::PROPID_ToolObject->GetPropertyValue( iobj )  )
        {
            GetDrawingPart()->AddPendingUpdateArea( iobj, 1 );
            iter = childs->erase( iter );
        }
        else
            iter++;
    }
    */
}

a2dDrawer2D* a2dBaseTool::GetDrawer2D()
{
    return m_controller->GetDrawingPart()->GetDrawer2D();
}

wxWindow* a2dBaseTool::GetDisplayWindow()
{
    return m_controller->GetDrawingPart()->GetDisplayWindow();
}

a2dDrawing* a2dBaseTool::GetDrawing()
{
    if ( !GetDrawingPart() )
        return NULL;
    return GetDrawingPart()->GetDrawing();
}

a2dCanvasCommandProcessor* a2dBaseTool::GetCanvasCommandProcessor()
{
    if ( !GetDrawingPart() || !GetDrawingPart()->GetShowObject() )
        return NULL;
    return GetDrawingPart()->GetDrawing()->GetCanvasCommandProcessor();
}

double a2dBaseTool::GetHitMargin()
{
    if ( a2dPin::GetWorldBased() )
        return GetDrawing()->GetHabitat()->GetPinSize()/2.0;
    else
        return GetDrawer2D()->DeviceToWorldXRel( GetDrawing()->GetHabitat()->GetPinSize()/2.0 );
}

//-----------------------------------------------------------
// a2dToolEvtHandler
//-----------------------------------------------------------

BEGIN_EVENT_TABLE( a2dToolEvtHandler, a2dObject )
    EVT_COM_EVENT( a2dToolEvtHandler::OnComEvent )
END_EVENT_TABLE()

a2dToolEvtHandler::a2dToolEvtHandler()
{
}

void a2dToolEvtHandler::OnComEvent( a2dComEvent& event )
{
    event.Skip();
}

bool a2dToolEvtHandler::ProcessEvent( wxEvent& event )
{
    if ( a2dObject::ProcessEvent( event ) )
        return true;
    return false;
}

