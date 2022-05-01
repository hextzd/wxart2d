/*! \file editor/samples/vdrawm/docv.h
    \brief Document/view demo
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: vdrawm.h,v 1.1 2007/01/17 21:35:17 titato Exp $
*/

#ifndef __DOCVIEWSAMPLEH__
#define __DOCVIEWSAMPLEH__

#include "wx/editor/editmod.h"
#include "wx/docview/doccom.h"
#include "wx/editor/canpropedit.h"

class wxDrawingConnector;
class wxTextConnector;
class a2dVdrawDocManager;

// Define a new application
class MyApp: public wxApp
{
public:
    MyApp( void );
    bool OnInit( void );
    int OnExit( void );

protected:

    a2dSmrtPtr<wxDrawingConnector> m_connector;
    a2dSmrtPtr<wxTextConnector> m_connector2;
};

//GLOBAL use of wxGetApp()
DECLARE_APP( MyApp )


//! Overview canvas  in toplevel frame for showing all open drawings "symbolic"
class OverviewCanvas: public a2dCanvas
{
public:

    OverviewCanvas(  wxFrame* frame,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxScrolledWindowStyle );

    ~OverviewCanvas();

    //! Event Add a document to the manager's list
    /*
    */
    void OnAddDocument( a2dCommandEvent& event );

    //! Event remove a document from the manager's list
    /*
    */
    void OnRemoveDocument( a2dCommandEvent& event );

    //! when clicked on an Open file in the overview, open an extra View of it.
    void OnMouseEvent( wxMouseEvent& event );

    //! single controller with a zoom.
    a2dSmrtPtr<a2dStToolContr> m_contr;

    DECLARE_EVENT_TABLE()
};

// ID for the menu commands
enum
{
    MSCREEN_T  = wxID_HIGHEST + 1000,
    MSCREEN_FIRST,
    MSCREEN_E = MSCREEN_FIRST,
    MSCREEN_0,
    MSCREEN_1,
    MSCREEN_2,
    MSCREEN_3,
    MSCREEN_4,
    MSCREEN_5,
    MSCREEN_6,
    MSCREEN_7,
    MSCREEN_8,
    MSCREEN_9,
    MSCREEN_10,
    MSCREEN_11,
    MSCREEN_12,
    MSCREEN_13,
    MSCREEN_14,
    MSCREEN_15,
    MSCREEN_ALL,
    MSCREEN_LAST = MSCREEN_ALL,
    CANVAS_TOOLSDLG,
    CANVAS_STYLEDLG,
    CANVAS_ABOUT
};

//! Top Parent Frame
class VdrawFrame: public a2dDocumentFrame
{
    DECLARE_CLASS( VdrawFrame )

public:

    VdrawFrame( a2dDocumentCommandProcessor* manager, wxFrame* frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size,
                const long type );

    ~VdrawFrame();

    OverviewCanvas* GetOverViewCanvas() { return m_topcanvas; }

    void OnCloseWindow( wxCloseEvent& event );

    bool Destroy();

    void FillData( wxCommandEvent& event );

    void OnShowToolDlg( wxCommandEvent& event );

    void OnShowStyleDlg( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    a2dCanvasObjectPtr m_topgroup;

    ToolDlg* m_tooldlg;

    a2dStyleDialog* m_styledlg;

    a2dSmrtPtr<a2dCanvasDocument> m_document;

    wxMenu* editMenu;

    OverviewCanvas* m_topcanvas;

    DECLARE_EVENT_TABLE()
};

#endif
