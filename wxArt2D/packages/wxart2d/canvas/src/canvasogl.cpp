/*! \file canvas/src/canvas.cpp
    \author Robert Roebling and Klaas Holwerda

    Copyright: 2000-2004 (c) Robert Roebling

    Licence: wxWidgets Licence

    RCS-ID: $Id: canvas.cpp,v 1.114 2008/10/31 22:43:43 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
#include "wx/artbase/ogldrawer2d.h"

#ifdef __DARWIN__
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

#include "wx/canvas/canvasogl.h"
#include "wx/artbase/dcdrawer.h"

#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#endif


IMPLEMENT_DYNAMIC_CLASS( a2dDrawingPartOgl, a2dDrawingPart )

BEGIN_EVENT_TABLE( a2dDrawingPartOgl, a2dDrawingPart )
    EVT_IDLE( a2dDrawingPartOgl::OnIdle )
    EVT_UPDATE_DRAWING( a2dDrawingPartOgl::OnUpdate )
END_EVENT_TABLE()

a2dDrawingPartOgl::a2dDrawingPartOgl( const wxSize& size ):
    a2dDrawingPart( size )
{
    m_width = size.GetWidth();
    m_height = size.GetHeight();
    m_dirty = m_doSwap = false;
}

a2dDrawingPartOgl::a2dDrawingPartOgl( int width, int height ):
    a2dDrawingPart( width, height )
{
    m_dirty = m_doSwap = false;
}

a2dDrawingPartOgl::a2dDrawingPartOgl( int width, int height, a2dDrawer2D* drawer ):
    a2dDrawingPart( width, height, drawer )
{
    m_dirty = m_doSwap = false;
}

a2dDrawingPartOgl::a2dDrawingPartOgl( a2dDrawer2D* drawer ):
    a2dDrawingPart( drawer )
{
    m_dirty = m_doSwap = false;
}

a2dDrawingPartOgl::~a2dDrawingPartOgl()
{
}

a2dDrawingPartOgl::a2dDrawingPartOgl( const a2dDrawingPartOgl& other )
: a2dDrawingPart( other )
{
    m_dirty = m_doSwap = false;
}

void a2dDrawingPartOgl::SetBufferSize( int w, int h )
{
    m_width = w;
    m_height = h;
    if ( m_drawer2D )
        m_drawer2D->SetBufferSize( w, h );
}

void a2dDrawingPartOgl::SetDrawer2D( a2dDrawer2D* drawer2d, bool noDelete )
{
    if ( noDelete != true )
    {
        if ( m_drawer2D )
            delete m_drawer2D;
    }
    m_drawer2D = drawer2d;
    if ( m_drawer2D )
    {
        m_drawer2D->SetDisplay( m_drawingDisplay );
        m_width = m_drawer2D->GetWidth();
        m_height = m_drawer2D->GetHeight();
    }
}

void a2dDrawingPartOgl::OnIdle( wxIdleEvent& event )
{
    //wxLogDebug( "void a2dDrawingPartOgl::OnIdle(wxIdleEvent &event)" );

    if ( IsFrozen() || ( GetDisplayWindow() && !GetDisplayWindow()->IsShownOnScreen() ) )
    {
        event.Skip(); //to the display of the drawer.
        return;
    }

    if ( Get_UpdateAvailableLayers() )
    {
        Update( a2dCANVIEW_UPDATE_ALL );
    }

    // only redraw the areas which are currently in the updatelist,
    // waiting to be redrawn. After they are redrawn, directly blit those areas
    // to the window ( if available ).
    if ( m_dirty )
    {
        // Although pending areas most often result from pending object in the first place,
        // in some cases pending area are added directly and not via pending objects.
        // In those cases it is possible that there are pendingobjects in the document which do not have their
        // boundingbox yet calculated. e.g. SetMappingShowAll() followed by adding new objects in the code.
        // This idle event is first always sent to the drawing, so this should not be solved by arriving there first.
        // Therefore if the GetDrawing() has still pending objects wait until those have bin added too.

        //But better save then sorry!
        if ( GetDrawing() && !GetDrawing()->GetUpdatesPending() )
            Update( a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );
    }

    event.Skip(); //to the display of this view.
}

void a2dDrawingPartOgl::UpdateArea( int x, int y, int width, int height, wxUint8 id )
{
    if ( !GetDrawing() )
    {
        wxASSERT_MSG( GetDrawing() != 0, wxT( "invalid a2dDrawing in a2dDrawingPart" ) );
        return;
    }

    // clip to buffer
    if ( x < 0 )
    {
        width += x;
        x = 0;
    }
    if ( width <= 0 ) return;

    if ( y < 0 )
    {
        height += y;
        y = 0;
    }
    if ( height <= 0 ) return;

    if ( x + width > m_width )
    {
        width = m_width - x;
    }
    if ( width <= 0 ) return;

    if ( y + height > m_height )
    {
        height = m_height - y;
    }
    if ( height <= 0 ) return;

    m_drawer2D->SetDrawStyle( a2dFILLED );

    m_drawer2D->BeginDraw();

    //wxLogDebug("update x=%d, y=%d w=%d h=%d", x, y, width, height );

    //This to make all rendering only work on this area
    //when using Dc's
    //We do not do it inside the rendering routines since
    //setting the clippingregion for every object is expensive
    //and not usefull in case of DC's where clipping is already
    //part of API drawing routines.
    //Still the canvasobject is checked for intersection with the update region
    //This to prevent drawing objects that are not within
    //the update region. Which would be useless.
    //Calculalation of boundingboxes etc. for all object is seperate from the rendering stage.
    m_drawer2D->SetClippingRegionDev( x, y, width, height );

    bool ignore = GetDrawing()->GetIgnorePendingObjects();

    if ( !ignore )
        GetDrawing()->SetIgnorePendingObjects( true );

    PaintBackground( x, y, width, height );

    if ( m_grid && !m_gridatfront )
        PaintGrid( x, y, width, height );

    m_drawer2D->SetDrawerFill( *a2dTRANSPARENT_FILL );
    m_drawer2D->SetDrawerStroke( *a2dTRANSPARENT_STROKE );

    m_drawer2D->SetPreserveCache( true );
    RenderTopObject( m_documentDrawStyle, id );
    m_drawer2D->SetPreserveCache( false );

    //tools always render on top of everything else, excepts the grid.
    if ( m_toolcontroller )
        m_toolcontroller->Render();

    if ( m_grid && m_gridatfront )
        PaintGrid( x, y, width, height );

    m_drawer2D->DestroyClippingRegion();

    DrawOrigin();

    if ( !ignore )
        GetDrawing()->SetIgnorePendingObjects( false );


// debuggy
    /*
        if ( m_drawingDisplay )
        {
            m_drawer2D->PushIdentityTransform();
            m_drawer2D->SetDrawerFill( *a2dTRANSPARENT_FILL );
            m_drawer2D->SetDrawerStroke( *a2dBLACK_STROKE );
            m_drawer2D->DrawRoundedRectangle(x, y, width, height,0);
            m_drawer2D->PopTransform();
        }
    */
    m_drawer2D->EndDraw();

    // Flush
    glFlush();

    /*
        if ( m_drawingDisplay && m_drawingDisplay->GetChildren().GetCount() )
        {
            wxRect rect = wxRect(x, y, width, height);
            m_drawingDisplay->Refresh( false, &rect );
        }
    */

}

void a2dDrawingPartOgl::RedrawPendingUpdateAreas( bool noblit )
{
    if ( IsFrozen() )
        return;

    if ( m_recur ) return;
    wxASSERT_MSG( m_recur == false, wxT( "recursive calls in updating not allowed" ) );

    m_recur = true;

    if ( m_dirty )
    {    
        UpdateArea( 0, 0, m_width, m_height, 0 );
        m_doSwap = true;
        m_dirty = false;
    }
    m_recur = false;
}

void a2dDrawingPartOgl::AddPendingUpdateArea( int x, int y, int w, int h, wxUint8 id )
{
    // clip to buffer
    if ( x < 0 )
    {
        w += x;
        x = 0;
    }
    if ( w <= 0 ) return;

    if ( y < 0 )
    {
        h += y;
        y = 0;
    }
    if ( h <= 0 ) return;

    if ( x + w > m_width )
    {
        w = m_width - x;
    }
    if ( w <= 0 ) return;

    if ( y + h > m_height )
    {
        h = m_height  - y;
    }
    if ( h <= 0 ) return;

    int xmax = x + w;
    int ymax = y + h;

    m_dirty = true;
    //wxLogDebug("update x=%d, y=%d w=%d h=%d id=%d",  x, y, xmax - x, ymax - y, id );

    m_recur = false;
}

