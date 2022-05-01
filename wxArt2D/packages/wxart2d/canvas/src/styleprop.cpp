/*! \file canvas/src/styleprop.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: styleprop.cpp,v 1.33 2009/07/24 16:35:01 titato Exp $
*/

#include "a2dprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "wx/canvas/canglob.h"
#include "wx/canvas/styleprop.h"
#include "wx/canvas/canobj.h"

#define newline wxString("\n")

IMPLEMENT_DYNAMIC_CLASS( a2dFillProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dStrokeProperty, a2dNamedProperty )
IMPLEMENT_DYNAMIC_CLASS( a2dFontProperty, a2dNamedProperty )

//----------------------------------------------------------------------------
// a2dFillProperty
//----------------------------------------------------------------------------

a2dFillProperty::a2dFillProperty(): a2dNamedProperty()
{
}

a2dFillProperty::a2dFillProperty( const a2dPropertyIdFill* id, const a2dFill& value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dFillProperty::~a2dFillProperty()
{
}

a2dFillProperty::a2dFillProperty( const a2dFillProperty* ori )
    : a2dNamedProperty( *ori )
{
    m_value = ori->m_value;
}

a2dFillProperty::a2dFillProperty( const a2dFillProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dFillProperty::Clone( a2dObject::CloneOptions WXUNUSED( options ) ) const
{
    return new a2dFillProperty( *this );
};

void a2dFillProperty::Assign( const a2dNamedProperty& other )
{
    a2dFillProperty* propcast = wxStaticCast( &other, a2dFillProperty );
    m_value = propcast->m_value;
}

a2dFillProperty* a2dFillProperty::CreatePropertyFromString( const a2dPropertyIdFill* id, const wxString& WXUNUSED( value ) )
{
    return 0;
}

bool a2dFillProperty::NeedsSave()
{
    if ( m_value.IsNoFill() ) //layer stroke, same is no fill property at all at load.
        return false;
/*
    // next is not save, because a lyers fill will be used if not saved.
    a2dFillStyle style = m_value.GetStyle();
    if ( style == a2dFILL_TRANSPARENT )
        return false;

    switch ( m_value.GetType() )
    {
        case a2dFILL_NULL:
            return false;
    }
*/
    return true;
}

#if wxART2D_USE_CVGIO
void a2dFillProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        m_value.Save( parent, out, towrite );
    }
}

void a2dFillProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        m_value.Load( parent, parser );
    }
}
#endif //wxART2D_USE_CVGIO

void a2dFillProperty::SetValue( const a2dFill& value )
{
    m_value = value;
}

wxString a2dFillProperty::StringRepresentation() const
{
    return wxT( "VOID" );
}

wxString a2dFillProperty::StringValueRepresentation() const
{
    return wxT( "VOID" );
}

//----------------------------------------------------------------------------
// a2dStrokeProperty
//----------------------------------------------------------------------------

a2dStrokeProperty::a2dStrokeProperty(): a2dNamedProperty()
{
}

a2dStrokeProperty::a2dStrokeProperty( const a2dPropertyIdStroke* id, const a2dStroke& value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dStrokeProperty::~a2dStrokeProperty()
{
}

a2dStrokeProperty::a2dStrokeProperty( const a2dStrokeProperty* ori )
    : a2dNamedProperty( *ori )
{
    m_value = ori->m_value;
}

a2dStrokeProperty::a2dStrokeProperty( const a2dStrokeProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dStrokeProperty::Clone( a2dObject::CloneOptions WXUNUSED( options ) ) const
{
    return new a2dStrokeProperty( *this );
};

void a2dStrokeProperty::Assign( const a2dNamedProperty& other )
{
    a2dStrokeProperty* propcast = wxStaticCast( &other, a2dStrokeProperty );
    m_value = propcast->m_value;
}

a2dStrokeProperty* a2dStrokeProperty::CreatePropertyFromString( const a2dPropertyIdStroke* id, const wxString& WXUNUSED( value ) )
{
    return 0;
}

bool a2dStrokeProperty::NeedsSave()
{
    if ( m_value.IsNoStroke() ) //layer stroke, same is no fill property at all at load.
        return false;
/*
    // must save else layer fill.
    a2dStrokeStyle style = m_value.GetStyle();
    if ( style == a2dSTROKE_TRANSPARENT )
        return false;

    switch ( m_value.GetType() )
    {
        case a2dSTROKE_NULLSTROKE:
            return false;
    }
*/
    return true;
}

#if wxART2D_USE_CVGIO
void a2dStrokeProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        m_value.Save( parent, out, towrite );
    }
}

