/*! \file apps/mars/part.cpp
    \author Erik van der Wal

    Copyright: 2005 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: part.cpp,v 1.19 2008/08/19 23:17:05 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/edit.h"

#include "part.h"
#include "elements.h" // forPin

//----------------------------------------------------------------------------
// msPart
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( msPart, msObject )

msPart::msPart()
    : msObject()
{
    m_fixed = NULL;
    m_pinsandprops = NULL;
}

msPart::msPart( const msPart& other, CloneOptions options, a2dRefMap* refs )
    : msObject( other, options, refs )
{
    //! \todo
    wxASSERT( 0 );
}

a2dObject* msPart::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    //! \todo
    wxASSERT( 0 );
    return new msPart( *this, options, refs );
};

msPart::msPart( msDocument* doc )
{
    wxFileName filename = doc->GetFilename();
    SetName( filename.GetName() );

    //! \todo EW: some less wastefull copying.
    m_fixed = new msObject( *( const msObject* ) doc->GetSymbolPage(), clone_deep, NULL );
    Append( m_fixed );
    m_pinsandprops = new msObject( *( const msObject* ) doc->GetSymbolPage(), clone_deep, NULL );
    Append( m_pinsandprops );

    // Delete pin and properties from fixed
    a2dCanvasObjectList::iterator iter = m_fixed->GetChildObjectList()->begin();
    while( iter != m_fixed->GetChildObjectList()->end() )
    {
        a2dCanvasObject* data = ( *iter );
        iter++;
        a2dText* text = wxDynamicCast( data, a2dText );
        if ( text && text->GetText().GetChar( 0 ) == wxChar( '@' ) )
            m_fixed->ReleaseChild( text );
    }

    // Delete all but pin and properties from pinsandprops
    iter = m_pinsandprops->GetChildObjectList()->begin();
    while (  iter != m_pinsandprops->GetChildObjectList()->end() )
    {
        a2dCanvasObject* data = ( *iter );
        iter++;

        a2dText* text = wxDynamicCast( data, a2dText );
        if ( text && text->GetText().GetChar( 0 ) == wxChar( '@' ) )
        {
            wxString name = text->GetText().Mid( 1 );
            int idx = name.Find( wxChar( '=' ) );
            if ( idx != -1 )
                name = name.Left( idx );
            name = name.Trim( true ).Trim( false );

            if ( !m_pinsandprops->GetStringProperty( name ) )
                m_pinsandprops->SetStringProperty( name, wxEmptyString );

            a2dPropertyId* id = m_pinsandprops->HasPropertyId( name );
            wxASSERT( id );

            a2dVisibleProperty* pppvis;
            pppvis = new a2dVisibleProperty( m_pinsandprops, id, 0.0, 0.0, 0.0 );

            //! \todo EW: more general copying of attibutes, DoExchangeMembers?
            pppvis->SetTransformMatrix( text->GetTransformMatrix() );
            pppvis->SetAlignment( text->GetAlignment() );
            pppvis->SetFont( text->GetFont() );
            pppvis->SetFill( text->GetFill() );
            pppvis->ShowName( idx != -1 );
            pppvis->SetEngineeringText( text->GetEngineeringText() );
            m_pinsandprops->Prepend( pppvis );
        }
        msConnectorPin* pin = wxDynamicCast( data, msConnectorPin );
        if ( pin )
        {
            Pin* p = new Pin( m_pinsandprops, pin->GetNumber(), Pin::ElementObject, pin->GetPosX(), pin->GetPosY() );
            m_pinsandprops->Prepend( p );
        }
        m_pinsandprops->ReleaseChild( data );
    }
}

//----------------------------------------------------------------------------
// msComponent
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( msComponent, msObject )

BEGIN_EVENT_TABLE( msComponent, msObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( msComponent::OnCanvasObjectMouseEvent )
END_EVENT_TABLE()

msComponent::msComponent()
    : msObject()
{
}

msComponent::msComponent( const msComponent& other, CloneOptions options, a2dRefMap* refs )
    : msObject( other, options, refs )
{
}

a2dObject* msComponent::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new msComponent( *this, options, refs );
};

msComponent::msComponent( msPart* part )
    : msObject(  *( const msObject* ) part->m_pinsandprops, clone_deep, NULL )
{
    wxASSERT( part );
    Append( part->m_fixed );
    MergeProperties( part->m_pinsandprops );
}

void msComponent::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
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
            msComponent* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), msComponent );;

            a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
            if ( IsHitWorld( *ic, hitevent ) )
            {
                a2dHitEvent hitinfo( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
                hitinfo.m_xyRelToChildren = true;
                a2dCanvasObject* text = original->IsHitWorld( *ic, hitinfo );
                if ( wxDynamicCast( text, a2dText )  && text->GetEditable() )
                {
                    a2dIterCU cu( *ic, original );
                    a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr ) ;

                    ic->SetCorridorPath( true );
                    controller->StartEditingObject( text, *ic );

                    SetPending( true );
                    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
                    event.Skip(); //first event for new tool ( meaning text that is edited )
                }
                else
                    event.Skip();
            }
            else
                EndEdit();
        }
        else if ( event.GetMouseEvent().LeftDClick()  )
        {
            EndEdit();
        }
        else if ( event.GetMouseEvent().Moving() )
        {
            event.Skip();
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

//----------------------------------------------------------------------------
// msConnectorPin
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( msConnectorPin, msObject )

BEGIN_EVENT_TABLE( msConnectorPin, msObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( msComponent::OnCanvasObjectMouseEvent )
END_EVENT_TABLE()

void msConnectorPin::Create( const wxString& number, const wxString& label )
{
    Append( new a2dSLine( 0.0, 0.0, 4.0, 0.0 ) );

    m_label = new a2dText( label, 5.0, 0.0, a2dFont( 3.0, wxFONTFAMILY_SWISS ) );
    m_label->SetAlignment( wxMINX );
    m_label->SetFill( *a2dTRANSPARENT_FILL );
    m_label->SetName( wxT( "label" ) );
    m_label->SetEngineeringText();
    Append( m_label );

    m_number = new a2dText( number, 3.0, 1.0, a2dFont( 2.0, wxFONTFAMILY_SWISS ) );
    m_number->SetAlignment( wxMINX | wxMINY );
    m_number->SetFill( *a2dTRANSPARENT_FILL );
    m_number->SetName( wxT( "number" ) );
    m_number->SetEngineeringText();
    Append( m_number );
    SetStroke( *wxRED, 0.5 );
}

msConnectorPin::msConnectorPin()
    : msObject()
{
    m_number = NULL;
    m_label = NULL;
}

msConnectorPin::msConnectorPin( double x, double y )
    : msObject()
{
    Create( wxT( "1" ), wxT( "1" ) );
    SetPosXY( x, y );
}

msConnectorPin::msConnectorPin( const wxString& number, double x, double y )
    : msObject()
{
    Create( number, number );
    SetPosXY( x, y );
}

msConnectorPin::msConnectorPin( const wxString& number, const wxString& label, double x, double y )
    : msObject()
{
    Create( number, label );
    SetPosXY( x, y );
}

msConnectorPin::msConnectorPin( const msConnectorPin& other, CloneOptions options, a2dRefMap* refs )
    : msObject( other, options, refs )
{
    m_label = wxDynamicCast( Find( wxT( "label" ) ), a2dText );
    wxASSERT( m_label );
    m_number = wxDynamicCast( Find( wxT( "number" ) ), a2dText );
    wxASSERT( m_number );
}

a2dObject* msConnectorPin::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new msConnectorPin( *this, options, refs );
};

void msConnectorPin::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
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
            msComponent* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), msComponent );;

            a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
            if ( IsHitWorld( *ic, hitevent ) )
            {
                a2dHitEvent hitinfo( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
                hitinfo.m_xyRelToChildren = true;
                a2dCanvasObject* text = original->IsHitWorld( *ic, hitinfo );
                if ( wxDynamicCast( text, a2dText )  && text->GetEditable() )
                {
                    a2dIterCU cu( *ic, original );
                    a2dStToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dStToolContr ) ;

                    ic->SetCorridorPath( true );
                    controller->StartEditingObject( text, *ic );

                    SetPending( true );
                    ic->GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS ) );
                    event.Skip(); //first event for new tool ( meaning text that is edited )
                }
                else
                    event.Skip();
            }
            else
                EndEdit();
        }
        else if ( event.GetMouseEvent().LeftDClick()  )
        {
            EndEdit();
        }
        else if ( event.GetMouseEvent().Moving() )
        {
            event.Skip();
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

void msConnectorPin::SetLabel( const wxString& label )
{
    if ( !m_label )
    {
        m_label = wxDynamicCast( Find( wxT( "label" ) ), a2dText );
        wxASSERT( m_label );
    }

    m_label->SetText( label );
}

void msConnectorPin::SetNumber( const wxString& number )
{
    if ( !m_number )
    {
        m_number = wxDynamicCast( Find( wxT( "number" ) ), a2dText );
        wxASSERT( m_number );
    }

    m_number->SetText( number );
}

wxString msConnectorPin::GetNumber()
{
    if ( !m_number )
    {
        m_number = wxDynamicCast( Find( wxT( "number" ) ), a2dText );
        wxASSERT( m_number );
    }

    return m_number->GetText();
}

a2dBoundingBox msConnectorPin::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox = msObject::DoGetUnTransformedBbox( flags );
    bbox.Expand( 0.0, 1.0 );
    bbox.Expand( 1.0, 0.0 );
    bbox.Expand( 0.0, -1.0 );
    bbox.Expand( -1.0, 0.0 );
    return bbox;
}

void msConnectorPin::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( wxDynamicCast( ic.GetParent(), msSymbolPage ) )
    {
        ic.GetDrawer2D()->DrawLine( 0.0, 1.0, 1.0, 0.0 );
        ic.GetDrawer2D()->DrawLine( 1.0, 0.0, 0.0, -1.0 );
        ic.GetDrawer2D()->DrawLine( 0.0, -1.0, -1.0, 0.0 );
        ic.GetDrawer2D()->DrawLine( -1.0, 0.0, 0.0, 1.0 );
    }
}
