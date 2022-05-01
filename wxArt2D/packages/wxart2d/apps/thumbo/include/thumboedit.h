///////////////////////////////////////////////////////////////////////////////
// Name:        canedit.cpp
// Author:      Klaas Holwerda
// Created:     XX/XX/XX
// Copyright:   2000 (c) Klaas Holwerda
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __A2DCANVASGDSEDIT_H__
#define __A2DCANVASGDSEDIT_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/notebook.h"
#include <wx/aui/auibook.h>

#include "wx/canvas/canmod.h"
#include "wx/canvas/sttool.h"
#include "wx/editor/editmod.h"

class a2dCanvasThumboEditorFrame;
class wxDrawingConnector;

//----------------------------------------------------------------------------
// ThumboNotebook
//----------------------------------------------------------------------------
typedef wxAuiNotebook NoteBookType;

//! only to demo that a derived notebook can be used inside a a2dDocumentFrame
//! Notice that the focus on the pages automatically will set a2dDocumentFrame its m_view.
class ThumboNotebook : public NoteBookType
{
public:

    ThumboNotebook( a2dDocumentFrame* parent );

    ~ThumboNotebook() {}

    bool IsClosingTab() { return m_busyClosingTab; }

private:

    void OnSetFocus( wxFocusEvent& focusevent );
    void OnKillFocus( wxFocusEvent& focusevent );

    void OnChangedPage( wxAuiNotebookEvent& evt );
    void OnChangingPage( wxAuiNotebookEvent& evt );
    void OnPageClose( wxAuiNotebookEvent& evt );
    //void OnPageClose( wxNotebookEvent& evt );
    void OnTabBeginDrag( wxCommandEvent& evt );
    void OnTabDragMotion( wxCommandEvent& evt );
    void OnTabEndDrag( wxCommandEvent& evt );
    bool CaptionHit();
	void OnCloseView( a2dCloseViewEvent& event );

    bool m_dragToFrame;

    bool m_busyClosingTab;

    a2dCanvasThumboEditorFrame* m_dragViewFrame;

    DECLARE_EVENT_TABLE()
};


typedef wxNotebookConnector<ThumboNotebook, a2dDocumentViewScrolledWindow> ThumboNotebookConnector;

//----------------------------------------------------------------------------
// a2dCanvasThumboEditorFrame
//----------------------------------------------------------------------------

#if (wxART2D_USE_LUA == 1)
#include "wx/luawraps/luawrap.h"

typedef a2dLuaEditorFrame whichEditorFrame;
#else
typedef a2dEditorFrame whichEditorFrame;
#endif


//! sepcialized Frame for editor of a2dCanvas
class a2dCanvasThumboEditorFrame : public whichEditorFrame
{

public:

    DECLARE_DYNAMIC_CLASS( a2dCanvasThumboEditorFrame )

    //! this makes dynamic creation possible ( e.g. a derived a2dCanvasThumboEditorFrame )
    /*
        Can be used by connectors ( e.g. wxDrawingConnector ) to create
        a derived a2dCanvasThumboEditorFrame class using the ClassInfo.
        This makes one connector class enough for all derived a2dCanvasThumboEditorFrame's.
        This constructor is used in combination with Create() to initilize the a2dCanvasThumboEditorFrame.
    */
    a2dCanvasThumboEditorFrame( wxDrawingConnector* connector = NULL )
    {
        m_connector = connector;
    }

    a2dCanvasThumboEditorFrame( bool isParent, wxDrawingConnector* connector, wxFrame* parent,
                                const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE );

    ~a2dCanvasThumboEditorFrame();

    ThumboNotebook* GetNoteBook() { return m_notebook; }

protected:

    void CreateThemeXRC();

    void OnChangeFilename( a2dViewEvent& event );

    void OnActivate( wxActivateEvent& event );

    void OnCloseWindow( wxCloseEvent& event );

    void OnTheme( a2dEditorFrameEvent& themeEvent );

    void OnInit( a2dEditorFrameEvent& initEvent );

    void OnPostLoadDocument( a2dDocumentEvent& event );

    void Quit( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    void OnComEvent( a2dComEvent& event );

    wxDrawingConnector* m_connector;

    ThumboNotebook* m_notebook;

    DECLARE_EVENT_TABLE()
};


#endif //__A2DCANVASGDSEDIT_H__

