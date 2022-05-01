/*! \file editor/samples/zoom/zoom.h
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: zoom.h,v 1.3 2007/07/24 21:46:52 titato Exp $
*/

#ifndef __simple_H__
#define __simple_H__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/editor/editmod.h"
#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#endif //wxART2D_USE_AGGDRAWER

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#define ID_QUIT     101

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

    PlotCanvas( wxFrame* frame, wxWindowID id = -1,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxScrolledWindowStyle );

    ~PlotCanvas();

    void OnSize( wxSizeEvent& event );

    void OnIdle( wxIdleEvent& event );

    //! single controller with a zoom.
    a2dStToolContr* m_contr;

    bool m_startup;

    bool m_DoShowAllAuto;

    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

class MyFrame: public wxFrame
{
public:
    // constructors and destructors
    MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_FRAME_STYLE );
    ~MyFrame();

private:
    void CreateMyMenuBar();
    void OnDrawer( wxCommandEvent& event );


private:
    PlotCanvas* m_canvas;

private:

    void ZoomAll( wxCommandEvent& event );
    void SetSwitchY( wxCommandEvent& WXUNUSED( event ) );
    void TestFixScrollMaxmum( wxCommandEvent& event );

    void OnQuit( wxCommandEvent& event );
    void OnCloseWindow( wxCloseEvent& event );

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
