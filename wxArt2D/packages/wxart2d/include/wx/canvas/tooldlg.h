/*! \file wx/canvas/tooldlg.h
    \brief dialog for choosing a tool
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: tooldlg.h,v 1.13 2009/10/05 20:03:12 titato Exp $
*/


#ifndef __TOOLDLG_H__
#define __TOOLDLG_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/minifram.h"

#include "wx/canvas/canmod.h"
#include "wx/canvas/sttool.h"
#include "wx/canvas/edit.h"

#include "wx/canvas/strucdlg.h"

extern wxBitmap* GetBitmap( const wxString& name );

//class A2DCANVASDLLEXP a2dCentralCanvasCommandProcessor;

//! GUI to choose a tool, and execute it via the command processor
class A2DCANVASDLLEXP ToolDlg: public wxMiniFrame
{

public:
    //! Constructor
    ToolDlg( wxFrame* parent );

    //! Destructor
    ~ToolDlg();

    virtual void Populate();

    //! add a command menu to the parent menu, and connect it to the eventhandler of the dialog
    /*!
        See also a2dDocumentFrameAddCmdMenu()
    */
    void AddCmdMenu( wxBitmap& bitmap, const a2dMenuIdItem& cmdId );

    //! remove a command menu from the parent menu.
    /*!
        See AddCmdMenu()
    */
    void RemoveCmdMenu( const a2dMenuIdItem& cmdId );


protected:

    //! used by AddCmdMenu() to dynamically connect menu to function.
    //! The function just Skippes the event, so it will be handled by a command processor in the end.
    void OnCmdMenuId( wxCommandEvent& event );

    void OnCloseWindow( wxCloseEvent& event );

    wxPanel* m_panel;

    //the chosen tool.
    int m_chosen;

    wxGridSizer* m_sizer;

    // Declare used events.
    DECLARE_EVENT_TABLE()

};


#endif

