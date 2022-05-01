/*! \file curves/samples/curtuto1/curtuto1.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: curtuto1.cpp,v 1.9 2009/09/26 19:01:19 titato Exp $
*/

#include "a2dprec.h"
#include "wx/editor/candocproc.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "curtuto1.h"

#if wxUSE_XRC
#include <wx/xrc/xmlres.h>
#include <wx/docview/xh_a2dmenudoc.h>
#endif

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#include "wx/docview/docviewref.h"

#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#endif //wxART2D_USE_AGGDRAWER

static double gc_defmaxX = 480; // window default max X
static double gc_defmaxY = 100; // window default max Y
static int gc_defScreenmaxX = 960; // screen window default max X
static int gc_defScreenmaxY = 500; // screen window default max Y
static const double gc_plotBoundLeftK = 0.01;
static const double gc_plotBoundRightK = 0.01;
static const double gc_plotBoundUpK = 0.01;
static const double gc_plotBoundDownK = 0.01;

//#if wxUSE_PRINTING_ARCHITECTURE
//static wxPageSetupData* g_pageSetupData = NULL;
//#endif

IMPLEMENT_CLASS( PlotCanvas, a2dCanvas )

BEGIN_EVENT_TABLE( PlotCanvas, a2dCanvas )
    EVT_SIZE( PlotCanvas::OnSize )
    EVT_IDLE( PlotCanvas::OnIdle )
    EVT_CHAR( PlotCanvas::OnChar )
//  EVT_UPDATE_VIEWS( PlotCanvas::OnUpdateViews )
END_EVENT_TABLE()

// Define a constructor for my canvas
PlotCanvas::PlotCanvas(  wxFrame* frame, const wxPoint& pos, const wxSize& size, const long style ):
    a2dCanvas( new PlotPart(), frame, -1, pos, size, style )
//  a2dCanvas( frame, -1, pos, size, style)
{
    m_DoShowAllAuto = false; 

    m_startup = true;
    m_documentNr = 0;
}

void PlotCanvas::DoSetMappingShowAtAll( int clientw, int clienth, bool centre, int recursive )
{
    wxUint16 border = m_drawingPart->GetZoomOutBorder();

    a2dBoundingBox untr = GetShowObject()->GetBbox();
    a2dAffineMatrix cworld = GetShowObject()->GetTransformMatrix();
    cworld.Invert();
    untr.MapBbox( cworld );

    double w = untr.GetWidth();
    double h = untr.GetHeight();

    if ( w == 0 )
        w = 1000;
    if ( h == 0 )
        h = 1000;

    double uppx, uppy;
    //what is needed in units per pixel to make height of bbox fit.
    if ( clientw - border > 0 )
        uppx = w / ( clientw - border );
    else
        uppx = w / clientw;

    //what is needed in units per pixel to make width of bbox fit.
    if ( clienth - border > 0 )
        uppy = h / ( clienth - border );
    else
        uppy = h / clienth;


    if ( !centre )
    {
        SetMappingWidthHeight( untr.GetMinX(),
                               untr.GetMinY(),
                               w,
                               h,
                               false
                             );
    }
    else
    {
        double middlexworld = untr.GetMinX() + w / 2.0;
        double middleyworld = untr.GetMinY() + h / 2.0;
        GetDrawer2D()->SetMappingUpp( middlexworld - clientw / 2.0 * uppx, middleyworld - clienth / 2.0 * uppy, uppx, uppy );
    }

    GetDrawingPart()->UpdateViewDependentObjects();
    GetDrawingPart()->GetDrawing()->GetRootObject()->Update( a2dCanvasObject::updatemask_normal );

    if( recursive )
    {
        const a2dBoundingBox& untr2 = GetShowObject()->GetBbox();
        if( fabs( w - untr2.GetWidth() ) > uppx || fabs( h - untr.GetHeight() ) > uppy )
            DoSetMappingShowAtAll( clientw, clienth, centre, recursive - 1 );
        else
            recursive = 0; // this line for debugging
    }
}

void PlotCanvas::SetMappingShowAtAll( bool centre )
{
    if ( !GetDrawingPart() || !GetShowObject() )
        return;

    int clientw, clienth;
    GetSaveClientSize( &clientw, &clienth );
    GetDrawer2D()->StartRefreshDisplayDisable();
    GetDrawer2D()->SetMappingDeviceRect( 0, 0, clientw, clienth );

    DoSetMappingShowAtAll( clientw, clienth, centre, 5 ); // last param is recursion counter

    GetDrawer2D()->EndRefreshDisplayDisable();
    GetDrawingPart()->Update( a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );

    //tell root there or no more pending objects.
    GetDrawingPart()->GetDrawing()->SetUpdatesPending( false );
}

void PlotCanvas::OnSizeAsWXCanvas( wxSizeEvent& WXUNUSED( event ) )
{
    if ( !GetDrawingPart() )
        return;

    //TRICKS!
    //The buffer is a little bigger then the clientsize.
    //This is for two reasons.
    //1- A small resize on the window does not result in redrawing
    //everything.
    //1- To be able to ignore onsize events when only scrollbars (dis)appear
    //which also would cause a redraw here

    //Setting scrollbars (after a zoom in or out) normally can result
    //in 1 or 2 Onsize events, since scrollbars can (dis)appear.
    //This actually changes the clientsize, so Onsize events.
    //to prevent in this case redrawing, those onsize events are
    //ignored. The Onpaint events resulting from setting scrollbars or
    //processed as normal, so the buffer size must be at least with scrollbars.

    //ignore size events due to scrollbars on off,
    //while window itself did not change in size
    //Those changes are detected when setting the scrollbars
    //Two separate size events or generated, so handle them one
    //by one
    /*
    if (m_xscroll_changed == true)
    {
    m_xscroll_changed = false;
    return;
    }
    if (m_yscroll_changed == true)
    {
    m_yscroll_changed = false;
    return;
    }
    */
    int clientw, clienth;
    int w, h;

    GetSaveClientSize( &clientw, &clienth );
    //the client size may be with are without scrollbars
    //always draw to a buffer that is inclusif scrollbars
    w = clientw + m_delta;
    if ( m_has_y_scroll )
        w += wxSystemSettings::GetMetric( wxSYS_VSCROLL_X );
    h = clienth + m_delta;
    if ( m_has_x_scroll )
        h += wxSystemSettings::GetMetric( wxSYS_HSCROLL_Y );

    if ( abs( m_oldw - w ) > m_delta || abs( m_oldh - h ) > m_delta )
    {
        GetDrawingPart()->SetBufferSize( w, h );

        //first redraw/update that were not yet updated pending areas (if available)
        //blit pending updates to the window/screen
        //              m_view->Update( a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );

        // ...

        //also used for rescale!
        m_oldw = w; m_oldh = h;
    }
    m_prevw = w;
    m_prevh = h;
}


