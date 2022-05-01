/*! \ file canvas/src/recur.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: recur.cpp,v 1.107 2009/09/26 20:40:32 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/cameleon.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/algos.h"

#include <wx/wfstream.h>
#include <math.h>
#include <float.h>


//----------------------------------------------------------------------------
// a2dHasParameters
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dHasParameters, a2dCanvasObject )

a2dHasParameters::a2dHasParameters( double x, double y )
    : a2dCanvasObject( x, y )
{
}

a2dHasParameters::~a2dHasParameters()
{
}

a2dHasParameters::a2dHasParameters( const a2dHasParameters& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
}

a2dObject* a2dHasParameters::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    a2dHasParameters* a = new a2dHasParameters( *this, options, refs );
    return a;
}

void a2dHasParameters::AddStringParameter( const wxString& name, const wxString& value )
{
    a2dPropertyId* propid = GetParameterId( name );
    if ( !propid )
        propid = new a2dPropertyIdString( name, wxT( "" ), a2dPropertyId::flag_userDefined );
    a2dPropertyIdString* strprop = NULL;
    strprop = dynamic_cast<a2dPropertyIdString*>( propid );
    if ( !strprop )
        wxLogWarning( _T( "property with this name already exists, but with different type, skipped" ) );
    else
    {
        a2dStringProperty* nprop = new a2dStringProperty( strprop, value );
        m_parametermap[ propid ] = nprop;
    }
}

void a2dHasParameters::AddIntegerParameter( const wxString& name, wxInt32 value )
{
    a2dPropertyId* propid = GetParameterId( name );
    if ( !propid )
        propid = new a2dPropertyIdInt32( name, 0, a2dPropertyId::flag_userDefined );
    a2dPropertyIdInt32* intprop = NULL;
    intprop = dynamic_cast<a2dPropertyIdInt32*>( propid );
    if ( !intprop )
        wxLogWarning( _T( "property with this name already exists, but with different type, skipped" ) );
    else
    {
        a2dInt32Property* nprop = new a2dInt32Property( intprop, value );
        m_parametermap[ propid ] = nprop;
    }
}

void a2dHasParameters::AddBoolParameter( const wxString& name, bool value )
{
    a2dPropertyId* propid = GetParameterId( name );
    if ( !propid )
        propid = new a2dPropertyIdBool( name, false, a2dPropertyId::flag_userDefined );
    a2dPropertyIdBool* boolprop = NULL;
    boolprop = dynamic_cast<a2dPropertyIdBool*>( propid );
    if ( !boolprop )
        wxLogWarning( _T( "property with this name already exists, but with different type, skipped" ) );
    else
    {
        a2dBoolProperty* nprop = new a2dBoolProperty( boolprop, value );
        m_parametermap[ propid ] = nprop;
    }
}

void a2dHasParameters::AddDoubleParameter( const wxString& name, double value )
{
    a2dPropertyId* propid = GetParameterId( name );
    if ( !propid )
        propid = new a2dPropertyIdDouble( name, false, a2dPropertyId::flag_userDefined );
    a2dPropertyIdDouble* doubleprop = NULL;
    doubleprop = dynamic_cast<a2dPropertyIdDouble*>( propid );
    if ( !doubleprop )
        wxLogWarning( _T( "property with this name already exists, but with different type, skipped" ) );
    else
    {
        a2dDoubleProperty* nprop = new a2dDoubleProperty( doubleprop, value );
        m_parametermap[ propid ] = nprop;
    }
}

void a2dHasParameters::TakeParameters( a2dHasParameters* parInst ) const
{
    for ( a2dParameterMap::const_iterator i = m_parametermap.begin(); i != m_parametermap.end(); i++ )
    {
        a2dPropertyIdPtr propId = (*i).first;
        if ( propId->IsUserDefined() )
        {
            a2dNamedPropertyPtr namedprop = (*i).second;
            namedprop = namedprop->Clone( clone_deep );
            parInst->GetParameters().operator[](propId) = namedprop;
        }
    }
}

void a2dHasParameters::SetParameter( a2dPropertyId *dynproperty, a2dNamedProperty* namedproperty )
{
    m_parametermap[ dynproperty ] = namedproperty;
}

a2dNamedProperty* a2dHasParameters::GetParameter( const a2dPropertyIdPtr id ) const
{
    m_parametermap.find( id );
    return (* m_parametermap.find( id )).second;
}

a2dPropertyId* a2dHasParameters::GetParameterId( const wxString &idName ) const
{
    for ( a2dParameterMap::const_iterator i = m_parametermap.begin(); i != m_parametermap.end(); i++ )
    {
        a2dPropertyIdPtr propId = (*i).first;
        if ( propId->GetName() == idName )
            return propId;
    }
    return 0;
}

a2dNamedProperty* a2dHasParameters::GetParameter( const wxString &idName ) const
{
    for ( a2dParameterMap::const_iterator i = m_parametermap.begin(); i != m_parametermap.end(); i++ )
    {
        a2dPropertyIdPtr propId = (*i).first;
        if ( propId->GetName() == idName )
        {
            return (*i).second;
        }
    }
    return 0;
}


//----------------------------------------------------------------------------
// a2dCommand_Port
//----------------------------------------------------------------------------

//! specialized command using the Assign method of a2dPort to transfer changes in editcopy to original.
class A2DCANVASDLLEXP a2dCommand_Port: public a2dCommand
{
public:
    static const a2dCommandId Id;

    a2dCommand_Port() { m_target = NULL; }
    a2dCommand_Port( a2dPort* target, const wxString& portName );
    ~a2dCommand_Port( void );

    bool Do();
    bool Undo();

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

protected:
    a2dSmrtPtr<a2dPort> m_target;
    wxString m_portName;
};

const a2dCommandId a2dCommand_Port::Id( "a2dPort" );

a2dCommand_Port::a2dCommand_Port( a2dPort* target, const wxString& portName )
    : a2dCommand( true, a2dCommand_Port::Id )
{
    m_target = target;
    m_portName = portName;
}

a2dCommand_Port::~a2dCommand_Port( void )
{
}

bool a2dCommand_Port::Do( void )
{
    wxString portName = m_target->GetName();
    m_target->SetName( m_portName );
    m_target->SetPending( true );
    m_portName = portName;
    return true;
}

bool a2dCommand_Port::Undo( void )
{
    Do();
    return true;
}

//----------------------------------------------------------------------------
// a2dPort
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dPort, a2dCanvasObject )

a2dPropertyIdCanvasObject* a2dPort::PROPID_wasConnectedTo = NULL;

INITIALIZE_PROPERTIES( a2dPort, a2dCanvasObject )
{
    PROPID_wasConnectedTo = new a2dPropertyIdCanvasObject(  wxT( "wasConnectedTo" ),
            (a2dPropertyId::Flags) (a2dPropertyId::flag_temporary & ~a2dPropertyId::flag_none), 0 );
    AddPropertyId( PROPID_wasConnectedTo );
    return true;
}

double a2dPort::m_l1 = 1;
double a2dPort::m_l2 = 2;
double a2dPort::m_l3 = 3;
double a2dPort::m_b = 2;
bool a2dPort::m_doRender = true;

a2dPort::a2dPort()
    :
    a2dCanvasObject()
{
    wxString idstr;
    idstr << m_id;
    m_name = idstr;
    m_parPinClass = a2dPinClass::Standard;
}

a2dPort::a2dPort( a2dCanvasObject* parent, double x, double y, const wxString& name, a2dPinClass* pinclass, a2dPinClass* parPinClass )
    :
    a2dCanvasObject()
{
    SetPosXY( x, y );
    wxString idstr;
    idstr << m_id;
    m_name = idstr;
	if ( !name.IsEmpty() )
		m_name = name;
	m_parent = parent;

    a2dPin* pin = new a2dPin( this, wxT( "1" ), pinclass, 0.0, 0.0 );
    Append( pin );
    m_parPinClass = parPinClass;
}

a2dPort::~a2dPort()
{
}

a2dPort::a2dPort( const a2dPort& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
	wxString idstr;
    idstr << m_id;
    m_name = idstr;

    if ( options & clone_sameName )
	    m_name = other.m_name;

	m_parent = other.m_parent;
    m_parPinClass = other.m_parPinClass;

    if ( refs )
    {
	    wxLongLong id = other.GetId();
	    wxASSERT( id != 0 );
        wxString resolveKey;
        resolveKey << id;
        refs->GetObjectHashMap()[resolveKey] = this;
    }
}

a2dObject* a2dPort::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    a2dPort* a = new a2dPort( *this, options, refs );
    return a;
}

void a2dPort::SetParent( a2dCanvasObject* parent )
{
    m_parent = parent;
}

a2dPin* a2dPort::GetPin() const
{
    a2dPin* find = NULL;
    a2dCanvasObjectList::const_iterator iter = m_childobjects->begin();
    while ( iter != m_childobjects->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dPin );
        if ( find )
        {
            return find;
        }
        iter++;
    }
    return NULL;
}

void a2dPort::Set( double l1, double l2, double l3, double b )
{
    m_l1 = l1;
    m_l2 = l2;
    m_l3 = l3;
    m_b  = b;
}

a2dBoundingBox a2dPort::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    double grid = GetHabitat()->GetObjectGridSize();
    a2dBoundingBox bbox;
    bbox.Expand( 0 , grid * - m_b / 2 );
    bbox.Expand( grid * m_l3 , grid * m_b / 2 );
    return bbox;
}

void a2dPort::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    if ( !m_doRender )
        return;

    double grid = GetHabitat()->GetObjectGridSize();

    a2dVertexArray points;
    points.push_back( new a2dLineSegment( 0, 0 ) );
    points.push_back( new a2dLineSegment( grid * m_l1, grid * m_b / 2.0 ) );
    points.push_back( new a2dLineSegment( grid * m_l3, grid * m_b / 2.0 ) );
    points.push_back( new a2dLineSegment( grid * m_l2, 0 ) );
    points.push_back( new a2dLineSegment( grid * m_l3, grid * -m_b / 2.0 ) );
    points.push_back( new a2dLineSegment( grid * m_l1, grid * -m_b / 2.0 ) );

    ic.GetDrawer2D()->DrawPolygon( &points, false, wxWINDING_RULE );
}

bool a2dPort::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double grid = GetHabitat()->GetObjectGridSize();
    a2dVertexArray points;
    points.push_back( new a2dLineSegment( 0, 0 ) );
    points.push_back( new a2dLineSegment( grid * m_l1, grid * m_b / 2.0 ) );
    points.push_back( new a2dLineSegment( grid * m_l3, grid * m_b / 2.0 ) );
    points.push_back( new a2dLineSegment( grid * m_l2, 0 ) );
    points.push_back( new a2dLineSegment( grid * m_l3, grid * -m_b / 2.0 ) );
    points.push_back( new a2dLineSegment( grid * m_l1, grid * -m_b / 2.0 ) );

    a2dPoint2D P = a2dPoint2D( hitEvent.m_relx, hitEvent.m_rely );
    hitEvent.m_how = points.HitTestPolygon( P, ic.GetWorldStrokeExtend() + ic.GetTransformedHitMargin() );

    return hitEvent.m_how.IsHit();
}


#if wxART2D_USE_CVGIO
void a2dPort::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "pinclass" ), m_parPinClass->GetName() );
    }
    else
    {
    }
}

void a2dPort::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );

    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString idstr;
        idstr << m_id;
        m_name = parser.GetAttributeValue( wxT( "name" ), idstr );
        wxString pinclass = parser.GetAttributeValue( wxT( "pinclass" ) );
        m_parPinClass = a2dPinClass::GetClassByName( pinclass );
        if( !m_parPinClass )
        {
            m_parPinClass = a2dPinClass::Standard;
            wxString reference_name = GetName();
            a2dGeneralGlobals->ReportErrorF( a2dError_NoPinClass, wxT( "a2dPin with name %s \n Pin class %s not found (replaced with Standard)" ), reference_name.c_str(), pinclass.c_str() );
        }
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dPort::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        a2dPort* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dPort );
        original->SetSelected( true );

		wxString newName = wxGetTextFromUser( "give name for the port:", "Port Naming", GetName() );
        if ( !newName.IsEmpty() )
        {
            m_root->GetCommandProcessor()->Submit( new a2dCommand_Port( original, newName ) );
        }
        // we trigger end editing directly, since all editing is finished after closing the dialog.
        original->SetEditing( false );
        return true;
    }
    return false;
}


//----------------------------------------------------------------------------
// a2dParPin
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dParPin, a2dPin )

a2dParPin::a2dParPin()
    : a2dPin()
{
    SetVisible( true );
}

a2dParPin::a2dParPin( a2dCameleonInst* parent, a2dPort* portPin, a2dPinClass* parPinClass )
    : a2dPin( parent,
        portPin->GetName(),
        parPinClass,
        portPin->GetPosX(), portPin->GetPosY(),
        portPin->GetPin()->GetAbsAngle(),
        parPinClass->GetParPin()->GetWidth(),
        parPinClass->GetParPin()->GetHeight(),
        parPinClass->GetParPin()->GetRadius() )
{
    SetVisible( true );
    m_port = portPin;
}

a2dParPin::a2dParPin( a2dPinClass* parPinClass, double w, double h )
    : a2dPin( NULL,
        "unDefined",
        parPinClass,
        0,0,
        0,
        w,
        h,
        0 )
{
    SetVisible( true );
    m_port = NULL;
}

a2dParPin::~a2dParPin()
{
}

a2dParPin::a2dParPin( const a2dParPin& other, CloneOptions options, a2dRefMap* refs )
    : a2dPin( other, options, refs )
{
    m_port = other.m_port;
    if ( !(options & clone_noCameleonRef) && (options & clone_members)  )
    {
        m_port = NULL; // must be synced from above

        if( refs )
        {
	        wxASSERT_MSG( refs, wxT( "for reconnect a2dRefMap needed" ) );

            //Here we add make a connection to the cloned port based on the id of the other port.
            //In a2dPort for the cloned port the same id is added to map to here.
		    wxLongLong id = other.m_port->GetId();
		    wxASSERT( id != 0 );
		    wxString resolveKey;
		    resolveKey << id;
		    refs->ResolveOrAddLink( this, resolveKey );
        }
    }
}

a2dObject* a2dParPin::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    a2dParPin* a = new a2dParPin( *this, options, refs );
    return a;
}

bool a2dParPin::LinkReference( a2dObject* other )
{
    a2dPin* canvaspin = wxDynamicCast( other, a2dPin );
    if ( canvaspin )
        return a2dPin::LinkReference( other );

    if( !other )
        return false;

    if ( m_port.Get() )
        return false;

    a2dPort* port = wxDynamicCast( other, a2dPort );
    if ( !port )
    {
        wxString reference_name = GetName();
        //a2dGeneralGlobals->ReportErrorF( a2dError_LinkPin, _( "a2dParPin with name %s \n Linked to wrong object type" ), reference_name.c_str() );
        return false;
    }
    port->SetCheck( true );
    m_port = port;
    return true;
}

#if wxART2D_USE_CVGIO
void a2dParPin::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dPin::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "portid" ), m_port->GetId() );
    }
    else
    {
    }
}

void a2dParPin::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dPin::DoLoad( parent, parser, xmlparts );

    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( parser.HasAttribute( wxT( "portid" ) ) )
        {
            wxString portid = parser.GetAttributeValue( wxT( "portid" ) );
            parser.ResolveOrAddLink( this, portid );
        }
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dAppear
//----------------------------------------------------------------------------

bool a2dAppear::m_nextLine = false;

IMPLEMENT_CLASS( a2dAppear, a2dCanvasObject )

BEGIN_EVENT_TABLE( a2dAppear, a2dCanvasObject )
    EVT_CHANGEDMODIFY_DRAWING( a2dAppear::OnChangeDrawings )
END_EVENT_TABLE()

a2dAppear::a2dAppear( a2dCameleon* cameleon, double x, double y, bool offsetDrawing )
    : a2dCanvasObject( x, y )
{
    m_cameleon = cameleon;
    m_offsetDrawing = offsetDrawing;
}

a2dAppear::a2dAppear( const a2dAppear& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_cameleon = other.m_cameleon;
    m_offset = other.m_offset;
    m_offsetDrawing = other.m_offsetDrawing;
}

a2dAppear::~a2dAppear()
{
}

a2dObject* a2dAppear::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dAppear( *this, options, refs );
}

#if wxART2D_USE_CVGIO
void a2dAppear::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
}

void a2dAppear::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
}
#endif //wxART2D_USE_CVGIO

void a2dAppear::OnChangeDrawings( a2dDrawingEvent& event )
{
    m_cameleon->ProcessEvent( event );
}

//----------------------------------------------------------------------------
// a2dDiagram
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dDiagram, a2dAppear )

BEGIN_EVENT_TABLE( a2dDiagram, a2dAppear )
    EVT_UPDATE_DRAWING( a2dDiagram::OnUpdateFromDrawing )
END_EVENT_TABLE()

a2dDiagram::a2dDiagram()
    : 
    m_diagramDrawing( NULL ),
    a2dAppear( NULL, 0, 0 )
{
}

a2dDiagram::a2dDiagram( a2dCameleon* cameleon, double x, double y )
    : 
    a2dAppear( cameleon, x, y )
{
    wxASSERT_MSG( m_cameleon, wxT( "a2dCamelon not set in a2dDiagram" ) );
    if ( m_cameleon->GetHabitat() )
    {
        m_diagramDrawing = (a2dDrawing*) m_cameleon->GetHabitat()->GetDiagramDrawingTemplate()->Clone( clone_deep );
    }
    else
        m_diagramDrawing = new a2dDrawing( wxEmptyString, NULL );
    m_diagramDrawing->SetHabitat( m_cameleon->GetHabitat() );
    m_diagramDrawing->SetParent( this );
    //m_diagramDrawing->ConnectEvent( wxEVT_UPDATE_DRAWING, this );
}

a2dDiagram::a2dDiagram( const a2dDiagram& other, CloneOptions options, a2dRefMap* refs )
    : a2dAppear( other, options, refs )
{
    if ( ! (options & clone_members) )
    {
        m_diagramDrawing = other.m_diagramDrawing;
    }
    else if ( options & clone_members && other.m_diagramDrawing )
    {
        m_diagramDrawing = other.m_diagramDrawing->TClone( CloneOptions( options | clone_noCameleonRef ), refs );
        m_diagramDrawing->SetParent( this );
        //m_diagramDrawing->ConnectEvent( wxEVT_UPDATE_DRAWING, this );
    }
    else 
    {
        m_diagramDrawing = other.m_diagramDrawing;
    }
}

a2dObject* a2dDiagram::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dDiagram( *this, options, refs );
}

a2dDiagram::~a2dDiagram()
{
    //if ( m_diagramDrawing )
        //m_diagramDrawing->DisconnectEvent( wxEVT_UPDATE_DRAWING, this );
}

a2dBoundingBox a2dDiagram::GetUnTransformedBboxNoPorts() const
{
    a2dBoundingBox childbox;

    a2dBoundingBox bbox;
    a2dPort* find = NULL;
    a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->CreateChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dPort );
        if ( !find )
        {
            childbox.Expand( ( *iter )->GetBbox() );
        }
        else
            //only expand with its position, not its size, since that is not rendered at Cameleon level.
            childbox.Expand( ( *iter )->GetPosXY() );
        iter++;
    }
    if ( !childbox.GetValid() )
        childbox.Expand( 0, 0 );
    //childbox.MapBbox( m_lworld );
    bbox.Expand( childbox );
    return bbox;
}

void a2dDiagram::OnUpdateFromDrawing( a2dDrawingEvent& event )
{
    if (event.GetUpdateHint() & a2dCANVIEW_UPDATE_PENDING_POSTUPDATE)
    {
        a2dPort* find = NULL;
        a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
        while ( iter != m_diagramDrawing->GetRootObject()->GetChildObjectList()->end() )
        {
            find = wxDynamicCast( ( *iter ).Get(), a2dPort );
            if ( find && find->GetPending() )
            {
                GetCameleon()->TriggerChangedTime();
            }
            iter++;
        }

        CalculateOffset();
    }
    event.Skip();
}

void a2dDiagram::CalculateOffset()
{
    a2dCanvasObject* find = NULL;
    if ( find = FindOrigin() )
    {
        m_offset = a2dAffineMatrix( -find->GetPosX(), -find->GetPosY() );
    }
    else if ( m_offsetDrawing )
    {
        m_offset = a2dIDENTITY_MATRIX;
    }
    else if ( find = FindLeftUp() )
    {
        m_offset = a2dAffineMatrix( -find->GetPosX(), -find->GetPosY() );
    }
    else
    {
        a2dBoundingBox box = GetUnTransformedBboxNoPorts();
        m_offset = a2dAffineMatrix( box.GetCentre().m_x, box.GetCentre().m_y );
    }
}

a2dCanvasObject* a2dDiagram::PushInto( a2dCanvasObject* parent )
{
    return m_diagramDrawing->PushInto(  parent );
}

/*
wxLongLong a2dDiagram::FindPortNrNext( const wxString& prefix )
{
    a2dPort* find = NULL;
    a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->GetChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dPort );
        if ( find )
        {
        }
        iter++;
    }
}
*/

