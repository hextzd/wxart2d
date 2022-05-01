/*! \file editor/samples/editmdi/editmdi.h
    \author Klaas Holwerda

    Copyright: (C) 1998, Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: editmdi.h,v 1.2 2008/07/19 18:29:51 titato Exp $
*/
// For compilers that support precompilation, includes "wx/wx.h".

#ifndef __editmdi_H__
#define __editmdi_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/docview/docviewref.h"
#include "wx/docview/docmdiref.h"
#include "wx/canvas/canmod.h"
#include "wx/editor/editmod.h"

#include <wx/file.h>
#include <wx/log.h>

class MyCanvasDocument : public a2dCanvasDocument
{
    DECLARE_EVENT_TABLE()

    DECLARE_DYNAMIC_CLASS( MyCanvasDocument );

public:

    MyCanvasDocument();

    //! destructor
    ~MyCanvasDocument();

    //! constructor with other document
    MyCanvasDocument( const MyCanvasDocument& other );

    //! create exact copy
    MyCanvasDocument* Clone();

    //! called when a new document is created, it is filled with something default.
    void OnNewDocument( a2dDocumentEvent& event );

    //! to demo that here we want to close the document after closing the last view on it.
    void OnRemoveView( a2dDocumentEvent& event );

};

class a2dCanMdiFrameDrawingConnector: public a2dViewConnector
{
    DECLARE_EVENT_TABLE()

public:

    //! Constructor
    /*
        \param documentManager maintaining a pool of a2dDocumentTemplate's, a2dViewTemplate's
               a2dDocument's and their a2dView's
    */
    a2dCanMdiFrameDrawingConnector( wxClassInfo* EditorClassInfo = ( wxClassInfo* ) NULL );

    ~a2dCanMdiFrameDrawingConnector();

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
    EditorFrame_CUT = wxID_HIGHEST + 100,
    EditorFrame_ABOUT,
    EditorFrame_ZOOM_OUT,
    EditorFrame_STYLE_DLG,
    SHOW_TOOL_DLG,
    SET_POLYTOOL,
    SET_POLYLINETOOL,
    SET_POLYLINEWIRETOOL,
    SET_EDITTOOL,
    SET_RECTTOOL,
    SET_DRAGTOOL,
    SET_COPYTOOL,
    SET_DELETETOOL,
    SET_DRAGNEWTOOL,
    SET_MASTERTOOL,
    SET_TEXTTOOL,
    SET_SELECTTOOL,
    SET_FIXEDSTYLE,
    END_TOOL,
    REFRESH
};


//! example of having a single frame with several documents open.
class MDIEditorFrame: public a2dDocumentMDIChildFrame
{

    DECLARE_DYNAMIC_CLASS( MDIEditorFrame )

public:

    MDIEditorFrame() {};

    MDIEditorFrame(
        wxMDIParentFrame* parent,
        wxWindowID id, const wxString& title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        const long style = wxDEFAULT_FRAME_STYLE );

    ~MDIEditorFrame();

    void Refresh( wxCommandEvent& event );

    wxMenu* GetEditMenu() { return m_edit_menu; }

    void Activate( bool activate = true );

    void Zoomout( wxCommandEvent& event );

    void StyleDlg( wxCommandEvent& WXUNUSED( event ) );

    void SetPolygonTool( wxCommandEvent& event );

    void SetPolylineTool( wxCommandEvent& event );

    void SetPolylineWireTool( wxCommandEvent& event );

    void SetDragTool( wxCommandEvent& event );

    void SetCopyTool( wxCommandEvent& event );

    void SetDeleteTool( wxCommandEvent& event );

    void SetEditTool( wxCommandEvent& event );

    void SetRectangleTool( wxCommandEvent& event );

    void SetDragNewTool( wxCommandEvent& event );

    void SetMasterTool( wxCommandEvent& event );

    void SetTextTool( wxCommandEvent& event );

    void SetSelectTool( wxCommandEvent& event );

    void EndTool( wxCommandEvent& event );

    a2dViewCanvas* GetCanvas() { return m_canvas; }

protected:

    void Init();

    void CreateMyMenuBar();

    void OnExit( wxCommandEvent& event );

    void OnActivate( wxActivateEvent& event );

    void OnMenu( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    void OnPostLoadDocument( a2dDocumentEvent& event );

    //! just for demo, simple Destroy the frame, as would be default when not intercepted.
    void OnCloseWindow( wxCloseEvent& event );

    //! handler for event to set undo redo menustrings
    void OnSetmenuStrings( a2dCommandProcessorEvent& event );

    void OnUndo( wxCommandEvent& event );

    void OnRedo( wxCommandEvent& event );

    wxMenuBar* m_menu_bar;

    wxMenu* m_file_menu;
    wxMenu* m_edit_menu;
    wxMenu* m_drawing_menu;
    wxMenu* m_view_menu;
    wxMenu* m_document_menu;
    wxMenu* m_help_menu;

    //! if wanted add a tool controller
    a2dSmrtPtr<a2dStToolContr> m_contr;

    a2dViewCanvas* m_canvas;

    bool m_fixedToolStyle;


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

    wxMenu* m_file_menu;

private:

    void CreateMyMenuBar();

    void OnExit( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    void OnCloseWindow( wxCloseEvent& event );

    void OnShowToolDlg( wxCommandEvent& event );

    void OnUpdateShowToolDlg( wxUpdateUIEvent& event );

    ToolDlg* m_tooldlg;

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

    a2dSmrtPtr<a2dCanMdiFrameDrawingConnector> m_mainconnector;
};


#endif
