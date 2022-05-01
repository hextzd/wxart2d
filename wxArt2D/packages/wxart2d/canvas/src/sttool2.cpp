/*! \file editor/src/sttool2.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: sttool2.cpp,v 1.36 2009/08/20 18:59:15 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/sttool.h"

const a2dCommandId a2dDrawPolygonLTool::COMID_PushTool_DrawPolygonL( wxT( "PushTool_DrawPolygonL" ) );
const a2dCommandId a2dDrawPolygonLTool::COMID_PushTool_DrawPolygonL_Splined( wxT( "PushTool_DrawPolygonL_Splined" ) );
const a2dCommandId a2dDrawPolylineLTool::COMID_PushTool_DrawPolylineL( wxT( "PushTool_DrawPolylineL" ) );
const a2dCommandId a2dDrawPolylineLTool::COMID_PushTool_DrawPolylineL_Splined( wxT( "PushTool_DrawPolylineL_Splined" ) );
const a2dCommandId a2dDrawWirePolylineLTool::COMID_PushTool_DrawWirePolylineL( wxT( "PushTool_DrawWirePolylineL" ) );

//-----------------------------------------------------------
// a2dStToolEvtHandler
//-----------------------------------------------------------

BEGIN_EVENT_TABLE( a2dStToolEvtHandler, a2dToolEvtHandler )
    EVT_KEY_DOWN( a2dStToolEvtHandler::OnKeyDown )
    EVT_KEY_UP( a2dStToolEvtHandler::OnKeyUp )
    EVT_COM_EVENT( a2dStToolEvtHandler::OnComEvent )
    EVT_DO( a2dStToolEvtHandler::OnDoEvent )
    EVT_UNDO( a2dStToolEvtHandler::OnUndoEvent )
    EVT_REDO( a2dStToolEvtHandler::OnRedoEvent )
    EVT_IDLE( a2dStToolEvtHandler::OnIdle )
END_EVENT_TABLE()

a2dStToolEvtHandler::a2dStToolEvtHandler( a2dStToolContr* controller )
{
    m_stcontroller = controller;
    //a2dCanvasCommandProcessor* drawingCmdh = m_stcontroller->GetDrawingPart()->GetDrawing()->GetCanvasCommandProcessor();
    m_options = a2dTakeToolstyleFromCmdhOnPop | a2dTakeToolstyleFromCmdhOnPush | a2dTakeToolstyleToNewObject | a2dTakeToolstyleToCentral;
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );

    // to recieve central style changes
/*  already received via view
    if ( m_stcontroller->GetDrawingPart() && m_stcontroller->GetDrawingPart()->GetShowObject() )
    {
        drawingCmdh->ConnectEvent( wxEVT_DO, this );
        drawingCmdh->ConnectEvent( wxEVT_UNDO, this );
        drawingCmdh->ConnectEvent( wxEVT_REDO, this );
        drawingCmdh->ConnectEvent( a2dEVT_COM_EVENT, this );
    }
*/
}

a2dStToolEvtHandler::~a2dStToolEvtHandler()
{
    //a2dCanvasCommandProcessor* drawingCmdh = m_stcontroller->GetDrawingPart()->GetDrawing()->GetCanvasCommandProcessor();

    a2dCanvasGlobals->DisconnectEvent( a2dEVT_COM_EVENT, this );

/*  already received via view
    if ( m_stcontroller->GetDrawingPart() && m_stcontroller->GetDrawingPart()->GetDrawing() )
    {
        drawingCmdh->DisconnectEvent( wxEVT_DO, this );
        drawingCmdh->DisconnectEvent( wxEVT_UNDO, this );
        drawingCmdh->DisconnectEvent( wxEVT_REDO, this );
        drawingCmdh->DisconnectEvent( a2dEVT_COM_EVENT, this );
    }
*/
}

void a2dStToolEvtHandler::OnKeyDown( wxKeyEvent& event )
{
    a2dBaseTool* first = m_stcontroller->GetFirstTool();
    switch( event.GetKeyCode() )
    {
        case 'Z':
        case 'z':
        {
            if  ( !first || ( first && !first->GetBusy() ) )
            {
                if ( event.m_controlDown && event.m_shiftDown )
                    first->GetCanvasCommandProcessor()->Redo();
                else if ( event.m_controlDown )
                    first->GetCanvasCommandProcessor()->Undo();
                else
                    event.Skip();
            }
            else
                event.Skip();
            break;
        }
        case 'Y':
        case 'y':
        {
            if  ( !first || ( first && !first->GetBusy() ) )
            {
                if ( event.m_controlDown )
                    first->GetCanvasCommandProcessor()->Redo();
                else
                    event.Skip();
            }
            else
                event.Skip();
            break;
        }
        default:
            event.Skip();
    }
}

void a2dStToolEvtHandler::OnKeyUp( wxKeyEvent& event )
{
    event.Skip();
}

void a2dStToolEvtHandler::TakeToCmdhFromEditedObject( a2dObjectEditTool* editTool )
{
    a2dHabitat* habitat = editTool->GetDrawing()->GetHabitat();

    editTool->SetLayer( editTool->m_original->GetLayer() );
    editTool->SetFill( editTool->m_original->GetFill() );
    editTool->SetStroke( editTool->m_original->GetStroke() );
    habitat->SetFill( editTool->m_original->GetFill() );
    habitat->SetStroke( editTool->m_original->GetStroke() );
    habitat->SetLayer( editTool->m_original->GetLayer() );

    if ( wxDynamicCast( editTool->m_original.Get(), a2dText ) )
    {
        a2dText* text = wxDynamicCast( editTool->m_original.Get(), a2dText );
        if ( text )
            habitat->SetTextFont( text->GetFont() );
    }
    if ( wxDynamicCast( editTool->m_original.Get(), a2dPolylineL ) )
    {
        a2dPolylineL* poly = wxDynamicCast( editTool->m_original.Get(), a2dPolylineL );
        if ( poly )
        {
            //habitat->SetSpline( poly->GetSpline() );
            habitat->SetLineBegin( poly->GetBegin() );
            habitat->SetLineEnd( poly->GetEnd() );
            habitat->SetEndScaleX( poly->GetEndScaleX() );
            habitat->SetEndScaleY( poly->GetEndScaleY() );
        }
    }
    if ( wxDynamicCast( editTool->m_original.Get(), a2dEndsLine ) )
    {
        a2dEndsLine* line = wxDynamicCast( editTool->m_original.Get(), a2dEndsLine );
        if ( line )
        {
            habitat->SetLineBegin( line->GetBegin() );
            habitat->SetLineEnd( line->GetEnd() );
            habitat->SetEndScaleX( line->GetEndScaleX() );
            habitat->SetEndScaleY( line->GetEndScaleY() );
        }
    }
    if ( editTool->m_original && editTool->m_original->GetContourWidth() != habitat->GetContourWidth() / editTool->GetDrawing()->GetUnitsScale() )
    {
        habitat->SetContourWidth( editTool->m_original->GetContourWidth() * editTool->GetDrawing()->GetUnitsScale() );
        editTool->m_contourwidth = editTool->m_original->GetContourWidth();
        editTool->m_original->SetContourWidth( editTool->m_contourwidth );
    }
    a2dPolylineL* poly = wxDynamicCast( editTool->m_original.Get(), a2dPolylineL );
    if ( poly && poly->GetPathType() != habitat->GetPathType() )
        habitat->SetPathType( poly->GetPathType() );
    a2dSLine* line = wxDynamicCast( editTool->m_canvasobject.Get(), a2dSLine );
    if ( line && line->GetPathType() != habitat->GetPathType() )
        habitat->SetPathType( line->GetPathType() );
}

void a2dStToolEvtHandler::OnIdle( wxIdleEvent& event )
{
    /*
        static wxDateTime lastTimeHere = wxDateTime::UNow();

        if ( lastTimeHere + wxTimeSpan::Milliseconds( 100 ) > wxDateTime::UNow() )
            return;

        lastTimeHere = wxDateTime::UNow();

        a2dBaseTool* tool = m_stcontroller->GetFirstTool();
        if ( tool->GetActive() && tool->GetEvtHandlerEnabled() && tool->GetDrawingPart())
        {
            tool->SetEvtHandlerEnabled(false);
        }
        tool->SetEvtHandlerEnabled(true);
    */
    event.Skip();
}

