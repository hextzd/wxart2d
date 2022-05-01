/*! \file modules/editor/samples/wires/elements.cpp
    \author Erik van der Wal
   \brief First attempts for a wire demo, sorry q&d code at the moment.
    EW: code gets more and more messy, definitely needs cleanup/documentation

    Copyright: 2004 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: elements.cpp,v 1.25 2009/07/24 16:35:00 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/canvas/sttool.h>
#include <wx/canvas/edit.h>
#include "elements.h"
#include "part.h"

IMPLEMENT_DYNAMIC_CLASS( Element, msObject )
IMPLEMENT_DYNAMIC_CLASS( Label, Element )
IMPLEMENT_DYNAMIC_CLASS( Resistor, Element )
IMPLEMENT_DYNAMIC_CLASS( Capacitor, Element )
IMPLEMENT_DYNAMIC_CLASS( Ground, Label )
IMPLEMENT_DYNAMIC_CLASS( Input, Label )
IMPLEMENT_DYNAMIC_CLASS( Output, Label )
//IMPLEMENT_DYNAMIC_CLASS(Text, a2dText )
IMPLEMENT_DYNAMIC_CLASS( Pin, a2dPin )

a2dPinClass* Pin::ElementObject = new a2dPinClass( wxT( "eleObject" ) );
a2dPinClass* Pin::ElementWire = new a2dPinClass( wxT( "eleWire" ) );

//----------------------------------------------------------------------------
// msComponent
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( Element, msObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( Element::OnCanvasObjectMouseEvent )
END_EVENT_TABLE()

a2dPropertyIdString* Element::PROPID_refdes = NULL;
a2dPropertyIdString* Element::PROPID_spice = NULL;
a2dPropertyIdString* Element::PROPID_freeda = NULL;

INITIALIZE_PROPERTIES( Element, a2dCanvasObject )
{
    PROPID_refdes = new a2dPropertyIdString( wxT( "REFDES" ), a2dPropertyId::flag_none, wxT( "CMP1" ) );
    AddPropertyId( PROPID_refdes );
    PROPID_spice = new a2dPropertyIdString( wxT( "SPICE" ), a2dPropertyId::flag_none, wxT( "" ) );
    AddPropertyId( PROPID_spice );
    PROPID_freeda = new a2dPropertyIdString( wxT( "FREEDA" ), a2dPropertyId::flag_none, wxT( "" ) );
    AddPropertyId( PROPID_freeda );

    return true;
}

Element::Element()
    : msObject()
{
}

Element::Element( const Element& other, CloneOptions options, a2dRefMap* refs )
    : msObject( other, options, refs )
{
}

a2dObject* Element::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new Element( *this, options, refs );
};

#if wxART2D_USE_CVGIO

void Element::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    msObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void Element::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    msObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void Element::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
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
            Element* original = wxDynamicCast( PROPID_Original->GetPropertyValue( this ).Get(), Element );;

            a2dHitEvent hitevent = a2dHitEvent( xw, yw, false );
            if ( IsHitWorld( *ic, hitevent ) )
            {
                a2dHitEvent hitinfo( xw, yw, false, a2dCANOBJHITOPTION_LAYERS );
                hitinfo.m_xyRelToChildren = true;
                a2dCanvasObject* text = original->IsHitWorld( *ic, hitinfo );
                if ( wxDynamicCast( text, a2dText ) && text->GetEditable() )
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

bool PointerSorter( const a2dCanvasObjectPtr& x, const a2dCanvasObjectPtr& y )
{
    return x.Get() > y.Get();
}

wxString Element::GetNetName( a2dPin* pin )
{
    // Create a list of all pins connected to pin.
    a2dCanvasObjectList pinlist;
    GetNetPins( &pinlist, pin );
    // Sort list, based on pointer values.
    // This way, we will allways get the same pointer, independant of starting point.
    s_a2dCanvasObjectSorter = &PointerSorter;

    pinlist.sort();

    a2dCanvasObjectList::iterator iter;
    // First check if a pin is part of a label.
    for( iter = pinlist.begin(); iter != pinlist.end(); ++iter )
    {
        a2dPin* pin = wxDynamicCast( ( *iter ).Get(), a2dPin );
        if ( pin && wxDynamicCast( pin->GetParent(), Label ) )
        {
            // return label.
            Label* label = ( Label* ) pin->GetParent();
            return Label::PROPID_label->GetPropertyValue( label );
        }
    }

    // Find a pin as part of an Element
    for( iter = pinlist.begin(); iter != pinlist.end(); ++iter )
    {
        a2dPin* pin = wxDynamicCast( ( *iter ).Get(), a2dPin );
        if ( pin && wxDynamicCast( pin->GetParent(), Element ) )
        {
            // return "net{REFDES}_{PINNAME}".
            Element* elm = ( Element* ) pin->GetParent();
            return wxT( "net" ) + PROPID_refdes->GetPropertyValue( elm ) + wxT( "_" ) + pin->GetName();
        }
    }

    // If pin is part of a floating wire, return pointer to pin as a unique net name.
    if ( pinlist.size() )
    {
        iter = pinlist.begin();
        wxString result;
        result.Printf( wxT( "net%p" ), ( *iter ).Get() );
        return result;
    }

    return wxEmptyString;
}

// Recursively fill a pinlist with all pins connected to a certain pin.
void Element::GetNetPins( a2dCanvasObjectList* pinlist, a2dPin* pin )
{
    wxASSERT( pinlist );

    // Only continue if pin exists and pin is not allready in list
    if ( pin && !pinlist->Find( pin ) )
    {
        pinlist->push_back( pin );
        // If pin is connected, also proces this one.
        a2dPin* connectedpin = pin->IsConnectedTo();
        if ( connectedpin )
            GetNetPins( pinlist, connectedpin );

        // If parent of pin is a wire, proces all pins connected to the wire
        a2dWirePolylineL* wire = wxDynamicCast( pin->GetParent(), a2dWirePolylineL );
        if ( wire )
        {
            for( a2dCanvasObjectList::iterator iter = wire->GetChildObjectList()->begin(); iter != wire->GetChildObjectList()->end(); ++iter )
            {
                GetNetPins( pinlist,  wxDynamicCast( ( *iter ).Get(), a2dPin ) );
            }
        }
    }
}

/*
    Generate netlist lines based on a string.
    lets assume pin 1 is connected to net001 and C is 10pF and OPT=""
    replaces {C} with contents of vairable C, so 10pF
    replaces {var=C} with var=10pF, only if C != "", otherwise just empty string
    replaces [1] with net of pin 1, so net001
    eg:
    C_{REFDES} [1] [2] var={C} {optional=OPT}
    becomes
    C_C1 net001 net002 var=10pF
*/

