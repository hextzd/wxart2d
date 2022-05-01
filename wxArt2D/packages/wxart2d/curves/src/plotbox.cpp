/*! \file curves/src/plotbox.cpp
    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: plotbox.cpp,v 1.86 2008/07/30 21:54:02 titato Exp $
*/

// plotbox.cpp: implementation of the plotbox class.
//
//////////////////////////////////////////////////////////////////////

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/curves/plotbox.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/canvas.h"
#include "wx/canvas/sttool.h"
#include "wx/canvas/edit.h"

IMPLEMENT_DYNAMIC_CLASS( a2dPlot, a2dCanvasXYDisplayGroup )
IMPLEMENT_DYNAMIC_CLASS( a2dCurveGroupLegend, a2dCanvasObject )

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define text_HEIGHT_FACTOR (1.0/40)
#define padding_FACTOR (1.0/100)

BEGIN_EVENT_TABLE( a2dPlot, a2dCanvasXYDisplayGroup )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dPlot::OnCanvasObjectMouseEvent )
END_EVENT_TABLE()

a2dPlot::a2dPlot( double x, double y )
    : a2dCanvasXYDisplayGroup( x, y )
{
    m_topPadding = 0;
    m_bottomPadding = 0;
    m_leftPadding = 0;
    m_rightPadding = 0;
    m_autoPlace = true;
    m_autoYNames = false;
}

a2dPlot::~a2dPlot()
{
}

a2dPlot::a2dPlot( const a2dPlot& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasXYDisplayGroup( other, options, refs )
{
    m_topPadding = other.m_topPadding;
    m_bottomPadding = other.m_bottomPadding;
    m_leftPadding = other.m_leftPadding;
    m_rightPadding = other.m_rightPadding;
    m_autoPlace = other.m_autoPlace;
    m_autoYNames = other.m_autoYNames;
}

a2dObject* a2dPlot::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dPlot( *this, options, refs );
}

void a2dPlot::SetPadding( double leftPadding, double rightPadding, double topPadding, double bottomPadding )
{
    m_topPadding = topPadding;
    m_bottomPadding = bottomPadding;
    m_leftPadding = leftPadding;
    m_rightPadding = rightPadding;
}

void a2dPlot::SetTitle( const wxString& title )
{
    a2dText* textobj = ( a2dText* ) Find( _T( "__TITLE__" ) );
    if ( title.IsEmpty() )
        ReleaseChild( textobj );
    else
    {
        if ( !textobj )
        {
            textobj = new a2dText( title, 0, 0, a2dFont( 10, wxFONTFAMILY_SWISS ), 0 );
            textobj->SetSpecificFlags( !m_autoPlace, a2dCanvasOFlags::DRAGGABLE );
            textobj->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD );

            Append( textobj );
            textobj->SetName( _T( "__TITLE__" ) );
        }
        else
            textobj->SetText( title );
    }
}

void a2dPlot::SetXLabel( const wxString& xlabel, const wxColour& color )
{
    a2dText* textobj = ( a2dText* ) Find( _T( "__XLABEL__" ) );
    if ( xlabel.IsEmpty() )
        ReleaseChild( textobj );
    else
    {
        if ( !textobj )
        {
            textobj = new a2dText( xlabel, 0, 0, a2dFont( 10, wxFONTFAMILY_SWISS ), 0 );
            textobj->SetSpecificFlags( !m_autoPlace, a2dCanvasOFlags::DRAGGABLE );
            textobj->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD );
            Append( textobj );
            textobj->SetName(  _T( "__XLABEL__" ) );
        }
        else
            textobj->SetText( xlabel );
        if( color != wxNullColour )
            textobj->SetStroke( color );
    }
}

void a2dPlot::SetY1Label( const wxString& ylabel, const wxColour& color )
{
    a2dText* textobj = ( a2dText* ) Find( _T( "__Y1LABEL__" ) );
    if ( ylabel.IsEmpty() )
        ReleaseChild( textobj );
    else
    {
        if ( !textobj )
        {
            textobj =  new a2dText( ylabel, 0, 0,  a2dFont( 10, wxFONTFAMILY_SWISS ), 0 );
            textobj->SetSpecificFlags( !m_autoPlace, a2dCanvasOFlags::DRAGGABLE );
            textobj->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD );
            Append( textobj );
            textobj->SetName( _T( "__Y1LABEL__" ) );
        }
        else
            textobj->SetText( ylabel );
        if( color != wxNullColour )
            textobj->SetStroke( color );
    }
}