void a2dStToolEvtHandler::OnComEvent( a2dComEvent& event )
{
	a2dSignal comId = event.GetId();

    if ( event.GetId() == a2dBaseTool::sig_toolBeforePush )
    {
        a2dNamedProperty* prop = event.GetProperty();
        a2dStTool* tool = wxDynamicCast( prop->GetRefObject(), a2dStTool ); //tool pushed
        if ( tool->GetDeselectAtStart() &&
             !wxDynamicCast( tool, a2dObjectEditTool )
           )
        {
            a2dCanvasObjectList* objects = tool->GetParentObject()->GetChildObjectList();
            objects->SetSpecificFlags( true, a2dCanvasOFlags::PENDING, "", a2dCanvasOFlags::SELECTED );
            objects->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED );
            tool->GetParentObject()->GetRoot()->SetUpdatesPending( true );
        }
        return;
    }

    bool baseContinue = true;
    if ( comId == a2dRecursiveEditTool::sig_toolStartEditObject && ( m_options & a2dTakeToolstyleToCentral ))
    {
        a2dObjectEditTool* tool = wxDynamicCast( event.GetEventObject(), a2dObjectEditTool );
        if ( tool && tool->GetActive() && tool->GetEvtHandlerEnabled() && tool->GetDrawingPart() )
        {
            tool->SetEvtHandlerEnabled( false );
            TakeToCmdhFromEditedObject( tool );

            //! this is used by dialogs and such to takeover style.
            tool->GetDrawing()->ProcessEvent( event );

            tool->SetEvtHandlerEnabled( true );
        }
    }


    
    if ( wxDynamicCast( event.GetEventObject(), a2dDrawWirePolylineLTool ) )
    {
        if (  m_options & a2dTakeToolstyleFromCmdhOnPush  && event.GetId() == a2dStTool::sig_toolPushed )
        {
            a2dBaseTool* tool = wxStaticCast( event.GetEventObject(), a2dStTool );
        }
        if ( event.GetId() == a2dStTool::sig_toolPoped )
        {
            a2dBaseTool* tool = wxStaticCast( event.GetEventObject(), a2dStTool );
        }
        baseContinue = false;
    }
    else if ( wxDynamicCast( event.GetEventObject(), a2dDrawPolygonLTool ) )
    {
        a2dDrawPolygonLTool* tool = wxStaticCast( event.GetEventObject(), a2dDrawPolygonLTool );
        a2dHabitat* habitat = tool->GetDrawing()->GetHabitat();
        if (  m_options & a2dTakeToolstyleFromCmdhOnPush && event.GetId() == a2dStTool::sig_toolPushed )
        {
            habitat->SetSpline( tool->GetSpline() );
        }
        if ( m_options & a2dTakeToolstyleToCmdhOnPush &&  event.GetId() == a2dStTool::sig_toolPushed )
        {
            habitat->SetSpline( tool->GetSpline() );
        }
    }
    else if ( wxDynamicCast( event.GetEventObject(), a2dDrawPolylineLTool ) )
    {
        if ( m_options & a2dTakeToolstyleFromCmdhOnPush &&  event.GetId() == a2dStTool::sig_toolPushed )
        {
            a2dDrawPolylineLTool* tool = wxStaticCast( event.GetEventObject(), a2dDrawPolylineLTool );
            a2dHabitat* habitat = tool->GetDrawing()->GetHabitat();

            tool->SetSpline( habitat->GetSpline() );
            tool->SetLineBegin( habitat->GetLineBegin() );
            tool->SetLineEnd( habitat->GetLineEnd() );
            tool->SetEndScaleX( habitat->GetEndScaleX() );
            tool->SetEndScaleY( habitat->GetEndScaleY() );
        }
        if ( m_options & a2dTakeToolstyleToCmdhOnPush &&  event.GetId() == a2dStTool::sig_toolPushed )
        {
            a2dDrawPolylineLTool* tool = wxStaticCast( event.GetEventObject(), a2dDrawPolylineLTool );
            a2dHabitat* habitat = tool->GetDrawing()->GetHabitat();

            habitat->SetSpline( tool->GetSpline() );
            habitat->SetLineBegin( tool->GetLineBegin() );
            habitat->SetLineEnd( tool->GetLineEnd() );
            habitat->SetEndScaleX( tool->GetEndScaleX()  );
            habitat->SetEndScaleY( tool->GetEndScaleY()  );
        }
    }
    else if ( wxDynamicCast( event.GetEventObject(), a2dDrawLineTool ) )
    {
        if (  m_options & a2dTakeToolstyleFromCmdhOnPush && event.GetId() == a2dStTool::sig_toolPushed )
        {
            a2dDrawLineTool* tool = wxStaticCast( event.GetEventObject(), a2dDrawLineTool );
            a2dHabitat* habitat = tool->GetDrawing()->GetHabitat();

            tool->SetLineBegin( habitat->GetLineBegin() );
            tool->SetLineEnd( habitat->GetLineEnd() );
            tool->SetEndScaleX( habitat->GetEndScaleX() );
            tool->SetEndScaleY( habitat->GetEndScaleY() );
        }
        if ( m_options & a2dTakeToolstyleToCmdhOnPush &&  event.GetId() == a2dStTool::sig_toolPushed )
        {
            a2dDrawPolylineLTool* tool = wxStaticCast( event.GetEventObject(), a2dDrawPolylineLTool );
            a2dHabitat* habitat = tool->GetDrawing()->GetHabitat();

            habitat->SetLineBegin( tool->GetLineBegin() );
            habitat->SetLineEnd( tool->GetLineEnd() );
            habitat->SetEndScaleX( tool->GetEndScaleX()  );
            habitat->SetEndScaleY( tool->GetEndScaleY()  );
        }
    }
    else if ( wxDynamicCast( event.GetEventObject(), a2dDrawTextTool ) )
    {
        a2dDrawTextTool* tool = wxStaticCast( event.GetEventObject(), a2dDrawTextTool );
        a2dHabitat* habitat = tool->GetDrawing()->GetHabitat();
        if (  m_options & a2dTakeToolstyleFromCmdhOnPush && event.GetId() == a2dStTool::sig_toolPushed )
        {
            a2dFont globfont = habitat->GetTextTemplateObject()->GetFont();
            tool->GetTemplateObject()->SetFont( globfont );
        }
        if ( m_options & a2dTakeToolstyleToCmdhOnPush &&  event.GetId() == a2dStTool::sig_toolPushed )
        {
            habitat->SetTextFont( tool->GetTemplateObject()->GetFont() );
        }
        if (  m_options & a2dTakeToolstyleFromCmdhOnPop && event.GetId() == a2dStTool::sig_toolPoped )
        {
            tool->GetTemplateObject()->SetFont( habitat->GetTextTemplateObject()->GetFont() );
        }
        baseContinue = true;
    }
    else if ( wxDynamicCast( event.GetEventObject(), a2dRecursiveEditTool ) )
    {
        baseContinue = false;
    }
    else if ( wxDynamicCast( event.GetEventObject(), a2dObjectEditTool ) )
    {
        baseContinue = false;
    }
    else if ( wxDynamicCast( event.GetEventObject(), a2dDragNewTool ) )
    {
        baseContinue = false;
    }


    if ( baseContinue &&
         ( wxDynamicCast( event.GetEventObject(), a2dStDrawTool )
         )
       )
    {
        a2dStDrawTool* tool = wxDynamicCast( event.GetEventObject(), a2dStDrawTool );
        a2dHabitat* habitat = tool->GetDrawing()->GetHabitat();
        a2dDrawTextTool* textTool = wxDynamicCast( tool, a2dDrawTextTool );
        //Most Drawing tool takes style of the central global which may have changed by other
        //drawing tools or style commands etc.
		// Texttool has his own stroke and fill style based on the text template.
        if ( textTool )
        {
            if ( (m_options & a2dTakeToolstyleToCmdhOnPush) && event.GetId() == a2dStTool::sig_toolPushed )
            {
                habitat->GetTextTemplateObject()->SetFill( tool->GetFill() );
                habitat->GetTextTemplateObject()->SetStroke( tool->GetStroke() );
                habitat->GetTextTemplateObject()->SetLayer( tool->GetLayer() );
            }
            if ( (m_options & a2dTakeToolstyleFromCmdhOnPush) && event.GetId() == a2dStTool::sig_toolPushed )
            {
                a2dBaseTool::PROPID_Fill->SetPropertyToObject( tool,  habitat->GetTextTemplateObject()->GetFill() );
                a2dBaseTool::PROPID_Stroke->SetPropertyToObject( tool, habitat->GetTextTemplateObject()->GetStroke() );
                a2dBaseTool::PROPID_Layer->SetPropertyToObject( tool, habitat->GetTextTemplateObject()->GetLayer() );
            }
            if ( m_options & a2dTakeToolstyleFromCmdhOnPop && event.GetId() == a2dStTool::sig_toolPoped )
            {
                a2dBaseTool::PROPID_Fill->SetPropertyToObject( tool,  habitat->GetTextTemplateObject()->GetFill() );
                a2dBaseTool::PROPID_Stroke->SetPropertyToObject( tool, habitat->GetTextTemplateObject()->GetStroke() );
                a2dBaseTool::PROPID_Layer->SetPropertyToObject( tool, habitat->GetTextTemplateObject()->GetLayer() );
            }
            if ( event.GetId() == a2dStTool::sig_toolPoped )
            {
                tool->CloseCommandGroup();
            }
            if ( (m_options & a2dTakeToolstyleToNewObject) && event.GetId() == a2dStTool::sig_toolComEventAddObject )
            {
                a2dProperty *property = (a2dProperty *) event.GetProperty();
                a2dCanvasObject* obj = wxStaticCast( property->GetValue(), a2dCanvasObject );

                if ( obj->GetFilled() ) 
            	    a2dCanvasObject::PROPID_Fill->SetPropertyToObject( obj,  tool->GetFill() );
                a2dCanvasObject::PROPID_Stroke->SetPropertyToObject( obj, tool->GetStroke() );
                a2dCanvasObject::PROPID_Layer->SetPropertyToObject( obj, tool->GetLayer() );
            }
        }
        else
        {
            if ( (m_options & a2dTakeToolstyleToCmdhOnPush) && event.GetId() == a2dStTool::sig_toolPushed )
            {
                habitat->SetFill( tool->GetFill() );
                habitat->SetStroke( tool->GetStroke() );
                habitat->SetLayer( tool->GetLayer() );
            }
            if ( (m_options & a2dTakeToolstyleFromCmdhOnPush) && event.GetId() == a2dStTool::sig_toolPushed )
            {
                a2dBaseTool::PROPID_Fill->SetPropertyToObject( tool,  habitat->GetFill() );
                a2dBaseTool::PROPID_Stroke->SetPropertyToObject( tool, habitat->GetStroke() );
                a2dBaseTool::PROPID_Layer->SetPropertyToObject( tool, habitat->GetLayer() );
                tool->SetContourWidth( habitat->GetContourWidth() / tool->GetDrawing()->GetUnitsScale() );
            }
            if ( m_options & a2dTakeToolstyleFromCmdhOnPop && event.GetId() == a2dStTool::sig_toolPoped )
            {
                a2dBaseTool::PROPID_Fill->SetPropertyToObject( tool,  habitat->GetFill() );
                a2dBaseTool::PROPID_Stroke->SetPropertyToObject( tool, habitat->GetStroke() );
                a2dBaseTool::PROPID_Layer->SetPropertyToObject( tool, habitat->GetLayer() );
            }
            if ( event.GetId() == a2dStTool::sig_toolPoped )
            {
                tool->CloseCommandGroup();
            }
            if ( (m_options & a2dTakeToolstyleToNewObject) && event.GetId() == a2dStTool::sig_toolComEventAddObject )
            {
                a2dProperty *property = (a2dProperty *) event.GetProperty();
                a2dCanvasObject* obj = wxStaticCast( property->GetValue(), a2dCanvasObject );

                if ( obj->GetFilled() ) 
            	    a2dCanvasObject::PROPID_Fill->SetPropertyToObject( obj,  tool->GetFill() );
                a2dCanvasObject::PROPID_Stroke->SetPropertyToObject( obj, tool->GetStroke() );
                a2dCanvasObject::PROPID_Layer->SetPropertyToObject( obj, tool->GetLayer() );
            }
        }
    }  

    a2dStTool* tool = wxDynamicCast( m_stcontroller->GetFirstTool(), a2dStTool );

    if ( tool && tool->GetActive() && tool->GetEvtHandlerEnabled() && tool->GetDrawingPart() )
    {
        a2dHabitat* habitat = tool->GetDrawing()->GetHabitat();
        if ( ( wxDynamicCast( tool, a2dStDrawTool ) || wxDynamicCast( tool, a2dObjectEditTool ) ) &&
                ( comId == a2dHabitat::sig_changedFill ||
                  comId == a2dHabitat::sig_changedStroke ||
                  comId == a2dHabitat::sig_changedLayer ||
                  comId == a2dHabitat::sig_changedFont ||
                  comId == a2dHabitat::sig_changedTextFill ||
                  comId == a2dHabitat::sig_changedTextStroke ||
                  comId == a2dHabitat::sig_changedTextFont
                )
           )
        {
            //drawing tools set the style to the object that is drawn.
            a2dStTool* sttool = wxDynamicCast( tool, a2dStTool );
            a2dStDrawTool* drawtool = wxDynamicCast( tool, a2dStDrawTool );
			a2dDrawTextTool* textTool = wxDynamicCast( tool, a2dDrawTextTool );
            a2dObjectEditTool* editTool = wxDynamicCast( tool, a2dObjectEditTool );
            a2dCanvasCommandProcessor* docCmdh = tool->GetCanvasCommandProcessor();

            if ( !textTool )
            {
				if ( editTool && editTool->GetBusy() )
				{	
					a2dText* text = wxDynamicCast( editTool->m_original.Get() , a2dText );
					if ( text )
					{
						a2dText* textcopy = wxStaticCast( editTool->m_canvasobject.Get() , a2dText );
						text->SetFont( habitat->GetTextFont() );
						text->SetStroke( habitat->GetTextStroke() );
						text->SetFill( habitat->GetTextFill() );
						textcopy->SetFont( habitat->GetTextFont() );
						textcopy->SetStroke( habitat->GetTextStroke() );
						textcopy->SetFill( habitat->GetTextFill() );
					}
                    else
                    {
						a2dCanvasObject* original = editTool->m_original;
						a2dCanvasObject* copy = editTool->m_canvasobject;
						original->SetStroke( habitat->GetStroke() );
						original->SetFill( habitat->GetFill() );
						copy->SetStroke( habitat->GetStroke() );
						copy->SetFill( habitat->GetFill() );
                    }
                }
                else
                {
                    a2dFill f = habitat->GetFill();
                    if ( comId == a2dHabitat::sig_changedFill && ! tool->GetFill().IsSameAs( f ) )
                    {
                        tool->SetFill( f );
                        if ( tool->GetBusy() )
                            docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( sttool->m_original, f ) );
                    }
                    a2dStroke s = habitat->GetStroke();
                    if ( comId == a2dHabitat::sig_changedStroke && ! tool->GetStroke().IsSameAs( s ) )
                    {
                        tool->SetStroke( s );
                        if ( tool->GetBusy() )
                            docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( sttool->m_original, s ) );
                    }
                }
            }
			else //textTool
			{
				a2dFont font = habitat->GetTextFont();
				if ( ! textTool->GetTemplateObject()->GetFont().IsSameAs( font ) )
				{
					textTool->GetTemplateObject()->SetFont( habitat->GetTextFont() );
				}
                //special stroke and fill for text objects.
                a2dFill f = habitat->GetTextFill();
                if ( comId == a2dHabitat::sig_changedTextFill && ! tool->GetFill().IsSameAs( f ) )
                {
                    tool->SetFill( f );
                    if ( tool->GetBusy() )
                        docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( sttool->m_original, f ) );
                }
                a2dStroke s = habitat->GetTextStroke();
                if ( comId == a2dHabitat::sig_changedTextStroke && ! tool->GetStroke().IsSameAs( s ) )
                {
                    tool->SetStroke( s );
                    if ( tool->GetBusy() )
                        docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( sttool->m_original, s ) );
                }

                a2dFill cf = habitat->GetFill();
                if ( comId == a2dHabitat::sig_changedFill && ! tool->GetFill().IsSameAs( cf ) )
                {
                    tool->SetFill( cf );
                    if ( tool->GetBusy() )
                        docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( sttool->m_original, cf ) );
                }
                a2dStroke cs = habitat->GetStroke();
                if ( comId == a2dHabitat::sig_changedStroke && ! tool->GetStroke().IsSameAs( cs ) )
                {
                    tool->SetStroke( cs );
                    if ( tool->GetBusy() )
                        docCmdh->Submit( new a2dCommand_ChangeCanvasObjectStyle( sttool->m_original, cs ) );
                }

			}

            if ( comId == a2dHabitat::sig_changedLayer && tool->GetLayer() != habitat->GetLayer() )
            {
                tool->SetLayer( habitat->GetLayer() );
                if ( drawtool && drawtool->m_original )
                {
                    drawtool->m_original->SetLayer( habitat->GetLayer() );
                }
                if ( editTool && editTool->m_original )
                {
                    editTool->m_original->SetLayer( habitat->GetLayer() );
                }
            }
            if ( comId == a2dHabitat::sig_changedFont )
            {
				if ( editTool && editTool->GetBusy() )
				{	
					a2dText* text = wxDynamicCast( editTool->m_original.Get() , a2dText );
					if ( text )
					{
						a2dText* textcopy = wxStaticCast( editTool->m_canvasobject.Get() , a2dText );
						text->SetFont( habitat->GetFont() );
						textcopy->SetFont( habitat->GetFont() );
					}					
                }
            }
        }
        else if ( comId == a2dHabitat::sig_changedTextFont && wxDynamicCast( tool, a2dDrawTextTool ) )
        {
            a2dDrawTextTool* textTool = wxDynamicCast( tool, a2dDrawTextTool );
            a2dHabitat* habitat = textTool->GetDrawing()->GetHabitat();
            a2dFont font = habitat->GetTextFont();
            if ( ! textTool->GetTemplateObject()->GetFont().IsSameAs( font ) )
            {
                textTool->GetTemplateObject()->SetFont( habitat->GetTextFont() );
            }
        }
        else if ( wxDynamicCast( tool, a2dDrawPolylineLTool ) )
        {
            a2dDrawPolylineLTool* toolt = wxStaticCast( tool, a2dDrawPolylineLTool );
            a2dHabitat* habitat = toolt->GetDrawing()->GetHabitat();

            if ( toolt->GetAllowModifyTemplate() )
            {
				if ( toolt->GetSpline() != habitat->GetSpline() )
					toolt->SetSpline( habitat->GetSpline() );
				if ( toolt->GetLineBegin() != habitat->GetLineBegin() )
					toolt->SetLineBegin( habitat->GetLineBegin() );
				if ( toolt->GetLineEnd() != habitat->GetLineEnd() )
					toolt->SetLineEnd( habitat->GetLineEnd() );
				if ( toolt->GetEndScaleX() != habitat->GetEndScaleX() )
					toolt->SetEndScaleX( habitat->GetEndScaleX() );
				if ( toolt->GetEndScaleY() != habitat->GetEndScaleY() )
					toolt->SetEndScaleY( habitat->GetEndScaleY() );
				toolt->SetContourWidth( habitat->GetContourWidth() / toolt->GetDrawing()->GetUnitsScale() );
            }
        }
        else if ( wxDynamicCast( tool, a2dDrawLineTool ) )
        {
            a2dDrawLineTool* toolt = wxStaticCast( tool, a2dDrawLineTool );
            a2dHabitat* habitat = toolt->GetDrawing()->GetHabitat();

            if ( toolt->GetAllowModifyTemplate() )
            {
                if ( toolt->GetLineBegin() != habitat->GetLineBegin() )
                    toolt->SetLineBegin( habitat->GetLineBegin() );
                if ( toolt->GetLineEnd() != habitat->GetLineEnd() )
                    toolt->SetLineEnd( habitat->GetLineEnd() );
                if ( toolt->GetEndScaleX() != habitat->GetEndScaleX() )
                    toolt->SetEndScaleX( habitat->GetEndScaleX() );
                if ( toolt->GetEndScaleY() != habitat->GetEndScaleY() )
                    toolt->SetEndScaleY( habitat->GetEndScaleY() );
                toolt->SetContourWidth( habitat->GetContourWidth() / toolt->GetDrawing()->GetUnitsScale() );
            }
        }
        /*
                if ( event.GetId() == &a2dComEvent::sm_changedProperty && *(m_propertiesTakeFromCommandProc.Find( (a2dPropertyId*) &(event.GetProperty()->GetId()) )) )
                {
                    event.GetProperty()->SetToObjectClone( this );
                    tool->SetPending(true);
                }
                if ( tool->GetBusy() && event.GetId() == &a2dComEvent::sm_changedProperty && *(m_propertiesSetToOriginalObject.Find( (a2dPropertyId*)  &(event.GetProperty()->GetId()) )) )
                {
                    if ( tool->m_original )
                        event.GetProperty()->SetToObjectClone( tool->m_original );

                    tool->SetPending(true);
                }
                if ( tool->GetBusy() && event.GetId() == &a2dComEvent::sm_changedProperty && *(m_propertiesSetToEditObject.Find( (a2dPropertyId*)  &(event.GetProperty()->GetId()) )) )
                {
                    if ( tool->m_canvasobject )
                        event.GetProperty()->SetToObjectClone( tool->m_canvasobject );

                    tool->SetPending(true);
                }
        */
        event.Skip();

    }
    event.Skip();

}

