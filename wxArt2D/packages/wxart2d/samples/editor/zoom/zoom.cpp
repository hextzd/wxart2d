/*! \file editor/samples/zoom/zoom.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: zoom.cpp,v 1.9 2009/06/09 19:46:35 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "zoom.h"

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

// Include image
#include "smile.xpm"
#include "wx/bitmap.h"

// WDR: class implementations

class A2DCANVASDLLEXP ZoomView: public a2dDrawingPart
{
public:
    ZoomView( int width = 1000, int height = 1000 )
        : a2dDrawingPart( width, height )
    {
        //m_image.LoadFile( "image2.jpg", wxBITMAP_TYPE_JPEG );
        m_image.LoadFile( wxT( "image2.png" ), wxBITMAP_TYPE_PNG );
        double scale = m_image.GetHeight() / ( double ) m_image.GetWidth();
        m_imageW2048 = m_image;
        m_imageW2048.Rescale( 2048, ( int ) ( 2048 * scale ) );
        m_imageW1024 = m_imageW2048;
        m_imageW1024.Rescale( 1024, ( int ) ( 1024 * scale ) );
        m_imageW512 = m_imageW1024;
        m_imageW512.Rescale( 512, ( int ) ( 512 * scale ) );
        m_imageW256 = m_imageW512;
        m_imageW256.Rescale( 256, ( int ) ( 256 * scale ) );
    }

    ~ZoomView()
    {
    }

    virtual void PaintBackground( int x, int y, int width, int height )
    {
        if ( m_image.IsOk() )
        {
            //position of background image in world coordinates
            double xImageOrigin = -500;
            double yImageOrigin = 500;
            double wImage = 1400;
            double hImage = 1400;

            a2dDrawingPart::PaintBackground( x, y, width, height );
            //m_drawer2D->DrawImage( m_image, xImageOrigin, yImageOrigin, wImage, hImage );
            //return;

            int orgdevicew, devicew, orgdeviceh, deviceh;
            orgdevicew = devicew = m_drawer2D->WorldToDeviceXRel( wImage );
            orgdeviceh = deviceh = abs( m_drawer2D->WorldToDeviceYRel( hImage ) );
            wxImage imageToDraw;
            if ( orgdevicew < 256 )
                imageToDraw = m_imageW256;
            else if ( orgdevicew < 512 )
                imageToDraw = m_imageW512;
            else if ( orgdevicew < 1024 )
                imageToDraw = m_imageW1024;
            else if ( orgdevicew < 2048 )
                imageToDraw = m_imageW2048;
            else
                imageToDraw = m_image;
            int imagew = imageToDraw.GetWidth();
            int imageh = imageToDraw.GetHeight();

            m_drawer2D->DrawImage( imageToDraw,  xImageOrigin, yImageOrigin, wImage, hImage );
            //OR
            /*
                         //device x/y of projected (imaginairy) image
                        int devicex = m_drawer2D->WorldToDeviceX( xImageOrigin );
                        int devicey = m_drawer2D->WorldToDeviceY( yImageOrigin );

                        int clipx = x;
                        int clipy = y;
                        int clipwidth = width;
                        int clipheight = height;

                        //the above gives jitter because of rounding i think, so just take the whole window as clip.
                        clipx = 0;
                        clipy = 0;
                        clipwidth = m_drawer2D->GetWidth();
                        clipheight = m_drawer2D->GetHeight();

                        int mindx=0; //miminum device x in projected (imaginairy) image
                        int mindy=0; //miminum device y in projected (imaginairy) image

                        // clip to clipping area
                        if (devicex < clipx)
                        {
                            mindx = clipx-devicex;
                            devicew -= clipx-devicex;
                        }
                        if (devicew <= 0) return;

                        if (devicey < clipy)
                        {
                            mindy = clipy-devicey;
                            deviceh -= clipy-devicey;
                        }
                        if (deviceh <= 0) return;

                        if (devicex+orgdevicew >= clipx+clipwidth)
                        {
                            devicew -= devicex + orgdevicew - (clipx + clipwidth);
                        }
                        if (devicew <= 0) return;

                        if (devicey+orgdeviceh >= clipy+clipheight)
                        {
                            deviceh -= devicey + orgdeviceh - (clipy + clipheight);
                        }
                        if (deviceh <= 0) return;

                        //now the projected image to be drawn at the given width and height in world coordinates,
                        //is clipped to the current clip rectangle.

                        // and it is something to be drawn, else we would have returned by now

                        //based on the device rectangle to be drawn, calculate what subimage we need.
                        wxRect SubImageRect( (mindx * imagew) / orgdevicew,
                                             (mindy * imageh) / orgdeviceh,
                                             (devicew * imagew) / orgdevicew,
                                             (deviceh * imageh) / orgdeviceh
                                           );

                        if ( SubImageRect.GetHeight() <= 0 || SubImageRect.GetWidth() <= 0 )
                            return;

                        wxImage SubImage = imageToDraw.GetSubImage(SubImageRect);
                        // Scale it properly.
                        SubImage.Rescale( devicew, deviceh);

                        // Generate a bitmap and a memory device context and select the bitmap
                        // into the memory device context.
                        wxBitmap Bitmap( SubImage );

                        wxMemoryDC MemoryDc;
                        MemoryDc.SelectObject(Bitmap);

                        // Copy the pre-stretched bitmap to the display device.
                        m_drawer2D->GetRenderDC()->Blit( devicex + mindx, devicey + mindy,
                                devicew, deviceh,
                                &MemoryDc, 0, 0, wxCOPY, false);
            */
        }
        else
            a2dDrawingPart::PaintBackground( x, y, width, height );
    }

    wxImage m_image;
    wxImage m_imageW2048;
    wxImage m_imageW1024;
    wxImage m_imageW512;
    wxImage m_imageW256;
};

