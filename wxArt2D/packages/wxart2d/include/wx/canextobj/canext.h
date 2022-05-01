/*! \file wx/canextobj/canext.h
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canext.h,v 1.17 2009/09/26 20:40:32 titato Exp $
*/

#ifndef __WXCANEXT_H__
#define __WXCANEXT_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canprim.h"
#include "wx/canvas/sttool.h"

//! to render/hold a groups of text objects.
/*!
    The idea is to more efficiently render lines of text.

   EXPERIMENTAL, and not yet done.
*/
class A2DEDITORDLLEXP a2dTextGroup: public a2dCanvasObject
{
public:

    a2dTextGroup();

    a2dTextGroup( double x, double y );
    ~a2dTextGroup();
    a2dTextGroup( const a2dTextGroup& other, CloneOptions options, a2dRefMap* refs );

private:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    //!to render the child objects
    virtual void RenderChildObjectsOneLayer( a2dIterC& ic, RenderChild& whichchilds, OVERLAP clipparent );

    DECLARE_DYNAMIC_CLASS( a2dTextGroup )

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dTextGroup( const a2dTextGroup& other );
};

//! To use as a drawing frame within a drawing.
/*!
    \ingroup canvasobject
*/
class A2DEDITORDLLEXP a2dDrawingFrame: public a2dPolygonLClipper2
{
public:

    //!construct at given position
    /*!
    \param x X position
    \param y Y position
    \param width width of window
    \param height height of window
    */
    a2dDrawingFrame( double x = 0 , double y = 0, double width = 100, double height = 100 );

    //! copy/clone constructor
    a2dDrawingFrame( const a2dDrawingFrame& other, CloneOptions options, a2dRefMap* refs );

    //!destructor
    virtual ~a2dDrawingFrame();

    DECLARE_DYNAMIC_CLASS( a2dDrawingFrame )

    DECLARE_EVENT_TABLE()

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

    virtual void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

    bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    void OnCanvasObjectMouseEvent( a2dCanvasObjectMouseEvent& event );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dDrawingFrame( const a2dDrawingFrame& other );
};

#endif

