/*! \file curves/samples/curtuto1/curtuto1.h
    \author Sirin

    Copyright: 2001-2004 (C) Sirin

    Licence: wxWidgets Licence

    RCS-ID: $Id: curtuto1.h,v 1.1 2007/01/17 21:51:09 titato Exp $
*/

#ifndef __simple_H__
#define __simple_H__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/curves/plotbox.h"
#include "wx/canvas/canmod.h"
#include "wx/canextobj/imageioh.h"
#include "wx/canvas/sttool.h"
#include "wx/canvas/edit.h"
#include <wx/printdlg.h>
#include "wx/timer.h"

//----------------------------------------------------------------------------
// PlotCanvas
//----------------------------------------------------------------------------

//! Do a remapping when resizing.
class PlotCanvas: public a2dCanvas
{
public:

    PlotCanvas( wxFrame* frame,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxScrolledWindowStyle );

    void OnSize( wxSizeEvent& event );
    void OnChar( wxKeyEvent& event );
//  void OnUpdateViews(a2dDocumentEvent& event);

    void OnIdle( wxIdleEvent& event );

    void SetMappingShowAtAll( bool centre = true );

    //! single controller with a zoom.
    a2dStToolContr* m_contr;

    bool m_startup;

    bool m_DoShowAllAuto;

    wxUint16 m_documentNr;
protected:
    void DoSetMappingShowAtAll( int clientw, int clienth, bool centre, int recursive );
    void OnSizeAsWXCanvas( wxSizeEvent& event );

    DECLARE_DYNAMIC_CLASS( PlotCanvas )
    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
// PlotView
//----------------------------------------------------------------------------

//! Do a remapping for view dependent objects when resizing and curve zooming.
class PlotPart: public a2dDrawingPartTiled
{
public:

    PlotPart();
    ~PlotPart();

    void OnUpdate( a2dDrawingEvent& event );

protected:

    bool m_alreadyUpdating;

    DECLARE_DYNAMIC_CLASS( PlotPart )
    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

class MyFrame: public a2dDocumentFrame
{

#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    a2dInitCurrentSmartPointerOwner m_initCurrentSmartPointerOwner;
#endif

public:

    MyFrame( wxFrame* parent, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_FRAME_STYLE );
    ~MyFrame();

private:
    void CreateMyMenuBar();

private:

    wxMenu*    m_edit_menu;

    PlotCanvas*    m_canvas;

    wxMenu* m_plotmousemenu;

#if wxART2D_USE_EDITOR
    //! if wanted add a tool controller
    a2dSmrtPtr<a2dStToolContr> m_contr;
#endif //wxART2D_USE_EDITOR

    wxTimer*          m_timer;

    //! it is better the wxEvent* than the bool for a recursive check
    //  wxEvent *m_alreadyInProcessEvent;
    //! but it is better the wxList than wxEvent* for a recursive check
    wxList  m_alreadyInProcessEvents;

    a2dPlot* m_plot3;

    a2dSmrtPtr<a2dDrawing> m_drawing;

    void FillData( wxCommandEvent& event );

    void FillDataDocNr( wxUint16 documentNr );

    void FillDocument11( a2dDrawing* drawing );
    void FillDocument10( a2dDrawing* drawing );
    void FillDocument9( a2dDrawing* drawing );
    void FillDocument8( a2dDrawing* drawing );
    void FillDocument7( a2dDrawing* drawing );
    void FillDocument6( a2dDrawing* drawing );
    void FillDocument5( a2dDrawing* drawing );
    void FillDocument0( a2dDrawing* drawing );
    void FillDocument1( a2dDrawing* drawing );
    void FillDocument2( a2dDrawing* drawing );
    void FillDocument3( a2dDrawing* drawing );
    void FillDocument4( a2dDrawing* drawing );
    void FillDocument12( a2dDrawing* drawing );


#if wxART2D_USE_AGGDRAWER
    void OnDrawer( wxCommandEvent& event );
#endif //wxART2D_USE_AGGDRAWER

    void OnCursorOnlyByCurves( wxCommandEvent& event );
    void OnInvertXAxis( wxCommandEvent& event );
    void OnInvertYAxis( wxCommandEvent& event );
    void OnSwitchXAxis( wxCommandEvent& event );

private:

    void Refresh( wxCommandEvent& event );

    void PushCurveTool( wxCommandEvent& event );
    void EndCurveTool( wxCommandEvent& event );
    void PushPolygonTool( wxCommandEvent& event );
    void PushRectTool( wxCommandEvent& event );
    void PushEditTool( wxCommandEvent& event );
    void PushTextTool( wxCommandEvent& event );
    void PushSelectTool( wxCommandEvent& event );
    void PushDeleteTool( wxCommandEvent& event );

    void EndTool( wxCommandEvent& event );
    void OnAbout( wxCommandEvent& event );

    void OnExit( wxCommandEvent& event );
    void OnCloseWindow( wxCloseEvent& event );

//    void OnPrint(wxCommandEvent& WXUNUSED(event));
//    void OnPreview(wxCommandEvent& WXUNUSED(event));
//    void OnPrintSetup(wxCommandEvent& WXUNUSED(event));
    void OnTimer( wxTimerEvent& event );

    void OnFileOpen( wxCommandEvent& WXUNUSED( event ) );
    void OnFileSave( wxCommandEvent& WXUNUSED( event ) );
    void OnFileSaveAs( wxCommandEvent& WXUNUSED( event ) );

    void OnSvgSave( wxCommandEvent& WXUNUSED( event ) );
    void OnBitmapSave( wxCommandEvent& WXUNUSED( event ) );
    void OnAnyBitmapSave( wxCommandEvent& WXUNUSED( event ) );
    void OnAnyBitmapSaveIoHandler( wxCommandEvent& WXUNUSED( event ) );

    bool ProcessEvent( wxEvent& event );

private:
    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
// MyApp
//----------------------------------------------------------------------------

class MyApp: public wxApp
{
public:
    MyApp();

    virtual bool OnInit();
    virtual int OnExit();

    MyFrame* m_frame;
};

#endif
