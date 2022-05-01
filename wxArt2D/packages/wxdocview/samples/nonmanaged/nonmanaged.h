/*! \file docview/samples/nonmanaged/nonmanaged.h
    \author XX

    Copyright: 2001-2004 (C) XX

    Licence: wxWidgets Licence

    RCS-ID: $Id: nonmanaged.h,v 1.1 2007/01/17 21:47:41 titato Exp $
*/

#ifndef __nonmanaged_H__
#define __nonmanaged_H__

// Include wxWindows' headers

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/docview/docviewref.h"
#include "view.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------


// ID for the menu commands
enum
{
    WXDOCVIEW_CUT = wxID_HIGHEST + 100,
    WXDOCVIEW_ABOUT,
    WXDOCVIEW_DRAWINGCOLOUR,
    WXDOCVIEW_DRAWINGWIDTH,
    ID_QUIT
};

//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

class MyFrame: public a2dDocumentFrame
{
public:

    MyFrame( wxFrame* parent, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_FRAME_STYLE );
    ~MyFrame();

    wxMenu* m_edit_menu;

private:

    void CreateMyMenuBar();

    void OnQuit( wxCommandEvent& event );

    void OnCloseView( a2dCloseViewEvent& event );

    void OnCloseWindow( wxCloseEvent& event );

    void OnUpdateUndo( wxUpdateUIEvent& event );
    void OnUpdateRedo( wxUpdateUIEvent& event );

    //! handler for event to set undo redo menustrings
    void OnSetmenuStrings( a2dCommandProcessorEvent& event );

    void OnAbout( wxCommandEvent& WXUNUSED( event ) );

    void OnUndo( wxCommandEvent& event );
    void OnRedo( wxCommandEvent& event );

    void SelectDrawingColour( wxCommandEvent& event );

    void SetDrawingWidth( wxCommandEvent& event );

private:

    a2dSmrtPtr<DrawingView> m_view;
    a2dDocumentViewScrolledWindow* m_outputwindow;

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
