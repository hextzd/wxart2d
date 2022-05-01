/*! \file artbase/src/artglob.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: artglob.cpp,v 1.26 2009/08/20 20:39:37 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/artbase/artglob.h"
#include "wx/artbase/stylebase.h"

WX_DEFINE_LIST( a2dClipRegionList );
WX_DEFINE_LIST( a2dAETList );
WX_DEFINE_LIST( a2dCriticalPointList );

const double wxPI = 3.1415926535897932384626433832795;

#if !wxCHECK_VERSION(3,1,0)

A2DARTBASEDLLEXP double wxDegToRad( double deg ) { return ( deg * wxPI ) / 180.0; }
A2DARTBASEDLLEXP double wxRadToDeg( double rad ) { return ( rad * 180 ) / wxPI; }
#endif

//the one and only
a2dGlobal* a2dGlobals;


IMPLEMENT_DYNAMIC_CLASS( a2dArtBaseModule, wxModule )

a2dArtBaseModule::a2dArtBaseModule()
{
    AddDependency( CLASSINFO( a2dGeneralModule ) );
    AddDependency( CLASSINFO( a2dFreetypeModule ) );

    a2dGeneralGlobals->GetVariablesHash().SetVariableString( wxT( "wxart2dlayers" ), wxT( "wxart2dlayers.cvg" ) );
    a2dGeneralGlobals->GetVariablesHash().SetVariableString( wxT( "layerFileSavePath" ), wxGetHomeDir() );
}

bool a2dArtBaseModule::OnInit()
{
    a2dGlobals = new a2dGlobal();

    //initiliaze after a2dGlobals, since stock objects use font paths.
    a2dCanvasInitializeStockObjects ();

    return true;
}

void a2dArtBaseModule::OnExit()
{
    delete a2dGlobals;
    a2dGlobals = NULL;

    a2dCanvasDeleteStockObjects ();
}

void Aberration( double aber, double angle, double radius, double& dphi, unsigned int& segments )
{
    if ( aber >= radius )
        dphi = wxPI / 4;
    else
        dphi = 2.0 * acos( ( radius - aber ) / radius );

    //smaller then one degree?
    if ( dphi < wxPI / 180.0 )
        dphi = wxPI / 180.0;

    //too big does not makes sence also
    if ( dphi > wxPI / 4 )
        dphi = wxPI / 4;

    //set the number of segments
    if ( segments )
    {
        int calcSegments = ( unsigned int ) ceil( fabs( angle ) / dphi ); //at least 1
        if ( calcSegments > segments )
            segments = calcSegments;
    }
    if ( segments == 0 )
        dphi = 0;
    else
        dphi = angle / segments; //equal angle for each segment
}

/**************************************************
    a2dGlobal
**************************************************/

a2dPathList a2dGlobal::m_fontpath = a2dPathList();
a2dPathList a2dGlobal::m_imagepath = a2dPathList();
a2dPathList a2dGlobal::m_iconpath = a2dPathList();
wxUint16 a2dGlobal::m_drawingthreshold = 1;
wxUint16 a2dGlobal::m_polygonFillThreshold = 0;
bool a2dGlobal::m_asrectangle = true;
double a2dGlobal::m_arc2polyaber = 0;
double a2dGlobal::m_poly2arcaber = 0;
double a2dGlobal::m_displayaberration = 0;
double a2dGlobal::m_roundFactor = 1.5;

IMPLEMENT_CLASS( a2dGlobal, a2dGeneralGlobal )