wxString Element::GetNetlistLine( wxString simulator )
{
    wxString spice;
    const a2dNamedProperty* prop = FindProperty( simulator );
    if ( prop )
        spice = prop->StringValueRepresentation();
    else
        return wxEmptyString;

    // replace property names by value.
    size_t lastchar = spice.Length();
    size_t index = lastchar;
    while ( index > 0 )
    {
        index--;
        // Replace property name by its value
        if ( spice[index] == wxChar( '{' ) )
        {
            size_t idx1 = index;
            size_t idx2 = index;
            while ( spice[idx1] != wxChar( '}' ) && spice[idx1] != wxChar( '=' ) && idx1 < lastchar )
                idx1++;
            wxString propname;
            if ( spice[idx1] == wxChar( '=' ) )
            {
                idx2 = idx1 + 1;
                while ( spice[idx2] != wxChar( '}' ) && idx2 < lastchar )
                    idx2++;
                propname = spice.Mid( idx1 + 1, idx2 - idx1 - 1 );
            }
            else
                propname = spice.Mid( index + 1, idx1 - index - 1 );

            wxString propval;
            const a2dNamedProperty* prop = FindProperty( propname );
            if ( prop )
                propval = prop->StringValueRepresentation();

            if ( idx2 > idx1 )
            {
                if ( !propval.IsEmpty() )
                    spice = spice.Left( index ) + spice.Mid( index + 1, idx1 - index ) + propval + spice.Mid( idx2 + 1 );
                else
                    spice = spice.Left( index ) + spice.Mid( idx2 + 1 );
            }
            else
                spice = spice.Left( index ) + propval + spice.Mid( idx1 + 1 );
            lastchar = index;
        }
        // Replace pin name by its net
        else if ( spice[index] == wxChar( '[' ) )
        {
            size_t idx2 = index;
            while ( spice[idx2] != wxChar( ']' ) && idx2 < lastchar )
                idx2++;
            wxString number = spice.Mid( index + 1, idx2 - index - 1 );

            for( a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin(); iter != GetChildObjectList()->end(); ++iter )
            {
                a2dPin* pin = wxDynamicCast( ( *iter ).Get(), a2dPin );
                if ( pin && pin->GetName() == number )
                    number = GetNetName( pin );
            }

            spice = spice.Left( index ) + number + spice.Mid( idx2 + 1 );
            lastchar = index;
        }
    }
    return spice;
}