void a2dDrawingPartOgl::OnUpdate( a2dDrawingEvent& event )
{
    //wxLogDebug( _T( "update in view: %p with m_top %p" ), this, m_top );

    if ( !m_top )
        return;

    wxASSERT_MSG( m_top->GetRoot(), wxT( "showobject without root set" ) );

    if ( !m_top || m_top->GetRoot() != event.GetDrawing() )
        return;

    if ( !GetEvtHandlerEnabled() )
        return;

    int how = a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_BLIT ;

    if ( event.GetUpdateHint() != 0 )
        how = event.GetUpdateHint();

    //order of actions is important!

    if ( how & a2dCANVIEW_UPDATE_PENDING &&
            how & a2dCANVIEW_UPDATE_PENDING_PREUPDATE
       )
    {
        if ( m_layerRenderArray.size() != GetDrawing()->GetLayerSetup()->GetLayerIndex().size() )
            m_layerRenderArray.clear();

        for ( a2dLayerIndex::iterator it= GetDrawing()->GetLayerSetup()->GetLayerSort().begin(); it != GetDrawing()->GetLayerSetup()->GetLayerSort().end(); ++it)
        {
            a2dLayerInfo* layerobj = *it;
            wxUint16 i = (*it)->GetLayer();
            m_layerRenderArray[ i ].SetObjectCount( 0 );
        }

        AddObjectPendingUpdates( how );

        for ( a2dLayerViewList::iterator it= m_layerRenderArray.begin(); it != m_layerRenderArray.end(); ++it)
        {
            wxUint16 count = it->second.GetObjectCount();
            it->second.SetPreviousObjectCount( count );
        }
    }

    if ( how & a2dCANVIEW_UPDATE_PENDING &&
            how & a2dCANVIEW_UPDATE_PENDING_POSTUPDATE
       )
    {
        if ( m_layerRenderArray.size() != GetDrawing()->GetLayerSetup()->GetLayerSort().size() )
            m_layerRenderArray.clear();

        for ( a2dLayerIndex::iterator it= GetDrawing()->GetLayerSetup()->GetLayerSort().begin(); it != GetDrawing()->GetLayerSetup()->GetLayerSort().end(); ++it)
        {
            a2dLayerInfo* layerobj = *it;
            wxUint16 i = (*it)->GetLayer();
            m_layerRenderArray[ i ].SetObjectCount( 0 );
        }

        AddObjectPendingUpdates( how );

        for ( a2dLayerViewList::iterator it= m_layerRenderArray.begin(); it != m_layerRenderArray.end(); ++it)
            it->second.SetAvailable( it->second.GetObjectCount() != 0 );
        for ( a2dLayerViewList::iterator it= m_layerRenderArray.begin(); it != m_layerRenderArray.end(); ++it)
        {
            //check if first object put on a layer or last removed from a layer
            if ( ( it->second.GetPreviousObjectCount() != 0  && it->second.GetObjectCount() == 0 ) ||
                    ( it->second.GetPreviousObjectCount() == 0  && it->second.GetObjectCount() != 0 )
               )
            {
                a2dComEvent changedlayer( this, it->first, sig_changedLayerAvailable );
                ProcessEvent( changedlayer );
            }
        }
    }

    if ( how & a2dCANVIEW_UPDATE_VIEWDEPENDENT )
    {
        UpdateViewDependentObjects();
    }

    if ( how & a2dCANVIEW_UPDATE_OLDNEW )
    {
        GetDrawing()->AddPendingUpdatesOldNew();
    }

    if ( how & a2dCANVIEW_UPDATE_ALL )
    {
        SetAvailable();
        AddPendingUpdateArea( 0, 0, m_width, m_height );
    }

    if ( how & a2dCANVIEW_UPDATE_AREAS )
    {
        //redraw pending update areas into buffer
        RedrawPendingUpdateAreas();
    }

    if ( how & a2dCANVIEW_UPDATE_AREAS_NOBLIT )
    {
        //redraw pending update areas into buffer
        RedrawPendingUpdateAreas( true );
    }

    if ( m_doSwap || how & a2dCANVIEW_UPDATE_BLIT )
    {
        m_doSwap = false;
        if ( GetDisplayWindow() && GetDisplayWindow()->IsShown() )
        {
            //no need since we swap buffers anyway.
            //m_drawer2D->BeginDraw();
            //m_drawer2D->BlitBuffer( 0,0, m_width, m_height, 0, 0 );
            //m_drawer2D->EndDraw();

                        //place for code which needs to refresh objects which are drawn on top of the buffer bitmap
            // for example wxWindows derived controls.
            if ( m_drawingDisplay && m_drawingDisplay->GetChildren().GetCount() )
            {
                //m_drawingDisplay->Refresh( false, wxRect( 0,0, m_width, m_height ) );
            }

            UpdateCrossHair( m_crosshairx, m_crosshairy );

            if ( m_drawingDisplay )
            {
                a2dComEvent swapBuffer( this, sig_swapBuffer );
                m_drawingDisplay->GetEventHandler()->ProcessEvent( swapBuffer );
            }
        }
    }
}

void a2dDrawingPartOgl::Scroll( int dxy, bool yscroll, bool  total )
{
    int bw = m_width;
    int bh = m_height;
    //no need to optimize, since Ogl uses cached display lists, which are fast enough
    AddPendingUpdateArea( 0, 0, bw, bh );
    RedrawPendingUpdateAreas();
}

// ---------------------------------------------------------------------------
// a2dOglCanvas
// ---------------------------------------------------------------------------
#define MIN_MAPSIZE 10

BEGIN_EVENT_TABLE( a2dOglCanvas, wxGLCanvas )
    EVT_PAINT( a2dOglCanvas::OnPaint )
    EVT_ERASE_BACKGROUND( a2dOglCanvas::OnEraseBackground )
    EVT_SCROLLWIN( a2dOglCanvas::OnScroll )
    EVT_CHAR( a2dOglCanvas::OnChar )
    EVT_SIZE( a2dOglCanvas::OnSize )
    EVT_MOUSEWHEEL( a2dOglCanvas::OnWheel )
	EVT_ACTIVATE( a2dOglCanvas::OnActivate )
    EVT_COM_EVENT( a2dOglCanvas::OnComEvent )
END_EVENT_TABLE()

a2dOglCanvas::a2dOglCanvas( wxWindow *parent, wxWindowID id, 
    const wxPoint& pos,
    const wxSize& size, long style,
    a2dDrawer2D* drawer2D):
    wxGLCanvas(parent, id, NULL, pos, size, style | wxFULL_REPAINT_ON_RESIZE| wxWANTS_CHARS)
{
    // Explicitly create a new rendering context instance for this canvas.
    m_glRC = new wxGLContext(this);

    InitGL();

#ifdef _WIN32
    SetCurrent(*m_glRC);
#endif

    m_wantScroll = false;
    if ( style & wxVSCROLL || style & wxHSCROLL )
        m_wantScroll = true;

    m_inSetScrollMaximum = false;
    int dvx = size.GetWidth();
    int dvy = size.GetHeight();

    if ( size == wxDefaultSize )
    {
        dvx = 1000;
        dvy = 1000;
    }
    else
        GetSaveClientSize( &dvx, &dvy ); //this will be visible width and height in  world coordinates

	m_drawing = new a2dDrawing();

    m_drawingPart = new a2dDrawingPartOgl( dvx, dvy );
    m_drawingPart->SetDisplayWindow( this );

    if ( !drawer2D )
        drawer2D = new a2dOglDrawer( dvx, dvy, m_glRC );
    else
        drawer2D->SetBufferSize( dvx, dvy );

    m_drawingPart->SetDrawer2D( drawer2D );

    //make some maximum virtual area for this window, that fits at least the view
    m_virtm_minX = 0;
    m_virtm_minY = 0;
    m_virtm_maxX = drawer2D->GetWidth();
    if ( m_virtm_maxX == 0 )
        m_virtm_maxX = 1000;
    m_virtm_maxY = drawer2D->GetHeight();
    if ( m_virtm_maxY == 0 )
        m_virtm_maxY = 1000;

    Init( dvx, dvy );
    GetDrawer2D()->SetMappingDeviceRect( 0, 0, dvx, dvy );
	m_drawingPart->SetShowObject( m_drawing->GetRootObject() );

    //following sets m_scrollrangex and m_scrollrangey;
    //SetScrollStepX(GetDrawer2D()->GetWidth()/10);
    //SetScrollStepY(GetDrawer2D()->GetHeight()/10);

    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
}