void a2dPlot::SetY2Label( const wxString& ylabel, const wxColour& color )
{
    a2dText* textobj = ( a2dText* ) Find( _T( "__Y2LABEL__" ) );
    if ( ylabel.IsEmpty() )
        ReleaseChild( textobj );
    else
    {
        if ( !textobj )
        {
            textobj =  new a2dText( ylabel, 0, 0, a2dFont( 10, wxFONTFAMILY_SWISS ), 0 );
            textobj->SetSpecificFlags( !m_autoPlace, a2dCanvasOFlags::DRAGGABLE );
            textobj->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD );
            Append( textobj );
            textobj->SetName( _T( "__Y2LABEL__" ) );
        }
        else
            textobj->SetText( ylabel );
        if( color != wxNullColour )
            textobj->SetStroke( color );
    }
}

void a2dPlot::SetTitleTextDc( a2dText* title )
{
    if ( title )
    {
        title->SetName( _T( "__TITLE__" ) );
        title->SetSpecificFlags( !m_autoPlace, a2dCanvasOFlags::DRAGGABLE );
        title->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD );
        if ( !SwitchChildNamed( _T( "__TITLE__" ) , title ) )
            Append( title );
    }
    else
    {
        a2dText* textobj = ( a2dText* ) Find( _T( "__TITLE__" ) );
        ReleaseChild( textobj );
    }
}

void a2dPlot::SetMarkerShow( a2dMarkerShow* showm )
{
    if ( showm )
    {
        showm->SetName( _T( "__SHOWM__" ) );
        showm->SetSpecificFlags( !m_autoPlace, a2dCanvasOFlags::DRAGGABLE );
        if ( !SwitchChildNamed( _T( "__SHOWM__" ) , showm ) )
            Append( showm );
    }
    else
    {
        a2dMarkerShow* showobj = ( a2dMarkerShow* ) Find( _T( "__SHOWM__" ) );
        ReleaseChild( showobj );
    }
}

void a2dPlot::SetXLabelTextDc( a2dText* xLabelTextDc )
{
    if ( xLabelTextDc )
    {
        xLabelTextDc->SetName( _T( "__XLABEL__" ) );
        xLabelTextDc->SetSpecificFlags( !m_autoPlace, a2dCanvasOFlags::DRAGGABLE );
        xLabelTextDc->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD );
        if ( !SwitchChildNamed( _T( "__XLABEL__" ), xLabelTextDc ) )
            Append( xLabelTextDc );
    }
    else
    {
        a2dText* textobj = ( a2dText* ) Find( _T( "__XLABEL__" ) );
        ReleaseChild( textobj );
    }
}

void a2dPlot::SetY1LabelTextDc( a2dText* yLabelTextDc )
{
    if ( yLabelTextDc )
    {
        yLabelTextDc->SetName( _T( "__Y1LABEL__" ) );
        yLabelTextDc->SetSpecificFlags( !m_autoPlace, a2dCanvasOFlags::DRAGGABLE );
        yLabelTextDc->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD );
        if ( !SwitchChildNamed( _T( "__Y1LABEL__" ), yLabelTextDc ) )
            Append( yLabelTextDc );
    }
    else
    {
        a2dText* textobj = ( a2dText* ) Find( _T( "__Y1LABEL__" ) );
        ReleaseChild( textobj );
    }
}

void a2dPlot::SetY2LabelTextDc( a2dText* yLabelTextDc )
{
    if ( yLabelTextDc )
    {
        yLabelTextDc->SetName( _T( "__Y2LABEL__" ) );
        yLabelTextDc->SetSpecificFlags( !m_autoPlace, a2dCanvasOFlags::DRAGGABLE );
        yLabelTextDc->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD );
        if ( !SwitchChildNamed( _T( "__Y2LABEL__" ), yLabelTextDc ) )
            Append( yLabelTextDc );
    }
    else
    {
        a2dText* textobj = ( a2dText* ) Find( _T( "__Y2LABEL__" ) );
        ReleaseChild( textobj );
    }
}

