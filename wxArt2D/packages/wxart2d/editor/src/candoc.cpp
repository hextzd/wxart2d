/*! \file canvas/src/candoc.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: candoc.cpp,v 1.190 2009/09/02 19:46:07 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/wfstream.h>
#include <math.h>
#include <wx/tokenzr.h>
#include <wx/metafile.h>
#include <locale.h>


#if wxUSE_STD_IOSTREAM
#include "wx/ioswrap.h"
#if wxUSE_IOSTREAMH
#include <fstream.h>
#else
#include <fstream>
#endif
#else
#include "wx/wfstream.h"
#endif


#include "wx/editor/candoc.h"
#include "wx/editor/cancom.h"

#include "wx/canvas/xmlpars.h"
#include "wx/canvas/drawer.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// a2dViewPrintout
//----------------------------------------------------------------------------

#if wxUSE_PRINTING_ARCHITECTURE

bool a2dViewPrintout::m_printAsBitmap = true;

/* print out for drawer */
a2dViewPrintout::a2dViewPrintout( const wxPageSetupDialogData& pageSetupData, a2dCanvasView* drawingView, const wxString& title, const wxString& filename, a2dPrintWhat typeOfPrint, bool drawframe, double scalelimit, bool fitToPage ) : 
    m_pageSetupData( pageSetupData ),
    a2dDocumentPrintout( drawingView, title )
{
    m_typeOfPrint = typeOfPrint;
    m_drawingView = drawingView;
    m_title = title;
    m_filename = filename;
    m_scalelimit = scalelimit;
    m_drawframe = drawframe;
    m_fitToPage = fitToPage;
}

a2dViewPrintout::~a2dViewPrintout( void )
{
}