void a2dStToolEvtHandler::OnDoEvent( a2dCommandProcessorEvent& event )
{
    a2dBaseTool* tool = m_stcontroller->GetFirstTool();
    const a2dCommandId* comId = event.GetCommand()->GetCommandId();
    if ( tool && tool->GetActive() && tool->GetEvtHandlerEnabled() && tool->GetDrawingPart() )
    {
        if ( ( wxDynamicCast( tool, a2dStDrawTool ) || wxDynamicCast( tool, a2dObjectEditTool ) ) )
        {

            //drawing tools set the style to the object that is drawn.
            a2dStTool* sttool = wxDynamicCast( tool, a2dStTool );
            a2dStDrawTool* drawtool = wxDynamicCast( tool, a2dStDrawTool );
            a2dObjectEditTool* editTool = wxDynamicCast( tool, a2dObjectEditTool );
            a2dCanvasCommandProcessor* docCmdh = tool->GetCanvasCommandProcessor();
		}
	}
    event.Skip();
}

void a2dStToolEvtHandler::OnUndoEvent( a2dCommandProcessorEvent& event )
{
    a2dBaseTool* tool = m_stcontroller->GetFirstTool();
    if ( tool && tool->GetActive() && tool->GetEvtHandlerEnabled() && tool->GetDrawingPart() )
    {
        if ( wxDynamicCast( tool, a2dObjectEditTool ) )
        {
        }
    }
    event.Skip();

}

void a2dStToolEvtHandler::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    OnUndoEvent( event );
    event.Skip();
}

//-----------------------------------------------------------
// a2dStToolFixedToolStyleEvtHandler
//-----------------------------------------------------------

BEGIN_EVENT_TABLE( a2dStToolFixedToolStyleEvtHandler, a2dToolEvtHandler )
    EVT_KEY_DOWN( a2dStToolFixedToolStyleEvtHandler::OnKeyDown )
    EVT_KEY_UP( a2dStToolFixedToolStyleEvtHandler::OnKeyUp )
    EVT_COM_EVENT( a2dStToolFixedToolStyleEvtHandler::OnComEvent )
    EVT_DO( a2dStToolFixedToolStyleEvtHandler::OnDoEvent )
    EVT_UNDO( a2dStToolFixedToolStyleEvtHandler::OnUndoEvent )
    EVT_REDO( a2dStToolFixedToolStyleEvtHandler::OnRedoEvent )
END_EVENT_TABLE()

a2dStToolFixedToolStyleEvtHandler::a2dStToolFixedToolStyleEvtHandler( a2dStToolContr* controller )
{
    m_stcontroller = controller;
    m_takeToolstyleToCmdhOnPush = false;
}

a2dStToolFixedToolStyleEvtHandler::~a2dStToolFixedToolStyleEvtHandler()
{
}

void a2dStToolFixedToolStyleEvtHandler::OnKeyDown( wxKeyEvent& event )
{
    a2dBaseTool* first = m_stcontroller->GetFirstTool();
    switch( event.GetKeyCode() )
    {
        case 'Z':
        case 'z':
        {
            if  ( !first || ( first && !first->GetBusy() ) )
            {
                if ( event.m_controlDown && event.m_shiftDown )
                    first->GetCanvasCommandProcessor()->Redo();
                else if ( event.m_controlDown )
                    first->GetCanvasCommandProcessor()->Undo();
                else
                    event.Skip();
            }
            else
                event.Skip();
            break;
        }
        case 'Y':
        case 'y':
        {
            if  ( !first || ( first && !first->GetBusy() ) )
            {
                if ( event.m_controlDown )
                    first->GetCanvasCommandProcessor()->Redo();
                else
                    event.Skip();
            }
            else
                event.Skip();
            break;
        }
        default:
            event.Skip();
    }
}

void a2dStToolFixedToolStyleEvtHandler::OnKeyUp( wxKeyEvent& event )
{
    event.Skip();
}

void a2dStToolFixedToolStyleEvtHandler::OnComEvent( a2dComEvent& event )
{
    if ( event.GetId() == a2dBaseTool::sig_toolBeforePush )
    {
        a2dNamedProperty* prop = event.GetProperty();
        a2dStTool* tool = wxDynamicCast( prop->GetRefObject(), a2dStTool ); //tool pushed
        //Tools which work on a selection should not deselect first.
        if ( tool->GetDeselectAtStart() )
        {
            a2dCanvasObjectList* objects = tool->GetParentObject()->GetChildObjectList();
            if ( objects->Find( "", "", a2dCanvasOFlags::SELECTED ) )
            {
                objects->SetSpecificFlags( true, a2dCanvasOFlags::PENDING, "", a2dCanvasOFlags::SELECTED );
                objects->SetSpecificFlags( true, a2dCanvasOFlags::PENDING, "", a2dCanvasOFlags::SELECTED2 );
                objects->SetSpecificFlags( false, a2dCanvasOFlags::SELECTED | a2dCanvasOFlags::SELECTED2 );
                tool->GetParentObject()->GetRoot()->SetUpdatesPending( true );
            }
        }
        return;
    }

    bool baseContinue = true;
    if ( baseContinue && wxDynamicCast( event.GetEventObject(), a2dStDrawTool ) )
    {
        //Most Drawing tool takes style of the commandprocessor which may have change by other
        //drawing tools or style commands etc.
        a2dStDrawTool* tool = wxDynamicCast( event.GetEventObject(), a2dStDrawTool );
        if ( m_takeToolstyleToCmdhOnPush && event.GetId() == a2dStTool::sig_toolPushed )
        {
            a2dHabitat* habitat = tool->GetDrawing()->GetHabitat();
            habitat->SetFill( tool->GetFill() );
            habitat->SetStroke( tool->GetStroke() );
        }
        if ( event.GetId() == a2dStTool::sig_toolPoped )
        {
            tool->CloseCommandGroup();
        }
        if ( event.GetId() == a2dStTool::sig_toolComEventAddObject )
        {
            a2dProperty* property = ( a2dProperty* ) event.GetProperty();
            a2dCanvasObject* obj = wxStaticCast( property->GetValue(), a2dCanvasObject );

            //style can change in tool while busy, so set it to object one more.
            a2dCanvasObject::PROPID_Fill->SetPropertyToObject( obj,  tool->GetFill() );
            a2dCanvasObject::PROPID_Stroke->SetPropertyToObject( obj, tool->GetStroke() );
        }
    }
    event.Skip();
}

void a2dStToolFixedToolStyleEvtHandler::OnDoEvent( a2dCommandProcessorEvent& event )
{
    /*
    a2dBaseTool* tool = m_stcontroller->GetFirstTool();
    if ( tool->GetActive() && tool->GetEvtHandlerEnabled() && tool->GetDrawingPart())
    {
    }
    */
    event.Skip();
}

