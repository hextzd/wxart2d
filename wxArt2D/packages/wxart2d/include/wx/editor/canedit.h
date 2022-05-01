/*! \file wx/editor/canedit.h
    \brief ready made editing class for a a2dCanvas and/or a2dCanvasView
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canedit.h,v 1.37 2009/09/26 19:01:05 titato Exp $
*/

#ifndef __A2DCANVASEDIT_H__
#define __A2DCANVASEDIT_H__


#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "a2dprivate.h"

#include "wx/canvas/canmod.h"
#include "wx/canvas/sttool.h"
#include "wx/canvas/edit.h"
#include "wx/editor/candoc.h"

//! Creates and Connect a wxEditorFrame + a2dCanvas with a view.
class A2DEDITORDLLEXP a2dEditorMultiFrameViewConnector : public a2dFrameViewConnector
{
    DECLARE_CLASS( a2dEditorMultiFrameViewConnector )
    DECLARE_EVENT_TABLE()

public:

    a2dEditorMultiFrameViewConnector( wxClassInfo* EditorClassInfo = ( wxClassInfo* ) NULL );

    void OnPostCreateDocument( a2dTemplateEvent& event );

    //! Creates a a2dEditorFrame and sets its its a2dCanvas as the DisplayWindow for the view.
    void OnPostCreateView( a2dTemplateEvent& event );

    //! For dynamic creation of appropriate instances.
    wxClassInfo*      m_editorClassInfo;
};

// ID for the menu commands
extern const long TOOLOPTION_FIRST;
extern const long TOOLOPTION_ONTOP;
extern const long TOOLOPTION_RECTANGLE;
extern const long TOOLOPTION_REDRAW;
extern const long TOOLOPTION_COPY;

extern const long TOOLOPTION_FILLED;
extern const long TOOLOPTION_WIRE;
extern const long TOOLOPTION_INVERT;
extern const long TOOLOPTION_WIRE_ZERO_WIDTH;
extern const long TOOLOPTION_INVERT_ZERO_WIDTH;
extern const long TOOLOPTION_LAST;


extern const long MSHOW_T;
extern const long MSHOW_FIRST;
extern const long MSHOW_LIBPOINTS;
extern const long MSHOW_LIB;
extern const long MSHOW_LAST;
//extern const long MTOOL_PlaceFromLibByRef;
//extern const long MTOOL_PlaceFromLibByCopy;

extern const long Layer_view;

extern const long a2dShapeClipboard_Copy;
extern const long a2dShapeClipboard_Paste;
extern const long a2dShapeClipboard_Drag;


class A2DEDITORDLLEXP a2dEditorFrame;

//! used to theme a a2dEditorFrame
/*!
    The event Object is a a2dEditorFrame, and it can be intercepted to
    overrule the default theming ( menu's menubar etc. )

    \ingroup docview docviewevents  events
*/
class A2DGENERALDLLEXP a2dEditorFrameEvent: public wxEvent
{
public:
    a2dEditorFrameEvent( a2dEditorFrame* editorFrame, wxEventType type, int id = 0 );

    a2dEditorFrame* GetEditorFrame() const;

    wxEvent* Clone( void ) const;
};

BEGIN_DECLARE_EVENT_TYPES()
//! see a2dEditorFrameEvent \ingroup eventid
DECLARE_EXPORTED_EVENT_TYPE( A2DEDITORDLLEXP, a2dEVT_THEME_EVENT, 1 )
DECLARE_EXPORTED_EVENT_TYPE( A2DEDITORDLLEXP, a2dEVT_INIT_EVENT, 1 )
END_DECLARE_EVENT_TYPES()

typedef void ( wxEvtHandler::*a2dEditorFrameEventFunction ) ( a2dEditorFrameEvent& );

#define a2dEditorFrameEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(a2dEditorFrameEventFunction, &func)

/*! \addtogroup eventhandlers
*  @{
*/

//! static wxEvtHandler for theme event, send from a2dEditorFrame
#define EVT_THEME_EVENT(func) wx__DECLARE_EVT0( a2dEVT_THEME_EVENT, a2dEditorFrameEventHandler( func ))
#define EVT_INIT_EVENT(func) wx__DECLARE_EVT0( a2dEVT_INIT_EVENT, a2dEditorFrameEventHandler( func ))
//!  @} eventhandlers


//! sepcialized Frame for editor of a2dCanvas
class A2DEDITORDLLEXP a2dEditorFrame : public a2dDocumentFrame
{

public:

    typedef void ( a2dEditorFrame::*wxObjectEventFunctionM )( wxCommandEvent& );

public:

    DECLARE_DYNAMIC_CLASS( a2dEditorFrame )