bool a2dViewPrintout::OnPrintPage( int )
{

    wxDC* dc;
    dc = GetDC();

    // We get the paper size in device units and the margins in mm,
    // so we need to calculate the conversion with this trick
    wxCoord pw, ph;
    dc->GetSize( &pw, &ph );
    wxCoord mw, mh;
    GetPageSizeMM(&mw, &mh);
    float mmToDeviceX = float(pw) / mw;
    float mmToDeviceY = float(ph) / mh;

    // paper size in device units
    wxRect paperRect = wxRect( 0,0,pw,ph);

    // margins in mm
    wxPoint topLeft = m_pageSetupData.GetMarginTopLeft();
    wxPoint bottomRight = m_pageSetupData.GetMarginBottomRight();

    // calculate margins in device units
    wxRect pageMarginsRect(
        paperRect.x      + wxRound(mmToDeviceX * topLeft.x),
        paperRect.y      + wxRound(mmToDeviceY * topLeft.y),
        paperRect.width  - wxRound(mmToDeviceX * (topLeft.x + bottomRight.x)),
        paperRect.height - wxRound(mmToDeviceY * (topLeft.y + bottomRight.y)));

    //! needed ?
    dc->SetBrush( *wxWHITE_BRUSH );
    dc->SetBackground( *wxWHITE_BRUSH );
    dc->Clear();
    dc->SetBackgroundMode( wxTRANSPARENT );
    dc->SetTextForeground( *wxBLACK );
    dc->SetBrush( *wxTRANSPARENT_BRUSH );


	int marginX = ( int )( ph * 0.015 );
    int marginY = marginX;
    int fontMarginYTop = 0;
    int fontMarginYBottom = 0;
    wxFont fontTitle = wxFont( marginY, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    wxFont fontFilename = wxFont( int( marginY / 2. + 0.5 ), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );

    if ( !m_title.IsEmpty() )
    {
        dc->SetFont( fontTitle );
        fontMarginYTop += 2 * fontTitle.GetPointSize();
        wxCoord xExtent, yExtent;
        dc->GetTextExtent( m_title, &xExtent, &yExtent );
        dc->DrawText( m_title, pw / 2 - xExtent / 2, marginY );
    }

    if ( !m_filename.IsEmpty() )
    {
        dc->SetFont( fontFilename );
        fontMarginYBottom += 2 * fontFilename.GetPointSize();
        dc->DrawText( m_filename, marginX, ph - marginY - fontMarginYBottom );
    }

 
    int widthX = pageMarginsRect.GetWidth();
    int widthY = pageMarginsRect.GetHeight();
    int orgX = pageMarginsRect.GetTopLeft().x;
    int orgY = pageMarginsRect.GetTopLeft().y;

    dc->SetBackgroundMode( wxSOLID );
    dc->SetFont( wxNullFont );

/* todo
    double scaleX = ( widthX / ( double ) pw );
    double scaleY = ( widthY / ( double ) ph );
//  double scaleX = (w/widthX);
//  double scaleY = (h/widthY);

    if( !m_fitToPage )
    {
        double actualScale = wxMax( scaleX, scaleY );

        if( actualScale < m_scalelimit )
            actualScale = m_scalelimit;
        scaleX = scaleY = actualScale;
    }

    //if scale in X and Y did change, adjust incoming Dc to draw on that exact area.
    // Calculate the position on the DC for centering the graphic
    orgX = orgX + Round( ( pw - ( widthX / scaleX ) ) / 2.0 );
    orgY = orgY + fontMarginYTop + Round( ( ph - ( widthY / scaleY ) ) / 2.0 );
*/

    wxDC* memprint = wxDynamicCast( dc, wxMemoryDC );

    a2dDcDrawer* dcdrawer = NULL;
    if (!memprint && !m_printAsBitmap)
        dcdrawer = new a2dDcDrawer( widthX, widthY );
    else
        dcdrawer = new a2dMemDcDrawer( widthX, widthY );

    dcdrawer->SetPrintMode( true );
    dcdrawer->SetRealScale( !m_fitToPage );
    //dcdrawer->SetSmallTextThreshold( 10000 );

    // initialize with the view to print, but scaling  will be changed later.
    a2dSmrtPtr<a2dDrawingPart> drawPart = new a2dDrawingPart( *m_drawingView->GetDrawingPart() );

    //drawView is a a2dView and therefore gets events, but this is not wanted here.
    drawPart->SetEvtHandlerEnabled( false );
    drawPart->SetDrawer2D( dcdrawer );
    drawPart->SetShowOrigin( false );
    drawPart->SetGrid( false );

    // take some important settings from the view where this a2dViewPrintout was created:
    // see a2dCanvasView::OnCreatePrintout()
    a2dCanvasDocument* doc = wxStaticCastNull( m_drawingView->GetDocument(), a2dCanvasDocument );
    drawPart->SetShowObjectAndRender( m_drawingView->GetDrawingPart()->GetShowObject() );
    drawPart->SetDocumentDrawStyle( m_drawingView->GetDrawingPart()->GetDocumentDrawStyle() );
    drawPart->SetAvailable();
	a2dDrawer2D* drawcont =  m_drawingView->GetDrawingPart()->GetDrawer2D();
    dcdrawer->SetYaxis( drawcont->GetYaxis() );
	
    if ( memprint || (!memprint && !m_printAsBitmap) )
    {
        //is wxPrinterDc, and we draw on it;

        if ( m_typeOfPrint == a2dPRINT_Print )
        {
            dcdrawer->SetMappingWidthHeight( drawcont->GetVisibleMinX(),
                                             drawcont->GetVisibleMinY(),
                                             drawcont->GetVisibleWidth(),
                                             drawcont->GetVisibleHeight() );
        }
        else if ( m_typeOfPrint == a2dPRINT_PrintView ||
                  m_typeOfPrint == a2dPRINT_PreviewView )
        {
            dcdrawer->SetMappingWidthHeight( drawcont->GetVisibleMinX(),
                                             drawcont->GetVisibleMinY(),
                                             drawcont->GetVisibleWidth(),
                                             drawcont->GetVisibleHeight() );
        }
        else //default do all of document
        {
            drawPart->SetMappingShowAll();
        }

        //now start drawing the document on our created view
		// Set the scale and origin
		//  dc->SetUserScale(scaleX, scaleY);
        dc->SetDeviceOrigin( orgX, orgY );
        dcdrawer->SetRenderDC( dc ); //a trick to set the drawers internal dc.
        drawPart->UpdateArea( 0, 0, widthX, widthY );
        dcdrawer->SetRenderDC( NULL );
    }
    else if ( !memprint && m_printAsBitmap )
    {
        if ( m_typeOfPrint == a2dPRINT_Print )
        {
            dcdrawer->SetMappingWidthHeight( drawcont->GetVisibleMinX(),
                                                drawcont->GetVisibleMinY(),
                                                drawcont->GetVisibleWidth(),
                                                drawcont->GetVisibleHeight() );
        }
        else if ( m_typeOfPrint == a2dPRINT_PrintView ||
                    m_typeOfPrint == a2dPRINT_PreviewView )
        {
            dcdrawer->SetMappingWidthHeight( drawcont->GetVisibleMinX(),
                                                drawcont->GetVisibleMinY(),
                                                drawcont->GetVisibleWidth(),
                                                drawcont->GetVisibleHeight() );
        }
        else //default do all of document
        {
            drawPart->SetMappingShowAll();
        }

        //now start drawing the document on our created view
        drawPart->UpdateArea( 0, 0, widthX, widthY );

		wxBitmap buffer(dcdrawer->GetBuffer());
		drawPart = NULL; // Delete the 2nd reference to the bitmap. Otherwise dcb.SelectObject( buffer ); causes an exception

        //buffer.SaveFile( "c:/soft/aap.png", wxBITMAP_TYPE_PNG );
        wxMemoryDC dcb;
        dcb.SelectObject( buffer );
        dc->Blit( orgX, orgY, widthX, widthY, &dcb, 0, 0, wxCOPY, false );
        dcb.SelectObject( wxNullBitmap );
/* if above does not work, this will!
        wxMemoryDC dcb;
        dcb.SelectObject( buffer );
        int stripHeight = 1000;
        int i, strips = widthY / stripHeight;
        int rest = widthY - strips * stripHeight;
        for ( i = 0; i < strips; i++ )
        {
            dc->Blit( 0 , stripHeight * i, widthX, stripHeight, &dcb, 0, stripHeight*i, wxCOPY, false );
        }    
        dc->Blit( 0 , stripHeight * i, widthX, rest, &dcb, 0, stripHeight*i , wxCOPY, false );
        dcb.SelectObject( wxNullBitmap );
*/
        //dc->DrawBitmap( buffer , 0,0 );
    }

    if( m_drawframe )
    {
	    dc->SetDeviceOrigin(  0, 0 );
	    dc->SetBrush( *wxTRANSPARENT_BRUSH );
        dc->SetPen( *wxRED_PEN );
	    dc->DrawRectangle( orgX, orgY, widthX, widthY );
        const wxPen& aPenForFrame = dc->GetPen();
    }


    /* debug
    dc->SetDeviceOrigin(  0, 0 );
    dc->SetPen( *wxBLACK_PEN );
    dc->SetBrush( *wxTRANSPARENT_BRUSH );
    dc->DrawRectangle( orgX, orgY, widthX, widthY);
    */
    return true;
}
#endif //wxUSE_PRINTING_ARCHITECTURE




//----------------------------------------------------------------------------
// a2dCanvasView
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( a2dCanvasView, a2dView )

BEGIN_EVENT_TABLE( a2dCanvasView, a2dView )
    EVT_CREATE_VIEW( a2dCanvasView::OnCreateView )
END_EVENT_TABLE()

a2dCanvasView::a2dCanvasView( const wxSize& size ): a2dView()
{
    Init();
}

a2dCanvasView::a2dCanvasView( int width, int height ): a2dView()
{
    Init();
}

void a2dCanvasView::Init()
{
    m_printtitle = true;
    m_printfilename = true;
    m_printscalelimit = 0;
    m_printframe = true;
    m_printfittopage = false;
}

a2dCanvasView::~a2dCanvasView()
{
    //prevent event handling from now one ( else problem from event distribution can happen ).
    //Unregister is done in a2dView.
    SetEvtHandlerEnabled( false );

    m_display = NULL; //is used to test inside controllers.
}

a2dCanvasView::a2dCanvasView( const a2dCanvasView& other )
    : a2dView( other )
{
    Init();
}

a2dCanvasDocument* a2dCanvasView::GetCanvasDocument() const
{
    return wxStaticCastNull( GetDocument(), a2dCanvasDocument );
}

void a2dCanvasView::OnCreateView( a2dViewEvent& event )
{
	event.Skip();
    //m_display = new a2dCanvas(); THIS would be one way of doing it in a derived/any view. But more flexible is to create windows/GUI in a a2dConnector Class, where this event goes also.
}

a2dDrawingPart* a2dCanvasView::GetDrawingPart() const
{ 
	if ( a2dCanvas* can = wxDynamicCast( m_display, a2dCanvas ) )
        return can->GetDrawingPart();
    if ( a2dCanvasSim* can = wxDynamicCast( m_display, a2dCanvasSim ) )
        return can->GetDrawingPart();
#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
    else if ( a2dOglCanvas* can = wxDynamicCast( m_display, a2dOglCanvas ) )
        return can->GetDrawingPart();
#endif //wxUSE_GLCANVAS 

    return 0;
}

bool a2dCanvasView::ProcessEvent( wxEvent& event )
{
    bool processed = false;

    // An event handler can be enabled or disabled
    if ( ( GetEvtHandlerEnabled() && m_viewEnabled ) ||
            ( event.GetEventType() == wxEVT_ENABLE_VIEW ) ||
            ( event.GetEventType() == wxEVT_ENABLE_VIEWS )
       )
    {
		// via a2dCanvas	
		if ( m_display )
            processed = m_display->GetEventHandler()->ProcessEvent( event );
        if ( ! processed )
        {
            processed = a2dView::ProcessEvent( event );
        }
        return processed;
    }

    return false;
}


void a2dCanvasView::OnDraw( wxDC* WXUNUSED( dc ) )
{
    wxFAIL_MSG( wxT( "a2dCanvasView::OnDraw is not implemented, see a2dViewPrintout::OnPrintPage(int)" ) );
}

#if wxUSE_PRINTING_ARCHITECTURE
wxPrintout* a2dCanvasView::OnCreatePrintout( a2dPrintWhat typeOfPrint, const wxPageSetupDialogData& pageSetupData )
{
    a2dCanvasDocument* doc = wxStaticCast( GetDocument(), a2dCanvasDocument );
    return new a2dViewPrintout( pageSetupData, this, m_printtitle ? doc->GetDescription() : ( const wxString& )wxEmptyString, m_printfilename ? doc->GetFilename().GetFullPath() : wxString( wxT( "" ) ), typeOfPrint, m_printframe, m_printscalelimit, m_printfittopage );
}
#endif

IMPLEMENT_CLASS( a2dZoomedView, wxWindow )

BEGIN_EVENT_TABLE( a2dZoomedView, wxWindow )
    EVT_UPDATE_VIEWS( a2dZoomedView::OnUpdate )
    EVT_COM_EVENT( a2dZoomedView::OnComEvent )
    EVT_PAINT( a2dZoomedView::OnPaint )
    EVT_SIZE( a2dZoomedView::OnSize )
    EVT_REMOVE_VIEW( a2dZoomedView::OnRemoveView )
END_EVENT_TABLE()

a2dZoomedView::a2dZoomedView( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, a2dCanvasView* canvasview ):
    wxWindow( parent, id, pos, size, style )
{
    m_canvasDocview = canvasview;
}

a2dZoomedView::~a2dZoomedView()
{
}

void a2dZoomedView::OnUpdate( a2dDocumentEvent& event )
{
    a2dCanvasDocument* doc = wxStaticCast( m_canvasDocview->GetDocument(), a2dCanvasDocument );
    if ( !m_canvasDocview || doc != event.GetEventObject() )
        return;

    if ( !GetEvtHandlerEnabled() )
        return;

    if ( !m_canvasDocview || doc )
        return;

    //update totalbbox or set dirty.
}

void a2dZoomedView::OnPaint( wxPaintEvent& WXUNUSED( event ) )
{
    if ( !m_canvasDocview || m_canvasDocview->IsClosed() )
    {
        wxPaintDC dc( this ); //needed to prevent looping
        return;
    }

    wxPaintDC dc( this );

    int dvx, dvy;
    GetClientSize( &dvx, &dvy );
    if ( dvx == 0 ) dvx = 1;
    if ( dvy == 0 ) dvy = 1;

    a2dCanvasDocument* doc = wxStaticCast( m_canvasDocview->GetDocument(), a2dCanvasDocument );
    double w = doc->GetDrawing()->GetRootObject()->GetBboxWidth();
    double h = doc->GetDrawing()->GetRootObject()->GetBboxHeight();
    double xview = m_canvasDocview->GetDrawingPart()->GetDrawer2D()->GetVisibleMinX();
    double yview = m_canvasDocview->GetDrawingPart()->GetDrawer2D()->GetVisibleMinY();
    double wview = m_canvasDocview->GetDrawingPart()->GetDrawer2D()->GetVisibleWidth();
    double hview = m_canvasDocview->GetDrawingPart()->GetDrawer2D()->GetVisibleHeight();

    if ( h * dvx / w < dvy )
    {
        //Do draw two rectangles, one of size document and one of size m_canvasDocview.
        dc.DrawRectangle( 0, 0, dvx, ( int ) ( h * dvx / w ) );
        dc.DrawRectangle( ( int ) ( xview * dvx / w ), ( int ) ( yview * dvx / w ), ( int ) ( wview * dvx / w ), ( int ) ( hview * dvx / w ) );
    }
    else
    {
        //Do draw two rectangles, one of size document and one of size m_canvasDocview.
        dc.DrawRectangle( 0, 0, ( int ) ( w * dvx / h ), dvy );
        dc.DrawRectangle( ( int ) ( xview * dvy / h ), ( int ) ( yview * dvy / h ), ( int ) ( wview * dvy / h ), ( int ) ( hview * dvy / h ) );
    }

}

void a2dZoomedView::OnSize( wxSizeEvent& WXUNUSED( event ) )
{
    Refresh();
}


void a2dZoomedView::OnComEvent( a2dComEvent& event )
{
    if ( event.GetId() == a2dDrawer2D::sig_changedZoom )
    {
        Refresh();
    }
}

void a2dZoomedView::OnRemoveView( a2dDocumentEvent& event )
{
    a2dView* viewremoved = ( a2dView* ) event.GetView();
    if ( viewremoved == m_canvasDocview )
        m_canvasDocview = NULL;
}

//----------------------------------------------------------------------------
// a2dCanvasDocument
//----------------------------------------------------------------------------

const a2dSignal a2dCanvasDocument::sig_layersetupChanged = wxNewId();
const a2dSignal a2dCanvasDocument::sig_changedLayer = wxNewId();

IMPLEMENT_DYNAMIC_CLASS( a2dCanvasDocument, a2dDocument )

BEGIN_EVENT_TABLE( a2dCanvasDocument, a2dDocument )
    EVT_IDLE( a2dCanvasDocument::OnIdle )
    EVT_OPEN_DOCUMENT( a2dCanvasDocument::OnOpenDocument )
    EVT_DO( a2dCanvasDocument::OnDoEvent )
    EVT_UNDO( a2dCanvasDocument::OnUndoEvent )
    EVT_REDO( a2dCanvasDocument::OnRedoEvent )
    EVT_CHANGEDMODIFY_DRAWING( a2dCanvasDocument::OnChangeDrawings )
    EVT_NEW_CAMELEON(  a2dCanvasDocument::OnNewCameleon )
END_EVENT_TABLE()

a2dCanvasDocument::a2dCanvasDocument(): a2dDocument()
{
    m_documentModified = 0;
    if ( a2dCanvasGlobals->GetHabitat()->GetLayerSetup() )
        m_layersetup = ( a2dLayers* ) a2dCanvasGlobals->GetHabitat()->GetLayerSetup()->Clone( clone_deep );
   
    m_drawing = (a2dDrawing*) a2dCanvasGlobals->GetHabitat()->GetDrawingTemplate()->Clone( clone_deep );
    m_drawing->SetName( wxT("top") );
    m_drawing->SetDrawingId( a2dDrawingId_physical() );
    m_drawing->SetParent( this );

    if ( m_layersetup )
    {
        m_layersetup->SetRoot( NULL ); //these layers do not belong to a drawing.
    }
    m_askSaveWhenOwned = false;

    m_units_accuracy = 0.001;
    m_units_scale = 1.0;
    m_normalize_scale = 1.0;

    m_units = wxT( "non" );
    m_units_measure = wxT( "non" );

    m_description = wxT( "a2dCanvasDocument generated by wxArt2D" );
    m_libraryname = wxT( "Not specified" );
    m_version = wxT( "1" );
    m_mayEdit = true;

    m_accesstime = wxDateTime::Now();
    m_changedInternalAccesstime = wxDateTime::Now();

    m_documentTypeName = wxT( "a2dCanvasDocument" );

    m_multiRoot = false;

    m_show = NULL;
    m_start = NULL;
    m_main = NULL;
}

a2dCanvasDocument::a2dCanvasDocument( const a2dCanvasDocument& other )
    : a2dDocument( other )
{
    m_show = NULL;
    m_start = NULL;
    m_main = NULL;
    a2dRefMap refs;
	wxLongLong id = 0;
	wxString resolveKey;
    if ( other.m_show )
    {
	    id = other.m_show->GetId();
	    wxASSERT( id != 0 );
	    resolveKey << id;
        a2dSmrtPtr<class a2dObject>* obj = ( a2dSmrtPtr<class a2dObject>* ) & ( m_show );
	    refs.ResolveOrAdd( obj, resolveKey );
    }
    if ( other.m_start )
    {
	    id = other.m_start->GetId();
	    wxASSERT( id != 0 );
	    resolveKey << id;
        a2dSmrtPtr<class a2dObject>* obj = ( a2dSmrtPtr<class a2dObject>* ) & ( m_start );
	    refs.ResolveOrAdd( obj, resolveKey );
    }
    if ( other.m_main )
    {
        id = other.m_main->GetId();
	    wxASSERT( id != 0 );
	    resolveKey << id;
        a2dSmrtPtr<class a2dObject>* obj = ( a2dSmrtPtr<class a2dObject>* ) & ( m_main );
	    refs.ResolveOrAdd( obj, resolveKey );
    }

    if ( other.m_drawing )
    {
        m_drawing = other.m_drawing->TClone( clone_deep, &refs );
        m_drawing->SetParent( this );
    }
    if ( other.m_layersetup )
    {
        m_layersetup =  ( a2dLayers* ) other.m_layersetup->Clone( clone_deep, &refs );
        m_layersetup->SetRoot( NULL );
    }
    refs.LinkReferences();

    m_units_accuracy = other.m_units_accuracy;
    m_units = other.m_units;
    m_units_scale = other.m_units_scale;
    m_normalize_scale = other.m_normalize_scale;
    m_units_measure = other.m_units_measure;
    m_description = other.m_description;
    m_libraryname = other.m_libraryname;
    m_mayEdit = other.m_mayEdit;
    m_version = other.m_version;
    m_accesstime = other.m_accesstime;
    m_documentTypeName = other.m_documentTypeName;
    m_multiRoot = other.m_multiRoot;

    m_documentModified = 0;

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    //klion: it is because the CurrentSmartPointerOwner can change in Clone ( )
    CurrentSmartPointerOwner = this;
#endif
}

a2dObject* a2dCanvasDocument::DoClone( a2dObject::CloneOptions options, a2dRefMap* refs ) const
{
    a2dCanvasDocument* a = new a2dCanvasDocument( *this );
    return a;
}

void a2dCanvasDocument::CreateCommandProcessor()
{
    m_drawing->CreateCommandProcessor();
    m_commandProcessor = NULL;
}

void a2dCanvasDocument::OnDoEvent( a2dCommandProcessorEvent& event )
{
    if ( event.GetCommand()->Modifies() )
        m_documentModified++;
    event.Skip();
}

void a2dCanvasDocument::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    if ( event.GetCommand()->Modifies() )
        m_documentModified++;
    event.Skip();
}

