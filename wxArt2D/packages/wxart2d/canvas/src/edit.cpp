/*! \file editor/src/edit.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: edit.cpp,v 1.173 2009/07/04 10:26:39 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"

#include "wx/canvas/edit.h"

#include <wx/wfstream.h>
#include <math.h>

IMPLEMENT_CLASS( a2dRecursiveEditTool, a2dObjectEditTool )

const a2dCommandId a2dRecursiveEditTool::COMID_PushTool_RecursiveEdit( wxT( "PushTool_RecursiveEdit" ) );
const a2dCommandId a2dMultiEditTool::COMID_PushTool_MultiEdit( wxT( "PushTool_MultiEdit" ) );


BEGIN_EVENT_TABLE( a2dRecursiveEditTool, a2dObjectEditTool )
    EVT_COM_EVENT( a2dRecursiveEditTool::OnComEvent )
    EVT_MOUSE_EVENTS( a2dRecursiveEditTool::OnMouseEvent )
    EVT_IDLE( a2dRecursiveEditTool::OnIdle )
    EVT_CHAR( a2dRecursiveEditTool::OnChar )
    EVT_UNDO( a2dRecursiveEditTool::OnUndoEvent )
    EVT_REDO( a2dRecursiveEditTool::OnRedoEvent )
    EVT_KEY_UP( a2dRecursiveEditTool::OnKeyUp )
END_EVENT_TABLE()

a2dRecursiveEditTool::a2dRecursiveEditTool( a2dStToolContr* controller, a2dIterC& ic, int editmode, bool SingleClickToEnd )
    : a2dObjectEditTool( controller, ic, editmode, SingleClickToEnd )
{
    m_oneshot = false;
}

a2dRecursiveEditTool::a2dRecursiveEditTool( a2dStToolContr* controller, int editmode, bool SingleClickToEnd )
    : a2dObjectEditTool( controller, editmode, SingleClickToEnd )
{
    m_oneshot = false;
}

a2dRecursiveEditTool::~a2dRecursiveEditTool()
{
}

void a2dRecursiveEditTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active || m_halted )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw, yw;
    MouseToToolWorld( m_x, m_y, xw, yw );
    m_xwprev = xw;
    m_ywprev = yw;

    if ( event.Dragging() )
    {

    }
    else if ( event.Moving() && !GetBusy() )
    {
        a2dCanvasObject* hit = GetDrawingPart()->IsHitWorld( xw, yw );
        if ( hit && hit->GetDraggable() )
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );
        else
            GetDrawingPart()->SetCursor( m_toolcursor );
    }

    if ( event.LeftDClick() )
    {
        if ( GetBusy() )
            FinishBusyMode();
        else
            StopTool();
    }
    else if ( event.LeftDown() && !GetBusy() )
    {
        // if there is a hit on a a2dCanvasObject, the editing starts on that object
        a2dIterC ic( GetDrawingPart() );
        a2dIterCU cu( ic, m_corridor.GetInverseTransform() );
        a2dHitEvent hitevent = a2dHitEvent( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
        m_original = m_parentobject->IsHitWorld( ic, hitevent );

        if ( !m_original.Get() )
            return; //no hit

        if ( !EnterBusyMode() )
            return; //not editable object

        //RedirectToEditObject( event );
    }
    else if ( event.LeftDown() && GetBusy() )
    {
        //maybe go to another object for editing, even subediting might
        //have bin initiated in the a2dCanvasObject that is being edited.

        //goto the edit object itself, maybe for sub-editing.
        //Halted if sub-editing ( another tool is pushed on the tool stack )
        if ( RedirectToEditObject( event ) || m_stop || m_halted )
            return;

        //in case of not having a oneshot action, we can go to another object
        //while editing this one.
        if ( !m_oneshot )
        {
            a2dCanvasObject* neweditobject = NULL;
            a2dIterC ic( GetDrawingPart() );
            a2dIterCU cu( ic, m_corridor.GetInverseTransform() );
            a2dHitEvent hitevent = a2dHitEvent( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
            neweditobject = m_parentobject->IsHitWorld( ic, hitevent );

            if ( neweditobject == m_original || neweditobject == m_canvasobject )
                //nothing new, stay with the object until a double click.
                return;

            if ( !neweditobject )
            {
                if ( m_SingleClickToEnd )
                {
                    FinishBusyMode();

                    a2dToolList::const_iterator iter = m_controller->GetToolList().begin();
                    iter++;

                    if ( iter != m_controller->GetToolList().end() )
                    {
                        //! test for subediting ( next tool is also editing tool )
                        a2dRecursiveEditTool* c = wxDynamicCast( ( *iter ).Get(), a2dRecursiveEditTool );
                        if ( c )
                            StopTool();
                    }
                }

                //nothing new, stay with the object until a double click.
                return;
            }

            if ( !neweditobject->GetEditable() )
                return;

            //the other object is hit, and oke for editing
            //Stop the current one, and go to the new object
            FinishBusyMode();

            m_original = neweditobject;

            if ( !EnterBusyMode() )
                return;

            RedirectToEditObject( event );
        }
        else
        {
            a2dCanvasObject* neweditobject = NULL;
            a2dIterC ic( GetDrawingPart() );
            a2dIterCU cu( ic, m_corridor.GetInverseTransform() );
            a2dHitEvent hitevent = a2dHitEvent( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
            neweditobject = m_parentobject->IsHitWorld( ic, hitevent );

            if ( !neweditobject && !m_SingleClickToEnd || neweditobject == m_original || neweditobject == m_canvasobject )
                //nothing new so continue with current editobject
                return;

            StopTool();
        }
    }
    else if ( event.LeftUp() && GetBusy() )
    {
        if ( !RedirectToEditObject( event ) )
            event.Skip();
    }
    else if ( GetBusy() )
    {
        if ( !RedirectToEditObject( event ) )
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}


void a2dRecursiveEditTool::OnComEvent( a2dComEvent& event )
{
    if ( GetBusy() )
    {
        if ( event.GetId() == a2dStTool::sig_toolBeforePush )
        {
            // If tool is busy, it will enter halted state
            if ( GetBusy() && !m_halted )
            {
                m_halted = true;
                CleanupToolObjects();
            }
            m_pending = true;
        }
        else if ( event.GetId() == a2dDrawingPart::sig_changedShowObject )
        {
            a2dCanvasObject* newtop = wxStaticCast( event.GetProperty()->GetRefObject(), a2dCanvasObject );
            if ( newtop )
            {
                AbortBusyMode();
                a2dCorridor corridor;
                corridor.push_back( newtop );
                SetCorridor( corridor );
            }
            else
                StopTool();
        }
        else
            event.Skip();
    }

    event.Skip();
}


//! used to add object to a a2dCanvasDocument in the current parent
/*!

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_AddObjectToGroup: public a2dCommand
{

public:
    //! used to add object to a a2dCanvasDocument in the current parent
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_AddObjectToGroup( a2dMultiEditTool* tool = NULL, a2dCanvasObject* object = NULL );

    ~a2dCommand_AddObjectToGroup( void );

    bool Do();
    bool Undo();

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

    a2dCanvasObject* GetCanvasObject() { return m_canvasobject; }

protected:
    a2dCanvasObjectPtr m_canvasobject;
    a2dMultiEditTool* m_tool;
};

//! used to release object from a a2dCanvasDocument in the current parent
/*!

    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ReleaseObjectFromGroup: public a2dCommand
{
public:

    //! used to release object from a a2dCanvasDocument in the current parent
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_ReleaseObjectFromGroup( a2dMultiEditTool* tool = NULL, a2dCanvasObject* object = NULL );

    ~a2dCommand_ReleaseObjectFromGroup( void );

    bool Do();
    bool Undo();

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

protected:

    a2dCanvasObjectPtr m_canvasobject;
    int m_index;
    a2dMultiEditTool* m_tool;
};

const a2dCommandId a2dCommand_AddObjectToGroup::Id( wxT( "AddObjectToGroup" ) );
const a2dCommandId a2dCommand_ReleaseObjectFromGroup::Id( wxT( "ReleaseObjectFromGroup" ) );

/*
*   a2dCommand_AddObjectToGroup
*/
a2dCommand_AddObjectToGroup::a2dCommand_AddObjectToGroup( a2dMultiEditTool* tool, a2dCanvasObject* object ):
    a2dCommand( true, a2dCommand_AddObjectToGroup::Id )
{
    m_canvasobject = object;
    m_tool = tool;
}

