/*! \file canvas/src/canpin.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canprim.cpp,v 1.351 2009/07/24 16:35:01 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/wfstream.h>
#include <wx/module.h>
#include <wx/clipbrd.h>

#include "wx/canvas/canobj.h"
#include "wx/canvas/canpin.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/wire.h"
#include "wx/canvas/cameleon.h"

#include <algorithm>

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

//----------------------------------------------------------------------------
// a2dHandle
//----------------------------------------------------------------------------

a2dPropertyIdInt32* a2dHandle::PROPID_Width = NULL;
a2dPropertyIdInt32* a2dHandle::PROPID_Height = NULL;
a2dPropertyIdInt32* a2dHandle::PROPID_Radius = NULL;

INITIALIZE_PROPERTIES( a2dHandle, a2dCanvasObject )
{
/*
    PROPID_Width = new a2dPropertyIdInt32( wxT( "Width" ),
                                           a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdInt32::Mptr >( &a2dHandle::m_width ) );
    AddPropertyId( PROPID_Width );
    PROPID_Height = new a2dPropertyIdInt32( wxT( "Height" ),
                                            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdInt32::Mptr >( &a2dHandle::m_height ) );
    AddPropertyId( PROPID_Height );
    PROPID_Radius = new a2dPropertyIdInt32( wxT( "Radius" ),
                                            a2dPropertyId::flag_none, 0, static_cast < a2dPropertyIdInt32::Mptr >( &a2dHandle::m_radius ) );
    AddPropertyId( PROPID_Radius );
*/
    return true;
}

IMPLEMENT_DYNAMIC_CLASS( a2dHandle, a2dCanvasObject )

const long a2dHandle::sm_HandleNoHit = wxGenNewId();
const long a2dHandle::sm_HandleHit = wxGenNewId();
bool a2dHandle::m_worldBased = false;

BEGIN_EVENT_TABLE( a2dHandle, a2dCanvasObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dHandle::OnCanvasObjectMouseEvent )
    EVT_CANVASOBJECT_ENTER_EVENT( a2dHandle::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT( a2dHandle::OnLeaveObject )
END_EVENT_TABLE()

a2dHandle::a2dHandle()
    : a2dCanvasObject()
{
    m_parent = NULL;
    m_flags.m_prerenderaschild = false;
    m_width = GetHabitat()->GetHandleSize();
    m_height = GetHabitat()->GetHandleSize();
    m_radius  = GetHabitat()->GetHandle()->GetRadius();
    m_mode = sm_HandleNoHit;
}

a2dHandle::a2dHandle( a2dCanvasObject* parent, double xc, double yc, const wxString& name,
                      double w, double h, double angle , double radius )
    : a2dCanvasObject()
{
    m_flags.m_editingCopy = true;

    m_name = name;
    m_parent = parent;
    m_flags.m_prerenderaschild = false;

    Rotate( angle );
    m_lworld.Translate( xc, yc );

    if ( w )
        m_width = w;
    else if ( parent && parent->GetRoot() )
        m_width = parent->GetHabitat()->GetHandleSize();
    else
        m_width = a2dCanvasGlobals->GetHabitat()->GetHandleSize();
    if ( h )
        m_height = h;
    else if ( parent && parent->GetRoot() )
        m_height = parent->GetHabitat()->GetHandleSize();
    else
        m_height = a2dCanvasGlobals->GetHabitat()->GetHandleSize();

    m_radius  = radius;
    m_mode = sm_HandleNoHit;

    SetPending( true );
}

void a2dHandle::Set( double xc, double yc, double w, double h, double angle, double radius )
{
    m_flags.m_prerenderaschild = false;

    m_lworld.Identity();
    Rotate( angle );
    m_lworld.Translate( xc, yc );

    m_width = w;
    m_height = h;
    m_radius  = radius;

    SetPending( true );
}

void a2dHandle::Set2( double xc, double yc, const wxString& name )
{
    m_flags.m_prerenderaschild = false;

    m_lworld.Identity();
    m_lworld.Translate( xc, yc );

    if ( !name.IsEmpty() )
        m_name = name;

    SetPending( true );
}

a2dHandle::~a2dHandle()
{
}

a2dHandle::a2dHandle( const a2dHandle& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_width = other.m_width;
    m_height = other.m_height;
    m_radius = other.m_radius;
    m_mode = other.m_mode;
    m_parent = other.m_parent;
    m_name = other.m_name;

}

void a2dHandle::SetParent( a2dCanvasObject* parent )
{
    m_parent = parent;
}

a2dObject* a2dHandle::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dHandle( *this, options, refs );
};

a2dBoundingBox a2dHandle::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    bbox.Enlarge( 0 );
    return bbox;
}

bool a2dHandle::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        if ( m_worldBased )
            m_worldExtend = wxMax( m_worldExtend, wxMax( m_width / 2.0, m_height / 2.0 ) );
        else
            m_pixelExtend = ( wxUint16 ) wxMax( m_pixelExtend, ( int ) wxMax( m_width / 2.0, m_height / 2.0 ) );
        //assert( m_pixelExtend < 40 );
        return true;
    }
    return false;
}

void a2dHandle::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    ic.GetDrawer2D()->OverRuleFixedStyle();
    if ( !m_flags.m_selected )
    {
        ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetHandle()->GetFill() );
        ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetHandle()->GetStroke() );
    }
    else
    {
        ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetSelectFill() );
        ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetSelectStroke() );
    }

    double x1 = 0;
    double y1 = 0;
    double dx = 0;
    double dy = 0;
    double dw = m_width;
    double dh = m_height;
    double dr = m_radius;
    ic.GetTransform().TransformPoint( 0, 0, x1, y1 ); // the object position in absolute coordinates.
    dx = ic.GetDrawer2D()->WorldToDeviceX( x1 );
    dy = ic.GetDrawer2D()->WorldToDeviceY( y1 );

    if ( m_worldBased )
    {
        dw = fabs( ic.GetDrawer2D()->WorldToDeviceXRelNoRnd( dw ) );// + ic.GetWorldStrokeExtend();
        dh = fabs( ic.GetDrawer2D()->WorldToDeviceYRelNoRnd( dh ) );// + ic.GetWorldStrokeExtend();
        dr = fabs( ic.GetDrawer2D()->WorldToDeviceYRelNoRnd( dr ) );
    }
    else
    {
        dw = dw + ic.GetWorldStrokeExtend();
        dh = dh + ic.GetWorldStrokeExtend();
    }

    ic.GetDrawer2D()->PushIdentityTransform();

    if (  m_mode == sm_HandleHit )
    {
        ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
        ic.GetDrawer2D()->DrawLine( dx - dw / 2.0, dy,  dx + dh / 2.0, dy );
        ic.GetDrawer2D()->DrawLine( dx, dy + dh / 2.0,  dx, dy - dh / 2.0 );
    }
    else if (  m_mode == sm_HandleNoHit )
    {
        ic.GetDrawer2D()->DrawCenterRoundedRectangle( dx, dy, dw, dh, dr );
    }
    else
    {
        //sofar un defined mode.
#ifdef __WXMSW__
        ic.GetDrawer2D()->DrawCircle( dx, dy, _hypot( dw, dh ) );
#else
        ic.GetDrawer2D()->DrawCircle( dx, dy, hypot( dw, dh ) );
#endif
    }
    ic.GetDrawer2D()->PopTransform();

    ic.GetDrawer2D()->ReStoreFixedStyle();
}