void a2dDiagram::TakePortsTo( a2dCameleonInst* parInst )
{
    CalculateOffset();

    double dx = m_offset.GetValue( 2, 0 );
    double dy = m_offset.GetValue( 2, 1 );

    a2dPort* find = NULL;
    a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->GetChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dPort );
        if ( find )
        {
            a2dParPin* parPin = NULL;
            if (  find->GetParPinClass() && find->GetParPinClass()->GetParPin() )
            {
                parPin = (a2dParPin*) ( find->GetParPinClass()->GetParPin()->Clone( clone_deep ) );
                parPin->SetPort( find );
                parPin->Set( dx + find->GetPosX(), dy + find->GetPosY(), find->GetAngle(), find->GetName() );
            }
            else
            {
                parPin = new a2dParPin( parInst, find, find->GetParPinClass() );
                parPin->Set( dx + find->GetPosX(), dy + find->GetPosY(), find->GetAngle(), find->GetName() );
            }
            parInst->Append( parPin );
            a2dPin* wasConnectedTo = wxStaticCastNull( a2dPort::PROPID_wasConnectedTo->GetPropertyValue( find ).Get(), a2dPin );
            if ( wasConnectedTo )
            {
                wasConnectedTo->ConnectTo( parPin );
                find->RemoveProperty( a2dPort::PROPID_wasConnectedTo );
            }
        }
        iter++;
    }
}

void a2dDiagram::CreateSymbol( a2dDrawing* symbolDrawing, double x, double y)
{
    double portStep = GetHabitat()->GetPortDistance();
    double symbolwidth = GetHabitat()->GetSymbolSize();

    int incx = 0;
    int incy = 0;
    int nrPorts = 0;

    a2dPort* find = NULL;
    a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->GetChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dPort );
        if ( find )
            nrPorts++;
        iter++;
    }

    iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->GetChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dPort );
        if ( find )
        {
            if ( incy == (nrPorts+1)/2 )
            {
                incy = 0; incx++;
            }
            a2dPort* port = new a2dPort( symbolDrawing->GetRootObject(), x + incx * symbolwidth, y + incy * portStep, find->GetName(), find->GetPin()->GetPinClass(), find->GetParPinClass() );
            symbolDrawing->Append( port );

            a2dPin* wasConnectedTo = wxStaticCastNull( a2dPort::PROPID_wasConnectedTo->GetPropertyValue( find ).Get(), a2dPin );
            if ( wasConnectedTo )
            {
                a2dPort::PROPID_wasConnectedTo->SetPropertyToObject( port, wasConnectedTo );
                find->RemoveProperty( a2dPort::PROPID_wasConnectedTo );
            }

            incy++;
            if ( incx < 1 )
                port->SetRotation( 180 );
        }
        iter++;
    }
    symbolDrawing->Append( new a2dRect( x, y - portStep, symbolwidth, (nrPorts+2)/2 * portStep + portStep )  ); 
}

void a2dDiagram::TranslateTo( double dx, double dy )
{
    a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->GetChildObjectList()->end() )
    {
        a2dCanvasObject* obj = ( *iter );
        if ( obj )
        {
            obj->Translate( dx, dy );
        }
        iter++;
    }
}

a2dPort* a2dDiagram::FindLeftUp() const
{
    double xmin =  DBL_MAX;
    double ymax =  -DBL_MAX;

    a2dPort* find = NULL;
    a2dPort* best = NULL;
    a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->GetChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dPort );
        if ( find )
        {
            if ( xmin > find->GetPosX() )
            {
                best = find;
                xmin = best->GetPosX();
            }
            else if ( xmin == find->GetPosX() )
            {
                best = find;
                ymax = find->GetPosY();
            }
        }
        iter++;
    }
    return best;
}

a2dOrigin* a2dDiagram::FindOrigin() const
{
    a2dOrigin* find = NULL;
    a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->GetChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dOrigin );
        if ( find )
            return find;
        iter++;
    }
    return find;
}

a2dPort* a2dDiagram::FindPort( a2dPort* symPin ) const
{
    a2dPort* find = NULL;
    a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->GetChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dPort );
        if ( find && find->GetName() == symPin->GetName() )
            return find;
        iter++;
    }
    return NULL;
}

a2dPort* a2dDiagram::FindPort( a2dParPin* parPin ) const
{
    a2dPort* find = NULL;
    a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->GetChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dPort );
        if ( find && find->GetName() == parPin->GetName() )
            return find;
        iter++;
    }
    return NULL;
}

a2dPort* a2dDiagram::FindPortByName( const wxString& parPinName ) const
{
    a2dPort* find = NULL;
    a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->GetChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dPort );
        if ( find && find->GetName() == parPinName )
            return find;
        iter++;
    }
    return NULL;
}

void a2dDiagram::TakeVisibleParameters( a2dCameleonInst* parInst )
{
    a2dVisibleParameter* find = NULL;
    a2dCanvasObjectList::iterator iter = m_diagramDrawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != m_diagramDrawing->GetRootObject()->CreateChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dVisibleParameter );
        if ( find )
        {
            a2dVisibleParameter* visPar = wxDynamicCast( find->Clone( a2dObject::clone_deep ), a2dVisibleParameter );
            visPar->SetParent( parInst );
            parInst->Append( visPar );
        }
        iter++;
    }
}

void a2dDiagram::DependencyPending( a2dWalkerIOHandler* handler )
{
    a2dCanvasObject* parent = wxDynamicCast( handler->GetParent(), a2dCanvasObject );
    if ( parent && parent->GetHighLight() && !parent->GetPending() && GetPending() )
    {
        parent->SetPending( true );
    }
}

void a2dDiagram::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    a2dCanvasObject::DoWalker( parent, handler );

    if ( m_diagramDrawing )
        m_diagramDrawing->Walker( this, handler );
}

#if wxART2D_USE_CVGIO
void a2dDiagram::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dAppear::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_diagramDrawing )
            out.WriteAttribute( wxT( "drawing" ), m_diagramDrawing->GetId() );
    }
    else
    {
        if ( m_diagramDrawing && !m_diagramDrawing->GetCheck() )
        {
            a2dObjectPtr multiRef = m_diagramDrawing.Get();

            out.WriteStartElement( wxT( "drawing" ) );
            m_diagramDrawing->Save( this, out, towrite, NULL );
            out.WriteEndElement();
        }
    }
}

void a2dDiagram::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dAppear::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( parser.HasAttribute( wxT( "drawing" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_diagramDrawing, parser.GetAttributeValue( wxT( "drawing" ) ) );
        }
    }
    else
    {
        if ( parser.GetTagName() != wxT( "drawing" ) )
            return;

        parser.Require( START_TAG, wxT( "drawing" ) );
        parser.Next();

        m_diagramDrawing = (a2dDrawing*) parser.LoadOneObject( this );
        m_diagramDrawing->SetParent( this );
        //m_diagramDrawing->ConnectEvent( wxEVT_UPDATE_DRAWING, this );

        parser.Require( END_TAG, wxT( "drawing" ) );
        parser.Next();

        CalculateOffset();
    }
}
#endif //wxART2D_USE_CVGIO

