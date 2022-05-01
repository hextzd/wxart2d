/*! \file curves/src/axis.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: axis.cpp,v 1.23 2009/07/24 16:35:01 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/wfstream.h>
#include <math.h>

#include "wx/canvas/eval.h"

#include "wx/curves/meta.h"

#include "wx/canvas/canobj.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/canvas.h"

#if wxART2D_USE_EDITOR
#include "wx/canvas/edit.h"
#endif //wxART2D_USE_EDITOR

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dTicFormatter, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dTimeTicFormatter, a2dTicFormatter )
IMPLEMENT_DYNAMIC_CLASS( a2dCurveAxis, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dCurveAxisLin, a2dCurveAxis )
#if 0
IMPLEMENT_DYNAMIC_CLASS( a2dCurveAxisArea, a2dCurveObject )
#endif

//----------------------------------------------------------------------------
// a2dTicFormatter
//----------------------------------------------------------------------------

a2dTicFormatter::a2dTicFormatter()
    : a2dCanvasObject()
{
    m_format = _T( "%3.0f" );
}

a2dTicFormatter::~a2dTicFormatter()
{
}

a2dTicFormatter::a2dTicFormatter( const a2dTicFormatter& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_format = other.m_format;
}

a2dObject* a2dTicFormatter::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dTicFormatter( *this, options, refs );
}


wxString a2dTicFormatter::GetTicText( double dTicValue ) const
{
    wxString    strTicText;

    strTicText.Printf( m_format, dTicValue );
    return strTicText;
}


a2dTimeTicFormatter::a2dTimeTicFormatter()
    : a2dTicFormatter()
{
    m_format.empty();
}

a2dTimeTicFormatter::~a2dTimeTicFormatter()
{
}

void a2dTimeTicFormatter::SetTicFormat( const wxString& format )
{
    wxASSERT_MSG( format.IsEmpty(), wxT( "format string not supported" ) );
}

wxString a2dTimeTicFormatter::GetTicText( double dTicValue )
{
    wxString    strTicText;
    wxString    str;
    wxChar      cSeparator = _T( '\0' );
    int         i, nDivisor[] = { 60, 60, 24 };
    int         i0 = 0;
    wxUint32    ulTic = ( wxUint32 ) dTicValue;

    // converts dTicValue, which is the elapsed time in seconds, to a
    // d:mm:hh:ss formatted string. the implementation is iterative and
    // runs from ss to d (right to left).

    // no seconds please!
    i0 = 1, ulTic /= nDivisor[0];

    strTicText.empty();
    // ss, mm, hh
    for ( i = i0; i < 3; i++ )
    {
        str.Printf( _T( "%02lu%c" ), ulTic % nDivisor[i], cSeparator );
        strTicText = str + strTicText;
        ulTic /= nDivisor[i];
        cSeparator = _T( ':' );
    }
    // d (days) - optional
    if ( ulTic > 0 )
    {
        str.Printf( _T( "%lu%c" ), ulTic, cSeparator );
        strTicText = str + strTicText;
    }
    return strTicText;
}



//----------------------------------------------------------------------------
// a2dCurveAxis
//----------------------------------------------------------------------------
/*
const a2dPropertyIdRefObject a2dCurveAxis::PROPID_stroketic( CLASSNAME( a2dCurveAxis ), wxT("stroketic"), a2dPropertyId::flag_none, 0 );
const a2dPropertyIdRefObject a2dCurveAxis::PROPID_stroketictext( CLASSNAME( a2dCurveAxis ), wxT("stroketictext"), a2dPropertyId::flag_none, 0 );
const a2dPropertyIdRefObject a2dCurveAxis::PROPID_strokeaxis( CLASSNAME( a2dCurveAxis ), wxT("strokeaxis"), a2dPropertyId::flag_none, 0 );
*/

a2dCurveAxis::a2dCurveAxis( double length, bool yaxis )
    : a2dCurveObject()
{
    m_length = length;
    m_yaxis = yaxis;
    m_showtics = true;
    m_tic = 0;
    m_ticheight = 0;
    m_stroketic = *a2dBLACK_STROKE;
    m_stroketictext = *a2dBLACK_STROKE;
    m_strokeunits = *a2dBLACK_STROKE;

    SetStroke( *a2dBLACK_STROKE );

    m_sidetic = true;//false;
    m_inverttic = false;
    m_autosizedtic = false;

    m_min = 0;
    m_max = 1000;
    m_ticmin = m_min;
    m_ticmax = m_max;
    m_position = 0;

    m_pTicFormatter = new a2dTicFormatter();
    m_pTicFormatter->SetAxis( this );

    m_font = a2dFont( 6, wxFONTFAMILY_SWISS,  wxFONTSTYLE_ITALIC );
}

a2dCurveAxis::~a2dCurveAxis()
{
}

a2dObject* a2dCurveAxis::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCurveAxis( *this, options, refs );
};

a2dCurveAxis::a2dCurveAxis( const a2dCurveAxis& other, CloneOptions options, a2dRefMap* refs )
    : a2dCurveObject( other, options, refs )
{
    m_showtics = other.m_showtics;
    m_tic = other.m_tic;
    m_ticheight = other.m_ticheight;
    if( options & clone_members )
    {
        m_stroketic = other.m_stroketic;
        m_stroketictext = other.m_stroketictext;
        m_strokeunits = other.m_strokeunits;
        m_pTicFormatter = ( a2dTicFormatter* ) other.m_pTicFormatter->Clone( CloneOptions( options & ~ clone_seteditcopy ) );
        m_font = other.m_font;
    }
    else
    {
        m_stroketic = other.m_stroketic;
        m_stroketictext = other.m_stroketictext;
        m_strokeunits = other.m_strokeunits;
        m_pTicFormatter = other.m_pTicFormatter;
        m_font = other.m_font;
    }
    m_units = other.m_units;
    m_length = other.m_length;
    m_min = other.m_min;
    m_max = other.m_max;
    m_ticmin = other.m_ticmin;
    m_ticmax = other.m_ticmax;
    m_yaxis = other.m_yaxis;

    m_sidetic = other.m_sidetic;
    m_inverttic = other.m_inverttic;
    m_position = other.m_position;
    m_autosizedtic = other.m_autosizedtic;
    m_commonTicFormat = other.m_commonTicFormat;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in property->TClone()
    CurrentSmartPointerOwner = this;
#endif

}