a2dCommand_AddObjectToGroup::~a2dCommand_AddObjectToGroup( void )
{
}

bool a2dCommand_AddObjectToGroup::Do( void )
{
    m_tool->AddToGroup( m_canvasobject );
    return true;
}

bool a2dCommand_AddObjectToGroup::Undo( void )
{
    m_tool->RemoveFromGroup( m_canvasobject, 0 );
    return true;
}

/*
*   a2dCommand_ReleaseObjectFromGroup
*/
a2dCommand_ReleaseObjectFromGroup::a2dCommand_ReleaseObjectFromGroup( a2dMultiEditTool* tool, a2dCanvasObject* object ):
    a2dCommand( true, a2dCommand_ReleaseObjectFromGroup::Id )
{
    m_canvasobject = object;
    //for undo preserve location
    m_index = 0;
    m_tool = tool;
}

a2dCommand_ReleaseObjectFromGroup::~a2dCommand_ReleaseObjectFromGroup( void )
{
}

bool a2dCommand_ReleaseObjectFromGroup::Do( void )
{
    m_tool->RemoveFromGroup( m_canvasobject, m_index );

    if ( m_index == -1 )
        a2dGeneralGlobals->ReportError( a2dError_canvasObjectRelease );

    return true;
}

bool a2dCommand_ReleaseObjectFromGroup::Undo( void )
{
    m_tool->AddToGroup( m_canvasobject );
    return true;
}


//----------------------------------------------------------------------------
// a2dMultiSelectGroup
//----------------------------------------------------------------------------
//! a2dRecursiveEditTool holds multiple edit object in here
/*! When this object is set pending also its child objects are set pending.

    \ingroup tools commands
*/
class A2DCANVASDLLEXP a2dMultiSelectGroup: public a2dCanvasObject
{
    DECLARE_EVENT_TABLE()

public:

    //!constructor
    a2dMultiSelectGroup();

    //!destructor
    ~a2dMultiSelectGroup() {};

    a2dMultiSelectGroup( const a2dMultiSelectGroup& other, CloneOptions options, a2dRefMap* refs );

    a2dCanvasObject* CloneShallow();

    virtual a2dCanvasObject* StartEdit( a2dBaseTool* tool, wxUint16 editmode, wxEditStyle editstyle = wxEDITSTYLE_COPY, a2dRefMap* refs = NULL );

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    virtual void RenderChildObjects( a2dIterC& ic, RenderChild& whichchilds, a2dAffineMatrix* tworld, OVERLAP clipparent );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    void OnHandleEvent( a2dHandleMouseEvent& event );


private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dMultiSelectGroup( const a2dMultiSelectGroup& other );
};

template class A2DCANVASDLLEXP a2dSmrtPtr<a2dMultiSelectGroup>;


BEGIN_EVENT_TABLE( a2dMultiSelectGroup, a2dCanvasObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dMultiSelectGroup::OnCanvasObjectMouseEvent )
    EVT_CANVASHANDLE_MOUSE_EVENT( a2dMultiSelectGroup::OnHandleEvent )
END_EVENT_TABLE()

a2dMultiSelectGroup::a2dMultiSelectGroup(): a2dCanvasObject()
{
    m_childobjects = new a2dCanvasObjectList();
}

a2dMultiSelectGroup::a2dMultiSelectGroup( const a2dMultiSelectGroup& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
}

a2dObject* a2dMultiSelectGroup::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dMultiSelectGroup( *this, options, refs );
}

a2dCanvasObject* a2dMultiSelectGroup::CloneShallow()
{
    a2dMultiSelectGroup* newgr = new a2dMultiSelectGroup();
    for( a2dCanvasObjectList::iterator iter = m_childobjects->begin(); iter != m_childobjects->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        newgr->Append( obj );
    }
    return newgr;
}

