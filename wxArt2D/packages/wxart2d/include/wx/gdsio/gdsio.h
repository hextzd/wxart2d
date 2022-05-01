/*! \file wx/gdsio/gdsio.h
    \brief GdsII format input and output

    GDSII is an important mask industry interfacing format.
    Primitives on layers, stored in structures. This format was one of the main sources of
    insperation for the design of wxCanvasDocument's. The most important feature is that
    a group/structure can be placed at several location in a drawing without duplicating the data.
    So multiple reference to a drawing, which lead to reference counted objects (a2dObject)
    and smart pointers.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: gdsio.h,v 1.25 2009/06/07 13:28:08 titato Exp $
*/

#ifndef _GDSIO_H
#define _GDSIO_H

#include "a2dprivate.h"

#if wxART2D_USE_GDSIO

#include "wx/canvas/layerinf.h"
#include "wx/canvas/cantext.h"
#include <wx/wfstream.h>

class Strans;
class a2dVpath;
class a2dCanvasDocument;

// when creating a 16 or 32 bits application, the size of the values must be
// the same size

typedef wxByte  G_BYTE;     // 8 bits   -127    128

// 32 bits application definition
typedef wxUint16    TWO_G_BYTE_UNSIGNED_INTEGER;
typedef wxUint32    FOUR_G_BYTE_UNSIGNED_INTEGER;
typedef wxInt16     TWO_G_BYTE_SIGNED_INTEGER;
typedef wxInt32     FOUR_G_BYTE_SIGNED_INTEGER;
typedef wxFloat32   FOUR_G_BYTE_REAL;
typedef wxFloat64   EIGHT_G_BYTE_REAL;


// constants for file i/o used by the written routines
#define GDS_ERROR   -1

// definition gdsii record constants:
#define GDS_DEFAULT_STRING_LENGTH                           44      // default size of and GDS-II string
const int GDS_HEADERSIZE              =                     4;      // size of header
#define GDSTYPE_TWO_G_BYTE_UNSIGNED_INTEGER                 7       // record contains 2 byte unsigned integers
#define GDSTYPE_FOUR_G_BYTE_UNSIGNED_INTEGER                8       // record contains 4 byte unsigned integers

// gdsii data type constants:
#define GDSTYPE_NO_DATA                                     0       // no data in record contained
#define GDSTYPE_BIT_ARRAY                                   1       // record contains bitarray vars
#define GDSTYPE_TWO_G_BYTE_SIGNED_INTEGER                   2       // record contains 2 byte integers
#define GDSTYPE_FOUR_G_BYTE_SIGNED_INTEGER                  3       // record contains 4 byte integers
#define GDSTYPE_FOUR_G_BYTE_REAL                            4       // record contains 4 byte reals
#define GDSTYPE_EIGHT_G_BYTE_REAL                           5       // record contains 8 byte reals
#define GDSTYPE_ASCII_STRING                                6       // record contains GDSII ASCII-strings