void a2dCanvasDocument::OnUndoEvent( a2dCommandProcessorEvent& event )
{
    if ( event.GetCommand()->Modifies() )
        m_documentModified--;
    event.Skip();
}

void a2dCanvasDocument::OnChangeDrawings( a2dDrawingEvent& event )
{
    // lower drawing ( of a2dCameleons for instance )
    m_documentModified += event.GetModified();
}

void a2dCanvasDocument::OnNewCameleon( a2dCameleonEvent& event )
{
    a2dCameleon* cam = event.GetCameleon();
    cam->SetCameleonRoot( m_drawing->GetRootObject() );
    cam->AddToRoot();
}

a2dCanvasDocument::~a2dCanvasDocument()
{
    if ( a2dCanvasGlobals->GetActiveDrawingPart() && a2dCanvasGlobals->GetActiveDrawingPart()->GetDrawing() == m_drawing )
        a2dCanvasGlobals->SetActiveDrawingPart( NULL );

    if ( m_drawing )
        m_drawing->DisconnectEventAll( this );
}

//void a2dCanvasDocument::operator delete(void* recordptr)
//{
//  ::delete recordptr;
//}

void a2dCanvasDocument::DeleteContents()
{
    if ( m_drawing )
    {
        // the command processor holds smart pointers to the m_drawing.
        m_drawing->SetCommandProcessor( NULL );
        m_drawing->GetRootObject()->ReleaseChildObjects();
    }
}

