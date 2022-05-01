/*! \file keyio/src/keyio.cpp
    \brief KEY parser
    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: keyio.cpp,v 1.64 2009/05/15 16:34:28 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef new
#undef new
#endif

#include "a2dprivate.h"

#if wxART2D_USE_KEYIO

#include "wx/docview/doccom.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"
#include "wx/canvas/canobj.h"
#include "wx/canvas/vpath.h"
#include "wx/editor/candoc.h"
#include "wx/canvas/layerinf.h"
#include "wx/canvas/recur.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/cameleon.h"
#include "wx/canvas/polygon.h"
#include "wx/keyio/keyio.h"
#include "wx/gdsio/gdserr.h"

#include <wx/datetime.h>
#include <limits.h>

/*
Purpose Used by GDS driver for conversions and read en write
        GDSII elements
*/

// ----------------------------------------------------------------------------
// a2dIOHandlerKeyIn
// ----------------------------------------------------------------------------

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
    m_b.Empty();

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
    m_b.Empty();

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
    m_keyword.Empty();
    m_value.Empty();
    m_error_mes.Empty();

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

        m_keyword = _T( "comment" );
        return true;
    }
    else
    {
        m_b.Empty();

        if ( !NormalWord() )
            return false;

        m_keyword = m_b;

        Word();

        if ( m_error_mes.Len() )
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
bool a2dIOHandlerKeyIn::ReadItem( const wxString& type )
{
//  <item>      ::= KEYWORD VALUE
    if ( m_back )
    {
        if ( ! m_keyword.CmpNoCase( type ) )
            m_back = false;
        else
            m_back = true;
        return bool( !m_back );
    }

    wxString buf;
    bool Next = true;
    do
    {
        if( !GetCommand() )
        {
            if ( !m_error_mes.IsEmpty() )
            {
                m_b.Printf( _T( "Could not Parse line %d: \n Error: %s" ),
                            m_linenumber, m_error_mes.c_str() );

                throw GDS_Error( m_b, wxT( "Command Parsing Error" ) );

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

    if ( m_value.IsEmpty() )
    {
        wxString buf;
        buf = wxT( "Argument missing (in " );
        if ( !m_keyword.CmpNoCase( type ) )
        {
            // These commands don't have arguments...
            if ( !m_keyword.CmpNoCase( wxT( "BGNLIB" ) ) );
            else if (!m_keyword.CmpNoCase(wxT("UNITS")));
            else if ( !m_keyword.CmpNoCase( wxT( "BGNSTR" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "BOUNDARY" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "SURFACE" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "LINE" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "CIRCLE" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "BOX" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "PATH" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "TEXT" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "AREF" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "SREF" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "ARC" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "ENDEL" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "ENDLIB" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "ENDMASKS" ) ) );
            //next may or may not be empty
            else if ( !m_keyword.CmpNoCase( wxT( "PROPVALUE" ) ) );
            else if ( !m_keyword.CmpNoCase( wxT( "PARAMETERVALUE" ) ) );
            else
            {
                buf += m_keyword;
                buf += wxT( ")" );
                throw GDS_Error( buf ); // There is no argument, so throw an error...
            }
        }
    }

    if ( !m_keyword.CmpNoCase( type ) )
        m_back = false;
    else
        m_back = true;
    return bool( !m_back );
}

bool a2dIOHandlerKeyIn::SkipXYData()
{
    if ( !ReadItem( wxT( "xy" ) ) )
        return false;

    while ( 1 )
    {
        if ( ( ReadItem( wxT( "x" ) ) ) ||
                ( ReadItem( wxT( "y" ) ) ) ||
                ( ReadItem( wxT( "xm" ) ) ) ||
                ( ReadItem( wxT( "ym" ) ) ) ||
                ( ReadItem( wxT( "xo" ) ) ) ||
                ( ReadItem( wxT( "yo" ) ) )
           )
            continue;
        else
            break;
    }

    return true;
}

double a2dIOHandlerKeyIn::ReadDouble()
{
    return m_scale_in * wxAtof( m_value );
}

wxChar a2dIOHandlerKeyIn::PeekNextC()
{
    wxChar p = ( wxChar ) Peek();
    return p;
}

void a2dIOHandlerKeyIn::IncC()
{
    a = ( wxChar ) GetC();

    if ( a == wxT( '\0' ) || a == wxT( '\n' )  )
        m_linenumber++;


    if ( Eof() )
        a = wxT( '\0' );
    if ( a == wxChar( EOF ) )
        a = wxT( '\0' );
    if ( a == wxT( '\r' ) )
        a = wxT( ' ' );
}

// -------------- use it -----------------

a2dIOHandlerKeyIn::a2dIOHandlerKeyIn()
{
    m_scale_in = 1;
    m_back = false;
    m_asCameleons = false;
    m_asMultiroot = false;
    m_keyword.Clear();
    m_value.Clear();
    m_lastElementLayer = 0;

    m_docClassInfo = &a2dCanvasDocument::ms_classInfo;
    m_refMaxx = 10; 
    m_dx = m_dy = 0;
    m_hasLayers = false;
    m_gridx = 0; 
    m_gridy = 0;
}

a2dIOHandlerKeyIn::~a2dIOHandlerKeyIn()
{
}

void a2dIOHandlerKeyIn::InitializeLoad(  )
{
    m_gridx = 0; 
    m_gridy = 0;
    m_linenumber = 0;
    a2dIOHandlerStrIn::InitializeLoad();
    IncC(); //get the first character;
    if ( m_doc )
        a2dCanvasGlobals->GetHabitat()->SetAberPolyToArc( double( a2dCanvasGlobals->GetHabitat()->GetAberPolyToArc() ) / m_doc->GetUnitsScale() );
}

void a2dIOHandlerKeyIn::ResetLoad(  )
{
    a2dIOHandlerStrIn::ResetLoad();
}

bool a2dIOHandlerKeyIn::CanLoad( a2dDocumentInputStream& stream, const wxObject* obj, wxClassInfo* docClassInfo )
{
    if ( obj && !wxDynamicCast( obj, a2dCanvasDocument ) )
        return false;

    if ( docClassInfo && m_docClassInfo && !docClassInfo->IsKindOf( m_docClassInfo ) )
        return false;

    m_streami = &stream;
    m_doc = 0;
    m_hasLayers = false;

    SeekI( 0 );
    InitializeLoad();

    m_recordsize      = 0;
    m_recordtype      = 0;
    m_back            = false;

    try
    {
        if ( !ReadItem( wxT( "header" ) ) ) // Header expected here
        {
            SeekI( 0 );
            return false;
        }

        //skip rest for the moment
        while( !ReadItem( wxT( "units" ) ) )
        {
            if ( ReadItem( wxT( "mask" ) ) )
            {
                m_hasLayers = true;
                break;
            }
            else
                m_back = false;
        }
    }
    catch ( GDS_Error& WXUNUSED( error ) )
    {
        ResetLoad();
        SeekI( 0 );
        return false;
    }
    SeekI( 0 );

    return true;
}

bool a2dIOHandlerKeyIn::LinkReferences()
{
    bool res = a2dIOHandler::LinkReferences();

    //search in the root object for the childs that have the bin flag set,
    //which means they are referenced, and can be deleted.
    //All others are top structures.
    a2dCanvasObject* root = m_doc->GetDrawing()->GetRootObject();
    a2dCanvasObjectList::iterator rootchild = root->GetChildObjectList()->begin();
    while ( rootchild != root->GetChildObjectList()->end() )
    {
        a2dCanvasObjectList::iterator delnode = rootchild;
        rootchild++;
        if ( ( *delnode )->GetCheck() )
        {
            root->GetChildObjectList()->erase( delnode );
        }
    }
    return res;
}

bool a2dIOHandlerKeyIn::Load( a2dDocumentInputStream& stream, wxObject* doc )
{
    m_streami = &stream;

    m_doc = wxStaticCast( doc, a2dCanvasDocument );
    InitializeLoad();

    wxString path;
    path = m_doc->GetFilename().GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR  ) + m_doc->GetFilename().GetName() + wxT( ".cvg" );
    if ( wxFileExists( path ) )
        m_doc->LoadLayers( path );

    m_layers = m_doc->GetLayerSetup();

    if ( !m_hasLayers ) 
    {
        for ( a2dLayerIndex::iterator it= m_layers->GetLayerSort().begin(); it != m_layers->GetLayerSort().end(); ++it)
        {
            a2dLayerInfo* layerobj = *it;
            wxUint16 i = layerobj->GetLayer();
            m_mapping[ layerobj->GetInMapping() ] = layerobj;
        }
    }
    else
    {
        m_layers->ReleaseChildObjects();
        m_layers->SetPending( true );
        m_layers->UpdateIndexes();
        m_mapping.clear();
    }
    m_doc->GetDrawing()->SetLayerSetup( m_layers );

    m_recordsize      = 0;
    m_recordtype      = 0;
    m_back            = false;

    try
    {
        ReadKey();
    }
    catch ( GDS_Error& error )
    {
        ResetLoad();
        a2dDocviewGlobals->ReportErrorF( a2dError_NotSpecified, wxT( "error in KEY file \n %s \n %d \n" ), error.GetErrorMessage().c_str(), m_linenumber );
        return false;
    }

    ResetLoad();
    return true;
}

// GdsII --------------------------------------------------------------------
void a2dIOHandlerKeyIn::ReadKey()
{
    //The a2dObject::m_check is used for removing structure from the rootobject after resolving references.
    //when a new document is filled this would not be needed, sine all m_check flags are default set false.
    //But maybe for the future import it is important to do.
    a2dWalker_SetCheck setp( false );
    setp.Start( m_doc->GetDrawing() );

    a2dCameleon::SetCameleonRoot( m_doc->GetDrawing()->GetRootObject() );

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

    m_doc->SetVersion( m_value );
    if ( !ReadBgnlib() )
        throw GDS_Error( wxT( "BGNLIB is missing (in KEY-file)" ) );
    if ( !ReadLibrarySettings() )
        throw GDS_Error( wxT( "LIBNAME is missing (in KEY-file)" ) );
    if ( m_asCameleons )
    {
        while ( ReadCameleonStructure( m_doc->GetDrawing()->GetRootObject() ) )
        {
        }
    }
    else
    {
        while ( ReadStructure( m_doc->GetDrawing()->GetRootObject() ) )
        {
        }
    }
    if ( !ReadItem( wxT( "endlib" ) ) )
        throw GDS_Error( wxT( "ENDLIB is missing (in KEY-file)" ) );

    //references to other structure are known
    //by name now. Set the pointers in those references
    //to the right structures
    LinkReferences(); //GDS links on Name

    if ( m_asCameleons )
    {
        m_dx = m_dy = 0;

        m_doc->GetDrawing()->SetDrawingId( a2dDrawingId_cameleonrefs() );
        if ( m_doc->GetDrawing()->GetRootObject()->GetChildObjectsCount() == 0 )
            throw GDS_Error( wxT( "no structures found" ) );

        m_doc->GetDrawing()->SetRootRecursive();

        a2dCameleon* topcam = NULL;
        a2dCanvasObjectList::iterator rootchild = m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->begin();
        while ( rootchild != m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->end() )
        {
            a2dCameleon* ref = wxDynamicCast( (*rootchild).Get(), a2dCameleon );
            if ( ref )
            {
                a2dLayout* layout = ref->GetAppearance<a2dDiagram>();
                layout->GetDrawing()->SetRootRecursive();
                if ( ! layout->GetCheck() )
                {
                    topcam = ref;
                    m_doc->SetTopCameleon( topcam );
                    m_doc->SetStartObject( layout->GetDrawing()->GetRootObject() );
                }
            }
            rootchild++;
        }
    }
    else
    {
        if ( m_asMultiroot )
        {

            //search in the root object for the childs that have the bin flag set,
            //which means they are referenced, and can be deleted.
            //All othere are top structures.
            a2dCanvasObject* root = m_doc->GetDrawing()->GetRootObject();
            a2dCanvasObject* top = NULL;
            a2dCanvasObjectList::iterator rootchild = root->GetChildObjectList()->begin();
            while ( rootchild != root->GetChildObjectList()->end() )
            {
                a2dCanvasObjectList::iterator node = rootchild;
                rootchild++;
                a2dNameReference* ref = wxDynamicCast( node->Get(), a2dNameReference );
                if ( ref && !ref->GetCanvasObject()->GetCheck() )
                {
                    top = *node;
                    break;
                }
            }

            m_doc->SetMultiRoot();
            if ( top )
            {
                a2dNameReference* ref = wxDynamicCast( top, a2dNameReference );
                m_doc->SetStartObject( ref->GetCanvasObject() );
            }
            else
                m_doc->SetStartObject( m_doc->GetDrawing()->GetRootObject() );
        }
        else
        {
            if ( m_doc->GetDrawing()->GetRootObject()->GetChildObjectsCount() != 1 )
            {
                m_doc->SetMultiRoot();
                m_doc->SetStartObject( m_doc->GetDrawing()->GetRootObject() );
            }
            else
            {
                m_doc->SetMultiRoot( false );
                m_doc->GetDrawing()->SetRootRecursive();
                m_doc->SetStartObject( m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->front() );
            }
        }

        m_doc->GetDrawing()->SetUnits( m_doc->GetUnits() );
        m_doc->GetDrawing()->SetUnitsAccuracy( m_doc->GetUnitsAccuracy() );
        m_doc->GetDrawing()->SetUnitsScale( m_doc->GetUnitsScale() );
    }
}

// Bgnlib -------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadBgnlib()
{
    if( !ReadItem( wxT( "bgnlib" ) ) ) // BgnLib expected here
        return false;
    if( !ReadItem( wxT( "lastmod" ) ) )
        throw GDS_Error( wxT( "BGNLIB: LASTMOD is missing (in KEY-file)" ) );

    wxString _int_str_ptr = m_value;

    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    year = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    month = wxAtoi( _int_str_ptr ) - 1;
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    day = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "  " ) ) + 2;
    hour = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    minute = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    second = wxAtoi( _int_str_ptr );

    m_doc->GetModificationTime().Set( day, ( wxDateTime::Month )month, year, hour, minute, second );

    if( !ReadItem( wxT( "lastacc" ) ) )
        throw GDS_Error( wxT( "BGNLIB: LASTACC is missing (in KEY-file)" ) );

    _int_str_ptr = m_value;

    year = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    month = wxAtoi( _int_str_ptr ) - 1;
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    day = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "  " ) ) + 2;
    hour = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    minute = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    second = wxAtoi( _int_str_ptr );

    m_doc->GetAccessTime().Set( day, ( wxDateTime::Month )month, year, hour, minute, second );

    return( true );
}

