/*! \file canvas/src/drawer.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: drawer.cpp,v 1.241 2009/09/30 18:38:59 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/dcprint.h>

#include <wx/print.h>

#include "wx/canvas/layerinf.h"
#include "wx/canvas/drawer.h"

#include "wx/canvas/canvas.h"
#include "wx/canvas/canvasogl.h"

#if wxART2D_USE_OPENGL && wxUSE_GLCANVAS 
#include "wx/canvas/canvasogl.h"
#endif //wxART2D_USE_OPENGL

#include "wx/canvas/cansim.h"
#include "wx/canvas/tools.h"
#include "wx/artbase/drawer2d.h"
#include "wx/artbase/dcdrawer.h"
#include "wx/canvas/canglob.h"
#include "wx/canvas/algos.h"
#include "wx/canvas/drawing.h"
#include "wx/canvas/cameleon.h"

#include <wx/wfstream.h>

#include <wx/stdstream.h>

#define CROSS_HAIR_X  300
#define CROSS_HAIR_Y  300

//----------------------------------------------------------------------------
// a2dCursorStack
//----------------------------------------------------------------------------

a2dCursorStack::a2dCursorStack()
{
}

//----------------------------------------------------------------------------
// a2dPushInStack
//----------------------------------------------------------------------------

a2dPushInStack::a2dPushInStack()
{
}

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------
WX_DEFINE_LIST( a2dUpdateListBase );


#ifdef CLASS_MEM_MANAGEMENT
a2dMemManager a2dUpdateList::sm_memManager( wxT( "a2dUpdateList memory manager" ) );
#endif //CLASS_MEM_MANAGEMENT

//----------------------------------------------------------------------------
// a2dTileBox
//----------------------------------------------------------------------------
a2dTileBox::a2dTileBox( int x1, int y1, int x2, int y2 )
{
    Init( x1, y1, x2, y2 );
}

a2dTileBox::~a2dTileBox()
{
}

void a2dTileBox::Init( int x1, int y1, int x2, int y2 )
{
    m_x1 = x1;
    m_y1 = y1;
    m_x2 = x2;
    m_y2 = y2;
    m_rectPrevRow = NULL;
    m_valid = false;
    if ( m_x1 != m_x2 && m_y1 != m_y2 )
        m_valid = true;
}

void a2dTileBox::Expand( int x1, int y1, int x2, int y2 )
{
    if ( !m_valid )
    {
        m_x1 = x1; m_y1 = y1;
        m_x2 = x2; m_y2 = y2;
    }
    else
    {
        m_x1 = wxMin( m_x1, x1 );
        m_x2 = wxMax( m_x2, x2 );
        m_y1 = wxMin( m_y1, y1 );
        m_y2 = wxMax( m_y2, y2 );
    }
    if ( m_x1 != m_x2 && m_y1 != m_y2 )
        m_valid = true;
}

//----------------------------------------------------------------------------
// a2dTiles
//----------------------------------------------------------------------------
a2dTiles::a2dTiles( int width, int height, wxUint8 id ): m_tiles( width* height )
{
    m_width = width;
    m_height = height;
    m_id = id;
    m_refcount = 0;
}

a2dTiles::~a2dTiles()
{
}

void a2dTiles::SetSize( int width, int height )
{
    m_tiles.resize( width * height );
    m_width = width;
    m_height = height;
}

void a2dTiles::Clear()
{
    int t;
    for ( t = 0; t < m_tiles.size(); t++ )
    {
        m_tiles[t].m_valid = false;
        m_tiles[t].m_rectPrevRow = NULL;
    }
}

bool a2dTiles::HasFilledTiles()
{
    int t;
    for ( t = 0; t < m_tiles.size(); t++ )
        if ( m_tiles[t].m_valid == true )
            return true;
    return false;
}

void a2dTiles::DrawTiles( a2dDrawer2D* drawer )
{
    int x, y;
    drawer->PushIdentityTransform();

    drawer->SetDrawerStroke( a2dStroke( wxColour( 0, 0, 0 ), 1  ) );

    for ( y = 0; y < m_height; y++ )
    {
        drawer->DrawLine( 0, toP( y ), toP( m_width ), toP( y ) );
        for ( x = 0; x < m_width; x++ )
        {
            drawer->DrawLine( toP( x ), 0, toP( x ), toP( m_height ) );
        }
    }

    drawer->SetDrawerStroke( a2dStroke( wxColour( 210, 22, 25 ), 2 ) );
    drawer->SetDrawerFill( a2dFill( wxColour( 10, 187, 15 ), a2dFILL_HORIZONTAL_HATCH ) );

    for ( y = 0; y < m_height; y++ )
    {
        for ( x = 0; x < m_width; x++ )
        {
            a2dTileBox box = m_tiles[ x + y * m_width ];
            if ( box.m_valid )
            {
                int x1 = toP( x ) + box.m_x1;
                int y1 = toP( y ) + box.m_y1;
                int x2 = toP( x ) + box.m_x2;
                int y2 = toP( y ) + box.m_y2;
                drawer->DrawRoundedRectangle( x1, y1, x2 - x1, y2 - y1, 0 );
            }
        }
    }
    drawer->PopTransform();
}

void a2dTiles::DrawRects( a2dDrawer2D* drawer )
{
    drawer->PushIdentityTransform();

    a2dUpdateList* a2dTiles = GenerateUpdateRectangles();

    a2dUpdateList::compatibility_iterator nodeb = a2dTiles->GetFirst();
    while ( nodeb )
    {
        a2dUpdateArea* uobj = nodeb->GetData();

        int x, y, width, height;
        x = uobj->x;
        y = uobj->y;
        width = uobj->width;
        height = uobj->height;

        drawer->DrawRoundedRectangle( x, y, width, height, 0 );

        nodeb = nodeb->GetNext();
    }

    drawer->PopTransform();
    a2dTiles->DeleteContents( true );
    delete a2dTiles;
}

void a2dTiles::FillTiles( const wxRect& rect, bool expand )
{
    FillTiles( rect.x, rect.y, rect.width, rect.height, expand );
}

void a2dTiles::FillTiles(  int xr, int yr, int w, int h, bool expand )
{
    int x, y, x1, y1, x2, y2; //maximum and minimum tile corners which rect fits in

    if ( !w || !h )
        return;

    int widthPix = m_width * a2d_TILESIZE;
    int heightPix = m_height * a2d_TILESIZE;
    // clip to buffer
    if ( xr < 0 )
    {
        w += xr;
        xr = 0;
    }
    if ( w <= 0 ) return;

    if ( yr < 0 )
    {
        h += yr;
        yr = 0;
    }
    if ( h <= 0 ) return;

    if ( xr + w > widthPix )
    {
        w = widthPix - xr;
    }
    if ( w <= 0 ) return;

    if ( yr + h > heightPix )
    {
        h = heightPix - yr;
    }
    if ( h <= 0 ) return;

    if ( !expand )
        Clear();

    x1 = toT( xr );
    y1 = toT( yr );
    x2 = toT( xr + w + a2d_TILESIZE );
    y2 = toT( yr + h + a2d_TILESIZE );

    int width = x2 - x1; //tiles to check in x
    int height = y2 - y1; //tiles to check in y

    int tx1, ty1, tx2, ty2; //rest at corner tiles.

    // get the rest at the corners of the rectangle, which partly covers 4 tiles.
    tx1 = ModT( xr );
    ty1 = ModT( yr );
    // E.G Tiles(0 -> 255) ( 0 ->255 ) == pixels (0 -> 611)
    tx2 = ModT( xr + w );
    ty2 = ModT( yr + h );
    if ( height == 1 )
    {
        if ( width == 1 )
            m_tiles[ x1 + y1 * m_width ].Expand( tx1, ty1, tx2, ty2 );
        else
        {
            int ti = x1 + y1 * m_width;
            m_tiles[ ti++ ].Expand( tx1, ty1, a2d_TILESIZE - 1, ty2 );
            for ( x = 1; x < width - 1; x++ )
                m_tiles[ ti++ ].Expand( 0, ty1, a2d_TILESIZE - 1, ty2 );
            m_tiles[ ti++ ].Expand( 0, ty1, tx2, ty2 );
        }
    }
    else
    {
        if ( width == 1 )
        {
            m_tiles[ x1 + y1 * m_width ].Expand( tx1, ty1, tx2, a2d_TILESIZE - 1 );
            for ( y = 1; y < height - 1; y++ )
                m_tiles[ x1 + ( y1 + y )*m_width ].Expand( tx1, 0, tx2, a2d_TILESIZE - 1 );
            m_tiles[ x1 + ( y1 + y )*m_width ].Expand( tx1, 0, tx2, ty2 );
        }
        else
        {
            //bottom row
            int ti = x1 + y1 * m_width;
            m_tiles[ ti++ ].Expand( tx1, ty1, a2d_TILESIZE - 1, a2d_TILESIZE - 1 );
            for ( x = 1; x < width - 1; x++ )
                m_tiles[ ti++ ].Expand(  0, ty1, a2d_TILESIZE - 1, a2d_TILESIZE - 1 );
            m_tiles[ ti++ ].Expand( 0, ty1, tx2, a2d_TILESIZE - 1 );

            //all in the middle rows
            for ( y = 1; y < height - 1; y++ )
            {
                ti = x1 + ( y1 + y ) * m_width;
                m_tiles[ ti++ ].Expand( tx1, 0, a2d_TILESIZE - 1, a2d_TILESIZE - 1 );
                for ( x = 1; x < width - 1; x++ )
                    m_tiles[ ti++ ].Expand(  0, 0, a2d_TILESIZE - 1, a2d_TILESIZE - 1 );
                m_tiles[ ti++ ].Expand( 0, 0, tx2, a2d_TILESIZE - 1 );
            }

            ti = x1 + ( y1 + y ) * m_width;
            //top row
            m_tiles[ ti++ ].Expand( tx1, 0, a2d_TILESIZE - 1, ty2 );
            for ( x = 1; x < width - 1; x++ )
                m_tiles[ ti++ ].Expand( 0, 0, a2d_TILESIZE - 1, ty2 );
            m_tiles[ ti++ ].Expand( 0, 0, tx2, ty2 );
        }
    }
}

a2dUpdateList* a2dTiles::GenerateUpdateRectangles()
{
    a2dUpdateList* rects = new a2dUpdateList();

    GenerateUpdateRectangles( rects );

    return rects;
}

void a2dTiles::GenerateUpdateRectangles( a2dUpdateList* rects )
{
    int x1, y1, x2, y2;

    a2dTileBox tile;
    //tile indexes
    int tilei, lefttilei;
    int x, y;
    tilei = 0;
    for ( y = 0; y < m_height; y++ )
    {
        for ( x = 0; x < m_width; x++ )
        {
            tile = m_tiles[ tilei ];
            if ( tile.m_valid )
            {
                x1 = toP( x ) + tile.m_x1;
                y1 = toP( y ) + tile.m_y1;
                y2 = toP( y ) + tile.m_y2;

                lefttilei = tilei;

                //extend rectangle to the right when this one is full in x and other is
                //continue-ing at same y1 and y2
                tile = m_tiles[tilei];
                while ( x != m_width - 1 &&
                        tile.m_x2 == ( a2d_TILESIZE - 1 ) && //this tile connect to next tile in x
                        m_tiles[tilei + 1].m_x1 == 0 &&
                        tile.m_y1 == m_tiles[tilei + 1].m_y1 &&
                        tile.m_y2 == m_tiles[tilei + 1].m_y2
                      )
                {
                    tilei++;
                    tile = m_tiles[tilei];
                    x++;
                }
                x2 = toP( x ) + tile.m_x2;

                //if we found some tiles filled ( and combined ) in this row?
                if ( ( x1 ^ x2 ) | ( y1 ^ y2 ) )
                {
                    a2dUpdateArea* prevRows = m_tiles[lefttilei].m_rectPrevRow;
                    if ( prevRows &&
                            x1 == prevRows->x &&
                            x2 == prevRows->x + prevRows->width  &&
                            y1 == prevRows->y + prevRows->height + 1 //only when y1 is at border to previous row
                       )
                    {
                        //extend lower rectangle with this one
                        prevRows->height = y2 - prevRows->y;
                    }
                    else
                    {
                        //add a new rectangle for output, and use it as prevRow pointer in coming row
                        prevRows = new a2dUpdateArea( x1, y1, x2 - x1, y2 - y1, m_id );
                        rects->Append( prevRows );
                    }

                    //store the found rect or NULL inside the coming row, in order to later merge with that row
                    // its found rectangles when possible.
                    if ( y != m_height - 1 )
                        m_tiles[lefttilei + m_width].m_rectPrevRow = prevRows;
                }
            }
            tilei++;
        }
    }
}

//----------------------------------------------------------------------------
// a2dTilesVec
//----------------------------------------------------------------------------

a2dTilesVec::a2dTilesVec()
{
}

a2dTilesVec::~a2dTilesVec()
{
}

void a2dTilesVec::SetSize( int width, int height )
{
    for ( iterator it= begin(); it!= end(); ++it)
    {
        a2dTiles* tiles = *it;
        tiles->SetSize( width, height );
    }
}

void a2dTilesVec::Clear()
{
    for ( iterator it= begin(); it!= end(); ++it)
    {
        a2dTiles* tiles = *it;
        tiles->Clear();
    }
}

void a2dTilesVec::GenerateUpdateRectangles( a2dUpdateList* rects )
{
    for ( iterator it= begin(); it!= end(); ++it)
    {
        a2dTiles* tiles = *it;
        tiles->GenerateUpdateRectangles( rects );
    }
}

void a2dTilesVec::FillTiles( int x, int y, int w, int h, bool expand )
{
    for ( iterator it= begin(); it!= end(); ++it)
    {
        a2dTiles* tiles = *it;
        tiles->FillTiles( x, y, w, h, expand );
    }
}

bool a2dTilesVec::HasFilledTiles()
{
    for ( iterator it= begin(); it!= end(); ++it)
    {
        a2dTiles* tiles = *it;
        if ( tiles->HasFilledTiles() )
            return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// a2dUpdateArea
//----------------------------------------------------------------------------
#ifdef CLASS_MEM_MANAGEMENT
a2dMemManager a2dUpdateArea::sm_memManager( wxT( "a2dUpdateArea memory manager" ) );
#endif //CLASS_MEM_MANAGEMENT

a2dUpdateArea::a2dUpdateArea( int x, int y, int width, int height, wxUint8 id ): wxRect( x, y, width, height )
{
    m_update_done = false;
    m_update_direct = false;
    m_id = id;
}

a2dUpdateArea::~a2dUpdateArea()
{
}



//----------------------------------------------------------------------------
// a2dDrawingPart
//----------------------------------------------------------------------------

a2dPropertyIdUint16* a2dDrawingPart::PROPID_drawstyle = NULL;
a2dPropertyIdBool* a2dDrawingPart::PROPID_gridlines = NULL;
a2dPropertyIdBool* a2dDrawingPart::PROPID_grid = NULL;
a2dPropertyIdBool* a2dDrawingPart::PROPID_showorigin = NULL;
a2dPropertyIdUint16* a2dDrawingPart::PROPID_hitmargin = NULL;
a2dPropertyIdBool* a2dDrawingPart::PROPID_gridatfront = NULL;
a2dPropertyIdUint16* a2dDrawingPart::PROPID_gridsize = NULL;
a2dPropertyIdUint16* a2dDrawingPart::PROPID_gridthres = NULL;
a2dPropertyIdDouble* a2dDrawingPart::PROPID_gridx = NULL;
a2dPropertyIdDouble* a2dDrawingPart::PROPID_gridy = NULL;

INITIALIZE_PROPERTIES( a2dDrawingPart, a2dObject )
{
    A2D_PROPID_D( a2dPropertyIdUint16, drawstyle, a2dFILLED );
    A2D_PROPID_M( a2dPropertyIdBool, a2dDrawingPart, gridlines, false, m_gridlines );
    A2D_PROPID_M( a2dPropertyIdBool, a2dDrawingPart, grid, false, m_grid );
    A2D_PROPID_M( a2dPropertyIdBool, a2dDrawingPart, showorigin, false, m_showorigin );
    A2D_PROPID_M( a2dPropertyIdBool, a2dDrawingPart, gridatfront, false, m_gridatfront );
    A2D_PROPID_M( a2dPropertyIdUint16, a2dDrawingPart, hitmargin, 0, m_hitmargin );
    A2D_PROPID_M( a2dPropertyIdUint16, a2dDrawingPart, gridsize, 0, m_gridsize );
    A2D_PROPID_M( a2dPropertyIdUint16, a2dDrawingPart, gridthres, 0, m_gridthres );
    A2D_PROPID_M( a2dPropertyIdDouble, a2dDrawingPart, gridx, 0, m_gridx );
    A2D_PROPID_M( a2dPropertyIdDouble, a2dDrawingPart, gridy, 0, m_gridy );
    return true;
}

const a2dSignal a2dDrawingPart::sig_changedLayers = wxNewId();
const a2dSignal a2dDrawingPart::sig_changedLayerAvailable = wxNewId();
const a2dSignal a2dDrawingPart::sig_changedLayerVisibleInView = wxNewId();
const a2dSignal a2dDrawingPart::sig_changedShowObject = wxNewId();
const a2dSignal a2dDrawingPart::sig_triggerNewView = wxNewId();
const a2dSignal a2dDrawingPart::sig_swapBuffer = wxNewId();

void a2dDrawingPart::OnPropertyChanged( const a2dPropertyId* WXUNUSED( id ) )
{
    Update();
}

IMPLEMENT_DYNAMIC_CLASS( a2dDrawingPart, a2dObject )

BEGIN_EVENT_TABLE( a2dDrawingPart, a2dObject )
    EVT_ENTER_WINDOW( a2dDrawingPart::OnEnter )
    EVT_IDLE( a2dDrawingPart::OnIdle )
    EVT_MOUSE_EVENTS( a2dDrawingPart::OnMouseEvent )
    EVT_CHAR( a2dDrawingPart::OnCharEvent )
    EVT_COM_EVENT( a2dDrawingPart::OnComEvent )
    //EVT_CLOSE_VIEW( a2dDrawingPart::OnCloseView )
#ifdef _DEBUG
    EVT_DO( a2dDrawingPart::OnDoEvent )
    EVT_UNDO( a2dDrawingPart::OnUndoEvent )
    EVT_REDO( a2dDrawingPart::OnRedoEvent )
#endif // _DEBUG
    EVT_BEGINBUSY( a2dDrawingPart::OnBusyBegin )
    EVT_ENDBUSY( a2dDrawingPart::OnBusyEnd )
	EVT_SET_FOCUS( a2dDrawingPart::OnSetFocus )
	EVT_KILL_FOCUS( a2dDrawingPart::OnKillFocus )
END_EVENT_TABLE()

a2dDrawingPart::a2dDrawingPart( const wxSize& size )
/*:
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_initCurrentSmartPointerOwner( this )
#endif
*/
{
    m_width = size.GetWidth();
    m_height = size.GetHeight();

    m_drawer2D  = new a2dMemDcDrawer( m_width, m_height );

    Init();
}