// gdsiifile recordtype constants:
// see calma document
#define GDS_HEADER                                          0
#define GDS_BGNLIB                                          1
#define GDS_LIBNAME                                         2
#define GDS_UNITS                                           3
#define GDS_ENDLIB                                          4
#define GDS_BGNSTR                                          5
#define GDS_STRNAME                                         6
#define GDS_ENDSTR                                          7
#define GDS_BOUNDARY                                        8
#define GDS_PATH                                            9
#define GDS_SREF                                            10
#define GDS_AREF                                            11
#define GDS_TEXT                                            12
#define GDS_LAYER                                           13
#define GDS_DATATYPE                                        14
#define GDS_WIDTH                                           15
#define GDS_XY                                              16
#define GDS_ENDEL                                           17
#define GDS_SNAME                                           18
#define GDS_COLROW                                          19
#define GDS_TEXTNODE                                        20
#define GDS_NODE                                            21
#define GDS_TEXTTYPE                                        22
#define GDS_PRESENTATION                                    23
#define GDS_SPACING                                         24
#define GDS_STRING                                          25
#define GDS_STRANS                                          26
#define GDS_MAG                                             27
#define GDS_ANGLE                                           28
#define GDS_UINTEGER                                        29
#define GDS_USTRING                                         30
#define GDS_REFLIBS                                         31
#define GDS_FONTS                                           32
#define GDS_PATHTYPE                                        33
#define GDS_GENERATIONS                                     34
#define GDS_ATTRTABLE                                       35
#define GDS_STYPTABLE                                       36
#define GDS_STRTYPE                                         37
#define GDS_ELFLAGS                                         38
#define GDS_ELKEY                                           39
#define GDS_LINKTYPE                                        40
#define GDS_LINKKEYS                                        41
#define GDS_NODETYPE                                        42
#define GDS_PROPATTR                                        43
#define GDS_PROPVALUE                                       44
#define GDS_BOX                                             45
#define GDS_BOXTYPE                                         46
#define GDS_PLEX                                            47
#define GDS_BGNEXTN                                         48
#define GDS_ENDEXTN                                         49
#define GDS_TAPENUM                                         50
#define GDS_TAPECODE                                        51
#define GDS_STRCLASS                                        52
#define GDS_RESERVED                                        53
#define GDS_FORMAT                                          54
#define GDS_MASK                                            55
#define GDS_ENDMASKS                                        56
// The following 3 items are from the Cadence Construct v4.0 GDS-extention
//  and not part of the original GDS-II format (added for read compatibility)
#define GDS_LIBDIRSIZE                                      57
#define GDS_SRFNAME                                         58
#define GDS_LIBSECUR                                        59

class Boundary;
class a2dCanvasObject;
class Path;
class a2dCameleonInst;
class a2dCameleon;

//
// defaults for Strans
//
#define DEFAULT_STRANS_ANGLE            0
#define DEFAULT_STRANS_SCALE            1.0
#define DEFAULT_STRANS_ABS_ANGLE        false
#define DEFAULT_STRANS_ABS_SCALE        false
#define DEFAULT_STRANS_REFLECTION   false

//! holds GDS-II info for reflection and positioning
typedef struct gds_strans_bitflags
{
#if wxBYTE_ORDER == wxBIG_ENDIAN
    unsigned    reflection: 1;
    unsigned    reserved1 : 7;
    unsigned    reserved2 : 5;
    unsigned    abs_scale : 1;
    unsigned    abs_angle : 1;
    unsigned    reserved  : 1;
#else
    unsigned    reserved  : 1;
    unsigned    abs_angle : 1;
    unsigned    abs_scale : 1;
    unsigned    reserved2 : 5;
    unsigned    reserved1 : 7;
    unsigned    reflection: 1;
#endif
} gds_strans_bitflags ;

//! class to hold GDS-II transformation info, only used within the GDSII reader
class A2DCANVASDLLEXP Strans
{
public:
    //! constructor
    Strans( double nw_angle = DEFAULT_STRANS_ANGLE,
            double nw_scale = DEFAULT_STRANS_SCALE,
            bool nw_abs_angle = DEFAULT_STRANS_ABS_ANGLE,
            bool nw_abs_scale = DEFAULT_STRANS_ABS_SCALE,
            bool nw_reflection = DEFAULT_STRANS_REFLECTION );

    Strans& operator=( Strans& a_strans );

    //! create an Strans record for GDSII file
    void MakeStrans( const a2dAffineMatrix& matrix );

    bool   GetStrans();
    void   SetAngle( double );
    void   SetScale( double );
    void   SetAbsAngle( bool );
    void   SetAbsScale( bool );
    void   SetReflection( bool );
    double GetAngle();
    double GetScale();
    bool   GetAbsAngle();
    bool   GetAbsScale();
    bool   GetReflection();

    //! Strans bit flags for reading and writing to binary file
    union
    {
        short int total;
        gds_strans_bitflags bits;
    } m_stransflags;

protected:
    double  m_angle;
    double  m_scale;
    bool    m_strans_need;
};


