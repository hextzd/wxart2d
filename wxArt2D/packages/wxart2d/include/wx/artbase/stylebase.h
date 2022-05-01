/*! \file wx/artbase/stylebase.h

    \brief Stroke and fill base classes.

    Holds style classes, derived from a2dObject. All wxDrawer2D understand the style classes
    which are set as a2dStroke a2dFill to them.
    Style classes can be added to a2dCanvasObject's as properties, and this is how these object
    can be drawn colours which are customizable by the user.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: stylebase.h,v 1.57 2009/10/01 19:22:34 titato Exp $
*/


#ifndef __STYLEBASE_H__
#define __STYLEBASE_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/fontenc.h>
#include <wx/encconv.h>
#include <wx/bitmap.h>

#include "a2dprivate.h"

#include "wx/geometry.h"

#include <map>

using namespace std;


#if wxART2D_USE_FREETYPE
#include <ft2build.h>
#ifdef _MSC_VER
// Do not use the define include for MSVC, because of a bug with precompiled
// headers. See (freetype)/doc/INSTALL.ANY for (some) more information
#include <freetype.h>
#include <ftglyph.h>
#include <ftoutln.h>
#else // _MSC_VER
// This is the way it should be included.
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <freetype/ftoutln.h>
#endif // _MSC_VER
#endif

#include "wx/general/gen.h"

#if wxART2D_USE_XMLPARSE
#include "wx/xmlparse/genxmlpars.h"
#endif //wxART2D_USE_XMLPARSE

#include "wx/artbase/artglob.h"
#include "wx/artbase/liner.h"
#include "wx/artbase/afmatrix.h"
#include "wx/artbase/bbox.h"

//! Normalization font size used for bigger size drawing of text.
#define NORMFONT 1000
#define SMALLTEXTTHRESHOLD 10

class A2DARTBASEDLLEXP a2dBoundingBox;
class A2DARTBASEDLLEXP a2dAffineMatrix;

class a2dDrawer2D;
class a2dVpath;

//! General colour property id
extern a2dPropertyIdColour PROPID_colour;

//!Filling styles for a2dFill
/*!
    Transparent filling when bitmap/patternfill is monochrome and fill style is
    wxSTIPPLE_MASK_OPAQUE_TRANSPARENT.
    The fill colour will be used for non Transparent parts in the brush bitmap,
    while other parts will be transparent.
    a2dFILL_GRADIENT_FILL_LINEAR gradient filling using lines changing in colour from fill colour1 to fill colour2
    a2dFILL_GRADIENT_FILL_RADIAL, gradient filling using circles changing in colour from fill colour1 to fill colour2

    \ingroup  style
*/
enum a2dFillStyle
{
    a2dFILL_TRANSPARENT,      /*!<  No fill is used.*/
    a2dFILL_SOLID,            /*!<  Solid style. */
    a2dFILL_BDIAGONAL_HATCH,  /*!<  Backward diagonal hatch.*/
    a2dFILL_CROSSDIAG_HATCH,  /*!<  Cross-diagonal hatch.*/
    a2dFILL_FDIAGONAL_HATCH,  /*!<  Forward diagonal hatch.*/
    a2dFILL_CROSS_HATCH,      /*!<  Cross hatch.*/
    a2dFILL_HORIZONTAL_HATCH, /*!<  Horizontal hatch.*/
    a2dFILL_VERTICAL_HATCH,   /*!<  Vertical hatch.*/
    a2dFIRST_HATCH = a2dFILL_BDIAGONAL_HATCH,
    a2dLAST_HATCH = a2dFILL_VERTICAL_HATCH,
    a2dFILL_TWOCOL_BDIAGONAL_HATCH,  /*!<  Backward diagonal hatch.*/
    a2dFILL_TWOCOL_CROSSDIAG_HATCH,  /*!<  Cross-diagonal hatch.*/
    a2dFILL_TWOCOL_FDIAGONAL_HATCH,  /*!<  Forward diagonal hatch.*/
    a2dFILL_TWOCOL_CROSS_HATCH,      /*!<  Cross hatch.*/
    a2dFILL_TWOCOL_HORIZONTAL_HATCH, /*!<  Horizontal hatch.*/
    a2dFILL_TWOCOL_VERTICAL_HATCH,   /*!<  Vertical hatch.*/
    a2dFIRST_TWOCOL_HATCH = a2dFILL_TWOCOL_BDIAGONAL_HATCH,
    a2dLAST_TWOCOL_HATCH = a2dFILL_TWOCOL_VERTICAL_HATCH,
    a2dFILL_STIPPLE,          /*!<  Use the stipple bitmap.*/
    a2dFILL_STIPPLE_MASK_OPAQUE, /*!<  Use the stipple bitmap.*/
    a2dFILL_STIPPLE_MASK_OPAQUE_TRANSPARENT, /*!<  Use the stipple bitmap.*/
    a2dFILL_GRADIENT_FILL_XY_LINEAR,  /*!< linear scan from colour 1 to colour two .*/
    a2dFILL_GRADIENT_FILL_XY_RADIAL, /*!< circular scan from colour 1 to colour two .*/
    a2dFILL_GRADIENT_FILL_XY_DROP,  /*!< sort of side scan from colour 1 to colour two .*/
    a2dFILL_INHERIT, /*!<  Use the style of the parent object*/
    a2dFILL_LAYER, /*!<  Use the style of the layer from the layersettings */
    a2dFILL_NULLFILL         /*!<  special NULL fill used for a2dNullFill */
};

//! for a2dFill to define the type of filling.
/*!
    \ingroup style
*/
enum a2dFillType
{
    a2dFILL_NULL, /*!< null type */
    a2dFILL_ONE_COLOUR, /*!< one colour */
    a2dFILL_HATCH_TWO_COLOUR, /*!< two colour hatch*/
    a2dFILL_GRADIENT_FILL_LINEAR, /*!< linear scan from colour one to colour two */
    a2dFILL_GRADIENT_FILL_RADIAL, /*!< circular scan from colour one to colour two */
    a2dFILL_BITMAP, /*!< using a bitmap */
};

