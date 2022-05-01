/*! \file curves/samples/curtuto1/curtuto1.cpp
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: curtuto2.cpp,v 1.10 2009/07/24 16:35:20 titato Exp $
*/

#include "a2dprec.h"
#include "wx/editor/candocproc.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Include private headers
#include "curtuto1.h"

// Include icon header
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "mondrian.xpm"
#endif

#include "wx/docview/docviewref.h"

#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#endif //wxART2D_USE_AGGDRAWER

static double gc_defmaxX = 480; // window default max X
static double gc_defmaxY = 100; // window default max Y
static int gc_defScreenmaxX = 960; // screen window default max X
static int gc_defScreenmaxY = 500; // screen window default max Y
static const double gc_plotBoundLeftK = 0.01;
static const double gc_plotBoundRightK = 0.01;
static const double gc_plotBoundUpK = 0.01;
static const double gc_plotBoundDownK = 0.01;

//#if wxUSE_PRINTING_ARCHITECTURE
//static wxPageSetupData* g_pageSetupData = NULL;
//#endif

void MyFrame::FillDocument0( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();

    a2dText* tempty = new a2dText( wxT( "ANGLED TTF" ), 350, 150,
                                   a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 14.0, wxFONTENCODING_DEFAULT  ),
                                   270.0, false, wxMINY | wxMINX );

    tempty->SetFill( *a2dTRANSPARENT_FILL ); //a2dFill(wxColour(133,133,250)));
    tempty->SetEngineeringText();
    tempty->SetStroke( wxColour( 127, 127, 0 ) );
    root->Append( tempty );
    m_canvas->GetDrawingPart()->SetViewDependentObjects( true );
    tempty->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);

    /*
        tempty = new a2dText(wxT("ANGLED DC"), 450,-150,
            a2dFont(14.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ),
            -40.0, false, wxMINY | wxMINX);

        tempty->SetFill(*a2dTRANSPARENT_FILL);//a2dFill(wxColour(133,133,250)));
        tempty->SetStroke(wxColour(127,127,0));
        root->Append( tempty );
        tempty->SetViewDependent(m_canvas->GetDrawingPart(),true);//,true);
    */

    m_plotmousemenu = new wxMenu( _( "PLOT" ), ( long )0 );


    a2dPlot* plot = new a2dPlot( 100, 150 );
    plot->SetDraggable( false );
    a2dCanvasObject::PROPID_PopupMenu->SetPropertyToObject( plot, m_plotmousemenu );

    root->Append( plot );

//  plot->SetBoundaries(-50,-50,200,200);
    plot->SetBoundaries( 0, 0, 200, 200 );
    plot->SetInternalBoundaries( 200, -100, -100, 200 ); // klion: inverted x axis
    plot->SetInternalBoundaries( -100, -100, 200, 200 ); // klion: non-inverted x axis
    plot->GetAxisY()->SetTic( 20 );
    plot->SetFill( *a2dTRANSPARENT_FILL );
    plot->SetFill(  wxColour( 66, 247, 247 ), a2dFILL_SOLID );
    plot->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 0 ) );
    plot->SetAxisAreaFill( a2dFill( wxColour( 233, 250, 250 ) ) );
    plot->GetAxisX()->SetStroke( a2dStroke( wxColour( 239, 100, 0 ), 2, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisY()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 1 ) );
    plot->GetAxisX()->SetPosition( -100 );
    plot->GetAxisX()->SetTicHeight ( 6 );
    plot->GetAxisX()->SetTicTextHeight ( 10 );
    plot->GetAxisX()->SetAutoSizedTic( true );
    plot->GetAxisX()->SetViewDependent( m_canvas->GetDrawingPart(), true, false, true );
    plot->GetAxisY()->SetPosition( -100 );  // klion: inverted x-scale
    plot->GetAxisY()->SetPosition( 200 );  // klion: non-inverted x-scale
    plot->GetAxisY()->SetInvertTic( true );
    plot->GetAxisY()->SetTicHeight ( 6 );
    plot->GetAxisY()->SetTicTextHeight ( 10 );
    plot->GetAxisY()->SetTicFormat( _T( "y=%4.1f" ) );
    plot->GetAxisY()->SetAutoSizedTic( true );
    plot->GetAxisY()->SetViewDependent( m_canvas->GetDrawingPart(), true, false, true );

    a2dAxisMarker* aLevel1 = new a2dAxisMarker( wxT( "" ), plot->GetAxisX() );
    aLevel1->Set( 10, 10, 8 );
    aLevel1->Rotate( 180 );
    aLevel1->SetStroke( a2dStroke( wxColour( wxT( "RED" ) ), 1 ) );
    aLevel1->SetFill( a2dFill( wxColour( wxT( "YELLOW" ) ) ) );

    aLevel1->SetPromptFormat( wxT( "%.2f" ) );
    a2dText* aLevelPrompt1 = new a2dText( wxT( "" ), 0, 0,
                                          a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 18.0, wxFONTENCODING_DEFAULT  ),
                                          0.0, false, wxMINY | wxMINX );
    aLevelPrompt1->Rotate( -180 );
    aLevelPrompt1->SetStroke( a2dStroke( wxColour( wxT( "RED" ) ), 1 ) );
    aLevelPrompt1->SetViewDependent( m_canvas->GetDrawingPart(), true );

    aLevel1->SetViewDependent( m_canvas->GetDrawingPart(), true );
    aLevel1->SetPrompt( aLevelPrompt1 );
    plot->GetAxisX()->AddLevel( wxT( "maxx" ), 10, aLevel1 );

    a2dAxisMarker* aLevel2 = new a2dAxisMarker( wxT( "" ), plot->GetAxisY() );
    aLevel2->Set( 10, 10, 8 );
    aLevel2->Rotate( -90 );
    aLevel2->SetStroke( a2dStroke( wxColour( wxT( "RED" ) ), 1 ) );

    aLevel2->SetFill( a2dFill( wxColour( wxT( "YELLOW" ) ) ) );

    aLevel2->SetPromptFormat( wxT( "%.2f" ) );
    a2dText* aLevelPrompt2 = new a2dText( wxT( "" ), 0, 0,
                                          a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 18.0, wxFONTENCODING_DEFAULT  ),
                                          0.0, false, wxMINY | wxMINX );
    aLevelPrompt2->Rotate( 90 );
    aLevelPrompt2->SetStroke( a2dStroke( wxColour( wxT( "YELLOW" ) ), 1 ) );
    aLevelPrompt2->SetViewDependent( m_canvas->GetDrawingPart(), true );

    aLevel2->SetViewDependent( m_canvas->GetDrawingPart(), true );
    aLevel2->SetPrompt( aLevelPrompt2 );
    plot->GetAxisY()->AddLevel( wxT( "maxy" ), 20, aLevel2 );

    //plot->SetShowTics(false);
    //plot->SetSideTic(false);
    //plot->SetTicFormat("%4.1f") );
    //plot->SetTicFormat("%4.1f") );
    plot->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot->GetAxisY()->SetTicStroke( a2dStroke( wxColour( 239, 122, 57 ), 3 ) );
    plot->GetAxisY()->SetTicTextStroke( a2dStroke( wxColour( 9, 112, 134 ), 2 ) );

    plot->SetTitle( wxString( _T( "Simple Plot doc0" ) ) );