void a2dStrokeProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        m_value.Load( parent, parser );
    }
}
#endif //wxART2D_USE_CVGIO

void a2dStrokeProperty::SetValue( const a2dStroke& value )
{
    m_value = value;
}

wxString a2dStrokeProperty::StringRepresentation() const
{
    return wxT( "VOID" );
}

wxString a2dStrokeProperty::StringValueRepresentation() const
{
    return wxT( "VOID" );
}

//----------------------------------------------------------------------------
// a2dFontProperty
//----------------------------------------------------------------------------

a2dFontProperty::a2dFontProperty(): a2dNamedProperty()
{
}

a2dFontProperty::a2dFontProperty( const a2dPropertyIdFont* id, const a2dFont& value )
    : a2dNamedProperty( id )
{
    m_value = value;
}

a2dFontProperty::a2dFontProperty( const a2dFontProperty* ori )
    : a2dNamedProperty( *ori )
{
    m_value = ori->m_value;
}

a2dFontProperty::~a2dFontProperty()
{
}

a2dFontProperty::a2dFontProperty( const a2dFontProperty& other )
    : a2dNamedProperty( other )
{
    m_value = other.m_value;
}

a2dNamedProperty* a2dFontProperty::Clone( a2dObject::CloneOptions WXUNUSED( options ) ) const
{
    return new a2dFontProperty( *this );
};

void a2dFontProperty::Assign( const a2dNamedProperty& other )
{
    a2dFontProperty* propcast = wxStaticCast( &other, a2dFontProperty );
    m_value = propcast->m_value;
}

a2dFontProperty* a2dFontProperty::CreatePropertyFromString( const a2dPropertyIdFont* id, const wxString& WXUNUSED( value ) )
{
    return 0;
}

#if wxART2D_USE_CVGIO
void a2dFontProperty::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dNamedProperty::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        //m_value.Save( parent, out, towrite );
    }
}

void a2dFontProperty::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dNamedProperty::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
        //m_value.Load( parent, parser );
    }
}
#endif //wxART2D_USE_CVGIO

void a2dFontProperty::SetValue( const a2dFont& value )
{
    m_value = value;
}

wxString a2dFontProperty::StringRepresentation() const
{
    return wxT( "VOID" );
}

wxString a2dFontProperty::StringValueRepresentation() const
{
    return wxT( "VOID" );
}



//IMPLEMENT_DYNAMIC_CLASS(a2dCanvasObjectStroke, a2dOneColourStroke)


