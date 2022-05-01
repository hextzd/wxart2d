/*! \file canvas/src/canwidget.cpp
    \author Michael Sögtrop
    \date Created 11/19/2003

    Copyright: 2003-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: canwidget.cpp,v 1.62 2009/10/01 19:22:35 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canobj.h"

#include "wx/canvas/canwidget.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"

IMPLEMENT_CLASS( a2dWidgetButton, a2dWindowMM )
IMPLEMENT_CLASS( a2dWidgetButtonGroup, a2dCanvasObject )
IMPLEMENT_CLASS( a2dWidgetButtonCommand, a2dWidgetButton )

BEGIN_EVENT_TABLE( a2dWidgetButton, a2dWindowMM )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dWidgetButton::OnCanvasObjectMouseEvent )
    EVT_CANVASOBJECT_ENTER_EVENT( a2dWidgetButton::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT( a2dWidgetButton::OnLeaveObject )
END_EVENT_TABLE()

a2dWidgetButton::a2dWidgetButton( a2dCanvasObject* parent, int buttonId, double x, double y, double w, double h, WidgetMode mode )
    : a2dWindowMM( x, y, w, h )
{
    m_parent = parent;
    m_selectedStroke = *a2dBLACK_STROKE;
    m_selectedFill = *a2dTRANSPARENT_FILL;

    m_highLightStroke = *a2dNullSTROKE;
    m_highLightFill = *a2dNullFILL;

    SetStroke( *a2dTRANSPARENT_STROKE );
    SetFill( *a2dTRANSPARENT_FILL );
    m_buttonId = buttonId;
    m_mode = mode;
    m_content = 0;
    m_contentBorder = 0.0;
}

a2dWidgetButton::a2dWidgetButton( const a2dWidgetButton& other, CloneOptions options, a2dRefMap* refs )
    : a2dWindowMM( other, options, refs )
{
    m_parent = other.m_parent;
    m_selectedStroke = other.m_selectedStroke;
    m_selectedFill = other.m_selectedFill;
    m_highLightStroke = other.m_highLightStroke;
    m_highLightFill = other.m_highLightFill;
    m_buttonId = other.m_buttonId;
    m_mode = other.m_mode;
    if ( m_content )
        m_content = other.m_content;
    m_contentBorder = other.m_contentBorder;
}

a2dObject* a2dWidgetButton::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dWidgetButton( *this, options, refs );
};

a2dWidgetButton::~a2dWidgetButton()
{
}

void a2dWidgetButton::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dCanvasObject::DoWalker( parent, handler );

    if (  m_content )
        m_content->Walker( this, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}

void a2dWidgetButton::DrawHighLighted( a2dIterC& ic )
{
    if ( !m_highLightStroke.IsNoStroke() )
        ic.GetDrawer2D()->SetDrawerStroke( m_highLightStroke );
    else
        ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetHighLightStroke() );

    if ( !m_highLightFill.IsNoFill() )
        ic.GetDrawer2D()->SetDrawerFill( m_highLightFill );
    else
        ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetHighLightFill() );

    a2dStroke m_shadowStroke = a2dStroke( *wxBLACK, 0, a2dSTROKE_SOLID );
    a2dStroke m_lightStroke = a2dStroke( *wxWHITE, 0, a2dSTROKE_SOLID );

    double x1 = m_bbox.GetMinX();
    double x2 = m_bbox.GetMaxX();
    double y1 = m_bbox.GetMinY();
    double y2 = m_bbox.GetMaxY();
    ic.GetDrawer2D()->SetDrawerStroke( m_shadowStroke );
    ic.GetDrawer2D()->DrawLine( x1, y1, x1, y2 );
    ic.GetDrawer2D()->DrawLine( x1, y2, x2, y2 );
    ic.GetDrawer2D()->SetDrawerStroke( m_lightStroke );
    ic.GetDrawer2D()->DrawLine( x2, y1, x2, y2 );
    ic.GetDrawer2D()->DrawLine( x2, y1, x1, y1 );


    //ic.GetDrawer2D()->DrawRoundedRectangle( m_bbox.GetMinX(), m_bbox.GetMinY(), m_bbox.GetWidth(), m_bbox.GetHeight() ,0 );

    //a2dBoundingBox tbbox = DoGetUnTransformedBbox();

    //ic.GetDrawer2D()->SetTransform( ic.GetTransform() );
    //ic.GetDrawer2D()->DrawRoundedRectangle( tbbox.GetMinX(), tbbox.GetMinY(), tbbox.GetWidth(), tbbox.GetHeight() ,0 );
}


void a2dWidgetButton::Render( a2dIterC& ic, OVERLAP clipparent )
{
    // a2dWidgetButton does its own selection drawing, so base class behaviour is not wanted
    if( ic.GetDrawStyle() != RenderWIREFRAME_SELECT && ic.GetDrawStyle() != RenderWIREFRAME_SELECT_INVERT )
        a2dWindowMM::Render( ic, clipparent );
}

bool a2dWidgetButton::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        m_bbox = DoGetUnTransformedBbox();
        switch( m_mode )
        {
            case Boundingbox:
                m_contentWorld.Identity();
                break;

            case BoundingboxSize:
                if ( m_content )
                {
                    a2dBoundingBox tbbox;
                    tbbox.Expand( m_content->GetCalculatedBoundingBox( INT_MAX ) );
                    double xshift = -tbbox.GetMinX();
                    double yshift = -tbbox.GetMinY();
                    m_contentWorld = a2dAffineMatrix( xshift, yshift );
                }
                break;

            case ScaledContent:
            case ScaledContentKeepAspect:
                if ( m_content )
                {
                    a2dBoundingBox tbbox;
                    tbbox.Expand( m_content->GetCalculatedBoundingBox( INT_MAX ) );

                    // This code even works, if 1 content dimension is zero (e.g. a line)
                    // In this case, the content is centered
                    double
                    scaleX = tbbox.GetWidth() ? ( m_maxx - m_minx - 2 * m_contentBorder ) / tbbox.GetWidth() : 0,
                    scaleY = tbbox.GetHeight() ? ( m_maxy - m_miny - 2 * m_contentBorder ) / tbbox.GetHeight() : 0;

                    if( m_mode == ScaledContentKeepAspect )
                    {
                        if( scaleX && fabs( scaleX ) < fabs( scaleY ) )
                            scaleY = scaleX;
                        else if( scaleY && fabs( scaleY ) < fabs( scaleX ) )
                            scaleX = scaleY;
                    }

                    m_contentWorld = a2dAffineMatrix(
                                         0.5 * ( m_maxx - m_minx ) - 0.5 * scaleX * ( tbbox.GetMinX() + tbbox.GetMaxX() ),
                                         0.5 * ( m_maxy - m_miny ) - 0.5 * scaleY * ( tbbox.GetMinY() + tbbox.GetMaxY() ),
                                         scaleX,
                                         scaleY
                                     );
                }

                break;
            default:
                break;
        }
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

a2dBoundingBox a2dWidgetButton::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    switch( m_mode )
    {
        case Boundingbox:
            if ( m_content )
            {
                a2dBoundingBox tbbox;
                tbbox.Expand( m_content->GetCalculatedBoundingBox( INT_MAX ) );
                bbox.Expand( tbbox );
            }

            bbox.Expand( 0, 0 );
            bbox.Expand( m_maxx - m_minx , m_maxy - m_miny );
            break;

        case BoundingboxSize:
            if ( m_content )
            {
                a2dBoundingBox tbbox;
                tbbox.Expand( m_content->GetCalculatedBoundingBox( INT_MAX ) );
                double xshift = -tbbox.GetMinX();
                double yshift = -tbbox.GetMinY();

                tbbox.Translate( xshift, yshift );
                bbox.Expand( tbbox );
            }

            bbox.Expand( 0, 0 );
            bbox.Expand( m_maxx - m_minx , m_maxy - m_miny );
            bbox.Enlarge( m_contentBorder );
            break;

        case ScaledContent:
        case ScaledContentKeepAspect:

            bbox.Expand( 0, 0 );
            bbox.Expand( m_maxx - m_minx , m_maxy - m_miny );
            break;

        default:
            break;
    }
    return bbox;
}

void a2dWidgetButton::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    a2dWindowMM::DoRender( ic, clipparent );

    if ( m_content )
    {
        bool old = m_content->GetChildrenOnSameLayer();
        a2dAffineMatrix oldMatrix = ic.GetDrawer2D()->GetTransform();

        m_content->SetChildrenOnSameLayer( true );

        a2dIterCU cu( ic, m_contentWorld );
        m_content->Render( ic, clipparent );

        m_content->SetChildrenOnSameLayer( old );
        ic.GetDrawer2D()->SetTransform( oldMatrix );
    }

    if( IsSelected() )
    {
        if( !m_selectedFill.IsNoFill() )
            ic.GetDrawer2D()->SetDrawerFill( m_selectedFill );
        if( !m_selectedStroke.IsNoStroke() )
            ic.GetDrawer2D()->SetDrawerStroke( m_selectedStroke );

        // This is needed, because the default selected drawing is disabled
        a2dBoundingBox bbox = DoGetUnTransformedBbox();
        ic.GetDrawer2D()->DrawRoundedRectangle( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth(), bbox.GetHeight(), 0 );
    }
}

void a2dWidgetButton::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    m_flags.m_HighLight = true;
    SetPending( true );
    event.Skip(); //e.g. for base class object tip
}

void a2dWidgetButton::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    m_flags.m_HighLight = false;
    SetPending( true );
    event.Skip(); //e.g. for base class object tip
}

void a2dWidgetButton::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    if ( event.GetMouseEvent().Moving() )
    {
        event.Skip();
//        m_root->GetCanvasView()->SetCursor( GetHabitat()->GetCursor( a2dCURSOR_HAND ) );
    }
    else if ( event.GetMouseEvent().LeftDown() )
    {
        wxCommandEvent commandEvent( wxEVT_COMMAND_BUTTON_CLICKED, m_buttonId );
        commandEvent.SetEventObject( this );
        if ( !ProcessEvent( commandEvent ) && m_parent )
            m_parent->ProcessEvent( commandEvent );
    }
    else if ( event.GetMouseEvent().LeftUp() )
    {
    }
    else
    {
        event.Skip();
    }
}

bool a2dWidgetButton::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    if ( m_content )
    {
        a2dIterCU cu( ic, m_contentWorld );
        if ( m_content->IsHitWorld( ic, hitEvent ) != 0 )
            return true;
    }

    double margin = ic.GetTransformedHitMargin();

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, m_minx, m_miny, m_maxx, m_maxy, ic.GetTransformedHitMargin() + margin );

    return hitEvent.m_how.IsHit();
}


BEGIN_EVENT_TABLE( a2dWidgetButtonGroup, a2dCanvasObject )
    EVT_BUTTON_ANY( a2dWidgetButtonGroup::OnAnyButton )
END_EVENT_TABLE()

a2dWidgetButtonGroup::a2dWidgetButtonGroup( a2dCanvasObject* WXUNUSED( parent ), double x, double y, float extra )
    : a2dCanvasObject( x, y )
{
    m_worldExtend = extra;
    m_singleSelect = true;
}

a2dWidgetButtonGroup::~a2dWidgetButtonGroup()
{
}

void a2dWidgetButtonGroup::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    a2dAffineMatrix invlworld = m_lworld;
    invlworld.Invert();

    a2dAffineMatrix oldMatrix = ic.GetDrawer2D()->GetTransform();
    a2dIterCU cu( ic, invlworld );
    ic.GetDrawer2D()->DrawRoundedRectangle( m_bbox.GetMinX(), m_bbox.GetMinY(), m_bbox.GetWidth(), m_bbox.GetHeight() , 0 );
    ic.GetDrawer2D()->SetTransform( oldMatrix );
}

void a2dWidgetButtonGroup::OnAnyButton( wxCommandEvent& event )
{
    a2dCanvasObject* hit = wxStaticCast( event.GetEventObject(), a2dCanvasObject );

    if( m_singleSelect )
    {
        a2dWalker_SetPendingFlags setflags( a2dCanvasOFlags::SELECTED );
        setflags.Start( m_root->GetRootObject(), true );
        a2dWalker_SetSpecificFlagsCanvasObjects setflags2( a2dCanvasOFlags::SELECTED );
        setflags2.SetSkipNotRenderedInDrawing( true );
        setflags2.Start( m_root->GetRootObject(), false );

        hit->SetSelected( true );
    }
    else
    {
        if ( hit->IsSelected() )
            hit->SetSelected( false );
        else
            hit->SetSelected( true );
    }

    hit->SetPending( true );

    if( hit->IsSelected() )
    {
        // If hit is a a2dWidgetButton (it usually is), get the button id
        a2dWidgetButton* hitButton = wxDynamicCast( event.GetEventObject(), a2dWidgetButton );
        int buttonId = hitButton ? hitButton->GetButtonId() : 0;

        wxCommandEvent commandEvent( wxEVT_COMMAND_CHOICE_SELECTED, buttonId );
        commandEvent.SetEventObject( hit );
        hit->ProcessEvent( commandEvent );
    }

    event.Skip();
}


BEGIN_EVENT_TABLE( a2dWidgetButtonCommand, a2dWidgetButton )
    EVT_CHOICE_ANY( a2dWidgetButtonCommand::OnAnyChoice )
END_EVENT_TABLE()

a2dWidgetButtonCommand::a2dWidgetButtonCommand( a2dCanvasObject* parent, int buttonId, double x, double y, double w, double h, WidgetMode mode )
    : a2dWidgetButton( parent, buttonId, x, y, w, h, mode )
{
    m_docviewCommand = 0;
    m_event = 0;
    m_clientData = 0;
}

a2dWidgetButtonCommand::~a2dWidgetButtonCommand()
{
    if( m_docviewCommand )
        delete m_docviewCommand;
    if( m_event )
        delete m_event;
    if( m_clientData )
        delete m_clientData;
}

void a2dWidgetButtonCommand::SetCanvasCommand( class a2dCommand* command )
{
    if( m_docviewCommand )
        delete m_docviewCommand;

    m_docviewCommand = command;
}

void a2dWidgetButtonCommand::SetEvent( wxEvent* event )
{
    if( m_event )
        delete m_event;

    m_event = event;
}

void a2dWidgetButtonCommand::OnAnyChoice( wxCommandEvent& event )
{
    if( event.GetEventObject() == this )
    {
        if( m_docviewCommand )
        {
            assert( 0 );
            //!todo, is undo info here needed, e.g. what is the inverse of command/event sent around.?
            /*
                        a2dDocumentCommandProcessor *cp = a2dDocviewGlobals->GetDocviewCommandProcessor();

                        if( cp )
                        {
                            a2dCentralCanvasCommandProcessor *ccp = wxDynamicCast( cp, a2dCentralCanvasCommandProcessor );
                            if( ccp )
                            {
                                a2dCanvasObjectList objects;
                                ccp->GetParentObject()->CollectObjects( &objects, wxT(""), a2dCANOBJ_SELECTED );
                                ccp->GetParentObject()->CollectObjects( &objects, wxT(""), a2dCANOBJ_Editing );
                                ccp->GetParentObject()->CollectObjects( &objects, wxT(""), a2dCANOBJ_EditingCopy );

                                a2dCanvasDocument* CheckCanvasDocument();

                                forEachIn( a2dCanvasObjectList, &objects )
                                {
                                    ccp->SubmitMultiple( m_docviewCommand->CloneAndBind( 0 ), &objects );
                                }
                            }
                            else
                            {
                                cp->Submit( m_docviewCommand->CloneAndBind( 0 ) );
                            }
                        }
            */
        }

        if( m_event )
        {
            wxMDIParentFrame* mdiFrame = wxDynamicCast( wxTheApp->GetTopWindow(), wxMDIParentFrame );

            if ( mdiFrame && mdiFrame->GetActiveChild() )
                mdiFrame->GetActiveChild()->GetEventHandler()->ProcessEvent( *m_event );
            else
                wxTheApp->GetTopWindow()->GetEventHandler()->ProcessEvent( *m_event );
        }
    }

    event.Skip();
}

