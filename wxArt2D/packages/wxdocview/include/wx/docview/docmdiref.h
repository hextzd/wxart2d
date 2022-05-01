/*! \file wx/docview/docmdiref.h
    \brief Frame classes for MDI document/view applications

    MSW way of having application with one wxFrame and several draggable
    sub windows in that frame is called MDI. In here are the classes to be able to
    connect views and documents into such a GUI.

    \author Julian Smart
    \date Created 01/02/97

    Copyright: (c)

    Licence: wxWidgets licence

    RCS-ID: $Id: docmdiref.h,v 1.14 2009/09/29 20:06:46 titato Exp $
*/

#ifndef _WX_DOCMDIREF_H_
#define _WX_DOCMDIREF_H_

#include "wx/defs.h"

#if wxUSE_MDI_ARCHITECTURE && wxUSE_DOC_VIEW_ARCHITECTURE

#include "wx/docview/docviewref.h"
#include "wx/mdi.h"

//! Use wxDocMDIParentFrame instead of wxDocMDIParentFrame
/*!
    This class is the replacement for the standard wxWindow wxDocMDIParentFrame

    \ingroup docview

*/
class A2DDOCVIEWDLLEXP a2dDocumentMDIParentFrame: public wxMDIParentFrame

{
public:
    //! Constructor.
    /*!
        \param parent The parent frame. This should be <code>NULL</code>
        \param id     The window identifier.
                      It may take a value of -1 to indicate a default value.
        \param title  The caption to be displayed on the frame's title bar.
        \param pos    The window position.
        \param size   The window size.
        \param style  Window style, please look into wxWindows docs for an overview.
        \param name   The name of the window.
    */
    a2dDocumentMDIParentFrame( wxFrame* parent, wxWindowID id,
                               const wxString& title, const wxPoint& pos = wxDefaultPosition,
                               const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE, const wxString& name = wxT( "frame" ) );

    a2dDocumentMDIParentFrame();

    //! Constructor for two-step creation (2nd step).
    /*!
        \param frame The parent frame. This should not be <code>NULL</code>
        \param id    The window identifier.
                     It may take a value of -1 to indicate a default value.
        \param title The caption to be displayed on the frame's title bar.
        \param pos   The window position.
        \param size  The window size.
        \param style Window style, please look into wxWindows docs for an overview.
        \param name  The name of the window.
    */
    bool Create(
        wxWindow* frame,
        wxWindowID id = -1,
        const wxString& title = wxT( "frame" ),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long  style = wxDEFAULT_FRAME_STYLE,
        const wxString& name = wxT( "frame" ) );

    //! Extend event processing to search the document manager's event table
    virtual bool ProcessEvent( wxEvent& event );

    //! Does a call to Close()
    void OnExit( wxCommandEvent& event );

    //! Creates a new view for a file from file-history (a new MDI child will be created)
    void OnMRUFile( wxCommandEvent& event );

    //! Closes the this window, if not vetoed.
    void OnCloseWindow( wxCloseEvent& event );

    //! add a command menu to the parent menu, and connect it to the eventhandler of the frame
    /*!
        Many commands which can be submitted to a commandprocessor in the following manner:
        \code
        commandprocessor->Submit(  new a2dCommand_SomeCommand() )
        \endcode
        Submitting the command makes it possible to undo the action.
        To issue such a command from a menu requires an Id. Therefore wxDocview defines many predefined
        menus items in the form of a2dMenuIdItem. They produce a unique Id to be handled in
        the commandprocessor. Like in:
        \code
        void a2dDocumentCommandProcessor::OnMenu( wxCommandEvent& event )
        \endcode
        Several modules in wxDocview extend, in the derived a2dDocumentCommandProcessor, the set of handled
        menu Id's. The OnMenu intercepts the menu event, and sets up a new a2dCommand to be submitted.
        Understand that a2dDocumentFrame and a2dDocumentMDIParentFrame already redirect non handled events to
        the commandprocessor of the application.
        An a2dMenuIdItem defined menu text and helpstring, therefore all you need to do to add a command to
        your application is AddCmdMenu().
        Many commands have parameters, and a a2dMenuIdItem its Id, can only be coupled to one a2dCommand with
        some predefined set of parameters.
        If you need a different implementation of the same command you can extend in the same manner
        in your derived commandprocessor. But you can always use the standard way of wxWidgets to add a menu,
        and add a handler in the frame. In the handler setsup the a2dCommand and submits it.
        Exactly the same behaviour, only not predefined.

        \remark a2dMenuIdItem uses Id my name ready to be used by XRC files.
    */
    void AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId );

    //! same as for a2dMenuIdItem, Id in wxMenuItem must be a valid id from one a2dMenuIdItem
    //! used in XRC files
    void AddCmdMenu( wxMenu* parentMenu, wxMenuItem* item );

    //! remove a command menu from the parent menu.
    /*!
        See AddCmdMenu()
    */
    void RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId );

    //! add items to toolbar using a predefined command ID.
    void AddCmdToToolbar( const a2dMenuIdItem& cmdId );