//  plot->SetXLabel(wxString(_T("time")));
//  plot->SetYLabel(wxString(_T("voltage")));

    a2dText* tx = new a2dText(  _T( "time" ), 0, 0,
                                a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    tx->SetFill(  *wxBLACK, a2dFILL_SOLID );
    tx->SetStroke( wxColour( 255, 255, 255 ) );
    plot->SetXLabelTextDc( tx );
    tx->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);
    a2dText* ty = new a2dText(  _T( "voltage" ), 0, 0,
                                a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    ty->SetFill(  *wxBLACK, a2dFILL_SOLID );
    ty->SetStroke( wxColour( 255, 255, 255 ) );
    plot->SetYLabelTextDc( ty );
    ty->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);


    a2dText* ttt = new a2dText(  _T( "Two LineTitle" ), 0, 0,
                                 a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    ttt->SetFill(  *wxRED, a2dFILL_SOLID );
    ttt->SetStroke( wxColour( 0, 10, 0 ) );
    plot->SetTitleTextDc( ttt );
    ttt->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);

    /*
        a2dText* ct= new a2dText(  _T("Hello Freetype Text does transform properly") , 0,100,
                          "./fonts/LiberationSans-Bold.ttf") , 13 );
        ct->Scale( 0.3, 1);
        ct->Rotate(34);
        plot->Append( ct );
    */
    a2dText* tt = new a2dText(  _T( "Normal Text does\n  not transform" ), 20, 30,
                                a2dFont( 8.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    tt->SetFill( *a2dTRANSPARENT_FILL );
    tt->SetStroke( wxColour( 220, 10, 200 ) );
    root->Append( tt );
    //  tt->SetViewDependent(m_canvas->GetDrawingPart(),true);//,true);

    a2dText* tt1 = new a2dText( wxT( "Latin text! \xd0\xf3\xf1\xf1\xea\xe8\xe9 \xf2\xe5\xea\xf1\xf2" ), 10, 50, //Русский текст
                                a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 8.0, wxFONTENCODING_DEFAULT  ) );

    tt1->SetFill( *a2dTRANSPARENT_FILL );
    tt1->SetStroke( wxColour( 255, 0, 0 ) );
    root->Append( tt1 );
    tt1->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);

    a2dVertexCurve* curve1 = new a2dVertexCurve();
    curve1->SetStroke( a2dStroke( wxColour( 255, 127, 0 ), 0, a2dSTROKE_DOT_DASH ) );
    curve1->AddPoint( 70, 140 );
    curve1->AddPoint( 80, 130 );
    curve1->AddPoint( 90, 120 );
    curve1->AddPoint( 100, 110 );
    curve1->AddPoint( 110, 110 );
    curve1->AddPoint( 150, 140 );
    curve1->AddPoint( 160, 150 );
    curve1->AddPoint( 170, 160 );
    curve1->AddPoint( 210, 210 );
    curve1->SetRenderMode( a2dCurve::a2dCURVES_RENDER_NORMAL | a2dCurve::a2dCURVES_RENDER_NO_POINTS );
    curve1->SetEditMode( a2dCurve::a2dCURVES_EDIT_ALLOW_ADD | a2dCurve::a2dCURVES_EDIT_FIXED_Y );
    a2dCanvasObject::PROPID_PopupMenu->SetPropertyToObject( curve1, m_plotmousemenu );

    a2dText* label = new a2dText(  _T( "curve 1" ), 100, 114, //a2dFont( 8.0, wxFONTFAMILY_SWISS ) );
                                   a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 8.0, wxFONTENCODING_DEFAULT  ) );
    curve1->Append( label );
    plot->AddCurve( curve1, _T( "curve1" ) );

    a2dMarker* markOnCurve1 = new a2dMarker(  _T( "M1 %5.2f %5.2f" ), curve1, 150, 10 );
    markOnCurve1->SetStroke( wxColour( 255, 0, 0 ) );
    plot->AddMarker( markOnCurve1 );
    markOnCurve1->SetViewDependent( m_canvas->GetDrawingPart(), true, false, false, true );

    a2dVertexCurve* curveempty = new a2dVertexCurve();
    curveempty->SetStroke( a2dStroke( wxColour( 255, 127, 0 ), 0, a2dSTROKE_DOT_DASH ) );
    curveempty->SetRenderMode( a2dCurve::a2dCURVES_RENDER_NORMAL | a2dCurve::a2dCURVES_RENDER_NO_POINTS );
    plot->AddCurve( curveempty, _T( "curveempty" ) );
    a2dMarker* markOnCurveEmpty = new a2dMarker(  _T( "ME %5.2f %5.2f" ), curveempty, 150, 10 );
    markOnCurveEmpty->SetStroke( wxColour( 255, 0, 255 ) );
    plot->AddMarker( markOnCurveEmpty );
    markOnCurveEmpty->SetViewDependent( m_canvas->GetDrawingPart(), true, false, false, true );

    a2dMarkerShow* showm = new a2dMarkerShow( 10, 10, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.2f" ), _T( "%5.2f" ), markOnCurve1, 10 );
    plot->SetMarkerShow( showm );
    showm->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);

    a2dMarker* markOnCurve2 = new a2dMarker(  _T( "M2 %5.2f %5.2f" ), curve1, 40 );
    markOnCurve2->SetStroke( wxColour( 25, 70, 0 ) );
    markOnCurve2->SetFill( wxColour( 0, 255, 155 ) );
    plot->AddMarker( markOnCurve2 );

    a2dMarkerShow* showm2 = new a2dMarkerShow( 340, 50, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.1f" ), _T( "%5.1f" ), markOnCurve2, 10 );
    showm2->SetStroke( wxColour( 25, 70, 0 ) );
    showm2->SetFill( wxColour( 0, 255, 155 ) );
    root->Append( showm2 );

    a2dVertexCurve* curve2 = new a2dVertexCurve();
    curve2->SetEditMode( a2dCurve::a2dCURVES_EDIT_FIXED_X );
    curve2->SetStroke( a2dStroke( wxColour( 255, 1, 0 ), 2, a2dSTROKE_DOT ) );
    curve2->AddPoint( -70, -50 );
    curve2->AddPoint( -50, -10 );
    curve2->AddPoint( -30, 50 );
    curve2->AddPoint( 70, 100 );
    curve2->AddPoint( 80, 150 );
    curve2->AddPoint( 90, 130 );
    curve2->AddPoint( 100, 120 );
    curve2->AddPoint( 110, 130 );
    curve2->AddPoint( 150, 150 );
    curve2->AddPoint( 160, 160 );
    curve2->AddPoint( 170, 170 );
    curve2->AddPoint( 210, 180 );
    //curve2->SetObjectTip(  _T("curve 2") , 85, 155, 8 );
    plot->AddCurve( curve2, _T( "curve2" ) );

    a2dMarker* markOnCurve3 = new a2dMarker(  _T( "MX %5.2f %5.2f" ), curve2, 120 );
    markOnCurve3->SetStroke( wxColour( 255, 0, 0 ) );
    markOnCurve3->SetFill( wxColour( 0, 255, 0 ) );
    plot->AddMarker( markOnCurve3 );

    a2dMarkerShow* showmmarkOnCurve3 = new a2dMarkerShow( 240, 50, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.1f" ), _T( "%5.1f" ), markOnCurve3, 10 );
    showmmarkOnCurve3->SetStroke( wxColour( 255, 0, 0 ) );
    showmmarkOnCurve3->SetFill( wxColour( 0, 255, 0 ) );
    root->Append( showmmarkOnCurve3 );

    a2dVertexCurve* curveget = ( a2dVertexCurve* ) plot->GetCurve(  _T( "curve1" ) );
    curveget->AddPoint( 215, 196 );

    //plot->Rotate(-35);

#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
    m_contr->SetDefaultBehavior( wxTC_NoContextMenu );

    //m_contr->Zoomout();
#endif //wxART2D_USE_EDITOR

    if( m_timer )
        delete m_timer;
    m_timer = new wxTimer( this );
}

void MyFrame::FillDocument8( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();
    m_canvas->GetDrawingPart()->SetViewDependentObjects( true );

    a2dText* tempty = new a2dText( wxT( "ANGLED TTF" ), 350, 150,
                                   a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 14.0, wxFONTENCODING_DEFAULT  ),
                                   270.0, false, wxMINY | wxMINX );

    tempty->SetFill( *a2dTRANSPARENT_FILL ); //a2dFill(wxColour(133,133,250)));
    tempty->SetEngineeringText();
    tempty->SetStroke( wxColour( 127, 127, 0 ) );
    root->Append( tempty );
    tempty->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);

    m_plotmousemenu = new wxMenu( _( "PLOT" ), ( long )0 );

    a2dPlot* plot = new a2dPlot( 100, 150 );
    plot->SetDraggable( false );
    a2dCanvasObject::PROPID_PopupMenu->SetPropertyToObject( plot, m_plotmousemenu );

    root->Append( plot );

    //  plot->SetBoundaries(-50,-50,200,200);
    plot->SetBoundaries( 0, 0, 200, 200 );
    plot->SetInternalBoundaries( 200, -100, -100, 200 ); // klion: inverted x axis
    plot->SetInternalBoundaries( -100, -100, 200, 200 ); // klion: non-inverted x axis
    plot->GetAxisY()->SetTic( 20 );
    plot->SetFill( *a2dTRANSPARENT_FILL );
    plot->SetFill(  wxColour( 66, 247, 247 ), a2dFILL_SOLID );
    plot->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 0 ) );
    plot->SetAxisAreaFill( a2dFill( wxColour( 233, 250, 250 ) ) );
    plot->GetAxisX()->SetStroke( a2dStroke( wxColour( 239, 100, 0 ), 2, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisY()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 1 ) );
    plot->GetAxisX()->SetPosition( -100 );
    plot->GetAxisX()->SetTicHeight ( 6 );
    plot->GetAxisX()->SetTicTextHeight ( 10 );
    plot->GetAxisX()->SetAutoSizedTic( true );
    plot->GetAxisX()->SetViewDependent( m_canvas->GetDrawingPart(), true, false, true );
    plot->GetAxisY()->SetPosition( -100 );  // klion: inverted x-scale
    plot->GetAxisY()->SetPosition( 200 );  // klion: non-inverted x-scale
    plot->GetAxisY()->SetInvertTic( true );
    plot->GetAxisY()->SetTicHeight ( 6 );
    plot->GetAxisY()->SetTicTextHeight ( 10 );
    plot->GetAxisY()->SetTicFormat( _T( "y=%4.1f" ) );
    plot->GetAxisY()->SetAutoSizedTic( true );
    plot->GetAxisY()->SetViewDependent( m_canvas->GetDrawingPart(), true, false, true );

    a2dAxisMarker* aLevel1 = new a2dAxisMarker( wxT( "" ), plot->GetAxisX() );
    aLevel1->Set( 10, 10, 8 );
    aLevel1->Rotate( 180 );
    aLevel1->SetStroke( a2dStroke( wxColour( wxT( "RED" ) ), 1 ) );
    aLevel1->SetFill( a2dFill( wxColour( wxT( "YELLOW" ) ) ) );

    aLevel1->SetPromptFormat( wxT( "%.2f" ) );
    a2dText* aLevelPrompt1 = new a2dText( wxT( "" ), 0, 0,
                                          a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 18.0, wxFONTENCODING_DEFAULT  ),
                                          0.0, false, wxMINY | wxMINX );
    aLevelPrompt1->Rotate( -180 );
    aLevelPrompt1->SetStroke( a2dStroke( wxColour( wxT( "RED" ) ), 1 ) );
    aLevelPrompt1->SetViewDependent( m_canvas->GetDrawingPart(), true );

    aLevel1->SetViewDependent( m_canvas->GetDrawingPart(), true );
    aLevel1->SetPrompt( aLevelPrompt1 );
    plot->GetAxisX()->AddLevel( wxT( "maxx" ), 10, aLevel1 );

    a2dAxisMarker* aLevel2 = new a2dAxisMarker( wxT( "" ), plot->GetAxisY() );
    aLevel2->Set( 10, 10, 8 );
    aLevel2->Rotate( -90 );
    aLevel2->SetStroke( a2dStroke( wxColour( wxT( "RED" ) ), 1 ) );

    aLevel2->SetFill( a2dFill( wxColour( wxT( "YELLOW" ) ) ) );

    aLevel2->SetPromptFormat( wxT( "%.2f" ) );
    a2dText* aLevelPrompt2 = new a2dText( wxT( "" ), 0, 0,
                                          a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 18.0, wxFONTENCODING_DEFAULT  ),
                                          0.0, false, wxMINY | wxMINX );
    aLevelPrompt2->Rotate( 90 );
    aLevelPrompt2->SetStroke( a2dStroke( wxColour( wxT( "YELLOW" ) ), 1 ) );
    aLevelPrompt2->SetViewDependent( m_canvas->GetDrawingPart(), true );

    aLevel2->SetViewDependent( m_canvas->GetDrawingPart(), true );
    aLevel2->SetPrompt( aLevelPrompt2 );
    plot->GetAxisY()->AddLevel( wxT( "maxy" ), 20, aLevel2 );

    //plot->SetShowTics(false);
    //plot->SetSideTic(false);
    //plot->SetTicFormat("%4.1f") );
    //plot->SetTicFormat("%4.1f") );
    plot->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot->GetAxisY()->SetTicStroke( a2dStroke( wxColour( 239, 122, 57 ), 3 ) );
    plot->GetAxisY()->SetTicTextStroke( a2dStroke( wxColour( 9, 112, 134 ), 2 ) );

    plot->SetTitle( wxString( _T( "Simple Plot doc0" ) ) );
    //  plot->SetXLabel(wxString(_T("time")));
    //  plot->SetYLabel(wxString(_T("voltage")));

    a2dText* tx = new a2dText(  _T( "time" ), 0, 0,
                                a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    tx->SetFill(  *wxBLACK, a2dFILL_SOLID );
    tx->SetStroke( wxColour( 255, 255, 255 ) );
    plot->SetXLabelTextDc( tx );
    tx->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);
    a2dText* ty = new a2dText(  _T( "voltage" ), 0, 0,
                                a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    ty->SetFill(  *wxBLACK, a2dFILL_SOLID );
    ty->SetStroke( wxColour( 255, 255, 255 ) );
    plot->SetYLabelTextDc( ty );
    ty->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);


    a2dText* ttt = new a2dText(  _T( "Two LineTitle" ), 0, 0,
                                 a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    ttt->SetFill(  *wxRED, a2dFILL_SOLID );
    ttt->SetStroke( wxColour( 0, 10, 0 ) );
    plot->SetTitleTextDc( ttt );
    ttt->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);

    a2dText* tt = new a2dText(  _T( "Normal Text does\n  not transform" ), 20, 30,
                                a2dFont( 8.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    tt->SetFill( *a2dTRANSPARENT_FILL );
    tt->SetStroke( wxColour( 220, 10, 200 ) );
    root->Append( tt );
    tt->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);

    a2dText* tt1 = new a2dText( wxT( "Latin text! \xd0\xf3\xf1\xf1\xea\xe8\xe9 \xf2\xe5\xea\xf1\xf2" ), 10, 50, //Русский текст
                                a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 8.0, wxFONTENCODING_DEFAULT  ) );

    tt1->SetFill( *a2dTRANSPARENT_FILL );
    tt1->SetStroke( wxColour( 255, 0, 0 ) );
    root->Append( tt1 );
    tt1->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);

    a2dVertexCurve* curve1 = new a2dVertexCurve();
    curve1->SetStroke( a2dStroke( wxColour( 255, 127, 0 ), 0, a2dSTROKE_DOT_DASH ) );
    curve1->AddPoint( 70, 140 );
    curve1->AddPoint( 80, 130 );
    curve1->AddPoint( 90, 120 );
    curve1->AddPoint( 100, 110 );
    curve1->AddPoint( 110, 110 );
    curve1->AddPoint( 150, 140 );
    curve1->AddPoint( 160, 150 );
    curve1->AddPoint( 170, 160 );
    curve1->AddPoint( 210, 210 );
    curve1->SetRenderMode( a2dCurve::a2dCURVES_RENDER_NORMAL | a2dCurve::a2dCURVES_RENDER_NO_POINTS );
    a2dCanvasObject::PROPID_PopupMenu->SetPropertyToObject( curve1, m_plotmousemenu );

    a2dText* label = new a2dText(  _T( "curve 1" ), 100, 114, //a2dFont( 8.0, wxFONTFAMILY_SWISS ) );
                                   a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 8.0, wxFONTENCODING_DEFAULT  ) );
    curve1->Append( label );
    plot->AddCurve( curve1, _T( "curve1" ) );

    a2dMarker* markOnCurve1 = new a2dMarker(  _T( "M1 %5.2f %5.2f" ), curve1, 150, 10 );
    markOnCurve1->SetStroke( wxColour( 255, 0, 0 ) );
    plot->AddMarker( markOnCurve1 );
    markOnCurve1->SetViewDependent( m_canvas->GetDrawingPart(), true, false, false, true );

    a2dVertexCurve* curveempty = new a2dVertexCurve();
    curveempty->SetStroke( a2dStroke( wxColour( 255, 127, 0 ), 0, a2dSTROKE_DOT_DASH ) );
    curveempty->SetRenderMode( a2dCurve::a2dCURVES_RENDER_NORMAL | a2dCurve::a2dCURVES_RENDER_NO_POINTS );
    plot->AddCurve( curveempty, _T( "curveempty" ) );
    a2dMarker* markOnCurveEmpty = new a2dMarker(  _T( "ME %5.2f %5.2f" ), curveempty, 150, 10 );
    markOnCurveEmpty->SetStroke( wxColour( 255, 0, 255 ) );
    plot->AddMarker( markOnCurveEmpty );
    markOnCurveEmpty->SetViewDependent( m_canvas->GetDrawingPart(), true, false, false, true );

    a2dMarkerShow* showm = new a2dMarkerShow( 10, 10, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.2f" ), _T( "%5.2f" ), markOnCurve1, 10 );
    plot->SetMarkerShow( showm );
    showm->SetViewDependent( m_canvas->GetDrawingPart(), true ); //,true);

    a2dMarker* markOnCurve2 = new a2dMarker(  _T( "M2 %5.2f %5.2f" ), curve1, 40 );
    markOnCurve2->SetStroke( wxColour( 25, 70, 0 ) );
    markOnCurve2->SetFill( wxColour( 0, 255, 155 ) );
    plot->AddMarker( markOnCurve2 );

    a2dMarkerShow* showm2 = new a2dMarkerShow( 340, 50, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.1f" ), _T( "%5.1f" ), markOnCurve2, 10 );
    showm2->SetStroke( wxColour( 25, 70, 0 ) );
    showm2->SetFill( wxColour( 0, 255, 155 ) );
    root->Append( showm2 );

    a2dVertexCurve* curve2 = new a2dVertexCurve();
    curve2->SetEditMode( a2dCurve::a2dCURVES_EDIT_FIXED_X );
    curve2->SetStroke( a2dStroke( wxColour( 255, 1, 0 ), 2, a2dSTROKE_DOT ) );
    curve2->AddPoint( -70, -50 );
    curve2->AddPoint( -50, -10 );
    curve2->AddPoint( -30, 50 );
    curve2->AddPoint( 70, 100 );
    curve2->AddPoint( 80, 150 );
    curve2->AddPoint( 90, 130 );
    curve2->AddPoint( 100, 120 );
    curve2->AddPoint( 110, 130 );
    curve2->AddPoint( 150, 150 );
    curve2->AddPoint( 160, 160 );
    curve2->AddPoint( 170, 170 );
    curve2->AddPoint( 210, 180 );
    //curve2->SetObjectTip(  _T("curve 2") , 85, 155, 8 );
    plot->AddCurve( curve2, _T( "curve2" ) );

    a2dMarker* markOnCurve3 = new a2dMarker(  _T( "MX %5.2f %5.2f" ), curve2, 120 );
    markOnCurve3->SetStroke( wxColour( 255, 0, 0 ) );
    markOnCurve3->SetFill( wxColour( 0, 255, 0 ) );
    plot->AddMarker( markOnCurve3 );

    a2dMarkerShow* showmmarkOnCurve3 = new a2dMarkerShow( 240, 50, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.1f" ), _T( "%5.1f" ), markOnCurve3, 10 );
    showmmarkOnCurve3->SetStroke( wxColour( 255, 0, 0 ) );
    showmmarkOnCurve3->SetFill( wxColour( 0, 255, 0 ) );
    root->Append( showmmarkOnCurve3 );

    a2dVertexCurve* curveget = ( a2dVertexCurve* ) plot->GetCurve(  _T( "curve1" ) );
    curveget->AddPoint( 215, 196 );

    //plot->Rotate(-35);

#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
    m_contr->SetDefaultBehavior( wxTC_NoContextMenu );

    //m_contr->Zoomout();
#endif //wxART2D_USE_EDITOR

    if( m_timer )
        delete m_timer;
    m_timer = new wxTimer( this );
}

void MyFrame::FillDocument9( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();
    m_canvas->GetDrawingPart()->SetViewDependentObjects( true );

    a2dPlot* plot = new a2dPlot( 0, 0 );
    plot->SetAutoYAxesNames( true );
    plot->SetAutoShowYAxes( true );
    root->Append( plot );

#ifdef __WIN32__
    double ticTextHeight = double( wxSystemSettings::GetMetric( wxSYS_MENU_Y ) ) / 1.5;
#else
    double ticTextHeight = 14;
#endif
    double ticHeight = ticTextHeight / 2;

    double plotWidth = gc_defmaxX;
    double plotHeight = gc_defmaxY;

    plot->SetBoundaries( 0, 0, plotWidth, plotHeight );
    plot->SetPadding ( plotWidth * gc_plotBoundLeftK, plotWidth * gc_plotBoundRightK,
                       plotHeight * gc_plotBoundUpK, plotHeight * gc_plotBoundDownK );

    wxColour colour( wxT( "BLUE" ) );
    wxColour backgndColour( 66, 247, 247 );
    wxColour axisFill( 255, 255, 255 );
    wxColour axisXColour( 0, 0, 0 );
    wxColour gridColour( 127, 127, 127 );
    wxColour axisXTicColour( 0, 0, 0 );
    wxColour cursorColour( 0, 127, 255 );
    wxColour cursorBGndColour( 0, 63, 127 );

    bool axisXSideTic = false;
    wxString axisXTicFormat = wxT( "%4.0f" );

    a2dText* ttt = new a2dText(  _T( "View dependent graphic\nYou must enable FreeType fonts support" ), 0, 0,
                                 a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    ttt->SetFill(  *wxRED, a2dFILL_SOLID );
    ttt->SetStroke( wxColour( 0, 10, 0 ) );
    plot->SetTitleTextDc( ttt );
    ttt->SetViewDependent( m_canvas->GetDrawingPart(), true );

    plot->SetFill( backgndColour, a2dFILL_SOLID );
    plot->SetStroke( a2dStroke( colour, 2 ) );
    plot->SetAxisAreaStroke( *a2dBLACK_STROKE );
    plot->SetAxisAreaFill( a2dFill( axisFill ) );
    plot->GetAxisX()->SetStroke( a2dStroke( axisXColour, 1 ) );
    plot->GetAxisX()->SetViewDependent( m_canvas->GetDrawingPart(), true, false, true );

    plot->GetAxisX()->SetSideTic( axisXSideTic );
    plot->GetAxisX()->SetTicFormat( axisXTicFormat );
    plot->SetGridStroke( a2dStroke( gridColour, 0, a2dSTROKE_DOT ) );
    plot->GetAxisX()->SetTicStroke( a2dStroke( axisXTicColour, 1 ) );

    plot->GetAxisX()->SetFontTic( a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), ticTextHeight, wxFONTENCODING_DEFAULT  ) );
    plot->GetAxisX()->SetTicTextHeight ( ticTextHeight );
    plot->GetAxisX()->SetTicHeight ( ticHeight );

    a2dText* ylabel = new a2dText(  wxT( "Ylabel1" ) , 0, 0, a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), ticTextHeight, wxFONTENCODING_DEFAULT  ) );
    ylabel->Rotate( 270 );
    ylabel->SetFill( wxColour( wxT( "RED" ) ) ); //ylabel->SetFill(a2dWHITE_FILL);
    ylabel->SetViewDependent( m_canvas->GetDrawingPart(), true );
    plot->SetYLabelTextDc( ylabel );

    a2dText* ylabel2 = new a2dText(  wxT( "Ylabel2" ) , 0, 0, a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), ticTextHeight, wxFONTENCODING_DEFAULT ) );
    ylabel2->Rotate( 270 );
    ylabel2->SetFill( wxColour( wxT( "RED" ) ) ); //ylabel2->SetFill(a2dWHITE_FILL);
    ylabel2->SetViewDependent( m_canvas->GetDrawingPart(), true );
    plot->SetY2LabelTextDc( ylabel2 );

    a2dCursor* cursor = new a2dCursor( 1 );
    cursor->SetStroke( a2dStroke( cursorColour, 2. ) );
    cursor->SetFill( cursorBGndColour );
    cursor->SetPosition( 0, 0, plot->GetCurvesAreaList().GetBaseCurvesArea()->GetCurveAreaTransform() );
    cursor->SetViewDependent( m_canvas->GetDrawingPart(), true, false, true );
    plot->SetCursor( cursor );

    plot->ClearCurvesAreas();

    static wxColour cs_lineColor[3] =
    {
        0x0000FF, 0xFF00, 0xFF0000
    };
    static a2dStrokeStyle ls_strokeStyle[3] =
    {
        a2dSTROKE_SOLID,
        a2dSTROKE_DOT,
        a2dSTROKE_DOT_DASH
    };
    static a2dFillStyle ls_fillStyle[7] =
    {
        //                  a2dFILL_GRADIENT_FILL_XY_LINEAR,  /*!< linear scan from colour 1 to colour two .*/
        //                  a2dFILL_GRADIENT_FILL_XY_RADIAL, /*!< circular scan from colour 1 to colour two .*/
        //                  a2dFILL_GRADIENT_FILL_XY_DROP,  /*!< sort of side scan from colour 1 to colour two .*/
        a2dFILL_BDIAGONAL_HATCH,
        a2dFILL_FDIAGONAL_HATCH,
        a2dFILL_SOLID,
        a2dFILL_CROSSDIAG_HATCH,
        a2dFILL_CROSS_HATCH,
        a2dFILL_HORIZONTAL_HATCH,
        a2dFILL_VERTICAL_HATCH
    };
    static wxString s_units[7] =
    {
        wxT( "m" ), wxT( "m/s" ), wxT( "m/s2" ), wxT( "A" ), wxT( "kg" ), wxT( "mm" ), wxT( "s" )
    };

    int curveNum = 0;
    for( int areaNum = 0; areaNum < 3; areaNum++ )
    {
        wxString areaName = wxString::Format( _T( "area_%d" ), areaNum );
        wxString areaText = wxString::Format( _T( "area_%d_text" ), areaNum );
        plot->SetInternalBoundaries( -90, -5 - areaNum * 20, 130, 5 + areaNum * 20, areaName );
        plot->SetAxisText( areaName, areaText, cs_lineColor[areaNum % 3] );
        a2dCurveAxis* pAxis = plot->GetAreaAxisY( areaName );
        pAxis->SetUnits( s_units[curveNum % 7] );
        pAxis->SetUnitsStroke( cs_lineColor[areaNum % 3] );
        pAxis->SetFontTic( a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), ticTextHeight, wxFONTENCODING_DEFAULT ) );
        pAxis->SetStroke( a2dStroke( wxColour( 0, 0, 0 ), 1 ) );
        pAxis->SetTicFormat( wxT( "%5.2f" ) );
        // see difference in Document 11 where Y-axis jumps without SetCommonTicFormat() when user moves a mouse over curve
        //(i.e. when he selects curve and its Y-axis)
        pAxis->SetCommonTicFormat( wxT( "%5.2f" ) );
        pAxis->SetTicStroke( a2dStroke( wxColour( 0, 0, 0 ), 1 ) );
        pAxis->SetTicTextHeight ( ticTextHeight );
        pAxis->SetSideTic( false );
        pAxis->SetTicHeight ( ticHeight );
        pAxis->SetViewDependent( m_canvas->GetDrawingPart(), true, false, true );
        for( int curvesInArea = 0; curvesInArea < 3; curvesInArea++, curveNum++ )
        {
            wxString curveName = wxString::Format( _T( "curve_%d" ) , curveNum );
            a2dVertexCurve* curveMax = new a2dVertexCurve;
            curveMax->SetStroke( a2dStroke( cs_lineColor[areaNum % 3], 0, ls_strokeStyle[curvesInArea % 3] ) );
            curveMax->SetRenderMode( a2dCurve::a2dCURVES_RENDER_NORMAL );
            curveMax->SetRenderLogicalFunction( wxOR_REVERSE );

            curveMax->SetFill( a2dFill( cs_lineColor[areaNum % 3], ls_fillStyle[curveNum % 7] ) );
            for( int i = 0; i < 20; i++ )
            {
                double x = i * 5 + ( curveNum % 2 ) * -30;
                double y = ( 5 + 5 * ( curveNum % 8 ) ) * sin( 2 * M_PI * double( i ) / ( ( 1 + ( curveNum % 3 ) ) * 10 ) );
                curveMax->AddPoint( x, y );
            }
            plot->AddCurveToArea( areaName, curveMax, curveName );

            a2dMarker* markOnCurve = new a2dMarker(  curveName + wxT( " %5.2f %5.2f" ),
                    curveMax,
                    0,
                    a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ),
                             ticTextHeight, wxFONTENCODING_DEFAULT  ) );
            markOnCurve->SetStroke( cs_lineColor[areaNum % 3] );
            markOnCurve->Set( 0, 0, 0 );
            markOnCurve->SetStep( 1 );
            markOnCurve->SetViewDependent( m_canvas->GetDrawingPart(), true );
            plot->AddMarkerToArea( areaName, markOnCurve );

            a2dMarkerShow* showm = new a2dMarkerShow( 0, 0, _T( "x=" ), _T( "y=" ), _T( "%5.2f" ), _T( "%5.2f" ), markOnCurve, 10 );
            plot->SetMarkerShow( showm );
            showm->SetViewDependent( m_canvas->GetDrawingPart(), true );
        }
    }

    a2dCurveAxis* pAxisX = plot->GetAxisX();
    pAxisX->SetUnits( wxT( "X Values" ) );

    cursor->SetPosition( 0, 0, plot->GetCurvesAreaList().GetBaseCurvesArea()->GetCurveAreaTransform() );