a2dBoundingBox a2dDiagram::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{    
    double grid = GetHabitat()->GetObjectGridSize();

    a2dBoundingBox bbox;
    bbox.Expand( 0, 0 );
    bbox.Expand( 30*grid, m_nextLine ? 4*grid: -4*grid );
    return bbox;
}

void a2dDiagram::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    double grid = GetHabitat()->GetObjectGridSize();
    a2dDrawer2D *d(ic.GetDrawer2D());

	int align = m_nextLine ? wxMINX|wxMAXY: wxMINX|wxMINY;
    
    d->DrawRoundedRectangle( 0, 0, 30*grid, m_nextLine ? 4*grid: -4*grid , 1*grid );
    d->SetFont( a2dCanvasModule::GetFontMedBold() );
    d->DrawText( " diagram : " + GetName(), 0, m_nextLine ? 1.5*grid: -1.5*grid, align, false);
    d->SetFont( a2dCanvasModule::GetFontSmall() );
    wxString classn = GetClassInfo()->GetClassName();
    classn = "type: " + classn;
    d->DrawText( classn, 2, m_nextLine ? 2.6*grid: -2.6*grid, align, false);
}

bool a2dDiagram::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double grid = GetHabitat()->GetObjectGridSize();
    double margin = ic.GetTransformedHitMargin();

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, 0, 0, 30*grid, m_nextLine ? 4*grid: -4*grid, ic.GetWorldStrokeExtend() + margin );

    return hitEvent.m_how.IsHit();
}



//----------------------------------------------------------------------------
// a2dSymbol
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dSymbol, a2dDiagram )

a2dSymbol::a2dSymbol()
    : a2dDiagram()
{
}

a2dSymbol::a2dSymbol( a2dCameleon* cameleon, double x, double y )
    : a2dDiagram( cameleon, x, y )
{
    wxASSERT_MSG( m_cameleon, wxT( "a2dCamelon not set in a2dDiagram" ) );
    if ( m_cameleon->GetHabitat() )
    {
        m_diagramDrawing = (a2dDrawing*) m_cameleon->GetHabitat()->GetSymbolDrawingTemplate()->Clone( clone_deep );
    }
    else
        m_diagramDrawing = new a2dDrawing( wxEmptyString, NULL );
    m_diagramDrawing->SetHabitat( m_cameleon->GetHabitat() );
    m_diagramDrawing->SetParent( this );
    //m_diagramDrawing->ConnectEvent( wxEVT_UPDATE_DRAWING, this );
}

a2dSymbol::a2dSymbol( const a2dSymbol& other, CloneOptions options, a2dRefMap* refs )
    : a2dDiagram( other, options, refs )
{
}

a2dObject* a2dSymbol::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dSymbol( *this, options, refs );
}

a2dSymbol::~a2dSymbol()
{
}

void a2dSymbol::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    double grid = GetHabitat()->GetObjectGridSize();
    a2dDrawer2D *d(ic.GetDrawer2D());
    
	int align = m_nextLine ? wxMINX|wxMAXY: wxMINX|wxMINY;

    d->DrawRoundedRectangle( 0, 0, 30*grid, m_nextLine ? 4*grid: -4*grid , 1 );
    d->SetFont( a2dCanvasModule::GetFontMedBold() );
    d->DrawText( " symbol : " + GetName(), 0, m_nextLine ? 1.5*grid: -1.5*grid, align, false);
    d->SetFont( a2dCanvasModule::GetFontSmall() );
    wxString classn = GetClassInfo()->GetClassName();
    classn = "type: " + classn;
    d->DrawText( classn, 2*grid, m_nextLine ? 2.6*grid: -2.6*grid, align, false);
}

//----------------------------------------------------------------------------
// a2dBuildIn
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dBuildIn, a2dAppear )

BEGIN_EVENT_TABLE( a2dBuildIn, a2dAppear )
    EVT_UPDATE_DRAWING( a2dBuildIn::OnUpdateFromDrawing )
END_EVENT_TABLE()

a2dBuildIn::a2dBuildIn()
    : 
    m_drawing( NULL ),
    a2dAppear( NULL, 0, 0 )
{
}

a2dBuildIn::a2dBuildIn( a2dCameleon* cameleon, double x, double y, a2dCanvasObject* buildIn )
    : 
    a2dAppear( cameleon, x, y )
{
    wxASSERT_MSG( m_cameleon, wxT( "a2dCamelon not set in a2dDiagram" ) );
    if ( m_cameleon->GetHabitat() )
    {
        m_drawing = (a2dDrawing*) m_cameleon->GetHabitat()->GetBuildInDrawingTemplate()->Clone( clone_deep );
    }
    else
        m_drawing = new a2dDrawing( wxEmptyString, NULL );
    m_drawing->SetHabitat( m_cameleon->GetHabitat() );
    m_drawing->GetRootObject()->Append( buildIn );
    m_drawing->ConnectEvent( wxEVT_UPDATE_DRAWING, this );
    m_drawing->SetMayEdit( false );
}

a2dBuildIn::a2dBuildIn( const a2dBuildIn& other, CloneOptions options, a2dRefMap* refs )
    : a2dAppear( other, options, refs )
{
    if ( ! (options & clone_members) )
    {
        m_drawing = other.m_drawing;
    }
    else if ( options & clone_members && other.m_drawing )
    {
        m_drawing = other.m_drawing->TClone( CloneOptions( options | clone_noCameleonRef ) );
        m_drawing->ConnectEvent( wxEVT_UPDATE_DRAWING, this );
    }
    else 
    {
        m_drawing = other.m_drawing;
    }
}

a2dObject* a2dBuildIn::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dBuildIn( *this, options, refs );
}

a2dBuildIn::~a2dBuildIn()
{
    if ( m_drawing )
        m_drawing->DisconnectEvent( wxEVT_UPDATE_DRAWING, this );
}

a2dCanvasObject* a2dBuildIn::GetBuildIn() const
{
    return *(m_drawing->GetRootObject()->GetChildObjectList()->begin());
}

void a2dBuildIn::SetBuildIn( a2dCanvasObject* buildIn )
{
    m_drawing->GetRootObject()->ReleaseChildObjects();
    m_drawing->GetRootObject()->Append( buildIn );
}

void a2dBuildIn::OnUpdateFromDrawing( a2dDrawingEvent& event )
{
    if (event.GetUpdateHint() & a2dCANVIEW_UPDATE_PENDING_POSTUPDATE)
    {
        a2dPort* find = NULL;
        a2dCanvasObjectList::iterator iter = m_drawing->GetRootObject()->CreateChildObjectList()->begin();
        while ( iter != m_drawing->GetRootObject()->GetChildObjectList()->end() )
        {
            find = wxDynamicCast( ( *iter ).Get(), a2dPort );
            if ( find && find->GetPending() )
            {
                GetCameleon()->TriggerChangedTime();
            }
            iter++;
        }
        event.Skip();
    }
}

#if wxART2D_USE_CVGIO
void a2dBuildIn::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dAppear::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
		if ( GetBuildIn() )	
			out.WriteAttribute( "buildin", GetBuildIn()->GetClassInfo()->GetClassName() );
    }
    else
    {
		/*
        if ( m_drawing && !m_drawing->GetCheck() )
        {
            a2dObjectPtr multiRef = m_drawing.Get();

            out.WriteStartElement( "drawing" );
            m_drawing->Save( this, out, towrite, NULL );
            out.WriteEndElement();
        }
        */
		
    }
}

void a2dBuildIn::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
	static wxString lastBuildIn;

    a2dAppear::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        lastBuildIn = parser.GetAttributeValue( "buildin" );
        if ( parser.HasAttribute( "drawing" ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_drawing, parser.GetAttributeValue( "drawing" ) );
        }
    }
    else
    {
        if ( parser.GetTagName() != "drawing" )
		{
			m_drawing = new a2dDrawing();
			a2dCanvasObject* obj = wxStaticCast( parser.CreateObject( lastBuildIn ), a2dCanvasObject );
            obj->Initialize();
			m_drawing->GetRootObject()->Append( obj );
			m_drawing->ConnectEvent( wxEVT_UPDATE_DRAWING, this );
			m_drawing->SetMayEdit( false );
            return;
		}
		else
		{
			parser.Require( START_TAG, "drawing" );
			parser.Next();

			m_drawing = (a2dDrawing*) parser.LoadOneObject( this );
			m_drawing->SetParent( this );

			parser.Require( END_TAG, "drawing" );
			parser.Next();
		}
    }
}
#endif //wxART2D_USE_CVGIO

a2dBoundingBox a2dBuildIn::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    double grid = GetHabitat()->GetObjectGridSize();

    a2dBoundingBox bbox;
    bbox.Expand( 0, 0 );
    bbox.Expand( 30*grid, -4*grid );
    return bbox;
}

void a2dBuildIn::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    a2dCanvasObject::DoWalker( parent, handler );

    if ( m_drawing )
        m_drawing->Walker( this, handler );
}

void a2dBuildIn::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    double grid = GetHabitat()->GetObjectGridSize();
    a2dDrawer2D *d(ic.GetDrawer2D());
    
    d->DrawRoundedRectangle( 0, 0, 30*grid, -4*grid , 1*grid );

	int align = m_nextLine ? wxMINX|wxMAXY: wxMINX|wxMINY;

    d->SetFont( a2dCanvasModule::GetFontMedBold() );
    wxString nameBuildIn = "Not Defined";
    if ( GetBuildIn() )
        nameBuildIn = GetBuildIn()->GetClassInfo()->GetClassName();        
    d->DrawText( " buildin : " + GetName() + " : "+ nameBuildIn, 0, -2*grid, align, false);
    d->SetFont( a2dCanvasModule::GetFontSmall() );
    wxString classn = GetClassInfo()->GetClassName();
    classn = "type: " + classn;
    d->DrawText( classn, 2*grid, -3.1*grid, align, false);
}

bool a2dBuildIn::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double grid = GetHabitat()->GetObjectGridSize();
    double margin = ic.GetTransformedHitMargin();

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, 0, 0, 30*grid, -4*grid, ic.GetWorldStrokeExtend() + margin );

    return hitEvent.m_how.IsHit();
}

a2dCanvasObject* a2dBuildIn::PushInto( a2dCanvasObject* parent )
{
    return m_drawing->PushInto(  parent );
}


//----------------------------------------------------------------------------
// a2dParameters
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dParameters, a2dAppear )

a2dParameters::a2dParameters( a2dCameleon* cameleon, double x, double y )
    : a2dAppear( cameleon, x, y )
{
}

a2dParameters::a2dParameters( const a2dParameters& other, CloneOptions options, a2dRefMap* refs )
    : a2dAppear( other, options, refs )
{
}

a2dObject* a2dParameters::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dParameters( *this, options,refs );
}


#if wxART2D_USE_CVGIO
void a2dParameters::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dAppear::DoSave( parent, out, xmlparts, towrite );
}

void a2dParameters::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dAppear::DoLoad( parent, parser, xmlparts );
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dGui
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dGui, a2dAppear )

a2dGui::a2dGui( a2dCameleon* cameleon, double x, double y )
    : a2dAppear( cameleon, x, y )
{
}

a2dGui::a2dGui( const a2dGui& other, CloneOptions options, a2dRefMap* refs )
    : a2dAppear( other, options, refs )
{
}

a2dObject* a2dGui::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dGui( *this, options, refs );
}

#if wxART2D_USE_CVGIO
void a2dGui::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dAppear::DoSave( parent, out, xmlparts, towrite );
}

void a2dGui::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dAppear::DoLoad( parent, parser, xmlparts );
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dCameleon
//----------------------------------------------------------------------------

bool a2dCameleon::m_nextLine = false;


DEFINE_EVENT_TYPE( wxEVT_NEW_CAMELEON )

a2dCanvasObject* a2dCameleon::ms_centralCameleonRoot = NULL;
double a2dCameleon::ms_dx = 1;
double a2dCameleon::ms_dy = -1;

IMPLEMENT_DYNAMIC_CLASS( a2dCameleon, a2dHasParameters )

INITIALIZE_PROPERTIES( a2dCameleon, a2dHasParameters )
{
    return true;
}

BEGIN_EVENT_TABLE( a2dCameleon, a2dHasParameters )
    EVT_CHANGEDMODIFY_DRAWING( a2dCameleon::OnChangeDrawings )
END_EVENT_TABLE()

a2dCameleon::a2dCameleon( const wxString& name, double x, double y, a2dHabitat* habitat )
    : a2dHasParameters( x, y ),
    m_habitat( habitat ),
	m_isMultiRef( true ),
	m_isTemplate( false ),
	m_name( name )
{
    if ( m_habitat )
    {
        m_appearances = (a2dDrawing*) m_habitat->GetDrawingTemplate()->Clone( clone_deep );
        m_appearances->SetName( name );
    }
    else
    {
        m_appearances = new a2dDrawing( wxEmptyString );
        m_habitat = a2dCanvasObject::GetHabitat();
    }
    m_appearances->SetHabitat( m_habitat );
    m_appearances->SetParent( this );
    m_appearances->SetDrawingId( a2dDrawingId_appearances() );
    m_changedInternalAccesstime = wxDateTime::Now(); 
}

