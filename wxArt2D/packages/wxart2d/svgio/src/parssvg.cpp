/*! \file svgio/src/parssvg.cpp
    \brief a2dCanvasDocument - SVG reader via Expat
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: parssvg.cpp,v 1.109 2009/10/01 19:22:36 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "a2dprivate.h"

// This file has highly nested STL-templates, and with this you cannot get around this warning
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include "wx/wfstream.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/strconv.h"
#include <wx/tokenzr.h>

#include "wx/docview/doccom.h"
#include "wx/canvas/canmod.h"

#include "wx/svgio/parssvg.h"


using namespace std;

// ----------------------------------------------------------------------------
// a2dIOHandlerSVGIn
// ----------------------------------------------------------------------------

a2dIOHandlerSVGIn::a2dIOHandlerSVGIn()
{
    m_fill = a2dFill();
    m_stroke = *a2dNullSTROKE;
    m_docClassInfo = &a2dCanvasDocument::ms_classInfo;
}

a2dIOHandlerSVGIn::~a2dIOHandlerSVGIn()
{
}

bool a2dIOHandlerSVGIn::CanLoad( a2dDocumentInputStream& stream, const wxObject* obj, wxClassInfo* docClassInfo )
{
    if ( obj && !wxDynamicCast( obj, a2dCanvasDocument ) )
        return false;

    if ( docClassInfo && m_docClassInfo && !docClassInfo->IsKindOf( m_docClassInfo ) )
        return false;

    m_streami = &stream;
    SeekI( 0 );

    wxString header;
    char buf[1000];
    int last_read = Read( buf, 1000 );

    if( last_read > 1000 )
    {
        last_read = 1000;
    }
    header = wxString( buf, wxConvUTF8, last_read );

    SeekI( 0 );

    return ( header.Contains( _T( "<?xml" ) ) && header.Contains( _T( "<!DOCTYPE" ) ) && header.Contains( _T( "<svg" ) )
             ||
             header.Contains( _T( "<svg" ) ) //simple file
           );
}

bool a2dIOHandlerSVGIn::Load( a2dDocumentInputStream& stream , wxObject* doc )
{
    m_streami = &stream;
    m_doc = wxStaticCast( doc, a2dCanvasDocument );

    InitializeLoad();

    bool oke;

    try
    {
        oke = LoadSvg( m_doc );
    }
    catch ( const a2dIOHandlerXMLException& e )
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_XMLparse, _( "%s at line %d column %d" ),  e.getMessage().c_str(), e.getLineNumber(), e.getColumnNumber() );
        oke = false;
    }

    ResetLoad();

    return oke;
}

#ifdef new
#undef new
#endif

#include <wx/hashmap.h>

#if defined(__WXDEBUG__) && wxUSE_GLOBAL_MEMORY_OPERATORS && wxUSE_DEBUG_NEW_ALWAYS
#define new new(__TFILE__,__LINE__)
#endif

WX_DECLARE_STRING_HASH_MAP( wxString,  StringStringHashMap );
static StringStringHashMap m_shapeclass;
static wxString m_defaultclass;
static double m_defaultfontsize;

bool a2dIOHandlerSVGIn::LoadSvg( a2dCanvasDocument* doc )
{
    Next();
    Require( START_TAG, _T( "svg" ) );

    m_defaultclass = GetAttributeValue( _T( "class" ) );

    Next();
    if ( GetTagName() == _T( "title" ) )
    {
        doc->SetTitle( GetContent() );
        Next();
        Require( END_TAG, _T( "title" ) );
        Next();
    }
    if ( GetTagName() == _T( "desc" ) )
    {
        doc->SetDescription( GetContent() );
        Next();
        Require( END_TAG, _T( "desc" ) );
        Next();
    }

    a2dFill fill = m_fill;
    a2dStroke stroke = m_stroke;
    m_doc->GetDrawing()->GetRootObject()->SetFill( fill );
    m_doc->GetDrawing()->GetRootObject()->SetStroke( stroke );

    if ( GetEventType() == START_TAG && GetTagName() == _T( "g" ) )
    {
        do
        {
            LoadSvgGroup( m_doc->GetDrawing()->GetRootObject() );
            m_fill = m_doc->GetDrawing()->GetRootObject()->GetFill();
            m_stroke = m_doc->GetDrawing()->GetRootObject()->GetStroke();
        }
        while( GetTagName() != _T( "svg" ) );
    }
    else
    {
        do
        {
            LoadSvgGroupElement( m_doc->GetDrawing()->GetRootObject() );
        }
        while( GetTagName() != _T( "svg" ) );
    }
    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgGroupElement( a2dCanvasObject* g )
{
    if ( GetEventType() != START_TAG )
        return false;

    wxString elementname = GetTagName();
    if (
        elementname != _T( "g" ) &&
        elementname != _T( "a" ) &&
        elementname != _T( "rect" ) &&
        elementname != _T( "text" ) &&
        elementname != _T( "circle" ) &&
        elementname != _T( "ellipse" ) &&
        elementname != _T( "polygon" ) &&
        elementname != _T( "polyline" ) &&
        elementname != _T( "line" ) &&
        elementname != _T( "path" ) &&
        elementname != _T( "image" )
    )
    {
        if( elementname == _T( "style" ) && g == m_doc->GetDrawing()->GetRootObject() )
        {
            wxString style = GetContent();

            style.Trim();
            wxStringTokenizer tkz( style, _T( "{}" ) );
            while ( tkz.HasMoreTokens() )
            {
                wxString shapeclass = tkz.GetNextToken().Trim( false ).Trim();
                wxString shapestyle = tkz.GetNextToken().Trim( false ).Trim();
                if ( shapeclass.Length() > 1 && shapestyle.IsEmpty () == false && shapeclass[( size_t )0] == _T( '.' ) )
                {
                    shapeclass = shapeclass.Right( shapeclass.Length() - 1 );
                    m_shapeclass[shapeclass] = shapestyle ;
                }
            }

            if( m_defaultclass != _T( "" ) )
            {
                wxString defaultstyle = m_shapeclass[m_defaultclass];
                if( defaultstyle.Find( _T( "font-size:" ) ) >= 0 )
                {
                    defaultstyle = defaultstyle.Right( defaultstyle.Length() - defaultstyle.Find( _T( "font-size:" ) ) - wxString( _T( "font-size:" ) ).Length() );
                    defaultstyle.ToDouble( &m_defaultfontsize );
                }
            }

            SkipSubTree();
            Require( END_TAG, elementname );
            Next();
            return true;
        }
        else
        {
            //error or not implemented element
            wxString parseerror = _T( "not implemented or unknown element " ) + GetTagName();
            a2dDocviewGlobals->ReportErrorF( a2dError_XMLparse, _( "SVG parsing error: '%s' at line %d" ), parseerror.c_str(), GetCurrentLineNumber() );
            SkipSubTree();
            Require( END_TAG, elementname );
            Next();
            return true;
        }
    }

    if (  LoadSvgGroup( g ) ||
            LoadSvgAnchor( g ) ||
            LoadSvgRect( g ) ||
            LoadSvgText( g ) ||
            LoadSvgCircle( g ) ||
            LoadSvgEllipse( g ) ||
            LoadSvgPolygon( g ) ||
            LoadSvgPolyline( g ) ||
            LoadSvgLine( g ) ||
            LoadSvgPath( g ) ||
            LoadSvgImage( g )
       )
    {
    }
    else
    {
        //error or not implemented element
        wxString parseerror = _T( "not implemented or unknown element " ) + GetTagName();
        a2dDocviewGlobals->ReportErrorF( a2dError_XMLparse, _( "SVG parsing error: '%s' at line %d" ), parseerror.c_str(), GetCurrentLineNumber() );
        //throw XmlPullParserException( "expected group element not:" + name ,GetCurrentLineNumber(), GetCurrentColumnNumber() );
    }

    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgGroup( a2dCanvasObject* parent )
{
    if ( GetTagName() != _T( "g" ) )
        return false;

    a2dCanvasObject* g = new a2dCanvasObject();
    m_currentobject = g;
    parent->Append( m_currentobject );

    ParseSvgTransForm( m_currentobject );
    ParseSvgStyle( m_currentobject );

    Next();

    while( 1 )
    {
        if ( !LoadSvgGroupElement( g ) )
        {
            Require( END_TAG, _T( "g" ) );
            Next();
            break;
        }
        else
        {
            //restore style at this level
            m_fill = g->GetFill();
            m_stroke = g->GetStroke();
        }
    }

    m_currentobject = g;
    m_fill = g->GetFill();
    m_stroke = g->GetStroke();
    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgAnchor( a2dCanvasObject* parent )
{
    if ( GetTagName() != _T( "a" ) )
        return false;

    a2dCanvasObject* g = new a2dCanvasObject();
    m_currentobject = g;
    parent->Append( m_currentobject );

    ParseSvgTransForm( m_currentobject );
    ParseSvgStyle( m_currentobject );
    ParseSvgLink( m_currentobject );

    Next();

    while( 1 )
    {
        if ( !LoadSvgGroupElement( g ) )
        {
            Require( END_TAG, _T( "a" ) );
            Next();
            break;
        }
        else
        {
            //restore style at this level
            m_fill = g->GetFill();
            m_stroke = g->GetStroke();
        }
    }

    m_currentobject = g;
    m_fill = g->GetFill();
    m_stroke = g->GetStroke();

    return true;
}

bool a2dIOHandlerSVGIn::ParseSvgTransForm( a2dCanvasObject* parent )
{
    wxString str = GetAttributeValue( _T( "transform" ) );

    if ( str == _T( "" ) )
        return false;

    wxString numstr;
    double matrixcoef[12];
    int nr_matrixcoef;

    wxString keywstr;
    size_t i;


    a2dAffineMatrix matrixtotal;
    matrixtotal.Identity();
    a2dAffineMatrix matrix;

    for ( i = 0; i < str.Len(); i++ )
    {
        //skip space
        while ( wxIsspace( str[i] ) ) i++;

        nr_matrixcoef = 0;
        keywstr.Clear();
        //transform keyword
        while ( i < str.Len() &&  wxIsalpha ( str[i] ) )
        {
            keywstr += str.GetChar( i );
            i++;
        }

        //skip space
        while ( wxIsspace( str[i] ) ) i++;

        if ( str[i] != _T( '(' ) )
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_XMLparse, _( "SVG parsing error: missing ( at line %d" ), GetCurrentLineNumber() );
            return true;
        }
        i++;

        while ( wxIsspace( str[i] ) ) i++;

        size_t len = str.Len();
        while ( i < len &&  str[i] != _T( ')' ) )
        {
            numstr.Clear();
            while ( i < len && ( wxIsdigit( str[i] ) || str[i] == _T( '+' ) || str[i] == _T( '-' ) || str[i] == _T( '.' ) || str[i] == _T( 'E' ) )  )
            {
                numstr += str[i];//str.GetChar( i );
                i++;
            }
            double val;
            numstr.ToDouble( &val );
            matrixcoef[nr_matrixcoef] = val;
            nr_matrixcoef++;
            while ( i < len && ( wxIsspace( str[i] ) || str[i] == _T( ',' ) ) ) i++;
        }

        if ( str[i] != _T( ')' ) )
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_XMLparse, _( "SVG parsing error: missing ) at line %d" ), GetCurrentLineNumber() );
            return true;
        }
        i++;

        if ( keywstr == _T( "matrix" ) )
        {
            if ( nr_matrixcoef != 6 )
            {
                wxLogWarning( _( "SVG : wrong number of arguments %s at line %d" ),
                              keywstr.c_str(),
                              GetCurrentLineNumber() );
                return true;
            }
            matrix.SetValue( 0, 0, matrixcoef[0] );
            matrix.SetValue( 0, 1, matrixcoef[1] );
            matrix.SetValue( 1, 0, matrixcoef[2] );
            matrix.SetValue( 1, 1, matrixcoef[3] );
            matrix.SetValue( 2, 0, matrixcoef[4] );
            matrix.SetValue( 2, 1, matrixcoef[5] );

            matrixtotal = matrixtotal * matrix;
        }
        else if ( keywstr == _T( "translate" ) )
        {
            if ( nr_matrixcoef == 1 )
                matrixcoef[0] = 0;
            else if ( nr_matrixcoef != 2 )
            {
                wxLogWarning( _( "SVG : wrong number of arguments %s at line %d" ),
                              keywstr.c_str(),
                              GetCurrentLineNumber() );
                return true;
            }

            matrix.Identity();
            matrix.Translate( matrixcoef[0] , matrixcoef[1] );
            matrix = a2dAffineMatrix( matrixcoef[0] , matrixcoef[1] );
            matrixtotal = matrixtotal * matrix;
        }
        else if ( keywstr == _T( "scale" ) )
        {
            if ( nr_matrixcoef == 1 )
                matrixcoef[1] = matrixcoef[0];
            else if ( nr_matrixcoef != 2 )
            {
                wxLogWarning( _( "SVG : wrong number of arguments %s at line %d" ),
                              keywstr.c_str(),
                              GetCurrentLineNumber() );
                return true;
            }

            matrix.Identity();
            matrix.Scale( matrixcoef[0] , matrixcoef[1], 0, 0 );
            matrix = a2dAffineMatrix( 0, 0, matrixcoef[0] , matrixcoef[1] );

            matrixtotal = matrixtotal * matrix;
        }
        else if ( keywstr == _T( "rotate" ) )
        {
            if ( nr_matrixcoef != 1 )
            {
                wxLogWarning( _( "SVG : wrong number of arguments %s at line %d" ),
                              keywstr.c_str(),
                              GetCurrentLineNumber() );
                return true;
            }

            matrix.Identity();
            matrix.SetRotation( matrixcoef[0] );
            matrix = a2dAffineMatrix( 0, 0, 1, 1, -matrixcoef[0] );
            matrixtotal = matrixtotal * matrix;
        }
        else if ( keywstr == _T( "skewX" ) )
        {
            if ( nr_matrixcoef != 1 )
            {
                wxLogWarning( _( "SVG : wrong number of arguments %s at line %d" ),
                              keywstr.c_str(),
                              GetCurrentLineNumber() );
                return true;
            }

            matrix.Identity();
            matrix.SkewX( matrixcoef[0] );
            matrixtotal = matrixtotal * matrix;
        }
        else if ( keywstr == _T( "skewY" ) )
        {
            if ( nr_matrixcoef != 1 )
            {
                wxLogWarning( _( "SVG : wrong number of arguments %s at line %d" ),
                              keywstr.c_str(),
                              GetCurrentLineNumber() );
                return true;
            }

            matrix.Identity();
            matrix.SkewY( matrixcoef[0] );
            matrixtotal = matrixtotal * matrix;
        }
        else if ( keywstr == _T( "flipX" ) )
        {
            if ( nr_matrixcoef != 0 )
            {
                wxLogWarning( _( "SVG : wrong number of arguments %s at line %d" ),
                              keywstr.c_str(),
                              GetCurrentLineNumber() );
                return true;
            }

            matrix.Identity();
            matrix.Mirror( true, false );
            matrixtotal = matrixtotal * matrix;
        }
        else if ( keywstr == _T( "flipY" ) )
        {
            if ( nr_matrixcoef != 0 )
            {
                wxLogWarning( _( "SVG : wrong number of arguments %s at line %d" ),
                              keywstr.c_str(),
                              GetCurrentLineNumber() );
                return true;
            }

            matrix.Identity();
            matrix.Mirror( false, true );
            matrixtotal = matrixtotal * matrix;
        }
        else
        {
            wxLogWarning( _( "SVG : invalid transform %s at line %d" ),
                          keywstr.c_str(),
                          GetCurrentLineNumber() );
            return true;
        }
    }

    parent->SetTransformMatrix( matrixtotal );
    return true;
}

bool a2dIOHandlerSVGIn::ParseSvgStyle( a2dCanvasObject* parent )
{
    /*
        if ( GetAttributeValue( _T("style") ) == _T("") )
        {
            parent->SetFill( a2dINHERIT_FILL );
            parent->SetStroke( a2dINHERIT_STROKE );

            return false;
        }
    */

    wxString styleattrib = GetAttributeValue( _T( "style" ) ).Trim( false ).Trim();

    //if no style atrribute, a class might be used for defining the style
    if ( styleattrib == _T( "" ) )
    {
        wxString classname = GetAttributeValue( _T( "class" ) );
        if ( classname != _T( "" ) )
        {
            //class may contain several items
            wxStringTokenizer tkz( classname, _T( " " ) );
            while ( tkz.HasMoreTokens() )
            {
                wxString oneitem = tkz.GetNextToken();

                if( !m_shapeclass[oneitem].IsEmpty() )
                {
                    if ( styleattrib.IsEmpty() )
                        styleattrib = m_shapeclass[oneitem] ; //contents of the class is the style.
                    else
                        styleattrib = styleattrib + wxT( ";" ) + m_shapeclass[oneitem] ; //contents of the class is the style.
                }
            }
        }
    }
    if ( styleattrib == _T( "" ) )
    {
        parent->SetFill( *a2dINHERIT_FILL );
        parent->SetStroke( *a2dINHERIT_STROKE );
        return false;
    }

    wxStringTokenizer tkz( styleattrib, _T( ";" ) );
    while ( tkz.HasMoreTokens() )
    {
        wxStringTokenizer stylepart( tkz.GetNextToken(), _T( ":" ) );
        wxString substyle = stylepart.GetNextToken().Trim( false ).Trim();
        wxString value = stylepart.GetNextToken().Trim( false ).Trim();
        double adouble;
        long aint;
        value.ToDouble( &adouble );
        value.ToLong( &aint );
        if ( substyle == _T( "fill" ) )
        {
            if ( value != _T( "none" ) )
            {
                m_fill = a2dFill( ParseSvgColour( value ) );
            }
            else
            {
                m_fill = *a2dNullFILL;
            }
        }
        else if ( substyle == _T( "fill-rule" ) )
        {
            if ( 0 != wxDynamicCast( parent, a2dPolygonL ) )
            {
                a2dPolygonL* obj = wxDynamicCast( parent, a2dPolygonL );

                if ( value == _T( "evenodd" ) )
                    obj->SetFillRule( wxODDEVEN_RULE );
                else if ( value == _T( "nonzero" ) )
                    obj->SetFillRule( wxWINDING_RULE );
                else
                    wxLogWarning( _( "SVG unknown style fill-rule: %s at line %d" ),
                                  value.c_str(),
                                  GetCurrentLineNumber() );
            }
        }
        else if ( substyle == _T( "stroke" ) )
        {
            if ( value != _T( "none" ) )
            {
                m_stroke = a2dStroke( ParseSvgColour( value ), a2dSTROKE_SOLID );
            }
            else
            {
                m_stroke = *a2dNullSTROKE;
            }
        }
        else if ( substyle == _T( "opacity" ) )
        {
            m_fill.SetAlpha( aint );
            m_stroke.SetAlpha( aint );
        }
        else if ( substyle == _T( "fill-opacity" ) )
        {
            m_fill.SetAlpha( aint );
        }
        else if ( substyle == _T( "stroke-opacity" ) )
        {
            m_stroke.SetAlpha( aint );
        }
        else if ( substyle == _T( "stroke-width" ) )
        {
            m_stroke.SetWidth( adouble );
        }
        else if ( substyle == _T( "stroke-linecap" ) )
        {
            if ( value == _T( "butt" ) )
                m_stroke.SetCap( wxCAP_BUTT );
            else if ( value == _T( "round" ) )
                m_stroke.SetCap( wxCAP_ROUND );
            else if ( value == _T( "square" ) )
                m_stroke.SetCap( wxCAP_PROJECTING );
            else
                wxLogWarning( _( "SVG unknown line cap style: %s at line %d" ),
                              value.c_str(),
                              GetCurrentLineNumber() );
        }
        else if ( substyle == _T( "stroke-linejoin" ) )
        {
            if ( value == _T( "miter" ) )
                m_stroke.SetJoin( wxJOIN_MITER );
            else if ( value == _T( "round" ) )
                m_stroke.SetJoin( wxJOIN_ROUND );
            else if ( value == _T( "bevel" ) )
                m_stroke.SetJoin( wxJOIN_BEVEL );
            else
                wxLogWarning( _( "SVG unknown line join style: %s at line %d" ),
                              value.c_str(),
                              GetCurrentLineNumber() );
        }
        else if ( substyle == _T( "font-size" ) )
        {
            /*!
               \todo
               font-size
            */
            a2dText* obj = wxDynamicCast( parent, a2dText );
            if( obj )
            {
                if( value.Right( 2 ) == _T( "em" ) )
                {
                    if( m_defaultfontsize < 0.1 )
                        obj->SetTextHeight( 7 );
                    else
                        obj->SetTextHeight( m_defaultfontsize * adouble );
                }
                else
                    obj->SetTextHeight( adouble );
            }
        }
        else if ( substyle == _T( "font-family" ) )
        {
            /*!
               \todo
               font-family
            */
        }
        else if ( substyle == _T( "stroke-miterlimit" ) )
        {
            /*!
               \todo
               stroke-miterlimit
            */
        }
        else
            wxLogWarning( _( "SVG unknown style: %s: %s at line %d" ),
                          substyle.c_str(), value.c_str(),
                          GetCurrentLineNumber() );
    }

    a2dFill fill = m_fill;
    a2dStroke stroke = m_stroke;

    parent->SetFill( fill );
    parent->SetStroke( stroke );

    return true;
}