void a2dPlot::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    a2dCanvasXYDisplayGroup::DoRender( ic, clipparent );
}

a2dBoundingBox a2dPlot::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;

    //the next is fast, it does not recalculate curves etc.
    bbox.Expand( a2dCanvasXYDisplayGroup::DoGetUnTransformedBbox() );

    if ( m_autoPlace )
    {
        a2dText* titleTextDc = ( a2dText* ) Find( _T( "__TITLE__" ) );
        a2dText* xLabelTextDc = ( a2dText* ) Find( _T( "__XLABEL__" ) );
        a2dText* y1LabelTextDc = ( a2dText* ) Find( _T( "__Y1LABEL__" ) );
        a2dText* y2LabelTextDc = ( a2dText* ) Find( _T( "__Y2LABEL__" ) );
        a2dMarkerShow* markerShow = ( a2dMarkerShow* ) Find( _T( "__SHOWM__" ) );

        if ( titleTextDc )
        {
            a2dBoundingBox tbox = titleTextDc->GetBbox();
            tbox.EnlargeXY( m_leftPadding, m_topPadding );
            bbox.Expand( tbox );
        }

        if ( xLabelTextDc )
        {
            a2dBoundingBox tbox = xLabelTextDc->GetBbox();
            tbox.EnlargeXY( m_leftPadding, m_bottomPadding );
            bbox.Expand( tbox );
        }

        if ( y1LabelTextDc )
        {
            a2dBoundingBox tbox = y1LabelTextDc->GetBbox();
            tbox.EnlargeXY( m_leftPadding, m_bottomPadding );
            bbox.Expand( tbox );
        }

        if ( y2LabelTextDc )
        {
            a2dBoundingBox tbox = y2LabelTextDc->GetBbox();
            tbox.EnlargeXY( m_rightPadding, m_bottomPadding );
            bbox.Expand( tbox );
        }

        if ( markerShow )
        {
            a2dBoundingBox tbox = markerShow->GetBbox();
            tbox.EnlargeXY( m_leftPadding, m_bottomPadding );
            bbox.Expand( tbox );
        }
    }

    return bbox;
}