bool a2dHandle::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    //wxLogDebug(wxT("x=%12.6lf, y=%12.6lf"),x , y );
    double xh, yh;
    ic.GetTransform().TransformPoint( 0, 0, xh, yh );

    // Hit Tests are sometimes made for algorithmic purposes
    // Then it might be hard to supply a drawer
    // In this cases, the handle will appear to have zero size
    // A better solution would be to have a drawer independent Device/World conversion scheme in the ic
    double w = m_width;
    double h = m_height;

    if ( m_worldBased )
    {
        w = w + ic.GetWorldStrokeExtend();
        h = h + ic.GetWorldStrokeExtend();
    }
    else
    {
        w = fabs( ic.GetDrawer2D()->DeviceToWorldXRel( w ) ) + ic.GetWorldStrokeExtend();
        h = fabs( ic.GetDrawer2D()->DeviceToWorldYRel( h ) ) + ic.GetWorldStrokeExtend();
    }

    hitEvent.m_how = HitTestRectangle( hitEvent.m_x, hitEvent.m_y, xh - w / 2.0, yh - h / 2.0, xh + w / 2.0, yh + h / 2.0, 0 );
    if ( hitEvent.m_how.IsHit() )
        return hitEvent.m_how.IsHit();

    return hitEvent.m_how.IsHit();
}

#if wxART2D_USE_CVGIO
void a2dHandle::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "width" ), m_width );
        out.WriteAttribute( wxT( "height" ), m_height );
        if ( m_radius != 0.0 )
        {
            out.WriteAttribute( wxT( "radius" ), m_radius );
        }
    }
    else
    {
    }
}
void a2dHandle::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_width = parser.RequireAttributeValueInt( wxT( "width" ) );
        m_height = parser.RequireAttributeValueInt( wxT( "height" ) );
        m_radius = parser.GetAttributeValueInt( wxT( "radius" ) );
        m_name = parser.GetAttributeValue( wxT( "name" ), m_name );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dHandle::IsTemporary_DontSave() const
{
    // handles are always temporary
    return true;
}

void a2dHandle::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    if ( m_parent && !m_parent->GetEditingRender( ) )
    {
        event.Skip();
        return;
    }
/*
    if ( ! (m_parent->GetBin2() && m_parent->GetBin() ) )
    {
        event.Skip();
        return;
    }
*/
    a2dIterC* ic = event.GetIterC();

    static double xprev;
    static double yprev;
    static bool pushed = false;

    double xw, yw;
    xw = event.GetX();
    yw = event.GetY();

    a2dRestrictionEngine* restrictEngine = GetHabitat()->GetRestrictionEngine();
    /*
        if (restrictEngine )
        {
            restrictEngine->SetShiftKeyDown( event.GetMouseEvent().ShiftDown() );
            restrictEngine->SetAltKeyDown( event.GetMouseEvent().AltDown() );
        }
    */
    // this is the absolute matrix relative to the parent object, so with out this handle its
    // m_lworld included.
    // a2dAffineMatrix atWorld = ic->GetTransform();

    if ( event.GetMouseEvent().LeftDown() )
    {
        ic->SetCorridorPathToObject( this );
        xprev = xw;
        yprev = yw;
        SetMode( sm_HandleHit );
        pushed = true;
        ic->GetDrawingPart()->PushCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditWireVertex ) );
        a2dHandleMouseEvent handleEvent( ic, this, xw, yw, event.GetMouseEvent() );
        if ( m_parent )
            m_parent->ProcessEvent( handleEvent );
    }
    else if ( event.GetMouseEvent().Dragging() )
    {
        double x, y;
        x = xw - ( xprev - GetPosX() );
        y = yw - ( yprev - GetPosY() );

        xprev = xw;
        yprev = yw;
        a2dHandleMouseEvent handleEvent( ic, this, xw, yw, event.GetMouseEvent() );
        if ( m_parent )
            m_parent->ProcessEvent( handleEvent );
    }
    else if ( event.GetMouseEvent().Moving() )
    {
        a2dHandleMouseEvent handleEvent( ic, this, xw, yw, event.GetMouseEvent() );
        if ( m_parent )
            m_parent->ProcessEvent( handleEvent );
    }
    else if ( event.GetMouseEvent().LeftUp() && ic->GetDrawingPart()->GetEndCorridorObject() == this )
    {
        SetMode( sm_HandleNoHit );
        ic->SetCorridorPathToParent();
        if ( pushed )
            ic->GetDrawingPart()->PopCursor();
        pushed = false;
        a2dHandleMouseEvent handleEvent( ic, this, xw, yw, event.GetMouseEvent() );
        if ( m_parent )
            m_parent->ProcessEvent( handleEvent );
    }
    else
        event.Skip();
}

//#define _DEBUG_REPORTHIT
void a2dHandle::OnEnterObject( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();
    m_mode = sm_HandleHit;
    SetPending( true );
#ifdef _DEBUG_REPORTHIT
    wxLogDebug( wxT( "hit = %s %p" ), wxT( "enter handle" ), this );
#endif

    double xw, yw;
    xw = event.GetX();
    yw = event.GetY();
    a2dHandleMouseEvent handleEvent( ic, this, xw, yw, event.GetMouseEvent(), wxEVT_CANVASHANDLE_MOUSE_EVENT_ENTER );
    if ( m_parent )
        m_parent->ProcessEvent( handleEvent );

    //ic->GetDrawingPart()->PushCursor( GetHabitat()->GetCursor( a2dCURSOR_HandleEnter ) );
}

void a2dHandle::OnLeaveObject( a2dCanvasObjectMouseEvent& event )
{
    a2dIterC* ic = event.GetIterC();
    m_mode = sm_HandleNoHit;
    SetPending( true );
#ifdef _DEBUG_REPORTHIT
    wxLogDebug( wxT( "hit = %s %p" ), wxT( "leave handle" ), this );
#endif

    double xw, yw;
    xw = event.GetX();
    yw = event.GetY();
    a2dHandleMouseEvent handleEvent( ic, this, xw, yw, event.GetMouseEvent(), wxEVT_CANVASHANDLE_MOUSE_EVENT_LEAVE );
    if ( m_parent )
        m_parent->ProcessEvent( handleEvent );

    //ic->GetDrawingPart()->PopCursor();
}

//----------------------------------------------------------------------------
// a2dPinClass
//----------------------------------------------------------------------------

a2dPinClass* a2dPinClass::Any = NULL;

a2dPinClass* a2dPinClass::Standard = NULL;

std::list< a2dPinClass* > a2dPinClass::m_allPinClasses;

a2dPinClass::a2dPinClass( const wxString& name )
:m_flags(0)
{
    m_name = name;
    m_connectionGenerator = NULL;
}

a2dPinClass::~a2dPinClass()
{
}

void a2dPinClass::RemoveConnect( a2dPinClass* pinClass )
{
    std::list< a2dPinClass* >::iterator iter = m_canConnectTo.begin();
    while( iter != m_canConnectTo.end() )
    {
        if ( ( *iter ) == pinClass )
            iter = m_canConnectTo.erase( iter );
        else
            iter++;
    }
}

