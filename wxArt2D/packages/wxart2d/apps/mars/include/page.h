/*! \file apps/mars/page.h
    \author Erik van der Wal

    Copyright: 2004 (C) Erik van der Wal

    Licence: wxWidgets license

    RCS-ID: $Id: page.h,v 1.9 2007/02/06 21:53:34 titato Exp $
*/

#ifndef __PAGEH__
#define __PAGEH__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/grid.h>
#include "object.h"
#include "wx/docview/doccom.h"

class msDocument;
class msComponent;
class msPart;

//! Page class for a storage component
class msPage : public msObject
{
public:
    //! Constructor.
    msPage();

    //! Constructor.
    msPage( const wxString& pagename );

    //! Copy constructor
    msPage( const msPage& other, CloneOptions options, a2dRefMap* refs  );

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs  ) const;

    virtual wxWindow* CreateView( wxWindow* parent, a2dDocumentFrame* statusFrame, msDocument* doc );

protected:
#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

private:
    DECLARE_ABSTRACT_CLASS( msPage )
    msPage( const msPage& other );
};

class msSymbolPage : public msPage
{
public:
    //! Constructor.
    msSymbolPage();

    //! Copy constructor
    msSymbolPage( const msSymbolPage& other, CloneOptions options, a2dRefMap* refs  );

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs  ) const;

    // Documented in base class
    virtual wxWindow* CreateView( wxWindow* parent, a2dDocumentFrame* statusFrame, msDocument* doc );

    void HidePins();
    void HideVisibleProperties();

    DECLARE_PROPERTIES()

protected:
#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

private:
    DECLARE_DYNAMIC_CLASS( msSymbolPage )
};

class msScionPage : public msPage
{
public:
    //! Constructor.
    msScionPage();

    //! Copy constructor
    msScionPage( const msScionPage& other, CloneOptions options, a2dRefMap* refs  );

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs  ) const;

    // Documented in base class
    virtual wxWindow* CreateView( wxWindow* parent, a2dDocumentFrame* statusFrame, msDocument* doc );

protected:
#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

private:
    DECLARE_DYNAMIC_CLASS( msScionPage )
};

class msCircuitPage : public msPage
{
public:
    //! Constructor.
    msCircuitPage();

    //! Copy constructor
    msCircuitPage( const msCircuitPage& other, CloneOptions options, a2dRefMap* refs  );

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs  ) const;

    // Documented in base class
    virtual wxWindow* CreateView( wxWindow* parent, a2dDocumentFrame* statusFrame, msDocument* doc );

protected:
#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

private:
    DECLARE_DYNAMIC_CLASS( msCircuitPage )
};

class msLocallibPage : public msPage
{
public:
    //! Constructor.
    msLocallibPage();

    //! Copy constructor
    msLocallibPage( const msLocallibPage& other, CloneOptions options, a2dRefMap* refs  );

    // Documented in base class
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs  ) const;

    // Documented in base class
    virtual wxWindow* CreateView( wxWindow* parent, a2dDocumentFrame* statusFrame, msDocument* doc );

    msPart* GetPart( wxString name );

protected:
#if wxART2D_USE_CVGIO
    // Documented in base class
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    // Documented in base class
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

private:
    DECLARE_DYNAMIC_CLASS( msLocallibPage )
};





//! specialized view which knows how to display a DrawingDocument
//! as Text.
//! Updates write directly the document data into the display window which is a text control.
//! This will automatically cause a refresh of the window.
//! This view does not need a local storage,
class msPropView: public a2dView
{
    DECLARE_EVENT_TABLE()

public:

    msPropView(): a2dView() {}

    ~msPropView( void ) {}

    //! the view is flagged for redisplay because document did change.
    void OnUpdate( a2dDocumentEvent& event );

protected:

    void OnDraw( wxDC* dc ) {};

    //! just for demo
    void OnIdle( wxIdleEvent& event );

    DECLARE_DYNAMIC_CLASS( msPropView )

private:


};


//! specialized wxWindow to handle a2dView events
/*!
    The following events are sent up from the a2dView to this window.

    wxEVT_CLOSE_VIEW  wxEVT_CHANGEDFILENAME_DOCUMENT

    When the a2dView sent this events it disables itself until return, to prevent looping.

    wxEVT_CLOSE_VIEW should be handled in a derived class, or in some/the parent window of this window.
    The handler must call a2dScrolledWindow::SetView( NULL )
    or set m_view to NULL and release it, this Detaches the view from the window,
    In case the window gets deleted via a Destroy() this will be done automatically.

    These event are of type commandevents, and travel up to the parent of this window if not handled here.

    All other events to the window are sent first to the a2dView and if not handled there,
    can be handled here.
*/
class msPropViewWindow: public a2dDocumentViewWindow
{
public:
    //! Constructor for creation with view.
    /*!
        \param view     The view for the document ( can be <code>NULL</code> )
        \param parent   the parent window for this window
        \param id       The window identifier.
                        It may take a value of -1 to indicate a default value.
        \param pos      The window position.
        \param size     The window size.
        \param style    Window style, please look into wxWindows docs for an overview.
        \param name     The name of the window.
    */
    msPropViewWindow( a2dView* view,
                      wxWindow* parent,
                      wxWindowID id = -1,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      long style = 0,
                      const wxString& name = wxT( "msPropViewWindow" ) );

    wxGrid* GetGrid() { return m_grid; }
protected:
    wxGrid* m_grid;
    void OnSize( wxSizeEvent& event );

private:
    DECLARE_EVENT_TABLE()
    DECLARE_CLASS( msPropViewWindow )
};


#endif // __PAGEH__