a2dDrawingPart::a2dDrawingPart( int width, int height )
/*:
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_initCurrentSmartPointerOwner( this )
#endif
*/
{
    m_width = width;
    m_height = height;

    m_drawer2D  = new a2dMemDcDrawer( m_width, m_height );

    Init();
}

a2dDrawingPart::a2dDrawingPart( int width, int height, a2dDrawer2D* drawer )
/*:
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_initCurrentSmartPointerOwner( this )
#endif
*/
{
    m_width = width;
    m_height = height;

    m_drawer2D  = drawer;
    m_drawer2D->DestroyClippingRegion();

    Init();
}

a2dDrawingPart::a2dDrawingPart( a2dDrawer2D* drawer )
/*:
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_initCurrentSmartPointerOwner( this )
#endif
*/
{
    m_drawer2D  = drawer;
    m_width = drawer->GetWidth();
    m_height = drawer->GetHeight();

    Init();
}

void a2dDrawingPart::Init()
{
    m_uppXyMax = 0;
    m_ignoreFocus = false;

    a2dHabitat* habitat = a2dCanvasGlobals->GetHabitat();

    m_hitmargin = habitat->GetHitMarginDevice();
    m_documentDrawStyle = habitat->GetDrawStyle() | habitat->GetSelectDrawStyle();
    m_selectFill = habitat->GetSelectFill();
    m_selectStroke = habitat->GetSelectStroke();
    m_select2Fill = habitat->GetSelect2Fill();
    m_select2Stroke = habitat->GetSelect2Stroke();
    m_reverse_order = habitat->GetReverseOrder();

    m_toolWantsIt = false;
    // wxLogDebug( "(a2dDrawingPart*)0x%p", this );
   
    m_update_available_layers = true;

    m_recur = false;
	m_drawingDisplay = NULL;
    //we want all layers to be rendered and selected objects.
    m_documentDrawStyleRestore = m_documentDrawStyle;
    m_fixFill = *a2dBLACK_FILL;
    m_fixStroke = *a2dBLACK_STROKE;

	if ( m_drawer2D && m_drawer2D->HasAlpha() )
    {
        m_highLightStroke = a2dStroke( wxColor( 0, 255, 255, 255 ), 2 );
        m_highLightFill = a2dFill( wxColor( 255, 122, 0, 165 ) );
    }
    else
    {
        m_highLightStroke = a2dStroke( wxColor( 0, 255, 255 ), 2 );
        m_highLightFill = *a2dTRANSPARENT_FILL;
    }

    if ( m_drawer2D )
    {
        m_drawer2D->SetDisplay( m_drawingDisplay );
    }

    m_toolcontroller = 0; //needed

    //mouse events handled by canvas itself
    m_mouseevents = true;

    m_top  = 0;
	m_border = 0;

    m_freezeCount = 0;
    m_thawUpdateAll = false;

    m_virtualarea_set = false;

    m_gridatfront = false;
    m_gridx = habitat->GetGridX();
    m_gridy = habitat->GetGridY();
    m_gridlines = false;
    m_gridthres = 5;
    m_gridsize = 1;
    m_grid = false;

    m_showorigin = true;

    m_backgroundfill = *a2dWHITE_FILL;
    m_gridstroke = *a2dBLACK_STROKE;
    m_gridfill = *a2dTRANSPARENT_FILL;

    m_endCorridorObject = ( a2dCanvasObject* )NULL;
    m_startCorridorObject = ( a2dCanvasObject* )NULL;

    m_crosshairStroke = a2dStroke( wxColour( 200, 30, 30 ), 0, a2dSTROKE_DOT );
    m_crosshair = false;
    m_crosshairx = 0;
    m_crosshairy = 0;
    m_crosshairLengthX = CROSS_HAIR_X;
    m_crosshairLengthY = CROSS_HAIR_Y;
    m_mouse_x = m_mouse_y = 0;

    m_printtitle = true;
    m_printfilename = true;
    m_printscalelimit = 0;
    m_printframe = true;
    m_printfittopage = false;

    m_viewDependentObjects = false;
#if defined(_DEBUG) 
    m_updatesVisible = false;
#endif
}

a2dDrawingPart::~a2dDrawingPart()
{
    if ( GetDrawing() && GetDrawing()->GetCanvasCommandProcessor() &&
         a2dCanvasGlobals->GetActiveDrawingPart() == this )
            a2dCanvasGlobals->SetActiveDrawingPart( NULL );

    //prevent event handling from now one ( else problem from event distribution can happen ).
    //Unregister is done in a2dView.
    SetEvtHandlerEnabled( false );

    m_drawingDisplay = NULL; //is used to test inside controllers.
	m_top = NULL;
    m_toolcontroller = 0;

    if ( m_drawer2D )
        delete m_drawer2D;
}

a2dDrawingPart::a2dDrawingPart( const a2dDrawingPart& other )
: a2dObject( other, a2dObject::clone_deep, NULL )
/*,
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    m_initCurrentSmartPointerOwner( this )
#endif
*/
{
    m_drawer2D  = NULL;

    Init();

    m_backgroundfill = other.m_backgroundfill;

    m_gridatfront = other.m_gridatfront;

    m_gridstroke = other.m_gridstroke;

    m_gridfill = other.m_gridfill;

    m_fixFill = other.m_fixFill;
    m_fixStroke = other.m_fixStroke;

    m_selectFill = other.m_selectFill;
    m_selectStroke = other.m_selectStroke;
    m_select2Fill = other.m_select2Fill;
    m_select2Stroke = other.m_select2Stroke;

    m_gridx = other.m_gridx;
    m_gridy = other.m_gridy;
    m_gridlines = other.m_gridlines;
    m_gridsize = other.m_gridsize;
    m_gridthres = other.m_gridthres;

    m_showorigin = other.m_showorigin;

    m_grid = other.m_grid;

    m_top  = other.m_top;

    m_crosshairStroke = other.m_crosshairStroke;
    m_crosshair = other.m_crosshair;
    m_crosshairx = other.m_crosshairx;
    m_crosshairy = other.m_crosshairy;
    m_crosshairLengthX = other.m_crosshairLengthX;
    m_crosshairLengthY = other.m_crosshairLengthY;

    m_printtitle = other.m_printtitle;
    m_printfilename = other.m_printfilename;
    m_printscalelimit = other.m_printscalelimit;
    m_printframe = other.m_printframe;
    m_printfittopage = other.m_printfittopage;

    m_documentDrawStyle = other.m_documentDrawStyle;
    m_reverse_order = other.m_reverse_order;

}

#if wxUSE_PRINTING_ARCHITECTURE
wxPrintout* a2dDrawingPart::OnCreatePrintout( a2dPrintWhat typeOfPrint, const wxPageSetupDialogData& pageSetupData )
{
    return new a2dDrawingPrintOut( pageSetupData, this, m_printtitle ? GetDrawing()->GetDescription() : ( const wxString& )wxEmptyString, "", typeOfPrint, false, false, false );
}
#endif

a2dCanvas* a2dDrawingPart::GetCanvas() const
{ 
    return wxStaticCast( m_drawingDisplay, a2dCanvas ); 
}

#if wxART2D_USE_OPENGL && wxUSE_GLCANVAS 
a2dOglCanvas* a2dDrawingPart::GetOglCanvas() const
{ 
    return wxStaticCast( m_drawingDisplay, a2dOglCanvas ); 
}
#endif //wxART2D_USE_OPENGL

a2dDrawing* a2dDrawingPart::GetDrawing() const
{
	if  ( m_top )
	   return m_top->GetRoot();
	return NULL;
}

void a2dDrawingPart::SetReverseOrder( bool revorder )
{
    m_reverse_order = revorder;
    Update( a2dCANVIEW_UPDATE_ALL );
}

double a2dDrawingPart::GetHitMarginWorld() const
{
    assert( m_drawer2D );
    return m_drawer2D->DeviceToWorldXRel( m_hitmargin );
}

void a2dDrawingPart::SetDisplayWindow( wxWindow* display )
{
	m_drawingDisplay = display;
    if ( m_drawingDisplay )
        m_drawingDisplay->SetDropTarget( new a2dDnDCanvasObjectDropTarget( this ));

    if ( m_drawer2D )
        m_drawer2D->SetDisplay( display );
}

void a2dDrawingPart::SetBufferSize( int w, int h )
{
    m_width = w;
    m_height = h;
    if ( m_drawer2D )
        m_drawer2D->SetBufferSize( w, h );
}

void a2dDrawingPart::SetDrawer2D( a2dDrawer2D* drawer2d, bool noDelete )
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


bool a2dDrawingPart::SetCanvasToolContr( a2dToolContr* controller )
{
    a2dToolContr* returnc = m_toolcontroller;

    if ( m_toolcontroller ) //remove the old one
    {
        wxASSERT_MSG( m_toolcontroller->GetDrawingPart() == this, wxT( "a2dToolContr has different a2dDrawingPart set for it" ) );
        m_toolcontroller->Disable();
    }

    if ( controller ) //set the new one
    {
        wxASSERT_MSG( controller->GetDrawingPart() == this, wxT( "a2dToolContr has different a2dDrawingPart set for it" ) );
    }
    m_toolcontroller = controller;
    return returnc != 0;
}

