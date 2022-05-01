/*! \file wx/svgio/parssvg.h
    \brief SVG loader and saver of a wxCanvasDocument
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: parssvg.h,v 1.17 2009/05/15 16:34:28 titato Exp $
*/

#ifndef _WX_CANVASSVGIO_H_
#define _WX_CANVASSVGIO_H_

#include <map>
#include "wx/canvas/xmlpars.h"
#include "wx/canvas/layerinf.h"
#include "wx/canvas/canprop.h"
#include "wx/canvas/vpath.h"
#include "wx/canvas/polygon.h"
#include "wx/editor/candoc.h"

#if wxART2D_USE_SVGIO

#if (wxART2D_USE_XMLPARSE == 0)
#error Cannot build with SVG support without XMLPARSE module
#else
#include "wx/xmlparse/genxmlpars.h"
#include "wx/xmlparse/xmlencode.h"
#endif

using namespace std;

class a2dCanvasDocument;

//! input handler for the SVG format
/*!
    \ingroup fileio svgfileio
*/
class A2DCANVASDLLEXP a2dIOHandlerSVGIn : public a2dIOHandlerXMLPullExpat
{
public:

    a2dIOHandlerSVGIn();

    ~a2dIOHandlerSVGIn();

    virtual bool CanLoad( a2dDocumentInputStream& stream, const wxObject* obj = NULL, wxClassInfo* docClassInfo = NULL );

    virtual bool Load( a2dDocumentInputStream& stream , wxObject* doc );

protected:

    bool LoadSvg( a2dCanvasDocument* doc );
    bool LoadSvgGroup( a2dCanvasObject* parent );
    bool LoadSvgAnchor( a2dCanvasObject* parent );
    bool LoadSvgGroupElement( a2dCanvasObject* parent );

    bool LoadSvgRect( a2dCanvasObject* parent );

    bool LoadSvgCircle( a2dCanvasObject* parent );

    bool LoadSvgEllipse( a2dCanvasObject* parent );

    bool LoadSvgText( a2dCanvasObject* parent );

    bool ParsePoints( a2dVertexList* points );

    bool LoadSvgPolygon( a2dCanvasObject* parent );

    bool LoadSvgPolyline( a2dCanvasObject* parent );

    bool LoadSvgLine( a2dCanvasObject* parent );

    bool ParsePathPoints( a2dVpath* points );

    bool LoadSvgPath( a2dCanvasObject* parent );

    bool LoadSvgImage( a2dCanvasObject* parent );

    bool ParseSvgTransForm( a2dCanvasObject* parent );

    bool ParseSvgStyle( a2dCanvasObject* parent );

    bool ParseSvgLink( a2dCanvasObject* parent );

    //! the document to store/load the data found into
    a2dCanvasDocument* m_doc;

    //current object under creation
    a2dCanvasObjectPtr m_currentobject;

    //! currentfill style
    a2dFill m_fill;

    //! currentstroke style
    a2dStroke m_stroke;
};

//! output handler for the SVG format
/*!
    \ingroup fileio svgfileio
*/
class A2DCANVASDLLEXP a2dIOHandlerSVGOut : public a2dIOHandlerXMLWrite
{
public:

    a2dIOHandlerSVGOut();

    ~a2dIOHandlerSVGOut();

    //! document will be saved as seen from this object, if NULL, the root of the document will be used.
    void SetShowObject( a2dCanvasObject* showObject ) { m_showObject = showObject; };

    //!return pointer of the currently set object to save.
    /*!
        \return: pointer to the current object that is shown.
    */
    a2dCanvasObject* GetShowObject() const { return m_showObject; }

    void SetIterC( a2dIterC& ic ) { m_ic = &ic; }

    bool CanSave( const wxObject* obj = NULL );

    virtual bool Save( a2dDocumentOutputStream& stream, const wxObject* doc );

protected:
    void Save( const a2dCanvasObject* object, int layer );
    void SetSVGStyle( const a2dFill& fill, const a2dStroke& stroke, const a2dCanvasObject* object, int layer );
    void SaveGradient( const a2dFill& fill, const a2dCanvasObject* object );

    //! Generates a quadratic spline path in SVG from a set of points representing the control polygon.  This correctly renders the spline in the same way as the drawers for both polylines and polygons.
    void CreateSVGSpline( const a2dVertexList* vertexList, bool asPolygon = false, const wxString& style = _T( "" ) );
    void CreateSVGSpline( const a2dVertexArray* vertexArray, bool asPolygon = false, const wxString& style = _T( "" ) );

    //! Generates SVG XML code for different types of canvas objects
    void DoSave( const a2dCanvasObject* object, int layer );

    void DoSaveProperty( const a2dCanvasObject* object, const a2dNamedProperty* property, int layer );

    wxString UniqueName( wxString baseName );

    //! the document to store/load the data found into
    a2dCanvasDocument* m_doc;

    //current object under creation
    a2dCanvasObjectPtr m_currentobject;

    //! currentfill style
    a2dFill m_fill;

    //! currentstroke style
    a2dStroke m_stroke;

    a2dIterC* m_ic;

    //! save the document from this object down.
    a2dCanvasObject* m_showObject;

    map<wxString, int> m_nameMap;
};
#endif //wxART2D_USE_SVGIO

#endif