IMPLEMENT_CLASS( a2dWidgetSlider, a2dWindowMM )

BEGIN_EVENT_TABLE( a2dWidgetSlider, a2dWindowMM )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dWidgetSlider::OnCanvasObjectMouseEvent )
    EVT_CANVASOBJECT_ENTER_EVENT( a2dWidgetSlider::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT( a2dWidgetSlider::OnLeaveObject )
    EVT_SLIDER( -1, a2dWidgetSlider::OnSliderEvent )
END_EVENT_TABLE()

a2dWidgetSlider::a2dWidgetSlider( a2dCanvasObject* parent, int sliderId, double x, double y, double w, double h )
    : a2dWindowMM( x, y, w, h )
{
    m_parent = parent;
    m_selectedStroke = *a2dBLACK_STROKE;
    m_selectedFill = *a2dTRANSPARENT_FILL;

    m_highLightStroke = *a2dNullSTROKE;
    m_highLightFill = *a2dNullFILL;

    SetStroke( *a2dTRANSPARENT_STROKE );
    SetFill( *a2dTRANSPARENT_FILL );
    m_sliderId = sliderId;
    m_position = 0;
    m_max = 100;
    m_min = 0;
    m_handleH = h *0.05;
    m_handleW = w *0.9;
    m_slideW = w *0.05;
}

a2dWidgetSlider::a2dWidgetSlider( const a2dWidgetSlider& other, CloneOptions options, a2dRefMap* refs )
    : a2dWindowMM( other, options, refs )
{
    m_parent = other.m_parent;
    m_selectedStroke = other.m_selectedStroke;
    m_selectedFill = other.m_selectedFill;
    m_highLightStroke = other.m_highLightStroke;
    m_highLightFill = other.m_highLightFill;
    m_sliderId = other.m_sliderId;
    m_position = other.m_position;
    m_max = other.m_max;
    m_min = other.m_min;
    m_handleH = other.m_handleH;
    m_handleW = other.m_handleW;
    m_slideW =  other.m_slideW;
}

a2dObject* a2dWidgetSlider::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dWidgetSlider( *this, options, refs );
};

