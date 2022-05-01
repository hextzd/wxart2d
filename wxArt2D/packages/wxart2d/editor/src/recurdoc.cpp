/*! \file canvas/src/recur.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: recur.cpp,v 1.107 2009/09/26 20:40:32 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/editor/recurdoc.h"
#include "wx/canvas/cameleon.h"
#include "wx/canvas/objlist.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/drawer.h"
#include "wx/docview/doccom.h"

#include <wx/wfstream.h>
#include <math.h>

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasDocumentReference, a2dLibraryReference )
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasDocumentReference2, a2dCanvasObject )
IMPLEMENT_DYNAMIC_CLASS( a2dCanvasDocumentReferenceAsText, a2dText )

//----------------------------------------------------------------------------
// a2dCanvasDocumentReference
//----------------------------------------------------------------------------

a2dCanvasDocumentReference::a2dCanvasDocumentReference( double x, double y, a2dCanvasDocument* doc, double totalheight,
        const a2dFont& font, int imagew, int imageh, a2dCanvasObject* symbol )
    : a2dLibraryReference( x, y, doc->GetDrawing()->GetRootObject() ,
                           doc->GetFilename().GetFullName() != wxT( "" ) ? doc->GetFilename().GetFullName() : doc->GetPrintableName(),
                           totalheight, symbol, font, imagew, imageh )
{

    SetObjectTip( doc->GetFilename().GetFullPath(), m_height / 2 , 0, m_height / 5 );

    m_doc = doc;
}

a2dCanvasDocumentReference::a2dCanvasDocumentReference( double x, double y, double totalheight, const wxFileName& filename,
        const a2dFont& font, int imagew, int imageh, a2dCanvasObject* symbol )
    : a2dLibraryReference( x, y, NULL, filename.GetFullName(), totalheight, symbol, font, imagew, imageh )
{

    SetObjectTip( filename.GetFullPath(), m_height / 2 , 0, m_height / 5 );

    m_doc = NULL;
}

a2dCanvasDocumentReference::a2dCanvasDocumentReference( const a2dCanvasDocumentReference& other, CloneOptions options, a2dRefMap* refs )
    : a2dLibraryReference( other, options, refs )
{
    m_doc = other.m_doc;
}

a2dCanvasDocumentReference::~a2dCanvasDocumentReference()
{
}

a2dObject* a2dCanvasDocumentReference::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCanvasDocumentReference( *this, options, refs );
}

void a2dCanvasDocumentReference::DependencyPending( a2dWalkerIOHandler* WXUNUSED( handler ) )
{
    if ( !m_flags.m_pending && m_doc->GetDrawing()->GetRootObject()->GetPending() )
        SetPending( true );
}

//----------------------------------------------------------------------------
// a2dCanvasDocumentReference2
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dCanvasDocumentReference2, a2dCanvasObject )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dCanvasDocumentReference2::OnCanvasObjectMouseEvent )
END_EVENT_TABLE()

a2dCanvasDocumentReference2::a2dCanvasDocumentReference2( double x, double y, a2dCanvasDocument* doc )
    : a2dCanvasObject( x, y )
{
    if ( doc )
    {
        SetObjectTip( doc->GetFilename().GetFullPath(), 100 , 0, 100 );
        m_fileName = wxFileName( doc->GetFilename().GetFullPath() );
    }
    SetURI( m_fileName.GetFullPath() );
    m_doc = doc;
}

a2dCanvasDocumentReference2::a2dCanvasDocumentReference2( double x, double y, const wxFileName& filename )
    : a2dCanvasObject( x, y )
{

    SetObjectTip( filename.GetFullPath(), 100 , 0, 100 );
    m_fileName = filename;
    SetURI( m_fileName.GetFullPath() );
    m_doc = NULL;
}

a2dCanvasDocumentReference2::a2dCanvasDocumentReference2( const a2dCanvasDocumentReference2& other, CloneOptions options, a2dRefMap* refs )
    : a2dCanvasObject( other, options, refs )
{
    m_doc = other.m_doc;
}

a2dCanvasDocumentReference2::~a2dCanvasDocumentReference2()
{
}

a2dObject* a2dCanvasDocumentReference2::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCanvasDocumentReference2( *this, options, refs );
}

#if wxART2D_USE_CVGIO
void a2dCanvasDocumentReference2::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dCanvasObject::DoSave( parent, out, xmlparts, towrite );
}

void a2dCanvasDocumentReference2::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dCanvasObject::DoLoad( parent, parser, xmlparts );
}
#endif //wxART2D_USE_CVGIO

bool a2dCanvasDocumentReference2::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    return a2dCanvasObject::DoIsHitWorld( ic, hitEvent );
}

void a2dCanvasDocumentReference2::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    //if ( !m_doc )
    //    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileOpen( ( a2dDocumentPtr& ) m_doc, m_fileName );

    //else
    //    event.Skip();
}

//----------------------------------------------------------------------------
// a2dCanvasDocumentReferenceAsText
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE( a2dCanvasDocumentReferenceAsText, a2dText )
    EVT_CANVASOBJECT_MOUSE_EVENT( a2dCanvasDocumentReferenceAsText::OnCanvasObjectMouseEvent )
END_EVENT_TABLE()

a2dCanvasDocumentReferenceAsText::a2dCanvasDocumentReferenceAsText( a2dCanvasDocument* doc, double x, double y, const a2dFont& font, double angle,
             bool up, int alignment )
             : a2dText( wxT("not specified"), x, y, font, angle, up, alignment )
{
    m_doc = doc;
    if ( doc )
    {
        m_fileName = doc->GetFilename().GetFullPath();
    }
    m_displayShort = true;
    m_displayExt = true;

    if ( m_displayShort && m_displayExt )
        m_text = m_fileName.GetName();
    else if ( m_displayShort )
        m_text = m_fileName.GetFullName();
    else
        m_text = m_fileName.GetFullPath();

    wxString pathToFile = wxT( "file:///" ) + m_fileName.GetVolume() + m_fileName.GetVolumeSeparator() + m_fileName.GetPath(wxPATH_GET_SEPARATOR, wxPATH_UNIX);
    SetURI( pathToFile + m_fileName.GetFullName() );
    if ( doc )
        SetObjectTip( pathToFile, 100 , 0, 100 );
}

a2dCanvasDocumentReferenceAsText::a2dCanvasDocumentReferenceAsText( const wxFileName& filename, double x, double y, const a2dFont& font, double angle,
             bool up, int alignment )
    : a2dText( filename.GetFullPath(), x, y, font, angle, up, alignment )
{
    m_fileName = filename;

    if ( m_displayShort && m_displayExt )
        m_text = m_fileName.GetName();
    else if ( m_displayShort )
        m_text = m_fileName.GetFullName();
    else
        m_text = m_fileName.GetFullPath();

    wxString pathToFile = wxT( "file:///" ) + m_fileName.GetVolume() + m_fileName.GetVolumeSeparator() + m_fileName.GetPath(wxPATH_GET_SEPARATOR, wxPATH_UNIX);
    SetURI( pathToFile + m_fileName.GetFullName() );
    SetObjectTip( pathToFile, 100 , 0, 100 );

    m_doc = NULL;
}

a2dCanvasDocumentReferenceAsText::a2dCanvasDocumentReferenceAsText( const a2dCanvasDocumentReferenceAsText& other, CloneOptions options, a2dRefMap* refs )
    : a2dText( other, options, refs )
{
    m_fileName = other.m_fileName;
    m_doc = other.m_doc;
    m_displayShort = other.m_displayShort;
    m_displayExt = other.m_displayExt;
}

a2dCanvasDocumentReferenceAsText::~a2dCanvasDocumentReferenceAsText()
{
}

a2dObject* a2dCanvasDocumentReferenceAsText::DoClone( CloneOptions options, a2dRefMap* refs ) const
{
    return new a2dCanvasDocumentReferenceAsText( *this, options, refs );
}

#if wxART2D_USE_CVGIO
void a2dCanvasDocumentReferenceAsText::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    a2dText::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        out.WriteAttribute( wxT( "filename" ), m_fileName.GetFullPath() );
    }
    else
    {
    }
}

void a2dCanvasDocumentReferenceAsText::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    a2dText::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
        m_fileName = parser.GetAttributeValue( wxT( "filename" ) );
        /* URI is saved as a property, and will give the same as this when read in a2dObject
        wxString pathToFile = wxT( "file:///" ) + m_fileName.GetVolume() + m_fileName.GetVolumeSeparator() + m_fileName.GetPath(wxPATH_GET_SEPARATOR, wxPATH_UNIX);
        SetURI( pathToFile + m_fileName.GetFullName() );
        wxURI uri = GetURI();
        wxString link = uri.BuildURI();
        */
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dCanvasDocumentReferenceAsText::DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent )
{
    return a2dText::DoIsHitWorld( ic, hitEvent );
}