// Library ------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadLibrarySettings()
{
    if( !ReadItem( wxT( "libname" ) ) ) // BgnLib expected here
        throw GDS_Error( wxT( "libname record missing (in KEY)" ), wxT( "Error" ) );

    m_doc->SetLibraryName( m_value );
    m_doc->SetTitle( m_value, false );

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
                wxString layerstr = m_value;
                wxInt16 layernr = 0;
                a2dLayerInfo* info = NULL;

                a2dFill layerfill;
                a2dStroke layerstroke;

                while( 1 )
                {
                    if ( ReadItem( wxT( "layernr" ) ) )
                    {
                        layernr = wxAtoi( m_value );
                        info = m_mapping[ layernr ];
                        if ( !info )
                        {
                            info = new a2dLayerInfo( layernr, layerstr );
                            m_layers->Append( info );
                            m_mapping[ layernr ] = info;
                        }
                        else
                            info->SetName( layerstr );
                        layerfill = info->GetFill();
                        layerstroke = info->GetStroke();
                    }
                    else if ( ReadItem( wxT( "color" ) ) )
                    {
                        wxColour fill;
                        wxString fillColour = m_value;
                        if ( !fillColour.IsEmpty() )
                        {
                            if ( fillColour.GetChar( 0 ) == wxT( '#' ) )
                                fill = HexToColour( fillColour.After( wxT( '#' ) ) );
                            else
                            {
                                fill = wxTheColourDatabase->Find( fillColour );
                                if ( !fill.Ok() )
                                    fill = *wxBLACK;
                            }
                            layerfill.SetColour( fill );
                            layerstroke.SetColour( fill );
                        }
                    }
                    else if ( ReadItem( wxT( "trans" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "binding" ) ) )
                    {
                    }
                    else if ( ReadItem( wxT( "linestyle" ) ) )
                    {
                        a2dStrokeStyle style = StrokeString2Style( m_value );
                        layerstroke.SetStyle( style );
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
                        wxUint16 shapestyle = wxAtoi( m_value );
                        switch( shapestyle )
                        {
                            case 0:
                                layerfill.SetFilling( false );
                                break;
                            case 1:
                                layerfill.SetFilling( true );
                                break;
                            case 2:
                                layerfill.SetFilling( true );
                                break;
                            default:
                                layerfill.SetFilling( true );;
                        }
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
                    else if ( m_keyword.CmpNoCase( wxT( "mask" ) ) == 0 || m_keyword.CmpNoCase( wxT( "endmasks" ) ) == 0 )
                        break;
                    else
                        m_back = false; //skip this
                }
                if ( info != wxNullLayerInfo )
                {
                    info->SetEvtHandlerEnabled(false);
                    info->SetFill( layerfill );
                    info->SetStroke( layerstroke );
                    info->SetEvtHandlerEnabled(true);
                }
            }
        }
    }
    if ( m_hasLayers )
    {
        m_layers->SetPending( true );
        m_layers->UpdateIndexes();
        for ( a2dLayerIndex::iterator it= m_layers->GetLayerSort().begin(); it != m_layers->GetLayerSort().end(); ++it)
        {
            a2dLayerInfo* layerobj = *it;
            wxUint16 i = layerobj->GetLayer();
            m_mapping[ layerobj->GetInMapping() ] = layerobj;
        }
        m_layers->AddDefaultLayers();
        m_layers->UpdateIndexes();
    }
    if( !ReadItem( wxT( "units" ) ) ) // Units expected here
        return false;
    m_doc->SetUnits( m_value );

    if( !ReadItem( wxT( "userunits" ) ) )
        throw GDS_Error( wxT( "USERUNITS missing (in KEY-file)" ) );
    m_userunits_out = ( double )wxAtof( m_value );
    m_doc->SetUnitsAccuracy( m_userunits_out );
    if( !ReadItem( wxT( "physunits" ) ) )
        throw GDS_Error( wxT( "PHYSUNITS missing (in KEY-file)" ) );

    double metersScale = ( ( double )wxAtof( m_value ) ) / m_userunits_out;
    m_doc->SetUnitsScale( metersScale );

    /*
    a2dDoMu unit = a2dDoMu( 1, 1 );
    m_doc->SetUnits( unit.GetMultiplierString() );
    unit = a2dDoMu( 1, metersScale );
    m_doc->SetUnits( unit.GetMultiplierString() );
    */

    return true;
}

// Read Element -------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadElement( a2dCanvasObject* parent )
{
//  <element>           ::= {<boundary> | <path> | <SREF> | <AREF> | <text> | <node> | <box>}
//                                  /* {<property>}* */
//                                  ENDEL

    if  ( ReadBoundary( parent ) ||
            ReadSurface( parent ) ||
            ReadPath( parent ) ||
            ReadStructureReference( parent ) ||
            ReadArrayReference( parent ) ||
            ReadText( parent ) ||
            ReadNode( parent ) ||
            ReadBox( parent ) ||
            ReadArc( parent ) ||
            ReadCircle( parent ) ||
            ReadLine( parent ) ||
            ReadImage( parent )
        )
    {
    }
    else
        return false;

    while ( ReadProperties( parent->GetChildObjectList()->back() ) );
    while ( ReadParameters( parent->GetChildObjectList()->back() ) );

    if ( !ReadItem( wxT( "endel" ) ) )
        throw GDS_Error( wxT( "ENDEL is missing (in KEY-file) (too many points in element?)" ) );
    return true;
}


// Property -----------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadProperties( a2dCanvasObject* parent )
{
    if ( !ReadItem( wxT( "property" ) ) )
        return false;

    do
    {
        wxString name;
        wxString value;
        wxString type;
        name = m_value;

        if ( ReadItem( wxT( "proptype" ) ) )
            type = m_value;
        if ( ReadItem( wxT( "propvalue" ) ) )
            value = m_value;

        if ( m_layers->GetRead( m_lastElementLayer ) )
        {
            if ( type == wxT( "string" ) )
            {
                a2dPropertyId* propid = parent->HasPropertyId( name );
                if ( !propid )
                {
                    propid = new a2dPropertyIdString( name, wxT( "" ), a2dPropertyId::flag_userDefined );
                    parent->AddPropertyId( propid );
                }
                a2dPropertyIdString* propidstr = dynamic_cast<a2dPropertyIdString*>( propid );
                if ( !propidstr )
                    wxLogWarning( wxT( "KEYIO : string property: %s exist already with different type" ), name.c_str() );
                else
                    propidstr->SetPropertyToObject( parent, value );
            }
            else if ( type == wxT( "integer" ) )
            {
                a2dPropertyId* propid = parent->HasPropertyId( name );
                if ( !propid )
                {
                    propid = new a2dPropertyIdInt32( name, 0, a2dPropertyId::flag_userDefined );
                    parent->AddPropertyId( propid );
                }
                a2dPropertyIdInt32* propidint = dynamic_cast<a2dPropertyIdInt32*>( propid );
                if ( !propidint )
                    wxLogWarning( wxT( "KEYIO : int property: %s exist already with different type" ), name.c_str() );
                else
                    propidint->SetPropertyToObject( parent, wxAtoi( value ) );
            }
            else if ( type == wxT( "real" ) )
            {
                a2dPropertyId* propid = parent->HasPropertyId( name );
                if ( !propid )
                {
                    propid = new a2dPropertyIdDouble( name, 0, a2dPropertyId::flag_userDefined );
                    parent->AddPropertyId( propid );
                }
                a2dPropertyIdDouble* propidreal = dynamic_cast<a2dPropertyIdDouble*>( propid );
                if ( !propidreal )
                    wxLogWarning( wxT( "KEYIO : real property: %s exist already with different type" ), name.c_str() );
                else
                {
                    double doubleval;
                    value.ToDouble( &doubleval );
                    propidreal->SetPropertyToObject( parent, doubleval );
                }
            }
            else if ( type == wxT( "bool" ) )
            {
                a2dPropertyId* propid = parent->HasPropertyId( name );
                if ( !propid )
                {
                    propid = new a2dPropertyIdBool( name, false, a2dPropertyId::flag_userDefined );
                    parent->AddPropertyId( propid );
                }
                a2dPropertyIdBool* propidbool = dynamic_cast<a2dPropertyIdBool*>( propid );
                if ( !propidbool )
                    wxLogWarning( wxT( "KEYIO : bool property: %s exist already with different type" ), name.c_str() );
                else
                {
                    propidbool->SetPropertyToObject( parent, value.IsSameAs( wxT( "true" ), true ) ? true : false );
                }
            }
            else
                wxLogWarning( wxT( "KEYIO : property type: %s is not implemented" ), type.c_str() );

        }
    }
    while ( ReadItem( wxT( "property" ) ) );
    return true;
}

// Property -----------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadParameters( a2dCanvasObject* parent )
{
    if ( !ReadItem( wxT( "parameter" ) ) )
        return false;

    do
    {
        wxString name;
        wxString value;
        wxString type;
        name = m_value;
        //else confusion with properties using same name (allowed in other tools like ADS).
        if ( ! wxDynamicCast( parent, a2dCameleon ) )
            name = "__PAR_" + m_value; 

        if ( ReadItem( wxT( "parametertype" ) ) )
            type = m_value;
        if ( ReadItem( wxT( "parametervalue" ) ) )
            value = m_value;

        if ( m_layers->GetRead( m_lastElementLayer ) )
        {
            if ( type == wxT( "string" ) )
            {
                a2dPropertyId* propid = parent->HasPropertyId( name );
                if ( !propid )
                {
                    propid = new a2dPropertyIdString( name, wxT( "" ), a2dPropertyId::flag_userDefined );
                    parent->AddPropertyId( propid );
                }
                a2dPropertyIdString* propidstr = dynamic_cast<a2dPropertyIdString*>( propid );
                if ( !propidstr )
                    wxLogWarning( wxT( "KEYIO : string property: %s exist already with different type" ), name.c_str() );
                else
                    propidstr->SetPropertyToObject( parent, value );
            }
            else if ( type == wxT( "integer" ) )
            {
                a2dPropertyId* propid = parent->HasPropertyId( name );
                if ( !propid )
                {
                    propid = new a2dPropertyIdInt32( name, 0, a2dPropertyId::flag_userDefined );
                    parent->AddPropertyId( propid );
                }
                a2dPropertyIdInt32* propidint = dynamic_cast<a2dPropertyIdInt32*>( propid );
                if ( !propidint )
                    wxLogWarning( wxT( "KEYIO : int property: %s exist already with different type" ), name.c_str() );
                else
                    propidint->SetPropertyToObject( parent, wxAtoi( value ) );
            }
            else if ( type == wxT( "real" ) )
            {
                a2dPropertyId* propid = parent->HasPropertyId( name );
                if ( !propid )
                {
                    propid = new a2dPropertyIdDouble( name, 0, a2dPropertyId::flag_userDefined );
                    parent->AddPropertyId( propid );
                }
                a2dPropertyIdDouble* propidreal = dynamic_cast<a2dPropertyIdDouble*>( propid );
                if ( !propidreal )
                    wxLogWarning( wxT( "KEYIO : real property: %s exist already with different type" ), name.c_str() );
                else
                {
                    double doubleval;
                    value.ToDouble( &doubleval );
                    propidreal->SetPropertyToObject( parent, doubleval );
                }
            }
            else if ( type == wxT( "bool" ) )
            {
                a2dPropertyId* propid = parent->HasPropertyId( name );
                if ( !propid )
                {
                    propid = new a2dPropertyIdBool( name, false, a2dPropertyId::flag_userDefined );
                    parent->AddPropertyId( propid );
                }
                a2dPropertyIdBool* propidbool = dynamic_cast<a2dPropertyIdBool*>( propid );
                if ( !propidbool )
                    wxLogWarning( wxT( "KEYIO : bool property: %s exist already with different type" ), name.c_str() );
                else
                {
                    propidbool->SetPropertyToObject( parent, value.IsSameAs( wxT( "true" ), true ) ? true : false );
                }
            }
            else
                wxLogWarning( wxT( "KEYIO : parameter type: %s is not implemented" ), type.c_str() );

        }
    }
    while ( ReadItem( wxT( "parameter" ) ) );
    return true;
}

// Surface -----------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadSurface( a2dCanvasObject* parent )
{
// <Surface>    ::= SURFACE [ELFLAGS] [PLEX] LAYER DATATYPE [WIDTH] XY

    if ( !ReadItem( wxT( "surface" ) ) )
        return false;

    // Create a new Boundary
    a2dSmrtPtr<a2dSurface> Surface = new a2dSurface();

    ReadElflags( Surface );

    ReadItem( wxT( "plex" ) );

    if ( !ReadLayer( Surface ) )
        throw GDS_Error( wxT( "Surface: LAYER missing (in KEY-file)" ) );

    ReadItem( wxT( "datatype" ) );
    SetDataTypeProperty( Surface, wxAtoi( m_value ) );

    if ( ReadItem( wxT( "spline" ) ) )
        Surface->SetSpline( wxAtoi( m_value ) > 0 );

    if ( ReadItem( wxT( "width" ) ) )
        Surface->SetContourWidth( ReadDouble() );

    if ( m_layers->GetRead( Surface->GetLayer() ) )
    {
        if ( !ReadPolygon( Surface->GetSegments() ) )
            throw GDS_Error( wxT( "Header: XY missing (in Boundary)" ) );

        while ( ReadItem( wxT( "hole" ) ) )
        {
            a2dVertexList* rlist = new a2dVertexList();
            if ( !ReadPolygon( rlist ) )
                throw GDS_Error( wxT( "Header: XY missing (in Surface Hole)" ) );
            Surface->AddHole( rlist );
        }
        parent->Append( Surface );
    }
    else
    {
        SkipXYData();
        while ( ReadItem( wxT( "hole" ) ) )
        {
            a2dVertexList* rlist = new a2dVertexList();
            SkipXYData();
        }
    }
    return true;
}

// Boundary -----------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadBoundary( a2dCanvasObject* parent )
{
// <boundary>   ::= BOUNDARY [ELFLAGS] [PLEX] LAYER DATATYPE [WIDTH] XY

    if ( !ReadItem( wxT( "boundary" ) ) )
        return false;

    // Create a new Boundary
    a2dSmrtPtr<a2dPolygonL> Boundary = new a2dPolygonL();

    ReadElflags( Boundary );

    ReadItem( wxT( "plex" ) );

    if ( !ReadLayer( Boundary ) )
        throw GDS_Error( wxT( "Boundary: LAYER missing (in KEY-file)" ) );

    ReadItem( wxT( "datatype" ) );
    SetDataTypeProperty( Boundary, wxAtoi( m_value ) );

    if ( ReadItem( wxT( "spline" ) ) )
        Boundary->SetSpline( wxAtoi( m_value ) > 0 );

    if ( ReadItem( wxT( "width" ) ) )
        Boundary->SetContourWidth( ReadDouble() );

    if ( m_layers->GetRead( Boundary->GetLayer() ) )
    {
        if ( !ReadPolygon( Boundary->GetSegments() ) )
            throw GDS_Error( wxT( "Header: XY missing (in Boundary)" ) );

        parent->Append( Boundary );
    }
    else
        SkipXYData();

    return true;
}

// Path ---------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadPath( a2dCanvasObject* parent )
{
//  <path>              ::= PATH [ELFLAGS] [PLEX] LAYER
//                                  DATATYPE [PATHTYPE] [WIDTH]
//                                  [BGNEXTN] [ENDEXTN] XY

    if ( !ReadItem( wxT( "path" ) ) )
        return false;

    // Create a new Path
    a2dSmrtPtr<a2dPolylineL> path = new a2dPolylineL();

    ReadElflags( path );
    ReadItem( wxT( "plex" ) );

    if ( !ReadLayer( path ) )
        throw GDS_Error( wxT( "record LAYER missing (in Path)" ) );

    if ( ReadItem( wxT( "datatype" ) ) )
        SetDataTypeProperty( path, wxAtoi( m_value ) );

    if ( ReadItem( wxT( "pathtype" ) ) )
        path->SetPathType( ( a2dPATH_END_TYPE )wxAtoi( m_value ) );

    if ( ReadItem( wxT( "spline" ) ) )
        path->SetSpline( wxAtoi( m_value ) > 0 );

    if ( ReadItem( wxT( "width" ) ) )
        path->SetContourWidth( ReadDouble() );

    ReadItem( wxT( "bgnextn" ) );
    ReadItem( wxT( "endextn" ) );

    if ( m_layers->GetRead( path->GetLayer() ) )
    {
        if ( !ReadPolyline( path->GetSegments() ) )
            throw GDS_Error( wxT( "Header: XY missing (in Path)" ) );
        parent->Append( path );
    }
    else
        SkipXYData();

    return true;
}


