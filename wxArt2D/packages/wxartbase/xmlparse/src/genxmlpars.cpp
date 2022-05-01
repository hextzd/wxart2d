/*! \file xmlparse/src/genxmlpars.cpp
    \brief a2dDocument - XML reader via Expat
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: genxmlpars.cpp,v 1.46 2009/07/15 21:23:43 titato Exp $
*/

#include "wxartbaseprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxartbaseprivate.h"

#if wxART2D_USE_XMLPARSE

#include "wx/wfstream.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/strconv.h"
#include <wx/tokenzr.h>

#include "wx/xmlparse/genxmlpars.h"
#include "wx/xmlparse/xmlencode.h"


//----------------------------------------------------------------------------
// a2dXMLTag
//----------------------------------------------------------------------------

a2dXMLTag::a2dXMLTag( XMLeventType type, wxString tagname )
{
    m_tag = tagname;
    m_type = type;
    m_line = 0;
    m_column = 0;
    m_hasContent = false;
    m_hasattributes = false;
}

a2dXMLTag::a2dXMLTag( const a2dXMLTag& other )
{
    m_tag = other.m_tag;
    m_type = other.m_type;
    m_hasContent = false;
    m_hasattributes = false;
    m_attributes = other.m_attributes;
    m_line = 0;
    m_column = 0;
}

a2dXMLTag::~a2dXMLTag()
{
}

a2dObject* a2dXMLTag::Clone( CloneOptions WXUNUSED( options ) ) const
{
    return new a2dXMLTag( *this );
}

bool a2dXMLTag::SetAttribute( const wxString& attributeName, const wxString& Value )
{
    a2dAttributeHash::iterator iter = m_attributes.find( attributeName );
    m_attributes[ attributeName ] = Value;
    return iter != m_attributes.end();
}

wxString* a2dXMLTag::GetAttribute( const wxString& attributeName )
{
    a2dAttributeHash::iterator iter = m_attributes.find( attributeName );
    if( iter == m_attributes.end() )
        return NULL;// error, alias does not exist

    return &iter->second;
}

#if wxART2D_USE_CVGIO

void a2dXMLTag::DoSave( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ), a2dObjectList* WXUNUSED( towrite ) )
{
    wxASSERT( 0 );
}

#endif //wxART2D_USE_CVGIO

#if wxART2D_USE_CVGIO

void a2dXMLTag::DoLoad( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) )
{
    wxASSERT( 0 );
}

#endif //wxART2D_USE_CVGIO

wxString GetTypeString( const XMLeventType& type )
{
    switch ( type )
    {
        case START_DOCUMENT: { return wxT( "START_DOCUMENT" ); break; }
        case END_DOCUMENT:   { return wxT( "END_DOCUMENT" ); break;  }
        case START_TAG:      { return wxT( "START_TAG" ); break;  }
        case END_TAG:        { return wxT( "END_TAG" ); break;  }
        case CONTENT:        { return wxT( "CONTENT" ); break;  }
        default: break;
    }
    return wxT( "" );
}

//----------------------------------------------------------------------------
// a2dIOHandlerXMLPull
//----------------------------------------------------------------------------

wxString a2dIOHandlerXMLPull::m_encoding = wxT( "UTF-8" );

wxString a2dIOHandlerXMLPull::m_version = wxT( "1.0" );

a2dIOHandlerXMLPull::a2dIOHandlerXMLPull()
{
    m_depth = 0;
    m_current = 0 ;
    m_startdoc = new a2dXMLTag( START_DOCUMENT, wxT( "startdocument" ) );
    m_enddoc = new a2dXMLTag( END_DOCUMENT, wxT( "enddocument" ) );
    m_scale = 1;
}

a2dIOHandlerXMLPull::~a2dIOHandlerXMLPull()
{
    delete m_startdoc;
    delete m_enddoc;
}

void a2dIOHandlerXMLPull::InitializeLoad()
{
    m_depth = 0;
    m_current = m_startdoc;
    a2dIOHandlerStrIn::InitializeLoad();
}

void a2dIOHandlerXMLPull::ResetLoad( )
{
    m_elements.clear();
    m_current = 0;

    a2dIOHandlerStrIn::ResetLoad( );
}

double a2dIOHandlerXMLPull::ParseDouble( const wxString& buffer, unsigned int& position )
{
    const wxChar* start = buffer.c_str() + ( long ) position;
    wxChar* end;
    double val = wxStrtod( start, &end );
    position = end - buffer.c_str();

    if ( start == end )
        throw a2dIOHandlerXMLException( wxT( "not a number near: " ), GetCurrentLineNumber(), GetCurrentColumnNumber() + position );

    return val;
}

void a2dIOHandlerXMLPull::SkipCommaSpaces( const wxString& buffer, unsigned int& position )
{
    char current;
    for( ; position < buffer.length(); )
    {
        current = buffer[position];
        switch ( current )
        {
            default:
                return;
            case wxT( ',' ): case 0x20: case 0x09: case 0x0D: case 0x0A:
                position++;
                break;
        }
    }
}

