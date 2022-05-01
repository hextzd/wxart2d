/*! \file docview/samples/singleframe/singleframe.h
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: singleframe.h,v 1.1 2007/01/17 21:47:41 titato Exp $
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

#include <wx/image.h>
#include <wx/file.h>
#include <wx/log.h>


//! extend a2dWindowViewConnector  to set initial mapping in world coordinates for the view.
/*! In principle it sets the initial values for the a2dView.
    Next to that it swicthes the toolcontroller to this view
*/
class a2dSingleConnector: public a2dScrolledWindowViewConnector
{
    DECLARE_EVENT_TABLE()

public:

    a2dSingleConnector();

    void OnPostCreateView( a2dTemplateEvent& event );
    void OnDisConnectView(  a2dTemplateEvent& event );

    DrawingView* m_view;
};

//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

// ID for the menu commands
enum
{
    WXDOCVIEW_CUT = wxID_HIGHEST + 100,
    WXDOCVIEW_ABOUT,
    WXDOCVIEW_DRAWINGCOLOUR,
    WXDOCVIEW_DRAWINGWIDTH,
    WXDOCVIEW_CONNECTDOC
};

//! example of having a single frame with several documents open.
class MyFrame: public EditorFrame
{
public:

    MyFrame( wxFrame* parent, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             const long style = wxDEFAULT_FRAME_STYLE );

    ~MyFrame();

    a2dDocumentViewScrolledWindow* GetCanvas() { return m_canvas; }

private:

    void OnCloseView( a2dCloseViewEvent& event );


//    void OnMRUFile(wxCommandEvent& event);

    void OnAbout( wxCommandEvent& event );

    void OnCloseWindow( wxCloseEvent& event );

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

    a2dSmrtPtr<a2dSingleConnector> m_singleconnector;
    a2dSmrtPtr<a2dMultiFrameDrawingConnector> m_multiconnector;

};

#endif