void a2dCanvasDocument::Update( a2dCanvasObject::UpdateMode mode )
{
    if ( m_layersetup )
    {
        m_layersetup->SetRoot( m_drawing );
        m_layersetup->SetAllLayersAvailable( true );
        //this one always available because editing handles are on it.
        m_layersetup->AddDefaultLayers();
    }
    if ( m_drawing )
    {
        m_drawing->GetRootObject()->Update( mode );

        //todo what if more drawings
        if ( m_layersetup && m_drawing->GetLayerSetup() )
            m_layersetup->SetAvailable( m_drawing );//->GetLayerSetup() );
    }
}

void a2dCanvasDocument::SetDrawing( a2dDrawing* newRoot )
{
    wxASSERT_MSG( newRoot, wxT( "a2dCanvasDocument its root object can not be NULL" ) );
    m_drawing = newRoot;
}

void a2dCanvasDocument::SetLayerSetup( a2dLayers* layersetup )
{
    m_layersetup = layersetup;
    if ( m_layersetup )
        m_layersetup->SetRoot( m_drawing );

    a2dComEvent changedlayers( this, m_layersetup, sig_layersetupChanged );
    ProcessEvent( changedlayers );
};

void a2dCanvasDocument::OnOpenDocument( a2dDocumentEvent& event )
{
    a2dDocument::OnOpenDocument( event );

    UpdateAllViews( NULL, a2dCANVIEW_UPDATE_ALL );
}

