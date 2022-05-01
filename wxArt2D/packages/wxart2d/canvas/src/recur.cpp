/*! \file canvas/src/recur.cpp
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

#include "wx/canvas/recur.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/drawer.h"

#include <wx/wfstream.h>
#include <math.h>

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasObjectReference, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasObjectArrayReference, a2dCanvasObjectReference )
IMPLEMENT_DYNAMIC_CLASS( a2dLibraryReference, a2dText )
IMPLEMENT_DYNAMIC_CLASS( a2dNameReference, a2dText )
IMPLEMENT_DYNAMIC_CLASS( a2dExternalReference, a2dText )
IMPLEMENT_DYNAMIC_CLASS( a2dSymbolicReference, a2dCanvasObject )

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// a2dCanvasObjectReference
//----------------------------------------------------------------------------

const a2dPropertyIdCanvasObject a2dCanvasObjectReference::PROPID_refobject( wxT( "refobject" ), a2dPropertyId::flag_transfer | a2dPropertyId::flag_onlymemberhint, 0 );

a2dCanvasObjectReference::a2dCanvasObjectReference()
    : a2dCanvasObject()
{
    m_object = 0;
    m_flags.m_ignoreLayer = true;
}

a2dCanvasObjectReference::a2dCanvasObjectReference( double x, double y, a2dCanvasObject* obj )
    : a2dCanvasObject()
{
    m_lworld.Translate( x, y );
    m_object = obj;
    m_flags.m_ignoreLayer = true;
}

a2dCanvasObjectReference::~a2dCanvasObjectReference()
{
}

a2dObject* a2dCanvasObjectReference::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCanvasObjectReference( *this, options, refs );
};

a2dCanvasObjectReference::a2dCanvasObjectReference( const a2dCanvasObjectReference& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    // If needed, understand that if reference has wires connected to it, clone brothers go wrong (not reset).
    // Maybe also use clone_noReference.
    //if ( options & clone_members && other.m_object )
    //    m_object = other.m_object->TClone( CloneOptions( options & ~ clone_seteditcopy ) );
    //else  
        m_object = other.m_object;
}

void a2dCanvasObjectReference::DoAddPending( a2dIterC& ic )
{
    if ( m_object )
        m_object->AddPending( ic );
}

bool a2dCanvasObjectReference::LinkReference( a2dObject* other )
{
    if( !other )
        return false;

#if defined(_DEBUG)
    wxString name = other->GetName();
#endif

    if ( m_object.Get() )
        return false;

    m_object = wxStaticCast( other, a2dCanvasObject );
    m_object->SetCheck( true );
    return true;
}

a2dCanvasObject* a2dCanvasObjectReference::GetCanvasObject()
{
    return m_object;
}

void a2dCanvasObjectReference::SetCanvasObject( a2dCanvasObject* object )
{
    m_object = object;
}

void a2dCanvasObjectReference::MakeReferencesUnique()
{
    if ( m_object && m_object->GetOwnedBy() > 1 )
    {
        m_object = wxStaticCast( m_object->Clone( a2dObject::clone_flat ), a2dCanvasObject );
    }
    a2dCanvasObject::MakeReferencesUnique();
}

a2dCanvasObject* a2dCanvasObjectReference::PushInto( a2dCanvasObject* parent )
{
    if ( m_object )
        return m_object;
    return this;
}

void a2dCanvasObjectReference::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    if ( m_object.Get() )
        m_object->Walker( this, handler );

    a2dCanvasObject::DoWalker( parent, handler );
}

a2dBoundingBox a2dCanvasObjectReference::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox bbox;
    if ( m_object )
        bbox.Expand( m_object->GetBbox() );
    else
        bbox.Expand( 0, 0 );
    return bbox;
}

bool a2dCanvasObjectReference::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( m_object )
    {
        calc = m_object->Update( mode );
    }

    if ( !m_bbox.GetValid() || calc )
    {
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

void a2dCanvasObjectReference::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( !m_object )
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

    OVERLAP childclip = _IN;
    if ( clipparent != _IN )
        childclip = m_object->GetClipStatus( ic, clipparent );

    if ( childclip != _OUT ) //if a child is _OUT, no need to render it.
    {
        // Translate2D over XY from the structurereference
        m_object->Render( ic, childclip );
    }
}

void a2dCanvasObjectReference::DoWalkerWithContext( a2dIterC& ic, wxObject* parent, a2dWalkerIOHandlerWithContext& handler )
{
    if ( !m_object )
        return;

    a2dCanvasObject::DoWalkerWithContext( ic, parent, handler );

    m_object->WalkerWithContext( ic, this, handler );
}

#if wxART2D_USE_CVGIO
void a2dCanvasObjectReference::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_object )
            out.WriteAttribute( wxT( "object" ), m_object->GetId() );
    }
    else
    {
        //if object is not saved yet, we need to save it here.
        if ( m_object && !m_object->GetCheck() )
        {
            //trick the system to have multiple refs on those objects, else we will not get an id attribute
            a2dCanvasObjectPtr multiRef = m_object;

            out.WriteStartElement( wxT( "derived" ) );
            m_object->Save( this, out, towrite );
            out.WriteEndElement();
        }
    }
}

void a2dCanvasObjectReference::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( parser.HasAttribute( wxT( "object" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_object, parser.GetAttributeValue( wxT( "object" ) ) );
        }
    }
    else
    {
        if ( parser.GetTagName() != wxT( "derived" ) )
            return;

        parser.Require( START_TAG, wxT( "derived" ) );
        parser.Next();

        m_object = (a2dCanvasObject*) parser.LoadOneObject( this );

        parser.Require( END_TAG, wxT( "derived" ) );
        parser.Next();
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dCanvasObjectReference::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    if ( !m_object )
        return false;
    return m_object->IsHitWorld( ic, hitEvent ) != 0;
}

void a2dCanvasObjectReference::RemoveHierarchy()
{
    if ( m_object && !m_object->GetRelease() )
    {
        CreateChildObjectList()->push_front( m_object );
    }

    m_object = NULL;

    a2dCanvasObject::RemoveHierarchy();
}

//----------------------------------------------------------------------------
// a2dCanvasObjectArrayReference
//----------------------------------------------------------------------------

a2dCanvasObjectArrayReference::a2dCanvasObjectArrayReference()
    : a2dCanvasObjectReference()
{
}

a2dCanvasObjectArrayReference::a2dCanvasObjectArrayReference( double x, double y, int rows, int columns, double horzSpace, double vertSpace, a2dCanvasObject* obj )
    : a2dCanvasObjectReference( x, y, obj )
{
    m_horzSpace = horzSpace;
    m_vertSpace = vertSpace;
    m_rows = rows;
    m_columns = columns;
}

a2dCanvasObjectArrayReference::~a2dCanvasObjectArrayReference()
{
}

a2dObject* a2dCanvasObjectArrayReference::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCanvasObjectArrayReference( *this, options, refs );
};

a2dCanvasObjectArrayReference::a2dCanvasObjectArrayReference( const a2dCanvasObjectArrayReference& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObjectReference( other, options, refs )
{
    m_horzSpace = other.m_horzSpace;
    m_vertSpace = other.m_vertSpace;
    m_rows = other.m_rows;
    m_columns = other.m_columns;
}

a2dBoundingBox a2dCanvasObjectArrayReference::DoGetUnTransformedBbox( a2dBboxFlags WXUNUSED( flags ) ) const
{
    a2dBoundingBox tmp;

    if ( m_object )
        tmp = m_object->GetBbox();
    else
        tmp.Expand ( 0, 0 );

    a2dBoundingBox bbox;
    bbox.Expand( tmp );

    tmp.Translate( ( m_columns - 1 )*m_horzSpace, 0 );
    bbox.Expand( tmp );

    tmp.Translate( 0, ( m_rows - 1 )*m_vertSpace );
    bbox.Expand( tmp );

    tmp.Translate( -( m_columns - 1 )*m_horzSpace, 0 );
    bbox.Expand( tmp );
    return bbox;
}

bool a2dCanvasObjectArrayReference::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );
        return true;
    }
    return false;
}

void a2dCanvasObjectArrayReference::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    if ( !m_object )
        return;

    a2dAffineMatrix offsetXY;

    int i, j;
    for ( i = 0  ; i < m_rows; i++ )
    {
        for ( j = 0  ; j < m_columns; j++ )
        {
            a2dIterCU cu( ic, offsetXY );

            OVERLAP childclip = _IN;
            if ( clipparent != _IN )
                childclip = m_object->GetClipStatus( ic, clipparent );

            if ( childclip != _OUT ) //if a child is _OUT, no need to render it.
            {
                // Translate2D over XY from the structurereference
                m_object->Render( ic, childclip );
            }

            offsetXY.Translate( m_horzSpace, 0 );
        }
        //translate back (one row of columns) and add one row
        offsetXY.Translate( -m_horzSpace * m_columns, m_vertSpace );
    }
}

#if wxART2D_USE_CVGIO
void a2dCanvasObjectArrayReference::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObjectReference::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "columns" ), m_columns );
        out.WriteAttribute( wxT( "rows" ), m_rows );
        out.WriteAttribute( wxT( "horzspace" ), m_horzSpace );
        out.WriteAttribute( wxT( "vertspace" ), m_vertSpace );
        out.WriteNewLine();
    }
    else
    {
    }
}

void a2dCanvasObjectArrayReference::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObjectReference::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_columns = parser.RequireAttributeValueInt( wxT( "columns" ) );
        m_rows    = parser.RequireAttributeValueInt( wxT( "rows" ) );
        m_horzSpace = parser.RequireAttributeValueDouble( wxT( "horzspace" ) );
        m_vertSpace = parser.RequireAttributeValueDouble( wxT( "vertspace" ) );
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dCanvasObjectArrayReference::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    if ( !m_object )
        return false;

    double pw = ic.GetTransformedHitMargin();
    double margin = ic.GetTransformedHitMargin();

    a2dBoundingBox tmp = DoGetUnTransformedBbox();

    double xmin = tmp.GetMinX();
    double ymin = tmp.GetMinY();
    double xmax = tmp.GetMaxX();
    double ymax = tmp.GetMaxY();

    hitEvent.m_how = HitTestRectangle( hitEvent.m_relx, hitEvent.m_rely, xmin, ymin, xmax, ymax, pw + margin );
    return hitEvent.m_how.IsHit();
}

void a2dCanvasObjectArrayReference::RemoveHierarchy()
{
    if ( m_object && !m_object->GetRelease() )
    {
        a2dCanvasObjectPtr cobj;

        a2dAffineMatrix currentWorld = a2dIDENTITY_MATRIX;
        a2dAffineMatrix offsetXY;

        int i, j;
        for ( i = 0  ; i < GetRows(); i++ )
        {
            for ( j = 0  ; j < GetColumns(); j++ )
            {
                a2dAffineMatrix tworld = offsetXY;
                tworld *= m_lworld;

                cobj = ( a2dCanvasObject* ) m_object->Clone( clone_deep );
                //cobj->Transform( offsetXY );
                //cobj->Transform( m_lworld );
                cobj->SetTransformMatrix( tworld );
                cobj->SetRelease( false );
                cobj->SetPending( true );

                CreateChildObjectList()->push_back( cobj );

                offsetXY.Translate( GetHorzSpace(), 0 );
            }
            //translate back (one row of columns) and add one row
            offsetXY.Translate( -GetHorzSpace()*GetColumns(), GetVertSpace() );
        }
        m_object = NULL;
    }

    a2dCanvasObject::RemoveHierarchy();
}


//----------------------------------------------------------------------------
// a2dLibraryReference
//----------------------------------------------------------------------------

a2dLibraryReference::a2dLibraryReference()
    : a2dImage()
{
    m_layer = GetHabitat()->GetSymbolLayer();
    SetName( wxT( "libraryref" ) );
    m_symbol = 0;

    m_height = 100;
    m_width  = 0;

    m_text = new a2dText( wxT( "libraryref" ), -m_height / 2 , -m_height / 2 - m_height / 5, a2dFont( m_height / 5, wxFONTFAMILY_SWISS ) );

    m_object = 0;
}


a2dLibraryReference::a2dLibraryReference( double x, double y, a2dCanvasObject* obj, const wxString& name,
        double totalheight, a2dCanvasObject* symbol,
        const a2dFont& font, int imagew, int imageh )

    : a2dImage( wxImage( imagew, imageh ), x, y, totalheight, totalheight )
{
    m_object = obj;
    m_symbol = symbol;

    m_layer = GetHabitat()->GetSymbolLayer();

    RenderObject( symbol ? symbol : obj, imagew, imageh );

    SetName( name );

    if ( !name.IsEmpty() )
    {
        a2dFont lfont = font;
        lfont.SetSize( totalheight / 5.0 );
        m_text = new a2dText( name, -totalheight / 2, -totalheight / 2 - totalheight / 5, lfont );
        m_text->SetStroke( *a2dBLACK_STROKE );
        m_text->SetFill( *a2dTRANSPARENT_FILL );
        m_text->SetLayer( GetHabitat()->GetSymbolLayer() );
    }
    else
    {
        m_text = 0;
    }

    m_flags.m_visiblechilds = false;

    m_height = totalheight;
    m_width  = totalheight;

    SetFill( *a2dTRANSPARENT_FILL );
}

a2dLibraryReference::a2dLibraryReference( const a2dLibraryReference& other, CloneOptions options, a2dRefMap* refs )
    : a2dImage( other, options, refs )
{
    m_object = other.m_object;

    m_height = other.m_height;
    m_width = other.m_width;
    m_description = other.m_description;
    m_text = ( a2dText* )other.m_text->Clone( clone_deep );
    m_symbol = other.m_symbol;
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //it is because the CurrentSmartPointerOwner can change in property->TClone()
    CurrentSmartPointerOwner = this;
#endif
}

a2dLibraryReference::~a2dLibraryReference()
{
}

void a2dLibraryReference::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    a2dCanvasObject::DoWalker( parent, handler );
    if ( m_symbol )
        m_symbol->Walker( this, handler );

    if ( m_text )
        m_text->Walker( this, handler );
}

a2dCanvasObject* a2dLibraryReference::GetCanvasObject()
{
    return m_object;
}

void a2dLibraryReference::SetCanvasObject( a2dCanvasObject* object )
{
    m_object = object;
}


void a2dLibraryReference::SetSymbol( a2dCanvasObject* symbol )
{
    SetPending( true );
    m_symbol = symbol;
    if ( m_symbol && m_root  )
        m_symbol->SetRoot( m_root );

    RenderObject( m_symbol ? m_symbol.Get() : GetCanvasObject() );

}

a2dCanvasObject* a2dLibraryReference::GetSymbol()
{
    return m_symbol;
}

a2dObject* a2dLibraryReference::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dLibraryReference( *this, options, refs );
}

a2dBoundingBox a2dLibraryReference::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox = a2dImage::DoGetUnTransformedBbox( flags );

    bbox.Expand( m_text->GetUnTransformedBbox() );
    return bbox;
}

bool a2dLibraryReference::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
        RenderObject( m_symbol ? m_symbol : m_object, ( int ) m_width, ( int ) m_height );

        a2dImage::DoUpdate( mode, childbox, clipbox, propbox );
        if ( m_text )
        {
            a2dBoundingBox tbbox;
            m_text->Update( mode );
            tbbox.Expand( m_text->GetBbox() );
            tbbox.MapBbox( m_lworld );
            m_bbox.Expand( tbbox );
        }

        m_width = m_bbox.GetWidth();

        return true;
    }
    return false;
}

void a2dLibraryReference::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    a2dImage::DoRender( ic, clipparent );
    if ( m_text )
        m_text->Render( ic, clipparent );
}

#if wxART2D_USE_CVGIO
void a2dLibraryReference::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dImage::DoSave( parent, out, xmlparts, towrite );
}

void a2dLibraryReference::DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) )
{
}
#endif //wxART2D_USE_CVGIO

bool a2dLibraryReference::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    if ( m_text && m_text->IsHitWorld( ic, hitEvent ) )
        return true;

    return a2dImage::DoIsHitWorld( ic, hitEvent );
}


//----------------------------------------------------------------------------
// a2dExternalReference
//----------------------------------------------------------------------------
a2dExternalReference::a2dExternalReference()
    : a2dText( wxT( "" ) , 0, 0 )
{
    m_text = wxT( "nameref" );
    m_object = 0;
}

a2dExternalReference::a2dExternalReference( double x, double y, a2dCanvasObject* obj, const wxString& text, double angle,  const a2dFont& font )
    : a2dText( text, x, y, font, angle )
{
    if ( m_text.IsEmpty() && obj )
        m_text = obj->GetName();
    else
        m_text = text;

    m_object = obj;
}

a2dExternalReference::a2dExternalReference( const a2dExternalReference& other, CloneOptions options, a2dRefMap* refs )
    : a2dText( other, options, refs )
{
    m_object = other.m_object;
}

a2dExternalReference::~a2dExternalReference()
{
}

void a2dExternalReference::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    a2dCanvasObject::DoWalker( parent, handler );
}

a2dCanvasObject* a2dExternalReference::GetCanvasObject()
{
    return m_object;
}

void a2dExternalReference::SetCanvasObject( a2dCanvasObject* object )
{
    m_object = object;
}

a2dObject* a2dExternalReference::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dExternalReference( *this, options, refs );
};

//----------------------------------------------------------------------------
// a2dNameReference
//----------------------------------------------------------------------------
a2dNameReference::a2dNameReference()
    : a2dText( wxT( "" ) , 0, 0 )
{
    m_text = wxT( "nameref" );
}

a2dNameReference::a2dNameReference( double x, double y, a2dCanvasObject* obj, const wxString& text, double angle,  const a2dFont& font )
    : a2dText( text, x, y, font, angle )
{
    if ( m_text.IsEmpty() && obj )
        m_text = obj->GetName();
    else
        m_text = text;

    m_objectRef = obj;
}

a2dNameReference::a2dNameReference( const a2dNameReference& other, CloneOptions options, a2dRefMap* refs )
    : a2dText( other, options, refs )
{
    m_objectRef = other.m_objectRef;
}

a2dNameReference::~a2dNameReference()
{
}

a2dCanvasObject* a2dNameReference::GetCanvasObject()
{
    return m_objectRef;
}

void a2dNameReference::SetCanvasObject( a2dCanvasObject* object )
{
    m_objectRef = object;
}

a2dObject* a2dNameReference::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dNameReference( *this, options, refs );
}

void a2dNameReference::DoAddPending( a2dIterC& ic )
{
    if ( m_objectRef )
        m_objectRef->AddPending( ic );
}

a2dCanvasObject* a2dNameReference::PushInto( a2dCanvasObject* parent )
{
    if ( m_objectRef )
        return m_objectRef;
    return this;
}

bool a2dNameReference::LinkReference( a2dObject* other )
{
    if( !other )
        return false;

#if defined(_DEBUG)
    wxString name = other->GetName();
#endif

    if ( m_objectRef.Get() )
        return false;

    m_objectRef = wxStaticCast( other, a2dCanvasObject );
    m_objectRef->SetCheck( true );
    return true;
}

void a2dNameReference::DoWalker( wxObject* parent, a2dWalkerIOHandler& handler )
{
    if ( m_objectRef.Get() )
        m_objectRef->Walker( this, handler );

    a2dText::DoWalker( parent, handler );
}

bool a2dNameReference::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    bool calc = false;
    if ( m_objectRef )
    {
        calc = m_objectRef->Update( mode );
    }

    return a2dText::DoUpdate( mode, childbox, clipbox, propbox );
}

#if wxART2D_USE_CVGIO
void a2dNameReference::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dText::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( m_objectRef )
            out.WriteAttribute( wxT( "object" ), m_objectRef->GetId() );
    }
    else
    {
        //if object is not saved yet, we need to save it here.
        if ( m_objectRef && !m_objectRef->GetCheck() )
        {
            //trick the system to have multiple refs on those objects, else we will not get an id attribute
            a2dCanvasObjectPtr multiRef = m_objectRef;

            out.WriteStartElement( wxT( "derived" ) );
            m_objectRef->Save( this, out, towrite );
            out.WriteEndElement();
        }
    }
}

void a2dNameReference::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dText::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( parser.HasAttribute( wxT( "object" ) ) )
        {
            parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_objectRef, parser.GetAttributeValue( wxT( "object" ) ) );
        }
    }
    else
    {
        if ( parser.GetTagName() != wxT( "derived" ) )
            return;

        parser.Require( START_TAG, wxT( "derived" ) );
        parser.Next();

        m_objectRef = (a2dCanvasObject*) parser.LoadOneObject( this );

        parser.Require( END_TAG, wxT( "derived" ) );
        parser.Next();
    }
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dSymbolicReference
//----------------------------------------------------------------------------

a2dSymbolicReference::a2dSymbolicReference()
    : a2dCanvasObject()
{
}


a2dSymbolicReference::a2dSymbolicReference( double x, double y, a2dCanvasObject* obj )

    : a2dCanvasObject()
{
    m_object = obj;
}

a2dSymbolicReference::a2dSymbolicReference( const a2dSymbolicReference& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_object = other.m_object;
}

a2dSymbolicReference::~a2dSymbolicReference()
{
}

a2dObject* a2dSymbolicReference::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dSymbolicReference( *this, options, refs );
}

#if wxART2D_USE_CVGIO
void a2dSymbolicReference::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
}

void a2dSymbolicReference::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
}
#endif //wxART2D_USE_CVGIO

bool a2dSymbolicReference::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    return a2dCanvasObject::DoIsHitWorld( ic, hitEvent );
}

