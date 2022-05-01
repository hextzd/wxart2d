//////////////////////////////////////////////////////////////////////////////
// Name:        canedit.cpp
// Author:      Klaas Holwerda
// Created:     XX/XX/XX
// Copyright:   2000 (c) Klaas Holwerda
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

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
#include <wx/fontdlg.h>

#include "tedit.h"
#include "docv.h"

#include "wx/canvas/booloper.h"

#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#include "wx/aggdrawer/graphicagg.h"
#endif //wxART2D_USE_AGGDRAWER

#if wxART2D_USE_GDIPLUSDRAWER
#include "wx/gdiplusdrawer/gdiplusdrawer.h"
#endif //wxART2D_USE_GDIPLUSDRAWER

//int __cdecl _CrtSetDbgFlag( int a ) {
//    return 0;
//}
//
//void *_imp___CrtSetDbgFlag = _CrtSetDbgFlag;

/********************************************************************
* a2dCanvasTetoEditorFrame
*********************************************************************/
const long SCRIPT_lua_script = wxNewId();

IMPLEMENT_DYNAMIC_CLASS( a2dCanvasTetoEditorFrame, whichEditorFrame )

BEGIN_EVENT_TABLE( a2dCanvasTetoEditorFrame, whichEditorFrame )
    EVT_MENU_RANGE( DrawerOption_First, DrawerOption_Last, a2dCanvasTetoEditorFrame::OnDrawer )
    EVT_MENU( MasterToolOption, a2dCanvasTetoEditorFrame::OnMasterToolOption )
    EVT_MENU( wxID_EXIT, a2dCanvasTetoEditorFrame::Quit )
    EVT_MENU( wxID_ABOUT, a2dCanvasTetoEditorFrame::OnAbout )
    EVT_POST_LOAD_DOCUMENT( a2dCanvasTetoEditorFrame::OnPostLoadDocument )
    EVT_THEME_EVENT( a2dCanvasTetoEditorFrame::OnTheme )
    EVT_INIT_EVENT( a2dCanvasTetoEditorFrame::OnInit )
END_EVENT_TABLE()

//! canvas window to display the view upon to be defined by user
#define DOC m_view->GetCanvasDocument()

a2dCanvasTetoEditorFrame::a2dCanvasTetoEditorFrame( bool isParent, 
        wxFrame* parent, const wxPoint& pos, const wxSize& size, long style )
    : whichEditorFrame( isParent, parent, pos, size, style )
{
    Init();
}

bool a2dCanvasTetoEditorFrame::Create( bool isParent, 
                                       wxFrame* parent, const wxPoint& pos , const wxSize& size, long style )
{
    bool res = whichEditorFrame::Create( isParent, parent, pos, size, style );

    a2dCanvas* canvas = new a2dCanvas( this, -1, pos,  size, a2dCanvasGlobals->m_initialWorldCanvasStyle );
    m_drawingPart = canvas->GetDrawingPart();

    return res;
}

void a2dCanvasTetoEditorFrame::OnTheme( a2dEditorFrameEvent& themeEvent )
{
    Theme();
    //CreateThemeXRC();

    wxMenu* drawEng = new wxMenu;
    drawEng->Append( Switch_Drawer_aggRgba, _T( "Switch Drawer &Agg AntiAlias Rgba" ) );
    drawEng->Append( Switch_Drawer_agg, _T( "Switch Drawer &Agg AntiAlias" ) );
    drawEng->Append( Switch_Drawer_agggc, _T( "Switch Drawer Graphics &Context &Agg Gc" ) );

    drawEng->Append( Switch_Drawer_dc, _T( "Switch Drawer &wxDC" ) );
    drawEng->Append( Switch_Drawer_gdiplus, _T( "Switch Drawer &GdiPlus" ) );
    drawEng->Append( Switch_Drawer_gdigc, _T( "Switch Drawer Graphics &Context GDI" ) );
    drawEng->Append( Switch_Drawer_dcgc, _T( "Switch Drawer Graphics Context &wxDC" ) );

    wxMenu* menumasterTool = new wxMenu;
    menumasterTool->Append( MasterToolOption, _( "Edit Mode zoom or select first" ), _( "set the main tool in mode zoom first or select first" ), true );

    wxMenuBar* bar =  GetMenuBar();
    bar->Append( drawEng, _T( "&Drawer Type" ) );
    bar->Append( menumasterTool, _T( "&Choose MasterTool" ) );


    //wxMenu *oper_and_menu = new wxMenu;
    //oper_and_menu->Append( ID_XXX, _T("menu "), _T("menu  help") );
    //m_menuBar->Append( oper_and_menu, _T("Operations") );

    //m_contr->SetCrossHair( true );

    //defines the whole area in world coordinates that can be seen by scrolling
    m_drawingPart->GetCanvas()->SetScrollMaximum( -1000, -1000, 2000, 2000 );
    //defines the number of world units scrolled when line up or down events in X
    m_drawingPart->GetCanvas()->SetScrollStepX( 100 );
    //defines the number of world units scrolled when line up or down events in Y
    m_drawingPart->GetCanvas()->SetScrollStepY( 100 );

    if ( m_drawingPart )
    {
        m_drawingPart->SetGridStroke( a2dStroke( wxColour( 239, 5, 64 ), 0, a2dSTROKE_DOT ) );
        //to show grid as lines
        //m_canvas1->SetGridLines(true);

        m_drawingPart->SetGridSize( 2 );
        m_drawingPart->SetGridX( 200 );
        m_drawingPart->SetGridY( 100 );

        //show it?
        m_drawingPart->SetGrid( false );
        m_drawingPart->SetGridAtFront( true );
    }

    a2dCanvasGlobals->GetHabitat()->SetRouteOneLine( true );

    m_contr->GetFirstTool()->SetShowAnotation( false );
    //m_contr->SetOpacityFactorEditcopy( 125 );
    //m_contr->SetUseOpaqueEditcopy( true );

    m_contr->SetZoomFirst( false );

    m_editMode = true;

    if ( m_editMode )
    {
        a2dMasterDrawSelectFirst* tool = new a2dMasterDrawSelectFirst( m_contr );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
        bar->Check( MasterToolOption, m_editMode );
    }
    else
    {
        a2dMasterDrawZoomFirst* tool = new a2dMasterDrawZoomFirst( m_contr );
        //tool->SetFastTools( true );
        m_contr->StopAllTools();
        m_contr->SetTopTool( tool );
        bar->Check( MasterToolOption, m_editMode );
    }

    a2dLuaEditorFrame::OnTheme( themeEvent );

    themeEvent.Skip( false );
}