a2dDocumentInputStream& a2dCanvasDocument::LoadObject( a2dDocumentInputStream& stream, a2dIOHandlerStrIn* handler )
{
    // Set LC_NUMERIC facet to "C" locale for this I/O operation
    // !!!! It would be better to set the locale of the stream itself,
    // but this does not work for two reasons:
    // 1. wxWindows streams don't support this
    // 2. CVG-IO uses e.g. wxString::ToDouble, which is independent of the stream
  //a2dCNumericLocaleSetter locSetter;

    bool ownhandler = false;
    if ( ! handler )
    {
#if wxART2D_USE_CVGIO
        //assume it is the default CVG format.
        handler = new a2dIOHandlerCVGIn();
        ownhandler = true;
#else
        a2dDocviewGlobals->ReportErrorF( a2dError_NoDocTemplateRef, _( "No template defined in a2dDocument::LoadObject" ) );
#endif //wxART2D_USE_CVGIO
    }

#if wxUSE_STD_IOSTREAM
    bool isOK = ( stream.fail() || stream.bad() ) == 0;
#else
    bool isOK = stream.IsOk ();
#endif

    bool oke = false;
    if ( isOK )
    {
        if ( handler )
        {
            if ( handler->CanLoad( stream, this, m_documentTemplate ? m_documentTemplate->GetDocumentClassInfo() : NULL )  )
            {
                if ( !handler->Load( stream, this ) )
                {
                    a2dDocviewGlobals->ReportErrorF( a2dError_CouldNotLoad, _( "I/O handler error during loading this format." ) );
                    m_lastError = a2dError_CouldNotLoad;
                }
                else
                    oke = true;
            }
            else
            {
                a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "I/O handler not capable of loading in this format." ) );
                m_lastError = a2dError_IOHandler;
            }
        }
        else
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "I/O handler must be != NULL, maybe template I/O handler not available in document template." ) );
            m_lastError = a2dError_IOHandler;
        }
    }
    else
        a2dDocviewGlobals->ReportErrorF( a2dError_ImportObject, _( "bad stream" ) );

    if ( ownhandler )
        delete handler;

    // now we are ready to prepare the document for rendering
    if ( oke )
    {
        Update( a2dCanvasObject::updatemask_force_and_count );

        a2dDocumentEvent event( wxEVT_POST_LOAD_DOCUMENT );
        event.SetEventObject( this );
        ProcessEvent( event );
    }
    else
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_CouldNotLoad, _( "bad stream" ) );
        m_lastError = a2dError_CouldNotLoad;
    }

    return stream;
}

