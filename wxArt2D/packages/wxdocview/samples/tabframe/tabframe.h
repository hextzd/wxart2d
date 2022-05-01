/*! \file docview/samples/tabframe/tabframe.h
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: tabframe.h,v 1.6 2009/07/31 15:41:14 titato Exp $
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

#include "wx/notebook.h"
#include "view.h"

#include <wx/image.h>
#include <wx/file.h>
#include <wx/log.h>

#include <wx/aui/auibook.h>
//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// MyNotebook
//----------------------------------------------------------------------------
#define AUI_NOTEBOOK 1
#ifdef AUI_NOTEBOOK
typedef wxAuiNotebook NoteBookType;
#else
typedef wxNotebook NoteBookType;
#endif

//! only to demo that a derived notebook can be used inside a a2dDocumentFrame
//! Notice that the focus on the pages automatically will set a2dDocumentFrame its m_view.
class MyNotebook : public NoteBookType
{
public:

    MyNotebook( a2dDocumentFrame* parent );

    ~MyNotebook() {}

#ifdef AUI_NOTEBOOK
    void OnChangedPage( wxAuiNotebookEvent& evt );
#else
    void OnChangedPage( wxNotebookEvent& evt );
#endif

#ifdef AUI_NOTEBOOK
    void OnTabBeginDrag( wxCommandEvent& evt );
    void OnTabDragMotion( wxCommandEvent& evt );
    void OnTabEndDrag( wxCommandEvent& evt );
    bool CaptionHit();
#endif

private:

    bool m_dragToFrame;
    EditorFrame* m_dragViewFrame;

    DECLARE_EVENT_TABLE()
};


typedef wxNotebookConnector<MyNotebook, a2dDocumentViewScrolledWindow> MyNoteBookConnector;

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

    MyFrame( bool parentFrame, MyNoteBookConnector* connector, DrawingView* view, wxFrame* parent, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             const long style = wxDEFAULT_FRAME_STYLE );

    ~MyFrame();

private:

    void OnActivate( wxActivateEvent& event );

    void OnCloseView( a2dCloseViewEvent& event );

    void OnAbout( wxCommandEvent& event );

    void OnCloseWindow( wxCloseEvent& event );

    void OnMRUFile( wxCommandEvent& event );

    MyNotebook* m_notebook;

    MyNoteBookConnector* m_notebookconnector;


    DECLARE_EVENT_TABLE()
};



// MyApp

class MyApp: public wxApp
{
public:
    virtual bool OnInit();

    virtual int OnExit();

    MyNoteBookConnector* m_notebookconnector;

private:

    MyFrame* m_frame;

};
#endif