void a2dCanvasTetoEditorFrame::OnInit( a2dEditorFrameEvent& initEvent )
{
    initEvent.Skip();
}

a2dCanvasTetoEditorFrame::~a2dCanvasTetoEditorFrame()
{
}

void a2dCanvasTetoEditorFrame::Quit( wxCommandEvent& event )
{
    Close( true );
    //event.Skip();
}

void a2dCanvasTetoEditorFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    ( void )wxMessageBox( wxT( "a2dCanvasTetoEditorFrame\n Klaas Holwerda 2002" ),
                          wxT( "About a2dCanvasTetoEditorFrame" ), wxICON_INFORMATION | wxOK );
}

void a2dCanvasTetoEditorFrame::OnPostLoadDocument( a2dDocumentEvent& event )
{
    if ( m_view->GetDocument() != event.GetEventObject() )
        return;

    m_drawingPart->GetCanvas()->SetMappingShowAll();
}

void a2dCanvasTetoEditorFrame::OnDrawer( wxCommandEvent& event )
{
    int width = m_drawingPart->GetDrawer2D()->GetWidth();
    int height = m_drawingPart->GetDrawer2D()->GetHeight();

    switch ( event.GetId() )
    {
#if wxART2D_USE_AGGDRAWER
        case Switch_Drawer_aggRgba:
        {
            m_drawingPart->SetDrawer2D( new a2dAggDrawerRgba( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
        case Switch_Drawer_agg:
        {
            m_drawingPart->SetDrawer2D( new a2dAggDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#if wxART2D_USE_GRAPHICS_CONTEXT
        case Switch_Drawer_agggc:
        {
            m_drawingPart->SetDrawer2D( new a2dGcAggDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#endif // wxART2D_USE_GRAPHICS_CONTEXT

#endif //wxART2D_USE_AGGDRAWER
        case Switch_Drawer_dc:
            m_drawingPart->SetDrawer2D( new a2dMemDcDrawer( width, height ) );
            m_contr->SetUseOpaqueEditcopy( false );
            break;

#if wxART2D_USE_GDIPLUSDRAWER
        case Switch_Drawer_gdiplus:
        {
            m_drawingPart->SetDrawer2D( new a2dGDIPlusDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#endif //wxART2D_USE_GDIPLUSDRAWER

#if wxART2D_USE_GRAPHICS_CONTEXT
        case Switch_Drawer_dcgc:
        {
            m_drawingPart->SetDrawer2D( new a2dGcDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
        case Switch_Drawer_gdigc:
        {
            m_drawingPart->SetDrawer2D( new a2dNativeGcDrawer( width, height ) );
            m_contr->SetOpacityFactorEditcopy( 125 );
            m_contr->SetUseOpaqueEditcopy( true );
        }
        break;
#endif // wxART2D_USE_GRAPHICS_CONTEXT
        default:
            // skip Refresh()
            return;
    }

    m_drawingPart->GetDrawer2D()->SetYaxis( true );
    m_drawingPart->GetCanvas()->SetMappingShowAll();
}

void a2dCanvasTetoEditorFrame::OnMasterToolOption( wxCommandEvent& event )
{
    {
        m_editMode = !m_editMode;
        if ( m_editMode )
        {
            a2dMasterDrawSelectFirst* tool = new a2dMasterDrawSelectFirst( m_contr );
            //tool->SetFastTools( true );
            m_contr->StopAllTools();
            m_contr->SetTopTool( tool );
            m_menuBar->Check( MasterToolOption, m_editMode );
        }
        else
        {
            a2dMasterDrawZoomFirst* tool = new a2dMasterDrawZoomFirst( m_contr );
            //tool->SetFastTools( true );
            m_contr->StopAllTools();
            m_contr->SetTopTool( tool );
            m_menuBar->Check( MasterToolOption, m_editMode );
        }
    }
}