void a2dPinClass::InitializeStockPinClasses()
{
    Standard = new a2dPinClass( wxT( "standard" ) );
    Standard->AddConnect( a2dPinClass::Standard );
    Standard->SetAngleLine( false );
    m_allPinClasses.push_back( Standard );

    Any = new a2dPinClass( wxT( "anyPinClass" ) );
    Any->AddConnect( a2dPinClass::Standard );
    m_allPinClasses.push_back( Any );
}

void a2dPinClass::DeleteStockPinClasses()
{
    std::list< a2dPinClass* >::iterator iter;

    for( iter = m_allPinClasses.begin(); iter != m_allPinClasses.end(); ++iter )
        delete ( *iter );
    m_allPinClasses.clear();
}

a2dPinClass* a2dPinClass::CanConnectTo( a2dPinClass* other ) const
{
    std::list< a2dPinClass* >::const_iterator iter;

    for( iter = m_canConnectTo.begin(); iter != m_canConnectTo.end(); ++iter )
    {
        if ( ( other == NULL || ( *iter ) == other ) )
            return ( *iter );
    }
    return NULL;
}

a2dPinClass* a2dPinClass::GetClassByName( const wxString& name )
{
    std::list< a2dPinClass* >::iterator iter;

    for( iter = m_allPinClasses.begin(); iter != m_allPinClasses.end(); ++iter )
    {
        if ( ( *iter )->m_name == name )
            return ( *iter );
    }
    return NULL;
}

void a2dPinClass::SetPin( a2dPin* newpin )
{
    m_defPin = newpin;
}

a2dPin* a2dPinClass::GetPin()
{
    if ( !m_defPin )
    {
        m_defPin = new a2dPin( NULL, wxT( "templatePin" ), this, 0, 0, 0, a2dCanvasGlobals->GetHabitat()->GetPinSize(), a2dCanvasGlobals->GetHabitat()->GetPinSize() );
        m_defPin->SetFill( *wxGREEN );
        m_defPin->SetStroke( wxColour( 66, 159, 235 ), 0 );
    }
    return m_defPin;
}

void a2dPinClass::SetPinCannotConnect( a2dPin* newpin )
{
    m_defCannotConnectPin = newpin;
}

a2dPin* a2dPinClass::GetPinCannotConnect()
{
    if ( !m_defCannotConnectPin )
    {
        m_defCannotConnectPin = new a2dPin( NULL, wxT( "templatePin" ), this, 0, 0, 0, a2dCanvasGlobals->GetHabitat()->GetPinSize(), a2dCanvasGlobals->GetHabitat()->GetPinSize() );
        m_defCannotConnectPin->SetFill( wxColour( 255, 128, 128 ) );
        m_defCannotConnectPin->SetStroke( *wxRED, 0 );
    }
    return m_defCannotConnectPin;
}

void a2dPinClass::SetPinCanConnect( a2dPin* newpin )
{
    m_defCanConnectPin = newpin;
}

a2dPin* a2dPinClass::GetPinCanConnect()
{
    if ( !m_defCanConnectPin )
    {
        m_defCanConnectPin = new a2dPin( NULL, wxT( "templatePin" ), this, 0, 0, 0, a2dCanvasGlobals->GetHabitat()->GetPinSize(), a2dCanvasGlobals->GetHabitat()->GetPinSize() );
        m_defCanConnectPin->SetFill( wxColour( 128, 255, 128 ) );
        m_defCanConnectPin->SetStroke( *wxGREEN, 0 );
    }
    return m_defCanConnectPin;
}

void a2dPinClass::SetParPin( a2dParPin* newpin )
{
    m_defParPin = newpin;
}

a2dParPin* a2dPinClass::GetParPin()
{
    if ( !m_defParPin )
    {
        m_defParPin = new a2dParPin( this, a2dCanvasGlobals->GetHabitat()->GetPinSize(), a2dCanvasGlobals->GetHabitat()->GetPinSize() );
    }
    return m_defParPin;    
}

//----------------------------------------------------------------------------
// a2dPin
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dPin, a2dCanvasObject )

bool a2dPin::m_worldBased = false;
bool a2dPin::m_doRender = true;

BEGIN_EVENT_TABLE( a2dPin, a2dCanvasObject )
    EVT_CANVASOBJECT_ENTER_EVENT( a2dPin::OnEnterObject )
    EVT_CANVASOBJECT_LEAVE_EVENT( a2dPin::OnLeaveObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dPin::OnCanvasObjectMouseEvent )
END_EVENT_TABLE()

const long a2dPin::sm_PinUnConnected = wxGenNewId();
const long a2dPin::sm_PinConnected = wxGenNewId();
const long a2dPin::sm_PinCanConnect = wxGenNewId();
const long a2dPin::sm_PinCannotConnect = wxGenNewId();
const long a2dPin::sm_PinCanConnectToPinClass = wxGenNewId();

a2dPin::a2dPin()
    : a2dCanvasObject()
{
    m_parent = NULL;
    m_flags.m_prerenderaschild = false;
    m_width = a2dPinClass::Standard->GetPin()->GetWidth();
    m_height = a2dPinClass::Standard->GetPin()->GetHeight();
    //m_width = GetHabitat()->GetPinSize();
    //m_height = GetHabitat()->GetPinSize();
    m_radius  = GetHabitat()->GetPin()->GetRadius();

    m_mode = sm_PinUnConnected;
    m_pinclass = a2dPinClass::Standard;
    m_RenderConnected = false;
    m_dynamicPin = false;
    m_temporaryPin = false;
    m_internal = false;
    m_objectPin = true;
}

a2dPin::a2dPin( a2dCanvasObject* parent, const wxString& name, a2dPinClass* pinclass, double xc, double yc,
                double angle , double w, double h, double radius )
    : a2dCanvasObject()
{
    m_name = name;
    m_parent = parent;

    Rotate( angle );
    m_lworld.Translate( xc, yc );

    m_radius  = radius;

    m_width = w;
    m_height = h;
    if ( !w )
        m_width = pinclass->GetPin()->GetWidth();

    if ( !h )
        m_height = pinclass->GetPin()->GetHeight();

    m_mode = sm_PinUnConnected;
    m_name = name;
    m_pinclass = pinclass;
    m_flags.m_prerenderaschild = false;
    m_RenderConnected = false;
    m_dynamicPin = false;
    m_temporaryPin = false;
    m_internal = false;
    m_objectPin = true;

    if ( m_parent )
    {
        m_parent->SetPending( true );
        m_parent->SetSpecificFlags( true, a2dCanvasOFlags::HasPins );
    }
}


void a2dPin::Set( double xc, double yc, double angle, const wxString& name, bool dynamic )
{
    m_dynamicPin = dynamic;
    m_flags.m_prerenderaschild = false;
    m_lworld.Identity();
    Rotate( angle );
    m_lworld.Translate( xc, yc );

    SetPending( true );
    if ( !name.IsEmpty() )
        m_name = name;
}

a2dPin::~a2dPin()
{
    //disconnect from the connected pins (and object)
    a2dPinList::iterator iter;
    for ( iter = m_connectedPins.begin( ) ; iter != m_connectedPins.end( ) ; iter++ )
    {
        if ( (*iter ) )
            (*iter)->Disconnect( this );
    }
    m_connectedPins.clear();
}

