/*! \file editor/src/canedit.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canedit.cpp,v 1.225 2009/09/30 19:17:00 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/colordlg.h"

#if wxCHECK_VERSION(2, 5, 0)
#if !WXWIN_COMPATIBILITY_2_4
#include "wx/numdlg.h"
#endif
#endif

#include <wx/wfstream.h>
#if wxUSE_XRC
#include <wx/xrc/xmlres.h>
#include <wx/docview/xh_a2dmenudoc.h>
#endif

#include "wx/editor/editmod.h"
#include "wx/canextobj/sttoolext.h"

#if wxART2D_USE_CANEXTOBJ
#include "wx/canextobj/canextmod.h"
#endif

#if wxART2D_USE_GDSIO
#include "wx/gdsio/gdsio.h"
#endif //wxART2D_USE_GDSIO

#include "images/smile.xpm"
#include "wx/docview/doccom.h"

IMPLEMENT_CLASS( a2dEditorMultiFrameViewConnector, a2dFrameViewConnector )
IMPLEMENT_DYNAMIC_CLASS( a2dEditorFrame, a2dDocumentFrame )


BEGIN_EVENT_TABLE( a2dEditorMultiFrameViewConnector, a2dFrameViewConnector )
    EVT_POST_CREATE_DOCUMENT( a2dEditorMultiFrameViewConnector::OnPostCreateDocument )
    EVT_POST_CREATE_VIEW( a2dEditorMultiFrameViewConnector::OnPostCreateView )
END_EVENT_TABLE()

a2dEditorMultiFrameViewConnector::a2dEditorMultiFrameViewConnector( wxClassInfo* EditorClassInfo )
    : a2dFrameViewConnector()
{
    m_editorClassInfo = EditorClassInfo;
    m_initialStyle = a2dCanvasGlobals->m_initialDocChildFrameStyle;
    m_initialPos = a2dCanvasGlobals->m_initialDocChildFramePos;
    m_initialSize = a2dCanvasGlobals->m_initialDocChildFrameSize;
}


void a2dEditorMultiFrameViewConnector::OnPostCreateDocument( a2dTemplateEvent& event )
{
    //a2dDocumentTemplate* templ = (a2dDocumentTemplate*) event.GetEventObject();
    //a2dDocument *doc = event.GetDocument();

    a2dViewTemplate* viewtempl;
    viewtempl = a2dDocviewGlobals->GetDocviewCommandProcessor()->SelectViewType( event.GetDocument(),
                m_viewTemplates.size() ? m_viewTemplates : a2dDocviewGlobals->GetDocviewCommandProcessor()->GetViewTemplates() );

    if ( !viewtempl )
    {
        wxLogMessage( wxT( "No view available for this document in a2dEditorMultiFrameViewConnector" ) );
        return;
    }

    if ( !viewtempl->CreateView( event.GetDocument(), event.GetFlags() ) )
    {
        wxLogMessage( wxT( "a2dEditorMultiFrameViewConnector no view was created" ) );
        return;
    }
}


void a2dEditorMultiFrameViewConnector::OnPostCreateView( a2dTemplateEvent& event )
{
    a2dCanvasView* drawerview = ( a2dCanvasView* ) event.GetView();

    //a2dCanvasDocument* doc = (a2dCanvasDocument*) event.GetView()->GetDocument();

    if ( drawerview->GetViewTypeName() == wxT( "Drawing View Frame" ) )
    {
        a2dEditorFrame* editorFrame;
        if ( m_editorClassInfo )
        {
            editorFrame = ( a2dEditorFrame* ) m_editorClassInfo->CreateObject();
            editorFrame->Create( false,
                                 m_docframe,
                                 GetInitialPosition(),
                                 GetInitialSize(),
                                 GetInitialStyle()
                               );
#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
	        a2dViewCanvasOgl* canvas = new a2dViewCanvasOgl( drawerview, editorFrame, -1, GetInitialPosition(), GetInitialSize(), a2dCanvasGlobals->m_initialWorldCanvasStyle );
#else
	        a2dViewCanvas* canvas = new a2dViewCanvas( drawerview, editorFrame, -1, GetInitialPosition(), GetInitialSize(), a2dCanvasGlobals->m_initialWorldCanvasStyle );
#endif
            editorFrame->SetDrawingPart( canvas->GetDrawingPart() );
            drawerview->SetDisplayWindow( canvas );
            //canvas->GetDrawingPart()->SetBufferSize( m_initialSize.GetWidth(), m_initialSize.GetHeight() );
            editorFrame->Init();
        }
        else
        {
#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
            a2dOglCanvas* canvas = wxStaticCast( drawerview->GetDisplayWindow(), a2dOglCanvas );
#else
            a2dCanvas* canvas = wxStaticCast( drawerview->GetDisplayWindow(), a2dCanvas );
#endif

            canvas->GetDrawingPart()->SetBufferSize( m_initialSize.GetWidth(), m_initialSize.GetHeight() );
            editorFrame = new a2dEditorFrame( false,
                                              m_docframe,
                                              GetInitialPosition(),
                                              GetInitialSize(),
                                              GetInitialStyle()
                                            );
            drawerview->SetDisplayWindow( editorFrame->GetDrawingPart()->GetDisplayWindow() );
            editorFrame->Init();
        }
        editorFrame->Enable();
        editorFrame->Show( true );
    }
    else if ( drawerview->GetViewTypeName() == wxT( "Drawing Anti Aliased View Frame" ) )
    {
        a2dEditorFrame* editorFrame;
        if ( m_editorClassInfo )
        {
            editorFrame = ( a2dEditorFrame* ) m_editorClassInfo->CreateObject();
            editorFrame->Create( false,
                                 m_docframe,
                                 GetInitialPosition(),
                                 GetInitialSize(),
                                 GetInitialStyle()
                               );
            a2dCanvasDocument* doc = wxStaticCast( drawerview->GetDocument(), a2dCanvasDocument );
	        a2dViewCanvas* canvas = new a2dViewCanvas( drawerview, editorFrame, -1, GetInitialPosition(), GetInitialSize(), a2dCanvasGlobals->m_initialWorldCanvasStyle );
			editorFrame->SetDrawingPart( canvas->GetDrawingPart() );
            drawerview->SetDisplayWindow( editorFrame->GetDrawingPart()->GetDisplayWindow() );
            drawerview->GetDrawingPart()->SetBufferSize( m_initialSize.GetWidth(), m_initialSize.GetHeight() );
			drawerview->GetDrawingPart()->SetShowObject( doc->GetDrawing()->GetRootObject() );
            editorFrame->Init();
        }
        else
        {
            drawerview->GetDrawingPart()->SetBufferSize( m_initialSize.GetWidth(), m_initialSize.GetHeight() );
            editorFrame = new a2dEditorFrame( false,
                                              m_docframe,
                                              GetInitialPosition(),
                                              GetInitialSize(),
                                              GetInitialStyle()
                                            );
            drawerview->SetDisplayWindow( editorFrame->GetDrawingPart()->GetDisplayWindow() );
            editorFrame->Init();
        }
        editorFrame->Enable();
        editorFrame->Show( true );
    }
    else
        wxLogMessage( wxT( "a2dEditorMultiFrameViewConnector not a -Drawing View Frame-, no view was created" ) );

    //this is the parent frame of all child frames, and keeps overview
    m_docframe->Refresh();
}

/********************************************************************
* a2dEditorFrame
*********************************************************************/
a2dEditorFrameEvent::a2dEditorFrameEvent( a2dEditorFrame* editorFrame, wxEventType type, int id )
    : wxEvent( id, type )
{
    SetEventObject( editorFrame );
}

a2dEditorFrame* a2dEditorFrameEvent::GetEditorFrame() const
{
    return wxDynamicCast( GetEventObject(), a2dEditorFrame );
}

wxEvent* a2dEditorFrameEvent::Clone( void ) const
{
    return new a2dEditorFrameEvent( GetEditorFrame(), m_eventType, m_id );
}