bool a2dCanvasDocument::Load( const wxString& filename, a2dIOHandlerStrIn* handler )
{
    m_lastError = a2dError_NoError;
#if wxUSE_STD_IOSTREAM
    a2dDocumentFileInputStream stream( filename.mb_str(), wxSTD ios_base::in | wxSTD ios_base::binary );
    if ( stream.fail() || stream.bad() )
#else
    wxFileInputStream stream( filename );
    if ( !stream.IsOk() )
#endif
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s for loading" ), filename.c_str() );
        return false;
    }

    if ( handler )
    {
        if ( handler->CanLoad( stream, this, m_documentTemplate ? m_documentTemplate->GetDocumentClassInfo() : NULL )  )
        {
            if ( !handler->Load( stream, this ) )
            {
                a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "template I/O handler failed loading from stream." ) );
                m_lastError = a2dError_CouldNotLoad;
            }
            else
            {
                UpdateAllViews( NULL, a2dCANVIEW_UPDATE_ALL );
                return true;
            }
        }
        else
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "template I/O handler not capable of loading this format." ) );
            m_lastError = a2dError_IOHandler;
        }
    }
    else
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_LoadObject, _( "I/O handler not set in document template." ) );
        m_lastError = a2dError_NoDocTemplateRef;
    }
    return false;
}

#if wxART2D_USE_CVGIO
void a2dCanvasDocument::Load( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& parser )
{
    a2dWalker_SetCheck setp( false );
    setp.Start( m_drawing );
    if ( m_layersetup )
        setp.Start( m_layersetup );

    parser.Next();
    parser.Require( START_TAG, wxT( "cvg" ) );

    wxString main;
    wxString show;
    wxString start;
    if ( parser.HasAttribute( wxT( "main" ) ) )
    {
        main = parser.GetAttributeValue( "main" );
    }
    if ( parser.HasAttribute( wxT( "show" ) ) )
    {
        show = parser.GetAttributeValue( "show" );
    }
    if ( parser.HasAttribute( wxT( "start" ) ) )
    {
        start = parser.GetAttributeValue( "start" );
    }
    if ( parser.HasAttribute( wxT( "multiroot" ) ) )
    {
        m_multiRoot = parser.GetAttributeValueBool( "multiroot" );
    }
    m_mayEdit = parser.GetAttributeValueBool( wxT( "mayEdit" ), true );

    parser.Next();
    if ( parser.GetTagName() == wxT( "title" ) )
    {
        m_documentTitle = parser.GetContent();
        parser.Next();
        parser.Require( END_TAG, wxT( "title" ) );
        parser.Next();
    }
    if ( parser.GetTagName() == wxT( "desc" ) )
    {
        m_description = parser.GetContent();
        parser.Next();
        parser.Require( END_TAG, wxT( "desc" ) );
        parser.Next();
    }
    if ( parser.GetTagName() == wxT( "libraryname" ) )
    {
        m_libraryname = parser.GetContent();
        parser.Next();
        parser.Require( END_TAG, wxT( "libraryname" ) );
        parser.Next();
    }
    if ( parser.GetTagName() == wxT( "version" ) )
    {
        m_version = parser.GetContent();
        parser.Next();
        parser.Require( END_TAG, wxT( "version" ) );
        parser.Next();
    }
    else
	    m_version = wxT( "1" );

    // now define format version in parser for nested object to test
    parser.SetFormatVersion( m_version );

    if ( parser.GetTagName() == wxT( "o" ) && parser.GetAttributeValue( wxT( "classname" ) ) == wxT( "a2dLayers" ) )
    {
        if ( m_layersetup )
           m_layersetup->ReleaseChildObjects();
        else
        {
            m_layersetup = new a2dLayers();
        }
        m_layersetup->Load( NULL, parser );
        m_layersetup->GetLayerIndex();
    }

    UpdateLoadedLayerSetup();

    //old CVG did not have drawings, make it compatible
    if ( parser.GetTagName() == wxT( "o" ) && parser.GetAttributeValue( wxT( "classname" ) ) == wxT( "a2dDrawing" ) )
    {
        m_drawing->Load( (wxObject*) this, parser );
    }
    else if ( parser.GetTagName() == wxT( "o" ) )
    {
        m_drawing = new a2dDrawing( wxT("") );
        m_drawing->SetLayerSetup( m_layersetup );
        m_drawing->GetRootObject()->Load( this, parser );
    }
    //m_drawing->SetRootRecursive(); Is done inside a2dDrawing when after loading

    parser.Require( END_TAG, wxT( "cvg" ) );

    if ( !main.IsEmpty() )
    {
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_main, main );
    }
    if ( !show.IsEmpty() )
    {
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_show, show );
    }
    if ( !start.IsEmpty() )
    {
        parser.ResolveOrAdd( ( a2dSmrtPtr<class a2dObject>* ) &m_start, start );
    }
}
#endif //wxART2D_USE_CVGIO