void a2dCurveAxis::SetBoundaries( double min, double max )
{
    if( m_min != min || m_max != max )
    {
        if ( m_childobjects != wxNullCanvasObjectList )
        {
            forEachIn( a2dCanvasObjectList, m_childobjects )
            {
                a2dCanvasObject* obj = *iter;
                double curlength = m_length != 0 ? m_length : 1.;
                if( m_yaxis )
                {
                    obj->SetPosXY( 0, obj->GetPosY() / curlength * ( m_max - m_min ) + m_min );
                    double val = obj->GetPosY();
                    if( val < wxMin( min, max ) || val > wxMax( min, max ) )
                        obj->SetVisible( false );
                    else
                        obj->SetVisible( true );
                    obj->SetPosXY( 0, ( obj->GetPosY() - min ) / ( max - min )*curlength );
                }
                else
                {
                    obj->SetPosXY( obj->GetPosX() / curlength * ( m_max - m_min ) + m_min, 0 );
                    double val = obj->GetPosX();
                    if( val < wxMin( min, max ) || val > wxMax( min, max ) )
                        obj->SetVisible( false );
                    else
                        obj->SetVisible( true );
                    obj->SetPosXY( ( obj->GetPosX() - min ) / ( max - min )*curlength, 0 );
                }
            }
        }
        if( m_min == m_ticmin && m_max == m_ticmax )
        {
            m_ticmin = min;
            m_ticmax = max;
        }
        m_min = min;
        m_max = max;
        SetPending( true );
    }
}

void a2dCurveAxis::SetTicBoundaries( double min, double max )
{
    if ( m_ticmin != min || m_ticmax != max )
    {
        m_ticmin = min;
        m_ticmax = max;
        SetPending( true );
    }
}

void a2dCurveAxis::SetLength( double length )
{
    if ( m_length != length )
    {
        if ( m_childobjects != wxNullCanvasObjectList )
        {
            forEachIn( a2dCanvasObjectList, m_childobjects )
            {
                a2dCanvasObject* obj = *iter;
                double curlength = m_length != 0 ? m_length : 1.;
                double newlength = length != 0 ? length : 1.;
                if ( m_yaxis )
                {
                    obj->SetPosXY( 0, obj->GetPosY() / curlength * ( m_max - m_min ) + m_min );
                    obj->SetPosXY( 0, ( obj->GetPosY() - m_min ) / ( m_max - m_min )*newlength );
                }
                else
                {
                    obj->SetPosXY( obj->GetPosX() / curlength * ( m_max - m_min ) + m_min, 0 );
                    obj->SetPosXY( ( obj->GetPosX() - m_min ) / ( m_max - m_min )*newlength, 0 );
                }
            }
        }
        m_length = length;
        SetPending( true );
    }
}

#if wxART2D_USE_CVGIO
void a2dCurveAxis::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCurveObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute(  wxT( "min" ) , m_min );
        out.WriteAttribute( wxT( "max" ) , m_max );
        out.WriteAttribute(  wxT( "ticmin" ) , m_ticmin );
        out.WriteAttribute( wxT( "ticmax" ) , m_ticmax );
        out.WriteAttribute( wxT( "yaxis" ) , m_yaxis );
        out.WriteAttribute( wxT( "showtics" ) , m_showtics );
        out.WriteNewLine();
        out.WriteAttribute( wxT( "font" ), m_font.CreateString() );
        out.WriteNewLine();

        out.WriteAttribute( wxT( "tic" ) , m_tic );
        out.WriteAttribute( wxT( "ticheight" ) , m_ticheight );
        out.WriteAttribute( wxT( "sidetic" ) , m_sidetic );
        out.WriteNewLine();
        out.WriteAttribute( wxT( "position" ) , m_position );
        out.WriteAttribute( wxT( "inverttic" ) , m_inverttic );
        out.WriteAttribute( wxT( "autosizedtic" ) , m_autosizedtic );
        out.WriteNewLine();
        out.WriteAttribute( wxT( "units" ) , m_units );
        out.WriteAttribute( wxT( "commonFormat" ), m_commonTicFormat );

        out.WriteNewLine();
    }
    else
    {
        out.WriteStartElement( wxT( "derived" ) );

        m_stroketic.Save( this, out, towrite );
        m_stroketictext.Save( this, out, towrite );
        m_strokeunits.Save( this, out, towrite );

        m_pTicFormatter->Save( this, out, towrite );

        out.WriteEndElement();
    }
}

