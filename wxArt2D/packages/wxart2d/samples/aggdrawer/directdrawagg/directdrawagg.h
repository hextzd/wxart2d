/*! \file aggdrawer/samples/directdrawagg/directdrawagg.h
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: directdrawagg.h,v 1.4 2009/09/10 17:04:09 titato Exp $
*/

#ifndef __directdraw_H__
#define __directdraw_H__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "a2dprivate.h"

// wxArt2d headers
#if wxART2D_USE_ARTBASE
#include "wx/artbase/artmod.h"
#include "wx/artbase/drawer2d.h"
#else
#error "Cannot compile this sample without ARTBASE module !"
#endif
#if wxART2D_USE_AGGDRAWER
#include "wx/aggdrawer/aggdrawer.h"
#else
#error "Cannot compile this sample without AGGDRAWER module !"
#endif



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
    DrawerOption_First,
    Switch_Drawer_aggRgba = DrawerOption_First,
    Switch_Drawer_agg,
    Switch_Drawer_agggc,
    Switch_Drawer_dc,
    Switch_Drawer_gdiplus,
    Switch_Drawer_dcgc,
    Switch_Drawer_gdigc,
    DrawerOption_Last = Switch_Drawer_gdigc,
    Map_Restore,
    MenuOption_Last = Map_Restore
};

class wxMyCanvas: public wxScrolledWindow
{
public:

    //!constructor
    /*!
    construct a canvas window.

    \param parent parent window (use wxNO_FULL_REPAINT_ON_RESIZE on parent wxFrame)
    \param id window id
    \param pos position of window
    \param size size of window
    \param style type of window (wxHSCROLL|wxVSCROLL)

    */
    wxMyCanvas( wxWindow* parent, wxWindowID id = -1,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxScrolledWindowStyle );

    //!destructor
    /*!
    */
    virtual ~wxMyCanvas();

    //!background fill for the canvas
    void SetBackgroundFill( const a2dFill& backgroundfill );

    //!get current background fill for the canvas
    a2dFill& GetBackgroundFill() { return m_backgroundfill; }

    //!set if the Yaxis goes up or down
    void SetYaxis( bool up );

    //!get currently used Yaxis setting
    bool GetYaxis() const;

    void ClearBackground();

    //! update buffer of drawer.
    /*!
        here the drawing is defined.
    */
    void Update();

    a2dDrawer2D* GetDrawer2D() { return m_drawer2d; }

    void SetDrawer2D( a2dDrawer2D* drawer2d );

protected:

    //! resize, adjusting buffer of a2dCanvasView if needed.
    void OnSize( wxSizeEvent& event );

    //! repaint damaged araes, taking into acount non updated araes in a2dCanvasView.
    void OnPaint( wxPaintEvent& event );

    //! Not yet implemented
    void OnEraseBackground( wxEraseEvent& event );

    a2dDrawer2D* m_drawer2d;

    //!background fill of canvas and background color of background fill in case of mono colour fill
    a2dFill m_backgroundfill;

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
    // method declarations for MyFrame
    void CreateMyMenuBar();

private:
    // member variable declarations for MyFrame
    wxMyCanvas* m_canvas;

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