void a2dStToolFixedToolStyleEvtHandler::OnUndoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();

}

void a2dStToolFixedToolStyleEvtHandler::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}


BEGIN_EVENT_TABLE( a2dDrawPolygonLTool, a2dStDrawTool )
    EVT_IDLE( a2dDrawPolygonLTool::OnIdle )
    EVT_CHAR( a2dDrawPolygonLTool::OnChar )
    EVT_MOUSE_EVENTS( a2dDrawPolygonLTool::OnMouseEvent )
    EVT_DO( a2dDrawPolygonLTool::OnDoEvent )
    EVT_UNDO( a2dDrawPolygonLTool::OnUndoEvent )
    EVT_REDO( a2dDrawPolygonLTool::OnRedoEvent )
    EVT_CANUNDO( a2dDrawPolygonLTool::OnCanUndoEvent )
    EVT_CANREDO( a2dDrawPolygonLTool::OnCanRedoEvent )
END_EVENT_TABLE()

void a2dDrawPolygonLTool::AddPoint( a2dLineSegment* point )
{
    if ( m_reverse )
        m_points->push_front( point );
    else
        m_points->push_back( point );
}

a2dDrawPolygonLTool::a2dDrawPolygonLTool( a2dStToolContr* controller, a2dPolygonL* templateObject ): a2dStDrawTool( controller )
{
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );
    m_contourwidth = GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale();

    m_canvasobject = ( a2dCanvasObject* )0;
    m_points = ( a2dVertexList* ) 0;

    m_reverse = false;

    m_templateObject = templateObject;
    if ( !m_templateObject )
    {
        m_templateObject = new a2dPolygonL();
        m_templateObject->SetContourWidth( GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale() );
        m_contourwidth = GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale();
        m_templateObject->SetSpline( GetDrawing()->GetHabitat()->GetSpline() );
    }
    else
    {
        m_layer = m_templateObject->GetLayer();
        m_fill = m_templateObject->GetFill();
        m_stroke = m_templateObject->GetStroke();
        m_contourwidth = m_templateObject->GetContourWidth();
    }
}

a2dDrawPolygonLTool::~a2dDrawPolygonLTool()
{
}

void a2dDrawPolygonLTool::SetSpline( bool spline )
{
    m_templateObject->SetSpline( spline );
}

void a2dDrawPolygonLTool::OnIdle( wxIdleEvent& event )
{
    if ( m_pending && GetBusy() )
    {
        RemoveAllDecorations();

        a2dPolygonL* poly = wxStaticCast( m_canvasobject.Get(), a2dPolygonL );
        a2dRect* around = NULL;
        a2dPolygonL* nonSplined = NULL;

        switch ( m_mode )
        {
            case 2:
            {
                //a nice to see normal polygon around it and if stroke transparent show it.
                if ( poly->GetSpline() )
                {
                    nonSplined = ( a2dPolygonL* ) poly->TClone( clone_childs );
                    nonSplined->SetVisible( true );
                    nonSplined->SetSpline( false );
                    nonSplined->SetFill( *a2dTRANSPARENT_FILL );

                    nonSplined->SetStroke( *wxBLACK, 0, a2dSTROKE_DOT_DASH );
                    AddDecorationObject( nonSplined );

                }
            }
            case 1:
            {
                around = new a2dRect( poly->GetBboxMinX(), poly->GetBboxMinY(), poly->GetBboxWidth(), poly->GetBboxHeight() );
                around->SetStroke( *wxRED, 0, a2dSTROKE_DOT_DASH );
                around->SetFill( *a2dTRANSPARENT_FILL );
                around->SetRoot( GetDrawing(), false );
                AddDecorationObject( around );
                break;
            }
            default:
                break;
        }

        event.Skip();
    }
    else
        event.Skip();
}

void a2dDrawPolygonLTool::BeginPolygon( double x, double y )
{
	m_xwprev = x;
	m_ywprev = y;

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
        restrict->RestrictPoint( m_xwprev, m_ywprev );
    GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
    m_original = m_templateObject->TClone( clone_childs | clone_members );
    m_original->Translate( m_xwprev, m_ywprev );
    m_original->SetRoot( GetDrawing() );
    m_original->EliminateMatrix();
    m_original->SetContourWidth( m_contourwidth );
    m_original->SetStroke( m_stroke );
    m_original->SetFill( m_fill );
    m_original->SetLayer( m_layer );

    if ( !EnterBusyMode() )
        return;
    AddCurrent( m_original );

    // Append first point to original
    GetCanvasCommandProcessor()->Submit(
        new a2dCommand_AddPoint( m_original, m_xwprev, m_ywprev, -1 )
    );
    // Append first point to editcopy
    m_points = wxStaticCast( m_canvasobject.Get(), a2dPolygonL )->GetSegments();
    m_point = new a2dLineSegment( m_xwprev, m_ywprev );
    AddPoint( m_point );

    // Record commands
    a2dGeneralGlobals->RecordF( this, _T( "polygon" ) );
    a2dGeneralGlobals->RecordF( this, _T( "addpoint %f %f" ), m_xwprev, m_ywprev );

    //add second point to drag
    m_point = new a2dLineSegment( m_xwprev, m_ywprev );
    AddPoint( m_point );

    m_prev_x = m_xwprev;
    m_prev_y = m_ywprev;
    m_pending = true;
    if( restrict )
        restrict->SetRestrictPoint( m_xwprev, m_ywprev );
}

void a2dDrawPolygonLTool::AddPoint( double x, double y )
{
	m_xwprev = x;
	m_ywprev = y;

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
	if( restrict )
		restrict->RestrictPoint( m_xwprev, m_ywprev );

	m_point->m_x = m_xwprev;
	m_point->m_y = m_ywprev;

	GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
	int xd = GetDrawer2D()->WorldToDeviceX( m_prev_x );
	int yd = GetDrawer2D()->WorldToDeviceY( m_prev_y );
	if ( abs( m_x - xd ) >= 3 || abs( m_y - yd ) >= 3 )
	{
		GetCanvasCommandProcessor()->Submit(
			new a2dCommand_AddPoint( m_original, m_xwprev, m_ywprev, -1 )
		);

		a2dGeneralGlobals->RecordF( this, _T( "addpoint %f %f " ), m_xwprev, m_ywprev );

		m_point = new a2dLineSegment( m_xwprev, m_ywprev );
		AddPoint( m_point );
		m_prev_x = m_xwprev;
		m_prev_y = m_ywprev;
	}
	m_pending = true;
	if( restrict )
		restrict->SetRestrictPoint( m_xwprev, m_ywprev );
}

void a2dDrawPolygonLTool::OnDoEvent( a2dCommandProcessorEvent& event )
{
    if ( GetBusy() && !m_halted )
    {
		if ( event.GetCommand()->GetCommandId() == &a2dCommand_SetCursor::Id )
		{
			a2dCommand_SetCursor* setcursor = wxStaticCast( event.GetCommand(), a2dCommand_SetCursor );
			AddPoint( setcursor->GetX(), setcursor->GetY() );
		}
    }
	else
	{
		if ( event.GetCommand()->GetCommandId() == &a2dCommand_SetCursor::Id )
		{
			a2dCommand_SetCursor* setcursor = wxStaticCast( event.GetCommand(), a2dCommand_SetCursor );
			BeginPolygon( setcursor->GetX(), setcursor->GetY() );
		}
	}
    event.Skip();
}

void a2dDrawPolygonLTool::OnCanUndoEvent( a2dCommandProcessorEvent& event )
{
    if ( GetBusy() )
    {
        if ( !m_halted &&  event.GetCommand()->GetCommandId() ==  &a2dCommand_AddPoint::Id )
        {
            a2dPolygonL* poly = wxStaticCast( m_original.Get(), a2dPolygonL );
            if ( poly->GetNumberOfSegments() > 1)
                event.Veto( false );
            else
                event.Veto( true );
        }
        else
        {
            event.Veto( true );
        }
    }
}

void a2dDrawPolygonLTool::OnCanRedoEvent( a2dCommandProcessorEvent& event )
{
    if ( GetBusy() )
    {
        if ( !m_halted &&  event.GetCommand()->GetCommandId() ==  &a2dCommand_AddPoint::Id )
            event.Veto( false );
        else
        {
            event.Veto( true );
        }
    }
}

void a2dDrawPolygonLTool::OnUndoEvent( a2dCommandProcessorEvent& event )
{
    if ( GetBusy() && !m_halted &&  event.GetCommand()->GetCommandId() ==  &a2dCommand_AddPoint::Id )
    {
        // recreate edit copies so the edit copy becomes updated from the original.

        a2dPolygonL* poly = wxStaticCast( m_original.Get(), a2dPolygonL );
        //if ( poly->GetNumberOfSegments() )
        {
            CleanupToolObjects();
            GetDrawingPart()->Update( a2dCANVIEW_UPDATE_OLDNEW );
            CreateToolObjects();

            m_points = wxStaticCast( m_canvasobject.Get(), a2dPolygonL )->GetSegments();

            //add second point to drag
            m_point = new a2dLineSegment( m_xwprev, m_ywprev );
            AddPoint( m_point );
        }
    }
}

void a2dDrawPolygonLTool::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    OnUndoEvent( event );
}

void a2dDrawPolygonLTool::SetMode( int mode )
{
    m_mode = mode;
    if ( m_mode > 2 ) m_mode = 0;

    if ( !m_canvasobject )
        return;

    m_pending = true;
}

void a2dDrawPolygonLTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() && m_canvasobject )
    {
        double shiftx;
        double shifty;
        GetKeyIncrement( &shiftx, &shifty );

        switch( event.GetKeyCode() )
        {
            case WXK_TAB:
            {
                m_mode++;
                SetMode( m_mode );
                return;
            }
            break;

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

        if ( event.m_controlDown )
        {
            double x = m_canvasobject->GetPosX();
            double y = m_canvasobject->GetPosY();
            m_canvasobject->SetPosXY( x + shiftx, y + shifty, true );
            a2dPolygonL* poly = wxStaticCast( m_canvasobject.Get(), a2dPolygonL );
            poly->EliminateMatrix();
        }
        else
        {
            m_point->m_x += shiftx;
            m_point->m_y += shifty;
        }

        GetDisplayWindow()->WarpPointer( GetDrawer2D()->WorldToDeviceX( m_point->m_x ), GetDrawer2D()->WorldToDeviceY( m_point->m_y ) );

        m_pending = true;
    }
    else
        event.Skip();

}

void a2dDrawPolygonLTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
        restrict->SetShiftKeyDown( event.ShiftDown() );

    if ( event.LeftDown() && !GetBusy() )
    {
		BeginPolygon( m_xwprev, m_ywprev );
    }
    else if ( event.LeftDown() && GetBusy() )
    {
		AddPoint( m_xwprev, m_ywprev );
    }
    else if ( event.Moving() && GetBusy() )
    {
        if( restrict )
            restrict->RestrictPoint( m_xwprev, m_ywprev );
        a2dBoundingBox updateareas;
        //AddPoint( m_point );

        updateareas.Expand( m_prev_x, m_prev_y );
        updateareas.Expand( m_point->GetPoint() );
        //a2dCanvasObject::PROPID_BoundingBox->SetPropertyToObject( m_canvasobject, updateareas );
        updateareas.Reset();
        updateareas.Expand( m_points->front()->GetPoint() );
        updateareas.Expand( m_point->GetPoint() );
        //a2dCanvasObject::PROPID_BoundingBox->SetPropertyToObject( m_canvasobject, updateareas );

        m_point->m_x = m_xwprev;
        m_point->m_y = m_ywprev;
        m_canvasobject->SetPending( true );

        updateareas.Reset();
        updateareas.Expand( m_prev_x, m_prev_y );
        updateareas.Expand( m_point->GetPoint() );
        //a2dCanvasObject::PROPID_BoundingBox->SetPropertyToObject( m_canvasobject, updateareas );

        updateareas.Reset();
        updateareas.Expand( m_points->front()->GetPoint() );
        updateareas.Expand( m_point->GetPoint() );
        //a2dCanvasObject::PROPID_BoundingBox->SetPropertyToObject( m_canvasobject, updateareas );

        m_pending = true;

		double dx =  m_xwprev - m_prev_x;
		double dy =  m_ywprev - m_prev_y;

	    double unitScale = GetDrawing()->GetUnitsScale();
		wxString state;
		wxString form = _("Lpx") + m_stcontroller->GetFormat() + _(" Lpy") + m_stcontroller->GetFormat();
		state.Printf( form, m_prev_x*unitScale, m_prev_y*unitScale );
		SetStateString( state, 10 );
		form = _("dx = ") + m_stcontroller->GetFormat() + _(" dy = ") + m_stcontroller->GetFormat();
		state.Printf( form, dx*unitScale, dy*unitScale );
		SetStateString( state, 11 );

        event.Skip();
    }
    else if ( ( event.LeftDClick() || event.RightUp() ) && GetBusy() )
    {
        if( restrict )
            restrict->RestrictPoint( m_xwprev, m_ywprev );
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dGeneralGlobals->RecordF( this, _T( "end" ) );
        StartEditingOrFinish();
    }
    else
        event.Skip();
}

