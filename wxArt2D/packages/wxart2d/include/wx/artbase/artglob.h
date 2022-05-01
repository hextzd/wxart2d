/*! \file wx/artbase/artglob.h
    \brief classes for initializing the artbase modules, and set paths to be used for fonts etc.
    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: artglob.h,v 1.23 2009/08/20 20:39:37 titato Exp $
*/


#ifndef __A2DARTGLOB_H__
#define __A2DARTGLOB_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/module.h>

#include "wx/geometry.h"
#include "wx/general/genmod.h"

#ifdef A2DARTBASEMAKINGDLL
#define A2DARTBASEDLLEXP WXEXPORT
#define A2DARTBASEDLLEXP_DATA(type) WXEXPORT type
#define A2DARTBASEDLLEXP_CTORFN
#elif defined(WXART2D_USINGDLL)
#define A2DARTBASEDLLEXP WXIMPORT
#define A2DARTBASEDLLEXP_DATA(type) WXIMPORT type
#define A2DARTBASEDLLEXP_CTORFN
#else // not making nor using DLL
#define A2DARTBASEDLLEXP
#define A2DARTBASEDLLEXP_DATA(type) type
#define A2DARTBASEDLLEXP_CTORFN
#endif


//! accuracy used to have some limit to calculation like hittesting
#define a2dACCUR 1e-6

//! this to define if coordinate numbers are integer or doubles
#ifdef wxUSE_INTPOINT
typedef  wxPoint2DInt a2dPoint2D;
#else
typedef  wxPoint2DDouble a2dPoint2D;
#endif

//! defines PI
A2DARTBASEDLLEXP_DATA( extern const double ) wxPI;

//! conversion from degrees to radians
extern A2DARTBASEDLLEXP double wxDegToRad( double deg );

//! conversion from radians to degrees
extern A2DARTBASEDLLEXP double wxRadToDeg( double rad );

//! round to integer such that e.g 2.5 < x < 3.5 becomes 3
#define Round( x ) (int) floor( (x) + 0.5 )

//! calculate number of segments in an arc such that a certain accuracy is maintained
extern A2DARTBASEDLLEXP void Aberration( double aber, double angle, double radius, double& dphi, unsigned int& segments );

//! good replacement for pow(x,2)
inline double wxSqr( double x ) { return x * x; }

//! initiation module for the wxArt2D library
/*!
    One instance of is used to initiate global settings.

    Like a2dGlobal

    \ingroup global
*/
class A2DARTBASEDLLEXP a2dArtBaseModule : public wxModule
{
public:

    a2dArtBaseModule();

    virtual bool OnInit();
    virtual void OnExit();

private:
    DECLARE_DYNAMIC_CLASS( a2dArtBaseModule )
};

//! class for storing paths and settings, which are used within the artbase module.
class A2DARTBASEDLLEXP a2dGlobal : public a2dGeneralGlobal
{

    DECLARE_CLASS( a2dGlobal )

public:

    //! constructor
    a2dGlobal();

    //! destructor
    virtual ~a2dGlobal();

    //! based on angle and radius and m_displayaberration calculate a proper delta phi and number of segments
    /*!
        The calculation is used for circular arc segments
    */
    void Aberration( double angle, double radius, double& dphi, unsigned int& segments );

    //! set the display aberation, display of curved shapes
    /*! will not deviate more then this from the ideal curve.
       \param aber maximum deviation in device coordianates
       \remark the smaller the number the longer the drawing takes.
    */
    static void SetDisplayAberration( double aber ) { m_displayaberration = aber; }

    //! get the display aberation, display of curved shapes
    //! will not deviate more then this from the ideal curve.
    static double GetDisplayAberration() { return m_displayaberration; }

    static void SetRoundFactor( double roundFactor ) { m_roundFactor = roundFactor; }

    //! get the display aberation, display of curved shapes
    //! will not deviate more then this from the ideal curve.
    static double GetRoundFactor() { return m_roundFactor; }