//  cursor->SetPosition(0, 0, plot->GetCurvesAreaList().GetBaseCurvesArea()->GetTransformMatrix());

    drawing->SetRootRecursive();

#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
#endif //wxART2D_USE_EDITOR

    if( m_timer )
        delete m_timer;
    m_timer = new wxTimer( this );
}

void MyFrame::FillDocument11( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();
    m_canvas->GetDrawingPart()->SetViewDependentObjects( true );

    a2dPlot* plot = new a2dPlot( 0, 0 );
    plot->SetAutoYAxesNames( true );
    plot->SetAutoShowYAxes( true );
    root->Append( plot );

    double ticTextHeight = double( wxSystemSettings::GetMetric( wxSYS_MENU_Y ) ) / 1.5;
    double ticHeight = ticTextHeight / 2;

    double plotWidth = gc_defmaxX;
    double plotHeight = gc_defmaxY;

    plot->SetBoundaries( 0, 0, plotWidth, plotHeight );
    plot->SetPadding ( plotWidth * gc_plotBoundLeftK, plotWidth * gc_plotBoundRightK,
                       plotHeight * gc_plotBoundUpK, plotHeight * gc_plotBoundDownK );

    wxColour colour( wxT( "BLUE" ) );
    wxColour backgndColour( 66, 247, 247 );
    wxColour axisFill( 255, 255, 255 );
    wxColour axisXColour( 0, 0, 0 );
    wxColour gridColour( 127, 127, 127 );
    wxColour axisXTicColour( 0, 0, 0 );
    wxColour cursorColour( 0, 127, 255 );
    wxColour cursorBGndColour( 0, 63, 127 );

    bool axisXSideTic = false;
    wxString axisXTicFormat = wxT( "%4.0f" );

    a2dText* ttt = new a2dText(  _T( "View dependent graphic without FreeType fonts" ), 0, 0,
                                 a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    ttt->SetFill(  *wxRED, a2dFILL_SOLID );
    ttt->SetStroke( wxColour( 0, 10, 0 ) );
    plot->SetTitleTextDc( ttt );
    ttt->SetViewDependent( m_canvas->GetDrawingPart(), true );

    plot->SetFill( backgndColour, a2dFILL_SOLID );
    plot->SetStroke( a2dStroke( colour, 2 ) );
    plot->SetAxisAreaStroke( *a2dBLACK_STROKE );
    plot->SetAxisAreaFill( a2dFill( axisFill ) );
    plot->GetAxisX()->SetStroke( a2dStroke( axisXColour, 1 ) );
    plot->GetAxisX()->SetViewDependent( m_canvas->GetDrawingPart(), true, false, true );

    plot->GetAxisX()->SetSideTic( axisXSideTic );
    plot->GetAxisX()->SetTicFormat( axisXTicFormat );
    plot->SetGridStroke( a2dStroke( gridColour, 0, a2dSTROKE_DOT ) );
    plot->GetAxisX()->SetTicStroke( a2dStroke( axisXTicColour, 1 ) );
    plot->GetAxisX()->SetFontTic( a2dFont( ticTextHeight, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL ) );
    plot->GetAxisX()->SetTicTextHeight ( ticTextHeight );
    plot->GetAxisX()->SetTicHeight ( ticHeight );

    a2dText* ylabel = new a2dText(  wxT( "Ylabel1" ) , 0, 0, a2dFont( ticTextHeight, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL ) );
    ylabel->Rotate( 270 );
    ylabel->SetFill( wxColour( wxT( "RED" ) ) ); //ylabel->SetFill(a2dWHITE_FILL);
    ylabel->SetViewDependent( m_canvas->GetDrawingPart(), true );
    plot->SetYLabelTextDc( ylabel );

    a2dText* ylabel2 = new a2dText(  wxT( "Ylabel2" ) , 0, 0, a2dFont( ticTextHeight, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL ) );
    ylabel2->Rotate( 270 );
    ylabel2->SetFill( wxColour( wxT( "RED" ) ) ); //ylabel2->SetFill(a2dWHITE_FILL);
    ylabel2->SetViewDependent( m_canvas->GetDrawingPart(), true );
    plot->SetY2LabelTextDc( ylabel2 );

    a2dCursor* cursor = new a2dCursor( 1 );
    cursor->SetStroke( a2dStroke( cursorColour, 2. ) );
    cursor->SetFill( cursorBGndColour );
    cursor->SetPosition( 0, 0, plot->GetCurvesAreaList().GetBaseCurvesArea()->GetCurveAreaTransform() );
    cursor->SetViewDependent( m_canvas->GetDrawingPart(), true, false, true );
    plot->SetCursor( cursor );

    plot->ClearCurvesAreas();

    static wxColour cs_lineColor[3] =
    {
        0x0000FF, 0xFF00, 0xFF0000
    };
    static a2dStrokeStyle ls_strokeStyle[3] =
    {
        a2dSTROKE_SOLID,
        a2dSTROKE_DOT,
        a2dSTROKE_DOT_DASH
    };
    static a2dFillStyle ls_fillStyle[7] =
    {
        //                  a2dFILL_GRADIENT_FILL_XY_LINEAR,  /*!< linear scan from colour 1 to colour two .*/
        //                  a2dFILL_GRADIENT_FILL_XY_RADIAL, /*!< circular scan from colour 1 to colour two .*/
        //                  a2dFILL_GRADIENT_FILL_XY_DROP,  /*!< sort of side scan from colour 1 to colour two .*/
        a2dFILL_BDIAGONAL_HATCH,
        a2dFILL_FDIAGONAL_HATCH,
        a2dFILL_SOLID,
        a2dFILL_CROSSDIAG_HATCH,
        a2dFILL_CROSS_HATCH,
        a2dFILL_HORIZONTAL_HATCH,
        a2dFILL_VERTICAL_HATCH
    };
    static wxString s_units[7] =
    {
        wxT( "m" ), wxT( "m/s" ), wxT( "m/s2" ), wxT( "A" ), wxT( "kg" ), wxT( "mm" ), wxT( "s" )
    };

    int curveNum = 0;
    for( int areaNum = 0; areaNum < 3; areaNum++ )
    {
        //setup curve area
        wxString areaName = wxString::Format( _T( "area_%d" ), areaNum );
        wxString areaText = wxString::Format( _T( "area_%d_text" ), areaNum );
        plot->SetInternalBoundaries( -90, -5 - areaNum * 20, 130, 5 + areaNum * 20, areaName );
        plot->SetAxisText( areaName, areaText, cs_lineColor[areaNum % 3] );
        //setup Yaxis for this area
        a2dCurveAxis* pAxis = plot->GetAreaAxisY( areaName );
        pAxis->SetUnits( s_units[curveNum % 7] );
        pAxis->SetUnitsStroke( cs_lineColor[areaNum % 3] );
        pAxis->SetFontTic( a2dFont( ticTextHeight, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL ) );
        pAxis->SetStroke( a2dStroke( wxColour( 0, 0, 0 ), 1 ) );
        pAxis->SetTicFormat( wxT( "%5.2f" ) );
        pAxis->SetTicStroke( a2dStroke( wxColour( 0, 0, 0 ), 1 ) );
        pAxis->SetTicTextHeight ( ticTextHeight );
        pAxis->SetSideTic( false );
        pAxis->SetTicHeight ( ticHeight );
        pAxis->SetViewDependent( m_canvas->GetDrawingPart(), true, false, true );

        //setup curves in area
        for( int curvesInArea = 0; curvesInArea < 3; curvesInArea++, curveNum++ )
        {
            wxString curveName = wxString::Format( _T( "curve_%d" ), curveNum );
            a2dVertexCurve* curveMax = new a2dVertexCurve;
            curveMax->SetStroke( a2dStroke( cs_lineColor[areaNum % 3], 0, ls_strokeStyle[curvesInArea % 3] ) );
            curveMax->SetRenderMode( a2dCurve::a2dCURVES_RENDER_NORMAL );
            curveMax->SetRenderLogicalFunction( wxOR_REVERSE );

            curveMax->SetFill( a2dFill( cs_lineColor[areaNum % 3], ls_fillStyle[curveNum % 7] ) );
            for( int i = 0; i < 20; i++ )
            {
                double x = i * 5 + ( curveNum % 2 ) * -30;
                double y = ( 5 + 5 * ( curveNum % 8 ) ) * sin( 2 * M_PI * double( i ) / ( ( 1 + ( curveNum % 3 ) ) * 10 ) );
                curveMax->AddPoint( x, y );
            }
            plot->AddCurveToArea( areaName, curveMax, curveName );

            a2dMarker* markOnCurve = new a2dMarker(  curveName + wxT( " %5.2f %5.2f" ),
                    curveMax,
                    0,
                    a2dFont( ticTextHeight, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL ) );
            markOnCurve->SetStroke( cs_lineColor[areaNum % 3] );
            markOnCurve->Set( 0, 0, 0 );
            markOnCurve->SetStep( 1 );
            markOnCurve->SetViewDependent( m_canvas->GetDrawingPart(), true );
            plot->AddMarkerToArea( areaName, markOnCurve );

            a2dMarkerShow* showm = new a2dMarkerShow( 0, 0, _T( "x=" ), _T( "y=" ), _T( "%5.2f" ), _T( "%5.2f" ), markOnCurve, 10 );
            plot->SetMarkerShow( showm );
            showm->SetViewDependent( m_canvas->GetDrawingPart(), true );
        }
    }

    a2dCurveAxis* pAxisX = plot->GetAxisX();
    pAxisX->SetUnits( wxT( "X Values" ) );

    cursor->SetPosition( 0, 0, plot->GetCurvesAreaList().GetBaseCurvesArea()->GetCurveAreaTransform() );
    //  cursor->SetPosition(0, 0, plot->GetCurvesAreaList().GetBaseCurvesArea()->GetTransformMatrix());

    drawing->SetRootRecursive();

#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
#endif //wxART2D_USE_EDITOR

    if( m_timer )
        delete m_timer;
    m_timer = new wxTimer( this );
}

void MyFrame::FillDocument5( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();

    a2dText* tempty = new a2dText( wxT( "ANGLED TTF" ), 380, 150,
                                   a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 14.0, wxFONTENCODING_DEFAULT ),
                                   270.0, false, wxMINY | wxMINX );

    tempty->SetFill( *a2dTRANSPARENT_FILL ); //a2dFill(wxColour(133,133,250)));
    tempty->SetEngineeringText();
    tempty->SetStroke( wxColour( 127, 127, 0 ) );
    root->Append( tempty );

    /*
        tempty = new a2dText(wxT("ANGLED DC"), 450,-150,
            a2dFont(14.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ),
            -40.0, false, wxMINY | wxMINX);

        tempty->SetFill(*a2dTRANSPARENT_FILL);//a2dFill(wxColour(133,133,250)));
        tempty->SetStroke(wxColour(127,127,0));
        root->Append( tempty );
    */

    a2dPlot* plot = new a2dPlot( 100, 150 );
    plot->SetDraggable( false );
    root->Append( plot );

//  plot->SetBoundaries(-50,-50,200,200);
    plot->SetBoundaries( 0, 0, 200, 200 );
    plot->SetInternalBoundaries( 200, -100, -100, 200 ); // klion: inverted x axis
    plot->SetInternalBoundaries( -100, -100, 200, 200 ); // klion: non-inverted x axis
    plot->GetAxisY()->SetTic( 20 );
    plot->SetFill( *a2dTRANSPARENT_FILL );
    plot->SetFill(  wxColour( 66, 247, 247 ), a2dFILL_SOLID );
    plot->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 0 ) );
    plot->SetAxisAreaFill( a2dFill( wxColour( 233, 250, 250 ) ) );
    plot->GetAxisX()->SetStroke( a2dStroke( wxColour( 239, 100, 0 ), 2, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisY()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 1 ) );
    plot->GetAxisX()->SetPosition( -100 );
    plot->GetAxisX()->SetTicHeight( 3 );
    plot->GetAxisX()->SetTicTextHeight( 5 );
    plot->GetAxisX()->SetAutoSizedTic( true );
    plot->GetAxisX()->SetUnits( wxT( "Umeter" ) );
    plot->GetAxisY()->SetPosition( -100 );  // klion: inverted x-scale
    plot->GetAxisY()->SetPosition( 200 );  // klion: non-inverted x-scale
    plot->GetAxisY()->SetInvertTic( true );
    plot->GetAxisY()->SetTicHeight ( 6 );
    plot->GetAxisY()->SetTicTextHeight ( 5 );
    plot->GetAxisY()->SetAutoSizedTic( true );
    plot->GetAxisY()->SetUnits( wxT( "Umeter" ) );

    a2dAxisMarker* aLevel1 = new a2dAxisMarker( wxT( "" ), plot->GetAxisX() );
    aLevel1->Set( 5, 5, 4 );
    aLevel1->Rotate( 180 );
    aLevel1->SetStroke( a2dStroke( wxColour( wxT( "RED" ) ), 1 ) );
    aLevel1->SetFill( a2dFill( wxColour( wxT( "YELLOW" ) ) ) );

    aLevel1->SetPromptFormat( wxT( "%.2f" ) );
    a2dText* aLevelPrompt1 = new a2dText( wxT( "" ), 0, 0,
                                          a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 5.0, wxFONTENCODING_DEFAULT ),
                                          0.0, false, wxMINY | wxMINX );
    aLevelPrompt1->Rotate( -180 );
    aLevelPrompt1->SetStroke( a2dStroke( wxColour( wxT( "RED" ) ), 1 ) );

    aLevel1->SetPrompt( aLevelPrompt1 );
    plot->GetAxisX()->AddLevel( wxT( "maxx" ), 10, aLevel1 );

    a2dAxisMarker* aLevel2 = new a2dAxisMarker( wxT( "" ), plot->GetAxisY() );
    aLevel2->Set( 5, 5, 4 );
    aLevel2->Rotate( -90 );
    aLevel2->SetStroke( a2dStroke( wxColour( wxT( "BLUE" ) ), 1 ) );
    aLevel2->SetFill( a2dFill( wxColour( wxT( "YELLOW" ) ) ) );

    aLevel2->SetPromptFormat( wxT( "%.2f" ) );
    a2dText* aLevelPrompt2 = new a2dText( wxT( "" ), 0, 0,
                                          a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 8.0, wxFONTENCODING_DEFAULT ),
                                          0.0, false, wxMINY | wxMINX );
    aLevelPrompt2->Rotate( 90 );
    aLevelPrompt2->SetStroke( a2dStroke( wxColour( wxT( "YELLOW" ) ), 1 ) );

    aLevel2->SetPrompt( aLevelPrompt2 );
    plot->GetAxisY()->AddLevel( wxT( "maxy" ), 20, aLevel2 );

    //plot->SetShowTics(false);
    //plot->GetAxisY()->SetSideTic(false);
    //plot->SetTicFormat("%4.1f") );
    //plot->SetTicFormat("%4.1f") );
    plot->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot->GetAxisY()->SetTicStroke( a2dStroke( wxColour( 239, 122, 57 ), 3 ) );
    plot->GetAxisY()->SetTicTextStroke( a2dStroke( wxColour( 9, 112, 134 ), 2 ) );

    plot->SetTitle( wxString( _T( "Simple Plot doc0" ) ) );