BEGIN_EVENT_TABLE( a2dDrawPolylineLTool, a2dDrawPolygonLTool )
    EVT_MOUSE_EVENTS( a2dDrawPolylineLTool::OnMouseEvent )
    EVT_DO( a2dDrawPolylineLTool::OnDoEvent )
    EVT_COM_EVENT( a2dDrawPolylineLTool::OnComEvent )
END_EVENT_TABLE()

a2dDrawPolylineLTool::a2dDrawPolylineLTool( a2dStToolContr* controller, a2dPolylineL* templateObject ):
    a2dDrawPolygonLTool( controller, NULL )
{
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );

    m_AllowModifyTemplate = true;
    m_templatePline = templateObject;
    m_templateObject = templateObject;
    if ( !m_templatePline )
    {
        m_templatePline = new a2dPolylineL();
        m_templateObject = ( a2dPolygonL* ) m_templatePline;
        m_templatePline->SetEndScaleX( GetDrawing()->GetHabitat()->GetEndScaleX() );
        m_templatePline->SetEndScaleY( GetDrawing()->GetHabitat()->GetEndScaleY() );
        m_templatePline->SetBegin( GetDrawing()->GetHabitat()->GetLineBegin() );
        m_templatePline->SetEnd( GetDrawing()->GetHabitat()->GetLineEnd() );
        m_templatePline->SetPathType( GetDrawing()->GetHabitat()->GetPathType() );
        m_templatePline->SetContourWidth( GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale() );
        m_contourwidth = GetDrawing()->GetHabitat()->GetContourWidth() / GetDrawing()->GetUnitsScale();
    }
    else
    {
        m_layer = m_templatePline->GetLayer();
        m_fill = m_templatePline->GetFill();
        m_stroke = m_templatePline->GetStroke();
        m_contourwidth = m_templatePline->GetContourWidth();
    }
}

a2dDrawPolylineLTool::~a2dDrawPolylineLTool()
{
    a2dCanvasGlobals->DisconnectEvent( a2dEVT_COM_EVENT, this );
}

void a2dDrawPolylineLTool::SetPathType( a2dPATH_END_TYPE pathtype )
{
    if ( GetBusy() )
    {
        a2dPolylineL* poly = wxStaticCastNull( m_canvasobject.Get(), a2dPolylineL );
        poly->SetPathType( pathtype );
        a2dPolylineL* polyorg = wxStaticCastNull( m_original.Get(), a2dPolylineL );
        polyorg->SetPathType( pathtype );
    }

    m_templatePline->SetPathType( pathtype );
}

void a2dDrawPolylineLTool::SetLineBegin( a2dCanvasObject* begin )
{
    if ( GetBusy() )
    {
        a2dPolylineL* poly = wxStaticCastNull( m_canvasobject.Get(), a2dPolylineL );
        poly->SetBegin( begin );
        a2dPolylineL* polyorg = wxStaticCastNull( m_original.Get(), a2dPolylineL );
        polyorg->SetBegin( begin );
    }

    m_templatePline->SetBegin( begin );
}

void a2dDrawPolylineLTool::SetLineEnd( a2dCanvasObject* end )
{
    if ( GetBusy() )
    {
        a2dPolylineL* poly = wxStaticCastNull( m_canvasobject.Get(), a2dPolylineL );
        poly->SetEnd( end );
        a2dPolylineL* polyorg = wxStaticCastNull( m_original.Get(), a2dPolylineL );
        polyorg->SetEnd( end );
    }

    m_templatePline->SetEnd( end );
}

void a2dDrawPolylineLTool::SetEndScaleX( double xs )
{
    m_templatePline->SetEndScaleX( xs );
}


void a2dDrawPolylineLTool::SetEndScaleY( double ys )
{
    m_templatePline->SetEndScaleY( ys );
}

void a2dDrawPolylineLTool::OnComEvent( a2dComEvent& event )
{
    if ( m_AllowModifyTemplate )
    {
        if ( event.GetId() == a2dHabitat::sig_SetContourWidth )
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
                poly->SetPathType( pathtype );
                poly = wxStaticCast( m_canvasobject.Get(), a2dPolylineL );
                poly->SetPathType( pathtype );
            }
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

void a2dDrawPolylineLTool::OnDoEvent( a2dCommandProcessorEvent& event )
{
    if ( !m_templateObject )
    {
    }
    event.Skip();
}

void a2dDrawPolylineLTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
        restrict->SetShiftKeyDown( event.ShiftDown() );

    if ( event.LeftDown() && !GetBusy() )
    {
        if( restrict )
            restrict->RestrictPoint( m_xwprev, m_ywprev );
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        m_original = m_templateObject->TClone( clone_childs | clone_members );
        m_original->SetRoot( GetDrawing() );
        m_original->Translate( m_xwprev, m_ywprev );
        m_original->EliminateMatrix();
        if ( m_contourwidth ) // to not set m_filled flag
            m_original->SetContourWidth( m_contourwidth );
        m_original->SetStroke( m_stroke );
        if ( m_original->GetFilled() )
            m_original->SetFill( m_fill );
        m_original->SetLayer( m_layer );
        a2dPolylineL* line = wxStaticCast( m_original.Get(), a2dPolylineL );
        if ( line->GetBegin() && line->GetBegin()->GetFlag( a2dCanvasOFlags::normalized ) )
        {
            line->GetBegin()->SetSpecificFlags( false, a2dCanvasOFlags::normalized );
            double norm = GetDrawing()->GetNormalizeScale();
            line->GetBegin()->Scale( norm, norm );
        }
        if ( line->GetEnd() && line->GetEnd()->GetFlag( a2dCanvasOFlags::normalized ) )
        {
            line->GetEnd()->SetSpecificFlags( false, a2dCanvasOFlags::normalized );
            double norm = GetDrawing()->GetNormalizeScale();
            line->GetEnd()->Scale( norm, norm );
        }

        if ( !EnterBusyMode() )
            return;
        AddCurrent( m_original );
        m_canvasobject->SetContourWidth( m_contourwidth );
        a2dPolylineL* poly = wxStaticCast( m_canvasobject.Get(), a2dPolylineL );
        m_points = poly->GetSegments();

        a2dGeneralGlobals->RecordF( this, _T( "polyline" ) );

        m_canvasobject->SetRoot( GetDrawing() );

        // add point to the original
        if ( m_reverse )
            GetCanvasCommandProcessor()->Submit(
                new a2dCommand_AddPoint( m_original, m_xwprev, m_ywprev, 0 )
            );
        else
            GetCanvasCommandProcessor()->Submit(
                new a2dCommand_AddPoint( m_original, m_xwprev, m_ywprev, -1 )
            );

        m_point = new a2dLineSegment( m_xwprev, m_ywprev );
        AddPoint( m_point );

        a2dGeneralGlobals->RecordF( this, _T( "addpoint %f %f " ), m_xwprev, m_ywprev );

        //add second point to drag
        m_point = new a2dLineSegment( m_xwprev, m_ywprev );
        AddPoint( m_point );

        m_prev_x = m_xwprev;
        m_prev_y = m_ywprev;

        if( restrict )
            restrict->SetRestrictPoint( m_xwprev, m_ywprev );

        m_pending = true;
    }
    else
        event.Skip();
}

BEGIN_EVENT_TABLE( a2dDrawWirePolylineLTool, a2dDrawPolylineLTool )
    EVT_MOUSE_EVENTS( a2dDrawWirePolylineLTool::OnMouseEvent )
    EVT_KEY_DOWN( a2dDrawWirePolylineLTool::OnKeyDown )
    EVT_CHAR( a2dDrawWirePolylineLTool::OnChar )
    EVT_UNDO( a2dDrawWirePolylineLTool::OnUndoEvent )
    EVT_REDO( a2dDrawWirePolylineLTool::OnRedoEvent )
    EVT_CANUNDO( a2dDrawWirePolylineLTool::OnCanUndoEvent )
    EVT_CANREDO( a2dDrawWirePolylineLTool::OnCanRedoEvent )

    EVT_IDLE( a2dDrawWirePolylineLTool::OnIdle )
END_EVENT_TABLE()

a2dDrawWirePolylineLTool::a2dDrawWirePolylineLTool( a2dStToolContr* controller )
    : a2dDrawPolylineLTool( controller )
{
    m_hadDoubleClick = false;

    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_pinClassStartWire = NULL;
    m_pinClassEndWire = NULL;
    m_wiringMode = a2d_BasedOnClassStartPin;

    m_toolBusyCursor = *wxSTANDARD_CURSOR;
    m_splitAtEnd = true;
    m_startWireX = 0;
    m_startWireY = 0;
    m_manhattan = false;
	m_endSegmentMode = a2dCanvasGlobals->GetHabitat()->GetEndSegmentMode();

    //only modes which are usefull here
    m_snapTargetFeatures =
            a2dRestrictionEngine::snapToGridPos |
            a2dRestrictionEngine::snapToGridPosForced |
            a2dRestrictionEngine::snapToPinsUnconnected |
            a2dRestrictionEngine::snapToObjectVertexes |
            a2dRestrictionEngine::snapToObjectSegmentsMiddle |
            a2dRestrictionEngine::snapToObjectSegments;

    // pins on wire need to snap.
    m_snapSourceFeatures = a2dRestrictionEngine::snapToPins;
}

a2dDrawWirePolylineLTool::~a2dDrawWirePolylineLTool()
{
}

void a2dDrawWirePolylineLTool::AdjustRenderOptions()
{
    // The only realy reasonable mode for wire drawing is OnTop mode, so always use this one
    m_renderOriginal = false;
    m_renderEditcopy = false;
    m_renderEditcopyEdit = false;
    m_renderEditcopyOnTop = true;
    m_renderEditcopyRectangle = false;

    m_renderOriginal = true;
    m_renderEditcopy = false;
    m_renderEditcopyOnTop = false;
    m_renderEditcopyEdit = true;
    m_renderEditcopyRectangle = false;
}