    //!set drawing threshold to the given number of pixels
    /*! every object with a boundingbox (in device coordinates) smaller than
        the number of pixels in width or height will not be rendered.
        For drawing with a high dynamic range (small and big objects combined),
        this means a gain in speed.

        \remark default value is 0
    */
    inline static void SetPrimitiveThreshold( wxUint16 pixels, bool asrect = true ) { m_drawingthreshold = pixels; m_asrectangle = asrect; }

    //!set threshold at which polygon is drawn filled or only outline
    inline void SetPolygonFillThreshold( wxUint16 pixels ) { m_polygonFillThreshold = pixels; }

    //!get threshold at which polygon is drawn filled or only outline
    inline wxUint16 GetPolygonFillThreshold() { return m_polygonFillThreshold; }

    //!get drawing threshold \sa SetDrawingThreshold
    inline static wxUint16 GetPrimitiveThreshold() { return m_drawingthreshold; }

    //! underneath the threshold draw rectangles instead of the real object.
    inline static bool GetThresholdDrawRectangle() { return m_asrectangle; }

    //! Path for Fonts
    a2dPathList& GetFontPathList() { return m_fontpath; }

    //! Path for Images
    a2dPathList& GetImagePathList() { return m_imagepath; }

    //! Path for Icons and small bitmaps
    a2dPathList& GetIconPathList() { return m_iconpath; }

    //! conversion from polygon/polyline into arc in database units
    inline static void SetAberPolyToArc( double aber ) { m_poly2arcaber = aber; }

    //! conversion from polygon/polyline into arc in database units
    inline static double GetAberPolyToArc() { return m_poly2arcaber; }

    //! conversion from arc into lines in database units
    inline static void SetAberArcToPoly( double aber ) { m_arc2polyaber = aber; }

    //! conversion from arc into lines in database units
    inline static double GetAberArcToPoly() { return m_arc2polyaber; }

protected:

    static a2dPathList m_fontpath;

    static a2dPathList m_imagepath;

    static a2dPathList m_iconpath;

    //! object smaller than this value will not be rendered
    static wxUint16 m_drawingthreshold;

    //!get threshold at which polygon is drawn filled or only outline
    static wxUint16 m_polygonFillThreshold;

    //! underneath the threshold draw rectangles if true else nothing
    static bool m_asrectangle;

    //! conversion from arc into lines in database units
    static double m_arc2polyaber;

    static double m_poly2arcaber;

    static double m_displayaberration;

    static double m_roundFactor;

};

//! store and convert number to number with unit and visa versa. e.g. 1.23e-6 => 1.23 * 1e-6
/*!
    This makes it easy to recover the unit as a string if needed.
    Als one can normalize the multiplier part to something that closest to a string multiplier.

    Used often to convert to and from user units. If the default unit used by the
    user is in MicroMeters (um), numbers stored as doubles, need to be converted back and forth
    to this unit.
    The full number is stored as a number + multiplier part. The multiplier is always stored in metric.
    So if inch, it will be 0.0254, and mil will be 0.00254
*/
class a2dDoMu
{
public:
    a2dDoMu();
    ~a2dDoMu();

    //!constructor on number and multiplier 1.1 and 1e-6
    a2dDoMu( double number, double multi = 1, bool normalize = true );
    //!constructor on number in meters and recalculate for multiplier as string
    a2dDoMu( double number, const wxString& multi );
    //!constructor on number string and multiplier string "1.1" "um"
    a2dDoMu( const wxString& number, const wxString& multi );

    //!constructor on string containing number and multiplier "1.1 um"
    a2dDoMu( const wxString& numberMulti );

    //!compose a unit based a string "1.1um"
    bool Eval( const wxString& param );
    //!get value in meters
    double GetValue() const;
    //!get the number 1.1 um -> 1.1
    double GetNumber() const;
    //!get the number 1.1 um -> 1e-6
    double GetMultiplier() const;
    //!get the number 1.1 um -> "1.1 um"
    wxString GetValueString() const;
    //!get the number 1.1 um -> "um"
    wxString GetMultiplierString() const;
    //!get the number 1.1 um -> "1.1"
    wxString GetNumberString() const;