void a2dCurveAxis::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCurveObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_min = parser.RequireAttributeValueDouble( wxT( "min" ) );
        m_max = parser.RequireAttributeValueDouble( wxT( "max" ) );
        m_ticmin = parser.RequireAttributeValueDouble( wxT( "ticmin" ) );
        m_ticmax = parser.RequireAttributeValueDouble( wxT( "ticmax" ) );
        m_yaxis = parser.RequireAttributeValueBool( wxT( "yaxis" ) );
        m_showtics = parser.RequireAttributeValueBool( wxT( "showtics" ) );
        a2dFontInfo fontinfo;
        fontinfo.ParseString( parser.RequireAttributeValue( wxT( "font" ) ) );
        m_font = a2dFont::CreateFont( fontinfo );

        m_tic = parser.RequireAttributeValueDouble( _T( "tic" ) );
        m_ticheight = parser.RequireAttributeValueDouble( wxT( "ticheight" ) );
        m_sidetic = parser.RequireAttributeValueBool( wxT( "sidetic" ) );
        m_position = parser.GetAttributeValueDouble( wxT( "position" ) );
        m_inverttic = parser.GetAttributeValueBool( wxT( "inverttic" ) );
        m_autosizedtic = parser.GetAttributeValueBool( wxT( "autosizedtic" ) );

        m_units = parser.GetAttributeValue( wxT( "units" ) );
        m_commonTicFormat = parser.GetAttributeValue( wxT( "commonTicFormat" ) );
    }
    else
    {
        parser.Require( START_TAG, wxT( "derived" ) );
        parser.Next();

        // new strokes, since the current ones might be shared by other objects too.
        m_stroketic.UnRef();
        m_stroketictext.UnRef();
        m_strokeunits.UnRef();

        m_stroketic.Load( parent, parser );
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_stroketic );
        m_stroketictext.Load( parent, parser );
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_stroketictext );
        m_strokeunits.Load( parent, parser );
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_strokeunits );

        m_pTicFormatter->Load( parent, parser );
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_pTicFormatter );

        parser.Require( END_TAG, wxT( "derived" ) );
        parser.Next();
    }
}
#endif //wxART2D_USE_CVGIO

void a2dCurveAxis::SetTicStroke( const a2dStroke& stroke )
{
    m_stroketic = stroke;
    SetPending( true );
}

void a2dCurveAxis::SetTicTextStroke( const a2dStroke& stroke )
{
    m_stroketictext = stroke;
    SetPending( true );
}

void a2dCurveAxis::SetUnitsStroke( const a2dStroke& stroke )
{
    m_strokeunits = stroke;
    SetPending( true );
}

void a2dCurveAxis::SetUnitsStroke( const wxColour& color, double width, a2dStrokeStyle style )
{
    m_strokeunits = a2dStroke( color, width, style );
    SetPending( true );
}

void a2dCurveAxis::DoUpdateViewDependentObjects( a2dIterC& ic )
{
    a2dCanvasObject::DoUpdateViewDependentObjects( ic );
    if( m_autosizedtic )
    {
        const a2dMatrixProperty* propSpec = PROPID_IntViewDependTransform->GetPropertyListOnly( this );
        if ( propSpec )
        {
            int clientw, clienth;
            ic.GetDrawingPart()->GetDisplayWindow()->GetClientSize( &clientw, &clienth );

            if ( clientw == 0 ) clientw = 10;
            if ( clienth == 0 ) clienth = 10;

            double w = fabs( ic.GetDrawer2D()->DeviceToWorldXRel( clientw ) );
            double h = fabs( ic.GetDrawer2D()->DeviceToWorldYRel( clienth ) );
            double relw = ic.GetInverseTransform().TransformDistance( w );
            double relh = ic.GetInverseTransform().TransformDistance( h );
            /* another way
                        double worldtic;
                        if(m_yaxis)
                        {
                            if ( clienth == 0 ) clienth = 10;
                            double h = fabs(ic.GetDrawer2D()->DeviceToWorldYRel(clienth));
                            double relh = h*ic.GetInverseTransform().Get_scaleY();
                            worldtic = relh/m_length*15;
                        }
                        else
                        {
                            if ( clientw == 0 ) clientw = 10;
                            double w = fabs(ic.GetDrawer2D()->DeviceToWorldXRel(clientw));
                            double relw = w*ic.GetInverseTransform().Get_scaleX();
                            worldtic = relw/m_length*15;
                        }
            */

            double worldtic = wxMin( relw, relh ) / m_length * 15;
            double tic = worldtic / m_length * ( m_max - m_min );
            if( tic > 10 ) tic = int( tic + 0.5 );

            if( m_tic != tic )
            {
                m_tic = tic;
                SetPending( true );
            }
        }
    }
}

bool a2dCurveAxis::HasLevelMarkers() const
{
    a2dCanvasObject* pObject = NULL;
    a2dAxisMarker* pMarker = NULL;

    const_forEachIn( a2dCanvasObjectList, GetChildObjectList() )
    {
        pObject = *iter;
        pMarker = wxDynamicCast( pObject, a2dAxisMarker );
        if ( pMarker )
            return TRUE;
    }
    return FALSE;
}

a2dBoundingBox a2dCurveAxis::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dBoundingBox bbox;

#if 0
    // tictextheightX and tictextheigtY are not longer used.
    // this may be an error since the scaling is not respected.
    // should be investigated! FIXME
    double tictextheight = GetTicTextHeight();
    double tictextheightX = tictextheight;
    double tictextheightY = tictextheight;

    a2dNamedProperty* propSpec = PROPID_intViewDependTransform->GetPropertyListOnly( this );
    if ( propSpec )
    {
        a2dMatrixProperty* propMatrix = wxStaticCast( propSpec, a2dMatrixProperty );
        double scalex = propMatrix->GetValue().Get_scaleX();
        double scaley = propMatrix->GetValue().Get_scaleY();
        tictextheightX = tictextheightX * scalex;
        tictextheightY = tictextheightY * scaley;
//        maybe it is bad for different scale by X and by Y.
//        tictextheight = propMatrix->GetValue().TransformDistance(tictextheight);
    }