a2dPin* a2dDrawWirePolylineLTool::FindPin( a2dCanvasObject* root, double x, double y )
{
    // the next will remove temporary pins, but we stil need to find them!!
    // GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( GetDrawingPart()->GetShowObject() );

    //there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
    // m_connectionGenerator.
    a2dPin* pin = GetDrawing()->GetHabitat()->GetConnectionGenerator()->
		        SearchPinForStartWire( GetDrawingPart()->GetShowObject(), x, y, a2dPinClass::Any, GetHitMargin() );

    if ( pin )
    {
        switch ( m_wiringMode )
        {
            case a2d_BasedOnClassStartPin:
                m_pinClassStartWire = GetDrawing()->GetHabitat()->GetConnectionGenerator()->GetReturnPinClass();
                return pin;
                break;
            case a2d_BasedOnWireClassRequired:
                m_pinClassStartWire = pin->GetPinClass();
                return pin;
                break;
            case a2d_BasedOnObjectClassRequired:
                m_pinClassStartWire = pin->GetPinClass();
                return pin;
                break;
			case a2d_StartGenerateSearchFinish:
                m_pinClassStartWire = pin->GetPinClass();
                return pin;
                break;
        }
    }
    return NULL;

/*
    // Check if the parent of this pin is a DIRECT child of the root given
    assert( !m_canvasobject || root->Find( m_canvasobject ) );

    root->GetChildObjectList()->SetSpecificFlags( false, a2dCanvasOFlags::BIN );

    // The pin calculates its coordinates in roots coordinate system
    a2dAffineMatrix tRootInv = root->GetTransformMatrix();
    tRootInv.Invert();
    double xh, yh;
    tRootInv.TransformPoint( x, y, xh, yh );


    // normal hittests go reverse drawing order of layers, eventually hiting what is drawn last.
    // Drawn last is of course on top of everything else.
    // In this case here we only want to hit pins, even if this pin is part of an object
    // is displayed underneath another object having pins too.
    // So the pin is not displayed on top still we want to hit it.
    // Therefore we need a special hittest here, to only search for pins, still when
    // we find a pin we do a normal hittest on that pin.

    // Go through the childs of the root and look for objects having pins
    for( a2dCanvasObjectList::iterator iter = root->GetChildObjectList()->begin(); iter != root->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;

        if( !obj->HasPins() )
            continue;
        if( obj == m_canvasobject )
            continue;
        if( obj == m_original )
            continue;

        a2dIterC ic( GetDrawingPart() );
        a2dIterCU cu( ic );

        // Check if obj has a pin that can connect to this pin

        for( a2dCanvasObjectList::iterator iterp = obj->GetChildObjectList()->begin(); iterp != obj->GetChildObjectList()->end(); ++iterp )
        {
            a2dPin* pin = wxDynamicCast( ( *iterp ).Get(), a2dPin );
            if ( pin )
            {
                // Don't return connected pins or pins that are to be deleted
                if( pin->ConnectedTo() || pin->GetRelease( ) )
                    continue;

                a2dIterCU cu( ic, obj );
                a2dHitEvent hitevent = a2dHitEvent( xh, yh, false );
                if ( pin->IsHitWorld( ic, hitevent ) )
                {
                    switch ( m_wiringMode )
                    {
                        case a2d_BasedOnClassStartPin:
                            if ( m_pinClassStartWire = pin->GetPinClass()->GetPinClassForTask( a2d_ToConnectPinClassWire_ObjectPinClass_InOut, obj ) )
                                return pin;
                            break;
                        case a2d_BasedOnWireClassRequired:
                            if ( m_pinClassStartWire->CanConnectTo( pin->GetPinClass(), a2dPinClass::PC_input | a2dPinClass::PC_output ) )
                                return pin;
                            break;
                        case a2d_BasedOnObjectClassRequired:
                            if ( pin->GetPinClass()->CanConnectTo( m_pinClassStartWire, a2dPinClass::PC_input | a2dPinClass::PC_output ) )
                                return pin;
                            break;
						case a2d_StartGenerateSearchFinish:
                            if ( m_pinClassStartWire = pin->GetPinClass()->GetPinClassForTask( a2d_StartWire, obj, pin->GetPinClass() ) )
                                return pin;
                            break;
                    }
                }
            }
        }
    }
    return 0;
*/
}

/* todo 
a2dPin* a2dDrawWirePolylineLTool::FindFinishPin( a2dCanvasObject* root, a2dPin* wirefinishpin )
{
    // Check if the parent of this pin is a DIRECT child of the root given
    assert( !m_canvasobject || root->Find( m_canvasobject ) );

    root->GetChildObjectList()->SetSpecificFlags( false, a2dCanvasOFlags::BIN );

    // The pin calculates its coordinates in roots coordinate system
    //a2dAffineMatrix tRootInv = root->GetTransformMatrix();
    //tRootInv.Invert();
    //tRootInv.TransformPoint( x, y, xh, yh );
    double xh, yh;
    xh = wirefinishpin->GetAbsX();
    yh = wirefinishpin->GetAbsY();


    // normal hittests go reverse drawing order of layers, eventually hiting what is drawn last.
    // Drawn last is of course on top of everything else.
    // In this case here we only want to hit pins, even if this pin is part of an object
    // is displayed underneath another object having pins too.
    // So the pin is not displayed on top still we want to hit it.
    // Therefore we need a special hittest here, to only search for pins, still when
    // we find a pin we do a normal hittest on that pin.

    // Go through the childs of the root and look for objects having pins
    for( a2dCanvasObjectList::iterator iter = root->GetChildObjectList()->begin(); iter != root->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;

        if( !obj->HasPins() )
            continue;
        if( obj == m_canvasobject )
            continue;
        if( obj == m_original )
            continue;

        a2dIterC ic( GetDrawingPart() );
        a2dIterCU cu( ic );

        // Check if obj has a pin that can connect to this pin

        for( a2dCanvasObjectList::iterator iterp = obj->GetChildObjectList()->begin(); iterp != obj->GetChildObjectList()->end(); ++iterp )
        {
            a2dPin* pin = wxDynamicCast( ( *iterp ).Get(), a2dPin );
            if ( pin )
            {
                // Don't return connected pins or pins that are to be deleted
                if( pin->ConnectedTo() || pin->GetRelease( ) )
                    continue;

                a2dIterCU cu( ic, obj );
                a2dHitEvent hitevent = a2dHitEvent( xh, yh, false );
                if ( pin->IsHitWorld( ic, hitevent ) )
                {
                    a2dConnectTask task = a2d_ToConnectPinClassWire_ObjectPinClass_InOut;

                    switch ( m_wiringMode )
                    {
                        case a2d_BasedOnClassStartPin:
                            if ( m_pinClassEndWire->CanConnectTo( pin->GetPinClass(), a2dPinClass::PC_input | a2dPinClass::PC_output ) )
                                return pin;
                            break;
                        case a2d_BasedOnWireClassRequired:
                            if ( m_pinClassEndWire->GetPinClassForTask( task, obj ) ==  pin->GetPinClass() )
                                return pin;
                            break;
                        case a2d_BasedOnObjectClassRequired:
                            if ( m_pinClassEndWire->GetPinClassForTask( task, obj ) ==  pin->GetPinClass() )
                                return pin;
                            break;
						case a2d_StartGenerateSearchFinish:
                            if ( m_pinClassEndWire->GetPinClassForTask( a2d_FinishWire, obj, pin->GetPinClass(), wirefinishpin ) )
                                return pin;
                            break;
                    }
                }
            }
        }
    }
    return 0;
}
*/

bool a2dDrawWirePolylineLTool::EnterBusyMode()
{
    return a2dDrawPolylineLTool::EnterBusyMode();
}

void a2dDrawWirePolylineLTool::FinishBusyMode( bool closeCommandGroup )
{
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToEndState( GetDrawingPart()->GetShowObject() );

    a2dDrawPolylineLTool::FinishBusyMode();
}

void a2dDrawWirePolylineLTool::AbortBusyMode()
{
    GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToEndState( GetDrawingPart()->GetShowObject() );
    a2dDrawPolylineLTool::AbortBusyMode();
}

void a2dDrawWirePolylineLTool::OnIdle( wxIdleEvent& event )
{
    if ( m_pending && GetBusy() )
    {
 
	}
}

void a2dDrawWirePolylineLTool::SetEndSegmentMode( a2dNextSeg mode )
{
    m_endSegmentMode = mode;
	switch ( m_endSegmentMode )
    {
        case 0:
        {
            m_manhattan = false;
			m_like = true;
            break;
        }

		case 1:
        {
            m_manhattan = true;
			m_like = true;
            break;
        }
        case 2:
        {
            m_manhattan = true;
			m_like = false;
            break;
        }
        default:
            m_manhattan = false;
			m_like = true;
            break;
    }

    if ( !m_canvasobject )
        return;

    if ( m_like )
    {
        m_manx = m_xwprev;
        m_many = m_prev_y;
    }
    else
    {
        m_manx = m_prev_x;
        m_many = m_ywprev;
    }

    a2dPolygonL* poly = wxStaticCast( m_original.Get(), a2dPolygonL );
    if ( poly->GetNumberOfSegments() )
    {
        a2dLineSegment* lastpoint = poly->GetSegments()->back();

        CleanupToolObjects();
        GetDrawingPart()->Update( a2dCANVIEW_UPDATE_OLDNEW );
        CreateToolObjects();
		//CreateToolObjects makes a deep copy, which includes the created pins made by GetConnectionGenerator().
		//That is less atractive here, the next remove them, instead we could have overridden CreateToolObjects too.
		m_canvasobject->RemovePins( true, false, true );

        m_points = wxStaticCast( m_canvasobject.Get(), a2dPolygonL )->GetSegments();
        if ( m_manhattan )
        {
            m_pointmanhattan =  new a2dLineSegment( m_manx, m_many );
			AddPoint( m_pointmanhattan );
        }
        m_point = new a2dLineSegment( m_xwprev, m_ywprev );
		AddPoint( m_point );
    }

    a2dPin* drawPin;
    if ( m_reverse )
        drawPin = m_original->HasPinNamed( wxT( "begin" ) );
    else
        drawPin = m_original->HasPinNamed( wxT( "end" ) );
    if ( drawPin )
    {
        drawPin->SetPosXY( m_point->m_x, m_point->m_y );
    }

	m_original->SetPending( true );
	m_canvasobject->SetPending( true );
	m_pending = true;
}

void a2dDrawWirePolylineLTool::OnKeyDown( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_SPACE:
            {
				AtNewVertex();
				if ( !m_stop )
				{
					a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
					if( restrict )
						restrict->RestrictPoint( m_xwprev, m_ywprev );

					GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
					a2dGeneralGlobals->RecordF( this, _T( "end" ) );
					m_original->SetGeneratePins( true );
					m_original->SetAlgoSkip( false );
       
					int endx = m_x;
					int endy = m_y;
                    GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToEndState( GetDrawingPart()->GetShowObject() );
					if ( abs( m_startWireXdev - endx ) > GetDrawing()->GetHabitat()->GetHandle()->GetWidth() || abs( m_startWireYdev - endy ) > GetDrawing()->GetHabitat()->GetHandle()->GetWidth() ) //prevent zero lines
						StartEditingOrFinish();
					else
						AbortBusyMode();
				}
				break;
            }
            case WXK_ESCAPE:
            {
                AbortBusyMode();
                break;
            }
            default:
                event.Skip();
        }
    }
    else
        event.Skip();
}

void a2dDrawWirePolylineLTool::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() && m_canvasobject )
    {
        double shiftx;
        double shifty;
        GetKeyIncrement( &shiftx, &shifty );

        switch( event.GetKeyCode() )
        {
            case WXK_TAB:
            {
				//rotate mode
				if ( m_endSegmentMode == WIREEND_STRAIGHT )
					m_endSegmentMode = WIREEND_MANHATTAN_X;
				else if ( m_endSegmentMode == WIREEND_MANHATTAN_X )
					m_endSegmentMode = WIREEND_MANHATTAN_Y;
				else if ( m_endSegmentMode == WIREEND_MANHATTAN_Y )
					m_endSegmentMode = WIREEND_STRAIGHT;
                SetEndSegmentMode( m_endSegmentMode );
                return;
            }
            break;

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

        if ( event.m_controlDown )
        {
            m_xwprev += shiftx;
            m_ywprev += shifty;
        }
        else
        {
            m_xwprev += shiftx;
            m_ywprev += shifty;
        }

        m_point->m_x = m_xwprev;
        m_point->m_y = m_ywprev;
        GetDisplayWindow()->WarpPointer( GetDrawer2D()->WorldToDeviceX( m_xwprev ), GetDrawer2D()->WorldToDeviceY( m_ywprev ) );

        m_pending = true;
    }
    else
        event.Skip();

}