void a2dIOHandlerXMLPull::SkipSpaces( const wxString& buffer, unsigned int& position )
{
    char current;
    for( ; position < buffer.length() ; )
    {
        current = buffer[position];
        switch ( current )
        {
            default:
                return;
            case 0x20: case 0x09: case 0x0D: case 0x0A:
                position++;
                break;
        }
    }
}

int a2dIOHandlerXMLPull::GetCurrentColumnNumber()
{
    if ( m_current )
        return m_current->m_column;
    return -1;
}

int a2dIOHandlerXMLPull::GetCurrentLineNumber()
{
    if ( m_current )
        return m_current->m_line;
    return -1;
}

wxString a2dIOHandlerXMLPull::GetErrorString()
{
    return "";
}


void a2dIOHandlerXMLPull::Require( const XMLeventType& type, wxString name )
{
    if ( !m_elements.empty() )
    {
        if ( m_current->m_type != type || m_current->m_tag != name )
        {

            a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "expected: %s '%s'\ngot %s '%s' at line %d" ), GetTypeString( type ).c_str(), name.c_str(),
                                             GetTypeString( m_current->m_type ).c_str(), m_current->m_tag.c_str(), GetCurrentLineNumber() );
            throw a2dIOHandlerXMLException( wxT( "expected:" ) + name , GetCurrentLineNumber(), GetCurrentColumnNumber() );
        }
        return;
    }
    else
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "XML end of file reached  at line %d" ), GetCurrentLineNumber() );
        throw a2dIOHandlerXMLException( wxT( "XML end of file reached" ), GetCurrentLineNumber(), GetCurrentColumnNumber() );
    }
}


XMLeventType a2dIOHandlerXMLPull::NextTag()
{
    XMLeventType eventType = Next();

    if ( eventType != START_TAG && eventType != END_TAG )
    {
        throw a2dIOHandlerXMLException( wxT( "expected start or end tag" ) , GetCurrentLineNumber(), GetCurrentColumnNumber() );
    }
    return eventType;
}

XMLeventType a2dIOHandlerXMLPull::Next()
{
    if ( !m_elements.empty() )
    {
        m_elements.pop_front();
        m_depth--;

        if ( !m_elements.empty() )
        {
            m_depth++;
            m_current = m_elements.front();
            return m_current->m_type;
        }
    }

    FillQueue();

    if ( !m_elements.empty() )
    {
        m_depth++;
        m_current = m_elements.front();
        return m_current->m_type;
    }

    m_current = m_enddoc;
    return END_DOCUMENT;
}

wxString a2dIOHandlerXMLPull::GetAttributeValue( const wxString& attrib, const wxString& defaultv )
{
    if ( m_current->m_type != START_TAG )
        throw a2dIOHandlerXMLException( wxT( "GetAttributeValue on non starttag" ) , GetCurrentLineNumber(), GetCurrentColumnNumber() );

    wxString* val = m_current->GetAttribute( attrib );
    if ( m_current->GetAttribute( attrib ) )
        return *val;
    return defaultv;
}

wxString a2dIOHandlerXMLPull::RequireAttributeValue( const wxString& attrib )
{
    if ( m_current->m_type != START_TAG )
        throw a2dIOHandlerXMLException( wxT( "GetAttributeValue on non starttag" ) , GetCurrentLineNumber(), GetCurrentColumnNumber() );

    wxString* val = m_current->GetAttribute( attrib );
    if ( val )
        return *val;

    throw a2dIOHandlerXMLException( wxT( "No such attribute: " ) + attrib , GetCurrentLineNumber(), GetCurrentColumnNumber() );
    return *val;
}

double a2dIOHandlerXMLPull::GetAttributeValueDouble( const wxString& attrib, double defaultv )
{
    wxString s = GetAttributeValue( attrib );
    if ( !s.IsEmpty() )
    {
        double num;
        s.ToCDouble( &num );
        return num;
    }

    return defaultv;
}

double a2dIOHandlerXMLPull::RequireAttributeValueDouble( const wxString& attrib )
{
    wxString s = RequireAttributeValue( attrib );
    double num;
    s.ToCDouble( &num );
    return num;
}

wxLongLong a2dIOHandlerXMLPull::GetAttributeValueLongLong( const wxString& attrib, wxLongLong defaultv )
{
    wxString s = GetAttributeValue( attrib );
    if ( !s.IsEmpty() )
    {
        wxLongLong_t num;
        s.ToLongLong( &num );
        return num;
    }

    return defaultv;
}

wxLongLong a2dIOHandlerXMLPull::RequireAttributeValueLongLong( const wxString& attrib )
{
    wxString s = RequireAttributeValue( attrib );
    wxLongLong_t num;
    s.ToLongLong( &num );
    return num;
}

int a2dIOHandlerXMLPull::GetAttributeValueInt( const wxString& attrib, int defaultv )
{
    wxString s = GetAttributeValue( attrib );
    if ( !s.IsEmpty() )
    {
        long num;
        s.ToLong( &num );
        return num;
    }

    return defaultv;
}

int a2dIOHandlerXMLPull::RequireAttributeValueInt( const wxString& attrib )
{
    wxString s = RequireAttributeValue( attrib );
    long num;
    s.ToLong( &num );
    return num;
}