void a2dMultiSelectGroup::OnHandleEvent( a2dHandleMouseEvent& event )
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
            if ( restrictEngine )
                restrictEngine->SetRestrictPoint( xw, yw );
        }
        else if ( event.GetMouseEvent().LeftUp() )
        {
            if ( m_lworld != original->GetTransformMatrix() )
            {
                for( a2dCanvasObjectList::iterator iter = original->GetChildObjectList()->begin(); iter != original->GetChildObjectList()->end(); iter++ )
                {
                    a2dCanvasObject* obj = *iter;
                    a2dAffineMatrix newtrans = m_lworld;
                    newtrans *= obj->GetTransformMatrix();
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( obj, a2dCanvasObject::PROPID_TransformMatrix, newtrans ) );
                }
                for( a2dCanvasObjectList::iterator iter2 = GetChildObjectList()->begin(); iter2 != GetChildObjectList()->end(); iter2++ )
                {
                    a2dCanvasObject* obj = *iter2;
                    if ( obj->GetBin2() )
                    {
                        a2dAffineMatrix newtrans = m_lworld;
                        newtrans *= obj->GetTransformMatrix();
                        obj->SetTransformMatrix( newtrans );
                    }
                }
                // matrix is set to children, now reset group matrix of editcopy (original is always identity matrix )
                SetTransformMatrix();
            }
        }
        else if ( event.GetMouseEvent().Dragging() )
        {
            if ( restrictEngine )
                restrictEngine->RestrictPoint( xw, yw );
            ic->GetInverseTransform().TransformPoint( xw, yw, xwi, ywi );

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

void a2dMultiSelectGroup::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();
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
            ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );
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
            a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
            if ( IsHitWorld( *ic, hitevent ) )
            {
                if ( restrictEngine )
                    restrictEngine->SetRestrictPoint( xw, yw );

                if ( 1 )
                {
                    a2dCanvasObject* hit = NULL;
                    if ( hit && hit->GetEditable() ) //subediting of child or labels, is we have a hit.
                    {
                        a2dIterCU cu( *ic, original );
                        a2dToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dToolContr );

                        //editing is always a oneshot, since editing of children here is under control of the
                        //a2dCanvasObject (can be different for derived classes). So editing tool should not take over
                        //the decision to edit one after another child.
                        ic->SetCorridorPath( true );
                        controller->StartEditingObject( hit, *ic );
                    }
                    //subediting was not wanted or hit, therefore now start dragging the object itself
                    //at mouse LeftDown
                    else if ( IsDraggable() )
                    {
                        // Corridor is already in place, since edit tool is active.
                        // Set corridor captured on this object, in order to drag.
                        ic->SetCorridorPathToObject( this );
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
            ic->SetCorridorPathToParent();
            ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );

            if ( original->GetTransformMatrix() != m_lworld )
            {
                for( a2dCanvasObjectList::iterator iter = original->GetChildObjectList()->begin(); iter != original->GetChildObjectList()->end(); iter++ )
                {
                    a2dCanvasObject* obj = *iter;
                    a2dAffineMatrix newtrans = m_lworld;
                    newtrans *= obj->GetTransformMatrix();
                    m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( obj, a2dCanvasObject::PROPID_TransformMatrix, newtrans ) );
                }
                for( a2dCanvasObjectList::iterator iter2 = GetChildObjectList()->begin(); iter2 != GetChildObjectList()->end(); iter2++ )
                {
                    a2dCanvasObject* obj = *iter2;
                    if ( obj->GetBin2() )
                    {
                        a2dAffineMatrix newtrans = m_lworld;
                        newtrans *= obj->GetTransformMatrix();
                        obj->SetTransformMatrix( newtrans );
                    }
                }
                // matrix is set to children, now reset group matrix of editcopy (original is always identity matrix )
                SetTransformMatrix();
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
void a2dMultiSelectGroup::RenderChildObjects( a2dIterC& ic, RenderChild& whichchilds, a2dAffineMatrix* WXUNUSED( tworld ), OVERLAP clipparent )
{
    //ic.GetDrawer2D()->SetDrawStyle( a2dWIREFRAME_INVERT_ZERO_WIDTH );
    ic.GetDrawer2D()->SetDrawStyle( a2dWIREFRAME_ZERO_WIDTH );
    a2dCanvasObject::RenderChildObjects( ic, whichchilds, clipparent );
    //ic.GetDrawer2D()->SetDrawStyle( a2dFILLED );
}

a2dCanvasObject* a2dMultiSelectGroup::StartEdit( a2dBaseTool* tool, wxUint16 editmode, wxEditStyle editstyle, a2dRefMap* refs )
{
    if ( m_flags.m_editable )
    {
        a2dCanvasObjectPtr editcopy;
        editcopy = TClone( clone_flat | clone_seteditcopy | clone_setoriginal, refs );

        editcopy->DoConnect( false );
        SetSnapTo( false );
        editcopy->SetSnapTo( false );
        SetSelected( false );
        editcopy->SetSelected( false );

        if ( ! PROPID_Allowrotation->GetPropertyValue( this ) )
            PROPID_Allowrotation->SetPropertyToObject( editcopy, true );
        if ( ! PROPID_Allowsizing->GetPropertyValue( this ) )
            PROPID_Allowsizing->SetPropertyToObject( editcopy, true );
        if ( ! PROPID_Allowskew->GetPropertyValue( this ) )
            PROPID_Allowskew->SetPropertyToObject( editcopy, true );
        PROPID_Parent->SetPropertyToObject( editcopy, tool->GetParentObject() );
        editcopy->SetEditingRender( true );

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
            EndEdit();
            return NULL;
        }

        // Add the editcopy to the parent and set some flags
        tool->AddEditobject( editcopy );

        return editcopy;
    }

    return NULL;
}

IMPLEMENT_CLASS( a2dMultiEditTool, a2dObjectEditTool )

BEGIN_EVENT_TABLE( a2dMultiEditTool, a2dObjectEditTool )
    EVT_COM_EVENT( a2dMultiEditTool::OnComEvent )
    EVT_MOUSE_EVENTS( a2dMultiEditTool::OnMouseEvent )
    EVT_IDLE( a2dMultiEditTool::OnIdle )
    EVT_CHAR( a2dMultiEditTool::OnChar )
    EVT_UNDO( a2dMultiEditTool::OnUndoEvent )
    EVT_REDO( a2dMultiEditTool::OnRedoEvent )
END_EVENT_TABLE()

a2dMultiEditTool::a2dMultiEditTool( a2dStToolContr* controller ): a2dObjectEditTool( controller )
{
    GetDrawingPart()->SetMouseEvents( false );
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_oneshot = false;
}

a2dMultiEditTool::~a2dMultiEditTool()
{
    if ( GetDrawingPart() )
        GetDrawingPart()->SetMouseEvents( true );
}

void a2dMultiEditTool::DoStopTool( bool abort )
{
    a2dCanvasObjectPtr keepit = m_canvasobject;

//  m_controller->SetEvtHandlerEnabled( false );
//  SetEvtHandlerEnabled( false );

    a2dBaseTool::DoStopTool( abort );

    if ( GetDrawingPart() )
        GetDrawing()->AddPendingUpdatesOldNew();

    m_parentobject->SetSpecificFlags( false, a2dCanvasOFlags::HighLight );

//  SetEvtHandlerEnabled( true );
//  m_controller->SetEvtHandlerEnabled( true );
}

bool a2dMultiEditTool::AddToGroup( a2dCanvasObject* canvasobject )
{
    a2dAffineMatrix inverse = m_canvasobject->GetTransformMatrix();
    inverse.Invert();
    canvasobject->Transform( inverse );
    m_original->Append( canvasobject );
    m_canvasobject->Append( canvasobject );
    m_canvasobject->Update( a2dCanvasObject::updatemask_force );
    m_canvasobject->ReStartEdit( 0 );
    return true;
}

bool a2dMultiEditTool::RemoveFromGroup( a2dCanvasObject* canvasobject, int index )
{
    m_canvasobject->ReleaseChild( canvasobject, false, false, true );
    m_original->ReleaseChild( canvasobject, false, false, true );
    m_canvasobject->Update( a2dCanvasObject::updatemask_force );
    m_original->Update( a2dCanvasObject::updatemask_force );
    m_canvasobject->ReStartEdit( 0 );
    return true;
}

void a2dMultiEditTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw = GetDrawer2D()->DeviceToWorldX( m_x );
    double yw = GetDrawer2D()->DeviceToWorldY( m_y );
    m_xwprev = xw;
    m_ywprev = yw;

    if ( event.Moving() && !GetBusy() )
    {
        if ( GetDrawingPart()->IsHitWorld( xw, yw ) )
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );
        else
            GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  ) );

    }

    if ( event.LeftDown() && !GetBusy() )
    {
        a2dCanvasObject* hitobject = GetDrawingPart()->IsHitWorld( xw, yw );

        if ( !hitobject )
            return;

        a2dAffineMatrix inverse = hitobject->GetTransformMatrix();
        m_original = new a2dMultiSelectGroup();
        m_original->Append( hitobject );
        m_original->SetVisible( false );

        if ( !EditStart() )
            return;

        RedirectToEditObject( event );

    }
    else if ( event.LeftDown() && GetBusy() )
    {
        a2dCanvasObject* hitobject = NULL;
        hitobject = GetDrawingPart()->IsHitWorld( xw, yw );

        if ( hitobject == m_canvasobject || hitobject == m_original )
        {
            //test the edit copy object, to test on handles
            a2dAffineMatrix selectworld = m_original->GetTransformMatrix();
            a2dIterC ic( GetDrawingPart() );
            a2dIterCU cu( ic, selectworld );
            a2dHitEvent hitevent = a2dHitEvent( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
            hitobject = m_canvasobject->IsHitWorld( ic, hitevent );
            if ( hitobject )
            {
                if ( !event.m_shiftDown )
                {
                    RedirectToEditObject( event );
                }
                else
                {
                    //remove object hit from selection by setting its bin flag off
                    if ( m_original->Find( hitobject ) )
                    {
                        GetDrawingPart()->GetDrawing()->GetCommandProcessor()->Submit( new a2dCommand_ReleaseObjectFromGroup( this, hitobject ) );
                    }
                }
                return;
            }
        }

        {
            //no hit in selection
            if ( !m_oneshot )
            {
                hitobject = GetDrawingPart()->IsHitWorld( xw, yw );

                if ( !hitobject )
                {
                    //nothing new so continue with current editobject
                    RedirectToEditObject( event );
                    return;
                }

                if ( !hitobject->GetEditable() || hitobject == m_canvasobject )
                {
                    return;
                }

                if ( event.m_shiftDown ) //user wants to add object to the  multigroup
                {
                    GetDrawingPart()->GetDrawing()->GetCommandProcessor()->Submit( new a2dCommand_AddObjectToGroup( this, hitobject ) );
                }
                else
                {
                }
            }
            else
            {
                FinishBusyMode();
                //RedirectToEditObject( event );
            }
        }
    }
    else if ( event.LeftDClick() && GetBusy() )
    {
        if ( GetBusy() )
            FinishBusyMode();
        else
            StopTool();
    }
    else if ( GetBusy() )
    {
        if ( event.RightDown() )
            event.Skip();
        else
            RedirectToEditObject( event );
    }
    else
    {
        event.Skip();
    }
}