protected:

    //! used by AddCmdMenu() to dynamically connect menu to function.
    //! The function just Skippes the event, so it will be handled by a command processor in the end.
    void OnCmdMenuId( wxCommandEvent& event );

private:

    DECLARE_CLASS( a2dDocumentMDIParentFrame )
    DECLARE_EVENT_TABLE()
};

//!The a2dDocumentMDIChildFrame class provides a default frame for displaying documents
/*!
    In case of a wxEVT_CLOSE_WINDOW event, it is first redirected to the window, and
    if not vetod to the view.

    \ingroup docview
*/
class A2DDOCVIEWDLLEXP a2dDocumentMDIChildFrame: public wxMDIChildFrame
{

public:

    //! Constructor for two-step creation. Call a2dDocumentMDIChildFrame::Create later
    a2dDocumentMDIChildFrame();

    //! Constructor
    /*!
        \param frame The parent frame. This should not be <code>NULL</code>
        \param view  The view for the document ( can be <code>NULL</code> )
        \param id    The window identifier.
                     It may take a value of -1 to indicate a default value.
        \param title The caption to be displayed on the frame's title bar.
        \param pos   The window position.
        \param size  The window size.
        \param style Window style, please look into wxWindows docs for an overview.
        \param name  The name of the window.
    */
    a2dDocumentMDIChildFrame(
        wxMDIParentFrame* frame,
        a2dView* view = NULL,
        wxWindowID id = -1,
        const wxString& title = wxT( "a2dDocumentMDIChildFrame" ),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long  style = wxDEFAULT_FRAME_STYLE,
        const wxString& name = wxT( "a2dDocumentMDIChildFrame" ) );

    //! Constructor for two-step creation (2nd step).
    /*!
        \param frame The parent frame. This should not be <code>NULL</code>
        \param view  The view for the document ( can be <code>NULL</code> )
        \param id    The window identifier.
                     It may take a value of -1 to indicate a default value.
        \param title The caption to be displayed on the frame's title bar.
        \param pos   The window position.
        \param size  The window size.
        \param style Window style, please look into wxWindows docs for an overview.
        \param name  The name of the window.
    */
    bool Create(
        wxMDIParentFrame* frame,
        a2dView* view = NULL,
        wxWindowID id = -1,
        const wxString& title = wxT( "a2dDocumentMDIChildFrame" ),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long  style = wxDEFAULT_FRAME_STYLE,
        const wxString& name = wxT( "a2dDocumentMDIChildFrame" ) );

    //! Destructor
    ~a2dDocumentMDIChildFrame();

    //! set a pointer to a a2dView to sent view events to.
    /*!
        Normally set via OnActivateViewSentFromChild()

        The m_view is used to redirect the frame event for closing the window,
        and the activate event for the frame, to the view.
        Only if the m_view->GetDisplayWindow() is the a2dDocumentFrame itself, it will also be used for
        drawing the view ( See OnDraw() )
    */
    void SetView( a2dView* view );

