/*! \file editor/src/sttoolmes.cpp
    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: sttoolmes.cpp,v 1.9 2009/10/01 19:22:36 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/sttoolmes.h"

#define ID_X 10001
#define ID_Y 10006
#define ID_LISTBOX 10002
#define ID_HIDE 10003
#define ID_CLEAR 10004
#define ID_SUM 10005

/*!
 * a2dMeasureDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( a2dMeasureDlg, wxDialog )

/*!
 * a2dMeasureDlg event table definition
 */

BEGIN_EVENT_TABLE( a2dMeasureDlg, wxDialog )
    EVT_CLOSE( a2dMeasureDlg::OnCloseWindow )
    EVT_IDLE( a2dMeasureDlg::OnIdle )

    EVT_TEXT_ENTER( ID_X, a2dMeasureDlg::OnXEnter )
    EVT_UPDATE_UI( ID_X, a2dMeasureDlg::OnXUpdate )

    EVT_TEXT_ENTER( ID_Y, a2dMeasureDlg::OnYEnter )
    EVT_UPDATE_UI( ID_Y, a2dMeasureDlg::OnYUpdate )

    EVT_LISTBOX( ID_LISTBOX, a2dMeasureDlg::OnListboxSelected )
    EVT_UPDATE_UI( ID_LISTBOX, a2dMeasureDlg::OnListboxUpdate )

    EVT_BUTTON( ID_HIDE, a2dMeasureDlg::OnHideClick )

    EVT_BUTTON( ID_CLEAR, a2dMeasureDlg::OnClearClick )

    EVT_UPDATE_UI( ID_SUM, a2dMeasureDlg::OnSumUpdate )

    EVT_COM_EVENT( a2dMeasureDlg::OnComEvent )
    EVT_DO( a2dMeasureDlg::OnDoEvent )
    EVT_UNDO( a2dMeasureDlg::OnDoEvent )

END_EVENT_TABLE()

/*!
 * a2dMeasureDlg constructors
 */

a2dMeasureDlg::a2dMeasureDlg( )
{
    a2dCanvasGlobals->GetActiveDrawingPart()->GetDrawing()->GetCanvasCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );
}

a2dMeasureDlg::~a2dMeasureDlg( )
{
    a2dCanvasGlobals->DisconnectEvent( a2dEVT_COM_EVENT, this );
}

a2dMeasureDlg::a2dMeasureDlg( a2dHabitat* habitat, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_habitat = habitat;
    a2dCanvasGlobals->GetActiveDrawingPart()->GetDrawing()->GetCanvasCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );
    a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );
    Create( parent, id, caption, pos, size, style );
}

/*!
 * a2dMeasureDlg creator
 */

bool a2dMeasureDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_x = NULL;
    m_y = NULL;
    m_lb = NULL;
    m_hide = NULL;
    m_clear = NULL;
    m_sum = NULL;

    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
    Centre();
    return TRUE;
}

/*!
 * Control creation for a2dMeasureDlg
 */