bool a2dDrawingPart::ProcessEvent( wxEvent& event )
{
    bool processed = false;

    // An event handler can be enabled or disabled
    if ( GetEvtHandlerEnabled() )
    {
        //prevent the object from being deleted until this process event is at an end
        a2dREFOBJECTPTR_KEEPALIVE;

        if ( event.GetEventType() == wxEVT_IDLE )
        {
            if ( m_top && m_top->GetRoot() )
                processed = m_top->GetRoot()->ProcessEvent( event );

            event.Skip(); 
            processed = a2dObject::ProcessEvent( event );
            if ( m_toolcontroller )
            {
                event.Skip(); //skipped because not processed so prepare for controller
                processed = m_toolcontroller->ProcessEvent( event );
            }
        }
        else if ( m_toolcontroller && event.GetEventType() == wxEVT_PAINT )
        {
            if ( m_drawingDisplay )
            {
                SetEvtHandlerEnabled( false ); //this event coming back from the display will not be handled
                event.ResumePropagation( wxEVENT_PROPAGATE_MAX );
                m_drawingDisplay->GetEventHandler()->ProcessEvent( event );
                event.StopPropagation();
                SetEvtHandlerEnabled( true );
            }
            processed = a2dObject::ProcessEvent( event );
            if ( !processed || m_toolWantsIt )
            {
                event.Skip(); //skipped because not processed so prepare for controller
                processed = m_toolcontroller->ProcessEvent( event );
                m_toolWantsIt = false;
            }
        }
        else if ( m_toolcontroller && event.GetEventType() == a2dEVT_COM_EVENT )
        {
            // communication event of changes in state, should always continue
            processed = m_toolcontroller->ProcessEvent( event );
            event.Skip(); //skipped because not processed so prepare for view
            processed = a2dObject::ProcessEvent( event );
        }
        else if ( m_toolcontroller && event.GetEventType() == wxEVT_CHAR )
        {
            processed = m_toolcontroller->ProcessEvent( event );
            if ( !processed || m_toolWantsIt )
            {
                event.Skip(); //skipped because not processed so prepare for view
                processed = a2dObject::ProcessEvent( event );
                m_toolWantsIt = false;
            }
        }
        else if ( m_toolcontroller && event.GetEventType() == wxEVT_KEY_DOWN )
        {
            processed = m_toolcontroller->ProcessEvent( event );
            if ( !processed || m_toolWantsIt )
            {
                event.Skip(); //skipped because not processed so prepare for view
                processed = a2dObject::ProcessEvent( event );
                m_toolWantsIt = false;
            }
        }
        else if ( m_toolcontroller && event.GetEventType() == wxEVT_KEY_UP )
        {
            processed = m_toolcontroller->ProcessEvent( event );
            if ( !processed || m_toolWantsIt )
            {
                event.Skip(); //skipped because not processed so prepare for view
                processed = a2dObject::ProcessEvent( event );
                m_toolWantsIt = false;
            }
        }
        else if ( m_toolcontroller )
        {
            processed = m_toolcontroller->ProcessEvent( event );
            if ( !processed || m_toolWantsIt )
            {
                event.Skip(); //skipped because not processed so prepare for view
                processed = a2dObject::ProcessEvent( event );
                m_toolWantsIt = false;
            }
        }
        else
        {
            processed = a2dObject::ProcessEvent( event );
        }

        return processed;
    }

    return false;
}

void a2dDrawingPart::OnEnter( wxMouseEvent& event )
{

    //if ( GetDisplayWindow() )
    //GetDisplayWindow()->SetFocus();

    event.Skip();
}

void a2dDrawingPart::OnIdle( wxIdleEvent& event )
{
/*
    //wxLogDebug( "void a2dDrawingPart::OnIdle(wxIdleEvent &event)" );

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
    if ( m_updateareas.GetCount() )
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
*/
    event.Skip(); //to the display of this view.
}

void a2dDrawingPart::OnSetFocus( wxFocusEvent& focusevent )
{
    //wxLogDebug( "focus %p", this );

    if ( !m_ignoreFocus && GetDrawing() )
    {
		a2dCanvasGlobals->SetActiveDrawingPart( this );
    }
	focusevent.Skip();
}

void a2dDrawingPart::OnKillFocus( wxFocusEvent& focusevent )
{
    //wxLogDebug( "off focus %p", this );
    if ( !m_ignoreFocus && GetDrawing() && GetDrawing()->GetCanvasCommandProcessor() )
    {
        // keep this one active until some other becomes active. (e.g. Dialog uses this a2dDrawingPart, while not InFocus)
		//GetDrawing()->GetCanvasCommandProcessor()->SetActiveDrawingPart( NULL );
    }
	focusevent.Skip();
}

void a2dDrawingPart::SetDocumentDrawStyle( wxUint32 documentDrawStyle )
{
    m_documentDrawStyleRestore = m_documentDrawStyle; m_documentDrawStyle = documentDrawStyle;
}

void a2dDrawingPart::Freeze()
{
    if ( !m_freezeCount++ )
    {
        // was not frozen yet (if something extra needs to be done when freezing)
    }
}

void a2dDrawingPart::Thaw( bool update )
{
    wxASSERT_MSG( m_freezeCount, "Thaw() without matching Freeze()" );

    if ( update )
        m_thawUpdateAll = true;

    if ( !--m_freezeCount )
    {
        // is now thawed (if something needs to be done when thawing)

        if ( m_thawUpdateAll )
        {
            Update( a2dCANVIEW_UPDATE_ALL );
        }
        m_thawUpdateAll = false;
    }
}

void a2dDrawingPart::SetCorridor( a2dCanvasObject* start, a2dCanvasObject* end )
{
    //wxLogDebug( wxT( "start=%p end=%p" ), start, end);
    if ( m_startCorridorObject )
        m_startCorridorObject->SetIsOnCorridorPath( false );
    if ( m_endCorridorObject )
        m_endCorridorObject->SetIsOnCorridorPath( false );

    m_startCorridorObject = start;
    if ( m_startCorridorObject )
        m_startCorridorObject->SetIsOnCorridorPath( true );
    m_endCorridorObject = end;
    if ( m_endCorridorObject )
        m_endCorridorObject->SetIsOnCorridorPath( true );

    if ( wxDynamicCast( start, a2dHandle ) )
            m_startCorridorObject = start;
}

void a2dDrawingPart::SetCursor( const wxCursor& cursor )
{
    //first pop
    if ( m_cursorStack.size() )
        m_cursorStack.pop_back();
    m_cursorStack.push_back( cursor );
    if ( m_drawingDisplay )
        m_drawingDisplay->SetCursor( cursor );
}

const wxCursor& a2dDrawingPart::GetCursor() const
{
    if ( m_cursorStack.size() )
        return m_cursorStack.back();
    return wxNullCursor;
}

void a2dDrawingPart::PushCursor( const wxCursor& cursor )
{
    m_cursorStack.push_back( cursor );
    if ( m_drawingDisplay )
        m_drawingDisplay->SetCursor( cursor );
}

void a2dDrawingPart::PopCursor()
{
    if ( m_cursorStack.size() )
    {
        m_cursorStack.pop_back();
        if ( m_drawingDisplay )
        {
            if ( m_cursorStack.size() )
                m_drawingDisplay->SetCursor( m_cursorStack.back() );
            else
                m_drawingDisplay->SetCursor( wxNullCursor );
        }
    }
}

void a2dDrawingPart::ClearCursorStack()
{
    m_cursorStack.clear();
    if ( m_drawingDisplay )
        m_drawingDisplay->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_ARROW  )  );
}

void a2dDrawingPart::SetMouseEvents( bool onoff )
{
    m_mouseevents = onoff;
}

void a2dDrawingPart::SetCrossHairStroke( const a2dStroke& crosshairStroke )
{
    m_crosshairStroke = crosshairStroke;
};

void a2dDrawingPart::SetCrossHair( bool onoff )
{
    m_crosshair = onoff;
    Update( a2dCANVIEW_UPDATE_ALL );
}

void a2dDrawingPart::MouseToToolWorld( int x, int y, double& xWorldLocal, double& yWorldLocal )
{
    if ( wxDynamicCast( GetDisplayWindow(), a2dCanvas ) )
    {
        //a2dCanvas is aligned with view its buffer origin
    }
    else if ( wxDynamicCast( GetDisplayWindow(), a2dCanvasSim ) )
    {
        //a2dCanvasSim its virtual origin is aligned with view its buffer origin
        a2dCanvasSim* can = wxStaticCast( GetDisplayWindow(), a2dCanvasSim );
        can->CalcUnscrolledPosition( x, y, &x , &y );
    }

    xWorldLocal = GetDrawer2D()->DeviceToWorldX( x );
    yWorldLocal = GetDrawer2D()->DeviceToWorldY( y );
}

void a2dDrawingPart::ToolWorldToMouse( double xWorld, double yWorld, int& x, int& y  )
{
    x = GetDrawer2D()->WorldToDeviceX( xWorld );
    y = GetDrawer2D()->WorldToDeviceY( yWorld );

    if ( wxDynamicCast( GetDisplayWindow(), a2dCanvas ) )
    {
        //a2dCanvas is aligned with view its buffer origin
    }
    else if ( wxDynamicCast( GetDisplayWindow(), a2dCanvasSim ) )
    {
        //a2dCanvasSim its virtual origin is aligned with view its buffer origin
        a2dCanvasSim* can = wxStaticCast( GetDisplayWindow(), a2dCanvasSim );
        can->CalcScrolledPosition( x, y, &x , &y );
    }
}

void a2dDrawingPart::OnMouseEvent( wxMouseEvent& event )
{
    m_mouse_x = event.GetX();
    m_mouse_y = event.GetY();

    UpdateCrossHair( m_mouse_x, m_mouse_y );

    if ( !GetDrawing() || !m_mouseevents )
    {
        event.Skip();
        return;
    }

    //to world coordinates to do hit test in world coordinates
    double xw = m_drawer2D->DeviceToWorldX( m_mouse_x );
    double yw = m_drawer2D->DeviceToWorldY( m_mouse_y );

    // Why needed??
    //if ( m_capture && !GetDrawing()->GetRootObject()->Find( m_capture ) )
    //    m_capture = ( a2dCanvasObject* ) NULL;

    if ( m_startCorridorObject && m_startCorridorObject != m_top )
    {
        a2dIterC ic( this );
        a2dHitEvent hitinfot( xw, yw );
        hitinfot.m_event = &event;
        if ( m_startCorridorObject->ProcessCanvasObjectEvent( ic, hitinfot ) )
            return;
    }
    else
    {
        a2dIterC ic( this );
        a2dAffineMatrix cworld = GetShowObject()->GetTransformMatrix();
        cworld.Invert();
        a2dIterCU cu( ic, cworld );
        ic.SetPerLayerMode( false );
        ic.SetLayer( wxLAYER_ALL );
        a2dHitEvent hitinfo( xw, yw );
        hitinfo.m_event = &event;
        if ( GetShowObject()->ProcessCanvasObjectEvent( ic, hitinfo ) )
            return;
    }
    event.Skip(); //windows containing view may handle it
}

void a2dDrawingPart::OnCharEvent( wxKeyEvent& event )
{
    if ( m_endCorridorObject )
        m_endCorridorObject->ProcessEvent( event );
    else
        event.Skip(); // to base class, but also a2dCanvas and tools.
}

bool a2dDrawingPart::ProcessCanvasObjectEvent( wxEvent& event, bool& isHit,
        double x, double y, int margin,
        int layer )
{
	
    a2dIterC ict( this );
    ict.SetHitMarginDevice( margin );
    ict.SetPerLayerMode( false );
    ict.SetLayer( wxLAYER_ALL );

    a2dHitEvent hitinfot( x, y );
    hitinfot.m_event = &event;

    if ( m_startCorridorObject && m_startCorridorObject != m_top )
    {
        //rotateable view experiment
        //cworld.Rotate( -30 );

        //we start at m_startCorridorObject, so m_top its matrix of no interest here.
        bool res = m_startCorridorObject->ProcessCanvasObjectEvent( ict, hitinfot );
        isHit = hitinfot.m_isHit;

        return res;
    }
    else
    {   //we start at m_top, need to invert its own matrix, as applied in ProcessCanvasObjectEvent
        a2dAffineMatrix tcworld = GetShowObject()->GetTransformMatrix();
        tcworld.Invert();
        a2dIterCU cut( ict, tcworld );
        bool res = GetShowObject()->ProcessCanvasObjectEvent( ict, hitinfot );
        isHit = hitinfot.m_isHit;
        return res;
    }	

    if ( !GetDrawing() )
    {
        event.Skip();
        return false;
    }

    return false;
}

bool a2dDrawingPart::FindAndSetCorridorPath( a2dCanvasObject* findObject )
{
    a2dWalker_FindAndSetCorridorPath findcorridor( findObject );
    findcorridor.SetSkipNotRenderedInDrawing( true );
    findcorridor.Start( GetShowObject() );

    if( findcorridor.GetResult() )
    {
        a2dCanvasObject* start = NULL;
        a2dCanvasObjectList::iterator iter = m_top->GetChildObjectList()->begin();
        while( iter != m_top->GetChildObjectList()->end() )
        {
            a2dCanvasObject* obj = ( *iter );
            if ( obj->GetIsOnCorridorPath() )
            {
                start = obj;
                break;
            }
            iter++;
        }

        SetCorridor( start, findObject );
        return true;
    }
    return false;
}

void a2dDrawingPart::SetCorridorPath( const a2dCorridor& corridor )
{
    ClearCorridorPath();
    a2dCanvasObjectList::const_iterator iter = corridor.begin();
    while( iter != corridor.end() )
    {
        a2dCanvasObjectList::value_type obj = *iter;
        if (obj )
        {
            obj->SetIsOnCorridorPath( true );
        }
        iter++;
    }
    if ( !corridor.empty() )
        SetCorridor( corridor.front(), corridor.back() );
}

void a2dDrawingPart::ClearCorridorPath()
{
    if ( m_startCorridorObject )
    {
        SetCorridor( NULL, NULL );

        if ( GetShowObject() )
        {
            a2dWalker_SetSpecificFlagsCanvasObjects setflags( a2dCanvasOFlags::IsOnCorridorPath );
            setflags.SetSkipNotRenderedInDrawing( true );
            setflags.Start( GetShowObject(), false );
        }
    }
}

