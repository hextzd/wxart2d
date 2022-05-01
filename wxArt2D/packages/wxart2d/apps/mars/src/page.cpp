/*! \file apps/mars/page.cpp
    \author Erik van der Wal

    Copyright: 2004 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: page.cpp,v 1.13 2008/07/19 18:29:41 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/canvas/canmod.h>
#include <wx/canvas/edit.h>
#include "elements.h"
#include <wx/grid.h>
#include "page.h"
#include "part.h"
#include "document.h"
#include "wx/editor/xmlparsdoc.h"

//----------------------------------------------------------------------------
// msPage
//----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS( msPage, msObject )

msPage::msPage()
    : msObject()
{
}

msPage::msPage( const wxString& pagename )
    : msObject()
{
    SetName( pagename );
}

msPage::msPage( const msPage& other, CloneOptions options, a2dRefMap* refs  )
    : msObject( other, options, refs )
{
}

a2dObject* msPage::DoClone( CloneOptions options, a2dRefMap* refs  ) const
{
    return NULL;
}

wxWindow* msPage::CreateView( wxWindow* parent, a2dDocumentFrame* statusFrame, msDocument* doc )
{
    return NULL;

}

#if wxART2D_USE_CVGIO
void msPage::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    msObject::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void msPage::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    msObject::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

//----------------------------------------------------------------------------
// msSymbolPage
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( msSymbolPage, msPage )

INITIALIZE_PROPERTIES( msSymbolPage, msPage )
{
/*
    PROPID_r = new a2dPropertyIdBool( wxT( "R" ), a2dPropertyId::flag_none, false  );
    AddPropertyId( PROPID_r );

    PROPID_c = new a2dPropertyIdBool( wxT( "C" ), a2dPropertyId::flag_none, false  );
    AddPropertyId( Capacitor::PROPID_c );


    PROPID_refdes = new a2dPropertyIdString( wxT( "REFDES" ), a2dPropertyId::flag_none, wxT( "CMP1" ) );
    AddPropertyId( PROPID_refdes );
    PROPID_spice = new a2dPropertyIdString( wxT( "SPICE" ), a2dPropertyId::flag_none, wxT( "" ) );
    AddPropertyId( PROPID_spice );
    PROPID_freeda = new a2dPropertyIdString( wxT( "FREEDA" ), a2dPropertyId::flag_none, wxT( "" ) );
    AddPropertyId( PROPID_freeda );
*/
    return true;
}

msSymbolPage::msSymbolPage()
    : msPage( wxT( "symbol" ) )
{
}

msSymbolPage::msSymbolPage( const msSymbolPage& other, CloneOptions options, a2dRefMap* refs  )
    : msPage( other, options, refs )
{
}

a2dObject* msSymbolPage::DoClone( CloneOptions options, a2dRefMap* refs  ) const
{
    return new msSymbolPage( *this, options, refs );
};

#if wxART2D_USE_CVGIO
void msSymbolPage::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    msPage::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void msSymbolPage::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    msPage::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

void msSymbolPage::HidePins()
{
    for( a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin(); iter != GetChildObjectList()->end(); ++iter )
    {
        a2dPin* pin = wxDynamicCast( ( *iter ).Get(), a2dPin );
        if ( pin )
            pin->SetVisible( false );
    }
}

void msSymbolPage::HideVisibleProperties()
{
    for( a2dCanvasObjectList::iterator iter = GetChildObjectList()->begin(); iter != GetChildObjectList()->end(); ++iter )
    {
        a2dText* text = wxDynamicCast( ( *iter ).Get(), a2dText );
        if ( text && text->GetText().GetChar( 0 ) == wxChar( '@' ) )
            text->SetVisible( false );
    }
}

wxWindow* msSymbolPage::CreateView( wxWindow* parent, a2dDocumentFrame* statusFrame, msDocument* doc )
{
    wxASSERT( doc );

    a2dCanvasView* view = new a2dCanvasView();
    view->SetDocument( doc );

    //! \todo EW: wxDefaultPosition/Size does not seem to work?
    a2dViewCanvas* canvas = new a2dViewCanvas( view, parent, -1, wxPoint( 0, 0 ), wxSize( 200, 200 ) );
    canvas->SetShowObject( this );

    a2dLayers* docLayers = doc->GetLayerSetup();
    docLayers->SetStroke( 0, a2dStroke( *wxBLACK, 0.5 ) );
    docLayers->SetFill( 0, a2dFill( wxColour( 0xff, 0xff, 0xc0 ) ) );

    canvas->SetGrid( true );
    canvas->SetGridX( 2.0 );
    canvas->SetGridY( 2.0 );
    canvas->SetGridLines( true );
    canvas->SetGridStroke( a2dStroke( wxColour( 0xf0, 0xf0, 0xf0 ), 1 ) );
    canvas->SetMappingUpp( -20, -20, .2, .2 );
    canvas->SetYaxis( true );

	a2dStToolContr* contr  = new a2dStToolContr( canvas->GetDrawingPart(), statusFrame );
    contr->SetZoomFirst( true );
    a2dDrawCircleTool* draw = new a2dDrawCircleTool( contr );
    draw->SetEditAtEnd( true );
    contr->PushTool( draw );
    doc->GetDrawing()->GetCommandProcessor()->Initialize();
    return canvas;
}