void Element::CopyPins( msSymbolPage* symbol )
{
    for( a2dCanvasObjectList::iterator iter = symbol->GetChildObjectList()->begin(); iter != symbol->GetChildObjectList()->end(); ++iter )
    {
        a2dPin* pin = wxDynamicCast( ( *iter ).Get(), a2dPin );
        if ( pin )
        {
            a2dPin* newpin = ( a2dPin* ) pin->Clone( clone_deep );
            newpin->SetParent( this );
            newpin->SetVisible( true );
            Append( newpin );
        }
    }
}

void Element::CopyVisibleProperties( msSymbolPage* symbol )
{
    for( a2dCanvasObjectList::iterator iter = symbol->GetChildObjectList()->begin(); iter != symbol->GetChildObjectList()->end(); ++iter )
    {
        a2dText* text = wxDynamicCast( ( *iter ).Get(), a2dText );
        if ( text && text->GetText().GetChar( 0 ) == wxChar( '@' ) )
        {
            wxString name = text->GetText().Mid( 1 );
            int idx = name.Find( wxChar( '=' ) );
            if ( idx != -1 )
                name = name.Left( idx );
            name = name.Trim( true ).Trim( false );

            if ( !GetStringProperty( name ) )
                SetStringProperty( name, wxEmptyString );

            a2dPropertyId* id = this->HasPropertyId( name );
            wxASSERT( id );

            a2dVisibleProperty* pppvis;
            pppvis = new a2dVisibleProperty( this, id, 0.0, 0.0, 0.0 );

            //! \todo EW: more general copying of attibutes, DoExchangeMembers?
            pppvis->SetTransformMatrix( text->GetTransformMatrix() );
            pppvis->SetAlignment( text->GetAlignment() );
            pppvis->SetFont( text->GetFont() );
            pppvis->SetFill( text->GetFill() );
            pppvis->ShowName( idx != -1 );
            Append( pppvis );
        }
    }
}

Element::Element( msComponent* comp )
{
    wxASSERT( 0 );
    /*
        msObject *locallib = new msObject();
        Append( locallib );
        locallib->SetVisible( false );
        msSymbolPage *smbl = (msSymbolPage*) comp->GetSymbolPage()->Clone( clone_deep );
        locallib->Append( smbl );
        smbl->HidePins();
        smbl->HideVisibleProperties();

        Append( new a2dCanvasObjectReference( 0.0, 0.0, smbl ) );
        MergeProperties( smbl );
        CopyPins( smbl );
        CopyVisibleProperties( smbl );
        */
}

a2dPropertyIdString* Label::PROPID_label = NULL;

INITIALIZE_PROPERTIES( Label, Element )
{
    PROPID_label = new a2dPropertyIdString( wxT( "LABEL" ), a2dPropertyId::flag_none, wxT( "label" ) );
    AddPropertyId( PROPID_label );
    return true;
}

Label::Label()
    : Element()
{
    PROPID_spice->SetPropertyToObject( this, wxT( "" ) );
    PROPID_freeda->SetPropertyToObject( this, wxT( "" ) );
}

Label::Label( wxString label )
    : Element()
{
    PROPID_spice->SetPropertyToObject( this, wxT( "" ) );
    PROPID_freeda->SetPropertyToObject( this, wxT( "" ) );
    PROPID_label->SetPropertyToObject( this, label );
}

Label::Label( const Label& other, CloneOptions options, a2dRefMap* refs )
    : Element( other, options, refs )
{
}

a2dObject* Label::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new Label( *this, options, refs );
};

#if wxART2D_USE_CVGIO
void Label::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    Element::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void Label::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    Element::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