bool a2dPlot::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    //call base class ( so the m_axisarea & its curves will be updated )
    calc = a2dCanvasXYDisplayGroup::DoUpdate( mode, childbox, clipbox, propbox );

    if ( !m_bbox.GetValid() || calc )
    {
        // auto Ynames is taking the a2dCurveArea that is used to display the Yaxis at the moment
        if( m_autoYNames )
        {
            if( m_axesarealist->m_leftAxisY != ( a2dCurvesArea* ) NULL )
            {
                wxString axisName = m_axesarealist->m_leftAxisY->GetAxisText();
                wxColour areacolor = m_axesarealist->m_leftAxisY->GetColor();
                if( !axisName.IsEmpty() )
                    SetY1Label( axisName, areacolor );
                else
                    SetY1Label( wxT( "-" ), *wxBLACK );
            }
            if( m_axesarealist->m_rightAxisY != ( a2dCurvesArea* ) NULL )
            {
                wxString axisName = m_axesarealist->m_rightAxisY->GetAxisText();
                wxColour areacolor = m_axesarealist->m_rightAxisY->GetColor();
                if( !axisName.IsEmpty() )
                    SetY2Label( axisName, areacolor );
                else
                    SetY2Label( wxT( "-" ), *wxBLACK );
            }
        }

        // get the untransformed bounding box of the base a2dCanvasXYDisplayGroup
        m_bbox.SetValid( false );
        m_bbox.Expand( a2dCanvasXYDisplayGroup::DoGetUnTransformedBbox() );

        // first get the bounding box corner points of the axesRect in plotbox world coordinates.
        double minx, miny;
        double maxx, maxy;

        minx = m_bbox.GetMinX();
        maxx = m_bbox.GetMaxX();
        miny = m_bbox.GetMinY();
        maxy = m_bbox.GetMaxY();

        double width = m_bbox.GetWidth();
        double height = m_bbox.GetHeight();

        a2dText* titleTextDc = ( a2dText* ) Find( _T( "__TITLE__" ) );
        a2dText* xLabelTextDc = ( a2dText* ) Find( _T( "__XLABEL__" ) );
        a2dText* y1LabelTextDc = ( a2dText* ) Find( _T( "__Y1LABEL__" ) );
        a2dText* y2LabelTextDc = ( a2dText* ) Find( _T( "__Y2LABEL__" ) );
        a2dMarkerShow* markerShow = ( a2dMarkerShow* ) Find( _T( "__SHOWM__" ) );

        // set appropriate top padding if not specified
        if( m_topPadding == 0 )
            m_topPadding = height * padding_FACTOR;

        // set appropriate bottom padding if not specified
        if( m_bottomPadding == 0 )
            m_bottomPadding =  height * padding_FACTOR;

        // set appropriate left padding if not specified
        if( m_leftPadding == 0 )
            m_leftPadding = width * padding_FACTOR;

        // set appropriate left padding if not specified
        if( m_rightPadding == 0 )
            m_rightPadding = width * padding_FACTOR;

        if ( m_autoPlace )
        {
            if ( titleTextDc )
            {
                //            titleTextDc->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD );
                // set the appropriate title text height
                //          if(titleTextDc->GetTextHeight() == 1)
                //              titleTextDc->SetTextHeight( height*text_HEIGHT_FACTOR );

                // position title in centered on the top
                titleTextDc->SetAlignment( wxMIDX | wxMINY );
                // Get y-offset for multiline heading
                //! \bug display bug: solve editing of multiple lines
                double dy = 0; //( titleTextDc->GetLines() - 1 ) * ( titleTextDc->GetLineHeight() + titleTextDc->GetLineSpacing() );
                titleTextDc->SetPosXY( minx + width / 2.0, maxy + m_topPadding + dy );
                titleTextDc->Update( updatemask_normal );
                a2dDrawingPart* view = wxStaticCastNull( PROPID_ViewDependent->GetPropertyValue( titleTextDc ).Get(), a2dDrawingPart );
                // it is for viewDependent objects in the title of the plot
                if( view && titleTextDc->GetBbox().GetWidth() > m_bbox.GetWidth() )
                {
                    titleTextDc->GetBbox().SetMin( minx, titleTextDc->GetBbox().GetMinY() );
                    titleTextDc->GetBbox().SetMax( maxx, titleTextDc->GetBbox().GetMaxY() );
                }
                a2dBoundingBox tbox = titleTextDc->GetBbox();
                //          if ( !prop || !prop->GetRefObject())
                //          {
                //              double lineHeight = titleTextDc->GetTextHeight();
                //              tbox.Enlarge( lineHeight / 2 );
                //          }
                tbox.EnlargeXY( m_leftPadding, m_topPadding );
                m_bbox.Expand( tbox );
            }

            if ( xLabelTextDc )
            {
                //            xLabelTextDc->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD );
                // set the appropriate x label text height
                //            if(xLabelTextDc->GetTextHeight() == 1)
                //                    xLabelTextDc->SetTextHeight( height*text_HEIGHT_FACTOR );

                // position xlabel at the bottom - rotation not handled
                xLabelTextDc->SetAlignment( wxMIDX | wxMAXY );
                xLabelTextDc->SetPosXY( minx + width / 2,
                                        miny - m_bottomPadding - xLabelTextDc->GetTextHeight() );
                xLabelTextDc->Update( updatemask_normal );
                a2dDrawingPart* view = wxStaticCastNull( PROPID_ViewDependent->GetPropertyValue( xLabelTextDc ).Get(), a2dDrawingPart );
                // it is for viewDependent objects in the XLABEL of the plot
                if( view && xLabelTextDc->GetBbox().GetWidth() > m_bbox.GetWidth() )
                {
                    xLabelTextDc->GetBbox().SetMin( minx, xLabelTextDc->GetBbox().GetMinY() );
                    xLabelTextDc->GetBbox().SetMax( maxx, xLabelTextDc->GetBbox().GetMaxY() );
                }
                a2dBoundingBox tbox = xLabelTextDc->GetBbox();
                //          if ( !prop || !prop->GetRefObject())
                //          {
                //              double lineHeight = xLabelTextDc->GetTextHeight();
                //              tbox.Enlarge( lineHeight / 4 );
                //          }
                tbox.EnlargeXY( m_leftPadding, m_bottomPadding );
                m_bbox.Expand( tbox );
            }

            if ( y1LabelTextDc )
            {
                SetPosYLabel( y1LabelTextDc, minx - m_leftPadding, miny + height / 2, height, true );
                a2dDrawingPart* view = wxStaticCastNull( PROPID_ViewDependent->GetPropertyValue( y1LabelTextDc ).Get(), a2dDrawingPart );
                // it is for viewDependent objects in the XLABEL of the plot
                if( view && y1LabelTextDc->GetBbox().GetHeight() > m_bbox.GetHeight() )
                {
                    y1LabelTextDc->GetBbox().SetMin( y1LabelTextDc->GetBbox().GetMinX(), miny );
                    y1LabelTextDc->GetBbox().SetMax( y1LabelTextDc->GetBbox().GetMaxX(), maxy );
                }
                a2dBoundingBox tbox = y1LabelTextDc->GetBbox();
                //          if ( !prop || !prop->GetRefObject())
                //          {
                //              double lineHeight = y1LabelTextDc->GetTextHeight();
                //              tbox.Enlarge( lineHeight / 4 );
                //          }
                tbox.EnlargeXY( m_leftPadding, m_bottomPadding );
                m_bbox.Expand( tbox );
            }

            if ( y2LabelTextDc )
            {
                SetPosYLabel( y2LabelTextDc, maxx + m_rightPadding, miny + height / 2, height, false );
                a2dDrawingPart* view = wxStaticCastNull( PROPID_ViewDependent->GetPropertyValue( y2LabelTextDc ).Get(), a2dDrawingPart );
                // it is for viewDependent objects in the XLABEL of the plot
                if( view && y2LabelTextDc->GetBbox().GetHeight() > m_bbox.GetHeight() )
                {
                    y2LabelTextDc->GetBbox().SetMin( y2LabelTextDc->GetBbox().GetMinX(), minx );
                    y2LabelTextDc->GetBbox().SetMax( y2LabelTextDc->GetBbox().GetMaxX(), maxy );
                }
                a2dBoundingBox tbox = y2LabelTextDc->GetBbox();
                //          if ( !prop || !prop->GetRefObject())
                //          {
                //              double lineHeight = y2LabelTextDc->GetTextHeight();
                //              tbox.Enlarge( lineHeight / 4 );
                //          }
                tbox.EnlargeXY( m_rightPadding, m_bottomPadding );
                m_bbox.Expand( tbox );
            }

            if ( markerShow )
            {
                markerShow->SetSpecificFlags( false, a2dCanvasOFlags::PRERENDERASCHILD );
                //          // set the appropriate title text height
                //          if(markerShow->GetTextHeight() == 1)
                //              markerShow->SetTextHeight( height*text_HEIGHT_FACTOR );

                markerShow->SetPosXY( minx + width / 2.0, miny );
                markerShow->Update( updatemask_normal );
                a2dBoundingBox tbox = markerShow->GetBbox();
                tbox.EnlargeXY( m_leftPadding, m_bottomPadding );
                m_bbox.Expand( tbox );
            }
        }
        m_bbox.MapBbox( m_lworld );
        return true;
    }

    return false;
}