#endif

    a2dAffineMatrix internalTransform;
    const a2dNamedProperty* propSpec = PROPID_IntViewDependTransform->GetPropertyListOnly( this );
    if ( propSpec )
    {
        a2dMatrixProperty* propMatrix = wxStaticCast( propSpec, a2dMatrixProperty );
        internalTransform = propMatrix->GetValue();
    }
    double length = GetLength();

    double sizeticstroke = m_stroketic.GetWidth();
    double ticheight = GetTicHeight();
    int hasLevels = HasLevelMarkers() ? 1 : 0;

    if( !internalTransform.IsIdentity() )
    {
        double scalex = internalTransform.Get_scaleX();
        double scaley = internalTransform.Get_scaleY();
        if ( m_yaxis )
        {
            length = length / scaley;
            sizeticstroke = sizeticstroke / scaley;
//          hasLevels = (int) (hasLevels / scalex);
        }
        else
        {
            length = length / scalex;
            sizeticstroke = sizeticstroke / scalex;
//          hasLevels = (int) (hasLevels / scaley);
        }
    }

    if( m_showtics )
    {
        wxString ticstr = m_commonTicFormat.IsEmpty() ?
                          m_pTicFormatter->GetTicText( - wxMax( fabs( m_min ), fabs( m_max ) ) )
                          :   wxString::Format( m_commonTicFormat.c_str(), - wxMax( fabs( m_min ), fabs( m_max ) ) );
        // bboxtext alignment depends on axis direction and flags! See below!

//        int hasLevels = HasLevelMarkers() ? 1 : 0;

        // Strategy: we calculate a complete tic-bbox, consisting of line and text
        // at xy = 0 and expand bbox. Then we translate the tic-bbox to m_length and
        // expand bbox again.
        double xy = 0;
        double transX = 0, transY = 0;
//        double sizeticstroke = m_stroketic->GetWidth();

        a2dBoundingBox bboxtic;
        a2dBoundingBox bboxunits;

        a2dBoundingBox bboxtext;
        a2dBoundingBox bboxline;

        if ( m_sidetic )
        {
            // Line 1:1 (or 2:1 with markers) on both axis sides.
            // Text center on tic
            double ticsize = ticheight * ( 1.1 + hasLevels );
            double textticpos = ticheight * ( 1.2 + hasLevels );
            if ( m_yaxis )
            {
                bboxtext = m_font.GetTextExtent( ticstr,  wxMIDY | ( m_inverttic ? wxMINX : wxMAXX ) );
                if ( m_inverttic )
                {
                    bboxline = a2dBoundingBox( -ticheight, xy, ticsize, xy );
                    bboxtext.Translate( textticpos, xy );
                }
                else
                {
                    bboxline = a2dBoundingBox( -ticsize, xy, ticheight, xy );
                    bboxtext.Translate( -textticpos, xy );
                }
                transX = 0, transY = length;
            }
            else
            {
                bboxtext = m_font.GetTextExtent( ticstr,  wxMIDX | ( m_inverttic ? wxMINY : wxMAXY ) );
                if ( m_inverttic )
                {
                    bboxline = a2dBoundingBox( xy, -ticheight, xy, ticsize );
                    bboxtext.Translate( xy, textticpos );
                }
                else
                {
                    bboxline = a2dBoundingBox( xy, -ticsize, xy, ticheight );
                    bboxtext.Translate( xy, -textticpos );
                }
                transX = length, transY = 0;
            }
            bboxtic.Expand( bboxline );
            bboxtic.Expand( bboxtext );

            if ( !m_units.IsEmpty() )
            {
                if ( m_yaxis )
                {
                    bboxunits = m_font.GetTextExtent( m_units,  ( m_inverttic ? wxMINX : wxMAXX ) );
                    if ( m_inverttic )
                        bboxunits.Translate( textticpos, m_max > m_min ? length : 0 );
                    else
                        bboxunits.Translate( -textticpos, m_max > m_min ? length : 0 );
                }
                else
                {
                    bboxunits = m_font.GetTextExtent( m_units,  ( m_inverttic ? wxMINY : wxMAXY ) | ( m_max > m_min ? wxMINX : wxMAXX ) );
                    if ( m_inverttic )
                        bboxunits.Translate( m_max > m_min ? length : 0, textticpos );
                    else
                        bboxunits.Translate( m_max > m_min ? length : 0, -textticpos );
                }
            }

        }
        else // ==> !m_sidetic
        {
            // Line on one side of the axis.
            // Text above the tic for Y-axis and beside for X-Axis. Not sure about this but sample look like that.
            // we use sizeticstroke+1 to get a better separation of line and text
            double ticsize = ticheight * ( 2 + hasLevels );
            double textticpos = ticheight * ( 0.5 + hasLevels );
            if ( m_yaxis )
            {
                bboxtext = m_font.GetTextExtent( ticstr,  wxMINY | ( m_inverttic ? wxMINX : wxMAXX ) );
                if ( m_inverttic )
                {
                    bboxline = a2dBoundingBox( 0, xy, ticsize, xy );
                    bboxtext.Translate( textticpos, xy + sizeticstroke ); //klion +1
                }
                else
                {
                    bboxline = a2dBoundingBox( -ticsize, xy, 0, xy );
                    bboxtext.Translate( -textticpos, xy + sizeticstroke ); //klion +1
                }
                transX = 0, transY = length;
                if( !m_units.IsEmpty() )
                    transY -= bboxtext.GetHeight();
            }
            else
            {
                bboxtext = m_font.GetTextExtent( ticstr, wxMINX | ( m_inverttic ? wxMINY : wxMAXY ) );
                if ( m_inverttic )
                {
                    bboxline = a2dBoundingBox( xy, ticsize, xy, 0  );
                    bboxtext.Translate( xy + sizeticstroke, textticpos ); //klion +1
                }
                else
                {
                    bboxline = a2dBoundingBox( xy, -ticsize, xy, 0 );
                    bboxtext.Translate( xy + sizeticstroke, -textticpos ); //klion +1
                }
                transX = length, transY = 0;
                if( !m_units.IsEmpty() )
                    transX -= bboxtext.GetWidth();
            }
            bboxtic.Expand( bboxline );
            bboxtic.Expand( bboxtext );

            if ( !m_units.IsEmpty() )
            {
                if ( m_yaxis )
                {
                    bboxunits = m_font.GetTextExtent( m_units, wxMINY | ( m_inverttic ? wxMINX : wxMAXX ) );
                    if ( m_inverttic )
                        bboxunits.Translate( textticpos, ( m_max > m_min ? length : 0 ) + sizeticstroke );
                    else
                        bboxunits.Translate( -textticpos, ( m_max > m_min ? length : 0 ) + sizeticstroke );
                }
                else
                {
                    bboxunits = m_font.GetTextExtent( m_units, ( m_max > m_min ? wxMINX : wxMAXX ) | ( m_inverttic ? wxMINY : wxMAXY ) );
                    if ( m_inverttic )
                        bboxunits.Translate( ( m_max > m_min ? length : 0 ) - sizeticstroke, textticpos );
                    else
                        bboxunits.Translate( ( m_max > m_min ? length : 0 ) - sizeticstroke, -textticpos );
                }
            }

        }

        m_untransbboxtictext = bboxtext;
        bbox.Expand( bboxtic );
        bboxtic.Translate( transX, transY );
        bbox.Expand( bboxtic );
        bbox.Expand( bboxunits );

    }
    else
    {
        bbox.Expand( 0, 0 );
        if ( m_yaxis )
            bbox.Expand( 0, length );
        else
            bbox.Expand( length, 0 );
    }

    if( !internalTransform.IsIdentity() )
    {
        bbox.MapBbox( internalTransform );
    }

    return bbox;
}