/*
msComponent* CreateResistor(void)
{
    msComponent* comp = new msComponent();
    msSymbolPage* symbolpage = comp->GetSymbolPage();

    // Create properties
    Resistor::PROPID_r->SetPropertyToObject( symbolpage, wxT("50") );
    Element::PROPID_spice->SetPropertyToObject( symbolpage, wxT("R_{REFDES}    [1] [2]    {R}") );
    Element::PROPID_freeda->SetPropertyToObject( symbolpage, wxT("r:{REFDES}    [1] [2]    r={R}") );

    // Create symbol
    Pin* pin = new Pin( symbolpage, wxT("1"), Pin::ElementObject, -12.0, 0.0 );
    symbolpage->Append( pin );
    pin = new Pin( symbolpage, wxT("2"), Pin::ElementObject, 12.0, 0.0 );
    symbolpage->Append( pin );

    symbolpage->Append( new a2dSLine(-8.0, 0.0, -12.0, 0.0) );
    symbolpage->Append( new a2dSLine(8.0, 0.0, 12.0, 0.0) );

    a2dPolygonL* poly = new a2dPolygonL();
    poly->AddPoint( -8.0, -2.0 );
    poly->AddPoint( -8.0, 2.0 );
    poly->AddPoint( 8.0, 2.0 );
    poly->AddPoint( 8.0, -2.0 );
    symbolpage->Append( poly );

    a2dText* text;
    text = new a2dText( wxT("@REFDES"), 0.0, 3.0, a2dFont( 3.0, wxFONTFAMILY_SWISS ) );
    text->SetAlignment( wxMINY );
    text->SetFill( *a2dTRANSPARENT_FILL );
    symbolpage->Append( text );

    text = new a2dText( wxT("@R="), 0.0, -3.0, a2dFont( 3.0, wxFONTFAMILY_SWISS ) );
    text->SetAlignment( wxMAXY );
    text->SetFill( *a2dTRANSPARENT_FILL );
    symbolpage->Append( text );

    return comp;
}

msComponent* CreateCapacitor(void)
{
    msComponent* comp = new msComponent();
    msSymbolPage* symbolpage = comp->GetSymbolPage();

    // Create properties
    Capacitor::PROPID_c->SetPropertyToObject( symbolpage, wxT("1 pF") );
    Element::PROPID_spice->SetPropertyToObject( symbolpage, wxT("C_{REFDES}    [1] [2]    {C}") );
    Element::PROPID_freeda->SetPropertyToObject( symbolpage, wxT("c:{REFDES}    [1] [2]    c={C} {int_g=INT_G} {time_d=TIME_D}") );

    // Create symbol
    Pin* pin = new Pin(symbolpage, wxT("1"), Pin::ElementObject, -8.0, 0.0 );
    symbolpage->Append( pin );
    pin = new Pin(symbolpage, wxT("2"), Pin::ElementObject, 8.0, 0.0 );
    symbolpage->Append( pin );

    symbolpage->Append( new a2dSLine( -1.0, 0.0, -8.0, 0.0 ) );
    symbolpage->Append( new a2dSLine( 1.0, 0.0, 8.0, 0.0 ) );
    symbolpage->Append( new a2dSLine( -1.0, -3.0, -1.0, 3.0 ) );
    symbolpage->Append( new a2dSLine( 1.0, -3.0, 1.0, 3.0 ) );

    a2dText* text;
    text = new a2dText( wxT("@REFDES"), 0.0, 4.0, a2dFont( 3.0, wxFONTFAMILY_SWISS ) );
    text->SetAlignment( wxMINY );
    text->SetFill( *a2dTRANSPARENT_FILL );
    symbolpage->Append( text );

    text = new a2dText( wxT("@C="), 0.0, -4.0, a2dFont( 3.0, wxFONTFAMILY_SWISS ) );
    text->SetAlignment( wxMAXY );
    text->SetFill( *a2dTRANSPARENT_FILL );
    symbolpage->Append( text );

    return comp;
}
*/

a2dPropertyIdString* Resistor::PROPID_r = NULL;

INITIALIZE_PROPERTIES( Resistor, Element )
{
    PROPID_r = new a2dPropertyIdString( wxT( "R" ), a2dPropertyId::flag_none, wxT( "1" )  );
    AddPropertyId( PROPID_r );
    return true;
}