a2dPin::a2dPinFlagsMask a2dPin::GetPinFlags() const
{
    a2dPinFlagsMask oflags = a2dPin::NON;

    if ( m_dynamicPin )   oflags = oflags | a2dPin::dynamic;
    if ( m_temporaryPin ) oflags = oflags | a2dPin::temporary;
    if ( m_internal )  oflags = oflags | a2dPin::internal;
    if ( m_objectPin )   oflags = oflags | a2dPin::objectPin;

    return oflags;
}

void a2dPin::SetPinFlags( a2dPinFlagsMask which, bool value )
{
    if ( value )
    {
        m_dynamicPin =  0 < (which & a2dPin::dynamic);
        m_temporaryPin =  0 < (which & a2dPin::temporary);
        m_internal =  0 < (which & a2dPin::internal);
        m_objectPin =  0 < (which & a2dPin::objectPin);
    }
    else
    {
        m_dynamicPin =  0 > (which & a2dPin::dynamic);
        m_temporaryPin =  0 > (which & a2dPin::temporary);
        m_internal =  0 > (which & a2dPin::internal);
        m_objectPin =  0 > (which & a2dPin::objectPin);
    }
}

void a2dPin::SetPinFlags( a2dPinFlagsMask which )
{
    m_dynamicPin =  0 < (which & a2dPin::dynamic);
    m_temporaryPin =  0 < (which & a2dPin::temporary);
    m_internal =  0 < (which & a2dPin::internal);
    m_objectPin =  0 < (which & a2dPin::objectPin);
}

void a2dPin::CleanUpNonConnected()
{
    a2dPinList::iterator iter = m_connectedPins.begin();
    while ( iter != m_connectedPins.end() )
    {
        if ( !(*iter ) )
            iter = m_connectedPins.erase( iter );
        else
            iter++;
    }
}

wxUint32 a2dPin::GetConnectedPinsNr() const
{
    wxUint32 connected = 0;
    a2dPinList::const_iterator iter = m_connectedPins.begin();
    while ( iter != m_connectedPins.end() )
    {
        if ( (*iter ) )
            connected++;
        iter++;
    }
    return connected;
}

a2dPin* a2dPin::IsConnectedTo( a2dPin* pin ) const
{
    if ( pin )
    {
        a2dPinList::const_iterator iter;
        iter = std::find( m_connectedPins.begin(), m_connectedPins.end(), pin );
        bool found = false;
        bool found2 = false;

        found = iter != m_connectedPins.end();
        if ( found )
        {
            a2dPinList::const_iterator iter2;
            for ( iter2 = pin->m_connectedPins.begin( ) ; iter2 != pin->m_connectedPins.end( ) ; iter2++ )
            {
                if ( *iter2 == this )
                    found2 = true;
            }
            wxASSERT_MSG( found2, _T( "connection not found in other pin" ) );
            return (*iter);
        }
        return NULL;
    }
    else if ( m_connectedPins.empty() )
        return NULL;
    else
    {   //return first connected pin, empty items not counts as connected.
        a2dPinList::const_iterator iter = m_connectedPins.begin();
        while ( iter != m_connectedPins.end() )
        {
            if ( (*iter ) )
                return (*iter);
            iter++;
        }
    }
    return NULL;
}

bool a2dPin::Disconnect( a2dPin* pin, bool forceErase )
{
    bool found = false;
    a2dPinList::iterator iter = m_connectedPins.begin( );
    while( iter != m_connectedPins.end( ) )
    {
        a2dPin* connectedPin = *iter;
        if ( connectedPin && (connectedPin == pin || !pin) )
        {
            found = true;
            bool found2 = false;
            a2dPinList::iterator iter2 = connectedPin->m_connectedPins.begin( );
            while( iter2 != connectedPin->m_connectedPins.end( ) )
            {
                if ( *iter2 == this )
                {
                    (*iter2) = NULL;
                    if ( forceErase )
                        iter2 = connectedPin->m_connectedPins.erase( iter2 );
                    else
                        iter2++;
                    found2 = true;
                }
                else
                    iter2++;
            }
            connectedPin->m_mode = sm_PinUnConnected;
            if ( connectedPin->IsConnectedTo() )
                connectedPin->m_mode = sm_PinConnected;

            wxASSERT_MSG( found2, _T( "connection not found in other pin" ) );
            (*iter) = NULL;
            if ( forceErase )
                iter = m_connectedPins.erase( iter );
            else
                iter++;
        }
        else
            iter++;
    }
    m_mode = sm_PinUnConnected;
    if ( IsConnectedTo() )
        m_mode = sm_PinConnected;
    return found;
}


void a2dPin::DuplicateConnectedToOtherPins( bool undo )
{
    a2dPinList::iterator iter = m_connectedPins.begin( );
    while( iter != m_connectedPins.end( ) )
    {
        a2dPin* connectedPin = *iter;
        if ( connectedPin && !connectedPin->GetRelease() )
        {
            bool found2 = false;
            a2dPinList::iterator iter2 = m_connectedPins.begin( );
            while( iter2 != m_connectedPins.end( ) )
            {
                a2dPin* connectedPinconnect = *iter2;

                if ( connectedPin != connectedPinconnect && !connectedPin->IsConnectedTo( connectedPinconnect )  )
                {
                    if ( m_root->GetCommandProcessor() && undo )
                        m_root->GetCommandProcessor()->Submit( new a2dCommand_ConnectPins( connectedPin, connectedPinconnect ), true );
                    else
                        connectedPin->ConnectTo( connectedPinconnect );
                }
                iter2++;
            }
        }
        iter++;
    }
}

void a2dPin::DuplicateConnectedPins( a2dPin* other, bool undo )
{
    a2dPinList::iterator iter = other->m_connectedPins.begin( );
    while( iter != other->m_connectedPins.end( ) )
    {
        a2dPin* connectedPin = *iter;
        if ( connectedPin && !connectedPin->GetRelease() )
        {
            if ( connectedPin != this )
            {
                bool found2 = false;
                a2dPinList::iterator iter2 = m_connectedPins.begin( );
                while( iter2 != m_connectedPins.end( ) )
                {
                    if ( *iter2 == connectedPin )
                        found2 = true;
                    iter2++;
                }
                if ( !found2 )
                {
                    if ( undo )
                        m_root->GetCommandProcessor()->Submit( new a2dCommand_ConnectPins( this, connectedPin ), true );
                    else
                    ConnectTo( connectedPin );
                }
            }
        }
        iter++;
    }
}

void a2dPin::RemoveDuplicateConnectedPins( a2dPin* other )
{
    a2dPinList::iterator iter = other->m_connectedPins.begin( );
    while( iter != other->m_connectedPins.end( ) )
    {
        a2dPin* connectedPin = *iter;
        if ( connectedPin && !connectedPin->GetRelease() && connectedPin != this )
        {
            bool found2 = false;
            a2dPinList::iterator iter2 = m_connectedPins.begin( );
            while( iter2 != m_connectedPins.end( ) )
            {
                if ( *iter2 == connectedPin )
                    Disconnect( connectedPin );
                iter2++;
            }
            iter++;
        }
        else
            iter++;
    }
}

void a2dPin::ConnectTo( a2dPin* connectto )
{
    wxASSERT_MSG( connectto, _T( "connect pin should not be NULL" ) );
	wxASSERT_MSG( connectto != this , _T( "connect pin should not be itself" ) );
	if ( connectto != this )
	{
		//push in front, as it will be the first found when disconnecting it.
		m_connectedPins.push_front( connectto );
		connectto->m_connectedPins.push_front( this );
		connectto->m_mode = sm_PinConnected;
		m_mode = sm_PinConnected;
	}
}