long a2dIOHandlerXMLPull::GetAttributeValueLong( const wxString& attrib, long defaultv )
{
    wxString s = GetAttributeValue( attrib );
    if ( !s.IsEmpty() )
    {
        long num;
        s.ToLong( &num );
        return num;
    }

    return defaultv;
}

long a2dIOHandlerXMLPull::RequireAttributeValueLong( const wxString& attrib )
{
    wxString s = RequireAttributeValue( attrib );
    long num;
    s.ToLong( &num );
    return num;
}

bool a2dIOHandlerXMLPull::GetAttributeValueBool( const wxString& attrib, bool defaultv )
{
    wxString s = GetAttributeValue( attrib );
    if ( !s.IsEmpty() )
    {
        if ( s.MakeLower() == wxT( "true" ) )
            return true;
        else
            return false;
    }

    return defaultv;
}

bool a2dIOHandlerXMLPull::RequireAttributeValueBool( const wxString& attrib )
{
    wxString s = RequireAttributeValue( attrib );
    if ( s.MakeLower() == wxT( "true" ) )
        return true;
    else
        return false;
}

bool a2dIOHandlerXMLPull::HasAttribute( const wxString& attrib )
{
    return m_current->GetAttribute( attrib ) != NULL;
}

wxString a2dIOHandlerXMLPull::GetTagName()
{
    return m_current->m_tag;
}

XMLeventType a2dIOHandlerXMLPull::GetEventType()
{
    return m_current->m_type;
}

void a2dIOHandlerXMLPull::SkipSubTree()
{
    int sublevel = 1;

    while( sublevel > 0 )
    {
        XMLeventType type = Next();
        switch( type )
        {
            case START_TAG:
                ++sublevel;
                break;
            case END_TAG:
                --sublevel;
                break;
            default:
                break;
        }
    }
}

//----------------------------------------------------------------------------
// a2dIOHandlerXMLPullExpat
//----------------------------------------------------------------------------

a2dIOHandlerXMLPullExpat::a2dIOHandlerXMLPullExpat()
{
    m_parser = NULL;
    m_buffer = NULL;
    m_done = false;
}

a2dIOHandlerXMLPullExpat::~a2dIOHandlerXMLPullExpat()
{
    if ( m_parser )
        XML_ParserFree( m_parser );
}

void a2dIOHandlerXMLPullExpat::InitializeLoad()
{
    assert( m_buffer == NULL );

    m_buffer = NULL;
    m_done = false;
    a2dIOHandlerXMLPull::InitializeLoad();

    assert( m_parser == NULL );
    m_parser = XML_ParserCreate( NULL );

    //the C++ parser object itself as userdata
    XML_SetUserData( m_parser, ( void* ) this );

    XML_SetElementHandler( m_parser, BaseStartElementHnd, BaseEndElementHnd );
    XML_SetCharacterDataHandler( m_parser, BaseCharacterDataHnd );
    XML_SetCommentHandler( m_parser, BaseCommentHnd );
    XML_SetDefaultHandler( m_parser, BaseDefaultHnd );

}

void a2dIOHandlerXMLPullExpat::ResetLoad( )
{
    m_buffer = NULL;
    m_done = false;

    a2dIOHandlerXMLPull::ResetLoad( );

    if ( m_parser )
    {
        XML_ParserFree( m_parser );
        m_parser = NULL;
    }
}

int a2dIOHandlerXMLPullExpat::GetExpatCurrentColumnNumber()
{
    if ( m_parser )
        return XML_GetCurrentColumnNumber( m_parser );
    return 0;
}

int a2dIOHandlerXMLPullExpat::GetExpatCurrentLineNumber()
{
    if ( m_parser )
        return XML_GetCurrentLineNumber( m_parser );
    return 0;
}

wxString a2dIOHandlerXMLPullExpat::GetExpatErrorString()
{
    if ( m_parser )
        return XmlDecodeStringToString( XML_ErrorString( XML_GetErrorCode( m_parser ) ) );
    return wxT( "" );
}

void a2dIOHandlerXMLPullExpat::BaseStartElementHnd( void* userData, const char* name, const char** atts )
{
    a2dIOHandlerXMLPullExpat* parser = ( a2dIOHandlerXMLPullExpat* ) userData;

    a2dXMLTag* tag = new a2dXMLTag( START_TAG, XmlDecodeStringToString( name ) );

    int i = 0;
    while ( atts[i] )
    {
        tag->SetAttribute( XmlDecodeStringToString( atts[i] ), XmlDecodeStringToString( atts[i + 1] ) );
        i += 2;
    }

    tag->m_line = parser->GetExpatCurrentLineNumber();
    tag->m_column = parser->GetExpatCurrentColumnNumber();
    parser->m_elements.push_back( tag );
}

void a2dIOHandlerXMLPullExpat::BaseEndElementHnd( void* userData, const char* name )
{
    a2dIOHandlerXMLPullExpat* parser = ( a2dIOHandlerXMLPullExpat* ) userData;

    a2dXMLTag* tag = new a2dXMLTag( END_TAG, XmlDecodeStringToString( name ) );
    parser->m_elements.push_back( tag );
    tag->m_line = parser->GetExpatCurrentLineNumber();
    tag->m_column = parser->GetExpatCurrentColumnNumber();
}