a2dWidgetSlider::~a2dWidgetSlider()
{
}

void a2dWidgetSlider::DrawHighLighted( a2dIterC& ic )
{
    if ( !m_highLightStroke.IsNoStroke() )
        ic.GetDrawer2D()->SetDrawerStroke( m_highLightStroke );
    else
        ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetHighLightStroke() );

    if ( !m_highLightFill.IsNoFill() )
        ic.GetDrawer2D()->SetDrawerFill( m_highLightFill );
    else
        ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetHighLightFill() );

    a2dStroke m_shadowStroke = a2dStroke( *wxBLACK, 0, a2dSTROKE_SOLID );
    a2dStroke m_lightStroke = a2dStroke( *wxWHITE, 0, a2dSTROKE_SOLID );

    double x1 = m_bbox.GetMinX();
    double x2 = m_bbox.GetMaxX();
    double y1 = m_bbox.GetMinY();
    double y2 = m_bbox.GetMaxY();
    ic.GetDrawer2D()->SetDrawerStroke( m_shadowStroke );
    ic.GetDrawer2D()->DrawLine( x1, y1, x1, y2 );
    ic.GetDrawer2D()->DrawLine( x1, y2, x2, y2 );
    ic.GetDrawer2D()->SetDrawerStroke( m_lightStroke );
    ic.GetDrawer2D()->DrawLine( x2, y1, x2, y2 );
    ic.GetDrawer2D()->DrawLine( x2, y1, x1, y1 );
}


