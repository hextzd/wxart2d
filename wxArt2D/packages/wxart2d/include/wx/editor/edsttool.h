/*! \file wx/editor/edsttool.h
    \brief stack based tools controller and tools for drawing and editing.

    This is a whole set of tools and controller which work closely together,
    such that they can function well inside one application.
    The controller uses a stack of tools, where the first one is always active.

    \author Klaas Holwerda

    Copyright: 2001-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: sttool.h,v 1.48 2009/10/01 19:22:35 titato Exp $
*/

#ifndef __A2DEDITORSTTOOL_H__
#define __A2DEDITORSTTOOL_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/listimpl.cpp>

#include "wx/canvas/sttool.h"


//!Interactive Insertion of an text object with a wxURI Object.
/*!Either with just one click or draging a rectangle to fit the text height in.

\remark  Left Click and/or drag.

\sa  a2dStToolContr


  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dLinkTool: public a2dStDrawTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_Link;

    a2dLinkTool( a2dStToolContr* controller );

    ~a2dLinkTool();

    bool ZoomSave() { return true;};

    //! font to use
    void SetFont( const a2dFont& font ) { m_font = font; }

    //! return used font
    const a2dFont& GetFont() { return m_font; }


protected:

    virtual void GenerateAnotation();

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

    a2dFont m_font;

public:

    DECLARE_CLASS( a2dLinkTool )
    DECLARE_EVENT_TABLE()

};

//!Interactive follow link.
/*!
\sa  a2dStToolContr

  \ingroup  tools
*/
class A2DCANVASDLLEXP a2dFollowLinkDocDrawCam: public a2dStTool
{
public:

    //! push this tool on the tools stack
    /*! \ingroup commandid
    */
    static const a2dCommandId COMID_PushTool_FollowLinkDocDrawCam;

    a2dFollowLinkDocDrawCam( a2dStToolContr* controller );

    bool StartFollowHere( double x, double y );

    bool StartFollowHere( int x, int y );

    bool ZoomSave() { return true;};

protected:

    void OnChar( wxKeyEvent& event );

    void OnMouseEvent( wxMouseEvent& event );

public:
    DECLARE_CLASS( a2dFollowLinkDocDrawCam )
    DECLARE_EVENT_TABLE()

};

//!Master tool for objects graphics slecting and dragging 
/*!This tool controls other tools such that simple object graphics
can be edited without manually changing tools.

- Left Down + Drag + Shift Down => select objects in rectangle add to already selected
- Left Down + Drag + Cntrl Down => select objects in rectangle deselect already selected first
- Left Down + Drag => move selected objects
- Left Down + Drag => move object and select it
- Left Down + Drag + Ctrl Down + Shift Down => copy selected objects 
- Right Down => Drag canvas view
- Right Down + Up => Show Popup

( wire mode )

- First LeftDown click on non wire object selects it.
- Shift + LeftDown to select a wire.
- Shift + LeftDown to select more.
- Second click on Selected object does NOT deselect
- Wire is draggable in segment and vertex and vertex insert when selected.
- Wire starts on pin of non wire object.
- Wire starts on wire and pin of wire object.
- LeftDown + drag on wire pin, makes it draggable.
- Dragging starting at non hit, or wire, starts a rectangle selection. 
- Double Click => Edit Selected of hit

( select mode )

- First LeftDown click on object selects it.
- Shift + LeftDown to select more.
- Second click on Selected object does de-select
- wire is draggable in segment and vertex and vertex insert, without selecting needed.
- Wire starts on pin of non wire object.
- Wire starts on wire and pin object when Ctrl is down.
- LeftDown + drag on wire pin, makes it draggable.
- Dragging starting at non hit, or wire, starts a rectangle selection. 
- Double Click => Edit Selected of hit

\ingroup  tools
*/
class a2dMasterTagGroups3Link : public a2dMasterTagGroups3
{
public:

    a2dMasterTagGroups3Link ( a2dStToolContr* controller );
    ~a2dMasterTagGroups3Link ();

	wxString GetToolString() const;

    virtual void EditDlgOrHandles( a2dCanvasObject* hit, bool modifier, bool noHandleEditForWire );
	
protected:

	void PushEditTool( a2dCanvasObject* hit );
    void PushLinkTool( a2dCanvasObject* hit );

    DECLARE_CLASS( a2dMasterTagGroups3Link  )
    DECLARE_EVENT_TABLE()
};


#endif /* __A2DEDITORSTTOOL_H__ */