a2dCanvasObject* a2dDrawingPart::IsHitWorld(
    double x, double y,
    int layer,
    a2dHitOption option
)
{
    Update( a2dCANVIEW_UPDATE_PENDING );
    a2dIterC ic( this );
    ic.SetPerLayerMode( true );
    if ( layer == wxLAYER_ALL )
    ic.SetPerLayerMode( false );
    ic.SetLayer( layer );
    ic.SetObjectFilter( new a2dCanvasObjectFilterLayerMask( layer, a2dCanvasOFlags::VISIBLE | a2dCanvasOFlags::SELECTABLE ) );

    a2dHitEvent hitinfo( x, y );
    hitinfo.m_option = option;
    //at the top level the group its matrix is to be ignored.
    //Since it is normally NOT ignored within a2dCanvasObject, force an inverse.
    hitinfo.m_xyRelToChildren = true;

    a2dCanvasObject* res = GetShowObject()->IsHitWorld( ic, hitinfo );
    return res;
}

a2dCanvasObject* a2dDrawingPart::IsHitWorld(
    a2dHitEvent& hitEvent,
    int layer
)
{
    Update( a2dCANVIEW_UPDATE_PENDING );
    a2dIterC ic( this );
    ic.SetLayer( layer );
    a2dCanvasObject* res = GetShowObject()->IsHitWorld( ic, hitEvent );
    return res;
}

void a2dDrawingPart::SetLayerCheck( wxUint16 layer )
{
    m_layerRenderArray[ layer ].SetCheck( true );
    Set_UpdateAvailableLayers( true ); //see OnIdle
}

void a2dDrawingPart::SetAvailable()
{
    m_layerRenderArray.clear();
    if ( m_top )
    {
        a2dWalker_SetAvailable set( this );
        set.SetSkipNotRenderedInDrawing( true );
        set.Start( m_top );
    }
    //this one always available because editing handles are on it.
    m_layerRenderArray[wxLAYER_DEFAULT].SetAvailable( true );
    Set_UpdateAvailableLayers( false );
}

/*
void a2dDrawingPart::OnCloseView( a2dCloseViewEvent& event )
{
    //when a view is closed, and a controller, and its tools or still active,
    //they need to be terminated and removed.

    if ( m_toolcontroller.Get() )
        m_toolcontroller->Disable();
    m_toolcontroller = 0;
    event.Skip();
}
*/

void a2dDrawingPart::OnComEvent( a2dComEvent& event )
{
    if ( GetEvtHandlerEnabled() )
    {
        if ( event.GetId() == sig_changedLayers )
        {
            Set_UpdateAvailableLayers( true );
        }
        else if ( event.GetId() == a2dDrawing::sig_changedLayer )
        {
            a2dDrawing* drawing = wxDynamicCast( event.GetEventObject(), a2dDrawing );

            if ( drawing == GetDrawing() )
            {
                a2dNamedProperty* property = event.GetProperty();
                if ( property->GetUint16() != wxLAYER_ALL )
                {
                    m_layerRenderArray[property->GetUint16()].SetCheck( true );
                    //m_layerRenderArray[property->GetUint16()].SetAvailable( true );
                }
                else
                    Set_UpdateAvailableLayers( true );
            }
        }
        else if ( event.GetId() == a2dDrawing::sig_layersetupChanged )
        {
            a2dDrawing* drawing = wxDynamicCast( event.GetEventObject(), a2dDrawing );
            if ( drawing == GetDrawing() )
                Update( a2dCANVIEW_UPDATE_ALL );
            //Set_UpdateAvailableLayers(true);
        }
        else if ( event.GetId() == a2dLayerInfo::sig_changedLayerInfo )
        {
            a2dCanvasObject* obj = wxDynamicCast( event.GetEventObject(), a2dCanvasObject );
            if ( m_top == obj->GetRoot()->GetRootObject() )
                Update( a2dCANVIEW_UPDATE_ALL );
        }
        else
            event.Skip();
    }
    else
        event.Skip();
}

#ifdef _DEBUG
void a2dDrawingPart::OnDoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}

void a2dDrawingPart::OnRedoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}

void a2dDrawingPart::OnUndoEvent( a2dCommandProcessorEvent& event )
{
    event.Skip();
}
#endif // _DEBUG

void a2dDrawingPart::OnBusyBegin( a2dCommandProcessorEvent& event )
{
    event.SetCursorSet( true );
    PushCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_WAIT ) );
    //wxLogDebug( wxT("true") );
}

void a2dDrawingPart::OnBusyEnd( a2dCommandProcessorEvent& event )
{
    event.SetCursorSet( true );
    PopCursor();
    //wxLogDebug( wxT(" false") );
}

void a2dDrawingPart::Update( unsigned int how, wxObject* hintObject )
{
    if ( !m_top || !GetDrawing() )
        return;

    a2dDrawingEvent event( how, GetDrawing() );
    ProcessEvent( event );
}

void a2dDrawingPart::UpdateArea( int x, int y, int width, int height, wxUint8 id )
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

#if wxART2D_USE_OPENGL && wxUSE_GLCANVAS 
    if ( a2dOglCanvas* can = wxDynamicCast( m_drawingDisplay, a2dOglCanvas ) )
    {
        // Flush
        glFlush();
    }
#endif //wxART2D_USE_OPENGL

    /*
        if ( m_drawingDisplay && m_drawingDisplay->GetChildren().GetCount() )
        {
            wxRect rect = wxRect(x, y, width, height);
            m_drawingDisplay->Refresh( false, &rect );
        }
    */

}

void a2dDrawingPart::ClearArea( int x, int y, int width, int height )
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

    m_drawer2D->SetClippingRegionDev( x, y, width, height );

    PaintBackground( x, y, width, height );
    if ( m_grid )
        PaintGrid( x, y, width, height );

    m_drawer2D->DestroyClippingRegion();
    DrawOrigin();

    m_drawer2D->EndDraw();
}