void a2dDrawWirePolylineLTool::AtNewVertex()
{
	double unRestx = m_xwprev;
	double unResty = m_ywprev;
	a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();

    if( restrict )
    {
        a2dSnapToWhatMask snapTo = m_snapTargetFeatures;
        a2dSnapToWhatMask was = restrict->GetSnapSourceFeatures();
        if ( m_snapSourceFeatures != a2dRestrictionEngine::snapToNone )
            restrict->SetSnapSourceFeatures( m_snapSourceFeatures );
        if ( snapTo == a2dRestrictionEngine::snapToNone )
            restrict->GetSnapTargetFeatures();

        restrict->RestrictPoint( m_xwprev, m_ywprev, snapTo, true );
        restrict->SetSnapSourceFeatures( was );
    }

    m_point->m_x = m_xwprev;
    m_point->m_y = m_ywprev;

    GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
    m_original->RemovePins( true, true );

    a2dPin* drawPin;
    if ( m_reverse )
        drawPin = m_canvasobject->HasPinNamed( wxT( "begin" ) );
    else
        drawPin = m_canvasobject->HasPinNamed( wxT( "end" ) );
    if ( drawPin )
        drawPin->SetPosXY( m_xwprev, m_ywprev );

    if ( m_reverse )
        drawPin = m_original->HasPinNamed( wxT( "begin" ) );
    else
        drawPin = m_original->HasPinNamed( wxT( "end" ) );
    drawPin->SetPosXY( m_xwprev, m_ywprev );

    double hitDistance = GetHitMargin();

    a2dPin* pin = drawPin->GetPinClass()->GetConnectionGenerator()->
	            SearchPinForFinishWire( GetDrawingPart()->GetShowObject(), drawPin, NULL, hitDistance  );

    if( !pin )
    {
        drawPin->SetPosXY( unRestx, unResty );
        //second try for un-snapped pin.
        pin = drawPin->GetPinClass()->GetConnectionGenerator()->
	            SearchPinForFinishWire( GetDrawingPart()->GetShowObject(), drawPin, NULL, hitDistance  );
    }

    int x = GetDrawer2D()->WorldToDeviceX( m_prev_x );
    int y = GetDrawer2D()->WorldToDeviceY( m_prev_y );
    if ( abs( m_x - x ) >= 3 || abs( m_y - y ) >= 3 ) //to close a second point is not excepted??
    {
        if( pin && !pin->GetPinClass()->CanConnectTo( m_original->HasPinNamed( m_reverse ? wxT( "begin" ) : wxT( "end" ) )->GetPinClass() ) )
        {
            ( void )wxMessageBox( _( "You cannot connect to this end pin" ), _( "Connect" ), wxICON_INFORMATION | wxOK );
        }
        else
        {
            // for next point to draw
            if( restrict )
                restrict->SetRestrictPoint( m_xwprev, m_ywprev );

            if ( pin )
            {
                //the intention is to click on the pin and therefore snap to that pin,
                // even if not perfect on the grid.
                m_xwprev = pin->GetAbsX();
                m_ywprev = pin->GetAbsY();
                drawPin->SetPosXY( m_xwprev, m_ywprev );
            }
            else
                drawPin->SetPosXY( m_xwprev, m_ywprev );

		    double dx =  m_xwprev - m_prev_x;
		    double dy =  m_ywprev - m_prev_y;

            if ( m_manhattan )
            {
                if ( m_like )
                {
                    m_manx = m_xwprev;
                    m_many = m_prev_y;
                }
                else
                {
                    m_manx = m_prev_x;
                    m_many = m_ywprev;
                }

			    m_pointmanhattan->m_x = m_manx;
			    m_pointmanhattan->m_y = m_many;

                // add point to the original
                if ( m_reverse )
                    GetCanvasCommandProcessor()->Submit(
                        new a2dCommand_AddPoint( m_original, m_manx, m_many, 0 )
                    );
                else
                    GetCanvasCommandProcessor()->Submit(
                        new a2dCommand_AddPoint( m_original, m_manx, m_many, -1 )
                    );

                // add point to editcopy
                m_pointmanhattan = new a2dLineSegment( m_xwprev, m_ywprev );
                AddPoint( m_pointmanhattan );
            }

            // add point to the original
            if ( m_reverse )
                GetCanvasCommandProcessor()->Submit(
                    new a2dCommand_AddPoint( m_original, m_xwprev, m_ywprev, 0 )
                );
            else
                GetCanvasCommandProcessor()->Submit(
                    new a2dCommand_AddPoint( m_original, m_xwprev, m_ywprev, -1 )
                );

            // add point to editcopy
            m_point = new a2dLineSegment( m_xwprev, m_ywprev );
            AddPoint( m_point );

            m_prev_x = m_xwprev;
            m_prev_y = m_ywprev;

            // add point to record
            a2dGeneralGlobals->RecordF( this, _T( "addpoint %f %f " ), m_xwprev, m_ywprev );

            a2dWirePolylineL* wire = wxStaticCast( m_original.Get(), a2dWirePolylineL );
            wire->RemoveRedundantPoints();

            // if it was a pin hit, add the pin at the last point
            // also end the tool then
            if( pin )
            {
                if ( pin->IsTemporaryPin() ) //those pins are generated without commands, not good for undo.
                {
                    pin = wxStaticCast( pin->Clone( clone_deep ), a2dPin );
                    pin->SetTemporaryPin( false );
                    GetCanvasCommandProcessor()->Submit( new a2dCommand_AddObject( pin->GetParent(), pin ), true ); //this makes it save
                }
                GetCanvasCommandProcessor()->Submit( new a2dCommand_ConnectPins( drawPin, pin ), true );
                if ( m_splitAtEnd == true && pin->IsDynamicPin() && pin->GetParent()->IsConnect() )
                {
                    a2dWirePolylineL* wireconnected = wxStaticCast( pin->GetParent(), a2dWirePolylineL );
                    if ( wireconnected )
                    {
                        bool done = GetCanvasCommandProcessor()->Submit( new a2dCommand_SplitAtPin( GetDrawingPart()->GetShowObject(), pin, GetHitMargin() ) );
                    }
                }
                a2dGeneralGlobals->RecordF( this, _T( "end" ) );

                m_original->SetGeneratePins( true );
                m_original->SetAlgoSkip( false );
                // restrict prevent
                // m_original->SetHitFlags( a2dCANOBJ_EVENT_ALL );
                // m_canvasobject->SetHitFlags( a2dCANOBJ_EVENT_ALL );

                //m_original->RemoveProperty( a2dCanvasObject::PROPID_DisableFeedback );
                GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToEndState( GetDrawingPart()->GetShowObject() );
                StartEditingOrFinish();
            }
        }
    }
}


