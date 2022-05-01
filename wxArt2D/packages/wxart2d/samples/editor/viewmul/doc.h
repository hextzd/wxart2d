/*! \file editor/samples/viewmul/doc.h
    \brief Document classes
    \author Klaas Holwerda

    Copyright: 2001-2004 (C) Klaas Holwerda

    Licence: wxWidgets license

    RCS-ID: $Id: doc.h,v 1.1 2007/01/17 21:35:20 titato Exp $
*/

#ifndef __DOCSAMPLEH__
#define __DOCSAMPLEH__

#include "wx/editor/editmod.h"

//! a Document class just to override the default Save and Load Object
class DrawingDocument: public a2dCanvasDocument
{
public:

    DrawingDocument( void );

    ~DrawingDocument( void );

    //! save to a file a set of rectangles
    a2dDocumentOutputStream& SaveObject( a2dDocumentOutputStream& stream, a2dIOHandlerStrOut* handler );

    //! load from a file a set of rectangles
    a2dDocumentInputStream& LoadObject( a2dDocumentInputStream& stream, a2dIOHandlerStrIn* handler );

    //! a way to save the data in the document to a string
    void SaveObject( wxString& stringstream, a2dIOHandlerStrOut* handler );

    DECLARE_DYNAMIC_CLASS( DrawingDocument )

};

#endif
