/*! \file editor/samples/viewmul/view.h
    \brief View classes
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: viewmul.h,v 1.2 2009/07/24 16:35:20 titato Exp $
*/

#ifndef __VIEWSAMPLEH__
#define __VIEWSAMPLEH__

#include "wx/canvas/canmod.h"
#include "wx/docview/docviewref.h"
#include "wx/canvas/sttool.h"
#include "wx/editor/candoc.h"
#include "wx/editor/xmlparsdoc.h"

class a2dVdrawDocManager;

//! extend wxSPMCFrameViewConnector to set initial mapping in world coordinates for the view.
//! A tools controler + a2dCanvas is added to the frame
//! In principle it sets the initial values for the a2dView.
class a2dMultiDrawingConnector: public a2dFrameViewConnector
{
    DECLARE_DYNAMIC_CLASS( a2dMultiDrawingConnector )
    DECLARE_EVENT_TABLE()

public:

    a2dMultiDrawingConnector();

    ~a2dMultiDrawingConnector();

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
    virtual void OnUpdate( a2dDocumentEvent& event );

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
    a2dCanvasObjectPtr m_top;

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

    void OnShowObject( wxCommandEvent& event );

    wxTextCtrl* m_textdisplay;

private:

    void OnSetMenuStrings( a2dCommandProcessorEvent& event );

    TextView* m_view;
    wxMenu* m_edit_menu;


};



#endif