bool a2dMultiEditTool::RedirectToEditObject( wxMouseEvent& event )
{
    if ( !m_original->GetFlag( a2dCanvasOFlags::Editing ) )
        return false;

    bool isHit;
    bool processed = false;

    int margin = GetDrawing()->GetHabitat()->GetHitMarginDevice();

    a2dAffineMatrix cWorld;

    processed = GetDrawingPart()->ProcessCanvasObjectEvent( event, isHit, m_xwprev, m_ywprev, margin );

    wxPoint pos = m_stcontroller->GetMousePosition();
    GetDrawingPart()->UpdateCrossHair( pos.x, pos.y );

    //the event did stop the editing of the object
    if ( m_original && !m_original->GetFlag( a2dCanvasOFlags::Editing ) )
    {
        if ( m_halted )
            CleanupToolObjects();
        else
            FinishBusyMode();
    }

    return processed;
}

bool a2dMultiEditTool::StartEditing( a2dCanvasObject* startobject )
{
    m_original = new a2dMultiSelectGroup();
    m_original->Append( startobject );
    m_original->SetVisible( false );
    EditStart();
    return true;
}

bool a2dMultiEditTool::StartEditingSelected()
{
    a2dCanvasObjectList* objects = m_parentobject->GetChildObjectList();
    a2dCanvasObjectList* objs = objects->Clone( a2dCanvasOFlags::SELECTED, a2dObject::clone_flat );

    m_original = new a2dMultiSelectGroup();
    m_original->SetVisible( false );
    forEachIn( a2dCanvasObjectList, objs )
    {
        a2dCanvasObject* obj = *iter;
        m_original->Append( obj );
    }
    delete objs;
    EditStart();
    return true;
}

bool a2dMultiEditTool::EditStart()
{
    //temporary add the a2dMultiSelectGroup object for editing the group.
    m_parentobject->Append( m_original );

    //to be able to do mouse hits already bbox's need to be up to date.
    m_original->Update( a2dCanvasObject::updatemask_force );

    //m_original->SetFill( a2dTRANSPARENT_FILL );
    //m_original->SetStroke( GetDrawing()->GetHabitat()->GetHandle()->GetStroke() );

    EnterBusyMode();

    return true;
}

void a2dMultiEditTool::CleanupToolObjects()
{
    SetEditObject( NULL );

    if ( GetBusy() )
    {
        if ( m_original->GetFlag( a2dCanvasOFlags::Editing ) )
            m_canvasobject->EndEdit();

        if ( GetDrawingPart() ) //in case of destruction this is set to NULL
            GetDrawingPart()->SetCursor( *wxSTANDARD_CURSOR );

        m_original->SetSelected( m_preserve_select );
        a2dCanvasObjectList* objects = m_original->GetChildObjectList();

        GetDrawingPart()->GetDrawing()->GetCommandProcessor()->GetCurrentGroup()->ClearCommandsById( a2dCommand_AddObjectToGroup::Id );
        GetDrawingPart()->GetDrawing()->GetCommandProcessor()->GetCurrentGroup()->ClearCommandsById( a2dCommand_ReleaseObjectFromGroup::Id );

        m_parentobject->ReleaseChild( m_original, false, false, true );

        m_original->AdjustPinLocation();

        GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );

        if ( !m_halted )
            CloseCommandGroup();

        if ( m_disableOtherViews && GetDrawingPart() )
        {
            //GetDrawingPart()->GetCanvasDocument()->EnableAllViews( true, GetDrawingPart() );
        }
    }
    a2dStTool::CleanupToolObjects();

    m_original = 0;
    m_canvasobject = 0;
}

void a2dMultiEditTool::OnIdle( wxIdleEvent& event )
{
    if ( m_original )
    {
        if ( m_pending )
        {
            //m_editobject->SetPending( true );
            //m_pending = false;
        }
    }
    else
        a2dStTool::OnIdle( event );
}

void a2dMultiEditTool::OnChar( wxKeyEvent& event )
{
    event.Skip();
}

void a2dMultiEditTool::OnComEvent( a2dComEvent& event )
{
    event.Skip();
}


void a2dMultiEditTool::SetActive( bool active )
{
    if ( active )
    {
        if ( m_halted )
        {
            m_halted = false;
            //RESTART editing so the edit copy becomes updated from the original.
            //original may have changed because of sub-editing of child objects.
            CreateToolObjects();
            m_active = true;
        }
        m_pending = true;
        a2dBaseTool::SetActive( active );
    }
    else
    {
        //the subedit or other subtools should not influence style of the object.
        //When this tool is activated again, the CurrentCanvasObject is set right again.
        //SetEditObject( NULL );

        m_pending = true;
        a2dBaseTool::SetActive( active );
    }
}

void a2dMultiEditTool::OnUndoEvent( a2dCommandProcessorEvent& WXUNUSED( event ) )
{
    if ( GetBusy() && !m_halted )
    {
        // recreate edit copies so the edit copy becomes updated from the original.
        wxUint16 editmode = a2dCanvasObject::PROPID_Editmode->GetPropertyValue( m_canvasobject );
        ReStartEdit( editmode );
    }
}

void a2dMultiEditTool::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    OnUndoEvent( event );
}




//----------------------------------------------------------------------------
// a2dObjectEditTool
//----------------------------------------------------------------------------



DEFINE_EVENT_TYPE( a2dObjectEditTool::sig_toolStartEditObject )

IMPLEMENT_CLASS( a2dObjectEditTool, a2dStTool )

const a2dCommandId a2dObjectEditTool::COMID_PushTool_ObjectEdit( wxT( "PushTool_ObjectEdit" ) );

BEGIN_EVENT_TABLE( a2dObjectEditTool, a2dStTool )
    EVT_COM_EVENT( a2dObjectEditTool::OnComEvent )
    EVT_MOUSE_EVENTS( a2dObjectEditTool::OnMouseEvent )
    EVT_IDLE( a2dObjectEditTool::OnIdle )
    EVT_CHAR( a2dObjectEditTool::OnChar )
    EVT_UNDO( a2dObjectEditTool::OnUndoEvent )
    EVT_REDO( a2dObjectEditTool::OnRedoEvent )
    EVT_DO( a2dRecursiveEditTool::OnDoEvent )
    EVT_KEY_UP( a2dObjectEditTool::OnKeyUp )
    EVT_KEY_DOWN( a2dObjectEditTool::OnKeyDown )
