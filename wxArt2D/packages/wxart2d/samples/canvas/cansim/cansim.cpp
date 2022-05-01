/*! \file canvas/samples/cansim/cansim.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: cansim.cpp,v 1.4 2009/02/20 21:05:51 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "cansim.h"

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

// Include image
#include "smile.xpm"

// WDR: class implementations

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

// WDR: event table for MyFrame

BEGIN_EVENT_TABLE( MyFrame, wxFrame )
    EVT_MENU( ID_QUIT, MyFrame::OnQuit )
    EVT_CLOSE( MyFrame::OnCloseWindow )
    EVT_TIMER( -1, MyFrame::OnTimer )
    EVT_MENU_RANGE( MenuOption_First, MenuOption_Last, MyFrame::OnOption )
END_EVENT_TABLE()

MyFrame::MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    wxFrame( parent, id, title, position, size, style )
{
    m_xScale = 1.0;
    m_yScale = 1.0;
    m_xOrigin = 0;
    m_yOrigin = 0;
    m_yAxisReversed = true;

    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( _T( "Welcome to a2dCanvas sample!" ) );

    SetIcon( wxICON( mondrian ) );

    //default 1000,1000 mapping
    m_canvas = new a2dCanvasSim( this );

    //if you do not want the origin shown
    //m_canvas->SetShowOrigin( false );

    m_canvas->SetYaxis( m_yAxisReversed );

    //! set virtual size in pixels, this independent of the mapping below
    m_canvas->SetVirtualSize( wxSize(4,4) );
    m_canvas->SetScrollRate( 50, 50 );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->SetMappingUpp( m_xOrigin, m_yOrigin, m_xScale, m_yScale );

    //Start filling document now.

    // One object group is the root in every canvas.
	a2dCanvasObject* root = m_canvas->GetDrawing()->GetRootObject();

    //lets check the scroll area
    a2dRect* r = new a2dRect( -50, -40, 800, 600 );
    r->SetFill( *a2dTRANSPARENT_FILL );
    r->SetStroke( wxColour( 255, 255, 0 ), 2 );
    root->Append( r );

    // Bunch of rects and images.
    wxBitmap bitmap( smile_xpm );
    wxImage image( bitmap.ConvertToImage() );

    m_smile1 = new a2dImage( image, 80, 70, 32, 32 );
    m_smile1->SetFill( *a2dTRANSPARENT_FILL );
    root->Append( m_smile1 );

    a2dCircle* circ = new a2dCircle( 170, 70, 50 );
    circ->SetFill(  wxColour( 55, 55, 120 ) );
    root->Append( circ );

    int i;
    for ( i = 10; i < 300; i += 10 )
    {
        a2dRect* r = new a2dRect( i, 50, 3, 140 );
        r->SetFill( wxColour( 0, 255, 0 ) );
        root->Append( r );
    }

    m_smile2 = new a2dImage( image, 40, 110, 32, 32 );
    m_smile2->SetFill( *a2dTRANSPARENT_FILL );
    root->Append( m_smile2 );

    for ( i = 15; i < 300; i += 10 )
    {
        a2dRect* r = new a2dRect( i, 50, 3, 140 );
        r->SetFill(  wxColour( 0, 255, 0 ) );
        root->Append( r );
    }

    a2dText* tt = new a2dText( _T( "Hello from a2dCanvas" ), 10, 20,
                               a2dFont( 10.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ), 15.0 );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( wxColour( 0, 255, 0 ) );

    root->Prepend( tt );

    m_timer = new wxTimer( this );
    m_timer->Start( 80, false );
}

MyFrame::~MyFrame()
{
    delete m_timer;
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

void MyFrame::OnTimer( wxTimerEvent& event )
{
    m_smile1->Translate( 1, 0 );
    m_smile2->Translate( 1, 0 );

    wxWakeUpIdle();
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

    m_canvas->SetMappingUpp( m_xOrigin, m_yOrigin, m_xScale, m_yScale );
    m_canvas->SetYaxis( m_yAxisReversed );

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
    wxInitAllImageHandlers();

    m_frame = new MyFrame( NULL, -1, _T( "SuperApp" ), wxPoint( 20, 30 ), wxSize( 600, 440 ) );
    m_frame->Show( true );

    return true;
}

int MyApp::OnExit()
{
    return 0;
}