a2dCameleon::a2dCameleon( const a2dCameleon& other, CloneOptions options, a2dRefMap* refs )
    : a2dHasParameters( other, options, refs )
{
    wxLongLong id = GetId();
    //wxString camNr = wxString::Format("%s_%lld", other.GetName(), id );
    //SetName( camNr );

	m_habitat = other.m_habitat;
	m_isMultiRef = other.m_isMultiRef;
	m_isTemplate = other.m_isTemplate;
    m_drawingId = other.m_drawingId;
    
    if ( options & clone_toDrag )
    {
        SetName( wxString::Format( wxT( "drag" ) ) );
        m_appearances = other.m_appearances;
    }
    else
    {
        SetName( wxString::Format( wxT( "%lld" ), GetId() ) );
        //cloning is always clone_members, because cameleon is set to the a2dAppear as parent, which can be only one.
        if ( ! (options & clone_members) )
        {
            m_appearances = other.m_appearances;
        }
        else if ( options & clone_members && other.m_appearances )
        {
            m_appearances = other.m_appearances->TClone( CloneOptions( options | clone_noCameleonRef ), refs );
            a2dCanvasObjectList::const_iterator iter = m_appearances->GetRootObject()->GetChildObjectList()->begin();
            while ( iter != m_appearances->GetRootObject()->GetChildObjectList()->end() )
            {
                a2dAppear* appear = wxDynamicCast( ( *iter ).Get(), a2dAppear );
                if ( appear && !appear->GetRelease() )
                    appear->SetCameleon( this );
                iter++;
            }
        }
        else 
        {
            m_appearances = other.m_appearances;
        }
        m_appearances->SetParent( this );
    }

    /*
    m_appearances.clear();
    a2dCanvasObjectList::const_iterator iter = other.m_appearances.begin();
    while ( iter != other.m_appearances.end() )
    {
        a2dAppear* appear = wxDynamicCast( ( *iter ).Get(), a2dAppear );
        if ( appear && !appear->GetRelease() )
        {
            a2dObject* objn = appear->Clone( CloneOptions( options & ~ clone_seteditcopy ) );
            a2dAppear* appearn = wxStaticCast( objn, a2dAppear );
            appearn->SetCameleon( this );
            m_appearances.push_back( appearn );
        }
        iter++;
    }
    */
/*
    a2dSymbol* sym = GetAppearance<a2dSymbol>();
    a2dGui* gui = GetAppearance<a2dGui>();
    a2dParameters* pars = GetAppearance<a2dParameters>();
    a2dDiagram* gram = GetAppearance<a2dDiagram>();
*/
    m_changedInternalAccesstime = wxDateTime::Now(); 
}

a2dCameleon::~a2dCameleon()
{
}

a2dObject* a2dCameleon::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCameleon( *this, options, refs );
}

void a2dCameleon::OnDoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}

void a2dCameleon::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}

void a2dCameleon::OnUndoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}

void a2dCameleon::OnChangeDrawings( a2dDrawingEvent& event )
{
    //document contaning a2dCameleon objects may be dynamically connected.
    event.Skip();
}

a2dCameleon* a2dCameleon::HasInLocalLibrary( const wxString& name )
{
    if ( GetCameleonRoot() )
	{
		a2dCanvasObjectList::iterator iter = GetCameleonRoot()->CreateChildObjectList()->begin();
		while ( iter != GetCameleonRoot()->CreateChildObjectList()->end() )
		{
			a2dCameleon* cam = wxDynamicCast( (*iter).Get(), a2dCameleon );
			if ( cam && cam->GetName() == name )
			{
				return cam;
			}
			iter++;
		}
	}
	return NULL;
}

void a2dCameleon::AddToRoot( bool autoPlace ) 
{
    if ( GetCameleonRoot() )
	{
		a2dCanvasObjectList::iterator iter = GetCameleonRoot()->CreateChildObjectList()->begin();
		while ( iter != GetCameleonRoot()->CreateChildObjectList()->end() )
		{
			a2dCameleon* cam = wxDynamicCast( (*iter).Get(), a2dCameleon );
			if ( cam == this )
			{
				return; //nothing to do
			}
			iter++;
		}

		if ( autoPlace  )
		{
			a2dPoint2D pos = a2dCameleon::GetSuitablePoint();
			SetPosXyPoint( pos );
		}
		if ( GetCameleonRoot() )
			GetCameleonRoot()->Append( this );
	}
}

a2dPoint2D a2dCameleon::GetSuitablePoint() 
{ 
    if ( GetCameleonRoot() )
    {
        //calculate/get boundingbox and place at upper left.
        GetCameleonRoot()->Update( a2dCanvasObject::updatemask_force );
        a2dBoundingBox bbox = GetCameleonRoot()->GetBbox();
		if ( m_nextLine )
			return a2dPoint2D( bbox.GetMinX(), bbox.GetMaxY() + ms_dy );     
		else
			return a2dPoint2D( bbox.GetMinX(), bbox.GetMinY() + ms_dy );     
    }
    return a2dPoint2D( 0,0 );     
}

a2dPoint2D a2dCameleon::GetSuitablePointForNewAppearance() const 
{ 
    a2dBoundingBox bbox = m_appearances->GetRootObject()->GetBbox();
	if ( m_nextLine )
		return a2dPoint2D( bbox.GetMinX(), bbox.GetMaxY() + ms_dy );     
	else
	    return a2dPoint2D( bbox.GetMinX(), bbox.GetMinY() + ms_dy );     
}

a2dAppear* a2dCameleon::GetAppearanceByName( const wxString& name ) const
{
    a2dCanvasObjectList::iterator iter = m_appearances->GetRootObject()->GetChildObjectList()->begin();
    while ( iter != m_appearances->GetRootObject()->GetChildObjectList()->end() )
    {
        a2dAppear* appear = wxDynamicCast( ( *iter ).Get(), a2dAppear );
        if ( appear && appear->GetName() == name )
            return appear;
        iter++;
    }
    return NULL;
}

a2dAppear* a2dCameleon::GetAppearanceByClassName( const wxString& appearranceClassName ) const
{
    a2dCanvasObjectList::iterator iter = m_appearances->GetRootObject()->GetChildObjectList()->begin();
    while ( iter != m_appearances->GetRootObject()->GetChildObjectList()->end() )
    {
        a2dAppear* appear = wxDynamicCast( ( *iter ).Get(), a2dAppear );
        if ( appear && appear->GetClassInfo()->GetClassName() == appearranceClassName )
            return appear;
        iter++;
    }
    return NULL;
}

a2dDiagram* a2dCameleon::GetDiagram( bool autoCreate )
{ 
    a2dDiagram* ret = wxStaticCastNull( GetAppearanceByClassName( "a2dDiagram" ), a2dDiagram );
    if ( !ret && autoCreate )
    {
        ret = new a2dDiagram( this );
        ret->SetPosXyPoint( GetSuitablePointForNewAppearance() );
        m_appearances->GetRootObject()->Append( ret );
    }
    return ret;
}

a2dSymbol* a2dCameleon::GetSymbol( bool autoCreate )
{ 
    a2dSymbol* ret = wxStaticCastNull( GetAppearanceByClassName( "a2dSymbol" ), a2dSymbol );
    if ( !ret && autoCreate )
    {
        ret = new a2dSymbol( this );
        ret->SetPosXyPoint( GetSuitablePointForNewAppearance() );
        m_appearances->GetRootObject()->Append( ret );
    }
    return ret;
}

a2dBuildIn* a2dCameleon::GetBuildIn( bool autoCreate )
{ 
    a2dBuildIn* ret = wxStaticCastNull( GetAppearanceByClassName( "a2dBuildIn" ), a2dBuildIn );
    if ( !ret && autoCreate )
    {
        ret = new a2dBuildIn( this );
        ret->SetPosXyPoint( GetSuitablePointForNewAppearance() );
        m_appearances->GetRootObject()->Append( ret );
    }
    return ret;
}

a2dGui* a2dCameleon::GetGui( bool autoCreate )
{ 
    a2dGui* ret = wxStaticCast( GetAppearanceByClassName( "a2dGui" ), a2dGui );
    if ( !ret && autoCreate )
    {
        ret = new a2dGui( this );
        ret->SetPosXyPoint( GetSuitablePointForNewAppearance() );
        m_appearances->GetRootObject()->Append( ret );
    }
    return ret;
}

void a2dCameleon::AddAppearance( a2dAppear* appearance )
{
    m_appearances->GetRootObject()->Append( appearance );
	appearance->SetCameleon( this );
}

a2dCanvasObject* a2dCameleon::PushInto( a2dCanvasObject* parent )
{
    a2dCameleonInst* inst = wxDynamicCast( parent, a2dCameleonInst );
    if ( inst )
    {
        //thinking in subcircuits, the diagram is the one to show, one can change the view to the symbol
        if ( inst->GetAppearance() == GetDiagram() )
        {
            return GetDiagram()->PushInto( parent );
        }
        if ( inst->GetAppearance() == GetSymbol() )
        {
            if ( GetDiagram() )
                return GetDiagram()->PushInto( parent );
            return GetSymbol()->PushInto( parent );
        }
    }
    return m_appearances->PushInto( parent );
}

void a2dCameleon::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    a2dCanvasObject::DoWalker( parent, handler );

    if ( handler.GetSkipNotRenderedInDrawing() )
    {
        int b = 1;
        //cameleon has subdrawings, not part of rendering the drawing containing this objects.
    }
    else if ( wxDynamicCast( &handler, a2dWalker_SetCheck ) )
        m_appearances->Walker( parent, handler );
    else
        m_appearances->Walker( parent, handler );
}

a2dBoundingBox a2dCameleon::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    double grid = m_habitat->GetObjectGridSize();
    a2dBoundingBox bbox;
    bbox.Expand( 0, 0 );
    bbox.Expand( 30*grid, m_nextLine ? 10*grid: -10*grid );
    return bbox;
}

#define BBOX2XYWH(bbox) (bbox).GetMinX(), (bbox).GetMinY(), (bbox).GetWidth(), (bbox).GetHeight()

void a2dCameleon::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    double grid = m_habitat->GetObjectGridSize();
    a2dDrawer2D *d(ic.GetDrawer2D());

    // For debugging
    //d->SetDrawerFill(*a2dBLACK_FILL);
    //d->DrawRoundedRectangle(BBOX2XYWH(GetUnTransformedBbox() ), 0);

	int align = m_nextLine ? wxMINX|wxMAXY: wxMINX|wxMINY;

    d->DrawRoundedRectangle( 0, 0, 30*grid, m_nextLine ? 10*grid: -10*grid , 1*grid );
    d->SetFont( a2dCanvasModule::GetFontMedBold() );
    d->DrawText( " cam : " + GetName(), 0, m_nextLine ? 2*grid: -2*grid, align, false);
    d->SetFont( a2dCanvasModule::GetFontSmall() );

    double y = m_nextLine ? 3.1*grid: -3.1*grid;
    a2dCanvasObjectList::iterator iter = m_appearances->GetRootObject()->GetChildObjectList()->begin();
    while ( iter != m_appearances->GetRootObject()->GetChildObjectList()->end() )
    {
        a2dCanvasObjectPtr obj = *iter;
        a2dAppear* appearance = wxDynamicCast( obj.Get(), a2dAppear );
        //if object is not saved yet, we need to save it here.
        if ( appearance && !appearance->GetRelease() )
        {
            wxString classn = appearance->GetClassInfo()->GetClassName();
            classn = "type: " + classn;
            d->DrawText( classn, 2*grid, y, align, false);
            d->SetFont( a2dCanvasModule::GetFontSmall() );
            y += m_nextLine ? 1.2*grid: -1.2*grid;
            d->DrawText( "name: " + appearance->GetName(), 2*grid, y, align, false);
            y += m_nextLine ? 1.2*grid: -1.2*grid;
        }
        iter++;
    }

}

bool a2dCameleon::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double grid = m_habitat->GetObjectGridSize();
    double margin = ic.GetTransformedHitMargin();

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, 0, 0, 30*grid, m_nextLine ? 10*grid: -10*grid, ic.GetWorldStrokeExtend() + margin );

    return hitEvent.m_how.IsHit();
}

#if wxART2D_USE_CVGIO
void a2dCameleon::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_appearances )
            out.WriteAttribute( wxT( "appearances" ), m_appearances->GetId() );
        out.WriteAttribute( wxT( "isMultiRef" ), m_isMultiRef );
        out.WriteAttribute( wxT( "isTemplate" ), m_isTemplate );
        out.WriteAttribute( wxT( "drawingId" ), m_drawingId.m_name );
    }
    else
    {
        if ( m_appearances && !m_appearances->GetCheck() )
        {
            a2dObjectPtr multiRef = m_appearances.Get();

            out.WriteStartElement( wxT( "appearances" ) );
            m_appearances->Save( this, out, towrite, NULL );
            out.WriteEndElement();
        }
    }

}

void a2dCameleon::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( parser.HasAttribute( wxT( "appearances" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_appearances, parser.GetAttributeValue( wxT( "appearances" ) ) );
        }
		m_isMultiRef = parser.GetAttributeValueBool( wxT( "isMultiRef" ), true );
		m_isTemplate = parser.GetAttributeValueBool( wxT( "isTemplate" ), false );
		wxString drawingIdName = parser.GetAttributeValue( wxT( "drawingId" ), a2dDrawingId_normal().m_name );
        m_drawingId = a2dDrawingId::GetItemByName( drawingIdName );
        m_name = parser.GetAttributeValue( wxT( "name" ), m_name );
    }
    else
    {
		if ( m_name == "" )
			m_name = a2dObject::GetName();

        if ( parser.GetTagName() != wxT( "appearances" ) )
            return;

        parser.Require( START_TAG, wxT( "appearances" ) );
        parser.Next();

        m_appearances = (a2dDrawing*) parser.LoadOneObject( this );

        a2dCanvasObjectList::iterator iter = m_appearances->GetRootObject()->GetChildObjectList()->begin();
        while ( iter != m_appearances->GetRootObject()->GetChildObjectList()->end() )
        {
            a2dCanvasObjectPtr obj = *iter;
            a2dAppear* appearance = wxDynamicCast( obj.Get(), a2dAppear );
            //if object is not saved yet, we need to save it here.
            if ( appearance )
            {
                appearance->SetCameleon( this );
            }
            iter++;
        }


        parser.Require( END_TAG, wxT( "appearances" ) );
        parser.Next();
    }

    // this does not add the same cameleon twice, so cameleon position is as in file.
    // Autoplace, if already read the first write from file, position is orrect, else autoplace
    AddToRoot( true );
}
#endif //wxART2D_USE_CVGIO