/*!
    To set fill on a2dDrawer2D and a2dCanvasObject classes.

    When style is set a2dFILL_TRANSPARENT, the rest of the fill data is preserved until style is changed.

    \ingroup property style
*/
class A2DARTBASEDLLEXP a2dFill : public wxObject
{

#ifdef CLASS_MEM_MANAGEMENT
    //! memory manager for speed up to replace system calls allocation and deallocation
    static a2dMemManager sm_memManager;
public:
    //! overloaded operator new for this class and all it's derived classes
    void* operator new( size_t bytes )
    {
        return sm_memManager.Allocate( bytes );
    }

    //! overloaded operator delete for this class and all it's derived classes
    /*!
    This function doesn't free to OS-system memory block by pointer 'space'.
    It adds memory block by pointer 'space' to internal lists.
    It is speed up.
    */
    void operator delete( void* space, size_t bytes )
    {
        sm_memManager.Deallocate( space, bytes );
    }
#endif //CLASS_MEM_MANAGEMENT

public:

    //!constructor
    a2dFill( void );

    a2dFill( const a2dFill& fill );

    a2dFill( const wxBrush& brush );

    a2dFill( const wxColour& col, a2dFillStyle style = a2dFILL_SOLID );

    a2dFill( const wxColour& col, const wxColour& col2, a2dFillStyle style );

    a2dFill( const wxBitmap& stipple, a2dFillStyle style = a2dFILL_STIPPLE );

    a2dFill( const wxBitmap& stipple, const wxColour& col, const wxColour& col2, a2dFillStyle style = a2dFILL_STIPPLE );

    a2dFill( const wxFileName& filename, a2dFillStyle style = a2dFILL_STIPPLE );

    a2dFill( const wxFileName& filename, const wxColour& col, const wxColour& col2, a2dFillStyle style = a2dFILL_STIPPLE );

    a2dFill( const wxColour& col, const wxColour& col2,
             double x1, double y1, double x2, double y2 );

    a2dFill( const wxColour& col, const wxColour& col2,
             double xfc, double yfc, double xe, double ye, double radius );

    bool operator==( const a2dFill& a )
    {
        return m_refData == a.m_refData;
    }

    bool operator!=( const a2dFill& a )
    {
        return m_refData != a.m_refData;
    }

    friend inline bool operator==( const a2dFill& a, const a2dFill& b )
    {
        return a.m_refData == b.m_refData;
    }

    friend inline bool operator!=( const a2dFill& a, const a2dFill& b )
    {
        return a.m_refData != b.m_refData;
    }

    a2dFill& UnShare();

    wxObjectRefData* CreateRefData() const;

    wxObjectRefData* CloneRefData( const wxObjectRefData* data ) const;

    //!destructor
    virtual ~a2dFill( void );

    a2dFillType GetType( void ) const;

    a2dFillStyle GetStyle() const;

    void SetStyle( a2dFillStyle style );

    a2dFillType GetTypeForStyle( a2dFillStyle style ) const;

    void SetStipple( const wxBitmap& stipple );

    const wxBitmap& GetStipple() const;

    //! return colour
    wxColour GetColour() const;

    //! return colour 2
    wxColour GetColour2() const;

    //! set colour  used for gradient and wxSTIPPLE_MASK_OPAQUE filling.
    void SetColour( const wxColour& col );

    //! set colour 2 used for gradient and wxSTIPPLE_MASK_OPAQUE filling.
    void SetColour2( const wxColour& col );

    void SetAlpha( wxUint8 val );
    wxUint8 GetAlpha() const;

    //!Get start for gradient fill colour change
    a2dPoint2D GetStart() const;

    //!Get stop for gradient fill colour change
    a2dPoint2D GetStop() const;

    //! time stamp gradient fill
    wxUint32 GetStamp() const;

    //!Get start for gradient fill colour change
    a2dPoint2D GetCenter() const;

    //!Get start for gradient fill colour change
    a2dPoint2D GetFocal() const;

    //!Get stop for gradient fill colour change
    double GetRadius() const;

    //! used to define fills that are not yielding (either need to be deleted,
    //! or indicate that there is no fill defined for an object.
    //! This is can be used on any fill. This method, is handy since one can still clone
    //! such fills.
    bool IsNoFill() const { return m_refData == NULL; }

    //! from now one this object defines as if no stroke is defined of the object having this stroke.
    a2dFill* SetNoFill() { UnRef(); return this; }

    //! is set true, filling is on else filling is transparent.
    //! This is to quickly switch to filling or not, without changing style.
    void SetFilling( bool OnOff );

    //! if true, filling is on else filling is transparent.
    bool GetFilling() const;

    DECLARE_DYNAMIC_CLASS( a2dFill )

#if wxART2D_USE_CVGIO
    virtual void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser );
    virtual void Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

};

//! stroke styles for a2dStroke
/*!
    \ingroup style
*/
enum a2dStrokeStyle
{
    a2dSTROKE_TRANSPARENT, /*!<  No stroke is used.*/
    a2dSTROKE_SOLID, /*!<  Solid style. */
    a2dSTROKE_DOT, /*!< Dotted style.*/
    a2dSTROKE_LONG_DASH, /*!<  Long dashed style.*/
    a2dSTROKE_SHORT_DASH, /*!<  Short dashed style.*/
    a2dSTROKE_DOT_DASH, /*!<  Dot and dash style.*/
    a2dSTROKE_USER_DASH, /*!<  Use the user dashes*/
    a2dSTROKE_BDIAGONAL_HATCH, /*!<  Backward diagonal hatch.*/
    a2dSTROKE_CROSSDIAG_HATCH, /*!<  Cross-diagonal hatch.*/
    a2dSTROKE_FDIAGONAL_HATCH, /*!<  Forward diagonal hatch.*/
    a2dSTROKE_CROSS_HATCH, /*!<  Cross hatch.*/
    a2dSTROKE_HORIZONTAL_HATCH, /*!<  Horizontal hatch.*/
    a2dSTROKE_VERTICAL_HATCH, /*!<  Vertical hatch.*/
    a2dSTROKE_STIPPLE, /*!<  Use the stipple bitmap.*/
    a2dSTROKE_STIPPLE_MASK_OPAQUE, /*!<  Use the stipple bitmap.*/
    a2dSTROKE_STIPPLE_MASK_OPAQUE_TRANSPARENT, /*!<  Use the stipple bitmap.*/
    a2dSTROKE_OBJECTS, /*!<  Use a a2dCanvasObject. */
    a2dSTROKE_INHERIT, /*!<  Use the style of the parent object*/
    a2dSTROKE_LAYER, /*!<  Use the style of the layer from the layersettings */
    a2dSTROKE_NULLSTROKE       /*!<  special NULL fill used for a2dNullStroke */
};

