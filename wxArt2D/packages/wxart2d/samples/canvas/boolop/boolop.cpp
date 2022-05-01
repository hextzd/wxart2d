/*! \file canvas/samples/boolop/boolop.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: boolop.cpp,v 1.7 2009/09/10 17:04:09 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if wxCHECK_VERSION(2, 5, 0)
#if !WXWIN_COMPATIBILITY_2_4
#include "wx/numdlg.h"
#endif
#endif

// Include private headers
#include "boolop.h"

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/booloper.h"

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

BEGIN_EVENT_TABLE( OverviewCanvas, a2dCanvas )
    EVT_CHAR( OverviewCanvas::OnChar )
END_EVENT_TABLE()

// Define a constructor for my canvas
OverviewCanvas::OverviewCanvas(  MyFrame* frame ):
    a2dCanvas(  frame )
{
    m_myframe = frame;
}

OverviewCanvas::~OverviewCanvas()
{
}

void OverviewCanvas::OnChar( wxKeyEvent& event )
{
    GetDrawing()->GetRootObject()->ReleaseChildObjects();
    m_myframe->FillDoc3( GetDrawing()->GetRootObject() );
    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
    wxWakeUpIdle();
#if wxART2D_USE_KBOOL
    m_myframe->m_booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_OR );
    m_myframe->m_booloper.Start( GetDrawing()->GetRootObject() );
#endif //wxART2D_USE_KBOOL
    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
    wxWakeUpIdle();
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

// WDR: event table for MyFrame

BEGIN_EVENT_TABLE( MyFrame, wxFrame )
    EVT_MENU( ID_HIERARCHY, MyFrame::OnRemoveHierarchy )
    EVT_MENU( ID_QUIT, MyFrame::OnQuit )
    EVT_MENU( ID_FILLDOC, MyFrame::OnRefill )
    EVT_MENU_RANGE( ID_OPER_OR, ID_OPER_Ring, MyFrame::OnBoolOperation_And )
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
    m_canvas = new OverviewCanvas( this );

    //NEXT 7 lines or optional, and depend on what you need.

    //if you do not want the origin shown
    //m_canvas->SetShowOrigin( false );

    //defines the whole area in world coordinates that can be seen by scrolling
    m_canvas->SetScrollMaximum( 0, 0, 800, 600 );

    //defines the minimum x and y to show and the number of world units per pixel in x and y
    m_canvas->SetMappingUpp( -50, -40, 1, 1 );

    m_canvas->SetYaxis( false );
    //defines the number of world units scrolled when line up or down events in X
    m_canvas->SetScrollStepX( 50 );
    //defines the number of world units scrolled when line up or down events in Y
    m_canvas->SetScrollStepY( 50 );
    //do not go outide the scroll maximum
    m_canvas->FixScrollMaximum( true ); //what to do when resizing above ScrollMaximum is still ??

    //Start filling document now.

    // One object group is the root in every canvas.
    m_root = m_canvas->GetDrawing();

    m_root->GetLayerSetup()->SetOrder( 0 , 0 );
    m_root->GetLayerSetup()->SetOrder( 1 , 1 );
    m_root->GetLayerSetup()->SetOrder( 2 , 2 );
    m_root->GetLayerSetup()->SetOrder( 3 , 3 );
    m_root->GetLayerSetup()->SetFill( 1 , a2dFill( wxColour( 255, 55, 55 ) ) );
    m_root->GetLayerSetup()->SetFill( 2 , a2dFill( wxColour( 55, 255, 55 ) ) );
    m_root->GetLayerSetup()->SetFill( 3 , a2dFill( wxColour( 55, 55, 255 ) ) );

#if wxART2D_USE_KBOOL
    a2dLayerGroup groupA;
    groupA.push_back( 1 );
    m_booloper.SetGroupA( groupA );
    a2dLayerGroup groupB;
    groupB.push_back( 2 );
    m_booloper.SetGroupB( groupB );
    //m_booloper.SetRecursionDepth( 2 );
    m_booloper.SetTarget( 3 );

    m_booloper.SetDebug( true );//false );
    m_booloper.SetLog( false );
#endif //wxART2D_USE_KBOOL

    //FillDoc1( m_root );
	FillDoc2( m_root->GetRootObject() );

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
    file_menu->Append( ID_QUIT, _T( "Quit" ), _T( "Quit apllication" ) );

    wxMenu* oper_and_menu = new wxMenu;
    oper_and_menu->Append( ID_HIERARCHY, _T( "Remove Hierachy" ), _T( "Remove all hierachy (nested objects)" ) );
    oper_and_menu->Append( ID_FILLDOC, _T( "Restore Document" ), _T( "Restore document to original" ) );
    oper_and_menu->Append( ID_OPER_OR, _T( "Boolean Or" ), _T( "Boolean Or operation" ) );
    oper_and_menu->Append( ID_OPER_AND, _T( "Boolean And" ), _T( "Boolean And operation" ) );
    oper_and_menu->Append( ID_OPER_EXOR, _T( "Boolean Exor" ), _T( "Boolean Exor operation" ) );
    oper_and_menu->Append( ID_OPER_A_B, _T( "Boolean A-B" ), _T( "Boolean A-B operation" ) );
    oper_and_menu->Append( ID_OPER_B_A, _T( "Boolean B-A" ), _T( "Boolean B-A operation" ) );
    oper_and_menu->Append( ID_OPER_Offset, _T( "Boolean Offset" ), _T( "Boolean Offset operation" ) );
    oper_and_menu->Append( ID_OPER_Smooth, _T( "Boolean Smooth" ), _T( "Boolean Smooth operation" ) );
    oper_and_menu->Append( ID_OPER_Ring, _T( "Boolean Ring" ), _T( "Boolean Smooth operation" ) );

    wxMenuBar* menu_bar = new wxMenuBar();
    menu_bar->Append( file_menu, _T( "File" ) );
    menu_bar->Append( oper_and_menu, _T( "Boolean Operation" ) );

    SetMenuBar( menu_bar );
}

void MyFrame::OnTimer( wxTimerEvent& event )
{

    m_root->GetRootObject()->ReleaseChildObjects();
    FillDoc2( m_root->GetRootObject() );
#if wxART2D_USE_KBOOL
    m_booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_OR );
    m_booloper.Start( m_root->GetRootObject() );
#endif //wxART2D_USE_KBOOL

    wxWakeUpIdle();
}

void MyFrame::OnQuit( wxCommandEvent& event )
{
    Close( true );
}

void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
    Destroy();
}

void MyFrame::OnBoolOperation_And( wxCommandEvent& event )
{
#if wxART2D_USE_KBOOL

	a2dCanvasObject* drawing = m_canvas->GetDrawing()->GetRootObject();

    switch( event.GetId() )
    {
        case ID_OPER_OR:
            m_booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_OR );
            m_booloper.Start( drawing );
            break;
        case ID_OPER_AND:
            m_booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_AND );
            m_booloper.Start( drawing );
            break;
        case ID_OPER_EXOR:
            m_booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_EXOR );
            m_booloper.Start( drawing );
            break;
        case ID_OPER_A_B:
            m_booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_A_SUB_B );
            m_booloper.Start( drawing );
            break;
        case ID_OPER_B_A:
            m_booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_B_SUB_A );
            m_booloper.Start( drawing );
            break;
        case ID_OPER_Offset:
        {
            long correction = wxGetNumberFromUser( _( "give correction factor (eg. 100.0 or -100.0)<return>:" ), _( "Correction" ), _( "Correction number" ), 100, -10000, 10000 );
            m_booloper.GetBooleanEngine()->SetCorrectionFactor( ( double ) correction );
            m_booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_CORRECTION );
            m_booloper.Start( drawing );
            break;
        }
        case ID_OPER_Smooth:
            m_booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_SMOOTHEN );
            m_booloper.Start( drawing );
            break;
        case ID_OPER_Ring:
        {
            long correction = wxGetNumberFromUser( _( "give width of ring <return>:" ), _( "Correction" ), _( "Correction number" ), 100, -10000, 10000 );
            m_booloper.GetBooleanEngine()->SetCorrectionFactor( fabs( correction / 2.0 ) );
            m_booloper.SetOperation( a2dBooleanWalkerHandler::Boolean_MAKERING );
            m_booloper.Start( drawing );
            break;
        }
        default:
            break;
    }
#endif //wxART2D_USE_KBOOL

}


void MyFrame::OnRemoveHierarchy( wxCommandEvent& event )
{
    a2dWalker_RemoveHierarchy remh( a2dCanvasOFlags::ALL );
    remh.Start( m_root->GetRootObject() );
}

void MyFrame::OnRefill( wxCommandEvent& event )
{
    m_root->GetRootObject()->ReleaseChildObjects();
    FillDoc2( m_root->GetRootObject() );
}

void MyFrame::FillDoc1( a2dCanvasObject* root )
{
    //lets check the scroll area
    a2dRect* r = new a2dRect( 0, 0, 800, 600 );
    r->SetFill( *a2dTRANSPARENT_FILL );
    r->SetStroke( wxColour( 255, 255, 0 ), 2 );
    root->Append( r );

    a2dCircle* circ = new a2dCircle( 170, 70, 50 );
    circ->SetFill(  wxColour( 55, 55, 120 ) );
    circ->SetLayer( 2 );
    root->Append( circ );

    a2dPolygonL* poly = new a2dPolygonL();
    poly->AddPoint( -30, -20 );
    poly->AddPoint( 100, 0 );
    poly->AddPoint( 100, 100 );
    poly->AddPoint( 50, 150 );
    poly->AddPoint( 0, 100 );
    poly->SetLayer( 1 );
    poly->SetFill( wxColour( 100, 17, 255 ), a2dFILL_FDIAGONAL_HATCH );
    poly->SetStroke( wxColour( 9, 115, 64 ), 4.0 );
    root->Prepend( poly );

    a2dPolygonL* poly2 = new a2dPolygonL();
    poly2->AddPoint( -30, -20 );
    poly2->AddPoint( 100, 0 );
    poly2->AddPoint( 100, 100 );
    poly2->AddPoint( 50, 150 );
    poly2->AddPoint( 0, 100 );
    poly2->SetLayer( 2 );
    poly2->SetFill( wxColour( 100, 17, 255 ), a2dFILL_VERTICAL_HATCH  );
    poly2->SetStroke( wxColour( 9, 115, 164 ), 4.0 );
    poly2->Translate( 70, 0 );
    root->Prepend( poly2 );

    a2dRect* r3 = new a2dRect( 150, 100, 200, 300 );
    r3->SetFill( wxColour( 255, 25, 120 ) );
    r3->SetStroke( wxColour( 255, 155, 0 ), 2 );
    r3->SetLayer( 1 );
    root->Append( r3 );

    //r3->Append( poly );
    //r3->Append( poly2 );


    int i;
    for ( i = 10; i < 300; i += 10 )
    {
        a2dRect* r = new a2dRect( i, 50, 3, 140 );
        r->SetFill( wxColour( 0, 255, 0 ) );
        r->SetLayer( 1 );
        root->Append( r );
    }

    for ( i = 15; i < 300; i += 10 )
    {
        a2dRect* r = new a2dRect( i, 50, 3, 140 );
        r->SetFill(  wxColour( 0, 255, 0 ) );
        r->SetLayer( 2 );
        root->Append( r );
    }
}

void MyFrame::FillDoc2( a2dCanvasObject* root )
{
    //lets check the scroll area
    a2dRect* r = new a2dRect( 0, 0, 500, 500 );
    r->SetFill( *a2dTRANSPARENT_FILL );
    r->SetStroke( wxColour( 255, 255, 0 ), 2 );
    root->Append( r );
    /*
        a2dPolygon* poly= new a2dPolygon();
        int i;
        poly->AddPoint( 0, 500 );
        poly->AddPoint( 500, 500 );
        poly->AddPoint( 500, 300 );
        poly->SetLayer( 1 );
        root->Prepend( poly );

        a2dPolygon* poly2= new a2dPolygon();
        poly->AddPoint( 140, 500 );
        poly->AddPoint( 50, 500 );
        poly->AddPoint( 50, 300 );
        poly2->SetLayer( 2 );
        root->Prepend( poly2 );
    */

    a2dPolygonL* poly = new a2dPolygonL();
    int i;
    for( i = 0; i < 10; i++ )
    {
        poly->AddPoint( rand() % 500, rand() % 500 );
    }
    poly->SetLayer( 1 );
    root->Prepend( poly );

    a2dPolygonL* poly2 = new a2dPolygonL();
    for( i = 0; i < 10; i++ )
    {
        poly2->AddPoint( rand() % 500, rand() % 500 );
    }
    poly2->SetLayer( 2 );
    root->Prepend( poly2 );
}

void MyFrame::FillDoc3( a2dCanvasObject* root )
{

//    randomize();
    int i;
    for( i = 0; i < 10
            ; i++ )
    {
        a2dRect* r = new a2dRect( rand() % 500, rand() % 500, 30 + rand() % 100, 20 + rand() % 300 );
        r->SetLayer( 1 );
        root->Append( r );
    }

    for( i = 0; i < 20; i++ )
    {
        a2dRect* r = new a2dRect( rand() % 500, rand() % 500, 30 + rand() % 100, 20 + rand() % 300 );
        r->SetLayer( 2 );
        root->Append( r );
    }
}


//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------

IMPLEMENT_APP( MyApp )

MyApp::MyApp()
{
    //_CrtSetBreakAlloc(25875);

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


