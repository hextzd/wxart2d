/*! \file canvas/samples/cansim/cansim.h
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: cansim.h,v 1.1 2007/01/17 20:56:46 titato Exp $
*/

#ifndef __simplecan_H__
#define __simplecan_H__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/cansim.h"
#include "wx/timer.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#define ID_QUIT     101

// IDs the menu commands
enum
{
    MenuOption_First,
    Scale_StretchHoriz = MenuOption_First,
    Scale_ShrinkHoriz,
    Scale_StretchVertic,
    Scale_ShrinkVertic,
    Origin_MoveDown,
    Origin_MoveUp,
    Origin_MoveLeft,
    Origin_MoveRight,
    Axis_Vertic,
    Map_Restore,
    MenuOption_Last = Map_Restore
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
    // method declarations for MyFrame
    void CreateMyMenuBar();

private:
    // member variable declarations for MyFrame
    a2dCanvasSim*      m_canvas;

    a2dImage*    m_smile1;
    a2dImage*    m_smile2;
    wxTimer*          m_timer;

private:
    // handler declarations for MyFrame
    void OnQuit( wxCommandEvent& event );
    void OnCloseWindow( wxCloseEvent& event );
    void OnTimer( wxTimerEvent& event );

    void OnOption( wxCommandEvent& event );

    double      m_xScale;
    double      m_yScale;
    double      m_xOrigin;
    double      m_yOrigin;
    bool        m_yAxisReversed;

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
