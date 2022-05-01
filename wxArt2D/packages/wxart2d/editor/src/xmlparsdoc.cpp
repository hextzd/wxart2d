/*!
    \file canvas/src/xmlpars.cpp

    \brief a2dCanvasDocument - XML reader via Expat
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: xmlpars.cpp,v 1.70 2009/07/24 16:35:01 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "a2dprivate.h"

#if wxART2D_USE_CVGIO

#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"
#include "wx/canvas/cameleon.h"
#include "wx/editor/candoc.h"
#include "wx/editor/xmlparsdoc.h"

//----------------------------------------------------------------------------
// a2dIOHandlerDocCVGIn
//----------------------------------------------------------------------------

a2dIOHandlerDocCVGIn::a2dIOHandlerDocCVGIn()
{
    // stay compatible for the moment
    m_objectCreate[ wxT( "a2dOneColourFill" ) ] = wxT( "a2dFill" );
    m_objectCreate[ wxT( "a2dLinearGradientFill" ) ] = wxT( "a2dFill" );
    m_objectCreate[ wxT( "a2dRadialGradientFill" ) ] = wxT( "a2dFill" );
    m_objectCreate[ wxT( "a2dOneColourStroke" ) ] = wxT( "a2dStroke" );
    m_docClassInfo = &a2dCanvasDocument::ms_classInfo;
}


a2dIOHandlerDocCVGIn::~a2dIOHandlerDocCVGIn()
{
}

bool a2dIOHandlerDocCVGIn::CanLoad( a2dDocumentInputStream& stream, const wxObject* obj, wxClassInfo* docClassInfo )
{
    if ( obj && !wxDynamicCast( obj, a2dCanvasDocument ) && !wxDynamicCast( obj, a2dLayers ) )
        return false;

    if ( docClassInfo && m_docClassInfo && !docClassInfo->IsKindOf( m_docClassInfo ) )
        return false;

    m_streami = &stream;
    SeekI( 0 );
    char cheader[170];
    cheader[169] = 0;

    Read( cheader, 169 );
    SeekI( 0 );

    // XML header:
    if ( memcmp( cheader, "<?xml ", 6 ) == 0 )
    {
        wxString buf = XmlDecodeStringToString( cheader, 169 );
        // <cvg classname="a2dCanvasDocument">

        int pos;
        pos = buf.Find( wxT( "<cvg" ) );
        if ( pos == wxNOT_FOUND )
            return false;

        pos = buf.Find( wxT( "classname" ) );
        if ( pos == wxNOT_FOUND )
            return false;

#if wxUSE_STD_IOSTREAM
        return ( m_streami->fail() || m_streami->bad() ) == 0;
#else
        return true;
#endif
    }

    return false;
}

bool a2dIOHandlerDocCVGIn::LoadLayers( a2dDocumentInputStream& stream, a2dLayers* layers )
{
    bool oke = true;

    m_streami = &stream;

    InitializeLoad();

    try
    {

        Next();
        Require( START_TAG, wxT( "cvg" ) );
        Next();
        if ( GetTagName() == wxT( "title" ) )
        {
            Next();
            Require( END_TAG, wxT( "title" ) );
            Next();
        }
        if ( GetTagName() == wxT( "desc" ) )
        {
            Next();
            Require( END_TAG, wxT( "desc" ) );
            Next();
        }

        if ( GetTagName() == wxT( "o" ) && GetAttributeValue( wxT( "classname" ) ) == wxT( "a2dLayers" ) )
        {
            //add ON top existing setup
            layers->Load( NULL, *this );
        }

        Require( END_TAG, wxT( "cvg" ) );
        Next();

        //now resolve references on id.
        //CVG links on Id's
        LinkReferences();
        layers->UpdateIndexes();
    }
    catch ( a2dIOHandlerXMLException& error ) // error from the parser: process here
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "CVG parsing error: '%s' at line %d column %d" ), error.getMessage().c_str(), error.getLineNumber(), error.getColumnNumber() );
        oke = false;
    }

    ResetLoad();
    return oke;
}



bool a2dIOHandlerDocCVGIn::Load( a2dDocumentStringInputStream& stream, a2dCanvasDocument* doc, a2dCanvasObject* parent )
{
    a2dCanvasObject* readobject;
    m_streami = &stream;

    bool oke = true;

    InitializeLoad();

    try
    {
        Next();
        Require( START_TAG, wxT( "cvg" ) );
        Next();
        if ( GetTagName() == wxT( "title" ) )
        {
            doc->SetTitle( GetContent() );
            Next();
            Require( END_TAG, wxT( "title" ) );
            Next();
        }
        if ( GetTagName() == wxT( "desc" ) )
        {
            doc->SetDescription( GetContent() );
            Next();
            Require( END_TAG, wxT( "desc" ) );
            Next();
        }

        if ( GetTagName() == wxT( "a2dLayers" ) )
        {
            //add ON top existing setup
            m_doc->GetLayerSetup()->Load( NULL, *this );
        }

        //this way allows empty documents also
        //Require( START_TAG, "canvasobject" );

        while( GetEventType() == START_TAG && GetTagName() == wxT( "o" ) )
        {
            wxString classname = GetAttributeValue( wxT( "classname" ) );
            readobject = ( a2dCanvasObject* ) CreateObject( classname );
            if ( !readobject )
            {
                a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "could not create a2dCanvasObject %s, will be skipped line %d" ),
                                                 GetAttributeValue( wxT( "classname" ) ).c_str(), GetCurrentLineNumber() );
                return false;
            }
            else
            {
                parent->Append( readobject );
                readobject->Load( NULL, *this );
            }
        }

        Require( END_TAG, wxT( "cvg" ) );
        Next();

        //now resolve references on id.
        //CVG links on Id's
        LinkReferences();
    }
    catch ( a2dIOHandlerXMLException& error ) // error from the parser: process here
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_XMLparse, _( "CVG parsing error: '%s' at line %d column %d" ), error.getMessage().c_str(), error.getLineNumber(), error.getColumnNumber() );
        oke = false;
    }

    ResetLoad();

    return oke;
}

//----------------------------------------------------------------------------
// a2dIOHandlerDocCVGOut
//----------------------------------------------------------------------------

a2dIOHandlerDocCVGOut::a2dIOHandlerDocCVGOut()
{
}


a2dIOHandlerDocCVGOut::~a2dIOHandlerDocCVGOut()
{
}

bool a2dIOHandlerDocCVGOut::SaveLayers( a2dDocumentOutputStream& stream, a2dLayers* layers )
{
    m_streamo = &stream;

#if wxUSE_STD_IOSTREAM
    if ( stream.fail() || stream.bad() )
    {
#else
    if ( !stream.IsOk() )
    {
#endif
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open stream for saving" ) );
        return false;
    }
    InitializeSave();

    //check flag is used for writing once an object which is referenced many times
    a2dWalker_SetCheck setp( false );
    setp.Start( layers );

    WriteStartDocument( wxT( "1.0" ), wxT( "UTF-8" ) , true );

    WriteStartElementAttributes( wxT( "cvg" ) );
    WriteAttribute( wxT( "classname" ), GetClassInfo()->GetClassName() );
    WriteEndAttributes();

    WriteElement( wxT( "title" ), wxT( "layersettings" ) );
    WriteElement( wxT( "desc" ), wxT( "description" ) );

    a2dObjectList towritelayer;
    layers->Save( NULL, *this, &towritelayer );

    WriteEndElement();
    WriteEndDocument();

    ResetSave();

    return true;
}

bool a2dIOHandlerDocCVGOut::SaveStartAt( a2dDocumentOutputStream& stream, const a2dCanvasDocument* doc, a2dCanvasObject* start )
{
    m_doc = ( a2dCanvasDocument* ) doc;
    m_streamo = &stream;

#if wxUSE_STD_IOSTREAM
    if ( stream.fail() || stream.bad() )
    {
#else
    if ( !stream.IsOk() )
    {
#endif
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open stream for saving" ) );
        return false;
    }

    InitializeSave();
    m_doc->Save( *this, start );
    ResetSave();

    return true;
}

void a2dIOHandlerDocCVGOut::WriteCvgStartDocument( a2dDocumentOutputStream& stream )
{
    m_streamo = &stream;

    InitializeSave();

    WriteStartDocument( wxT( "1.0" ), wxT( "UTF-8" ) , true );

    WriteStartElementAttributes( wxT( "cvg" ) );
    WriteAttribute( wxT( "classname" ), GetClassInfo()->GetClassName() );
    WriteEndAttributes();

    WriteElement( wxT( "title" ), wxT( "" ) );
    WriteElement( wxT( "desc" ), wxT( "" ) );
}

void a2dIOHandlerDocCVGOut::WriteCvgEndDocument()
{
    WriteEndElement();
    WriteEndDocument();
    ResetSave( );
}

void a2dIOHandlerDocCVGOut::WriteObject( a2dCanvasObject* start )
{
    //check flag is used for writing once an object which is referenced many times
    a2dWalker_SetCheck setp( false );
    setp.Start( start );

    a2dObjectList towrite;
    towrite.push_back( start );

    a2dObjectList::iterator iter = towrite.begin();
    while ( towrite.size() )
    {
        a2dObject* obj = *iter;
        obj->Save( NULL, *this, &towrite );
        towrite.erase( iter );
        iter = towrite.begin();
    }
}

#endif /* wxART2D_USE_CVGIO */
