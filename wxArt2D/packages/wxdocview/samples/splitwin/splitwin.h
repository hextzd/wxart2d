/*! \file docview/samples/splitwin/splitwin.h
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: splitwin.h,v 1.2 2007/07/17 15:49:07 titato Exp $
*/
// For compilers that support precompilation, includes "wx/wx.h".

#ifndef __test_H__
#define __test_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/docview/docviewref.h"
#include "view.h"
#include "doc.h"

#include "wx/splitter.h"
#include "view.h"

#include <wx/image.h>
#include <wx/file.h>
#include <wx/log.h>

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// MySplitterWindow
//----------------------------------------------------------------------------

class MySplitterWindow : public wxSplitterWindow
{
public:
    MySplitterWindow( wxFrame* parent );

    // event handlers
    void OnPositionChanged( wxSplitterEvent& event );
    void OnPositionChanging( wxSplitterEvent& event );
    void OnDClick( wxSplitterEvent& event );
    void OnUnsplit( wxSplitterEvent& event );

    a2dDocumentViewScrolledWindow* GetWindow1() { return ( a2dDocumentViewScrolledWindow* ) wxSplitterWindow::GetWindow1(); }
    a2dDocumentViewScrolledWindow* GetWindow2() { return ( a2dDocumentViewScrolledWindow* ) wxSplitterWindow::GetWindow2(); }

private:
    wxFrame* m_frame;

    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
// wxDrawingConnector
//----------------------------------------------------------------------------
class MyFrame;


//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

// ID for the menu commands
enum
{
    WXDOCVIEW_ABOUT11 = wxID_HIGHEST + 200,
    WXDOCVIEW_CONNECTWINDOW,
    WXDOCVIEW_CONNECTWINDOW1,
    WXDOCVIEW_CONNECTWINDOW2,
};

//! example of having a single frame with several documents open.
class MyFrame: public EditorFrame
{
public:

    MyFrame( a2dSplitterConnector<a2dDocumentViewScrolledWindow>* connector, wxFrame* parent, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             const long style = wxDEFAULT_FRAME_STYLE );

    ~MyFrame();

    void SetConnectWindow( wxCommandEvent& event );

private:

    void OnCloseView( a2dCloseViewEvent& event );

    void OnAbout( wxCommandEvent& event );

    void OnCloseWindow( wxCloseEvent& event );

    void OnMRUFile( wxCommandEvent& event );

    MySplitterWindow* m_splitter;

    a2dSplitterConnector<a2dDocumentViewScrolledWindow>* m_splitconnector;

    DECLARE_EVENT_TABLE()
};



// MyApp

class MyApp: public wxApp
{
public:
    virtual bool OnInit();

    virtual int OnExit();

private:

    MyFrame* m_frame;

};

#endif