void PlotCanvas::OnSize( wxSizeEvent& event )
{
    if( ( m_documentNr == 8 || m_documentNr == 9 || m_documentNr == 11 ) && GetDrawingPart() && GetShowObject() )
    {
        int clientw, clienth;
        GetSaveClientSize( &clientw, &clienth );
        wxSize size( clientw, clienth );

        const double defaulttic = 25;
        double atic = defaulttic;

        int koeff = ( int ) ( ( ( gc_defmaxX / ( double )size.GetWidth() ) + 1.0 ) - ( atic / defaulttic ) );
        if( koeff > 0 )
            atic *= koeff + 1;
        if( gc_defmaxX / size.GetWidth() >= 2 )
            atic = gc_defmaxX;

        a2dCanvasObject* aRoot = GetShowObject();
        if( aRoot )
        {
            forEachIn( a2dCanvasObjectList, aRoot->GetChildObjectList() )
            {
                a2dCanvasObject* obj = *iter;
                a2dPlot* plot = wxDynamicCast( obj, a2dPlot );
                if( plot )
                {
                    int numXLines = size.GetWidth() / ( gc_defScreenmaxX / 10 );
                    if( numXLines == 3 )
                        numXLines = 2;
                    else if( numXLines > 5 )
                        numXLines = ( numXLines / 5 ) * 5;
                    a2dCurveAxis* axisX = plot->GetAxisX();

                    const a2dBoundingBox& curRect = plot->GetCurvesAreaList().GetBaseCurvesArea()->GetInternalBoundaries();
                    double curWidth = curRect.GetWidth();
                    double meanX = curRect.GetMinX() + curWidth / 2.;
                    double newWidth = a2dTicsRound( curWidth / ( double )numXLines ) * numXLines;
                    double aTic = newWidth / ( double )numXLines;
                    wxASSERT( aTic > 1E-12 );
                    double newMeanX = int( ( meanX / aTic ) + ( meanX < 0. ? -0.5 : 0.5 ) ) * aTic;
                    double newMinX = newMeanX - newWidth / 2.;
                    double newMaxX = newMinX + newWidth;

                    plot->SetInternalXBoundaries( newMinX, newMaxX );
                    axisX->SetTic( aTic );
                    plot->SetAxisTicPrecision( axisX, newMinX, newMaxX );


//                    double aticX = a2dBoundsRound(fabs(axisX->GetBoundaryMax()-axisX->GetBoundaryMin()) / (double)numXLines /2.);
//                  plot->GetAxisX()->SetTic( aticX ); // non-inverted axis x
                    //plot->GetAxisX()->SetTic( -atic ); // inverted axis x

                    int numLines = size.GetHeight() / ( gc_defScreenmaxY / 3 );
                    if( numLines == 3 )
                        numLines = 2;
                    else if( numLines == 4 )
                        numLines = 5;
                    else if( numLines > 5 )
                        numLines = ( numLines / 5 ) * 5;
                    plot->SetTicY( numLines * 2 );
                }
            }
        }
        GetDrawingPart()->Update( a2dCanvasObject::updatemask_normal );
        OnSizeAsWXCanvas( event );
        SetMappingShowAtAll();
    }
    else
    {
        a2dCanvas::OnSize( event );
        //klion maybe in another derived classes
        // that don't call a2dCanvas::OnSize()
        // then they must call
        // GetCanvasView()->UpdateViewDependentObjects();

        if ( m_DoShowAllAuto )
            SetMappingShowAll();

        //For always seeing everything
        //m_contr->Zoomout();

        //For having a fixed Lower Left Corner.
        //SetMappingUpp(-150,-1000,GetUppX(),GetUppY());

        //For having the same Lower Left Corner
        //SetMappingUpp( GetVisibleMinX() ,GetVisibleMinY(),GetUppX(),GetUppY());
    }
}

void PlotCanvas::OnIdle( wxIdleEvent& WXUNUSED( event ) )
{
    if ( m_startup )
    {
        m_startup = false;
        SetMappingShowAll();
    }
}

void PlotCanvas::OnChar( wxKeyEvent& event )
{
    if( m_documentNr == 11 || m_documentNr == 9 || m_documentNr == 2 || m_documentNr == 10 )
    {
        a2dCanvasObject* aRoot = GetShowObject();
        if( aRoot )
        {
            forEachIn( a2dCanvasObjectList, aRoot->GetChildObjectList() )
            {
                a2dCanvasObject* obj = *iter;
                a2dPlot* plot = wxDynamicCast( obj, a2dPlot );
                if( plot )
                {
                    switch( event.GetKeyCode() )
                    {
                        case WXK_LEFT:
                            plot->ChangeCursorPos( -1 );
                            break;
                        case WXK_PAGEDOWN:
                            //case WXK_PAGEUP:
                            plot->ChangeCursorPos( -10 );
                            break;
                        case WXK_RIGHT:
                            plot->ChangeCursorPos( 1 );
                            break;
                        case WXK_PAGEUP:
                            //case WXK_PAGEDOWN:
                            plot->ChangeCursorPos( 10 );
                            break;
                        case WXK_HOME:
                            plot->ChangeCursorPos( -1024 );
                            break;
                        case WXK_END:
                            plot->ChangeCursorPos( 1024 );
                            break;
                        default:
                            event.Skip();
                    }
                    break;
                }
            }
        }

    }
    else
        event.Skip();
}

/*
void PlotCanvas::OnUpdateViews(a2dDocumentEvent& event)
{
    int canvasSizeX, canvasSizeY;
    GetClientSize(&canvasSizeX,&canvasSizeY);
    wxSizeEvent szEvt(wxSize(canvasSizeX, canvasSizeY));
    ProcessEvent(szEvt);
}
*/

IMPLEMENT_CLASS( PlotPart, a2dDrawingPartTiled )

BEGIN_EVENT_TABLE( PlotPart, a2dDrawingPart )
    EVT_UPDATE_DRAWING( PlotPart::OnUpdate )
END_EVENT_TABLE()

PlotPart::PlotPart()
    : a2dDrawingPartTiled()
{
    m_alreadyUpdating = false;
}

PlotPart::~PlotPart()
{
}

