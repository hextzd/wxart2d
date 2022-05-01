/*! \file editor/samples/viewmul/doc.cpp
    \brief Implements drawing document functionality
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: doc.cpp,v 1.1 2007/01/17 21:35:20 titato Exp $
*/


#include "a2dprec.h"

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


IMPLEMENT_DYNAMIC_CLASS( DrawingDocument, a2dCanvasDocument )

DrawingDocument::DrawingDocument( void )
{
    m_documentTypeName = wxT( "DrawingDocument" );
}

DrawingDocument::~DrawingDocument( void )
{
    DeleteContents();
}


a2dDocumentOutputStream& DrawingDocument::SaveObject( a2dDocumentOutputStream& stream, a2dIOHandlerStrOut* handler )
{
    wxInt32 n = GetDrawing()->GetRootObject()->GetChildObjectList()->size();

#if wxUSE_STD_IOSTREAM
    a2dDocumentOutputStream& streamw = stream;
#else
    wxTextOutputStream streamw( stream );
#endif

    streamw << n << wxT( '\n' );

    forEachIn( a2dCanvasObjectList, GetDrawing()->GetRootObject()->GetChildObjectList() )
    {
        a2dRect* rect = wxDynamicCast( ( *iter ).Get(), a2dRect );
//      streamw << "#" << ColourToHex(m_colour) << _T(" ") << m_width << _T("\n");

        streamw << rect->GetPosX() << wxT( " " ) << rect->GetPosY() << wxT( " " ) <<
                rect->GetWidth() << wxT( " " ) << rect->GetHeight() << wxT( "\n" );
    }

    return stream;
}


void DrawingDocument::SaveObject( wxString& stringstream, a2dIOHandlerStrOut* handler )
{
    wxInt32 n = GetDrawing()->GetRootObject()->GetChildObjectList()->size();
    stringstream << n << wxT( '\n' );
    forEachIn( a2dCanvasObjectList, GetDrawing()->GetRootObject()->GetChildObjectList() )
    {
        a2dRect* rect = wxDynamicCast( ( *iter ).Get(), a2dRect );
//      stringstream << "#" << ColourToHex(m_colour) << _T(" ") << m_width << _T("\n");

        stringstream << rect->GetPosX() << wxT( " " ) << rect->GetPosY() << wxT( " " ) <<
                     rect->GetWidth() << wxT( " " ) << rect->GetHeight() << wxT( "\n" );

    }
}

a2dDocumentInputStream& DrawingDocument::LoadObject( a2dDocumentInputStream& stream, a2dIOHandlerStrIn* handler )
{

    wxInt32 n = 0;

#if wxUSE_STD_IOSTREAM
    a2dDocumentInputStream& streamw = stream;
#else
    wxTextInputStream streamw( stream );
#endif

    streamw >> n;

    for ( int i = 0; i < n; i++ )
    {
//      wxString strokeColour;
//      text_stream >> strokeColour >> m_width;
//      m_colour = HexToColour( strokeColour.After('#') );

        double x, y, width, height;
        streamw >> x >> y >> width >> height;

        a2dRect* tr = new a2dRect( x, y, width, height );
        tr->SetStroke( wxColour( 1, 3, 205 ), 30.0 ) ;
        tr->SetFill( wxColour( 219, 21, 6 ), a2dFILL_HORIZONTAL_HATCH  );
        GetDrawing()->GetRootObject()->Append( tr );
    }

    //set this root for all object stored in theis root.

     GetDrawing()->SetRootRecursive();

    return stream;
}




