void a2dMeasureDlg::CreateControls()
{
    a2dMeasureDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer3, 0, wxGROW | wxALL, 1 );

    m_x = new wxTextCtrl( itemDialog1, ID_X, _( "0" ), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    m_x->SetHelpText( _( "X of vertex" ) );
    if ( ShowToolTips() )
        m_x->SetToolTip( _( "X of vertex" ) );
    itemBoxSizer3->Add( m_x, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    m_y = new wxTextCtrl( itemDialog1, ID_Y, _( "0" ), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    m_y->SetHelpText( _( "Y of vertex" ) );
    if ( ShowToolTips() )
        m_y->SetToolTip( _( "Y of vertex" ) );
    itemBoxSizer3->Add( m_y, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer6, 1, wxGROW | wxALL, 1 );

    wxString* m_lbStrings = NULL;
    m_lb = new wxListBox( itemDialog1, ID_LISTBOX, wxDefaultPosition, wxDefaultSize, 0, m_lbStrings, wxLB_SINGLE | wxLB_NEEDED_SB );
    m_lb->SetHelpText( _( "list of measured points" ) );
    if ( ShowToolTips() )
        m_lb->SetToolTip( _( "list of measured points" ) );
    itemBoxSizer6->Add( m_lb, 1, wxGROW | wxALL, 1 );

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer8, 0, wxGROW | wxALL, 1 );

    m_hide = new wxButton( itemDialog1, ID_HIDE, _( "Hide" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add( m_hide, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    m_clear = new wxButton( itemDialog1, ID_CLEAR, _( "Clear" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_clear->SetHelpText( _( "Clear all measured points" ) );
    if ( ShowToolTips() )
        m_clear->SetToolTip( _( "Clear all measured points" ) );
    itemBoxSizer8->Add( m_clear, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    m_sum = new wxTextCtrl( itemDialog1, ID_SUM, _( "0" ), wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_PROCESS_ENTER );
    m_sum->SetHelpText( _( "Sum of all measured points to rel start" ) );
    if ( ShowToolTips() )
        m_sum->SetToolTip( _( "Sum of all measured points to rel start" ) );
    itemBoxSizer8->Add( m_sum, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1 );
}

/*!
 * Should we show tooltips?
 */

bool a2dMeasureDlg::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap a2dMeasureDlg::GetBitmapResource( const wxString& name )
{
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon a2dMeasureDlg::GetIconResource( const wxString& name )
{
    return wxNullIcon;
}
/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_X
 */

void a2dMeasureDlg::OnXEnter( wxCommandEvent& event )
{
    double x, y;
    m_x->GetValue().ToDouble( &x );
    m_y->GetValue().ToDouble( &y );

    m_habitat->SetRelativeStart( x, y );

    m_lb->Clear();
    a2dPoint2D p1;
    for( a2dVertexList::iterator iter = m_vertexes.begin(); iter != m_vertexes.end(); ++iter )
    {
        a2dLineSegment* seg = *iter;
        p1 = seg->GetPoint();
        AppendMeasure( p1.m_x, p1.m_y );
    }
    wxString tmp;
    tmp.Printf( wxT( "%lg" ), m_vertexes.Length() );
    m_sum->SetValue( tmp );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_X
 */

void a2dMeasureDlg::OnXUpdate( wxUpdateUIEvent& event )
{
    event.Skip();
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_Y
 */

void a2dMeasureDlg::OnYEnter( wxCommandEvent& event )
{
    double x, y;
    m_x->GetValue().ToDouble( &x );
    m_y->GetValue().ToDouble( &y );

    m_habitat->SetRelativeStart( x, y );

    m_lb->Clear();
    a2dPoint2D p1;
    for( a2dVertexList::iterator iter = m_vertexes.begin(); iter != m_vertexes.end(); ++iter )
    {
        a2dLineSegment* seg = *iter;
        p1 = seg->GetPoint();
        AppendMeasure( p1.m_x, p1.m_y );
    }
    wxString tmp;
    tmp.Printf( wxT( "%lg" ), m_vertexes.Length() );
    m_sum->SetValue( tmp );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_Y
 */

void a2dMeasureDlg::OnYUpdate( wxUpdateUIEvent& event )
{
    event.Skip();
}

/*!
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX
 */

void a2dMeasureDlg::OnListboxSelected( wxCommandEvent& event )
{
    event.Skip();
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_LISTBOX
 */

void a2dMeasureDlg::OnListboxUpdate( wxUpdateUIEvent& event )
{
    event.Skip();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_HIDE
 */

void a2dMeasureDlg::OnHideClick( wxCommandEvent& event )
{
    Show( FALSE );
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR
 */

void a2dMeasureDlg::OnClearClick( wxCommandEvent& event )
{
    m_lb->Clear();
    m_vertexes.clear();
    event.Skip();
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_SUM
 */

void a2dMeasureDlg::OnSumUpdate( wxUpdateUIEvent& event )
{
    event.Skip();
}


/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_DIALOG
 */

void a2dMeasureDlg::OnCloseWindow( wxCloseEvent& event )
{
    Show( FALSE );
}

/*!
 * wxEVT_IDLE event handler for ID_DIALOG
 */

void a2dMeasureDlg::OnIdle( wxIdleEvent& event )
{
    event.Skip();
}

void a2dMeasureDlg::OnComEvent( a2dComEvent& event )
{
    if ( GetEvtHandlerEnabled() )
    {
        if ( event.GetId() == a2dDrawingPart::sig_changedShowObject )
        {
            m_lb->Clear();
            m_vertexes.clear();
        }
        else if ( event.GetId() == a2dCanvasCommandProcessor::sig_SetRelativeStart )
        {
            double x = m_habitat->GetRelativeStartX();
            double y = m_habitat->GetRelativeStartY();
            wxString coordstr;
            coordstr.Printf( wxT( "%lg" ), x );
            m_x->SetValue( coordstr );
            coordstr.Printf( wxT( "%lg" ), y );
            m_y->SetValue( coordstr );
        }
        else if ( event.GetId() == a2dCanvasCommandProcessor::sig_AddMeasurement )
        {
            a2dCanvasCommandProcessor* proc = wxStaticCast( event.GetEventObject(), a2dCanvasCommandProcessor );
            m_vertexes.push_back( new a2dLineSegment( proc->m_meas_x, proc->m_meas_y ) );
            wxString tmp;
            tmp.Printf( wxT( "%lg" ), m_vertexes.Length() );
            m_sum->SetValue( tmp );

            AppendMeasure( proc->m_meas_x, proc->m_meas_y );
        }
        else if ( event.GetId() == a2dCanvasCommandProcessor::sig_ClearMeasurements )
        {
            m_lb->Clear();
            m_vertexes.clear();
        }
    }

}

void a2dMeasureDlg::OnDoEvent( a2dCommandProcessorEvent& event )
{
    if ( GetEvtHandlerEnabled() )
    {
    }
}

void a2dMeasureDlg::AppendMeasure( double x, double y )
{
    int exp = 1;
    double rel_x, rel_y, length, angle;
    double start_x = wxAtoi( m_x->GetValue() );
    double start_y = wxAtoi( m_y->GetValue() );

    rel_x = x - start_x;
    rel_y = y - start_y;
    length = sqrt( pow( rel_x, 2 ) + pow( rel_y, 2 ) );
    if ( rel_x != 0 && rel_y != 0 )
        angle = asin( rel_y / length ) * 180 / M_PI;
    else
        angle = 0;
    if ( rel_x < 0 )
        angle = 180 - angle;

    a2dDrawing* drawing = NULL;//todo GetDrawing();
    wxString unitmeasure = wxT( "non" );
    double scaleToMeters = 1;
    if ( drawing )
    {
        unitmeasure = drawing->GetUnits();
        scaleToMeters = drawing->GetUnitsScale();
    }

    double multi = 1;
    a2dDoMu::GetMultiplierFromString( unitmeasure, multi );

    x *= scaleToMeters / multi;
    y *= scaleToMeters / multi;


    wxString tmp;
    //tmp.Printf( "(%.*f, %.*f) %s dx:%.*f dy:%.*f  l:%.*f  angle:%.*f", -exp, command->m_x, -exp, command->m_y,
    //            "um", -exp, rel_x, -exp, rel_y, -exp, length, -exp, angle);
    tmp.Printf( wxT( "(%lg, %lg) %s dx:%lg dy:%lg l:%lg angle:%lg" ), x, y,
                unitmeasure.mb_str(), rel_x, rel_y, length, angle );

    m_lb->Append( tmp );
    m_lb->SetSelection( m_lb->GetCount() - 1, true );
}

IMPLEMENT_CLASS( a2dMeasureTool, a2dDrawPolylineLTool )

const a2dCommandId a2dMeasureTool::COMID_PushTool_Measure( wxT( "PushTool_Measure" ) );

BEGIN_EVENT_TABLE( a2dMeasureTool, a2dDrawPolylineLTool )
    EVT_CHAR( a2dMeasureTool::OnChar )
    EVT_MOUSE_EVENTS( a2dMeasureTool::OnMouseEvent )
END_EVENT_TABLE()

a2dMeasureTool::a2dMeasureTool( a2dStToolContr* controller ): a2dDrawPolylineLTool( controller )
{
    m_eventHandler = new a2dStToolFixedToolStyleEvtHandler( controller );

    m_anotate = true;
    m_editatend = false;
    m_toolcursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );
    m_toolBusyCursor = a2dCanvasGlobals->GetCursor( a2dCURSOR_CROSS );

    //i like transparent fill and specific stroke
    SetFill( *a2dTRANSPARENT_FILL );
    SetStroke( a2dStroke( *wxBLACK, 2, a2dSTROKE_LONG_DASH ) );
}

a2dMeasureTool::~a2dMeasureTool()
{
}

void a2dMeasureTool::GenerateAnotation()
{
    // Add old (and later new) annotation area to pending region
    a2dStTool::AddAnotationToPendingUpdateArea();

    a2dPolylineL* poly = wxStaticCast( m_canvasobject.Get(), a2dPolylineL );
    double len = poly->GetSegments()->Length();
    m_anotation.Printf( _T( "x %6.3f y %6.3f l %6.3f" ), m_xwprev, m_ywprev, len );
    m_xanotation = m_x;
    m_yanotation = m_y;

    wxCoord w, h, descent, external;
    GetTextExtent( m_anotation, &w, &h, &descent, &external );

    m_xanotation = m_x + h;
    m_yanotation = m_y + h;

    a2dStTool::AddAnotationToPendingUpdateArea();
}

void a2dMeasureTool::OnChar( wxKeyEvent& event )
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

void a2dMeasureTool::OnMouseEvent( wxMouseEvent& event )
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
        GetCanvasCommandProcessor()->ClearMeasurements();
        GetDrawing()->GetHabitat()->SetRelativeStart( m_xwprev, m_ywprev );
    }
    if ( event.LeftDClick() && GetBusy() )
    {
        // force this, since this tool does never want that.
        m_editatend = false;
        GetCanvasCommandProcessor()->ClearMeasurements();
        event.Skip();
    }
    else if ( event.LeftDown() && !GetBusy() )
    {
        GetCanvasCommandProcessor()->SetRelativeStart( m_xwprev, m_ywprev );

        GetDrawing()->GetHabitat()->SetLastXyEntry( m_xwprev, m_ywprev );
        m_original = m_templateObject->TClone( clone_childs | clone_members );
        m_original->SetRoot( GetDrawing() );
        m_original->Translate( m_xwprev, m_ywprev );
        m_original->EliminateMatrix();
        m_original->SetContourWidth( m_contourwidth );
        m_original->SetStroke( m_stroke );
        m_original->SetFill( m_fill );
        m_original->SetLayer( m_layer );
        a2dPolylineL* line = wxStaticCast( m_original.Get(), a2dPolylineL );
        if ( line->GetBegin() && line->GetBegin()->GetFlag( a2dCanvasOFlags::normalized ) )
        {
            line->GetBegin()->SetSpecificFlags( false, a2dCanvasOFlags::normalized );
            double norm = GetDrawingPart()->GetDrawing()->GetNormalizeScale();
            line->GetBegin()->Scale( norm, norm );
        }
        if ( line->GetEnd() && line->GetEnd()->GetFlag( a2dCanvasOFlags::normalized ) )
        {
            line->GetEnd()->SetSpecificFlags( false, a2dCanvasOFlags::normalized );
            double norm = GetDrawingPart()->GetDrawing()->GetNormalizeScale();
            line->GetEnd()->Scale( norm, norm );
        }

        EnterBusyMode();
        AddDecorationObject( m_original );

        m_points = wxStaticCast( m_canvasobject.Get(), a2dPolylineL )->GetSegments();

        a2dGeneralGlobals->RecordF( this, _T( "polyline" ) );

        // add point to the original
        if ( m_reverse )
            GetCanvasCommandProcessor()->Submit(
                new a2dCommand_AddPoint( m_original, m_xwprev, m_ywprev, 0 )
            );
        else
            GetCanvasCommandProcessor()->Submit(
                new a2dCommand_AddPoint( m_original, m_xwprev, m_ywprev, -1 )
            );

        m_point = new a2dLineSegment( m_xwprev, m_ywprev );
        AddPoint( m_point );

        a2dGeneralGlobals->RecordF( this, _T( "addpoint %f %f " ), m_xwprev, m_ywprev );

        //add second point to drag
        m_point = new a2dLineSegment( m_xwprev, m_ywprev );
        AddPoint( m_point );

        m_prev_x = m_xwprev;
        m_prev_y = m_ywprev;

        if( restrict )
            restrict->SetRestrictPoint( m_xwprev, m_ywprev );

        m_pending = true;
    }
    else if ( event.LeftDown() && GetBusy() )
    {
        GetCanvasCommandProcessor()->AddMeasurement( m_xwprev, m_ywprev );
        event.Skip();
    }
    else
        event.Skip();
}