//! stroke types
//! TODO ( only Xwindows ) a2dPatternStroke(const wxBitmap& stipple, double width);
//! TODO ( only Xwindows ) a2dPatternStroke(const wxBitmap& stipple, int width);
enum a2dStrokeType
{
    a2dSTROKE_NULL,
    a2dSTROKE_OBJECT,
    a2dSTROKE_NORMAL, /*!< not using a bitmap or gradient */
    a2dSTROKE_ONE_COLOUR, /*!< one colour */
    //a2dSTROKE_GRADIENT_FILL_LINEAR, /*!< linear scan from colour one to colour two */
    //a2dSTROKE_GRADIENT_FILL_RADIAL, /*!< circular scan from colour one to colour two */
    a2dSTROKE_BITMAP, /*!< using a bitmap */
};

typedef float a2dDash;

#if !wxCHECK_VERSION(2,9,0)
typedef int wxPenJoin;
typedef int wxPenCap;
typedef int wxPolygonFillMode;
typedef int wxRasterOperationMode;

// NOTE: these values cannot be combined together!
enum wxBrushStyle
{
    wxBRUSHSTYLE_INVALID = -1,

    wxBRUSHSTYLE_SOLID = wxSOLID,
    wxBRUSHSTYLE_TRANSPARENT = wxTRANSPARENT,
    wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE = wxSTIPPLE_MASK_OPAQUE,
    wxBRUSHSTYLE_STIPPLE_MASK = wxSTIPPLE_MASK,
    wxBRUSHSTYLE_STIPPLE = wxSTIPPLE,
    wxBRUSHSTYLE_BDIAGONAL_HATCH = wxBDIAGONAL_HATCH,
    wxBRUSHSTYLE_CROSSDIAG_HATCH = wxCROSSDIAG_HATCH,
    wxBRUSHSTYLE_FDIAGONAL_HATCH = wxFDIAGONAL_HATCH,
    wxBRUSHSTYLE_CROSS_HATCH = wxCROSS_HATCH,
    wxBRUSHSTYLE_HORIZONTAL_HATCH = wxHORIZONTAL_HATCH,
    wxBRUSHSTYLE_VERTICAL_HATCH = wxVERTICAL_HATCH,
    wxBRUSHSTYLE_FIRST_HATCH = wxFIRST_HATCH,
    wxBRUSHSTYLE_LAST_HATCH = wxLAST_HATCH
};

#endif

//! Base class for all types of strokes, understood by a2dDrawer2D classes.
/*!
Any shape that needs to be stroked gets a a2dStroke derived class set for it.
The derived classes define the way the stroking is done.
The a2dDrawer2D class needs to understand the different styles of stroking.

The stroke can be set to be of pixel type,
which means it is a fixed size in pixels.
Depending on the constructor used a pixel or non pixel stroke will be created.

    \ingroup property style
*/
class A2DARTBASEDLLEXP a2dStroke : public wxObject
{
#ifdef CLASS_MEM_MANAGEMENT

    //! memory manager for speed up to replace system calls allocation and deallocation
    static a2dMemManager sm_memManager;
public:
    //! overloaded operator new for this class and it all derived classes
    void* operator new( size_t bytes )
    {
        return sm_memManager.Allocate( bytes );
    }

    //! overloaded operator delete for this class and it all derived classes
    /*!
    This function doesn't free to OS-system memory block by pointer 'space'.
    It adds memory block by pointer 'space' to internal lists.
    It is speed up.
    */
    void operator delete( void* space, size_t bytes )
    {
        sm_memManager.Deallocate( space, bytes );
    }
#endif //CLASS_MEM_MANAGEMENT

public:

    //!constructor
    a2dStroke( void );

    a2dStroke( const wxColour& col, a2dStrokeStyle style = a2dSTROKE_SOLID );

    a2dStroke( const wxColour& col, double width, a2dStrokeStyle style = a2dSTROKE_SOLID );

    //!width defined in world coordinates.
    a2dStroke( const wxColour& col, int width, a2dStrokeStyle style = a2dSTROKE_SOLID );

    a2dStroke( const wxBitmap& stipple, a2dStrokeStyle style = a2dSTROKE_STIPPLE );

    a2dStroke( const wxBitmap& stipple, const wxColour& col, const wxColour& col2, a2dStrokeStyle style = a2dSTROKE_STIPPLE );

    a2dStroke( const wxFileName& filename, a2dStrokeStyle style = a2dSTROKE_STIPPLE );

    a2dStroke( const wxFileName& filename, const wxColour& col, const wxColour& col2, a2dStrokeStyle style = a2dSTROKE_STIPPLE );

    //!constructor
    a2dStroke( const a2dStroke& stroke );

    //!width gets defined in pixels.
    a2dStroke( const wxPen& stroke );

    //!destructor
    virtual ~a2dStroke( void );

    a2dStroke& UnShare();

    bool operator==( const a2dStroke& a )
    {
        return m_refData == a.m_refData;
    }

    bool operator!=( const a2dStroke& a )
    {
        return m_refData != a.m_refData;
    }

    friend inline bool operator==( const a2dStroke& a, const a2dStroke& b )
    {
        return a.m_refData == b.m_refData;
    }

    friend inline bool operator!=( const a2dStroke& a, const a2dStroke& b )
    {
        return a.m_refData != b.m_refData;
    }

    wxObjectRefData* CreateRefData() const;

    wxObjectRefData* CloneRefData( const wxObjectRefData* data ) const;

    virtual a2dStrokeType GetType( void ) const;

    void SetStipple( const wxBitmap& stipple );

    const wxBitmap& GetStipple() const;

    //! return colour 1
    wxColour GetColour() const;

    //! set colour  used for gradient and wxSTROKE_MASK_OPAQUE filling.
    void SetColour( const wxColour& col );

    //! return colour 2
    wxColour GetColour2() const;

    //! set colour 2 used for gradient and wxSTROKE_MASK_OPAQUE filling.
    void SetColour2( const wxColour& col );

    void SetJoin( wxPenJoin join_style );

    void SetCap( wxPenCap cap_style );

    wxPenCap GetCap() const;

    wxPenJoin GetJoin() const;