void a2dPin::SetParent( a2dCanvasObject* parent )
{
    m_parent = parent;
    if ( m_parent )
    {
        //No need m_parent->SetPending( true );
        m_parent->SetHasPins( true );
        m_mode = sm_PinUnConnected;
        if ( IsConnectedTo() )
            m_mode = sm_PinConnected;
    }
}

void a2dPin::SetPending( bool pending )
{
    a2dCanvasObject::SetPending( pending );
    //if ( m_parent && m_parent->IsConnect() )
    //    m_parent->SetPending( pending );
}

a2dPoint2D a2dPin::GetAbsXY() const
{
    a2dAffineMatrix tworld = m_parent->GetTransformMatrix();
    tworld *= m_lworld;
    return a2dPoint2D( tworld.GetValue( 2, 0 ), tworld.GetValue( 2, 1 ) );
}

double a2dPin::GetAbsX() const
{
    a2dAffineMatrix tworld = m_parent->GetTransformMatrix();
    tworld *= m_lworld;
    return tworld.GetValue( 2, 0 );
}

double a2dPin::GetAbsY() const
{
    a2dAffineMatrix tworld = m_parent->GetTransformMatrix();
    tworld *= m_lworld;
    return tworld.GetValue( 2, 1 );
}

void a2dPin::SetAbsXY( double x, double y )
{
    a2dAffineMatrix tworldInv = m_parent->GetTransformMatrix();
    tworldInv.Invert();
    double xt, yt;
    tworldInv.TransformPoint( x, y, xt, yt );
    SetPosXY( xt, yt );

    assert( fabs( x - GetAbsX() ) < 1e-3 );
    assert( fabs( y - GetAbsY() ) < 1e-3 );
}

void a2dPin::SetAbsXY( const a2dPoint2D& point )
{
    SetAbsXY( point.m_x, point.m_y );
}

double a2dPin::GetAbsAngle() const
{
    a2dAffineMatrix tworld = m_parent->GetTransformMatrix();
    tworld *= m_lworld;
    return tworld.GetRotation();
}

bool a2dPin::MayConnectTo( a2dPin* connectto )
{
    // Check if both sides agree if they can connect or not.
    // If one can connect to the other, but the other is not allowed to connect to the one,
    // this indicates conflicting pin classes.
    wxASSERT_MSG( ( GetPinClass()->CanConnectTo( connectto->GetPinClass() ) != NULL ) == ( connectto->GetPinClass()->CanConnectTo( GetPinClass() ) != NULL )
		, wxT( "pinclass incompatible on both sides" ) );

    //!todo is a seperate task needed?
    return GetPinClass()->GetPinClassForTask( a2d_GeneratePinsForPinClass, m_parent, connectto->GetPinClass(), connectto ) != NULL;

    // Otherwise check if the pin classes are compatible
    return GetPinClass()->CanConnectTo( connectto->GetPinClass() ) != NULL;
}

a2dPin* a2dPin::FindNonWirePin( a2dCanvasObjectFlagsMask mask )
{
    //return first connected non wire pin, empty items not count as connected.
    a2dPinList::const_iterator iter = m_connectedPins.begin();
    while ( iter != m_connectedPins.end() )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj && !obj->GetRelease() )
        {
            if ( !obj->GetParent()->IsConnect() && obj->GetParent()->CheckMask( mask )  )
                return *iter;
        }
        iter++;
    }
    return NULL;
}

a2dPin* a2dPin::FindWirePin( a2dCanvasObjectFlagsMask mask )
{
    //return first connected non wire pin, empty items not count as connected.
    a2dPinList::const_iterator iter = m_connectedPins.begin();
    while ( iter != m_connectedPins.end() )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj && !obj->GetRelease() )
        {
            if ( obj->GetParent()->IsConnect() && obj->GetParent()->CheckMask( mask )  )
                return *iter;
        }
        iter++;
    }
    return NULL;
}

a2dPin* a2dPin::FindConnectablePin( a2dCanvasObject* root, double margin, bool autocreate )
{
    //!todo One bad thing about this is, that the margin is given in world units

    // Check if the parent of this pin is a DIRECT child of the root given
    assert( root->Find( m_parent ) );

    // Go through the childs of the root and look for objects having pins
    a2dIterC ic;
    a2dIterCU icu( ic, root );

    for( a2dCanvasObjectList::iterator iter = root->GetChildObjectList()->begin(); iter != root->GetChildObjectList()->end(); ++iter )
    {
        a2dCanvasObject* obj = *iter;
        if( !obj->CheckMask( a2dCanvasOFlags::HasPins ) ) continue;
        if( obj == m_parent ) continue;

        // Check if obj has a pin that can connect to this pin
        a2dPin* rslt = obj->CanConnectWith( ic, this, margin, autocreate );

        if( rslt )
            return rslt;
    }
    return 0;
}

void a2dPin::AutoConnect( a2dCanvasObject* root, double margin )
{
    // Check if this is an original object, not an editcopy
    wxASSERT( !PROPID_Original->GetPropertyValue( this ) );

    // First check if this pin is dislocated or unconnected
    if( IsDislocated() || !IsConnectedTo() )
    {
        a2dPin* rslt = FindConnectablePin( root, margin, true );
        if( rslt )
        {
            // Connect them
            GetRoot()->GetCanvasCommandProcessor()->Submit( new a2dCommand_ConnectPins( this, rslt ), true );
        }
    }
}

a2dPin* a2dPin::IsDislocated() const
{
    double maxdx = GetHabitat()->GetCoordinateEpsilon();
    double maxdy = GetHabitat()->GetCoordinateEpsilon();
    a2dPin* found = NULL;

    a2dPoint2D thispos = GetAbsXY();
    a2dPinList::const_iterator iter;
    for ( iter = m_connectedPins.begin( ) ; iter != m_connectedPins.end( ) ; iter++ )
    {
        a2dPin* connectedPin = *iter;
        if ( !connectedPin || connectedPin->GetRelease() )
            continue;
        a2dPoint2D otherpos = connectedPin->GetAbsXY();
        double dx = fabs( thispos.m_x - otherpos.m_x );
        double dy = fabs( thispos.m_y - otherpos.m_y );
        if ( dx > maxdx || dy > maxdy )
        {
            found = connectedPin;
            maxdx = dx;
            maxdy = dy;
        }
    }
    return found;
}

bool a2dPin::IsSameLocation( a2dPin* other, double margin ) const
{
    if ( ! margin )
        margin = GetHabitat()->GetCoordinateEpsilon();

    a2dPoint2D thispos = GetAbsXY();
    a2dPoint2D otherpos = other->GetAbsXY();
    if ( 
        fabs( thispos.m_x - otherpos.m_x ) < margin &&
        fabs( thispos.m_y - otherpos.m_y ) < margin
        )
        return true;
    return false;
}