// Text ---------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadText( a2dCanvasObject* parent )
{
    // Grammar: TEXT [ELFLAGS] [PLEX] LAYER
    //              TEXTTYPE [PRESENTATION] [PATHTYPE] [WIDTH]
    //              [<strans>] XY STRING
    //
    //              <strans> = STRANS [MAG] [ANGLE]

    if ( !ReadItem( wxT( "text" ) ) )
        return false;

    // Create a new Text
    a2dSmrtPtr<a2dTextGDS> Text = new a2dTextGDS( a2dFONT_STROKED );

    ReadElflags( Text );
    ReadItem( wxT( "plex" ) );
    if ( !ReadLayer( Text ) )
        throw GDS_Error( wxT( "Text: LAYER is missing (in GDS-file)" ) );

    if ( !ReadItem( wxT( "texttype" ) ) )
        throw GDS_Error( wxT( "Text: TEXTBODY is missing (in GDS-file)" ) );
    Text->SetTextType( wxAtoi( m_value ) );

    if ( ReadItem( wxT( "presentation" ) ) )
    {
        //now that we know the size, in order to set font stroke weight based on size.
        wxString _int_str_ptr = m_value;
        Text->SetHorizontal( wxAtoi( _int_str_ptr ) % 4 );
        _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "," ) ) + 1;
        Text->SetVertical( wxAtoi( _int_str_ptr ) % 4 );
        _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "," ) ) + 1;
        Text->SetFontGDS( wxAtoi( _int_str_ptr ) % 4 );
    }

    if ( ReadItem( wxT( "pathtype" ) ) )
    {
        Text->SetPathtype( ( TEXT_PATHTYPE )wxAtoi( m_value ) );
    }

    if ( ReadItem( wxT( "width" ) ) )
        Text->SetTextHeight( ReadDouble() * m_userunits_out );
    else
        Text->SetTextHeight( 1.0 );

    Strans Strans;
    ReadStrans( Strans );

    // this is line to line height in ADS.
    double lineheight = Strans.GetScale();
    double factHeightSize = Text->GetTextHeight()/Text->GetLineHeight();
    Text->SetTextHeight( factHeightSize * lineheight );

    if ( !ReadItem( wxT( "xy" ) ) )
        throw GDS_Error( wxT( "Text: XY is missing (in KEY-file)" ) );

    a2dPoint2D point;
    Read( point );

    a2dAffineMatrix relative_matrix;

    // Ok, now we have a strans, but we want to work with a matrix,
    // so let's convert it to one:

    // Mirror in X if necessary
    if ( Strans.GetReflection() )
        relative_matrix.Mirror();

    //  Rotate if necessary
    if ( Strans.GetAbsAngle() == 0 )
        relative_matrix.Rotate( Strans.GetAngle(), 0, 0 );

    // Scale if necessary
    if ( Strans.GetAbsScale() == 0 )
    {
        //WE DIRECTLY SET IT TO TEXT HEIGHT
        //double scale = Strans.GetScale();
        //relative_matrix.Scale(scale, scale, 0, 0);
    }

    // Translate2D over XY from the structrereference
    relative_matrix.Translate( point.m_x, point.m_y );

    Text->SetTransformMatrix( relative_matrix );
    //Text->SetLineSpacing( Text->GetLineHeight() /  );
    double textheight = Text->GetLineHeight();
    //Text->SetLineSpacing( textheight * -0.37 );
    Text->SetLineSpacing( 0 );

    if ( !ReadItem( wxT( "string" ) ) )
        throw GDS_Error( wxT( "Text: STRING is missing (in KEY-file)" ) );

    Text->SetText( m_value );

    if ( m_layers->GetRead( Text->GetLayer() ) )
        parent->Append( Text );

    return true;
}


// StructureReference -------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadStructureReference( a2dCanvasObject* parent )
{
//  <SREF>              ::= SREF [ELFLAGS] [PLEX] SNAME
//                                  [<strans>] XY
//  <strans>                ::= STRANS [MAG] [ANGLE]

    if ( m_asCameleons )
        return ReadStructureCameleonReference( parent );


    if ( !ReadItem( wxT( "sref" ) ) )
        return false;

    a2dPoint2D point;
    a2dAffineMatrix relative_matrix;
    a2dSmrtPtr<class a2dCanvasObjectReference> sref = new a2dCanvasObjectReference();
    try
    {
        ReadElflags( sref );
        ReadItem( wxT( "plex" ) );

        if ( !ReadItem( wxT( "sname" ) ) )
            throw GDS_Error( wxT( "Sref: SNAME is missing (in KEY-file)" ) );
        sref->SetName( m_value );
        //wxLogDebug(wxT("structure reference= %s"), m_value.c_str() );

        Strans strans;
        ReadStrans( strans );

        if ( !ReadItem( wxT( "xy" ) ) )
            throw GDS_Error( wxT( "Sref: XY is missing (in Structure Reference)" ) );

        Read( point );

        // Ok, now we have a strans, but we want to work with a matrix,
        // so let's convert it to one:

        // Mirror in X if necessary
        if ( strans.GetReflection() )
            relative_matrix.Mirror();

        //  Rotate if necessary
        if ( strans.GetAbsAngle() == 0 )
            relative_matrix.Rotate( strans.GetAngle(), 0, 0 );

        // Scale if necessary
        if ( strans.GetAbsScale() == 0 )
        {
            double scale = strans.GetScale();
            relative_matrix.Scale( scale, scale, 0, 0 );
        }

        // Translate2D over XY from the structure reference
        relative_matrix.Translate( point.m_x, point.m_y );

        sref->SetTransformMatrix( relative_matrix );

        parent->Append( sref );
        ResolveOrAddLink( sref.Get(), sref->GetName() );
    }
    catch ( GDS_Error& _error )
    {
        throw _error;
    }

    return true;
}

// StructureReference -------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadStructureCameleonReference( a2dCanvasObject* parent )
{
//  <SREF>              ::= SREF [ELFLAGS] [PLEX] SNAME
//                                  [<strans>] XY
//  <strans>                ::= STRANS [MAG] [ANGLE]


    if ( !ReadItem( wxT( "sref" ) ) )
        return false;

    a2dPoint2D point;
    a2dAffineMatrix relative_matrix;
    a2dSmrtPtr<class a2dCameleonInst> sref = new a2dCameleonInst( 0, 0, (a2dDiagram*) 0 );
    try
    {
        ReadElflags( sref );
        ReadItem( wxT( "plex" ) );

        if ( !ReadItem( wxT( "sname" ) ) )
            throw GDS_Error( wxT( "Sref: SNAME is missing (in KEY-file)" ) );
        sref->SetAppearanceName( m_value + wxT(":layout") );

        sref->SetName( m_value );
        //wxLogDebug(wxT("structure reference= %s"), m_value.c_str() );

        Strans strans;
        ReadStrans( strans );

        if ( !ReadItem( wxT( "xy" ) ) )
            throw GDS_Error( wxT( "Sref: XY is missing (in Structure Reference)" ) );

        Read( point );

        // Ok, now we have a strans, but we want to work with a matrix,
        // so let's convert it to one:

        // Mirror in X if necessary
        if ( strans.GetReflection() )
            relative_matrix.Mirror();

        //  Rotate if necessary
        if ( strans.GetAbsAngle() == 0 )
            relative_matrix.Rotate( strans.GetAngle(), 0, 0 );

        // Scale if necessary
        if ( strans.GetAbsScale() == 0 )
        {
            double scale = strans.GetScale();
            relative_matrix.Scale( scale, scale, 0, 0 );
        }

        // Translate2D over XY from the structure reference
        relative_matrix.Translate( point.m_x, point.m_y );

        sref->SetTransformMatrix( relative_matrix );

        parent->Append( sref );
        ResolveOrAddLink( sref.Get(), sref->GetName() );
    }
    catch ( GDS_Error& _error )
    {
        throw _error;
    }

    return true;
}

// Arrayreference -----------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadArrayReference( a2dCanvasObject* parent )
{
    // Grammar: AREF [ELFLAGS] [PLEX] SNAME [<strans>]
    //              COLROW XY

    if ( !ReadItem( wxT( "aref" ) ) )
        return false;

    a2dPoint2D Point;
    a2dAffineMatrix relative_matrix;
    a2dPoint2D HorPoint;
    a2dPoint2D VerPoint;

    // Create a new Aref
    a2dSmrtPtr<class a2dCanvasObjectArrayReference> arrayref = new a2dCanvasObjectArrayReference();
    try
    {
        ReadElflags( arrayref );

        ReadItem( wxT( "plex" ) );

        if ( !ReadItem( wxT( "sname" ) ) )
            throw GDS_Error( wxT( "Aref: SNAME missing (in Structure Reference)" ) );

        arrayref->SetName( m_value );
        //wxLogDebug(wxT("structure reference= %s"), m_value.c_str() );

        Strans strans;
        ReadStrans( strans );

        if ( !ReadItem( wxT( "colrow" ) ) )
            throw GDS_Error( wxT( "Aref: COLROW is missing (in Array Reference)" ) );

        wxString _int_str_ptr = m_value;
        arrayref->SetColumns( wxAtoi( _int_str_ptr ) );
        _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "," ) ) + 1;
        arrayref->SetRows( wxAtoi( _int_str_ptr ) );

        if ( !ReadItem( wxT( "xy" ) ) )
            throw GDS_Error( wxT( "Aref: XY missing (in Array Reference)" ) );

        Read( Point );
        Read( HorPoint );
        Read( VerPoint );

        arrayref->SetHorzSpace( sqrt( pow( HorPoint.m_x - Point.m_x, 2 ) + pow( HorPoint.m_y - Point.m_y, 2 ) ) / arrayref->GetColumns() );
        arrayref->SetVertSpace( sqrt( pow( VerPoint.m_x - Point.m_x, 2 ) + pow( VerPoint.m_y - Point.m_y, 2 ) ) / arrayref->GetRows() );

        // Ok, now we have a strans, but we want to work with a matrix,
        // so let's convert it to one:

        // Mirror in X if necessary
        if ( strans.GetReflection() )
            relative_matrix.Mirror();

        //  Rotate if necessary
        if ( strans.GetAbsAngle() == 0 )
            relative_matrix.Rotate( strans.GetAngle(), 0, 0 );

        // Scale if necessary
        if ( strans.GetAbsScale() == 0 )
        {
            EIGHT_G_BYTE_REAL scale = strans.GetScale();
            relative_matrix.Scale( scale, scale, 0, 0 );
        }

        // Translate2D over XY from the structurereference
        relative_matrix.Translate( Point.m_x, Point.m_y );
        arrayref->SetTransformMatrix( relative_matrix );

        parent->Append( arrayref );
        ResolveOrAddLink( arrayref.Get(), arrayref->GetName() );
    }
    catch ( GDS_Error& _error )
    {
        throw _error;
    }

    return true;
}


bool a2dIOHandlerKeyIn::ReadNode( a2dCanvasObject* WXUNUSED( parent ) )
{
// <node>   ::= NODE [ELFLAGS] [PLEX] LAYER NODETYPE [WIDTH] XY

    // not implemented
    if ( ReadItem( wxT( "node" ) ) )
        throw GDS_Error( wxT( "Node: NODE not implemented yet! (in KEY-file)" ) );
    return false;
}


// Box ----------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadBox( a2dCanvasObject* parent )
{
// <box>    ::= BOX [ELFLAGS] [PLEX] LAYER BOXTYPE [WIDTH] XY

    if ( !ReadItem( wxT( "box" ) ) )
        return false;

    // Create a new Boundary
    a2dSmrtPtr<class a2dRect> Box = new a2dRect();

    // Optional
    ReadElflags( Box );
    ReadItem( wxT( "plex" ) );

    // Must be there
    if ( !ReadLayer( Box ) )
        throw GDS_Error( wxT( "Box: LAYER is missing (in KEY-file)" ) );

    if ( !ReadItem( wxT( "boxtype" ) ) )
        throw GDS_Error( wxT( "Box: boxtype is missing (in KEY-file)" ) );
    Box->SetBoxType( wxAtoi( m_value ) );

    if ( ReadItem( wxT( "width" ) ) )
        Box->SetContourWidth( ReadDouble() );

    if ( !ReadItem( wxT( "xy" ) ) )
        return false;

    int points = wxAtoi( m_value );

    if ( points != 5 )
        throw GDS_Error( wxT( "Wrong number of points in BOX XY." ), wxT( "Fatal GDSII error" ) );


    a2dPoint2D seg;
    Read( seg );
    a2dPoint2D seg2;
    Read( seg2 );
    a2dPoint2D seg3;
    Read( seg3 );
    a2dPoint2D seg4;
    Read( seg4 );
    a2dPoint2D seg5;
    Read( seg5 ); // Last point matches first one read

    double minx, miny, maxx, maxy;

    minx = maxx = seg.m_x;
    miny = maxy = seg.m_y;

    minx = wxMin( minx, seg2.m_x );
    maxx = wxMax( maxx, seg2.m_x );
    miny = wxMin( miny, seg2.m_y );
    maxy = wxMax( maxy, seg2.m_y );

    minx = wxMin( minx, seg3.m_x );
    maxx = wxMax( maxx, seg3.m_x );
    miny = wxMin( miny, seg3.m_y );
    maxy = wxMax( maxy, seg3.m_y );

    minx = wxMin( minx, seg4.m_x );
    maxx = wxMax( maxx, seg4.m_x );
    miny = wxMin( miny, seg4.m_y );
    maxy = wxMax( maxy, seg4.m_y );

    Box->SetWidth( fabs( maxx - minx ) );
    Box->SetHeight( fabs( maxy - miny ) );
    Box->SetPosXY( minx, miny );

    if ( m_layers->GetRead( Box->GetLayer() ) )
        parent->Append( Box );

    return true;
}

bool a2dIOHandlerKeyIn::ReadLine( a2dCanvasObject* parent )
{
    if ( !ReadItem( wxT( "line" ) ) )
        return false;

    a2dSmrtPtr<class a2dSLine> line = new a2dSLine();

    // Optional
    ReadElflags( line );
    ReadItem( wxT( "plex" ) );

    // Must be there
    if ( !ReadLayer( line ) )
        throw GDS_Error( wxT( "Line: LAYER is missing (in KEY-file)" ) );

    if ( ReadItem( wxT( "width" ) ) )
        line->SetContourWidth( ReadDouble() );

    if ( !ReadItem( wxT( "xy" ) ) )
        return false;

    a2dPoint2D p1;
    Read( p1 );
    a2dPoint2D p2;
    Read( p2 );
    line->SetPosXY12( p1.m_x, p1.m_y, p2.m_x, p2.m_y );

    if ( m_layers->GetRead( line->GetLayer() ) )
        parent->Append( line );

    return true;
}

bool a2dIOHandlerKeyIn::ReadImage( a2dCanvasObject* parent )
{
    if ( !ReadItem( wxT( "image" ) ) )
        return false;

    a2dSmrtPtr<class a2dImage> image = new a2dImage();

    // Optional
    ReadElflags( image );
    ReadItem( wxT( "plex" ) );

    // Must be there
    if ( !ReadLayer( image ) )
        throw GDS_Error( wxT( "Line: LAYER is missing (in KEY-file)" ) );

    ReadItem( wxT( "width" ) );

    if ( ReadItem( wxT( "w" ) ) )
        image->SetWidth( ReadDouble() * m_userunits_out );

    if ( ReadItem( wxT( "h" ) ) )
        image->SetHeight( ReadDouble() * m_userunits_out );

    wxBitmapType type = wxBITMAP_TYPE_INVALID;
    if ( ReadItem( wxT( "type" ) ) )
    {
        if ( m_value.CmpNoCase( wxT( "gif" ) ) == 0 )
            type = wxBITMAP_TYPE_GIF;
        else if ( m_value.CmpNoCase( wxT( "png" ) )  == 0 )
            type = wxBITMAP_TYPE_PNG;
        else if ( m_value.CmpNoCase( wxT( "bmp" ) )  == 0 )
            type = wxBITMAP_TYPE_BMP;
        else if ( m_value.CmpNoCase( wxT( "jpeg" ) )  == 0 )
            type = wxBITMAP_TYPE_JPEG;
        else
            throw GDS_Error( wxT( "Image: not supported type" ) );
    }

    if ( ReadItem( wxT( "path" ) ) )
        image->SetFilename( m_value, type, true );


    a2dPoint2D point;
    a2dAffineMatrix relative_matrix;
    Strans strans;
    ReadStrans( strans );

    if ( !ReadItem( wxT( "xy" ) ) )
        throw GDS_Error( wxT( "Sref: XY is missing (in Structure Reference)" ) );

    Read( point );

    // Ok, now we have a strans, but we want to work with a matrix,
    // so let's convert it to one:

    // Mirror in X if necessary
    if ( strans.GetReflection() )
        relative_matrix.Mirror();

    //  Rotate if necessary
    if ( strans.GetAbsAngle() == 0 )
        relative_matrix.Rotate( strans.GetAngle(), 0, 0 );

    // Scale if necessary
    if ( strans.GetAbsScale() == 0 )
    {
        double scale = strans.GetScale();
        relative_matrix.Scale( scale, scale, 0, 0 );
    }

    // Translate2D over XY from the structure reference
    relative_matrix.Translate( point.m_x, point.m_y );

    image->SetTransformMatrix( relative_matrix );

    if ( m_layers->GetRead( image->GetLayer() ) )
        parent->Append( image );

    return true;
}