OVERLAP a2dCameleon::GetClipStatusData( a2dAppear* appearance, a2dIterC& ic, OVERLAP clipparent ) const
{
    if ( appearance )
    {
        if ( wxDynamicCast( appearance, a2dSymbol ) )
        {
            a2dSymbol* draw = wxDynamicCast( appearance, a2dSymbol );
            return draw->GetDrawing()->GetRootObject()->GetClipStatus( ic, clipparent );
        }
        else if ( wxDynamicCast( appearance, a2dDiagram ) )
        {
            a2dDiagram* draw = wxDynamicCast( appearance, a2dDiagram );
            return draw->GetDrawing()->GetRootObject()->GetClipStatus( ic, clipparent );
        }
    }
    return _OUT;
}

a2dBoundingBox a2dCameleon::GetUnTransformedBboxData( a2dAppear* appearance, a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    if ( appearance )
    {
        if ( wxDynamicCast( appearance, a2dSymbol ) )
        {
            a2dSymbol* draw = wxDynamicCast( appearance, a2dSymbol );
            bbox.Expand( draw->GetBbox() );
        }
        else if ( wxDynamicCast( appearance, a2dDiagram ) )
        {
            a2dDiagram* draw = wxDynamicCast( appearance, a2dDiagram );
            bbox.Expand( draw->GetBbox() );
        }
    }
    else
        bbox.Expand( 0, 0 );
    return bbox;
}

bool a2dCameleon::UpdateData(  a2dAppear* appearance, UpdateMode mode )
{
    bool calc = false;
    if ( appearance )
    {
        if ( wxDynamicCast( appearance, a2dSymbol ) )
        {
            a2dSymbol* draw = wxDynamicCast( appearance, a2dSymbol );
            if ( ! draw->GetDrawing()->GetUpdatesPending() )
                return false;

            calc = draw->GetDrawing()->GetRootObject()->Update( mode );
            //if something there in the drawing was pending, it is now updates (outside idle time of drawing itself ).
            draw->GetDrawing()->SetUpdatesPending( false );
        }
        else if ( wxDynamicCast( appearance, a2dDiagram ) )
        {
            a2dDiagram* draw = wxDynamicCast( appearance, a2dDiagram );
            if ( ! draw->GetDrawing()->GetUpdatesPending() )
                return false;

            calc = draw->GetDrawing()->GetRootObject()->Update( mode );
            //if something there in the drawing was pending, it is now updates (outside idle time of drawing itself ).
            draw->GetDrawing()->SetUpdatesPending( false );
        }
    }

    if ( !m_dataBbox.GetValid() || calc )
    {
        m_dataBbox = GetUnTransformedBboxData( appearance );
        m_dataBbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

void a2dCameleon::RenderData( a2dAppear* appearance, a2dIterC& ic, OVERLAP clipparent )
{
    // For debugging
    //ic.GetDrawer2D()->SetDrawerFill(*a2dWHITE_FILL);
    //ic.GetDrawer2D()->DrawRoundedRectangle(BBOX2XYWH( DoGetUnTransformedBbox() ), 0);

    if ( !ic.GetRenderChildDerived() )
        return;

    OVERLAP childclip = _IN;
    if ( clipparent != _IN )
    {
        if ( wxDynamicCast( appearance, a2dSymbol ) )
        {
            a2dSymbol* draw = wxDynamicCast( appearance, a2dSymbol );
            childclip = draw->GetDrawing()->GetRootObject()->GetClipStatus( ic, clipparent );
        }
        else if ( wxDynamicCast( appearance, a2dDiagram ) )
        {
            a2dDiagram* draw = wxDynamicCast( appearance, a2dDiagram );
            childclip = draw->GetDrawing()->GetRootObject()->GetClipStatus( ic, clipparent );
        }
    }

    if ( childclip != _OUT ) //if a child is _OUT, no need to render it.
    {
        bool skip = a2dPin::GetDoRender();
        if ( skip )
        {
            a2dPin::SetDoRender( false );
            a2dPort::SetDoRender( false );
            a2dOrigin::SetDoRender( false );
        }
        if ( wxDynamicCast( appearance, a2dSymbol ) )
        {
            a2dSymbol* draw = wxDynamicCast( appearance, a2dSymbol );
            draw->GetDrawing()->GetRootObject()->Render( ic, childclip );
        }
        else if ( wxDynamicCast( appearance, a2dDiagram ) )
        {
            a2dDiagram* draw = wxDynamicCast( appearance, a2dDiagram );
            draw->GetDrawing()->GetRootObject()->Render( ic, childclip );
        }
        if ( skip )
        {
            a2dPin::SetDoRender( true );
            a2dPort::SetDoRender( true );
            a2dOrigin::SetDoRender( true );
            skip = false;
        }      
    }
}

bool a2dCameleon::IsHitWorldData( a2dAppear* appearance, a2dIterC& ic, a2dHitEvent& hitEvent )
{
    if ( appearance )
    {
        if ( wxDynamicCast( appearance, a2dSymbol ) )
        {
            a2dSymbol* draw = wxDynamicCast( appearance, a2dSymbol );
            return draw->GetDrawing()->GetRootObject()->IsHitWorld( ic, hitEvent ) != 0;
        }
        else if ( wxDynamicCast( appearance, a2dDiagram ) )
        {
            a2dDiagram* draw = wxDynamicCast( appearance, a2dDiagram );
            return draw->GetDrawing()->GetRootObject()->IsHitWorld( ic, hitEvent ) != 0;
        }
    }

    return false;
}

//----------------------------------------------------------------------------
// a2dVisibleParameter
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE( a2dVisibleParameter, a2dText )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dVisibleParameter::OnMouseEvent )
    EVT_CHAR( a2dVisibleParameter::OnChar )
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS( a2dVisibleParameter, a2dText );

void a2dVisibleParameter::OnPropertyChanged( a2dComEvent& event )
{
    if ( event.GetPropertyId() == m_propId )
    {
        SetPending( true );
    }
}

a2dVisibleParameter::a2dVisibleParameter()
    : a2dText()
{
    m_flags.m_visible = true;
    m_flags.m_prerenderaschild = false;
    m_flags.m_subEditAsChild = true;
    m_propId = NULL;
    m_showname = true;
    m_parent = NULL;
}

a2dVisibleParameter::a2dVisibleParameter( a2dHasParameters* parent, a2dPropertyIdPtr propertyId, double x, double y, double angle )
    : a2dText( wxT( "" ), x, y, *a2dDEFAULT_CANVASFONT, angle )
{
    m_flags.m_visible = true;
    m_flags.m_prerenderaschild = false;
    m_flags.m_subEditAsChild = true;
    m_parent = parent;
    m_propId = propertyId;

    Connect( wxID_ANY, wxID_ANY, a2dEVT_COM_EVENT, wxObjectEventFunction( &a2dVisibleParameter::OnPropertyChanged ), 0, this );
    m_showname = true;
}

a2dVisibleParameter::a2dVisibleParameter( a2dHasParameters* parent, a2dPropertyIdPtr propertyId, double x, double y, bool visible, const a2dFont& font, double angle )
    : a2dText( wxT( "" ), x, y, font, angle )
{
    m_flags.m_visible = visible;
    m_flags.m_prerenderaschild = false;
    m_flags.m_subEditAsChild = true;
    m_parent = parent;
    m_propId = propertyId;
    Connect( wxID_ANY, wxID_ANY, a2dEVT_COM_EVENT, wxObjectEventFunction( &a2dVisibleParameter::OnPropertyChanged ), 0, this );
    m_showname = true;
}

a2dVisibleParameter::~a2dVisibleParameter()
{
    Disconnect( wxID_ANY, wxID_ANY, a2dEVT_COM_EVENT, wxObjectEventFunction( &a2dVisibleParameter::OnPropertyChanged ), 0, this );
}

a2dVisibleParameter::a2dVisibleParameter( const a2dVisibleParameter& other, CloneOptions options, a2dRefMap* refs )
    : a2dText( other, options, refs )
{
    m_propId = other.m_propId;
    m_parent = other.m_parent;
    m_showname = other.m_showname;
}

void a2dVisibleParameter::ShowName( bool show )
{
    m_showname = show;
    SetPending( true );
}

void a2dVisibleParameter::SetParent( a2dHasParameters* parent )
{
    m_parent = parent;
    SetPending( true );
}

a2dObject* a2dVisibleParameter::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dVisibleParameter( *this, options, refs );
};

void a2dVisibleParameter::OnMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    event.Skip();
}

void a2dVisibleParameter::DoEndEdit()
{
    a2dNamedProperty*  property = m_propId->CreatePropertyFromString( m_text.Mid( m_firsteditable ) );
    //m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( m_parent, property ) );

    a2dText::DoEndEdit();
}

void a2dVisibleParameter::OnChar( wxKeyEvent& event )
{
    wxString text = m_text;

    a2dText::OnChar( event );
    if ( m_flags.m_editingCopy )
    {
        //m_propId->ValidateString( m_text )
    }
}

bool a2dVisibleParameter::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        if ( m_showname )
            m_text = m_propId->GetName() + wxT( " = " );
        else
            m_text = wxEmptyString;
        m_firsteditable = m_text.Length();
        if ( m_caret < m_firsteditable )
            m_caret = m_firsteditable;
        a2dNamedPropertyPtr property = m_parent->GetParameter( m_propId );
        if ( property )
            m_text = m_text + property->StringValueRepresentation();
        m_utbbox_changed = true;
    }

    return a2dText::DoUpdate( mode, childbox, clipbox, propbox );
}

#if wxART2D_USE_CVGIO
void a2dVisibleParameter::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dText::DoSave( parent, out, xmlparts, towrite );

    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "showname" ), m_showname, true );
        out.WriteAttribute( wxT( "propertyId" ), m_propId->GetName() );
    }
    else
    {

    }
}

void a2dVisibleParameter::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dText::DoLoad( parent, parser, xmlparts );

    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString resolveKey;
        if ( parser.HasAttribute( wxT( "propertyId" ) ) )
        {
            m_propId = wxStaticCast( parent, a2dObject )->HasPropertyId( parser.GetAttributeValue( wxT( "propertyId" ) ) );
        }
        m_showname = parser.GetAttributeValueBool( wxT( "showname" ) );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO


//----------------------------------------------------------------------------
// a2dCameleonSymbolicRef
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dCameleonSymbolicRef, a2dHasParameters )

bool a2dCameleonSymbolicRef::m_nextLine = false;

a2dCameleonSymbolicRef::a2dCameleonSymbolicRef( double x, double y, a2dCameleon* cameleon )
    : a2dCanvasObject( x, y )
{
    m_appearance = NULL;
    m_cameleon = cameleon;
}

a2dCameleonSymbolicRef::a2dCameleonSymbolicRef( double x, double y, a2dSymbol* symbol )
    : a2dCanvasObject( x, y )
{
    m_appearance = symbol;
    m_cameleon = symbol->GetCameleon();
    wxASSERT_MSG( m_cameleon, wxT( "a2dCamelon not set in a2dSymbol" ) );
}

a2dCameleonSymbolicRef::a2dCameleonSymbolicRef( double x, double y, a2dDiagram* diagram )
    : a2dCanvasObject( x, y )
{
    m_appearance = diagram;
    m_cameleon = diagram->GetCameleon();
    wxASSERT_MSG( m_cameleon, wxT( "a2dCamelon not set in a2dDiagram" ) );
}

a2dCameleonSymbolicRef::a2dCameleonSymbolicRef( double x, double y, const wxString& appearanceName )
    : a2dCanvasObject( x, y )
{
    m_appearanceName = appearanceName;
    m_appearance = 0;
}

a2dCameleonSymbolicRef::~a2dCameleonSymbolicRef()
{
}

a2dCameleonSymbolicRef::a2dCameleonSymbolicRef( const a2dCameleonSymbolicRef& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_appearance = other.m_appearance;
    m_cameleon = other.m_cameleon;
}

a2dObject* a2dCameleonSymbolicRef::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    a2dCameleonSymbolicRef* a = new a2dCameleonSymbolicRef( *this, options, refs );
    return a;
}

void a2dCameleonSymbolicRef::SetAppearanceName( const wxString& appearanceName ) 
{ 
    m_appearanceName = appearanceName; 
    m_appearance = 0;
}

a2dCameleon* a2dCameleonSymbolicRef::GetCameleon()
{
    wxASSERT_MSG( m_cameleon == m_appearance->GetCameleon(), wxT( "a2dCamelon not same in m_appearance" ) );

    if ( m_cameleon )
        return m_cameleon;
    return NULL;
}

a2dBoundingBox a2dCameleonSymbolicRef::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    double grid = GetHabitat()->GetObjectGridSize();
    a2dBoundingBox bbox;
    bbox.Expand( 0, 0 );
    bbox.Expand( 30*grid, -3*grid );
    return bbox;
}

bool a2dCameleonSymbolicRef::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( !m_bbox.GetValid() || !m_appearance )
    {
        calc = a2dCanvasObject::DoUpdate( mode, childbox, clipbox, propbox );
        //m_appearance = m_cameleon->GetAppearanceByName( m_appearanceName );
    }
	return calc;
}

bool a2dCameleonSymbolicRef::LinkReference( a2dObject* linkto )
{
    if( !linkto )
        return false;

    if ( wxDynamicCast( linkto, a2dCameleon ) )
    {
        a2dCameleon* cam = wxStaticCast( linkto, a2dCameleon );
        m_cameleon = cam;
        wxASSERT_MSG( m_cameleon, wxT( "a2dCamelon not set" ) );
        if ( !m_appearanceName.IsEmpty() )
        {
            m_appearance = cam->GetAppearanceByName( m_appearanceName );
            if ( m_appearance )
                m_appearance->SetCheck( true );
            else
            {
                wxString error;
                error.Printf( wxT( "Appearance with name: %s not found in a2dCameleonSymbolicRef" ), m_appearanceName.c_str() );
                a2dGeneralGlobals->ReportErrorF( a2dError_LinkRef, error );
            }
        }
    }
    else if  ( wxDynamicCast( linkto, a2dAppear ) )
    {
        m_appearance = wxStaticCast( linkto, a2dAppear );
        m_appearance->SetCheck( true );
        m_cameleon = m_appearance->GetCameleon();;
    }
    else
    {
        wxString name = linkto->GetName();
        wxString error;
        error.Printf( wxT( "cannot Link To: %s with name %s" ), linkto->GetClassInfo()->GetClassName(), name.c_str() );
        a2dGeneralGlobals->ReportErrorF( a2dError_LinkRef, error );
    }
    return true;
}

