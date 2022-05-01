/*! \file wx/keyio/keyio.h
    \brief KEY input and output

    The loader and saver for the KEY format. The KEY format is GDSII in ascii,
    but extended to be able to contain more types of primitives.
    It is typically used in the electronic mask industry. For instance Color and line thickness
    or of no importance, primtives are placed on layers, just to tell which are on the layers is filled.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: keyio.h,v 1.18 2009/05/15 16:34:28 titato Exp $
*/

#ifndef _KEYIO_H
#define _KEYIO_H

#include <wx/wfstream.h>

#include "a2dprivate.h"

#if wxART2D_USE_KEYIO

#include "wx/canvas/layerinf.h"
#include "wx/gdsio/gdsio.h"


//!Input driver for KEY format files
/*!
    Every record in the KEY file is read/mapped into the right internal structure
    The KEY file is parsed according to the BNF of the KEY format.
    Info on the different record types that are read can be found in the KEY format manual
    All basic data types are read using a specialized function that now how those types
    are stored in KEY.
    For layers settings this handler checks for a file with the same name as the document that is loaded,
    but with extension ".cvg", if it does exist it will be used to read layers (in CVG format).

    \ingroup fileio keyfileio
*/
class A2DCANVASDLLEXP a2dIOHandlerKeyIn : public a2dIOHandlerStrIn
{

public:

    a2dIOHandlerKeyIn();
    ~a2dIOHandlerKeyIn();

    bool LinkReferences();

    wxChar PeekNextC();

    void IncC();

    virtual bool CanLoad( a2dDocumentInputStream& stream, const wxObject* obj = NULL, wxClassInfo* docClassInfo = NULL  );

    virtual bool Load( a2dDocumentInputStream& stream, wxObject* doc );

    virtual void InitializeLoad(  );

    virtual void ResetLoad();

    void SetReadAsCameleon( bool asCameleons ) { m_asCameleons = asCameleons; }

    bool GetReadAsCameleon() { return m_asCameleons; }

    void SetReadAsMultiroot( bool asMultiroot ) { m_asMultiroot = asMultiroot; }

    bool GetReadAsMultiroot() { return m_asMultiroot; }

protected:

    bool GetCommand();

    bool NormalWord();
    bool QuotedString();
    bool BracedString();
    bool Comment();

    bool Blanks();

    bool Word();

    bool ReadItem( const wxString& type );

    bool SkipXYData();

    double ReadDouble();

    //! set datatype of elements as a property
    void SetDataTypeProperty( a2dCanvasObject* toobject, wxUint16 type );

    void ReadKey();
    bool ReadBgnlib();
    bool ReadProperties( a2dCanvasObject* parent );
    bool ReadParameters( a2dCanvasObject* parent );
    bool ReadBoundary( a2dCanvasObject* parent );
    bool ReadSurface( a2dCanvasObject* parent );
    bool ReadPath( a2dCanvasObject* parent );
    bool ReadText( a2dCanvasObject* parent );
    bool ReadStructureReference( a2dCanvasObject* parent );
    bool ReadStructureCameleonReference( a2dCanvasObject* parent );
    bool ReadArrayReference( a2dCanvasObject* parent );
    bool ReadNode( a2dCanvasObject* parent );
    bool ReadBox( a2dCanvasObject* parent );
    bool ReadCircle( a2dCanvasObject* parent );
    bool ReadArc( a2dCanvasObject* parent );
    bool ReadLine( a2dCanvasObject* parent );
    bool ReadImage( a2dCanvasObject* parent );

    bool ReadElflags( a2dCanvasObject* object );
    bool ReadGdsHeader();
    bool ReadLayer( a2dCanvasObject* object );
    bool ReadLibrarySettings();
    bool Read( a2dPoint2D& _Point );
    bool ReadStrans( Strans& _Strans );
    bool ReadStructure( a2dCanvasObject* parent );
    bool ReadCameleonStructure( a2dCanvasObject* parent );
    bool ReadElement( a2dCanvasObject* parent );
    bool ReadTextBody();

    bool ReadPolygon( a2dVertexList* segments );
    bool ReadPolyline( a2dVertexList* segments );

    void SetFlags( a2dCanvasObject* Element );

    //!last read record is stored
    bool m_back;

    //!keyword for record in KEY data
    wxString m_keyword;
    //!value for record in KEY data
    wxString m_value;

    //!type of the record in KEY data
    int m_recordtype;

    //!last read line in DATA
    wxUint32 m_linenumber;

    wxChar a;
    wxString m_error_mes;
    wxString m_b;

    //!size of the last read record
    TWO_G_BYTE_UNSIGNED_INTEGER m_recordsize;

    //!mapping of GDSII layers to internal layers
    a2dLayerMapNr m_mapping;

    //! the document to store/load the data found into
    a2dCanvasDocument* m_doc;

    double m_scale_in;

    double m_userunits_out;

    int m_objectFlags;

    wxUint16 m_lastElementLayer;

    int m_refMaxx; 
    int m_dx, m_dy;

    a2dLayers* m_layers;
    bool m_hasLayers;

    bool m_asCameleons;
    bool m_asMultiroot;
    double m_gridx, m_gridy;
};