void a2dIOHandlerXMLPullExpat::BaseCharacterDataHnd( void* userData, const char* s, int len )
{
    a2dIOHandlerXMLPullExpat* parser = ( a2dIOHandlerXMLPullExpat* ) userData;

    char* buf = new char[len + 1];

    buf[len] = wxT( '\0' );
    memcpy( buf, s, ( size_t )len );

    //TODO like this no mixed content
    //either search last open tag ( without a matching endtag )
    //or create a a2dXMLTag with only content and of type CONTENT
    //of which there may be several in a row or concatenate.

    if ( !parser->m_elements.empty() )
        parser->m_elements.back()->m_text += XmlDecodeStringToString( buf );

    delete [] buf;
}

void a2dIOHandlerXMLPullExpat::BaseCommentHnd( void* userData, const char* data )
{
    a2dIOHandlerXMLPullExpat* parser = ( a2dIOHandlerXMLPullExpat* ) userData;
}

void a2dIOHandlerXMLPullExpat::BaseDefaultHnd( void* userData, const char* s, int len )
{
    a2dIOHandlerXMLPullExpat* parser = ( a2dIOHandlerXMLPullExpat* ) userData;

    wxString buf = XmlDecodeStringToString( s, ( size_t )len );

    // XML header:
    if ( len > 6 && memcmp( s, "<?xml ", 6 ) == 0 )
    {
        wxString buf = XmlDecodeStringToString( s, ( size_t )len );
        int pos;
        pos = buf.Find( wxT( "encoding=" ) );
        if ( pos != wxNOT_FOUND )
            m_encoding = buf.Mid( pos + 10 ).BeforeFirst( buf[( size_t )pos + 9] );
        pos = buf.Find( wxT( "version=" ) );
        if ( pos != wxNOT_FOUND )
            m_version = buf.Mid( pos + 9 ).BeforeFirst( buf[( size_t )pos + 8] );
    }
}

wxString a2dIOHandlerXMLPullExpat::GetContent()
{
    // it is possible that a content text is just overlapping
    // one buffer read to the other. Therefore is at the moment of wanting the content
    // the number of elements is 1 or less, start parsing the next part of the file.
    // In case of large content it will all be concatenated.

    while( m_elements.size() <= 1 && !m_done )
    {
        //read/parse some more

        size_t bytes_read;
        m_buffer = GetBuffer(); 
        if ( m_buffer == NULL )
        {
            a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "XML couldnot allocate buffer" ) );
            throw a2dIOHandlerXMLException( GetExpatErrorString() );
        }
        bytes_read = Read( m_buffer, XMLBUFSIZE );
        if ( bytes_read < 0 )
        {
            /* handle error */
        }

        m_done = ( bytes_read < XMLBUFSIZE );

        if ( !ParseBuffer( bytes_read, m_done ) )
        {
            a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "XML parsing error: '%s' at line %d" ), GetExpatErrorString().c_str(), GetExpatCurrentLineNumber() );
            throw a2dIOHandlerXMLException( GetExpatErrorString(), GetExpatCurrentLineNumber(), GetExpatCurrentColumnNumber() );
        }
    }

    return m_current->m_text;
}

void a2dIOHandlerXMLPullExpat::FillQueue()
{
    //queue might be empty, need to read more.

    while( m_elements.empty() && !m_done )
    {
        //read/parse some more

        size_t bytes_read;
        m_buffer = ( char* ) GetBuffer();;
        if ( m_buffer == NULL )
        {
            a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "XML couldnot allocate buffer" ) );
            throw a2dIOHandlerXMLException( GetExpatErrorString() );
        }
        /*
        #if wxUSE_UNICODE
                wxChar* buf = new wxChar[XMLBUFSIZE];

                bytes_read = Read( buf, XMLBUFSIZE);
                m_buffer = (char*) wxString(buf).mb_str().data();
                delete buf;
        #else
        */
        bytes_read = Read( m_buffer, XMLBUFSIZE );
//#endif // wxUSE_UNICODE
        if ( bytes_read < 0 )
        {
            /* handle error */
        }

        m_done = ( bytes_read < XMLBUFSIZE );

        if ( !ParseBuffer( bytes_read, m_done ) )
        {
            a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "XML parsing error: '%s' at line %d" ), GetExpatErrorString().c_str(), GetExpatCurrentLineNumber() );
            throw a2dIOHandlerXMLException( GetExpatErrorString(), GetExpatCurrentLineNumber(), GetExpatCurrentColumnNumber() );
        }
    }
}

//----------------------------------------------------------------------------
// a2dIOHandlerXMLWrite
//----------------------------------------------------------------------------

a2dIOHandlerXMLWrite::a2dIOHandlerXMLWrite()
{
    m_indentSize = 4;
    m_scale = 1;
    m_indent = 0;
    m_depth = 0;
    m_current = 0 ;
    m_startdoc = new a2dXMLTag( START_DOCUMENT, wxT( "startdocument" ) );
    m_enddoc = new a2dXMLTag( END_DOCUMENT, wxT( "enddocument" ) );
}