a2dCanvasObject* a2dCameleonSymbolicRef::PushInto( a2dCanvasObject* parent )
{
    if ( m_appearance )
    {
        if ( wxDynamicCast( m_appearance.Get(), a2dSymbol ) ) 
        {
            a2dSymbol* draw = wxDynamicCast( m_appearance.Get(), a2dSymbol );
            return draw->GetDrawing()->GetRootObject();
        }
        else if ( wxDynamicCast( m_appearance.Get(), a2dDiagram ) )
        {
            a2dDiagram* draw = wxDynamicCast( m_appearance.Get(), a2dDiagram );
            return draw->GetDrawing()->GetRootObject();
        }
    }
    else if ( m_cameleon )
        return m_cameleon->GetAppearances()->GetRootObject();

    return this;
}

void a2dCameleonSymbolicRef::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    if ( m_appearance.Get() && m_cameleon )
        m_cameleon->Walker( this, handler );

    a2dCanvasObject::DoWalker( parent, handler );
}

void a2dCameleonSymbolicRef::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    double grid = GetHabitat()->GetObjectGridSize();
    a2dDrawer2D *d(ic.GetDrawer2D());
    
	int align = m_nextLine ? wxMINX|wxMAXY: wxMINX|wxMINY;

    d->DrawRoundedRectangle( 0, 0, 30*grid, -3*grid, 0.2*grid);
    d->SetFont( a2dCanvasModule::GetFontMedBold() );
    d->DrawText( " cam : " + m_cameleon->GetName(), 0, -1*grid, align, false);
    d->SetFont( a2dCanvasModule::GetFontSmall() );
    if ( !m_appearance )
        return;
    wxString classn = m_appearance->GetClassInfo()->GetClassName();
    classn = " type: " + classn;
    d->DrawText( classn, 0, -2.1*grid, align, false);
    d->SetFont( a2dCanvasModule::GetFontSmall() );
    d->DrawText( " name: " + m_appearance->GetName(), 0, -3.2*grid, align, false);
}

#if wxART2D_USE_CVGIO
void a2dCameleonSymbolicRef::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_appearance )
		{
            out.WriteAttribute( wxT( "cameleon" ), m_cameleon->GetId() );
            out.WriteAttribute( wxT( "appearance" ), m_appearance->GetId() );
            out.WriteAttribute( wxT( "appearanceClassName" ), m_appearance->GetClassInfo()->GetClassName() );
            out.WriteAttribute( wxT( "appearanceName" ), m_appearanceName );
		}
    }
    else
    {
        //if object is not saved yet, we need to save it here.
        if ( m_cameleon && !m_cameleon->GetCheck() )
        {
            //trick the system to have multiple refs on those objects, else we will not get an id attribute
            a2dCanvasObjectPtr multiRef = m_cameleon.Get();

            out.WriteStartElement( wxT( "cameleon" ) );
            m_cameleon->Save( this, out, towrite );
            out.WriteEndElement();
        }
    }
}

void a2dCameleonSymbolicRef::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_appearanceName = parser.GetAttributeValue( wxT( "appearanceName" ) );
        if ( parser.HasAttribute( wxT( "appearance" ) ) )
        {
            parser.ResolveOrAddLink( this, parser.GetAttributeValue( wxT( "appearance" ) ) );
            //parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_appearance, parser.GetAttributeValue( wxT( "appearance" ) ) );
        }
        if ( parser.HasAttribute( wxT( "cameleon" ) ) )
        {
            parser.ResolveOrAddLink( this, parser.GetAttributeValue( wxT( "cameleon" ) ) );
            //parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_cameleon, parser.GetAttributeValue( wxT( "cameleon" ) ) );
        }
    }
    else
    {
        if ( parser.GetTagName() != wxT( "cameleon" ) )
            return;

        parser.Require( START_TAG, wxT( "cameleon" ) );
        parser.Next();

        m_cameleon = (a2dCameleon*) parser.LoadOneObject( this );

        parser.Require( END_TAG, wxT( "cameleon" ) );
        parser.Next();
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dCameleonSymbolicRef::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    double grid = GetHabitat()->GetObjectGridSize();
    double margin = ic.GetTransformedHitMargin();

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, 0, 0, 30*grid, -3*grid, ic.GetWorldStrokeExtend() + margin );

    return hitEvent.m_how.IsHit();
}


//----------------------------------------------------------------------------
// a2dCameleonInst
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dCameleonInst, a2dHasParameters )

a2dCameleonInst::a2dCameleonInst( double x, double y, a2dSymbol* symbol )
    : a2dHasParameters( x, y )
{
    m_CameleonUpdateTime.Set( 0.0 ); 	
    //TriggerCameleonUpdateTime();

    m_appearance = symbol;
    m_appearanceName = "a2dSymbol";
    if ( symbol )
    {
        m_cameleon = symbol->GetCameleon();
        wxASSERT_MSG( m_cameleon, wxT( "a2dCameleon not set in a2dSymbol" ) );
    }
    if ( symbol )
    {
        symbol->TakePortsTo( this );
        symbol->TakeVisibleParameters( this );
        symbol->GetCameleon()->TakeParameters( this );
    }
}

a2dCameleonInst::a2dCameleonInst( double x, double y, a2dDiagram* diagram )
    : a2dHasParameters( x, y )
{
    m_CameleonUpdateTime.Set( 0.0 ); 	
    //TriggerCameleonUpdateTime();
    m_appearance = diagram;
    m_appearanceName = "a2dDiagram";
    if (diagram )
    {
        m_cameleon = diagram->GetCameleon();
        wxASSERT_MSG( m_cameleon, wxT( "a2dCameleon not set in a2dDiagram" ) );
    }
    if ( diagram )
        diagram->TakePortsTo( this );
    //m_diagram->TakeVisibleParameters( this );
    //m_diagram->GetCameleon()->TakeParameters( this );
}

a2dCameleonInst::a2dCameleonInst( double x, double y, const wxString& appearanceName )
    : a2dHasParameters( x, y )
{
    m_CameleonUpdateTime.Set( 0.0 ); 	
    //TriggerCameleonUpdateTime();
    m_appearanceName = appearanceName;
    m_cameleon = 0;
    m_appearance = 0;
}

a2dCameleonInst::~a2dCameleonInst()
{
}

a2dCameleonInst::a2dCameleonInst( const a2dCameleonInst& other, CloneOptions options, a2dRefMap* refs )
    : a2dHasParameters( other, options, refs )
{
    TriggerCameleonUpdateTime();

    if ( !(options & clone_noCameleonRef) && (options & clone_members) && other.m_cameleon )
    {
        //cloned cameleon, and therefore also cloned appearances need to be searched by classname.
        m_cameleon = wxStaticCast( other.m_cameleon->Clone( CloneOptions( options & ~ clone_seteditcopy | clone_sameName ), refs ), a2dCameleon );
        m_appearanceName = other.m_appearanceName;
        a2dAppear* appear = m_cameleon->GetAppearanceByClassName( m_appearanceName );
        a2dDiagram* diagram = wxDynamicCast( appear, a2dDiagram );
        a2dAppear* appearother = other.m_cameleon->GetAppearanceByClassName( m_appearanceName );
        a2dDiagram*	diagramother = wxDynamicCast( appearother, a2dDiagram );

        //diagram->CalculateOffset();
        //diagramother->CalculateOffset();

        if ( diagram )
        {
            m_appearance = appear;
			refs->LinkReferences( true );
			/*
			// find parpins, in order to name port the same in cloned drawing
			a2dCanvasObjectList::iterator iterparInst = CreateChildObjectList()->begin();
			while ( iterparInst != CreateChildObjectList()->end() )
			{
				a2dParPin* pin = wxDynamicCast( ( *iterparInst ).Get(), a2dParPin );
				if ( pin )
				{
					//find the port in the other drawing (cloned a2dParPin still point to that one).
					a2dPort* portInDrawing = HasPort(diagramother->GetDrawing(), pin );
					if ( !portInDrawing )
					{
				        wxASSERT_MSG( m_appearance == appear, wxT( "Port not avaialble in other" ) );
					}
					else
					{
						//assign same name in other to cloned ports
						portInDrawing->SetName( pin->GetPort()->GetName() ); 
					}
				}
				iterparInst++;
			}
			*/
			// now sunc the new named ports to par pins in clone.
            SyncPinsTo( diagram->GetDrawing(), diagram->GetOffset() );
        }
    }
    else 
    {
        m_appearanceName = other.m_appearanceName;
        m_appearance = other.m_appearance;
        m_cameleon = other.m_cameleon;
        a2dAppear* appear = m_cameleon->GetAppearanceByClassName( m_appearanceName );
        a2dDiagram* diagram = wxDynamicCast( appear, a2dDiagram );
        if ( diagram )
        {
            wxASSERT_MSG( m_appearance == appear, wxT( "Appearance with name different from pointer" ) );
            SyncPinsTo( diagram->GetDrawing(), diagram->GetOffset() );
        }
    }
}

a2dObject* a2dCameleonInst::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    a2dCameleonInst* a = new a2dCameleonInst( *this, options, refs );
    return a;
}

void a2dCameleonInst::Assign( const a2dCameleonInst& src )
{
    if (this != &src)
    {
        //a2dHasParameters::Assign(src);

        //take over content of appearance it is pointing to.
        a2dDrawing* drawingsrc = NULL;
        a2dDiagram* diagramsrc = NULL;
        a2dDrawing* drawing = NULL;
        a2dDiagram* diagram = NULL;
        if ( diagram = wxDynamicCast( m_appearance.Get(), a2dDiagram ) )
        {
            drawing = diagram->GetDrawing();

            if ( diagramsrc = wxDynamicCast( src.m_appearance.Get(), a2dDiagram ) )
            {
                drawingsrc = diagramsrc->GetDrawing();
                drawing->AssignContent( *(drawingsrc) );
		        drawing->SetRootObject( drawingsrc->GetRootObject() );
            }
            //the drawing was assigned, but offset not yet.
            diagram->SetOffset( diagramsrc->GetOffset() );
            //above should be correct.
            //diagram->CalculateOffset();
            //diagramsrc->CalculateOffset();
            SyncPinsTo( drawing, diagram->GetOffset(), true );
        }
    }
}


void a2dCameleonInst::SetAppearanceName( const wxString& appearanceName ) 
{ 
    m_appearanceName = appearanceName; 
    m_cameleon = 0;
    m_appearance = 0;
}

void a2dCameleonInst::SetAppearance( a2dAppear* appearance )
{ 
    m_appearance = appearance; 
    m_cameleon = appearance->GetCameleon();
}

a2dCameleon* a2dCameleonInst::GetCameleon()
{
    if ( m_appearance )
        return m_appearance->GetCameleon();
    return NULL;
}

void a2dCameleonInst::Flatten( a2dCanvasObject* parent )
{
    if ( m_appearance )
    {
        a2dDrawing* drawing = NULL;
        if ( wxDynamicCast( m_appearance.Get(), a2dDiagram ) )
        {
            a2dDiagram* diagram = wxDynamicCast( m_appearance.Get(), a2dDiagram );
            drawing = diagram->GetDrawing();
        }
        a2dRefMap refs;
        a2dCanvasObjectList::iterator iterchilds = drawing->GetRootObject()->CreateChildObjectList()->begin();
        while ( iterchilds != drawing->GetRootObject()->CreateChildObjectList()->end() )
        {
            a2dCanvasObject* obj = *iterchilds;
            if ( obj && !obj->GetRelease() )
            {
                a2dCanvasObject* cloned = obj->TClone( CloneOptions( clone_deep & ~ clone_seteditcopy ), &refs );
                double x = GetPosX();
                double y = GetPosY();
                cloned->Translate( x, y );
                parent->Append( cloned );
            }
            iterchilds++;
        }
        refs.LinkReferences();
    }
}

#define BBOX2XYWH(bbox) (bbox).GetMinX(), (bbox).GetMinY(), (bbox).GetWidth(), (bbox).GetHeight()

bool a2dCameleonInst::LinkReference( a2dObject* linkto )
{
    if( !linkto )
        return false;

    if ( m_appearance.Get() )
        return false;

    if ( wxDynamicCast( linkto, a2dCameleon ) )
    {
        a2dCameleon* cam = wxStaticCast( linkto, a2dCameleon );
        m_appearance = cam->GetAppearanceByName( m_appearanceName );
        m_cameleon = cam;
        wxASSERT_MSG( m_cameleon, wxT( "a2dCamelon not set in a2dDiagram" ) );
        if ( m_appearance )
            m_appearance->SetCheck( true );
        else
        {
            wxString error;
            error.Printf( wxT( "Appearance with name: %s not found in a2dCameleonInst" ), m_appearanceName.c_str() );
            a2dGeneralGlobals->ReportErrorF( a2dError_LinkRef, error );
        }
    }
    else if  ( wxDynamicCast( linkto, a2dAppear ) )
    {
        m_appearance = wxStaticCast( linkto, a2dAppear );
        m_appearance->SetCheck( true );
        m_cameleon = m_appearance->GetCameleon();;
    }
    else
    {
        wxString name = linkto->GetName();
        wxString error;
        error.Printf( wxT( "cannot Link To: %s with name %s" ), linkto->GetClassInfo()->GetClassName(), name.c_str() );
        a2dGeneralGlobals->ReportErrorF( a2dError_LinkRef, error );
    }
    m_CameleonUpdateTime.Set( 0.0 ); 	
    //TriggerCameleonUpdateTime();
    return true;
}

void a2dCameleonInst::MakeReferencesUnique()
{
    if ( m_cameleon ) //&& m_cameleon->GetOwnedBy() > 1 )
    {
        m_cameleon = wxStaticCast( m_cameleon->Clone( a2dObject::clone_deep ), a2dCameleon );
    }
    a2dCanvasObject::MakeReferencesUnique();
}

