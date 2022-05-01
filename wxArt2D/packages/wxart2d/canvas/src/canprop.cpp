/*! \file canvas/src/canprop.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canprop.cpp,v 1.96 2008/08/14 18:33:28 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canprop.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/polygon.h"
//#include "wx/general/id.inl"
#include <wx/tokenzr.h>

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_DYNAMIC_CLASS( a2dStyleProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dShadowStyleProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dClipPathProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasObjectPtrProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dTagVecProperty, a2dNamedProperty )

//----------------------------------------------------------------------------
// template instantiations
//----------------------------------------------------------------------------
#if (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))

template class a2dPropertyIdProp<class a2dClipPathProperty>;

#endif // (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))


//----------------------------------------------------------------------------
// a2dStyleProperty
//----------------------------------------------------------------------------
a2dStyleProperty::a2dStyleProperty( )
    : a2dNamedProperty()
{
    //we choose  a2dNullFILL etc. because that is the default behaviour if no style is specified
    m_fill = *a2dNullFILL;
    m_stroke = *a2dNullSTROKE;
}

a2dStyleProperty::a2dStyleProperty( const a2dPropertyId* id )
    : a2dNamedProperty( id )
{
    //we choose  a2dNullFILL etc. because that is the default behaviour if no style is specified
    m_fill = *a2dNullFILL;
    m_stroke = *a2dNullSTROKE;
}

a2dStyleProperty::~a2dStyleProperty()
{
}

a2dStyleProperty::a2dStyleProperty( const a2dStyleProperty& other )
    : a2dNamedProperty( other )
{
    m_fill = other.m_fill;
    m_stroke = other.m_stroke;
}

a2dNamedProperty* a2dStyleProperty::Clone( a2dObject::CloneOptions WXUNUSED( options ) ) const
{
    return new a2dStyleProperty( *this );
};

void a2dStyleProperty::Assign( const a2dNamedProperty& other )
{
    a2dStyleProperty* propcast = wxStaticCast( &other, a2dStyleProperty );
    m_stroke = propcast->m_stroke;
    m_fill = propcast->m_fill;
}

wxString a2dStyleProperty::StringRepresentation() const
{
    return GetName() + wxT( " = " ) + StringValueRepresentation();
}

wxString a2dStyleProperty::StringValueRepresentation() const
{
    wxString s;
    s = s + wxT( "fill " );
    s = s + wxT( "stroke " );
    return s;
}

bool a2dStyleProperty::AllNo()
{
    if ( m_fill.IsNoFill() && m_stroke.IsNoStroke() )
        return true;
    return false;
}

void a2dStyleProperty::SetFill( const a2dFill& fill )
{
    m_fill = fill;
}

void a2dStyleProperty::SetFill( const wxColour& fillcolor, a2dFillStyle style )
{
    m_fill = a2dFill( fillcolor, style );
}

void a2dStyleProperty::SetFill(  const wxColour& fillcolor, const wxColour& fillcolor2, a2dFillStyle style )
{
    m_fill = a2dFill( fillcolor, fillcolor2, style );
}


void a2dStyleProperty::SetStroke( const wxColour& strokecolor, float width, a2dStrokeStyle style )
{
    m_stroke = a2dStroke( strokecolor, width, style );
}

void a2dStyleProperty::SetStroke( const wxColour& strokecolor, int width, a2dStrokeStyle style )
{
    m_stroke = a2dStroke( strokecolor, width, style );
}

void a2dStyleProperty::SetStroke( const a2dStroke& stroke )
{
    m_stroke = stroke;
}

#if wxART2D_USE_CVGIO
void a2dStyleProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        if ( !m_fill.IsNoFill() )
            m_fill.Save( parent, out, towrite );

        if ( !m_stroke.IsNoStroke() )
            m_stroke.Save( parent, out, towrite );
    }
}

void a2dStyleProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag WXUNUSED( xmlparts ) )
{
    parser.Next();

    if ( parser.GetTagName() == wxT( "fill" ) )
    {
        if ( parser.GetAttributeValue( wxT( "style" ) ) == wxT( "inherit" ) )
        {
            m_fill = *a2dINHERIT_FILL;
            parser.Next();
            parser.Require( END_TAG, wxT( "fill" ) );
            parser.Next();
        }
        else
        {
            a2dFill* fill = ( a2dFill* ) parser.CreateObject( parser.GetAttributeValue( wxT( "classname" ) ) );
            wxASSERT_MSG( fill, wxT( "wrong a2dFill" ) );
            fill->Load( parent, parser );
            m_fill = *fill;
            delete fill;
        }
    }

    if ( parser.GetTagName() == wxT( "stroke" ) )
    {
        if ( parser.GetAttributeValue( wxT( "style" ) ) == wxT( "inherit" ) )
        {
            m_stroke = *a2dINHERIT_STROKE;
            parser.Next();
            parser.Require( END_TAG, wxT( "stroke" ) );
            parser.Next();
        }
        else
        {
            a2dStroke* stroke = ( a2dStroke* ) parser.CreateObject( parser.GetAttributeValue( wxT( "classname" ) ) );
            wxASSERT_MSG( stroke, wxT( "wrong a2dStroke" ) );
            stroke->Load( parent, parser );
            m_stroke = *stroke;
            delete stroke;
        }
    }
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dShadowStyleProperty
//----------------------------------------------------------------------------

a2dShadowStyleProperty::a2dShadowStyleProperty()
    : a2dStyleProperty()
{
    m_depth = 0;
    m_angle3d = 30;
}

a2dShadowStyleProperty::a2dShadowStyleProperty( const a2dPropertyIdCanvasShadowStyle* id, double depth, double angle )
    : a2dStyleProperty( id )
{
    m_depth = depth;
    m_angle3d = angle;
}


a2dShadowStyleProperty::~a2dShadowStyleProperty()
{
}

a2dShadowStyleProperty::a2dShadowStyleProperty( const a2dShadowStyleProperty& other )
    : a2dStyleProperty( other )
{
    m_depth = other.m_depth;
    m_angle3d = other.m_angle3d;
}

a2dNamedProperty* a2dShadowStyleProperty::Clone( a2dObject::CloneOptions WXUNUSED( options ) ) const
{
    return new a2dShadowStyleProperty( *this );
};

#if wxART2D_USE_CVGIO
void a2dShadowStyleProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dStyleProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "depth" ), m_depth );
        out.WriteAttribute( wxT( "angle" ), m_angle3d );
    }
    else
    {
    }
}


void a2dShadowStyleProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dStyleProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_depth = parser.GetAttributeValueDouble( wxT( "depth" ) );
        m_angle3d = parser.GetAttributeValueDouble( wxT( "angle" ) );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO


//----------------------------------------------------------------------------
// a2dClipPathProperty
//----------------------------------------------------------------------------

a2dClipPathProperty::a2dClipPathProperty()
    : a2dNamedProperty()
{
    m_clip = 0;
    m_visible = false;
    m_render = true;
}

a2dClipPathProperty::a2dClipPathProperty( const a2dPropertyIdCanvasClipPath* id, a2dPolygonL* clip )
    : a2dNamedProperty( id )
{
    m_clip = clip;
    m_visible = false;
    m_render = true;
}

a2dClipPathProperty::~a2dClipPathProperty()
{
}

a2dClipPathProperty::a2dClipPathProperty( const a2dClipPathProperty& other, a2dObject::CloneOptions options )
    : a2dNamedProperty( other )
{
    if ( options & a2dObject::clone_members )
    {
        if ( other.m_clip )
            m_clip = ( a2dPolygonL* ) other.m_clip->TClone( options );
    }
    else
        m_clip = other.m_clip;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in Clone ( )
    CurrentSmartPointerOwner = this;
#endif
}

a2dNamedProperty* a2dClipPathProperty::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dClipPathProperty( *this, options );
};

void a2dClipPathProperty::Assign( const a2dNamedProperty& other )
{
    a2dClipPathProperty* propcast = wxStaticCast( &other, a2dClipPathProperty );
    m_clip = propcast->m_clip;
    m_visible = propcast->m_visible;
    m_render = propcast->m_render;
}

a2dObject* a2dClipPathProperty::GetRefObject() const
{
    return m_clip;
}

a2dObject* a2dClipPathProperty::GetRefObjectNA() const
{
    return m_clip;
}

void a2dClipPathProperty::SetCanvasObject( a2dPolygonL* clip )
{
    m_clip = ( a2dPolygonL* ) clip;
}

void a2dClipPathProperty::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    if ( m_clip && GetCanRender() )
        m_clip->Walker( this, handler );
}

#if wxART2D_USE_CVGIO
void a2dClipPathProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "clippath" ), GetName() );
    }
    else
    {
        if ( m_clip )
            m_clip->Save( this, out, towrite );
    }
}

void a2dClipPathProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString clipobject = parser.GetAttributeValue( wxT( "clippath" ) );
        if ( clipobject != wxT( "" ) )
        {
            //TODO
            //m_clip = clipobject;
        }
    }
    else
    {
        parser.Require( START_TAG, wxT( "o" ) );
        wxString classname = parser.GetAttributeValue( wxT( "classname" ) );

        if ( classname == wxT( "ref" ) )
        {
            parser.SkipSubTree();
            parser.Require( END_TAG, wxT( "o" ) );
            parser.Next();
        }
        else
        {
            a2dPolygonL* object = ( a2dPolygonL* ) parser.CreateObject( classname );
            if ( !object )
            {
                wxLogError( wxT( "could not create a2dPolygonL from %s, will be skipped line %d" ),
                            classname.c_str(), parser.GetCurrentLineNumber() );
                parser.SkipSubTree();
                parser.Require( END_TAG, wxT( "o" ) );
                parser.Next();
            }
            else
            {
                m_clip = object;
                m_clip->Load( this, parser );
            }
        }
    }

}
#endif //wxART2D_USE_CVGIO

void a2dClipPathProperty::PushClip( a2dDrawingPart* drawer, a2dBooleanClip clipoperation )
{
    if ( m_clip )
        drawer->GetDrawer2D()->ExtendAndPushClippingRegion( m_clip->GetSegments(), m_clip->GetSpline(),  m_clip->GetFillRule(), clipoperation );
}

void a2dClipPathProperty::PopClip( a2dDrawingPart* drawer )
{
    if ( m_clip )
    {
        drawer->GetDrawer2D()->PopClippingRegion();
    }
}

//----------------------------------------------------------------------------
// a2dVisibleProperty
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE( a2dVisibleProperty, a2dText )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dVisibleProperty::OnMouseEvent )
    EVT_CHAR( a2dVisibleProperty::OnChar )
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS( a2dVisibleProperty, a2dText );

void a2dVisibleProperty::OnPropertyChanged( a2dComEvent& event )
{
    if ( event.GetPropertyId() == m_propId )
    {
        SetPending( true );
    }
}

a2dVisibleProperty::a2dVisibleProperty()
    : a2dText()
{
    m_flags.m_visible = true;
    m_flags.m_prerenderaschild = false;
    m_flags.m_subEditAsChild = true;
    m_propId = NULL;
    m_showname = true;
    m_parent = NULL;
}

a2dVisibleProperty::a2dVisibleProperty( a2dCanvasObject* parent, const a2dPropertyId* propertyId, double x, double y, double angle )
    : a2dText( wxT( "" ), x, y, *a2dDEFAULT_CANVASFONT, angle )
{
    m_flags.m_visible = true;
    m_flags.m_prerenderaschild = false;
    m_flags.m_subEditAsChild = true;
    m_parent = parent;
    m_propId = propertyId;

    Connect( wxID_ANY, wxID_ANY, a2dEVT_COM_EVENT, wxObjectEventFunction( &a2dVisibleProperty::OnPropertyChanged ), 0, this );
    m_showname = true;
}

a2dVisibleProperty::a2dVisibleProperty( a2dCanvasObject* parent, const a2dPropertyId* propertyId, double x, double y, bool visible, const a2dFont& font, double angle )
    : a2dText( wxT( "" ), x, y, font, angle )
{
    m_flags.m_visible = visible;
    m_flags.m_prerenderaschild = false;
    m_flags.m_subEditAsChild = true;
    m_parent = parent;
    m_propId = propertyId;
    Connect( wxID_ANY, wxID_ANY, a2dEVT_COM_EVENT, wxObjectEventFunction( &a2dVisibleProperty::OnPropertyChanged ), 0, this );
    m_showname = true;
}

a2dVisibleProperty::~a2dVisibleProperty()
{
    Disconnect( wxID_ANY, wxID_ANY, a2dEVT_COM_EVENT, wxObjectEventFunction( &a2dVisibleProperty::OnPropertyChanged ), 0, this );
}

a2dVisibleProperty::a2dVisibleProperty( const a2dVisibleProperty& other, CloneOptions options, a2dRefMap* refs )
    : a2dText( other, options, refs )
{
    m_propId = other.m_propId;
    m_parent = other.m_parent;
    m_showname = other.m_showname;
}

void a2dVisibleProperty::ShowName( bool show )
{
    m_showname = show;
    SetPending( true );
}

void a2dVisibleProperty::SetParent( a2dCanvasObject* parent )
{
    m_parent = parent;
    SetPending( true );
}

a2dObject* a2dVisibleProperty::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dVisibleProperty( *this, options, refs );
};

void a2dVisibleProperty::OnMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    event.Skip();
}

void a2dVisibleProperty::DoEndEdit()
{
    a2dNamedProperty*  property = m_propId->CreatePropertyFromString( m_text.Mid( m_firsteditable ) );
    m_root->GetCommandProcessor()->Submit( new a2dCommand_SetCanvasProperty( m_parent, property ) );

    a2dText::DoEndEdit();
}

void a2dVisibleProperty::OnChar( wxKeyEvent& event )
{
    //klion: warning C4189: 'selection_start_pos' : local variable is initialized but not referenced
    //size_t selection_start_pos = m_selection_start_pos;
    //klion: warning C4189: 'selection_end_pos' : local variable is initialized but not referenced
    //size_t selection_end_pos = m_selection_end_pos;
    //klion: warning C4189: 'caret' : local variable is initialized but not referenced
    //size_t caret = m_caret;
    wxString text = m_text;

    a2dText::OnChar( event );
    if ( m_flags.m_editingCopy )
    {
        //m_propId->ValidateString( m_text )
    }
}

bool a2dVisibleProperty::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
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
        a2dNamedPropertyPtr property = m_parent->GetProperty( m_propId );
        if ( property )
            m_text = m_text + property->StringValueRepresentation();
        m_utbbox_changed = true;
    }

    return a2dText::DoUpdate( mode, childbox, clipbox, propbox );
}

#if wxART2D_USE_CVGIO
void a2dVisibleProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
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

void a2dVisibleProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dText::DoLoad( parent, parser, xmlparts );

    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_parent = wxStaticCast( parent, a2dCanvasObject );
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
// a2dCanvasObjectPtrProperty
//----------------------------------------------------------------------------

a2dCanvasObjectPtrProperty::a2dCanvasObjectPtrProperty()
    : a2dNamedProperty()
{
    m_object = 0;
    m_visible = false;
    m_render = false;
    m_prerender = false;
    m_selectedonly = false;
}

a2dCanvasObjectPtrProperty::a2dCanvasObjectPtrProperty( const a2dPropertyIdCanvasObject* id, a2dCanvasObject* object, bool visible, bool render )
    : a2dNamedProperty( id )
{
    m_object = wxStaticCastNull( object, a2dCanvasObject );
    m_visible = visible;
    m_render = render;
    m_prerender = false;
    m_selectedonly = false;
}

a2dCanvasObjectPtrProperty::~a2dCanvasObjectPtrProperty()
{
}

a2dCanvasObjectPtrProperty::a2dCanvasObjectPtrProperty( const a2dCanvasObjectPtrProperty& other, a2dObject::CloneOptions options )
    : a2dNamedProperty( other )
{
    if ( options & a2dObject::clone_properties && m_id->IsCloneDeep() )
    {
        if ( other.m_object )
            m_object = other.m_object->TClone( options );
    }
    else
        m_object = other.m_object;
    m_visible = other.m_visible;
    m_render = other.m_render;
    m_prerender = other.m_prerender;
    m_selectedonly = other.m_selectedonly;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in Clone ( )
    CurrentSmartPointerOwner = this;
#endif
}

a2dNamedProperty* a2dCanvasObjectPtrProperty::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCanvasObjectPtrProperty( *this, options );
};

void a2dCanvasObjectPtrProperty::Assign( const a2dNamedProperty& other )
{
    a2dCanvasObjectPtrProperty* propcast = wxStaticCast( &other, a2dCanvasObjectPtrProperty );
    m_object = propcast->m_object;
    m_prerender = propcast->m_prerender;
    m_selectedonly = propcast->m_selectedonly;
    m_visible = propcast->m_visible;
    m_render = propcast->m_render;
}

a2dCanvasObjectPtrProperty* a2dCanvasObjectPtrProperty::CreatePropertyFromString( const a2dPropertyIdCanvasObject* WXUNUSED( id ), const wxString& WXUNUSED( value ) )
{
    return 0;
}

void a2dCanvasObjectPtrProperty::SetObject( wxObject* object )
{
    m_object = wxStaticCastNull( object, a2dCanvasObject );
}

void a2dCanvasObjectPtrProperty::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    if ( m_object && GetCanRender() )
        m_object->Walker( this, handler );
}


#if wxART2D_USE_CVGIO
void a2dCanvasObjectPtrProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        if ( m_object )
            m_object->Save( parent, out, towrite  );
    }
}

void a2dCanvasObjectPtrProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_object = NULL;
    }
    else
    {
        if ( parser.GetTagName() == wxT( "o" ) && parser.GetEventType() != END_TAG )
        {
            parser.Require( START_TAG, wxT( "o" ) );

            wxString classname = parser.GetAttributeValue( wxT( "classname" ) );
            m_object = wxStaticCast( parser.CreateObject( classname ), a2dCanvasObject );
            if ( !m_object )
            {
                a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "could not create a2dCanvasObject %s, will be skipped line %d" ),
                                                 classname.c_str(), parser.GetCurrentLineNumber() );
                parser.SkipSubTree();
                parser.Require( END_TAG, wxT( "o" ) );
                parser.Next();
            }
            else
            {
                m_object->Load( this, parser );
                parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_object );
            }
        }
    }
}
#endif //wxART2D_USE_CVGIO


//----------------------------------------------------------------------------
// a2dBoudingBoxProperty
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dBoudingBoxProperty, a2dNamedProperty )

a2dBoudingBoxProperty::a2dBoudingBoxProperty(): a2dNamedProperty()
{
    m_value = a2dBoundingBox();
}

a2dBoudingBoxProperty::a2dBoudingBoxProperty( const a2dPropertyIdBoundingBox* id, const a2dBoundingBox& value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dBoudingBoxProperty::a2dBoudingBoxProperty( const a2dPropertyIdBoundingBox* id, const wxString& value )
    : a2dNamedProperty( id )
{
    wxStringTokenizer args( value, wxT( "," ) );
    while ( args.HasMoreTokens() )
    {
        double x;
        args.GetNextToken().ToDouble( &x );
        double y;
        args.GetNextToken().ToDouble( &y );
        m_value.Expand( x, y );
    }
}

a2dBoudingBoxProperty::~a2dBoudingBoxProperty()
{
}

a2dBoudingBoxProperty::a2dBoudingBoxProperty( const a2dBoudingBoxProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dBoudingBoxProperty::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dBoudingBoxProperty( *this );
};

void a2dBoudingBoxProperty::Assign( const a2dNamedProperty& other )
{
    a2dBoudingBoxProperty* propcast = wxStaticCast( &other, a2dBoudingBoxProperty );
    m_value = propcast->m_value;
}

a2dBoudingBoxProperty* a2dBoudingBoxProperty::CreatePropertyFromString( const a2dPropertyIdBoundingBox* id, const wxString& value )
{
    a2dBoundingBox valuenew;
    wxStringTokenizer args( value );
    while ( args.HasMoreTokens() )
    {
        double x;
        args.GetNextToken().ToDouble( &x );
        double y;
        args.GetNextToken().ToDouble( &y );
        valuenew.Expand( x, y );
    }
    return new a2dBoudingBoxProperty( id, valuenew );
}

#if wxART2D_USE_CVGIO
void a2dBoudingBoxProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        wxString attrib;
        attrib << wxString::FromCDouble( m_value.GetMinX() ) << wxT( " " );
        attrib << wxString::FromCDouble( m_value.GetMinY() ) << wxT( " " );
        attrib << wxString::FromCDouble( m_value.GetMaxX() ) << wxT( " " );
        attrib << wxString::FromCDouble( m_value.GetMaxY() );

        out.WriteAttribute( wxT( "value" ), attrib );
    }
    else
    {
    }
}

void a2dBoudingBoxProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dBoudingBoxProperty::SetValue( const a2dBoundingBox& value )
{
    m_value = value;
}

wxString a2dBoudingBoxProperty::StringRepresentation() const
{
    wxString attrib;
    attrib << wxString::FromCDouble( m_value.GetMinX() ) << wxT( " " );
    attrib << wxString::FromCDouble( m_value.GetMinY() ) << wxT( " " );
    attrib << wxString::FromCDouble( m_value.GetMaxX() ) << wxT( " " );
    attrib << wxString::FromCDouble( m_value.GetMaxY() );

    wxString form;
    form.Printf( wxT( "%s = %s" ), GetName().c_str(), attrib.c_str() );
    return form;
}

wxString a2dBoudingBoxProperty::StringValueRepresentation() const
{
    wxString attrib;
    attrib << wxString::FromCDouble( m_value.GetMinX() ) << wxT( " " );
    attrib << wxString::FromCDouble( m_value.GetMinY() ) << wxT( " " );
    attrib << wxString::FromCDouble( m_value.GetMaxX() ) << wxT( " " );
    attrib << wxString::FromCDouble( m_value.GetMaxY() );

    wxString form;
    form.Printf( wxT( "%s" ), attrib.c_str() );
    return form;
}

//----------------------------------------------------------------------------
// a2dTagVecProperty
//----------------------------------------------------------------------------

static int a2dTagCount = 0;

A2DCANVASDLLEXP a2dTag a2dNewTag()
{
    a2dTagCount++;
    if ( a2dTagCount >= 255 )
        a2dTagCount = 0;
    return a2dTagCount;
}

a2dTagVecProperty::a2dTagVecProperty(): a2dNamedProperty()
{
}

a2dTagVecProperty::a2dTagVecProperty( const a2dPropertyIdTagVec* id, a2dTagVec value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dTagVecProperty::~a2dTagVecProperty()
{
}

a2dTagVecProperty::a2dTagVecProperty( const a2dTagVecProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dTagVecProperty::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dTagVecProperty( *this );
};

void a2dTagVecProperty::Assign( const a2dNamedProperty& other )
{
    a2dTagVecProperty* propcast = wxStaticCast( &other, a2dTagVecProperty );
    m_value = propcast->m_value;
}

a2dTagVecProperty* a2dTagVecProperty::CreatePropertyFromString( const a2dPropertyIdTagVec* id, const wxString& value )
{
    a2dTagVec result;
    //value.ParseItToa2dTagVec();
    return new a2dTagVecProperty( id, result );
}

#if wxART2D_USE_CVGIO
void a2dTagVecProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        for( a2dTagVec::iterator iter = m_value.begin(); iter != m_value.end(); ++iter )
        {
            wxUint8 tag = *iter;
            out.WriteStartElementAttributes( wxT( "tag" ) );
            out.WriteAttribute( wxT( "v" ), tag );
            out.WriteEndAttributes();
        }
    }
}

void a2dTagVecProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        while( parser.GetTagName() == wxT( "tag" )  )
        {
            a2dTag value = parser.GetAttributeValueUint16( wxT( "v" ) );
            m_value.push_back( value );
            parser.Next();
            parser.Require( END_TAG, wxT( "tag" ) );
            parser.Next();
        }
    }
}
#endif //wxART2D_USE_CVGIO

void a2dTagVecProperty::SetValue( a2dTagVec value )
{
    m_value = value;
}

wxString a2dTagVecProperty::StringRepresentation() const
{
    wxString form;
    //form.Printf( wxT( "%s = %d" ), GetName().c_str(), m_value );
    return  form;
}

wxString a2dTagVecProperty::StringValueRepresentation() const
{
    wxString form;
    //form.Printf( wxT( "%d" ), m_value );
    return form;
}

void a2dTagVecProperty::PushTag( a2dTag tag )
{
    m_value.push_back( tag );
}

a2dTag a2dTagVecProperty::PopTag()
{
    a2dTag tag = m_value.back();
    m_value.pop_back();
    return tag;
}

a2dTag a2dTagVecProperty::Last() const
{
    return m_value.back();
}