a2dIOHandlerXMLWrite::~a2dIOHandlerXMLWrite()
{
    delete m_startdoc;
    delete m_enddoc;
}

void a2dIOHandlerXMLWrite::InitializeSave( )
{
    m_indent = 0;
    m_depth = 0;
    m_current = m_startdoc;
    a2dIOHandlerStrOut::InitializeSave( );
}

void a2dIOHandlerXMLWrite::ResetSave( )
{
    a2dIOHandlerStrOut::ResetSave( );

    m_current = 0;
}

int a2dIOHandlerXMLWrite::GetCurrentColumnNumber()
{
    return m_current->m_column;
}

int a2dIOHandlerXMLWrite::GetCurrentLineNumber()
{
    return m_current->m_line;
}

//output


void a2dIOHandlerXMLWrite::WriteStartDocument( const wxString& version,  const wxString& encoding, bool standalone )
{
    m_depth = 0;
    *this << wxT( "<?xml version=\"" ) << version << wxT( "\" encoding=\"" ) << encoding << wxT( "\"" );
    *this << wxT( " standalone=\"" ) << ( standalone ? wxT( "yes" ) : wxT( "no" ) ) << wxT( "\"?>\n" );
}

void a2dIOHandlerXMLWrite::WriteEndDocument()
{
    wxASSERT_MSG( m_elements.empty(), wxT( "not end of document, still elements open" ) );
    wxASSERT_MSG( m_current == m_enddoc, wxT( "unBalanced" ) );
}



void a2dIOHandlerXMLWrite::WriteStartElement( const wxString& name, bool newLine )
{
    //the current that is the parent of this tag to write, has content
    m_current->m_hasContent = true;

    if ( newLine )
        WriteNewLine();

    m_current = new a2dXMLTag( START_TAG, name );
    m_elements.push_front( m_current );


    m_depth++;
    m_indent = m_depth;

    *this << wxT( "<" ) << name << wxT( ">" );
}


void a2dIOHandlerXMLWrite::WriteStartElementAttributes( const wxString& name, bool newLine )
{
    //the current that is the parent of this tag to write, has content
    m_current->m_hasContent = true;

    if ( newLine )
        WriteNewLine();

    m_current = new a2dXMLTag( START_TAG, name );
    m_current->m_hasattributes = true;
    m_elements.push_front( m_current );


    m_depth++;
    m_indent = m_depth;
    *this << wxT( "<" ) << name;
}


void a2dIOHandlerXMLWrite::WriteStartElement( const a2dXMLTag& tag, bool newLine )
{
    //the current that is the parent of this tag to write, has content
    m_current->m_hasContent = true;

    if ( newLine )
        WriteNewLine();

    m_current = new a2dXMLTag( tag );
    m_elements.push_front( m_current );

    m_current->m_hasattributes  = m_current->m_attributes.size() != 0;

    m_depth++;
    m_indent = m_depth;
    *this << wxT( "<" ) << m_current->m_tag << wxT( ">" );
}

void a2dIOHandlerXMLWrite::WriteElement( const wxString& name, const wxString& content, bool newLine )
{
    if ( newLine )
        WriteNewLine();

    *this << wxT( "<" ) << name << wxT( ">" );
    WriteStringEnt( content );
    *this << wxT( "</" ) << name << wxT( ">" );

    m_indent = m_depth;
}

void a2dIOHandlerXMLWrite::WriteAttribute( const wxString& name, bool value, bool onlyIfTrue )
{
    wxString error = wxT( "Wrong start tag" ) + m_current->m_tag;
    wxASSERT_MSG(  m_current->m_hasattributes , error );
    if ( ( onlyIfTrue && value ) || !onlyIfTrue )
    {
        if ( value == true )
            *this << wxT( " " ) << name << wxT( "=\"true\"" );
        else
            *this << wxT( " " ) << name << wxT( "=\"false\"" );
    }
}

void a2dIOHandlerXMLWrite::WriteAttribute( const a2dNamedProperty& attribute )
{
    wxString error = wxT( "Wrong start tag" ) + m_current->m_tag;
    wxASSERT_MSG(  m_current->m_hasattributes , error );

    *this << wxT( " " ) << attribute.GetName() << wxT( "=\"" ) << attribute.StringValueRepresentation();
}

void a2dIOHandlerXMLWrite::WriteAttributeEnt( const wxString& name, const wxString& value )
{
    wxString error = wxT( "Wrong start tag" ) + this->m_current->m_tag;
    wxASSERT_MSG(  this->m_current->m_hasattributes , error );

    *this << wxT( " " ) << name << wxT( "=\"" );
    WriteStringEnt( value );
    *this << wxT( "\"" );
}

void a2dIOHandlerXMLWrite::WriteEndAttributes( bool close )
{
    wxString error = wxT( "Wrong start tag" ) + m_current->m_tag;
    wxASSERT_MSG(  m_current->m_hasattributes , error );
    m_current->m_hasattributes = false;

    if ( close )
    {
        m_depth--;

        *this << wxT( "/>" );

        m_elements.pop_front();
        if ( !m_elements.empty() )
            m_current = m_elements.front();
        else
            m_current = m_enddoc;
    }
    else
        *this << wxT( ">" );

    m_indent = m_depth;  //at begin of starttag
}


