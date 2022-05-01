/*! \file docview/samples/mdi/mdi.h
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: mdi.h,v 1.1 2007/01/17 21:47:35 titato Exp $
*/
// For compilers that support precompilation, includes "wx/wx.h".

#ifndef __test_H__
#define __test_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/docview/docviewref.h"
#include "wx/docview/docmdiref.h"
#include "view.h"
#include "doc.h"

#include <wx/file.h>
#include <wx/log.h>

class wxMdiFrameDrawingConnector: public a2dViewConnector
{
    DECLARE_EVENT_TABLE()

public:

    //! Constructor
    /*
        \param documentManager maintaining a pool of a2dDocumentTemplate's, a2dViewTemplate's
               a2dDocument's and their a2dView's
    */
    wxMdiFrameDrawingConnector( wxClassInfo* EditorClassInfo = ( wxClassInfo* ) NULL );

    ~wxMdiFrameDrawingConnector();

    //! The pointer m_docframe will be initialized.
    /*! An application can have only one Parent a2dDocumentMDIParentFrame in the application, which is the Frame which
        other Child a2dDocumentMDIChildFrame(s) have as Parent. The Parent Frame is in general created in
        bool wxApp::OnInit(void).

        \param docframe The parent frame of the whole application
    */
    void Init( a2dDocumentMDIParentFrame* docframe );

    //! get the frame
    a2dDocumentMDIParentFrame* GetDocFrame() { return m_docframe; }

private:

    void OnPostCreateDocument( a2dTemplateEvent& event );
    void OnPostCreateView( a2dTemplateEvent& event );

    //! For dynamic creation of appropriate instances.
    wxClassInfo*      m_editorClassInfo;

    //! the frame which receives events and often redirect those to the document manager.
    a2dDocumentMDIParentFrame* m_docframe;

};


//----------------------------------------------------------------------------
// MyFrame
//----------------------------------------------------------------------------

// ID for the menu commands
enum
{
    WXDOCVIEW_ABOUT = wxID_HIGHEST + 100
};

//! extended frame with some standard menu's.
//! It display a TextView using a wxTextctrl in text.
class wxMDITextViewFrame : public a2dDocumentMDIChildFrame
{

public:

    wxMDITextViewFrame( TextView* view, const wxPoint& pos, const wxSize& size, wxMDIParentFrame* parent );

    wxTextCtrl* m_textdisplay;

private:

    TextView* m_view;

};

class MyFrame: public a2dDocumentMDIChildFrame
{

public:

    MyFrame( a2dDocumentMDIParentFrame* parent, a2dView* view, wxWindowID id, const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             const long style = wxDEFAULT_FRAME_STYLE );

    bool Create( bool isParent, a2dDocumentMDIParentFrame* parent, DrawingView* view, wxWindowID id, const wxString& title,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 const long style = wxDEFAULT_FRAME_STYLE );

    ~MyFrame();

private:

    void OnAbout( wxCommandEvent& event );

    //! the frame which receives events and often redirect those to the document manager.
    a2dDocumentMDIParentFrame* m_docframe;

    a2dDocumentViewScrolledWindow* m_canvas;

    DECLARE_EVENT_TABLE()
};


//! example of having a single frame with several documents open.
class MDIEditorFrame: public a2dDocumentMDIChildFrame
{
    DECLARE_DYNAMIC_CLASS( MDIEditorFrame )

public:

    MDIEditorFrame() {};

    MDIEditorFrame(
        wxMDIParentFrame* parent,
        DrawingView* window,
        wxWindowID id, const wxString& title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        const long style = wxDEFAULT_FRAME_STYLE );

    virtual bool Create(
        wxMDIParentFrame* parent,
        DrawingView* view,
        wxWindowID id, const wxString& title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        const long style = wxDEFAULT_FRAME_STYLE );


    ~MDIEditorFrame();

    //! when connecting an existing document to current view, creating a new view if needed.
    virtual void ConnectDocument( a2dDocument* doc );

    void CreateAddView( wxCommandEvent& event );

    void CreateInWindowView( wxCommandEvent& event );

    void SelectDrawingColour( wxCommandEvent& event );

    void SetDrawingWidth( wxCommandEvent& event );

    void SetDrawingZoom( wxCommandEvent& event );

    void ChooseDocumentForView( wxCommandEvent& event );
    void ChooseViewFromDocument( wxCommandEvent& event );

    wxMenu* GetEditMenu() { return m_edit_menu; }

    a2dDocumentViewScrolledWindow* GetCanvas() { return m_canvas; }

protected:

    void Init( DrawingView* view );

    void OnSelectDrawingColour( wxUpdateUIEvent& event );
    void OnSetDrawingWidth( wxUpdateUIEvent& event );
    void OnSetDrawingZoom( wxUpdateUIEvent& event );

    void CreateMyMenuBar();

    void OnExit( wxCommandEvent& event );

    void OnActivate( wxActivateEvent& event );

    void OnUndo( wxCommandEvent& event );
    void OnRedo( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    //! just for demo, simple Destroy the frame, as would be default when not intercepted.
    void OnCloseWindow( wxCloseEvent& event );

    //! handler for event to set undo redo menustrings
    void OnSetmenuStrings( a2dCommandProcessorEvent& event );

    wxMenuBar* m_menu_bar;

    wxMenu* m_file_menu;
    wxMenu* m_edit_menu;
    wxMenu* m_drawing_menu;
    wxMenu* m_view_menu;
    wxMenu* m_document_menu;
    wxMenu* m_help_menu;

    a2dSmrtPtr<a2dScrolledWindowViewConnector> m_singleconnector;
    a2dSmrtPtr<a2dDocumentTemplate> m_editordoctemplatenew;

    a2dDocumentViewScrolledWindow* m_canvas;

    DECLARE_EVENT_TABLE()
};



//! example of having a single frame with several documents open.
class MyParentFrame: public a2dDocumentMDIParentFrame
{
public:

    MyParentFrame( wxFrame* parent, const wxString& title,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   const long style = wxDEFAULT_FRAME_STYLE );

    ~MyParentFrame();

    void Refresh( bool eraseBackground = true, const wxRect* rect = ( const wxRect* ) NULL );

    wxMenu* m_file_menu;

private:

    void CreateMyMenuBar();

    void OnExit( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    void OnCloseWindow( wxCloseEvent& event );

    DECLARE_EVENT_TABLE()
};


//! MyApp
class MyApp: public wxApp
{
public:
    virtual bool OnInit();

    virtual int OnExit();

private:

    MyParentFrame* m_frame;

    a2dSmrtPtr<wxMdiFrameDrawingConnector> m_mainconnector;

};


#endif