DEFINE_EVENT_TYPE( a2dEVT_THEME_EVENT )
DEFINE_EVENT_TYPE( a2dEVT_INIT_EVENT )


const long TOOLOPTION_FIRST = wxNewId();
const long TOOLOPTION_ONTOP = TOOLOPTION_FIRST;
const long TOOLOPTION_RECTANGLE = wxNewId();
const long TOOLOPTION_REDRAW = wxNewId();
const long TOOLOPTION_COPY = wxNewId();

const long TOOLOPTION_FILLED = wxNewId();
const long TOOLOPTION_WIRE = wxNewId();
const long TOOLOPTION_INVERT = wxNewId();
const long TOOLOPTION_WIRE_ZERO_WIDTH = wxNewId();
const long TOOLOPTION_INVERT_ZERO_WIDTH = wxNewId();
const long TOOLOPTION_LAST = TOOLOPTION_INVERT_ZERO_WIDTH;

const long MSHOW_T = wxNewId();
const long MSHOW_FIRST = wxNewId();
const long MSHOW_LIBPOINTS = wxNewId();
const long MSHOW_LIB = wxNewId();
const long MSHOW_LAST = MSHOW_LIB;

const long MSCREEN_T = wxNewId();

//const long MTOOL_PlaceFromLibByRef= wxNewId();
//const long MTOOL_PlaceFromLibByCopy= wxNewId();

const long Layer_view = wxNewId();
const long a2dShapeClipboard_Copy = wxNewId();
const long a2dShapeClipboard_Paste = wxNewId();
const long a2dShapeClipboard_Drag = wxNewId();

BEGIN_EVENT_TABLE( a2dEditorFrame, a2dDocumentFrame )

    EVT_MENU( wxID_EXIT, a2dEditorFrame::Quit )

//  EVT_MENU(MTOOL_PlaceFromLibByRef,a2dEditorFrame::PlaceFromLibByRef)
//  EVT_MENU(MTOOL_PlaceFromLibByCopy,a2dEditorFrame::PlaceFromLibByCopy)

    EVT_MENU( wxID_ABOUT, a2dEditorFrame::OnAbout )
    EVT_MENU( wxID_UNDO, a2dEditorFrame::OnUndo )
    EVT_MENU( wxID_REDO, a2dEditorFrame::OnRedo )
    EVT_MENUSTRINGS( a2dEditorFrame::OnSetmenuStrings )

    EVT_DO( a2dEditorFrame::OnDoEvent )
    EVT_UNDO( a2dEditorFrame::OnUndoEvent )

    EVT_COM_EVENT( a2dEditorFrame::OnComEvent )

    EVT_CLOSE( a2dEditorFrame::OnCloseWindow )

    EVT_ACTIVATE( a2dEditorFrame::OnActivate )

    EVT_THEME_EVENT( a2dEditorFrame::OnTheme )
    EVT_INIT_EVENT( a2dEditorFrame::OnInit )

    EVT_MENU( MSCREEN_T, a2dEditorFrame::FillData )

    EVT_ACTIVATE_VIEW_SENT_FROM_CHILD( a2dEditorFrame::OnActivateViewSentFromChild )

    EVT_MENU( a2dShapeClipboard_Copy,  a2dEditorFrame::OnCopyShape)
    EVT_MENU( a2dShapeClipboard_Paste, a2dEditorFrame::OnPasteShape)
    EVT_MENU( a2dShapeClipboard_Drag,  a2dEditorFrame::OnDragSimulate)
    EVT_UPDATE_UI( a2dShapeClipboard_Paste, a2dEditorFrame::OnUpdatePasteShape)
    EVT_UPDATE_UI( a2dShapeClipboard_Copy, a2dEditorFrame::OnUpdateCopyShape)

END_EVENT_TABLE()

a2dEditorFrame::a2dEditorFrame()
    : a2dDocumentFrame()
{
    m_initialized = false;
    m_drawingPart = NULL;
}

a2dEditorFrame::a2dEditorFrame( bool isParent, wxFrame* parent, a2dCanvas* canvas, const wxPoint& pos, const wxSize& size, long style )
    : a2dDocumentFrame()
{
    m_initialized = false;
    m_drawingPart = NULL;

    // create a canvas in Create, the first arg. is true.
    Create( isParent, parent, pos, size, style );
    if ( !canvas )
        new a2dCanvas( this, -1, pos,  size, a2dCanvasGlobals->m_initialWorldCanvasStyle );
    m_drawingPart = canvas->GetDrawingPart();
}

#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
a2dEditorFrame::a2dEditorFrame( bool isParent, wxFrame* parent, a2dOglCanvas* canvas, const wxPoint& pos, const wxSize& size, long style )
    : a2dDocumentFrame()
{
    m_initialized = false;
    m_drawingPart = NULL;

    // create a canvas in Create, the first arg. is true.
    Create( isParent, parent, pos, size, style );

    if ( !canvas )
        canvas = new a2dOglCanvas( this, -1, pos,  size, a2dCanvasGlobals->m_initialWorldCanvasStyle );
    m_drawingPart = canvas->GetDrawingPart();
}
#endif //wxUSE_GLCANVAS 

a2dEditorFrame::a2dEditorFrame( bool isParent, wxFrame* parent, const wxPoint& pos, const wxSize& size, long style )
    : a2dDocumentFrame()
{
    m_initialized = false;
    m_drawingPart = NULL;

    Create( isParent, parent, pos, size, style );
}

bool a2dEditorFrame::Create( bool isParent,
                             wxFrame* parent, const wxPoint& pos , const wxSize& size, long style )
{
    m_drawingPart = NULL;
    m_initialized = false;

    bool res = a2dDocumentFrame::Create( isParent, parent, NULL, -1, wxT( "canvas editor" ) , pos, size, style );

    // A control test, with editing.
    //new wxButton(m_window,-1,  _T("Show a button"),wxPoint(100,100),wxSize(200,110) );
    return res;
}

void a2dEditorFrame::OnInit( a2dEditorFrameEvent& initEvent )
{
#ifdef __WXMSW__
    SetIcon( wxString( wxT( "chrt_icn" ) ) );
#endif

    m_editMenu = NULL;

    if ( m_drawingPart )
    {
        a2dFill backgr = a2dFill( wxColour( 255, 255, 255 ) );
        m_drawingPart->SetBackgroundFill( backgr );

        m_drawingPart->SetGridStroke( a2dStroke( wxColour( 239, 5, 64 ), 0, a2dSTROKE_DOT ) );
        //to show grid as lines
        //m_drawingPart1->SetGridLines(true);

        m_drawingPart->SetGridSize( 1 );
        m_drawingPart->SetGridX( 100 );
        m_drawingPart->SetGridY( 100 );

        //show it?
        m_drawingPart->SetGrid( false );
        m_drawingPart->SetGridAtFront( true );
    }
    m_initialized = true;
}

void a2dEditorFrame::OnTheme( a2dEditorFrameEvent& themeEvent )
{
    a2dEditorFrame* ef = themeEvent.GetEditorFrame();
    ef->Theme();

    wxUint16 lay = m_drawingPart->GetDrawing()->GetHabitat()->GetLayer();
    SetStatusText( m_drawingPart->GetDrawing()->GetLayerSetup()->GetName( lay ), 1 );
}

void a2dEditorFrame::Init()
{
    a2dCentralCanvasCommandProcessor* docmanager = a2dGetCmdh();
    wxASSERT_MSG( docmanager, wxT( "a2dCentralCanvasCommandProcessor is needed by a2dEditorFrame" ) );

    if ( m_drawingPart )
    {
        m_contr  = new a2dStToolContr( m_drawingPart, this, false );
        //m_contr->SetCrossHair( true );

        //the next is handy, but as you prefer
        m_contr->SetZoomFirst( true );
    }

    wxPathList pathList;
    pathList.Add( wxT( "../common/icons" ) );

    a2dEditorFrameEvent tevent( this, a2dEVT_INIT_EVENT );
    ProcessEvent( tevent );

    a2dEditorFrameEvent ievent( this, a2dEVT_THEME_EVENT );
    ProcessEvent( ievent );
}