bool a2dCanvasDocument::SaveLayers( const wxString& filename )
{
    wxASSERT_MSG( m_layersetup, wxT( "no a2dLayers set for a2dCanvasDocument" ) );
    return m_layersetup->SaveLayers( filename );
}

bool a2dCanvasDocument::LoadLayers( const wxString& filename )
{
    m_lastError = a2dError_NoError;
#if wxUSE_STD_IOSTREAM
    a2dDocumentFileInputStream stream( filename.mb_str(), wxSTD ios_base::in | wxSTD ios_base::binary );
    if ( stream.fail() || stream.bad() )
#else
    wxFileInputStream stream( filename );
    if ( !stream.Ok() )
#endif
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_LoadLayers, _( "Sorry, could not open file %s for loading" ), filename.c_str() );
        m_lastError = a2dError_LoadLayers;
        return false;
    }

    if ( !m_layersetup )
    {
        m_layersetup = new a2dLayers();
    }
    m_layersetup->SetFileName( filename );
    m_layersetup->ReleaseChildObjects();

    bool done = true;

#if wxART2D_USE_CVGIO
    //layers or in CVG format.
    a2dIOHandlerCVGIn* handler = new a2dIOHandlerCVGIn();

    if ( handler && handler->CanLoad( stream, m_layersetup, NULL )  )
    {
        if ( !handler->LoadLayers( stream, m_layersetup ) )
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_LoadLayers, _( "template I/O handler not capable of loading this format." ) );
            m_lastError = a2dError_LoadLayers;
            done = false;
        }
    }
    else
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_LoadLayers, _( "template I/O handler not capable of loading this format." ) );
        m_lastError = a2dError_LoadLayers;
        done = false;
    }
#else
    wxFAIL_MSG( wxT( "wxART2D_USE_CVGIO is needed to load layers from a file CVG file" ) );
#endif //wxART2D_USE_CVGIO

#if wxART2D_USE_CVGIO
    delete handler;
#endif //wxART2D_USE_CVGIO
    return done;
}

void a2dCanvasDocument::UpdateLoadedLayerSetup()
{
}

a2dDocumentOutputStream& a2dCanvasDocument::SaveObject( a2dDocumentOutputStream& stream, a2dIOHandlerStrOut* handler )
{
    //make sure all available layers are checked.
    m_layersetup->SetAvailable( m_drawing );

    // Set LC_NUMERIC facet to "C" locale for this I/O operation
    // !!!! It would be better to set the locale of the stream itself,
    // but this does not work for two reasons:
    // 1. wxWindows streams don't support this
    // 2. CVG-IO uses e.g. wxString::ToDouble, which is independent of the stream
  //a2dCNumericLocaleSetter locSetter;

#if wxUSE_STD_IOSTREAM
    bool isOK = ( stream.fail() || stream.bad() ) == 0;
#else
    bool isOK = stream.IsOk ();
#endif

    bool ownhandler = false;
    if ( ! handler )
    {
#if wxART2D_USE_CVGIO
        //assume it is the default CVG format.
        handler = new a2dIOHandlerCVGOut();
        ownhandler = true;
#else
        wxFAIL_MSG( wxT( "wxART2D_USE_CVGIO is needed to load object from a file CVG file" ) );
#endif //wxART2D_USE_CVGIO
    }

    if ( isOK )
    {
        if ( handler )
        {
            if ( handler->CanSave( this )  )
            {
                if ( !handler->Save( stream, this ) )
                {
                    a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "template I/O handler error during saving in this format" ) );
                    m_lastError = a2dError_IOHandler;
                }
            }
            else
            {
                a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "template I/O handler not capable of saving in this format." ) );
                m_lastError = a2dError_IOHandler;
            }
        }
        else
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "template I/O handler not set in document template" ) );
            m_lastError = a2dError_IOHandler;
        }
    }
    else
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_SaveObject,  _( "bad stream" ) );
        m_lastError = a2dError_SaveObject;
    }

    if ( ownhandler )
        delete handler;

    return stream;
}