// Elflags ------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadElflags( a2dCanvasObject* object )
{
    if ( !ReadItem( wxT( "elflags" ) ) )
        return false;

    object->SetTemplate( ( bool )( wxAtoi( m_value ) && 1 ) );
    object->SetExternal( ( bool )( wxAtoi( m_value ) && 2 ) );

    return true;
}

// Layer --------------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadLayer( a2dCanvasObject* object )
{
    if ( !ReadItem( wxT( "layer" ) ) )
        return false;

    m_lastElementLayer = wxAtoi( m_value );

    a2dLayerInfo* info = m_mapping[ m_lastElementLayer ];
    if ( !info )
    {
        wxString buf;
        buf.Printf( wxT( "layer %d" ), m_lastElementLayer );
        info = new a2dLayerInfo( m_lastElementLayer, buf );
        m_layers->Append( info );
        m_layers->SetPending( true );
        m_mapping[ m_lastElementLayer ] = info;
    }
    object->SetLayer( m_lastElementLayer );
    return true;
}

// Point --------------------------------------------------------------------
bool a2dIOHandlerKeyIn::Read( a2dPoint2D& Point )
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
bool a2dIOHandlerKeyIn::ReadStrans( Strans& Strans )
{
//  <strans>                ::= STRANS [MAG] [ANGLE]
    if ( !ReadItem( wxT( "strans" ) ) )
        return false;

    wxString _int_str_ptr = m_value;

    Strans.m_stransflags.bits.reflection = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "," ) ) + 1;
    Strans.m_stransflags.bits.abs_angle = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "," ) ) + 1;
    Strans.m_stransflags.bits.abs_scale = wxAtoi( _int_str_ptr );

    if ( ReadItem( wxT( "mag" ) ) )
        Strans.SetScale( ReadDouble() );

    if ( ReadItem( wxT( "angle" ) ) )
        Strans.SetAngle( ReadDouble() );

    return true;
}

// Structure ----------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadStructure( a2dCanvasObject* parent )
{
    // Grammar: BGNSTR STRNAME [STRCLASS] {<element>}*
    //              ENDSTR

    if( !ReadItem( wxT( "bgnstr" ) ) ) // BgnStr expected here
        return false;

    if( !ReadItem( wxT( "creation" ) ) )
        throw GDS_Error( wxT( "Structure: CREATION is missing (in KEY-file)" ) );

    wxString _int_str_ptr = m_value;

    a2dSmrtPtr<class a2dCanvasObject> Structure = new a2dCanvasObject();
    //structures contains object on layers, but is itself just an abstract object without layer
    Structure->SetIgnoreLayer( true );

    //TODO store as property on current object????
    wxDateTime m_modificationtime = wxDateTime::Now();
    wxDateTime m_accesstime = wxDateTime::Now();

    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    year = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    month = wxAtoi( _int_str_ptr ) - 1;
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    day = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "  " ) ) + 2;
    hour = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    minute = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    second = wxAtoi( _int_str_ptr );

    m_modificationtime.Set( day, ( wxDateTime::Month )month, year, hour, minute, second );
    a2dCanvasObject::PROPID_ModificationDateTime->SetPropertyToObject( Structure, m_modificationtime );

    if( !ReadItem( wxT( "lastmod" ) ) )
        throw GDS_Error( wxT( "Structure: LASTACC is missing (in KEY-file)" ) );

    _int_str_ptr = m_value;

    year = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    month = wxAtoi( _int_str_ptr ) - 1;
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    day = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "  " ) ) + 2;
    hour = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    minute = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    second = wxAtoi( _int_str_ptr );

    m_accesstime.Set( day, ( wxDateTime::Month )month, year, hour, minute, second );
    a2dCanvasObject::PROPID_AccessDateTime->SetPropertyToObject( Structure, m_accesstime );

    if ( !ReadItem( wxT( "strname" ) ) )
        throw GDS_Error( wxT( "Structure: STRNAME missing (in KEY-file)" ) );

    Structure->SetName( m_value );

    ReadItem( wxT( "strclass" ) ); // Just read it

    while ( ReadParameters( Structure ) );

    while ( ReadElement( Structure ) )
        ;

    //wxLogDebug(wxT("structure = %s"), Structure->GetName().c_str() );

    if ( !ReadItem( wxT( "endstr" ) ) )
    {
        wxString errbuf;
        errbuf.Printf( wxT( "Unknown Element: %s (in KEY-file)" ), m_keyword.c_str() );
        throw GDS_Error( errbuf );
    }

    if ( m_asMultiroot )
    {      
        a2dNameReference* ref = new a2dNameReference( m_gridx, m_gridy,  Structure, Structure->GetName() );
        ref->SetDrawFrame();
        parent->Append( ref );

        if ( m_gridx < 5 * (10 * a2dCanvasGlobals->GetHabitat()->GetObjectGridSize()) )
            m_gridx = a2dCanvasGlobals->GetHabitat()->GetObjectGridSize()*10 + m_gridx; 
        else
        {    
            m_gridx = 0;
            m_gridy = a2dCanvasGlobals->GetHabitat()->GetObjectGridSize() + m_gridy;
        }
    }
    else
        parent->Append( Structure );

    // this one needs to be resolved later
    GetObjectHashMap()[ Structure->GetName() ] = Structure;
    //wxLogDebug(wxT("structure= %s"), strucname.c_str() );

    return true;
}

// Structure ----------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadCameleonStructure( a2dCanvasObject* parent )
{
    // Grammar: BGNSTR STRNAME [STRCLASS] {<element>}*
    //              ENDSTR

    if( !ReadItem( wxT( "bgnstr" ) ) ) // BgnStr expected here
        return false;

    if( !ReadItem( wxT( "creation" ) ) )
        throw GDS_Error( wxT( "Structure: CREATION is missing (in KEY-file)" ) );

    wxString _int_str_ptr = m_value;

    a2dSmrtPtr<class a2dCameleon> cam = new a2dCameleon();
    a2dLayout* layout = new a2dLayout( cam, 0,0 );
    cam->AddAppearance( layout ); 
    a2dSmrtPtr<a2dDrawing> Structure = layout->GetDrawing();
    Structure->SetLayerSetup( m_layers );

    //TODO store as property on current object????
    wxDateTime m_modificationtime = wxDateTime::Now();
    wxDateTime m_accesstime = wxDateTime::Now();

    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    year = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    month = wxAtoi( _int_str_ptr ) - 1;
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    day = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "  " ) ) + 2;
    hour = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    minute = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    second = wxAtoi( _int_str_ptr );

    m_modificationtime.Set( day, ( wxDateTime::Month )month, year, hour, minute, second );
    a2dCanvasObject::PROPID_ModificationDateTime->SetPropertyToObject( Structure, m_modificationtime );

    if( !ReadItem( wxT( "lastmod" ) ) )
        throw GDS_Error( wxT( "Structure: LASTACC is missing (in KEY-file)" ) );

    _int_str_ptr = m_value;

    year = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    month = wxAtoi( _int_str_ptr ) - 1;
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "-" ) ) + 1;
    day = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( "  " ) ) + 2;
    hour = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    minute = wxAtoi( _int_str_ptr );
    _int_str_ptr = wxStrstr( _int_str_ptr, wxT( ":" ) ) + 1;
    second = wxAtoi( _int_str_ptr );

    m_accesstime.Set( day, ( wxDateTime::Month )month, year, hour, minute, second );
    a2dCanvasObject::PROPID_AccessDateTime->SetPropertyToObject( Structure, m_accesstime );

    if ( !ReadItem( wxT( "strname" ) ) )
        throw GDS_Error( wxT( "Structure: STRNAME missing (in KEY-file)" ) );

    cam->SetName( m_value );
    layout->SetName( m_value + wxT(":layout") );

    ReadItem( wxT( "strclass" ) ); // Just read it

    while ( ReadParameters( Structure->GetRootObject() ) );

    while ( ReadElement( Structure->GetRootObject() ) )
        ;

    //wxLogDebug(wxT("structure = %s"), Structure->GetName().c_str() );

    if ( !ReadItem( wxT( "endstr" ) ) )
    {
        wxString errbuf;
        errbuf.Printf( wxT( "Unknown Element: %s (in KEY-file)" ), m_keyword.c_str() );
        throw GDS_Error( errbuf );
    }

    cam->AddToRoot();
    int px = m_dx * 30 * 1.5;
    int py = -m_dy * 10 * 1.5;
    cam->SetPosXY( px, py );
    m_dx++;
    if ( m_dx > m_refMaxx ) 
        { m_dy++; m_dx =0; }

    // this one needs to be resolved later
    GetObjectHashMap()[ Structure->GetName() ] = cam;
    //wxLogDebug(wxT("structure= %s"), strucname.c_str() );

    return true;
}

// Circle -----------------------------------------------------------------
bool a2dIOHandlerKeyIn::ReadCircle( a2dCanvasObject* parent )
{
// <circle> ::= CIRCLE [ELFLAGS] [PLEX] LAYER [WIDTH] XY

    if ( !ReadItem( wxT( "circle" ) ) )
        return false;

    a2dSmrtPtr<class a2dCircle> circle = new a2dCircle();

    // Optional
    ReadElflags( circle );
    ReadItem( wxT( "plex" ) );
    // Must be there
    if ( !ReadLayer( circle ) )
        throw GDS_Error( wxT( "Circle: LAYER is missing (in KEY-file)" ) );

    if ( ReadItem( wxT( "datatype" ) ) )
        SetDataTypeProperty( circle, wxAtoi( m_value ) );

    if ( ReadItem( wxT( "width" ) ) )
        circle->SetContourWidth( ReadDouble() );

    if ( !ReadItem( wxT( "xy" ) ) )
        throw GDS_Error( wxT( "Circle: XY is missing (in KEY-file)" ) );

    a2dPoint2D point;
    Read( point );
    circle->SetPosXY( point.m_x, point.m_y );
    if ( !ReadItem( wxT( "radius" ) ) )
        throw GDS_Error( wxT( "Circle: RADIUS is missing (in KEY-file)" ) );
    circle->SetRadius( ReadDouble() );

    if ( m_layers->GetRead( circle->GetLayer() ) )
        parent->Append( circle );

    return true;
}

bool a2dIOHandlerKeyIn::ReadArc( a2dCanvasObject* parent )
{
// <arc>            ::= ARC [ELFLAGS] [PLEX] LAYER [WIDTH] XY

    if ( !ReadItem( wxT( "arc" ) ) )
        return false;

    a2dSmrtPtr<class a2dArc> arc = new a2dArc();
    //no NON chords in KEY
    arc->SetChord( true );

    // Optional
    ReadElflags( arc );
    ReadItem( wxT( "plex" ) );
    // Must be there
    if ( !ReadLayer( arc ) )
        throw GDS_Error( wxT( "Arc: LAYER is missing (in KEY-file)" ) );

    if ( ReadItem( wxT( "datatype" ) ) )
        SetDataTypeProperty( arc, wxAtoi( m_value ) );

    if ( ReadItem( wxT( "width" ) ) )
        arc->SetContourWidth( ReadDouble() );

    if ( m_layers->GetRead( arc->GetLayer() ) )
    {
        if ( !ReadItem( wxT( "xy" ) ) )
            throw GDS_Error( wxT( "Polyline: XY is missing (in KEY-file)" ) );

        double xs, ys, xe, ye, xm, ym;
        xm = ym = 0.0;

        if ( !ReadItem( wxT( "x" ) ) ) //read x
            throw GDS_Error( wxT( "Polygon: X is missing (in KEY-file)" ) );

        xs = ReadDouble();

        if ( !ReadItem( wxT( "y" ) ) ) //read y
            throw GDS_Error( wxT( "Polygon: Y is missing (in KEY-file)" ) );

        ys = ReadDouble();

        // Read XM or X
        if ( ReadItem( wxT( "xm" ) ) ) //an arc segment is coming
        {
            xm = ReadDouble();
            if ( !ReadItem( wxT( "ym" ) ) ) //read ym of arc
                throw GDS_Error( wxT( "Polygon: YM is missing (in KEY-file)" ) );
            ym = ReadDouble();

            double xo, yo;
            if ( !ReadItem( wxT( "xo" ) ) ) //read xo of arc
                throw GDS_Error( wxT( "Polygon: XO is missing (in KEY-file)" ) );
            xo = ReadDouble();

            if ( !ReadItem( wxT( "yo" ) ) ) //read yo of arc
                throw GDS_Error( wxT( "Polygon: YO is missing (in KEY-file)" ) );
            yo = ReadDouble();
        }

        if ( !ReadItem( wxT( "x" ) ) ) //read x
            throw GDS_Error( wxT( "Polygon: X is missing (in KEY-file)" ) );
        xe = ReadDouble();

        if ( !ReadItem( wxT( "y" ) ) ) //read y
            throw GDS_Error( wxT( "Polygon: Y is missing (in KEY-file)" ) );
        ye = ReadDouble();

        arc->Set( xs, ys, xm, ym, xe, ye );

        if ( m_layers->GetRead( arc->GetLayer() ) )
            parent->Append( arc );
    }
    else
    {
        SkipXYData();
    }
    return true;
}