void a2dIOHandlerXMLWrite::WriteEndElement( bool newLine )
{
    wxString error = wxT( "Wrong start tag or attributes not written" ) + m_current->m_tag;
    wxASSERT_MSG(  !m_current->m_hasattributes , wxT( "Wrong start tag or attributes not written" ) );

    m_depth--;
    m_indent = m_depth;

    if ( newLine )
        WriteNewLine();

    m_current = m_elements.front();

    *this << wxT( "</" ) << m_current->m_tag << wxT( ">" );

    m_elements.pop_front();
    if ( !m_elements.empty() )
        m_current = m_elements.front();
    else
        m_current = m_enddoc;
}

void a2dIOHandlerXMLWrite::WriteContent( const wxString& content )
{
    m_indent = m_depth;
    *this << content;
}

void a2dIOHandlerXMLWrite::WriteNewLine()
{
    WriteIndent();
}


void a2dIOHandlerXMLWrite::WriteIndent()
{
    wxString str = wxT( "\n" );
    if ( m_indentSize )
    {
		str += wxString ( ' ', m_indent * m_indentSize );
    }
    if ( str.IsEmpty() ) return;

    WriteString( str );
}

void a2dIOHandlerXMLWrite::EndlWriteString( const wxString& string )
{
    WriteNewLine();
    WriteString( string );
}

void a2dIOHandlerXMLWrite::WriteStringEnt( const wxString& string )
{
    wxString buf;
    size_t i, last, len;
    wxChar c;

    len = string.Len();
    last = 0;
    for ( i = 0; i < len; i++ )
    {
        c = string.GetChar( i );
        if ( c == wxT( '<' ) || c == wxT( '>' ) ||
                ( c == wxT( '&' ) && string.Mid( i + 1, 4 ) != wxT( "amp;" ) ) ||
                ( c == wxT( '"' ) ) )
        {
            a2dIOHandlerStrOut::WriteString( string.Mid( last, i - last ) );
            switch ( c )
            {
                case wxT( '<' ):
                    a2dIOHandlerStrOut::WriteString( wxT( "&lt;" ) );
                    break;
                case wxT( '>' ):
                    a2dIOHandlerStrOut::WriteString( wxT( "&gt;" ) );
                    break;
                case wxT( '&' ):
                    a2dIOHandlerStrOut::WriteString( wxT( "&amp;" ) );
                    break;
                case wxT( '"' ):
                    a2dIOHandlerStrOut::WriteString( wxT( "&quot;" ) );
                    break;
                default:
                    break;
            }
            last = i + 1;
        }
    }
    a2dIOHandlerStrOut::WriteString( string.Mid( last, i - last ) );
}

void a2dIOHandlerXMLWrite::XmlEncodeStringToStream(
    const wxChar* string,
    size_t len
)
{
#if wxUSE_UNICODE

    for( const wxChar* pos = string; *pos; pos++ )
    {
        if( *pos >= 32 && *pos <= 126 && *pos != wxChar( '<' ) && *pos != wxChar( '>' )
                && *pos != wxChar( '&' ) && *pos != wxChar( '\"' ) )
            *this << ( char ) *pos;
        else
            *this << wxT( "&#" ) << ( unsigned int )( wxUChar )( *pos ) << wxT( ";" );
    }

#else
    size_t nLen = ( len != wxStringBase::npos )
                  ? len
                  : wxConvLocal.MB2WC( ( wchar_t* ) NULL, string, 0 );

    if( nLen == ( size_t ) - 1 )
    {
        // Some error occured, write the string as is
        unsigned i = 0;

        for( const wxChar* pos = string; *pos && i < nLen; pos++, i++ )
        {
            if( *pos >= 32 && *pos <= 126 && *pos != '<' && *pos != '>' && *pos != '&' && *pos != '\"' )
                *this << ( char ) *pos;
            else
                *this << "&#" << ( unsigned int )( wchar_t )( *pos ) << ";";
        }
    }

    wchar_t* buf = new wchar_t[nLen + 1];
    wxConvLocal.MB2WC( buf, string, nLen );
    buf[nLen] = 0;

    for( const wchar_t* pos = buf; *pos; pos++ )
    {
        if( *pos >= 32 && *pos <= 126 && *pos != '<' && *pos != '>' && *pos != '&' && *pos != '\"' )
            *this << ( char ) *pos;
        else
            *this << wxT( "&#" ) << ( unsigned int )( wchar_t )( *pos ) << wxT( ";" );
    }

    delete[] buf;
#endif

}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const wxChar* string )
{
    WriteString( wxString( string ) );
    return *this;
}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const wxString& string )
{
    WriteString( string );
    return *this;
}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( char c )
{
    WriteString( wxString::FromAscii( c ) );

    return *this;
}

#if wxUSE_UNICODE && wxWCHAR_T_IS_REAL_TYPE

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( wchar_t wc )
{
    WriteString( wxString( &wc, m_conv, 1 ) );

    return *this;
}