//----------------------------------------------------------------------------
// msScionPage
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( msScionPage, msPage )

msScionPage::msScionPage()
    : msPage( wxT( "scion" ) )
{
}

msScionPage::msScionPage( const msScionPage& other, CloneOptions options, a2dRefMap* refs  )
    : msPage( other, options, refs )
{
}

a2dObject* msScionPage::DoClone( CloneOptions options, a2dRefMap* refs  ) const
{
    return new msScionPage( *this, options, refs );
};

#if wxART2D_USE_CVGIO
void msScionPage::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    msPage::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void msScionPage::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    msPage::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

wxWindow* msScionPage::CreateView( wxWindow* parent, a2dDocumentFrame* statusFrame, msDocument* doc )
{
    wxASSERT( doc );

    msPropView* view = new msPropView();
    view->SetDocument( doc );

    msPropViewWindow* viewwin = new msPropViewWindow( view, parent );
    return viewwin;
}

//----------------------------------------------------------------------------
// msCircuitPage
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( msCircuitPage, msPage )

msCircuitPage::msCircuitPage()
    : msPage( wxT( "circuit" ) )
{
}

msCircuitPage::msCircuitPage( const msCircuitPage& other, CloneOptions options, a2dRefMap* refs  )
    : msPage( other, options, refs )
{
}

a2dObject* msCircuitPage::DoClone( CloneOptions options, a2dRefMap* refs  ) const
{
    return new msCircuitPage( *this, options, refs );
};

#if wxART2D_USE_CVGIO
void msCircuitPage::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    msPage::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void msCircuitPage::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    msPage::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

wxWindow* msCircuitPage::CreateView( wxWindow* parent, a2dDocumentFrame* statusFrame, msDocument* doc )
{
    wxASSERT( doc );

    a2dCanvasView* view = new a2dCanvasView();
    view->SetDocument( doc );

    //! \todo EW: wxDefaultPosition/Size does not seem to work?
    a2dViewCanvas* canvas = new a2dViewCanvas( view, parent, -1, wxPoint( 0, 0 ), wxSize( 200, 200 ) );
    canvas->SetShowObject( this );

    /*
        a2dRestrictionEngine *restrict=new a2dRestrictionEngine();
        restrict->SetSnapGrid( 2, 2 );
        restrict->SetPosModes(a2dRestrictionEngine::pos3P|a2dRestrictionEngine::posSngl|a2dRestrictionEngine::posOther);
        restrict->SetSizeModes(a2dRestrictionEngine::sizeLength|a2dRestrictionEngine::sizeMin);
        restrict->SetRotationModes(a2dRestrictionEngine::rotAllAngle);
        restrict->SetRotationAngle(90);
        a2dCanvasGlobals->SetRestrictionEngine(restrict);
    */
    canvas->SetGrid( true );
    canvas->SetGridX( 2.0 );
    canvas->SetGridY( 2.0 );
    canvas->SetGridLines( true );
    canvas->SetGridStroke( a2dStroke( wxColour( 0xf0, 0xf0, 0xf0 ), 1 ) );
    canvas->SetMappingUpp( 80, 10, .25, .25 );
    canvas->SetYaxis( true );

	a2dStToolContr* contr  = new a2dStToolContr( canvas->GetDrawingPart(), statusFrame );
    contr->SetZoomFirst( true );
    a2dDrawRectangleTool* draw = new a2dDrawRectangleTool( contr );
    draw->SetEditAtEnd( true );
    contr->PushTool( draw );
    doc->GetDrawing()->GetCommandProcessor()->Initialize();
    return canvas;
}

//----------------------------------------------------------------------------
// msLocallibPage
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( msLocallibPage, msPage )

msLocallibPage::msLocallibPage()
    : msPage( wxT( "locallib (debug)" ) )
{
}

msLocallibPage::msLocallibPage( const msLocallibPage& other, CloneOptions options, a2dRefMap* refs  )
    : msPage( other, options, refs )
{
}

a2dObject* msLocallibPage::DoClone( CloneOptions options, a2dRefMap* refs  ) const
{
    return new msLocallibPage( *this, options, refs );
};

#if wxART2D_USE_CVGIO
void msLocallibPage::DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite )
{
    msPage::DoSave( parent, out, xmlparts, towrite );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}