// ID for the menu commands
enum
{
    wxID_ZOOM_ALL,
    wxOPTION_SWITCHY,
    wxID_TEST_FIX_SCROLL_MAXIMUM,
    DRAWER_CHANGE
};

BEGIN_EVENT_TABLE( PlotCanvas, a2dCanvas )
    EVT_SIZE( PlotCanvas::OnSize )
    EVT_IDLE( PlotCanvas::OnIdle )
END_EVENT_TABLE()

// Define a constructor for my canvas
PlotCanvas::PlotCanvas(  wxFrame* frame, const wxPoint& pos, const wxSize& size, const long style ):
    a2dCanvas( frame, -1, pos, size, style )
{
    a2dDrawingPart* myview = new ZoomView( size.GetWidth(), size.GetHeight() );
	SetDrawingPart( myview );

    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( this->GetDrawingPart(), frame );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );

    m_startup = true;
}

PlotCanvas::PlotCanvas( wxFrame* frame, wxWindowID id, const wxPoint& pos, const wxSize& size, const long style ):
    a2dCanvas( frame, id, pos, size, style )
{
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_drawingPart, frame );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
    m_contr->GetFirstTool()->SetShowAnotation( false );

    m_startup = true;
}

PlotCanvas::~PlotCanvas()
{
}

void PlotCanvas::OnSize( wxSizeEvent& event )
{
    a2dCanvas::OnSize( event );

    if ( m_DoShowAllAuto )
        SetMappingShowAll();

    m_drawingPart->Update( a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );


    //For always seeing everything
    //m_contr->Zoomout();

    //For having a fixed Lower Left Corner.
    //SetMappingUpp(-150,-1000,GetUppX(),GetUppY());

    //For having the same Lower Left Corner
    //SetMappingUpp( GetVisibleMinX() ,GetVisibleMinY(),GetUppX(),GetUppY());
}

void PlotCanvas::OnIdle( wxIdleEvent& event )
{
    if ( m_startup )
    {
        m_startup = false;
        SetMappingShowAll();
    }
}

//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

// WDR: event table for MyFrame

BEGIN_EVENT_TABLE( MyFrame, wxFrame )
    EVT_MENU( ID_QUIT, MyFrame::OnQuit )
    EVT_MENU( wxID_ZOOM_ALL, MyFrame::ZoomAll )
    EVT_MENU( wxOPTION_SWITCHY, MyFrame::SetSwitchY )
    EVT_MENU( wxID_TEST_FIX_SCROLL_MAXIMUM, MyFrame::TestFixScrollMaxmum )
    EVT_CLOSE( MyFrame::OnCloseWindow )
