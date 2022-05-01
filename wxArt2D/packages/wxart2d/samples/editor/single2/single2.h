/*! \file editor/samples/single2/simple.h
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: single2.h,v 1.4 2009/07/28 19:47:16 titato Exp $
*/

#ifndef __simple_H__
#define __simple_H__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/editor/editmod.h"

//----------------------------------------------------------------------------
//   a2dConnector
//----------------------------------------------------------------------------

//! extend a2dWindowViewConnector  to set initial mapping in world coordinates for the view.
/*! In principle it sets the initial values for the a2dView.
    Next to that it swicthes the toolcontroller to this view
*/
class a2dConnector: public a2dViewConnector
{
    DECLARE_EVENT_TABLE()

public:
    
    //! window used by view to display itself.
    a2dCanvas* GetDisplayWindow() { return m_display; }

    //! get window used by view to display itself.
    void SetDisplayWindow( a2dCanvas* window ) { m_display = window; }

public:

    a2dConnector();

    void OnPostCreateView( a2dTemplateEvent& event );
    void OnPreAddCreateDocument( a2dTemplateEvent& event );
    void OnPostCreateDocument( a2dTemplateEvent& event );
    void OnDisConnectView(  a2dTemplateEvent& event );

    a2dCanvas* m_display;  
    
    a2dCanvasView* m_view;

    a2dDocumentPtr m_newdoc;

};

//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------
// #define SIMPLE

extern const long OPTION_SWITCHYAXIS;
extern const long DOCCANVAS_ABOUT;
extern const long SET_POLYTOOL;
extern const long SET_RECTTOOL;
extern const long END_TOOL;

class MyFrame: public a2dDocumentFrame
{
    DECLARE_CLASS( MyFrame )

public:

    typedef void ( MyFrame::*wxObjectEventFunctionM )( wxCommandEvent& );

public:


    MyFrame(  a2dConnector* connect, wxFrame* parent, wxWindowID id, const wxString& title,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              const long style = wxDEFAULT_FRAME_STYLE );
    ~MyFrame();

#ifdef SIMPLE
    a2dCanvasSim*  m_canvas;
#else
    a2dCanvas*  m_canvas;
#endif

    a2dSmrtPtr<a2dStToolContr> m_contr;

private:

     wxMenuBar* m_menuBar;
    wxMenu*    m_edit_menu;
    wxMenu*    m_optionMenu;

    void CreateMyMenuBar();

    void SetSwitchY( wxCommandEvent& WXUNUSED( event ) );

    void SetPolygonTool( wxCommandEvent& event );
    void SetRectangleTool( wxCommandEvent& event );
    void SetDragNewTool( wxCommandEvent& event );
    void SetEditTool( wxCommandEvent& event );
    void EndTool( wxCommandEvent& event );
    void OnAbout( wxCommandEvent& event );

    void OnExit( wxCommandEvent& event );
    void OnCloseWindow( wxCloseEvent& event );

    void OnUndo( wxCommandEvent& WXUNUSED( event ) );
    void OnRedo( wxCommandEvent& WXUNUSED( event ) );

    //! add a menu which executes a function
    /*!
        \param id wxID_ANY is non specific for use in func
        \param text text of menu
        \param helpString help for menu
        \param func function in this are derived class to be called.
    */
    void AddFunctionToMenu( int id, wxMenu* parentMenu, const wxString& text, const wxString& helpString, wxObjectEventFunctionM func, bool check = false );

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

protected:

};

DECLARE_APP( MyApp )

#endif
