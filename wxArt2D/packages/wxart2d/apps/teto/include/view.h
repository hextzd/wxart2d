/////////////////////////////////////////////////////////////////////////////
// Name:        view.h
// Purpose:     View classes
// Author:      Klaas Holwerda
// Modified by:
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef __VIEWSAMPLEH__
#define __VIEWSAMPLEH__

#include <wx/canvas/canmod.h>
#include "wx/canvas/canmod.h"
#include "tedit.h"

//! extend a2dEditorMultiFrameViewConnector  to set initial mapping in world coordinates for the view.
//! In principle it sets the initial values for the a2dView.
class wxDrawingConnector: public a2dEditorMultiFrameViewConnector
{
    DECLARE_DYNAMIC_CLASS( wxDrawingConnector )
    DECLARE_EVENT_TABLE()

public:

    wxDrawingConnector( wxClassInfo* EditorClassInfo = ( wxClassInfo* ) NULL );

    void OnPostCreateView( a2dTemplateEvent& event );

};

//! specialized view which knows how to display a a2dCanvasDocument
//! as Text starting at a certain top a2dCanvasObject.
//! It uses wxTextCtrl to display the text.
class TextView: public a2dView
{
    DECLARE_EVENT_TABLE()

public:


    TextView(): a2dView() {}

    ~TextView( void ) {}

    //! the view will (re)display what it needs to show.
    virtual void OnUpdate( a2dView* sender, wxObject* hint = ( wxObject* ) NULL );

    //!set start of display in CVG format
    bool SetShowObject( a2dCanvasObject* obj );

    //!get start of display in CVG format
    a2dCanvasObject* GetShowObject() const {  return m_top; }

    void OnDraw( wxDC* dc ) {};

    a2dCanvasDocument* GetCanvasDocument() { return ( a2dCanvasDocument* ) GetDocument(); }

protected:

    void OnCreateView( a2dViewEvent& event );

    void OnSetDocument( a2dViewEvent& event );

    //! top to start writing in CVG
    a2dCanvasObject* m_top;

    DECLARE_DYNAMIC_CLASS( TextView )

private:

};

//! Generates Frame and Display Window for a text view.
class wxTextConnector: public a2dFrameViewConnector
{
    DECLARE_CLASS( wxTextConnector )
    DECLARE_EVENT_TABLE()

public:

    wxTextConnector();

    void OnPostCreateView( a2dTemplateEvent& event );

};

// ID for the menu commands
enum
{
    MSHOW_TOP = wxID_HIGHEST + 1,
};


//! extended frame with some standard menu's.
//! It display a TextView using a wxTextctrl in text.
class wxTextViewFrame : public a2dDocumentFrame
{
    DECLARE_EVENT_TABLE()

public:

    wxTextViewFrame( TextView* view, const wxPoint& pos, const wxSize& size, wxFrame* parent );

    void SetShowObject();

    wxTextCtrl* m_textdisplay;

private:

    void OnSetMenuStrings( a2dCommandProcessorEvent& event );

    TextView* m_view;

    wxMenu* m_edit_menu;

};


#endif