a2dEditorFrame::~a2dEditorFrame()
{
}

void a2dEditorFrame::OnActivateViewSentFromChild( a2dViewEvent& viewevent )
{
    a2dView* view = ( a2dView* ) viewevent.GetEventObject();
    a2dCanvasView* canview = wxDynamicCast( view, a2dCanvasView );

    if ( canview != m_view ) 
         //canview->GetDrawingPart() != m_drawingPart ||
         //canview->GetDrawingPart()->GetDrawing() != m_drawingPart->GetDrawing() )
    {
        if ( viewevent.GetActive() )
        {
            //disconnect current m_drawingPart
            if ( m_view && m_drawingPart && m_drawingPart->GetDrawing() && m_drawingPart->GetDrawing()->GetCommandProcessor() )
            {
                m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_MENUSTRINGS, this );
                m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_DO, this );
                m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_UNDO, this );
            }
            m_view = view;
            canview = wxDynamicCast( view, a2dCanvasView );		
            m_drawingPart = canview->GetDrawingPart();
            m_contr  = wxDynamicCast(  m_drawingPart->GetCanvasToolContr(), a2dStToolContr );
            //connect new m_drawingPart
            if ( canview && m_drawingPart->GetDrawing() && m_drawingPart->GetDrawing()->GetCommandProcessor() )
            {
                m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_MENUSTRINGS, this );
                m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_DO, this );
                m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_UNDO, this );
                m_drawingPart->GetDrawing()->ConnectEvent( wxEVT_REDO, this );
            }
        }
        else
        {
            m_drawingPart = canview->GetDrawingPart();
            if ( m_drawingPart->GetDrawing() && m_drawingPart->GetDrawing()->GetCommandProcessor() )
            {
                m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_MENUSTRINGS, this );
                m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_DO, this );
                m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_UNDO, this );
                m_drawingPart->GetDrawing()->DisconnectEvent( wxEVT_REDO, this );
            }
            m_view = view;
        }
    }
    viewevent.Skip();
}

void a2dEditorFrame::OnCloseWindow( wxCloseEvent& event )
{
    wxTheClipboard->Clear();
    if ( !event.CanVeto() )
    {
    }
    event.Skip();
}

void a2dEditorFrame::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
    if ( !m_drawingPart )
        return;

    if ( m_drawingPart->GetDrawing() && m_drawingPart->GetDrawing()->GetCommandProcessor() )
        m_drawingPart->GetDrawing()->GetCommandProcessor()->Undo();
}

void a2dEditorFrame::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
    if ( !m_drawingPart )
        return;

    if ( m_drawingPart->GetDrawing() && m_drawingPart->GetDrawing()->GetCommandProcessor() )
        m_drawingPart->GetDrawing()->GetCommandProcessor()->Redo();
}

void a2dEditorFrame::Update()
{
    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
}

void a2dEditorFrame::Quit( wxCommandEvent& WXUNUSED( event ) )
{
    Close( true );
}

void a2dEditorFrame::OnActivate( wxActivateEvent& event )
{
    if ( !m_initialized )
	{
	    event.Skip();
        return;
	}
    event.Skip(); //skip to base OnActivate Handler which sets proper focus to child window(s)
}

void a2dEditorFrame::OnSetmenuStrings( a2dCommandProcessorEvent& event )
{
    if ( !m_drawingPart )
        return;

    if ( m_editMenu && m_drawingPart->GetDrawing() && event.GetEventObject() == m_drawingPart->GetDrawing()->GetCommandProcessor() )
    {
        m_editMenu->SetLabel( wxID_UNDO, event.GetUndoMenuLabel() );
        m_editMenu->Enable( wxID_UNDO, event.CanUndo() );

        m_editMenu->SetLabel( wxID_REDO, event.GetRedoMenuLabel() );
        m_editMenu->Enable( wxID_REDO, event.CanRedo() );
    }
}

void a2dEditorFrame::OnUndoEvent( a2dCommandProcessorEvent& WXUNUSED( event ) )
{

}

void a2dEditorFrame::OnDoEvent( a2dCommandProcessorEvent& event )
{
    wxString cmdName = event.GetCommand()->GetName();

    if ( !m_drawingPart )
        return;

    if ( m_drawingPart->GetDrawing() && event.GetEventObject() == m_drawingPart->GetDrawing()->GetCommandProcessor() )
    {
    }
}

void a2dEditorFrame::OnComEvent( a2dComEvent& event )
{
    if ( event.GetId() == a2dComEvent::sm_changedProperty )
    {
        a2dNamedProperty* property = event.GetProperty();
        a2dObject* object = property->GetRefObjectNA();

        if ( 0 != wxDynamicCast( property, a2dStrokeProperty ) )
        {
            a2dStroke stroke = ( ( a2dStrokeProperty* )property )->GetValue();
        }
        else if ( 0 != wxDynamicCast( property, a2dFillProperty ) )
        {
            a2dFill fill = ( ( a2dFillProperty* )property )->GetValue();
        }
        else
            event.Skip();
    }
    else if ( m_drawingPart && event.GetId() == a2dStToolContr::sm_showCursor && GetStatusBar() )
    {
        a2dStToolContr* contr = wxStaticCast( event.GetEventObject(), a2dStToolContr );
        wxString str;

        SetStatusText( contr->GetStatusStrings()[0], 0 );
        SetStatusText( contr->GetStatusStrings()[1], 1 );
        SetStatusText( contr->GetStatusStrings()[2], 2 );
        SetStatusText( contr->GetStatusStrings()[3], 3 );
        if (  m_drawingPart->GetDrawing() )
        {
            SetStatusText( m_drawingPart->GetDrawing()->GetHabitat()->GetConnectionGenerator()->GetRouteMethodAsString(), 4 );

            wxUint16 lay = m_drawingPart->GetDrawing()->GetHabitat()->GetLayer();
            SetStatusText( contr->GetDrawingPart()->GetDrawing()->GetLayerSetup()->GetName( lay ), 5 );
        }
/*
        a2dStToolContr* contr = wxStaticCast( event.GetEventObject(), a2dStToolContr );

        wxPoint mpos = contr->GetMousePosition();

        double mouse_worldx, mouse_worldy;
        contr->GetDrawingPart()->MouseToToolWorld( mpos.x, mpos.y, mouse_worldx, mouse_worldy );

        wxString unitmeasure = contr->GetDrawingPart()->GetDrawing()->GetUnits();
        double scaleToMeters = contr->GetDrawingPart()->GetDrawing()->GetUnitsScale();

        double multi = 1;
        a2dDoMu::GetMultiplierFromString( unitmeasure, multi );

        mouse_worldx *= scaleToMeters / multi;
        mouse_worldy *= scaleToMeters / multi;

        wxString str;
        str.Printf( _T( "%6.3f,%6.3f" ), mouse_worldx, mouse_worldy );

        if  ( contr->GetFirstTool() )
            str = str + _T( " " ) + unitmeasure + _T( " :" ) + contr->GetFirstTool()->GetClassInfo()->GetClassName();


        SetStatusText( str );
        wxUint16 lay = a2dCanvasGlobals->GetLayer();
        if ( m_drawingPart )
        {   
            wxUint16 lay = a2dCanvasGlobals->GetLayer();
            SetStatusText( m_drawingPart->GetDrawing()->GetLayerSetup()->GetName( lay ), 1 );
        }
*/
    }
    else if ( event.GetId() == a2dHabitat::sig_changedLayer )
    {
        a2dStToolContr* contr = wxStaticCast( event.GetEventObject(), a2dStToolContr );
        wxUint16 lay = m_drawingPart->GetDrawing()->GetHabitat()->GetLayer();
        SetStatusText( contr->GetDrawingPart()->GetDrawing()->GetLayerSetup()->GetName( lay ), 5 );
    }

    else
        event.Skip();
}

