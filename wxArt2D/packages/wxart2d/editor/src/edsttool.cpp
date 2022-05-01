/*! \file canvas/src/sttool.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: sttool.cpp,v 1.389 2009/10/01 19:22:35 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/regex.h>
#include <wx/url.h>


#include "wx/docview/doccom.h"
#include "wx/canvas/canmod.h"
#include "wx/editor/recurdoc.h"
#include "wx/filename.h"

#include "wx/editor/edsttool.h"

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

IMPLEMENT_CLASS( a2dLinkTool, a2dStDrawTool )

const a2dCommandId a2dLinkTool::COMID_PushTool_Link( wxT( "PushTool_Link" ) );

BEGIN_EVENT_TABLE( a2dLinkTool, a2dStDrawTool )
    EVT_CHAR( a2dLinkTool::OnChar )
    EVT_MOUSE_EVENTS( a2dLinkTool::OnMouseEvent )
END_EVENT_TABLE()

a2dLinkTool::a2dLinkTool( a2dStToolContr* controller ): a2dStDrawTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_anotate = true;//false;
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_PENCIL );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );

    //i like transparent fill and specific stroke
    SetFill( *a2dTRANSPARENT_FILL );
    SetStroke( a2dStroke( *wxRED, 1, a2dSTROKE_DOT ) );
    m_font = *a2dDEFAULT_CANVASFONT;
}

a2dLinkTool::~a2dLinkTool()
{
}

void a2dLinkTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStDrawTool::AddAnotationToPendingUpdateArea();

    a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
    m_anotation.Printf( _T( "w %6.3f h %6.3f" ), rec->GetWidth(), rec->GetHeight() );

    wxCoord w, h, d, l;
    GetTextExtent( m_anotation, &w, &h, &d, &l );
    m_xanotation = m_x + ( int ) h;
    m_yanotation = m_y + ( int ) h;
    a2dStDrawTool::AddAnotationToPendingUpdateArea();
}

void a2dLinkTool::OnChar( wxKeyEvent& event )
{
    if ( !GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_RETURN:
            {
                m_stcontroller->Zoomout();
            }
            break;
            case WXK_SUBTRACT:
            {
                m_stcontroller->ZoomUndo();
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();
}

void a2dLinkTool::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }

    if ( GetBusy() )
        GetDrawingPart()->SetCursor( m_toolBusyCursor );
    else
        GetDrawingPart()->SetCursor( m_toolcursor );

    m_x = event.GetX();
    m_y = event.GetY();
    MouseToToolWorld( m_x, m_y, m_xwprev, m_ywprev );

    a2dRestrictionEngine* restrict = GetDrawing()->GetHabitat()->GetRestrictionEngine();
    if( restrict )
        restrict->RestrictPoint( m_xwprev, m_ywprev );

    if ( event.LeftDClick() && !GetBusy() )
    {

    }
    else if ( event.LeftDown() && !GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        a2dRect* rec = new a2dRect( m_xwprev, m_ywprev, 0, 0, 0 );
        rec->SetRoot( GetDrawingPart()->GetDrawing() );
        rec->SetFill( m_fill );
        rec->SetStroke( m_stroke );
        rec->Update( a2dCanvasObject::updatemask_force );

        m_original = rec;
        if ( !EnterBusyMode() )
            return;

        //special case to have the canvas itself recieve at least this one also.
        //because often the zoomtool is active as only tool, but
        //some object on the canvas are meant to react to MD (Href's etc.)
        event.Skip();
    }
    else if ( event.LeftDown() && GetBusy() )
    {
        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        CleanupToolObjects();

        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );

        double w = GetDrawer2D()->WorldToDeviceXRel( rec->GetWidth() );
        double h = GetDrawer2D()->WorldToDeviceYRel( rec->GetHeight() );

        static wxString link_dir = *( a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "APPLICATION_DATA" ) ) );

        static wxString filter = _T( "All files (*.*)| *.*| CVG files (*.cvg)| *.cvg" );

        wxFileName link_file;

        wxFileDialog dialog( GetDisplayWindow(), _T( "Select Image File" ), link_dir, link_file.GetFullPath(), filter, wxFD_OPEN );

        if ( dialog.ShowModal() != wxID_OK )
            return;

        // save for the next time
        link_dir = dialog.GetPath();
        link_file = dialog.GetFilename();

        wxFileName link_dirf = wxFileName( link_dir );
        link_dir = link_dirf.GetVolume() + link_dirf.GetVolumeSeparator() + link_dirf.GetPathWithSep( wxPATH_UNIX );

        a2dSmrtPtr<a2dText> otext;

        wxString link_fullpath = link_dir + link_file.GetFullName();

        double x1 = rec->GetBbox().GetMinX();
        double y1 = rec->GetBbox().GetMaxY();
        double link_w = fabs( rec->GetWidth() );
        double link_h = fabs( rec->GetHeight() );

        bool up = false;
        if ( GetDrawingPart() && GetDrawingPart()->GetDrawer2D() )
            up = !GetDrawingPart()->GetDrawer2D()->GetYaxis();

        a2dFont font = m_font;
        if ( fabs( w ) < 3 && fabs( h ) < 3 )
        {
            //use current font
            otext = new a2dCanvasDocumentReferenceAsText( link_fullpath, x1, y1, font, 0, up );
        }
        else
        {
            font.SetSize( link_h );
            otext = new a2dCanvasDocumentReferenceAsText( link_fullpath, x1, y1, font, 0, up );
        }
        //central font like this, else editing takes over the wrong one.
        GetDrawing()->GetHabitat()->SetFont( font );

        otext->SetRoot( GetDrawingPart()->GetDrawing() );
        otext->SetFill( *a2dTRANSPARENT_FILL );
        otext->SetStroke( *a2dTRANSPARENT_STROKE );
        otext->SetFill( m_fill );
        otext->SetStroke( m_stroke );
        otext->SetLayer( m_layer );

        bool backupstartedit = m_editatend;
        m_editatend = false;
        a2dREFOBJECTPTR_KEEPALIVE;
        FinishBusyMode();
        m_editatend = backupstartedit;

        m_original = otext;
        AddCurrent( otext );
        if ( !EnterBusyMode() )
            return;
        FinishBusyMode();
        GetDrawingPart()->Update( a2dCANVIEW_UPDATE_SYNC_DRAWERS );
    }
    else  if ( ( event.Dragging() && GetBusy() ) || ( event.Moving() && GetBusy() ) )
    {
        a2dRect* rec = wxStaticCast( m_canvasobject.Get(), a2dRect );
        rec->SetWidth( m_xwprev - rec->GetPosX() );
        rec->SetHeight( m_ywprev - rec->GetPosY() );

        m_pending = true;
    }
    else
        event.Skip();
}

//-----------------------------------------------------------
// a2dFollowLinkDocDrawCam
//-----------------------------------------------------------

IMPLEMENT_CLASS( a2dFollowLinkDocDrawCam, a2dStTool )

const a2dCommandId a2dFollowLinkDocDrawCam::COMID_PushTool_FollowLinkDocDrawCam( wxT( "PushTool_FollowLinkDocDrawCam" ) );

BEGIN_EVENT_TABLE( a2dFollowLinkDocDrawCam, a2dStTool )
    EVT_CHAR( a2dFollowLinkDocDrawCam::OnChar )
    EVT_MOUSE_EVENTS( a2dFollowLinkDocDrawCam::OnMouseEvent )
END_EVENT_TABLE()

a2dFollowLinkDocDrawCam::a2dFollowLinkDocDrawCam( a2dStToolContr* controller ): a2dStTool( controller )
{
    m_eventHandler = controller->GetDefaultToolEventHandlerFixedStyle();

    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_BULLSEYE );
    m_canvasobject = 0;
}

void a2dFollowLinkDocDrawCam::OnChar( wxKeyEvent& event )
{
    if ( GetBusy() )
    {
        switch( event.GetKeyCode() )
        {
            case WXK_SPACE:
            {
                FinishBusyMode();
            }
            break;
            default:
                event.Skip();
        }
    }
    else
        event.Skip();

}

void a2dFollowLinkDocDrawCam::OnMouseEvent( wxMouseEvent& event )
{
    if ( !m_active )
    {
        event.Skip();
        return;
    }
    GetDrawingPart()->SetCursor( m_toolcursor );

    int x = event.GetX();
    int y = event.GetY();

    //to world coordinates to do hit test in world coordinates
    double xw, yw;
    MouseToToolWorld( x, y, xw, yw );

    if ( event.LeftDown() )
    {
        m_canvasobject = GetDrawingPart()->IsHitWorld( xw, yw, wxLAYER_ALL, a2dCANOBJHITOPTION_LAYERS );

        if ( !m_canvasobject )
        {
            m_canvasobject = 0;
            event.Skip();
            return;
        }

        //if ( event.ControlDown() )
        {
            wxURI uri = m_canvasobject->GetURI();
            wxString link = uri.BuildURI();
            if ( !link.IsEmpty() )
            {
                wxString file = link;
                wxString scheme = uri.GetScheme();
                if ( scheme == wxT( "file" ) )  
                {
	                wxRegEx reVolume( wxT("^\\/[a-fA-F].*$") );
                    file = uri.GetPath();
                    if ( reVolume.Matches( file ) )
                        file = file.Mid( 1 );

                    if ( !::wxFileExists( file ) )
                    {
                        wxFileName fileo = wxFileName( file, wxPATH_UNIX );
                        file = *( a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "APPLICATION_DATA" ) ) ) + wxFileName::GetPathSeparator(wxPATH_UNIX) + fileo.GetFullName();
                        if ( !::wxFileExists( file ) )
                        {
                            //a2dGeneralGlobals->ReportErrorF( a2dError_FileCouldNotOpen, _( "Sorry, could not open file %s" ), file.c_str() );
                            a2dDocumentPtr doc;
                            a2dError res = a2dDocviewGlobals->GetDocviewCommandProcessor()->FileNew( doc );
                            doc->SetTitle( file, true );
                            doc->SetFilename( wxFileName( file ), true );
                            return;
                        }
                    }
                    a2dDocumentPtr doc;
                    a2dDocviewGlobals->GetDocviewCommandProcessor()->FileOpenCheck( doc, file, true );
                } 
            }
        }

        m_canvasobject->SetPending( true );
        CloseCommandGroup();
        m_canvasobject = 0;
        if ( m_oneshot )
            StopTool();
    }
    else
        event.Skip();

}

bool a2dFollowLinkDocDrawCam::StartFollowHere( double x, double y )
{
    int mouse_x = GetDrawer2D()->WorldToDeviceX( x );
    int mouse_y = GetDrawer2D()->WorldToDeviceY( y );

    return StartFollowHere( mouse_x, mouse_y );
}

bool a2dFollowLinkDocDrawCam::StartFollowHere( int x, int y )
{
    wxMouseEvent event( wxEVT_LEFT_DOWN );
    event.m_x = x;
    event.m_y = y;

    OnMouseEvent( event );

    return true;
}

//----------------------------------------------------------------------------
// a2dMasterTagGroups3Link 
//----------------------------------------------------------------------------

IMPLEMENT_CLASS( a2dMasterTagGroups3Link , a2dMasterTagGroups3 )

BEGIN_EVENT_TABLE( a2dMasterTagGroups3Link , a2dMasterTagGroups3 )
END_EVENT_TABLE()

a2dMasterTagGroups3Link ::a2dMasterTagGroups3Link ( a2dStToolContr* controller ): 
	a2dMasterTagGroups3( controller )
{
    m_drawWire = false;

    controller->SetDefaultBehavior( controller->GetDefaultBehavior() | wxTC_NoDefaultKeys | wxTC_NoDefaultMouseActions );
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_Select  );
}

a2dMasterTagGroups3Link::~a2dMasterTagGroups3Link ()
{
}

wxString a2dMasterTagGroups3Link::GetToolString() const
{
	wxString str = "Edit";
	if ( m_drawWire )
		return str + " Wires";
	if ( m_dlgOrEdit )
		return str + " Dlg";
	return str;
}

void a2dMasterTagGroups3Link::PushEditTool( a2dCanvasObject* hit )
{
    a2dSmrtPtr< a2dObjectEditTool > tool = new a2dObjectEditTool( m_stcontroller );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();
    tool->SetCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditSegment ) );
    tool->SetBusyCursorType( a2dCanvasGlobals->GetCursor( a2dCURSOR_EditSegment ) );

    if ( hit )
    {

    }
    //tool->SetEvtHandler( m_eventHandler  );
    
    m_stcontroller->PushTool( tool );
    SelectHitObject( hit );
    tool->StartToEdit( hit );
}

void a2dMasterTagGroups3Link::PushLinkTool( a2dCanvasObject* hit )
{
    a2dSmrtPtr< a2dFollowLinkDocDrawCam > tool = new a2dFollowLinkDocDrawCam( m_stcontroller );

    tool->SetShowAnotation( m_anotate );
    tool->SetOneShot();

    if ( hit )
    {

    }
    //tool->SetEvtHandler( m_eventHandler  );
    
    m_stcontroller->PushTool( tool );
    SelectHitObject( hit );
    tool->StartFollowHere( m_x, m_y );
}

void a2dMasterTagGroups3Link::EditDlgOrHandles( a2dCanvasObject* hit, bool modifier, bool noHandleEditForWire )
{
	if ( hit )
	{
		if ( m_dlgOrEdit && !modifier )
		{
        	a2dCanvasDocumentReferenceAsText* link = wxDynamicCast( hit, a2dCanvasDocumentReferenceAsText );
            if ( link )
		    {
			    hit->SetSelected( m_modehitLastSelectState );
			    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
			    m_mode = mode_edit;
			    PushLinkTool( hit );
		    }
            else if ( !m_dlgOrEditModal )
				GetDrawing()->GetCanvasCommandProcessor()->ShowDlgStyle( true );
			else
			{
				bool hitSel = hit->GetSelected();
				hit->SetSelected( true );

                a2dExtFill eFi;
                a2dExtStroke eSt;
                wxUint32 nrst = 0;
                wxUint32 nrfi = 0;

				bool withFill = false;
                a2dCanvasObjectList* objects = GetDrawingPart()->GetShowObject()->GetChildObjectList();
                //nrst = a2dSetExtStroke( eSt, objects, a2dCanvasOFlags::SELECTED, a2dCanvasGlobals->GetLayerSetup() );
                //nrfi = a2dSetExtFill( eFi, objects, a2dCanvasOFlags::SELECTED, GetDrawing()->GetHabitat()->GetLayerSetup() );
                nrst = a2dSetExtStroke( eSt, objects, a2dCanvasOFlags::SELECTED, GetDrawing()->GetLayerSetup() );
                nrfi = a2dSetExtFill( eFi, objects, a2dCanvasOFlags::SELECTED, GetDrawing()->GetLayerSetup() );
				bool showDlg = true;
                if ( nrst + nrfi == 0 )
                {
                    //eFi.Set( hit->GetFill() );  
                    //eSt.Set( hit->GetStroke() ); 
					//withFill = true; // we want to be able to choose a fill
					showDlg = false;
                }
				else
					withFill = 0 != nrfi;

				if ( showDlg )
				{
					a2dDialogStyle dlg( NULL, false, withFill, true );
					dlg.SetUnitsScale( GetDrawing()->GetUnitsScale() );
					dlg.SetExtFill(eFi);
					dlg.SetExtStroke(eSt);
					//dlg.SetCustomColors(....);

					if ( wxID_OK == dlg.ShowModal() )
					{
						eSt = dlg.GetExtStroke(); // get edited ExtStroke from dialog
						eFi = dlg.GetExtFill(); // get edited ExtFill from dialog

						if ( !hit->GetSelected() )
						{
							hit->SetFill( eFi.Get( hit->GetFill() ) );
							hit->SetStroke( eSt.Get( hit->GetStroke() ) );
						}
						else
							GetDrawing()->GetCanvasCommandProcessor()->Submit( new a2dCommand_SetFillStrokeExtMask( GetDrawingPart()->GetShowObject(), eSt, eFi, a2dCanvasOFlags::BIN2 ) );
					}
					//SetCustomColors(dlg.GetCustomColors());
				}
				hit->SetSelected( hitSel );
                objects->SetSpecificFlags( false, a2dCanvasOFlags::BIN2 );
			}
		}
		else
		{
            {
			    a2dWirePolylineL* wire = wxDynamicCast( hit, a2dWirePolylineL );
			    if ( (wire && wire->GetSelected() ) || (wire && !noHandleEditForWire ) || !wire )
			    {
				    hit->SetSelected( m_modehitLastSelectState );
				    GetDrawingPart()->SetCursor( a2dCanvasGlobals->GetCursor( a2dCURSOR_Select ) );
				    m_mode = mode_edit;
				    PushEditTool( hit );
			    }
            }
		}
	}
}

