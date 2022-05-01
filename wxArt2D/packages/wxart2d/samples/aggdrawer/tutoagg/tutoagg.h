/*! \file aggdrawer/samples/tutoagg/simple.h
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: tutoagg.h,v 1.4 2008/07/19 18:29:50 titato Exp $
*/

#ifndef __simple_H__
#define __simple_H__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/editor/editmod.h"
#include "wx/aggdrawer/aggdrawer.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#define DOCCANVAS_ABOUT  1
#define SET_POLYTOOL 2
#define SET_EDITTOOL 3
#define SET_RECTTOOL 4
#define SET_ELLIPTOOL 7
#define SET_IMAGETOOL 8
#define SET_TEXTTOOL 9
#define SET_DELTOOL 10
#define END_TOOL 5
#define REFRESH 6

//----------------------------------------------------------------------------
// MyCircle
//----------------------------------------------------------------------------
class MyCircle: public a2dCircle
{
public:
    MyCircle( double x, double y, double radius );

    void OnMouseEvent( a2dCanvasObjectMouseEvent& event );

    DECLARE_EVENT_TABLE()

};

//! extend a2dWindowViewConnector  to set initial mapping in world coordinates for the view.
/*! In principle it sets the initial values for the a2dView.
    Next to that it swicthes the toolcontroller to this view
*/
class a2dPaintConnector: public  a2dViewConnector
{
    DECLARE_EVENT_TABLE()

public:

    a2dPaintConnector();

    //! window used by view to display itself.
    a2dCanvas* GetDisplayWindow() { return m_display; }

    //! get window used by view to display itself.
    void SetDisplayWindow( a2dCanvas* window ) { m_display = window; }

    void OnPostCreateView( a2dTemplateEvent& event );
    void OnPreAddCreateDocument( a2dTemplateEvent& event );
    void OnPostCreateDocument( a2dTemplateEvent& event );
    void OnDisConnectView(  a2dTemplateEvent& event );

    a2dCanvasView* m_view;
	a2dCanvas* m_display;
};


//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

class MyFrame: public a2dDocumentFrame
{
public:

    MyFrame( a2dCanvasView* canvasview, wxFrame* parent, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_FRAME_STYLE );
    ~MyFrame();

    a2dCanvas* GetCanvas() { return m_canvas; }

private:
    void CreateMyMenuBar();

private:

    wxMenu*    m_edit_menu;

    a2dCanvas*    m_canvas;

#if wxART2D_USE_EDITOR
    //! if wanted add a tool controller
    a2dSmrtPtr<a2dStToolContr> m_contr;
#endif //wxART2D_USE_EDITOR

private:

    void Refresh( wxCommandEvent& event );

    void SetPolygonTool( wxCommandEvent& event );
    void SetEditTool( wxCommandEvent& event );
    void SetRectangleTool( wxCommandEvent& event );
    void SetEllipseTool( wxCommandEvent& event );
    void SetImageTool( wxCommandEvent& event );
    void SetTextTool( wxCommandEvent& event );
    void SetDeleteTool( wxCommandEvent& event );
    void EndTool( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    void OnExit( wxCommandEvent& event );

    void OnCloseWindow( wxCloseEvent& event );

    void OnFileNew( wxCommandEvent& event );


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
