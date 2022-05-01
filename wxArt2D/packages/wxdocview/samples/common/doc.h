/*! \file docview/samples/common/doc.h
    \brief Document classes
    \author Julian Smart
    \date Created 04/01/98

    Copyright: (c) Julian Smart and Markus Holzem

    Licence: wxWidgets license

    RCS-ID: $Id: doc.h,v 1.2 2009/05/15 16:34:29 titato Exp $
*/

#ifndef __DOCSAMPLEH__
#define __DOCSAMPLEH__

#include "wx/docview/docviewref.h"
#include "wx/docview/doccom.h"
#include "wx/cmdproc.h"

// Plots a line from one point to the other
class DoodleLine: public wxObject
{
public:
    double x1;
    double y1;
    double x2;
    double y2;
};

// Contains a list of lines: represents a mouse-down doodle
class DoodleSegment: public wxObject
{
public:

    DoodleSegment();

    DoodleSegment( const wxColour& colour, int penwidth );
    DoodleSegment( DoodleSegment& seg );
    ~DoodleSegment( void );

    void Draw( wxDC* dc );

    a2dDocumentOutputStream& SaveObject( a2dDocumentOutputStream& stream );
    a2dDocumentInputStream& LoadObject( a2dDocumentInputStream& stream );

    wxList m_lines;

    wxColour m_colour;

    int m_width;
};

class DrawingDocument: public a2dDocument
{
    DECLARE_DYNAMIC_CLASS( DrawingDocument )
public:

    DrawingDocument( void );
    ~DrawingDocument( void );


    a2dDocumentOutputStream& SaveObject( a2dDocumentOutputStream& stream, a2dIOHandlerStrOut* handler );
    a2dDocumentInputStream& LoadObject( a2dDocumentInputStream& stream, a2dIOHandlerStrIn* handler );

    inline wxList& GetDoodleSegments( void ) const { return ( wxList& ) doodleSegments; };

private:
    wxList doodleSegments;

};

#define DOODLE_CUT          1
#define DOODLE_ADD          2

class DrawingCommandAddSegment: public a2dCommand
{
protected:
    DoodleSegment* segment;
    DrawingDocument* doc;
public:
    DrawingCommandAddSegment( DrawingDocument* ddoc, DoodleSegment* seg );
    ~DrawingCommandAddSegment( void );

    bool Do( void );
    bool Undo( void );

    static const a2dCommandId COMID_AddSegment;
};

class DrawingCommandCutSegment: public a2dCommand
{
protected:
    DoodleSegment* segment;
    DrawingDocument* doc;
public:
    DrawingCommandCutSegment( DrawingDocument* ddoc, DoodleSegment* seg );
    ~DrawingCommandCutSegment( void );

    bool Do( void );
    bool Undo( void );

    static const a2dCommandId COMID_CutSegment;

};

class DoodleDocumentIoHandlerIn: public a2dIOHandlerStrIn
{
public:

    DoodleDocumentIoHandlerIn();

    ~DoodleDocumentIoHandlerIn( void );

    bool CanLoad( a2dDocumentInputStream& stream, const wxObject* obj = NULL, wxClassInfo* docClassInfo = NULL );

    virtual bool Load( a2dDocumentInputStream& stream, wxObject* doc );
};

class DoodleDocumentIoHandlerOut: public a2dIOHandlerStrOut
{
public:

    DoodleDocumentIoHandlerOut();

    ~DoodleDocumentIoHandlerOut( void );

    bool CanSave( const wxObject* obj = NULL ) { return true; }

    virtual bool Save( a2dDocumentOutputStream& stream, const wxObject* doc );
};

class DoodleDocumentIoHandlerDxrIn: public a2dIOHandlerStrIn
{
public:

    DoodleDocumentIoHandlerDxrIn();

    ~DoodleDocumentIoHandlerDxrIn( void );

    bool CanLoad( a2dDocumentInputStream& stream, const wxObject* obj = NULL, wxClassInfo* docClassInfo = NULL );

    virtual bool Load( a2dDocumentInputStream& stream, wxObject* doc );
};

class DoodleDocumentIoHandlerDxrOut: public a2dIOHandlerStrOut
{
public:

    DoodleDocumentIoHandlerDxrOut();

    ~DoodleDocumentIoHandlerDxrOut( void );

    bool CanSave( const wxObject* obj = NULL ) { return true; }

    virtual bool Save( a2dDocumentOutputStream& stream, const wxObject* doc );
};


#endif