a2dPin::a2dPin( const a2dPin& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_width = other.m_width;
    m_height = other.m_height;
    m_radius = other.m_radius;
    m_parent = other.m_parent;
    m_name = other.m_name;

    // wxLogDebug("Cloning pin %p into %p\n", &other, this );
    m_pinclass = other.m_pinclass;
    // Don't copy the m_connectedPins. The pins are mutally connected, and we can not copy that.
    m_mode = sm_PinUnConnected;
    m_RenderConnected = false;
    m_dynamicPin = other.m_dynamicPin;
    m_temporaryPin = other.m_temporaryPin;
    m_internal = other.m_internal;
    m_objectPin = other.m_objectPin;

    if( refs )
    {
        wxASSERT_MSG( refs, wxT( "for reconnect a2dRefMap needed" ) );

		a2dPinList::const_iterator iter;
		for ( iter = other.m_connectedPins.begin( ) ; iter != other.m_connectedPins.end( ) ; iter++ )
		{
			a2dPin* connectedPin = *iter;
			if ( connectedPin )
			{
				wxLongLong id = connectedPin->GetId();
				wxASSERT( id != 0 );
				wxString resolveKey;
				resolveKey << id;
				refs->ResolveOrAddLink( this, resolveKey );
			}
		}
    }
}

a2dObject* a2dPin::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dPin( *this, options, refs );
};

a2dBoundingBox a2dPin::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    bbox.Enlarge( 0 );

    if( IsConnectedTo() )
    {
        // check if the pin is dislocated
        // This shouldn't happen, but show it, if it does happen
        if( IsDislocated() || m_RenderConnected )
        {
            bbox.Expand( 0, 0 );
            // bbox is in pixels, so use m_pixelExtend
            //bbox.Expand( -m_width, -m_height );
            //bbox.Expand( m_width, m_height );
        }
    }
    else
    {
        bbox.Expand( 0, 0 );
        // bbox is in pixels, so use m_pixelExtend
        //bbox.Expand( -m_width, -m_height );
        //bbox.Expand( m_width, m_height );
    }

    return bbox;
}

bool a2dPin::DoUpdate( UpdateMode WXUNUSED( mode ), const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        CleanUpNonConnected();
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        //rotation indicator included
        if(  m_pinclass && m_pinclass->HasAngleLine() )
        {
            if ( m_worldBased )
                m_worldExtend = wxMax( m_worldExtend, wxMax( m_width / 2.0, m_height / 2.0 ) );
            else
                m_pixelExtend = wxMax( m_pixelExtend, wxMax( m_width / 2.0, m_height / 2.0 ) );
        }
        else
        {
            if ( m_worldBased )
                m_worldExtend = wxMax( m_worldExtend, wxMax( m_width / 2.0, m_height / 2.0 ) );
            else
                m_pixelExtend = wxMax( m_pixelExtend, wxMax( m_width / 2.0, m_height / 2.0 ) );
        }
        //assert( m_pixelExtend < 40 );
        return true;
    }
    return false;
}

void a2dPin::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    // Call base, instead of a2dHandle, if skipped there, 

    // just to debug stuff, differentiate.
    if ( event.GetMouseEvent().LeftDown() )
    {
        a2dCanvasObject::OnCanvasObjectMouseEvent( event );
    }
    else if ( event.GetMouseEvent().Dragging() )
    {
        a2dCanvasObject::OnCanvasObjectMouseEvent( event );
    }
    else if ( event.GetMouseEvent().Moving() )
    {
        a2dCanvasObject::OnCanvasObjectMouseEvent( event );
    }
    else if ( event.GetMouseEvent().LeftUp() )
    {
        a2dCanvasObject::OnCanvasObjectMouseEvent( event );
    }
    else
    {
        a2dCanvasObject::OnCanvasObjectMouseEvent( event );
    }
}


void a2dPin::OnEnterObject( a2dCanvasObjectMouseEvent& WXUNUSED( event ) )
{
}

void a2dPin::OnLeaveObject( a2dCanvasObjectMouseEvent& WXUNUSED( event ) )
{
}

void a2dPin::DrawHighLighted( a2dIterC& ic )
{
    ic.GetDrawer2D()->SetDrawerStroke( GetHabitat()->GetHighLightStroke() );
    ic.GetDrawer2D()->SetDrawerFill( GetHabitat()->GetHighLightFill() );

    double x1;
    double y1;
    ic.GetTransform().TransformPoint( 0, 0, x1, y1 );
    int dx = ic.GetDrawer2D()->WorldToDeviceX( x1 );
    int dy = ic.GetDrawer2D()->WorldToDeviceY( y1 );

    ic.GetDrawer2D()->PushIdentityTransform();
    ic.GetDrawer2D()->DrawCenterRoundedRectangle( dx, dy, m_width * 1.1, m_height * 1.1, m_radius );
    ic.GetDrawer2D()->PopTransform();
}

