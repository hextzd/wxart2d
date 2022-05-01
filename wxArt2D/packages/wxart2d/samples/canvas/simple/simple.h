/*! \file canvas/samples/simple/simple.h
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: simple.h,v 1.2 2007/08/21 00:05:36 titato Exp $
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

#define ID_QUIT     101

// WDR: class declarations

//----------------------------------------------------------------------------
// MyCanvasStToolContr
//----------------------------------------------------------------------------

//!The MyCanvasStToolContr is a Tool Controller specialized for working with a2dCanvasView.
class MyCanvasStToolContr: public a2dStToolContr
{
    DECLARE_EVENT_TABLE()

public:

    //! Constructor
    /*!
        \param canvas a2dCanvasView where the controller takes events from
        \param where wxFrame to display statusbar text in.
    */
    MyCanvasStToolContr( a2dDrawingPart* drawer, wxFrame* where );

    //! default implementation for property editing
    void MyEditProperties( wxCommandEvent& );

    ~MyCanvasStToolContr(){}
};

//----------------------------------------------------------------------------
// MyCircle
//----------------------------------------------------------------------------
class MyCircle: public a2dCircle
{
public:
    MyCircle();

    MyCircle( double x, double y, double radius );

    MyCircle( const MyCircle& ori, CloneOptions options, a2dRefMap* refs );

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void OnEnterObject( a2dCanvasObjectMouseEvent& event );
    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    DECLARE_EVENT_TABLE()

    DECLARE_DYNAMIC_CLASS( MyCircle )

    static a2dPropertyIdString* PROPID_tipproperty;
    static a2dPropertyIdString* PROPID_node_stats;

    DECLARE_PROPERTIES()

};


//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

class MyDrawingPart: public a2dDrawingPartTiled
{
public:

    MyDrawingPart( int width = 1000, int height = 1000 ): a2dDrawingPartTiled( width, height ) {} 

    void PaintBackground( int x, int y, int width, int height );

};

//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------
#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
    #define with_GLCANVASdraw 1
#endif

class MyFrame: public wxFrame
{
public:
    // constructors and destructors
    MyFrame( wxWindow* parent, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_FRAME_STYLE );
    ~MyFrame();

    //! if wanted add a tool controller
    a2dSmrtPtr<MyCanvasStToolContr> m_contr;

#if with_GLCANVASdraw
    a2dOglCanvas* m_canvas;
#else
    a2dCanvas*    m_canvas;
#endif


private:
    // WDR: method declarations for MyFrame
    void CreateMyMenuBar();

private:

    a2dImage*  m_smile1;
    a2dImage*  m_smile2;
    wxTimer*   m_timer;
    wxTimer*   m_timer2;

    wxButton*  m_button;

    wxMenuBar* m_menuBar;
    wxMenu*    m_edit_menu;
    wxMenu*    m_optionMenu;

    void FillDoc( wxCommandEvent& event );

    void FillDoc0( a2dCanvasObject* root );
    void FillDoc1( a2dCanvasObject* root );
    void FillDoc2( a2dCanvasObject* root );
    void FillDoc3( a2dCanvasObject* root );
    void FillDoc4( a2dCanvasObject* root );
    void FillDoc5( a2dCanvasObject* root );
    void FillDoc6( a2dCanvasObject* root );
    void FillDoc7( a2dCanvasObject* root );

    void OnQuit( wxCommandEvent& event );
    void OnCloseWindow( wxCloseEvent& event );

    void SetSwitchY( wxCommandEvent& event );

	void SetMasterTool( wxCommandEvent& event );

    void SetEditTool( wxCommandEvent& event );
    void SetRectangleTool( wxCommandEvent& event );
    void SetSelectTool( wxCommandEvent& event );
    void SetPolygonTool( wxCommandEvent& event );
    void SetDragTool( wxCommandEvent& event );
    void SetDragNewTool( wxCommandEvent& event );

    void EndTool( wxCommandEvent& event );

    void OnUndo( wxCommandEvent& event );

    void OnRedo( wxCommandEvent& event );
    void OnTimer( wxTimerEvent& event );
    void OnTimer2( wxTimerEvent& event );
    void OnLoad( wxCommandEvent& event );
    void OnSave( wxCommandEvent& event );
    void OnClear( wxCommandEvent& event );

    void OnDrawer( wxCommandEvent& event );

    void OnCopyShape(wxCommandEvent& event);
    void OnPasteShape(wxCommandEvent& event);
    void OnUpdatePasteShape( wxUpdateUIEvent& event  );
    void OnUpdateCopyShape( wxUpdateUIEvent& event  );

    void OnDragSimulate(wxCommandEvent& event);

private:

    void AddCmdToToolbar( const a2dMenuIdItem& cmdId );
    void ConnectCmdId( const a2dMenuIdItem& cmdId );

    void AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId );
    void AddCmdMenu( wxMenu* parentMenu, wxMenuItem* item );
    void RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId );
    void OnCmdMenuId( wxCommandEvent& event );

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
