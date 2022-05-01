/*! \file artbase/samples/directdraw/directdraw.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: tiletest.cpp,v 1.5 2009/07/24 16:35:21 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "wx/canvas/canmod.h"

#include "tiletest.h"


WX_DEFINE_LIST( wxRectList );

// WDR: class implementations

BEGIN_EVENT_TABLE( wxMyCanvas, wxScrolledWindow )
    EVT_PAINT( wxMyCanvas::OnPaint )
    EVT_ERASE_BACKGROUND( wxMyCanvas::OnEraseBackground )
    EVT_SIZE( wxMyCanvas::OnSize )
    EVT_CHAR( wxMyCanvas::OnChar )
END_EVENT_TABLE()

wxMyCanvas::wxMyCanvas( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ):
    wxScrolledWindow( parent, id, pos, size, style )
{
    m_backgroundfill = *a2dWHITE_FILL;

    int dvx = size.GetWidth();
    int dvy = size.GetHeight();

    if ( size == wxDefaultSize )
    {
        dvx = 1000;
        dvy = 1000;
    }
    else
        GetClientSize( &dvx, &dvy ); //this will be visible width and height in  world coordinates

    m_drawer2d = new a2dMemDcDrawer( dvx, dvy );
    //blit to this
    m_drawer2d->SetDisplay( this );

    //m_rects.Append( new wxRect( 0, 0, 800, 512 ) );
    //m_rects.Append( new wxRect( 82, 27, 428, 670 ) );
    //m_rects.Append( new wxRect( 100,200, 700, 500 ) );
    //m_rects.Append( new wxRect( 614, 247, 166, 167 ) );
    //m_rects.Append( new wxRect( 620, 489, 80, 160 ) );

    m_rects.Append( new wxRect( 111, 408, 718, 77 ) );
    m_rects.Append( new wxRect( 345, 129, 130, 522 ) );
    m_rects.Append( new wxRect( 539, 163, 202, 469 ) );
    m_rects.Append( new wxRect( 192, 223, 49, 313 ) );
    m_rects.Append( new wxRect( 385, 213, 75, 352 ) );

    m_nrRect = 35;
}

wxMyCanvas::~wxMyCanvas()
{
    delete m_drawer2d;
}

void wxMyCanvas::Clear()
{
    SetBackgroundColour( m_backgroundfill.GetColour() );

    wxWindow::ClearBackground();
}

void wxMyCanvas::OnEraseBackground( wxEraseEvent& event )
{
}

void wxMyCanvas::SetBackgroundFill( const a2dFill& backgroundfill )
{
    m_backgroundfill = backgroundfill;
}

bool wxMyCanvas::GetYaxis() const
{
    return m_drawer2d->GetYaxis();
}

void wxMyCanvas::SetYaxis( bool up )
{
    m_drawer2d->SetYaxis( up );
}

void wxMyCanvas::Update()
{
    //ininiate the wxDc's etc. in a2dCanvasView.
    m_drawer2d->BeginDraw();
    m_drawer2d->SetYaxis( true );
    m_drawer2d->SetTransform( a2dIDENTITY_MATRIX );

    m_drawer2d->SetDrawerStroke( *a2dTRANSPARENT_STROKE );
    m_drawer2d->SetDrawerFill( m_backgroundfill );
    m_drawer2d->PushIdentityTransform();
    m_drawer2d->DrawRoundedRectangle( 0, 0, m_drawer2d->GetWidth(), m_drawer2d->GetHeight(), 0 );
    m_drawer2d->PopTransform();

    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 10, 222, 215 ) ) );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 100, 17, 155 ), a2dFILL_FDIAGONAL_HATCH ) );

    a2dTiles tiles( 10, 10, 0 );

    m_drawer2d->PushIdentityTransform();
    wxRectList::compatibility_iterator nodeb = m_rects.GetFirst();
    while ( nodeb )
    {
        wxRect* uobj = nodeb->GetData();

        int x, y, width, height;
        x = uobj->x;
        y = uobj->y;
        width = uobj->width;
        height = uobj->height;

        //FILE* fp = fopen( "apenoot", "a+" );
        //fprintf( fp,"aap" );
        //fclose( fp );

        tiles.FillTiles( *uobj );

        m_drawer2d->DrawRoundedRectangle( x, y, width, height, 0 );

        nodeb = nodeb->GetNext();
    }
    m_drawer2d->PopTransform();

    tiles.DrawTiles( m_drawer2d );

    m_drawer2d->SetDrawerStroke( a2dStroke( wxColour( 10, 255, 15 ), 2 ) );
    m_drawer2d->SetDrawerFill( a2dFill( wxColour( 255, 0, 0 ), a2dFILL_VERTICAL_HATCH ) );

    tiles.DrawRects( m_drawer2d );

    m_drawer2d->EndDraw();
}

void wxMyCanvas::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );

    //ininiate the wxDc's etc. in a2dCanvasView.
    m_drawer2d->BeginDraw();

    //get the regions to update and add to the list
    //of areas that need to be blitted
    wxRegionIterator it( GetUpdateRegion() );
    while ( it )
    {
        int x = it.GetX();
        int y = it.GetY();

        int w = it.GetWidth();
        int h = it.GetHeight();

        int xx;
        int yy;
        CalcUnscrolledPosition( x, y, &xx, &yy );

        m_drawer2d->BlitBuffer( xx, yy, w, h );

        it++;
    }
    m_drawer2d->EndDraw();
}

void wxMyCanvas::OnSize( wxSizeEvent& event )
{
    int dvx;
    int dvy;

    GetClientSize( &dvx, &dvy ); //this will be visible width and height in  world coordinates
    if ( dvx && dvy )
        m_drawer2d->SetBufferSize( dvx, dvy );
    Update();
}

void wxMyCanvas::OnChar( wxKeyEvent& event )
{
    switch  ( event.GetKeyCode() )
    {
        case  WXK_TAB:
        {
            int dvx;
            int dvy;
            GetClientSize( &dvx, &dvy );

            int x, y;

            m_rects.DeleteContents( true );
            m_rects.Clear();

            int i;
            for ( i = 0; i < m_nrRect; i++ )
            {
                x = rand() % dvx;
                y = rand() % dvy;
                m_rects.Append( new wxRect( x, y, abs( rand() % 55 ), abs( rand() % 55 ) ) );
            }
            Update();
            break;
        }
        case 'q':
        {
            int dvx;
            int dvy;
            GetClientSize( &dvx, &dvy );

            int x, y, x2, y2;

            m_rects.DeleteContents( true );
            m_rects.Clear();

            int i;
            for ( i = 0; i < m_nrRect; i++ )
            {
                x = rand() % dvx;
                y = rand() % dvy;
                x2 = rand() % dvx;
                y2 = rand() % dvy;
                m_rects.Append( new wxRect( wxMin( x, x2 ), wxMin( y, y2 ), abs( x - x2 ), abs( y - y2 ) ) );
            }
            Update();
            break;
        }
        case 'w':
        {
            int dvx;
            int dvy;
            GetClientSize( &dvx, &dvy );

            int x, y;

            m_rects.DeleteContents( true );
            m_rects.Clear();

            int i;
            for ( i = 0; i < m_nrRect; i++ )
            {
                x = rand() % dvx;
                y = rand() % dvy;
                m_rects.Append( new wxRect( x, y, abs( rand() % 255 ), abs( rand() % 255 ) ) );
            }
            Update();
        }
    }
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

// WDR: event table for MyFrame

BEGIN_EVENT_TABLE( MyFrame, wxFrame )
    EVT_MENU( ID_QUIT, MyFrame::OnQuit )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_MENU_RANGE( MenuOption_First, MenuOption_Last, MyFrame::OnOption )
END_EVENT_TABLE()

MyFrame::MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    wxFrame( parent, id, title, position, size, style )
{
    m_xScale = 2.0;
    m_yScale = 2.0;
    m_xOrigin = -100;
    m_yOrigin = -200;
    m_yAxisReversed = false;

    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( _T( "Welcome to a2dCanvas sample!" ) );

    m_canvas = new wxMyCanvas( this );
    m_canvas->SetVirtualSize( size );
    m_canvas->SetScrollRate( 50, 50 );

    //Start filling document now.

    a2dFill backgr = a2dFill( wxColour( 0, 245, 245 ) );
    m_canvas->SetBackgroundFill( backgr );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->GetDrawer2D()->SetMappingUpp( m_xOrigin, m_yOrigin, m_xScale, m_yScale );
    m_canvas->GetDrawer2D()->SetYaxis( m_yAxisReversed );

    //draw on it
    m_canvas->Update();
}

MyFrame::~MyFrame()
{
}

void MyFrame::CreateMyMenuBar()
{
    wxMenu* file_menu = new wxMenu;
    file_menu->Append( ID_QUIT, _T( "Quit..." ), _T( "Quit program" ) );

    wxMenu* menuMap = new wxMenu;
    menuMap->Append( Scale_StretchHoriz, _T( "Stretch &horizontally\tCtrl-H" ) );
    menuMap->Append( Scale_ShrinkHoriz, _T( "Shrin&k horizontally\tCtrl-G" ) );
    menuMap->Append( Scale_StretchVertic, _T( "Stretch &vertically\tCtrl-V" ) );
    menuMap->Append( Scale_ShrinkVertic, _T( "&Shrink vertically\tCtrl-W" ) );
    menuMap->AppendSeparator();
    menuMap->Append( Map_Restore, _T( "&Restore to normal\tCtrl-0" ) );

    wxMenu* menuAxis = new wxMenu;
    menuAxis->Append( Axis_Vertic, _T( "Mirror vertically\tCtrl-N" ), _T( "" ), true );

    wxMenu* menuLogical = new wxMenu;
    menuLogical->Append( Origin_MoveDown, _T( "Move &down\tCtrl-D" ) );
    menuLogical->Append( Origin_MoveUp, _T( "Move &up\tCtrl-U" ) );
    menuLogical->Append( Origin_MoveLeft, _T( "Move &right\tCtrl-L" ) );
    menuLogical->Append( Origin_MoveRight, _T( "Move &left\tCtrl-R" ) );
    menuLogical->AppendSeparator();

    // now append the freshly created menu to the menu bar...
    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append( file_menu, _T( "File" ) );
    menuBar->Append( menuMap, _T( "&Mapping" ) );
    menuBar->Append( menuAxis, _T( "&Axis" ) );
    menuBar->Append( menuLogical, _T( "&Origin" ) );
    SetMenuBar( menuBar );
}

void MyFrame::OnQuit( wxCommandEvent& event )
{
    Close( true );
}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    Destroy();
}

void MyFrame::OnOption( wxCommandEvent& event )
{
    switch ( event.GetId() )
    {
        case Origin_MoveDown:
            m_yOrigin += 10;
            break;
        case Origin_MoveUp:
            m_yOrigin -= 10;
            break;
        case Origin_MoveLeft:
            m_xOrigin += 10;
            break;
        case Origin_MoveRight:
            m_xOrigin -= 10;
            break;

        case Scale_StretchHoriz:
            m_xScale *= 1.10;
            break;
        case Scale_ShrinkHoriz:
            m_xScale /= 1.10;
            break;
        case Scale_StretchVertic:
            m_yScale *= 1.10;
            break;
        case Scale_ShrinkVertic:
            m_yScale /= 1.10;
            break;
        case Map_Restore:
            m_xScale =
                m_yScale = 1.0;
            m_xOrigin =
                m_yOrigin = 0;
            break;

        case Axis_Vertic:
            m_yAxisReversed = !m_yAxisReversed;
            break;

        default:
            // skip Refresh()
            return;
    }

    m_canvas->GetDrawer2D()->SetMappingUpp( m_xOrigin, m_yOrigin, m_xScale, m_yScale );
    m_canvas->GetDrawer2D()->SetYaxis( m_yAxisReversed );

    m_canvas->Update();
    m_canvas->Refresh();
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
    m_frame = new MyFrame( NULL, -1, _T( "SuperApp" ), wxPoint( 20, 30 ), wxSize( 900, 740 ) );
    m_frame->Show( true );

    return true;
}

int MyApp::OnExit()
{
    return 0;
}