void msLocallibPage::DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts )
{
    msPage::DoLoad( parent, parser, xmlparts );
    if ( xmlparts == a2dXmlSer_attrib )
    {
    }
    else
    {
    }
}
#endif //wxART2D_USE_CVGIO

wxWindow* msLocallibPage::CreateView( wxWindow* parent, a2dDocumentFrame* statusFrame, msDocument* doc )
{
    wxASSERT( doc );

    a2dCanvasView* view = new a2dCanvasView();
    view->SetDocument( doc );

    //! \todo EW: wxDefaultPosition/Size does not seem to work?
    a2dViewCanvas* canvas = new a2dViewCanvas( view, parent, -1, wxPoint( 0, 0 ), wxSize( 200, 200 ) );
    canvas->SetShowObject( this );

    canvas->SetGrid( true );
    canvas->SetGridX( 2.0 );
    canvas->SetGridY( 2.0 );
    canvas->SetGridLines( true );
    canvas->SetGridStroke( a2dStroke( wxColour( 0xf0, 0xf0, 0xf0 ), 1 ) );
    canvas->SetMappingUpp( 80, 10, .25, .25 );
    canvas->SetYaxis( true );

	a2dStToolContr* contr  = new a2dStToolContr( canvas->GetDrawingPart(), statusFrame );
    contr->SetZoomFirst( true );
    a2dDrawRectangleTool* draw = new a2dDrawRectangleTool( contr );
    draw->SetEditAtEnd( true );
    contr->PushTool( draw );
    doc->GetDrawing()->GetCommandProcessor()->Initialize();
    return canvas;
}

msPart* msLocallibPage::GetPart( wxString name )
{
    msPart* part = ( msPart* ) Find( name, wxT( "msPart" ) );
    if ( !part )
    {
        if ( ::wxFileExists( name + wxT( ".mrs" ) ) )
        {
            msDocument doc;
            a2dIOHandlerDocCVGIn* iohandler = new a2dIOHandlerDocCVGIn;
            doc.Load( name + wxT( ".mrs" ),  iohandler );
            doc.SetFilename( wxFileName( name + wxT( ".mrs" ) ) );
            part = new msPart( &doc );
            Append( part );
            delete iohandler;
        }
    }
    return part;
}






//----------------------------------------------------------------------------
// msPropView
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( msPropView, a2dView )

BEGIN_EVENT_TABLE( msPropView, a2dView )
    EVT_UPDATE_VIEWS( msPropView::OnUpdate )
END_EVENT_TABLE()

void msPropView::OnUpdate( a2dDocumentEvent& event )
{
    if ( GetDocument() != event.GetEventObject() )
        return;

    if ( m_display )
    {
        wxGrid* grid = ( ( msPropViewWindow* ) GetDisplayWindow() )->GetGrid();
        msDocument* doc = ( msDocument* ) GetDocument();
        msSymbolPage* page = doc->GetSymbolPage();
        a2dNamedPropertyList::iterator iter;
        int r = 0;
        for( iter = m_propertylist.begin(); iter != m_propertylist.end(); ++iter )
        {
            a2dNamedProperty* prop = *iter;
            if ( wxDynamicCast( prop, msProperty ) )
            {
                grid->SetCellValue( r, 0, prop->GetName() );
                grid->SetCellValue( r++, 1, prop->StringValueRepresentation() );
            }
        }
    }
}

//----------------------------------------------------------------------------
// msPropViewWindow
//----------------------------------------------------------------------------

IMPLEMENT_CLASS( msPropViewWindow, a2dDocumentViewWindow )

BEGIN_EVENT_TABLE( msPropViewWindow, a2dDocumentViewWindow )
    EVT_SIZE( msPropViewWindow::OnSize )
END_EVENT_TABLE()

msPropViewWindow::msPropViewWindow( a2dView* view, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, const long style, const wxString& name )
    : a2dDocumentViewWindow( view, parent, id, pos, size, style, name )
{
    //! \todo EW: Make grid size with parent
    m_grid = new wxGrid( this, -1 );
    m_grid->CreateGrid( 20, 2 );
    m_grid->SetSize( 400, 400 );
    m_grid->SetCellValue( 0, 0, wxT( "test" ) );
    m_grid->SetColLabelValue( 0, wxT( "Name" ) );
    m_grid->SetColLabelValue( 1, wxT( "Default value" ) );
    m_grid->SetRowLabelSize( 0 );
    m_grid->SetColLabelSize( m_grid->GetRowSize( 0 ) );
    m_grid->SetColSize( 1, 200 );
    wxBoxSizer* sizerPanel = new wxBoxSizer( wxVERTICAL );
    sizerPanel->Add( m_grid, 1, wxEXPAND );
    SetSizer( sizerPanel );
}

void msPropViewWindow::OnSize( wxSizeEvent& event )
{
    if ( GetAutoLayout() )
        Layout();
    event.Skip();
}