void a2dPlot::SetPosYLabel( a2dText* yLabelTextDc, double x, double y, double tHeight, bool left )
{
    double yangle = yLabelTextDc->GetTransformMatrix().GetRotation();
    // position ylabel centered on the right - rotation by ±90 degrees handled
    int alignment = wxMIDX;
    if ( yangle == 0 )
        alignment |= left ? wxMAXX : wxMINX;
    else if ( yangle == -90 )
        alignment |= left ? wxMAXY : wxMINY;
    else
        alignment |= left ? wxMINY : wxMAXY;

    yLabelTextDc->SetAlignment( alignment );

    yLabelTextDc->SetPosXY( x, y );
    yLabelTextDc->Update( updatemask_normal );
}

void a2dPlot::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();
    if ( m_flags.m_editingCopy )
    {
        //to world group coordinates to do hit test in world group coordinates
        double xw, yw;
        xw = event.GetX();
        yw = event.GetY();

        if ( event.GetMouseEvent().LeftDown() )
        {
            a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
            if ( IsHitWorld( *ic, hitevent ) )
            {
                a2dPlot* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dPlot );

                a2dIterC iclocal( ic->GetDrawingPart() );
                iclocal.SetLayer( m_layer );
                a2dIterCU cu( iclocal, m_lworld );

                a2dText* titleTextDc = ( a2dText* ) original->Find( _T( "__TITLE__" ) );
                a2dText* xLabelTextDc = ( a2dText* ) original->Find( _T( "__XLABEL__" ) );
                a2dText* y1LabelTextDc = ( a2dText* ) original->Find( _T( "__Y1LABEL__" ) );
                a2dText* y2LabelTextDc = ( a2dText* ) original->Find( _T( "__Y2LABEL__" ) );
                a2dMarkerShow* markerShow = ( a2dMarkerShow* ) Find( _T( "__SHOWM__" ) );

                a2dCanvasObject* hittext = NULL;
                a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
                if ( titleTextDc )
                    hittext = titleTextDc->IsHitWorld( iclocal, hitevent );

                if ( !hittext && xLabelTextDc )
                    hittext = xLabelTextDc->IsHitWorld( iclocal, hitevent );

                if ( !hittext && y1LabelTextDc  )
                    hittext = y1LabelTextDc->IsHitWorld( iclocal, hitevent );

                if ( !hittext && y2LabelTextDc  )
                    hittext = y2LabelTextDc->IsHitWorld( iclocal, hitevent );

                if( !hittext && markerShow )
                    hittext = markerShow->IsHitWorld( iclocal, hitevent );

                if ( hittext && hittext->GetEditable() )
                {
#if wxART2D_USE_EDITOR
                    //a2dIterCU cuw( *ic, m_lworld );
                    a2dIterCU cuw( *ic, original );

                    a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr );
                    ic->SetCorridorPath( true );
                    controller->StartEditingObject( hittext, *ic );
#else //wxART2D_USE_EDITOR
                    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
                }
                else
                    event.Skip();
            }
            else
                event.Skip();
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