void a2dEditorFrame::OnMenu( wxCommandEvent& event )
{
    if ( event.GetId() == Layer_view )
    {
        a2dSmrtPtr<a2dCanvasDocument> doc = new a2dCanvasDocument();
        doc->SetDocumentTypeName( _T( "a2dCanvasDocument" ) );
        doc->GetDrawing()->GetRootObject()->Append( m_drawingPart->GetDrawing()->GetLayerSetup() );

        a2dView* createdview;
        createdview = a2dDocviewGlobals->GetDocviewCommandProcessor()->AddDocumentCreateView( doc, _T( "Drawing View Frame" ) );
    }
    else if ( event.GetId() == MSHOW_LIBPOINTS )
    {
        ConnectDocument( a2dGetCmdh()->GetLibraryPoints() );
        m_contr->Zoomout();
    }
    else
        event.Skip();
}

/*
void a2dEditorFrame::PlaceFromLibByRef()
{
    a2dCanvasObjectList total;

    m_library->CollectObjects( &total, _T("a2dNameReference"),a2dCANOBJ_ALL,_T(""), false );

    CanvasObjectsDialog objects(this,m_library,&total, true, (wxSTAY_ON_TOP |wxRESIZE_BORDER | wxCAPTION));
    if (objects.ShowModal() == wxID_OK)
    {
        a2dCanvasObject* obj=((a2dNameReference*)objects.GetCanvasObject())->GetCanvasObject();
        obj->SetPosXY(0,0);
        a2dCanvasObjectReference* ref = new a2dCanvasObjectReference(0,0,obj);
        m_drawingPart->GetShowObject()->Append(ref);
        DOC->SetCanvasDocumentRecursive();
        m_contr->Zoomout();
    }
}

void a2dEditorFrame::PlaceFromLibByCopy()
{
    a2dCanvasObjectList total;

    m_library->CollectObjects( &total,_T("a2dNameReference"),a2dCANOBJ_ALL,_T(""), false );

    CanvasObjectsDialog objects(this,m_library,&total, true, (wxSTAY_ON_TOP |wxRESIZE_BORDER | wxCAPTION));
    if (objects.ShowModal() == wxID_OK)
    {
        a2dCanvasObject* obj=((a2dNameReference*)objects.GetCanvasObject())->GetCanvasObject()->TClone();
        obj->SetPosXY(0,0);
        m_drawingPart->GetShowObject()->Append(obj);
        DOC->SetCanvasDocumentRecursive();
        m_contr->Zoomout();
    }
}
*/

void a2dEditorFrame::SetToolDragingMode( wxCommandEvent& event )
{
    m_menuBar->Check( TOOLOPTION_ONTOP, false );
    m_menuBar->Check( TOOLOPTION_RECTANGLE, false );
    m_menuBar->Check( TOOLOPTION_REDRAW, false );
    m_menuBar->Check( TOOLOPTION_COPY, false );
    if ( event.GetId() == TOOLOPTION_ONTOP )
    {
        m_contr->SetDragMode( wxDRAW_ONTOP );
        m_menuBar->Check( TOOLOPTION_ONTOP, true );
    }
    if ( event.GetId() == TOOLOPTION_RECTANGLE )
    {
        m_contr->SetDragMode( wxDRAW_RECTANGLE );
        m_menuBar->Check( TOOLOPTION_RECTANGLE, true );
    }
    if ( event.GetId() == TOOLOPTION_REDRAW )
    {
        m_contr->SetDragMode( wxDRAW_REDRAW );
        m_menuBar->Check( TOOLOPTION_REDRAW, true );
    }
    if ( event.GetId() == TOOLOPTION_COPY )
    {
        m_contr->SetDragMode( wxDRAW_COPY );
        m_menuBar->Check( TOOLOPTION_COPY, true );
    }
}

void a2dEditorFrame::SetToolDrawingMode( wxCommandEvent& event )
{
    m_menuBar->Check( TOOLOPTION_FILLED, false );
    m_menuBar->Check( TOOLOPTION_WIRE, false );
    m_menuBar->Check( TOOLOPTION_INVERT, false );
    m_menuBar->Check( TOOLOPTION_WIRE_ZERO_WIDTH, false );
    m_menuBar->Check( TOOLOPTION_INVERT_ZERO_WIDTH, false );

    if ( event.GetId() == TOOLOPTION_FILLED )
    {
        m_contr->SetDrawMode( a2dFILLED );
        m_menuBar->Check( TOOLOPTION_FILLED, true );
    }
    if ( event.GetId() == TOOLOPTION_WIRE )
    {
        m_contr->SetDrawMode( a2dWIREFRAME );
        m_menuBar->Check( TOOLOPTION_WIRE, true );
    }
    if ( event.GetId() == TOOLOPTION_INVERT )
    {
        m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
        m_menuBar->Check( TOOLOPTION_INVERT, true );
    }
    if ( event.GetId() == TOOLOPTION_WIRE_ZERO_WIDTH )
    {
        m_contr->SetDrawMode( a2dWIREFRAME_ZERO_WIDTH );
        m_menuBar->Check( TOOLOPTION_WIRE_ZERO_WIDTH, true );
    }
    if ( event.GetId() == TOOLOPTION_INVERT_ZERO_WIDTH )
    {
        m_contr->SetDrawMode( a2dWIREFRAME_INVERT_ZERO_WIDTH );
        m_menuBar->Check( TOOLOPTION_INVERT_ZERO_WIDTH, true );
    }
}

void a2dEditorFrame::AddFunctionToMenu( int id, wxMenu* parentMenu, const wxString& text, const wxString& helpString, wxObjectEventFunctionM func, bool check )
{
    Connect( id, wxEVT_COMMAND_MENU_SELECTED, ( wxObjectEventFunction ) wxStaticCastEvent( wxCommandEventFunction, func ) );
    parentMenu->Append( id, text, helpString, check );
}

void a2dEditorFrame::RemoveFromMenu( int id, wxMenu* parentMenu, wxObjectEventFunctionM func )
{
    Disconnect( id, wxEVT_COMMAND_MENU_SELECTED, ( wxObjectEventFunction ) wxStaticCastEvent( wxCommandEventFunction, func ) );
    parentMenu->Delete( id );
}

void a2dEditorFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( _T( "a2dEditorFrame\nKlaas Holwerda 2002" ),
                          _T( "About a2dEditorFrame" ), wxICON_INFORMATION | wxOK );
}

void a2dEditorFrame::ConnectDocument( a2dCanvasDocument* doc, a2dCanvasView* drawer )
{
    if ( !doc )
        return;

/*
    a2dViewList allviews;
    doc->ReportViews( &allviews );

    if ( !drawer || wxNOT_FOUND == allviews.IndexOf( drawer ) )
    {
        if ( m_drawingPart->GetDrawingPart() )  //document change means: this view will now belong to the new document
             m_drawingPart->GetDrawingPart()->SetDocument( doc );
    }
    else
    {
        m_drawingPart->SetDrawingPart( drawer );
    }

    //if ( !doc->GetFirstView() )
    //    a2dDocviewGlobals->GetDocviewCommandProcessor()CreateView(doc,"Drawing View" )->OnUpdate(NULL);

    if ( m_view )
    {
        m_view->SetDisplayWindow( NULL );
    }

    m_view->SetDisplayWindow( m_drawingPart );
    m_view->Update();
    if ( m_view->GetDocument() )
    {
        //maybe something to change the title
    }
    m_view->Activate( true );
*/
}