a2dOglCanvas::a2dOglCanvas( wxWindow* parent, const int *attribList, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, a2dDrawer2D* drawer2D ): 
                wxGLCanvas(parent, id, attribList, pos, size, style | wxFULL_REPAINT_ON_RESIZE| wxWANTS_CHARS)
{
    // Explicitly create a new rendering context instance for this canvas.
    m_glRC = new wxGLContext(this);

    m_wantScroll = false;
    if ( style & wxVSCROLL || style & wxHSCROLL )
        m_wantScroll = true;

    m_inSetScrollMaximum = false;
    int dvx = size.GetWidth();
    int dvy = size.GetHeight();

    if ( size == wxDefaultSize )
    {
        dvx = 1000;
        dvy = 1000;
    }
    else
        GetSaveClientSize( &dvx, &dvy ); //this will be visible width and height in  world coordinates

	m_drawing = new a2dDrawing();

    m_drawingPart = new a2dDrawingPartOgl( dvx, dvy );
    m_drawingPart->SetDisplayWindow( this );

    InitGL();

    // This is normally only necessary if there is more than one wxGLCanvas
    // or more than one wxGLContext in the application.

#ifdef _WIN32
    SetCurrent(*m_glRC);
#endif
    if ( !drawer2D )
        drawer2D = new a2dOglDrawer( dvx, dvy, m_glRC );
    else
        drawer2D->SetBufferSize( dvx, dvy );

    m_drawingPart->SetDrawer2D( drawer2D );


    //make some maximum virtual area for this window, that fits at least the view
    m_virtm_minX = 0;
    m_virtm_minY = 0;
    m_virtm_maxX = drawer2D->GetWidth();
    if ( m_virtm_maxX == 0 )
        m_virtm_maxX = 1000;
    m_virtm_maxY = drawer2D->GetHeight();
    if ( m_virtm_maxY == 0 )
        m_virtm_maxY = 1000;

    Init( dvx, dvy );
    GetDrawer2D()->SetMappingDeviceRect( 0, 0, dvx, dvy );
	m_drawingPart->SetShowObject( m_drawing->GetRootObject() );

    //following sets m_scrollrangex and m_scrollrangey;
    //SetScrollStepX(GetDrawer2D()->GetWidth()/10);
    //SetScrollStepY(GetDrawer2D()->GetHeight()/10);

    //m_drawingPart->SetReverseOrder( true );
    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
}

a2dOglCanvas::a2dOglCanvas( a2dDrawingPart* drawingPart, wxWindow* parent, wxWindowID id,
                      const wxPoint& position, const wxSize& size, long style, const int *attribList ) :
    wxGLCanvas(parent, id, attribList, position, size, style | wxFULL_REPAINT_ON_RESIZE| wxWANTS_CHARS)
{
    // Explicitly create a new rendering context instance for this canvas.
    m_glRC = new wxGLContext(this);
#ifdef _WIN32
    SetCurrent(*m_glRC);
#endif

    m_wantScroll = false;
    if ( style & wxVSCROLL || style & wxHSCROLL )
        m_wantScroll = true;

    m_inSetScrollMaximum = false;
	m_drawing = NULL;

	m_drawingPart = drawingPart;
    drawingPart->SetDisplayWindow( this );
    drawingPart->GetDrawer2D()->SetDisplay( this );

    m_virtm_minX = 0;
    m_virtm_minY = 0;

    m_virtm_maxX = size.GetWidth();
    if ( m_virtm_maxX == 0 || m_virtm_maxX == -1 )
        m_virtm_maxX = 1000;

    m_virtm_maxY = size.GetHeight();
    if ( m_virtm_maxY == 0 || m_virtm_maxY == -1 )
        m_virtm_maxY = 1000;

    int dvx, dvy;
    GetSaveClientSize( &dvx, &dvy ); //this will be used for visible width and height in  world coordinates

    Init( dvx, dvy );

	if ( size != wxDefaultSize )
	{
    //following sets m_scrollrangex and m_scrollrangey;
    SetScrollStepX( size.GetWidth() / 10 );
    SetScrollStepY( size.GetHeight() / 10 );
	}
	else
	{
		//following sets m_scrollrangex and m_scrollrangey;
		SetScrollStepX( 10 );
		SetScrollStepY( 10 );
	}
    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
}

#if wxCHECK_VERSION(3,1,0)
a2dOglCanvas::a2dOglCanvas( wxWindow* parent,  const wxGLAttributes& dispAttrs, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, a2dDrawer2D* drawer2D ): 
                wxGLCanvas(parent, dispAttrs, id, pos, size, style | wxFULL_REPAINT_ON_RESIZE| wxWANTS_CHARS)
{
    // Explicitly create a new rendering context instance for this canvas.
    m_glRC = new wxGLContext(this);

#ifdef _WIN32
    SetCurrent(*m_glRC);
#endif

    InitGL();

    m_wantScroll = false;
    if ( style & wxVSCROLL || style & wxHSCROLL )
        m_wantScroll = true;

    m_inSetScrollMaximum = false;
    int dvx = size.GetWidth();
    int dvy = size.GetHeight();

    if ( size == wxDefaultSize )
    {
        dvx = 1000;
        dvy = 1000;
    }
    else
        GetSaveClientSize( &dvx, &dvy ); //this will be visible width and height in  world coordinates

	m_drawing = new a2dDrawing();

    m_drawingPart = new a2dDrawingPartOgl( dvx, dvy );
    m_drawingPart->SetDisplayWindow( this );

    if ( !drawer2D )
        drawer2D = new a2dOglDrawer( dvx, dvy, m_glRC );
    else
        drawer2D->SetBufferSize( dvx, dvy );

    m_drawingPart->SetDrawer2D( drawer2D );

    //make some maximum virtual area for this window, that fits at least the view
    m_virtm_minX = 0;
    m_virtm_minY = 0;
    m_virtm_maxX = drawer2D->GetWidth();
    if ( m_virtm_maxX == 0 )
        m_virtm_maxX = 1000;
    m_virtm_maxY = drawer2D->GetHeight();
    if ( m_virtm_maxY == 0 )
        m_virtm_maxY = 1000;

    Init( dvx, dvy );
    GetDrawer2D()->SetMappingDeviceRect( 0, 0, dvx, dvy );
	m_drawingPart->SetShowObject( m_drawing->GetRootObject() );

    //following sets m_scrollrangex and m_scrollrangey;
    //SetScrollStepX(GetDrawer2D()->GetWidth()/10);
    //SetScrollStepY(GetDrawer2D()->GetHeight()/10);

    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
}

a2dOglCanvas::a2dOglCanvas( a2dDrawingPart* drawingPart,  const wxGLAttributes& dispAttrs, wxWindow* parent, wxWindowID id,
                      const wxPoint& position, const wxSize& size, long style ) :
    wxGLCanvas(parent, dispAttrs, id, position, size, style | wxFULL_REPAINT_ON_RESIZE| wxWANTS_CHARS)
{
    m_wantScroll = false;
    if ( style & wxVSCROLL || style & wxHSCROLL )
        m_wantScroll = true;

    m_inSetScrollMaximum = false;
	m_drawing = NULL;

	m_drawingPart = drawingPart;
    drawingPart->SetDisplayWindow( this );
    drawingPart->GetDrawer2D()->SetDisplay( this );

    m_virtm_minX = 0;
    m_virtm_minY = 0;

    m_virtm_maxX = size.GetWidth();
    if ( m_virtm_maxX == 0 || m_virtm_maxX == -1 )
        m_virtm_maxX = 1000;

    m_virtm_maxY = size.GetHeight();
    if ( m_virtm_maxY == 0 || m_virtm_maxY == -1 )
        m_virtm_maxY = 1000;

    int dvx, dvy;
    GetSaveClientSize( &dvx, &dvy ); //this will be used for visible width and height in  world coordinates

    Init( dvx, dvy );

	if ( size != wxDefaultSize )
	{
        //following sets m_scrollrangex and m_scrollrangey;
        SetScrollStepX( size.GetWidth() / 10 );
        SetScrollStepY( size.GetHeight() / 10 );
	}
	else
	{
		//following sets m_scrollrangex and m_scrollrangey;
		SetScrollStepX( 10 );
		SetScrollStepY( 10 );
	}
    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
}
#endif 

bool a2dOglCanvas::ProcessEvent( wxEvent& event )
{
    //some wxViewEvents  go upstream ( document -> view -> window -> frame )
    //These event are of the commandevent type, so they do get processed by the parent window
    //of this window ( when not processed here ).a

    // most events will be processed first by the m_drawingPart
    if (  m_drawingPart &&
          m_drawingPart->ProcessEvent( event ) )
        return true;

	return wxEvtHandler::ProcessEvent( event );
}

a2dDrawing* a2dOglCanvas::GetDrawing() const
{ 
	if ( m_drawingPart )
		return m_drawingPart->GetDrawing(); 
	return NULL;
}

void a2dOglCanvas::SetDrawing( a2dDrawing* drawing )
{
    if ( !drawing )
    {
    	m_drawingPart = NULL;
	    m_drawing = NULL;
    }
    else
    {
	    m_drawingPart->SetShowObject( drawing->GetRootObject() );
     	m_drawing = drawing;
    }
}

void a2dOglCanvas::SetDrawingPart( a2dDrawingPart* drawingPart )
{
    int w, h;
    if ( m_drawingPart )
    {
        w = m_drawingPart->GetDrawer2D()->GetWidth();
        h = m_drawingPart->GetDrawer2D()->GetHeight();
    }
    else
    {
        GetSaveClientSize( &w, &h );
    }

	m_drawingPart = drawingPart;

    if ( m_drawingPart )
    {
        if ( m_drawingPart->GetDrawing() )
            a2dCanvasGlobals->SetActiveDrawingPart( m_drawingPart );
        m_drawingPart->SetBufferSize( w, h );
        m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
        Enable();
    }
}