// KEY has (X Y ) 
// or ( X Y XM YM XO YO ) last meaning how to go to next segment, through XM,YM as an arc, stopping at next (X,Y)
// But in wxart2d we want to store that (arc info) at the next segment, meaning how do we go to the next X,Y is stored in the next segment.
bool a2dIOHandlerKeyIn::ReadPolygon( a2dVertexList* poly )
{
    if ( !ReadItem( wxT( "xy" ) ) )
        return false;

    int j = wxAtoi( m_value );
    int i;

    double x, y, xm, ym, xo, yo;
    a2dSegType tprev;
    bool arcPiece;
    bool lastWasArc = false;
    bool firstRead = false;

    for ( i = 0; i < j - 1; i++ )
    {
        a2dSegType t = a2dNORMAL_SEG;
        if ( ReadItem( wxT( "st" ) ) ) //read segtype
        {
            if ( m_value.CmpNoCase( wxT( "N" ) ) == 0 )
                t = a2dNORMAL_SEG;
            else if ( m_value.CmpNoCase( wxT( "H" ) ) == 0 )
                t = a2dHOLE_SEG;
            else if ( m_value.CmpNoCase( wxT( "L" ) ) == 0 )
                t = a2dLINK_SEG;
        }
        if ( ReadItem( "so" ) ) //read segtype
            arcPiece = true;
        else
            arcPiece = false;

        if ( !ReadItem( wxT( "x" ) ) ) //read x
            throw GDS_Error( wxT( "Polygon: X is missing (in KEY-file)" ) );

        x = ReadDouble();

        if ( !ReadItem( wxT( "y" ) ) ) //read y
            throw GDS_Error( wxT( "Polygon: Y is missing (in KEY-file)" ) );

        y = ReadDouble();

        if ( firstRead )
        {
            if ( lastWasArc )
            {
                lastWasArc = false;

                a2dArcSegment* sega = new a2dArcSegment();
                sega->m_x = x;
                sega->m_y = y;
                sega->m_x2 = xm;
                sega->m_y2 = ym;
                sega->SetSegType( tprev );
                poly->push_back( sega );
            }
            else
            {
                a2dLineSegment* seg = new a2dLineSegment();
                seg->m_x = x;
                seg->m_y = y;
                seg->SetSegType( tprev );
                seg->SetArcPiece( arcPiece );
                poly->push_back( seg );
            }
        }
        else
        {
            a2dLineSegment* seg = new a2dLineSegment();
            seg->m_x = x;
            seg->m_y = y;
            seg->SetSegType( t );
            seg->SetArcPiece( arcPiece );
            poly->push_back( seg );

            firstRead = true;
        }

        // Read XM or X
        if ( ReadItem( wxT( "xm" ) ) ) //an arc segment is coming towards next point
        {
            lastWasArc = true;

            xm = ReadDouble();

            if ( !ReadItem( wxT( "ym" ) ) ) //read ym of arc
                throw GDS_Error( wxT( "Polygon: YM is missing (in KEY-file)" ) );
            ym = ReadDouble();

            if ( !ReadItem( wxT( "xo" ) ) ) //read xo of arc
                throw GDS_Error( wxT( "Polygon: XO is missing (in KEY-file)" ) );
            xo = ReadDouble();

            if ( !ReadItem( wxT( "yo" ) ) ) //read yo of arc
                throw GDS_Error( wxT( "Polygon: YO is missing (in KEY-file)" ) );
            yo = ReadDouble();
            tprev = t;
        }
        else if ( !m_keyword.CmpNoCase( wxT( "endel" ) ) || !m_keyword.CmpNoCase( wxT( "x" ) ) ||
                  !m_keyword.CmpNoCase( wxT( "st" ) ) || !m_keyword.CmpNoCase( "so" ) ) // Read is already done
        {
            tprev = t;
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

    // if previous segment was an arc, the closing point (being eqaul to the start),
    // defines the end of the arc.
    if ( lastWasArc )
    {
        lastWasArc = false;

        a2dArcSegment* sega = new a2dArcSegment();
        sega->m_x = x;
        sega->m_y = y;
        sega->m_x2 = xm;
        sega->m_y2 = ym;
        poly->push_back( sega );
    }
    return true;
}

bool a2dIOHandlerKeyIn::ReadPolyline( a2dVertexList* poly )
{
    if ( !ReadItem( wxT( "xy" ) ) )
        throw GDS_Error( wxT( "Polyline: XY is missing (in KEY-file)" ) );

    int j = wxAtoi( m_value );
    int i;
    double x, y, xm, ym, xo, yo;
    bool lastWasArc = false;
    bool firstRead = false;

    for ( i = 0; i < j; i++ )
    {
        if ( !ReadItem( wxT( "x" ) ) ) //read x
            throw GDS_Error( wxT( "Polyline: X is missing (in KEY-file)" ) );
        x = ReadDouble();

        if ( !ReadItem( wxT( "y" ) ) ) //read y
            throw GDS_Error( wxT( "Polyline: Y is missing (in KEY-file)" ) );
        y = ReadDouble();

        if ( firstRead )
        {
            if ( lastWasArc )
            {
                lastWasArc = false;

                a2dArcSegment* sega = new a2dArcSegment();
                sega->m_x = x; //current segment
                sega->m_y = y;
                sega->m_x2 = xm; //as stored when read from the previous segment in the key file
                sega->m_y2 = ym;
                poly->push_back( sega );
            }
            else
            {
                a2dLineSegment* seg = new a2dLineSegment();
                seg->m_x = x;
                seg->m_y = y;
                poly->push_back( seg );
            }
        }
        else
        {
            // even if Arc, we will store that info at the next segment.
            a2dLineSegment* seg = new a2dLineSegment();
            seg->m_x = x;
            seg->m_y = y;
            poly->push_back( seg );

            firstRead = true;
        }

        // Read XM or X
        if ( ReadItem( wxT( "xm" ) ) ) //an arc segment is coming towards next point
        {
            // arcs in wxArt2D are stored at the next segment.
            lastWasArc = true;

            xm = ReadDouble();

            if ( !ReadItem( wxT( "ym" ) ) ) //read ym of arc
                throw GDS_Error( wxT( "Polygon: YM is missing (in KEY-file)" ) );
            ym = ReadDouble();

            if ( !ReadItem( wxT( "xo" ) ) ) //read xo of arc
                throw GDS_Error( wxT( "Polygon: XO is missing (in KEY-file)" ) );
            xo = ReadDouble();

            if ( !ReadItem( wxT( "yo" ) ) ) //read yo of arc
                throw GDS_Error( wxT( "Polygon: YO is missing (in KEY-file)" ) );
            yo = ReadDouble();
        }
        else if ( !m_keyword.CmpNoCase( wxT( "endel" ) ) || !m_keyword.CmpNoCase( wxT( "x" ) )
                  || !m_keyword.CmpNoCase( wxT( "st" ) ) || !m_keyword.CmpNoCase( wxT( "so" ) )
                  || !m_keyword.CmpNoCase( wxT( "property" ) ) ) // Read is already done
        {
        }
        else
            throw GDS_Error( wxT( "Polyline: X, XM, ST, PROPERTY or ENDEL missing (in KEY-file)" ) );
    }
    // DO NOT: Skip last coordinate (is not always same as 1st)
    if ( lastWasArc )
    {
        throw GDS_Error( wxT( "Last Segment Should not be an Arc segment (in KEY-file)" ) );
    }

    //poly->RemoveRedundant();
    return true;
}

// Elflags ------------------------------------------------------------------
void a2dIOHandlerKeyIn::SetFlags( a2dCanvasObject* Element )
{
    if ( ( Element->GetTemplate() != 0 ) ||
            ( Element->GetExternal() != 0 ) )
    {
        m_objectFlags = 0;

        if ( Element->GetTemplate() != 0 )
            m_objectFlags += 1 << 1;

        if ( Element->GetExternal() != 0 )
            m_objectFlags += 1 << 2;
    }
}

void a2dIOHandlerKeyIn::SetDataTypeProperty( a2dCanvasObject* toobject, wxUint16 type )
{
    if ( type != 0 )
        a2dCanvasObject::PROPID_Datatype->SetPropertyToObject( toobject, type );
}

// ----------------------------------------------------------------------------
// a2dIOHandlerKeyOut
// ----------------------------------------------------------------------------

a2dIOHandlerKeyOut& a2dIOHandlerKeyOut::operator<<( const wxChar* string )
{
    WriteString( wxString( string ) );
    return *this;
}

a2dIOHandlerKeyOut& a2dIOHandlerKeyOut::operator<<( const wxString& string )
{
    WriteString( string );
    return *this;
}

a2dIOHandlerKeyOut& a2dIOHandlerKeyOut::operator<<( char c )
{
    WriteString( wxString::FromAscii( c ) );

    return *this;
}

#if wxUSE_UNICODE && wxWCHAR_T_IS_REAL_TYPE

a2dIOHandlerKeyOut& a2dIOHandlerKeyOut::operator<<( wchar_t wc )
{
    WriteString( wxString( &wc, m_conv, 1 ) );

    return *this;
}

#endif // wxUSE_UNICODE

a2dIOHandlerKeyOut& a2dIOHandlerKeyOut::operator<<( wxInt16 c )
{
    wxString str;
    str.Printf( wxT( "%d" ), ( signed int )c );
    WriteString( str );

    return *this;
}

a2dIOHandlerKeyOut& a2dIOHandlerKeyOut::operator<<( wxInt32 c )
{
    wxString str;
    str.Printf( wxT( "%ld" ), ( signed long )c );
    WriteString( str );

    return *this;
}

a2dIOHandlerKeyOut& a2dIOHandlerKeyOut::operator<<( wxUint16 c )
{
    wxString str;
    str.Printf( wxT( "%u" ), ( unsigned int )c );
    WriteString( str );

    return *this;
}

a2dIOHandlerKeyOut& a2dIOHandlerKeyOut::operator<<( wxUint32 c )
{
    wxString str;
    str.Printf( wxT( "%lu" ), ( unsigned long )c );
    WriteString( str );

    return *this;
}

a2dIOHandlerKeyOut& a2dIOHandlerKeyOut::operator<<( double f )
{
    WriteDouble( f );
    return *this;
}

a2dIOHandlerKeyOut& a2dIOHandlerKeyOut::operator<<( float f )
{
    WriteDouble( ( double )f );
    return *this;
}

const wxChar a2dIOHandlerKeyOut::Endl()
{
    return wxT( '\n' );
}

// -------------- use it -----------------

a2dIOHandlerKeyOut::a2dIOHandlerKeyOut()
{
    m_scale_out = 1;
    m_lastElementLayer = 0;
    m_textAsPath = false;
    m_precision = 6;
    m_fromViewAsTop = false;
    m_asCameleons = false;
}

a2dIOHandlerKeyOut::~a2dIOHandlerKeyOut()
{
}

bool a2dIOHandlerKeyOut::CanSave( const wxObject* obj )
{
    if ( !wxDynamicCast( obj, a2dCanvasDocument ) )
        return false;
    return true;
}

void a2dIOHandlerKeyOut::InitializeSave(  )
{
    m_points_written = 0;

    a2dIOHandlerStrOut::InitializeSave();
    a2dCanvasGlobals->GetHabitat()->SetAberPolyToArc( double( a2dCanvasGlobals->GetHabitat()->GetAberPolyToArc() ) / m_doc->GetUnitsScale() );
}

void a2dIOHandlerKeyOut::ResetSave(  )
{
    a2dIOHandlerStrOut::ResetSave();
}

bool a2dIOHandlerKeyOut::LinkReferences()
{
    bool res = a2dIOHandler::LinkReferences();

    //search in the root object for the childs that have the bin flag set,
    //which means they are referenced, and can be deleted.
    //All othere are top structures.
    a2dCanvasObjectList::iterator rootchild = m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->begin();
    while ( rootchild != m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->end() )
    {
        a2dCanvasObjectList::iterator delnode = rootchild;
        rootchild++;
        if ( ( *delnode )->GetCheck() )
        {
            m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->erase( delnode );
        }
    }
    return res;
}

bool a2dIOHandlerKeyOut::Save( a2dDocumentOutputStream& stream, const wxObject* doc )
{
    m_doc = ( a2dCanvasDocument* ) doc;
    InitializeSave();

    m_streamo = &stream;

    a2dCanvasObject* showobject = m_doc->GetStartObject();
    if ( !showobject )
        showobject = m_doc->GetDrawing()->GetRootObject();

    if ( m_fromViewAsTop )
    {
        a2dCanvasView* drawer = wxDynamicCast( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetCurrentView(), a2dCanvasView );
        if ( drawer )
            showobject = drawer->GetDrawingPart()->GetShowObject();
    }

    m_layers = m_doc->GetDrawing()->GetLayerSetup();

    if  ( !m_AberArcToPoly )
        m_AberArcToPoly = double( m_doc->GetDrawing()->GetHabitat()->GetAberPolyToArc() ) / m_doc->GetUnitsScale();

    a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::BIN );
    setflags.Start( showobject, false );

    for ( a2dLayerIndex::iterator it= m_layers->GetLayerSort().begin(); it != m_layers->GetLayerSort().end(); ++it)
    {
        a2dLayerInfo* layerobj = *it;
        wxUint16 i = layerobj->GetLayer();
        m_mapping[ layerobj->GetOutMapping() ] = layerobj;
    }

    //first check if the dat is not to big to fit in four byte integers
    //this is the maximum for vertexes in GDSII data

    a2dBoundingBox drawing = showobject->GetBbox();

    m_userunits_out = m_doc->GetUnitsAccuracy();
    drawing.SetMin( drawing.GetMinX() / m_userunits_out, drawing.GetMinY() / m_userunits_out );
    drawing.SetMax( drawing.GetMaxX() / m_userunits_out, drawing.GetMaxY() / m_userunits_out );
    a2dBoundingBox maxint( LONG_MIN, LONG_MIN, LONG_MAX, LONG_MAX );

    int l = 0;
    while ( maxint.Intersect( drawing, 0 ) != _IN )
    {
        drawing.SetMin( drawing.GetMinX() / 10.0, drawing.GetMinY() / 10.0 );
        drawing.SetMax( drawing.GetMaxX() / 10.0, drawing.GetMaxY() / 10.0 );
        l++;
    }
    m_scale_out = pow( 10.0, l );

    *this << wxT( "# KEY file for GDS-II postprocessing tool" ) << Endl();
    *this << wxT( "# File = " ) << m_doc->GetTitle() << Endl();
    *this << wxT( "# ====================================================================" ) << Endl() << Endl();

    *this << wxT( "HEADER " ) << m_doc->GetVersion() << wxT( "; # version " ) << Endl();
    *this << wxT( "BGNLIB; " ) << Endl();
    *this << wxT( "LASTMOD {" )
          << m_doc->GetModificationTime().GetYear() << wxT( "-" )
          << m_doc->GetModificationTime().GetMonth() + 1 << wxT( "-" )
          << m_doc->GetModificationTime().GetDay() << wxT( "  " )
          << m_doc->GetModificationTime().GetHour() << wxT( ":" )
          << m_doc->GetModificationTime().GetMinute() << wxT( ":" )
          << m_doc->GetModificationTime().GetSecond() << wxT( "}; # last modification time" ) << Endl();
    *this << wxT( "LASTACC {" )
          << m_doc->GetAccessTime().GetYear() << wxT( "-" )
          << m_doc->GetAccessTime().GetMonth() + 1 << wxT( "-" )
          << m_doc->GetAccessTime().GetDay() << wxT( "  " )
          << m_doc->GetAccessTime().GetHour() << wxT( ":" )
          << m_doc->GetAccessTime().GetMinute() << wxT( ":" )
          << m_doc->GetAccessTime().GetSecond() << wxT( "}; # last access time" ) << Endl();

    *this << wxT( "LIBNAME \"" ) << m_doc->GetLibraryName() << wxT( "\"; " ) << Endl();

    //start writing

    if ( m_layers )
    {
        *this << wxT( "FORMAT 1; " ) << Endl();

        for ( a2dLayerIndex::iterator it= m_layers->GetOrderSort().begin(); it != m_layers->GetOrderSort().end(); ++it)
        {
            a2dLayerInfo* layerobj = *it;
            wxUint16 j = layerobj->GetLayer();

            if ( a2dLayerInfo::PROPID_TemporaryObject->GetPropertyValue( layerobj ) )
                continue;

            //wxLogDebug( wxT("index %d layername %s, layer %d order %d"), j, layerobj->GetName(), layerobj->GetLayer(), layerobj->GetOrder() );

            //important!
            //if layer is visible it will be rendered
            if ( layerobj->GetAvailable() && layerobj->GetVisible() )
            {
                //wxLogDebug( "layer %d", layerobj->GetLayer() );
                *this << wxT( "MASK \"" ) << layerobj->GetName() << wxT( "\"; " );
                *this << wxT( "layernr " ) << layerobj->GetLayer() << wxT( "; " );

                wxString brushColour = wxTheColourDatabase->FindName( layerobj->GetFill().GetColour() );

                if ( brushColour.IsEmpty() )
                    *this << wxT( "color \"" ) << wxT( "#" ) + ColourToHex( layerobj->GetFill().GetColour() ) << wxT( "\"; " );
                else
                    *this << wxT( "color \"" ) << brushColour << wxT( "\"; " );
                if ( layerobj->GetFill().GetStyle() != a2dFILL_TRANSPARENT )
                    *this << wxT( "trans " ) << layerobj->GetFill().GetAlpha() << wxT( "; " );
                else
                    *this << wxT( "trans " ) << 0 << wxT( "; " );
                *this << wxT( "linestyle " ) << StrokeStyle2String( layerobj->GetStroke().GetStyle() ) << wxT( "; " );

                *this << wxT( "binding {*}; " );
                *this << wxT( "stream " ) << j << wxT( "; " );
                *this << wxT( "iges " ) << j << wxT( "; " );
                *this << wxT( "type " ) << 1 << wxT( "; " );
                *this << wxT( "dxf " ) << 1 << wxT( "; " );
                if ( !layerobj->GetFill().GetFilling() ) //OUTLINE 0 FILL 1 BOTH 2
                    *this << wxT( "plotmode " ) << 0 << wxT( "; " );
                else
                    *this << wxT( "plotmode " ) << 1 << wxT( "; " );
                *this << wxT( "protected " ) << 0 << wxT( "; " );
                *this << wxT( "visible " ) << 1 << wxT( "; " );
                *this << wxT( "patternnr " ) << 0 << wxT( "; " );
                *this << Endl();
            }
        }

        *this << wxT( "ENDMASKS" ) << Endl();
        *this << Endl();
    }

    *this << wxT( "UNITS " ) << m_doc->GetUnits() << wxT( "; " ) << Endl()
          << wxT( "USERUNITS " ) << wxString::FromCDouble(m_userunits_out ) << wxT( "; " )
          << wxT( "PHYSUNITS " ) << wxString::FromCDouble(m_doc->GetUnitsScale()*m_userunits_out* m_scale_out ) << wxT( "; " ) << Endl();


    a2dCanvasObjectList towrite;
    if ( m_doc->GetDrawing()->GetDrawingId() == a2dDrawingId_cameleonrefs() )
    {
        if ( m_fromViewAsTop )
            towrite.push_back( showobject );
        else
        {
            a2dCanvasObjectList::iterator rootchild = m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->begin();
            while ( rootchild != m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->end() )
            {
                a2dCameleon* ref = wxDynamicCast( (*rootchild).Get(), a2dCameleon );
                if ( ref )
                {
                    //a2dLayout* layout = ref->GetCameleon()->GetAppearance<a2dDiagram>();
                    //towrite.push_back( layout->GetDrawing() );
                    //layout->SetBin( false );
                    towrite.push_back( ref );
                    ref->SetBin( false );
                }
                rootchild++;
            }
        }
    }
    else
    {
        if ( m_fromViewAsTop || !m_doc->GetMultiRoot() )
            towrite.push_back( showobject );
        else
        {
            m_doc->GetDrawing()->GetRootObject()->GetChildObjectList()->CollectObjects( &towrite );
        }
    }

    a2dCanvasObjectList::iterator iter = towrite.begin();
    while ( towrite.size() )
    {
        a2dCanvasObject* obj = *iter;
        if ( m_doc->GetMultiRoot() &&  !m_fromViewAsTop )
        {
            a2dNameReference* ref = wxDynamicCast( obj, a2dNameReference );
            if ( ref )
                SaveStructure( ref->GetCanvasObject(), &towrite );
        }
        else
            SaveStructure( obj, &towrite );

        obj->SetBin( true );
        towrite.erase( iter );
        iter = towrite.begin();
    }

    *this << wxT( "ENDLIB; " ) << Endl();

    ResetSave();

    return true;
}

void a2dIOHandlerKeyOut::SaveStructure( a2dCanvasObject* object, a2dCanvasObjectList* towrite )
{
    SetFlags( object );

    if (  !object->GetRelease() )
    {
        if ( !object->GetBin() )
        {
            //write a struture BEGIN

            // BGNSTR
            *this << Endl() << wxT( "BGNSTR; # Begin of structure " ) << Endl();
            *this << wxT( "CREATION {" )
                  << m_doc->GetModificationTime().GetYear() << wxT( "-" )
                  << m_doc->GetModificationTime().GetMonth() + 1 << wxT( "-" )
                  << m_doc->GetModificationTime().GetDay() << wxT( "  " )
                  << m_doc->GetModificationTime().GetHour() << wxT( ":" )
                  << m_doc->GetModificationTime().GetMinute() << wxT( ":" )
                  << m_doc->GetModificationTime().GetSecond() << wxT( "}; # creation time" ) << Endl();
            *this << wxT( "LASTMOD {" )
                  << m_doc->GetAccessTime().GetYear() << wxT( "-" )
                  << m_doc->GetAccessTime().GetMonth() + 1 << wxT( "-" )
                  << m_doc->GetAccessTime().GetDay() << wxT( "  " )
                  << m_doc->GetAccessTime().GetHour() << wxT( ":" )
                  << m_doc->GetAccessTime().GetMinute() << wxT( ":" )
                  << m_doc->GetAccessTime().GetSecond() << wxT( "}; # last modification time" ) << Endl();

            *this << wxT( "STRNAME " ) << object->GetName() << wxT( "; " ) << Endl();

            if ( 0 != wxDynamicCast( object, a2dCameleon ) )
            {
                a2dCameleon* cam = wxDynamicCast( object, a2dCameleon );
                //a2dDiagram* dia = wxDynamicCast( cam->GetAppearanceByName( cam->GetName() ), a2dDiagram );
                a2dDiagram* dia = wxDynamicCast( cam->GetAppearance<a2dDiagram>(), a2dDiagram );
                if ( dia )
                { 
                    a2dCanvasObjectList* childobjects = dia->GetDrawing()->GetRootObject()->GetChildObjectList();
                    if ( childobjects != wxNullCanvasObjectList )
                    {
                        forEachIn( a2dCanvasObjectList, childobjects )
                        {
                            a2dCanvasObject* obj = *iter;
                            if (  !obj->GetRelease() &&  m_layers->GetVisible( obj->GetLayer() ) || obj->GetIgnoreLayer() )
                            {
                                Save( obj, towrite );
                            }
                        }
                    }
                }
            }
            else
            {
                a2dCanvasObjectList* childobjects = object->GetChildObjectList();
                if ( childobjects != wxNullCanvasObjectList )
                {
                    forEachIn( a2dCanvasObjectList, childobjects )
                    {
                        a2dCanvasObject* obj = *iter;
                        if (  !obj->GetRelease() &&  m_layers->GetVisible( obj->GetLayer() ) || obj->GetIgnoreLayer() )
                        {
                            Save( obj, towrite );
                        }
                    }
                }
            }
            *this << Endl() << wxT( "ENDSTR " ) << object->GetName() << wxT( "; " ) << Endl();
        }
    }
}

void a2dIOHandlerKeyOut::Save( a2dCanvasObject* object, a2dCanvasObjectList* towrite )
{
    if( object->IsTemporary_DontSave() )
        return;

    bool skip = false;

    //in case an object can not be written as is, and need to be converted to vector paths, and written that as childs.
    a2dCanvasObjectList* vectorpaths = wxNullCanvasObjectList;

    SetFlags( object );
    //write the object itself as part of the structure (which is the parent )
    if ( 0 != wxDynamicCast( object, a2dPolylineL ) )
    {
        a2dPolylineL* obj = wxDynamicCast( object, a2dPolylineL );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dSurface ) )
    {
        a2dSurface* obj = wxDynamicCast( object, a2dSurface );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dPolygonL ) )
    {
        a2dPolygonL* obj = wxDynamicCast( object, a2dPolygonL );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dVectorPath ) )
    {
        a2dVectorPath* obj = wxDynamicCast( object, a2dVectorPath );
        vectorpaths = obj->GetAsCanvasVpaths( false );
    }
    else if ( 0 != wxDynamicCast( object, a2dCanvasObjectArrayReference ) )
    {
        a2dCanvasObjectArrayReference* obj = wxDynamicCast( object, a2dCanvasObjectArrayReference );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dCanvasObjectReference ) )
    {
        a2dCanvasObjectReference* obj = wxDynamicCast( object, a2dCanvasObjectReference );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dCameleonInst ) )
    {
        a2dCameleonInst* obj = wxDynamicCast( object, a2dCameleonInst );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dCameleon ) )
    {
        a2dCameleon* obj = wxDynamicCast( object, a2dCameleon );
        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dTextGDS ) )
    {
        a2dTextGDS* obj = wxDynamicCast( object, a2dTextGDS );

        if ( !m_textAsPath )
            DoSave( obj, towrite );
        else
            vectorpaths = obj->GetAsCanvasVpaths( false );
    }
    else if ( 0 != wxDynamicCast( object, a2dText ) )
    {
        a2dText* obj = wxDynamicCast( object, a2dText );

        if ( !m_textAsPath )
            DoSave( obj, towrite );
        else
            vectorpaths = obj->GetAsCanvasVpaths( false );
    }
    else if ( 0 != wxDynamicCast( object, a2dCircle ) )
    {
        a2dCircle* obj = wxDynamicCast( object, a2dCircle );

        DoSave( obj, towrite );
    }
    else if ( 0 != wxDynamicCast( object, a2dRect ) )
    {
        vectorpaths = object->GetAsCanvasVpaths( false );
    }
    else if (   ( 0 != wxDynamicCast( object, a2dArrow ) ) ||
                ( 0 != wxDynamicCast( object, a2dRectC ) ) ||
                ( 0 != wxDynamicCast( object, a2dEllipse ) ) ||
                ( 0 != wxDynamicCast( object, a2dEllipticArc ) ) ||
                ( 0 != wxDynamicCast( object, a2dArc ) ) ||
                ( 0 != wxDynamicCast( object, a2dSLine ) )
            )
    {
        vectorpaths = object->GetAsCanvasVpaths( false );
    }
    else if ( wxString( object->GetClassInfo()->GetClassName() ) == wxT( "a2dCanvasObject" ) )
    {
        *this  << Endl() << wxT( "SREF; " ) << Endl();

        WriteFlags( m_objectFlags );

        *this << wxT( "SNAME " ) << object->GetName() << wxT( "; " ) << Endl();

        //write a structure reference in the parent object
        a2dAffineMatrix lworld = object->GetTransformMatrix();
        if ( !lworld.IsIdentity() )
        {
            Strans strans = Strans();
            strans.MakeStrans( lworld );
            if ( strans.GetStrans() )
                Write( &strans );
        }

        *this << Endl() << wxT( "   XY 1; " ) << Endl();

        WritePoint( object->GetPosX(), object->GetPosY() );

        *this << Endl() << wxT( "ENDEL; " ) << Endl();
        towrite->push_back( object );

        skip = true;
    }
    else
    {
        wxLogWarning( wxT( "KEYIO : object %s is not implemented for output" ), object->GetClassInfo()->GetClassName() );
        skip = true;
    }

    if ( !skip )
    {
        //remark properties are lost when the object is converted into vectorpaths

        //now write extra vector paths ( e.g. as a result of a converted object )
        if ( vectorpaths != wxNullCanvasObjectList )
        {
            a2dAffineMatrix lworld = object->GetTransformMatrix();

            for( a2dCanvasObjectList::iterator iter = vectorpaths->begin(); iter != vectorpaths->end(); ++iter )
            {
                a2dVectorPath* canpath = ( a2dVectorPath* ) ( *iter ).Get();
                WriteVpath( lworld, canpath->GetSegments(), canpath->GetLayer(), canpath->GetDataType(), canpath->GetPathType(), canpath->GetContourWidth() );
            }
            delete vectorpaths;
        }
        else
        {
            WriteProperties( object->GetPropertyList() );
            WriteParameters( object->GetPropertyList() );
            *this << Endl() << wxT( "ENDEL; " ) << Endl();
        }

        //write normal childs.
        if ( object->GetChildObjectList() != wxNullCanvasObjectList &&  object->GetChildObjectsCount() )
        {
            *this  << Endl() << wxT( "SREF; " ) << Endl();

            WriteFlags( m_objectFlags );

            *this << wxT( "SNAME " ) << object->GetName() << wxT( "; " ) << Endl();

            //write a structure reference in the parent object
            a2dAffineMatrix lworld = object->GetTransformMatrix();
            if ( !lworld.IsIdentity() )
            {
                Strans strans = Strans();
                strans.MakeStrans( lworld );
                if ( strans.GetStrans() )
                    Write( &strans );
            }

            *this << Endl() << wxT( "   XY 1; " ) << Endl();

            WritePoint( object->GetPosX(), object->GetPosY() );

            *this << Endl() << wxT( "ENDEL; " ) << Endl();
            towrite->push_back( object );
        }
    }
}