double a2dCurveAxis::ConvertWorld2Axis( double RelativeWorldValue )
{
    double curlength = m_length != 0 ? m_length : 1.;
    return RelativeWorldValue / curlength * ( m_max - m_min ) +  m_min;
}

double a2dCurveAxis::ConvertAxis2World( double AxisValue )
{
    double curlength = m_length != 0 ? m_length : 1.;
    return ( AxisValue - m_min ) / ( m_max - m_min ) * curlength;
}

void a2dCurveAxis::AddLevel( const wxString& levelName, double val, a2dCanvasObject* object )
{
    if ( !levelName.IsEmpty() )
        object->SetName( levelName );
    if ( m_inverttic )
    {
        object->Mirror( !m_yaxis, m_yaxis );
        a2dBaseMarker* aMarker = wxDynamicCast( object, a2dBaseMarker );
        if ( aMarker )
        {
            a2dText* aText = aMarker->GetPrompt();
            if ( aText )
                aText->Mirror(); //aText->Mirror(m_yaxis,!m_yaxis);
        }
    }
    Append( object );
    a2dBaseMarker* aMarker = wxDynamicCast( object, a2dBaseMarker );
    if( aMarker )
        aMarker->SetPosition( val );

    if ( val < wxMin( m_min, m_max ) || val > wxMax( m_min, m_max ) )
        object->SetVisible( false );
    else
        object->SetVisible( true );
    if ( m_yaxis )
        object->SetPosXY( 0, ConvertAxis2World( val ) );
    else
        object->SetPosXY( ConvertAxis2World( val ), 0 );
    SetPending( true );
}

a2dCanvasObject* a2dCurveAxis::GetLevel( const wxString& levelName )
{
    a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin();
    while( iter != GetChildObjectList()->end() )
    {
        if ( ( *iter )->GetName() == levelName )
            return ( a2dCanvasObject* ) ( *iter ).Get();
        ++iter;
    }
    return NULL;
}

bool a2dCurveAxis::GetLevelValue( const wxString& levelName, double& val )
{
    a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin();
    while( iter != GetChildObjectList()->end() )
    {
        if ( ( *iter )->GetName() == levelName )
        {
            a2dCanvasObject* obj = ( ( a2dCanvasObject* ) ( *iter ).Get() );
            if ( m_yaxis )
                val = ConvertWorld2Axis( obj->GetPosY() );
            else
                val = ConvertWorld2Axis( obj->GetPosX() );
            return true;
        }
        ++iter;
    }
    return false;
}

void a2dCurveAxis::SetInvertTic( bool inverttic )
{
    if( m_inverttic != inverttic )
    {
        MirrorLevels();
        m_inverttic = inverttic;
        SetPending( true );
    }
}

void a2dCurveAxis::MirrorLevels()
{
    if ( GetChildObjectList() != wxNullCanvasObjectList )
    {
        forEachIn( a2dCanvasObjectList, GetChildObjectList() )
        {
            a2dCanvasObject* obj = *iter;
            obj->Mirror( !m_yaxis, m_yaxis );
            a2dBaseMarker* aMarker = wxDynamicCast( obj, a2dBaseMarker );
            if ( aMarker )
            {
                a2dText* aPrompt = aMarker->GetPrompt();
                if ( aPrompt )
                    aPrompt->Mirror(); // aPrompt->Mirror(m_yaxis,!m_yaxis);
            }
        }
    }
}