    //!Set width of stroke in world coordinates.
    void SetWidth( float width );

    //!Get width of stroke in world or device coordinates.
    //!(depending on pixel or non pixel stroke).
    float GetWidth() const;
    float GetExtend() const;

    //!set pixelstoke flag, stroke width is defined in pixels else in worldcoordinates
    void SetPixelStroke( bool pixelstroke );

    //!if the width is pixels or not.
    bool GetPixelStroke() const;

    void SetAlpha( wxUint8 val );

    wxUint8 GetAlpha() const;

    a2dStrokeStyle GetStyle() const;

    void SetStyle( a2dStrokeStyle style );

    //! used to define strokes that are not yielding (either need to be deleted,
    //! or indicate that there is no stroke defined for an object.
    //! This is can be used on any stroke. This method, is handy since one can still clone
    //! such strokes.
    bool IsNoStroke() const { return m_refData == NULL; }

    //! from now one this object defines as if no stroke is defined of the object having this stroke.
    a2dStroke* SetNoStroke() { UnRef(); return this; }

    a2dStrokeType GetTypeForStyle( a2dStrokeStyle style ) const;

    DECLARE_DYNAMIC_CLASS( a2dStroke )

#if wxART2D_USE_CVGIO
    virtual void Load( wxObject* parent, a2dIOHandlerXmlSerIn& parser );
    virtual void Save( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

};

//----------------------------------------------------------------------------
// a2dFont
//----------------------------------------------------------------------------

enum a2dFontType
{
    a2dFONT_NULL,
    a2dFONT_WXDC,
    a2dFONT_FREETYPE,
    a2dFONT_STROKED,
};

#if defined(__WXMSW__)
#define __USE_WINAPI__
#endif

class a2dVertexList;
//! A list of pointers to vertexlists. Ends with NULL.
typedef a2dVertexList** a2dStrokeGlyph;

#if !wxART2D_USE_FREETYPE
// in case freetype is not used, define replacement types.
typedef void* FT_Glyph;
typedef void* FT_Vector;
typedef void* FT_Face;
typedef unsigned int FT_UInt;
#endif // wxART2D_USE_FREETYPE

//! wxChar hashing function, used in hash maps.
/*!
    \ingroup textrelated
*/
class A2DARTBASEDLLEXP a2dCharHash
{
public:
    a2dCharHash() { }
    unsigned long operator()( const wxChar& k ) const
    { return ( unsigned long ) k; }
    a2dCharHash& operator=( const a2dCharHash& ) { return * this; }
};
//! wxChar equality class, used in hash maps.
/*!
     \class a2dCharEqual stylebase.h artbase/stylebase.h
    \ingroup textrelated
*/
class A2DARTBASEDLLEXP a2dCharEqual
{
public:
    a2dCharEqual() { }
    bool operator()( const wxChar& a, const wxChar& b ) const
    { return ( a == b ); }
    a2dCharEqual& operator=( const a2dCharEqual& ) { return * this; }
};

//! holes one glyph from a freetype font.
struct A2DARTBASEDLLEXP a2dGlyph
{
#if wxART2D_USE_FREETYPE
    FT_UInt  m_index;
    FT_Glyph m_glyph;
    FT_Int  m_lsb;
    FT_Int  m_rsb;
    FT_Face  m_face;
#endif

    a2dGlyph()
    {
#if wxART2D_USE_FREETYPE
        m_index = ( FT_UInt ) - 1; m_glyph = NULL;
#endif
    }
    ~a2dGlyph()
    {
#if wxART2D_USE_FREETYPE
        if( m_glyph )
            FT_Done_Glyph( m_glyph );
#endif
    }
};

/*! \class FT_GlyphHash stylebase.h artbase/stylebase.h
    \brief A hash map, used for storage of freetype glyphs.
    \ingroup textrelated
*/
WX_DECLARE_HASH_MAP_WITH_DECL( wxChar, a2dGlyph, a2dCharHash, a2dCharEqual, a2dGlyphHash, class A2DARTBASEDLLEXP );


/*! \class DcWidthHash stylebase.h artbase/stylebase.h
    \brief A hash map, used for storage of character widths of a font
    \ingroup textrelated
*/
WX_DECLARE_HASH_MAP_WITH_DECL( wxChar, double, a2dCharHash, a2dCharEqual, DcWidthHash, class A2DARTBASEDLLEXP );

/*! \class DcKerningHash stylebase.h artbase/stylebase.h
    \brief A hash map, used for storage of kerning between characters of a font
    \ingroup textrelated
*/
WX_DECLARE_HASH_MAP_WITH_DECL( wxString, double, wxStringHash, wxStringEqual, DcKerningHash, class A2DARTBASEDLLEXP );

//! wxMINY and wxMAXY is defined as lowest and highest of a string boundingbox.
//! Understand on the canvas y axis can be up or down, but text is always written up.
//! Still de size and position of the boundingbox in wordl coordinates stays the same.
//! Fontsize is defined by EM square in world coordinates. And is NOT the highest Y all glyphs en lowest Y.
//! wxMINX (baseline start just before first character)
//! wxMAXX (baseline end after last character)
//! else middle of baseline, being width/2
//! wxBASELINE text origin at baseline
//! wxBASELINE_CONTRA text origin at baseline but seen from top of boundingbox
//! wxMINY lowest Y of bbox.
//! wxMAXY highest Y of bbox.
//! else middle Bbox in Y.
enum a2dFontAlignment
{
    wxMIDY = 0x0000,
    wxMIDX = wxMIDY,
    wxMINX = 0x0001,
    wxMAXX = 0x0002,
    wxMINY = 0x0004,
    wxMAXY = 0x0008,
    wxBASELINE = 0x0010,
    wxBASELINE_CONTRA = 0x0020,
    a2dDEFAULT_ALIGNMENT = wxMAXY | wxMINX
};

class a2dFont;

//! Font info class, used as a single element for enumerating fonts.
/*!
    The a2dFontInfo class is used as a font description class. Fonts can be generated from
    this. It is also used for saving and loading a font.

    This class consists of six items
    - type  currently "Device Font", "Freetype Font" or "Stroke Font", but more might be added in the future.
    - fontname: names like "Arial", "Courier New", etc.
    - style: Font type dependant, e.g. "Bold Italic" or "Pixel Stroke".
    - size: size in world coordinates.
    - extra: This is used for extra information, e.g. the filename of the font.

    When a font description in string form is needed call CreateString(). (or the other way with
    ParseString() ).

    This will create a string in the form of "[FAMILY-LIST] [STYLE-OPTIONS] [SIZE]",  where
    FAMILY-LIST is a comma separated list of families optionally terminated by a comma,
    STYLE_OPTIONS is a whitespace separated list of words where each WORD describes one of
    style, variant, weight, or stretch, and
    SIZE is an decimal number (size in points). Any one of the options may be absent.
    If FAMILY-LIST is absent, then the family_name field of the resulting font description will be
    initialized to NULL. If STYLE-OPTIONS is missing, then all style options will be set to the
    default values. If SIZE is missing, the size in the resulting font description will be set to 0.

    The string representation is compatible with the pango font descriptions.

    e.g.

    Device font-Arial For CAE-Light Italic-300.000000-

    Freetype font-Book Antiqua-Bold Italic-300.000000--ANTQUABI.TTF

    \ingroup style textrelated
*/
class A2DARTBASEDLLEXP a2dFontInfo
{
public:
    //! Constructor
    a2dFontInfo() {}
    //! Constructor
    a2dFontInfo( const wxString& type, const wxString& name, const wxString& style = wxT( "" ),
                 double size = 1.0, const wxString& extra = wxT( "" ) );
    //! Constructor
    a2dFontInfo( const wxString& type, const wxString& name,
                 const wxString& style, const wxString& extra = wxT( "" ) );
    //! Constructor
    a2dFontInfo( const wxString& info );

