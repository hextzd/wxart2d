/*! \file canextobj/src/canext.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canext.cpp,v 1.65 2009/09/26 20:40:32 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canextobj/canext.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/drawer.h"
#include "wx/editor/editmod.h"

#include <math.h>

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dTextGroup, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dDrawingFrame, a2dPolygonLClipper2 )

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// a2dTextGroup
//----------------------------------------------------------------------------

a2dTextGroup::a2dTextGroup()
    : a2dCanvasObject()
{
}

a2dTextGroup::a2dTextGroup( double x, double y )
    : a2dCanvasObject()
{
    m_lworld.Translate( x, y );
}

a2dTextGroup::~a2dTextGroup()
{
}

a2dObject* a2dTextGroup::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dTextGroup( *this, options, refs );
};

a2dTextGroup::a2dTextGroup( const a2dTextGroup& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
}

void a2dTextGroup::RenderChildObjectsOneLayer( a2dIterC& ic, RenderChild& WXUNUSED( whichchilds ), OVERLAP clipparent )
{
    if ( m_childobjects == wxNullCanvasObjectList )
        return;

    //save the current style state of the a2dDrawer2D, in order to inherit and restore style of object
    a2dFill currentdrawer_fill = ic.GetDrawer2D()->GetDrawerFill();
    a2dStroke currentdrawer_stroke = ic.GetDrawer2D()->GetDrawerStroke();

    forEachIn( a2dCanvasObjectList, m_childobjects )
    {
        a2dCanvasObject* obj = *iter;

        int olayer = obj->GetLayer();

        if ( olayer == ic.GetLayer() || ic.GetLayer() == wxLAYER_ALL || obj->GetChildObjectList() )
        {

            // clipparent is either _IN or _ON      ( _OUT would not get us to this point )
            // testing clipping of children only needed when not totaly _IN.

            OVERLAP childclip = _IN;
            if ( clipparent != _IN )
                childclip = obj->GetClipStatus( ic, clipparent );

            //line++;

            if ( childclip != _OUT ) //if a child is _OUT, no need to render it.
            {
                obj->Render( ic, childclip );
            }
        }
    }

    ic.GetDrawer2D()->SetDrawerFill( currentdrawer_fill );
    ic.GetDrawer2D()->SetDrawerStroke( currentdrawer_stroke );
}



//----------------------------------------------------------------------------
// a2dDrawingFrame
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dDrawingFrame, a2dPolygonLClipper2 )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dDrawingFrame::OnCanvasObjectMouseEvent )
    EVT_CHAR( a2dDrawingFrame::OnChar )
END_EVENT_TABLE()

void a2dDrawingFrame::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dPolygonLClipper2::DoWalker( parent, handler );
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}

bool a2dDrawingFrame::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        PROPID_IncludeChildren->SetPropertyToObject( this, false );
        PROPID_Allowrotation->SetPropertyToObject( this, false );
        PROPID_Allowskew->SetPropertyToObject( this, false );

        return a2dPolygonLClipper2::DoStartEdit( editmode, editstyle );
    }

    return false;
}


/*
void a2dDrawingFrame::OnChar(wxKeyEvent& event)
{
    if ( m_flags.m_editingCopy )
    {
        switch(event.GetKeyCode())
        {
            case 'Z':
            case 'z':
            {
                a2dZoomTool* zoom = new a2dZoomTool(this);
                PushTool(zoom);
            }
            break;
            case WXK_SPACE:
            {
                StopTool();
            }
            break;
            case WXK_ESCAPE:
            {
                StopTool();
            }
            break;
            default:
                event.Skip();
    }
    else
        event.Skip();
}
*/

void a2dDrawingFrame::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();

    if ( m_flags.m_editingCopy )
    {
        //to world group coordinates to do hit test in world group coordinates
        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        a2dAffineMatrix atWorld = ic->GetTransform();

        if ( event.GetMouseEvent().LeftDown() )
        {
            if ( event.GetMouseEvent().m_shiftDown )
            {

                a2dDrawingFrame* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dDrawingFrame );
                a2dIterCU cu( *ic, original );

                a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr );
                ic->SetCorridorPath( true );

                /* dependency problem, can not be in editor module, if used here.
                                a2dSubDrawMasterTool* draw = new a2dSubDrawMasterTool( controller );
                                controller->PushTool( draw );
                */
                SetPending( true );
                ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
                /*
                                a2dDrawRectangleTool* drawrec = new a2dDrawRectangleTool( controller );
                                controller->PushTool( drawrec );
                                drawrec->SetEditAtEnd( true );

                                SetPending( true );
                                ic->GetCanvasView()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
                */
            }
            else if ( event.GetMouseEvent().m_controlDown )
            {
                a2dDrawingFrame* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dDrawingFrame );
                a2dIterCU cu( *ic, original );

                a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr );
                ic->SetCorridorPath( true );
                a2dDrawPolygonLTool* draw = new a2dDrawPolygonLTool( controller );
                controller->PushTool( draw );
                draw->SetEditAtEnd( true );

                SetPending( true );
                ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
            }
            else
            {
                a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
                if ( IsHitWorld( *ic, hitevent ) )
                {
                    a2dDrawingFrame* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dDrawingFrame );
                    a2dHitEvent hitinfo( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
                    hitinfo.m_xyRelToChildren = true;
                    a2dCanvasObject* curve = original->IsHitWorld( *ic, hitinfo );
                    if ( curve && curve->GetEditable() )
                    {
                        a2dIterCU cu2( *ic, original );
                        a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr );

                        ic->SetCorridorPath( true );
                        controller->StartEditingObject( curve, *ic );

                        SetPending( true );
                        ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
                    }
                    else
                        event.Skip();
                }
                else
                    EndEdit();
            }
        }
        else if ( event.GetMouseEvent().LeftDClick()  )
        {
            EndEdit();
        }
        else if ( event.GetMouseEvent().Moving() )
        {
            /*
                        a2dAffineMatrix cworld;
                        a2dHit how;
                        a2dAffineMatrix tworld;
                        tworld *= m_lworld;

                        a2dCanvasObject* curve = m_axesarea->WhichIsHitWorld( &tworld, xw, yw, NULL, how );
                        if ( curve )
                        {
                            event.Skip();
                        }
                        else
                        {
                            event.Skip();
                        }
            */
            event.Skip();
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

a2dDrawingFrame::a2dDrawingFrame( double x, double y, double width, double height ): a2dPolygonLClipper2()
{
    Clear();
    AddPoint( 0, 0 );
    AddPoint( 0 , height );
    AddPoint( width , height );
    AddPoint( width , 0 );

    m_lworld.Translate( x, y );
}

a2dDrawingFrame::~a2dDrawingFrame()
{
}

a2dDrawingFrame::a2dDrawingFrame( const a2dDrawingFrame& other, CloneOptions options, a2dRefMap* refs )
    : a2dPolygonLClipper2( other, options, refs )
{
}

a2dObject* a2dDrawingFrame::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dDrawingFrame( *this, options, refs );
}

void a2dDrawingFrame::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    a2dPolygonLClipper2::DoRender( ic, clipparent );
}

bool a2dDrawingFrame::DoIsHitWorld( a2dIterC& WXUNUSED( ic ), a2dHitEvent& hitEvent )
{
    hitEvent.m_how = a2dHit::stock_fill;
    return true;
}

#if wxART2D_USE_CVGIO
void a2dDrawingFrame::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dPolygonLClipper2::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dDrawingFrame::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dPolygonLClipper2::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