#if wxART2D_USE_AGGDRAWER
    EVT_MENU( DRAWER_CHANGE, MyFrame::OnDrawer )
#endif //wxART2D_USE_AGGDRAWER
END_EVENT_TABLE()

const int worldw = 600;
const int worldh = 400;

const double origenx = -139;
const double origeny = -234;

const double nx = 3;
const double ny = 4;

MyFrame::MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
                  const wxPoint& position, const wxSize& size, long style ) :
    wxFrame( parent, id, title, position, size, style )
{
    CreateMyMenuBar();

    CreateStatusBar( 1 );
    SetStatusText( _T( "Zoom test sample!" ) );

    SetIcon( wxICON( mondrian ) );


    bool wantScrollBars = false;//true;

    m_canvas = new PlotCanvas( this, -1, wxPoint( 0, 0 ), wxSize( worldw, worldh ) );
    m_canvas->GetDrawingPart()->SetZoomOutBorder( 20 );
    m_canvas->m_DoShowAllAuto = true;//false;

    m_canvas->SetYaxis( true );

    m_canvas->SetGridStroke( a2dStroke( wxColour( 135, 135, 135 ), 0, a2dSTROKE_DOT ) );
    m_canvas->SetGridLines( true );

    m_canvas->SetGridSize( 2 );
    m_canvas->SetGridX( 50 );
    m_canvas->SetGridY( 50 );

    //show it?
    m_canvas->SetGrid( true );
    m_canvas->SetGridAtFront( true );

    if ( wantScrollBars )
    {
        m_canvas->SetScrollBarsVisible( true );
        m_canvas->SetScrollStepX( 50 );
        m_canvas->SetScrollStepY( 50 );
        m_canvas->SetScrollMaximum( origenx, origeny, worldw * nx, worldh * ny );
    }
    else
        m_canvas->SetScrollBarsVisible( false );

    //m_canvas->FixScrollMaximum(true);
    //m_canvas->SetScaleOnResize(true);
    m_canvas->SetMappingUpp( origenx, origeny, 1.0, 1.0 );
    m_canvas->SetShowOrigin( true );

    //Start filling document now.

    // One object group is the root in every canvas.
	a2dCanvasObject* root = m_canvas->GetDrawing()->GetRootObject();

    const double rectw = 300;
    const double recth = 500;

    //rect at lowwest maximum scrollarea
    a2dRect* rect = new a2dRect( origenx, origeny, rectw, recth );
    rect->SetStroke( wxColour( 255, 255, 0 ), 2 );
    rect->SetFill( wxColour( 155, 155, 0 ) );
    root->Append( rect );

    a2dRect* rect3 = new a2dRect( worldw * nx - rectw, worldh * ny - recth, rectw, recth );
    rect3->SetStroke( wxColour( 255, 255, 0 ), 2 );
    rect3->SetFill( wxColour( 155, 155, 0 ) );
    root->Append( rect3 );

    wxBitmap bitmap( smile_xpm );
    a2dImage* m_smile = new a2dImage( bitmap.ConvertToImage(), 0, 0, 150, 150 );
    root->Append( m_smile );

    //rect size of maximum scrollarea
    a2dRect* rect5 = new a2dRect( origenx, origeny, worldw * nx - origenx, worldh * ny - origeny );
    rect5->SetStroke( wxColour( 255, 5, 5 ), 5 );
    rect5->SetFill( *a2dTRANSPARENT_FILL );
    root->Append( rect5 );

    // One object is the root object which needs to be know by all other.
    m_canvas->GetDrawing()->SetRootRecursive();
}

MyFrame::~MyFrame()
{
}