    //! this makes dynamic creation possible ( e.g. a derived a2dEditorFrame )
    /*!
        Can be used by connectors ( e.g. a2dEditorMultiFrameViewConnector ) to create
        a derived a2dEditorFrame class using the ClassInfo.
        This makes one connector class enough for all derived a2dEditorFrame's.
        This constructor is used in combination with Create() to initilize the a2dEditorFrame.
    */
    a2dEditorFrame();

    //!construct  with a a2dCanvas as input
    a2dEditorFrame( bool isParent, wxFrame* parent, a2dCanvas* canvas = NULL,
                    const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE );

#if wxUSE_GLCANVAS && wxART2D_USE_OPENGL
    //!construct  with a a2dOglCanvas as input
    a2dEditorFrame( bool isParent, wxFrame* parent, a2dOglCanvas* canvas = NULL,
                    const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE );
#endif //wxUSE_GLCANVAS 

    a2dEditorFrame( bool isParent, wxFrame* parent,
                    const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE );

    //! delayed creation of window.
    /*!
        No cnavas is created, must be done by user.
    */
    bool Create( bool isParent, wxFrame* parent,
                 const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE );

    //! initialize
    void Init();
    //! setup menu
    void Theme( const wxString& themeName = wxT( "default" ) );

    ~a2dEditorFrame();

    virtual void CreateStatusBarThemed( const wxString& themeName = wxT( "default" ) );

    void SetupToolbar();

    //! return edit menu to set Undo and Redo commands
    wxMenu* GetEditMenu() { return m_editMenu; }

    void Update();

    void ConnectDocument( a2dCanvasDocument* doc, a2dCanvasView* drawer = NULL  );

    a2dDrawingPart* GetDrawingPart() { return m_drawingPart; }

    void SetDrawingPart( a2dDrawingPart* part ) { m_drawingPart = part; }

    //! get active controller set on a a2dCanvasView in this frame
    a2dStToolContr* GetToolController() { return m_contr; }

    //! set active controller set on a a2dCanvasView in this frame
    void SetToolController( a2dStToolContr* contr ) { m_contr = contr; }

    //! add a menu which executes a function
    /*!
        \param id wxID_ANY is non specific for use in func
        \param text text of menu
        \param helpString help for menu
        \param func function in this are derived class to be called.
    */
    void AddFunctionToMenu( int id, wxMenu* parentMenu, const wxString& text, const wxString& helpString, wxObjectEventFunctionM func, bool check = false );

    void RemoveFromMenu( int id, wxMenu* parentMenu, wxObjectEventFunctionM func );

    void SetToolDragingMode( wxCommandEvent& event );

    void SetToolDrawingMode( wxCommandEvent& event );

    void OnUndoEvent( a2dCommandProcessorEvent& event );

    void OnDoEvent( a2dCommandProcessorEvent& event );

    //!intercepted to initialize redo and undo menu's
    void OnActivate( wxActivateEvent& event );

    //! handler for event to set undo redo menustrings
    void OnSetmenuStrings( a2dCommandProcessorEvent& event );

    //! called on Close window
    void OnCloseWindow( wxCloseEvent& event );

    void OnComEvent( a2dComEvent& event );

    //! tool controller to be defined by user
    a2dSmrtPtr<a2dStToolContr> m_contr;

    void OnUndo( wxCommandEvent& event );
    void OnRedo( wxCommandEvent& event );

    void Quit( wxCommandEvent& event );

    void OnAbout( wxCommandEvent& event );

    void ShowLibs( wxCommandEvent& event );

    void PlaceFromLibByRef();
    void PlaceFromLibByCopy();

    void OnMenu( wxCommandEvent& event );

    void CreateThemeTest();
    void CreateThemeDefault();
    void CreateThemeXRC();


protected:

    void OnCopyShape(wxCommandEvent& event);
    void OnPasteShape(wxCommandEvent& event);
    void OnUpdatePasteShape( wxUpdateUIEvent& event  );
    void OnUpdateCopyShape( wxUpdateUIEvent& event  );

    void OnDragSimulate(wxCommandEvent& event);

    void FillData( wxCommandEvent& event );

    void OnUpdateUI( wxUpdateUIEvent& event );

    void OnTheme( a2dEditorFrameEvent& themeEvent );

    void OnInit( a2dEditorFrameEvent& initEvent );

    void OnActivateViewSentFromChild( a2dViewEvent& viewevent );

    a2dDrawingPart* m_drawingPart;

    wxMenuBar*  m_menuBar;

    //! to hold undo redo menu's which are reinitialized at activation of view
    wxMenu* m_editMenu;

    bool m_initialized;

    DECLARE_EVENT_TABLE()
};





#endif /* __A2DCANVASEDIT_H__ */