Resistor::Resistor()
    : Element()
{
}

Resistor::Resistor( const Resistor& other, CloneOptions options, a2dRefMap* refs )
    : Element( other, options, refs )
{
}

a2dObject* Resistor::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new Resistor( *this, options, refs );
};

Resistor::Resistor( wxString refdes, wxString val )
{
    /*
        PROPID_refdes->SetPropertyToObject( this, refdes );
        PROPID_r->SetPropertyToObject( this, val );

        msComponent *rescmp = CreateResistor();
        msObject *locallib = new msObject();
        Append( locallib );
        locallib->SetVisible( false );
        msSymbolPage *smbl = (msSymbolPage*) rescmp->GetSymbolPage()->Clone( clone_deep );
        locallib->Append( smbl );
        smbl->HidePins();
        smbl->HideVisibleProperties();

        Append( new a2dCanvasObjectReference( 0.0, 0.0, smbl ) );
        MergeProperties( smbl );
        CopyPins( smbl );
        CopyVisibleProperties( smbl );

        delete rescmp;
    */
}

#if wxART2D_USE_CVGIO

void Resistor::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    Element::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void Resistor::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    Element::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

a2dPropertyIdString* Capacitor::PROPID_c = NULL;

INITIALIZE_PROPERTIES( Capacitor, Element )
{
    PROPID_c = new a2dPropertyIdString( wxT( "C" ), a2dPropertyId::flag_none, wxT( "1nF" ) );
    AddPropertyId( PROPID_c );
    return true;
}

Capacitor::Capacitor()
    : Element()
{
}

Capacitor::Capacitor( const Capacitor& other, CloneOptions options, a2dRefMap* refs )
    : Element( other, options, refs )
{
}

a2dObject* Capacitor::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new Capacitor( *this, options, refs );
};

Capacitor::Capacitor( wxString refdes, wxString val )
{
    /*
        PROPID_refdes->SetPropertyToObject( this, refdes );
        PROPID_c->SetPropertyToObject( this, val );

        msComponent *rescmp = CreateCapacitor();
        msObject *locallib = new msObject();
        Append( locallib );
        locallib->SetVisible( false );
        msSymbolPage *smbl = (msSymbolPage*) rescmp->GetSymbolPage()->Clone( clone_deep );
        locallib->Append( smbl );
        smbl->HidePins();
        smbl->HideVisibleProperties();

        Append( new a2dCanvasObjectReference( 0.0, 0.0, smbl ) );
        MergeProperties( smbl );
        CopyPins( smbl );
        CopyVisibleProperties( smbl );

        delete rescmp;
    */
}




Ground::Ground( const Ground& other, CloneOptions options, a2dRefMap* refs )
    : Label( other, options, refs )
{
}

a2dObject* Ground::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new Ground( *this, options, refs );
};

Ground::Ground()
    : Label( wxT( "0" ) )
{
    Pin* pin = new Pin( this, wxT( "1" ), Pin::ElementObject, 0.0, 4.0 );
    Append( pin );

    Append( new a2dSLine( 0.0, 0.0, 0.0, 4.0 ) );
    Append( new a2dSLine( -4.0, 0.0, 4.0, 0.0 ) );

    a2dText* text;
    text = new a2dText( wxT( "GND" ), 0.0, -1.0, a2dFont( 3.0, wxFONTFAMILY_SWISS ) );
    text->SetAlignment( wxTOP );
    text->SetFill( *a2dTRANSPARENT_FILL );
    Append( text );
}

#if wxART2D_USE_CVGIO
void Ground::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    Label::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void Ground::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    Label::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO




Input::Input( const Input& other, CloneOptions options, a2dRefMap* refs )
    : Label( other, options, refs )
{
}

a2dObject* Input::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new Input( *this, options, refs );
};

Input::Input()
    : Label( wxT( "vin" ) )
{
    Pin* pin = new Pin( this, wxT( "1" ), Pin::ElementObject, 2.0, 0.0 );
    Append( pin );

    a2dPolygonL* poly = new a2dPolygonL();
    poly->AddPoint( 2.0, 0.0 );
    poly->AddPoint( 0.5, 1.5 );
    poly->AddPoint( -2.0, 1.5 );
    poly->AddPoint( -2.0, -1.5 );
    poly->AddPoint( 0.5, -1.5 );
    Append( poly );

    a2dVisibleProperty* pppvis;
    pppvis = new a2dVisibleProperty( this, PROPID_label, -3, 0, true, a2dFont( 3.0, wxFONTFAMILY_SWISS ), 0.0 );
    pppvis->SetAlignment( wxMAXX );
    pppvis->SetFill( *a2dTRANSPARENT_FILL );
    pppvis->ShowName( false );
    Append( pppvis );
}