void a2dIOHandlerKeyOut::DoSave( a2dPolylineL* obj, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    WriteVertexListPolyline( obj->GetTransformMatrix(), obj->GetSegments(),
                             obj->GetLayer(), a2dCanvasObject::PROPID_Datatype->GetPropertyValue( obj ),
                             obj->GetPathType(),
                             obj->GetContourWidth(), obj->GetSpline(), false );

    /*
        double xscale = obj->GetEndScaleX();
        double yscale = obj->GetEndScaleY();

        if ( obj->GetBegin() )
        {
            a2dVertexList::compatibility_iterator node = rlist->GetFirst();
            a2dLineSegment* point1 = (a2dLineSegment*)node->GetData();
            node = node->GetNext();
            a2dLineSegment* point2 = (a2dLineSegment*)node->GetData();

            double dx, dy;
            dx=point2->m_x-point1->m_x;
            dy=point2->m_y-point1->m_y;

            double ang1;
            if (!dx && !dy)
                ang1=0;
            else
                ang1 = wxRadToDeg(atan2(dy,dx));

            a2dAffineMatrix lworld;
            //clockwise rotation so minus
            lworld.Scale(xscale,yscale,0,0);
            lworld.Rotate(-ang1);
            lworld.Translate(point1->m_x,point1->m_y);

            WriteStartElementAttributes( wxT("g") );
            s.Printf ( wxT("matrix( %f %f %f %f %f %f )"),
                       lworld.GetValue(0,0), lworld.GetValue(0,1),
                       lworld.GetValue(1,0), lworld.GetValue(1,1),
                       lworld.GetValue(2,0), lworld.GetValue(2,1)
                     );
            WriteAttribute( wxT("transform"), s );
            WriteEndAttributes();

            Save( obj->GetBegin(), layer );

            WriteEndElement();
        }

        if ( obj->GetEnd() )
        {
            a2dVertexList::compatibility_iterator node = rlist->GetLast();
            a2dLineSegment* point1 = (a2dLineSegment*)node->GetData();
            node = node->GetPrevious();
            a2dLineSegment* point2 = (a2dLineSegment*)node->GetData();

            double dx, dy;
            dx=point2->m_x-point1->m_x;
            dy=point2->m_y-point1->m_y;
            double ang2;
            if (!dx && !dy)
                ang2=0;
            else
                ang2 = wxRadToDeg(atan2(dy,dx));

            a2dAffineMatrix lworld;
            //clockwise rotation so minus
            lworld.Scale(xscale,yscale,0,0);
            lworld.Rotate(-ang2);
            lworld.Translate(point1->m_x,point1->m_y);

            WriteStartElementAttributes( wxT("g") );
            s.Printf ( wxT("matrix( %f %f %f %f %f %f )"),
                       lworld.GetValue(0,0), lworld.GetValue(0,1),
                       lworld.GetValue(1,0), lworld.GetValue(1,1),
                       lworld.GetValue(2,0), lworld.GetValue(2,1)
                     );
            WriteAttribute( wxT("transform"), s );
            WriteEndAttributes();

            Save( obj->GetEnd(), layer );

            WriteEndElement();
        }
    */

}

void a2dIOHandlerKeyOut::DoSave( a2dPolygonL* obj, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    WriteVertexListPolygon( obj->GetTransformMatrix(), obj->GetSegments(), obj->GetLayer(), a2dCanvasObject::PROPID_Datatype->GetPropertyValue( obj ), obj->GetContourWidth(), obj->GetSpline(), false );
}

