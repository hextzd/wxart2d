/*! \file wx/canvas/xmlpars.h
    \brief a2dIOHandlerCVGIn and a2dIOHandlerCVGOut - XML I/O classes for the CVG format.

    The main way of serialisation is done with XML and the format is called CVG ( canvas vector graphics ).

    \author Probably Klaas Holwerda

    Copyright: 2001-2004 (C) Probably Klaas Holwerda

    Licence: wxWidgets licence

    RCS-ID: $Id: xmlpars.h,v 1.16 2009/05/15 16:34:28 titato Exp $
*/

#ifndef _WX_CANVASXMLIO_H_
#define _WX_CANVASXMLIO_H_

#include "a2dprivate.h"

#if wxART2D_USE_XMLPARSE

#include <wx/defs.h>
#include <wx/string.h>

#if wxCHECK_VERSION(2, 5, 3)
#include <wx/sstream.h>
#endif

#include <expat.h>

#include "wx/xmlparse/genxmlpars.h"
#include "wx/xmlparse/xmlencode.h"
#include "wx/canvas/candefs.h"
#include "wx/canvas/canobj.h"

#if wxART2D_USE_CVGIO


//! Input handler for the CVG format.
/*!
    The CVG format is the native file format of wxArt2d.
    It's a XML format and each a2dCanvasObject is able to load its data from CVG.
    There's no specification of the CVG, please have a look into the DoLoad methods
    of the a2dCanvasObject derived classes to see details.

    In general the following is done.
        - each a2dObject has a2dObject::Save() and a2dObject::Load() as a start for serialisation.
        - a2dObject::DoLoad and a2dObject::DoSave() methods are virtual and should be implemented
        to save also the data of the derived classes.
        - Saving is a two pass process, in order to first write all XML attributes of an object and next
        its NON attribute data and nested objects.
        - one always needs to call derivedFrom::DoLoad in the implementation of a Doload()
        - one always needs to call derivedFrom::DoSave in the implementation of a DoSave()
        - multiple referenced objects are written only by refid attribute. When loading the file,
        those references are resolved.

    \ingroup fileio cvgfileio
*/
class A2DCANVASDLLEXP a2dIOHandlerCVGIn : public a2dIOHandlerXmlSerIn
{
   DECLARE_DYNAMIC_CLASS( a2dIOHandlerCVGIn )

public:
    //! Constructor.
    a2dIOHandlerCVGIn();

    //! Destructor.
    ~a2dIOHandlerCVGIn();

    //! test header of the file to see if its CVG format
    virtual bool CanLoad( a2dDocumentInputStream& stream, const wxObject* obj = NULL, wxClassInfo* docClassInfo = NULL );

    //! load a layer definition from a CVG file.
    bool LoadLayers( a2dDocumentInputStream& stream, a2dLayers* layers );

    //! reading a CVG document and add the contents as children to a given a2dCanvasObject parent.
    /*!
        This can be used to include a CVG document in an existing document.
        Also clipboard action might use this.
    */
    bool Load( a2dDocumentStringInputStream& stream, a2dDrawing* doc, a2dCanvasObject* parent );

    //! reading a single CVG canvasobject from a stream.
    /*!
        used Drag and Drop.
        \param stream the stream to read from
        \param canvasobject on return contain the read CVG canvas object.
    */
    bool Load( a2dDocumentStringInputStream& stream, a2dCanvasObjectPtr& canvasobject );

protected:


};

//! output handler for the CVG format.
/*!
    The CVG format is the native file format of wxArt2d.
    It's a XML format and each a2dCanvasObject is able to save its data as CVG.
    There's no specification of the CVG, please have a look into the DoSave methods
    of the a2dCanvasObject derived classes to see details.

    \ingroup fileio cvgfileio
*/
class A2DCANVASDLLEXP a2dIOHandlerCVGOut : public a2dIOHandlerXmlSerOut
{
   DECLARE_DYNAMIC_CLASS( a2dIOHandlerCVGOut )

public:
    //! Constructor.
    a2dIOHandlerCVGOut();

    //! Destructor.
    ~a2dIOHandlerCVGOut();

    //! this handler can also save as CVG
    bool CanSave( const wxObject* obj = NULL ) { return true; }

    //! saves as CVG starting at object start
    bool SaveStartAt( a2dDocumentOutputStream& stream, const a2dDrawing* drawing, a2dCanvasObject* start );

    //! save a layer definition to a CVG file.
    bool SaveLayers( a2dDocumentOutputStream& stream, a2dLayers* layers );

    void WriteCvgStartDocument( a2dDocumentOutputStream& stream );

    void WriteCvgEndDocument();

    void WriteObject( a2dCanvasObject* start );

protected:

    //! the document to store/load the data found into
    a2dDrawing* m_drawing;
};

#endif //wxART2D_USE_CVGIO

#endif // wxART2D_USE_XMLPARSE

#endif /* _WX_CANVASXMLIO_H_ */

