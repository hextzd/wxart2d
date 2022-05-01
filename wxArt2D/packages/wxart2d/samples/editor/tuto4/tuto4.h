/*! \file editor/samples/tuto4/simple.h
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: tuto4.h,v 1.1 2007/01/17 21:35:15 titato Exp $
*/

#ifndef __simple_H__
#define __simple_H__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/editor/editmod.h"

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

    void OnSize( wxSizeEvent& event );

    void OnIdle( wxIdleEvent& event );

    //! single controller with a zoom.
    a2dSmrtPtr<a2dStToolContr> m_contr;

    bool m_startup;

    DECLARE_EVENT_TABLE()
};


//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

class MyFrame: public wxFrame
{
public:

    MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_FRAME_STYLE );
    ~MyFrame();

private:
    void CreateMyMenuBar();

private:

    PlotCanvas* m_canvas;

private:
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
