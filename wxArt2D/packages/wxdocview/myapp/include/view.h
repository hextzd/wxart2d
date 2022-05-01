/*! \file docview/samples/common/view.h
    \brief View classes
    \author Klaas Holwerda
    \date Created 04/01/98

    Copyright: (c) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: view.h,v 1.3 2007/05/30 20:14:34 titato Exp $
*/

#ifndef __VIEWSAMPLEH__
#define __VIEWSAMPLEH__

#include "wx/docview/docviewref.h"
#include "doc.h"

#include <wx/mstream.h>
#include <wx/hashmap.h>

class DrawingView: public a2dView
{
    DECLARE_DYNAMIC_CLASS( DrawingView )

public:

    DrawingView();
    ~DrawingView( void ) {};

    // Just for fun, to demo.
    bool ProcessEvent( wxEvent& event );

    void OnDraw( wxDC* dc );
    void OnUpdate( a2dDocumentEvent& event );
    void OnUpdate2( wxFocusEvent& event );

    void SetDrawingColour( const wxColour& drawingcolor ) { m_drawingcolor = drawingcolor; }
    void SetDrawingWidth( int penWidth ) { m_penWidth = penWidth; }

    int GetDrawingWidth() { return m_penWidth; }

    void SetScaler( double scale ) { m_scaler = scale; }
    double GetScaler() { return m_scaler; }

private:

    void OnMouseEvent( wxMouseEvent& event );

    wxColour m_drawingcolor;
    int m_penWidth;

    double m_scaler;

    DECLARE_EVENT_TABLE()
};

//! specialized view which knows how to display a DrawingDocument
//! as Text.
//! Updates write directly the document data into the display window which is a text control.
//! This will automatically cause a refresh of the window.
//! This view does not need a local storage,
class TextView: public a2dView
{
    DECLARE_EVENT_TABLE()

public:

    TextView(): a2dView() {}

    ~TextView( void ) {}

    //! the view is flagged for redisplay because document did change.
    void OnUpdate( a2dDocumentEvent& event );

protected:

    void OnDraw( wxDC* dc ) {};

    //! just for demo
    void OnIdle( wxIdleEvent& event );

    DECLARE_DYNAMIC_CLASS( TextView )

private:


};

// wxString colour keys and int count of colour
WX_DECLARE_STRING_HASH_MAP( int, ColourHash );


//! extended frame with some standard menu's.
//! It display a TextView using a wxTextctrl in text.
class wxTextViewFrame : public a2dDocumentFrame
{
    DECLARE_EVENT_TABLE()

public:

    wxTextViewFrame( TextView* view, const wxPoint& pos, const wxSize& size, wxFrame* parent );

    wxTextCtrl* m_textdisplay;

private:

    void OnSetMenuStrings( a2dCommandProcessorEvent& event );

    TextView* m_view;

    wxMenu* m_edit_menu;

};


//! specialized view which knows how to display
//! the number of object in the DrawingDocument + all colors used.
class StatisticsView: public a2dView
{
public:

    StatisticsView(): a2dView() { m_objectcount = 0; }

    ~StatisticsView( void ) {}

    //! the view is flagged for redisplay because document did change.
    void OnUpdate( a2dDocumentEvent& event );

    //! used for printing the view and also
    //! the view will (re)display what view needs to show.
    void OnDraw( wxDC* dc );

protected:

    int m_objectcount;

    ColourHash m_usedcolours;

    DECLARE_DYNAMIC_CLASS( StatisticsView )

    DECLARE_EVENT_TABLE()

private:

};

//----------------------------------------------------------------------------
// a2dMultiFrameDrawingConnector
//----------------------------------------------------------------------------

// ID for the menu commands
extern const long EditorFrame_CUT;
extern const long EditorFrame_ABOUT;
extern const long EditorFrame_DRAWINGCOLOUR;
extern const long EditorFrame_DRAWINGWIDTH;
extern const long EditorFrame_DRAWINGSCALE;
extern const long EditorFrame_WINDOWWVIEW;
extern const long EditorFrame_ADDVIEW;
extern const long EditorFrame_CHOOSEVIEW;
extern const long EditorFrame_CONNECTDOC;

class EditorFrame;

//! extend a2dFrameViewConnector to set initial mapping in world coordinates for the view.
//! In principle it sets the initial values for the a2dView.
class a2dMultiFrameDrawingConnector: public a2dFrameViewConnector
{
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS( a2dMultiFrameDrawingConnector )

public:

    a2dMultiFrameDrawingConnector( wxClassInfo* EditorClassInfo = ( wxClassInfo* ) NULL );

    ~a2dMultiFrameDrawingConnector();

private:

    void OnPostCreateDocument( a2dTemplateEvent& event );
    void OnPostCreateView( a2dTemplateEvent& event );

    //! For dynamic creation of appropriate instances.
    wxClassInfo*      m_editorClassInfo;
};

//! example of having a single frame with several documents open.
class EditorFrame: public a2dDocumentFrame
{

    DECLARE_DYNAMIC_CLASS( EditorFrame )

public:

    EditorFrame() {};

    EditorFrame( bool isParent,
                 wxFrame* parent,
                 DrawingView* window,
                 wxWindowID id, const wxString& title,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 const long style = wxDEFAULT_FRAME_STYLE );

    virtual bool Create( bool isParent,
                         wxFrame* parent,
                         DrawingView* view,
                         wxWindowID id, const wxString& title,
                         const wxPoint& pos = wxDefaultPosition,
                         const wxSize& size = wxDefaultSize,
                         const long style = wxDEFAULT_FRAME_STYLE );


    ~EditorFrame();

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

    void OnCut( wxCommandEvent& event );

    void OnSelectDrawingColour( wxUpdateUIEvent& event );
    void OnSetDrawingWidth( wxUpdateUIEvent& event );
    void OnSetDrawingZoom( wxUpdateUIEvent& event );

    void CreateMyMenuBar();

    void OnExit( wxCommandEvent& event );

    void OnActivate( wxActivateEvent& event );
    void OnActivateViewSentFromChild( a2dViewEvent& viewevent );

    void OnUndo( wxCommandEvent& event );
    void OnRedo( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    //! just for demo, simple Destroy the frame, as would be default when not intercepted.
    void OnCloseWindow( wxCloseEvent& event );

    void OnSetMenuStrings( a2dCommandProcessorEvent& event );

    a2dDocumentViewScrolledWindow* m_canvas;

    wxMenuBar* m_menu_bar;

    wxMenu* m_file_menu;
    wxMenu* m_edit_menu;
    wxMenu* m_drawing_menu;
    wxMenu* m_view_menu;
    wxMenu* m_document_menu;
    wxMenu* m_help_menu;

    a2dSmrtPtr<a2dScrolledWindowViewConnector> m_singleconnector;
    a2dSmrtPtr<a2dDocumentTemplate> m_editordoctemplatenew;

    DECLARE_EVENT_TABLE()
};

#endif