//  plot->SetXLabel(wxString(_T("time")));
//  plot->SetYLabel(wxString(_T("voltage")));

    a2dText* tx = new a2dText(  _T( "time" ), 0, 0,
                                a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    tx->SetFill(  *wxBLACK, a2dFILL_SOLID );
    tx->SetStroke( wxColour( 255, 255, 255 ) );
    plot->SetXLabelTextDc( tx );
    a2dText* ty = new a2dText(  _T( "voltage" ), 0, 0,
                                a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    ty->SetFill(  *wxBLACK, a2dFILL_SOLID );
    ty->SetStroke( wxColour( 255, 255, 255 ) );
    plot->SetYLabelTextDc( ty );


    a2dText* ttt = new a2dText(  _T( "Two LineTitle" ), 0, 0,
                                 a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    ttt->SetFill(  *wxRED, a2dFILL_SOLID );
    ttt->SetStroke( wxColour( 0, 10, 0 ) );
    plot->SetTitleTextDc( ttt );

    /*
        a2dText* ct= new a2dText(  _T("Hello Freetype Text does transform properly") , 0,100,
                          "./fonts/LiberationSans-Bold.ttf") , 13 );
        ct->Scale( 0.3, 1);
        ct->Rotate(34);
        plot->Append( ct );
    */
    a2dText* tt = new a2dText(  _T( "Normal Text does\n  not transform" ), 20, 30,
                                a2dFont( 8.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    tt->SetFill( *a2dTRANSPARENT_FILL );
    tt->SetStroke( wxColour( 220, 10, 200 ) );
    root->Append( tt );

    a2dText* tt1 = new a2dText( wxT( "Latin text! \xd0\xf3\xf1\xf1\xea\xe8\xe9 \xf2\xe5\xea\xf1\xf2" ), 10, 50, //Русский текст
                                a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 8.0, wxFONTENCODING_DEFAULT  ) );

    tt1->SetFill( *a2dTRANSPARENT_FILL );
    tt1->SetStroke( wxColour( 255, 0, 0 ) );
    root->Append( tt1 );

    a2dVertexCurve* curve1 = new a2dVertexCurve();
    curve1->SetStroke( a2dStroke( wxColour( 255, 127, 0 ), 0, a2dSTROKE_DOT_DASH ) );
    curve1->AddPoint( 70, 140 );
    curve1->AddPoint( 80, 130 );
    curve1->AddPoint( 90, 120 );
    curve1->AddPoint( 100, 110 );
    curve1->AddPoint( 110, 110 );
    curve1->AddPoint( 150, 140 );
    curve1->AddPoint( 160, 150 );
    curve1->AddPoint( 170, 160 );
    curve1->AddPoint( 210, 210 );
    curve1->SetRenderMode( a2dCurve::a2dCURVES_RENDER_NORMAL | a2dCurve::a2dCURVES_RENDER_NO_POINTS );

    a2dText* label = new a2dText(  _T( "curve 1" ), 100, 114, //a2dFont( 8.0, wxFONTFAMILY_SWISS ) );
                                   a2dFont( a2dGlobals->GetFontPathList().FindValidPath( wxT( "LiberationSans-Bold.ttf" ) ), 8.0, wxFONTENCODING_DEFAULT  ) );
    curve1->Append( label );
    plot->AddCurve( curve1, _T( "curve1" ) );

    a2dVertexCurve* curveget = ( a2dVertexCurve* ) plot->GetCurve(  _T( "curve1" ) );
    curveget->AddPoint( 215, 196 );

    a2dMarker* markOnCurve1 = new a2dMarker(  _T( "M1 %5.2f %5.2f" ), curve1, 150, 3 );
    markOnCurve1->SetStroke( wxColour( 255, 0, 0 ) );
    plot->AddMarker( markOnCurve1 );

    a2dVertexCurve* curveempty = new a2dVertexCurve();
    curveempty->SetStroke( a2dStroke( wxColour( 255, 127, 0 ), 0, a2dSTROKE_DOT_DASH ) );
    curveempty->SetRenderMode( a2dCurve::a2dCURVES_RENDER_NORMAL | a2dCurve::a2dCURVES_RENDER_NO_POINTS );
    plot->AddCurve( curveempty, _T( "curveempty" ) );
    a2dMarker* markOnCurveEmpty = new a2dMarker(  _T( "ME %5.2f %5.2f" ), curveempty, 150, 3 );
    markOnCurveEmpty->SetStroke( wxColour( 255, 0, 255 ) );
    plot->AddMarker( markOnCurveEmpty );

    a2dMarkerShow* showm = new a2dMarkerShow( 10, 10, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.2f" ), _T( "%5.2f" ), markOnCurve1, 10 );
    plot->SetMarkerShow( showm );

    a2dMarker* markOnCurve2 = new a2dMarker(  _T( "M2 %5.2f %5.2f" ), curve1, 40 );
    markOnCurve2->SetStroke( wxColour( 25, 70, 0 ) );
    markOnCurve2->SetFill( wxColour( 0, 255, 155 ) );
    plot->AddMarker( markOnCurve2 );

    a2dMarkerShow* showm2 = new a2dMarkerShow( 340, 50, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.1f" ), _T( "%5.1f" ), markOnCurve2, 20 );
    showm2->SetStroke( wxColour( 25, 70, 0 ) );
    showm2->SetFill( wxColour( 0, 255, 155 ) );
    showm2->GetXLabel()->SetEditable( false );
    showm2->GetYLabel()->SetEditable( false );
    showm2->GetYText()->SetEditable( false );
    root->Append( showm2 );

    a2dVertexCurve* curve2 = new a2dVertexCurve();
    curve2->SetEditMode( a2dCurve::a2dCURVES_EDIT_FIXED_X );
    curve2->SetStroke( a2dStroke( wxColour( 255, 1, 0 ), 2, a2dSTROKE_DOT ) );
    curve2->AddPoint( -70, -50 );
    curve2->AddPoint( -50, -10 );
    curve2->AddPoint( -30, 50 );
    curve2->AddPoint( 70, 100 );
    curve2->AddPoint( 80, 150 );
    curve2->AddPoint( 90, 130 );
    curve2->AddPoint( 100, 120 );
    curve2->AddPoint( 110, 130 );
    curve2->AddPoint( 150, 150 );
    curve2->AddPoint( 160, 160 );
    curve2->AddPoint( 170, 170 );
    curve2->AddPoint( 210, 180 );
    curve2->SetObjectTip(  _T( "curve 2" ) , 85, 155, 8 );
    plot->AddCurve( curve2, _T( "curve2" ) );

    a2dMarker* markOnCurve3 = new a2dMarker(  _T( "MX %5.2f %5.2f" ), curve2, 120 );
    markOnCurve3->SetStroke( wxColour( 255, 0, 0 ) );
    markOnCurve3->SetFill( wxColour( 0, 255, 0 ) );
    plot->AddMarker( markOnCurve3 );

    a2dMarkerShow* showmmarkOnCurve3 = new a2dMarkerShow( 240, 50, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.1f" ), _T( "%5.1f" ), markOnCurve3, 10 );
    showmmarkOnCurve3->SetStroke( wxColour( 255, 0, 0 ) );
    showmmarkOnCurve3->SetFill( wxColour( 0, 255, 0 ) );
    root->Append( showmmarkOnCurve3 );

    a2dRectMM* tr = new a2dRectMM( 340, 50, 200, 140 );
    tr->SetResizeOnChildBox( true );
    tr->SetFill( wxColour( 1, 227, 225 ), a2dFILL_HORIZONTAL_HATCH );
    tr->SetStroke( wxColour( 198, 3, 105 ), 0.0 );
    tr->SetSubEdit( true );
    tr->SetBorder( 10 );
    root->Append( tr );

    a2dMarkerShow* showmmarkOnCurve4 = new a2dMarkerShow( 50, 50, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.1f" ), _T( "%5.1f" ), markOnCurve3, 10 );
    showmmarkOnCurve4->SetStroke( wxColour( 255, 0, 0 ) );
    showmmarkOnCurve4->SetFill( wxColour( 0, 255, 0 ) );
    showmmarkOnCurve4->SetSubEditAsChild( true );
    tr->Append( showmmarkOnCurve4 );

    a2dMarkerShow* showm22 = new a2dMarkerShow( 50, 100, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.1f" ), _T( "%5.1f" ), markOnCurve2, 10 );
    showm22->SetStroke( wxColour( 25, 70, 0 ) );
    showm22->SetFill( wxColour( 0, 255, 155 ) );
    showm22->GetXLabel()->SetEditable( false );
    showm22->GetYLabel()->SetEditable( false );
    showm22->GetYText()->SetEditable( false );
    showm22->SetSubEditAsChild( true );
    tr->Append( showm22 );


    //plot->Rotate(-35);

#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
    //m_contr->Zoomout();
#endif //wxART2D_USE_EDITOR
}

void MyFrame::FillDocument1( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();

    a2dPlot* plot = new a2dPlot( 100, 150 );
    root->Append( plot );

    plot->SetBoundaries( 0, 0, 200, 200 );
    plot->SetInternalBoundaries( -30, -60, 200, 200 );
    plot->SetFill( *a2dTRANSPARENT_FILL );
    plot->SetFill(  wxColour( 66, 247, 247 ), a2dFILL_SOLID );
    plot->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 0 ) );
    plot->SetAxisAreaFill( a2dFill( wxColour( 233, 250, 250 ) ) );
    plot->GetAxisX()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 2 ) );
    plot->GetAxisY()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 2 ) );
    //plot->SetShowTics(false);
    //plot->SetSideTic(false);
    //plot->SetTicFormat("%4.1f") );
    //plot->SetTicFormat("%4.1f") );
    plot->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot->GetAxisY()->SetTicStroke( a2dStroke( wxColour( 239, 122, 57 ), 3 ) );
    plot->SetTitle( wxString( _T( "Simple Plot" ) ) );
    plot->SetXLabel( wxString( _T( "time" ) ) );
    plot->SetYLabel( wxString( _T( "voltage" ) ) );

    a2dText* ttt = new a2dText(  _T( "Two Line\nTitle" ), 0, 0,
                                 a2dFont( 10.0, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_ITALIC ) );
    ttt->SetFill(  *wxRED, a2dFILL_SOLID );
    ttt->SetStroke( wxColour( 0, 10, 0 ) );
    plot->SetTitleTextDc( ttt );


    a2dText* tt = new a2dText(  _T( "Normal Text does\n  not transform" ), 20, 30,
                                a2dFont( 8.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ), 0 , false  );
    tt->SetFill( *a2dTRANSPARENT_FILL );
    tt->SetStroke( wxColour( 220, 10, 200 ) );
    root->Append( tt );

    a2dVertexCurve* curve1 = new a2dVertexCurve();
    curve1->SetStroke( a2dStroke( wxColour( 255, 127, 0 ), 0, a2dSTROKE_DOT_DASH ) );
    curve1->AddPoint( 70, 140 );
    curve1->AddPoint( 80, 130 );
    curve1->AddPoint( 90, 120 );
    curve1->AddPoint( 100, 110 );
    curve1->AddPoint( 110, 110 );
    curve1->AddPoint( 150, 140 );
    curve1->AddPoint( 160, 150 );
    curve1->AddPoint( 170, 160 );
    curve1->AddPoint( 210, 210 );
    curve1->SetRenderMode( a2dCurve::a2dCURVES_RENDER_NORMAL | a2dCurve::a2dCURVES_RENDER_NO_POINTS );

    a2dText* label = new a2dText(  _T( "curve 1" ), 100, 114, a2dFont( 8.0, wxFONTFAMILY_SWISS ) );
    curve1->Append( label );
    plot->AddCurve( curve1, _T( "curve1" ) );

    a2dMarker* markOnCurve1 = new a2dMarker(  _T( "M1 %5.2f %5.2f" ), curve1, 150 );
    markOnCurve1->SetStroke( wxColour( 255, 0, 0 ) );
    plot->AddMarker( markOnCurve1 );

    a2dMarkerShow* showm = new a2dMarkerShow( 80, 120, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.2f" ), _T( "%5.2f" ), markOnCurve1, 10 );
    root->Append( showm );

    a2dMarker* markOnCurve2 = new a2dMarker(  _T( "M2 %5.2f %5.2f" ), curve1, 40 );
    markOnCurve2->SetStroke( wxColour( 25, 70, 0 ) );
    markOnCurve2->SetFill( wxColour( 0, 255, 155 ) );
    plot->AddMarker( markOnCurve2 );

    a2dMarkerShow* showm2 = new a2dMarkerShow( 240, 120, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.1f" ), _T( "%5.1f" ), markOnCurve2, 10 );
    showm2->SetStroke( wxColour( 25, 70, 0 ) );
    showm2->SetFill( wxColour( 0, 255, 155 ) );
    root->Append( showm2 );

    a2dVertexCurve* curve2 = new a2dVertexCurve();
    curve2->SetEditMode( a2dCurve::a2dCURVES_EDIT_FIXED_X );
    curve2->SetStroke( a2dStroke( wxColour( 255, 1, 0 ), 2, a2dSTROKE_DOT ) );
    curve2->AddPoint( 70, 100 );
    curve2->AddPoint( 80, 150 );
    curve2->AddPoint( 90, 130 );
    curve2->AddPoint( 100, 120 );
    curve2->AddPoint( 110, 130 );
    curve2->AddPoint( 150, 150 );
    curve2->AddPoint( 160, 160 );
    curve2->AddPoint( 170, 170 );
    curve2->AddPoint( 210, 180 );
    //curve2->SetObjectTip(  _T("curve 2") , 85, 155, 8 );
    plot->AddCurve( curve2, _T( "curve2" ) );

    a2dMarker* markOnCurve3 = new a2dMarker(  _T( "M1 %5.2f %5.2f" ), curve2, 120 );
    markOnCurve3->SetStroke( wxColour( 255, 0, 0 ) );
    markOnCurve3->SetFill( wxColour( 0, 255, 0 ) );
    plot->AddMarker( markOnCurve3 );

    a2dMarkerShow* showmmarkOnCurve3 = new a2dMarkerShow( 80, 90, _T( "Time =" ), _T( "Voltage =" ), _T( "%5.1f" ), _T( "%5.1f" ), markOnCurve3, 10 );
    showmmarkOnCurve3->SetStroke( wxColour( 255, 0, 0 ) );
    showmmarkOnCurve3->SetFill( wxColour( 0, 255, 0 ) );
    root->Append( showmmarkOnCurve3 );

    a2dVertexCurve* curveget = ( a2dVertexCurve* ) plot->GetCurve(  _T( "curve1" ) );
    curveget->AddPoint( 215, 196 );

//extra test of scaled version
    a2dPlot* plot2 = new a2dPlot( 0, 0 );

    plot2->SetFill( *a2dTRANSPARENT_FILL );
    plot2->SetFill(  wxColour( 66, 247, 247 ), a2dFILL_SOLID );
    plot2->SetStroke( a2dStroke( wxColour( 29, 0, 0 ), 0 ) );
    plot2->SetAxisAreaFill( a2dFill( wxColour( 23, 25, 20 ) ) );
    plot2->GetAxisX()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 2 ) );
    plot2->GetAxisY()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 2 ) );
    plot2->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    plot2->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot2->GetAxisY()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot2->GetAxisX()->SetTicTextStroke( a2dStroke( wxColour( 9, 212, 134 ), 2 ) );
    plot2->GetAxisY()->SetTicTextStroke( a2dStroke( wxColour( 129, 112, 234 ), 2 ) );
    plot2->SetTitle( wxString(  _T( "Simple Plot2" ) ) );
    plot2->SetXLabel( wxString(  _T( "time" ) ) );
    plot2->SetYLabel( wxString(  _T( "voltage" ) ) );

    plot2->SetBoundaries( 0, 0, 200, 200 );
    plot2->GetCurves()->clear();
    a2dBarCurve* curve22 = new a2dBarCurve();
    curve22->SetEditMode( a2dCurve::a2dCURVES_EDIT_FIXED_X );
    curve22->SetStroke( a2dStroke( wxColour( 255, 1, 0 ), 0, a2dSTROKE_SOLID ) );
    curve22->SetFill( a2dFill( wxColour( 29, 245, 164 ), a2dFILL_FDIAGONAL_HATCH ) );
    curve22->SetBarWidth( 5 );
    curve22->SetExtrudeDepth( 2.5 );
    curve22->SetExtrudeFill( a2dFill( wxColour( 29, 24, 164 ), a2dFILL_SOLID )  );

    unsigned int c;
    for ( c = 0; c < 9; c++ )
        curve22->GetPointFillList().push_back(  a2dFill( wxColour( rand() % 254, rand() % 254, rand() % 254 ) ) );


    curve22->SetRenderMode( a2dBarCurve::a2dBAR_RENDER_3D );
    curve22->AddPoint( -20, -140 );
    curve22->AddPoint( -10, -130 );
    curve22->AddPoint( 0, -120 );
    curve22->AddPoint( 20, -100 );
    curve22->AddPoint( 50, -50 );
    curve22->AddPoint( 60, -30 );
    curve22->AddPoint( 70, -100 );
    curve22->AddPoint( 80, -50 );
    curve22->AddPoint( 90, -30 );
    curve22->AddPoint( 100, 20 );
    curve22->AddPoint( 110, 30 );
    curve22->AddPoint( 150, 50 );
    curve22->AddPoint( 160, 60 );
    curve22->AddPoint( 170, 70 );
    curve22->AddPoint( 210, 80 );
    plot2->AddCurve( curve22, _T( "curve22" ) );

    a2dBarCurve* curve33 = new a2dBarCurve();
    curve33->SetStroke( a2dStroke( wxColour( 255, 1, 0 ), 0, a2dSTROKE_SOLID ) );
    curve33->SetFill( a2dFill( wxColour( 29, 245, 244 ), a2dFILL_BDIAGONAL_HATCH ) );
    curve33->SetBarWidth( 5 );
    curve33->SetExtrudeDepth( 2.5 );
    curve33->SetExtrudeFill( a2dFill( wxColour( 29, 224, 64 ), a2dFILL_SOLID )  );

    curve33->SetRenderMode( a2dBarCurve::a2dBAR_RENDER_NORMAL );
    curve33->AddPoint( -20, -90 );
    curve33->AddPoint( -10, -120 );
    curve33->AddPoint( 0, -100 );
    curve33->AddPoint( 20, -80 );
    curve33->AddPoint( 50, -30 );
    curve33->AddPoint( 60, -20 );
    curve33->AddPoint( 70, -70 );
    curve33->AddPoint( 80, -40 );
    curve33->AddPoint( 90, -20 );
    curve33->AddPoint( 100, 30 );
    curve33->AddPoint( 110, 40 );
    curve33->AddPoint( 150, 70 );
    curve33->AddPoint( 160, 80 );
    curve33->AddPoint( 170, 90 );
    curve33->AddPoint( 210, 100 );

    unsigned int cc;
    for ( cc = 0; cc < 9; cc++ )
        curve33->GetPointFillList().push_back(  a2dFill( wxColour( rand() % 254, rand() % 254, rand() % 254 ), a2dFILL_BDIAGONAL_HATCH  ) );

    plot2->AddCurve( curve33, _T( "curve33" ) );

    plot2->SetInternalBoundaries( -30, -160, 240, 120 );
    root->Append( plot2 );
    //plot2->Scale( 6, 6);
    //plot2->Rotate(34);
    plot2->Translate( 400, 150 );

    a2dPlot* plot3 = new a2dPlot( 900, 150 );

    plot3->SetFill(  wxColour( 16, 17, 17 ), a2dFILL_SOLID );
    plot3->SetStroke( a2dStroke( wxColour( 255, 0, 0 ), 0 ) );
    plot3->SetAxisAreaFill( a2dFill( wxColour( 213, 215, 210 ) ) );
    plot3->SetGridStroke( a2dStroke( wxColour( 255, 17, 27 ), 0, a2dSTROKE_DOT_DASH ) );
    plot3->GetAxisX()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 2 ) );
    plot3->GetAxisY()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 2 ) );
    plot3->GetAxisY()->SetTicStroke( a2dStroke( wxColour( 239, 122, 75 ), 2 ) );
    plot3->GetAxisY()->SetTicTextStroke( a2dStroke( wxColour( 9, 232, 235 ), 2 ) );
    plot3->GetAxisY()->SetTicTextHeight( 5 );
    plot3->SetTitle( wxString(  _T( "List Plot 3" ) ) );
    plot3->SetXLabel( wxString(  _T( "bigs" ) ) );
    plot3->SetYLabel( wxString(  _T( "in space" ) ) );

    plot3->SetBoundaries( 0, 0, 200, 200 );
    plot3->SetInternalBoundaries( -100, -100, 200, 250 );

    a2dVertexListCurve* curve3 = new a2dVertexListCurve();
    curve3->SetStroke( a2dStroke( wxColour( 98, 25, 30 ), 2, a2dSTROKE_DOT ) );
    int i;
    for( i = -100; i < 225; i = i + 5 )
    {
        curve3->AddPoint( i, 110 + 100 * sin( i / 10.0 ) );
    }
    plot3->AddCurve( curve3, _T( "curve3" ) );

    a2dVertexListCurve* curve4 = new a2dVertexListCurve();
    curve4->SetStroke( a2dStroke( wxColour( 5, 25, 0 ), 0, a2dSTROKE_SOLID ) );
    for( i = -100; i < 150; i = i + 4 )
    {
        curve4->AddPoint( i, 110 + 100 * cos( i / 10.0 ) + 50 * sin( i / 20.0 ) );
    }
    plot3->AddCurve( curve4, _T( "curve4" ) );
    root->Append( plot3 );



    a2dPlot* plot4 = new a2dPlot( 200, 550 );

    plot4->SetFill(  wxColour( 16, 17, 17 ), a2dFILL_SOLID );
    plot4->SetStroke( a2dStroke( wxColour( 255, 0, 0 ), 0 ) );
    plot4->SetAxisAreaFill( a2dFill( wxColour( 213, 215, 210 ) ) );
    plot4->SetGridStroke( a2dStroke( wxColour( 255, 17, 27 ), 0, a2dSTROKE_DOT_DASH ) );
    plot4->GetAxisX()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 2 ) );
    plot4->GetAxisY()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 2 ) );
    plot4->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot4->GetAxisX()->SetTicTextStroke( a2dStroke( wxColour( 9, 232, 235 ), 2 ) );
    plot4->SetGridStroke( a2dStroke( wxColour( 255, 17, 27 ), 0, a2dSTROKE_DOT_DASH ) );
    plot4->GetAxisY()->SetTicTextHeight( 10 );
    plot4->GetAxisY()->SetTicFormat( wxT( "%4.1f" ) );
    plot4->SetTitle( wxString(  _T( "List Plot 4" ) ) );
    plot4->SetXLabel( wxString(  _T( "bigs" ) ) );
    plot4->SetYLabel( wxString(  _T( "in space" ) ) );

    plot4->SetBoundaries( 0, 0, 200, 200 );
    plot4->SetInternalBoundaries( -100, -1.5, 200, 2.5 );

    a2dVertexCurve* curve44 = new a2dVertexCurve();
    curve44->SetRenderMode( a2dCurve::a2dCURVES_RENDER_FILL );
    curve44->SetStroke( a2dStroke( wxColour( 5, 25, 0 ), 0, a2dSTROKE_SOLID ) );
    curve44->SetFill( a2dFill( wxColour( 29, 245, 164 ), a2dFILL_SOLID ) );
    for( i = -100; i < 200; i = i + 4 )
    {
        curve44->AddPoint( i, 1.1 + cos( i / 10.0 ) + 0.5 * sin( i / 20.0 ) );
    }
    plot4->AddCurve( curve44, _T( "curve44" ) );

    a2dVertexCurve* curve43 = new a2dVertexCurve();
    curve43->SetRenderMode( a2dCurve::a2dCURVES_RENDER_FILL );
    curve43->SetStroke( a2dStroke( wxColour( 98, 25, 30 ), 2, a2dSTROKE_DOT ) );
    curve43->SetFill( a2dFill( wxColour( 129, 25, 44 ), a2dFILL_FDIAGONAL_HATCH ) );
    for( i = -100; i < 220; i = i + 4 )
    {
        curve43->AddPoint( i, 1.1 + sin( i / 10.0 ) );
    }
    plot4->AddCurve( curve43, _T( "curve43" ) );

    root->Append( plot4 );