void a2dDrawingPart::BlitBuffer( int x, int y, int width, int height, int xbuf, int ybuf )
{
    if ( !GetDrawing() )
    {
        //wxASSERT_MSG( GetDrawing() != 0, wxT( "invalid a2dDrawing in a2dDrawingPart" ) );
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

    m_drawer2D->BlitBuffer( x, y, width, height , xbuf, ybuf );

    m_drawer2D->EndDraw();
}

void a2dDrawingPart::UpdateViewDependentObjects()
{
    if ( ! m_viewDependentObjects )
        return;

    a2dDrawing* drawing = GetDrawing();
    if( drawing )
    {
        if( !m_top )
            return;
        a2dIterC ic( this );
        a2dAffineMatrix cworld = m_top->GetTransformMatrix();
        cworld.Invert();
        a2dIterCU cu( ic, cworld );
        m_top->UpdateViewDependentObjects( ic );
    }
}

void a2dDrawingPart::RenderTopObject( wxUint32 documentDrawStyle, wxUint8 id )
{
    if ( documentDrawStyle & RenderLAYERED_All )
    {
        m_drawer2D->SetDrawStyle( a2dFILLED );
        RenderTopObjectOneStyle( a2dCanvasOFlags::VISIBLE, RenderLAYERED_All, id );
    }
    if ( documentDrawStyle & RenderLAYERED )
    {
        m_drawer2D->SetDrawStyle( a2dFILLED );
        RenderTopObjectOneStyle( a2dCanvasOFlags::VISIBLE, RenderLAYERED, id );
    }
    if ( documentDrawStyle & RenderWIREFRAME )
    {
        m_drawer2D->SetDrawStyle( a2dWIREFRAME );
        RenderTopObjectOneStyle( a2dCanvasOFlags::VISIBLE, RenderWIREFRAME, id );
    }
    if ( documentDrawStyle & RenderWIREFRAME_ZERO_WIDTH )
    {
        m_drawer2D->SetDrawStyle( a2dWIREFRAME_ZERO_WIDTH );
        RenderTopObjectOneStyle( a2dCanvasOFlags::VISIBLE, RenderWIREFRAME_ZERO_WIDTH, id );
    }
    if ( documentDrawStyle & RenderWIREFRAME_INVERT )
    {
        m_drawer2D->SetDrawStyle( a2dWIREFRAME_INVERT );
        RenderTopObjectOneStyle( a2dCanvasOFlags::VISIBLE, RenderWIREFRAME_INVERT, id );
    }
    if ( documentDrawStyle & RenderWIREFRAME_INVERT_ZERO_WIDTH )
    {
        m_drawer2D->SetDrawStyle( a2dWIREFRAME_INVERT_ZERO_WIDTH );
        RenderTopObjectOneStyle( a2dCanvasOFlags::VISIBLE, RenderWIREFRAME_INVERT_ZERO_WIDTH, id );
    }
    if ( documentDrawStyle & RenderFIX_STYLE )
    {
        m_drawer2D->SetDrawerFill( m_fixFill );
        m_drawer2D->SetDrawerStroke( m_fixStroke );
        m_drawer2D->SetDrawStyle( a2dFIX_STYLE );
        RenderTopObjectOneStyle( a2dCanvasOFlags::VISIBLE, RenderFIX_STYLE, id );
        m_drawer2D->ResetFixedStyle();
    }
    if ( documentDrawStyle & RenderRectangles )
    {
        m_drawer2D->SetDrawerFill( m_fixFill );
        m_drawer2D->SetDrawerStroke( m_fixStroke );
        m_drawer2D->SetDrawStyle( a2dFIX_STYLE );
        RenderTopObjectOneStyle( a2dCanvasOFlags::VISIBLE, RenderRectangles, id );
        m_drawer2D->ResetFixedStyle();
    }

    if ( documentDrawStyle & RenderWIREFRAME_HighLight )
    {
        m_drawer2D->SetDrawerFill( m_highLightFill );
        m_drawer2D->SetDrawerStroke( m_highLightStroke );
        m_drawer2D->SetDrawStyle( a2dFIX_STYLE );
        RenderTopObjectOneStyle(  a2dCanvasOFlags::VISIBLE, RenderWIREFRAME_HighLight, id );
        m_drawer2D->ResetFixedStyle();
    }

    if ( documentDrawStyle & RenderTOOL_OBJECTS )
    {
        //we want the object that is being edited, on top of all others, but before selection
        m_drawer2D->SetDrawerFill( m_fixFill );
        m_drawer2D->SetDrawerStroke( m_fixStroke );
        m_drawer2D->SetDrawStyle( a2dFIX_STYLE );
        RenderTopObjectOneStyle(  a2dCanvasOFlags::VISIBLE, RenderTOOL_OBJECTS, id );
        m_drawer2D->ResetFixedStyle();
    }

    if ( documentDrawStyle & RenderTOOL_OBJECTS_STYLED )
    {
        m_drawer2D->SetDrawStyle( a2dFILLED );
        RenderTopObjectOneStyle(  a2dCanvasOFlags::VISIBLE, RenderTOOL_OBJECTS_STYLED, id );
    }

    if ( documentDrawStyle & RenderRectangleTOOL_OBJECTS )
    {
        m_drawer2D->SetDrawerFill( m_fixFill );
        m_drawer2D->SetDrawerStroke( m_fixStroke );
        m_drawer2D->SetDrawStyle( a2dFIX_STYLE );
        RenderTopObjectOneStyle(  a2dCanvasOFlags::VISIBLE, RenderRectangleTOOL_OBJECTS, id );
        m_drawer2D->ResetFixedStyle();
    }

    if ( documentDrawStyle & RenderTOOL_DECORATIONS )
    {
        m_drawer2D->SetDrawStyle( a2dFILLED );
        RenderTopObjectOneStyle(  a2dCanvasOFlags::VISIBLE, RenderTOOL_DECORATIONS, id );
    }

    //we want the objects that are selected, in a select color, on top of all others, that is done next.
    if ( documentDrawStyle & RenderWIREFRAME_SELECT )
    {
        m_drawer2D->SetDrawerFill( m_selectFill );
        m_drawer2D->SetDrawerStroke( m_selectStroke );
        m_drawer2D->SetDrawStyle( a2dFIX_STYLE );
        RenderTopObjectOneStyle(  a2dCanvasOFlags::VISIBLE, RenderWIREFRAME_SELECT, id );
        m_drawer2D->ResetFixedStyle();
    }
    if ( documentDrawStyle & RenderWIREFRAME_SELECT_INVERT )
    {
        m_drawer2D->SetDrawerFill( m_selectFill );
        m_drawer2D->SetDrawerStroke( m_selectStroke );
        m_drawer2D->SetDrawStyle( a2dFIX_STYLE_INVERT );
        RenderTopObjectOneStyle(  a2dCanvasOFlags::VISIBLE, RenderWIREFRAME_SELECT_INVERT, id );
        m_drawer2D->ResetFixedStyle();
    }
    if ( documentDrawStyle & RenderWIREFRAME_SELECT2 )
    {
        m_drawer2D->SetDrawerFill( m_select2Fill );
        m_drawer2D->SetDrawerStroke( m_select2Stroke );
        m_drawer2D->SetDrawStyle( a2dFIX_STYLE );
        RenderTopObjectOneStyle(  a2dCanvasOFlags::VISIBLE, RenderWIREFRAME_SELECT2, id );
        m_drawer2D->ResetFixedStyle();
    }

    m_drawer2D->SetDrawStyle( a2dFILLED );
}

void a2dDrawingPart::RenderTopObjectOneStyle( a2dCanvasObjectFlagsMask mask, a2dDocumentRenderStyle drawstyle, wxUint8 id )
{
    if ( GetDrawing()->GetUpdatesPending() )
        GetDrawing()->AddPendingUpdatesOldNew(); //in general there are non, since the pending object are already updates before calling render

    a2dIterC ic( this );
    ic.SetDisableInvert( true );
    ic.SetDrawStyle( drawstyle );

    //at the top level the group its matrix is to be ignored.
    //Since it is normally NOT ignored within a2dCanvasObject, do an inverse first here.
    a2dAffineMatrix cworld = m_top->GetTransformMatrix();
    cworld.Invert();
    //rotateable view experiment
    //cworld.Rotate( -30 );

    // all special object (like tool objects or on top.
    m_drawer2D->SetLayer( 0 );

    a2dBoundingBox absarea = m_top->GetMappedBbox( cworld );
    OVERLAP clipparent = m_drawer2D->GetClippingBox().Intersect( absarea );
    if ( clipparent != _OUT )
    {
        if ( drawstyle == RenderLAYERED_All )
        {
            ic.SetPerLayerMode( false );
            wxUint16 layer = 0;
            if ( !GetDrawing()->GetLayerSetup() )
            {
                //wxLogDebug( "layer %d", layerobj->GetLayer() );
                m_drawer2D->ResetStyle();
                ic.SetObjectFilter( new a2dCanvasObjectFilterLayerMaskNoToolNoEdit( layer, mask ) );
                ic.SetLayer( layer );

                a2dIterCU cu( ic, cworld );
                m_top->Render( ic, clipparent );

                ic.Reset();
            }
            else
            {
                //wxLAYER_ALL is reserved and should never be part of the layersettings
                m_drawer2D->ResetStyle();
                ic.SetObjectFilter( new a2dCanvasObjectFilterLayerMaskNoToolNoEdit( wxLAYER_ALL, mask ) );
                ic.SetLayer( wxLAYER_ALL );

                a2dIterCU cu( ic, cworld );
                m_top->Render( ic, clipparent );

                ic.Reset();
            }
        }
        else if (
                  drawstyle == RenderLAYERED ||
                  drawstyle == RenderWIREFRAME ||
                  drawstyle == RenderWIREFRAME_ZERO_WIDTH
           )
        {
            a2dLayers* layers = GetDrawing()->GetLayerSetup();
            ic.SetPerLayerMode( true );

            if ( 0 ) //contextbased
            {
                a2dLayerIndex::iterator itb = layers->GetOrderSort().begin();
                a2dLayerIndex::iterator ite = layers->GetOrderSort().end();
                // YES here we do the opposite of when rendering, hitting the top layer first
                if ( ic.GetDrawingPart()->GetReverseOrder() )
                {
                    itb = layers->GetReverseOrderSort().begin();
                    ite = layers->GetReverseOrderSort().end();
                }
                for ( a2dLayerIndex::iterator it= itb; it != ite; ++it)
                {
                    a2dLayerInfo* layerobj = *it;

                    //wxLogDebug( wxT("index %d layername %s, layer %d order %d"), j, layerobj->GetName(), layerobj->GetLayer(), layerobj->GetOrder() );


                    //important!
                    //if layer is visible it will be rendered
                    //If an object on a layer is itself invisible it will not be drawn
                    //wxLAYER_ALL is reserved and should never be part of the layersettings
                    if ( m_layerRenderArray[ layerobj->GetLayer() ].DoRenderLayer() &&
                            layerobj->GetVisible()
                        )
                    {
                        //wxLogDebug( "layer %d", layerobj->GetLayer() );
                        m_drawer2D->ResetStyle();
                        ic.SetObjectFilter( new a2dCanvasObjectFilterLayerMaskNoToolNoEdit( layerobj->GetLayer(), mask ) );
                        ic.SetLayer( layerobj->GetLayer() );

                        a2dIterCU cu( ic, cworld );
                        a2dWalker_RenderLayers renderLay( ic );
                        renderLay.SetSkipNotRenderedInDrawing( true );
                        renderLay.Start( m_top );

                        ic.Reset();
                    }
                }
            }
            else
            {
                wxUint16 layer = 0;
                if ( !layers )
                {
                    //wxLogDebug( "layer %d", layerobj->GetLayer() );
                    m_drawer2D->ResetStyle();
                    ic.SetObjectFilter( new a2dCanvasObjectFilterLayerMaskNoToolNoEdit( layer, mask ) );
                    ic.SetLayer( layer );

                    a2dIterCU cu( ic, cworld );
                    m_top->Render( ic, clipparent );

                    ic.Reset();
                }
                else
                {
                    a2dLayerIndex::iterator itb = layers->GetOrderSort().begin();
                    a2dLayerIndex::iterator ite = layers->GetOrderSort().end();
                    if ( ic.GetDrawingPart()->GetReverseOrder() )
                    {
                        itb = layers->GetReverseOrderSort().begin();
                        ite = layers->GetReverseOrderSort().end();
                    }
                    for ( a2dLayerIndex::iterator it= itb; it != ite; ++it)
                    {
                        a2dLayerInfo* layerobj = *it;

                        //wxLogDebug( wxT("index %d layername %s, layer %d order %d"), j, layerobj->GetName(), layerobj->GetLayer(), layerobj->GetOrder() );

                        layer = layerobj->GetLayer();
                        // we set the layer to be used for adding objects at the right Z level.
                        m_drawer2D->SetLayer( layer );

                        //important!
                        //if layer is visible it will be rendered
                        //If an object on a layer is itself invisible it will not be drawn
                        //wxLAYER_ALL is reserved and should never be part of the layersettings
                        if ( m_layerRenderArray[ layer ].DoRenderLayer() &&
                                layerobj->GetVisible()
                            )
                        {
                            m_drawer2D->SetDrawerFill( layers->GetFill( layer ) );
                            m_drawer2D->SetDrawerStroke( layers->GetStroke( layer ) );

                            //wxLogDebug( "layer %d", layerobj->GetLayer() );
                            m_drawer2D->ResetStyle();
                            ic.SetObjectFilter( new a2dCanvasObjectFilterLayerMaskNoToolNoEdit( layer, mask ) );
                            ic.SetLayer( layer );

                            a2dIterCU cu( ic, cworld );
                            m_top->Render( ic, clipparent );

                            ic.Reset();
                        }
                    }
                }
            }
        }
        else if ( drawstyle == RenderWIREFRAME_HighLight )
        {
            ic.SetPerLayerMode( false );
            m_drawer2D->SetDisableDrawing( true );
            ic.SetObjectFilter( new a2dCanvasObjectFilterHighLighted( mask ) );
            ic.SetLayer( wxLAYER_ALL );
            a2dIterCU cu( ic, cworld );
            m_top->Render( ic, clipparent );
            m_drawer2D->SetDisableDrawing( false );

            ic.Reset();
            /*
                        ic.SetPerLayerMode( false );

                        ic.SetObjectFilter( new a2dCanvasObjectFilterLayerMask( wxLAYER_ALL, mask )  );
                        ic.SetLayer( wxLAYER_ALL );
                        a2dIterCU cu( ic, cworld );
                        top->Render( ic, clipparent );
                        ic.Reset();
            */
        }
        else if ( drawstyle == RenderRectangles )
        {
            ic.SetPerLayerMode( false );
            a2dCanvasObjectList::iterator iter = m_top->GetChildObjectList()->begin();
            while( iter != m_top->GetChildObjectList()->end() )
            {
                a2dCanvasObject* obj = ( *iter );
                a2dIterCU cu( ic, cworld );
                a2dBoundingBox bbox = obj->GetBbox();
                if ( obj->CheckMask( mask ) )
                    m_drawer2D->DrawRoundedRectangle( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth(), bbox.GetHeight() , 0 );
                iter++;
            }
        }
        else if ( drawstyle == RenderRectangleTOOL_OBJECTS )
        {
            ic.SetPerLayerMode( false );
            a2dCanvasObjectList::iterator iter = m_top->GetChildObjectList()->begin();
            while( iter != m_top->GetChildObjectList()->end() )
            {
                a2dCanvasObject* obj = ( *iter );
                a2dIterCU cu( ic, cworld );
                a2dBoundingBox bbox = obj->GetBbox();
                if ( obj->CheckMask( mask ) && a2dCanvasObject::PROPID_ToolObject->GetPropertyValue( obj ) )
                    m_drawer2D->DrawRoundedRectangle( bbox.GetMinX(), bbox.GetMinY(), bbox.GetWidth(), bbox.GetHeight() , 0 );
                iter++;
            }
        }
        else if ( drawstyle == RenderTOOL_OBJECTS ||
                  drawstyle == RenderTOOL_OBJECTS_STYLED
                )
        {
            ic.SetPerLayerMode( false );
            m_drawer2D->SetDisableDrawing( true );
            ic.SetObjectFilter( new a2dCanvasObjectFilterToolObjects( a2dCanvasObject::PROPID_ToolObject, mask ) );
            ic.SetLayer( wxLAYER_ALL );

            a2dIterCU cu( ic, cworld );
            m_top->Render( ic, clipparent );
            m_drawer2D->SetDisableDrawing( false );

            ic.Reset();
        }
        else if ( drawstyle == RenderTOOL_DECORATIONS )
        {
            ic.SetPerLayerMode( false );
            m_drawer2D->SetDisableDrawing( true );
            ic.SetObjectFilter( new a2dCanvasObjectFilterToolObjects( a2dCanvasObject::PROPID_ToolDecoration, mask ) );
            ic.SetLayer( wxLAYER_ALL );

            a2dIterCU cu( ic, cworld );
            m_top->Render( ic, clipparent );
            m_drawer2D->SetDisableDrawing( false );

            ic.Reset();
        }
        else if ( drawstyle == RenderWIREFRAME_SELECT ||
                  drawstyle == RenderWIREFRAME_SELECT_INVERT
                )
        {
            ic.SetPerLayerMode( false );
            m_drawer2D->SetDisableDrawing( true );
            ic.SetObjectFilter( new a2dCanvasObjectFilterSelected( mask ) );
            ic.SetLayer( wxLAYER_ALL );
            a2dIterCU cu( ic, cworld );
            m_top->Render( ic, clipparent );
            m_drawer2D->SetDisableDrawing( false );

            ic.Reset();
        }
        else if ( drawstyle == RenderWIREFRAME_SELECT2 ||
                  drawstyle == RenderWIREFRAME_SELECT_INVERT
                )
        {
            ic.SetPerLayerMode( false );
            m_drawer2D->SetDisableDrawing( true );
            ic.SetObjectFilter( new a2dCanvasObjectFilterSelected2( mask ) );
            ic.SetLayer( wxLAYER_ALL );
            a2dIterCU cu( ic, cworld );
            m_top->Render( ic, clipparent );
            m_drawer2D->SetDisableDrawing( false );

            ic.Reset();
        }        
		else
        {
            ic.SetPerLayerMode( false );
            m_drawer2D->SetDisableDrawing( true );
            ic.SetObjectFilter( new a2dCanvasObjectFilterOnlyNoMaskBlind( mask ) );
            ic.SetLayer( wxLAYER_ALL );
            a2dIterCU cu( ic, cworld );
            m_top->Render( ic, clipparent );
            m_drawer2D->SetDisableDrawing( false );

            ic.Reset();
        }

    }
}

void a2dDrawingPart::AddPendingUpdateArea( a2dCanvasObject* obj,  const a2dBoundingBox& box )
{
    int xi = m_drawer2D->WorldToDeviceX( box.GetMinX() );
    int yi = m_drawer2D->WorldToDeviceY( box.GetMinY() );
    int wi = m_drawer2D->WorldToDeviceXRel( box.GetWidth() );
    int hi = m_drawer2D->WorldToDeviceYRel( box.GetHeight() );

    if ( m_drawer2D->GetYaxis() )
    {
        wxRect rect( xi, yi + hi, wi, -hi );
        rect.Inflate( 2 ); //boundingbox is exact, to get rid of 1 pixel problems, always extend a little.
        AddPendingUpdateArea( obj, rect );
    }
    else
    {
        wxRect rect( xi, yi, wi, hi );
        rect.Inflate( 2 ); //boundingbox is exact, to get rid of 1 pixel problems, always extend a little.
        AddPendingUpdateArea( obj, rect );
    }
}

void a2dDrawingPart::AddPendingUpdateArea( const wxRect& recnew )
{
    AddPendingUpdateArea( recnew.x, recnew.y, recnew.width, recnew.height, 0 );
}

void a2dDrawingPart::AddPendingUpdateArea( int x, int y, int w, int h, wxUint8 id )
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

    wxLogDebug("update x=%d, y=%d w=%d h=%d id=%d",  x, y, xmax - x, ymax - y, id );

    m_recur = false;
}

void a2dDrawingPart::AddPendingUpdateArea( a2dCanvasObject* obj, bool refsalso )
{
    a2dIterC ic( this );
    a2dIterCU cu( ic, a2dIDENTITY_MATRIX );
    wxRect recnew = obj->GetAbsoluteArea( ic );
    AddPendingUpdateArea( obj, recnew );

    /*
            //if this object is referenced somewhere else
            //Then there is a chance that it is inside this drawer, so update it.
            if ( refsalso && obj->GetOwnedBy() > 1)
            {
                obj->SetPending( true );
                //at the top level the group its matrix is to be ignored.
                //Since it is normally NOT ignored within a2dCanvasObject, do an inverse first here.
                a2dAffineMatrix cworld= m_top->GetTransformMatrix();
                cworld.Invert();

                a2dIterC ic( this );
                a2dIterCU cu( ic, cworld );

                m_top->AddPending( ic );
                obj->SetPending( false );
            }
    */
}

void a2dDrawingPart::AddPendingUpdateArea( a2dCanvasObject* obj, const wxRect& recnew )
{
    if ( obj )
    {
        if ( obj->GetPendingSelect() )
            AddPendingUpdateArea( recnew.x, recnew.y, recnew.width, recnew.height, 1 );
        if ( obj->GetPending() )
        {
            if ( obj->HasProperty( a2dCanvasObject::PROPID_ToolObject ) ||
                    obj->HasProperty( a2dCanvasObject::PROPID_ToolDecoration )  ||
                    obj->GetEditingRender()
                )
                AddPendingUpdateArea( recnew.x, recnew.y, recnew.width, recnew.height, 1 );
            else
                AddPendingUpdateArea( recnew.x, recnew.y, recnew.width, recnew.height, 0 );
        }
    }
    else
    {
        AddPendingUpdateArea( recnew.x, recnew.y, recnew.width, recnew.height, 1 );
        AddPendingUpdateArea( recnew.x, recnew.y, recnew.width, recnew.height, 0 );
    }
}

bool a2dDrawingPart::AddObjectPendingUpdates( a2dCanViewUpdateFlagsMask how )
{
    if ( !GetDrawing() )
    {
        wxASSERT_MSG( GetDrawing() != 0, wxT( "invalid a2dCanvasDocument in a2dDrawingPart" ) );
        return false;
    }

    if ( GetDrawing()->GetUpdatesPending() )
    {
        //at the top level the group its matrix is to be ignored.
        //Since it is normally NOT ignored within a2dCanvasObject, do an inverse first here.
        a2dAffineMatrix cworld = m_top->GetTransformMatrix();
        cworld.Invert();

        a2dIterC ic( this );
        ic.SetUpdateHint( how );
        a2dIterCU cu( ic, cworld );

        m_top->AddPending( ic );
        return true;
    }
    return false;
}