#if wxART2D_USE_CVGIO
void a2dPlot::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasXYDisplayGroup::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( _T( "autoPlace" ) , m_autoPlace );
        out.WriteAttribute( _T( "topPadding" ) , m_topPadding );
        out.WriteAttribute( _T( "bottomPadding" ) , m_bottomPadding );
        out.WriteAttribute( _T( "leftPadding" ) , m_leftPadding );
        out.WriteAttribute( _T( "rightPadding" ) , m_rightPadding );
        out.WriteNewLine();
    }
    else
    {
    }
}

void a2dPlot::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasXYDisplayGroup::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_autoPlace = parser.RequireAttributeValueBool( _T( "autoPlace" ) );
        m_topPadding = parser.RequireAttributeValueDouble( _T( "topPadding" ) );
        m_bottomPadding = parser.RequireAttributeValueDouble( _T( "bottomPadding" ) );
        m_leftPadding = parser.RequireAttributeValueDouble( _T( "leftPadding" ) );
        m_rightPadding = parser.RequireAttributeValueDouble( _T( "rightPadding" ) );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO



//----------------------------------------------------------------------------
// a2dCurveGroupLegend
//----------------------------------------------------------------------------

void a2dCurveGroupLegend::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectStart );
    a2dCanvasObject::DoWalker( parent, handler );
    if ( m_curveGroup )
        m_curveGroup->Walker( this, handler );

    handler.WalkTask( parent, this, a2dWalker_a2dDerivedCanvasObjectEnd );
}

BEGIN_EVENT_TABLE( a2dCurveGroupLegend, a2dCanvasObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dCurveGroupLegend::OnCanvasObjectMouseEvent )
    EVT_CHAR( a2dCurveGroupLegend::OnChar )
END_EVENT_TABLE()

void a2dCurveGroupLegend::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    //a2dIterC* ic = event.GetIterC();

    event.Skip();
}