END_EVENT_TABLE()

a2dObjectEditTool::a2dObjectEditTool( a2dStToolContr* controller, a2dIterC& ic, int editmode, bool SingleClickToEnd )
    : a2dStTool( controller )
{
    m_mode = editmode;

    GetDrawingPart()->SetMouseEvents( false );
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_canvasobject = 0;
    m_halted = false;
    m_oneshot = true;
    m_triggerRestart = false;
    m_releaseEnd = false;
    m_abortInStartEdit = false;

    m_SingleClickToEnd = SingleClickToEnd;

    m_disableOtherViews = false;//true;

    // overrule corridor of base class setting, which is based on the view is less flexible,
    // matrixes calculated might not contain all in case of complex object in corridor.
    SetContext( ic, NULL );

    a2dCanvasCommandProcessor* docCmdh = GetCanvasCommandProcessor();
    docCmdh->ConnectEvent( wxEVT_DO, this );
    docCmdh->ConnectEvent( wxEVT_UNDO, this );
    docCmdh->ConnectEvent( wxEVT_REDO, this );
    docCmdh->ConnectEvent( a2dEVT_COM_EVENT, this );

    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );
}

a2dObjectEditTool::a2dObjectEditTool( a2dStToolContr* controller, int editmode, bool SingleClickToEnd )
    : a2dStTool( controller )
{
    m_corridor = a2dCorridor( *GetDrawingPart() );
    if ( !m_corridor.empty() )
    {
        m_parentobject = m_corridor.back();
        wxASSERT_MSG( m_parentobject->GetRoot() != 0 , wxT( "parentobject of tools has no drawing root set" ) );
    }
    else
        m_parentobject = GetDrawingPart()->GetShowObject();

    m_original = NULL;
    m_mode = editmode;

    GetDrawingPart()->SetMouseEvents( false );
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_canvasobject = 0;
    m_halted = false;
    m_oneshot = true;
    m_triggerRestart = false;
    m_releaseEnd = false;

    m_SingleClickToEnd = SingleClickToEnd;

    m_disableOtherViews = false;//true;

    if ( m_parentobject != GetDrawingPart()->GetShowObject() )
        m_parentobject->SetSpecificFlags( true, a2dCanvasOFlags::HighLight );

    a2dCanvasCommandProcessor* docCmdh = GetCanvasCommandProcessor();
    docCmdh->ConnectEvent( wxEVT_DO, this );
    docCmdh->ConnectEvent( wxEVT_UNDO, this );
    docCmdh->ConnectEvent( wxEVT_REDO, this );
    docCmdh->ConnectEvent( a2dEVT_COM_EVENT, this );

    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );
}

bool a2dObjectEditTool::SetContext( a2dIterC& ic, a2dCanvasObject* objectToEdit )
{
    m_corridor = a2dCorridor( ic );
    m_parentobject = m_corridor.back();

    if ( objectToEdit )
        StartToEdit( objectToEdit );

    // add captured editclone too
    m_corridor = a2dCorridor( ic );

    return true;
}

void a2dObjectEditTool::StartToEdit( a2dCanvasObject* objectToEdit )
{
    if ( !GetDrawingPart()->GetDrawing()->GetMayEdit() )
    {
        wxLogWarning( _T( "You may not edit this drawing" ) );              
        return;   
    }

    m_original = objectToEdit;

    if ( m_parentobject != GetDrawingPart()->GetShowObject() )
        m_parentobject->SetSpecificFlags( true, a2dCanvasOFlags::HighLight );

    bool ret = true;
    m_abortInStartEdit = false;
    if ( m_original )
        ret = EnterBusyMode();

    if ( !ret || !objectToEdit || ( objectToEdit && !m_canvasobject ) )
    {
        wxMessageBox(  _( "wanted object can not be edited" ), _( "Editing Component" ), wxOK );
        return;
    }

    // the editing process was started, but maybe if ended by the object directly. 
    // ( e.g. just a dialog editing, and no mouse handling ).
    if ( m_original && !m_original->GetEditing() )
    {
		//  m_abortInStartEdit is set true in a2dObjectEditTool::OnUndoEvent when dialog editing was canceled inside edited canvasobject.
        if ( m_abortInStartEdit )
            AbortBusyMode();
        else
            FinishBusyMode();
   }
}

a2dObjectEditTool::~a2dObjectEditTool()
{
    if ( GetDrawingPart() )
        GetDrawingPart()->SetMouseEvents( true );

    a2dCanvasCommandProcessor* docCmdh = GetCanvasCommandProcessor();
    if ( docCmdh )
    {
        docCmdh->DisconnectEvent( wxEVT_DO, this );
        docCmdh->DisconnectEvent( wxEVT_UNDO, this );
        docCmdh->DisconnectEvent( wxEVT_REDO, this );
        docCmdh->DisconnectEvent( a2dEVT_COM_EVENT, this );
    }
    a2dCanvasGlobals->DisconnectEvent( a2dEVT_COM_EVENT, this );
}

void a2dObjectEditTool::SetEditObject( a2dCanvasObject* objectToEdit )
{
    if ( GetDrawingPart() )
    {
        a2dComEvent event( this, objectToEdit, sig_toolComEventSetEditObject );
        ProcessEvent( event );
    }
}

void a2dObjectEditTool::SetMode( int mode )
{
    m_canvasobject->EndEdit();
    GetDrawingPart()->Update( a2dCANVIEW_UPDATE_OLDNEW );

    m_mode = mode;
    if ( m_mode > 2 ) m_mode = 0;

    CreateToolObjects();
}

void a2dObjectEditTool::DoStopTool( bool abort )
{
    a2dCanvasObjectPtr keepit = m_canvasobject;

//  m_controller->SetEvtHandlerEnabled( false );
//  SetEvtHandlerEnabled( false );

    a2dBaseTool::DoStopTool( abort );

    if ( GetDrawingPart() )
        GetDrawing()->AddPendingUpdatesOldNew();

    m_parentobject->SetSpecificFlags( false, a2dCanvasOFlags::HighLight );

//  SetEvtHandlerEnabled( true );
//  m_controller->SetEvtHandlerEnabled( true );
}

void a2dObjectEditTool::AdjustRenderOptions()
{
    switch( m_stcontroller->GetDragMode() )
    {
        case wxDRAW_RECTANGLE:
            m_renderOriginal = true;
            m_renderEditcopy = false;
            m_renderEditcopyOnTop = false;
            m_renderEditcopyEdit = true;
            m_renderEditcopyRectangle = false;
            break;

        case wxDRAW_ONTOP:
            m_renderOriginal = false;
            m_renderEditcopy = false;
            m_renderEditcopyOnTop = true;
            m_renderEditcopyEdit = false;
            m_renderEditcopyRectangle = false;
            break;

        case wxDRAW_REDRAW:
            wxASSERT_MSG( 0, _( "m_renderEditcopy is not yet implemented, m_renderEditcopyOnTop used instead" ) );
            m_renderOriginal = false;
            m_renderEditcopy = false;
            m_renderEditcopyOnTop = true;
            m_renderEditcopyEdit = false;
            m_renderEditcopyRectangle = false;
            break;
        default:
            ;
    }
}

bool a2dObjectEditTool::EnterBusyMode()
{
    return a2dStTool::EnterBusyMode();
}