#if wxART2D_USE_CVGIO
void Input::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    Label::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void Input::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    Label::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO




Output::Output( const Output& other, CloneOptions options, a2dRefMap* refs )
    : Label( other, options, refs )
{
}

a2dObject* Output::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new Output( *this, options, refs );
};

Output::Output()
    : Label( wxT( "vout" ) )
{
    Pin* pin = new Pin( this, wxT( "1" ), Pin::ElementObject, -2.0, 0.0 );
    Append( pin );

    a2dPolygonL* poly = new a2dPolygonL();
    poly->AddPoint( 2.0, 0.0 );
    poly->AddPoint( 0.5, 1.5 );
    poly->AddPoint( -2.0, 1.5 );
    poly->AddPoint( -2.0, -1.5 );
    poly->AddPoint( 0.5, -1.5 );
    Append( poly );

    a2dVisibleProperty* pppvis;
    pppvis = new a2dVisibleProperty( this, PROPID_label, 3, 0, true, a2dFont( 3.0, wxFONTFAMILY_SWISS ), 0.0 );
    pppvis->SetAlignment( wxMINX );
    pppvis->SetFill( *a2dTRANSPARENT_FILL );
    pppvis->ShowName( false );
    Append( pppvis );
}

#if wxART2D_USE_CVGIO
void Output::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    Label::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void Output::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    Label::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO



Pin::Pin()
    : a2dPin()
{
}

Pin::Pin( a2dCanvasObject* parent, const wxString& name, a2dPinClass* pinclass,  double xc, double yc, double angle, int w, int h,  int radius )
    : a2dPin( parent, name, pinclass, xc, yc, angle, w, h, radius )
{
}

Pin::Pin( const a2dPin& other, CloneOptions options, a2dRefMap* refs )
    : a2dPin( other, options, refs )
{
}

Pin::~Pin()
{
}

a2dObject* Pin::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new Pin( *this, options, refs );
};