a2dGlobal::a2dGlobal()
{
    m_iconpath.Add( wxT( "." ) );
    m_iconpath.Add( wxT( "./icon" ) );
    m_iconpath.Add( wxT( ".." ) );
    m_iconpath.Add( wxT( "../common/icons" ) );
    m_iconpath.Add( wxT( "../../common/icons" ) );

    m_imagepath.Add( wxT( "." ) );
    m_imagepath.Add( wxT( "./images" ) );
    m_imagepath.Add( wxT( ".." ) );
    m_imagepath.Add( wxT( "../common/images" ) );
    m_imagepath.Add( wxT( "../../common/images" ) );

    wxString envValue;
    wxGetEnv( wxT( "TRUETYPE" ), &envValue );

    m_fontpath.Add( envValue );

    wxString art2dPath = a2dGeneralGlobals->GetWxArt2DArtVar( true, true );
    if ( ! art2dPath.IsEmpty() )
    {
        m_fontpath.Add( art2dPath + "fonts" );
        m_fontpath.Add( art2dPath + "fonts/liberation-fonts-ttf-2.00.1" );
        m_imagepath.Add( art2dPath + "layers" );
    }
    m_fontpath.Add( wxT( "./fonts" ) );
#if defined(__WXMSW__)
    m_fontpath.Add( wxGetOSDirectory() + wxT( "\\Fonts\\" ) );
#else
    m_fontpath.Add( "/usr/share/fonts/truetype" );
#endif // defined(__WXMSW__)

    m_arc2polyaber = 1;
    m_poly2arcaber = 1;

    m_drawingthreshold = 3;
    m_polygonFillThreshold = 0;
    m_asrectangle = false;//true;
    m_displayaberration = 0.5;
    a2dDoMu::SetAccuracy( 3 );
}

a2dGlobal::~a2dGlobal()
{
}

void a2dGlobal::Aberration( double angle, double radius, double& dphi, unsigned int& segments )
{
    ::Aberration( m_arc2polyaber, angle, radius, dphi, segments );
}


/**************************************************
    a2dDoMu
**************************************************/
int a2dDoMu::m_accuracy = 3;

a2dDoMu::a2dDoMu()
{
    m_multi = 1.0;
    m_number = 0.0;
}

a2dDoMu::a2dDoMu( double number, double multi, bool normalize )
{
    m_multi = multi;
    m_number = number;

    if ( normalize )
    {
        if ( multi >= 1e-12 && multi < 1e-9 )
        {
            m_multi = 1e-12;
            m_number = m_number * multi * 1e12;
        }
        else if ( multi >= 1e-9 && multi < 1e-6 )
        {
            m_multi = 1e-9;
            m_number = m_number * multi * 1e9;
        }
        else if ( multi >= 1e-6 && multi < 1e-3 )
        {
            m_multi = 1e-6;
            m_number = m_number * multi * 1e6;
        }
        else if ( multi >= 1e-3 && multi < 1e-2 )
        {
            m_multi = 1e-3;
            m_number = m_number * multi * 1e3;
        }
        else if ( multi >= 1e-2 && multi < 1e-1 )
        {
            m_multi = 1e-2;
            m_number = m_number * multi * 1e2;
        }
        else if ( multi >= 0.1 && multi < 1 )
        {
            m_multi = 0.1;
            m_number = m_number * multi * 0.1;
        }
        else if ( multi >= 1 && multi < 1000 )
        {
            m_multi = 1;
            m_number = m_number * multi;
        }
        else if ( multi == 0.00254 )
        {
            m_multi = 0.00254;
            m_number = m_number * multi / 0.00254;
        }
        else if ( multi == 0.0254 )
        {
            m_multi = 0.0254;
            m_number = m_number * multi / 0.0254;
        }
    }
}

a2dDoMu::a2dDoMu( double number, const wxString& multi )
{
    m_number = number;
    GetMultiplierFromString( multi, m_multi );
}

a2dDoMu::a2dDoMu( const wxString& number, const wxString& multi )
{
    number.ToDouble( &m_number );
    multi.ToDouble( &m_multi );
}

a2dDoMu::a2dDoMu( const wxString& numberMulti )
{
    Eval( numberMulti );
}

a2dDoMu::~a2dDoMu()
{}

wxString a2dDoMu::GetValueString() const
{
    return GetNumberString() + wxT( " " ) + GetMultiplierString();
}

double a2dDoMu::GetNumber() const
{
    return m_number;
}

double a2dDoMu::GetValue() const
{
    return m_number * m_multi;
}

wxString a2dDoMu::GetMultiplierString( double multi )
{
    wxString multistr = wxT( "" );

    if ( multi == 1e-12 )
        multistr = wxT( "pm" );
    else if ( multi == 1e-9 )
        multistr = wxT( "nm" );
    else if ( multi == 1e-6 )
        multistr = wxT( "um" );
    else if ( multi == 1e-3 )
        multistr = wxT( "mm" );
    else if ( multi == 1e-2 )
        multistr = wxT( "cm" );
    else if ( multi == 1e-1 )
        multistr = wxT( "dm" );
    else if ( multi == 1 )
        multistr = wxT( "m" );
    else if ( multi == 0.00254 )
        multistr = wxT( "mil" );
    else if ( multi == 0.0254 )
        multistr = wxT( "inch" );
    else
    {
        // treat as a number multiplier
        multistr.Printf( wxT( "%f" ), multi );
        multistr = wxT( "* " ) + multistr;
    }
    return multistr;
}