    //! Assignment operator.
    a2dFontInfo& operator=( const a2dFontInfo& other );

    //! Comparison operator.
    bool operator == ( const a2dFontInfo& other ) const;

    //! Create this enumerated font
    /*! This function will allways return a valid font. NULL will never be returned
    */
    a2dFont CreateFont( double size );

    //! Get type of font, e.g. Freetype or Stroke.
    inline const wxString& GetType() const { return m_type; }
    //! Set type of font, e.g. Freetype or Stroke.
    inline void SetType( const wxString& type ) { m_type = type; }

    //! Get name of font, e.g. Arial.
    inline const wxString& GetName() const { return m_name; }
    //! Set name of font, e.g. Arial.
    inline void SetName( const wxString& name ) { m_name = name; }

    //! Get style of font, e.g. Bold.
    inline const wxString& GetStyle() const { return m_style; }
    //! Set style of font, e.g. Bold.
    inline void SetStyle( const wxString& style ) { m_style = style; }

    //! Get size of the font.
    inline double GetSize() const { return m_size; }
    //! Set size of the font.
    /*!
        Sets the size of the EM square in world coordinates. Characters/glyphs are designed on this square.
        Glyphs can be bigger than it.
        See freetype documentation.
        The EM sqaure is often 2048 (font units) in size, meaning it has thus resolution.
    */
    inline void SetSize( double size ) { m_size = size; }

    //! Assemble a string containing this class in a readable format.
    wxString CreateString() const;

    //! Fill this class from a string.
    void ParseString( wxString string );

    //! Get the extra information ( e.g. the filename of the font )
    inline const wxString& GetExtra() const { return m_extra; }

    //! Set the extra information ( e.g. the filename of the font )
    inline void SetExtra( const wxString& extra ) { m_extra = extra; }

protected:
    //! Font type.
    wxString m_type;
    //! Font name.
    wxString m_name;
    //! Font style.
    wxString m_style;
    //! Font Size.
    double m_size;
    //! Extra information (e.g. the filename of the font).
    wxString m_extra;
};

/*! \class a2dFontInfoList stylebase.h artbase/stylebase.h
    \brief A list used for enumerating fonts.
    \sa a2dFontInfo
    \ingroup textrelated
*/
WX_DECLARE_LIST_WITH_DECL( a2dFontInfo, a2dFontInfoList, class A2DARTBASEDLLEXP );

//----------------------------------------------------------------------------
// a2dFont
//----------------------------------------------------------------------------

//! Defines a font to be set to a2dDrawer2D or stored in a2dCanvsObject etc.
/*!
    There are 3 types of fonts as in a2dFontType. The a2dDrawer2D classes know how to draw strings with these fonts.

    \ingroup style textrelated
*/
class A2DARTBASEDLLEXP a2dFont : public wxObject
{
public:

    a2dFontType GetType( void ) const;

    //! Constructor.
    a2dFont();

    //! Copy constructor.
    a2dFont( const a2dFont& other );

    //! Constructor.
    /*! This constructor tries to locate the font file, belonging to the wxFont class, which
        is then used for drawing. At present only a Windows version is implemented.
        \param font Font format. The size of the font is not used and should be set using the next parameter.
        \param size Size in world coordinates.
        \param encoding endocing of font
    */
    a2dFont( const wxFont& font, double size, a2dFontType type = a2dFONT_WXDC, wxFontEncoding encoding = wxFONTENCODING_DEFAULT  );

    //! Constructor
    /*!
        Creates a font object (see font encoding overview, in the wxWidgets documentation, for the meaning of the last parameter).
        \param size Size in world coordinates.
        \param family Font family, a generic way of referring to fonts without specifying actual facename. One of:
            - wxFONTFAMILY_DEFAULT  Chooses a default font.
            - wxFONTFAMILY_DECORATIVE  A decorative font.
            - wxFONTFAMILY_ROMAN  A formal, serif font.
            - wxFONTFAMILY_SCRIPT  A handwriting font.
            - wxFONTFAMILY_SWISS  A sans-serif font.
            - wxFONTFAMILY_MODERN  A fixed pitch font.
        \param style One of wxFONTSTYLE_NORMAL, wxFONTSTYLE_SLANT and wxFONTSTYLE_ITALIC.
        \param weight One of wxFONTWEIGHT_NORMAL, wxFONTWEIGHT_LIGHT and wxFONTWEIGHT_BOLD.
        \param underline The value can be true or false. At present this has an effect on Windows and Motif 2.x only.
        \param faceName An optional string specifying the actual typeface to be used. If the empty string, a default typeface will chosen based on the family.
        \param encoding An encoding which may be one of
            - wxFONTENCODING_SYSTEM  Default system encoding.
            - wxFONTENCODING_DEFAULT  Default application encoding.
            - wxFONTENCODING_ISO8859_1...15  ISO8859 encodings.
            - wxFONTENCODING_KOI8  The standard Russian encoding for Internet.
            - wxFONTENCODING_CP1250...1252  Windows encodings similar to ISO8859 (but not identical).

            If the specified encoding isn't available, no font is created.

            wxFONTENCODING_DEFAULT is the encoding set by calls to SetDefaultEncoding and which
            may be set to, say, KOI8 to create all fonts by default with KOI8 encoding. Initially,
            the default application encoding is the same as default system encoding.
        \remark
            - If the desired font does not exist, the closest match will be chosen.
            - Under Windows, only scalable TrueType fonts are used.
    */
    a2dFont( double size, wxFontFamily family, wxFontStyle style = wxFONTSTYLE_NORMAL, wxFontWeight weight = wxFONTWEIGHT_NORMAL,
             const bool underline = false, const wxString& faceName = wxT( "" ),
             wxFontEncoding encoding = wxFONTENCODING_DEFAULT );

#if wxART2D_USE_FREETYPE
    //! constructor of Freetype font based on filename of fontfile.
    a2dFont( const wxString& filename, double size = 0.0, wxFontEncoding encoding = wxFONTENCODING_DEFAULT, int faceindex = 0 );
#endif