void Pin::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    a2dPin::DoRender( ic, clipparent );
    return;
    double x1, y1;
    ic.GetTransform().TransformPoint( 0.0, 0.0, x1, y1 );
    int dx = ( int ) ic.GetDrawer2D()->WorldToDeviceX( x1 );
    int dy = ( int ) ic.GetDrawer2D()->WorldToDeviceY( y1 );
    ic.GetDrawer2D()->PushIdentityTransform();

    // Calculate handle size.
    double w2 = m_width / 2 ;
    double h2 = m_height / 2;

    if ( !IsConnectedTo() ) //connected ?
    {
        if ( GetAlgoSkip() && m_parent->HasProperty( a2dCanvasObject::PROPID_ToolObject ) )
            ic.GetDrawer2D()->OverRuleFixedStyle();

        if ( m_mode == sm_PinUnConnected )
        {
            // This is the usual mode of an unconnecting pin just sitting there
            ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( *wxRED, 0 )  );
            ic.GetDrawer2D()->DrawLine( ( int ) ( dx - w2 ), ( int ) ( dy + h2 ),  ( int ) ( dx + w2 ), ( int ) ( dy - h2 ) );
            ic.GetDrawer2D()->DrawLine( ( int ) ( dx - w2 ), ( int ) ( dy - h2 ),  ( int ) ( dx + w2 ), ( int ) ( dy + h2 ) );
        }

        else if (  m_mode == sm_PinCanConnectToPinClass )
        {
            // This is the mode of a pin that is hit by the mouse (set in OnEnter)
            ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( wxColour( 12, 34, 78 ), 0.5 ) );
            ic.GetDrawer2D()->DrawCircle( dx, dy, m_width / 4 );
        }
        else if ( m_mode == sm_PinCanConnect )
        {
            // This is the mode of a pin that can be connected
            int w = m_pinclass->GetPinCanConnect()->GetWidth();
            int h = m_pinclass->GetPinCanConnect()->GetHeight();
            int r = m_pinclass->GetPinCanConnect()->GetRadius();
            ic.GetDrawer2D()->SetDrawerFill( a2dFill( wxColour( 12, 134, 228 ) ) );
            ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( *wxBLUE, 1 ) );
            ic.GetDrawer2D()->DrawCenterRoundedRectangle( dx, dy, w, h, r );
        }
        else if (  m_mode == sm_PinCannotConnect )
        {
            // This is the mode of a pin that cannot be connected
            int w = m_pinclass->GetPinCannotConnect()->GetWidth();
            int h = m_pinclass->GetPinCannotConnect()->GetHeight();
            int r = m_pinclass->GetPinCanConnect()->GetRadius();
            ic.GetDrawer2D()->SetDrawerFill( a2dFill( wxColour( 245, 134, 178 ) ) );
            ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( wxColour( 12, 34, 178 ), 1 ) );
            ic.GetDrawer2D()->DrawCenterRoundedRectangle( dx, dy, ( int ) ( w * 1.5 ), ( int ) ( h * 1.5 ), r );
        }

        if ( GetAlgoSkip() &&  m_parent->HasProperty( a2dCanvasObject::PROPID_ToolObject ) )
            ic.GetDrawer2D()->ReStoreFixedStyle();
    }
    else
    {
        // check if the pin is dislocated
        // This shouldn't happen, but show it, if it does happen
        if( IsDislocated() || m_RenderConnected )
        {
            ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );
            ic.GetDrawer2D()->DrawLine( ( int )( dx - w2 ), ( int )( dy + h2 ),  ( int )( dx + w2 ), ( int )( dy - h2 ) );
            ic.GetDrawer2D()->DrawLine( ( int )( dx - w2 ), ( int )( dy - h2 ),  ( int )( dx + w2 ), ( int )( dy + h2 ) );
        }
    }

    if ( m_flags.m_MouseInObject )
    {
        // This is the usual mode of an unconnecting pin just sitting there
        ic.GetDrawer2D()->SetDrawerFill( m_pinclass->GetPin()->GetFill() );
        ic.GetDrawer2D()->SetDrawerStroke( m_pinclass->GetPin()->GetStroke() );
        ic.GetDrawer2D()->DrawCircle( ( int )dx, ( int )dy, ( int ) w2 );
    }
    ic.GetDrawer2D()->PopTransform();
}


a2dBoundingBox Pin::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    return a2dPin::DoGetUnTransformedBbox( flags );
}

ElementConnectionGenerator::ElementConnectionGenerator()
    : a2dConnectionGenerator()
{

}

ElementConnectionGenerator::~ElementConnectionGenerator()
{
}

a2dPinClass* ElementConnectionGenerator::GetPinClassForTask( a2dPinClass* pinClass, a2dConnectTask task, a2dCanvasObject* obj, a2dPinClass* pinClassTo, a2dPin* pinFrom ) const
{
    if ( pinClass == a2dPinClass::Any )
    {
        if ( wxDynamicCast( obj, Element ) )
        {
            return Pin::ElementObject;
        }
        else if ( obj->IsConnect() )
        {
            if ( wxDynamicCast( obj, a2dWirePolylineL )->GetStartPinClass() ==  Pin::ElementWire )
                return Pin::ElementWire;
            if ( wxDynamicCast( obj, a2dWirePolylineL )->GetStartPinClass() ==  a2dConnectionGeneratorDirectedFlow::WireInput )
                return a2dConnectionGeneratorDirectedFlow::WireOutput;
            if ( wxDynamicCast( obj, a2dWirePolylineL )->GetStartPinClass() ==  a2dConnectionGeneratorDirectedFlow::WireOutput )
                return a2dConnectionGeneratorDirectedFlow::WireInput;
            else
                return a2dPinClass::Standard;
        }
        else
        {
            return a2dPinClass::Standard;
        }
    }

    if ( pinClass == Pin::ElementObject )
    {
        if ( obj->IsConnect() )
            return Pin::ElementObject;
        else
            return Pin::ElementWire;
    }

    if ( pinClass == Pin::ElementWire )
    {
        if ( obj->IsConnect() )
            return Pin::ElementWire;
        else
            return Pin::ElementObject;
    }

    return NULL;
}