void a2dCurveAxis::SetFormatter( a2dTicFormatter* pFormatter )
{
    m_pTicFormatter = pFormatter;
    m_pTicFormatter->SetAxis( this );
    SetPending( TRUE );
}

double a2dCurveAxis::GetTicStart() const
{
    double tic = GetTic();
    double ticstart = floor( m_min / tic + 0.5 ) * tic;
    if ( ( tic > 0. && ticstart <= m_min ) || ( tic < 0. && ticstart >= m_min ) )
        ticstart += tic;
    return ticstart;
}



//----------------------------------------------------------------------------
// a2dCurveAxisLin
//----------------------------------------------------------------------------

a2dCurveAxisLin::a2dCurveAxisLin( double length, bool yaxis )
    : a2dCurveAxis( length, yaxis )
{
}

a2dCurveAxisLin::~a2dCurveAxisLin()
{
}

a2dCurveAxisLin::a2dCurveAxisLin( const a2dCurveAxisLin& other, CloneOptions options, a2dRefMap* refs )
    : a2dCurveAxis( other, options, refs )
{
}

a2dObject* a2dCurveAxisLin::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCurveAxisLin( *this, options, refs );
}

#if wxART2D_USE_CVGIO
void a2dCurveAxisLin::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCurveAxis::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void a2dCurveAxisLin::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCurveAxis::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

//! expand a2dBoundingBox to x,y,w,h
#define BBOX2XYWH(bbox) (bbox).GetMinX(), (bbox).GetMinY(), (bbox).GetWidth(), (bbox).GetHeight()

