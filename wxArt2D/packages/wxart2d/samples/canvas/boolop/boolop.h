/*! \file canvas/samples/boolop/boolop.h
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: boolop.h,v 1.3 2007/03/26 13:58:44 titato Exp $
*/

#ifndef __simple_H__
#define __simple_H__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/canvas/canmod.h"
#include "wx/timer.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------


enum
{
    ID_FILLDOC = wxID_HIGHEST + 1,
    ID_HIERARCHY,
    ID_OPER_OR,
    ID_OPER_AND,
    ID_OPER_EXOR,
    ID_OPER_A_B,
    ID_OPER_B_A,
    ID_OPER_Offset,
    ID_OPER_Smooth,
    ID_OPER_Ring,
    ID_QUIT
};

class MyFrame;

//! Overview canvas
class OverviewCanvas: public a2dCanvas
{
public:

    OverviewCanvas( MyFrame* frame );

    ~OverviewCanvas();

    void OnChar( wxKeyEvent& event );

    MyFrame* m_myframe;

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

    void FillDoc1( a2dCanvasObject* root );

    void FillDoc2( a2dCanvasObject* root );

    void FillDoc3( a2dCanvasObject* root );

#if wxART2D_USE_KBOOL
    a2dBooleanWalkerHandler m_booloper;
#endif //wxART2D_USE_KBOOL

private:
    // WDR: method declarations for MyFrame
    void CreateMyMenuBar();

    void OnTimer( wxTimerEvent& event );

    // WDR: member variable declarations for MyFrame
    a2dCanvas*         m_canvas;

    a2dDrawing* m_root;

    wxTimer* m_timer;


private:
    // WDR: handler declarations for MyFrame
    void OnQuit( wxCommandEvent& event );
    void OnCloseWindow( wxCloseEvent& event );

    void OnBoolOperation_And( wxCommandEvent& event );

    void OnRefill( wxCommandEvent& event );

    void OnRemoveHierarchy( wxCommandEvent& event );

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
