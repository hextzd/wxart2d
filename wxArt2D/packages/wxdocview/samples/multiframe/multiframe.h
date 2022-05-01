/*! \file docview/samples/multiframe/multiframe.h
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: multiframe.h,v 1.2 2009/09/26 19:01:19 titato Exp $
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

#include <wx/file.h>
#include <wx/log.h>




class a2dMultiViewDocManager;

class wxOverViewWindow: public wxScrolledWindow
{
    DECLARE_EVENT_TABLE()

public:

    wxOverViewWindow( a2dMultiViewDocManager* docmanager, wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );

    ~wxOverViewWindow();

    void DrawDocumentsAndViews();

    void OnPaint( wxPaintEvent& event );

private:
    unsigned int m_oldcount;

    a2dMultiViewDocManager* m_docmanager;
};

//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

// ID for the menu commands
enum
{
    WXDOCVIEW_ABOUT = wxID_HIGHEST + 100,
};


class MyFrame: public EditorFrame
{
    DECLARE_DYNAMIC_CLASS( MyFrame )

public:

    MyFrame() {};

    MyFrame( wxFrame* parent, DrawingView* view, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             const long style = wxDEFAULT_FRAME_STYLE );

    bool Create( bool isParent, wxFrame* parent, DrawingView* view, wxWindowID id, const wxString& title,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 const long style = wxDEFAULT_FRAME_STYLE );

    ~MyFrame();

private:

    void OnAbout( wxCommandEvent& event );

    DECLARE_EVENT_TABLE()
};

//! example of having a single frame with several documents open.
class MyParentFrame: public a2dDocumentFrame
{
public:

    MyParentFrame( a2dMultiViewDocManager* manager, wxFrame* parent, wxWindowID id, const wxString& title,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   const long style = wxDEFAULT_FRAME_STYLE );

    ~MyParentFrame();

    void Refresh( bool eraseBackground = true, const wxRect* rect = ( const wxRect* ) NULL );

    wxOverViewWindow* m_outputwindow;

private:

    void CreateMyMenuBar();

    void OnExit( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    void OnCloseWindow( wxCloseEvent& event );

    void OnMenu( wxCommandEvent& event );

    DECLARE_EVENT_TABLE()
};

//! specialized document manager for overview of open files.
class a2dMultiViewDocManager: public a2dDocumentCommandProcessor
{

    DECLARE_DYNAMIC_CLASS( a2dMultiViewDocManager )

public:

    a2dMultiViewDocManager( long flags = a2dDEFAULT_DOCUMENT_FLAGS, bool initialize = true );

    ~a2dMultiViewDocManager();

    void SetParentWindow( MyParentFrame* parentFrameInApp ) { m_parentFrameInApp = parentFrameInApp; }

    void SetOverViewWindow( wxOverViewWindow* overview );

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
    wxOverViewWindow* m_overview;

    MyParentFrame* m_parentFrameInApp;
};


//! MyApp
class MyApp: public wxApp
{
public:
    virtual bool OnInit();

    virtual int OnExit();

private:

    MyParentFrame* m_frame;

protected:

    a2dMultiViewDocManager* m_docmanager;
    a2dSmrtPtr<a2dMultiFrameDrawingConnector> m_mainconnector;
};


#endif