void a2dCurveGroupLegend::OnChar( wxKeyEvent& event )
{
}

a2dCurveGroupLegend::a2dCurveGroupLegend( const wxString& format,  a2dCanvasXYDisplayGroupAreas* curveGroup, const a2dFont& font )
    : a2dCanvasObject()
{
    m_curveGroup = curveGroup;
    m_format = format;
    m_font = font;
    m_linespace = font.GetSize() / 10.0;
}

a2dCurveGroupLegend::~a2dCurveGroupLegend()
{
}

a2dCurveGroupLegend::a2dCurveGroupLegend( const a2dCurveGroupLegend& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_curveGroup = other.m_curveGroup;
    m_font = other.m_font;
    m_format = other.m_format;
    m_linespace = other.m_linespace;
}

a2dObject* a2dCurveGroupLegend::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCurveGroupLegend( *this, options, refs );
};

void a2dCurveGroupLegend::DoAddPending( a2dIterC& WXUNUSED( ic ) )
{
}

void a2dCurveGroupLegend::DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler )  )
{
    if ( !m_flags.m_pending && m_curveGroup->GetPending() )
    {
        SetPending( true );
    }
}

a2dBoundingBox a2dCurveGroupLegend::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;

    double height = 0;

    if ( !m_format.IsEmpty() && m_font.GetSize() )
    {
        const a2dCurvesAreaList& areaList = m_curveGroup->GetCurvesAreaList();
        for( size_t i = 0; i < areaList.GetCount(); i++ )
        {
            const a2dCurvesArea* area = areaList.Item( i );
            a2dCanvasObjectList::const_iterator iter = area->GetChildObjectList()->begin();
            while( iter != area->GetChildObjectList()->end() )
            {
                const a2dCurveObject* item = wxDynamicCast( ( *iter ).Get(), a2dCurve );
                if ( item )
                {
                    wxString form;
                    form.Printf( m_format, item->GetName().c_str() );

                    a2dBoundingBox linebbox = m_font.GetTextExtent( form, a2dDEFAULT_ALIGNMENT );
                    linebbox.Translate( 0.0, height );
                    bbox.Expand( linebbox );
                    height =  height - ( GetLineHeight() + m_linespace );
                }
                ++iter;
            }
        }
    }
    return bbox;
}

bool a2dCurveGroupLegend::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( m_curveGroup && !m_bbox.GetValid() )
    {
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

#if wxART2D_USE_CVGIO

void a2dCurveGroupLegend::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "curvegroupname" ) , m_curveGroup->GetName() );
    }
    else
    {
    }
}

void a2dCurveGroupLegend::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }

    //m_curve = ;
}
#endif //wxART2D_USE_CVGIO

void a2dCurveGroupLegend::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    if( !m_format.IsEmpty() )
    {

        ic.GetDrawer2D()->SetFont( m_font );
        ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );

        double height = 0;

        const a2dCurvesAreaList& areaList = m_curveGroup->GetCurvesAreaList();
        for( size_t i = 0; i < areaList.GetCount(); i++ )
        {
            const a2dCurvesArea* area = areaList.Item( i );
            a2dCanvasObjectList::const_iterator iter = area->GetChildObjectList()->begin();
            while( iter != area->GetChildObjectList()->end() )
            {
                const a2dCurveObject* item = wxDynamicCast( ( *iter ).Get(), a2dCurve );
                if ( item )
                {
                    wxString form;
                    form.Printf( m_format, item->GetName().c_str() );

                    ic.GetDrawer2D()->DrawText( form, 0, height, a2dDEFAULT_ALIGNMENT );
                    height =  height - ( GetLineHeight() + m_linespace );
                }
                ++iter;
            }
        }
        ic.GetDrawer2D()->SetFont( *a2dNullFONT );
    }
}

bool a2dCurveGroupLegend::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    a2dPoint2D P = a2dPoint2D( hitEvent.m_relx, hitEvent.m_rely );
    //double pw = ic.GetWorldStrokeExtend();
    //double margin = ic.GetTransformedHitMargin();

    hitEvent.m_how.m_hit = a2dHit::hit_fill;
    return true;
}