#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
#endif //wxART2D_USE_EDITOR

    //plot->SetInternalBoundaries(120,110,200,200);
    //plot->SetPending(true);

    a2dCanvasObject* rcht = new a2dCanvasObject(  80, -10 );
    root->Append( rcht );


    a2dText* cht = new a2dText(  _T( "Two LineTitle" ) , 0, 0,
                                 a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    cht->SetFill(  *wxRED, a2dFILL_SOLID );
    cht->SetStroke( wxColour( 0, 10, 0 ) );
    rcht->Append( cht );

    drawing->SetRootRecursive();

    //m_contr->Zoomout();

    if( m_timer )
        delete m_timer;
    m_timer = new wxTimer( this );
    //m_timer->Start( 280, false );
    //m_plot3 = plot3;
}


void MyFrame::FillDocument2( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();
    
    a2dPlot* plot = new a2dPlot( 100, 50 );
    root->Append( plot );

    plot->SetBoundaries( 0, 0, 300, 300 );
    plot->SetInternalBoundaries( 100, 100, 200, 200 );
    //plot->SetInternalBoundaries(0,0,300,300);
    plot->SetFill( *a2dTRANSPARENT_FILL );
    plot->SetAxisAreaFill( a2dFill( wxColour( 233, 250, 250 ) ) );
    plot->SetStroke( a2dStroke( wxColour( 250, 250, 250 ), 0 ) );
    plot->GetAxisX()->SetTicFormat( wxT( "%4.1f" ) );
    plot->GetAxisY()->SetTicFormat( wxT( "%4.1f" ) );
    plot->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot->GetAxisY()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot->SetTitle( _( "Plot 2" ) );
    plot->SetXLabel( _T( "Xlabel" ) );
    plot->SetYLabel( _T( "Ylabel" ) );
    a2dCanvasObject::PROPID_Allowrotation->SetPropertyToObject( plot, false );
    a2dCanvasObject::PROPID_Allowsizing->SetPropertyToObject( plot, false );
    a2dCanvasObject::PROPID_Allowskew->SetPropertyToObject( plot, false );

    wxColour cursorColour( 0, 127, 255 );
    wxColour cursorBGndColour( 0, 63, 127 );
    a2dCursor* cursor = new a2dCursor( 1 );
    cursor->SetStroke( a2dStroke( cursorColour, 2. ) );
    cursor->SetFill( cursorBGndColour );
    cursor->SetPosition( 0, 0, plot->GetCurvesAreaList().GetBaseCurvesArea()->GetCurveAreaTransform() );
    plot->SetCursor( cursor );

    a2dVertexCurve* curve2 = new a2dVertexCurve();
    curve2->SetStroke( a2dStroke( wxColour( 255, 1, 0 ), 2, a2dSTROKE_DOT ) );
    curve2->AddPoint( 70, 100 );
    curve2->AddPoint( 80, 153 );
    curve2->AddPoint( 90, 130 );
    curve2->AddPoint( 100, 124 );
    curve2->AddPoint( 110, 130 );
    curve2->AddPoint( 150, 154 );
    curve2->AddPoint( 160, 163 );
    curve2->AddPoint( 170, 140 );
    curve2->AddPoint( 185, 195 );
    curve2->AddPoint( 210, 180 );
    curve2->SetEditMode( a2dCurve::a2dCURVES_EDIT_ALLOW_ADD | a2dCurve::a2dCURVES_EDIT_FIXED_X );

#if !defined (__GNUC__) || (__GNUC__ > 3)
    //curve2->SetObjectTip(  _T("curve 2") , 85, 155, 8 );
#endif
    plot->AddCurve( curve2, _T( "curve2" ) );

    drawing->SetRootRecursive();

    a2dMarker* markOnCurve1 = new a2dMarker(  _T( "M1 %5.2f %5.2f" ), curve2, 150, 10 );
    markOnCurve1->SetStroke( wxColour( 255, 0, 0 ) );
    plot->AddMarker( markOnCurve1 );

    a2dStroke markerColor = a2dStroke( cursorColour, 1. );
    a2dAxisMarker* cursorAxisMarkerBottom = new a2dAxisMarker(  _T( "" ), plot->GetAxisX() );
    cursorAxisMarkerBottom->SetStroke( markerColor );
    cursorAxisMarkerBottom->SetFill( *a2dTRANSPARENT_FILL );
    cursorAxisMarkerBottom->Rotate( 180 );
    cursorAxisMarkerBottom->Set( 10, 8, 8 );
    plot->GetAxisX()->SetTicHeight ( 5 );
    plot->GetAxisX()->AddLevel( _( "CursorAxisMarkerBottom" ), 150., cursorAxisMarkerBottom );
    cursor->AddMarker( cursorAxisMarkerBottom );

    a2dCursorAxisMarker* cursorAxisMarkerTop = new a2dCursorAxisMarker(  _T( "" ),
            plot->GetAxisX(), // axis object for moving
            cursor,           // cursor object to get Y-position
            150,              // X-pos
            100 );            // Y-pos in the percent of cursor's length
    cursorAxisMarkerTop->SetStroke( markerColor );
    cursorAxisMarkerTop->SetFill( *a2dTRANSPARENT_FILL );
    cursorAxisMarkerTop->Set( 10, 8, 8 );
    plot->GetAxisX()->AddLevel( _( "CursorAxisMarkerTop" ), 150., cursorAxisMarkerTop );
    cursor->AddMarker( cursorAxisMarkerTop );

    cursor->SetPosition( 150, 0, plot->GetCurvesAreaList().GetBaseCurvesArea()->GetCurveAreaTransform() );

#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::FillDocument10( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();
    
    a2dPlot* plot = new a2dPlot( 100, 50 );
    root->Append( plot );

    plot->SetBoundaries( 0, 0, 300, 300 );
    plot->SetInternalBoundaries( 100, 100, 200, 200 );
    plot->SetFill( *a2dTRANSPARENT_FILL );
    plot->SetAxisAreaFill( a2dFill( wxColour( 233, 250, 250 ) ) );
    plot->SetStroke( a2dStroke( wxColour( 250, 250, 250 ), 0 ) );
    plot->GetAxisX()->SetTicFormat( wxT( "%4.1f" ) );
    plot->GetAxisY()->SetTicFormat( wxT( "%4.1f" ) );
    plot->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot->GetAxisY()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    //plot->SetTitle(_("Plot 2"));
    //plot->SetXLabel(_T("Xlabel"));
    //plot->SetYLabel(_T("Ylabel"));

    wxColour cursorColour( 0, 127, 255 );
    wxColour cursorBGndColour( 0, 63, 127 );
    a2dCursor* cursor = new a2dCursor( 1 );
    cursor->SetStroke( a2dStroke( cursorColour, 1. ) );
    cursor->SetFill( cursorBGndColour );
    cursor->SetPosition( 0, 0, plot->GetCurvesAreaList().GetBaseCurvesArea()->GetCurveAreaTransform() );
    plot->SetCursor( cursor );

    a2dVertexCurve* curve1 = new a2dVertexCurve();
    curve1->SetStroke( a2dStroke( wxColour( 1, 255, 0 ), 8, a2dSTROKE_SOLID ) );
    //          curve1->SetRenderLogicalFunction(wxXOR);
    //          curve1->SetRenderLogicalFunction(wxOR_REVERSE);
    curve1->SetRenderLogicalFunction( wxEQUIV );
    curve1->AddPoint( 70, 190 );
    curve1->AddPoint( 80, 183 );
    curve1->AddPoint( 90, 160 );
    curve1->AddPoint( 100, 154 );
    curve1->AddPoint( 110, 140 );
    curve1->AddPoint( 150, 134 );
    curve1->AddPoint( 160, 123 );
    curve1->AddPoint( 170, 115 );
    curve1->AddPoint( 210, 110 );
//#ifndef __QNXNTO__
#if !defined (__GNUC__) || (__GNUC__ > 3)
    curve1->SetObjectTip(  _T( "curve 1" ) , 70, 175, 8 );
#endif
    plot->AddCurve( curve1, _T( "curve1" ) );

    a2dVertexCurve* curve2 = new a2dVertexCurve();
    curve2->SetStroke( a2dStroke( wxColour( 255, 1, 0 ), 8, a2dSTROKE_LONG_DASH ) );
    //          curveMax->SetRenderLogicalFunction(wxXOR);
    curve2->SetRenderLogicalFunction( wxOR_REVERSE );
    //  curve2->SetRenderLogicalFunction(wxEQUIV);
    curve2->AddPoint( 70, 100 );
    curve2->AddPoint( 80, 153 );
    curve2->AddPoint( 90, 130 );
    curve2->AddPoint( 100, 124 );
    curve2->AddPoint( 110, 130 );
    curve2->AddPoint( 150, 154 );
    curve2->AddPoint( 160, 163 );
    curve2->AddPoint( 170, 170 );
    curve2->AddPoint( 210, 180 );
//#ifndef __QNXNTO__
#if !defined (__GNUC__) || (__GNUC__ > 3)
    curve2->SetObjectTip(  _T( "curve 2" ) , 85, 155, 8 );
#endif
    plot->AddCurve( curve2, _T( "curve2" ) );


    a2dVertexCurve* curve44 = new a2dVertexCurve();
    curve44->SetRenderMode( a2dCurve::a2dCURVES_RENDER_FILL );
    curve44->SetRenderLogicalFunction( wxOR_REVERSE );
    curve44->SetStroke( a2dStroke( wxColour( 5, 25, 0 ), 0, a2dSTROKE_SOLID ) );
    curve44->SetFill( a2dFill( wxColour( 29, 245, 164 ), a2dFILL_SOLID ) );
    int i;
    for( i = 100; i < 200; i = i + 4 )
    {
        curve44->AddPoint( i, 150 + 30 * ( cos( i / 10.0 ) + 0.5 * sin( i / 20.0 ) ) );
    }
    plot->AddCurve( curve44, _T( "curve44" ) );

    a2dVertexCurve* curve43 = new a2dVertexCurve();
    curve43->SetRenderMode( a2dCurve::a2dCURVES_RENDER_FILL );
    curve43->SetRenderLogicalFunction( wxOR_REVERSE );
    curve43->SetStroke( a2dStroke( wxColour( 98, 25, 30 ), 5, a2dSTROKE_DOT ) );
    curve43->SetFill( a2dFill( wxColour( 129, 25, 44 ), a2dFILL_FDIAGONAL_HATCH ) );
    for( i = 100; i < 200; i = i + 4 )
    {
        curve43->AddPoint( i, 150 + 20 * ( 1.1 + sin( i / 10.0 ) ) );
    }
    plot->AddCurve( curve43, _T( "curve43" ) );

    drawing->SetRootRecursive();

    a2dMarker* markOnCurve1 = new a2dMarker(  _T( "M1 %5.2f %5.2f" ), curve1, 150, 5 );
    markOnCurve1->SetStroke( wxColour( 0, 255, 0 ) );
    plot->AddMarker( markOnCurve1 );

    a2dMarker* markOnCurve2 = new a2dMarker(  _T( "M2 %5.2f %5.2f" ), curve2, 150, 5 );
    markOnCurve2->SetStroke( wxColour( 255, 0, 0 ) );
    plot->AddMarker( markOnCurve2 );

    a2dMarker* markOnCurve43 = new a2dMarker(  _T( "M43 %5.2f %5.2f" ), curve43, 150, 10 );
    markOnCurve43->SetStroke( wxColour( 98, 25, 30 ) );
    plot->AddMarker( markOnCurve43 );

    a2dMarker* markOnCurve44 = new a2dMarker(  _T( "M44 %5.2f %5.2f" ), curve44, 150, 10 );
    markOnCurve44->SetStroke( wxColour( 29, 245, 164 ) );
    plot->AddMarker( markOnCurve44 );

    cursor->SetPosition( 150, 0, plot->GetCurvesAreaList().GetBaseCurvesArea()->GetCurveAreaTransform() );

#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::FillDocument3( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();
    
    a2dPlot* plot1 = new a2dPlot( 50, 150 );
    root->Append( plot1 );
    plot1->SetBoundaries( 0, 0, 200, 300 );
    plot1->SetInternalBoundaries( 0, 0, 200, 300 );
    plot1->SetFill( *a2dTRANSPARENT_FILL );
    plot1->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 0 ) );
    plot1->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    a2dText* text1 = new a2dText(  _T( "Plot1 Yaxis" ), 0, 0,
                                   a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    plot1->SetTitle( _( "Plot 1" ) );
    plot1->SetXLabel( _T( "Xlabel" ) );
    plot1->SetYLabelTextDc( ( a2dText* ) text1 );

    a2dPlot* plot2 = new a2dPlot( 300, 250 );
    root->Append( plot2 );
    plot2->SetBoundaries( 0, 0, 100, 150 );
    plot2->SetInternalBoundaries( 0, 0, 100, 150 );
    plot2->SetFill( *a2dTRANSPARENT_FILL );
    plot2->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 0 ) );
    plot2->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    a2dText* text2 = new a2dText(  _T( "Plot2 Yaxis" ), 0, 0,
                                   a2dFont( 10.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ), 90.0 );
    plot2->SetTitle( _( "Plot 2" ) );
    plot2->SetXLabel( _T( "Xlabel" ) );
    plot2->SetYLabelTextDc( ( a2dText* ) text2 );

    a2dPlot* plot3 = new a2dPlot( 500, 250 );
    root->Append( plot3 );
    plot3->SetBoundaries( 0, 0, 100, 150 );
    plot3->SetInternalBoundaries( 0, 0, 100, 150 );
    plot3->SetFill( *a2dTRANSPARENT_FILL );
    plot3->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 0 ) );
    plot3->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    a2dText* text3 = new a2dText(  _T( "Plot3 Yaxis" ), 0, 0,
                                   a2dFont( 5.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ), -90.0 );
    plot3->SetTitle( _( "Plot 3" ) );
    plot3->SetXLabel( _T( "Xlabel" ) );
    plot3->SetYLabelTextDc( ( a2dText* ) text3 );

    a2dPlot* plot4 = new a2dPlot( 700, 250 );
    root->Append( plot4 );
    plot4->SetBoundaries( 0, 0, 100, 150 );
    plot4->SetInternalBoundaries( 0, 0, 100, 150 );
    plot4->SetFill( *a2dTRANSPARENT_FILL );
    plot4->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 0 ) );
    plot4->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    a2dText* text4 = new a2dText(  _T( "Plot4 Yaxis" ), 0, 0,
                                   a2dFont( 5.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ), -90.0 );
    plot4->SetTitle( _( "Plot 4" ) );
    plot4->SetXLabel( _T( "Xlabel" ) );
    plot4->SetYLabelTextDc( ( a2dText* ) text4 );

    a2dVertexCurve* curve1 = new a2dVertexCurve();
    curve1->SetStroke( a2dStroke( wxColour( 255, 127, 0 ), 0, a2dSTROKE_DOT_DASH ) );
    curve1->AddPoint( 10, 40 );
    curve1->AddPoint( 20, 30 );
    curve1->AddPoint( 40, 20 );
    curve1->AddPoint( 50, 10 );
    curve1->AddPoint( 60, 10 );
    curve1->AddPoint( 70, 40 );
    curve1->AddPoint( 80, 50 );
    curve1->AddPoint( 90, 60 );
    curve1->AddPoint( 100, 10 );
    a2dText* label = new a2dText(  _T( "curve 1" ), 100, 114, a2dFont( 8.0, wxFONTFAMILY_SWISS ) );
    curve1->Append( label );

    plot1->AddCurve( curve1, _T( "curve1" ) );

    a2dVertexCurve* curve2 = new a2dVertexCurve();
    curve2->SetStroke( a2dStroke( wxColour( 255, 127, 0 ), 0, a2dSTROKE_DOT_DASH ) );
    curve2->AddPoint( -20, 35 );
    curve2->AddPoint( -10, 30 );
    curve2->AddPoint( 0, 40 );
    curve2->AddPoint( 20, 30 );
    curve2->AddPoint( 40, 20 );
    curve2->AddPoint( 50, 10 );
    curve2->AddPoint( 60, 10 );
    curve2->AddPoint( 70, 40 );
    curve2->AddPoint( 80, 50 );
    curve2->AddPoint( 90, 60 );
    curve2->AddPoint( 100, 10 );
    a2dText* label2 = new a2dText(  _T( "curve 2" ), 100, 114, a2dFont( 8.0, wxFONTFAMILY_SWISS ) );
    curve2->Append( label2 );

    plot2->AddCurve( curve2, _T( "curve2" ) );

    a2dVertexCurve* curve3 = new a2dVertexCurve();
    curve3->SetStroke( a2dStroke( wxColour( 255, 127, 0 ), 0, a2dSTROKE_DOT_DASH ) );
    curve3->AddPoint( -20, 35 );
    curve3->AddPoint( -10, 30 );
    curve3->AddPoint( 0, 40 );
    curve3->AddPoint( 20, 30 );
    curve3->AddPoint( 40, 20 );
    curve3->AddPoint( 50, 10 );
    curve3->AddPoint( 60, 10 );
    curve3->AddPoint( 70, 40 );
    curve3->AddPoint( 80, 50 );
    curve3->AddPoint( 90, 60 );
    curve3->AddPoint( 100, 10 );
    a2dText* label3 = new a2dText(  _T( "curve 3" ), 100, 114, a2dFont( 8.0, wxFONTFAMILY_SWISS ) );
    curve3->Append( label3 );

    plot3->AddCurve( curve3, _T( "curve3" ) );

    a2dVertexCurve* curve4 = new a2dVertexCurve();
    curve4->SetStroke( a2dStroke( wxColour( 255, 127, 0 ), 0, a2dSTROKE_DOT_DASH ) );
    curve4->AddPoint( -20, 35 );
    curve4->AddPoint( -10, 30 );
    curve4->AddPoint( 0, 40 );
    curve4->AddPoint( 20, 30 );
    curve4->AddPoint( 40, 20 );
    curve4->AddPoint( 50, 10 );
    curve4->AddPoint( 60, 10 );
    curve4->AddPoint( 70, 40 );
    curve4->AddPoint( 80, 50 );
    curve4->AddPoint( 90, 60 );
    curve4->AddPoint( 100, 10 );
    a2dText* label4 = new a2dText(  _T( "curve 4" ), 100, 114, a2dFont( 8.0, wxFONTFAMILY_SWISS ) );
    curve4->Append( label4 );

    plot4->AddCurve( curve4, _T( "curve4" ) );