    //! constructor of stroked font based on filename of fontfile. See a2dFontStrokeData
    a2dFont( const wxString& filename, double size, bool monoSpaced, double weight = 0.0 );

    //! Destructor.
    virtual ~a2dFont();

    //! return true if the same font (same reference)
    bool operator==( const a2dFont& a )
    {
        return m_refData == a.m_refData;
    }

    //! return true if not the same font (same reference)
    bool operator!=( const a2dFont& a )
    {
        return m_refData != a.m_refData;
    }

    //! return true if the same font (same reference)
    friend inline bool operator==( const a2dFont& a, const a2dFont& b )
    {
        return a.m_refData == b.m_refData;
    }

    //! return true if not the same font (same reference)
    friend inline bool operator!=( const a2dFont& a, const a2dFont& b )
    {
        return a.m_refData != b.m_refData;
    }

    //! Check font validity.
    /*! \return True if this object is a valid font, false otherwise.
    */
    bool Ok() const;

    //! Get fontinfo of the font
    const a2dFontInfo& GetFontInfo() const;

    //! Set the font size.
    /*! \param size Font size in world coordinates. */
    void SetSize( double size );

    //! Get the font size.
    /*! \return The font size in world coordinates */
    double GetSize() const;

    //! Get the line height.
    /*! The line height is calculated from the text bounding box.
        \return The line height in world coordinates.
    */
    double GetLineHeight() const;

    //! Get descent
    /*! The descent is the distance from the basline to the bottom of the bounding box.
        \return The descent in world coordinates.
    */
    double GetDescent() const;

    //! Get width of a single character
    double GetWidth( wxChar c );

    //! Get kerning space between two characters.
    /*! Kerning is a method, which moves characters closer together/further apart, to
        improve the look of the font. Eg. the letter combination LT will be moved closer together.
        The return value is negative or positive, to respectively move characters closer together, or further
        apart.
    */
    double GetKerning( wxChar WXUNUSED( c1 ), wxChar WXUNUSED( c2 ) ) const;

    //! Get the linegap (external lead).
    /*! Return line gap between lines of text.

        \return The linegap in world coordinates.
    */
    double GetExternalLead() const;

    //! Get stroke width.
    /*!
        \return The stroke width in world coordinates. In case of a pixel font, 0.0 is returned.
        \sa GetWeight()
    */
    double GetStrokeWidth() const;

    //! Get the glyph of a single character.
    /*! \return The glyph of the character, in a coordinate space, normalized to the font size.
    */
    a2dStrokeGlyph GetGlyphStroke( wxChar c ) const;

    const a2dGlyph* GetGlyphFreetype( wxChar c ) const;

    void SetDeviceHeight( double sizeInPixels );

    double GetDeviceHeight();

#if wxART2D_USE_FREETYPE && defined(__USE_WINAPI__)
    //! Get the freetype equivalent of the font
    a2dFont GetFreetypeFont();

    void SetFreetypeFont( a2dFont font );
#endif

    //! wxFont in case of a2dFONT_WXDC
    wxFont& GetFont( void );

    //! Get the dimensions in world coordinates of the string.
    /*! \param string The text string to measure (should be a single line of text).
        \param alignment Alignment of the text (eg. wxLEFT)
        \param w Total width in world coordinates.
        \param h Total height in world coordinates.
        \param descent The dimension from the baseline of the font to the bottom of the descender in
         world coordinates.
        \param externalLeading Any extra vertical space added to the font by the font designer in
            world coordinates (usually is zero).
        \return A bounding box, with the anchor point positioned at (0, 0).
    */
    a2dBoundingBox GetTextExtent( const wxString& string,
                                  int alignment = wxMINX | wxMINY,  double* w = NULL,
                                  double* h = NULL, double* descent = NULL,
                                  double* externalLeading = NULL ) const;

    //! Get the dimensions in world coordinates of the string.
    /*! \param string The text string to measure (should be a single line of text).
        \param w Total width in world coordinates.
        \param h Total height in world coordinates.
        \param descent The dimension from the baseline of the font to the bottom of the descender in
         world coordinates.
        \param externalLeading Any extra vertical space added to the font by the font designer in
            world coordinates (usually is zero).
    */
    void GetTextExtent( const wxString& string, double& w, double& h,
                        double& descent, double& externalLeading ) const;

    //! Get the dimensions in world coordinates of the string.
    /*! \param string The text string to measure (should be a single line of text).
        \param w Total width in world coordinates.
        \param h Total height in world coordinates.
    */
    void GetTextExtentWH( const wxString& string, double& w, double& h ) const;

    bool GetPartialTextExtents  ( const wxString&   text,  wxArrayInt&   widths  )   const;

    //! Get the glyph of a single character as a vpath.
    /*!  The position, indicated by x and y, is the lowerleft bbox corner of the glyph.
        \param c The character.
        \param glyph A vectorpath, to which the glyph is added.
        \param affine Position etc., where the glyph is constructed.
        \return A boolean, indicating if the glyph was succesfully added.
    */
    bool GetVpath( wxChar c, a2dVpath& glyph, const a2dAffineMatrix& affine = a2dIDENTITY_MATRIX );