void a2dIOHandlerKeyOut::DoSave( a2dSurface* surface, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    a2dAffineMatrix lworld = surface->GetTransformMatrix();
    a2dVertexList* points = surface->GetSegments();

    m_points_written = 0;

    *this << Endl() << wxT( "SURFACE; " );

    WriteFlags( m_objectFlags );

    *this << wxT( "LAYER " ) << m_mapping[ surface->GetLayer() ]->GetLayer() << wxT( ";  " );
    *this << wxT( "DATATYPE " ) << a2dCanvasObject::PROPID_Datatype->GetPropertyValue( surface ) << wxT( ";  " );

    WritePoly( lworld, points, true );

    a2dListOfa2dVertexList& holes = surface->GetHoles();
    for( a2dListOfa2dVertexList::iterator iterp = holes.begin(); iterp != holes.end(); iterp++ )
    {
        *this << Endl() << wxT( "HOLE; " );
        a2dVertexListPtr vlist = ( *iterp );
        if ( vlist->HasArcs() )
            vlist->ConvertToLines(m_AberArcToPoly);
        m_points_written = 0;
        WritePoly( lworld, vlist, true );
    }
}

void a2dIOHandlerKeyOut::DoSave( a2dText* obj, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    *this  << Endl() << wxT( "TEXT; " );

    WriteFlags( m_objectFlags );

    *this << wxT( "LAYER " ) << m_mapping[obj->GetLayer()]->GetLayer() << wxT( "; " ) << Endl();

    *this << wxT( "TEXTTYPE " ) << TEXT_PATH_END_SQAURE << wxT( "; " );

    // for the moment this is in the strans together with matrix
    //if( obj->GetTextHeight() != 0 )
    //  *this << wxT("WIDTH ") << obj->GetTextHeight() / m_scale_out << wxT("; ");

    a2dAffineMatrix lworld = obj->GetTransformMatrix();
    if ( !lworld.IsIdentity() )
    {
        Strans strans = Strans();
        strans.MakeStrans( lworld );

        if( obj->GetTextHeight() != 0 )
            strans.SetScale( strans.GetScale() * obj->GetTextHeight() );

        if ( strans.GetStrans() )
            Write( &strans );
    }
    *this << Endl() << wxT( "   XY 1; " ) << Endl();

    WritePoint( obj->GetPosX(), obj->GetPosY() );

    *this << Endl() << wxT( "STRING {" ) << obj->GetText() << wxT( "}; " );
}

void a2dIOHandlerKeyOut::DoSave( a2dTextGDS* obj, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    *this  << Endl() << wxT( "TEXT; " );

    WriteFlags( m_objectFlags );

    *this << wxT( "LAYER " ) << m_mapping[obj->GetLayer()]->GetLayer() << wxT( "; " ) << Endl();

    *this << wxT( "TEXTTYPE " ) << obj->GetTextType() << wxT( "; " );

    if ( obj->GetPresentationFlags() )
    {
        // write only when a_presentation isn't default
        if ( ( obj->GetFontGDS() != DEFAULT_PRESENTATION_FONT ) ||
                ( obj->GetVertical() != DEFAULT_PRESENTATION_VERTICAL ) ||
                ( obj->GetHorizontal() != DEFAULT_PRESENTATION_HORIZONTAL ) )
        {
            *this << wxT( "PRESENTATION " ) <<
                  ( int )obj->GetHorizontal() << wxT( "," ) <<
                  ( int )obj->GetVertical() << wxT( "," ) <<
                  ( int )obj->GetFontGDS() << wxT( "; " );
        }
    }
    if ( obj->GetPathtype() )
        *this << wxT( "PATHTYPE " ) << obj->GetPathtype() << wxT( "; " );

    // for the moment this is in the strans together with matrix
    //if( obj->GetTextHeight() != 0 )
    //  *this << wxT("WIDTH ") << obj->GetTextHeight() / m_scale_out << wxT("; ");

    a2dAffineMatrix lworld = obj->GetTransformMatrix();
    if ( !lworld.IsIdentity() )
    {
        Strans strans = Strans();
        strans.MakeStrans( lworld );

        if( obj->GetTextHeight() != 0 )
            strans.SetScale( strans.GetScale() * obj->GetTextHeight() );

        if ( strans.GetStrans() )
            Write( &strans );
    }
    *this << Endl() << wxT( "   XY 1; " ) << Endl();

    WritePoint( obj->GetPosX(), obj->GetPosY() );

    *this << Endl() << wxT( "STRING {" ) << obj->GetText() << wxT( "}; " );
}

void a2dIOHandlerKeyOut::DoSave( a2dCanvasObjectReference* obj, a2dCanvasObjectList* towrite )
{
    *this  << Endl() << wxT( "SREF; " ) << Endl();

    WriteFlags( m_objectFlags );

    *this << wxT( "SNAME " ) << obj->GetName() << wxT( "; " ) << Endl();

    //write a structure reference in the parent object
    a2dAffineMatrix lworld = obj->GetTransformMatrix();
    if ( !lworld.IsIdentity() )
    {
        Strans strans = Strans();
        strans.MakeStrans( lworld );
        if ( strans.GetStrans() )
            Write( &strans );
    }

    *this << Endl() << wxT( "   XY 1; " ) << Endl();

    WritePoint( obj->GetPosX(), obj->GetPosY() );

    if ( obj->GetCanvasObject() )
        towrite->push_back( obj->GetCanvasObject() );
}

void a2dIOHandlerKeyOut::DoSave( a2dCanvasObjectArrayReference* obj, a2dCanvasObjectList* towrite )
{
    *this  << Endl() << wxT( "AREF; " ) << Endl();

    WriteFlags( m_objectFlags );

    *this << wxT( "SNAME " ) << obj->GetName() << wxT( ";  " );

    a2dAffineMatrix lworld = obj->GetTransformMatrix();
    if ( !lworld.IsIdentity() )
    {
        Strans strans = Strans();
        strans.MakeStrans( lworld );
        if ( strans.GetStrans() )
            Write( &strans );
    }

    *this << wxT( "COLROW {" )
          << obj->GetColumns() << wxT( " , " )
          << obj->GetRows() << wxT( "};  " );

    m_points_written = 0;

    *this << Endl() << wxT( "   XY 3; " ) << Endl();

    WritePoint( obj->GetPosX(), obj->GetPosY() );
    WritePoint( obj->GetPosX() + obj->GetHorzSpace() * obj->GetColumns(), obj->GetPosY() );
    WritePoint( obj->GetPosX(), obj->GetPosY() + obj->GetVertSpace() * obj->GetRows() );

    if ( obj->GetCanvasObject() )
        towrite->push_back( obj->GetCanvasObject() );
}

void a2dIOHandlerKeyOut::DoSave( a2dCameleonInst* obj, a2dCanvasObjectList* towrite )
{
    a2dDiagram* dia = wxDynamicCast( obj->GetAppearance(), a2dDiagram );
    if ( ! dia )
        return;

    *this  << Endl() << wxT( "SREF; " ) << Endl();

    WriteFlags( m_objectFlags );

    *this << wxT( "SNAME " ) << dia->GetCameleon()->GetName() << wxT( "; " ) << Endl();

    //write a structure reference in the parent object
    a2dAffineMatrix lworld = obj->GetTransformMatrix();
    if ( !lworld.IsIdentity() )
    {
        Strans strans = Strans();
        strans.MakeStrans( lworld );
        if ( strans.GetStrans() )
            Write( &strans );
    }

    *this << Endl() << wxT( "   XY 1; " ) << Endl();

    WritePoint( obj->GetPosX(), obj->GetPosY() );

    if ( m_asCameleons && dia->GetCameleon() )
        towrite->push_back( dia->GetCameleon() );
}

void a2dIOHandlerKeyOut::DoSave( a2dCameleon* obj, a2dCanvasObjectList* towrite )
{
    SaveStructure( obj, towrite );
}

// Circle -----------------------------------------------------------------
void a2dIOHandlerKeyOut::DoSave( a2dCircle* Circle, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    *this << Endl() << wxT( "CIRCLE; " );

    WriteFlags( m_objectFlags );

    *this << wxT( "LAYER " ) << m_mapping[Circle->GetLayer()]->GetLayer() << wxT( ";  " );
    *this << wxT( "DATATYPE " ) << a2dCanvasObject::PROPID_Datatype->GetPropertyValue( Circle ) << wxT( "; " );

    if ( Circle->GetContourWidth() )
        *this << wxT( "WIDTH " ) << Circle->GetContourWidth() / m_scale_out << wxT( "; " );

    // write the coordinate to the outputstream
    m_points_written = 0;

    *this << Endl() << wxT( "   XY 1; " ) << Endl();

    WritePoint( Circle->GetPosX(), Circle->GetPosY() );

    *this << wxT( "RADIUS " ) << Circle->GetRadius() / m_scale_out << wxT( ";  " ) << Endl();
}

void a2dIOHandlerKeyOut::Write( Strans* Strans )
{
    // writes only when strans isn't default
    if ( ( Strans->m_stransflags.bits.abs_angle != DEFAULT_STRANS_ABS_ANGLE ) ||
            ( Strans->m_stransflags.bits.abs_scale != DEFAULT_STRANS_ABS_SCALE ) ||
            ( Strans->m_stransflags.bits.reflection != DEFAULT_STRANS_REFLECTION ) ||
            ( Strans->GetScale() != DEFAULT_STRANS_SCALE ) ||
            ( Strans->GetAngle() != DEFAULT_STRANS_ANGLE ) )
    {

        *this << wxT( "STRANS " )
              << Strans->m_stransflags.bits.reflection << wxT( "," )
              << Strans->m_stransflags.bits.abs_angle << wxT( "," )
              << Strans->m_stransflags.bits.abs_scale << wxT( "; " );

        // writes only the scale when different from default
        if ( Strans->GetScale() != DEFAULT_STRANS_SCALE )
            *this << wxT( "MAG " ) << Strans->GetScale() << wxT( "; " );

        // writes only the angle when different from default
        if ( Strans->GetAngle() != DEFAULT_STRANS_ANGLE )
            *this << wxT( "ANGLE " ) << Strans->GetAngle() << wxT( "; " );
    }
}

// Elflags ------------------------------------------------------------------
void a2dIOHandlerKeyOut::SetFlags( a2dCanvasObject* Element )
{
    if ( ( Element->GetTemplate() != 0 ) ||
            ( Element->GetExternal() != 0 ) )
    {
        m_objectFlags = 0;

        if ( Element->GetTemplate() != 0 )
            m_objectFlags += 1 << 1;

        if ( Element->GetExternal() != 0 )
            m_objectFlags += 1 << 2;
    }
}

void a2dIOHandlerKeyOut::WriteFlags( int flags )
{
    if ( ( flags && 2 != 0 ) ||
            ( flags && 1 != 0 ) )
    {
        int value = 0;

#ifdef _G_UNIX
        if ( ( bool ) ( flags && 2 ) )
            value += 2 ^ 1;
        if ( ( bool ) ( flags && 1 ) )
            value += 2 ^ 2;
#else
        if ( ( bool ) ( flags && 1 ) )
            value += 2 ^ 2;
        if ( ( bool ) ( flags && 2 ) )
            value += 2 ^ 1;
#endif

        *this << wxT( "ELFLAGS " ) << value << wxT( ";  " );
    }
}

// Point --------------------------------------------------------------------
void a2dIOHandlerKeyOut::WritePoint( double xi, double yi )
{
    if ( !( m_points_written % 2 ) )
        *this << Endl() << wxT( "   " );

    m_points_written++;

    double x;
    double y;
    if ( xi > 0 )
        x = xi / m_scale_out;
    else
        x = xi / m_scale_out;
    if ( yi > 0 )
        y = yi / m_scale_out;
    else
        y = yi / m_scale_out;

    *this <<  wxT( "X " ) << x << wxT( "; " );
    *this <<  wxT( "Y " ) << y << wxT( "; " );
}

// Segment  -----------------------------------------------------------------
void a2dIOHandlerKeyOut::WriteSegment(  const a2dAffineMatrix& lworld, a2dLineSegment* Segment, a2dLineSegment* nextSegment )
{
    if ( !( m_points_written % 2 ) )
        *this << Endl() << wxT( "   " );

    m_points_written++;

    switch ( Segment->GetSegType() )
    {
        case a2dNORMAL_SEG:break;
        case a2dLINK_SEG: *this << wxT( "ST L; " ); break;
        case a2dHOLE_SEG: *this << wxT( "ST H; " ); break;
    }

    if ( Segment->GetArcPiece() )
        *this << wxT( "SO A; " ); 

    double x, y;
    lworld.TransformPoint( Segment->m_x, Segment->m_y, x, y );

    *this <<  wxT( "X " ) << x / m_scale_out << wxT( "; " );
    *this <<  wxT( "Y " ) << y / m_scale_out << wxT( ";   " );

    if ( nextSegment->GetArc() )
    {
        if ( !( m_points_written % 2 ) )
            *this << Endl() << wxT( "   " );
        m_points_written++;
        a2dArcSegment* aseg = ( a2dArcSegment* ) nextSegment;
        lworld.TransformPoint( aseg->m_x2, aseg->m_y2, x, y );

        *this <<  wxT( "XM " ) << x / m_scale_out << wxT( "; " );
        *this <<  wxT( "YM " ) << y / m_scale_out << wxT( ";   " );
        if ( !( m_points_written % 2 ) )
            *this << Endl() << wxT( "   " );
        m_points_written++;

        lworld.TransformPoint( aseg->GetOx( *Segment ), aseg->GetOy( *Segment ), x, y );

        *this <<  wxT( "XO " ) << x / m_scale_out << wxT( "; " );
        *this <<  wxT( "YO " ) << y / m_scale_out << wxT( ";   " );
    }
}