void a2dWidgetSlider::Render( a2dIterC& ic, OVERLAP clipparent )
{

    // a2dWidgetSlider does its own selection drawing, so base class behaviour is not wanted
    //if( ic.GetDrawStyle() != RenderWIREFRAME_SELECT && ic.GetDrawStyle() != RenderWIREFRAME_SELECT_INVERT )
        a2dWindowMM::Render( ic, clipparent );

}


bool a2dWidgetSlider::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

void a2dWidgetSlider::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    a2dWindowMM::DoRender( ic, clipparent );
    ic.GetDrawer2D()->OverRuleFixedStyle();

    a2dDrawer2D* draw = ic.GetDrawer2D();
    //double x1 = m_bbox.GetMinX();
    //double x2 = m_bbox.GetMaxX();
    //double y1 = m_bbox.GetMinY();
    //double y2 = m_bbox.GetMaxY();
    double x1 = m_minx;
    double x2 = m_maxx;
    double y1 = m_miny;
    double y2 = m_maxy;

    
    double sliderH = (y2-y1)-m_handleH*2;
    double xm = ( x1 + x2 )/ 2.0;
    a2dStroke SliderStroke = a2dStroke( *wxRED, 0, a2dSTROKE_SOLID );
    a2dFill   SliderFill = a2dFill( *wxRED, a2dFILL_SOLID );
    draw->SetDrawerStroke( SliderStroke );
    draw->SetDrawerFill( SliderFill );

    double ySlider = (m_position - m_min) / double (m_max-m_min) * sliderH + m_handleH + y1;
    draw->DrawRoundedRectangle( xm-m_slideW/2.0, y1 + m_handleH, m_slideW, sliderH, 0 );
    SliderStroke = a2dStroke( *wxBLACK,0, a2dSTROKE_SOLID );
    draw->SetDrawerStroke( SliderStroke );

    draw->DrawRoundedRectangle( x1 + ((x2-x1) - m_handleW)/2.0 , ySlider-m_handleH/2.0, m_handleW, m_handleH, 0 );
    ic.GetDrawer2D()->ReStoreFixedStyle();

    if( IsSelected() )
    {
        if( !m_selectedFill.IsNoFill() )
            ic.GetDrawer2D()->SetDrawerFill( m_selectedFill );
        if( !m_selectedStroke.IsNoStroke() )
            ic.GetDrawer2D()->SetDrawerStroke( m_selectedStroke );

        // This is needed, because the default selected drawing is disabled
        a2dBoundingBox bbox = DoGetUnTransformedBbox();
        ic.GetDrawer2D()->DrawRoundedRectangle( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth(), bbox.GetHeight(), 0 );
    }
}

