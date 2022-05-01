/*! \file editor/src/layerdlg.cpp
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: layerdlg.cpp,v 1.68 2009/07/04 10:26:39 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/styledialg.h"

#include <wx/statline.h>

#include "wx/canvas/layerdlg.h"

#if defined(__WXMSW__) && defined(__MEMDEBUG__)
#include <wx/msw/msvcrt.h>
#endif

#define LPD_B_W     20
#define LPD_B_H     20

#ifdef __UNIX__
#define LPD_PATTERNBOX_W    50
#else
#define LPD_PATTERNBOX_W    30
#endif

const int    LAY_BUTTON_HIDE     = wxID_HIGHEST + 4801 ;

const int    LAY_BUTTON_CANCEL   = wxID_HIGHEST + 4802 ;

const int    LAY_BUTTON_VISIBLE   = wxID_HIGHEST + 4803 ;

const int    LAY_BUTTON_OUTLINE   = wxID_HIGHEST + 5961 ;

const int    LAY_BUTTON_READ   = wxID_HIGHEST + 4804 ;

const int    LAY_BUTTON_SELECT = wxID_HIGHEST + 4805 ;

const int    LAY_BUTTON_PREV   = wxID_HIGHEST + 4806 ;

const int    LAY_BUTTON_NEXT   = wxID_HIGHEST + 4807 ;

const int    LAY_EDIT_NAME     = wxID_HIGHEST + 4808 ;

const int    LAY_BUTTON_RESTORE = wxID_HIGHEST + 4812 ;

const int    LAY_BUTTON_SAVETOFILE = wxID_HIGHEST + 4813 ;

const int    LAY_BUTTON_LOADFROMFILE = wxID_HIGHEST + 4814 ;

const int    LAY_BUTTON_ADDLAYER = wxID_HIGHEST + 4815 ;

const int    LAY_BUTTON_SORT = wxID_HIGHEST + 4816 ;


const int    ID_Data = wxID_HIGHEST + 5970;

// check document every 3 seconds
#define CHECKTIME 3000

BEGIN_EVENT_TABLE( LayerPropertiesDialog, wxDialog )

    EVT_DATAVIEW_COLUMN_HEADER_CLICK( ID_Data, LayerPropertiesDialog::Cm_SortColumn )

	EVT_COM_EVENT( LayerPropertiesDialog::OnComEvent )
    EVT_DO( LayerPropertiesDialog::OnDoEvent )

    EVT_BUTTON    ( LAY_BUTTON_SORT, LayerPropertiesDialog::CmSort )

    EVT_BUTTON    ( LAY_BUTTON_HIDE, LayerPropertiesDialog::CmHide )

    EVT_BUTTON    ( LAY_BUTTON_CANCEL, LayerPropertiesDialog::CmCancel )

    EVT_BUTTON    ( LAY_BUTTON_RESTORE, LayerPropertiesDialog::CmRestore )

    EVT_BUTTON    ( LAY_BUTTON_SAVETOFILE, LayerPropertiesDialog::CmSaveToFile )

    EVT_BUTTON    ( LAY_BUTTON_LOADFROMFILE, LayerPropertiesDialog::CmLoadFromFile )

    EVT_BUTTON    ( LAY_BUTTON_ADDLAYER, LayerPropertiesDialog::CmAddLayer )

    EVT_BUTTON    ( LAY_BUTTON_VISIBLE, LayerPropertiesDialog::CmVisible )

    EVT_BUTTON    ( LAY_BUTTON_READ, LayerPropertiesDialog::CmRead )

    EVT_BUTTON    ( LAY_BUTTON_OUTLINE, LayerPropertiesDialog::CmOutLine )

    EVT_BUTTON    ( LAY_BUTTON_SELECT, LayerPropertiesDialog::CmSelect )

    EVT_CLOSE( LayerPropertiesDialog::OnCloseWindow )

    EVT_DATAVIEW_ITEM_ACTIVATED( ID_Data, LayerPropertiesDialog::Cm_edit )
    EVT_DATAVIEW_ITEM_VALUE_CHANGED( ID_Data, LayerPropertiesDialog::Cm_changed )

END_EVENT_TABLE()


//****** IMPLEMENTATION OF THE LAYER PROPERTIES DIALOG CLASS ******

a2dLayerInfo* LayerPropertiesDialog::GetTargetLayerInfo( int i )
{
    if ( m_onorder )
        return m_targetLayerSetup->GetLayerIndex()[i];
    else
        return m_targetLayerSetup->GetLayerIndex()[i];
}



// Constructor
LayerPropertiesDialog::LayerPropertiesDialog( wxFrame* parent, a2dLayers* layersetup, bool activeDrawingPart, const wxString& title, long style, const wxString& name ):
    wxDialog( parent, -1, title, wxDefaultPosition, wxDefaultSize, style, name )

{
    m_activeDrawingPart = activeDrawingPart;
    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if (  m_activeDrawingPart && !drawingPart )
        return;

    a2dLayers* layers = drawingPart->GetDrawing()->GetLayerSetup();
    if (  m_activeDrawingPart && !layers )
        return;

    if ( m_activeDrawingPart && drawingPart && layersetup )
        layersetup->SetAvailable( drawingPart->GetShowObject()->GetRoot() );


/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_CHANGED_DOCUMENT, this );
    //a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_ACTIVATE_VIEW, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( wxEVT_REDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );
*/
    if ( m_activeDrawingPart )
    {
        a2dCanvasGlobals->ConnectEvent( a2dEVT_COM_EVENT, this );
        drawingPart->GetDrawing()->GetCommandProcessor()->ConnectEvent( a2dEVT_COM_EVENT, this );    
    }
    //drawingPart->ConnectEvent( a2dEVT_COM_EVENT, this );

    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );

    m_dialogLastUpdate = wxDateTime::Now();

    m_fillpatterns = new FillPatterns();

    m_parent = parent;

    m_onorder = false;

    // no standard colours
    // set extra part of dialog true
    m_cdata     = new wxColourData();
    m_cdata->SetChooseFull( true );

    long style_noresize;