/*
void a2dEditorFrame::FillLibrary()
{
    //root group always at 0,0
    m_library = new a2dCanvasDocument();
    m_library->Own();

    //always add the commandprocessor
    //this is needed for tools, so not strictly needed.
    m_library->CreateCommandProcessor();

    {
        a2dCircle* cire = new  a2dCircle( -100,0, 70 );
        cire->SetFill( wxColour(219,21,6),a2dFILL_HORIZONTAL_HATCH );
        cire->SetStroke(wxColour(1,3,205 ),3.0);

        a2dLibraryReference* libobject = new a2dLibraryReference(0,0,cire,_T("circle"),100);
        libobject->SetStroke(wxColour(229,5,64 ),0);
        m_library->Append( libobject );
    }

    {
        wxBitmap bitmap( smile_xpm );
        a2dImage* i=new a2dImage( bitmap.ConvertToImage(), 200,0,62,62 );

        a2dLibraryReference* libobject = new a2dLibraryReference(0,-200,i,_T("smilly"),100);
        libobject->SetStroke(wxColour(229,5,64 ),0);
        m_library->Append( libobject );
    }

    {
        wxBitmap bitmap2;
        wxPathList pathList;
        pathList.Add(_T("../common/images"));

        wxString path = pathList.FindValidPath(_T("image1.png"));

        a2dImage* im = new a2dImage( path,wxBITMAP_TYPE_PNG, -500,-265,382,332 );

        im->SetStroke(wxColour(229,255,244 ),40.0);
        im->SetFill(wxColour(0,117,220),a2dFILL_VERTICAL_HATCH);

        a2dLibraryReference* libobject = new a2dLibraryReference(0,-400,im,_T("milkyway"),100);
        libobject->SetStroke(wxColour(229,5,64 ),0);
        m_library->Append( libobject );
    }

    {
        a2dRect* rec3 = new a2dRect(0,0,50,70);
        rec3->SetFill(wxColour(0,12,240));
        rec3->SetStroke(wxColour(252,54,25 ),3.0);
        rec3->SetCanvasDocument( m_library );

        a2dLibraryReference* libobject = new a2dLibraryReference(0,-600,rec3,_T("rectangle"),100);
        libobject->SetStroke(wxColour(229,5,64 ),0);
        m_library->Append( libobject );
    }

    m_library->SetCanvasDocumentRecursive();
}
*/


void a2dEditorFrame::ShowLibs( wxCommandEvent& event )
{
    if ( event.GetId() == MSHOW_LIBPOINTS )
    {
        ConnectDocument( a2dGetCmdh()->GetLibraryPoints() );
        m_contr->Zoomout();
    }

}

void a2dEditorFrame::CreateStatusBarThemed( const wxString& themeName )
{
    CreateStatusBar( 6 );
    int widths[] = { 80, 120, -1, -1 , -1, -1 };
    SetStatusWidths( 6, widths );
}

void a2dEditorFrame::Theme( const wxString& themeName )
{
    if ( themeName == wxT( "test" ) )
        CreateThemeTest();
    if ( themeName == wxT( "default" ) )
        CreateThemeDefault();
}

void a2dEditorFrame::SetupToolbar()
{
    //toolbar = CreateToolBar(wxTB_HORIZONTAL|wxTB_FLAT|wxTB_TEXT |wxTB_3DBUTTONS);
    wxToolBar* toolbar = new wxToolBar( this, wxNewId(), wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_FLAT | wxTB_DOCKABLE );
    toolbar->SetMargins( 2, 2 );
    SetToolBar( toolbar );

#ifndef __UNIX__
    toolbar->SetToolBitmapSize( wxSize( 32, 32 ) );
#endif

    AddCmdToToolbar( CmdMenu_PushInto() );
    AddCmdToToolbar( CmdMenu_PopOut() );
    AddCmdToToolbar( CmdMenu_PushTool_Zoom() );
    AddCmdToToolbar( CmdMenu_PushTool_Cameleon() );
    AddCmdToToolbar( CmdMenu_PushTool_Port() );
    AddCmdToToolbar( CmdMenu_Selected_CameleonSymbol() );          
    AddCmdToToolbar( CmdMenu_Selected_CameleonDiagram() );
    AddCmdToToolbar( CmdMenu_PushTool_Cameleon() );
    AddCmdToToolbar( CmdMenu_PushTool_Origin_OneShot() );
    AddCmdToToolbar( CmdMenu_PushTool_CameleonInst() );
    AddCmdToToolbar( CmdMenu_PushTool_CameleonInstDlg() );             

    AddCmdToToolbar( CmdMenu_PushTool_DrawPolygonL() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawPolygonL_Splined() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawPolylineL() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawPolylineL_Splined() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawRectangle() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawCircle() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawEllipse() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawEllipticArc() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawEllipticArc_Chord() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawArc() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawArc_Chord() );
    AddCmdToToolbar( CmdMenu_PushTool_DrawText() );
    AddCmdToToolbar( CmdMenu_PushTool_Image() );
    AddCmdToToolbar( CmdMenu_PushTool_Link() );
    AddCmdToToolbar( CmdMenu_PushTool_Rotate() );
    AddCmdToToolbar( CmdMenu_PushTool_Drag() );
    AddCmdToToolbar( CmdMenu_PushTool_Copy() );
    AddCmdToToolbar( CmdMenu_PushTool_Delete() );
    AddCmdToToolbar( CmdMenu_PushTool_RecursiveEdit() );
    AddCmdToToolbar( CmdMenu_PushTool_Select() );
    AddCmdToToolbar( CmdMenu_PushTool_Select2() );
    AddCmdToToolbar( CmdMenu_Selected_SelectAll() );
    AddCmdToToolbar( CmdMenu_Selected_DeSelectAll() );
    AddCmdToToolbar( CmdMenu_Selected_Delete() );
    AddCmdToToolbar( CmdMenu_PushTool_DragMulti() );
    AddCmdToToolbar( CmdMenu_PushTool_CopyMulti() );

    toolbar->Realize();
}