void a2dWidgetSlider::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    m_flags.m_HighLight = true;
    SetPending( true );
    event.Skip(); //e.g. for base class object tip
}

void a2dWidgetSlider::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    m_flags.m_HighLight = false;
    SetPending( true );
    event.Skip(); //e.g. for base class object tip
}

bool a2dWidgetSlider::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        if ( editmode == 1 )
        {
            PROPID_Allowrotation->SetPropertyToObject( this, false );
            PROPID_Allowskew->SetPropertyToObject( this, false );

            a2dCanvasObject* original = PROPID_Original->GetPropertyValue( this );
            original->SetVisible( false );

            //calculate bbox's else mouse events may take place when first idle event is not yet
            //processed to do this calculation.
            Update( updatemask_force );
            //still set it pending to do the redraw ( in place )
            SetPending( true );
            return true;
        }
        else
        {
            PROPID_IncludeChildren->SetPropertyToObject( this, false );
            PROPID_Allowrotation->SetPropertyToObject( this, false );
            PROPID_Allowskew->SetPropertyToObject( this, false );

            return a2dCanvasObject::DoStartEdit( editmode, editstyle );
        }
    }

    return false;
}

void a2dWidgetSlider::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    double xw, yw;
    double xlw, ylw;
    xw = event.GetX();
    yw = event.GetY();
    a2dIterC* ic = event.GetIterC();
    a2dIterCU cu( *ic, this );
    // prepare relative to object coordinates for derived objects
    ic->GetInverseTransform().TransformPoint( xw, yw, xlw, ylw );

    if ( m_flags.m_editingCopy && m_flags.m_editable )
    {
        a2dWidgetSlider* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dWidgetSlider );
        static bool drag = false;

        double x1 = m_bbox.GetMinX();
        double x2 = m_bbox.GetMaxX();
        double y1 = m_bbox.GetMinY();
        double y2 = m_bbox.GetMaxY();
        double sliderH = (y2-y1)-m_handleH*2;
        double xm = ( x1 + x2 )/ 2.0;
        double ySlider = (m_position - m_min) / double (m_max-m_min) * sliderH + m_handleH + y1;

        a2dHit hit;
        hit = HitTestRectangle( xw, yw, xm - m_handleW/2.0,ySlider-m_handleH/2.0, xm + m_handleW/2.0, ySlider + m_handleH/2.0, 0 );
        if ( drag || hit.IsHit() )
        {        
            if ( event.GetMouseEvent().Moving() )
            {
                ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_HAND ) );
            }
            else if ( event.GetMouseEvent().LeftDown() &&  IsDraggable() )
            {
                // Corridor is already in place, since edit tool is active.
                ic->GetDrawingPart()->PushCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Drag ) );
            }
            else if ( IsDraggable() && event.GetMouseEvent().Dragging() && ic->GetDrawingPart()->GetEndCorridorObject() == this )
            {
                drag = true;
                double sliderH = (y2-y1)-m_handleH*2;
                if ( yw < y1 + m_handleH )
                    yw = y1 + m_handleH;
                if ( yw > y2 - m_handleH )
                    yw = y2 - m_handleH;

                m_position = (yw-y1-m_handleH) / sliderH * (m_max - m_min );  
                SetPending( true );
                wxScrollEvent scrollEvent( wxEVT_SCROLL_CHANGED, m_sliderId );
                scrollEvent.SetEventObject( this );
                if ( !original->ProcessEvent( scrollEvent ) && m_parent )
                    m_parent->ProcessEvent( scrollEvent );    
            }
            else if ( event.GetMouseEvent().LeftUp() && ic->GetDrawingPart()->GetEndCorridorObject() == this )
            {
                drag = false;
                double sliderH = (y2-y1)-m_handleH*2;
                if ( yw < y1 + m_handleH )
                    yw = y1 + m_handleH;
                if ( yw > y2 - m_handleH )
                    yw = y2 - m_handleH;

                ic->GetDrawingPart()->PopCursor();
                    
                m_position = (yw-y1-m_handleH) / sliderH * (m_max - m_min );  
                SetPending( true );
                original->SetPosition( m_position );
                wxCommandEvent commandEvent( wxEVT_SLIDER, m_sliderId );
                commandEvent.SetEventObject( this );
                if ( !original->ProcessEvent( commandEvent ) && m_parent )
                    m_parent->ProcessEvent( commandEvent );    
            }
            else
            {
                event.Skip();
            }
        }
        else
        {
            /*
            if ( event.GetMouseEvent().Moving() )
            {
                if ( event.m_how.IsHit() )
                {
                    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CanvasObjectHit ) );
                }
                else
                {
                    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Edit ) );
                }
            }
            else
            */
                event.Skip();
        }
    }
    else
        event.Skip();
}

void a2dWidgetSlider::OnSliderEvent( wxCommandEvent& event )
{
}

bool a2dWidgetSlider::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double margin = ic.GetTransformedHitMargin();

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, m_minx, m_miny, m_maxx, m_maxy, ic.GetTransformedHitMargin() + margin );

    return hitEvent.m_how.IsHit();
}