#ifdef __UNIX__
    style_noresize = ( wxCAPTION );
#else
    style_noresize = ( wxSYSTEM_MENU | wxCAPTION );
#endif

    // make colour dialog
    m_cd = new wxColourDialog( this, m_cdata );


    wxSizer* all = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* scrollsiz = new wxBoxSizer(wxHORIZONTAL);
    all->Add(scrollsiz,  1, wxGROW|wxALL, 5 );


    wxScrolledWindow* scrollLayers = new wxScrolledWindow( this, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    scrollLayers->SetScrollbars(1, 1, 0, 0);
    scrollsiz->Add( scrollLayers, 1, wxGROW|wxALL, 5);
    scrollLayers->SetMinSize(wxSize(600, 200));

    m_lc = new wxDataViewListCtrl( scrollLayers, ID_Data, wxDefaultPosition,
                                wxDefaultSize, style );


    m_lc->AppendTextColumn( "Nr", wxDATAVIEW_CELL_INERT, 40 );
    m_lc->AppendTextColumn( "Name", wxDATAVIEW_CELL_EDITABLE );
    m_lc->AppendToggleColumn( "Avai", wxDATAVIEW_CELL_INERT, 30 );
    m_lc->AppendToggleColumn( "Vis", wxDATAVIEW_CELL_ACTIVATABLE, 30 );
    m_lc->AppendToggleColumn( "Sel", wxDATAVIEW_CELL_ACTIVATABLE, 30 );
    m_lc->AppendToggleColumn( "Outl", wxDATAVIEW_CELL_ACTIVATABLE, 35 );
    m_lc->AppendToggleColumn( "Read", wxDATAVIEW_CELL_ACTIVATABLE, 40 );

    m_lc->AppendBitmapColumn( "fill", 7, wxDATAVIEW_CELL_INERT, 40  );
    m_lc->AppendBitmapColumn( "stroke", 8, wxDATAVIEW_CELL_INERT, 40  );
    m_lc->AppendBitmapColumn( "pattern", 9, wxDATAVIEW_CELL_INERT, 50 );

    m_lc->AppendTextColumn( "inMap", wxDATAVIEW_CELL_EDITABLE, 40  );
    m_lc->AppendTextColumn( "outMap", wxDATAVIEW_CELL_EDITABLE, 40  );
    m_lc->AppendTextColumn( "order", wxDATAVIEW_CELL_EDITABLE, 40  );

    wxSizer *lcsiz = new wxBoxSizer( wxHORIZONTAL );
    lcsiz->Add( m_lc, 1, wxGROW|wxALL, 5);
    scrollLayers->SetSizer(lcsiz);

    wxStaticBox* item1 = new wxStaticBox( this, -1, _T( "" ) );
    wxSizer* botline = new wxStaticBoxSizer( item1, wxVERTICAL );

    wxSizer* botline1 = new wxBoxSizer( wxHORIZONTAL );
    botline->Add( botline1 );


    m_buttonSORT  = new wxButton( this, LAY_BUTTON_SORT,  _T( "Sort Order" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    botline1->Add( m_buttonSORT, 0, wxALIGN_CENTRE | wxALL, 0 );

    m_buttonVISIBLE  = new wxButton( this, LAY_BUTTON_VISIBLE,  _T( "Hide All" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    botline1->Add( m_buttonVISIBLE, 0, wxALIGN_CENTRE | wxALL, 0 );
    m_buttonSELECT  = new wxButton( this, LAY_BUTTON_SELECT,  _T( "None Selectable" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    botline1->Add( m_buttonSELECT, 0, wxALIGN_CENTRE | wxALL, 0 );
    m_buttonREAD    = new wxButton( this, LAY_BUTTON_READ,    _T( "Read Non" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    botline1->Add( m_buttonREAD, 0, wxALIGN_CENTRE | wxALL, 0 );
    m_buttonOutLine = new wxButton( this, LAY_BUTTON_OUTLINE, _T( "OutLine" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    botline1->Add( m_buttonOutLine, 0, wxALIGN_CENTRE | wxALL, 0 );

    botline->Add( new wxStaticLine( this, -1, wxDefaultPosition,  wxSize( 200, 2 ), wxLI_HORIZONTAL ) , 0, wxALIGN_LEFT | wxALL, 2 );

    wxSizer* botline2 = new wxBoxSizer( wxHORIZONTAL );
    botline->Add( botline2 );

    m_buttonHIDE     = new wxButton( this, LAY_BUTTON_HIDE,  _T( "Hide" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    botline2->Add( m_buttonHIDE, 0, wxALIGN_CENTRE | wxALL, 0 );
    m_buttonCANCEL  = new wxButton( this, LAY_BUTTON_CANCEL, _T( "Cancel" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    botline2->Add( m_buttonCANCEL, 0, wxALIGN_CENTRE | wxALL, 0 );

    botline2->Add( new wxStaticLine( this, -1, wxDefaultPosition, wxSize( 2, 20 ), wxLI_VERTICAL ) , 0, wxALIGN_CENTRE | wxALL, 2 );

    m_buttonRESTORE = new wxButton( this, LAY_BUTTON_RESTORE, _T( "Restore" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );

    wxSizer* botline3 = new wxBoxSizer( wxHORIZONTAL );
    botline->Add( botline3 );
    botline3->Add( m_buttonRESTORE, 0, wxALIGN_CENTRE | wxALL, 0 );
    m_buttonSAVETOFILE = new wxButton( this, LAY_BUTTON_SAVETOFILE, _T( "Save File" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    botline3->Add( m_buttonSAVETOFILE, 0, wxALIGN_CENTRE | wxALL, 0 );
    m_buttonLOADFROMFILE = new wxButton( this, LAY_BUTTON_LOADFROMFILE, _T( "Load File" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    botline3->Add( m_buttonLOADFROMFILE, 0, wxALIGN_CENTRE | wxALL, 0 );
    m_buttonAddLayer = new wxButton( this, LAY_BUTTON_ADDLAYER, _T( "Add Layer" ), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    botline3->Add( m_buttonAddLayer, 0, wxALIGN_CENTRE | wxALL, 0 );

    all->Add( botline );

    SetSizer( all );

    all->Fit( this );
    SetAutoLayout( true );

    all->SetSizeHints( this );

    Centre( wxBOTH );
    Layout();

    Init( layersetup );
}

LayerPropertiesDialog::~LayerPropertiesDialog()
{
/*
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_DO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_UNDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( wxEVT_REDO, this );
    a2dDocviewGlobals->GetDocviewCommandProcessor()->DisconnectEvent( a2dEVT_COM_EVENT, this );
*/
    a2dCanvasGlobals->DisconnectEvent( a2dEVT_COM_EVENT, this );

    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if (  m_activeDrawingPart && !drawingPart )
    {
        drawingPart->GetDrawing()->GetCommandProcessor()->DisconnectEvent( a2dEVT_COM_EVENT, this );    
    }
    delete m_fillpatterns;

    delete m_cdata;
    delete m_cd;
}

void LayerPropertiesDialog::Init( a2dLayers* layersetup )
{
	if ( !layersetup )
		return;

    m_backuplayersetup = wxStaticCast( layersetup->Clone( a2dObject::clone_deep ) , a2dLayers );
    m_targetLayerSetup = layersetup;

    FillControls();

}

void LayerPropertiesDialog::OnComEvent( a2dComEvent& event )
{
    if ( GetEvtHandlerEnabled() && m_activeDrawingPart )
    {
		a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();

        if ( !part || !part->GetDrawing() )
            return;

        if ( event.GetId() == a2dDrawingPart::sig_changedLayers )
        {
            Init( part->GetDrawing()->GetLayerSetup() );
        }
        else if ( event.GetId() == a2dDrawing::sig_changedLayer )
        {
            //Init( doc->GetLayerSetup() );
            m_targetLayerSetup->SetAvailable( part->GetDrawing()->GetLayerSetup() );
            FillControls();
        }
        else if ( event.GetId() == a2dHabitat::sig_changedLayer )
        {
            m_targetLayerSetup->SetAvailable( part->GetDrawing()->GetLayerSetup() );
            FillControls();
        }
        else if ( event.GetId() == a2dDrawing::sig_layersetupChanged )
        {
            Init( part->GetDrawing()->GetLayerSetup() );
        }
		else if ( event.GetId() == a2dCanvasGlobal::sig_changedActiveDrawing )
		{
			a2dDrawingPart* part = a2dCanvasGlobals->GetActiveDrawingPart();
			if ( part && part->GetShowObject() && part->GetShowObject()->GetRoot() )
			{
				if ( m_targetLayerSetup != part->GetShowObject()->GetRoot()->GetLayerSetup() )
				{
					part->GetShowObject()->GetRoot()->SetAvailable();
				    part->GetDrawing()->DisconnectEvent( a2dEVT_COM_EVENT, this );
					Init( part->GetShowObject()->GetRoot()->GetLayerSetup() );
				    part->GetDrawing()->ConnectEvent( a2dEVT_COM_EVENT, this );

				}
			}
		}
    }
}

void LayerPropertiesDialog::OnDoEvent( a2dCommandProcessorEvent& event )
{
    if ( !GetEvtHandlerEnabled() )
        return;

    if ( !m_activeDrawingPart )
        return;

    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawingPart )
        return;

    a2dLayers* layers = drawingPart->GetDrawing()->GetLayerSetup();
    if ( !layers )
        return;
}

void LayerPropertiesDialog::FillControls()
{
    int All_Hided = true;
    int All_Read = true;
    int All_Outline = true;

    m_lc->DeleteAllItems ();
    wxVector<wxVariant> data;

    a2dLayerIndex::iterator itb = m_targetLayerSetup->GetLayerSort().begin();
    a2dLayerIndex::iterator ite = m_targetLayerSetup->GetLayerSort().end();
    if ( m_onorder )
    {
        itb = m_targetLayerSetup->GetOrderSort().begin();
        ite = m_targetLayerSetup->GetOrderSort().end();
    }
    for ( a2dLayerIndex::iterator it= itb; it != ite; ++it)
    {
        a2dLayerInfo* layerinfo = *it;
        bool available = layerinfo->GetAvailable();

        data.clear();
        data.push_back( wxString::Format( "%03d", layerinfo->GetLayer() ) );
        data.push_back( layerinfo->GetName() );
        data.push_back( available );
        data.push_back( layerinfo->GetVisible() );
        data.push_back( layerinfo->GetSelectable() );
        data.push_back( !layerinfo->GetFill().GetFilling() );
        data.push_back( layerinfo->GetRead() );

        // fill of polygons rect
        wxColour fillCol = layerinfo->GetFillColour();
        wxColour strokeCol = layerinfo->GetStrokeColour();
        wxBitmap fbm = wxBitmap(30,20);
        wxMemoryDC fdc( fbm );
        fdc.SetBrush  ( wxBrush( fillCol ) ); 
        fdc.DrawRectangle(0,0,30,20);
        data.push_back( wxVariant( fbm ) );

        wxBitmap sbm = wxBitmap(30,20);
        wxMemoryDC sdc( sbm );
        sdc.SetBrush  ( wxBrush( strokeCol ) ); 
        sdc.DrawRectangle(0,0,30,20);
        data.push_back( wxVariant( sbm ) );

        {
            int w = 40;
            int h = 20;
            a2dMemDcDrawer* drawer = new a2dMemDcDrawer( w, h );
            drawer->BeginDraw();
            drawer->SetDrawerFill( *a2dWHITE_FILL );
            drawer->DrawRoundedRectangle( 0,0, w, h, 0 );
            drawer->SetDrawerFill( layerinfo->GetFill() );
            drawer->SetDrawerStroke( layerinfo->GetStroke() );
            drawer->DrawRoundedRectangle( 0,0, w, h, 0 );
            drawer->EndDraw();
            data.push_back( wxVariant( drawer->GetBuffer() ) );
            delete drawer;
        }

        data.push_back( wxString::Format( "%i", layerinfo->GetInMapping() ));
        data.push_back( wxString::Format( "%i", layerinfo->GetOutMapping() ));
        data.push_back( wxString::Format( "%i", layerinfo->GetOrder() ) );
        m_lc->AppendItem( data );

        if ( layerinfo->GetVisible() )
            All_Hided = false;

        if ( !layerinfo->GetFill().GetFilling() )
            All_Outline = false;

        if ( layerinfo->GetRead() )
            All_Read = false;
    }

    if ( All_Hided )
    {
        m_buttonVISIBLE->SetLabel( _T( "Show All" ) );
        m_buttonSELECT->SetLabel( _T( "All Selectable" ) );
    }

    if ( m_onorder ) 
        m_buttonSORT->SetLabel( _T( "Sort Layer" ) ); //is on order, so set this opposite
    else
        m_buttonSORT->SetLabel( _T( "Sort Order" ) );

    if ( All_Outline )
        m_buttonOutLine->SetLabel( _T( "Outline All" ) );
    else
        m_buttonOutLine->SetLabel( _T( "Outline Non" ) );

    if ( All_Read )
        m_buttonREAD->SetLabel( _T( "Read All" ) );
}

void LayerPropertiesDialog::CmHide( wxCommandEvent& )
{
    Show( false );

    if ( !m_activeDrawingPart )
        return;

    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawingPart )
        return;

    a2dLayers* layers = drawingPart->GetDrawing()->GetLayerSetup();
    if ( !layers )
        return;
    layers->SetPending( true );
}

void LayerPropertiesDialog::CmCancel( wxCommandEvent& event )
{
    *m_targetLayerSetup = *m_backuplayersetup;

    SetEvtHandlerEnabled( false );
    a2dComEvent changedlayer( m_targetLayerSetup->GetRoot(), m_targetLayerSetup, a2dDrawing::sig_layersetupChanged );
    ProcessEvent( changedlayer );
    SetEvtHandlerEnabled( true );

    FillControls();

    CmHide( event );
}

void LayerPropertiesDialog::CmRestore( wxCommandEvent& WXUNUSED( event ) )
{
    *m_targetLayerSetup = *m_backuplayersetup;

    SetEvtHandlerEnabled( false );
    a2dComEvent changedlayer( m_targetLayerSetup->GetRoot(), m_targetLayerSetup, a2dDrawing::sig_layersetupChanged );
    ProcessEvent( changedlayer );
    SetEvtHandlerEnabled( true );

    FillControls();

    if ( !m_activeDrawingPart )
        return;

    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawingPart )
        return;

    a2dLayers* layers = drawingPart->GetDrawing()->GetLayerSetup();
    if ( !layers )
        return;

    SetEvtHandlerEnabled( false ); //prevent OnComEvent on this one.
    drawingPart->GetDrawing()->SetLayerSetup( m_targetLayerSetup );
    SetEvtHandlerEnabled( true );
}

void LayerPropertiesDialog::CmSaveToFile( wxCommandEvent& WXUNUSED( event ) )
{
    wxString fullname = _( "Mywxart2dlayers.cvg" );

    wxString* layerfiledir = a2dGeneralGlobals->GetVariablesHash().GetVariableString( wxT( "lastLayerFileSavePath" ) );

    if ( !layerfiledir )
        layerfiledir = a2dGeneralGlobals->GetVariablesHash().GetVariableString(  wxT( "layerFileSavePath" ) );

    wxFileDialog fileDialog( this,
                             _( "Choose layer file to load" ),
                             layerfiledir ? *layerfiledir : wxT( "" ),
                             fullname,
                             wxT( "*.cvg" ),
                             wxFD_SAVE | wxFD_OVERWRITE_PROMPT
                           );

    if ( fileDialog.ShowModal() != wxID_OK )
        return;

    fullname = fileDialog.GetPath();

    a2dGeneralGlobals->GetVariablesHash().SetVariableString( wxT( "lastLayerFileSavePath" ), wxPathOnly( fullname ) );

    SetEvtHandlerEnabled( false ); //prevent OnComEvent on this one.

    SetCursor( wxCURSOR_WAIT );
    m_targetLayerSetup->SaveLayers( fullname );

    SetEvtHandlerEnabled( true );
    SetCursor( wxCURSOR_ARROW );
}

void LayerPropertiesDialog::CmLoadFromFile( wxCommandEvent& WXUNUSED( event ) )
{
    wxString fullname;

    wxString* path = a2dGeneralGlobals->GetVariablesHash().GetVariableString( wxT( "lastLayerFileSavePath" ) );

    wxFileDialog fileDialog( this,
                             _( "Choose layer file to load" ),
                             path ? *path : wxT( "" ),
                             fullname,
                             wxT( "*.cvg" ),
                             wxFD_OPEN | wxFD_FILE_MUST_EXIST
                           );

    if ( fileDialog.ShowModal() != wxID_OK )
        return;

    fullname = fileDialog.GetPath();

    a2dGeneralGlobals->GetVariablesHash().SetVariableString( wxT( "lastLayerFileSavePath" ), wxPathOnly( fullname ) );

    SetCursor( wxCURSOR_WAIT );
#if wxART2D_USE_CVGIO

    a2dLayers* layersetup = new a2dLayers();
    layersetup->LoadLayers( fullname );

    if ( !m_activeDrawingPart )
    {
        a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawingPart )
            return;

        a2dLayers* layers = drawingPart->GetDrawing()->GetLayerSetup();
        if ( !layers )
            return;
        drawingPart->GetDrawing()->SetLayerSetup( layersetup );
    }
#else
    wxFAIL_MSG( wxT( "wxART2D_USE_CVGIO is needed to load object from a file CVG file" ) );
#endif //wxART2D_USE_CVGIO


    SetEvtHandlerEnabled( false ); //prevent OnComEvent on this one.
    Init( layersetup );
    SetEvtHandlerEnabled( true ); //prevent OnComEvent on this one.
    SetCursor( wxCURSOR_ARROW );
}

void LayerPropertiesDialog::CmAddLayer( wxCommandEvent& WXUNUSED( event ) )
{
    wxString layerName = wxGetTextFromUser( "give name for new layer:", "Add Layer", "" );
    if ( !layerName.IsEmpty() )
    {
        m_targetLayerSetup->AddIfMissing( layerName );
    }

    FillControls();
}

void LayerPropertiesDialog::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    Show( false );

    if ( !m_activeDrawingPart )
        return;

    a2dDrawingPart* drawingPart = a2dCanvasGlobals->GetActiveDrawingPart();
    if ( !drawingPart )
        return;

    if ( drawingPart->GetDrawing()->GetLayerSetup() )
        drawingPart->GetDrawing()->GetLayerSetup()->SetPending( true );
}

void LayerPropertiesDialog::CmSort( wxCommandEvent& )
{
    wxString buf;
    buf = m_buttonSORT->GetLabel();
    //set opposite
    if ( buf == wxT( "Sort Layer" ) ) //is currenly onorder?
        m_onorder = false;
    else
        m_onorder = true;

    FillControls();
}

void LayerPropertiesDialog::CmVisible( wxCommandEvent& )
{
    wxString buf;
    bool onoff = false;
    buf = m_buttonVISIBLE->GetLabel();
    if ( buf == wxT( "Hide All" ) )
    {
        onoff = false;
        m_buttonVISIBLE->SetLabel( wxT( "Show All" ) );
    }
    else
    {
        onoff = true;
        m_buttonVISIBLE->SetLabel( wxT( "Hide All" ) );
    }

	m_targetLayerSetup->SetAll_Layers_Visible( onoff );
    FillControls();
}

void LayerPropertiesDialog::CmRead( wxCommandEvent& )
{
    wxString buf;
    bool onoff = false;
    buf = m_buttonREAD->GetLabel();
    if ( buf == wxT( "Read Non" ) )
    {
        onoff = false;
        m_buttonREAD->SetLabel( _T( "Read All" ) );
    }
    else
    {
        onoff = true;
        m_buttonREAD->SetLabel( wxT( "Read Non" ) );
    }

    m_targetLayerSetup->SetAll_Layers_Read( onoff );
    FillControls();
}

void LayerPropertiesDialog::CmOutLine( wxCommandEvent& )
{
    wxString buf;
    bool onoff = false;
    buf = m_buttonOutLine->GetLabel();
    if ( buf == wxT( "Outline Non" ) )
    {
        onoff = false;
        m_buttonOutLine->SetLabel( _T( "Outline All" ) );
    }
    else
    {
        onoff = true;
        m_buttonOutLine->SetLabel( wxT( "Outline Non" ) );
    }

    m_targetLayerSetup->SetAll_Layers_Outline( onoff );
    FillControls();
}

void LayerPropertiesDialog::CmSelect( wxCommandEvent& )
{
    wxString buf;
    bool onoff = false;
    buf = m_buttonSELECT->GetLabel();
    if ( buf == wxT( "All Selectable" ) )
    {
        onoff = true;
        m_buttonSELECT->SetLabel( _T( "None Selectable" ) );
    }
    else
    {
        onoff = false;
        m_buttonSELECT->SetLabel( _T( "All Selectable" ) );
    }

    m_targetLayerSetup->SetAll_Layers_Selectable( onoff );
    FillControls();
}

void LayerPropertiesDialog::Cm_changed( wxDataViewEvent& event )
{
    wxDataViewItem item = event.GetItem();
    int column  = event.GetColumn();
    m_lc->Select( item );


    if ( column == 0 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );
        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );
        layerinfo->SetLayer( layer );

        wxVariant val;
        m_lc->GetValue(val,  row, column ) ;
        //m_lc->SetValue  ( wxVariant( wxString::Format( "%03d", val.GetInteger() ) ), row, column ) ;
       	layerinfo->SignalChange();
    }
    else if ( column == 1 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );
        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        wxVariant val;
        m_lc->GetValue(val,  row, column ) ;
        layerinfo->SetName( val.GetString() );
       	layerinfo->SignalChange();
    }
    else if ( column == 2 )
    {
    }
    else if ( column == 3 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );
        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        wxVariant val;
        m_lc->GetValue(val,  row, column ) ;
        layerinfo->SetVisible( val.GetBool() );
       	layerinfo->SignalChange();
    }
    else if ( column == 4 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );
        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        wxVariant val;
        m_lc->GetValue(val,  row, column ) ;
        layerinfo->SetSelectable( val.GetBool() );
       	layerinfo->SignalChange();
    }
    else if ( column == 5 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );
        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        wxVariant val;
        m_lc->GetValue(val, row, column ) ;
        a2dFill fill = layerinfo->GetFill();
        fill.SetFilling( !val.GetBool() );
        layerinfo->SetFill( fill );
       	layerinfo->SignalChange();
    }
    else if ( column == 6 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );
        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        wxVariant val;
        m_lc->GetValue(val,  row, column ) ;
        layerinfo->SetRead( val.GetBool() );
       	layerinfo->SignalChange();
    }
    else if ( column == 10 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );

        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        nrstr = m_lc->GetTextValue( row, column );
        long in = 0;
        nrstr.ToLong( &in );
        layerinfo->SetInMapping( in );
       	layerinfo->SignalChange();
    }
    else if ( column == 11 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );

        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        nrstr = m_lc->GetTextValue( row, column );
        long out = 0;
        nrstr.ToLong( &out );
        layerinfo->SetOutMapping( out );
       	layerinfo->SignalChange();
    }
    else if ( column == 12 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );

        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        nrstr = m_lc->GetTextValue( row, column );
        long order = 0;
        nrstr.ToLong( &order );
        layerinfo->SetOrder( order );
       	layerinfo->SignalChange();

        FillControls();
    }

    SetFocus();
    Show( true );
    Refresh();
}

void LayerPropertiesDialog::Cm_edit( wxDataViewEvent& event )
{
    wxDataViewItem item = event.GetItem();
    int column = event.GetColumn();
    column = m_lc->GetCurrentColumn  (  )->GetModelColumn  (  )  ;

    if ( column == 0 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );
        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );
        layerinfo->SetLayer( layer );

        wxVariant val;
        m_lc->GetValue(val,  row, column ) ;
        //m_lc->SetValue  ( wxVariant( wxString::Format( "%03d", val.GetInteger() ) ), row, column ) ;
       	layerinfo->SignalChange();
    }
    else if ( column == 1 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );

        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        wxVariant val;
        m_lc->GetValue(val,  row, column ) ;
        layerinfo->SetName( val.GetString() );
    }
    else if ( column == 9 )
    {
        PatternDialog patternDialog( this, m_fillpatterns, wxT( "Patterns" ), ( wxSYSTEM_MENU | wxCAPTION ) );
        int choosen = patternDialog.ShowModal();
        patternDialog.Close();

        wxBitmap pat = *( m_fillpatterns->GetPattern( choosen ) );

        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );
        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );
        a2dFill fill;

        // if pattern is not full or non, use pattern fill
        if ( choosen != 0 && choosen != 1 )
        {
            a2dFillStyle style = layerinfo->GetFill().GetStyle();
            fill = a2dFill( pat );

            if ( style != a2dFILL_STIPPLE_MASK_OPAQUE &&
                    style != a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT &&
                    style != a2dFILL_STIPPLE )
                //the default is transparent
                fill.SetStyle( a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT );
            else if ( style == a2dFILL_STIPPLE )
                fill.SetStyle( a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT );
            else
                fill.SetStyle( style );
            fill.SetColour( layerinfo->GetFill().GetColour() );
            fill.SetColour2( layerinfo->GetFill().GetColour2() );
            layerinfo->SetFill( fill );
        }
        else
        {
            if ( choosen == 0 )
            {
                fill = a2dFill( layerinfo->GetFill().GetColour(), a2dFILL_TRANSPARENT );
                layerinfo->SetFill( fill );
            }
            else
            {
                fill = a2dFill( layerinfo->GetFill().GetColour(), a2dFILL_SOLID );
                layerinfo->SetFill( fill );
            }
        }

        int w = 40;
        int h = 20;
        a2dMemDcDrawer* drawer = new a2dMemDcDrawer( w, h );
        drawer->BeginDraw();
        drawer->SetDrawerFill( *a2dWHITE_FILL );
        drawer->DrawRoundedRectangle( 0,0, w, h, 0 );
        drawer->SetDrawerFill( layerinfo->GetFill() );
        drawer->SetDrawerStroke( layerinfo->GetStroke() );
        drawer->DrawRoundedRectangle( 0,0, w, h, 0 );
        drawer->EndDraw();
        m_lc->SetValue  ( wxVariant( drawer->GetBuffer() ), row, column ) ;
        delete drawer;

    	layerinfo->SignalChange();
    }
    else if ( column == 8 )
    {
        if ( m_cd->ShowModal() == wxID_OK )
        {
            wxColour col;
            *m_cdata = m_cd->GetColourData();
            col = m_cdata->GetColour();

            int row = m_lc->GetSelectedRow();
            wxString nrstr = m_lc->GetTextValue( row, 0 );
            long layer = 0;
            nrstr.ToLong( &layer );

            a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );
            if ( col != layerinfo->GetStroke().GetColour() )
            {
	            layerinfo->SetStrokeColour( col );
			    layerinfo->SignalChange();
            }

            // fill of polygons rect
            wxColour strokeCol = layerinfo->GetStrokeColour();
            wxBitmap sbm = wxBitmap(30,20);
            wxMemoryDC sdc( sbm );
            sdc.SetBrush  ( wxBrush( strokeCol ) ); 
            sdc.DrawRectangle(0,0,30,20);
            m_lc->SetValue  (  wxVariant( sbm ) , row, column ) ;      
        }
    }
    else if ( column == 7 )
    {
        if ( m_cd->ShowModal() == wxID_OK )
        {
            wxColour col;
            *m_cdata = m_cd->GetColourData();
            col = m_cdata->GetColour();

            int row = m_lc->GetSelectedRow();
            wxString nrstr = m_lc->GetTextValue( row, 0 );
            long layer = 0;
            nrstr.ToLong( &layer );
            a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );
            if ( col != layerinfo->GetFill().GetColour() )
            {
	            layerinfo->SetFillColour( col );
			    layerinfo->SignalChange();
            }

            // fill of polygons rect
            wxColour fillCol = layerinfo->GetFillColour();
            wxBitmap fbm = wxBitmap(30,20);
            wxMemoryDC fdc( fbm );
            fdc.SetBrush  ( wxBrush( fillCol ) ); 
            fdc.DrawRectangle(0,0,30,20);
            m_lc->SetValue  (  wxVariant( fbm ) , row, column ) ;

            int w = 40;
            int h = 20;
            a2dMemDcDrawer* drawer = new a2dMemDcDrawer( w, h );
            drawer->BeginDraw();
            drawer->SetDrawerFill( *a2dWHITE_FILL );
            drawer->DrawRoundedRectangle( 0,0, w, h, 0 );
            drawer->SetDrawerFill( layerinfo->GetFill() );
            drawer->SetDrawerStroke( layerinfo->GetStroke() );
            drawer->DrawRoundedRectangle( 0,0, w, h, 0 );
            drawer->EndDraw();
            m_lc->SetValue  ( wxVariant( drawer->GetBuffer() ), row, 9 ) ;
            delete drawer;
        }
    }
    else if ( column == 10 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );

        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        nrstr = m_lc->GetTextValue( row, column );
        long in = 0;
        nrstr.ToLong( &in );
        layerinfo->SetInMapping( in );
       	layerinfo->SignalChange();
    }
    else if ( column == 11 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );

        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        nrstr = m_lc->GetTextValue( row, column );
        long out = 0;
        nrstr.ToLong( &out );
        layerinfo->SetOutMapping( out );
       	layerinfo->SignalChange();
    }
    else if ( column == 12 )
    {
        int row = m_lc->GetSelectedRow();
        wxString nrstr = m_lc->GetTextValue( row, 0 );
        long layer = 0;
        nrstr.ToLong( &layer );

        a2dLayerInfo* layerinfo = GetTargetLayerInfo( layer );

        nrstr = m_lc->GetTextValue( row, column );
        long order = 0;
        nrstr.ToLong( &order );
        layerinfo->SetOrder( order );
       	layerinfo->SignalChange();
    }

    SetFocus();
    Show( true );
    Refresh();
}

void LayerPropertiesDialog::Cm_SortColumn( wxDataViewEvent& event )
{
    wxDataViewItem item = event.GetItem();
    int column = event.GetColumn();

    if ( column == 0 )
    {
        m_onorder = false;
        FillControls();
    }
    else if ( column == 1 )
    {
    }
    else if ( column == 12 )
    {
        m_onorder = true;
        FillControls();
    }

    SetFocus();
    Show( true );
    Refresh();
}