void a2dPin::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    if ( !m_doRender )
        return;

    //if( ic.GetDrawStyle() == RenderWIREFRAME_SELECT || ic.GetDrawStyle() == RenderWIREFRAME_SELECT_INVERT ) 
    //    return;

    if( ic.GetDrawStyle() == RenderWIREFRAME_SELECT_INVERT ) 
        return;

    double x1;
    double y1;
    ic.GetTransform().TransformPoint( 0, 0, x1, y1 );
    int dx = ic.GetDrawer2D()->WorldToDeviceX( x1 );
    int dy = ic.GetDrawer2D()->WorldToDeviceY( y1 );

    // Calculate handle size.
    double w2 = m_width / 2.0 ;
    double h2 = m_height / 2.0;
    double dr = m_radius;

    if (  m_mode == sm_PinCanConnectToPinClass )
    {
        // This is the mode of a pin that is hit by the mouse (set in OnEnter)
        //ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( wxColour(12,34,78), 0.5) );
        //ic.GetDrawer2D()->SetDrawerFill( a2dFill( wxColour(244,2,2)) );
        // This is the mode of a pin that can be connected to a specific pinclass
        w2 = m_pinclass->GetPinCanConnect()->GetWidth();
        h2 = m_pinclass->GetPinCanConnect()->GetHeight();
        dr = m_pinclass->GetPinCanConnect()->GetRadius();
    }
    else if ( m_mode == sm_PinCanConnect )
    {
        // This is the mode of a pin that can be connected
        w2 = m_pinclass->GetPinCanConnect()->GetWidth();
        h2 = m_pinclass->GetPinCanConnect()->GetHeight();
        dr = m_pinclass->GetPinCanConnect()->GetRadius();
    }
    else if (  m_mode == sm_PinCannotConnect )
    {
        // This is the mode of a pin that cannot be connected
        w2 = m_pinclass->GetPinCanConnect()->GetWidth();
        h2 = m_pinclass->GetPinCanConnect()->GetHeight();
        dr = m_pinclass->GetPinCanConnect()->GetRadius();
    }
    

    if ( m_worldBased )
    {
        w2 = fabs( ic.GetDrawer2D()->WorldToDeviceXRelNoRnd( w2 ) );// + ic.GetWorldStrokeExtend();
        h2 = fabs( ic.GetDrawer2D()->WorldToDeviceYRelNoRnd( h2 ) );// + ic.GetWorldStrokeExtend();
        dr = fabs( ic.GetDrawer2D()->WorldToDeviceYRelNoRnd( dr ) );
    }
    else
    {
        w2 = w2 + ic.GetWorldStrokeExtend();
        h2 = h2 + ic.GetWorldStrokeExtend();
    }

    //wxLogDebug(wxT("OUT2 %p"), this );
    ic.GetDrawer2D()->PushIdentityTransform();

    if ( !IsConnectedTo() ) // not connected ?
    {
        if ( GetAlgoSkip() && m_parent->HasProperty( a2dCanvasObject::PROPID_ToolObject ) )
            ic.GetDrawer2D()->SetDrawStyle( a2dFILLED );

        if ( m_mode == sm_PinUnConnected )
        {
            // This is the usual mode of an unconnecting pin just sitting there
            ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( *wxBLUE, 0 )  );
            // end of line one pixel short with wxDC
            if ( ic.GetDrawer2D()->GetDrawerStroke().GetPixelStroke() &&
                    ic.GetDrawer2D()->GetDrawerStroke().GetWidth() == 0 )
            {
                ic.GetDrawer2D()->DrawLine( dx - w2, dy + h2, dx + w2 + 1, dy - h2 - 1 );
                ic.GetDrawer2D()->DrawLine( dx - w2, dy - h2, dx + w2 + 1, dy + h2 + 1 );
            }
            else
            {
                ic.GetDrawer2D()->DrawLine( dx - w2, dy + h2, dx + w2, dy - h2 );
                ic.GetDrawer2D()->DrawLine( dx - w2, dy - h2, dx + w2, dy + h2 );
            }
            if( m_pinclass && m_pinclass->HasAngleLine() )
            {
                int x2, y2;
                double ang;
                ang = ic.GetTransform().GetRotation();
                x2 = Round( dx + m_pinclass->GetPin()->GetWidth() * 3 * cos( wxDegToRad( ang ) ) );
                y2 = Round( dy - m_pinclass->GetPin()->GetWidth() * 3 * sin( wxDegToRad( ang ) ) );

                ic.GetDrawer2D()->DrawLine( dx, dy, x2, y2 );
            }
        }
        else if (  m_mode == sm_PinCanConnectToPinClass )
        {
            // This is the mode of a pin that is hit by the mouse (set in OnEnter)
            //ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( wxColour(12,34,78), 0.5) );
            //ic.GetDrawer2D()->SetDrawerFill( a2dFill( wxColour(244,2,2)) );
            // This is the mode of a pin that can be connected to a specific pinclass
            ic.GetDrawer2D()->SetDrawerFill( m_pinclass->GetPinCanConnect()->GetFill() );
            ic.GetDrawer2D()->SetDrawerStroke( m_pinclass->GetPinCanConnect()->GetStroke() );
            ic.GetDrawer2D()->DrawCenterRoundedRectangle( dx, dy, w2, h2, dr );
        }
        else if ( m_mode == sm_PinCanConnect )
        {
            // This is the mode of a pin that can be connected
            //ic.GetDrawer2D()->SetDrawerFill( a2dFill( wxColour(12,134,228)) );
            //ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( *wxBLUE, 1) );
            ic.GetDrawer2D()->SetDrawerFill( m_pinclass->GetPinCanConnect()->GetFill() );
            ic.GetDrawer2D()->SetDrawerStroke( m_pinclass->GetPinCanConnect()->GetStroke() );
            ic.GetDrawer2D()->DrawCenterRoundedRectangle( dx, dy, w2, h2, dr );
        }
        else if (  m_mode == sm_PinCannotConnect )
        {
            // This is the mode of a pin that cannot be connected
            ic.GetDrawer2D()->SetDrawerFill( m_pinclass->GetPinCannotConnect()->GetFill() );
            ic.GetDrawer2D()->SetDrawerStroke( m_pinclass->GetPinCannotConnect()->GetStroke() );
            //ic.GetDrawer2D()->SetDrawerFill( a2dFill( wxColour(245,134,178)) );
            //ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( wxColour(12,34,178), 1) );
            ic.GetDrawer2D()->DrawCenterRoundedRectangle( dx, dy, w2, h2, dr );
        }

        if ( GetAlgoSkip() && m_parent->HasProperty( a2dCanvasObject::PROPID_ToolObject ) )
            ic.GetDrawer2D()->ReStoreFixedStyle();
    }
    else
    {
        // check if the pin is dislocated
        // This shouldn't happen, but show it, if it does happen
        if( IsDislocated() || m_RenderConnected )
        {
            ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
            ic.GetDrawer2D()->DrawCircle( dx , dy , (w2 + h2) / 2.0);
        }
        else
        {
            // This is the usual mode of a connected pin
            ic.GetDrawer2D()->SetDrawerFill( m_pinclass->GetPin()->GetFill() );
            ic.GetDrawer2D()->SetDrawerStroke( m_pinclass->GetPin()->GetStroke() );
            ic.GetDrawer2D()->DrawCircle( dx, dy, w2 / 2.0 );
        }
    }
    ic.GetDrawer2D()->PopTransform();
}

/*
void a2dPin::RestoreConnectionsAfterCloning( a2dCanvasCommandProcessor* cp )
{
    if( m_clonebrother )
    {
        wxASSERT( m_clonebrother->m_clonebrother == this );

        // Ok the clone original was connected
        // Check if this connected object was also cloned (might be a partial clone)
        // A partial clone means that only part of a group of connected objects was
        // cloned. On the outer side there will be no clone brothers in the original
        // objects, since those were not cloned.

        //iterate on connected pins in the original
        a2dPinList::const_iterator iterconp;
        for ( iterconp = m_clonebrother->GetConnectedPins().begin( ) ; iterconp != m_clonebrother->GetConnectedPins().end( ) ; iterconp++ )
        {
            a2dPin* pincother = *iterconp;
            if ( !pincother || pincother->GetRelease() )
                continue;

            if( pincother->m_clonebrother && !pincother->m_clonebrother->IsConnectedTo( this ) )
            {
                // The clone original should be properly connected
                assert( pincother->IsConnectedTo( m_clonebrother ) );

                // Connect the clone of the connection of the original with this
                // and vice versa
                if( cp )
                {
                    cp->Submit( new a2dCommand_ConnectPins( this, pincother->m_clonebrother ), true );
                }
                else
                {
                    //the orginal objects are found via the this its clone brother,
                    // and the object connected to that orginal object did get a clonebrother too.
                    // This makes it possible to restore the same connection in the cloned group.
                    this->ConnectTo( pincother->m_clonebrother );
                }
            }
        }
    }
}

*/

#if wxART2D_USE_CVGIO
void a2dPin::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "width" ), m_width );
        out.WriteAttribute( wxT( "height" ), m_height );
        if ( m_radius != 0.0 )
        {
            out.WriteAttribute( wxT( "radius" ), m_radius );
        }
        out.WriteAttribute( wxT( "pinclass" ), m_pinclass->GetName() );
        if ( GetConnectedPinsNr() == 1 )
        {
            a2dPinList::const_iterator iter;
            for ( iter = m_connectedPins.begin( ) ; iter != m_connectedPins.end( ) ; iter++ )
            {
                a2dPin* connectedPin = *iter;
                if ( connectedPin )
                {
                    out.WriteAttribute( wxT( "resolveid" ), connectedPin->GetId() );
                    wxASSERT_MSG( !m_temporaryPin, wxT( "temporary pins should have bin deleted" ) );
                    break;
                }
            }
        }
        out.WriteAttribute( wxT( "RenderConnected" ), m_RenderConnected, true );
        out.WriteAttribute( wxT( "dynamicPin" ), m_dynamicPin, true );
        out.WriteAttribute( wxT( "internal" ), m_internal, true );
    }
    else
    {
        a2dPinList::const_iterator iter;
        if ( GetConnectedPinsNr() > 1 )
        {
            for ( iter = m_connectedPins.begin( ) ; iter != m_connectedPins.end( ) ; iter++ )
            {
                a2dPin* connectedPin = *iter;
                if ( connectedPin )
                {
                    out.WriteStartElementAttributes( wxT( "connect" ) );
                    out.WriteAttribute( wxT( "resolveid" ), connectedPin->GetId() );
                    wxASSERT_MSG( !m_temporaryPin, wxT( "temporary pins should have bin deleted" ) );       
                    out.WriteEndAttributes( true);
                }
            }
        }
    }
}