void a2dObjectEditTool::FinishBusyMode( bool closeCommandGroup )
{
    if ( m_releaseEnd && GetBusy() )
        m_original->SetRelease( true );
    if ( m_oneshot && !m_halted )
    {
        m_parentobject->SetSpecificFlags( false, a2dCanvasOFlags::HighLight );
    }
    a2dStTool::FinishBusyMode();
}

void a2dObjectEditTool::AbortBusyMode()
{
    if ( m_releaseEnd && GetBusy() )
        m_original->SetRelease( true );
    a2dStTool::AbortBusyMode();
}

void a2dObjectEditTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active || m_halted )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw, yw;
    MouseToToolWorld( m_x, m_y, xw, yw );
    m_xwprev = xw;
    m_ywprev = yw;

    if ( event.Dragging() )
    {

    }

    if ( event.LeftDClick() )
    {
        if ( GetBusy() )
            FinishBusyMode();
        else
            StopTool();
    }
    else if ( event.LeftDown() && GetBusy() )
    {
        //goto the edit object itself, maybe for sub-editing.
        //Halted if sub-editing ( another tool is pushed on the tool stack )
        if ( RedirectToEditObject( event ) || m_stop || m_halted )
            return;

        // if left down was not handle inside object that is being edited,
        // that should lead to an end edit of the object, if the hit is outside the object.
        a2dCanvasObject* neweditobject = NULL;
        a2dIterC ic( GetDrawingPart() );
        a2dIterCU cu( ic, m_corridor.GetInverseTransform() );
        a2dHitEvent hitevent = a2dHitEvent( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
        neweditobject = m_parentobject->IsHitWorld( ic, hitevent );

        if ( !neweditobject && !m_SingleClickToEnd || neweditobject == m_original || neweditobject == m_canvasobject )
            //nothing new so continue with current editobject
            return;

        StopTool();
    }
    else if ( event.LeftUp() && GetBusy() )
    {
        if ( !RedirectToEditObject( event ) )
            event.Skip();
    }
    else if ( GetBusy() )
    {
        if ( !RedirectToEditObject( event ) )
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

bool a2dObjectEditTool::RedirectToEditObject( wxMouseEvent& event )
{
    if ( !m_original->GetFlag( a2dCanvasOFlags::Editing ) )
        return false;

	//SetStateString( "", 10 );
	//SetStateString( "", 11 );

    bool isHit;
    bool processed = false;

    int margin = GetDrawing()->GetHabitat()->GetHitMarginDevice();

    // the next NOT to tool world but to view world coordinates, since the event is going
    // from ShowObject all the way down.
    double xw = GetDrawer2D()->DeviceToWorldX( m_x );
    double yw = GetDrawer2D()->DeviceToWorldY( m_y );
    processed = GetDrawingPart()->ProcessCanvasObjectEvent( event, isHit, xw, yw, margin );

    if ( m_canvasobject && !event.Moving() )
    {
        m_canvasobject->AdjustPinLocation();

        a2dBoundingBox untrans = m_canvasobject->GetUnTransformedBbox( a2dCanvasObject::a2dCANOBJ_BBOX_EDIT );
        untrans.MapBbox(  m_canvasobject->GetTransform() );

       // a2dIterC ic( GetDrawingPart() );
       // ic.SetHitMarginDevice( ( int ) ( GetDrawing()->GetHabitat()->GetPin()->GetWidth() / 2.0 ) );
       // GetDrawing()->GetHabitat()->GetConnectionGenerator()->GeneratePinsToConnectObject( ic, m_parentobject, m_canvasobject );
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( false );

		double unitScale = GetDrawing()->GetUnitsScale();
		wxString state, form;
		//if ( m_stcontroller->GetStatusStrings()[10].IsEmpty() )
		{
			form = m_stcontroller->GetFormat() + " " + m_stcontroller->GetFormat();
			state.Printf( form, untrans.GetMinX()*unitScale, untrans.GetMinY()*unitScale );
			SetStateString( state, 10 );
		}
		//if ( m_stcontroller->GetStatusStrings()[11].IsEmpty() )
		{
			form = _T("width = ") + m_stcontroller->GetFormat() + _T(" height = ") + m_stcontroller->GetFormat();

			state.Printf( form, untrans.GetWidth()*unitScale, untrans.GetHeight()*unitScale );
			SetStateString( state, 11 );
		}
    }

    wxPoint pos = m_stcontroller->GetMousePosition();
    GetDrawingPart()->UpdateCrossHair( pos.x, pos.y );

    // If the event did stop the editing of the object, the editing flag is gone.
    // This may be because the object inside decided to stop editing,
    // OR it started pushing another subediting tool, and this tool should
    // continue later.
    if ( m_original && !m_original->GetEditing() )
    {
        if ( m_halted )
            CleanupToolObjects();
        else
            FinishBusyMode();
    }
    return processed;
}

bool a2dObjectEditTool::ReStartEdit( wxUint16 editmode )
{
    m_canvasobject->Update( a2dCanvasObject::updatemask_force );
    m_original->Update( a2dCanvasObject::updatemask_force );
    SetEditObject( NULL );

    if ( GetBusy() )
    {
        if ( m_original->GetFlag( a2dCanvasOFlags::Editing ) )
            m_canvasobject->EndEdit();

        m_original->AdjustPinLocation();
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );
    }
    a2dStTool::CleanupToolObjects();

    m_canvasobject = 0;

    m_mode = editmode;

    return CreateToolObjects();
}

bool a2dObjectEditTool::CreateToolObjects()
{
    AdjustRenderOptions();
    m_original->SetVisible( m_renderOriginal );

    m_preserve_select = m_original->GetSelected();
    m_original->SetSelected( false );
    m_original->SetSelected2( false );

    a2dRefMap refs;
    m_canvasobject = m_original->StartEdit( this, m_mode, wxEDITSTYLE_COPY, &refs );

    if ( !m_canvasobject )
        return false; //not editable object

    m_canvasobject->SetSelected( false );
    m_original->SetSnapTo( false );

#ifdef EDITBUF
    m_original->SetVisible( false );
    GetDrawingPart()->Update( a2dCANVIEW_UPDATE_PENDING | a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );
    GetCanvasDocument()->SetUpdatesPending( false );
    GetDrawingPart()->SetDocumentDrawStyle(  RenderTOOL_OBJECTS );
    GetCanvasDocument()->SetIgnorePendingObjects( true );
#endif
    //! event to make it possible to set style in for instance modeless style dialog
    a2dComEvent event( this, m_original, sig_toolStartEditObject );
    ProcessEvent( event );


    //drag is comming so first create wires where there are non.
    a2dCanvasObjectList dragList;
    dragList.push_back( m_original );
	if ( m_canvasobject->IsConnect() ) // for a wire, we do not want extra wires at its ends, for normal objects they are often needed.
		PrepareForRewire( dragList, true, false, false, false, &refs ); 
	else
		PrepareForRewire( dragList, true, false, false, true, &refs ); 


    // only now we call this, earlier not possible, because clones of connectedwires not setup yet.
    //m_canvasobject->SetBin2( true );
    //m_canvasobject->SetBin( true );
    refs.LinkReferences( true );


    // Set the visibility of the original dragged object and the original connected wires.
    // When object itself sets the original invisible, that has priority. This is used in ediitng text,
    // where original visible is not wanted.
    if ( m_original->GetVisible() )
        m_original->SetVisible( m_renderOriginal );

    SetEditObject( m_original );

    GetDrawingPart()->SetCorridor( m_canvasobject, m_canvasobject );

    //if ( m_disableOtherViews )
    //    GetDrawingPart()->GetCanvasDocument()->EnableAllViews( false, GetDrawingPart() );

    return true;
}

void a2dObjectEditTool::CleanupToolObjects()
{
    SetEditObject( NULL );

    if ( GetBusy() )
    {
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( true );

        if ( m_original->GetFlag( a2dCanvasOFlags::Editing ) )
            if ( m_canvasobject )
                m_canvasobject->EndEdit();

        m_original->SetSelected( m_preserve_select );
        m_original->SetSelected2( m_preserve_select );

        if ( m_stcontroller->GetSelectionStateUndo() ) 
        {
    	    GetCanvasCommandProcessor()->Submit( new a2dCommandMasked( GetDrawingPart()->GetShowObject(), GetCommandgroup(), 
											 a2dCanvasOFlags::SELECTED, a2dCanvasOFlags::SELECTED2 ) );
        }
        m_original->AdjustPinLocation();

        if ( !m_halted )
            CloseCommandGroup();

#ifdef EDITBUF
        GetDrawingPart()->SetDocumentDrawStyle( RenderLAYERED | GetDrawing()->GetHabitat()->GetSelectDrawStyle()  );
        GetCanvasDocument()->SetIgnorePendingObjects( false );
#endif

        if ( m_disableOtherViews && GetDrawingPart() )
        {
            //GetDrawingPart()->GetCanvasDocument()->EnableAllViews( true, GetDrawingPart() );
        }
    }
    a2dStTool::CleanupToolObjects();
    m_canvasobject = 0;
}

void a2dObjectEditTool::Render()
{
    if ( m_original && GetBusy() )
    {
        // Render edit objects in Edit mode
        //if( m_renderEditcopyEdit )
        {
            if ( m_useEditOpaque == a2dOpaqueMode_Tool )
            {
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( m_editOpacityFactor );
                GetDrawingPart()->RenderTopObject( RenderTOOL_OBJECTS_STYLED, 1 );
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( 255 );
            }
            else if ( m_useEditOpaque == a2dOpaqueMode_Tool_FixedStyle )
            {
                a2dFill fill = *a2dTRANSPARENT_FILL;
                a2dStroke stroke = GetDrawing()->GetHabitat()->GetHandle()->GetStroke();
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( m_editOpacityFactor );
                GetDrawingPart()->SetFixedStyleFill( fill );
                GetDrawingPart()->SetFixedStyleStroke( stroke );
                GetDrawingPart()->RenderTopObject( RenderTOOL_OBJECTS, 1 );
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( 255 );
            }
            else if (  m_useEditOpaque == a2dOpaqueMode_Controller && m_stcontroller->GetUseOpaqueEditcopy() )
            {
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( m_stcontroller->GetOpacityFactorEditcopy() );
                GetDrawingPart()->RenderTopObject( RenderTOOL_OBJECTS_STYLED, 1 );
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( 255 );
            }
            else //if ( m_useEditOpaque == a2dOpaqueMode_Off )
            {
                a2dFill fill = *a2dTRANSPARENT_FILL;
                a2dStroke stroke = GetDrawing()->GetHabitat()->GetHandle()->GetStroke();
                GetDrawingPart()->GetDrawer2D()->SetOpacityFactor( 255 );
                GetDrawingPart()->SetFixedStyleFill( fill );
                GetDrawingPart()->SetFixedStyleStroke( stroke );
                GetDrawingPart()->RenderTopObject( RenderTOOL_OBJECTS, 1 );
            }
        }
    }
}

void a2dObjectEditTool::TriggerReStartEdit( wxUint16 editmode )
{
    m_triggerRestart = true;
}

void a2dObjectEditTool::OnIdle( wxIdleEvent& event )
{
    if ( m_triggerRestart && GetBusy() && !m_halted )
    {
        // recreate edit copies so the edit copy becomes updated from the original.
        wxUint16 editmode = a2dCanvasObject::PROPID_Editmode->GetPropertyValue( m_canvasobject );
        ReStartEdit( editmode );
        m_triggerRestart = false;
    }

#ifdef EDITBUF
    if ( m_canvasobject )
        if ( m_canvasobject->GetPending() )
            GetDrawingPart()->AddPendingObject( m_canvasobject );
#endif

    if ( m_original )
    {
        if ( m_pending )
        {
            //m_canvasobject->SetPending( true );
            //m_pending = false;
        }
    }
    else
        a2dStTool::OnIdle( event );
}

void a2dObjectEditTool::OnKeyDown( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_ESCAPE:
            {
                StopTool();
                break;
            }
            default:
                event.Skip();
            break;
        }
    }
    else
        //all text like keys should arrive in a text object when its edited.
        event.Skip();
}