void a2dIOHandlerKeyOut::WriteVpath( const a2dAffineMatrix& lworld, const a2dVpath* path, int layer, int datatype, int pathtype, double width )
{
    double tstep = 1 / ( double ) SPLINE_STEP;
    unsigned int i;

    a2dVertexArray* cpoints = new a2dVertexArray;

    double x, y;
    bool move = false;
    int count = 0;
    bool nostrokeparts = false;

    //first draw as much as possible ( nostroke parts may stop this first round )
    for ( i = 0; i < path->size(); i++ )
    {
        a2dVpathSegment* seg = path->Item( i );
        switch ( seg->GetType()  )
        {
            case a2dPATHSEG_MOVETO:
                if ( count == 0 )
                {
                    lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    cpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                }
                else
                {
                    i--;
                    move = true;
                }
                break;

            case a2dPATHSEG_LINETO:
                lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                cpoints->push_back( new a2dLineSegment( x, y ) );
                count++;
                break;
            case a2dPATHSEG_LINETO_NOSTROKE:
                lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                cpoints->push_back( new a2dLineSegment( x, y ) );
                count++;
                nostrokeparts = true;
                break;

            case a2dPATHSEG_CBCURVETO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_CBCURVETO:
            {
                double xw, yw;

                double xwl = path->Item( i ? i - 1 : 0 )->m_x1;
                double ywl = path->Item( i ? i - 1 : 0 )->m_y1;
                a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg;

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                    yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
                    lworld.TransformPoint( xw, yw, x, y );
                    cpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_QBCURVETO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_QBCURVETO:
            {
                double xw, yw;

                double xwl = path->Item( i ? i - 1 : 0 )->m_x1;
                double ywl = path->Item( i ? i - 1 : 0 )->m_y1;
                a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg;

                int step;
                double t = 0;
                for ( step = 0; step <= SPLINE_STEP; step++ )
                {
                    xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                    yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
                    lworld.TransformPoint( xw, yw, x, y );
                    cpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                    t = t + tstep;
                }
            }
            break;

            case a2dPATHSEG_ARCTO_NOSTROKE:
                nostrokeparts = true;
            case a2dPATHSEG_ARCTO:
            {
                a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg;

                double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                if ( cseg->CalcR( path->Item( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                {
                    double dphi;
                    unsigned int segments = 20;
                    a2dGlobals->Aberration( phit, radius , dphi, segments );

                    double theta = beginrad;
                    unsigned int step;

                    for ( step = 0; step < segments + 1; step++ )
                    {
                        lworld.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                        theta = theta + dphi;
                    }
                }
                else
                {
                    lworld.TransformPoint( cseg->m_x1, cseg->m_y1, x, y );
                    cpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                }
            }
            break;
        }

        if ( move )
        {
            WriteVertexArrayPolyline( a2dIDENTITY_MATRIX, cpoints, layer, datatype, pathtype, width, false, true  );
            cpoints->clear();
            move = false;
            count = 0;
        }
        else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
        {
            if ( nostrokeparts || seg->GetClose() == a2dPATHSEG_END_CLOSED_NOSTROKE )
            {
                WriteVertexArrayPolygon( a2dIDENTITY_MATRIX, cpoints, layer, datatype, 0, false, true  );
                nostrokeparts = true;
            }
            else
            {
                WriteVertexArrayPolygon( a2dIDENTITY_MATRIX, cpoints, layer, datatype, 0, false, true );
            }

            cpoints->clear();

            move = false;
            count = 0;
        }
        else if ( i == path->size() - 1 ) //last segment?
        {
            WriteVertexArrayPolyline( a2dIDENTITY_MATRIX, cpoints, layer, datatype, pathtype, width, false, true  );
            cpoints->clear();
        }
    }

    if ( nostrokeparts )
    {
        move = false;
        count = 0;
        cpoints->clear();

        nostrokeparts = false;

        double lastmovex = 0;
        double lastmovey = 0;

        for ( i = 0; i < path->size(); i++ )
        {
            a2dVpathSegment* seg = path->Item( i );
            switch ( seg->GetType()  )
            {
                case a2dPATHSEG_MOVETO:
                    if ( count == 0 )
                    {
                        lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        lastmovex = x;
                        lastmovey = y;
                        count++;
                    }
                    else
                    {
                        i--;
                        move = true;
                    }
                    break;

                case a2dPATHSEG_LINETO:
                    lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                    cpoints->push_back( new a2dLineSegment( x, y ) );
                    count++;
                    break;

                case a2dPATHSEG_LINETO_NOSTROKE:
                case a2dPATHSEG_CBCURVETO_NOSTROKE:
                case a2dPATHSEG_QBCURVETO_NOSTROKE:
                case a2dPATHSEG_ARCTO_NOSTROKE:
                    if ( count == 0 )
                    {
                        lworld.TransformPoint( seg->m_x1, seg->m_y1, x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                    }
                    else
                    {
                        i--;
                        nostrokeparts = true;
                    }
                    break;

                case a2dPATHSEG_CBCURVETO:
                {
                    double xw, yw;

                    double xwl = path->Item( i ? i - 1 : 0 )->m_x1;
                    double ywl = path->Item( i ? i - 1 : 0 )->m_y1;
                    a2dVpathCBCurveSegment* cseg = ( a2dVpathCBCurveSegment* ) seg;

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 3 ) + cseg->m_x2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_x3 * ( 1 - t ) * t * t * 3 + cseg->m_x1 * pow( t, 3 );
                        yw = ywl * pow( 1 - t, 3 ) + cseg->m_y2 * pow( 1 - t, 2 ) * t * 3 + cseg->m_y3 * ( 1 - t ) * t * t * 3 + cseg->m_y1 * pow( t, 3 );
                        lworld.TransformPoint( xw, yw, x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                        t = t + tstep;
                    }
                }
                break;

                case a2dPATHSEG_QBCURVETO:
                {
                    double xw, yw;

                    double xwl = path->Item( i ? i - 1 : 0 )->m_x1;
                    double ywl = path->Item( i ? i - 1 : 0 )->m_y1;
                    a2dVpathQBCurveSegment* cseg = ( a2dVpathQBCurveSegment* ) seg;

                    int step;
                    double t = 0;
                    for ( step = 0; step <= SPLINE_STEP; step++ )
                    {
                        xw = xwl * pow( 1 - t, 2 ) + cseg->m_x2 * ( 1 - t ) * t * 2 + cseg->m_x1 * pow( t, 2 );
                        yw = ywl * pow( 1 - t, 2 ) + cseg->m_y2 * ( 1 - t ) * t * 2 + cseg->m_y1 * pow( t, 2 );
                        lworld.TransformPoint( xw, yw, x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                        t = t + tstep;
                    }
                }
                break;

                case a2dPATHSEG_ARCTO:
                {
                    a2dVpathArcSegment* cseg = ( a2dVpathArcSegment* ) seg;

                    double radius, center_x, center_y, beginrad, midrad, endrad, phit;

                    if ( cseg->CalcR( path->Item( i ? i - 1 : 0 ), radius, center_x, center_y, beginrad, midrad, endrad, phit ) )
                    {
                        double dphi;
                        unsigned int segments = 20;
                        a2dGlobals->Aberration( phit, radius , dphi, segments );

                        double theta = beginrad;
                        unsigned int step;

                        for ( step = 0; step < segments + 1; step++ )
                        {
                            lworld.TransformPoint( center_x + radius * cos ( theta ), center_y + radius * sin ( theta ), x, y );
                            cpoints->push_back( new a2dLineSegment( x, y ) );
                            count++;
                            theta = theta + dphi;
                        }
                    }
                    else
                    {
                        lworld.TransformPoint( cseg->m_x1, cseg->m_y1, x, y );
                        cpoints->push_back( new a2dLineSegment( x, y ) );
                        count++;
                    }
                }
                break;
            }

            if ( move || nostrokeparts )
            {
                WriteVertexArrayPolyline( a2dIDENTITY_MATRIX, cpoints, layer, datatype, pathtype, width, true  );
                cpoints->clear();
                move = false;
                nostrokeparts = false;
                count = 0;
            }
            else if ( seg->GetClose() != a2dPATHSEG_END_OPEN )
            {
                if ( seg->GetClose() == a2dPATHSEG_END_CLOSED )
                {
                    cpoints->push_back( new a2dLineSegment( lastmovex, lastmovey ) );
                    count++;
                }
                WriteVertexArrayPolyline( a2dIDENTITY_MATRIX, cpoints, layer, datatype, pathtype, width, true  );
                cpoints->clear();
                nostrokeparts = false;
                move = false;
                count = 0;
            }
            else if ( i == path->size() )
            {
                WriteVertexArrayPolyline( a2dIDENTITY_MATRIX, cpoints, layer, datatype, pathtype, width, true  );
                cpoints->clear();
            }
        }
    }

    delete cpoints;

}

// Path ---------------------------------------------------------------------
void a2dIOHandlerKeyOut::WriteVertexArrayPolyline( const a2dAffineMatrix& lworld, a2dVertexArray* points, int layer, int datatype, int pathtype, double width, bool spline, bool close )
{
    m_points_written = 0;
    *this  << Endl() << wxT( "PATH; " );

    WriteFlags( m_objectFlags );

    *this << wxT( "LAYER " ) << m_mapping[layer]->GetLayer() << wxT( "; " );
    *this << wxT( "DATATYPE " ) << datatype << wxT( "; " );

    if ( pathtype )
        *this << wxT( "PATHTYPE " ) << pathtype << wxT( "; " );

    if ( spline )
        *this << wxT( "SPLINE 1 ;" );

    if( width )
        *this << wxT( "WIDTH " ) << width / m_scale_out << wxT( "; " );

    unsigned int n = points->size();

    *this << Endl() << wxT( "   XY " ) << n << wxT( "; " ) << Endl();

    a2dLineSegment* prevseg = 0;
    unsigned int i;
    a2dLineSegment* nextseg = 0;
    for ( i = 0; i < n;  i++ )
    {
        if ( i != n - 1 )
            nextseg = points->Item( i + 1 ).Get();
        else
            nextseg = points->Item( 0 ).Get();
        WriteSegment( lworld, points->Item( i ).Get(), nextseg );
    }

    if ( close )
        *this << Endl() << wxT( "ENDEL; " ) << Endl();
}

// Path ---------------------------------------------------------------------
void a2dIOHandlerKeyOut::WriteVertexListPolyline( const a2dAffineMatrix& lworld, a2dVertexList* points, int layer, int datatype, int pathtype, double width, bool spline, bool close )
{
    m_points_written = 0;

    *this  << Endl() << wxT( "PATH; " );

    WriteFlags( m_objectFlags );

    *this << wxT( "LAYER " ) << m_mapping[layer]->GetLayer() << wxT( "; " );
    *this << wxT( "DATATYPE " ) << datatype << wxT( "; " );

    if ( pathtype )
        *this << wxT( "PATHTYPE " ) << pathtype << wxT( "; " );

    if ( spline )
        *this << wxT( "SPLINE 1 ;" );

    if( width )
        *this << wxT( "WIDTH " ) << width /  m_scale_out << wxT( "; " );

    unsigned int n = points->size();

    WritePoly( lworld , points, false );

    if ( close )
        *this << Endl() << wxT( "ENDEL; " ) << Endl();
}

// Boundary------------------------------------------------------------------
void a2dIOHandlerKeyOut::WriteVertexArrayPolygon( const a2dAffineMatrix& lworld, a2dVertexArray* points, int layer, int datatype, double width, bool spline, bool close )
{
    m_points_written = 0;

    *this << Endl() << wxT( "BOUNDARY; " );

    WriteFlags( m_objectFlags );

    *this << wxT( "LAYER " ) << m_mapping[layer]->GetLayer() << wxT( ";  " );
    *this << wxT( "DATATYPE " ) << datatype << wxT( ";  " );

    if ( spline )
        *this << wxT( "SPLINE 1 ;" );

    if( width )
        *this << wxT( "WIDTH " ) << width / m_scale_out << wxT( "; " );

    unsigned int n = points->size();

    //last point is repeated so + 1
    *this << Endl() << wxT( "XY " ) << n + 1 << wxT( "; " ) << Endl();

    unsigned int i;
    a2dLineSegment* nextseg = 0;
    for ( i = 0; i < n;  i++ )
    {
        if ( i != n - 1 )
            nextseg = points->Item( i + 1 ).Get();
        else
            nextseg = points->Item( 0 ).Get();
        WriteSegment( lworld, points->Item( i ).Get(), nextseg );
    }
    // not a curve!
    //WriteSegment( lworld, points->Item(0).Get(), nextseg );

    WritePoint( points->Item( 0 )->m_x, points->Item( 0 )->m_y );

    if ( close )
    {
        *this << Endl() << wxT( "ENDEL; " ) << Endl();
    }
}

// Boundary------------------------------------------------------------------
void a2dIOHandlerKeyOut::WriteVertexListPolygon( const a2dAffineMatrix& lworld, a2dVertexList* points, int layer, int datatype, double width, bool spline, bool close )
{
    m_points_written = 0;

    *this << Endl() << wxT( "BOUNDARY; " );

    WriteFlags( m_objectFlags );

    *this << wxT( "LAYER " ) << m_mapping[layer]->GetLayer() << wxT( ";  " );
    *this << wxT( "DATATYPE " ) << datatype << wxT( ";  " );

    if ( spline )
        *this << wxT( "SPLINE 1 ;" );

    if( width )
        *this << wxT( "WIDTH " ) << width  / m_scale_out << wxT( "; " );

    unsigned int n = points->size();

    WritePoly( lworld , points, true );

    if ( close )
    {
        *this << Endl() << wxT( "ENDEL; " ) << Endl();
    }
}

void a2dIOHandlerKeyOut::WritePoly( const a2dAffineMatrix& lworld , a2dVertexList* vlist, bool close )
{
    unsigned int n = vlist->size();

    //last point is repeated so + 1
    *this << Endl() << wxT( "XY " ) << ( close ? n + 1 : n ) << wxT( "; " );

    a2dLineSegment* nextseg = 0;
    a2dVertexList::iterator iter = vlist->begin();
    unsigned int i = 0;
    a2dLineSegment* point;
    while ( i < n )
    {
        point = *iter;
        iter++;
        if ( i != n - 1 )
            nextseg = *iter;
        else
            nextseg = vlist->front();

        WriteSegment( lworld, point, nextseg );
        i++;
    }
    if ( close )
        WritePoint( vlist->front()->m_x, vlist->front()->m_y );
}

void a2dIOHandlerKeyOut::WriteProperties( const a2dNamedPropertyList& props )
{
    *this << Endl();
    a2dNamedPropertyList::const_iterator iter;
    for( iter = props.begin(); iter !=  props.end(); ++iter )
    {
        const a2dNamedProperty* prop = *iter;
        if ( prop->GetId()->IsUserDefined() )
        {
            if ( !prop->GetName().StartsWith( "__PAR_") )
            {
                *this << wxT( "PROPERTY {" ) << prop->GetName() << wxT( "};  " );
                if ( wxDynamicCast(  prop, a2dStringProperty ) )
                {
                    *this << wxT( "PROPTYPE string; " );
                    *this << wxT( "PROPVALUE {" ) << prop->StringValueRepresentation() << wxT( "};  " ) << Endl();
                }
                else if ( wxDynamicCast(  prop, a2dInt32Property ) )
                {
                    *this << wxT( "PROPTYPE integer; " );
                    *this << wxT( "PROPVALUE " ) << prop->StringValueRepresentation() << wxT( ";  " ) << Endl();
                }
                else if ( wxDynamicCast(  prop, a2dDoubleProperty ) )
                {
                    *this << wxT( "PROPTYPE real; " );
                    *this << wxT( "PROPVALUE " ) << prop->StringValueRepresentation() << wxT( ";  " ) << Endl();
                }
                else if ( wxDynamicCast(  prop, a2dBoolProperty ) )
                {
                    *this << wxT( "PROPTYPE bool; " );
                    *this << wxT( "PROPVALUE " ) << prop->StringValueRepresentation() << wxT( ";  " ) << Endl();
                }
            }
        }
    }
}

void a2dIOHandlerKeyOut::WriteParameters( const a2dNamedPropertyList& props )
{
    *this << Endl();
    a2dNamedPropertyList::const_iterator iter;
    for( iter = props.begin(); iter !=  props.end(); ++iter )
    {
        const a2dNamedProperty* prop = *iter;
        if ( prop->GetId()->IsUserDefined() )
        {
            if ( prop->GetName().StartsWith( "__PAR_") )
            {
                wxString name = prop->GetName().Right( prop->GetName().Len()-6 );
                *this << wxT( "PARAMETER {" ) << name << wxT( "};  " );
                if ( wxDynamicCast(  prop, a2dStringProperty ) )
                {
                    *this << wxT( "PARAMETERTYPE string; " );
                    *this << wxT( "PARAMETERVALUE {" ) << prop->StringValueRepresentation() << wxT( "};  " ) << Endl();
                }
                else if ( wxDynamicCast(  prop, a2dInt32Property ) )
                {
                    *this << wxT( "PARAMETERTYPE integer; " );
                    *this << wxT( "PARAMETERVALUE " ) << prop->StringValueRepresentation() << wxT( ";  " ) << Endl();
                }
                else if ( wxDynamicCast(  prop, a2dDoubleProperty ) )
                {
                    *this << wxT( "PARAMETERTYPE real; " );
                    *this << wxT( "PARAMETERVALUE " ) << prop->StringValueRepresentation() << wxT( ";  " ) << Endl();
                }
                else if ( wxDynamicCast(  prop, a2dBoolProperty ) )
                {
                    *this << wxT( "PARAMETERTYPE bool; " );
                    *this << wxT( "PARAMETERVALUE " ) << prop->StringValueRepresentation() << wxT( ";  " ) << Endl();
                }
            }
        }
    }
}


#endif //wxART2D_USE_KEYIO