void a2dEditorFrame::CreateThemeDefault()
{
    CreateStatusBarThemed( wxT( "default" ) );

    m_menuBar = new wxMenuBar;
    SetMenuBar( m_menuBar );

    //Centre(wxBOTH);

    //SETUP all menu dynamic, so can be removed too.
    wxMenu* file_menu = new wxMenu;
    AddCmdMenu( file_menu, CmdMenu_FileOpen() );
    AddCmdMenu( file_menu, CmdMenu_FileClose() );
    AddCmdMenu( file_menu, CmdMenu_FileSave() );
    AddCmdMenu( file_menu, CmdMenu_FileSaveAs() );
    AddCmdMenu( file_menu, CmdMenu_Print() );
    file_menu->AppendSeparator();
    AddCmdMenu( file_menu, CmdMenu_PrintView() );
    AddCmdMenu( file_menu, CmdMenu_PreviewView() );
    AddCmdMenu( file_menu, CmdMenu_PrintDocument() );
    AddCmdMenu( file_menu, CmdMenu_PreviewDocument() );
    AddCmdMenu( file_menu, CmdMenu_PrintSetup() );
    file_menu->AppendSeparator();

    wxMenu* file_import = new wxMenu;
    file_menu->Append( wxNewId(), wxT( "import file" ), file_import, _( "Import a file" ) );
    AddCmdMenu( file_import, CmdMenu_FileImport() );

    wxMenu* file_export_as = new wxMenu;
    file_menu->Append( wxNewId(), wxT( "Export As" ), file_export_as, _( "Export in other format" ) );
    AddCmdMenu( file_export_as, CmdMenu_FileExport() );

    file_export_as->AppendSeparator();
    AddCmdMenu( file_export_as, CmdMenu_ViewAsImage_Png() );
    AddCmdMenu( file_export_as, CmdMenu_ViewAsImage_Bmp() );
    AddCmdMenu( file_export_as, CmdMenu_DocumentAsImage_Png() );
    AddCmdMenu( file_export_as, CmdMenu_DocumentAsImage_Bmp() );
#if wxART2D_USE_SVGIO
    AddCmdMenu( file_export_as, CmdMenu_ViewAsSvg() );
#endif //wxART2D_USE_SVGIO
    file_menu->AppendSeparator();

    if ( m_isParentFrame )
        AddCmdMenu( file_menu, CmdMenu_Exit() );

    m_editMenu = new wxMenu;
    m_editMenu->Append( wxID_UNDO, _( "&Undo" ) );
    m_editMenu->Append( wxID_REDO, _( "&Redo" ) );

    Connect( MSHOW_FIRST, MSHOW_LAST, wxEVT_COMMAND_MENU_SELECTED, ( wxObjectEventFunction ) &a2dEditorFrame::ShowLibs );

    wxMenu* optionMenu = new wxMenu;
    AddFunctionToMenu( TOOLOPTION_ONTOP, optionMenu, _( "Drag &OnTop" ), _( "Drag on top of other objects" ), &a2dEditorFrame::SetToolDragingMode, true );
    AddFunctionToMenu( TOOLOPTION_RECTANGLE, optionMenu, _( "Drag &Rectangle" ), _( "Drag using a rectangle" ), &a2dEditorFrame::SetToolDragingMode, true );
    AddFunctionToMenu( TOOLOPTION_REDRAW, optionMenu, _( "Drag Re &Draw" ), _( "Drag and redraw all objects all the time" ), &a2dEditorFrame::SetToolDragingMode, true );
    AddFunctionToMenu( TOOLOPTION_COPY, optionMenu, _( "Drag &Copy" ), _( "Drag copy of original untill finished drag" ), &a2dEditorFrame::SetToolDragingMode, true );
    AddFunctionToMenu( TOOLOPTION_FILLED, optionMenu, _( "Drag/Draw Filled" ), _( "Drag/Draw filled objects" ), &a2dEditorFrame::SetToolDragingMode, true );
    AddFunctionToMenu( TOOLOPTION_WIRE, optionMenu, _( "Drag/Draw OutLine" ), _( "Drag/Draw an wireframe/outline of object" ), &a2dEditorFrame::SetToolDrawingMode, true );
    AddFunctionToMenu( TOOLOPTION_INVERT, optionMenu, _( "Drag/Draw Invert" ), _( "Drag/Draw an inverted wireframe of object" ), &a2dEditorFrame::SetToolDrawingMode, true );
    AddFunctionToMenu( TOOLOPTION_WIRE_ZERO_WIDTH, optionMenu, _( "Drag/Draw OutLine zero" ), _( "Drag/Draw zero width wireframe/outline of object" ), &a2dEditorFrame::SetToolDrawingMode, true );
    AddFunctionToMenu( TOOLOPTION_INVERT_ZERO_WIDTH, optionMenu, _( "Drag/Draw Invert zero" ), _( "Drag/Draw a zero width inverted wireframe of object" ), &a2dEditorFrame::SetToolDrawingMode, true );
    optionMenu->AppendSeparator();
    AddCmdMenu( optionMenu, CmdMenu_Option_SplinePoly() );
    AddCmdMenu( optionMenu, CmdMenu_Option_CursorCrosshair() );
    AddCmdMenu( optionMenu, CmdMenu_Option_RescaleAtReSize() );
    AddCmdMenu( optionMenu, CmdMenu_Option_ReverseLayers() );
    AddCmdMenu( optionMenu, CmdMenu_SetYAxis() );


    wxMenu* cameleonMenu = new wxMenu;
    AddCmdMenu( cameleonMenu,CmdMenu_ShowDlgCameleonModal() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_ShowDiagram() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_ShowSymbol() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_ShowGui() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_Selected_CameleonSymbol() );             
    AddCmdMenu( cameleonMenu,CmdMenu_Selected_CameleonDiagram() );             
    AddCmdMenu( cameleonMenu,CmdMenu_Selected_CloneCameleonFromInst() );             
    AddCmdMenu( cameleonMenu,CmdMenu_Selected_FlattenCameleon() );             
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_CameleonInst() );             
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_CameleonInstDlg() );             
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_Cameleon() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_Cameleon_BuildIn() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_Port() ); 
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_CameleonRefDlg() );         
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_CameleonRefDlg_Symbol() );         
    AddCmdMenu( cameleonMenu,CmdMenu_PushTool_CameleonRefDlg_Diagram() );         

    wxMenu* toolMenu = new wxMenu;

    AddCmdMenu( toolMenu, CmdMenu_PushTool_Select() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_RecursiveEdit() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_MultiEdit() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Drag() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Copy() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Rotate() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Delete() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawText() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Image() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Link() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_FollowLinkDocDrawCam() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawRectangle() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawCircle() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawLine() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawLineScaledArrow() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawEllipse() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawEllipticArc() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawEllipticArc_Chord() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawArc() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawArc_Chord() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawPolylineL() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawPolygonL() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Property() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DragMulti() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_CopyMulti() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Measure() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawWirePolylineL() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_DrawVPath() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Port() );
    AddCmdMenu( toolMenu, CmdMenu_PushTool_Cameleon() );

    toolMenu->AppendSeparator();
    AddCmdMenu( toolMenu, CmdMenu_RotateObject90Left() );
    AddCmdMenu( toolMenu, CmdMenu_RotateObject90Right() );
    AddCmdMenu( toolMenu, CmdMenu_LineBegin() );
    AddCmdMenu( toolMenu, CmdMenu_LineEnd() );
    AddCmdMenu( toolMenu, CmdMenu_LineScale() );
    AddCmdMenu( toolMenu, CmdMenu_SetNormalizeFactor() );

    //toolMenu->Append(MTOOL_PlaceFromLibByRef, _("add ref to object from lib"), _(""));
    //toolMenu->Append(MTOOL_PlaceFromLibByCopy, _("add object from lib"), _(""));


    wxMenu* drawMenu = new wxMenu;
    AddCmdMenu( drawMenu, CmdMenu_ShowDlgStructure() );
    AddFunctionToMenu( MSHOW_LIBPOINTS, drawMenu, _( "Point Library" ), _( "a library used for end and begin points of lines" ), &a2dEditorFrame::OnMenu );

    wxMenu* refMenu = new wxMenu;

    AddCmdMenu( refMenu, CmdMenu_Refresh() );
    AddCmdMenu( refMenu, CmdMenu_PushInto() );
    AddCmdMenu( refMenu, CmdMenu_NewGroup() );
    AddCmdMenu( refMenu, CmdMenu_NewPin() );

    wxMenu* performMenu = new wxMenu;
    wxMenu* selected = new wxMenu;
    wxMenu* groups = new wxMenu;

    /////////////////////////////////////////
    // selected like operation
    /////////////////////////////////////////
    performMenu->Append( wxNewId(), wxT( "selected objects" ), selected, _( "operation on selected objects" ) );

    AddCmdMenu( selected, CmdMenu_Selected_SelectAll() );
    AddCmdMenu( selected, CmdMenu_Selected_DeSelectAll() );
    AddCmdMenu( selected, CmdMenu_Selected_Delete() );
    AddCmdMenu( selected, CmdMenu_Selected_MoveXY() );
    AddCmdMenu( selected, CmdMenu_Selected_MoveLayer() );
    AddCmdMenu( selected, CmdMenu_Selected_CopyXY() );
    AddCmdMenu( selected, CmdMenu_Selected_CopyLayer() );
    AddCmdMenu( selected, CmdMenu_Selected_Transform() );
    AddCmdMenu( selected, CmdMenu_Selected_Rotate() );
    AddCmdMenu( selected, CmdMenu_Selected_RotateObject90Right() );
    AddCmdMenu( selected, CmdMenu_Selected_RotateObject90Left() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MinX() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MaxX() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MinY() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MaxY() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MidX() );
    AddCmdMenu( selected, CmdMenu_Selected_Align_MidY() );
    AddCmdMenu( selected, CmdMenu_Selected_Group() );
    AddCmdMenu( selected, CmdMenu_Selected_ImageToRectangles() );
    AddCmdMenu( selected, CmdMenu_Selected_ToTop() );
    AddCmdMenu( selected, CmdMenu_Selected_ToBack() );
    AddCmdMenu( selected, CmdMenu_Selected_SetStyle() );
    AddCmdMenu( selected, CmdMenu_Selected_SetExtStyle() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertToPolygonPolylinesWithArcs() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertToPolygonPolylinesWithoutArcs() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertToPolylines() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertLinesArcs() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertPolygonToArcs() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertPolylineToArcs() );
    AddCmdMenu( selected, CmdMenu_Selected_ConvertToVPaths() );
    AddCmdMenu( selected, CmdMenu_Selected_UnGroup() );
    AddCmdMenu( selected, CmdMenu_Selected_UnGroupDeep() );
    AddCmdMenu( selected, CmdMenu_Selected_Merge() );
    AddCmdMenu( selected, CmdMenu_Selected_Offset() );
    AddCmdMenu( selected, CmdMenu_Selected_CreateRing() );

    wxMenu* ungroupObjects  = new wxMenu;
    refMenu->Append( wxNewId(), wxT( "ungroup objects" ), ungroupObjects, _( "un-group selected objects" ) );
    AddCmdMenu( ungroupObjects, CmdMenu_Selected_UnGroup() );
    AddCmdMenu( ungroupObjects, CmdMenu_Selected_UnGroupDeep() );

