/*! \file editor/samples/viewmul/docv.h
    \brief Document/view demo
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: docv.h,v 1.8 2006/12/13 21:43:21 titato Exp $
*/

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifndef __DOCVIEWSAMPLEH__
#define __DOCVIEWSAMPLEH__

#include "wx/editor/editmod.h"
#include "document.h"

class a2dMultiDrawingConnector;
class wxTextConnector;

// Define a new application
class MyApp: public wxApp
{
public:
    MyApp( void );
    bool OnInit( void );
    int OnExit( void );

public:

    a2dSmrtPtr<a2dMultiDrawingConnector> m_connector;
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

	void OnAddDocument( a2dCommandEvent& event );
	void OnRemoveDocument( a2dCommandEvent& event );

    //! when clicked on an Open file in the overview, open an extra View of it.
    void OnMouseEvent( wxMouseEvent& event );

    //! single controller with a zoom.
    a2dSmrtPtr<a2dStToolContr> m_contr;

    DECLARE_EVENT_TABLE()
};

// Define a new frame
class MyFrame: public a2dDocumentFrame
{
    DECLARE_CLASS( MyFrame )
public:

    MyFrame( wxFrame* frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size,
             const long type );

    OverviewCanvas* GetOverViewCanvas() { return m_topcanvas; }

    ~MyFrame();

    void OnCloseWindow( wxCloseEvent& event );

    wxMenu* editMenu;

    OverviewCanvas* m_topcanvas;

    a2dSmrtPtr<msDocument> m_document;

    void OnAbout( wxCommandEvent& event );

    DECLARE_EVENT_TABLE()
};



#endif