/**************************************************
a2dCanvasObjectStroke
**************************************************/
/*
a2dCanvasObjectStroke::a2dCanvasObjectStroke(): a2dOneColourStroke()
{
    m_distance = 0;
    m_style = a2dSTROKE_OBJECTS;
    SetStyle( m_style );
    m_obj = 0;
}

a2dCanvasObjectStroke::a2dCanvasObjectStroke( a2dCanvasObject* object ): a2dOneColourStroke()
{
    m_distance = 0;
    m_style = a2dSTROKE_OBJECTS;
    SetStyle( m_style );
    wxASSERT_MSG( object != 0 , wxT("a2dCanvasObject for a2dCanvasObjectStroke may not be Null") );
    m_obj = object;
}

a2dCanvasObjectStroke::a2dCanvasObjectStroke( a2dCanvasObject* object, const wxColour& col, float width, a2dStrokeStyle style )
            :a2dOneColourStroke( col, width, style )
{
    m_distance = 0;
    m_style = a2dSTROKE_OBJECTS;
    SetStyle( m_style );
    wxASSERT_MSG( object != 0 , wxT("a2dCanvasObject for a2dCanvasObjectStroke may not be Null") );
    m_obj = object;
}

a2dCanvasObjectStroke::a2dCanvasObjectStroke( a2dCanvasObject* object, const wxColour& col, int width, a2dStrokeStyle style)
            :a2dOneColourStroke( col, width, style )
{
    m_distance = 0;
    m_style = a2dSTROKE_OBJECTS;
    SetStyle( m_style );
    wxASSERT_MSG( object != 0 , wxT("a2dCanvasObject for a2dCanvasObjectStroke may not be Null") );
    m_obj = object;
}

a2dCanvasObjectStroke::a2dCanvasObjectStroke( a2dCanvasObject* object, const wxPen& stroke )
            :a2dOneColourStroke( stroke )
{
    m_distance = 0;
    m_style = a2dSTROKE_OBJECTS;
    SetStyle( m_style );
    wxASSERT_MSG( object != 0 , wxT("a2dCanvasObject for a2dCanvasObjectStroke may not be Null") );
    m_obj = object;
}


a2dCanvasObjectStroke::~a2dCanvasObjectStroke()
{
}

a2dObject* a2dCanvasObjectStroke::Clone( CloneOptions WXUNUSED(options) ) const
{
    return new a2dCanvasObjectStroke( *this );
}

a2dCanvasObjectStroke::a2dCanvasObjectStroke( const a2dCanvasObjectStroke& stroke ): a2dOneColourStroke( stroke )
{
    m_distance = stroke.m_distance;
    m_obj = stroke.m_obj;
    m_style = a2dSTROKE_OBJECTS;
    SetStyle( m_style );
}

#if wxART2D_USE_CVGIO
void a2dCanvasObjectStroke::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dStroke::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        if ( this == a2dINHERIT_STROKE )
        {
            out.WriteAttribute( wxT("style"), wxT("inherit") );
        }
        else
        {
        }
    }
    else
    {
    }
}

void a2dCanvasObjectStroke::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dStroke::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void a2dCanvasObjectStroke::Render( a2dIterC& ic, a2dDrawer2D* drawer, const a2dBoundingBox& WXUNUSED(clipbox), int n, wxPoint points[] )
{
    if ( m_distance == 0 )
    {
        int i;
        double x,y;
        for (i = 0; i < n; i++)
        {
            x = drawer->DeviceToWorldX( points[i].x );
            y = drawer->DeviceToWorldY( points[i].y );
            a2dAffineMatrix relative;
            relative.Translate( x, y );

            a2dIterCU cu( ic, relative );

            m_obj->Render( ic, _ON );
        }
    }
    else
    { //at every distance further around the contour place the object
        int i;
        double x,y;
        double prevx,prevy;
        double done = 0;

        prevx = drawer->DeviceToWorldX( points[0].x );
        prevy = drawer->DeviceToWorldY( points[0].y );

        for (i = 0; i < n-1; i++)
        {

            x = drawer->DeviceToWorldX( points[i+1].x );
            y = drawer->DeviceToWorldY( points[i+1].y );

            double dx =(x-prevx);
            double dy =(y-prevy);

            double lseg = sqrt( dx*dx + dy*dy );

            while ( done <= lseg  && lseg)
            {
                x = prevx + done/lseg * dx ;
                y = prevy + done/lseg * dy ;
                a2dAffineMatrix relative;
                relative.Translate( x, y );
                a2dIterC ic( ic.GetCanvasView() );
                a2dIterCU cu( ic, relative );

                m_obj->Render( ic, _ON );
                done += m_distance;
            }

            done = done - lseg;
            prevx = drawer->DeviceToWorldX( points[i+1].x );
            prevy = drawer->DeviceToWorldY( points[i+1].y );
        }
    }
//
    drawer->DeviceDrawLines(n, points, spline );
    int i;
    double x,y;
    for (i = 0; i < n; i++)
    {
        x = drawer->DeviceToWorldX( points[i].x );
        y = drawer->DeviceToWorldY( points[i].y );
        drawer->SetDrawerStroke( a2dTRANSPARENT_STROKE );
        drawer->SetDrawerContour( wxLAYER_CONTOUR );
        a2dFill* localfill = a2dFill( wxColour(1,233,5 ));
        drawer->SetDrawerFill( localfill );
        drawer->DrawCircle( points[i].x, points[i].y, drawer->WorldToDeviceXRel(22) );
        delete localfill;
    }
//
}
*/
/*
float a2dCanvasObjectStroke::GetExtend() const
{
    a2dBoundingBox box;
    box = m_obj->GetBbox();

    double xdisttozero = wxMax( fabs(box.GetMinX()),fabs(box.GetMaxX()) );
    double ydisttozero = wxMax( fabs(box.GetMinY()),fabs(box.GetMaxY()) );
    double max = sqrt ( xdisttozero*xdisttozero + ydisttozero*ydisttozero);
    return max;
}

*/