#endif // wxUSE_UNICODE

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const wxInt16& c )
{
    wxString str;
    str.Printf( wxT( "%d" ), ( signed int )c );
    WriteString( str );

    return *this;
}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const wxInt32& c )
{
    wxString str;
    str.Printf( wxT( "%ld" ), ( signed long )c );
    WriteString( str );

    return *this;
}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const wxInt64& c )
{
    wxString str;
    str.Printf( wxT( "%lld" ), ( unsigned long long ) c );
    WriteString( str );

    return *this;
}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const wxLongLong& c )
{
    WriteString(  c.ToString  ()   );
    return *this;
}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const wxUint16& c )
{
    wxString str;
    str.Printf( wxT( "%u" ), ( unsigned int )c );
    WriteString( str );

    return *this;
}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const wxUint32& c )
{
    wxString str;
    str.Printf( wxT( "%lu" ), ( unsigned long )c );
    WriteString( str );

    return *this;
}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const wxUint64& c )
{
    wxString str;
    str.Printf( wxT( "%llu" ), ( unsigned long long )c );
    WriteString( str );

    return *this;
}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const wxULongLong& c )
{
    WriteString(  c.ToString  ()   );
    return *this;
}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const double& f )
{
    WriteDouble( f );
    return *this;
}

a2dIOHandlerXMLWrite& a2dIOHandlerXMLWrite::operator<<( const float& f )
{
    WriteDouble( ( double )f );
    return *this;
}

#if wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// a2dIOHandlerXmlSerIn
//----------------------------------------------------------------------------
#if wxDOCVIEW_USE_IRRXML
    IMPLEMENT_DYNAMIC_CLASS( a2dIOHandlerXmlSerIn, a2dIOHandlerXMLPull2 )
#else
    IMPLEMENT_DYNAMIC_CLASS( a2dIOHandlerXmlSerIn, a2dIOHandlerXMLPullExpat )
#endif //wxDOCVIEW_USE_IRRXML

a2dIOHandlerXmlSerIn::a2dIOHandlerXmlSerIn()
{
    m_doc = NULL;
    m_formatVersion = wxEmptyString;
}


a2dIOHandlerXmlSerIn::~a2dIOHandlerXmlSerIn()
{
}

bool a2dIOHandlerXmlSerIn::CanLoad( a2dDocumentInputStream& stream, const wxObject* obj, wxClassInfo* docClassInfo )
{
    SeekI( 0 );
    char cheader[7];
    cheader[6] = 0;
    Read( cheader, 6 );
    SeekI( 0 );

    // A check for a XML header should be enough,
    // encoding etc. is set by a2dIOHandlerXML::BaseDefaultHnd
    return ( memcmp( cheader, "<?xml ", 6 ) == 0 );
}

bool a2dIOHandlerXmlSerIn::Load( a2dDocumentInputStream& stream , wxObject* doc )
{
    m_streami = &stream;
    m_doc = doc;

    InitializeLoad();

    bool oke = true;

    try
    {
        ( ( a2dObject* )m_doc )->Load( NULL, *this );

        //now resolve references on id.
        //CVG links on Id's
        LinkReferences();

    }
    catch ( const a2dIOHandlerXMLException& e )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "CVG parsing error: %s at line %d column %d" ),
                                         e.getMessage().c_str(), e.getLineNumber(), e.getColumnNumber() );
        oke = false;
    }

    ResetLoad();

    return oke;
}

a2dObject* a2dIOHandlerXmlSerIn::LoadOneObject( wxObject* parent )
{
    if ( GetTagName() == wxT( "o" ) )
    {
        Require( START_TAG, wxT( "o" ) );

        wxString classname = GetAttributeValue( wxT( "classname" ) );
        a2dObject* o = wxStaticCast( CreateObject( classname ), a2dObject );
        if ( !o )
        {
            a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "could not create a2dObject %s, will be skipped line %d" ),
                                             classname.c_str(), GetCurrentLineNumber() );
            SkipSubTree();
            Require( END_TAG, wxT( "o" ) );
            Next();
        }
        else
            o->Load( parent, *this );
		return o;
    }
	return NULL;
}

//----------------------------------------------------------------------------
// a2dIOHandlerXmlSerOut
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dIOHandlerXmlSerOut, a2dIOHandlerXMLWrite )

a2dIOHandlerXmlSerOut::a2dIOHandlerXmlSerOut()
{
}


a2dIOHandlerXmlSerOut::~a2dIOHandlerXmlSerOut()
{
}

bool a2dIOHandlerXmlSerOut::Save( a2dDocumentOutputStream& stream, const wxObject* doc )
{
    m_doc = ( wxObject* ) doc;
    m_streamo = &stream;

    if ( !IsOk() )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open stream for saving" ) );
        return false;
    }

    InitializeSave();

    ( ( a2dObject* )m_doc )->Save( NULL, *this, NULL );

    ResetSave();

    return true;
}
#endif //wxART2D_USE_CVGIO


//----------------------------------------------------------------------------
// a2dXmlString
//----------------------------------------------------------------------------

a2dXmlString::a2dXmlString( const wxString& str )
{
    m_streamo = &m_strstream;
    InitializeSave();
    WriteString( str );
}


a2dXmlString::~a2dXmlString()
{
    ResetSave();
}