void PlotPart::OnUpdate( a2dDrawingEvent& event )
{
    PlotCanvas* aPlotWin = wxDynamicCast( GetCanvas(), PlotCanvas );
    if( aPlotWin && GetViewDependentObjects() && ( aPlotWin->m_documentNr == 8 || aPlotWin->m_documentNr == 9 || aPlotWin->m_documentNr == 11 ) )
    {
        if( ( event.GetUpdateHint()&a2dCANVIEW_UPDATE_VIEWDEPENDENT_RIGHTNOW ) && !m_alreadyUpdating )
        {
            m_alreadyUpdating = true;
            int canvasSizeX, canvasSizeY;
            aPlotWin->GetClientSize( &canvasSizeX, &canvasSizeY );
            wxSizeEvent szEvt( wxSize( canvasSizeX, canvasSizeY ) );
            aPlotWin->OnSize( szEvt );
            m_alreadyUpdating = false;
        }
    }
    a2dDrawingPartTiled::OnUpdate( event );
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------


BEGIN_EVENT_TABLE( MyFrame,    a2dDocumentFrame )
    EVT_CLOSE( MyFrame::OnCloseWindow )
#if wxUSE_XRC
//doesn't work  EVT_MENU_RANGE(XRCID(wxT("DOCUMENT_0"), XRCID(wxT("AXISX_SWITCH")), MyFrame::FillData)
    EVT_MENU( XRCID( "DOCUMENT_0" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_1" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_2" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_3" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_4" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_5" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_6" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_7" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_8" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_9" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_10" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_11" ), MyFrame::FillData )
    EVT_MENU( XRCID( "DOCUMENT_12" ), MyFrame::FillData )
#if wxART2D_USE_AGGDRAWER
    EVT_MENU( XRCID( "DRAWER_CHANGE" ), MyFrame::OnDrawer )
#endif //wxART2D_USE_AGGDRAWER
    EVT_MENU( XRCID( "CURSOR_ONLY_BY_CURVES" ), MyFrame::OnCursorOnlyByCurves )
    EVT_MENU( XRCID( "AXISX_INVERT" ), MyFrame::OnInvertXAxis )
    EVT_MENU( XRCID( "AXISY_INVERT" ), MyFrame::OnInvertYAxis )
    EVT_MENU( XRCID( "AXISX_SWITCH" ), MyFrame::OnSwitchXAxis )

    EVT_MENU( XRCID( "wxID_EXIT" ), MyFrame::OnExit )
    EVT_MENU( XRCID( "REFRESH" ), MyFrame::Refresh )
    EVT_MENU( XRCID( "SET_CURVEZOOM" ), MyFrame::PushCurveTool )
    EVT_MENU( XRCID( "SET_POLYTOOL" ), MyFrame::PushPolygonTool )
    EVT_MENU( XRCID( "SET_RECTTOOL" ), MyFrame::PushRectTool )
    EVT_MENU( XRCID( "SET_EDITTOOL" ), MyFrame::PushEditTool )
    EVT_MENU( XRCID( "SET_TEXTTOOL" ), MyFrame::PushTextTool )
    EVT_MENU( XRCID( "SET_SELECTTOOL" ), MyFrame::PushSelectTool )
    EVT_MENU( XRCID( "SET_DELETETOOL" ), MyFrame::PushDeleteTool )
    EVT_MENU( XRCID( "END_TOOL" ), MyFrame::EndTool )
    EVT_MENU( XRCID( "DOCCANVAS_ABOUT" ), MyFrame::OnAbout )

    EVT_MENU( XRCID( "wxID_OPEN" ), MyFrame::OnFileOpen )
    EVT_MENU( XRCID( "wxID_SAVE" ), MyFrame::OnFileSave )
    EVT_MENU( XRCID( "wxID_SAVEAS" ), MyFrame::OnFileSaveAs )
    EVT_MENU( XRCID( "SAVE_SVG" ), MyFrame::OnSvgSave )
    EVT_MENU( XRCID( "SAVE_BITMAP" ), MyFrame::OnBitmapSave )
    EVT_MENU( XRCID( "SAVE_ANYBITMAP" ), MyFrame::OnAnyBitmapSave )
    EVT_MENU( XRCID( "SAVE_ANYBITMAP_IOH" ), MyFrame::OnAnyBitmapSaveIoHandler )

#else
    EVT_MENU_RANGE( DOCUMENT_FIRST, DOCUMENT_LAST, MyFrame::FillData )
    EVT_MENU( wxID_EXIT, MyFrame::OnExit )
    EVT_MENU( REFRESH, MyFrame::Refresh )
    EVT_MENU( SET_CURVEZOOM, MyFrame::PushCurveTool )
    EVT_MENU( SET_POLYTOOL, MyFrame::PushPolygonTool )
    EVT_MENU( SET_RECTTOOL, MyFrame::PushRectTool )
    EVT_MENU( SET_EDITTOOL, MyFrame::PushEditTool )
    EVT_MENU( SET_TEXTTOOL, MyFrame::PushTextTool )
    EVT_MENU( SET_SELECTTOOL, MyFrame::PushSelectTool )
    EVT_MENU( SET_DELETETOOL, MyFrame::PushDeleteTool )
    EVT_MENU( END_TOOL, MyFrame::EndTool )
    EVT_MENU( DOCCANVAS_ABOUT, MyFrame::OnAbout )

    EVT_MENU( wxID_OPEN, MyFrame::OnFileOpen )
    EVT_MENU( wxID_SAVE, MyFrame::OnFileSave )
    EVT_MENU( wxID_SAVEAS, MyFrame::OnFileSaveAs )
    EVT_MENU( SAVE_SVG, MyFrame::OnSvgSave )
    EVT_MENU( SAVE_BITMAP, MyFrame::OnBitmapSave )
    EVT_MENU( SAVE_ANYBITMAP, MyFrame::OnAnyBitmapSave )
    EVT_MENU( SAVE_ANYBITMAP_IOH, MyFrame::OnAnyBitmapSaveIoHandler )

#if wxART2D_USE_AGGDRAWER
    EVT_MENU( DRAWER_CHANGE, MyFrame::OnDrawer )
#endif //wxART2D_USE_AGGDRAWER
    EVT_MENU( CURSOR_ONLY_BY_CURVES, MyFrame::OnCursorOnlyByCurves )
    EVT_MENU( AXISX_INVERT, MyFrame::OnInvertXAxis )
    EVT_MENU( AXISY_INVERT, MyFrame::OnInvertYAxis )
    EVT_MENU( AXISX_SWITCH, MyFrame::OnSwitchXAxis )
#endif // else wxUSE_XRC

    EVT_TIMER( -1, MyFrame::OnTimer )
END_EVENT_TABLE()


MyFrame::MyFrame( wxFrame* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    a2dDocumentFrame( true, parent, NULL, -1, title, position, size, style )
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    , m_initCurrentSmartPointerOwner( this )
#endif

{
//  m_alreadyInProcessEvent = NULL;
    m_plotmousemenu = NULL;
    m_timer = NULL;

    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText(  _T( "Welcome to a2dCanvas sample!" ) );

    SetIcon( wxICON( mondrian ) );

    m_canvas = new PlotCanvas( this, wxDefaultPosition, size, 0 );

#if wxART2D_USE_AGGDRAWER
    m_canvas->GetDrawingPart()->SetDrawer2D( new a2dAggDrawer( size ) );
#else
    m_canvas->GetDrawingPart()->SetDrawer2D( new a2dMemDcDrawer( size ) );
#endif

    //to always show full canvas after a resize fo window, use next.
    m_canvas->m_DoShowAllAuto = true;

    //this is to keep the middle of the window the same, but the drawing is scaled
    //to the new width or height.
    m_canvas->SetScaleOnResize( true );
    m_canvas->SetYaxis( true );

    FillDataDocNr( 3 );
}

MyFrame::~MyFrame()
{
    m_canvas->GetDrawing()->Modify( false );
    m_canvas->GetDrawingPart()->SetShowObject( NULL );
    m_drawing = NULL;
    delete m_timer;
    if( m_plotmousemenu )
        delete m_plotmousemenu;
}

void MyFrame::CreateMyMenuBar()
{
#if 0 //wxUSE_XRC
    if( !wxXmlResource::Get() )
        return ;

    wxXmlResource::Get()->LoadObject( this, wxT( "frameMenu" ), wxT( "a2dMenuBar" ) );
//  wxMenuBar* menu_bar =
//      wxXmlResource::Get()->LoadMenuBar(this,wxT("frameMenu"));
//  if(menu_bar)
//  {
    /*
            int pos = menu_bar->FindMenu(_("File"));
            if(wxNOT_FOUND != pos)
            {
                wxMenu* _fileMenu = menu_bar->GetMenu(pos);
                if(_fileMenu)
                    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileHistoryUseMenu(_fileMenu);
            }
    */
//      SetMenuBar( menu_bar );
//  }
#else
    wxMenu* file_menu = new wxMenu;

    wxMenuBar* menu_bar = new wxMenuBar();

    file_menu->Append( wxID_OPEN, _T( "&Open..." ) );
    file_menu->Append( wxID_SAVE, _T( "&Save" ) );
    file_menu->Append( wxID_SAVEAS, _T( "Save &As..." ) );
    file_menu->Append( XRCID( "SAVE_SVG" ), _T( "Save to &Svg" ) );
    file_menu->Append( XRCID( "SAVE_BITMAP" ), _T( "Save &Jpeg Bitmap" ) );
    file_menu->Append( XRCID( "SAVE_ANYBITMAP" ), _T( "Save As Any &Bitmap" ) );
    file_menu->Append( XRCID( "SAVE_ANYBITMAP_IOH" ), _T( "Save As Bitmap via IOHandler" ) );

    AddCmdMenu( file_menu, CmdMenu_FileNew() );
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
    file_menu->Append( wxID_EXIT, _T( "E&xit" ) );

    m_edit_menu = new wxMenu;
    m_edit_menu->Append( XRCID( "REFRESH" ), _T( "&refresh" ) );
    m_edit_menu->Append( XRCID( "SET_CURVEZOOM" ), _T( "&PushCurveZoomtool" ) );
    m_edit_menu->Append( XRCID( "SET_POLYTOOL" ), _T( "&PushPolygontool" ) );
    m_edit_menu->Append( XRCID( "SET_RECTTOOL" ), _T( "&PushRecttool" ) );
    m_edit_menu->Append( XRCID( "SET_EDITTOOL" ), _T( "&PushEdittool" ) );
    m_edit_menu->Append( XRCID( "SET_TEXTTOOL" ), _T( "&PushTexttool" ) );
    m_edit_menu->Append( XRCID( "SET_DELETETOOL" ), _T( "&PushDeletetool" ) );
    m_edit_menu->Append( XRCID( "SET_SELECTTOOL" ), _T( "&PushSelecttool" ) );
    m_edit_menu->Append( XRCID( "END_TOOL" ), _T( "&popTool" ) );

    wxMenu* drawMenu = new wxMenu;
    drawMenu->Append( XRCID( "DOCUMENT_0" ), _T( "document0" ) , _T( "" ) );
    drawMenu->Append( XRCID( "DOCUMENT_1" ), _T( "document1" ) , _T( "" ) );
    drawMenu->Append( XRCID( "DOCUMENT_2" ), _T( "document2" ) , _T( "" ) );
    drawMenu->Append( XRCID( "DOCUMENT_3" ), _T( "document3" ) , _T( "" ) );
    drawMenu->Append( XRCID( "DOCUMENT_4" ), _T( "document4" ) , _T( "" ) );
    drawMenu->Append( XRCID( "DOCUMENT_5" ), _T( "document5" ) , _T( "" ) );
    drawMenu->Append( XRCID( "DOCUMENT_6" ), _T( "document6" ) , _T( "" ) );
    drawMenu->Append( XRCID( "DOCUMENT_7" ), _T( "document7" ) , _T( "" ) );
    drawMenu->Append( XRCID( "DOCUMENT_8" ), _T( "document8 with view dependent objs" ) , _T( "" ) );
    drawMenu->Append( XRCID( "DOCUMENT_9" ), _T( "document9 with view dependent obj and freetype" ) , _T( "" ) );
    drawMenu->Append( XRCID( "DOCUMENT_10" ), _T( "document10 - XOR operation " ) , _T( "" ) );
    drawMenu->Append( XRCID( "DOCUMENT_11" ), _T( "document11 with simple view dependent obj and without freetype" ) , _T( "" ) );
#if wxART2D_USE_AGGDRAWER
    drawMenu->Append( XRCID( "DRAWER_CHANGE" ), wxT( "change drawer" ), _( "switch to aggdrawer or back to dcdrawer" ) );
#endif //wxART2D_USE_AGGDRAWER
    drawMenu->Append( XRCID( "CURSOR_ONLY_BY_CURVES" ), wxT( "cursor only by curves" ), _( "switch cursor only by curves or by all area" ) );
    drawMenu->Append( XRCID( "AXISX_INVERT" ), wxT( "invert area by X" ), _( "invert curves area by X in all plots" ) );
    drawMenu->Append( XRCID( "AXISY_INVERT" ), wxT( "invert area by Y" ), _( "invert curves area by Y in all plots" ) );
    drawMenu->Append( XRCID( "AXISX_SWITCH" ), wxT( "switch axis X" ), _( "switch axis x to scale 0,10 in all plots" ) );

    wxMenu* help_menu = new wxMenu;
    help_menu->Append( XRCID( "DOCCANVAS_ABOUT" ), _T( "&About" ) );

    menu_bar->Append( file_menu, _T( "&File" ) );
    menu_bar->Append( m_edit_menu, _T( "&Edit" ) );
    menu_bar->Append( help_menu, _T( "&Help" ) );
    menu_bar->Append( drawMenu, _T( "&samples" ) );

    SetMenuBar( menu_bar );
#endif
}


void MyFrame::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    m_canvas->GetDrawing()->GetRootObject()->SetViewDependent( NULL, false, false, false, true );
    Destroy();
}

void MyFrame::Refresh( wxCommandEvent& WXUNUSED( event ) )
{
    m_canvas->Update();
}

void MyFrame::OnExit( wxCommandEvent& WXUNUSED( event ) )
{
    //i don't want to save no matter what
    //BUT you might want to save it.
    m_canvas->GetDrawing()->Modify( false );
    Close( true );
}


void MyFrame::PushCurveTool( wxCommandEvent& WXUNUSED( event ) )
{
#if wxART2D_USE_EDITOR
    a2dCurveZoomTool* zoomt = new a2dCurveZoomTool( m_contr );
    m_contr->PushTool( zoomt );
    zoomt->SetStroke( a2dStroke( wxColour( 255, 0, 0 ), 0, a2dSTROKE_LONG_DASH ) );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}


void MyFrame::PushPolygonTool( wxCommandEvent& WXUNUSED( event ) )
{
#if wxART2D_USE_EDITOR
    a2dDrawPolygonLTool* draw = new a2dDrawPolygonLTool( m_contr );

    a2dFill fill = a2dFill( wxColour( 29, 245, 164 ), a2dFILL_HORIZONTAL_HATCH );

    draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 239, 5, 64 ), 0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );

    m_contr->PushTool( draw );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::PushRectTool( wxCommandEvent& WXUNUSED( event ) )
{
#if wxART2D_USE_EDITOR
    a2dDrawRectangleTool* draw = new a2dDrawRectangleTool( m_contr );
    draw->SetEditAtEnd( true );

    a2dFill fill = a2dFill( wxColour( 245, 29, 164 ), a2dFILL_VERTICAL_HATCH );

    draw->SetFill( fill );

    a2dStroke stroke = a2dStroke( wxColour( 5, 239, 64 ), 0, a2dSTROKE_LONG_DASH );

    draw->SetStroke( stroke );

    m_contr->PushTool( draw );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::PushEditTool( wxCommandEvent& WXUNUSED( event ) )
{
#if wxART2D_USE_EDITOR
    a2dRecursiveEditTool* edit = new a2dRecursiveEditTool( m_contr );

    m_contr->PushTool( edit );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::PushTextTool( wxCommandEvent& WXUNUSED( event ) )
{
#if wxART2D_USE_EDITOR
    a2dDrawTextTool* text = new a2dDrawTextTool( m_contr );
    text->SetStroke( a2dStroke( wxColour( 239, 5, 64 ), 0, a2dSTROKE_SOLID ) );

    text->GetTemplateObject()->SetFont( a2dFont( wxFont( 1, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL ), 30.0 ) );
    text->GetTemplateObject()->SetLineSpacing( 1 );
    m_contr->PushTool( text );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::PushSelectTool( wxCommandEvent& WXUNUSED( event ) )
{
#if wxART2D_USE_EDITOR
    a2dSelectTool* selt = new a2dSelectTool( m_contr );
    selt->SetStroke( a2dStroke( wxColour( 200, 0, 0 ), 1.0, a2dSTROKE_LONG_DASH ) );
    m_contr->PushTool( selt );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::PushDeleteTool( wxCommandEvent& WXUNUSED( event ) )
{
#if wxART2D_USE_EDITOR
    a2dDeleteTool* deletetool = new a2dDeleteTool( m_contr );
    m_contr->PushTool( deletetool );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::EndTool( wxCommandEvent& WXUNUSED( event ) )
{
#if wxART2D_USE_EDITOR
    a2dSmrtPtr< a2dBaseTool > tool;
    m_contr->PopTool( tool, false );
#else //wxART2D_USE_EDITOR
    wxMessageBox( wxT( "Need editor module enabled for this" ) );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( _T( "wxArt2D Curve Demo\nAuthor: Klaas Holwerda" ) , _T( "About Ploting" ) );
}

/*
void MyFrame::OnPrint(wxCommandEvent& event)
{
#if wxUSE_PRINTING_ARCHITECTURE

    wxPrintDialogData printDialogData( g_pageSetupData->GetPrintData());
    wxPrintout *printout = m_canvas->GetCanvasView()->OnCreatePrintout( event.GetId() );
    if (printout)
    {
        wxPrinter printer(&printDialogData);
        if (printer.Print( m_canvas->GetCanvasView()->GetDisplayWindow(), printout, true)
            || wxPrinter::GetLastError() != wxPRINTER_ERROR)
            g_pageSetupData->SetPrintData(printer.GetPrintDialogData().GetPrintData());

        delete printout;
    }
#endif // wxUSE_PRINTING_ARCHITECTURE
}

void MyFrame::OnPreview(wxCommandEvent& event)
{
#if wxUSE_PRINTING_ARCHITECTURE
    wxPrintDialogData printDialogData( g_pageSetupData->GetPrintData() );
    wxPrintout *printout = m_canvas->GetCanvasView()->OnCreatePrintout( event.GetId() );
    if (printout)
    {
        // Pass two printout objects: for preview, and possible printing.
        wxPrintPreviewBase *preview = (wxPrintPreviewBase *) NULL;
        preview = new wxPrintPreview(printout, m_canvas->GetCanvasView()->OnCreatePrintout( event.GetId() ), &printDialogData);

        wxPreviewFrame *frame = new wxPreviewFrame(preview, (wxFrame *)wxTheApp->GetTopWindow(), _("Print Preview"),
                wxPoint(100, 100), wxSize(600, 650));
        frame->Centre(wxBOTH);
        frame->Initialize();
        frame->Show(true);
    }
#endif // wxUSE_PRINTING_ARCHITECTURE
}

void MyFrame::OnPrintSetup(wxCommandEvent& WXUNUSED(event))
{
#if wxUSE_PRINTING_ARCHITECTURE
    wxPageSetupDialog pageSetupDialog(this, g_pageSetupData);
    pageSetupDialog.ShowModal();
    *g_pageSetupData = pageSetupDialog.GetPageSetupData();
#endif // wxUSE_PRINTING_ARCHITECTURE
}
*/

void MyFrame::OnTimer( wxTimerEvent& WXUNUSED( event ) )
{
    a2dVertexListCurve* curveget = ( a2dVertexListCurve* ) m_plot3->GetCurve(  _T( "curve3" ) );
    curveget->GetPoints()->pop_front();

    a2dLineSegment* pointo;
    forEachIn( a2dVertexList, curveget->GetPoints() )
    {
        pointo = *iter;
        pointo->m_x -= 4;
    }

    double x = curveget->GetPoints()->back()->m_x + 8;
    curveget->AddPoint( x,  110 + 100 * sin( x / 10.0 ) );

    wxWakeUpIdle();
}


void MyFrame::OnFileOpen( wxCommandEvent& WXUNUSED( event ) )
{
#if wxART2D_USE_CVGIO
    wxString tmp = wxFileSelector( _( "Load File" ),
                                   _T( "" ),
                                   _T( "" ),
                                   _T( "cvg" ),
                                   _T( "*.cvg" ),
                                   wxFD_OPEN,
                                   this );

    if ( tmp.IsEmpty() )
        return;


    a2dIOHandlerCVGIn in;
    m_canvas->GetDrawing()->LoadFromFile( tmp, &in );

#else //wxART2D_USE_CVGIO
    wxMessageBox( _T( "CVG not enabled" ), _T( "Open CVG" ), wxOK );
#endif //wxART2D_USE_CVGIO

}

void MyFrame::OnFileSave( wxCommandEvent& WXUNUSED( event ) )
{
#if wxART2D_USE_CVGIO
    a2dIOHandlerCVGOut out;
    a2dDrawing* drawing = m_canvas->GetDrawing();
    wxString filen = "drawing";
    m_canvas->GetDrawing()->SaveToFile( filen, &out );
#else //wxART2D_USE_CVGIO
    wxMessageBox( _T( "CVG not enabled" ), _T( "Save CVG" ), wxOK );
#endif //wxART2D_USE_CVGIO
}

void MyFrame::OnFileSaveAs( wxCommandEvent& WXUNUSED( event ) )
{
#if wxART2D_USE_CVGIO
    wxString tmp = wxFileSelector( _( "Save File" ),
                                   _T( "" ),
                                   _T( "" ),
                                   _T( "cvg" ),
                                   _T( "*.cvg" ),
                                   wxFD_SAVE,
                                   this );

    if ( tmp.IsEmpty() )
        return;

    a2dIOHandlerCVGOut out;
    m_canvas->GetDrawing()->SaveToFile( tmp, &out );
#else //wxART2D_USE_CVGIO
    wxMessageBox( _T( "CVG not enabled" ), _T( "Save CVG" ), wxOK );
#endif //wxART2D_USE_CVGIO
}

void MyFrame::OnSvgSave( wxCommandEvent& WXUNUSED( event ) )
{
    wxMessageBox( _T( "Svg not implemented for those objects" ), _T( "Saving SVG" ), wxOK );
    
        wxString tmp = wxFileSelector(_("Save Bitmap"),
                _T(""),
                _T(""),
                _T("svg"),
                _T("*.svg"),
                wxFD_SAVE,
                this);

        if (tmp.IsEmpty())
            return;

        //todo a2dIOHandlerSVGin out;
        //todo m_canvas->GetDrawing()->SaveToFile( tmp, &out );
    
}

void MyFrame::OnBitmapSave( wxCommandEvent& WXUNUSED( event ) )
{
    wxString tmp = wxFileSelector( _( "Save Bitmap" ),
                                   _T( "" ),
                                   _T( "" ),
                                   _T( "jpeg" ),
                                   _T( "*.jpeg" ),
                                   wxFD_SAVE,
                                   this );

    if ( tmp.IsEmpty() )
        return;

    int dx, dy;
    m_canvas->GetClientSize( &dx, &dy );
    wxRect rect ( 0, 0, dx, dy );

    wxBitmap bitmap = m_canvas->GetDrawingPart()->GetDrawer2D()->GetBuffer();
    bitmap = bitmap.GetSubBitmap( rect );

    if ( !bitmap.SaveFile( tmp, wxBITMAP_TYPE_JPEG ) )
    {
        delete &bitmap;
        wxMessageBox( _T( "There was a problem saving the bitmap." ), _T( "Saving Bitmap" ), wxOK );
        return;
    }
}

void MyFrame::OnAnyBitmapSave( wxCommandEvent& WXUNUSED( event ) )
{
    // GIF isn't supported (because of the licence)
    wxString tmp = wxFileSelector( _T( "Save Bitmap (PNG/BMP/JPEG)" ),
                                   _T( "" ),
                                   _T( "" ),
                                   _T( "" ),
                                   _T( "All files (*.*)|*.*|PNG files (*.png)|*.png|BMP files (*.bmp)|*.bmp|JPEG files (*.jpeg)|*.jpeg" ),
                                   wxFD_SAVE,
                                   this );

    if ( tmp.IsEmpty() )
        return;

    int dx, dy;
    m_canvas->GetClientSize( &dx, &dy );
    wxRect rect ( 0, 0, dx, dy );
    wxBitmap bitmap = m_canvas->GetDrawingPart()->GetDrawer2D()->GetSubBitmap( rect );
    wxImage asImage = bitmap.ConvertToImage();

    //use extension for type define
    if ( !asImage.SaveFile( tmp ) )
    {
        delete &bitmap;
        wxMessageBox( _T( "There was a problem saving the image as bitmap." ), _T( "Saving Bitmap" ), wxOK );
        return;
    }
}

void MyFrame::OnAnyBitmapSaveIoHandler( wxCommandEvent& WXUNUSED( event ) )
{
    // we have created the document internal ( not read from a file)
    // Therefore a template was not set, we set it here to the only template we have.
    //m_canvas->GetCanvasDocument()->SetDocumentTemplate( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocTemplates().GetFirst()->GetData() );

    //m_canvas->GetCanvasDocument()->Export( NULL );

    // this one is easier.
    m_view->GetDocument()->Export( a2dDocviewGlobals->GetDocviewCommandProcessor()->GetDocTemplates().front() );
}

#if wxART2D_USE_AGGDRAWER
void MyFrame::OnDrawer( wxCommandEvent& event )
{
    if ( wxDynamicCast( m_canvas->GetDrawingPart()->GetDrawer2D(), a2dAggDrawer ) )
        m_canvas->GetDrawingPart()->SetDrawer2D( new a2dMemDcDrawer( m_canvas->GetSize() ) );
    else
        m_canvas->GetDrawingPart()->SetDrawer2D( new a2dAggDrawer( m_canvas->GetSize() ) );

    m_canvas->GetDrawingPart()->SetShowObject( m_canvas->GetDrawing()->GetRootObject() );
    m_canvas->SetYaxis( true );
    m_canvas->GetDrawingPart()->Update( a2dCANVIEW_UPDATE_ALL );
}
#endif //wxART2D_USE_AGGDRAWER

void MyFrame::OnCursorOnlyByCurves( wxCommandEvent& event )
{
    a2dCanvasObject* aRoot = m_canvas->GetShowObject();
    if( aRoot )
    {
        forEachIn( a2dCanvasObjectList, aRoot->GetChildObjectList() )
        {
            a2dCanvasObject* obj = *iter;
            a2dPlot* plot = wxDynamicCast( obj, a2dPlot );
            if( plot )
            {
                plot->SetCursorOnlyByCurves( !plot->GetCursorOnlyByCurves() );
            }
        }
        m_canvas->GetDrawingPart()->Update( a2dCANVIEW_UPDATE_ALL );
    }
}

void MyFrame::OnInvertXAxis( wxCommandEvent& event )
{
    a2dCanvasObject* aRoot = m_canvas->GetShowObject();
    if( aRoot )
    {
        forEachIn( a2dCanvasObjectList, aRoot->GetChildObjectList() )
        {
            a2dCanvasObject* obj = *iter;
            a2dPlot* plot = wxDynamicCast( obj, a2dPlot );
            if( plot )
            {
                const a2dBoundingBox& extbox = plot->GetPlotAreaRect();
                const a2dCurvesAreaList& axesarealist = plot->GetCurvesAreaList();
                for( int i = 0; i < axesarealist.GetCount(); i++ )
                {
                    a2dCurvesArea* area = axesarealist.Item( i );
                    a2dBoundingBox intrect = area->GetInternalBoundaries();
                    double tempx = intrect.GetMinX();
                    intrect.SetMinX( intrect.GetMaxX() );
                    intrect.SetMaxX( tempx );
                    area->SetInternalBoundaries( extbox, intrect );
                }
                plot->GetAxisX()->SetPending( true );
            }
        }
        m_canvas->GetDrawingPart()->Update( a2dCANVIEW_UPDATE_ALL );
    }
}

void MyFrame::OnInvertYAxis( wxCommandEvent& event )
{
    a2dCanvasObject* aRoot = m_canvas->GetShowObject();
    if( aRoot )
    {
        forEachIn( a2dCanvasObjectList, aRoot->GetChildObjectList() )
        {
            a2dCanvasObject* obj = *iter;
            a2dPlot* plot = wxDynamicCast( obj, a2dPlot );
            if( plot )
            {
                const a2dBoundingBox& extbox = plot->GetPlotAreaRect();
                const a2dCurvesAreaList& axesarealist = plot->GetCurvesAreaList();
                for( int i = 0; i < axesarealist.GetCount(); i++ )
                {
                    a2dCurvesArea* area = axesarealist.Item( i );
                    a2dBoundingBox intrect = area->GetInternalBoundaries();
                    double tempy = intrect.GetMinY();
                    intrect.SetMinY( intrect.GetMaxY() );
                    intrect.SetMaxY( tempy );
                    area->SetInternalBoundaries( extbox, intrect );
                }
                plot->GetAxisX()->SetPending( true );
            }
        }
        m_canvas->GetDrawingPart()->Update( a2dCANVIEW_UPDATE_ALL );
    }
}

void MyFrame::OnSwitchXAxis( wxCommandEvent& event )
{
    a2dCanvasObject* aRoot = m_canvas->GetShowObject();
    if( aRoot )
    {
        forEachIn( a2dCanvasObjectList, aRoot->GetChildObjectList() )
        {
            a2dCanvasObject* obj = *iter;
            a2dPlot* plot = wxDynamicCast( obj, a2dPlot );
            if( plot )
            {
                const a2dCurvesAreaList& axesarealist = plot->GetCurvesAreaList();
                a2dCurvesArea* area = axesarealist.Item( 0 );
                a2dBoundingBox intrect = area->GetInternalBoundaries();

                a2dCurveAxis* pAxisX = plot->GetAxisX();

                if( 0 != pAxisX->GetTicMin() || 10 != pAxisX->GetTicMax() )
                    pAxisX->SetTicBoundaries( 0, 10 );
                else
                    pAxisX->SetTicBoundaries( intrect.GetMinX(), intrect.GetMaxX() );
            }
        }
        m_canvas->GetDrawingPart()->Update( a2dCANVIEW_UPDATE_ALL );
    }
}

void MyFrame::FillData( wxCommandEvent& event )
{
#if wxUSE_XRC
    FillDataDocNr( event.GetId() - XRCID( "DOCUMENT_0" ) );
#else
    FillDataDocNr( event.GetId() - DOCUMENT_FIRST );
#endif
}

void MyFrame::FillDataDocNr( wxUint16 documentNr )
{
    a2dDrawingPtr olddrawing = m_drawing;
    m_drawing = new a2dDrawing();
    a2dDrawingPtr drawing = m_drawing;
    m_canvas->GetDrawingPart()->SetShowObject( drawing->GetRootObject() );
    m_canvas->m_documentNr = documentNr;
    if( m_plotmousemenu )
    {
        delete m_plotmousemenu;
        m_plotmousemenu = NULL;
    }

    switch ( documentNr )
    {
        case 11:
        {
            FillDocument11( drawing );
            break;
        }

        case 10:
        {
            FillDocument10( drawing );
            break;
        }
        case 9:
        {
            FillDocument9( drawing );
            break;
        }

        case 8:
        {
            FillDocument8( drawing );
            break;
        }
        case 7:
        {
            FillDocument7( drawing );
            break;
        }
        case 6:
        {
            FillDocument6( drawing );
            break;
        }
        case 5:
        {
            FillDocument5( drawing );
            break;
        }
        case 0:
        {
            FillDocument0( drawing );
            break;
        }
        case 1:
        {
            FillDocument1( drawing );
            break;
        }
        case 2:
        {
            FillDocument2( drawing );
            break;
        }
        case 3:
        {
            FillDocument3( drawing );
            break;
        }
        case 4:
        {
            FillDocument4( drawing );
            break;
        }

        case 12:
        {
            FillDocument12( drawing );
            break;
        }
        default:
            return;
    }


    //calculate the boundingboxes to know how to set the mapping
    drawing->SetRootRecursive();
    a2dCanvasGlobals->SetActiveDrawingPart( m_canvas->GetDrawingPart() );
    m_canvas->GetDrawingPart()->Update( a2dCANVIEW_UPDATE_SYNC_DRAWERS );

    if( documentNr != 8 && documentNr != 9 && documentNr != 11 )
    {
        m_canvas->GetDrawingPart()->SetPrintFitToPage( false );
        int dx2, dy2;
        m_canvas->GetClientSize( &dx2, &dy2 );

        double width = drawing->GetRootObject()->GetBboxWidth();
        double height = drawing->GetRootObject()->GetBboxHeight();
        double xupp = ( 1.1 * width ) / dx2;
        double yupp = ( 1.1 * height ) / dy2;
        double minX = drawing->GetRootObject()->GetBboxMinX() - ( 0.05 * width );
        double minY = drawing->GetRootObject()->GetBboxMinY() - ( 0.05 * height );

        if ( yupp > xupp )
        {
            xupp = yupp;
            // first set the upp
            m_canvas->SetMappingUpp( minX, minY, xupp, yupp );
        }
        else
        {
            yupp = xupp;
            m_canvas->SetMappingUpp( minX, minY, xupp, yupp );
        }
    }
    else
    {
        m_canvas->GetDrawingPart()->SetPrintFitToPage( true );
        int canvasSizeX, canvasSizeY;
        m_canvas->GetClientSize( &canvasSizeX, &canvasSizeY );
        wxSizeEvent szEvt( wxSize( canvasSizeX, canvasSizeY ) );
        m_canvas->ProcessEvent( szEvt );
    }
}

//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------

IMPLEMENT_APP( MyApp )

MyApp::MyApp()
{
}

bool MyApp::OnInit()
{
#if wxART2D_USE_CANEXTOBJ
    wxInitAllImageHandlers();

    a2dCanvasDocumentIOHandlerImageOut* imageh = new a2dCanvasDocumentIOHandlerImageOut();

    a2dDocumentTemplate* doctemplate;
    /*
        doctemplate = new a2dDocumentTemplate( _T("All Files"),
            _T("All files (*.*)|*.*|PNG files (*.png)|*.png|BMP files (*.bmp)|*.bmp|JPEG files (*.jpeg)|*.jpeg"),
            _T("") , _T("") , _T("a2dCanvasDocument") ,
            CLASSINFO(a2dCanvasDocument), NULL, a2dTemplateFlag::DEFAULT, imageh );
    */
    doctemplate = new a2dDocumentTemplate( _T( "BMP Files" ),
                                           _T( "BMP files (*.bmp)|*.bmp" ),
                                           _T( "" ) , _T( "" ) , _T( "a2dCanvasDocument" ) ,
                                           CLASSINFO( a2dCanvasDocument ), NULL, a2dTemplateFlag::DEFAULT, NULL, imageh );

    a2dDocviewGlobals->GetDocviewCommandProcessor()->AssociateDocTemplate( doctemplate );
#endif //wxART2D_USE_CANEXTOBJ

//#if wxUSE_PRINTING_ARCHITECTURE
//  g_pageSetupData = new wxPageSetupDialogData;
//#endif

#if wxUSE_XRC
    if( wxXmlResource::Get() )
    {
        wxXmlResource::Get()->InitAllHandlers();
        wxXmlResource::Get()->AddHandler( new a2dMenuBarXmlHandlerDoc );
        wxXmlResource::Get()->AddHandler( new a2dMenuXmlHandlerDoc );

//      wxString aResName = wxString::Format(wxT("../resource/%s.xrc"),GetAppName().c_str());
        wxString aResName = wxT( "curtuto1.xrc" );
        if( !wxXmlResource::Get()->Load( aResName ) )
            wxMessageBox( wxString::Format( _( "Error load XRC-file '%s'.\nWorking directory is '%s'" ), aResName.c_str(), wxGetCwd().c_str() ) );
    }
#endif

    m_frame = new MyFrame( NULL, -1, _T( "SuperApp" ) , wxPoint( -1, -1 ), wxSize( 540, 700 ) );
    m_frame->SetSizeHints( 100, 200 );
    m_frame->Show( true );

//klion2006    a2dCanvasGlobals->GetLayerSetup()->Set_Stroke( 0, a2dStroke(wxColour(55,12,25), 0,a2dSTROKE_SOLID) );

    return true;
}

int MyApp::OnExit()
{
//  a2dDocviewGlobals->SetDocviewCommandProcessor( NULL );
//#if wxUSE_PRINTING_ARCHITECTURE
//  if (g_pageSetupData) delete g_pageSetupData;
//#endif
    return 0;
}

// this ProcessEvent is to pass command events from a menu to a2dEventDistributer
bool MyFrame::ProcessEvent( wxEvent& event )
{
    // it is better the wxEvent* than the bool for a recursive check
//  if(m_alreadyInProcessEvent == &event)
//      return false;
//  m_alreadyInProcessEvent = &event;
    //! but it is better the wxList than wxEvent* for a recursive check
    if( m_alreadyInProcessEvents.Find( &event ) )
        return false;
    m_alreadyInProcessEvents.Append( &event );
    if ( !wxFrame::ProcessEvent( event ) )
    {
        if( event.IsCommandEvent() )
        {
            if( ProcessEvent( event ) )
            {
//              m_alreadyInProcessEvent = NULL;
                m_alreadyInProcessEvents.DeleteObject( &event );
                return true;
            }
        }
//      m_alreadyInProcessEvent = NULL;
        m_alreadyInProcessEvents.DeleteObject( &event );
        return false;
    }
//  m_alreadyInProcessEvent = NULL;
    m_alreadyInProcessEvents.DeleteObject( &event );
    return true;
}