void a2dCurveAxisLin::DoRender( a2dIterC& ic, OVERLAP WXUNUSED( clipparent ) )
{
    double tic;
    double ticheight;
    double tictextheight;
    double length;
    a2dAffineMatrix internalTransform;

    // For debugging
//  ic.GetDrawer2D()->SetDrawerFill(a2dWHITE_FILL);
//  ic.GetDrawer2D()->DrawRoundedRectangle(BBOX2XYWH(m_untransbbox), 0);

    double tictextsize = m_yaxis ? m_untransbboxtictext.GetHeight() : m_untransbboxtictext.GetWidth();
    tic = GetTic();
    ticheight = GetTicHeight();
    tictextheight = m_font.GetSize();
    length = GetLength();

    double sizeticstroke = m_stroketic.GetWidth();

    const a2dMatrixProperty* propSpec = PROPID_IntViewDependTransform->GetPropertyListOnly( this );
    if ( propSpec )
    {
        a2dMatrixProperty* propMatrix = wxStaticCast( propSpec, a2dMatrixProperty );
        internalTransform = propMatrix->GetValue();
    }
    /*
        if (!internalTransform.IsIdentity())
        {
            ticheight = internalTransform.TransformDistance(ticheight);
            tictextheight = internalTransform.TransformDistance(tictextheight);
        }
    */

    double x;

    if ( m_yaxis )
        ic.GetDrawer2D()->DrawLine( 0, 0, 0, m_length );
    else
        ic.GetDrawer2D()->DrawLine( 0, 0, m_length, 0 );

    if ( m_showtics )
    {
        int hasLevels = HasLevelMarkers() ? 1 : 0;

        // it is better then tictextheight and ticheight transforming.
        // it is better for different scale by X and by Y
        if ( !internalTransform.IsIdentity() )
        {
            double scalex = internalTransform.Get_scaleX();
            double scaley = internalTransform.Get_scaleY();
            if ( m_yaxis )
            {
                length = length / scaley;
                sizeticstroke = sizeticstroke / scaley;
            }
            else
            {
                length = length / scalex;
                sizeticstroke = sizeticstroke / scalex;
            }
        }
        a2dIterCU cu( ic, internalTransform );

        double ticstart;
        ic.GetDrawer2D()->SetDrawerStroke( m_stroketictext );
        ic.GetDrawer2D()->SetDrawerFill( *a2dTRANSPARENT_FILL );

        ticstart = floor( m_min / tic + 0.5 ) * tic;
        if ( ( tic > 0 && ticstart < m_min ) || ( tic < 0 && ticstart > m_min ) )
            ticstart += tic;

        ic.GetDrawer2D()->SetFont( m_font );
        if ( m_sidetic )
        {
            //add tics
            double textticpos = ticheight * ( 1.2 + hasLevels );
            double textmax = m_max; //m_units.IsEmpty() ? m_max : m_max - tic;
            for ( x = ticstart; ( tic > 0 && x <= textmax ) || ( tic < 0 && x >= textmax ); x = x + tic )
            {
                double dTicValue;
                if ( m_max != m_ticmax || m_min != m_ticmin )
                    dTicValue = ( x - m_min ) / ( m_max - m_min ) * ( m_ticmax - m_ticmin ) + m_ticmin;
                else
                    dTicValue = x;

                double xy = ( x - m_min ) / ( m_max - m_min ) * length;

                wxString ticstr = m_pTicFormatter->GetTicText( dTicValue );

                if ( m_yaxis )
                {
                    if ( m_inverttic )
                        ic.GetDrawer2D()->DrawText( ticstr, textticpos, xy, wxMINX );
                    else
                        ic.GetDrawer2D()->DrawText( ticstr, -textticpos, xy, wxMAXX );
                }
                else
                {
                    if ( m_inverttic )
                        ic.GetDrawer2D()->DrawText( ticstr, xy, textticpos, wxMINY );
                    else
                        ic.GetDrawer2D()->DrawText( ticstr, xy, -textticpos, wxMAXY );
                }
            }
            if ( !m_units.IsEmpty() )
            {
                ic.GetDrawer2D()->SetDrawerStroke( m_strokeunits );
                if ( m_yaxis )
                {
                    if ( m_inverttic )
                        ic.GetDrawer2D()->DrawText( m_units, textticpos, m_max > m_min ? length : 0, wxMINX );
                    else
                        ic.GetDrawer2D()->DrawText( m_units, -textticpos, m_max > m_min ? length : 0, wxMAXX );
                }
                else
                {
                    if( m_inverttic )
                        ic.GetDrawer2D()->DrawText( m_units, m_max > m_min ? length : 0, textticpos, wxMINY | ( m_max > m_min ? wxMINX : wxMAXX ) );
                    else
                        ic.GetDrawer2D()->DrawText( m_units, m_max > m_min ? length : 0, -textticpos, wxMAXY | ( m_max > m_min ? wxMINX : wxMAXX ) );
                }
            }

            ic.GetDrawer2D()->SetDrawerStroke( m_stroketic );

            //add tics
            double ticsize = ticheight * ( 1.1 + hasLevels );
            for ( x = ticstart; ( tic > 0 && x <= m_max ) || ( tic < 0 && x >= m_max ); x = x + tic )
            {
                double xy = ( x - m_min ) / ( m_max - m_min ) * length;
                if ( m_yaxis )
                {
                    if ( m_inverttic )
                        ic.GetDrawer2D()->DrawLine( -ticheight, xy, ticsize, xy );
                    else
                        ic.GetDrawer2D()->DrawLine( -ticsize, xy, ticheight, xy );
                }
                else
                {
                    if ( m_inverttic )
                        ic.GetDrawer2D()->DrawLine( xy, -ticheight, xy, ticsize );
                    else
                        ic.GetDrawer2D()->DrawLine( xy, -ticsize, xy, ticheight );
                }
            }
        }
        else
        {
            double textticpos = ticheight * ( 0.5 + hasLevels );
            double textmax = m_max; //m_units.IsEmpty() ? m_max : m_max - tic;
            double transformedTic = fabs( tic ) / fabs( m_max - m_min ) * length;
            bool enDrawLastTic = transformedTic > ( 2 * tictextsize + 3 * sizeticstroke );
            //add tics
            for ( x = ticstart; ( tic > 0 && x <= textmax ) || ( tic < 0 && x >= textmax ); x = x + tic )
            {
                double dTicValue;
                if ( m_max != m_ticmax || m_min != m_ticmin )
                    dTicValue = ( x - m_min ) / ( m_max - m_min ) * ( m_ticmax - m_ticmin ) + m_ticmin;
                else
                    dTicValue = x;

                double xy = ( x - m_min ) / ( m_max - m_min ) * length;

                wxString ticstr = m_pTicFormatter->GetTicText( dTicValue );

                bool notLastTic = fabs( xy - length ) > tictextsize; //fabs(x - textmax) > tictextsize; // fabs(tic/2);
                if ( m_yaxis )
                {
                    if ( m_units.IsEmpty() || notLastTic )
                    {
                        if ( m_inverttic )
                            ic.GetDrawer2D()->DrawText( ticstr, textticpos, xy + sizeticstroke, wxMINY | wxMINX );
                        else
                            ic.GetDrawer2D()->DrawText( ticstr, - textticpos, xy + sizeticstroke, wxMINY | wxMAXX );
                    }
                    else if( enDrawLastTic )
                    {
                        if ( m_inverttic )
                            ic.GetDrawer2D()->DrawText( ticstr, textticpos, xy - sizeticstroke, wxMAXY | wxMINX );
                        else
                            ic.GetDrawer2D()->DrawText( ticstr, - textticpos, xy - sizeticstroke, wxMAXY | wxMAXX );
                    }
                }
                else
                {
                    if( m_units.IsEmpty() || notLastTic )
                    {
                        if ( m_inverttic )
                            ic.GetDrawer2D()->DrawText( ticstr, xy + sizeticstroke, textticpos, wxMINX | wxMINY );
                        else
                            ic.GetDrawer2D()->DrawText( ticstr, xy + sizeticstroke, -textticpos, wxMINX | wxMAXY );
                    }
                    else if( enDrawLastTic )
                    {
                        if ( m_inverttic )
                            ic.GetDrawer2D()->DrawText( ticstr, xy - sizeticstroke, textticpos, wxMAXX | wxMINY );
                        else
                            ic.GetDrawer2D()->DrawText( ticstr, xy - sizeticstroke, -textticpos, wxMAXX | wxMAXY );
                    }
                }
            }
            if ( !m_units.IsEmpty() )
            {
                ic.GetDrawer2D()->SetDrawerStroke( m_strokeunits );
                if ( m_yaxis )
                {
                    if ( m_inverttic )
                        ic.GetDrawer2D()->DrawText( m_units, textticpos, ( m_max > m_min ? length : 0 ) + sizeticstroke , wxMINY | wxMINX );
                    else
                        ic.GetDrawer2D()->DrawText( m_units, -textticpos, ( m_max > m_min ? length : 0 ) + sizeticstroke , wxMINY | wxMAXX );
                }
                else
                {
                    if ( m_inverttic )
                        ic.GetDrawer2D()->DrawText( m_units, ( m_max > m_min ? length : 0 ) - sizeticstroke, textticpos, ( m_max > m_min ? wxMINX : wxMAXX ) | wxMINY );
                    else
                        ic.GetDrawer2D()->DrawText( m_units, ( m_max > m_min ? length : 0 ) - sizeticstroke, -textticpos, ( m_max > m_min ? wxMINX : wxMAXX ) | wxMAXY );
                }
            }

            ic.GetDrawer2D()->SetDrawerStroke( m_stroketic );

            //add tics
            double ticsize = ticheight * ( 2 + hasLevels );
            for ( x = ticstart; ( tic > 0 && x <= m_max ) || ( tic < 0 && x >= m_max ); x = x + tic )
            {
                double xy = ( x - m_min ) / ( m_max - m_min ) * length;
                if ( m_yaxis )
                {
                    if ( m_inverttic )
                        ic.GetDrawer2D()->DrawLine( 0, xy, ticsize, xy );
                    else
                        ic.GetDrawer2D()->DrawLine( -ticsize, xy, 0, xy );
                }
                else
                {
                    if ( m_inverttic )
                        ic.GetDrawer2D()->DrawLine( xy, ticsize, xy, 0 );
                    else
                        ic.GetDrawer2D()->DrawLine( xy, -ticsize, xy, 0 );
                }
            }
        }
    }
}