#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    m_contr->SetDragMode( wxDRAW_RECTANGLE );
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
#endif //wxART2D_USE_EDITOR

    drawing->SetRootRecursive();
    if( m_timer )
        delete m_timer;
    m_timer = new wxTimer( this );
}

void MyFrame::FillDocument4( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();

    a2dPlot* plot4 = new a2dPlot( 200, 550 );

    plot4->SetFill(  wxColour( 16, 17, 17 ), a2dFILL_SOLID );
    plot4->SetStroke( a2dStroke( wxColour( 255, 0, 0 ), 0 ) );
    plot4->SetAxisAreaFill( a2dFill( wxColour( 213, 215, 210 ) ) );
    plot4->SetGridStroke( a2dStroke( wxColour( 255, 17, 27 ), 0, a2dSTROKE_DOT_DASH ) );
    plot4->GetAxisX()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 2 ) );
    plot4->GetAxisY()->SetStroke( a2dStroke( wxColour( 239, 0, 0 ), 2 ) );
    plot4->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot4->GetAxisX()->SetTicTextStroke( a2dStroke( wxColour( 9, 232, 235 ), 2 ) );
    plot4->SetGridStroke( a2dStroke( wxColour( 255, 17, 27 ), 0, a2dSTROKE_DOT_DASH ) );
    plot4->GetAxisY()->SetTicTextHeight( 10 );
    plot4->SetTitle( wxString(  _T( "List Plot 4" ) ) );
    plot4->SetXLabel( wxString(  _T( "bigs" ) ) );
    plot4->SetYLabel( wxString(  _T( "in space" ) ) );
    plot4->GetAxisY()->SetTicFormat( wxT( "%4.1f" ) );

    plot4->SetBoundaries( 0, 0, 200, 200 );
    //plot4->SetInternalBoundaries(-100,-100,200,250);
    plot4->SetInternalBoundaries( -100, -1.5, 200, 2.5 );

    a2dVertexCurve* curve44 = new a2dVertexCurve();
    curve44->SetRenderMode( a2dCurve::a2dCURVES_RENDER_FILL );
    curve44->SetStroke( a2dStroke( wxColour( 5, 25, 0 ), 0, a2dSTROKE_SOLID ) );
    a2dFill cur44fill = a2dFill( wxColour( 29, 245, 164 ), a2dFILL_SOLID );
    cur44fill.SetAlpha( 170 );
    curve44->SetFill( cur44fill );

    int i;
    for( i = -100; i < 200; i = i + 4 )
    {
        curve44->AddPoint( i, 1.1 + cos( i / 10.0 ) + 0.5 * sin( i / 20.0 ) );
        /* my changed line instead curve44->AddPoint( i, 110 +
        100*cos(i/10.0) + 50 * sin(i/20.0) ); */
    }
    plot4->AddCurve( curve44, _T( "curve44" ) );

    a2dVertexCurve* curve43 = new a2dVertexCurve();
    curve43->SetRenderMode( a2dCurve::a2dCURVES_RENDER_FILL );
    curve43->SetStroke( a2dStroke( wxColour( 98, 25, 30 ), 2, a2dSTROKE_DOT ) );
    a2dFill cur34fill = a2dFill( wxColour( 129, 25, 44 ), a2dFILL_FDIAGONAL_HATCH );
    cur34fill.SetAlpha( 120 );
    curve43->SetFill( cur34fill );

    for( i = -100; i < 200; i = i + 4 )
    {
        curve43->AddPoint( i, 1.1 + sin( i / 10.0 ) );     /* my changed
                              line instead curve43->AddPoint( i, 110 + 100*sin(i/10.0) ); */
    }
    plot4->AddCurve( curve43, _T( "curve43" ) );

    root->Append( plot4 );