void a2dOglCanvas::Init( int dvx, int dvy )
{
    m_normalize = 10000;

    m_aboveScrolledFixedAdjustUpp = true;
    m_clipToScrollMax = true;

    double dmvx = m_virtm_maxX - m_virtm_minX; //total scrollable width
    double dmvy = m_virtm_maxY - m_virtm_minY; //total scrollable height

    //following sets m_scrollrangex and m_scrollrangey;
    m_scrollstepx = dmvx / 10; //step to scroll in width
    m_thumbx = ( int )( dvx / m_scrollstepx * m_normalize ); //thumb is visible size in scroll steps
    m_scrollrangex = ( int )( dmvx / m_scrollstepx * m_normalize );

    m_scrollstepy = dmvy / 10; //step to scroll in height
    m_thumby = ( int ) ( dvy / m_scrollstepy * m_normalize ); //thumb is visible size in scroll steps
    m_scrollrangey = ( int )( dmvy / m_scrollstepy * m_normalize );

    m_scaleonresize = false;
    m_ContinuesSizeUpdate = true;

    m_scrollmaxfixed = false;

    m_xscroll_changed = false;
    m_yscroll_changed = false;
    m_has_x_scroll = false;
    m_has_y_scroll = false;

    m_scrolled = false;
    m_scrollTrackDraw = true;

    m_oldw = m_oldh = m_prevh = 0;
    m_prevClientW = m_prevClientH = 0;
}

void a2dOglCanvas::GetSaveClientSize( int* dvx, int* dvy ) const
{
    GetClientSize( dvx, dvy );

    if ( *dvx == 0 ) *dvx = MIN_MAPSIZE;
    if ( *dvy == 0 ) *dvy = MIN_MAPSIZE;
}

void a2dOglCanvas::ClearBackground()
{
    SetBackgroundColour( m_drawingPart->GetBackgroundFill().GetColour() );

    wxWindow::ClearBackground();
}

void a2dOglCanvas::Refresh( bool eraseBackground, const wxRect* rect )
{
    if ( IsFrozen() )
        return;

    if ( m_drawingPart && !rect )
    {
        m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
        GetDrawer2D()->StartRefreshDisplayDisable();
        MakeVirtualMaxFitVisibleArea();
        GetDrawer2D()->EndRefreshDisplayDisable();
    }
    wxWindow::Refresh( eraseBackground, rect );
}

a2dOglCanvas::~a2dOglCanvas()
{
    delete m_glRC;

    if ( m_drawingPart )
    {
    }
	m_drawingPart = NULL;
	m_drawing = NULL;
}

void a2dOglCanvas::SetBackgroundFill( const a2dFill& backgroundfill )
{
    if ( !m_drawingPart )
        return;
    m_drawingPart->SetBackgroundFill( backgroundfill );
    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
}

a2dCanvasObject* a2dOglCanvas::SetShowObject( const wxString& name )
{
    if ( !m_drawingPart )
        return NULL;
    return m_drawingPart->SetShowObject( name );
}

bool a2dOglCanvas::SetShowObject( a2dCanvasObject* obj )
{
    if ( !m_drawingPart )
        return false;
    return m_drawingPart->SetShowObject( obj );
}

void a2dOglCanvas::SetScrollBarsVisible( bool onoff )
{
    m_wantScroll = onoff;
    MakeVirtualMaxFitVisibleArea();
}

void a2dOglCanvas::AppendEventHandler( wxEvtHandler* handler )
{
    wxEvtHandler* handlerA = GetEventHandler();
    while ( handlerA->GetNextHandler() )
    {
        handlerA = ( wxEvtHandler* )handlerA->GetNextHandler();
    }
    handlerA->SetNextHandler( handler );
    handler->SetNextHandler( ( wxEvtHandler* )NULL );
}

wxEvtHandler* a2dOglCanvas::RemoveLastEventHandler( bool deleteHandler )
{
    //always the first in the row
    wxEvtHandler* handlerA = GetEventHandler();
    wxEvtHandler* handlerB = handlerA;
    //goto the end
    while ( handlerA->GetNextHandler() )
    {
        handlerB = handlerA;
        handlerA = ( wxEvtHandler* )handlerA->GetNextHandler();
    }

    handlerB->SetNextHandler( ( wxEvtHandler* )NULL );
    if ( deleteHandler )
    {
        delete handlerA;
    }

    return GetEventHandler();
}

double a2dOglCanvas::GetVisibleMinX() const
{
    if ( !m_drawingPart )
        return 0;
    return GetDrawer2D()->GetVisibleMinX();
}

double a2dOglCanvas::GetVisibleMinY() const
{
    if ( !m_drawingPart )
        return 0;
    return GetDrawer2D()->GetVisibleMinY();
}

double a2dOglCanvas::GetUppX() const
{
    if ( !m_drawingPart )
        return 1;
    return GetDrawer2D()->GetUppX();
}

double a2dOglCanvas::GetUppY() const
{
    if ( !m_drawingPart )
        return 1;
    return GetDrawer2D()->GetUppY();
}

double a2dOglCanvas::GetVisibleMaxX() const
{
    int dx, dy;
    GetSaveClientSize( &dx, &dy );
    return GetVisibleMinX() + GetUppX() * dx;
}

double a2dOglCanvas::GetVisibleMaxY() const
{
    int dx, dy;
    GetSaveClientSize( &dx, &dy );
    return GetVisibleMinY() + GetUppY() * dy;
}

double a2dOglCanvas::GetVisibleWidth() const
{
    int dx, dy;
    GetSaveClientSize( &dx, &dy );
    return GetUppX() * dx;
}

double a2dOglCanvas::GetVisibleHeight() const
{
    int dx, dy;
    GetSaveClientSize( &dx, &dy );
    return GetUppY() * dy;
}

bool a2dOglCanvas::GetYaxis() const
{
    if ( !GetDrawer2D() )
        return true;
    return GetDrawer2D()->GetYaxis();
}

void a2dOglCanvas::DoFreeze()
{
    if ( m_drawingPart )
        m_drawingPart->Freeze();
}

void a2dOglCanvas::DoThaw()
{
    if ( m_drawingPart )
        m_drawingPart->Thaw( true );
}

void a2dOglCanvas::OnPaint( wxPaintEvent& WXUNUSED( event ) )
{
    if ( IsFrozen() || !m_drawingPart )
        return;

    wxPaintDC dc( this ); //needed to prevent looping

    if ( !m_drawingPart )
    {
        return;
    }

    if ( m_drawingPart->IsFrozen() )
        return;

    //TODOif (!GetDrawer2D()->GetBuffer()->Ok()) return;

    //first redraw/update that were not yet updated pending areas (if available)
    //with the magic flag wxNO_FULL_REPAINT_ON_RESIZE blitting
    //also what is in the blitting list is not usefull
    //because Onpaint ignores blits outside the rectangles
    //that it finds damaged, therefore now blit flag for blitting areas.
    //This is done in idle time.

    //Mind that Onpaint events received from the GetCanvasView() is done with that eventhandler of that specific view disabled, else
    // the event would directly loop back to that view.
    //So we enable the event handler here temporarely if needed, in order to get the update event handled.
    bool eventHandlerEnabledState = m_drawingPart->GetEvtHandlerEnabled();
    if ( !eventHandlerEnabledState )
        m_drawingPart->SetEvtHandlerEnabled( true );

    /*
    //get the regions to update and add to the list
    //of areas that need to be blitted
    wxRegionIterator it( GetUpdateRegion() );
    while ( it )
    {
        int x = it.GetX();
        int y = it.GetY();

        int w = it.GetWidth();
        int h = it.GetHeight();

        //this rect is in device coordinates,
        m_drawingPart->BlitBuffer( x, y, w, h, 0, 0 );

        it++;
    }
    */

    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );
    //restore state
    m_drawingPart->SetEvtHandlerEnabled( eventHandlerEnabledState );
}

void a2dOglCanvas::SetMouseEvents( bool onoff )
{
    m_drawingPart->SetMouseEvents( onoff );
}

void a2dOglCanvas::OnEraseBackground( wxEraseEvent& WXUNUSED( event ) )
{
    //Update all needed? TODO
    //m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_BLIT );
}

a2dCanvasObject* a2dOglCanvas::IsHitWorld(
    double x, double y,
    int layer,
    a2dHitOption option
)
{
    return m_drawingPart->IsHitWorld( x, y, layer, option );
}

bool a2dOglCanvas::WriteSVG( const wxString& filename, double Width, double Height, wxString unit )
{
    return false;//m_drawingPart->WriteSVG( GetCanvasView(), filename, GetShowObject(), Width, Height, unit );
}

void a2dOglCanvas::SetMappingWidthHeight( double vx1, double vy1, double width, double height, bool scrollbars )
{
    int dxn, dyn;
    GetSaveClientSize( &dxn, &dyn );

    double xupp = width / dxn;
    double yupp = height / dyn;

    if ( yupp == 0 || xupp == 0 ) //no drawing at all
    {
        yupp = 1; xupp = 1; //some value
    }

    if ( yupp > xupp )
    {
        SetMappingUpp( vx1, vy1, yupp, yupp );
        //scroll region eqaul
        if ( scrollbars )
            SetScrollMaximum( vx1, vy1, vx1 + yupp * dyn, vy1 + yupp * dyn );
    }
    else
    {
        SetMappingUpp( vx1, vy1, xupp, xupp );
        //scroll region eqaul
        if ( scrollbars )
            SetScrollMaximum( vx1, vy1, vx1 + xupp * dxn, vy1 + xupp * dxn );
    }
}

