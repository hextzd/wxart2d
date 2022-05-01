/*! \file editor/src/tooldlg.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: tooldlg.cpp,v 1.37 2009/10/05 20:03:12 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/tooldlg.h"

BEGIN_EVENT_TABLE( ToolDlg, wxMiniFrame )

    EVT_CLOSE( ToolDlg::OnCloseWindow )

END_EVENT_TABLE()


wxBitmap* GetBitmap( const wxString& name )
{
    wxImage image( 32, 32 );
    bool loaded = false;
    wxString file;
    do
    {
#if wxUSE_LIBPNG
        file = a2dGlobals->GetIconPathList().FindValidPath( name + _T( ".png" ), false );
        if ( !file.IsEmpty() && image.LoadFile( file , wxBITMAP_TYPE_PNG ) )
        {
            loaded = true;
            break;
        }
#endif
        file = a2dGlobals->GetIconPathList().FindValidPath( name + _T( ".ico" ), false );
        if ( !file.IsEmpty() && image.LoadFile( file ) )
        {
            loaded = true;
            break;
        }
        file = a2dGlobals->GetIconPathList().FindValidPath( name + _T( ".bmp" ), false );
        if ( !file.IsEmpty() && image.LoadFile( file , wxBITMAP_TYPE_BMP ) )
        {
            loaded = true;
            break;
        }
    }
    while( false );

    if ( !loaded )
    {
        a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "invalid bitmap %s" ), name.c_str() );
    }
    return new wxBitmap( image );
}

ToolDlg::ToolDlg( wxFrame* parent ):
    wxMiniFrame( parent, -1, _T( "Tools" ), wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP | wxDEFAULT_DIALOG_STYLE | wxDIALOG_NO_PARENT )
{
    //a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_ACTIVATE_VIEW, this );

    //m_sizer = new wxBoxSizer( wxHORIZONTAL );
    //m_sizer = new wxBoxSizer( wxVERTICAL );
    m_sizer = new wxGridSizer( 2 );

    m_panel = new wxPanel( this, -1, wxDefaultPosition, wxDefaultSize, 0, _T( "panel" ) );
    m_panel->SetSizer( m_sizer );

    Populate();

    m_panel->SetAutoLayout( true );
    m_sizer->SetSizeHints( this );
    m_sizer->Fit( this );

    if ( parent )
    {
        // align on left side of parent if given
        wxPoint pt = parent->GetPosition();
        wxSize dim = GetSize();
        Move( pt.x - dim.x, pt.y );
    }
    else
    {
        Centre( wxBOTH );
    }
}

ToolDlg::~ToolDlg()
{
    //a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_ACTIVATE_VIEW, this );
}

void ToolDlg::Populate()
{
    wxBitmap* toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_Zoom" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_Zoom() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawPolygonL" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawPolygonL() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawPolygonL_Splined" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawPolygonL_Splined() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawPolylineL" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawPolylineL() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawPolylineL_Splined" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawPolylineL_Splined() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawRectangle" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawRectangle() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawCircle" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawCircle() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawEllipse" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawEllipse() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawEllipticArc" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawEllipticArc() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawEllipticArc_Chord" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawEllipticArc_Chord() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawArc" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawArc() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawArc_Chord" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawArc_Chord() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_DrawText" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_DrawText() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_Image" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_Image() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_Rotate" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_Rotate() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_Drag" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_Drag() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_Copy" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_Copy() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_Delete" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_Delete() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_RecursiveEdit" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_RecursiveEdit() );
    delete toolbitmap;

    toolbitmap = GetBitmap( _T( "CmdMenu_PushTool_Select" ) );
    AddCmdMenu( *toolbitmap, CmdMenu_PushTool_Select() );
    delete toolbitmap;
}

void ToolDlg::RemoveCmdMenu( const a2dMenuIdItem& cmdId )
{
    Disconnect( cmdId.GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ToolDlg::OnCmdMenuId ) );
    //!todo
}

void ToolDlg::AddCmdMenu( wxBitmap& bitmap, const a2dMenuIdItem& cmdId )
{
    Connect( cmdId.GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ToolDlg::OnCmdMenuId ) );
    wxBitmapButton* tool = new wxBitmapButton( m_panel, cmdId.GetId(), bitmap );
    tool->SetToolTip( cmdId.GetHelp() );
    m_sizer->Add( tool, 0, wxALL, 1 );
}

void ToolDlg::OnCmdMenuId( wxCommandEvent& event )
{
    // translate into a menu event, to be handled by command handler classes
    wxMenuEvent eventMenu( wxEVT_COMMAND_MENU_SELECTED, event.GetId() );

    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawingPart )
        return;

    drawingPart->GetDrawing()->GetCommandProcessor()->ProcessEvent( eventMenu );
    if ( drawingPart->GetDisplayWindow() && !drawingPart->GetDisplayWindow()->HasFocus() )
        drawingPart->GetDisplayWindow()->SetFocus();
}

void ToolDlg::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    Show( false );
}

