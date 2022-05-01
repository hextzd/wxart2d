/*! \file artbase/samples/directdraw/directdraw.h
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: tiletest.h,v 1.2 2008/07/19 18:29:52 titato Exp $
*/

#ifndef __directdraw_H__
#define __directdraw_H__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/artbase/drawer2d.h"

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

WX_DECLARE_LIST( wxRect, wxRectList );

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

    void Clear();

    //! update buffer of drawer.
    /*!
        here the drawing is defined.
    */
    void Update();

    a2dDrawer2D* GetDrawer2D() { return m_drawer2d; }

protected:

    void OnChar( wxKeyEvent& event );

    //! resize, adjusting buffer of a2dCanvasView if needed.
    void OnSize( wxSizeEvent& event );

    //! repaint damaged araes, taking into acount non updated araes in a2dCanvasView.
    void OnPaint( wxPaintEvent& event );

    //! Not yet implemented
    void OnEraseBackground( wxEraseEvent& event );

    a2dDrawer2D* m_drawer2d;

    //!background fill of canvas and background color of background fill in case of mono colour fill
    a2dFill m_backgroundfill;

    wxUint16 m_nrRect;

    wxRectList m_rects;

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