A2DCANVASDLLEXP extern const a2dError a2dError_GDSIIparse; /*!< GDSII parsing error*/

//!Input driver for GDSII files
/*!
    Every record in the GDSII file is read/mapped into the right internal structure
    The GDSII file is parsed according to the BNF of the GDSII format.
    Info on the different record types that are read can be found in the GDSII format manual
    All basic data types are read using a specialized function that now how those types
    are stored in GDSII.
    For layers settings this handler checks for a file with the same name as the document that is loaded,
    but with extension ".cvg", if it does exist it will be used to read layers (in CVG format).

    \ingroup fileio gdsfileio
*/
class A2DCANVASDLLEXP a2dIOHandlerGDSIn : public a2dIOHandlerStrIn
{

public:

    a2dIOHandlerGDSIn();
    ~a2dIOHandlerGDSIn();

    bool LinkReferences();

    virtual bool CanLoad( a2dDocumentInputStream& stream, const wxObject* obj = NULL, wxClassInfo* docClassInfo = NULL  );

    virtual bool Load( a2dDocumentInputStream& stream, wxObject* doc );

    virtual void InitializeLoad(  );

    virtual void ResetLoad();

    //type of font to use for text
    void SetFontType( a2dFontType fontType ) { m_fontType = fontType; }

    void SetReadAsCameleon( bool asCameleons ) { m_asCameleons = asCameleons; }

    bool GetReadAsCameleon() { return m_asCameleons; }

    void SetReadAsMultiroot( bool asMultiroot ) { m_asMultiroot = asMultiroot; }

    bool GetReadAsMultiroot() { return m_asMultiroot; }

protected:

    void SetDataTypeProperty(  a2dCanvasObject* toobject, wxUint16 type );

    unsigned int ReadUnsignedInt();

    int ReadInt();

    long ReadLong();

    float ReadFloat();

    double ReadDouble();

    //! convert a gdsii real to a machine real
    void gds2double( unsigned char fb[] );

    void ReadGdsII();
    bool ReadBgnlib();
    bool ReadProperty();
    bool ReadBoundary( a2dCanvasObject* parent );

    bool ReadPath( a2dCanvasObject* parent );
    bool ReadText( a2dCanvasObject* parent );
    bool ReadStructureReference( a2dCanvasObject* parent );
    bool ReadStructureCameleonReference( a2dCanvasObject* parent );
    bool ReadArrayReference( a2dCanvasObject* parent );
    bool ReadNode( a2dCanvasObject* parent );
    bool ReadBox( a2dCanvasObject* parent );
    bool ReadElflags( a2dCanvasObject* object );
    bool ReadGdsHeader();
    bool ReadLayer( a2dCanvasObject* object );
    bool ReadLayer( int& layer );
    bool ReadLibraryName();
    bool ReadLibrarySettings();
    bool Read( a2dPoint2D& _Point );
    bool ReadStrans( Strans& _Strans );
    bool ReadStructure( a2dCanvasObject* parent );
    bool ReadCameleonStructure( a2dCanvasObject* parent );
    bool ReadElement( a2dCanvasObject* parent );
    bool ReadTextBody();

    bool Read( a2dVertexList* segments );

    bool Read( a2dPolygonL* polygon );

    //!read the specified record type from the GDSII file
    //!if the last record is not this type, it is stored for the next read
    bool ReadHeader( int type );

    //!skip the date of the last read record
    void SkipData();

    //!read string data for Text element, structure names etc.
    void ReadString( wxString& a_string );

    //!last read record is stored
    bool  m_back;

    //!size of the last read record
    TWO_G_BYTE_UNSIGNED_INTEGER m_recordsize;

    //!recordtype of last read record
    G_BYTE m_recordtype;

    //!the type of the data in the last read record
    G_BYTE m_datatype;

    //!mapping of GDSII layers to internal layers
    a2dLayerMapNr m_mapping;