    //! Render a text string into a vector path
    /*!  The position, indicated by x and y, is the lowerleft bbox corner of the text.
        \param text The string to render.
        \param vpath The vectorpath, to which the string is rendered
        \param x Position, where the string is constructed.
        \param y Position, where the string is constructed.
        \param alignment how to position text relative top position
        \param yaxis up or down text
        \param affine Position etc., where the text is constructed.
    */
    void GetVpath( const wxString& text, a2dVpath& vpath, double x = 0, double y = 0, int alignment = wxMINX | wxMINY, bool yaxis = true, const a2dAffineMatrix& affine = a2dIDENTITY_MATRIX );

    // Documented in base class
    bool AlwaysWriteSerializationId() const { return true; }

    //! Append fonts of this type to the list
    /*! Using a2dFont::GetInfoList(list), will enumerate all fonts, of all types
    */
    static void GetInfoList( a2dFontInfoList& list );

    //! Create the font from a fontinfo description.
    /*! The a2dFont::CreateFont will allways return a valid font. The derived fonts should
        return NULL.

        This routine will first try to find the exact font, as described in the fontinfo. If it does
        not find this font, it will try to create any font of any font type, that matches part of the
        string. eg. if a user asks for a dc-font called "Arial Black" and this is not installed on the
        system, this routine might return as a replacement a freetype-font called "Arial".

        \param info font info for creating font
        \param force This param does not have a function in a2dFont, but it should be used in
            its descendants. When force is false, only the exact match should be made. When true, the
            routine should try to match any likely candidate.
    */
    static a2dFont CreateFont( const a2dFontInfo& info, bool force = false );

    //! Create a string description of the font.
    /*! \return A string of the format type-name-style-size-alignment(-extra)
    */
    wxString CreateString();

    //! create a unique string presenting a a2dFONT_FREETYPE type font (see constructor for parameters)
    static wxString CreateString( const wxString& filename, double size, wxFontEncoding encoding, int weight );
    //! create a unique string presenting a a2dFONT_STROKED type font (see constructor for parameters)
    static wxString CreateString( const wxString& filename, double size, bool monoSpaced, double weight );
    //! create a unique string presenting a a2dFONT_WXDC type font (see constructor for parameters)
    static wxString CreateString( a2dFontType type, const wxFont& font, double size, wxFontEncoding encoding );

    //! Set the font filename and load the font.
    /*! \param filename The filename of the font. This may either be a full path, or just the
        fontname. In the latter case, the fontpath of a2dGlobals will be used to search the font.
        \sa a2dGlobal
    */
    void SetFilename( const wxString& filename );
    //! Get the font filename.
    /*! \param filenameonly The returned filename will be stripped of drive-letters and directories.
        \return The filename of the font.
    */
    wxString GetFilename( bool filenameonly = false ) const;

    //! Get the weight of the font.
    /*! \return The weight (in this case stroke width) in percentages of the font size. There are two exceptions.
        0: Draws the font with a pen of 1 pixel wide. -1: Draws the font with a normalized pen, as the font-designer intended it.
    */
    double GetWeight() const;

    //! Set the weight of the font.
    /*! \param weight The weight (in this case stroke width) in percentages of the font size. There are two exceptions.
        0: Draws the font with a pen of 1 pixel wide. -1: Draws the font with a normalized pen, as the font-designer intended it.
    */
    void SetWeight( double weight );

    //! Set the weight of the font, from a string.
    /*! \param weight One of: "Normalized Pen", "Pixel pen" or a string starting with the width in percentages, ending with %.
    */
    void SetWeight( const wxString& weight );

    //! load method of glyph, see Freetype refence for meaning of FT_LOAD_*
    enum a2dFontLoadMethod
    {
        a2d_LOAD_DEFAULT                      = 0x0,
        a2d_LOAD_NO_SCALE                     = 0x1,
        a2d_LOAD_NO_HINTING                   = 0x2,
        a2d_LOAD_RENDER                       = 0x4,
        a2d_LOAD_NO_BITMAP                    = 0x8,
        a2d_LOAD_VERTICAL_LAYOUT              = 0x10,
        a2d_LOAD_FORCE_AUTOHINT               = 0x20,
        a2d_LOAD_CROP_BITMAP                  = 0x40,
        a2d_LOAD_PEDANTIC                     = 0x80,
        a2d_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH  = 0x200,
        a2d_LOAD_NO_RECURSE                   = 0x400,
        a2d_LOAD_IGNORE_TRANSFORM             = 0x800,
        a2d_LOAD_MONOCHROME                   = 0x1000,
        a2d_LOAD_LINEAR_DESIGN                = 0x2000,
        a2d_LOAD_NO_AUTOHINT                  = 0x8000U
    };

    //! Set the way glyphs are loaded by freetype. Important for small fonts.
    //! a2d_LOAD_DEFAULT;
    //! a2d_LOAD_DEFAULT | a2d_LOAD_NO_HINTING;
    //! a2d_LOAD_DEFAULT | a2d_LOAD_FORCE_AUTOHINT;
    static void SetLoadFlags( wxInt32 loadMethodMask );


    wxObjectRefData* CreateRefData() const;

    wxObjectRefData* CloneRefData( const wxObjectRefData* data ) const;

#if wxART2D_USE_CVGIO
private:
    // Not implemented anymore, please use ...
    virtual void Load( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerIn& WXUNUSED( parser ), a2dXmlSer_flag WXUNUSED( xmlparts ) ) {}
    virtual void Save( wxObject* WXUNUSED( parent ), a2dIOHandlerXmlSerOut& WXUNUSED( out ), a2dXmlSer_flag WXUNUSED( xmlparts ) , a2dObjectList* WXUNUSED( towrite ) ) {}
#endif //wxART2D_USE_CVGIO

public:
private:
    DECLARE_ABSTRACT_CLASS( a2dFont )

    static void ClearFontCache() { ms_fontCache.clear(); }