/** end of updating routines **/

a2dCanvasObject* a2dDrawingPart::SetShowObject( const wxString& name )
{
    ClearCorridorPath();
    
    if ( !GetDrawing() )
    {
        wxASSERT_MSG( GetDrawing() != 0, wxT( "invalid a2dCanvasDocument in a2dDrawingPart" ) );
        return ( a2dCanvasObject* ) NULL;
    }

    if ( m_top )
    {
        m_top->GetRoot()->DisconnectEventAll( this );
        m_cursorStack.clear();
    }

    a2dCanvasObject* newtop = wxStaticCast( GetDrawing()->GetRootObject()->Find( name, wxT( "" ), 0 ), a2dCanvasObject );
    if ( !newtop )
        newtop = GetDrawing()->GetRootObject();

    m_top = newtop;
    if ( m_top )
    {
        a2dHabitat* habitat = m_top->GetRoot()->GetHabitat();

		wxASSERT_MSG( m_top->GetRoot(), "root not set on show object" ); 
		a2dCanvasGlobals->SetHabitat( habitat );

        m_hitmargin = habitat->GetHitMarginDevice();
        m_documentDrawStyle = habitat->GetDrawStyle() | habitat->GetSelectDrawStyle();
        m_selectFill = habitat->GetSelectFill();
        m_selectStroke = habitat->GetSelectStroke();
        m_select2Fill = habitat->GetSelect2Fill();
        m_select2Stroke = habitat->GetSelect2Stroke();
        m_reverse_order = habitat->GetReverseOrder();

        if ( m_top->GetRoot()->GetCanvasCommandProcessor() )
        {
            a2dCanvasGlobals->SetActiveDrawingPart( this, true );
            m_top->GetRoot()->GetCanvasCommandProcessor()->ConnectEvent( wxEVT_BEGINBUSY, this );
            m_top->GetRoot()->GetCanvasCommandProcessor()->ConnectEvent( wxEVT_ENDBUSY, this );
        }
        m_top->GetRoot()->ConnectEvent( wxEVT_MENUSTRINGS, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_DO, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_UNDO, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_REDO, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_CANUNDO, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_CANREDO, this );
        //m_top->GetRoot()->ConnectEvent( wxEVT_UPDATE_VIEWS, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_UPDATE_DRAWING, this );
    }

    Update( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_ALL );

    a2dComEvent changedShow( this, m_top, sig_changedShowObject );
    if ( m_toolcontroller )
    {
        m_toolcontroller->ProcessEvent( changedShow );
        changedShow.Skip();
    }
    ProcessEvent( changedShow );

    return newtop;
}

bool a2dDrawingPart::SetShowObject( a2dCanvasObject* obj )
{
    ClearCorridorPath();

    if ( m_top )
    {
        m_top->GetRoot()->DisconnectEventAll( this );
        m_cursorStack.clear();
    }
    a2dCanvasObject* found = wxDynamicCast( obj, a2dCanvasObject );
    if ( !found )
		m_top = NULL; //GetDrawing()->GetRootObject();
    else
        m_top = found;

    if ( m_top && m_top->GetRoot() )
    {
		wxASSERT_MSG( m_top->GetRoot(), "root not set on show object" ); 

        // use habitat of drawing ( a2dDrawingPart can be used on various drawing with different habitats).
        a2dHabitat* habitat = m_top->GetRoot()->GetHabitat();

		a2dCanvasGlobals->SetHabitat( habitat );

        m_hitmargin = habitat->GetHitMarginDevice();
        m_documentDrawStyle = habitat->GetDrawStyle() | habitat->GetSelectDrawStyle();
        m_selectFill = habitat->GetSelectFill();
        m_selectStroke = habitat->GetSelectStroke();
        m_select2Fill = habitat->GetSelect2Fill();
        m_select2Stroke = habitat->GetSelect2Stroke();
        m_reverse_order = habitat->GetReverseOrder();

        if ( m_top->GetRoot()->GetCanvasCommandProcessor() )
        {
            a2dCanvasGlobals->SetActiveDrawingPart( this, true );
            m_top->GetRoot()->GetCanvasCommandProcessor()->ConnectEvent( wxEVT_BEGINBUSY, this );
            m_top->GetRoot()->GetCanvasCommandProcessor()->ConnectEvent( wxEVT_ENDBUSY, this );
        }
        m_top->GetRoot()->ConnectEvent( wxEVT_MENUSTRINGS, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_DO, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_UNDO, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_REDO, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_CANUNDO, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_CANREDO, this );
        //m_top->GetRoot()->ConnectEvent( wxEVT_UPDATE_VIEWS, this );
        m_top->GetRoot()->ConnectEvent( wxEVT_UPDATE_DRAWING, this );
    }

    Update( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_ALL );

    a2dComEvent changedShow( this, m_top, sig_changedShowObject );
    ProcessEvent( changedShow );

    if ( m_toolcontroller )
    {
        m_toolcontroller->SetStateString( "", 0 );
    }   

    return ( m_top.Get() != 0 );
}

void a2dDrawingPart::SetShowObjectAndRender( a2dCanvasObject* obj )
{
    ClearCorridorPath();

    if ( m_top )
    {
        m_top->GetRoot()->DisconnectEventAll( this );
        m_cursorStack.clear();
    }
    a2dCanvasObject* found = wxDynamicCast( obj, a2dCanvasObject );
    if ( !found )
		m_top = NULL; //GetDrawing()->GetRootObject();
    else
        m_top = found;

    if ( m_top && m_top->GetRoot() )
    {
        a2dHabitat* habitat = m_top->GetRoot()->GetHabitat();

		wxASSERT_MSG( m_top->GetRoot(), "root not set on show object" ); 
		a2dCanvasGlobals->SetHabitat( habitat );

        m_hitmargin = habitat->GetHitMarginDevice();
        m_documentDrawStyle = habitat->GetDrawStyle() | habitat->GetSelectDrawStyle();
        m_selectFill = habitat->GetSelectFill();
        m_selectStroke = habitat->GetSelectStroke();
        m_select2Fill = habitat->GetSelect2Fill();
        m_select2Stroke = habitat->GetSelect2Stroke();
        m_reverse_order = habitat->GetReverseOrder();

        //update from drawing needed.
        m_top->GetRoot()->ConnectEvent( wxEVT_UPDATE_DRAWING, this );
    }

    Update( a2dCANVIEW_UPDATE_OLDNEW | a2dCANVIEW_UPDATE_ALL );
}

void a2dDrawingPart::PushIn( a2dCanvasObject* pushin )
{
    m_pushInStack.push_back( m_top );
    SetShowObject( pushin );
}

a2dCanvasObject* a2dDrawingPart::PopOut()
{
    if ( m_pushInStack.size() )
    {
        SetShowObject( m_pushInStack.back() );
        a2dCanvasObject* ret = m_pushInStack.back();
        m_pushInStack.pop_back();
        return ret;
    }
    return NULL;
}

void a2dDrawingPart::ClearPushInStack()
{
    m_pushInStack.clear();
}

void a2dDrawingPart::SetBackgroundFill( const a2dFill& backgroundfill )
{
    m_backgroundfill = backgroundfill;
    Update( a2dCANVIEW_UPDATE_ALL );
}

void a2dDrawingPart::SetGridStroke( const a2dStroke& gridstroke )
{
    m_gridstroke = gridstroke;
    Update( a2dCANVIEW_UPDATE_ALL );
}

void a2dDrawingPart::SetGridFill( const a2dFill& gridfill )
{
    m_gridfill = gridfill;
    Update( a2dCANVIEW_UPDATE_ALL );
}

void a2dDrawingPart::SetMappingShowAll()
{
    if ( !m_top || !GetDrawing() )
        return;

    a2dBoundingBox untr = m_top->GetBbox();
    a2dAffineMatrix cworld = m_top->GetTransformMatrix();
    cworld.Invert();
    untr.MapBbox( cworld );

    untr.Enlarge( m_drawer2D->DeviceToWorldXRel( m_border ) );

    double worldw = untr.GetWidth();
    double worldh = untr.GetHeight();

    if ( worldw == 0 || worldh == 0 )
    {
        worldw = GetDrawing()->GetInitialSizeX();
        worldh = GetDrawing()->GetInitialSizeY();
        untr = a2dBoundingBox( 0, 0, worldw, worldh );
    }

    double xupp = worldw / m_width;
    double yupp = worldh / m_height;

    if ( yupp == 0 || xupp == 0 ) //no drawing at all
    {
        yupp = 1; xupp = 1; //some value
    }

    if ( yupp > xupp )
    {
        //centrate
        if ( worldw > yupp * m_width )
            untr.Translate( ( worldw - yupp * m_width ) / 2.0, 0 );
    }
    else
    {
        if ( worldh > xupp * m_height )
            untr.Translate( 0, ( worldh - xupp * m_height ) / 2.0 );
    }

    m_drawer2D->SetMappingWidthHeight( untr );
}

void a2dDrawingPart::SetMappingShowAllCanvas()
{
    if ( a2dCanvas* canvas = wxDynamicCast( m_drawingDisplay, a2dCanvas ) )
        canvas->SetMappingShowAll(); 
#if wxART2D_USE_OPENGL && wxUSE_GLCANVAS 
    if ( a2dOglCanvas* canvas = wxDynamicCast( m_drawingDisplay, a2dOglCanvas ) )
        canvas->SetMappingShowAll(); 
#endif //wxART2D_USE_OPENGL
}

void a2dDrawingPart::SetMappingShowSelectedObjects( double minWidth, double minHeight )
{
    if ( !m_top || !GetDrawing() )
        return;

    a2dBoundingBox viewBox;

    forEachIn( a2dCanvasObjectList, m_top->GetChildObjectList() )
    {
        a2dCanvasObject* obj = *iter;
        if ( obj->GetSelected() )
        {
            if ( !viewBox.GetValid() )
                viewBox = obj->GetBbox();
            else
                viewBox.Expand( obj->GetBbox() );
        }
    }

    if ( viewBox.GetValid() )
    {
        viewBox.Enlarge( m_drawer2D->DeviceToWorldXRel( GetZoomOutBorder() ) );

        double w = viewBox.GetWidth();
        double h = viewBox.GetHeight();
        if( w < minWidth || h < minHeight )
        {
            double enlargeX = wxMax( 0, (minWidth - w) / 2 );
            double enlargeY = wxMax( 0, (minHeight - h) / 2 );
            viewBox.EnlargeXY( enlargeX, enlargeY );
        }

        m_drawer2D->SetMappingWidthHeight( viewBox );
    }
}

/*******************************************************************
a2dDrawingPart  (grid and background)
********************************************************************/


void a2dDrawingPart::DrawOrigin()
{
    if ( m_showorigin )
    {
        //origin is in world coordinates at 0,0
        int x = m_drawer2D->WorldToDeviceX( 0.0 );
        int y = m_drawer2D->WorldToDeviceY( 0.0 );
        int w = 10;
        int h = 10;
        m_drawer2D->SetDrawerStroke( *a2dBLACK_STROKE );
        m_drawer2D->SetDrawerFill( *a2dBLACK_FILL );
        m_drawer2D->PushIdentityTransform();
        m_drawer2D->DrawLine( x - w, y, w + x, y );
        m_drawer2D->DrawLine( x, y - h, x, y + h );
        m_drawer2D->PopTransform();
    }
}

void a2dDrawingPart::PaintGrid( int x, int y, int width, int height )
{
    m_drawer2D->SetDrawerStroke( m_gridstroke );
    m_drawer2D->SetDrawerFill( m_gridfill );
    m_drawer2D->SetLayer( wxMAXLAYER );
    // calculate in world coordinates the input size to redraw
    double min_x, max_x, min_y, max_y;

    /*  This if buffer size based.

    double dvx = m_drawer2D->GetUppX() * m_width;
    double dvy = m_drawer2D->GetUppY() * m_height;

    min_x = m_drawer2D->GetVisibleMinX();
    max_x = m_drawer2D->GetVisibleMinX() + dvx;

    if ( m_drawer2D->GetYaxis() )
    {
        min_y = m_drawer2D->DeviceToWorldY( m_height );
        //the next would be wrong since GetVisibleMinY() is the miminum Y on screen
        //the buffer is bigger.
        //min_y = GetVisibleMinY();
        max_y = m_drawer2D->GetVisibleMinY() + dvy;
    }
    else
    {
        min_y = m_drawer2D->GetVisibleMinY();
        max_y = m_drawer2D->GetVisibleMinY() + dvy;
    }
    */

    double gridx = m_gridx;
    double gridy = m_gridy;
    while ( m_drawer2D->DeviceToWorldXRel( m_gridthres ) > gridx )
        gridx *= 2;
    while ( m_drawer2D->DeviceToWorldXRel( m_gridthres ) > gridy )
        gridy *= 2;

    const a2dAffineMatrix worldtodevice = m_drawer2D->GetMappingMatrix();
    min_x = m_drawer2D->DeviceToWorldX( x );
    max_x = m_drawer2D->DeviceToWorldX( x + width );
    if ( !m_drawer2D->GetYaxis() )
    {    
        min_y = m_drawer2D->DeviceToWorldY( y );
        max_y = m_drawer2D->DeviceToWorldY( y + height );
    }
    else
    {    
        max_y = m_drawer2D->DeviceToWorldY( y );
        min_y = m_drawer2D->DeviceToWorldY( y + height );
    }
    
    max_x = ceil( max_x / gridx ) * gridx;
    min_x = floor( min_x / gridx ) * gridx;
    min_y = floor( min_y / gridy ) * gridy;
    max_y = ceil( max_y / gridy ) * gridy;

    m_drawer2D->PushIdentityTransform();
    if ( !m_gridlines )
    {
        double i, j;
        for ( i = min_x; i < max_x; i += gridx )
        {
            for ( j = min_y; j < max_y; j += gridy )
            {
                double xi, yi;
                worldtodevice.TransformPoint( i, j, xi, yi );
                if ( m_gridsize > 1 )
                    m_drawer2D->DrawCircle( xi, yi, m_gridsize / 2 );
                else
                    m_drawer2D->DrawPoint( xi, yi );
            }
        }
    }
    else
    {
        double i, j;
        for ( i = min_x; i < max_x; i += gridx )
        {
            double xi, yi;
            worldtodevice.TransformPoint( i, 0, xi, yi );
            m_drawer2D->DrawLine( xi, 0, xi, m_height );
        }
        for ( j = min_y; j < max_y; j += gridy )
        {
            double xi, yi;
            worldtodevice.TransformPoint( 0, j, xi, yi );
            m_drawer2D->DrawLine( 0, yi, m_width, yi );
        }
    }
    m_drawer2D->PopTransform();

    m_drawer2D->SetDrawerFill( *a2dTRANSPARENT_FILL );
    m_drawer2D->SetDrawerStroke( *a2dTRANSPARENT_STROKE );
}