void a2dCanvasDocumentReferenceAsText::OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event )
{
    wxMouseEvent mouseevent = event.GetMouseEvent();

    if( mouseevent.ControlDown() && mouseevent.ShiftDown() && m_doc )
    {
        a2dDocumentPtr doc = m_doc.Get();
        a2dDocviewGlobals->GetDocviewCommandProcessor()->FileOpenCheck( doc, m_fileName, true );
    }
    else
        event.Skip();
}

bool a2dCanvasDocumentReferenceAsText::DoStartEdit( wxUint16 editmode, wxEditStyle editstyle )
{
    if ( m_flags.m_editable )
    {
        a2dText* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dText );
        RemoveProperty( PROPID_Objecttip );
        m_text = m_fileName.GetFullPath();
        return a2dText::DoStartEdit( editmode, editstyle );
    }
    return false;
}

void a2dCanvasDocumentReferenceAsText::DoEndEdit()
{
    a2dText::DoEndEdit();
    a2dCanvasDocumentReferenceAsText* original = wxStaticCast( PROPID_Original->GetPropertyValue( this ).Get(), a2dCanvasDocumentReferenceAsText );
    original->m_fileName = wxFileName( m_text );
    if ( m_displayShort && m_displayExt )
        original->m_text = original->m_fileName.GetName();
    else if ( m_displayShort )
        original->m_text = original->m_fileName.GetFullName();
    else
        original->m_text = original->m_fileName.GetFullPath();

    wxString pathToFile = wxT( "file:///" ) + m_fileName.GetVolume() + m_fileName.GetVolumeSeparator() + m_fileName.GetPath(wxPATH_GET_SEPARATOR, wxPATH_UNIX);
    original->SetURI( pathToFile + m_fileName.GetFullName() );
    original->SetObjectTip( pathToFile, 100 , 0, 100 );

    wxURI uri = original->GetURI();
    wxString link = uri.BuildURI();

}

bool a2dCanvasDocumentReferenceAsText::DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox )
{
    if ( !m_bbox.GetValid() )
    {
    }
    return a2dText::DoUpdate( mode, childbox, clipbox, propbox );
}