a2dCanvasObject* a2dCameleonInst::PushInto( a2dCanvasObject* parent )
{
    if ( m_appearance && m_appearance->GetCameleon() )
    {   
        return m_appearance->GetCameleon()->PushInto( this );
        //stick to the appearance.
        //return m_appearance->PushInto( parent );
    }
    return this;
}

void a2dCameleonInst::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    wxString handlername = handler.GetClassInfo()->GetClassName();
    if ( m_cameleon.Get() && handlername != wxT("a2dWalker_SetRoot") )
        m_cameleon->Walker( this, handler );

    a2dCanvasObject::DoWalker( parent, handler );
}

a2dBoundingBox a2dCameleonInst::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;
    if ( m_appearance )
    {
        a2dDiagram* diagram = wxDynamicCast( m_appearance.Get(), a2dDiagram );
        if ( diagram )
        {
            bbox.Expand( diagram->GetUnTransformedBboxNoPorts() );
            bbox.MapBbox( diagram->GetOffset() );
        }
    }
    else
        bbox.Expand( 0, 0 );
    return bbox;
}

a2dPort* a2dCameleonInst::HasPort( a2dDrawing* drawing, a2dParPin* parPin )
{
    a2dPort* find = NULL;
    a2dCanvasObjectList::iterator iter = drawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != drawing->GetRootObject()->GetChildObjectList()->end() )
    {
        find = wxDynamicCast( ( *iter ).Get(), a2dPort );
        if ( find )
			if ( find == parPin->GetPort() )
				return find;
        iter++;
    }
    return NULL;
}

a2dParPin* a2dCameleonInst::HasParPinForPort( a2dPort* port )
{
    a2dCanvasObjectList::iterator iterparInst = CreateChildObjectList()->begin();
    while ( iterparInst != CreateChildObjectList()->end() )
    {
        a2dParPin* pin = wxDynamicCast( ( *iterparInst ).Get(), a2dParPin );
        if ( pin->GetPort() == port )
        {
            return pin;
        }
        iterparInst++;
    }
    return NULL;
}

a2dParPin* a2dCameleonInst::HasParPinForPortByName( a2dPort* port )
{
    a2dCanvasObjectList::iterator iterparInst = CreateChildObjectList()->begin();
    while ( iterparInst != CreateChildObjectList()->end() )
    {
        a2dParPin* pin = wxDynamicCast( ( *iterparInst ).Get(), a2dParPin );
        if ( pin->GetPort()->GetName() == port->GetName() )
        {
            return pin;
        }
        iterparInst++;
    }
    return NULL;
}

void a2dCameleonInst::SyncPinsTo( a2dDrawing* drawing, const a2dAffineMatrix& matrix, bool byname )
{
    double dx = matrix.GetValue( 2, 0 );
    double dy = matrix.GetValue( 2, 1 );
	/*debug
    a2dCanvasObjectList::iterator iterparInst1 = CreateChildObjectList()->begin();
    while ( iterparInst1 != CreateChildObjectList()->end() )
    {
        a2dParPin* pin = wxDynamicCast( ( *iterparInst1 ).Get(), a2dParPin );
        if ( pin )
        {
            wxLogDebug( "Name2: %s, %s", pin->GetName(), pin->GetPinClass()->GetName() );
        }
        iterparInst1++;
    }
	*/
    // first make sure all ports in the drawing of the diagram are available here as a2dParPin
    // If not add a new one.
    a2dPort* port = NULL;
    a2dCanvasObjectList::iterator iter = drawing->GetRootObject()->CreateChildObjectList()->begin();
    while ( iter != drawing->GetRootObject()->CreateChildObjectList()->end() )
    {
        port = wxDynamicCast( ( *iter ).Get(), a2dPort );
        if ( port )
        {      
            a2dParPin* parPinfound = NULL;
            if (byname)
                 parPinfound = HasParPinForPortByName( port );
            else
                 parPinfound = HasParPinForPort( port );
            if ( !parPinfound )
            {
                a2dParPin* parPin = NULL;
                if (  port->GetParPinClass() && port->GetParPinClass()->GetParPin() )
                {
                    parPin = (a2dParPin*) ( port->GetParPinClass()->GetParPin()->Clone( clone_deep ) );
                    parPin->SetPort( port );
                    parPin->Set( port->GetPosX() + dx, port->GetPosY() + dy, port->GetAngle(), port->GetName() );
                }
                else
                {
                    parPin = new a2dParPin( this, port, port->GetParPinClass() );
                    parPin->Set( port->GetPosX() + dx, port->GetPosY() + dy, port->GetAngle(), port->GetName() );
                }
                Append( parPin );
            }
            else
                parPinfound->SetPort( port );

        }
        iter++;
    }

    // test all parInst a2dParPins for having an equivalent in the drawing ( symbol/diagram )
    a2dCanvasObjectList::iterator iterparInst = CreateChildObjectList()->begin();
    while ( iterparInst != CreateChildObjectList()->end() )
    {
        a2dParPin* pin = wxDynamicCast( ( *iterparInst ).Get(), a2dParPin );
        if ( pin ) 
        {
            // if there is a pin in the cameleon reference which is not in the drawing
            // we remove it.
            a2dPort* portInDrawing = HasPort(drawing,  pin );
            if ( !portInDrawing )
            {
                // disconnect the pin from others
                a2dPinList::const_iterator iterconp;
                for ( iterconp = pin->GetConnectedPins().begin( ) ; iterconp != pin->GetConnectedPins().end( ) ; iterconp++ )
                {
                    a2dPin* other = *iterconp;
                    if ( !other || other->GetRelease() )
                        continue;
                    pin->Disconnect( other );
                }
                //release it in idle time (redraw oke ).
                pin->SetRelease( true );
                iterparInst++;
                //parInst->CreateChildObjectList()->erase( iterparInst );
            }
            else
            {
                // the pin is in the drawing, now check its position.
                pin->SetPosXY( portInDrawing->GetPosX() + dx, portInDrawing->GetPosY() + dy );
				pin->SetPort( portInDrawing ); 
                iterparInst++;
            }
        }
        else
            iterparInst++;
    }
	/* debug
    iterparInst = CreateChildObjectList()->begin();
    while ( iterparInst != CreateChildObjectList()->end() )
    {
        a2dParPin* pin = wxDynamicCast( ( *iterparInst ).Get(), a2dParPin );
        if ( pin )
        {
            wxLogDebug( "Name: %s, %s", pin->GetName(), pin->GetPinClass()->GetName() );

        }
        iterparInst++;
    }
	*/
}

void a2dCameleonInst::DependencyPending( a2dWalkerIOHandler* handler )
{
    if ( m_CameleonUpdateTime < m_cameleon->GetInternalChangedTime() )
    {
        TriggerCameleonUpdateTime();
        if ( wxDynamicCast( m_appearance.Get(), a2dDiagram ) )
        {
            a2dDiagram* diagram = wxDynamicCast( m_appearance.Get(), a2dDiagram );
            // need a flag or a check for this, else existing connection at this level
            // will get lost.
            // Preserve existing pins, and add others.
            SyncPinsTo( diagram->GetDrawing(), diagram->GetOffset() );
        }
        if ( GetPending() )
            SetPending( true );
    }
    a2dCanvasObject* parent = wxDynamicCast( handler->GetParent(), a2dCanvasObject );
    if ( parent && parent->GetHighLight() && !parent->GetPending() && GetPending() )
    {
        parent->SetPending( true );
    }
}

bool a2dCameleonInst::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( m_appearance )
    {
        calc = m_cameleon->UpdateData( m_appearance, mode );
    }

    if ( !m_bbox.GetValid() || calc )
    {
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

void a2dCameleonInst::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( !m_appearance )
    {
        double x1;
        double y1;
        ic.GetTransform().TransformPoint( 0, 0, x1, y1 );
        int dx = ic.GetDrawer2D()->WorldToDeviceX( x1 );
        int dy = ic.GetDrawer2D()->WorldToDeviceY( y1 );

        ic.GetDrawer2D()->PushIdentityTransform();
        ic.GetDrawer2D()->DrawLine( dx - 3, dy,  dx + 4, dy );
        ic.GetDrawer2D()->DrawLine( dx, dy + 3,  dx, dy - 4 );
        ic.GetDrawer2D()->PopTransform();
        return;
    }

    if ( !ic.GetRenderChildDerived() )
        return;

    // For debugging
    //ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( wxColour( 255, 0, 0 ) ) );
    //ic.GetDrawer2D()->DrawRoundedRectangle(BBOX2XYWH( DoGetUnTransformedBbox() ), 0);

    OVERLAP childclip = _IN;
    if ( clipparent != _IN )
    {
        m_cameleon->GetClipStatusData( m_appearance, ic, clipparent );
    }

    if ( childclip != _OUT ) //if a child is _OUT, no need to render it.
    {
        a2dAffineMatrix offset = m_appearance->GetOffset();
        a2dIterCU cu( ic, offset );
        m_cameleon->RenderData( m_appearance, ic, childclip );
    }
}

bool a2dCameleonInst::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    if ( !m_appearance )
        return false;


    if ( m_appearance )
    {
        a2dAffineMatrix offset = m_appearance->GetOffset();
        a2dIterCU cu( ic, offset );
        return m_cameleon->IsHitWorldData( m_appearance, ic, hitEvent ) != 0;
    }
    return false;
}



#if wxART2D_USE_CVGIO
void a2dCameleonInst::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_cameleon )
            out.WriteAttribute( wxT( "cameleon" ), m_cameleon->GetId() );
        if ( m_appearance )
            out.WriteAttribute( wxT( "appearance" ), m_appearance->GetId() );
    }
    else
    {
        //if object is not saved yet, we need to save it here.
        //Normally it will be save from the top of the document, but if reference is the only one, save it here.
        if ( m_cameleon && !m_cameleon->GetCheck() )// && m_cameleon->GetOwnedBy() == 1 )
        {
            //trick the system to have multiple refs on those objects, else we will not get an id attribute
            a2dCanvasObjectPtr multiRef = m_cameleon.Get();

            out.WriteStartElement( wxT( "cameleon" ) );
            m_cameleon->Save( this, out, towrite );
            out.WriteEndElement();
        }
    }
}

void a2dCameleonInst::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( parser.HasAttribute( wxT( "cameleon" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_cameleon, parser.GetAttributeValue( wxT( "cameleon" ) ) );
        }
        if ( parser.HasAttribute( wxT( "appearance" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_appearance, parser.GetAttributeValue( wxT( "appearance" ) ) );
        }
    }
    else
    {
        if ( parser.GetTagName() != wxT( "cameleon" ) )
            return;

        parser.Require( START_TAG, wxT( "cameleon" ) );
        parser.Next();

        m_cameleon = (a2dCameleon*) parser.LoadOneObject( this );
        if ( m_cameleon )
            m_appearance = m_cameleon->GetAppearance<a2dDiagram>();
		else
			m_cameleon = NULL;

        parser.Require( END_TAG, wxT( "cameleon" ) );
        parser.Next();
    }
    m_CameleonUpdateTime.Set( 0.0 ); 	
}
#endif //wxART2D_USE_CVGIO

bool a2dCameleonInst::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        //to test matrix editing return a2dCanvasObject::DoStartEdit( editmode, editstyle );

        //a2dToolContr* controller = wxStaticCast( PROPID_Controller->GetPropertyValue( this ).Get(), a2dToolContr );
        a2dCameleonInst* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dCameleonInst );
        original->SetSelected( true );

        a2dCommandProcessor* cmd = m_root->GetCommandProcessor();
        a2dCommandGroup* commandgroup = cmd->CommandGroupBegin( wxT( "a2dCameleonInst edit" ) );

		a2dHabitat* habitat = m_root->GetHabitat();
		bool result = habitat->EditCanvasObject( original, this );

        if ( result )
        {
            a2dDiagram* diagram = wxDynamicCast( m_appearance.Get(), a2dDiagram );
            if ( diagram )
                SyncPinsTo( diagram->GetDrawing(), diagram->GetOffset() );

            cmd->Submit( new a2dCommand_CameleonInst( original, this ) );
            cmd->CommandGroupEnd( commandgroup );
        }
        else
        {
            cmd->CommandGroupEnd( commandgroup );
            //group is ended, but group with subcommands submitted, should be undone directly.
            // This trigger a EVT_UNDO event, which is handled in edit tool itself.
            cmd->Undo();
        }
        // we trigger end editing directly, since all editing is finished after closing the dialog.
        original->SetEditing( false );
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// a2dCommand_CreateCameleonMask
//----------------------------------------------------------------------------
const a2dCommandId a2dCommand_CreateCameleonMask::Id( wxT( "Create Cameleon Selected" ) );