#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
#endif //wxART2D_USE_EDITOR

}

void MyFrame::FillDocument6( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();
    
    a2dPlot* plot = new a2dPlot( 100, 50 );
    root->Append( plot );

    plot->SetBoundaries( -300, -300, 300, 300 );
    plot->SetInternalBoundaries( 0, -50, 110, 220 );
    plot->SetFill( *a2dTRANSPARENT_FILL );
    plot->SetAxisAreaFill( a2dFill( wxColour( 233, 250, 250 ) ) );
    plot->SetStroke( a2dStroke( wxColour( 250, 250, 250 ), 0 ) );
    plot->GetAxisX()->SetTicFormat( wxT( "%4.1f" ) );
    plot->GetAxisY()->SetTicFormat( wxT( "%4.1f" ) );
    plot->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot->GetAxisY()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    //plot->SetTitle(_("Plot 2"));
    //plot->SetXLabel(_T("Xlabel"));
    //plot->SetYLabel(_T("Ylabel"));

    a2dBandCurve* curve2 = new a2dBandCurve();
    a2dFill fill2 = a2dFill( wxColour( 129, 125, 64 ), a2dFILL_SOLID );
    fill2.SetAlpha( 130 );
    curve2->SetFill( fill2 );
    curve2->SetStroke( a2dStroke( wxColour( 255, 1, 0 ), 2, a2dSTROKE_DOT ) );
    curve2->AddPoint( 10, 70, 100 );
    curve2->AddPoint( 20, 80, 153 );
    curve2->AddPoint( 30, 90, 130 );
    curve2->AddPoint( 40, 100, 124 );
    curve2->AddPoint( 50, 110, 130 );
    curve2->AddPoint( 60, 150, 154 );
    curve2->AddPoint( 70, 160, 163 );
    curve2->AddPoint( 80, 170, 175 );
    curve2->AddPoint( 90, 210, 180 );

//#ifndef __QNXNTO__
#if !defined (__GNUC__) || (__GNUC__ > 3)
    curve2->SetObjectTip(  _T( "curve 2" ) , 85, 155, 8 );
#endif
    curve2->SetTipWindow( _T( "tip window for curve2" ) );
    plot->AddCurve( curve2, _T( "curve2" ) );
    curve2->SetRenderMode( a2dCurve::a2dCURVES_RENDER_FILL | a2dCurve::a2dCURVES_RENDER_NO_POINTS );

    a2dBandMarker* markOnCurve2 = new a2dBandMarker(  _T( "M1 %5.2f %5.2f" ), curve2, 150, 10 );
    markOnCurve2->SetStroke( wxColour( 255, 0, 0 ) );
    markOnCurve2->SetFill( wxColour( 0, 255, 255 ) );
    markOnCurve2->SetMarkerUpLow( false );
    plot->AddMarker( markOnCurve2 );

    a2dBandCurve* curve3 = new a2dBandCurve();
    a2dFill fill = a2dFill( wxColour( 29, 225, 164 ), a2dFILL_SOLID );
    fill.SetAlpha( 130 );
    curve3->SetFill( fill );
    curve3->SetStroke( a2dStroke( wxColour( 255, 121, 0 ), 2, a2dSTROKE_SOLID ) );
    curve3->AddPoint( 10, 7, -35 );
    curve3->AddPoint( 20, 8, -50 );
    curve3->AddPoint( 30, 9, -45 );
    curve3->AddPoint( 40, 10, -34 );
    curve3->AddPoint( 50, 11, -35 );
    curve3->AddPoint( 60, 15, -30 );
    curve3->AddPoint( 70, 12, -20 );
    curve3->AddPoint( 80, 17, -30 );
    curve3->AddPoint( 90, 6, -50 );
    curve3->SetRenderMode( a2dCurve::a2dCURVES_RENDER_FILL );
    curve3->SetEditMode( a2dCurve::a2dCURVES_EDIT_FIXED_X );
    curve3->SetTipWindow( _T( "tip window for curve3" ) );
    plot->AddCurve( curve3, _T( "curve3" ) );


    a2dCurveGroupLegend* legend = new a2dCurveGroupLegend( _T( "name = %s" ), plot, a2dFont( 30.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    legend->SetStroke( a2dStroke( wxColour( 255, 121, 0 ), 2, a2dSTROKE_SOLID ) );
    legend->SetPosXY( 0, -302 );

    //plot->SetDraggable( false );
    a2dCanvasObject::PROPID_Allowsizing->SetPropertyToObject( plot, false );

    root->Append( legend );

    drawing->SetRootRecursive();

#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::FillDocument7( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();
    
    drawing->GetLayerSetup()->SetStroke( 0, a2dStroke( wxColour( 55, 12, 25 ), 0, a2dSTROKE_SOLID ) );

    a2dPlot* plot = new a2dPlot( 100, 50 );
    root->Append( plot );

    plot->SetBoundaries( -300, -300, 300, 300 );
    plot->SetInternalBoundaries( 0, 50, 210, 220 );
    plot->SetFill( *wxLIGHT_GREY, a2dFILL_SOLID );
    plot->SetAxisAreaFill( a2dFill( wxColour( 233, 250, 250 ) ) );
    plot->SetStroke( a2dStroke( wxColour( 250, 250, 250 ), 0 ) );
    plot->GetAxisX()->SetTicFormat( wxT( "%4.1f" ) );
    plot->GetAxisY()->SetTicFormat( wxT( "%4.1f" ) );
    plot->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot->GetAxisY()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    //plot->SetTitle(_("Plot 2"));
    //plot->SetXLabel(_T("Xlabel"));
    //plot->SetYLabel(_T("Ylabel"));

    a2dPieCurve* curve3 = new a2dPieCurve();
    curve3->SetStroke( a2dStroke( wxColour( 255, 121, 0 ), 2, a2dSTROKE_SOLID ) );
    curve3->AddPoint( 10, 7 );
    curve3->AddPoint( 20, 8 );
    curve3->AddPoint( 30, 9 );
    curve3->AddPoint( 40, 10 );
    curve3->AddPoint( 50, 11 );
    curve3->AddPoint( 60, 15 );
    curve3->AddPoint( 70, 12 );
    curve3->AddPoint( 80, 17 );
    curve3->AddPoint( 90, 6 );
    curve3->SetRenderMode( a2dCurve::a2dCURVES_RENDER_FILL );
    plot->AddCurve( curve3, _T( "curve3" ) );


    plot->SetFill( *a2dTRANSPARENT_FILL );
    plot->SetAxisAreaFill( *a2dTRANSPARENT_FILL );
    plot->SetAxisAreaStroke( *a2dTRANSPARENT_STROKE );

    plot->SetShowGrid( false );
    plot->SetShowXaxis( false );
    plot->SetShowYaxis( false );
    plot->SetXLabel( wxString( _T( "whatever" ) ) );
    plot->SetYLabel( wxString( _T( "whatever" ) ) );

    a2dText* ttt = new a2dText(  _T( "Pie plot" ), 0, 0,
                                 a2dFont( 30.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    ttt->SetFill(  *wxRED, a2dFILL_SOLID );
    ttt->SetStroke( wxColour( 0, 10, 0 ) );
    plot->SetTitleTextDc( ttt );


    unsigned int i;
    for ( i = 0; i < 9; i++ )
        curve3->GetPointFillList().push_back( a2dFill( wxColour( rand() % 254, rand() % 254, rand() % 254 ) ) );

    drawing->SetRootRecursive();


#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
#endif //wxART2D_USE_EDITOR
}

void MyFrame::FillDocument12( a2dDrawing* drawing )
{
    a2dCanvasObjectPtr root = drawing->GetRootObject();
    
    a2dPlot* plot = new a2dPlot( 100, 50 );
    root->Append( plot );

    plot->SetBoundaries( -300, -300, 300, 300 );
    plot->SetInternalBoundaries( 0, -50, 110, 220 );
    plot->SetFill( *a2dTRANSPARENT_FILL );
    plot->SetAxisAreaFill( a2dFill( wxColour( 233, 250, 250 ) ) );
    plot->SetStroke( a2dStroke( wxColour( 250, 250, 250 ), 0 ) );
    plot->GetAxisX()->SetTicFormat( wxT( "%4.1f" ) );
    plot->GetAxisY()->SetTicFormat( wxT( "%4.1f" ) );
    plot->SetGridStroke( a2dStroke( wxColour( 127, 127, 127 ), 0, a2dSTROKE_DOT_DASH ) );
    plot->GetAxisX()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    plot->GetAxisY()->SetTicStroke( a2dStroke( wxColour( 239, 122, 5 ), 2 ) );
    //plot->SetTitle(_("Plot 2"));
    //plot->SetXLabel(_T("Xlabel"));
    //plot->SetYLabel(_T("Ylabel"));

    a2dSweepCurve* curve1 = new a2dSweepCurve();
    a2dFill fill1 = a2dFill( wxColour( 129, 125, 64 ), a2dFILL_SOLID );
    fill1.SetAlpha( 130 );
    curve1->SetFill( fill1 );
    curve1->SetStroke( a2dStroke( wxColour( 255, 1, 0 ), 2, a2dSTROKE_DOT ) );
    curve1->AddPoint( 10, 70, 100 );
    curve1->AddPoint( 20, 80, 153 );
    curve1->AddPoint( 30, 90, 130 );
    curve1->AddPoint( 40, 100, 124 );
    curve1->AddPoint( 50, 110, 130 );
    curve1->AddPoint( 60, 100, 154 );
    curve1->AddPoint( 70, 90, 163 );
    curve1->AddPoint( 80, 80, 175 );
    curve1->AddPoint( 90, 75, 180 );

//#ifndef __QNXNTO__
#if !defined (__GNUC__) || (__GNUC__ > 3)
    curve1->SetObjectTip(  _T( "sweep curve 1" ) , 85, 155, 8 );
#endif
    curve1->SetTipWindow( _T( "tip window for sweep curve1" ) );
    plot->AddCurve( curve1, _T( "sweep curve1" ) );
    curve1->SetRenderMode( a2dCurve::a2dCURVES_RENDER_FILL | a2dCurve::a2dCURVES_RENDER_NO_POINTS );

    a2dMarker* markOnCurve1 = new a2dMarker(  _T( "M1 %5.2f %5.2f" ), curve1, 10, 10 );
    markOnCurve1->SetStroke( wxColour( 255, 0, 0 ) );
    markOnCurve1->SetFill( wxColour( 0, 255, 255 ) );
//  markOnCurve1->SetMarkerUpLow( false );
    plot->AddMarker( markOnCurve1 );

    a2dSweepCurve* curve2 = new a2dSweepCurve();
    a2dFill fill2 = a2dFill( wxColour( 29, 225, 164 ), a2dFILL_SOLID );
    fill2.SetAlpha( 130 );
    curve2->SetFill( fill2 );
    curve2->SetStroke( a2dStroke( wxColour( 255, 121, 0 ), 2, a2dSTROKE_SOLID ) );
    curve2->AddPoint( 10, 7, 35 );
    curve2->AddPoint( 20, 8, 50 );
    curve2->AddPoint( 30, 9, 45 );
    curve2->AddPoint( 40, 10, 34 );
    curve2->AddPoint( 50, 11, 35 );
    curve2->AddPoint( 60, 17, 30 );
    curve2->AddPoint( 70, 12, 20 );
    curve2->AddPoint( 80, 17, 30 );
    curve2->AddPoint( 90, 6, 50 );
    curve2->SetRenderMode( a2dCurve::a2dCURVES_RENDER_FILL );
    curve2->SetEditMode( a2dCurve::a2dCURVES_EDIT_FIXED_X );
    curve2->SetTipWindow( _T( "tip window for sweep curve2" ) );
    plot->AddCurve( curve2, _T( "sweep curve2" ) );

    a2dMarker* markOnCurve2 = new a2dMarker(  _T( "M2 %5.2f %5.2f" ), curve2, 10, 10 );
    markOnCurve2->SetStroke( wxColour( 0, 255, 0 ) );
    markOnCurve2->SetFill( wxColour( 255, 0, 255 ) );
    //  markOnCurve1->SetMarkerUpLow( false );
    plot->AddMarker( markOnCurve2 );

    a2dCurveGroupLegend* legend = new a2dCurveGroupLegend( _T( "name = %s" ), plot, a2dFont( 30.0, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC ) );
    legend->SetStroke( a2dStroke( wxColour( 255, 121, 0 ), 2, a2dSTROKE_SOLID ) );
    legend->SetPosXY( 0, -302 );

    //plot->SetDraggable( false );
    a2dCanvasObject::PROPID_Allowsizing->SetPropertyToObject( plot, false );

    root->Append( legend );

    drawing->SetRootRecursive();

#if wxART2D_USE_EDITOR
    //Optional a tool controller (to drag with ctrl mouse left down and zoom etc.)
    m_contr = new a2dStToolContr( m_canvas->GetDrawingPart(), this );
    //m_contr->SetDragMode(wxDRAW_RECTANGLE);
    m_contr->SetDrawMode( a2dWIREFRAME_INVERT );
    //always first tool is zoom tool
    m_contr->SetZoomFirst( true );
#endif //wxART2D_USE_EDITOR
}