void a2dDrawingPart::PaintBackground( int x, int y, int width, int height )
{
    m_drawer2D->SetDrawerStroke( *a2dTRANSPARENT_STROKE );
    m_drawer2D->SetDrawerFill( m_backgroundfill );
    m_drawer2D->SetLayer( wxMAXLAYER );
    //clear the buffer using the canvas background colour
    //remark (a transparent pen must mean a fully filled rectangle)

    if ( m_backgroundfill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR || m_backgroundfill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL )
    {
        a2dVertexArray cpoints;
        //TODO it not completely rectangular
        //there is missing a scanline in the fill for horizontal lines.
        //But since the clipping regions should be set, this is a working hack.
        cpoints.AddPoint(  0, -10 );
        cpoints.AddPoint(  0, m_height + 10 );
        cpoints.AddPoint(  m_width, m_height );
        cpoints.AddPoint(  m_width, 0 );

        m_drawer2D->PushIdentityTransform();
        m_drawer2D->DrawPolygon( &cpoints );
        m_drawer2D->PopTransform();
    }
    else
    {
        m_drawer2D->PushIdentityTransform();
        m_drawer2D->DrawRoundedRectangle( x, y, width, height, 0 );
        m_drawer2D->PopTransform();
    }
    m_drawer2D->SetDrawerFill( *a2dTRANSPARENT_FILL );
    m_drawer2D->SetDrawerStroke( *a2dTRANSPARENT_STROKE );

    // the next gives you very good indication of what is updated or not on the canvas.
#if defined(_DEBUG) 
    if ( m_updatesVisible && !(x==0 && y==0 && m_width==width && m_height==height) ) 
    {
        wxUint8 range_max = 210;
        wxUint8 range_min = 190;
        wxUint8 r = double( rand() / double(RAND_MAX + 1) ) * (range_max - range_min) + range_min;
        wxUint8 g = double( rand() / double(RAND_MAX + 1) ) * (range_max - range_min) + range_min;
        wxUint8 b = double( rand() / double(RAND_MAX + 1) ) * (range_max - range_min) + range_min;

        m_drawer2D->PushIdentityTransform();
        m_drawer2D->SetDrawerFill( a2dFill( wxColour( r,g,b ) ) );
        m_drawer2D->DrawRoundedRectangle( x, y, width, height, 0 );
        m_drawer2D->PopTransform();
    }
#endif
}

void a2dDrawingPart::UpdateCrossHair( int x, int y )
{
    if ( !m_crosshair ) //&& m_mouseevents )
        return;

    m_drawer2D->BeginDraw();
    m_drawer2D->PushIdentityTransform();
    m_drawer2D->SetDrawStyle( a2dWIREFRAME_INVERT );

    wxRect rect1( m_crosshairx - m_crosshairLengthX / 2 - 10, m_crosshairy - m_crosshairLengthY / 2 - 10,
                  m_crosshairLengthX + 20, m_crosshairLengthY + 20 );

    //paint cross
    m_drawer2D->SetDrawerStroke( m_crosshairStroke );

    m_drawer2D->DrawLine( m_crosshairx, m_crosshairy - m_crosshairLengthY / 2, m_crosshairx, m_crosshairy + m_crosshairLengthY / 2 );
    m_drawer2D->DrawLine( m_crosshairx - m_crosshairLengthX / 2, m_crosshairy, m_crosshairx + m_crosshairLengthX / 2, m_crosshairy );

    m_crosshairx = x;
    m_crosshairy = y;

    m_drawer2D->DrawLine( m_crosshairx, m_crosshairy - m_crosshairLengthY / 2, m_crosshairx, m_crosshairy + m_crosshairLengthY / 2 );
    m_drawer2D->DrawLine( m_crosshairx - m_crosshairLengthX / 2, m_crosshairy, m_crosshairx + m_crosshairLengthX / 2, m_crosshairy );

    m_drawer2D->BlitBuffer( rect1 );

    wxRect rect( m_crosshairx - m_crosshairLengthX / 2 - 10, m_crosshairy - m_crosshairLengthY / 2 - 10,
                 m_crosshairLengthX + 20, m_crosshairLengthY + 20 );
    m_drawer2D->BlitBuffer( rect );

    m_drawer2D->SetDrawerStroke( *a2dTRANSPARENT_STROKE );

    m_drawer2D->PopTransform();
    m_drawer2D->EndDraw();
    m_drawer2D->SetDrawStyle( a2dFILLED );
}

void a2dDrawingPart::Scroll( int dxy, bool yscroll, bool  total )
{
    //REDRAW
    int bw = m_width;
    int bh = m_height;
}

void a2dDrawingPart::OnDrop(wxCoord x, wxCoord y, a2dDrawing* drawing )
{
    double xWorldLocal, yWorldLocal;
    MouseToToolWorld( x, y, xWorldLocal, yWorldLocal );
    a2dCanvasObjectList* objects = drawing->GetRootObject()->GetChildObjectList();
    forEachIn( a2dCanvasObjectList, objects )
    {
        a2dCanvasObject* obj = *iter;
        obj->Translate( xWorldLocal, yWorldLocal );
        m_top->Append( obj );
    }

}

void a2dDrawingPart::SetMappingWidthHeight(double vx1, double vy1, double width, double height )
{
    double xupp = width / m_width;
    double yupp = height / m_height;

    if (yupp == 0 || xupp == 0) //no drawing at all
    {
        yupp = 1; xupp = 1; //some value
    }

    if (yupp > xupp)
    {
        SetMappingUpp(vx1, vy1, yupp, yupp);
    }
    else
    {
        SetMappingUpp(vx1, vy1, xupp, xupp);
    }
}

void a2dDrawingPart::SetMappingUpp(double vx1, double vy1, double xpp, double ypp, double x, double y )
{
    //use the centre as fixed point for zooming
    if (x == -1) x = vx1 + GetDrawer2D()->GetMapWidth() / 2.0 * xpp;
    if (y == -1) y = vy1 + GetDrawer2D()->GetMapHeight() / 2.0 * ypp;

    // The mapping width and height here, can be different from the buffer m_width and m_height. 
    // Normally the buffer should at least be width*height, else clipping will take place, especially when used within a a2dCanvas context.
    if (m_uppXyMax && (m_uppXyMax > xpp || m_uppXyMax > ypp))
    {
            //stick to what it was, unless the maximum zoom in has not yet been reached
            if (m_uppXyMax != GetDrawer2D()->GetUppX() && m_uppXyMax != GetDrawer2D()->GetUppY())
            {
                //zoom in to the maximum allowed
                double vxMax = vx1;
                if (m_uppXyMax > xpp)
                    vxMax -= ((x - vx1) / xpp) * (m_uppXyMax - xpp);
           
                double vyMax = vy1;
                if (m_uppXyMax > ypp)
                    vyMax -= ((y - vy1) / ypp) * (m_uppXyMax - ypp);
            
                GetDrawer2D()->SetMappingUpp(vxMax, vyMax, m_uppXyMax, m_uppXyMax);
            }
    }
    else
        GetDrawer2D()->SetMappingUpp(vx1, vy1, xpp, ypp);
}

IMPLEMENT_DYNAMIC_CLASS( a2dDrawingPartTiled, a2dDrawingPart )

BEGIN_EVENT_TABLE( a2dDrawingPartTiled, a2dDrawingPart )
    EVT_IDLE( a2dDrawingPartTiled::OnIdle )
    EVT_UPDATE_DRAWING( a2dDrawingPartTiled::OnUpdate )
END_EVENT_TABLE()

a2dDrawingPartTiled::a2dDrawingPartTiled( const wxSize& size ):
    a2dDrawingPart( size )
{
    m_tileVec.push_back( new a2dTiles( ( size.GetWidth() >> a2d_TILESHIFT ) + 1 , ( size.GetHeight() >> a2d_TILESHIFT ) + 1, 0 ) );
    m_tileVec.push_back( new a2dTiles( ( size.GetWidth() >> a2d_TILESHIFT ) + 1 , ( size.GetHeight() >> a2d_TILESHIFT ) + 1, 1 ) );

    m_width = size.GetWidth();
    m_height = size.GetHeight();
}

a2dDrawingPartTiled::a2dDrawingPartTiled( int width, int height ):
    a2dDrawingPart( width, height )
{
    m_tileVec.push_back( new a2dTiles(  ( width >> a2d_TILESHIFT ) + 1 , ( height >> a2d_TILESHIFT ) + 1, 0 ) );
    m_tileVec.push_back( new a2dTiles(  ( width >> a2d_TILESHIFT ) + 1 , ( height >> a2d_TILESHIFT ) + 1, 1 ) );
}

a2dDrawingPartTiled::a2dDrawingPartTiled( int width, int height, a2dDrawer2D* drawer ):
    a2dDrawingPart( width, height, drawer )
{
    m_tileVec.push_back( new a2dTiles(  ( width >> a2d_TILESHIFT ) + 1 , ( height >> a2d_TILESHIFT ) + 1, 0 ) );
    m_tileVec.push_back( new a2dTiles(  ( width >> a2d_TILESHIFT ) + 1 , ( height >> a2d_TILESHIFT ) + 1, 1 ) );
}

a2dDrawingPartTiled::a2dDrawingPartTiled( a2dDrawer2D* drawer ):
    a2dDrawingPart( drawer )
{
    m_tileVec.push_back( new a2dTiles( ( drawer->GetWidth() >> a2d_TILESHIFT ) + 1 , ( drawer->GetHeight() >> a2d_TILESHIFT ) + 1, 0 ) );
    m_tileVec.push_back( new a2dTiles( ( drawer->GetWidth() >> a2d_TILESHIFT ) + 1 , ( drawer->GetHeight() >> a2d_TILESHIFT ) + 1, 1 ) );
}

a2dDrawingPartTiled::~a2dDrawingPartTiled()
{
    DeleteAllPendingAreas();
}

a2dDrawingPartTiled::a2dDrawingPartTiled( const a2dDrawingPartTiled& other )
: a2dDrawingPart( other )
{
    m_tileVec.push_back( new a2dTiles( ( other.m_width >> a2d_TILESHIFT ) + 1 , ( other.m_height >> a2d_TILESHIFT ) + 1, 0 ) );
    m_tileVec.push_back( new a2dTiles( ( other.m_width >> a2d_TILESHIFT ) + 1 , ( other.m_height >> a2d_TILESHIFT ) + 1, 1 ) );
}

void a2dDrawingPartTiled::SetBufferSize( int w, int h )
{
    m_width = w;
    m_height = h;
    m_tileVec.SetSize( ( w >> a2d_TILESHIFT ) + 1, ( h >> a2d_TILESHIFT ) + 1 );
    if ( m_drawer2D )
        m_drawer2D->SetBufferSize( w, h );
}

void a2dDrawingPartTiled::SetDrawer2D( a2dDrawer2D* drawer2d, bool noDelete )
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
        // klion: we change m_width and m_height, so we must change m_tiles
        m_tileVec.SetSize( ( m_width >> a2d_TILESHIFT ) + 1, ( m_height >> a2d_TILESHIFT ) + 1 );
    }
}

void a2dDrawingPartTiled::OnIdle( wxIdleEvent& event )
{
    //wxLogDebug( "void a2dDrawingPartTiled::OnIdle(wxIdleEvent &event)" );

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
    if ( m_tileVec.HasFilledTiles() || m_updateareas.GetCount() )
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

void a2dDrawingPartTiled::DeleteAllPendingAreas()
{
    m_tileVec.Clear();
    wxASSERT_MSG( m_recur == false, wxT( "recursive calls in updating not allowed" ) );
    m_recur = true;

    //all pending updates become redundant, make the list empty
    a2dUpdateList::compatibility_iterator nodeb = m_updateareas.GetFirst();
    while ( nodeb )
    {
        a2dUpdateArea* uobj = nodeb->GetData();
        delete uobj;
        m_updateareas.DeleteNode( nodeb );
        nodeb = m_updateareas.GetFirst();
    }

    m_recur = false;
}

void a2dDrawingPartTiled::RedrawPendingUpdateAreas( bool noblit )
{
    m_tileVec.GenerateUpdateRectangles( &m_updateareas );
    m_tileVec.Clear();

    if ( IsFrozen() )
        return;

    if ( m_recur ) return;
    wxASSERT_MSG( m_recur == false, wxT( "recursive calls in updating not allowed" ) );

    m_recur = true;

    a2dUpdateList::compatibility_iterator nodeb = m_updateareas.GetFirst();
    while ( nodeb )
    {
        a2dUpdateArea* uobj = nodeb->GetData();

        if ( !uobj->m_update_done )
        {
            //update a little more then is strictly needed,
            //to get rid of the 1 bit bugs
            //SEE wxRect a2dCanvasObject::GetAbsoluteArea( const a2dAffineMatrix& cworld )

            int x, y, width, height;
            uobj->Inflate( 2 );
            x = uobj->x;
            y = uobj->y;
            width = uobj->width;
            height = uobj->height;


            UpdateArea( x, y, width, height, uobj->m_id );

            //update is done Blitting needs to be done
            uobj->m_update_done = true;

            if ( noblit )
            {
                //we can delete this rect
                delete uobj;
                a2dUpdateList::compatibility_iterator nodeh = nodeb;
                nodeb = nodeb->GetNext();
                m_updateareas.DeleteNode( nodeh );
            }
            else
                nodeb = nodeb->GetNext();
        }
        else
            nodeb = nodeb->GetNext();
    }

    m_recur = false;
}

void a2dDrawingPartTiled::AddPendingUpdateArea( int x, int y, int w, int h, wxUint8 id )
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

    m_tileVec[id]->FillTiles( x, y, xmax - x, ymax - y, true );

    //wxLogDebug("update x=%d, y=%d w=%d h=%d id=%d",  x, y, xmax - x, ymax - y, id );

    m_recur = false;
}

