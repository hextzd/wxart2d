/*! \file docview/samples/common/doc.cpp
    \brief Implements document functionality
    \author Julian Smart
    \date Created 04/01/98

    Copyright: (c) Julian Smart and Markus Holzem

    Licence: wxWidgets license

    RCS-ID: $Id: doc.cpp,v 1.6 2009/07/24 16:35:20 titato Exp $
*/

#include "docviewprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include "doc.h"

IMPLEMENT_DYNAMIC_CLASS( DrawingDocument, a2dDocument )

DrawingDocument::DrawingDocument( void )
{
    m_documentTypeName = _T( "DrawingDocument" );
}

DrawingDocument::~DrawingDocument( void )
{
    doodleSegments.DeleteContents( true );
}

a2dDocumentOutputStream& DrawingDocument::SaveObject( a2dDocumentOutputStream& stream, a2dIOHandlerStrOut* handler )
{
#if wxUSE_STD_IOSTREAM
    if ( !stream.fail() && !stream.bad() )
    {
        a2dDocumentOutputStream& streamw = stream;
#else
    if ( stream.IsOk() )
    {
        wxTextOutputStream streamw( stream );
#endif

        if ( handler )
        {
            if ( handler->CanSave( this ) )
            {
                if ( !handler->Save( stream, this ) )
                    wxLogError( _( "template I/O handler error during saving in this format." ) );
                return stream;
            }
            else
                wxLogError( _( "template I/O handler not capable of saving in this format." ) );
        }
        else
        {
            //fall back on Native format for which an Iohandler is not required
            wxInt32 n = doodleSegments.GetCount();
            streamw << n << '\n';

            wxList::compatibility_iterator iSegment = doodleSegments.GetFirst();
            while ( iSegment )
            {
                DoodleSegment* segment = ( DoodleSegment* )iSegment->GetData();
                segment->SaveObject( stream );
                streamw << '\n';

                iSegment = iSegment->GetNext();
            }
        }
    }
    else
        wxLogError( _( "bad stream" ) );

    return stream;
}

a2dDocumentInputStream& DrawingDocument::LoadObject( a2dDocumentInputStream& stream, a2dIOHandlerStrIn* handler )
{

    if ( ! m_documentTemplate )
    {
        wxLogError( _( "No template defined" ) );
        return stream;
    }

    handler = m_documentTemplate->GetDocumentIOHandlerForLoad( stream, this );
    if ( handler )
    {
        if ( !handler->Load( stream, this ) )
            wxLogError( _( "template I/O handler not capable of loading this format." ) );
    }
    else
        wxLogError( _( "template I/O handler not available to load this file." ) );

    return stream;
}


DoodleSegment::DoodleSegment()
{
    m_colour = *wxBLACK;
    m_width = 0;
}

DoodleSegment::DoodleSegment( const wxColour& colour, int penwidth )
{
    m_colour = colour;
    m_width = penwidth;
}

DoodleSegment::DoodleSegment( DoodleSegment& seg )
{
    m_colour = seg.m_colour;
    m_width = seg.m_width;

    wxList::compatibility_iterator iLine = seg.m_lines.GetFirst();
    while ( iLine )
    {
        DoodleLine* line = ( DoodleLine* )iLine->GetData();
        DoodleLine* newLine = new DoodleLine;
        newLine->x1 = line->x1;
        newLine->y1 = line->y1;
        newLine->x2 = line->x2;
        newLine->y2 = line->y2;

        m_lines.Append( newLine );

        iLine = iLine->GetNext();
    }
}

DoodleSegment::~DoodleSegment( void )
{
    m_lines.DeleteContents( true );
}


a2dDocumentOutputStream& DoodleSegment::SaveObject( a2dDocumentOutputStream& stream )
{
    wxInt32 n = m_lines.GetCount();

#if wxUSE_STD_IOSTREAM
    a2dDocumentOutputStream& streamw = stream;
#else
    wxTextOutputStream streamw( stream );
#endif

    streamw
            << n
            << _T( '\n' )
            << _T( "#" ) << ColourToHex( m_colour )
            << _T( " " ) << m_width
            << _T( "\n" );

    wxList::compatibility_iterator iLine = m_lines.GetFirst();
    while ( iLine )
    {
        DoodleLine* line = ( DoodleLine* )iLine->GetData();

        streamw
                << line->x1
                << _T( " " ) << line->y1
                << _T( " " ) << line->x2
                << _T( " " ) << line->y2
                << _T( "\n" );

        iLine = iLine->GetNext();
    }

    return stream;
}

a2dDocumentInputStream& DoodleSegment::LoadObject( a2dDocumentInputStream& stream )
{

#if wxUSE_STD_IOSTREAM
    a2dDocumentInputStream& wstream = stream;
    wxInt32 n = 0;
    wstream >> n;
    char c[100];
    wstream.getline( c, 100, ' ' );
    wstream >> m_width;
    wxString strokeColour( wxString( c, wxConvUTF8 ) );
#else
    wxTextInputStream wstream( stream );
    wxInt32 n = 0;
    wstream >> n;
    wxString strokeColour;
    wstream >> strokeColour >> m_width;
#endif

    m_colour = HexToColour( strokeColour.After( '#' ) );

    for ( int i = 0; i < n; i++ )
    {
        DoodleLine* line = new DoodleLine;
        wstream >> line->x1 >> line->y1 >>  line->x2 >>  line->y2;
        m_lines.Append( line );
    }

    return stream;
}

void DoodleSegment::Draw( wxDC* dc )
{
    wxPen pen = wxPen( m_colour, m_width, wxPENSTYLE_SOLID );
    dc->SetPen( pen );

    wxList::compatibility_iterator iLine = m_lines.GetFirst();
    while ( iLine )
    {
        DoodleLine* line = ( DoodleLine* )iLine->GetData();
        dc->DrawLine(
            ( int ) line->x1,
            ( int ) line->y1,
            ( int ) line->x2,
            ( int ) line->y2 );
        iLine = iLine->GetNext();
    }
}

/*
* Implementation of drawing command
*/

const a2dCommandId DrawingCommandAddSegment::COMID_AddSegment( wxT( "AddSegment" ) );

DrawingCommandAddSegment::DrawingCommandAddSegment( DrawingDocument* ddoc, DoodleSegment* seg ):
    a2dCommand( true, COMID_AddSegment )
{
    doc = ddoc;
    segment = seg;
}

DrawingCommandAddSegment::~DrawingCommandAddSegment( void )
{
    if ( segment )
        delete segment;
}

bool DrawingCommandAddSegment::Do( void )
{
    doc->GetDoodleSegments().Append( new DoodleSegment( *segment ) );
    doc->Modify( true );
    doc->UpdateAllViews();
    return true;
}

bool DrawingCommandAddSegment::Undo( void )
{
    // Cut the last segment
    if ( doc->GetDoodleSegments().GetCount() > 0 )
    {
        wxList::compatibility_iterator iSegment = doc->GetDoodleSegments().GetLast();
        DoodleSegment* seg = ( DoodleSegment* )iSegment->GetData();
        delete seg;
        doc->GetDoodleSegments().Erase( iSegment );

        doc->Modify( true );
        doc->UpdateAllViews();
    }
    return true;
}

const a2dCommandId DrawingCommandCutSegment::COMID_CutSegment( wxT( "CutSegment" ) );

DrawingCommandCutSegment::DrawingCommandCutSegment( DrawingDocument* ddoc, DoodleSegment* seg ):
    a2dCommand( true, COMID_CutSegment )
{
    doc = ddoc;
    segment = seg;
}

DrawingCommandCutSegment::~DrawingCommandCutSegment( void )
{
    if ( segment )
        delete segment;
}

bool DrawingCommandCutSegment::Do( void )
{
    // Cut the last segment
    if ( doc->GetDoodleSegments().GetCount() > 0 )
    {
        wxList::compatibility_iterator iSegment =
            doc->GetDoodleSegments().GetLast();
        if ( segment )
            delete segment;

        segment = ( DoodleSegment* )iSegment->GetData();
        doc->GetDoodleSegments().Erase( iSegment );

        doc->Modify( true );
        doc->UpdateAllViews();
    }
    return true;
}

bool DrawingCommandCutSegment::Undo( void )
{
    // Paste the segment
    if ( segment )
    {
        doc->GetDoodleSegments().Append( segment );
        doc->Modify( true );
        doc->UpdateAllViews();
        segment = ( DoodleSegment* ) NULL;
    }
    doc->Modify( true );
    doc->UpdateAllViews();
    return true;
}

DoodleDocumentIoHandlerIn::DoodleDocumentIoHandlerIn()
{
}

DoodleDocumentIoHandlerIn::~DoodleDocumentIoHandlerIn( void )
{
}

bool DoodleDocumentIoHandlerIn::CanLoad( a2dDocumentInputStream& stream, const wxObject* obj, wxClassInfo* docClassInfo )
{

    if ( obj && !wxDynamicCast( obj, DrawingDocument ) )
        return false;

    wxString doctype = docClassInfo->GetClassName();
    if ( docClassInfo && doctype != wxT( "DrawingDocument" ) )
        return false;

#if wxUSE_STD_IOSTREAM
    stream.seekg( 0 );
    char cheader[30];
    cheader[29] = 0;

    stream.read( cheader, 29 );
    stream.seekg( 0 );

#else
    stream.SeekI( 0 );
    char cheader[30];
    cheader[29] = 0;

    stream.Read( cheader, 29 );
    stream.SeekI( 0 );

#endif

    return ( memcmp( cheader, "tricked you DXR format is same as DWR except for this line", 29 ) != 0 );
}

bool DoodleDocumentIoHandlerIn::Load( a2dDocumentInputStream& stream, wxObject* doc )
{
    DrawingDocument* drawdoc = ( DrawingDocument* ) doc;

#if wxUSE_STD_IOSTREAM
    a2dDocumentInputStream& streamw = stream;
#else
    wxTextInputStream streamw( stream );
#endif

    wxInt32 n = 0;
    streamw >> n;

    for ( int i = 0; i < n; i++ )
    {
        DoodleSegment* segment = new DoodleSegment();

        wxInt32 l = 0;
        streamw >> l;
#if wxUSE_STD_IOSTREAM
        char c[100];
        streamw.getline( c, 100, ' ' );
        wxString strokeColour( wxString( c, wxConvUTF8 ) );
#else
        wxString strokeColour;
        streamw >> strokeColour;
#endif
        streamw >> segment->m_width;
        segment->m_colour = HexToColour( strokeColour.After( '#' ) );

        for ( int i = 0; i < l; i++ )
        {
            DoodleLine* line = new DoodleLine;
            streamw >> line->x1 >> line->y1 >> line->x2 >>  line->y2;
            segment->m_lines.Append( line );
        }
        drawdoc->GetDoodleSegments().Append( segment );
    }
    return true;
}

DoodleDocumentIoHandlerOut::DoodleDocumentIoHandlerOut()
{
}

DoodleDocumentIoHandlerOut::~DoodleDocumentIoHandlerOut( void )
{
}

bool DoodleDocumentIoHandlerOut::Save( a2dDocumentOutputStream& stream, const wxObject* doc )
{
    DrawingDocument* drawdoc = ( DrawingDocument* ) doc;

#if wxUSE_STD_IOSTREAM
    a2dDocumentOutputStream& streamw = stream;
#else
    wxTextOutputStream streamw( stream );
#endif

    wxInt32 n = drawdoc->GetDoodleSegments().GetCount();
    streamw << n << _T( '\n' );

    wxList::compatibility_iterator segmentNode =
        drawdoc->GetDoodleSegments().GetFirst();
    while ( segmentNode )
    {
        DoodleSegment* segment = ( DoodleSegment* )segmentNode->GetData();

        wxInt32 n = segment->m_lines.GetCount();
        streamw
                << n
                << _T( '\n' )
                << _T( "#" ) << ColourToHex( segment->m_colour )
                << _T( " " ) << segment->m_width
                << _T( "\n" );

        wxList::compatibility_iterator linesNode = segment->m_lines.GetFirst();
        while ( linesNode )
        {
            DoodleLine* line = ( DoodleLine* )linesNode->GetData();

            streamw
                    << line->x1
                    << _T( " " ) << line->y1
                    << _T( " " ) << line->x2
                    << _T( " " ) << line->y2
                    << _T( "\n" );

            linesNode = linesNode->GetNext();
        }

        streamw << '\n';

        segmentNode = segmentNode->GetNext();
    }

    return true;
}

DoodleDocumentIoHandlerDxrIn::DoodleDocumentIoHandlerDxrIn()
{
}

DoodleDocumentIoHandlerDxrIn::~DoodleDocumentIoHandlerDxrIn( void )
{
}

bool DoodleDocumentIoHandlerDxrIn::CanLoad( a2dDocumentInputStream& stream, const wxObject* obj, wxClassInfo* docClassInfo  )
{
#if wxUSE_STD_IOSTREAM
    stream.seekg( 0 );
    char cheader[30];
    cheader[29] = 0;

    stream.read( cheader, 29 );
    stream.seekg( 0 );

#else
    stream.SeekI( 0 );
    char cheader[30];
    cheader[29] = 0;

    stream.Read( cheader, 29 );
    stream.SeekI( 0 );

#endif

    return ( memcmp( cheader, "tricked you DXR format is same as DWR except for this line", 29 ) == 0 );
}

bool DoodleDocumentIoHandlerDxrIn::Load( a2dDocumentInputStream& stream, wxObject* doc )
{
    DrawingDocument* drawdoc = ( DrawingDocument* ) doc;

#if wxUSE_STD_IOSTREAM
    a2dDocumentInputStream& streamw = stream;
#else
    wxTextInputStream streamw( stream );
#endif

    wxString s;
#if wxUSE_STD_IOSTREAM
    char str[250];
    streamw.getline( str, 250 );
    s = wxString( str, wxConvUTF8 );
#else
    s = streamw.ReadLine();
#endif

    wxInt32 n = 0;
    streamw >> n;

    for ( int i = 0; i < n; i++ )
    {
        DoodleSegment* segment = new DoodleSegment();

        wxInt32 l = 0;
        streamw >> l;
#if wxUSE_STD_IOSTREAM
        char c[100];
        streamw.getline( c, 100, ' ' );
        wxString strokeColour( wxString( c, wxConvUTF8 ) );
#else
        wxString strokeColour;
        streamw >> strokeColour;
#endif
        streamw >> segment->m_width;
        segment->m_colour = HexToColour( strokeColour.After( '#' ) );

        for ( int i = 0; i < l; i++ )
        {
            DoodleLine* line = new DoodleLine;
            streamw >> line->x1 >> line->y1 >>  line->x2 >>  line->y2;
            segment->m_lines.Append( line );
        }
        drawdoc->GetDoodleSegments().Append( segment );
    }
    return true;
}


DoodleDocumentIoHandlerDxrOut::DoodleDocumentIoHandlerDxrOut()
{
}

DoodleDocumentIoHandlerDxrOut::~DoodleDocumentIoHandlerDxrOut( void )
{
}

bool DoodleDocumentIoHandlerDxrOut::Save( a2dDocumentOutputStream& stream, const wxObject* doc )
{
    DrawingDocument* drawdoc = ( DrawingDocument* ) doc;

#if wxUSE_STD_IOSTREAM
    a2dDocumentOutputStream& streamw = stream;
#else
    wxTextOutputStream streamw( stream );
#endif
    streamw << _T( "tricked you DXR format is same as DWR except for this line" ) << _T( '\n' );

    wxInt32 n = drawdoc->GetDoodleSegments().GetCount();
    streamw << n << _T( '\n' );

    wxList::compatibility_iterator iSegment =
        drawdoc->GetDoodleSegments().GetFirst();
    while ( iSegment )
    {
        DoodleSegment* segment = ( DoodleSegment* )iSegment->GetData();

        wxInt32 n = segment->m_lines.GetCount();
        streamw
                << n
                << _T( '\n' )
                << _T( "#" ) << ColourToHex( segment->m_colour )
                << _T( " " ) << segment->m_width
                << _T( "\n" );

        wxList::compatibility_iterator iLine = segment->m_lines.GetFirst();
        while ( iLine )
        {
            DoodleLine* line = ( DoodleLine* )iLine->GetData();

            streamw
                    << line->x1
                    << _T( " " ) << line->y1
                    << _T( " " ) << line->x2
                    << _T( " " ) << line->y2
                    << _T( "\n" );

            iLine = iLine->GetNext();
        }

        streamw << '\n';

        iSegment = iSegment->GetNext();
    }

    return true;
}
