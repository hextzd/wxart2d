///////////////////////////////////////////////////////////////////////////////
// Name:        canedit.cpp
// Author:      Klaas Holwerda
// Created:     XX/XX/XX
// Copyright:   2000 (c) Klaas Holwerda
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __A2DCANVASGDSEDIT_H__
#define __A2DCANVASGDSEDIT_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/sttool.h"
#include "wx/editor/editmod.h"
#include "tetostrucdlg.h"

enum
{
    ID_FILLDOC = wxID_HIGHEST + 1,
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
};

#if (wxART2D_USE_LUA == 1)
#include "wx/luawraps/luawrap.h"

typedef a2dLuaEditorFrame whichEditorFrame;
#else
typedef a2dEditorFrame whichEditorFrame;
#endif

//! sepcialized Frame for editor of a2dCanvas
class a2dCanvasTetoEditorFrame : public whichEditorFrame
{

public:

    DECLARE_DYNAMIC_CLASS( a2dCanvasTetoEditorFrame )

    //! this makes dynamic creation possible ( e.g. a derived a2dCanvasTetoEditorFrame )
    /*
        Can be used by connectors ( e.g. wxDrawingConnector ) to create
        a derived a2dCanvasTetoEditorFrame class using the ClassInfo.
        This makes one connector class enough for all derived a2dCanvasTetoEditorFrame's.
        This constructor is used in combination with Create() to initilize the a2dCanvasTetoEditorFrame.
    */
    a2dCanvasTetoEditorFrame() {}

    a2dCanvasTetoEditorFrame( bool isParent, wxFrame* parent,
                              const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE );

    bool Create( bool isParent, wxFrame* parent,
                 const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE );
    ~a2dCanvasTetoEditorFrame();
protected:

    void OnTheme( a2dEditorFrameEvent& themeEvent );

    void OnInit( a2dEditorFrameEvent& initEvent );

    void OnPostLoadDocument( a2dDocumentEvent& event );

    void Quit( wxCommandEvent& event );

    void OnDrawer( wxCommandEvent& event );

    void OnMasterToolOption( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    bool m_editMode;

    DECLARE_EVENT_TABLE()
};


#endif //__A2DCANVASGDSEDIT_H__

