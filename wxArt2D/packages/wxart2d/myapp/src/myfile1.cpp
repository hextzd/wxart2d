/////////////////////////////////////////////////////////////////////////////
// Name:        simple.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:
/////////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"

// Include private headers
#include "myfile1.h"

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
END_EVENT_TABLE()

MyFrame::MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    wxFrame( parent, id, title, position, size, style )
{
    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( _T( "Welcome to a2dCanvas sample!" ) );

    SetIcon( wxICON( mondrian ) );

    //default 1000,1000 mapping
    m_canvas = new a2dCanvas( this );

    //NEXT 7 lines or optional, and depend on what you need.

    //if you do not want the origin shown
    //m_canvas->SetShowOrigin( false );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->SetMappingUpp( -50, -40, 1, 1 );

    m_canvas->SetYaxis( false );
    //defines the whole area in world coordinates that can be seen by scrolling
    m_canvas->SetScrollMaximum( -50, -40, 800, 600 );
    //defines the number of world units scrolled when line up or down events in X
    m_canvas->SetScrollStepX( 50 );
    //defines the number of world units scrolled when line up or down events in Y
    m_canvas->SetScrollStepY( 50 );
    //do not go outide the scroll maximum
    m_canvas->FixScrollMaximum( true ); //what to do when resizing above ScrollMaximum is still ??

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

    m_smile1 = new a2dImage( image, 0, 70, 32, 32 );
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

    m_smile2 = new a2dImage( image, 0, 110, 32, 32 );
    m_smile2->SetFill( *a2dTRANSPARENT_FILL );
    root->Append( m_smile2 );

    for ( i = 15; i < 300; i += 10 )
    {
        a2dRect* r = new a2dRect( i, 50, 3, 140 );
        r->SetFill(  wxColour( 0, 255, 0 ) );
        root->Append( r );
    }

    a2dText* tt = new a2dText( wxT( "Hello, world with shared font" ), 10, 20,  a2dFont( 18, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL )  );
    tt->SetFill( wxColour( 0, 10, 200 ) );
    tt->SetStroke( wxColour( 0, 255, 0 ) );

    root->Prepend( tt );

    // One object is the root object which needs to be know by all other.
    m_canvas->GetDrawing()->SetRootRecursive();

    new wxButton( m_canvas, -1,  _T( "Show a button" ), wxPoint( 100, 100 ), wxSize( 200, 110 ) );

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

    wxMenuBar* menu_bar = new wxMenuBar();
    menu_bar->Append( file_menu, _T( "File" ) );

    SetMenuBar( menu_bar );
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

//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------

IMPLEMENT_APP( MyApp )

MyApp::MyApp()
{
}

bool MyApp::OnInit()
{
    m_frame = new MyFrame( NULL, -1, _T( "SuperApp" ), wxPoint( 20, 30 ), wxSize( 600, 440 ) );
    m_frame->Show( true );

    return true;
}

int MyApp::OnExit()
{
    return 0;
}


