/*! \file wx/canextobj/canobj3d.h
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canobj3d.h,v 1.11 2008/09/05 19:01:10 titato Exp $
*/

#ifndef __WXCANOBJ3D_H__
#define __WXCANOBJ3D_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/canvas/canmod.h"
#include "wx/canvas/sttool.h"

#define __USE_EVENTS  1

//! to make object having a shadow behind it OR to extrude them in 3D
/*!
This object is a special kind of reference.
This object is initiated with a a2dCanvasObject, for that shape it
renders a shadow or 3D extruded structure.
The shadow uses the brush and pen set while the referenced object itself
uses it's own brush and pen.

    \ingroup  canvasobject meta
*/
class A2DEDITORDLLEXP a2d3DShape: public a2dCanvasObject
{
public:

    a2d3DShape( a2dCanvasObject* toshadow, double depth, double angle );

    a2d3DShape( const a2d3DShape& other, CloneOptions options, a2dRefMap* refs );

    virtual ~a2d3DShape();

    double GetExtrudeDepth() { return m_depth; }
    double GetExtrudeAngle() { return wxRadToDeg( m_angle3d ); }

    void SetExtrudeDepth( double depth )  { m_depth = depth; SetPending( true ); }
    void SetExtrudeAngle( double angle3d )  { m_angle3d = wxDegToRad( angle3d ); SetPending( true ); }

    void SetExtrudeFillFromShape();
    void SetExtrudeStrokeFromShape();

    DECLARE_CLASS( a2d3DShape )

protected:
    virtual a2dObject*  DoClone( CloneOptions options, a2dRefMap* refs ) const;

    void DoWalker( wxObject* parent, a2dWalkerIOHandler& handler );

#if wxART2D_USE_CVGIO
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );

    void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
#endif //wxART2D_USE_CVGIO

    bool DoUpdate( UpdateMode mode, const a2dBoundingBox& childbox, const a2dBoundingBox& clipbox, const a2dBoundingBox& propbox );

    void DoRender( a2dIterC& ic, OVERLAP clipparent );

    bool DoIsHitWorld( a2dIterC& ic, a2dHitEvent& hitEvent );

    void Set3DBrushPen();

    double m_depth;
    double m_angle3d;

    a2dCanvasObjectPtr m_shape;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2d3DShape( const a2d3DShape& other );
};

#if defined(WXART2D_USINGDLL)
template class A2DEDITORDLLEXP a2dSmrtPtr<a2d3DShape>;
#endif

#endif