// maps the virtual window (Real drawing to the window coordinates
// also used for zooming
void a2dOglCanvas::SetMappingUpp( double vx1, double vy1, double xpp, double ypp )
{
    int dxn, dyn;
    GetSaveClientSize( &dxn, &dyn );

    GetDrawer2D()->StartRefreshDisplayDisable();
    GetDrawer2D()->SetMappingDeviceRect( 0, 0, dxn, dyn, false );
    GetDrawer2D()->SetMappingUpp( vx1, vy1, xpp, ypp );

    MakeVirtualMaxFitVisibleArea();
    GetDrawer2D()->EndRefreshDisplayDisable();
    // redraw and swap after redraw
    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );
}

void a2dOglCanvas::SetMappingShowAll( bool centre )
{
    if ( !GetShowObject() || !GetDrawing() )
        return;

    wxUint16 border = m_drawingPart->GetZoomOutBorder();
    m_drawingPart->Update( a2dCANVIEW_UPDATE_OLDNEW );
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

    if ( !m_scrollmaxfixed )
        SetScrollMaximum( untr.GetMinX(), untr.GetMinY(), untr.GetMinX() + w, untr.GetMinY() + h );

    double uppx, uppy;
    int clientw, clienth;
    GetSaveClientSize( &clientw, &clienth );

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

    // now take the largets, to make sure it will fit either in x or y
    if ( uppy > uppx )
        uppx = uppy;

    GetDrawer2D()->StartRefreshDisplayDisable();
    GetDrawer2D()->SetMappingDeviceRect( 0, 0, clientw, clienth, false );

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
        GetDrawer2D()->SetMappingUpp( middlexworld - clientw / 2.0 * uppx, middleyworld - clienth / 2.0 * uppx, uppx, uppx );
    }

    // if scrolling bars were added are removed because of the above, we do it once more.
    int clientwNew, clienthNew;
    GetSaveClientSize( &clientwNew, &clienthNew );
    if ( clientw != clientwNew || clienth != clienthNew )
    {
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

        // now take the largets, to make sure it will fit either in x or y
        if ( uppy > uppx )
            uppx = uppy;

        GetDrawer2D()->SetMappingDeviceRect( 0, 0, clientwNew, clienthNew, false );
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
            GetDrawer2D()->SetMappingUpp( middlexworld - clientwNew / 2.0 * uppx, middleyworld - clienthNew / 2.0 * uppx, uppx, uppx );
        }
    }

    m_drawingPart->Update( a2dCANVIEW_UPDATE_OLDNEW );
    //Refresh() already result in the next
    //| a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );
    GetDrawer2D()->EndRefreshDisplayDisable();
    Refresh();
}

void a2dOglCanvas::ZoomOut( double n )
{
    int dx, dy;
    GetClientSize( &dx, &dy );
    if ( dy > dx ) dx = dy;

    SetMappingUpp( GetDrawer2D()->GetVisibleMinX() + ( 1 - n ) * GetDrawer2D()->GetUppX() * dx * 0.5,
                   GetDrawer2D()->GetVisibleMinY() + ( 1 - n ) * GetDrawer2D()->GetUppY() * dy * 0.5,
                   GetDrawer2D()->GetUppX()*n, GetDrawer2D()->GetUppY()*n );
}

void a2dOglCanvas::ZoomOutAtXy( int x, int y, double n )
{
    int dx, dy;
    GetClientSize( &dx, &dy );
    if ( dy > dx ) dx = dy;

    double mouse_worldx, mouse_worldy;
    m_drawingPart->MouseToToolWorld( x, y, mouse_worldx, mouse_worldy );

    mouse_worldx -= ( mouse_worldx - GetDrawer2D()->GetVisibleMinX() ) * n;
    mouse_worldy -= ( mouse_worldy - GetDrawer2D()->GetVisibleMinY() ) * n;

    SetMappingUpp( mouse_worldx, mouse_worldy, GetDrawer2D()->GetUppX()*n, GetDrawer2D()->GetUppY()*n );
}

void a2dOglCanvas::SetScrollbars( double pixelsPerUnitX, double pixelsPerUnitY,
                               double noUnitsX, double noUnitsY,
                               double xPos , double yPos )
{
    //set mapping to one pixel per user unit
    //and yaxis false (may also be true)
    //this will give exactly a one to one mapping (pixel based)
    //Device to World etc. still does the calculation but this is fast.
    GetDrawer2D()->SetYaxis( false );
    //A Zerobased mapping is just a normal mapping for a2dOglCanvas,
    //and result in a pixel 1 to 1 for wxWindow.

    int dxn, dyn;
    GetSaveClientSize( &dxn, &dyn );

    m_scrollstepx = dxn / pixelsPerUnitX;
    m_scrollstepy = dyn / pixelsPerUnitY;
    SetScrollMaximum( 0, 0, dxn / pixelsPerUnitX * noUnitsX, dyn / pixelsPerUnitY * noUnitsY );

    SetMappingUpp( xPos, yPos, 1, 1 );
}

