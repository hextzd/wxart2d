/*! \file canvas/samples/split/include/split.h
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: split.h,v 1.1 2007/01/17 20:56:47 titato Exp $
*/

#ifndef __simple_H__
#define __simple_H__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/canwidget.h"
#include "wx/timer.h"

//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

class MyFrame: public a2dDrawingEditor
{
public:
    // constructors and destructors
    MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_FRAME_STYLE );
    ~MyFrame();

private:

    void OnTheme( a2dDrawingEditorFrameEvent& themeEvent );

    void OnInit( a2dDrawingEditorFrameEvent& initEvent );

private:

    a2dImage*    m_smile1;
    a2dImage*    m_smile2;
    a2dCircle*   m_circle;
    a2dRect*     m_rect;
    wxTimer*          m_timer;

private:
    void OnCloseWindow( wxCloseEvent& event );
    void OnTimer( wxTimerEvent& event );

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