bool a2dDoMu::GetMultiplierFromString( const wxString& mul, double& multi )
{
    multi = 1;
    if ( !mul.IsEmpty() ) //is there a valid ascii multiplier
    {
        multi = 0;
        if ( mul == wxT( "pm" ) )
        {
            multi = 1e-12;
        }
        else if ( mul == wxT( "nm" ) )
        {
            multi = 1e-9;
        }
        else if ( mul == wxT( "um" ) )
        {
            multi = 1e-6;
        }
        else if ( mul == wxT( "mm" ) )
        {
            multi = 1e-3;
        }
        else if ( mul == wxT( "cm" ) )
        {
            multi = 1e-2;
        }
        else if ( mul == wxT( "dm" ) )
        {
            multi = 1e-1;
        }
        else if ( mul == wxT( "m" ) )
        {
            multi = 1;
        }
        else if ( mul == wxT( "e" ) )
        {
            multi = 0.00254;
        }
        else if ( mul == wxT( "mil" ) )
        {
            multi = 0.00254;
        }
        else if ( mul == wxT( "inch" ) )
        {
            multi = 0.0254;
        }
        if ( multi == 0 )
        {
            multi = 1;
            return false;
        }
    }
    return true;
}

double a2dDoMu::GetMultiplier() const
{
    return m_multi;
}

wxString a2dDoMu::GetMultiplierString() const
{
    return GetMultiplierString( m_multi );
}

wxString a2dDoMu::GetNumberString() const
{
    wxString numstr;
    if ( m_accuracy == -1 )
        numstr.Printf( wxT( "%f" ), m_number );
    else
        numstr.Printf( wxT( "%.*f" ), m_accuracy, m_number );
    return numstr;
}

// copy the contents of this class in another
a2dDoMu& a2dDoMu::operator=( const a2dDoMu& other )
{
    m_multi = other.m_multi;
    m_number = other.m_number;

    return *this;
}

// equal or not
int a2dDoMu::operator==( const a2dDoMu& other ) const
{
    if ( GetValue() == other.GetValue() )
        return 0;
    return 1;
}

int a2dDoMu::operator!=( const a2dDoMu& other ) const
{
    return ! ( GetValue() != GetValue() );
}

a2dDoMu& a2dDoMu::operator=( const wxChar* str )
{
    Eval( str );
    return *this;
}

a2dDoMu& a2dDoMu::operator=( double dd )
{
    m_number = dd;
    m_multi = 1;
    return *this;
}

a2dDoMu::operator double() const
{
    return GetValue();
}

bool a2dDoMu::Eval( const wxString& param )
{
    wxString multi;
    wxChar* endptr = 0;

    if ( param.Len() == 0 )
    {
        return false;
    }

#if wxUSE_UNICODE
    wcstod( param, &endptr );
#else
    strtod( param, &endptr );
#endif // wxUSE_UNICODE
    if ( endptr == param ) //no number found
    {
        m_number = 0;
        m_multi = 1;
    }
    else
    {
        wxString number = param.Left( endptr - param.c_str() );
        number.ToDouble( &m_number );
        multi = endptr;
    }

    multi.Trim( true );
    multi.Trim( false );

    if ( GetMultiplierFromString( multi, m_multi ) )
        return true;
    return false;
}

void a2dAET::CalculateLineParameters( const wxRealPoint& p1 , const wxRealPoint& p2 )
{
    double A = p2.y - p1.y; //A (y2-y1)
    if ( A == 0 )
    {
        m_horizontal = true;
        m_BdivA = 0;
        m_CdivA = 0;
    }
    else
    {
        m_horizontal = false;
        m_BdivA =  ( p1.x - p2.x ) / A; //B (x1-x2)
        //normalize
        m_CdivA =  ( ( p2.x * p1.y ) - ( p1.x * p2.y ) ) / A ;
    }
}

void a2dAET::CalculateXs( double y )
{
    m_xs = ( int ) ( -m_BdivA * y - m_CdivA );
}