bool a2dOglCanvas::MakeVirtualMaxFitVisibleArea()
{

    // all checks have a tolerance of 1 pixel for rounding
    double tol = 0;// 1 * GetUppX();

    GetDrawer2D()->StartRefreshDisplayDisable();
    bool visAreaSizeChange = false;
    bool old_has_y_scroll = m_has_y_scroll;
    bool old_has_x_scroll = m_has_x_scroll;

    // Avoid endless recursion via size events
    if( m_inSetScrollMaximum )
    {
        GetDrawer2D()->EndRefreshDisplayDisable();
        return false;
    }
    m_inSetScrollMaximum = true;

    //always use the real client size to set the scrollbars
    int dx, dy;
    GetSaveClientSize( &dx, &dy );

    //visible width in  world coordinates
    double dvx = GetUppX() * dx;
    //visible height in  world coordinates
    double dvy = GetUppY() * dy;

    // maximum width and height allowed
    double dmvx = m_virtm_maxX - m_virtm_minX;
    double dmvy = m_virtm_maxY - m_virtm_minY;

    //THE MAXIMUM SCROLLABLE AREA is leading here.
    // Anything else might change, if it is not allowed to change THE MAXIMUM SCROLLABLE AREA itself.

    assert( dmvx > 0 );
    assert( dmvy > 0 );

    // a start up issue.
    if ( !GetDrawer2D()->GetVirtualAreaSet() )
    {
        //set a visible area eqaul to the maximum scrollable area.
        double uppnew;
        if ( dmvx > dmvy )
            uppnew = dmvx / dvx;
        else
            uppnew = dmvy / dvy;

        SetMappingUpp( m_virtm_minX, m_virtm_minY, uppnew, uppnew );
        visAreaSizeChange = true;
    }

    // what is the visible on the canvas window in world coordinates according
    // to the a2dOglCanvasView its drawing context.
    double minvx = GetDrawer2D()->GetVisibleMinX();
    double minvy = GetDrawer2D()->GetVisibleMinY();
    double maxvx = GetDrawer2D()->GetVisibleMinX() + dvx;
    double maxvy = GetDrawer2D()->GetVisibleMinY() + dvy;

    //wxLogDebug( _T( "dvx=%f, dvy=%f" ), dvx, dvy );
    //wxLogDebug( _T( "m_virtm_minX=%f, m_virtm_minY=%f m_virtm_maxY=%f m_virtm_maxY=%f" ), m_virtm_minX, m_virtm_minY, m_virtm_maxX, m_virtm_maxY );
    //wxLogDebug( _T( "minvx=%f, minvy=%f maxvx=%f maxvy=%f" ), minvx, minvy, maxvx, maxvy );

    if ( !m_scrollmaxfixed )
    {
        //adjust/enlarge maximum allowed scrollable visible area, to include the current visible area.
        if ( m_virtm_minX - tol >= minvx ) m_virtm_minX = minvx;
        if ( m_virtm_maxX + tol <= maxvx ) m_virtm_maxX = maxvx;
        if ( m_virtm_minY - tol >= minvy ) m_virtm_minY = minvy;
        if ( m_virtm_maxY + tol <= maxvy ) m_virtm_maxY = maxvy;

        dmvx = m_virtm_maxX - m_virtm_minX;
        dmvy = m_virtm_maxY - m_virtm_minY;

        visAreaSizeChange = false;
    }
    else
    {
        // we can not go out the maximum scrollable area, we need to adjust the virtual area
        // such that it stays within this area, or if not possible, adjust its size to stay in this area.

        if ( m_clipToScrollMax )
        {
            bool clip = false;
            bool clipRight = false;
            bool clipLeft = false;
            bool clipUp = false;
            bool clipDown = false;
            //the clipped values of current visible area towards the maximum scrollable area.
            if ( m_virtm_minX - tol >= minvx )
            { 
                minvx = m_virtm_minX; clip = true; clipLeft = true; 
                //wxLogDebug( "Left" ); 
            }
            if ( m_virtm_maxX + tol <= maxvx )
            { 
                maxvx = m_virtm_maxX; clip = true; clipRight = true; 
                //wxLogDebug( "Right" ); 
            }
            if ( m_virtm_minY - tol >= minvy )
            { 
                minvy = m_virtm_minY; clip = true; clipDown = true;
                //wxLogDebug( "Down" ); 
            }
            if ( m_virtm_maxY + tol <= maxvy )
            { 
                maxvy = m_virtm_maxY; clip = true; clipUp = true; 
                //wxLogDebug( "Up" ); 
            }

            // The visible rectangle now fits the maximum scrollable area,
            // but we want to preserve the same Upp as much as possible.
            // Therefore we adjust to the old values to get the same virtual size again, either in width or height.
            // THIS MAY result in going outside the maximum scrollable area, if window ratio is different from maxscroll area ratio. 
            // At least height or width of virtual much fit in maximum scollable area, else scale to the maximum size possible and stay at minvx, minvy.
            // This last simply makes the virtual area either fit in one dimension being maximum X or Y, the other smaller.

            if ( clip )
            {
                // there was a clip and we need to adjust
                if ( dmvx > dvx || dmvy > dvy ) //if it fits in height or width, only need to shift the visible area such that it is in the maximum area.
                {
                    //wxLogDebug( _T( "A minvx=%f, minvy=%f maxvx=%f maxvy=%f" ), minvx, minvy, maxvx, maxvy );

                    // The orginal size does fit in the maximum scrollable area in at least one dimension.
                    // We will adjust such that minvx and minvy stay the same.

                    if ( clipLeft ) 
                    {
                        //minvx == m_virtm_minX
                        maxvx = m_virtm_minX + dvx;
                    }
                    else if ( clipRight ) 
                    {
                        //maxvx == m_virtm_maxX
                        minvx = m_virtm_maxX - dvx;
                        //with maximum zoomout, clipping to the left might happen now. So clip again, since we prefer origin stays TopLeft
                        if ( minvx < m_virtm_minX ) 
                            minvx = m_virtm_minX;
                    }

                    //else
                    //no clip in X or both, which default to clipLeft

                    if ( clipDown ) 
                        //minvy == m_virtm_minY
                        maxvy = m_virtm_minY + dvy;
                    else if ( clipUp ) 
                    {
                        //maxvy == m_virtm_maxY
                        minvy = m_virtm_maxY - dvy;
                        //with maximum zoomout, clipping to the top might happen now. So clip again, since we prefer origin stays TopLeft
                        if ( minvy < m_virtm_minY )
                            minvy = m_virtm_minY;
                    }
                    //else
                    //no clip in Y or both, which default to clipUp
                    //wxLogDebug( _T( "C minvx=%f, minvy=%f maxvx=%f maxvy=%f" ), minvx, minvy, maxvx, maxvy );

                    GetDrawer2D()->SetMinX( minvx );
                    GetDrawer2D()->SetMinY( minvy );
                }
                else
                {
                    //wxLogDebug( _T( "B minvx=%f, minvy=%f maxvx=%f maxvy=%f" ), minvx, minvy, maxvx, maxvy );

                    //the original virtual size does not fit in the maximum scrollable area.
                    // We will adjust to the maximum possible Upp.
                    // The maximum height or width should be adjusted to fit within the scrollable area
                    // + 1 since pixel (0,0) exist too.
                    double uppX = dmvx / ( dx + 1 ); //units needed to display the whole x range in window
                    double uppY = dmvy / ( dy + 1 ); //units needed to display the whole y range in window

                    if ( uppY > uppX )
                        uppX = uppY;

                    GetDrawer2D()->SetMappingUpp( minvx, minvy, uppX, uppX );

                    dvx = GetUppX() * dx;
                    dvy = GetUppY() * dy;
                }

                visAreaSizeChange = true;
            }
        }
    }


    //All should be oke now:
    // Maximum Scrollable area is bigger than the visible area.
    /*
    assert (m_virtm_minX - tol <= GetVisibleMinX());
    assert (m_virtm_maxX + tol >= GetVisibleMinX()+dvx);
    assert (m_virtm_minY - tol <=  GetVisibleMinY());
    assert (m_virtm_maxY + tol >= GetVisibleMinY()+dvy);
    */

    if ( !m_wantScroll )
    {
        m_inSetScrollMaximum = false;
        m_drawingPart->UpdateViewDependentObjects();
        GetDrawer2D()->EndRefreshDisplayDisable();
        //remove scroll bars if in cased they are there.
        if ( HasScrollbar( wxHORIZONTAL ) )
            SetScrollbar( wxHORIZONTAL, 0, 1, 1, true );
        if ( HasScrollbar( wxVERTICAL ) )
            SetScrollbar( wxVERTICAL, 0, 1, 1, true );
        return visAreaSizeChange;
    }

    // FROM NOW ON down here we are sure we want scrolling bars.


    // Now check, if we need a vertical scrollbar and set them to get the right sizes
    // of the client screen.
    m_has_y_scroll = dmvy > fabs( dvy );
    if( m_has_y_scroll )
    {
        // virtual area is larger than visible area => need vertical scrollbar
        SetScrollbar( wxVERTICAL, 0, 1, 2, false );
        // reget client size with this scrollbar
        //GetSaveClientSize(&dx,&dy);
        //dvx = GetUppX()*dx;
        //dvy = GetUppY()*dy;
    }

    // Now check, if we need a horizontal scrollbar
    m_has_x_scroll = dmvx > fabs( dvx );
    if( m_has_x_scroll )
    {
        // virtual area is larger than visible area => need horizontal scrollbar
        SetScrollbar( wxHORIZONTAL, 0, 1, 2, false );
        // reget client size with this scrollbar
        //GetSaveClientSize(&dx,&dy);
        //dvx = GetUppX()*dx;
        //dvy = GetUppY()*dy;

        // If we don't have a vertical scrollbar yet, we might need one cause of
        // the horizontal scrollbar
        if( !m_has_y_scroll )
        {
            m_has_y_scroll = dmvy > fabs( dvy );
            if( m_has_y_scroll )
            {
                // virtual area is larger than visible area => need vertical scrollbar
                SetScrollbar( wxVERTICAL, 0, 1, 2, false );
                // reget client size with this scrollbar
                //GetSaveClientSize(&dx,&dy);
                //dvx = GetUppX()*dx;
                //dvy = GetUppY()*dy;
            }
        }
    }

    // Now we have the scrollbars enabled and set
    m_scrolled = ( m_has_x_scroll || m_has_y_scroll );

    // Set the scrollbars in the right state.
    if( m_has_x_scroll )
    {
        // Recalculate thumb size and range and draw the scrollbar
        // always use the real client size to set the scrollbars
        m_thumbx = ( int )floor( dvx / m_scrollstepx * m_normalize ); //thumb is visible size in scroll steps

        m_scrollrangex = ( int ) ( dmvx / m_scrollstepx * m_normalize );

        //set the scroll bar to the right position
        //in SetScrollbar there will directly be a call to onsize
        //if scrollbar visibility changes
        m_xscroll_changed = true;
        int position = ( int )( ( GetVisibleMinX() - m_virtm_minX ) / dmvx * m_scrollrangex );
        SetScrollbar( wxHORIZONTAL, position, m_thumbx, m_scrollrangex, true );

        m_xscroll_changed = false;

    }
    else
    {
        // Just redraw a non existing scrollbar
        SetScrollbar( wxHORIZONTAL, 0, 1, 1, true );
    }

    if( m_has_y_scroll )
    {
        // Recalculate thumb size and range and draw the scrollbar
        m_thumby = ( int ) ( dvy / m_scrollstepy * m_normalize );

        m_scrollrangey = ( int ) ( dmvy / m_scrollstepy * m_normalize );

        m_yscroll_changed = true;

        if ( GetYaxis() )
        {
            // The y-Axis is going up (mathematical coords / screen inverted )

            //set the scroll bar to the right position
            //in SetScrollbar there will directly be a call to onsize
            //if scrollbar visibility changes
            int position = ( int ) ( ( m_virtm_maxY - ( GetVisibleMinY() + dvy ) ) / dmvy * m_scrollrangey );
            SetScrollbar( wxVERTICAL, position, m_thumby, m_scrollrangey, true );
        }
        else
        {
            // The y-Axis is going down (word processing coords / like screen )

            //set the scroll bar to the right position
            //in SetScrollbar there will directly be a call to onsize
            //if scrollbar visibility changes
            int position = ( int ) ( ( GetVisibleMinY() - m_virtm_minY ) / dmvy * m_scrollrangey );
            SetScrollbar( wxVERTICAL, position, m_thumby, m_scrollrangey, true );
        }
        m_yscroll_changed = false;
    }
    else
    {
        // Just redraw a non existing scrollbar
        SetScrollbar( wxVERTICAL, 0, 1, 1, true );
    }

    m_inSetScrollMaximum = false;

    if ( old_has_y_scroll != m_has_y_scroll || old_has_x_scroll != m_has_x_scroll )
        visAreaSizeChange = true;

    m_drawingPart->UpdateViewDependentObjects();
    GetDrawer2D()->EndRefreshDisplayDisable();

    return visAreaSizeChange;
}