#if wxART2D_USE_KBOOL
    /////////////////////////////////////////
    // group like operation
    /////////////////////////////////////////
    performMenu->Append( wxNewId(), wxT( "layer groups" ), groups, _( "operation on objects in layer groups" ) );
    AddCmdMenu( groups, CmdMenu_ShowDlgGroups() );
    AddCmdMenu( groups, CmdMenu_GroupAB_Offset() );
    AddCmdMenu( groups, CmdMenu_GroupAB_Smooth() );
    AddCmdMenu( groups, CmdMenu_GroupAB_CreateRing() );
    AddCmdMenu( groups, CmdMenu_GroupAB_Delete() );
    AddCmdMenu( groups, CmdMenu_GroupAB_Move() );
    AddCmdMenu( groups, CmdMenu_GroupAB_Copy() );
    AddCmdMenu( groups, CmdMenu_GroupAB_ToArcs() );

    wxMenu* booleanObjects  = new wxMenu;
    groups->Append( wxNewId(), wxT( "boolean objects" ), booleanObjects, _( "boolean group A-B objects" ) );

    //boolean operation based on group settings
    AddCmdMenu( booleanObjects, CmdMenu_GroupAB_Or() );
    AddCmdMenu( booleanObjects, CmdMenu_GroupAB_And() );
    AddCmdMenu( booleanObjects, CmdMenu_GroupAB_Exor() );
    AddCmdMenu( booleanObjects, CmdMenu_GroupAB_AsubB() );
    AddCmdMenu( booleanObjects, CmdMenu_GroupAB_BsubA() );

#endif //wxART2D_USE_KBOOL

    AddCmdMenu( performMenu, CmdMenu_EmptyDocument() );
    AddCmdMenu( performMenu, CmdMenu_EmptyShownObject() );

    wxMenu* menuSettings = new wxMenu;

    AddCmdMenu( menuSettings, CmdMenu_ShowDlgStyle() );
    AddCmdMenu( menuSettings, CmdMenu_SetFont() );
    AddCmdMenu( menuSettings, CmdMenu_SetBackgroundStyle() );
    AddCmdMenu( menuSettings, CmdMenu_DrawGridLines() );
    AddCmdMenu( menuSettings, CmdMenu_DrawGridAtFront() );
    AddCmdMenu( menuSettings, CmdMenu_DrawGrid() );
    AddCmdMenu( menuSettings, CmdMenu_SetSnap() );
    AddCmdMenu( menuSettings, CmdMenu_SetDrawWireFrame() );

    wxMenu* menuLayers = new wxMenu;

    wxMenu* savelayersLoc  = new wxMenu;
    menuLayers->Append( wxNewId(), wxT( "save layerfile" ), savelayersLoc, _( "save layer file to chosen location" ) );
    {
        AddCmdMenu( savelayersLoc, CmdMenu_SaveLayers() );
        AddCmdMenu( savelayersLoc, CmdMenu_SaveLayersHome() );
        AddCmdMenu( savelayersLoc, CmdMenu_SaveLayersLastDir() );
        AddCmdMenu( savelayersLoc, CmdMenu_SaveLayersCurrentDir() );
    }
    wxMenu* loadlayersLoc  = new wxMenu;
    menuLayers->Append( wxNewId(), wxT( "load layerfile" ), loadlayersLoc, _( "load layer file from chosen location" ) );
    {
        AddCmdMenu( loadlayersLoc, CmdMenu_LoadLayers() );
        AddCmdMenu( loadlayersLoc, CmdMenu_LoadLayersHome() );
        AddCmdMenu( loadlayersLoc, CmdMenu_LoadLayersLastDir() );
        AddCmdMenu( loadlayersLoc, CmdMenu_LoadLayersCurrentDir() );
    }
    AddCmdMenu( menuLayers, CmdMenu_SetLayerDlg() );
    AddCmdMenu( menuLayers, CmdMenu_SetLayerDlgModeless() );
    AddCmdMenu( menuLayers, CmdMenu_SetTargetDlg() );

    AddFunctionToMenu( Layer_view, menuLayers, _( "Show layer view" ), _( "layers of this document" ), &a2dEditorFrame::OnMenu, true );

    wxMenu* layerDlgInit  = new wxMenu;
    menuLayers->Append( wxNewId(), wxT( "LayerDlg" ), layerDlgInit, _( "Show LayerDlg" ) );
    AddCmdMenu( layerDlgInit, CmdMenu_ShowDlgLayersDocument() );
    AddCmdMenu( layerDlgInit, CmdMenu_ShowDlgLayersDrawing() );
    AddCmdMenu( layerDlgInit, CmdMenu_ShowDlgLayersGlobal() );

    wxMenu* orderDlgInit  = new wxMenu;
    menuLayers->Append( wxNewId(), wxT( "OrderDlg" ), orderDlgInit, _( "Show OrderDlg" ) );
    AddCmdMenu( orderDlgInit, CmdMenu_ShowDlgLayerOrderDocument() );
    AddCmdMenu( orderDlgInit, CmdMenu_ShowDlgLayerOrderGlobal() );

    wxMenu* dlg_menu = new wxMenu;

    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgPathSettings() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgSettings() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgTrans() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgTools() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgStyle() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgSnap() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgStructure() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgIdentify() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgMeasure() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgCoordEntry() );

    //AddCmdMenu( dlg_menu, CmdMenu_InsertGroupRef() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgLayersDocument() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgGroups() );
    AddCmdMenu( dlg_menu, CmdMenu_ShowDlgPropEdit() );

#if defined(_DEBUG)
    wxMenu* drawTest = new wxMenu;
    drawTest->Append( MSCREEN_T, wxT( "test screen" ), wxT( "whatever" ) );
#endif

    wxMenu* help_menu = new wxMenu;
    AddFunctionToMenu( wxID_ABOUT, help_menu, _( "&About" ), _( "&About" ), &a2dEditorFrame::OnAbout );

    m_menuBar->Append( file_menu, _( "&File" ) );
    m_menuBar->Append( m_editMenu, _( "&Edit" ) );
    m_menuBar->Append( optionMenu, _( "&Options" ) );
    m_menuBar->Append( cameleonMenu,  _( "&Cameleons" ) );
    m_menuBar->Append( toolMenu, _( "&Tools" ) );
    m_menuBar->Append( drawMenu, _( "&Show" ) );
    m_menuBar->Append( refMenu, _( "&Action" ) );
    m_menuBar->Append( performMenu, _( "&Perform Operation" ) );
    m_menuBar->Append( menuSettings, _( "Settings" ) );
    m_menuBar->Append( menuLayers, _( "Layers" ) );
    m_menuBar->Append( dlg_menu, _( "&Dialogs" ) );
#if defined(_DEBUG)
    m_menuBar->Append( drawTest, _( "&Test" ) );
#endif
    m_menuBar->Append( help_menu, _( "&Help" ) );

    SetupToolbar();

    //FillLibraryPoints();

}