bool a2dCanvasDocument::Save( const wxString& filename, a2dIOHandlerStrOut* handler ) const
{
#if wxUSE_STD_IOSTREAM
    a2dDocumentFileOutputStream stream( filename.mb_str(), wxSTD ios_base::out | wxSTD ios_base::binary );
    if ( stream.fail() || stream.bad() )
#else
    wxFileOutputStream stream( filename );
    if ( stream.GetLastError() != wxSTREAM_NO_ERROR )
#endif
    {
        a2dDocviewGlobals->ReportErrorF( a2dError_SaveFile, _( "Sorry, could not open file %s for saving" ), filename.c_str() );

        // Saving error
        return false;
    }

    if ( handler )
    {
        if ( handler->CanSave( this )  )
        {
            if ( handler->Save( stream, this ) )
                return true;

            a2dDocviewGlobals->ReportErrorF( a2dError_SaveFile, _( "template I/O handler not succeeded to save in this format to file %s" ), filename.c_str() );
        }
        else
            a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "template I/O handler not capable of saving in this format." ) );
    }
    else
        a2dDocviewGlobals->ReportErrorF( a2dError_IOHandler, _( "template I/O handler not set in document template" ) );
    return false;
}


#if wxART2D_USE_CVGIO
void a2dCanvasDocument::Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* WXUNUSED( towrite ) )
{
    //check flag is used for writing once an object which is referenced many times
    a2dWalker_SetCheck setp( false );
    setp.Start( m_drawing );
    setp.Start( m_layersetup );

    out.WriteStartDocument( wxT( "1.0" ), wxT( "UTF-8" ) , true );

    out.WriteStartElementAttributes( wxT( "cvg" ) );
    out.WriteAttribute( wxT( "classname" ), GetClassInfo()->GetClassName() );
    if ( m_main )
        out.WriteAttribute( wxT( "main" ), m_main->GetId() );
    if ( m_show )
        out.WriteAttribute( wxT( "show" ), m_show->GetId() );
    if ( m_start )
        out.WriteAttribute( wxT( "start" ), m_start->GetId() );
    out.WriteAttribute( wxT( "multiroot" ), m_multiRoot );
    out.WriteAttribute( wxT( "mayEdit" ), m_mayEdit );
    out.WriteEndAttributes();

    out.WriteElement( wxT( "title" ), GetPrintableName() );
    out.WriteElement( wxT( "desc" ), m_description );
    out.WriteElement( wxT( "libraryname" ), m_libraryname );
	out.WriteElement( wxT( "version" ), m_version );

    a2dObjectList towritelayer;
    if ( m_layersetup )
        m_layersetup->Save( NULL, out, &towritelayer );

    m_drawing->Save( this, ( a2dIOHandlerCVGOut& ) out, NULL, NULL );

    out.WriteEndElement();
    out.WriteEndDocument();

}

void a2dCanvasDocument::Save( a2dIOHandlerCVGOut& out, a2dCanvasObject* start )
{
    //check flag is used for writing once an object which is referenced many times
    a2dWalker_SetCheck setp( false );
    setp.Start( m_drawing );
    setp.Start( m_layersetup );

    out.WriteStartDocument( wxT( "1.0" ), wxT( "UTF-8" ) , true );

    out.WriteStartElementAttributes( wxT( "cvg" ) );
    out.WriteAttribute( wxT( "classname" ), GetClassInfo()->GetClassName() );
    if ( m_main )
        out.WriteAttribute( wxT( "main" ), m_main->GetId() );
    if ( m_show )
        out.WriteAttribute( wxT( "show" ), m_show->GetId() );
    if ( m_start )
        out.WriteAttribute( wxT( "start" ), m_start->GetId() );
    out.WriteAttribute( wxT( "mayEdit" ), m_mayEdit );
    out.WriteEndAttributes();

    out.WriteElement( wxT( "title" ), GetPrintableName() );
    out.WriteElement( wxT( "desc" ), m_description );
    out.WriteElement( wxT( "libraryname" ), m_libraryname );
    out.WriteElement( wxT( "version" ), m_version );

    a2dObjectList towritelayer;
    if ( m_layersetup )
        m_layersetup->Save( NULL, out, &towritelayer );

    m_drawing->Save( this, ( a2dIOHandlerCVGOut& ) out, NULL, start );

    out.WriteEndElement();
    out.WriteEndDocument();
}

void a2dCanvasDocument::DoSave( a2dIOHandlerCVGOut& out, a2dCanvasObjectList* WXUNUSED( towrite ) )
{
    out.WriteEndAttributes();
}
#endif //wxART2D_USE_CVGIO

bool a2dCanvasDocument::WriteSVG( a2dCanvasView* drawer, const wxString& filename, a2dCanvasObject* WXUNUSED( top ), double WXUNUSED( Width ), double WXUNUSED( Height ), wxString unit )
{
    wxASSERT_MSG( drawer, wxT( "no a2dCanvasView set for a2dCanvasDocument" ) );

    a2dWalker_SetCheck setp( false );
    setp.Start( m_drawing );
    if ( m_layersetup )
        setp.Start( m_layersetup );

    bool done = false;
#if wxART2D_USE_SVGIO
    a2dIOHandlerSVGOut* handler = new a2dIOHandlerSVGOut();
    a2dIterC ic( drawer->GetDrawingPart() );

    handler->SetIterC( ic );

    done = Save( filename, handler );

    delete handler;
#else
    wxFAIL_MSG( wxT( "wxART2D_USE_SVGIO is needed to write doc into SVG file" ) );
#endif //wxART2D_USE_SVGIO

    return done;
}

void a2dCanvasDocument::OnIdle( wxIdleEvent& event )
{
    event.Skip();
    //m_drawing->ProcessEvent( event );
}