bool a2dCommand_CreateCameleonMask::Do( void )
{
    static int nr = 0;
    nr++;
    wxString portNr = wxString::Format("port_%ld", nr);

    static int camnr = 0;
    camnr++;
    wxString camNr = wxString::Format("cam_%ld", camnr);


    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    m_objects = objects->Clone( m_mask, a2dObject::clone_flat, &m_objectsIndex );
    if ( m_objects != wxNullCanvasObjectList )
    {
        //check for connection to non selected, and disconnect them.
        a2dCanvasObjectList result;
        a2dCanvasObjectList::iterator iterp = objects->begin();
        while ( iterp != objects->end() )
        {
            a2dCanvasObject* obj = *iterp;
            if ( obj && obj->CheckMask( m_mask ) )
            {
                forEachIn( a2dCanvasObjectList, obj->GetChildObjectList() )
                {
                    a2dCanvasObject* objchild = *iter;
                    a2dPin* pin = wxDynamicCast( objchild, a2dPin );
                    if ( !pin || pin->GetRelease( ) )
                        continue;

                    a2dPinList::iterator iterpins;
                    for ( iterpins = pin->GetConnectedPins().begin( ) ; iterpins != pin->GetConnectedPins().end( ) ; iterpins++ )
                    {
                        a2dPin* pincother = wxDynamicCast( iterpins->Get(), a2dPin );
                        if ( !pincother || pincother->GetRelease() )
                            continue;

                        if ( pincother && ! pincother->GetParent()->IsSelected() )
                        {
                            pincother->Disconnect( pin ); 
                            a2dPinClass* pclass = pincother->GetPinClass();
                            a2dPort* port = a2dCanvasGlobals->GetHabitat()->GetConnectionGenerator()->GetPortForPinClasses( pin, pincother );
                            port->SetSelected( true );
                            m_parent->Prepend( port );
                            pin->ConnectTo( port->GetPin() );
                            a2dPort::PROPID_wasConnectedTo->SetPropertyToObject( port, pincother );
                        }
                    }
                }
            }
            iterp++;
        }
    
        wxString camName = wxGetTextFromUser( _T( "Give Name For Created Cameleon:" ), _T( "Create Cameleon" ) , camNr );
        m_groupobject = new a2dCameleon( camName, 0, 0, a2dCanvasGlobals->GetHabitat() );
		a2dCameleonEvent eventNewCameleon( m_parent->GetRoot(), m_groupobject );
		ProcessEvent( eventNewCameleon );

        a2dCanvasObject* drawingroot = NULL;
        a2dDiagram* diagram = NULL;
        if ( m_task == CreateSymbol )
        {
            diagram = new a2dSymbol( m_groupobject );
            a2dDrawing* currentDrawing = wxDynamicCast( m_parent->GetRoot(), a2dDrawing );
            if ( currentDrawing )
                diagram->GetDrawing()->SetDrawingId( currentDrawing->GetDrawingId() );
            drawingroot = diagram->GetDrawing()->GetRootObject();
            m_groupobject->AddAppearance( diagram );
        }
        else 
        {
            diagram = new a2dDiagram( m_groupobject );
            a2dDrawing* currentDrawing = wxDynamicCast( m_parent->GetRoot(), a2dDrawing );
            if ( currentDrawing )
                diagram->GetDrawing()->SetDrawingId( currentDrawing->GetDrawingId() );
            drawingroot = diagram->GetDrawing()->GetRootObject();
            m_groupobject->AddAppearance( diagram );
        }

        m_groupobject->SetIgnoreLayer( true );

        a2dCanvasObjectList::iterator iter = objects->begin();
        while ( iter != objects->end() )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && obj->CheckMask( m_mask ) )
            {
                a2dPort* port = wxDynamicCast( obj, a2dPort );
                /*
                a2dPort* pin = wxDynamicCast( obj, a2dPort );
                if ( pin )
                {
                    a2dPinClass* pclass = pin->GetPin()->GetPinClass();
                    obj->Translate( -dx, -dy );
                    diagram->AppendToDrawing( new a2dPort( obj->GetPosX(), obj->GetPosY(), portNr, pclass ) );
                }
                else
                */
                {
                    //translate the objects within the group in opposite direction
                    drawingroot->Append( obj );
                }
                iter = objects->erase( iter );
            }
            else
                iter++;
        }

        //parent boxes don't change
        drawingroot->Update( a2dCanvasObject::updatemask_force );

        //todo some where inside the symbol drawing a2dCanvasGlobals->GetHabitat()->GetObjectGridSize()*10
        double pagex, pagey;
        pagex = pagey = a2dCanvasGlobals->GetHabitat()->GetObjectGridSize()*10;
        a2dSymbol* symbol;
        if ( m_task == CreateSymbolDiagram )
        {
            symbol = new a2dSymbol( m_groupobject );
            diagram->CreateSymbol( symbol->GetDrawing(), pagex, pagey );
            a2dDrawing* currentDrawing = wxDynamicCast( m_parent->GetRoot(), a2dDrawing );
            if ( currentDrawing )
                symbol->GetDrawing()->SetDrawingId( currentDrawing->GetDrawingId() );
            drawingroot = diagram->GetDrawing()->GetRootObject();
            m_groupobject->AddAppearance( symbol );
        }


        diagram->CalculateOffset();
        // Because of above, the a2dCameleonInst needs to be placed at dx,dy in order to not shift in position.

        double dx = diagram->GetOffset().GetValue( 2, 0 );
        double dy = diagram->GetOffset().GetValue( 2, 1 );

        //let the bounding be the position of the diagram in its own drawing
        double dxB, dyB;
        dxB = drawingroot->GetBbox().GetMinX();
        dyB = drawingroot->GetBbox().GetMinY();
        diagram->TranslateTo( pagex-dxB, pagey-dyB );

        if ( !m_name.IsEmpty() )
            m_groupobject->SetName( m_name );

        if ( m_task == CreateSymbol )
        {
            m_camref = new a2dCameleonInst( dx, dy, (a2dSymbol*) diagram ); 
            m_camref->SetName( "ref1" );
            m_camref->SetRoot( m_parent->GetRoot() );
            m_parent->Append( m_camref );
        }
        else if ( m_task == CreateSymbolDiagram )
        {
            m_camref = new a2dCameleonInst( dx, dy, symbol ); 
            m_camref->SetName( "ref1" );
            m_camref->SetRoot( m_parent->GetRoot() );
            m_parent->Append( m_camref );
        }
        else if ( m_task == CreateDiagram )
        {
            m_camref = new a2dCameleonInst( dx, dy, diagram ); 
            m_camref->SetName( "ref1" );
            m_camref->SetRoot( m_parent->GetRoot() );
            m_parent->Append( m_camref );
        }
        else if ( m_task == CreateTemplateDiagram )
        {
            m_groupobject->SetTemplate( true );
            a2dCanvasObjectList* templateObjects = diagram->GetDrawing()->GetRootObject()->GetChildObjectList();
            a2dRefMap refs;
            // Clone all given objects
            a2dCanvasObjectList::iterator iter = templateObjects->begin();
            for( iter = templateObjects->begin(); iter !=  templateObjects->end(); ++iter )
            {
                a2dCanvasObjectPtr original = *iter;
                //clone deep, because the cloned objects end up in the document, the originals not, they or only a template.
                a2dCanvasObject* copy = original->TClone( clone_deep | clone_noCameleonRef | clone_noReference | clone_setoriginal, &refs );
                copy->SetRoot( diagram->GetDrawing() );
                copy->SetSelected( true ); //this is used to find connected wires to new/copied objects.
                m_parent->Append( copy );
        	}
            // if template is a bunch of connected objects, the clone needs to be reconnected.
            refs.LinkReferences( true );
        }


        m_parent->Update( a2dCanvasObject::updatemask_force );

        a2dWalker_SetSpecificFlagsCanvasObjects setflags( m_mask );
        setflags.Start( m_parent, false );
        m_parent->SetPending( true );
    }
    return true;
}

bool a2dCommand_CreateCameleonMask::Undo( void )
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
// a2dCommand_FlattenCameleonMask
//----------------------------------------------------------------------------
const a2dCommandId a2dCommand_FlattenCameleonMask::Id( wxT( "Flatten Cameleon" ) );

bool a2dCommand_FlattenCameleonMask::Do( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    m_objects = objects->Clone( m_mask, a2dObject::clone_flat, &m_objectsIndex );
    if ( m_objects != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, m_objects )
        {
            a2dCameleonInst* group = wxDynamicCast( iter->Get(), a2dCameleonInst );
            if ( group && group->CheckMask( m_mask ) )
            {
                group->Flatten( m_parent );
                group->SetRelease( true );
            }
        }
    }
    return true;
}

bool a2dCommand_FlattenCameleonMask::Undo( void )
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
// a2dCommand_CloneCameleonFromInstMask
//----------------------------------------------------------------------------
const a2dCommandId a2dCommand_CloneCameleonFromInstMask::Id( wxT( "CloneCameleonFromInstMask" ) );

bool a2dCommand_CloneCameleonFromInstMask::Do( void )
{
    static int nr = 1;
    nr++;
    wxString camNr = wxString::Format("ref_%ld", nr);
    m_newobjects = new a2dCanvasObjectList();

    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    m_objects = objects->Clone( m_mask, a2dObject::clone_flat );
    if ( m_objects != wxNullCanvasObjectList )
    {
        a2dCanvasObjectList::iterator iter = m_objects->begin();
        while ( iter != m_objects->end() )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj && obj->CheckMask( m_mask ) )
            {
                a2dCameleonInst* ref = wxDynamicCast( obj, a2dCameleonInst );
                if ( ref )
                {
                    ref->SetSelected( false );
                    double dx, dy;
                    dx = ref->GetBbox().GetWidth()/10;
                    dy = dx;
                    //a2dCameleon* cloned = wxStaticCast( ref->GetCameleon()->Clone( a2dObject::clone_deep ), a2dCameleon );
                    a2dCameleonInst* camref = wxStaticCast( ref->Clone( a2dObject::clone_deep ), a2dCameleonInst );
                    camref->Translate( dx, dy );
                    camref->SetName( camNr );
                    camref->SetSelected( true );
                    a2dCameleon* cam = camref->GetCameleon();
                    m_parent->Append( camref );
                    m_newobjects->push_back( camref );
                }
                iter++;
            }
            else
                iter = m_objects->erase( iter );
        }
        m_parent->Update( a2dCanvasObject::updatemask_force );
    }
    return true;
}

bool a2dCommand_CloneCameleonFromInstMask::Undo( void )
{
    a2dCanvasObjectList* objects = m_parent->GetChildObjectList();
    if ( m_newobjects != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, m_newobjects )
        {
            a2dCanvasObject* obj = *iter;
            obj->SetRelease( true );
        }
    }
    m_objects->clear();
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_CameleonInst
//----------------------------------------------------------------------------

const a2dCommandId a2dCommand_CameleonInst::Id("edit CameleonInst");

a2dCommand_CameleonInst::a2dCommand_CameleonInst( a2dCameleonInst* target, a2dCameleonInst* objectWithChanges )
    :  a2dCommand( true, a2dCommand_CameleonInst::Id )
{
    m_target = target;
    m_objectchanges = objectWithChanges;
}

a2dCommand_CameleonInst::~a2dCommand_CameleonInst( void )
{
}

bool a2dCommand_CameleonInst::Do( void )
{
    bool selectold = m_target->GetSelected();
    bool select2old = m_target->GetSelected2();
    a2dRefMap refs;
    a2dSmrtPtr<a2dCameleonInst> objectoldstate = (a2dCameleonInst *) m_target->Clone( a2dCanvasObject::clone_deep, &refs );
    refs.LinkReferences( true ); //skip connections to other wire pins etc.

    m_target->Assign( *m_objectchanges );
    m_objectchanges = objectoldstate;

    m_target->SetSelected( selectold );
    m_target->SetSelected2( select2old );
    return true;
}

bool a2dCommand_CameleonInst::Undo( void )
{
    Do(); // Do() and Undo() are identical
    return true;
}

//----------------------------------------------------------------------------
// a2dCommand_ReplaceCameleonInst
//----------------------------------------------------------------------------
const a2dCommandId a2dCommand_ReplaceCameleonInst::Id("ReplaceCameleonInst");

a2dCommand_ReplaceCameleonInst::a2dCommand_ReplaceCameleonInst( a2dCameleonInst* camold, a2dCameleonInst* camnew )
    :   a2dCommand( true, a2dCommand_ReplaceCameleonInst::Id )
{
    wxASSERT( camold );
    wxASSERT( camnew );

    m_cam1 = camold;
    m_cam2 = camnew; 

    a2dCanvasObjectList::iterator iterparInst = m_cam2->CreateChildObjectList()->begin();
    while ( iterparInst != m_cam2->CreateChildObjectList()->end() )
    {
        a2dParPin* pin = wxDynamicCast( ( *iterparInst ).Get(), a2dParPin );
        if ( pin )
        {
            wxASSERT( pin->GetPort() );
            wxASSERT( !pin->IsConnected( NULL ) );
        }
        iterparInst++;
    }

}

a2dCommand_ReplaceCameleonInst::~a2dCommand_ReplaceCameleonInst( void )
{
}

bool a2dCommand_ReplaceCameleonInst::Do( void )
{
    a2dPinList connectedcam1;

    //disconnect at all par pins.
    a2dCanvasObjectList::iterator iterparInst = m_cam1->CreateChildObjectList()->begin();
    while ( iterparInst != m_cam1->CreateChildObjectList()->end() )
    {
        a2dParPin* pin = wxDynamicCast( ( *iterparInst ).Get(), a2dParPin );
        if ( pin && !pin->GetRelease() && pin->GetPort() )
        {
            wxLogDebug( "Name: %s", pin->GetPinClass()->GetName() );
            // disconnect the pin from others
            a2dPinList::const_iterator iterconp;
            for ( iterconp = pin->GetConnectedPins().begin( ) ; iterconp != pin->GetConnectedPins().end( ) ; iterconp++ )
            {
                a2dPin* other = *iterconp;
                if ( !other || other->GetRelease() )
                    continue;
                connectedcam1.push_back( other ); //remember them to connect to new Cameleon instance
                GetCanvasCmp()->Submit( new a2dCommand_DisConnectPins( pin, other ) );
            }
        }
        iterparInst++;
    }

    //connect at all par pins of m_cam2 as long as there are pins.
    iterparInst = m_cam2->CreateChildObjectList()->begin();
    while ( iterparInst != m_cam2->CreateChildObjectList()->end() )
    {
        a2dParPin* pin = wxDynamicCast( ( *iterparInst ).Get(), a2dParPin );
        if ( pin && !pin->GetRelease() && pin->GetPort() )
        {
            wxLogDebug( "Name: %s", pin->GetPinClass()->GetName() );

			if ( connectedcam1.size() )
			{
				//connect the pin to others
				a2dPinPtr other = (*connectedcam1.begin()).Get();
				connectedcam1.pop_front();
	            GetCanvasCmp()->Submit( new a2dCommand_ConnectPins( pin, other ) );
			}
			else
				break;
        }
        iterparInst++;
    }

    a2dSmrtPtr<a2dCameleonInst> swap;
    swap = m_cam1;
    m_cam1 = m_cam2;
    m_cam2 = swap;

    return true;
}

bool a2dCommand_ReplaceCameleonInst::Undo( void )
{
    return Do();
}
