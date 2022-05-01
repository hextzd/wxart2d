/*! \file editor/samples/viewmul/docv.h
    \brief Document/view demo
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: docv.h,v 1.1 2007/01/17 21:35:21 titato Exp $
*/

#ifndef __DOCVIEWSAMPLEH__
#define __DOCVIEWSAMPLEH__

#include "wx/editor/editmod.h"
#include "doc.h"

class a2dMultiDrawingConnector;
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

    a2dSmrtPtr<a2dMultiDrawingConnector> m_connector;
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

	void OnAddDocument( a2dCommandEvent& event );
	void OnRemoveDocument( a2dCommandEvent& event );

    //! when clicked on an Open file in the overview, open an extra View of it.
    void OnMouseEvent( wxMouseEvent& event );

    //! single controller with a zoom.
    a2dSmrtPtr<a2dStToolContr> m_contr;

    DECLARE_EVENT_TABLE()
};

//! specialized document manager for overview of open files.
class a2dVdrawDocManager: public a2dCentralCanvasCommandProcessor
{
    DECLARE_DYNAMIC_CLASS( a2dVdrawDocManager )

public:

    a2dVdrawDocManager( long flags = a2dDEFAULT_DOCUMENT_FLAGS, bool initialize = true );

    ~a2dVdrawDocManager();

    void SetOverViewCanvas( OverviewCanvas* overview );

    //! Add  a document to the manager's list
    /*
        And at the same time add it to the overview
    */
    virtual void AddDocument( a2dDocument* doc );

    //! remove a document from the manager's list
    /*
        And at the same time remove it from the overview
    */
    virtual void RemoveDocument( a2dDocument* doc );

protected:

    //! put overview icons of open files on here
    OverviewCanvas* m_overview;
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

    void OnAbout( wxCommandEvent& event );

    DECLARE_EVENT_TABLE()
};



#endif