void a2dDrawWirePolylineLTool::OnMouseEvent( wxMouseEvent& event )
{
    // This is mostly a copy of a2dDrawPolylineLTool::OnMouseEvent
    // changes are
    // - type of created object
    // - recorded event names
    // - creation and connection of a2dPin's
    // - end of tool if a pin is hint when adding a point

    if ( !m_active )
    {
        event.Skip();
        return;
    }

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    double unRestx = m_xwprev;
    double unResty = m_ywprev;

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
        restrict->SetShiftKeyDown( event.ShiftDown() );
    if ( event.LeftDown() )
    {
        m_hadDoubleClick = false;
        m_dragstartx = m_x;
        m_dragstarty = m_y;
    }
    else if ( event.LeftUp() && !GetBusy() && m_hadDoubleClick )
    {
        m_hadDoubleClick = false;
    }
    else if ( event.LeftUp() && !GetBusy() )
    {
        if ( abs( m_x - m_dragstartx ) >= 3 || abs( m_y - m_dragstarty ) >= 3 ) //to close a second point is not excepted??
        {
            ( void )wxMessageBox( _( "you can not drag a pin" ), _( "Connect" ), wxICON_INFORMATION | wxOK );
            return;
        }

        if( restrict )
        {
            a2dSnapToWhatMask snapTo = m_snapTargetFeatures;
            a2dSnapToWhatMask was = restrict->GetSnapSourceFeatures();
            if ( m_snapSourceFeatures != a2dRestrictionEngine::snapToNone )
                restrict->SetSnapSourceFeatures( m_snapSourceFeatures );
            if ( snapTo == a2dRestrictionEngine::snapToNone )
                restrict->GetSnapTargetFeatures();

            restrict->RestrictPoint( m_xwprev, m_ywprev, snapTo, true );
            restrict->SetSnapSourceFeatures( was );
        }

        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        // Check if the user clicked on a pin
        a2dPin* pin = FindPin( GetDrawingPart()->GetShowObject(), m_xwprev, m_ywprev );
        if( !pin )
            pin = FindPin( GetDrawingPart()->GetShowObject(), unRestx, unResty );
        if( !pin )
        {
            ( void )wxMessageBox( _( "A connection must start at a pin or another connection" ), _( "Connect" ), wxICON_INFORMATION | wxOK );
            if( m_oneshot )
                StopTool();
            return;
        }
        if ( !pin->GetParent()->DoConnect() )
            return;

        // starting a new group here, and not closing it, will have EnterBusyMode() using this group instead of creating
        // one itself.
        OpenCommandGroupNamed( wxT( "create connection" ) );

        if ( pin->IsTemporaryPin() ) //those pins are generated without commands, not good for undo.
        {
            pin = wxStaticCast( pin->Clone( clone_deep ), a2dPin );
            pin->SetTemporaryPin( false );
            GetCanvasCommandProcessor()->Submit( new a2dCommand_AddObject( pin->GetParent(), pin ), true ); //this makes it save
        }

        if ( m_splitAtEnd == true && pin->IsDynamicPin() && pin->GetParent()->IsConnect() )
        {
            a2dWirePolylineL* wireconnected = wxStaticCast( pin->GetParent(), a2dWirePolylineL );
            if ( wireconnected )
            {
                bool done = GetCanvasCommandProcessor()->Submit( new a2dCommand_SplitAtPin( GetDrawingPart()->GetShowObject(), pin, GetHitMargin() ) );
            }
        }

        //we want to start and connect to the pin, so snap to it, even if pin is not on the grid.
        m_xwprev = pin->GetAbsX();
        m_ywprev = pin->GetAbsY();

        m_original = pin->GetParent()->CreateConnectObject( m_parentobject, pin, NULL, true );
        if ( !m_original )
            return;
        m_original->SetAlgoSkip( true );
        //a2dCanvasObject::PROPID_DisableFeedback->SetPropertyToObject( m_original, true );
        m_original->SetGeneratePins( false );
        m_reverse = pin->GetPinClass()->GetConnectionGenerator()->GetLastConnectCreationDirection();
        wxASSERT_MSG( m_original.Get() != 0, wxT( "a connection object could not be created for the pin clicked" ) );

        // Either clone the template object or create a new one
        //m_original->Translate( m_xwprev, m_ywprev );
        m_original->EliminateMatrix();

        if ( !EnterBusyMode() )
            return;

        //CreateToolObjects makes a deep copy, which includes the created pins made by GetConnectionGenerator().
        //That is less atractive here, the next remove them, instead we could have overridden CreateToolObjects too.
        m_canvasobject->RemovePins( true, false, true );

        m_points = wxStaticCast( m_canvasobject.Get(), a2dPolylineL )->GetSegments();
        m_point = new a2dLineSegment( m_xwprev, m_ywprev );
        AddPoint( m_point );

        // restrict prevent
        //m_original->SetHitFlags( a2dCANOBJ_EVENT_NON );
        //m_canvasobject->SetHitFlags( a2dCANOBJ_EVENT_NON );

        // Highlight matching pins in other objects
        {
            a2dPin* drawPin;
            if ( m_reverse )
                drawPin = m_original->HasPinNamed( wxT( "begin" ) );
            else
                drawPin = m_original->HasPinNamed( wxT( "end" ) );
            m_pinClassEndWire = drawPin->GetPinClass();
            //drawPin->SetVisible( false );
        }

        // Record commands
        a2dGeneralGlobals->RecordF( this, _T( "polylinewire" ) );
        a2dGeneralGlobals->RecordF( this, _T( "addpoint %f %f " ), m_xwprev, m_ywprev );

        if ( m_manhattan )
        {
            m_pointmanhattan = new a2dLineSegment( m_xwprev, m_ywprev );
            AddPoint( m_pointmanhattan );
        }
        //add second point to drag
        m_point = new a2dLineSegment( m_xwprev, m_ywprev );
        AddPoint( m_point );

        m_prev_x = m_xwprev;
        m_prev_y = m_ywprev;

        if( restrict )
            restrict->SetRestrictPoint( m_xwprev, m_ywprev );

        m_startWireX = m_xwprev;
        m_startWireY = m_ywprev;
		m_startWireXdev = m_x;
		m_startWireYdev = m_y;

        m_pending = true;

    }
    else if ( event.LeftUp() && GetBusy() )
    {
		AtNewVertex();
    }
    else if ( event.LeftDClick() && GetBusy() )
    {
        m_hadDoubleClick = true;

        if( restrict )
            restrict->RestrictPoint( m_xwprev, m_ywprev );

        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dGeneralGlobals->RecordF( this, _T( "end" ) );
        m_original->SetGeneratePins( true );
        m_original->SetAlgoSkip( false );
       
        int endx = m_x;
        int endy = m_y;
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToEndState( GetDrawingPart()->GetShowObject() );
        if ( abs( m_startWireXdev - endx ) > GetDrawing()->GetHabitat()->GetHandle()->GetWidth() || abs( m_startWireYdev - endy ) > GetDrawing()->GetHabitat()->GetHandle()->GetWidth() ) //prevent zero lines
            StartEditingOrFinish();
        else
            AbortBusyMode();
    }
    else if ( event.RightUp() && GetBusy() )
    {
        m_hadDoubleClick = true;
        if( restrict )
            restrict->RestrictPoint( m_xwprev, m_ywprev );

        a2dPin* drawPin;
        a2dPolylineL* org = wxStaticCast( m_original.Get(), a2dPolylineL );
        if ( m_reverse )
        {
            m_point = org->GetFirstPoint();
            m_xwprev = m_point->m_x;
            m_ywprev = m_point->m_y;
            drawPin = org->HasPinNamed( wxT( "begin" ) );
        }
        else
        {
            m_xwprev = m_prev_x;
            m_ywprev = m_prev_y;
            drawPin = org->HasPinNamed( wxT( "end" ) );
        }
        drawPin->SetPosXY( m_xwprev, m_ywprev );

        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dGeneralGlobals->RecordF( this, _T( "end" ) );
        m_original->SetGeneratePins( true );
        m_original->SetAlgoSkip( false );
        
        int endx = m_x;
        int endy = m_y;
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToEndState( GetDrawingPart()->GetShowObject() );
        if ( ( m_manhattan && org->GetNumberOfSegments() > 2 ) ||
             ( !m_manhattan && org->GetNumberOfSegments() > 1 ) )
            StartEditingOrFinish();
        else
            AbortBusyMode();
    }
    else if ( ( event.Dragging() || event.Moving() ) && GetBusy() )
    {
        if( restrict )
        {
            a2dSnapToWhatMask snapTo = m_snapTargetFeatures;
            a2dSnapToWhatMask was = restrict->GetSnapSourceFeatures();
            if ( m_snapSourceFeatures != a2dRestrictionEngine::snapToNone )
                restrict->SetSnapSourceFeatures( m_snapSourceFeatures );
            if ( snapTo == a2dRestrictionEngine::snapToNone )
                restrict->GetSnapTargetFeatures();

            restrict->RestrictPoint( m_xwprev, m_ywprev, snapTo, true );
            restrict->SetSnapSourceFeatures( was );
        }
        //The wire was created, and a clone also.
        //Show end/begin pin at right position.
        a2dPin* drawPin;
        if ( m_reverse )
            drawPin = m_original->HasPinNamed( wxT( "begin" ) );
        else
            drawPin = m_original->HasPinNamed( wxT( "end" ) );
        drawPin->SetPosXY( m_xwprev, m_ywprev );

        // Give other objects an opportunity to show dynamic pins
        drawPin->GetPinClass()->GetConnectionGenerator()->SetPinsToBeginState( GetDrawingPart()->GetShowObject() );
		drawPin->GetPinClass()->GetConnectionGenerator()->
            GeneratePinsToConnect( GetDrawingPart(), GetDrawingPart()->GetShowObject(), drawPin->GetPinClass(), a2d_GeneratePinsForFinishWire, m_xwprev, m_ywprev );

        double hitDistance = GetHitMargin();

        a2dPin* foundPin = NULL;
        //there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
        // m_connectionGenerator.
        switch ( m_wiringMode )
        {
            case a2d_BasedOnClassStartPin:
            {
				foundPin = drawPin->GetPinClass()->GetConnectionGenerator()->
					SearchPinForFinishWire( GetDrawingPart()->GetShowObject(), drawPin, m_pinClassEndWire, hitDistance );
            }
            break;
            case a2d_BasedOnWireClassRequired:
            case a2d_BasedOnObjectClassRequired:
            {
				foundPin = drawPin->GetPinClass()->GetConnectionGenerator()->
					SearchPinForFinishWire( GetDrawingPart()->GetShowObject(), drawPin, m_pinClassStartWire, hitDistance );
            }
            break;
			case a2d_StartGenerateSearchFinish:
            {
				foundPin = drawPin->GetPinClass()->GetConnectionGenerator()->
					SearchPinForFinishWire( GetDrawingPart()->GetShowObject(), drawPin, drawPin->GetPinClass(), hitDistance );
            }
            break;
        }
        if ( foundPin )
            SetStateString( _("Click to end wire at this pin"), STAT_toolHelp );
        else
            SetStateString( _("Click to add vertex or end wire at other pin or at Double Click"), STAT_toolHelp );

        a2dBoundingBox updateareas;
        //AddPoint( m_point );

        updateareas.Expand( m_prev_x, m_prev_y );
        updateareas.Expand( m_point->GetPoint() );
        //a2dCanvasObject::PROPID_BoundingBox->SetPropertyToObject( m_canvasobject, updateareas );
        updateareas.Reset();
        updateareas.Expand( m_points->front()->GetPoint() );
        updateareas.Expand( m_point->GetPoint() );
        //a2dCanvasObject::PROPID_BoundingBox->SetPropertyToObject( m_canvasobject, updateareas );

		double dx =  m_xwprev - m_prev_x;
		double dy =  m_ywprev - m_prev_y;
        int x = GetDrawer2D()->WorldToDeviceX( m_prev_x );
        int y = GetDrawer2D()->WorldToDeviceY( m_prev_y );
		float dxdev = x - m_x;
		float dydev = y - m_y;
        double lendev = sqrt( dxdev*dxdev + dydev*dydev );

        if ( lendev < 25 )
        {
            if ( fabs( dx ) < fabs( dy ) )
                m_like = false;
            else
                m_like = true;
        }

        if ( m_like )
        {
            m_manx = m_xwprev;
            m_many = m_prev_y;
        }
        else
        {
            m_manx = m_prev_x;
            m_many = m_ywprev;
        }

        if ( m_manhattan )
        {        
			m_pointmanhattan->m_x = m_manx;
			m_pointmanhattan->m_y = m_many;
		}

        m_point->m_x = m_xwprev;
        m_point->m_y = m_ywprev;
        m_canvasobject->SetPending( true );

        updateareas.Reset();
        updateareas.Expand( m_prev_x, m_prev_y );
        updateareas.Expand( m_point->GetPoint() );
        //a2dCanvasObject::PROPID_BoundingBox->SetPropertyToObject( m_canvasobject, updateareas );

        updateareas.Reset();
        updateareas.Expand( m_points->front()->GetPoint() );
        updateareas.Expand( m_point->GetPoint() );
        //a2dCanvasObject::PROPID_BoundingBox->SetPropertyToObject( m_canvasobject, updateareas );
        // do not skip the event, which would lead to another restrict in a2dDrawPolygonLTool.
        GetDrawingPart()->SetToolWantsEventToContinue( true );
        m_pending = true;

	    double unitScale = GetDrawing()->GetUnitsScale();
		wxString state;
		wxString form = _("Lpx") + m_stcontroller->GetFormat() + _(" Lpy") + m_stcontroller->GetFormat();
		state.Printf( form, m_prev_x*unitScale, m_prev_y*unitScale );
		SetStateString( state, 10 );
		form = _("dx = ") + m_stcontroller->GetFormat() + _(" dy = ") + m_stcontroller->GetFormat();
		state.Printf( form, dx*unitScale, dy*unitScale );
		SetStateString( state, 11 );
    }
    else if ( event.Moving() && !GetBusy() )
    {
        if( restrict )
            restrict->RestrictPoint( m_xwprev, m_ywprev );

        //there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
        // m_connectionGenerator.
        GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToBeginState( GetDrawingPart()->GetShowObject() );
		GetDrawing()->GetHabitat()->GetConnectionGenerator()->
			GeneratePinsToConnect( GetDrawingPart(), GetDrawingPart()->GetShowObject(), a2dPinClass::Any, a2d_GeneratePinsForStartWire, unRestx, unResty );

        double hitDistance = GetHitMargin();

        //there is no wire nor pin yet, therefore we can just feedback possible pin connection, based on the
        // m_connectionGenerator.
        switch ( m_wiringMode )
        {
            case a2d_BasedOnClassStartPin:
            {
				GetDrawing()->GetHabitat()->GetConnectionGenerator()->
					SearchPinForStartWire( GetDrawingPart()->GetShowObject(), unRestx, unResty, a2dPinClass::Any, hitDistance );
            }
            break;
            case a2d_BasedOnWireClassRequired:
            case a2d_BasedOnObjectClassRequired:
            {
				GetDrawing()->GetHabitat()->GetConnectionGenerator()->
					SearchPinForStartWire( GetDrawingPart()->GetShowObject(), unRestx, unResty, m_pinClassStartWire, hitDistance );
            }
            break;
			case a2d_StartGenerateSearchFinish:
            {
				GetDrawing()->GetHabitat()->GetConnectionGenerator()->
					SearchPinForStartWire( GetDrawingPart()->GetShowObject(), unRestx, unResty, a2dPinClass::Any, hitDistance );
            }
            break;
        }
        GetDrawingPart()->SetToolWantsEventToContinue( true );
    }
    else
    {
        event.Skip();
    }
}

void a2dDrawWirePolylineLTool::SetActive( bool active )
{
    // reset editing hints
    if( !active )
    {
        if( GetDrawingPart() )
        {
            GetDrawing()->GetHabitat()->GetConnectionGenerator()->SetPinsToEndState( GetDrawingPart()->GetShowObject() );
        }
    }
    a2dDrawPolylineLTool::SetActive( active );
}

void a2dDrawWirePolylineLTool::OnCanUndoEvent( a2dCommandProcessorEvent& event )
{
    if ( GetBusy() )
    {
        if ( !m_halted &&  event.GetCommand()->GetCommandId() ==  &a2dCommand_AddPoint::Id )
            event.Veto( false );
        else
            event.Veto( true );
    }
}

void a2dDrawWirePolylineLTool::OnCanRedoEvent( a2dCommandProcessorEvent& event )
{
    if ( GetBusy() )
    {
        if ( !m_halted &&  event.GetCommand()->GetCommandId() ==  &a2dCommand_AddPoint::Id )
            event.Veto( false );
        else
            event.Veto( true );
    }
}

void a2dDrawWirePolylineLTool::OnUndoEvent( a2dCommandProcessorEvent& event )
{
    if ( GetBusy() && !m_halted &&  event.GetCommand()->GetCommandId() ==  &a2dCommand_AddPoint::Id )
    {
        // recreate edit copies so the edit copy becomes updated from the original.

        a2dPolygonL* poly = wxStaticCast( m_original.Get(), a2dPolygonL );
        if ( poly->GetNumberOfSegments() )
        {
            a2dLineSegment* lastpoint = poly->GetSegments()->back();

            CleanupToolObjects();
            GetDrawingPart()->Update( a2dCANVIEW_UPDATE_OLDNEW );
            CreateToolObjects();
			//CreateToolObjects makes a deep copy, which includes the created pins made by GetConnectionGenerator().
			//That is less atractive here, the next remove them, instead we could have overridden CreateToolObjects too.
			m_canvasobject->RemovePins( true, false, true );
            m_points = wxStaticCast( m_canvasobject.Get(), a2dPolygonL )->GetSegments();
            if ( m_manhattan )
            {
                m_pointmanhattan = lastpoint->Clone();
                m_points->push_back( m_pointmanhattan );
            }
            m_point = lastpoint->Clone();
            m_points->push_back( m_point );
            m_prev_x = m_xwprev = m_point->m_x;
            m_prev_y = m_ywprev = m_point->m_y;

        }
        a2dPin* drawPin;
        if ( m_reverse )
            drawPin = m_original->HasPinNamed( wxT( "begin" ) );
        else
            drawPin = m_original->HasPinNamed( wxT( "end" ) );
        if ( drawPin )
        {
            drawPin->SetPosXY( m_point->m_x, m_point->m_y );
        }
    }
}

void a2dDrawWirePolylineLTool::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    OnUndoEvent( event );
}
