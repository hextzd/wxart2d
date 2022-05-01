/*! \file editor/samples/common/screen.h
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: screen.h,v 1.3 2008/07/30 21:54:04 titato Exp $
*/


#ifndef __SCREENS_H__
#define __SCREENS_H__

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/editor/editmod.h"

class RelativeRectangle: public a2dRect
{
public:

    RelativeRectangle( double w, double h , double radius = 0 );

    void DependencyPending( a2dWalkerIOHandler* handler );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    DECLARE_PROPERTIES()
};

enum wxRelativeTextDcPos
{
    wxCANVAS_RIGHT,
    wxCANVAS_LEFT,
    wxCANVAS_TOP,
    wxCANVAS_BOTTOM,
    wxCANVAS_X,
    wxCANVAS_Y,
};

class RelativeRectangle2: public a2dRect
{
public:

    RelativeRectangle2( a2dCanvasObject* refx, wxRelativeTextDcPos posx, a2dCanvasObject* refy, wxRelativeTextDcPos posy, double w, double h , double radius = 0 );

    void DependencyPending( a2dWalkerIOHandler* handler );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    wxRelativeTextDcPos m_posx;
    wxRelativeTextDcPos m_posy;

    a2dCanvasObjectPtr m_refx;
    a2dCanvasObjectPtr m_refy;
};

class wxRelativeTextDc: public a2dText
{
public:

    wxRelativeTextDc( a2dCanvasObject* refx, wxRelativeTextDcPos posx, a2dCanvasObject* refy, wxRelativeTextDcPos posy, const wxString& text,
                      const a2dFont& font = *a2dDEFAULT_CANVASFONT );

    void DependencyPending( a2dWalkerIOHandler* handler );

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    wxRelativeTextDcPos m_posx;
    wxRelativeTextDcPos m_posy;

    a2dCanvasObjectPtr m_refx;
    a2dCanvasObjectPtr m_refy;
};

class AnimaterObject: public a2dCanvasObject
{
public:
    AnimaterObject( double x, double y );
    ~AnimaterObject();
    void OnTimer( wxTimerEvent& WXUNUSED( event ) );

    a2dTimer* m_timer;

    double m_x, m_y;

    DECLARE_EVENT_TABLE()
};

class MyCanvasObjectGroup: public a2dCanvasObject
{
public:
    MyCanvasObjectGroup( double x, double y );

    void OnMouseEvent( a2dCanvasObjectMouseEvent& event );

    virtual a2dCanvasObject*    Clone();

    wxString m_text;
    wxString m_answer;

    DECLARE_EVENT_TABLE()
};

class MyCircle: public a2dCircle
{
public:
    MyCircle();

    MyCircle( double x, double y, double radius );

    DECLARE_EVENT_TABLE()

    DECLARE_DYNAMIC_CLASS( MyCircle )

    static a2dPropertyIdString* PROPID_tipproperty;

    DECLARE_PROPERTIES()

protected:

    void OnSliderEvent( wxCommandEvent& event );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    void OnEnterObject( a2dCanvasObjectMouseEvent& event );

    void OnLeaveObject( a2dCanvasObjectMouseEvent& event );
};

class MyEventHandler: public a2dObject
{
public:
    MyEventHandler( wxFrame* frame );

    void OnMouseEvent( a2dCanvasObjectMouseEvent& event );

private:

    wxFrame* m_frame;

    DECLARE_EVENT_TABLE()
};

extern void FillData0( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData1( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData2( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData3( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData4( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData5( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData6( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData7( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData8( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData9( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData10( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData11( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData12( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData13( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData14( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData15( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData16( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillData17( a2dCanvasDocument* datatree, a2dCanvasObject** settop );
extern void FillData18( a2dCanvasDocument* datatree, a2dCanvasObject** settop );
extern void FillData19( a2dCanvasDocument* datatree, a2dCanvasObject** settop );
extern void FillData20( a2dCanvasDocument* datatree, a2dCanvasObject** settop );
extern void FillData21( a2dCanvasDocument* datatree, a2dCanvasObject** settop );
extern void FillData22( a2dCanvasDocument* datatree, a2dCanvasObject** settop );
extern void FillData23( a2dCanvasDocument* datatree, a2dCanvasObject** settop );
extern void FillData24( a2dCanvasDocument* datatree, a2dCanvasObject** settop );
extern void FillData25( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

extern void FillDataAll( a2dCanvasDocument* datatree, a2dCanvasObject** settop );

#endif