void a2dXmlString::Reset()
{
    //m_streamo->SeekO(0);
}

void a2dXmlString::WriteCommand( const wxString& name, bool close )
{
    WriteStartElementAttributes( _T( "command" ), false );
    WriteAttribute( _T( "name" ), name );
    if ( close )
        WriteEndAttributes( true );
}

#if wxDOCVIEW_USE_IRRXML

//----------------------------------------------------------------------------
// a2dIOHandlerXMLPull2
//----------------------------------------------------------------------------
#include "CXMLReaderImpl.h"

//! Implementation of the file read callback for ordinary files
class a2dFileReadCallBack : public IFileReadCallBack
{
public:

	//! construct from filename
	a2dFileReadCallBack(const char* filename)
		: Size(0), Close(true)
	{
        OwnStream = true;       

		// open file
        File = new a2dDocumentFileInputStream( filename, std::ios_base::in | std::ios_base::binary );
        if ( File->fail() || File->bad() )
        {
        }
    	getFileSize();
        File->seekg( 0, std::ios::beg );
	}

	//! construct from FILE pointer
	a2dFileReadCallBack( a2dDocumentInputStream& file)
		: Size(0), Close(false)
	{
        File = &file;
        if ( File->fail() || File->bad() )
        {
        }
    	getFileSize();
        File->seekg( 0, std::ios::beg );
        OwnStream = false;       
	}

	//! destructor
	virtual ~a2dFileReadCallBack()
	{
        if ( OwnStream )
            delete File;
	}

	//! Reads an amount of bytes from the file.
	virtual int read( void* buffer, int sizeToRead)
	{
		if (!File)
			return 0;

        File->read( (char*) buffer, sizeToRead );
        size_t len = File->gcount();

		return len;
	}

	//! Returns size of file in bytes
	virtual int getSize()
	{
		return Size;
	}

private:

	//! retrieves the file size of the open file
	void getFileSize()
	{
        File->seekg( 0, std::ios::end );
        Size = File->tellg(); // not interested in stream errors here
        File->seekg( 0, std::ios::cur );
	}

	a2dDocumentInputStream* File;
    bool OwnStream;
	int Size;
	bool Close;

}; // end class a2dFileReadCallBack

//! Creates an instance of an UFT-8 or ASCII character xml parser. 
IrrXMLReader* a2dCreateIrrXMLReader( a2dDocumentInputStream& file)
{
	return new CXMLReaderImpl<char, IXMLBase>(new a2dFileReadCallBack(file)); 
}

a2dIOHandlerXMLPull2::a2dIOHandlerXMLPull2()
{
    m_parser = NULL;
}

a2dIOHandlerXMLPull2::~a2dIOHandlerXMLPull2()
{
    if ( m_parser )
    {
        delete m_parser;
        m_parser = NULL;
    }
}

void a2dIOHandlerXMLPull2::InitializeLoad( )
{
    a2dIOHandlerXMLPull::InitializeLoad( );

    m_parser =  a2dCreateIrrXMLReader( *m_streami );
}

void a2dIOHandlerXMLPull2::ResetLoad( )
{
    m_elements.clear();
    m_current = 0;

    if ( m_parser )
    {
        delete m_parser;
        m_parser = NULL;
    }

    a2dIOHandlerStrIn::ResetLoad( );
}

wxString a2dIOHandlerXMLPull2::GetContent()
{
    while( m_elements.size() <= 1 )
    {
        FillQueue();
    }

    return m_current->m_text;
}

void a2dIOHandlerXMLPull2::FillQueue()
{
    bool next = true;
    while ( next && m_parser->read() )
    {        
		switch( m_parser->getNodeType())
		{
		    case EXN_ELEMENT:
            {
                a2dXMLTag* tag = new a2dXMLTag( START_TAG, XmlDecodeStringToString( m_parser->getNodeName() ) );
                for ( int i = 0 ; i < m_parser->getAttributeCount(); i++ )
                    tag->SetAttribute( m_parser->getAttributeName( i ), m_parser->getAttributeValue( i ) );

                m_elements.push_back( tag );
                if ( m_parser->isEmptyElement() )
                {
                    a2dXMLTag* tag = new a2dXMLTag( END_TAG, XmlDecodeStringToString( m_parser->getNodeName() ) );
                    m_elements.push_back( tag );
                }

                tag->m_line = m_parser->GetLine();
                tag->m_column = m_parser->GetColumn();
                next = true;
			    break;
            }
            case EXN_ELEMENT_END:
            {
                a2dXMLTag* tag = new a2dXMLTag( END_TAG, XmlDecodeStringToString( m_parser->getNodeName() ) );
                m_elements.push_back( tag );
                tag->m_line = m_parser->GetLine();
                tag->m_column = m_parser->GetColumn();
                next = true;
			    break;
            }
            case EXN_TEXT:
            {
                if ( !m_elements.empty() )
                {
                    m_elements.back()->m_text += XmlDecodeStringToString( m_parser->getNodeName() );
                }
                next = true;//false;
			    break;
            }
            default:
            {
                next = true;
            }
        }
    }   
}
#endif //wxDOCVIEW_USE_IRRXML

#endif /* wxART2D_USE_XMLPARSE */