bool a2dOglCanvas::SetScrollMaximum( double vx1, double vy1, double vx2, double vy2 )
{
    if( !GetDrawer2D() )
        return false;

    wxASSERT_MSG( vx1 < vx2, wxT( "min X bigger than max X" ) );
    wxASSERT_MSG( vy1 < vy2, wxT( "min Y bigger than max Y" ) );

    m_virtm_minX = vx1;
    m_virtm_minY = vy1;
    m_virtm_maxX = vx2;
    m_virtm_maxY = vy2;

    return MakeVirtualMaxFitVisibleArea();
}

void a2dOglCanvas::SetMinX( double vxmin )
{
    GetDrawer2D()->SetMinX( vxmin );
    MakeVirtualMaxFitVisibleArea();
}

void a2dOglCanvas::SetMinY( double vymin )
{
    GetDrawer2D()->SetMinY( vymin );
    MakeVirtualMaxFitVisibleArea();
}

void a2dOglCanvas::SetYaxis( bool up )
{
    if ( !GetDrawer2D() )
        return;
    GetDrawer2D()->SetYaxis( up );
    MakeVirtualMaxFitVisibleArea();

    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );
}

void a2dOglCanvas::SetScrollStepX( double x )
{
    m_scrollstepx = x;

    MakeVirtualMaxFitVisibleArea();
}

void a2dOglCanvas::SetScrollStepY( double y )
{
    m_scrollstepy = y;

    MakeVirtualMaxFitVisibleArea();
}

bool a2dOglCanvas::CheckInsideScrollMaximum( int dx, int dy )
{
    int dvx, dvy;
    GetSaveClientSize( &dvx, &dvy );

    double dyworld = GetDrawer2D()->DeviceToWorldYRel( dy );
    double dxworld = GetDrawer2D()->DeviceToWorldXRel( dx );
    double worldminy = GetVisibleMinY() - dyworld;
    double worldminx = GetVisibleMinX() - dxworld;

    return CheckInsideScrollMaximum( worldminx, worldminy );
}

bool a2dOglCanvas::CheckInsideScrollMaximum( double worldminx, double worldminy )
{
    if( !m_scrollmaxfixed )
        return true;

    int dvx, dvy;
    GetSaveClientSize( &dvx, &dvy );
    double worlddvx = GetUppX() * dvx;
    double worlddvy = GetUppY() * dvy;

    return
        ( m_virtm_minX <= worldminx ) &&
        ( m_virtm_maxX >= worldminx + worlddvx ) &&
        ( m_virtm_minY <= worldminy ) &&
        ( m_virtm_maxY >= worldminy + worlddvy );
}

bool a2dOglCanvas::GetMouseEvents()
{
    if ( !m_drawingPart )
        return false;

    return m_drawingPart->GetMouseEvents();
}

// ----------------------------------------------------------------------------
// scrolling behaviour
// ----------------------------------------------------------------------------

void a2dOglCanvas::OnScroll( wxScrollWinEvent& event )
{
    //always use the real client size to set the scrollbars
    int dx, dy;
    GetSaveClientSize( &dx, &dy );

    if ( event.GetEventType() == wxEVT_SCROLLWIN_THUMBRELEASE || 
         ( m_scrollTrackDraw && event.GetEventType() == wxEVT_SCROLLWIN_THUMBTRACK  )
       ) 
    {
        if ( event.GetOrientation() == wxHORIZONTAL )
        {
            double x = m_virtm_minX + event.GetPosition() / ( m_scrollrangex * 1.0 ) * ( m_virtm_maxX - m_virtm_minX );
            x = x - GetVisibleMinX();
            ScrollWindowConstrained( x, 0 );
        }
        else
        {
            double y;
            int position = event.GetPosition();
            if ( GetDrawer2D()->GetYaxis() )
            {
                double dvy = GetUppY() * dy;

                y = m_virtm_maxY - position / ( m_scrollrangey * 1.0 ) * ( m_virtm_maxY - m_virtm_minY );
                y = y - dvy - GetVisibleMinY();
            }
            else
            {
                y = m_virtm_minY + position / ( m_scrollrangey * 1.0 ) * ( m_virtm_maxY - m_virtm_minY );
                y = y - GetVisibleMinY();
            }
            ScrollWindowConstrained( 0, y );
        }
        // rounded pixel problems, redraw at end
        //if ( event.GetEventType() == wxEVT_SCROLLWIN_THUMBRELEASE ) 
        //    m_drawingPart->Update(  a2dCANVIEW_UPDATE_ALL );
    }
    else if ( event.GetEventType() == wxEVT_SCROLLWIN_PAGEUP )
    {
        //thumb is visible size in scroll steps
        if ( event.GetOrientation() == wxHORIZONTAL )
            ScrollWindowConstrained( -m_scrollstepx * ( m_thumbx / m_normalize ), 0 );
        else
        {
            if ( GetDrawer2D()->GetYaxis() )
                ScrollWindowConstrained( 0, m_scrollstepy * ( m_thumby / m_normalize ) );
            else
                ScrollWindowConstrained( 0, -m_scrollstepy * ( m_thumby / m_normalize ) );
        }
    }
    else if ( event.GetEventType() == wxEVT_SCROLLWIN_PAGEDOWN )
    {
        if ( event.GetOrientation() == wxHORIZONTAL )
            ScrollWindowConstrained( m_scrollstepx * ( m_thumbx / m_normalize ), 0 );
        else
        {
            if ( GetDrawer2D()->GetYaxis() )
                ScrollWindowConstrained( 0, -m_scrollstepy * ( m_thumby / m_normalize ) );
            else
                ScrollWindowConstrained( 0, m_scrollstepy * ( m_thumby / m_normalize ) );
        }
    }
    else if ( event.GetEventType() == wxEVT_SCROLLWIN_LINEUP )
    {
        if ( event.GetOrientation() == wxHORIZONTAL )
            ScrollWindowConstrained( -m_scrollstepx, 0 );
        else
        {
            if ( GetDrawer2D()->GetYaxis() )
                ScrollWindowConstrained( 0, m_scrollstepy );
            else
                ScrollWindowConstrained( 0, -m_scrollstepy );
        }
    }
    else if ( event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN )
    {
        if ( event.GetOrientation() == wxHORIZONTAL )
            ScrollWindowConstrained( m_scrollstepx, 0 );
        else
        {
            if ( GetDrawer2D()->GetYaxis() )
                ScrollWindowConstrained( 0, -m_scrollstepy );
            else
                ScrollWindowConstrained( 0, m_scrollstepy );
        }
    }

}

void a2dOglCanvas::OnChar( wxKeyEvent& event )
{
    //always use the real client size to set the scrollbars
    int dx, dy;
    GetSaveClientSize( &dx, &dy );

    switch ( event.GetKeyCode() )
    {
        case WXK_PAGEUP:
            //case WXK_PAGEUP:
        {
            double y = -m_scrollstepy * m_thumby / m_normalize ;
            if ( GetDrawer2D()->GetYaxis() )
                y = -y;
            ScrollWindowConstrained( 0, y );
        }
        break;
        case WXK_PAGEDOWN:
            //case WXK_PAGEDOWN:
        {
            double y = m_scrollstepy * m_thumby / m_normalize;
            if ( GetDrawer2D()->GetYaxis() )
                y = -y;
            ScrollWindowConstrained( 0, y );
        }
        break;
        case WXK_HOME:
        {
            double y;
            if ( GetDrawer2D()->GetYaxis() )
            {
                double dvy = GetUppY() * dy;
                y = m_virtm_maxY - dvy - GetVisibleMinY();
            }
            else
                y = m_virtm_minY - GetVisibleMinY();
            ScrollWindowConstrained( 0, y );
        }
        break;
        case WXK_END:
        {
            double y;
            if ( GetDrawer2D()->GetYaxis() )
                y = m_virtm_minY - GetVisibleMinY();
            else
            {
                double dvy = GetUppY() * dy;
                y = m_virtm_maxY - GetVisibleMinY() - dvy;
            }
            ScrollWindowConstrained( 0, y );
        }
        break;
        case WXK_UP:
        {
            double y;
            if  ( !event.ControlDown() )
                y = -m_scrollstepy;
            else
                y = -m_scrollstepy * m_thumby / m_normalize;

            if ( GetDrawer2D()->GetYaxis() )
                y = -y;

            ScrollWindowConstrained( 0, y );
        }
        break;

        case WXK_DOWN:
        {
            double y;
            if  ( !event.ControlDown() )
                y = m_scrollstepy;
            else
                y = m_scrollstepy * m_thumby / m_normalize;

            if ( GetDrawer2D()->GetYaxis() )
                y = -y;

            ScrollWindowConstrained( 0, y );
        }
        break;

        case WXK_LEFT:
        {
            double x;
            if  ( !event.ControlDown() )
                x = -m_scrollstepx;
            else
                x = -m_scrollstepx * m_thumbx / m_normalize;
            ScrollWindowConstrained( x, 0 );
        }
        break;
        case WXK_RIGHT:
        {
            double x;
            if  ( !event.ControlDown() )
                x = m_scrollstepx;
            else
                x = m_scrollstepx * m_thumbx / m_normalize;
            ScrollWindowConstrained( x, 0 );
        }
        break;
        default:
            // not for us
        {
            event.ResumePropagation( wxEVENT_PROPAGATE_MAX );
            event.Skip();
        }
    }
}