    static map< wxString, a2dFont > ms_fontCache;
};


/******************************************************************************
    predefined style properties
*******************************************************************************/

//! several predefined style properties
/*! stock style objects
    \defgroup stockobjects predefined StyleProperties

    \ingroup property style
*/

//! global a2dFill stock object for WHITE filling
/*! \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dFill* )    a2dWHITE_FILL;

//! global a2dFill stock object for BLACK filling
/*! \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dFill* )    a2dBLACK_FILL;

//! global a2dFill stock object for TRANSPARENT filling
/*! \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dFill* )    a2dTRANSPARENT_FILL;

//! global a2dFill stock object for INHERTED from parent object filling
/*! \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dFill* )    a2dINHERIT_FILL;

//! global a2dFill stock object for drawing fill of selected objects
/*! \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dFill* )    a2dSELECT_FILL;

//! global a2dFill stock object for defining NO filling
/*! If an a2dCanvasObject's fill is set to a2dNullFILL, it will use the layer
fill, if there is one. If you want to make sure, that no fill is used,
use a2dTRANSPARENT_FILL
\ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dFill* )    a2dNullFILL;

//! global a2dStroke stock object for BLACK stroking
/*! \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dStroke* )    a2dBLACK_STROKE;

//! global a2dStroke stock object for WHITE stroking
/*! \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dStroke* )    a2dWHITE_STROKE;

//! global a2dStroke stock object for TRANSPARENT stroking
/*! \see wxNullStroke \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dStroke* )    a2dTRANSPARENT_STROKE;

//! global a2dStroke stock object for INHERTED from parent object stroking
/*! \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dStroke* )    a2dINHERIT_STROKE;

//! global a2dStroke stock object for NO stroking
/*! If an a2dCanvasObject's stroke is set to a2dNullSTROKE, it will use the layer
stroke, if there is one. If you want to make sure, that no stroke is used,
use a2dTRANSPARENT_STROKE
\ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dStroke* )    a2dNullSTROKE;

//! global a2dStroke stock object for SELECTED object stroking
/*! \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dStroke* )    a2dLAYER_SELECT_STROKE;

//! global a2dFont stock object for NO font
/*! \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dFont* )    a2dNullFONT;

//! global a2dFont stock object for default font
/*! \ingroup stockobjects */
A2DARTBASEDLLEXP_DATA( extern const a2dFont* )    a2dDEFAULT_CANVASFONT;

//! to initialize stock style ( a2dStroke a2dFill ) objects.
/*!
  \sa  a2dCanvasModule

  \ingroup stockobjects

*/
extern A2DARTBASEDLLEXP void a2dCanvasInitializeStockObjects ();

//! to delete stock style ( a2dStroke a2dFill ) objects.
/*!
    \sa  a2dCanvasModule

    \ingroup stockobjects
*/
extern A2DARTBASEDLLEXP void a2dCanvasDeleteStockObjects ();

//!initiates Freetype library
class A2DARTBASEDLLEXP a2dFreetypeModule : public wxModule
{
public:

    a2dFreetypeModule();

    virtual bool OnInit();
    virtual void OnExit();

private:

    DECLARE_DYNAMIC_CLASS( a2dFreetypeModule )
};


extern wxString StrokeStyle2String( a2dStrokeStyle style );
extern a2dStrokeStyle StrokeString2Style( const wxString& stylestr );
extern wxString FillStyle2String( a2dFillStyle style );
extern a2dFillStyle FillString2Style( const wxString& stylestr );


//===========================================================================
// a2dExtStroke - a2dStroke and flags for validity of its attributes:
//===========================================================================

class a2dExtStroke
{
	static bool HasIndividualAttributes(a2dStrokeStyle style);
public:
	a2dExtStroke();
	a2dExtStroke(const a2dStroke& stroke);

	// Initialises all attributes with <stroke>.
	void Set(const a2dStroke& stroke);
	// Compares with <stroke> and sets all unequal attributes to invalid.
	// Attributes that are still invalid will not compared again.
	void Mix(const a2dStroke& stroke);
	// Returns the result of setting all valid attributes to <stroke>.
	a2dStroke Get(const a2dStroke& stroke) const;

	// Default properties, used by ::Get(...) to initialise new stroke object
	// when replacing non-stroke-object style (e.g. 'transparent', 'layer' or 'inherit').
	void SetDefault(const a2dStroke& stroke) { m_Default = stroke; }
	const a2dStroke& GetDefault() const { return m_Default; }

	// -- style --
	void SetStyle(a2dStrokeStyle style);
	bool IsStyleValid() const { return m_bStyle; }
	a2dStrokeStyle GetStyle() const { return m_Style; }

	// Returns wether stroke attributes are initialised.
	bool IsInitialised() const { return m_bInitialised; }

	// Returns wether style has individual, editable attributes.
	// E.g. 'transparent', 'layer' or 'inherit' styles have no individual attributes.
	bool HasEditableStyle() const { return m_bEditable; }

	// -- color --
	void SetColor(const wxColour& color);
	bool IsColorValid() const { return m_bColor; }
	wxColour GetColor() const { return m_Color; }

	// -- width --
	void SetWidth(const float& width);
	bool IsWidthValid() const { return m_bWidth; }
	float GetWidth() const { return m_Width; }

	// -- cap --
	void SetCap(wxPenCap cap);
	bool IsCapValid() const { return m_bCap; }
	wxPenCap GetCap() const { return m_Cap; }

	// -- join --
	void SetJoin(wxPenJoin join);
	bool IsJoinValid() const { return m_bJoin; }
	wxPenJoin GetJoin() const { return m_Join; }

private:
	void SetAttributes(const a2dStroke& stroke);

private:
	a2dStroke m_Default;

	a2dStrokeStyle m_Style;
	wxColour m_Color;
	float m_Width;
	wxPenCap m_Cap;
	wxPenJoin m_Join;

	bool m_bStyle;
	bool m_bInitialised;
	bool m_bEditable;
	bool m_bColor;
	bool m_bWidth;
	bool m_bCap;
	bool m_bJoin;
};


//===========================================================================
// a2dExtFill - a2dFill and flag for validity:
//===========================================================================

class a2dExtFill
{
public:
	a2dExtFill();
	a2dExtFill(const a2dFill& fill);

    // Initialises with <fill>.
	void Set(const a2dFill& fill);
	// Compares with <fill> and sets to invalid if unequal.
	void Mix(const a2dFill& fill);
	// Returns the a2dFill member object if valid, or <fill> instead.
	a2dFill Get(const a2dFill& fill) const;
	
	bool IsValid() const { return m_bFill; }
	// Returns the a2dFill member object.
	a2dFill Get() const { return m_Fill; }

private:
	a2dFill m_Fill;
	bool m_bFill;
};

#endif /* __STYLEBASE_H__ */