    //! calculate from m_multi a string like "1e-6" => "um"
    static wxString GetMultiplierString( double multi );

    //! calculate how to get to meters from a multiplier string e.g. um => 1e-6
    static bool GetMultiplierFromString( const wxString& mul, double& multi );

    //!copy
    a2dDoMu& operator=( const a2dDoMu& );
    //!as Eval
    a2dDoMu& operator=( const wxChar* );
    //!convert to double
    operator double() const;
    //!double becomes the number
    a2dDoMu&   operator=( double );
    int operator==( const a2dDoMu& ) const;
    int operator!=( const a2dDoMu& ) const;

    //! when converting a number to a string this accuracy will be used
    static void SetAccuracy( int accur ) { m_accuracy = accur; }

protected:
    //!multiplier e.g. "um" will be 1e-6
    double m_multi;
    //!non multiplied number
    double m_number;

    //! how much fractional digits, when conversion to string
    static int m_accuracy;
};

//! global a2dCanvasGlobal to have easy access to global settings
A2DARTBASEDLLEXP_DATA( extern a2dGlobal* ) a2dGlobals;

//! Define the manner in which a2dDrawer2D draws to the device.
/*!
    The draw style tells how a a2dDrawer2D should draw independent of
    the style ( a2dFill a2dStroke ) set.
    The drawstyle overrules style if needed.

    \ingroup drawer
*/
enum a2dDrawStyle
{
    a2dFILLED, /*!< normal drawing style, primitive is filled and outlined according to the style ( a2dStroke a2dFill ) set */
    a2dWIREFRAME, /*!< only a wire frame of the drawn primitive is drawn, as is set by the current a2dStroke */
    a2dWIREFRAME_ZERO_WIDTH, /*!< only a zero width wire frame of the drawn primitive is drawn, Color of the current a2dStroke, is used */
    a2dWIREFRAME_INVERT, /*!< as a2dWIREFRAME, but the outline is the inverted of what is on the device/buffer */
    a2dWIREFRAME_INVERT_ZERO_WIDTH, /*!<  as wxINVERT_WIREFRAME but zero width outline */
    a2dFIX_STYLE, /*!< last set stroke and fill remain until a2dFILLED set as drawing style*/
    a2dFIX_STYLE_INVERT /*!< last set stroke and fill remain but using inverted drawing, until a2dFILLED set as drawing style*/
};

#include <wx/dynarray.h>

#include <wx/listimpl.cpp>

#if defined(WXART2D_USINGDLL)
template class A2DARTBASEDLLEXP std::allocator<wxPoint>;
template class A2DARTBASEDLLEXP std::vector<wxPoint>;
template class A2DARTBASEDLLEXP std::allocator<wxRealPoint>;
template class A2DARTBASEDLLEXP std::vector<wxRealPoint>;
#endif

//! Used internal for scanned ( gradient etc. ) polygon filling. \sa a2dAETList
/*!
    \ingroup drawer
*/
class A2DARTBASEDLLEXP a2dAET
{
public:
    void CalculateLineParameters( const wxRealPoint& p1 , const wxRealPoint& p2 );
    void CalculateXs( double y );

    //line parameters
    bool m_horizontal;
    double m_BdivA;
    double m_CdivA;
    int m_index;
    int m_direction;
    //intersection point with scanline;
    int m_xs;
};

//! Holds Active edge table. Used internal for scanned ( gradient etc. ) polygon filling.
WX_DECLARE_LIST_WITH_DECL( a2dAET, a2dAETList, class A2DARTBASEDLLEXP );

//! Holds local minimums in polygon. Used internal for scanned ( gradient etc. ) polygon filling.
WX_DECLARE_LIST_WITH_DECL( int, a2dCriticalPointList, class A2DARTBASEDLLEXP );

#include <wx/listimpl.cpp>

//! Holds all clipping regions within a a2dDrawer2D
WX_DECLARE_LIST_WITH_DECL( wxRegion, a2dClipRegionList, class A2DARTBASEDLLEXP );

//! number of steps when converting a spline to lines.
const int SPLINE_STEP = 20;

#endif /* __A2DARTGLOB_H__ */