bool a2dIOHandlerSVGIn::ParseSvgLink(  a2dCanvasObject* object )
{
    wxString linkattrib = GetAttributeValue( _T( "xlink:href" ) ).Trim( false ).Trim();
    wxStringTokenizer tkz( linkattrib, _T( ";" ) );
    while ( tkz.HasMoreTokens() )
    {
        wxString url = tkz.GetNextToken();
        object->SetURI( url );
    }

    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgRect( a2dCanvasObject* parent )
{
    if ( GetTagName() != _T( "rect" ) )
        return false;

    a2dRect* rect = new a2dRect();
    m_currentobject = rect;
    parent->Append( m_currentobject );

    ParseSvgTransForm( m_currentobject );
    ParseSvgStyle( m_currentobject );

    a2dAffineMatrix matrixtotal = rect->GetTransformMatrix();
    a2dAffineMatrix matrix;
    matrix.Identity();
    matrix.Translate( GetAttributeValueDouble( _T( "x" ) ) , GetAttributeValueDouble( _T( "y" ) ) );
    matrixtotal = matrixtotal * matrix;
    rect->SetTransformMatrix( matrixtotal );

    rect->SetWidth( GetAttributeValueDouble( _T( "width" ) ) );
    rect->SetHeight( GetAttributeValueDouble( _T( "height" ) ) );

    Next();
    Require( END_TAG, _T( "rect" ) );
    Next();

    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgCircle( a2dCanvasObject* parent )
{
    if ( GetTagName() != _T( "circle" ) )
        return false;

    a2dCircle* circle = new a2dCircle();
    m_currentobject = circle;
    parent->Append( m_currentobject );

    ParseSvgTransForm( m_currentobject );
    ParseSvgStyle( m_currentobject );

    a2dAffineMatrix matrixtotal = circle->GetTransformMatrix();
    a2dAffineMatrix matrix;
    matrix.Identity();
    matrix.Translate( GetAttributeValueDouble( _T( "cx" ) ) , GetAttributeValueDouble( _T( "cy" ) ) );
    matrixtotal = matrixtotal * matrix;
    circle->SetTransformMatrix( matrixtotal );

    circle->SetRadius( GetAttributeValueDouble( _T( "r" ) ) );

    Next();
    Require( END_TAG, _T( "circle" ) );
    Next();

    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgEllipse( a2dCanvasObject* parent )
{
    if ( GetTagName() != _T( "ellipse" ) )
        return false;

    a2dEllipse* Ellipse = new a2dEllipse();
    m_currentobject = Ellipse;
    parent->Append( m_currentobject );

    ParseSvgTransForm( m_currentobject );
    ParseSvgStyle( m_currentobject );

    a2dAffineMatrix matrixtotal = Ellipse->GetTransformMatrix();
    a2dAffineMatrix matrix;
    matrix.Identity();
    matrix.Translate( GetAttributeValueDouble( _T( "cx" ) ) , GetAttributeValueDouble( _T( "cy" ) ) );
    matrixtotal = matrixtotal * matrix;
    Ellipse->SetTransformMatrix( matrixtotal );

    Ellipse->SetWidth( GetAttributeValueDouble( _T( "rx" ) ) * 2 );
    Ellipse->SetHeight( GetAttributeValueDouble( _T( "ry" ) ) * 2 );

    Next();
    Require( END_TAG, _T( "ellipse" ) );
    Next();

    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgText( a2dCanvasObject* parent )
{
    if ( GetTagName() != _T( "text" ) )
        return false;

    a2dText* text = new a2dText( GetContent() );
    m_currentobject = text;
    parent->Append( m_currentobject );

    ParseSvgTransForm( m_currentobject );
    ParseSvgStyle( m_currentobject );

    text->SetFill( *a2dTRANSPARENT_FILL );

    a2dAffineMatrix matrixtotal = text->GetTransformMatrix();
    a2dAffineMatrix matrix;
    matrix.Identity();
    matrix.Translate( GetAttributeValueDouble( _T( "x" ) ) , GetAttributeValueDouble( _T( "y" ) ) );
    matrixtotal = matrixtotal * matrix;
    text->SetTransformMatrix( matrixtotal );

    Next();

    if( GetEventType() == END_TAG )
    {
        Require( END_TAG, _T( "text" ) );
        Next();
    }
    else
    {
        wxString s = _T( "" );
        while( GetEventType() != END_TAG || GetTagName() != _T( "text" ) )
        {
            if( !GetContent().IsEmpty() )
            {
                s = s + GetContent();
            }
            Next();
        }
        text->SetText( s );
        Next();
    }


    return true;
}

bool a2dIOHandlerSVGIn::ParsePoints( a2dVertexList* points )
{
    wxString pstr = GetAttributeValue( _T( "points" ) );

    if ( pstr == _T( "" ) )
        return false;

    wxString numstr;

    int j = 0;
    double x = 0.0, y = 0.0;
    size_t i;

    for ( i = 0; i < pstr.Len(); i++ )
    {
        if ( pstr[i] != _T( ' ' ) && pstr[i] != _T( ',' ) ) //spaces between coord and , between x and y
        {
            if ( j == 0 )
            {
                numstr.Clear();
                while ( i < pstr.Len() &&  pstr[i] != _T( ' ' ) && pstr[i] != _T( ',' ) )
                {
                    numstr += pstr.GetChar( i );
                    i++;
                }
                numstr.ToDouble( &x );
                j++;
            }
            else
            {
                numstr = _T( "" );
                while ( i < pstr.Len() &&  pstr[i] != _T( ' ' ) && pstr[i] != _T( ',' ) )
                {
                    numstr += pstr.GetChar( i );
                    i++;
                }
                numstr.ToDouble( &y );
                j--;
                points->push_back( new a2dLineSegment( x, y ) );
            }
        }
    }
    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgPolygon( a2dCanvasObject* parent )
{
    if ( GetTagName() != _T( "polygon" ) )
        return false;

    a2dVertexList* points = new a2dVertexList();

    if ( !ParsePoints( points ) )
    {
        delete points;
        wxString parseerror = _T( "points missing in polygon" );
        a2dDocviewGlobals->ReportErrorF( a2dError_XMLparse, _( "SVG parsing error: '%s' at line %d" ), parseerror.c_str(), GetCurrentLineNumber() );
        return true;
    }

    a2dPolygonL* poly = new a2dPolygonL( points );
    m_currentobject = poly;
    parent->Append( m_currentobject );

    ParseSvgTransForm( m_currentobject );
    ParseSvgStyle( m_currentobject );

    Next();
    Require( END_TAG, _T( "polygon" ) );
    Next();

    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgPolyline( a2dCanvasObject* parent )
{
    if ( GetTagName() != _T( "polyline" ) )
        return false;

    a2dVertexList* points = new a2dVertexList();

    if ( !ParsePoints( points ) )
    {
        delete points;
        wxString parseerror = _T( "points missing in polyline" );
        a2dDocviewGlobals->ReportErrorF( a2dError_XMLparse, _( "SVG parsing error: '%s' at line %d" ),
                                         parseerror.c_str(),
                                         GetCurrentLineNumber() );
        return true;
    }

    a2dPolylineL* poly = new a2dPolylineL( points );
    m_currentobject = poly;
    parent->Append( m_currentobject );

    ParseSvgTransForm( m_currentobject );
    ParseSvgStyle( m_currentobject );

    Next();
    Require( END_TAG, _T( "polyline" ) );
    Next();

    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgLine( a2dCanvasObject* parent )
{
    if ( GetTagName() != _T( "line" ) )
        return false;

    double x1 = GetAttributeValueDouble( _T( "x1" ) );
    double y1 = GetAttributeValueDouble( _T( "y1" ) );
    double x2 = GetAttributeValueDouble( _T( "x2" ) );
    double y2 = GetAttributeValueDouble( _T( "y2" ) );

    a2dSLine* line = new a2dSLine( x1, y1, x2, y2 );

    m_currentobject = line;
    parent->Append( m_currentobject );

    ParseSvgTransForm( m_currentobject );
    ParseSvgStyle( m_currentobject );

    Next();
    Require( END_TAG, _T( "line" ) );
    Next();

    return true;
}


#ifndef  M_PI
#define M_PI        (3.1415926535897932384626433832795028841972)
#endif

//! parse svg arc
/*!

   copy from sodipodi-0.32
   svg-path.c: Parse SVG path element data into bezier path.

   Copyright (C) 2000 Eazel, Inc.
   Copyright (C) 2000 Lauris Kaplinski
   Copyright (C) 2001 Ximian, Inc.

 * rsvg_path_arc: Add an RSVG arc to the path context.
 * \param startx startpoint x
 * \param starty startpoint y
 * \param rx Radius in x direction (before rotation).
 * \param ry Radius in y direction (before rotation).
 * \param x_axis_rotation Rotation angle for axes.
 * \param large_arc_flag 0 for arc length <= 180, 1 for arc >= 180.
 * \param sweep_flag 0 for "negative angle", 1 for "positive angle".
 * \param x nextpoint x
 * \param y nextpoint y
 * \param pcx New x coordinate.
 * \param pcy New y coordinate.
 * \param pangle angle
 *
 **/
static void
rsvg_path_arc ( double startx, double starty,
                double rx, double ry, double x_axis_rotation,
                int large_arc_flag, int sweep_flag,
                double x, double y,
                double* pcx, double* pcy,
                double* pangle )
{
    double sin_th, cos_th;
    double a00, a01, a10, a11;
    double x0, y0, x1, y1, xc, yc;
    double d, sfactor, sfactor_sq;
    double th0, th1, th_arc;
    int n_segs;

    sin_th = sin ( x_axis_rotation * ( M_PI / 180.0 ) );
    cos_th = cos ( x_axis_rotation * ( M_PI / 180.0 ) );
    a00 = cos_th / rx;
    a01 = sin_th / rx;
    a10 = -sin_th / ry;
    a11 = cos_th / ry;
    x0 = a00 * startx + a01 * starty;
    y0 = a10 * startx + a11 * starty;
    x1 = a00 * x + a01 * y;
    y1 = a10 * x + a11 * y;
    /* (x0, y0) is current point in transformed coordinate space.
       (x1, y1) is new point in transformed coordinate space.

       The arc fits a unit-radius circle in this space.
    */
    d = ( x1 - x0 ) * ( x1 - x0 ) + ( y1 - y0 ) * ( y1 - y0 );
    sfactor_sq = 1.0 / d - 0.25;
    if ( sfactor_sq < 0 ) sfactor_sq = 0;
    sfactor = sqrt ( sfactor_sq );
    if ( sweep_flag == large_arc_flag ) sfactor = -sfactor;
    xc = 0.5 * ( x0 + x1 ) - sfactor * ( y1 - y0 );
    yc = 0.5 * ( y0 + y1 ) + sfactor * ( x1 - x0 );
    /* (xc, yc) is center of the circle. */

    th0 = atan2 ( y0 - yc, x0 - xc );
    th1 = atan2 ( y1 - yc, x1 - xc );

    th_arc = th1 - th0;
    if ( th_arc < 0 && sweep_flag )
        th_arc += 2 * M_PI;
    else if ( th_arc > 0 && !sweep_flag )
        th_arc -= 2 * M_PI;

    n_segs = ( int ) ceil ( fabs ( th_arc / ( M_PI * 0.5 + 0.001 ) ) );

    //*************************
    sin_th = sin ( x_axis_rotation * ( M_PI / 180.0 ) );
    cos_th = cos ( x_axis_rotation * ( M_PI / 180.0 ) );
    /* inverse transform compared with rsvg_path_arc */
    a00 = cos_th * rx;
    a01 = -sin_th * ry;
    a10 = sin_th * rx;
    a11 = cos_th * ry;

    double th_half = 0.5 * ( th1 - th0 );
    double t = ( 8.0 / 3.0 ) * sin ( th_half * 0.5 ) * sin ( th_half * 0.5 ) / sin ( th_half );
    x1 = xc + cos ( th0 ) - t * sin ( th0 );
    y1 = yc + sin ( th0 ) + t * cos ( th0 );

//  double mx =  xc + cos (th0+th_arc/2) - t * sin (th0+th_arc/2);
//  double my =  xc + sin (th0+th_arc/2) + t * cos (th0+th_arc/2);

    *pcx = a00 * xc + a01 * yc;
    *pcy = a10 * xc + a11 * yc;

    *pangle = th_arc;
}
///
bool a2dIOHandlerSVGIn::ParsePathPoints( a2dVpath* points )
{
    wxString buffer = GetAttributeValue( _T( "d" ) );

    if ( buffer == _T( "" ) )
        return false;

    unsigned int position = 0;

    double lastmovex = 0;
    double lastmovey = 0;
    double lastx = 0;
    double lasty = 0;


    a2dVpathSegment* prev = 0;
    a2dVpathSegment* seg = 0;

    char current;

    SkipSpaces( buffer, position );

    while ( position < buffer.Len() )
    {
        current = buffer[ position ];
        char command = current;
        position++;
        SkipSpaces( buffer, position );
        try
        {
            switch ( current )
            {
                case _T( 'z' ):
                case _T( 'Z' ):
                {
                    lastx = lastmovex;
                    lasty = lastmovey;
                    prev = seg;
                    seg = new a2dVpathSegment( lastx, lasty, a2dPATHSEG_LINETO, a2dPATHSEG_END_CLOSED );
                    points->push_back( seg );
                    break;
                }
                case _T( 'm' ):
                case _T( 'M' ):
                {
                    if ( command == _T( 'm' ) )
                        lastx += ParseDouble( buffer, position );
                    else
                        lastx = ParseDouble( buffer, position );

                    SkipCommaSpaces( buffer, position );

                    if ( command == _T( 'm' ) )
                        lasty += ParseDouble( buffer, position );
                    else
                        lasty = ParseDouble( buffer, position );

                    SkipCommaSpaces( buffer, position );

                    prev = seg;
                    seg = new a2dVpathSegment( lastx, lasty, a2dPATHSEG_MOVETO );
                    points->push_back( seg );

                    lastmovex = lastx;
                    lastmovey = lasty;
                    for ( ; position < buffer.length(); )
                    {
                        current = buffer[ position ];
                        if ( current == wxT( '+' ) || current == wxT( '-' ) || wxIsdigit( current ) )
                        {
                            if ( command == _T( 'm' ) )
                                lastx += ParseDouble( buffer, position );
                            else
                                lastx = ParseDouble( buffer, position );

                            SkipCommaSpaces( buffer, position );

                            if ( command == _T( 'm' ) )
                                lasty += ParseDouble( buffer, position );
                            else
                                lasty = ParseDouble( buffer, position );

                            prev = seg;
                            seg = new a2dVpathSegment( lastx, lasty, a2dPATHSEG_LINETO );
                            points->push_back( seg );
                        }
                        else
                            break;
                        SkipCommaSpaces( buffer, position );
                    }
                    break;
                }
                case _T( 'l' ):
                case _T( 'L' ):
                {
                    for ( ; position < buffer.length(); )
                    {
                        current = buffer[ position ];
                        if ( current == wxT( '+' ) || current == wxT( '-' ) || wxIsdigit( current ) )
                        {
                            if ( command == _T( 'l' ) )
                                lastx += ParseDouble( buffer, position );
                            else
                                lastx = ParseDouble( buffer, position );

                            SkipCommaSpaces( buffer, position );

                            if ( command == _T( 'l' ) )
                                lasty += ParseDouble( buffer, position );
                            else
                                lasty = ParseDouble( buffer, position );

                            prev = seg;
                            seg = new a2dVpathSegment( lastx, lasty, a2dPATHSEG_LINETO );
                            points->push_back( seg );
                        }
                        else
                            break;
                        SkipCommaSpaces( buffer, position );
                    }
                    break;
                }
                case _T( 'h' ):
                case _T( 'H' ):
                {
                    for ( ; position < buffer.length(); )
                    {
                        current = buffer[ position ];
                        if ( current == wxT( '+' ) || current == wxT( '-' ) || wxIsdigit( current ) )
                        {
                            if ( command == _T( 'h' ) )
                                lastx += ParseDouble( buffer, position );
                            else
                                lastx = ParseDouble( buffer, position );

                            prev = seg;
                            seg = new a2dVpathSegment( lastx, lasty, a2dPATHSEG_LINETO );
                            points->push_back( seg );
                        }
                        else
                            break;
                        SkipCommaSpaces( buffer, position );
                    }
                    break;
                }
                case _T( 'V' ):
                case _T( 'v' ):
                {
                    for ( ; position < buffer.length(); )
                    {
                        current = buffer[ position ];
                        if ( current == wxT( '+' ) || current == wxT( '-' ) || wxIsdigit( current ) )
                        {
                            if ( command == _T( 'v' ) )
                                lasty += ParseDouble( buffer, position );
                            else
                                lasty = ParseDouble( buffer, position );

                            prev = seg;
                            seg = new a2dVpathSegment( lastx, lasty, a2dPATHSEG_LINETO );
                            points->push_back( seg );
                        }
                        else
                            break;
                        SkipCommaSpaces( buffer, position );
                    }
                    break;
                }
                case _T( 'c' ):
                case _T( 'C' ):
                case _T( 's' ):
                case _T( 'S' ):
                {
                    double x1 = 0.0, y1 = 0.0, x2, y2, x, y;
                    for ( ; position < buffer.length(); )
                    {
                        current = buffer[ position ];
                        if ( current == wxT( '+' ) || current == wxT( '-' ) || wxIsdigit( current ) )
                        {
                            if ( command == _T( 'C' ) || command == _T( 'c' ) )
                            {
                                x1 = ParseDouble( buffer, position );
                                SkipCommaSpaces( buffer, position );
                                y1 = ParseDouble( buffer, position );
                                SkipCommaSpaces( buffer, position );
                            }
                            x2 = ParseDouble( buffer, position );
                            SkipCommaSpaces( buffer, position );
                            y2 = ParseDouble( buffer, position );
                            SkipCommaSpaces( buffer, position );
                            x = ParseDouble( buffer, position );
                            SkipCommaSpaces( buffer, position );
                            y = ParseDouble( buffer, position );
                            SkipCommaSpaces( buffer, position );

                            if ( command == _T( 'c' ) || command == _T( 's' ) )
                            {
                                x1 += lastx;
                                y1 += lasty;
                                x2 += lastx;
                                y2 += lasty;
                                x += lastx;
                                y += lasty;
                            }

                            prev = seg;
                            if ( command == _T( 'C' ) || command == _T( 'c' ) )
                                seg = new a2dVpathCBCurveSegment( x, y, x1, y1 , x2, y2, a2dPATHSEG_CBCURVETO );
                            else
                                seg = new a2dVpathCBCurveSegment( prev, x, y , x2, y2, a2dPATHSEG_CBCURVETO );

                            points->push_back( seg );
                            lastx = x;
                            lasty = y;
                        }
                        else
                            break;
                        SkipCommaSpaces( buffer, position );
                    }
                    break;
                }
                case _T( 'q' ):
                case _T( 'Q' ):
                case _T( 't' ):
                case _T( 'T' ):
                {
                    double x1 = 0.0, y1 = 0.0, x, y;
                    for ( ; position < buffer.length(); )
                    {
                        current = buffer[ position ];
                        if ( current == wxT( '+' ) || current == wxT( '-' ) || wxIsdigit( current ) )
                        {

                            if ( command == _T( 'Q' ) || command == _T( 'q' ) )
                            {
                                x1 = ParseDouble( buffer, position );
                                SkipCommaSpaces( buffer, position );
                                y1 = ParseDouble( buffer, position );
                                SkipCommaSpaces( buffer, position );
                            }
                            x = ParseDouble( buffer, position );
                            SkipCommaSpaces( buffer, position );
                            y = ParseDouble( buffer, position );
                            SkipCommaSpaces( buffer, position );

                            if ( command == _T( 'q' ) || command == _T( 't' ) )
                            {
                                x1 += lastx;
                                y1 += lasty;
                                x += lastx;
                                y += lasty;
                            }

                            prev = seg;
                            if ( command == _T( 'Q' ) || command == _T( 'q' ) )
                                seg = new a2dVpathQBCurveSegment( x, y, x1, y1, a2dPATHSEG_QBCURVETO );
                            else
                                seg = new a2dVpathQBCurveSegment( prev, x, y, a2dPATHSEG_QBCURVETO );
                            points->push_back( seg );
                            lastx = x;
                            lasty = y;
                        }
                        else
                            break;
                        SkipCommaSpaces( buffer, position );
                    }
                    break;
                }
                case _T( 'a' ):
                case _T( 'A' ):
                {
                    double rx, ry, ax, x, y;
                    for ( ; position < buffer.length(); )
                    {
                        current = buffer[ position ];
                        if ( current == wxT( '+' ) || current == wxT( '-' ) || wxIsdigit( current ) )
                        {
                            rx = ParseDouble( buffer, position );
                            SkipCommaSpaces( buffer, position );
                            ry = ParseDouble( buffer, position );
                            SkipCommaSpaces( buffer, position );
                            ax = ParseDouble( buffer, position );
                            SkipCommaSpaces( buffer, position );

                            current = buffer[ position ];

                            bool laf = false;
                            switch ( current )
                            {
                                case _T( '0' ):
                                    laf = false;
                                    break;
                                case _T( '1' ):
                                    laf = true;
                                    break;
                            }
                            position++;
                            SkipCommaSpaces( buffer, position );
                            current = buffer[ position ];

                            bool sf = false;
                            switch ( current )
                            {
                                case _T( '0' ):
                                    sf = false;
                                    break;
                                case _T( '1' ):
                                    sf = true;
                                    break;
                            }
                            position++;

                            SkipCommaSpaces( buffer, position );
                            x = ParseDouble( buffer, position );
                            SkipCommaSpaces( buffer, position );
                            y = ParseDouble( buffer, position );
                            SkipCommaSpaces( buffer, position );

                            if ( command == _T( 'a' ) )
                            {
                                x += lastx;
                                y += lasty;
                            }

                            prev = seg;
                            double xc, yc;
                            double angle;

                            rsvg_path_arc( lastx, lasty, rx, ry, ax, laf, sf, x, y, &xc, &yc, &angle );
/// \!todo should be EllipticalArcSegment?
                            seg = new a2dVpathArcSegment( prev, xc, yc, angle * 180 / M_PI );

                            points->push_back( seg );
                            lastx = x;
                            lasty = y;
                        }
                        else
                            break;
                        SkipCommaSpaces( buffer, position );
                    }
                    break;
                }
                default:
                {
                    a2dDocviewGlobals->ReportErrorF( a2dError_XMLparse, _( "expected path command: '%s' at line %d" ), buffer.c_str(), GetCurrentLineNumber() );
                }
            }
        }
        catch ( const a2dIOHandlerXMLException& e )
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_XMLparse, _( "%s at line %d column %d" ),  e.getMessage().c_str(), e.getLineNumber(), e.getColumnNumber() );
            a2dDocviewGlobals->ReportErrorF( a2dError_XMLparse, _( "expected path command: '%s' at line %d" ), buffer.c_str(), GetCurrentLineNumber() );
        }
    }

    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgPath( a2dCanvasObject* parent )
{
    if ( GetTagName() != _T( "path" ) )
        return false;

    a2dVpath* points = new a2dVpath();

    if ( !ParsePathPoints( points ) )
    {
        delete points;
        wxString parseerror = _T( "points missing in polygon" );
        a2dDocviewGlobals->ReportErrorF( a2dError_XMLparse, _( "SVG parsing error: '%s' at line %d" ),
                                         parseerror.c_str(),
                                         GetCurrentLineNumber() );
        return true;
    }

    a2dVectorPath* path = new a2dVectorPath( points );
    m_currentobject = path;
    parent->Append( m_currentobject );

    ParseSvgTransForm( m_currentobject );
    ParseSvgStyle( m_currentobject );

    Next();
    Require( END_TAG, _T( "path" ) );
    Next();

    return true;
}

bool a2dIOHandlerSVGIn::LoadSvgImage( a2dCanvasObject* parent )
{
    if ( GetTagName() != _T( "image" ) )
        return false;

    a2dImage* Image = new a2dImage();
    m_currentobject = Image;
    parent->Append( m_currentobject );

    ParseSvgTransForm( m_currentobject );
    ParseSvgStyle( m_currentobject );

    Image->Translate( GetAttributeValueDouble( _T( "x" ) ) , GetAttributeValueDouble( _T( "y" ) ) );

    Image->SetWidth( GetAttributeValueDouble( _T( "width" ) ) );
    Image->SetHeight( GetAttributeValueDouble( _T( "height" ) ) );
    Image->SetFilename( GetAttributeValue( _T( "xlink:href" ) ), wxBITMAP_TYPE_PNG );


    Next();
    Require( END_TAG, _T( "image" ) );
    Next();

    return true;
}

// ----------------------------------------------------------------------------
// a2dIOHandlerSVGOut
// ----------------------------------------------------------------------------

a2dIOHandlerSVGOut::a2dIOHandlerSVGOut()
{
    m_fill = a2dFill();
    m_stroke = *a2dNullSTROKE;
    m_showObject = NULL;
    m_ic = NULL;
}

a2dIOHandlerSVGOut::~a2dIOHandlerSVGOut()
{
}

bool a2dIOHandlerSVGOut::CanSave( const wxObject* obj )
{
    if ( !wxDynamicCast( obj, a2dCanvasDocument ) )
        return false;
    return true;
}

bool a2dIOHandlerSVGOut::Save( a2dDocumentOutputStream& stream, const wxObject* doc )
{
    a2dIterC ic;
    m_doc = wxStaticCast( doc, a2dCanvasDocument );

    if ( !m_showObject )
        m_showObject = m_doc->GetDrawing()->GetRootObject();

    if ( !m_ic )
    {
        a2dCanvasView* drawer = wxDynamicCast( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView(), a2dCanvasView );
        if ( drawer )
        {
            ic = a2dIterC( drawer->GetDrawingPart() );
            m_ic = &ic;
        }
    }

    wxASSERT_MSG( m_ic, wxT( "No iteration context set while writing SVG output" ) );
    wxASSERT_MSG( m_ic->GetDrawer2D(), wxT( "a2dIOHandlerSVGOut no a2dDrawer2D while writing SVG output" ) );

    InitializeSave();

    double Width = 1000;
    double Height = 1000;
    wxString unit = _T( "um" );

    bool yaxis = m_ic->GetDrawer2D()->GetYaxis();

    //a2dCanvasObject* root = doc->GetRootObject();
    a2dCanvasObject* root = m_ic->GetDrawingPart()->GetShowObject();

    a2dWalker_SetCheck setp( false );
    setp.Start( root );

    m_streamo = &stream;

    WriteStartDocument( _T( "1.0" ), _T( "UTF-8" ) , false );
    EndlWriteString ( _T( "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" " ) );

    EndlWriteString ( _T( "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"> " ) );

    WriteStartElementAttributes( _T( "svg" ) );

    WriteAttribute( _T( "xmlns" ), _T( "http://www.w3.org/2000/svg" ) );
    WriteAttribute( _T( "xmlns:xlink" ), _T( "http://www.w3.org/1999/xlink" ) );

    wxString s;
    s.Printf ( _T( " width=\"%.2f%s\" height=\"%.2f%s\" viewBox=\"%f %f %f %f \" " ),
               Width, unit.c_str(), Height, unit.c_str(),
               m_ic->GetDrawer2D()->GetVisibleMinX(),
               m_ic->GetDrawer2D()->GetVisibleMinY(),
               m_ic->GetDrawer2D()->GetVisibleWidth(),
               m_ic->GetDrawer2D()->GetVisibleHeight() );
    //s.Printf ( "<svg width=\"%.2f%s\" height=\"%.2f%s\" viewBox=\"%f %f %f %f \"> \n",
    //Width,unit.c_str(),Height,unit.c_str(), root->GetBboxMinX(),root->GetBboxMinY(),
    //                                        root->GetBboxWidth(),root->GetBboxHeight());
    WriteString( s );
    WriteEndAttributes();

    WriteElement( _T( "title" ), m_doc->GetPrintableName() );
    WriteElement( _T( "desc" ), m_doc->GetDescription() );

    WriteStartElementAttributes( _T( "g" ) );
    WriteAttribute( _T( "style" ), _T( "fill:black; stroke:black; stroke-width:1" ) );
    WriteEndAttributes();

    if ( yaxis )
    {
        WriteStartElementAttributes( _T( "g" ) );
        WriteAttribute( _T( "transform" ), _T( "scale( 1,-1)" ) );
        WriteEndAttributes();
    }

    m_nameMap.clear();

    Save( root, wxLAYER_ALL );

    if ( yaxis )
    {
        WriteEndElement();
    }

    WriteEndElement();
    WriteEndElement();
    WriteEndDocument();

    ResetSave();

    return true;
}

wxString a2dIOHandlerSVGOut::UniqueName ( wxString baseName )
{
    if ( m_nameMap.find( baseName ) == m_nameMap.end() )
    {
        m_nameMap[baseName] = 0;
    }
    else
    {
        ++m_nameMap[baseName];
    }
    baseName.Printf( _T( "obj_%s_%d" ), baseName.c_str(), m_nameMap[baseName] );
    return baseName;
}


void a2dIOHandlerSVGOut::Save( const a2dCanvasObject* object, int layer )
{
    int objectlayer = object->GetLayer();
    a2dAffineMatrix lworld = object->GetTransformMatrix();
    if ( layer != objectlayer && layer != wxLAYER_ALL )
    {
        return;
    }

    if( object->IsTemporary_DontSave() )
        return;

    const a2dShadowStyleProperty* shadow = object->GetShadowStyle();

    if ( object->GetFill().GetType() == a2dFILL_GRADIENT_FILL_LINEAR || object->GetFill().GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        SaveGradient( object->GetFill(), object );
    }

    if ( shadow &&
            ( shadow->GetFill().GetType() == a2dFILL_GRADIENT_FILL_LINEAR || shadow->GetFill().GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
       )
    {
        SaveGradient( shadow->GetFill(), object );
    }

    wxString s;
    wxString url = object->GetURI().BuildURI();

    if ( !url.IsEmpty() )
    {
        WriteStartElementAttributes( _T( "a" ) );
        WriteAttribute( _T( "xlink:href" ), url );
        WriteAttribute( _T( "id" ), UniqueName( object->GetName() ) );
    }
    else
    {
        WriteStartElementAttributes( _T( "g" ) );
        WriteAttribute( _T( "id" ), UniqueName( object->GetName() ) );
    }

    if ( !lworld.IsIdentity() )
    {
        WriteNewLine();
        s.Printf ( _T( "matrix( %g %g %g %g %g %g )" ),
                   lworld.GetValue( 0, 0 ), lworld.GetValue( 0, 1 ),
                   lworld.GetValue( 1, 0 ), lworld.GetValue( 1, 1 ),
                   lworld.GetValue( 2, 0 ), lworld.GetValue( 2, 1 )
                 );
        WriteAttribute( _T( "transform" ), s );
        WriteNewLine();
    }

    SetSVGStyle( object->GetFill(), object->GetStroke(), object, layer );

    WriteEndAttributes(); //end of attributes for style and transform

    if ( shadow ) //this style is seperate from layer id
    {
        double dx = cos( shadow->GetExtrudeAngle() ) * shadow->GetExtrudeDepth();
        double dy = sin( shadow->GetExtrudeAngle() ) * shadow->GetExtrudeDepth();

        WriteStartElementAttributes( _T( "g" ) );
        s.Printf ( _T( " %f %f " ), dx, dy );
        WriteAttribute( _T( "translate" ), s );

        SetSVGStyle( shadow->GetFill(), shadow->GetStroke(), object, layer );

        WriteEndAttributes(); //end of attributes for style and transform

        DoSave( object, layer );

        WriteEndElement();
    }

    DoSave( object, layer );

    a2dNamedPropertyList::const_iterator iter;
    for( iter = object->GetPropertyList().begin(); iter !=  object->GetPropertyList().end(); ++iter )
    {
        const a2dNamedProperty* prop = *iter;
        DoSaveProperty( object, prop, wxLAYER_ALL );
    }

    const a2dCanvasObjectList* childobjects = object->GetChildObjectList();

    if ( childobjects != wxNullCanvasObjectList )
    {
        a2dCanvasObjectList::const_iterator iter = childobjects->begin();

        if ( iter != childobjects->end() )
        {

            //iterate over layers if this object is meant to be drawn on a given layer at once
            // (m_layer != wxLAYER_ALL) If layer == wxLAYER_ALL, the iteration is done in some parent.
            //OR in case the object is the topobject that is displayed on the DRAWER

            if (  ( layer == wxLAYER_ALL || object == m_ic->GetDrawingPart()->GetShowObject() )
                    &&
                    m_doc->GetLayerSetup() //is there a layer setup
               )
            {
                if ( m_ic->GetDrawingPart()->GetReverseOrder() )
                {
                    for ( a2dLayerIndex::iterator it= m_doc->GetLayerSetup()->GetOrderSort().begin(); it != m_doc->GetLayerSetup()->GetOrderSort().end(); ++it)
                    {
                        a2dLayerInfo* objlayer = *it;


                        //important!
                        //if layer is visible it will be rendered
                        //If an object on a layer is itself invisible it will not be drawn
                        //wxLAYER_ALL is reserved and should never be part of the layersettings

                        //! todo this save of SVG should itself make a record of available layers,
                        //! maybe use an internal view to do so.

                        if ( objlayer->GetVisible() )
                        {
                            for( a2dCanvasObjectList::const_iterator iter2 = childobjects->begin(); iter2 != childobjects->end(); ++iter2 )
                            {
                                a2dCanvasObject* obj = ( *iter2 );

                                int olayer = obj->GetLayer();

                                if ( olayer == objlayer->GetLayer() )
                                {
                                    Save( obj, objlayer->GetLayer() );
                                }
                            }
                        }
                    }
                }
                else
                {
                    for ( a2dLayerIndex::reverse_iterator it= m_doc->GetLayerSetup()->GetOrderSort().rbegin(); it != m_doc->GetLayerSetup()->GetOrderSort().rend(); ++it)
                    {
                        a2dLayerInfo* objlayer = *it;

                        //important!
                        //if layer is visible it will be rendered
                        //If an object on a layer is itself invisible it will not be drawn
                        //wxLAYER_ALL is reserved and should never be part of the layersettings

                        //! todo this save of SVG should itself make a record of available layers,
                        //! maybe use an internal view to do so.

                        if ( objlayer->GetVisible() )
                        {
                            for( a2dCanvasObjectList::const_iterator iter2 = childobjects->begin(); iter2 != childobjects->end(); ++iter2 )
                            {
                                a2dCanvasObject* obj = ( *iter2 );

                                int olayer = obj->GetLayer();

                                if ( olayer == objlayer->GetLayer() )
                                {
                                    Save( obj, objlayer->GetLayer() );
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for( a2dCanvasObjectList::const_iterator iter2 = childobjects->begin(); iter2 != childobjects->end(); ++iter2 )
                {
                    a2dCanvasObject* obj = ( *iter2 );
                    int olayer = obj->GetLayer();

                    if ( olayer == layer )
                    {
                        Save( obj, layer );
                        WriteNewLine();
                    }
                }
            }
        }
    }

    WriteNewLine();
    WriteEndElement();
}

void a2dIOHandlerSVGOut::SaveGradient( const a2dFill& fill, const a2dCanvasObject* object )
{
    if ( fill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
    {
        double x1 = fill.GetStart().m_x;
        double y1 = fill.GetStart().m_y;
        double x2 = fill.GetStop().m_x;
        double y2 = fill.GetStop().m_y;

        if ( x1 == x2 && y1 == y2 )
        {
            //assume vertical gradient size of bbox
            x1 = object->GetUnTransformedBbox( a2dCanvasObject::a2dCANOBJ_BBOX_CHILDREN ).GetMinX();
            y1 = object->GetUnTransformedBbox( a2dCanvasObject::a2dCANOBJ_BBOX_CHILDREN ).GetMinY();
            x2 = object->GetUnTransformedBbox( a2dCanvasObject::a2dCANOBJ_BBOX_CHILDREN ).GetMaxX();
            y2 = object->GetUnTransformedBbox( a2dCanvasObject::a2dCANOBJ_BBOX_CHILDREN ).GetMaxY();
        }

        WriteStartElementAttributes( _T( "linearGradient" ) );
        WriteAttribute( _T( "id" ), fill.GetStamp() );
        WriteAttribute( _T( "gradientUnits" ), _T( "userSpaceOnUse" ) );
        WriteAttribute( _T( "x1" ), x1 );
        WriteAttribute( _T( "y1" ), y1 );
        WriteAttribute( _T( "x2" ), x2 );
        WriteAttribute( _T( "y2" ), y2 );
        WriteEndAttributes();

        wxString s;

        WriteStartElementAttributes( _T( "stop" ) );
        WriteAttribute( _T( "offset" ), 0 );
        s.Printf (   _T( "stop-color:#%s" ), ColourToHex( fill.GetColour() ).c_str() );
        WriteAttribute( _T( "style" ), s );
        WriteEndAttributes();
        WriteEndElement();

        WriteStartElementAttributes( _T( "stop" ) );
        WriteAttribute( _T( "offset" ), 1 );
        s.Printf (   _T( "stop-color:#%s" ), ColourToHex( fill.GetColour2() ).c_str() );
        WriteAttribute( _T( "style" ), s );
        WriteEndAttributes();
        WriteEndElement();

        WriteEndElement();
    }
    else if ( fill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        double radius = fill.GetRadius();

        if ( fill.GetRadius() == 0 )
        {
            //assume vertical gradient size of bbox
            radius = object->GetUnTransformedBbox( a2dCanvasObject::a2dCANOBJ_BBOX_CHILDREN ).GetWidth() / 2.0;
        }

        WriteStartElementAttributes( _T( "radialGradient" ) );
        WriteAttribute( _T( "id" ), fill.GetStamp() );
        WriteAttribute( _T( "gradientUnits" ), _T( "userSpaceOnUse" ) );

        WriteAttribute( _T( "cx" ), fill.GetCenter().m_x );
        WriteAttribute( _T( "cy" ), fill.GetCenter().m_y );
        WriteAttribute( _T( "r" ), radius );
        WriteAttribute( _T( "xfc" ), fill.GetFocal().m_x );
        WriteAttribute( _T( "yfc" ), fill.GetFocal().m_y );
        WriteEndAttributes();

        wxString s;

        WriteStartElementAttributes( _T( "stop" ) );
        WriteAttribute( _T( "offset" ), 0 );
        s.Printf (   _T( "stop-color:#%s" ), ColourToHex( fill.GetColour() ).c_str() );
        WriteAttribute( _T( "style" ), s );
        WriteEndAttributes();
        WriteEndElement();

        WriteStartElementAttributes( _T( "stop" ) );
        WriteAttribute( _T( "offset" ), 1 );
        s.Printf (   _T( "stop-color:#%s" ), ColourToHex( fill.GetColour2() ).c_str() );
        WriteAttribute( _T( "style" ), s );
        WriteEndAttributes();
        WriteEndElement();

        WriteEndElement();
    }
}

void a2dIOHandlerSVGOut::SetSVGStyle( const a2dFill& fill, const a2dStroke& stroke, const a2dCanvasObject* object, int WXUNUSED( layer ) )
{
    int objectlayer = object->GetLayer();

    a2dLayers* layers = m_doc->GetLayerSetup();

    a2dFill fillw;
    fillw = fill;
    a2dStroke strokew;
    strokew = stroke;
    if ( object->GetFilled() )
    {
        if ( fill.IsNoFill() )
            fillw = layers->GetFill( objectlayer );
    }
    else
    {
        fillw = *a2dTRANSPARENT_FILL;
    }

    if ( stroke.IsNoStroke() )
        strokew = layers->GetStroke( objectlayer );

    wxString s;

    if ( strokew != *a2dINHERIT_STROKE )
    {
        if ( strokew.GetType() == a2dSTROKE_ONE_COLOUR )
        {
            wxString s2, s3 ;

            if ( *a2dTRANSPARENT_STROKE != strokew  )
            {
                s2 = _T( " stroke:#" ) + ColourToHex( strokew.GetColour() ) + _T( "; " ) ;

                if ( strokew.GetPixelStroke() )
                {
                    double worldwidth = m_ic->GetDrawer2D()->DeviceToWorldXRel( stroke.GetWidth() );
                    s3.Printf (   _T( "stroke-width:%f; " ), worldwidth );
                }
                else
                {
                    s3.Printf (   _T( "stroke-width:%f; " ), strokew.GetWidth() );
                }

                if ( strokew.GetAlpha() < 255 )
                {
                    wxString strokeAlpha;
                    strokeAlpha.Printf
                    (
                        _T( "stroke-opacity:%f; " ),
                        ( double )( strokew.GetAlpha() / 255.0 )
                    );
                    s3 = s3 + strokeAlpha;
                }
                if ( strokew.GetStyle() != a2dSTROKE_SOLID )
                {
                    wxString strokeDash;
                    switch ( strokew.GetStyle() )
                    {
                        case a2dSTROKE_DOT:
                            strokeDash = _T( "stroke-dasharray: 0.3 0.3;" );
                            break;
                        case a2dSTROKE_LONG_DASH:
                            strokeDash = _T( "stroke-dasharray: 1.5 0.5;" );
                            break;
                        case a2dSTROKE_SHORT_DASH:
                            strokeDash = _T( "stroke-dasharray: 1 0.5;" );
                            break;
                        case a2dSTROKE_DOT_DASH:
                            strokeDash = _T( "stroke-dasharray: 0.5 1 1.5 1;" );
                            break;
                        case a2dSTROKE_USER_DASH:
                        case a2dSTROKE_TRANSPARENT:
                        case a2dSTROKE_BDIAGONAL_HATCH:
                        case a2dSTROKE_CROSSDIAG_HATCH:
                        case a2dSTROKE_FDIAGONAL_HATCH:
                        case a2dSTROKE_CROSS_HATCH:
                        case a2dSTROKE_HORIZONTAL_HATCH:
                        case a2dSTROKE_VERTICAL_HATCH:
                        case a2dSTROKE_STIPPLE:
                        case a2dSTROKE_STIPPLE_MASK_OPAQUE:
                        case a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT:
                        case a2dSTROKE_OBJECTS:
                        case a2dSTROKE_INHERIT:
                        case a2dSTROKE_LAYER:
                        case a2dSTROKE_NULLSTROKE:
                        default:
                            // No support right now
                            break;
                    };
                    if ( strokeDash.Length() > 0 )
                    {
                        s3 = s3 + strokeDash;
                    }
                }
            }
            else
            {
                s2 = _T( " stroke:none; " ) ;
            }
            s = s2 + s3;

        }
        /*
        else if ( 0 != wxDynamicCast( stroke, a2dCanvasObjectStroke ))
        {
            //a2dCanvasObjectStroke* obj = wxDynamicCast( stroke, a2dCanvasObjectStroke );

        }
        */
    }

    if ( fillw != *a2dINHERIT_FILL )
    {
        wxString s1( _T( "" ) );
        if ( fillw.GetType() == a2dFILL_ONE_COLOUR )
        {
            if ( fillw.GetStyle() != a2dFILL_TRANSPARENT )
            {
                s1 = _T( " fill:#" ) + ColourToHex( fillw.GetColour() ) + _T( "; " ) ;
            }
            else
            {
                s1 = _T( " fill:none; " ) ;
            }
        }
        else if ( fillw.GetType() == a2dFILL_GRADIENT_FILL_LINEAR )
        {
            wxString gradientstamp;
            gradientstamp.Printf( _T( "%d" ), fillw.GetStamp() );

            if ( fillw != *a2dTRANSPARENT_FILL )
            {
                s1 = _T( " fill-rule:nonzero; fill:url(#" ) + gradientstamp + _T( "); " );
            }
            else
            {
                s1 = _T( " fill:none; " ) ;
            }

        }
        else if ( fillw.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
        {
            wxString gradientstamp;
            gradientstamp.Printf( _T( "%d" ), fillw.GetStamp() );

            if ( fillw != *a2dTRANSPARENT_FILL )
            {
                s1 = _T( " fill-rule:nonzero; fill:url(#" ) + gradientstamp + _T( "); " );
            }
        }
        else
        {
            s1 = _T( " fill:none; " ) ;
        }

        if ( fillw.GetAlpha() < 255 )
        {
            wxString fillAlpha;
            fillAlpha.Printf
            (
                _T( "fill-opacity:%f; " ),
                ( double )( fillw.GetAlpha() / 255.0 )
            );
            s1 = s1 + fillAlpha;
        }


        s = s + s1;
    }

    a2dNamedPropertyList::const_iterator iter;
    for( iter = object->GetPropertyList().begin(); iter !=  object->GetPropertyList().end(); ++iter )
    {
        const a2dNamedProperty* prop = *iter;
        if ( wxDynamicCast( prop, a2dClipPathProperty ) )
        {
            wxString s1;
            s1.Printf ( _T( "clip-path:url(#%d)" ), prop->GetRefObject()->GetId() );
            s = s + s1;
        }
    }

    if ( !s.IsEmpty() )
    {
        *this << _T( " style = \" " ) << s << _T( " \" " );
    }
}

void a2dIOHandlerSVGOut::CreateSVGSpline
(
    const a2dVertexList*        vertexList,
    bool                        asPolygon,
    const wxString&             style
)
{

    // If you want to read off the points from the SVG into wxArt2D, use the
    // control points as the "real" points of the control polygon.

    wxString s;
    WriteStartElementAttributes( _T( "path" ) );

    if ( asPolygon )
    {
        if ( style.Length() > 0 )
        {
            WriteAttribute( _T( "style" ), style );
        }
        else
        {
            WriteAttribute( _T( "style" ), _T( "fill-rule:nonzero; " ) );
        }
    }
    else
    {
        WriteAttribute( _T( "style" ), _T( "fill:none;" ) );
    }
    EndlWriteString( _T( "d=\"" ) );


    a2dVertexList::const_iterator iter = vertexList->begin();

    // First coordinate moveTo:
    a2dLineSegment* point = *iter;

    double x = point->m_x;
    double y = point->m_y;
    ++iter;

    double xOld = x;
    double yOld = y;


    point = *iter;
    x = point->m_x;
    y = point->m_y;
    double xFirstMidpoint = xOld + 0.5 * ( x - xOld );
    double yFirstMidpoint = yOld + 0.5 * ( y - yOld );

    if ( asPolygon )
    {
        // Polygons begin at the first midpoint, so that the curve can close.
        s.Printf( _T( "M %f,%f" ), xFirstMidpoint, yFirstMidpoint );
        EndlWriteString( s );
    }
    else
    {
        // Generate a line to the midpoint between the first point and the
        // first midpoint.  This prevents the line endings from curving
        // immediately, which will look better:

        s.Printf( _T( "M %f,%f" ), xOld, yOld );
        EndlWriteString( s );

        s.Printf( _T( "L %f,%f" ), xFirstMidpoint, yFirstMidpoint );
        EndlWriteString( s );
    }


    // We now generate a series of curveTos.  This will produce the  same
    // result as the Chaikin algorithm used for the vertex list  spline
    // routines. This is achieved by using the midpoints of the original line
    // segments as endpoints for our curves, while using the line segment
    // endpoints as the control points for the quadratic curves.

    for
    (
        ++iter;
        iter != vertexList->end();
        ++iter
    )
    {
        xOld = x;
        yOld = y;
        point = *iter;
        x = point->m_x;
        y = point->m_y;

        // Endpoint is the halfway point between the segments:

        s.Printf
        (
            _T( "Q %f,%f,%f,%f" ),
            xOld,
            yOld,
            xOld + 0.5 * ( x - xOld ),
            yOld + 0.5 * ( y - yOld )
        );
        EndlWriteString ( s );
    }

    if ( asPolygon )
    {
        // Two final curves back to the first midpoint:
        xOld = x;
        yOld = y;
        point = vertexList->front();
        x = point->m_x;
        y = point->m_y;
        s.Printf
        (
            _T( "Q %f,%f,%f,%f" ),
            xOld,
            yOld,
            xOld + 0.5 * ( x - xOld ),
            yOld + 0.5 * ( y - yOld )
        );
        EndlWriteString( s );

        s.Printf
        (
            _T( "Q %f,%f,%f,%f" ),
            x,
            y,
            xFirstMidpoint,
            yFirstMidpoint
        );
        EndlWriteString( s );

    }
    else
    {
        // Create the last line:
        s.Printf ( _T( "L %f,%f" ), x, y );
        EndlWriteString( s );
    }

    EndlWriteString( _T( "\"" ) ) ;
    WriteEndAttributes();
    WriteEndElement();
}
void a2dIOHandlerSVGOut::CreateSVGSpline
(
    const a2dVertexArray*       vertexArray,
    bool                        asPolygon,
    const wxString&             style
)
{
    a2dVertexList h;
    unsigned int i;
    for ( i = 0; i < vertexArray->size(); i++ )
    {
        h.push_back( vertexArray->Item( i )->Clone() );
    }

    CreateSVGSpline( &h, asPolygon, style );
}

void a2dIOHandlerSVGOut::DoSave( const a2dCanvasObject* object, int layer )
{
    wxString s;

    // CANVAS OBJECT ARRAY REFERENCE
    if ( 0 != wxDynamicCast( object, a2dCanvasObjectArrayReference ) )
    {
        a2dCanvasObjectArrayReference* obj = wxDynamicCast( object, a2dCanvasObjectArrayReference );

        a2dAffineMatrix currentWorld = a2dIDENTITY_MATRIX;

        a2dAffineMatrix offsetXY;

        a2dAffineMatrix originalRef = obj->GetCanvasObject()->GetTransformMatrix();

        int i, j;
        for ( i = 0  ; i < obj->GetRows(); i++ )
        {
            for ( j = 0  ; j < obj->GetColumns(); j++ )
            {
                a2dAffineMatrix tworld = currentWorld;
                tworld *= originalRef;
                obj->GetCanvasObject()->SetTransformMatrix( tworld );
                Save( obj->GetCanvasObject(), layer );
                obj->GetCanvasObject()->SetTransformMatrix( originalRef );

                offsetXY.Translate( obj->GetHorzSpace(), 0 );

                currentWorld = a2dIDENTITY_MATRIX;
                currentWorld *= offsetXY;
            }
            //translate back (one row of columns) and add one row
            offsetXY.Translate( -obj->GetHorzSpace()*obj->GetColumns(), obj->GetVertSpace() );
            currentWorld = a2dIDENTITY_MATRIX;
            currentWorld *= offsetXY;
        }
    }
    // CANVAS OBJECT REFERENCE
    else if ( 0 != wxDynamicCast( object, a2dCanvasObjectReference ) )
    {
        a2dCanvasObjectReference* obj = wxDynamicCast( object, a2dCanvasObjectReference );
        Save( obj->GetCanvasObject(), layer );
    }
    // ORIGIN
    else if ( 0 != wxDynamicCast( object, a2dOrigin ) )
    {
        a2dOrigin* obj = wxDynamicCast( object, a2dOrigin );

        double w = ( int ) m_ic->GetDrawer2D()->DeviceToWorldXRel( obj->GetWidth() );
        double h = ( int ) m_ic->GetDrawer2D()->DeviceToWorldYRel( obj->GetHeight() );

        s.Printf ( _T( "M%f %f L %f %f" ), -w, 0.0, w, 0.0  );
        WriteStartElementAttributes( _T( "path" ) );
        WriteAttribute( _T( "d" ), s );
        WriteEndAttributes();
        WriteEndElement();
        s.Printf ( _T( "M %f %f L %f %f" ), 0.0, -h, 0.0, h );
        WriteStartElementAttributes( _T( "path" ) );
        WriteAttribute( _T( "d" ), s );
        WriteEndAttributes();
        WriteEndElement();
    }
    /*
    // These two are defined in screen coordinates, and are made for the
    // interface, not so much the graphics themselves.  They will render
    // disproportionately from the other objects.  We skip them as a result.
    // HANDLE
    else if (0 != wxDynamicCast( object, a2dHandle ))
    {
        a2dHandle* obj = wxDynamicCast( object, a2dHandle );

        //transform points to absolute
        double x = - obj->GetWidth()/2;
        double y = - obj->GetHeight()/2;

        WriteStartElementAttributes( _T("rect") );
        WriteAttribute( _T("x"), x );
        WriteAttribute( _T("y"), y );
        WriteAttribute( _T("width"), obj->GetWidth() );
        WriteAttribute( _T("height"), obj->GetHeight() );

        if ( obj->GetRadius() != 0 )
            WriteAttribute( _T("rx"), obj->GetRadius() );

        WriteEndAttributes();
        WriteEndElement();
    }
    // PIN
    else if (0 != wxDynamicCast( object, a2dPin ))
    {
        a2dPin* obj = wxDynamicCast( object, a2dPin );

        //transform points to absolute
        double x = - obj->GetWidth()/2;
        double y = - obj->GetHeight()/2;

        WriteStartElementAttributes( _T("rect") );
        WriteAttribute( _T("x"), x );
        WriteAttribute( _T("y"), y );
        WriteAttribute( _T("width"), obj->GetWidth() );
        WriteAttribute( _T("height"), obj->GetHeight() );

        if ( obj->GetRadius() != 0 )
            WriteAttribute( _T("rx"), obj->GetRadius() );

        WriteEndAttributes(true);
    }
    */
    // ARROW
    else if ( 0 != wxDynamicCast( object, a2dArrow ) )
    {
        a2dArrow* obj = wxDynamicCast( object, a2dArrow );

        a2dPoint2D* cpoints = new a2dPoint2D[5];
        cpoints[0].m_x = 0   ; cpoints[0].m_y = 0;
        cpoints[1].m_x = obj->GetL1(); cpoints[1].m_y = obj->GetBase() / 2;
        cpoints[2].m_x = obj->GetL2(); cpoints[2].m_y = 0;
        cpoints[3].m_x = obj->GetL1(); cpoints[3].m_y = -obj->GetBase() / 2;

        WriteStartElementAttributes( _T( "polygon" ) );

        wxPolygonFillMode fillStyle = wxWINDING_RULE;
        if ( fillStyle == wxODDEVEN_RULE )
            s = _T( "fill-rule:evenodd; " );
        else
            s = _T( "fill-rule:nonzero; " );

        WriteAttribute( _T( "style" ), s );
        EndlWriteString( _T( " points=\"" ) );

        for ( int i = 0; i < 5;  i++ )
        {
            s.Printf ( _T( "%f,%f " ), cpoints [i].m_x, cpoints[i].m_y );
            *this <<  s;
        }

        EndlWriteString( _T( "\"" ) ) ;
        WriteEndAttributes( true ); // closes element

        delete [] cpoints;
    }
    // RECT_C
    else if ( 0 != wxDynamicCast( object, a2dRectC ) )
    {
        a2dRectC* obj = wxDynamicCast( object, a2dRectC );

        //transform points to absolute
        double x = - obj->GetWidth() / 2;
        double y = - obj->GetHeight() / 2;

        WriteStartElementAttributes( _T( "rect" ) );
        WriteAttribute( _T( "x" ), x );
        WriteAttribute( _T( "y" ), y );
        WriteAttribute( _T( "width" ), obj->GetWidth() );
        WriteAttribute( _T( "height" ), obj->GetHeight() );

        if ( obj->GetRadius() != 0 )
            WriteAttribute( _T( "rx" ), obj->GetRadius() );

        WriteEndAttributes( true ); // closes element
    }
    // RECT
    else if ( 0 != wxDynamicCast( object, a2dRect ) )
    {
        a2dRect* obj = wxDynamicCast( object, a2dRect );

        //transform points to absolute
        double x = 0;
        double y = 0;

        WriteStartElementAttributes( _T( "rect" ) );
        WriteAttribute( _T( "x" ), x );
        if ( obj->GetHeight() < 0 )
        {
            WriteAttribute( _T( "y" ), obj->GetHeight() );
            WriteAttribute( _T( "width" ), obj->GetWidth() );
            WriteAttribute( _T( "height" ), -obj->GetHeight() );
        }
        else
        {
            WriteAttribute( _T( "y" ), y );
            WriteAttribute( _T( "width" ), obj->GetWidth() );
            WriteAttribute( _T( "height" ), obj->GetHeight() );
        }

        if ( obj->GetRadius() != 0 )
            WriteAttribute( _T( "rx" ), obj->GetRadius() );

        WriteEndAttributes( true ); // closes element
    }
    // CIRCLE
    else if ( 0 != wxDynamicCast( object, a2dCircle ) )
    {
        a2dCircle* obj = wxDynamicCast( object, a2dCircle );

        WriteStartElementAttributes( _T( "circle" ) );
        WriteAttribute( _T( "cx" ), 0 );
        WriteAttribute( _T( "cy" ), 0 );
        WriteAttribute( _T( "r" ), obj->GetRadius() );

        WriteEndAttributes( true ); // closes element
    }
    // ELLIPSE
    else if ( 0 != wxDynamicCast( object, a2dEllipse ) )
    {
        a2dEllipse* obj = wxDynamicCast( object, a2dEllipse );

        double rh = obj->GetHeight() / 2 ;
        double rw = obj->GetWidth() / 2 ;


        WriteStartElementAttributes( _T( "ellipse" ) );
        WriteAttribute( _T( "cx" ), 0 );
        WriteAttribute( _T( "cy" ), 0 );
        WriteAttribute( _T( "rx" ), rw );
        WriteAttribute( _T( "ry" ), rh );

        WriteEndAttributes( true ); // closes element
    }
    // ELLIPTIC ARC
    else if ( 0 != wxDynamicCast( object, a2dEllipticArc ) )
    {
        a2dEllipticArc* obj = wxDynamicCast( object, a2dEllipticArc );

        //TODO rotation
        // pens have width, color
        // dashes, stipple ends, joins not implemented
        a2dStroke pen = object->GetStroke();
        /*
            wxColour c = pen->GetColour () ;
            unsigned char pr, pg, pb ;
            pr = c.Red ();
            pg = c.Green ();
            pb = c. Blue ();
        */
        wxString s, s1, s2, s3 ;

        a2dFill fill = object->GetFill();

        double xc = 0;
        double yc = 0;

        double rx = obj->GetHeight() / 2 ;
        double ry = obj->GetWidth() / 2 ;

        double xs, ys, xe, ye ;
        xs = xc + rx * cos ( wxDegToRad( obj->GetStart() ) ) ;
        xe = xc + rx * cos ( wxDegToRad( obj->GetEnd() ) ) ;
        ys = yc - ry * sin ( wxDegToRad( obj->GetStart() ) ) ; //coordinate system y axis goes down
        ye = yc - ry * sin ( wxDegToRad( obj->GetEnd() ) ) ;

        ///now same as circle arc...

        double theta1 = atan2( yc - ys, xs - xc );
        if ( theta1 < 0 ) theta1 = theta1 + wxPI * 2;
        double theta2 = atan2( yc - ye, xe - xc );
        if ( theta2 < 0 ) theta2 = theta2 + wxPI * 2;
        if ( theta2 < theta1 ) theta2 = theta2 + wxPI * 2 ;

        bool fArc;                                   // flag for large or small arc 0 means less than 180 degrees
        if ( fabs( ( theta2 - theta1 ) ) > wxPI ) fArc = true; else fArc = false;

        bool fSweep = false;                              // flag for sweep always 0

        if ( fill != *a2dTRANSPARENT_FILL )
        {
            WriteStartElementAttributes( _T( "path" ) );

            //first only filled
            s.Printf ( _T( "M%f %f A%f %f 0.0 %s %s  %f %f L%f %f Z" ),
                       xs,  ys,  rx,   ry,   fArc ? _T( "1" ) : _T( "0" ) , fSweep ? _T( "1" ) : _T( "0" ), xe,  ye, xc, yc  );
            WriteAttribute( _T( "d" ), s );
            WriteAttribute( _T( "style" ), _T( "stroke:none; " ) );
            WriteEndAttributes();
            WriteEndElement();
        }

        if ( pen != *a2dTRANSPARENT_STROKE )
        {
            //again only outline
            WriteStartElementAttributes( _T( "path" ) );

            //first only filled
            s.Printf ( _T( "M%f %f A%f %f 0.0 %s %s  %f %f L%f %f Z" ),
                       xs,  ys,  rx,   ry,    fArc ? _T( "1" ) : _T( "0" ) , fSweep ? _T( "1" ) : _T( "0" ), xe,  ye, xc, yc  );
            //NON closed outline
            //s.Printf ( "M%f %f A%f %f 0.0 %f %f  %f %f",
            //                       xs,  ys,  rx,   ry,    fArc? "1": "0" , fSweep? "1": "0",xe,  ye );

            WriteAttribute( _T( "d" ), s );
            WriteAttribute( _T( "style" ), _T( "fill:none; " ) );
            WriteEndAttributes();
            WriteEndElement();


        }
    }
    // ARC
    else if ( 0 != wxDynamicCast( object, a2dArc ) )
    {
        a2dArc* obj = wxDynamicCast( object, a2dArc );

        //TODO rotation
        // pens have width, color
        // dashes, stipple ends, joins not implemented
        a2dStroke pen = object->GetStroke();

        wxString s, s1, s2, s3 ;

        a2dFill fill = object->GetFill();

        double xc = 0;
        double yc = 0;

        double dx = obj->GetX1();
        double dy = obj->GetY1();
        double radius = sqrt( dx * dx + dy * dy );

        double start = wxRadToDeg( atan2( dy, dx ) );

        dx = obj->GetX2();
        dy = obj->GetY2();
        double end = wxRadToDeg( atan2( dy, dx ) );

        double rx = radius;
        double ry = radius;

        double xs, ys, xe, ye ;
        if ( m_ic->GetDrawer2D()->GetYaxis() )
        {
            xs = xc + rx * cos ( wxDegToRad( start ) ) ;
            xe = xc + rx * cos ( wxDegToRad( end ) ) ;
            ys = yc + ry * sin ( wxDegToRad( start ) ) ;
            ye = yc + ry * sin ( wxDegToRad( end ) ) ;
        }
        else
        {
            xe = xc + rx * cos ( wxDegToRad( start ) ) ;
            xs = xc + rx * cos ( wxDegToRad( end ) ) ;
            ye = yc + ry * sin ( wxDegToRad( start ) ) ; //coordinate system y axis goes down
            ys = yc + ry * sin ( wxDegToRad( end ) ) ;
        }

        ///now same as circle arc...

        double theta1 = atan2( yc - ys, xs - xc );
        if ( theta1 < 0 ) theta1 = theta1 + wxPI * 2;
        double theta2 = atan2( yc - ye, xe - xc );
        if ( theta2 < 0 ) theta2 = theta2 + wxPI * 2;
        if ( theta2 < theta1 ) theta2 = theta2 + wxPI * 2 ;

        bool fArc;                                   // flag for large or small arc 0 means less than 180 degrees
        if ( fabs( ( theta2 - theta1 ) ) > wxPI ) fArc = true; else fArc = false;

        bool fSweep = false;                              // flag for sweep always 0

        if ( fill != *a2dTRANSPARENT_FILL )
        {
            WriteStartElementAttributes( _T( "path" ) );

            //first only filled
            s.Printf ( _T( "M%f %f A%f %f 0.0 %s %s  %f %f L%f %f Z" ),
                       xs,  ys,  rx,   ry,   fArc ? _T( "1" ) : _T( "0" ) , fSweep ? _T( "1" ) : _T( "0" ), xe,  ye, xc, yc  );
            WriteAttribute( _T( "d" ), s );
            WriteAttribute( _T( "style" ), _T( "stroke:none; " ) );
            WriteEndAttributes();
            WriteEndElement();
        }

        if ( pen != *a2dTRANSPARENT_STROKE )
        {
            //again only outline
            WriteStartElementAttributes( _T( "path" ) );

            //first only filled
            s.Printf ( _T( "M%f %f A%f %f 0.0 %s %s  %f %f L%f %f Z" ),
                       xs,  ys,  rx,   ry,  fArc ? _T( "1" ) : _T( "0" ) , fSweep ? _T( "1" ) : _T( "0" ), xe,  ye, xc, yc  );
            //NON closed outline
            //s.Printf ( "M%f %f A%f %f 0.0 %f %f  %f %f",
            //                       xs,  ys,  rx,   ry,   fArc,fSweep,xe,  ye );

            WriteAttribute( _T( "d" ), s );
            WriteAttribute( _T( "style" ), _T( "fill:none; " ) );
            WriteEndAttributes();
            WriteEndElement();
        }
    }
    // ENDS LINE
    else if ( 0 != wxDynamicCast( object, a2dEndsLine ) )
    {
        a2dEndsLine* obj = wxDynamicCast( object, a2dEndsLine );

        double x1 = obj->GetPosX1( false );
        double y1 = obj->GetPosY1( false );
        double x2 = obj->GetPosX2( false );
        double y2 = obj->GetPosY2( false );
        double xscale = obj->GetEndScaleX();
        double yscale = obj->GetEndScaleY();

        WriteStartElementAttributes( _T( "path" ) );
        s.Printf ( _T( "M%f %f L%f %f" ), x1, y1, x2, y2 );
        WriteAttribute( _T( "d" ), s );
        WriteEndAttributes();
        WriteEndElement();

        if ( obj->GetBegin() )
        {
            double dx, dy;
            dx = x2 - x1;
            dy = y2 - y1;
            double ang1;
            if ( !dx && !dy )
                ang1 = 0;
            else
                ang1 = wxRadToDeg( atan2( dy, dx ) );

            a2dAffineMatrix lworld;
            //clockwise rotation so minus
            lworld.Scale( xscale, yscale, 0, 0 );
            lworld.Rotate( -ang1 );
            lworld.Translate( x1, y1 );

            WriteStartElementAttributes( _T( "g" ) );
            s.Printf ( _T( "matrix( %g %g %g %g %g %g )" ),
                       lworld.GetValue( 0, 0 ), lworld.GetValue( 0, 1 ),
                       lworld.GetValue( 1, 0 ), lworld.GetValue( 1, 1 ),
                       lworld.GetValue( 2, 0 ), lworld.GetValue( 2, 1 )
                     );
            WriteAttribute( _T( "transform" ), s );
            WriteEndAttributes();

            Save( obj->GetBegin(), layer );

            WriteEndElement();
        }

        if ( obj->GetEnd() )
        {
            double dx, dy;
            dx = x2 - x1;
            dy = y2 - y1;
            double ang2;
            if ( !dx && !dy )
                ang2 = 0;
            else
                ang2 = wxRadToDeg( atan2( -dy, -dx ) );

            a2dAffineMatrix lworld;
            //clockwise rotation so minus
            lworld.Scale( xscale, yscale, 0, 0 );
            lworld.Rotate( -ang2 );
            lworld.Translate( x2, y2 );

            WriteStartElementAttributes( _T( "g" ) );
            s.Printf ( _T( "matrix( %g %g %g %g %g %g )" ),
                       lworld.GetValue( 0, 0 ), lworld.GetValue( 0, 1 ),
                       lworld.GetValue( 1, 0 ), lworld.GetValue( 1, 1 ),
                       lworld.GetValue( 2, 0 ), lworld.GetValue( 2, 1 )
                     );
            WriteAttribute( _T( "transform" ), s );
            WriteEndAttributes();

            Save( obj->GetEnd(), layer );

            WriteEndElement();
        }
    }
    // SLINE
    else if ( 0 != wxDynamicCast( object, a2dSLine ) )
    {
        a2dSLine* obj = wxDynamicCast( object, a2dSLine );

        WriteStartElementAttributes( _T( "path" ) );
        s.Printf ( _T( "M%f %f L%f %f" ), obj->GetPosX1( false ), obj->GetPosY1( false ), obj->GetPosX2( false ), obj->GetPosY2( false ) );
        WriteAttribute( _T( "d" ), s );
        WriteEndAttributes();
        WriteEndElement();
    }
    // IMAGE
    else if ( 0 != wxDynamicCast( object, a2dImage ) )
    {
        a2dImage* obj = wxDynamicCast( object, a2dImage );

        double width = obj->GetWidth();
        double height = obj->GetHeight();

        //transform points to absolute
        double x = -width / 2;
        double y = -height / 2;

        WriteStartElementAttributes( _T( "image" ) );
        WriteAttribute( _T( "x" ), x );
        WriteAttribute( _T( "y" ), y );
        WriteAttribute( _T( "width" ), width );
        WriteAttribute( _T( "height" ), height );
        WriteAttribute( _T( "xlink:href" ), obj->GetFilename() );
        WriteEndAttributes();
        WriteEndElement();
    }
    // TEXT
    else if ( 0 != wxDynamicCast( object, a2dText ) )
    {
        a2dText* obj = wxDynamicCast( object, a2dText );

        //a2dStroke pen = object->GetStroke();

        const a2dFontInfo& font = obj->GetFont().GetFontInfo();

        wxString fontFamily =
            ( font.GetName().Length() > 0 )
            ?   font.GetName()
            :   _T( "Verdana, LiberationSans, sans-serif" );

        wxString fontStyleBold =
            ( font.GetStyle().Lower().Find( _T( "bold" ) ) != -1 )
            ?   _T( "font-weight:bold; " )
            :   ( font.GetStyle().Lower().Find( _T( "light" ) ) != -1 )
            ?   _T( "font-weight:lighter; " )
            :   _T( "" );

        wxString fontStyleItalic =
            ( font.GetStyle().Lower().Find( _T( "italic" ) ) != -1 )
            ?   _T( "font-style:italic; " )
            :   ( font.GetStyle().Lower().Find( _T( "slant" ) ) != -1 )
            ?   _T( "font-style:oblique; " )
            :   _T( "" );


        wxString alignmentH =
            ( obj->GetAlignment() & ( wxMAXX ) )
            ?   _T( "end" )         // right
            :   ( obj->GetAlignment() & ( wxMINX ) )
            ?   _T( "start" )   // left
            :   _T( "middle" ); // center, default

        wxString alignmentV =
            ( obj->GetAlignment() & ( wxMINY ) )
            ?   _T( "-100%" )
            :   ( obj->GetAlignment() & ( wxMIDY ) )
            ?   _T( "-50%" )
            :   _T( "0%" ); // top, default

        wxString color = ColourToHex( obj->GetStroke().GetColour() );

        s.Printf
        (
            _T( "font-family:%s; " )
            _T( "font-size:%fpt; " )  // Remember the "pt"
            _T( "fill:#%s; " )
            _T( "stroke:#%s; " )
            _T( "text-anchor:%s; " )    // For h-align
            _T( "baseline-shift:%s; " ) // For v-align
            _T( "%s%s" ),
            fontFamily.c_str(),
            obj->GetTextHeight(),
#if wxCHECK_VERSION(2, 9, 0)
            color.GetData().AsChar(),
            color.GetData().AsChar(),
            alignmentH.GetData().AsChar(),
            alignmentV.GetData().AsChar(),
            fontStyleBold.GetData().AsChar(),
            fontStyleItalic.GetData().AsChar()
#else
            color.c_str(),
            color.c_str(),
            alignmentH.c_str(),
            alignmentV.c_str(),
            fontStyleBold.c_str(),
            fontStyleItalic.c_str()
#endif
        );

        WriteStartElementAttributes( _T( "text" ) );
        WriteAttribute( _T( "x" ), 0 );
        WriteAttribute( _T( "y" ), 0 );
        WriteAttribute( _T( "style" ), s );
        WriteEndAttributes();
        /*!
            \todo
            Multiline text (tspan)
        */
        WriteContent( obj->GetText() );
        WriteEndElement();
    }
    // POLYLINE L NEW
    else if ( 0 != wxDynamicCast( object, a2dPolylineL ) )
    {
        a2dPolylineL* obj = wxDynamicCast( object, a2dPolylineL );

        a2dVertexList* points = obj->GetSegments();

        if ( obj->GetSpline() && ( points->size() > 2 ) )
        {
            CreateSVGSpline ( points );
        }
        else
        {

            // Polyline (should be in else statement):
            WriteStartElementAttributes( _T( "polyline" ) );

            WriteAttribute( _T( "style" ), _T( "fill:none;" ) );
            EndlWriteString( _T( " points=\"" ) );

            unsigned int i = 0;
            for
            (
                a2dVertexList::iterator iter = points->begin();
                iter != points->end();
                ++iter
            )
            {
                a2dLineSegment* point = *iter;
                s.Printf ( _T( "%f,%f " ), point->m_x, point->m_y );
                if ( i % 6 == 0 )
                    WriteNewLine();
                WriteString( s ) ;
                i++;
            }

            EndlWriteString( _T( "\"" ) ) ;
            WriteEndAttributes();
            WriteEndElement();
        }

        // Beginning and end graphics:


        double xscale = obj->GetEndScaleX();
        double yscale = obj->GetEndScaleY();

        if ( obj->GetBegin() )
        {
            a2dVertexList::iterator iter = points->begin();
            a2dLineSegment* point1 = *iter;
            iter++;
            a2dLineSegment* point2 = *iter;

            double dx, dy;
            dx = point2->m_x - point1->m_x;
            dy = point2->m_y - point1->m_y;

            double ang1;
            if ( !dx && !dy )
                ang1 = 0;
            else
                ang1 = wxRadToDeg( atan2( dy, dx ) );

            a2dAffineMatrix lworld;
            //clockwise rotation so minus
            lworld.Scale( xscale, yscale, 0, 0 );
            lworld.Rotate( -ang1 );
            lworld.Translate( point1->m_x, point1->m_y );

            WriteStartElementAttributes( _T( "g" ) );
            s.Printf ( _T( "matrix( %g %g %g %g %g %g )" ),
                       lworld.GetValue( 0, 0 ), lworld.GetValue( 0, 1 ),
                       lworld.GetValue( 1, 0 ), lworld.GetValue( 1, 1 ),
                       lworld.GetValue( 2, 0 ), lworld.GetValue( 2, 1 )
                     );
            WriteAttribute( _T( "transform" ), s );
            WriteEndAttributes();

            Save( obj->GetBegin(), layer );

            WriteEndElement();
        }

        if ( obj->GetEnd() )
        {
            a2dVertexList::reverse_iterator iter = points->rbegin();
            a2dLineSegment* point1 = *iter;
            iter++;
            a2dLineSegment* point2 = *iter;

            double dx, dy;
            dx = point2->m_x - point1->m_x;
            dy = point2->m_y - point1->m_y;
            double ang2;
            if ( !dx && !dy )
                ang2 = 0;
            else
                ang2 = wxRadToDeg( atan2( dy, dx ) );

            a2dAffineMatrix lworld;
            //clockwise rotation so minus
            lworld.Scale( xscale, yscale, 0, 0 );
            lworld.Rotate( -ang2 );
            lworld.Translate( point1->m_x, point1->m_y );

            WriteStartElementAttributes( _T( "g" ) );
            s.Printf ( _T( "matrix( %g %g %g %g %g %g )" ),
                       lworld.GetValue( 0, 0 ), lworld.GetValue( 0, 1 ),
                       lworld.GetValue( 1, 0 ), lworld.GetValue( 1, 1 ),
                       lworld.GetValue( 2, 0 ), lworld.GetValue( 2, 1 )
                     );
            WriteAttribute( _T( "transform" ), s );
            WriteEndAttributes();

            Save( obj->GetEnd(), layer );

            WriteEndElement();
        }

    }
    // POLYGON (LIST)
    else if ( 0 != wxDynamicCast( object, a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxDynamicCast( object, a2dPolygonL );

        a2dVertexList* points = obj->GetSegments();
        wxPolygonFillMode fillStyle = wxWINDING_RULE;
        if ( fillStyle == wxODDEVEN_RULE )
            s = _T( "fill-rule:evenodd; " );
        else
            s = _T( "fill-rule:nonzero; " );

        if ( obj->GetSpline() && ( points->size() > 2 ) )
        {
            CreateSVGSpline( points, true, s );
        }
        else
        {
            WriteStartElementAttributes( _T( "polygon" ) );

            WriteAttribute( _T( "style" ), s );
            WriteNewLine();
            EndlWriteString( _T( " points=\"" ) );

            unsigned int i = 0;
            for
            (
                a2dVertexList::iterator iter = points->begin();
                iter != points->end();
                ++iter
            )
            {
                a2dLineSegment* seg = *iter;
                if ( i % 6 == 0 )
                    WriteNewLine();
                s.Printf ( _T( "%f,%f " ), seg->m_x, seg->m_y );
                WriteString( s );
                i++;
            }
            WriteString( _T( "\"" ) ) ;
            WriteNewLine();
            WriteEndAttributes();
            WriteEndElement();
        }
    }
    // VECTOR PATH
    else if ( 0 != wxDynamicCast( object, a2dVectorPath ) )
    {
        a2dVectorPath* obj = wxDynamicCast( object, a2dVectorPath );

        WriteStartElementAttributes( _T( "path" ) );
        WriteString( _T( " d=\"" ) );

        unsigned int i;
        for ( i = 0; i < obj->GetCount(); i++ )
        {
            a2dVpathSegment* seg = obj->GetSegments()->Item( i );

            switch ( seg->m_type )
            {
                case a2dPATHSEG_MOVETO:
                {
                    s.Printf ( _T( "M %f %f " ), seg->m_x1, seg->m_y1 );
                    break;
                }
                case a2dPATHSEG_LINETO:
                {
                    s.Printf ( _T( "L %f %f " ), seg->m_x1, seg->m_y1 );
                    break;
                }
                case a2dPATHSEG_LINETO_NOSTROKE:
                {
                    //TODO how to emulate this
                    s.Printf ( _T( "L %f %f " ), seg->m_x1, seg->m_y1 );
                    break;
                }

                case a2dPATHSEG_CBCURVETO:
                {
                    a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg;
                    s.Printf ( _T( "C %g %g %g %g %g %g " ),  cseg->m_x2, cseg->m_y2,  cseg->m_x3, cseg->m_y3, cseg->m_x1, cseg->m_y1 );
                    break;
                }
                case a2dPATHSEG_CBCURVETO_NOSTROKE:
                {
                    a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg;
                    s.Printf ( _T( "C %g %g %g %g %g %g " ),  cseg->m_x2, cseg->m_y2,  cseg->m_x3, cseg->m_y3, cseg->m_x1, cseg->m_y1 );
                    break;
                }

                case a2dPATHSEG_QBCURVETO:
                {
                    a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg;
                    s.Printf ( _T( "T %f %f %f %f " ),  cseg->m_x2, cseg->m_y2, cseg->m_x1, cseg->m_y1 );
                    break;
                }
                case a2dPATHSEG_QBCURVETO_NOSTROKE:
                {
                    a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg;
                    s.Printf ( _T( "T %f %f %f %f " ),  cseg->m_x2, cseg->m_y2, cseg->m_x1, cseg->m_y1 );
                    break;
                }

                case a2dPATHSEG_ARCTO:
                {
                    //TODO how to emulate this
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg;
                    s.Printf ( _T( "T %f %f %f %f " ),  cseg->m_x2, cseg->m_y2, cseg->m_x1, cseg->m_y1 );
                    break;
                }
                case a2dPATHSEG_ARCTO_NOSTROKE:
                {
                    //TODO how to emulate this
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg;
                    s.Printf ( _T( "T %f %f %f %f " ),  cseg->m_x2, cseg->m_y2, cseg->m_x1, cseg->m_y1 );
                    break;
                }
                default:
                    break;
            }

            WriteString( s );

            switch ( seg->GetClose() )
            {
                case a2dPATHSEG_END_CLOSED:
                {
                    WriteString( _T( " Z" ) );
                    break;
                }
                case a2dPATHSEG_END_CLOSED_NOSTROKE:
                {
                    //TODO ignore nostroke for the moment
                    WriteString( _T( " Z" ) );
                    break;
                }
                default:
                    break;
            }

            if ( i % 2 == 0 )
                WriteNewLine();
        }

        WriteString( _T( "\"" ) ) ;
        WriteEndAttributes();
        WriteEndElement();
    }

}

void a2dIOHandlerSVGOut::DoSaveProperty( const a2dCanvasObject* WXUNUSED( object ), const a2dNamedProperty* property, int layer )
{
    wxString s;
    if ( 0 != wxDynamicCast( property, a2dClipPathProperty ) )
    {
        a2dClipPathProperty* prop = wxDynamicCast( property, a2dClipPathProperty );

        //we have a clip path
        if ( prop->GetClipObject() )
        {
            WriteStartElement( _T( "defs" ) );
            WriteStartElementAttributes( _T( "clipPath" ) );
            WriteAttribute( _T( "id" ), prop->GetRefObject()->GetId() );

            //oh ooh next not possible since clippath does not understand groups within,
            //only plain shapes!
            //Save( indent+2, stream, prop->GetClip(), layer );

            a2dAffineMatrix lworld = prop->GetClipObject()->GetTransformMatrix();
            if ( !lworld.IsIdentity() )
            {
                s.Printf ( _T( "matrix( %g %g %g %g %g %g )" ),
                           lworld.GetValue( 0, 0 ), lworld.GetValue( 0, 1 ),
                           lworld.GetValue( 1, 0 ), lworld.GetValue( 1, 1 ),
                           lworld.GetValue( 2, 0 ), lworld.GetValue( 2, 1 )
                         );
                WriteAttribute( _T( "transform" ), s );
            }

            SetSVGStyle( prop->GetClipObject()->GetFill(),  prop->GetClipObject()->GetStroke(), prop->GetClipObject(), layer );
            WriteEndAttributes();

            a2dCanvasObjectList* childobjects = prop->GetClipObject()->GetChildObjectList();

            //the next is a workaround which should work for valid SVG i think.
            //THE PROBLEM is that it can only be one level deep.
            //SO either it is the clipobject itself or when it is a group it is the children of the object
            //which defines the clip.
            //This misses the case of both havin children and the object itself.
            if ( childobjects != wxNullCanvasObjectList )
            {
                forEachIn( a2dCanvasObjectList, childobjects )
                DoSave( ( *iter ), layer );
            }

            WriteEndElement();
            WriteEndElement();
        }

        if ( prop->GetVisible()  )
        {
            SetSVGStyle( prop->GetClipObject()->GetFill(),  prop->GetClipObject()->GetStroke(), prop->GetClipObject(), layer );
            Save( prop->GetClipObject(), layer );
        }

    }
    else if (  0 != wxDynamicCast( property, a2dProperty ) &&
               property->GetCanRender() && property->GetVisible()  )
    {
        Save( ( a2dCanvasObject* ) property->GetRefObject(), layer );
    }
}