bool a2dOglCanvas::ScrollWindowConstrained( double dxworld, double dyworld )
{
    bool oke = true;

    // If any updates are pending, do them now since they will
    // expect the current mapping.

    //first redraw/update that were not yet updated pending areas (if available)
    //blit pending updates to the window/screen
    m_drawingPart->Update( a2dCANVIEW_UPDATE_AREAS );//| a2dCANVIEW_UPDATE_BLIT );

    double dx = -GetDrawer2D()->WorldToDeviceXRel( dxworld );
    double dy = -GetDrawer2D()->WorldToDeviceYRel( dyworld );

    //If dx/yworld is so small, that dx/y become zero, although dx/yworld!=0
    //we scroll at least one pixel
    if( dxworld != 0 && dx == 0 )
    {
        dx = -GetDrawer2D()->WorldToDeviceXRelNoRnd( dxworld ) < 0 ? -1 : 1;
    }
    if( dyworld != 0 && dy == 0 )
    {
        dy = -GetDrawer2D()->WorldToDeviceYRelNoRnd( dyworld ) < 0 ? -1 : 1;
    }

    //NOW based on the rounded dy which is an integer, set the dyworld again to have
    //proper mapping in combination with shifting contents of m_buffer.
    //THIS means we do not exactly step in the given amount of worldcoordinates,
    //but more a "pixel rounded" amount.
    dyworld = -GetDrawer2D()->DeviceToWorldYRel( floor( dy + 0.5 ) );
    dxworld = -GetDrawer2D()->DeviceToWorldXRel( floor( dx + 0.5 ) );

    //what are the new wanted minimum values in world coodinates for x an y
    double newworldminx = GetVisibleMinX() + dxworld;
    double newworldminy = GetVisibleMinY() + dyworld;

    GetDrawer2D()->StartRefreshDisplayDisable();
    GetDrawer2D()->SetMinY( newworldminy );
    GetDrawer2D()->SetMinX( newworldminx );
    if ( !MakeVirtualMaxFitVisibleArea() )
    {
        //the next does shift DIRECTLY the contents of the WINDOW and
        //will generate proper Onpaint events for area to re-blit from buffer
        // when rect = NULL total buffer will be blitted (fast enough)
        wxWindow::ScrollWindow( ( int ) dx, ( int ) dy, NULL );        
        GetDrawer2D()->SetMinX( newworldminx );
    }

    GetDrawer2D()->EndRefreshDisplayDisable();
    m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );
    // next would mean redraw in idel time, gives unpleasent result when draging canvas, only use if above too slow.
    //m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL );

    return oke;
}

void a2dOglCanvas::OnSize( wxSizeEvent& WXUNUSED( event ) )
{
    if ( IsFrozen() || !m_drawingPart )
        return;

    if ( !IsShownOnScreen() )
        return;

    //ignore size events due to scrollbars on off,
    //while window itself did not change in size
    //Those changes are detected when setting the scrollbars
    //Two separate size events or generated, so handle them one
    //by one
    if ( m_xscroll_changed == true )
    {
        m_xscroll_changed = false;
        return;
    }
    if ( m_yscroll_changed == true )
    {
        m_yscroll_changed = false;
        return;
    }

    SetSizeOgl();
}     

void a2dOglCanvas::SetSizeOgl()
{
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


    int clientw, clienth;
    int w, h;

    GetSaveClientSize( &clientw, &clienth );
    //the client size may be with are without scrollbars
    //always draw to a buffer that is inclusif scrollbars
    w = clientw;
    if ( m_has_y_scroll )
        w += wxSystemSettings::GetMetric( wxSYS_VSCROLL_X );
    h = clienth;
    if ( m_has_x_scroll )
        h += wxSystemSettings::GetMetric( wxSYS_HSCROLL_Y );

    //wxLogDebug( wxT("ow %d oh %d"), m_oldw, m_oldh );
    //wxLogDebug( wxT("w %d h %d"), w, h );

    //wxLogDebug( wxT("   change w %d h %d"), w, h );
    m_drawingPart->GetDrawer2D()->BeginDraw();
    m_drawingPart->SetBufferSize( w, h );
    m_drawingPart->GetDrawer2D()->EndDraw();

    glViewport(0, 0, (GLint) w, (GLint) h);
    if ( GetScaleOnResize() )
    {
        GetDrawer2D()->StartRefreshDisplayDisable();
        if ( m_prevClientW == 0 )
            m_prevClientW = clientw;
        if ( m_prevClientH == 0 )
            m_prevClientH = clienth;

        double worldoldw = GetDrawer2D()->GetVisibleWidth();
        double worldoldh = GetDrawer2D()->GetVisibleHeight();

        double upp;

        if ( clientw < clienth )
            upp = worldoldw / clientw;
        else
            upp = worldoldh / clienth;

        double middlexworld = GetDrawer2D()->DeviceToWorldX( m_prevClientW / 2.0 );
        double middleyworld = GetDrawer2D()->DeviceToWorldY( m_prevClientH / 2.0 );
        GetDrawer2D()->SetMappingDeviceRect( 0, 0, clientw, clienth, false );
        GetDrawer2D()->SetMappingUpp( middlexworld - clientw / 2.0 * upp, middleyworld - clienth / 2.0 * upp, upp, upp );

        MakeVirtualMaxFitVisibleArea();

        m_prevClientW = clientw;
        m_prevClientH = clienth;

        GetDrawer2D()->EndRefreshDisplayDisable();

        //although an onpaint event will follow, DO it now
        //since Onpaint ignores blits outside what is outside what he thinks
        //was damaged
        //This in case of Style wxNO_FULL_REPAINT_ON_RESIZE for wxFrame
        //which is needed for a2dOglCanvas
        m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );
    }
    else
    {
        GetDrawer2D()->StartRefreshDisplayDisable();
        GetDrawer2D()->SetMappingDeviceRect( 0, 0, clientw, clienth, true );

        MakeVirtualMaxFitVisibleArea();
        // now directly update those above areas and blit them to the window.
        // else we would get an ugly black border while sizing.
        m_drawingPart->Update( a2dCANVIEW_UPDATE_ALL | a2dCANVIEW_UPDATE_AREAS | a2dCANVIEW_UPDATE_BLIT );
        GetDrawer2D()->EndRefreshDisplayDisable();

        Refresh();
    }

    //also used for rescale!
    m_oldw = w; m_oldh = h;

    m_prevw = w;
    m_prevh = h;
}


void a2dOglCanvas::OnWheel( wxMouseEvent& event )
{
    int rot = event.GetWheelRotation();

    if ( rot > 0 )
    {
        if ( event.AltDown() )
        {
            if ( !event.ShiftDown() )
            {
                double y;
                if  ( !event.ControlDown() )
                    y = -m_scrollstepy;
                else
                    y = -m_scrollstepy * m_thumby / m_normalize;

                if ( GetDrawer2D()->GetYaxis() )
                    y = -y;

                ScrollWindowConstrained( 0, y );
            }
            else
            {
                double x;
                if  ( !event.ControlDown() )
                    x = -m_scrollstepx;
                else
                    x = -m_scrollstepx * m_thumbx / m_normalize;
                ScrollWindowConstrained( x, 0 );
            }
        }
        else
        {
            ZoomOutAtXy( event.m_x, event.m_y, 0.75 );
        }
    }
    else
    {
        if ( event.AltDown() )
        {
            if ( !event.ShiftDown() )
            {
                double y;
                if  ( !event.ControlDown() )
                    y = m_scrollstepy;
                else
                    y = m_scrollstepy * m_thumby / m_normalize;

                if ( GetDrawer2D()->GetYaxis() )
                    y = -y;

                ScrollWindowConstrained( 0, y );
            }
            else
            {
                double x;
                if  ( !event.ControlDown() )
                    x = m_scrollstepx;
                else
                    x = m_scrollstepx * m_thumbx / m_normalize;
                ScrollWindowConstrained( x, 0 );
            }
        }
        else
        {
            ZoomOutAtXy( event.m_x, event.m_y, 1.5 );
        }
    }
}

void a2dOglCanvas::OnActivate( wxActivateEvent& event )
{
}

void a2dOglCanvas::InitGL()
{
    if ( !IsShownOnScreen() )
        return;

    int w, h;
    GetClientSize(&w, &h);
}


void a2dOglCanvas::OnComEvent( a2dComEvent& event )
{
    if ( GetEvtHandlerEnabled() )
    {
        if ( event.GetId() == a2dDrawingPart::sig_swapBuffer )
        {
            SwapBuffers();
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}


#endif //wxUSE_GLCANVAS 