void a2dObjectEditTool::OnKeyUp( wxKeyEvent& event )
{
    if ( GetBusy() && m_pending )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_UP:
            case WXK_DOWN:
            case WXK_LEFT:
            case WXK_RIGHT:
            {
                a2dDrawing* drawing = GetDrawingPart()->GetDrawing();
                drawing->GetCommandProcessor()->Submit(
                    new a2dCommand_SetCanvasProperty( m_original, a2dCanvasObject::PROPID_TransformMatrix,
                                                      m_canvasobject->GetTransformMatrix() ) );

            }
            break;
            default:
            {
                event.Skip();
            }
        }
    }
    else
        event.Skip();
}

void a2dObjectEditTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_TAB:
            {
                if ( event.m_shiftDown )
                {
                    int mode = m_mode;
                    SetMode( ++mode );
                }
                else if ( !m_canvasobject || !m_canvasobject->ProcessEvent( event ) ) //handover event to object.
                    event.Skip();
            }
            break;
            default:
            {
                bool processed = false;
                if( m_canvasobject )
                    processed = m_canvasobject->ProcessEvent( event ); //handover event to object.

                if ( !processed && m_canvasobject )
                {
                    if ( m_canvasobject->IsDraggable() )
                    {
                        double shiftx;
                        double shifty;
                        GetKeyIncrement( &shiftx, &shifty );

                        //now shift with snap distances in X or Y

                        switch( event.GetKeyCode() )
                        {
                            case WXK_UP:
                                shiftx = 0;
                                break;

                            case WXK_DOWN:
                                shiftx = 0;
                                shifty = -shifty;
                                break;

                            case WXK_LEFT:
                                shiftx = -shiftx;
                                shifty = 0;
                                break;

                            case WXK_RIGHT:
                                shifty = 0;
                                break;
                            default:
                            {
                                shiftx = 0;
                                shifty = 0;
                                event.Skip();
                            }
                        }

                        m_canvasobject->Translate( shiftx, shifty );
                        event.Skip( false );
                        m_pending = true;
                    }
                    else
                        event.Skip( false );
                }
                if( !m_canvasobject )
                    event.Skip();
            }
        }

        if( m_canvasobject )
        {
            m_canvasobject->AdjustPinLocation();
            GetDrawing()->GetHabitat()->GetConnectionGenerator()->RerouteWires( false );
        }
    }
    else
        event.Skip();
}