    a2dLayers* m_layers;

    //! the document to store/load the data found into
    a2dCanvasDocument* m_doc;

    double m_userunits_out;

    int m_objectFlags;

    //type of font to use for text
    a2dFontType m_fontType;

    //! structure that is being read at the moment
    wxString m_strucname;

    int m_refMaxx; 
    int m_dx, m_dy;

    bool m_asCameleons;
    bool m_asMultiroot;
    double m_gridx, m_gridy;
};


//!Output driver for GDSII files
/*!
    Every record in the GDSII file is read/mapped into the right gds file structure
    Info on the different record types that are read can be found in the GDSII format manual

    \ingroup fileio gdsfileio
*/
class A2DCANVASDLLEXP a2dIOHandlerGDSOut : public a2dIOHandlerStrOut
{

public:

    a2dIOHandlerGDSOut();
    ~a2dIOHandlerGDSOut();

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

protected:

    unsigned int ReadUnsignedInt();

    int ReadInt();

    long ReadLong();

    float ReadFloat();

    double ReadDouble();

    void WriteByte( G_BYTE ch );

    void WriteUnsignedInt( TWO_G_BYTE_UNSIGNED_INTEGER i );

    void WriteInt( TWO_G_BYTE_SIGNED_INTEGER i );

    void WriteLong( FOUR_G_BYTE_SIGNED_INTEGER i );

    void WriteFloat( FOUR_G_BYTE_REAL i );

    void WriteDouble( EIGHT_G_BYTE_REAL i );

    void Write( Strans* Strans );

    void WriteHeader( TWO_G_BYTE_UNSIGNED_INTEGER nw_recordsize, G_BYTE nw_datatype, G_BYTE nw_recordtype );

    void WriteString( const wxString& a_string );

    void SetFlags( a2dCanvasObject* Element );

    void WritePoint( double xi, double yi );

    void WriteVpath( const a2dAffineMatrix& lworld, const a2dVpath* path, int layer, int datatype, int pathtype, double width );

    void WritePath( const a2dAffineMatrix& lworld, a2dVertexArray* points, int layer, int datatype, int pathtype, double width );

    void WritePath( const a2dAffineMatrix& lworld, a2dVertexList* points, int layer, int datatype, int pathtype, double width );

    void WriteBoundary( const a2dAffineMatrix& lworld, a2dVertexArray* points, int layer, int datatype );

    void WriteBoundary( const a2dAffineMatrix& lworld, a2dVertexList* points, int layer, int datatype, bool spline = false );

    void SaveStructure( a2dCanvasObject* object, a2dCanvasObjectList* towrite );

    void Save( a2dCanvasObject* object, a2dCanvasObjectList* towrite );

    void DoSave( a2dPolylineL* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dPolygonL* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dVectorPath* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dText* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dTextGDS* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dCanvasObjectReference* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dCameleonInst* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dCameleon* obj, a2dCanvasObjectList* towrite );

    void DoSave( a2dCanvasObjectArrayReference* obj, a2dCanvasObjectList* towrite );

    //! convert a machine real to a gdsii real
    void double2gds( unsigned char fb[] );

    //!last read record is stored
    bool  m_back;

    //!size of the last read record
    TWO_G_BYTE_UNSIGNED_INTEGER m_recordsize;

    //!recordtype of last read record
    G_BYTE m_recordtype;

    //!the type of the data in the last read record
    G_BYTE m_datatype;

    //!mapping of GDSII layers to internal layers
    a2dLayerMapNr m_mapping;

    //! the document to store/load the data found into
    a2dCanvasDocument* m_doc;

    double m_scale_out;

    double m_userunits_out;

    int m_objectFlags;

    //! if true, convert text to a2dVpath
    bool m_textAsPath;

    bool m_fromViewAsTop;

    a2dLayers* m_layers;

    bool m_asCameleons;

    double m_AberArcToPoly;
};

#endif //wxART2D_USE_GDSIO

#endif //_GDSIO_H