void a2dEditorFrame::CreateThemeXRC()
{
    CreateStatusBarThemed( wxT( "default" ) );

    m_menuBar = new wxMenuBar;
    SetMenuBar( m_menuBar );

#if wxUSE_XRC
    if( wxXmlResource::Get() )
    {
        wxXmlResource::Get()->InitAllHandlers();
        wxXmlResource::Get()->AddHandler( new a2dMenuBarXmlHandlerDoc );
        wxXmlResource::Get()->AddHandler( new a2dMenuXmlHandlerDoc );
        wxXmlResource::Get()->AddHandler( new a2dToolBarXmlHandlerDoc );

        wxString art2d = a2dGeneralGlobals->GetWxArt2DVar( true );
        art2d += wxT( "art/resources/" );

//      wxString aResName = wxString::Format(wxT("../resource/%s.xrc"),GetAppName().c_str());
        wxString aResName = wxT( "a2dEditorFrame.xrc" );
        if( !wxXmlResource::Get()->Load( art2d + aResName ) )
            wxMessageBox( wxString::Format( _( "Error load XRC-file '%s'.\nWorking directory is '%s'" ), aResName.c_str(), wxGetCwd().c_str() ) );
    }

    wxXmlResource::Get()->LoadObject( this, wxT( "frameMenu" ), wxT( "a2dMenuBar" ) );
    wxXmlResource::Get()->LoadObject( this, wxT( "toolbar" ), wxT( "a2dToolBar" ) );

    m_editMenu = m_menuBar->GetMenu( m_menuBar->FindMenu( wxT("Edit") ) );
#endif
}

void a2dEditorFrame::CreateThemeTest()
{
    /*
        wxString path = pathList.FindValidPath("pat36.bmp");
        wxBitmap mono;
        mono.LoadFile(path, wxBITMAP_TYPE_BMP);
        wxMask* mask25 = new wxMask(mono, *wxWHITE);
        // associate a monochrome mask with this bitmap
        mono.SetMask(mask25);

        a2dPatternFill* pFill = new a2dPatternFill( mono );
        pFill->SetStyle(a2dFILL_STIPPLE_MASK_OPAQUE);
    */

    CreateThemeDefault();
    //FillLibrary();
}

void a2dEditorFrame::OnUpdateUI( wxUpdateUIEvent& event )
{
}

void a2dEditorFrame::FillData( wxCommandEvent& event )
{
    a2dCanvasObject* datatree = m_drawingPart->GetShowObject();

    a2dRect* tr = new a2dRect( -500, 750, 900, 80 );
    tr->SetStroke( wxColour( 229, 5, 64 ), 0 );
    tr->SetFill( wxColour( 0, 117, 245 ) );
    datatree->Append( tr );

    a2dCanvasObjectReference* ref1 = new a2dCanvasObjectReference( 1500, -500,  tr );
//    ref1->SetScale(2,3.2);
    ref1->SetRotation( 35 );
    datatree->Append( ref1 );


    a2dText* tt = new a2dText( _T( "Hello NON rotated text in a2dCanvas World" ), -500, 750,
                               a2dFont( 80.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ) );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( *wxGREEN );
    datatree->Append( tt );

    a2dText* tt2 = new a2dText( _T( "Hello NON rotated \ntext in\n a2dCanvas World" ), -500, 550,
                                a2dFont( 50.0 , wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ) );
    tt2->SetFill( *a2dTRANSPARENT_FILL );
    tt2->SetStroke( *wxGREEN );
    datatree->Append( tt2 );


    a2dArc* aca2 = new  a2dArc( -630.0, 350.0, -730.0, -200.0, -730.0, 250.0 );
    aca2->SetFill( *a2dTRANSPARENT_FILL );
    aca2->SetStroke( wxColour( 1, 215, 6 ), 10, a2dSTROKE_SOLID );
    datatree->Prepend( aca2 );


    m_drawingPart->SetMappingShowAll();
}

void a2dEditorFrame::OnDragSimulate(wxCommandEvent& event)
{
    a2dDragTool* drag = new a2dDragTool( m_contr );
    drag->SetDropAndDrop( true );
    m_contr->PushTool( drag );

}

void a2dEditorFrame::OnCopyShape(wxCommandEvent& WXUNUSED(event))
{
    a2dDrawingPart* part = m_drawingPart;
    a2dDnDCanvasObjectDataCVG* copied = new a2dDnDCanvasObjectDataCVG( part);
    copied->SetExportWidthHeightImage( 10000 );
    //a2dDnDCameleonData* copied = new a2dDnDCameleonData();

    if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
    {
        a2dCanvasObjectList* objects = part->GetShowObject()->GetChildObjectList();
        forEachIn( a2dCanvasObjectList, objects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj->GetRelease() || !obj->IsVisible()  || !obj->GetSelected() )
                continue;
            copied->Append( obj );
        }
        copied->Reconnect();

        wxClipboardLocker clipLocker;
        if ( !clipLocker )
        {
            wxLogError(wxT("Can't open the clipboard"));

            return;
        }

        wxTheClipboard->AddData( copied );
    }
}

void a2dEditorFrame::OnPasteShape(wxCommandEvent& WXUNUSED(event))
{
    wxClipboardLocker clipLocker;
    if ( !clipLocker )
    {
        wxLogError(wxT("Can't open the clipboard"));
        return;
    }

    a2dDrawingPart* part = m_drawingPart;

    if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
    {
        a2dDnDCanvasObjectDataCVG shapeDataObject( part);
        //a2dDnDCameleonData shapeDataObject(NULL);
        shapeDataObject.SetImportWidthHeightImage( 10000 );
        if ( wxTheClipboard->GetData(shapeDataObject) )
        {
            a2dDrawingPtr drawing = shapeDataObject.GetDrawing();
            a2dCanvasObjectPtr shape = drawing->GetRootObject();
    	    a2dCanvasObject* root = part->GetDrawing()->GetRootObject();

            a2dWalker_SetSpecificFlagsCanvasObjects setflags2( a2dCanvasOFlags::SELECTED | a2dCanvasOFlags::SELECTED2 );
            setflags2.SetSkipNotRenderedInDrawing( true );
            setflags2.Start( part->GetShowObject(), false );
            shape->Update( a2dCanvasObject::updatemask_force );
            shape->SetRoot( part->GetDrawing() );
            a2dBoundingBox box = shape->GetBbox();
            a2dPoint2D p = box.GetCentre();
            shape->Translate( -p.m_x, -p.m_y );
            shape->EliminateMatrix();
            
            // to test
            //root->Append( shape );

            if ( shape->GetChildObjectList()->size() )
            {
                a2dDragMultiNewTool* dragnew = new a2dDragMultiNewTool( (a2dStToolContr *) (part->GetCanvasToolContr()), shape->GetChildObjectList() );
                dragnew->SetOneShot();
                dragnew->SetStroke( a2dStroke( *wxRED, 1 ) );
                dragnew->SetFill( a2dFill( wxColour( 0xc0, 0xff, 0xff ) ) );
                (a2dStToolContr *) (part->GetCanvasToolContr())->PushTool( dragnew );
            }            
            else
            {
                wxLogStatus(wxT("No shape on the clipboard"));
            }
        }
        else
        {
            wxLogStatus(wxT("No shape on the clipboard"));
        }
    }
}

void a2dEditorFrame::OnUpdatePasteShape( wxUpdateUIEvent& event  )
{
    a2dDrawingPart* part = m_drawingPart;
    a2dDnDCanvasObjectDataCVG shapeDataObject( part);
    wxDataFormat formats[10];
    shapeDataObject.GetAllFormats( formats );
    int nr = shapeDataObject.GetFormatCount();

    bool ret = false;
        
    for ( int i = 0 ; i < nr ; i++ )
    {
        ret |= wxTheClipboard->IsSupported( formats[i] );
    }

    event.Enable( ret );
}

void a2dEditorFrame::OnUpdateCopyShape( wxUpdateUIEvent& event  )
{
    bool ret = false;
    a2dDrawingPart* part = m_drawingPart;
    if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
    {
        a2dCanvasObjectList* objects = part->GetShowObject()->GetChildObjectList();
        forEachIn( a2dCanvasObjectList, objects )
        {
            a2dCanvasObject* obj = *iter;
            if ( obj->GetRelease() || !obj->IsVisible()  || !obj->GetSelected() )
                continue;
            ret = true;
            break;
        }
    }
    event.Enable( ret );
}