void a2dDrawingPartTiled::OnUpdate( a2dDrawingEvent& event )
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
        DeleteAllPendingAreas();
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

    if ( how & a2dCANVIEW_UPDATE_BLIT )
    {
        if ( GetDisplayWindow() && GetDisplayWindow()->IsShown() )
            BlitPendingUpdateAreas();
        else
            // if there is something in the buffer to blit to the windows which is not visible,
            // they can be skipped.
            DeleteAllPendingAreas();
    }
}

bool a2dDrawingPartTiled::BlitPendingUpdateAreas()
{
    //if there are blits we need a refresh for tools and/or controls
    bool refresh = false;

    if ( !GetDrawing() || IsFrozen() ||  !m_updateareas.GetFirst() )
        return refresh;

    wxASSERT_MSG( m_recur == false, wxT( "recursive calls in updating not allowed" ) );
    m_recur = true;

    //first find out if there will be blit, to prevent expensive call if there are not
    //areas to blit
    a2dUpdateList::compatibility_iterator node = m_updateareas.GetFirst();
    while ( node )
    {
        a2dUpdateArea* rect = node->GetData();

        if ( rect->m_update_done )
            refresh = true;
        node = node->GetNext();
    }

    if ( refresh )
    {
        m_drawer2D->BeginDraw();

        a2dUpdateList::compatibility_iterator node = m_updateareas.GetFirst();
        while ( node )
        {
            a2dUpdateArea* rect = node->GetData();

            if ( rect->m_update_done )
            {
                m_drawer2D->BlitBuffer( *rect, wxPoint( m_drawer2D->GetMapX(), m_drawer2D->GetMapY() ) );

                //place for code which needs to refresh objects which are drawn on top of the buffer bitmap
                // for example wxWindows derived controls.
                if ( m_drawingDisplay && m_drawingDisplay->GetChildren().GetCount() )
                {
                    m_drawingDisplay->Refresh( false, rect );
                }

                //we can delete this rect
                delete rect;
                a2dUpdateList::compatibility_iterator nodeh = node;
                node = node->GetNext();
                m_updateareas.DeleteNode( nodeh );
            }
            else
                node = node->GetNext();
        }


        if ( m_drawingDisplay )
        {
            a2dComEvent swapBuffer( this, sig_swapBuffer );
            m_drawingDisplay->GetEventHandler()->ProcessEvent( swapBuffer );
        }

        UpdateCrossHair( m_crosshairx, m_crosshairy );

        m_drawer2D->EndDraw();
    }

    m_recur = false;

    return refresh;
}

void a2dDrawingPartTiled::Scroll( int dxy, bool yscroll, bool  total )
{
    bool gradient = ( m_backgroundfill.GetType() == a2dFILL_GRADIENT_FILL_LINEAR || m_backgroundfill.GetType() == a2dFILL_GRADIENT_FILL_RADIAL );
    int bw = m_width;
    int bh = m_height;
    if ( yscroll )
    {
        if ( total || gradient )
        {
            AddPendingUpdateArea( 0, 0, bw, bh );
        }
        else
        {
            if ( dxy > 0 && dxy < bh )
            {
                m_drawer2D->ShiftBuffer( dxy, yscroll );
                AddPendingUpdateArea( 0, 0, bw, dxy );
            }
            else  if ( dxy < 0 && dxy > -bh )
            {
                m_drawer2D->ShiftBuffer( dxy, yscroll );
                AddPendingUpdateArea( 0, bh + dxy, bw, -dxy );
            }
            else
                AddPendingUpdateArea( 0, 0, bw, bh );
        }
    }
    else
    {
        if ( total || gradient )
        {
            AddPendingUpdateArea( 0, 0, bw, bh );
        }
        else
        {
            if ( dxy > 0 && dxy < bw )
            {
                m_drawer2D->ShiftBuffer( dxy, yscroll );
                AddPendingUpdateArea( 0, 0, dxy, bh );
            }
            else if ( dxy < 0 && dxy > -bw )
            {
                m_drawer2D->ShiftBuffer( dxy, yscroll );
                AddPendingUpdateArea( bw + dxy, 0, -dxy, bh );
            }
            else
                AddPendingUpdateArea( 0, 0, bw, bh );
        }
    }

    //now make sure the buffer is fully updated
    RedrawPendingUpdateAreas();
}

#if wxUSE_PRINTING_ARCHITECTURE


//----------------------------------------------------------------------------
// a2dPrintFactory
//----------------------------------------------------------------------------

wxPrinterBase *a2dPrintFactory::CreatePrinter( wxPrintDialogData *data )
{
#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    return new wxWindowsPrinter( data );
#elif defined(__WXMAC__)
    return new wxMacPrinter( data );
#elif defined(__WXPM__)
    return new wxOS2Printer( data );
#else
    return new wxPostScriptPrinter( data );
#endif
}

wxPrintPreviewBase *a2dPrintFactory::CreatePrintPreview( wxPrintout *preview,
    wxPrintout *printout, wxPrintDialogData *data )
{
#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    return new a2dWindowsPrintPreview( preview, printout, data );
#elif defined(__WXMAC__)
    return new wxMacPrintPreview( preview, printout, data );
#elif defined(__WXPM__)
    return new wxOS2PrintPreview( preview, printout, data );
#else
    return new wxPostScriptPrintPreview( preview, printout, data );
#endif
}

wxPrintPreviewBase *a2dPrintFactory::CreatePrintPreview( wxPrintout *preview,
    wxPrintout *printout, wxPrintData *data )
{
#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    return new a2dWindowsPrintPreview( preview, printout, data );
#elif defined(__WXMAC__)
    return new wxMacPrintPreview( preview, printout, data );
#elif defined(__WXPM__)
    return new wxOS2PrintPreview( preview, printout, data );
#else
    return new wxPostScriptPrintPreview( preview, printout, data );
#endif
}

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)

/*
* Print preview
*/

IMPLEMENT_CLASS( a2dWindowsPrintPreview, wxWindowsPrintPreview )

a2dWindowsPrintPreview::a2dWindowsPrintPreview(wxPrintout *printout,
                                             wxPrintout *printoutForPrinting,
                                             wxPrintDialogData *data)
                     : wxWindowsPrintPreview(printout, printoutForPrinting, data)
{
    DetermineScaling();
}

a2dWindowsPrintPreview::a2dWindowsPrintPreview(wxPrintout *printout,
                                             wxPrintout *printoutForPrinting,
                                             wxPrintData *data)
                     : wxWindowsPrintPreview( printout, printoutForPrinting, data)
{
    DetermineScaling();
}

a2dWindowsPrintPreview::~a2dWindowsPrintPreview()
{
}

bool a2dWindowsPrintPreview::RenderPageIntoBitmap(wxBitmap& bmp, int pageNum)
{    
    wxMemoryDC memoryDC;
    memoryDC.SelectObject(bmp);
    memoryDC.Clear();

    return RenderPageIntoDC(memoryDC, pageNum);
}

#endif

#endif // wxUSE_PRINTING_ARCHITECTURE



#if wxUSE_DRAG_AND_DROP

// ----------------------------------------------------------------------------
// A wxDataObject specialisation for the application-specific data
// ----------------------------------------------------------------------------

const wxChar *a2dCanvasObjectFormatId = wxT("a2dCanvasObject");


// ----------------------------------------------------------------------------
// A a2dDnDCameleonData specialisation for the application-specific data
// ----------------------------------------------------------------------------

a2dDnDCameleonData::a2dDnDCameleonData( a2dCanvasObject* canvasobject, double dragStartX, double dragStartY, a2dDrawingPart* drawingPart )
: a2dDnDCanvasObjectDataCVG( drawingPart )
{
}

a2dDnDCameleonData::~a2dDnDCameleonData() 
{
}

size_t a2dDnDCameleonData::GetDataSize(const wxDataFormat& format) const
{
    if ( format == m_formatShape )
    {
        a2dCanvasObject::SetIgnoreAllSetpending( true );

        // Set LC_NUMERIC facet to "C" locale for this I/O operation
        // !!!! It would be better to set the locale of the stream itself,
        // but this does not work for two reasons:
        // 1. wxWindows streams don't support this
        // 2. CVG-IO uses e.g. wxString::ToDouble, which is independent of the stream
      //a2dCNumericLocaleSetter locSetter;

        m_mem.str("");
        a2dIOHandlerCVGOut CVGwriter;
        a2dCanvasObjectList* objects = m_drawing->GetRootObject()->GetChildObjectList();
        forEachIn( a2dCanvasObjectList, objects )
        {
            a2dCanvasObject* obj = *iter;
            obj->Translate( -m_dragStartX, -m_dragStartY);

            a2dCameleonSymbolicRef* ref = wxDynamicCast( obj, a2dCameleonSymbolicRef );
            if ( ref )
            {
                //a2dCameleonInst* refinst = new a2dCameleonInst( 2, 5, ref->GetCameleon()->GetAppearance<a2dSymbol>() ); 
                a2dCameleonInst* refinst = new a2dCameleonInst( 2, 5, ref->GetCameleon()->GetAppearance<a2dDiagram>() ); 
                *iter = refinst;
            }
        }
        a2dCanvasObject::SetIgnoreAllSetpending( false );

        CVGwriter.SaveStartAt( m_mem, m_drawing, m_drawing->GetRootObject() );

/* to debug contents
        wxFFileOutputStream file("c:/data/soft/out.txt");
        wxStdOutputStream out(file);

        out << m_mem.str() << std::endl;
*/

        m_mem.seekp(0);

        return m_mem.str().length()+10;
    }
    else 
    {
        return a2dDnDCanvasObjectDataCVG::GetDataSize( format ); 
    }
}

bool a2dDnDCameleonData::SetData(const wxDataFormat& format, size_t len, const void *buf )
{
    if ( format == m_formatShape )
    {
        
        wxCharBuffer cvgString = wxCharBuffer( (const char*) buf );

        // Set LC_NUMERIC facet to "C" locale for this I/O operation
        // !!!! It would be better to set the locale of the stream itself,
        // but this does not work for two reasons:
        // 1. wxWindows streams don't support this
        // 2. CVG-IO uses e.g. wxString::ToDouble, which is independent of the stream
      //a2dCNumericLocaleSetter locSetter;

    #if wxART2D_USE_CVGIO 
        #if wxUSE_STD_IOSTREAM
            #if wxUSE_UNICODE
                a2dDocumentStringInputStream stream( cvgString.data(), wxSTD ios_base::in );
            #else
                a2dDocumentStringInputStream stream( cvgString, wxSTD ios_base::in );
            #endif // wxUSE_UNICODE
        #else
            a2dDocumentStringInputStream stream( cvgStringtoExecute );
        #endif
            a2dIOHandlerCVGIn CvgString;

            CvgString.Load( stream, m_drawing, m_drawing->GetRootObject() );

            // the shape has changed
            m_hasBitmap = false;

        #if wxUSE_METAFILE
            m_hasMetaFile = false;
        #endif // wxUSE_METAFILE
            return true;
    #else
        return false;
    #endif //wxART2D_USE_CVGIO 
    }
    else 
    {
        a2dDnDCanvasObjectDataCVG::SetData( format, len, buf );
    }
    return false;
}

#endif // wxUSE_DRAG_AND_DROP

//----------------------------------------------------------------------------
// a2dDrawingPrintOut
//----------------------------------------------------------------------------

#if wxUSE_PRINTING_ARCHITECTURE

bool a2dDrawingPrintOut::m_printAsBitmap = true;

/* print out for drawer */
a2dDrawingPrintOut::a2dDrawingPrintOut( const wxPageSetupDialogData& pageSetupData, a2dDrawingPart* drawingPart, const wxString& title, const wxString& filename, a2dPrintWhat typeOfPrint, bool drawframe, double scalelimit, bool fitToPage ) : 
    m_pageSetupData( pageSetupData ),
    wxPrintout( title )
{
    m_typeOfPrint = typeOfPrint;
    m_drawingPart = drawingPart;
    m_title = title;
    m_filename = filename;
    m_scalelimit = scalelimit;
    m_drawframe = drawframe;
    m_fitToPage = fitToPage;
}

a2dDrawingPrintOut::~a2dDrawingPrintOut( void )
{
}

bool a2dDrawingPrintOut::OnPrintPage( int )
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
    a2dSmrtPtr<a2dDrawingPart> drawPart = new a2dDrawingPart( *m_drawingPart );

    //drawView is a a2dView and therefore gets events, but this is not wanted here.
    drawPart->SetEvtHandlerEnabled( false );
    drawPart->SetDrawer2D( dcdrawer );
    drawPart->SetShowOrigin( false );
    drawPart->SetGrid( false );

    // take some important settings from the view where this a2dDrawingPrintOut was created:
    // see a2dDrawingPart::OnCreatePrintout()
    drawPart->SetShowObjectAndRender( m_drawingPart->GetShowObject() );
    drawPart->SetDocumentDrawStyle( m_drawingPart->GetDocumentDrawStyle() );
    drawPart->SetAvailable();
	a2dDrawer2D* drawcont =  m_drawingPart->GetDrawer2D();
    dcdrawer->SetYaxis( drawcont->GetYaxis() );
	
    if ( memprint || (!memprint && !m_printAsBitmap) )
    {
        //is wxPrinterDc, and we draw on it;

        if ( m_typeOfPrint == a2dPRINT_PrintDrawingPart || m_typeOfPrint == a2dPRINT_PreviewDrawingPart )
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
        if ( m_typeOfPrint == a2dPRINT_PrintDrawingPart || m_typeOfPrint == a2dPRINT_PreviewDrawingPart )
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

bool a2dDrawingPrintOut::HasPage( int pageNum )
{
    return ( pageNum == 1 );
}

bool a2dDrawingPrintOut::OnBeginDocument( int startPage, int endPage )
{
    if ( !wxPrintout::OnBeginDocument( startPage, endPage ) )
        return false;

    return true;
}

void a2dDrawingPrintOut::GetPageInfo( int* minPage, int* maxPage, int* selPageFrom, int* selPageTo )
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

#endif //wxUSE_PRINTING_ARCHITECTURE