void a2dPin::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    m_parent = wxStaticCast( parent, a2dCanvasObject );
    m_parent->SetSpecificFlags( true, a2dCanvasOFlags::HasPins );

    a2dCanvasObject::DoLoad( parent, parser, xmlparts );

    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_width = parser.RequireAttributeValueInt( wxT( "width" ) );
        m_height = parser.RequireAttributeValueInt( wxT( "height" ) );
        m_radius = parser.GetAttributeValueInt( wxT( "radius" ) );
        m_name = parser.GetAttributeValue( wxT( "name" ), m_name );

        wxString pinclass = parser.RequireAttributeValue( wxT( "pinclass" ) );
        m_pinclass = a2dPinClass::GetClassByName( pinclass );
        if( !m_pinclass )
        {
            m_pinclass = a2dPinClass::Standard;
            wxString reference_name = GetName();
            a2dGeneralGlobals->ReportErrorF( a2dError_NoPinClass, _( "a2dPin with name %s \n Pin class %s not found (replaced with Standard)" ), reference_name.c_str(), pinclass.c_str() );
        }
        if ( parser.HasAttribute( wxT( "resolveid" ) ) )
        {
            wxString resolveid = parser.GetAttributeValue( wxT( "resolveid" ) );
            parser.ResolveOrAddLink( this, resolveid );
        }

        m_RenderConnected = parser.GetAttributeValueBool( wxT( "RenderConnected" ) );
        m_dynamicPin = parser.GetAttributeValueBool( wxT( "dynamicPin" ) );
        m_internal = parser.GetAttributeValueBool( wxT( "internal" ) );
    }
    else
    {
        while( parser.GetTagName() == wxT( "connect" ) )
		{
			parser.Require( START_TAG, wxT( "connect" ) );
            if ( parser.HasAttribute( wxT( "resolveid" ) ) )
            {
                wxString resolveid = parser.GetAttributeValue( wxT( "resolveid" ) );
                parser.ResolveOrAddLink( this, resolveid );
            }
			parser.Next();
			parser.Require( END_TAG, wxT( "connect" ) );
			parser.Next();
		}
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dPin::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    //wxLogDebug(wxT("x=%12.6lf, y=%12.6lf"),x , y );
    double xh, yh;
    ic.GetTransform().TransformPoint( 0, 0, xh, yh );

    // Hit Tests are sometimes made for algorithmic purposes
    // Then it might be hard to supply a drawer
    // In this cases, the handle will appear to have zero size
    // A better solution would be to have a drawer independent Device/World conversion scheme in the ic
    double w = m_width;
    double h = m_height;

    if ( m_worldBased )
    {
        w = w + ic.GetWorldStrokeExtend();
        h = h + ic.GetWorldStrokeExtend();
    }
    else
    {
        w = fabs( ic.GetDrawer2D()->DeviceToWorldXRel( w ) ) + ic.GetWorldStrokeExtend();
        h = fabs( ic.GetDrawer2D()->DeviceToWorldYRel( h ) ) + ic.GetWorldStrokeExtend();
    }

    hitEvent.m_how = HitTestRectangle( hitEvent.m_x, hitEvent.m_y, xh - w / 2.0, yh - h / 2.0, xh + w / 2.0, yh + h / 2.0, 0 );
    if ( hitEvent.m_how.IsHit() )
        return hitEvent.m_how.IsHit();

    return hitEvent.m_how.IsHit();
}

bool a2dPin::IsTemporary_DontSave() const
{
    // pins are usual objects unlike handles, so skip the overload in a2dHandle
    return a2dCanvasObject::IsTemporary_DontSave();
}

bool a2dPin::AlwaysWriteSerializationId() const
{
    return true;
}

bool a2dPin::LinkReference( a2dObject* other )
{
    if( !other )
        return false;

    a2dPin* canvaspin = wxDynamicCast( other, a2dPin );

    if ( !canvaspin )
    {
        wxString reference_name = GetName();
        a2dGeneralGlobals->ReportErrorF( a2dError_LinkPin, _( "a2dPin with name %s \n Linked to wrong object type" ), reference_name.c_str() );
        return false;
    }

    if( !IsConnectedTo( canvaspin ) )
    {
        ConnectTo( canvaspin );
    }
    return true;
}

bool a2dPin::FindWiresToPin( a2dPin* pinTo, a2dCanvasObjectList* result )
{    
    bool found = false;

    SetBin( true ); // pin was processed

    a2dPinList::const_iterator iterconp;
    for ( iterconp = m_connectedPins.begin( ) ; iterconp != m_connectedPins.end( ) ; iterconp++ )
    {
        a2dPin* pincother = *iterconp;
        // the bin flag on pin prevents going back where we came from, or continuing to an already passed object.
        if ( !pincother )
            continue;
        if ( pincother->GetRelease() || pincother->GetBin() )                
            continue;

        wxASSERT_MSG( 
                pincother->IsConnectedTo( this )
                , _( "connected pin not reflected in connected pin" ) );

        //we are on the pin asked for, or we pass all pins, and not go onto branches from this wire.
        if ( pincother == pinTo )
        {
            found = true;
            return found;
        }                 

        // we search for wires on this pin, or on connected pins.
        a2dCanvasObject* parentOther = pincother->GetParent();
        if ( parentOther->IsConnect() || pincother->GetParent()->IsVirtConnect() )
        {
			a2dPinPtr wentToPinHere = NULL;
            // we are on a wire and go to all pins in that wire, and maybe find the path to pinTo
            found = parentOther->FindWiresPinToPin2( pincother, pinTo, result, wentToPinHere );
            if ( found )
            {    
                //zero lenght wire optimization.
                // outcommented, since this means a connection can be lost, 
                // e.g. if the marked wires are used for removing unconnected objects and such.
                // A zero wire can be removed only by merging its pins first.
                //if ( !IsSameLocation( wentToPinHere ) ) 
                {
                    parentOther->SetBin2( true );
                    if ( result )
                        result->push_back( parentOther );
                }
                return found;
            }
        }
        else
        {   //a non wire object pin, but it can have connections to other wires at its pin (not passing that object, still go to other wires)        
            found = pincother->FindWiresToPin( pinTo, result );
            if ( found )
                return found;
        }

    }

    return found;
}


#ifdef _DEBUG

void a2dPin::DoDump( int indent, wxString* line )
{
    a2dCanvasObject::DoDump( indent, line );
    *line += wxString::Format( _T( " name=%s" ), GetName() );
    a2dPinList::const_iterator iter;
    for ( iter = m_connectedPins.begin( ) ; iter != m_connectedPins.end( ) ; iter++ )
    {
        a2dPin* connectedPin = *iter;
        if ( connectedPin )
        {
            *line += wxString::Format( _T( " cp=%p" ), connectedPin );
        }
    }
}

#endif

