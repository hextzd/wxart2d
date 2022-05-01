/*! \file samples/boolonly/keyinout.cpp
    \brief read polygon data from a file based on simplified the KEY format
    \author Klaas Holwerda
 
    Copyright: 2001-2004 (C) Klaas Holwerda
 
    Licence: see kboollicense.txt 
 
    RCS-ID: $Id: keyinout.cpp,v 1.2 2006/11/05 15:32:23 titato Exp $
*/

#include <stdio.h>
#include <assert.h>
#include <string>
#include <math.h>
#include <stdlib.h>


#include "keyinout.h"
#include "kbool/booleng.h"


bool iequals(const string& a, const string& b)
{
    unsigned int sz = a.size();
    if (b.size() != sz)
        return false;
    for (unsigned int i = 0; i < sz; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}

FILE* GetPolygonsFromBoolEngKEYStart( string filename )
{
    FILE * file = fopen( filename.c_str(), "w" );

    fprintf( file, "\
             HEADER 5; \
             BGNLIB; \
             LASTMOD {2-11-15  15:39:21}; \
             LASTACC {2-11-15  15:39:21}; \
             LIBNAME trial; \
             UNITS; \
             USERUNITS 0.0001; PHYSUNITS 2.54e-009; \
             \
             BGNSTR;  \
             CREATION {2-11-15  15:39:21}; \
             LASTMOD  {2-11-15  15:39:21}; \
             STRNAME top; \
             ");

    return file;
}

void GetPolygonsFromBoolEngKEY( kbBool_Engine* booleng, FILE * file )
{
    // foreach resultant polygon in the booleng ...
    while ( booleng->StartPolygonGet() )
    {
        fprintf(file,"BOUNDARY; LAYER 2;  DATATYPE 0;\n");
        fprintf(file," XY % d; \n",booleng->GetNumPointsInPolygon()+1 );

        booleng->PolygonHasMorePoints();
        double firstx = booleng->GetPolygonXPoint();
        double firsty = booleng->GetPolygonYPoint();
        fprintf(file,"X % f;\t", firstx);
        fprintf(file,"Y % f; \n", firsty);

        // foreach point in the polygon
        while ( booleng->PolygonHasMorePoints() )
        {
            fprintf(file,"X % f;\t", booleng->GetPolygonXPoint());
            fprintf(file,"Y % f; \n", booleng->GetPolygonYPoint());
        }
        booleng->EndPolygonGet();
        fprintf(file,"X % f;\t", firstx);
        fprintf(file,"Y % f; \n", firsty);
        fprintf(file,"ENDEL;\n");
    }
}

void GetPolygonsFromBoolEngKEYStop( FILE * file )
{
    fprintf(file,"\
            ENDSTR top; \
            ENDLIB; \
            ");
    fclose (file);
}

#define wxT(x) x

// ----------------------------------------------------------------------------
// a2dIOHandlerKeyIn
// ----------------------------------------------------------------------------
//! GDSII format input and output
/*!
    \ingroup fileio gdsfileio
*/
class GDS_Error
{
public:
    GDS_Error( const string& message, const string& header = wxT( "" ) );
    GDS_Error( const GDS_Error& other );
    ~GDS_Error();

    string& GetErrorMessage() { return m_message; }
    string& GetHeaderMessage() { return m_header; }

protected:

    string m_message;
    string m_header;
};

GDS_Error::GDS_Error( const string& message, const string& header )
{
    m_message = message;
    m_header = header;
}

GDS_Error::GDS_Error( const GDS_Error& other )
{
    m_message = other.m_message;
    m_header = other.m_header;
}

GDS_Error::~GDS_Error()
{
}

// strips all the spaces and tabs from the left of the string
//  <blanks>            ::= { TAB | SPACE }*
bool a2dIOHandlerKeyIn::Blanks()
{
    int i = 0;
    // delete blanks at the beginning
    while ( a == wxT( ' ' ) || a == wxT( '\t' ) || a == wxT( '\\' ) || a == 0xd  )
    {
        if ( a == wxT( '\\' ) )
        {
            if ( PeekNextC() != wxT( '\n' ) )
                break;
            else
                IncC(); i++;
        }
        IncC(); i++;
    }

    return ( i != 0 );
}

//  <comment>       ::= # {CHAR}*
bool a2dIOHandlerKeyIn::Comment()
{
    if( a != wxT( '#' ) ) //quote must be there
    {
        m_error_mes = wxT( "wrong comment" );
        return false;
    }
    m_b.clear();

    m_b += a;
    IncC();

    while( a != wxT( '\0' ) )
    {
        if ( a == wxT( '\\' ) )
        {
            IncC();
            if ( a == wxT( '\n' ) )
            {
                m_b += wxT( ' ' );
                IncC();
                continue;
            }
        }
        if ( a == wxT( '\n' ) )
            break; //end of comment

        m_b += a;
        IncC();
    }
    return true;
}

// <word> ::=  { <quotedstring> | <bracedstring> | <normalword> }
bool a2dIOHandlerKeyIn::Word()
{
    Blanks();
    m_b.clear();

    if ( a == wxT( '"' ) )
    {
        if ( !QuotedString() )
            return false;
    }
    else if ( a == wxT( '{' ) )
    {
        if ( !BracedString() )
            return false;
    }
    else
    {
        if ( !NormalWord() )
            return false;
    }

    return true;
}

//<multipartword>   ::=   {CHAR}*
bool a2dIOHandlerKeyIn::NormalWord()
{
    while( a != wxT( ' ' ) && a != wxT( '\t' ) && a != wxT( '\0' ) && a != wxT( '\n' ) && a != wxT( ';' ) )
    {
        if ( a == wxT( '\\' ) ) //take the next character
        {
            IncC();

            if ( a == wxT( ' ' ) || a == wxT( '\t' ) || a == wxT( '\0' ) || a == wxT( ';' ) )
                break; //treat it as word seperator

            //all other backslash character take them
            m_b += a;
            IncC();
            continue;
        }

        //all other characters
        m_b += a;
        IncC();
    }
    return true;
}

//<quotedstring>        ::= ('"') {CHAR | '\"' | '\\n' }* ('"')
bool a2dIOHandlerKeyIn::QuotedString()
{
    if( a != wxT( '"' ) ) //quote must be there
    {
        m_error_mes = wxT( "wrong quotedstring, begin quote missing" );
        return false;
    }

    IncC();

    while( a != wxT( '"' ) && a != wxT( '\0' ) )
    {
        if ( a == wxT( '\\' ) ) //take the next character
        {
            IncC();
            if ( a == wxT( '\0' ) )
                break; //error handled later

            m_b += a;
            IncC();
            continue;
        }

        //all other characters
        m_b += a;
        IncC();
    }

    if( a != wxT( '"' ) ) //quote must be there
    {
        m_error_mes = wxT( "wrong quotedstring, end quote missing" );
        return false;
    }
    IncC();

    return true;
}


//<bracedstring>        ::= ('{') {CHAR | '\{' | '\}' | '\\n'}* ('}')
bool a2dIOHandlerKeyIn::BracedString()
{
    if( a != wxT( '{' ) ) //quote must be there
    {
        m_error_mes = wxT( "wrong bracedstring, begin brace missing" );
        return false;
    }

    IncC();

    int brnr = 0;
    while( ( a != wxT( '}' ) || brnr ) && a != wxT( '\0' ) )
    {
        if ( a == '\\' ) //take the next character
        {
            IncC();
            if ( a == wxT( '\0' ) )
                break; //error handled later

            m_b += a;
            IncC();
            continue;
        }

        //nested braces are allowed
        if ( a == wxT( '{' ) ) //nested brace
            brnr++;

        if ( a == wxT( '}' ) ) //nested brace
            brnr--;

        //all other characters
        m_b += a;
        IncC();
    }


    if( a != wxT( '}' ) ) //quote must be there
    {
        m_error_mes = wxT( "wrong bracedstring, end brace missing" );
        return false;
    }
    IncC();

    return true;
}

bool a2dIOHandlerKeyIn::GetCommand()
{
    m_keyword.clear();
    m_value.clear();
    m_error_mes.clear();

    if ( !m_linenumber ) //the very first lien will be parsed
        m_linenumber++;


    // already skip blanks in front of next command
    Blanks();

    while( a == wxT( '\0' ) || a == wxT( '\n' ) || a == wxT( ';' ) ) //skip empty command
    {
        if ( a == wxT( '\0' ) ) //end of file no error will be set
        {
            return false;
        }
        IncC();
        Blanks();
    }

    if ( a == wxT( '\0' ) ) //end of file no error will be set
        return false;
    else if ( a == wxT( '#' ) )
    {
        if ( !Comment() )   //it is comment
        {
            m_error_mes = wxT( "comment wrong" );
            return false;
        }

        m_keyword = "comment";
        return true;
    }
    else
    {
        m_b.clear();

        if ( !NormalWord() )
            return false;

        m_keyword = m_b;

        Word();

        if ( m_error_mes.size() )
            return false;

        Blanks();

        // <endcommand>   ::= '\0' | ';'
        if( a == wxT( '\0' ) || a == wxT( '\n' ) || a == wxT( ';' ) ) //not at end of m_command
            IncC();
        else
        {
            m_error_mes = wxT( "; or EOL expected" );
            return false;
        }

        //skip empty commands
        Blanks();

        if( a == wxT( '\0' ) || a == wxT( '\n' ) || a == wxT( ';' ) )
            IncC();

        m_value = m_b;
    }

    return true;
}

// ReadItem (parser) --------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadItem( const string& type )
{
//  <item>      ::= KEYWORD VALUE
    if ( m_back )
    {
        if ( iequals( m_keyword, type ) )
            m_back = false;
        else
            m_back = true;
        return bool( !m_back );
    }

    string buf;
    bool Next = true;
    do
    {
        if( !GetCommand() )
        {
            if ( !m_error_mes.empty() )
            {
              char buff[100];
              sprintf( buff, "Could not Parse line %d: \n Error: %s",
                            m_linenumber, m_error_mes.c_str() );
              m_b = buff;

                throw GDS_Error( buff, wxT( "Command Parsing Error" ) );

            }
            return false;
        }
        if ( m_keyword == wxT( "comment" ) )
            Next = true;
        else if ( m_keyword == wxT( "" ) )
            Next = true;
        else
            Next = false;
    }
    while ( Next );

    if ( m_value.empty() )
    {
        string buf;
        buf = wxT( "Argument missing (in " );
        if ( iequals( m_keyword, type ) )
        {
            // These commands don't have arguments...
            if ( iequals( m_keyword, wxT( "BGNLIB" ) ) );
            //else if (iequals( m_keyword,wxT("UNITS")));
            else if ( iequals( m_keyword, wxT( "BGNSTR" ) ) );
            else if ( iequals( m_keyword, wxT( "BOUNDARY" ) ) );
            else if ( iequals( m_keyword, wxT( "ENDEL" ) ) );
            else if ( iequals( m_keyword, wxT( "ENDLIB" ) ) );
            else if ( iequals( m_keyword, wxT( "ENDMASKS" ) ) );
            else
            {
                buf += m_keyword;
                buf += wxT( ")" );
                throw GDS_Error( buf ); // There is no argument, so throw an error...
            }
        }
    }

    if ( iequals( m_keyword, type ) )
        m_back = false;
    else
        m_back = true;
    return bool( !m_back );
}

double a2dIOHandlerKeyIn::ReadDouble()
{
    return m_scale_in * atof( m_value.c_str() );
}

char a2dIOHandlerKeyIn::PeekNextC()
{
    char p = ( char ) m_streami->peek();
    return p;
}

void a2dIOHandlerKeyIn::IncC()
{
    a = ( char ) m_streami->get();

    if ( a == wxT( '\0' ) || a == wxT( '\n' )  )
        m_linenumber++;


    if ( m_streami->eof() )
        a = wxT( '\0' );
    if ( a == char( EOF ) )
        a = wxT( '\0' );
    if ( a == wxT( '\r' ) )
        a = wxT( ' ' );
}

// -------------- use it -----------------

a2dIOHandlerKeyIn::a2dIOHandlerKeyIn( kbBool_Engine* booleng )
{
    m_booleng = booleng;
    m_scale_in = 1;
    m_back = false;
    m_keyword.clear();
    m_value.clear();

    m_refMaxx = 10; 
    m_dx = m_dy = 0;
}

a2dIOHandlerKeyIn::~a2dIOHandlerKeyIn()
{
}

void a2dIOHandlerKeyIn::InitializeLoad(  )
{
    m_linenumber = 0;
    IncC(); //get the first character;
}

bool a2dIOHandlerKeyIn::Load( a2dDocumentInputStream& stream )
{
    m_streami = &stream;

    InitializeLoad();

    m_recordsize      = 0;
    m_recordtype      = 0;
    m_back            = false;

    ReadKey();

    return true;
}

// GdsII --------------------------------------------------------------------
void a2dIOHandlerKeyIn::ReadKey()
{
    // Grammar: HEADER BGNLIB LIBNAME [REFLIBS] [FONTS]
    //              [ATTRTABLE] [GENERATION] [<FormatType>]
    //              UNITS {<Structure>}* ENDLIB
    //

//  <stream format> ::= HEADER
//                                  BGNLIB
//                                  <LibrarySettings>
//                                  {<Structure>}*
//                                  ENDLIB

    if ( !ReadItem( wxT( "header" ) ) ) // Header expected here
        throw GDS_Error( wxT( "HEADER is missing (in KEY-file)" ) );

    if ( !ReadBgnlib() )
        throw GDS_Error( wxT( "BGNLIB is missing (in KEY-file)" ) );
    if ( !ReadLibrarySettings() )
        throw GDS_Error( wxT( "LIBNAME is missing (in KEY-file)" ) );
    while ( ReadStructure() )
    {
    }
    if ( !ReadItem( wxT( "endlib" ) ) )
        throw GDS_Error( wxT( "ENDLIB is missing (in KEY-file)" ) );
}

// Bgnlib -------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadBgnlib()
{
    if( !ReadItem( wxT( "bgnlib" ) ) ) // BgnLib expected here
        return false;
    if( !ReadItem( wxT( "lastmod" ) ) )
        throw GDS_Error( wxT( "BGNLIB: LASTMOD is missing (in KEY-file)" ) );

    string _int_str_ptr = m_value;


    if( !ReadItem( wxT( "lastacc" ) ) )
        throw GDS_Error( wxT( "BGNLIB: LASTACC is missing (in KEY-file)" ) );

    _int_str_ptr = m_value;


    return( true );
}

// Library ------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadLibrarySettings()
{
    if( !ReadItem( wxT( "libname" ) ) ) // BgnLib expected here
        throw GDS_Error( wxT( "libname record missing (in KEY)" ), wxT( "Error" ) );

    ReadItem( wxT( "reflibs" ) ); // RefLibs optional
    ReadItem( wxT( "fonts" ) );
    ReadItem( wxT( "attrtable" ) );
    ReadItem( wxT( "generations" ) );
    if ( ReadItem( wxT( "format" ) ) )
    {
        //skip rest for the moment
        while( !ReadItem( wxT( "endmasks" ) ) )
        {
            if ( ReadItem( wxT( "mask" ) ) )
            {
                string layerstr = m_value;
                int layernr = 0;

                while( 1 )
                {
                    if ( ReadItem( wxT( "layernr" ) ) )
                    {
                        layernr = atoi( m_value.c_str() );
                    }
                    else if ( ReadItem( wxT( "color" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "trans" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "binding" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "linestyle" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "stream" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "iges" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "type" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "dxf" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "plotmode" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "protected" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "visible" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "patternnr" ) ) )
                    {
                    }
                    else if ( iequals( m_keyword, wxT( "mask" ) ) == 0 || iequals( m_keyword, wxT( "endmasks" ) ) == 0 )
                        break;
                    else
                        m_back = false; //skip this
                }
            }
        }
    }

    if( !ReadItem( wxT( "units" ) ) ) // Units expected here
        return false;

    //m_doc->SetUnits( m_value );

    if( !ReadItem( wxT( "userunits" ) ) )
        throw GDS_Error( wxT( "USERUNITS missing (in KEY-file)" ) );
    m_userunits_out = ( double ) atof( m_value.c_str() );

    //m_doc->SetUnitsAccuracy( m_userunits_out );

    if( !ReadItem( wxT( "physunits" ) ) )
        throw GDS_Error( wxT( "PHYSUNITS missing (in KEY-file)" ) );

    double metersScale = ( ( double ) atof( m_value.c_str() ) ) / m_userunits_out;
    
    //m_doc->SetUnitsScale( metersScale );

    return true;
}

// Read Element -------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadElement()
{
    if  ( ReadBoundary( ) )
    {
    }
    else if ( ReadItem( wxT( "sref" ) ) ||
              ReadItem( wxT( "path" ) ) ||
              ReadItem( wxT( "text" ) ) ||
              ReadItem( wxT( "circle" ) ) ||
              ReadItem( wxT( "aref" ) ) ||
              ReadItem( wxT( "box" ) ) ||
              ReadItem( wxT( "line" ) ) ||
              ReadItem( wxT( "arc" ) ) ||
              ReadItem( wxT( "surface" ) ) 
        )
    {
        //skip all others
        while ( !ReadItem( wxT( "endel" ) ) )
            m_back = false;
        m_back = true;
    }
    else 
        return false;

    if ( !ReadItem( wxT( "endel" ) ) )
        throw GDS_Error( wxT( "ENDEL is missing (in KEY-file) (too many points in element?)" ) );
    return true;
}


// Boundary -----------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadBoundary()
{
// <boundary>   ::= BOUNDARY [ELFLAGS] [PLEX] LAYER DATATYPE [WIDTH] XY

    if ( !ReadItem( wxT( "boundary" ) ) )
        return false;

    ReadElflags();

    ReadItem( wxT( "plex" ) );

    if ( !ReadLayer() )
        throw GDS_Error( wxT( "Boundary: LAYER missing (in KEY-file)" ) );

    int layernr = atoi( m_value.c_str() );

    if ( layernr == 0 )
        m_booleng->StartPolygonAdd( GROUP_A );
    else if ( layernr == 1 )
        m_booleng->StartPolygonAdd( GROUP_B );
    else
        throw GDS_Error( wxT( "Boundary: LAYER only layer 0 or 1 allowed" ) );

    ReadItem( wxT( "datatype" ) );

    if ( ReadItem( wxT( "spline" ) ) )
    {}

    if ( ReadItem( wxT( "width" ) ) )
    {}

    if ( !ReadPolygon() )
        throw GDS_Error( wxT( "Header: XY missing (in Boundary)" ) );

    m_booleng->EndPolygonAdd();

    return true;
}

// Elflags ------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadElflags()
{
    if ( !ReadItem( wxT( "elflags" ) ) )
        return false;
    return true;
}

// Layer --------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadLayer(  )
{
    if ( !ReadItem( wxT( "layer" ) ) )
        return false;

    //atoi( m_value );
    return true;
}

// Point --------------------------------------------------------------------
bool a2dIOHandlerKeyIn::Read( kbPoint& Point )
{
    if ( !ReadItem( wxT( "x" ) ) )
        throw GDS_Error( wxT( "Point: X is missing (in KEY-file)" ) );
    Point.m_x = ReadDouble();
    if ( !ReadItem( wxT( "y" ) ) )
        throw GDS_Error( wxT( "Point: Y is missing (in KEY-file)" ) );
    Point.m_y = ReadDouble();
    return true;
}

// Strans -------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadStrans()
{
//  <strans>                ::= STRANS [MAG] [ANGLE]
    if ( !ReadItem( wxT( "strans" ) ) )
        return false;

    string _int_str_ptr = m_value;

    if ( ReadItem( wxT( "mag" ) ) )
        ReadDouble();

    if ( ReadItem( wxT( "angle" ) ) )
        ReadDouble();

    return true;
}

// Structure ----------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadStructure()
{
    // Grammar: BGNSTR STRNAME [STRCLASS] {<element>}*
    //              ENDSTR

    if( !ReadItem( wxT( "bgnstr" ) ) ) // BgnStr expected here
        return false;

    if( !ReadItem( wxT( "creation" ) ) )
        throw GDS_Error( wxT( "Structure: CREATION is missing (in KEY-file)" ) );

    string _int_str_ptr = m_value;

    if( !ReadItem( wxT( "lastmod" ) ) )
        throw GDS_Error( wxT( "Structure: LASTACC is missing (in KEY-file)" ) );

    if ( !ReadItem( wxT( "strname" ) ) )
        throw GDS_Error( wxT( "Structure: STRNAME missing (in KEY-file)" ) );

    m_structures.push_back( m_value );

    if ( m_value == m_wantedStructure )
    {
        ReadItem( wxT( "strclass" ) ); // Just read it

        while ( ReadElement() )
            ;

        if ( !ReadItem( wxT( "endstr" ) ) )
        {
            char errbuf[200];
            sprintf( errbuf, wxT( "Unknown Element: %s (in KEY-file)" ), m_keyword.c_str() );
            throw GDS_Error( errbuf );
        }
    }
    else
    {
        while ( !ReadItem( wxT( "endstr" ) ) )
            m_back = false;

    }
    return true;
}



bool a2dIOHandlerKeyIn::ReadPolygon()
{
    if ( !ReadItem( wxT( "xy" ) ) )
        return false;

    int j = atoi( m_value.c_str() );
    int i;

    double x, y, xp, yp;
    bool lastWasArc = false;
    bool firstRead = false;

    for ( i = 0; i < j - 1; i++ )
    {
        if ( ReadItem( wxT( "st" ) ) ) //read segtype
        {
        }

        if ( !ReadItem( wxT( "x" ) ) ) //read x
            throw GDS_Error( wxT( "Polygon: X is missing (in KEY-file)" ) );

        x = ReadDouble();

        if ( !ReadItem( wxT( "y" ) ) ) //read y
            throw GDS_Error( wxT( "Polygon: Y is missing (in KEY-file)" ) );

        y = ReadDouble();

        if ( firstRead )
        {
            m_booleng->AddPoint( x, y );
        }
        else
        {
            m_booleng->AddPoint( x, y );
            firstRead = true;
        }

        if ( !iequals( m_keyword, wxT( "endel" ) ) || !iequals( m_keyword, wxT( "x" ) ) || !iequals( m_keyword, wxT( "st" ) ) ) // Read is already done
        {
            xp = x;
            yp = y;
        }
        else
            throw GDS_Error( wxT( "Polygon: X or XM missing (in KEY-file)" ) );
    }

    //last POINT must be eqaul first.

    if ( !ReadItem( wxT( "x" ) ) ) //read x
        throw GDS_Error( wxT( "Polygon: X is missing (in KEY-file)" ) );

    x = ReadDouble();

    if ( !ReadItem( wxT( "y" ) ) ) //read y
        throw GDS_Error( wxT( "Polygon: Y is missing (in KEY-file)" ) );

    y = ReadDouble();

    if ( ReadItem( wxT( "xm" ) ) ) //last point may not be arc
        throw GDS_Error( wxT( "Polygon: expected ENDEL found XM (in KEY-file)" ) );

    return true;
}