void MyFrame::CreateMyMenuBar()
{
    wxMenu* file_menu = new wxMenu;
    file_menu->Append( ID_QUIT, _T( "Quit..." ), _T( "Quit program" ) );

    wxMenuBar* menu_bar = new wxMenuBar();
    menu_bar->Append( file_menu, _T( "File" ) );

    wxMenu* zoom_menu = new wxMenu;
    zoom_menu->Append( wxID_ZOOM_ALL, _( "Zoom All" ) );
    zoom_menu->Append( wxOPTION_SWITCHY, _( "Yaxis Positive" ), _( "switch Y axis (positive or negative" ), true );
    zoom_menu->Append( wxID_TEST_FIX_SCROLL_MAXIMUM, _( "Test FixScrollMaximum" ) );
#if wxART2D_USE_AGGDRAWER
    zoom_menu->Append( DRAWER_CHANGE, wxT( "change drawer" ), _( "switch to aggdrawer or back to dcdrawer" ) );
#endif //wxART2D_USE_AGGDRAWER

    menu_bar->Append( zoom_menu, _T( "Zoom" ) );

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

void MyFrame::ZoomAll( wxCommandEvent& event )
{
    m_canvas->SetMappingShowAll();
}

void MyFrame::SetSwitchY( wxCommandEvent& WXUNUSED( event ) )
{
    m_canvas->SetYaxis( !m_canvas->GetYaxis() );
    GetMenuBar()->Check( wxOPTION_SWITCHY, m_canvas->GetYaxis() );
    m_canvas->SetMappingShowAll();
}

void MyFrame::TestFixScrollMaxmum( wxCommandEvent& event )
{
    m_canvas->m_DoShowAllAuto = false;
    bool wantScrollBars = true;

    // Problem 1:
    // Whenever FixScrollMaximum is true, it is only possible to see the whole image,
    // when the image and window have the same aspect ratio. THIS IS VERY BAD

    // Problem 2:
    // If FixScrollMaximum is true before SetMappingUpp,
    // the scaling given in SetMappingUpp has no effect

    m_canvas->SetYaxis( false );

    if ( wantScrollBars )
    {
        m_canvas->SetScrollBarsVisible( true );
        m_canvas->SetScrollStepX( 1 );
        m_canvas->SetScrollStepY( 1 );
        m_canvas->SetScrollMaximum( origenx, origeny, worldw * nx, worldh * ny );
    }
    else
        m_canvas->SetScrollBarsVisible( false );

    m_canvas->FixScrollMaximum( true );
    m_canvas->ClipToScrollMaximum( false );
    m_canvas->SetShowOrigin( true );
    m_canvas->SetMappingUpp( origenx, origeny, 5.0, 5.0 );

    /*
    Michael:
    I think the main miss conception is that FixScrollMaximum does not fix only the scroll
    maximum but also the visible area. These two are different things.

    Klaas: these are currently the same thing, but i understand that they can be made two.

    1- the maximum scrollable area, is the area within which the view can be scrolled.
    But when the view is already bigger then this area ( e.g. after sizing the window ),
    one can do:
    a- make it stay within again, by adjusting the scale/upp
    b- ignore the maximum, and maybe remove the scrollbars, until there is something to scroll again.
    I think this is how it works currently?

    Michael:

    Before if FixScrollMaximum was false, the scroll maximum was automatically extended,
    if the view area was larger than the scroll area. This I don't want.

    On the other hand, i still want to allow that the visible area is larger than the
    scroll area.

    */
}

#if wxART2D_USE_AGGDRAWER
void MyFrame::OnDrawer( wxCommandEvent& event )
{
    if ( wxDynamicCast( m_canvas->GetDrawingPart()->GetDrawer2D(), a2dAggDrawer ) )
    {
        m_canvas->GetDrawingPart()->SetDrawer2D( new a2dMemDcDrawer( m_canvas->GetSize() ) );
        m_canvas->SetYaxis( true );
    }
    else
    {
        m_canvas->GetDrawingPart()->SetDrawer2D( new a2dAggDrawer( m_canvas->GetSize() ) );
        m_canvas->SetYaxis( true );
    }
    m_canvas->GetDrawingPart()->Update( a2dCANVIEW_UPDATE_ALL );

}
#endif //wxART2D_USE_AGGDRAWER



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

    a2dCentralCanvasCommandProcessor* docManager = new a2dCentralCanvasCommandProcessor();
    a2dDocviewGlobals->SetDocviewCommandProcessor( docManager );

    m_frame = new MyFrame( NULL, -1, _T( "Zoom test" ), wxPoint( 20, 30 ), wxSize( 600, 400 ) );
    m_frame->Show( true );


    return true;
}

int MyApp::OnExit()
{
    return 0;
}


