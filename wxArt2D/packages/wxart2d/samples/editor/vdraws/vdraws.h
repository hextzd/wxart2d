/*! \file editor/samples/vdraws/vdraw.h
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: vdraws.h,v 1.15 2009/09/26 19:01:19 titato Exp $
*/
// For compilers that support precompilation, includes "wx/wx.h".

#ifndef __test_H__
#define __test_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/editor/editmod.h"

#include <wx/image.h>
#include <wx/file.h>
#include <wx/log.h>

#if wxART2D_USE_OPENGL && wxUSE_GLCANVAS 
#define with_GLCANVASdraw 1
#endif

class MyFrame;

//! extend a2dWindowViewConnector  to set initial mapping in world coordinates for the view.
/*! In principle it sets the initial values for the a2dView.
    Next to that it swicthes the toolcontroller to this view
*/
class a2dConnector: public a2dViewConnector
{
    DECLARE_EVENT_TABLE()

public:

    a2dConnector();
    ~a2dConnector();

    void OnPostCreateView( a2dTemplateEvent& event );
    void OnPreAddCreatedDocument( a2dTemplateEvent& event );
    void OnPostCreateDocument( a2dTemplateEvent& event );
    void OnDisConnectView(  a2dTemplateEvent& event );

	MyFrame* m_frame;

    a2dCanvasView* m_view;

    a2dDocumentPtr m_newdoc;
};

//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------
#define DOCCANVAS_ABOUT  1

// ID for the menu commands
enum
{
    MDSCREEN_T  = wxID_HIGHEST + 2000,
    MDSCREEN_FIRST,
    MDSCREEN_E = MDSCREEN_FIRST,
    MDSCREEN_0,
    MDSCREEN_1,
    MDSCREEN_2,
    MDSCREEN_3,
    MDSCREEN_4,
    MDSCREEN_5,
    MDSCREEN_6,
    MDSCREEN_10,
    MDSCREEN_16,
    MDSCREEN_17,
    MDSCREEN_18,
    MDSCREEN_19,
    MDSCREEN_20,
    MDSCREEN_21,
    MDSCREEN_22,
    MDSCREEN_23,
    MDSCREEN_24,
    MDSCREEN_LAST = MDSCREEN_24,
    MasterToolOption,
    DrawerOption_First,
    Switch_Drawer_aggRgba = DrawerOption_First,
    Switch_Drawer_agg,
    Switch_Drawer_agggc,
    Switch_Drawer_dc,
    Switch_Drawer_gdiplus,
    Switch_Drawer_dcgc,
    Switch_Drawer_gdigc,
    DrawerOption_Last = Switch_Drawer_gdigc,
    CANVAS_ABOUT,
    DRAWER_CHANGE
};

//! example fo havin a single frame with several documents open.
class MyFrame: public a2dEditorFrame
{
public:

    MyFrame( wxFrame* parent,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_FRAME_STYLE );
    ~MyFrame();

    void FillData( wxCommandEvent& event );

    a2dCanvasObjectPtr m_topobject;


private:

    void OnInit( a2dEditorFrameEvent& initEvent );

    void OnTheme( a2dEditorFrameEvent& themeEvent );

    void OnCloseView( a2dCloseViewEvent& event );

    void OnMRUFile( wxCommandEvent& event );

    void OnCloseWindow( wxCloseEvent& event );

    void OnDrawer( wxCommandEvent& event );

    DECLARE_EVENT_TABLE()
};



// MyApp

class MyApp: public wxApp
{
public:
    virtual bool OnInit();

    virtual int OnExit();

    wxFrame* m_over;

    a2dSmrtPtr<a2dConnector> m_singleconnector;
};

#endif