    //! get the pointer to a a2dView to which view events are sent
    inline a2dView* GetView( void ) const { return m_view; }

    void SetDestroyOnCloseView( bool destroyOnCloseView ) { m_destroyOnCloseView = destroyOnCloseView; }

    //! Extend event processing to search the view's event table
    virtual bool ProcessEvent( wxEvent& event );

    void OnActivate( wxActivateEvent& event );
    void OnCloseWindow( wxCloseEvent& event );

    bool Destroy();

    //! Override this function to draw the graphic (or just process EVT_PAINT)
    /*!
        The default handler for EVT_PAINT is OnPaint, and it calls OnDraw().
        OnDraw its default implementation is calling the m_view its OnDraw().
    */
    virtual void OnDraw( wxDC& dc );

    //! add a command menu to the parent menu, and connect it to the eventhandler of the frame
    /*!
        Many commands which can be submitted to a commandprocessor in the following manner:
        \code
        commandprocessor->Submit(  new a2dCommand_SomeCommand() )
        \endcode
        Submitting the command makes it possible to undo the action.
        To issue such a command from a menu requires an Id. Therefore wxDocview defines many predefined
        menus items in the form of a2dMenuIdItem. They produce a unique Id to be handled in
        the commandprocessor. Like in:
        \code
        void a2dDocumentCommandProcessor::OnMenu( wxCommandEvent& event )
        \endcode
        Several modules in wxDocview extend, in the derived a2dDocumentCommandProcessor, the set of handled
        menu Id's. The OnMenu intercepts the menu event, and sets up a new a2dCommand to be submitted.
        Understand that a2dDocumentFrame and a2dDocumentMDIParentFrame already redirect non handled events to
        the commandprocessor of the application.
        An a2dMenuIdItem defined menu text and helpstring, therefore all you need to do to add a command to
        your application is AddCmdMenu().
        Many commands have parameters, and a a2dMenuIdItem its Id, can only be coupled to one a2dCommand with
        some predefined set of parameters.
        If you need a different implementation of the same command you can extend in the same manner
        in your derived commandprocessor. But you can always use the standard way of wxWidgets to add a menu,
        and add a handler in the frame. In the handler setsup the a2dCommand and submits it.
        Exactly the same behaviour, only not predefined.

        \remark a2dMenuIdItem uses Id my name ready to be used by XRC files.
    */
    void AddCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId );

    //! same as for a2dMenuIdItem, Id in wxMenuItem must be a valid id from one a2dMenuIdItem
    //! used in XRC files
    void AddCmdMenu( wxMenu* parentMenu, wxMenuItem* item );

    //! remove a command menu from the parent menu.
    /*!
        See AddCmdMenu()
    */
    void RemoveCmdMenu( wxMenu* parentMenu, const a2dMenuIdItem& cmdId );

    //! add items to toolbar using a predefined command ID.
    void AddCmdToToolbar( const a2dMenuIdItem& cmdId );

protected:

    //! used by AddCmdMenu() to dynamically connect menu to function.
    //! The function just Skippes the event, so it will be handled by a command processor in the end.
    void OnCmdMenuId( wxCommandEvent& event );

    //! default handler for paint events, it calls OnDraw().
    void OnPaint( wxPaintEvent& event );

    //! default for closing a view in this frame
    /*!
        Close the view of the current m_window and destroy the frame.
    */
    void OnCloseView( a2dCloseViewEvent& event );

    //! sets m_view to the current active view, generate by a2dView::Activate()
    void OnActivateViewSentFromChild( a2dViewEvent& viewevent );

    //! used for redirecting close and activate events to the view which has the focus.
    //! See SetView()
    a2dSmrtPtr<a2dView> m_view;

    //! when closing the view the frame will be destroyed.
    bool m_destroyOnCloseView;

private:

    DECLARE_EVENT_TABLE()
    DECLARE_CLASS( a2dDocumentMDIChildFrame )
};

#endif /* wxUSE_MDI_ARCHITECTURE && wxUSE_DOC_VIEW_ARCHITECTURE */

#endif /* _WX_DOCMDIREF_H_ */