void a2dObjectEditTool::OnComEvent( a2dComEvent& event )
{
    if ( GetBusy() )
    {
        if ( event.GetId() == a2dStTool::sig_toolBeforePush )
        {
            // If tool is busy, it will enter halted state
            if ( GetBusy() && !m_halted )
            {
                m_halted = true;
                CleanupToolObjects();
            }
            m_pending = true;
        }
        else if ( GetDrawingPart() && event.GetId() == a2dDrawingPart::sig_changedShowObject )
        {
            if ( event.GetEventObject() == GetDrawingPart() )
            {
                StopTool();
            }
        }
        else if ( event.GetId() == a2dHabitat::sig_SetContourWidth )
        {
            m_contourwidth = GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale();
            if ( GetBusy() )
            {
                m_canvasobject->SetContourWidth( m_contourwidth );
                m_original->SetContourWidth( m_contourwidth );
            }
        }
        else if ( event.GetId() == a2dHabitat::sig_SetPathType )
        {
            a2dPATH_END_TYPE pathtype = GetDrawing()->GetHabitat()->GetPathType();
            if ( GetBusy() )
            {
                a2dPolylineL* poly = wxStaticCast( m_original.Get(), a2dPolylineL );
                if ( poly )
                {
                    poly->SetPathType( pathtype );
                    poly = wxStaticCast( m_canvasobject.Get(), a2dPolylineL );
                    poly->SetPathType( pathtype );
                }
            }
        }
        else
            event.Skip();
    }

    event.Skip();
}

void a2dObjectEditTool::SetActive( bool active )
{
    if ( active )
    {
        if ( m_halted )
        {
            m_halted = false;
            //RESTART editing so the edit copy becomes updated from the original.
            //original may have changed because of sub-editing of child objects.
            CreateToolObjects();
            m_active = true;
        }
        m_pending = true;
        a2dBaseTool::SetActive( active );
    }
    else
    {
        //the subedit or other subtools should not influence style of the object.
        //When this tool is activated again, the CurrentCanvasObject is set right again.
        SetEditObject( NULL );

        m_pending = true;
        a2dBaseTool::SetActive( active );
    }
}

void a2dObjectEditTool::OnUndoEvent( a2dCommandProcessorEvent& event )
{
    if ( GetBusy() && !m_halted )
    {
		if ( event.GetCommand()->GetCommandId() == &a2dCommand_ChangeText::Id )
		{
			a2dCommand_ChangeText* cmd = wxDynamicCast( event.GetCommand(), a2dCommand_ChangeText );
			if ( cmd )
			{
				a2dText* textorg = cmd->GetTextObject();
				a2dText* textcopy = wxStaticCast( m_canvasobject.Get(), a2dText );
				textcopy->SetText( textorg->GetText() );
				textcopy->SetCaret( textorg->GetCaret() );
			}
		}
		else
		{
            if ( m_canvasobject ) //dlg editing does not create editcopies
            {
			    // recreate edit copies so the edit copy becomes updated from the original.
			    wxUint16 editmode = a2dCanvasObject::PROPID_Editmode->GetPropertyValue( m_canvasobject );
			    TriggerReStartEdit( editmode );
            }
            else //currently only just when a dialog which was canceled and interactive editing was not done.
            {
                 m_abortInStartEdit = true;
            }
		}
    }
    event.Skip();
}

void a2dObjectEditTool::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    OnUndoEvent( event );
    event.Skip();
}

void a2dObjectEditTool::OnDoEvent( a2dCommandProcessorEvent& event )
{
    const a2dCommandId* comId = event.GetCommand()->GetCommandId();
    if ( GetBusy() )
    {
        if ( ( comId == &a2dCommand_ChangeText::Id ) && wxDynamicCast( m_original.Get(), a2dText ) )
        {
            a2dText* text = wxDynamicCast( m_original.Get(), a2dText );
            if ( text )
            {
                //WHY text->SetFont( GetDrawing()->GetHabitat()->GetTextTemplateObject()->GetFont() );
                //WHY a2dText* texteditobj = wxDynamicCast( m_canvasobject.Get(), a2dText );
                //WHY texteditobj->SetFont( GetDrawing()->GetHabitat()->GetTextTemplateObject()->GetFont() );
            }
        }
        /* a COMMAND WHICH CHANGES CENTRAL STORED THINGS IN GetDrawing()->GetHabitat(), MAY CHANGE EDIT OBJECT ITS STYLE LIKE DOWN HERE.
        But not any commands should lead to changes (e.g. fixed style objects should not change ).

        if ( wxDynamicCast( m_original.Get(), a2dPolylineL ) )
        {
            a2dPolylineL* poly = wxDynamicCast( m_original.Get(), a2dPolylineL );
            if ( poly )
            {
                if ( poly->GetSpline() != GetDrawing()->GetHabitat()->GetSpline() )
                    poly->SetSpline( GetDrawing()->GetHabitat()->GetSpline() );
                if ( poly->GetBegin() != GetDrawing()->GetHabitat()->GetLineBegin() )
                    poly->SetBegin( GetDrawing()->GetHabitat()->GetLineBegin() );
                if ( poly->GetEnd() != GetDrawing()->GetHabitat()->GetLineEnd() )
                    poly->SetEnd( GetDrawing()->GetHabitat()->GetLineEnd() );
                if ( poly->GetEndScaleX() != GetDrawing()->GetHabitat()->GetEndScaleX() )
                    poly->SetEndScaleX( GetDrawing()->GetHabitat()->GetEndScaleX() );
                if ( poly->GetEndScaleY() != GetDrawing()->GetHabitat()->GetEndScaleY() )
                    poly->SetEndScaleY( GetDrawing()->GetHabitat()->GetEndScaleY() );
            }
        }
        if ( wxDynamicCast( m_original.Get(), a2dEndsLine ) )
        {
            a2dEndsLine* line = wxDynamicCast( m_original.Get(), a2dEndsLine );
            if ( line )
            {
                if ( line->GetBegin() != GetDrawing()->GetHabitat()->GetLineBegin() )
                    line->SetBegin( GetDrawing()->GetHabitat()->GetLineBegin() );
                if ( line->GetEnd() != GetDrawing()->GetHabitat()->GetLineEnd() )
                    line->SetEnd( GetDrawing()->GetHabitat()->GetLineEnd() );
                if ( line->GetEndScaleX() != GetDrawing()->GetHabitat()->GetEndScaleX() )
                    line->SetEndScaleX( GetDrawing()->GetHabitat()->GetEndScaleX() );
                if ( line->GetEndScaleY() != GetDrawing()->GetHabitat()->GetEndScaleY() )
                    line->SetEndScaleY( GetDrawing()->GetHabitat()->GetEndScaleY() );
            }
        }
        if ( m_original && m_original->GetContourWidth() != GetDrawing()->GetHabitat()->GetContourWidth() / doc->GetUnitsScale() )
        {
            m_contourwidth = GetDrawing()->GetHabitat()->GetContourWidth() / doc->GetUnitsScale();
            m_canvasobject->SetContourWidth( m_contourwidth );
            m_original->SetContourWidth( m_contourwidth );
        }
        a2dPolylineL* poly = wxDynamicCast( m_original.Get(), a2dPolylineL );
        if ( poly && poly->GetPathType() != GetDrawing()->GetHabitat()->GetPathType() )
            poly->SetPathType( GetDrawing()->GetHabitat()->GetPathType() );
        a2dSLine* line = wxDynamicCast( m_canvasobject.Get(), a2dSLine );
        if ( line && line->GetPathType() != GetDrawing()->GetHabitat()->GetPathType() )
            line->SetPathType( GetDrawing()->GetHabitat()->GetPathType() );

        */
    }
    event.Skip();
}