#if 0
//----------------------------------------------------------------------------
// a2dCurveAxisArea
//----------------------------------------------------------------------------

a2dCurveAxisArea::a2dCurveAxisArea()
    : a2dCurveObject()
{
}

a2dCurveAxisArea::~a2dCurveAxisArea()
{
}

a2dObject* a2dCurveAxisArea::Clone( CloneOptions options ) const
{
    return new a2dCurveAxisArea( *this, options );
}

a2dCurveAxisArea::a2dCurveAxisArea( const a2dCurveAxisArea& other, CloneOptions options )
    : a2dCurveObject( other, options )
{
}

a2dCurveAxis* a2dCurveAxisArea::GetAxis( const wxString axisname )
{
    a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin();
    while( iter != GetChildObjectList()->end() )
    {
        if ( ( *iter )->GetName() == axisname )
            return ( a2dCurveAxis* ) ( *iter ).Get();
        ++iter;
    }
    return NULL;
}

a2dCurveAxis* a2dCurveAxisArea::GetAxis( int nIndex )
{
    a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin();
    int i = 0;
    while( ( i <= nIndex ) && ( iter != GetChildObjectList()->end() ) )
    {
        if ( i == nIndex )
            return ( a2dCurveAxis* ) ( *iter ).Get();
        ++iter;
    }
    return NULL;
}

bool a2dCurveAxisArea::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )//WXUNUSED(mode) )
{
    a2dCurveAxis* pAxis;
    bool calc = FALSE;
    double dXPos = 0;
    double dYPos = 0;
    a2dPoint2D point;
    a2dBoundingBox bbox;

    forEachIn( a2dCanvasObjectList, GetChildObjectList() )
    {
        a2dCanvasObject* pObject = *iter;
        pAxis = wxDynamicCast( pObject, a2dCurveAxis );
        if ( pAxis )
        {
            point = pAxis->GetPosXY();
//            if ((dXPos != point.m_x) || (dYPos != point.m_y)) {
            pAxis->SetPosXY( dXPos, dYPos );
            calc = TRUE;
//            }
//            bbox = pAxis->GetBbox(); // Debug
            if ( pAxis->IsYAxis() )
                dXPos -= pAxis->GetBbox().GetWidth();
            else
                dYPos -= pAxis->GetBbox().GetHeight();
        }
    }

    if ( !m_bbox.GetValid() || calc )
    {
        m_bbox = DoGetUnTransformedBbox();
        m_bbox.MapBbox( m_lworld );

        return true;
    }

    return false;
}

a2dBoundingBox a2dCurveAxisArea::DoGetUnTransformedBbox( a2dBboxFlags flags ) const
{
    a2dCurveAxis* pAxis;
    a2dBoundingBox bbox;

    forEachIn( a2dCanvasObjectList, GetChildObjectList() )
    {
        a2dCanvasObject* pObject = *iter;
        pAxis = wxDynamicCast( pObject, a2dCurveAxis );
        if ( pAxis )
            bbox.Expand( pAxis->GetBbox() );
    }

    if ( !bbox.GetValid() ) //no child or empty childlist secure
    {
        //no objects, therefore make the bounding box the x,y of this object
        bbox.Expand( 0, 0 );
    }

    return bbox;
}

void a2dCurveAxisArea::DoRender( a2dIterC& ic, OVERLAP clipparent )
{
    bool bShowBbox = TRUE;
    a2dCurveAxis* pAxis = NULL;

    a2dCurveObject::DoRender( ic, clipparent );

    if ( bShowBbox )
    {
        ic.GetDrawer2D()->SetDrawerStroke( a2dStroke( wxColour( 0xff0000 ), 2, a2dSTROKE_SOLID ) );
        ic.GetDrawer2D()->SetDrawerFill( a2dTRANSPARENT_FILL );
        forEachIn( a2dCanvasObjectList, GetChildObjectList() )
        {
            a2dCanvasObject* pObject = *iter;
            pAxis = wxDynamicCast( pObject, a2dCurveAxis );
            if ( pAxis )
            {
                wxRect absareadev = pAxis->GetAbsoluteArea( ic );
                ic.GetDrawer2D()->PushIdentityTransform();
                ic.GetDrawer2D()->DrawRoundedRectangle( absareadev.x, absareadev.y, absareadev.width, absareadev.height, 0 );
                ic.GetDrawer2D()->PopTransform();
            }
        }
    }
}

void a2dCurveAxisArea::SetLength( double length )
{
    a2dCurveAxis* pAxis;

    if ( m_length != length )
    {
        forEachIn( a2dCanvasObjectList, GetChildObjectList() )
        {
            a2dCanvasObject* pObject = *iter;
            pAxis = wxDynamicCast( pObject, a2dCurveAxis );
            if ( pAxis )
                pAxis->SetLength( length );
        }
        m_length = length;
        SetPending( true );
    }
}
#endif