//!Output driver for KEY files
/*!
    Every object in the document is written into the right KEY file structure
    Info on the different record types that are read can be found in the KEY format manual

    \ingroup fileio keyfileio
*/
class A2DCANVASDLLEXP a2dIOHandlerKeyOut : public a2dIOHandlerStrOut
{

public:

    a2dIOHandlerKeyOut();
    ~a2dIOHandlerKeyOut();

    bool LinkReferences();

    bool CanSave( const wxObject* obj = NULL );

    virtual bool Save( a2dDocumentOutputStream& stream, const wxObject* doc );

    virtual void InitializeSave(  );

    virtual void ResetSave();

    void SetReadAsCameleon( bool asCameleons ) { m_asCameleons = asCameleons; }

    bool GetReadAsCameleon() { return m_asCameleons; }

    //! if set text will be converted to a vectorpath, and as polygons and polylines.
    void SetSaveTextAsVpath( bool textAsPath ) { m_textAsPath = textAsPath; }

    //! /see SetSaveTextAsVpath()
    bool GetSaveTextAsVpath() { return m_textAsPath; }

    //! if set, the current view its ShowObject will be saved top down.
    void SetFromViewAsTop( bool fromViewAsTop ) { m_fromViewAsTop = fromViewAsTop; }

    //! /see SetFromViewAsTop()
    bool GetFromViewAsTop() { return m_fromViewAsTop; }

    void SetAberArcToPoly( double aber ) { m_AberArcToPoly = aber; }

private:

protected:

    //! write a double number.
    virtual void WriteDouble( double d )
    {
        wxString str;
        str.Printf( "%s",  wxString::FromCDouble( d, m_precision ) );
        WriteString( str );
    }

    a2dIOHandlerKeyOut& operator<<( const wxChar* string );
    a2dIOHandlerKeyOut& operator<<( const wxString& string );
    a2dIOHandlerKeyOut& operator<<( char c );
#if wxUSE_UNICODE && wxWCHAR_T_IS_REAL_TYPE
    a2dIOHandlerKeyOut& operator<<( wchar_t wc );
#endif // wxUSE_UNICODE
    a2dIOHandlerKeyOut& operator<<( wxInt16 c );
    a2dIOHandlerKeyOut& operator<<( wxInt32 c );
    a2dIOHandlerKeyOut& operator<<( wxUint16 c );
    a2dIOHandlerKeyOut& operator<<( wxUint32 c );
    a2dIOHandlerKeyOut& operator<<( double f );
    a2dIOHandlerKeyOut& operator<<( float f );

    const wxChar Endl();

    void WriteProperties( const a2dNamedPropertyList& props );

    void WriteParameters( const a2dNamedPropertyList& props );

    void WriteFlags( int i );

    void Write( Strans* Strans );

    void SetFlags( a2dCanvasObject* Element );

    void WritePoint( double xi, double yi );

    void WriteSegment( const a2dAffineMatrix& lworld, a2dLineSegment* prevSegment, a2dLineSegment* Segment );

    void WriteVpath( const a2dAffineMatrix& lworld, const a2dVpath* path, int layer, int datatype, int pathtype, double width );

    void WritePoly( const a2dAffineMatrix& lworld , a2dVertexList* vlist, bool close );

    void WriteVertexArrayPolyline( const a2dAffineMatrix& lworld, a2dVertexArray* points, int layer, int datatype, int pathtype, double width, bool spline = false, bool close = true );

    void WriteVertexListPolyline( const a2dAffineMatrix& lworld, a2dVertexList* points, int layer, int datatype, int pathtype, double width, bool spline = false, bool close = true );

    void WriteVertexArrayPolygon( const a2dAffineMatrix& lworld, a2dVertexArray* points, int layer, int datatype, double width, bool spline = false, bool close = true );

    void WriteVertexListPolygon( const a2dAffineMatrix& lworld, a2dVertexList* points, int layer, int datatype, double width, bool spline = false, bool close = true );

    void SaveStructure( a2dCanvasObject* object, a2dCanvasObjectList* towrite );

    void Save( a2dCanvasObject* object, a2dCanvasObjectList* towrite );

    void DoSave( a2dPolylineL* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dPolygonL* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dSurface* surface, a2dCanvasObjectList* towrite );

    void DoSave( a2dText* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dTextGDS* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dCanvasObjectReference* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dCanvasObjectArrayReference* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dCameleonInst* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dCameleon* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dCircle* Circle, a2dCanvasObjectList* towrite );

    //!mapping of GDSII layers to internal layers
    a2dLayerMapNr m_mapping;

    //! the document to store/load the data found into
    a2dCanvasDocument* m_doc;

    double m_scale_out;

    double m_userunits_out;

    int m_objectFlags;

    unsigned int m_points_written;

    wxUint16 m_lastElementLayer;

    //! if true, vonvert text to a2dVpath
    bool m_textAsPath;

    bool m_fromViewAsTop;

    a2dLayers* m_layers;

    bool m_asCameleons;

    double m_AberArcToPoly;
};

#endif //wxART2D_USE_KEYIO


#endif //_KEYIO_H
